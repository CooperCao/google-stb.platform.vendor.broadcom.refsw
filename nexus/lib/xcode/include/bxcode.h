/***************************************************************************
*     (c)2014 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef BXCODE_H__
#define BXCODE_H__

#include "nexus_types.h"
#include "nexus_surface.h"
#include "nexus_platform_features.h"
#ifdef NEXUS_HAS_DISPLAY
#include "nexus_display.h"
#endif
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#include "nexus_audio_encoder.h"
#include "nexus_audio_mux_output.h"
#include "nexus_stream_mux.h"
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input.h"
#endif
#include "nexus_video_decoder.h"
#include "nexus_video_window.h"
#include "nexus_audio_decoder.h"
#include "nexus_playback.h"
#include "nexus_recpump.h"
#include "nexus_parser_band.h"

/* TODO: this API is under development, subject to changes */

/* This include file defines the public interfaces to the BXCode (Broadcom XCode
   Library)

   This includes:
   1.  Public BXCode macros, datatypes and enums.
   2.  Function prototypes for BXCode APIs.

   The API functions are grouped into:
   1.  Open/Close;
   2.  Start/Stop;
   3.  GetSettings/SetSettings;
   4.  GetStatus;
   5.  Input data feeding;
   6.  Output data fetch;

   TODO: add usage example snippets.
-----------------------------------
Usages:
   1. TS input file with MPEG2 video -> TTS output file with AVC transcode.
    NEXUS_Platform_Init();
    hBxcode = BXCode_Open(0, NULL);
    BXCode_GetSettings(hBxcode, &settings);
    settings.video.width  = 640;
    settings.video.height = 480;
    settings.video.encoder.bitrateMax = 1000000;
    BXCode_SetSettings(hBxcode, &settings);
    BXCode_GetDefaultStartSettings(&startSettings);
    startSettings.inputType = BXCode_InputType_eFile;
    startSettings.input.file.data  = "videos/cnnticker.mpg";
    startSettings.input.file.type  = NEXUS_TransportType_eTs;
    startSettings.input.file.vPid = 0x21;
    startSettings.input.file.vCodec = NEXUS_VideoCodec_eMpeg2;
    startSettings.input.file.aPid[0] = 0x22;
    startSettings.input.file.aCodec[0] = NEXUS_AudioCodec_eMpeg;
    startSettings.input.file.eofDone.callback = doneHandler;
    startSettings.input.file.eofDone.context  = doneEvent;
    startSettings.output.video.pid = 0x12;
    startSettings.output.audio[0].pid = 0x13;
    startSettings.output.transport.type = BXCode_OutputType_eTs;
    startSettings.output.transport.config.ts.pcrPid = 0x11;
    startSettings.output.transport.config.ts.pmtPid = 0x10;
    startSettings.output.transport.config.ts.file = "videos/output.ts";
    BXCode_Start(hBxcode, &startSettings);

    BKNI_WaitForEvent(&doneEvent, BKNI_INFINITE);

    BXCode_Stop(hBxcode);
    BXCode_Close(hBxcode);
    NEXUS_Platform_Uninit();

   2. TS stream playpump input -> HLS segmented TS output (TODO)

*/

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
*  Define public BXCode datatypes and enums.
**************************************************************************/

/* opaque handle */
typedef struct BXCode_P_Context *BXCode_Handle;

/* max audio PIDs per xcoder context: supports up to 6x audios */
#define BXCODE_MAX_AUDIO_PIDS    6

/* FNRT video xcoder pipes per xcoder context */
#define BXCODE_MAX_VIDEO_PIPES   NEXUS_NUM_VIDEO_ENCODERS

/* input type */
typedef enum BXCode_InputType
{
    BXCode_InputType_eFile, /* container file */
    BXCode_InputType_eHdmi, /* HDMI Rx */
    BXCode_InputType_eLive, /* live */
    BXCode_InputType_eStream, /* stream or ip */
    BXCode_InputType_eImage, /* YUV image input from image decoder or camera */
    BXCode_InputType_eMax
} BXCode_InputType;

/* output type */
typedef enum BXCode_OutputType
{
    BXCode_OutputType_eTs, /* can be fetched via stream output interface (GetBuffer/ReadComplete) or stored to file */
    BXCode_OutputType_eEs, /* must be fetched via stream output interface (GetBuffer/ReadComplete) */
    BXCode_OutputType_eMp4File, /* must be stored to output file */
    BXCode_OutputType_eMax
} BXCode_OutputType;

