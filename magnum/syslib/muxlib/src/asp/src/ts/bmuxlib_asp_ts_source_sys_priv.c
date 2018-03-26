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

#include "bmuxlib_asp_ts_source_sys_priv.h"
#include "bmuxlib_asp_source_list_priv.h"
#include "bmuxlib_asp_ts_memory_priv.h"

BDBG_MODULE(BMUXLIB_ASP_TS_SOURCE_SYS);
BDBG_FILE_MODULE(BMUXLIB_ASP_TS_SOURCE_SYS_STATE);
BDBG_FILE_MODULE(BMUXLIB_ASP_TS_SOURCE_SYS_PCR);

typedef struct BMUXlib_ASP_TS_Source_Sys_Context_t* BMUXlib_Source_Sys_Handle;

#define BMUXLIB_ASP_P_GET_MUX_SYS_STATE(_handle) ((_handle)->eState)
#define BMUXLIB_ASP_P_SET_MUX_SYS_STATE(_handle,_state) ((_handle)->eState = (_state))

#define BMUXLIB_ASP_TS_P_MOD300_WRAP(x)    (((((x) >> 32) % 0x258) << 32) | ((x) & 0xFFFFFFFF))
#define BMUXLIB_ASP_TS_P_GET_PCR_BASE(x)   ((((((x) / 300) >> 32) & 0x1 ) << 32 ) | (((x) / 300) & 0xFFFFFFFF))
#define BMUXLIB_ASP_TS_P_GET_PCR_EXT(x)    (((x) % 300) & 0x1FF)
#define BMUXLIB_ASP_TS_P_SCALE_MS_TO_27MHZ    (27000000 / 1000)

static uint8_t astDefaultTSPacket[BMUXLIB_ASP_TS_HEADER_SIZE] =
{
   0x47, /* Sync Byte */
   0x00, 0x00, /* PUSI/PID */
   0x00 /* Scrambling/Adaptation/CC */
};

#define BMUXLIB_ASP_TS_P_TSPacket_SetPID(_tsPacket, _pid) \
   ((uint8_t*)_tsPacket)[1] = ( ( _pid >> 8 ) & 0x1F ) | ( ((uint8_t*)_tsPacket)[1] & ~0x1F ); \
   ((uint8_t*)_tsPacket)[2] = ( ( _pid      ) & 0xFF );

#define BMUXLIB_ASP_TS_P_TSPacket_SetAdaptationPresent(_tsPacket, _bAdaptationPresent) \
   ((uint8_t*)_tsPacket)[3] = ( _bAdaptationPresent ? 0x20 : 0x00 ) | (((uint8_t*)_tsPacket)[3] & ~0x20 );

#define BMUXLIB_ASP_TS_P_TSPacket_SetPayloadPresent(_tsPacket, _bPayloadPresent) \
   ((uint8_t*)_tsPacket)[3] = ( _bPayloadPresent ? 0x10 : 0x00 ) | (((uint8_t*)_tsPacket)[3] & ~0x10 );

#define BMUXLIB_ASP_TS_P_TSPacket_SetAdaptationLength(_tsPacket, _AdaptationLength) \
   ((uint8_t*)_tsPacket)[4] = _AdaptationLength;   \
   if (_AdaptationLength != 0) ((uint8_t*)_tsPacket)[5] = 0

#define BMUXLIB_ASP_TS_P_TSPacket_SetPCRPresent(_tsPacket, _bPCRPresent) \
   ((uint8_t*)_tsPacket)[5] = ( _bPCRPresent ? 0x10 : 0x00 ) | ( ((uint8_t*)_tsPacket)[5] & ~0x10 );

#define BMUXLIB_ASP_TS_P_TSPacket_SetPCRBase( _tsPacket, _PCRBase ) \
   ((uint8_t*)_tsPacket)[ 6] = ( (_PCRBase >> 25 ) & 0xFF ); \
   ((uint8_t*)_tsPacket)[ 7] = ( (_PCRBase >> 17 ) & 0xFF ); \
   ((uint8_t*)_tsPacket)[ 8] = ( (_PCRBase >>  9 ) & 0xFF ); \
   ((uint8_t*)_tsPacket)[ 9] = ( (_PCRBase >>  1 ) & 0xFF ); \
   ((uint8_t*)_tsPacket)[10] = ( (_PCRBase <<  7 ) & 0x80 ) | ( ((uint8_t*)_tsPacket)[10] & 0x01 ) | 0x7E;

