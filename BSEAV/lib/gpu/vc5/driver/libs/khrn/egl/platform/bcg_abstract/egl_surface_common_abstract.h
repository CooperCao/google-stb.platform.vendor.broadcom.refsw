/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_SURFACE_COMMON_ABSTRACT_H
#define EGL_SURFACE_COMMON_ABSTRACT_H

#include "../../egl_display.h"

#include "egl_platform_abstract.h"

extern khrn_image *image_from_surface_abstract(void *nativeSurface, bool flipY, unsigned *num_mip_levels);

/* same as above, but if existing image != NULL, compare the new information
 * from surface with the one in image and if they are different (eg: different
 * physical locations, different fmts/ different sizes, etc) return a new
 * image; otherwise, return the existing image, with ref count incremented */
extern khrn_image *image_from_surface_abstract_with_existing(void *nativeSurface,
      bool flipY, unsigned *num_mip_levels, khrn_image *existing_img);

extern bool surface_get_info(void *nativeSurface, BEGL_SurfaceInfo *info);

#endif
