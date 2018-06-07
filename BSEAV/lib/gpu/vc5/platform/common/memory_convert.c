/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#if NEXUS_HAS_GRAPHICS2D

#include "display_helpers.h"

#include "bchp_common.h"
#ifdef BCHP_M2MC_REG_START
#include "bchp_m2mc.h"
#endif
#include "memory_convert.h"
#include "assert.h"

#include "bdbg.h"

#ifdef BCHP_M2MC_REG_START
#define M2MC_HAS_UIF_SUPPORT (BCHP_M2MC_REVISION_MAJOR_DEFAULT >= 2)
#else
#define M2MC_HAS_UIF_SUPPORT 0
#endif

static void EventHandler(void *data, int unused)
{
   BSTD_UNUSED(unused);
   BKNI_SetEvent((BKNI_EventHandle)data);
}

static bool HasMipmapper(void)
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
   bool useMipmapper = HasMipmapper() && info->dst.format == BEGL_BufferFormat_eTILED;

   if (cache->gfx2d != NULL && cache->gfxEvent != NULL &&
       cache->secure == info->secure && cache->mipmapper == useMipmapper)
      return true;  // We already have what we need

   MemoryConvertClearCache(cache);

   BKNI_CreateEvent(&cache->gfxEvent);
   if (cache->gfxEvent == NULL)
      return false;

   BKNI_CreateEvent(&cache->packetSpaceAvailableEvent);
   if (cache->packetSpaceAvailableEvent == NULL)
   {
      MemoryConvertClearCache(cache);
      return false;
   }

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
      gfxSettings.packetSpaceAvailable.callback = EventHandler;
      gfxSettings.packetSpaceAvailable.context = cache->packetSpaceAvailableEvent;
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

   if (cache->packetSpaceAvailableEvent)
   {
      BKNI_DestroyEvent(cache->packetSpaceAvailableEvent);
      cache->packetSpaceAvailableEvent = NULL;
   }
}

static void *GetPacketBuffer(MemConvertCache *cache, size_t size)
{
   void *pkt_buffer = NULL;
   size_t pkt_size = 0;
   NEXUS_Error rc;

   while (1)
   {
      rc = NEXUS_Graphics2D_GetPacketBuffer(cache->gfx2d, &pkt_buffer, &pkt_size, size);
      BDBG_ASSERT(!rc);
      if (!pkt_size)
      {
         rc = BERR_OS_ERROR;
         while (rc != BERR_SUCCESS)
         {
            /* 20ms timeout */
            rc = BKNI_WaitForEvent(cache->packetSpaceAvailableEvent, 20);
         }
      }
      else
      {
         BDBG_ASSERT(pkt_size >= size);
         break;
      }
   }
   return pkt_buffer;
}

static bool ExecPacketBuffer(MemConvertCache *cache, size_t size)
{
   NEXUS_Error rc;

   rc = NEXUS_Graphics2D_PacketWriteComplete(cache->gfx2d, size);
   BDBG_ASSERT(!rc);

   // Wait for the conversion to complete (timeout after 250ms)
   do
   {
      rc = NEXUS_Graphics2D_Checkpoint(cache->gfx2d, NULL);
      if (rc == NEXUS_GRAPHICS2D_QUEUED)
         rc = BKNI_WaitForEvent(cache->gfxEvent, 250);
   } while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);
   return rc == NEXUS_SUCCESS;
}

static BM2MC_PACKET_PixelFormat BeglToM2MCFormat(BEGL_BufferFormat format)
{
   NEXUS_PixelFormat result;
   if (!BeglToNexusFormat(&result, format))
      return BM2MC_PACKET_PixelFormat_eUnknown;
   return (BM2MC_PACKET_PixelFormat)result; //Nexus and M2MC pixel formats match
}

