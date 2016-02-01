/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 and 2.0 client-side attribute information structure declarations.
=============================================================================*/

#ifndef GLXX_INT_ATTRIB_H
#define GLXX_INT_ATTRIB_H

#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES2/gl2.h"

#include <stddef.h>

typedef struct {
   GLboolean enabled;

   GLint size;
   GLenum type;
   GLboolean normalized;
   GLsizei stride;

   const GLvoid *pointer;

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

#endif
