/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "middleware/khronos/common/khrn_memaccess.h"
#include "gmem.h"
#include <string.h>

static void khrn_mem_read(void *dst, v3d_addr_t src_addr, size_t size,
   const char *file, uint32_t line, const char *func, void *p)
{
   memcpy(dst, gmem_addr_to_ptr(src_addr), size);
}

const struct simcom_memaccess khrn_memaccess_ro = {
   .read = khrn_mem_read};
