/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "v3d_common.h"
#include "v3d_reg_desc.h"
#include "v3d_reg_desc_maps.h"
#include "v3d_regs.h"

#include "helpers/gfx/gfx_util_str.h"
#include "vcos_string.h"

#include <ctype.h>

size_t v3d_sprint_reg(char *buf, size_t buf_size, size_t offset, uint32_t addr)
{
   uint32_t core = UINT32_MAX;
   uint32_t core0_addr = addr;
   bool hub_reg = v3d_is_hub_reg(addr);
#if V3D_VER_EQUAL(3,3)
   bool mmu_reg = v3d_is_mmu_reg(addr);
   if(mmu_reg)
   {
      core0_addr = v3d_mmu_reg_to_core0(addr);
      core = v3d_mmu_reg_core(addr);
   }
   else
#endif
   if(!hub_reg)
   {
      core0_addr = v3d_reg_to_core0(addr);
      core = v3d_reg_core(addr);
   }

   uint32_t offset_from_desc;
   const char *desc = desc_map_try_value_to_desc_and_offset(&offset_from_desc,
      (core == UINT32_MAX) ? &v3d_reg_hub_desc_map : &v3d_reg_core0_desc_map,
      core0_addr);
   assert(desc);

   offset = vcos_safe_sprintf(buf, buf_size, offset, "%s", desc);
   if (core != UINT32_MAX)
      offset = vcos_safe_sprintf(buf, buf_size, offset, "(%u)", core);
   if (offset_from_desc != 0)
      offset = vcos_safe_sprintf(buf, buf_size, offset, "+0x%x", offset_from_desc);

   return offset;
}

uint32_t v3d_try_reg_from_desc(const char *desc_in)
{
   char desc[256];
   if (vcos_safe_strcpy(desc, desc_in, sizeof(desc), 0) >= sizeof(desc))
      return UINT32_MAX;

   /* Parse offset */
   char *plus = strrchr(desc, '+');
   unsigned long offset = 0;
   if (plus)
   {
      if (!gfx_try_strtoul(&offset, plus + 1, 0))
         return UINT32_MAX;
      while ((plus > desc) && isspace(plus[-1]))
         --plus;
      *plus = '\0';
   }

   /* Try parsing as hub reg */
   {
      uint32_t addr;
      if (desc_map_try_desc_to_value(&addr, &v3d_reg_hub_desc_map, desc))
         return addr + offset;
   }

   /* Assume it's a core reg... */

   unsigned long core = 0;
   if (!*desc)
      return UINT32_MAX;
   char *last = desc + strlen(desc) - 1;
   if (*last == ')')
   {
      *last = '\0';
      char *open_paren = strrchr(desc, '(');
      if (!open_paren)
         return UINT32_MAX;
      if (!gfx_try_strtoul(&core, open_paren + 1, 0))
         return UINT32_MAX;
      *open_paren = '\0';
   }
   else
   {
      char *first_underscore = strchr(desc, '_');
      char *second_underscore = first_underscore ? strchr(first_underscore + 1, '_') : NULL;
      if (second_underscore)
      {
         *second_underscore = '\0';
         if (gfx_try_strtoul(&core, first_underscore + 1, 0))
            memmove(first_underscore + 1, second_underscore + 1, last - second_underscore + 1);
         else
            *second_underscore = '_';
      }
   }

   uint32_t core0_addr;
   if (desc_map_try_desc_to_value(&core0_addr, &v3d_reg_core0_desc_map, desc))
   {
#if V3D_VER_EQUAL(3,3)
      if(v3d_is_mmu_reg(core0_addr))
      {
         return v3d_mmu_reg_to_core(core, core0_addr) + offset;
      }
      else
#endif
      {
         return v3d_reg_to_core(core, core0_addr) + offset;
      }
   }

   return UINT32_MAX;
}
