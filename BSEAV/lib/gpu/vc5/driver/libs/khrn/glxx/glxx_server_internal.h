/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_SERVER_INTERNAL_H
#define GLXX_SERVER_INTERNAL_H

#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "../common/khrn_mem.h"
#include "glxx_translate.h"
#include "glxx_tf.h"

extern void glxx_clear_color_internal(float red, float green, float blue, float alpha);
extern void glxx_clear_depth_internal(float depth);

GLenum glxx_get_booleans(const GLXX_SERVER_STATE_T *state, GLenum pname, GLboolean *params);
#if V3D_VER_AT_LEAST(3,3,0,0)
GLenum glxx_get_booleans_i(const GLXX_SERVER_STATE_T *state, GLenum pname, GLuint index, GLboolean *params);
#endif
GLenum glxx_get_fixeds(const GLXX_SERVER_STATE_T *state, GLenum pname, GLfixed *params);
extern uint32_t glxx_get_texparameter_internal(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname, GLint *params);

extern uint32_t glxx_get_stencil_size(GLXX_SERVER_STATE_T const* state);
extern void glxx_depth_range_internal(float zNear, float zFar);
extern void glxx_update_color_material(GLXX_SERVER_STATE_T *state);
extern void glxx_line_width_internal(float width);
extern void glxx_polygon_offset_internal(GLfloat factor, GLfloat units);
extern void glxx_sample_coverage_internal(GLclampf value, GLboolean invert); // S
extern void glxx_texparameter_internal(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname, const GLint *i);
extern bool glxx_is_int_texparam(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname);

extern bool glxx_valid_draw_frame_buffer(GLXX_SERVER_STATE_T *state);
extern bool glxx_valid_read_frame_buffer(GLXX_SERVER_STATE_T *state);
extern bool glxx_is_float_texparam(GLenum pname);
extern bool glxx_is_vector_texparam(GLenum pname);
extern bool glxx_is_int_sampler_texparam(GLXX_SERVER_STATE_T *state, GLenum pname);


/* Separated from glxx_pixel_rectangle_get_stride. See comment above that function for more information. */
static inline void glxx_elements_per_group(GLenum format, GLenum type, uint32_t *elements_per_group, uint32_t *bytes_per_element)
{
   uint32_t n = 0, s = 0;

   switch (format) {
   case GL_RGBA:
   case GL_BGRA_EXT:
   case GL_RGBA_INTEGER:
      switch (type) {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
         n = 4;
         s = 1;
         break;
      case GL_UNSIGNED_SHORT_4_4_4_4:
      case GL_UNSIGNED_SHORT_5_5_5_1:
         n = 1;
         s = 2;
         break;
      case GL_UNSIGNED_INT_2_10_10_10_REV:
         n = 1;
         s = 4;
         break;
      case GL_FLOAT:
      case GL_UNSIGNED_INT:
      case GL_INT:
         n = 4;
         s = 4;
         break;
      case GL_HALF_FLOAT:
      case GL_UNSIGNED_SHORT:
      case GL_SHORT:
         n = 4;
         s = 2;
         break;
      default:
         unreachable();
         break;
      }
      break;
   case GL_RGB:
   case GL_RGB_INTEGER:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         n = 3;
         s = 1;
         break;
      case GL_UNSIGNED_SHORT_5_6_5:
         n = 1;
         s = 2;
         break;
      case GL_UNSIGNED_INT_10F_11F_11F_REV:
      case GL_UNSIGNED_INT_5_9_9_9_REV:
         n = 1;
         s = 4;
         break;
      case GL_FLOAT:
      case GL_UNSIGNED_INT:
      case GL_INT:
         n = 3;
         s = 4;
         break;
      case GL_HALF_FLOAT:
      case GL_UNSIGNED_SHORT:
      case GL_SHORT:
         n = 3;
         s = 2;
         break;
      case GL_BYTE:
         n = 3;
         s = 1;
         break;
      default:
         unreachable();
         break;
      }
      break;
   case GL_DEPTH_STENCIL:
      switch (type) {
      case GL_UNSIGNED_INT_24_8:
         n = 1;
         s = 4;
         break;
      case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
         n = 2; /* TODO: I'm not sure if these are right */
         s = 4;
         break;
      default:
         unreachable();
      }
      break;
   case GL_RG:
   case GL_RG_INTEGER:
      switch (type) {
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
         n = 2;
         s = 1;
         break;
      case GL_FLOAT:
      case GL_UNSIGNED_INT:
      case GL_INT:
         n = 2;
         s = 4;
         break;
      case GL_HALF_FLOAT:
      case GL_UNSIGNED_SHORT:
      case GL_SHORT:
         n = 2;
         s = 2;
         break;
      default: unreachable();
      }
      break;
   case GL_RED:
   case GL_RED_INTEGER:
   case GL_DEPTH_COMPONENT:
      switch (type) {
      case GL_FLOAT:
      case GL_UNSIGNED_INT:
      case GL_INT:
         n = 1;
         s = 4;
         break;
      case GL_HALF_FLOAT:
      case GL_UNSIGNED_SHORT:
      case GL_SHORT:
         n = 1;
         s = 2;
         break;
      case GL_UNSIGNED_BYTE:
      case GL_BYTE:
         n = 1;
         s = 1;
         break;
      default: unreachable();
      }
      break;
   case GL_LUMINANCE_ALPHA:
      n = 2;
      s = 1;
      break;
   case GL_LUMINANCE:
   case GL_ALPHA:
      n = 1;
      s = 1;
      break;
   case GL_STENCIL_INDEX:
      switch(type)
      {
      case GL_UNSIGNED_BYTE:
         n = 1;
         s = 1;
         break;
      default: unreachable();
      }
      break;
   default:
      unreachable();
      break;
   }

   *elements_per_group = n;
   *bytes_per_element  = s;
}

