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
 ******************************************************************************/

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */

#include "bmuxlib_ts.h"
#include "bmuxlib_ts_consts.h"
#include "bmuxlib_ts_priv.h"
#if BMUXLIB_TS_P_USE_MUX_V2
#include "bmuxlib_ts_asp_priv.h"
#endif

BDBG_MODULE(BMUXLIB_TS_PROXY);
BDBG_OBJECT_ID(BMUXlib_TS_P_Context);

typedef enum BMUXlib_TS_P_Type
{
   BMUXlib_TS_P_Type_eUnknown = 0,
   BMUXlib_TS_P_Type_eLegacy,
#if BMUXLIB_TS_P_USE_MUX_V2
   BMUXlib_TS_P_Type_eASP,
#endif
   BMUXlib_TS_P_Type_eMax
} BMUXlib_TS_P_Type;

#define BMUXLIB_TS_P_SYSTEM_DATA_PRE_Q_COUNT 5

typedef struct BMUXlib_TS_P_Context
{
   BDBG_OBJECT(BMUXlib_TS_P_Context)

   BMUXlib_TS_P_Type eType;
   BMUXlib_TS_MuxSettings stCurrentMuxSettings;

   BMUXlib_TS_SystemData astSystemData[BMUXLIB_TS_P_SYSTEM_DATA_PRE_Q_COUNT];
   unsigned uiSystemDataCount;

   BMUXlib_TS_Legacy_Handle hLegacyMux;
#if BMUXLIB_TS_P_USE_MUX_V2
   BMUXlib_TS_ASP_Handle hASPMux;
#endif
} BMUXlib_TS_P_Context;

void
BMUXlib_TS_GetDefaultCreateSettings(
         BMUXlib_TS_CreateSettings *pCreateSettings
         )
{
   BMUXlib_TS_CreateSettings stCreateSettings;

   BKNI_Memset( &stCreateSettings, 0, sizeof( stCreateSettings ) );
   BKNI_Memset( pCreateSettings, 0, sizeof( *pCreateSettings ) );
   BMUXlib_TS_Legacy_GetDefaultCreateSettings( pCreateSettings );
#if BMUXLIB_TS_P_USE_MUX_V2
   BMUXlib_TS_ASP_GetDefaultCreateSettings( &stCreateSettings );
#endif

   if ( pCreateSettings->stMemoryConfig.uiSharedBufferSize < stCreateSettings.stMemoryConfig.uiSharedBufferSize )
   {
      pCreateSettings->stMemoryConfig.uiSharedBufferSize = stCreateSettings.stMemoryConfig.uiSharedBufferSize;
   }

   if ( pCreateSettings->stMemoryConfig.uiSystemBufferSize < stCreateSettings.stMemoryConfig.uiSystemBufferSize )
   {
      pCreateSettings->stMemoryConfig.uiSystemBufferSize = stCreateSettings.stMemoryConfig.uiSystemBufferSize;
   }
}

BERR_Code
BMUXlib_TS_Create(
         BMUXlib_TS_Handle *phMuxTS,  /* [out] TSMuxer handle returned */
         const BMUXlib_TS_CreateSettings *pstCreateSettings
         )
{
   BMUXlib_TS_Handle hMuxTS = NULL;

   BERR_Code rc = BERR_UNKNOWN;

   BDBG_ENTER( BMUXlib_TS_Create );

   BDBG_ASSERT( phMuxTS );
   BDBG_ASSERT( pstCreateSettings );
   BDBG_ASSERT( BMUXLIB_TS_P_SIGNATURE_CREATESETTINGS == pstCreateSettings->uiSignature );

   /************************/
   /* Create MUX TS Handle */
   /************************/

   /* Set the handle to NULL in case the allocation fails */
   *phMuxTS = NULL;

   /* Allocate MUX TS Context (system memory) */
   hMuxTS = ( BMUXlib_TS_P_Context* ) BKNI_Malloc( sizeof( BMUXlib_TS_P_Context ) );
   if ( NULL == hMuxTS )
   {
      rc = BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY );
      goto BMUXLIB_TS_CREATE_ERROR;
   }

   /* Zero out the newly allocated context */
   BKNI_Memset( hMuxTS, 0, sizeof( *hMuxTS ) );

   BDBG_OBJECT_SET(hMuxTS, BMUXlib_TS_P_Context);

   rc = BMUXlib_TS_Legacy_Create( &hMuxTS->hLegacyMux, pstCreateSettings );
   if ( BERR_SUCCESS != rc ) goto BMUXLIB_TS_CREATE_ERROR;

