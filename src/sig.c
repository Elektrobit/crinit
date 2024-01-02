// SPDX-License-Identifier: MIT
/**
 * @file sig.c
 * @brief Implementation of signature checking.
 */

#include "sig.h"

#include <linux/keyctl.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "common.h"
#include "fseries.h"
#include "logio.h"
#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/sha256.h"

#define CRINIT_SIGNATURE_PK_DATA_MAX_SIZE 4096uL
#define CRINIT_SIGNATURE_PK_PEM_EXTENSION ".pem"
#define CRINIT_SIGNATURE_PK_DER_EXTENSION ".der"

static struct {
    mbedtls_pk_context crinitRootKey;
    mbedtls_pk_context *crinitSignedKeys;
    size_t numSignedKeys;
    pthread_mutex_t crinitSigLock;
} crinitSigCtx;

/**
 * Convenience wrapper macro for the read operation of the keyctl system call.
 *
 * See https://man7.org/linux/man-pages/man2/keyctl.2.html
 *
 * @param keyID  They key ID to read the payload from (int32_t).
 * @param pld    Return pointer where the payload gets written (char *).
 * @param pldSz  Size of the payload buffer (size_t).
 *
 * @return  The length of the key's payload on success, -1 otherwise.
 */
#define crinitKeyctlRead(keyID, pld, pldSz) syscall(SYS_keyctl, KEYCTL_READ, keyID, pld, pldSz, 0)
/**
 * Convenience wrapper macro for the search operation of the keyctl system call.
 *
 * See https://man7.org/linux/man-pages/man2/keyctl.2.html
 *
 * @param keyringID  ID of the keyring to search in (int32_t). Search is recursive for all nested keyrings with `search`
 *                   permission.
 * @param keyType    The type of the key to search for (char *, see man page).
 * @param keyDesc    The description string of the key to search for (char *).
 *
 * @return  The key's ID (as int32_t) on success, -1 otherwise.
 */
#define crinitKeyctlSearch(keyringID, keyType, keyDesc) \
    syscall(SYS_keyctl, KEYCTL_SEARCH, keyringID, keyType, keyDesc, 0)

static int crinitLoadAndVerifySignedKeysFromFileSeries(mbedtls_pk_context *tgt, const crinitFileSeries_t *src);

int crinitSigSubsysInit(char *rootKeyDesc) {
    crinitSigCtx.numSignedKeys = 0;
    crinitSigCtx.crinitSignedKeys = NULL;
    pthread_mutex_init(&crinitSigCtx.crinitSigLock, NULL);

    long rootKeyId = crinitKeyctlSearch(KEY_SPEC_USER_KEYRING, "user", rootKeyDesc);
    if (rootKeyId == -1) {
        crinitErrnoPrint("Could not find crinit root key named '%s' in user keyring.", rootKeyDesc);
        free(crinitSigCtx.crinitSignedKeys);
        pthread_mutex_destroy(&crinitSigCtx.crinitSigLock);
        return -1;
    }

    unsigned char rootKeyData[CRINIT_SIGNATURE_PK_DATA_MAX_SIZE];
    long rootKeyLen = crinitKeyctlRead(rootKeyId, rootKeyData, sizeof(rootKeyData));
    if (rootKeyLen == -1) {
        crinitErrnoPrint("Could not read crinit root key named '%s' from user keyring.", rootKeyDesc);
        free(crinitSigCtx.crinitSignedKeys);
        pthread_mutex_destroy(&crinitSigCtx.crinitSigLock);
        return -1;
    }
    if ((size_t)rootKeyLen > sizeof(rootKeyData)) {
        crinitErrPrint(
            "Crinit root key named '%s' in user keyring is larger (%zu Bytes) than the allowed maximum of %zu Bytes.",
            rootKeyDesc, (size_t)rootKeyLen, sizeof(rootKeyData));
        free(crinitSigCtx.crinitSignedKeys);
        pthread_mutex_destroy(&crinitSigCtx.crinitSigLock);
        return -1;
    }

    mbedtls_pk_init(&crinitSigCtx.crinitRootKey);
    mbedtls_pk_parse_public_key(&crinitSigCtx.crinitRootKey, rootKeyData, rootKeyLen);
    mbedtls_pk_type_t keyType = mbedtls_pk_get_type(&crinitSigCtx.crinitRootKey);
    if (keyType == MBEDTLS_PK_NONE) {
        crinitErrPrint("Could not get type of user keyring public key \'%s\'.", rootKeyDesc);
        mbedtls_pk_free(&crinitSigCtx.crinitRootKey);
        free(crinitSigCtx.crinitSignedKeys);
        pthread_mutex_destroy(&crinitSigCtx.crinitSigLock);
        return -1;
    }
    crinitInfoPrint("Key \'%s\' successfully loaded.", rootKeyDesc);
    if (mbedtls_pk_can_do(&crinitSigCtx.crinitRootKey, MBEDTLS_PK_RSA) == 0) {
        crinitErrPrint("The key data from \'%s\' out of the user keyring did not contain a valid RSA public key.",
                       rootKeyDesc);
        mbedtls_pk_free(&crinitSigCtx.crinitRootKey);
        free(crinitSigCtx.crinitSignedKeys);
        pthread_mutex_destroy(&crinitSigCtx.crinitSigLock);
        return -1;
    }
    return 0;
}

