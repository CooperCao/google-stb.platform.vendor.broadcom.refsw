/******************************************************************************
 *    (c)2010-2014 Broadcom Corporation
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
#if NEXUS_HAS_HDMI_INPUT
#include "nexus_hdmi_input.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include "bstd.h"
#include "bkni.h"

#include "namevalue.h"
#include "nxapps_cmdline.h"

BDBG_MODULE(transcode);

typedef struct EncodeContext
{
    NEXUS_SimpleEncoderHandle hEncoder;
    FILE *pOutputFile;
    FILE *pOutputAes;
    void *pVideoBase;
    void *pAudioBase;
    size_t videoSize, lastVideoSize;
    size_t audioSize, lastAudioSize;
}EncodeContext;

#define IS_START_OF_UNIT(pDesc) (((pDesc)->flags & (NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_FRAME_START|NEXUS_VIDEOENCODERDESCRIPTOR_FLAG_EOS))?true:false)
#define MB (1024*1024)

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
#define DESC_DATA_PTR(ptr0, ptr1, base) ((void *)((uint32_t)ptr0->offset + (uint32_t)base))

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
                    pData = DESC_DATA_PTR(pDesc0, pDesc1, pBufferBase);
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
                    pData = DESC_DATA_PTR(pAudioDesc0, pAudioDesc1, pAudioBase);
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
        "  -video_type    output video codec\n"
        "  -video_bitrate RATE   output video bitrate in Mbps\n"
        "  -video_size    WIDTH,HEIGHT (default is 1280,720)\n"
        "  -window X,Y,WIDTH,HEIGHT    window within video_size (default is full size)\n"
        "  -clip L,R,T,B  source clip size (Left, Right, Top, Bottom; default no clip)\n"
        );
    printf(
        "  -interlaced\n"
        "  -video_framerate HZ    video encode frame rate (for example 29.97, 30, 59.94, 60)\n"
        "  -video_refreshrate HZ  video encode display refresh rate (for example 29.97, 30, 59.94, 60)\n"
        "  -profile       output video codec profile\n"
        "  -level         output video codec level\n"
        );
    printf(
        "  -audio         output audio pid (0 for no audio)\n"
        "  -audio_type    output audio codec\n"
        "  -audio_passthrough    audio codec passthrough\n"
        "  -audio_samplerate     audio output sample rate in HZ\n"
        "  -loop          loop playback\n"
        "  -timeout SECONDS\n"
        "  -ramp_bitrate  to ramp up output bitrate from a quarter of video_bitrate\n"
        );
    printf(
        "  -stopStartVideoEncoder   stop/start video encoder only\n"
        "  -output_es     output elementary stream\n"
        "  -rt            real-time encoding (default is non-real-time)\n"
        "  -tts           record with 4 byte timestamp\n"
        "  -gui off \n"
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
    print_list_option("format",g_videoFormatStrs);
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
        BDBG_MSG(("RAI[%u] byte offset: %llu", raiCnt, bytesRecordedTillCurrentRai));
        /* log the rai info */
        fprintf(fp, "%u,%#x%08x\n", raiCnt++, (uint32_t)highByte, (uint32_t)(bytesRecordedTillCurrentRai));
    }
    return i;
}

