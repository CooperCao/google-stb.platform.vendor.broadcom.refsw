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
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_RECORD && NEXUS_HAS_SIMPLE_DECODER && NEXUS_HAS_TRANSPORT
#include "nxclient.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_platform.h"
#include "nexus_record.h"
#include "nexus_file_fifo.h"
#include "nexus_file_chunk.h"
#include "nexus_file_fifo_chunk.h"
#include "nexus_parser_band.h"
#include "live_decode.h"
#include "live_source.h"
#include "tspsimgr3.h"
#include "namevalue.h"
#include "brecord_gui.h"
#include "dvr_crypto.h"
#include "media_probe.h"
#include "nxapps_cmdline.h"

#include "bcmplayer.h"


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

BDBG_MODULE(record);


struct recordContext {
    struct {
        NEXUS_ChunkedFifoRecordHandle chunkedFifofile;
        NEXUS_FifoRecordHandle fifofile;
        NEXUS_FileRecordHandle file;
        NEXUS_RecpumpHandle recpump;
        NEXUS_RecordHandle record;
        struct {
            NEXUS_PlaypumpHandle playpump;
            NEXUS_PlaybackHandle playback;
            NEXUS_FilePlayHandle file;
        } playback;
        NEXUS_FrontendHandle frontend;
        NEXUS_ParserBand parserBand;
        dvr_crypto_t crypto;
        bpsi_injection_t psi;
    } handles;

    struct {
        const char *filename;
        const char *indexname;
        struct {
            const char *filename;
        } playback;
        unsigned program;
        unsigned timeshift;
        NEXUS_SecurityAlgorithm encrypt_algo;
        NEXUS_TransportTimestampType timestampType;
        NEXUS_TristateEnable psi_injection;
        bool append;
        bool indexonly;
        struct btune_settings tune_settings;
        unsigned chunkSize;
        bool allpass;
    } settings;

    struct {
        bool started;
        bool quit;
        NEXUS_PlatformStandbyMode mode;
    } state;

    tspsimgr_scan_results scan_results;
};

char default_filename[64], default_indexname[64];

static void complete(void *data)
{
    BKNI_SetEvent((BKNI_EventHandle)data);
}

int start_play(NEXUS_RecordHandle record, NEXUS_FifoRecordHandle fifofile,
    const char *filename, const char *indexname,
    tspsimgr_scan_results *pscan_results, unsigned program);

static void print_usage(const struct nxapps_cmdline *cmdline)
{
    printf(
        "Usage: record [OUTPUTFILE [OUTPUTINDEX]]\n"
        "\n"
        "Default OUTPUTFILE and OUTPUTINDEX is videos/stream#.mpg and .nav where # is the RAVE context used\n"
        "\n"
        "OPTIONS:\n"
        "  --help or -h for help\n"
    );
    nxapps_cmdline_print_usage(cmdline);
    printf(
        "  -file NAME               record from playback. Use OUTPUTFILE of \"none\" to do index-only record\n"
        "  -program #               program number to record (default is 0)\n"
    );
    printf(
        "  -prompt\n"
        "  -gui off\n"
        "  -timeshift MINUTES       enable timeshifting\n"
        "  -q                       don't print progress\n"
        "  -tts                     record with 4 byte timestamp\n"
        "  -timeout SECONDS\n"
        );
    print_list_option(
        "  -crypto                  encrypt stream",g_securityAlgoStrs);
    printf(
        "  -psi {on|off}            inject PSI into recording. defaults on with crypto.\n"
        "  -append                  append recording to OUTPUTFILE\n"
        "  -chunk SIZE              chunked record, SIZE in mb\n"
        "  -allpass\n"
        );
}

static void print_record_status(struct recordContext *recContext)
{
    NEXUS_RecordStatus status;
    NEXUS_Record_GetStatus(recContext->handles.record, &status);

    if (recContext->state.mode > NEXUS_PlatformStandbyMode_eActive)
        return;

    if (recContext->settings.indexonly) {
        status.recpumpStatus.data.bytesRecorded = 0;
    }
    BDBG_WRN(("%s "BDBG_UINT64_FMT" bytes (%d%%), %s %u (%d%%), %d seconds, %d pictures",
        recContext->settings.filename,
        BDBG_UINT64_ARG(status.recpumpStatus.data.bytesRecorded),
        status.recpumpStatus.data.fifoSize ? (unsigned)(status.recpumpStatus.data.fifoDepth*100/status.recpumpStatus.data.fifoSize) : 0,
        recContext->settings.indexname,
        (unsigned)status.recpumpStatus.index.bytesRecorded,
        status.recpumpStatus.index.fifoSize ? (unsigned)(status.recpumpStatus.index.fifoDepth*100/status.recpumpStatus.index.fifoSize) : 0,
        (unsigned)status.lastTimestamp / 1000,
        (unsigned)status.picturesIndexed));
}

