/******************************************************************************
 * Copyright (C) 2017-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef CMDLINE_ARGS_H__
#define CMDLINE_ARGS_H__

#include "nexus_types.h"
#if NEXUS_HAS_DISPLAY
#include "nexus_display.h"
#include "nexus_video_window.h"
#endif
#include "nexus_pid_channel.h"
#include "nexus_stc_channel.h"
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#endif
#include "bmedia_pcm.h"
#include "namevalue.h"

#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#endif

struct common_opts_t {
    NEXUS_TransportType transportType;
    unsigned videoPid, pcrPid, audioPid, extVideoPid;
    unsigned videoCdb;
    unsigned audioCdb;
    unsigned videoDecoder;
    NEXUS_VideoCodec videoCodec;
    NEXUS_VideoCodec extVideoCodec;
    NEXUS_AudioCodec audioCodec;
    NEXUS_VideoFormat displayFormat;
#if NEXUS_HAS_DISPLAY
    NEXUS_DisplayType displayType;
    NEXUS_DisplayAspectRatio displayAspectRatio;
    NEXUS_VideoWindowContentMode contentMode;
    NEXUS_VideoWindowAfdSettings afd;
    NEXUS_Display3DSourceBuffer displaySourceBuffer;
#endif
    NEXUS_TransportTimestampType tsTimestampType;
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderTimestampMode decoderTimestampMode;
    NEXUS_VideoDecoderSourceOrientation sourceOrientation;
#endif
    NEXUS_VideoFrameRate videoFrameRate;
    NEXUS_AspectRatio aspectRatio;
    NEXUS_VideoOrientation  displayOrientation;
    NEXUS_VideoDecoderScanMode scanMode;
    struct {
        unsigned x, y;
    } sampleAspectRatio;

    bool masterModeTimingGenerator;
    bool useCcir656Output;
    bool useCompositeOutput;
    bool useComponentOutput;
    bool useHdmiOutput;
    bool smoothSourceChange;
    bool compressedAudio;
    bool ignore_edid;
    bool multichannelAudio;
    bool decodedAudio;
    bool decodedVideo;
    bool probe;
    bool cdxaFile;
    bool dtsCd;
    bool detectAvcExtension;
    bool playpumpTimestampReordering;
    bool pcm;
    bool audioUseLittle;
    bool dolbyDrcModeRf;
    bpcm_file_config pcm_config;
    unsigned maxWidth, maxHeight;
    unsigned maxAudioRate;
    NEXUS_AudioLoudnessEquivalenceMode audioLoudnessMode;

    /* asf wma drc */
#if B_HAS_ASF
    bool dynamicRangeControlValid;
    struct {
        unsigned peakReference;
        unsigned peakTarget;
        unsigned averageReference;
        unsigned averageTarget;
    } dynamicRangeControl;
#endif
};

struct util_opts_t {
    struct common_opts_t common;
    const char *filename;
    const char *indexname;
    bool stcTrick;
    bool astm;
    bool sync;
    bool looseAudioTsm;
    bool streamProcessing;
    bool autoBitrate;
    bool closedCaptionEnabled;
    bool startPaused;
    bool avc51;
    bool graphics;
    bool gfx_bar;
    bool customFileIo;
    bool playbackMonitor;
    bool avd_crc;
    bool cmp_crc;
    bool hdmi_crc;
    bool mfd_crc;
    bool scriptRunning;
    enum {playback_mem_high, playback_mem_medium, playback_mem_low, playback_mem_minimal} mem;
    unsigned fixedBitrate; /* non-zero */
    NEXUS_StcChannelAutoModeBehavior stcChannelMaster;
#if NEXUS_HAS_PLAYBACK
    NEXUS_PlaybackLoopMode beginningOfStreamAction;
    NEXUS_PlaybackLoopMode endOfStreamAction;
    bool accurateSeek;
#endif
#if NEXUS_HAS_VIDEO_DECODER
    NEXUS_VideoDecoderErrorHandling videoErrorHandling;
#endif
    unsigned maxDecoderRate;
    unsigned maxPlaybackDataRate;
    unsigned maxLiveDataRate;
    unsigned timeout;
    unsigned playbackFifoSize;
    int playbackHeap;
};

#define MAX_RECORD_PIDS 16
struct util_opts_record_t {
    struct common_opts_t common;
    /* record options */
    unsigned short otherPids[MAX_RECORD_PIDS];
    unsigned numOtherPids;
    struct {
        unsigned bufferSize;
        unsigned dataReadyThreshold;
    } data, index;

    /* display */
    bool allpass;
    bool acceptNullPackets;
    bool decode;

    /* output */
    const char *recfname;
    const char *recidxname;

    /* source */
    bool streamer;
    const char *playfname;
    unsigned maxRecordDataRate;
#if NEXUS_HAS_HDMI_INPUT
    bool hdmiInput;
#endif
#if NEXUS_HAS_VIDEO_ENCODER
    bool encoder;
    NEXUS_VideoFormat encodeFormat;
    unsigned maxVideoBitRate;
    NEXUS_VideoFrameRate encodeFrameRate;
    bool variableFrameRate;
    unsigned gopFrameP;
    unsigned gopFrameB;
    NEXUS_VideoCodec videoTranscodec;
    NEXUS_VideoCodecProfile     videoProfile;
    NEXUS_VideoCodecLevel       videoLevel;
    NEXUS_AudioCodec audioTranscodec;
    bool                        audioEncode;
    unsigned                    audioBitRate;
    unsigned muxLatencyTolerance;
#endif

    bool scriptRunning;
    unsigned timeout;
};

extern const float g_frameRateValues[NEXUS_VideoFrameRate_eMax];
typedef int cmdline_parsefn(int *argc, const char *argv[], void *param);
typedef void cmdline_usagefn(void *param);
void cmdline_register_module(cmdline_parsefn *parsefn, cmdline_usagefn *usagefn, void *param);

/*
cmdline_parse should be called before NEXUS_Platform_Init
returns non-zero if app should exit
*/
int cmdline_parse(int argc, const char *argv[], struct util_opts_t *opts);
int cmdline_parse_record(int argc, const char *argv[], struct util_opts_record_t *opts);

/*
cmdline_probe should be called after NEXUS_Platform_Init
*/
int cmdline_probe(struct common_opts_t *opts, const char *filename, const char **indexname);

/*
print_usage can be called if cmdline_parse fails
*/
void print_usage(const char *app /* argv[0] */);
void print_usage_record(const char *app /* argv[0] */);

#endif
