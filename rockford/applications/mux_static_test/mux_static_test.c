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
#include <memory.h>
#include "framework.h"

/* base modules */
#include "bstd.h"           /* standard types */
#include "berr.h"           /* error code */
#include "bdbg.h"           /* debug interface */
#include "bkni.h"           /* kernel interface */

#include "btst_kni.h"       /* test interface */

#include "bmuxlib.h"
#include "bmuxlib_ts.h"

#include "bxpt.h"
#include "bxpt_playback.h"
#include "bxpt_rave.h"
#include "bxpt_tsmux.h"

#include "video_encoder_stub.h"
#include "audio_encoder_stub.h"
#include "transport_stub.h"

BDBG_MODULE(mux_static_test);

#define APP_VIDEO_PID 0x0011
#define APP_VIDEO_TRANSPORT_IDX 0

#define APP_AUDIO_PID 0x0021
#define APP_AUDIO_TRANSPORT_IDX 1

#define APP_SYSTEM_TRANSPORT_IDX 2

#define APP_PCR_PID 0x0012
#define APP_PCR_INTERVAL 50
#define APP_PCR_TRANSPORT_IDX APP_SYSTEM_TRANSPORT_IDX

#define APP_PAT_PID 0x0000
#define APP_PAT_TRANSPORT_IDX APP_SYSTEM_TRANSPORT_IDX
static const uint8_t s_auiTSPacket_PAT[188] =
{
   0x47,0x40,0x00,0x30,0xa6,0x40,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0x00,0x00,0xb0,0x0d,0x00,
   0x00,0xc1,0x00,0x00,0x00,0x01,0xe0,0x01,
   0x65,0x80,0x5e,0xdc,
};

#define APP_PMT_PID 0x0001
#define APP_PMT_TRANSPORT_IDX APP_SYSTEM_TRANSPORT_IDX
static const uint8_t s_auiTSPacket_PMT[188] =
{
   0x47,0x40,0x01,0x30,0xa1,0x40,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x02,
   0xb0,0x12,0x00,0x01,0xc1,0x00,0x00,0xe0,
   0x12,0xf0,0x00,0x1b,0xe0,0x11,0xf0,0x00,
   0x3d,0x19,0x07,0x2f,
};

static const uint8_t s_auiTSPacket_PCR[188] =
{
   0x47,0x40,0x12,0x30,0xb7,0x50,0x00,0x00,
   0x00,0x00,0x7e,0x00,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
};


static const uint8_t g_ProgramAssociationTable[] =
{
 0x00, 0x00, 0x00 /* TODO: Fill in PAT */
};

static const uint8_t g_ProgramMapTable[] =
{
 0x00, 0x00, 0x00 /* TODO: Fill in PMT */
};

typedef struct TaskHandles
{
      BMUXlib_TS_Handle hMuxTS;

      TransportDeviceHandle hDeviceTransport;

      uint32_t uiVideoEncoderCount;
      VideoEncoderHandle *ahVideoEncoder;

      BKNI_EventHandle hFinishedEvent;
} TaskHandles;

#define APP_SIMULATION_TIME_INCREMENT 100

void app_MuxTask(
         BTST_TaskHandle hTask,
         void *pvContext
         )
{
   BERR_Code rc;
   TaskHandles *pstHandles = (TaskHandles*) pvContext;
   uint32_t i;

   BMUXlib_DoMux_Status stStatus;

   BSTD_UNUSED(hTask);

   do
   {
      BDBG_MSG(("Executing User Thread"));
      BKNI_EnterCriticalSection();

      rc = BMUXlib_TS_DoMux(
               pstHandles->hMuxTS,
               &stStatus
               );

      if ( BMUXlib_State_eFinished == stStatus.eState )
      {
         BDBG_MSG(("Finished Callback"));
         BKNI_SetEvent_isr( pstHandles->hFinishedEvent );
      }

      BKNI_LeaveCriticalSection();

      if ( 0 != stStatus.uiNextExecutionTime )
      {
#if 1
         BKNI_Sleep( stStatus.uiNextExecutionTime );
#elif 0
         sleep( 1 );
#endif

         app_TransportIncrementTime(
                  pstHandles->hDeviceTransport,
                  stStatus.uiNextExecutionTime
                  );

         for (i = 0; i < pstHandles->uiVideoEncoderCount; i++ )
         {
            if ( NULL != pstHandles->ahVideoEncoder[i] )
            {
               app_VideoEncoderIncrementTime(
                        pstHandles->ahVideoEncoder[i],
                        stStatus.uiNextExecutionTime
                        );
            }
         }
      }
   } while ( BERR_SUCCESS == rc );
}

