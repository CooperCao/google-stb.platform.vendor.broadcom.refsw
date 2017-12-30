/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "nxbitmap.h"

#include "native_format.h"
#include <stdio.h>

namespace wlpl
{

NxBitmap::NxBitmap(NEXUS_HeapHandle heap, const BEGL_BufferSettings *settings) :
      NxBitmap()
{
   m_settings = *settings;
   m_settings.physOffset = 0;
   m_settings.cachedAddr = NULL;

   NEXUS_SurfaceCreateSettings surfSettings;
   NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);
   surfSettings.pixelFormat = ::GetNativeFormat(m_settings.format);
   surfSettings.width = m_settings.width;
   surfSettings.height = m_settings.height;
   surfSettings.alignment = m_settings.alignment;
   surfSettings.pitch = m_settings.pitchBytes;
   surfSettings.heap = heap;
   surfSettings.pixelMemory = NEXUS_MemoryBlock_Allocate(heap,
         m_settings.totalByteSize, m_settings.alignment, NULL);

   m_surface = NEXUS_Surface_Create(&surfSettings);

   if (!m_surface)
   {
      fprintf(stderr, "error : out of memory\n");
      exit(1);
   }

   NEXUS_Addr devPtr;
   NEXUS_Error err = NEXUS_MemoryBlock_LockOffset(surfSettings.pixelMemory,
         &devPtr);
   if (err != NEXUS_SUCCESS)
   {
      fprintf(stderr, "unable to lock offset\n");
      exit(1);
   }
   m_settings.physOffset = (uint32_t)devPtr;

   void *res = NULL;
   err = NEXUS_MemoryBlock_Lock(surfSettings.pixelMemory, &res);
   if (err != NEXUS_SUCCESS)
   {
      fprintf(stderr, "unable to lock\n");
      exit(1);
   }
   m_settings.cachedAddr = res;
}

NxBitmap::~NxBitmap()
{
   if (m_surface)
   {
      NEXUS_SurfaceCreateSettings surfSettings;
      NEXUS_Surface_GetCreateSettings(m_surface, &surfSettings);
      if (m_settings.physOffset)
         NEXUS_MemoryBlock_UnlockOffset(surfSettings.pixelMemory);
      if (m_settings.cachedAddr)
         NEXUS_MemoryBlock_Unlock(surfSettings.pixelMemory);
      NEXUS_Surface_Destroy(m_surface);
      NEXUS_MemoryBlock_Free(surfSettings.pixelMemory);
      m_surface = NULL;
   }
}

int NxBitmap::GetBpp() const
{
   NEXUS_PixelFormatInfo info;
   NEXUS_PixelFormat_GetInfo(::GetNativeFormat(m_settings.format), &info);
   return info.bpp;
}

}
