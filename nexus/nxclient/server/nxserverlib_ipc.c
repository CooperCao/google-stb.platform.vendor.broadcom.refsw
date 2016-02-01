/******************************************************************************
 *    (c)2011-2014 Broadcom Corporation
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
#include "nexus_types.h"
#include "nexus_platform.h"
#include "nxclient.h"
#include "bipc_server.h"
#include "nxserver_ipc.h"
#include "nxserverlib.h"
#include "blst_list.h"
#include "namevalue.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

BDBG_MODULE(nxserverlib_ipc);
#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

#include "ipc_stubs_server.h"

#define B_MAX_CLIENTS   64

struct nxclient_ipc;

struct ipc_server {
    bool exit;
    bipc_t ipc;
    BLST_D_HEAD(clientlist, nxclient_ipc) clients;
    nxserver_t server;
    BKNI_MutexHandle lock;
};

/* pointer to struct nxclient_ipc is passed to client app as handle */
BDBG_OBJECT_ID(nxclient_ipc);
struct nxclient_ipc {
    BDBG_OBJECT(nxclient_ipc)
    BLST_D_ENTRY(nxclient_ipc) link;
    int fd;
    bipc_server_client_t ipc;
    nxclient_t client;
    struct ipc_server *server;
    bool standbyClient;
};

static struct ipc_server g_ipc;

/* called from ipc thread */
static struct nxclient_ipc *nxserver_client_create(struct ipc_server *server)
{
    struct nxclient_ipc *client;

    client = BKNI_Malloc(sizeof(*client));
    if(client==NULL) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);goto err_alloc;}
    BKNI_Memset(client, 0, sizeof(*client));
    BDBG_OBJECT_SET(client, nxclient_ipc);
    BLST_D_INSERT_HEAD(&server->clients, client, link);
    client->server = server;
    client->standbyClient = true; /* Initialize this to true, so that Join can completed. */
    return client;

err_alloc:
    return NULL;
}

/* called from ipc thread */
static void nxserver_client_destroy(struct ipc_server *server, struct nxclient_ipc *client)
{
    BDBG_OBJECT_ASSERT(client, nxclient_ipc);
    bipc_server_client_destroy(server->ipc, client->ipc);
    if (client->fd != -1) {
        close(client->fd);
    }
    if (client->client) {
        NxClient_P_DestroyClient(client->client);
        client->client = NULL;
    }
    BLST_D_REMOVE(&server->clients, client, link);
    BDBG_OBJECT_DESTROY(client, nxclient_ipc);
    BKNI_Free(client);
    return;
}

