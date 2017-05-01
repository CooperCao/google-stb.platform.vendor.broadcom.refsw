/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_VIDEO_DECODER_TYPES_H__
#define NEXUS_VIDEO_DECODER_TYPES_H__

#include "nexus_pid_channel.h"
#include "nexus_stc_channel.h"
#include "nexus_surface_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
VideoDecoder handle obtained from NEXUS_VideoDecoder_Open.

Description:
It is separated from the nexus_video_decoder.h API for use by NEXUS_VideoInput.

See Also:
NEXUS_VideoDecoder_Open
NEXUS_VideoDecoder_Close
**/
typedef struct NEXUS_VideoDecoder *NEXUS_VideoDecoderHandle;

/*
Summary:
Decoder error handling modes

Description:
The video decoder can detect some (but not all) decoding errors. These errors can result from a badly encoded stream or bad transmission of a good stream.
For pictures where decoding errors are detected, the decoder can either display them as-is, or it can discard them.
When discarding, it can discard aggressively (i.e. to minimize the chance of the user seeing errors) or conservatively (i.e. to minimize the number of pictures dropped).
These two options have tradeoffs. This enum instructs the decoder what decision to make based on application preferences.

If a picture is discarded, the previous picture will be repeated. The end-user will see a frozen picture until a new picture is available.

Please be aware that there is no guarantee that all decoder errors will be hidden. This is simply best effort.
*/
typedef enum NEXUS_VideoDecoderErrorHandling
{
   NEXUS_VideoDecoderErrorHandling_eNone,        /* display all pictures regardless of any detected errors (default) */
   NEXUS_VideoDecoderErrorHandling_ePicture,     /* drop any picture if an error is detected by the decoder  */
   NEXUS_VideoDecoderErrorHandling_ePrognostic,  /* drop all pictures to the next random access point if an error is detected by the decoder */
   NEXUS_VideoDecoderErrorHandling_eMax
} NEXUS_VideoDecoderErrorHandling;

/**
Summary:
480p/576p/1080p override mode
**/
typedef enum NEXUS_VideoDecoderProgressiveOverrideMode
{
   NEXUS_VideoDecoderProgressiveOverrideMode_eTopBottom, /* Assume 480p/576p/1080p content with pulldown="F" is pulldown="TB" + source_format="Progressive" */
   NEXUS_VideoDecoderProgressiveOverrideMode_eBottomTop, /* Assume 480p/576p/1080p content with pulldown="F" is pulldown="BT" + source_format="Progressive" */
   NEXUS_VideoDecoderProgressiveOverrideMode_eDisable,   /* Assume 480p/576p/1080p content with pulldown="F" is correct */
   NEXUS_VideoDecoderProgressiveOverrideMode_eMax
} NEXUS_VideoDecoderProgressiveOverrideMode;

/**
Summary:
Enum to notify the decoder of timestamp ordering in the stream.

Description:
NEXUS_VideoDecoderTimestampMode_eDecode is used for streams that are using DTS in place of PTS (for example H.264 video in AVI and ASF container files).
**/
typedef enum NEXUS_VideoDecoderTimestampMode
{
    NEXUS_VideoDecoderTimestampMode_eDisplay = 0,   /* Timestamps are expected in display order (default) */
    NEXUS_VideoDecoderTimestampMode_eDecode,        /* Timestamps are expected in decode order */
    NEXUS_VideoDecoderTimestampMode_eMax
} NEXUS_VideoDecoderTimestampMode;


/*
Summary:
A video program to be decoded with NEXUS_VideoDecoder_Start.

Description:
See Also:
NEXUS_VideoDecoder_GetDefaultStartSettings
NEXUS_VideoDecoder_Start
*/
typedef struct NEXUS_VideoDecoderStartSettings
{
    NEXUS_PidChannelHandle pidChannel;  /* The source of the video data from Transport demux. See NEXUS_PidChannel_Open, NEXUS_Playpump_OpenPidChannel
                                           or NEXUS_Playback_OpenPidChannel. */
    NEXUS_PidChannelHandle enhancementPidChannel;  /* The source of the enhancement video data from Transport demux. See NEXUS_PidChannel_Open,
                                           NEXUS_Playpump_OpenPidChannel or NEXUS_Playback_OpenPidChannel. */
    NEXUS_VideoCodec codec;             /* The type of video being decoded. */
    NEXUS_StcChannelHandle stcChannel;  /* The StcChannel connects audio and video decode to provide lipsync (also called Time Stamp Managed mode, or TSM mode).
                                            In TSM mode, pictures are sent to Display only when the PTS matches the STC within a certain threshold.
                                            If NULL, the decoder performs no TSM and is in "vsync mode". A new pictures is sent to the display on every vsync interrupt.
                                            See NEXUS_StcChannel_Open. */
    NEXUS_VideoDecoderErrorHandling errorHandling;
    unsigned errorHandlingThreshold;    /* Units are percent. If at least this percentage of macro blocks has an error, the whole picture is treated as an
                                            error according to 'errorHandling'. Default is zero. */
    unsigned prerollRate;                /* A non-zero value tells the decoder to feed pictures to the display before the PTS matches STC after starting decode.
                                            This gives a perceived increase to channel change speed.
                                            The value is the number of times each decoded picture should be fed (i.e. repeated) to the display on the display's vsync. */
    NEXUS_VideoFrameRate frameRate;      /* User-specified frame rate. For some streams, the decoder cannot determine the frame rate.
                                            The user may be able to determine the frame rate from meta data.
                                            This value is overridden by any value found in the stream. */
    NEXUS_AspectRatio aspectRatio;       /* User-specified aspect ratio. For some streams, the decoder cannot determine the aspect ratio.
                                            The user may be able to determine the aspect ratio from meta data.
                                            This value is overridden by any value found in the stream. */
    struct {
        unsigned x, y;
    } sampleAspectRatio;                 /* Valid is user-specified aspectRatio is NEXUS_AspectRatio_eSar */

    NEXUS_VideoDecoderProgressiveOverrideMode progressiveOverrideMode;
    NEXUS_VideoDecoderTimestampMode timestampMode;

    NEXUS_MatrixCoefficients defaultMatrixCoefficients; /* if NEXUS_MatrixCoefficients is eUnknown, assign this before sending a picture to the display. */
    NEXUS_ColorPrimaries defaultColorPrimaries; /* if NEXUS_ColorPrimaries is eUnknown, assign this before sending a picture to the display. */
    NEXUS_TransferCharacteristics defaultTransferCharacteristics; /* if NEXUS_TransferCharacteristics is eUnknown, assign this before sending a picture to the display. */

    /* See NEXUS_VideoDecoderExtendedSettings for settings used for streams with special encoding characterisitcs. */
    bool nonRealTime; /* Normal operation for decoding is real time, if this is set to 'true' then decoding is used as source for non-realtime transcode operation */
    bool appDisplayManagement;  /* Application Display Management Mode.  If enabled, the application must use NEXUS_VideoDecoder_GetDecodedFrames and
                                   NEXUS_VideoDecoder_ReturnDecodedFrames to manage the decoded picture queue */
} NEXUS_VideoDecoderStartSettings;