#ifndef BTST_P_USE_XPT_MUX_STUB
typedef struct BTST_Muxer_Info
{
      TransportDeviceHandle hDeviceTransport;
      BTST_TaskHandle           hCaptureMuxOutputTask;
      bool                      bEnableCaptureMuxOutputHandler;
      FILE *hTSOutput;

#if 0
      BREG_Handle               hReg;

      TransportChannelHandle ahChannelTransport[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES];

      BMUXlib_TS_Handle hMuxTS;

      BTST_TaskHandle           hMuxerTask;
      bool                      bEnableMuxerHandler;
      BKNI_EventHandle          hFinishedEvent;
#endif
} BTST_Muxer_Info;

void app_CaptureMuxBufferHandler
        ( BTST_TaskHandle          hTask,
          void                    *pContext )
{
   BXPT_Rave_Handle hRave;
   BXPT_RaveCx_Handle hCtx;
   BTST_Muxer_Info *pstMuxerInfo = (BTST_Muxer_Info *) pContext;

   pstMuxerInfo->hTSOutput = fopen("encoder_output.h264.ts", "wb");

   if ( NULL == pstMuxerInfo->hTSOutput )
   {
    BDBG_ERR(("Error Opening encoder output file %s", "encoder_output.h264.ts"));
    return;
   }

   BSTD_UNUSED(hTask);

   {
     BXPT_Rave_ChannelSettings stRaveSettings;
     BXPT_Rave_RecordSettings stRaveRecordSettings;

     BAVC_CdbItbConfig CdbItb =
     {
      { 512 * 1024, 4, true },
      { 512 * 1024, 4, true },
      false
     };

     BXPT_Rave_GetChannelDefaultSettings(
              pstMuxerInfo->hDeviceTransport->hXpt,
              0,
              &stRaveSettings
              );

     BDBG_MSG(("Opening RAVE"));
     BXPT_Rave_OpenChannel(
              pstMuxerInfo->hDeviceTransport->hXpt,
              &hRave,
              0,
              &stRaveSettings
              );

     BDBG_MSG(("Allocating Record Context"));
     BXPT_Rave_AllocContext(
              hRave,
              BXPT_RaveCx_eRecord,
              &CdbItb,
              &hCtx
              );

     BXPT_Rave_GetRecordConfig(
              hCtx,
              &stRaveRecordSettings
              );

     stRaveRecordSettings.OutputFormat = BAVC_StreamType_eTsMpeg;
     stRaveRecordSettings.UseTimeStamps = false;
     stRaveRecordSettings.TsInitEn = false;
     stRaveRecordSettings.StreamIdHi = 0xEF;
     stRaveRecordSettings.StreamIdLo = 00;
     stRaveRecordSettings.BandHoldEn = true;

     {
        BXPT_Rave_ContextThresholds stRaveThresholds;

        BXPT_Rave_ComputeThresholds( hCtx, CdbItb.Cdb.Length, CdbItb.Itb.Length, &stRaveThresholds );
        stRaveRecordSettings.CdbUpperThreshold = stRaveThresholds.CdbUpper;
        stRaveRecordSettings.CdbLowerThreshold = 1;
        stRaveRecordSettings.ItbUpperThreshold = stRaveThresholds.ItbUpper;
        stRaveRecordSettings.ItbLowerThreshold = 1;
        BDBG_MSG(( "CdbUpperThreshold %u, CdbLowerThreshold %u, ItbUpperThreshold %u, ItbLowerThreshold %u",
            stRaveRecordSettings.CdbUpperThreshold, stRaveRecordSettings.CdbLowerThreshold, stRaveRecordSettings.ItbUpperThreshold, stRaveRecordSettings.ItbLowerThreshold ));
     }

     BDBG_MSG(("Setting Record Config"));
     BXPT_Rave_SetRecordConfig(
              hCtx,
              &stRaveRecordSettings
              );

     /* VIDEO Channel */
     {
        BXPT_Playback_PacketizeConfig stPacketConfig =
        {
         APP_VIDEO_PID,                                                                        /* PID. Will be placed into the TS header */
         0,                                                        /* Mapped to PID channel 0 */
         BXPT_Playback_PacketizerMode_Pes_MapAll,        /* Map everything, regardless of Stream ID */
         {
          0x00                                                                            /* Ignored, since we're mapping all PES streams */
         }
        };

        BDBG_MSG(("Adding PID Channel", 0));
        BXPT_Rave_AddPidChannel(
                 hCtx,
                 0,
                 false
                 );

        BDBG_MSG(("Configuring PID Channel: %04x from PB channel %d", APP_VIDEO_PID, APP_VIDEO_TRANSPORT_IDX + 1));
        BXPT_ConfigurePidChannel(
                 pstMuxerInfo->hDeviceTransport->hXpt,
                 0,
                 APP_VIDEO_PID,
                 BXPT_PB_PARSER( APP_VIDEO_TRANSPORT_IDX + 1 )
                 );

        BDBG_MSG(("Enabling PID Channel: %04d", 0));
        BXPT_EnablePidChannel(
                 pstMuxerInfo->hDeviceTransport->hXpt,
                 0
                 );


        BDBG_MSG(("Start Packetizing Stream"));
        BXPT_Playback_PacketizeStream(
                 pstMuxerInfo->hDeviceTransport->ahChannel[APP_VIDEO_TRANSPORT_IDX].hXptPb,
                 0,
                 &stPacketConfig,
                 true
                 );
     }

     /* SYSTEM Channel */
     {
        /* PCR PID */
        {
           BDBG_MSG(("Adding PID Channel", 1));
           BXPT_Rave_AddPidChannel(
                    hCtx,
                    1,
                    false
                    );

           BDBG_MSG(("Configuring PID Channel: %04x from PB channel %d", APP_PCR_PID, APP_PCR_TRANSPORT_IDX + 1));
           BXPT_ConfigurePidChannel(
                    pstMuxerInfo->hDeviceTransport->hXpt,
                    1,
                    APP_PCR_PID,
                    BXPT_PB_PARSER( APP_PCR_TRANSPORT_IDX + 1 )
                    );

           BDBG_MSG(("Enabling PID Channel: %04d", 1));
           BXPT_EnablePidChannel(
                    pstMuxerInfo->hDeviceTransport->hXpt,
                    1
                    );
        }

        /* PAT PID */
        {
           BDBG_MSG(("Adding PID Channel", 2));
           BXPT_Rave_AddPidChannel(
                    hCtx,
                    2,
                    false
                    );

           BDBG_MSG(("Configuring PID Channel: %04x from PB channel %d", APP_PAT_PID, APP_PAT_TRANSPORT_IDX + 1));
           BXPT_ConfigurePidChannel(
                    pstMuxerInfo->hDeviceTransport->hXpt,
                    2,
                    APP_PAT_PID,
                    BXPT_PB_PARSER( APP_PAT_TRANSPORT_IDX + 1 )
                    );

           BDBG_MSG(("Enabling PID Channel: %04d", 2));
           BXPT_EnablePidChannel(
                    pstMuxerInfo->hDeviceTransport->hXpt,
                    2
                    );
        }

        /* PMT PID */
        {
           BDBG_MSG(("Adding PID Channel", 3));
           BXPT_Rave_AddPidChannel(
                    hCtx,
                    3,
                    false
                    );

           BDBG_MSG(("Configuring PID Channel: %04x from PB channel %d", APP_PMT_PID, APP_PMT_TRANSPORT_IDX + 1));
           BXPT_ConfigurePidChannel(
                    pstMuxerInfo->hDeviceTransport->hXpt,
                    3,
                    APP_PMT_PID,
                    BXPT_PB_PARSER( APP_PAT_TRANSPORT_IDX + 1 )
                    );

           BDBG_MSG(("Enabling PID Channel: %04d", 3));
           BXPT_EnablePidChannel(
                    pstMuxerInfo->hDeviceTransport->hXpt,
                    3
                    );
        }
     }

   }

   BDBG_MSG(("Enabling Context"));
   BXPT_Rave_EnableContext(
            hCtx
            );

   while( pstMuxerInfo->bEnableCaptureMuxOutputHandler )
   {
      BXPT_Rave_ContextPtrs stCtxPtrs;

      BXPT_Rave_CheckBuffer(
               hCtx,
               &stCtxPtrs
               );

      if ( 0 != stCtxPtrs.Cdb.ByteCount )
      {
         BDBG_MSG(("Wrote %d bytes of TS", stCtxPtrs.Cdb.ByteCount));
         fwrite(
                  (void *) stCtxPtrs.Cdb.DataPtr,
                  sizeof( char ),
                  stCtxPtrs.Cdb.ByteCount,
                  pstMuxerInfo->hTSOutput
                 );

         BXPT_Rave_UpdateReadOffset(
                  hCtx,
                  stCtxPtrs.Cdb.ByteCount,
                  stCtxPtrs.Itb.ByteCount
                  );

         fflush(pstMuxerInfo->hTSOutput);
      }
      else
      {
         BDBG_MSG(("No TS data"));
      }
      BKNI_Sleep(100); /* Wait 100 ms before checking for more frames */
   }

   BDBG_MSG(("Removing PID Channel"));
   BXPT_Rave_RemovePidChannel(
            hCtx,
            0
            );

   BDBG_MSG(("Disabling Context"));
   BXPT_Rave_DisableContext(
            hCtx
            );

   BDBG_MSG(("Freeing Context"));
   BXPT_Rave_FreeContext(
            hCtx
            );

   BDBG_MSG(("Closing RAVE"));
   BXPT_Rave_CloseChannel(
            hRave
            );

   fclose( pstMuxerInfo->hTSOutput );
   pstMuxerInfo->hTSOutput = NULL;
}