#if BMUXLIB_TS_P_USE_MUX_V2
   rc = BMUXlib_TS_ASP_Create( &hMuxTS->hASPMux, pstCreateSettings );
   if ( BERR_SUCCESS != rc ) goto BMUXLIB_TS_CREATE_ERROR;
#endif

   BMUXlib_TS_GetDefaultMuxSettings( &hMuxTS->stCurrentMuxSettings );

   *phMuxTS = hMuxTS;
   goto BMUXLIB_TS_CREATE_SUCCESS;

BMUXLIB_TS_CREATE_ERROR:
   BMUXlib_TS_Destroy( hMuxTS );

BMUXLIB_TS_CREATE_SUCCESS:
   BDBG_LEAVE(BMUXlib_TS_Create);
   return BERR_TRACE( rc );
}

void
BMUXlib_TS_Destroy(
         BMUXlib_TS_Handle hMuxTS
         )
{
   if ( NULL != hMuxTS )
   {
      BDBG_OBJECT_ASSERT(hMuxTS, BMUXlib_TS_P_Context);

      if ( NULL != hMuxTS->hLegacyMux )
      {
         BMUXlib_TS_Legacy_Destroy( hMuxTS->hLegacyMux );
         hMuxTS->hLegacyMux = NULL;
      }

#if BMUXLIB_TS_P_USE_MUX_V2
      if ( NULL != hMuxTS->hASPMux )
      {
         BMUXlib_TS_ASP_Destroy( hMuxTS->hASPMux );
         hMuxTS->hASPMux = NULL;
      }
#endif

      BDBG_OBJECT_DESTROY(hMuxTS, BMUXlib_TS_P_Context);
      BKNI_Free( hMuxTS );
   }

   return;
}

/****************/
/* Mux Settings */
/****************/
void
BMUXlib_TS_GetDefaultMuxSettings(
         BMUXlib_TS_MuxSettings *pstMuxSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_GetDefaultMuxSettings );

   BDBG_ASSERT( pstMuxSettings );

   BKNI_Memset( pstMuxSettings, 0, sizeof( *pstMuxSettings ) );

   pstMuxSettings->uiSignature = BMUXLIB_TS_P_SIGNATURE_MUXSETTINGS;
   pstMuxSettings->uiSystemDataBitRate = BMUXLIB_TS_P_MUX_SYS_DATA_BR_DEFAULT;

   /* SW7436-1363: Enable all inputs by default */
   {
      unsigned i;

      for ( i = 0; i < BMUXLIB_TS_MAX_VIDEO_PIDS; i++ )
      {
         pstMuxSettings->stInputEnable.bVideo[i] = true;
      }

      for ( i = 0; i < BMUXLIB_TS_MAX_AUDIO_PIDS; i++ )
      {
         pstMuxSettings->stInputEnable.bAudio[i] = true;
      }
   }

   BDBG_LEAVE( BMUXlib_TS_GetDefaultMuxSettings );
   return;
}

BERR_Code
BMUXlib_TS_SetMuxSettings(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_MuxSettings *pstMuxSettings
         )
{
   BERR_Code rc = BERR_UNKNOWN;
   switch ( hMuxTS->eType )
   {
      case BMUXlib_TS_P_Type_eLegacy:
         rc = BMUXlib_TS_Legacy_SetMuxSettings( hMuxTS->hLegacyMux, pstMuxSettings );
         break;

#if BMUXLIB_TS_P_USE_MUX_V2
      case BMUXlib_TS_P_Type_eASP:
         rc = BMUXlib_TS_ASP_SetMuxSettings( hMuxTS->hASPMux, pstMuxSettings );
         break;
#endif

      case BMUXlib_TS_P_Type_eUnknown:
      default:
         rc = BERR_SUCCESS;
         break;
   }

   if ( BERR_SUCCESS == rc )
   {
      hMuxTS->stCurrentMuxSettings = *pstMuxSettings;
   }

   return BERR_TRACE( rc );
}

