/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#if NEXUS_HAS_INPUT_ROUTER && NEXUS_HAS_SIMPLE_DECODER && NEXUS_HAS_TRANSPORT
#include "nxclient.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_surface_client.h"
#include "nexus_platform.h"
#include "nexus_simple_video_decoder.h"
#include "nexus_simple_audio_decoder.h"
#include "nexus_parser_band.h"
#include "live_decode.h"
#include "live_source.h"
#include "tspsimgr3.h"
#include "namevalue.h"
#include "blst_queue.h"
#include "binput.h"
#include "nxapps_cmdline.h"
#include "namevalue.h"
#include "bgui.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

BDBG_MODULE(live);

enum gui {gui_no_alpha_hole, gui_on, gui_constellation};
static int gui_init(NEXUS_FrontendHandle frontend, NEXUS_SurfaceClientHandle surfaceClient, enum gui gui);
static void gui_uninit(void);

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
        "Usage: live\n"
        "  --help or -h for help\n"
        );
#if NEXUS_HAS_FRONTEND
    printf(
        "  -const                   show demod constellation\n"
        );
#endif
    nxapps_cmdline_print_usage(cmdline);
    printf(
    "  -program #               program number of first channel to start (default is 0)\n"
    "  -primers #               number of primers (default is 2)\n"
    "  -pip                     sets -rect and -zorder for picture-in-picture\n"
    "  -prompt\n"
    "  -timeout SECONDS\n"
    "  -gui no_alpha_hole\n"
    "  -pig                     example of picture-in-graphics video window sizing\n"
    );
    printf(
    "  -q                       don't print status\n"
    );
    printf(
    "  -max WIDTH,HEIGHT        max video decoder resolution\n"
    "  -colorDepth {8|10}\n"
    "  -scrtext SCRIPT          run --help-script to learn script commands\n"
    "  -script SCRIPTFILE       run --help-script to learn script commands\n"
    "  -smooth                  set smoothResolutionChange to disable scale factor rounding\n"
    "  -preroll X               display pre-TSM pictures X times to boost perceived channel change speed\n"
    );
    print_list_option("channelChange",g_channelChangeMode);
    printf(
    "  -video PID               override PSI scan. use 0 for no video. default video_type is MPEG.\n"
    );
    print_list_option(
    "  -video_type              override PSI scan",g_videoCodecStrs);
    printf(
    "  -audio PID               override PSI scan. use 0 for no audio. default audio_type is AC3.\n"
    );
    print_list_option(
    "  -audio_type              override PSI scan",g_audioCodecStrs);
    print_list_option(
    "  -format                  max source video format",g_videoFormatStrs);
    print_list_option(
    "  -ar                      source aspect ratio",g_contentModeStrs);
    print_list_option(
    "  -sync                    sync_channel mode",g_syncModeStrs);
}

struct channel_map {
    BLST_Q_ENTRY(channel_map) link;
    char name[32];
    struct btune_settings tune;
    bchannel_scan_t scan;
    enum { scan_state_pending, scan_state_started, scan_state_done } scan_state;
    tspsimgr_scan_results scan_results;
    unsigned scan_start;
};
static BLST_Q_HEAD(channellist_t, channel_map) g_channel_map = BLST_Q_INITIALIZER(g_channel_map);
static unsigned g_total_channels;

struct frontend {
    BLST_Q_ENTRY(frontend) link;
    NEXUS_FrontendHandle frontend;
    struct channel_map *map;
    unsigned refcnt;
};
static BLST_Q_HEAD(frontendlist_t, frontend) g_frontends;

struct decoder {
    BLST_Q_ENTRY(decoder) link;
    NEXUS_ParserBand parserBand;
    live_decode_channel_t channel;
    live_decode_start_settings start_settings;
    unsigned video_pid, audio_pid; /* scan override */
    NEXUS_VideoCodec video_type;
    NEXUS_AudioCodec audio_type;

    unsigned chNum; /* global number */
    struct frontend *frontend;
    struct channel_map *map;
    unsigned program; /* relative to map */
};
static BLST_Q_HEAD(decoderlist_t, decoder) g_decoders;
static bool g_quiet = false;

static struct {
    bool done;
    pthread_t thread;
    NEXUS_SurfaceClientHandle surfaceClient;
    NEXUS_SurfaceHandle surface;
    NEXUS_SurfaceCreateSettings createSettings;
    NEXUS_FrontendHandle frontend;
    BKNI_EventHandle displayedEvent;
} g_constellation;

struct client_state
{
    bool done;
};

static void complete2(void *data, int param)
{
    BSTD_UNUSED(param);
    /* race condition between unsetting the callback and destroying the event, so protect with a global */
    if (g_constellation.done) return;
    BKNI_SetEvent((BKNI_EventHandle)data);
}

