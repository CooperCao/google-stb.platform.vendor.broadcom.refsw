/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "desc_map.h"

#include <string.h>

/* Return the first entry in entries satisfying pred. entries should be ordered
 * such that all entries after this also satisfy pred.
 *
 * If there is no entry that satisfies pred, entries + num_entries is returned. */
static const struct desc_map_entry *first_satisfying(
   const struct desc_map_entry *entries, size_t num_entries,
   bool (*pred)(const struct desc_map_entry *entry, void *p), void *p)
{
   const struct desc_map_entry *begin = entries, *end = entries + num_entries;
   while (begin != end)
   {
      const struct desc_map_entry *mid = begin + ((end - begin) / 2);
      if (pred(mid, p))
         end = mid;
      else
         begin = mid + 1;
   }
   return end;
}

static bool value_ge(const struct desc_map_entry *entry, void *p)
{
   const uint32_t *value = p;
   return entry->value >= *value;
}

const char *desc_map_try_value_to_desc_and_offset(
   uint32_t *offset,
   const struct desc_map *map, uint32_t value)
{
   const struct desc_map_entry *entry = first_satisfying(
      map->entries_value_ordered, map->num_entries, value_ge, &value);
   if (entry == (map->entries_value_ordered + map->num_entries))
      return NULL;
   *offset = entry->value - value;
   return entry->desc;
}

static bool desc_gt(const struct desc_map_entry *entry, void *p)
{
   const char *desc = p;
   return strcmp(entry->desc, desc) > 0;
}

/* Is a a prefix of b? */
static bool is_prefix(const char *a, const char *b)
{
   for (;;)
   {
      if (!*a)
         return true;
      if (*a != *b)
         return false;
      ++a;
      ++b;
   }
}

const char *desc_map_past_longest_prefix(
   uint32_t *value,
   const struct desc_map *map, const char *desc)
{
   /* This is a bit rubbish. Would be better if we had a desc->value prefix
    * tree. */
   const struct desc_map_entry *entry = first_satisfying(
      map->entries_desc_ordered, map->num_entries, desc_gt, (void *)desc);
   while (entry != map->entries_desc_ordered)
   {
      --entry;
      if (*entry->desc != *desc)
         break;
      if (is_prefix(entry->desc, desc))
      {
         *value = entry->value;
         return desc + strlen(entry->desc);
      }
   }
   return desc;
}

static bool desc_ge(const struct desc_map_entry *entry, void *p)
{
   const char *desc = p;
   return strcmp(entry->desc, desc) >= 0;
}

bool desc_map_try_desc_to_value(
   uint32_t *value,
   const struct desc_map *map, const char *desc)
{
   const struct desc_map_entry *entry = first_satisfying(
      map->entries_desc_ordered, map->num_entries, desc_ge, (void *)desc);
   if ((entry == (map->entries_desc_ordered + map->num_entries)) || strcmp(entry->desc, desc))
      return false;
   *value = entry->value;
   return true;
}
