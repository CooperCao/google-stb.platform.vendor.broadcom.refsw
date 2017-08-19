/******************************************************************************
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
 ******************************************************************************/
#ifndef NEXUS_SIMPLE_ENCODER_H__
#define NEXUS_SIMPLE_ENCODER_H__

#include "nexus_types.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_recpump.h"
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#include "nexus_video_encoder_output.h"
#else
typedef unsigned NEXUS_VideoEncoderSettings;
typedef struct NEXUS_VideoEncoderStartSettings
{
    NEXUS_DisplayHandle input;
    NEXUS_VideoWindowHandle window;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_CallbackDesc dataReady;
} NEXUS_VideoEncoderStartSettings;
typedef unsigned NEXUS_VideoEncoderDescriptor;
typedef unsigned NEXUS_VideoEncoderStatus;
#endif
#ifdef NEXUS_HAS_AUDIO
#include "nexus_audio_encoder.h"
#include "nexus_audio_mux_output.h"
#else
typedef unsigned NEXUS_AudioEncoderCodecSettings;
typedef unsigned NEXUS_AudioMuxOutputFrame;
typedef unsigned NEXUS_AudioMuxOutputStatus;
#endif
#ifdef NEXUS_HAS_STREAM_MUX
#include "nexus_stream_mux.h"
#endif
#include "nexus_core_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS 4

typedef struct NEXUS_SimpleEncoder *NEXUS_SimpleEncoderHandle;

/**
Summary:
**/
NEXUS_SimpleEncoderHandle NEXUS_SimpleEncoder_Acquire(  /* attr{release=NEXUS_SimpleEncoder_Release} */
    unsigned id
    );

/**
Summary:
**/
void NEXUS_SimpleEncoder_Release(
    NEXUS_SimpleEncoderHandle handle
    );

/**
NEXUS_SimpleEncoderStartSettingsOutput and NEXUS_SimpleEncoderStartSettingsOutputVideo are
explicitly named to satisfy the Nexus perl parser requirement for parsing handles and callbacks.
The other substructs don't have the same requirements but could be typedef'd as needed.
**/
typedef struct NEXUS_SimpleEncoderStartSettingsOutputVideo
{
    NEXUS_VideoEncoderStartSettings settings;
    unsigned pid;
    bool index; /* NAV index */
    bool raiIndex; /* Index the random access indicator */
    NEXUS_KeySlotHandle keyslot;
} NEXUS_SimpleEncoderStartSettingsOutputVideo;

typedef struct NEXUS_SimpleEncoderStartSettingsOutput
{
    NEXUS_SimpleEncoderStartSettingsOutputVideo video;
    struct {
        bool passthrough;       /* If true, pass the original audio through without re-encoding */
        NEXUS_AudioCodec codec; /* Output codec (ignored if passthrough = true) */
        unsigned pid;
        unsigned sampleRate;    /* Output sample rate in Hz.  If 0 (default) the output sample rate will
                                       be based on the current default sample rate  */
        NEXUS_KeySlotHandle keyslot;
    } audio;
    struct {
        NEXUS_TransportType type;   /* If ES, App must call NEXUS_SimpleEncoder_GetVideoBuffer/GetAudioBuffer to retrieve frames */
        unsigned pcrPid;            /* if zero, no PCR */
        unsigned pmtPid;            /* if zero, no PSI */
        unsigned interval;          /* internal for psi insertion, in millseconds */
    } transport;
    struct {
        NEXUS_KeySlotHandle keyslot;
    } passthrough[NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS];
} NEXUS_SimpleEncoderStartSettingsOutput;

