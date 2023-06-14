/**
 * @file confmap.h
 * @brief Definitions related to mapping configuration options and their string representations.
 *
 * @author emlix GmbH, 37083 Göttingen, Germany
 *
 * @copyright 2023 Elektrobit Automotive GmbH
 *            All rights exclusively reserved for Elektrobit Automotive GmbH,
 *            unless otherwise expressly agreed
 */
#ifndef __CONFMAP_H__
#define __CONFMAP_H__

#include "confhdl.h"

/**
 * A structure defining a mapping between a configuration option, its string respresentation, and further properties.
 */
typedef struct ebcl_ConfigMapping_t {
    crinitConfigs_t config;            ///< The index of the configuration option.
    const char *configKey;            ///< The string representation (name) of the option.
    bool arrayLike;                   ///< If the configuration option is array-like, i.e. may be defined multiple
                                      ///< times and appended to.
    bool includeSafe;                 ///< If the configuration option may be used inside an include file.
    ebcl_ConfigHandler_t cfgHandler;  ///< Pointer to the handler function to use if the configuration
                                      ///< option is encountered.
} ebcl_ConfigMapping_t;

/**
 * Constant (at compile-time) array of mappings between configuration names and their indices and properties.
 *
 * Must be lexicographically ordered (by ebcl_ConfigMapping_t::configKey), so that EBCL_findConfigMapping() works. This
 * is tested by a unit/regression test.
 */
extern const ebcl_ConfigMapping_t EBCL_cfgMap[];
/**
 * Size of EBCL_cfgMap, known at compile-time.
 */
extern const size_t EBCL_cfgMapSize;

/**
 * Searches for an entry in EBCL_cfgMap by ebcl_ConfigMapping_t::configKey.
 *
 * Uses bsearch() with the assumption that EBCL_cfgMap is lexicographically ordered by ebcl_ConfigMapping_t::configKey.
 *
 * @param keyStr  The name of the mapping to search for.
 */
const ebcl_ConfigMapping_t *EBCL_findConfigMapping(const char *keyStr);

#endif /* __CONFMAP_H__ */