BERR_Code
BMUXlib_TS_GetMuxSettings(
         BMUXlib_TS_Handle hMuxTS,
         BMUXlib_TS_MuxSettings *pstMuxSettings
         )
{
   BERR_Code rc = BERR_UNKNOWN;

   switch ( hMuxTS->eType )
   {
      case BMUXlib_TS_P_Type_eLegacy:
         rc = BMUXlib_TS_Legacy_GetMuxSettings( hMuxTS->hLegacyMux, pstMuxSettings );
         break;

#if BMUXLIB_TS_P_USE_MUX_V2
      case BMUXlib_TS_P_Type_eASP:
         rc = BMUXlib_TS_ASP_GetMuxSettings( hMuxTS->hASPMux, pstMuxSettings );
         break;
#endif

      case BMUXlib_TS_P_Type_eUnknown:
      default:
         *pstMuxSettings = hMuxTS->stCurrentMuxSettings;
         rc = BERR_SUCCESS;
         break;
   }

   if ( BERR_SUCCESS == rc )
   {
      hMuxTS->stCurrentMuxSettings = *pstMuxSettings;
   }

   return BERR_TRACE( rc );
}

/**************/
/* Start/Stop */
/**************/
void
BMUXlib_TS_GetDefaultStartSettings(
         BMUXlib_TS_StartSettings *pstStartSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_GetDefaultStartSettings );

   BDBG_ASSERT( pstStartSettings );

   BKNI_Memset( pstStartSettings, 0, sizeof( *pstStartSettings ) );

   pstStartSettings->uiSignature = BMUXLIB_TS_P_SIGNATURE_STARTSETTINGS;
   pstStartSettings->uiServiceLatencyTolerance = 20;
   pstStartSettings->uiServicePeriod = BMUXLIB_TS_P_MUX_SERVICE_PERIOD_DEFAULT;
   pstStartSettings->stPCRData.uiInterval = BMUXLIB_TS_P_MUX_PCR_INTERVAL_DEFAULT;
   pstStartSettings->uiA2PDelay = BMUXLIB_TS_P_A2PDELAY_DEFAULT;

   BDBG_LEAVE( BMUXlib_TS_GetDefaultStartSettings );
   return;
}

BERR_Code
BMUXlib_TS_Start(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_StartSettings *pstStartSettings
         )
{
   BERR_Code rc = BERR_UNKNOWN;

#if BMUXLIB_TS_P_USE_MUX_V2
   /* See if MCPB is in use */
   {
      unsigned i;
      unsigned uiSystemTransportChannel = pstStartSettings->stPCRData.uiTransportChannelIndex;

      hMuxTS->eType = BMUXlib_TS_P_Type_eASP;

      for ( i = 0; i < pstStartSettings->uiNumValidVideoPIDs; i++ )
      {
         if ( uiSystemTransportChannel != pstStartSettings->video[i].uiTransportChannelIndex )
         {
            hMuxTS->eType = BMUXlib_TS_P_Type_eLegacy;
            break;
         }
      }

      for ( i = 0; i < pstStartSettings->uiNumValidAudioPIDs; i++ )
      {
         if ( uiSystemTransportChannel != pstStartSettings->audio[i].uiTransportChannelIndex )
         {
            hMuxTS->eType = BMUXlib_TS_P_Type_eLegacy;
            break;
         }
      }
   }
#else
   hMuxTS->eType = BMUXlib_TS_P_Type_eLegacy;
#endif

   rc = BMUXlib_TS_SetMuxSettings( hMuxTS, &hMuxTS->stCurrentMuxSettings );
   if ( BERR_SUCCESS == rc )
   {
      if ( 0 != hMuxTS->uiSystemDataCount )
      {
         size_t uiQueuedCount = 0;

         BMUXlib_TS_AddSystemDataBuffers( hMuxTS, hMuxTS->astSystemData, hMuxTS->uiSystemDataCount, &uiQueuedCount );
         if ( uiQueuedCount != hMuxTS->uiSystemDataCount )
         {
            BDBG_WRN(("%u Pre-Queued System Data Buffers Dropped", (unsigned)(hMuxTS->uiSystemDataCount - uiQueuedCount)));
         }
         hMuxTS->uiSystemDataCount = 0;
      }
      switch ( hMuxTS->eType )
      {
         case BMUXlib_TS_P_Type_eLegacy:
            rc = BMUXlib_TS_Legacy_Start( hMuxTS->hLegacyMux, pstStartSettings );
            break;

#if BMUXLIB_TS_P_USE_MUX_V2
         case BMUXlib_TS_P_Type_eASP:
            rc = BMUXlib_TS_ASP_Start( hMuxTS->hASPMux, pstStartSettings );
            break;
#endif

         case BMUXlib_TS_P_Type_eUnknown:
         default:
            rc = BERR_TRACE( BERR_INVALID_PARAMETER );
      }
   }

   if ( BERR_SUCCESS != rc )
   {
      hMuxTS->eType = BMUXlib_TS_P_Type_eUnknown;
   }

   return BERR_TRACE( rc );
}

