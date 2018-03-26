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

#include "bmuxlib_asp_ts_source_av_priv.h"
#include "bmuxlib_asp_output_priv.h"
#include "bmuxlib_asp_ts_memory_priv.h"

BDBG_MODULE(BMUXLIB_ASP_TS_SOURCE_AV);
BDBG_FILE_MODULE(BMUXLIB_ASP_TS_SOURCE_AV_STATE);
BDBG_FILE_MODULE(BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE);
BDBG_FILE_MODULE(BMUXLIB_ASP_TS_SOURCE_AV_PES);
BDBG_FILE_MODULE(BMUXLIB_ASP_TS_SOURCE_AV_TIMING);
BDBG_FILE_MODULE(BMUXLIB_ASP_TS_SOURCE_AV_ITB);
BDBG_FILE_MODULE(BMUXLIB_ASP_TS_SOURCE_AV_ITB_INDEX);
BDBG_FILE_MODULE(BMUXLIB_ASP_TS_SOURCE_AV_CDB);

#define BMUXLIB_PUSI_DATA_SIZE 20
#define BMUXLIB_PES_HEADER_PAYLOAD_SIZE 13
#define BMUXLIB_PES_HEADER_DATA_SIZE ( BMUXLIB_PES_HEADER_PAYLOAD_SIZE + 6 )


#define BMUXLIB_ASP_P_GET_MUX_AV_STATE(_handle) ((_handle)->eState)
#define BMUXLIB_ASP_P_SET_MUX_AV_STATE(_handle,_state) ((_handle)->eState = (_state))

#define BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE(_handle) ((_handle)->eITBState)
#define BMUXLIB_ASP_P_SET_MUX_AV_ITB_STATE(_handle,_state) ((_handle)->eITBState = (_state))

#define BMUXLIB_ASP_TS_P_PESHeader_SetStreamID(_pesHeader, _streamID) \
         (_pesHeader)[3] = ( (_streamID) & 0xFF );

#define BMUXLIB_ASP_TS_P_PESHeader_SetPTS(_pesHeader, _pts) \
         (_pesHeader)[13] = ( ( ( ( _pts ) << 1  ) & 0xFE ) | (0x01) ); \
         (_pesHeader)[12] = (   ( ( _pts ) >> 7  ) & 0xFF ); \
         (_pesHeader)[11] = ( ( ( ( _pts ) >> 14 ) & 0xFE ) | (0x01) ); \
         (_pesHeader)[10] = (   ( ( _pts ) >> 22 ) & 0xFF ); \
         (_pesHeader)[9] =  ( ( ( ( _pts ) >> 29 ) & 0x0E ) | (0x21) ); \
         (_pesHeader)[7] |= 0x80;

#define BMUXLIB_ASP_TS_P_PESHeader_SetDTS(_pesHeader, _pts) \
         (_pesHeader)[18] = ( ( ( ( _pts ) << 1  ) & 0xFE ) | (0x01) ); \
         (_pesHeader)[17] = (   ( ( _pts ) >> 7  ) & 0xFF ); \
         (_pesHeader)[16] = ( ( ( ( _pts ) >> 14 ) & 0xFE ) | (0x01) ); \
         (_pesHeader)[15] = (   ( ( _pts ) >> 22 ) & 0xFF ); \
         (_pesHeader)[14] = ( ( ( ( _pts ) >> 29 ) & 0x0E ) | (0x11) ); \
         (_pesHeader)[9] |= (0x30); \
         (_pesHeader)[7] |= 0x40;

#define BMUXLIB_ASP_TS_P_PESHeader_SetLength(_pesHeader, _length) \
         (_pesHeader)[4] = ( ( ( _length ) >> 8 ) & 0xFF ); \
         (_pesHeader)[5] = ( ( ( _length ) )      & 0xFF );

static void
BMUXlib_ASP_TS_S_Source_AV_ITB_Shadow_Consume(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
   BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstITBEntry
   )
{
   /* Bump up CDB Shadow Read pointer */
   hSourceAVHandle->stShadowIndex.uiRead += BMUXLIB_ASP_ITB_ENTRY_SIZE_IN_BYTES;
   if ( hSourceAVHandle->stShadowIndex.uiRead >= hSourceAVHandle->stIndex.uiEnd )
   {
      hSourceAVHandle->stShadowIndex.uiRead = hSourceAVHandle->stIndex.uiBase +
         (hSourceAVHandle->stShadowIndex.uiRead - hSourceAVHandle->stIndex.uiEnd);
   }

   pstITBEntry->uiITBSize += BMUXLIB_ASP_ITB_ENTRY_SIZE_IN_BYTES;
}

static void
BMUXlib_ASP_TS_S_Source_AV_ITB_Consume(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
   BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstITBEntry
)
{
   /* Bump up ITB Read pointer */
   hSourceAVHandle->stIndex.uiRead += pstITBEntry->uiITBSize;
   if ( hSourceAVHandle->stIndex.uiRead >= hSourceAVHandle->stIndex.uiEnd )
   {
      hSourceAVHandle->stIndex.uiRead = hSourceAVHandle->stIndex.uiBase +
         (hSourceAVHandle->stIndex.uiRead - hSourceAVHandle->stIndex.uiEnd);
   }

   /* Update the actual ITB read pointer */
   hSourceAVHandle->pstRegisterInterface->fWrite64(
     hSourceAVHandle->pstRegisterInterface->pvContext,
     hSourceAVHandle->pstSourceAVInterface->stIndex.uiRead,
     hSourceAVHandle->stIndex.uiRead );

   {
      memset( &pstITBEntry->stFrameInfo, 0, sizeof( pstITBEntry->stFrameInfo ) );
      pstITBEntry->uiITBSize = 0;

      BMUXLIB_ASP_P_SET_MUX_AV_ITB_STATE( pstITBEntry, BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_READ );
   }
}

static void
BMUXlib_ASP_TS_S_Source_AV_CDB_Shadow_Consume(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
   size_t uiSize
   )
{
   /* Bump up CDB Shadow Read pointer */
   hSourceAVHandle->stShadowData.uiRead += uiSize;
   if ( hSourceAVHandle->stShadowData.uiRead >= hSourceAVHandle->stData.uiEnd )
   {
      hSourceAVHandle->stShadowData.uiRead = hSourceAVHandle->stData.uiBase +
         (hSourceAVHandle->stShadowData.uiRead - hSourceAVHandle->stData.uiEnd);
   }
}

