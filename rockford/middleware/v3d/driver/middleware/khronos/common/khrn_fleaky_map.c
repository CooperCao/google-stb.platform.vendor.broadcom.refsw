/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Fixed-size leaky map

FILE DESCRIPTION
Implementation.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#include "middleware/khronos/common/khrn_fleaky_map.h"
#include "interface/khronos/common/khrn_int_util.h"

void khrn_fleaky_map_init(KHRN_FLEAKY_MAP_T *fleaky_map, KHRN_FLEAKY_MAP_ENTRY_T *storage, uint32_t capacity)
{
   uint32_t i;

   vcos_assert(capacity >= 4);
   vcos_assert(is_power_of_2(capacity));

   for (i = 0; i != capacity; ++i) {
      storage[i].value = MEM_INVALID_HANDLE;
   }

   fleaky_map->entries = 0;
   fleaky_map->next_victim = 0;
   fleaky_map->storage = storage;
   fleaky_map->capacity = capacity;
}

void khrn_fleaky_map_term(KHRN_FLEAKY_MAP_T *fleaky_map)
{
   uint32_t i;
   for (i = 0; i != fleaky_map->capacity; ++i) {
      if (fleaky_map->storage[i].value != MEM_INVALID_HANDLE) {
         mem_release(fleaky_map->storage[i].value);
      }
   }
}

static INLINE uint32_t hash(uint32_t key, uint32_t capacity)
{
   return key & (capacity - 1);
}

static void delete_entry(KHRN_FLEAKY_MAP_T *fleaky_map, uint32_t i)
{
   uint32_t j;

   mem_release(fleaky_map->storage[i].value);
   fleaky_map->storage[i].value = MEM_INVALID_HANDLE;
   --fleaky_map->entries;

   for (j = (i + 1) & (fleaky_map->capacity - 1); fleaky_map->storage[j].value != MEM_INVALID_HANDLE; j = (j + 1) & (fleaky_map->capacity - 1)) {
      uint32_t h = hash(fleaky_map->storage[j].key, fleaky_map->capacity);
      if ((j > i) ? ((h <= i) || (h > j)) : ((h > j) && (h <= i))) {
         fleaky_map->storage[i] = fleaky_map->storage[j];
         fleaky_map->storage[j].value = MEM_INVALID_HANDLE;
         i = j;
      }
   }
}

void khrn_fleaky_map_insert(KHRN_FLEAKY_MAP_T *fleaky_map, uint32_t key, MEM_HANDLE_T value)
{
   uint32_t i;

   vcos_assert(khrn_fleaky_map_lookup(fleaky_map, key) == MEM_INVALID_HANDLE);

   if (fleaky_map->entries > (fleaky_map->capacity >> 1)) {
      uint32_t i;
      for (i = fleaky_map->next_victim; fleaky_map->storage[i].value == MEM_INVALID_HANDLE; i = (i + 1) & (fleaky_map->capacity - 1)) ;
      delete_entry(fleaky_map, i);
      fleaky_map->next_victim = (i + 1) & (fleaky_map->capacity - 1);
   }

   for (i = hash(key, fleaky_map->capacity); fleaky_map->storage[i].value != MEM_INVALID_HANDLE; i = (i + 1) & (fleaky_map->capacity - 1)) ;
   fleaky_map->storage[i].key = key;
   mem_acquire(value);
   fleaky_map->storage[i].value = value;
   ++fleaky_map->entries;
}

MEM_HANDLE_T khrn_fleaky_map_lookup(KHRN_FLEAKY_MAP_T *fleaky_map, uint32_t key)
{
   uint32_t i;
   for (i = hash(key, fleaky_map->capacity); fleaky_map->storage[i].value != MEM_INVALID_HANDLE; i = (i + 1) & (fleaky_map->capacity - 1)) {
      if (fleaky_map->storage[i].key == key) {
         return fleaky_map->storage[i].value;
      }
   }
   return MEM_INVALID_HANDLE;
}
