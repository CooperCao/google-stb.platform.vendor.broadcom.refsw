/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glxx_hw_render_state.h"

#include "libs/core/v3d/v3d_cl.h"

struct glxx_hw_tile_list_fb_ops
{
   struct v3d_tlb_ldst_params rt_nonms_ls[V3D_MAX_RENDER_TARGETS];
   struct v3d_tlb_ldst_params rt_ms_ls[V3D_MAX_RENDER_TARGETS];
   uint32_t rt_nonms_layer_stride[V3D_MAX_RENDER_TARGETS];
   uint32_t rt_ms_layer_stride[V3D_MAX_RENDER_TARGETS];

   uint32_t rt_clear_mask;
   uint32_t rt_nonms_load_mask, rt_ms_load_mask;
   uint32_t rt_nonms_store_mask, rt_ms_store_mask;

   struct v3d_tlb_ldst_params depth_ls, stencil_ls;
   uint32_t depth_layer_stride, stencil_layer_stride;


   bool stencil_in_depth; /* If true, stencil_ls unused; stencil loaded/stored from/to stencil channel of depth_ls */
   bool depth_clear, stencil_clear;
   bool depth_load, stencil_load;
   bool depth_store, stencil_store;
};

struct glxx_hw_tile_list_rcfg
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   /* if adding more data members, change rcfgs_not_equal*/
   bool early_ds_clear;
#else
   struct v3d_tlb_ldst_params rt_ls[V3D_MAX_RENDER_TARGETS];
   uint32_t rt_store_mask;

   struct v3d_tlb_ldst_params depth_ls, sep_stencil_ls;
   bool sep_stencil, depth_store, stencil_store;
#endif
};

#if V3D_VER_AT_LEAST(4,1,34,0)
static inline bool glxx_hw_tile_list_rcfgs_equal(const struct glxx_hw_tile_list_rcfg *rcfg1,
      const struct glxx_hw_tile_list_rcfg *rcfg2)
{
   return  rcfg1->early_ds_clear == rcfg2->early_ds_clear;
}
#endif

extern bool glxx_hw_prep_tile_list_fb_ops(struct glxx_hw_tile_list_fb_ops *fb_ops,
   GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms);

static inline bool glxx_hw_tile_list_any_loads(const struct glxx_hw_tile_list_fb_ops *fb_ops)
{
   return fb_ops->rt_nonms_load_mask || fb_ops->rt_ms_load_mask ||
      fb_ops->depth_load || fb_ops->stencil_load;
}

extern bool glxx_hw_create_generic_tile_list(v3d_addr_range *range,
   struct glxx_hw_tile_list_rcfg *rcfg,
   GLXX_HW_RENDER_STATE_T *rs, bool tlb_ms, bool double_buffer,
   const struct glxx_hw_tile_list_fb_ops *fb_ops, uint32_t layer);
