/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Generic map

FILE DESCRIPTION
KHRN_GENERIC_MAP_KEY_T to KHRN_GENERIC_MAP_VALUE_T map.
=============================================================================*/

#include <stdlib.h>

#include "interface/khronos/common/khrn_int_generic_map.h"
#include "interface/khronos/common/khrn_int_util.h"

/** ifdef stuff */

static inline uint32_t hash_key(KHRN_GENERIC_MAP_KEY_T key)
{
#ifdef KHRN_GENERIC_MAP_HASH_KEY
   return KHRN_GENERIC_MAP_HASH_KEY(key);
#else
   return (uint32_t)key;
#endif
}

static inline bool cmp_key(KHRN_GENERIC_MAP_KEY_T a, KHRN_GENERIC_MAP_KEY_T b)
{
#ifdef KHRN_GENERIC_MAP_CMP_KEY
   return KHRN_GENERIC_MAP_CMP_KEY(a, b);
#else
   return a == b;
#endif
}

static inline bool cmp_value(KHRN_GENERIC_MAP_VALUE_T a, KHRN_GENERIC_MAP_VALUE_T b)
{
#ifdef KHRN_GENERIC_MAP_CMP_VALUE
   return KHRN_GENERIC_MAP_CMP_VALUE(a, b);
#else
   return a == b;
#endif
}

static inline void acquire_value(KHRN_GENERIC_MAP_VALUE_T value)
{
#ifdef KHRN_GENERIC_MAP_ACQUIRE_VALUE
   KHRN_GENERIC_MAP_ACQUIRE_VALUE(value);
#endif
}

static inline void release_value(KHRN_GENERIC_MAP_VALUE_T value)
{
#ifdef KHRN_GENERIC_MAP_RELEASE_VALUE
   KHRN_GENERIC_MAP_RELEASE_VALUE(value);
#endif
}

static inline bool is_inited(KHRN_GENERIC_MAP(T) *map)
{
#ifdef KHRN_GENERIC_MAP_STONE_RELOCATABLE
   return map->storage != MEM_HANDLE_INVALID;
#elif defined(KHRN_GENERIC_MAP_RELOCATABLE)
   return map->storage != KHRN_MEM_HANDLE_INVALID;
#else
   return !!map->storage;
#endif
}

static inline bool alloc_storage(KHRN_GENERIC_MAP(T) *map, uint32_t capacity)
{
   uint32_t size = capacity * sizeof(KHRN_GENERIC_MAP(ENTRY_T));
   map->storage = malloc(size);
   return is_inited(map);
}

static inline void free_storage(KHRN_GENERIC_MAP(T) *map)
{
   free(map->storage);
   map->storage = NULL;
}

static inline KHRN_GENERIC_MAP(ENTRY_T) *lock_storage(KHRN_GENERIC_MAP(T) *map)
{
   return map->storage;
}

/** helpers */
static inline bool is_none(KHRN_GENERIC_MAP_VALUE_T value)
{
   return cmp_value(value, KHRN_GENERIC_MAP_VALUE_NONE);
}

static inline bool is_deleted(KHRN_GENERIC_MAP_VALUE_T value)
{
   return cmp_value(value, KHRN_GENERIC_MAP_VALUE_DELETED);
}

static inline uint32_t hash(KHRN_GENERIC_MAP_KEY_T key, uint32_t capacity)
{
   return hash_key(key) & (capacity - 1);
}

static KHRN_GENERIC_MAP(ENTRY_T) *get_entry(KHRN_GENERIC_MAP(ENTRY_T) *base,
   uint32_t capacity, KHRN_GENERIC_MAP_KEY_T key)
{
   uint32_t h = hash(key, capacity);
   while (!is_none(base[h].value)) {
      if (cmp_key(base[h].key, key)) {
         return is_deleted(base[h].value) ? NULL : (base + h);
      }
      if (++h == capacity) {
         h = 0;
      }
   }
   return NULL;
}

static KHRN_GENERIC_MAP(ENTRY_T) *get_free_entry(KHRN_GENERIC_MAP(ENTRY_T) *base,
   uint32_t capacity, KHRN_GENERIC_MAP_KEY_T key)
{
   uint32_t h = hash(key, capacity);
   while (!is_deleted(base[h].value) && !is_none(base[h].value)) {
      if (++h == capacity) {
         h = 0;
      }
   }
   return base + h;
}

static void raw_insert(KHRN_GENERIC_MAP(T) *map,
   KHRN_GENERIC_MAP_KEY_T key, KHRN_GENERIC_MAP_VALUE_T value)
{
   KHRN_GENERIC_MAP(ENTRY_T) *entry = get_free_entry(lock_storage(map), map->capacity, key);
   if (is_deleted(entry->value)) {
      assert(map->deletes > 0);
      --map->deletes;
   }
   entry->key = key;
   entry->value = value;
   ++map->entries;
}

static bool realloc_storage(KHRN_GENERIC_MAP(T) *map, uint32_t new_capacity)
{
   KHRN_GENERIC_MAP(T) new_map;
   KHRN_GENERIC_MAP(ENTRY_T) *base;
   uint32_t i;

   if (!khrn_generic_map(init)(&new_map, new_capacity)) {
      return false;
   }

   base = lock_storage(map);
   for (i = 0; i != map->capacity; ++i) {
      if (!is_deleted(base[i].value) && !is_none(base[i].value)) {
         raw_insert(&new_map, base[i].key, base[i].value);
      }
   }
   free_storage(map);

   *map = new_map;

   return true;
}

