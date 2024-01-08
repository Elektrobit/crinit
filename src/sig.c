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
#include "mbedtls/version.h"

#define CRINIT_SIGNATURE_PK_DATA_MAX_SIZE 4096uL
#define CRINIT_RSASSA_PSS_SIGNATURE_SIZE 512uL
#define CRINIT_RSASSA_PSS_HASH_SIZE 32uL
#define CRINIT_SIGNATURE_PK_PEM_EXTENSION ".pem"
#define CRINIT_SIGNATURE_PK_DER_EXTENSION ".der"
#define CRINIT_MBEDTLS_ERR_MAX_LEN 128  ///< Maximum length of a string generated by mbedtls_strerror()

static struct {
    mbedtls_pk_context rootKey;
    mbedtls_pk_context *signedKeys;
    size_t numSignedKeys;
    pthread_mutex_t sigCtxLock;
} crinitSigCtx;

// Macro definition to support both MbedTLS 2 and 3 interfaces.
#if MBEDTLS_VERSION_MAJOR == 2
#define crinitMbedtlsVerify(ctx, mdAlg, hashlen, hash, sig) \
    mbedtls_rsa_rsassa_pss_verify((ctx), NULL, NULL, MBEDTLS_RSA_PUBLIC, (mdAlg), (hashlen), (hash), (sig))
#else
#define crinitMbedtlsVerify(ctx, mdAlg, hashlen, hash, sig) \
    mbedtls_rsa_rsassa_pss_verify((ctx), (mdAlg), (hashlen), (hash), (sig))
#endif

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

static int crinitGenerateHash(uint8_t *dataHash, const uint8_t *data, size_t dataSz);
static int crinitLoadAndVerifySignedKeysFromFileSeries(mbedtls_pk_context *tgt, const crinitFileSeries_t *src);

int crinitSigSubsysInit(char *rootKeyDesc) {
    crinitSigCtx.numSignedKeys = 0;
    crinitSigCtx.signedKeys = NULL;
    pthread_mutex_init(&crinitSigCtx.sigCtxLock, NULL);

    long rootKeyId = crinitKeyctlSearch(KEY_SPEC_USER_KEYRING, "user", rootKeyDesc);
    if (rootKeyId == -1) {
        crinitErrnoPrint("Could not find crinit root key named '%s' in user keyring.", rootKeyDesc);
        free(crinitSigCtx.signedKeys);
        pthread_mutex_destroy(&crinitSigCtx.sigCtxLock);
        return -1;
    }

    unsigned char rootKeyData[CRINIT_SIGNATURE_PK_DATA_MAX_SIZE];
    long rootKeyLen = crinitKeyctlRead(rootKeyId, rootKeyData, sizeof(rootKeyData));
    if (rootKeyLen == -1) {
        crinitErrnoPrint("Could not read crinit root key named '%s' from user keyring.", rootKeyDesc);
        free(crinitSigCtx.signedKeys);
        pthread_mutex_destroy(&crinitSigCtx.sigCtxLock);
        return -1;
    }
    if ((size_t)rootKeyLen > sizeof(rootKeyData)) {
        crinitErrPrint(
            "Crinit root key named '%s' in user keyring is larger (%zu Bytes) than the allowed maximum of %zu Bytes.",
            rootKeyDesc, (size_t)rootKeyLen, sizeof(rootKeyData));
        free(crinitSigCtx.signedKeys);
        pthread_mutex_destroy(&crinitSigCtx.sigCtxLock);
        return -1;
    }

    mbedtls_pk_init(&crinitSigCtx.rootKey);
    mbedtls_pk_parse_public_key(&crinitSigCtx.rootKey, rootKeyData, rootKeyLen);
    mbedtls_pk_type_t keyType = mbedtls_pk_get_type(&crinitSigCtx.rootKey);
    if (keyType == MBEDTLS_PK_NONE) {
        crinitErrPrint("Could not get type of user keyring public key \'%s\'.", rootKeyDesc);
        mbedtls_pk_free(&crinitSigCtx.rootKey);
        free(crinitSigCtx.signedKeys);
        pthread_mutex_destroy(&crinitSigCtx.sigCtxLock);
        return -1;
    }
    crinitInfoPrint("Key \'%s\' successfully loaded.", rootKeyDesc);
    if (mbedtls_pk_can_do(&crinitSigCtx.rootKey, MBEDTLS_PK_RSA) == 0) {
        crinitErrPrint("The key data from \'%s\' out of the user keyring did not contain a valid RSA public key.",
                       rootKeyDesc);
        mbedtls_pk_free(&crinitSigCtx.rootKey);
        free(crinitSigCtx.signedKeys);
        pthread_mutex_destroy(&crinitSigCtx.sigCtxLock);
        return -1;
    }
    return 0;
}