static void ipc_thread(void *context)
{
    struct ipc_server *server = context;
    int listen_fd;
    int rc;
    struct nxclient_ipc *client;

    BKNI_AcquireMutex(server->lock);

    listen_fd = b_nxclient_socket_listen();
    if (listen_fd < 0) {
        BERR_TRACE(-1);
        goto done;
    }

    rc = listen(listen_fd, 10);
    if(rc!=0) { perror("");rc=BERR_TRACE(errno); goto done; }
    while(!server->exit) {
        struct pollfd fds[B_MAX_CLIENTS];
        struct nxclient_ipc *clients[B_MAX_CLIENTS];
        unsigned i,nfds,events;
        const unsigned timeout = 1000;
        bool standby = nxserver_is_standby(server->server);

        i=0;
        for(client=BLST_D_FIRST(&server->clients);client;) {
            struct nxclient_ipc *next = BLST_D_NEXT(client, link);
            if (client->fd == -1) {
                BDBG_MSG_TRACE(("destroy client %p", client));
                nxserver_client_destroy(server, client);
            }
            else {
                if (i==B_MAX_CLIENTS) break;
                if (!standby || client->standbyClient) {
                    BDBG_MSG_TRACE(("add client %p: %d %d", client, client->fd, client->standbyClient));
                    clients[i] = client;
                    fds[i].revents = 0;
                    fds[i].events = POLLIN;
                    fds[i].fd = client->fd;
                    i++;
                }
            }
            client = next;
        }
        if (i<B_MAX_CLIENTS) {
            /* if reached B_MAX_CLIENTS, stop listening for new clients */
            clients[i] = NULL;
            fds[i].revents = 0;
            fds[i].events = POLLIN;
            fds[i].fd = listen_fd;
            i++;
        }
        nfds = i;
        BKNI_ReleaseMutex(server->lock);
        BDBG_MSG_TRACE(("poll %u", nfds));
        do {
            rc = poll(fds, nfds, timeout);
        } while (rc < 0 && (errno == EAGAIN || errno == EINTR));
        BDBG_MSG_TRACE(("poll %u->%d", nfds, rc));
        BKNI_AcquireMutex(server->lock);
        if(rc<0) { perror("");rc=BERR_TRACE(errno); goto done; }
        events = (unsigned)rc;
        for(i=0;i<nfds && events ;i++) {
            if(fds[i].revents & POLLIN) {
                events --;
                client = clients[i];
                if(client) {
                    if(nxserver_is_standby(server->server) && !client->standbyClient) continue;
                    rc = bipc_server_client_process(server->ipc, client->ipc);
                    if(rc!=0) {
                        BDBG_MSG(("closing client:%#lx(%#lx) with fd:%d ", (unsigned long)client, client->client, client->fd));
                        nxserver_client_destroy(server, client);
                    }
                } else {
                    bipc_server_client_create_settings settings;
                    int fd;

                    BDBG_ASSERT(listen_fd==fds[i].fd);
                    fd = accept(listen_fd, NULL, NULL);
                    if(fd<0) {
                        BDBG_WRN(("unable to accept incoming client"));
                        continue;
                    }

                    BDBG_MSG(("new client with fd:%d connected", fd));
                    client = nxserver_client_create(server);
                    if(!client) {(void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);continue;}
                    bipc_server_get_default_client_create_settings(&settings);
                    client->fd = fd;

                    settings.recv_fd = fd;
                    settings.send_fd = fd;
                    settings.ipc_context = client;
                    /* fcntl(rc, F_SETFL, O_NONBLOCK); */
                    client->ipc = bipc_server_client_create(server->ipc, &settings);
                    if(!client->ipc) {
                        BDBG_WRN(("unable to connect incoming client"));
                        nxserver_client_destroy(server, client);
                    }
                }
            }
        }
    }
done:
    while(NULL!=(client=BLST_D_FIRST(&server->clients))) {
        nxserver_client_destroy(server, client);
    }
    if (listen_fd >= 0) {
        close(listen_fd);
    }
    BKNI_ReleaseMutex(server->lock);
    return;
}

static const bipc_server_descriptor * const ipc_interfaces [] = {
    &bipc_nxclient_p_server_descriptor
};

static NEXUS_ThreadHandle ipc_thread_id;

int nxserver_ipc_init(nxserver_t nxserver, BKNI_MutexHandle lock)
{
    struct ipc_server *server = &g_ipc;
    bipc_server_create_settings ipc_settings;
    
    bipc_server_get_default_create_settings(&ipc_settings);
    ipc_settings.interfaces = ipc_interfaces;
    ipc_settings.interface_count = sizeof(ipc_interfaces)/sizeof(*ipc_interfaces);

    BKNI_Memset(server, 0, sizeof(*server));
    server->server = nxserver;
    server->lock = lock;
    BLST_D_INIT(&server->clients);
    server->exit = false;
    server->ipc = bipc_server_create(&ipc_settings);
    BDBG_ASSERT(server->ipc);

    signal(SIGPIPE, SIG_IGN);
    ipc_thread_id = NEXUS_Thread_Create("bipc", ipc_thread, server, NULL);
    BDBG_ASSERT(ipc_thread_id);
    
    return 0;
}