static void
BMUXlib_ASP_TS_S_Source_AV_CDB_Consume(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
   size_t uiSize
   )
{
   /* Bump up CDB Read pointer */
   hSourceAVHandle->stData.uiRead += uiSize;
   if ( hSourceAVHandle->stData.uiRead >= hSourceAVHandle->stData.uiEnd )
   {
      hSourceAVHandle->stData.uiRead = hSourceAVHandle->stData.uiBase +
         (hSourceAVHandle->stData.uiRead - hSourceAVHandle->stData.uiEnd);
   }

   /* Update the actual CDB read pointer */
   hSourceAVHandle->pstRegisterInterface->fWrite64(
     hSourceAVHandle->pstRegisterInterface->pvContext,
     hSourceAVHandle->pstSourceAVInterface->stCommon.stData.uiRead,
     hSourceAVHandle->stData.uiRead );
}

static void
BMUXlib_ASP_TS_S_Source_AV_ContextMap_Snapshot_Start(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
  BMUXlib_ASP_Source_ContextValues_t *pstDstContextValues,
  const BMUXlib_ASP_Source_ContextMap_t *pstSrcContextMap
  )
{
   /* Snapshot Context Register Values (BASE/END) */
   pstDstContextValues->uiBase =
      hSourceAVHandle->pstRegisterInterface->fRead64(
         hSourceAVHandle->pstRegisterInterface->pvContext,
         pstSrcContextMap->uiBase
      );
   pstDstContextValues->uiEnd =
      hSourceAVHandle->pstRegisterInterface->fRead64(
         hSourceAVHandle->pstRegisterInterface->pvContext,
         pstSrcContextMap->uiEnd
      );
   pstDstContextValues->uiEnd++;
}

static void
BMUXlib_ASP_TS_S_Source_AV_Snapshot_Start(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle
   )
{
   /* Initialize ITB State */
   memset( &hSourceAVHandle->stITB, 0, sizeof( hSourceAVHandle->stITB ) );
   hSourceAVHandle->stITB.pstCurrentEntry = &hSourceAVHandle->stITB.astEntry[0];
   hSourceAVHandle->stITB.pstNextEntry = &hSourceAVHandle->stITB.astEntry[1];
   hSourceAVHandle->bRegistersInitialized = FALSE;
}

static void
BMUXlib_ASP_TS_S_Source_AV_ContextMap_Snapshot_Reconcile(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
   BMUXlib_ASP_Source_ContextValues_t *pstDstContextValues,
   const BMUXlib_ASP_Source_ContextMap_t *pstSrcContextMap
  )
{
  /* Snapshot Context Register Values (READ/VALID) */
   pstDstContextValues->uiRead =
     hSourceAVHandle->pstRegisterInterface->fRead64(
        hSourceAVHandle->pstRegisterInterface->pvContext,
        pstSrcContextMap->uiRead
     );

   pstDstContextValues->uiValid =
     hSourceAVHandle->pstRegisterInterface->fRead64(
        hSourceAVHandle->pstRegisterInterface->pvContext,
        pstSrcContextMap->uiValid
     );

   pstDstContextValues->uiValid++;
}

static void
BMUXlib_ASP_TS_S_Source_AV_Snapshot_Reconcile(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle
   )
{
   const BMUXlib_ASP_TS_Source_AV_Interface_t *pstSourceAVInterface = hSourceAVHandle->pstSourceAVInterface;

   if ( FALSE == hSourceAVHandle->bRegistersInitialized )
   {
      BMUXlib_ASP_TS_S_Source_AV_ContextMap_Snapshot_Start( hSourceAVHandle, &hSourceAVHandle->stData, &pstSourceAVInterface->stCommon.stData );
      BMUXlib_ASP_TS_S_Source_AV_ContextMap_Snapshot_Start( hSourceAVHandle, &hSourceAVHandle->stIndex, &pstSourceAVInterface->stIndex );

      /* Initialize Shadow Pointers */
      hSourceAVHandle->stShadowData.uiRead = hSourceAVHandle->stData.uiBase;
      hSourceAVHandle->stShadowIndex.uiRead = hSourceAVHandle->stIndex.uiBase;

      hSourceAVHandle->bRegistersInitialized = TRUE;
   }

   BMUXlib_ASP_TS_S_Source_AV_ContextMap_Snapshot_Reconcile( hSourceAVHandle, &hSourceAVHandle->stData, &pstSourceAVInterface->stCommon.stData );
   BMUXlib_ASP_TS_S_Source_AV_ContextMap_Snapshot_Reconcile( hSourceAVHandle, &hSourceAVHandle->stIndex, &pstSourceAVInterface->stIndex );

   BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_ITB, ("[%d] ITB B/E/R/V: "BDBG_UINT64_FMT"/"BDBG_UINT64_FMT"/"BDBG_UINT64_FMT"/"BDBG_UINT64_FMT,
      hSourceAVHandle->pstSourceAVInterface->eType,
      BDBG_UINT64_ARG(hSourceAVHandle->stIndex.uiBase),
      BDBG_UINT64_ARG(hSourceAVHandle->stIndex.uiEnd),
      BDBG_UINT64_ARG(hSourceAVHandle->stIndex.uiRead),
      BDBG_UINT64_ARG(hSourceAVHandle->stIndex.uiValid)
      ));

   BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_CDB, ("[%d] CDB B/E/R/V: "BDBG_UINT64_FMT"/"BDBG_UINT64_FMT"/"BDBG_UINT64_FMT"/"BDBG_UINT64_FMT,
      hSourceAVHandle->pstSourceAVInterface->eType,
      BDBG_UINT64_ARG(hSourceAVHandle->stData.uiBase),
      BDBG_UINT64_ARG(hSourceAVHandle->stData.uiEnd),
      BDBG_UINT64_ARG(hSourceAVHandle->stData.uiRead),
      BDBG_UINT64_ARG(hSourceAVHandle->stData.uiValid)
      ));
}

static bool_t
BMUXlib_ASP_TS_S_Source_AV_IndexExists(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle
   )
{
   /* Use *shadow* pointer For AV Index */
   unsigned uiDepth = 0;
   if ( hSourceAVHandle->stIndex.uiValid >= hSourceAVHandle->stShadowIndex.uiRead )
   {
      uiDepth = hSourceAVHandle->stIndex.uiValid - hSourceAVHandle->stShadowIndex.uiRead;
   }
   else
   {
      uiDepth = hSourceAVHandle->stIndex.uiEnd - hSourceAVHandle->stShadowIndex.uiRead;
      uiDepth += hSourceAVHandle->stIndex.uiBase - hSourceAVHandle->stIndex.uiValid;
   }

   return ( uiDepth >= BMUXLIB_ASP_ITB_ENTRY_SIZE_IN_BYTES );
}

static bool_t
BMUXlib_ASP_TS_S_Source_AV_DataExists(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle
   )
{
   /* Use *shadow* pointer For AV Data */
   return ( hSourceAVHandle->stShadowData.uiRead != hSourceAVHandle->stData.uiValid );
}