#define BMUXLIB_ASP_TS_P_TSPacket_SetPCRExtension( _tsPacket, _PCRExtension ) \
   ((uint8_t*)_tsPacket)[10] = ( (_PCRExtension >>  8 ) & 0xFF ) | (((uint8_t*)_tsPacket)[10] & 0x80 ) | 0x7E; \
   ((uint8_t*)_tsPacket)[11] = ( (_PCRExtension       ) & 0xFF ); \

static void
BMUXLIB_ASP_S_Source_Sys_Generate_PCR(
   BMUXlib_Source_Sys_Handle hSourceSysHandle,
   uint8_t auiPacket[]
   )
{
   memset( auiPacket, 0xFF, BMUXLIB_ASP_TS_PACKET_SIZE );
   memcpy( auiPacket, astDefaultTSPacket, BMUXLIB_ASP_TS_HEADER_SIZE );

   BMUXLIB_ASP_TS_P_TSPacket_SetPID( auiPacket, hSourceSysHandle->pstSystemDataSettings->stPCR.uiPID );
   BMUXLIB_ASP_TS_P_TSPacket_SetAdaptationPresent( auiPacket, TRUE );
   BMUXLIB_ASP_TS_P_TSPacket_SetAdaptationLength( auiPacket, 183 ); /* Always 183 for TS packets that contain only adaptation and no payload */
   BMUXLIB_ASP_TS_P_TSPacket_SetPCRPresent( auiPacket, TRUE );
   BMUXLIB_ASP_TS_P_TSPacket_SetPCRBase( auiPacket, BMUXLIB_ASP_TS_P_GET_PCR_BASE( hSourceSysHandle->stPCR.uiESCR ) );
   BMUXLIB_ASP_TS_P_TSPacket_SetPCRExtension( auiPacket, BMUXLIB_ASP_TS_P_GET_PCR_EXT( hSourceSysHandle->stPCR.uiESCR ) );
}

void
BMUXLIB_ASP_S_Source_Sys_PCR_SetTiming(
   BMUXlib_Source_Entry_Handle hSourceEntryHandle,
   BMUXlib_Source_Sys_Handle hSourceSysHandle
   )
{
   hSourceEntryHandle->stTiming.uiCurrentESCR = hSourceSysHandle->stPCR.uiESCR & 0xFFFFFFFF;
   hSourceEntryHandle->stTiming.uiCurrentPTS = BMUXLIB_ASP_TS_P_GET_PCR_BASE( hSourceSysHandle->stPCR.uiESCR );
   hSourceEntryHandle->stTiming.bValid = TRUE;

   hSourceSysHandle->stPCR.uiNumPacketsLeft = hSourceSysHandle->stPCR.uiNumPacketsBetweenPCRs;

   BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_SYS_PCR, ("Timing: %08x/"BDBG_UINT64_FMT,
      hSourceEntryHandle->stTiming.uiCurrentESCR,
      BDBG_UINT64_ARG(hSourceEntryHandle->stTiming.uiCurrentPTS)
      ));
}