/*************************************************************************
*  Define public BXCode functions.
**************************************************************************/

/**
Video and audio decoder/encode handles are opened internally.
**/
typedef struct BXCode_OpenSettings
{
    /* app manages timebase allocation */
    NEXUS_Timebase                 timebase;

    /* There needs to be load balance among multiple local displays and xcodes if multiple DSP cores exist.
       Defaults to NEXUS_ANY_ID and BXCode internally tries to balance by evenly split xcodes among DSPs */
    unsigned                       audioDspId;

    /* It's recommended to allocate top-down started from (NEXUS_NUM_VIDEO_DECODERS-1).
       Defaults to NEXUS_ANY_ID and BXCode internally will try to iterate top-down to find one unused. */
    unsigned                       videoDecoderId;

    /* to customize memory config */
    NEXUS_VideoEncoderOpenSettings videoEncoder;

    /* number of video pipes for FNRT(1x file input -> Nx parallel xcode pipes -> 1x file output;
      if 0, no video transcoding; default 1 for single video transcoder.
     */
    unsigned                       vpipes;

    /* if true, defer decoder/encoder open to start time */
    bool                           deferOpen;

} BXCode_OpenSettings;

/**
Summary:
**/
void BXCode_GetDefaultOpenSettings(
    BXCode_OpenSettings *pSettings   /* [out] */
    );

/**
Summary:
**/
BXCode_Handle BXCode_Open(
    unsigned                   id,
    const BXCode_OpenSettings *pSettings /* Pass NULL for default settings */
    );

/**
Summary:
**/
void BXCode_Close(
    BXCode_Handle hBxcode
    );

/**
Summary:
Static settings
**/
typedef struct BXCode_StartSettings
{
    bool nonRealTime;

    struct {
        BXCode_InputType      type;

        /* file input type or common */
        const char           *data;  /* relative file path from current directory. */
        const char           *index; /* relative index file path from current directory. */
        NEXUS_TransportType   transportType;  /* container type (ts, pes, mp4, mkv etc) */
        NEXUS_TransportTimestampType timestampType; /* support mod300 or binary TTS format */
        unsigned              vPid;  /* if zeo, no video */
        NEXUS_VideoCodec      vCodec;
        unsigned              aPid[BXCODE_MAX_AUDIO_PIDS]; /* multi-audio programs; if zero, no audio */
        NEXUS_AudioCodec      aCodec[BXCODE_MAX_AUDIO_PIDS];
        unsigned              numTsUserDataPids; /* support TS layer userdata passthrough. 0 to disable; unsigned(-1) to pass through all */
        unsigned              userdataPid[NEXUS_MAX_MUX_PIDS]; /* Default 0 to auto detect it from PMT */
        NEXUS_CallbackDesc    eofDone; /* to notify app the file transcoder reaches EOF */
        bool                  loop; /* looping the input file */

        /* stream input type specific */
        /* Note, if app allocates data feed buffer, it should use the default NEXUS user heap */
        bool                  userBuffer; /* if true: app allocated stream buffer and scatter/gather feed via BXCode_Input_SubmitDescriptor;
                                                          else, xcode internal allocates buffer and app feeds by BXCode_Input_GetBuffer/WriteComplete */
        /* playpump source: App to feed data stream via BXCode_Input_SubmitDescriptor or BXCode_Input_GetBuffer/WriteComplete */
        /* image input: App to feed surface via NEXUS_VideoImageInput_PushSurface/NEXUS_VideoImageInput_RecycleSurface */
        NEXUS_CallbackDesc    dataCallback; /* stream/image input Callback when space becomes available. */

        /* live input type specific: MP2TS tuner or streamer input */
        NEXUS_ParserBand      parserBand; /* [in] */
        unsigned              pcrPid; /* for live input timebase recovery */

        /* hdmi input type specific */
        NEXUS_HdmiInputHandle hdmiInput; /* [in] */
    } input;

    struct {
        struct {
            NEXUS_VideoEncoderStartSettings encoder; /* display/window/stc opened internally by default */
            unsigned                 pid; /* if zeo, no video */
            bool                     index; /* NAV or RAI index */
        } video;
        struct {
            bool                     passthrough; /* If true, pass the original audio through without re-encoding */
            NEXUS_AudioCodec         codec; /* Output codec (ignored if passthrough = true) */
            unsigned                 pid; /* if zeo, no audio */
        } audio[BXCODE_MAX_AUDIO_PIDS]; /* up to 6x audio PIDs per context */
        struct {
            BXCode_OutputType type;

            /* TS output type */
            /* if file=NULL, app must call BXCode_Output_GetDescriptors/ReadComplete to retrieve TS stream; else output TS to file. */
            NEXUS_TransportTimestampType timestampType; /* support TTS format */
            unsigned         userdataPid[NEXUS_MAX_MUX_PIDS]; /* support TS layer userdata passthrough with PID remap. Default 0 without PID remap */
            unsigned         pcrPid;   /* if zero, no PCR */
            unsigned         pmtPid;   /* if zero, no PSI */
            bool             segmented;    /* if true, ts output is segmented with PAT/PMT insertion right before each RAI video packets,
                                                                 and RAI index is enabled instead of NAV index. App could specify segment duration via
                                                                 BXCode_Settings.video.encoder.streamStructure.duration. */
            unsigned         intervalPsi; /* internal for psi insertion, in millseconds; */
            const char      *file;  /* output TS data file path relative from current directory. If NULL, app needs to fetch output stream via
                                                                 BXCode_Output_GetBuffer/ReadComplete APIs. */
            const char      *index; /* output index file path relative from current directory. */
            unsigned         timeshift; /* timeshift file duration in seconds; if 0, output ts file is not timeshift. */
            NEXUS_CallbackDesc dataCallback; /* Callback when output data becomes available. Used for stream output */

            /* MP4 file output type specific: App must provide output file path and temproary dir path */
            const char      *tmpDir; /* temporary storage directory path to store intermediate data */
            bool             progressiveDownload; /* default OFF to finish mp4 mux quickly; if ON, moov box is before mdat box. */
        } transport;
    } output;

} BXCode_StartSettings;

