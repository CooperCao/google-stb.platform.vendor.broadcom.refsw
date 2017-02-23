/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_HELPERS__H__
#define __DISPLAY_HELPERS__H__

#include <EGL/begl_displayplatform.h>
#include "nexus_surface.h"

bool NexusToBeglFormat(BEGL_BufferFormat *result, NEXUS_PixelFormat format);
bool BeglToNexusFormat(NEXUS_PixelFormat *result, BEGL_BufferFormat format);
int BeglFormatNumBytes(BEGL_BufferFormat format);

#endif /* __DISPLAY_HELPERS__H__ */
