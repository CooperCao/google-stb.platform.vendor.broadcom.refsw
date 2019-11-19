/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "bitmap.h"
#include "nexus_platform.h"
#include "../common/nexus_begl_format.h"
#include <cstdio>

namespace nxpl
{

Bitmap::Bitmap(void *context, NEXUS_HeapHandle heap, uint32_t width,
      uint32_t height, BEGL_BufferFormat format)
{
   Init(context, heap, width, height, format);
}

Bitmap::Bitmap(void *context, uint32_t width, uint32_t height,
      BEGL_BufferFormat format, bool secure)
{
   NEXUS_HeapHandle heap;
   if (secure)
      heap = NXPL_MemHeapSecure(static_cast<NXPL_Display*>(context)->memInterface);
   else
      heap = NXPL_MemHeap(static_cast<NXPL_Display*>(context)->memInterface);
   Init(context, heap, width, height, format);
}

void Bitmap::Init(void *context, NEXUS_HeapHandle heap, uint32_t width,
      uint32_t height, BEGL_BufferFormat format)
{
   m_data = static_cast<NXPL_Display*>(context);

   NEXUS_SurfaceCreateSettings surfSettings;
   NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);
   surfSettings.pixelFormat = BeglToNexusFormat(format);
   surfSettings.width = width;
   surfSettings.height = height;
   surfSettings.heap = heap;
   surfSettings.compatibility.graphicsv3d = true;

   m_surface = NEXUS_Surface_Create(&surfSettings);
   if (m_surface == NULL)
   {
      printf("error : out of memory\n");
      exit(0);
   }

   m_extend = helper::Extent2D(width, height);
}

Bitmap::~Bitmap()
{
   if (m_surface)
   {
      NEXUS_Surface_Destroy(m_surface);
      m_surface = NULL;
   }
}

}
