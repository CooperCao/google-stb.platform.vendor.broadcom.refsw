/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_FASTMEM_H
#define GLSL_FASTMEM_H

#include "middleware/khronos/common/khrn_mem.h"
#include "middleware/khronos/common/khrn_hw.h"

#include "middleware/khronos/glsl/glsl_errors.h"

#include <string.h>

// Fast, inlined allocation functions.
// - no way to free individual allocations
// - triggers a compiler error if out of memory, so user NULL checks are not required
#define malloc_fast(n)  glsl_fastmem_malloc((n), true)
#define strdup_fast(s)  glsl_fastmem_strdup(s)

//
// The details.
//

typedef struct _Hunk {
   char *data;
   size_t hunk_size;

   size_t used;

   struct _Hunk *next;
} Hunk;

extern Hunk *fastmem_hunk;

extern void glsl_fastmem_init(void);
extern void glsl_fastmem_term(void);

extern Hunk *glsl_fastmem_alloc_hunk(Hunk *next, size_t curBlockSize);

/*
   void *glsl_fastmem_malloc(size_t count, bool error)

   Allocates count bytes of memory on the fastmem stack. This allocation remains valid
   until the whole stack is torn down with glsl_fastmem_term.

   Khronos documentation:

   -

   Implementation notes:

   -

   Preconditions:

   Postconditions:

   If out of memory then:
      If error then:
         Raises C0006: Out of memory
      Else:
         Returns NULL
   Else:
      Returns a pointer to count bytes, valid for the lifetime of fastmem

   Invariants preserved:

   -
*/
static INLINE void *glsl_fastmem_malloc(size_t count, bool error)
{
   void *v;

   /*
      round count up to next multiple of 4
   */

   count = (count + 3) & ~3;

   /*
      allocate a new hunk if necessary
   */

   if (!fastmem_hunk || fastmem_hunk->used + count > fastmem_hunk->hunk_size) {
      Hunk *hunk = glsl_fastmem_alloc_hunk(fastmem_hunk, count);

      if (!hunk) {
         if (error)
            glsl_compile_error(ERROR_CUSTOM, 6, -1, NULL);

         return NULL;
      }

      fastmem_hunk = hunk;
   }

   /*
      calculate result
   */

   v = fastmem_hunk->data + fastmem_hunk->used;

   /*
      increment bytes used
   */

   fastmem_hunk->used += count;

   return v;
}

static INLINE char *glsl_fastmem_strdup(const char *s)
{
   size_t n = strlen(s) + 1;
   char *d = (char *)glsl_fastmem_malloc(n, true);
   khrn_memcpy(d, s, n);
   return d;
}

#endif // fastmem_H
