/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/
/* xcode lib example app */
#include <stdio.h>
#ifdef NEXUS_HAS_VIDEO_ENCODER
#include "namevalue.h"
#include "transcode_test.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_timebase.h"
#include "nexus_video_decoder_userdata.h"
#include "nexus_video_adj.h"
#include "nexus_core_utils.h"
#include "budp_dccparse.h"

BDBG_MODULE(loopback_player);
BDBG_FILE_MODULE(userdataCb);

#define BTST_MAX_CC_DATA_COUNT    32

NEXUS_PlatformConfiguration g_platformConfig;

enum BTST_P_DecoderId {
    BTST_P_DecoderId_eLoopback,
    BTST_P_DecoderId_eSource0,
    BTST_P_DecoderId_eSource1,
    BTST_P_DecoderId_eSource2,
    BTST_P_DecoderId_eSource3,
    BTST_P_DecoderId_eMax
} BTST_P_DecoderId;

static const char *g_userDataDecoderName[] = {
    "loopback",
    "source0",
    "source1",
    "source2",
    "source3"
};
typedef struct BTST_P_DecoderContext {
    struct {
        FILE *fLog;
        bool  bFilterNull;
        bool  bInit;
    } output608, output708;
    NEXUS_VideoDecoderHandle   videoDecoder;
    NEXUS_VideoCodec           codec;
    BUDP_DCCparse_ccdata       ccData[BTST_MAX_CC_DATA_COUNT];
    BAVC_USERDATA_info         userData;
} BTST_P_DecoderContext;
static BTST_P_DecoderContext g_decoderContext[BTST_P_DecoderId_eMax];

static void userdataCallback(void *context, int param)
{
    NEXUS_Error rc;
    BTST_P_DecoderContext *pContext = (BTST_P_DecoderContext*)context;
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)pContext->videoDecoder;

    BDBG_ASSERT(param < BTST_P_DecoderId_eMax);
    BDBG_MODULE_MSG(userdataCb, ("decoder %d user data callback!",param));

    /* It's crucial that the callback process all userdata packets in the buffer.
    The app cannot assume that it can process one packet per callback. */
    while (1) {
        unsigned size;
        uint8_t *buffer;
        uint32_t offset = 0;
        NEXUS_UserDataHeader *pHeader;
        BAVC_USERDATA_info   *toBeParsed = &pContext->userData;

        /* get the header */
        rc = NEXUS_VideoDecoder_GetUserDataBuffer(videoDecoder, (void**)&buffer, &size);
        BDBG_ASSERT(!rc);

        /* The app can assume that userdata will be placed into the buffer in whole packets.
        The termination condition for the loop is that there are no more bytes in the buffer. */
        if (!size) break;

        /* guaranteed whole header + payload */
        pHeader = (NEXUS_UserDataHeader *)buffer;
        BDBG_ASSERT(size >= pHeader->blockSize);
        BDBG_ASSERT(pHeader->blockSize == sizeof(*pHeader) + pHeader->payloadSize);
        BDBG_ASSERT(pHeader->blockSize % 4 == 0); /* 32 bit aligned */

        /* 1) resume the to-be-parsed user data structure as input of UDP; */
        toBeParsed->pUserDataBuffer     = (void*)((uint8_t*)buffer + sizeof(NEXUS_UserDataHeader));
        toBeParsed->ui32UserDataBufSize = pHeader->payloadSize;
        toBeParsed->eUserDataType = pHeader->type;
        toBeParsed->eSourcePolarity = pHeader->polarity;
        toBeParsed->bTopFieldFirst = pHeader->topFieldFirst;
        toBeParsed->bRepeatFirstField = pHeader->repeatFirstField;
        toBeParsed->bPTSValid = pHeader->ptsValid;
        toBeParsed->ui32PTS = pHeader->pts;
        toBeParsed->ePicCodingType = pHeader->pictureCoding;
        BKNI_Memcpy(toBeParsed->ui32PicCodExt, pHeader->pictureCodingExtension, sizeof(toBeParsed->ui32PicCodExt));

        /* 2) call UDP to parse cc data; */
        while (offset < toBeParsed->ui32UserDataBufSize) {
            BERR_Code rc;
            unsigned i;
            size_t bytesParsed = 0;
            uint8_t ccCount = 0;
            BUDP_DCCparse_ccdata *ccData = pContext->ccData;

            if (pContext->codec == NEXUS_VideoCodec_eH264) {
                rc = BUDP_DCCparse_SEI_isr(toBeParsed, offset, &bytesParsed, &ccCount, ccData);
            }
            else {
                rc = BUDP_DCCparse_isr(toBeParsed, offset, &bytesParsed, &ccCount, ccData);
            }

            for( i = 0; i < ccCount; i++ )
            {
                FILE *fLog = NULL;
                bool bHeader = false;

                if ( true == ccData[i].bIsAnalog )
                {
                   if ( ( ( true == pContext->output608.bFilterNull ) && ( 0 != (ccData[i].cc_data_1 & 0x7F) ) )
                        || ( ( false == pContext->output608.bFilterNull ) ) )
                   {
                      fLog = pContext->output608.fLog;
                      if ( pContext->output608.bInit )
                      {
                         bHeader = true;
                         pContext->output608.bInit = false;
                      }
                   }
                }
                else
                {
                   if ( ( ( true == pContext->output708.bFilterNull ) && ( 0 != (ccData[i].cc_data_1) ) )
                        || ( ( false == pContext->output708.bFilterNull ) ) )
                   {
                      fLog = pContext->output708.fLog;
                      if ( pContext->output708.bInit )
                      {
                         bHeader = true;
                         pContext->output708.bInit = false;
                      }
                   }
                }

                if ( NULL != fLog)
                {
                   if ( true == bHeader )
                   {
                      fprintf( fLog, "size,ccCnt,PTS,Polarity,Format,CC Valid,CC Priority,CC Line Offset,CC Type,CC Data[0],CC Data[1]\n");
                   }

                   fprintf(fLog, "%u,%u,%u,%d,%d,%d,%d,%d,%d,%d,%d\n",
                      pHeader->payloadSize, ccCount,
                      pHeader->pts,
                      ccData[i].polarity,
                      ccData[i].format,
                      ccData[i].cc_valid,
                      ccData[i].cc_priority,
                      ccData[i].line_offset,
                      ccData[i].seq.cc_type,
                      ccData[i].cc_data_1,
                      ccData[i].cc_data_2
                      );
                }
            }
            offset += bytesParsed;
        }

        NEXUS_VideoDecoder_UserDataReadComplete(videoDecoder, pHeader->blockSize);
    }
}

