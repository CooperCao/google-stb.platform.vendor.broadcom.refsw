/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <sys/stat.h>

#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "ssdd.h"

static bool g_has_exit_signal = false;
static bool g_is_running = true; /* So if we don't run daemonized, SSDD_Running returns true */
static int g_lockfd = -1;

#define LOCK_FILE "/tmp/ssddlock"

bool SSDD_Running(void)
{
    return g_is_running;
}

BERR_Code SSDD_Singleton_Lock(void)
{
    g_lockfd = open(LOCK_FILE, O_RDWR | O_CREAT, 0700);
    if (g_lockfd < 0) {
        return BERR_NOT_AVAILABLE;
    }

    if (lockf(g_lockfd, F_TLOCK, 0) < 0) {
        return BERR_NOT_AVAILABLE;
    }

    return BERR_SUCCESS;
}

void SSDD_Singleton_Unlock(void)
{
    if (g_lockfd >= 0) {
        lockf(g_lockfd, F_ULOCK, 0);
        close(g_lockfd);
        unlink(LOCK_FILE);
        g_lockfd = -1;
    }
}

bool SSDD_Singleton_Lock_Is_Locked(void)
{
    BERR_Code rc = SSDD_Singleton_Lock();

    if (rc == BERR_SUCCESS) {
        SSDD_Singleton_Unlock();
        return false;
    }

    /* Lock is currently held */
    return true;
}

// signals setting flags to terminate the process
static void SSDD_Sighandler(int sig)
{
    switch (sig) {
    case SIGHUP:
        SSDD_Debug_printf("SIGHUP received.. no op\n");
        break;
    case SIGINT:
        SSDD_Debug_printf("SIGINT received, terminating\n");
        g_is_running = false;
        g_has_exit_signal = true;
        break;
    case SIGTERM:
        /* if this is result of reboot command, we may crash due to race condition during cleanup*/
        SSDD_Debug_printf("SIGTERM received, terminating\n");
        g_is_running = false;
        g_has_exit_signal = true;
        break;
    default:
        SSDD_Debug_printf("unhandled signal received\n");
        break;
    }
    return;
}

void SSDD_Daemonize(void)
{
    int i = fork();

    if (i < 0) {
        printf("fork failed\n");
        exit(1);
    }

    /* Parent process needs to exit */
    if (i > 0) {
        printf("Daemon process id:%d\n", i);
        exit(0);
    }

    /* New process group */
    pid_t p = setsid();
    if (p < 0) {
        SSDD_Debug_printf("setsid() failed\n");
        exit(1);
    }

    /* close any unnecessarily open FD */
    close(0);
    close(1);

    /* exclusive access to any files */
    umask(0077);

    g_is_running = true;

    signal(SIGHUP, SSDD_Sighandler);
    signal(SIGINT, SSDD_Sighandler);
    signal(SIGTERM, SSDD_Sighandler);

    return;
}
