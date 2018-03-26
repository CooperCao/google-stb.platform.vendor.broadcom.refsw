/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bmuxlib_asp_types.h"

#include "bmuxlib_asp_ts_memory_priv.h"
#include "bmuxlib_asp_ts_consts.h"

BDBG_MODULE(BMUXLIB_ASP_TS_MEMORY);

static uint64_t
BMUXlib_ASP_TS_S_Memory_Packet_Pop(
   BMUXlib_ASP_TS_Memory_Handle hMemoryHandle
   )
{
   uint64_t uiBufferOffset = 0;

   uiBufferOffset = hMemoryHandle->stTSPacketBuffer.uiBase + ( (uint64_t) hMemoryHandle->stTSPacketBuffer.auiFreeList[hMemoryHandle->stTSPacketBuffer.uiValid])*BMUXLIB_ASP_TS_PACKET_SIZE;
   hMemoryHandle->stTSPacketBuffer.auiFreeList[hMemoryHandle->stTSPacketBuffer.uiValid] = 0;
   hMemoryHandle->stTSPacketBuffer.uiValid++;
   hMemoryHandle->stTSPacketBuffer.uiValid %= hMemoryHandle->stTSPacketBuffer.uiNumEntries;

   return uiBufferOffset;
}

static void
BMUXlib_ASP_TS_S_Memory_Packet_Push(
   BMUXlib_ASP_TS_Memory_Handle hMemoryHandle,
   size_t uiIndex
   )
{
   hMemoryHandle->stTSPacketBuffer.auiFreeList[hMemoryHandle->stTSPacketBuffer.uiRead] = uiIndex;
   hMemoryHandle->stTSPacketBuffer.uiRead++;
   hMemoryHandle->stTSPacketBuffer.uiRead %= hMemoryHandle->stTSPacketBuffer.uiNumEntries;
}

void
BMUXlib_ASP_TS_P_Memory_Packet_Start(
   BMUXlib_ASP_TS_Memory_Handle hMemoryHandle
   )
{
   hMemoryHandle->stTSPacketBuffer.uiBase = hMemoryHandle->pstMemoryInterface->uiOffset;
   hMemoryHandle->stTSPacketBuffer.uiValid = 0;
   hMemoryHandle->stTSPacketBuffer.uiRead = 0;
   hMemoryHandle->stTSPacketBuffer.uiNumEntries = 0;
   {
      size_t uiSize;
      for ( uiSize = 0; uiSize < hMemoryHandle->pstMemoryInterface->uiSize; uiSize += BMUXLIB_ASP_TS_PACKET_SIZE )
      {
         if ( BMUXLIB_ASP_TS_MAX_TS_PACKETS_PER_INSTANCE <= hMemoryHandle->stTSPacketBuffer.uiNumEntries )
         {
            BDBG_WRN(("More TS buffer data provided than can be supported (%d > %d)", hMemoryHandle->pstMemoryInterface->uiSize, BMUXLIB_ASP_TS_MAX_TS_PACKETS_PER_INSTANCE*BMUXLIB_ASP_TS_PACKET_SIZE ));
            break;
         }
         hMemoryHandle->stTSPacketBuffer.auiFreeList[hMemoryHandle->stTSPacketBuffer.uiNumEntries] = hMemoryHandle->stTSPacketBuffer.uiNumEntries;
         hMemoryHandle->stTSPacketBuffer.uiNumEntries++;
      }
   }
}

bool_t
BMUXlib_ASP_TS_P_Memory_Packet_Available(
   BMUXlib_ASP_TS_Memory_Handle hMemoryHandle
   )
{
   uint32_t uiTemp = hMemoryHandle->stTSPacketBuffer.uiValid + 1;
   uiTemp %= hMemoryHandle->stTSPacketBuffer.uiNumEntries;

   return ( uiTemp != hMemoryHandle->stTSPacketBuffer.uiRead );
}

bool_t
BMUXlib_ASP_TS_P_Memory_Packet_StartWrite(
   BMUXlib_ASP_TS_Memory_Handle hMemoryHandle,
   uint8_t *auiPacket,
   uint64_t *puiBufferAddress,
   void **pDMAToken
   )
{
   BDBG_ASSERT( NULL == *pDMAToken );

   if ( FALSE == BMUXlib_ASP_TS_P_Memory_Packet_Available( hMemoryHandle ) )
   {
      goto error;
   }

   {
      uint64_t uiBufferAddress = BMUXlib_ASP_TS_S_Memory_Packet_Pop( hMemoryHandle );

      hMemoryHandle->pstDMAInterface->fDccm2Dram(
           hMemoryHandle->pstDMAInterface->pvContext,
           uiBufferAddress,
           auiPacket,
           BMUXLIB_ASP_TS_PACKET_SIZE,
           pDMAToken
           );

      if ( NULL == pDMAToken )
      {
         goto error;
      }

      *puiBufferAddress = uiBufferAddress;
   }

   return TRUE;
error:
   return FALSE;
}

bool_t
BMUXlib_ASP_TS_P_Memory_Packet_FinishWrite(
   BMUXlib_ASP_TS_Memory_Handle hMemoryHandle,
   void **pDMAToken
   )
{
   BSTD_UNUSED( hMemoryHandle );

   if ( FALSE == hMemoryHandle->pstDMAInterface->fIdle( hMemoryHandle->pstDMAInterface->pvContext, pDMAToken ) )
   {
      goto error;
   }

   *pDMAToken = NULL;

   return TRUE;
error:
   return FALSE;
}

void
BMUXlib_ASP_TS_P_Memory_Output_Descriptor_Initialize(
   BMUXlib_ASP_TS_Memory_Handle hMemoryHandle,
   BMUXlib_ASP_Output_Handle hOutputHandle,
   uint64_t uiBufferAddress,
   size_t uiPayloadSize,
   uint16_t uiPIDChannelIndex,
   BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor,
   BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata
   )
{
   BMUXlib_ASP_P_Output_Descriptor_Initialize( hOutputHandle, uiBufferAddress, uiPayloadSize, uiPIDChannelIndex, pstOutputDescriptor, pstOutputDescriptorMetadata );

   /* Set descriptor metadata for use by ProcessCompletedBuffers */
   pstOutputDescriptorMetadata->fConsumeResource = (BMUXlib_ASP_P_Output_ConsumeResource) BMUXlib_ASP_TS_S_Memory_Packet_Push;
   pstOutputDescriptorMetadata->pvContext = hMemoryHandle;
   pstOutputDescriptorMetadata->uiResourceId = (uiBufferAddress - hMemoryHandle->stTSPacketBuffer.uiBase)/BMUXLIB_ASP_TS_PACKET_SIZE;
}