static void
vidSrcStreamChangedCallback(void *context, int param)
{
    NEXUS_VideoDecoderStreamInformation streamInfo;
    NEXUS_VideoDecoderHandle decoder = (NEXUS_VideoDecoderHandle)context;

    NEXUS_VideoDecoder_GetStreamInformation(decoder, &streamInfo);
    if(param == -1) {
        BDBG_WRN(("Loopback Video Source Stream Change callback:"));
    } else {
        BDBG_WRN(("Xcoder%d Video Source Stream Change callback:", param));
    }
    BDBG_WRN((" %ux%u@%s%c",
        streamInfo.sourceHorizontalSize,
        streamInfo.sourceVerticalSize,
        lookup_name(g_videoFrameRateStrs, streamInfo.frameRate),
        streamInfo.streamProgressive? 'p' : 'i'));
}

static const char *s3DTVFormat[NEXUS_VideoDecoder3DTVFormat_eMax] = {
    "2D",
    "3D_LeftRightHalf",
    "3D_TopBottomHalf",
    "3D_FramePackedOverUnder",
};

static void decode3dCallback(void *pContext, int param)
{
    NEXUS_VideoDecoderHandle videoDecoder = (NEXUS_VideoDecoderHandle)pContext;
    NEXUS_VideoDecoder3DTVStatus status;
    bool valid;

    NEXUS_VideoDecoder_Get3DTVStatus(videoDecoder, &status);
    if(param==-1) {
        printf("Loopback decoder");
    } else {
        printf("Xcoder[%d]", param);
    }
    printf(" 3DTV status: %s\n", (status.codec==NEXUS_VideoCodec_eH264)?"AVC":"MPEG2");
    if (status.codec==NEXUS_VideoCodec_eH264) {
        valid = status.codecData.avc.valid;
    }
    else {
        valid = status.codecData.mpeg2.valid;
    }
    printf("   valid %d, format %s\n", valid, s3DTVFormat[status.format]);

    /* At this point, the application should set the necessary 3D configuration for its platform,
       based on the reported 3DTV format. For example, DTVs should configure the video windows;
       Settops should configure the HDMI VSI. */
}

static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    fprintf(stderr, "HDMI hotplug event: %s\n", status.connected?"connected":"not connected");

    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( status.connected )
    {
        NEXUS_DisplaySettings displaySettings;
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !status.videoFormatSupported[displaySettings.format] )
        {
            fprintf(stderr, "\nCurrent format not supported by attached monitor. Switching to preferred format %d\n", status.preferredVideoFormat);
            displaySettings.format = status.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
        }
    }
}

