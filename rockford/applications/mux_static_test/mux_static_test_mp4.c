/***************************************************************************
 *     Copyright (c) 2010-2011, Broadcom Corporation
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
 * Mux static test for MP4
 *
 * Note that this test does NOT use hardware - does not use Transport.
 * Output is File I/O
 *
 * Revision History:
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "bstd.h"
#include "btst_kni.h"       /* test interface */
#include "framework.h"
#include "bavc.h"
#include "bavc_vce.h"
#include "video_encoder_stub.h"
#include "audio_encoder_stub.h"
#include "bmuxlib_file_mp4.h"
#include "muxer_file.h"

#define VIDEO_ENABLED 1
#define AUDIO_ENABLED 0

BDBG_MODULE(mux_static_test);
BDBG_FILE_MODULE(mux_storage);

typedef struct
{
  BMUXlib_File_MP4_Handle hMP4Mux;

  uint32_t uiVideoEncoderCount;
  VideoEncoderHandle *ahVideoEncoder;
  uint32_t uiAudioEncoderCount;
  AudioEncoderHandle *ahAudioEncoder;
  BKNI_EventHandle hFinishedEvent;
  BKNI_EventHandle hInputDoneEvent;
} TaskHandles;

BTMR_TimerHandle ghTimer;

#define APP_SIMULATION_TIME_INCREMENT 50

void app_MuxTask(BTST_TaskHandle hTask, void *pvContext);

