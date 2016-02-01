/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_HELPERS__H__
#define __DISPLAY_HELPERS__H__

#include "egl_platform_abstract.h"
#include "nexus_surface.h"

bool NexusToBeglFormat(BEGL_BufferFormat *result, NEXUS_PixelFormat format);
bool BeglToNexusFormat(NEXUS_PixelFormat *result, BEGL_BufferFormat format);
int BeglFormatNumBytes(BEGL_BufferFormat format);

#endif /* __DISPLAY_HELPERS__H__ */