void xcode_loopback_setup( BTST_Context_t  *pContext )
{
    NEXUS_HdmiOutputSettings hdmiOutputSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_VideoDecoderSettings vidDecodeSettings;
    NEXUS_VideoDecoderExtendedSettings extSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_StcChannelSettings stcSettings;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowScalerSettings scalerSettings;
    BTST_Transcoder_t *pTranscoder = &pContext->xcodeContext[pContext->selectedXcodeContextId];
    int cnt = 0;

    if(pContext->loopbackStarted) {
        BDBG_WRN(("Transcoder[%u] loopback player already started!", pTranscoder->id));
        return;
    }
    if(pTranscoder->output.type == BXCode_OutputType_eMp4File) {
        BDBG_WRN(("Transcoder[%u] output MP4 file cannot be loopback played while transcoding!", pTranscoder->id));
        return;
    }

    BDBG_MSG(("To start the loopback player for transcoder%d...", pTranscoder->id));
    NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
    if((stcSettings.timebase = NEXUS_Timebase_Open(NEXUS_ANY_ID)) == (NEXUS_Timebase)NULL) {
        BDBG_ERR(("Loopback player failed to open timebase!"));
        return;
    }
    pContext->timebaseLoopback = stcSettings.timebase;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    pContext->stcChannelLoopback = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
    BDBG_MSG(("Transcoder%d opened Loopback STC [%p].", pTranscoder->id, pContext->stcChannelLoopback));
#if NEXUS_HAS_SYNC_CHANNEL
    /* create a sync channel for xcoder loopback decode */
    NEXUS_SyncChannel_GetDefaultSettings(&syncChannelSettings);
    pContext->syncChannelLoopback = NEXUS_SyncChannel_Create(&syncChannelSettings);
#endif

    if(pTranscoder->startSettings.output.video.pid) {
        unsigned decoderId = 0;
        NEXUS_VideoDecoderCapabilities cap;

        NEXUS_GetVideoDecoderCapabilities(&cap);
        BDBG_MSG(("start xcode loopback display on HDMI and Component outputs..."));
        do {
            pContext->videoDecoderLoopback = NEXUS_VideoDecoder_Open(decoderId, NULL); /* take default capabilities */
            decoderId++;
        } while(!pContext->videoDecoderLoopback && decoderId < cap.numVideoDecoders);
        BDBG_MSG(("Transcoder%d opened Loopback video decoder%d [%p].", pTranscoder->id, decoderId-1, pContext->videoDecoderLoopback));
        NEXUS_VideoDecoder_GetSettings(pContext->videoDecoderLoopback, &vidDecodeSettings);
        vidDecodeSettings.supportedCodecs[pTranscoder->output.vCodec] = true;
        if(pContext->logClosedCaption) {/* to log loopback xcoded user data */
            char fname[256];
            vidDecodeSettings.userDataEnabled = true;
            vidDecodeSettings.appUserDataReady.callback = userdataCallback;
            g_decoderContext[BTST_P_DecoderId_eLoopback].videoDecoder = pContext->videoDecoderLoopback;
            g_decoderContext[BTST_P_DecoderId_eLoopback].codec = pTranscoder->output.vCodec;
            sprintf(fname, "userdata_%s_608.csv", g_userDataDecoderName[BTST_P_DecoderId_eLoopback]);
            g_decoderContext[BTST_P_DecoderId_eLoopback].output608.fLog        = fopen(fname, "wb");
            g_decoderContext[BTST_P_DecoderId_eLoopback].output608.bInit       = true;
            g_decoderContext[BTST_P_DecoderId_eLoopback].output608.bFilterNull = false;
            sprintf(fname, "userdata_%s_708.csv", g_userDataDecoderName[BTST_P_DecoderId_eLoopback]);
            g_decoderContext[BTST_P_DecoderId_eLoopback].output708.fLog = fopen(fname, "wb");
            g_decoderContext[BTST_P_DecoderId_eLoopback].output708.bInit       = true;
            g_decoderContext[BTST_P_DecoderId_eLoopback].output708.bFilterNull = false;
            vidDecodeSettings.appUserDataReady.context  = &g_decoderContext[BTST_P_DecoderId_eLoopback];
            vidDecodeSettings.appUserDataReady.param    = BTST_P_DecoderId_eLoopback;/* loopback */
        }
        /* channel change mode: hold until first new picture */
        vidDecodeSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilFirstPicture;
        vidDecodeSettings.streamChanged.callback = vidSrcStreamChangedCallback;
        vidDecodeSettings.streamChanged.context  = pContext->videoDecoderLoopback;
        vidDecodeSettings.streamChanged.param  = -1;
        NEXUS_VideoDecoder_SetSettings(pContext->videoDecoderLoopback, &vidDecodeSettings);

        /* register the 3DTV status change callback */
        if(pTranscoder->output.orientation) {
            NEXUS_VideoDecoder_GetExtendedSettings(pContext->videoDecoderLoopback, &extSettings);
            extSettings.s3DTVStatusEnabled = true;
            extSettings.s3DTVStatusChanged.callback = decode3dCallback;
            extSettings.s3DTVStatusChanged.context = pContext->videoDecoderLoopback;
            extSettings.s3DTVStatusChanged.param = -1;/* for loopback decoder */
            NEXUS_VideoDecoder_SetExtendedSettings(pContext->videoDecoderLoopback, &extSettings);
        }
    }

    pContext->playpumpLoopback = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT(pContext->playpumpLoopback);
    BDBG_MSG(("Transcoder%d opened Loopback playpump [%p].", pTranscoder->id, pContext->playpumpLoopback));
    pContext->playbackLoopback = NEXUS_Playback_Create();
    BDBG_ASSERT(pContext->playbackLoopback);

    pContext->filePlayLoopback = NEXUS_FilePlay_OpenPosix(pTranscoder->output.data,
        ((pTranscoder->output.type != BXCode_OutputType_eTs) || pTranscoder->output.segmented || !pTranscoder->output.file)?
            pTranscoder->output.data : pTranscoder->output.index);
    if (!pContext->filePlayLoopback) {
        BDBG_ERR(("can't open file: %s, index %s\n", pTranscoder->output.data, pTranscoder->startSettings.output.transport.index));
        BDBG_ASSERT(0);
    }

    NEXUS_Playback_GetSettings(pContext->playbackLoopback, &playbackSettings);
    playbackSettings.playpump = pContext->playpumpLoopback;
    /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
    switch(pTranscoder->output.type) {
    case BXCode_OutputType_eEs:
        playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eEs;
        break;
    case BXCode_OutputType_eTs:
        playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
        playbackSettings.playpumpSettings.timestamp.type = pTranscoder->output.transportTimestamp;
        break;
    case BXCode_OutputType_eMp4File:
        playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eMp4;
        break;
    default:
        BDBG_ERR(("Transcoder[%u]: invalid output type %d!", pTranscoder->id, pTranscoder->output.type));
        BDBG_ASSERT(0);
    }
    playbackSettings.stcChannel = pContext->stcChannelLoopback; /* loopback channel  */
    playbackSettings.timeshifting = true; /* allow for timeshift the growing file  */
   if ( pContext->logClosedCaption )
   {
      playbackSettings.timeshiftingSettings.endOfStreamGap = 5000; /* Add a delay to prevent repeated CC data being dumped */
   }
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay; /* when play hits the end, wait for record */
    NEXUS_Playback_SetSettings(pContext->playbackLoopback, &playbackSettings);

    /* Open the video pid channel */
    if(pTranscoder->startSettings.output.video.pid) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = pTranscoder->output.vCodec; /* must be told codec for correct handling */
        playbackPidSettings.pidTypeSettings.video.index = pTranscoder->startSettings.output.transport.index;
        playbackPidSettings.pidTypeSettings.video.decoder = pContext->videoDecoderLoopback;
        pContext->videoPidChannelLoopback = NEXUS_Playback_OpenPidChannel(pContext->playbackLoopback, pTranscoder->startSettings.output.video.pid, &playbackPidSettings);

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up
        the audio outputs. */
        NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
        videoProgram.codec = pTranscoder->output.vCodec;
        videoProgram.pidChannel = pContext->videoPidChannelLoopback;
        videoProgram.stcChannel = pContext->stcChannelLoopback;

        /* Bring up loopback video display and outputs */
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        displaySettings.timebase = NEXUS_Timebase_e0;
        displaySettings.displayType = NEXUS_DisplayType_eAuto;
        displaySettings.format =
            (pTranscoder->output.framerate == NEXUS_VideoFrameRate_e25 ||
             pTranscoder->output.framerate == NEXUS_VideoFrameRate_e50) ?
            NEXUS_VideoFormat_e720p50hz : NEXUS_VideoFormat_e720p;
        pContext->displayLoopback = NEXUS_Display_Open(0, &displaySettings);
        BDBG_MSG(("Transcoder[%u] opened Loopback display%d [%p].", pTranscoder->id, 0, pContext->displayLoopback));
#if NEXUS_NUM_COMPONENT_OUTPUTS
        if(g_platformConfig.outputs.component[0]){
            NEXUS_Display_AddOutput(pContext->displayLoopback, NEXUS_ComponentOutput_GetConnector(g_platformConfig.outputs.component[0]));
        }
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
            if(g_platformConfig.outputs.hdmi[0]){
                NEXUS_Display_AddOutput(pContext->displayLoopback, NEXUS_HdmiOutput_GetVideoConnector(g_platformConfig.outputs.hdmi[0]));
            }
#endif
        /* Install hotplug callback -- video only for now */
        NEXUS_HdmiOutput_GetSettings(g_platformConfig.outputs.hdmi[0], &hdmiOutputSettings);
        hdmiOutputSettings.hotplugCallback.callback = hotplug_callback;
        hdmiOutputSettings.hotplugCallback.context = g_platformConfig.outputs.hdmi[0];
        hdmiOutputSettings.hotplugCallback.param = (int)pContext->displayLoopback;
        NEXUS_HdmiOutput_SetSettings(g_platformConfig.outputs.hdmi[0], &hdmiOutputSettings);

        pContext->windowLoopback = NEXUS_VideoWindow_Open(pContext->displayLoopback, 0);

        NEXUS_VideoWindow_AddInput(pContext->windowLoopback, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback));

        /* set loopback vnet mode bias to avoid black frame transition during dynamic resolution change */
        NEXUS_VideoWindow_GetScalerSettings(pContext->windowLoopback, &scalerSettings);
        scalerSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
        scalerSettings.bandwidthEquationParams.delta = 1*1000*1000;
        NEXUS_VideoWindow_SetScalerSettings(pContext->windowLoopback, &scalerSettings);
    }

    /* if audio DAC0 for loopback player */
    if(pTranscoder->input.enableAudio)
    {
        /* Open the audio loopback decoder */
        pContext->audioDecoderLoopback = NEXUS_AudioDecoder_Open(NEXUS_ANY_ID, NULL);
        BDBG_MSG(("Transcoder%d opened Loopback audio decoder [%p].", pTranscoder->id, pContext->audioDecoderLoopback));

        /* Open the audio pid channel */
        {
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.primary = pContext->audioDecoderLoopback;
            pContext->audioPidChannelLoopback = NEXUS_Playback_OpenPidChannel(pContext->playbackLoopback, pTranscoder->startSettings.output.audio[0].pid, &playbackPidSettings);
        }

        /* Set up decoder Start structures now. We need to know the audio codec to properly set up
        the audio outputs. */
        NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);

        audioProgram.codec = pTranscoder->output.audioCodec[0];
        audioProgram.pidChannel = pContext->audioPidChannelLoopback;
        audioProgram.stcChannel = pContext->stcChannelLoopback;

        /* Connect audio decoders to outputs */
