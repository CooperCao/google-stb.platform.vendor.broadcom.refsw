/***************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/
#include "nxclient.h"
#include "nexus_platform_client.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

BDBG_MODULE(watchdog);

static void print_usage(void)
{
    printf(
    "Usage: watchdog [OPTIONS]\n"
    "\n"
    "Options:\n"
    "  --help or -h for help\n"
    "  -period X          set watchdog of X seconds (default is 30 seconds)\n"
    "  -sleep X           pet watchdog of X seconds (default is half of -period X)\n"
    "  -timeout X         after X seconds, clear watchdog and exit\n"
    "  -stall X           stall after X loops, causing system watchdog\n"
    );
}

static bool g_exit = false;
static void sig_handler(int signum)
{
    if (signum == SIGTERM) {
        g_exit = true;
    }
}

int main(int argc, const char **argv)
{
    NxClient_JoinSettings joinSettings;
    NEXUS_Error rc;
    int curarg = 1;
    unsigned timeout = 0;
    unsigned period = 30;
    unsigned sleep_time = 0;
    unsigned loops = 0;
    bool quiet = false;
    unsigned stall = 0;

    while (argc > curarg) {
        if (!strcmp(argv[curarg], "--help") || !strcmp(argv[curarg], "-h")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-timeout") && curarg+1 < argc) {
            timeout = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-period") && curarg+1 < argc) {
            period = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-sleep") && curarg+1 < argc) {
            sleep_time = atoi(argv[++curarg]);
        }
        else if (!strcmp(argv[curarg], "-q")) {
            quiet = true;
        }
        else if (!strcmp(argv[curarg], "-stall") && curarg+1 < argc) {
            stall = atoi(argv[++curarg]);
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }
    if (!sleep_time) sleep_time = period / 2;

    signal(SIGTERM, sig_handler);

    /* connect to server and nexus */
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;

    if (!quiet) BDBG_LOG(("start"));
    while (!timeout || (loops * sleep_time) < timeout) {
        unsigned i;
        if (g_exit) break;
        if (!quiet) BDBG_LOG(("period %u, sleep %u", period, sleep_time));
        rc = NxClient_SetWatchdogTimeout(period);
        if (rc) BERR_TRACE(rc);
        for (i=0;i<sleep_time && !g_exit;i++) {
            BKNI_Sleep(1000);
        }
        loops++;
        if (stall && loops == stall) {
            BDBG_WRN(("stalling after %u loops", stall));
            while (1) BKNI_Sleep(1000);
        }
    }
    if (!quiet) BDBG_LOG(("stop"));

    rc = NxClient_SetWatchdogTimeout(0);
    if (rc) BERR_TRACE(rc);

    NxClient_Uninit();
    return 0;
}
