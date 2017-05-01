/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/2708/glsl_allocator_4.h"
#include "middleware/khronos/glsl/glsl_fastmem.h"
#include "middleware/khronos/common/khrn_hw.h"

#define HUNK_SIZE 65536

#include <stdlib.h>

Hunk *fastmem_hunk;

void glsl_fastmem_init(void)
{
   fastmem_hunk = NULL;
}

void glsl_fastmem_term(void)
{
   Hunk *hunk, *next;

   for (hunk = fastmem_hunk; hunk; hunk = next)
   {
      next = hunk->next;
      free(hunk->data);
      free(hunk);
   }

   fastmem_hunk = NULL;

   glsl_allocator_mem_term();
}

Hunk *glsl_fastmem_alloc_hunk(Hunk *next, size_t curBlockSize)
{
   size_t hunk_size = _max(curBlockSize, HUNK_SIZE);

   /*
      check free list for available hunks; allocate one from the global heap if none found
   */
   Hunk *hunk = (Hunk*)malloc(sizeof(Hunk));
   khrn_memset(hunk, 0, sizeof(Hunk));

   hunk->data = (char*)malloc(hunk_size);
   khrn_memset(hunk->data, 0, hunk_size);

   /*
      initialize hunk
   */
   hunk->hunk_size = hunk_size;
   hunk->used = 0;
   hunk->next = next;

   return hunk;
}
