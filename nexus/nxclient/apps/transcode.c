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
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_SIMPLE_DECODER && NEXUS_HAS_STREAM_MUX
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_simple_encoder.h"
#include "nexus_playback.h"
#include "nexus_file.h"
#include "nexus_core_utils.h"
#include "media_probe.h"
#include "bcmindexer.h"
#include "brecord_gui.h"
#include "dvr_crypto.h"
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input.h"
#endif
#include "nexus_file_mux.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "bstd.h"
#include "bkni.h"

#include "namevalue.h"
#include "nxapps_cmdline.h"

BDBG_MODULE(transcode);

typedef struct EncodeContext
{
    NEXUS_SimpleEncoderHandle hEncoder;
    NEXUS_SimpleVideoDecoderHandle hVideoDecoder;
    NEXUS_SimpleAudioDecoderHandle hAudioDecoder;
    NEXUS_PlaybackHandle hPlayback;
    NEXUS_PlaypumpHandle hPlaypump;
    NEXUS_RecpumpHandle hRecpump;
    BNAV_Indexer_Handle bcmindexer;
#if NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInputHandle hdmiInput;
#endif
    NEXUS_FileMuxHandle fileMux;
    NEXUS_MuxFileHandle muxFileOutput;
    BKNI_EventHandle finishEvent;
    const char *filename, *indexname, *outputfile, *outputindex, *outputAes;
    NEXUS_FilePlayHandle file;
    FILE *pOutputFile;
    FILE *pOutputIndex;
    FILE *pOutputAes;
    void *pVideoBase;
    void *pAudioBase;
    size_t videoSize, lastVideoSize;
    size_t audioSize, lastAudioSize;
    unsigned streamTotal, indexTotal, lastsize;
    bool stopped;
    bool outputEs;
    bool outputMp4;
    struct {
        unsigned videoBitrate;/* bps */
        unsigned width;
        unsigned height;
        unsigned window_x, window_y, window_w, window_h;
        NEXUS_ClipRect clip;
        bool interlaced;
        NEXUS_VideoFrameRate frameRateEnum;
        unsigned frameRate, refreshRate;
        bool rampBitrate;
        NEXUS_SimpleEncoderStopMode stopMode;
        unsigned audioBitrate;/* bps */
        bool overrideOrientation;
        NEXUS_VideoOrientation orientation;
    } encoderSettings;
    struct {
        unsigned videoPid;
        unsigned videoCodec;
        unsigned videoCodecProfile;
        unsigned videoCodecLevel;
        unsigned audioPid;
        unsigned audioCodec;
        unsigned audioSampleRate;
        bool audPassThrough;
        bool raiIndex;
        NEXUS_PidChannelHandle passthrough[NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS];
        unsigned num_passthrough;
        NEXUS_KeySlotHandle keyslot;
        bool useInitialPts;
        uint32_t initialPts;
    } encoderStartSettings;
    NEXUS_SimpleEncoderStartSettings startSettings;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    enum {input_type_decoder, input_type_hdmi, input_type_graphics} input_type;
    struct {
        NEXUS_VideoImageInputHandle handle;
        NEXUS_SurfaceHandle surface;
    } imageInput;
}EncodeContext;

#define IS_START_OF_UNIT(pDesc) (((pDesc)->flags & (NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_START|NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS))?true:false)
#define MB (1024*1024)
#define NULL_PID 0x1fff

static bool HaveCompletePacket(
    const NEXUS_VideoEncoderDescriptor *pDesc0,
    size_t size0,
    const NEXUS_VideoEncoderDescriptor *pDesc1,
    size_t size1,
    size_t *pNumDesc)
{
    *pNumDesc=0;
    if ( size0 <= 0 )
    {
        return false;
    }
    if ( size0 + size1 < 2 )
    {
        return false;
    }
    if ( IS_START_OF_UNIT(pDesc0) )
    {
        /* We have a start of frame or data unit */
        *pNumDesc=*pNumDesc+1;
        pDesc0++;
        size0--;
        /* Look for next one */
        while ( size0 > 0 )
        {
            if ( IS_START_OF_UNIT(pDesc0) )
            {
                return true;
            }
            *pNumDesc=*pNumDesc+1;
            pDesc0++;
            size0--;
        }
        while ( size1 > 0 )
        {
            if ( IS_START_OF_UNIT(pDesc1) )
            {
                return true;
            }
            *pNumDesc=*pNumDesc+1;
            pDesc1++;
            size1--;
        }
    }
    *pNumDesc=0;
    return false;
}

#define ADVANCE_DESC(ptr0, ptr1) do { if ( size0 > 1 ) { ptr0++; size0--; } else { ptr0=ptr1; size0=size1; ptr1=NULL; size1=0; } } while (0)
#define DESC_DATA_PTR(PDESC, BASEPTR) ((void*)&((unsigned char *)(BASEPTR))[(PDESC)->offset])

static void *encoder_capture(void *context)
{
    EncodeContext *pContext = context;
    const NEXUS_VideoEncoderDescriptor *pDesc0, *pDesc1;
    const NEXUS_AudioMuxOutputFrame *pAudioDesc0, *pAudioDesc1;
    size_t size0, size1;
    void *pBufferBase=pContext->pVideoBase, *pAudioBase=pContext->pAudioBase;
    NEXUS_Error rc;
    void *pData;

    {
        size_t numRequired=0;
        bool foundData=false;

        /* capture video es */
        rc = NEXUS_SimpleEncoder_GetVideoBuffer(pContext->hEncoder, &pDesc0, &size0, &pDesc1, &size1);
        if ( NEXUS_SUCCESS == rc && size0 > 0 )
        {
            foundData = true;
            /* Check for eos */
            if ( pDesc0->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS )
            {
                BDBG_WRN(("Encoder EOS received"));
                BKNI_Sleep(1000);
                goto capture_audio_es;
            }
            /* Drop metadata and empty frames first */
            if ( pDesc0->flags & (NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_METADATA|NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EMPTY_FRAME) )
            {
                /* Drop this and retry */
                NEXUS_SimpleEncoder_VideoReadComplete(pContext->hEncoder, 1);
                goto capture_audio_es;
            }
            /* See if we have a complete unit of data */
            if ( false == HaveCompletePacket(pDesc0,size0,pDesc1,size1,&numRequired) )
            {
                goto capture_audio_es;
            }
            if ( pDesc0->flags & NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_START )
            {
                /* Frame data, just write to file */
                while ( numRequired > 0 )
                {
                    /* Write payload to file */
                    pData = DESC_DATA_PTR(pDesc0, pBufferBase);
                    fwrite(pData, 1, pDesc0->length, pContext->pOutputFile);
                    numRequired--;
                    pContext->videoSize += pDesc0->length;
                    NEXUS_SimpleEncoder_VideoReadComplete(pContext->hEncoder, 1);
                    ADVANCE_DESC(pDesc0, pDesc1);
                }
            }
        }
capture_audio_es:
        /* capture audio es */
        rc = NEXUS_SimpleEncoder_GetAudioBuffer(pContext->hEncoder, &pAudioDesc0, &size0, &pAudioDesc1, &size1);
        if ( NEXUS_SUCCESS == rc && size0 > 0 )
        {
            foundData = true;
            numRequired = size0 + size1;
            /* Drop metadata and empty frames first */
            if ( pAudioDesc0->flags & (NEXUS_AUDIOMUXOUTPUTFRAME_FLAG_METADATA) )
            {
                /* Drop this and retry */
                NEXUS_SimpleEncoder_AudioReadComplete(pContext->hEncoder, 1);
                goto done;
            }
            else
            {
                /* Frame data, just write to file */
                while ( numRequired > 0 )
                {
                    /* Write payload to file */
                    pData = DESC_DATA_PTR(pAudioDesc0, pAudioBase);
                    fwrite(pData, 1, pAudioDesc0->length, pContext->pOutputAes);
                    pContext->audioSize += pAudioDesc0->length;
                    numRequired--;
                    NEXUS_SimpleEncoder_AudioReadComplete(pContext->hEncoder, 1);
                    ADVANCE_DESC(pAudioDesc0, pAudioDesc1);
                }
            }
        }

        if ( !foundData )
        {
            BKNI_Sleep(100);
        }
    }

done:
    return NULL;

}