#if NEXUS_NUM_AUDIO_DACS
        NEXUS_AudioOutput_AddInput(
            NEXUS_AudioDac_GetConnector(g_platformConfig.outputs.audioDacs[0]),
            NEXUS_AudioDecoder_GetConnector(pContext->audioDecoderLoopback, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
        /* add hdmi pcm audio output to loopback display */
        NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(g_platformConfig.outputs.hdmi[0]),
                                   NEXUS_AudioDecoder_GetConnector(pContext->audioDecoderLoopback, NEXUS_AudioDecoderConnectorType_eStereo));

#if NEXUS_HAS_SYNC_CHANNEL
        /* connect sync channel before start decode */
        NEXUS_SyncChannel_GetSettings(pContext->syncChannelLoopback, &syncChannelSettings);
        if(pTranscoder->startSettings.output.video.pid)
            syncChannelSettings.videoInput = NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback);
        if(pTranscoder->input.enableAudio)
        {
            syncChannelSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(pContext->audioDecoderLoopback, NEXUS_AudioDecoderConnectorType_eStereo);
        }
        NEXUS_SyncChannel_SetSettings(pContext->syncChannelLoopback, &syncChannelSettings);
#endif

        /* start audio loopback decoder */
        NEXUS_AudioDecoder_Start(pContext->audioDecoderLoopback, &audioProgram);
    }

    /* Start video loopback decoder */
    if(pTranscoder->startSettings.output.video.pid)
        NEXUS_VideoDecoder_Start(pContext->videoDecoderLoopback, &videoProgram);

    /* delay the loopback until A2P passed to avoid stutter since playback jams PTS to STC instead of locking PCR so the encode buffer
       model is not enforced at loopback decoder plus the O_DIRECT timeshift buffer seems to have some latency. */
    BKNI_Sleep(4000);
    /* wait for timeshift file index to be available */
    while (cnt++ < 100) {
        BXCode_OutputStatus status;
        BXCode_GetOutputStatus(pTranscoder->hBxcode, &status);
        if(pTranscoder->output.type == BXCode_OutputType_eEs) {
            if(pTranscoder->startSettings.output.video.pid && status.video.picturesEncoded>30) {
                BDBG_WRN(("%u video pictures encoded\n", status.video.picturesEncoded));
                break;
            }
            if(pTranscoder->startSettings.output.audio[0].pid && status.audio[0].numFrames>30) {
                BDBG_WRN(("%u audio frames encoded\n", status.audio[0].numFrames));
                break;
            }
        } else if(pTranscoder->output.type == BXCode_OutputType_eTs) {
            if(status.mux.duration > 3000) {
                BDBG_WRN(("%u ms TS mux'd\n", status.mux.duration));
                if(status.mux.recpumpStatus.data.bytesRecorded > 4096*188) {
                    BDBG_WRN(("%llu bytes TS mux'd\n", status.mux.recpumpStatus.data.bytesRecorded));
                    break;
                }
            }
        }
        BKNI_Sleep(100);
    }
    if(cnt >= 100) /* try 10 seconds */
    {
        fprintf(stderr, "**** Encoder stalls and cannot timeshift xcoded stream! Please debug! ****\n");
    }

    /* Start playback */
    NEXUS_Playback_Start(pContext->playbackLoopback, pContext->filePlayLoopback, NULL);

    pContext->loopbackStarted = true;
}

