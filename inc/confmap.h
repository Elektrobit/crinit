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
typedef struct crinitConfigMapping_t {
    crinitConfigs_t config;            ///< The index of the configuration option.
    const char *configKey;             ///< The string representation (name) of the option.
    bool arrayLike;                    ///< If the configuration option is array-like, i.e. may be defined multiple
                                       ///< times and appended to.
    bool includeSafe;                  ///< If the configuration option may be used inside an include file.
    crinitConfigHandler_t cfgHandler;  ///< Pointer to the handler function to use if the configuration
                                       ///< option is encountered.
} crinitConfigMapping_t;

/**
 * Constant (at compile-time) array of mappings between configuration names and their indices and properties.
 *
 * Must be lexicographically ordered (by crinitConfigMapping_t::configKey), so that crinitFindConfigMapping() works.
 * This is tested by a unit/regression test.
 */
extern const crinitConfigMapping_t crinitTaskCfgMap[];
/**
 * Size of crinitTaskCfgMap, known at compile-time.
 */
extern const size_t crinitTaskCfgMapSize;

/**
 * Constant (at compile-time) array of mappings between configuration names and their indices and properties.
 *
 * Must be lexicographically ordered (by crinitConfigMapping_t::configKey), so that crinitFindConfigMapping() works.
 * This is tested by a unit/regression test.
 */
extern const crinitConfigMapping_t crinitSeriesCfgMap[];
/**
 * Size of crinitSeriesCfgMap, known at compile-time.
 */
extern const size_t crinitSeriesCfgMapSize;

/**
 * Searches for an entry in crinitTaskCfgMap by crinitConfigMapping_t::configKey.
 *
 * Uses bsearch() with the assumption that crinitTaskCfgMap is lexicographically ordered by
 * crinitConfigMapping_t::configKey.
 *
 * @param keyStr  The name of the mapping to search for.
 */
const crinitConfigMapping_t *crinitFindConfigMapping(const crinitConfigMapping_t *map, size_t mapSize,
                                                     const char *keyStr);

#endif /* __CONFMAP_H__ */