void nxserver_ipc_uninit(void)
{
    struct ipc_server *server = &g_ipc;
    server->exit = true;
    NEXUS_Thread_Destroy(ipc_thread_id);
    bipc_server_destroy(server->ipc);
}

void nxserver_ipc_close_client(nxclient_t nxclient)
{
    struct nxclient_ipc *client;
    struct ipc_server *server = &g_ipc;
    for(client=BLST_D_FIRST(&server->clients);client;client=BLST_D_NEXT(client,link))  {
        if (client->client == nxclient) {
            int fd = client->fd;
            client->fd = -1;
            close(fd);
            break;
        }
    }
}

/* convert IPC to nxserverlib calls */

nxclient_ipc_t nxclient_p_create(bipc_t ipc, const NxClient_JoinSettings *pJoinSettings, nxclient_p_client_info *info)
{
    /* ipc corresponds to settings.ipc_context */
    struct nxclient_ipc *client = (struct nxclient_ipc *)ipc;
    bipc_server_client_create_settings create_settings;
    int rc;
    struct ucred credentials;
    socklen_t ucred_length = sizeof(struct ucred);

    BDBG_OBJECT_ASSERT(client, nxclient_ipc);

    bipc_server_client_get_create_settings(client->ipc, &create_settings);
    rc = getsockopt(create_settings.recv_fd, SOL_SOCKET, SO_PEERCRED, &credentials, &ucred_length);
    if (rc) {BERR_TRACE(rc); return NULL;}

    client->client = NxClient_P_CreateClient(client->server->server, pJoinSettings, &info->certificate, credentials.pid);
    if (!client->client) {
        return NULL;
    }
    client->standbyClient = pJoinSettings->standbyClient;

    return client;
}

void nxclient_p_destroy(nxclient_ipc_t _client)
{
    struct nxclient_ipc *client = _client;
    if (!client) {
        /* will be NULL if nxclient_p_create fails */
        return;
    }
    BDBG_OBJECT_ASSERT(client, nxclient_ipc);
    if (client->client) {
        NxClient_P_DestroyClient(client->client);
        client->client = NULL;
    }
    return;
}

void nxclient_p_get_display_settings(nxclient_ipc_t _client, NxClient_DisplaySettings *pSettings)
{
    NxClient_P_GetDisplaySettings(_client->client, NULL, pSettings);
}

int  nxclient_p_set_display_settings(nxclient_ipc_t _client, const NxClient_DisplaySettings *pSettings)
{
    return NxClient_P_SetDisplaySettings(_client->client, NULL, pSettings);
}

int  nxclient_p_get_display_status(nxclient_ipc_t _client, NxClient_DisplayStatus *pStatus)
{
    return NxClient_P_GetDisplayStatus(nxserver_get_client_session(_client->client), pStatus);
}

void nxclient_p_get_audio_settings(nxclient_ipc_t _client, NxClient_AudioSettings *pSettings)
{
    NxClient_P_GetAudioSettings(_client->client, pSettings);
}

int  nxclient_p_set_audio_settings(nxclient_ipc_t _client, const NxClient_AudioSettings *pSettings)
{
    return NxClient_P_SetAudioSettings(_client->client, pSettings);
}

int nxclient_p_alloc(nxclient_ipc_t _client, const NxClient_AllocSettings *pSettings, NxClient_AllocResults *pResults)
{
    return NxClient_P_Alloc(_client->client, pSettings, pResults);
}

void nxclient_p_free(nxclient_ipc_t _client, const NxClient_AllocResults *pResults)
{
    NxClient_P_Free(_client->client, pResults);
}

int nxclient_p_connect(nxclient_ipc_t _client, const NxClient_ConnectSettings *pSettings, unsigned *pConnectId)
{
    return NxClient_P_Connect(_client->client, pSettings, pConnectId);
}

int nxclient_p_refresh_connect(nxclient_ipc_t _client, unsigned connectId)
{
    return NxClient_P_RefreshConnect(_client->client, connectId);
}

