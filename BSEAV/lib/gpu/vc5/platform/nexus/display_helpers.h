/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DISPLAY_HELPERS__H__
#define __DISPLAY_HELPERS__H__

#include <EGL/begl_displayplatform.h>
#include "nexus_surface.h"

bool NexusToBeglFormat(BEGL_BufferFormat *result, NEXUS_PixelFormat format);
bool BeglToNexusFormat(NEXUS_PixelFormat *result, BEGL_BufferFormat format);
int BeglFormatNumBytes(BEGL_BufferFormat format);

#endif /* __DISPLAY_HELPERS__H__ */
