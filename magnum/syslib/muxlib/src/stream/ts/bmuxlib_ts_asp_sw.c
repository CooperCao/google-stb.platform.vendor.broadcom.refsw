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

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */

#include "bmuxlib_ts.h"
#include "bmuxlib_ts_asp_priv.h"

BDBG_MODULE(BMUXlib_TS_ASP_SW);

static unsigned
BMUXlib_TS_ASP_S_FindMemoryBlockIndexFromOffset(
   BMUXlib_TS_ASP_Handle hMuxTS,
   BMMA_DeviceOffset uiOffset
   )
{
   unsigned uiMemoryBlockIndex = 0;
   bool bMemoryBlockTypeBestMatchValid = false;
   unsigned uiMemoryBlockIndexBestMatch = 0;

   for ( uiMemoryBlockIndex = 0; uiMemoryBlockIndex < BMUXlib_TS_MemoryBlock_Type_eMax; uiMemoryBlockIndex++ )
   {
      if ( NULL == hMuxTS->stMemoryBlock[uiMemoryBlockIndex].hBlock ) continue;

      if ( uiOffset >= hMuxTS->stMemoryBlock[uiMemoryBlockIndex].uiOffset )
      {
         if ( ( true == bMemoryBlockTypeBestMatchValid )
              && ( hMuxTS->stMemoryBlock[uiMemoryBlockIndex].uiOffset < hMuxTS->stMemoryBlock[uiMemoryBlockIndexBestMatch].uiOffset ) )
         {
            continue;
         }
         uiMemoryBlockIndexBestMatch = uiMemoryBlockIndex;
         bMemoryBlockTypeBestMatchValid = true;
      }
   }

   if ( false == bMemoryBlockTypeBestMatchValid )
   {
      BDBG_ERR(("Could not find matching block, defaulting to 0"));
   }

   return uiMemoryBlockIndexBestMatch;
}

static void
BMUXlib_TS_ASP_S_DMA_Host2ASP(
   void *pvContext,
   void* pBuffer,
   uint64_t uiOffset,
   unsigned uiSize,
   void **pDMAToken
   )
{
   BMUXlib_TS_ASP_Handle hMuxTS = (BMUXlib_TS_ASP_Handle) pvContext;

   unsigned uiMemoryBlockIndex = BMUXlib_TS_ASP_S_FindMemoryBlockIndexFromOffset( hMuxTS, uiOffset );
   void *pSrcBuffer = (void*) ( (unsigned) hMuxTS->stMemoryBlock[uiMemoryBlockIndex].pBuffer + (unsigned) ( uiOffset - hMuxTS->stMemoryBlock[uiMemoryBlockIndex].uiOffset ) );

   BMMA_FlushCache( hMuxTS->stMemoryBlock[uiMemoryBlockIndex].hBlock, pSrcBuffer, uiSize );

   BKNI_Memcpy(
      pBuffer,
      pSrcBuffer,
      uiSize
      );

   *pDMAToken = (void*) 1;
}

static void
BMUXlib_TS_ASP_S_DMA_ASP2Host(
   void *pvContext,
   uint64_t uiOffset,
   const void* pBuffer,
   unsigned uiSize,
   void **pDMAToken
   )
{
   BMUXlib_TS_ASP_Handle hMuxTS = (BMUXlib_TS_ASP_Handle) pvContext;

   unsigned uiMemoryBlockIndex = BMUXlib_TS_ASP_S_FindMemoryBlockIndexFromOffset( hMuxTS, uiOffset );
   void *pDstBuffer = (void*) ( (unsigned) hMuxTS->stMemoryBlock[uiMemoryBlockIndex].pBuffer + (unsigned) ( uiOffset - hMuxTS->stMemoryBlock[uiMemoryBlockIndex].uiOffset ) );

   BKNI_Memcpy(
      pDstBuffer,
      pBuffer,
      uiSize
      );

   BMMA_FlushCache( hMuxTS->stMemoryBlock[uiMemoryBlockIndex].hBlock, pDstBuffer, uiSize );

   *pDMAToken = (void*) 1;
}

static bool
BMUXlib_TS_ASP_S_DMA_Idle(
   void *pvContext,
   void **pDMAToken
   )
{
   BSTD_UNUSED( pvContext );
   *pDMAToken = NULL;

   return true;
}

static uint64_t
BMUXlib_TS_ASP_S_RegRead64(
   void *pvContext,
   uint32_t uiRegister
   )
{
   return BREG_Read64( pvContext, uiRegister );
}