static void complete(void *data, int unused)
{
    BSTD_UNUSED(unused);
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
        "Usage: transcode [OPTIONS] {INPUTFILE|-hdmi_input|-graphics} [OUTPUTFILE [OUTPUTINDEX]]\n"
        "\n"
        "Default OUTPUTFILE videos/stream#.mpg where # is the RAVE context used\n"
        "\n"
        "OPTIONS:\n"
        "  --help or -h for help\n"
        );
    printf(
        "  -video         output video pid (0 for no video)\n"
        );
    print_list_option(
        "  -video_type    output video type",g_videoCodecStrs);
    printf(
        "  -video_bitrate RATE   output video bitrate in Mbps\n"
        "  -video_size    WIDTH,HEIGHT (default is 1280,720)\n"
        "  -window X,Y,WIDTH,HEIGHT    window within video_size (default is full size)\n"
        "  -clip L,R,T,B  source clip size (Left, Right, Top, Bottom; default no clip)\n"
        );
    printf(
        "  -interlaced\n"
        "  -video_framerate HZ    video encode frame rate (for example 29.97, 30, 59.94, 60)\n"
        "  -video_refreshrate HZ  video encode display refresh rate (for example 29.97, 30, 59.94, 60)\n"
        "  -3d {lr|ou}    output stereoscopic (3D) video\n"
        "  -profile       output video codec profile\n"
        "  -level         output video codec level\n"
        );
    printf(
        "  -audio         output audio pid (0 for no audio)\n"
        );
    print_list_option(
        "  -audio_type    output audio type",g_audioCodecStrs);
    printf(
        "  -audio_passthrough    audio codec passthrough\n"
        "  -audio_samplerate     audio output sample rate in HZ\n"
        "  -audio_bitrate  output audio bitrate in bps\n"
        "  -loop          loop playback\n"
        "  -timeout SECONDS\n"
        "  -ramp_bitrate  to ramp up output bitrate from a quarter of video_bitrate\n"
        );
    printf(
        "  -stopStartVideoEncoder   stop/start video encoder only\n"
        "  -output_es     output elementary stream\n"
        "  -output_mp4    output mp4 file\n"
        "  -rt            real-time encoding (default is non-real-time)\n"
        "  -tts           record with 4 byte timestamp\n"
        "  -gui off \n"
        "  -initial_pts PTS\n"
        );
    printf(
        "  -passthrough PID[=REMAP] Passthrough PID with optional remap PID. Option can be given %d times.\n", NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS
        );
    nxapps_cmdline_print_usage(cmdline);
    printf(
        "\n"
        "Input parameters\n"
        "  -hdmi_input    encode hdmi input instead of INPUTFILE\n"
        "  -graphics      encode graphics instead of INPUTFILE\n"
        );
    print_list_option(
        "  -format        max source format",g_videoFormatStrs);
}

static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static void get_default_bcmindexer_settings(NEXUS_VideoCodec videoCodec, FILE *indexOut, BNAV_Indexer_Settings *pSettings)
{
    BNAV_Indexer_GetDefaultSettings(pSettings);
    pSettings->writeCallback = (BNAV_WRITE_CB)fwrite;
    pSettings->filePointer = (void *)indexOut;
    pSettings->sctVersion = BSCT_Version6wordEntry;
    switch (videoCodec) {
    default:
    case NEXUS_VideoCodec_eMpeg2:
        pSettings->videoFormat = BNAV_Indexer_VideoFormat_MPEG2;
        break;
    case NEXUS_VideoCodec_eH264:
        pSettings->videoFormat = BNAV_Indexer_VideoFormat_AVC;
        pSettings->navVersion = BNAV_Version_AVC;
        break;
    case NEXUS_VideoCodec_eH264_Svc:
        pSettings->videoFormat = BNAV_Indexer_VideoFormat_AVC_SVC;
        pSettings->navVersion = BNAV_Version_AVC_Extended;
        break;
    case NEXUS_VideoCodec_eH264_Mvc:
        pSettings->videoFormat = BNAV_Indexer_VideoFormat_AVC_MVC;
        pSettings->navVersion = BNAV_Version_AVC_Extended;
        break;
    case NEXUS_VideoCodec_eVc1:
        pSettings->videoFormat = BNAV_Indexer_VideoFormat_VC1;
        break;
    case NEXUS_VideoCodec_eAvs:
        pSettings->videoFormat = BNAV_Indexer_VideoFormat_AVS;
        break;
    }
}

static unsigned rai_Indexer_Feed(FILE *fp, const void*indexBuffer, unsigned numEntries, unsigned raiCnt)
{
    unsigned i;
    off_t highByte, bytesRecordedTillCurrentRai;
    const BSCT_SixWord_Entry *pIndex = (const BSCT_SixWord_Entry*)indexBuffer;

    BDBG_MSG(("numEntries = %u", numEntries));
    for(i = 0; i < numEntries; i++, pIndex++) {
        /* byte offset since the start of the record session */
        highByte = ((off_t)*((uint32_t*)indexBuffer + 2) >> 24);
        bytesRecordedTillCurrentRai = highByte << 32;
        bytesRecordedTillCurrentRai |= (off_t)*((uint32_t*)indexBuffer + 3);
        BDBG_MSG(("RAI[%u] byte offset: "BDBG_UINT64_FMT, raiCnt, BDBG_UINT64_ARG(bytesRecordedTillCurrentRai)));
        /* log the rai info */
        fprintf(fp, "%u,%#x%08x\n", raiCnt++, (uint32_t)highByte, (uint32_t)(bytesRecordedTillCurrentRai));
    }
    return i;
}

