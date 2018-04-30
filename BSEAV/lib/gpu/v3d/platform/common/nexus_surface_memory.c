/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "nexus_surface_memory.h"
#include "nexus_platform.h"

#include <stdio.h>

static NEXUS_MemoryBlockHandle CloneNexusMemoryBlock(NEXUS_MemoryBlockHandle block,
      NEXUS_Addr *phys)
{
   if (!block)
      return NULL;

   NEXUS_MemoryBlockTokenHandle token = NEXUS_MemoryBlock_CreateToken(block);
   NEXUS_MemoryBlockHandle clone = NEXUS_MemoryBlock_Clone(token);
   if (clone)
   {
      NEXUS_Error err;
      err = NEXUS_MemoryBlock_LockOffset(clone, phys);
      if (err != NEXUS_SUCCESS)
      {
         NEXUS_MemoryBlock_Free(clone);
         clone = NULL;
      }
   }
   return clone;
}

static bool IsValidHeap(NEXUS_HeapHandle heap)
{
   NEXUS_HeapHandle valid[2] =
   {
         NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE),
         NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE)
   };
   return heap && (heap == valid[0] || heap == valid[1]);
}

NEXUS_MemoryBlockHandle AcquireNexusSurfaceMemory(
      NEXUS_SurfaceHandle surface, BEGL_BufferSettings *settings)
{
   if (!surface)
   {
      printf("%s error: no surface\n", __FUNCTION__);
      return NULL;
   }

   NEXUS_SurfaceCreateSettings createSettings;
   NEXUS_Surface_GetCreateSettings(surface, &createSettings);
   if (!createSettings.compatibility.graphicsv3d)
   {
      printf("%s error: graphicsv3d compatibility not enabled\n", __FUNCTION__);
      return NULL;
   }

   if (!IsValidHeap(createSettings.heap))
   {
      printf("%s error: invalid heap\n", __FUNCTION__);
      return NULL;
   }

   NEXUS_SurfaceMemory surfMemory;
   if (NEXUS_Surface_GetMemory(surface, &surfMemory) != NEXUS_SUCCESS)
   {
      printf("%s error: can't get surface memory\n", __FUNCTION__);
      return NULL;
   }

   NEXUS_SurfaceStatus surfStatus;
   NEXUS_Surface_GetStatus(surface, &surfStatus);
   BEGL_BufferFormat format = NexusToBeglFormat(surfStatus.pixelFormat);
   if (format == BEGL_BufferFormat_INVALID)
   {
      printf("%s error: invalid pixel format\n", __FUNCTION__);
      return NULL;
   }

   NEXUS_SurfaceMemoryProperties memProperties;
   NEXUS_Addr physicalOffset = 0;
   NEXUS_Surface_GetMemoryProperties(surface, &memProperties);
   NEXUS_MemoryBlockHandle clone = CloneNexusMemoryBlock(
         memProperties.pixelMemory, &physicalOffset);
   if (clone)
   {
      settings->cachedAddr    = surfMemory.buffer;
      settings->physOffset    = physicalOffset;
      settings->width         = surfStatus.width;
      settings->height        = surfStatus.height;
      settings->pitchBytes    = surfStatus.pitch;
      settings->totalByteSize = surfMemory.bufferSize;
      settings->secure        = surfMemory.buffer == NULL;
      settings->format        = format;
   }
   else
   {
      printf("%s error: can't clone memory block\n", __FUNCTION__);
   }
   return clone;
}

void ReleaseNexusSurfaceMemory(NEXUS_MemoryBlockHandle memory)
{
   if (!memory)
      return;
   NEXUS_MemoryBlock_UnlockOffset(memory);
   NEXUS_MemoryBlock_Free(memory);
}