/**
Summary:
**/
typedef struct NEXUS_SimpleEncoderStartSettings
{
    struct {
        bool display; /* if true, encode graphics and video and audio from the main display. if false, use handles below. */
        NEXUS_SimpleVideoDecoderHandle video;
        NEXUS_SimpleAudioDecoderHandle audio;
    } input;
    struct {
        NEXUS_DisplayHandle display; /* client-opened display */
        unsigned nonRealTimeRate; /* Rate in units of NEXUS_NORMAL_PLAY_SPEED for non realtime muxing */
        bool useInitialPts;    /* (only used if nonRealTime=true) Enables seeding of initial PTS */
        uint32_t initialPts;   /* (only used if nonRealTime=true) Indicates the desired PTS for the first *video* frame. All other A/V timing parameters are adjusted accordingly (32-bits of 45Khz clock ticks) */
    } transcode;

    NEXUS_RecpumpHandle recpump; /* Recpump where encoded stream and index will be captured.
                                    Call NEXUS_Recpump_SetSettings before NEXUS_SimpleEncoder_Start; do not call after Start.
                                    Call NEXUS_Recpump_Start after NEXUS_SimpleEncoder_Start. */

    NEXUS_SimpleEncoderStartSettingsOutput output;

    NEXUS_PidChannelHandle passthrough[NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS]; /* Passthrough data.
                                       TS/PES layer will be re-done, but ES data will passthrough.
                                       Used for passthrough of pids like subtitle and teletext. */
} NEXUS_SimpleEncoderStartSettings;

/**
Summary:
**/
void NEXUS_SimpleEncoder_GetDefaultStartSettings(
    NEXUS_SimpleEncoderStartSettings *pSettings
    );

/**
Summary:
Start encoding
**/
NEXUS_Error NEXUS_SimpleEncoder_Start(
    NEXUS_SimpleEncoderHandle handle,
    const NEXUS_SimpleEncoderStartSettings *pSettings
    );

/**
Summary:
Stop encoding
**/
void NEXUS_SimpleEncoder_Stop(
    NEXUS_SimpleEncoderHandle handle
    );

/**
Summary:
Modify the behavior of the next NEXUS_SimpleEncoder_Stop call.
**/
typedef enum NEXUS_SimpleEncoderStopMode
{
    NEXUS_SimpleEncoderStopMode_eAll, /* NEXUS_SimpleEncoder_Stop will stop entire audio/video/mux pipeline. */
    NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly, /* Do not stop audio. Stops video encoder.
        After this, user can do an eAll stop or can call Start again with the same NEXUS_SimpleEncoderStartSettings. */
    NEXUS_SimpleEncoderStopMode_eMax
} NEXUS_SimpleEncoderStopMode;

/**
Summary:
Run-time settings which can be changed before or after Start
**/
typedef struct NEXUS_SimpleEncoderSettings
{
    struct {
        unsigned width, height;
        bool interlaced;
        unsigned refreshRate; /* units of 1/1000 Hz. 59940 = 59.94Hz, 60000 = 60Hz */
        NEXUS_Rect window; /* Default is 0,0,0,0, which is full screen.
                              Allows for downscale within width and height specified above. */
        NEXUS_ClipRect clip; /* Default is 0,0,0,0, which is no clip. */
        struct {
            bool overrideOrientation;
            NEXUS_VideoOrientation orientation;
        } display3DSettings;
    } video;
    NEXUS_VideoEncoderSettings videoEncoder;
    NEXUS_AudioEncoderCodecSettings audioEncoder; /* must set audioEncoder.codec to match NEXUS_SimpleEncoderStartSettings.audio.codec. */
    NEXUS_CallbackDesc resourceChanged;
    NEXUS_CallbackDesc finished;
    NEXUS_SimpleEncoderStopMode stopMode;
    struct {
        struct {
            bool video[1];
            bool audio[1];
        } enable;
    } streamMux; /* maps to NEXUS_StreamMuxSettings */
} NEXUS_SimpleEncoderSettings;

/**
Summary:
Get current run-time settings
**/
void NEXUS_SimpleEncoder_GetSettings(
    NEXUS_SimpleEncoderHandle handle,
    NEXUS_SimpleEncoderSettings *pSettings
    );

/**
Summary:
Change run-time settings
**/
NEXUS_Error NEXUS_SimpleEncoder_SetSettings(
    NEXUS_SimpleEncoderHandle handle,
    const NEXUS_SimpleEncoderSettings *pSettings
    );

