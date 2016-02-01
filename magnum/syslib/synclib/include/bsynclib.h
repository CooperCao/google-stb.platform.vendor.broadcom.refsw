/***************************************************************************
*     Copyright (c) 2004-2013, Broadcom Corporation
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
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/

#include "bstd.h"
#include "bavc.h"
#include "bsyslib.h"

#ifndef BSYNCLIB_H__
#define BSYNCLIB_H__

/*
Summary:
A handle to the global/singleton sync module
Description:
*/
typedef struct BSYNClib_Impl * BSYNClib_Handle;

/*
Summary:
A handle to a sync channel.  A sync channel manages the following:
1 video decode
up to 2 audio decodes (same content decode + passthru)
up to N windows displaying the same content on different displays (1 window per display)
up to N audio displays with the same content (either decoded or passed through) and independent delays
Description:
*/
typedef struct BSYNClib_Channel_Impl * BSYNClib_Channel_Handle;

/*
Summary:
Global settings for lip sync.
*/
typedef struct
{
	bool bEnabled; /* global enable for sync, defaults to true */
	struct
	{
		bool bRequireFullScreen; /* is full screen video required before we sync? Defaults to true. */
		unsigned int uiTsmLockTimeout; /* time to wait after receiving a video TSM callback before accepting the PTSSTCDiff value */
		unsigned int uiUnmuteTimeout; /* time to wait before unmuting video after adjusting the PTS offset */
		unsigned int uiUnconditionalUnmuteTimeout; /* time to wait before unconditionally unmuting video (in case of error with audio sample rate callback or failure to start audio) */

		struct
		{
			unsigned int uiTimeout; /* time to wait before checking the number of callbacks received vs the acceptable callback count for rate mismatch */
			unsigned int uiAcceptableMtbcLower; /* acceptable mean time between callbacks lower bound */
			unsigned int uiAcceptableMtbcUpper; /* acceptable mean time between callbacks upper bound */
			unsigned int uiAcceptableTtlc; /* acceptable time to last callback for mismatches */
		} sRateMismatchDetection;
		struct
		{
			unsigned int uiTimeout; /* time to wait before checking the number of callbacks received vs the acceptable callback count for rate rematch */
			unsigned int uiAcceptableTtlc; /* acceptable time to last callback for rematches */
		} sRateRematchDetection;
	} sVideo;
	struct
	{
		unsigned int uiUnmuteTimeout; /* time to wait before unmuting audio after accepting video TSM callback */
		unsigned int uiReceiverDelayCompensation; /* compensate for external audio receiver delay */
		unsigned int uiUnconditionalUnmuteTimeout; /* time to wait before unconditionally unmuting audio (in case of error with video TSM callback or failure to start video) */
	} sAudio;
} BSYNClib_Settings;

/*
Summary:
Units of various measurements and parameters passed to synclib
*/
typedef enum BSYNClib_Units
{
	BSYNClib_Units_eMilliseconds = 0,
	BSYNClib_Units_e24HzVsyncs, /* TODO: change 24,50,60 to just generic VSYNCS, use rate in config to find out rate */
	BSYNClib_Units_e50HzVsyncs,
	BSYNClib_Units_e60HzVsyncs,
	BSYNClib_Units_e45KhzTicks,
	BSYNClib_Units_e90KhzTicks,
	BSYNClib_Units_e27MhzTicks,
	BSYNClib_Units_eMax = 0x7fffffff
} BSYNClib_Units;

/*
Summary:
An unsigned value with its units
*/
typedef struct BSYNClib_UnsignedValue
{
	unsigned int uiValue;
	BSYNClib_Units eUnits;
} BSYNClib_UnsignedValue;

/*
Summary:
Set mute state callback delegate
Description:
Sync expects the system to set the mute state of the device with the specifed
index (and named in the callback name) to the specified mute state.
This needs to happen before returning from the callback.
*/
typedef BERR_Code (*BSYNClib_Channel_SetMute)
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	bool bMute /* true means mute, false means unmute */
);

/*
Summary:
Encapsulation of mute state callback function and context
*/
typedef struct BSYNClib_Channel_MuteCallback
{
	BSYNClib_Channel_SetMute pfSetMute;
	void * pvParm1;
	int iParm2;
} BSYNClib_Channel_MuteCallback;

