// SPDX-License-Identifier: MIT
/**
 * @file sig.h
 * @brief Header related to signature checking.
 */
#ifndef __SIG_H__
#define __SIG_H__

#include <stddef.h>
#include <stdint.h>

#define CRINIT_SIGNATURE_DEFAULT_ROOT_KEY_DESC \
    "crinit-root"                               ///< The key description of the root public key within the user keyring
#define CRINIT_SIGNATURE_FILE_SUFFIX ".sig"     ///< The filename suffix identifying signature files
#define CRINIT_RSASSA_PSS_SIGNATURE_SIZE 512uL  ///< The size in Bytes of a signature as used by crinit.

/**
 * Initializes the Crinit signature subsystem.
 *
 * Will read the root key from Kernel user keyring.
 *
 * This function musst be called once before any other function from this header file is used.
 *
 * @param rootKeyDesc  The key description value to search for the root key in the user keyring.
 *
 * @return  0 on success, -1 otherwise.
 */
int crinitSigSubsysInit(char *rootKeyDesc);
/**
 * Frees memory allocated by crinitSigSubsysInit().
 *
 * After calling this function no other fucntions from this header file may be used.
 */
void crinitSigSubsysDestroy(void);
/**
 * Searches given directory for signed public keys and loads them into the signature subsystem.
 *
 * Signatures of the loaded keys must match the root key.
 *
 * If the signed downstream public keys should be used to verify configuration files, this function must be called
 * before parsing them.
 *
 * Keys may be in DER (.der) or PEM (.pem) format and must each have a signature file (e.g. `<keyfile>.pem.sig`) in the
 * same directory.
 *
 * @param sigKeyDir  The path to the directory from where to load/verify the keys.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitLoadAndVerifySignedKeys(char *sigKeyDir);
/**
 * Verify the signature of arbitrary data using the keys loaded to the signature subsystem.
 *
 * See crinitSigSubsysInit() and crinitLoadAndVerifySignedKeys() for information on prior subsytem setup.
 *
 * Verification uses the RSA-PSS algorithm with SHA256 hashes. It will check the hashed data against all loaded keys. If
 * one matches, verification is passed.
 *
 * @param data       The data array to check against the signature.
 * @param dataSz     The number of elements in the data array.
 * @param signature  A byte array containing the signature, must be of #CRINIT_RSASSA_PSS_SIGNATURE_SIZE.
 *
 * @return  0 on success, -1 otherwise
 */
int crinitVerifySignature(const uint8_t *data, size_t dataSz, const uint8_t *signature);

#endif /* __SIG_H__ */
