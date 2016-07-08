/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "glsl_map.h"
#include "glsl_fastmem.h"

#include "vcos.h"

static inline unsigned int glsl_map_hash(const void *k, int num_bits)
{
   assert(k != NULL);
   // the magic constant is prime close to 2^32 / golden ratio
   return ((unsigned int)(uintptr_t)k * 2654435761u) >> (8 * sizeof(unsigned int) - num_bits);
}

static void glsl_map_resize(Map *map, int num_bits)
{
   size_t size = sizeof(MapNode*) * (1 << num_bits);
   map->hashmap = malloc_fast(size);
   map->num_bits = num_bits;
   memset(map->hashmap, 0, size);
   // Add all nodes to the new hashmap (the order of iteration is important)
   for (MapNode *node = map->head; node; node = node->next) {
      // Prepend the value so that the most recent value is returned from glsl_map_get
      unsigned int hash = glsl_map_hash(node->k, num_bits);
      node->hashmap_next = map->hashmap[hash];
      map->hashmap[hash] = node;
   }
}

Map *glsl_map_new()
{
   Map *map = malloc_fast(sizeof(Map));
   map->head = NULL;
   map->tail = NULL;
   map->count = 0;
   glsl_map_resize(map, 4);
   return map;
}

void glsl_map_put(Map *map, const void *k, void *v)
{
   if (map->count >= (1 << map->num_bits))
      glsl_map_resize(map, map->num_bits + 2);

   MapNode *node = malloc_fast(sizeof(MapNode));
   node->k = k;
   node->v = v;

   // Add to linked list of all nodes
   node->next = NULL;
   node->prev = map->tail;
   if (map->count == 0) {
      map->head = node;
   } else {
      map->tail->next = node;
   }
   map->tail = node;
   map->count++;

   // Add to hash map
   // Prepend the value so that the most recent value is returned from glsl_map_get
   unsigned int hash = glsl_map_hash(node->k, map->num_bits);
   node->hashmap_next = map->hashmap[hash];
   map->hashmap[hash] = node;
}

void *glsl_map_get(const Map *map, const void *k)
{
   unsigned int hash = glsl_map_hash(k, map->num_bits);
   for (MapNode *node = map->hashmap[hash]; node; node = node->hashmap_next) {
      if (node->k == k)
         return node->v;
   }
   return NULL;
}