void app_GetMuxedFrames
        (void *pContext)
{
   BTST_Muxer_Info *pstMuxerInfo = (BTST_Muxer_Info *) pContext;

   /* toggle it */
   pstMuxerInfo->bEnableCaptureMuxOutputHandler ^= true;

   if( pstMuxerInfo->bEnableCaptureMuxOutputHandler )
   {
      printf("start to capture muxed buffers\n");
      BTST_CreateTask(&pstMuxerInfo->hCaptureMuxOutputTask, app_CaptureMuxBufferHandler, (void*)pstMuxerInfo);
   }
   else
   {
      printf("stop to capture muxed buffers\n");
      BKNI_Sleep(100);
      BTST_DestroyTask(pstMuxerInfo->hCaptureMuxOutputTask);
   }
   return;
}
#endif

int app_main( int argc, char **argv )
{
   /* Framework and System specific declarations */
   BERR_Code iErr = 0;
   BSystem_Info sysInfo;
   BFramework_Info frmInfo;
#ifndef BTST_P_USE_XPT_MUX_STUB
   BTST_Muxer_Info stMuxerInfo;
#endif

   VideoEncoderHandle ahVideoEncoder[BMUXLIB_TS_MAX_VIDEO_PIDS];
   TransportDeviceHandle hDeviceTransport;
   TransportChannelHandle ahChannelTransport[BMUXLIB_TS_MAX_TRANSPORT_INSTANCES];
   TaskHandles stHandles;

   BMUXlib_TS_Handle hMuxTS;
   BTST_TaskHandle hTask;

#define APP_SYSTEMDATA_COUNT 2
   BMUXlib_TS_SystemData astSystemDataBuffer[APP_SYSTEMDATA_COUNT];

   BSTD_UNUSED(argc);
   BSTD_UNUSED(argv);

   BDBG_MSG(("Start"));
   BDBG_Init();

   /* DBG Modules */
   BDBG_SetModuleLevel("mux_static_test", BDBG_eMsg);
#if 0
   BDBG_SetModuleLevel("BMUXlib_TS_EOS", BDBG_eMsg);
   BDBG_SetModuleLevel("BMUXlib_TS_FINISH", BDBG_eMsg);
   BDBG_SetModuleLevel("BMUXlib_TS_PENDING", BDBG_eMsg);
   BDBG_SetModuleLevel("BMUXlib_TS", BDBG_eMsg);
   BDBG_SetModuleLevel("BMUXlib_TS_PRIV", BDBG_eMsg);
#endif
#if 0
   BDBG_SetModuleLevel("TRANSPORT_STUB", BDBG_eMsg);
   BDBG_SetModuleLevel("VIDEO_ENCODER_STUB", BDBG_eMsg);
#endif

   BDBG_MSG(("Initializing Framework"));
   BSystem_Init(argc, argv, &sysInfo);
   BFramework_Init(&sysInfo, &frmInfo);

#ifndef BTST_P_USE_XPT_MUX_STUB
   BKNI_Memset(
            &stMuxerInfo,
            0,
            sizeof (BTST_Muxer_Info)
            );
#endif

   /* Open Video Encoder Stub */
   BKNI_Memset(ahVideoEncoder, 0, sizeof(VideoEncoderHandle) * BMUXLIB_TS_MAX_VIDEO_PIDS );
   {
      VideoEncoderSettings stSettings;

      BDBG_MSG(("Open Video Encoder"));
      BKNI_Memset(
               &stSettings,
               0,
               sizeof( VideoEncoderSettings )
               );

      stSettings.hMem = frmInfo.hMem;

      app_OpenVideoEncoder(
               &ahVideoEncoder[0],
               &stSettings
               );
   }

   /* Open Video Transport Stub */
   {
      TransportDeviceSettings stSettings;

      BDBG_MSG(("Open Transport Device"));

      BKNI_Memset(
               &stSettings,
               0,
               sizeof( TransportDeviceSettings )
               );

      stSettings.hChp = frmInfo.hChp;
      stSettings.hReg = frmInfo.hReg;
      stSettings.hInt = frmInfo.hInt;
      stSettings.hMem = frmInfo.hMem;

      stSettings.szFriendlyName = "XPT";

      app_OpenTransportDevice(
               &hDeviceTransport,
               &stSettings
               );
   }
   BKNI_Memset(ahChannelTransport, 0, sizeof(TransportChannelHandle) *  BMUXLIB_TS_MAX_TRANSPORT_INSTANCES );
   {
      TransportChannelSettings stSettings;

      BDBG_MSG(("Open Video Transport"));

      BKNI_Memset(
               &stSettings,
               0,
               sizeof( TransportChannelSettings )
               );

      stSettings.szFriendlyName = "Video XPT";
      stSettings.uiTransportDepth = 256;
      stSettings.uiTransportIndex = APP_VIDEO_TRANSPORT_IDX;

      app_OpenTransportChannel(
               hDeviceTransport,
               &ahChannelTransport[APP_VIDEO_TRANSPORT_IDX],
               &stSettings
               );
   }

   {
      TransportChannelSettings stSettings;

      BDBG_MSG(("Open Audio Transport"));

      BKNI_Memset(
               &stSettings,
               0,
               sizeof( TransportChannelSettings )
               );

      stSettings.szFriendlyName = "Audio XPT";
      stSettings.uiTransportDepth = 256;
      stSettings.uiTransportIndex = APP_AUDIO_TRANSPORT_IDX;

      app_OpenTransportChannel(
               hDeviceTransport,
               &ahChannelTransport[APP_AUDIO_TRANSPORT_IDX],
               &stSettings
               );
   }

   /* Open System Transport Stub */
   {
      TransportChannelSettings stSettings;

      BDBG_MSG(("Open System Transport"));

      BKNI_Memset(
               &stSettings,
               0,
               sizeof( TransportChannelSettings )
               );

      stSettings.szFriendlyName = "System XPT";
      stSettings.uiTransportDepth = 256;
      stSettings.uiTransportIndex = APP_SYSTEM_TRANSPORT_IDX;

      app_OpenTransportChannel(
               hDeviceTransport,
               &ahChannelTransport[APP_SYSTEM_TRANSPORT_IDX],
               &stSettings
               );
   }

   /* Create TS Muxer */
   {
      BMUXlib_TS_CreateSettings stCreateSettings;
      BDBG_MSG(("Creating Muxer"));

      BMUXlib_TS_GetDefaultCreateSettings(
               &stCreateSettings
               );

      stCreateSettings.hMem = frmInfo.hMem;

      BMUXlib_TS_Create(
               &hMuxTS,
               &stCreateSettings
               );
   }

   /* Setup Events */
   {
      BKNI_CreateEvent( &stHandles.hFinishedEvent );
   }

   /* Configure Muxer */
   {
      BMUXlib_TS_MuxSettings stMuxSettings;
      BDBG_MSG(("Configuring Muxer"));

      BMUXlib_TS_GetDefaultMuxSettings(
               &stMuxSettings
               );

      BMUXlib_TS_SetMuxSettings(
               hMuxTS,
               &stMuxSettings
               );
   }

   /* Start Muxer */
   {
      BMUXlib_TS_StartSettings stStartSettings;
      BDBG_MSG(("Starting Muxer"));

      BMUXlib_TS_GetDefaultStartSettings(
               &stStartSettings
               );

      /* Setup Transport */
      stStartSettings.transport.stDeviceInterface.pContext = hDeviceTransport;
      stStartSettings.transport.stDeviceInterface.fGetTransportSettings = (BMUXlib_TS_GetTransportSettings) app_GetTransportSettings;
      stStartSettings.transport.stDeviceInterface.fSetTransportSettings = (BMUXlib_TS_SetTransportSettings) app_SetTransportSettings;
      stStartSettings.transport.stDeviceInterface.fGetTransportStatus = (BMUXlib_TS_GetTransportStatus) app_GetTransportStatus;

      stStartSettings.transport.stChannelInterface[APP_VIDEO_TRANSPORT_IDX].pContext = ahChannelTransport[APP_VIDEO_TRANSPORT_IDX];
      stStartSettings.transport.stChannelInterface[APP_VIDEO_TRANSPORT_IDX].fAddTransportDescriptors = (BMUXlib_TS_AddTransportDescriptors) app_AddTransportDescriptors;
      stStartSettings.transport.stChannelInterface[APP_VIDEO_TRANSPORT_IDX].fGetCompletedTransportDescriptors = (BMUXlib_TS_GetCompletedTransportDescriptors) app_GetCompletedTransportDescriptors;

      stStartSettings.transport.stChannelInterface[APP_AUDIO_TRANSPORT_IDX].pContext = ahChannelTransport[APP_AUDIO_TRANSPORT_IDX];;
      stStartSettings.transport.stChannelInterface[APP_AUDIO_TRANSPORT_IDX].fAddTransportDescriptors = (BMUXlib_TS_AddTransportDescriptors) app_AddTransportDescriptors;
      stStartSettings.transport.stChannelInterface[APP_AUDIO_TRANSPORT_IDX].fGetCompletedTransportDescriptors = (BMUXlib_TS_GetCompletedTransportDescriptors) app_GetCompletedTransportDescriptors;

      stStartSettings.transport.stChannelInterface[APP_SYSTEM_TRANSPORT_IDX].pContext = ahChannelTransport[APP_SYSTEM_TRANSPORT_IDX];
      stStartSettings.transport.stChannelInterface[APP_SYSTEM_TRANSPORT_IDX].fAddTransportDescriptors = (BMUXlib_TS_AddTransportDescriptors) app_AddTransportDescriptors;
      stStartSettings.transport.stChannelInterface[APP_SYSTEM_TRANSPORT_IDX].fGetCompletedTransportDescriptors = (BMUXlib_TS_GetCompletedTransportDescriptors) app_GetCompletedTransportDescriptors;

      /* Setup Video */
      stStartSettings.video[0].uiPID = APP_VIDEO_PID;
      stStartSettings.video[0].stInputInterface.pContext = ahVideoEncoder[0];
      stStartSettings.video[0].stInputInterface.fGetBufferDescriptors = (BMUXlib_GetVideoBufferDescriptors) app_GetVideoBufferDescriptors;
      stStartSettings.video[0].stInputInterface.fConsumeBufferDescriptors = (BMUXlib_ConsumeVideoBufferDescriptors) app_ConsumeVideoBufferDescriptors;
      stStartSettings.video[0].stInputInterface.fGetBufferStatus = (BMUXlib_GetVideoBufferStatus) app_GetVideoBufferStatus;
      stStartSettings.video[0].uiTransportChannelIndex = APP_VIDEO_TRANSPORT_IDX;
      stStartSettings.uiNumValidVideoPIDs = 1;

#if 0
      /* Setup Audio */
      stStartSettings.audio[0].uiPID = APP_AUDIO_PID;
      stStartSettings.audio[0].stInputInterface.pContext = hAudioEncoder;
      stStartSettings.audio[0].stInputInterface.fGetBufferDescriptors = app_GetAudioBufferDescriptors;
      stStartSettings.audio[0].stInputInterface.fConsumeBufferDescriptors = app_ConsumeAudioBufferDescriptors;
      stStartSettings.audio[0].stInputInterface.fGetBufferStatus = app_GetAudioBufferStatus;
      stStartSettings.audio[0].uiTransportChannelIndex = APP_AUDIO_TRANSPORT_IDX;
      stStartSettings.uiNumValidAudioPIDs = 1;
#endif

      /* Setup PCR */
      stStartSettings.stPCRData.uiPID = APP_PCR_PID;
      stStartSettings.stPCRData.uiInterval = APP_PCR_INTERVAL;
      stStartSettings.stPCRData.uiTransportChannelIndex = APP_PCR_TRANSPORT_IDX;

      /* Setup Mux Mode */
      stStartSettings.bNonRealTimeMode = false;

      /* Setup Mux Service Period */
      stStartSettings.uiServiceLatencyTolerance = 20;

      {
         unsigned i;
         size_t uiSystemDataBufferQueued;

         for ( i = 0; i < APP_SYSTEMDATA_COUNT; i++ )
         {
            BKNI_Memset( &astSystemDataBuffer[i], 0, sizeof( BMUXlib_TS_SystemData) );

            astSystemDataBuffer[i].pData = BMEM_Alloc( frmInfo.hMem, 188 );
            astSystemDataBuffer[i].uiSize = 188;
         }

         astSystemDataBuffer[0].uiTimestampDelta = 0;
         BKNI_Memcpy(
                  (void *) astSystemDataBuffer[0].pData,
                  s_auiTSPacket_PAT,
                  sizeof( s_auiTSPacket_PAT )
                  );

         astSystemDataBuffer[1].uiTimestampDelta = 0;
         BKNI_Memcpy(
                  (void *) astSystemDataBuffer[1].pData,
                  s_auiTSPacket_PMT,
                  sizeof( s_auiTSPacket_PMT )
                  );

         BMUXlib_TS_AddSystemDataBuffers(
                  hMuxTS,
                  (const BMUXlib_TS_SystemData *) astSystemDataBuffer,
                  APP_SYSTEMDATA_COUNT,
                  &uiSystemDataBufferQueued
                  );

         BDBG_ASSERT( uiSystemDataBufferQueued == APP_SYSTEMDATA_COUNT );
      }

      BMUXlib_TS_Start(
               hMuxTS,
               &stStartSettings
               );
   }

#ifndef BTST_P_USE_XPT_MUX_STUB
   /* Start Muxer Consumption Thread */
   {
      BDBG_MSG(("Starting Muxer Consumer Thread"));

      stMuxerInfo.hDeviceTransport = hDeviceTransport;

      stHandles.uiVideoEncoderCount = BMUXLIB_TS_MAX_VIDEO_PIDS;
      stHandles.ahVideoEncoder = ahVideoEncoder;

      app_GetMuxedFrames(
               &stMuxerInfo
               );
   }

#if 1
   BDBG_MSG(("Sleeping...press enter continue"));
#if 0
   BKNI_Sleep(3000);
#elif 0
   sleep( 20 );
#else
   {
      char str[255];
      gets(str);
   }
#endif
#endif
#endif

   /* Start User Thread */
   {
      BDBG_MSG(("Starting Muxer Feeder Thread"));

      stHandles.hMuxTS = hMuxTS;
      stHandles.hDeviceTransport = hDeviceTransport;

      stHandles.uiVideoEncoderCount = BMUXLIB_TS_MAX_VIDEO_PIDS;
      stHandles.ahVideoEncoder = ahVideoEncoder;

      BTST_CreateTask(
               &hTask,
               app_MuxTask,
               &stHandles
               );
   }

   BDBG_MSG(("Sleeping...press enter continue"));
#if 0
   BKNI_Sleep(3000);
#elif 0
   sleep( 20 );
#else
   {
      char str[255];
      gets(str);
   }
#endif

   /* Stop Encoder */
   {
      BDBG_MSG(("Stopping Encoder"));
      app_StopVideoEncoder(
         ahVideoEncoder[0]
      );
   }

   /* Finish Muxer */
   {
      BMUXlib_TS_FinishSettings stFinishSettings;
      BDBG_MSG(("Finishing"));

      BMUXlib_TS_GetDefaultFinishSettings(
               &stFinishSettings
               );

      BKNI_EnterCriticalSection();

      BMUXlib_TS_Finish(
               hMuxTS,
               &stFinishSettings
               );

      BKNI_LeaveCriticalSection();
   }

   /* Wait for Finish */
   {
      BERR_Code rc;
      BDBG_MSG(("Wait for Finish"));

      rc = BKNI_WaitForEvent( stHandles.hFinishedEvent, 5000 );

      if ( BERR_SUCCESS != rc )
      {
         BDBG_ERR(("Finish Event Timed Out!"));
      }
   }

   /* Stop Muxer */
   {
      BDBG_MSG(("Stopping"));
      BMUXlib_TS_Stop(
               hMuxTS
               );
   }

   {
      unsigned i;

      for ( i = 0; i < APP_SYSTEMDATA_COUNT; i++ )
      {
         BMEM_Free( frmInfo.hMem, (void *) astSystemDataBuffer[i].pData );
      }
   }

   BDBG_MSG(("Sleeping...press enter continue"));
#if 0
   BKNI_Sleep(1000);
#elif 1
   sleep(1);
#else
   {
      char str[255];
      gets(str);
   }
#endif

#ifndef BTST_P_USE_XPT_MUX_STUB
   /* Stop Muxer Consumption Thread */
   {
      BDBG_MSG(("Stopping Muxer Consumer Thread"));

      app_GetMuxedFrames(
               &stMuxerInfo
               );
   }
#endif

   /* Stop User Thread */
   {
      BDBG_MSG(("Stopping User Thread"));
      BKNI_EnterCriticalSection();
      BKNI_LeaveCriticalSection();

      BTST_DestroyTask(
               hTask
               );

   }

   /* Teardown Events */
   {
      BDBG_MSG(("Teardown Events"));
      BKNI_DestroyEvent( stHandles.hFinishedEvent );
   }

   /* Destroy TS Muxer */
   {
      BDBG_MSG(("Destroy Muxer"));
      BMUXlib_TS_Destroy(
               hMuxTS
               );
   }

   /* Close System Transport Stub */
   {
      BDBG_MSG(("Close System Transport"));
      app_CloseTransportChannel(
               ahChannelTransport[APP_SYSTEM_TRANSPORT_IDX]
               );
   }

   /* Close Audio Transport Stub */
   {
      BDBG_MSG(("Close Audio Transport"));
      app_CloseTransportChannel(
               ahChannelTransport[APP_AUDIO_TRANSPORT_IDX]
               );
   }

   /* Close Video Transport Stub */
   {
      BDBG_MSG(("Close Video Transport"));
      app_CloseTransportChannel(
               ahChannelTransport[APP_VIDEO_TRANSPORT_IDX]
               );
   }

   /* Close Transport Device */
   {
      BDBG_MSG(("Close Transport Device"));

      app_CloseTransportDevice(
               hDeviceTransport
               );
   }

   /* Close Video Encoder Stub */
   {
      BDBG_MSG(("Close Video Encoder"));
      app_CloseVideoEncoder(
               ahVideoEncoder[0]
               );
   }

#if 0
   BDBG_MSG(("Uninitialize Framework"));
   BFramework_Uninit(&frmInfo);
   BSystem_Uninit(&sysInfo);
   BDBG_MSG(("Done"));
   BDBG_Uninit();
#endif
   return iErr;
}
