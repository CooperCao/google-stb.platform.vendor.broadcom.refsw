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
#define BDBG_MSG_TRACE(X) /*BDBG_MSG(X)*/

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
    bool do_close;
    bool do_close_after_read;
    bipc_server_client_t ipc;
    nxclient_t client;
    struct ipc_server *server;
    nxclient_ipc_thread id;
    pid_t pid;
    struct nxclient_ipc *other_client;
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
    return client;

err_alloc:
    return NULL;
}

/* called from ipc thread */
static void nxserver_client_destroy(struct ipc_server *server, struct nxclient_ipc *client)
{
    BDBG_OBJECT_ASSERT(client, nxclient_ipc);
    bipc_server_client_destroy(server->ipc, client->ipc);
    close(client->fd);
    if (client->other_client) {
        /* uncross link */
        client->other_client->other_client = NULL;
    }
    else {
        /* only destroy nxclient_t for unlinked client */
        if (client->client) {
            NxClient_P_DestroyClient(client->client);
        }
    }
    BLST_D_REMOVE(&server->clients, client, link);
    BDBG_OBJECT_DESTROY(client, nxclient_ipc);
    BKNI_Free(client);
    return;
}

struct nxclient_ipc *nxserverlib_create_local_ipcstub(nxclient_t client)
{
    struct nxclient_ipc *ipcclient = BKNI_Malloc(sizeof(*ipcclient));
    if (!ipcclient) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(ipcclient, 0, sizeof(*ipcclient));
    ipcclient->client = client;
    return ipcclient;
}

void nxserverlib_destroy_local_ipcstub(struct nxclient_ipc *client)
{
    BKNI_Free(client);
}

static struct ipc_thread_context {
    struct ipc_server *server;
    nxclient_ipc_thread id;
} g_context[nxclient_ipc_thread_max];