static void
BMUXlib_TS_ASP_S_RegWrite64(
   void *pvContext,
   uint32_t uiRegister,
   uint64_t uiValue
   )
{
   BREG_Write64( pvContext, (uint32_t) uiRegister, (uint32_t) uiValue );
}

static int
BMUXlib_TS_ASP_S_AddTransportDescriptors(
      void *pvContext,
      const BMUXlib_ASP_Output_Descriptor_t *astOutputDescriptors,
      size_t uiCount,
      size_t *puiQueuedCount
      )
{
   BMUXlib_TS_ASP_Handle hMuxTS = (BMUXlib_TS_ASP_Handle) pvContext;
   unsigned i;
   BERR_Code rc;

    BDBG_ASSERT(astOutputDescriptors);
    BDBG_ASSERT(puiQueuedCount);
    *puiQueuedCount=0;
    for( i=0; i < uiCount; i++ )
    {
       size_t uiQueued;
       const BMUXlib_ASP_Output_Descriptor_t *pstAspTransportDescriptor = &astOutputDescriptors[i];
       BMUXlib_TS_TransportDescriptor astTransportDescriptor[1];
       BKNI_Memset( astTransportDescriptor, 0, sizeof( *astTransportDescriptor ) );

       astTransportDescriptor[0].uiBufferOffset = pstAspTransportDescriptor->uiBufferAddressHi;
       astTransportDescriptor[0].uiBufferOffset <<= 32;
       astTransportDescriptor[0].uiBufferOffset |= pstAspTransportDescriptor->uiBufferAddressLo;
       astTransportDescriptor[0].uiBufferLength = pstAspTransportDescriptor->uiBufferLength;

       astTransportDescriptor[0].stTsMuxDescriptorConfig.uiNextPacketPacingTimestamp = pstAspTransportDescriptor->uiNextPacketPacingTimestamp;
       astTransportDescriptor[0].stTsMuxDescriptorConfig.uiPacket2PacketTimestampDelta = pstAspTransportDescriptor->uiPacket2PacketTimestampDelta;
       astTransportDescriptor[0].stTsMuxDescriptorConfig.uiPidChannelNo = pstAspTransportDescriptor->uiPidChannelIndex;
       astTransportDescriptor[0].stTsMuxDescriptorConfig.bNextPacketPacingTimestampValid = pstAspTransportDescriptor->bNextPacketPacingTimestampValid;
       astTransportDescriptor[0].stTsMuxDescriptorConfig.bPacket2PacketTimestampDeltaValid = pstAspTransportDescriptor->bPacket2PacketTimestampDeltaValid;
       astTransportDescriptor[0].stTsMuxDescriptorConfig.bPidChannelValid = pstAspTransportDescriptor->bPidChannelIndexValid;
       astTransportDescriptor[0].stTsMuxDescriptorConfig.bRandomAccessIndication = pstAspTransportDescriptor->bRandomAccessIndication;
       astTransportDescriptor[0].stTsMuxDescriptorConfig.bPushPartialPacket = pstAspTransportDescriptor->bPushPartialPacket;
       astTransportDescriptor[0].stTsMuxDescriptorConfig.bPushPreviousPartialPacket = pstAspTransportDescriptor->bPushPreviousPartialPacket;
       astTransportDescriptor[0].stTsMuxDescriptorConfig.bHostDataInsertion = pstAspTransportDescriptor->bHostDataInsertion;
       astTransportDescriptor[0].stTsMuxDescriptorConfig.bInsertHostDataAsBtp = pstAspTransportDescriptor->bInsertHostDataAsBtp;

       rc = hMuxTS->stStartSettings.transport.stChannelInterface[0].fAddTransportDescriptors(
             hMuxTS->stStartSettings.transport.stChannelInterface[0].pContext,
             astTransportDescriptor, 1, &uiQueued);
       if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
       if ( 0 == uiQueued ) break;
       *puiQueuedCount += uiQueued;
    }
    return BERR_SUCCESS;
}

static int
BMUXlib_TS_ASP_S_GetCompletedTransportDescriptors(
   void *pvContext,
   size_t *puiCompletedCount
   )
{
   BMUXlib_TS_ASP_Handle hMuxTS = (BMUXlib_TS_ASP_Handle) pvContext;
   BERR_Code rc;

   rc = hMuxTS->stStartSettings.transport.stChannelInterface[0].fGetCompletedTransportDescriptors(
         hMuxTS->stStartSettings.transport.stChannelInterface[0].pContext,
         puiCompletedCount);
   return BERR_TRACE(rc);
}

