/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/util/assert_helpers.h"
#include "libs/util/common.h"
#ifdef __cplusplus
#include "libs/util/gfx_util/gfx_util_wildcard.h"
#endif
#include <stdint.h>
#include <stdbool.h>

EXTERN_C_BEGIN

struct desc_map_entry
{
   const char *desc;
   uint32_t value;
};

struct desc_map
{
   size_t num_entries;
   const struct desc_map_entry *entries_value_ordered;
   /* A prefix tree would be a better data structure for desc --> value
    * lookups, but this is simpler and lookup performance isn't particularly
    * important */
   const struct desc_map_entry *entries_desc_ordered;
};

/** value --> desc */

/* Find the entry in map with the smallest offset from value (entry.value -
 * value), only considering entries with entry.value >= value. Set *offset to
 * the offset and return entry.desc.
 *
 * If there is no entry with entry.value >= value, just return NULL. */
extern const char *desc_map_try_value_to_desc_and_offset(
   uint32_t *offset,
   const struct desc_map *map, uint32_t value);

/* If value is in map, the corresponding desc is returned. Otherwise, NULL is
 * returned. */
static inline const char *desc_map_try_value_to_desc(
   const struct desc_map *map, uint32_t value)
{
   uint32_t offset;
   const char *desc = desc_map_try_value_to_desc_and_offset(&offset, map, value);
   return (desc && (offset == 0)) ? desc : NULL;
}

/* value must be in map. The corresponding desc is returned. */
static inline const char *desc_map_value_to_desc(
   const struct desc_map *map, uint32_t value)
{
   const char *desc = desc_map_try_value_to_desc(map, value);
   assert(desc);
   return desc;
}

/** desc --> value */

/* Find the entry in map with the longest desc, considering only entries where
 * entry.desc is a prefix of desc. Set *value to entry.value and return desc +
 * strlen(entry.desc).
 *
 * If there is no such entry, just return desc. */
extern const char *desc_map_past_longest_prefix(
   uint32_t *value,
   const struct desc_map *map, const char *desc);

/* If desc is in map, true is returned and *value is set to the corresponding
 * value. Otherwise, false is returned. */
extern bool desc_map_try_desc_to_value(
   uint32_t *value,
   const struct desc_map *map, const char *desc);

/* desc must be in map. The corresponding value is returned. */
static inline uint32_t desc_map_desc_to_value(
   const struct desc_map *map, const char *desc)
{
   uint32_t value;
   verif(desc_map_try_desc_to_value(&value, map, desc));
   return value;
}

EXTERN_C_END

#ifdef __cplusplus

/* Call f(e) for each entry e in map where gfx_wildcard_pattern_matches(pattern, e.desc) */
template<class F>
static inline void desc_map_for_each_matching(
   const desc_map &map, const char *pattern, F f)
{
   // Brainless implementation...
   for (size_t i = 0; i != map.num_entries; ++i)
   {
      const desc_map_entry &e = map.entries_value_ordered[i];
      if (gfx_wildcard_pattern_matches(pattern, e.desc))
         f(e);
   }
}

#endif
