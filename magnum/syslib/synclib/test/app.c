
#include <stdio.h>
#include <stdlib.h>

#include "bsyslib_impl.h"
#include "bsynclib.h"
#include "framework.h"

BDBG_MODULE(app);

BERR_Code app_video_source_set_mute
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	bool bMute /* true means mute, false means unmute */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(app_video_source_set_mute);
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);
	BDBG_MSG(("uiDeviceIndex = %u, bMute = %s", uiDeviceIndex, bMute ? "true" : "false"));
	BDBG_LEAVE(app_video_source_set_mute);
	return rc;
}

BERR_Code app_video_source_set_delay
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	BSYNClib_UnsignedValue * psDelay /* the delay with which to program the device */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(app_video_source_set_delay);
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);
	BDBG_MSG(("uiDeviceIndex = %u, uiDelay = %u", uiDeviceIndex, psDelay->uiValue));
	BDBG_LEAVE(app_video_source_set_delay);
	return rc;
}

BERR_Code app_video_source_set_delay_notification
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	bool bEnable, /* true means enable delay notification, false means disable it */
	BSYNClib_UnsignedValue * psThreshold /* delay notification threshold when enabled */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(app_video_source_set_delay_notification);
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);
	BDBG_MSG(("uiDeviceIndex = %u, bEnable = %s, uiThreshold = %u", uiDeviceIndex, bEnable ? "true" : "false", psThreshold->uiValue));
	BDBG_LEAVE(app_video_source_set_delay_notification);
	return rc;
}

BERR_Code app_video_sink_set_delay
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	BSYNClib_UnsignedValue * psDelay /* the delay with which to program the device */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(app_video_sink_set_delay);
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);
	BDBG_MSG(("uiDeviceIndex = %u, uiDelay = %u", uiDeviceIndex, psDelay->uiValue));
	BDBG_LEAVE(app_video_sink_set_delay);
	return rc;
}

BERR_Code app_video_sink_set_delay_notification
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	bool bEnable, /* true means enable delay notification, false means disable it */
	BSYNClib_UnsignedValue * psThreshold /* delay notification threshold when enabled */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(app_video_sink_set_delay_notification);
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);
	BDBG_MSG(("uiDeviceIndex = %u, bEnable = %s, uiThreshold = %u", uiDeviceIndex, bEnable ? "true" : "false", psThreshold->uiValue));
	BDBG_LEAVE(app_video_sink_set_delay_notification);
	return rc;
}

BERR_Code app_audio_source_set_mute
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	bool bMute /* true means mute, false means unmute */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(app_audio_source_set_mute);
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);
	BDBG_MSG(("uiDeviceIndex = %u, bMute = %s", uiDeviceIndex, bMute ? "true" : "false"));
	BDBG_LEAVE(app_audio_source_set_mute);
	return rc;
}

BERR_Code app_audio_source_set_delay
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	BSYNClib_UnsignedValue * psDelay /* the delay with which to program the device */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(app_audio_source_set_delay);
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);
	BDBG_MSG(("uiDeviceIndex = %u, uiDelay = %u", uiDeviceIndex, psDelay->uiValue));
	BDBG_LEAVE(app_audio_source_set_delay);
	return rc;
}

BERR_Code app_audio_source_set_delay_notification
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	bool bEnable, /* true means enable delay notification, false means disable it */
	BSYNClib_UnsignedValue * psThreshold /* delay notification threshold when enabled */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(app_audio_source_set_delay_notification);
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);
	BDBG_MSG(("uiDeviceIndex = %u, bEnable = %s, uiThreshold = %u", uiDeviceIndex, bEnable ? "true" : "false", psThreshold->uiValue));
	BDBG_LEAVE(app_audio_source_set_delay_notification);
	return rc;
}

BERR_Code app_audio_sink_set_delay
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	BSYNClib_UnsignedValue * psDelay /* the delay with which to program the device */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(app_audio_sink_set_delay);
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);
	BDBG_MSG(("uiDeviceIndex = %u, uiDelay = %u", uiDeviceIndex, psDelay->uiValue));
	BDBG_LEAVE(app_audio_sink_set_delay);
	return rc;
}