static uint8_t auiDefaultPUSIBTPPacket[BMUXLIB_PUSI_DATA_SIZE] =
{
        0x47, /* Sync Byte */
        0x00, 0x00, /* PUSI/PID */
        0x20, /* Scrambling/Adaptation/CC */
        /* Adaptation/Payload */
        0xb7, /* Adaptation field length (183, the remainder of the packet) */
        0x82,
        0x2D, /* Transport private data length */
        0x00, /* Align Byte */
        'B','R','C','M', /* "BRCM" signature for BTP */
        0x00,0x00,0x00,0x18, /* Control Word 1 - BTP Command (MUX_TIMESTAMP_UPDATE) */
        0x00,0x00,0x00,0x0C, /* Control Word 2 - SET_PUSI_IN_NEXT_PKT | SET_ADAPTATION_FIELD_TO_3 */
#if 0
        0xFF,0xFF,0xFF,0xFF, /* Control Word 3 */
        0xFF,0xFF,0xFF,0xFF, /* Control Word 4 */
        0xFF,0xFF,0xFF,0xFF, /* Control Word 5 */
        0xFF,0xFF,0xFF,0xFF, /* Control Word 6 */
        0xFF,0xFF,0xFF,0xFF, /* Control Word 7 */
        0xFF,0xFF,0xFF,0xFF, /* Control Word 8 */
        0xFF,0xFF,0xFF,0xFF, /* Control Word 9 */
        0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
#endif
};

static void
BMUXlib_ASP_TS_S_Source_AV_Generate_PUSIBtp(
   uint8_t auiPacket[]
   )
{
   memset( auiPacket, 0xFF, BMUXLIB_ASP_TS_PACKET_SIZE );
   memcpy( auiPacket, auiDefaultPUSIBTPPacket, BMUXLIB_PUSI_DATA_SIZE );
}

static uint8_t astDefaultPESHeader[BMUXLIB_PES_HEADER_DATA_SIZE] =
{
  0x00, 0x00, 0x01, /* Start Code */
  0xE0, /* Stream ID */
  0x00, 0x00, /* Packet Length */
  0x80, 0x00, /* Extension w/ PTS+DTS invalid */
  0x0A, /* PES Header Data Length */
  0xFF, /* PTS[32:30] */
  0xFF, 0xFF, /* PTS[29:15] */
  0xFF, 0xFF, /* PTS[14:00] */
  0xFF, /* DTS[32:30] */
  0xFF, 0xFF, /* DTS[29:15] */
  0xFF, 0xFF  /* DTS[14:00] */
};

static uint32_t
BMUXlib_ASP_TS_S_Source_AV_Calculate_FrameSize(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle
   )
{
   uint32_t uiFrameSize = 0;
   uint64_t uiCurrentCDBOffset = hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.uiCDBOffset;
   uint64_t uiNextCDBOffset = hSourceAVHandle->stITB.pstNextEntry->stFrameInfo.uiCDBOffset;

   if ( uiNextCDBOffset >= uiCurrentCDBOffset )
   {
      uiFrameSize = uiNextCDBOffset - uiCurrentCDBOffset;
   }
   else
   {
      /* Handle Wrap */
      uiFrameSize = hSourceAVHandle->stData.uiEnd - uiCurrentCDBOffset;
      uiFrameSize += ( uiNextCDBOffset - hSourceAVHandle->stData.uiBase );
   }

   return uiFrameSize;
}

static void
BMUXlib_ASP_TS_S_Source_AV_Generate_PESHeader(
   BMUXlib_Source_Entry_Handle hSourceEntryHandle,
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
   uint8_t auiTSPacket[]
   )
{
   uint8_t uiOffset = 0;

   /* Populate RAI/Adaptation/Stuffing/PES Header */
   memset( auiTSPacket, 0xFF, BMUXLIB_ASP_TS_PAYLOAD_SIZE );

   /* Set Adaptation Length */
   auiTSPacket[uiOffset++] = ( BMUXLIB_ASP_TS_PAYLOAD_SIZE - (19 + 1) );
   /* Set RAI Indicator */
   if ( TRUE == hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.bRAI )
   {
      auiTSPacket[uiOffset++] = 0x40;
   }
   else
   {
      auiTSPacket[uiOffset++] = 0x00;
   }
   /* Skip over adaptation */
   uiOffset += (auiTSPacket[0] - 1);

   /* Initialize PES Header */
   memcpy( &auiTSPacket[uiOffset], astDefaultPESHeader, BMUXLIB_PES_HEADER_DATA_SIZE );

   /* Set Stream ID */
   BMUXLIB_ASP_TS_P_PESHeader_SetStreamID(
      &auiTSPacket[uiOffset],
      hSourceAVHandle->pstSourceAVInterface->uiPESStreamID
      );

   /* Set PTS */
   {
      uint64_t uiPTS = hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.uiPTS;
      uint64_t uiDTS = hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.uiDTS;

      BMUXLIB_ASP_TS_P_PESHeader_SetPTS(
         &auiTSPacket[uiOffset],
         uiPTS
      );

      BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_PES, ("[%d] rai:%u streamId:%u pts:"BDBG_UINT64_FMT,
         hSourceEntryHandle->uiIndex,
         ( TRUE == hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.bRAI ),
         hSourceAVHandle->pstSourceAVInterface->uiPESStreamID,
         BDBG_UINT64_ARG(uiPTS)
         ));

      if ( uiPTS != uiDTS )
      {
         BMUXLIB_ASP_TS_P_PESHeader_SetDTS(
            &auiTSPacket[uiOffset],
            uiDTS
         );
      }
   }

   /* Set Frame Size for Audio */
   switch ( hSourceAVHandle->pstSourceAVInterface->eType )
   {
      case BMUXLIB_ASP_SOURCE_AV_TYPE_RAAGA:
         BMUXLIB_ASP_TS_P_PESHeader_SetLength(
            &auiTSPacket[uiOffset],
            BMUXlib_ASP_TS_S_Source_AV_Calculate_FrameSize( hSourceAVHandle ) + BMUXLIB_PES_HEADER_PAYLOAD_SIZE
            );
         break;

      case BMUXLIB_ASP_SOURCE_AV_TYPE_VICE:
      case BMUXLIB_ASP_SOURCE_AV_TYPE_UNKNOWN:
      case BMUXLIB_ASP_SOURCE_AV_TYPE_MAX:
         /* Do Nothing */
         break;
   }
}

