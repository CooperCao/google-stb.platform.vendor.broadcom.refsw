/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "display_helpers.h"
#include "bchp_m2mc.h"
#include "memory_convert.h"
#include "assert.h"

#define M2MC_HAS_UIF_SUPPORT (BCHP_M2MC_REVISION_MAJOR_DEFAULT >= 2)

static void EventHandler(void *data, int unused)
{
   BSTD_UNUSED(unused);
   BKNI_SetEvent((BKNI_EventHandle)data);
}

static bool HasMipmapper()
{
   static bool hasMipmapper = false;
   static bool firstRun     = true;

   if (!firstRun)
      return hasMipmapper;

   NEXUS_Graphics2DOpenSettings graphics2dOpenSettings;
   NEXUS_Graphics2D_GetDefaultOpenSettings(&graphics2dOpenSettings);
   graphics2dOpenSettings.mode = NEXUS_Graphics2DMode_eMipmap;
   NEXUS_Graphics2DHandle mipmapper = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &graphics2dOpenSettings);

   hasMipmapper = (mipmapper != NULL);
   firstRun     = false;

   if (hasMipmapper)
      NEXUS_Graphics2D_Close(mipmapper);

   return hasMipmapper;
}

static bool OpenGraphics2D(MemConvertCache *cache, const BEGL_SurfaceConversionInfo *info)
{
   bool useMipmapper = HasMipmapper() && info->dstFormat == BEGL_BufferFormat_eTILED;

   if (cache->gfx2d != NULL && cache->gfxEvent != NULL &&
       cache->secure == info->secure && cache->mipmapper == useMipmapper)
      return true;  // We already have what we need

   MemoryConvertClearCache(cache);

   BKNI_CreateEvent(&cache->gfxEvent);
   if (cache->gfxEvent == NULL)
      return false;

   NEXUS_Graphics2DOpenSettings graphics2dOpenSettings;
   NEXUS_Graphics2D_GetDefaultOpenSettings(&graphics2dOpenSettings);
   graphics2dOpenSettings.secure = info->secure;
   cache->secure                 = info->secure;

   if (useMipmapper)
   {
      // Use mipmap M2MC when available for TILED formats
      graphics2dOpenSettings.mode = NEXUS_Graphics2DMode_eMipmap;
      cache->gfx2d = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &graphics2dOpenSettings);
   }
   if (cache->gfx2d == NULL)
   {
      // Use normal M2MC when mipmap version not available and for non TILED formats
      graphics2dOpenSettings.mode = NEXUS_Graphics2DMode_eBlitter;
      cache->gfx2d = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &graphics2dOpenSettings);
      cache->mipmapper = false;
   }
   else
      cache->mipmapper = true;

   if (cache->gfx2d != NULL)
   {
      // Configure the gfx2d checkpoint to call BKNI_SetEvent
      NEXUS_Graphics2DSettings gfxSettings;
      NEXUS_Graphics2D_GetSettings(cache->gfx2d, &gfxSettings);
      gfxSettings.checkpointCallback.callback = EventHandler;
      gfxSettings.checkpointCallback.context  = cache->gfxEvent;
      NEXUS_Graphics2D_SetSettings(cache->gfx2d, &gfxSettings);
      return true;
   }
   else
   {
      MemoryConvertClearCache(cache);
      return false;
   }
}

/* Close any cached objects */
void MemoryConvertClearCache(MemConvertCache *cache)
{
   if (cache->gfx2d)
   {
      NEXUS_Graphics2D_Close(cache->gfx2d);
      cache->gfx2d = NULL;
   }

   if (cache->gfxEvent)
   {
      BKNI_DestroyEvent(cache->gfxEvent);
      cache->gfxEvent = NULL;
   }
}

// NOTE : srcStripedSurf & srcSurf are mutually exclusive. One must be NULL.
//        if validateOnly is true, both may be NULL.
BEGL_Error MemoryConvertSurface(const BEGL_SurfaceConversionInfo *info,
                                NEXUS_StripedSurfaceHandle srcStripedSurf,
                                NEXUS_SurfaceHandle srcSurf,
                                bool validateOnly, MemConvertCache *cache)
{
   BEGL_Error          result = BEGL_Fail;
   NEXUS_SurfaceHandle dstSurface = NULL;
   NEXUS_Error         rc;

   if (!validateOnly)
      assert((srcStripedSurf == NULL) ^ (srcSurf == NULL));

   if (info == NULL)
      goto error;

   NEXUS_PixelFormat dstFormat;
   if (!BeglToNexusFormat(&dstFormat, info->dstFormat))
      goto error;

   if (info->dstFormat == BEGL_BufferFormat_eTILED && !M2MC_HAS_UIF_SUPPORT)
      goto error;

   // We only check the dst format for validation
   if (validateOnly)
      goto good;

   // We will need an m2mc
   if (!OpenGraphics2D(cache, info))
      goto error;

   // Wrap the destination as a NEXUS_Surface
   NEXUS_SurfaceCreateSettings   scs;
   NEXUS_Surface_GetDefaultCreateSettings(&scs);
   scs.pixelFormat       = dstFormat;
   scs.width             = info->width;
   scs.height            = info->height;
   scs.alignment         = info->dstAlignment;
   scs.pitch             = info->dstPitch;
   scs.pixelMemoryOffset = info->dstMemoryOffset;
   scs.pixelMemory       = info->dstMemoryBlock;

   dstSurface = NEXUS_Surface_Create(&scs);
   if (dstSurface == NULL)
      goto error;

   if (srcStripedSurf != NULL)
   {
      // Destripe to the target format
      NEXUS_Graphics2DDestripeBlitSettings settings;
      NEXUS_Graphics2D_GetDefaultDestripeBlitSettings(&settings);
      settings.source.stripedSurface = srcStripedSurf;
      settings.output.surface        = dstSurface;
      settings.horizontalFilter      = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;
      settings.verticalFilter        = NEXUS_Graphics2DFilterCoeffs_eAnisotropic;

      // Note: from Nexus version 18.1, NEXUS_Graphics2D_DestripeBlit() sets up the correct
      // YUV->RGB color matrix conversion internally, based on the colorMatrix in the striped
      // surface. We don't have to explicitly set that up any more.
      rc = NEXUS_Graphics2D_DestripeBlit(cache->gfx2d, &settings);
      assert(!rc);
   }
   else
   {
      // Non-sand conversions
      NEXUS_Graphics2DBlitSettings settings;

      NEXUS_Graphics2D_GetDefaultBlitSettings(&settings);
      settings.source.surface = srcSurf;
      settings.output.surface = dstSurface;
      settings.dest.surface   = dstSurface;

      rc = NEXUS_Graphics2D_Blit(cache->gfx2d, &settings);
      assert(!rc);
   }

   // Wait for the conversion to complete (timeout after 250ms)
   do
   {
      rc = NEXUS_Graphics2D_Checkpoint(cache->gfx2d, NULL);
      if (rc == NEXUS_GRAPHICS2D_QUEUED)
         rc = BKNI_WaitForEvent(cache->gfxEvent, 250);
   } while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

good:
   result = BEGL_Success;
   goto finish;

error:
   result = BEGL_Fail;

finish:
   if (dstSurface)
      NEXUS_Surface_Destroy(dstSurface);

   return result;
}