/** interface */

bool khrn_generic_map(init)(KHRN_GENERIC_MAP(T) *map, uint32_t capacity)
{
   KHRN_GENERIC_MAP(ENTRY_T) *base;
   uint32_t i;

   /* to ensure we always have at least 1 unused slot, we need:
    * (capacity - 1) > (capacity / 2) and (capacity - 1) > ((3 * capacity) / 4)
    *
    * the smallest number that satisfies these constraints is 8 (7 > 4, 7 > 6) */
   assert(capacity >= 8);

   /* hash stuff assumes this */
   assert(is_power_of_2(capacity));

   if (!alloc_storage(map, capacity)) {
      return false;
   }

   base = lock_storage(map);
   for (i = 0; i != capacity; ++i) {
      base[i].value = KHRN_GENERIC_MAP_VALUE_NONE;
   }

   map->entries = 0;
   map->deletes = 0;
   map->capacity = capacity;

   return true;
}

void khrn_generic_map(term)(KHRN_GENERIC_MAP(T) *map)
{
   if (is_inited(map)) {
#ifdef KHRN_GENERIC_MAP_RELEASE_VALUE
      KHRN_GENERIC_MAP(ENTRY_T) *base = lock_storage(map);
      uint32_t i;
      for (i = 0; i != map->capacity; ++i) {
         if (!is_deleted(base[i].value) && !is_none(base[i].value)) {
            release_value(base[i].value);
         }
      }
#endif
      free_storage(map);
   }
}

bool khrn_generic_map(insert)(KHRN_GENERIC_MAP(T) *map,
   KHRN_GENERIC_MAP_KEY_T key, KHRN_GENERIC_MAP_VALUE_T value)
{
   KHRN_GENERIC_MAP(ENTRY_T) *entry;

   assert(!is_deleted(value));
   assert(!is_none(value));

   entry = get_entry(lock_storage(map), map->capacity, key);
   if (entry) {
      acquire_value(value);
      release_value(entry->value);
      entry->value = value;
   } else {
      if (!khrn_generic_map(insert_not_present)(map, key, value)) {
         return false;
      }
   }

   return true;
}

bool khrn_generic_map(insert_not_present)(KHRN_GENERIC_MAP(T) *map,
   KHRN_GENERIC_MAP_KEY_T key, KHRN_GENERIC_MAP_VALUE_T value)
{
   uint32_t capacity = map->capacity;

   assert(!is_deleted(value));
   assert(!is_none(value));

   if (map->entries > (capacity / 2)) {
      capacity *= 2;
      if (!realloc_storage(map, capacity)) { return false; }
   } else if ((map->entries + map->deletes) > ((3 * capacity) / 4)) {
      if (!realloc_storage(map, capacity)) { return false; }
   }

   acquire_value(value);
   raw_insert(map, key, value);

   return true;
}

bool khrn_generic_map(delete)(KHRN_GENERIC_MAP(T) *map, KHRN_GENERIC_MAP_KEY_T key)
{
   KHRN_GENERIC_MAP(ENTRY_T) *storage = lock_storage(map);
   if (storage)
   {
      KHRN_GENERIC_MAP(ENTRY_T) *entry = get_entry(storage, map->capacity, key);
      if (entry)
      {
         release_value(entry->value);
         entry->value = KHRN_GENERIC_MAP_VALUE_DELETED;
         ++map->deletes;
         assert(map->entries > 0);
         --map->entries;
      }
      return !!entry;
   }
   return !!storage;
}

uint32_t khrn_generic_map(get_count)(KHRN_GENERIC_MAP(T) *map)
{
   return map->entries;
}

KHRN_GENERIC_MAP_VALUE_T khrn_generic_map(lookup)(KHRN_GENERIC_MAP(T) *map,
   KHRN_GENERIC_MAP_KEY_T key)
{
   KHRN_GENERIC_MAP(ENTRY_T) *entry = get_entry(lock_storage(map), map->capacity, key);
   return entry ? entry->value : KHRN_GENERIC_MAP_VALUE_NONE;
}

void khrn_generic_map(iterate)(KHRN_GENERIC_MAP(T) *map,
   KHRN_GENERIC_MAP(CALLBACK_T) func, void *data)
{
   KHRN_GENERIC_MAP(ENTRY_T) *base = lock_storage(map);
   uint32_t i;
   for (i = 0; i != map->capacity; ++i) {
      if (!is_deleted(base[i].value) && !is_none(base[i].value)) {
         func(map, base[i].key, base[i].value, data);
      }
   }
}

void khrn_generic_map(iterate_modify)(KHRN_GENERIC_MAP(T) *map,
   KHRN_GENERIC_MAP(MODIFY_CALLBACK_T) func, void *data)
{
   KHRN_GENERIC_MAP(ENTRY_T) *base = lock_storage(map);
   uint32_t i;
   for (i = 0; i != map->capacity; ++i) {
      if (!is_deleted(base[i].value) && !is_none(base[i].value)) {
         func(map, base + i, data);
      }
   }
}
