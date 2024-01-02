// SPDX-License-Identifier: MIT
/**
 * @file sig.h
 * @brief Header related to signature checking.
 */
#ifndef __SIG_H__
#define __SIG_H__

#include <stdint.h>

#define CRINIT_SIGNATURE_DEFAULT_ROOT_KEY_DESC "crinit-root"

#define CRINIT_SIGNED_KEYS_INITIAL_SIZE 32
#define CRINIT_SIGNED_KEYS_SIZE_INCREMENT 32

int crinitSigSubsysInit(char *rootKeyDesc);
void crinitSigSubsysDestroy(void);
int crinitLoadAndVerifySignedKeys(char *sigKeyDir);
int crinitVerifySignature(uint8_t *data, uint8_t *signature);

#endif /* __SIG_H__ */
