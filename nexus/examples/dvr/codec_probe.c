/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
 *****************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_recpump.h"
#include "nexus_file.h"

#include <assert.h>
#include "bstd.h"
#include "bkni.h"

#include "bmpeg2ts_parser.h"
#include "bmpeg2pes_parser.h"
#include "bmedia_probe_es.h"
#include "bmpeg_video_probe.h"
#include "bmpeg_audio_probe.h"
#include "bmedia_probe_impl.h"

BDBG_MODULE(codec_probe);

static bool g_eos = false;
void eos(void *context, int param)
{
    BSTD_UNUSED(context);
    BSTD_UNUSED(param);
    g_eos = true;
}

static void dataready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

typedef struct {
    bmpeg2ts_parser_pid ts; /* should be first */
    bmpeg2pes_parser pes;
    const bmedia_probe_es_desc *probe_desc;
    bmedia_probe_base_es_t probe;
	bmedia_probe_track *track;
	batom_accum_t pes_accum;
    bool active;
} payload_probe;

typedef struct {
    batom_factory_t factory;
    batom_accum_t accum;
} payload_feed;

static bmpeg2ts_parser_action 
payload_probe_ts_data(bmpeg2ts_parser_pid *pid, unsigned flags, batom_accum_t src, batom_cursor *cursor, size_t len)
{
    payload_probe *probe = (payload_probe*)pid;
    BDBG_MSG(("payload_probe_ts_data: %#lx pid:%#x %u bytes", probe, probe->ts.pid, len));
    if(probe->active) {
        return bmpeg2pes_parser_feed(&probe->pes, flags, src, cursor, len);
    }
    return bmpeg2ts_parser_action_skip;
}

static void 
payload_print_track(payload_probe *probe)
{
    if(probe->track) {
        bmedia_probe_stream stream;
        char str[128];

        bmedia_probe_stream_init(&stream, bstream_mpeg_type_es);
        bmedia_probe_add_track(&stream, probe->track);
        bmedia_stream_to_string(&stream, str, sizeof(str));
        BDBG_LOG(("%s", str));
    }
    return;
}

static void payload_probe_last(payload_probe *probe)
{
    unsigned probability = 0;
    probe->track =  probe->probe_desc->last(probe->probe, &probability);
    payload_print_track(probe);
    return;
}

static void 
payload_probe_pes_data(void *packet_cnxt, batom_accum_t src, batom_cursor *payload, size_t len, const bmpeg2pes_atom_info *info)
{
    payload_probe *probe = packet_cnxt;
    size_t accum_len;
    batom_t packet = NULL;
    batom_cursor payload_start;

    BDBG_MSG(("payload_probe_pes_data: %#lx stream %#x:%#x pes data %u:%u", (unsigned long)probe, (unsigned)probe->ts.pid, (unsigned)info->pes_id, info->data_offset, len));

    if(!probe->active || probe->track) {
        return;
    }
    accum_len = batom_accum_len(probe->pes_accum);
    if( accum_len>(8*1024) || (accum_len>0 && info->data_offset == 0)) {
        packet = batom_from_accum(probe->pes_accum, NULL, NULL); 
    }
    BATOM_CLONE(&payload_start, payload);
    batom_cursor_skip(payload, len);
    batom_accum_append(probe->pes_accum, src, &payload_start, payload);
    if(packet) {
        bool done = false;
        probe->track = probe->probe_desc->feed(probe->probe, packet, &done);
        if(done||probe->track) {
            payload_print_track(probe);
            probe->active = false;
        }
        batom_release(packet);
    }
    return;
}

static void payload_probe_init(payload_probe *probe, const payload_feed *feed, uint16_t pid, const bmedia_probe_es_desc *probe_desc)
{
    bmpeg2ts_parser_pid_init(&probe->ts, pid);
    bmpeg2pes_parser_init(feed->factory, &probe->pes, BMPEG2PES_ID_ANY);
    probe->ts.payload = payload_probe_ts_data;
    probe->pes.packet = payload_probe_pes_data;
    probe->pes.packet_cnxt = probe;
    probe->probe_desc = probe_desc;
    probe->probe =  probe_desc->create(feed->factory);
    BDBG_ASSERT(probe->probe);
    probe->pes_accum = batom_accum_create(feed->factory);
    BDBG_ASSERT(probe->pes_accum);
    probe->active = true;
    probe->track = NULL;
    return;
}