/**
Summary:
Specify what decoder should do with the last picture when stopping decode and when to display the first
picture when starting the next decode.

Description:
"Mute" means displaying a all-black picture.
"TSM lock" is the time when the first picture's PTS matches the current STC allowing a picture to be displayed with proper lipsync.
    The first decoded picture may have to be discarded in order to achieve TSM lock.
"FirstPicture" is defined as the first decoded picture, without reference to TSM lock.

Used in NEXUS_VideoDecoderSettings
**/
typedef enum NEXUS_VideoDecoder_ChannelChangeMode
{
    NEXUS_VideoDecoder_ChannelChangeMode_eMute,                  /* Mute immediately on stop. Show new picture when TSM lock achieved. */
    NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock,      /* Hold last picture after stop until TSM lock occurs on next decode. No muted or stationary pictures ever appear. */
    NEXUS_VideoDecoder_ChannelChangeMode_eMuteUntilFirstPicture, /* Mute immediately on stop. Show first decoded picture, before TSM lock achieved. */
    NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture  /* Hold last picture after stop until first decoded picture, before TSM lock achieved. No muted pictures ever appear. */
} NEXUS_VideoDecoder_ChannelChangeMode;

/**
Summary:
Interpolation mode for still content (like paused pictures)
**/
typedef enum NEXUS_StillContentInterpolationMode
{
    NEXUS_StillContentInterpolationMode_eDefault,    /* Default mode. No interpolation. */
    NEXUS_StillContentInterpolationMode_eSingleField,/* Last Valid Field Display on Pause */
    NEXUS_StillContentInterpolationMode_eBothField,  /* Both Field Display on Pause */
    NEXUS_StillContentInterpolationMode_eFrame,      /* Frame Display (conditional) on Pause */
    NEXUS_StillContentInterpolationMode_eMax
} NEXUS_StillContentInterpolationMode;

/**
Summary:
Interpolation mode for moving content (i.e. not still)
**/
typedef enum NEXUS_MovingContentInterpolationMode
{
    NEXUS_MovingContentInterpolationMode_eDefault,
    NEXUS_MovingContentInterpolationMode_eInterlacedScanout,
    NEXUS_MovingContentInterpolationMode_eProgressiveScanout,
    NEXUS_MovingContentInterpolationMode_eMax
} NEXUS_MovingContentInterpolationMode;

/*
Summary:
Scan mode
*/
typedef enum NEXUS_VideoDecoderScanMode
{
    NEXUS_VideoDecoderScanMode_eAuto, /* default. use the correct display table (CDT) for interlaced scan-out.
                                         that is, do 1080i scan-out for 1080p@24/25/30 sources.
                                         this setting has no effect on 1080p60 because 1080p60 sources must be scanned out by a 1080p60 capable MFD. */
    NEXUS_VideoDecoderScanMode_ePsF,  /* deprecated */
    NEXUS_VideoDecoderScanMode_e1080p, /* scan 1080p@24/25/30 as progressive */
    NEXUS_VideoDecoderScanMode_eMax
} NEXUS_VideoDecoderScanMode;

/*
Summary:
Horizontal overscan mode
*/
typedef enum NEXUS_VideoDecoderHorizontalOverscanMode
{
    NEXUS_VideoDecoderHorizontalOverscanMode_eAuto,
    NEXUS_VideoDecoderHorizontalOverscanMode_eDisable,
    NEXUS_VideoDecoderHorizontalOverscanMode_eMax
} NEXUS_VideoDecoderHorizontalOverscanMode;

typedef enum NEXUS_VideoDecoderSourceOrientation {
    NEXUS_VideoDecoderSourceOrientation_e2D,             /* 2D */
    NEXUS_VideoDecoderSourceOrientation_e3D_LeftRight,   /* 3D left right, half resolution */
    NEXUS_VideoDecoderSourceOrientation_e3D_OverUnder,   /* 3D over under, half resolution */
    NEXUS_VideoDecoderSourceOrientation_e3D_LeftRightFullFrame,   /* sequence of 3D left and right frames, full resolution */
    NEXUS_VideoDecoderSourceOrientation_e3D_RightLeftFullFrame,   /* sequence of 3D right and left frames, full resolution */
    NEXUS_VideoDecoderSourceOrientation_e3D_LeftRightEnhancedResolution,
    NEXUS_VideoDecoderSourceOrientation_eMax
} NEXUS_VideoDecoderSourceOrientation;

