/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 and 2.0 server-side internal functions
needed by glxx/glxx_server.c, gl11/server.c and gl20/server.c
=============================================================================*/

#ifndef GLXX_SERVER_INTERNAL_H
#define GLXX_SERVER_INTERNAL_H

#include "middleware/khronos/gl20/gl20_program.h" //needed for definition of GL20_PROGRAM_T

extern GLenum glxx_check_framebuffer_status (GLXX_SERVER_STATE_T *state, GLenum target);
extern void glxx_clear_color_internal(float red, float green, float blue, float alpha);
extern void glxx_clear_depth_internal (float depth);
extern int glxx_get_boolean_internal(GLXX_SERVER_STATE_T *state, GLenum pname, GLboolean *params);
extern GLXX_BUFFER_T * glxx_get_bound_buffer(GLXX_SERVER_STATE_T *state, GLenum target, MEM_HANDLE_T *handle);
extern KHRN_IMAGE_FORMAT_T glxx_get_draw_image_format(GLXX_SERVER_STATE_T *state);
extern KHRN_IMAGE_FORMAT_T glxx_get_depth_image_format(GLXX_SERVER_STATE_T *state);
extern int glxx_get_float_internal(GLXX_SERVER_STATE_T *state, GLenum pname, GLfloat *params);
extern int glxx_get_integer_internal(GLXX_SERVER_STATE_T *state, GLenum pname, int *params);
extern uint32_t glxx_get_stencil_size(GLXX_SERVER_STATE_T *state);
extern GLboolean glxx_is_boolean (GLXX_SERVER_STATE_T *state, GLenum pname);
extern GLboolean glxx_is_float (GLXX_SERVER_STATE_T *state, GLenum pname);
extern GLboolean glxx_is_integer (GLXX_SERVER_STATE_T *state, GLenum pname);
extern GLboolean glxx_is_texture_target(GLXX_SERVER_STATE_T *state, GLenum target);
extern void glxx_depth_range_internal(float zNear, float zFar);
extern void glxx_update_color_material(GLXX_SERVER_STATE_T *state);
extern GLboolean glxx_validate_program(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T *program);
extern int glxx_get_float_or_fixed_internal(GLXX_SERVER_STATE_T *state, GLenum pname, float *params);
extern int glxx_get_texparameter_internal(GLenum target, GLenum pname, GLint *params);
extern void glxx_line_width_internal(float width);
extern void glxx_polygon_offset_internal(GLfloat factor, GLfloat units);
extern void glxx_sample_coverage_internal(GLclampf value, GLboolean invert); // S
extern void glxx_texparameter_internal(GLenum target, GLenum pname, const GLint *i);
extern void glxx_update_viewport_internal(GLXX_SERVER_STATE_T *state);

#endif