/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#if NEXUS_HAS_GRAPHICS2D

#include "nexus_platform.h"
#include "nexus_graphics2d.h"

#include "packet_rgba.h"

#include <string.h>
#include <stdio.h>

static BM2MC_PACKET_PixelFormat get_conversion_format(BEGL_BufferFormat f)
{
   BM2MC_PACKET_PixelFormat res;

   if (f == BEGL_BufferFormat_eA8B8G8R8)
      res = BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8;
   else if (f == BEGL_BufferFormat_eR5G6B5)
      res = BM2MC_PACKET_PixelFormat_eR5_G6_B5;
   else
   {
      printf("Unsupported color format\n");
      res = BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8;
   }

   return res;
}

static const BM2MC_PACKET_Blend copyColor = { BM2MC_PACKET_BlendFactor_eSourceColor, BM2MC_PACKET_BlendFactor_eOne, false,
                                              BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false,
                                              BM2MC_PACKET_BlendFactor_eZero };
static const BM2MC_PACKET_Blend copyAlpha = { BM2MC_PACKET_BlendFactor_eSourceAlpha, BM2MC_PACKET_BlendFactor_eOne, false,
                                              BM2MC_PACKET_BlendFactor_eZero, BM2MC_PACKET_BlendFactor_eZero, false,
                                              BM2MC_PACKET_BlendFactor_eZero };

void memCopy2d_rgba(NXPL_MemoryData *data, BEGL_MemCopy2d *params)
{
   if ((params->height * params->stride) > (296 * 1024) || params->secure)
   {
      void *pkt_buffer, *next;
      size_t pkt_size;
      NEXUS_Error rc;

      NEXUS_Graphics2DHandle gfx = params->secure ? data->gfxSecure : data->gfx;

      while (1)
      {
         unsigned int packet_size =
            sizeof(BM2MC_PACKET_PacketSourceFeeders) +
            sizeof(BM2MC_PACKET_PacketOutputFeeder) +
            sizeof(BM2MC_PACKET_PacketBlend) +
            sizeof(BM2MC_PACKET_PacketCopyBlit);

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
         BM2MC_PACKET_PacketOutputFeeder *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, OutputFeeder, false);
         pPacket->plane.address = params->dstOffset;
         pPacket->plane.pitch = params->dstStride;
         pPacket->plane.format = get_conversion_format(params->format);
         pPacket->plane.width = params->width;
         pPacket->plane.height = params->height;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketSourceFeeder *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, SourceFeeder, false);
         pPacket->plane.address = params->srcOffset;
         pPacket->plane.pitch = params->stride;
         pPacket->plane.format = get_conversion_format(params->format);
         pPacket->plane.width = params->width;
         pPacket->plane.height = params->height;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketBlend *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, Blend, false);
         pPacket->color_blend = copyColor;
         pPacket->alpha_blend = copyAlpha;
         pPacket->color = 0;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketCopyBlit *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, CopyBlit, true);
         pPacket->src_rect.x = 0;
         pPacket->src_rect.y = 0;
         pPacket->src_rect.width = params->width;
         pPacket->src_rect.height = params->height;
         pPacket->out_point.x = 0;
         pPacket->out_point.y = 0;
         next = ++pPacket;
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
   else
   {
      /* only for small copies, not to be used as a fallback path */
      memcpy(params->dstCached, params->srcCached, params->height * params->stride);
      NEXUS_Memory_FlushCache(params->dstCached, params->height * params->stride);
   }
}
#endif
