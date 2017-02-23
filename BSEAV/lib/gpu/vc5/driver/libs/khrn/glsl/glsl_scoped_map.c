/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_scoped_map.h"
#include "glsl_fastmem.h"

#include <stdlib.h>

ScopedMap* glsl_scoped_map_new(void)
{
   ScopedMap* map = (ScopedMap*)malloc_fast(sizeof(ScopedMap));
   map->scopes = NULL;
   glsl_scoped_map_push_scope(map);
   return map;
}

void glsl_scoped_map_put(ScopedMap* map, const void* k, void* v)
{
   glsl_map_put(map->scopes->map, k, v);
}

void* glsl_scoped_map_get(const ScopedMap* map, const void* k)
{
   ScopeList* scope;
   for (scope = map->scopes; scope; scope = scope->next) {
      void* v = glsl_map_get(scope->map, k);
      if (v != NULL)
         return v;
   }
   return NULL;
}

void glsl_scoped_map_push_scope(ScopedMap* map)
{
   ScopeList* scope;
   scope = (ScopeList*)malloc_fast(sizeof(ScopeList));
   scope->map = glsl_map_new();
   scope->next = map->scopes;
   map->scopes = scope;
}

void glsl_scoped_map_pop_scope(ScopedMap* map)
{
   map->scopes = map->scopes->next;
}
