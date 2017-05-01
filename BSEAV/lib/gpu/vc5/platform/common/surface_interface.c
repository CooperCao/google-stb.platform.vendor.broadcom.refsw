/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "surface_interface.h"

#include <stddef.h>
#include <assert.h>

bool SurfaceInterface_Create(const SurfaceInterface *si,
      void *surface, uint32_t width, uint32_t height, BEGL_BufferFormat format,
      bool secure)
{
   assert(si != NULL);
   assert(si->create != NULL);
   assert(surface != NULL);
   assert(width > 0);
   assert(height > 0);

   return si->create(si->base.context, surface, width, height, format, secure);
}

void SurfaceInterface_Destroy(const SurfaceInterface *si,
      void *surface)
{
   assert(si != NULL);
   assert(si->destroy != NULL);

   if (surface)
      si->destroy(si->base.context, surface);
}