/* (section 3.6 of the OpenGL ES 1.1 spec)
   the first element of the Nth row is indicated by  p + Nk
   where N is the row number (counting from zero) and k is defined as
   k = nl                  if s >= align
   = align/s * ceil(snl/align)   otherwise
   where n is the number of elements in a group, l is the number of groups in
   the row, align is the value of UNPACK ALIGNMENT, and s is the size, in units of GL
   ubytes, of an element.

   TODO maybe gfx_buffer fmt_detail could do this?
   */
static inline uint32_t glxx_pixel_rectangle_get_stride(uint32_t l, GLenum
      format, GLenum type, uint32_t align)
{
   uint32_t n = 0, s = 0, k;

   glxx_elements_per_group(format, type, &n, &s);

   if (s < align)
      k = align * ((s * n * l + align - 1) / align);
   else
      k = s * n * l;

   return k;
}

/*
return (format,type,lfmt) of the destination rectangle that glReadPixels will
write to, when the framebuffer is fb_lfmt.

This function covers the first glReadPixels option in the ES3 spec:

"""
The first varies depending on the format of the currently bound rendering
surface.

- For normalized fixed-point rendering surfaces, the combination format RGBA
  and type UNSIGNED_BYTE is accepted.
- For signed integer rendering surfaces, the combination format RGBA_INTEGER
  and type INT is accepted.
- For unsigned integer rendering surfaces, the combination format RGBA_INTEGER
  and type UNSIGNED_INT is accepted.
"""

(and, tentatively, for a future extension:
- For floating-point rendering surfaces, the combination format RGBA and type
  FLOAT is accepted.
)

*/
static inline void glxx_readpixels_std_formats(GLenum *format, GLenum *type,
      GFX_LFMT_T *dst_lfmt, GFX_LFMT_T fb_lfmt)
{
   bool is_srgb = false;

   switch (gfx_lfmt_get_type(&fb_lfmt))
   {
   case GFX_LFMT_TYPE_SRGB:
   case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
      is_srgb = true;
      /* no break */
   case GFX_LFMT_TYPE_UNORM:
      *format = GL_RGBA;
      *type = GL_UNSIGNED_BYTE;
      break;
   case GFX_LFMT_TYPE_UINT:
      *format = GL_RGBA_INTEGER;
      *type = GL_UNSIGNED_INT;
      break;
   case GFX_LFMT_TYPE_INT:
      *format = GL_RGBA_INTEGER;
      *type = GL_INT;
      break;
   case GFX_LFMT_TYPE_FLOAT:
   case GFX_LFMT_TYPE_UFLOAT:
      *format = GL_RGBA;
      *type = GL_FLOAT;
      break;
   default: unreachable();
   }

   *dst_lfmt = gfx_lfmt_from_format_type(*format, *type, is_srgb);
   *dst_lfmt = gfx_lfmt_to_2d_rso(*dst_lfmt);
}