static void
BMUXLIB_ASP_S_Source_Sys_PCR_Seed(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle,
   uint32_t uiESCR,
   uint64_t uiPTS
   )
{
   BMUXlib_Source_Sys_Handle hSourceSysHandle = &hChannelHandle->stContext.stSystem;
   uint32_t uiTargetESCR = uiESCR;
   uint64_t uiTargetPTSin27Mhz = uiPTS * 300;

   /* determine the desired "pre-offset" prior to the first media
      set the PCRs to go out 3 PCR intervals prior to the first media.
      NOTE: This is done to ensure we have at least 2 PCRs *prior* to the media to allow time
      for decoder lock to PCRs (if this is not the case, the first GOP will likely be lost) */
   uint32_t uiMaxDiff = 3 * hSourceSysHandle->stPCR.uiPCRIntervalIn27Mhz;

   BDBG_ENTER( BMUXLIB_ASP_P_Source_Sys_PCR_Seed );

   /* We compute the final target ESCR by subtracting the "pre-offset" */
   uiTargetESCR -= uiMaxDiff;

   {
      /* determine the distance (offset) between the PTS and the Target ESCR (modulo-32-bits) */
      uint32_t uiOffset = (uint32_t)( uiTargetPTSin27Mhz & 0xFFFFFFFF ) - uiTargetESCR;
      hSourceSysHandle->stPCR.uiESCR = BMUXLIB_ASP_TS_P_MOD300_WRAP( uiTargetPTSin27Mhz - uiOffset );
   }

   BMUXLIB_ASP_S_Source_Sys_PCR_SetTiming( hSourceEntryHandle, hSourceSysHandle );

   BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_SYS_PCR, ("Seed: escr/pts=%08x/"BDBG_UINT64_FMT" --> escr/pcr=%08x/"BDBG_UINT64_FMT,
      uiESCR,
      BDBG_UINT64_ARG(uiPTS),
      hSourceEntryHandle->stTiming.uiCurrentESCR,
      BDBG_UINT64_ARG(hSourceEntryHandle->stTiming.uiCurrentPTS)
      ));

   BDBG_LEAVE( BMUXLIB_ASP_P_Source_Sys_PCR_Seed );
}

#if 0
void
BMUXLIB_ASP_S_SystemData_Consume(
   BMUXlib_SystemData_Metadata_t *pstUserDataMetadata,
   uint32_t uiNumBytes
   )
{
   BMUXlib_ASP_TS_SystemData_Queue_t *pstQueue = pstUserDataMetadata->pstSystemDataQueue;

   pstUserDataMetadata->uiCompletedBytes += uiNumBytes;

   while ( ( 0 != pstUserDataMetadata->uiCompletedBytes )
           && ( pstUserDataMetadata->uiCompletedBytes >= pstQueue->astData[pstQueue->uiRead].uiLength ) )
   {
      pstUserDataMetadata->uiCompletedBytes -= pstQueue->astData[pstQueue->uiRead].uiLength;
      pstUserDataMetadata->uiCompletedDescriptors++;
      pstQueue->uiRead++;
      pstQueue->uiRead %= BMUXLIB_ASP_TS_MAX_SYS_PACKETS_QUEUED;
   }
}

void
BMUXLIB_ASP_S_SystemData_Output_Descriptor_Initialize(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
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
   pstOutputDescriptorMetadata->fConsumeResource = (BMUXlib_ASP_P_Output_ConsumeResource) BMUXLIB_ASP_S_SystemData_Consume;
   pstOutputDescriptorMetadata->pvContext = &hChannelHandle->stContext.stSystem.stSystemData;
   pstOutputDescriptorMetadata->uiResourceId = uiPayloadSize;
}
#endif

/* Public Functions */
void
BMUXib_ASP_TS_P_Source_Sys_Start(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle
   )
{
   BMUXlib_Source_Sys_Handle hSourceSysHandle = &hChannelHandle->stContext.stSystem;

   BDBG_ENTER( BMUXib_ASP_TS_P_Source_Sys_Start );

   BSTD_UNUSED( hSourceEntryHandle );

   hChannelHandle->stContext.stSystem.pstSystemDataSettings = &hChannelHandle->stStartSettings.stSystemData;
   hSourceSysHandle->stPCR.uiPCRIntervalIn27Mhz = BMUXLIB_ASP_TS_P_SCALE_MS_TO_27MHZ * hSourceSysHandle->pstSystemDataSettings->stPCR.uiInterval;
   hSourceEntryHandle->stTiming.uiCurrentPacket2PacketTimestampDelta = ((uint64_t)BMUXLIB_ASP_TS_PACKET_SIZE * 8 * 27000000) / hSourceSysHandle->pstSystemDataSettings->uiBitRate;
   hSourceSysHandle->stPCR.uiNumPacketsBetweenPCRs = hSourceSysHandle->stPCR.uiPCRIntervalIn27Mhz / hSourceEntryHandle->stTiming.uiCurrentPacket2PacketTimestampDelta;

#if 0
   /* Initialize internal system data interface context */
   hChannelHandle->stContext.stSystem.stSystemData.pstSystemDataQueue = &hChannelHandle->stContext.stSystem.stSystemDataQueue;
#endif

   BDBG_LEAVE( BMUXib_ASP_TS_P_Source_Sys_Start );

   return;
}