void crinitSigSubsysDestroy(void) {
    mbedtls_pk_free(&crinitSigCtx.rootKey);
    for (size_t i = 0; i < crinitSigCtx.numSignedKeys; i++) {
        mbedtls_pk_free(&crinitSigCtx.signedKeys[i]);
    }
    free(crinitSigCtx.signedKeys);
    pthread_mutex_destroy(&crinitSigCtx.sigCtxLock);
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

int crinitVerifySignature(const uint8_t *data, size_t dataSz, const uint8_t *signature) {
    crinitNullCheck(-1, data, signature);

    // Generate SHA-256 of input data.
    uint8_t dataHash[CRINIT_RSASSA_PSS_HASH_SIZE];
    if (crinitGenerateHash(dataHash, data, dataSz) == -1) {
        crinitErrPrint("Could not calculate sha256 hash of input data.");
        return -1;
    }

    // Try to verify with root key.
    if (crinitMbedtlsVerify(mbedtls_pk_rsa(crinitSigCtx.rootKey), MBEDTLS_MD_SHA256, CRINIT_RSASSA_PSS_HASH_SIZE,
                            dataHash, signature) == 0) {
        return 0;
    }

    // If that didn't work, try if one of the other keys matches.
    for (size_t i = 0; i < crinitSigCtx.numSignedKeys; i++) {
        if (crinitMbedtlsVerify(mbedtls_pk_rsa(crinitSigCtx.signedKeys[i]), MBEDTLS_MD_SHA256,
                                CRINIT_RSASSA_PSS_HASH_SIZE, dataHash, signature) == 0) {
            return 0;
        }
    }

    // Fall through here if no context has led to a match between hash and signature.
    crinitErrPrint("RSA-PSS signature verification failed.");
    return -1;
}

static int crinitGenerateHash(uint8_t *dataHash, const uint8_t *data, size_t dataSz) {
    crinitNullCheck(-1, dataHash, data);

    char errBuf[CRINIT_MBEDTLS_ERR_MAX_LEN];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
#if MBEDTLS_VERSION_MAJOR == 2
    int err = mbedtls_sha256_starts_ret(&ctx, 0);
#else
    int err = mbedtls_sha256_starts(&ctx, 0);
#endif
    if (err != 0) {
        mbedtls_strerror(err, errBuf, sizeof(errBuf));
        crinitErrPrint("Could not start sha256 calculation. %s", errBuf);
        mbedtls_sha256_free(&ctx);
        return -1;
    }
#if MBEDTLS_VERSION_MAJOR == 2
    err = mbedtls_sha256_update_ret(&ctx, data, dataSz);
#else
    err = mbedtls_sha256_update(&ctx, data, dataSz);
#endif
    if (err != 0) {
        mbedtls_strerror(err, errBuf, sizeof(errBuf));
        crinitErrPrint("Could not perform sha256 calculation. %s", errBuf);
        mbedtls_sha256_free(&ctx);
        return -1;
    }
#if MBEDTLS_VERSION_MAJOR == 2
    err = mbedtls_sha256_finish_ret(&ctx, dataHash);
#else
    err = mbedtls_sha256_finish(&ctx, dataHash);
#endif
    if (err != 0) {
        mbedtls_strerror(err, errBuf, sizeof(errBuf));
        crinitErrPrint("Could not finish sha256 calculation. %s", errBuf);
        mbedtls_sha256_free(&ctx);
        return -1;
    }
    mbedtls_sha256_free(&ctx);
    return 0;
}

static int crinitLoadAndVerifySignedKeysFromFileSeries(mbedtls_pk_context *tgt, const crinitFileSeries_t *src) {
    crinitNullCheck(-1, tgt, src);
    uint8_t readbufKey[CRINIT_SIGNATURE_PK_DATA_MAX_SIZE], readbufSig[CRINIT_RSASSA_PSS_SIGNATURE_SIZE];
    char pathbuf[PATH_MAX];
    for (size_t i = 0; i < src->size; i++) {
        // Read public key.
        int ret = snprintf(pathbuf, sizeof(pathbuf), "%s/%s", src->baseDir, src->fnames[i]);
        if (ret == -1) {
            crinitErrnoPrint("Could not format full path of public key '%s/%s'.", src->baseDir, src->fnames[i]);
            return -1;
        }
        if ((unsigned long)ret >= PATH_MAX - strlen(CRINIT_SIGNATURE_FILE_SUFFIX)) {
            crinitErrPrint("The path '%s/%s' is too long to process.", src->baseDir, src->fnames[i]);
            return -1;
        }
        int keySz = crinitBinReadAll(readbufKey, crinitNumElements(readbufKey), pathbuf);
        if (keySz == -1) {
            crinitErrPrint("Could not read whole file '%s' to memory.", pathbuf);
            return -1;
        }

        // Read signature.
        ret = snprintf(pathbuf, sizeof(pathbuf), "%s%s", pathbuf, CRINIT_SIGNATURE_FILE_SUFFIX);
        if (ret == -1) {
            crinitErrnoPrint("Could not format full path of public key signature file '%s%s'.", pathbuf,
                             CRINIT_SIGNATURE_FILE_SUFFIX);
            return -1;
        }
        if (crinitBinReadAll(readbufSig, crinitNumElements(readbufSig), pathbuf) == -1) {
            crinitErrPrint("Could not read whole file '%s' to memory.", pathbuf);
            return -1;
        }

        // Regenerate original path
        pathbuf[strlen(pathbuf) - strlen(CRINIT_SIGNATURE_FILE_SUFFIX)] = '\0';

        // Verify against root key.
        if (crinitVerifySignature(readbufKey, (size_t)keySz, readbufSig) == -1) {
            crinitErrPrint("Signature verification of '%s' failed.", pathbuf);
            return -1;
        }

        // Build key context and prepare for RSA-PSS.
        mbedtls_pk_parse_public_key(&tgt[i], readbufKey, (size_t)keySz);
        mbedtls_pk_type_t keyType = mbedtls_pk_get_type(&tgt[i]);
        if (keyType == MBEDTLS_PK_NONE) {
            crinitErrPrint("Could not get type of public key \'%s\'.", pathbuf);
            return -1;
        }
        crinitInfoPrint("Key \'%s\' successfully loaded.", pathbuf);
        if (mbedtls_pk_can_do(&tgt[i], MBEDTLS_PK_RSA) == 0) {
            crinitErrPrint("The key data from \'%s\' did not contain a valid RSA public key.", pathbuf);
            return -1;
        }

#if MBEDTLS_VERSION_MAJOR == 2
        mbedtls_rsa_set_padding(mbedtls_pk_rsa(tgt[i]), MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
#else
        int err = mbedtls_rsa_set_padding(mbedtls_pk_rsa(tgt[i]), MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
        if (err != 0) {
            char errBuf[CRINIT_MBEDTLS_ERR_MAX_LEN];
            mbedtls_strerror(err, errBuf, sizeof(errBuf));
            crinitErrPrint("Could not set RSASSA-PSS-compatible padding for RSA context. %s", errBuf);
            return -1;
        }
#endif
    }
    return 0;
}
