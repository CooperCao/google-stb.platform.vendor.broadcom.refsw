/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include "nexus_platform.h"
#include <stdio.h>

#if !NEXUS_HAS_PLAYBACK
int main(void)
{
    printf("This application is not supported on this platform (needs Playback)!\n");
    return 0;
}
#else

#include "nexus_core_utils.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_trick.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_video_adj.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_display_vbi.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_video_input_crc.h"
#include "nexus_picture_ctrl.h"
#if NEXUS_HAS_AUDIO
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_decoder_trick.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_audio_playback.h"
#include "nexus_spdif_output.h"
#include "nexus_audio_dummy_output.h"
#endif
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#include "nexus_hdmi_output_extra.h"
#endif
#if NEXUS_NUM_656_OUTPUTS
#include "nexus_ccir656_output.h"
#endif
#include "nexus_surface.h"
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif
#if NEXUS_HAS_SYNC_CHANNEL
#include "nexus_sync_channel.h"
#endif
#include "cmdline_args.h"
#include "nexus_platform_memconfig.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "bstd.h"
#include "bkni.h"
#include "fileio_custom.h"
#include "bmedia_cdxa.h"
#include "nexus_file_pvr.h"
#include "decoder_bitrate.h"

#ifndef NEXUS_NUM_AUDIO_DUMMY_OUTPUTS
#define NEXUS_NUM_AUDIO_DUMMY_OUTPUTS 0
#endif

BDBG_MODULE(playback);

static void print_cmp_crc(NEXUS_DisplayHandle display, BKNI_EventHandle endOfStreamEvent);
static void print_avd_crc(NEXUS_VideoDecoderHandle videoDecoder, BKNI_EventHandle endOfStreamEvent);
static void print_hdmi_crc(NEXUS_HdmiOutputHandle display, BKNI_EventHandle endOfStreamEvent);
static void print_mfd_crc(NEXUS_VideoDecoderHandle videoDecoder, BKNI_EventHandle endOfStreamEvent);

static struct util_opts_t opts;

#include "hotplug.c"

/* set a counter and max number to retry resetting the Scramble configuration */
#define HDMI_MAX_SCRAMBLE_RETRY 5

/* this should be enabled with conjunction of enabling  READ_TIMED_DATA in BSEAV/lib/utils/bsink_playback.c */
/* #define READ_TIMED_DATA 1 */


#if READ_TIMED_DATA
#include "nexus_memory.h"
#include "barena.h"
static void *b_nexus_alloc(balloc_iface_t alloc, size_t size)
{
    NEXUS_Error rc;
    void *b;
    BSTD_UNUSED(alloc);
    rc = NEXUS_Memory_Allocate(size, NULL, &b);
    if(rc==NEXUS_SUCCESS) {return b;}
    else {return NULL;}
}

static void b_nexus_free(balloc_iface_t alloc, void *ptr)
{
    BSTD_UNUSED(alloc);
    NEXUS_Memory_Free(ptr);
}

static const struct balloc_iface b_nexus_alloc_iface = {
    b_nexus_alloc, b_nexus_free
};

extern balloc_iface_t bsink_dump_reader_allocator;
#endif /* READ_TIMED_DATA */


static int start_video(const struct util_opts_t *opts, NEXUS_VideoDecoderHandle videoDecoder, const NEXUS_VideoDecoderStartSettings *videoProgram)
{
    NEXUS_Error rc;
    if (opts->common.decodedVideo && opts->common.videoPid) {
        rc = NEXUS_VideoDecoder_Start(videoDecoder, videoProgram);
        if(rc!=NEXUS_SUCCESS) {
            bool supported=true;
            NEXUS_VideoDecoder_IsCodecSupported(videoDecoder, videoProgram->codec, &supported);
            if(!supported) {
                unsigned i;
                bool found_supported=false;
                unsigned num_decoders = NEXUS_NUM_VIDEO_DECODERS;

                BDBG_LOG(("VideoDecoder:%u doesn't support codec %u", opts->common.videoDecoder, videoProgram->codec));
#if NEXUS_NUM_DSP_VIDEO_DECODERS
                num_decoders += NEXUS_NUM_DSP_VIDEO_DECODERS;
#endif

#if NEXUS_NUM_SID_VIDEO_DECODERS
                num_decoders += NEXUS_NUM_SID_VIDEO_DECODERS;
#endif
#if NEXUS_NUM_SOFT_VIDEO_DECODERS
                num_decoders += NEXUS_NUM_SOFT_VIDEO_DECODERS;
#endif

                for(i=0;i<num_decoders;i++) {
                    NEXUS_VideoDecoderHandle decoder;
                    if(i==opts->common.videoDecoder) {
                        continue;
                    }
                    decoder = NEXUS_VideoDecoder_Open(i, NULL);
                    if(decoder) {
                        NEXUS_VideoDecoder_IsCodecSupported(decoder, videoProgram->codec, &supported);
                        if(supported) {
                            if(!found_supported) {
                                found_supported = true;
                                BDBG_LOG(("try to run with the '-video_decoder n' option, where 'n' is the index of the video decoder that supports codec %u", videoProgram->codec));
                            }
                            BDBG_LOG(("VideoDecoder:%u supports codec %u", i, videoProgram->codec));
                        }
                        NEXUS_VideoDecoder_Close(decoder);
                    }
                }
            }
            return -1;
        }
    }
    return 0;
}
static void stop_video(const struct util_opts_t *opts, NEXUS_VideoDecoderHandle videoDecoder)
{
    if (opts->common.videoPid) {
        NEXUS_VideoDecoder_Stop(videoDecoder);
    }
    return;
}

#if NEXUS_HAS_AUDIO
static void start_audio(const struct util_opts_t *opts, NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderHandle compressedDecoder, NEXUS_AudioDecoderStartSettings *audioProgram)
{
    NEXUS_Error rc;
    if (opts->common.audioPid) {
        if (opts->common.decodedAudio) {
            NEXUS_AudioDecoderCodecSettings codecSettings;
            NEXUS_AudioDecoder_GetCodecSettings(audioDecoder, audioProgram->codec, &codecSettings);
            codecSettings.codec = audioProgram->codec;
            switch (audioProgram->codec) {
    #if B_HAS_ASF
            case NEXUS_AudioCodec_eWmaPro:
            /* if DRC for WMA pro is available apply now */
                if (opts->common.dynamicRangeControlValid) {
                    BDBG_WRN(("wma-pro drc enabled,ref peak %d,ref target %d,ave peak %d, ave target %d",
                              opts->common.dynamicRangeControl.peakReference,opts->common.dynamicRangeControl.peakTarget,
                              opts->common.dynamicRangeControl.averageReference,opts->common.dynamicRangeControl.averageTarget));
                    codecSettings.codecSettings.wmaPro.dynamicRangeControlValid = true;
                    codecSettings.codecSettings.wmaPro.dynamicRangeControl.peakReference = opts->common.dynamicRangeControl.peakReference;
                    codecSettings.codecSettings.wmaPro.dynamicRangeControl.peakTarget = opts->common.dynamicRangeControl.peakTarget;
                    codecSettings.codecSettings.wmaPro.dynamicRangeControl.averageReference = opts->common.dynamicRangeControl.averageReference;
                    codecSettings.codecSettings.wmaPro.dynamicRangeControl.averageTarget = opts->common.dynamicRangeControl.averageTarget;
                    NEXUS_AudioDecoder_SetCodecSettings(audioDecoder,&codecSettings);
                }
                break;
    #endif
            case NEXUS_AudioCodec_eAc3:
                if (opts->common.dolbyDrcModeRf) {
                    codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
                    codecSettings.codecSettings.ac3.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eRf;
                }
                else {
                    codecSettings.codecSettings.ac3.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
                    codecSettings.codecSettings.ac3.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eLine;
                }
                break;
            case NEXUS_AudioCodec_eAc3Plus:
                if (opts->common.dolbyDrcModeRf) {
                    codecSettings.codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
                    codecSettings.codecSettings.ac3Plus.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eRf;
                }
                else {
                    codecSettings.codecSettings.ac3Plus.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
                    codecSettings.codecSettings.ac3Plus.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eLine;
                }
                break;
            case NEXUS_AudioCodec_eAc4:
                if (opts->common.dolbyDrcModeRf) {
                    codecSettings.codecSettings.ac4.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eRf;
                    codecSettings.codecSettings.ac4.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eRf;
                }
                else {
                    codecSettings.codecSettings.ac4.drcMode = NEXUS_AudioDecoderDolbyDrcMode_eLine;
                    codecSettings.codecSettings.ac4.drcModeDownmix = NEXUS_AudioDecoderDolbyDrcMode_eLine;
                }
                audioProgram->mixingMode = NEXUS_AudioDecoderMixingMode_eStandalone;
                break;
            case NEXUS_AudioCodec_eAacAdts:
            case NEXUS_AudioCodec_eAacLoas:
                if (opts->common.dolbyDrcModeRf) {
                    codecSettings.codecSettings.aac.drcMode = NEXUS_AudioDecoderDolbyPulseDrcMode_eRf;
                }
                else {
                    codecSettings.codecSettings.aac.drcMode = NEXUS_AudioDecoderDolbyPulseDrcMode_eLine;
                }
                break;
            case NEXUS_AudioCodec_eAacPlusAdts:
            case NEXUS_AudioCodec_eAacPlusLoas:
                if (opts->common.dolbyDrcModeRf) {
                    codecSettings.codecSettings.aacPlus.drcMode = NEXUS_AudioDecoderDolbyPulseDrcMode_eRf;
                }
                else {
                    codecSettings.codecSettings.aacPlus.drcMode = NEXUS_AudioDecoderDolbyPulseDrcMode_eLine;
                }
                break;
            default:
                break;
            }

            NEXUS_AudioDecoder_SetCodecSettings(audioDecoder,&codecSettings);
        }
        if(opts->common.decodedAudio) {
            rc = NEXUS_AudioDecoder_Start(audioDecoder, audioProgram);
            BDBG_ASSERT(!rc);
        }
        if(compressedDecoder) {
            rc = NEXUS_AudioDecoder_Start(compressedDecoder, audioProgram);
            /* This may fail in some cases because we haven't atached any outputs.  Don't assert. */
            /*BDBG_ASSERT(!rc);*/
        }
    }
    return;
}

static void stop_audio(const struct util_opts_t *opts, NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderHandle compressedDecoder)
{
    if (opts->common.audioPid) {
        if(opts->common.decodedAudio) {
            NEXUS_AudioDecoder_Stop(audioDecoder);
        }
        if(compressedDecoder) {
            NEXUS_AudioDecoder_Stop(compressedDecoder);
        }
    }
    return;
}
#else
static void start_audio(const struct util_opts_t *opts, NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderHandle compressedDecoder, NEXUS_AudioDecoderStartSettings *audioProgram)
{
    BSTD_UNUSED(opts);
    BSTD_UNUSED(audioDecoder);
    BSTD_UNUSED(compressedDecoder);
    BSTD_UNUSED(audioProgram);
}
static void stop_audio(const struct util_opts_t *opts, NEXUS_AudioDecoderHandle audioDecoder, NEXUS_AudioDecoderHandle compressedDecoder)
{
    BSTD_UNUSED(opts);
    BSTD_UNUSED(audioDecoder);
    BSTD_UNUSED(compressedDecoder);
}
#endif

#define  B_HAS_PLAYBACK_MONITOR 0
#if B_HAS_PLAYBACK_MONITOR
#include <pthread.h>
#include "bkni_multi.h"

typedef struct PlaybackMonitorState {
    bool terminate;
    pthread_t thread;
    BKNI_MutexHandle lock;
    const struct util_opts_t *opts;
    NEXUS_PlaybackHandle playback;
    NEXUS_VideoDecoderHandle videoDecoder;
    const NEXUS_VideoDecoderStartSettings *videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderHandle compressedDecoder;
    const NEXUS_AudioDecoderStartSettings *audioProgram;
    NEXUS_FilePlayHandle file;
    NEXUS_FilePlayHandle customFile;
    NEXUS_FilePlayHandle stickyFile;
    const NEXUS_PlaybackStartSettings *playbackStartSettings;
} PlaybackMonitorState;

static void *
monitor_thread(void *state_)
{
    const PlaybackMonitorState *state=state_;
    while(!state->terminate) {
        NEXUS_PlaybackStatus status;
        NEXUS_PlaybackSettings playbackSettings;
        NEXUS_VideoDecoderSettings videoDecoderSettings;
        BERR_Code rc;
        bool failed;

        rc = NEXUS_Playback_GetStatus(state->playback, &status);
        BDBG_ASSERT(!rc);
        BKNI_Sleep(1000);
        FileIoSticky_GetFailBit(state->stickyFile, &failed);
        if(!failed) {
            continue;
        }

        BDBG_WRN(("restarting from %u", status.position));
        BKNI_AcquireMutex(state->lock);
        NEXUS_Playback_Stop(state->playback);
        FileIoSticky_ClearFailBit(state->stickyFile);
        if(state->customFile) {
            FileIoCustomProbabilities probabilities;
            FileIoCustom_GetProbabilities(state->customFile, NULL, &probabilities);
            probabilities.error = 0;
            probabilities.nodata = 0;
            probabilities.partial_read = 0;
            FileIoCustom_SetProbabilities(state->customFile, &probabilities, &probabilities);
        }

        if (state->opts->videoPid) {
            /* don't show black frame */
            NEXUS_VideoDecoder_GetSettings(state->videoDecoder, &videoDecoderSettings);
            videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
            rc=NEXUS_VideoDecoder_SetSettings(state->videoDecoder, &videoDecoderSettings);
            BDBG_ASSERT(!rc);
        }
        /* stop decoder */
        stop_video(state->opts, state->videoDecoder);
        stop_audio(state->opts, state->audioDecoder, state->compressedDecoder);

        NEXUS_Playback_GetSettings(state->playback, &playbackSettings);
        playbackSettings.startPaused = true;
        rc = NEXUS_Playback_SetSettings(state->playback, &playbackSettings);
        BDBG_ASSERT(!rc);

        /* Start decoders */
        start_video(state->opts, state->videoDecoder, state->videoProgram);
        start_audio(state->opts, state->audioDecoder, state->compressedDecoder, state->audioProgram);

        /* start playback  */
        rc = NEXUS_Playback_Start(state->playback, state->file, state->playbackStartSettings);
        BDBG_ASSERT(!rc);

        /* seek into desired location */
        rc = NEXUS_Playback_Seek(state->playback, status.position);
        BDBG_ASSERT(!rc);

        /* start playing */
        rc = NEXUS_Playback_Play(state->playback);
        BDBG_ASSERT(!rc);
        BKNI_ReleaseMutex(state->lock);
    }
    return NULL;
}

static void
monitor_thread_start(PlaybackMonitorState *state)
{
    int rc;
    BKNI_CreateMutex(&state->lock);
    state->terminate = false;
    rc = pthread_create(&state->thread, NULL, monitor_thread, state);
    BDBG_ASSERT(rc==0);
    return;
}

static void
monitor_thread_stop(PlaybackMonitorState *state)
{
    state->terminate = true;
    pthread_join(state->thread, NULL);
}
#endif /* B_HAS_PLAYBACK_MONITOR */

static void endOfStreamCallback(void *context, int param)
{
    BKNI_SetEvent((BKNI_EventHandle)context);
    if (param) {
        BDBG_WRN(("end of stream"));
    }
    else {
        BDBG_WRN(("beginning of stream"));
    }
    return;
}

struct CdxaFile {
    struct NEXUS_FilePlay interface; /* must be the first member */
    bcdxa_file_t data;
    bcdxa_file_t index;
    NEXUS_FilePlayHandle parent;
};

static void Cdxa_File_Close(struct NEXUS_FilePlay *file_)
{
    struct CdxaFile *file = (void *)file_;
    bcdxa_file_destroy(file->data);
    if(file->index) {
        bcdxa_file_destroy(file->index);
    }
    file->parent->file.close(file->parent);
    BKNI_Free(file);
    return;
}