BERR_Code app_audio_sink_set_delay_notification
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	bool bEnable, /* true means enable delay notification, false means disable it */
	BSYNClib_UnsignedValue * psThreshold /* delay notification threshold when enabled */
)
{
	BERR_Code rc = BERR_SUCCESS;
	BDBG_ENTER(app_audio_sink_set_delay_notification);
	BSTD_UNUSED(pvParm1);
	BSTD_UNUSED(iParm2);
	BDBG_MSG(("uiDeviceIndex = %u, bEnable = %s, uiThreshold = %u", uiDeviceIndex, bEnable ? "true" : "false", psThreshold->uiValue));
	BDBG_LEAVE(app_audio_sink_set_delay_notification);
	return rc;
}

int app_main(int argc, char * argv[])
{
	BERR_Code rc = BERR_SUCCESS;
	BSYSlib_OS_Context_Handle hContext;
	BSYNClib_Settings sLibSettings;
	BSYNClib_Handle hSync;
	BSYNClib_Channel_Settings sChnSettings;
	BSYNClib_Channel_Handle hChn;
	BSYNClib_Channel_Config sConfig;
	BSYNClib_VideoSource_Config sVideoSourceConfig;
	BSYNClib_VideoSink_Config sVideoSinkConfig;
	BSYNClib_AudioSource_Config sAudioSourceConfig;
	BSYNClib_AudioSink_Config sAudioSinkConfig;
	
	BSystem_Info sysInfo;
	BFramework_Info frmInfo;

	int iErr = BSystem_Init( argc, argv, &sysInfo );
	if ( iErr )
	{
		printf( "System init FAILED!\n" );
		goto err_sysinit;
	}

	iErr = BFramework_Init( &sysInfo, &frmInfo );
	if ( iErr )
	{
		printf( "Framework init FAILED!\n" );
		goto err_fwinit;
	}

	BDBG_SetModuleLevel("syslib_impl", BDBG_eMsg);
	BDBG_SetModuleLevel("syslib_list", BDBG_eErr);
	BDBG_SetModuleLevel("synclib", BDBG_eMsg);
	BDBG_SetModuleLevel("app", BDBG_eErr);

	rc = BSYSlib_OS_Context_Create(&hContext);
	if (rc) goto err_context;

	rc = BSYSlib_OS_Context_Start(hContext);
	if (rc) goto err_context;

	printf("\n\nThis is a simulation of the sequence of calls made to bsettop_sync in SETAPI by using brutus on the Sarnoff 480i lipsync stream.\n");
	printf("Press enter to begin simulation.  We start in HD/SD Single PIG mode with MAD enabled.\n\n");
	getchar();

	BSYNClib_GetDefaultSettings(&sLibSettings);

	rc = BSYNClib_Open(&sLibSettings, &hSync);
	if (rc) goto err_synclib;

	BSYNClib_GetChannelDefaultSettings(&sChnSettings);

	sChnSettings.pvUserCallbackContextParm1 = hContext;
	sChnSettings.iUserCallbackContextParm2 = 0;
	sChnSettings.pfCreateTimer = &BSYSlib_Impl_Timer_Create;
	sChnSettings.pfDestroyTimer = &BSYSlib_Impl_Timer_Destroy;
#ifdef BSYSLIB_TASK_SUPPORT
	sChnSettings.pfScheduleTask_isr = &BSYSlib_Impl_Task_Schedule_isr;
#endif
	sChnSettings.pfStartTimer_isr = &BSYSlib_Impl_Timer_Start_isr;
	sChnSettings.pfCancelTimer_isr = &BSYSlib_Impl_Timer_Cancel_isr;
	sChnSettings.pfGetTime_isr = &BSYSlib_Impl_GetTime_isr;

	sChnSettings.sVideo.sSource.pfSetMute = &app_video_source_set_mute;
	sChnSettings.sVideo.sSource.pfSetDelay = &app_video_source_set_delay;
	sChnSettings.sVideo.sSource.pfSetDelayNotification = &app_video_source_set_delay_notification;
	sChnSettings.sVideo.sSink.pfSetDelay = &app_video_sink_set_delay;
	sChnSettings.sVideo.sSink.pfSetDelayNotification = &app_video_sink_set_delay_notification;
	sChnSettings.sAudio.sSource.pfSetMute = &app_audio_source_set_mute;
	sChnSettings.sAudio.sSource.pfSetDelay = &app_audio_source_set_delay;
	sChnSettings.sAudio.sSource.pfSetDelayNotification = &app_audio_source_set_delay_notification;
	sChnSettings.sAudio.sSink.pfSetDelay = &app_audio_sink_set_delay;
	sChnSettings.sAudio.sSink.pfSetDelayNotification = &app_audio_sink_set_delay_notification;

	rc = BSYNClib_CreateChannel(hSync, &sChnSettings, &hChn);
	if (rc) goto err_synclib;

	/* set up sources and sinks */
	BSYNClib_Channel_GetConfig(hChn, &sConfig);
	sConfig.uiVideoSourceCount = 1;
	sConfig.uiVideoSinkCount = 2;
	sConfig.uiAudioSourceCount = 2;
	sConfig.uiAudioSinkCount = 4;
	rc = BSYNClib_Channel_SetConfig(hChn, &sConfig);
	if (rc) goto err_synclib;

	/* display 0 format change */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	sVideoSinkConfig.sFormat.bReceived = true;
	sVideoSinkConfig.sFormat.uiHeight = 480;
	sVideoSinkConfig.sFormat.bInterlaced = true;
	sVideoSinkConfig.sFormat.eFrameRate = BAVC_FrameRateCode_e29_97;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* display 1 format change */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	sVideoSinkConfig.sFormat.bReceived = true;
	sVideoSinkConfig.sFormat.uiHeight = 480;
	sVideoSinkConfig.sFormat.bInterlaced = true;
	sVideoSinkConfig.sFormat.eFrameRate = BAVC_FrameRateCode_e29_97;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* start dac output */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSinkConfig_isr(hChn, 0, &sAudioSinkConfig);
	sAudioSinkConfig.bSynchronize = true;
	sAudioSinkConfig.bCompressed = false;
	sAudioSinkConfig.sDelay.bReceived = true;
	sAudioSinkConfig.sDelay.sMeasured.uiValue = 0;
	sAudioSinkConfig.sDelay.sCustom.uiValue = 0;
	rc = BSYNClib_Channel_SetAudioSinkConfig_isr(hChn, 0, &sAudioSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* start i2s output */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSinkConfig_isr(hChn, 1, &sAudioSinkConfig);
	sAudioSinkConfig.bSynchronize = true;
	sAudioSinkConfig.bCompressed = false;
	sAudioSinkConfig.sDelay.bReceived = true;
	sAudioSinkConfig.sDelay.sMeasured.uiValue = 0;
	sAudioSinkConfig.sDelay.sCustom.uiValue = 0;
	rc = BSYNClib_Channel_SetAudioSinkConfig_isr(hChn, 1, &sAudioSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* start hdmi(pcm) output */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSinkConfig_isr(hChn, 2, &sAudioSinkConfig);
	sAudioSinkConfig.bSynchronize = true;
	sAudioSinkConfig.bCompressed = false;
	sAudioSinkConfig.sDelay.bReceived = true;
	sAudioSinkConfig.sDelay.sMeasured.uiValue = 0;
	sAudioSinkConfig.sDelay.sCustom.uiValue = 0;
	rc = BSYNClib_Channel_SetAudioSinkConfig_isr(hChn, 2, &sAudioSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* start spdif(cmp) output */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSinkConfig_isr(hChn, 3, &sAudioSinkConfig);
	sAudioSinkConfig.bSynchronize = true;
	sAudioSinkConfig.bCompressed = true;
	sAudioSinkConfig.sDelay.bReceived = true;
	sAudioSinkConfig.sDelay.sMeasured.uiValue = 0;
	sAudioSinkConfig.sDelay.sCustom.uiValue = 0;
	rc = BSYNClib_Channel_SetAudioSinkConfig_isr(hChn, 3, &sAudioSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* open decode window 0 */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	sVideoSinkConfig.bSynchronize = false;
	sVideoSinkConfig.bFullScreen = false;
	sVideoSinkConfig.bVisible = true;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* open decode window 1 */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	sVideoSinkConfig.bSynchronize = false;
	sVideoSinkConfig.bFullScreen = false;
	sVideoSinkConfig.bVisible = true;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* display 0 format change */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	sVideoSinkConfig.sFormat.bReceived = true;
	sVideoSinkConfig.sFormat.uiHeight = 480;
	sVideoSinkConfig.sFormat.bInterlaced = true;
	sVideoSinkConfig.sFormat.eFrameRate = BAVC_FrameRateCode_e29_97;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* display 1 format change */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	sVideoSinkConfig.sFormat.bReceived = true;
	sVideoSinkConfig.sFormat.uiHeight = 480;
	sVideoSinkConfig.sFormat.bInterlaced = true;
	sVideoSinkConfig.sFormat.eFrameRate = BAVC_FrameRateCode_e29_97;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* resize decode window 0 */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	sVideoSinkConfig.bFullScreen = false;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* resize decode window 1 */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	sVideoSinkConfig.bFullScreen = false;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* display 0 format change */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	sVideoSinkConfig.sFormat.bReceived = true;
	sVideoSinkConfig.sFormat.uiHeight = 1080;
	sVideoSinkConfig.sFormat.bInterlaced = true;
	sVideoSinkConfig.sFormat.eFrameRate = BAVC_FrameRateCode_e30;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* display 1 format change */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	sVideoSinkConfig.sFormat.bReceived = true;
	sVideoSinkConfig.sFormat.uiHeight = 480;
	sVideoSinkConfig.sFormat.bInterlaced = true;
	sVideoSinkConfig.sFormat.eFrameRate = BAVC_FrameRateCode_e29_97;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* validate decode window 0 */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	sVideoSinkConfig.bSynchronize = true;
	sVideoSinkConfig.bForcedCaptureEnabled = true;
	sVideoSinkConfig.bMasterFrameRateEnabled = true;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* receive window 0 post proc delay */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	sVideoSinkConfig.sDelay.bReceived = true;
	sVideoSinkConfig.sDelay.sMeasured.uiValue = 2; /* received 2 vsyncs */
	sVideoSinkConfig.sDelay.sMeasured.eUnits = BSYNClib_Units_e60HzVsyncs;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* validate decode window 1 */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	sVideoSinkConfig.bSynchronize = true;
	sVideoSinkConfig.bForcedCaptureEnabled = true;
	sVideoSinkConfig.bMasterFrameRateEnabled = true;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	printf("\n\nPress enter to simulate start of decode: MPEG2 Video, AC-3 Audio\n\n");
	getchar();

	/* start video 0 decode */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	sVideoSourceConfig.bStarted = true;
	sVideoSourceConfig.bDigital = true;
	sVideoSourceConfig.bLastPictureHeld = false;
	sVideoSourceConfig.sDelay.sCustom.uiValue = 0;
	rc = BSYNClib_Channel_SetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* start audio 0 decode */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	sAudioSourceConfig.bStarted = true;
	sAudioSourceConfig.bDigital = true;
	sAudioSourceConfig.sDelay.bReceived = true;
	sAudioSourceConfig.sDelay.sMeasured.uiValue = 144;
	sAudioSourceConfig.sDelay.sCustom.uiValue = 0;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* start audio 1 passthrough */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 1, &sAudioSourceConfig);
	sAudioSourceConfig.bStarted = true;
	sAudioSourceConfig.bDigital = true;
	sAudioSourceConfig.sDelay.bReceived = true;
	sAudioSourceConfig.sDelay.sMeasured.uiValue = 144;
	sAudioSourceConfig.sDelay.sCustom.uiValue = 0;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 1, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* receive window 1 post proc delay */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	sVideoSinkConfig.sDelay.bReceived = true;
	sVideoSinkConfig.sDelay.sMeasured.uiValue = 3; /* received 3 vsyncs */
	sVideoSinkConfig.sDelay.sMeasured.eUnits = BSYNClib_Units_e60HzVsyncs;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* receive audio 0 sampling rate */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	sAudioSourceConfig.bSamplingRateReceived = true;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* receive audio 1 sampling rate */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 1, &sAudioSourceConfig);
	sAudioSourceConfig.bSamplingRateReceived = true;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 1, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* receive video 0 picture params */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	sVideoSourceConfig.sFormat.bReceived = true;
	sVideoSourceConfig.sFormat.uiHeight = 480;
	sVideoSourceConfig.sFormat.bInterlaced = true;
	sVideoSourceConfig.sFormat.eFrameRate = BAVC_FrameRateCode_e29_97;
	rc = BSYNClib_Channel_SetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* todo: receive video 0 picture params every picture */
		
	/* receive video 0 decode delay */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	sVideoSourceConfig.sDelay.bReceived = true;
	sVideoSourceConfig.sDelay.sMeasured.uiValue = 0;
	sVideoSourceConfig.sDelay.sMeasured.eUnits = BSYNClib_Units_e45KhzTicks;
	rc = BSYNClib_Channel_SetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	printf("\n\nPress enter to simulate switch to full screen\n\n");
	getchar();

	/* resize decode window 0 */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	sVideoSinkConfig.bFullScreen = true;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* resize decode window 1 */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	sVideoSinkConfig.bFullScreen = true;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* receive window 0 post proc delay */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	sVideoSinkConfig.sDelay.bReceived = true;
	sVideoSinkConfig.sDelay.sMeasured.uiValue = 5; /* received 5 vsyncs */
	sVideoSinkConfig.sDelay.sMeasured.eUnits = BSYNClib_Units_e60HzVsyncs;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* receive video 0 decode delay */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	sVideoSourceConfig.sDelay.bReceived = true;
	sVideoSourceConfig.sDelay.sMeasured.uiValue = 360;
	sVideoSourceConfig.sDelay.sMeasured.eUnits = BSYNClib_Units_e45KhzTicks;
	rc = BSYNClib_Channel_SetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	printf("\n\nPress enter to simulate channel change to nonexistant channel\n\n");
	getchar();

	/* stop audio 0 decode */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	sAudioSourceConfig.bStarted = false;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* stop audio 1 passthrough */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 1, &sAudioSourceConfig);
	sAudioSourceConfig.bStarted = false;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 1, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* stop video 0 decode */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	sVideoSourceConfig.bStarted = false;
	rc = BSYNClib_Channel_SetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	printf("\n\nPress enter to simulate channel change back to first channel\n\n");
	getchar();

	/* start video 0 decode */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	sVideoSourceConfig.bStarted = true;
	sVideoSourceConfig.bDigital = true;
	sVideoSourceConfig.bLastPictureHeld = false;
	sVideoSourceConfig.sDelay.sCustom.uiValue = 0;
	rc = BSYNClib_Channel_SetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* start audio 0 decode */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	sAudioSourceConfig.bStarted = true;
	sAudioSourceConfig.bDigital = true;
	sAudioSourceConfig.sDelay.bReceived = true;
	sAudioSourceConfig.sDelay.sMeasured.uiValue = 144;
	sAudioSourceConfig.sDelay.sCustom.uiValue = 0;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* start audio 1 passthrough */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 1, &sAudioSourceConfig);
	sAudioSourceConfig.bStarted = true;
	sAudioSourceConfig.bDigital = true;
	sAudioSourceConfig.sDelay.bReceived = true;
	sAudioSourceConfig.sDelay.sMeasured.uiValue = 144;
	sAudioSourceConfig.sDelay.sCustom.uiValue = 0;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 1, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* receive audio 0 sampling rate */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	sAudioSourceConfig.bSamplingRateReceived = true;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* receive video 0 picture params */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	sVideoSourceConfig.sFormat.bReceived = true;
	sVideoSourceConfig.sFormat.uiHeight = 480;
	sVideoSourceConfig.sFormat.bInterlaced = true;
	sVideoSourceConfig.sFormat.eFrameRate = BAVC_FrameRateCode_e29_97;
	rc = BSYNClib_Channel_SetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* todo: receive video 0 picture params every picture */

	/* receive video 0 decode delay */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	sVideoSourceConfig.sDelay.bReceived = true;
	sVideoSourceConfig.sDelay.sMeasured.uiValue = 0;
	sVideoSourceConfig.sDelay.sMeasured.eUnits = BSYNClib_Units_e60HzVsyncs;
	rc = BSYNClib_Channel_SetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* some time goes by */
	
	/* receive video 0 decode delay */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	sVideoSourceConfig.sDelay.bReceived = true;
	sVideoSourceConfig.sDelay.sMeasured.uiValue = 360;
	sVideoSourceConfig.sDelay.sMeasured.eUnits = BSYNClib_Units_e60HzVsyncs;
	rc = BSYNClib_Channel_SetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	printf("\n\nPress enter to simulate quit\n\n");
	getchar();

	/* stop audio 0 decode */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	sAudioSourceConfig.bStarted = false;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 0, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* stop audio 1 passthrough */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 1, &sAudioSourceConfig);
	sAudioSourceConfig.bStarted = false;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 1, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* stop video 0 decode */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	sVideoSourceConfig.bStarted = false;
	rc = BSYNClib_Channel_SetVideoSourceConfig_isr(hChn, 0, &sVideoSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* close decode window 0 */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	sVideoSinkConfig.bSynchronize = false;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 0, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
	
	/* close decode window 1 */
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	sVideoSinkConfig.bSynchronize = false;
	rc = BSYNClib_Channel_SetVideoSinkConfig_isr(hChn, 1, &sVideoSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;

	/* stop dac output */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSinkConfig_isr(hChn, 0, &sAudioSinkConfig);
	sAudioSinkConfig.bSynchronize = false;
	rc = BSYNClib_Channel_SetAudioSinkConfig_isr(hChn, 0, &sAudioSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* stop i2s output */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSinkConfig_isr(hChn, 1, &sAudioSinkConfig);
	sAudioSinkConfig.bSynchronize = false;
	rc = BSYNClib_Channel_SetAudioSinkConfig_isr(hChn, 1, &sAudioSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* stop hdmi(pcm) output */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSinkConfig_isr(hChn, 2, &sAudioSinkConfig);
	sAudioSinkConfig.bSynchronize = false;
	rc = BSYNClib_Channel_SetAudioSinkConfig_isr(hChn, 2, &sAudioSinkConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	/* stop spdif(cmp) output */	
	BKNI_EnterCriticalSection();
	BSYNClib_Channel_GetAudioSourceConfig_isr(hChn, 3, &sAudioSourceConfig);
	sAudioSinkConfig.bSynchronize = false;
	rc = BSYNClib_Channel_SetAudioSourceConfig_isr(hChn, 3, &sAudioSourceConfig);
	BKNI_LeaveCriticalSection();
	if (rc) goto err_synclib;
		
	BSYNClib_DestroyChannel(hSync, hChn);

	BSYNClib_Close(hSync);

	BSYSlib_OS_Context_Destroy(hContext);
	BFramework_Uninit( &frmInfo );
	BSystem_Uninit( &sysInfo );

	goto end;

err_synclib:

err_sysinit:

err_fwinit:

err_context:

end:

	return (int)rc;
}

