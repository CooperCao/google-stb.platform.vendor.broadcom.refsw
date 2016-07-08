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
static const BM2MC_PACKET_Blend copyAlpha = { BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eOne, false,
                                              BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false,
                                              BM2MC_PACKET_BlendFactor_eZero };
static const BM2MC_PACKET_Blend copyColor = { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, false,
                                              BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false,
                                              BM2MC_PACKET_BlendFactor_eZero };
static const BM2MC_PACKET_Blend constAlpha = { BM2MC_PACKET_BlendFactor_eConstantAlpha, BM2MC_PACKET_BlendFactor_eOne, false,
                                               BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false,
                                               BM2MC_PACKET_BlendFactor_eZero };

void memCopy2d_yv12(NXPL_MemoryData *data, BEGL_MemCopy2d *params)
{
   void *pkt_buffer, *next;
   size_t pkt_size;
   NEXUS_Error rc;
   int y;

   /* 420 -> RGBA */
   unsigned int stride = (params->width + (16 - 1)) & ~(16 - 1);
   unsigned int cstride = (stride / 2 + (16 - 1)) & ~(16 - 1);
   unsigned int cr_offset = stride * params->height;
   unsigned int cb_offset = cr_offset + (cstride * (params->height / 2));

   while (1)
   {
      /* Packet space is rather big as we setup an interleaved conversion for 16 lines at a time.
         Its currently 328bytes per strip, and at a max of 2k image, this is about 42kb, round up for
         future proofing */
      rc = NEXUS_Graphics2D_GetPacketBuffer(data->gfx, &pkt_buffer, &pkt_size, 48 * 1024);
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

   for (y = 0; y < params->height; y += STRIP_HEIGHT)
   {
      int stripHeight;
      if ((y + STRIP_HEIGHT) < params->height)
         stripHeight = STRIP_HEIGHT;
      else
         stripHeight = params->height - y;

      /* YUV 420 -> 422 */
      {

         int offset = (y / 2) * cstride;
         BM2MC_PACKET_PacketSourceFeeders *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, SourceFeeders, false);

         pPacket->plane0.address = params->srcOffset + cb_offset + offset;
         pPacket->plane0.pitch = cstride;
         pPacket->plane0.format = BM2MC_PACKET_PixelFormat_eCb8;
         pPacket->plane0.width = params->width / 2;
         pPacket->plane0.height = stripHeight / 2;

         pPacket->plane1.address = params->srcOffset + cr_offset + offset;
         pPacket->plane1.pitch = cstride;
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
         pPacket->plane.address = data->yuvScratchPhys;
         pPacket->plane.pitch = params->width * 2;
         pPacket->plane.format = BM2MC_PACKET_PixelFormat_eCr8_Y18_Cb8_Y08;
         pPacket->plane.width = params->width;
         pPacket->plane.height = stripHeight;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketBlend *pPacket = (BM2MC_PACKET_PacketBlend *)next;
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
         NEXUS_Graphics2D_ConvertColorMatrix(&s_ai32_Matrix_YCbCrtoRGB, &pPacket->matrix);
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketDestinationColor *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, DestinationColor, false);
         pPacket->color = 0;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketSourceFeeder *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, SourceFeeder, false);
         pPacket->plane.address = data->yuvScratchPhys;
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
         int pitch = params->width * 4;
         int offset = y * pitch;
         BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
         pPacket->plane.address = params->dstOffset + offset;
         pPacket->plane.pitch = pitch;
         pPacket->plane.format = BM2MC_PACKET_PixelFormat_eA8_B8_G8_R8;
         pPacket->plane.width = params->width;
         pPacket->plane.height = stripHeight;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketBlend *pPacket = (BM2MC_PACKET_PacketBlend *)next;
         BM2MC_PACKET_INIT(pPacket, Blend, false);
         pPacket->color_blend = copyColor;
         pPacket->alpha_blend = constAlpha;
         pPacket->color = 0xFF000000;
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

   rc = NEXUS_Graphics2D_PacketWriteComplete(data->gfx, (uint8_t*)next - (uint8_t*)pkt_buffer);
   BDBG_ASSERT(!rc);

   rc = NEXUS_Graphics2D_Checkpoint(data->gfx, NULL);
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