void
BMUXlib_TS_GetDefaultFinishSettings(
         BMUXlib_TS_FinishSettings *pstFinishSettings
         )
{
   BDBG_ENTER( BMUXlib_TS_GetDefaultFinishSettings );

   BDBG_ASSERT( pstFinishSettings );

   BKNI_Memset( pstFinishSettings, 0, sizeof( *pstFinishSettings ) );

   pstFinishSettings->uiSignature = BMUXLIB_TS_P_SIGNATURE_FINISHSETTINGS;
   pstFinishSettings->eFinishMode = BMUXlib_FinishMode_ePrepareForStop;

   BDBG_LEAVE( BMUXlib_TS_GetDefaultFinishSettings );
   return;
}

BERR_Code
BMUXlib_TS_Finish(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_FinishSettings *pstFinishSettings
         )
{
   switch ( hMuxTS->eType )
   {
      case BMUXlib_TS_P_Type_eLegacy:
         return BMUXlib_TS_Legacy_Finish( hMuxTS->hLegacyMux, pstFinishSettings );
         break;

#if BMUXLIB_TS_P_USE_MUX_V2
      case BMUXlib_TS_P_Type_eASP:
         return BMUXlib_TS_ASP_Finish( hMuxTS->hASPMux, pstFinishSettings );
         break;
#endif

      case BMUXlib_TS_P_Type_eUnknown:
      default:
         return BERR_TRACE( BERR_INVALID_PARAMETER );
   }
}

BERR_Code
BMUXlib_TS_Stop(
         BMUXlib_TS_Handle hMuxTS
         )
{
   switch ( hMuxTS->eType )
   {
      case BMUXlib_TS_P_Type_eLegacy:
         hMuxTS->eType = BMUXlib_TS_P_Type_eUnknown;
         return BMUXlib_TS_Legacy_Stop( hMuxTS->hLegacyMux );
         break;

#if BMUXLIB_TS_P_USE_MUX_V2
      case BMUXlib_TS_P_Type_eASP:
         hMuxTS->eType = BMUXlib_TS_P_Type_eUnknown;
         return BMUXlib_TS_ASP_Stop( hMuxTS->hASPMux );
         break;
#endif

      case BMUXlib_TS_P_Type_eUnknown:
      default:
         return BERR_TRACE( BERR_INVALID_PARAMETER );
   }
}

/**********/
/* Memory */
/**********/
void
BMUXlib_TS_GetMemoryConfig(
         const BMUXlib_TS_MuxConfig *pstMuxConfig,
         BMUXlib_TS_MemoryConfig *pstMemoryConfig
         )
{
   /* TODO: Return worst case of all MUXES */
   BMUXlib_TS_Legacy_GetMemoryConfig( pstMuxConfig, pstMemoryConfig );
}

