/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdint.h>
#include <stdbool.h>

struct khrn_map_entry
{
   uint32_t key;
   void *value;
};

typedef struct khrn_map
{
   uint32_t entries;
   uint32_t deletes;

   struct khrn_map_entry *storage;
   uint32_t capacity;
} khrn_map;

/* capacity must be >= 8. false returned on failure. */
extern bool khrn_map_init(khrn_map *map, uint32_t capacity);

/* It's fine to call this on a zero-initialised map, even after a failed call
 * to khrn_map_init(). */
extern void khrn_map_term(khrn_map *map);

/* Inserts value into map with key key. The map will hold a reference to value
 * (it must have been allocated with khrn_mem). If another value is already in
 * the map with this key, the function will not fail; the new value replaces
 * the old. On failure, false is returned and the map is unchanged. */
extern bool khrn_map_insert(khrn_map *map, uint32_t key, void *value);

/* If present, deletes the element identified by key from the map and returns
 * true. If not present, returns false. */
extern bool khrn_map_delete(khrn_map *map, uint32_t key);

static inline uint32_t khrn_map_get_count(const khrn_map *map)
{
   return map->entries;
}

/* Returns the value corresponding to key, or NULL if key is not in the map.
 * value's reference count *is not* incremented. */
extern void *khrn_map_lookup(const khrn_map *map, uint32_t key);

/* Runs the given callback function once for every (key, value) pair in the
 * map. The iterator function is allowed to delete the element it is given, but
 * not modify the structure of map in any other way (eg by adding new
 * elements). */
typedef void (*khrn_map_callback_t)(khrn_map *map, uint32_t key, void *value, void *p);
extern void khrn_map_iterate(khrn_map *map, khrn_map_callback_t func, void *p);