int app_main( int argc, char **argv )
{
   /* Framework and System specific declarations */
   BERR_Code iErr = 0;
   BSystem_Info sysInfo;
   BFramework_Info frmInfo;

#if VIDEO_ENABLED == 1
   VideoEncoderHandle ahVideoEncoder[BMUXLIB_FILE_MP4_MAX_VIDEO_INPUTS];
#endif
#if AUDIO_ENABLED == 1
   AudioEncoderHandle ahAudioEncoder[BMUXLIB_FILE_MP4_MAX_AUDIO_INPUTS];
#endif
   BMUXlib_File_MP4_Handle hMP4Mux;
   BMUXlib_StorageSystemInterface stStorageSystem;
   BMUXlib_StorageObjectInterface stOutput;
   TaskHandles stHandles;
   BTST_TaskHandle hTask;

   BSTD_UNUSED(argc);
   BSTD_UNUSED(argv);

   BSystem_Init(argc, argv, &sysInfo);
   BFramework_Init(&sysInfo, &frmInfo);

   /* DBG Modules */

   /*BDBG_SetModuleLevel("mux_static_test", BDBG_eMsg);*/
   /*BDBG_SetModuleLevel("mux_storage", BDBG_eMsg);*/

   BDBG_SetModuleLevel("BMUXLIB_FILE_MP4", BDBG_eMsg);
   BDBG_SetModuleLevel("BMUXLIB_FILE_MP4_PRIV", BDBG_eMsg);
   BDBG_SetModuleLevel("BMUXLIB_FILE_MP4_BOXES", BDBG_eMsg);
   BDBG_SetModuleLevel("BMUX_MP4_USAGE", BDBG_eMsg);
/*   BDBG_SetModuleLevel("BMUX_MP4_IN_DESC", BDBG_eMsg);
   BDBG_SetModuleLevel("BMUX_MP4_DU", BDBG_eMsg);*/
   /*BDBG_SetModuleLevel("BMUX_MP4_FINISH", BDBG_eMsg);*/
   /*BDBG_SetModuleLevel("BMUX_MP4_OUTPUT", BDBG_eMsg);*/
   /*BDBG_SetModuleLevel("BMUX_MP4_STATE", BDBG_eMsg);*/

#if VIDEO_ENABLED == 1
   /*BDBG_SetModuleLevel("VIDEO_ENCODER_STUB", BDBG_eMsg);*/
   /*BDBG_SetModuleLevel("VIDEO_ENCODER_STUB_NALU", BDBG_eMsg);*/
#endif
#if AUDIO_ENABLED == 1
   /*BDBG_SetModuleLevel("AUDIO_ENCODER_STUB", BDBG_eMsg);*/
#endif
   /* create a timer device for profiling */
   BDBG_MSG(("Opening free-run Timer ... "));
   {
      BTMR_Settings Settings = { BTMR_Type_eSharedFreeRun, NULL, NULL, 0, false };
      /* open a free-run timer */
      BTMR_CreateTimer(frmInfo.hTmr, &ghTimer, &Settings);
   }

#if VIDEO_ENABLED == 1
   /* Open Video Encoder Stub */
   BDBG_MSG(("Open Video Encoder"));

   BKNI_Memset(ahVideoEncoder, 0, sizeof(VideoEncoderHandle) * BMUXLIB_FILE_MP4_MAX_VIDEO_INPUTS );
   {
         VideoEncoderSettings stSettings;
         BKNI_Memset(&stSettings, 0, sizeof( VideoEncoderSettings ));
         stSettings.hMem = frmInfo.hMem;
         app_OpenVideoEncoder(&ahVideoEncoder[0], &stSettings);
   }
#endif

#if AUDIO_ENABLED == 1
   /* Open Audio Encoder Stub */
   BDBG_MSG(("Open Audio Encoder"));

   BKNI_Memset(ahAudioEncoder, 0, sizeof(AudioEncoderHandle) * BMUXLIB_FILE_MP4_MAX_AUDIO_INPUTS );
   {
         AudioEncoderSettings stSettings;
         BKNI_Memset(&stSettings, 0, sizeof(AudioEncoderSettings ));
         stSettings.hMem = frmInfo.hMem;
         app_OpenAudioEncoder(&ahAudioEncoder[0], &stSettings);
         BDBG_MSG(("Done Open Audio Encoder"));
   }
#endif
   BKNI_CreateEvent( &stHandles.hFinishedEvent );
   BKNI_CreateEvent( &stHandles.hInputDoneEvent );

/********************************************
   MP4 Mux Code goes here
********************************************/

   /* Create MP4 Mux */
   {
      BMUXlib_File_MP4_CreateSettings stCreateSettings;
      BDBG_MSG(("Creating Muxer"));

      BMUXlib_File_MP4_GetDefaultCreateSettings(&stCreateSettings);
      /* Nothing needs to be overridden right now */
      if (BERR_SUCCESS != BMUXlib_File_MP4_Create(&hMP4Mux,&stCreateSettings))
      {
         BDBG_ERR(("Failure Creating Mux"));
      }
   }

   /* Start Mux */
   {
      BMUXlib_File_MP4_StartSettings stStartSettings;
      struct timeval currentTime;

      BDBG_MSG(("Starting Mux"));
      BMUXlib_File_MP4_GetDefaultStartSettings(&stStartSettings);

      /* configure required settings */
      /* setup storage */
      if (BERR_SUCCESS != App_StorageCreate(&stStorageSystem))
      {
         BDBG_ERR(("Unable to create the storage system"));
      }
      stStartSettings.stStorage = stStorageSystem;

      /* create an output interface for the final output MP4 content */
      stOutput.pContext = (App_StorageInterfaceContext *)BKNI_Malloc(sizeof(App_StorageInterfaceContext));
      if (NULL == stOutput.pContext )
      {
         BDBG_ERR(("Unable to create the output storage"));
      }
      else
      {
         App_StorageInterfaceContext *pContext = stOutput.pContext;
         BKNI_Memset(pContext, 0, sizeof(App_StorageInterfaceContext));
         stOutput.pfAddDescriptors = (BMUXlib_AddStorageDescriptors)App_StorageInterfaceAddDescriptors;
         stOutput.pfGetCompleteDescriptors = (BMUXlib_GetCompletedStorageDescriptors)App_StorageInterfaceGetCompletedDescriptors;
         /* open the output file for r/w binary */
         strcpy(pContext->fname, "muxtest.mp4");
         BDBG_MODULE_MSG(mux_storage, ("Creating Storage for interface: %p (file: %s)", stOutput.pContext, pContext->fname));
         pContext->fp = fopen(pContext->fname, "w+b");
         if (NULL == pContext->fp)
            BDBG_ERR(("Unable to create output storage"));
         stStartSettings.stOutput = stOutput;
      }

      /* configure the inputs */
#if VIDEO_ENABLED == 1
      stStartSettings.stVideoInputs[0].uiTrackID = 1;
      stStartSettings.stVideoInputs[0].stInterface.pContext = ahVideoEncoder[0];
      stStartSettings.stVideoInputs[0].stInterface.fGetBufferDescriptors = (BMUXlib_GetVideoBufferDescriptors)app_GetVideoBufferDescriptors;
      stStartSettings.stVideoInputs[0].stInterface.fConsumeBufferDescriptors = (BMUXlib_ConsumeVideoBufferDescriptors)app_ConsumeVideoBufferDescriptors;
      stStartSettings.stVideoInputs[0].stInterface.fGetBufferStatus = (BMUXlib_GetVideoBufferStatus)app_GetVideoBufferStatus;
      /* FIXME: hard coded for now */
      stStartSettings.stVideoInputs[0].stInterface.stBufferInfo.eProtocol = BAVC_VideoCompressionStd_eH264;
      stStartSettings.uiNumVideoInputs = 1;
#endif

#if AUDIO_ENABLED == 1
      stStartSettings.stAudioInputs[0].uiTrackID = 2;
      stStartSettings.stAudioInputs[0].stInterface.pContext = ahAudioEncoder[0];
      stStartSettings.stAudioInputs[0].stInterface.fGetBufferDescriptors = (BMUXlib_GetAudioBufferDescriptors)app_GetAudioBufferDescriptors;
      stStartSettings.stAudioInputs[0].stInterface.fConsumeBufferDescriptors = (BMUXlib_ConsumeAudioBufferDescriptors)app_ConsumeAudioBufferDescriptors;
      stStartSettings.stAudioInputs[0].stInterface.fGetBufferStatus = (BMUXlib_GetAudioBufferStatus)app_GetAudioBufferStatus;
      /* FIXME: hard coded for now */
      stStartSettings.stAudioInputs[0].stInterface.stBufferInfo.eProtocol = BAVC_AudioCompressionStd_eAac;
      stStartSettings.uiNumAudioInputs = 1;
#endif

      /* configure rest of settings */
      stStartSettings.uiExpectedDurationMs = 0;
      stStartSettings.bProgressiveDownloadCompatible = true;
      stStartSettings.bUseEditList = true;
      gettimeofday(&currentTime, NULL);
      /* current time is returned as seconds since 00:00:00 UTC (Coordinated Universal Time), January 1 1970
         so convert to seconds since midnight Jan 1, 1904 as required by MP4.  Thus, this is a difference of
         66 years = 2,082,844,800 seconds */
      stStartSettings.uiCreateTimeUTC = currentTime.tv_sec + 2082844800;

      if (BERR_SUCCESS != BMUXlib_File_MP4_Start(hMP4Mux, &stStartSettings))
      {
         BDBG_ERR(("Failure to start Mux"));
      }
   }

   /* Start Domux Thread */
   {
      BDBG_MSG(("Starting Muxer Thread"));

      stHandles.hMP4Mux = hMP4Mux;
#if VIDEO_ENABLED == 1
      stHandles.uiVideoEncoderCount = BMUXLIB_FILE_MP4_MAX_VIDEO_INPUTS;
      stHandles.ahVideoEncoder = ahVideoEncoder;
#endif

#if AUDIO_ENABLED == 1
      stHandles.uiAudioEncoderCount = BMUXLIB_FILE_MP4_MAX_AUDIO_INPUTS;
      stHandles.ahAudioEncoder = ahAudioEncoder;
#endif
      BTST_CreateTask(&hTask, app_MuxTask, &stHandles);
   }

   /* wait for input to complete ... */
   if (BERR_SUCCESS != BKNI_WaitForEvent( stHandles.hInputDoneEvent, 5000 ))
   {
      BDBG_WRN(("Input Not Completed ... Forcing Finish!"));
   }
   else
      BDBG_MSG(("Input Done ... Finishing"));

   /* Finish() */
   {
      BMUXlib_File_MP4_FinishSettings stFinishSettings;
      BMUXlib_File_MP4_GetDefaultFinishSettings(&stFinishSettings);
      /* first stop the encoder */
#if VIDEO_ENABLED == 1
      BDBG_MSG(("Stop Video Encoder"));
      app_StopVideoEncoder(ahVideoEncoder[0]);
#endif
#if AUDIO_ENABLED == 1
      BDBG_MSG(("Stop Audio Encoder"));
      app_StopAudioEncoder(ahAudioEncoder[0]);
#endif
      /* then stop the mux */
      if (BERR_SUCCESS != BMUXlib_File_MP4_Finish(hMP4Mux, &stFinishSettings))
      {
         BDBG_ERR(("Failure to Finish mux"));
      }
   }
   /* when finished, stop the mux ... */
   BDBG_MSG(("Wait for Finish"));

   if (BERR_SUCCESS != BKNI_WaitForEvent( stHandles.hFinishedEvent, 1000 ))
   {
      BDBG_ERR(("Finish Event Timed Out!"));
   }

   /* Stop Mux */
   BDBG_MSG(("Stopping Mux"));
   BMUXlib_File_MP4_Stop(hMP4Mux);

   /* FIXME: May need to sleep in here to allow mux time to complete the stop() */
   /* Destroy Mux Thread */
   BDBG_MSG(("Destroy Mux Thread"));
   BTST_DestroyTask(hTask);

   BDBG_MSG(("Destroy Output Storage Interface"));
   {
      App_StorageInterfaceContext *pInterfaceContext = stOutput.pContext;
      BDBG_MODULE_MSG(mux_storage, ("Destroying Storage for Context: %p (file: %s)", pInterfaceContext->fname));
      /* if file open, close it */
      if (NULL != pInterfaceContext->fp)
         fclose(pInterfaceContext->fp);
      if (NULL != pInterfaceContext)
         BKNI_Free(pInterfaceContext);
      BKNI_Memset(&stOutput, 0, sizeof(BMUXlib_StorageObjectInterface));
   }

   BDBG_MSG(("Destroy Storage System"));
   App_StorageDestroy(&stStorageSystem);

   BDBG_MSG(("Destroy Muxer"));
   BMUXlib_File_MP4_Destroy(hMP4Mux);

/***********************************************

***********************************************/
#if VIDEO_ENABLED == 1
   /* Close Video Encoder Stub */
   BDBG_MSG(("Close Video Encoder"));
   app_CloseVideoEncoder(ahVideoEncoder[0]);
#endif

#if AUDIO_ENABLED == 1
   /* Close Audio Encoder Stub */
   BDBG_MSG(("Close Audio Encoder"));
   app_CloseAudioEncoder(ahAudioEncoder[0]);
#endif

   BDBG_MSG(("Destroy Finish Event"));
   BKNI_DestroyEvent( stHandles.hFinishedEvent );
   BDBG_MSG(("Destroy Input Done Event"));
   BKNI_DestroyEvent( stHandles.hInputDoneEvent );
   BDBG_MSG(("Destroy Free-Run Timer"));
	BTMR_DestroyTimer(ghTimer);
   BDBG_MSG(("Uninitialize Framework"));
   BFramework_Uninit(&frmInfo);
   BSystem_Uninit(&sysInfo);
   BDBG_MSG(("Done"));
   return iErr;
}