static void payload_probe_shutdown(payload_probe *probe)
{
    bmpeg2pes_parser_shutdown(&probe->pes);
    probe->probe_desc->destroy(probe->probe);
    batom_accum_destroy(probe->pes_accum);
    return;
}

static void payload_probe_reset(payload_probe *probe)
{
    probe->probe_desc->reset(probe->probe);
    bmpeg2pes_parser_reset(&probe->pes);
    batom_accum_clear(probe->pes_accum);
    if(probe->track) {
        BKNI_Free(probe->track);
        probe->track = NULL;
    }
    probe->active = true;
    return;
}

static void payload_feed_init(payload_feed *feed)
{
    feed->factory = batom_factory_create(bkni_alloc, 256);
    BDBG_ASSERT(feed->factory);
    feed->accum = batom_accum_create(feed->factory);
    BDBG_ASSERT(feed->accum);
    return;
}

static void payload_feed_shutdown(payload_feed *feed)
{
    batom_accum_destroy(feed->accum);
    batom_factory_destroy(feed->factory);
    return;
}

static void 
b_atom_free_media(batom_t atom, void *user)
{
    void *block = *(void **)user;
    BDBG_MSG(("free_media %p(%p)", block, atom));
    BSTD_UNUSED(atom);
    BKNI_Free(block);
    return;
}

static const batom_user b_atom_media = {
    b_atom_free_media,
    sizeof(void *)
};

static void payload_probe_feed(const payload_feed *feed, payload_probe *probes, unsigned nprobes, const void *data, size_t data_len)
{
    batom_t atom;
    void *buf;
    unsigned i, pid;
    batom_cursor cursor;


    for(i=0;i<nprobes;i++) {
        if(probes[i].active) {
            break;
        }
    }
    if(i>=nprobes) {
        return; /* nothing to do */
    }

    buf = BKNI_Malloc(data_len);
    BDBG_ASSERT(buf);
    BKNI_Memcpy(buf, data, data_len);
    atom = batom_from_range(feed->factory, buf, data_len, &b_atom_media, &buf);
    BDBG_ASSERT(atom);
    batom_accum_add_atom(feed->accum, atom);
    batom_release(atom);

    batom_cursor_from_accum(&cursor, feed->accum);
    for(pid=i;;) {
        int rc;

        if(!probes[pid].active) {
            for(i=0;i<nprobes;i++) {
                if(probes[i].active) {
                    break;
                }
            }
            if(i>=nprobes) {
                break; /* nothing to do */
            }
            pid = i;
        }
        rc = bmpeg2ts_parser_pid_feed(&probes[pid].ts, feed->accum, &cursor);
        if(probes[pid].track) {
            BDBG_MSG(("track %p for pid %#x", probes[pid].track, probes[pid].ts.pid));
        }
        if(rc<=0) {
            break;
        }
        for(i=0;i<nprobes;i++) {
            if(probes[i].active && probes[i].ts.pid == i) {
                break;
            }
        }
        if(i>=nprobes) {
            batom_cursor_skip(&cursor, BMPEG2TS_PKT_LEN);
        }
    }
    batom_accum_clear(feed->accum);
    return;
}


