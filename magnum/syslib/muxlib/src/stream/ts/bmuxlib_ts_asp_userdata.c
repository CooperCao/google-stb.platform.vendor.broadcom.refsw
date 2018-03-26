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

#include "bmuxlib_ts_asp_priv.h"
#include "bmuxlib_ts_asp_userdata.h"
#include "bmuxlib_asp_ts_userdata.h"

BDBG_MODULE(BMUXlib_TS_ASP_USERDATA);

/* TODO List
 *
 * System Data Processing
 *  1) Queue system data packets internally
 *  2) On each DoMux()
 *     - Release muxed system data
 *     - Read video encoder index
 *     - Find nearest I frame
 *     - Set ESCR for system data prior to I-frame's ESCR
 *     - Add system data to FW queue
 *
 * Phase 0.1:
 *    + System Data Only
 *
 * Phase 0.2:
 *    + Untimed User Data
 *
 * Phase 0.5:
 *    + Timed User Data
 *
 * Phase 1.0: Timed User Data
 *    + Discontinuity Handling
 */

/* User Data Flow
 *
 *  Host to Internal ASP Queue: stInputQueue
 *  Internal ASP to FW Queue: stUserData
 *
 *  Queueing Data
 *   - Host Queues Data:
 *     - stInputQueue.uiWriteOffset incremented
 *
 *    - ASP SW Queues Data:
 *     - Buffer Offset is locked
 *     - stUserData.uiWriteOffset incremented
 *     - stInputQueue.uiShadowReadOffset incremented
 *
 *  Consumes Data
 *   - FW consumes Data:
 *      - stUserData.uiReadOffset incremented
 *
 *   - ASP SW Consumes Data:
 *      - Buffer Offset is freed
 *      - stUserData.uiShadowReadOffset incremented
 *      - stInputQueue.uiReadOffset incremented
 *      - uiCompletedCount incremented
 */

#define BMUXLIB_TS_P_MOD300_SUB32(_ui64Timestamp, _ui32Offset) ( (_ui64Timestamp) >= (_ui32Offset) ) ? ((_ui64Timestamp) - (_ui32Offset)) : (((uint64_t)0x257 << 32 ) | (((_ui64Timestamp) - (_ui32Offset)) & 0xFFFFFFFF ));

