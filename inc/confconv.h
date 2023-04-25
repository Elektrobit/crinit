/**
 * @file confconv.h
 * @brief Definitions related to conversion operations from configuration values to structured data.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */

#include <stdbool.h>

#include "envset.h"
#include "ioredir.h"

char **EBCL_confConvToStrArr(int *numElements, const char *confVal, bool doubleQuoting);
int EBCL_confConvToIoRedir(ebcl_IoRedir_t *ior, const char *confVal);

/**
 * Parses a single ENV_SET directive and sets the variable in question accordingly.
 *
 * For details on the syntax, see the relevant section in README.md.
 *
 * @param es       The environment set to be modified, must be initialized.
 * @param confVal  The ENV_SET directive to be parsed and "executed" on the set.
 *
 * @return  0 on success, -1 otherwise
 */
int EBCL_confConvToEnvSetMember(ebcl_EnvSet_t *es, const char *confVal);

int EBCL_confConvToIntegerI(int *x, const char *confVal, int base);
int EBCL_confConvToIntegerULL(unsigned long long *x, const char *confVal, int base);
#define EBCL_confConvToInteger(out, confVal, base)         \
    _Generic((*(out)), int                                 \
             : EBCL_confConvToIntegerI, unsigned long long \
             : EBCL_confConvToIntegerULL)(out, confVal, base)

int EBCL_confConvToBool(bool *b, const char *confVal);

