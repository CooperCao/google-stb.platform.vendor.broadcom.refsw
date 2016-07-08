/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "khrn_mem.h"
#include <stdbool.h>

struct khrn_map_entry
{
   uint32_t key;
   KHRN_MEM_HANDLE_T value;
};

typedef struct khrn_map
{
   uint32_t entries;
   uint32_t deletes;

   struct khrn_map_entry *storage;
   uint32_t capacity;
} KHRN_MAP_T;

/* capacity must be >= 8. false returned on failure. */
extern bool khrn_map_init(KHRN_MAP_T *map, uint32_t capacity);

/* It's fine to call this on a zero-initialised map, even after a failed call
 * to khrn_map_init(). */
extern void khrn_map_term(KHRN_MAP_T *map);

/* Inserts value into map with key key. The map will hold a reference to value.
 * If another value is already in the map with this key, the function will not
 * fail; the new value replaces the old. On failure, false is returned and the
 * map is unchanged. */
extern bool khrn_map_insert(KHRN_MAP_T *map, uint32_t key, KHRN_MEM_HANDLE_T value);

/* If present, deletes the element identified by key from the map and returns
 * true. If not present, returns false. */
extern bool khrn_map_delete(KHRN_MAP_T *map, uint32_t key);

static inline uint32_t khrn_map_get_count(const KHRN_MAP_T *map)
{
   return map->entries;
}

/* Returns the value corresponding to key, or NULL if key is not in the map.
 * value's reference count *is not* incremented. */
extern KHRN_MEM_HANDLE_T khrn_map_lookup(const KHRN_MAP_T *map, uint32_t key);

/* Runs the given callback function once for every (key, value) pair in the
 * map. The iterator function is allowed to delete the element it is given, but
 * not modify the structure of map in any other way (eg by adding new
 * elements). */
typedef void (*KHRN_MAP_CALLBACK_T)(KHRN_MAP_T *map, uint32_t key, KHRN_MEM_HANDLE_T value, void *p);
extern void khrn_map_iterate(KHRN_MAP_T *map, KHRN_MAP_CALLBACK_T func, void *p);