static void
BMUXlib_ASP_TS_S_Source_AV_Output_Descriptor_Initialize(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
   BMUXlib_ASP_Output_Handle hOutputHandle,
   size_t uiPayloadSize,
   BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor,
   BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata
   )
{
   uint64_t uiBufferAddress = hSourceAVHandle->stShadowData.uiRead;

   BMUXlib_ASP_P_Output_Descriptor_Initialize( hOutputHandle, uiBufferAddress, uiPayloadSize, hSourceAVHandle->pstSourceAVInterface->stCommon.uiPIDChannelIndex, pstOutputDescriptor, pstOutputDescriptorMetadata );

   /* Set descriptor metadata for use by ProcessCompletedBuffers */
   pstOutputDescriptorMetadata->fConsumeResource = (BMUXlib_ASP_P_Output_ConsumeResource) BMUXlib_ASP_TS_S_Source_AV_CDB_Consume;
   pstOutputDescriptorMetadata->pvContext = hSourceAVHandle;
   pstOutputDescriptorMetadata->uiResourceId = uiPayloadSize;
}

static uint32_t
BMUXlib_ASP_TS_S_Source_AV_Payload_ComputeSize(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
   bool_t *pbFrameDone
   )
{
   /* Determine how much payload to send */
   uint32_t uiPayloadSize = 0;
   uint64_t uiPayloadEndOffsetUsingCDBValid = hSourceAVHandle->stData.uiValid;
   uint64_t uiPayloadEndOffsetUsingCDBOffset = ( (uint64_t) 0xFFFFFFFF << 32 ) | 0xFFFFFFFF;

   *pbFrameDone = FALSE;

   if ( BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_VALID == BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE( hSourceAVHandle->stITB.pstNextEntry ) )
   {
      /* We have the next ITB entry, so know the absolute max we can send for this frame */
      uiPayloadEndOffsetUsingCDBOffset = hSourceAVHandle->stITB.pstNextEntry->stFrameInfo.uiCDBOffset;
   }

   /* We need to handle the wrap */
   if ( hSourceAVHandle->stShadowData.uiRead > uiPayloadEndOffsetUsingCDBValid )
   {
      /* The frame is on a wrap boundary, so send the 1st part of the frame */
      uiPayloadEndOffsetUsingCDBValid = hSourceAVHandle->stData.uiEnd;
   }

   if ( hSourceAVHandle->stShadowData.uiRead > uiPayloadEndOffsetUsingCDBOffset)
   {
      /* The frame is on a wrap boundary, so send the 1st part of the frame */
      uiPayloadEndOffsetUsingCDBOffset = hSourceAVHandle->stData.uiEnd;
   }

   /* Pick the MIN( uiPayloadEndOffsetUsingCDBValid, uiPayloadEndOffsetUsingCDBOffset ) because we don't want to
    * send data beyond the valid and not beyond the cdb offset */
   if ( uiPayloadEndOffsetUsingCDBValid < uiPayloadEndOffsetUsingCDBOffset )
   {
      uiPayloadSize = uiPayloadEndOffsetUsingCDBValid - hSourceAVHandle->stShadowData.uiRead;
      if ( uiPayloadEndOffsetUsingCDBValid != hSourceAVHandle->stData.uiEnd )
      {
         /* We don't want to send partial packets unless it's the end of frame or the end of buffer, so we ensure the payload size is
          * a multiple of 184 bytes */
         uiPayloadSize -= uiPayloadSize % 184;
      }
   }
   else
   {
      uiPayloadSize = uiPayloadEndOffsetUsingCDBOffset - hSourceAVHandle->stShadowData.uiRead;
      *pbFrameDone = TRUE;
   }

   return uiPayloadSize;
}

#if BDBG_DEBUG_BUILD
static const char* astSourceAVITBStateFriendlyNameLUT[BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_MAX] =
{
   "BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_READ",
   "BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_DMA_START",
   "BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_DMA_FINISH",
   "BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_VALID",
};
#endif

static void
BMUXlib_ASP_TS_S_Source_AV_ITB_SetTiming(
   BMUXlib_Source_Entry_Handle hSourceEntryHandle,
   BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstITBEntry
   )
{
   hSourceEntryHandle->stTiming.uiCurrentESCR = pstITBEntry->stFrameInfo.uiESCR;
   hSourceEntryHandle->stTiming.uiCurrentPTS = pstITBEntry->stFrameInfo.uiPTS;
   hSourceEntryHandle->stTiming.bValid = TRUE;

   BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_TIMING, ("[%d] Timing: %08x/"BDBG_UINT64_FMT,
      hSourceEntryHandle->uiIndex,
      hSourceEntryHandle->stTiming.uiCurrentESCR,
      BDBG_UINT64_ARG(hSourceEntryHandle->stTiming.uiCurrentPTS)
      ));

}

static bool_t
BMUXlib_ASP_TS_S_Source_AV_ITB_StartRead(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
   BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstITBEntry
)
{
   hSourceAVHandle->pstDMAInterface->fDram2Dccm(
     hSourceAVHandle->pstDMAInterface->pvContext,
     hSourceAVHandle->stTemp.auiITBEntry,
     hSourceAVHandle->stShadowIndex.uiRead,
     BMUXLIB_ASP_ITB_ENTRY_SIZE_IN_BYTES,
     &pstITBEntry->pDMAToken
   );

   return ( NULL != pstITBEntry->pDMAToken );
}

static bool_t
BMUXlib_ASP_TS_S_Source_AV_ITB_FinishRead(
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle,
   BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstITBEntry
)
{
   if ( FALSE == hSourceAVHandle->pstDMAInterface->fIdle( hSourceAVHandle->pstDMAInterface->pvContext, &pstITBEntry->pDMAToken ) )
   {
      goto error;
   }

   BMUXlib_ASP_TS_S_Source_AV_ITB_Shadow_Consume( hSourceAVHandle, pstITBEntry );

   return TRUE;
error:
   return FALSE;
}