/*
Summary:
Set delay callback.
Description:
Sync expects the system to set the delay of the device with the specifed index
(and named in the callback name) to the specified delay.
This needs to happen before returning from the callback.
*/
typedef BERR_Code (*BSYNClib_Channel_SetDelay)
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	BSYNClib_UnsignedValue * psDelay /* the delay with which to program the device */
);

/*
Summary:
Set delay notification state callback.
Description:
Sync expects the system to set the delay notification of the device specified
(and named in the callback name) to the specified delay notification state.
This needs to happen before returning from the callback.
bEnable = true means that delay notification is enabled.
bEnable = false means that delay notification is disabled.
*/
typedef BERR_Code (*BSYNClib_Channel_SetDelayNotification)
(
	void * pvParm1, /* first user context parameter */
	int iParm2, /* second user context parameter */
	unsigned int uiDeviceIndex, /* device index */
	bool bEnable, /* true means enable delay notification, false means disable it */
	BSYNClib_UnsignedValue * psThreshold /* delay notification threshold when enabled */
);

/*
Summary:
Encapsulation of delay and delay notification state callback functions and context
*/
typedef struct BSYNClib_Channel_DelayCallback
{
	BSYNClib_Channel_SetDelay pfSetDelay;
	BSYNClib_Units preferredDelayUnits;
	BSYNClib_Channel_SetDelayNotification pfSetDelayNotification;
	BSYNClib_Units preferredNotificationThresholdUnits;
	void * pvParm1;
	int iParm2;
} BSYNClib_Channel_DelayCallback;

/*
Summary:
Sync channel settings.	Currently this encapsulates a multitude of callbacks
*/
typedef struct
{
	BSYSlib_TimerCallback cbTimer; /* timer-related callbacks */
	BSYSlib_TimeCallback cbTime; /* system-time-related callbacks */

	struct
	{
		struct
		{
			BSYNClib_Channel_MuteCallback cbMute;
			BSYNClib_Channel_DelayCallback cbDelay;
		} sSource;
		struct
		{
			BSYNClib_Channel_DelayCallback cbDelay;
		} sSink;
	} sVideo;
	struct
	{
		struct
		{
			BSYNClib_Channel_MuteCallback cbMute;
			BSYNClib_Channel_DelayCallback cbDelay;
		} sSource;
		struct
		{
			BSYNClib_Channel_DelayCallback cbDelay;
		} sSink;
	} sAudio;
} BSYNClib_Channel_Settings;

/*
Summary:
The information required by the sync algorithms from the system.  Messages passed into the sync module are level-based.
*/
typedef struct
{
	bool bEnabled; /* is this sync channel enabled? Defaults to true. */
	bool bPrecisionLipsyncEnabled; /* are subframe audio adjustments based on video feedback enabled? Defaults to true. */
	struct
	{
		bool bEnabled; /* when this is enabled, sync will attempt to mute audio and video
		while performing adjustments and unmute when finished.  This requires that the
		source counts below accurately reflect exactly which of the decoders you plan
		on using for this session.  For audio-only sessions, set the video source count to zero.
		For video-only sessions, set the audio source count to zero. Defaults to false. */
		bool bSimultaneousUnmute; /* this will cause all devices to unmute at the
		same time.  This time will be the largest of all of the unmute timout values
		set amongst the devices (excluding the audio unconditional unmute). Defaults to false. */
		bool bAllowIncrementalStart; /* defaults false, which requires all audio
		and video sync configuration before start. If true, you can
		incrementally configure sync for audio and video, but audio-only or
        video-only streams will be muted for 5 second timeout, which is not
        recommended. */
	} sMuteControl;
	unsigned int uiVideoSourceCount; /* how many video sources are there, defaults to 0. */
	unsigned int uiVideoSinkCount; /* how many video sinks are there, defaults to 0. */
	unsigned int uiAudioSourceCount; /* how many audio sources are there, defaults to 0. */
	unsigned int uiAudioSinkCount; /* how many audio sinks are there, defaults to 0. */
} BSYNClib_Channel_Config;