static int start_encode(EncodeContext *pContext)
{
    NEXUS_SimpleEncoderSettings encoderSettings;
    NEXUS_SimpleEncoderStartSettings startSettings;
    NEXUS_SimpleEncoderStatus encStatus;
    unsigned i;
    NEXUS_Error rc;

    if (pContext->input_type == input_type_decoder) {
        pContext->file = NEXUS_FilePlay_OpenPosix(pContext->filename, pContext->indexname);
        if (!pContext->file) {
            BDBG_ERR(("can't open files: %s %s", pContext->filename, pContext->indexname));
            return -1;
        }
    }
    if (!pContext->outputMp4) {
        pContext->pOutputFile = fopen(pContext->outputfile, "w+");
        if (!pContext->pOutputFile) {
            BDBG_ERR(("unable to open '%s' for writing", pContext->outputfile));
            return -1;
        }
        if (pContext->outputindex) {
            pContext->pOutputIndex = fopen(pContext->outputindex, "w+");
            if (!pContext->pOutputIndex) {
                BDBG_ERR(("unable to open '%s' for writing", pContext->outputindex));
                return -1;
            }
        }
        if (pContext->outputAes) {
            pContext->pOutputAes = fopen(pContext->outputAes, "w+");
            if (!pContext->pOutputAes) {
                BDBG_ERR(("unable to open '%s' for writing", pContext->outputAes));
                return -1;
            }
        }
    }
    NEXUS_SimpleEncoder_GetSettings(pContext->hEncoder, &encoderSettings);
    if (pContext->encoderSettings.videoBitrate) {
        encoderSettings.videoEncoder.bitrateMax = pContext->encoderSettings.videoBitrate;
    } else {
        pContext->encoderSettings.videoBitrate = encoderSettings.videoEncoder.bitrateMax;
    }
    if (pContext->encoderSettings.width || pContext->encoderSettings.height) {
        encoderSettings.video.width = pContext->encoderSettings.width;
        encoderSettings.video.height = pContext->encoderSettings.height;
    } else {/* save default */
        pContext->encoderSettings.width  = encoderSettings.video.width;
        pContext->encoderSettings.height = encoderSettings.video.height;
    }
    encoderSettings.video.window.x = pContext->encoderSettings.window_x;
    encoderSettings.video.window.y = pContext->encoderSettings.window_y;
    encoderSettings.video.window.width = pContext->encoderSettings.window_w;
    encoderSettings.video.window.height = pContext->encoderSettings.window_h;
    encoderSettings.video.clip = pContext->encoderSettings.clip;

    if(pContext->encoderSettings.rampBitrate) {/* ramp from quarter dimension and bitrate */
        encoderSettings.videoEncoder.bitrateMax /= 4;
        encoderSettings.video.width /= 2;
        encoderSettings.video.height /= 2;
        BDBG_WRN(("Video bitrate ramps up -> %u bps; resolution: %ux%u", encoderSettings.videoEncoder.bitrateMax,
            encoderSettings.video.width, encoderSettings.video.height));
    }

#if !NEXUS_NUM_DSP_VIDEO_ENCODERS
    if (pContext->encoderSettings.frameRateEnum) {
        encoderSettings.videoEncoder.frameRate = pContext->encoderSettings.frameRateEnum;
    }
    switch (encoderSettings.videoEncoder.frameRate) {
    case NEXUS_VideoFrameRate_e50:
    case NEXUS_VideoFrameRate_e25:
    case NEXUS_VideoFrameRate_e12_5:
        encoderSettings.video.refreshRate = pContext->encoderSettings.refreshRate? pContext->encoderSettings.refreshRate : 50000;
        break;
    default:
        encoderSettings.video.refreshRate = pContext->encoderSettings.refreshRate? pContext->encoderSettings.refreshRate : 60000;
        break;
    }
#endif

    encoderSettings.video.interlaced = pContext->encoderSettings.interlaced;
    encoderSettings.video.display3DSettings.overrideOrientation = pContext->encoderSettings.overrideOrientation;
    encoderSettings.video.display3DSettings.orientation = pContext->encoderSettings.orientation;
    encoderSettings.stopMode = pContext->encoderSettings.stopMode;


    if (pContext->encoderStartSettings.audioCodec)
    {
        NEXUS_AudioEncoder_GetDefaultCodecSettings(pContext->encoderStartSettings.audioCodec, &encoderSettings.audioEncoder);
        if (pContext->encoderSettings.audioBitrate)
        {
            switch (pContext->encoderStartSettings.audioCodec) {
            case NEXUS_AudioCodec_eMp3:
                encoderSettings.audioEncoder.codecSettings.mp3.bitRate = pContext->encoderSettings.audioBitrate;
                break;
            case NEXUS_AudioCodec_eAacAdts:
            case NEXUS_AudioCodec_eAacLoas:
                encoderSettings.audioEncoder.codecSettings.aac.bitRate = pContext->encoderSettings.audioBitrate;
                break;
            case NEXUS_AudioCodec_eAacPlusAdts:
            case NEXUS_AudioCodec_eAacPlusLoas:
                encoderSettings.audioEncoder.codecSettings.aacPlus.bitRate = pContext->encoderSettings.audioBitrate;
                break;
            default:
                break;
            }
        }
        if (pContext->encoderStartSettings.audioSampleRate) {
            switch (pContext->encoderStartSettings.audioCodec) {
            case NEXUS_AudioCodec_eAacAdts:
            case NEXUS_AudioCodec_eAacLoas:
                encoderSettings.audioEncoder.codecSettings.aac.sampleRate = pContext->encoderStartSettings.audioSampleRate;
                break;
            case NEXUS_AudioCodec_eAacPlusAdts:
            case NEXUS_AudioCodec_eAacPlusLoas:
                encoderSettings.audioEncoder.codecSettings.aacPlus.sampleRate = pContext->encoderStartSettings.audioSampleRate;
                break;
            default:
                break;
            }
        }
    }

    encoderSettings.finished.callback = complete;
    encoderSettings.finished.context = pContext->finishEvent;
    rc = NEXUS_SimpleEncoder_SetSettings(pContext->hEncoder, &encoderSettings);
    BDBG_ASSERT(!rc);

    NEXUS_SimpleEncoder_GetDefaultStartSettings(&startSettings);
    startSettings.input.video = pContext->hVideoDecoder;
    startSettings.input.audio = pContext->hAudioDecoder;
    startSettings.recpump = (pContext->outputEs || pContext->outputMp4)? NULL : pContext->hRecpump;
    if (pContext->outputEs)
        startSettings.output.transport.type = NEXUS_TransportType_eEs;
    else if (pContext->outputMp4)
        startSettings.output.transport.type = NEXUS_TransportType_eMp4;
    else
        startSettings.output.transport.type = NEXUS_TransportType_eTs;
    startSettings.output.video.index = !pContext->encoderStartSettings.raiIndex;
    startSettings.output.video.raiIndex = pContext->encoderStartSettings.raiIndex;
    startSettings.output.video.settings.bounds.inputDimension.max.width = pContext->encoderSettings.width;
    startSettings.output.video.settings.bounds.inputDimension.max.height = pContext->encoderSettings.height;
    startSettings.output.video.settings.interlaced = pContext->encoderSettings.interlaced;
    startSettings.transcode.useInitialPts = pContext->encoderStartSettings.useInitialPts;
    startSettings.transcode.initialPts = pContext->encoderStartSettings.initialPts;
    if (pContext->encoderStartSettings.videoCodec) {
        startSettings.output.video.settings.codec = pContext->encoderStartSettings.videoCodec;
    }
    if (pContext->encoderStartSettings.videoCodecProfile) {
        startSettings.output.video.settings.profile = pContext->encoderStartSettings.videoCodecProfile;
    }
    if (pContext->encoderStartSettings.videoCodecLevel) {
        startSettings.output.video.settings.level = pContext->encoderStartSettings.videoCodecLevel;
    }
    if (pContext->encoderStartSettings.videoPid != NULL_PID) {
        startSettings.output.video.pid = pContext->encoderStartSettings.videoPid;
    }
    if (pContext->encoderStartSettings.audioCodec) {
        startSettings.output.audio.codec = pContext->encoderStartSettings.audioCodec;
    }
    if (pContext->encoderStartSettings.audioSampleRate != 0) {
        startSettings.output.audio.sampleRate = pContext->encoderStartSettings.audioSampleRate;
    }
    if (pContext->encoderStartSettings.audPassThrough) {
        startSettings.output.audio.passthrough = true;
        startSettings.output.audio.codec = pContext->audioProgram.primary.codec;
        startSettings.output.audio.sampleRate = 0; /* passthrough cannot change audio sample rate */
    }
    if (pContext->encoderStartSettings.audioPid != NULL_PID) {
        startSettings.output.audio.pid = pContext->encoderStartSettings.audioPid;
    }
    for (i=0;i<pContext->encoderStartSettings.num_passthrough;i++) {
        startSettings.passthrough[i] = pContext->encoderStartSettings.passthrough[i];
    }
    startSettings.output.video.keyslot = pContext->encoderStartSettings.keyslot;
    startSettings.output.audio.keyslot = pContext->encoderStartSettings.keyslot;
    for (i=0;i<pContext->encoderStartSettings.num_passthrough;i++) {
        startSettings.output.passthrough[i].keyslot = pContext->encoderStartSettings.keyslot;
    }

    rc = NEXUS_SimpleEncoder_Start(pContext->hEncoder, &startSettings);
    if (rc) {
        BDBG_ERR(("unable to start encoder"));
        return -1;
    }
    pContext->startSettings = startSettings;

    /* can't start decode before encode because encoder provides display for decoder */
    if (pContext->videoProgram.settings.pidChannel) {
        rc = NEXUS_SimpleVideoDecoder_Start(pContext->hVideoDecoder, &pContext->videoProgram);
        BDBG_ASSERT(!rc);
    }
    if (pContext->hAudioDecoder && pContext->audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_Start(pContext->hAudioDecoder, &pContext->audioProgram);
        /* decode may fail if audio codec not supported */
    }
#if NEXUS_HAS_HDMI_INPUT
    if (pContext->input_type == input_type_hdmi) {
        if (pContext->hVideoDecoder) {
            rc = NEXUS_SimpleVideoDecoder_StartHdmiInput(pContext->hVideoDecoder, pContext->hdmiInput, NULL);
            BDBG_ASSERT(!rc);
        }
        if (pContext->hAudioDecoder) {
            rc = NEXUS_SimpleAudioDecoder_StartHdmiInput(pContext->hAudioDecoder, pContext->hdmiInput, NULL);
            BDBG_ASSERT(!rc);
        }
    }
#endif

    if (pContext->outputMp4) {
        NEXUS_FileMuxStartSettings muxFileConfig;

        pContext->muxFileOutput = NEXUS_MuxFile_OpenPosix(pContext->outputfile);
        if (!pContext->muxFileOutput) {
            BDBG_ERR(("unable to create MuxFile '%s' for FileMux", pContext->outputfile));
            return -1;
        }

        /* hard-code codecs for now */
        if (!pContext->encoderStartSettings.videoCodec)
            pContext->encoderStartSettings.videoCodec = NEXUS_VideoCodec_eH264;
        if (!pContext->encoderStartSettings.audioCodec)
            pContext->encoderStartSettings.audioCodec = NEXUS_AudioCodec_eAac;
        NEXUS_FileMux_GetDefaultStartSettings(&muxFileConfig, NEXUS_TransportType_eMp4);
        /* progressive download comptible means the meta data will be relocated to the beginning of the
         * mp4 file, which means longer finish time to stop the file mux  */
        /*muxFileConfig.streamSettings.mp4.progressiveDownloadCompatible = true;*/
        if (pContext->hVideoDecoder) {
            muxFileConfig.video[0].track = 1;
            muxFileConfig.video[0].codec = pContext->encoderStartSettings.videoCodec;
            muxFileConfig.video[0].simpleEncoder = pContext->hEncoder;
        }
        if(pContext->hAudioDecoder) {
            muxFileConfig.audio[0].track = 2;
            muxFileConfig.audio[0].codec = pContext->encoderStartSettings.audioCodec;
            muxFileConfig.audio[0].simpleEncoder = pContext->hEncoder;
        }

        /* start mux */
        rc = NEXUS_FileMux_Start(pContext->fileMux, &muxFileConfig, pContext->muxFileOutput);
        BDBG_MSG(("NEXUS_FileMux_Start return %d", rc));
    }

    if (pContext->input_type == input_type_graphics) {
        NEXUS_SurfaceCreateSettings surfaceCreateSettings;
        NEXUS_SurfaceMemory mem;
        unsigned x,y,i;
        NEXUS_ClientConfiguration clientConfig;
        NEXUS_VideoImageInputStatus imageInputStatus;

        NEXUS_Platform_GetClientConfiguration(&clientConfig);

        pContext->imageInput.handle = NEXUS_SimpleVideoDecoder_StartImageInput(pContext->hVideoDecoder, NULL);
        if (!pContext->imageInput.handle) {
            BDBG_WRN(("graphics transcode not supported"));
            return -1;
        }

        NEXUS_VideoImageInput_GetStatus(pContext->imageInput.handle, &imageInputStatus);

        if(pContext->imageInput.surface == NULL) {
            NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
            surfaceCreateSettings.width  = 720;
            surfaceCreateSettings.height = 480;
            surfaceCreateSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
            for (i=0;i<NEXUS_MAX_HEAPS;i++) {
                NEXUS_MemoryStatus s;
                if (!clientConfig.heap[i] || NEXUS_Heap_GetStatus(clientConfig.heap[i], &s)) continue;
                if (s.memcIndex == imageInputStatus.memcIndex && (s.memoryType & NEXUS_MemoryType_eApplication) && s.largestFreeBlock >= 960*1080*2) {
                    surfaceCreateSettings.heap = clientConfig.heap[i];
                    BDBG_WRN(("found heap[%d] on MEMC%d for VideoImageInput", i, s.memcIndex));
                    break;
                }
            }
            if (!surfaceCreateSettings.heap) {
                BDBG_ERR(("no heap found. RTS failure likely."));
            }
            /* create, render and push a single surface */
            pContext->imageInput.surface = NEXUS_Surface_Create(&surfaceCreateSettings);
            NEXUS_Surface_GetMemory(pContext->imageInput.surface, &mem);
            /* draw green/magenta checker board splash screen */
            for (y=0;y<surfaceCreateSettings.height;y++) {
                uint32_t *ptr = (uint32_t *)(((uint8_t*)mem.buffer) + y * mem.pitch);
                for (x=0;x<surfaceCreateSettings.width;x++) {
                    if ((x/10)%2 != (y/10)%2) {
                        ptr[x] = 0x00800080;
                    }
                    else {
                        ptr[x] = 0xFF80FF80;
                    }
                }
            }
            NEXUS_Surface_Flush(pContext->imageInput.surface);
            NEXUS_VideoImageInput_PushSurface(pContext->imageInput.handle, pContext->imageInput.surface, NULL);
        }
    }

    /* use bcmindexer to convert SCT to NAV */
    if (pContext->outputindex) {
        BNAV_Indexer_Settings settings;
        get_default_bcmindexer_settings(startSettings.output.video.settings.codec, pContext->pOutputIndex, &settings);
        rc = BNAV_Indexer_Open(&pContext->bcmindexer, &settings);
        BDBG_ASSERT(!rc);
    }

    /* recpump must start after decoders start */
    if(!pContext->outputEs && !pContext->outputMp4) {
        rc = NEXUS_Recpump_Start(pContext->hRecpump);
        BDBG_ASSERT(!rc);
    }

    if (pContext->hPlayback) {
        rc = NEXUS_Playback_Start(pContext->hPlayback, pContext->file, NULL);
        BDBG_ASSERT(!rc);
    }

    NEXUS_SimpleEncoder_GetStatus(pContext->hEncoder, &encStatus);
    rc = NEXUS_MemoryBlock_Lock(encStatus.video.bufferBlock, &pContext->pVideoBase);
    BDBG_ASSERT(!rc);
    if (encStatus.audio.bufferBlock) {
        rc = NEXUS_MemoryBlock_Lock(encStatus.audio.bufferBlock, &pContext->pAudioBase);
        BDBG_ASSERT(!rc);
    }

    return 0;
}