/***************/
/* System Data */
/***************/
BERR_Code
BMUXlib_TS_AddSystemDataBuffers(
         BMUXlib_TS_Handle hMuxTS,
         const BMUXlib_TS_SystemData *astSystemDataBuffer, /* Array of system data buffers */
         size_t uiCount, /* Count of system data buffers in array */
         size_t *puiQueuedCount /* Count of system data buffers queued by muxer (*puiQueuedCount <= uiCount) */
         )
{
   *puiQueuedCount = 0;

   switch ( hMuxTS->eType )
   {
      case BMUXlib_TS_P_Type_eLegacy:
         return BMUXlib_TS_Legacy_AddSystemDataBuffers( hMuxTS->hLegacyMux, astSystemDataBuffer, uiCount, puiQueuedCount );
         break;

#if BMUXLIB_TS_P_USE_MUX_V2
      case BMUXlib_TS_P_Type_eASP:
         return BMUXlib_TS_ASP_AddSystemDataBuffers( hMuxTS->hASPMux, astSystemDataBuffer, uiCount, puiQueuedCount );
         break;
#endif

      case BMUXlib_TS_P_Type_eUnknown:
      default:
         while ( ( *puiQueuedCount != uiCount) && ( hMuxTS->uiSystemDataCount < BMUXLIB_TS_P_SYSTEM_DATA_PRE_Q_COUNT ) )
         {
            hMuxTS->astSystemData[hMuxTS->uiSystemDataCount] = astSystemDataBuffer[*puiQueuedCount];
            hMuxTS->uiSystemDataCount++;
            (*puiQueuedCount)++;
         }
         return BERR_TRACE( BERR_SUCCESS );
   }
}

BERR_Code
BMUXlib_TS_GetCompletedSystemDataBuffers(
         BMUXlib_TS_Handle hMuxTS,
         size_t *puiCompletedCount /* Returns count of system data buffers fully muxed */
         )
{
   switch ( hMuxTS->eType )
   {
      case BMUXlib_TS_P_Type_eLegacy:
         return BMUXlib_TS_Legacy_GetCompletedSystemDataBuffers( hMuxTS->hLegacyMux, puiCompletedCount );
         break;

#if BMUXLIB_TS_P_USE_MUX_V2
      case BMUXlib_TS_P_Type_eASP:
         return BMUXlib_TS_ASP_GetCompletedSystemDataBuffers( hMuxTS->hASPMux, puiCompletedCount );
         break;
#endif

      case BMUXlib_TS_P_Type_eUnknown:
      default:
         *puiCompletedCount = 0;
         return BERR_TRACE( BERR_SUCCESS );
   }
}

void
BMUXlib_TS_GetStatus(
   BMUXlib_TS_Handle hMuxTS,
   BMUXlib_TS_Status *pstStatus
   )
{
   switch ( hMuxTS->eType )
   {
   case BMUXlib_TS_P_Type_eLegacy:
      BMUXlib_TS_Legacy_GetStatus( hMuxTS->hLegacyMux, pstStatus );
      break;

#if BMUXLIB_TS_P_USE_MUX_V2
   case BMUXlib_TS_P_Type_eASP:
      BMUXlib_TS_ASP_GetStatus( hMuxTS->hASPMux, pstStatus );
      break;
#endif

   case BMUXlib_TS_P_Type_eUnknown:
   default:
      BKNI_Memset( pstStatus, 0, sizeof( *pstStatus ) );
      break;
   }
}

/***********/
/* Execute */
/***********/
BERR_Code
BMUXlib_TS_DoMux(
   BMUXlib_TS_Handle hMuxTS,
   BMUXlib_DoMux_Status *pstStatus
   )
{
   BERR_Code rc = BERR_UNKNOWN;

   switch ( hMuxTS->eType )
   {
      case BMUXlib_TS_P_Type_eLegacy:
         rc = BMUXlib_TS_Legacy_DoMux( hMuxTS->hLegacyMux, pstStatus );
         break;

#if BMUXLIB_TS_P_USE_MUX_V2
      case BMUXlib_TS_P_Type_eASP:
         rc = BMUXlib_TS_ASP_DoMux( hMuxTS->hASPMux, pstStatus );
         break;
#endif

      case BMUXlib_TS_P_Type_eUnknown:
      default:
         if ( NULL != pstStatus )
         {
            BKNI_Memset(pstStatus, 0, sizeof(*pstStatus));
            pstStatus->uiNextExecutionTime = BMUXLIB_TS_P_MUX_SERVICE_PERIOD_DEFAULT;
         }
         rc = BERR_SUCCESS;
         break;
   }

   return BERR_TRACE( rc );
}
