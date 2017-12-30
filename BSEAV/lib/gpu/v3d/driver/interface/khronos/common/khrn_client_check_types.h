/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_util.h"

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/GLES/gl.h"

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