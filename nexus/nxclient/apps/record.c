/******************************************************************************
 *    (c)2013 Broadcom Corporation
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
 *****************************************************************************/
#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_RECORD && NEXUS_HAS_SIMPLE_DECODER && NEXUS_HAS_TRANSPORT
#include "nxclient.h"
#if NEXUS_HAS_FRONTEND
#include "nexus_frontend.h"
#endif
#include "nexus_platform.h"
#include "nexus_record.h"
#include "nexus_file_fifo.h"
#include "nexus_parser_band.h"
#include "live_decode.h"
#include "live_source.h"
#include "tspsimgr3.h"
#include "namevalue.h"
#include "brecord_gui.h"
#include "dvr_crypto.h"
#include "media_probe.h"
#include "nxapps_cmdline.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

BDBG_MODULE(record);

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
    print_list_option("crypto",g_securityAlgoStrs);
    printf(
        "  -psi {on|off}            inject PSI into recording. defaults on with crypto.\n"
        );
}

static void print_record_status(const char *filename, const char *indexname, NEXUS_RecordHandle record, bool indexonly)
{
    NEXUS_RecordStatus status;
    NEXUS_Record_GetStatus(record, &status);
    if (indexonly) {
        status.recpumpStatus.data.bytesRecorded = 0;
    }
    BDBG_WRN(("%s %llu bytes (%d%%), %s %u (%d%%), %d seconds, %d pictures",
        filename,
        status.recpumpStatus.data.bytesRecorded,
        status.recpumpStatus.data.fifoSize ? status.recpumpStatus.data.fifoDepth*100/status.recpumpStatus.data.fifoSize : 0,
        indexname,
        (unsigned)status.recpumpStatus.index.bytesRecorded,
        status.recpumpStatus.index.fifoSize ? status.recpumpStatus.index.fifoDepth*100/status.recpumpStatus.index.fifoSize : 0,
        status.lastTimestamp / 1000,
        status.picturesIndexed));
}

