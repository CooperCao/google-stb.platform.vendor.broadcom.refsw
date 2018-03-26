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

#include "bmuxlib_asp_ts_source_user_priv.h"
#include "bmuxlib_asp_output_priv.h"
#include "bmuxlib_asp_source_helper_priv.h"

BDBG_MODULE(BMUXLIB_ASP_TS_SOURCE_USER);
BDBG_FILE_MODULE(BMUXLIB_ASP_TS_SOURCE_USER_STATE);

#define BMUXLIB_ASP_P_GET_MUX_USER_STATE(_handle) ((_handle)->eState)
#define BMUXLIB_ASP_P_SET_MUX_USER_STATE(_handle,_state) ((_handle)->eState = (_state))

typedef struct BMUXlib_TS_ASP_Source_User_Context_t* BMUXlib_Source_User_Handle;

void
BMUXLIB_ASP_S_UserData_Consume(
   BMUXlib_Source_User_Handle hSourceUserHandle,
   uint32_t uiUnused
   )
{
   BSTD_UNUSED( uiUnused );
   *hSourceUserHandle->stQueue.puiReadOffset = (*hSourceUserHandle->stQueue.puiReadOffset + 1) % BMUXLIB_ASP_TS_MAX_USERDATA_ENTRIES_PER_QUEUE;
   *hSourceUserHandle->stQueue.pbReadOffsetUpdated = TRUE;
}

static void
BMUXLIB_ASP_S_Source_User_Output_Descriptor_Initialize(
   BMUXlib_Source_User_Handle hSourceUserHandle,
   uint32_t uiEndESCR,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle,
   BMUXlib_ASP_Output_Handle hOutputHandle,
   BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor,
   BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata
   )
{
   size_t uiPayloadSize = hSourceUserHandle->stCurrent.stEntry.stBuffer[hSourceUserHandle->stCurrent.uiCurrentBuffer].uiSize - hSourceUserHandle->stCurrent.uiBytesQueued;
   uint64_t uiPayloadOffset;

   uiPayloadOffset = hSourceUserHandle->stCurrent.stEntry.stBuffer[hSourceUserHandle->stCurrent.uiCurrentBuffer].uiOffsetHi;
   uiPayloadOffset <<= 32;
   uiPayloadOffset |= hSourceUserHandle->stCurrent.stEntry.stBuffer[hSourceUserHandle->stCurrent.uiCurrentBuffer].uiOffsetLo;
   uiPayloadOffset += hSourceUserHandle->stCurrent.uiBytesQueued;

   BMUXlib_ASP_P_Source_Helper_Output_Descriptor_Initialize( hSourceEntryHandle, uiEndESCR, BMUXLIB_ASP_TS_PACKET_SIZE, hOutputHandle, uiPayloadOffset, uiPayloadSize, hSourceUserHandle->uiPIDChannelIndex, pstOutputDescriptor, pstOutputDescriptorMetadata );

   /* Set HDI specific flags */
   pstOutputDescriptor->bPushPreviousPartialPacket = TRUE;
   pstOutputDescriptor->bHostDataInsertion = TRUE;

   if ( ( (hSourceUserHandle->stCurrent.uiBytesQueued + pstOutputDescriptor->uiBufferLength)  == hSourceUserHandle->stCurrent.stEntry.stBuffer[hSourceUserHandle->stCurrent.uiCurrentBuffer].uiSize )
        && ( hSourceUserHandle->stCurrent.uiCurrentBuffer == ( ( hSourceUserHandle->stCurrent.stEntry.uiBufferInfo & BMUXLIB_ASP_TS_USERDATA_BUFFER_INFO_NUM_VALID_BUFFERS_MASK ) >> BMUXLIB_ASP_TS_USERDATA_BUFFER_INFO_NUM_VALID_BUFFERS_SHIFT ) ) )
   {
      /* Set descriptor metadata for use by ProcessCompletedBuffers */
      pstOutputDescriptorMetadata->fConsumeResource = (BMUXlib_ASP_P_Output_ConsumeResource) BMUXLIB_ASP_S_UserData_Consume;
      pstOutputDescriptorMetadata->pvContext = hSourceUserHandle;
      pstOutputDescriptorMetadata->uiResourceId = 0;
   }
}

