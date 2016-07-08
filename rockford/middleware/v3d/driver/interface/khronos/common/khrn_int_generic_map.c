/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Generic map

FILE DESCRIPTION
KHRN_GENERIC_MAP_KEY_T to KHRN_GENERIC_MAP_VALUE_T map. In
KHRN_GENERIC_MAP_RELOCATABLE mode, KHRN_GENERIC_MAP(T) is compatible both with
the c++-style (init/term correspond to constructor/destructor) object semantics
used by vg and with the object semantics used by gl (structure is initialised to
-1 before any functions are called, term can be called at any time -- before
init, after init even if it fails, and after term).
=============================================================================*/

#include "interface/khronos/common/khrn_int_generic_map.h"
#include "interface/khronos/common/khrn_int_util.h"

static INLINE uint32_t hash(KHRN_GENERIC_MAP_KEY_T key, uint32_t capacity)
{
   return (uint32_t)key & (capacity - 1);
}

static KHRN_GENERIC_MAP(ENTRY_T) *get_entry(KHRN_GENERIC_MAP(ENTRY_T) *base, uint32_t capacity, KHRN_GENERIC_MAP_KEY_T key)
{
   uint32_t h = hash(key, capacity);
   while (base[h].value != KHRN_GENERIC_MAP_VALUE_NONE) {
      if (base[h].key == key) {
         return (base[h].value == KHRN_GENERIC_MAP_VALUE_DELETED) ? NULL : (base + h);
      }
      if (++h == capacity) {
         h = 0;
      }
   }
   return NULL;
}

static KHRN_GENERIC_MAP(ENTRY_T) *get_free_entry(KHRN_GENERIC_MAP(ENTRY_T) *base, uint32_t capacity, KHRN_GENERIC_MAP_KEY_T key)
{
   uint32_t h = hash(key, capacity);
   while ((base[h].value != KHRN_GENERIC_MAP_VALUE_DELETED) && (base[h].value != KHRN_GENERIC_MAP_VALUE_NONE)) {
      if (++h == capacity) {
         h = 0;
      }
   }
   return base + h;
}

static bool realloc_storage(KHRN_GENERIC_MAP(T) *map, uint32_t new_capacity)
{
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
   MEM_HANDLE_T handle = map->storage;
   KHRN_GENERIC_MAP(ENTRY_T) *base;
#else
   KHRN_GENERIC_MAP(ENTRY_T) *base = map->storage;
#endif
   uint32_t capacity = map->capacity;
   uint32_t i;

   /*
      new map
   */

   if (!khrn_generic_map(init)(map, new_capacity)) {
      /* khrn_generic_map(init) fills in struct only once it is sure to succeed,
       * so if we get here struct will be unmodified */
      return false;
   }

   /*
      copy entries across to new map and destroy old map
   */

#ifdef KHRN_GENERIC_MAP_RELOCATABLE
   base = (KHRN_GENERIC_MAP(ENTRY_T) *)mem_lock(handle, NULL);
#endif
   for (i = 0; i != capacity; ++i) {
      if ((base[i].value != KHRN_GENERIC_MAP_VALUE_DELETED) && (base[i].value != KHRN_GENERIC_MAP_VALUE_NONE)) {
         verify(khrn_generic_map(insert)(map, base[i].key, base[i].value)); /* khrn_generic_map(insert) can only fail if the map is too small */
#ifdef KHRN_GENERIC_MAP_RELEASE_VALUE
         KHRN_GENERIC_MAP_RELEASE_VALUE(base[i].value); /* new reference added by khrn_generic_map(insert) */
#endif
      }
   }
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
   mem_unlock(handle);
   mem_release(handle);
#else
   KHRN_GENERIC_MAP_FREE(base);
#endif

   return true;
}