bool_t
BMUXlib_ASP_TS_P_Source_Sys_Reconcile(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle
   )
{
   BMUXlib_Source_Sys_Handle hSourceSysHandle = &hChannelHandle->stContext.stSystem;

   BDBG_ENTER( BMUXlib_ASP_TS_P_Source_Sys_Reconcile );

   if ( BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_SEED == BMUXLIB_ASP_P_GET_MUX_SYS_STATE( hSourceSysHandle ) )
   {
      BMUXlib_Source_Entry_Handle hFirstSourceEntry = NULL;

      BMUXlib_ASP_P_Source_List_Peek( &hChannelHandle->stContext.stSourceList, &hFirstSourceEntry );

      if ( NULL != hFirstSourceEntry )
      {
         BMUXLIB_ASP_S_Source_Sys_PCR_Seed( hChannelHandle, hSourceEntryHandle, hFirstSourceEntry->stTiming.uiCurrentESCR, hFirstSourceEntry->stTiming.uiCurrentPTS );

         BMUXLIB_ASP_P_SET_MUX_SYS_STATE( hSourceSysHandle, BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_GENERATE);
      }
   }

   BDBG_LEAVE( BMUXlib_ASP_TS_P_Source_Sys_Reconcile );

   return ( TRUE == hSourceEntryHandle->stTiming.bValid );
}

#if BDBG_DEBUG_BUILD
static const char* astSourceSysStateFriendlyNameLUT[BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PROCESS_MAX] =
{
   "SOURCE_SYS_STATE_PCR_SEED",
   "SOURCE_SYS_STATE_PCR_GENERATE",
   "SOURCE_SYS_STATE_PCR_DMA_START",
   "SOURCE_SYS_STATE_PCR_DMA_FINISH",
   "SOURCE_SYS_STATE_PCR_SEND",
#if 0
   "SOURCE_SYS_STATE_DATA_QUEUE"
#endif
};
#endif

