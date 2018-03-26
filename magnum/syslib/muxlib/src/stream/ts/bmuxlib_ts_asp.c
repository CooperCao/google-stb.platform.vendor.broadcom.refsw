/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */

#include "bmuxlib_ts.h"
#include "bmuxlib_ts_asp_priv.h"
#include "bmuxlib_ts_asp_userdata.h"
#include "bmuxlib_ts_asp_sw.h"

BDBG_MODULE(BMUXlib_TS_ASP);
BDBG_OBJECT_ID(BMUXlib_TS_ASP_P_Context);

void
BMUXlib_TS_ASP_GetDefaultCreateSettings(
         BMUXlib_TS_CreateSettings *pCreateSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_ASP_GetDefaultCreateSettings );

   BDBG_ASSERT( pCreateSettings );

   BKNI_Memset( pCreateSettings, 0, sizeof(*pCreateSettings) );

   pCreateSettings->uiSignature = BMUXLIB_TS_P_SIGNATURE_CREATESETTINGS;

   {
      BMUXlib_TS_MuxConfig stMuxConfig;

      BMUXlib_TS_GetDefaultStartSettings( &stMuxConfig.stMuxStartSettings );
      BMUXlib_TS_GetDefaultMuxSettings( &stMuxConfig.stMuxSettings );
      BMUXlib_TS_ASP_GetMemoryConfig( &stMuxConfig, &pCreateSettings->stMemoryConfig );
   }

   BDBG_LEAVE( BMUXlib_TS_ASP_GetDefaultCreateSettings );
}

BERR_Code
BMUXlib_TS_ASP_Create(
         BMUXlib_TS_ASP_Handle *phMuxTS,  /* [out] TSMuxer handle returned */
         const BMUXlib_TS_CreateSettings *pstCreateSettings
         )
{
   BMUXlib_TS_ASP_Handle hMuxTS;

   BDBG_ENTER( BMUXlib_TS_ASP_Create );

   BDBG_ASSERT( phMuxTS );
   BDBG_ASSERT( pstCreateSettings );
   BDBG_ASSERT( BMUXLIB_TS_P_SIGNATURE_CREATESETTINGS == pstCreateSettings->uiSignature );

   /************************/
   /* Create MUX TS Handle */
   /************************/

   /* Set the handle to NULL in case the allocation fails */
   *phMuxTS = NULL;

   if (/* verify the required hMem/hReg handle is provided ... */
      (NULL == pstCreateSettings->hMma)
      || (NULL == pstCreateSettings->hReg)
      )
   {
      BDBG_LEAVE(BMUXlib_TS_ASP_Create);
      return BERR_TRACE( BERR_INVALID_PARAMETER );
   }

   /* Allocate MUX TS Context (system memory) */
   hMuxTS = ( BMUXlib_TS_ASP_P_Context* ) BKNI_Malloc( sizeof( BMUXlib_TS_ASP_P_Context ) );
   if ( NULL == hMuxTS )
   {
      BDBG_LEAVE(BMUXlib_TS_ASP_Create);
      return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
   }

   /* Zero out the newly allocated context */
   BKNI_Memset( hMuxTS, 0, sizeof( *hMuxTS ) );

   BDBG_OBJECT_SET(hMuxTS, BMUXlib_TS_ASP_P_Context);

   hMuxTS->stCreateSettings = *pstCreateSettings;

   /* Allocate TS Packet Buffer */
   if ( NULL == pstCreateSettings->stMemoryBuffers.hSharedBufferBlock )
   {
      hMuxTS->hTSPacketBufferBlock = BMMA_Alloc( pstCreateSettings->hMma, pstCreateSettings->stMemoryConfig.uiSharedBufferSize, 1 << 4, NULL );
      if ( NULL == hMuxTS->hTSPacketBufferBlock )
      {
         BMUXlib_TS_ASP_Destroy( hMuxTS );
         BDBG_LEAVE(BMUXlib_TS_ASP_Create);
         return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
      }
   }
   else
   {
      hMuxTS->hTSPacketBufferBlock = pstCreateSettings->stMemoryBuffers.hSharedBufferBlock;
   }

   /* Allocate User Data Queue */
   hMuxTS->stUserData.hBlock = BMMA_Alloc( pstCreateSettings->hMma,sizeof( BMUXlib_ASP_TS_Userdata_Host_Interface_t ), 1 << 4, NULL );
   if ( NULL == hMuxTS->stUserData.hBlock )
   {
      BMUXlib_TS_ASP_Destroy( hMuxTS );
      BDBG_LEAVE(BMUXlib_TS_ASP_Create);
      return BERR_TRACE( BERR_OUT_OF_DEVICE_MEMORY );
   }

   /* Set default mux settings */
   BMUXlib_TS_GetDefaultMuxSettings( &hMuxTS->stMuxSettings );
   BMUXlib_TS_GetDefaultFinishSettings( &hMuxTS->stFinishSettings );

   {
      hMuxTS->stASPInterface = BMUXlib_TS_ASP_SW_Interface;
   }

   /* Provide handle to caller */
   *phMuxTS = hMuxTS;
   BDBG_LEAVE(BMUXlib_TS_ASP_Create);
   return BERR_TRACE( BERR_SUCCESS );
}

