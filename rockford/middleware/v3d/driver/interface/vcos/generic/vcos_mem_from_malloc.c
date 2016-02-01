/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  vcos
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION
VideoCore OS Abstraction Layer - memory alloc implementation
=============================================================================*/

#include "interface/vcos/vcos.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef struct malloc_header_s {
   uint32_t guardword;
   uint32_t size;
   const char *description;
   void *ptr;
} MALLOC_HEADER_T;


#define MIN_ALIGN sizeof(MALLOC_HEADER_T)

#define VCOS_ALIGN_DOWN(p,n) (((ptrdiff_t)(p)) & ~((n)-1))
#define VCOS_ALIGN_UP(p,n) VCOS_ALIGN_DOWN((ptrdiff_t)(p)+(n)-1,(n))
#define GUARDWORDHEAP  0xa55a5aa5

void *vcos_generic_mem_alloc_aligned(VCOS_UNSIGNED size, VCOS_UNSIGNED align, const char *desc)
{
   int local_align = align == 0 ? 1 : align;
   int required_size = size + local_align + sizeof(MALLOC_HEADER_T);
   void *ptr = malloc(required_size);
   void *ret = (void *)VCOS_ALIGN_UP(((char *)ptr)+sizeof(MALLOC_HEADER_T), local_align);
   MALLOC_HEADER_T *h = ((MALLOC_HEADER_T *)ret)-1;

   h->size = size;
   h->description = desc;
   h->guardword = GUARDWORDHEAP;
   h->ptr = ptr;

   return ret;
}

void *vcos_generic_mem_alloc(VCOS_UNSIGNED size, const char *desc)
{
   return vcos_generic_mem_alloc_aligned(size,MIN_ALIGN,desc);
}

void *vcos_generic_mem_calloc(VCOS_UNSIGNED count, VCOS_UNSIGNED sz, const char *desc)
{
   uint32_t size = count*sz;
   void *ptr = vcos_generic_mem_alloc_aligned(size,MIN_ALIGN,desc);
   if (ptr)
   {
      memset(ptr, 0, size);
   }
   return ptr;
}

void vcos_generic_mem_free(void *ptr)
{
   MALLOC_HEADER_T *h;
   vcos_assert(ptr);

   h = ((MALLOC_HEADER_T *)ptr)-1;
   vcos_assert(h->guardword == GUARDWORDHEAP);
   free(h->ptr);
}