/**
Summary:
**/
void BXCode_GetDefaultStartSettings(
    BXCode_StartSettings *pSettings   /* [out] */
    );

/**
Summary:
Start xcoder
**/
NEXUS_Error BXCode_Start(
    BXCode_Handle               handle,
    const BXCode_StartSettings *pSettings
    );

/**
Summary:
**/
void BXCode_Stop(
    BXCode_Handle handle
    );

/**
Summary:
Dynamic settings
**/
typedef struct BXCode_Settings
{
    struct {
        bool                         enabled; /* to dynamically enable/disable video output */
        NEXUS_VideoEncoderSettings   encoder; /* bitrate, framerate, gop settings */
        unsigned                     width, height; /* resolution */
        unsigned                     refreshRate; /* units of 1/1000 Hz. 59940 = 59.94Hz, 60000 = 60Hz; 0 uses default 59.94 or 50hz based on encoder framerate settings */
        NEXUS_VideoOrientation       orientation;  /* support 2D or half-resol 3D encode */
        NEXUS_DisplayAspectRatio     aspectRatio; /* Aspect ratio of display when windowSettings.contentMode!=eFull or gfxSettings.enabled */
        struct {
            unsigned                 x, y; /* if aspectRatio is eSar, here are the SAR x:y components */
        } sampleAspectRatio;
        NEXUS_VideoWindowSettings    windowSettings;/* video window settings including contentMode for aspect ratio correction */
        struct {
            unsigned                 maxWidth, maxHeight; /* max format supported by video decoder */
        } decoder;
        NEXUS_GraphicsSettings       gfxSettings;   /* gfx window settings */
        NEXUS_SurfaceHandle          frameBuffer;   /* GUI surface */
    } video;
    struct {
        bool                         enabled; /* dynamically enable/disable audio output */
        NEXUS_AudioEncoderCodecSettings codec; /* ignored if passthrough = true */
    } audio[BXCODE_MAX_AUDIO_PIDS];
} BXCode_Settings;

void BXCode_GetSettings(
    BXCode_Handle             handle,
    BXCode_Settings *pSettings /* [out] */
    );

NEXUS_Error BXCode_SetSettings(
    BXCode_Handle             handle,
    const BXCode_Settings *pSettings
    );