static void insert_channel(const struct btune_settings *tune)
{
    struct channel_map *c = BKNI_Malloc(sizeof(*c));
    BKNI_Memset(c, 0, sizeof(*c));
    c->tune = *tune;
    bchannel_source_print(c->name, sizeof(c->name), tune);
    BLST_Q_INSERT_TAIL(&g_channel_map, c, link);
}

static void uninit_channels(void)
{
    struct channel_map *map;
    while ((map = BLST_Q_FIRST(&g_channel_map))) {
        BLST_Q_REMOVE(&g_channel_map, map, link);
        BKNI_Free(map);
    }
}

static void parse_channels(const struct btune_settings *tune, const char *freq_list)
{
#if NEXUS_HAS_FRONTEND
    unsigned first_channel = 0, last_channel = 0;
    const unsigned _mhz = 1000000;
#endif
    if (tune->source == channel_source_streamer) {
        insert_channel(tune);
        return;
    }
#if NEXUS_HAS_FRONTEND
    if (!strcmp(freq_list, "all")) {
        if (tune->source == channel_source_qam) {
            /* simple scheme doesn't include 79, 85 exceptions */
            first_channel = 57 * _mhz;
            last_channel = 999 * _mhz;
        }
        else if (tune->source == channel_source_ofdm) {
            /* simple scheme doesn't include 79, 85 exceptions */
            if((tune->mode == 0) || (tune->mode == 1))
                first_channel = 578 * _mhz;
            else
                first_channel = 473 * _mhz;
            last_channel = 602 * _mhz;
        }
        else {
            return;
        }
    }

    while (freq_list) {
        unsigned freq;
        if (first_channel) {
            freq = first_channel;
            if (freq > last_channel) break;
            if((tune->source == channel_source_ofdm) && ((tune->mode == 0) || (tune->mode == 1)))
                first_channel += 8 * _mhz;
            else
                first_channel += 6 * _mhz;
        }
        else {
            float f;
            if (sscanf(freq_list, "%f", &f) != 1) f = 0;
            if (f <  _mhz) {
                /* convert to Hz, rounding to nearest 1000 */
                freq = (unsigned)(f * 1000) * 1000;
            }
            else {
                freq = f;
            }
        }

        if (freq) {
            struct btune_settings t = *tune;
            t.freq = freq;
            insert_channel(&t);
        }
        if (!first_channel) {
            freq_list = strchr(freq_list, ',');
            if (freq_list) freq_list++;
        }
    }
#else
    BSTD_UNUSED(freq_list);
#endif
}

static void stop_decode(struct decoder *d)
{
    if (!d->channel) return;

    BDBG_MSG(("stop_decode(%p) %p frontend=%p(%d) map=%p program=%d", (void*)d, (void*)d->channel, d->frontend?(void*)d->frontend->frontend:NULL, d->frontend?d->frontend->refcnt:0, (void*)d->map, d->program));
    live_decode_stop_channel(d->channel);
    if (d->frontend) {
        BDBG_ASSERT(d->frontend->map);
        BDBG_ASSERT(d->frontend->refcnt);
        if (--d->frontend->refcnt == 0) {
            d->frontend->map = NULL;
        }
        d->frontend = NULL;
    }
}

static int set_channel(struct decoder *d, unsigned chNum)
{
    struct channel_map *map;
    if (chNum >= g_total_channels && g_total_channels) {
        chNum = chNum % g_total_channels;
    }
    d->chNum = chNum;
    for (map = BLST_Q_FIRST(&g_channel_map); map; map = BLST_Q_NEXT(map, link)) {
        if (chNum < map->scan_results.num_programs) break;
        chNum -= map->scan_results.num_programs;
    }
    if (!map) {
        d->map = NULL;
        return -1;
    }
    d->map = map;
    d->program = chNum;
    BDBG_MSG(("set_channel(%p,%d) %p %d", (void*)d, chNum, (void*)d->map, d->program));
    return 0;
}