/*
Summary:
Video decoder settings.

Description:
These settings can be changed before or after NEXUS_VideoDecoder_Start.

Used in NEXUS_VideoDecoder_GetSettings and NEXUS_VideoDecoder_SetSettings.
*/
typedef struct NEXUS_VideoDecoderSettings
{
    bool freeze;            /* hold current picture. decoder can discard incoming data. */
    bool mute;              /* black out video, but keep decoding */
    bool userDataEnabled;   /* Enable raw userdata capture with NEXUS_VideoDecoder_GetUserDataBuffer */

    bool panScanEnabled;    /* Allow decoder to process panscan vectors if present. */

    NEXUS_VideoDecoderErrorHandling errorHandling; /* This value will be overwritten by StartSettings.errorHandling. So, if you want runtime control
                                                      of errorHandling, you must set it after calling Start. */
    NEXUS_VideoDecoder_ChannelChangeMode channelChangeMode;

    unsigned fifoThreshold; /* If set, this can limit the maximum FIFO depth to a value <= the value specified
                               in NEXUS_VideoDecoderOpenSettings.fifoSize.  If 0, the entire FIFO can fill.  */

    int ptsOffset;  /* Add an offset used in the decoder's TSM equation. Measured in PTS units (45KHz for MPEG2TS, 27MHz for DSS).
                       This is added to any PTS offset also set by SyncChannel. */

    unsigned discardThreshold; /* in PTS units. defaults to 0, which allows Nexus to set the default discard threshold based on codec and trick mode state.
                                  if PTS-STC is greater than the discard threshold, the picture is dropped. */

    NEXUS_CallbackDesc sourceChanged; /* Fires when the format (e.g. size, framerate) of pictures being fed to Display changes.
                                         Call NEXUS_VideoDecoder_GetStatus to get the current format information. */

    NEXUS_CallbackDesc streamChanged; /* Fires when the format (e.g. size, framerate) of pictures being decoded changes.
                                         This is similar to an MPEG sequence header interrupt, but with some important differences. See NEXUS_VideoDecoderStreamInformation
                                         for a complete description. Call NEXUS_VideoDecoder_GetStreamInformation to get the new video format information. */

    NEXUS_CallbackDesc appUserDataReady; /* Fires when application userdata is available. userDataEnabled must be true. */
    NEXUS_CallbackDesc ptsError;         /* Fires on any PTS discontinuity. This callback is used for application notification. No response is required for TSM. */
    NEXUS_CallbackDesc firstPts;         /* Fires when the decoder receives the first PTS after Start.
                                            This event is also called "request STC" because the decoder needs StcChannel to set the STC in Auto mode.
                                            This callback is used for application notification. No response is required for TSM. */
    NEXUS_CallbackDesc firstPtsPassed;   /* Fires when the decoder allows the first picture to pass to the display after TSM pass.
                                            This callback is used for application notification. No response is required for TSM. This
                                            callback also fires in non-TSM mode when the first frame is delivered to the display. */
    NEXUS_CallbackDesc fifoEmpty;        /* Fires when the source FIFO becomes empty during normal decode, usually because of the loss of the source. */

    NEXUS_CallbackDesc afdChanged;       /* Fires when the NEXUS_VideoDecoderUserDataStatus.afdValue (active format description) changes. */

    NEXUS_CallbackDesc decodeError;      /* Fires when the decoder reports an error. Call NEXUS_VideoDecoder_GetStatus and check numDecodeErrors. */

    struct {
        bool enable;
        unsigned preChargeCount;
        NEXUS_CallbackDesc chunkDone;    /* Fires when the FNRT chunk decoder completes a chunk delivery. */
    } fnrtSettings; /* fast non-realtime transcode settings */

    struct {
            struct {
                bool fifoNoLongerEmpty; /* if true, triggers the state changed callback the first time we see a new frame emerge from the decoder after every fifoEmpty event */
                bool fifoOverflow; /* if true, triggers the state changed callback each time we see a FIFO overflow occur. See NEXUS_VideoDecoderStatus.numDecodeOverflows for status. */
            } enabled;
            NEXUS_CallbackDesc callback; /* fires any time an event specified in the enabled substruct occurs */
    } stateChanged;

    bool preferredUserDataFormat;        /* Whatever format is set by NEXUS_VideoDecoder_SetUserDataFormatFilter is treated as the preferred format, but
                                            until that format is detected, VideoDecoder will operate in NEXUS_UserDataFormat_eAny mode.
                                            When the preferred format is detected, VideoDecoder will switch to the preferred mode until decode is stopped. */
    unsigned userDataFilterThreshold;    /* Threshold for NEXUS_UserDataFormat_eAny algorithm. */
    bool dropFieldMode;                  /* By default, if VideoDecoder needs to drop pictures because of TSM, it will drop field pairs. This prevents
                                            any jitter from the deinterlacer. However, for 60->50Hz conversion dropping field pairs may result in motion judder.
                                            By setting dropFieldMode to true, VideoDecoder will drop only single fields and 60->50Hz conversion will only have slight jitter instead. */
    bool manualPowerState;               /* If true, internal hardware state will be left up, even when VideoDecoder is disconnected.
                                            See NEXUS_VideoDecoder_SetPowerState for manual control if manualPowerState = true.
                                            If false (default), internal HW state will be automatically brought up on NEXUS_VideoWindow_AddInput and down on NEXUS_VideoWindow_RemoveInput.
                                            Bringing down internal HW state results in reduced power consumption. */
    NEXUS_StillContentInterpolationMode stillContentInterpolationMode;
    NEXUS_MovingContentInterpolationMode movingContentInterpolationMode;
    NEXUS_VideoDecoderScanMode scanMode;
    bool customSourceOrientation; /* allows application to override source's orientation derived by the decoder */
    NEXUS_VideoDecoderSourceOrientation sourceOrientation; /* video orientation for 3D sources, ignored if customSourceOrientation is false */
    NEXUS_VideoDecoderHorizontalOverscanMode horizontalOverscanMode;

    /* The following must be set before calling NEXUS_VideoWindow_AddInput or after NEXUS_VideoInput_Shutdown to take effect.*/
    bool supportedCodecs[NEXUS_VideoCodec_eMax]; /* List of codecs that can be decoded. */
    unsigned maxWidth, maxHeight;        /* max resolution that will be decoded */
    NEXUS_VideoFrameRate maxFrameRate; /* unused */
    unsigned colorDepth; /* 8 or 10 bit */
} NEXUS_VideoDecoderSettings;

/*
Summary:
Information of the last GOP time code found in the MPEG stream.

Description:
If the timecode is not found in the stream or if the codec doesn't support a time code, all members will be zero.

Used in NEXUS_VideoDecoderStatus
*/
typedef struct NEXUS_VideoDecoderTimeCode
{
    unsigned short hours;    /* ISO 13818-2 time_code_hours field */
    unsigned short minutes;  /* ISO 13818-2 time_code_minutes field */
    unsigned short seconds;  /* ISO 13818-2 time_code_seconds field */
    unsigned short pictures; /* ISO 13818-2 time_code_pictures field */
} NEXUS_VideoDecoderTimeCode;


/**
Summary:
The NEXUS_CHANNELSTATUS_AVD_xxx defines are values returned in the
NEXUS_VideoDecoderStatus.avdStatusBlock bitmask.

Description:
See bxvd.h comments and BXVD_CHANNELSTATUS_AVD_XXX macros for more values.
**/
#define NEXUS_CHANNELSTATUS_AVD_RAP_NOT_DETECTED              0x100
#define NEXUS_CHANNELSTATUS_AVD_UNSUPPORTED_FEATURE_DETECTED  0x200
#define NEXUS_CHANNELSTATUS_AVD_IMAGE_SIZE_TOO_BIG            0x400

/***************************************************************************
Summary:
Video protocol level

Description:
Returned by NEXUS_VideoDecoderStatus.protocolLevel
****************************************************************************/
typedef enum NEXUS_VideoProtocolLevel
{
   NEXUS_VideoProtocolLevel_eUnknown = 0,
   NEXUS_VideoProtocolLevel_e00,
   NEXUS_VideoProtocolLevel_e10,
   NEXUS_VideoProtocolLevel_e1B,
   NEXUS_VideoProtocolLevel_e11,
   NEXUS_VideoProtocolLevel_e12,
   NEXUS_VideoProtocolLevel_e13,
   NEXUS_VideoProtocolLevel_e20,
   NEXUS_VideoProtocolLevel_e21,
   NEXUS_VideoProtocolLevel_e22,
   NEXUS_VideoProtocolLevel_e30,
   NEXUS_VideoProtocolLevel_e31,
   NEXUS_VideoProtocolLevel_e32,
   NEXUS_VideoProtocolLevel_e40,
   NEXUS_VideoProtocolLevel_e41,
   NEXUS_VideoProtocolLevel_e42,
   NEXUS_VideoProtocolLevel_e50,
   NEXUS_VideoProtocolLevel_e51,
   NEXUS_VideoProtocolLevel_e60,
   NEXUS_VideoProtocolLevel_e62,
   NEXUS_VideoProtocolLevel_eLow,
   NEXUS_VideoProtocolLevel_eMain,
   NEXUS_VideoProtocolLevel_eHigh,
   NEXUS_VideoProtocolLevel_eHigh1440,
   NEXUS_VideoProtocolLevel_eMax
} NEXUS_VideoProtocolLevel;

