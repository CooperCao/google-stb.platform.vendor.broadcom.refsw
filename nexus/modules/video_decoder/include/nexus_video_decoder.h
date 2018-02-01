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
 **************************************************************************/
#ifndef NEXUS_VIDEO_DECODER_H__
#define NEXUS_VIDEO_DECODER_H__

#include "nexus_types.h"
#include "nexus_video_types.h"
#include "nexus_video_decoder_types.h"
#include "nexus_video_decoder_userdata.h"
#include "nexus_striped_surface.h"
#include "nexus_rave.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=*******************************************
VideoDecoder is able to decode compressed digital streams from transport.
It outputs the decoded pictures to one or more Display VideoWindows.

VideoDecoder is also able to extract "userdata" from digital streams. If the decoder
is connected to a VideoWindow, this userdata will be sent directly to that window for
VBI encoding. VBI encoding control is done at the window.
Userdata can also be captured for processing by the application using VideoDecoder api's.
**********************************************/

typedef enum NEXUS_VideoDecoderSecureType
{
    NEXUS_VideoDecoderSecureType_eUnsecure,         /* GLR */
    NEXUS_VideoDecoderSecureType_eSecure,           /* URR */
    NEXUS_VideoDecoderSecureType_eSecureTranscode,  /* URRT */
    NEXUS_VideoDecoderSecureType_eMax
} NEXUS_VideoDecoderSecureType;

/*
Summary:
Settings for opening a new VideoDecoder.
*/
typedef struct NEXUS_VideoDecoderOpenSettings
{
    unsigned fifoSize;                /* Size of compressed data buffer in bytes. If 0, it will pick a value based on codec-capabilities.
                                         The minimum size depends on the maximum bitrate of your streams.
                                         For live, it also depends on the PCR/PTS difference. The CDB must hold all the data until the PCR arrives for a given PTS.
                                         For playback, it also depends on the video/audio muxing difference. The CDB must hold all the data until the audio's PTS matures.
                                         If the size chosen is too small, you will have TSM errors and stuttering on the screen. */
    unsigned userDataBufferSize;      /* Size of userdata buffer in bytes. Increase this for high bitrate userdata applications. */

    NEXUS_HeapHandle pictureHeap;     /* Optional picture buffer heap. This overrides the NEXUS_VideoDecoderModuleInternalSettings.avdHeapIndex setting. */
    NEXUS_HeapHandle secondaryPictureHeap; /* Same as above for split picture buffer systems */
    NEXUS_VideoDecoderSecureType secureVideo; /* Select pictureHeap and secondaryPictureHeap for this decoder and any connected window from secure heaps.
                                         If false (default) and only secure buffers are available, they will be used. */

    bool avc51Enabled;                /* Enable AVC 5.1 mode */
    bool svc3dSupported;              /* Enable support for 3D SVC mode */
    bool excessDirModeEnabled;        /* Enable excess direct memory mode for this decoder instance */

    unsigned itbFifoSize;             /* Size of ITB in bytes. If 0, it will pick a value in proportion to fifoSize based on typical codec requirements. */

    bool enhancementPidChannelSupported;
    NEXUS_HeapHandle cdbHeap;         /* Heap for CDB. For compressed restricted region (CRR), set to heap[NEXUS_VIDEO_SECURE_HEAP]. */
} NEXUS_VideoDecoderOpenSettings;

/*
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
*/
void NEXUS_VideoDecoder_GetDefaultOpenSettings(
    NEXUS_VideoDecoderOpenSettings *pOpenSettings   /* [out] default settings */
    );

/*
Summary:
Open a new VideoDecoder.

Description:
Each VideoDecoder instance is able to decode a single program.

See Also:
NEXUS_VideoDecoder_GetDefaultOpenSettings
NEXUS_VideoDecoder_Close
NEXUS_VideoDecoderModule_Init - must init VideoDecoder module before calling this function
NEXUS_VideoDecoder_OpenMosaic
*/
NEXUS_VideoDecoderHandle NEXUS_VideoDecoder_Open( /* attr{destructor=NEXUS_VideoDecoder_Close}  */
    unsigned index,
    const NEXUS_VideoDecoderOpenSettings *pOpenSettings /* attr{null_allowed=y} */
    );