BERR_Code
BMUXlib_TS_ASP_SW_SetSettings(
      BMUXlib_TS_ASP_Handle hMuxTS
      )
{
   /* TODO: Add BDBG_CASSERT to check sizeof( BMUXlib_ASP_ChannelStartSettings_t ) */
   BMUXlib_ASP_TS_ChannelStartSettings_t *pstAspStartSettings = &hMuxTS->stAspChannelContext.stStartSettings;
   BKNI_Memset( pstAspStartSettings, 0, sizeof( *pstAspStartSettings ) );

   /* Set DMA Functions */
   {
      pstAspStartSettings->stDMA.pvContext = hMuxTS;
      pstAspStartSettings->stDMA.fDram2Dccm = BMUXlib_TS_ASP_S_DMA_Host2ASP;
      pstAspStartSettings->stDMA.fDccm2Dram = BMUXlib_TS_ASP_S_DMA_ASP2Host;
      pstAspStartSettings->stDMA.fIdle = BMUXlib_TS_ASP_S_DMA_Idle;
   }

   /* Set Register Access Functions */
   {
      pstAspStartSettings->stRegister.pvContext = hMuxTS->stCreateSettings.hReg;
      pstAspStartSettings->stRegister.fRead64 = BMUXlib_TS_ASP_S_RegRead64;
      pstAspStartSettings->stRegister.fWrite64 = BMUXlib_TS_ASP_S_RegWrite64;
   }

   /* Set Memory Interface */
   {
      pstAspStartSettings->stMemory.uiOffset = hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eTSBuffer].uiOffset;
      pstAspStartSettings->stMemory.uiSize = hMuxTS->stCreateSettings.stMemoryConfig.uiSharedBufferSize;
   }

   /* Setup Output */
   pstAspStartSettings->stOutput.pvContext = hMuxTS;
   pstAspStartSettings->stOutput.fAddTransportDescriptors = BMUXlib_TS_ASP_S_AddTransportDescriptors;
   pstAspStartSettings->stOutput.fGetCompletedTransportDescriptors = BMUXlib_TS_ASP_S_GetCompletedTransportDescriptors;

   /* A/V Source will be set later in DoMux() */

   /* Setup System Data */
   {
      pstAspStartSettings->stSystemData.uiBitRate = hMuxTS->stMuxSettings.uiSystemDataBitRate;
      pstAspStartSettings->stSystemData.uiPIDChannelIndex = hMuxTS->stStartSettings.stPCRData.uiPIDChannelIndex;
      pstAspStartSettings->stSystemData.stPCR.uiPID = hMuxTS->stStartSettings.stPCRData.uiPID;
      pstAspStartSettings->stSystemData.stPCR.uiInterval = hMuxTS->stStartSettings.stPCRData.uiInterval;
   }

   /* Setup User Data */
   {
      pstAspStartSettings->stUserData.uiOffset = hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eUserdataInterface].uiOffset;
      pstAspStartSettings->stUserData.uiNumValidUserData = 1 + hMuxTS->stStartSettings.uiNumValidUserdataPIDs;
   }


   return BERR_SUCCESS;
}

