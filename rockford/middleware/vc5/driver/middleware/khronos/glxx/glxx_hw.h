/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Hardware abstraction layer API declaration.
Functions common to OpenGL ES 1.1 and OpenGL ES 2.0
=============================================================================*/
#ifndef GLXX_HW_H
#define GLXX_HW_H

#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/egl/egl_context_gl.h"
#include "middleware/khronos/glxx/glxx_inner.h"

typedef struct {
   uint32_t color_buffer_mask;
   /* Raw 32-bit color value, interpreted as int/float depending on buffer format */
   uint32_t color_value[4];
   float    depth_value;
   uint8_t  stencil_value;
   bool     color;
   bool     depth;
   bool     stencil;
} GLXX_CLEAR_T;

typedef struct
{
   v3d_addr_t  shader_record;
   unsigned    num_attributes_hw;
} GLXX_HW_SHADER_RECORD_INFO_T;

typedef struct
{
   v3d_addr_t block_addr;
   bool valid;
} GL20_HW_INDEXED_UNIFORM_T;

typedef struct glxx_hw_compute_uniforms_s
{
   unsigned const* num_work_groups;
   v3d_addr_t shared_ptr;
} glxx_hw_compute_uniforms;

extern bool glxx_hw_clear(GLXX_SERVER_STATE_T *state, GLXX_CLEAR_T *clear);

extern void glxx_get_attribs_live(const GLXX_SERVER_STATE_T *state,
                                  uint32_t *attribs_live);

extern bool glxx_calculate_and_hide(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs, GLXX_PRIMITIVE_T draw_mode);

extern GLXX_LINK_RESULT_DATA_T *glxx_get_shaders(GLXX_SERVER_STATE_T *state);

extern bool glxx_compute_texture_uniforms(GLXX_SERVER_STATE_T *state, KHRN_FMEM_T *fmem);

extern bool glxx_update_tex_unifs(KHRN_FMEM_T *fmem,
                             const GLXX_LINK_RESULT_DATA_T *link_data,
                             GLXX_TEXTURE_UNIF_T *texture_unif);

extern v3d_addr_t glxx_hw_install_uniforms(
   KHRN_FMEM_T                      *fmem,
   const GLXX_SERVER_STATE_T        *state,
   const GLXX_UNIFORM_MAP_T         *map,
   GL20_HW_INDEXED_UNIFORM_T        *iu,
   const glxx_hw_compute_uniforms   *compute_uniforms);

extern bool glxx_hw_draw_triangles(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs,
      const GLXX_DRAW_T *draw,
      GLXX_ATTRIB_CONFIG_T attrib_config[GLXX_CONFIG_MAX_VERTEX_ATTRIBS],
      const GLXX_ATTRIBS_MAX *attribs_max,
      const GLXX_VERTEX_BUFFER_CONFIG_T *vb_config,
      const GLXX_STORAGE_T *indices,
      const GLXX_VERTEX_POINTERS_T *vertex_pointers);

/* return false if we run out of memory;
 * returns true otherwise and have_rs will be true if there is an existing rs
 * for this fb */
extern bool glxx_hw_invalidate_frame(GLXX_SERVER_STATE_T *state, GLXX_FRAMEBUFFER_T *fbo,
                                     bool rt[GLXX_MAX_RENDER_TARGETS],
                                     bool color, bool multisample,
                                     bool depth, bool stencil,
                                     bool *have_rs);

/* similar with glxx_hw_invalidate_frame but it works on the default draw fb */
extern bool glxx_invalidate_default_draw_framebuffer(GLXX_SERVER_STATE_T *state,
                                                     bool color, bool multisample,
                                                     bool depth, bool stencil,
                                                     bool *have_rs);

extern GLXX_HW_RENDER_STATE_T *glxx_install_framebuffer_renderstate(GLXX_SERVER_STATE_T *state);

extern GLXX_HW_RENDER_STATE_T *glxx_find_existing_rs_for_fb(const GLXX_HW_FRAMEBUFFER_T *hw_fb);

/*
 * Get an existing render state if there is one compatible with the given fb or
 * create an new render state, install it as the current render state on the server
 * state and return it.
 * Return NULL if it fails due to out of memory failures
 */
extern GLXX_HW_RENDER_STATE_T* glxx_install_rs(GLXX_SERVER_STATE_T *state,
      GLXX_HW_FRAMEBUFFER_T *fb, bool do_not_flush_existing);

/*
 * If there is an existing render state compatible with the given fb, install
 * it as the current render state on the server state, return true and fill in
 * rs with this render state.
 * Return true and fill in rs with NULL if there is no compatible render state.
 * Return false if it fails due to out of memory failures
 */
extern bool glxx_install_existing_rs(GLXX_SERVER_STATE_T *state,
      GLXX_HW_FRAMEBUFFER_T *fb, GLXX_HW_RENDER_STATE_T **rs);

extern bool glxx_tf_emit_spec(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs,
   uint8_t **instr, bool point_size_used);

uint32_t *glxx_draw_alternate_install_nvshader(KHRN_FMEM_T *fmem, uint32_t shaderSize, uint32_t *shaderCode);

// Prepare fb
extern void glxx_init_hw_framebuffer(GLXX_HW_FRAMEBUFFER_T *hw_fb);

/*
 * Creates a collection of ref counted images from framebuffer attachments.
 * Framebuffer must be complete before calling this function
 */
extern bool glxx_build_hw_framebuffer(const GLXX_FRAMEBUFFER_T *fb,
                                      GLXX_HW_FRAMEBUFFER_T *hw_fb);
extern void glxx_destroy_hw_framebuffer(GLXX_HW_FRAMEBUFFER_T *hw_fb);

extern bool glxx_draw_rect(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs,
      const GLXX_CLEAR_T *clear, int x, int y, int xmax, int ymax);
extern bool glxx_draw_alternate_cle(
      GLXX_HW_RENDER_STATE_T  *rs,
      glxx_dirty_set_t *dirty,
      const GLXX_FRAMEBUFFER_T *fb,
      bool change_color, const glxx_color_write_t *color_write,
      bool change_depth,
      bool change_stencil, uint8_t stencil_value, const struct stencil_mask *stencil_mask,
      int x, int y, int xmax, int ymax,
      bool front_prims, bool back_prims, bool cwise_is_front);

uint32_t glxx_workaround_gfxh_1313_size(void);
bool glxx_workaround_gfxh_1313(uint8_t** instr_ptr, KHRN_FMEM_T* fmem);

uint32_t glxx_workaround_gfxh_1320_size(void);
bool glxx_workaround_gfxh_1320(uint8_t** instr_ptr, KHRN_FMEM_T* fmem);

#endif
