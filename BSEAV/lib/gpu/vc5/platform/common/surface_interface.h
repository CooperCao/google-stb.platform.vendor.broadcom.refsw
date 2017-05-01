/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __SURFACE_INTERFACE_H__
#define __SURFACE_INTERFACE_H__

#include "interface.h"

#include "list.h"
#include <EGL/begl_displayplatform.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct SurfaceInterface
{
   Interface base;

   size_t sizeof_surface;

   /* create surface in the provided pre-allocated buffer */
   bool (*create)(void *context, void *surface, uint32_t width,
         uint32_t height, BEGL_BufferFormat format, bool secure);

   /* destroy surface in the provided buffer, do not free the buffer */
   void (*destroy)(void *context, void *surface);
} SurfaceInterface;

bool SurfaceInterface_Create(const SurfaceInterface *si,
      void *surface, uint32_t width, uint32_t height, BEGL_BufferFormat format,
      bool secure);

void SurfaceInterface_Destroy(const SurfaceInterface *si,
      void *surface);

#ifdef __cplusplus
}
#endif

#endif /* __SURFACE_INTERFACE_H__ */
