/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#pragma once

#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES2/gl2.h"
#include "middleware/khronos/glxx/glxx_buffer.h"

#include <stddef.h>
#include <stdint.h>

typedef struct {
   GLboolean enabled;

   GLint size;
   GLenum type;
   GLboolean normalized;
   GLsizei stride;

   const GLvoid *pointer;
   uintptr_t offset;
   GLXX_BUFFER_T *attrib;
   GLuint buffer;

   GLfloat value[4];
} GLXX_ATTRIB_T;

/* GL 1.1 specific For indexing into arrays of handles/pointers */
#define GL11_IX_COLOR 1//0
#define GL11_IX_NORMAL 2//1
#define GL11_IX_VERTEX 0//2
#define GL11_IX_TEXTURE_COORD 3
#define GL11_IX_POINT_SIZE 7
#define GL11_IX_MAX_ATTRIBS 8