static void
BMUXlib_ASP_TS_S_Source_AV_ITB_Read(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle,
   BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstITBEntry
   )
{
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle = &hChannelHandle->stContext.stSourceAV[hSourceEntryHandle->uiIndex];
   BMUXlib_TS_ASP_Source_AV_ITB_State_e ePreviousState;

   do
   {
      ePreviousState = BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE( pstITBEntry );
      BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE, ("[%d] %s", hSourceEntryHandle->uiIndex, astSourceAVITBStateFriendlyNameLUT[ePreviousState] ));

      switch ( BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE( pstITBEntry ) )
      {
         case BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_READ:
            if ( TRUE == BMUXlib_ASP_TS_S_Source_AV_IndexExists ( hSourceAVHandle ) )
            {
               BMUXLIB_ASP_P_SET_MUX_AV_ITB_STATE( pstITBEntry, BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_DMA_START );
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_DMA_START:
            /* DMA the ITB entry from the host ITB memory to the internal DCCM */
            if ( TRUE == BMUXlib_ASP_TS_S_Source_AV_ITB_StartRead( hSourceAVHandle, pstITBEntry ) )
            {
               BMUXLIB_ASP_P_SET_MUX_AV_ITB_STATE( pstITBEntry, BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_DMA_FINISH );
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_DMA_FINISH:
            if ( TRUE == BMUXlib_ASP_TS_S_Source_AV_ITB_FinishRead( hSourceAVHandle, pstITBEntry ) )
            {
               uint8_t uiEntryType = BMUXLIB_ASP_ITBEntry_GetEntryType( hSourceAVHandle->stTemp.auiITBEntry );

               /* Read the entry type and store the pointer and increment index if we need it */
               switch ( uiEntryType )
               {
                  case BMUXLIB_ASP_ITBENTRY_TYPE_BASE:
                     pstITBEntry->stFrameInfo.uiCDBOffset = BMUXLIB_ASP_ITBEntry_GetCDBOffset( hSourceAVHandle->stTemp.auiITBEntry );

                     BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_ITB_INDEX, ("[%d] BASE ENTRY (%02x)", hSourceEntryHandle->eSourceType, uiEntryType ));
                     pstITBEntry->stFrameInfo.uiITBEntriesParsed |= BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eBase;
                     break;

                  case BMUXLIB_ASP_ITBENTRY_TYPE_BASE_OFFSET:
                     pstITBEntry->stFrameInfo.uiCDBOffset = BMUXLIB_ASP_ITBEntry_GetCDBOffset( hSourceAVHandle->stTemp.auiITBEntry );
                     pstITBEntry->stFrameInfo.uiCDBOffset += hSourceAVHandle->stData.uiBase;

                     BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_ITB_INDEX, ("[%d] BASE OFFSET ENTRY (%02x)", hSourceEntryHandle->eSourceType, uiEntryType ));
                     pstITBEntry->stFrameInfo.uiITBEntriesParsed |= BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eBase;
                     break;

                  case BMUXLIB_ASP_ITBENTRY_TYPE_TIMESTAMP:
                     pstITBEntry->stFrameInfo.uiPTS = BMUXLIB_ASP_ITBEntry_GetPTS( hSourceAVHandle->stTemp.auiITBEntry );
                     if ( BMUXLIB_ASP_SOURCE_AV_TYPE_VICE == hSourceAVHandle->pstSourceAVInterface->eType )
                     {
                        pstITBEntry->stFrameInfo.uiDTS = BMUXLIB_ASP_ITBEntry_GetDTS( hSourceAVHandle->stTemp.auiITBEntry );
                     }
                     else
                     {
                        pstITBEntry->stFrameInfo.uiDTS = pstITBEntry->stFrameInfo.uiPTS;
                     }
                     pstITBEntry->stFrameInfo.bRAI = (0 != BMUXLIB_ASP_ITBEntry_GetIFrame( hSourceAVHandle->stTemp.auiITBEntry ) );

                     BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_ITB_INDEX, ("[%d] TIMESTAMP ENTRY (%02x)", hSourceEntryHandle->eSourceType, uiEntryType ));
                     pstITBEntry->stFrameInfo.uiITBEntriesParsed |= BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eTimestamp;
                     break;

                  case BMUXLIB_ASP_ITBENTRY_TYPE_BITRATE:
                     pstITBEntry->stFrameInfo.uiTicksPerBit = BMUXLIB_ASP_ITBEntry_GetTicksPerBit( hSourceAVHandle->stTemp.auiITBEntry );
                     pstITBEntry->stFrameInfo.iSHR = BMUXLIB_ASP_ITBEntry_GetSHR( hSourceAVHandle->stTemp.auiITBEntry );

                     BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_ITB_INDEX, ("[%d] BITRATE ENTRY (%02x)", hSourceEntryHandle->eSourceType, uiEntryType ));
                     pstITBEntry->stFrameInfo.uiITBEntriesParsed |= BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eBitrate;
                     break;

                  case BMUXLIB_ASP_ITBENTRY_TYPE_ESCR:
                     pstITBEntry->stFrameInfo.uiESCR = BMUXLIB_ASP_ITBEntry_GetESCR( hSourceAVHandle->stTemp.auiITBEntry );
                     pstITBEntry->stFrameInfo.uiOriginalPTS = BMUXLIB_ASP_ITBEntry_GetOriginalPTS( hSourceAVHandle->stTemp.auiITBEntry );

                     BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_ITB_INDEX, ("[%d] ESCR ENTRY (%02x)", hSourceEntryHandle->eSourceType, uiEntryType ));
                     pstITBEntry->stFrameInfo.uiITBEntriesParsed |= BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eESCR;
                     break;

                  case BMUXLIB_ASP_ITBENTRY_TYPE_EOS:
                     pstITBEntry->stFrameInfo.uiCDBOffset = BMUXLIB_ASP_ITBEntry_GetEOSCDBOffset( hSourceAVHandle->stTemp.auiITBEntry );
                     pstITBEntry->stFrameInfo.uiCDBOffset += hSourceAVHandle->stData.uiBase;
                     pstITBEntry->stFrameInfo.uiESCR = BMUXLIB_ASP_ITBEntry_GetEOSESCR( hSourceAVHandle->stTemp.auiITBEntry );
                     pstITBEntry->stFrameInfo.uiITBEntriesParsed |= BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eAll;
                     break;

                  default:
                     BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_ITB_INDEX, ("[%d] UNKNOWN ENTRY (%02x)", hSourceEntryHandle->eSourceType, uiEntryType ));
                     break;
               }

               if ( pstITBEntry->stFrameInfo.uiITBEntriesParsed != BMUXlib_TS_ASP_Source_AV_ITBEntry_Type_eAll )
               {
                  BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_ITB_INDEX, ("[%d] MORE", hSourceEntryHandle->eSourceType ));
                  /* We don't have all of the ITB data, so read the next ITB entry */
                  BMUXLIB_ASP_P_SET_MUX_AV_ITB_STATE( pstITBEntry, BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_READ );
               }
               else
               {
                  BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_ITB_INDEX, ("[%d] DONE", hSourceEntryHandle->eSourceType ));

                  /* We need to seed the timing params for the first time */
                  if ( FALSE == hSourceEntryHandle->stTiming.bValid )
                  {
                     BMUXlib_ASP_TS_S_Source_AV_ITB_SetTiming( hSourceEntryHandle, pstITBEntry );
                  }
                  BMUXLIB_ASP_P_SET_MUX_AV_ITB_STATE( pstITBEntry, BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_VALID );
               }
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_VALID:
            /* Do Nothing */
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_MAX:
            /* TODO: Add assert - Should never come here */
            break;
      }
   } while ( ePreviousState != BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE( pstITBEntry ) );
}

