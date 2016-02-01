/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include <stdio.h>          /* for printf */
#include <stdlib.h>
#include <string.h>

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */
#include "bmem.h"           /* memory interface */

#include "bavc_xpt.h"

#include "bxpt.h"
#include "bxpt_playback.h"
#include "bxpt_rave.h"
#include "bxpt_tsmux.h"
#include "bxpt_pcr_offset.h"

#include "bmuxlib_ts.h"

#include "transport_stub.h"

BDBG_MODULE(TRANSPORT_STUB);

#define APP_ENCODER_DPCR 0

/**********/
/* Device */
/**********/
BERR_Code
app_OpenTransportDevice(
         TransportDeviceHandle *phDeviceTransport,
         const TransportDeviceSettings* pstSettings
         )
{
   TransportStubDeviceContext *pTransportDeviceContext = NULL;

   pTransportDeviceContext = ( TransportStubDeviceContext* ) BKNI_Malloc( sizeof( TransportStubDeviceContext ) );

   BKNI_Memset(
            pTransportDeviceContext,
            0,
            sizeof( TransportStubDeviceContext )
            );

   pTransportDeviceContext->stSettings = *pstSettings;

#ifndef BTST_P_USE_XPT_MUX_STUB
   {
      BXPT_DefaultSettings stXptSettings;

      BXPT_GetDefaultSettings(
               &stXptSettings,
               pstSettings->hChp
               );

      BXPT_Open(
               &pTransportDeviceContext->hXpt,
               pstSettings->hChp,
               pstSettings->hReg,
               pstSettings->hMem,
               pstSettings->hInt,
               &stXptSettings
               );

      {
         uint32_t uiInputBand;
         BXPT_InputBandConfig stInputBandConfig;

         for ( uiInputBand = 0; uiInputBand < BXPT_NUM_INPUT_BANDS; uiInputBand++ )
         {
            BXPT_GetInputBandConfig(
                     pTransportDeviceContext->hXpt,
                     uiInputBand,
                     &stInputBandConfig );

            stInputBandConfig.ForceValid = true;

            BXPT_SetInputBandConfig(
                     pTransportDeviceContext->hXpt,
                     uiInputBand,
                     &stInputBandConfig );
         }
      }

      /* Create PCROffset Handle */
      {
         BXPT_PcrOffset_Defaults Defaults;
         BXPT_PcrOffset_Settings Settings;

         BXPT_PcrOffset_GetChannelDefSettings( pTransportDeviceContext->hXpt, 1, &Defaults );  /* Pick a ChannelNo that is not used. 1 is just an example */
         BXPT_PcrOffset_Open( pTransportDeviceContext->hXpt, 1, &Defaults, &pTransportDeviceContext->hXptPcrOffset );
         BXPT_PcrOffset_GetSettings( pTransportDeviceContext->hXptPcrOffset, &Settings );

         Settings.UsePcrTimeBase = true;
         Settings.WhichPcr = APP_ENCODER_DPCR;  /* Where X is the same DPCR that the decoders are using */ /* TODO: Check this!!! */
         Settings.CountMode = BXPT_PcrOffset_StcCountMode_eBinary;
         Settings.BroadcastMode = BXPT_PcrOffset_BroadcastMode_eFull42;
         Settings.PidChannelNum = 0;

         BXPT_PcrOffset_SetSettings( pTransportDeviceContext->hXptPcrOffset, &Settings );
      }

      /* Create TSMux Handle */
      {
         BXPT_TsMux_Settings stTsMuxSettings;
         BXPT_TsMux_GetDefaultSettings( &stTsMuxSettings );

         stTsMuxSettings.uiMuxDelay = 100; /* 100 ms for now */
         stTsMuxSettings.hPcrOffset = pTransportDeviceContext->hXptPcrOffset;

         BXPT_TsMux_Create(
                  pTransportDeviceContext->hXpt,
                  &pTransportDeviceContext->hXptTsMux
             );

         BXPT_TsMux_SetSettings(
        		 pTransportDeviceContext->hXptTsMux,
        		 &stTsMuxSettings
        		 );
      }
   }
#endif

   *phDeviceTransport = pTransportDeviceContext;

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
app_CloseTransportDevice(
         TransportDeviceHandle hDeviceTransport
         )
{
#if 0
   uint32_t i;

   for ( i = 0; i < MAX_TRANSPORT_CHANNELS; i++ )
   {
      if ( NULL != &hDeviceTransport->ahChannel[i] )
      {
         app_CloseTransportChannel( &hDeviceTransport->ahChannel[i] );
      }
   }
#endif

#ifndef BTST_P_USE_XPT_MUX_STUB
   BXPT_TsMux_Destroy(
            hDeviceTransport->hXptTsMux
       );

   BXPT_Close(
            hDeviceTransport->hXpt
            );
#endif

   BKNI_Free( hDeviceTransport );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
app_TransportIncrementTime(
         TransportDeviceHandle hDeviceTransport,
         unsigned uiMilliseconds
         )
{
   hDeviceTransport->stStatus.uiESCR += 27000000 / 1000 * uiMilliseconds;
   hDeviceTransport->stStatus.uiSTC += 27000000 / 1000 * uiMilliseconds;
   hDeviceTransport->stStatus.uiSTC &= 0x3FFFFFFFFFFLLU; /* 42-bit wrap */

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
app_GetTransportSettings(
         TransportDeviceHandle hDeviceTransport,
         BMUXlib_TS_TransportSettings *pstTransportSettings
         )
{
   BXPT_TsMux_Settings stTsMuxSettings;

   BXPT_TsMux_GetSettings(
            hDeviceTransport->hXptTsMux,
            &stTsMuxSettings);

   BKNI_Memset(
            pstTransportSettings,
            0,
            sizeof( BMUXlib_TS_TransportSettings )
            );

   pstTransportSettings->uiMuxDelay = stTsMuxSettings.uiMuxDelay;
   /* TODO: Add AFAP parameters */

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
app_SetTransportSettings(
         TransportDeviceHandle hDeviceTransport,
         const BMUXlib_TS_TransportSettings *pstTransportSettings
         )
{
   BXPT_TsMux_Settings stTsMuxSettings;

   BXPT_TsMux_GetSettings(
            hDeviceTransport->hXptTsMux,
            &stTsMuxSettings);

   stTsMuxSettings.uiMuxDelay = pstTransportSettings->uiMuxDelay;
   /* TODO: Add AFAP parameters */

   BXPT_TsMux_SetSettings(
            hDeviceTransport->hXptTsMux,
            &stTsMuxSettings);

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
app_GetTransportStatus(
         TransportDeviceHandle hDeviceTransport,
         BMUXlib_TS_TransportStatus *pstTransportStatus
         )
{
   BXPT_TsMux_Status stTsMuxStatus;

   BDBG_ENTER( app_GetTransportStatus );

   BDBG_ASSERT( hDeviceTransport );
   BDBG_ASSERT( pstTransportStatus );

   BXPT_TsMux_GetStatus(
            hDeviceTransport->hXptTsMux,
            &stTsMuxStatus
            );

   hDeviceTransport->stStatus.uiSTC = stTsMuxStatus.uiSTC;
   hDeviceTransport->stStatus.uiESCR = stTsMuxStatus.uiESCR;

   *pstTransportStatus = hDeviceTransport->stStatus;

   BDBG_LEAVE( app_GetTransportStatus );

   return BERR_TRACE( BERR_SUCCESS );
}

/***********/
/* Channel */
/***********/
BERR_Code
app_OpenTransportChannel(
         TransportDeviceHandle hDeviceTransport,
         TransportChannelHandle *phChannelTransport,
         const TransportChannelSettings* pstSettings
         )
{
   TransportStubContext *pTransportContext = &hDeviceTransport->ahChannel[pstSettings->uiTransportIndex];

   pTransportContext->stSettings = *pstSettings;

   pTransportContext->uiQueueSize = pTransportContext->stSettings.uiTransportDepth + 1;
#ifdef BTST_P_USE_XPT_MUX_STUB
   pTransportContext->astTransportDescriptors = ( const BMUXlib_TS_TransportDescriptor **) BKNI_Malloc( pTransportContext->uiQueueSize * sizeof( BMUXlib_TS_TransportDescriptor* ) );
#else
   pTransportContext->astPvrDescriptors = (BXPT_PvrDescriptor8 *) BMEM_AllocAligned(
            hDeviceTransport->stSettings.hMem,
            pTransportContext->uiQueueSize * sizeof( BXPT_PvrDescriptor8 ),
            4,
            0 );
   {
      BXPT_Playback_ChannelSettings stPlaybackSettings;

      BXPT_Playback_GetChannelDefaultSettings(
               hDeviceTransport->hXpt,
               pstSettings->uiTransportIndex,
               &stPlaybackSettings
               );

      if ( pstSettings->uiTransportIndex == 2 ) /* System Channel should be MPEG Blind */
      {
         BDBG_MSG(("PB[%d] = MPEG_BLIND", pstSettings->uiTransportIndex));
         stPlaybackSettings.SyncMode = BXPT_PB_SYNC_MPEG_BLIND;
         stPlaybackSettings.PacketLength = 0xBC;
      }
      else
      {
         BDBG_MSG(("PB[%d] = SYNC_PES", pstSettings->uiTransportIndex));
         stPlaybackSettings.SyncMode = BXPT_PB_SYNC_PES;
         stPlaybackSettings.PacketLength = 0xB8;
      }

      stPlaybackSettings.Use8WordDesc = true;
      stPlaybackSettings.PesBasedPacing = true;
      stPlaybackSettings.UsePcrTimeBase = true;
      stPlaybackSettings.WhichPcrToUse = APP_ENCODER_DPCR; /* Where X is the same DPCR that was set in BXPT_PcrOffset_Settings above */
      stPlaybackSettings.TimestampMode = BXPT_TimestampMode_e30_2U_Binary;
      stPlaybackSettings.Use32BitTimestamps = true;

      BXPT_Playback_OpenChannel(
               hDeviceTransport->hXpt,
               &pTransportContext->hXptPb,
               pstSettings->uiTransportIndex + 1, /* Add 1 because playback channel 0 is used by vdc_test */
               &stPlaybackSettings
               );

      BXPT_Playback_ConfigPacing(
               pTransportContext->hXptPb,
               BXPT_PacingControl_eStart
               );

      BXPT_TsMux_AddPlayback(
               hDeviceTransport->hXptTsMux,
               pTransportContext->hXptPb
               );

      BXPT_Playback_StartChannel(
               pTransportContext->hXptPb
               );
   }
#endif

   pTransportContext->hDeviceTransport = hDeviceTransport;
   pTransportContext->bInUse = true;

   *phChannelTransport = pTransportContext;

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
app_CloseTransportChannel(
         TransportChannelHandle hChannelTransport
         )
{
#ifdef BTST_P_USE_XPT_MUX_STUB
   if ( NULL != hChannelTransport->astTransportDescriptors )
   {
      BKNI_Free(
               hChannelTransport->astTransportDescriptors
               );
      hChannelTransport->astTransportDescriptors = NULL;
   }
#else
   BXPT_Playback_StopChannel(
            hChannelTransport->hXptPb
            );

   BXPT_Playback_CloseChannel(
            hChannelTransport->hXptPb
            );
   hChannelTransport->hXptPb = NULL;

   if ( NULL != hChannelTransport->astPvrDescriptors )
   {
      BMEM_Free(
               hChannelTransport->hDeviceTransport->stSettings.hMem,
               hChannelTransport->astPvrDescriptors
               );
      hChannelTransport->astPvrDescriptors = NULL;
   }
#endif

   hChannelTransport->hDeviceTransport = NULL;
   hChannelTransport->bInUse = false;

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
app_AddTransportDescriptors(
          TransportChannelHandle hChannelTransport,
          const BMUXlib_TS_TransportDescriptor *astTransportDescriptors, /* Array of pointers to transport descriptors */
          size_t uiCount, /* Count of descriptors in array */
          size_t *puiQueuedCount /* Count of descriptors queued (*puiQueuedCount <= uiCount) */
          )
{
   uint32_t i;
   *puiQueuedCount = 0;

   BDBG_ENTER( app_AddTransportDescriptors );

   for ( i = 0; (i < uiCount); i++ )
   {
      uint32_t uiTempWrite;
      uiTempWrite = hChannelTransport->uiTransportDescriptorsWrite + 1;
      uiTempWrite %= hChannelTransport->uiQueueSize;

      if ( uiTempWrite == hChannelTransport->uiTransportDescriptorsRead )
      {
         /* Queue is Full */
         break;
      }

      {
         static FILE *hFile = NULL;

         if ( hFile == NULL ) {
            hFile = fopen("encoder_output.h264.pes", "wb");
         }

#if 0
         if ( hChannelTransport->stSettings.uiTransportIndex == 2 )
#endif
         {
            fwrite( astTransportDescriptors[i].pBufferAddress,
                    1,
                    astTransportDescriptors[i].uiBufferLength,
                    hFile);

            fflush(hFile);
         }
      }

#ifdef BTST_P_USE_XPT_MUX_STUB
      hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsWrite] = astTransportDescriptors[i];
#else
      BKNI_Memset(
               &hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsWrite],
               0,
               sizeof ( BXPT_PvrDescriptor8 )
               );

      BXPT_Playback_CreateDesc(
               hChannelTransport->hDeviceTransport->hXpt,
               &hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsWrite].Desc,
               (uint8_t *) astTransportDescriptors[i].pBufferAddress,
               astTransportDescriptors[i].uiBufferLength,
               false,
               false,
               NULL
               );

      BXPT_Tsmux_ConfigDesc(
               &hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsWrite],
               &astTransportDescriptors[i].stTsMuxDescriptorConfig
               );

      BDBG_MSG(("TRANSPORT[%d] (@%08x, %8u bytes, escr:%08x) - QUEUED",
               hChannelTransport->stSettings.uiTransportIndex,
               hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsWrite].Desc.BufferStartAddr,
               hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsWrite].Desc.BufferLength,
               hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsWrite].NextPacketPacingTimestamp
               ));

      BXPT_Playback_AddDescriptors(
               hChannelTransport->hXptPb,
               &hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsWrite].Desc,
               &hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsWrite].Desc
               );