static int set_fifo_settings(struct recordContext *recContext, unsigned interval, unsigned maxBitRate)
{
    NEXUS_Error rc;
    if (recContext->handles.chunkedFifofile) {
        NEXUS_ChunkedFifoRecordSettings fifoRecordSettings;
        NEXUS_ChunkedFifoRecord_GetSettings(recContext->handles.chunkedFifofile, &fifoRecordSettings);
        fifoRecordSettings.interval = interval;
        fifoRecordSettings.data.chunkSize = recContext->settings.chunkSize * 1024*1024;
        rc = NEXUS_ChunkedFifoRecord_SetSettings(recContext->handles.chunkedFifofile, &fifoRecordSettings);
        if (rc) return BERR_TRACE(rc);
    }
    else {
        NEXUS_FifoRecordSettings fifoRecordSettings;
        NEXUS_FifoRecord_GetSettings(recContext->handles.fifofile, &fifoRecordSettings);
        fifoRecordSettings.interval = interval * 60;
        fifoRecordSettings.data.soft =(uint64_t)fifoRecordSettings.interval * (maxBitRate / 8);
        fifoRecordSettings.data.hard = fifoRecordSettings.data.soft*2;
#define B_DATA_ALIGN  ((188/4)*4096)
        fifoRecordSettings.data.soft -= fifoRecordSettings.data.soft % B_DATA_ALIGN;
        fifoRecordSettings.data.hard -= fifoRecordSettings.data.hard % B_DATA_ALIGN;
        fifoRecordSettings.index.soft = fifoRecordSettings.data.soft / 20;
        fifoRecordSettings.index.hard = fifoRecordSettings.index.soft*2;
        rc = NEXUS_FifoRecord_SetSettings(recContext->handles.fifofile, &fifoRecordSettings);
        if (rc) return BERR_TRACE(rc);
    }
    BDBG_WRN(("timeshift %d minutes, %d Mbps", interval, maxBitRate/1024/1024));
    return 0;
}

#include <sys/time.h>
static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static void convert_to_scan_results(tspsimgr_scan_results *pscan_results, const struct probe_results *pprobe_results)
{
    unsigned i;
    memset(pscan_results, 0, sizeof(*pscan_results));
    pscan_results->program_info[0].num_video_pids = pprobe_results->num_video;
    for (i=0;i<pprobe_results->num_video;i++) {
        pscan_results->program_info[0].video_pids[i].pid = pprobe_results->video[i].pid;
        pscan_results->program_info[0].video_pids[i].codec = pprobe_results->video[i].codec;
    }
    pscan_results->program_info[0].num_audio_pids = pprobe_results->num_audio;
    for (i=0;i<pprobe_results->num_audio;i++) {
        pscan_results->program_info[0].audio_pids[i].pid = pprobe_results->audio[i].pid;
        pscan_results->program_info[0].audio_pids[i].codec = pprobe_results->audio[i].codec;
    }
}

/**
Reading the existing file size inside NEXUS_Record is easy.
Reading the timestamp is difficult. It would require punching through several layers.
Instead, to support the appended record feature, we will require the app to pass in both the file size and the timestamp.
They are relatively easy to get outside of nexus. */
int get_stats_from_previous_recording(const char *fname, const char *index, uint64_t *offset, unsigned *timestamp)
{
    BNAV_Player_Settings cfg;
    BNAV_Player_Handle bcmplayer;
    BNAV_Player_Position position;
    FILE *file;
    NEXUS_Error rc;
    uint64_t fsize, trunc;
    long firstIndex, lastIndex;

    /* read data file size. this will allow indexing to resume with correct offsets. */
    file = fopen(fname, "r");
    if (!file) return BERR_TRACE(-1);
    fseek(file, 0, SEEK_END);
    fsize = ftello(file);
    fclose(file);

    /* data file must be truncated to a satisfy both packet (188) and direct I/O page (4096) alignment.
       see NEXUS_Record_StartAppend() header for why this is necessary */
    trunc = fsize%(uint64_t)(188/4*4096);
    rc = truncate(fname, fsize-trunc);
    if (rc) return BERR_TRACE(-1);
    BDBG_WRN(("truncate %s %u -> %u", fname, (unsigned)fsize, (unsigned)(fsize-trunc)));
    *offset = fsize-trunc;
    BDBG_ASSERT((*offset)%188==0);
    BDBG_ASSERT((*offset)%4096==0);


    /* read last timestamp from the existing index. this will allow indexing to resume with continuously incrementing timestamps */
    file = fopen(index, "r");
    if (!file) return BERR_TRACE(-1);

    fseek(file, 0, SEEK_END);
    fsize = ftello(file);

    BNAV_Player_GetDefaultSettings(&cfg);
    cfg.videoPid = 1; /* don't care */
    cfg.filePointer = file;
    cfg.readCb = (BP_READ_CB)fread;
    cfg.tellCb = (BP_TELL_CB)ftell;
    cfg.seekCb = (BP_SEEK_CB)fseek;
    rc = BNAV_Player_Open(&bcmplayer, &cfg);
    if (rc) return BERR_TRACE(rc);

    /* read them back to learn navVersion for BNAV_GetEntrySize */
    BNAV_Player_GetSettings(bcmplayer, &cfg);

    /* index file may contain an entry that points to an offset that we just truncated */
    trunc = 0;
    rc = BNAV_Player_DefaultGetBounds(bcmplayer, file, &firstIndex, &lastIndex);
    if (rc) return BERR_TRACE(rc);
    while (1) {
        rc = BNAV_Player_GetPositionInformation(bcmplayer, lastIndex-(long)trunc, &position);
        if (rc) return BERR_TRACE(rc);
        *timestamp = position.timestamp;
        if ((uint64_t)position.offsetLo < *offset) {
            break;
        }
        else if (++trunc) {
            BDBG_WRN(("removing nav entry %ld with offset %lu", position.index, position.offsetLo));
        }
    }

    BNAV_Player_Close(bcmplayer);
    fclose(file);

    rc = truncate(index, fsize-trunc*BNAV_GetEntrySize(cfg.navVersion));
    if (rc) return BERR_TRACE(-1);

    return 0;
}


