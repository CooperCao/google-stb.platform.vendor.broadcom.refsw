/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <EGL/begl_platform.h>
#include "nexus_graphics2d.h"

static inline BEGL_BufferFormat NexusToBeglFormat(NEXUS_PixelFormat nexus_format)
{
   switch (nexus_format)
   {
   case NEXUS_PixelFormat_eA8_B8_G8_R8:   return BEGL_BufferFormat_eA8B8G8R8;
   case NEXUS_PixelFormat_eX8_B8_G8_R8:   return BEGL_BufferFormat_eX8B8G8R8;
   case NEXUS_PixelFormat_eR5_G6_B5:      return BEGL_BufferFormat_eR5G6B5;
   default:                               return BEGL_BufferFormat_INVALID;
   }
}

static inline NEXUS_PixelFormat BeglToNexusFormat(BEGL_BufferFormat begl_format)
{
   switch (begl_format)
   {
   case BEGL_BufferFormat_eA8B8G8R8:   return NEXUS_PixelFormat_eA8_B8_G8_R8;
   case BEGL_BufferFormat_eX8B8G8R8:   return NEXUS_PixelFormat_eX8_B8_G8_R8;
   case BEGL_BufferFormat_eR5G6B5:     return NEXUS_PixelFormat_eR5_G6_B5;
   default:                            return NEXUS_PixelFormat_eUnknown;
   }
}