void xcode_loopback_shutdown( BTST_Context_t *pContext )
{
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelSettings syncChannelSettings;
#endif
    BTST_Transcoder_t *pTranscoder = &pContext->xcodeContext[pContext->selectedXcodeContextId];

    if(!pContext->loopbackStarted) {
        BDBG_WRN(("Transcoder[%u] loopback player didn't start!", pTranscoder->id));
        return;
    }
    pContext->loopbackStarted = false;

    BDBG_MSG(("To shutdown the loopback player for transcoder%d...", pTranscoder->id));
    NEXUS_Playback_Stop(pContext->playbackLoopback);

    if(pTranscoder->startSettings.output.video.pid)
        NEXUS_VideoDecoder_Stop(pContext->videoDecoderLoopback);
    /* disconnect sync channel after stop decoder */
    if(pTranscoder->input.enableAudio)
    {
        NEXUS_AudioDecoder_Stop(pContext->audioDecoderLoopback);
#if NEXUS_HAS_SYNC_CHANNEL
        NEXUS_SyncChannel_GetSettings(pContext->syncChannelLoopback, &syncChannelSettings);
        syncChannelSettings.videoInput = NULL;
        syncChannelSettings.audioInput[0] = NULL;
        syncChannelSettings.audioInput[1] = NULL;
        NEXUS_SyncChannel_SetSettings(pContext->syncChannelLoopback, &syncChannelSettings);
#endif
        NEXUS_AudioDecoder_Close(pContext->audioDecoderLoopback);
    }
    NEXUS_Playback_CloseAllPidChannels(pContext->playbackLoopback);
    NEXUS_FilePlay_Close(pContext->filePlayLoopback);
    NEXUS_Playback_Destroy(pContext->playbackLoopback);
    pContext->playbackLoopback = NULL;
    NEXUS_Playpump_Close(pContext->playpumpLoopback);

    if(pTranscoder->startSettings.output.video.pid) {
        NEXUS_VideoWindow_RemoveInput(pContext->windowLoopback, NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback));
        NEXUS_VideoInput_Shutdown(NEXUS_VideoDecoder_GetConnector(pContext->videoDecoderLoopback));
        NEXUS_VideoDecoder_Close(pContext->videoDecoderLoopback);
        NEXUS_VideoWindow_Close(pContext->windowLoopback);
#if NEXUS_NUM_HDMI_OUTPUTS
        NEXUS_Display_RemoveOutput(pContext->displayLoopback, NEXUS_HdmiOutput_GetVideoConnector(g_platformConfig.outputs.hdmi[0]));
        NEXUS_StopCallbacks(g_platformConfig.outputs.hdmi[0]);
#endif
        NEXUS_Display_Close(pContext->displayLoopback);
    }