static NEXUS_FilePlayHandle Cdxa_File_Attach(NEXUS_FilePlayHandle parent)
{
    struct CdxaFile *file;
    file = BKNI_Malloc(sizeof(*file));
    BDBG_ASSERT(file);
    file->parent = parent;
    file->data = bcdxa_file_create(parent->file.data);
    file->interface.file.close = Cdxa_File_Close;
    BDBG_ASSERT(file->data);
    file->interface.file.data = bcdxa_file_get_file_interface(file->data);
    file->interface.file.index = NULL;
    file->index = NULL;
    if(parent->file.index) {
        file->index = bcdxa_file_create(parent->file.index);
        BDBG_ASSERT(file->index);
        file->interface.file.index = bcdxa_file_get_file_interface(file->index);
    }
    return &file->interface;
}

struct PcmFile {
    struct NEXUS_FilePlay interface; /* must be the first member */
    bpcm_file_t data;
    bpcm_file_t index;
    NEXUS_FilePlayHandle parent;
};

static void Pcm_File_Close(struct NEXUS_FilePlay *file_)
{
    struct PcmFile *file = (void *)file_;
    bpcm_file_destroy(file->data);
    if(file->index) {
        bpcm_file_destroy(file->index);
    }
    file->parent->file.close(file->parent);
    BKNI_Free(file);
    return;
}

static NEXUS_FilePlayHandle Pcm_File_Attach(NEXUS_FilePlayHandle parent, const bpcm_file_config *pcm_config)
{
    struct PcmFile *file;
    file = BKNI_Malloc(sizeof(*file));
    BDBG_ASSERT(file);
    file->parent = parent;
    file->data = bpcm_file_create(parent->file.data, pcm_config);
    BDBG_ASSERT(file->data);
    file->interface.file.data = bpcm_file_get_file_interface(file->data);
    if(parent->file.index) {
        file->index = bpcm_file_create(parent->file.index, pcm_config);
        BDBG_ASSERT(file->index);
        file->interface.file.index= bpcm_file_get_file_interface(file->index);
    } else {
        file->interface.file.index = NULL;
        file->index = NULL;
    }
    file->interface.file.close = Pcm_File_Close;
    return &file->interface;
}

#if NEXUS_NUM_HDMI_OUTPUTS
static void hdmiRxChangedCallback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_HdmiOutputStatus status ;
    BSTD_UNUSED(iParam) ;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status) ;

    /* Some early HDMI 2.0 TVs do not enable deScrambling after being configured to do so */
    /* sometimes the re-write of the Scramble bit will work and sometimes it does not */
    /* Hot Plug or switching inputs and back to HDMI port sometimes helps */
    if (status.txHardwareStatus.scrambling != status.rxHardwareStatus.descrambling)
    {
        /**************************************************/
        /* Possible No Display due to Scrambling Mismatch */
        /**************************************************/

        BDBG_ERR(("Mismatch in Scramble Status for STB/Tx= %s vs. %s/Rx= %s",
            status.txHardwareStatus.scrambling ? "Yes" : "No",
            status.monitorName, status.rxHardwareStatus.descrambling ? "Yes" : "No")) ;

        /* The app could fall to a lower resolution that does not require scrambling */
        /* although this could also be an issue *if* the Rx supports scrambling <= 340 */
        /* it can also be an issue if the content is required to display at 4Kp60 which requires scrambling */
    }
    else
    {
        BDBG_LOG(("HDMI Link Scramble Status: %s",
            status.txHardwareStatus.scrambling ? "***Scrambling***" : "Normal video")) ;
    }
}
#endif

#if NEXUS_HAS_HDMI_OUTPUT
typedef enum HdmiDrmEotfSource
{
    HdmiDrmEotfSource_eInput,
    HdmiDrmEotfSource_eToggle,
    HdmiDrmEotfSource_eUser,
    HdmiDrmEotfSource_eMax
} HdmiDrmEotfSource;

typedef struct HdmiDrmEotfSourceNameValueMapEntry
{
    const char * name;
    HdmiDrmEotfSource value;
} HdmiDrmEotfSourceNameValueMapEntry;

static const HdmiDrmEotfSourceNameValueMapEntry hdmiDrmEotfSourceNameValueMap[]=
{
    { "input", HdmiDrmEotfSource_eInput },
    { "toggle", HdmiDrmEotfSource_eToggle },
    { "user", HdmiDrmEotfSource_eUser },
    {  NULL, HdmiDrmEotfSource_eMax },
};

typedef struct HdmiDrmEotfNameValueMapEntry
{
    const char * name;
    NEXUS_VideoEotf value;
} HdmiDrmEotfNameValueMapEntry;

static const HdmiDrmEotfNameValueMapEntry hdmiDrmEotfNameValueMap[]=
{
    { "SDR", NEXUS_VideoEotf_eSdr },
    { "sdr", NEXUS_VideoEotf_eSdr },
    { "HDR10", NEXUS_VideoEotf_eHdr10 },
    { "hdr10", NEXUS_VideoEotf_eHdr10 },
    { "PQHDR", NEXUS_VideoEotf_eHdr10 },
    { "pqhdr", NEXUS_VideoEotf_eHdr10 },
    { "smpte2084", NEXUS_VideoEotf_eHdr10 },
    { "smpte", NEXUS_VideoEotf_eHdr10 },
    { "2084", NEXUS_VideoEotf_eHdr10 },
    { "HLG", NEXUS_VideoEotf_eHlg },
    { "hlg", NEXUS_VideoEotf_eHlg },
    { "aribb67", NEXUS_VideoEotf_eHlg },
    { "arib", NEXUS_VideoEotf_eHlg },
    { "b67", NEXUS_VideoEotf_eHlg },
    { NULL, NEXUS_VideoEotf_eMax }
};

static const char * unknownString = "unknown";

const char * hdmi_drm_get_eotf_source_name(HdmiDrmEotfSource value)
{
    const char * name = unknownString;
    const HdmiDrmEotfSourceNameValueMapEntry * e = NULL;

    for (e = &hdmiDrmEotfSourceNameValueMap[0]; e->name; e++)
    {
        if (e->value == value)
        {
            name = e->name;
            break;
        }
    }

    return name;
}

HdmiDrmEotfSource hdmi_drm_get_eotf_source_value(const char * name)
{
    HdmiDrmEotfSource value = HdmiDrmEotfSource_eMax;
    const HdmiDrmEotfSourceNameValueMapEntry * e = NULL;

    for (e = &hdmiDrmEotfSourceNameValueMap[0]; e->name; e++)
    {
        if (!strcmp(e->name, name))
        {
            value = e->value;
            break;
        }
    }

    return value;
}

const char * hdmi_drm_get_eotf_name(NEXUS_VideoEotf value)
{
    const char * name = unknownString;
    const HdmiDrmEotfNameValueMapEntry * e = NULL;

    for (e = &hdmiDrmEotfNameValueMap[0]; e->name; e++)
    {
        if (e->value == value)
        {
            name = e->name;
            break;
        }
    }

    return name;
}

NEXUS_VideoEotf hdmi_drm_get_eotf_value(const char * name)
{
    NEXUS_VideoEotf value = NEXUS_VideoEotf_eMax;
    const HdmiDrmEotfNameValueMapEntry * e = NULL;

    for (e = &hdmiDrmEotfNameValueMap[0]; e->name; e++)
    {
        if (!strcmp(e->name, name))
        {
            value = e->value;
            break;
        }
    }

    return value;
}

typedef struct HdmiDrmContext
{
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_HdmiOutputHandle hdmiOutput;
    struct
    {
        HdmiDrmEotfSource source;
        NEXUS_VideoEotf user;
    } eotf;
    struct
    {
        NEXUS_HdmiDynamicRangeMasteringInfoFrame input;
        NEXUS_HdmiDynamicRangeMasteringInfoFrame output;
    } infoFrame;
} HdmiDrmContext;

static HdmiDrmContext hdmiDrmContext;

void hdmi_drm_init_context(HdmiDrmContext * pHdmiDrmContext, NEXUS_VideoDecoderHandle videoDecoder, NEXUS_HdmiOutputHandle hdmiOutput)
{
    BKNI_Memset(pHdmiDrmContext, 0, sizeof(*pHdmiDrmContext));
    pHdmiDrmContext->videoDecoder = videoDecoder;
    pHdmiDrmContext->hdmiOutput = hdmiOutput;
    pHdmiDrmContext->eotf.source = HdmiDrmEotfSource_eInput;
    pHdmiDrmContext->eotf.user = NEXUS_VideoEotf_eSdr;
    pHdmiDrmContext->infoFrame.input.eotf = NEXUS_VideoEotf_eMax;
    pHdmiDrmContext->infoFrame.output.eotf = NEXUS_VideoEotf_eSdr;
}

void hdmi_drm_apply_settings(const HdmiDrmContext * pDrmContext)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputExtraSettings hdmiSettings;
    NEXUS_HdmiOutput_GetExtraSettings(pDrmContext->hdmiOutput, &hdmiSettings);
    hdmiSettings.overrideDynamicRangeMasteringInfoFrame = (pDrmContext->eotf.source != HdmiDrmEotfSource_eInput);
    if (pDrmContext->eotf.source != HdmiDrmEotfSource_eInput)
    {
        BKNI_Memcpy(&hdmiSettings.dynamicRangeMasteringInfoFrame, &pDrmContext->infoFrame.output, sizeof(hdmiSettings.dynamicRangeMasteringInfoFrame));
    }
    rc = NEXUS_HdmiOutput_SetExtraSettings(pDrmContext->hdmiOutput, &hdmiSettings);
    if (rc) { BERR_TRACE(rc); }
}

void hdmi_drm_video_stream_changed(void * ctx, int param)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    HdmiDrmContext * hdc = ctx;
    NEXUS_VideoDecoderStreamInformation streamInfo;
    bool apply = false;

    BSTD_UNUSED(param);

    rc = NEXUS_VideoDecoder_GetStreamInformation(hdc->videoDecoder, &streamInfo);
    if (rc) { BERR_TRACE(rc); goto end; }

    /* copy from stream */
    hdc->infoFrame.input.eotf = streamInfo.eotf;
    hdc->infoFrame.input.metadata.type = NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1;
    BKNI_Memcpy(&hdc->infoFrame.input.metadata.typeSettings.type1.masteringDisplayColorVolume, &streamInfo.masteringDisplayColorVolume, sizeof(hdc->infoFrame.input.metadata.typeSettings.type1.masteringDisplayColorVolume));
    BKNI_Memcpy(&hdc->infoFrame.input.metadata.typeSettings.type1.contentLightLevel, &streamInfo.contentLightLevel, sizeof(hdc->infoFrame.input.metadata.typeSettings.type1.contentLightLevel));

    /* no input override for metadata yet */
    if (BKNI_Memcmp(&hdc->infoFrame.output.metadata, &hdc->infoFrame.input.metadata, sizeof(hdc->infoFrame.output.metadata)))
    {
        BKNI_Memcpy(&hdc->infoFrame.output.metadata, &hdc->infoFrame.input.metadata, sizeof(hdc->infoFrame.output.metadata));
        apply = true;
    }

    if (apply)
    {
        hdmi_drm_apply_settings(hdc);
    }

end:
    return;
}

void hdmi_drm_set_eotf_source(HdmiDrmContext * pHdmiDrmContext, HdmiDrmEotfSource source)
{
    if ((pHdmiDrmContext->eotf.source != HdmiDrmEotfSource_eInput) ||
        (source != HdmiDrmEotfSource_eInput))
    {
        switch (source)
        {
            case HdmiDrmEotfSource_eInput:
                pHdmiDrmContext->infoFrame.output.eotf = pHdmiDrmContext->infoFrame.input.eotf;
                break;
            case HdmiDrmEotfSource_eUser:
                pHdmiDrmContext->infoFrame.output.eotf = pHdmiDrmContext->eotf.user;
                break;
            default:
            case HdmiDrmEotfSource_eToggle:
                break;
        }
        pHdmiDrmContext->eotf.source = source;
        hdmi_drm_apply_settings(pHdmiDrmContext);
    }
}

void hdmi_drm_do_toggle(HdmiDrmContext * pHdmiDrmContext)
{
    unsigned i = 0;
    char buf[2];
    NEXUS_VideoEotf toggler[] =
    {
        NEXUS_VideoEotf_eSdr,
        NEXUS_VideoEotf_eHdr10
    };
    unsigned toggles = sizeof(toggler) / sizeof(toggler[0]);

    /* skip the first entry if we are already there */
    if (pHdmiDrmContext->infoFrame.output.eotf == toggler[0])
    {
        i = (i + 1) % toggles;
    }
    /* toggle! */
    while (1)
    {
        BKNI_Sleep(500); /* give console time to settle */
        printf("Press enter to switch to %s ('q' to quit toggling)\n", hdmi_drm_get_eotf_name(toggler[i]));
        fgets(buf, 2, stdin);
        if (buf[0] == 'q') break;
        pHdmiDrmContext->infoFrame.output.eotf = toggler[i];
        i = (i + 1) % toggles;
        hdmi_drm_apply_settings(pHdmiDrmContext);
    }
}

void hdmi_drm_do_status(const HdmiDrmContext * pHdmiDrmContext)
{
    printf("eotf status:\n");
    printf("\tsource: %s\n", hdmi_drm_get_eotf_source_name(pHdmiDrmContext->eotf.source));
    printf("\toutput: %s\n", hdmi_drm_get_eotf_name(pHdmiDrmContext->infoFrame.output.eotf));
    printf("\tinput: %s\n", hdmi_drm_get_eotf_name(pHdmiDrmContext->infoFrame.input.eotf));
    printf("\tuser: %s\n", hdmi_drm_get_eotf_name(pHdmiDrmContext->eotf.user));
}

void hdmi_drm_do_cmd(HdmiDrmContext * pHdmiDrmContext, const char * buf)
{
    if (!strcmp(buf, "hdmi_eotf(input)")) {
        hdmi_drm_set_eotf_source(pHdmiDrmContext, HdmiDrmEotfSource_eInput);
    }
    else if (!strcmp(buf, "hdmi_eotf(sdr)")) {
        pHdmiDrmContext->eotf.user = NEXUS_VideoEotf_eSdr;
        hdmi_drm_set_eotf_source(pHdmiDrmContext, HdmiDrmEotfSource_eUser);
    }
    else if (!strcmp(buf, "hdmi_eotf(hdr10)")) {
        pHdmiDrmContext->eotf.user = NEXUS_VideoEotf_eHdr10;
        hdmi_drm_set_eotf_source(pHdmiDrmContext, HdmiDrmEotfSource_eUser);
    }
    else if (!strcmp(buf, "hdmi_eotf(hlg)")) {
        pHdmiDrmContext->eotf.user = NEXUS_VideoEotf_eHlg;
        hdmi_drm_set_eotf_source(pHdmiDrmContext, HdmiDrmEotfSource_eUser);
    }
    else if (!strcmp(buf, "hdmi_eotf(toggle)")) {
        HdmiDrmEotfSource oldSource = pHdmiDrmContext->eotf.source;
        hdmi_drm_set_eotf_source(pHdmiDrmContext, HdmiDrmEotfSource_eToggle);
        hdmi_drm_do_toggle(pHdmiDrmContext);
        hdmi_drm_set_eotf_source(pHdmiDrmContext, oldSource);
    }
    else if (!strcmp(buf, "hdmi_eotf()") || !strcmp(buf, "hdmi_eotf")) {
        hdmi_drm_do_status(pHdmiDrmContext);
    }
    else {
        printf("Unknown eotf; No Change\n");
    }
}
#endif


