/**
 * @file utest-cfgmap-order.h
 * @brief Header declaring the regression test for the order and completeness of crinit{Task,Series}CfgMap.
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
 * Regression test for the crinit{Task,Series}CfgMap constant arrays.
 *
 * Checks that the arrays are alphabetically ordered (by its crinitConfigMapping_t::configKey field) and that both
 * together contain all crinitConfigs_t.
 */
void crinitCfgMapRegressionTest(void **state);

#endif /* __UTEST_CFGMAP_ORDER__ */