static NEXUS_MatrixCoefficients BEGLColorimetryToNexus(BEGL_Colorimetry col)
{
   switch (col)
   {
   case BEGL_Colorimetry_RGB                    : return NEXUS_MatrixCoefficients_eHdmi_RGB;
   case BEGL_Colorimetry_BT_709                 : return NEXUS_MatrixCoefficients_eItu_R_BT_709;
   case BEGL_Colorimetry_Unknown                : return NEXUS_MatrixCoefficients_eUnknown;
   case BEGL_Colorimetry_Dvi_Full_Range_RGB     : return NEXUS_MatrixCoefficients_eDvi_Full_Range_RGB;
   case BEGL_Colorimetry_FCC                    : return NEXUS_MatrixCoefficients_eFCC;
   case BEGL_Colorimetry_BT_470_2_BG            : return NEXUS_MatrixCoefficients_eItu_R_BT_470_2_BG;
   case BEGL_Colorimetry_Smpte_170M             : return NEXUS_MatrixCoefficients_eSmpte_170M;
   case BEGL_Colorimetry_Smpte_240M             : return NEXUS_MatrixCoefficients_eSmpte_240M;
   case BEGL_Colorimetry_XvYCC_709              : return NEXUS_MatrixCoefficients_eXvYCC_709;
   case BEGL_Colorimetry_XvYCC_601              : return NEXUS_MatrixCoefficients_eXvYCC_601;
   case BEGL_Colorimetry_BT_2020_NCL            : return NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL;
   case BEGL_Colorimetry_BT_2020_CL             : return NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL;
   case BEGL_Colorimetry_Hdmi_Full_Range_YCbCr  : return NEXUS_MatrixCoefficients_eHdmi_Full_Range_YCbCr;
   default                                      : return NEXUS_MatrixCoefficients_eXvYCC_601;
   }
}

static const BM2MC_PACKET_Blend copyColor =
{
      .a = BM2MC_PACKET_BlendFactor_eSourceColor,
      .b = BM2MC_PACKET_BlendFactor_eOne,
      .sub_cd = false,
      .c = BM2MC_PACKET_BlendFactor_eZero,
      .d = BM2MC_PACKET_BlendFactor_eZero,
      .sub_e = false,
      .e = BM2MC_PACKET_BlendFactor_eZero
};

static const BM2MC_PACKET_Blend copyAlpha =
{
      .a = BM2MC_PACKET_BlendFactor_eSourceAlpha,
      .b = BM2MC_PACKET_BlendFactor_eOne,
      .sub_cd = false,
      .c = BM2MC_PACKET_BlendFactor_eZero,
      .d = BM2MC_PACKET_BlendFactor_eZero,
      .sub_e = false,
      .e = BM2MC_PACKET_BlendFactor_eZero
};

