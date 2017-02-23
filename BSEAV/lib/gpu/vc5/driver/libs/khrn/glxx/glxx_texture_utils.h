
/*=============================================================================
  Broadcom Proprietary and Confidential. (c)2013 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
   Helper functions for texture
=============================================================================*/
#ifndef GLXX_TEXTURE_UTILS_H
#define GLXX_TEXTURE_UTILS_H

#include "gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "libs/core/lfmt/lfmt.h"
#include "libs/core/gfx_buffer/gfx_buffer.h"
#include "glxx_pixel_store.h"
#include "glxx_buffer.h"
#include "glxx_server_state.h"

enum glxx_tex_target;

/* maximum mip_level allowed in es3.0 based on the texture target type*/
extern bool glxx_texture_is_legal_level(GLenum target, int level);

/* maximum width, height and depth in es3.0 based on the texture target type*/
extern bool glxx_texture_are_legal_dimensions(GLenum target, int w, int h,
      int d);

extern bool glxx_texture_is_legal_layer(GLenum target, int layer);

/* returns true if this target is a valid texture target (as in enum
 * glxx_tex_target), for a certain es version */
extern bool glxx_texture_is_tex_target(const GLXX_SERVER_STATE_T *state, GLenum target);

extern uint32_t glxx_texture_get_face(GLenum target);
extern bool glxx_texture_is_cube_face(GLenum target);

/* adds dimension to the lfmts, for num_planes */
extern void  glxx_lfmt_add_dim(GFX_LFMT_T *lfmts, uint32_t num_planes,
      uint32_t dims);

extern bool glxx_is_texture_filterable_api_fmt(GFX_LFMT_T api_fmt);

void glxx_hw_fmts_from_api_fmt(
   uint32_t *num_planes,
   GFX_LFMT_T hw_fmts[GFX_BUFFER_MAX_PLANES],
   GFX_LFMT_T api_fmt);

static inline bool glxx_tex_target_is_1d(GLenum target)
{
   return (target == GL_TEXTURE_1D_BRCM) || (target == GL_TEXTURE_1D_ARRAY_BRCM);
}

extern bool glxx_tex_target_valid_in_es1(GLenum target);

extern bool glxx_tex_target_is_multisample(GLenum target);

extern bool glxx_tex_target_has_layers(enum glxx_tex_target target);
extern bool glxx_tex_target_has_layers_or_faces(enum glxx_tex_target target);

extern void glxx_choose_copy_format_from_unsized_internalformat(GLenum
      internalformat, GFX_LFMT_T src_lfmt, GFX_LFMT_T
      dst_fmts[GFX_BUFFER_MAX_PLANES], uint32_t *num_planes);

/* having rectangle(0, 0, src_width, src_height) and rectangle(x, y, width,
 * height) store in x, y, width, height the intersection of these 2 rectangles
 * and adjust the origin of the destination accordingly. This is used in
 * general for copying e rectangle from a source to a destination. Pixels
 * outside the source will be undefined in the destination rectangle.
 */
extern void glxx_clamp_rect(int src_width, int src_height, int *x, int
      *y, int *width, int *height, int *dst_x, int *dst_y);

struct glxx_pixels_info
{
   size_t offset;
   size_t width;
   size_t stride;
   size_t slice_pitch;
};

void glxx_get_pack_unpack_info(const GLXX_PIXEL_STORE_STATE_T
      *pack_unpack, bool for_pack, unsigned width, unsigned height, GLenum fmt,
      GLenum type, struct glxx_pixels_info *info);

/* Check that if we treat pixel_buffer as a buffer described by desc, and we
 * want to copy data from/to pixel buffer starting from offset_1 + offset_2,
 * and we want to copy num_array_elems, where each elem is of size array_pitch,
 * we are not accesing memory beyond the size of the pixel buffer.
 * If num_array_elems is 1, src_array_pitch is disregarded.
 * Return true if the access is inside the pixel buffer size.
 * Note: we have two offsets because we emulate the gl api where we have 2
 * offsets, one provided by the user and one by setting
 * UNPACK_SKIP_PIXELS/SKIP_IMAGES, etc.
 */
extern bool glxx_check_buffer_valid_access(const GLXX_BUFFER_T *pixel_buffer,
      const GFX_BUFFER_DESC_T *desc, size_t offset_1, size_t offset_2,
      unsigned num_array_elems, size_t array_pitch);
#endif