int main(int argc, const char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    struct probe_results probe_results;
    unsigned connectId;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleEncoderSettings encoderSettings;
    NEXUS_SimpleAudioDecoderHandle audioDecoder = NULL;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_PlaypumpOpenSettings openSettings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback = NULL;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_SimpleStcChannelSettings stcSettings;
    NEXUS_Error rc = 0;
    int curarg = 1;
    const char *filename = NULL, *outputfile = NULL, *outputindex = NULL, *outputAes = NULL;
    NEXUS_SimpleEncoderHandle encoder;
    NEXUS_SimpleEncoderStartSettings startSettings;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpSettings recpumpSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    BKNI_EventHandle dataReadyEvent;
    FILE *streamOut, *indexOut, *aesOut;
    BNAV_Indexer_Handle bcmindexer;
    unsigned streamTotal = 0, indexTotal = 0;
#define NULL_PID 0x1fff
    unsigned videoPid = NULL_PID;
    unsigned videoCodec = NEXUS_VideoCodec_eNone;
    unsigned videoCodecProfile = NEXUS_VideoCodecProfile_eUnknown;
    unsigned videoCodecLevel   = NEXUS_VideoCodecLevel_eUnknown;
    unsigned videoBitrate = 0;/* bps */
    unsigned width = 0;
    unsigned height = 0;
    unsigned window_x = 0, window_y = 0, window_w = 0, window_h = 0;
    NEXUS_ClipRect clip = {0, 0, 0, 0};
    bool interlaced = false;
    unsigned frameRate = 0, refreshRate=0;
    unsigned audioPid = NULL_PID;
    unsigned audioSampleRate = 0;
    NEXUS_SimpleEncoderStopMode stopMode = NEXUS_SimpleEncoderStopMode_eAll;
    NEXUS_SimpleEncoderStatus encStatus;
    bool outputEs = false;
    bool stopped = false;
    EncodeContext context;
    unsigned audioCodec = NEXUS_AudioCodec_eUnknown;
    bool audPassThrough = false;
    unsigned timeout = 0;
    bool loop = false;
    unsigned starttime, thistime;
    unsigned lastsize = 0;
    bool gui = true;
    bool rampBitrate = false;
    brecord_gui_t record_gui = NULL;
    enum {input_type_decoder, input_type_hdmi, input_type_graphics} input_type = input_type_decoder;
#if NEXUS_HAS_HDMI_INPUT
    NEXUS_HdmiInputHandle hdmiInput = NULL;
#endif
    struct {
        NEXUS_VideoImageInputHandle handle;
        NEXUS_SurfaceHandle surface;
    } imageInput;
    bool realtime = false;
    NEXUS_TransportTimestampType timestampType = NEXUS_TransportTimestampType_eNone;
    bool raiIndex = false;
    struct {
        unsigned pid, remap_pid;
    } passthrough[NEXUS_SIMPLE_ENCODER_NUM_PASSTHROUGH_PIDS];
    unsigned num_passthrough = 0;
    unsigned i;
    struct nxapps_cmdline cmdline;
    int n;
    NEXUS_VideoFormat maxFormat = 0;

    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings);

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage(&cmdline);
            return 0;
        }
        else if (!strcmp(argv[curarg], "-video") && curarg+1 < argc) {
            videoPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-audio") && curarg+1 < argc) {
            audioPid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-audio_samplerate") && curarg+1 < argc) {
            audioSampleRate = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-stopStartVideoEncoder")) {
            stopMode = NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly;
        }
        else if (!strcmp(argv[curarg], "-video_type") && curarg+1 < argc) {
            videoCodec = lookup(g_videoCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-profile") && curarg+1 < argc) {
            videoCodecProfile = lookup(g_videoCodecProfileStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-level") && curarg+1 < argc) {
            videoCodecLevel = lookup(g_videoCodecLevelStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-video_bitrate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            videoBitrate = rate * 1000 * 1000;
        }
        else if (!strcmp(argv[curarg], "-video_size") && curarg+1 < argc) {
            if (sscanf(argv[++curarg], "%u,%u", &width, &height) != 2) {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-window") && curarg+1 < argc) {
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &window_x, &window_y, &window_w, &window_h) != 4) {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-clip") && curarg+1 < argc) {
            if (sscanf(argv[++curarg], "%u,%u,%u,%u", &clip.left, &clip.right, &clip.top, &clip.bottom) != 4) {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-output_es")) {
            outputEs = true;
        }
        else if (!strcmp(argv[curarg], "-interlaced")) {
            interlaced = true;
        }
        else if (!strcmp(argv[curarg], "-video_framerate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            frameRate = rate * 1000;
        }
        else if (!strcmp(argv[curarg], "-video_refreshrate") && curarg+1 < argc) {
            float rate;
            sscanf(argv[++curarg], "%f", &rate);
            refreshRate = rate * 1000;
        }
        else if (!strcmp(argv[curarg], "-audio_type") && curarg+1 < argc) {
            audioCodec = lookup(g_audioCodecStrs, argv[++curarg]);
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
            rampBitrate = true;
        }
        else if (!strcmp(argv[curarg], "-hdmi_input")) {
#if NEXUS_HAS_HDMI_INPUT
            input_type = input_type_hdmi;
            realtime = true;
#else
            BDBG_ERR(("hdmi input not supported"));
#endif
        }
        else if (!strcmp(argv[curarg], "-graphics")) {
            input_type = input_type_graphics;
            realtime = true;
        }
        else if (!strcmp(argv[curarg], "-format")) {
            maxFormat = lookup(g_videoFormatStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-rt")) {
            realtime = true;
        }
        else if (!strcmp(argv[curarg], "-audio_passthrough")) {
            audPassThrough = true;
        }
        else if (!strcmp(argv[curarg], "-tts")) {
            timestampType = NEXUS_TransportTimestampType_eBinary;
        }
        else if (!strcmp(argv[curarg], "-rai_index")) {
            raiIndex = true;
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
            num_passthrough++;
        }
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else if (!filename && input_type == input_type_decoder) {
            filename = argv[curarg];
        }
        else if (!outputfile) {
            outputfile = argv[curarg];
        }
        else if (!outputindex) {
            outputindex = argv[curarg];
        }
        curarg++;
    }

    if (stopMode == NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly && !realtime) {
        BDBG_WRN(("forcing -rt because of -stopStartVideoEncoder"));
        realtime = true;
    }
    if (input_type == input_type_graphics) {
        if (filename) {
            print_usage(&cmdline);
            return -1;
        }
        filename = "graphics";
    }
    else if (input_type == input_type_hdmi) {
        if (filename) {
            print_usage(&cmdline);
            return -1;
        }
        filename = "hdmi";
    }
    if (!filename) {
        print_usage(&cmdline);
        return -1;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s %s", argv[0], filename);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    BDBG_ASSERT(stcChannel);

    if (input_type == input_type_decoder) {
        rc = probe_media(filename, &probe_results);
        if (rc) return BERR_TRACE(rc);

        file = NEXUS_FilePlay_OpenPosix(filename, probe_results.useStreamAsIndex?filename:NULL);
        if (!file) {
            BDBG_ERR(("can't open files: %s %s", filename, filename));
            return -1;
        }

        NEXUS_Playpump_GetDefaultOpenSettings(&openSettings);
        openSettings.fifoSize /= 2;
        playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &openSettings);
        if (!playpump) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}
        playback = NEXUS_Playback_Create();
        BDBG_ASSERT(playback);

        NEXUS_SimpleStcChannel_GetSettings(stcChannel, &stcSettings);
        stcSettings.modeSettings.Auto.transportType = probe_results.transportType;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        rc = NEXUS_SimpleStcChannel_SetSettings(stcChannel, &stcSettings);
        if (rc) {BDBG_WRN(("unable to set stcsettings")); return -1;}

        NEXUS_Playback_GetSettings(playback, &playbackSettings);
        playbackSettings.playpump = playpump;
        playbackSettings.playpumpSettings.transportType = probe_results.transportType;
        playbackSettings.playpumpSettings.timestamp.type = probe_results.timestampType;
        playbackSettings.simpleStcChannel = stcChannel;
        playbackSettings.endOfStreamAction = loop ? NEXUS_PlaybackLoopMode_eLoop : NEXUS_PlaybackLoopMode_ePause;
        NEXUS_Playback_SetSettings(playback, &playbackSettings);
    }
    else {
        memset(&probe_results, 0, sizeof(probe_results));
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = (input_type != input_type_decoder || (probe_results.num_video&&videoPid))?1:0;
    allocSettings.simpleAudioDecoder = (input_type == input_type_hdmi || (probe_results.num_audio&&audioPid))?1:0;
    allocSettings.simpleEncoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) {BDBG_WRN(("unable to alloc transcode resources")); return -1;}

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].windowCapabilities.encoder = true;
    if (input_type == input_type_decoder) {
        if (maxFormat) {
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxFormat = maxFormat;
        }
        else {
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = probe_results.video[0].width;
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = probe_results.video[0].height;
        }
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.supportedCodecs[probe_results.video[0].codec] = true;
    }
    else if (input_type == input_type_hdmi) {
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth =
        connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = 0;
    }
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    connectSettings.simpleEncoder[0].id = allocResults.simpleEncoder[0].id;
    connectSettings.simpleEncoder[0].nonRealTime = !realtime;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) {BDBG_WRN(("unable to connect transcode resources")); return -1;}

    memset(&videoProgram, 0, sizeof(videoProgram));
    memset(&audioProgram, 0, sizeof(audioProgram));

    if (allocResults.simpleVideoDecoder[0].id) {
        videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
        if (!videoDecoder) {
            BDBG_WRN(("video decoder not available"));
        }
        else if (playback) {
            NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            playbackPidSettings.pidTypeSettings.video.codec = probe_results.video[0].codec;
            playbackPidSettings.pidTypeSettings.video.index = true;
            playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
            videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, probe_results.video[0].pid, &playbackPidSettings);
            videoProgram.settings.codec = probe_results.video[0].codec;
            videoProgram.maxWidth = probe_results.video[0].width;
            videoProgram.maxHeight = probe_results.video[0].height;
            if (probe_results.video[0].colorDepth > 8) {
                NEXUS_VideoDecoderSettings settings;
                NEXUS_SimpleVideoDecoder_GetSettings(videoDecoder, &settings);
                settings.colorDepth = probe_results.video[0].colorDepth;
                NEXUS_SimpleVideoDecoder_SetSettings(videoDecoder, &settings);
            }
            BDBG_MSG(("transcode video %#x %s", probe_results.video[0].pid, lookup_name(g_videoCodecStrs, probe_results.video[0].codec)));
        }
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }
    else {
        videoDecoder = NULL;
    }

    if (allocResults.simpleAudioDecoder.id) {
        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
        if (!audioDecoder) {
            BDBG_WRN(("audio decoder not available"));
        }
        else if (playback) {
            NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
            NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
            playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder;
            audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(playback, probe_results.audio[0].pid, &playbackPidSettings);
            audioProgram.primary.codec = probe_results.audio[0].codec;
            BDBG_MSG(("transcode audio %#x %s", probe_results.audio[0].pid, lookup_name(g_audioCodecStrs, probe_results.audio[0].codec)));
        }
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }
    else {
        audioDecoder = NULL;
    }

    BKNI_CreateEvent(&dataReadyEvent);

    encoder = NEXUS_SimpleEncoder_Acquire(allocResults.simpleEncoder[0].id);
    BDBG_ASSERT(encoder);

    if(outputEs) {
        if (!outputfile) {
            static char buf[64], aes[64];
            outputfile = buf;
            outputAes  = aes;
            snprintf(buf, sizeof(buf), "videos/stream.ves");
            snprintf(aes, sizeof(aes), "videos/stream.aes");
            BKNI_Memset(&context, 0, sizeof(context));
        }
    } else {
        NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
        recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
        BDBG_ASSERT(recpump);

        NEXUS_Recpump_GetSettings(recpump, &recpumpSettings);
        recpumpSettings.data.dataReady.callback = complete;
        recpumpSettings.data.dataReady.context = dataReadyEvent;
        recpumpSettings.index.dataReady.callback = complete;
        recpumpSettings.index.dataReady.context = dataReadyEvent;
        recpumpSettings.timestampType = timestampType;
        recpumpSettings.localTimestamp = (timestampType != NEXUS_TransportTimestampType_eNone);
        recpumpSettings.adjustTimestampUsingPcrs = (timestampType != NEXUS_TransportTimestampType_eNone);
        recpumpSettings.dropBtpPackets = (timestampType != NEXUS_TransportTimestampType_eNone);
        recpumpSettings.bandHold = !realtime; /* flow control required for NRT mode */
        rc = NEXUS_Recpump_SetSettings(recpump, &recpumpSettings);
        BDBG_ASSERT(!rc);

        if (!outputfile) {
            static char buf[64];
            NEXUS_RecpumpStatus status;
            NEXUS_Recpump_GetStatus(recpump, &status);
            outputfile = buf;
            snprintf(buf, sizeof(buf), "videos/stream%d.mpg", status.rave.index);
        }
    }

    /* starting encode */
    context.pOutputFile = streamOut = fopen(outputfile, "w+");
    if (!streamOut) {
        BDBG_ERR(("unable to open '%s' for writing", outputfile));
        return -1;
    }
    if (outputindex) {
        indexOut = fopen(outputindex, "w+");
        if (!indexOut) {
            BDBG_ERR(("unable to open '%s' for writing", outputindex));
            return -1;
        }
    }
    if (outputAes) {
        context.pOutputAes = aesOut = fopen(outputAes, "w+");
        if (!aesOut) {
            BDBG_ERR(("unable to open '%s' for writing", outputAes));
            return -1;
        }
    }

    if (gui && !outputEs) {
        struct brecord_gui_settings settings;
        brecord_gui_get_default_settings(&settings);
        settings.sourceName = filename;
        settings.destName = outputfile;
        settings.recpump = recpump;
        settings.color = 0xFF00AAAA;
        record_gui = brecord_gui_create(&settings);
    }

    NEXUS_SimpleEncoder_GetSettings(encoder, &encoderSettings);
    if (videoBitrate) {
        encoderSettings.videoEncoder.bitrateMax = videoBitrate;
    } else {
        videoBitrate = encoderSettings.videoEncoder.bitrateMax;
    }
    if (width || height) {
        encoderSettings.video.width = width;
        encoderSettings.video.height = height;
    } else {/* save default */
        width  = encoderSettings.video.width;
        height = encoderSettings.video.height;
    }
    encoderSettings.video.window.x = window_x;
    encoderSettings.video.window.y = window_y;
    encoderSettings.video.window.width = window_w;
    encoderSettings.video.window.height = window_h;
    encoderSettings.video.clip = clip;

    if(rampBitrate) {/* ramp from quarter dimension and bitrate */
        encoderSettings.videoEncoder.bitrateMax /= 4;
        encoderSettings.video.width /= 2;
        encoderSettings.video.height /= 2;
        BDBG_WRN(("Video bitrate ramps up -> %u bps; resolution: %ux%u", encoderSettings.videoEncoder.bitrateMax,
            encoderSettings.video.width, encoderSettings.video.height));
    }

#if !NEXUS_NUM_DSP_VIDEO_ENCODERS
    /* with 1000/1001 rate tracking by default */
    if (frameRate == 29970 || frameRate == 30000) {
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e30;
        encoderSettings.video.refreshRate = refreshRate? refreshRate : 60000;
    } else if (frameRate == 59940 || frameRate == 60000) {
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e60;
        encoderSettings.video.refreshRate = refreshRate? refreshRate : 60000;
    } else if (frameRate == 20000) {
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e20;
        encoderSettings.video.refreshRate = refreshRate? refreshRate : 60000;
    } else if (frameRate == 23976 || frameRate == 24000) {
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e24;
        encoderSettings.video.refreshRate = refreshRate? refreshRate : 60000;
    } else if (frameRate == 14985 || frameRate == 15000) {
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e14_985;
        encoderSettings.video.refreshRate = refreshRate? refreshRate : 60000;
    } else if (frameRate == 50000) {
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e50;
        encoderSettings.video.refreshRate = refreshRate? refreshRate : 50000;
    } else if (frameRate == 25000) {
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e25;
        encoderSettings.video.refreshRate = refreshRate? refreshRate : 50000;
    } else if (frameRate == 12500) {
        encoderSettings.videoEncoder.frameRate = NEXUS_VideoFrameRate_e12_5;
        encoderSettings.video.refreshRate = refreshRate? refreshRate : 50000;
    }
#endif

    encoderSettings.video.interlaced = interlaced;
    encoderSettings.stopMode = stopMode;
    rc = NEXUS_SimpleEncoder_SetSettings(encoder, &encoderSettings);
    BDBG_ASSERT(!rc);

    NEXUS_SimpleEncoder_GetDefaultStartSettings(&startSettings);
    startSettings.input.video = videoDecoder;
    startSettings.input.audio = audioDecoder;
    startSettings.recpump = outputEs? NULL : recpump;
    startSettings.output.transport.type = outputEs? NEXUS_TransportType_eEs : NEXUS_TransportType_eTs;
    startSettings.output.video.index = !raiIndex;
    startSettings.output.video.raiIndex = raiIndex;
    startSettings.output.video.settings.interlaced = interlaced;
    if (videoCodec) {
        startSettings.output.video.settings.codec = videoCodec;
    }
    if (videoCodecProfile) {
        startSettings.output.video.settings.profile = videoCodecProfile;
    }
    if (videoCodecLevel) {
        startSettings.output.video.settings.level = videoCodecLevel;
    }
    if (videoPid != NULL_PID) {
        startSettings.output.video.pid = videoPid;
    }
    if (audioCodec) {
        startSettings.output.audio.codec = audioCodec;
    }
    if (audioSampleRate != 0) {
        startSettings.output.audio.sampleRate = audioSampleRate;
    }
    if (audPassThrough) {
        startSettings.output.audio.passthrough = true;
        startSettings.output.audio.codec = audioProgram.primary.codec;
        startSettings.output.audio.sampleRate = 0; /* passthrough cannot change audio sample rate */
    }
    if (audioPid != NULL_PID) {
        startSettings.output.audio.pid = audioPid;
    }
    for (i=0;i<num_passthrough;i++) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
        if (passthrough[i].remap_pid) {
            playbackPidSettings.pidSettings.pidSettings.remap.enabled = true;
            playbackPidSettings.pidSettings.pidSettings.remap.pid = passthrough[i].remap_pid;
        }
        startSettings.passthrough[i] = NEXUS_Playback_OpenPidChannel(playback, passthrough[i].pid, &playbackPidSettings);
    }
    rc = NEXUS_SimpleEncoder_Start(encoder, &startSettings);
    if (rc) {
        BDBG_ERR(("unable to start encoder"));
        return -1;
    }

    if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings)) {
        NEXUS_SimpleVideoDecoderPictureQualitySettings settings;
        NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(videoDecoder, &settings);
        nxapps_cmdline_apply_SimpleVideoDecodePictureQualitySettings(&cmdline, &settings);
        NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(videoDecoder, &settings);
    }

    /* can't start decode before encode because encoder provides display for decoder */
    if (videoProgram.settings.pidChannel) {
        rc = NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
        BDBG_ASSERT(!rc);
    }
    if (audioDecoder && audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
        /* decode may fail if audio codec not supported */
    }