static int set_fifo_settings(NEXUS_FifoRecordHandle fifofile, unsigned interval, unsigned maxBitRate)
{
    NEXUS_Error rc;
    NEXUS_FifoRecordSettings fifoRecordSettings;
    NEXUS_FifoRecord_GetSettings(fifofile, &fifoRecordSettings);
    fifoRecordSettings.interval = interval * 60;
    fifoRecordSettings.data.soft =(uint64_t)fifoRecordSettings.interval * (maxBitRate / 8);
    fifoRecordSettings.data.hard = fifoRecordSettings.data.soft*2;
#define B_DATA_ALIGN  ((188/4)*4096)
    fifoRecordSettings.data.soft -= fifoRecordSettings.data.soft % B_DATA_ALIGN;
    fifoRecordSettings.data.hard -= fifoRecordSettings.data.hard % B_DATA_ALIGN;
    fifoRecordSettings.index.soft = fifoRecordSettings.data.soft / 20;
    fifoRecordSettings.index.hard = fifoRecordSettings.index.soft*2;
    rc = NEXUS_FifoRecord_SetSettings(fifofile, &fifoRecordSettings);
    if (rc) return BERR_TRACE(rc);
    BDBG_WRN(("timeshift %d minutes, %d Mbps for file limit of %d MB", interval, maxBitRate/1024/1024, (unsigned)(fifoRecordSettings.data.soft/1024/1024)));
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

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    BKNI_EventHandle event;
    unsigned i;
    int curarg = 1;
    char source_str[64];
    unsigned program = 0;
    NEXUS_FrontendHandle frontend = NULL;
    bchannel_scan_t scan = NULL;
    bchannel_scan_start_settings scan_settings;
    tspsimgr_scan_results scan_results;
    NEXUS_ParserBand parserBand;
    NEXUS_FifoRecordHandle fifofile = NULL;
    NEXUS_FileRecordHandle file;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;
    NEXUS_RecordHandle record;
    NEXUS_RecordPidChannelSettings pidSettings;
    NEXUS_RecordSettings recordSettings;
    const char *filename = NULL;
    const char *indexname = NULL;
    char default_filename[64], default_indexname[64];
    struct {
        const char *filename;
        NEXUS_PlaypumpHandle playpump;
        NEXUS_PlaybackHandle playback;
        NEXUS_FilePlayHandle file;
    } playback;
    bool indexonly;
#define MAX_PIDS 8
    NEXUS_PidChannelHandle pidChannel[MAX_PIDS];
    unsigned total_pids = 0;
    bool prompt = false;
    bool gui = true;
    brecord_gui_t record_gui = NULL;
    char sourceName[64];
    unsigned timeshift = 0;
    dvr_crypto_t crypto = NULL;
    NEXUS_SecurityAlgorithm encrypt_algo = NEXUS_SecurityAlgorithm_eMax;
    bpsi_injection_t psi = NULL;
    bool quiet = false;
    NEXUS_TransportTimestampType timestampType = NEXUS_TransportTimestampType_eNone;
    unsigned timeout = 0, starttime;
    struct nxapps_cmdline cmdline;
    int n;
    NEXUS_TristateEnable psi_injection = NEXUS_TristateEnable_eNotSet;

    nxapps_cmdline_init(&cmdline);
    nxapps_cmdline_allow(&cmdline, nxapps_cmdline_type_frontend);

    memset(pidChannel, 0, sizeof(pidChannel));
    memset(&playback, 0, sizeof(playback));
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage(&cmdline);
            return 0;
        }
        else if (!strcmp(argv[curarg], "-file") && argc>curarg+1) {
            playback.filename = argv[++curarg];
        }
        else if (!strcmp(argv[curarg], "-program") && argc>curarg+1) {
            program = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-prompt")) {
            prompt = true;
        }
        else if (!strcmp(argv[curarg], "-timeshift") && argc>curarg+1) {
            timeshift = atoi(argv[++curarg]);
            if (!timeshift) {
                print_usage(&cmdline);
                return 1;
            }
        }
        else if (!strcmp(argv[curarg], "-gui") && argc>curarg+1) {
            gui = strcmp(argv[++curarg], "off");
        }
        else if (!strcmp(argv[curarg], "-crypto") && curarg+1 < argc) {
            encrypt_algo = lookup(g_securityAlgoStrs, argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-q")) {
            quiet = true;
        }
        else if (!strcmp(argv[curarg], "-tts")) {
            timestampType = NEXUS_TransportTimestampType_eBinary;
        }
        else if (!strcmp(argv[curarg], "-timeout") && argc>curarg+1) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-psi") && argc>curarg+1) {
            psi_injection = parse_boolean(argv[++curarg]) ? NEXUS_TristateEnable_eEnable : NEXUS_TristateEnable_eDisable;
        }
        else if ((n = nxapps_cmdline_parse(curarg, argc, argv, &cmdline))) {
            if (n < 0) {
                print_usage(&cmdline);
                return -1;
            }
            curarg += n;
        }
        else if (!filename) {
            filename = argv[curarg];
        }
        else if (!indexname) {
            indexname = argv[curarg];
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

    indexonly = filename && !strcmp(filename, "none");
    if (playback.filename) {
        struct probe_request probe_request;
        struct probe_results probe_results;
        NEXUS_PlaybackSettings playbackSettings;

        probe_media_get_default_request(&probe_request);
        probe_request.streamname = playback.filename;
        probe_request.program = program;
        rc = probe_media_request(&probe_request, &probe_results);
        if (rc) {
            BDBG_ERR(("unable to probe %s", playback.filename));
            return -1;
        }

        playback.file = NEXUS_FilePlay_OpenPosix(playback.filename, NULL);
        if (!playback.file) {
            BDBG_ERR(("unable to open %s", playback.filename));
            return -1;
        }
        playback.playpump = NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL);
        playback.playback = NEXUS_Playback_Create();
        NEXUS_Playback_GetSettings(playback.playback, &playbackSettings);
        playbackSettings.playpump = playback.playpump;
        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause; /* once */
        if (indexonly) {
            playbackSettings.playpumpSettings.allPass = true;
            playbackSettings.playpumpSettings.acceptNullPackets = true;
        }
        rc = NEXUS_Playback_SetSettings(playback.playback, &playbackSettings);
        BDBG_ASSERT(!rc);

        /* convert to scan_results */
        strncpy(source_str, playback.filename, sizeof(source_str)-1);
        convert_to_scan_results(&scan_results, &probe_results);
        printf("playing video %#x:%s, audio %#x:%s\n",
            scan_results.program_info[0].video_pids[0].pid,
            lookup_name(g_videoCodecStrs, scan_results.program_info[0].video_pids[0].codec),
            scan_results.program_info[0].audio_pids[0].pid,
            lookup_name(g_audioCodecStrs, scan_results.program_info[0].audio_pids[0].codec));
        /* media probe picks the program and convert_to_scan_results just puts it in program 0 */
        program = 0;
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
        bchannel_scan_get_results(scan, &scan_results);
        printf("%d programs found\n", scan_results.num_programs);
        if (program >= scan_results.num_programs) {
            BDBG_ERR(("program %d unavailable", program));
            goto err_scan_results;
        }

        bchannel_scan_get_resources(scan, &frontend, &parserBand);
    }
    snprintf(sourceName, sizeof(sourceName), "%s, %#x/%#x", source_str,
        scan_results.program_info[program].video_pids[0].pid,
        scan_results.program_info[program].audio_pids[0].pid);

    NEXUS_Recpump_GetDefaultOpenSettings(&recpumpOpenSettings);
    recpumpOpenSettings.data.bufferSize = 192512 * 8; /* limit to 1.5 MB because of limited eFull client heap space */
    recpumpOpenSettings.data.dataReadyThreshold = recpumpOpenSettings.data.bufferSize/4;
    if (filename && !indexname) {
        recpumpOpenSettings.index.bufferSize = 0;
    }
    recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, &recpumpOpenSettings);
    if (!recpump) {
        BDBG_ERR(("unable to open recpump"));
        return -1;
    }

    record = NEXUS_Record_Create();

    NEXUS_Record_GetSettings(record, &recordSettings);
    recordSettings.recpump = recpump;
    recordSettings.recpumpSettings.timestampType = timestampType;
    if (playback.filename) {
        recordSettings.recpumpSettings.bandHold = true; /* pace playback to avoid overflow */
        recordSettings.writeAllTimeout          = 1000; /* wait if necessary for slow record media */
    }
    NEXUS_Record_SetSettings(record, &recordSettings);

    /* if no filename, then we default the filename and indexname.
    if filename, don't default indexname: the user doesn't want an index. */
    if (!filename) {
        NEXUS_RecpumpStatus status;
        NEXUS_Recpump_GetStatus(recpump, &status);
        if (!filename) {
            filename = default_filename;
            snprintf(default_filename, sizeof(default_filename), "videos/stream%d.mpg", status.rave.index);
        }
        if (!indexname) {
            indexname = default_indexname;
            snprintf(default_indexname, sizeof(default_indexname), "videos/stream%d.nav", status.rave.index);
        }
    }

    if (timeshift) {
        fifofile = NEXUS_FifoRecord_Create(indexonly?NULL:filename, indexname);
        set_fifo_settings(fifofile, timeshift, 20*1024*1024);
        file = NEXUS_FifoRecord_GetFile(fifofile);
    }
    else {
        file = NEXUS_FileRecord_OpenPosix(indexonly?NULL:filename, indexname);
    }

    if (playback.playback && indexonly) {
        /* index-only requires allpass because stream is unfiltered */
        NEXUS_PlaybackPidChannelSettings pidCfg0;
        NEXUS_Playback_GetDefaultPidChannelSettings(&pidCfg0);
        NEXUS_Playpump_GetAllPassPidChannelIndex(playback.playpump, (unsigned *)&pidCfg0.pidSettings.pidSettings.pidChannelIndex);
        pidChannel[total_pids] = NEXUS_Playback_OpenPidChannel(playback.playback, 0x0, &pidCfg0);
        if (pidChannel[total_pids]) {
            NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
            pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
            pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
            pidSettings.recpumpSettings.pidTypeSettings.video.codec = scan_results.program_info[program].video_pids[0].codec;
            pidSettings.recpumpSettings.pidTypeSettings.video.pid = scan_results.program_info[program].video_pids[0].pid;
            NEXUS_Record_AddPidChannel(record, pidChannel[total_pids], &pidSettings);
            total_pids++;
        }
    }
    else {
        for (i=0;i<scan_results.program_info[program].num_video_pids && total_pids < MAX_PIDS;i++) {
            NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
            pidSettings.recpumpSettings.pidType = NEXUS_PidType_eVideo;
            if (i == 0 && indexname) {
                pidSettings.recpumpSettings.pidTypeSettings.video.index = true;
                pidSettings.recpumpSettings.pidTypeSettings.video.codec = scan_results.program_info[program].video_pids[i].codec;
            }
            if (playback.playback) {
                pidChannel[total_pids] = NEXUS_Playback_OpenPidChannel(playback.playback, scan_results.program_info[program].video_pids[i].pid, NULL);
            }
            else {
                pidChannel[total_pids] = NEXUS_PidChannel_Open(parserBand, scan_results.program_info[program].video_pids[i].pid, NULL);
            }
            if (pidChannel[total_pids]) {
                NEXUS_Record_AddPidChannel(record, pidChannel[total_pids], &pidSettings);
                total_pids++;
            }
        }

        for (i=0;i<scan_results.program_info[program].num_audio_pids && total_pids < MAX_PIDS;i++) {
            NEXUS_Record_GetDefaultPidChannelSettings(&pidSettings);
            pidSettings.recpumpSettings.pidType = NEXUS_PidType_eAudio;
            if (playback.playback) {
                pidChannel[total_pids] = NEXUS_Playback_OpenPidChannel(playback.playback, scan_results.program_info[program].audio_pids[i].pid, NULL);
            }
            else {
                pidChannel[total_pids] = NEXUS_PidChannel_Open(parserBand, scan_results.program_info[program].audio_pids[i].pid, NULL);
            }
            if (pidChannel[total_pids]) {
                NEXUS_Record_AddPidChannel(record, pidChannel[total_pids], &pidSettings);
                total_pids++;
            }
        }
    }

    if (encrypt_algo < NEXUS_SecurityAlgorithm_eMax) {
        struct dvr_crypto_settings settings;
        unsigned i;
        dvr_crypto_get_default_settings(&settings);
        settings.algo = encrypt_algo;
        settings.encrypt = true;
        for (i=0;i<total_pids;i++) {
            settings.pid[i] = pidChannel[i];
        }
        crypto = dvr_crypto_create(&settings);

        scan_results.program_info[program].other_pids[0].pid = NXAPPS_DVR_CRYPTO_TAG_PID_BASE + encrypt_algo;
        scan_results.program_info[program].num_other_pids = 1;
    }

    rc = NEXUS_Record_Start(record, file);
    BDBG_ASSERT(!rc);

    if (psi_injection == NEXUS_TristateEnable_eNotSet) {
        psi_injection = crypto ? NEXUS_TristateEnable_eEnable : NEXUS_TristateEnable_eDisable;
    }
    if (playback.playback) {
        rc = NEXUS_Playback_Start(playback.playback, playback.file, NULL);
        if (rc) {BERR_TRACE(rc); return -1;}
        NEXUS_Record_AddPlayback(record, playback.playback);
    }
    else {
        if (psi_injection == NEXUS_TristateEnable_eEnable) {
            /* TODO: PSI injection for playback */
            psi = bpsi_injection_open(parserBand, &scan_results, program);
            if (psi) {
                NEXUS_Record_AddPidChannel(record, bpsi_injection_get_pid_channel(psi), NULL);
            }
        }
    }

    if (gui) {
        struct brecord_gui_settings settings;
        brecord_gui_get_default_settings(&settings);
        settings.sourceName = sourceName;
        settings.destName = filename;
        settings.record = record;
        settings.color = 0xFF0000FF;
        record_gui = brecord_gui_create(&settings);
    }

    BDBG_WRN(("recording to %s%s%s", filename, indexname?" and ":"", indexname?indexname:""));
    starttime = b_get_time();
    while (1) {
        if (prompt) {
            char buf[64];
            printf("record>");
            fflush(stdout);
            fgets(buf, sizeof(buf), stdin);
            if (feof(stdin)) break;
            buf[strlen(buf)-1] = 0; /* chop off \n */
            if (!strcmp(buf, "q")) {
                break;
            }
            else if (!strcmp(buf, "st")) {
                print_record_status(filename, indexname, record, indexonly);
            }
            else if (timeshift && !strcmp(buf, "play")) {
                /* TODO: this is a very simple addition of playback. a better integration could be done */
                start_play(record, fifofile, filename, indexname, &scan_results, program);
            }
            else if (timeshift && strstr(buf, "timeshift")) {
                unsigned interval, maxBitRate, n;
                n = sscanf(buf+10, "%u,%u", &interval, &maxBitRate);
                if (n == 2 && interval && maxBitRate) {
                    set_fifo_settings(fifofile, interval, maxBitRate*1024*1024);
                }
            }
        }
        else {
            static int cnt = 0;
            BKNI_Sleep(200);
            if (!quiet && ++cnt == 10) {
                print_record_status(filename, indexname, record, indexonly);
                cnt = 0;
            }
            if (record_gui) {
                brecord_gui_update(record_gui);
            }
        }
        if (timeout && b_get_time() - starttime >= timeout*1000) {
            break;
        }
        if (playback.playback) {
            NEXUS_PlaybackStatus status;
            NEXUS_Playback_GetStatus(playback.playback, &status);
            if (status.state != NEXUS_PlaybackState_ePlaying) break;
        }

    }

    if (record_gui) {
        brecord_gui_destroy(record_gui);
    }
    if (playback.playback) {
        NEXUS_Record_RemovePlayback(record, playback.playback);
    }
    NEXUS_Record_Stop(record);
    if (psi) {
        NEXUS_Record_RemovePidChannel(record, bpsi_injection_get_pid_channel(psi));
        bpsi_injection_close(psi);
    }
    if (crypto) {
        dvr_crypto_destroy(crypto);
    }
    NEXUS_FileRecord_Close(file);
    NEXUS_Record_Destroy(record);
    NEXUS_Recpump_Close(recpump);
    if (playback.playback) {
        NEXUS_Playback_Stop(playback.playback);
        NEXUS_Playback_Destroy(playback.playback);
        NEXUS_Playpump_Close(playback.playpump);
        NEXUS_FilePlay_Close(playback.file);
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
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleAudioDecoderHandle audioDecoder;
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
    if (allocResults.simpleAudioDecoder.id) {
        audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(allocResults.simpleAudioDecoder.id);
    }

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);

    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&audioProgram);
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);

    NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
    playbackPidSettings.pidTypeSettings.audio.simpleDecoder = audioDecoder;
    audioProgram.primary.pidChannel = NEXUS_Playback_OpenPidChannel(playback, pscan_results->program_info[program].audio_pids[0].pid, &playbackPidSettings);
    audioProgram.primary.codec = pscan_results->program_info[program].audio_pids[0].codec;

    playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
    playbackPidSettings.pidTypeSettings.video.codec = pscan_results->program_info[program].video_pids[0].codec;
    playbackPidSettings.pidTypeSettings.video.index = true;
    playbackPidSettings.pidTypeSettings.video.simpleDecoder = videoDecoder;
    videoProgram.settings.pidChannel = NEXUS_Playback_OpenPidChannel(playback, pscan_results->program_info[program].video_pids[0].pid, &playbackPidSettings);
    videoProgram.settings.codec = playbackPidSettings.pidTypeSettings.video.codec;

    if (videoProgram.settings.pidChannel) {
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }
    if (audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_SetStcChannel(audioDecoder, stcChannel);
    }
    if (videoProgram.settings.pidChannel) {
        NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
    }
    if (audioProgram.primary.pidChannel) {
        NEXUS_SimpleAudioDecoder_Start(audioDecoder, &audioProgram);
    }
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
