/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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

#ifndef SINGLE_PROCESS
      NEXUS_HeapHandle unsecureHeap = NULL;
      NEXUS_HeapHandle secureHeap = NULL;
      {
         NEXUS_ClientConfiguration clientConfig;
         NEXUS_Platform_GetClientConfiguration(&clientConfig);

         if (clientConfig.mode == NEXUS_ClientMode_eUntrusted)
            unsecureHeap = clientConfig.heap[0];
         else
#ifdef NXCLIENT_SUPPORT
            unsecureHeap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);
#else
            /* NEXUS_Platform_GetFramebufferHeap() is not callable under NEXUS_CLIENT_SUPPORT=y */
            unsecureHeap = NULL;
#endif

#ifdef NXCLIENT_SUPPORT
         secureHeap = clientConfig.heap[NXCLIENT_SECURE_GRAPHICS_HEAP];
#else
         /* not available in NEXUS_CLIENT_SUPPORT=y mode */
         secureHeap = NULL;
#endif
      }
      NEXUS_HeapHandle  heap = secure ? secureHeap : unsecureHeap;

#else
      /* Framebuffer heap 0 is always GFD accessible */
      NEXUS_HeapHandle  heap = NEXUS_Platform_GetFramebufferHeap(secure ? NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE : 0);
#endif

      surfSettings.pixelMemory = NEXUS_MemoryBlock_Allocate(heap, bytes, 4096, NULL);

      surfSettings.pixelMemoryOffset = 0;

      if (surfSettings.pixelMemory != NULL)
      {
         s->surface = NEXUS_Surface_Create(&surfSettings);
         if (s->surface != NULL)
         {
            NEXUS_Addr offset;
            NEXUS_MemoryBlock_LockOffset(surfSettings.pixelMemory, &offset);
            s->physicalOffset = offset;
            // Secure memory can't be mapped as the ARM will try to access it
            if (!secure)
               NEXUS_MemoryBlock_Lock(surfSettings.pixelMemory, &s->cachedPtr);
            else
               s->cachedPtr = NULL;

            s->magic = NXPL_INFO_MAGIC;
            s->fence  = -1;
            s->format = format;
            s->secure = secure;

            return true;
         }
         else
         {
            NEXUS_MemoryBlock_Free(surfSettings.pixelMemory);
         }
      }
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