#if NEXUS_HAS_HDMI_INPUT
    if (input_type == input_type_hdmi) {
        unsigned index = 0;
        hdmiInput = NEXUS_HdmiInput_Open(index, NULL);
        if (!hdmiInput) {
            BDBG_ERR(("HdmiInput %d not available", index));
            return -1;
        }

        if (videoDecoder) {
            rc = NEXUS_SimpleVideoDecoder_StartHdmiInput(videoDecoder, hdmiInput, NULL);
            BDBG_ASSERT(!rc);
        }
        if (audioDecoder) {
            rc = NEXUS_SimpleAudioDecoder_StartHdmiInput(audioDecoder, hdmiInput, NULL);
            BDBG_ASSERT(!rc);
        }
    }
#endif
    if (input_type == input_type_graphics) {
        NEXUS_SurfaceCreateSettings surfaceCreateSettings;
        NEXUS_SurfaceMemory mem;
        unsigned x,y,i;
        NEXUS_ClientConfiguration clientConfig;
        NEXUS_VideoImageInputStatus imageInputStatus;

        NEXUS_Platform_GetClientConfiguration(&clientConfig);

        imageInput.handle = NEXUS_SimpleVideoDecoder_StartImageInput(videoDecoder, NULL);
        if (!imageInput.handle) {
            BDBG_WRN(("graphics transcode not supported"));
            return -1;
        }

        NEXUS_VideoImageInput_GetStatus(imageInput.handle, &imageInputStatus);

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
        imageInput.surface = NEXUS_Surface_Create(&surfaceCreateSettings);
        NEXUS_Surface_GetMemory(imageInput.surface, &mem);
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
        NEXUS_Surface_Flush(imageInput.surface);
        NEXUS_VideoImageInput_PushSurface(imageInput.handle, imageInput.surface, NULL);
    }

    /* use bcmindexer to convert SCT to NAV */
    if (outputindex) {
        BNAV_Indexer_Settings settings;
        get_default_bcmindexer_settings(startSettings.output.video.settings.codec, indexOut, &settings);
        rc = BNAV_Indexer_Open(&bcmindexer, &settings);
        BDBG_ASSERT(!rc);
    }

    /* recpump must start after decoders start */
    if(!outputEs) {
        rc = NEXUS_Recpump_Start(recpump);
        BDBG_ASSERT(!rc);
    }

    if (playback) {
        rc = NEXUS_Playback_Start(playback, file, NULL);
        BDBG_ASSERT(!rc);
    }

    context.hEncoder = encoder;

    NEXUS_SimpleEncoder_GetStatus(context.hEncoder, &encStatus);
    rc = NEXUS_MemoryBlock_Lock(encStatus.video.bufferBlock, &context.pVideoBase);
    BDBG_ASSERT(!rc);
    rc = NEXUS_MemoryBlock_Lock(encStatus.audio.bufferBlock, &context.pAudioBase);
    BDBG_ASSERT(!rc);

    BDBG_WRN(("%s -> %s: started", filename, outputfile));
    starttime = b_get_time();
    do {
        thistime = b_get_time();
        if(!outputEs) {
            const void *dataBuffer, *indexBuffer;
            unsigned dataSize, indexSize;

            rc = NEXUS_Recpump_GetDataBuffer(recpump, &dataBuffer, &dataSize);
            if (rc) {
                BDBG_ERR(("recpump not internally started"));
                break;
            }
            rc = NEXUS_Recpump_GetIndexBuffer(recpump, &indexBuffer, &indexSize);
            BDBG_ASSERT(!rc);
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
                streamTotal += dataSize;
                fwrite(dataBuffer, 1, dataSize, streamOut);
                rc = NEXUS_Recpump_DataReadComplete(recpump, dataSize);
                BDBG_ASSERT(!rc);
            }
            if (indexSize) {
                indexSize -= indexSize % sizeof(BSCT_SixWord_Entry);
                if (outputindex) {
                    if(raiIndex) {
                        indexSize = rai_Indexer_Feed(indexOut, indexBuffer,indexSize/sizeof(BSCT_SixWord_Entry), indexTotal/sizeof(BSCT_SixWord_Entry))
                            * sizeof(BSCT_SixWord_Entry);
                    } else {
                        indexSize = BNAV_Indexer_Feed(bcmindexer, (void*)indexBuffer, indexSize/sizeof(BSCT_SixWord_Entry)) * sizeof(BSCT_SixWord_Entry);
                    }
                }
                indexTotal += indexSize;
                rc = NEXUS_Recpump_IndexReadComplete(recpump, indexSize);
                BDBG_ASSERT(!rc);
            }
            if (lastsize + MB < streamTotal) {
                lastsize = streamTotal;
                BDBG_WRN(("%s -> %s: %d MB stream, %d KB index", filename, outputfile, streamTotal/MB, indexTotal/1024));

                /* dynamically ramp up bitrate in 4 steps */
                if(rampBitrate && encoderSettings.videoEncoder.bitrateMax < videoBitrate) {/* ramp from quarter dimension and bitrate */
                    NEXUS_SimpleEncoder_GetSettings(encoder, &encoderSettings);
                    encoderSettings.videoEncoder.bitrateMax += videoBitrate/4;
                    encoderSettings.video.width += width/4;
                    encoderSettings.video.height += height/4;
                    if(encoderSettings.videoEncoder.bitrateMax > videoBitrate) {
                        encoderSettings.videoEncoder.bitrateMax = videoBitrate;
                    }
                    if(encoderSettings.video.width > width || encoderSettings.video.height > height) {
                        encoderSettings.video.width  = width;
                        encoderSettings.video.height = height;
                    }
                    rc = NEXUS_SimpleEncoder_SetSettings(encoder, &encoderSettings);
                    BDBG_ASSERT(!rc);
                    BDBG_WRN(("Video bitrate ramps up -> %u bps; resolution: %ux%u", encoderSettings.videoEncoder.bitrateMax,
                        encoderSettings.video.width, encoderSettings.video.height));
                }
            }

        } else {
            /* capture ES outputs */
            encoder_capture(&context);

            /* toggle stop/start every MB video and 100KB audio */
            /* NOTE: stop mode 1 allows audio output to continue in mute while stopping video and audio input */
            if(context.videoSize - context.lastVideoSize > MB) {
                context.lastVideoSize = context.videoSize;
                BDBG_WRN(("%s -> %s: %u MB video ES", filename, outputfile, context.videoSize/MB));

                /* stop simple encoder/decoder/playback */
                if(stopMode == NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly) {
                    BDBG_WRN(("stopping transcoder"));
                    NEXUS_SimpleEncoder_Stop(encoder);
                    stopped = true;
                }
            }
            if(context.audioSize - context.lastAudioSize > 1024*100) {
                context.lastAudioSize = context.audioSize;
                BDBG_WRN(("%s -> %s: %u KB audio ES", filename, outputAes, context.audioSize/1024));

                /* restart simple encoder/decoder/playback */
                if(stopped && stopMode == NEXUS_SimpleEncoderStopMode_eVideoEncoderOnly) {
                    NEXUS_SimpleEncoder_GetSettings(encoder, &encoderSettings);
                    if(encoderSettings.video.height == 480) {
                        encoderSettings.video.width = width;
                        encoderSettings.video.height = height;
                    } else {
                        encoderSettings.video.width = 720;
                        encoderSettings.video.height = 480;
                    }
                    BDBG_WRN(("restarting transcoder with resolution %ux%u..", encoderSettings.video.width, encoderSettings.video.height));
                    rc = NEXUS_SimpleEncoder_SetSettings(encoder, &encoderSettings);
                    BDBG_ASSERT(!rc);
                    rc = NEXUS_SimpleEncoder_Start(encoder, &startSettings);
                    if (rc) {
                        BDBG_ERR(("unable to restart transcoder"));
                        return -1;
                    }
                    stopped = false;
                }
            }
        }

