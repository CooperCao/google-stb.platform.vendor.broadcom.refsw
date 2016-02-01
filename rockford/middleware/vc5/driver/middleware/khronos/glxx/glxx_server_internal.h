/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 and 2.0 server-side internal functions
needed by glxx/glxx_server.c, gl11/server.c and gl20/server.c
=============================================================================*/

#ifndef GLXX_SERVER_INTERNAL_H
#define GLXX_SERVER_INTERNAL_H

#include "helpers/gfx/gfx_lfmt_translate_gl.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "middleware/khronos/glxx/glxx_translate.h"
#include "helpers/gfx/gfx_bufstate.h"

extern void glxx_clear_color_internal(float red, float green, float blue, float alpha);
extern void glxx_clear_depth_internal(float depth);

/*
 * All these store a maximum of MAX_PARAMS items into params (the actual number
 * depends on pname and is what they return).
 */
#define GLXX_GET_MAX_PARAMS (16)
extern int glxx_get_boolean_internal(GLXX_SERVER_STATE_T *state, GLenum pname, GLboolean *params);
extern int glxx_get_float_or_fixed_internal(GLXX_SERVER_STATE_T *state, GLenum pname, float *params);
extern void glxx_get_fixed_internal (GLXX_SERVER_STATE_T *state, GLenum pname, GLfixed *params);
extern uint32_t glxx_get_texparameter_internal(GLXX_SERVER_STATE_T *state, GLenum target, GLenum pname, GLint *params);

extern uint32_t glxx_get_stencil_size(GLXX_SERVER_STATE_T *state);
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
extern bool glxx_is_int_sampler_texparam(GLXX_SERVER_STATE_T *state, GLenum pname);


/* Separated from glxx_pixel_rectangle_get_stride. See comment above that function for more information. */
static inline void glxx_elements_per_group(GLenum format, GLenum type, uint32_t *elements_per_group, uint32_t *bytes_per_element)
{
   uint32_t n = 0, s = 0;

   switch (format) {
   case GL_RGBA:
#if GL_EXT_texture_format_BGRA8888
   case GL_BGRA_EXT:
#endif
#if GL_texture_format_RGBX8888_BRCM
   case GL_RGBX_BRCM:
#endif
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
         UNREACHABLE();
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
         UNREACHABLE();
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
         UNREACHABLE();
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
      UNREACHABLE();
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
   case GFX_LFMT_BSTCYFLIP_RGBA_UNORM: *dst_lfmt = GFX_LFMT_R8_G8_B8_A8_UNORM; break;
   case GFX_LFMT_R8_G8_B8_X8_UNORM: *dst_lfmt = GFX_LFMT_R8_G8_B8_UNORM; break;
   case GFX_LFMT_R8_G8_B8_X8_SNORM: *dst_lfmt = GFX_LFMT_R8_G8_B8_SNORM; break;
   case GFX_LFMT_R8_G8_B8_X8_SRGB: *dst_lfmt = GFX_LFMT_R8_G8_B8_SRGB; break;
   case GFX_LFMT_R16_G16_B16_X16_FLOAT: *dst_lfmt = GFX_LFMT_R16_G16_B16_FLOAT; break;
   case GFX_LFMT_R32_G32_B32_X32_FLOAT: *dst_lfmt = GFX_LFMT_R32_G32_B32_FLOAT; break;
   case GFX_LFMT_R8_G8_B8_X8_INT: *dst_lfmt = GFX_LFMT_R8_G8_B8_INT; break;
   case GFX_LFMT_R8_G8_B8_X8_UINT: *dst_lfmt = GFX_LFMT_R8_G8_B8_UINT; break;
   case GFX_LFMT_R16_G16_B16_X16_INT: *dst_lfmt = GFX_LFMT_R16_G16_B16_INT; break;
   case GFX_LFMT_R16_G16_B16_X16_UINT: *dst_lfmt = GFX_LFMT_R16_G16_B16_UINT; break;
   case GFX_LFMT_R32_G32_B32_X32_INT: *dst_lfmt = GFX_LFMT_R32_G32_B32_INT; break;
   case GFX_LFMT_R32_G32_B32_X32_UINT: *dst_lfmt = GFX_LFMT_R32_G32_B32_UINT; break;
   default: *dst_lfmt = api_fmt;break;
   }

   *dst_lfmt = gfx_lfmt_to_2d_rso(*dst_lfmt);

   gfx_lfmt_to_format_type_maybe(format, type, *dst_lfmt);
}