static int start_record(struct recordContext *recContext)
{
#define MAX_PIDS 8
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_PidChannelHandle pidChannel[MAX_PIDS];
    NEXUS_RecordPidChannelSettings pidSettings;
    unsigned total_pids = 0;
    unsigned program = recContext->settings.program;
    unsigned i;

    if(recContext->state.started)
        return rc;

    memset(pidChannel, 0, sizeof(pidChannel));

    if(recContext->settings.playback.filename){
        recContext->handles.playback.file = NEXUS_FilePlay_OpenPosix(recContext->settings.playback.filename, NULL);
        if (!recContext->handles.playback.file) {
            BDBG_ERR(("unable to open %s", recContext->settings.playback.filename));
            return -1;
        }
    } else {
        recContext->handles.frontend = acquire_frontend(&recContext->settings.tune_settings);
        if (recContext->settings.tune_settings.source != channel_source_streamer && !recContext->handles.frontend) {
            /* TODO: there's a race condition between scan and tune. we may get a frontend to scan, but be
            unable to get it again for tune. to fix, we should not release it. */
            BDBG_ERR(("unable to acquire frontend"));
            return -1;
        }
        rc = tune(recContext->handles.parserBand, recContext->handles.frontend, &recContext->settings.tune_settings, false);
        if (rc) return BERR_TRACE(rc);
    }
    /* if no filename, then we default the filename and indexname.
    if filename, don't default indexname: the user doesn't want an index. */
    if (!recContext->settings.filename) {
        NEXUS_RecpumpStatus status;
        NEXUS_Recpump_GetStatus(recContext->handles.recpump, &status);
        if (!recContext->settings.filename) {
            recContext->settings.filename = default_filename;
            snprintf(default_filename, sizeof(default_filename), "videos/stream%d.mpg", status.rave.index);
        }
        if (!recContext->settings.indexname) {
            recContext->settings.indexname = default_indexname;
            snprintf(default_indexname, sizeof(default_indexname), "videos/stream%d.nav", status.rave.index);
        }
    }

    if (recContext->settings.timeshift) {
        if (recContext->settings.chunkSize) {
            recContext->handles.chunkedFifofile = NEXUS_ChunkedFifoRecord_Create(recContext->settings.filename, recContext->settings.indexname);
            recContext->handles.file = NEXUS_ChunkedFifoRecord_GetFile(recContext->handles.chunkedFifofile);
        }
        else {
            recContext->handles.fifofile = NEXUS_FifoRecord_Create(recContext->settings.indexonly?NULL:recContext->settings.filename, recContext->settings.indexname);
            recContext->handles.file = NEXUS_FifoRecord_GetFile(recContext->handles.fifofile);
        }
        set_fifo_settings(recContext, recContext->settings.timeshift, 20*1024*1024);
    }
    else if (recContext->settings.append) {
        recContext->handles.file = NEXUS_FileRecord_AppendPosix(recContext->settings.filename, recContext->settings.indexname);
    }
    else if (recContext->settings.chunkSize) {
        NEXUS_ChunkedFileRecordOpenSettings chunkedFileRecordOpenSettings;
        NEXUS_ChunkedFileRecord_GetDefaultOpenSettings(&chunkedFileRecordOpenSettings);
        chunkedFileRecordOpenSettings.chunkSize = recContext->settings.chunkSize*1024*1024;
        strcpy(chunkedFileRecordOpenSettings.chunkTemplate, "%s_%04u");
        recContext->handles.file = NEXUS_ChunkedFileRecord_Open(recContext->settings.filename, recContext->settings.indexname, &chunkedFileRecordOpenSettings);
    }
    else {
        recContext->handles.file = NEXUS_FileRecord_OpenPosix(recContext->settings.indexonly?NULL:recContext->settings.filename, recContext->settings.indexname);
    }

    if (recContext->settings.allpass) {
        if (recContext->handles.playback.playback) {
            NEXUS_PlaybackPidChannelSettings pidCfg0;
            NEXUS_Playback_GetDefaultPidChannelSettings(&pidCfg0);
            NEXUS_Playpump_GetAllPassPidChannelIndex(recContext->handles.playback.playpump, &pidCfg0.pidSettings.pidSettings.pidChannelIndex);
            pidChannel[total_pids] = NEXUS_Playback_OpenPidChannel(recContext->handles.playback.playback, 0x0, &pidCfg0);
        }
        else {
            NEXUS_PidChannelSettings pidCfg0;
            NEXUS_ParserBandSettings parserBandSettings;

            NEXUS_PidChannel_GetDefaultSettings(&pidCfg0);
            NEXUS_ParserBand_GetAllPassPidChannelIndex(recContext->handles.parserBand, &pidCfg0.pidChannelIndex);
            pidChannel[total_pids] = NEXUS_PidChannel_Open(recContext->handles.parserBand, 0x0, &pidCfg0);

            NEXUS_ParserBand_GetSettings(recContext->handles.parserBand, &parserBandSettings);
            parserBandSettings.allPass = true;
            parserBandSettings.acceptNullPackets = true;
            NEXUS_ParserBand_SetSettings(recContext->handles.parserBand, &parserBandSettings);
        }
        if (pidChannel[total_pids]) {
            NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
            if (recContext->settings.indexonly) {
                pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
                pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
                pidSettings.recpumpSettings.pidTypeSettings.video.codec = recContext->scan_results.program_info[program].video_pids[0].codec;
                pidSettings.recpumpSettings.pidTypeSettings.video.pid = recContext->scan_results.program_info[program].video_pids[0].pid;
            }
            NEXUS_Record_AddPidChannel(recContext->handles.record, pidChannel[total_pids], &pidSettings);
            total_pids++;
        }
    }
    else {
        for (i=0;i<recContext->scan_results.program_info[program].num_video_pids && total_pids < MAX_PIDS;i++) {
            NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
            pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
            if (i == 0 && recContext->settings.indexname) {
                pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
                pidSettings.recpumpSettings.pidTypeSettings.video.codec = recContext->scan_results.program_info[program].video_pids[i].codec;
            }
            if (recContext->handles.playback.playback) {
                pidChannel[total_pids] = NEXUS_Playback_OpenPidChannel(recContext->handles.playback.playback, recContext->scan_results.program_info[program].video_pids[i].pid, NULL);
            }
            else {
                pidChannel[total_pids] = NEXUS_PidChannel_Open(recContext->handles.parserBand, recContext->scan_results.program_info[program].video_pids[i].pid, NULL);
            }
            if (pidChannel[total_pids]) {
                NEXUS_Record_AddPidChannel(recContext->handles.record, pidChannel[total_pids], &pidSettings);
                total_pids++;
            }
        }

        for (i=0;i<recContext->scan_results.program_info[program].num_audio_pids && total_pids < MAX_PIDS;i++) {
            NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
            pidSettings.recpumpSettings.pidType = NEXUS_PidType_eAudio;
            if (recContext->handles.playback.playback) {
                pidChannel[total_pids] = NEXUS_Playback_OpenPidChannel(recContext->handles.playback.playback, recContext->scan_results.program_info[program].audio_pids[i].pid, NULL);
            }
            else {
                pidChannel[total_pids] = NEXUS_PidChannel_Open(recContext->handles.parserBand, recContext->scan_results.program_info[program].audio_pids[i].pid, NULL);
            }
            if (pidChannel[total_pids]) {
                NEXUS_Record_AddPidChannel(recContext->handles.record, pidChannel[total_pids], &pidSettings);
                total_pids++;
            }
        }
    }

    if (recContext->settings.encrypt_algo < NEXUS_SecurityAlgorithm_eMax) {
        struct dvr_crypto_settings settings;
        unsigned i;
        dvr_crypto_get_default_settings(&settings);
        settings.algo = recContext->settings.encrypt_algo;
        settings.encrypt = true;
        for (i=0;i<total_pids;i++) {
            settings.pid[i] = pidChannel[i];
        }
        recContext->handles.crypto = dvr_crypto_create(&settings);

        recContext->scan_results.program_info[program].other_pids[0].pid = NXAPPS_DVR_CRYPTO_TAG_PID_BASE + recContext->settings.encrypt_algo;
        recContext->scan_results.program_info[program].num_other_pids = 1;
    }

    if (recContext->settings.append) {
        NEXUS_RecordAppendSettings appendSettings;
        NEXUS_Record_GetDefaultAppendSettings(&appendSettings);
        get_stats_from_previous_recording(recContext->settings.filename, recContext->settings.indexname, &appendSettings.startingOffset, &appendSettings.timestamp);
        rc = NEXUS_Record_StartAppend(recContext->handles.record, recContext->handles.file, &appendSettings);
        BDBG_ASSERT(!rc);

    }
    else {
        rc = NEXUS_Record_Start(recContext->handles.record, recContext->handles.file);
        BDBG_ASSERT(!rc);
    }

    if (recContext->settings.psi_injection == NEXUS_TristateEnable_eNotSet) {
        recContext->settings.psi_injection = recContext->handles.crypto ? NEXUS_TristateEnable_eEnable : NEXUS_TristateEnable_eDisable;
    }
    if (recContext->handles.playback.playback) {
        rc = NEXUS_Playback_Start(recContext->handles.playback.playback, recContext->handles.playback.file, NULL);
        if (rc) {BERR_TRACE(rc); return -1;}
        NEXUS_Record_AddPlayback(recContext->handles.record, recContext->handles.playback.playback);
    }
    else {
        if (recContext->settings.psi_injection == NEXUS_TristateEnable_eEnable) {
            /* TODO: PSI injection for playback */
            recContext->handles.psi = bpsi_injection_open(recContext->handles.parserBand, &recContext->scan_results, program);
            if (recContext->handles.psi) {
                NEXUS_Record_AddPidChannel(recContext->handles.record, bpsi_injection_get_pid_channel(recContext->handles.psi), NULL);
            }
        }
    }

    recContext->state.started = true;

    return rc;
}

