/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_VIDEO_ENCODER_H__
#define NEXUS_VIDEO_ENCODER_H__

#include "nexus_video_encoder_types.h"
#include "nexus_video_encoder_init.h"
#include "nexus_display.h"
#include "nexus_stc_channel.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Options for video encoder configuration
**/
typedef enum NEXUS_VideoEncoderType
{
   NEXUS_VideoEncoderType_eMulti, /* Multiple instances allowed at a time
                                   * Combined Resolution of active encodes CANNOT exceed Max Resolution
                                   * No Low Delay Support: NEXUS_VideoEncoderStartSettings.lowDelayPipeline MUST BE false
                                   * Increased A2PDelay for ALL streams */
   NEXUS_VideoEncoderType_eSingle, /* Only single instance allowed at a time
                                    * Low Delay Supported: NEXUS_VideoEncoderStartSettings.lowDelayPipeline CAN be true
                                    * Max Resolution is supported */
   NEXUS_VideoEncoderType_eMultiNonRealTime,  /* Multiple non-realtime instances allowed at a time
                                               * NRT mode ONLY: NEXUS_VideoEncoderStartSettings.nonRealTime MUST BE true
                                               * Max Resolution is supported on EACH stream */

   NEXUS_VideoEncoderType_eCustom, /* Application controls the max number of simultaneous channels via NEXUS_VideoEncoderOpenSettings.maxChannelCount */
   NEXUS_VideoEncoderType_eMax
} NEXUS_VideoEncoderType;


/**
Summary:
Settings for opening a new VideoEncoder.
**/
typedef struct NEXUS_VideoEncoderOpenSettings
{
    struct {
        unsigned fifoSize;
        NEXUS_HeapHandle heap;
    } data, index;
    struct {
        bool interlaced; /* interlaced requires more memory than progressive */
        unsigned maxWidth, maxHeight;
    } memoryConfig;
    NEXUS_CallbackDesc errorCallback; /* deprecated */
    NEXUS_CallbackDesc watchdogCallback; /* Called whenever encoder watchdog fires. Application is responsible to restart video encoder. */
    NEXUS_VideoEncoderType type;
    unsigned maxChannelCount; /* used if type==NEXUS_VideoEncoderType_eCustom, specifies the maximum number of simultaneous channels
                                 that will be operating simultaneously on this device.  A value of 0 indicates the max channels supports by the device
                                 as determined by FW. */
    bool enableDataUnitDetecton; /* enables the SW based Data Unit (aka NALU) Detection logic that is needed for certain A/V containers (E.g. MP4) */
} NEXUS_VideoEncoderOpenSettings;

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
**/
void NEXUS_VideoEncoder_GetDefaultOpenSettings(
    NEXUS_VideoEncoderOpenSettings *pSettings   /* [out] */
    );

/**
Summary:
Open a new VideoEncoder.

Description:
Each VideoEncoder instance is able to encode a single video stream.
**/
NEXUS_VideoEncoderHandle NEXUS_VideoEncoder_Open( /* attr{destructor=NEXUS_VideoEncoder_Close}  */
    unsigned index,
    const NEXUS_VideoEncoderOpenSettings *pSettings     /* attr{null_allowed=y} Pass NULL for default settings */
    );

/**
Summary:
Close a VideoEncoder.
**/
void NEXUS_VideoEncoder_Close(
    NEXUS_VideoEncoderHandle handle
    );

/**
Summary:
Expected properties of the video source and output

Description:
If properties of the video source and output are known and specified, encoder latency can be improved.
**/
typedef struct NEXUS_VideoEncoderBounds
{
    /* NEXUS_VideoEncoderSettings.frameRate must be within this range.
    Higher min frame rate could reduce encoder delay. */
    struct {
        NEXUS_VideoFrameRate min, max;
    } outputFrameRate;

    /* The input frame rate cannot go lower than this min.
    Higher min frame rate will reduce encoder delay. */
    struct {
        NEXUS_VideoFrameRate min;
    } inputFrameRate;

    /* The input picture dimensions cannot go above this max.
    And this max must be within NEXUS_VideoEncoderOpenSettings.memoryConfig.maxWidth and .maxHeight.
    Smaller max dimension will cost less encoder delay. */
    struct {
        struct {
            unsigned width, height;
        } max;
        struct {
            unsigned width, height;
        } maxInterlaced;
    } inputDimension;
    struct
    {
        struct
        {
            unsigned bitrateMax; /* units of bits per second */
            unsigned bitrateTarget; /* units of bits per second, If "0", means CBR, and bitrateTarget=bitrateMax.  If non-zero, then VBR. */
        } upper;
    } bitrate;
    struct {
        struct {
            unsigned framesP; /* number of P frames between I frames */
            unsigned framesB; /* number of B frames between I or P frames */
        } max;
    } streamStructure;
} NEXUS_VideoEncoderBounds;