#if BRDC_USE_CAPTURE_BUFFER
#include "bdbg_fifo.h"
static void read_rul_capture_fifo(void) {
    NEXUS_PlatformStatus status;
    void *buffer;
    BDBG_FifoReader_Handle fifoReader;
    int rc;

    rc = NEXUS_Platform_GetStatus(&status);
    BDBG_ASSERT(!rc);
    if (!status.displayModuleStatus.rulCapture.memory) {
        BDBG_ERR(("missing rul capture memory"));
        return;
    }
    NEXUS_MemoryBlock_Lock(status.displayModuleStatus.rulCapture.memory, &buffer);
    rc = BDBG_FifoReader_Create(&fifoReader, buffer);
    BDBG_ASSERT(!rc);
    while (1) {
#define BUFSIZE 4096
        char buf[BUFSIZE];
        int rc;

        /* simplistic state machine. you may want something more sophisticated. */
        rc = BDBG_FifoReader_Read(fifoReader, buf, BUFSIZE);
        switch(rc) {
        case BERR_SUCCESS:
            BDBG_WRN(("rul capture: got %d bytes", BUFSIZE));
            break;
        case BERR_FIFO_NO_DATA:
        case BERR_FIFO_BUSY:
            BKNI_Sleep(1);
            break;
        case BERR_FIFO_OVERFLOW:
            BERR_TRACE(rc);
            break;
        default:
            BERR_TRACE(rc);
            break;
        }
    }
    BDBG_FifoReader_Destroy(fifoReader);
}
#endif

int main(int argc, const char *argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel = NULL, audioPidChannel = NULL, pcrPidChannel, videoExtPidChannel=NULL;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayVbiSettings displayVbiSettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_DisplayHandle displaySD = NULL;
    NEXUS_VideoWindowHandle windowSD = NULL;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_VideoWindowAfdSettings windowAfdSettings;
    NEXUS_VideoWindowScalerSettings scalerSettings;
    NEXUS_VideoFormatInfo videoFormatInfo;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder=NULL;
    NEXUS_AudioDecoderHandle compressedDecoder=NULL;
    NEXUS_AudioDecoderStartSettings audioProgram;
    NEXUS_FilePlayHandle file,customFile=NULL, stickyFile=NULL, cdxaFile=NULL, pcmFile=NULL;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_PlaybackStartSettings playbackStartSettings;
    NEXUS_VideoDecoderOpenSettings openSettings;
    NEXUS_Error rc;
    bool exit;
    NEXUS_SurfaceHandle framebuffer = NULL;
#if NEXUS_HAS_AUDIO
    NEXUS_AudioDecoderSettings audioDecoderSettings;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
#endif
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannelHandle sync;
    NEXUS_SyncChannelSettings syncSettings;
#endif
    NEXUS_Rect full,scaled;
    int percent = 20;
    struct decoder_bitrate video_bitrate;
    struct decoder_bitrate audio_bitrate;
    NEXUS_PlaypumpOpenSettings playpumpSettings;
    BKNI_EventHandle endOfStreamEvent;
    bool echo = false;
    NEXUS_PlatformCapabilities platformCap;
    #if NEXUS_HAS_AUDIO
    NEXUS_AudioCapabilities audioCaps;
    bool allowStereo = true;
    #endif
#if NEXUS_NUM_HDMI_OUTPUTS
    struct hotplug_context hotplug_context;
#endif

    if (cmdline_parse(argc, argv, &opts)) {
        return 0;
    }
    /* They MUST include a file to play */
    if (!opts.filename) {
        BDBG_ERR(("Missing playback file; See usage."));
        print_usage(argv[0]);
        return 1;
    }

    NEXUS_GetPlatformCapabilities(&platformCap);
    if (!platformCap.display[0].supported || platformCap.display[0].encoder) {
        BDBG_ERR(("headless systems not supported"));
        return 0; /* if whole app is not supported, return success */
    }

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    if(opts.maxPlaybackDataRate) {
        unsigned i;
        for(i=0;i<NEXUS_MAX_PLAYPUMPS;i++) {
            platformSettings.transportModuleSettings.maxDataRate.playback[i] = opts.maxPlaybackDataRate;
        }
    }
#if NEXUS_HAS_AUDIO && NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    if ( opts.common.maxAudioRate > 48000 )
    {
        platformSettings.audioModuleSettings.maxPcmSampleRate = opts.common.maxAudioRate;
    }
    else
    {
        platformSettings.audioModuleSettings.maxPcmSampleRate = 48000;
    }
    platformSettings.audioModuleSettings.loudnessMode = opts.common.audioLoudnessMode;
#endif
    if (opts.avc51 || opts.mem) {
        NEXUS_MemoryConfigurationSettings memConfigSettings;
        unsigned i, j;

        NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);
        memConfigSettings.videoDecoder[0].avc51Supported = opts.avc51;

        switch (opts.mem) {
        case playback_mem_high:
            /* defaults */
            break;

        case playback_mem_medium:
            for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
                for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
                    memConfigSettings.display[i].window[j].used = i<2 || (i==2 && j==0); /* dual display w/ PIP + 1 transcode display */
                }
            }
            for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
                memConfigSettings.videoDecoder[i].used = i<2; /* dual decode */
            }
#if NEXUS_HAS_VIDEO_ENCODER
            for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
                memConfigSettings.videoEncoder[i].used = i<1; /* single transcode */
            }
#endif
            break;

        case playback_mem_minimal:
        case playback_mem_low:
            for (i=0;i<NEXUS_MAX_DISPLAYS;i++) {
                for (j=0;j<NEXUS_NUM_VIDEO_WINDOWS;j++) {
                    memConfigSettings.display[i].window[j].used = i==0 && j==0; /* single display, single window */
                    if (memConfigSettings.display[i].window[j].used) {
                        memConfigSettings.display[i].maxFormat = NEXUS_VideoFormat_e1080i;
                        /* leave deinterlacer and capture at default values */
                        memConfigSettings.display[i].window[j].convertAnyFrameRate = false;
                        memConfigSettings.display[i].window[j].precisionLipSync = false;
                        if (opts.mem == playback_mem_minimal) {
                            /* minimal requires progressive sources, full screen only. */
                            memConfigSettings.display[i].window[j].capture = false;
                            memConfigSettings.display[i].window[j].deinterlacer = false;
                        }
                    }
                }
            }
            for (i=0;i<NEXUS_MAX_VIDEO_DECODERS;i++) {
                memConfigSettings.videoDecoder[i].used = i<1; /* single decode */
                memConfigSettings.videoDecoder[i].maxFormat = NEXUS_VideoFormat_e1080i;
                memset(memConfigSettings.videoDecoder[i].supportedCodecs, 0, sizeof(memConfigSettings.videoDecoder[i].supportedCodecs));
                memConfigSettings.videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eMpeg2] = true;
                memConfigSettings.videoDecoder[i].supportedCodecs[NEXUS_VideoCodec_eH264] = true;
            }
#if NEXUS_HAS_VIDEO_ENCODER
            for (i=0;i<NEXUS_MAX_VIDEO_ENCODERS;i++) {
                memConfigSettings.videoEncoder[i].used = false; /* no transcode */
            }
#endif
            opts.common.useCcir656Output = false;
            opts.common.useCompositeOutput = false;
            break;
        default:
            break;
        }
        rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);
        BDBG_ASSERT(!rc);
    }
    else {
        rc = NEXUS_Platform_Init(&platformSettings);
        BDBG_ASSERT(!rc);
    }
    NEXUS_Platform_GetConfiguration(&platformConfig);
    BKNI_CreateEvent(&endOfStreamEvent);

    decoder_bitrate_init(&video_bitrate);
    decoder_bitrate_init(&audio_bitrate);

    if (cmdline_probe(&opts.common, opts.filename, &opts.indexname)) {
        BERR_TRACE(NEXUS_UNKNOWN);
        NEXUS_Platform_Uninit();
        return -1;
    }

    if ((opts.common.transportType == NEXUS_TransportType_eWav ||
         opts.common.transportType == NEXUS_TransportType_eEs) &&
        opts.common.audioCodec == NEXUS_AudioCodec_eDtsLegacy)
    {
        opts.common.transportType = NEXUS_TransportType_eEs;
        opts.common.dtsCd = true;
    }

    if (opts.common.transportType == NEXUS_TransportType_eMpeg2Pes &&
        opts.common.audioCodec == NEXUS_AudioCodec_eDtsLegacy)
    {
        opts.common.dtsCd = true;
    }

    /* for MVC-codec, default to likely source orientation */
    if ((opts.common.extVideoCodec == NEXUS_VideoCodec_eH264_Mvc) && (opts.common.sourceOrientation==NEXUS_VideoDecoderSourceOrientation_e2D))
    {
        opts.common.sourceOrientation = NEXUS_VideoDecoderSourceOrientation_e3D_LeftRightFullFrame;
    }

    if ((opts.indexname && !strcmp(opts.indexname, "same")) ||
        opts.common.transportType == NEXUS_TransportType_eMkv ||
        opts.common.transportType == NEXUS_TransportType_eMp4
        )
    {
        opts.indexname = opts.filename;
    }
    if(opts.indexname && strcmp(opts.indexname, "/dev/null")==0) {
        opts.indexname = NULL;
    }

    file = NEXUS_FilePlay_OpenPosix(opts.filename, opts.indexname);
    if (!file) {
        BDBG_ERR(("can't open files: '%s' '%s'", opts.filename, opts.indexname));
        NEXUS_Platform_Uninit();
        return -1;
    }

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpSettings);
    if(opts.playbackHeap >=0 ) {
        NEXUS_MemoryStatus heapStatus;

        if( (unsigned)opts.playbackHeap >= sizeof(platformConfig.heap)/sizeof(*platformConfig.heap) || platformConfig.heap[opts.playbackHeap]==NULL) {
            BDBG_ERR(("Invalid playbackHeap:%d", opts.playbackHeap));
            return -1;
        }
        playpumpSettings.heap = platformConfig.heap[opts.playbackHeap];
        NEXUS_Heap_GetStatus(platformConfig.heap[opts.playbackHeap], &heapStatus);
        playpumpSettings.dataNotCpuAccessible = (heapStatus.memoryType & NEXUS_MEMORY_TYPE_DRIVER_CACHED)==0;
        if(0) {
            playpumpSettings.memory = NEXUS_MemoryBlock_Allocate(playpumpSettings.heap, playpumpSettings.fifoSize, 0, NULL);
        }
    }
    if(opts.playbackFifoSize>0) {
        playpumpSettings.fifoSize = 1024*opts.playbackFifoSize;
    }
    playpump = NEXUS_Playpump_Open(0, &playpumpSettings);
    BDBG_ASSERT(playpump);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);

    if(opts.customFileIo) {
        customFile = file = FileIoCustom_Attach(file);
        BDBG_ASSERT(file);
    }
    if(opts.common.cdxaFile) {
        cdxaFile = file = Cdxa_File_Attach(file);
        BDBG_ASSERT(cdxaFile);
    }
    if(opts.common.pcm) {
        pcmFile = file = Pcm_File_Attach(file, &opts.common.pcm_config);
        BDBG_ASSERT(pcmFile);
    }
    if(opts.playbackMonitor) {
        stickyFile = file = FileIoSticky_Attach(file);
        BDBG_ASSERT(file);
    }

    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.modeSettings.Auto.behavior = opts.stcChannelMaster;
    stcSettings.modeSettings.Auto.transportType = opts.common.transportType;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_SyncChannel_GetDefaultSettings(&syncSettings);
    sync = NEXUS_SyncChannel_Create(&syncSettings);
#endif

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = opts.common.transportType;
    playbackSettings.playpumpSettings.timestamp.pacing = false;
    playbackSettings.playpumpSettings.timestamp.type = opts.common.tsTimestampType;
    if(opts.maxPlaybackDataRate) {
        playbackSettings.playpumpSettings.maxDataRate = opts.maxPlaybackDataRate;
    }
    playbackSettings.startPaused = opts.startPaused;
    playbackSettings.stcChannel = stcChannel;
    playbackSettings.stcTrick = opts.stcTrick;
    playbackSettings.beginningOfStreamAction = opts.beginningOfStreamAction;
    playbackSettings.beginningOfStreamCallback.callback = endOfStreamCallback;
    playbackSettings.beginningOfStreamCallback.context = endOfStreamEvent;
    playbackSettings.beginningOfStreamCallback.param = 0;
    playbackSettings.endOfStreamAction = opts.endOfStreamAction;
    playbackSettings.endOfStreamCallback.callback = endOfStreamCallback;
    playbackSettings.endOfStreamCallback.context = endOfStreamEvent;
    playbackSettings.endOfStreamCallback.param = 1;
    playbackSettings.enableStreamProcessing = opts.streamProcessing;
    playbackSettings.accurateSeek = opts.accurateSeek;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    #if NEXUS_HAS_AUDIO
    NEXUS_GetAudioCapabilities(&audioCaps);
    if (audioCaps.numOutputs.dac == 0 && audioCaps.numOutputs.i2s == 0 && audioCaps.numOutputs.dummy == 0 &&
        opts.common.compressedAudio) {
        /* If compressed output is requested and the board does not have DACs or I2S outputs
           then we need to attach a dummy output to put the decoder into simul mode.
           Some codecs (DDP for example) attach to the pcm decoder to get AC3 out.
           If no dummy outputs are available then pcm decoder will not be enabled
           and some compressed outputs may not have audio. */
        BDBG_WRN(("Compressed Audio: Disabling PCM audio decoding as no supported PCM audio interface"));
        opts.common.decodedAudio = false;
    }
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    if(opts.common.audioCdb) {
        audioDecoderOpenSettings.fifoSize = opts.common.audioCdb*1024;
    }
    if (opts.common.multichannelAudio) {
        audioDecoderOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
    }

    /* Bring up audio decoders and outputs */
    if(opts.common.decodedAudio) {
        audioDecoder = NEXUS_AudioDecoder_Open(0, &audioDecoderOpenSettings);
        BDBG_ASSERT(audioDecoder);
    }

    if ( opts.common.dtsCd && opts.common.decodedAudio) {
        NEXUS_AudioDecoderCodecSettings audioCodecSettings;
        NEXUS_AudioDecoder_GetCodecSettings(audioDecoder, NEXUS_AudioCodec_eDtsLegacy, &audioCodecSettings);
        audioCodecSettings.codecSettings.dts.littleEndian = opts.common.audioUseLittle;
        rc = NEXUS_AudioDecoder_SetCodecSettings(audioDecoder, &audioCodecSettings);
        BDBG_ASSERT(!rc);
    }

    if (opts.common.compressedAudio) {
        audioDecoderOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_eNone;
        audioDecoderOpenSettings.type = NEXUS_AudioDecoderType_ePassthrough;
        compressedDecoder = NEXUS_AudioDecoder_Open(1, &audioDecoderOpenSettings);
        BDBG_ASSERT(compressedDecoder);
        if ( opts.common.dtsCd ) {
            NEXUS_AudioDecoderCodecSettings audioCodecSettings;
            NEXUS_AudioDecoder_GetCodecSettings(compressedDecoder, NEXUS_AudioCodec_eDtsLegacy, &audioCodecSettings);
            audioCodecSettings.codecSettings.dts.littleEndian = true;
            rc = NEXUS_AudioDecoder_SetCodecSettings(compressedDecoder, &audioCodecSettings);
            BDBG_ASSERT(!rc);
        }
    }

