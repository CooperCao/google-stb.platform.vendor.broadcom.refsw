/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009-2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  autoclif recording interface

FILE DESCRIPTION
Drivers use this interface to trigger autoclif recording
=============================================================================*/

#include "khrn_record.h"

#ifdef KHRN_AUTOCLIF

#include <stdio.h>
#include <stdlib.h>
#include "libs/util/log/log.h"
#include "libs/platform/gmem.h"
#include "libs/util/demand.h"
#include "khrn_options.h"
#include "khrn_memaccess.h"
#include "khrn_record.h"
#include "khrn_control_list.h"
#include "libs/tools/autoclif/autoclif.h"

LOG_DEFAULT_CAT("khrn_record")

static const char *get_clif_filename(void)
{
   const char *s = khrn_options.autoclif_only_one_clif_name;
   if (s[0] && khrn_options.autoclif_only_one_clif_i >= 0)
      return s;

   {
      // use "rec" for filename if not specified
      s = s[0] ? s : "rec";

      // split filename into base + ext, use .clif if no ext specified
      const char* dot = strrchr(s, '.');
      const char* ext = dot ? dot + 1 : "clif";
      size_t slen = dot ? dot - s : strlen(s);

      static char filename[256];
      size_t offset = VCOS_SAFE_SPRINTF(filename, 0, "%*.*s_%.4u.%s",
         (int)slen, (int)slen, s, khrn_fmem_frame_i, ext);
      demand(offset < sizeof(filename));
      return filename;
   }
}

static void record_bin(const V3D_BIN_RENDER_INFO_T *br_info,
   autoclif_pseudo_ptr_t* pseudo_tile_alloc_ptrs, uint32_t tile_alloc_size)
{
   uint32_t core;
   for (core = 0; core != br_info->num_bins; ++core)
   {
      autoclif_bin_with_pseudo_mem(core,
         br_info->bin_begins[core], br_info->bin_ends[core],
         pseudo_tile_alloc_ptrs[core], tile_alloc_size
         );
   }
}

static void record_render(const V3D_BIN_RENDER_INFO_T *br_info,
   autoclif_pseudo_ptr_t* pseudo_tile_list_base_ptrs)
{
   uint32_t core;
   for (core = 0; core != br_info->num_renders; ++core)
   {
      autoclif_render_with_pseudo_tile_list_bases(core,
         br_info->render_begins[core],
         br_info->render_ends[core],
         br_info->num_bins, pseudo_tile_list_base_ptrs);
   }
}

void khrn_record(const V3D_BIN_RENDER_INFO_T *br_info)
{
   int32_t only = khrn_options.autoclif_only_one_clif_i;
   if ((only < 0) || (only == khrn_fmem_frame_i))
   {
      autoclif_pseudo_ptr_t pseudo_tile_alloc_ptrs[V3D_MAX_CORES];
      unsigned i;

      log_info("recording frame %u", khrn_fmem_frame_i);

      autoclif_begin(&khrn_memaccess_ro
#if !V3D_HAS_NEW_TMU_CFG
         , khrn_get_hw_misccfg()
#endif
         );

      for (i = 0; i != br_info->num_bins; ++i)
      {
         /* bin_offset is to workaround GFXH-1179 */
         char name[32];
         sprintf(name, "tile_alloc_%u", i);

         pseudo_tile_alloc_ptrs[i] = autoclif_new_pseudo_mem(
            name, khrn_options.autoclif_bin_block_size + br_info->bin_offset, V3D_TILE_ALLOC_ALIGN);
         pseudo_tile_alloc_ptrs[i] = autoclif_pseudo_ptr_offset(
            pseudo_tile_alloc_ptrs[i], br_info->bin_offset);
      }

      record_bin(br_info, pseudo_tile_alloc_ptrs, khrn_options.autoclif_bin_block_size);
      record_render(br_info, pseudo_tile_alloc_ptrs);

      autoclif_end(get_clif_filename());

      if (only >= 0)
      {
         log_warn("recorded frame %u, exiting", khrn_fmem_frame_i);
         fflush(NULL);
         _Exit(0);
      }
   }
   else
      log_info("not recording frame %u", khrn_fmem_frame_i);
}

#endif