/*
As glxx_readpixels_std_formats, but for the implementation-defined option of
glReadPixels.

ES3 spec: "The second is an implementation-chosen format [...]"

We choose to return a format and type that matches the API parameters it was
created with (e.g. the GL internalformat, EGL attributes, etc). This should
require no conversion of the pixel format, except where the hw-lfmt of the
framebuffer has padding channels, like R8_G8_B8_X8_UNORM, in which case we need
to remove the padding.

The above strategy also covers the third glReadPixels option in the spec:

"Additionally, when the internal format of the rendering surface is RGB10_A2, a
third combination of format RGBA and type UNSIGNED_INT_2_10_10_10_REV is
accepted."
*/
static inline void glxx_readpixels_impldefined_formats(GLenum *format, GLenum *type, GFX_LFMT_T *dst_lfmt,
   GFX_LFMT_T api_fmt)
{
   switch(api_fmt)
   {
   case GFX_LFMT_BSTC_RGBA_UNORM:
   case GFX_LFMT_BSTCYFLIP_RGBA_UNORM:  *dst_lfmt = GFX_LFMT_R8_G8_B8_A8_UNORM; break;
   case GFX_LFMT_R8_G8_B8_X8_UNORM:     *dst_lfmt = GFX_LFMT_R8_G8_B8_UNORM;    break;
   case GFX_LFMT_R8_G8_B8_X8_SNORM:     *dst_lfmt = GFX_LFMT_R8_G8_B8_SNORM;    break;
   case GFX_LFMT_R8_G8_B8_X8_SRGB:      *dst_lfmt = GFX_LFMT_R8_G8_B8_SRGB;     break;
   case GFX_LFMT_X4B4G4R4_UNORM:        *dst_lfmt = GFX_LFMT_A4B4G4R4_UNORM;    break;
   case GFX_LFMT_X1B5G5R5_UNORM:        *dst_lfmt = GFX_LFMT_A1B5G5R5_UNORM;    break;
   case GFX_LFMT_R16_G16_B16_X16_FLOAT: *dst_lfmt = GFX_LFMT_R16_G16_B16_FLOAT; break;
   case GFX_LFMT_R32_G32_B32_X32_FLOAT: *dst_lfmt = GFX_LFMT_R32_G32_B32_FLOAT; break;
   case GFX_LFMT_R8_G8_B8_X8_INT:       *dst_lfmt = GFX_LFMT_R8_G8_B8_INT;      break;
   case GFX_LFMT_R8_G8_B8_X8_UINT:      *dst_lfmt = GFX_LFMT_R8_G8_B8_UINT;     break;
   case GFX_LFMT_R16_G16_B16_X16_INT:   *dst_lfmt = GFX_LFMT_R16_G16_B16_INT;   break;
   case GFX_LFMT_R16_G16_B16_X16_UINT:  *dst_lfmt = GFX_LFMT_R16_G16_B16_UINT;  break;
   case GFX_LFMT_R32_G32_B32_X32_INT:   *dst_lfmt = GFX_LFMT_R32_G32_B32_INT;   break;
   case GFX_LFMT_R32_G32_B32_X32_UINT:  *dst_lfmt = GFX_LFMT_R32_G32_B32_UINT;  break;
   default: *dst_lfmt = api_fmt;break;
   }

   *dst_lfmt = gfx_lfmt_to_2d_rso(*dst_lfmt);

   gfx_lfmt_to_format_type(format, type, *dst_lfmt);
}

extern bool glxx_vao_initialise(GLXX_SERVER_STATE_T *state);
extern void glxx_vao_uninitialise(GLXX_SERVER_STATE_T *state);
GLXX_VAO_T *glxx_get_vao(GLXX_SERVER_STATE_T *state, uint32_t id);

extern bool glxx_pipeline_state_initialise(GLXX_SERVER_STATE_T *state);
extern void glxx_pipeline_state_term(GLXX_SERVER_STATE_T *state);

extern GLXX_QUERY_T *glxx_get_query(GLXX_SERVER_STATE_T *state, GLuint id);
extern bool glxx_server_has_active_query_type(enum glxx_query_type type,
   const GLXX_SERVER_STATE_T *state, const GLXX_HW_RENDER_STATE_T *rs);

extern uint32_t glxx_get_element_count(GLenum type);

extern void glintBindVertexArray(GLuint array);
extern void glintDeleteVertexArrays(GLsizei n, const GLuint* arrays);
extern void glintGenVertexArrays(GLsizei n, GLuint* arrays);
extern GLboolean glintIsVertexArray(GLuint array);

extern bool glxx_in_secure_context();

/* TF functions */
extern GLXX_TRANSFORM_FEEDBACK_T *glxx_get_transform_feedback(GLXX_SERVER_STATE_T *state, uint32_t id);
extern GLXX_TRANSFORM_FEEDBACK_T* glxx_get_bound_tf(GLXX_SERVER_STATE_T const* state);
extern bool glxx_server_program_used_by_any_tf(GLXX_SERVER_STATE_T *state,
      struct GL20_PROGRAM_T_ *program);
extern bool glxx_server_tf_install(GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs,
      bool point_size_used);
extern bool glxx_server_tf_install_post_draw(GLXX_SERVER_STATE_T *state,
      GLXX_HW_RENDER_STATE_T *rs);

#endif