/*
Summary:
Information about a video source (decoder or analog/hdmi input)
*/
typedef struct
{
	bool bStarted; /* has this delay path element been started? */
	bool bDigital; /* is the video source digital or analog */
	bool bSynchronize; /* should this element be synchronized? */
	bool bNonRealTime; /* are we conducting a non-real-time session? */

	struct
	{
		bool bReceived; /* was a delay notification received this time (self-clearing) */
		BSYNClib_UnsignedValue sMeasured; /* measured delay, defaults to 45 Khz ticks */
		BSYNClib_UnsignedValue sCustom; /* delay supplied by user that isn't in the normal path delay, defaults to ms */
	} sDelay;

	struct
	{
		bool bReceived; /* did we received the format on this call? (self-clearing) */
		unsigned int uiHeight; /* height of format, required to predict VDC MAD state changes */
		bool bInterlaced; /* whether the format is interlaced */
		BAVC_FrameRateCode eFrameRate; /* the frame rate of the format */
	} sFormat;

	bool bLastPictureHeld; /* is the decoder holding the last picture or blanking? */
	BSYNClib_UnsignedValue sJitterToleranceImprovementThreshold; /* how much should the pts/stc difference change before we attempt jitter tolerance improvement, defaults to ms */
} BSYNClib_VideoSource_Config;

/*
Summary:
Information about a video sink (window and display)
*/
typedef struct
{
	bool bSynchronize; /* should this element be synchronized? */

	struct
	{
		bool bReceived; /* was a delay notification received this time (self-clearing) */
		BSYNClib_UnsignedValue sMeasured; /* measured delay, defaults to vsyncs */
		BSYNClib_UnsignedValue sCustom; /* delay supplied by user that isn't in the normal path delay in ms */
		BSYNClib_UnsignedValue sMax; /* max applicable delay supported by memory allocated to this sink, defaults to 4 vsyncs */
		/* TODO: add flag for drift callback so we can filter some values out */
	} sDelay;

	struct
	{
		bool bReceived; /* did we received the format on this call? (self-clearing) */
		unsigned int uiHeight; /* height of format, required to predict VDC MAD state changes */
		bool bInterlaced; /* whether the format is interlaced */
		BAVC_FrameRateCode eFrameRate; /* the frame rate of the format */
	} sFormat;

	bool bForcedCaptureEnabled; /* is forced capture enabled on this window */
	bool bMasterFrameRateEnabled; /* is master frame rate enabled on the main window for this display */
	bool bFullScreen; /* does window rect match display rect? */
	bool bVisible; /* is this window visible? */
    bool bSyncLocked; /* is this window sync-locked */
} BSYNClib_VideoSink_Config;

/*
Summary:
Information about an audio source (decoder or analog/hdmi input)
*/
typedef struct
{
	bool bStarted; /* has this delay path element been started? */
	bool bDigital; /* is the audio source digital or analog */
	bool bSynchronize; /* should this element be synchronized? */

	struct
	{
		bool bReceived; /* was a delay notification received this time (self-clearing) */
		BSYNClib_UnsignedValue sMeasured; /* measured delay, defaults to ms */
		BSYNClib_UnsignedValue sCustom; /* delay supplied by user that isn't in the normal path delay in ms */
	} sDelay;

	bool bSamplingRateReceived; /* has the audio sampling rate callback been received since audio start? (self-clearing) */
} BSYNClib_AudioSource_Config;

/*
Summary:
Information about an audio sink (output port)
*/
typedef struct
{
	bool bSynchronize; /* should this element be synchronized? */
	bool bCompressed; /* is the audio sink compressed */

	struct
	{
		bool bReceived; /* was a delay notification received this time (self-clearing) */
		BSYNClib_UnsignedValue sMeasured; /* measured delay, defaults to ms */
		BSYNClib_UnsignedValue sCustom; /* delay supplied by user that isn't in the normal path delay in ms */
	} sDelay;

	/* TODO: more analog params expected for DTV projects */
	unsigned int uiSamplingRate; /* the audio sampling rate in Hz (for analog) */
} BSYNClib_AudioSink_Config;

