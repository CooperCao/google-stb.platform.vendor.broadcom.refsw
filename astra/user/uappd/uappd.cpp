/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ***************************************************************************/

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#include "uappd.h"
#include "uappd_uapp.h"
#include "uappd_msg.h"

// Static non-const data from TzIoc class
tzioc_client_handle UserAppDmon::hClient;
int UserAppDmon::msgQ;
uint8_t UserAppDmon::clientId;

uint32_t UserAppDmon::uappCnt;
UserAppDmon::UserApp *UserAppDmon::pUApps[UAPP_NUM_MAX];
int UserAppDmon::reapPid;

uint32_t UserAppDmon::ufileCnt;
UserAppDmon::UserFile *UserAppDmon::pUFiles[UFILE_NUM_MAX];

extern "C" void sigchldHandler(int sig)
{
    // Save errno in case
    int saved_errno = errno;

    int status;
    int pid = 0;

    // Check zombie child processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // reap zombie child process
        UserAppDmon::uappReap(pid);
    }

    // Restore errno
    errno = saved_errno;
}

void UserAppDmon::init()
{
    // Open TZIOC client
    hClient = tzioc_client_open(
        "uappd",
        &msgQ,
        &clientId);

    if (!hClient) {
        LOGE("Failed to open TZIOC client");
        throw(-EIO);
    }

    // Install SIGCHLD handler
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigchldHandler;

    int err = sigaction(SIGCHLD, &sa, NULL);
    if (err) {
        LOGE("Failed to install SIGCHLD handler");
        throw(err);
    }

    LOGI("User app daemon initialized!");
}

void UserAppDmon::deinit()
{
    // Terminate all user apps
    for (uint32_t i = 0; i < UAPP_NUM_MAX; i++) {
        if (pUApps[i]) {
            pUApps[i]->stop();
        }
    }

    // Close TZIOC client
    if (hClient) {
        tzioc_client_close(hClient);
        hClient = NULL;
    }

    LOGI("User app daemon deinitialized!");
}

void UserAppDmon::run()
{
    LOGI("User app daemon running!");

    while (1) {
        static uint8_t msg[TZIOC_MSG_SIZE_MAX];
        struct tzioc_msg_hdr *pHdr = (struct tzioc_msg_hdr *)msg;

        // Handle all reaped user apps
        uappReapProc();

        // Receive incoming msg
        int err = tzioc_msg_receive(
            hClient,
            pHdr,
            TZIOC_MSG_SIZE_MAX,
            -1);

        if (err) {
            if (err != -EINTR) {
                LOGE("Error receiving TZIOC msg, err %d", err);
            }
            continue;
        }

        // Process incoming msg
        switch (pHdr->ucType) {
        case UAPPD_MSG_UAPP_START:
            uappStartProc(pHdr);
            break;
        case UAPPD_MSG_UAPP_STOP:
            uappStopProc(pHdr);
            break;
        case UAPPD_MSG_UAPP_GETID:
            uappGetIdProc(pHdr);
            break;
        case UAPPD_MSG_FILE_OPEN:
            ufileOpenProc(pHdr);
            break;
        case UAPPD_MSG_FILE_CLOSE:
            ufileCloseProc(pHdr);
            break;
        case UAPPD_MSG_FILE_WRITE:
            ufileWriteProc(pHdr);
            break;
        case UAPPD_MSG_FILE_READ:
            ufileReadProc(pHdr);
            break;
        case UAPPD_MSG_UAPP_COREDUMP:
            uappCoreDumpProc(pHdr);
            break;
        default:
            LOGW("Unknown uappd msg %d", pHdr->ucType);
        }
    }

    LOGI("User app daemon finished!");
}

int main(int argc, char **argv)
{
    try {
        UserAppDmon::init();
        UserAppDmon::run();
        UserAppDmon::deinit();
    }
    catch (int exception) {
        LOGE("Fatal error %d", exception);
        LOGI("User app daemon terminated abnormally");
        exit(exception);
    }
    catch (...) {
        LOGI("Unhandled exception");
        exit(-1);
    }
    return 0;
}