/**
Summary:
Start-time settings.

Description:
See Also:
NEXUS_VideoEncoder_GetDefaultStartSettings
NEXUS_VideoEncoder_Start
**/
typedef struct NEXUS_VideoEncoderStartSettings
{
    NEXUS_DisplayHandle input;
    NEXUS_VideoWindowHandle window; /* optional. only used to specify PIP window for legacy soft transcode systems. */
    bool interlaced;
    bool nonRealTime;
    bool lowDelayPipeline; /* If set, enableFieldPairing cannot be true, and encoder type must be eSingle. */
    bool encodeUserData; /* To enable closed caption ES layer user data encode feature */
    bool encodeBarUserData; /* To enable bar data encode; by default is true; */
    bool adaptiveLowDelayMode;    /* If set  encoder will drop all incoming frames until the first decoded frame is seen.
                                     The first frame will be encoded with a low delay.  Then delay
                                     will automatically ramp to the value specified in NEXUS_VideoEncoderSettings.encoderDelay
                                  */
    NEXUS_VideoCodec codec;
    NEXUS_VideoCodecProfile profile;
    NEXUS_VideoCodecLevel level;
    NEXUS_VideoEncoderBounds bounds; /* Encoder Bounds - If the bounds are known and specified, encoder latency can be improved.
                                        These bounds are for a single encode session.  E.g. if the output frame rate is known to to be
                                        fixed at 30fps, then the output frame rate min/max can be set to 30. */
    unsigned rateBufferDelay;       /* in ms.  Higher values indicate better quality but more latency.  0 indicates use encoder's defaults */

    NEXUS_StcChannelHandle stcChannel;  /* In RT mode, the StcChannel connects audio and video encoders to provide
                                            common timebase. If audio and video encoders are connected to a stream mux, the same STC channel should
                                            also be set to the stream mux to output conforming TS stream. In NRT mode, the stcChannel is dummy for
                                            encoder and mux. */
    unsigned numParallelEncodes;    /* indicates how many parallel encodes are expected.
                                       0  - indicates the encoder defaults.
                                       1  - means regular non-realtime encoding.
                                       2+ - indicates fast non-realtime encoding. */
    bool bypassVideoProcessing;     /* Indicates if user wants bypass video processing like dering and color space conv,
                                       which are turn on by default as long as HW supports.  This is a test feature. */
    NEXUS_EntropyCoding entropyCoding;
    NEXUS_CallbackDesc dataReady;

    struct {
       bool disableFrameDrop; /* This flag only applies to HRD mode rate control. When set, encoder will not drop pictures due to HRD model buffer underflow. */
    } hrdModeRateControl;
    struct {
       bool enable;
       unsigned duration; /* duration of the segment (in ms) */
       unsigned upperTolerance; /* percentage tolerance above the target bitrate, between 0 and 100 */
       unsigned lowerTolerance; /* percentage tolerance below the target bitrate, between 0 and 100 */
    } segmentModeRateControl;
    struct {
        bool singleRefP; /* Force the encoder to use only one reference for P pictures. */
        bool requiredPatchesOnly; /* Force the encoder to use only the required patches (i.e. disable use of optional patches) */
    } memoryBandwidthSaving;
} NEXUS_VideoEncoderStartSettings;

/**
Summary:
Range for encoderDelay (arrival to presentation) delay in the video encoder buffer model.
The encoderDelay setting must be within the [min, max] range.
**/
typedef struct NEXUS_VideoEncoderDelayRange {
    unsigned min; /* min delay in 27MHz ticks */
    unsigned max; /* max delay in 27MHz ticks */
} NEXUS_VideoEncoderDelayRange;

