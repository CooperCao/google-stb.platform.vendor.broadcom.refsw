/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  EGL

FILE DESCRIPTION
EGL client <-> server interface stuff.
=============================================================================*/

#ifndef EGL_INT_H
#define EGL_INT_H

typedef enum {
   OPENGL_ES_11,
   OPENGL_ES_20,
   OPENVG
} EGL_CONTEXT_TYPE_T;

// A single colour buffer. Optional ancillary buffers. If triple-buffered,
// three EGL_SERVER_SURFACE_T's share ancillary buffers
typedef uint32_t EGL_SURFACE_ID_T;

// Either a GLES1.1 or GLES2.0 server state
typedef uint32_t EGL_GL_CONTEXT_ID_T;

typedef uint32_t EGL_VG_CONTEXT_ID_T;

typedef uint32_t EGL_CONTEXT_ID_T;

#define EGL_SERVER_NO_SURFACE 0
#define EGL_SERVER_NO_GL_CONTEXT 0
#define EGL_SERVER_NO_VG_CONTEXT 0

#define EGL_SERVER_GL11 1
#define EGL_SERVER_GL20 2

typedef uint32_t EGL_SYNC_ID_T;

#endif
