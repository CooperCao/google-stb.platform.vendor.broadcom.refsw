/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_map.h"
#include "khrn_int_util.h"

#include <stdlib.h>

#define DELETED_VALUE ((void *)~(uintptr_t)0)

static inline bool is_inited(const khrn_map *map)
{
   return !!map->storage;
}

static inline bool alloc_storage(khrn_map *map, uint32_t capacity)
{
   map->storage = malloc(capacity * sizeof(struct khrn_map_entry));
   return is_inited(map);
}

static inline void free_storage(khrn_map *map)
{
   free(map->storage);
   map->storage = NULL;
}

static inline bool is_deleted(void *value)
{
   return value == DELETED_VALUE;
}

static inline uint32_t hash(uint32_t key, uint32_t capacity)
{
   return key & (capacity - 1);
}

static struct khrn_map_entry *get_entry(const khrn_map *map, uint32_t key)
{
   uint32_t h = hash(key, map->capacity);
   while (map->storage[h].value)
   {
      if (map->storage[h].key == key)
         return is_deleted(map->storage[h].value) ? NULL : (map->storage + h);
      if (++h == map->capacity)
         h = 0;
   }
   return NULL;
}

static struct khrn_map_entry *get_free_entry(const khrn_map *map, uint32_t key)
{
   uint32_t h = hash(key, map->capacity);
   while (map->storage[h].value && !is_deleted(map->storage[h].value))
      if (++h == map->capacity)
         h = 0;
   return map->storage + h;
}

static void raw_insert(khrn_map *map, uint32_t key, void *value)
{
   struct khrn_map_entry *entry = get_free_entry(map, key);
   if (is_deleted(entry->value))
   {
      assert(map->deletes > 0);
      --map->deletes;
   }
   entry->key = key;
   entry->value = value;
   ++map->entries;
}

static bool realloc_storage(khrn_map *map, uint32_t new_capacity)
{
   khrn_map new_map;
   if (!khrn_map_init(&new_map, new_capacity))
      return false;

   for (uint32_t i = 0; i != map->capacity; ++i)
      if (map->storage[i].value && !is_deleted(map->storage[i].value))
         raw_insert(&new_map, map->storage[i].key, map->storage[i].value);

   free_storage(map);
   *map = new_map;

   return true;
}

static bool insert_not_present(khrn_map *map, uint32_t key, void *value)
{
   assert(value);
   assert(!is_deleted(value));

   if (map->entries > (map->capacity / 2))
   {
      if (!realloc_storage(map, map->capacity * 2))
         return false;
   }
   else if ((map->entries + map->deletes) > ((3 * map->capacity) / 4))
   {
      if (!realloc_storage(map, map->capacity))
         return false;
   }

   khrn_mem_acquire(value);
   raw_insert(map, key, value);

   return true;
}

/** Interface */

bool khrn_map_init(khrn_map *map, uint32_t capacity)
{
   /* To ensure we always have at least 1 unused slot, we need:
    * (capacity - 1) > (capacity / 2) and (capacity - 1) > ((3 * capacity) / 4)
    *
    * The smallest number that satisfies these constraints is 8 (7 > 4, 7 > 6) */
   assert(capacity >= 8);

   /* Hash stuff assumes this */
   assert(gfx_is_power_of_2(capacity));

   if (!alloc_storage(map, capacity))
      return false;

   for (uint32_t i = 0; i != capacity; ++i)
      map->storage[i].value = NULL;

   map->entries = 0;
   map->deletes = 0;
   map->capacity = capacity;

   return true;
}

void khrn_map_term(khrn_map *map)
{
   if (is_inited(map))
   {
      for (uint32_t i = 0; i != map->capacity; ++i)
         if (map->storage[i].value && !is_deleted(map->storage[i].value))
            khrn_mem_release(map->storage[i].value);
      free_storage(map);
   }
}

bool khrn_map_insert(khrn_map *map, uint32_t key, void *value)
{
   assert(value);
   assert(!is_deleted(value));

   struct khrn_map_entry *entry = get_entry(map, key);
   if (entry)
   {
      khrn_mem_acquire(value);
      khrn_mem_release(entry->value);
      entry->value = value;
   }
   else
   {
      if (!insert_not_present(map, key, value))
         return false;
   }

   return true;
}

bool khrn_map_delete(khrn_map *map, uint32_t key)
{
   if (!is_inited(map))
      return false;

   struct khrn_map_entry *entry = get_entry(map, key);
   if (entry)
   {
      khrn_mem_release(entry->value);
      entry->value = DELETED_VALUE;
      ++map->deletes;
      assert(map->entries > 0);
      --map->entries;
   }
   return !!entry;
}

void *khrn_map_lookup(const khrn_map *map, uint32_t key)
{
   struct khrn_map_entry *entry = get_entry(map, key);
   return entry ? entry->value : NULL;
}

void khrn_map_iterate(khrn_map *map, khrn_map_callback_t func, void *data)
{
   for (uint32_t i = 0; i != map->capacity; ++i)
      if (map->storage[i].value && !is_deleted(map->storage[i].value))
         func(map, map->storage[i].key, map->storage[i].value, data);
}
