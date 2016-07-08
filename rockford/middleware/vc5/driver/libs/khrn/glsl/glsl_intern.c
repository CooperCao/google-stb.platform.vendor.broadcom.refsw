/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <string.h>
#include <stdbool.h>
#include "glsl_fastmem.h"

struct {
   int size;
   int used;

   const char **table;
} state;

void glsl_init_intern(int size)
{
   state.size = size;
   state.used = 0;

   state.table = (const char **)malloc_fast(size * sizeof(const char *));
   /* cast to keep msvc happy */
   memset((void *)state.table, 0, size * sizeof(const char *));
}

static unsigned long hash(const char *str)
{
   unsigned long hash = 5381;
   int c;

   while ((c = *str++) != 0)
      hash = hash *33 ^ c;

   return hash;
}

const char *glsl_intern(const char *s, bool dup)
{
   const char **t;

   if (state.used >= state.size >> 1) {
      int i, size = state.size;
      const char **table = state.table;

      glsl_init_intern(state.size << 1);

      for (i = 0; i < size; i++)
         if (table[i])
            glsl_intern(table[i], false);
   }

   for (t = state.table + (hash(s) & (state.size - 1)); *t; t = (t == state.table + state.size - 1) ? state.table : t + 1)
      if (!strcmp(s, *t))
         return *t;

   state.used++;

   return *t = dup ? strdup_fast(s) : s;
}
