/**
 * @file common.h
 * @brief Header for common definitions not related to other specific features.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2021-2022 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __COMMON_H__
#define __COMMON_H__

/**
 * Suppress unused parameter warning for variable as per coding guideline `[OS_C_UNUSED_010]`.
 *
 * May be needed if an external interface is implemented.
 *
 * @param par  Unused variable that should not be warned about.
 */
#define EBCL_PARAM_UNUSED(par) \
    do {                       \
        (void)(par);           \
    } while (0)

#endif /* __COMMON_H__ */