static int start_priming(struct decoder *d)
{
    struct channel_map *map;
    struct frontend *frontend;
    int rc;

    if (!d->channel || !d->map) {
        return -1;
    }

    map = d->map;

    /* find frontend that's already tuned */
    for (frontend = BLST_Q_FIRST(&g_frontends); frontend; frontend = BLST_Q_NEXT(frontend, link)) {
        if (frontend->map == map) {
            break;
        }
    }
    if (!frontend) {
        /* or tune a new frontend */
        for (frontend = BLST_Q_FIRST(&g_frontends); frontend; frontend = BLST_Q_NEXT(frontend, link)) {
            if (!frontend->map) break;
        }
    }
    if (frontend) {
        rc = tune(d->parserBand, frontend->frontend, &map->tune, (frontend->map != NULL));
        if (rc) {
            frontend = NULL;
        }
        else {
            frontend->map = map;
        }
    }
    if (!frontend) {
        if (!g_quiet) BDBG_WRN(("unable to tune"));
        return -1;
    }

    d->frontend = frontend;
    frontend->refcnt++;

    d->start_settings.parserBand = d->parserBand;
    if (d->video_pid != 0x1fff) {
        d->start_settings.video.pid = d->video_pid;
        d->start_settings.video.codec = d->video_type;
    }
    else {
        d->start_settings.video.pid = map->scan_results.program_info[d->program].video_pids[0].pid;
        d->start_settings.video.codec = map->scan_results.program_info[d->program].video_pids[0].codec;
    }
    if (d->audio_pid != 0x1fff) {
        d->start_settings.audio.pid = d->audio_pid;
        d->start_settings.audio.codec = d->audio_type;
    }
    else {
        d->start_settings.audio.pid = map->scan_results.program_info[d->program].audio_pids[0].pid;
        d->start_settings.audio.codec = map->scan_results.program_info[d->program].audio_pids[0].codec;
    }
    d->start_settings.pcr_pid = map->scan_results.program_info[d->program].pcr_pid;
    rc = live_decode_start_channel(d->channel, &d->start_settings);
    if (rc) return BERR_TRACE(rc);

    BDBG_MSG(("start_priming(%p) %p frontend=%p(%d) parserBand=%p, map=%p program=%d", (void*)d, (void*)d->channel, (void*)d->frontend->frontend, d->frontend->refcnt, (void*)d->parserBand, (void*)d->map, d->program));

    return 0;
}

static void change_channels(int dir)
{
    struct decoder *d;

    if (dir > 0) {
        struct decoder *prev_d;
        d = BLST_Q_FIRST(&g_decoders);
        if (!d) return;

        stop_decode(d);

        /* if we're priming, the channel change happens in the next prime */
        BLST_Q_REMOVE_HEAD(&g_decoders, link);
        BLST_Q_INSERT_TAIL(&g_decoders, d, link);

        /* start priming the last */
        d = BLST_Q_LAST(&g_decoders);
        prev_d = BLST_Q_PREV(d, link);
        if (!prev_d) prev_d = d; /* no primers */
        set_channel(d, prev_d->chNum+1);
        start_priming(d);

        /* start decoding the first */
        d = BLST_Q_FIRST(&g_decoders);
        if (d->channel && d->map) {
            BDBG_MSG(("activate(%p)", (void*)d));
            live_decode_activate(d->channel);
        }
    }
    else {
        struct decoder *first_d;

        /* start priming the first */
        first_d = BLST_Q_FIRST(&g_decoders);
        stop_decode(first_d);
        start_priming(first_d);

        /* stop priming the last and use it to decode */
        d = BLST_Q_LAST(&g_decoders);
        if (!d) return;
        BLST_Q_REMOVE(&g_decoders, d, link);
        BLST_Q_INSERT_HEAD(&g_decoders, d, link);
        stop_decode(d);

        /* start decoding the first */
        if (!set_channel(d, first_d->chNum == 0 ? g_total_channels-1: first_d->chNum-1)) {
            BDBG_MSG(("activate(%p)", (void*)d));
            start_priming(d);
            live_decode_activate(d->channel);
        }
    }
}

static void init_decoders(unsigned ch)
{
    struct decoder *d;

    for (d = BLST_Q_FIRST(&g_decoders); d; d = BLST_Q_NEXT(d, link)) {
        stop_decode(d);
    }

    for (d = BLST_Q_FIRST(&g_decoders); d; d = BLST_Q_NEXT(d, link)) {
        set_channel(d, ch++);
        start_priming(d);
    }

    d = BLST_Q_FIRST(&g_decoders);
    if (d->channel && d->map) {
        BDBG_MSG(("activate(%p)", (void*)d));
        live_decode_activate(d->channel);
    }
}

static void print_status(void)
{
    struct decoder *d;
    for (d = BLST_Q_FIRST(&g_decoders); d; d = BLST_Q_NEXT(d, link)) {
        NEXUS_VideoDecoderStatus status;
        NEXUS_SimpleVideoDecoder_GetStatus(live_decode_get_video_decoder(d->channel), &status);
        BDBG_WRN(("%s %p: %d/%d %d%%, pts %#x, diff %d",
            d == BLST_Q_FIRST(&g_decoders) ? "decode":"primer",
            (void*)d, status.fifoDepth, status.fifoSize, status.fifoSize ? status.fifoDepth * 100 / status.fifoSize : 0,
            status.pts, status.ptsStcDifference));
    }
}

