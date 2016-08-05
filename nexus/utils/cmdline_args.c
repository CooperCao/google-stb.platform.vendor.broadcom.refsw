/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "cmdline_args.h"
#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#include "bmedia_cdxa.h"
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#if B_HAS_AVI
#include "bavi_probe.h"
#endif
#if NEXUS_HAS_VIDEO_ENCODER && NEXUS_HAS_STREAM_MUX
#include "nexus_video_encoder_types.h"
#include "nexus_stream_mux.h"
#endif
#include "bfile_stdio.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

BDBG_MODULE(cmdline_args);

static cmdline_parsefn *moduleparsefn;
static cmdline_usagefn *moduleusagefn;
static void *moduleparam;

#if NEXUS_HAS_PLAYBACK
static void process_stream_string(char *str)
{
    char ch;
    for(ch=*str;ch!='\0';ch=*(++str)) {
        if(ch == ' ' && str[1] == '[' && str[2] == ' ') {
            str[1] = '\n';
            str[2] = '\n';
        } else if(ch == ')' && str[1] == ' ' && str[2] == '0' && str[3] == 'x') {
            str[1] = '\n';
        } else if(ch == ')' && str[1] == ' ' && str[2] == ']' && str[3] == ' '  && str[4] == '[') {
            str[1] = '\n';
            str[2] = ' ';
            str[4] = '\n';
        } else if(ch == ')' && str[1] == ' ' && str[2] == ']' && str[3] == '\0') {
            str[2] = ' ';
        }
    }
    return;
}


static void b_print_media_string(const bmedia_probe_stream *stream)
{
    char stream_info[1024];
    bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
    process_stream_string(stream_info);
    printf( "Media Probe:\n" "%s\n\n", stream_info);
}
#else
#define b_print_media_string(x)
#endif

static void print_usage_common(void)
{
    printf(
        "  -h|--help  - this usage information\n"
        "  -pcr PID   - defaults to video PID\n"
        "  -mpeg_type");
    print_list(g_transportTypeStrs);
    printf("\n  -video PID\n");
    print_list_option("video_type",g_videoCodecStrs);
    print_list_option("audio_type",g_audioCodecStrs);
    printf("  -audio PID\n");
    print_list_option("frame_rate",g_videoFrameRateStrs);
    print_list_option("display_format",g_videoFormatStrs);
#if NEXUS_HAS_DISPLAY
    print_list_option("content_mode",g_contentModeStrs);
    printf(
        "  -afd MODE,CLIP,VAL - MODE={Disable|Stream|User} CLIP={0|1|2}\n"
        );
#endif
    printf(
#if NEXUS_NUM_COMPOSITE_OUTPUTS
        "  -composite {on|off}\n"
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
        "  -component {on|off}\n"
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
        "  -hdmi {on|off}\n"
#endif
        "  -cc {on|off} - enable closed caption routing and output\n"
        "  -probe - use media probe to discover stream format for playback (defaults on if no video or audio pid)\n"
        );
    printf(
        "  -video_cdb KBytes - size of compressed video buffer, in KBytes\n"
        "  -audio_cdb KBytes - size of compressed audio buffer, in KBytes\n"
        "  -avc51 - Enable AVC (H.264) Level 5.1 decoding\n"
#if NEXUS_HAS_PLAYBACK
        "  -max_decoder_rate rate - Set decoder max decoder rate\n"
#endif
        "  -compressed_audio - Also output compressed audio\n"
        "  -ignore_edid      - Send audio/video as requested, without regard for EDID\n"
        );
    printf(
        "  -multichannel_audio - Also output multichannel audio on hdmi\n"
        "  -no_decoded_audio - Don't output decoded (PCM) audio\n"
        "  -no_decoded_video - Don't decode video stream\n"
        "  -video_decoder index - Selects video decoder index\n"
        "  -pcm sr,channels,bits - Play LPCM file\n"
        );
    print_list_option("ts_timestamp",g_tsTimestampType);
#if NEXUS_HAS_VIDEO_DECODER
    print_list_option("video_error_handling", g_videoErrorHandling);
#endif
    printf(
        "  -detect_avc_extension - Detect SVC/MVC extensions\n"
        "  -ext_video PID\n"
        "  -max_playback_data_rate - Set limit for playback data rate (Kbps)\n"
        "  -max_live_data_rate     - Set limit for live data rate (Kbps)\n"
        "  -ext_video_type {svc,mvc}\n"
        );
#if NEXUS_HAS_DISPLAY
    print_list_option("display_ar", g_displayAspectRatioStrs);
    print_list_option("display_3d", g_videoOrientation);
    print_list_option("display_3d_source_buffer", g_videoSourceBuffer);
#endif
#if NEXUS_HAS_VIDEO_DECODER
    print_list_option("source_3d", g_sourceOrientation);
#endif
    printf(
        "  -mem {high|medium|low} - Configure heap memory in an example configuration\n");
    if (moduleusagefn) {
        (*moduleusagefn)(moduleparam);
    }
    return;
}

