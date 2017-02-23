/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#include "v3d_gmp.h"
#include "v3d_limits.h"
#include <assert.h>

void v3d_gmp_add_permissions(void* table_in, v3d_addr_t start, v3d_addr_t end, v3d_gmp_flags_t flags)
{
   assert(start < end);

   if (flags & V3D_GMP_WRITE_ACCESS)
      flags |= V3D_GMP_READ_ACCESS;

   uint8_t* table = (uint8_t*)table_in;
   unsigned first_page = (unsigned)(start / V3D_GMP_PAGE_SIZE);
   unsigned last_page = (unsigned)((end - 1) / V3D_GMP_PAGE_SIZE);

   for (unsigned p = first_page; p <= last_page; ++p)
   {
      unsigned byte = p / 4;
      unsigned bit = (p % 4) * 2;
      table[byte] |= flags << bit;
   }
}