static void stop_record(struct recordContext *recContext)
{
    if(!recContext->state.started)
        return;

    if (recContext->handles.playback.playback) {
        NEXUS_Record_RemovePlayback(recContext->handles.record, recContext->handles.playback.playback);
    }
    NEXUS_Record_Stop(recContext->handles.record);
    if (recContext->handles.psi) {
        NEXUS_Record_RemovePidChannel(recContext->handles.record, bpsi_injection_get_pid_channel(recContext->handles.psi));
        bpsi_injection_close(recContext->handles.psi);
    }
    if (recContext->handles.crypto) {
        dvr_crypto_destroy(recContext->handles.crypto);
    }
    NEXUS_FileRecord_Close(recContext->handles.file);
    if (recContext->handles.playback.playback) {
        NEXUS_Playback_Stop(recContext->handles.playback.playback);
        NEXUS_FilePlay_Close(recContext->handles.playback.file);
    }
#if NEXUS_HAS_FRONTEND
    if(recContext->handles.frontend) {
        NEXUS_Frontend_Release(recContext->handles.frontend);
    }
#endif
    recContext->state.started = false;
}

static void *standby_monitor(void *context)
{
    NEXUS_Error rc;
    struct recordContext *recContext = context;
    NxClient_StandbyStatus standbyStatus, prevStatus;

    rc = NxClient_GetStandbyStatus(&prevStatus);
    if (rc) exit(0); /* server is down, exit gracefully */

    while(!recContext->state.quit) {
        rc = NxClient_GetStandbyStatus(&standbyStatus);
        if (rc) exit(0); /* server is down, exit gracefully */

        if(standbyStatus.settings.mode == NEXUS_PlatformStandbyMode_ePassive || standbyStatus.settings.mode == NEXUS_PlatformStandbyMode_eDeepSleep) {
                stop_record(recContext);
        } else if((standbyStatus.settings.mode == NEXUS_PlatformStandbyMode_eOn || standbyStatus.settings.mode == NEXUS_PlatformStandbyMode_eActive)) {
                start_record(recContext);
        }
        if(standbyStatus.transition == NxClient_StandbyTransition_eAckNeeded) {
            printf("'record' acknowledges standby state: %s\n", lookup_name(g_platformStandbyModeStrs, standbyStatus.settings.mode));
            NxClient_AcknowledgeStandby(true);
        }
        recContext->state.mode = standbyStatus.settings.mode;

        prevStatus = standbyStatus;
        BKNI_Sleep(100);
    }

    return NULL;
}

