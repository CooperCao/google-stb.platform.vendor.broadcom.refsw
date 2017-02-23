/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009-2013 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  autoclif recording interface

FILE DESCRIPTION
Drivers use this interface to trigger autoclif recording
=============================================================================*/

#include "khrn_record.h"

#if KHRN_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include "libs/util/log/log.h"
#include "libs/platform/gmem.h"
#include "libs/util/demand.h"
#include "khrn_options.h"
#include "khrn_memaccess.h"
#include "khrn_record.h"
#include "libs/tools/autoclif/autoclif.h"
#include "libs/sim/simcom/simcom_memaccess.h"

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
#if !V3D_VER_AT_LEAST(4,0,2,0)
      /* Not supported by CLIF, HW doesn't work anyway */
      assert(br_info->empty_tile_mode != V3D_EMPTY_TILE_MODE_FILL);
#endif
      autoclif_render_with_pseudo_tile_list_bases(core,
         br_info->render_begins[core],
         br_info->render_ends[core],
         br_info->num_bins, pseudo_tile_list_base_ptrs
#if !V3D_VER_AT_LEAST(4,0,2,0)
         , br_info->empty_tile_mode == V3D_EMPTY_TILE_MODE_SKIP
#endif
         );
   }
}

void khrn_record(khrn_memaccess* ma, const V3D_BIN_RENDER_INFO_T *br_info)
{
   int32_t only = khrn_options.autoclif_only_one_clif_i;
   if (((only < 0) || (only == khrn_fmem_frame_i)) &&
       br_info->num_bins > 0)    /* Workaround broken compute shader recording */
   {
      autoclif_pseudo_ptr_t pseudo_tile_alloc_ptrs[V3D_MAX_CORES];

      log_info("recording frame %u", khrn_fmem_frame_i);

      khrn_memaccess_pre_read(ma);
      struct simcom_memaccess memaccess_ro = { 0, };
      memaccess_ro.read = khrn_memaccess_read_fn;
      memaccess_ro.p = ma;

      autoclif_begin(&memaccess_ro,
#if !V3D_VER_AT_LEAST(4,0,2,0)
         khrn_get_hw_misccfg(),
#endif
         /* Need at least 4 for compute with workgroup size of 128 to work.
          * Need at least 2 for worst-case tessellation control shader with
          * barriers to work. */
         /*min_qpus_per_core=*/br_info->num_bins ? (V3D_VER_AT_LEAST(4,0,2,0) ? 2 : 1) : 4);

      for (unsigned i = 0; i != br_info->num_bins; ++i)
      {
         char name[32];
         sprintf(name, "tile_alloc_%u", i);

         pseudo_tile_alloc_ptrs[i] = autoclif_new_pseudo_mem(
            name, khrn_options.autoclif_bin_block_size, V3D_TILE_ALLOC_ALIGN);
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