/* BMUXlib_TS_ASP_Destroy - Frees all system/device memory allocated */
BERR_Code
BMUXlib_TS_ASP_Destroy(
         BMUXlib_TS_ASP_Handle hMuxTS
         )
{
   BDBG_ENTER(BMUXlib_TS_ASP_Destroy);

   /* the following signifies an attempt to free up something that was either
      a) not created by Create()
      b) has already been destroyed
   */
   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_ASP_P_Context);

   /* SW7425-3642: Stop the mux if it hasn't already been stopped
      - this is necessary since Stop() now frees resources */
   if (BMUXlib_State_eStopped != BMUXLIB_TS_ASP_P_GET_MUX_STATE(hMuxTS))
   {
      BERR_Code rc = BMUXlib_TS_ASP_Stop(hMuxTS);
      if (rc != BERR_SUCCESS)
      {
         BDBG_LEAVE(BMUXlib_TS_ASP_Destroy);
         return BERR_TRACE(rc);
      }
   }

   /* Free Allocated Buffers */
   if ( NULL != hMuxTS->stUserData.hBlock )
   {
      BMMA_Free( hMuxTS->stUserData.hBlock );
      hMuxTS->stUserData.hBlock = NULL;
   }

   /* Free Allocated Buffers */
   if ( ( NULL != hMuxTS->hTSPacketBufferBlock )
        && ( NULL == hMuxTS->stCreateSettings.stMemoryBuffers.hSharedBufferBlock ) )
   {
      BMMA_Free( hMuxTS->hTSPacketBufferBlock );
      hMuxTS->hTSPacketBufferBlock = NULL;
   }

   /* the following prevents accidental reuse of the context */
   BDBG_OBJECT_DESTROY(hMuxTS, BMUXlib_TS_ASP_P_Context);
   BKNI_Free ( hMuxTS );

   BDBG_LEAVE(BMUXlib_TS_ASP_Destroy);
   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BMUXlib_TS_ASP_SetMuxSettings(
         BMUXlib_TS_ASP_Handle hMuxTS,
         const BMUXlib_TS_MuxSettings *pstMuxSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_ASP_SetMuxSettings );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_ASP_P_Context);
   BDBG_ASSERT( pstMuxSettings );
   BDBG_ASSERT( BMUXLIB_TS_P_SIGNATURE_MUXSETTINGS == pstMuxSettings->uiSignature );

   hMuxTS->stMuxSettings = *pstMuxSettings;

   /* TODO: Support dymanic MUX settings */
   BDBG_WRN(("BMUXlib_TS_ASP_SetMuxSettings not supported, yet"));

   BDBG_LEAVE( BMUXlib_TS_ASP_SetMuxSettings );
   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
BMUXlib_TS_ASP_GetMuxSettings(
         BMUXlib_TS_ASP_Handle hMuxTS,
         BMUXlib_TS_MuxSettings *pstMuxSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_ASP_GetMuxSettings );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_ASP_P_Context);
   BDBG_ASSERT( pstMuxSettings );

   *pstMuxSettings = hMuxTS->stMuxSettings;

   BDBG_LEAVE( BMUXlib_TS_ASP_GetMuxSettings );
   return BERR_TRACE( BERR_SUCCESS );
}

