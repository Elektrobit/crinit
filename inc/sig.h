// SPDX-License-Identifier: MIT
/**
 * @file sig.h
 * @brief Header related to signature checking.
 */
#ifndef __SIG_H__
#define __SIG_H__

#include <stddef.h>
#include <stdint.h>

#define CRINIT_SIGNATURE_DEFAULT_ROOT_KEY_DESC "crinit-root"
#define CRINIT_SIGNATURE_FILE_SUFFIX ".sig"
#define CRINIT_RSASSA_PSS_SIGNATURE_SIZE 512uL

#define CRINIT_SIGNED_KEYS_INITIAL_SIZE 32
#define CRINIT_SIGNED_KEYS_SIZE_INCREMENT 32

int crinitSigSubsysInit(char *rootKeyDesc);
void crinitSigSubsysDestroy(void);
int crinitLoadAndVerifySignedKeys(char *sigKeyDir);
int crinitVerifySignature(const uint8_t *data, size_t dataSz, const uint8_t *signature);

#endif /* __SIG_H__ */
