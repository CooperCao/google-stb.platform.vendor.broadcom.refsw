/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_FASTMEM_H
#define GLSL_FASTMEM_H

#include "glsl_arenamem.h"

extern ArenaAlloc *fastmem_arena;

// Legacy allocation functions - use ArenaAlloc directly for new code
// - no way to free individual allocations
// - triggers a compiler error if out of memory, so user NULL checks are not required
#define malloc_fast(n)  glsl_arena_malloc(fastmem_arena, n)
#define strdup_fast(s)  glsl_arena_strdup(fastmem_arena, s)

void glsl_fastmem_init();
void glsl_fastmem_term();

#endif // fastmem_H