void
BMUXlib_TS_ASP_P_ProcessNewUserData(
      BMUXlib_TS_ASP_Handle hMuxTS
      )
{
   unsigned uiNumDescriptors = 0;
   uint64_t uiPacket2PacketDelta = 0;

   /* Calculate PKT2PKT Delta every MSP because it can change */
   uiPacket2PacketDelta = ((uint64_t)BMUXLIB_ASP_TS_PACKET_SIZE * 8 * 27000000) / hMuxTS->stMuxSettings.uiSystemDataBitRate;

   /* Update Video Encoder Index */
   hMuxTS->stStartSettings.video[0].stInputInterface.fReadIndex( hMuxTS->stStartSettings.video[0].stInputInterface.pContext, hMuxTS->stUserData.astDescriptor, BMUXLIB_TS_MAX_VIDEO_FRAME_COUNT, &uiNumDescriptors );
   if ( BMUXLIB_TS_MAX_VIDEO_FRAME_COUNT == uiNumDescriptors ) BDBG_WRN(("Possible video buffer index overflow"));
   if ( 0 == uiNumDescriptors ) return;

   /* Update userdata queue state */
   BMMA_FlushCache( hMuxTS->stUserData.hBlock, hMuxTS->stUserData.pstQueue, sizeof(*hMuxTS->stUserData.pstQueue) );

   /* Process System Data */

   /* Calculate the system data buffer's starting ESCR */
   {
      unsigned i;
      for ( i = 0; i < uiNumDescriptors; i++ )
      {
         BAVC_VideoBufferDescriptor *pstBufferDescriptor = &hMuxTS->stUserData.astDescriptor[i];
         if ( ( 0 != ( pstBufferDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_ESCR_VALID )
              && ( 0 != ( pstBufferDescriptor->stCommon.uiFlags & BAVC_COMPRESSEDBUFFERDESCRIPTOR_FLAGS_PTS_VALID ) ) ) )
         {
            uint32_t uiOffset;
            uint64_t uiTargetPTSin27Mhz = pstBufferDescriptor->stCommon.uiPTS * 300;
            /* Seed the PCR with the earliest adjusted ESCR seen */
            uint32_t uiTargetESCR = pstBufferDescriptor->stCommon.uiESCR;
            uint64_t uiESCRExtended = 0;

            /* determine the distance (offset) between the PTS and the Target ESCR (modulo-32-bits) */
            uiOffset = (uint32_t)(uiTargetPTSin27Mhz & 0xFFFFFFFF) - uiTargetESCR;
            uiESCRExtended = BMUXLIB_TS_P_MOD300_SUB32(uiTargetPTSin27Mhz, uiOffset);

            if ( false == hMuxTS->stSystemData.bNextExpectedESCRValid )
            {
               /* We haven't sent any system data, yet, so the first ESCR should be BEFORE any A/V data is sent, to ensure PAT/PMT arrive prior to the first A/V data */
               hMuxTS->stSystemData.uiNextExpectedESCR = uiESCRExtended-1;
               hMuxTS->stSystemData.bNextExpectedESCRValid = true;
            }
            else
            {
               /* Ensure the uiNextExpectedESCR is >= the first first descriptor */
               if ( ( (int32_t) ( (uint32_t) (hMuxTS->stSystemData.uiNextExpectedESCR & 0xFFFFFFFF) - pstBufferDescriptor->stCommon.uiESCR ) ) < 0 )
               {
                  hMuxTS->stSystemData.uiNextExpectedESCR = uiESCRExtended;
               }
            }
            break;
         }
      }
   }

   while (1)
   {
      BMUXlib_TS_ASP_P_SystemData_Entry *pstSourceEntry = NULL;
      BMUXlib_ASP_TS_Userdata_Entry_t *pstDestEntry = NULL;

      unsigned uiNextWriteOffset = ( hMuxTS->stUserData.pstQueue->uiWriteOffset[0] + 1 ) % BMUXLIB_ASP_TS_MAX_USERDATA_ENTRIES_PER_QUEUE;
      if ( uiNextWriteOffset == hMuxTS->stUserData.pstQueue->uiReadOffset[0] ) break; /* ASP TS MUXlib Userdata Queue is full */
      if ( hMuxTS->stSystemData.stInputQueue.uiShadowReadOffset == hMuxTS->stSystemData.stInputQueue.uiWriteOffset ) break; /* No more system data descriptors to process */

      pstSourceEntry = &hMuxTS->stSystemData.stInputQueue.astEntry[hMuxTS->stSystemData.stInputQueue.uiShadowReadOffset];
      pstDestEntry = &hMuxTS->stUserData.pstQueue->stQueue[0].stEntry[hMuxTS->stUserData.pstQueue->uiWriteOffset[0]];
      BKNI_Memset( pstDestEntry, 0, sizeof( *pstDestEntry ) );

      /* Lock the system data buffer offset */
      {
         BMMA_DeviceOffset uiOffset;

         pstSourceEntry->uiOffset = BMMA_LockOffset( pstSourceEntry->stData.hBlock);
         uiOffset = pstSourceEntry->uiOffset + pstSourceEntry->stData.uiBlockOffset;
         pstDestEntry->stBuffer[0].uiOffsetHi = (uiOffset >> 32) & 0xFFFFFFFF;
         pstDestEntry->stBuffer[0].uiOffsetLo = (uiOffset >>  0) & 0xFFFFFFFF;
         pstDestEntry->uiBufferInfo = (0 << BMUXLIB_ASP_TS_USERDATA_BUFFER_INFO_NUM_VALID_BUFFERS_SHIFT) & BMUXLIB_ASP_TS_USERDATA_BUFFER_INFO_NUM_VALID_BUFFERS_MASK;
      }
      /* Set the total size of the buffer */
      pstDestEntry->stBuffer[0].uiSize = pstSourceEntry->stData.uiSize;

      /* TODO: Treat system data the same as untimed userdata:
       *    Calculate ESCR for each system data packet based on video encoder index */

      /* TODO: Insert a NULL user data entry for each active user input to pad it to the end of the last video frame processed.
       *    Doing so will prevent sparse system/user data from stalling the mux pipe. Any "late user data" will need to be discarded...who/how will the discard happen?
       */

      /* IDEA:
       *  - To ensure queues system data is not late, should we process system data backwards starting from the end of the video encoder index?
       */

      /* Set ESCR */
      pstDestEntry->uiESCR = (uint32_t) (hMuxTS->stSystemData.uiNextExpectedESCR & 0xFFFFFFFF);
      pstDestEntry->uiPTS = pstDestEntry->uiESCR / 300;
      {
         uint32_t uiTimestampDeltaIn27Mhz = pstSourceEntry->stData.uiTimestampDelta * 27000;

         if ( uiTimestampDeltaIn27Mhz > uiPacket2PacketDelta )
         {
            hMuxTS->stSystemData.uiNextExpectedESCR += uiTimestampDeltaIn27Mhz;
         }
         else
         {
            hMuxTS->stSystemData.uiNextExpectedESCR += uiPacket2PacketDelta;
         }
      }

      hMuxTS->stSystemData.stInputQueue.uiShadowReadOffset = (hMuxTS->stSystemData.stInputQueue.uiShadowReadOffset + 1 ) % BMUXLIB_TS_MAX_SYSTEM_DATA_COUNT;
      hMuxTS->stUserData.pstQueue->uiWriteOffset[0] = uiNextWriteOffset;
   }

#if 0
   unsigned i,j;
   unsigned uiNumDescriptors = 0;


   /* Update Video Encoder Index */
   hMuxTS->stStartSettings.video[0].stInputInterface.fReadIndex( hMuxTS->stStartSettings.video[0].stInputInterface.pContext, hMuxTS->stUserData.astDescriptor, BMUXLIB_TS_MAX_VIDEO_FRAME_COUNT, &uiNumDescriptors );
   if ( uiNumDescriptors == BMUXLIB_TS_MAX_VIDEO_FRAME_COUNT ) BDBG_WRN(("Possible video buffer index overflow"));

   /* TODO: Process System Data
    *  - Start sending system data buffer *before* next I frame
    */
   /* TODO:*/

   for ( i = 0; i < uiNumDescriptors; i++ )
   {
      BAVC_VideoBufferDescriptor *pstDescriptor = &hMuxTS->stUserData.astDescriptor[i];

      for ( j = 0; j < hMuxTS->stStartSettings.uiNumValidUserdataPIDs; j++ )
      {

      }

      /* for each video frame */
      /* TODO: update delta(oPTS, newPTS) if discontinuity detected */
      /* for each PES */
   }
   /* TODO: Detect oPTS discontinuity */

   /* For each contiguous range:
    *    - process userdata inputs up to newPTS
    *       - If untimed/system data
    *          - ESCR[0] = videoESCR[0]
    *       - If timed
    *           - calculate newPTS and ESCR
    *           - if ESCR < videoESCR[0], discard userdata
    */

   /* For each user data input (including PAT/PMT)
    *
    */
#endif
   /* Force update the userdata queue in DRAM */
   BMMA_FlushCache( hMuxTS->stUserData.hBlock, hMuxTS->stUserData.pstQueue, sizeof(*hMuxTS->stUserData.pstQueue) );
}

