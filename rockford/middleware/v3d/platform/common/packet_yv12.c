/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
This is a default implementation of a Nexus platform layer used by V3D.
This illustrates one way in which the abstract memory interface might be
implemented. You can replace this with your own custom version if preferred.
=============================================================================*/

#if NEXUS_HAS_GRAPHICS2D

#include "nexus_platform.h"
#include "nexus_graphics2d.h"

#include "packet_yv12.h"

#include <string.h>
#include <stdio.h>

#define SHIFT 10

/***************************************************************************
Description:
   YCbCr to RGB color matrix table.
****************************************************************************/
/* R = Y * 1.164 + Cr * 1.596 - 223 */
/* G = Y * 1.164 - Cr * 0.813 - Cb * 0.391 + 135 */
/* B = Y * 1.164 + Cb * 2.018 - 277 */
static NEXUS_Graphics2DColorMatrix s_ai32_Matrix_YCbCrtoRGB =
{
   SHIFT,
   {
      (int32_t) ( 1.164f * (1 << SHIFT)),   /*  Y factor for R */
      (int32_t) 0,                          /* Cb factor for R */
      (int32_t) ( 1.596f * (1 << SHIFT)),   /* Cr factor for R */
      (int32_t) 0,                          /*  A factor for R */
      (int32_t) (-223 * (1 << SHIFT)),      /* Increment for R */
      (int32_t) ( 1.164f * (1 << SHIFT)),   /*  Y factor for G */
      (int32_t) (-0.391f * (1 << SHIFT)),   /* Cb factor for G */
      (int32_t) (-0.813f * (1 << SHIFT)),   /* Cr factor for G */
      (int32_t) 0,                          /*  A factor for G */
      (int32_t) (134 * (1 << SHIFT)),       /* Increment for G */
      (int32_t) ( 1.164f * (1 << SHIFT)),   /*  Y factor for B */
      (int32_t) ( 2.018f * (1 << SHIFT)),   /* Cb factor for B */
      (int32_t) 0,                          /* Cr factor for B */
      (int32_t) 0,                          /*  A factor for B */
      (int32_t) (-277 * (1 << SHIFT)),      /* Increment for B */
      (int32_t) 0,                          /*  Y factor for A */
      (int32_t) 0,                          /* Cb factor for A */
      (int32_t) 0,                          /* Cr factor for A */
      (int32_t) (1 << SHIFT),               /*  A factor for A */
      (int32_t) 0,                          /* Increment for A */
   }
};

static const BM2MC_PACKET_Blend combColor = { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, false,
                                              BM2MC_PACKET_BlendFactor_eDestinationColor, BM2MC_PACKET_BlendFactor_eOne, false,
                                              BM2MC_PACKET_BlendFactor_eZero };
static const BM2MC_PACKET_Blend copyAlpha = { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, false,
                                              BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false,
                                              BM2MC_PACKET_BlendFactor_eZero };
static const BM2MC_PACKET_Blend copyColor = { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, false,
                                              BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false,
                                              BM2MC_PACKET_BlendFactor_eZero };
static const BM2MC_PACKET_Blend constOne = { BM2MC_PACKET_BlendFactor_eOne, BM2MC_PACKET_BlendFactor_eOne, false,
                                               BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false,
                                               BM2MC_PACKET_BlendFactor_eZero };

