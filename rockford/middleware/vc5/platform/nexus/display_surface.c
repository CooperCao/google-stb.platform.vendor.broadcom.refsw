/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "private_nexus.h"
#include "nexus_platform.h"
#include "display_helpers.h"

bool CreateSurface(NXPL_Surface *s,
   BEGL_BufferFormat format, uint32_t width, uint32_t height,
   const char *desc)
{
   if (s)
   {
      NEXUS_SurfaceCreateSettings    surfSettings;
      uint32_t                       bytes;

      NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);

      if (!BeglToNexusFormat(&surfSettings.pixelFormat, format))
         return false;

      surfSettings.width = width;
      surfSettings.height = height;

      bytes = width * height * BeglFormatNumBytes(format);

      /* always GFD accessible */
      surfSettings.pixelMemory = NEXUS_MemoryBlock_Allocate(NEXUS_Platform_GetFramebufferHeap(0), bytes, 4096, NULL);
      surfSettings.pixelMemoryOffset = 0;

      if (surfSettings.pixelMemory != NULL)
      {
         s->surface = NEXUS_Surface_Create(&surfSettings);
         if (s->surface != NULL)
         {
            NEXUS_Addr offset;
            NEXUS_MemoryBlock_LockOffset(surfSettings.pixelMemory, &offset);
            s->physicalOffset = (uint32_t)offset;
            NEXUS_MemoryBlock_Lock(surfSettings.pixelMemory, &s->cachedPtr);
         }
      }

      s->fence = -1;
      s->format = format;

      return true;
   }

   return false;
}

void DestroySurface(NXPL_Surface *s)
{
   if (s)
   {
      NEXUS_SurfaceMemoryProperties memProperties;
      NEXUS_Surface_GetMemoryProperties(s->surface, &memProperties);

      NEXUS_Surface_Destroy(s->surface);

      /* This will also unmap and unlock the memory block - no need to do that explicitly */
      NEXUS_MemoryBlock_Free(memProperties.pixelMemory);
   }
}