static bool ConvertStriped(const BEGL_SurfaceConversionInfo *info,
      MemConvertCache *cache)
{
   enum /* plane index */
   {
      luma = 0,
      chroma = 1
   };

   /* For now both Android and Nexus use non-interlaced frames only */
   bool interlaced = false;
   bool bottomField = false;

   BM2MC_PACKET_StripedPlane srcPlane =
   {
      .luma_address           = info->src[luma].offset +
            (bottomField ? info->stripeWidth : 0),
      .chroma_address         = info->src[chroma].offset +
            (bottomField ? info->stripeWidth : 0),
      .bottom_field_luma_address    = 0, /* TODO: add interlaced video support */
      .bottom_field_chroma_address  = 0, /* TODO: add interlaced video support */
      .width                  = info->width,
      .height                 = info->height,
      .stripe_width           = info->stripeWidth,
      .stripe_pitch           = interlaced ?
            info->stripeWidth * 2 : info->stripeWidth,
      .luma_stripe_height     = info->src[luma].stripeHeight,
      .chroma_stripe_height   = info->src[chroma].stripeHeight,
      .luma_format            = BeglToM2MCFormat(info->src[luma].format),
      .chroma_format          = BeglToM2MCFormat(info->src[chroma].format),
   };
   if (srcPlane.luma_format == BM2MC_PACKET_PixelFormat_eUnknown ||
         srcPlane.chroma_format == BM2MC_PACKET_PixelFormat_eUnknown)
      return false;

   BM2MC_PACKET_Plane outPlane =
   {
      .address = info->dst.offset,
      .pitch   = info->dst.pitch,
      .format  = BeglToM2MCFormat(info->dst.format),
      .width   = info->width,
      .height  = info->height
   };
   if (outPlane.format == BM2MC_PACKET_PixelFormat_eUnknown)
      return false;

   static const BM2MC_PACKET_FilterCoeffs bilinear =
   {
      /* copied from magnum/portinginterface/grc/src/bgrc_packet_priv.c
       * there was no explanation what they do */
      .coeffs =
      {
         { 0x0000, 0x0000, 0x0100 },
         { 0x0000, 0x0000, 0x0080 }
      }
   };

   static const bool enableColorMatrix = true;
   NEXUS_Graphics2DColorMatrix nexusMatrix;
   NEXUS_Graphics2D_GetColorMatrix(
         BEGLColorimetryToNexus(info->srcColorimetry),
         &nexusMatrix);
   BM2MC_PACKET_ColorMatrix colorMatrix;
   NEXUS_Graphics2D_ConvertColorMatrix(&nexusMatrix, &colorMatrix);

   bool chromaFilter = true; /* default */

   uint8_t mipLevel = 0; //TODO

   BM2MC_PACKET_Rectangle srcRect =
   {
      .x = 0,
      .y = 0,
      .width = info->width,
      .height = info->height
   };
   BM2MC_PACKET_Rectangle outRect = srcRect;

   static const uint32_t defaultColour = 0xFF000000;

   void *buffer = GetPacketBuffer(cache,
         sizeof(BM2MC_PACKET_PacketStripedSourceFeeders) +
         sizeof(BM2MC_PACKET_PacketDestinationColor) +
         sizeof(BM2MC_PACKET_PacketOutputFeeder) +
         sizeof(BM2MC_PACKET_PacketBlend) +
         sizeof(BM2MC_PACKET_PacketRop) +
         sizeof(BM2MC_PACKET_PacketFilterEnable) +
         sizeof(BM2MC_PACKET_PacketFilter) +
         sizeof(BM2MC_PACKET_PacketSourceColorMatrixEnable) +
         sizeof(BM2MC_PACKET_PacketSourceColorMatrix) +
         sizeof(BM2MC_PACKET_PacketSourceColorkeyEnable) +
         sizeof(BM2MC_PACKET_PacketDestinationColorkeyEnable) +
         sizeof(BM2MC_PACKET_PacketAlphaPremultiply) +
         sizeof(BM2MC_PACKET_PacketMirror) +
         sizeof(BM2MC_PACKET_PacketFixedScale) +
         sizeof(BM2MC_PACKET_PacketSourceControl) +
         sizeof(BM2MC_PACKET_PacketMipmapControl) +
         sizeof(BM2MC_PACKET_PacketOutputControl) +
         sizeof(BM2MC_PACKET_PacketScaleBlit));
   if (!buffer)
      return false;

   const void *start = buffer;

   BM2MC_PACKET_WRITE_StripedSourceFeeders(buffer, srcPlane, defaultColour, false);
   BM2MC_PACKET_WRITE_DestinationColor(buffer, defaultColour, false);
   BM2MC_PACKET_WRITE_OutputFeeder(buffer, outPlane, false);
   BM2MC_PACKET_WRITE_Blend(buffer, copyColor, copyAlpha, defaultColour, false);
   BM2MC_PACKET_WRITE_Rop(buffer, 0xCC, 0, 0, 0, defaultColour, false);
   BM2MC_PACKET_WRITE_FilterEnable(buffer, true, false);
   BM2MC_PACKET_WRITE_Filter(buffer, bilinear, bilinear, false);
   BM2MC_PACKET_WRITE_SourceColorMatrixEnable(buffer, enableColorMatrix, false);
   if (enableColorMatrix)
   {
      BM2MC_PACKET_WRITE_SourceColorMatrix(buffer, colorMatrix, false);
   }
   BM2MC_PACKET_WRITE_SourceColorkeyEnable(buffer, false, false);
   BM2MC_PACKET_WRITE_DestinationColorkeyEnable(buffer, false, false);
   BM2MC_PACKET_WRITE_AlphaPremultiply(buffer, false, false);
   BM2MC_PACKET_WRITE_Mirror(buffer, false, false, false, false, false, false, false);
   BM2MC_PACKET_WRITE_FixedScale(buffer, 0, 0, 0, 0, 0, false);
   BM2MC_PACKET_WRITE_SourceControl(buffer, false, chromaFilter, false);
   BM2MC_PACKET_WRITE_MipmapControl(buffer, mipLevel, false);
   BM2MC_PACKET_WRITE_OutputControl(buffer, false, chromaFilter, false);

   BM2MC_PACKET_WRITE_ScaleBlit(buffer, srcRect, outRect, /*exec=*/true);

   return ExecPacketBuffer(cache, (uint8_t*)buffer - (uint8_t*)start);
}

BEGL_Error MemoryConvertSurface(const BEGL_SurfaceConversionInfo *info,
                                bool validateOnly, MemConvertCache *cache)
{
   BEGL_Error result = BEGL_Fail;

   if (info == NULL)
      goto error;

   if (!BeglFormatIsSand(info->src[0].format) ||
        BeglFormatIsSand(info->dst.format))
      goto error;

   if (info->dst.format == BEGL_BufferFormat_eTILED && !M2MC_HAS_UIF_SUPPORT)
      goto error;

   // We only check the format for validation
   if (validateOnly)
      goto good;

   // We will need an m2mc
   if (!OpenGraphics2D(cache, info))
      goto error;

   if (!ConvertStriped(info, cache))
      goto error;

good:
   result = BEGL_Success;
   goto finish;

error:
   result = BEGL_Fail;

finish:

   return result;
}
#endif
