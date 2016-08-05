/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __SURFACE_INTERFACE_H__
#define __SURFACE_INTERFACE_H__

#include "interface.h"

#include "list.h"
#include <EGL/begl_displayplatform.h>
#include <stdbool.h>

struct SurfaceInterface
{
   struct Interface base;

   size_t sizeof_surface;

   /* create surface in the provided pre-allocated buffer */
   bool (*create)(void *context, void *surface, uint32_t width,
         uint32_t height, BEGL_BufferFormat format, bool secure);

   /* destroy surface in the provided buffer, do not free the buffer */
   void (*destroy)(void *context, void *surface);
};

bool SurfaceInterface_Create(const struct SurfaceInterface *si,
      void *surface, uint32_t width, uint32_t height, BEGL_BufferFormat format,
      bool secure);

void SurfaceInterface_Destroy(const struct SurfaceInterface *si,
      void *surface);

#endif /* __SURFACE_INTERFACE_H__ */