/*
Summary:
Close a VideoDecoder.
*/
void NEXUS_VideoDecoder_Close(
    NEXUS_VideoDecoderHandle handle
    );

/*
Summary:
Get open settings
*/
void NEXUS_VideoDecoder_GetOpenSettings(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderOpenSettings *pOpenSettings   /* [out] */
    );

/*
Summary:
Get the current NEXUS_VideoDecoderSettings from the decoder.
*/
void NEXUS_VideoDecoder_GetSettings(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderSettings *pSettings   /* [out] */
    );

/*
Summary:
Set new NEXUS_VideoDecoderSettings to the decoder.
*/
NEXUS_Error NEXUS_VideoDecoder_SetSettings(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderSettings *pSettings
    );

/*
Summary:
Get default settings for the structure.

Description:
This is required in order to make application code resilient to the addition of new strucutre members in the future.
The pidChannel must always be specified by the user.
*/
void NEXUS_VideoDecoder_GetDefaultStartSettings(
    NEXUS_VideoDecoderStartSettings *pSettings /* [out] */
    );

/**
Summary:
Start decoding a program

Description:

See Also:
NEXUS_VideoDecoder_GetDefaultStartSettings
**/
NEXUS_Error NEXUS_VideoDecoder_Start(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderStartSettings *pSettings /* the video program to be decoded */
    );

/**
Summary:
Stop decoding a program
**/
void NEXUS_VideoDecoder_Stop(
    NEXUS_VideoDecoderHandle handle
    );

/**
Summary:
Immediately discard all pictures in the decoder's fifo, including pictures which have been
decoded and those waiting to be decoded.

Description:
The currently displayed picture will continue being displayed.
**/
void NEXUS_VideoDecoder_Flush(
    NEXUS_VideoDecoderHandle handle
    );

/*
Summary:
Get current status information from the video decoder.
*/
NEXUS_Error NEXUS_VideoDecoder_GetStatus(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderStatus *pStatus   /* [out] */
    );

/*
Summary:
Returns the abstract NEXUS_VideoInput connector for the VideoDecoder.
This connector is passed to a VideoWindow to display the decoded video.

Description:
The video connector is typically passed into NEXUS_VideoWindow_AddInput to show video.

Nexus requires that NEXUS_VideoWindow_AddInput be done before NEXUS_VideoDecoder_Start.
AddInput gives the VEC vsync interrupt heartbeat to the AVD HW and also initializes the underlying state machine.
Mirroring this order, NEXUS_VideoDecoder_Stop should be called before NEXUS_VideoWindow_RemoveInput.
However, if NEXUS_VideoWindow_RemoveInput is called first, Stop will be called automatically.

Setting NEXUS_VideoDecoderSettings.manualPowerState = true allows you to disconnect from the display without stopping and restarting the decoder.
You must still AddInput before Start, but RemoveInput will not be automatically called on Stop.
The main use case is doing RemoveInput/AddInput swaps for Main and PIP without stopping and restarting the decoder.
See nexus/examples/video/digital_pip_swap.c for an example.
*/
NEXUS_VideoInputHandle NEXUS_VideoDecoder_GetConnector( /* attr{shutdown=NEXUS_VideoInput_Shutdown} */
    NEXUS_VideoDecoderHandle handle
    );

/**
Summary:
Get information about the last picture decoded.
**/
NEXUS_Error NEXUS_VideoDecoder_GetStreamInformation(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderStreamInformation *pStreamInformation /* [out] */
    );