#endif

      (*puiQueuedCount)++;
      hChannelTransport->uiTransportDescriptorsReceived++;
      hChannelTransport->uiTransportDescriptorsWrite = uiTempWrite;
   }

   BDBG_LEAVE( app_AddTransportDescriptors );

   return BERR_TRACE( BERR_SUCCESS );
}

BERR_Code
app_GetCompletedTransportDescriptors(
         TransportChannelHandle hChannelTransport,
         size_t *puiCompletedCount /* Count of descriptors completed */
         )
{
#ifndef BTST_P_USE_XPT_MUX_STUB
   BXPT_PvrDescriptor8 *pCurrentPvrDescriptor = NULL;
   BXPT_Playback_ChannelStatus stPlaybackStatus;

   BXPT_Playback_GetCurrentDescriptorAddress(
            hChannelTransport->hXptPb,
            (BXPT_PvrDescriptor **) &pCurrentPvrDescriptor
            );

   BXPT_Playback_GetChannelStatus(
            hChannelTransport->hXptPb,
            &stPlaybackStatus
            );
#endif

   *puiCompletedCount = 0;

   while ( hChannelTransport->uiTransportDescriptorsRead != hChannelTransport->uiTransportDescriptorsWrite )
   {
#ifdef BTST_P_USE_XPT_MUX_STUB
      uint32_t uiESCR;
      uint32_t uiPacket2PacketDelta;

      if ( hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->bNextPacketPacingTimestampValid )
      {
         uiESCR = hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiNextPacketPacingTimestamp;
      }
      else
      {
         uiESCR = hChannelTransport->hDeviceTransport->uiCurrentESCR;
      }

      if ( hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->bPacket2PacketTimestampDeltaValid)
      {
         uiPacket2PacketDelta = hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiPacket2PacketTimestampDelta;
      }
      else
      {
         uiPacket2PacketDelta = hChannelTransport->hDeviceTransport->uiCurrentPacket2PacketDelta;
      }

      hChannelTransport->hDeviceTransport->uiCurrentESCR = uiESCR;
      hChannelTransport->hDeviceTransport->uiCurrentPacket2PacketDelta = uiPacket2PacketDelta;

      if ( 0 != hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiBufferLength )
      {
         hChannelTransport->hDeviceTransport->uiCurrentESCR += (hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiBufferLength / 184) * hChannelTransport->hDeviceTransport->uiCurrentPacket2PacketDelta;
      }

      /* TODO: Handle wrap */
      if ( hChannelTransport->hDeviceTransport->uiCurrentESCR > (hChannelTransport->hDeviceTransport->stStatus.uiESCR & 0xFFFFFFFF) )
      {
         break;
      }

      BDBG_MSG(("TRANSPORT[%d] (@%08x, %8u bytes, rap=%d, escr=%08x (%d), pkt2pkt=%8x (%d))",
               hChannelTransport->stSettings.uiTransportIndex,
               hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->pBufferAddress,
               hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiBufferLength,
               hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->bRandomAccessIndication,
               hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiNextPacketPacingTimestamp,
               hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->bNextPacketPacingTimestampValid,
               hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiPacket2PacketTimestampDelta,
               hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->bPacket2PacketTimestampDeltaValid
               ));
#if 0
      if ( 0 != hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiBufferLength )
      {
         uint32_t j;
         BKNI_Printf("\t\t{\n\t\t");

         for (j = 0; j < hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiBufferLength && j <= 188; j++)
         {
            if ( j && (0 == (j % 8))) BKNI_Printf("\n\t\t");
            BKNI_Printf("%02x ", ((uint8_t*) hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->pBufferAddress)[j]);
         }
         if ( j != hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiBufferLength )
         {
            BKNI_Printf("...");
         }
         BKNI_Printf("\n\t\t}\n");

         if ( ( NULL != hChannelTransport->hFile )
              && ( 188 == hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiBufferLength ) )
         {
            fwrite(hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->pBufferAddress,
                   hChannelTransport->astTransportDescriptors[hChannelTransport->uiTransportDescriptorsRead]->uiBufferLength,
                   1,
                   hChannelTransport->hFile
                   );
         }
      }
#endif
#else
      if ( ( NULL != pCurrentPvrDescriptor )
           && ( pCurrentPvrDescriptor == &hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsRead] )
           && ( false == stPlaybackStatus.Finished )
         )
      {
         break;
      }

      BDBG_MSG(("TRANSPORT[%d] (@%08x, %8u bytes, escr:%08x) - DONE",
               hChannelTransport->stSettings.uiTransportIndex,
               hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsRead].Desc.BufferStartAddr,
               hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsRead].Desc.BufferLength,
               hChannelTransport->astPvrDescriptors[hChannelTransport->uiTransportDescriptorsRead].NextPacketPacingTimestamp
               ));

#endif

      (*puiCompletedCount)++;
      hChannelTransport->uiTransportDescriptorsRead++;
      hChannelTransport->uiTransportDescriptorsRead %= hChannelTransport->uiQueueSize;
   }

   return BERR_TRACE( BERR_SUCCESS );
}
