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

#include "bmuxlib_asp_output_context.h"
#include "bmuxlib_asp_output_priv.h"

BDBG_MODULE(BMUXLIB_ASP_OUTPUT);
BDBG_FILE_MODULE(BMUXLIB_ASP_OUTPUT_DESC);

bool_t
BMUXlib_ASP_P_Output_Descriptor_Available(
   BMUXlib_ASP_Output_Handle hOutputHandle
   )
{
   uint32_t uiWriteOffset = ( hOutputHandle->uiWrite + 1 ) % BMUXLIB_ASP_TS_MAX_OUTPUT_DESCRIPTORS_PER_INSTANCE;

   return ( uiWriteOffset != hOutputHandle->uiRead );
}

void
BMUXlib_ASP_P_Output_Descriptor_Initialize(
   BMUXlib_ASP_Output_Handle hOutputHandle,
   uint64_t uiBufferAddress,
   size_t uiLength,
   uint16_t uiPIDChannelIndex,
   BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor,
   BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata
   )
{
   BDBG_ENTER( BMUXlib_ASP_P_Output_Descriptor_Initialize );

   memset( pstOutputDescriptorMetadata, 0, sizeof( *pstOutputDescriptorMetadata ) );

   /* Set the output descriptor info for use by transport */
   memset( pstOutputDescriptor, 0, sizeof( *pstOutputDescriptor ) );

   /* Set Buffer Info */
   pstOutputDescriptor->uiBufferAddressLo = (uiBufferAddress & 0xFFFFFFFF);
   pstOutputDescriptor->uiBufferAddressHi = (uiBufferAddress >> 32) & 0xFFFFFFFF;
   pstOutputDescriptor->uiBufferLength = uiLength;

   /* Set PID Channel Index */
   pstOutputDescriptor->uiPidChannelIndex = uiPIDChannelIndex;
   pstOutputDescriptor->bPidChannelIndexValid = TRUE;

   if ( ( TRUE == hOutputHandle->bPreviousPIDChannelIndexValid )
        && ( hOutputHandle->uiPreviousPIDChannelIndex != pstOutputDescriptor->uiPidChannelIndex ) )
   {
      /* We are switching PID channels, so we need to set the push previous partial packet bit */
      pstOutputDescriptor->bPushPreviousPartialPacket = TRUE;
   }

   if ( FALSE == hOutputHandle->bPreviousPIDChannelIndexValid )
   {
      hOutputHandle->uiPreviousPIDChannelIndex = pstOutputDescriptor->uiPidChannelIndex;
      hOutputHandle->bPreviousPIDChannelIndexValid = pstOutputDescriptor->bPidChannelIndexValid;
   }

   BDBG_LEAVE( BMUXlib_ASP_P_Output_Descriptor_Initialize );
}

bool_t
BMUXlib_ASP_P_Output_Descriptor_Send(
   BMUXlib_ASP_Output_Handle hOutputHandle,
   BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor,
   BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata,
   BMUXlib_ASP_Timing_Metadata_t *pstTimingMetadata
   )
{
   size_t uiQueued = 0;

   BDBG_ENTER( BMUXlib_ASP_P_Output_Descriptor_Send );

   if ( FALSE == BMUXlib_ASP_P_Output_Descriptor_Available( hOutputHandle ) )
   {
      goto error;
   }

   hOutputHandle->pstOutputInterface->fAddTransportDescriptors(
      hOutputHandle->pstOutputInterface->pvContext,
      pstOutputDescriptor,
      1,
      &uiQueued
      );

   if ( 1 == uiQueued )
   {
      if ( TRUE == pstOutputDescriptor->bPacket2PacketTimestampDeltaValid )
      {
         pstTimingMetadata->uiCurrentPacket2PacketTimestampDelta = pstOutputDescriptor->uiPacket2PacketTimestampDelta;
      }
      if ( TRUE == pstOutputDescriptor->bNextPacketPacingTimestampValid )
      {
         pstTimingMetadata->uiCurrentESCR = pstOutputDescriptor->uiNextPacketPacingTimestamp;
      }

      BDBG_MODULE_MSG( BMUXLIB_ASP_OUTPUT_DESC,("@%x%08x (%5u bytes) ts(%u):%08x pkt2pkt(%u):%08x pidIdx(%u):%u (%s%s%s%s%s)",
         pstOutputDescriptor->uiBufferAddressHi,
         pstOutputDescriptor->uiBufferAddressLo,
         pstOutputDescriptor->uiBufferLength,
         pstOutputDescriptor->bNextPacketPacingTimestampValid,
         pstOutputDescriptor->uiNextPacketPacingTimestamp,
         pstOutputDescriptor->bPacket2PacketTimestampDeltaValid,
         pstOutputDescriptor->uiPacket2PacketTimestampDelta,
         pstOutputDescriptor->bPidChannelIndexValid,
         pstOutputDescriptor->uiPidChannelIndex,
         pstOutputDescriptor->bPushPreviousPartialPacket ? "[":" ",
         pstOutputDescriptor->bRandomAccessIndication ? "R":" ",
         pstOutputDescriptor->bHostDataInsertion ? "H":" ",
         pstOutputDescriptor->bInsertHostDataAsBtp ? "B":" ",
         pstOutputDescriptor->bPushPartialPacket ? "]":" "
         ));

      {
         uint8_t uiPacketSize = ( pstOutputDescriptor->bHostDataInsertion ? 188 : 184 );
         pstTimingMetadata->uiCurrentESCR += ( ( pstOutputDescriptor->uiBufferLength + ( uiPacketSize - 1 ) ) / uiPacketSize ) * pstTimingMetadata->uiCurrentPacket2PacketTimestampDelta;
      }

      /* Add the metadata to the output queue */
      hOutputHandle->astMetadata[hOutputHandle->uiWrite] = *pstOutputDescriptorMetadata;
      hOutputHandle->uiWrite++;
      hOutputHandle->uiWrite %= BMUXLIB_ASP_TS_MAX_OUTPUT_DESCRIPTORS_PER_INSTANCE;
   }
   else
   {
      goto error;
   }

   BDBG_LEAVE( BMUXlib_ASP_P_Output_Descriptor_Send );

   return TRUE;
error:
   return FALSE;
}

void
BMUXlib_ASP_P_Output_Descriptor_ProcessCompleted(
   BMUXlib_ASP_Output_Handle hOutputHandle
   )
{
   size_t uiCompletedCount = 0;

   BDBG_ENTER( BMUXlib_ASP_P_Output_Descriptor_ProcessCompleted );

   /* Get the number of completed transport descriptors */
   hOutputHandle->pstOutputInterface->fGetCompletedTransportDescriptors(
      hOutputHandle->pstOutputInterface->pvContext,
      &uiCompletedCount
      );

   if ( 0 != uiCompletedCount )
   {
      while ( 0 != uiCompletedCount )
      {
         BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata = &hOutputHandle->astMetadata[hOutputHandle->uiRead];

         pstOutputDescriptorMetadata->fConsumeResource( pstOutputDescriptorMetadata->pvContext, pstOutputDescriptorMetadata->uiResourceId );

         /* We've finished releasing the resources associated with this transport descriptor */
         hOutputHandle->uiRead++;
         hOutputHandle->uiRead %= BMUXLIB_ASP_TS_MAX_OUTPUT_DESCRIPTORS_PER_INSTANCE;
         uiCompletedCount--;
      }
   }

   BDBG_LEAVE( BMUXlib_ASP_P_Output_Descriptor_ProcessCompleted );
}
