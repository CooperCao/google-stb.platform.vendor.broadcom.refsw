/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header

FILE DESCRIPTION
Functions to create and destroy a native pixmap type that will be compatible
with OpenGL-ES rendering.
=============================================================================*/

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/common/khrn_int_image.h"
#include "interface/khronos/common/khrn_client.h"
#include "middleware/khronos/common/khrn_image.h"

#include <malloc.h>
#include <assert.h>

BEGL_BufferHandle BEGLint_PixmapCreateCompatiblePixmap(BEGL_PixmapInfo *pixmapInfo)
{
   BEGL_BufferSettings bufferSettings;
   BEGL_BufferHandle   buffer = NULL;
   KHRN_IMAGE_FORMAT_T format;
   uint32_t align, wd, ht;
   KHRN_IMAGE_CREATE_FLAG_T flags;
   /* needs to be max of CPU cache line and the GCACHE on the v3d */
   uint32_t cacheline_size = mem_cacheline_size();
   BEGL_DriverInterfaces * driverInterfaces = BEGL_GetDriverInterfaces();
   cacheline_size = (cacheline_size > BCG_GCACHE_LINE_SIZE) ? cacheline_size : BCG_GCACHE_LINE_SIZE;

   memset(&bufferSettings, 0, sizeof(BEGL_BufferSettings));

   flags = IMAGE_CREATE_FLAG_RENDER_TARGET | IMAGE_CREATE_FLAG_DISPLAY;

   switch (pixmapInfo->format)
   {
   case BEGL_BufferFormat_eA8B8G8R8:
   case BEGL_BufferFormat_eX8B8G8R8:
      format = ABGR_8888_RSO;
      break;
   case BEGL_BufferFormat_eR8G8B8A8:
   case BEGL_BufferFormat_eR8G8B8X8:
      format = RGBA_8888_RSO;
      break;
   case BEGL_BufferFormat_eR5G6B5:
   case BEGL_BufferFormat_eR5G6B5_Texture:
      format = RGB_565_RSO;
      break;
   case BEGL_BufferFormat_eA8B8G8R8_Texture:
   case BEGL_BufferFormat_eR8G8B8A8_Texture:
      format = ABGR_8888_RSO;
      flags = IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RSO_TEXTURE | IMAGE_CREATE_FLAG_RENDER_TARGET;
      break;
   case BEGL_BufferFormat_eA8R8G8B8_Texture:
   case BEGL_BufferFormat_eB8G8R8A8_Texture:
      format = ARGB_8888_RSO;
      flags = IMAGE_CREATE_FLAG_TEXTURE | IMAGE_CREATE_FLAG_RSO_TEXTURE | IMAGE_CREATE_FLAG_RENDER_TARGET;
      break;
   case BEGL_BufferFormat_eYUV422_Texture:
   case BEGL_BufferFormat_eVUY224_Texture:
      format = YUV_422_RSO;
      flags = 0;  /* don't allow anything apart from the APPLE extension */
      break;
   default:
      assert(0);
      format = IMAGE_FORMAT_INVALID;
   }

   if (bufferSettings.openvg)
   {
      switch (bufferSettings.colorFormat)
      {
         case BEGL_ColorFormat_eLinear:  format |= IMAGE_FORMAT_LIN;                            break;
         case BEGL_ColorFormat_eSRGB:  /* nothing */                                            break;
         case BEGL_ColorFormat_eLinear_Pre: format |= (IMAGE_FORMAT_LIN | IMAGE_FORMAT_PRE);    break;
         case BEGL_ColorFormat_eSRGB_Pre: format |= IMAGE_FORMAT_PRE;                           break;
         default:
            UNREACHABLE();
         break;
      }
      format |= IMAGE_FORMAT_OVG;
   }

   bufferSettings.usage = BEGL_BufferUsage_ePixmap;
   bufferSettings.format = pixmapInfo->format;
   bufferSettings.width = pixmapInfo->width;
   bufferSettings.height = pixmapInfo->height;

   wd = pixmapInfo->width;
   ht = pixmapInfo->height;
   align = 0;

   khrn_image_platform_fudge(&format, &wd, &ht, &align, flags);

   bufferSettings.alignment = (align > cacheline_size) ? align : cacheline_size;
   bufferSettings.pitchBytes = khrn_image_get_stride(format, wd);
   bufferSettings.totalByteSize = (khrn_image_get_size(format, wd, ht) + (cacheline_size - 1)) & ~(cacheline_size -1);

   if ((driverInterfaces != NULL) &&
         (driverInterfaces->displayInterface != NULL) &&
         (driverInterfaces->displayInterface->BufferCreate != NULL))
   {
      buffer = driverInterfaces->displayInterface->BufferCreate(driverInterfaces->displayInterface->context, &bufferSettings);
   }

   return buffer;
}
