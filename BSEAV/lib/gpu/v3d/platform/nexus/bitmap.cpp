/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

   NEXUS_Addr devPtr;
   NEXUS_SurfaceMemoryProperties memProperties;
   NEXUS_Surface_GetMemoryProperties(m_surface, &memProperties);
   NEXUS_Error err = NEXUS_MemoryBlock_LockOffset(memProperties.pixelMemory, &devPtr);
   if (err != NEXUS_SUCCESS)
   {
      printf("unable to lock offset\n");
      exit(0);
   }

   NEXUS_SurfaceMemory surfMemory;
   err = NEXUS_Surface_GetMemory(m_surface, &surfMemory);
   if (err != NEXUS_SUCCESS)
   {
      printf("unable to get surface memory\n");
      exit(0);
   }

   NEXUS_SurfaceStatus surfStatus;
   NEXUS_Surface_GetStatus(m_surface, &surfStatus);

   NEXUS_Surface_Flush(m_surface);

   m_settings = {};

   m_settings.cachedAddr = surfMemory.buffer;
   m_settings.physOffset = (uint32_t)devPtr;

   m_settings.width = surfStatus.width;
   m_settings.height = surfStatus.height;
   m_settings.pitchBytes = surfStatus.pitch;
   m_settings.totalByteSize = surfMemory.bufferSize;
   m_settings.secure = surfMemory.buffer == NULL;
   m_settings.format = format;
}

Bitmap::~Bitmap()
{
   if (m_surface)
   {
      NEXUS_SurfaceMemoryProperties memProperties;
      NEXUS_Surface_GetMemoryProperties(m_surface, &memProperties);

      NEXUS_MemoryBlock_UnlockOffset(memProperties.pixelMemory);
      NEXUS_Surface_Destroy(m_surface);
   }
}

int Bitmap::GetBpp() const
{
   NEXUS_PixelFormatInfo info;
   NEXUS_PixelFormat_GetInfo(BeglToNexusFormat(m_settings.format), &info);
   return info.bpp;
}

}
