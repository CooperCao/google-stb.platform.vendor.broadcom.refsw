/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Hardware abstraction layer API declaration.
Functions common to OpenGL ES 1.1 and OpenGL ES 2.0
=============================================================================*/
#ifndef GLXX_HW_H
#define GLXX_HW_H

#include "glxx_server.h"
#include "../egl/egl_context_gl.h"
#include "glxx_inner.h"
#include "../gl20/gl20_program.h"

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
   unsigned const* num_work_groups;
   v3d_addr_t shared_ptr;
   unsigned quorum;
} glxx_hw_compute_uniforms;

extern bool glxx_hw_clear(GLXX_SERVER_STATE_T *state, GLXX_CLEAR_T *clear);

extern uint32_t glxx_get_attribs_live(const GLXX_SERVER_STATE_T *state);

extern bool glxx_calculate_and_hide(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs, GLXX_PRIMITIVE_T draw_mode);

extern GLXX_LINK_RESULT_DATA_T *glxx_get_shaders(GLXX_SERVER_STATE_T *state);

extern bool glxx_compute_image_like_uniforms(GLXX_SERVER_STATE_T *state, glxx_render_state *rs);

extern v3d_addr_t glxx_hw_install_uniforms(
   glxx_render_state                *rs,
   const GLXX_SERVER_STATE_T        *state,
   enum shader_mode                  mode,
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
 * state and return it.
 * Return NULL if it fails due to out of memory failures
 */
extern GLXX_HW_RENDER_STATE_T* glxx_install_rs(GLXX_SERVER_STATE_T *state,
      const GLXX_HW_FRAMEBUFFER_T *fb, bool for_tlb_blit);


/*
 * Creates a collection of ref counted images from framebuffer attachments.
 * Framebuffer must be complete before calling this function
 */
extern bool glxx_init_hw_framebuffer(const GLXX_FRAMEBUFFER_T *fb,
                                      GLXX_HW_FRAMEBUFFER_T *hw_fb);
extern void glxx_destroy_hw_framebuffer(GLXX_HW_FRAMEBUFFER_T *hw_fb);

extern void glxx_assign_hw_framebuffer(GLXX_HW_FRAMEBUFFER_T *a, const GLXX_HW_FRAMEBUFFER_T *b);

extern bool glxx_draw_rect(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs,
      const GLXX_CLEAR_T *clear, int x, int y, int xmax, int ymax);

#if !V3D_VER_AT_LEAST(3,3,0,0)

uint32_t glxx_workaround_gfxh_1313_size(void);
bool glxx_workaround_gfxh_1313(uint8_t** instr_ptr, KHRN_FMEM_T* fmem);

uint32_t glxx_workaround_gfxh_1320_size(void);
bool glxx_workaround_gfxh_1320(uint8_t** instr_ptr, KHRN_FMEM_T* fmem);

#endif

#endif
