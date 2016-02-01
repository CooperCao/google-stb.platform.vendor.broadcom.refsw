/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
Common implementations for surface implementations
=============================================================================*/

#ifndef EGL_SURFACE_COMMON_ABSTRACT_H
#define EGL_SURFACE_COMMON_ABSTRACT_H

#include "middleware/khronos/egl/egl_display.h"

#include "egl_platform_abstract.h"

extern BEGL_BufferFormat get_begl_format_abstract(GFX_LFMT_T fmt);
extern KHRN_IMAGE_T     *image_from_surface_abstract(void *nativeSurface, bool flipY);

#endif