#if BDBG_DEBUG_BUILD
static const char* astSourceUserStateFriendlyNameLUT[BMUXLIB_ASP_TS_SOURCE_USER_STATE_PROCESS_MAX] =
{
   "BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_WAIT",
   "BMUXLIB_ASP_TS_SOURCE_USER_STATE_QUEUE_DMA_FINISH",
   "BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_DMA_START",
   "BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_DMA_FINISH",
   "BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_PAYLOAD",
   "BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_BUFFER_DONE",
};
#endif

/* Public Functions */
void
BMUXlib_ASP_P_Source_User_Start(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle
   )
{
   BMUXlib_Source_User_Handle hSourceUserHandle = &hChannelHandle->stContext.stUserData[hSourceEntryHandle->uiIndex];

   BDBG_ENTER( BMUXlib_ASP_P_Source_User_Start );

   /* Set the PKT2PKT delta based on the system data bitrate */
   hSourceEntryHandle->stTiming.uiCurrentPacket2PacketTimestampDelta = ((uint64_t)BMUXLIB_ASP_TS_PACKET_SIZE * 8 * 27000000) / hChannelHandle->stStartSettings.stSystemData.uiBitRate;
   hSourceUserHandle->uiPIDChannelIndex = (uint16_t) hChannelHandle->stStartSettings.stSystemData.uiPIDChannelIndex;
   hSourceUserHandle->stQueue.uiBaseOffset = hChannelHandle->stStartSettings.stUserData.uiOffset + ( sizeof( BMUXlib_ASP_TS_Userdata_Queue_t ) * hSourceEntryHandle->uiIndex );
   hSourceUserHandle->stQueue.pbReadOffsetUpdated = &hChannelHandle->stContext.stUserDataCommon.bReadOffsetUpdated;
   hSourceUserHandle->stQueue.puiReadOffset = &hChannelHandle->stContext.stUserDataCommon.uiReadOffset[hSourceEntryHandle->uiIndex];
   hSourceUserHandle->stQueue.puiWriteOffset = &hChannelHandle->stContext.stUserDataCommon.uiWriteOffset[hSourceEntryHandle->uiIndex];

   BDBG_LEAVE( BMUXlib_ASP_P_Source_User_Start );
}

