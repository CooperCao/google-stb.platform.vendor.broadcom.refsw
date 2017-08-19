/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_gen.h"

EXTERN_C_BEGIN

static inline bool v3d_tlb_rt_type_use_rw_cfg_16(v3d_rt_type_t rt)
{
   switch (rt) {
#if V3D_HAS_TLB_WR_CONVERT
      case V3D_RT_TYPE_8:
      case V3D_RT_TYPE_8I:
      case V3D_RT_TYPE_8UI:
      case V3D_RT_TYPE_16F:
      case V3D_RT_TYPE_16I:
      case V3D_RT_TYPE_16UI:
         return true;

      case V3D_RT_TYPE_32F:
      case V3D_RT_TYPE_32I:
      case V3D_RT_TYPE_32UI:
         return false;
#else
      case V3D_RT_TYPE_8:
      case V3D_RT_TYPE_16F:
         return true;

      case V3D_RT_TYPE_8I:
      case V3D_RT_TYPE_8UI:
      case V3D_RT_TYPE_16I:
      case V3D_RT_TYPE_16UI:
      case V3D_RT_TYPE_32F:
      case V3D_RT_TYPE_32I:
      case V3D_RT_TYPE_32UI:
         return false;
#endif

      default:
         unreachable();
         return false;
   }
}

static inline bool v3d_tlb_rt_type_is_int(v3d_rt_type_t rt)
{
   switch (rt) {
      case V3D_RT_TYPE_8I:
      case V3D_RT_TYPE_8UI:
      case V3D_RT_TYPE_16I:
      case V3D_RT_TYPE_16UI:
      case V3D_RT_TYPE_32I:
      case V3D_RT_TYPE_32UI:
         return true;

      case V3D_RT_TYPE_8:
      case V3D_RT_TYPE_16F:
      case V3D_RT_TYPE_32F:
         return false;

      default:
         unreachable();
         return false;
   }
}

static inline uint8_t v3d_tlb_config_color(unsigned rt, bool is_16, bool is_int, uint32_t vec_sz, bool per_sample)
{
#if !V3D_HAS_TLB_WR_CONVERT
   assert(!is_16 || !is_int);
#endif

   if (is_16) {
      V3D_TLB_CONFIG_COLOR_16_T cfg;
      cfg.num_words = vec_sz;
      cfg.no_swap = true;
      cfg.all_samples_same_data = !per_sample;
      cfg.rt = rt;
      return v3d_pack_tlb_config_color_16(&cfg);
   } else {
      V3D_TLB_CONFIG_COLOR_32_T cfg;
      cfg.num_words = vec_sz;
      cfg.all_samples_same_data = !per_sample;
      cfg.rt = rt;
#if !V3D_HAS_TLB_WR_CONVERT
      cfg.as_int = is_int;
#endif
      return v3d_pack_tlb_config_color_32(&cfg);
   }
}

EXTERN_C_END