/**
Summary:
Input Status
**/
typedef struct BXCode_InputStatus
{
    NEXUS_PlaybackStatus     playback;
    NEXUS_VideoDecoderStatus videoDecoder;
    unsigned                 numAudios; /* number of audio programs */
    NEXUS_AudioDecoderStatus audioDecoder[BXCODE_MAX_AUDIO_PIDS];

    NEXUS_PlaypumpStatus     playpump;
    NEXUS_ParserBandStatus   parserBand;
#if NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInputStatus    hdmiInput;
#endif
    NEXUS_VideoImageInputHandle imageInput;/* app must use it to feed input stream */
    NEXUS_VideoImageInputStatus imageInputStatus;
} BXCode_InputStatus;

/**
Summary:
**/
NEXUS_Error BXCode_GetInputStatus(
    BXCode_Handle       handle,
    BXCode_InputStatus *pStatus /* [out] */
    );

/**
* API for input stream data feeding
*/

/**
Summary:
Stream input scatter gather Descriptor
 */
typedef NEXUS_PlaypumpScatterGatherDescriptor BXCode_InputDescriptor;

/**
Summary:
Submit scatter gather descriptor to BXCode_Input stream interface with app allocated user buffers
App calls NEXUS_Memory_Allocate to allocate N data buffers, fill it then submit via this API to the stream input.
Note, app needs to check input status.stream.playpump.descFifoDepth < N to feed input; else (=N), wait for dataready event from input stream dataCallback.
**/
NEXUS_Error BXCode_Input_SubmitScatterGatherDescriptor(
    BXCode_Handle handle,
    const BXCode_InputDescriptor *pDesc,
    size_t        numDescriptors,
    size_t       *pNumConsumed /* [out] */
    );

/*
Summary:
For internally allocated stream feed buffer, app uses GetBuffer to get the pointer for memcpy.
*/
NEXUS_Error BXCode_Input_GetBuffer(
    BXCode_Handle handle,
    void        **pBuffer, /* [out] pointer to memory mapped region that is ready for playback data */
    size_t       *pSize    /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    );

/**
Summary:
For internally allocated stream feed buffer, app uses WrieComplete to notify BXCode_Input to consume the data.
**/
NEXUS_Error BXCode_Input_WriteComplete(
    BXCode_Handle handle,
    size_t        skip,      /* skip is the number of bytes at the beginning of the current buffer pointer
                                          which BXCode_Input should skip over. */
    size_t        amountUsed /* amountUsed is the number of bytes, following any skip bytes,
                                          which BXCode_Input should feed into transport. */
    );

/**
* API for output status
**/

/**
Summary:
Video Encoder Status
**/
typedef struct BXCode_Output_VideoStatus
{
    bool     enabled;

    void    *pBufferBase;           /* video output buffer base address */
    void    *pMetadataBufferBase;   /* video output metadata buffer base address */

    uint32_t errorFlags;
    uint32_t eventFlags;
    unsigned errorCount; /* Total number of errors that has occurred */
    unsigned picturesReceived; /* Number of pictures received at the input to the encoder */
    unsigned picturesDroppedFRC;  /* Number of pictures that the encoder has configured to drop in order to follow the requested frame rate (Frame Rate Conversion) */
    unsigned picturesDroppedHRD; /* Number of pictures that the encoder has dropped because of a drop request from the Rate Control. The Rate Control may decide to drop picture in order to maintain the HRD buffer model. */
    unsigned picturesDroppedErrors; /* Number of pictures that the encoder has configured to drop because encoder did not finish the processing of the previous pictures on time and buffers are full. */
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
} BXCode_Output_VideoStatus;

/**
Summary:
Audio Encoder Status
**/
typedef struct BXCode_Output_AudioStatus
{
    bool         enabled;
    void        *pBufferBase;           /* audio output buffer base address */
    void        *pMetadataBufferBase;   /* audio output metadata buffer base address */

    unsigned     numFrames;              /* Number of successfully encoded frames */
    unsigned     numErrorFrames;         /* Number of error frames */
    struct {
        unsigned fifoDepth;             /* depth in bytes of buffer */
        unsigned fifoSize;              /* size in bytes of buffer */
    } data;
} BXCode_Output_AudioStatus;