static void stop_encode(EncodeContext *pContext)
{
    NEXUS_SimpleEncoderSettings encoderSettings;

    if (pContext->hPlayback) {
        NEXUS_Playback_Stop(pContext->hPlayback);
    }
    if (pContext->hVideoDecoder) {
        NEXUS_SimpleVideoDecoder_Stop(pContext->hVideoDecoder);
    }
    if (pContext->hAudioDecoder) {
        NEXUS_SimpleAudioDecoder_Stop(pContext->hAudioDecoder);
    }

    if (BKNI_WaitForEvent(pContext->finishEvent, 5 * 60 * 1000) != BERR_SUCCESS) {
        BDBG_ERR(("SimpleEncoder finish timeout"));
    }
    /* resume auto stop mode before stop */
    NEXUS_SimpleEncoder_GetSettings(pContext->hEncoder,&encoderSettings);
    encoderSettings.stopMode = NEXUS_SimpleEncoderStopMode_eAll;
    NEXUS_SimpleEncoder_SetSettings(pContext->hEncoder,&encoderSettings);
    NEXUS_SimpleEncoder_Stop(pContext->hEncoder);

    if(!pContext->outputEs && !pContext->outputMp4) {
        NEXUS_Recpump_Stop(pContext->hRecpump);
    }

    if (pContext->input_type == input_type_hdmi) {
        if (pContext->hVideoDecoder) {
            NEXUS_SimpleVideoDecoder_StopHdmiInput(pContext->hVideoDecoder);
        }
        if (pContext->hAudioDecoder) {
            NEXUS_SimpleAudioDecoder_StopHdmiInput(pContext->hAudioDecoder);
        }
    }
    if (pContext->input_type == input_type_graphics) {
        NEXUS_SimpleVideoDecoder_StopImageInput(pContext->hVideoDecoder);
    }
    if (pContext->outputMp4) {
        NEXUS_FileMux_Finish(pContext->fileMux);
        if (BKNI_WaitForEvent(pContext->finishEvent, 5 * 60 * 1000) != BERR_SUCCESS) {
            BDBG_ERR(("MuxFinish TimeOut"));
        }
        NEXUS_FileMux_Stop(pContext->fileMux);
        NEXUS_MuxFile_Close(pContext->muxFileOutput);
        NEXUS_FileMux_Destroy(pContext->fileMux);
    }
    if (pContext->outputAes) {
        fclose(pContext->pOutputAes);
    }
    if (pContext->outputindex) {
        fclose(pContext->pOutputIndex);
        BNAV_Indexer_Close(pContext->bcmindexer);
    }
    if (!pContext->outputMp4)
        fclose(pContext->pOutputFile);
    if (pContext->input_type == input_type_decoder) {
        NEXUS_FilePlay_Close(pContext->file);
    }
    pContext->videoSize=pContext->lastVideoSize=pContext->audioSize=pContext->lastAudioSize=0;
    pContext->streamTotal=pContext->indexTotal=pContext->lastsize=0;
}

