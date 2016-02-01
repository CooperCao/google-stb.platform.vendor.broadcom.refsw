/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include "middleware/khronos/egl/egl_map.h"

#define CHUNK (16)

static bool resize(EGL_MAP_T *map)
{
   void *p;

   map->max += CHUNK;
   p = realloc(map->members, map->max * sizeof (EGL_PAIR_T));

   if (p)
   {
      map->members = p;
      return true;
   }
   else
   {
      map->max -= CHUNK;
      return false;
   }
}

void egl_map_init(EGL_MAP_T *map)
{
   memset(map, 0, sizeof *map);
}

static EGL_PAIR_T *find(const EGL_MAP_T *map, const void *handle)
{
   size_t i, n = map->count;

   for (i = 0; i < n; i++)
      if (map->members[i].handle == handle)
         return map->members + i;

   return NULL;
}

static EGL_PAIR_T *reverse_find(const EGL_MAP_T *map, void *value)
{
   size_t i, n = map->count;

   for (i = 0; i < n; i++)
      if (map->members[i].value == value)
         return map->members + i;

   return NULL;
}

bool egl_map_insert(EGL_MAP_T *map, const void *handle, void *value)
{
   EGL_PAIR_T *pair = find(map, handle);

   if (pair)
   {
      pair->value = value;
      return true;
   }

   if (map->count == map->max)
      if (!resize(map)) return false;

   pair = map->members + map->count++;
   pair->handle = handle;
   pair->value = value;
   return true;
}

void* egl_map_remove(EGL_MAP_T *map, const void *handle)
{
   size_t i, n = map->count;

   for (i = 0; i < n; i++)
   {
      if (map->members[i].handle == handle)
      {
         void *value = map->members[i].value;
         map->members[i] = map->members[--map->count];
         return value;
      }
   }
   return NULL;
}

void *egl_map_lookup(const EGL_MAP_T *map, const void *handle)
{
   EGL_PAIR_T *pair = find(map, handle);
   return pair ? pair->value : NULL;
}

const void *egl_map_reverse_lookup(const EGL_MAP_T *map, void *value)
{
   EGL_PAIR_T *pair = reverse_find(map, value);
   return pair ? pair->handle : NULL;
}

void egl_map_destroy(EGL_MAP_T *map)
{
   if (!map) return;
   free(map->members);
   memset(map, 0, sizeof *map);
}

void egl_map_move(EGL_MAP_T *dest, EGL_MAP_T *src)
{
   *dest = *src;
   memset(src, 0, sizeof *src);
}

EGL_PAIR_T *egl_map_items(EGL_MAP_T *map, size_t *i)
{
   size_t n = map->count;

   if (*i < n)
      return map->members + (*i)++;
   else
      return NULL;
}

EGL_PAIR_T *egl_map_pop(EGL_MAP_T *map)
{
   if (map->count == 0)
      return NULL;

   return map->members + --map->count;
}