bool_t
BMUXlib_ASP_TS_P_Source_Sys_DoMux(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle,
   uint32_t uiEndESCR
   )
{
   BMUXlib_ASP_TS_Source_Sys_State_e ePreviousState;
   BMUXlib_ASP_TS_Memory_Handle hMemoryHandle = &hChannelHandle->stContext.stMemory;
   BMUXlib_ASP_Output_Handle hOutputHandle = &hChannelHandle->stContext.stOutput;
   BMUXlib_Source_Sys_Handle hSourceSysHandle = &hChannelHandle->stContext.stSystem;

   BDBG_ENTER( BMUXlib_ASP_TS_P_Source_Sys_DoMux );

   do
   {
      ePreviousState = BMUXLIB_ASP_P_GET_MUX_SYS_STATE( hSourceSysHandle );
      BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_SYS_STATE, ("%s", astSourceSysStateFriendlyNameLUT[ePreviousState] ));

      switch ( ePreviousState )
      {
         case BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_SEED:
            break;

         case BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_GENERATE:
            if ( TRUE == BMUXlib_ASP_TS_P_Memory_Packet_Available( hMemoryHandle ) )
            {
               BMUXLIB_ASP_S_Source_Sys_Generate_PCR( hSourceSysHandle, hSourceSysHandle->stTemp.auiTSPacket );

               BMUXLIB_ASP_P_SET_MUX_SYS_STATE( hSourceSysHandle, BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_DMA_START );
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_DMA_START:
            if ( TRUE == BMUXlib_ASP_TS_P_Memory_Packet_StartWrite( hMemoryHandle, hSourceSysHandle->stTemp.auiTSPacket, &hSourceSysHandle->stTemp.uiBufferOffset, &hSourceSysHandle->pDMAToken ) )
            {
               BMUXLIB_ASP_P_SET_MUX_SYS_STATE( hSourceSysHandle, BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_DMA_FINISH);
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_DMA_FINISH:
            if ( TRUE == BMUXlib_ASP_TS_P_Memory_Packet_FinishWrite( hMemoryHandle, &hSourceSysHandle->pDMAToken ) )
            {
               /* We seed the currentESCR/PTS here to ensure that the next PCR packet is primed and ready to go */
               BMUXLIB_ASP_S_Source_Sys_PCR_SetTiming( hSourceEntryHandle, hSourceSysHandle );

               /* Set the next PCR start */
               hSourceSysHandle->stPCR.uiESCR += hSourceSysHandle->stPCR.uiPCRIntervalIn27Mhz;
               hSourceSysHandle->stPCR.uiESCR = BMUXLIB_ASP_TS_P_MOD300_WRAP( hSourceSysHandle->stPCR.uiESCR );

               BMUXLIB_ASP_P_SET_MUX_SYS_STATE( hSourceSysHandle, BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_SEND );
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_SEND:
            if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Available( hOutputHandle ) )
            {
               BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor = &hChannelHandle->stTemp.stOutput.astTransportDescriptors[0];
               BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata = &hChannelHandle->stTemp.stOutput.stOutputDescriptorMetadata;

               BMUXlib_ASP_TS_P_Memory_Output_Descriptor_Initialize( hMemoryHandle, hOutputHandle, hSourceSysHandle->stTemp.uiBufferOffset, BMUXLIB_ASP_TS_PACKET_SIZE, hSourceSysHandle->pstSystemDataSettings->uiPIDChannelIndex, pstOutputDescriptor, pstOutputDescriptorMetadata );

               /* Set Next Packet Pacing Timestamp */
               pstOutputDescriptor->uiNextPacketPacingTimestamp = hSourceEntryHandle->stTiming.uiCurrentESCR;
               pstOutputDescriptor->bNextPacketPacingTimestampValid = TRUE;

               /* Set Packet2Packet Timestamp Delta */
               pstOutputDescriptor->uiPacket2PacketTimestampDelta = hSourceEntryHandle->stTiming.uiCurrentPacket2PacketTimestampDelta;
               pstOutputDescriptor->bPacket2PacketTimestampDeltaValid = TRUE;

               /* Set PCR specific flags */
               pstOutputDescriptor->bPushPreviousPartialPacket = TRUE;
               pstOutputDescriptor->bHostDataInsertion = TRUE;

               if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Send( hOutputHandle, pstOutputDescriptor, pstOutputDescriptorMetadata, &hSourceEntryHandle->stTiming ) )
               {
                  hSourceSysHandle->stPCR.uiNumPacketsLeft--;

#if 0
                 BMUXLIB_ASP_P_SET_MUX_SYS_STATE( hSourceSysHandle, BMUXLIB_ASP_TS_SOURCE_SYS_STATE_DATA_QUEUE );
#else
                 BMUXLIB_ASP_P_SET_MUX_SYS_STATE( hSourceSysHandle, BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_GENERATE );
#endif
               }
            }
            break;

#if 0
         case BMUXLIB_ASP_TS_SOURCE_SYS_STATE_DATA_QUEUE:
            /* Schedule system data packets in between PCRs */
            {
               BMUXlib_ASP_TS_SystemData_Queue_t *pstQueue = &hSourceSysHandle->stSystemDataQueue;

               if ( ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Available( hOutputHandle ) )
                    && ( 0 != hSourceSysHandle->stPCR.uiNumPacketsLeft )
                    && ( pstQueue->uiQueued != pstQueue->uiValid ) )
               {
                  uint64_t uiOffsetToSend = pstQueue->astData[pstQueue->uiQueued].uiOffset + pstQueue->uiQueuedBytes;
                  size_t uiLengthToSend = pstQueue->astData[pstQueue->uiQueued].uiLength - pstQueue->uiQueuedBytes;

                  /* TODO: Add support for other types of system data */

                  /* Insert only enough packets that will fit in between PCRs */
                  {
                     size_t uiSizeLeft = hSourceSysHandle->stPCR.uiNumPacketsLeft * BMUXLIB_ASP_TS_PACKET_SIZE;

                     if ( uiSizeLeft < uiLengthToSend )
                     {
                        uiLengthToSend = uiSizeLeft;
                     }
                  }

                  {
                     BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor = &hChannelHandle->stTemp.stOutput.astTransportDescriptors[0];
                     BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata = &hChannelHandle->stTemp.stOutput.stOutputDescriptorMetadata;

                     BMUXLIB_ASP_S_SystemData_Output_Descriptor_Initialize( hChannelHandle, hOutputHandle, uiOffsetToSend, uiLengthToSend, hSourceSysHandle->pstSystemDataSettings->uiPIDChannelIndex, pstOutputDescriptor, pstOutputDescriptorMetadata );

                     /* Set UserData specific flags */
                     pstOutputDescriptor->bPushPreviousPartialPacket = TRUE;
                     pstOutputDescriptor->bHostDataInsertion = TRUE;

                     if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Send( hOutputHandle, pstOutputDescriptor, pstOutputDescriptorMetadata, &hSourceEntryHandle->stTiming ) )
                     {
                        hSourceSysHandle->stPCR.uiNumPacketsLeft -= ( uiLengthToSend / BMUXLIB_ASP_TS_PACKET_SIZE );

                        /* Bump up the userdata pending bytes buffer */
                        pstQueue->uiQueuedBytes += uiLengthToSend;
                        if ( pstQueue->uiQueuedBytes == pstQueue->astData[pstQueue->uiQueued].uiLength )
                        {
                           pstQueue->uiQueued++;
                           pstQueue->uiQueued %= BMUXLIB_ASP_TS_MAX_SYS_PACKETS_QUEUED;
                           pstQueue->uiQueuedBytes = 0;
                        }
                     }
                  }
               }
            }
            /* TODO: Transition to PCR_GENERATE only after timing critical user data has been sent */
            BMUXLIB_ASP_P_SET_MUX_SYS_STATE( hSourceSysHandle, BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_GENERATE );
            break;
#endif

         case  BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PROCESS_MAX:
            /* TODO: Add assert - Should never come here */
            break;
      }