void nxclient_p_disconnect(nxclient_ipc_t _client, unsigned connectId)
{
    NxClient_P_Disconnect(_client->client, connectId);
}

int nxclient_p_general(nxclient_ipc_t _client, enum nxclient_p_general_param_type type, const nxclient_p_general_param *param, nxclient_p_general_output *output)
{
    return NxClient_P_General(_client->client, type, param, output);
}

int nxclient_p_get_standby_status(nxclient_ipc_t _client, NxClient_StandbyStatus *pStatus)
{
    BSTD_UNUSED(_client);
    return NxClient_P_GetStandbyStatus(g_ipc.server, pStatus);
}

int nxclient_p_set_standby_settings(nxclient_ipc_t _client, const NxClient_StandbySettings *pSettings)
{
    return NxClient_P_SetStandbySettings(_client->client, pSettings);
}

int nxclient_p_config_get_join_settings(nxclient_ipc_t _client, NEXUS_ClientHandle client, NxClient_JoinSettings *pSettings )
{
    return NxClient_P_Config_GetJoinSettings(_client->client, client, pSettings );
}

void nxclient_p_config_get_surface_client_composition(nxclient_ipc_t _client, NEXUS_ClientHandle client, NEXUS_SurfaceClientHandle surfaceClient, NEXUS_SurfaceComposition *pComposition )
{
    NxClient_P_Config_GetSurfaceClientComposition(_client->client, client, surfaceClient, pComposition );
}

int nxclient_p_config_set_surface_client_composition(nxclient_ipc_t _client, NEXUS_ClientHandle client, NEXUS_SurfaceClientHandle surfaceClient, const NEXUS_SurfaceComposition *pComposition )
{
    return NxClient_P_Config_SetSurfaceClientComposition(_client->client, client, surfaceClient, pComposition );
}

int nxclient_p_config_get_connect_list(nxclient_ipc_t _client, NEXUS_ClientHandle client, NxClient_ConnectList *pList )
{
    return NxClient_P_Config_GetConnectList(_client->client, client, pList);
}

int nxclient_p_config_refresh_connect(nxclient_ipc_t _client, NEXUS_ClientHandle client, unsigned connectId )
{
    return NxClient_P_Config_RefreshConnect(_client->client, client, connectId);
}

void nxclient_p_config_get_connect_settings(nxclient_ipc_t _client, NEXUS_ClientHandle client, unsigned connectId, NxClient_ConnectSettings *pSettings )
{
    NxClient_P_Config_GetConnectSettings(_client->client, client, connectId, pSettings);
}

void nxclient_p_config_get_input_client_server_filter(nxclient_ipc_t _client, NEXUS_ClientHandle client, NEXUS_InputClientHandle inputClient, unsigned *pFilter )
{
    NxClient_P_Config_GetInputClientServerFilter(_client->client, client, inputClient, pFilter);
}

int nxclient_p_config_set_input_client_server_filter(nxclient_ipc_t _client, NEXUS_ClientHandle client, NEXUS_InputClientHandle inputClient, unsigned filter )
{
    return NxClient_P_Config_SetInputClientServerFilter(_client->client, client, inputClient, filter);
}

void nxclient_p_get_picture_quality_settings(nxclient_ipc_t _client, NxClient_PictureQualitySettings *pSettings)
{
    NxClient_P_GetPictureQualitySettings(_client->client, pSettings);
}

int  nxclient_p_set_picture_quality_settings(nxclient_ipc_t _client, const NxClient_PictureQualitySettings *pSettings)
{
    return NxClient_P_SetPictureQualitySettings(_client->client, pSettings);
}

int  nxclient_p_get_callback_status(nxclient_ipc_t _client, NxClient_CallbackStatus *pStatus )
{
    return NxClient_P_GetCallbackStatus(_client->client, pStatus);
}

int  nxclient_p_get_audio_status(nxclient_ipc_t _client, NxClient_AudioStatus *pStatus )
{
    return NxClient_P_GetAudioStatus(_client->client, pStatus);
}