void crinitSigSubsysDestroy(void) {
    mbedtls_pk_free(&crinitSigCtx.crinitRootKey);
    for (size_t i = 0; i < crinitSigCtx.numSignedKeys; i++) {
        mbedtls_pk_free(&crinitSigCtx.crinitSignedKeys[i]);
    }
    free(crinitSigCtx.crinitSignedKeys);
    pthread_mutex_destroy(&crinitSigCtx.crinitSigLock);
}

int crinitLoadAndVerifySignedKeys(char *sigKeyDir) {
    crinitNullCheck(-1, sigKeyDir);
    crinitFileSeries_t pemKeys, derKeys;

    if (crinitFileSeriesFromDir(&derKeys, sigKeyDir, CRINIT_SIGNATURE_PK_DER_EXTENSION, false) == -1) {
        crinitErrPrint("Could not search directory '%s' for DER-encoded public keys.", sigKeyDir);
        return -1;
    }
    if (crinitFileSeriesFromDir(&pemKeys, sigKeyDir, CRINIT_SIGNATURE_PK_PEM_EXTENSION, false) == -1) {
        crinitErrPrint("Could not search directory '%s' for PEM-encoded public keys.", sigKeyDir);
        crinitDestroyFileSeries(&derKeys);
        return -1;
    }

    size_t numSignedKeys = derKeys.size + pemKeys.size;
    mbedtls_pk_context *signedKeys = malloc(sizeof(*signedKeys) * numSignedKeys);
    if (signedKeys == NULL) {
        crinitErrnoPrint("Could not allocate memory for %zu signature public key contexts.", numSignedKeys);
        crinitDestroyFileSeries(&derKeys);
        crinitDestroyFileSeries(&pemKeys);
        return -1;
    }
    for (size_t i = 0; i < numSignedKeys; i++) {
        mbedtls_pk_init(&signedKeys[i]);
    }

    if (crinitLoadAndVerifySignedKeysFromFileSeries(signedKeys, &derKeys) == -1) {
        crinitErrPrint("Could not load and verify all DER-encoded keys in '%s'.", sigKeyDir);
        crinitDestroyFileSeries(&derKeys);
        crinitDestroyFileSeries(&pemKeys);
        for (size_t i = 0; i < numSignedKeys; i++) {
            mbedtls_pk_free(&signedKeys[i]);
        }
        free(signedKeys);
        return -1;
    }

    if (crinitLoadAndVerifySignedKeysFromFileSeries(signedKeys + derKeys.size, &pemKeys) == -1) {
        crinitErrPrint("Could not load and verify all PEM-encoded keys in '%s'.", sigKeyDir);
        crinitDestroyFileSeries(&derKeys);
        crinitDestroyFileSeries(&pemKeys);
        for (size_t i = 0; i < numSignedKeys; i++) {
            mbedtls_pk_free(&signedKeys[i]);
        }
        free(signedKeys);
        return -1;
    }

    return 0;
}

int crinitVerifySignature(uint8_t *data, uint8_t *signature) {
    crinitNullCheck(-1, data, signature);

    return 0;
}

static int crinitLoadAndVerifySignedKeysFromFileSeries(mbedtls_pk_context *tgt, const crinitFileSeries_t *src) {
    uint8_t readbuf[CRINIT_SIGNATURE_PK_DATA_MAX_SIZE];
    char pathbuf[PATH_MAX];
    for (size_t i = 0; i < src->size; i++) {
        int ret = snprintf(pathbuf, sizeof(pathbuf), "%s/%s", src->baseDir, src->fnames[i]);
        if (ret == -1) {
            crinitErrnoPrint("Could not format full path of public key '%s/%s'.", src->baseDir, src->fnames[i]);
            return -1;
        }

        if (ret >= PATH_MAX) {
            crinitErrPrint("The path '%s/%s' is too long to process.", src->baseDir, src->fnames[i]);
            return -1;
        }

        FILE *pkf = fopen(pathbuf, "rb");
        if (pkf == NULL) {
            crinitErrnoPrint("Could not open '%s' for reading.", pathbuf);
            return -1;
        }
        ret = fread(readbuf, sizeof(*readbuf), sizeof(readbuf), pkf);
        if (ferror(pkf) || !feof(pkf)) {
            crinitErrPrint("Could not read to the end of file of '%s'.", pathbuf);
            fclose(pkf);
            return -1;
        }
        fclose(pkf);
    }

    return 0;
}
