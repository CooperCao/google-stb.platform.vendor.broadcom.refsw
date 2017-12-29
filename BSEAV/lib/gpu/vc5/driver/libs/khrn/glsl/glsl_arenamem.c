/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glsl_alloc_tracker.h"
#include "glsl_arenamem.h"
#include "glsl_mem_utils.h"

typedef struct _ArenaHunk {
   struct _ArenaHunk *prev;
   bool               is_zero;
   size_t             data_used;
   size_t             data_size;
} ArenaHunk;

struct _ArenaAlloc {
   ArenaHunk *hunks;
   size_t     hunk_size;
#ifdef GLSL_MEMORY_DEBUG
   const char *origin_file;
   int         origin_line;
#endif
};

#ifdef GLSL_MEMORY_DEBUG
struct mem_record {
   const char  *file;
   int          line;
   size_t       bytes;
};


/* TODO: This is basically a copy of POSIX basename. Use that instead? */
const char *file_basename(const char *filename) {
   const char *p;
   const char *ret;

   ret = filename;
   for(p = filename; *p; ++p) {
      if(*p == '/' || *p == '\\') {
         ret = p+1;
      }
   }
   return ret;
}
#endif

#ifndef GLSL_MEMORY_DEBUG
ArenaAlloc *glsl_arena_create(size_t hunk_size)
#else
ArenaAlloc *glsl_arena_create_debug(size_t hunk_size, const char *file, int line)
#endif
{
   ArenaAlloc *ret  = glsl_safemem_malloc(sizeof(*ret));
   ret->hunks       = NULL;
   ret->hunk_size   = hunk_size;
#ifdef GLSL_MEMORY_DEBUG
   ret->origin_file = file_basename(file);
   ret->origin_line = line;
#endif

   return ret;
}

static void *do_alloc(ArenaAlloc *a, size_t size, bool is_zero)
{
   ArenaHunk *h = a->hunks;
   if(!h || h->data_used + size > h->data_size) {
      const size_t data_size  = size > a->hunk_size ? size : a->hunk_size;
      const size_t alloc_size = data_size + aligned_sizeof(*h);
      h            = is_zero ? glsl_safemem_calloc(1, alloc_size) : glsl_safemem_malloc(alloc_size);
      h->prev      = a->hunks;
      h->data_size = data_size;
      h->data_used = 0;
      h->is_zero   = is_zero;
      a->hunks     = h;
   }
   void *ret = ((char *)h) + aligned_sizeof(*h) + h->data_used;
   h->data_used += aligned_size(size);
   return ret;
}

#ifdef GLSL_MEMORY_DEBUG
static void *do_alloc_debug(ArenaAlloc *a, size_t size, bool is_zero, const char *file, int line)
{
   const size_t header_size = aligned_sizeof(struct mem_record);
   void *ret = do_alloc(a, size + header_size, is_zero);

   struct mem_record *rec = ret;
   rec->file  = file;
   rec->line  = line;
   rec->bytes = size;
   ret = ((char *)ret) + aligned_sizeof(struct mem_record);

   return ret;
}
#endif

#ifndef GLSL_MEMORY_DEBUG
void *glsl_arena_malloc(ArenaAlloc *a, size_t size)
#else
void *glsl_arena_malloc_debug(ArenaAlloc *a, size_t size, const char *file, int line)
#endif
{
#ifndef GLSL_MEMORY_DEBUG
   return do_alloc(a, size, false);
#else
   return do_alloc_debug(a, size, false, file, line);
#endif
}

#ifndef GLSL_MEMORY_DEBUG
void *glsl_arena_calloc(ArenaAlloc *a, size_t nmemb, size_t size)
#else
void *glsl_arena_calloc_debug(ArenaAlloc *a, size_t nmemb, size_t size, const char *file, int line)
#endif
{
   void *ret;
#ifndef GLSL_MEMORY_DEBUG
   ret = do_alloc(a, size * nmemb, true);
#else
   ret = do_alloc_debug(a, size * nmemb, true, file, line);
#endif
   if(!a->hunks->is_zero) {
      memset(ret, 0, size);
   }
   return ret;
}

#ifndef GLSL_MEMORY_DEBUG
char *glsl_arena_strdup(ArenaAlloc *a, const char *s)
#else
char *glsl_arena_strdup_debug(ArenaAlloc *a, const char *s, const char *file, int line)
#endif
{
   const size_t len = strlen(s) + 1;
   char *ret;

#ifndef GLSL_MEMORY_DEBUG
   ret = glsl_arena_malloc(a, len);
#else
   ret = glsl_arena_malloc_debug(a, len, file, line);
#endif
   memcpy(ret, s, len);
   return ret;
}


void glsl_arena_free(ArenaAlloc *a)
{
   ArenaHunk *cur, *prev;
   if(!a) {
      return;
   }

#ifdef GLSL_MEMORY_DEBUG
   glsl_arena_print(a);
#endif

   for(cur = a->hunks; cur; cur = prev) {
      prev = cur->prev;
      glsl_safemem_free(cur);
   }
   glsl_safemem_free(a);
}

#ifdef GLSL_MEMORY_DEBUG
void glsl_arena_print(const ArenaAlloc *a) {
   ArenaHunk *cur;
   int hunk_count;

   printf("Arena allocated at %s:%d\n", a->origin_file, a->origin_line);

   hunk_count = 0;
   for(cur = a->hunks; cur; cur = cur->prev) {
      glsl_alloc_tracker_t *tracker;
      int    alloc_count;
      size_t offset;
      printf("Hunk %d%s\n", hunk_count, hunk_count == 0 ? " (most recent)" : "");
      offset      = aligned_sizeof(*cur);
      alloc_count = 0;
      tracker     = glsl_alloc_tracker_create();
      while(offset < cur->data_used) {
         struct mem_record *rec = (void *)(((char *)cur) + offset);
         glsl_alloc_tracker_add(tracker, rec->file, rec->line, rec->bytes);
         offset += aligned_sizeof(*rec);
         offset += aligned_size  (rec->bytes);
         ++alloc_count;
      }
      ++hunk_count;
      glsl_alloc_tracker_print(tracker);
      printf("Total allocations: %5d Total bytes: %8zu\n", alloc_count, offset);
      glsl_alloc_tracker_free(tracker);
   }
}
#endif