static
void
BMUXlib_S_TS_ASP_LockBuffers(
      BMUXlib_TS_ASP_Handle hMuxTS
      )
{
   /* Memory Interface */
   hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eTSBuffer].hBlock = hMuxTS->hTSPacketBufferBlock;
   hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eTSBuffer].uiOffset = BMMA_LockOffset( hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eTSBuffer].hBlock );
   hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eTSBuffer].pBuffer = BMMA_Lock( hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eTSBuffer].hBlock );

   /* Userdata Interface */
   hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eUserdataInterface].hBlock = hMuxTS->stUserData.hBlock;
   hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eUserdataInterface].uiOffset = BMMA_LockOffset( hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eUserdataInterface].hBlock );
   hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eUserdataInterface].pBuffer = BMMA_Lock( hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eUserdataInterface].hBlock );
}

static
void
BMUXlib_S_TS_ASP_UnlockBuffers(
      BMUXlib_TS_ASP_Handle hMuxTS
      )
{
   unsigned i;

   for ( i = 0; i < BMUXlib_TS_MemoryBlock_Type_eMax; i++ )
   {
      if ( NULL != hMuxTS->stMemoryBlock[i].hBlock )
      {
         BMMA_Unlock( hMuxTS->stMemoryBlock[i].hBlock, hMuxTS->stMemoryBlock[i].pBuffer );
         BMMA_UnlockOffset( hMuxTS->stMemoryBlock[i].hBlock, hMuxTS->stMemoryBlock[i].uiOffset );
         hMuxTS->stMemoryBlock[i].uiOffset = 0;
         hMuxTS->stMemoryBlock[i].pBuffer = NULL;
      }
   }
}

/* BMUXlib_TS_ASP_Start - Configures the mux HW */
BERR_Code
BMUXlib_TS_ASP_Start(
         BMUXlib_TS_ASP_Handle hMuxTS,
         const BMUXlib_TS_StartSettings *pstStartSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;

   BDBG_ENTER( BMUXlib_TS_ASP_Start );

   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_ASP_P_Context);
   BDBG_ASSERT( pstStartSettings );
   BDBG_ASSERT( BMUXLIB_TS_P_SIGNATURE_STARTSETTINGS == pstStartSettings->uiSignature );

   hMuxTS->stStartSettings = *pstStartSettings;

   BMUXlib_S_TS_ASP_LockBuffers( hMuxTS );
   hMuxTS->stUserData.pstQueue = (BMUXlib_ASP_TS_Userdata_Host_Interface_t*) hMuxTS->stMemoryBlock[BMUXlib_TS_MemoryBlock_Type_eUserdataInterface].pBuffer;
   BKNI_Memset( hMuxTS->stUserData.pstQueue, 0, sizeof( *hMuxTS->stUserData.pstQueue ) );

   if ( NULL != hMuxTS->stASPInterface.setSettings ) rc = hMuxTS->stASPInterface.setSettings( hMuxTS );

   if ( BERR_SUCCESS != rc ) goto start_error;
   hMuxTS->bAllInputsReady = false;
   BMUXLIB_TS_ASP_P_SET_MUX_STATE( hMuxTS, BMUXlib_State_eStarted );
   goto start_success;

start_error:
   BMUXlib_TS_ASP_Stop( hMuxTS );

start_success:
   BDBG_LEAVE( BMUXlib_TS_ASP_Start );
   return BERR_TRACE( rc );

}

BERR_Code
BMUXlib_TS_ASP_Finish(
         BMUXlib_TS_ASP_Handle hMuxTS,
         const BMUXlib_TS_FinishSettings *pstFinishSettings
         )
{
   BERR_Code rc = BERR_SUCCESS;
   BDBG_ENTER( BMUXlib_TS_ASP_Finish );
   BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_ASP_P_Context);

   hMuxTS->stFinishSettings = *pstFinishSettings;
   /* TODO: Handle Finish Properly */
   BMUXLIB_TS_ASP_P_SET_MUX_STATE( hMuxTS, BMUXlib_State_eFinishingInput );

   BDBG_LEAVE( BMUXlib_TS_ASP_Finish );
   return rc;
}

BERR_Code
BMUXlib_TS_ASP_Stop(
         BMUXlib_TS_ASP_Handle hMuxTS
         )
{
   if ( NULL != hMuxTS->stASPInterface.stop ) hMuxTS->stASPInterface.stop( hMuxTS );

   hMuxTS->stUserData.pstQueue = NULL;
   BMUXlib_S_TS_ASP_UnlockBuffers( hMuxTS );

   BMUXLIB_ASP_TS_P_CHANNEL_SET_STATE( &hMuxTS->stAspChannelContext, BMUXLIB_ASP_CHANNEL_STATE_STOPPED );

   BMUXLIB_TS_ASP_P_SET_MUX_STATE( hMuxTS, BMUXlib_State_eStopped );

   return BERR_SUCCESS;
}

