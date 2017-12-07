/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "nexus_init_for_3d.h"

#include "nexus_platform.h"

#include <stdio.h>
#include <stdlib.h>

// Get the index of the primary graphics heap. There may be secondary graphics
// heaps on other memcs but we don't pay attention to those.
static unsigned get_graphics_heap_index(const NEXUS_PlatformSettings *settings)
{
   for (unsigned i = 0; i != sizeof(settings->heap) / sizeof(settings->heap[0]); ++i)
      if (settings->heap[i].heapType & NEXUS_HEAP_TYPE_GRAPHICS)
         return i;
   fprintf(stderr, "No graphics heap found!\n");
   abort();
}

static void maximise_graphics_heap(NEXUS_PlatformSettings *settings, unsigned graphics_heap_index)
{
   // Only care about the memc containing the primary graphics heap; ignore the others
   unsigned memc = settings->heap[graphics_heap_index].memcIndex;

   // Conservatively ignore all regions except the largest.
   // There is normally just one region per memc anyway.
   NEXUS_PlatformMemory mem;
   NEXUS_Platform_GetMemory(&mem);
   uint64_t size = 0;
   for (unsigned i = 0; i != sizeof(mem.osRegion) / sizeof(mem.osRegion[0]); ++i)
      if ((mem.osRegion[i].memcIndex == memc) && (size < mem.osRegion[i].length))
         size = mem.osRegion[i].length;

   // Shrink heaps other than the primary graphics heap on the memc, track how
   // much space we will have left after all of these heaps have been allocated
   for (unsigned i = 0; i != sizeof(settings->heap) / sizeof(settings->heap[0]); ++i)
   {
      NEXUS_PlatformHeapSettings *h = &settings->heap[i];
      if ((h->memcIndex == memc) && (i != graphics_heap_index))
      {
         if (i == NEXUS_MEMC0_MAIN_HEAP)
            h->size = 64 * 1024 * 1024;
         else if (i != NEXUS_MEMC0_DRIVER_HEAP)
            h->size = 0;
         size -= h->size;
      }
   }

   // Use all the remaining space for the primary graphics heap
   if (sizeof(void *) == 4)
   {
      // Limit to 1GB in 32-bit mode to avoid mmap failure
      uint64_t max_size = 1024 * 1024 * 1024;
      if (size > max_size)
         size = max_size;
   }
   settings->heap[graphics_heap_index].size = size;
}

static bool use_max_graphics_mem(void)
{
   const char *env = getenv("V3D_USE_MAX_GRAPHICS_MEM");
   return env && (
      (env[0] == 't') || (env[0] == 'T') ||
      (env[0] == 'y') || (env[0] == 'Y') ||
      (env[0] == '1'));
}

unsigned nexus_init_for_3d(void)
{
   NEXUS_PlatformSettings settings;
   NEXUS_Platform_GetDefaultSettings(&settings);

   settings.openFrontend = false;
#ifdef NEXUS_HAS_VIDEO_DECODER
   settings.videoDecoderModuleSettings.deferInit = true;
#endif
#ifdef NEXUS_HAS_VIDEO_ENCODER
   settings.videoEncoderSettings.deferInit = true;
#endif

   unsigned graphics_heap_index = get_graphics_heap_index(&settings);
   if (use_max_graphics_mem())
      maximise_graphics_heap(&settings, graphics_heap_index);

   NEXUS_Error err = NEXUS_Platform_Init(&settings);
   if (err != NEXUS_SUCCESS)
   {
      fprintf(stderr, "NEXUS_Platform_Init() failed!\n");
      abort();
   }

   return graphics_heap_index;
}
