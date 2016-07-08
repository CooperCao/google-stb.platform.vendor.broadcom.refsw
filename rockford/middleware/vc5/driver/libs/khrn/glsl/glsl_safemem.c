/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdio.h>
#include <stdlib.h>

#include "glsl_errors.h"
#include "glsl_mem_utils.h"
#include "glsl_safemem.h"

typedef struct _Alloc {
   struct _Alloc *next;
   struct _Alloc *prev;
#ifdef GLSL_MEMORY_DEBUG
   const char *file;
   int         line;
   size_t      bytes;
#endif
} Alloc;

static Alloc *s_safemem_head = NULL;
static Alloc *s_safemem_tail = NULL;

static Alloc *alloc_from_data(void *data) {
   return (Alloc *)(((char *)data) - aligned_sizeof(Alloc));
}

static void *alloc_to_data(Alloc *a) {
   return (void *)(((char *)a) + aligned_sizeof(Alloc));
}

static void repair_alloc(Alloc *a)
{
   if(a->prev) {
      a->prev->next = a;
   } else {
      s_safemem_head = a;
   }
   if(a->next) {
      a->next->prev = a;
   } else {
      s_safemem_tail = a;
   }
}

static void out_of_memory_if_null(void *data) {
   if(!data) {
      glsl_compile_error(ERROR_CUSTOM, 6, -1, NULL);
   }
}

static void insert_alloc(Alloc *a)
{
   a->next = NULL;
   a->prev = s_safemem_tail;

   repair_alloc(a);
}

#ifdef GLSL_MEMORY_DEBUG
static void tag_alloc_debug(Alloc *a, const char *file, int line, size_t bytes)
{
   a->file  = file;
   a->line  = line;
   a->bytes = bytes;
}
#endif

static void extract_alloc(Alloc *a) {
   if(a->prev) {
      a->prev->next  = a->next;
   } else {
      s_safemem_head = a->next;
   }
   if(a->next) {
      a->next->prev  = a->prev;
   } else {
      s_safemem_tail = a->prev;
   }
}

#ifndef GLSL_MEMORY_DEBUG
void *glsl_safemem_malloc(size_t size)
#else
void *glsl_safemem_malloc_debug(size_t size, const char *file, int line)
#endif
{
   Alloc *a = malloc(size + aligned_sizeof(*a));
   out_of_memory_if_null(a);
   insert_alloc(a);
#ifdef GLSL_MEMORY_DEBUG
   tag_alloc_debug(a, file, line, size);
#endif
   return alloc_to_data(a);
}

#ifndef GLSL_MEMORY_DEBUG
void *glsl_safemem_calloc(size_t nmemb, size_t size)
#else
void *glsl_safemem_calloc_debug(size_t nmemb, size_t size, const char *file, int line)
#endif
{
   Alloc *a = calloc(1, nmemb * size + aligned_sizeof(*a));
   out_of_memory_if_null(a);
   insert_alloc(a);
#ifdef GLSL_MEMORY_DEBUG
   tag_alloc_debug(a, file, line, nmemb*size);
#endif
   return alloc_to_data(a);
}

#ifndef GLSL_MEMORY_DEBUG
void *glsl_safemem_realloc(void *data, size_t size)
#else
void *glsl_safemem_realloc_debug(void *data, size_t size, const char *file, int line)
#endif
{
#ifndef GLSL_MEMORY_DEBUG
   if (!data) return glsl_safemem_malloc(size);
#else
   if (!data) return glsl_safemem_malloc_debug(size, file, line);
#endif

   Alloc *a = alloc_from_data(data);
   a = realloc(a, size + aligned_sizeof(*a));
   out_of_memory_if_null(a);
   repair_alloc(a);
#ifdef GLSL_MEMORY_DEBUG
   tag_alloc_debug(a, file, line, size);
#endif
   return alloc_to_data(a);
}

void glsl_safemem_free(void *data)
{
   if(!data) return;

   Alloc *a = alloc_from_data(data);
   extract_alloc(a);
   free(a);
}

void glsl_safemem_cleanup()
{
   Alloc *cur, *next;

   for(cur = s_safemem_head; cur; cur=next) {
      next = cur->next;
      free(cur);
   }
   s_safemem_head = NULL;
   s_safemem_tail = NULL;
}

void glsl_safemem_verify()
{
   if(s_safemem_head != NULL) {
      fprintf(stderr,
              "glsl_safemem_verify: Not all safemem allocated blocks have been freed\n");
      glsl_safemem_dump();
   }
}

void glsl_safemem_dump() {
   int count = 0;
   for(Alloc *cur = s_safemem_tail; cur; cur=cur->prev) {
#ifdef GLSL_MEMORY_DEBUG
      fprintf(stderr, "Alloc %d%s\n", count, count == 0 ? " (most recent)" : "");
      fprintf(stderr, " %u bytes (%s:%d)\n", cur->bytes, cur->file, cur->line);
#endif
      ++count;
   }
   fprintf(stderr, "glsl_safemem_dump:   Outstanding allocations: %d\n", count);
#ifndef GLSL_MEMORY_DEBUG
   fprintf(stderr, "glsl_safemem_dump:   Rebuild with GLSL_MEMORY_DEBUG to see detailed output\n");
#endif
}
