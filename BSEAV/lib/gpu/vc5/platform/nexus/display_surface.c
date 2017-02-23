/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#include "private_nexus.h"
#include "nexus_platform.h"
#include "display_helpers.h"

#define NXPL_INFO_MAGIC 0x4A694D5F

bool isNXPL_Surface(NXPL_Surface *s)
{
   return s->magic == NXPL_INFO_MAGIC;
}

bool CreateSurface(NXPL_Surface *s,
   BEGL_BufferFormat format,
   uint32_t width, uint32_t height,
   bool secure,
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

      /* Framebuffer heap 0 is always GFD accessible */
      NEXUS_HeapHandle  heap = NEXUS_Platform_GetFramebufferHeap(secure ? NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE : 0);

      surfSettings.pixelMemory = NEXUS_MemoryBlock_Allocate(heap, bytes, 4096, NULL);

      surfSettings.pixelMemoryOffset = 0;

      if (surfSettings.pixelMemory != NULL)
      {
         s->surface = NEXUS_Surface_Create(&surfSettings);
         if (s->surface != NULL)
         {
            NEXUS_Addr offset;
            NEXUS_MemoryBlock_LockOffset(surfSettings.pixelMemory, &offset);
            s->physicalOffset = (uint32_t)offset;
            // Secure memory can't be mapped as the ARM will try to access it
            if (!secure)
               NEXUS_MemoryBlock_Lock(surfSettings.pixelMemory, &s->cachedPtr);
            else
               s->cachedPtr = NULL;
         }
      }

      s->magic = NXPL_INFO_MAGIC;
      s->fence  = -1;
      s->format = format;
      s->secure = secure;

      return true;
   }

   return false;
}

void DestroySurface(NXPL_Surface *s)
{
   if (s && s->surface)
   {
      NEXUS_SurfaceMemoryProperties memProperties;
      NEXUS_Surface_GetMemoryProperties(s->surface, &memProperties);

      NEXUS_Surface_Destroy(s->surface);

      /* This will also unmap and unlock the memory block - no need to do that explicitly */
      NEXUS_MemoryBlock_Free(memProperties.pixelMemory);
   }
}
