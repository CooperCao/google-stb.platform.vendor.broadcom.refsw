/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
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
   MEM_HANDLE_T handle = MEM_INVALID_HANDLE;
   MEM_HANDLE_T dhandle = MEM_INVALID_HANDLE;
   Hunk *hunk;
   size_t hunk_size = _max(curBlockSize, HUNK_SIZE);

   /*
      check free list for available hunks; allocate one from the global heap if none found
   */
   hunk = (Hunk*)malloc(sizeof(Hunk));
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