void memCopy2d_yv12(NXPL_MemoryData *data, BEGL_MemCopy2d *params)
{
   void *pkt_buffer, *next;
   size_t pkt_size;
   NEXUS_Error rc;
   int y;

   /* 420 -> RGBA (spec from /system/core/include/system/graphics.h) */
   unsigned int stride = (params->width + (16 - 1)) & ~(16 - 1);
   unsigned int y_size = stride * params->height;
   unsigned int c_stride = ((stride / 2) + (16 - 1)) & ~(16 - 1);
   unsigned int c_size = c_stride * params->height / 2;
   unsigned int cr_offset = y_size;
   unsigned int cb_offset = y_size + c_size;

   /* YUV 420 -> 422 */

   BM2MC_PACKET_ColorMatrix ai32_Matrix_YCbCrtoRGB;
   NEXUS_Graphics2D_ConvertColorMatrix(&s_ai32_Matrix_YCbCrtoRGB, &ai32_Matrix_YCbCrtoRGB);

   NEXUS_Graphics2DHandle gfx = params->secure ? data->gfxSecure : data->gfx;
   NEXUS_Addr yuvScratchPhys = params->secure ? data->yuvScratchPhysSecure : data->yuvScratchPhys;

   while (1)
   {
      unsigned int packet_size =
         sizeof(BM2MC_PACKET_PacketSourceFeeders) +
         sizeof(BM2MC_PACKET_PacketDestinationFeeder) +
         sizeof(BM2MC_PACKET_PacketOutputFeeder) +
         sizeof(BM2MC_PACKET_PacketBlend) +
         sizeof(BM2MC_PACKET_PacketScaleBlendBlit) +
         sizeof(BM2MC_PACKET_PacketSourceColorMatrixEnable) +
         sizeof(BM2MC_PACKET_PacketSourceColorMatrix) +
         sizeof(BM2MC_PACKET_PacketSourceFeeder) +
         sizeof(BM2MC_PACKET_PacketDestinationNone) +
         sizeof(BM2MC_PACKET_PacketOutputFeeder) +
         sizeof(BM2MC_PACKET_PacketBlend) +
         sizeof(BM2MC_PACKET_PacketBlendBlit) +
         sizeof(BM2MC_PACKET_PacketSourceColorMatrixEnable);
      packet_size *= (params->height + (STRIP_HEIGHT - 1)) / STRIP_HEIGHT;
      packet_size +=
         sizeof(BM2MC_PACKET_PacketFilterEnable) +
         sizeof(BM2MC_PACKET_PacketSourceColorkeyEnable) +
         sizeof(BM2MC_PACKET_PacketDestinationColorkeyEnable) +
         sizeof(BM2MC_PACKET_PacketBlendColor);

      rc = NEXUS_Graphics2D_GetPacketBuffer(gfx, &pkt_buffer, &pkt_size, packet_size);
      BDBG_ASSERT(!rc);
      if (!pkt_size) {
         rc = BERR_OS_ERROR;
         while (rc != BERR_SUCCESS)
         {
            /* 20ms timeout */
            rc = BKNI_WaitForEvent(data->packetSpaceAvailableEvent, 20);
            if (rc == BERR_TIMEOUT)
               printf("TIMEOUT waiting for packet space in M2MC\n");
         }
      }
      else
         break;
   }

   next = pkt_buffer;
   {
      BM2MC_PACKET_PacketFilterEnable *pPacket = next;
      BM2MC_PACKET_INIT(pPacket, FilterEnable, false);
      pPacket->enable = 0;
      next = ++pPacket;
   }

   {
      BM2MC_PACKET_PacketSourceColorkeyEnable *pPacket = next;
      BM2MC_PACKET_INIT(pPacket, SourceColorkeyEnable, false);
      pPacket->enable = 0;
      next = ++pPacket;
   }

   {
      BM2MC_PACKET_PacketDestinationColorkeyEnable *pPacket = next;
      BM2MC_PACKET_INIT(pPacket, DestinationColorkeyEnable, false);
      pPacket->enable = 0;
      next = ++pPacket;
   }

   {
      BM2MC_PACKET_PacketBlendColor *pPacket = next;
      BM2MC_PACKET_INIT(pPacket, BlendColor, false);
      pPacket->color = 0;
      next = ++pPacket;
   }

   for (y = 0; y < params->height; y += STRIP_HEIGHT)
   {
      int stripHeight;
      if ((y + STRIP_HEIGHT) < params->height)
         stripHeight = STRIP_HEIGHT;
      else
         stripHeight = params->height - y;

      /* YUV 420 -> 422 */
      {
         int offset = (y / 2) * c_stride;
         BM2MC_PACKET_PacketSourceFeeders *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, SourceFeeders, false);

         pPacket->plane0.address = params->srcOffset + cb_offset + offset;
         pPacket->plane0.pitch = c_stride;
         pPacket->plane0.format = BM2MC_PACKET_PixelFormat_eCb8;
         pPacket->plane0.width = params->width / 2;
         pPacket->plane0.height = stripHeight / 2;

         pPacket->plane1.address = params->srcOffset + cr_offset + offset;
         pPacket->plane1.pitch = c_stride;
         pPacket->plane1.format = BM2MC_PACKET_PixelFormat_eCr8;
         pPacket->plane1.width = params->width / 2;
         pPacket->plane1.height = stripHeight / 2;

         pPacket->color = 0;
         next = ++pPacket;
      }

      {
         int offset = y * stride;
         BM2MC_PACKET_PacketDestinationFeeder *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, DestinationFeeder, false);

         pPacket->plane.address = params->srcOffset + offset;        /* Y */
         pPacket->plane.pitch = stride;
         pPacket->plane.format = BM2MC_PACKET_PixelFormat_eY8;
         pPacket->plane.width = params->width;
         pPacket->plane.height = stripHeight;
         pPacket->color = 0;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
         pPacket->plane.address = yuvScratchPhys;
         pPacket->plane.pitch = params->width * 2;
         pPacket->plane.format = BM2MC_PACKET_PixelFormat_eCr8_Y18_Cb8_Y08;
         pPacket->plane.width = params->width;
         pPacket->plane.height = stripHeight;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketBlend *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, Blend, false);
         pPacket->color_blend = combColor;
         pPacket->alpha_blend = copyAlpha;
         pPacket->color = 0;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketScaleBlendBlit *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, ScaleBlendBlit, true);
         pPacket->src_rect.x = 0;
         pPacket->src_rect.y = 0;
         pPacket->src_rect.width = params->width / 2;
         pPacket->src_rect.height = stripHeight / 2;
         pPacket->out_rect.x = 0;
         pPacket->out_rect.y = 0;
         pPacket->out_rect.width = params->width;
         pPacket->out_rect.height = stripHeight;
         pPacket->dst_point.x = 0;
         pPacket->dst_point.y = 0;
         next = ++pPacket;
      }

      /* RGB */
      {
         BM2MC_PACKET_PacketSourceColorMatrixEnable *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, SourceColorMatrixEnable, false);
         pPacket->enable = 1;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketSourceColorMatrix *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, SourceColorMatrix, false);
         pPacket->matrix = ai32_Matrix_YCbCrtoRGB;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketSourceFeeder *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, SourceFeeder, false);
         pPacket->plane.address = yuvScratchPhys;
         pPacket->plane.pitch = params->width * 2;
         pPacket->plane.format = BM2MC_PACKET_PixelFormat_eCr8_Y18_Cb8_Y08;
         pPacket->plane.width = params->width;
         pPacket->plane.height = stripHeight;
         pPacket->color = 0;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketDestinationNone *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, DestinationNone, false);
         next = ++pPacket;
      }

      {
         int offset = y * params->dstStride;
         BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
         pPacket->plane.address = params->dstOffset + offset;
         pPacket->plane.pitch = params->dstStride;
         pPacket->plane.format = BM2MC_PACKET_PixelFormat_eA8_B8_G8_R8;
         pPacket->plane.width = params->width;
         pPacket->plane.height = stripHeight;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketBlend *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, Blend, false);
         pPacket->color_blend = copyColor;
         pPacket->alpha_blend = constOne;
         pPacket->color = 0;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketBlendBlit *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, BlendBlit, true);
         pPacket->src_rect.x = 0;
         pPacket->src_rect.y = 0;
         pPacket->src_rect.width = params->width;
         pPacket->src_rect.height = stripHeight;
         pPacket->out_point.x = 0;
         pPacket->out_point.y = 0;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketSourceColorMatrixEnable *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, SourceColorMatrixEnable, false);
         pPacket->enable = 0;
         next = ++pPacket;
      }
   }

   rc = NEXUS_Graphics2D_PacketWriteComplete(gfx, (uint8_t*)next - (uint8_t*)pkt_buffer);
   BDBG_ASSERT(!rc);

   rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
   if (rc == NEXUS_GRAPHICS2D_QUEUED)
   {
      rc = BERR_OS_ERROR;
      while (rc != BERR_SUCCESS)
      {
         /* 20ms timeout */
         rc = BKNI_WaitForEvent(data->checkpointEvent, 20);
         if (rc == BERR_TIMEOUT)
            printf("TIMEOUT waiting for blit to complete\n");
      }
   }
}
#endif
