/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_map.h"
#include "glsl_safemem.h"

#include "libs/util/gfx_util/gfx_util.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define GLSL_MAP_MAX_ENTRIES(NUM_BITS) ((1 << ((NUM_BITS) - 2)) * 3) // 75%

static inline unsigned int glsl_map_hash(const void *k, int num_bits)
{
   assert(k != NULL);
   // the magic constant is prime close to 2^32 / golden ratio
   return ((unsigned int)(uintptr_t)k * 2654435761u) >> (8 * sizeof(unsigned int) - num_bits);
}

static void glsl_map_init(Map *map, int num_bits)
{
   map->count = 0;
   map->num_bits = num_bits;
   map->entries = glsl_safemem_calloc(GLSL_MAP_MAX_ENTRIES(num_bits), sizeof(MapEntry));
   map->buckets = glsl_safemem_calloc(1 << num_bits, sizeof(MapEntry*));
}

static void glsl_map_term(Map *map)
{
   glsl_safemem_free(map->buckets);
   glsl_safemem_free(map->entries);
}

static void glsl_map_grow(Map *map)
{
   Map old = *map;

   glsl_map_init(map, old.num_bits + 2);

   GLSL_MAP_FOREACH(e, &old)
      glsl_map_put(map, e->k, e->v);

   glsl_map_term(&old);
}

Map *glsl_map_new(void)
{
   Map *map = glsl_safemem_malloc(sizeof(Map));
   glsl_map_init(map, 4);
   return map;
}

void glsl_map_delete(Map *map)
{
   glsl_map_term(map);
   glsl_safemem_free(map);
}

void glsl_map_put(Map *map, const void *k, void *v)
{
   if (map->count >= GLSL_MAP_MAX_ENTRIES(map->num_bits))
      glsl_map_grow(map);
   assert(map->count < GLSL_MAP_MAX_ENTRIES(map->num_bits));

   MapEntry *to_insert = &map->entries[map->count++];
   to_insert->k = k;
   to_insert->v = v;
   unsigned int hash = glsl_map_hash(k, map->num_bits);

   unsigned int i = hash;
   for (;;) {
      MapEntry **bucket = &map->buckets[i];
      if (!*bucket) {
         *bucket = to_insert;
         break;
      }
      unsigned int existing_hash = glsl_map_hash((*bucket)->k, map->num_bits);
      if (((i - hash) & gfx_mask(map->num_bits)) >= ((i - existing_hash) & gfx_mask(map->num_bits))) {
         // Robin-Hood: steal this bucket!
         // Note >= ensures most recently inserted value is always first in case of duplicates.
         MapEntry *tmp = *bucket;
         *bucket = to_insert;
         to_insert = tmp;
         hash = existing_hash;
      }
      ++i;
      if (i == (1 << map->num_bits))
         i = 0;
      assert(i != hash); // If this fires we got all the way round the map without finding a gap...
   }
}

void *glsl_map_get(const Map *map, const void *k)
{
   unsigned int hash = glsl_map_hash(k, map->num_bits);
   unsigned int i = hash;
   for (;;) {
      const MapEntry *entry = map->buckets[i];
      if (!entry)
         return NULL;
      if (entry->k == k)
         return entry->v;
      unsigned int other_hash = glsl_map_hash(entry->k, map->num_bits);
      if (((i - hash) & gfx_mask(map->num_bits)) > ((i - other_hash) & gfx_mask(map->num_bits)))
         // As inserts use Robin-Hood rule if we see this we know we won't find the key
         // Note need > to ensure we check all keys with matching hash
         return NULL;
      ++i;
      if (i == (1 << map->num_bits))
         i = 0;
      assert(i != hash); // If this fires we got all the way round the map without hitting a gap...
   }
}