static void *standby_monitor(void *context)
{
    struct client_state *client = context;
    NEXUS_Error rc;
    NxClient_StandbyStatus standbyStatus, prevStatus;

    rc = NxClient_GetStandbyStatus(&prevStatus);
    BDBG_ASSERT(!rc);

    while(!client->done) {
        rc = NxClient_GetStandbyStatus(&standbyStatus);
        BDBG_ASSERT(!rc);

        if(standbyStatus.transition == NxClient_StandbyTransition_eAckNeeded) {
            struct decoder *d;
            if (!g_quiet) printf("'live' acknowledges standby state: %s", lookup_name(g_platformStandbyModeStrs, standbyStatus.settings.mode));
            for (d = BLST_Q_FIRST(&g_decoders); d; d = BLST_Q_NEXT(d, link)) {
                stop_decode(d);
            }
            NxClient_AcknowledgeStandby(true);
        } else {
            if(standbyStatus.settings.mode == NEXUS_PlatformStandbyMode_eOn && prevStatus.settings.mode != NEXUS_PlatformStandbyMode_eOn)
                init_decoders(0);
        }

        prevStatus = standbyStatus;
        BKNI_Sleep(100);
    }
    return NULL;
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NEXUS_SurfaceClientHandle surfaceClient, video_sc;
    NEXUS_Error rc;
    BKNI_EventHandle event;
    int curarg = 1;
    unsigned i;
    unsigned num_decodes = 3; /* 1 decode + 2 primers */
    struct channel_map *map = NULL;
    live_decode_t decode;
    live_decode_create_settings create_settings;
    live_decode_start_settings start_settings;
    unsigned video_pid = 0x1fff, audio_pid = 0x1fff;
    NEXUS_VideoCodec video_type = NEXUS_VideoCodec_eMpeg2;
    NEXUS_AudioCodec audio_type = NEXUS_AudioCodec_eAc3;
    struct decoder *d;
    struct frontend *frontend;
    bool prompt = false;
    binput_t input;
    struct client_state client_state;
    pthread_t standby_thread_id;
    unsigned program = 0;
    unsigned timeout = 0, starttime;
#if B_REFSW_TR69C_SUPPORT
    b_tr69c_t tr69c;
#endif
    struct binput_settings input_settings;
    struct nxapps_cmdline cmdline;
    int n;
    enum gui gui = gui_on;
    struct b_pig_inc pig_inc;
    NEXUS_VideoWindowContentMode contentMode = NEXUS_VideoWindowContentMode_eMax;

    memset(&pig_inc, 0, sizeof(pig_inc));
    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_frontend);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_SurfaceComposition);
    memset(&client_state, 0, sizeof(client_state));
    live_decode_get_default_create_settings(&create_settings);
    live_decode_get_default_start_settings(&start_settings);
    binput_get_default_settings(&input_settings);

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage(&cmdline);
            return 0;
        }
#if NEXUS_HAS_FRONTEND
        else if (!strcmp(argv[curarg], "-const")) {
            gui = gui_constellation;
        }