static void *standby_monitor(void *context)
{
    EncodeContext *pContext = context;
    NEXUS_Error rc;
    NxClient_StandbyStatus standbyStatus, prevStatus;

    rc = NxClient_GetStandbyStatus(&prevStatus);
    if (rc) exit(0); /* server is down, exit gracefully */

    while(!pContext->stopped) {
        rc = NxClient_GetStandbyStatus(&standbyStatus);
        if (rc) exit(0); /* server is down, exit gracefully */

        if(standbyStatus.transition == NxClient_StandbyTransition_eAckNeeded) {
            printf("'transcode' acknowledges standby state: %s\n", lookup_name(g_platformStandbyModeStrs, standbyStatus.settings.mode));
            stop_encode(pContext);
            NxClient_AcknowledgeStandby(true);
        } else {
            if(standbyStatus.settings.mode == NEXUS_PlatformStandbyMode_eOn && prevStatus.settings.mode != NEXUS_PlatformStandbyMode_eOn)
                start_encode(pContext);

        }

        prevStatus = standbyStatus;
        BKNI_Sleep(100);
    }

    return NULL;
}

int main(int argc, const char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    struct probe_results probe_results;
    unsigned connectId;
    NEXUS_SimpleEncoderSettings encoderSettings;
    NEXUS_PlaypumpOpenSettings openSettings;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_Error rc = 0;
    int curarg = 1;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    BKNI_EventHandle dataReadyEvent;
    EncodeContext context;
    unsigned timeout = 0;
    bool loop = false;
    unsigned starttime, thistime;
    bool gui = true;
    brecord_gui_t record_gui = NULL;
    bool realtime = false;
    NEXUS_TransportTimestampType timestampType = NEXUS_TransportTimestampType_eNone;
    struct {
        unsigned pid, remap_pid;
    } passthrough[NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS];
    unsigned num_passthrough = 0;
    unsigned i;
    struct nxapps_cmdline cmdline;
    int n;
    NEXUS_VideoFormat maxFormat = 0;
    pthread_t standby_thread_id;
    NEXUS_SecurityAlgorithm encrypt_algo = NEXUS_SecurityAlgorithm_eMax;
    dvr_crypto_t crypto = NULL;

    BKNI_Memset(&context, 0, sizeof(context));
    context.encoderStartSettings.videoPid = context.encoderStartSettings.audioPid = NULL_PID;

    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings);

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage(&cmdline);
            return 0;
        }
        else if (!strcmp(argv[curarg], "-video") && curarg+1 < argc) {
            context.encoderStartSettings.videoPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-audio") && curarg+1 < argc) {
            context.encoderStartSettings.audioPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-audio_samplerate") && curarg+1 < argc) {
            context.encoderStartSettings.audioSampleRate = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-stopStartVideoEncoder")) {
            context.encoderSettings.stopMode = NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly;
        }
        else if (!strcmp(argv[curarg], "-video_type") && curarg+1 < argc) {
            context.encoderStartSettings.videoCodec = lookup(g_videoCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-profile") && curarg+1 < argc) {
            context.encoderStartSettings.videoCodecProfile = lookup(g_videoCodecProfileStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-level") && curarg+1 < argc) {
            context.encoderStartSettings.videoCodecLevel = lookup(g_videoCodecLevelStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-video_bitrate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            context.encoderSettings.videoBitrate = rate * 1000 * 1000;
        }
        else if (!strcmp(argv[curarg], "-audio_bitrate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            context.encoderSettings.audioBitrate = rate;
        }
        else if (!strcmp(argv[curarg], "-video_size") && curarg+1 < argc) {
            if (sscanf(argv[++curarg], "%u,%u", &context.encoderSettings.width, &context.encoderSettings.height) != 2) {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-window") && curarg+1 < argc) {
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &context.encoderSettings.window_x, &context.encoderSettings.window_y, &context.encoderSettings.window_w, &context.encoderSettings.window_h) != 4) {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-clip") && curarg+1 < argc) {
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &context.encoderSettings.clip.left, &context.encoderSettings.clip.right, &context.encoderSettings.clip.top, &context.encoderSettings.clip.bottom) != 4) {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-output_es")) {
            context.outputEs = true;
        }
        else if (!strcmp(argv[curarg], "-output_mp4")) {
            context.outputMp4 = true;
        }
        else if (!strcmp(argv[curarg], "-interlaced")) {
            context.encoderSettings.interlaced = true;
        }
        else if (!strcmp(argv[curarg], "-video_framerate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            context.encoderSettings.frameRate = rate * 1000;
        }
        else if (!strcmp(argv[curarg], "-video_refreshrate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            context.encoderSettings.refreshRate = rate * 1000;
        }
        else if (!strcmp(argv[curarg], "-3d") && curarg+1 < argc) {
            curarg++;
            if (!strcmp(argv[curarg],"lr")) {
                context.encoderSettings.overrideOrientation = true;
                context.encoderSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
            }
            else if (!strcmp(argv[curarg],"ou")) {
                context.encoderSettings.overrideOrientation = true;
                context.encoderSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
            }
            else {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-audio_type") && curarg+1 < argc) {
            context.encoderStartSettings.audioCodec = lookup(g_audioCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-loop")) {
            loop = true;
        }
        else if (!strcmp(argv[curarg], "-gui") && argc>curarg+1) {
            gui = strcmp(argv[++curarg], "off");
        }
        else if (!strcmp(argv[curarg], "-ramp_bitrate")) {
            context.encoderSettings.rampBitrate = true;
        }
        else if (!strcmp(argv[curarg], "-hdmi_input")) {
#if NEXUS_HAS_HDMI_INPUT
            context.input_type = input_type_hdmi;
            realtime = true;
#else
            BDBG_ERR(("hdmi input not supported"));
#endif
        }
        else if (!strcmp(argv[curarg], "-graphics")) {
            context.input_type = input_type_graphics;
            realtime = true;
        }
        else if (!strcmp(argv[curarg], "-format")) {
            maxFormat = lookup(g_videoFormatStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-rt")) {
            realtime = true;
        }
        else if (!strcmp(argv[curarg], "-audio_passthrough")) {
            context.encoderStartSettings.audPassThrough = true;
        }
        else if (!strcmp(argv[curarg], "-tts")) {
            timestampType = NEXUS_TransportTimestampType_eBinary;
        }
        else if (!strcmp(argv[curarg], "-rai_index")) {
            context.encoderStartSettings.raiIndex = true;
        }
        else if (!strcmp(argv[curarg], "-passthrough") && argc>curarg+1) {
            const char *s = argv[++curarg];
            char *find;
            if (num_passthrough == NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS) {
                print_usage(&cmdline);
                return -1;
            }
            find = strchr(s, '=');
            if (find) {
                *find++ = 0;
            }
            passthrough[num_passthrough].pid = strtoul(s, NULL, 0);
            if (find) {
                passthrough[num_passthrough].remap_pid = strtoul(find, NULL, 0);
            }
            else {
                passthrough[num_passthrough].remap_pid = 0;
            }
            context.encoderStartSettings.num_passthrough = ++num_passthrough;
        }
        else if (!strcmp(argv[curarg], "-crypto") && curarg+1 < argc) {
            encrypt_algo = lookup(g_securityAlgoStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-initial_pts") && curarg+1 < argc) {
            context.encoderStartSettings.useInitialPts = true;
            context.encoderStartSettings.initialPts = strtoul(argv[++curarg], NULL, 0);
        }
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else if (!context.filename && context.input_type == input_type_decoder) {
            context.filename = argv[curarg];
        }
        else if (!context.outputfile) {
            context.outputfile = argv[curarg];
        }
        else if (!context.outputindex) {
            context.outputindex = argv[curarg];
        }
        else {
            print_usage(&cmdline);
            return -1;
        }
        curarg++;
    }

    if (context.input_type == input_type_graphics) {
        if (context.filename) {
            print_usage(&cmdline);
            return -1;
        }
        context.filename = "graphics";
    }
    else if (context.input_type == input_type_hdmi) {
        if (context.filename) {
            print_usage(&cmdline);
            return -1;
        }
        context.filename = "hdmi";
    }
    if (!context.filename) {
        print_usage(&cmdline);
        return -1;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s %s", argv[0], context.filename);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    rc = pthread_create(&standby_thread_id, NULL, standby_monitor, &context);
    BDBG_ASSERT(!rc);

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BDBG_ASSERT(stcChannel);

    if (context.input_type == input_type_decoder) {
        rc = probe_media(context.filename, &probe_results);
        if (rc) return BERR_TRACE(rc);

        context.indexname = probe_results.useStreamAsIndex?context.filename:NULL;

        NEXUS_Playpump_GetDefaultOpenSettings(&openSettings);
        openSettings.fifoSize /= 2;
        context.hPlaypump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &openSettings);
        if (!context.hPlaypump) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}
        context.hPlayback = NEXUS_Playback_Create();
        BDBG_ASSERT(context.hPlayback);

        NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.modeSettings.Auto.transportType = probe_results.transportType;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        rc = NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
        if (rc) {BDBG_WRN(("unable to set stcsettings")); return -1;}

        NEXUS_Playback_GetSettings(context.hPlayback, &playbackSettings);
        playbackSettings.playpump = context.hPlaypump;
        playbackSettings.playpumpSettings.transportType = probe_results.transportType;
        playbackSettings.playpumpSettings.timestamp.type = probe_results.timestampType;
        playbackSettings.simpleStcChannel = stcChannel;
        playbackSettings.endOfStreamAction = loop ? NEXUS_PlaybackLoopMode_eLoop : NEXUS_PlaybackLoopMode_ePause;
        NEXUS_Playback_SetSettings(context.hPlayback, &playbackSettings);
    }
    else {
        memset(&probe_results, 0, sizeof(probe_results));
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = (context.encoderStartSettings.videoPid && (context.input_type != input_type_decoder || probe_results.num_video))?1:0;
    allocSettings.simpleAudioDecoder = (context.encoderStartSettings.audioPid && (context.input_type == input_type_hdmi || probe_results.num_audio))?1:0;
    allocSettings.simpleEncoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.encoder = true;
    if (context.input_type == input_type_decoder) {
        if (maxFormat) {
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxFormat = maxFormat;
        }
        else {
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = probe_results.video[0].width;
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = probe_results.video[0].height;
        }
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[probe_results.video[0].codec] = true;
    }
    else if (context.input_type == input_type_hdmi) {
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.connectType = NxClient_VideoDecoderConnectType_eWindowOnly;
    }
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    connectSettings.simpleEncoder[0].id = allocResults.simpleEncoder[0].id;
    connectSettings.simpleEncoder[0].nonRealTime = !realtime;
    if (context.encoderSettings.width && context.encoderSettings.height) {
        connectSettings.simpleEncoder[0].encoderCapabilities.maxWidth = context.encoderSettings.width;
        connectSettings.simpleEncoder[0].encoderCapabilities.maxHeight = context.encoderSettings.height;
    }
    if (context.encoderSettings.frameRate) {
        NEXUS_LookupFrameRate(context.encoderSettings.frameRate, &context.encoderSettings.frameRateEnum);
        /* with 1000/1001 rate tracking by default */
        switch (context.encoderSettings.frameRateEnum) {
        case NEXUS_VideoFrameRate_e29_97:  context.encoderSettings.frameRateEnum = NEXUS_VideoFrameRate_e30; break;
        case NEXUS_VideoFrameRate_e59_94:  context.encoderSettings.frameRateEnum = NEXUS_VideoFrameRate_e60; break;
        case NEXUS_VideoFrameRate_e23_976: context.encoderSettings.frameRateEnum = NEXUS_VideoFrameRate_e24; break;
        case NEXUS_VideoFrameRate_e14_985: context.encoderSettings.frameRateEnum = NEXUS_VideoFrameRate_e15; break;
        default: break;
        }

        connectSettings.simpleEncoder[0].encoderCapabilities.maxFrameRate = context.encoderSettings.frameRateEnum;
    }
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) {BDBG_WRN(("unable to connect transcode resources")); return -1;}

    memset(&context.videoProgram, 0, sizeof(context.videoProgram));
    memset(&context.audioProgram, 0, sizeof(context.audioProgram));

    if (allocResults.simpleVideoDecoder[0].id) {
        context.hVideoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
        if (!context.hVideoDecoder) {
            BDBG_WRN(("video decoder not available"));
        }
        else if (context.hPlayback) {
            NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&context.videoProgram);
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            playbackPidSettings.pidTypeSettings.video.codec = probe_results.video[0].codec;
            playbackPidSettings.pidTypeSettings.video.index = true;
            playbackPidSettings.pidTypeSettings.video.simpleDecoder = context.hVideoDecoder;
            context.videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(context.hPlayback, probe_results.video[0].pid, &playbackPidSettings);
            context.videoProgram.settings.codec = probe_results.video[0].codec;
            context.videoProgram.maxWidth = probe_results.video[0].width;
            context.videoProgram.maxHeight = probe_results.video[0].height;
            if (probe_results.video[0].colorDepth > 8) {
                NEXUS_VideoDecoderSettings settings;
                NEXUS_SimpleVideoDecoder_GetSettings(context.hVideoDecoder, &settings);
                settings.colorDepth = probe_results.video[0].colorDepth;
                NEXUS_SimpleVideoDecoder_SetSettings(context.hVideoDecoder, &settings);
            }
            BDBG_MSG(("transcode video %#x %s", probe_results.video[0].pid, lookup_name(g_videoCodecStrs, probe_results.video[0].codec)));
        }
        NEXUS_SimpleVideoDecoder_SetStcChannel(context.hVideoDecoder, stcChannel);
    }
    else {
        context.hVideoDecoder = NULL;
    }

    if (allocResults.simpleAudioDecoder.id) {
        context.hAudioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
        if (!context.hAudioDecoder) {
            BDBG_WRN(("audio decoder not available"));
        }
        else if (context.hPlayback) {
            NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&context.audioProgram);
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.simpleDecoder = context.hAudioDecoder;
            context.audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(context.hPlayback, probe_results.audio[0].pid, &playbackPidSettings);
            context.audioProgram.primary.codec = probe_results.audio[0].codec;
            BDBG_MSG(("transcode audio %#x %s", probe_results.audio[0].pid, lookup_name(g_audioCodecStrs, probe_results.audio[0].codec)));
        }
        NEXUS_SimpleAudioDecoder_SetStcChannel(context.hAudioDecoder, stcChannel);
    }
    else {
        context.hAudioDecoder = NULL;
    }

    BKNI_CreateEvent(&dataReadyEvent);
    BKNI_CreateEvent(&context.finishEvent);

    context.hEncoder = NEXUS_SimpleEncoder_Acquire(allocResults.simpleEncoder[0].id);
    BDBG_ASSERT(context.hEncoder);

#if NEXUS_HAS_HDMI_INPUT
    if (context.input_type == input_type_hdmi) {
        unsigned index = 0;
        context.hdmiInput = NEXUS_HdmiInput_Open(index, NULL);
        if (!context.hdmiInput) {
            BDBG_ERR(("HdmiInput %d not available", index));
            return -1;
        }
    }
#endif

    if(context.outputEs) {
        if (!context.outputfile) {
            static char buf[64], aes[64];
            context.outputfile = buf;
            context.outputAes  = aes;
            snprintf(buf, sizeof(buf), "videos/stream.ves");
            snprintf(aes, sizeof(aes), "videos/stream.aes");
        }
    } if (context.outputMp4) {
        NEXUS_FileMuxCreateSettings muxFileCreateSettings;

        NEXUS_FileMux_GetDefaultCreateSettings(&muxFileCreateSettings);
        muxFileCreateSettings.finished.callback = complete;
        muxFileCreateSettings.finished.context = context.finishEvent;
#if 0
        muxFileCreateSettings.mp4.metadataCache *= 2;
        muxFileCreateSettings.mp4.heapSize *= 2;
        muxFileCreateSettings.mp4.sizeEntriesCache *= 2;
        muxFileCreateSettings.mp4.relocationBuffer *= 8;
#endif
        context.fileMux = NEXUS_FileMux_Create(&muxFileCreateSettings);
        if (!context.fileMux) {
            BDBG_ERR(("file mux open failed!"));
            return -1;
        }
    } else {
        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        context.hRecpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
        BDBG_ASSERT(context.hRecpump);

        NEXUS_Recpump_GetSettings(context.hRecpump, &recpumpSettings);
        recpumpSettings.data.dataReady.callback = complete;
        recpumpSettings.data.dataReady.context = dataReadyEvent;
        recpumpSettings.index.dataReady.callback = complete;
        recpumpSettings.index.dataReady.context = dataReadyEvent;
        recpumpSettings.timestampType = timestampType;
        recpumpSettings.localTimestamp = (timestampType != NEXUS_TransportTimestampType_eNone);
        recpumpSettings.adjustTimestampUsingPcrs = (timestampType != NEXUS_TransportTimestampType_eNone);
        recpumpSettings.dropBtpPackets = (timestampType != NEXUS_TransportTimestampType_eNone);
        recpumpSettings.bandHold = !realtime; /* flow control required for NRT mode */
        rc = NEXUS_Recpump_SetSettings(context.hRecpump, &recpumpSettings);
        BDBG_ASSERT(!rc);

        if (!context.outputfile) {
            static char buf[64];
            NEXUS_RecpumpStatus status;
            NEXUS_Recpump_GetStatus(context.hRecpump, &status);
            context.outputfile = buf;
            snprintf(buf, sizeof(buf), "videos/stream%d.mpg", status.rave.index);
        }

        if (encrypt_algo < NEXUS_SecurityAlgorithm_eMax) {
            struct dvr_crypto_settings settings;
            dvr_crypto_get_default_settings(&settings);
            settings.algo = encrypt_algo;
            settings.encrypt = true;
            crypto = dvr_crypto_create(&settings);
            context.encoderStartSettings.keyslot = dvr_crypto_keyslot(crypto);
        }
    }

    if (gui && !context.outputEs && !context.outputMp4) {
        struct brecord_gui_settings settings;
        brecord_gui_get_default_settings(&settings);
        settings.sourceName = context.filename;
        settings.destName = context.outputfile;
        settings.recpump = context.hRecpump;
        settings.color = 0xFF00AAAA;
        record_gui = brecord_gui_create(&settings);
    }

    if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings)) {
        NEXUS_SimpleVideoDecoderPictureQualitySettings settings;
        NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(context.hVideoDecoder, &settings);
        nxapps_cmdline_apply_SimpleVideoDecodePictureQualitySettings(&cmdline, &settings);
        NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(context.hVideoDecoder, &settings);
    }

    for (i=0;i<num_passthrough;i++) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
        if (passthrough[i].remap_pid) {
            playbackPidSettings.pidSettings.pidSettings.remap.enabled = true;
            playbackPidSettings.pidSettings.pidSettings.remap.pid = passthrough[i].remap_pid;
        }
        context.encoderStartSettings.passthrough[i] = NEXUS_Playback_OpenPidChannel(context.hPlayback, passthrough[i].pid, &playbackPidSettings);
    }

    /* starting encode */
    if(start_encode(&context) != 0)
        return -1;

    BDBG_WRN(("%s -> %s: started", context.filename, context.outputfile));
    starttime = b_get_time();
    do {
        thistime = b_get_time();
        if(!context.outputEs && !context.outputMp4) {
            const void *dataBuffer, *indexBuffer;
            size_t dataSize=0, indexSize=0;
            NEXUS_RecpumpStatus status;

            NEXUS_Recpump_GetStatus(context.hRecpump, &status);

            if(status.started) {
                rc = NEXUS_Recpump_GetDataBuffer(context.hRecpump, &dataBuffer, &dataSize);
                if (rc) {
                    BDBG_ERR(("recpump not internally started"));
                    break;
                }
                rc = NEXUS_Recpump_GetIndexBuffer(context.hRecpump, &indexBuffer, &indexSize);
                BDBG_ASSERT(!rc);
            }
            if (!dataSize && !indexSize) {
                BKNI_WaitForEvent(dataReadyEvent, 250);
                thistime = b_get_time();
                goto check_for_end;
            }
            if (record_gui) {
                static unsigned lasttime = 0;
                if (thistime - lasttime > 200) {
                    brecord_gui_update(record_gui);
                    lasttime = thistime;
                }
            }
            if (dataSize) {
                context.streamTotal += dataSize;
                fwrite(dataBuffer, 1, dataSize, context.pOutputFile);
                rc = NEXUS_Recpump_DataReadComplete(context.hRecpump, dataSize);
                BDBG_ASSERT(!rc);
            }
            if (indexSize) {
                indexSize -= indexSize % sizeof(BSCT_SixWord_Entry);
                if (context.outputindex) {
                    if(context.encoderStartSettings.raiIndex) {
                        indexSize = rai_Indexer_Feed(context.pOutputIndex, indexBuffer,indexSize/sizeof(BSCT_SixWord_Entry), context.indexTotal/sizeof(BSCT_SixWord_Entry))
                            * sizeof(BSCT_SixWord_Entry);
                    } else {
                        indexSize = BNAV_Indexer_Feed(context.bcmindexer, (void*)indexBuffer, indexSize/sizeof(BSCT_SixWord_Entry)) * sizeof(BSCT_SixWord_Entry);
                    }
                }
                context.indexTotal += indexSize;
                rc = NEXUS_Recpump_IndexReadComplete(context.hRecpump, indexSize);
                BDBG_ASSERT(!rc);
            }
            if (context.lastsize + MB < context.streamTotal) {
                context.lastsize = context.streamTotal;
                BDBG_WRN(("%s -> %s: %d MB stream, %d KB index", context.filename, context.outputfile, context.streamTotal/MB, context.indexTotal/1024));

                /* dynamically ramp up bitrate in 4 steps */
                NEXUS_SimpleEncoder_GetSettings(context.hEncoder, &encoderSettings);
                if(context.encoderSettings.rampBitrate && encoderSettings.videoEncoder.bitrateMax < context.encoderSettings.videoBitrate) {/* ramp from quarter dimension and bitrate */
                    encoderSettings.videoEncoder.bitrateMax += context.encoderSettings.videoBitrate/4;
                    encoderSettings.video.width += context.encoderSettings.width/4;
                    encoderSettings.video.height += context.encoderSettings.height/4;
                    if(encoderSettings.videoEncoder.bitrateMax > context.encoderSettings.videoBitrate) {
                        encoderSettings.videoEncoder.bitrateMax = context.encoderSettings.videoBitrate;
                    }
                    if(encoderSettings.video.width > context.encoderSettings.width || encoderSettings.video.height > context.encoderSettings.height) {
                        encoderSettings.video.width  = context.encoderSettings.width;
                        encoderSettings.video.height = context.encoderSettings.height;
                    }
                    rc = NEXUS_SimpleEncoder_SetSettings(context.hEncoder, &encoderSettings);
                    BDBG_ASSERT(!rc);
                    BDBG_WRN(("Video bitrate ramps up -> %u bps; resolution: %ux%u", encoderSettings.videoEncoder.bitrateMax,
                        encoderSettings.video.width, encoderSettings.video.height));
                }
            }

        } else if (context.outputEs){
            /* capture ES outputs */
            encoder_capture(&context);

            /* toggle stop/start every MB video and 100KB audio */
            /* NOTE: stop mode 1 allows audio output to continue in mute while stopping video and audio input */
            if(context.videoSize - context.lastVideoSize > MB) {
                context.lastVideoSize = context.videoSize;
                BDBG_WRN(("%s -> %s: %u MB video ES", context.filename, context.outputfile, (unsigned)context.videoSize/MB));

                /* stop simple encoder/decoder/playback */
                if(context.encoderSettings.stopMode == NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly) {
                    BDBG_WRN(("stopping transcoder"));
                    NEXUS_SimpleEncoder_Stop(context.hEncoder);
                    context.stopped = true;
                }
            }
            if(context.audioSize - context.lastAudioSize > 1024*100) {
                context.lastAudioSize = context.audioSize;
                BDBG_WRN(("%s -> %s: %u KB audio ES", context.filename, context.outputAes, (unsigned)context.audioSize/1024));

                /* restart simple encoder/decoder/playback */
                if(context.stopped && context.encoderSettings.stopMode == NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly) {
                    NEXUS_SimpleEncoder_GetSettings(context.hEncoder, &encoderSettings);
                    if(encoderSettings.video.height == 480) {
                        encoderSettings.video.width = context.encoderSettings.width;
                        encoderSettings.video.height = context.encoderSettings.height;
                    } else {
                        encoderSettings.video.width = 720;
                        encoderSettings.video.height = 480;
                    }
                    BDBG_WRN(("restarting transcoder with resolution %ux%u..", encoderSettings.video.width, encoderSettings.video.height));
                    rc = NEXUS_SimpleEncoder_SetSettings(context.hEncoder, &encoderSettings);
                    BDBG_ASSERT(!rc);
                    rc = NEXUS_SimpleEncoder_Start(context.hEncoder, &context.startSettings);
                    if (rc) {
                        BDBG_ERR(("unable to restart transcoder"));
                        return -1;
                    }
                    context.stopped = false;
                }
            }
        }
        else if (context.outputMp4)
        {
            /*
             * this is to prevent busy loop on NEXUS_Playback_GetStatus
             * which can prevent file module from servicing transaction completion requests
             * - ts path sleeps 250 or until data
             * - es path sleeps 100 if no data
             * - so we sleep 100 here.  BKNI_Sleep(1) also works with no other system loading.
             */
            BKNI_Sleep(100);
        }

check_for_end:
        if (!loop && context.hPlayback) {
            NEXUS_PlaybackStatus status;
            NEXUS_Playback_GetStatus(context.hPlayback, &status);
            if (status.state == NEXUS_PlaybackState_ePaused) {
                if(context.hEncoder && !realtime) {
                    /* unlink video from stream mux output to allow final audio frames to come output of mux in NRT mode. */
                    if(!context.outputEs) {
                        NEXUS_SimpleEncoder_GetSettings(context.hEncoder, &encoderSettings);
                        encoderSettings.stopMode = NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly;
                        NEXUS_SimpleEncoder_SetSettings(context.hEncoder, &encoderSettings);
                        NEXUS_SimpleEncoder_Stop(context.hEncoder);
                    }
                }
                break;
            }
        }
    } while (!timeout || (thistime - starttime)/1000 < timeout || context.stopped);

    /* encoder thread will exit when EOS is received */
    BDBG_WRN(("%s -> %s: stopping", context.filename, context.outputfile));
    stop_encode(&context);
    BDBG_WRN(("%s -> %s: stopped", context.filename, context.outputfile));

    /* Bring down system */
    if (record_gui) {
        brecord_gui_destroy(record_gui);
    }
    NEXUS_SimpleEncoder_Release(context.hEncoder);
    if (crypto) {
        dvr_crypto_destroy(crypto);
    }
    BKNI_DestroyEvent(context.finishEvent);
    BKNI_DestroyEvent(dataReadyEvent);
    if(!context.outputEs && !context.outputMp4) {
        NEXUS_Recpump_Close(context.hRecpump);
    }
    if (context.input_type == input_type_hdmi) {
#if NEXUS_HAS_HDMI_INPUT
        NEXUS_HdmiInput_Close(context.hdmiInput);
#endif
    }
    else if (context.input_type == input_type_graphics) {
        NEXUS_Surface_Destroy(context.imageInput.surface);
    }
    if (context.videoProgram.settings.pidChannel) {
        NEXUS_Playback_ClosePidChannel(context.hPlayback, context.videoProgram.settings.pidChannel);
    }
    if (context.audioProgram.primary.pidChannel) {
        NEXUS_Playback_ClosePidChannel(context.hPlayback, context.audioProgram.primary.pidChannel);
    }
    if (context.hVideoDecoder) {
        NEXUS_SimpleVideoDecoder_Release(context.hVideoDecoder);
    }
    if (context.hAudioDecoder) {
        NEXUS_SimpleAudioDecoder_Release(context.hAudioDecoder);
    }
    if (context.hPlayback) {
        NEXUS_Playback_Destroy(context.hPlayback);
        NEXUS_Playpump_Close(context.hPlaypump);
    }

    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return rc;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs playback, simple_decoder and stream_mux)!\n");
    return 0;
}
#endif
