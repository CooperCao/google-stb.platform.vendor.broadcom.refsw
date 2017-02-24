/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_arenamem.h"
#include "libs/util/common.h"

EXTERN_C_BEGIN

extern ArenaAlloc *fastmem_arena;

// Legacy allocation functions - use ArenaAlloc directly for new code
// - no way to free individual allocations
// - triggers a compiler error if out of memory, so user NULL checks are not required
#define malloc_fast(n)  glsl_arena_malloc(fastmem_arena, n)
#define strdup_fast(s)  glsl_arena_strdup(fastmem_arena, s)

void glsl_fastmem_init();
void glsl_fastmem_term();

EXTERN_C_END
