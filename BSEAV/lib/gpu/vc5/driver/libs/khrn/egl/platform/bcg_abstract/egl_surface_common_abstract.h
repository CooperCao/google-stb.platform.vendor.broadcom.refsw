/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_SURFACE_COMMON_ABSTRACT_H
#define EGL_SURFACE_COMMON_ABSTRACT_H

#include "../../egl_display.h"

#include "egl_platform_abstract.h"

extern BEGL_BufferFormat get_begl_format_abstract(GFX_LFMT_T fmt);
extern khrn_image     *image_from_surface_abstract(void *nativeSurface, bool flipY);

#endif
