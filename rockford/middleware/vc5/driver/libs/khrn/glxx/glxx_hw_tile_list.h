/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "glxx_hw_render_state.h"

#include "libs/core/v3d/v3d_cl.h"

struct glxx_hw_tile_list_fb_ops
{
   struct v3d_tlb_ldst_params rt_nonms_ls[GLXX_MAX_RENDER_TARGETS];
   struct v3d_tlb_ldst_params rt_ms_ls[GLXX_MAX_RENDER_TARGETS];
   uint32_t rt_clear_mask;
   uint32_t rt_nonms_load_mask, rt_ms_load_mask;
   uint32_t rt_nonms_store_mask, rt_ms_store_mask;

   struct v3d_tlb_ldst_params depth_ls, stencil_ls;
   bool stencil_in_depth; /* If true, stencil_ls unused; stencil loaded/stored from/to stencil channel of depth_ls */
   bool depth_clear, stencil_clear;
   bool depth_load, stencil_load;
   bool depth_store, stencil_store;
};

struct glxx_hw_tile_list_rcfg
{
#if V3D_HAS_NEW_TLB_CFG
   bool early_ds_clear;
#else
   struct v3d_tlb_ldst_params rt_ls[GLXX_MAX_RENDER_TARGETS];
   uint32_t rt_store_mask;

   struct v3d_tlb_ldst_params depth_ls, sep_stencil_ls;
   bool sep_stencil, depth_store, stencil_store;
#endif
};

extern bool glxx_hw_prep_tile_list_fb_ops(struct glxx_hw_tile_list_fb_ops *fb_ops,
   GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms);

static inline bool glxx_hw_tile_list_any_loads(const struct glxx_hw_tile_list_fb_ops *fb_ops)
{
   return fb_ops->rt_nonms_load_mask || fb_ops->rt_ms_load_mask ||
      fb_ops->depth_load || fb_ops->stencil_load;
}

extern bool glxx_hw_create_generic_tile_list(v3d_addr_t addrs[2],
   struct glxx_hw_tile_list_rcfg *rcfg,
   GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms, bool double_buffer,
   const struct glxx_hw_tile_list_fb_ops *fb_ops);