check_for_end:
        if (!loop && playback) {
            NEXUS_PlaybackStatus status;
            NEXUS_Playback_GetStatus(playback, &status);
            if (status.state == NEXUS_PlaybackState_ePaused) break;
        }
    } while (!timeout || (thistime - starttime)/1000 < timeout || stopped);

    /* encoder thread will exit when EOS is received */
    BDBG_WRN(("%s -> %s: stopped", filename, outputfile));

    fclose(streamOut);
    if (outputindex) {
        fclose(indexOut);
        BNAV_Indexer_Close(bcmindexer);
    }
    if (outputAes) {
        fclose(aesOut);
    }

    if(!outputEs) {
        NEXUS_Recpump_Stop(recpump);
    }
    /* resume auto stop mode before stop */
    NEXUS_SimpleEncoder_GetSettings(encoder,&encoderSettings);
    encoderSettings.stopMode = NEXUS_SimpleEncoderStopMode_eAll;
    NEXUS_SimpleEncoder_SetSettings(encoder,&encoderSettings);
    NEXUS_SimpleEncoder_Stop(encoder);

    if (videoDecoder) {
        NEXUS_SimpleVideoDecoder_Stop(videoDecoder);
    }
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_Stop(audioDecoder);
    }
    if (playback) {
        NEXUS_Playback_Stop(playback);
    }

    /* Bring down system */
    if (record_gui) {
        brecord_gui_destroy(record_gui);
    }
    NEXUS_SimpleEncoder_Release(encoder);
    BKNI_DestroyEvent(dataReadyEvent);
    if(!outputEs) {
        NEXUS_Recpump_Close(recpump);
    }
    if (input_type == input_type_hdmi) {
        if (videoDecoder) {
            NEXUS_SimpleVideoDecoder_StopHdmiInput(videoDecoder);
        }
        if (audioDecoder) {
            NEXUS_SimpleAudioDecoder_StopHdmiInput(audioDecoder);
        }
#if NEXUS_HAS_HDMI_INPUT
        NEXUS_HdmiInput_Close(hdmiInput);
#endif
    }
    else if (input_type == input_type_graphics) {
        NEXUS_SimpleVideoDecoder_StopImageInput(videoDecoder);
        NEXUS_Surface_Destroy(imageInput.surface);
    }
    if (videoProgram.settings.pidChannel) {
        NEXUS_Playback_ClosePidChannel(playback, videoProgram.settings.pidChannel);
    }
    if (audioProgram.primary.pidChannel) {
        NEXUS_Playback_ClosePidChannel(playback, audioProgram.primary.pidChannel);
    }
    if (videoDecoder) {
        NEXUS_SimpleVideoDecoder_Release(videoDecoder);
    }
    if (audioDecoder) {
        NEXUS_SimpleAudioDecoder_Release(audioDecoder);
    }
    if (playback) {
        NEXUS_Playback_Destroy(playback);
        NEXUS_Playpump_Close(playpump);
    }

    if (input_type == input_type_decoder) {
        NEXUS_FilePlay_Close(file);
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