/**
Summary:
Set the PTS where decode should start. All pictures before that PTS will be discarded.

Must be called before NEXUS_VideoDecoder_Start.
See NEXUS_VideoDecoderStartSettings.pauseAtStartPts for initial decoder pause option.
**/
NEXUS_Error NEXUS_VideoDecoder_SetStartPts(
    NEXUS_VideoDecoderHandle handle,
    uint32_t pts
    );

/**
Summary:
Discover if a codec is supported.

Description:
Codecs may not be supported because of lack of hardware support, compile-time options, or
run-time options like module memory settings.
**/
void NEXUS_VideoDecoder_IsCodecSupported(
    NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_VideoCodec codec,
    bool *pSupported
    );

typedef struct NEXUS_VideoDecoderCodecCapabilities
{
    bool supported;
    NEXUS_VideoProtocolProfile protocolProfile;
    NEXUS_VideoProtocolLevel protocolLevel;
    unsigned colorDepth;
} NEXUS_VideoDecoderCodecCapabilities;

void NEXUS_VideoDecoder_GetCodecCapabilities(
    NEXUS_VideoDecoderHandle videoDecoder,
    NEXUS_VideoCodec codec,
    NEXUS_VideoDecoderCodecCapabilities *pCodecCapabilities
    );

/*
Summary:
Set manual power state

Description:
If NEXUS_VideoDecoderSettings.manualPowerState is false (default), you do not need to call this function.
The default behavior is that NEXUS_VideoDecoder_SetPowerState(false) is automatically called on NEXUS_VideoWindow_RemoveInput and
NEXUS_VideoDecoder_SetPowerState(true) is called on NEXUS_VideoWindow_AddInput.
This results in lower power consumption.
However, powering down requires that the video decoder must be stopped and the current picture is lost.

One reason to set manualPowerState to false is to allow a video decoder to be swapped between two video windows
without having to stop decode or without having to lose the current picture.
See nexus/examples/video/digital_pip_swap.c for an example.

The following rules apply:
You cannot power down if currently connected to a video window.
You cannot power up unless you have connected to a video window at least once.

Because NEXUS_VideoDecoder_SetPowerState exposes low level control of internal state, the required order of operations is mutable.
It is possible that calling requirements may be tightened, loosened, or rearranged.
*/
NEXUS_Error NEXUS_VideoDecoder_SetPowerState(
    NEXUS_VideoDecoderHandle handle,
    bool powerUp
    );

/**
Summary:
Temporarily detach the video decoder's RAVE context for external use.

Description:
One use of the detached RAVE context is NEXUS_PidChannelScramblingSettings.raveContext.
**/
NEXUS_RaveHandle NEXUS_VideoDecoder_DetachRaveContext(
    NEXUS_VideoDecoderHandle handle
    );

/**
Summary:
Temporarily detach the video decoder's enhancement RAVE context for external use.

Description:
See NEXUS_VideoDecoder_DetachRaveContext for usage details.
**/
NEXUS_RaveHandle NEXUS_VideoDecoder_DetachEnhancementRaveContext(
    NEXUS_VideoDecoderHandle handle
    );

/**
Summary:
Re-attach a RAVE context that was detached with NEXUS_VideoDecoder_DetachRaveContext.

Description:
This function will fail if the RAVE context is not the one that was detached.
**/
NEXUS_Error NEXUS_VideoDecoder_AttachRaveContext(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_RaveHandle rave
    );

/**
Summary:
Get a decoded frame

Description:
If NEXUS_VideoDecoderStartSettings.appDisplayManagement is true,
this call will return the next decoded frame to the application.
It must be returned with NEXUS_VideoDecoder_ReturnDecodedFrames.

If NEXUS_VideoDecoderStartSettings.appDisplayManagement is false,
this call reads from the display queue, but the queue will not
wait. So it's not guaranteed to return every frame. It get every
frame you must poll frequently or set appDisplayManagement.
*/
NEXUS_Error NEXUS_VideoDecoder_GetDecodedFrames(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderFrameStatus *pStatus,  /* attr{nelem=numEntries;nelem_out=pNumEntriesReturned} [out] */
    unsigned numEntries,
    unsigned *pNumEntriesReturned /* [out] */
    );