void print_usage(const char *app)
{
    printf("%s usage:\n", app);
    print_usage_common();
    printf(
        "  -stream_processing - enable extra stream processing for playback\n"
        "  -auto_bitrate - enable bitrate detection for playback\n"
        "  -fixed_bitrate X - provide fixed bitrate to playback (units bits per second)\n"
        "  -graphics    - add a graphics plane\n"
        "  -gfx_bar     - add a graphics bar to test gfx sdr2hdr\n"
        );
    printf(
        "  -astm        - enable Astm (adaptive system time management). Used only for decode app\n"
        "  -loose_audio_tsm - widen decoded audio smooth tracking TSM region\n"
        "  -sync     - enable SyncChannel (lipsync adjusments for post-TSM system delays)\n"
        "  -decoder_reorder - handle timestamp reordering at decoder and disable at the host\n"
        "  -stctrick    - use STC trick modes instead of decoder trick modes\n"
        "  -start_paused - Start playback in a paused state\n"
        "  -max_video WIDTH,HEIGHT - max resolution for video decoder\n"
        );
    printf(
        "  -max_audio_rate RATE - specify the max decoder output/mixing rate\n"
        "  -audio_endian {le|be} - configure the audio decoder for the endianness of the data to expect\n"
        "  -playback_heap n - heap used for playback\n"
        "  -max_audio_rate RATE - specify the max decoder output/mixing rate\n"
        "  -playback_fifo_size KBytes - size of the playback buffer, in KBytes\n"
        );
    print_list_option("master", g_stcChannelMasterStrs);
    print_list_option("bof", g_endOfStreamActionStrs);
    print_list_option("eof", g_endOfStreamActionStrs);
    return;
}

struct cmdline_parse_common_state  {
    bool isVideoEs;
};