/***************************************************************************
Summary:
Video protocol profile

Description:
Returned by NEXUS_VideoDecoderStatus.protocolProfile
****************************************************************************/
typedef enum NEXUS_VideoProtocolProfile
{
   NEXUS_VideoProtocolProfile_eUnknown = 0,
   NEXUS_VideoProtocolProfile_eSimple,
   NEXUS_VideoProtocolProfile_eMain,
   NEXUS_VideoProtocolProfile_eHigh,
   NEXUS_VideoProtocolProfile_eAdvanced,
   NEXUS_VideoProtocolProfile_eJizhun,
   NEXUS_VideoProtocolProfile_eSnrScalable,
   NEXUS_VideoProtocolProfile_eSpatiallyScalable,
   NEXUS_VideoProtocolProfile_eAdvancedSimple,
   NEXUS_VideoProtocolProfile_eBaseline,
   NEXUS_VideoProtocolProfile_eMultiHigh,
   NEXUS_VideoProtocolProfile_eStereoHigh,
   NEXUS_VideoProtocolProfile_eMain10,

   NEXUS_VideoProtocolProfile_eMax
} NEXUS_VideoProtocolProfile;

/**
Summary:
Current status of the video decoder.

Description:
The picture format information comes from the picture currently being fed to the Display.

See NEXUS_VideoDecoderSettings.sourceChanged for notification when the picture format changes.
**/
typedef struct NEXUS_VideoDecoderStatus
{
    bool started;           /* true if decoder was started */

    struct
    {
        unsigned width, height; /* dimensions in pixels */
    } source, coded, display; /* source is the active area size.
                                 coded is the coded area. it could be greater than the active area because of clipping.
                                 display is the display size needed to preserve aspect ratio. the actual display size is
                                   determined by the NEXUS_VideoWindowSettings. */

    NEXUS_AspectRatio aspectRatio;  /* aspect ratio of the source */
    NEXUS_VideoFrameRate frameRate; /* frame rate of the source */
    bool interlaced;          /* true if the source is interlaced, regardless of scanout requirements.
                                 Note that VideoInputStatus.videoFormat will report interlaced or progressive based on scanout,
                                 which is not necessarily the source coding. Some progressive streams must be scanned out as fields. */
    NEXUS_VideoFormat format; /* The video format is derived from source.height, frameRate and interlaced fields. */
    NEXUS_VideoProtocolLevel protocolLevel;
    NEXUS_VideoProtocolProfile protocolProfile;

    bool muted;               /* VideoDecoder is feeding muted pictures to Display */
    NEXUS_VideoDecoderTimeCode timeCode; /* last encoded timecode found in the stream */
    unsigned long pictureTag; /* last picture tag found in the stream */
    NEXUS_PictureCoding pictureCoding;

    /* TSM status */
    bool tsm;                 /* VideoDecoder is in TSM (time stamp managed) mode. TSM may be not applicable (e.g. ES streams), may be disabled directly
                                 by the user (setting stcChannel = NULL) or indirectly by other API's (e.g. trick modes, ASTM, etc.) */
    uint32_t pts;             /* Current PTS of the video decoder
                                 Note: the stc is available from NEXUS_StcChannel_GetStc */
    int32_t  ptsStcDifference; /* Current PTS-STC difference including lipsync adjustments */
    NEXUS_PtsType ptsType;    /* Type of the current PTS */
    bool firstPtsPassed;      /* The first PTS for this stream has passed. This means that the information in this status structure is for pictures currently being displayed.
                                 See NEXUS_VideoDecoderSettings.firstPtsPassed for a callback related to this status. */

    /* buffer status */
    unsigned fifoDepth;       /* depth in bytes of the compressed buffer */
    unsigned fifoSize;        /* size in bytes of the compressed buffer */
    unsigned queueDepth;      /* the number of decoded pictures currently ready to be displayed */
    unsigned cabacBinDepth;   /* depth in bytes of the cabac bin buffer */
    unsigned enhancementFifoDepth; /* depth in bytes of the enhancement compressed buffer */
    unsigned enhancementFifoSize; /* size in bytes of the enhancement compressed buffer */
    /* unsigned bufferLatency was moved for performance reasons.  See NEXUS_VideoDecoderFifoStatus.bufferLatency. */

    /* cumulative status */
    unsigned numDecoded;      /* total number of decoded pictures since Start */
    unsigned numDisplayed;    /* total number of display pictures since Start */
    unsigned numIFramesDisplayed; /* total number of displayed I-Frames pictures since Start */
    unsigned numDecodeErrors; /* total number of decoder errors since Start */
    unsigned numDecodeOverflows; /* total number of overflows of the input to the decoder since Start */
    unsigned numDisplayErrors;/* total number of display manager errors since Start. This includes parity mismatches which may result in glitching on the display. */
    unsigned numDecodeDrops;  /* total number of pictures dropped by the decoder since Start */
    unsigned numPicturesReceived; /* total number of pictures which the decoder has received since Start. This includes pictures which are skipped due to TSM, NEXUS_VideoDecoderDecodeMode,
                                     or other factors. */
    unsigned numDisplayDrops; /* total number of pictures dropped by the display manager due to TSM failures since Start */
    unsigned numDisplayUnderflows;  /* total number of times the display manager was unable to deliver a new picture since Start */
    unsigned ptsErrorCount;   /* counter for number of PTS error interrupts since Start */
    unsigned avdStatusBlock;  /* snap shot of the AVD channel status. See NEXUS_CHANNELSTATUS_AVD_XXX macros for possible values. */
    unsigned numWatchdogs;    /* total number of watchdog events for the device since NEXUS_VideoDecoderModule_Init. the count is per AVD device, not per VideoDecoder channel. */
    uint64_t numBytesDecoded; /* total number of ES bytes decoded since last start or flush */
    unsigned fifoEmptyEvents; /* total number of fifoEmpty events since last start or flush */
    unsigned fifoNoLongerEmptyEvents; /* total number of fifoNoLongerEmpty events since last start or flush.  If fifoEmptyEvents > fifoNoLongerEmptyEvents, then fifo is still empty */
} NEXUS_VideoDecoderStatus;

/**
Summary:
Status for a single decoded video frame
**/
typedef struct NEXUS_VideoDecoderFrameStatus
{
    uint32_t serialNumber;              /* Frame Serial Number*/
    uint32_t pts;                       /* Frame PTS Value */
    bool ptsValid;                      /* True if the PTS value is coded */
    bool lastPicture;                   /* True if this is a dummy frame triggered by a LAST BTP packet.
                                           This flag is reset on the next NEXUS_VideoDecoder_Start() call. */
    NEXUS_PictureCoding pictureCoding;  /* Picture Type */
    NEXUS_VideoFrameRate frameRate;
    NEXUS_PicturePullDown pullDown;

    struct {
        unsigned x, y;
    } sampleAspectRatio; /* only required if aspectRatio is NEXUS_AspectRatio_eSar */
    NEXUS_AspectRatio aspectRatio;

    NEXUS_PictureScan sourceFormat;
    NEXUS_PictureScan sequence;

    NEXUS_StripedSurfaceCreateSettings surfaceCreateSettings;   /* Settings to create a striped surface for this frame */
} NEXUS_VideoDecoderFrameStatus;