/************************************************
  DoMux() Thread
************************************************/
void app_MuxTask(BTST_TaskHandle hTask, void *pvContext)
{
   BERR_Code rc;
   TaskHandles *pstHandles = (TaskHandles*) pvContext;
   uint32_t i;

   BMUXlib_DoMux_Status stStatus;
   BKNI_Memset(&stStatus, 0, sizeof(BMUXlib_DoMux_Status));

   BSTD_UNUSED(hTask);

   do
   {
      BDBG_MSG(("Executing Mux Thread"));
      BKNI_EnterCriticalSection();

      rc = BMUXlib_File_MP4_DoMux(pstHandles->hMP4Mux, &stStatus);

      BKNI_LeaveCriticalSection();
      BDBG_MSG(("Do Mux returning: 0x%x (state: %d)", rc, stStatus.eState));

      /* no need to have any sort of time delay ... this can run AFAP */
      /*BKNI_Sleep(stStatus.uiNextExecutionTime);*/

      if (BMUXlib_State_eStarted == stStatus.eState)
      {
#if VIDEO_ENABLED == 1
         for (i = 0; i < pstHandles->uiVideoEncoderCount; i++ )
         {
            if ( NULL != pstHandles->ahVideoEncoder[i] )
            {
               app_VideoEncoderIncrementTime(pstHandles->ahVideoEncoder[i], APP_SIMULATION_TIME_INCREMENT);
               if (app_IsInputDone(pstHandles->ahVideoEncoder[i]))
                  BKNI_SetEvent_isr(pstHandles->hInputDoneEvent);
            }
         }
#endif
#if AUDIO_ENABLED == 1
         for (i = 0; i < pstHandles->uiAudioEncoderCount; i++ )
         {
            if ( NULL != pstHandles->ahAudioEncoder[i] )
            {
               app_AudioEncoderIncrementTime(pstHandles->ahAudioEncoder[i], APP_SIMULATION_TIME_INCREMENT);
            }
         }
#endif
      }
   } while (BMUXlib_State_eFinished != stStatus.eState);
   BKNI_SetEvent_isr(pstHandles->hFinishedEvent);
}