/**
Summary:
Get a video encoder buffer if outputting to memory
**/
NEXUS_Error NEXUS_SimpleEncoder_GetVideoBuffer(
    NEXUS_SimpleEncoderHandle handle,
    const NEXUS_VideoEncoderDescriptor **pBuffer, /* [out] attr{memory=cached} pointer to NEXUS_VideoEncoderDescriptor structs */
    size_t *pSize, /* [out] number of NEXUS_VideoEncoderDescriptor elements in pBuffer */
    const NEXUS_VideoEncoderDescriptor **pBuffer2, /* [out] attr{memory=cached} pointer to NEXUS_VideoEncoderDescriptor structs after wrap around */
    size_t *pSize2 /* [out] number of NEXUS_VideoEncoderDescriptor elements in pBuffer2 */
    );

/**
Summary:
Return a video encoder buffer if outputting to memory
**/
NEXUS_Error NEXUS_SimpleEncoder_VideoReadComplete(
    NEXUS_SimpleEncoderHandle handle,
    unsigned descriptorsCompleted /* must be <= pSize+pSize2 returned by last NEXUS_SimpleEncoder_GetVideoBuffer call. */
    );

/**
Summary:
Get an audio encoder buffer if outputting to memory
**/
NEXUS_Error NEXUS_SimpleEncoder_GetAudioBuffer(
    NEXUS_SimpleEncoderHandle handle,
    const NEXUS_AudioMuxOutputFrame **pBuffer, /* [out] attr{memory=cached} pointer to NEXUS_AudioMuxOutputFrame structs */
    size_t *pSize, /* [out] number of NEXUS_AudioMuxOutputFrame elements in pBuffer */
    const NEXUS_AudioMuxOutputFrame **pBuffer2, /* [out] attr{memory=cached} pointer to NEXUS_AudioMuxOutputFrame structs after wrap around */
    size_t *pSize2 /* [out] number of NEXUS_AudioMuxOutputFrame elements in pBuffer2 */
    );

/**
Summary:
Return an audio encoder buffer if outputting to memory
**/
NEXUS_Error NEXUS_SimpleEncoder_AudioReadComplete(
    NEXUS_SimpleEncoderHandle handle,
    unsigned descriptorsCompleted /* must be <= pSize+pSize2 returned by last NEXUS_SimpleEncoder_GetAudioBuffer call. */
    );

/**
Summary:
Encoder Status
**/
typedef struct NEXUS_SimpleEncoderVideoStatus
{
    NEXUS_MemoryBlockHandle bufferBlock; /* block handle for NEXUS_VideoEncoderPicture.offset */
    NEXUS_MemoryBlockHandle metadataBufferBlock; /* block handle for NEXUS_VideoEncoderPicture.offset when it's used to carry metadata  */
    uint32_t errorFlags;
    uint32_t eventFlags;

    unsigned errorCount; /* Total number of errors that has occurred */
    unsigned picturesReceived; /* Number of pictures received at the input to the encoder */
    unsigned picturesDroppedFRC;  /* Number of pictures that the encoder has configured to drop in order to follow the requested
                             frame rate (Frame Rate Conversion) */
    unsigned picturesDroppedHRD; /* Number of pictures that the encoder has dropped because of a drop request from the Rate Control.
                             The Rate Control may decide to drop picture in order to maintain the HRD buffer model. */
    unsigned picturesDroppedErrors; /* Number of pictures that the encoder has configured to drop because encoder did not finish the
                             processing of the previous pictures on time and buffers are full. */
    unsigned picturesEncoded; /* Number of pictures output by the encoder */
    uint32_t pictureIdLastEncoded; /* Picture ID of the current picture being encoded. This is set as soon as the CME block decides to work on a picture. */
    unsigned picturesPerSecond; /* Averages pictures per second output by the encoder */
    struct {
        size_t fifoDepth; /* Current depth of video encoder output fifo in bytes */
        size_t fifoSize;  /* Size of video encoder output fifo in bytes. This could be less than the OpenSettings fifoSize because of
                             internal alignment requirements. */
    } data, index;
    struct {
        unsigned firmware;
    } version;
    struct {
        size_t elementSize;
        NEXUS_MemoryBlockHandle buffer; /* Memory Block containing the encoder internal BDBG_Fifo */
        unsigned offset; /* Offset from start of memory block where BDBG_Fifo begins */
    } debugLog;
} NEXUS_SimpleEncoderVideoStatus;