static uint32_t
BMUXlib_ASP_TS_S_Source_AV_CalculatePacket2PacketDelta(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle
   )
{
   uint32_t uiPacket2PacketTimestampDelta = 0;
   size_t uiFrameSize = BMUXlib_ASP_TS_S_Source_AV_Calculate_FrameSize( hSourceAVHandle );
   uint32_t uiTotalPackets = ( uiFrameSize + ( BMUXLIB_ASP_TS_PAYLOAD_SIZE - 1 ) )/BMUXLIB_ASP_TS_PAYLOAD_SIZE;
   uint64_t uiTotalTicks = 0;

   /* We need to add 1 packet extra for the PES header */
   uiTotalPackets++;

   /* Account for possible A/V/S packet interleave */
   uiTotalPackets += ( hChannelHandle->stStartSettings.uiNumValidSource + 1 );
   uiTotalPackets += hChannelHandle->stStartSettings.stUserData.uiNumValidUserData;

   uiTotalTicks = hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.uiTicksPerBit;
   uiTotalTicks *= ( uiFrameSize * 8 );

   if ( hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.iSHR > 0 )
   {
      uiTotalTicks >>= hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.iSHR;
   }
   else
   {
      uiTotalTicks <<= -hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.iSHR;
   }

   uiPacket2PacketTimestampDelta = uiTotalTicks / uiTotalPackets;

   return uiPacket2PacketTimestampDelta;
}


/* Public Functions */
void
BMUXlib_ASP_TS_P_Source_AV_Start(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle
   )
{
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle = &hChannelHandle->stContext.stSourceAV[hSourceEntryHandle->uiIndex];

   BDBG_ENTER( BMUXlib_ASP_TS_P_Source_AV_Start );

   /* Setup Source A/V Context */
   hSourceAVHandle->pstSourceAVInterface = &hChannelHandle->stStartSettings.stSource[hSourceEntryHandle->uiIndex];
   hSourceAVHandle->pstRegisterInterface = &hChannelHandle->stStartSettings.stRegister;
   hSourceAVHandle->pstDMAInterface = &hChannelHandle->stStartSettings.stDMA;

   /* Snapshot ITB/CDB Base/End once at start since they should not change during the transcode */
   BMUXlib_ASP_TS_S_Source_AV_Snapshot_Start( hSourceAVHandle );

   BDBG_LEAVE( BMUXlib_ASP_TS_P_Source_AV_Start );
}

bool_t
BMUXlib_ASP_TS_P_Source_AV_Reconcile(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle
   )
{
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle = &hChannelHandle->stContext.stSourceAV[hSourceEntryHandle->uiIndex];
   BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstITBEntry = hSourceAVHandle->stITB.pstCurrentEntry;

   BDBG_ENTER( BMUXlib_ASP_TS_P_Source_AV_Reconcile );

   BMUXlib_ASP_TS_S_Source_AV_Snapshot_Reconcile( hSourceAVHandle );

   BMUXlib_ASP_TS_S_Source_AV_ITB_Read( hChannelHandle, hSourceEntryHandle, pstITBEntry );

   /* Handle initial startup condition */
   if ( ( BMUXLIB_ASP_TS_SOURCE_AV_STATE_GET_FIRST_ITB == BMUXLIB_ASP_P_GET_MUX_AV_STATE( hSourceAVHandle ) )
        && ( BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_VALID == BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE( pstITBEntry ) ) )
   {
      BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_GET_NEXT_ITB );
   }

   BDBG_LEAVE( BMUXlib_ASP_TS_P_Source_AV_Reconcile );

   /* Here we want to return TRUE if there's CDB data but ARE NOT in the middle of reading the next ITB entry */
   return ( ( TRUE == hSourceEntryHandle->stTiming.bValid )
            && ( ( BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_VALID == BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE( pstITBEntry ) ) /* An ITB entry has been DMA'd to DCCM but hasn't been fully processed, yet */
                 || ( ( BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_READ == BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE( pstITBEntry ) ) /* An unprocessed ITB entry doesn't exist */
                      && ( BMUXlib_ASP_TS_S_Source_AV_DataExists ( hSourceAVHandle ) ) ) ) ); /* CDB data exists */
}

#if BDBG_DEBUG_BUILD
static const char* astSourceAVStateFriendlyNameLUT[BMUXLIB_ASP_TS_SOURCE_AV_STATE_MAX] =
{
   "SOURCE_AV_STATE_GET_FIRST_ITB",
   "SOURCE_AV_STATE_GET_NEXT_ITB",
   "SOURCE_AV_STATE_FRAME_START",
   "SOURCE_AV_STATE_FRAME_START_PUSI_GENERATE",
   "SOURCE_AV_STATE_FRAME_START_PUSI_DMA_START",
   "SOURCE_AV_STATE_FRAME_START_PUSI_DMA_FINISH",
   "SOURCE_AV_STATE_FRAME_START_PUSI_SEND",
   "SOURCE_AV_STATE_FRAME_START_PES_HEADER",
   "SOURCE_AV_STATE_FRAME_START_PES_HEADER_GENERATE",
   "SOURCE_AV_STATE_FRAME_START_PES_HEADER_DMA_START",
   "SOURCE_AV_STATE_FRAME_START_PES_HEADER_DMA_FINISH",
   "SOURCE_AV_STATE_FRAME_START_PES_HEADER_SEND",
   "SOURCE_AV_STATE_PAYLOAD",
   "SOURCE_AV_STATE_EOS"
};
#endif

bool_t
BMUXLIB_ASP_ITBEntry_IsEOS(
   BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstITBEntry
   )
{
   return ( ( 0 == pstITBEntry->stFrameInfo.iSHR )
            && ( 0 == pstITBEntry->stFrameInfo.uiTicksPerBit )
            && ( 0 == pstITBEntry->stFrameInfo.uiPTS )
            && ( 0 == pstITBEntry->stFrameInfo.uiDTS )
            && ( 0 == pstITBEntry->stFrameInfo.uiOriginalPTS )
           );
}