bool khrn_generic_map(init)(KHRN_GENERIC_MAP(T) *map, uint32_t capacity)
{
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
   MEM_HANDLE_T handle;
#else
   KHRN_GENERIC_MAP(ENTRY_T) *base;
   uint32_t i;
#endif

   /*
      we need (capacity - 1) > (capacity / 2) and (capacity - 1) > ((3 * capacity) / 4)
      to ensure we always have at least 1 unused slot

      the smallest number that satisfies these constraints is 8 (7 > 4, 7 > 6)
   */

   vcos_assert(capacity >= 8);
   vcos_assert(is_power_of_2(capacity)); /* hash stuff assumes this */

   /*
      alloc and clear storage
   */

   #define STRINGIZE2(X) #X
   #define STRINGIZE(X) STRINGIZE2(X) /* X will be expanded here */
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
   handle = mem_alloc_ex(capacity * sizeof(KHRN_GENERIC_MAP(ENTRY_T)), alignof(KHRN_GENERIC_MAP(ENTRY_T)),
      MEM_FLAG_INIT, STRINGIZE(KHRN_GENERIC_MAP(T)) ".storage", MEM_COMPACT_DISCARD); /* no term (struct containing KHRN_GENERIC_MAP(T) must call khrn_generic_map(term)()) */
   if (handle == MEM_INVALID_HANDLE) {
      return false;
   }
   { vcos_static_assert(MEM_INVALID_HANDLE == KHRN_GENERIC_MAP_VALUE_NONE); }
   /* all values already initialised to KHRN_GENERIC_MAP_VALUE_NONE */
#else
   base = (KHRN_GENERIC_MAP(ENTRY_T) *)KHRN_GENERIC_MAP_ALLOC(capacity * sizeof(KHRN_GENERIC_MAP(ENTRY_T)),
      STRINGIZE(KHRN_GENERIC_MAP(T)) ".storage");
   if (!base) {
      return false;
   }
   for (i = 0; i != capacity; ++i) {
      base[i].value = KHRN_GENERIC_MAP_VALUE_NONE;
   }
#endif
   #undef STRINGIZE
   #undef STRINGIZE2

   /*
      fill in struct (do this only once we are sure to succeed --
      realloc_storage and khrn_generic_map(term) under gl object semantics rely
      on this behaviour)
   */

   map->entries = 0;
   map->deletes = 0;
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
   map->storage = handle;
#else
   map->storage = base;
#endif
   map->capacity = capacity;

   return true;
}

/*
   in KHRN_GENERIC_MAP_RELOCATABLE mode, khrn_generic_map(term) may be called:
   - before init: map->storage will be MEM_INVALID_HANDLE.
   - after init fails: map is unchanged.
   - after term: map->storage will have been set back to MEM_INVALID_HANDLE.
*/

void khrn_generic_map(term)(KHRN_GENERIC_MAP(T) *map)
{
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
   if (map->storage != MEM_INVALID_HANDLE) {
#endif
#ifdef KHRN_GENERIC_MAP_RELEASE_VALUE
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
      KHRN_GENERIC_MAP(ENTRY_T) *base = (KHRN_GENERIC_MAP(ENTRY_T) *)mem_lock(map->storage, NULL);
#else
      KHRN_GENERIC_MAP(ENTRY_T) *base = map->storage;
#endif
      uint32_t i;
      for (i = 0; i != map->capacity; ++i) {
         if ((base[i].value != KHRN_GENERIC_MAP_VALUE_DELETED) && (base[i].value != KHRN_GENERIC_MAP_VALUE_NONE)) {
            KHRN_GENERIC_MAP_RELEASE_VALUE(base[i].value);
         }
      }
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
      mem_unlock(map->storage);
#endif
#endif
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
      mem_release(map->storage);
      map->storage = MEM_INVALID_HANDLE;
   }
#else
   KHRN_GENERIC_MAP_FREE(map->storage);
#endif
}