/*
Summary:
Sync channel status
Description:
*/
typedef struct
{
	struct
	{
		long lValue; /* the expected difference between audio and video assuming all delays have been applied as requested (in ms) */
		long lError; /* the expected range of variation of the delay between audio and video assuming all delays have been applied as requested (in ms) */
	} sExpectedAudioVisualDisparity;
	struct
	{
		long lValue; /* the expected difference between audio and video assuming all delays have been applied as requested (in ms) */
		long lError; /* the expected range of variation of the delay between audio and video assuming all delays have been applied as requested (in ms) */
	} sExpectedVideoVisualDisparity;
	struct
	{
		long lValue; /* the expected difference between audio and video assuming all delays have been applied as requested (in ms) */
		long lError; /* the expected range of variation of the delay between audio and video assuming all delays have been applied as requested (in ms) */
	} sExpectedAudioAuralDisparity;
} BSYNClib_Channel_Status;

/*
Summary:
Delay element source status.  This applies to video and audio sources.
*/
typedef struct
{
	bool bMuted;
    bool bDelayReceived;
	struct
	{
		bool bEnabled;
		BSYNClib_UnsignedValue sThreshold;
	} sDelayNotification;
	BSYNClib_UnsignedValue sAppliedDelay; /* defaults to ms */
} BSYNClib_Source_Status;

/*
Summary:
Delay element sink status.  This applies to video and audio sinks.
*/
typedef struct
{
    bool bDelayReceived;
	struct
	{
		bool bEnabled;
		BSYNClib_UnsignedValue sThreshold;
	} sDelayNotification;
	BSYNClib_UnsignedValue sAppliedDelay; /* defaults to ms */
} BSYNClib_Sink_Status;

/*
Summary:
Returns the default global sync lib module settings
Description:
*/
void BSYNClib_GetDefaultSettings
(
	BSYNClib_Settings * psSettings /* [out] */
);

/*
Summary:
Opens the global sync lib module
Description:
*/
BERR_Code BSYNClib_Open
(
	const BSYNClib_Settings * psSettings,
	BSYNClib_Handle * phSync /* [out] */
);

/*
Summary:
Closes the global sync lib module
Description:
*/
void BSYNClib_Close
(
	BSYNClib_Handle hSync
);

/*
Summary:
Returns the default settings for a sync channel
Description:
*/
void BSYNClib_GetChannelDefaultSettings
(
	BSYNClib_Channel_Settings * psSettings /* [out] */
);

/*
Summary:
Creates a sync lib channel
Description:
*/
BERR_Code BSYNClib_CreateChannel
(
	BSYNClib_Handle hSync,
	const BSYNClib_Channel_Settings * psSettings,
	BSYNClib_Channel_Handle * phChn /* [out] */
);

/*
Summary:
Closes a sync lib channel
Description:
*/
void BSYNClib_DestroyChannel
(
	BSYNClib_Handle hSync,
	BSYNClib_Channel_Handle hChn
);

/*
Summary:
Returns the current settings for a sync lib channel
*/
void BSYNClib_Channel_GetSettings
(
	const BSYNClib_Channel_Handle hChn,
	BSYNClib_Channel_Settings * psSettings /* [out] */
);

/*
Summary:
Sets the current settings for a sync lib channel
*/
BERR_Code BSYNClib_Channel_SetSettings
(
	BSYNClib_Channel_Handle hChn,
	const BSYNClib_Channel_Settings * psSettings
);

/*
Summary:
Returns the default configuration of a sync lib channel
*/
void BSYNClib_Channel_GetDefaultConfig(
    BSYNClib_Channel_Config * psConfig /* out */
);

/*
Summary:
Returns the current configuration of a sync lib channel
*/
void BSYNClib_Channel_GetConfig
(
	const BSYNClib_Channel_Handle hChn,
	BSYNClib_Channel_Config * psConfig /* [out] */
);

/*
Summary:
Sets the current configuration of a sync lib channel
*/
BERR_Code BSYNClib_Channel_SetConfig
(
	BSYNClib_Channel_Handle hChn,
	const BSYNClib_Channel_Config * psConfig
);

/*
Summary:
Returns the current status of a sync lib channel.
*/
void BSYNClib_Channel_GetStatus
(
	const BSYNClib_Channel_Handle hChn,
	BSYNClib_Channel_Status * psStatus /* [out] */
);

