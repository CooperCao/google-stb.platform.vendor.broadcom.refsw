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
#include "nxclient.h"
#include "nxserver_ipc.h"
#include "nxserverlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static unsigned NxClient_P_RegisterAcknowledgeStandby(void);
struct nxclient_ipc *nxserverlib_create_local_ipcstub(nxclient_t client);
void nxserverlib_destroy_local_ipcstub(struct nxclient_ipc *client);

static struct {
    nxclient_t client;
    nxserver_t server;
    bool external_init;
    unsigned refcnt;
    BKNI_MutexHandle mutex;
    struct {
        unsigned id;
        bool used;
    } implicitAckStandby;
    struct nxclient_ipc *ipc;
} g_state;

#define LOCK() BKNI_AcquireMutex(g_state.mutex)
#define UNLOCK() BKNI_ReleaseMutex(g_state.mutex)
static pthread_mutex_t g_joinMutex = PTHREAD_MUTEX_INITIALIZER;

NEXUS_Error NxClient_Join(const NxClient_JoinSettings *pSettings)
{
    int rc = 0;
    /* init the server with default params.
    require statically-initialized mutex to avoid race on refcnt. */
    (void)pthread_mutex_lock(&g_joinMutex);
    if (!g_state.external_init && !g_state.refcnt) {
        struct nxserver_settings settings;
        g_state.server = nxserverlib_get_singleton();
        if (g_state.server) {
            /* if nxserverlib is already up, we use it and won't close it */
            g_state.external_init = true;
        }
        else {
            g_state.server = nxserver_init(0, NULL, false);
            if (!g_state.server) {
                (void)pthread_mutex_unlock(&g_joinMutex);
                fprintf(stderr, "### nxserver_init failed\n");
                return -1;
            }
        }
        nxserverlib_get_settings(g_state.server, &settings);
        g_state.mutex = settings.lock;
    }
    (void)pthread_mutex_unlock(&g_joinMutex);

    LOCK();
    if (!g_state.client) {
        g_state.client = NxClient_P_CreateClient(g_state.server, pSettings, NULL, 0);
        if (!g_state.client) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto done;
        }
        g_state.ipc = nxserverlib_create_local_ipcstub(g_state.client);
        if (!pSettings || !pSettings->ignoreStandbyRequest) {
            g_state.implicitAckStandby.id = NxClient_P_RegisterAcknowledgeStandby();
            g_state.implicitAckStandby.used = false;
        }
    }
    g_state.refcnt++;
done:
    UNLOCK();
    return rc;
}

void NxClient_Uninit(void)
{
    LOCK();
    if (--g_state.refcnt == 0) {
        NxClient_P_DestroyClient(g_state.client);
        g_state.client = NULL;
    }
    if (g_state.ipc) {
        nxserverlib_destroy_local_ipcstub(g_state.ipc);
        g_state.ipc = NULL;
    }
    UNLOCK();

    if (!g_state.external_init && !g_state.refcnt) {
        nxserver_uninit(g_state.server);
    }
    return;
}

#define NXCLIENT_STATE_RESTRICTED (g_state.ipc)
#define NXCLIENT_STATE_REGULAR    (g_state.ipc)
#define LOCK_STANDBY()
#define UNLOCK_STANDBY()
#define nxclient_state g_state

#include "nxclient_api.inc"
