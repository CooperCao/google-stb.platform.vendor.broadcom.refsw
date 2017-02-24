/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_scoped_map.h"
#include "glsl_safemem.h"

#include <stdlib.h>

ScopedMap *glsl_scoped_map_new(void) {
   ScopedMap *map = glsl_safemem_malloc(sizeof(ScopedMap));
   map->scopes = NULL;
   glsl_scoped_map_push_scope(map);
   return map;
}

void glsl_scoped_map_delete(ScopedMap *map) {
   while (map->scopes != NULL) glsl_scoped_map_pop_scope(map);
   glsl_safemem_free(map);
}

void glsl_scoped_map_put(ScopedMap *map, const void *k, void *v)
{
   glsl_map_put(map->scopes->map, k, v);
}

void *glsl_scoped_map_get(const ScopedMap *map, const void *k)
{
   for (ScopeList *scope = map->scopes; scope; scope = scope->next) {
      void *v = glsl_map_get(scope->map, k);
      if (v != NULL)
         return v;
   }
   return NULL;
}

void glsl_scoped_map_push_scope(ScopedMap *map)
{
   ScopeList *scope = glsl_safemem_malloc(sizeof(ScopeList));
   scope->map = glsl_map_new();
   scope->next = map->scopes;
   map->scopes = scope;
}

void glsl_scoped_map_pop_scope(ScopedMap *map)
{
   ScopeList *popped_scope = map->scopes;
   map->scopes = map->scopes->next;

   glsl_map_delete(popped_scope->map);
   glsl_safemem_free(popped_scope);
}