BERR_Code
BMUXlib_TS_ASP_SW_SetAVSourceSettings(
      BMUXlib_TS_ASP_Handle hMuxTS,
      const BMUXlib_CompressedBufferStatus *pstBufferStatus,
      BMUXlib_ASP_Source_AV_Type_e eType,
      unsigned uiIndex
      )
{
   BMUXlib_ASP_TS_ChannelStartSettings_t *pstAspStartSettings = &hMuxTS->stAspChannelContext.stStartSettings;
   BMUXlib_TS_StartSettings *pstStartSettings = &hMuxTS->stStartSettings;

   BDBG_MSG(("%s[%d] ITB: B:%08x E:%08x R:%08x V:%08x",
         ( BMUXLIB_ASP_SOURCE_AV_TYPE_VICE == eType ) ? "Video" : "Audio",
         uiIndex,
         pstBufferStatus->stContext.stIndex.uiBase,
         pstBufferStatus->stContext.stIndex.uiEnd,
         pstBufferStatus->stContext.stIndex.uiRead,
         pstBufferStatus->stContext.stIndex.uiValid
         ));

   BDBG_MSG(("%s[%d] CDB: B:%08x E:%08x R:%08x V:%08x",
         ( BMUXLIB_ASP_SOURCE_AV_TYPE_VICE == eType ) ? "Video" : "Audio",
               uiIndex,
         pstBufferStatus->stContext.stData.uiBase,
         pstBufferStatus->stContext.stData.uiEnd,
         pstBufferStatus->stContext.stData.uiRead,
         pstBufferStatus->stContext.stData.uiValid
         ));

   pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].stCommon.eType = BMUXLIB_ASP_SOURCE_TYPE_INDEXED;
   pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].stCommon.stData.uiBase = pstBufferStatus->stContext.stData.uiBase;
   pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].stCommon.stData.uiEnd= pstBufferStatus->stContext.stData.uiEnd;
   pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].stCommon.stData.uiRead = pstBufferStatus->stContext.stData.uiRead;
   pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].stCommon.stData.uiValid = pstBufferStatus->stContext.stData.uiValid;

   pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].eType = eType;
   pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].stIndex.uiBase = pstBufferStatus->stContext.stIndex.uiBase;
   pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].stIndex.uiEnd= pstBufferStatus->stContext.stIndex.uiEnd;
   pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].stIndex.uiRead = pstBufferStatus->stContext.stIndex.uiRead;
   pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].stIndex.uiValid = pstBufferStatus->stContext.stIndex.uiValid;

   {
      unsigned uiPIDChannelIndex;
      uint8_t uiPESStreamID;
#if 0
      bool bEnablePESPacking;
#endif
      unsigned uiMemoryBlockIndex;

      switch ( eType )
      {
      case BMUXLIB_ASP_SOURCE_AV_TYPE_VICE:
         uiPIDChannelIndex = pstStartSettings->video[uiIndex].uiPIDChannelIndex;
         uiPESStreamID = pstStartSettings->video[uiIndex].uiPESStreamID;
         uiMemoryBlockIndex = BMUXlib_TS_MemoryBlock_Type_eVideoIndex + uiIndex;
         break;

      case BMUXLIB_ASP_SOURCE_AV_TYPE_RAAGA:
         uiPIDChannelIndex = pstStartSettings->audio[uiIndex].uiPIDChannelIndex;
         uiPESStreamID = pstStartSettings->audio[uiIndex].uiPESStreamID;
#if 0
         bEnablePESPacking = pstStartSettings->audio[uiIndex].bEnablePESPacking;
#endif
         uiMemoryBlockIndex = BMUXlib_TS_MemoryBlock_Type_eAudioIndex + uiIndex;
         break;

      default:
         return BERR_TRACE( BERR_UNKNOWN );
         break;
      }

      pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].stCommon.uiPIDChannelIndex = uiPIDChannelIndex;
      pstAspStartSettings->stSource[pstAspStartSettings->uiNumValidSource].uiPESStreamID = uiPESStreamID;

      /* Setup Memory Lookup Table*/
      hMuxTS->stMemoryBlock[uiMemoryBlockIndex].hBlock = pstBufferStatus->hIndexBufferBlock;
      hMuxTS->stMemoryBlock[uiMemoryBlockIndex].uiOffset = BMMA_LockOffset( pstBufferStatus->hIndexBufferBlock );
      hMuxTS->stMemoryBlock[uiMemoryBlockIndex].pBuffer = BMMA_Lock( pstBufferStatus->hIndexBufferBlock );
   }

   pstAspStartSettings->uiNumValidSource++;

   return BERR_SUCCESS;
}

BERR_Code
BMUXlib_TS_ASP_SW_Start(
            BMUXlib_TS_ASP_Handle hMuxTS
            )
{
   BMUXlib_ASP_TS_P_Channel_Start( &hMuxTS->stAspChannelContext );

   return BERR_SUCCESS;
}

BERR_Code
BMUXlib_TS_ASP_SW_DoMux(
            BMUXlib_TS_ASP_Handle hMuxTS
            )
{
   BMUXlib_ASP_TS_P_Channel_DoMux( &hMuxTS->stAspChannelContext );

   return BERR_SUCCESS;
}

BERR_Code
BMUXlib_TS_ASP_SW_GetStatus(
         BMUXlib_TS_ASP_Handle hMuxTS,
         BMUXlib_TS_ASP_Status *pstStatus
         )
{
   pstStatus->eState = hMuxTS->stAspChannelContext.eState;
   return BERR_SUCCESS;
}

const BMUXlib_TS_ASP_Interface BMUXlib_TS_ASP_SW_Interface =
{
      (BMUXlib_TS_ASP_Method_SetSettings) BMUXlib_TS_ASP_SW_SetSettings,
      (BMUXlib_TS_ASP_Method_SetAVSourceSettings) BMUXlib_TS_ASP_SW_SetAVSourceSettings,
      (BMUXlib_TS_ASP_Method_Start) BMUXlib_TS_ASP_SW_Start,
      NULL,
      (BMUXlib_TS_ASP_Method_DoMux) BMUXlib_TS_ASP_SW_DoMux,
      (BMUXlib_TS_ASP_Method_GetStatus) BMUXlib_TS_ASP_SW_GetStatus,
};