#if 0
      if ( ePreviousState != BMUXLIB_ASP_P_GET_MUX_SYS_STATE( hSourceAVHandle ) )
      {
         BKNI_Printf("%s --> %s\n", astSourceSysStateFriendlyNameLUT[ePreviousState], astSourceSysStateFriendlyNameLUT[BMUXLIB_ASP_P_GET_MUX_AV_STATE( hSourceAVHandle )]);
      }
#endif
   } while ( ( ePreviousState != BMUXLIB_ASP_P_GET_MUX_SYS_STATE( hSourceSysHandle ) )
             && ( ( (int32_t) ( uiEndESCR - hSourceEntryHandle->stTiming.uiCurrentESCR ) ) > 0 ) ); /* We haven't exceeded the endESCR */

   BDBG_LEAVE( BMUXlib_ASP_TS_P_Source_Sys_DoMux );

   /* The system data source always has data available once it has been seeded */
   return ( ( ePreviousState != BMUXLIB_ASP_P_GET_MUX_SYS_STATE( hSourceSysHandle ) )
            && ( BMUXLIB_ASP_TS_SOURCE_SYS_STATE_PCR_SEED != BMUXLIB_ASP_P_GET_MUX_SYS_STATE( hSourceSysHandle ) ) );
}

#if 0
bool_t
BMUXLIB_ASP_P_Source_Sys_AddSystemData(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   const BMUXlib_ASP_TS_SystemData_Entry_t *pstEntry
   )
{
   uint8_t uiTempValid;
   BMUXlib_Source_Sys_Handle hSourceSysHandle = &hChannelHandle->stContext.stSystem;

   /* Check if there's space in the system data queue */
   uiTempValid = hSourceSysHandle->stSystemDataQueue.uiValid + 1;
   uiTempValid %= BMUXLIB_ASP_TS_MAX_SYS_PACKETS_QUEUED;
   if ( hSourceSysHandle->stSystemDataQueue.uiRead == uiTempValid )
   {
      return false;
   }

   /* Copy the system data entry to our internal queue */
   hSourceSysHandle->stSystemDataQueue.astData[hSourceSysHandle->stSystemDataQueue.uiValid] = *pstEntry;
   hSourceSysHandle->stSystemDataQueue.uiValid = uiTempValid;

   return TRUE;
}

uint8_t
BMUXLIB_ASP_P_Source_Sys_GetCompletedSystemData(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle
   )
{
   uint8_t uiCompletedDescriptors;
   BMUXlib_Source_Sys_Handle hSourceSysHandle = &hChannelHandle->stContext.stSystem;
   uiCompletedDescriptors = hSourceSysHandle->stSystemData.uiCompletedDescriptors;
   hSourceSysHandle->stSystemData.uiCompletedDescriptors = 0;

   return uiCompletedDescriptors;
}
#endif
