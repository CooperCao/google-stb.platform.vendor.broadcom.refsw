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
#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/VG/openvg.h"

/*
   egl types
*/

static_assrt(sizeof(EGLint) == 4);
static_assrt(sizeof(EGLBoolean) == 4);
static_assrt(sizeof(EGLenum) == 4);
static_assrt(sizeof(EGLConfig) == 4);
static_assrt(sizeof(EGLContext) == 4);
static_assrt(sizeof(EGLDisplay) == 4);
static_assrt(sizeof(EGLSurface) == 4);
static_assrt(sizeof(EGLClientBuffer) == 4);
static_assrt(sizeof(NativeDisplayType) == 4);
static_assrt(sizeof(NativePixmapType) == 4);
static_assrt(sizeof(NativeWindowType) == 4);

/*
   gl types
*/

static_assrt(sizeof(GLenum) == 4);
static_assrt(sizeof(GLboolean) == 1);
static_assrt(sizeof(GLbitfield) == 4);
static_assrt(sizeof(GLbyte) == 1);
static_assrt(sizeof(GLshort) == 2);
static_assrt(sizeof(GLint) == 4);
static_assrt(sizeof(GLsizei) == 4);
static_assrt(sizeof(GLubyte) == 1);
static_assrt(sizeof(GLushort) == 2);
static_assrt(sizeof(GLuint) == 4);
static_assrt(sizeof(GLfloat) == 4);
static_assrt(sizeof(GLclampf) == 4);
static_assrt(sizeof(GLfixed) == 4);
static_assrt(sizeof(GLclampx) == 4);
static_assrt(sizeof(GLintptr) == 4);
static_assrt(sizeof(GLsizeiptr) == 4);

/*
   vg types
*/

static_assrt(sizeof(VGfloat) == 4);
static_assrt(sizeof(VGbyte) == 1);
static_assrt(sizeof(VGubyte) == 1);
static_assrt(sizeof(VGshort) == 2);
static_assrt(sizeof(VGint) == 4);
static_assrt(sizeof(VGuint) == 4);
static_assrt(sizeof(VGbitfield) == 4);
static_assrt(sizeof(VGboolean) == 4);

#endif
