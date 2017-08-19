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
#if NEXUS_HAS_TRANSPORT
#include "nxclient.h"
#include "nexus_platform.h"
#include "nexus_parser_band.h"
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#endif
#include "namevalue.h"
#include "tspsimgr3.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

BDBG_MODULE(psi);

static void print_usage(void)
{
    printf(
        "Usage: psi OPTIONS [FILE]\n"
        "\n"
        "OPTIONS:\n"
        "  --help or -h for help\n"
        "  -streamer X\n"
    );
}

void print_scan(const tspsimgr_scan_results *results)
{
    unsigned i,j;
    if (!results->num_programs) {
        printf("no programs\n");
        return;
    }
    for (i=0;i<results->num_programs;i++) {
        printf("program %u (pid %#x):\n", results->program_info[i].program_number, results->program_info[i].map_pid);
        if (results->program_info[i].ca_pid) {
            printf("  ca_pid %#x\n", results->program_info[i].ca_pid);
        }
        for (j=0;j<results->program_info[i].num_video_pids;j++) {
            printf("  video %#x, codec %s\n",
                results->program_info[i].video_pids[j].pid,
                lookup_name(g_videoCodecStrs, results->program_info[i].video_pids[j].codec));
            if (results->program_info[i].video_pids[j].ca_pid) {
                printf("    ca_pid %#x\n", results->program_info[i].video_pids[j].ca_pid);
            }
        }
        for (j=0;j<results->program_info[i].num_audio_pids;j++) {
            printf("  audio %#x, codec %s\n",
                results->program_info[i].audio_pids[j].pid,
                lookup_name(g_audioCodecStrs, results->program_info[i].audio_pids[j].codec));
            if (results->program_info[i].audio_pids[j].ca_pid) {
                printf("    ca_pid %#x\n", results->program_info[i].audio_pids[j].ca_pid);
            }
        }
    }
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc = NEXUS_SUCCESS;
#if NEXUS_HAS_PLAYBACK
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaypumpHandle playpump = NULL;
    NEXUS_PlaybackHandle playback = NULL;
    NEXUS_FilePlayHandle file;
#endif
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    int curarg = 1;
    const char *filename = NULL;
    tspsimgr_t tspsimgr;
    tspsimgr_scan_settings scan_settings;
    tspsimgr_scan_results scan_results;
    unsigned streamer = 0, cnt = 0;

    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-streamer") && curarg+1 < argc) {
            streamer = atoi(argv[++curarg]);
        }
        else if (!filename) {
            filename = argv[curarg];
        }
        else {
            print_usage();
            return -1;
        }
        curarg++;
    }

    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

#if NEXUS_HAS_PLAYBACK
    if (filename) {
        playpump = NEXUS_Playpump_Open(0, NULL);
        BDBG_ASSERT(playpump);
        playback = NEXUS_Playback_Create();
        BDBG_ASSERT(playback);

        file = NEXUS_FilePlay_OpenPosix(filename, NULL);
        if (!file) {
            fprintf(stderr, "can't open file:%s\n", filename);
            return -1;
        }

        NEXUS_Playback_GetSettings(playback, &playbackSettings);
        playbackSettings.playpump = playpump;
        playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_ePause;
        /* playbackSettings.playpumpSettings.timestamp.type - would require SW probe, which does SW PSI scan too :-) */
        rc = NEXUS_Playback_SetSettings(playback, &playbackSettings);
        BDBG_ASSERT(!rc);

        rc = NEXUS_Playback_Start(playback, file, NULL);
        BDBG_ASSERT(!rc);
    }
    else
#endif
    {
        parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        NEXUS_Platform_GetStreamerInputBand(streamer, &parserBandSettings.sourceTypeSettings.inputBand);
        NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    }

    /* HW PSI scan */
    tspsimgr = tspsimgr_create();
    tspsimgr_get_default_start_scan_settings(&scan_settings);
    scan_settings.parserBand = parserBand;
#if NEXUS_HAS_PLAYBACK
    scan_settings.playpump = playpump;
#endif
    scan_settings.exclude_ca_pid = false;
    rc = tspsimgr_start_scan(tspsimgr, &scan_settings);
    if (rc) return BERR_TRACE(rc);

    /* polling loop */
    scan_results.num_programs = 0;
    while (tspsimgr_get_scan_results(tspsimgr, &scan_results)) {
        if (++cnt >= 10*100) break;
#if NEXUS_HAS_PLAYBACK
        if (cnt > 5 && playback) {
            /* don't check immediately because playback prints a warning for GetStatus with no decoders */
            NEXUS_PlaybackStatus status;
            NEXUS_Playback_GetStatus(playback, &status);
            if (status.state == NEXUS_PlaybackState_ePaused) break;
        }
#endif
        BKNI_Sleep(10);
    }

    print_scan(&scan_results);

    tspsimgr_destroy(tspsimgr);
#if NEXUS_HAS_PLAYBACK
    if (playpump) {
        NEXUS_Playback_Stop(playback);
        NEXUS_FilePlay_Close(file);
        NEXUS_Playback_Destroy(playback);
        NEXUS_Playpump_Close(playpump);
    }
    else
#endif
    {
        NEXUS_ParserBand_Close(parserBand);
    }
    NxClient_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