/**
Summary:
Get Default Settings for a returned frame
*/
void NEXUS_VideoDecoder_GetDefaultReturnFrameSettings(
    NEXUS_VideoDecoderReturnFrameSettings *pSettings    /* [out] */
    );

/**
Summary:
Return a decoded frame

Description:
If NEXUS_VideoDecoderStartSettings.appDisplayManagement is true,
this call will display or drop a frame returned from
NEXUS_VideoDecoder_GetDecodedFrames.  Once returned, the frame
can not be reused and will become invalid.

If NEXUS_VideoDecoderStartSettings.appDisplayManagement is false,
this call is not currently required.
*/
NEXUS_Error NEXUS_VideoDecoder_ReturnDecodedFrames(
    NEXUS_VideoDecoderHandle handle,
    const NEXUS_VideoDecoderReturnFrameSettings *pSettings, /* attr{null_allowed=y;nelem=numFrames} Settings for each returned frame.  Pass NULL for defaults. */
    unsigned numFrames                                      /* Number of frames to return to the decoder */
    );

/**
Summary:
video decoder module capabilities
**/
typedef struct NEXUS_VideoDecoderCapabilities
{
    unsigned numVideoDecoders; /* total number of AVD-based decoders. base index is always 0. */
    NEXUS_VideoDecoderMemory memory[NEXUS_MAX_VIDEO_DECODERS];
    NEXUS_VideoDecoderMemory stillMemory[NEXUS_MAX_STILL_DECODERS];
    struct {
        unsigned avdIndex; /* mapping to HW decoder device */
        struct {
            unsigned colorDepth; /* 8 or 10 bit capability of MFD. If less than decoder (see memory[].colorDepth), it will downconvert. */
            unsigned index; /* MFD index */
        } feeder;
    } videoDecoder[NEXUS_MAX_VIDEO_DECODERS];
    unsigned numStcs; /* total number of XPT STC broadcasts accessible by AVD hardware */
    struct {
        unsigned total;
        unsigned baseIndex;
        bool useForVp6;
    } dspVideoDecoder; /* DSP-based decoders */
    struct {
        unsigned total;
        unsigned baseIndex;
        bool useForMotionJpeg;
    } sidVideoDecoder; /* SID-based decoders */
    struct {
        unsigned total;
        unsigned baseIndex;
        bool useForVp9;
    } softVideoDecoder; /* CPU-based decoders */
} NEXUS_VideoDecoderCapabilities;

/**
Summary:
video decoder module capabilities
**/
void NEXUS_GetVideoDecoderCapabilities(
    NEXUS_VideoDecoderCapabilities *pCapabilities
    );

typedef struct NEXUS_VideoDecoderMultiPassDqtData
{
    unsigned intraGopPictureIndex; /* Last intra gop picture index returned by decoder. Needed to advance MP DQT to previous GOP. */
    unsigned openGopPictures;
} NEXUS_VideoDecoderMultiPassDqtData;

/**
For NEXUS_VideoDecoderDqtMode_eMultiPass, read metadata from decoder which must be returned to host player.
Returns non-zero if none available.
**/
NEXUS_Error NEXUS_VideoDecoder_ReadMultiPassDqtData(
    NEXUS_VideoDecoderHandle handle,
    NEXUS_VideoDecoderMultiPassDqtData *pData
    );

typedef struct NEXUS_VideoDecoderModuleStatistics
{
    unsigned maxDecodedWidth, maxDecodedHeight; /* max with and height decoded since NEXUS_Platform_Init */
} NEXUS_VideoDecoderModuleStatistics;

void NEXUS_VideoDecoderModule_GetStatistics(
    NEXUS_VideoDecoderModuleStatistics *pStats
    );

#ifdef __cplusplus
}
#endif

#endif
