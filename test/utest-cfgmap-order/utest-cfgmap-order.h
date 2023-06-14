/**
 * @file utest-cfgmap-order.h
 * @brief Header declaring the regression test for the order/size of EBCL_cfgMap.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __UTEST_CFGMAP_ORDER_H__
#define __UTEST_CFGMAP_ORDER_H__

/**
 * Regression test for the EBCL_cfgMap constant array.
 *
 * Checks that the array is alphabetically ordered (by its ebcl_ConfigMapping_t::configKey field) and that it contains
 * CRINIT_CONFIGS_SIZE number of elements.
 */
void EBCL_cfgMapRegressionTest(void **state);

#endif /* __UTEST_CFGMAP_ORDER__ */