bool_t
BMUXlib_ASP_TS_P_Source_AV_DoMux(
   BMUXlib_ASP_TS_Channel_Handle hChannelHandle,
   BMUXlib_Source_Entry_Handle hSourceEntryHandle,
   uint32_t uiEndESCR
   )
{
   BMUXlib_ASP_TS_Source_AV_State_e ePreviousState;
   BMUXlib_ASP_TS_Memory_Handle hMemoryHandle = &hChannelHandle->stContext.stMemory;
   BMUXlib_ASP_Output_Handle hOutputHandle = &hChannelHandle->stContext.stOutput;
   BMUXlib_ASP_TS_Source_AV_Handle hSourceAVHandle = &hChannelHandle->stContext.stSourceAV[hSourceEntryHandle->uiIndex];
   uint32_t uiStartESCR = hSourceEntryHandle->stTiming.uiCurrentESCR;

   BDBG_ENTER( BMUXlib_ASP_TS_P_Source_AV_DoMux );

   do
   {
      ePreviousState = BMUXLIB_ASP_P_GET_MUX_AV_STATE( hSourceAVHandle );
      BDBG_MODULE_MSG( BMUXLIB_ASP_TS_SOURCE_AV_STATE, ("[%d] %s", hSourceEntryHandle->uiIndex, astSourceAVStateFriendlyNameLUT[ePreviousState] ));

      switch ( ePreviousState )
      {
         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_GET_FIRST_ITB:
            /* This is the initial state where we read the first ITB entry. */
            BMUXlib_ASP_TS_S_Source_AV_ITB_Read( hChannelHandle, hSourceEntryHandle, hSourceAVHandle->stITB.pstCurrentEntry );
            if ( BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_VALID == BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE( hSourceAVHandle->stITB.pstCurrentEntry ) )
            {
               BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_GET_NEXT_ITB );
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_GET_NEXT_ITB:
            /* If the current ITB is an EOS, we are done */
            if ( BMUXLIB_ASP_ITBEntry_IsEOS( hSourceAVHandle->stITB.pstCurrentEntry ) )
            {
               BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_EOS );
            }
            else
            {
               BMUXlib_ASP_TS_S_Source_AV_ITB_Read( hChannelHandle, hSourceEntryHandle, hSourceAVHandle->stITB.pstNextEntry );

               if ( BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_VALID == BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE( hSourceAVHandle->stITB.pstNextEntry ) )
               {
                  BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START );
               }
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START:
            BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_GENERATE );
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_GENERATE:
            if ( TRUE == BMUXlib_ASP_TS_P_Memory_Packet_Available( hMemoryHandle ) )
            {
               BMUXlib_ASP_TS_S_Source_AV_Generate_PUSIBtp( hSourceAVHandle->stTemp.auiTSPacket );

               BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_DMA_START );
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_DMA_START:
            if ( TRUE == BMUXlib_ASP_TS_P_Memory_Packet_StartWrite( hMemoryHandle, hSourceAVHandle->stTemp.auiTSPacket, &hSourceAVHandle->stTemp.uiBufferOffset, &hSourceAVHandle->pDMAToken ) )
            {
               BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_DMA_FINISH );
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_DMA_FINISH:
            if ( TRUE == BMUXlib_ASP_TS_P_Memory_Packet_FinishWrite( hMemoryHandle, &hSourceAVHandle->pDMAToken ) )
            {
               BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_SEND );
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PUSI_SEND:
            if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Available( hOutputHandle ) )
            {
               BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor = &hChannelHandle->stTemp.stOutput.astTransportDescriptors[0];
               BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata = &hChannelHandle->stTemp.stOutput.stOutputDescriptorMetadata;

               BMUXlib_ASP_TS_P_Memory_Output_Descriptor_Initialize( hMemoryHandle, hOutputHandle, hSourceAVHandle->stTemp.uiBufferOffset, BMUXLIB_ASP_TS_PACKET_SIZE, hSourceAVHandle->pstSourceAVInterface->stCommon.uiPIDChannelIndex, pstOutputDescriptor, pstOutputDescriptorMetadata );

               /* Set Next Packet Pacing Timestamp */
               pstOutputDescriptor->uiNextPacketPacingTimestamp = hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.uiESCR;
               pstOutputDescriptor->bNextPacketPacingTimestampValid = TRUE;

               /* Set Packet2Packet Timestamp Delta */
               pstOutputDescriptor->uiPacket2PacketTimestampDelta = 0; /* This is a PUSI BTP packet that will be consumed by RAVE and not be visible in the output,
                                                                        * so we don't want to increment the PACING_COUNTER with this packet.
                                                                        * We need to set it in the PESHeader so that the proper pacing occurs */
               pstOutputDescriptor->bPacket2PacketTimestampDeltaValid = TRUE;

               /* Set PUSI specific flags */
               pstOutputDescriptor->bPushPreviousPartialPacket = TRUE;
               pstOutputDescriptor->bHostDataInsertion = TRUE;
               pstOutputDescriptor->bInsertHostDataAsBtp = TRUE;

               if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Send( hOutputHandle, pstOutputDescriptor, pstOutputDescriptorMetadata, &hSourceEntryHandle->stTiming ) )
               {
                  BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER );
               }
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER:
            BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_GENERATE );
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_GENERATE:
            BMUXlib_ASP_TS_S_Source_AV_Generate_PESHeader( hSourceEntryHandle, hSourceAVHandle, hSourceAVHandle->stTemp.auiTSPacket );
            BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_DMA_START );
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_DMA_START:
            if ( TRUE == BMUXlib_ASP_TS_P_Memory_Packet_StartWrite( hMemoryHandle, hSourceAVHandle->stTemp.auiTSPacket, &hSourceAVHandle->stTemp.uiBufferOffset, &hSourceAVHandle->pDMAToken ) )
            {
               BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_DMA_FINISH );
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_DMA_FINISH:
            if ( TRUE == BMUXlib_ASP_TS_P_Memory_Packet_FinishWrite( hMemoryHandle, &hSourceAVHandle->pDMAToken ) )
            {
               BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_SEND );
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_FRAME_START_PES_HEADER_SEND:
            if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Available( hOutputHandle ) )
            {
               BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor = &hChannelHandle->stTemp.stOutput.astTransportDescriptors[0];
               BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata = &hChannelHandle->stTemp.stOutput.stOutputDescriptorMetadata;

               BMUXlib_ASP_TS_P_Memory_Output_Descriptor_Initialize( hMemoryHandle, hOutputHandle, hSourceAVHandle->stTemp.uiBufferOffset, BMUXLIB_ASP_TS_PAYLOAD_SIZE, hSourceAVHandle->pstSourceAVInterface->stCommon.uiPIDChannelIndex, pstOutputDescriptor, pstOutputDescriptorMetadata );

               /* Set Packet2Packet Timestamp Delta */
               /* This is the PESHeader, so we set the Packet2Packet Timestamp Delta
                * so that the PACING_COUNTER can increment accordingly. This is the
                * delta that transport will use until we set it again */
               pstOutputDescriptor->uiPacket2PacketTimestampDelta = BMUXlib_ASP_TS_S_Source_AV_CalculatePacket2PacketDelta( hChannelHandle, hSourceAVHandle );
               pstOutputDescriptor->bPacket2PacketTimestampDeltaValid = TRUE;

               /* Set PES Header specific flags */
               pstOutputDescriptor->bRandomAccessIndication = hSourceAVHandle->stITB.pstCurrentEntry->stFrameInfo.bRAI;

               if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Send( hOutputHandle, pstOutputDescriptor, pstOutputDescriptorMetadata, &hSourceEntryHandle->stTiming ) )
               {
                  /* Update the ITB Read Pointer */
                  BMUXlib_ASP_TS_S_Source_AV_ITB_Consume( hSourceAVHandle, hSourceAVHandle->stITB.pstCurrentEntry );

                 BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_PAYLOAD );
               }
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_PAYLOAD:
            if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Available( hOutputHandle ) )
            {
               bool_t bFrameDone = FALSE;
               size_t uiPayloadSize = BMUXlib_ASP_TS_S_Source_AV_Payload_ComputeSize( hSourceAVHandle, &bFrameDone );

               BDBG_ASSERT( 0 != hSourceEntryHandle->stTiming.uiCurrentPacket2PacketTimestampDelta );
               /* Calculate how much payload until uiEndESCR */
               {
                  size_t uiMaxPayloadSize = ( ( (uiEndESCR - hSourceEntryHandle->stTiming.uiCurrentESCR) + ( hSourceEntryHandle->stTiming.uiCurrentPacket2PacketTimestampDelta - 1 ) ) / hSourceEntryHandle->stTiming.uiCurrentPacket2PacketTimestampDelta ) * BMUXLIB_ASP_TS_PAYLOAD_SIZE;
                  if ( 0 == uiMaxPayloadSize )
                  {
                     /* Always send at least one packet to avoid deadlock when two inputs have the same currentESCR */
                     uiMaxPayloadSize = BMUXLIB_ASP_TS_PAYLOAD_SIZE;
                  }
                  if ( uiPayloadSize > uiMaxPayloadSize )
                  {
                     uiPayloadSize = uiMaxPayloadSize;
                  }
               }

               /* TODO: Update logic to Use BMUXlib_ASP_P_Source_Helper_Output_Descriptor_Initialize() */

               if ( FALSE == bFrameDone )
               {
                  size_t uiPayloadSizeAlignmentAdjustment = ( uiPayloadSize % BMUXLIB_ASP_TS_PAYLOAD_SIZE );

                  /* If we are not done with the frame, we want to send only multiples of the packet size so that we don't waste bits with stuffing bits */
                  if ( 0 != uiPayloadSizeAlignmentAdjustment )
                  {
                     uiPayloadSize -= uiPayloadSizeAlignmentAdjustment;
                     /* Artifically bump up the currentESCR timing by one packet, so that we can induce scheduling of the next payload */
                     hSourceEntryHandle->stTiming.uiCurrentESCR += hSourceEntryHandle->stTiming.uiCurrentPacket2PacketTimestampDelta;
                  }
               }

               if ( 0 != uiPayloadSize )
               {
                  BMUXlib_ASP_Output_Descriptor_t *pstOutputDescriptor = &hChannelHandle->stTemp.stOutput.astTransportDescriptors[0];
                  BMUXlib_ASP_Output_Descriptor_Metadata_t *pstOutputDescriptorMetadata = &hChannelHandle->stTemp.stOutput.stOutputDescriptorMetadata;

                  BMUXlib_ASP_TS_S_Source_AV_Output_Descriptor_Initialize( hSourceAVHandle, hOutputHandle, uiPayloadSize, pstOutputDescriptor, pstOutputDescriptorMetadata );

                  pstOutputDescriptor->bPushPartialPacket = bFrameDone;

                  if ( TRUE == BMUXlib_ASP_P_Output_Descriptor_Send( hOutputHandle, pstOutputDescriptor, pstOutputDescriptorMetadata, &hSourceEntryHandle->stTiming ) )
                  {
                     /* Bump up the shadow read pointer */
                     BMUXlib_ASP_TS_S_Source_AV_CDB_Shadow_Consume( hSourceAVHandle, uiPayloadSize );

                     if ( hSourceAVHandle->stITB.pstNextEntry->stFrameInfo.uiCDBOffset == hSourceAVHandle->stShadowData.uiRead )
                     {
                        /* We are done with this frame */
                        BDBG_ASSERT( BMUXLIB_ASP_TS_SOURCE_AV_ITB_STATE_VALID == BMUXLIB_ASP_P_GET_MUX_AV_ITB_STATE( hSourceAVHandle->stITB.pstNextEntry ) );

                        {
                           BMUXlib_TS_ASP_Source_AV_ITBEntry_t *pstTempEntry = hSourceAVHandle->stITB.pstCurrentEntry;
                           hSourceAVHandle->stITB.pstCurrentEntry = hSourceAVHandle->stITB.pstNextEntry;
                           hSourceAVHandle->stITB.pstNextEntry = pstTempEntry;

                           /* We want to set the timing parameters for this source with the new ITB data */
                           BMUXlib_ASP_TS_S_Source_AV_ITB_SetTiming( hSourceEntryHandle, hSourceAVHandle->stITB.pstCurrentEntry );
                        }

                        BMUXLIB_ASP_P_SET_MUX_AV_STATE( hSourceAVHandle, BMUXLIB_ASP_TS_SOURCE_AV_STATE_GET_NEXT_ITB );
                     }
                  }
               }
            }
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_EOS:
            /* Update the ITB Read Pointer */
            BMUXlib_ASP_TS_S_Source_AV_ITB_Consume( hSourceAVHandle, hSourceAVHandle->stITB.pstCurrentEntry );

            hSourceEntryHandle->bActive = FALSE;
            break;

         case BMUXLIB_ASP_TS_SOURCE_AV_STATE_MAX:
            /* TODO: Add assert - Should never come here */
            break;
      };
   } while ( ( BMUXLIB_ASP_TS_SOURCE_AV_STATE_GET_FIRST_ITB == ePreviousState )
             || ( ( ePreviousState != BMUXLIB_ASP_P_GET_MUX_AV_STATE( hSourceAVHandle ) )  /* The state is still changing, so we have enough resources to make progress */
                && ( ( (int32_t) ( uiEndESCR - hSourceEntryHandle->stTiming.uiCurrentESCR ) ) > 0 ) ) ); /* We haven't exceeded the endESCR */

   BDBG_LEAVE( BMUXlib_ASP_TS_P_Source_AV_DoMux );

   /* Return TRUE if more data exists and and we've made progress during this execution */
   return ( ( uiStartESCR != hSourceEntryHandle->stTiming.uiCurrentESCR )
            && ( BMUXlib_ASP_TS_S_Source_AV_DataExists ( hSourceAVHandle ) ) );
}