/*
Summary:
Returns the default config of a video source.
*/
void BSYNClib_Channel_GetDefaultVideoSourceConfig
(
    BSYNClib_VideoSource_Config * psConfig /* [out] */
);

/*
Summary:
Returns the current configuration of a sync lib video source at isr time
*/
void BSYNClib_Channel_GetVideoSourceConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_VideoSource_Config * psConfig /* [out] */
);

/*
Summary:
Sets the current configuration of a sync lib video source at isr time
*/
BERR_Code BSYNClib_Channel_SetVideoSourceConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	const BSYNClib_VideoSource_Config * psConfig
);

/*
Summary:
Returns the current configuration of a sync lib video sink at isr time
*/
void BSYNClib_Channel_GetVideoSinkConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_VideoSink_Config * psConfig /* [out] */
);

/*
Summary:
Sets the current configuration of a sync lib video sink at isr time
*/
BERR_Code BSYNClib_Channel_SetVideoSinkConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	const BSYNClib_VideoSink_Config * psConfig
);

/*
Summary:
Returns the current configuration of a sync lib audio source at isr time
*/
void BSYNClib_Channel_GetAudioSourceConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_AudioSource_Config * psConfig /* [out] */
);

/*
Summary:
Sets the current configuration of a sync lib audio source at isr time
*/
BERR_Code BSYNClib_Channel_SetAudioSourceConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	const BSYNClib_AudioSource_Config * psConfig
);

/*
Summary:
Returns the current configuration of a sync lib audio sink at isr time
*/
void BSYNClib_Channel_GetAudioSinkConfig_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_AudioSink_Config * psConfig /* [out] */
);

/*
Summary:
Sets the current configuration of a sync lib audio sink at isr time
*/
BERR_Code BSYNClib_Channel_SetAudioSinkConfig_isr
(
	BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	const BSYNClib_AudioSink_Config * psConfig
);

/*
Summary:
Returns the current status of a sync lib video source.
*/
void BSYNClib_Channel_GetVideoSourceStatus
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_Source_Status * psStatus /* [out] */
);

/*
Summary:
Returns the current status of a sync lib video sink.
*/
void BSYNClib_Channel_GetVideoSinkStatus_isr
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_Sink_Status * psStatus /* [out] */
);

/*
Summary:
Returns the current status of a sync lib Audio source.
*/
void BSYNClib_Channel_GetAudioSourceStatus
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSource,
	BSYNClib_Source_Status * psStatus /* [out] */
);

/*
Summary:
Returns the current status of a sync lib Audio sink.
*/
void BSYNClib_Channel_GetAudioSinkStatus
(
	const BSYNClib_Channel_Handle hChn,
	unsigned int uiSink,
	BSYNClib_Sink_Status * psStatus /* [out] */
);

/*
Summary:
Converts an unsigned value from 'from' units to 'to' units.
*/
BERR_Code BSYNClib_Convert
(
    unsigned int uiFromValue,
    BSYNClib_Units eFromUnits,
    BSYNClib_Units eToUnits,
    unsigned int * puiToValue /* [out] */
);

/*
Summary:
Converts an unsigned value from 'from' units to 'to' units, used at isr time.
*/
BERR_Code BSYNClib_Convert_isr
(
    unsigned int uiValue,
    BSYNClib_Units eFromUnits,
    BSYNClib_Units eToUnits,
    unsigned int * puiToValue /* [out] */
);

/*
Summary:
Converts a signed value from 'from' units to 'to' units.
*/
BERR_Code BSYNClib_ConvertSigned
(
    int iFromValue,
    BSYNClib_Units eFromUnits,
    BSYNClib_Units eToUnits,
    int * piToValue /* [out] */
);

/*
Summary:
Converts a signed value from 'from' units to 'to' units, used at isr time.
*/
BERR_Code BSYNClib_ConvertSigned_isr
(
    int iFromValue,
    BSYNClib_Units eFromUnits,
    BSYNClib_Units eToUnits,
    int * piToValue /* [out] */
);

#endif /* BSYNCLIB_H__ */