void
BMUXlib_TS_ASP_P_ProcessCompletedUserData(
      BMUXlib_TS_ASP_Handle hMuxTS
      )
{
   size_t uiCompletedCount = 0;
   /* Force update the userdata queue in DRAM */
   BMMA_FlushCache( hMuxTS->stUserData.hBlock, hMuxTS->stUserData.pstQueue, sizeof(*hMuxTS->stUserData.pstQueue) );

   /* Reclaim System Data */
   while ( hMuxTS->stUserData.uiShadowReadOffset != hMuxTS->stUserData.pstQueue->uiReadOffset[0] )
   {
      BMUXlib_TS_ASP_P_SystemData_Entry *pstSourceEntry = &hMuxTS->stSystemData.stInputQueue.astEntry[hMuxTS->stSystemData.stInputQueue.uiReadOffset];
      /* Unlock the system data buffer offset */
      BMMA_UnlockOffset( pstSourceEntry->stData.hBlock, pstSourceEntry->uiOffset );
      BKNI_Memset( pstSourceEntry, 0, sizeof( *pstSourceEntry ) );

      hMuxTS->stUserData.uiShadowReadOffset = ( hMuxTS->stUserData.uiShadowReadOffset + 1 ) % BMUXLIB_ASP_TS_MAX_USERDATA_ENTRIES_PER_QUEUE;
      hMuxTS->stSystemData.stInputQueue.uiReadOffset = ( hMuxTS->stSystemData.stInputQueue.uiReadOffset + 1 ) % BMUXLIB_TS_MAX_SYSTEM_DATA_COUNT;
      uiCompletedCount++;
   }
   hMuxTS->stSystemData.uiCompletedCount += uiCompletedCount;

   /* TODO: Reclaim all other user data types */

}