bool khrn_generic_map(insert)(KHRN_GENERIC_MAP(T) *map, KHRN_GENERIC_MAP_KEY_T key, KHRN_GENERIC_MAP_VALUE_T value)
{
   uint32_t capacity = map->capacity;
   KHRN_GENERIC_MAP(ENTRY_T) *entry;

   vcos_assert(value != KHRN_GENERIC_MAP_VALUE_DELETED);
   vcos_assert(value != KHRN_GENERIC_MAP_VALUE_NONE);

   entry = get_entry(
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
      (KHRN_GENERIC_MAP(ENTRY_T) *)mem_lock(map->storage, NULL),
#else
      map->storage,
#endif
      capacity, key);
   if (entry) {
#ifdef KHRN_GENERIC_MAP_ACQUIRE_VALUE
      KHRN_GENERIC_MAP_ACQUIRE_VALUE(value);
#endif
#ifdef KHRN_GENERIC_MAP_RELEASE_VALUE
      KHRN_GENERIC_MAP_RELEASE_VALUE(entry->value);
#endif
      entry->value = value;
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
      mem_unlock(map->storage);
#endif
   } else {
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
      mem_unlock(map->storage);
#endif

      if (map->entries > (capacity / 2)) {
         capacity *= 2;
         if (!realloc_storage(map, capacity)) { return false; }
      } else if ((map->entries + map->deletes) > ((3 * capacity) / 4)) {
         if (!realloc_storage(map, capacity)) { return false; }
      }

#ifdef KHRN_GENERIC_MAP_ACQUIRE_VALUE
      KHRN_GENERIC_MAP_ACQUIRE_VALUE(value);
#endif
      entry = get_free_entry(
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
         (KHRN_GENERIC_MAP(ENTRY_T) *)mem_lock(map->storage, NULL),
#else
         map->storage,
#endif
         capacity, key);
      if (entry->value == KHRN_GENERIC_MAP_VALUE_DELETED) {
         vcos_assert(map->deletes > 0);
         --map->deletes;
      }
      entry->key = key;
      entry->value = value;
      ++map->entries;
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
      mem_unlock(map->storage);
#endif
   }

   return true;
}

bool khrn_generic_map(delete)(KHRN_GENERIC_MAP(T) *map, KHRN_GENERIC_MAP_KEY_T key)
{
   KHRN_GENERIC_MAP(ENTRY_T) *entry = get_entry(
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
      (KHRN_GENERIC_MAP(ENTRY_T) *)mem_lock(map->storage, NULL),
#else
      map->storage,
#endif
      map->capacity, key);
   if (entry) {
#ifdef KHRN_GENERIC_MAP_RELEASE_VALUE
      KHRN_GENERIC_MAP_RELEASE_VALUE(entry->value);
#endif
      entry->value = KHRN_GENERIC_MAP_VALUE_DELETED;
      ++map->deletes;
      vcos_assert(map->entries > 0);
      --map->entries;
   }
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
   mem_unlock(map->storage);
#endif
   return !!entry;
}

uint32_t khrn_generic_map(get_count)(KHRN_GENERIC_MAP(T) *map)
{
   return map->entries;
}

KHRN_GENERIC_MAP_VALUE_T khrn_generic_map(lookup)(KHRN_GENERIC_MAP(T) *map, KHRN_GENERIC_MAP_KEY_T key)
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
{
   KHRN_GENERIC_MAP_VALUE_T value = khrn_generic_map(lookup_locked)(map, key, mem_lock(map->storage, NULL));
   mem_unlock(map->storage);
   return value;
}

KHRN_GENERIC_MAP_VALUE_T khrn_generic_map(lookup_locked)(KHRN_GENERIC_MAP(T) *map, KHRN_GENERIC_MAP_KEY_T key, void *storage)
#endif
{
   KHRN_GENERIC_MAP(ENTRY_T) *entry = get_entry(
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
      (KHRN_GENERIC_MAP(ENTRY_T) *)storage,
#else
      map->storage,
#endif
      map->capacity, key);
   return entry ? entry->value : KHRN_GENERIC_MAP_VALUE_NONE;
}

void khrn_generic_map(iterate)(KHRN_GENERIC_MAP(T) *map, KHRN_GENERIC_MAP(CALLBACK_T) func, void *data)
{
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
   KHRN_GENERIC_MAP(ENTRY_T) *base = (KHRN_GENERIC_MAP(ENTRY_T) *)mem_lock(map->storage, NULL);
#else
   KHRN_GENERIC_MAP(ENTRY_T) *base = map->storage;
#endif
   uint32_t i;
   for (i = 0; i != map->capacity; ++i) {
      if ((base[i].value != KHRN_GENERIC_MAP_VALUE_DELETED) && (base[i].value != KHRN_GENERIC_MAP_VALUE_NONE)) {
         func(map, base[i].key, base[i].value, data);
      }
   }
#ifdef KHRN_GENERIC_MAP_RELOCATABLE
   mem_unlock(map->storage);
#endif
}