/**
Summary:
Settings for a returned frame
*/
typedef struct NEXUS_VideoDecoderReturnFrameSettings
{
    bool display;   /* If true (default), frame will be displayed.  If false, it will be dropped. */
} NEXUS_VideoDecoderReturnFrameSettings;

/**
Summary:
Tell the decoder which types of pictures to decode based on picture type.

Description:
See Also:
NEXUS_VideoDecoderTrickState
**/
typedef enum NEXUS_VideoDecoderDecodeMode
{
    NEXUS_VideoDecoderDecodeMode_eAll, /* Decode all pictures */
    NEXUS_VideoDecoderDecodeMode_eIP,  /* Decode only I and P pictures. Drop B pictures. In AVC, this means dropping all discardable pictures. */
    NEXUS_VideoDecoderDecodeMode_eI,   /* Decode only I pictures. Drop P and B pictures. In AVC, this means dropping all non-I, discardable pictures. */
    NEXUS_VideoDecoderDecodeMode_eMax
} NEXUS_VideoDecoderDecodeMode;

/**
Summary:
Tell the decoder what type of data reordering the decoder is required to perform.

Description:
At this time data pre-processing is only supported for MPEG-2 video streams and requires special configuration.

See Also:
NEXUS_VideoDecoderTrickState
**/
typedef enum NEXUS_VideoDecoderReorderingMode
{
    NEXUS_VideoDecoderReorderingMode_eNone, /* no data reordering, this is default mode and it's supported for all codecs */
    NEXUS_VideoDecoderReorderingMode_eSequential, /* decoder expects data in the sequential order */
    NEXUS_VideoDecoderReorderingMode_eGop,  /* decoder expects data in the GOP boundaries with marking between each chunk */
    NEXUS_VideoDecoderReorderingMode_eInterleaved,  /* decoder expects data in chunks in a reverse order, possibly not GOP alligned,
                                                     * each chunk contains at least one complete GOP, each chunk has its mark,
                                                     * chunks has to be overlapped to allow complete recovery of the MPEG-2 stream */
    NEXUS_VideoDecoderReorderingMode_eChunkForward,  /* decoder expacts data in chunks in a normal order, possible not GOP alligned,
                                                      * chunks shall not be overlapped, complete stream recovery shall not be expected */
    NEXUS_VideoDecoderReorderingMode_eChunkBackward,  /* decoder expacts data in chunks in a reverse order, possible not GOP alligned,
                                                      * chunks shall not be overlapped, complete stream recovery shall not be expected */
    NEXUS_VideoDecoderReorderingMode_eMax
} NEXUS_VideoDecoderReorderingMode;

/**
Summary:
**/
typedef enum NEXUS_TsmMode
{
    NEXUS_TsmMode_eDisabled, /* vsync mode. also called "virtual TSM" mode. */
    NEXUS_TsmMode_eEnabled,  /* TSM with HW STC. only supports positive rates with limited fast speeds. See NEXUS_StcChannel_SetRate. */
    NEXUS_TsmMode_eSimulated, /* TSM with SW STC. supports negative rates. unlimited rates. playback only. */
    NEXUS_TsmMode_eMax
} NEXUS_TsmMode;

typedef enum NEXUS_VideoDecoderDqtMode
{
    NEXUS_VideoDecoderDqtMode_eDisabled,
    NEXUS_VideoDecoderDqtMode_eSinglePass,
    NEXUS_VideoDecoderDqtMode_eMultiPass,
    NEXUS_VideoDecoderDqtMode_eMax
} NEXUS_VideoDecoderDqtMode;

/**
Summary:
Set a collection of video decoder trick mode parameters.

Description:
Trick mode state is separated from other state so that higher-level modules can handle trick modes apart from direct app control.

See Also:
NEXUS_VideoDecoder_GetTrickState
NEXUS_VideoDecoder_SetTrickState
**/
typedef struct NEXUS_VideoDecoderTrickState
{
    bool forceStopped;              /* If true, decoder will not be permitted to run, otherwise it's allowed */
    int rate;                       /* Speed control based on units of NEXUS_NORMAL_DECODE_RATE:
                                        NEXUS_NORMAL_DECODE_RATE*2 = 2x fast-forward.
                                        NEXUS_NORMAL_DECODE_RATE/2 = 2x slow-motion.
                                        0 = pause.
                                        NEXUS_NORMAL_DECODE_RATE = normal play.
                                        negative rates are supported with NEXUS_TsmMode_eSimulated.
                                        */
    NEXUS_VideoDecoderDecodeMode decodeMode; /* Instruct decoder which frames to decode and which to drop based on picture type. */
    bool brcmTrickModesEnabled;     /* If true, BTP's will be processed by the decoder. For some decodes, BTP's are always on and this option is ignored. */
    bool hostTrickModesEnabled;     /* If true host has manipulated stream content.  */
    NEXUS_VideoDecoderDqtMode dqtEnabled; /* If true, the display manager will reverse the order of displayed pictures to achieve GOP-based reverse trick modes. */
    bool topFieldOnly;              /* If true and source is interlaced, only top fields will be sent to the display. */
    bool fieldByFieldAdvance;       /* If true, then NEXUS_VideoDecoder_FrameAdvance will advance one field at a time for interlaced content. */
    bool reverseFields;             /* If true, fields will be reversed before sending to the display. This is used for reverse trick modes. */
    NEXUS_TsmMode tsmEnabled;       /* This allows app to turn off TSM during trick modes.
                                       Set to false or NEXUS_TsmMode_eDisabled for vsync mode.
                                       Set to true or NEXUS_TsmMode_eEnabled for HW-STC TSM mode.
                                       NEXUS_TsmMode_eSimulated also supported. See enum. */
    bool stcTrickEnabled;           /* Set this true if you are doing STC trick modes. This allows VideoDecoder to properly handle PTS error interrupts. */
    NEXUS_VideoDecoderReorderingMode reorderingMode; /* Sets the required reordering mode for the decoder */
    unsigned forceSourceFrameRate;  /* If 0, not forced. If not 0, then force the source frame rate to the specified value to
                                       enable exact rate trick modes. In units of 1/1000 Hz. So, 23976 = 23.976 Hz. */
    unsigned maxFrameRepeat;        /* if zero, there is no max. if non-zero, only repeat pictures this number of times for
                                       a minimum smoothness. only used in NEXUS_TsmMode_eSimulated. */
    bool lowLatencyDisplayForTrickModes; /* when in a trick mode, minimize latency in the display. This may decrease quality due to deinterlacing, but
                                       prevents different pictures on HD/SD simul outputs, and prevents large time gaps between decoder's current picture
                                       and display's current picture. */
} NEXUS_VideoDecoderTrickState;