#if NEXUS_NUM_HDMI_OUTPUTS
    if (opts.common.useHdmiOutput) {
#if !BDBG_NO_WRN
        static const char *g_audioConnectorTypeStr[NEXUS_AudioConnectorType_eMax] = { "stereo", "multichannel", "compressed", "compressed 4x", "compressed HBR", "mono" };
#endif
        NEXUS_HdmiOutputStatus hdmiStatus;
        NEXUS_AudioDecoderHandle hdmiDecoder = audioDecoder;
        NEXUS_AudioConnectorType connectorType = NEXUS_AudioConnectorType_eStereo;
        NEXUS_AudioInputHandle connector;
        unsigned pollcnt = 10; /* 1 second = 10 loops of 100 msec */

        while (pollcnt--) {
            rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
            BDBG_ASSERT(!rc);
            if (hdmiStatus.connected) {
                break;
            }
            BKNI_Sleep(100);
        }

        if (!hdmiStatus.connected) {
            BDBG_WRN(("HDMI not connected"));
            /* if we don't know the EDID, output 2 ch pcm */
            connectorType = NEXUS_AudioDecoderConnectorType_eStereo;
        }
        else if ( opts.common.multichannelAudio && hdmiStatus.maxAudioPcmChannels > 2 ) {
            connectorType = NEXUS_AudioConnectorType_eMultichannel;
            if (opts.common.audioCodec == NEXUS_AudioCodec_eAc4) {
                allowStereo = false;
            }
        }
        else if ( opts.common.compressedAudio ) {
            if ( opts.common.ignore_edid || hdmiStatus.audioCodecSupported[opts.common.audioCodec] ) {
                /* Codec is directly supported */
                hdmiDecoder = (opts.common.audioCodec == NEXUS_AudioCodec_eWmaPro) ? audioDecoder:compressedDecoder;
                switch ( opts.common.audioCodec ) {
                case NEXUS_AudioCodec_eMlp:
                    connectorType = NEXUS_AudioConnectorType_eCompressed16x;
                    break;
                case NEXUS_AudioCodec_eAc3Plus:
                    connectorType = NEXUS_AudioConnectorType_eCompressed4x;
                    break;
                case NEXUS_AudioCodec_eDtsHd:
                    {
                        NEXUS_HdmiOutputBasicEdidData basicEdid;
                        NEXUS_HdmiOutput_GetBasicEdidData(platformConfig.outputs.hdmi[0], &basicEdid);
                        if ( basicEdid.edidRevision >= 3 ) {  /* Check for HDMI 1.3 or later */
                            connectorType = NEXUS_AudioConnectorType_eCompressed16x;      /* Send DTS-HD as MA (should parse stream to select but this is simpler) */
                        }
                        else {
                            connectorType = NEXUS_AudioConnectorType_eCompressed4x;      /* Send DTS-HD as HRA not MA */
                        }
                    }
                    break;
                default:
                    connectorType = NEXUS_AudioConnectorType_eCompressed;
                    break;
                }
            }
            else if ( opts.common.audioCodec == NEXUS_AudioCodec_eAc3Plus && hdmiStatus.audioCodecSupported[NEXUS_AudioCodec_eAc3] ) {
                /* AC3 is supported, convert ac3+ to ac3. */
                connectorType = NEXUS_AudioConnectorType_eCompressed;
            }
            else if ( opts.common.audioCodec == NEXUS_AudioCodec_eDtsLegacy || opts.common.audioCodec == NEXUS_AudioCodec_eDts ) {
                if ( hdmiStatus.audioCodecSupported[NEXUS_AudioCodec_eDts] || hdmiStatus.audioCodecSupported[NEXUS_AudioCodec_eDtsHd] ) {
                    hdmiDecoder = compressedDecoder;
                    connectorType = NEXUS_AudioConnectorType_eCompressed;
                }
            }
        }
        else if (!opts.common.decodedAudio) {
            connectorType = NEXUS_AudioConnectorType_eMax;
        }

        if ( connectorType == NEXUS_AudioConnectorType_eCompressed16x || connectorType == NEXUS_AudioConnectorType_eCompressed4x ) {
            BDBG_WRN(("Forcing HD display mode for HBR audio on HDMI"));
            opts.common.displayFormat = NEXUS_VideoFormat_e1080i;
            opts.common.useCcir656Output = false;
            opts.common.useCompositeOutput = false;
        }

        if (connectorType != NEXUS_AudioConnectorType_eMax && hdmiDecoder != NULL ) {
            BDBG_WRN(("hdmi audio: decoder %d -> %s", hdmiDecoder==audioDecoder?0:1, g_audioConnectorTypeStr[connectorType]));
            connector = NEXUS_AudioDecoder_GetConnector(hdmiDecoder, connectorType);
            if ( NULL == connector ) {
                /* This decoder doesn't support HBR output.  Try standard compressed instead */
                if ( connectorType == NEXUS_AudioConnectorType_eCompressed16x || connectorType == NEXUS_AudioConnectorType_eCompressed4x ) {
                    BDBG_WRN(("hdmi audio: decoder %d -> %s", hdmiDecoder==audioDecoder?0:1, g_audioConnectorTypeStr[NEXUS_AudioConnectorType_eCompressed]));
                    connector = NEXUS_AudioDecoder_GetConnector(hdmiDecoder, NEXUS_AudioConnectorType_eCompressed);
                }
            }
            if ( NULL != connector ) {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), connector);
                BDBG_ASSERT(!rc);
#if NEXUS_HAS_SYNC_CHANNEL
                if (opts.sync) {
                    NEXUS_SyncChannel_GetSettings(sync, &syncSettings);
                    if (hdmiDecoder == compressedDecoder && !syncSettings.audioInput[1]) {
                        syncSettings.audioInput[1] = NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioConnectorType_eCompressed);
                        NEXUS_SyncChannel_SetSettings(sync, &syncSettings);
                    }
                    else if (hdmiDecoder == audioDecoder && connectorType == NEXUS_AudioConnectorType_eMultichannel) {
                        syncSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eMultichannel);
                        NEXUS_SyncChannel_SetSettings(sync, &syncSettings);
                    }
                    else if (hdmiDecoder == audioDecoder && connectorType == NEXUS_AudioConnectorType_eStereo) {
                        syncSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo);
                        NEXUS_SyncChannel_SetSettings(sync, &syncSettings);
                    }
                }
#endif
            }
        }
        else {
            BDBG_ERR(("HDMI not connected because either connector type is max or decoder is not opened"));
        }
    }
#endif /*NEXUS_NUM_HDMI_OUTPUTS*/

    if ( audioCaps.numOutputs.spdif > 0 ) {
        if (opts.common.compressedAudio) {
            if ( opts.common.audioCodec == NEXUS_AudioCodec_eAc3Plus || opts.common.audioCodec == NEXUS_AudioCodec_eWmaPro ) {
                /* These codecs pasthrough as part of decode (ac3+ is transcoded to ac3, wma pro is not transcoded) */
                if (opts.common.decodedAudio) {
                    BDBG_WRN(("spdif audio: decoder 0 -> compressed"));
                    rc = NEXUS_AudioOutput_AddInput(
                        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eCompressed));
                    BDBG_ASSERT(!rc);
                }
                else {
                    BDBG_WRN(("spdif audio: decoder 0 -> disconnected (no PCM decoder configured)"));
                }
            }
            else if ( opts.common.audioCodec && opts.common.audioCodec != NEXUS_AudioCodec_eMlp ) {

               BDBG_WRN(("spdif audio: decoder 1 -> compressed"));
                rc = NEXUS_AudioOutput_AddInput(
                    NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                    NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioConnectorType_eCompressed));
                BDBG_ASSERT(!rc);
                #if NEXUS_HAS_SYNC_CHANNEL
                if (opts.sync) {
                    NEXUS_SyncChannel_GetSettings(sync, &syncSettings);
                    syncSettings.audioInput[1] = NEXUS_AudioDecoder_GetConnector(compressedDecoder, NEXUS_AudioConnectorType_eCompressed);
                    NEXUS_SyncChannel_SetSettings(sync, &syncSettings);
                }
                #endif
            }
            else if ( opts.common.audioCodec && opts.common.audioCodec == NEXUS_AudioCodec_eMlp ) {
                if (opts.common.decodedAudio) {
                    BDBG_WRN(("spdif audio: decoder 0 -> stereo"));
                    rc = NEXUS_AudioOutput_AddInput(
                        NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                        NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo));
                    BDBG_ASSERT(!rc);
                }
                else {
                    BDBG_WRN(("spdif audio: decoder 0 -> disconnected (no PCM decoder configured)"));
                }
            }
        }
        else {
            if (opts.common.decodedAudio && allowStereo) {
                BDBG_WRN(("spdif audio: decoder 0 -> stereo"));
                rc = NEXUS_AudioOutput_AddInput(
                    NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                    NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo));
                BDBG_ASSERT(!rc);
            }
        }
    }

    if (opts.common.audioPid && opts.common.decodedAudio && allowStereo) {
        if ( audioCaps.numOutputs.dac > 0 ) {
            rc = NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo));
            BDBG_ASSERT(!rc);
        }
        else if ( audioCaps.numOutputs.i2s > 0 ) {
            rc = NEXUS_AudioOutput_AddInput(
                NEXUS_I2sOutput_GetConnector(platformConfig.outputs.i2s[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo));
            BDBG_ASSERT(!rc);
        }
        else if ( audioCaps.numOutputs.dummy > 0 ) {
            NEXUS_AudioOutputSettings dummySettings;
            NEXUS_AudioOutput_GetSettings(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]), &dummySettings);
            dummySettings.pll = NEXUS_AudioOutputPll_e1;
            dummySettings.nco = NEXUS_AudioOutputNco_eMax;
            NEXUS_AudioOutput_SetSettings(NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]), &dummySettings);
            rc = NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDummyOutput_GetConnector(platformConfig.outputs.audioDummy[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo));
            BDBG_ASSERT(!rc);
        }

        #if NEXUS_HAS_SYNC_CHANNEL
        if (opts.sync) {
            NEXUS_SyncChannel_GetSettings(sync, &syncSettings);
            if (syncSettings.audioInput[0] == NULL) {
                syncSettings.audioInput[0] = NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioConnectorType_eStereo);
                NEXUS_SyncChannel_SetSettings(sync, &syncSettings);
            }
        }
        #endif
        NEXUS_AudioDecoder_GetSettings(audioDecoder, &audioDecoderSettings);
        audioDecoderSettings.wideGaThreshold = opts.looseAudioTsm;
        rc = NEXUS_AudioDecoder_SetSettings(audioDecoder, &audioDecoderSettings);
        BDBG_ASSERT(!rc);
    }
    #endif /* NEXUS_HAS_AUDIO */

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.displayType = opts.common.displayType;
    displaySettings.format = opts.common.displayFormat;
    if(opts.common.displayAspectRatio != NEXUS_DisplayAspectRatio_eSar) {
        displaySettings.aspectRatio = opts.common.displayAspectRatio;
    }
    if(opts.common.displayOrientation != NEXUS_VideoOrientation_e2D) {
        displaySettings.display3DSettings.overrideOrientation = true;
        displaySettings.display3DSettings.orientation  = opts.common.displayOrientation;
        displaySettings.display3DSettings.sourceBuffer = opts.common.displaySourceBuffer;
    }
    if (opts.cmp_crc) {
        displaySettings.crcQueueSize = 60; /* This enables the CRC capture */
    }
    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
    if (opts.common.masterModeTimingGenerator) {
        displaySettings.timingGenerator = NEXUS_DisplayTimingGenerator_eHdmiDvo; /* HDMI master mode */
    }
    display = NEXUS_Display_Open(0, &displaySettings);