int main(int argc, char **argv) {
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PidChannelHandle pidChannelVideo, pidChannelAudio;
    NEXUS_FilePlayHandle playbackFile;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpAddPidChannelSettings recpumpPidCfg;
    NEXUS_RecpumpSettings recpumpCfg;
    NEXUS_Error rc;
    BKNI_EventHandle event;
    const char *fname = "videos/cnnticker.mpg";
    payload_probe probes[2];
    payload_feed feed;
    unsigned bytes_parsed=0;

    if (argc > 1) fname = argv[1];

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    playpump = NEXUS_Playpump_Open(0, NULL);
    assert(playpump);
    playback = NEXUS_Playback_Create();
    assert(playback);

    playbackFile = NEXUS_FilePlay_OpenPosix(fname, NULL);
    if (!playbackFile) {
        fprintf(stderr, "can't open file:%s\n", fname);
        return -1;
    }

    rc=BKNI_CreateEvent(&event);
    BDBG_ASSERT(rc==NEXUS_SUCCESS);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
    playbackSettings.playpumpSettings.maxDataRate = 2 * 1000000; /* prevent recpump overflow because of limited disk i/o speed */
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
    playbackSettings.endOfStreamCallback.callback = eos;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    pidChannelVideo = NEXUS_Playback_OpenPidChannel(playback, 0x21, NULL);
    BDBG_ASSERT(pidChannelVideo);
    pidChannelAudio = NEXUS_Playback_OpenPidChannel(playback, 0x22, NULL);
    BDBG_ASSERT(pidChannelAudio);

    recpump = NEXUS_Recpump_Open(0, NULL);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&recpumpPidCfg);
    recpumpPidCfg.pidType = NEXUS_PidType_eVideo;
    recpumpPidCfg.pidTypeSettings.video.index = false;
    recpumpPidCfg.pidTypeSettings.video.codec = NEXUS_VideoCodec_eMpeg2;
    recpumpPidCfg.pidTypeSettings.video.pid = 0x21; 
    rc = NEXUS_Recpump_AddPidChannel(recpump, pidChannelVideo, &recpumpPidCfg);
    BDBG_ASSERT(!rc);

    NEXUS_Recpump_GetDefaultAddPidChannelSettings(&recpumpPidCfg);
    recpumpPidCfg.pidType = NEXUS_PidType_eAudio;
    rc = NEXUS_Recpump_AddPidChannel(recpump, pidChannelAudio, &recpumpPidCfg);
    BDBG_ASSERT(!rc);

    NEXUS_Recpump_GetSettings(recpump, &recpumpCfg);
    recpumpCfg.data.dataReady.callback = dataready_callback;
    recpumpCfg.data.dataReady.context = event;

    NEXUS_Recpump_SetSettings(recpump, &recpumpCfg);


    rc = NEXUS_Recpump_Start(recpump);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Playback_Start(playback, playbackFile, NULL);
    BDBG_ASSERT(!rc);

    payload_feed_init(&feed);
    payload_probe_init(&probes[0], &feed, 0x21, &bmpeg_video_probe);
    payload_probe_init(&probes[1], &feed, 0x22, &bmpeg_audio_probe);

    while (1) {
        const void *data_buffer;
        size_t data_buffer_size;
        unsigned i;

        rc = NEXUS_Recpump_GetDataBuffer(recpump, &data_buffer, &data_buffer_size);
        BDBG_ASSERT(!rc);
        if (data_buffer_size == 0) {
            BKNI_WaitForEvent(event, BKNI_INFINITE);
            continue;
        }
        payload_probe_feed(&feed, probes, sizeof(probes)/sizeof(*probes), data_buffer, data_buffer_size);
        bytes_parsed += data_buffer_size;
        rc = NEXUS_Recpump_DataReadComplete(recpump, data_buffer_size);
        BDBG_ASSERT(!rc);
        for(i=0;i<sizeof(probes)/sizeof(*probes);i++) {
            if(probes[i].active) {
                break;
            }
        }
        if(i==sizeof(probes)/sizeof(*probes)) {
            break; /* parsed all pids */
        }

        if (g_eos || bytes_parsed>=500*1024) { /* maximum of 500 KB */
            for(i=0;i<sizeof(probes)/sizeof(*probes);i++) {
                if(probes[i].active) {
                    payload_probe_last(&probes[i]);
                    break;
                }
            }
            break;
        }
    }
    payload_probe_reset(&probes[0]);
    payload_probe_reset(&probes[1]);

    payload_probe_shutdown(&probes[0]);
    payload_probe_shutdown(&probes[1]);
    payload_feed_shutdown(&feed);


    /* Bring down system */
    NEXUS_Recpump_Stop(recpump);
    NEXUS_Playback_Stop(playback);
    NEXUS_FilePlay_Close(playbackFile);
    NEXUS_Recpump_RemoveAllPidChannels(recpump);
    NEXUS_Recpump_RemoveAllPidChannels(recpump);
	NEXUS_Playback_Destroy(playback);
	NEXUS_Playpump_Close(playpump);
    NEXUS_Recpump_Close(recpump);
    BKNI_DestroyEvent(event);
    NEXUS_Platform_Uninit();
    return 0;
}
#else 
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