void
BMUXlib_TS_ASP_GetMemoryConfig(
         const BMUXlib_TS_MuxConfig *pstMuxConfig,
         BMUXlib_TS_MemoryConfig *pstMemoryConfig
         )
{
   BDBG_ENTER( BMUXlib_TS_ASP_GetMemoryConfig );

   BDBG_ASSERT(pstMuxConfig);
   BDBG_ASSERT(pstMemoryConfig);

   BKNI_Memset( pstMemoryConfig, 0, sizeof( *pstMemoryConfig )  );

   pstMemoryConfig->uiSharedBufferSize = BMUXLIB_ASP_TS_MAX_TS_PACKETS_PER_INSTANCE * 188;

   BDBG_LEAVE( BMUXlib_TS_ASP_GetMemoryConfig );
}

BERR_Code
BMUXlib_TS_ASP_AddSystemDataBuffers(
         BMUXlib_TS_ASP_Handle hMuxTS,
         const BMUXlib_TS_SystemData *astSystemDataBuffer, /* Array of system data buffers */
         size_t uiCount, /* Count of system data buffers in array */
         size_t *puiQueuedCount /* Count of system data buffers queued by muxer (*puiQueuedCount <= uiCount) */
         )
{
   unsigned i;
   *puiQueuedCount = 0;

   /* We just queue the system data buffers to an internal queue.  We will process and queue these to the ASP MUXlib in the DoMux() call */
   for ( i = 0; i < uiCount; i++ )
   {
      unsigned uiNextWriteOffset = ( hMuxTS->stSystemData.stInputQueue.uiWriteOffset + 1 ) % BMUXLIB_TS_MAX_SYSTEM_DATA_COUNT;
      if ( uiNextWriteOffset == hMuxTS->stSystemData.stInputQueue.uiReadOffset ) break;

      hMuxTS->stSystemData.stInputQueue.astEntry[hMuxTS->stSystemData.stInputQueue.uiWriteOffset].stData = astSystemDataBuffer[i];

      hMuxTS->stSystemData.stInputQueue.uiWriteOffset = uiNextWriteOffset;
      (*puiQueuedCount)++;
   }

   return BERR_SUCCESS;
}

BERR_Code
BMUXlib_TS_ASP_GetCompletedSystemDataBuffers(
         BMUXlib_TS_ASP_Handle hMuxTS,
         size_t *puiCompletedCount /* Returns count of system data buffers fully muxed */
         )
{
   *puiCompletedCount = hMuxTS->stSystemData.uiCompletedCount;
   hMuxTS->stSystemData.uiCompletedCount = 0;
   return BERR_SUCCESS;
}

void
BMUXlib_TS_ASP_GetStatus(
   BMUXlib_TS_ASP_Handle hMuxTS,
   BMUXlib_TS_Status *pstStatus
   )
{
   BKNI_Memset( pstStatus, 0, sizeof( *pstStatus ) );
   /* TODO: Populate the status struct */
   BSTD_UNUSED( hMuxTS );
}

