/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_SURFACE_COMMON_ABSTRACT_H
#define EGL_SURFACE_COMMON_ABSTRACT_H

#include "../../egl_display.h"

#include "egl_platform_abstract.h"

extern khrn_image *image_from_surface_abstract(uint32_t type, void *object,
      bool flipY, const char *description, unsigned *num_mip_levels,
      EGLint *error);

extern BEGL_BufferFormat get_pixmap_format(void *nativePixmap);

#endif
