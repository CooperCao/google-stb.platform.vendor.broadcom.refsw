/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "private_nexus.h"
#include "nexus_platform.h"
#include "display_helpers.h"
#include "display_surface.h"
#include "nexus_heap_selection.h"
#include "nexus_surface.h"
#include "nexus_striped_surface.h"

bool isNXPL_Surface(NXPL_Surface *s)
{
   return s->magic == NXPL_INFO_MAGIC;
}

bool CreateSurface(NXPL_Surface *s,
   BEGL_BufferFormat format,
   uint32_t width, uint32_t height, uint32_t miplevels,
   bool secure,
   const char *desc)
{
   if (s)
   {
      if (format == BEGL_BufferFormat_eY8 || format == BEGL_BufferFormat_eY10)
      {
         // can't render into display surface in SAND format;
         return false;
      }
      else
      {
         NEXUS_SurfaceCreateSettings surfSettings;
         NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);

         if (!BeglToNexusFormat(&surfSettings.pixelFormat, format))
            return false;

         // Note we allow NEXUS_Surface_Create to allocate the memory as
         // it will calculate the required size for UIF surfaces as well as
         // raster and we don't want to duplicate the logic for that here.
         surfSettings.compatibility.graphicsv3d = true;

         surfSettings.width = width;
         surfSettings.height = height;
         // NEXUS surface creation specifies the mip level number at the beginning
         // of the surface, not the number of miplevels.
         surfSettings.mipLevel = miplevels - 1;
         surfSettings.heap = secure ? GetSecureHeap() : GetDisplayHeap(0);
         surfSettings.alignment = 12; // log2(4096)

         s->surface = NEXUS_Surface_Create(&surfSettings);
         if (s->surface != NULL)
         {
            NEXUS_SurfaceMemoryProperties memProperties;
            NEXUS_Surface_GetMemoryProperties(s->surface, &memProperties);

            NEXUS_Addr offset;
            NEXUS_MemoryBlock_LockOffset(memProperties.pixelMemory, &offset);
            s->physicalOffset = offset;
            // Secure memory can't be mapped as the ARM will try to access it
            if (!secure)
               NEXUS_MemoryBlock_Lock(memProperties.pixelMemory, &s->cachedPtr);
            else
               s->cachedPtr = NULL;

            s->magic       = NXPL_INFO_MAGIC;
            s->fence       = -1;
            s->format      = format;
            s->colorimetry = BEGL_Colorimetry_RGB;
            s->secure      = secure;

            return true;
         }
      }
   }

   return false;
}

void DestroySurface(NXPL_Surface *s)
{
   // Will destroy the memory backing the surface as well
   if (s && s->surface)
      NEXUS_Surface_Destroy(s->surface);
}