static int cmdline_parse_common(int offset, int argc, const char *argv[], struct common_opts_t *opts, struct cmdline_parse_common_state *state, void (*usage)(const char *) )
{
    int i;

    if (offset==0) {
        memset(state,0,sizeof(*state));
        offset = 1;
        state->isVideoEs = true;
        memset(opts,0,sizeof(*opts));
        opts->transportType = NEXUS_TransportType_eTs;
        opts->extVideoCodec = NEXUS_VideoCodec_eNone;
        opts->videoCodec = NEXUS_VideoCodec_eMpeg2;
        opts->audioCodec = NEXUS_AudioCodec_eMpeg;
#if NEXUS_HAS_DISPLAY
        opts->contentMode = NEXUS_VideoWindowContentMode_eBox;
        opts->afd.mode      = NEXUS_AfdMode_eDisabled;
        opts->afd.clip      = NEXUS_AfdClip_eNominal;
        opts->afd.userValue = 0;
        opts->displayAspectRatio = NEXUS_DisplayAspectRatio_eAuto;
        opts->displaySourceBuffer = NEXUS_Display3DSourceBuffer_eDefault;
        opts->displayType = NEXUS_DisplayType_eAuto;
#endif
#if NEXUS_HAS_AUDIO
        opts->compressedAudio = false;
#endif
#if NEXUS_HAS_VIDEO_DECODER
        opts->sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e2D;
#endif
        opts->multichannelAudio = false;
        opts->detectAvcExtension = false;
        opts->decodedAudio = true;
        opts->decodedVideo = true;
        opts->playpumpTimestampReordering = true;
        opts->videoDecoder = 0;
        opts->tsTimestampType = NEXUS_TransportTimestampType_eNone;
        opts->videoFrameRate = NEXUS_VideoFrameRate_eUnknown;
        opts->displayOrientation = NEXUS_VideoOrientation_e2D;
        opts->pcm = false;
#if NEXUS_HAS_PLAYBACK
        bpcm_file_config_init(&opts->pcm_config);
#endif
        opts->masterModeTimingGenerator = false;
        opts->useCcir656Output = false;
        opts->useCompositeOutput = true;
        opts->useComponentOutput = true;
        opts->useHdmiOutput = true;
        opts->displayFormat = NEXUS_VideoFormat_eNtsc;
        opts->maxAudioRate = 48000;
        opts->audioUseLittle = true;
        opts->audioLoudnessMode = NEXUS_AudioLoudnessEquivalenceMode_eNone;
        opts->dolbyDrcModeRf = true;
    }

    for (i=offset;i<argc;i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(argv[0]);
            return -1;
        }
        else if (!strcmp(argv[i], "-pcr") && i+1<argc) {
            opts->pcrPid = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-mpeg_type") && i+1<argc) {
            opts->transportType=lookup(g_transportTypeStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-display_format") && i+1<argc) {
            opts->displayFormat=lookup(g_videoFormatStrs, argv[++i]);
            if (opts->displayFormat >= NEXUS_VideoFormat_e480p) {
                opts->useCcir656Output = false;
                opts->useCompositeOutput = false;
            }
        }
        else if (!strcmp(argv[i], "-master_mode") && i+1<argc) {
            opts->masterModeTimingGenerator = strcasecmp(argv[++i], "off");
        }
#if NEXUS_HAS_DISPLAY
        else if ((!strcmp(argv[i], "-ar") || !strcmp(argv[i], "-content_mode")) && i+1<argc) {
            opts->contentMode = lookup(g_contentModeStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-display_ar") && i+1<argc) {
            opts->displayAspectRatio=lookup(g_displayAspectRatioStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-display_3d") && i+1<argc) {
            opts->displayOrientation=lookup(g_videoOrientation, argv[++i]);
        }
        else if (!strcmp(argv[i], "-display_3d_source_buffer") && i+1<argc) {
            opts->displaySourceBuffer=lookup(g_videoSourceBuffer, argv[++i]);
        }
        else if (!strcmp(argv[i], "-afd") && i+1<argc) {
            unsigned clip, val;
            const char *str = argv[++i];
            if(sscanf(str, "Disable,%u,%u", &clip, &val) == 2) {
                opts->afd.mode = NEXUS_AfdMode_eDisabled;
            }
            else if(sscanf(str, "Stream,%u,%u", &clip, &val) == 2) {
                opts->afd.mode = NEXUS_AfdMode_eStream;
            }
            else if(sscanf(str, "User,%u,%u", &clip, &val) == 2) {
                opts->afd.mode = NEXUS_AfdMode_eUser;
            }
            else {
                continue;
            }
            opts->afd.clip = (clip==0) ? NEXUS_AfdClip_eNominal :
                             (clip==1) ? NEXUS_AfdClip_eOptionalLevel1 :
                                         NEXUS_AfdClip_eOptionalLevel2;
            opts->afd.userValue = val;
        }
#endif
        else if (!strcmp(argv[i], "-video") && i+1<argc) {
            opts->videoPid = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-ext_video") && i+1<argc) {
            opts->extVideoPid = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-audio") && i+1<argc) {
            opts->audioPid = strtoul(argv[++i], NULL, 0);
            state->isVideoEs = false;
        }
        else if (!strcmp(argv[i], "-video_type") && i+1<argc) {
            opts->videoCodec=lookup(g_videoCodecStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-ext_video_type") && i+1<argc) {
            opts->extVideoCodec=lookup(g_videoCodecStrs, argv[++i]);
            switch (opts->extVideoCodec) {
            case NEXUS_VideoCodec_eH264_Mvc:
            case NEXUS_VideoCodec_eH264_Svc:
                break;
            default:
                BDBG_ERR(("invalid -ext_video_type %s", argv[i]));
                return -1;
            }
        }
        else if (!strcmp(argv[i], "-audio_type") && i+1<argc) {
            opts->audioCodec=lookup(g_audioCodecStrs, argv[++i]);
            state->isVideoEs = false;
        }
        /* Note: leave in these options even if the hardware doesn't exist so we don't break any scripts that don't know what is available */
        else if (!strcmp(argv[i], "-ccir656") && i+1<argc) {
            opts->useCcir656Output = strcasecmp(argv[++i], "off");
        }
        else if (!strcmp(argv[i], "-composite") && i+1<argc) {
            opts->useCompositeOutput = strcasecmp(argv[++i], "off");
        }
        else if (!strcmp(argv[i], "-component") && i+1<argc) {
            opts->useComponentOutput = strcasecmp(argv[++i], "off");
        }
        else if (!strcmp(argv[i], "-hdmi") && i+1<argc) {
            opts->useHdmiOutput = strcasecmp(argv[++i], "off");
        }
        else if (!strcmp(argv[i], "-video_cdb") && i+1<argc) {
            opts->videoCdb = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-audio_cdb") && i+1<argc) {
            opts->audioCdb = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-probe")) {
            opts->probe = true;
        }
        else if (!strcmp(argv[i], "-smooth_source_change")) {
            opts->smoothSourceChange = true;
        }
        else if (!strcmp(argv[i], "-compressed_audio")) {
            opts->compressedAudio = true;
        }
        else if (!strcmp(argv[i], "-ignore_edid")) {
            opts->ignore_edid = true;
        }
        else if (!strcmp(argv[i], "-multichannel_audio")) {
            opts->multichannelAudio = true;
        }
        else if (!strcmp(argv[i], "-video_decoder") && i+1<argc) {
            opts->videoDecoder= strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-frame_rate") && i+1<argc) {
            opts->videoFrameRate=lookup(g_videoFrameRateStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-ts_timestamp") && i+1<argc) {
            opts->tsTimestampType=lookup(g_tsTimestampType, argv[++i]);
        }
        else if (!strcmp(argv[i], "-detect_avc_extension")) {
            opts->detectAvcExtension = true;
        }
        else if (!strcmp(argv[i], "-decoder_reorder")) {
            opts->playpumpTimestampReordering = false;
        }
        else if (!strcmp(argv[i], "-no_decoded_audio")) {
            opts->decodedAudio = false;
        }
        else if (!strcmp(argv[i], "-no_decoded_video")) {
            opts->decodedVideo = false;
        }
#if NEXUS_HAS_VIDEO_DECODER
        else if (!strcmp(argv[i], "-source_3d") && i+1<argc) {
            opts->sourceOrientation=lookup(g_sourceOrientation, argv[++i]);
        }
#endif
        else if (!strcmp(argv[i], "-pcm") && i+1<argc) {
            unsigned channel_count = opts->pcm_config.channel_count;
            unsigned sample_size = opts->pcm_config.sample_size;
            unsigned sample_rate = opts->pcm_config.sample_rate;

            sscanf(argv[++i], "%u,%u,%u", &sample_rate, &channel_count, &sample_size);
            opts->pcm_config.channel_count = channel_count;
            opts->pcm_config.sample_size = sample_size;
            opts->pcm_config.sample_rate = sample_rate;
            opts->pcm = true;
        }
        else if (!strcmp(argv[i], "-max_video") && i+1<argc) {
            sscanf(argv[++i], "%u,%u", &opts->maxWidth, &opts->maxHeight);
        }
        else if (!strcmp(argv[i], "-max_audio_rate") && i+1<argc) {
            opts->maxAudioRate = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-audio_endian") && i+1<argc){
            opts->audioUseLittle = strcasecmp(argv[++i], "be");
        }
        else if (!strcmp(argv[i], "-audio_loudness") && i+1<argc){
            opts->audioLoudnessMode = lookup(g_audioLoudnessStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-dolby_drc_mode") && i+1<argc) {
            opts->dolbyDrcModeRf = strcasecmp(argv[++i], "line");
        }
        else {
            return i;
        }
    }

#if NEXUS_HAS_DISPLAY
    opts->displayType = NEXUS_DisplayType_eAuto;
#endif

    /* this allows the user to set: "-mpeg_type es -video_type mpeg" and forget the "-video 1" option */
    if (opts->transportType == NEXUS_TransportType_eEs && !opts->videoPid && !opts->audioPid) {
        if (state->isVideoEs) {
            opts->videoPid = 1;
        }
        else {
            opts->audioPid = 1;
        }
    }

    return 0;
}

void
cmdline_register_module(cmdline_parsefn *parsefn, cmdline_usagefn *usagefn, void *param)
{
    moduleparsefn = parsefn;
    moduleusagefn = usagefn;
    moduleparam = param;
}
int cmdline_parse(int argc, const char *argv[], struct util_opts_t *opts)
{
    struct cmdline_parse_common_state  state;
    int i;
    int rc=-1;
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    memset(opts,0,sizeof(*opts));
    opts->stcChannelMaster = NEXUS_StcChannelAutoModeBehavior_eVideoMaster;
#if NEXUS_HAS_PLAYBACK
    opts->beginningOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
    opts->endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
#endif
#if NEXUS_HAS_VIDEO_DECODER
    opts->videoErrorHandling = NEXUS_VideoDecoderErrorHandling_eNone;
#endif
    opts->customFileIo = false;
    opts->playbackMonitor = false;
    opts->startPaused = false;
    opts->maxPlaybackDataRate = 0;
    opts->maxLiveDataRate = 0;
    opts->playbackHeap = -1;
    opts->playbackFifoSize = 0;

    for (i=0;i<argc;i++)
    {
        rc = cmdline_parse_common(i, argc, argv, &opts->common, &state, print_usage);
        if(rc==0) break;
        if(rc<0) return rc;

        /* unknown option */
        i = rc;
        if (moduleparsefn && (*moduleparsefn)(&i, argv, moduleparam)) {
        }
#if NEXUS_HAS_PLAYBACK
        else if (!strcmp(argv[i], "-bof") && i+1<argc) {
            opts->beginningOfStreamAction=lookup(g_endOfStreamActionStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-eof") && i+1<argc) {
            opts->endOfStreamAction=lookup(g_endOfStreamActionStrs, argv[++i]);
        }
#endif
        else if (!strcmp(argv[i], "-master") && i+1<argc) {
            opts->stcChannelMaster=lookup(g_stcChannelMasterStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-cc") && i+1<argc) {
            opts->closedCaptionEnabled = strcasecmp(argv[++i], "off");
        }
        else if (!strcmp(argv[i], "-stctrick")) {
            opts->stcTrick = true;
        }
        else if (!strcmp(argv[i], "-astm")) {
            opts->astm = true;
        }
        else if (!strcmp(argv[i], "-sync")) {
            opts->sync = true;
        }
        else if (!strcmp(argv[i], "-loose_audio_tsm")) {
            opts->looseAudioTsm = true;
        }
        else if (!strcmp(argv[i], "-custom_file_io")) {
            opts->customFileIo=true;
        }
        else if (!strcmp(argv[i], "-playback_monitor")) {
            opts->playbackMonitor=true;
        }
        else if (!strcmp(argv[i], "-stream_processing")) {
            opts->streamProcessing = true;
        }
        else if (!strcmp(argv[i], "-auto_bitrate")) {
            opts->autoBitrate = true;
        }
        else if (!strcmp(argv[i], "-fixed_bitrate") && i+1<argc) {
            opts->fixedBitrate = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-graphics")) {
            opts->graphics = true;
        }
        else if (!strcmp(argv[i], "-gfx_bar")) {
            opts->gfx_bar = true;
        }
        else if (!strcmp(argv[i], "-avc51")) {
            opts->avc51= true;
        }
        else if (!strcmp(argv[i], "-mem") && i+1<argc) {
            i++;
            if (!strcmp(argv[i], "high")) {
                opts->mem = playback_mem_high;
            }
            else if (!strcmp(argv[i], "medium")) {
                opts->mem = playback_mem_medium;
            }
            else if (!strcmp(argv[i], "low")) {
                opts->mem = playback_mem_low;
            }
            else if (!strcmp(argv[i], "minimal")) {
                opts->mem = playback_mem_minimal;
            }
            else {
                printf("invalid param -mem %s\n", argv[i]);
                return -1;
            }
        }
#if NEXUS_HAS_PLAYBACK
        else if (!strcmp(argv[i], "-max_decoder_rate") && i+1<argc) {
            opts->maxDecoderRate=atof(argv[++i])*NEXUS_NORMAL_PLAY_SPEED;
        }
#endif
#if NEXUS_HAS_VIDEO_DECODER
        else if (!strcmp(argv[i], "-video_error_handling") && i+1<argc) {
            opts->videoErrorHandling=lookup(g_videoErrorHandling, argv[++i]);
        }
#endif
        else if (!strcmp(argv[i], "-start_paused")) {
            opts->startPaused = true;
        }
        else if (!strcmp(argv[i], "-max_playback_data_rate") && i+1<argc) {
            opts->maxPlaybackDataRate=atof(argv[++i])*1000;
        }
        else if (!strcmp(argv[i], "-max_live_data_rate") && i+1<argc) {
            opts->maxLiveDataRate=atof(argv[++i])* 1000;
        }
        else if (!strcmp(argv[i], "-cmp_crc")) {
            opts->cmp_crc = true;
        }
        else if (!strcmp(argv[i], "-avd_crc")) {
            opts->avd_crc = true;
        }
        else if (!strcmp(argv[i], "-hdmi_crc")) {
            opts->hdmi_crc = true;
        }
        else if (!strcmp(argv[i], "-mfd_crc")) {
            opts->mfd_crc = true;
        }
        else if (!strcmp(argv[i], "-script")) {
            opts->scriptRunning = true;
        }
        else if (!strcmp(argv[i], "-timeout") ) {
            opts->timeout = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-playback_heap") ) {
            opts->playbackHeap = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-playback_fifo_size") ) {
            opts->playbackFifoSize = strtoul(argv[++i], NULL, 0);
        }
        else if (!opts->filename) {
            opts->filename = argv[i];
            /* if no filename was specified, playback will not work */
            rc = 0;
        }
        else if (!opts->indexname) {
            opts->indexname = argv[i];
        }
        else {
            printf("unknown parameter: '%s'\n", argv[i]);
            return -1;
        }
    }
    if(rc!=0) {
        cmdline_parse_common(i, argc, argv, &opts->common, &state, print_usage); /* allow common parser to execute finishing touches */
    }

    /* default -probe for playback if no -video or -audio option */
    if (opts->filename && !opts->common.videoPid && !opts->common.audioPid) {
        opts->common.probe = true;
    }
    return 0;
}

void print_usage_record(const char *app)
{
    printf("%s usage: nexus %s [-options] datafile [indexfile]", app, app);
    print_usage_common();
    printf(
        "  -decode{on|off} - A/V decode of source during record");
    printf(
        "\n  -streamer       - (default)"
        "\n  -playfile       - playback file to be used as source");
#if NEXUS_HAS_HDMI_INPUT
    printf("\n  -hdmi_input");
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    printf("\n  -encode              - enable video encoder");
    printf("\n  -encode_video_type");
    print_list(g_videoCodecStrs);
    printf("\n  -video_codec_profile");
    print_list(g_videoCodecProfileStrs);
    printf("\n  -video_codec_level");
    print_list(g_videoCodecLevelStrs);
    printf("\n  -encode_format");
    print_list(g_videoFormatStrs);
    printf("\n  -encode_frame_rate");
    print_list(g_videoFrameRateStrs);
    printf("\n  -video_bit_rate MAX - video encoder max bit rate");
    printf("\n  -gop_frameP num_of_P_between_Is");
    printf("\n  -gop_frameB num_of_B_between_IPs ");
    printf("\n  -mux_latency_tolerance MSEC   - Stream MUX latency tolerance in number of msecs");
    printf("\n  -encode_audio");
    printf("\n  -audio_bit_rate");
    printf("\n  -encode_audio_type");
    print_list(g_audioCodecStrs);
#endif
    printf(
        "\n  -probe          - use media probe to discover stream format for playback"
        "\n  -allpass        - allpass record"
        "\n  -acceptnull     - record null packets in source stream (applies only to allpass record)"
        "\n  -data_buffer_size           - CDB size in bytes"
        "\n  -data_data_ready_threshold  - CDB interrupt threshold in bytes");
    printf(
        "\n  -index_buffer_size          - ITB size in bytes"
        "\n  -index_data_ready_threshold - ITB interrupt threshold in bytes"
        "\n  -video_decoder index - Selects video decoder index"
        "\n  -max_record_data_rate MAX   - Set max record rate"
        "\n");
    return;
}

int cmdline_parse_record(int argc, const char *argv[], struct util_opts_record_t *opts)
{
    int i;
    int rc=-1;
    struct cmdline_parse_common_state  state;

    memset(opts,0,sizeof(*opts));
#if NEXUS_HAS_HDMI_INPUT
    opts->hdmiInput = false;
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    opts->encoder = false;
    opts->encodeFormat = NEXUS_VideoFormat_e480p;
    opts->videoTranscodec = NEXUS_VideoCodec_eH264;
    opts->videoProfile    = NEXUS_VideoCodecProfile_eBaseline;
    opts->videoLevel      = NEXUS_VideoCodecLevel_e31;
    opts->encodeFrameRate = NEXUS_VideoFrameRate_e29_97;
    opts->variableFrameRate = true;
    opts->maxVideoBitRate = 8*1000*1000;
    opts->gopFrameP  = 0;
    opts->gopFrameB  = 0;
    opts->muxLatencyTolerance = 20; /* 20 msec */
    opts->audioEncode = false;
    opts->audioBitRate = 48000;
    opts->audioTranscodec = NEXUS_AudioCodec_eAac;
#endif
    opts->decode = true;

    for (i=0;i<argc;i++)
    {
        rc = cmdline_parse_common(i, argc, argv, &opts->common, &state, print_usage);
        if(rc==0) break;
        if(rc<0) return rc;

        /* unknown option */
        i = rc;
        if (!strcmp(argv[i], "-pids") && i+1<argc) {
            unsigned j;
            char *cur, pid[16+1];
            const char *prev;

            /* parse the comma-separated list of pids */
            j=0;
            prev=argv[i+1];
            while (j<MAX_RECORD_PIDS) {
                if ((cur=strchr(prev,','))) {
                    if ((cur-prev)/sizeof(*cur)>16) { continue; }
                    strncpy(pid,prev,(cur-prev)/sizeof(*cur));
                    opts->otherPids[j] = strtoul(pid, NULL, 0);
                    j++;
                    prev = cur+1;
                }
                else {
                    strncpy(pid,prev,sizeof(pid));
                    opts->otherPids[j] = strtoul(pid, NULL, 0);
                    j++;
                    break;
                }
                pid[0]='\0';
            }
            opts->numOtherPids = j;
            i++;
        }
        else if (!strcmp(argv[i], "-decode") && i+1<argc) {
            opts->decode = strcasecmp(argv[++i], "off");
        }
        else if (!strcmp(argv[i], "-streamer")) {
            opts->streamer = true;
        }
        else if (!strcmp(argv[i], "-playfile")) {
            opts->playfname = argv[++i];
            printf("-playfile %s\n", opts->playfname);
        }
        else if (!strcmp(argv[i], "-max_record_data_rate")) {
            opts->maxRecordDataRate = strtoul(argv[++i], NULL, 0);
        }
        else if (moduleparsefn && (*moduleparsefn)(&i, argv, moduleparam)) {
        }
#if NEXUS_HAS_HDMI_INPUT
        else if (!strcmp(argv[i], "-hdmi_input")) {
            opts->hdmiInput = true;
        }
#endif
#if NEXUS_HAS_VIDEO_ENCODER
        else if (!strcmp(argv[i], "-encode")) {
            opts->encoder = true;
        }
        else if (!strcmp(argv[i], "-encode_audio")) {
            opts->audioEncode = true;
        }
        else if (!strcmp(argv[i], "-audio_bit_rate")) {
            opts->audioBitRate    = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-encode_audio_type")) {
            opts->audioTranscodec = lookup(g_audioCodecStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-encode_format")) {
            opts->encodeFormat = lookup(g_videoFormatStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-encode_video_type")) {
            opts->videoTranscodec = lookup(g_videoCodecStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-video_codec_profile")) {
            opts->videoProfile = lookup(g_videoCodecProfileStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-video_codec_level")) {
            opts->videoLevel = lookup(g_videoCodecLevelStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-encode_frame_rate")) {
            opts->encodeFrameRate = lookup(g_videoFrameRateStrs, argv[++i]);
        }
        else if (!strcmp(argv[i], "-video_bit_rate")) {
            opts->maxVideoBitRate    = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-gop_frameP")) {
            opts->gopFrameP = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-gop_frameB")) {
            opts->gopFrameB = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-mux_latency_tolerance")) {
            opts->muxLatencyTolerance    = strtoul(argv[++i], NULL, 0);
        }
#endif
        else if (!strcmp(argv[i], "-allpass")) {
            opts->allpass = true;
        }
        else if (!strcmp(argv[i], "-acceptnull")) {
            opts->acceptNullPackets = true;
        }
        else if (!strcmp(argv[i], "-data_buffer_size") && i+1<argc) {
            opts->data.bufferSize = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-data_data_ready_threshold") && i+1<argc) {
            opts->data.dataReadyThreshold = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-index_buffer_size") && i+1<argc) {
            opts->index.bufferSize = strtoul(argv[++i], NULL, 0);
        }
        else if (!strcmp(argv[i], "-index_data_ready_threshold") && i+1<argc) {
            opts->index.dataReadyThreshold = strtoul(argv[++i], NULL, 0);
        }
        else if (!opts->recfname) {
            opts->recfname = argv[i];
        }
        else if (!opts->recidxname) {
            opts->recidxname = argv[i];
        }
        else {
            printf("unknown param %s\n", argv[i]);
            return -1;
        }
    }

    if(rc!=0) {
        cmdline_parse_common(i, argc, argv, &opts->common, &state, print_usage); /* allow common parser to execute finishing touches */
    }

    if (!opts->recidxname) { /* if index is specified, require video codec to be explicit. otherwise, default to MPEG2 */
        opts->common.videoCodec = NEXUS_VideoCodec_eMpeg2;
    }
    return 0;
}

int cmdline_probe(struct common_opts_t *opts, const char *filename, const char **indexname)
{
    int rc = 0;

    BSTD_UNUSED(opts);
    BSTD_UNUSED(filename);
    BSTD_UNUSED(indexname);

#if NEXUS_HAS_PLAYBACK
    if (opts->probe) {
        /* use media probe to set values */
        bmedia_probe_t probe = NULL;
        bmedia_probe_config probe_config;
        const bmedia_probe_stream *stream = NULL;
        const bmedia_probe_track *track = NULL;
        bfile_io_read_t fd = NULL;
        bpcm_file_t pcm_file = NULL;
        bool foundAudio = false, foundVideo = false;
        FILE *fin;
        unsigned maxWidth = 0, maxHeight = 0;

        probe = bmedia_probe_create();

        opts->videoCodec = NEXUS_VideoCodec_eUnknown;
        opts->audioCodec = NEXUS_AudioCodec_eUnknown;

        fin = fopen64(filename,"rb");
        if (!fin) {
            printf("can't open media file '%s' for probing\n", filename);
            rc = -1;
            goto done;
        }

        fd = bfile_stdio_read_attach(fin);
        if(opts->pcm) {
            pcm_file = bpcm_file_create(fd, &opts->pcm_config);
            BDBG_ASSERT(pcm_file);
        }

        bmedia_probe_default_cfg(&probe_config);
        probe_config.file_name = filename;
        probe_config.type = bstream_mpeg_type_unknown;
        stream = bmedia_probe_parse(probe, pcm_file?bpcm_file_get_file_interface(pcm_file):fd, &probe_config);

        if(stream && stream->type == bstream_mpeg_type_cdxa) {
            bcdxa_file_t cdxa_file;
            b_print_media_string(stream);
            cdxa_file = bcdxa_file_create(fd);
            if(cdxa_file) {
                const bmedia_probe_stream *cdxa_stream;
                cdxa_stream = bmedia_probe_parse(probe, bcdxa_file_get_file_interface(cdxa_file), &probe_config);
                bcdxa_file_destroy(cdxa_file);
                if(cdxa_stream) {
                    bmedia_probe_stream_free(probe, stream);
                    stream = cdxa_stream;
                    opts->cdxaFile = true;
                }
            }
        }
        if (pcm_file) {
            bpcm_file_destroy(pcm_file);
        }

        /* now stream is either NULL, or stream descriptor with linked list of audio/video tracks */
        bfile_stdio_read_detach(fd);

        fclose(fin);
        if(!stream) {
            printf("media probe can't parse stream '%s'\n", filename);
            rc = -1;
            goto done;
        }

        /* if the user has specified the index, don't override */
        if (indexname && !*indexname) {
            if (stream->index == bmedia_probe_index_available || stream->index == bmedia_probe_index_required) {
                *indexname = filename;
            }
        }

        b_print_media_string(stream);

        opts->transportType = b_mpegtype2nexus(stream->type);

        if (stream->type == bstream_mpeg_type_ts) {
            if ((((bmpeg2ts_probe_stream*)stream)->pkt_len) == 192) {
                if(opts->tsTimestampType == NEXUS_TransportTimestampType_eNone) {
                    opts->tsTimestampType = NEXUS_TransportTimestampType_eMod300;
                }
            }
        }

        for(track=BLST_SQ_FIRST(&stream->tracks);track;track=BLST_SQ_NEXT(track, link)) {
            switch(track->type) {
                case bmedia_track_type_audio:
                    if(track->info.audio.codec != baudio_format_unknown && !foundAudio) {
                        opts->audioPid = track->number;
                        opts->audioCodec = b_audiocodec2nexus(track->info.audio.codec);
                        foundAudio = true;
                        if ( track->info.audio.codec == baudio_format_dts_cd )
                        {
                            opts->dtsCd = true;
                        }
                    }
                    break;
                case bmedia_track_type_video:
                    if(track->info.video.codec == bvideo_codec_h264_svc || track->info.video.codec == bvideo_codec_h264_mvc) {
                        if(opts->detectAvcExtension) {
                            opts->extVideoPid = track->number;
                            opts->extVideoCodec = b_videocodec2nexus(track->info.video.codec);
                        }
                        break;
                    } else if (track->info.video.codec != bvideo_codec_unknown && !foundVideo) {
                        opts->videoPid = track->number;
                        opts->videoCodec = b_videocodec2nexus(track->info.video.codec);
                        foundVideo = true;
#if NEXUS_HAS_VIDEO_DECODER
                        /* timestamp reordering can be done at the host or decoder.
                           to do it at the decoder, disable it at the host and use media_probe to
                           determine the correct decoder timestamp mode */
                        if (opts->playpumpTimestampReordering == false) {
                            opts->decoderTimestampMode = track->info.video.timestamp_order;
                        }
#endif
#if B_HAS_ASF
                        if (stream->type == bstream_mpeg_type_asf) {
                            basf_probe_track *asf_track = (basf_probe_track *)track;
                            if (asf_track->aspectRatioValid) {
                                opts->aspectRatio = NEXUS_AspectRatio_eSar;
                                opts->sampleAspectRatio.x = asf_track->aspectRatio.x;
                                opts->sampleAspectRatio.y = asf_track->aspectRatio.y;
                            }
                            if (asf_track->averageTimePerFrame) {
                                uint64_t framerate = 10*(1000000000/asf_track->averageTimePerFrame);
                                NEXUS_LookupFrameRate((unsigned)framerate, &opts->videoFrameRate);
                            }
                            if(asf_track->dynamicRangeControlValid) {
                                opts->dynamicRangeControlValid = true;
                                opts->dynamicRangeControl.peakReference = asf_track->dynamicRangeControl.peakReference;
                                opts->dynamicRangeControl.peakTarget = asf_track->dynamicRangeControl.peakTarget;
                                opts->dynamicRangeControl.averageReference = asf_track->dynamicRangeControl.averageReference;
                                opts->dynamicRangeControl.averageTarget = asf_track->dynamicRangeControl.averageTarget;
                            }
                        }
#endif
                    }

                    /* SW7445-963: Record stream maxWidth/maxHeight */
                    if(track->info.video.width > maxWidth){
                        maxWidth = track->info.video.width;
                    }
                    if(track->info.video.height > maxHeight){
                        maxHeight = track->info.video.height;
                    }
                    break;
                case bmedia_track_type_pcr:
                    opts->pcrPid = track->number;
                    break;
                default:
                    break;
            }
        }

        /* SW7445-963: Use stream's maxWidth/maxHeight if user
         * didn't specify -max_video to make sure we can decode
         * 4k streams by default */
        if((!opts->maxWidth && !opts->maxHeight) &&
           ((maxWidth > 1920) || (maxHeight > 1088))){
            opts->maxWidth  = maxWidth;
            opts->maxHeight = maxHeight;
        }

#if B_HAS_AVI
        if (stream->type == bstream_mpeg_type_avi && ((bavi_probe_stream *)stream)->video_framerate && opts->videoFrameRate==0) {
            NEXUS_LookupFrameRate(((bavi_probe_stream *)stream)->video_framerate, &opts->videoFrameRate);
        }
#endif

done:
        if (probe) {
            if (stream) {
                bmedia_probe_stream_free(probe, stream);
            }
            bmedia_probe_destroy(probe);
        }
    }
#endif /* NEXUS_HAS_PLAYBACK */

    return rc;
}
/* End of file */