#if NEXUS_HAS_SYNC_CHANNEL
    if(pContext->syncChannelLoopback) {
        NEXUS_SyncChannel_Destroy(pContext->syncChannelLoopback);
        pContext->syncChannelLoopback = NULL;
    }
#endif

    NEXUS_StcChannel_Close(pContext->stcChannelLoopback);
    NEXUS_Timebase_Close(pContext->timebaseLoopback);

    if(g_decoderContext[BTST_P_DecoderId_eLoopback].output608.fLog) {
        fclose(g_decoderContext[BTST_P_DecoderId_eLoopback].output608.fLog);
        g_decoderContext[BTST_P_DecoderId_eLoopback].output608.fLog = NULL;
    }
    if(g_decoderContext[BTST_P_DecoderId_eLoopback].output708.fLog) {
        fclose(g_decoderContext[BTST_P_DecoderId_eLoopback].output708.fLog);
        g_decoderContext[BTST_P_DecoderId_eLoopback].output708.fLog = NULL;
    }
}

static void loopbackUsage(void)
{
    printf("? - this help\n"
           "a - loopback window Aspect ratio mode\n"
           "c - Close selected transcoder loopback player\n"
           "d - loopback Display format switch\n"
           "f - Fast Forward\n"
           "i - Select xcoder Id for loopback\n"
           "l - Slow Motion\n"
           "o - Open selected transcoder loopback player\n"
           "p - Play\n"
           "r - Fast Rewind\n"
           "s - pauSe\n"
           "w - Wait 30msec\n"
           "z - resiZe loopback video window\n"
           "+ - Jump forward 5 seconds\n"
           "- - Jump backward 5 seconds\n"
           "0 - Jump to the beginning\n"
           "q - Go back to upper level menu\n"
           );
}

