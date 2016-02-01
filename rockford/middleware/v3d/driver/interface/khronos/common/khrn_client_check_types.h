/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Client-side

FILE DESCRIPTION
Compile-time checking of type sizes to avoid surprises when porting to new
platforms.
=============================================================================*/

#ifndef KHRN_CLIENT_CHECK_TYPES_H
#define KHRN_CLIENT_CHECK_TYPES_H

#include "interface/khronos/common/khrn_int_util.h"

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/VG/openvg.h"

/*
   egl types
*/

vcos_static_assert(sizeof(EGLint) == 4);
vcos_static_assert(sizeof(EGLBoolean) == 4);
vcos_static_assert(sizeof(EGLenum) == 4);
vcos_static_assert(sizeof(EGLConfig) == 4);
vcos_static_assert(sizeof(EGLContext) == 4);
vcos_static_assert(sizeof(EGLDisplay) == 4);
vcos_static_assert(sizeof(EGLSurface) == 4);
vcos_static_assert(sizeof(EGLClientBuffer) == 4);
vcos_static_assert(sizeof(NativeDisplayType) == 4);
vcos_static_assert(sizeof(NativePixmapType) == 4);
vcos_static_assert(sizeof(NativeWindowType) == 4);

/*
   gl types
*/

vcos_static_assert(sizeof(GLenum) == 4);
vcos_static_assert(sizeof(GLboolean) == 1);
vcos_static_assert(sizeof(GLbitfield) == 4);
vcos_static_assert(sizeof(GLbyte) == 1);
vcos_static_assert(sizeof(GLshort) == 2);
vcos_static_assert(sizeof(GLint) == 4);
vcos_static_assert(sizeof(GLsizei) == 4);
vcos_static_assert(sizeof(GLubyte) == 1);
vcos_static_assert(sizeof(GLushort) == 2);
vcos_static_assert(sizeof(GLuint) == 4);
vcos_static_assert(sizeof(GLfloat) == 4);
vcos_static_assert(sizeof(GLclampf) == 4);
vcos_static_assert(sizeof(GLfixed) == 4);
vcos_static_assert(sizeof(GLclampx) == 4);
vcos_static_assert(sizeof(GLintptr) == 4);
vcos_static_assert(sizeof(GLsizeiptr) == 4);

/*
   vg types
*/

vcos_static_assert(sizeof(VGfloat) == 4);
vcos_static_assert(sizeof(VGbyte) == 1);
vcos_static_assert(sizeof(VGubyte) == 1);
vcos_static_assert(sizeof(VGshort) == 2);
vcos_static_assert(sizeof(VGint) == 4);
vcos_static_assert(sizeof(VGuint) == 4);
vcos_static_assert(sizeof(VGbitfield) == 4);
vcos_static_assert(sizeof(VGboolean) == 4);

#endif
