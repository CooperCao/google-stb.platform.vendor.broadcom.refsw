/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdint.h>

typedef enum {
   OPENGL_ES_NONE = 0,
   OPENGL_ES_11 = 1 << 0,
   OPENGL_ES_20 = 1 << 1,
   OPENGL_ES_ANY = OPENGL_ES_11 | OPENGL_ES_20
} EGL_CONTEXT_TYPE_T;

typedef enum {
   WINDOW,
   PBUFFER,
   PIXMAP
} EGL_SURFACE_TYPE_T;

typedef enum {
   SRGB,
   LINEAR
} EGL_SURFACE_COLORSPACE_T;

typedef enum {
   NONPRE,
   PRE
} EGL_SURFACE_ALPHAFORMAT_T;

// Must be enough for triple-buffering (windows) and mipmaps (pbuffers)
#define EGL_MAX_BUFFERS       12