BERR_Code
BMUXlib_TS_ASP_DoMux(
   BMUXlib_TS_ASP_Handle hMuxTS,
   BMUXlib_DoMux_Status *pstStatus
   )
{
   switch ( BMUXLIB_TS_ASP_P_GET_MUX_STATE( hMuxTS ) )
   {
      case BMUXlib_State_eStopped:
         break;

      case BMUXlib_State_eStarted:
         {
            if ( false == hMuxTS->bAllInputsReady )
            {
               /* Check to make sure all inputs have data */
               bool bNotReady = false;

               /* Setup A/V Source(s) */
               {
                  BMUXlib_TS_StartSettings *pstStartSettings = &hMuxTS->stStartSettings;
                  unsigned i;
                  BMUXlib_CompressedBufferStatus stBufferStatus;

                  for ( i = 0; i < pstStartSettings->uiNumValidVideoPIDs; i++ )
                  {
                     if ( false == hMuxTS->stInputReady.bVideo[i] )
                     {
                        /* Setup the video source */
                        void *pContext = pstStartSettings->video[i].stInputInterface.pContext;

                        pstStartSettings->video[i].stInputInterface.fGetBufferStatus( pContext, &stBufferStatus );

                        if ( true == stBufferStatus.stContext.bReady )
                        {
                           if ( NULL != hMuxTS->stASPInterface.setAVsourceSettings ) hMuxTS->stASPInterface.setAVsourceSettings( hMuxTS, &stBufferStatus, BMUXLIB_ASP_SOURCE_AV_TYPE_VICE, i );

                           hMuxTS->stInputReady.bVideo[i] = true;
                        }
                        else
                        {
                           BDBG_MSG(("Video[%d] not ready...", i));
                           bNotReady = true;
                        }
                     }
                  }

                  for ( i = 0; i < pstStartSettings->uiNumValidAudioPIDs; i++ )
                  {
                     if ( false == hMuxTS->stInputReady.bAudio[i] )
                     {
                        /* Setup the audio source */
                        void *pContext = pstStartSettings->audio[i].stInputInterface.pContext;

                        pstStartSettings->audio[i].stInputInterface.fGetBufferStatus( pContext, &stBufferStatus );

                        if ( true == stBufferStatus.stContext.bReady )
                        {
                           if ( NULL != hMuxTS->stASPInterface.setAVsourceSettings ) hMuxTS->stASPInterface.setAVsourceSettings( hMuxTS, &stBufferStatus, BMUXLIB_ASP_SOURCE_AV_TYPE_RAAGA, i );

                           hMuxTS->stInputReady.bAudio[i] = true;
                        }
                        else
                        {
                           BDBG_MSG(("Audio[%d] not ready...", i));
                           bNotReady = true;
                        }
                     }
                  }
               }
               hMuxTS->bAllInputsReady = ( false == bNotReady );

               if ( true == hMuxTS->bAllInputsReady )
               {
                  if ( NULL != hMuxTS->stASPInterface.start ) hMuxTS->stASPInterface.start( hMuxTS );
               }
            }

            if ( true == hMuxTS->bAllInputsReady )
            {
               BMUXlib_TS_ASP_P_ProcessNewUserData( hMuxTS );
               if ( NULL != hMuxTS->stASPInterface.doMux ) hMuxTS->stASPInterface.doMux( hMuxTS );
               BMUXlib_TS_ASP_P_ProcessCompletedUserData( hMuxTS );

               if ( NULL != hMuxTS->stASPInterface.getStatus )
               {
                  BMUXlib_TS_ASP_Status stStatus;
                  BKNI_Memset( &stStatus, 0, sizeof( stStatus ) );
                  hMuxTS->stASPInterface.getStatus( hMuxTS, &stStatus );

                  switch ( stStatus.eState )
                  {
                  case BMUXLIB_ASP_CHANNEL_STATE_FINISHING:
                     BMUXLIB_TS_ASP_P_SET_MUX_STATE( hMuxTS, BMUXlib_State_eFinishingInput );
                     break;

                  case BMUXLIB_ASP_CHANNEL_STATE_FINISHED:
                     BMUXLIB_TS_ASP_P_SET_MUX_STATE( hMuxTS, BMUXlib_State_eFinished );
                     break;

                  default:
                     break;
                  }
               }
            }
         }
         break;

      case BMUXlib_State_eFinishingInput:
      case BMUXlib_State_eFinishingOutput:
         BMUXlib_TS_ASP_P_ProcessNewUserData( hMuxTS );
         if ( NULL != hMuxTS->stASPInterface.doMux ) hMuxTS->stASPInterface.doMux( hMuxTS );
         BMUXlib_TS_ASP_P_ProcessCompletedUserData( hMuxTS );

         if ( NULL != hMuxTS->stASPInterface.getStatus )
         {
            BMUXlib_TS_ASP_Status stStatus;
            BKNI_Memset( &stStatus, 0, sizeof( stStatus ) );
            hMuxTS->stASPInterface.getStatus( hMuxTS, &stStatus );

            switch ( stStatus.eState )
            {
            case BMUXLIB_ASP_CHANNEL_STATE_FINISHED:
               BMUXLIB_TS_ASP_P_SET_MUX_STATE( hMuxTS, BMUXlib_State_eFinished );
               break;

            default:
               break;
            }
         }
         break;

      case BMUXlib_State_eFinished:
         break;

      default:
         break;
   }

   if ( NULL != pstStatus )
   {
      BKNI_Memset( pstStatus, 0, sizeof( *pstStatus ) );
      pstStatus->eState = hMuxTS->eState;
      pstStatus->uiNextExecutionTime = hMuxTS->stStartSettings.uiServicePeriod;
   }

   return BERR_SUCCESS;
}