#if NEXUS_NUM_COMPOSITE_OUTPUTS
    if (opts.common.useCompositeOutput && platformConfig.outputs.composite[0] && platformCap.display[1].supported && !platformCap.display[1].encoder) {
        NEXUS_VideoFormatInfo videoFormatInfo;
        NEXUS_DisplaySettings compositeDisplaySettings;
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
        NEXUS_Display_GetDefaultSettings(&compositeDisplaySettings);
        compositeDisplaySettings.format = (videoFormatInfo.verticalFreq == 5000)
            ? NEXUS_VideoFormat_ePal : NEXUS_VideoFormat_eNtsc;
        displaySD = NEXUS_Display_Open(1, &compositeDisplaySettings);
        if(displaySD) {
            rc = NEXUS_Display_AddOutput(displaySD, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
            if (rc) BERR_TRACE(rc); /* keep going */
#if NEXUS_NUM_656_OUTPUTS
            if (opts.common.useCcir656Output && platformConfig.outputs.ccir656[0] && displaySD) {
               rc = NEXUS_Display_AddOutput(displaySD, NEXUS_Ccir656Output_GetConnector( platformConfig.outputs.ccir656[0]));
               if (rc) BERR_TRACE(rc); /* keep going */
            }
#endif
        }
    }
#endif

#if NEXUS_NUM_COMPONENT_OUTPUTS
    if (opts.common.useComponentOutput) {
        rc = NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
        if (rc) BERR_TRACE(rc); /* keep going */
    }
#endif
#if NEXUS_HAS_HDMI_OUTPUT
    if (opts.common.useHdmiOutput) {
        NEXUS_HdmiOutputSettings hdmiSettings;

        rc = NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
        BDBG_ASSERT(!rc);

        /* Install hotplug callback -- video only for now */
        NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
        hotplug_context.hdmi = platformConfig.outputs.hdmi[0];
        hotplug_context.display = display;
        hotplug_context.ignore_edid = opts.common.ignore_edid;
        hdmiSettings.hotplugCallback.callback = hotplug_callback;
        hdmiSettings.hotplugCallback.context =  &hotplug_context;
        if (opts.hdmi_crc) {
            hdmiSettings.crcQueueSize = 60; /* This enables the CRC capture */
        }

        hdmiSettings.hdmiRxScdcMonitoring = true ;
        hdmiSettings.hdmiRxStatusChanged.callback = hdmiRxChangedCallback ;
        hdmiSettings.hdmiRxStatusChanged.context = platformConfig.outputs.hdmi[0];

        NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

        /* Force a hotplug to switch to preferred format */
        hotplug_callback(&hotplug_context, 0);
    }
#endif

    if (opts.graphics || opts.gfx_bar) {
        NEXUS_SurfaceCreateSettings surfaceCreateSettings;
        NEXUS_SurfaceMemory mem;
        NEXUS_GraphicsSettings graphicsSettings;
        NEXUS_VideoFormatInfo videoFormatInfo;
        unsigned i,j;

        NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);

        NEXUS_Surface_GetDefaultCreateSettings(&surfaceCreateSettings);
        surfaceCreateSettings.width = 720;
        surfaceCreateSettings.height = (opts.gfx_bar)? 480 : videoFormatInfo.height;
        surfaceCreateSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);
        framebuffer = NEXUS_Surface_Create(&surfaceCreateSettings);
        NEXUS_Surface_GetMemory(framebuffer, &mem);
        if (opts.gfx_bar) {
            for (i=0;i<surfaceCreateSettings.height;i++) {
                for (j=0;j<surfaceCreateSettings.width;j++) {
                    if (j>=20 && j<700) {
                        int c = (((j-20) / 68) * 255) / 9;
                        if (i>=400 && i<410) {
                            ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (0xFF000000 | (c << 16));
                        } else if (i>=410 && i<420) {
                            ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (0xFF000000 | (c << 8));
                        } else if (i>=420 && i<430) {
                            ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (0xFF000000 | (c << 0));
                        } else if (i>=430 && i<440) {
                            ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (0xFF000000 | (c << 0) | (c << 8) | (c << 16));
                        } else {
                            ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = 0;
                        }
                    } else {
                        ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = 0;
                    }
                }
            }
        } else {
            for (i=0;i<surfaceCreateSettings.height;i++) {
                for (j=0;j<surfaceCreateSettings.width;j++) {
                    /* create checker board */
                    ((uint32_t*)((uint8_t*)mem.buffer + i*mem.pitch))[j] = (((i/50)%2) ^ ((j/50)%2)) ? 0x00000000 : 0xFFFFFFFF;
                }
            }
        }

        NEXUS_Surface_Flush(framebuffer);

        NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
        graphicsSettings.enabled = true;
        graphicsSettings.clip.width = surfaceCreateSettings.width;
        graphicsSettings.clip.height = surfaceCreateSettings.height;
        rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
        BDBG_ASSERT(!rc);
        rc = NEXUS_Display_SetGraphicsFramebuffer(display, framebuffer);
        BDBG_ASSERT(!rc);
    }

    window = NEXUS_VideoWindow_Open(display, 0);
    if (displaySD) {
        windowSD = NEXUS_VideoWindow_Open(displaySD, 0);
        if (windowSD != NULL) {
            NEXUS_VideoWindow_GetSettings(windowSD, &windowSettings);
            windowSettings.contentMode = opts.common.contentMode;
            rc = NEXUS_VideoWindow_SetSettings(windowSD, &windowSettings);
            BDBG_ASSERT(!rc);
        }
    }

    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    full = windowSettings.position;
    scaled.x = (full.width * percent) / 200;
    scaled.width = full.width - scaled.x * 2;
    scaled.y = (full.height * percent) / 200;
    scaled.height = full.height - scaled.y * 2;
    windowSettings.contentMode = opts.common.contentMode;
    if (opts.mem == playback_mem_minimal) {
        windowSettings.forceCapture = false;
    }
    rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    BDBG_ASSERT(!rc);

    NEXUS_VideoWindow_GetAfdSettings(window, &windowAfdSettings);
    windowAfdSettings.mode      = opts.common.afd.mode;
    windowAfdSettings.clip      = opts.common.afd.clip;
    windowAfdSettings.userValue = opts.common.afd.userValue;
    rc = NEXUS_VideoWindow_SetAfdSettings(window, &windowAfdSettings);
    BDBG_ASSERT(!rc);

    if (opts.mem == playback_mem_minimal) {
        NEXUS_VideoWindowMadSettings madSettings;
        NEXUS_VideoWindow_GetMadSettings(window, &madSettings);
        madSettings.deinterlace = false;
        NEXUS_VideoWindow_SetMadSettings(window, &madSettings);
    }

    if (opts.common.smoothSourceChange) {
        NEXUS_VideoWindowMadSettings madSettings;

        NEXUS_VideoWindow_GetSettings(window, &windowSettings);
        /* (1) Won't re-alloc buffers to meet source size changges and
         *     also put MADR/MCVP HW into bypass when not needed. */
        windowSettings.minimumSourceFormat = NEXUS_VideoFormat_e1080i;

        /* (2) Use fullscreen size buffer instead of PIP/PIG size to determine
         * allocation. */
        windowSettings.allocateFullScreen = true;

        /* (3) Not auto snapping to integer scaling factor. */
        windowSettings.scaleFactorRounding.enabled = true;
        windowSettings.scaleFactorRounding.verticalTolerance = 0;
        windowSettings.scaleFactorRounding.horizontalTolerance = 0;

        rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
        BDBG_ASSERT(!rc);

        /* (4) Fixed scaler -> capture vs capture -> scaler orienation to avoid
         *     reconfiguration of VNET (Video Network).  This require RTS
         *     and Usage analysis to make sure it supportted. */
        NEXUS_VideoWindow_GetScalerSettings(window, &scalerSettings);
        scalerSettings.bandwidthEquationParams.bias = NEXUS_ScalerCaptureBias_eScalerBeforeCapture;
        scalerSettings.bandwidthEquationParams.delta = 1*1000*1000;
        rc = NEXUS_VideoWindow_SetScalerSettings(window, &scalerSettings);
        BDBG_ASSERT(!rc);

        /* (5) For chips (7400/7405/7420/7408) that have older deinterlacer
         * that does not have HW bypass capability setting this flag will
         * prevent deinteralcer forced off when in 480i->480i,
         * 576i->576i, and 1080i->1080i non-scaled mode.  See also jira
         * SW7420-2423.*/
        NEXUS_VideoWindow_GetMadSettings(window, &madSettings);
        madSettings.pqEnhancement = NEXUS_MadPqEnhancement_eOff;
        rc = NEXUS_VideoWindow_SetMadSettings(window, &madSettings);
        BDBG_ASSERT(!rc);
    }

    NEXUS_VideoDecoder_GetDefaultOpenSettings(&openSettings);
    if(opts.common.videoCdb) {
        openSettings.fifoSize = opts.common.videoCdb*1024;
    }
    if(opts.avc51) {
        openSettings.avc51Enabled = opts.avc51;
    }
    /* bring up decoder and connect to display */
    switch (opts.common.extVideoCodec) {
    case NEXUS_VideoCodec_eH264_Svc:
    case NEXUS_VideoCodec_eH264_Mvc:
        openSettings.enhancementPidChannelSupported = true;
        break;
    default:
        break;
    }

    if (opts.common.videoDecoder == 0) {
        NEXUS_VideoDecoderCapabilities cap;
        NEXUS_GetVideoDecoderCapabilities(&cap);
        if (opts.common.videoCodec == NEXUS_VideoCodec_eMotionJpeg && cap.sidVideoDecoder.useForMotionJpeg) {
            opts.common.videoDecoder = cap.sidVideoDecoder.baseIndex;
        }
        else if (opts.common.videoCodec == NEXUS_VideoCodec_eVp6 && cap.dspVideoDecoder.useForVp6) {
            opts.common.videoDecoder = cap.dspVideoDecoder.baseIndex;
        }
    }
    videoDecoder = NEXUS_VideoDecoder_Open(opts.common.videoDecoder, &openSettings);
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    if (opts.common.sourceOrientation != NEXUS_VideoDecoderSourceOrientation_e2D) {
        videoDecoderSettings.customSourceOrientation = true;
        videoDecoderSettings.sourceOrientation = opts.common.sourceOrientation;
    }
    videoDecoderSettings.supportedCodecs[opts.common.extVideoCodec] = true;
    if (opts.common.maxWidth && opts.common.maxHeight) {
        videoDecoderSettings.maxWidth = opts.common.maxWidth;
        videoDecoderSettings.maxHeight = opts.common.maxHeight;
    }
    videoDecoderSettings.scanMode = opts.common.scanMode;
    rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);

    if (opts.mfd_crc) {
        NEXUS_VideoInputSettings inputSettings;
        NEXUS_VideoInput_GetSettings(NEXUS_VideoDecoder_GetConnector(videoDecoder),
            &inputSettings);
        inputSettings.crcQueueSize = 60; /* This enables the CRC capture */
        NEXUS_VideoInput_SetSettings(NEXUS_VideoDecoder_GetConnector(videoDecoder),
            &inputSettings);
        NEXUS_VideoWindow_GetSettings(window, &windowSettings);
        windowSettings.scaleFactorRounding.enabled = true;
        windowSettings.scaleFactorRounding.verticalTolerance = 0;
        windowSettings.scaleFactorRounding.horizontalTolerance = 3;
        rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    }

    if (opts.avd_crc) {
        NEXUS_VideoDecoderExtendedSettings settings;
        NEXUS_VideoDecoder_GetExtendedSettings(videoDecoder, &settings);
        settings.crcFifoSize = 30;
        rc = NEXUS_VideoDecoder_SetExtendedSettings(videoDecoder, &settings);
        BDBG_ASSERT(!rc);
    }
    rc = NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));
    BDBG_ASSERT(!rc);

    if (windowSD) {
        rc = NEXUS_VideoWindow_AddInput(windowSD, NEXUS_VideoDecoder_GetConnector(videoDecoder));
        BDBG_ASSERT(!rc);
    }

#if NEXUS_HAS_SYNC_CHANNEL
    if (opts.sync)
    {
        NEXUS_SyncChannel_GetSettings(sync, &syncSettings);
        syncSettings.videoInput = NEXUS_VideoDecoder_GetConnector(videoDecoder);
        NEXUS_SyncChannel_SetSettings(sync, &syncSettings);
    }
#endif

#if NEXUS_HAS_HDMI_OUTPUT
    hdmi_drm_init_context(&hdmiDrmContext, videoDecoder, platformConfig.outputs.hdmi[0]);
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.streamChanged.callback = &hdmi_drm_video_stream_changed;
    videoDecoderSettings.streamChanged.context = &hdmiDrmContext;
    videoDecoderSettings.streamChanged.param = 0;
    rc = NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    BDBG_ASSERT(!rc);
#endif

    if(displaySD) {
        NEXUS_Display_GetVbiSettings(displaySD, &displayVbiSettings);
        displayVbiSettings.vbiSource = NEXUS_VideoDecoder_GetConnector(videoDecoder);
        displayVbiSettings.closedCaptionEnabled = opts.closedCaptionEnabled;
        displayVbiSettings.closedCaptionRouting = opts.closedCaptionEnabled;
        rc = NEXUS_Display_SetVbiSettings(displaySD, &displayVbiSettings);
        if (rc) rc = BERR_TRACE(rc);
    }

    /* Open the audio and video pid channels */
    if (opts.common.videoCodec != NEXUS_VideoCodec_eNone && opts.common.videoPid!=0 && opts.common.decodedVideo) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidSettings.allowTimestampReordering = opts.common.playpumpTimestampReordering;
        playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.codec = opts.common.videoCodec;
        videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.common.videoPid, &playbackPidSettings);
    }

    if (opts.common.extVideoCodec != NEXUS_VideoCodec_eNone && opts.common.extVideoPid!=0 && opts.common.decodedVideo) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidSettings.allowTimestampReordering = opts.common.playpumpTimestampReordering;
        playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
        playbackPidSettings.pidTypeSettings.video.codec = opts.common.extVideoCodec;
        videoExtPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.common.extVideoPid, &playbackPidSettings);
    }

    if (opts.common.audioCodec != NEXUS_AudioCodec_eUnknown && opts.common.audioPid!=0 && (opts.common.decodedAudio || compressedDecoder)) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        if (audioDecoder != NULL)
        {
            playbackPidSettings.pidTypeSettings.audio.primary = audioDecoder;
            if (compressedDecoder != NULL)
            {
                playbackPidSettings.pidTypeSettings.audio.secondary = compressedDecoder;
            }
        }
        else if ( compressedDecoder != NULL)
        {
            playbackPidSettings.pidTypeSettings.audio.primary = compressedDecoder;
        }

        playbackPidSettings.pidSettings.pidTypeSettings.audio.codec = opts.common.audioCodec;
        audioPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.common.audioPid, &playbackPidSettings);
    }

    if (opts.common.pcrPid && opts.common.pcrPid!=opts.common.videoPid && opts.common.pcrPid!=opts.common.audioPid) {
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eOther;
        pcrPidChannel = NEXUS_Playback_OpenPidChannel(playback, opts.common.pcrPid, &playbackPidSettings);
        BSTD_UNUSED(pcrPidChannel);
    }

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = opts.common.videoCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    videoProgram.frameRate = opts.common.videoFrameRate;
    videoProgram.aspectRatio = opts.common.aspectRatio;
    videoProgram.sampleAspectRatio.x = opts.common.sampleAspectRatio.x;
    videoProgram.sampleAspectRatio.y = opts.common.sampleAspectRatio.y;
    videoProgram.errorHandling = opts.videoErrorHandling;
    videoProgram.timestampMode = opts.common.decoderTimestampMode;
    if(videoExtPidChannel) {
        videoProgram.enhancementPidChannel = videoExtPidChannel;
        videoProgram.codec = opts.common.extVideoCodec;
    }
    if (opts.mfd_crc) {
        videoProgram.crcMode = NEXUS_VideoDecoderCrcMode_eMfd;
    }
#if NEXUS_HAS_AUDIO
    NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
    audioProgram.codec = opts.common.audioCodec;
    audioProgram.pidChannel = audioPidChannel;
    audioProgram.stcChannel = stcChannel;
    audioProgram.maxOutputRate = opts.common.maxAudioRate;
#endif

    /* Start decoders */
    rc = start_video(&opts, videoDecoder, &videoProgram);
    if (rc) return -1;
    start_audio(&opts, audioDecoder, compressedDecoder, &audioProgram);

    decoder_bitrate_control_video(&video_bitrate, videoDecoder, true);
    decoder_bitrate_control_audio(&audio_bitrate, audioDecoder, true);

    /* Start playback */
    NEXUS_Playback_GetDefaultStartSettings(&playbackStartSettings);
    if (opts.fixedBitrate) {
        playbackStartSettings.mode = NEXUS_PlaybackMode_eFixedBitrate;
        playbackStartSettings.bitrate = opts.fixedBitrate;
    }
    else if (opts.autoBitrate) {
        playbackStartSettings.mode = NEXUS_PlaybackMode_eAutoBitrate;
    }
#if READ_TIMED_DATA
    {
        barena_t arena;

        arena = barena_create(&b_nexus_alloc_iface, 2*1024*1024);
        BDBG_ASSERT(arena);
        bsink_dump_reader_allocator = barena_alloc_iface(arena);
    }
#endif
    rc = NEXUS_Playback_Start(playback, file, &playbackStartSettings);
    BDBG_ASSERT(!rc);

