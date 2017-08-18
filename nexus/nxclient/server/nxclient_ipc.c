/******************************************************************************
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
 ******************************************************************************/
#include "nxclient.h"
#include "bipc_client.h"
#include "nxserver_ipc.h"
#include "ipc_stubs_client.h"
#include "nxserverlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <signal.h>

BDBG_MODULE(nxclient_ipc);

static const bipc_interface_descriptor * const client_interfaces [] = {
    &bipc_nxclient_p_descriptor
};

static pthread_mutex_t g_mutex[nxclient_ipc_thread_max] = {PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER};
#define LOCK() pthread_mutex_lock(&g_mutex[nxclient_ipc_thread_restricted])
#define UNLOCK() pthread_mutex_unlock(&g_mutex[nxclient_ipc_thread_restricted])
#define LOCK_STANDBY() pthread_mutex_lock(&g_mutex[nxclient_ipc_thread_regular])
#define UNLOCK_STANDBY() pthread_mutex_unlock(&g_mutex[nxclient_ipc_thread_regular])

static unsigned NxClient_P_RegisterAcknowledgeStandby(void);
static void NxClient_P_Uninit(unsigned id);

static struct nxclient_state {
    unsigned refcnt;
    struct {
        int fd;
        bipc_t ipc;
        nxclient_ipc_t nxclient_ipc;
        nxclient_p_client_info client_info;
    } client[nxclient_ipc_thread_max];
    struct {
        unsigned id;
        bool used;
    } implicitAckStandby;
} nxclient_state;

static NEXUS_Error NxClient_P_Join(nxclient_ipc_thread id, const NxClient_JoinSettings *pSettings)
{
    unsigned timeout = pSettings->timeout;

    while (1) {
        /* IPC */
        nxclient_state.client[id].fd = b_nxclient_client_connect();
        if (nxclient_state.client[id].fd > 0 || !timeout) break;

        printf("*** unable to connect to ipc server. try again...\n");
        BKNI_Sleep(1000);
        timeout--;
    }
    if (nxclient_state.client[id].fd < 0) {
        nxclient_state.client[id].fd = -1;
        fprintf(stderr, "### unable to connect to ipc server\n");
        goto err_connect;
    }

    nxclient_state.client[id].ipc = bipc_client_create(nxclient_state.client[id].fd, nxclient_state.client[id].fd, client_interfaces, sizeof(client_interfaces)/sizeof(*client_interfaces));
    if(!nxclient_state.client[id].ipc) {
        fprintf(stderr, "### unable to create bipc_client\n");
        goto err_createipc;
    }

    nxclient_state.client[id].nxclient_ipc = nxclient_p_create(nxclient_state.client[id].ipc, pSettings, &nxclient_state.client[id].client_info, id);
    if(!nxclient_state.client[id].nxclient_ipc) {
        fprintf(stderr, "### unable to create nxclient_ipc\n");
        goto err_create;
    }

    return 0;

err_create:
    bipc_client_destroy(nxclient_state.client[id].ipc);
    nxclient_state.client[id].ipc = NULL;
err_createipc:
    close(nxclient_state.client[id].fd);
    nxclient_state.client[id].fd = -1;
err_connect:
    return -1;
}

NEXUS_Error NxClient_Join(const NxClient_JoinSettings *pSettings)
{
    NEXUS_ClientAuthenticationSettings authSettings;
    NxClient_JoinSettings defaultJoinSettings;
    int rc;
    unsigned timeout;

    LOCK();
    if (nxclient_state.refcnt > 0) {
        nxclient_state.refcnt++;
        goto done;
    }

    /* local magnum */
    rc = BKNI_Init();
    if (rc) {
        fprintf(stderr, "### BKNI_Init failed: %d\n", rc);
        goto err_kni_init;
    }

    if (!pSettings) {
        NxClient_GetDefaultJoinSettings(&defaultJoinSettings);
        pSettings = &defaultJoinSettings;
    }

    timeout = pSettings->timeout;

    signal(SIGPIPE, SIG_IGN);

    rc = NxClient_P_Join(nxclient_ipc_thread_regular, pSettings);
    if(rc) {
        printf("*** Unable to join nxclient_ipc_thread_regular\n");
        goto err_regular;
    }
    rc = NxClient_P_Join(nxclient_ipc_thread_restricted, pSettings);
    if(rc) {
        printf("*** Unable to join nxclient_ipc_thread_restricted\n");
        goto err_restricted;

    }

    /* nexus driver */
    NEXUS_Platform_GetDefaultClientAuthenticationSettings(&authSettings);
    authSettings.certificate = nxclient_state.client[nxclient_ipc_thread_regular].client_info.certificate;

    while (1) {
        rc = NEXUS_Platform_AuthenticatedJoin(&authSettings);
        if (!rc || !timeout) break;

        printf("*** cannot join. try again...\n");
        BKNI_Sleep(1000);
        timeout--;
    };
    if (rc) {
        printf("### cannot join: %d\n", rc);
        goto err_join;
    }

    nxclient_state.refcnt++;
    if (!pSettings->ignoreStandbyRequest) {
        nxclient_state.implicitAckStandby.id = NxClient_P_RegisterAcknowledgeStandby();
        nxclient_state.implicitAckStandby.used = false;
    }

done:
    UNLOCK();
    return 0;

err_join:
    NxClient_P_Uninit(nxclient_ipc_thread_restricted);
err_restricted:
    NxClient_P_Uninit(nxclient_ipc_thread_regular);
err_regular:
    BKNI_Uninit();
err_kni_init:
    UNLOCK();
    return -1;
}

static void NxClient_P_Uninit(nxclient_ipc_thread id)
{
    if (nxclient_state.client[id].nxclient_ipc) {
        if (id == nxclient_ipc_thread_regular) {
            nxclient_p_destroy(nxclient_state.client[id].nxclient_ipc);
        }
        else {
            /* nxclient_p_destroy does IPC + free. Because _restricted could be blocked in standby,
            and _regular will tear down both sockets, we just free here. */
            BKNI_Free(nxclient_state.client[id].nxclient_ipc);
        }
        nxclient_state.client[id].nxclient_ipc = NULL;
    }
    if (nxclient_state.client[id].ipc) {
        bipc_client_destroy(nxclient_state.client[id].ipc);
        nxclient_state.client[id].ipc = NULL;
    }
    if (nxclient_state.client[id].fd >= 0) {
        close(nxclient_state.client[id].fd);
        nxclient_state.client[id].fd = -1;
    }
}

void NxClient_Uninit(void)
{
    NxClient_StopCallbackThread();
    LOCK();
    if (--nxclient_state.refcnt == 0) {
        NEXUS_Platform_Uninit();
        NxClient_P_Uninit(nxclient_ipc_thread_regular); /* must close regular first, since restricted calls get blocked if in standby */
        NxClient_P_Uninit(nxclient_ipc_thread_restricted);
        BKNI_Uninit();
    }
    UNLOCK();
    return;
}

#define NXCLIENT_STATE_RESTRICTED (nxclient_state.client[nxclient_ipc_thread_restricted].nxclient_ipc)
#define NXCLIENT_STATE_REGULAR    (nxclient_state.client[nxclient_ipc_thread_regular].nxclient_ipc)

#include "nxclient_api.inc"