/**
Summary:
Dedicated settings for the Playback module
**/
typedef struct NEXUS_VideoDecoderPlaybackSettings
{
    NEXUS_CallbackDesc firstPts; /* duplicate of NEXUS_PlaybackSettings.firstPts. This is provided so Playback can run an algorithm without colliding with
                                    application usage of this callback. */
    NEXUS_CallbackDesc firstPtsPassed; /* duplicate of NEXUS_PlaybackSettings.firstPtsPassed. This is provided so Playback can run an algorithm without colliding with
                                    application usage of this callback. */
} NEXUS_VideoDecoderPlaybackSettings;

/**
Summary:
Specifies a mode which controls the latency introduced in the video decoder's display queue
**/
typedef enum NEXUS_VideoDecoderLowLatencyMode
{
    NEXUS_VideoDecoderLowLatencyMode_eOff, /* no low latency algorithm */
    NEXUS_VideoDecoderLowLatencyMode_eFast, /* the low latency algo will
        attempt to maintain a fixed instantaneous latency at each display VSYNC
        interval.  This will achieve the latency target the quickest, but will
        pass input jitter directly to the display. Used for full-screen
        graphics and gaming. */
    NEXUS_VideoDecoderLowLatencyMode_eAverage, /* the low latency algo will
        attempt to maintain a fixed average latency over time.  This will take
        time to achieve the latency target, but has better input jitter
        tolerance than the fast mode. Used for full screen video. */
    NEXUS_VideoDecoderLowLatencyMode_eAdaptive, /* the low latency algo will
        attempt to adapt the target latency up or down between max and min to
        achieve better latency or better quality based on measuring video
        performance. Uses the slower measurement method of the average mode.
        Used for mixed graphics and video. */
    NEXUS_VideoDecoderLowLatencyMode_eMax
} NEXUS_VideoDecoderLowLatencyMode;

/**
Summary:
Settings to control the behavior of the low latency modes
**/
typedef struct NEXUS_VideoDecoderLowLatencySettings
{
    NEXUS_VideoDecoderLowLatencyMode mode;

    /* Applies to all modes */
    unsigned latency; /* in ms, target latency, rounded up to an integer
        multiple of the source picture time internally */

    /* applies to eAverage and eAdaptive */
    unsigned controlPeriod; /* in ms, period over which to execute the adaptive
        target latency algorithm */

    /* only applies to eAdaptive */
    unsigned minLatency; /* in ms, minimum target latency, rounded up to an
        integer multiple of the source picture time internally */
    unsigned maxLatency; /* in ms, maximum target latency, rounded up to an
        integer multiple of the source picture time internally */
} NEXUS_VideoDecoderLowLatencySettings;

/**
Summary:
Settings that are not used by most customers

Description:
This extra API allows the standard API to remain simple for most customers.
**/
typedef struct NEXUS_VideoDecoderExtendedSettings
{
    NEXUS_CallbackDesc dataReadyCallback; /* Fired each time a picture is made available to the Display.
                                             Call NEXUS_VideoDecoder_GetStatus and NEXUS_VideoDecoder_GetExtendedStatus from inside the
                                             callback to get information about the most recent picture. */

    bool s3DTVStatusEnabled;               /* Enable parsing of stream metadata for NEXUS_VideoDecoder3DTVStatus */
    NEXUS_CallbackDesc s3DTVStatusChanged; /* Fires when the 3DTV status changes.
                                              Call NEXUS_VideoDecoder_Get3DTVStatus to get the new 3DTV status. */
    unsigned s3DTVStatusTimeout;           /* Time in milliseconds before s3DTVStatusChanged fires due to lack of 3D status signal.
                                              If 0, then this timeout is ignored. */

    /* Decoder settings for streams with special encoding */
    bool ignoreDpbOutputDelaySyntax;      /* Ignore the AVC decoded picture buffer (DPB) output delay syntax. This assumes the decode and display ordering is the same.
                                             This must be set before NEXUS_VideoDecoder_Start() is called to take effect. */
    bool zeroDelayOutputMode;             /* Decoder will deliver picture to display queue as soon as it's decoded. This eliminates the pipeline used for picture reordering.
                                             This requires the pictures to be encoded in display order. For instance, an MPEG stream with only I's and P's, no B's.
                                             This feature is not supported on VP8 or VP9.
                                             This must be set before NEXUS_VideoDecoder_Start() is called to take effect. */
    bool earlyPictureDeliveryMode;        /* AVD FW will deliver the pictures to the DM before they are decoded. This reduces latency.
                                             This must be set before NEXUS_VideoDecoder_Start() is called to take effect. */
    bool svc3dEnabled;                    /* Enable 3D SVC decode. This must be set before NEXUS_VideoDecoder_Start() is called to take effect. */
    bool treatIFrameAsRap;                /* Tell the decoder to regard every AVC I-frame as a random access point. This is only true for some streams
                                             and cannot be enabled for all streams.
                                             This must be set before NEXUS_VideoDecoder_Start() is called to take effect. */
    bool ignoreNumReorderFramesEqZero;    /* Allow decoder to handle poorly encoded AVC streams.
                                             This must be set before NEXUS_VideoDecoder_Start() is called to take effect. */

    unsigned crcFifoSize;                 /* Defaults to zero, which disables CRC. If non-zero, CRC's will be captured and can be retrieved with NEXUS_VideoDecoder_GetCrc. */
    NEXUS_VideoDecoderLowLatencySettings lowLatencySettings; /* settings which
        control low latency behavior of the display queue */

    NEXUS_VideoEotf eotf;                 /* User-specified color grading / expected electro-optical transfer function.
                                             Forces color override if not set to invalid .
                                             This must be set before NEXUS_VideoDecoder_Start() is called to take effect. */
    NEXUS_ContentLightLevel contentLightLevel;
    NEXUS_MasteringDisplayColorVolume masteringDisplayColorVolume;
} NEXUS_VideoDecoderExtendedSettings;