#if B_HAS_PLAYBACK_MONITOR
    {
        PlaybackMonitorState monitorState;
        BKNI_Memset(&monitorState, 0, sizeof(monitorState));
        monitorState.opts = &opts;
        monitorState.playback = playback;
        monitorState.videoDecoder = videoDecoder;
        monitorState.videoProgram = &videoProgram;
        monitorState.audioDecoder = audioDecoder;
        monitorState.compressedDecoder = compressedDecoder;
        monitorState.audioProgram = &audioProgram;
        monitorState.file = file;
        monitorState.stickyFile = stickyFile;
        monitorState.customFile = customFile;
        monitorState.playbackStartSettings = &playbackStartSettings;
        if(stickyFile) {
            monitor_thread_start(&monitorState);
        }
#else
        BSTD_UNUSED(stickyFile);
#endif

    if (opts.avd_crc) {
        print_avd_crc(videoDecoder, opts.endOfStreamAction != NEXUS_PlaybackLoopMode_eLoop?endOfStreamEvent:NULL);
        goto uninit;
    }
    if (opts.cmp_crc) {
        print_cmp_crc(display, opts.endOfStreamAction != NEXUS_PlaybackLoopMode_eLoop?endOfStreamEvent:NULL);
        goto uninit;
    }
    if (opts.hdmi_crc) {
        print_hdmi_crc(platformConfig.outputs.hdmi[0], opts.endOfStreamAction != NEXUS_PlaybackLoopMode_eLoop?endOfStreamEvent:NULL);
        goto uninit;
    }
    if (opts.mfd_crc) {
        print_mfd_crc(videoDecoder, opts.endOfStreamAction != NEXUS_PlaybackLoopMode_eLoop?endOfStreamEvent:NULL);
        goto uninit;
    }

#if BRDC_USE_CAPTURE_BUFFER
    if (NEXUS_GetEnv("capture_ruls")) {
        read_rul_capture_fifo();
    }
#endif

    for (exit=false;!exit;)
    {
        char buffer[256];
        char *buf;

        printf("playback>"); fflush(stdout);
        fgets(buffer, 256, stdin);
        if (feof(stdin)) break;
        buffer[strlen(buffer)-1] = 0; /* chop off \n */

        buf = strtok(buffer, ";");
        if (!buf) continue;

#if B_HAS_PLAYBACK_MONITOR
        if(stickyFile) {
            BKNI_AcquireMutex(monitorState.lock);
        }
#endif
        do {
            if (echo) {
                printf("%s\n", buf);
            }
            if (!strcmp(buf, "?") || !strcmp(buf, "help")) {
                printf(
                "Commands:\n"
                "  play - resume normal playback\n"
                "  pause - pause playback\n"
                "  i - decode only I frames\n"
                "  ip - decode only I & P frames\n"
                "  all - decode all frames\n"
                );
                printf(
                "  fa - frame advance\n"
                "  fr - frame reverse\n"
                "  rate(rate) - set trick rate (floating point number, 1.0 is normal play)\n"
                "  host(type[,modifier,slowrate]) - set host trick mode\n"
                "    type=i,ip,all,bcm,dqt,dqtip,mdqt,mdqtip\n"
                "      if type=i, modifier is # of I frames to play\n"
                "      if type=bcm, modifier is rate\n"
                "      if type=dqt|dqtip, modifier%%100 is # of frames per GOP to play, modifier/100 is GOP skip\n"
                "    slowrate=decoder repeat rate (1=1x,2=2x)\n"
                );
                printf(
                "  hostrate(rate,type) - set rate and host trick mode\n"
                "    rate=1.0 is normal play\n"
                "    type=i,ip,all,bcm,dqt,dqtip,mdqt,mdqtip\n"
                );
                printf(
                "  seek(pos) - jump to position (in milliseconds)\n"
                "  sleep(msec) - sleep for given number of milli-seconds\n"
                "  wait - exit when playback ends\n"
                "  echo - toggle echo of stdin. useful for scripting.\n"
                "  st - print status\n"
                "  af - audio flush\n"
                "  restart - stop playback and restart play from the current position\n"
                "  4k  - toggle 4k2k hdmi output format\n");
                printf(
                "  Picture Quality Controls\n"
                "    anr(#|OFF) - enable anr (Analog Noise Reduction) and set level or turn off\n"
                "    dnr - toggle dnr (Digital Noise Reduction) filter\n"
                "    mad - toggle mad (Deinterlacer)\n"
                "    cap - toggle cap (Forced Capture)\n"
                "    sci - select scaler coefficients index [101->126 :: softness->sharpness]\n"
                );
                printf(
                "    sharpness(onoff,gain) - set sharpness level\n"
                "      on=1:enable - off=0:disable\n"
                "      gain=[-32768, 32767]\n"
                );
                printf(
                "  HDMI Output Controls\n"
                "    hdmi_colorspace(rgb|420|422|444)\n"
                "    hdmi_colorrange(auto|limited|full)\n"
                "    hdmi_matrixcoefficients(auto|709|170m|470_2_bg|2020ncl|2020cl)\n"
                "    hdmi_colordepth(0|8|10|12)\n"
                "    hdmi_matrix(0|2020ncl|2020cl|full-ycc|full-rgb)\n"
                "    hdmi_rescramble - call NEXUS_HdmiOutput_ResetScrambling\n"
                "    hdmi_edid - call NEXUS_HdmiOutput_DisplayRxEdid\n"
                "    hdmi_eotf(input,toggle,sdr,hdr10,hlg)\n"
                );
/*TODO* figure out what these do and add appropriate explanation*/
#if 0
                if (customFile) printf(
                "  error_index(error,nodata,partial_read) - get \n"
                "  error_data(error,nodata,partial_read) - set \n"
                );
#endif
                printf(
                "  q - quit\n"
                );
            }
            else if (!strcmp(buf, "q") || !strcmp(buf, "quit")) {
                exit = true;
                break;
            }
            else if (!strcmp(buf, "play")) {
                rc = NEXUS_Playback_Play(playback);
                decoder_bitrate_control_video(&video_bitrate, videoDecoder, true);
                decoder_bitrate_control_audio(&audio_bitrate, audioDecoder, true);
                BDBG_ASSERT(!rc);
            }
            else if (!strcmp(buf, "fa") || !strcmp(buf, "fr")) {
                if (videoPidChannel) { /* will fail if no video */
                    bool forward = !strcmp(buf, "fa");
                    unsigned i;
                    for (i=0;i<20;i++) {
                        NEXUS_VideoDecoderStatus status;
                        rc = NEXUS_Playback_FrameAdvance(playback, forward);
                        if (rc) break;
                        BKNI_Sleep(17);
                        rc = NEXUS_VideoDecoder_GetStatus(videoDecoder, &status);
                        if (!rc && status.numDisplayed) break;
                    }
                }
            }
            else if (!strcmp(buf, "pause")) {
                decoder_bitrate_control_video(&video_bitrate, videoDecoder, false);
                decoder_bitrate_control_audio(&audio_bitrate, audioDecoder, false);
                rc = NEXUS_Playback_Pause(playback);
                BDBG_ASSERT(!rc);
            }
            else if (!strcmp(buf, "scaled")) {
                NEXUS_VideoWindowSettings settings;
                NEXUS_VideoWindow_GetSettings(window, &settings);
                settings.position = scaled;
                NEXUS_VideoWindow_SetSettings(window, &settings);
            }
            else if (!strcmp(buf, "full")) {
                NEXUS_VideoWindowSettings settings;
                NEXUS_VideoWindow_GetSettings(window, &settings);
                settings.position = full;
                NEXUS_VideoWindow_SetSettings(window, &settings);
            }
            else if (!strcmp(buf, "i")) {
                NEXUS_PlaybackTrickModeSettings trickSettings;
                NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
                trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayI;
                trickSettings.skipControl = NEXUS_PlaybackSkipControl_eDecoder;
                trickSettings.rateControl = NEXUS_PlaybackRateControl_eDecoder;
                rc = NEXUS_Playback_TrickMode(playback, &trickSettings);
                BDBG_ASSERT(!rc);
            }
            else if (!strcmp(buf, "ip")) {
                NEXUS_PlaybackTrickModeSettings trickSettings;
                NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
                trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayIP;
                trickSettings.skipControl = NEXUS_PlaybackSkipControl_eDecoder;
                trickSettings.rateControl = NEXUS_PlaybackRateControl_eDecoder;
                rc = NEXUS_Playback_TrickMode(playback, &trickSettings);
                BDBG_ASSERT(!rc);
            }
            else if (!strcmp(buf, "all")) {
                NEXUS_PlaybackTrickModeSettings trickSettings;
                NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
                trickSettings.mode = NEXUS_PlaybackHostTrickMode_eNormal;
                trickSettings.skipControl = NEXUS_PlaybackSkipControl_eDecoder;
                trickSettings.rateControl = NEXUS_PlaybackRateControl_eDecoder;
                rc = NEXUS_Playback_TrickMode(playback, &trickSettings);
                BDBG_ASSERT(!rc);
            }
            else if (strstr(buf, "sleep(") == buf) {
                unsigned msec;
                sscanf(buf+6, "%u", &msec);
                BKNI_Sleep(msec);
            }
            else if (!strcmp(buf, "wait")) {
                BKNI_WaitForEvent(endOfStreamEvent, BKNI_INFINITE);
                exit = true;
            }
            else if (!strcmp(buf, "echo")) {
                echo = !echo;
                printf("echo %s\n", echo?"on":"off");
            }
            else if (strstr(buf, "rate(") == buf) {
                NEXUS_PlaybackTrickModeSettings trickSettings;
                float rate;

                sscanf(buf+5, "%f", &rate);
                NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
                trickSettings.rate = NEXUS_NORMAL_DECODE_RATE * rate;
                if (!opts.stcTrick &&
                    (trickSettings.rate > 0 && trickSettings.rate != NEXUS_NORMAL_DECODE_RATE && trickSettings.rate < 2*NEXUS_NORMAL_DECODE_RATE))
                {
                    BDBG_WRN(("Using decoder/host trick mode; restart with -stctrick for precise rates and DSOLA audio trick modes."));
                }
                if (!trickSettings.rate && rate) {
                    trickSettings.rate = rate>0?1:-1;
                }
                if(opts.maxDecoderRate) {
                    trickSettings.maxDecoderRate = opts.maxDecoderRate;
                }
                rc = NEXUS_Playback_TrickMode(playback, &trickSettings);
                decoder_bitrate_control_video(&video_bitrate, videoDecoder, trickSettings.rate == NEXUS_NORMAL_DECODE_RATE);
                decoder_bitrate_control_audio(&audio_bitrate, audioDecoder, trickSettings.rate == NEXUS_NORMAL_DECODE_RATE);
                /* don't assert for trick modes */
            }
            else if (strstr(buf, "host(") == buf) {
                char *s;
                char trick[64];
                int n, modifier, decoder_slowrate;
                NEXUS_PlaybackTrickModeSettings trickSettings;

                /* convert , and ) into spaces for sscanf */
                while ((s = strpbrk(buf, ",)"))) {
                    *s = ' ';
                }

                n = sscanf(buf+5, "%s %d %d", trick, &modifier, &decoder_slowrate);
                if (n < 2 || modifier == 0) modifier = 1;
                if (n < 3 || decoder_slowrate == 0) decoder_slowrate = 1;

                BDBG_MSG(("host(%s,%d,%d)", trick, modifier, decoder_slowrate));

                NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);
                trickSettings.skipControl = NEXUS_PlaybackSkipControl_eHost;
                trickSettings.rateControl = NEXUS_PlaybackRateControl_eDecoder;
                trickSettings.mode_modifier = modifier;
                if (!strcasecmp(trick, "ip")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayIP;
                }
                else if (!strcasecmp(trick, "i")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayI;
                }
                else if (!strcasecmp(trick, "all")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_eNormal;
                }
                else if (!strcasecmp(trick, "dqt") || !strcasecmp(trick, "gop")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayDqt;
                }
                else if (!strcasecmp(trick, "dqtip") || !strcasecmp(trick, "gopip")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayDqtIP;
                }
                else if (!strcasecmp(trick, "mdqt")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayMultiPassDqt;
                    trickSettings.mode_modifier = -1;
                }
                else if (!strcasecmp(trick, "mdqtip")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayMultiPassDqtIP;
                    trickSettings.mode_modifier = -1;
                }
                else if (!strcasecmp(trick, "bcm")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayBrcm;
                }
                else {
                    BDBG_WRN(("unknown trick mode: %s", trick));
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_eNone;
                }
                if (modifier < 0 && decoder_slowrate > 0) decoder_slowrate *= -1;
                trickSettings.rate = NEXUS_NORMAL_DECODE_RATE / decoder_slowrate;
                if (decoder_slowrate && !trickSettings.rate) {
                    trickSettings.rate = 1;
                }
                NEXUS_Playback_TrickMode(playback, &trickSettings);
            }
            else if (strstr(buf, "hostrate(") == buf) {
                char *s;
                char trick[64];
                float rate;
                int n;
                NEXUS_PlaybackTrickModeSettings trickSettings;

                /* convert , and ) into spaces for sscanf */
                while ((s = strpbrk(buf, ",)"))) {
                    *s = ' ';
                }

                NEXUS_Playback_GetDefaultTrickModeSettings(&trickSettings);

                n = sscanf(buf+9, "%f %s", &rate, trick);
                if (n < 2) continue;

                BDBG_WRN(("hostrate(%0.1f,%s)", rate, trick));

                trickSettings.rate = NEXUS_NORMAL_DECODE_RATE * rate;
                if (!trickSettings.rate && rate) {
                    trickSettings.rate = rate>0?1:-1;
                }
                if(opts.maxDecoderRate) {
                    trickSettings.maxDecoderRate = opts.maxDecoderRate;
                }

                trickSettings.skipControl = NEXUS_PlaybackSkipControl_eHost;
                trickSettings.rateControl = NEXUS_PlaybackRateControl_eStream;

                if (!strcasecmp(trick, "ip")) {
                    if (trickSettings.rate < 0) {
                        BDBG_ERR(("invalid host trick mode for reverse rate"));
                    }
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayIP;
                }
                else if (!strcasecmp(trick, "i")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayI;
                    trickSettings.mode_modifier = (trickSettings.rate < 0)?-1:1;
                }
                else if (!strcasecmp(trick, "dqt") || !strcasecmp(trick, "gop")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayDqt;
                    trickSettings.mode_modifier = (trickSettings.rate < 0)?-10:10;
                }
                else if (!strcasecmp(trick, "dqtip") || !strcasecmp(trick, "gopip")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayDqtIP;
                    trickSettings.mode_modifier = (trickSettings.rate < 0)?-10:10;
                }
                else if (!strcasecmp(trick, "mdqt")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayMultiPassDqt;
                    trickSettings.mode_modifier = -1;
                }
                else if (!strcasecmp(trick, "mdqtip")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayMultiPassDqtIP;
                    trickSettings.mode_modifier = -1;
                }
                else if (!strcasecmp(trick, "bcm")) {
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_ePlayBrcm;
                    trickSettings.mode_modifier = trickSettings.rate/NEXUS_NORMAL_DECODE_RATE;
                }
                else {
                    if (trickSettings.rate < 0) {
                        BDBG_ERR(("invalid host trick mode for reverse rate"));
                    }
                    trickSettings.mode = NEXUS_PlaybackHostTrickMode_eNormal;
                }
                NEXUS_Playback_TrickMode(playback, &trickSettings);
            }
            else if (strstr(buf, "seek(") == buf) {
                unsigned pos;
                unsigned min,sec,msec;
                NEXUS_PlaybackStatus status;

                if(sscanf(buf+5,"%u:%u.%u", &min, &sec, &msec)==3) {
                    pos = (min*60+sec)*1000+msec;
                } if(sscanf(buf+5,"%u:%u", &min, &sec)==2) {
                    pos = (min*60+sec)*1000;
                } else {
                    sscanf(buf+5, "%u", &pos);
                }

                rc = NEXUS_Playback_GetStatus(playback, &status);
                if (!rc) {
                    if (pos < status.first) {
                        pos = status.first;
                    }
                    else if (status.last > 0 && pos > status.last) {
                        pos = status.last;
                    }
                }

                rc = NEXUS_Playback_Seek(playback, pos);
                BDBG_ASSERT(!rc); /* because we've bounded it, it should not fail */
            }
            else if (!strcmp(buf, "st")) {
                NEXUS_VideoDecoderStatus vstatus;
                NEXUS_AudioDecoderStatus astatus;
                NEXUS_AudioDecoderStatus acstatus;
                NEXUS_PlaybackStatus pstatus;
                NEXUS_PlaypumpStatus pumpstatus;
                uint32_t stc;

                if(opts.common.decodedVideo) {
                    rc = NEXUS_VideoDecoder_GetStatus(videoDecoder, &vstatus);
                    BDBG_ASSERT(!rc);
                }
#if NEXUS_HAS_AUDIO
                if(opts.common.decodedAudio) {
                    rc = NEXUS_AudioDecoder_GetStatus(audioDecoder, &astatus);
                    BDBG_ASSERT(!rc);
                }
                if(compressedDecoder) {
                    rc = NEXUS_AudioDecoder_GetStatus(compressedDecoder, &acstatus);
                    BDBG_ASSERT(!rc);
                }
#else
                BKNI_Memset(&astatus, 0, sizeof(astatus));
                BKNI_Memset(&acstatus, 0, sizeof(acstatus));
#endif

                rc = NEXUS_Playback_GetStatus(playback, &pstatus);
                BDBG_ASSERT(!rc);
                rc = NEXUS_Playpump_GetStatus(playpump, &pumpstatus);
                NEXUS_StcChannel_GetStc(stcChannel, &stc);

                if (opts.common.decodedVideo && opts.common.videoPid) {
                    char enhancementInfo[64];
                    NEXUS_VideoDecoder3DTVStatus s3DTVStatus;
                    *enhancementInfo='\0';
                    if(vstatus.enhancementFifoSize) {
                        snprintf(enhancementInfo, sizeof(enhancementInfo),  " [%u/%u (%u%%)]",
                                vstatus.enhancementFifoDepth, vstatus.enhancementFifoSize,
                                vstatus.enhancementFifoDepth * 100 / vstatus.enhancementFifoSize);
                    }

                    printf("video %u/%u (%u%%)%s pts=%#x, stc=%#x (diff %d) fps=%sHz, queueDepth=%d %uKbps, profile %s, level %s\n", vstatus.fifoDepth, vstatus.fifoSize,
                        vstatus.fifoSize ? vstatus.fifoDepth * 100 / vstatus.fifoSize : 0,
                        enhancementInfo,
                        vstatus.pts, stc, vstatus.ptsStcDifference,
                        lookup_name(g_videoFrameRateStrs, vstatus.frameRate),
                        vstatus.queueDepth,
                        (decoder_bitrate_video(&video_bitrate, &vstatus) + 500)/1000,
                        lookup_name(g_videoCodecProfileStrs, vstatus.protocolProfile),
                        lookup_name(g_videoCodecLevelStrs, vstatus.protocolLevel));

                    rc = NEXUS_VideoDecoder_Get3DTVStatus(videoDecoder, &s3DTVStatus);
                    BDBG_ASSERT(!rc);
                    if (s3DTVStatus.format != NEXUS_VideoDecoder3DTVFormat_eNone) {
                        static const char *s_3dtvFormat[NEXUS_VideoDecoder3DTVFormat_eMax] = {"eNone","eSideBySideHalf","eTopAndBottomHalf","eFramePacking"};
                        switch (s3DTVStatus.codec) {
                        case NEXUS_VideoCodec_eMpeg2:
                            if (s3DTVStatus.codecData.mpeg2.valid) {
                                printf("3dtv %s MPEG: stereoVideoFormatSignalingType %x\n",
                                    s_3dtvFormat[s3DTVStatus.format],
                                    s3DTVStatus.codecData.mpeg2.jp3d.stereoVideoFormatSignalingType);
                                break;
                            }
                            /* fall through */
                        case NEXUS_VideoCodec_eH264:
                            if (s3DTVStatus.codecData.avc.valid) {
                                printf("3dtv %s AVC: %08x %d %d %d %d,%d %d,%d %d %d\n",
                                    s_3dtvFormat[s3DTVStatus.format],
                                    s3DTVStatus.codecData.avc.data.flags,
                                    s3DTVStatus.codecData.avc.data.framePackingArrangementId,
                                    s3DTVStatus.codecData.avc.data.framePackingArrangementType,
                                    s3DTVStatus.codecData.avc.data.contentInterpretationType,
                                    s3DTVStatus.codecData.avc.data.frame0GridPositionX,
                                    s3DTVStatus.codecData.avc.data.frame0GridPositionY,
                                    s3DTVStatus.codecData.avc.data.frame1GridPositionX,
                                    s3DTVStatus.codecData.avc.data.frame1GridPositionY,
                                    s3DTVStatus.codecData.avc.data.framePackingArrangementReservedByte,
                                    s3DTVStatus.codecData.avc.data.framePackingArrangementRepetitionPeriod);
                                break;
                            }
                            /* fall through */
                        default:
                            printf("3dtv %s: no valid metadata\n", s_3dtvFormat[s3DTVStatus.format]);
                            break;
                        }
                    }
                    /* else { printf("no 3dtv metadata\n"); } */
                }
                if (opts.common.audioPid) {
                    if(opts.common.decodedAudio) {
                        printf("audio %u/%u (%u%%) pts=%#x, stc=%#x (diff %d), queuedFrames=%d %uKbps\n", astatus.fifoDepth, astatus.fifoSize,
                            astatus.fifoSize ? astatus.fifoDepth * 100 / astatus.fifoSize : 0,
                            astatus.pts, stc, astatus.ptsStcDifference,
                            astatus.queuedFrames,
                            (decoder_bitrate_audio(&audio_bitrate, &astatus) + 500)/1000
                            );
                    }
                    if(compressedDecoder) {
                        printf("compressed %u/%u (%u%%) pts=%#x, stc=%#x (diff %d), queuedFrames=%d\n", acstatus.fifoDepth, acstatus.fifoSize,
                            acstatus.fifoSize ? acstatus.fifoDepth * 100 / acstatus.fifoSize : 0,
                            acstatus.pts, stc, acstatus.ptsStcDifference,
                            astatus.queuedFrames);
                    }
                }
                printf("playback %u/%u (%u%%) pos=%u:%02u.%03u(%d:%d) last=%u:%02u.%03u\n", pstatus.fifoDepth, pstatus.fifoSize,
                    pstatus.fifoSize ? pstatus.fifoDepth * 100 / pstatus.fifoSize : 0,
                    (unsigned)pstatus.position/60000,
                    (unsigned)(pstatus.position%60000)/1000,
                    (unsigned)pstatus.position%1000,
                    (int)(pstatus.readPosition - pstatus.position),
                    (int)(pumpstatus.mediaPts - (opts.common.videoPid?vstatus.pts:astatus.pts))/45,
                    (unsigned)pstatus.last/60000,
                    (unsigned)(pstatus.last%60000)/1000,
                    (unsigned)pstatus.last%1000
                    );
            }
#if NEXUS_HAS_AUDIO
            else if (!strcmp(buf, "af")) {
                NEXUS_AudioDecoder_Flush(audioDecoder);
                if(compressedDecoder) {
                    NEXUS_AudioDecoder_Flush(compressedDecoder);
                }
            }
#endif
            else if (!strcmp(buf, "restart")) {
                NEXUS_PlaybackStatus status;
                NEXUS_VideoDecoderSettings videoDecoderSettings;

                /* get current playback position */
                rc = NEXUS_Playback_GetStatus(playback, &status);
                BDBG_ASSERT(!rc);
                NEXUS_Playback_Stop(playback);

                if (opts.common.videoPid) {
                    /* don't show black frame */
                    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
                    videoDecoderSettings.channelChangeMode = NEXUS_VideoDecoder_ChannelChangeMode_eHoldUntilTsmLock;
                    rc=NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
                    BDBG_ASSERT(!rc);
                }
                /* stop decoder */
                stop_video(&opts, videoDecoder);
                stop_audio(&opts, audioDecoder, compressedDecoder);
                decoder_bitrate_control_video(&video_bitrate, videoDecoder, false);
                decoder_bitrate_control_audio(&audio_bitrate, audioDecoder, false);

                NEXUS_Playback_GetSettings(playback, &playbackSettings);
                playbackSettings.startPaused = true;
                rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
                BDBG_ASSERT(!rc);

                /* Start decoders */
                start_video(&opts, videoDecoder, &videoProgram);
                start_audio(&opts, audioDecoder, compressedDecoder, &audioProgram);
                decoder_bitrate_control_video(&video_bitrate, videoDecoder, true);
                decoder_bitrate_control_audio(&audio_bitrate, audioDecoder, true);

                /* start playback  */
                rc = NEXUS_Playback_Start(playback, file, &playbackStartSettings);
                BDBG_ASSERT(!rc);

                /* seek into desired location */
                rc = NEXUS_Playback_Seek(playback, status.position);
                BDBG_ASSERT(!rc);

                /* start playing */
                rc = NEXUS_Playback_Play(playback);
                BDBG_ASSERT(!rc);
            }
            else if (customFile && strstr(buf, "error_index(")==buf) {
                FileIoCustomProbabilities probabilities;
                FileIoCustom_GetProbabilities(customFile, NULL, &probabilities);
                sscanf(buf+strlen("error_index("),"%u,%u,%u", &probabilities.error, &probabilities.nodata, &probabilities.partial_read);
                FileIoCustom_SetProbabilities(customFile, NULL, &probabilities);
            }
            else if (customFile && strstr(buf, "error_data(")==buf) {
                FileIoCustomProbabilities probabilities;
                FileIoCustom_GetProbabilities(customFile , &probabilities, NULL);
                sscanf(buf+strlen("error_data("),"%u,%u,%u", &probabilities.error, &probabilities.nodata, &probabilities.partial_read);
                FileIoCustom_SetProbabilities(customFile, &probabilities, NULL);
            }
            else if (!strcmp(buf, "heaps")) {
                unsigned i;
                for (i=0;i<NEXUS_MAX_HEAPS;i++) {
                    if (platformConfig.heap[i]) {
                        BDBG_LOG(("heap[%d] %p", i, (void *)platformConfig.heap[i]));
                        NEXUS_Heap_Dump(platformConfig.heap[i]);
                    }
                }
            }
            else if (!strcmp(buf, "4k")) {
                NEXUS_HdmiOutputSettings settings;
                NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &settings);
                if(settings.outputFormat == NEXUS_VideoFormat_eUnknown)
                {
                    settings.outputFormat =
                        (displaySettings.format == NEXUS_VideoFormat_e1080p24hz) ? NEXUS_VideoFormat_e3840x2160p24hz :
                        (displaySettings.format == NEXUS_VideoFormat_e1080p25hz) ? NEXUS_VideoFormat_e3840x2160p25hz :
                        (displaySettings.format == NEXUS_VideoFormat_e1080p30hz) ? NEXUS_VideoFormat_e3840x2160p30hz :
                        (displaySettings.format == NEXUS_VideoFormat_e1080p50hz) ? NEXUS_VideoFormat_e3840x2160p50hz :
                        (displaySettings.format == NEXUS_VideoFormat_e1080p60hz) ? NEXUS_VideoFormat_e3840x2160p60hz :
                        displaySettings.format;
                }
                else
                    settings.outputFormat = NEXUS_VideoFormat_eUnknown;
                printf("HDMI format: %s\n\n",
                    (settings.outputFormat == NEXUS_VideoFormat_eUnknown) ? "Unknown" :
                    (settings.outputFormat == displaySettings.format) ? "display_format" : "4k");
                NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &settings);
            }
            else if (strstr(buf, "sharpness(") == buf) {
                char *s;
                int gain, enable;
                int n;
                NEXUS_PictureCtrlCommonSettings settings;

                /* convert , and ) into spaces for sscanf */
                while ((s = strpbrk(buf, ",)"))) {
                    *s = ' ';
                }

                n = sscanf(buf+10, "%d %d", &enable, &gain);
                if (n < 2) continue;

                BDBG_WRN(("sharpness(%s, %d)", (enable) ? "Enable" : "Disable", gain));
                NEXUS_PictureCtrl_GetCommonSettings(window, &settings);
                settings.sharpnessEnable = (enable) ? true : false;
                settings.sharpness = gain;
                NEXUS_PictureCtrl_SetCommonSettings(window, &settings);
            }
            else if (strstr(buf, "anr") == buf) {
                NEXUS_VideoWindowAnrSettings anrSettings;
                NEXUS_VideoWindow_GetAnrSettings(window, &anrSettings);
                if (strstr(buf, "anr(") != buf) { /* backward compat */
                    anrSettings.anr.mode = (NEXUS_VideoWindowFilterMode_eEnable == anrSettings.anr.mode)
                        ? NEXUS_VideoWindowFilterMode_eDisable : NEXUS_VideoWindowFilterMode_eEnable;
                }
                else if (!strcasecmp(buf+4, "off")) {
                    anrSettings.anr.mode = NEXUS_VideoWindowFilterMode_eDisable;
                }
                else {
                    anrSettings.anr.mode = NEXUS_VideoWindowFilterMode_eEnable;
                    anrSettings.anr.level = atoi(buf+4);
                }
                rc = NEXUS_VideoWindow_SetAnrSettings(window, &anrSettings);
                BERR_TRACE(rc);
                printf("anr is %s, level %d\n", (NEXUS_VideoWindowFilterMode_eEnable == anrSettings.anr.mode) ? "on" : "off",
                    anrSettings.anr.level);
            }
            else if (strstr(buf, "dnr") == buf) {
                NEXUS_VideoWindowDnrSettings dnrSettings;
                NEXUS_VideoWindow_GetDnrSettings(window, &dnrSettings);
                dnrSettings.mnr.mode = (NEXUS_VideoWindowFilterMode_eEnable == dnrSettings.mnr.mode)
                    ? NEXUS_VideoWindowFilterMode_eDisable : NEXUS_VideoWindowFilterMode_eEnable;
                dnrSettings.bnr.mode = (NEXUS_VideoWindowFilterMode_eEnable == dnrSettings.bnr.mode)
                    ? NEXUS_VideoWindowFilterMode_eDisable : NEXUS_VideoWindowFilterMode_eEnable;
                dnrSettings.dcr.mode = (NEXUS_VideoWindowFilterMode_eEnable == dnrSettings.dcr.mode)
                    ? NEXUS_VideoWindowFilterMode_eDisable : NEXUS_VideoWindowFilterMode_eEnable;
                rc = NEXUS_VideoWindow_SetDnrSettings(window, &dnrSettings);
                printf("dnr is %s\n", (NEXUS_VideoWindowFilterMode_eEnable == dnrSettings.mnr.mode)
                    ? "on" : "off");
            }
            else if (strstr(buf, "mad") == buf) {
                NEXUS_VideoWindowMadSettings madSettings;
                NEXUS_VideoWindow_GetMadSettings(window, &madSettings);
                madSettings.deinterlace = madSettings.deinterlace ? false : true;
                NEXUS_VideoWindow_SetMadSettings(window, &madSettings);
                printf("mad is %s\n", (madSettings.deinterlace) ? "on" : "off");
            }
            else if (strstr(buf, "sci(") == buf) {
                uint32_t ulIdx = 0;
                NEXUS_VideoWindowCoefficientIndexSettings sciSettings;
                sscanf(buf+4, "%u", &ulIdx);
                NEXUS_VideoWindow_GetCoefficientIndexSettings(window, &sciSettings);
                sciSettings.sclVertLuma   = sciSettings.sclHorzLuma   =
                sciSettings.sclVertChroma = sciSettings.sclHorzChroma = ulIdx;
                NEXUS_VideoWindow_SetCoefficientIndexSettings(window, &sciSettings);
                if (windowSD) {
                    NEXUS_VideoWindow_SetCoefficientIndexSettings(windowSD, &sciSettings);
                }
                printf("scaler coefficient index is %d\n", sciSettings.sclVertLuma);
            }
            else if (strstr(buf, "cap") == buf) {
                NEXUS_VideoWindow_GetSettings(window, &windowSettings);
                windowSettings.forceCapture ^= true;
                rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
                printf("forceCapture is %s\n", (windowSettings.forceCapture) ? "on" : "off");
                BDBG_ASSERT(!rc);
            }
            else if (strstr(buf, "automaster") == buf) {
                NEXUS_VideoWindow_GetSettings(window, &windowSettings);
                windowSettings.autoMaster ^= true;
                rc = NEXUS_VideoWindow_SetSettings(window, &windowSettings);
                printf("autoMaster is %s\n", (windowSettings.autoMaster) ? "on" : "off");
                BDBG_ASSERT(!rc);
            }
            else if (strstr(buf, "dropframe(") == buf) {
                NEXUS_DisplaySettings displaySettings;
                NEXUS_Display_GetSettings(display, &displaySettings);
                if (!strcmp(buf, "dropframe(0)")) {
                    displaySettings.dropFrame = NEXUS_TristateEnable_eDisable;
                }
                else if (!strcmp(buf, "dropframe(1)")) {
                    displaySettings.dropFrame = NEXUS_TristateEnable_eEnable;
                }
                else {  /* auto */
                    displaySettings.dropFrame = NEXUS_TristateEnable_eNotSet;
                }
                rc = NEXUS_Display_SetSettings(display, &displaySettings);
                printf("dropFrame is %s\n",
                   (NEXUS_TristateEnable_eNotSet == displaySettings.dropFrame) ? "auto" :
                   (NEXUS_TristateEnable_eEnable == displaySettings.dropFrame) ? "on"  : "off");
                BDBG_ASSERT(!rc);
            }
            else if (strstr(buf, "hdmi_colorspace(") == buf) {
                NEXUS_HdmiOutputSettings settings;
                NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &settings);
                if (!strcmp(buf, "hdmi_colorspace(rgb)")) {
                    settings.colorSpace = NEXUS_ColorSpace_eRgb;
                }
                else if (!strcmp(buf, "hdmi_colorspace(420)")) {
                    settings.colorSpace = NEXUS_ColorSpace_eYCbCr420;
                }
                else if (!strcmp(buf, "hdmi_colorspace(422)")) {
                    settings.colorSpace = NEXUS_ColorSpace_eYCbCr422;
                }
                else if (!strcmp(buf, "hdmi_colorspace(444)")) {
                    settings.colorSpace = NEXUS_ColorSpace_eYCbCr444;
                }
                NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &settings);
            }
            else if (strstr(buf, "hdmi_colorrange(") == buf) {
                NEXUS_HdmiOutputSettings settings;
                NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &settings);
                if (!strcmp(buf, "hdmi_colorrange(auto)")) {
                    settings.overrideColorRange = false;
                }
                else if (!strcmp(buf, "hdmi_colorrange(limited)")) {
                    settings.overrideColorRange = true;
                    settings.colorRange = NEXUS_ColorRange_eLimited;
                }
                else if (!strcmp(buf, "hdmi_colorrange(full)")) {
                    settings.overrideColorRange = true;
                    settings.colorRange = NEXUS_ColorRange_eFull;
                }
                NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &settings);
            }
            else if (strstr(buf, "hdmi_matrixcoefficients(") == buf) {
                NEXUS_HdmiOutputSettings settings;
                NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &settings);
                if (!strcmp(buf, "hdmi_matrixcoefficients(auto)")) {
                    settings.overrideMatrixCoefficients = false;
                }
                else if (!strcmp(buf, "hdmi_matrixcoefficients(709)")) {
                    settings.overrideMatrixCoefficients = true;
                    settings.matrixCoefficients = NEXUS_MatrixCoefficients_eItu_R_BT_709;
                }
                else if (!strcmp(buf, "hdmi_matrixcoefficients(170m)")) {
                    settings.overrideMatrixCoefficients = true;
                    settings.matrixCoefficients = NEXUS_MatrixCoefficients_eSmpte_170M;
                }
                else if (!strcmp(buf, "hdmi_matrixcoefficients(470_2_bg)")) {
                    settings.overrideMatrixCoefficients = true;
                    settings.matrixCoefficients = NEXUS_MatrixCoefficients_eItu_R_BT_470_2_BG;
                }
                else if (!strcmp(buf, "hdmi_matrixcoefficients(2020ncl)")) {
                    settings.overrideMatrixCoefficients = true;
                    settings.matrixCoefficients = NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL;
                }
                else if (!strcmp(buf, "hdmi_matrixcoefficients(2020cl)")) {
                    settings.overrideMatrixCoefficients = true;
                    settings.matrixCoefficients = NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL;
                }
                NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &settings);
            }
            else if (strstr(buf, "hdmi_colordepth(") == buf) {
                NEXUS_HdmiOutputSettings settings;
                    NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &settings);
                    if (!strcmp(buf, "hdmi_colordepth(0)")) {
                        settings.colorDepth = 0;
                    }
                    else if (!strcmp(buf, "hdmi_colordepth(8)")) {
                        settings.colorDepth = 8;
                    }
                    else if (!strcmp(buf, "hdmi_colordepth(10)")) {
                        settings.colorDepth = 10;
                    }
                    else if (!strcmp(buf, "hdmi_colordepth(12)")) {
                        settings.colorDepth = 12;
                    }
                    else {
                        BDBG_WRN(("Unknown colordepth; No Change")) ;
                        break ;
                    }
                NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &settings);
            }
            else if (strstr(buf, "gfx_sdr2hdr(") == buf) {
                int yScl = 0, cbScl = 0, crScl = 0;
                NEXUS_GraphicsSettings graphicsSettings;
                NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
                sscanf(buf+strlen("gfx_sdr2hdr("),"%d,%d,%d", &yScl, &cbScl, &crScl);
                graphicsSettings.sdrToHdr.y = (int16_t) yScl;
                graphicsSettings.sdrToHdr.cb = (int16_t) cbScl;
                graphicsSettings.sdrToHdr.cr = (int16_t) crScl;
                rc = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
                BDBG_ASSERT(!rc);
            }
            else if (strstr(buf, "hdmi_matrix") == buf) {
                NEXUS_HdmiOutputHandle hdmiOutput ;
                NEXUS_HdmiOutputSettings hdmiOutputSettings;
                NEXUS_HdmiOutputStatus hdmiOutputStatus ;
                bool bt2020nclYccSupport ;

                hdmiOutput = platformConfig.outputs.hdmi[0] ;

                NEXUS_HdmiOutput_GetStatus(hdmiOutput, &hdmiOutputStatus) ;
                bt2020nclYccSupport = hdmiOutputStatus.monitorColorimetry.extendedColorimetrySupported[NEXUS_HdmiEdidColorimetryDbSupport_eBT2020YCC] ;


                NEXUS_HdmiOutput_GetSettings(hdmiOutput, &hdmiOutputSettings) ;
                    hdmiOutputSettings.overrideMatrixCoefficients = true ;

                    if (!strcmp(buf, "hdmi_matrix(0)")) {
                        hdmiOutputSettings.overrideMatrixCoefficients = false  ;
                        BDBG_LOG(("MatrixCoefficients selected based on format")) ;
                    }
                    else if (!strcmp(buf, "hdmi_matrix(2020ncl)")) {
                        if (!bt2020nclYccSupport) {
                            BDBG_WRN(("HDMI Rx <%s> does not support BT2020 NCL", hdmiOutputStatus.monitorName)) ;
                        }
                        hdmiOutputSettings.matrixCoefficients = NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL ;
                    }
                    else if (!strcmp(buf, "hdmi_matrix(2020cl)")) {
                        if (!bt2020nclYccSupport) {
                            BDBG_WRN(("HDMI Rx <%s> does not support BT2020 CL", hdmiOutputStatus.monitorName)) ;
                        }
                        hdmiOutputSettings.matrixCoefficients = NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL ;
                    }
                    else if (!strcmp(buf, "hdmi_matrix(full-ycc)")) {
                        hdmiOutputSettings.matrixCoefficients = NEXUS_MatrixCoefficients_eHdmi_Full_Range_YCbCr ;
                    }
                    else if (!strcmp(buf, "hdmi_matrix(full-rgb)")) {
                        hdmiOutputSettings.matrixCoefficients = NEXUS_MatrixCoefficients_eDvi_Full_Range_RGB ;
                    }
                    else  {
                        BDBG_WRN(("Unknown matrix; no change")) ;
                        break ;
                    }
                NEXUS_HdmiOutput_SetSettings(hdmiOutput, &hdmiOutputSettings) ;
            }
            else if (strstr(buf, "hdmi_rescramble") == buf) {
                NEXUS_HdmiOutput_ResetScrambling(platformConfig.outputs.hdmi[0]) ;
            }
            else if (strstr(buf, "hdmi_edid") == buf) {
                NEXUS_HdmiOutput_DisplayRxEdid(platformConfig.outputs.hdmi[0]) ;
            }
