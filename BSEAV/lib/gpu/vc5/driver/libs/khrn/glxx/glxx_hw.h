/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glxx_server.h"
#include "glxx_inner.h"
#include "glxx_shader.h"
#include "libs/khrn/common/khrn_mem.h"
#include "../gl20/gl20_program.h"
#include "../egl/egl_context_gl.h"

typedef struct
{
   uint32_t color_buffer_mask;
   bool     depth;
   bool     stencil;
   uint32_t color_value[4]; /* Raw 32-bit color value, interpreted as int/float depending on buffer format */
   float    depth_value;
   uint8_t  stencil_value;
} GLXX_CLEAR_T;

typedef struct
{
   v3d_addr_t block_addr;
   bool valid;
} GL20_HW_INDEXED_UNIFORM_T;

typedef struct glxx_hw_compute_uniforms_s
{
   uint32_t const* num_work_groups;
   bool read_in_preprocess;
   v3d_addr_t shared_ptr;
} glxx_hw_compute_uniforms;

extern bool glxx_hw_clear(GLXX_SERVER_STATE_T *state, GLXX_CLEAR_T *clear);

extern uint32_t glxx_get_attribs_live(const GLXX_SERVER_STATE_T *state);

typedef struct
{
   GLXX_TEXTURE_UNIF_T tex[GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
   GLXX_TEXTURE_UNIF_T img[GLXX_CONFIG_MAX_IMAGE_UNITS];
} glxx_hw_image_like_uniforms;

extern bool glxx_compute_image_like_uniforms(
   GLXX_SERVER_STATE_T *state, glxx_render_state *rs,
   glxx_hw_image_like_uniforms *image_like_uniforms, GLSL_BACKEND_CFG_T *key);

extern GLXX_LINK_RESULT_DATA_T *glxx_get_shaders(
   GLXX_SERVER_STATE_T *state, const GLSL_BACKEND_CFG_T *key);

extern v3d_addr_t glxx_hw_install_uniforms(
   glxx_render_state                 *rs,
   const GLXX_SERVER_STATE_T         *state,
   enum shader_mode                   mode,
   const GLXX_UNIFORM_MAP_T          *map,
   GL20_HW_INDEXED_UNIFORM_T         *iu,
   const glxx_hw_image_like_uniforms *image_like_uniforms,
   const glxx_hw_compute_uniforms    *compute_uniforms);

typedef struct
{
   GLXX_PRIMITIVE_T mode;

   size_t count; /* Unused if is_indirect */
   size_t instance_count; /* Unused if is_indirect */

   bool is_draw_arrays;
   /* is_draw_arrays only */
   unsigned int first; /* Unused if is_indirect */
   /* !is_draw_arrays only */
   GLXX_INDEX_T index_type;

   int32_t basevertex;
   uint32_t baseinstance;

   bool is_indirect;
   /* is_indirect only */
   unsigned int num_indirect;
   v3d_size_t indirect_stride;
   v3d_size_t indirect_offset;
} glxx_hw_draw;

typedef struct
{
   v3d_addr_t addr;
   v3d_size_t offset;
   v3d_size_t size;
} glxx_hw_indices;

typedef struct
{
   v3d_addr_t addr;
   uint32_t stride;
   uint32_t divisor;
#if V3D_HAS_ATTR_MAX_INDEX
   uint32_t max_index;
#endif
} glxx_hw_vb;

extern bool glxx_hw_draw_triangles(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs,
      const glxx_hw_draw *draw,
      const glxx_hw_indices *indices,
      const GLXX_ATTRIB_CONFIG_T *attribs, const glxx_hw_vb *vbs
#if !V3D_HAS_ATTR_MAX_INDEX
      , const glxx_attribs_max *attribs_max
#endif
      );

/* RT i's multisample color buffer will be invalidated if either rt & (1 << i)
 * *or* all_color_ms is true.
 * RT i's non-multisample color buffer will be invalidated iff rt & (1 << i). */
extern void glxx_hw_invalidate_framebuffer(
   GLXX_SERVER_STATE_T *state, GLXX_FRAMEBUFFER_T *fb,
   uint32_t rt, bool all_color_ms, bool depth, bool stencil);

/* similar to glxx_hw_invalidate_framebuffer but it works on the default draw fb */
extern void glxx_hw_invalidate_default_draw_framebuffer(
   GLXX_SERVER_STATE_T *state,
   bool color, bool color_ms, bool depth, bool stencil);

extern GLXX_HW_RENDER_STATE_T *glxx_find_existing_rs_for_fb(const GLXX_HW_FRAMEBUFFER_T *hw_fb);

/*
 * Get an existing render state if there is one compatible with the given fb or
 * create an new render state, install it as the current render state on the server
 * state and return it. *ret_existing is false if this returns a new render-state.
 * Return NULL if it fails due to out of memory failures
 */
extern GLXX_HW_RENDER_STATE_T* glxx_install_rs(GLXX_SERVER_STATE_T *state,
      const GLXX_HW_FRAMEBUFFER_T *fb, bool* ret_existing, bool for_tlb_blit);


/*
 * Creates a collection of ref counted images from framebuffer attachments.
 * Important: Framebuffer must be complete before calling this function
 */
extern bool glxx_init_hw_framebuffer(const GLXX_FRAMEBUFFER_T *fb,
                                      GLXX_HW_FRAMEBUFFER_T *hw_fb,
                                      glxx_context_fences *fences);
extern void glxx_destroy_hw_framebuffer(GLXX_HW_FRAMEBUFFER_T *hw_fb);

extern void glxx_assign_hw_framebuffer(GLXX_HW_FRAMEBUFFER_T *a, const GLXX_HW_FRAMEBUFFER_T *b);

extern bool glxx_draw_rect(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs,
      const GLXX_CLEAR_T *clear, const glxx_rect *rect);

uint32_t *glxx_draw_rect_vertex_data(uint32_t *vdata_max_index, khrn_fmem *fmem,
      const glxx_rect *rect, uint32_t z);

#if !V3D_VER_AT_LEAST(3,3,0,0)
uint32_t glxx_workaround_gfxh_1313_size(void);
bool glxx_workaround_gfxh_1313(uint8_t** instr_ptr, khrn_fmem* fmem,
   uint32_t fb_width, uint32_t fb_height);
#endif

#if !V3D_HAS_GFXH1636_FIX
uint32_t glxx_fill_ocq_cache_size(void);
bool glxx_fill_ocq_cache(uint8_t** instr_ptr, khrn_fmem* fmem,
   uint32_t fb_width, uint32_t fb_height);
#endif

extern bool glxx_hw_tf_aware_sync_res(GLXX_HW_RENDER_STATE_T *rs,
   khrn_resource *res, v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags);

typedef struct glxx_hw_ubo_load_batch
{
   khrn_mem_handle_t uniform_map;
   uint32_t* dst_ptr;
   uint32_t const* src_ptr;
   glxx_shader_ubo_load const* loads;
   uint32_t num_loads;
} glxx_hw_ubo_load_batch;

static inline void glxx_hw_process_ubo_load_batch(glxx_hw_ubo_load_batch const* batch, bool release_uniform_map)
{
   glxx_shader_process_ubo_loads(
      batch->dst_ptr,
      batch->src_ptr,
      batch->loads,
      batch->num_loads);
   if (release_uniform_map)
      khrn_mem_release(batch->uniform_map);
}
