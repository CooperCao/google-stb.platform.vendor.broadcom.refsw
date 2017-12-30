/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_CLIENT_H
#define GLXX_CLIENT_H

#include "../gl11/gl11_int_config.h"
#include "glxx_int_config.h"

#include "../egl/egl_thread.h"
#include "../egl/egl_context_gl.h"
#include "glxx_buffer.h"

static inline bool glxx_is_aligned(GLenum type, size_t value)
{
   switch (type) {
   case GL_BYTE:
   case GL_UNSIGNED_BYTE:
      return GL_TRUE;
   case GL_SHORT:
   case GL_UNSIGNED_SHORT:
      return (value & 1) == 0;
   case GL_UNSIGNED_INT:
   case GL_FIXED:
   case GL_FLOAT:
      return (value & 3) == 0;
   default:
      unreachable();
      return GL_FALSE;
   }
}
static inline bool glxx_is_shader_type(GLenum type)
{
   return (type == GL_VERTEX_SHADER)   ||
          (type == GL_FRAGMENT_SHADER) ||
          (type == GL_COMPUTE_SHADER && V3D_VER_AT_LEAST(3,3,0,0)) ||
          (type == GL_TESS_CONTROL_SHADER && (KHRN_GLES32_DRIVER || V3D_VER_AT_LEAST(4,0,2,0))) ||
          (type == GL_TESS_EVALUATION_SHADER && (KHRN_GLES32_DRIVER || V3D_VER_AT_LEAST(4,0,2,0))) ||
          (type == GL_GEOMETRY_SHADER && (KHRN_GLES32_DRIVER || V3D_VER_AT_LEAST(4,0,2,0)));
}

static inline bool glxx_is_alignment(GLint param)
{
   return param == 1 ||
          param == 2 ||
          param == 4 ||
          param == 8;
}

/* Fake GL API calls */
void glintAttribPointer_GL11(GLXX_SERVER_STATE_T *state, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr ptr);
void *glintAttribGetPointer(GLXX_SERVER_STATE_T *state, uint32_t indx);

void glintColor(float red, float green, float blue, float alpha);

void glintAttribEnable(GLXX_SERVER_STATE_T *state, uint32_t indx, bool enabled);

void glintAttrib(uint32_t api, uint32_t indx, float x, float y, float z, float w);
void glintAttribI(uint32_t api, uint32_t indx, uint32_t x, uint32_t y, uint32_t z, uint32_t w, bool is_signed);
#endif