#endif
        else if (!strcmp(argv[curarg], "-q")) {
            g_quiet = true;
        }
        else if (!strcmp(argv[curarg], "-gui") && argc>curarg+1) {
            ++curarg;
            if (!strcmp(argv[curarg], "no_alpha_hole")) {
                gui = gui_no_alpha_hole;
            }
            else {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-pig")) {
            pig_inc.y = pig_inc.x = 4;
            pig_inc.width = 2;
        }
        else if (!strcmp(argv[curarg], "-primers") && argc>curarg+1) {
            num_decodes = atoi(argv[++curarg])+1;
        }
        else if (!strcmp(argv[curarg], "-prompt")) {
            prompt = true;
        }
        else if (!strcmp(argv[curarg], "-pip")) {
            const char *argv[] = {"-rect","960,0,960,540","-zorder","1"};
            if (!cmdline.comp.rect.set) {
                nxapps_cmdline_parse(0, 2, argv, &cmdline);
            }
            if (!cmdline.comp.zorder.set) {
                nxapps_cmdline_parse(2, 4, argv, &cmdline);
            }
            num_decodes = 1; /* no priming in pip */
            start_settings.video.videoWindowType = NxClient_VideoWindowType_ePip;
        }
        else if (!strcmp(argv[curarg], "-program") && argc>curarg+1) {
            program = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-ar") && curarg+1 < argc) {
            contentMode = lookup(g_contentModeStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-format") && argc>curarg+1) {
            start_settings.video.maxFormat = lookup(g_videoFormatStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-max") && curarg+1 < argc) {
            int n = sscanf(argv[++curarg], "%u,%u", &start_settings.video.maxWidth, &start_settings.video.maxHeight);
            if (n != 2) {
                print_usage(&cmdline);
                return -1;
            }
        }
        else if (!strcmp(argv[curarg], "-colorDepth") && curarg+1 < argc) {
            start_settings.video.colorDepth = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-script") && argc>curarg+1) {
            input_settings.script_file = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-scrtext") && argc>curarg+1) {
            input_settings.script = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "--help-script")) {
            binput_print_script_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-smooth")) {
            start_settings.video.smoothResolutionChange = true;
        }
        else if (!strcmp(argv[curarg], "-preroll") && argc>curarg+1) {
            start_settings.video.prerollRate = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-channelChange") && argc>curarg+1) {
            create_settings.video.channelChangeMode = lookup(g_channelChangeMode, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-video") && argc>curarg+1) {
            video_pid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-video_type") && argc>curarg+1) {
            video_type = lookup(g_videoCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-audio") && argc>curarg+1) {
            audio_pid = strtoul(argv[++curarg], NULL, 0);
        }
        else if (!strcmp(argv[curarg], "-audio_type") && argc>curarg+1) {
            audio_type = lookup(g_audioCodecStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-sync") && argc>curarg+1) {
            create_settings.sync = lookup(g_syncModeStrs, argv[++curarg]);
        }
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else {
            print_usage(&cmdline);
            return 1;
        }
        curarg++;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    BKNI_CreateEvent(&event);

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);
    surfaceClient = NEXUS_SurfaceClient_Acquire(allocResults.surfaceClient[0].id);
    if (surfaceClient) {
        if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SurfaceComposition)) {
            NEXUS_SurfaceComposition comp;
            NxClient_GetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
            nxapps_cmdline_apply_SurfaceComposition(&cmdline, &comp);
            NxClient_SetSurfaceClientComposition(allocResults.surfaceClient[0].id, &comp);
        }
        video_sc = NEXUS_SurfaceClient_AcquireVideoWindow(surfaceClient, 0);
        if (contentMode != NEXUS_VideoWindowContentMode_eMax) {
            NEXUS_SurfaceClientSettings settings;
            NEXUS_SurfaceClient_GetSettings(video_sc, &settings);
            settings.composition.contentMode = contentMode;
            NEXUS_SurfaceClient_SetSettings(video_sc, &settings);
        }
        if (pig_inc.x) {
            b_pig_init(video_sc);
        }
    }

    input = binput_open(&input_settings);

    get_default_channels(&cmdline.frontend.tune, &cmdline.frontend.freq_list);
    parse_channels(&cmdline.frontend.tune, cmdline.frontend.freq_list);

    {
        unsigned scanning = 0;
        bool waiting = false;
        /* start scanning as many as possible */
        while (1) {
            bool done = true;
            for (map = BLST_Q_FIRST(&g_channel_map); map; map = BLST_Q_NEXT(map, link)) {
                struct channel_map *nextmap;
                if (map->scan_state == scan_state_done) {
                    continue;
                }
                done = false;
                if (map->scan_state == scan_state_pending && !waiting) {
                    bchannel_scan_start_settings scan_settings;
                    bchannel_scan_get_default_start_settings(&scan_settings);
                    scan_settings.tune = map->tune;
                    if (!map->scan) {
                        map->scan = bchannel_scan_start(&scan_settings);
                        if (!map->scan) {
                            /* start fails if there are no resources.
                            if there's no scan going, there's no reason to expect new resources.
                            if there's a scan going, don't try again until one scan has finished. */
                            if (!scanning) {
                                map->scan_state = scan_state_done;
                            }
                            else {
                                waiting = true;
                            }
                        }
                    }
                    else {
                        bchannel_scan_restart(map->scan, &scan_settings);
                    }

                    if (map->scan) {
                        if (!g_quiet) BDBG_WRN(("Scanning %s...", map->name));
                        map->scan_state = scan_state_started;
                        map->scan_start = b_get_time();
                        scanning++;
                    }

                }
                if (!map->scan) {
                    continue;
                }

                rc = bchannel_scan_get_results(map->scan, &map->scan_results);
                if (rc == NEXUS_NOT_AVAILABLE) {
                    if (b_get_time() - map->scan_start < 7500) {
                        continue;
                    }
                }

                map->scan_state = scan_state_done;
                waiting = false;
                scanning--;
                if (!g_quiet) BDBG_WRN(("%d programs found on %s", map->scan_results.num_programs, map->name));
#if NEXUS_HAS_FRONTEND
                if (!map->scan_results.num_programs && map->tune.source != channel_source_streamer) {
                    NEXUS_FrontendHandle frontend;
                    NEXUS_ParserBand parserBand;
                    NEXUS_FrontendFastStatus status;
                    bchannel_scan_get_resources(map->scan, &frontend, &parserBand);
                    rc = NEXUS_Frontend_GetFastStatus(frontend, &status);
                    if (rc) {
                        BERR_TRACE(rc);
                    }
                    else {
                        const char *lockstr[NEXUS_FrontendLockStatus_eMax] = {"unknown", "unlocked", "locked", "no signal"};
                        if (!g_quiet) BDBG_WRN(("  frontend lock status: %s", lockstr[status.lockStatus]));
                    }
                }
#endif

                g_total_channels += map->scan_results.num_programs;

                /* give this scan to another if possible */
                for (nextmap = BLST_Q_NEXT(map, link); nextmap; nextmap = BLST_Q_NEXT(nextmap, link)) {
                    if (!nextmap->scan && !map->scan_state == scan_state_pending) {
                        nextmap->scan = map->scan;
                        map->scan = NULL;
                        break;
                    }
                }
                if (!nextmap) {
                    /* no longer needed */
                    bchannel_scan_stop(map->scan);
                    map->scan = NULL;
                }
            }
            if (done) break;
        }

        for (map = BLST_Q_FIRST(&g_channel_map); map; map = BLST_Q_NEXT(map, link)) {
            if (map->scan) {
                bchannel_scan_stop(map->scan);
                map->scan = NULL;
            }
        }
    }

    if (cmdline.frontend.tune.source == channel_source_streamer) {
        frontend = BKNI_Malloc(sizeof(*frontend));
        memset(frontend, 0, sizeof(*frontend));
        BLST_Q_INSERT_TAIL(&g_frontends, frontend, link);
    }
#if NEXUS_HAS_FRONTEND
    else {
        for (i=0;i<num_decodes;i++) {
            NEXUS_FrontendAcquireSettings settings;
            struct frontend *frontend;

            frontend = BKNI_Malloc(sizeof(*frontend));
            memset(frontend, 0, sizeof(*frontend));

            NEXUS_Frontend_GetDefaultAcquireSettings(&settings);
            settings.capabilities.qam = (cmdline.frontend.tune.source == channel_source_qam);
            settings.capabilities.ofdm = (cmdline.frontend.tune.source == channel_source_ofdm);
            settings.capabilities.satellite = (cmdline.frontend.tune.source == channel_source_sat);
            frontend->frontend = NEXUS_Frontend_Acquire(&settings);
            if (!frontend->frontend) {
                BKNI_Free(frontend);
                num_decodes = i; /* reduce number of decodes to number of frontends for simpler app logic */
                break;
            }
            BLST_Q_INSERT_TAIL(&g_frontends, frontend, link);
        }
    }
#endif

    frontend = BLST_Q_FIRST(&g_frontends);
    if (!frontend) {
        BDBG_ERR(("Unable to find capable frontend"));
        return -1;
    }
    gui_init(frontend->frontend, surfaceClient, gui);

    create_settings.primed = num_decodes>1;
    create_settings.window.surfaceClientId = allocResults.surfaceClient[0].id;
    decode = live_decode_create(&create_settings);
    for (i=0; i<num_decodes; i++) {
        struct decoder *dt = BKNI_Malloc(sizeof(*dt));
        NEXUS_SimpleVideoDecoderHandle videoDecoder;
        memset(dt, 0, sizeof(*dt));
        dt->parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
        if (!dt->parserBand) {
            BKNI_Free(dt);
            continue;
        }
        dt->start_settings = start_settings;
        dt->video_pid = video_pid;
        dt->video_type = video_type;
        dt->audio_pid = audio_pid;
        dt->audio_type = audio_type;
        dt->channel = live_decode_create_channel(decode);
        BDBG_ASSERT(dt->channel);
        BLST_Q_INSERT_TAIL(&g_decoders, dt, link);
        BDBG_MSG(("decoder %d = %p", i, (void*)dt));

        videoDecoder = live_decode_get_video_decoder(dt->channel);
        if (videoDecoder) {
            if (nxapps_cmdline_is_set(&cmdline, nxapps_cmdline_type_SimpleVideoDecoderPictureQualitySettings)) {
                NEXUS_SimpleVideoDecoderPictureQualitySettings settings;
                NEXUS_SimpleVideoDecoder_GetPictureQualitySettings(videoDecoder, &settings);
                nxapps_cmdline_apply_SimpleVideoDecodePictureQualitySettings(&cmdline, &settings);
                NEXUS_SimpleVideoDecoder_SetPictureQualitySettings(videoDecoder, &settings);
            }
        }

    }
    d = BLST_Q_FIRST(&g_decoders);
    if( !d ) {
        BDBG_ERR(("Unable to obtain parser band"));
        return -1;
    }

    init_decoders(program);

#if B_REFSW_TR69C_SUPPORT
    tr69c = b_tr69c_init(live_decode_get_set_tr69c_info, BLST_Q_FIRST(&g_decoders)->channel);
#endif

    pthread_create(&standby_thread_id, NULL, standby_monitor, &client_state);

    starttime = b_get_time();
    if (!g_total_channels) {
        if (!g_quiet) BDBG_WRN(("No channels found."));
    }
    else if (prompt) {
        if (!g_quiet) BDBG_WRN(("Press ENTER to change channel."));
    }
    else {
        if (!g_quiet) BDBG_WRN(("Use remote control to change channel. \"Stop\" or \"Clear\" will exit the app."));
    }
    /* channel change algo */
    while (!client_state.done) {
        char buf[64];

        d = BLST_Q_FIRST(&g_decoders);
        if (!d->map) break;

        if (!g_quiet) BDBG_WRN(("Decoding %s program %d: video %s %#x, audio %s %#x, pcr %#x",
            d->map->name,
            d->program,
            lookup_name(g_videoCodecStrs, d->start_settings.video.codec),
            d->start_settings.video.pid,
            lookup_name(g_audioCodecStrs, d->start_settings.audio.codec),
            d->start_settings.audio.pid,
            d->start_settings.pcr_pid));

        if (prompt) {
            printf("live>");
            fflush(stdout);
            fgets(buf, sizeof(buf), stdin);
            if (feof(stdin)) break;
            buf[strlen(buf)-1] = 0; /* chop off \n */
            if (!strcmp(buf, "q")) {
                client_state.done = true;
            }
            else if (!strcmp(buf, "st")) {
                print_status();
            }
            else {
                change_channels(1);
            }
        }
        else if (pig_inc.x) {
            while (1) {
                b_pig_move(video_sc, &pig_inc);
                rc = BKNI_WaitForEvent(g_constellation.displayedEvent, 5000);
                BDBG_ASSERT(!rc);

                if (timeout && b_get_time() - starttime >= timeout*1000) {
                    client_state.done = true;
                    break;
                }
            }
        }
        else {
            while (!client_state.done) {
                b_remote_key key;
                bool change = false;

                if (timeout && b_get_time() - starttime >= timeout*1000) {
                    client_state.done = true;
                    break;
                }

                if (binput_read_no_repeat(input, &key)) {
                    binput_wait(input, 1000);
                    continue;
                }
                switch (key) {
                case b_remote_key_up:
                case b_remote_key_chan_up:
                    change_channels(1);
                    change = true;
                    break;
                case b_remote_key_down:
                case b_remote_key_chan_down:
                    change_channels(-1); /* not fcc */
                    change = true;
                    break;
                case b_remote_key_stop:
                case b_remote_key_back:
                case b_remote_key_clear:
                    client_state.done = true;
                    break;
                default:
                    break;
                }
                if (change) break;
            }
        }
    }
    client_state.done = true;

    while ((d = BLST_Q_FIRST(&g_decoders))) {
        stop_decode(d);
        if (d->channel) {
            live_decode_destroy_channel(d->channel);
        }
        NEXUS_ParserBand_Close(d->parserBand);
        BLST_Q_REMOVE(&g_decoders, d, link);
        BKNI_Free(d);
    }
    live_decode_destroy(decode);

    gui_uninit();
    while ((frontend = BLST_Q_FIRST(&g_frontends))) {
#if NEXUS_HAS_FRONTEND
        if (frontend->frontend) {
            NEXUS_Frontend_Release(frontend->frontend);
        }
#endif
        BLST_Q_REMOVE(&g_frontends, frontend, link);
        BKNI_Free(frontend);
    }
    uninit_channels();

    pthread_join(standby_thread_id, NULL);

#if B_REFSW_TR69C_SUPPORT
        b_tr69c_uninit(tr69c);
#endif

    NxClient_Free(&allocResults);
    BKNI_DestroyEvent(event);
    binput_close(input);
    NxClient_Uninit();
    return 0;
}

#if NEXUS_HAS_FRONTEND
static void *gui_thread(void *context)
{
    NEXUS_SurfaceMemory mem;
    unsigned count = 0;
    NEXUS_Rect rect = {0,0,0,0};

    BSTD_UNUSED(context);
    NEXUS_Surface_GetMemory(g_constellation.surface, &mem);

    memset(mem.buffer, 0, mem.pitch * g_constellation.createSettings.height);
    rect.width = g_constellation.createSettings.width/2;
    rect.height = g_constellation.createSettings.height/2;
    rect.y = 0;
    rect.x = rect.width;

    while (!g_constellation.done) {
#define MAX_SOFTDEC 32
        NEXUS_FrontendSoftDecision softdec[MAX_SOFTDEC];
        size_t num;
        int rc;

        if (!count) {
            int x, y;
            for (y=rect.y;y<rect.y+rect.height;y++) {
                uint32_t *ptr = (uint32_t *)((uint8_t*)mem.buffer + mem.pitch*y);
                for (x=rect.x;x<rect.x+rect.width;x++) {
                    ptr[x] = 0xAA888888;
                }
            }
        }
        rc = NEXUS_Frontend_ReadSoftDecisions(g_constellation.frontend, softdec, MAX_SOFTDEC, &num);
        if (!rc) {
            unsigned i;
            for (i=0;i<num;i++) {
                unsigned x = rect.x + (rect.width * (softdec[i].i+32768)) / 65536;
                unsigned y = rect.y + (rect.height * (softdec[i].q+32768)) / 65536;
                uint32_t *ptr = (uint32_t *)((uint8_t*)mem.buffer + mem.pitch*y);

                ptr = &ptr[x];
                ptr[0] = ptr[1] = 0xFFBBBBBB;
                ptr = (uint32_t *)((uint8_t*)ptr + mem.pitch);
                ptr[0] = ptr[1] = 0xFFBBBBBB;
                count++;
            }
            NEXUS_Surface_Flush(g_constellation.surface);

            rc = NEXUS_SurfaceClient_SetSurface(g_constellation.surfaceClient, g_constellation.surface);
            BDBG_ASSERT(!rc);
            rc = BKNI_WaitForEvent(g_constellation.displayedEvent, 5000);
            BDBG_ASSERT(!rc);

            if (count > 4000) count = 0;
        }
    }
    return NULL;
}
#endif

static int gui_init(NEXUS_FrontendHandle frontend, NEXUS_SurfaceClientHandle surfaceClient, enum gui gui)
{
    int rc;
    NEXUS_SurfaceClientSettings client_settings;
    NEXUS_SurfaceMemory mem;

    memset(&g_constellation, 0, sizeof(g_constellation));
    g_constellation.frontend = frontend;
    g_constellation.surfaceClient = surfaceClient;

    BKNI_CreateEvent(&g_constellation.displayedEvent);

    NEXUS_SurfaceClient_GetSettings(g_constellation.surfaceClient, &client_settings);
    client_settings.displayed.callback = complete2;
    client_settings.displayed.context = g_constellation.displayedEvent;
    client_settings.windowMoved.callback = complete2;
    client_settings.windowMoved.context = g_constellation.displayedEvent;
    rc = NEXUS_SurfaceClient_SetSettings(g_constellation.surfaceClient, &client_settings);
    BDBG_ASSERT(!rc);

    if (gui > gui_no_alpha_hole) {
        NEXUS_Surface_GetDefaultCreateSettings(&g_constellation.createSettings);
        g_constellation.createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
        g_constellation.createSettings.width = 720;
        g_constellation.createSettings.height = 480;
        g_constellation.surface = NEXUS_Surface_Create(&g_constellation.createSettings);

        /* black screen, even if no constellation */
        NEXUS_Surface_GetMemory(g_constellation.surface, &mem);
        memset(mem.buffer, 0, mem.pitch * g_constellation.createSettings.height);
        NEXUS_Surface_Flush(g_constellation.surface);
        rc = NEXUS_SurfaceClient_SetSurface(g_constellation.surfaceClient, g_constellation.surface);
        BDBG_ASSERT(!rc);
        rc = BKNI_WaitForEvent(g_constellation.displayedEvent, 5000);
        BDBG_ASSERT(!rc);

#if NEXUS_HAS_FRONTEND
        if (gui == gui_constellation && frontend) {
            rc = pthread_create(&g_constellation.thread, NULL, gui_thread, NULL);
            BDBG_ASSERT(!rc);
        }
#endif
    }

    return 0;
}

static void gui_uninit(void)
{
    NEXUS_SurfaceClientSettings client_settings;
    g_constellation.done = true;
#if NEXUS_HAS_FRONTEND
    if (g_constellation.thread) {
        pthread_join(g_constellation.thread, NULL);
    }
#endif
    if (g_constellation.surface) {
        NEXUS_Surface_Destroy(g_constellation.surface);
    }
    NEXUS_SurfaceClient_GetSettings(g_constellation.surfaceClient, &client_settings);
    client_settings.displayed.callback = NULL;
    client_settings.windowMoved.callback = NULL;
    NEXUS_SurfaceClient_SetSettings(g_constellation.surfaceClient, &client_settings);

    BKNI_DestroyEvent(g_constellation.displayedEvent);
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs input_router, simple_decoder and transport)!\n");
    return 0;
}
#endif