static void jump_to_beginning(NEXUS_PlaybackHandle playback)
{
    unsigned pos;
    NEXUS_Error rc;
    NEXUS_PlaybackStatus playbackStatus;

    rc = NEXUS_Playback_GetStatus(playback, &playbackStatus);
    BDBG_ASSERT(!rc);
    pos = playbackStatus.first;
    printf("Jump to beginning %u, %u...%u\n", pos, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
    (void)NEXUS_Playback_Seek(playback, pos);
}

void loopbackPlayer( BTST_Context_t *pContext )
{
    int rate = NEXUS_NORMAL_PLAY_SPEED;
    BTST_Transcoder_t *pTranscoder = &pContext->xcodeContext[pContext->selectedXcodeContextId];
    for(;;) {
        NEXUS_PlaybackStatus playbackStatus;
        char cmd[16], input[80];
        unsigned i;
        bool quit=false;
        int choice;
        NEXUS_PlaybackTrickModeSettings trickmode_settings;
        NEXUS_DisplaySettings displaySettings;
        NEXUS_VideoWindowSettings windowSettings;


        printf("Xcoder loopback player CMD:>");
        loopbackUsage();
        if(fgets(cmd, sizeof(cmd)-1, stdin)==NULL) {
            break;
        }
        for(i=0;cmd[i]!='\0';i++) {
            if(!pContext->loopbackStarted && (cmd[i]!='i' && cmd[i]!='o' && cmd[i]!='q'))
                continue;
            switch(cmd[i]) {
            case '?':
                loopbackUsage();
                break;
            case 's':
                printf( "pause\n" );
                rate = 0;
                NEXUS_Playback_Pause(pContext->playbackLoopback);
                break;
            case 'p':
                printf( "play\n" );
                rate = NEXUS_NORMAL_PLAY_SPEED;
                NEXUS_Playback_Play(pContext->playbackLoopback);
                break;
            case 'f':
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                if(rate<=0) {
                    rate = NEXUS_NORMAL_PLAY_SPEED;
                } else
                rate *=2;
                trickmode_settings.rate = rate;
                printf( "fast forward %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode( pContext->playbackLoopback, &trickmode_settings );
                break;
            case 'l':
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                rate /=2;
                trickmode_settings.rate = rate;
                printf( "slow down %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode( pContext->playbackLoopback, &trickmode_settings );
                break;
            case 'q':
                quit = true;
                break;
            case 'r':
                NEXUS_Playback_GetDefaultTrickModeSettings( &trickmode_settings );
                if(rate>=0) {
                    rate = -NEXUS_NORMAL_PLAY_SPEED;
                } else
                rate *=2;
                trickmode_settings.rate = rate;
                printf( "rewind %d\n", trickmode_settings.rate );
                NEXUS_Playback_TrickMode( pContext->playbackLoopback, &trickmode_settings );
                break;
            case 'w':
                BKNI_Sleep(30);
                break;
            case '-':
                BDBG_ASSERT(!NEXUS_Playback_GetStatus(pContext->playbackLoopback, &playbackStatus));
                if (playbackStatus.position >= 5*1000) {
                    playbackStatus.position -= 5*1000;
                    /* it's normal for a Seek to fail if it goes past the beginning */
                    printf("Jump to %u, %u...%u\n", (unsigned)playbackStatus.position, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
                    (void)NEXUS_Playback_Seek(pContext->playbackLoopback, playbackStatus.position);
                }
                break;
            case '+':
                BDBG_ASSERT(!NEXUS_Playback_GetStatus(pContext->playbackLoopback, &playbackStatus));
                /* it's normal for a Seek to fail if it goes past the end */
                playbackStatus.position += 5*1000;
                printf("Jump to %u, %u...%u\n", (unsigned)playbackStatus.position, (unsigned)playbackStatus.first, (unsigned)playbackStatus.last);
                (void)NEXUS_Playback_Seek(pContext->playbackLoopback, playbackStatus.position);
                break;
            case '0':
                jump_to_beginning(pContext->playbackLoopback);
                break;
            case 'd': /* display format switch */
                NEXUS_Display_GetSettings(pContext->displayLoopback, &displaySettings);
                printf("switch loopback display (hdmi/component out) format to:\n");
                printf("\n displayFormat:\n");
                print_list(g_videoFormatStrs);
                scanf("%s", input);
                displaySettings.format = lookup(g_videoFormatStrs, input);
                NEXUS_Display_SetSettings(pContext->displayLoopback, &displaySettings);
                break;
            case 'z': /* resize loopback video window */
                NEXUS_VideoWindow_GetSettings(pContext->windowLoopback, &windowSettings);
                printf("Resize loopback window to:\n");
                printf("position x: \n"); scanf("%d", &choice); windowSettings.position.x = (int16_t)choice;
                printf("position y: \n"); scanf("%d", &choice); windowSettings.position.y = (int16_t)choice;
                printf("position width: \n"); scanf("%d", &choice); windowSettings.position.width = (uint16_t)choice;
                printf("position height: \n"); scanf("%d", &choice); windowSettings.position.height= (uint16_t)choice;
                NEXUS_VideoWindow_SetSettings(pContext->windowLoopback, &windowSettings);
                break;
            case 'a': /* change loopback display aspect ratio correction mode */
                printf("\n Please select loopback window's aspect ratio correction mode:\n"
                    " (%d) Zoom\n"
                    " (%d) Box\n"
                    " (%d) Pan and Scan\n"
                    " (%d) Bypass aspect ratio correction\n"
                    " (%d) PanScan without additional aspect ratio correction\n",
                    (NEXUS_VideoWindowContentMode_eZoom),
                    (NEXUS_VideoWindowContentMode_eBox),
                    (NEXUS_VideoWindowContentMode_ePanScan),
                    (NEXUS_VideoWindowContentMode_eFull),
                    (NEXUS_VideoWindowContentMode_ePanScanWithoutCorrection));
                NEXUS_VideoWindow_GetSettings(pContext->windowLoopback, &windowSettings);
                scanf("%d", (int *)&windowSettings.contentMode);
                printf("\n Enable letterbox auto cut? (0) Disable; (1) enable;\n");
                scanf("%d", &choice); windowSettings.letterBoxAutoCut = choice;
                windowSettings.letterBoxDetect = windowSettings.letterBoxAutoCut;
                NEXUS_VideoWindow_SetSettings(pContext->windowLoopback, &windowSettings);
                break;
            case 'i': /* select xcoder loopback ID */
                B_Mutex_Lock(pTranscoder->mutexStarted);
                if(pContext->loopbackStarted) {
                    B_Mutex_Unlock(pTranscoder->mutexStarted);
                    printf("Already started loopback on xcoder%d\n", pContext->loopbackXcodeId);
                    break;
                }
                B_Mutex_Unlock(pTranscoder->mutexStarted);
                printf("Please select Xcoder ID for loopback [0 ~ %u]: ", NEXUS_NUM_VIDEO_ENCODERS-1);
                scanf("%u", &pContext->loopbackXcodeId);
                pContext->loopbackXcodeId = (pContext->loopbackXcodeId > NEXUS_NUM_VIDEO_ENCODERS-1)
                    ? (NEXUS_NUM_VIDEO_ENCODERS-1):pContext->loopbackXcodeId;
                pTranscoder = &pContext->xcodeContext[pContext->loopbackXcodeId];
                break;
            case 'o': /* open xcoder loopback */
                B_Mutex_Lock(pTranscoder->mutexStarted);
                if(!pContext->loopbackStarted && (pContext->loopbackXcodeId != (unsigned)(-1))) {
                    printf("To start xcoder%d loopback...\n", pContext->loopbackXcodeId);
                    xcode_loopback_setup(pContext);
                }
                B_Mutex_Unlock(pTranscoder->mutexStarted);
                break;
            case 'c': /* close xcoder loopback */
                B_Mutex_Lock(pTranscoder->mutexStarted);
                if(pContext->loopbackStarted) {
                    printf("To stop xcoder%d loopback...\n", pContext->loopbackXcodeId);
                    xcode_loopback_shutdown(pContext);
                }
                B_Mutex_Unlock(pTranscoder->mutexStarted);
                break;
            default:
                break;
            }
        }
        if(quit)  {
            break;
        }

    }
}
#endif