bool_t
BMUXlib_ASP_P_Source_User_Reconcile(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle
   )
{
   BMUXlib_Source_User_Handle hSourceUserHandle = &hChannelHandle->stContext.stUserData[hSourceEntryHandle->uiIndex];
   BMUXlib_ASP_TS_Source_User_State_e ePreviousState;
   bool_t bQueueUpdatedAlready = FALSE;

   BDBG_ENTER( BMUXlib_ASP_P_Source_User_Reconcile );

   do
   {
      ePreviousState = BMUXLIB_ASP_P_GET_MUX_USER_STATE( hSourceUserHandle );
      BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_USER_STATE, ("%s", astSourceUserStateFriendlyNameLUT[ePreviousState] ));

      switch ( ePreviousState )
      {
         case BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_WAIT:
            /* We have a user data entry to read */
            if ( *hSourceUserHandle->stQueue.puiWriteOffset != hSourceUserHandle->stQueue.uiShadowRead )
            {
               BMUXLIB_ASP_P_SET_MUX_USER_STATE( hSourceUserHandle, BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_DMA_START);
            }
            else if ( NULL == hChannelHandle->stContext.stUserDataCommon.pDMATokenWriteOffset )
            {
               if ( FALSE == bQueueUpdatedAlready )
               {
                  hChannelHandle->stStartSettings.stDMA.fDram2Dccm(
                    hChannelHandle->stStartSettings.stDMA.pvContext,
                    hChannelHandle->stContext.stUserDataCommon.uiWriteOffset,
                    hSourceUserHandle->stQueue.uiBaseOffset + ( offsetof( BMUXlib_ASP_TS_Userdata_Host_Interface_t, uiWriteOffset ) ),
                    sizeof( uint32_t ) * hChannelHandle->stStartSettings.stUserData.uiNumValidUserData,
                    &hChannelHandle->stContext.stUserDataCommon.pDMATokenWriteOffset
                  );
                  if ( NULL != hChannelHandle->stContext.stUserDataCommon.pDMATokenWriteOffset )
                  {
                     BMUXLIB_ASP_P_SET_MUX_USER_STATE( hSourceUserHandle, BMUXLIB_ASP_TS_SOURCE_USER_STATE_QUEUE_DMA_FINISH);
                     bQueueUpdatedAlready = TRUE;
                  }
               }
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_USER_STATE_QUEUE_DMA_FINISH:
            if ( TRUE == hChannelHandle->stStartSettings.stDMA.fIdle(
                  hChannelHandle->stStartSettings.stDMA.pvContext,
                  &hChannelHandle->stContext.stUserDataCommon.pDMATokenWriteOffset
                  ) )
            {
               hChannelHandle->stContext.stUserDataCommon.pDMATokenWriteOffset = NULL;
               BMUXLIB_ASP_P_SET_MUX_USER_STATE( hSourceUserHandle, BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_WAIT);
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_DMA_START:
            hChannelHandle->stStartSettings.stDMA.fDram2Dccm(
              hChannelHandle->stStartSettings.stDMA.pvContext,
              &hSourceUserHandle->stCurrent.stEntry,
              hSourceUserHandle->stQueue.uiBaseOffset + ( sizeof( BMUXlib_ASP_TS_Userdata_Entry_t ) * hSourceUserHandle->stQueue.uiShadowRead ),
              sizeof( BMUXlib_ASP_TS_Userdata_Entry_t ),
              &hSourceUserHandle->pDMAToken
            );
            if ( NULL != hSourceUserHandle->pDMAToken )
            {
               BMUXLIB_ASP_P_SET_MUX_USER_STATE( hSourceUserHandle, BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_DMA_FINISH);
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_DMA_FINISH:
            if ( TRUE == hChannelHandle->stStartSettings.stDMA.fIdle(
                  hChannelHandle->stStartSettings.stDMA.pvContext,
                  &hSourceUserHandle->pDMAToken
                  ) )
            {
               hSourceUserHandle->pDMAToken = NULL;

               hSourceEntryHandle->stTiming.uiCurrentESCR = hSourceUserHandle->stCurrent.stEntry.uiESCR;
               hSourceEntryHandle->stTiming.uiCurrentPTS = hSourceUserHandle->stCurrent.stEntry.uiPTS;
               hSourceEntryHandle->stTiming.bValid = TRUE;
               hSourceUserHandle->stQueue.uiShadowRead = (hSourceUserHandle->stQueue.uiShadowRead + 1) % BMUXLIB_ASP_TS_MAX_USERDATA_ENTRIES_PER_QUEUE;

               hSourceUserHandle->stCurrent.uiCurrentBuffer = 0;
               hSourceUserHandle->stCurrent.uiBytesQueued = 0;

               BMUXLIB_ASP_P_SET_MUX_USER_STATE( hSourceUserHandle, BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_PAYLOAD );
            }
            break;

         case  BMUXLIB_ASP_TS_SOURCE_USER_STATE_PROCESS_MAX:
            /* TODO: Add assert - Should never come here */

         default:
            break;
      }
#if 0
      if ( ePreviousState != BMUXLIB_ASP_P_GET_MUX_USER_STATE( hSourceAVHandle ) )
      {
         BKNI_Printf("%s --> %s\n", astSourceSysStateFriendlyNameLUT[ePreviousState], astSourceSysStateFriendlyNameLUT[BMUXLIB_ASP_P_GET_MUX_AV_STATE( hSourceAVHandle )]);
      }
#endif
   } while ( ( ePreviousState != BMUXLIB_ASP_P_GET_MUX_USER_STATE( hSourceUserHandle ) )
             && ( BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_PAYLOAD != BMUXLIB_ASP_P_GET_MUX_USER_STATE( hSourceUserHandle ) ) );

   BDBG_LEAVE( BMUXlib_ASP_P_Source_User_Reconcile );

   return ( ( BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_DMA_START != BMUXLIB_ASP_P_GET_MUX_USER_STATE( hSourceUserHandle )
            && ( TRUE == hSourceEntryHandle->stTiming.bValid ) ) ); /* We are OK to proceed using the current ESCR unless we are in the middle of DMA'ing the next user data entry */
}

bool_t
BMUXlib_ASP_P_Source_User_DoMux(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle,
   uint32_t uiEndESCR
   )
{
   BMUXlib_ASP_TS_Source_User_State_e ePreviousState;
   BMUXlib_ASP_Output_Handle hOutputHandle = &hChannelHandle->stContext.stOutput;
   BMUXlib_Source_User_Handle hSourceUserHandle = &hChannelHandle->stContext.stUserData[hSourceEntryHandle->uiIndex];
   uint32_t uiStartESCR = hSourceEntryHandle->stTiming.uiCurrentESCR;

   BDBG_ENTER( BMUXlib_ASP_P_Source_User_DoMux );

   do
   {
      ePreviousState = BMUXLIB_ASP_P_GET_MUX_USER_STATE( hSourceUserHandle );
      BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_USER_STATE, ("%s", astSourceUserStateFriendlyNameLUT[ePreviousState] ));

      switch ( ePreviousState )
      {
         case BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_PAYLOAD:
            if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Available( hOutputHandle ) )
            {
               BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor = &hChannelHandle->stTemp.stOutput.astTransportDescriptors[0];
               BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata = &hChannelHandle->stTemp.stOutput.stOutputDescriptorMetadata;

               BMUXLIB_ASP_S_Source_User_Output_Descriptor_Initialize( hSourceUserHandle, uiEndESCR, hSourceEntryHandle, hOutputHandle, pstOutputDescriptor, pstOutputDescriptorMetadata );

              if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Send( hOutputHandle, pstOutputDescriptor, pstOutputDescriptorMetadata, &hSourceEntryHandle->stTiming ) )
               {
                  hSourceUserHandle->stCurrent.uiBytesQueued += pstOutputDescriptor->uiBufferLength;

                  if ( hSourceUserHandle->stCurrent.uiBytesQueued == hSourceUserHandle->stCurrent.stEntry.stBuffer[hSourceUserHandle->stCurrent.uiCurrentBuffer].uiSize )
                  {
                     /* We're done with the current buffer in this entry */
                     BMUXLIB_ASP_P_SET_MUX_USER_STATE( hSourceUserHandle, BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_BUFFER_DONE );
                  }

               }
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_BUFFER_DONE:
            /* Start processing the next buffer */
            hSourceUserHandle->stCurrent.uiBytesQueued = 0;
            if ( hSourceUserHandle->stCurrent.uiCurrentBuffer == ( ( hSourceUserHandle->stCurrent.stEntry.uiBufferInfo & BMUXLIB_ASP_TS_USERDATA_BUFFER_INFO_NUM_VALID_BUFFERS_MASK ) >> BMUXLIB_ASP_TS_USERDATA_BUFFER_INFO_NUM_VALID_BUFFERS_SHIFT ) )
            {
               /* We're done with all buffers in this entry */
               BMUXLIB_ASP_P_SET_MUX_USER_STATE( hSourceUserHandle, BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_WAIT );
            }
            else
            {
               /* We need to process the next buffer in this entry */
               hSourceUserHandle->stCurrent.uiCurrentBuffer++;
               BMUXLIB_ASP_P_SET_MUX_USER_STATE( hSourceUserHandle, BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_PAYLOAD );
            }
            break;

         case  BMUXLIB_ASP_TS_SOURCE_USER_STATE_PROCESS_MAX:
            /* TODO: Add assert - Should never come here */

         default:
            BMUXlib_ASP_P_Source_User_Reconcile( hChannelHandle, hSourceEntryHandle );
            break;
      }
#if 0
      if ( ePreviousState != BMUXLIB_ASP_P_GET_MUX_USER_STATE( hSourceAVHandle ) )
      {
         BKNI_Printf("%s --> %s\n", astSourceSysStateFriendlyNameLUT[ePreviousState], astSourceSysStateFriendlyNameLUT[BMUXLIB_ASP_P_GET_MUX_AV_STATE( hSourceAVHandle )]);
      }
#endif
   } while ( ( ePreviousState != BMUXLIB_ASP_P_GET_MUX_USER_STATE( hSourceUserHandle ) )
             && ( ( (int32_t) ( uiEndESCR - hSourceEntryHandle->stTiming.uiCurrentESCR ) ) > 0 ) ); /* We haven't exceeded the endESCR */

   BDBG_LEAVE( BMUXlib_ASP_P_Source_User_DoMux );

   /* Return TRUE if more data exists and and we've made progress during this execution */
   return ( ( uiStartESCR != hSourceEntryHandle->stTiming.uiCurrentESCR )
            && ( BMUXLIB_ASP_TS_SOURCE_USER_STATE_ENTRY_WAIT != BMUXLIB_ASP_P_GET_MUX_USER_STATE( hSourceUserHandle ) ) );
}
