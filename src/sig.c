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
#include "logio.h"
#include "mbedtls/error.h"
#include "mbedtls/pk.h"
#include "mbedtls/sha256.h"

#define CRINIT_SIGNATURE_PK_DATA_MAX_SIZE 4096uL

static struct {
    mbedtls_pk_context crinitRootKey;
    mbedtls_pk_context *crinitSignedKeys;
    size_t numSignedKeys;
    size_t allocSignedKeys;
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

int crinitSigSubsysInit(char *rootKeyDesc) {
    crinitSigCtx.numSignedKeys = 0;
    crinitSigCtx.crinitSignedKeys = malloc(sizeof(*crinitSigCtx.crinitSignedKeys) * CRINIT_SIGNED_KEYS_INITIAL_SIZE);
    if (crinitSigCtx.crinitSignedKeys == NULL) {
        crinitErrnoPrint("Could not allocate memory for signature public key contexts.");
        return -1;
    }
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
    return 0;
}

int crinitVerifySignature(char *str, uint8_t *signature) {
    crinitNullCheck(-1, str, signature);
    return 0;
}
