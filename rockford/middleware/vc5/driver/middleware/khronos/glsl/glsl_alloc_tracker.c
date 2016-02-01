/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include "glsl_alloc_tracker.h"
#include "glsl_file_utils.h"

struct alloc_loc {
   const char       *file;
   int               line;
   size_t            num_allocs;
   size_t            total_bytes;
   struct alloc_loc *next;
};

struct glsl_alloc_tracker_s {
   struct alloc_loc *allocs;
};

glsl_alloc_tracker_t *glsl_alloc_tracker_create() {
   glsl_alloc_tracker_t *ret;
   ret= malloc(sizeof(*ret));
   if(!ret) {
      return NULL;
   }
   ret->allocs = NULL;
   return ret;
}

bool glsl_alloc_tracker_add(glsl_alloc_tracker_t *tracker, const char *file, int line, size_t bytes) {
   struct alloc_loc *cur;
   for(cur = tracker->allocs;cur; cur = cur->next) {
      if(line == cur->line && !strcmp(file, cur->file)) {
         cur->num_allocs  += 1;
         cur->total_bytes += bytes;
         return true;
      }
   }
   cur              = malloc(sizeof(*cur));
   if(!cur) {
      return false;
   }
   cur->file        = file;
   cur->line        = line;
   cur->total_bytes = bytes;
   cur->num_allocs  = 1;
   cur->next        = tracker->allocs;
   tracker->allocs  = cur;
   return true;
}

void glsl_alloc_tracker_print(const glsl_alloc_tracker_t *tracker) {
   const struct alloc_loc *cur;
   for(cur = tracker->allocs ;cur; cur = cur->next) {
      printf("  %25s:%-5d %8zu bytes total, %4zu allocations\n",
             glsl_file_utils_basename(cur->file),
             cur->line,
             cur->total_bytes,
             cur->num_allocs);
   }
}

void glsl_alloc_tracker_free(glsl_alloc_tracker_t *tracker) {
   struct alloc_loc *cur, *next;
   for(cur = tracker->allocs; cur; cur = next) {
      next = cur->next;
      free(cur);
   }
   free(tracker);
}
