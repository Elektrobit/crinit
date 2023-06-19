/**
 * @file utest-cfgmap-order.h
 * @brief Header declaring the regression test for the order/size of crinitCfgMap.
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
 * Regression test for the crinitCfgMap constant array.
 *
 * Checks that the array is alphabetically ordered (by its crinitConfigMapping_t::configKey field) and that it contains
 * CRINIT_CONFIGS_SIZE number of elements.
 */
void crinitCfgMapRegressionTest(void **state);

#endif /* __UTEST_CFGMAP_ORDER__ */