static void ipc_thread(void *context)
{
    struct ipc_thread_context *thread_context = context;
    struct ipc_server *server = thread_context->server;
    nxclient_ipc_thread id = thread_context->id;
    int listen_fd;
    int rc;
    struct nxclient_ipc *client;
    char *dbg_str = id==nxclient_ipc_thread_regular?"regular":"restricted";

    BKNI_AcquireMutex(server->lock);

    if (id == nxclient_ipc_thread_regular) {
        listen_fd = b_nxclient_socket_listen();
        if (listen_fd < 0) {
            BERR_TRACE(-1);
            goto done;
        }
        rc = listen(listen_fd, 10);
        if(rc!=0) { perror("");rc=BERR_TRACE(errno); goto done; }
    }

    while(!server->exit) {
        struct pollfd fds[B_MAX_CLIENTS];
        struct nxclient_ipc *clients[B_MAX_CLIENTS];
        unsigned i,nfds,events;

        i=0;
        for(client=BLST_D_FIRST(&server->clients);client;) {
            struct nxclient_ipc *next = BLST_D_NEXT(client, link);
            if(client->id == id) {
                if (client->do_close) {
                    BDBG_MSG(("destroy %s client %p", dbg_str, (void*)client));
                    nxserver_client_destroy(server, client);
                }
                else {
                    if (i==B_MAX_CLIENTS) break;
                    BDBG_MSG_TRACE(("add %s client %p: %d", dbg_str, (void*)client, client->fd));
                    clients[i] = client;
                    fds[i].revents = 0;
                    fds[i].events = POLLIN;
                    fds[i].fd = client->fd;
                    i++;
                }
            }
            client = next;
        }

        if (id == nxclient_ipc_thread_regular) {
            if (i<B_MAX_CLIENTS) {
                /* if reached B_MAX_CLIENTS, stop listening for new clients */
                clients[i] = NULL;
                fds[i].revents = 0;
                fds[i].events = POLLIN;
                fds[i].fd = listen_fd;
                i++;
            }
        }

        nfds = i;
        BKNI_ReleaseMutex(server->lock);
        BDBG_MSG_TRACE(("poll %s %u", dbg_str, nfds));
        do {
            rc = poll(fds, nfds, nfds == 0 ? 25 : 1000);
        } while (rc < 0 && (errno == EAGAIN || errno == EINTR));
        BDBG_MSG_TRACE(("poll %s %u->%d", dbg_str, nfds, rc));
        BKNI_AcquireMutex(server->lock);
        if(rc<0) { perror("");rc=BERR_TRACE(errno); goto done; }
        events = (unsigned)rc;

        for(i=0;i<nfds && events ;i++) {
            if(fds[i].revents & POLLIN) {
                events --;
                client = clients[i];
                if(client) {
                    BDBG_ASSERT(client->id == id);
                    if (id == nxclient_ipc_thread_restricted) {
                        if(nxserver_is_standby(server->server)) {
                            if(client->do_close_after_read) { /* close 'restricted' portion of unwinding clients */
                                BDBG_MSG(("destroy %s client %p", dbg_str, (void*)client));
                                nxserver_client_destroy(server, client);
                            }
                            continue; /* don't accept calls over restricted wire when in standby */
                        }
                    }
                    rc = bipc_server_client_process(server->ipc, client->ipc);
                    if(rc!=0) {
                        BDBG_MSG(("closing %s client:%p(%p) with fd:%d ", dbg_str, (void*)client, (void*)client->client, client->fd));
                        nxserver_client_destroy(server, client);
                    }
                } else {
                    bipc_server_client_create_settings settings;
                    int fd;

                    BDBG_ASSERT(id == nxclient_ipc_thread_regular);
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
    for(client=BLST_D_FIRST(&server->clients);client;) {
        struct nxclient_ipc *next = BLST_D_NEXT(client, link);
        if(client->id == id) {
            nxserver_client_destroy(server, client);
        }
        client = next;
    }
    if (id == nxclient_ipc_thread_regular) {
        if (listen_fd >= 0) {
            close(listen_fd);
        }
    }
    BKNI_ReleaseMutex(server->lock);
    return;

}

static const bipc_server_descriptor * const ipc_interfaces [] = {
    &bipc_nxclient_p_server_descriptor
};

static NEXUS_ThreadHandle ipc_thread_id[nxclient_ipc_thread_max];

int nxserver_ipc_init(nxserver_t nxserver, BKNI_MutexHandle lock)
{
    struct ipc_server *server = &g_ipc;
    bipc_server_create_settings ipc_settings;
    unsigned i;

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

    for(i=0; i<sizeof(ipc_thread_id)/sizeof(ipc_thread_id[0]); i++) {
        g_context[i].server = server;
        g_context[i].id = i;
        ipc_thread_id[i] = NEXUS_Thread_Create("bipc", ipc_thread, &g_context[i], NULL);
        BDBG_ASSERT(ipc_thread_id[i]);
    }

    return 0;
}

void nxserver_ipc_uninit(void)
{
    unsigned i;
    struct ipc_server *server = &g_ipc;
    server->exit = true;
    for(i=0; i<sizeof(ipc_thread_id)/sizeof(ipc_thread_id[0]); i++)
        NEXUS_Thread_Destroy(ipc_thread_id[i]);
    bipc_server_destroy(server->ipc);
}

void nxserver_ipc_close_client(nxclient_t nxclient)
{
    struct nxclient_ipc *client;
    struct ipc_server *server = &g_ipc;
    for(client=BLST_D_FIRST(&server->clients);client;client=BLST_D_NEXT(client,link))  {
        if (client->client == nxclient) {
            client->do_close = true;
            if (client->other_client) {
                client->other_client->do_close = true;
            }
            break;
        }
    }
}

/* convert IPC to nxserverlib calls */

nxclient_ipc_t nxclient_p_create(bipc_t ipc, const NxClient_JoinSettings *pJoinSettings, nxclient_p_client_info *info, nxclient_ipc_thread id)
{
    /* ipc corresponds to settings.ipc_context */
    struct nxclient_ipc *pClient, *client = (struct nxclient_ipc *)ipc;
    struct ipc_server *server = &g_ipc;
    bipc_server_client_create_settings create_settings;
    int rc;
    struct ucred credentials;
    socklen_t ucred_length = sizeof(struct ucred);

    BDBG_OBJECT_ASSERT(client, nxclient_ipc);

    bipc_server_client_get_create_settings(client->ipc, &create_settings);
    rc = getsockopt(create_settings.recv_fd, SOL_SOCKET, SO_PEERCRED, &credentials, &ucred_length);
    if (rc) {BERR_TRACE(rc); return NULL;}

    for(pClient=BLST_D_FIRST(&server->clients);pClient;pClient=BLST_D_NEXT(pClient,link))  {
        if(pClient->pid == credentials.pid)
            break;
    }

    if(pClient && pClient->client) {
        BDBG_ASSERT(id == nxclient_ipc_thread_restricted);
        BDBG_ASSERT(pClient->id == nxclient_ipc_thread_regular);
        client->client = pClient->client;
        /* cross link */
        client->other_client = pClient;
        pClient->other_client = client;
    } else {
        if (id != nxclient_ipc_thread_regular) {
            /* the first nxclient_p_create was undone, so this second one must fail */
            return NULL;
        }
        client->client = NxClient_P_CreateClient(client->server->server, pJoinSettings, &info->certificate, credentials.pid);
    }
    if (!client->client) {
        return NULL;
    }
    client->id = id;
    client->pid = credentials.pid;

    return client;
}

void nxclient_p_destroy(nxclient_ipc_t _client)
{
    struct nxclient_ipc *client = _client;

    if (!client) {
        /* will be NULL if nxclient_p_create fails */
        return;
    }
    client->do_close = true;
    if(client->other_client) {
        client->other_client->do_close_after_read = true;
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

void nxclient_p_get_composition(nxclient_ipc_t client, unsigned surfaceClientId, NEXUS_SurfaceComposition *composition)
{
    NxClient_P_GetComposition(client->client, surfaceClientId, composition);
    return;
}

int nxclient_p_set_composition(nxclient_ipc_t client, unsigned surfaceClientId, const NEXUS_SurfaceComposition *composition)
{
    return NxClient_P_SetComposition(client->client, surfaceClientId, composition);
}

int nxclient_p_write_teletext(nxclient_ipc_t client, const nxclient_p_teletext_data *data, size_t numLines, size_t *pNumLinesWritten)
{
    return NxClient_P_WriteTeletext(client->client, data, numLines, pNumLinesWritten);
}

int nxclient_p_write_closedcaption(nxclient_ipc_t client, const nxclient_p_closecaption_data *data, size_t numEntries, size_t *pNumEntriesWritten )
{
    return NxClient_P_WriteClosedCaption(client->client, data, numEntries, pNumEntriesWritten);
}

void nxclient_p_get_audio_processing_settings(nxclient_ipc_t client, NxClient_AudioProcessingSettings *pSettings)
{
    NxClient_P_GetAudioProcessingSettings(client->client, pSettings);
    return;
}

int nxclient_p_set_audio_processing_settings(nxclient_ipc_t client, const NxClient_AudioProcessingSettings *pSettings)
{
    return NxClient_P_SetAudioProcessingSettings(client->client, pSettings);
}

int nxclient_p_display_set_wss(nxclient_ipc_t client, uint16_t wssData)
{
    return NxClient_P_Display_SetWss(client->client, wssData);
}

int nxclient_p_display_set_cgms(nxclient_ipc_t client, uint32_t cgmsData)
{
    return NxClient_P_Display_SetCgmsAorB(client->client, cgmsData, NULL);
}

int  nxclient_p_display_set_cgms_b(nxclient_ipc_t client, const nxclient_p_set_cgms_b_data *pdata)
{
    return NxClient_P_Display_SetCgmsAorB(client->client, 0, pdata);
}

int nxclient_p_reconfig(nxclient_ipc_t client, const NxClient_ReconfigSettings *pSettings)
{
    return NxClient_P_Reconfig(client->client, pSettings);
}

int nxclient_p_screenshot(nxclient_ipc_t client, const NxClient_ScreenshotSettings *pSettings, NEXUS_SurfaceHandle surface)
{
    return NxClient_P_Screenshot(client->client, pSettings, surface);
}

int nxclient_p_display_set_macrovision(nxclient_ipc_t client, NEXUS_DisplayMacrovisionType type, bool pTable_isNull, const NEXUS_DisplayMacrovisionTables *pTable)
{
    return NxClient_P_Display_SetMacrovision(client->client, type, pTable_isNull, pTable);
}

int nxclient_p_grow_heap(nxclient_ipc_t client, unsigned heapIndex )
{
    return NxClient_P_GrowHeap(client->client, heapIndex);
}

void nxclient_p_shrink_heap(nxclient_ipc_t client, unsigned heapIndex )
{
    NxClient_P_ShrinkHeap(client->client, heapIndex);
    return;
}

int nxclient_p_config_lookup_client(nxclient_ipc_t client, unsigned pid, NEXUS_ClientHandle *pHandle)
{
    return NxClient_P_Config_LookupClient(client->client,  pid, pHandle);
}

int nxclient_p_display_get_crc_data(nxclient_ipc_t client, unsigned displayIndex, NxClient_DisplayCrcData *pData)
{
    return NxClient_P_Display_GetCrcData(client->client, displayIndex, pData);
}

int nxclient_p_hdmi_output_get_crc_data(nxclient_ipc_t client, NxClient_HdmiOutputCrcData *pData )
{
    return NxClient_P_HdmiOutput_GetCrcData(client->client, pData);
}

int nxclient_p_register_acknowledge_standby(nxclient_ipc_t client, unsigned *id)
{
    return NxClient_P_RegisterAcknowledgeStandby_ipc(client->client, id);
}

void nxclient_p_unregister_acknowledge_standby(nxclient_ipc_t client, unsigned id )
{
    NxClient_P_UnregisterAcknowledgeStandby(client->client, id);
    return;
}

void nxclient_p_acknowledge_standby(nxclient_ipc_t client, unsigned id )
{
    NxClient_P_AcknowledgeStandby(client->client, id);
    return;
}

int nxclient_p_load_hdcp_keys(nxclient_ipc_t client, NxClient_HdcpType hdcpType, NEXUS_MemoryBlockHandle block, unsigned blockOffset,unsigned size)
{
    return NxClient_P_LoadHdcpKeys(client->client, hdcpType, block, blockOffset, size);
}

int nxclient_p_set_hdmi_input_repeater(nxclient_ipc_t client, NEXUS_HdmiInputHandle hdmiInput)
{
    return NxClient_P_SetHdmiInputRepeater(client->client, hdmiInput);
}

int nxclient_p_set_slave_display_graphics(nxclient_ipc_t client, unsigned slaveDisplay, NEXUS_SurfaceHandle surface)
{
    return NxClient_P_SetSlaveDisplayGraphics(client->client, slaveDisplay, surface);
}

int nxclient_p_get_status(nxclient_ipc_t client, NxClient_Status *pStatus)
{
    return NxClient_P_GetStatus(client->client, pStatus);
}

int nxclient_p_get_standby_status(nxclient_ipc_t _client, NxClient_StandbyStatus *pStatus)
{
    return NxClient_P_GetStandbyStatus(_client->client, pStatus);
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

int  nxclient_p_set_client_mode(nxclient_ipc_t _client, const NxClient_ClientModeSettings *pSettings )
{
    return NxClient_P_SetClientMode(_client->client, pSettings);
}