#if NEXUS_HAS_HDMI_OUTPUT
            else if (strstr(buf, "hdmi_eotf") == buf) {
                hdmi_drm_do_cmd(&hdmiDrmContext, buf);
            }
            else if (strstr(buf, "hdmi_dolby") == buf) {
                NEXUS_HdmiOutputExtraSettings settings;
                NEXUS_HdmiOutput_GetExtraSettings(platformConfig.outputs.hdmi[0], &settings);
                if (!strcmp(buf, "hdmi_dolby(on)")) {
                    settings.dolbyVision.outputMode = NEXUS_HdmiOutputDolbyVisionMode_eEnabled;
                } else if (!strcmp(buf, "hdmi_dolby(off)")) {
                    settings.dolbyVision.outputMode = NEXUS_HdmiOutputDolbyVisionMode_eDisabled;
                }
                NEXUS_HdmiOutput_SetExtraSettings(platformConfig.outputs.hdmi[0], &settings);
            }
#endif
            else if (strstr(buf, "display_format") == buf) {
                NEXUS_HdmiOutputStatus hdmiOutputStatus ;

                NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiOutputStatus) ;
                NEXUS_Display_GetSettings(display, &displaySettings);
                displaySettings.format = lookup(g_videoFormatStrs, buf+strlen("display_format "));

                if ((!opts.common.ignore_edid)
                &&  (!hdmiOutputStatus.videoFormatSupported[displaySettings.format]))
                {
                    BDBG_ERR(("HDMI Rx <%s> does not support %s; use cmdline option -ignore_edid to force",
                        hdmiOutputStatus.monitorName, lookup_name(g_videoFormatStrs, displaySettings.format))) ;
                    break ;
                }

                rc = NEXUS_Display_SetSettings(display, &displaySettings);
                if (rc) {
                    BDBG_WRN(("Unable to modify Nexus DisplaySettings")) ;
                }
                else if(displaySD) {
                    NEXUS_VideoFormatInfo videoFormatInfo;
                    NEXUS_DisplaySettings compositeDisplaySettings;
                    NEXUS_VideoFormat_GetInfo(displaySettings.format, &videoFormatInfo);
                    NEXUS_Display_GetSettings(displaySD, &compositeDisplaySettings);
                    compositeDisplaySettings.format = (videoFormatInfo.verticalFreq == 5000)
                        ? NEXUS_VideoFormat_ePal : NEXUS_VideoFormat_eNtsc;
                    rc = NEXUS_Display_SetSettings(displaySD, &compositeDisplaySettings);
                    if (rc) {
                        BDBG_WRN(("Unable to modify Nexus DisplaySettings")) ;
                    }
                }
            }