/**
Summary:
Encoder Status
**/
typedef struct NEXUS_SimpleEncoderAudioStatus
{
    NEXUS_MemoryBlockHandle bufferBlock;         /* block handle for NEXUS_AudioMuxOutputFrame.offset */
    NEXUS_MemoryBlockHandle metadataBufferBlock; /* block handle for NEXUS_AudioMuxOutputFrame.offset when it's used to carry metadata  */
    unsigned numFrames;                 /* Number of successfully encoded frames */
    unsigned numErrorFrames;            /* Number of error frames */
    unsigned numDroppedFrames;          /* Number of frames dropped due to overflow */
    struct {
        unsigned fifoDepth;                  /* depth in bytes of buffer */
        unsigned fifoSize;                   /* size in bytes of buffer */
    } data, index;
} NEXUS_SimpleEncoderAudioStatus;

/**
Summary:
Encoder Status
**/
typedef struct NEXUS_SimpleEncoderStatus
{
    struct {
        bool enabled; /* set true if the underlying VideoEncoder is available */
        unsigned index; /* index of underlying VideoEncoder */
    } videoEncoder;
    NEXUS_SimpleEncoderVideoStatus video;
    NEXUS_SimpleEncoderAudioStatus audio;
    struct {
        struct {
            uint32_t video[1];
            uint32_t audio[1];
        } currentTimestamp; /* most recent timestamp (DTS) completed (in 45 Khz) */
    } streamMux;
} NEXUS_SimpleEncoderStatus;

/**
Summary:
**/
void NEXUS_SimpleEncoder_GetStatus(
    NEXUS_SimpleEncoderHandle handle,
    NEXUS_SimpleEncoderStatus *pStatus /* [out] */
    );

/**
Summary:
Read CMP CRC

Description:
Will set NEXUS_DisplaySettings.crcQueueSize if zero.
**/
NEXUS_Error NEXUS_SimpleEncoder_GetCrcData(
    NEXUS_SimpleEncoderHandle handle,
    NEXUS_DisplayCrcData *pEntries, /* attr{nelem=numEntries;nelem_out=pNumReturned} */
    unsigned numEntries,
    unsigned *pNumReturned
    );

NEXUS_Error NEXUS_SimpleEncoder_InsertRandomAccessPoint(
    NEXUS_SimpleEncoderHandle handle
    );

/**
Insert system data into the stream output, like PSI.

You must turn off internal PSI insertion by setting NEXUS_SimpleEncoderStartSettingsOutput.transport.pmtPid to 0.
Each insertion is a one-shot.
Use NEXUS_StreamMux_GetDefaultSystemData to initialize the struct.

The memory pointed to by NEXUS_StreamMuxSystemData.pData is not copied. It must remain valid until consumed.
You can monitor that consumption using NEXUS_SimpleEncoder_GetCompletedSystemDataBuffers.
**/
NEXUS_Error NEXUS_SimpleEncoder_AddSystemDataBuffer(
    NEXUS_SimpleEncoderHandle handle,
    const NEXUS_StreamMuxSystemData *pSystemDataBuffer
    );

/**
Learn the number of NEXUS_StreamMuxSystemData entries consumed since the last call to NEXUS_SimpleEncoder_GetCompletedSystemDataBuffers.
**/
void NEXUS_SimpleEncoder_GetCompletedSystemDataBuffers(
    NEXUS_SimpleEncoderHandle handle,
    unsigned *pCompletedCount /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif
