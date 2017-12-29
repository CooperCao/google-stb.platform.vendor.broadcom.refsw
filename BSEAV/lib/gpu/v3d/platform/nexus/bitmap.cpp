/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bitmap.h"
#include <cstdio>

namespace nxpl
{

Bitmap::Bitmap(void *context, NEXUS_HeapHandle heap, const BEGL_PixmapInfoEXT *bufferRequirements)
{
   Init(context, heap, bufferRequirements);
}

Bitmap::Bitmap(void *context, const BEGL_PixmapInfoEXT *bufferRequirements)
{
   NEXUS_HeapHandle heap;
   if (bufferRequirements->secure)
      heap = NXPL_MemHeapSecure(static_cast<NXPL_Display*>(context)->memInterface);
   else
      heap = NXPL_MemHeap(static_cast<NXPL_Display*>(context)->memInterface);
   Init(context, heap, bufferRequirements);
}

void Bitmap::Init(void *context, NEXUS_HeapHandle heap, const BEGL_PixmapInfoEXT *bufferRequirements)
{
   m_data = static_cast<NXPL_Display*>(context);

   m_data->bufferGetRequirementsFunc(bufferRequirements, &m_settings);

   NEXUS_SurfaceCreateSettings surfSettings;
   NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);
   surfSettings.pixelFormat = GetNativeFormat();
   surfSettings.width = m_settings.width;
   surfSettings.height = m_settings.height;
   surfSettings.alignment = m_settings.alignment;
   surfSettings.pitch = m_settings.pitchBytes;
   surfSettings.heap = heap;
   surfSettings.pixelMemory = NEXUS_MemoryBlock_Allocate(heap, m_settings.totalByteSize, m_settings.alignment, NULL);

   m_surface = NEXUS_Surface_Create(&surfSettings);

   if (m_surface == NULL)
   {
      printf("error : out of memory\n");
      exit(0);
   }

   NEXUS_Addr devPtr;
   NEXUS_Error err = NEXUS_MemoryBlock_LockOffset(surfSettings.pixelMemory, &devPtr);
   if (err != NEXUS_SUCCESS)
   {
      printf("unable to lock offset\n");
      exit(0);
   }
   m_settings.physOffset = (uint32_t)devPtr;

   void *res = NULL;
   if (!bufferRequirements->secure)
   {
      err = NEXUS_MemoryBlock_Lock(surfSettings.pixelMemory, &res);
      if (err != NEXUS_SUCCESS)
      {
         printf("unable to lock\n");
         exit(0);
      }
   }
   m_settings.cachedAddr = res;
}

Bitmap::~Bitmap()
{
   if (m_surface)
   {
      NEXUS_SurfaceCreateSettings surfSettings;
      NEXUS_Surface_GetCreateSettings(m_surface, &surfSettings);

      NEXUS_MemoryBlock_UnlockOffset(surfSettings.pixelMemory);
      NEXUS_MemoryBlock_Unlock(surfSettings.pixelMemory);
      NEXUS_Surface_Destroy(m_surface);
      NEXUS_MemoryBlock_Free(surfSettings.pixelMemory);
   }
}

int Bitmap::GetBpp() const
{
   NEXUS_PixelFormatInfo info;
   NEXUS_PixelFormat_GetInfo(GetNativeFormat(), &info);
   return info.bpp;
}

NEXUS_PixelFormat Bitmap::GetNativeFormat() const
{
   NEXUS_PixelFormat format;
   switch (m_settings.format)
   {
   case BEGL_BufferFormat_eA8B8G8R8: format = NEXUS_PixelFormat_eA8_B8_G8_R8; break;
   case BEGL_BufferFormat_eX8B8G8R8: format = NEXUS_PixelFormat_eX8_B8_G8_R8; break;
   case BEGL_BufferFormat_eR5G6B5:   format = NEXUS_PixelFormat_eR5_G6_B5; break;
   default: format = NEXUS_PixelFormat_eUnknown; break;
   }
   return format;
}

}