static void init_settings(struct recordContext *recContext)
{
    memset(recContext, 0, sizeof(struct recordContext));
    recContext->settings.program = 0;
    recContext->settings.timeshift = 0;
    recContext->settings.append = false;
    recContext->settings.encrypt_algo = NEXUS_SecurityAlgorithm_eMax;
    recContext->settings.timestampType = NEXUS_TransportTimestampType_eNone;
    recContext->settings.psi_injection = NEXUS_TristateEnable_eNotSet;
    recContext->settings.filename = NULL;
    recContext->settings.indexname = NULL;
    recContext->settings.playback.filename = NULL;
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BKNI_EventHandle event;
    int curarg = 1;
    char source_str[64];
    bchannel_scan_t scan = NULL;
    bchannel_scan_start_settings scan_settings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecordSettings recordSettings;
    bool prompt = false;
    bool gui = true;
    brecord_gui_t record_gui = NULL;
    char sourceName[64];
    bool quiet = false;
    unsigned timeout = 0, starttime;
    struct nxapps_cmdline cmdline;
    int n;
    pthread_t standby_thread_id;
    struct recordContext recContext;
    NxClient_StandbyStatus standbyStatus;
    bool do_scan;

    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_frontend);

    init_settings(&recContext);

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage(&cmdline);
            return 0;
        }
        else if (!strcmp(argv[curarg], "-file") && argc>curarg+1) {
            recContext.settings.playback.filename = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-program") && argc>curarg+1) {
            recContext.settings.program = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-prompt")) {
            prompt = true;
        }
        else if (!strcmp(argv[curarg], "-timeshift") && argc>curarg+1) {
            recContext.settings.timeshift = atoi(argv[++curarg]);
            if (!recContext.settings.timeshift) {
                print_usage(&cmdline);
                return 1;
            }
        }
        else if (!strcmp(argv[curarg], "-gui") && argc>curarg+1) {
            gui = strcmp(argv[++curarg], "off");
        }
        else if (!strcmp(argv[curarg], "-crypto") && curarg+1 < argc) {
            recContext.settings.encrypt_algo = lookup(g_securityAlgoStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-q")) {
            quiet = true;
        }
        else if (!strcmp(argv[curarg], "-tts")) {
            recContext.settings.timestampType = NEXUS_TransportTimestampType_eBinary;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-psi") && argc>curarg+1) {
            recContext.settings.psi_injection = parse_boolean(argv[++curarg]) ? NEXUS_TristateEnable_eEnable : NEXUS_TristateEnable_eDisable;
        }
        else if (!strcmp(argv[curarg], "-append")) {
            recContext.settings.append = true;
        }
        else if (!strcmp(argv[curarg], "-chunk") && argc>curarg+1) {
            recContext.settings.chunkSize = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-allpass")) {
            recContext.settings.allpass = true;
        }
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else if (!recContext.settings.filename) {
            recContext.settings.filename = argv[curarg];
        }
        else if (!recContext.settings.indexname) {
            recContext.settings.indexname = argv[curarg];
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

    recContext.settings.indexonly = recContext.settings.filename && !strcmp(recContext.settings.filename, "none");
    if (recContext.settings.indexonly) {
        /* index-only requires allpass because stream is unfiltered */
        recContext.settings.allpass = true;
    }

    do_scan = !recContext.settings.allpass || recContext.settings.indexonly;

    if (recContext.settings.playback.filename) {
        struct probe_request probe_request;
        struct probe_results probe_results;
        NEXUS_PlaybackSettings playbackSettings;

        probe_media_get_default_request(&probe_request);
        probe_request.streamname = recContext.settings.playback.filename;
        probe_request.program = recContext.settings.program;
        rc = probe_media_request(&probe_request, &probe_results);
        if (rc) {
            BDBG_ERR(("unable to probe %s", recContext.settings.playback.filename));
            return -1;
        }

        recContext.handles.playback.playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        recContext.handles.playback.playback = NEXUS_Playback_Create();
        NEXUS_Playback_GetSettings(recContext.handles.playback.playback, &playbackSettings);
        playbackSettings.playpump = recContext.handles.playback.playpump;
        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause; /* once */
        playbackSettings.playpumpSettings.timestamp.type = probe_results.timestampType;
        if (recContext.settings.allpass) {
            playbackSettings.playpumpSettings.allPass = true;
            playbackSettings.playpumpSettings.acceptNullPackets = true;
        }
        rc = NEXUS_Playback_SetSettings(recContext.handles.playback.playback, &playbackSettings);
        BDBG_ASSERT(!rc);

        if (do_scan) {
            /* convert to scan_results */
            strncpy(source_str, recContext.settings.playback.filename, sizeof(source_str)-1);
            convert_to_scan_results(&recContext.scan_results, &probe_results);
            printf("playing video %#x:%s, audio %#x:%s\n",
                recContext.scan_results.program_info[0].video_pids[0].pid,
                lookup_name(g_videoCodecStrs, recContext.scan_results.program_info[0].video_pids[0].codec),
                recContext.scan_results.program_info[0].audio_pids[0].pid,
                lookup_name(g_audioCodecStrs, recContext.scan_results.program_info[0].audio_pids[0].codec));
            /* media probe picks the program and convert_to_scan_results just puts it in program 0 */
            recContext.settings.program = 0;
        }
    }
    else {
        get_default_channels(&cmdline.frontend.tune, &cmdline.frontend.freq_list);
        if (cmdline.frontend.freq_list) {
            float f;
            if (sscanf(cmdline.frontend.freq_list, "%f", &f) != 1) f = 0;
            if (f <  1000000) {
                /* convert to Hz, rounding to nearest 1000 */
                cmdline.frontend.tune.freq = (unsigned)(f * 1000) * 1000;
            }
            else {
                cmdline.frontend.tune.freq = f;
            }
        }
        recContext.settings.tune_settings = cmdline.frontend.tune;

        if (do_scan) {
            bchannel_source_print(source_str, sizeof(source_str), &cmdline.frontend.tune);
            printf("Scanning %s... ", source_str);
            fflush(stdout);

            bchannel_scan_get_default_start_settings(&scan_settings);
            scan_settings.tune = cmdline.frontend.tune;
            scan_settings.scan_done = complete;
            scan_settings.context = event;
            scan = bchannel_scan_start(&scan_settings);
            if (!scan) {
                BDBG_ERR(("unable to tune/scan"));
                goto err_start_scan;
            }
            BKNI_WaitForEvent(event, 2000);
            bchannel_scan_get_results(scan, &recContext.scan_results);
            printf("%d programs found\n", recContext.scan_results.num_programs);
            if (recContext.settings.program >= recContext.scan_results.num_programs) {
                BDBG_ERR(("program %d unavailable", recContext.settings.program));
                goto err_scan_results;
            }

            bchannel_scan_stop(scan);
            scan = NULL;
        }
    }
    if (do_scan) {
        snprintf(sourceName, sizeof(sourceName), "%s, %#x/%#x", source_str,
            recContext.scan_results.program_info[recContext.settings.program].video_pids[0].pid,
            recContext.scan_results.program_info[recContext.settings.program].audio_pids[0].pid);
    }
    else {
        snprintf(sourceName, sizeof(sourceName), "%s, allpass", source_str);
    }

    recContext.handles.parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);

    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    recpumpOpenSettings.data.bufferSize = 192512 * 8; /* limit to 1.5 MB because of limited eFull client heap space */
    recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.bufferSize/4;
    if (recContext.settings.filename && !recContext.settings.indexname) {
        recpumpOpenSettings.index.bufferSize = 0;
    }
    recContext.handles.recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
    if (!recContext.handles.recpump) {
        BDBG_ERR(("unable to open recpump"));
        return -1;
    }

    recContext.handles.record = NEXUS_Record_Create();

    NEXUS_Record_GetSettings(recContext.handles.record, &recordSettings);
    recordSettings.recpump = recContext.handles.recpump;
    recordSettings.recpumpSettings.timestampType = recContext.settings.timestampType;
    if (recContext.settings.playback.filename) {
        recordSettings.recpumpSettings.bandHold = true; /* pace playback to avoid overflow */
        recordSettings.writeAllTimeout          = 1000; /* wait if necessary for slow record media */
    }
    NEXUS_Record_SetSettings(recContext.handles.record, &recordSettings);

    rc = start_record(&recContext);
    if(rc) return -1;

    rc = NxClient_GetStandbyStatus(&standbyStatus);
    BDBG_ASSERT(!rc);
    if (gui && standbyStatus.settings.mode == NEXUS_PlatformStandbyMode_eOn) {
        struct brecord_gui_settings settings;
        brecord_gui_get_default_settings(&settings);
        settings.sourceName = sourceName;
        settings.destName = recContext.settings.filename;
        settings.record = recContext.handles.record;
        settings.color = 0xFF0000FF;
        record_gui = brecord_gui_create(&settings);
    } else {
        printf("Starting record without gui\n");
    }

    BDBG_WRN(("recording to %s%s%s", recContext.settings.filename, recContext.settings.indexname?" and ":"", recContext.settings.indexname?recContext.settings.indexname:""));

    pthread_create(&standby_thread_id, NULL, standby_monitor, &recContext);

    starttime = b_get_time();
    while (1) {
        if (prompt) {
            char buf[64];
            printf("record>");
            fflush(stdout);
            fgets(buf, sizeof(buf), stdin);
            if (feof(stdin)) break;
            buf[strlen(buf)-1] = 0; /* chop off \n */
            if (!strcmp(buf, "?") || !strcmp(buf, "h")) {
                printf(
                "st         - status\n"
                "play       - start timeshifting playback\n"
                "timeshift(interval,maxBiteRate)\n"
                "export(BEGIN,END) - export ChunkedFile from timeshifting ChunkedFileFifo, BEGIN,END in seconds\n"
                "q - quit\n"
                );
            }
            else if (!strcmp(buf, "q")) {
                break;
            }
            else if (!strcmp(buf, "st")) {
                print_record_status(&recContext);
            }
            else if (recContext.settings.timeshift && !strcmp(buf, "play")) {
                /* TODO: this is a very simple addition of playback. a better integration could be done */
                start_play(recContext.handles.record, recContext.handles.fifofile, recContext.settings.filename, recContext.settings.indexname, &recContext.scan_results, recContext.settings.program);
            }
            else if (recContext.settings.timeshift && strstr(buf, "timeshift")) {
                unsigned interval, maxBitRate, n;
                n = sscanf(buf+10, "%u,%u", &interval, &maxBitRate);
                if (n == 2 && interval && maxBitRate) {
                    set_fifo_settings(&recContext, interval, maxBitRate*1024*1024);
                }
            }
            else if (recContext.settings.timeshift && recContext.settings.chunkSize && strstr(buf, "export") == buf) {
                NEXUS_ChunkedFifoRecordExportSettings settings;
                NEXUS_ChunkedFifoRecordExportHandle exportHandle;
                unsigned begin_time, end_time;
                sscanf(buf+7, "%u,%u", &begin_time, &end_time);
                NEXUS_ChunkedFifoRecord_GetDefaultExportSettings(&settings);
                settings.filename = "videos/chunk_stream.mpg"; /* match nexus/examples/dvr/record_chunk name */
                settings.indexname = "videos/chunk_stream.nav";
                settings.first.timestamp = begin_time * 1000;
                settings.last.timestamp = end_time * 1000;
                BDBG_WRN(("exporting %u..%u to %s", begin_time, end_time, settings.filename));
                exportHandle = NEXUS_ChunkedFifoRecord_StartExport(recContext.handles.chunkedFifofile, &settings);
                if (exportHandle) {
                    NEXUS_ChunkedFifoRecordExportStatus status;
                    while (1) {
                        NEXUS_ChunkedFifoRecord_GetExportStatus(exportHandle, &status);
                        if (status.state >= NEXUS_ChunkedFifoRecordExportState_eFailed) break;
                        BKNI_Sleep(250);
                    }
                    NEXUS_ChunkedFifoRecord_StopExport(exportHandle);
                    if (status.state == NEXUS_ChunkedFifoRecordExportState_eFailed) {
                        BDBG_ERR(("export failed"));
                    }
                    BDBG_WRN(("export done"));
                }
            }
        }
        else {
            static int cnt = 0;
            BKNI_Sleep(200);
            if (!quiet && ++cnt == 10) {
                print_record_status(&recContext);
                cnt = 0;
            }
            if (record_gui && recContext.state.mode == NEXUS_PlatformStandbyMode_eOn) {
                brecord_gui_update(record_gui);
            }
        }
        if (timeout && b_get_time() - starttime >= timeout*1000) {
            break;
        }
        if (recContext.handles.playback.playback && recContext.state.mode == NEXUS_PlatformStandbyMode_eOn) {
            NEXUS_PlaybackStatus status;
            NEXUS_Playback_GetStatus(recContext.handles.playback.playback, &status);
            if (status.state != NEXUS_PlaybackState_ePlaying) break;
        }

    }

    recContext.state.quit = true;
    pthread_join(standby_thread_id, NULL);

    if (record_gui) {
        brecord_gui_destroy(record_gui);
    }

    stop_record(&recContext);

    NEXUS_Record_Destroy(recContext.handles.record);
    NEXUS_Recpump_Close(recContext.handles.recpump);
    if (recContext.handles.playback.playback) {
        NEXUS_Playback_Destroy(recContext.handles.playback.playback);
        NEXUS_Playpump_Close(recContext.handles.playback.playpump);
    }
err_scan_results:
    if (scan) {
        bchannel_scan_stop(scan);
    }
err_start_scan:
    BKNI_DestroyEvent(event);
    NxClient_Uninit();
    return 0;
}