/**
Summary:
Settings that could be changed synchronously with changes of the source
**/
typedef struct NEXUS_VideoEncoderSettingsOnInputChange
{
    unsigned bitrateMax; /* units of bits per second */
    unsigned bitrateTarget; /* units of bits per second, If "0", means CBR, and bitrateTarget=bitrateMax.  If non-zero, then VBR. */
} NEXUS_VideoEncoderSettingsOnInputChange;

/**
Summary:
Options for video encoder stop
**/
typedef enum NEXUS_VideoEncoderStopMode
{
    NEXUS_VideoEncoderStopMode_eNormal, /* Default: Existing pictures will be finished.
                                       An EOS descriptor will be appended to the output buffer.
                                       The application MUST continue to consume the output buffer until the EOS is reached. */
    NEXUS_VideoEncoderStopMode_eImmediate, /* Stop as soon as possible.
                                       Existing pictures in flight may be dropped.
                                       An EOS descriptor will be appended to the output buffer.
                                       The application MUST continue to consume the output buffer until the EOS is reached,
                                       or if a nexus mux module is consuming the encoder, application MUST wait for
                                       mux being finished before stopping the mux module. */
    NEXUS_VideoEncoderStopMode_eAbort, /* Stop as soon as possible.
                                       Existing pictures in flight may be dropped.
                                       An EOS descriptor will NOT be appended to the output buffer.
                                       The application MUST NOT wait for the EOS is reached,
                                       or if a nexus mux module is consuming the encoder, application MUST NOT wait for
                                       mux being finished, instead should stop the mux immediately. */
    NEXUS_VideoEncoderStopMode_eMax
} NEXUS_VideoEncoderStopMode;

/**
Summary:
Settings for stopping VideoEncoder.
**/
typedef struct NEXUS_VideoEncoderStopSettings
{
    NEXUS_VideoEncoderStopMode mode;
} NEXUS_VideoEncoderStopSettings;



/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.
**/
void NEXUS_VideoEncoder_GetDefaultStartSettings(
    NEXUS_VideoEncoderStartSettings *pSettings /* [out] */
    );

/**
Summary:
Start encoding a stream
**/
NEXUS_Error NEXUS_VideoEncoder_Start(
    NEXUS_VideoEncoderHandle handle,
    const NEXUS_VideoEncoderStartSettings *pSettings
    );

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.
**/
void NEXUS_VideoEncoder_GetDefaultStopSettings(
    NEXUS_VideoEncoderStopSettings *pSettings   /* [out] */
);



/**
Summary:
    Stop encoding a stream
**/
void NEXUS_VideoEncoder_Stop(
    NEXUS_VideoEncoderHandle handle,
    const NEXUS_VideoEncoderStopSettings *pSettings   /* attr{null_allowed=y} */
);


/**
Summary:
GOP Parameters
**/
typedef struct NEXUS_VideoEncoderStreamStructure
{
      bool newGopOnSceneChange; /* If true, the encoder will start a new GOP if it detects a scene change */
      unsigned duration; /* number of P frames is determined internally based on the specified duration (in milliseconds) */
      bool adaptiveDuration; /* If true and duration!=0, the encoder will start with shorter GOP duration, then adaptively
                                    ramp to the fixed duration specified by NEXUS_VideoEncoderStreamStructure.duration. Default false. */
      unsigned framesP; /* number of P frames between I frames. 0xFFFFFFFF indicates IP infinite mode, ignored if duration != 0 */
      unsigned framesB; /* number of B frames between I or P frames, ignored if framesP == 0xFFFFFFFF */
      bool openGop; /* only relevant if framesB != 0 */
} NEXUS_VideoEncoderStreamStructure;

/**
Summary:
Run-time settings

Description:
Some encoding settings are made with other nexus API in the video pipeline. For example,
you can make run-time resolution changes by calling NEXUS_Display_SetCustomFormatSettings on the
encoder's display input.
**/
typedef struct NEXUS_VideoEncoderSettings
{
    unsigned bitrateMax; /* units of bits per second */
    unsigned bitrateTarget; /* units of bits per second, If "0", means CBR, and bitrateTarget=bitrateMax.  If non-zero, then VBR. */

    bool variableFrameRate;
    bool enableFieldPairing; /* to enable picture repeat cadence detection feature to improve bit efficiency */

    NEXUS_VideoFrameRate    frameRate;
    NEXUS_VideoEncoderStreamStructure streamStructure; /* GOP structure */
    unsigned encoderDelay; /* encoder delay, should be within NEXUS_VideoEncoderDelayRange.min .. NEXUS_VideoEncoderDelayRange.max range */
} NEXUS_VideoEncoderSettings;