#if NEXUS_HAS_AUDIO
            else if (strstr(buf, "ac4") == buf) {
                NEXUS_AudioDecoderStatus audStatus;
                NEXUS_AudioDecoder_GetStatus(audioDecoder, &audStatus);

                BDBG_ERR(("Codec is %lu", (unsigned long)audStatus.codec));
                if ( audStatus.codec == NEXUS_AudioCodec_eAc4 ) {
                    unsigned i;
                    BDBG_ERR(("numPresentations %lu", (unsigned long)audStatus.codecStatus.ac4.numPresentations));
                    BDBG_ERR(("currentPresentationIndex %lu", (unsigned long)audStatus.codecStatus.ac4.currentPresentationIndex));
                    for (i = 0; i<audStatus.codecStatus.ac4.numPresentations; i++) {
                        NEXUS_AudioDecoderPresentationStatus presentStatus;
                        NEXUS_AudioDecoder_GetPresentationStatus(audioDecoder, i, &presentStatus);
                        if ( presentStatus.codec != audStatus.codec ) {
                            BDBG_ERR(("Something went wrong. Presentation Status Codec doesn't match the current decode codec."));
                        }
                        else {
                            BDBG_ERR(("Presentation %lu index: %lu", (unsigned long)i, (unsigned long)presentStatus.status.ac4.index));
                            BDBG_ERR(("  Presentation %lu id: %s", (unsigned long)i, presentStatus.status.ac4.id));
                            BDBG_ERR(("  Presentation %lu name: %s", (unsigned long)i, presentStatus.status.ac4.name));
                            BDBG_ERR(("  Presentation %lu language: %s", (unsigned long)i, presentStatus.status.ac4.language));
                            BDBG_ERR(("  Presentation %lu associateType: %lu", (unsigned long)i, (unsigned long)presentStatus.status.ac4.associateType));
                        }
                    }
                    BDBG_ERR(("Dialog Enhancer Max %lu", (unsigned long)audStatus.codecStatus.ac4.dialogEnhanceMax));
                }
            }
            else if (strstr(buf, "apres(") == buf) {
                NEXUS_AudioDecoderStatus audStatus;
                NEXUS_AudioDecoderCodecSettings codecSettings;
                unsigned presentationIndex;

                sscanf(buf+6, "%u", &presentationIndex);
                NEXUS_AudioDecoder_GetStatus(audioDecoder, &audStatus);

                BDBG_ERR(("Codec is %lu", (unsigned long)audStatus.codec));
                if ( audStatus.codec == NEXUS_AudioCodec_eAc4 )
                {
                    BDBG_ERR(("Selecting Presentation id %lu", (unsigned long)presentationIndex));
                    NEXUS_AudioDecoder_GetCodecSettings(audioDecoder, audStatus.codec, &codecSettings);
                    codecSettings.codecSettings.ac4.programs[NEXUS_AudioDecoderAc4Program_eMain].presentationIndex = presentationIndex;
                    NEXUS_AudioDecoder_SetCodecSettings(audioDecoder, &codecSettings);
                }
            }
            else if (strstr(buf, "ade(") == buf) {
                NEXUS_AudioDecoderStatus audStatus;
                NEXUS_AudioDecoderCodecSettings codecSettings;
                int dialogEnhancerAmount;

                sscanf(buf+4, "%d", &dialogEnhancerAmount);
                NEXUS_AudioDecoder_GetStatus(audioDecoder, &audStatus);

                BDBG_ERR(("Codec is %lu", (unsigned long)audStatus.codec));
                if ( audStatus.codec == NEXUS_AudioCodec_eAc4 )
                {
                    BDBG_ERR(("Changing dialog enhancement level to %ld", (long int)dialogEnhancerAmount));
                    NEXUS_AudioDecoder_GetCodecSettings(audioDecoder, audStatus.codec, &codecSettings);
                    codecSettings.codecSettings.ac4.dialogEnhancerAmount = dialogEnhancerAmount;
                    NEXUS_AudioDecoder_SetCodecSettings(audioDecoder, &codecSettings);
                }
            }
#endif
            else if (!*buf) {
                /* allow blank line */
            }
            else {
                printf("unknown command: '%s' (use '?' for list)\n", buf);
            }
        }
        while ((buf = strtok(NULL, ";")));

#if B_HAS_PLAYBACK_MONITOR
        if(stickyFile) {
            BKNI_ReleaseMutex(monitorState.lock);
        }
#endif
    }

uninit:

#if B_HAS_PLAYBACK_MONITOR
    if(stickyFile) {
        monitor_thread_stop(&monitorState);
    }
    }
#endif

#if NEXUS_HAS_SYNC_CHANNEL
    if (sync) NEXUS_SyncChannel_Destroy(sync);
#endif
    NEXUS_Playback_Stop(playback);
    stop_video(&opts, videoDecoder);
    stop_audio(&opts, audioDecoder, compressedDecoder);

    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_Playback_CloseAllPidChannels(playback);
    NEXUS_FilePlay_Close(file);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);

    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display); /* must precede NEXUS_VideoDecoder_Close to auto-shutdown of inputs */

    if (displaySD) {
        if (windowSD) {
            NEXUS_VideoWindow_Close(windowSD);
        }
        NEXUS_Display_Close(displaySD);
    }

    NEXUS_VideoDecoder_Close(videoDecoder);
    if (framebuffer) {
        NEXUS_Surface_Destroy(framebuffer);
    }
#if NEXUS_HAS_AUDIO
    if(audioDecoder) {
        NEXUS_AudioDecoder_Close(audioDecoder);
    }
    if(compressedDecoder) {
        NEXUS_AudioDecoder_Close(compressedDecoder);
    }
#endif
    BKNI_DestroyEvent(endOfStreamEvent);
    NEXUS_Platform_Uninit();

    return 0;
}

static void print_cmp_crc(NEXUS_DisplayHandle display, BKNI_EventHandle endOfStreamEvent)
{
    for (;;) {
        NEXUS_DisplayCrcData data[16];
        unsigned num, i;
        int rc;

        rc = NEXUS_Display_GetCrcData(display, data, 16, &num);
        BDBG_ASSERT(!rc);
        if (!num) {
            if (endOfStreamEvent && !BKNI_WaitForEvent(endOfStreamEvent, 0)) break;
            BKNI_Sleep(10);
            continue;
        }

        for (i=0;i<num;i++) {
            printf("CMP CRC %x %x %x\n", data[i].cmp.luma, data[i].cmp.cb, data[i].cmp.cr);
        }
    }
}

static void print_avd_crc(NEXUS_VideoDecoderHandle videoDecoder, BKNI_EventHandle endOfStreamEvent)
{
    for (;;) {
        NEXUS_VideoDecoderCrc data[16];
        unsigned num, i;
        int rc;

        rc = NEXUS_VideoDecoder_GetCrcData(videoDecoder, data, 16, &num);
        BDBG_ASSERT(!rc);
        if (!num) {
            if (endOfStreamEvent && !BKNI_WaitForEvent(endOfStreamEvent, 0)) break;
            BKNI_Sleep(10);
            continue;
        }
        for (i=0;i<num;i++) {
            printf("AVD CRC %x %x %x; %x %x %x\n", data[i].top.luma, data[i].top.cr, data[i].top.cb, data[i].bottom.luma, data[i].bottom.cr, data[i].bottom.cb);
        }
    }
}

static void print_hdmi_crc(NEXUS_HdmiOutputHandle hdmiOutput, BKNI_EventHandle endOfStreamEvent)
{
    for (;;) {
        NEXUS_HdmiOutputCrcData data[16];
        unsigned num, i;
        int rc;

        rc = NEXUS_HdmiOutput_GetCrcData(hdmiOutput, data, 16, &num);
        BDBG_ASSERT(!rc);
        if (!num) {
            if (endOfStreamEvent && !BKNI_WaitForEvent(endOfStreamEvent, 0)) break;
            BKNI_Sleep(10);
            continue;
        }

        for (i=0;i<num;i++) {
            printf("HDMI CRC %x\n", data[i].crc);
        }
    }
}

static void print_mfd_crc(NEXUS_VideoDecoderHandle videoDecoder, BKNI_EventHandle endOfStreamEvent)
{
    for (;;) {
        NEXUS_VideoInputCrcData data[16];
        unsigned num, i;
        int rc;

        rc = NEXUS_VideoInput_GetCrcData(NEXUS_VideoDecoder_GetConnector(videoDecoder), data, sizeof(data)/sizeof(data[0]), &num);
        BDBG_ASSERT(!rc);
        if (!num) {
            if (endOfStreamEvent && !BKNI_WaitForEvent(endOfStreamEvent, 0)) break;
            BKNI_Sleep(10);
            continue;
        }

        for (i=0;i<num;i++) {
            printf("MFD CRC %u,%u %u,%u right=%u,%u, field=%c\n", data[i].idrPictureId, data[i].pictureOrderCount,
                data[i].crc[0], data[i].crc[1], data[i].crc[3], data[i].crc[4], data[i].isField?'y':'n');
        }
    }
}

#endif /*!NEXUS_HAS_PLAYBACK*/

/*
************************************************

examples / test cases

# basic decode of streamer input
nexus playback -video 0x31 -audio 0x34 -video_type mpeg -audio_type ac3 /mnt/hd/italyriviera_spiderman2_cc_q64.mpg

# playback using media probe
nexus playback /mnt/hd/cnnticker.mpg /mnt/hd/cnnticker.nav

************************************************
*/