#include "nexus_playback.h"
int start_play(NEXUS_RecordHandle record, NEXUS_FifoRecordHandle fifofile, const char *filename, const char *indexname, tspsimgr_scan_results *pscan_results, unsigned program)
{
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    NEXUS_FilePlayHandle file;
    NEXUS_SimpleVideoDecoderHandle videoDecoder = NULL;
#if NEXUS_HAS_AUDIO
    NEXUS_SimpleAudioDecoderHandle audioDecoder = NULL;
#endif
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleAudioDecoderStartSettings audioProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_Error rc;

    rc = NxClient_Join(NULL);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    allocSettings.simpleAudioDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    file = NEXUS_FifoPlay_Open(filename, indexname, fifofile);
    if (!file) return -1;

    NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
    playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
    playback = NEXUS_Playback_Create();
    BDBG_ASSERT(playback);
    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);

    NEXUS_Playback_GetSettings(playback, &playbackSettings);
    playbackSettings.playpump = playpump;
    playbackSettings.simpleStcChannel = stcChannel;
    playbackSettings.stcTrick = true;
    playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePlay; /* when play hits the end, wait for record */
    playbackSettings.timeshifting = true;
    rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Record_AddPlayback(record, playback);
    BDBG_ASSERT(!rc);

    if (allocResults.simpleVideoDecoder[0].id) {
        videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    }