/**
Summary:
Picture information for the currently displayed picture or the first decoded picture for a new decode.

Description:
NEXUS_VideoDecoderStreamInformation picture information differs from NEXUS_VideoDecoderStatus information in that
NEXUS_VideoDecoderStatus returns information on the currently displayed picture. When starting a new decode,
this could be the last picture from the previous decode.
NEXUS_VideoDecoderStreamInformation always gives picture information from this decode.
When starting decode, after the first picture is decoded, it will be held until its PTS matures.
NEXUS_VideoDecoderStreamInformation will have information about that first picture, before it is displayed.
After the first picture is displayed, NEXUS_VideoDecoderStreamInformation will return the same
picture information as NEXUS_VideoDecoderStatus.

Because this information is available before display, it is similar to the old MPEG sequence header interrupt.
However, after that first picture is displayed, it does not provided any advance warning like a sequence header interrupt.

Any changes in this information will generate a NEXUS_VideoDecoderSettings.streamChanged callback.

Used in NEXUS_VideoDecoder_GetStreamInformation
**/
typedef struct NEXUS_VideoDecoderStreamInformation
{
    bool valid; /* if false, there is no picture so ignore all data */

    uint32_t sourceHorizontalSize; /* width of active area. possibly clipped from coded size. */
    uint32_t sourceVerticalSize; /* height of active area. possibly clipped from coded size. */

    uint32_t codedSourceHorizontalSize; /* coded width */
    uint32_t codedSourceVerticalSize; /* coded height */

    uint32_t displayHorizontalSize; /* intended display width of active area needed to preserve correct aspect ratio. this is not the actual display width used. */
    uint32_t displayVerticalSize; /* intended display height of active area needed to preserve correct aspect ratio. this is not the actual display height used. */

    NEXUS_AspectRatio aspectRatio;
    uint16_t sampleAspectRatioX; /* Only valid if aspectRatio is NEXUS_AspectRatio_eSar. NEXUS_AspectRatio_eSar is only used for some codecs.
                                    If used, the source's aspect ratio is calculated as the ratio of sampleAspectRatioX/sampleAspectRatioY.
                                    sampleAspectRatioX and sampleAspectRatioY should not be used except as a ratio. */
    uint16_t sampleAspectRatioY; /* See comments for sampleAspectRatioX */
    NEXUS_VideoFrameRate frameRate;
    NEXUS_ColorPrimaries colorPrimaries;
    NEXUS_TransferCharacteristics transferCharacteristics; /* SDR, HDR10, etc. */
    NEXUS_TransferCharacteristics preferredTransferCharacteristics; /* use this for HLG, if TV does not support, use transferCharacteristics instead */
    NEXUS_MatrixCoefficients matrixCoefficients;
    bool frameProgressive;
    bool streamProgressive;
    int32_t horizontalPanScan;
    int32_t verticalPanScan;

    uint32_t lowDelayFlag;       /* delay flag from the sequence extension */
    uint32_t fixedFrameRateFlag; /* fixed_frame_rate_flag from AVC VUI (video usability info) */

    bool dolbyVision;
    NEXUS_VideoEotf eotf; /* the source's color grading / expected electro-optical transfer function; indicates the highest level of transfer function the source supports */
    NEXUS_ContentLightLevel contentLightLevel;
    NEXUS_MasteringDisplayColorVolume masteringDisplayColorVolume;
    unsigned colorDepth;         /* 8 or 10 bit source */
} NEXUS_VideoDecoderStreamInformation;

/**
Summary:
3DTV formats
**/
typedef enum NEXUS_VideoDecoder3DTVFormat
{
    NEXUS_VideoDecoder3DTVFormat_eNone,             /* 2D */
    NEXUS_VideoDecoder3DTVFormat_eSideBySideHalf,
    NEXUS_VideoDecoder3DTVFormat_eTopAndBottomHalf,
    NEXUS_VideoDecoder3DTVFormat_eFramePacking,     /* aka O/U full */
    NEXUS_VideoDecoder3DTVFormat_eMax
} NEXUS_VideoDecoder3DTVFormat;

/**
Summary:
3DTV status

Description:
The 3DTV status is set via messages/userdata contained in the stream.
We recommend that applications use the generic NEXUS_VideoDecoder3DTVStatus.format
and only use the codec-specific data for special cases.
**/
typedef struct NEXUS_VideoDecoder3DTVStatus
{
    NEXUS_VideoDecoder3DTVFormat format; /* codec-generic 3DTV status */

    NEXUS_VideoCodec codec; /* use this to select the codecData union member */
    union {
        /* for avc, this is the SEI frame packing message */
        struct {
            bool valid; /* True if SEI message is valid. If false, the following values are undefined. */
            struct {
                uint32_t flags;
                uint32_t framePackingArrangementId;
                uint32_t framePackingArrangementType;
                uint32_t contentInterpretationType;
                uint32_t frame0GridPositionX;
                uint32_t frame0GridPositionY;
                uint32_t frame1GridPositionX;
                uint32_t frame1GridPositionY;
                uint32_t framePackingArrangementReservedByte;
                uint32_t framePackingArrangementRepetitionPeriod;
            } data;
        } avc; /* set for NEXUS_VideoCodec_eH264 */

        struct {
            bool valid; /* True if mpeg2 data is valid. If false, the following values are undefined. */
            struct {
                uint16_t stereoVideoFormatSignalingType;
            } jp3d; /* ISO/IEC JTC1/SC29/WG11 */

        } mpeg2; /* set for NEXUS_VideoCodec_eMpeg2 */
    } codecData;
} NEXUS_VideoDecoder3DTVStatus;

/**
Summary:
Video decoder primer settings.

Description:
These settings can be changed before or after NEXUS_VideoDecoderPrimer_Start.

When decode is started with NEXUS_VideoDecoderPrimer_StopPrimerAndStartDecode, Nexus will calculate the difference
between the current STC and the PTS of all random access points (RAP) currently in the buffer. Nexus will then
select a single RAP from which to start decode. This decision affects the momentary behavior immediately after
decode is started, while a STC/PTS discrepancy exists.

pastTolerance and futureTolerance control the above decision in the following manner:

- If there are no RAPs that satisfy either tolerance, then the larger of the two tolerances is taken as a preference.
If pastTolerance > futureTolerance, then Nexus will select the nearest past RAP (STC > PTS).
If pastTolerance < futureTolerance, then Nexus will select the nearest future RAP (STC < PTS)
If pastTolerance == futureTolerance, then Nexus will select the nearest RAP from the current STC.

- If there are one or more RAPs that satisfy one or both tolerances, then Nexus will select the nearest RAP from
the current STC that satisfies a tolerance.
**/
typedef struct NEXUS_VideoDecoderPrimerSettings
{
    unsigned ptsOffset; /* Add an offset to the primer's TSM equation. Measured in 45Hz PTS units.
                           A separate ptsOffset must be applied to VideoDecoder if desired. */
    unsigned pastTolerance;   /* time (in PTS units, typically 45KHz for MPEG2TS) between the current STC and the past, within which
                                 Nexus will look for a RAP to start decode from */
    unsigned futureTolerance; /* time (in PTS units, typically 45KHz for MPEG2TS) between the current STC and the future, within which
                                 Nexus will look for a RAP to start decode from */

    bool ptsStcDiffCorrectionEnabled; /* If true, then correct the PTS-STC difference when starting decode,
                                         by applying an additional PTS offset to the video decoder.
                                         If not using sync_channel, this may worsen A/V sync.
                                         If using sync_channel, this may worsen end-to-end system delay. */
} NEXUS_VideoDecoderPrimerSettings;

