/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "EGL/egl.h"
#include "nexus_graphics2d.h"

static inline NEXUS_PixelFormat GetNativeFormat(BEGL_BufferFormat begl_format)
{
   NEXUS_PixelFormat format;
   switch (begl_format)
   {
   case BEGL_BufferFormat_eA8B8G8R8:
      format = NEXUS_PixelFormat_eA8_B8_G8_R8;
      break;
   case BEGL_BufferFormat_eX8B8G8R8:
      format = NEXUS_PixelFormat_eX8_B8_G8_R8;
      break;
   case BEGL_BufferFormat_eR5G6B5:
      format = NEXUS_PixelFormat_eR5_G6_B5;
      break;
   default:
      format = NEXUS_PixelFormat_eUnknown;
      break;
   }
   return format;
}
