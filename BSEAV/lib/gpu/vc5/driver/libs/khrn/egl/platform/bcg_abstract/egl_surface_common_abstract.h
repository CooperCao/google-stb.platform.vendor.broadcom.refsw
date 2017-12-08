/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_SURFACE_COMMON_ABSTRACT_H
#define EGL_SURFACE_COMMON_ABSTRACT_H

#include "../../egl_display.h"

#include "egl_platform_abstract.h"

extern khrn_image *image_from_surface_abstract(void *nativeSurface, bool flipY, unsigned *num_mip_levels);
extern bool surface_get_info(void *nativeSurface, BEGL_SurfaceInfo *info);

#endif
