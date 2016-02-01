/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#include "middleware/khronos/glsl/glsl_common.h"

#include <stdlib.h>
#include "middleware/khronos/glsl/glsl_map.h"
#include "middleware/khronos/glsl/glsl_intern.h"
#include "middleware/khronos/glsl/glsl_fastmem.h"

Map* glsl_map_new()
{
   Map* map = (Map *)malloc_fast(sizeof(Map));
   map->head = NULL;
   return map;
}

Map* glsl_map_clear(Map* map)
{
   map->head = NULL;
   return map;
}

bool glsl_map_put(Map* map, const void* k, void* v)
{
   MapNode* new_head = NULL;

   if (!map) return false;
   if (!k) return false;

   //vcos_assert(k == glsl_intern((const char *)k, true));

   new_head = (MapNode *)malloc_fast(sizeof(MapNode));

   new_head->k = k;
   new_head->v = v;
   new_head->next = map->head;
   map->head = new_head;
   return true;
}

void glsl_map_reverse(Map* map)
{
	MapNode* node;

	if (!map) return;
	node = map->head;
	map->head = NULL;
	while(node)
	{
		MapNode* next = node->next;
		node->next = map->head;
		map->head = node;
		node = next;
	}
}

void *glsl_map_get(Map* map, const void* k, bool cross_checkpoints)
{
   MapNode* curr = NULL;

   if (!map) return NULL;
   if (!k) return NULL;

   //vcos_assert(k == glsl_intern((const char *)k, true));

   curr = map->head;

   while (curr)
   {
      vcos_assert(((size_t)curr & 3) == 0);

      if (NULL == curr->k && NULL == curr->v)
      {
         if (!cross_checkpoints)
            return NULL;
      }
      else if (k == curr->k)
      {
         return curr->v;
      }

      curr = curr->next;
   }

   return NULL;
}

bool glsl_map_set_checkpoint(Map* map)
{
   MapNode* new_head = NULL;

   if (!map) return false;

   new_head = (MapNode *)malloc_fast(sizeof(MapNode));

   new_head->k = NULL;
   new_head->v = NULL;
   new_head->next = map->head;
   map->head = new_head;
   return true;
}

bool glsl_map_clear_to_checkpoint(Map* map)
{
   MapNode* curr = NULL;

   if (!map) return false;

   curr = map->head;

   while (curr)
   {
      if (NULL == curr->k && NULL == curr->v)
      {
         // We found a checkpoint.
         map->head = curr->next;
         return true;
      }
      else
      {
         curr = curr->next;
      }
   }

   map->head = NULL;
   return false;
}

void* glsl_map_pop(Map* map)
{
   void* ret = NULL;
   MapNode* old_head = NULL;

   if (map->head)
   {
      ret = map->head->v;

      old_head = map->head;
      map->head = map->head->next;
   }

   return ret;
}