/**
Summary:
Get default NEXUS_VideoEncoderSettings
**/
void NEXUS_VideoEncoder_GetDefaultSettings(
    NEXUS_VideoEncoderSettings *pSettings /* [out] */
    );

/**
Summary:
Get the current NEXUS_VideoEncoderSettings from the encoder.
**/
void NEXUS_VideoEncoder_GetSettings(
    NEXUS_VideoEncoderHandle handle,
    NEXUS_VideoEncoderSettings *pSettings /* [out] */
    );

/**
Summary:
Set new NEXUS_VideoEncoderSettings to the encoder.
**/
NEXUS_Error NEXUS_VideoEncoder_SetSettings(
    NEXUS_VideoEncoderHandle handle,
    const NEXUS_VideoEncoderSettings *pSettings
    );

/**
Summary:
Returns range of delays supported by the encoder
**/
NEXUS_Error NEXUS_VideoEncoder_GetDelayRange (
    NEXUS_VideoEncoderHandle handle,
    const NEXUS_VideoEncoderSettings *pSettings,
    const NEXUS_VideoEncoderStartSettings *pStartSettings,
    NEXUS_VideoEncoderDelayRange *pDelayRange
    );

/**
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new structure members in the future.
**/
void
NEXUS_VideoEncoder_GetSettingsOnInputChange(
    NEXUS_VideoEncoderHandle handle,
    NEXUS_VideoEncoderSettingsOnInputChange *pSettings
    );

/**
Summary:
Set new NEXUS_VideoEncoderSettingsOnInputChange to the encoder.

Description:
Sets the encode parameters to be used in sync with change to
the input to the encoder (e.g. resolution change)
**/
NEXUS_Error NEXUS_VideoEncoder_SetSettingsOnInputChange(
    NEXUS_VideoEncoderHandle handle,
    const NEXUS_VideoEncoderSettingsOnInputChange *pSettings
    );

/**
Summary:
Instructs video encoder to insert random access point

Description:
Instructs the video encoder firmware to insert random access point (RAP) at earliest possibility
**/
NEXUS_Error NEXUS_VideoEncoder_InsertRandomAccessPoint(
    NEXUS_VideoEncoderHandle handle
    );

/**
Summary:
See nexus_video_encoder_output.h for bits definition of the errorFlags and eventFlags.
**/
typedef struct NEXUS_VideoEncoderClearStatus {
    uint32_t errorFlags;
    uint32_t eventFlags;
} NEXUS_VideoEncoderClearStatus;

/**
Summary: Clears error and events
**/
void NEXUS_VideoEncoder_ClearStatus(
    NEXUS_VideoEncoderHandle handle,
    const NEXUS_VideoEncoderClearStatus *pClearStatus /* attr{null_allowed=y} error and events bit to clear, if NULL, all bits would get cleared */
    );

/**
Summary: Clears error and events
**/
void NEXUS_VideoEncoder_GetDefaultClearStatus(
        NEXUS_VideoEncoderClearStatus *pClearStatus /* default to clear all bits */
     );

typedef struct NEXUS_VideoEncoderCapabilities
{
    struct {
        bool supported;
        unsigned displayIndex;
        unsigned deviceIndex;  /* mapping to HW encoder device */
        unsigned channelIndex; /* mapping to HW encoder channel on the device */
        NEXUS_VideoEncoderMemory memory; /* init time, max memory per channel */
    } videoEncoder[NEXUS_MAX_VIDEO_ENCODERS];
} NEXUS_VideoEncoderCapabilities;

void NEXUS_GetVideoEncoderCapabilities(
    NEXUS_VideoEncoderCapabilities *pCapabilities
    );

typedef struct NEXUS_VideoEncoderModuleStatistics
{
    unsigned numStarts; /* total number of times video encode was started since NEXUS_Platform_Init */
} NEXUS_VideoEncoderModuleStatistics;

void NEXUS_VideoEncoderModule_GetStatistics(
    NEXUS_VideoEncoderModuleStatistics *pStats
    );

#ifdef __cplusplus
}
#endif


#endif /* NEXUS_VIDEO_ENCODER_H__ */