/**
Summary:
Format of userdata that Nexus will filter after EIA-608/708 parsing for the Display module.

Description:
Some streams contain multiple formats of 608/708 data encoded into user data.
NEXUS_VideoDecoder_GetUserDataBuffer will return all user data in a raw form.
However, if NEXUS_VideoInputVbiSettings.closedCaptionEnabled is true, the EIA-608/708 parsed from the userdata must be filtered down to only one format.

MPEG userdata only parsers for eAtsc53, eScte20 and eScte21. H264/H265 parses for all formats.
**/
typedef enum NEXUS_UserDataFormat
{
    NEXUS_UserDataFormat_eAny = 0, /* Nexus will filter based on the first format it detects. */
    NEXUS_UserDataFormat_eAtsc53,
    NEXUS_UserDataFormat_eScte20,  /* formerly DVS 157 */
    NEXUS_UserDataFormat_eScte21,  /* formerly DVS 053 */
    NEXUS_UserDataFormat_eAtsc72,  /* H264/H265 userdata in SEI */
    NEXUS_UserDataFormat_eMax
} NEXUS_UserDataFormat;

/**
Memory configuration per VideoDecoder (that is, per channel)
Used in nexus_platform_memconfig.h, but cross-referenced if decoder config fails.
Also returned in
**/
typedef struct NEXUS_VideoDecoderMemory
{
    bool used;
    bool supportedCodecs[NEXUS_VideoCodec_eMax];
    NEXUS_VideoFormat maxFormat; /* max source format for this channel. if channel is unused, leave it as eNone */
    unsigned colorDepth; /* 8 or 10 bit decode */
    struct {
        unsigned maxNumber;
        unsigned maxWidth, maxHeight;
        unsigned colorDepth; /* 8 or 10 bit */
    } mosaic; /* Actual number of mosaics must not exceed total memory (approximately maxNumber*maxWidth*maxHeight).
                 Does not apply for StillDecoder */
    bool avc51Supported;
    bool dynamicPictureBuffers; /* If true, alloc from picture buffer at start time (specifically, NEXUS_VideoWindow_AddInput).
                                   This allows for video/graphics memory sharing at the risk of OOM due to fragmentation. */
    NEXUS_SecureVideo secure; /* allocate picture buffer memory for unsecure, secure or both */
} NEXUS_VideoDecoderMemory;

/**
Summary:
AVD CRC
**/
typedef struct NEXUS_VideoDecoderCrc
{
    struct {
        uint32_t luma, cb, cr;
    } top, bottom;
} NEXUS_VideoDecoderCrc;

/**
FIFO Status for a video decoder
**/
typedef struct NEXUS_VideoDecoderFifoStatus
{
    struct {
        uint32_t mostRecent;    /* PTS value most recently added to the FIFO */
        uint32_t leastRecent;   /* PTS value least recently added to the FIFO */
        bool valid;
    } pts;

    unsigned bufferLatency;     /* units of milliseconds, total latency from input to output (including CDB/ITB fifo and picture queue).
                                   this does not include latency in the display (e.g. capture buffers, deinterlacing) or latency before the CDB/ITB (e.g. RS/XC buffers).
                                   if backwards PTS discontinuity is detected, bufferLatency will be returned as 0.
                                   be aware that forward PTS discontinuities cannot be detected, so data may be unreliable. */
} NEXUS_VideoDecoderFifoStatus;

#define NEXUS_MAX_XVD_DEVICES 3

/**
Summary:
Settings used to configure the VideoDecoder module.

Description:

See Also:
NEXUS_VideoDecoderModule_GetDefaultSettings
NEXUS_VideoDecoderModule_Init
**/
typedef struct NEXUS_VideoDecoderModuleSettings
{
    unsigned avdHeapIndex[NEXUS_MAX_XVD_DEVICES]; /* Sets the heap[] index to be used per AVD core for picture data. */
    unsigned secondaryPictureHeapIndex[NEXUS_MAX_XVD_DEVICES]; /* Sets the heap[] index to be used per AVD core for secondary picture data. */
    struct {
        unsigned avdHeapIndex[NEXUS_MAX_XVD_DEVICES]; /* Sets the heap[] index to be used per AVD core for picture data. */
        unsigned secondaryPictureHeapIndex[NEXUS_MAX_XVD_DEVICES]; /* Sets the heap[] index to be used per AVD core for secondary picture data. */
    } secure;
    unsigned hostAccessibleHeapIndex; /* deprecated */
    unsigned mfdMapping[NEXUS_MAX_VIDEO_DECODERS]; /* Maps VideoDecoder handle to MFD block. */
    unsigned avdMapping[NEXUS_MAX_VIDEO_DECODERS]; /* Maps VideoDecoder handle to AVD device. Set to NEXUS_MAX_XVD_DEVICES if unused. */
    bool avdEnabled[NEXUS_MAX_XVD_DEVICES]; /* enables AVD cores. Defaults to true for all */

    NEXUS_VideoDecoderMemory memory[NEXUS_MAX_VIDEO_DECODERS];
    NEXUS_VideoDecoderMemory stillMemory[NEXUS_MAX_STILL_DECODERS];
    struct {
        struct {
            unsigned heapIndex; /* dynamically alloc memory from this heap. also, do search for MFD that matches this heap's MEMC. */
        } sid, dsp;
    } videoDecoderExtension;

    bool supportedCodecs[NEXUS_VideoCodec_eMax]; /* deprecated */
    NEXUS_VideoFormat maxDecodeFormat; /* deprecated */
    NEXUS_VideoFormat maxStillDecodeFormat; /* deprecated */
    int numDecodes; /* deprecated. used for non-memconfig platforms to populate capabilities. */
    int numStillDecodes; /* unused */
    int numMosaicDecodes; /* unused */

    struct { /* Heap sizes per AVD core. */
        unsigned general;   /* Host accessible memory like firmware, userdata. */
        unsigned secure;    /* CRR, for cabac bin buffer, use 0 for dynamic allocation */
        unsigned picture;   /* Picture buffer heap, use 0 for dynamic allocation  */
        unsigned secondaryPicture; /* Picture buffer for "split buffer" systems, use 0 for dynamic allocation */
    } heapSize[NEXUS_MAX_XVD_DEVICES];
    bool watchdogEnabled; /* Enable and process watchdog interrupts. Watchdog interrupts occur when the decoder stalls and must be reset. */

    bool deferInit; /* if set to true, HW and FW initialization will be deferred until actually used */
    unsigned debugLogBufferSize; /* required for NEXUS_VideoDecoderModule_SetDebugLog or export avd_monitor=# */
} NEXUS_VideoDecoderModuleSettings;

/**
Summary:
**/
typedef struct NEXUS_VideoDecoderPrimerCreateSettings
{
    unsigned fifoSize;
    unsigned maximumGops; /* unused */
} NEXUS_VideoDecoderPrimerCreateSettings;

#ifdef __cplusplus
}
#endif

#endif