extern bool glxx_tf_validate_draw_arrays(GLXX_SERVER_STATE_T *state,
   GLenum primitive_mode, GLsizei count, GLsizei instance_count);
extern void glxx_tf_write_primitives(GLXX_SERVER_STATE_T *state,
   v3d_prim_mode_t primitive_mode, GLsizei count, GLsizei instance_count);
extern GLint glxx_tf_get_bound(GLXX_SERVER_STATE_T *state);
extern GLuint glxx_tf_get_bound_buffer(GLXX_SERVER_STATE_T *state);
extern void glxx_tf_delete_buffer(struct GLXX_TRANSFORM_FEEDBACK_T_ *tf, GLXX_BUFFER_T *buffer_obj, GLuint buffer);
extern GLXX_BUFFER_BINDING_T *glxx_tf_get_buffer_binding(GLXX_SERVER_STATE_T *state);
bool glxx_tf_bind_buffer_valid(GLXX_SERVER_STATE_T *state);
GLXX_INDEXED_BINDING_POINT_T *glxx_tf_get_indexed_bindings(GLXX_SERVER_STATE_T *state);
extern void glxx_tf_get_integer64i(const GLXX_SERVER_STATE_T *state, GLenum target, GLuint index, GLint64 *data);
extern void glxx_tf_get_integeri(const GLXX_SERVER_STATE_T *state, GLenum target, GLuint index, GLint *data);
extern int glxx_tf_get_boolean(const GLXX_SERVER_STATE_T *state, GLenum pname, GLboolean *data);
extern bool glxx_tf_add_interlock_writes(const GLXX_SERVER_STATE_T *state, GLXX_HW_RENDER_STATE_T *rs, bool* requires_flush);

extern bool glxx_vao_initialise(GLXX_SERVER_STATE_T *state);
extern void glxx_vao_uninitialise(GLXX_SERVER_STATE_T *state);
GLXX_VAO_T *glxx_get_vao(GLXX_SERVER_STATE_T *state, uint32_t id);

extern GLXX_QUERY_T *glxx_get_query(GLXX_SERVER_STATE_T *state, GLuint id);
extern GLXX_TRANSFORM_FEEDBACK_T *glxx_get_transform_feedback(GLXX_SERVER_STATE_T *state, uint32_t id);

// If offset == -1, do not bind indexed binding point
// If size == -1, use full buffer size, dynamically evaluated each time binding is used
extern GLenum glxx_tf_set_bound_buffer(GLXX_SERVER_STATE_T *state, GLuint name,
   GLuint index, GLintptr offset, GLsizeiptr size);

struct GLXX_TRANSFORM_FEEDBACK_T_; // TODO: Just include the file?
extern struct GLXX_TRANSFORM_FEEDBACK_T_ *glxx_tf_create_default(GLXX_SERVER_STATE_T *state);

struct GL20_PROGRAM_T_; // TODO: Just include the file?
extern bool glxx_tf_program_used(GLXX_SERVER_STATE_T *state, struct GL20_PROGRAM_T_ *program);

extern uint32_t glxx_get_element_count(GLenum type);

extern void glintBindVertexArray(GLuint array);
extern void glintDeleteVertexArrays(GLsizei n, const GLuint* arrays);
extern void glintGenVertexArrays(GLsizei n, GLuint* arrays);
extern GLboolean glintIsVertexArray(GLuint array);

#endif
