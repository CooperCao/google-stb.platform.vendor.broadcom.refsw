/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
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

#include "packet_rgba.h"

#include <string.h>
#include <stdio.h>

void memCopy2d_rgba(NXPL_MemoryData *data, BEGL_MemCopy2d *params)
{
   if ((params->height * params->stride) > (296 * 1024))
   {
      void *pkt_buffer, *next;
      size_t pkt_size;
      NEXUS_Error rc;

      while (1)
      {
         rc = NEXUS_Graphics2D_GetPacketBuffer(data->gfx, &pkt_buffer, &pkt_size, 1024);
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
         pPacket->plane.pitch = params->stride;
         pPacket->plane.format = BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8;
         pPacket->plane.width = params->width;
         pPacket->plane.height = params->height;
         next = ++pPacket;
      }

      {
         BM2MC_PACKET_PacketSourceFeeder *pPacket = next;
         BM2MC_PACKET_INIT(pPacket, SourceFeeder, false);
         pPacket->plane.address = params->srcOffset;
         pPacket->plane.pitch = params->stride;
         pPacket->plane.format = BM2MC_PACKET_PixelFormat_eA8_R8_G8_B8;
         pPacket->plane.width = params->width;
         pPacket->plane.height = params->height;
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
   else
   {
      /* only for small copies, not to be used as a fallback path */
      memcpy(params->dstCached, params->srcCached, params->height * params->stride);
      NEXUS_Memory_FlushCache(params->dstCached, params->height * params->stride);
   }
}
#endif