/**
Summary:
Mux Status
**/
typedef struct BXCode_MuxStatus
{
    unsigned duration;                 /* current completed stream duration in miliseconds (only for TS/Mp4 output) */
    NEXUS_RecpumpStatus recpumpStatus; /* only for TS output */
} BXCode_MuxStatus;

/**
Summary:
**/
typedef struct BXCode_OutputStatus
{
    BXCode_Output_VideoStatus video;
    BXCode_Output_AudioStatus audio[BXCODE_MAX_AUDIO_PIDS];
    unsigned                  numAudios; /* number of audio output programs */
    double                    avSyncErr[BXCODE_MAX_AUDIO_PIDS]; /* current AV sync error in miliseconds (positive means video leads) */
    BXCode_MuxStatus          mux;
} BXCode_OutputStatus;

/**
Summary:
**/
NEXUS_Error BXCode_GetOutputStatus(
    BXCode_Handle        handle,
    BXCode_OutputStatus *pStatus /* [out] */
    );

/*
*  API for ES/TS stream output.
*/

/* output stream type */
typedef enum BXCode_OutputStreamType
{
    BXCode_OutputStreamType_eTs,  /* transport stream */
    BXCode_OutputStreamType_eVes, /* video elementary stream */
    BXCode_OutputStreamType_eAes, /* audio elementary stream */
    BXCode_OutputStreamType_eMax
} BXCode_OutputStreamType;

/**
Summary:
Select which output stream to BXCode_Output_GetBuffer

Description:
**/
typedef struct BXCode_OutputStream
{
    BXCode_OutputStreamType type;
    unsigned                id; /* which stream of the type? applicable to audio ES output selection for multi-audio program */
} BXCode_OutputStream;

/**
Summary:
TS Data descriptor returned by BXCode_Output_GetDescriptors

Description:
A wrapper for TS output stream buffer descriptor.
**/
typedef struct BXCode_OutputTsDescriptor
{
    uint32_t    flags; /* ts data descriptor */
    const void *pData; /* address of ts data */
    size_t      size;/* 0 if fragment is empty, size in bytes */

} BXCode_OutputTsDescriptor;

/**
Summary:
Flags for the BXCode_OutputTsDescriptor.flags field (for HLS)
**/
#define BXCODE_OUTPUTDESCRIPTOR_TSFLAG_SEGMENT_START 0x00000001 /* the ts buffer pointed by pData starts with a new TS segment with PAT/PMT in front */
#define BXCODE_OUTPUTDESCRIPTOR_TSFLAG_SEGMENT_END   0x00000002 /* the ts buffer pointed by pData ends a TS segment */

/**
Summary:
Get stream output data descriptors if outputting to memory. the descriptors include address pointers to the output data buffer.
The output buffer will be allocated internally from nexus user heap. The ts output buffer might not be contiguous to support
internally manual insertion of PAT/PMT prior to each video RAI packets for each start of HLS segments.

Note:
    1) For TS output stream type, the output data descriptors follow the BXCode_OutputTsDescriptor format;
    2) For VES output stream type, the output data descriptors follow the NEXUS_VideoEncoderDescriptor format;
    3) For AES output stream type, the output data descriptors follow the NEXUS_AudioMuxOutputFrame format;
**/
NEXUS_Error BXCode_Output_GetDescriptors(
    BXCode_Handle                   handle,
    BXCode_OutputStream             stream,   /* [in] */
    const void                    **pBuffer,  /* [out] pointer to output data descriptors structs array */
    size_t                         *pSize,    /* [out] number of output data descriptors in pBuffer */
    const void                    **pBuffer2, /* [out] pointer to output data descriptors structs array */
    size_t                         *pSize2    /* [out] number of output data descriptors in pBuffer2 */
    );

/**
Summary:
Return output stream data descriptors
**/
NEXUS_Error BXCode_Output_ReadComplete(
    BXCode_Handle           handle,
    BXCode_OutputStream     stream,   /* [in] */
    unsigned                descriptorsCompleted /* must be <= *pSize + *pSize2 returned by last BXCode_Output_GetDescriptors call. */
    );


#ifdef __cplusplus
}
#endif
#endif /* NEXUS_HAS_VIDEO_ENCODER */

#endif /* !defined BXCODE_H__ */
