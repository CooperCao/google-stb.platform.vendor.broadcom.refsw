/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_SURFACE_H__
#define __DISPLAY_SURFACE_H__

#include "private_nexus.h"
#include "egl_platform_abstract.h"

bool CreateSurface(NXPL_Surface *s,
   BEGL_BufferFormat format, uint32_t width, uint32_t height,
   const char *desc);

void DestroySurface(NXPL_Surface *s);

#endif /* __DISPLAY_SURFACE_H__ */