#if NEXUS_HAS_AUDIO
    if (allocResults.simpleAudioDecoder.id) {
        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
    }
#endif

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
#if NEXUS_HAS_AUDIO
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder;
    audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(playback, pscan_results->program_info[program].audio_pids[0].pid, &playbackPidSettings);
    audioProgram.primary.codec = pscan_results->program_info[program].audio_pids[0].codec;
#endif

    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = pscan_results->program_info[program].video_pids[0].codec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
    videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, pscan_results->program_info[program].video_pids[0].pid, &playbackPidSettings);
    videoProgram.settings.codec = playbackPidSettings.pidTypeSettings.video.codec;

    if (videoProgram.settings.pidChannel) {
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }
#if NEXUS_HAS_AUDIO
    if (audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }
#endif
    if (videoProgram.settings.pidChannel) {
        NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
    }
#if NEXUS_HAS_AUDIO
    if (audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
    }
#endif
    NEXUS_Playback_Start(playback, file, NULL);

    return 0;
}

#if 0
void stop_play(void)
{
    NEXUS_Playback_Stop(playback);
    NEXUS_Playback_Destroy(playback);
    NEXUS_Playpump_Close(playpump);
    NEXUS_FilePlay_Close(file);
    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
}
#endif
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform (needs playback, record, simple_decoder and transport)!\n");
    return 0;
}
#endif
