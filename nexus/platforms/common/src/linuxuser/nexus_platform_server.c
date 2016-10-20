/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_platform_priv.h"
#include "nexus_platform_audio_log.h"
#include "nexus_platform_sage_log.h"

#if NEXUS_SERVER_SUPPORT
#include "nexus_driver.h"
#include "nexus_base.h"
#include "server/nexus_ipc_api.h"
#include "server/nexus_server_prologue.h"
#include "bcm_driver.h"
#include "nexus_client_resources.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h> /* see SIGPIPE below */
#include <string.h>
#endif
#include "server/nexus_server_prologue.h"
#include "../common/ipc/nexus_ipc_server_api.h"
#include "blst_queue.h"


BDBG_MODULE(nexus_platform_server);

#if NEXUS_SERVER_SUPPORT

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

/**
Must extern all module-specific functions. We cannot build a master header file of all modules.
**/
#define NEXUS_PLATFORM_P_DRIVER_MODULE(X) \
    extern int nexus_server_##X##_open(struct nexus_driver_module_header **pHeader); \
    extern int nexus_server_##X##_close(void); \
    extern int nexus_server_##X##_process(void *driver_state, void *in_data, unsigned in_data_size, void *out_data, unsigned out_mem_size, struct nexus_p_server_process_output *out);
#include "nexus_ipc_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE

/**
index of g_nexus_server_handlers[] is the module id.
**/
static const struct {
    const char *name;
    int (*open)(struct nexus_driver_module_header **pHeader);
    int (*close)(void);
    int (*process)(void *driver_state, void *in_data, unsigned in_data_size, void *out_data, unsigned out_mem_size, struct nexus_p_server_process_output *out);
} g_nexus_server_handlers[] = {
#define NEXUS_PLATFORM_P_DRIVER_MODULE(X) \
    {#X, nexus_server_##X##_open, nexus_server_##X##_close, nexus_server_##X##_process},
#include "nexus_ipc_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE
};
#define NEXUS_PLATFORM_P_NUM_DRIVERS (sizeof(g_nexus_server_handlers)/sizeof(g_nexus_server_handlers[0]))

static struct nexus_driver_state {
    int16_t open_count;
    bool active;
    bool master_active;
    bool allow_unauthenticated_clients;

    struct nexus_driver_client_state *server; /* nexus_driver_open sets server_id = client_id of the server by calling nexus_driver_objects_init().
                        the server_id is required for non-verifiable functions like those that return structs with handles allocated inside the kernel.
                        it is also an optimization to skip verification for the trusted server. */

    BLST_S_HEAD(nexus_driver_client_list, nexus_driver_client_state) clients; /* allowed, open */
    BLST_S_HEAD(nexus_driver_allowed_client_list, nexus_driver_client_state) allowed_clients; /* allowed, but not open */
    BLST_S_HEAD(nexus_driver_cleanup_client_list, nexus_driver_client_state) cleanup_clients; /* allowed, closed, need to be cleaned up */
} nexus_driver_state;

BDBG_OBJECT_ID(nexus_driver_client_state);

struct nexus_callback_queue_entry {
    BLST_Q_ENTRY(nexus_callback_queue_entry) link;
    unsigned callbackId;
    struct nexus_callback_data data;
};

BDBG_OBJECT_ID(b_server_callback_cxn);
struct b_server_callback_cxn {
    BDBG_OBJECT(b_server_callback_cxn)
    BLST_S_ENTRY(b_server_callback_cxn) link;
    struct b_server_callback *callback;
    struct NEXUS_Client *client;
    int fd;
    BKNI_MutexHandle lock;
    BLST_Q_HEAD(callback_queue, nexus_callback_queue_entry) callback_queued;
};

BDBG_OBJECT_ID(b_server_callback);
struct b_server_callback {
    BDBG_OBJECT(b_server_callback)
    int listen_fd;
    BLST_S_HEAD(callback_connections, b_server_callback_cxn) clients;
};

struct b_client_module_cxn;

/* each client connection to an ServerChannel */
BDBG_OBJECT_ID(b_client_module_cxn);
struct b_client_module_cxn
{
    BDBG_OBJECT(b_client_module_cxn)
    BLST_S_ENTRY(b_client_module_cxn) client_link;
    BLST_S_ENTRY(b_client_module_cxn) channel_link;
    struct NEXUS_ServerChannel *channel;
    NEXUS_ClientHandle client;
    int fd;
    int pollnum;
    struct nexus_driver_module_driver_state client_module_state; /* generic struct for nexus_driver_callbacks.c/nexus_driver_objects.c */
    bool deleted; /* deleted from outside the channel thread. */
};

#define B_MAX_CLIENTS 32 /* required for poll() */

/**
Threading model

there is one server thread and N channel threads.
the server has one mutex and each channel has a mutex.
if you must acquire both mutexes, you must acquire server, then channel - never channel, than server.
the channel thread processes the IPC functions. when making those IPC calls, it does not hold either the server or channel lock.
this is essential to preventing deadlock in this scenario:
1) server calls NEXUS_Platform_RegisterClient. this acquires: platform module lock, then server lock, then channel lock
2) channel proceses NEXUS_Platform IPC calls. this acquires: channel lock, copies any data locally for call, then releases the channel lock, then acquires platform module lock.

to make this work, deletion of client connections and clients is deferred. the server never deletes them directly while the channel thread is going.
instead, then channel thread cleans up connections when it is not processing an IPC calls (see nexus_cleanup_channel_connections_lock).
then, the server thread cleans up clients when all of the client connections have been cleaned up (see nexus_cleanup_clients).
**/

BDBG_OBJECT_ID(NEXUS_ServerChannel);
struct NEXUS_ServerChannel
{
    BDBG_OBJECT(NEXUS_ServerChannel)
    BLST_D_ENTRY(NEXUS_ServerChannel) link;
    unsigned moduleId;
    const char *moduleName;
    unsigned dataSize;
    void *in_data; /* size is dataSize */
    void *out_data; /* size is dataSize */

    struct nexus_driver_module_header *header; /* points to local storage in each module's thunk */
    struct NEXUS_Server *server;
    int listen_fd;
    struct pollfd fds[B_MAX_CLIENTS+1];
    NEXUS_ThreadHandle thread;
    BKNI_MutexHandle mutex;
    bool done;
    NEXUS_ModulePriority priority;

    /* list of client connections per module. protected by channel->mutex. */
    BLST_S_HEAD(channel_connection_list, b_client_module_cxn) clients;
};

struct NEXUS_Client
{
    NEXUS_OBJECT(NEXUS_Client);
    BLST_S_ENTRY(NEXUS_Client) link;
    NEXUS_ClientSettings settings;
    struct NEXUS_Server *server;
    unsigned pid;
    int fd;
    int pollnum;
    struct nexus_driver_client_state client_state; /* generic struct for nexus_driver_callbacks.c/nexus_driver_objects.c */
    bool dynamicallyCreated;
    bool deleted;
    unsigned numJoins;

    struct b_server_callback_cxn callback_cxn[NEXUS_ModulePriority_eMax];

    /* list of client connections for this client */
    BLST_S_HEAD(connection_list, b_client_module_cxn) connections;
};

BDBG_OBJECT_ID(NEXUS_Server);
struct NEXUS_Server
{
    BDBG_OBJECT(NEXUS_Server)
    NEXUS_PlatformStartServerSettings settings;

    struct nexus_driver_client_state server_client_state; /* needed when server needs to interact w/ object database as a client */
    int listen_fd;
    struct pollfd fds[1+NEXUS_ModulePriority_eMax+B_MAX_CLIENTS];
    NEXUS_ThreadHandle thread;
    bool done;
    struct b_server_callback callback[NEXUS_ModulePriority_eMax];
    BKNI_MutexHandle callback_free_lock; /* must have separate locks because callbacks may come from platform or non-platform modules */
    BLST_Q_HEAD(callback_free_queue, nexus_callback_queue_entry) callback_free;

    BLST_D_HEAD(channel_list, NEXUS_ServerChannel) channels;
    BLST_S_HEAD(client_list, NEXUS_Client) clients;
    unsigned num_clients;
    struct {
        NEXUS_ClientSettings clientSettings;
        NEXUS_ClientAuthenticationSettings auth;
        struct nexus_client_init_data init_data;
    } thread_state;
};

static struct NEXUS_Server *g_server = NULL;

static void nexus_server_thread(void *context);
static void nexus_server_channel_thread(void *context);
static void nexus_unregister_client_lock(NEXUS_ClientHandle client);
static void nexus_autoclose_client_objects_lock(NEXUS_ClientHandle client);
static void nexus_cleanup_clients_lock(struct NEXUS_Server *server, bool *p_pending);
static void nexus_cleanup_channel_connections_lock(struct NEXUS_ServerChannel *channel);
static void nexus_platform_p_set_mmap_access(NEXUS_ClientHandle client, bool grant);

/* allocate channel buffer memory on demand so we don't waste memory on
channels that are unused. */
static int nexus_server_alloc_channel_buffer(struct NEXUS_ServerChannel *channel)
{
    if (channel->in_data && channel->out_data) {
        return 0;
    }
    channel->in_data = BKNI_Malloc(channel->dataSize);
    if (!channel->in_data) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    channel->out_data = BKNI_Malloc(channel->dataSize);
    if (!channel->out_data) {
        BKNI_Free(channel->in_data);
        channel->in_data = NULL;
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    return 0;
}

NEXUS_Error NEXUS_Platform_P_InitServer(void)
{
    struct NEXUS_Server *server;
    unsigned i;
    NEXUS_Error rc;

    /* this is required to keep magnum from terminating when a sockets are closed on abnormal termination.
    TODO: should the app be required to call this? */
    signal(SIGPIPE, SIG_IGN);

    server = BKNI_Malloc(sizeof(*server));
    if (!server) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(server, 0, sizeof(*server));
    BDBG_OBJECT_SET(server, NEXUS_Server);
    BLST_Q_INIT(&server->callback_free);
    rc = BKNI_CreateMutex(&server->callback_free_lock);
    if(rc!=BERR_SUCCESS) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }
    g_server = server;

    BDBG_OBJECT_SET(&server->server_client_state, nexus_driver_client_state);
    if (g_NEXUS_platformSettings.mode == NEXUS_ClientMode_eVerified) {
        server->server_client_state.client.mode = NEXUS_ClientMode_eVerified;
    }
    b_objdb_set_default_client(&server->server_client_state.client);

    nexus_driver_state.server = &server->server_client_state;

    /* init callback "callbacks", one per NEXUS_ModulePriority, shared for all modules & clients */
    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        BDBG_OBJECT_SET(&server->callback[i], b_server_callback);
        server->callback[i].listen_fd = -1;
    }

    /* TODO: consolidate some modules on a single thread */
    for (i=0;i<NEXUS_PLATFORM_P_NUM_DRIVERS;i++) {
        struct NEXUS_ServerChannel *channel;

        channel = BKNI_Malloc(sizeof(*channel));
        if (!channel) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto error;
        }
        BKNI_Memset(channel, 0, sizeof(*channel));
        BDBG_OBJECT_SET(channel, NEXUS_ServerChannel);

        rc = (g_nexus_server_handlers[i].open)(&channel->header);
        if (rc) {
            /* if the open fails, the module was likely compiled in, but not initialized. this is normal.
            we should only fail if someone tries to call one of the functions for that module. */
            BKNI_Free(channel);
            continue;
        }

        BDBG_ASSERT(channel->header);
        channel->moduleId = i;
        channel->moduleName = g_nexus_server_handlers[i].name;
        channel->dataSize = 8192; /* max size for single transaction with unix domain sockets */
        rc = BKNI_CreateMutex(&channel->mutex);
        if(rc!=BERR_SUCCESS) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto error;
        }
        channel->server = server;

        /* server->channels is ordered by module initialization order, greatest to least, which makes objdb cleanup possible */
        {
            struct NEXUS_ServerChannel *c, *last = NULL;
            unsigned order = channel->header->module ? NEXUS_Module_GetOrder(channel->header->module) : 0;
            for (c = BLST_D_FIRST(&server->channels); c; c = BLST_D_NEXT(c, link)) {
                unsigned o = NEXUS_Module_GetOrder(c->header->module);
                if (order > o) {
                    BLST_D_INSERT_BEFORE(&server->channels, c, channel, link);
                    break;
                }
                last = c;
            }
            if (!c) {
                if (last) {
                    BLST_D_INSERT_AFTER(&server->channels, last, channel, link);
                }
                else {
                    BLST_D_INSERT_HEAD(&server->channels, channel, link);
                }
            }
        }
    }

    rc = NEXUS_Platform_P_InitAudioLog();
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto error;
    }
    rc = NEXUS_Platform_P_InitSageLog();
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto error;
    }

    /* app must call NEXUS_Platform_StartServer to start threads */

    return 0;

error:
    NEXUS_Platform_P_UninitServer();
    BDBG_ASSERT(rc); /* failure path */
    return rc;
}

void NEXUS_Platform_P_UninitServer(void)
{
    struct NEXUS_ServerChannel *channel;
    struct NEXUS_Server *server = g_server;
    unsigned i;
    struct nexus_callback_queue_entry *queue_entry;

    BDBG_OBJECT_ASSERT(server, NEXUS_Server);

    NEXUS_Platform_P_UninitAudioLog();
    NEXUS_Platform_P_UninitSageLog();
    if (server->thread) {
        /* implicit Stop */
        NEXUS_Platform_StopServer();
    }

    nexus_autoclose_client_objects_lock(NULL);

    /* close all channels */
    /* coverity[use_after_free] */
    while ((channel = BLST_D_FIRST(&server->channels))) {
        BDBG_OBJECT_ASSERT(channel, NEXUS_ServerChannel);
        BDBG_ASSERT(!channel->thread);
        BDBG_ASSERT(channel->server == server);
        BLST_D_REMOVE(&server->channels, channel, link);
        BKNI_DestroyMutex(channel->mutex);
        if (channel->in_data) BKNI_Free(channel->in_data);
        if (channel->out_data) BKNI_Free(channel->out_data);
        nexus_driver_callback_uninit(channel->header);

        (g_nexus_server_handlers[channel->moduleId].close)();

        BDBG_OBJECT_DESTROY(channel, NEXUS_ServerChannel);
        BKNI_Free(channel);
    }

    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        BDBG_ASSERT(server->callback[i].listen_fd == -1);
        BDBG_OBJECT_DESTROY(&server->callback[i], b_server_callback);
    }
    while ((queue_entry = BLST_Q_FIRST(&server->callback_free))) {
        BLST_Q_REMOVE_HEAD(&server->callback_free, link);
        BKNI_Free(queue_entry);
    }
    BKNI_DestroyMutex(server->callback_free_lock);

    b_objdb_set_default_client(NULL);
    BDBG_OBJECT_DESTROY(server, NEXUS_Server);
    BKNI_Free(server);

    g_server = NULL;
}

NEXUS_Error NEXUS_Platform_StartServer(const NEXUS_PlatformStartServerSettings *pSettings)
{
    struct NEXUS_ServerChannel *channel;
    struct NEXUS_Server *server = g_server;
    int rc;
    NEXUS_PlatformStartServerSettings defaultSettings;

    if (!server) {
        /* InitServer was not called */
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    if (server->thread) {
        /* server already started */
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    BDBG_OBJECT_ASSERT(server, NEXUS_Server);

    if (!pSettings) {
        NEXUS_Platform_GetDefaultStartServerSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    server->settings = *pSettings;

    BDBG_MSG(("NEXUS_Platform_StartServer %d modules", (unsigned)NEXUS_PLATFORM_P_NUM_DRIVERS));

    server->thread = NEXUS_Thread_Create("ipcserver", nexus_server_thread, server, NULL);
    if (!server->thread) {rc = BERR_TRACE(NEXUS_UNKNOWN); goto error;}

    /* TODO: consolidate some modules on a single thread */
    for (channel = BLST_D_FIRST(&server->channels); channel; channel = BLST_D_NEXT(channel, link)) {
        BDBG_OBJECT_ASSERT(channel, NEXUS_ServerChannel);
        channel->thread = NEXUS_Thread_Create("ipcchannel", nexus_server_channel_thread, channel, NULL);
        if (!channel->thread) {rc = BERR_TRACE(NEXUS_UNKNOWN); goto error;}
    }

    BDBG_WRN(("server listening"));
    return 0;

error:
    NEXUS_Platform_StopServer();
    BDBG_ASSERT(rc); /* failure path */
    return rc;
}

void NEXUS_Platform_StopServer(void)
{
    struct NEXUS_ServerChannel *channel;
    struct NEXUS_Server *server = g_server;
    NEXUS_ClientHandle client;
    bool pending;

    BDBG_OBJECT_ASSERT(server, NEXUS_Server);

    BDBG_MSG(("NEXUS_Platform_StopServer"));
    if (!server->thread) {
        BDBG_ERR(("NEXUS_Platform_StartServer not called"));
        return;
    }

    server->done = true;

    /* unregister clients. they will be cleaned up when the channel threads exit.
    we unlock the platform module because b_objdb acquires the platform lock when cleaning up that module.
    in theory, all other calls into server code should be done at this point so it's safe. rework may be in order. */
    for (client = BLST_S_FIRST(&server->clients); client; client = BLST_S_NEXT(client, link)) {
        nexus_unregister_client_lock(client);
    }

    /* close all channels first. this stops all communication in-process. */
    for (channel = BLST_D_FIRST(&server->channels); channel; channel = BLST_D_NEXT(channel, link)) {
        BDBG_OBJECT_ASSERT(channel, NEXUS_ServerChannel);
        if (channel->thread) {
            channel->done = true;
            shutdown(channel->listen_fd, SHUT_RDWR); /* cause listening thread to wake up */
            NEXUS_UnlockModule();
            NEXUS_Thread_Destroy(channel->thread);
            NEXUS_LockModule();
            channel->thread = NULL;
            BDBG_ASSERT(channel->listen_fd == -1);
        }
    }

    /* terminate main server thread. this stops all new communication and the last thread. */
    if (server->thread) {
        shutdown(server->listen_fd, SHUT_RDWR); /* cause listening thread to wake up */
        NEXUS_UnlockModule();
        NEXUS_Thread_Destroy(server->thread);
        NEXUS_LockModule();
        server->thread = NULL;
        BDBG_ASSERT(server->listen_fd == -1);
    }
    nexus_cleanup_clients_lock(server, &pending);
    BDBG_ASSERT(!pending);

    /* NEXUS_Platform_P_UninitServer does cleanup */
}

/* route all address conversions through dedicated functions */
void
nexus_driver_send_addr(void **paddr)
{
    /* inplace convert address from virtual to physical */
    void *addr = *paddr;
    if(addr) {
        *paddr = (void *)(unsigned long)NEXUS_AddrToOffset(addr);
    }
    return;
}


void
nexus_driver_recv_addr_cached(void **paddr)
{
    void *addr = *paddr;
    /* inplace convert address from physical to virtual */
    if(addr) {
        *paddr = NEXUS_OffsetToCachedAddr((unsigned long)addr);
    }
    return;
}

void NEXUS_Platform_P_UnregisterClient(NEXUS_ClientHandle);

static void nexus_cleanup_clients_lock(struct NEXUS_Server *server, bool *p_pending)
{
    NEXUS_ClientHandle client;

    BDBG_OBJECT_ASSERT(server, NEXUS_Server);
    *p_pending = false;

    /* if there are no more connections, then we can remove the client */
    for (client = BLST_S_FIRST(&server->clients); client; ) {
        NEXUS_ClientHandle next;

        BDBG_OBJECT_ASSERT(client, NEXUS_Client);
        next = BLST_S_NEXT(client, link);

        if (client->deleted && !BLST_S_FIRST(&client->connections)) {
            NEXUS_Platform_P_UnregisterClient(client);
        }
        else if (client->deleted) {
            *p_pending = true;
        }

        client = next;
    }
}

static void nexus_autoclose_client_lock(struct NEXUS_Client *client)
{
    unsigned i;
    NEXUS_PlatformStandbySettings standbySettings;

    BDBG_OBJECT_ASSERT(client, NEXUS_Client);

    NEXUS_Platform_GetStandbySettings(&standbySettings);
    if(standbySettings.mode != NEXUS_PlatformStandbyMode_eOn) {
        BDBG_MSG(("Defer client cleanup %p:%u", (void *)client, (unsigned)client->pid));
        return;
    }

    if (client->fd != -1) {
        close(client->fd);
        client->fd = -1;
    }

    /* disconnect callbacks */
    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct b_server_callback_cxn *callback_cxn = &client->callback_cxn[i];
        BDBG_OBJECT_ASSERT(callback_cxn, b_server_callback_cxn);
        if (callback_cxn->fd != -1) {
            BDBG_MSG(("disconnect client(%p) from callback(%d)", (void *)client, i));
            close(callback_cxn->fd);
            callback_cxn->fd = -1;
        }
    }
    if (client->dynamicallyCreated) {
        nexus_unregister_client_lock(client);
    }
    else {
        nexus_autoclose_client_objects_lock(client);
    }
}

static void nexus_server_recycle_callback_queue_entry(struct NEXUS_Server *server, struct nexus_callback_queue_entry *queue_entry)
{
    BKNI_AcquireMutex(server->callback_free_lock);
    BLST_Q_INSERT_HEAD(&server->callback_free, queue_entry, link);
    BKNI_ReleaseMutex(server->callback_free_lock);
}

static struct nexus_callback_queue_entry *nexus_server_get_callback_queue_entry(struct NEXUS_Server *server)
{
    struct nexus_callback_queue_entry *queue_entry;
    BKNI_AcquireMutex(server->callback_free_lock);
    queue_entry = BLST_Q_FIRST(&server->callback_free);
    if (queue_entry) {
        BLST_Q_REMOVE_HEAD(&server->callback_free, link);
    }
    else {
        queue_entry = BKNI_Malloc(sizeof(*queue_entry));
        if (!queue_entry) {
            BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
        else {
            BKNI_Memset(queue_entry, 0, sizeof(*queue_entry));
        }
    }
    BKNI_ReleaseMutex(server->callback_free_lock);
    return queue_entry;
}


static void nexus_server_write_callback_locked(struct b_server_callback_cxn *callback_cxn)
{
    struct nexus_callback_queue_entry *queue_entry;

    for(queue_entry = BLST_Q_FIRST(&callback_cxn->callback_queued);queue_entry;queue_entry=BLST_Q_FIRST(&callback_cxn->callback_queued)) {
        int rc;
        BDBG_MSG_TRACE(("client %p: write callback %p(%p,%d)", callback_cxn->client, queue_entry->data.callback.callback, queue_entry->data.callback.context, queue_entry->data.callback.param));
        rc = write(callback_cxn->fd, &queue_entry->data, sizeof(queue_entry->data));
        if(rc>0) {
            if((unsigned)rc!=sizeof(queue_entry->data)) {
                BDBG_ERR(("client %p: partial callback write (%d,%u)", (void *)callback_cxn->client, rc, (unsigned)sizeof(queue_entry->data)));
                break;
            }
            BLST_Q_REMOVE_HEAD(&callback_cxn->callback_queued, link);
            nexus_server_recycle_callback_queue_entry(callback_cxn->client->server, queue_entry);
            continue;
        }
        rc = errno;
        if(rc==EINTR) {
            break;
        } else if(rc==EAGAIN || rc==EWOULDBLOCK) {
            break;
        }
        break;
    }
    return;
}

static void nexus_server_cancel_callback(struct b_server_callback_cxn *callback_cxn, void *interface)
{
    struct nexus_callback_queue_entry *queue_entry;

    BKNI_AcquireMutex(callback_cxn->lock);
    for(queue_entry = BLST_Q_FIRST(&callback_cxn->callback_queued);queue_entry;) {
        struct nexus_callback_queue_entry *next=BLST_Q_NEXT(queue_entry, link);
        if(queue_entry->data.interface == interface) {
            BDBG_MSG_TRACE(("client %p: cancel %p", callback_cxn->client, interface));
            BLST_Q_REMOVE(&callback_cxn->callback_queued, queue_entry, link);
            nexus_server_recycle_callback_queue_entry(callback_cxn->client->server, queue_entry);
        }
        queue_entry=next;
    }
    BKNI_ReleaseMutex(callback_cxn->lock);
    return;
}

static void nexus_server_thread(void *context)
{
    struct NEXUS_Server *server = context;
    int rc;
    unsigned i;

    NEXUS_Profile_MarkThread("nexus_server_thread");

    server->listen_fd = b_nexus_socket_listen(nexus_socket_type_main, 0);
    if (server->listen_fd == -1) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        goto done;
    }

    /* set up callback listener sockets */
    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct b_server_callback *callback = &server->callback[i];

        BDBG_OBJECT_ASSERT(callback, b_server_callback);

        callback->listen_fd = b_nexus_socket_listen(nexus_socket_type_scheduler, i);
        if (callback->listen_fd == -1) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            return;
        }
    }

    BDBG_MSG(("server listening..."));
    while (!server->done) {
        struct pollfd *fds = server->fds;
        struct NEXUS_Client *client;

        /* wake up every second to see if app is exiting or client list has changed */
        while (!server->done) {
            int err_no = 0;
            bool pending;
            unsigned timeout = 1000;
            unsigned num = 0;

            /* add main server listener */
            fds[num].fd = server->listen_fd;
            fds[num].events = POLLIN;
            num++;

            /* add callback listeners */
            for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
                fds[num].fd = server->callback[i].listen_fd;
                fds[num].events = POLLIN;
                num++;
            }

            NEXUS_LockModule();
            nexus_cleanup_clients_lock(server, &pending);
            NEXUS_UnlockModule();
            if (pending) {
                timeout = 1; /* very short timeout if channel threads are cleaning up */
            }

            /* add clients to detect disconnects. note that client->pollnum cannot be zero if included in the poll. */
            NEXUS_LockModule();
            for (client = BLST_S_FIRST(&server->clients); client; client = BLST_S_NEXT(client, link)) {
                BDBG_OBJECT_ASSERT(client, NEXUS_Client);
                if (client->fd != -1) {
                    BDBG_ASSERT(num<1+NEXUS_ModulePriority_eMax+B_MAX_CLIENTS);
                    client->pollnum = num;
                    fds[num].fd = client->fd;
                    fds[num].events = POLLIN;
                    num++;
                }
            }
            NEXUS_UnlockModule();

            do {
                rc = poll(fds, num, timeout);
                err_no = errno;
            } while (rc < 0 && (err_no == EAGAIN || err_no == EINTR));
            if (rc < 0) {
                rc = BERR_TRACE(err_no);
                goto done;
            }
            else if (rc > 0) {
                break;
            }
        }
        if (server->done) break;

        /* check if a client has disconnected */
        NEXUS_LockModule();
        for (client = BLST_S_FIRST(&server->clients); client; ) {
            struct NEXUS_Client *next;
            BDBG_OBJECT_ASSERT(client, NEXUS_Client);
            /* try to send all queued callbacks */
            for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
                struct b_server_callback_cxn *callback_cxn = &client->callback_cxn[i];
                BKNI_AcquireMutex(callback_cxn->lock);
                nexus_server_write_callback_locked(callback_cxn);
                BKNI_ReleaseMutex(callback_cxn->lock);
            }
            next = BLST_S_NEXT(client, link);
            if (client->fd != -1 && client->pollnum != -1) {
                if (fds[client->pollnum].revents & (POLLERR|POLLHUP)) {
                    nexus_autoclose_client_lock(client);
                }
                else if (fds[client->pollnum].revents) {
                    enum nexus_main_socket_message_type msg;
                    /* getting a message from the main client socket. */
                    rc = b_nexus_read(client->fd, &msg, sizeof(msg));
                    if (rc == sizeof(msg)) {
                        switch (msg) {
                        case nexus_main_socket_message_type_disconnect:
                            BDBG_MSG(("clean close of client %p", (void *)client));
                            nexus_autoclose_client_lock(client);
                            break;
                        case nexus_main_socket_message_type_stop_callbacks:
                            {
                                void *interface;
                                rc = b_nexus_read(client->fd, &interface, sizeof(interface));
                                if (rc == sizeof(interface)) {
                                    static unsigned stopCallbacksCount = 0; /* global id */
                                    NEXUS_CallbackDesc callback = NEXUS_CALLBACKDESC_INITIALIZER();
                                    NEXUS_Error nrc;
                                    callback.param = ++stopCallbacksCount; /* grab the next id. no sync needed. */

                                    b_objdb_set_client(&client->client_state.client);
                                    nrc = b_objdb_verify_any_object(interface);
                                    if(nrc==NEXUS_SUCCESS) {
                                        /* stop production of callbacks for this interface */
                                        NEXUS_StopCallbacks(interface);
                                    } else {
                                        rc = BERR_TRACE(nrc);
                                    }
                                    b_objdb_set_client(NULL);

                                    /* send "stop callbacks" id through every callback connection so client can sync */
                                    BDBG_MSG_TRACE(("stop callbacks: %p, %d", client, callback.param));
                                    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
                                        struct b_server_callback_cxn *callback_cxn = &client->callback_cxn[i];

                                        nexus_server_cancel_callback(callback_cxn, interface);
                                        nexus_server_send_callback(callback_cxn, NULL, 0, &callback);
                                    }
                                    b_nexus_write(client->fd, &callback.param, sizeof(callback.param));
                                }
                            }
                            break;
                        case nexus_main_socket_message_type_start_callbacks:
                            {
                                void *interface;
                                rc = b_nexus_read(client->fd, &interface, sizeof(interface));
                                if (rc == sizeof(interface)) {
                                    b_objdb_set_client(&client->client_state.client);
                                    if(b_objdb_verify_any_object(interface)==NEXUS_SUCCESS) {
                                        /* start production of callbacks for this interface */
                                        NEXUS_StartCallbacks(interface);
                                    } else {
                                        /* XXX NEXUS_StartCallbacks could be used with bad handle, in particularly XXX_Close, is paired with auto generated Stop>Start callbacks, where Start called _after_ obhect was already destroyed */
                                    }
                                    b_objdb_set_client(NULL);
                                }
                            }
                            break;
                        default:
                            break;
                        }
                    }
                    else if (rc == -1) {
                        nexus_autoclose_client_lock(client);
                    }
                }
            }
            client = next;
        }
        NEXUS_UnlockModule();

        /* check listener for a new client or client callback connection */
        for (i=0;i<1+NEXUS_ModulePriority_eMax;i++) {
            if (fds[i].revents) {
                int err_no = 0;
                int fd;
                struct ucred credentials;
                socklen_t ucred_length = sizeof(struct ucred);

                fd = accept(i == 0 ? server->listen_fd : server->callback[i-1].listen_fd, NULL, NULL);
                err_no = errno;
                if (fd < 0) {
                    if (!server->done) {
                        rc = BERR_TRACE(err_no);
                    }
                    continue;
                }

                rc = getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &credentials, &ucred_length);
                if (rc) {
                    BERR_TRACE(rc);
                    close(fd);
                }
                else {
                    /* credentials.pid is the process ID for this client */

                    if (i == 0) {

                        /* some manual ipc that matches nexus_platform_client.c code */
                        rc = b_nexus_read(fd, &server->thread_state.auth, sizeof(server->thread_state.auth));
                        if (rc != sizeof(server->thread_state.auth)) {
                            BDBG_WRN(("unable to read authentication settings: %d", rc));
                            close(fd);
                            continue;
                        }

                        /* search already registered clients */
                        NEXUS_LockModule();
                        for (client = BLST_S_FIRST(&server->clients); client; client = BLST_S_NEXT(client, link)) {
                            if (client->fd == -1 && !client->dynamicallyCreated) {
                                if (client->settings.authentication.certificate.length == server->thread_state.auth.certificate.length &&
                                    server->thread_state.auth.certificate.length <= sizeof(server->thread_state.auth.certificate.data) &&
                                    !BKNI_Memcmp(&client->settings.authentication.certificate.data, &server->thread_state.auth.certificate.data, server->thread_state.auth.certificate.length)) {
                                    /* found it */
                                    BDBG_MSG(("authenticated client %p for pid %d", (void*)client, credentials.pid));
                                    break;
                                }
                            }
                        }
                        NEXUS_UnlockModule();

                        /* if not found, and if unauthenticated clients are allowed, create a new one */
                        if (!client && server->settings.allowUnauthenticatedClients) {
                            BDBG_MSG(("creating dynamic client for pid %d", credentials.pid));
                            NEXUS_LockModule();
                            NEXUS_Platform_GetDefaultClientSettings(&server->thread_state.clientSettings);
                            server->thread_state.clientSettings.configuration = server->settings.unauthenticatedConfiguration;
                            client = NEXUS_Platform_RegisterClient(&server->thread_state.clientSettings);
                            NEXUS_UnlockModule();
                            if (client) {
                                client->dynamicallyCreated = true;
                            }
                        }

                        /* unable to authenticate or allow this client */
                        if (!client) {
                            BDBG_WRN(("rejected unauthenticated client %d connection", credentials.pid));
                            close(fd);
                            continue;
                        }

                        BKNI_Memset(&server->thread_state.init_data, 0, sizeof(server->thread_state.init_data));
                        server->thread_state.init_data.config = client->settings.configuration;
                        for (i=0;i<NEXUS_MAX_HEAPS;i++) {
                            if (server->thread_state.init_data.config.heap[i]) {
                                NEXUS_MemoryStatus status;
                                server->thread_state.init_data.heap[i].heap = server->thread_state.init_data.config.heap[i];
                                NEXUS_Heap_GetStatus_priv(server->thread_state.init_data.heap[i].heap, &status);
                                server->thread_state.init_data.heap[i].offset  = status.offset;
                                server->thread_state.init_data.heap[i].size = status.size;
                                server->thread_state.init_data.heap[i].memoryType = status.memoryType;
                            }
                        }

                        /* this client is ready. must set state before writing socket ack. */
                        client->pid = credentials.pid;
                        client->fd = fd;
                        client->numJoins++;

                        nexus_platform_p_set_mmap_access(client, true);
                        BDBG_MSG(("client %p (pid %d) ready", (void *)client, credentials.pid));

                        rc = b_nexus_write(fd, &server->thread_state.init_data, sizeof(server->thread_state.init_data));
                        if (rc != sizeof(server->thread_state.init_data)) {
                            BDBG_WRN(("unable to confirm client: %d", rc));
                            /* if socket ack fails, we have to unwind */
                            client->pid = 0;
                            client->fd = -1;
                            client->numJoins--;
                            close(fd);
                            if (client->dynamicallyCreated) {
                                /* equivalent of NEXUS_Platform_UnregisterClient, without platform lock */
                                NEXUS_LockModule();
                                nexus_unregister_client_lock(client);
                                NEXUS_UnlockModule();
                            }
                            continue;
                        }
                    }
                    else {
                        struct NEXUS_Client *client;
                        unsigned priority = i-1;

                        /* new callback connection */
                        /* find the client */
                        NEXUS_LockModule();
                        for (client = BLST_S_FIRST(&server->clients); client; client = BLST_S_NEXT(client, link)) {
                            if (client->pid == (unsigned)credentials.pid) {
                                if (client->callback_cxn[priority].fd != -1) {
                                    BDBG_WRN(("client(%p) callback(%d) already connected", (void *)client, priority));
                                    client = NULL;
                                }
                                break;
                            }
                        }
                        NEXUS_UnlockModule();
                        if (client) {
                            int flags;
                            /* client has connected */
                            BDBG_MSG(("connect client(%p) to callback(%d)", (void *)client, priority));
                            client->callback_cxn[priority].fd = fd;

                            /* must be non-blocking. this allows us to read back # of callbacks consumed without blocking, and
                            allows us to fire callbacks without client backing up server. */
                            flags = fcntl(fd, F_GETFL, 0);
                            fcntl(fd, F_SETFL, flags | O_NONBLOCK | FD_CLOEXEC);
                        }
                        else {
                            close(fd);
                        }
                    }
                }
            }
        }
    }

done:
    if (server->listen_fd != -1) {
        close(server->listen_fd);
        server->listen_fd = -1;
    }

    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct b_server_callback *callback = &server->callback[i];
        BDBG_OBJECT_ASSERT(callback, b_server_callback);
        if (callback->listen_fd != -1) {
            close(callback->listen_fd);
            callback->listen_fd = -1;
        }
    }

    return;
}

static void nexus_cleanup_channel_connections_lock(struct NEXUS_ServerChannel *channel)
{
    struct b_client_module_cxn *cxn;

    BDBG_OBJECT_ASSERT(channel, NEXUS_ServerChannel);
    BKNI_AcquireMutex(channel->mutex);
    for (cxn = BLST_S_FIRST(&channel->clients); cxn;) {
        struct b_client_module_cxn *next;

        BDBG_OBJECT_ASSERT(cxn, b_client_module_cxn);
        next = BLST_S_NEXT(cxn, channel_link);
        if (cxn->deleted) {
            /* remove from channel and from client */
            BLST_S_REMOVE(&channel->clients, cxn, b_client_module_cxn, channel_link);
            BDBG_OBJECT_ASSERT(cxn->client, NEXUS_Client);
            BLST_S_REMOVE(&cxn->client->connections, cxn, b_client_module_cxn, client_link);

            BDBG_OBJECT_DESTROY(cxn, b_client_module_cxn);
            BKNI_Free(cxn);
        }
        cxn = next;
    }
    BKNI_ReleaseMutex(channel->mutex);
}

/* this thread never acquires the server mutex */
static void nexus_server_channel_thread(void *context)
{
    struct NEXUS_ServerChannel *channel = context;
    int rc;

    NEXUS_Profile_MarkThread("nexus_server_channel_thread");

    channel->listen_fd = b_nexus_socket_listen(nexus_socket_type_module, channel->moduleId);
    if (channel->listen_fd == -1) {
        rc = BERR_TRACE(NEXUS_UNKNOWN);
        return;
    }

    BDBG_MSG(("%s(%d) server channel listening...", channel->moduleName, channel->moduleId));
    while (!channel->done) {
        struct pollfd *fds = channel->fds;
        struct b_client_module_cxn *cxn;

        /* wake up every second to see if app is exiting */
        while (!channel->done) {
            int err_no = 0;
            unsigned num = 0;

            /* add listener */
            fds[num].fd = channel->listen_fd;
            fds[num].events = POLLIN;
            num++;

            if (NEXUS_TryLockModule()) {
                nexus_cleanup_channel_connections_lock(channel);
                NEXUS_UnlockModule();
            }

            /* add clients. note that client->pollnum cannot be zero if included in the poll. */
            BKNI_AcquireMutex(channel->mutex);
            for (cxn = BLST_S_FIRST(&channel->clients); cxn; cxn = BLST_S_NEXT(cxn, channel_link)) {
                BDBG_OBJECT_ASSERT(cxn, b_client_module_cxn);
                if (cxn->fd != -1) {
                    BDBG_ASSERT(num<B_MAX_CLIENTS+1);
                    cxn->pollnum = num;
                    fds[num].fd = cxn->fd;
                    fds[num].events = POLLIN;
                    num++;
                }
            }
            BKNI_ReleaseMutex(channel->mutex);

            do {
                rc = poll(fds, num, 1000);
                err_no = errno;
            } while (rc < 0 && (err_no == EAGAIN || err_no == EINTR));
            if (rc < 0) {
                rc = BERR_TRACE(err_no);
                goto done;
            }
            else if (rc > 0) {
                break;
            }
        }
        if (channel->done) break;

        /* check clients */
        BKNI_AcquireMutex(channel->mutex);
        for (cxn = BLST_S_FIRST(&channel->clients); cxn; cxn = BLST_S_NEXT(cxn, channel_link)) {
            BDBG_OBJECT_ASSERT(cxn, b_client_module_cxn);
            if (!cxn->deleted && cxn->fd != -1 && cxn->pollnum != -1) {
                if (fds[cxn->pollnum].revents & (POLLERR|POLLHUP)) {
                    BDBG_OBJECT_ASSERT(cxn->channel, NEXUS_ServerChannel);
                    BDBG_OBJECT_ASSERT(cxn->client, NEXUS_Client);
                    BDBG_MSG(("disconnect client(%p) from module(%d)", (void *)cxn->client, cxn->channel->moduleId));
                    close(cxn->fd);
                    cxn->fd = -1;
                }
                else if (fds[cxn->pollnum].revents) {
                    /* we're going to make an IPC call. we must unlock, but this should be safe
                    because the cxn and client are guaranteed to not be destroyed here. */

                    rc = nexus_server_alloc_channel_buffer(channel);
                    if (rc) {
                        close(cxn->fd);
                        cxn->fd = -1;
                        continue;
                    }

                    BKNI_ReleaseMutex(channel->mutex);

                    /* process client message */
                    rc = b_nexus_read(cxn->fd, channel->in_data, channel->dataSize);
                    if (rc <= 0) {
                        /* not necessarily an error. client may have closed, which is normal. */
                        BDBG_MSG(("read failed: %d %d", rc, errno));
                        close(cxn->fd);
                        cxn->fd = -1;
                    }
                    else if (rc) {
                        struct nexus_p_server_process_output out;
                        unsigned received = rc;
                        void *in_data = channel->in_data;
                        size_t packet_size = ((NEXUS_Ipc_Header *)in_data)->packet_size;

                        if(packet_size > received) {
                            if(packet_size > channel->dataSize) {
                                /* The tainted variable, "packet_size", is exempt
                                 * from the TAINTED_SCALAR check because any non-zero
                                 * value may be valid, unless there is not enough
                                 * memory to allocate
                                 */
                                /* coverity[tainted_data] */
                                void *new_in_data = BKNI_Malloc(packet_size);
                                if(new_in_data==NULL) {
                                    BDBG_MSG(("malloc failed: %u %u", (unsigned)packet_size, channel->dataSize));
                                    close(cxn->fd);
                                    cxn->fd = -1;
                                    goto client_done;
                                }
                                BKNI_Memcpy(new_in_data, in_data, received);
                                in_data = new_in_data;
                            }
                            while(received < packet_size) {
                                rc = b_nexus_read(cxn->fd, (uint8_t *)in_data + received, packet_size - received);
                                if(rc <=0) {
                                    BDBG_MSG(("cont - read failed: %u(%u) %d %d", received, (unsigned)packet_size, rc, errno));
                                    if(in_data != channel->in_data) {
                                        BKNI_Free(in_data);
                                    }
                                    close(cxn->fd);
                                    cxn->fd = -1;
                                    goto client_done;
                                }
                                received += rc;
                            }
                        }

                        /*BDBG_MSG_TRACE(("read %d", received));*/
                        /* batom_range_dump(in_data, received, "RECV"); */
                        rc = (*g_nexus_server_handlers[channel->moduleId].process)(&cxn->client_module_state, in_data, received, channel->out_data, channel->dataSize, &out);
                        if (rc) {
                            BDBG_ERR(("client(%p,%p,%d) module(%d) call failed: %d", (void *)cxn->client, (void *)cxn, cxn->fd, cxn->channel->moduleId, rc));
                            /* write a 1 byte response, which will cause a failure on the client side, but keep connection open */
                            out.size = 1;
                            out.data = channel->out_data;
                        }
                        ((NEXUS_Ipc_Header *)out.data)->packet_size = out.size;

                        /* batom_range_dump(out.data, out.size, "SEND"); */
                        rc = b_nexus_write(cxn->fd, out.data, out.size);
                        if (rc < 0) {
                            /* if we can't write a response, we need to close connection */
                            BDBG_MSG(("write failed: %d %d", rc, errno));
                            close(cxn->fd);
                            cxn->fd = -1;
                        }
                        if(out.data !=channel->out_data) {
                            BKNI_Free(out.data);
                        }
                        if(in_data != channel->in_data) {
                            BKNI_Free(in_data);
                        }
                        /*BDBG_MSG_TRACE(("wrote %d out of %d", rc, channel->dataSize));*/
                    }
client_done:
                    BKNI_AcquireMutex(channel->mutex);
                }
            }
        }
        BKNI_ReleaseMutex(channel->mutex);

        /* check listener for client module connection */
        if (fds[0].revents) {
            int err_no = 0;
            int fd;
            struct ucred credentials;
            socklen_t ucred_length = sizeof(struct ucred);

            fd = accept(channel->listen_fd, NULL, NULL);
            err_no = errno;
            if (fd < 0) {
                if (!channel->done) {
                    BDBG_ERR(("accept socket failed: errno %d, %s", err_no, strerror(err_no)));
                    rc = BERR_TRACE(err_no);
                }
                continue;
            }

            /* find the client */
            rc = getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &credentials, &ucred_length);
            if (rc) {
                BERR_TRACE(rc);
                close(fd);
            }
            else {
                BKNI_AcquireMutex(channel->mutex);
                for (cxn = BLST_S_FIRST(&channel->clients); cxn; cxn = BLST_S_NEXT(cxn, channel_link)) {
                    BDBG_OBJECT_ASSERT(cxn, b_client_module_cxn);
                    BDBG_OBJECT_ASSERT(cxn->client, NEXUS_Client);
                    if (!cxn->deleted && cxn->client->pid == (unsigned)credentials.pid) {
                        if (cxn->fd != -1) {
                            BDBG_ERR(("client(%p) module(%d) already connected", (void *)cxn->client, cxn->channel->moduleId));
                            cxn = NULL;
                        }
                        break;
                    }
                }
                if (cxn) {
                    /* client has connected */
                    BDBG_MSG(("connect client(%p) to module(%d)", (void *)cxn->client, cxn->channel->moduleId));
                    cxn->fd = fd;
                }
                else {
                    /* not necessarily an error */
                    BDBG_MSG(("no module connection for client pid %d", credentials.pid));
                    close(fd);
                }
                BKNI_ReleaseMutex(channel->mutex);
            }
        }
    }

done:
    close(channel->listen_fd);
    channel->listen_fd = -1;

    /* when the channel thread exists, all connections must be deleted */
    NEXUS_LockModule();
    nexus_cleanup_channel_connections_lock(channel);
    NEXUS_UnlockModule();
    return;
}

static const char *g_clientModeStr[NEXUS_ClientMode_eMax] = {"unprotected","verified","protected","untrusted"};

NEXUS_ClientHandle NEXUS_Platform_RegisterClient(const NEXUS_ClientSettings *pSettings)
{
    NEXUS_ClientHandle client;
    struct NEXUS_Server *server = g_server;
    struct NEXUS_ServerChannel *channel;
    unsigned i;
    int rc;

    BDBG_OBJECT_ASSERT(server, NEXUS_Server);
    BDBG_ASSERT(pSettings);

    if (!server->thread) {
        BDBG_ERR(("NEXUS_Platform_StartServer not called"));
        return NULL;
    }

    if (server->num_clients == B_MAX_CLIENTS) {
        BDBG_ERR(("max clients %d reached", B_MAX_CLIENTS));
        return NULL;
    }

    client = BKNI_Malloc(sizeof(*client));
    if (!client) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    NEXUS_OBJECT_INIT(NEXUS_Client, client);
    client->pid = 0;
    client->fd = -1;
    client->pollnum = -1;
    client->server = server;
    client->settings = *pSettings;
    BDBG_OBJECT_SET(&client->client_state, nexus_driver_client_state);
    client->client_state.client.resources.allowed = client->settings.configuration.resources;
    switch (client->settings.configuration.mode) {
    case NEXUS_ClientMode_eUnprotected: /* deprecated */
        client->client_state.client.mode = NEXUS_ClientMode_eVerified;
        break;
    case NEXUS_ClientMode_eVerified:
    case NEXUS_ClientMode_eProtected:
    case NEXUS_ClientMode_eUntrusted:
        client->client_state.client.mode = client->settings.configuration.mode;
        break;
    default:
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_invalid_mode;
    }

    rc = b_get_client_default_heaps(&client->settings.configuration, &client->client_state.client.default_heaps);
    if (rc) {
        BERR_TRACE(rc);
        goto err_no_default_heap;
    }

    BKNI_Memcpy(&client->client_state.client.config, &client->settings.configuration, sizeof(client->client_state.client.config));

    BLST_S_INSERT_HEAD(&server->clients, client, link);
    server->num_clients++;

    /* pre-create all callback connections */
    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct b_server_callback_cxn *callback_cxn = &client->callback_cxn[i];
        BERR_Code rc;

        BDBG_OBJECT_SET(callback_cxn, b_server_callback_cxn);
        callback_cxn->fd = -1;
        callback_cxn->client = client;
        callback_cxn->callback = &server->callback[i];
        BLST_Q_INIT(&callback_cxn->callback_queued);
        BLST_S_INSERT_HEAD(&callback_cxn->callback->clients, callback_cxn, link);
        BDBG_OBJECT_ASSERT(callback_cxn->callback, b_server_callback);
        rc = BKNI_CreateMutex(&callback_cxn->lock);
        if(rc!=BERR_SUCCESS) { rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto err_create_mutex; }
    }

    /* pre-create all module connections */
    for (channel = BLST_D_FIRST(&server->channels); channel; channel = BLST_D_NEXT(channel, link)) {
        struct b_client_module_cxn *cxn;
        NEXUS_ModuleSettings moduleSettings;

        BDBG_OBJECT_ASSERT(channel, NEXUS_ServerChannel);
        cxn = BKNI_Malloc(sizeof(*cxn));
        if (!cxn) {
            BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto err_malloc_cxn;
        }
        BKNI_Memset(cxn, 0, sizeof(*cxn));
        BDBG_OBJECT_SET(cxn, b_client_module_cxn);
        cxn->fd = -1; /* will listen for this */
        cxn->pollnum = -1;
        cxn->client = client;
        cxn->channel = channel;
        cxn->client_module_state.client = &client->client_state; /* this is the object database's CLIENT_ID */
        NEXUS_Module_GetSettings(channel->header->module, &moduleSettings);
        cxn->client_module_state.slave_scheduler = (void*)&client->callback_cxn[moduleSettings.priority]; /* slave_scheduler is just a
            pointer to the correct callback connection so the callback can get routed to the right client with the right priority socket */

        BLST_S_INSERT_HEAD(&client->connections, cxn, client_link);
        BKNI_AcquireMutex(channel->mutex);
        BLST_S_INSERT_HEAD(&channel->clients, cxn, channel_link);
        BKNI_ReleaseMutex(channel->mutex);
    }

    NEXUS_OBJECT_REGISTER(NEXUS_Client, client, Create);
    BDBG_MSG(("registered client: %p, cert length %d, %s, total %d", (void *)client, client->settings.authentication.certificate.length, g_clientModeStr[client->settings.configuration.mode], server->num_clients));

    return client;

err_malloc_cxn:
err_create_mutex:
    nexus_unregister_client_lock(client);
    NEXUS_UnlockModule();
err_malloc:
    return NULL;

err_no_default_heap:
err_invalid_mode:
    NEXUS_OBJECT_DESTROY(NEXUS_Client, client);
    BKNI_Free(client);
    return NULL;
}

/* NEXUS_Platform_UnregisterClient can be called for explicitly and dynamically registered clients */
void NEXUS_Platform_UnregisterClient( NEXUS_ClientHandle client )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    /* we unlock the platform module because b_objdb acquires the platform lock when cleaning up that module.
    because nothing else is done in this function, it is safe. */
    nexus_unregister_client_lock(client);
}

static void NEXUS_Client_P_Release( NEXUS_ClientHandle client )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    NEXUS_OBJECT_UNREGISTER(NEXUS_Client, client, Destroy);
}

static void NEXUS_Client_P_Finalizer( NEXUS_ClientHandle client )
{
    unsigned i;
    struct NEXUS_Server *server;

    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    server = client->server;
    BDBG_ASSERT(client->deleted);
    BDBG_ASSERT(!BLST_S_FIRST(&client->connections));

    BLST_S_REMOVE(&server->clients, client, NEXUS_Client, link);
    server->num_clients--;
    BDBG_MSG(("remove client %p %d", (void *)client, server->num_clients));
    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct b_server_callback_cxn *callback_cxn = &client->callback_cxn[i];
        struct nexus_callback_queue_entry *queue_entry;
        BDBG_OBJECT_ASSERT(callback_cxn, b_server_callback_cxn);
        if (callback_cxn->fd != -1) {
            close(callback_cxn->fd);
        }
        BLST_S_REMOVE(&callback_cxn->callback->clients, callback_cxn, b_server_callback_cxn, link);
        BKNI_DestroyMutex(callback_cxn->lock);
        while ((queue_entry = BLST_Q_FIRST(&callback_cxn->callback_queued))) {
            BLST_Q_REMOVE_HEAD(&callback_cxn->callback_queued, link);
            nexus_server_recycle_callback_queue_entry(callback_cxn->client->server, queue_entry);
        }
    }

    NEXUS_OBJECT_DESTROY(NEXUS_Client, client);
    BKNI_Free(client);
}

/* The public NEXUS_Platform_UnregisterClient marks the client as deleted,
but the actual delete (NEXUS_Platform_P_UnregisterClient) only happens by sync with threads. */
NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Client, NEXUS_Platform_P_UnregisterClient);

static void nexus_autoclose_client_objects_lock(NEXUS_ClientHandle client)
{
    /* auto-close all objects associated with this client */
    struct NEXUS_Server *server = g_server;
    struct NEXUS_ServerChannel *channel;
    struct b_objdb_client *objdb_client;

    if (client) {
        BDBG_OBJECT_ASSERT(client, NEXUS_Client);
        BDBG_MSG(("autoclose client(%p) objects", (void *)client));
         objdb_client = nexus_p_platform_objdb_client(client);
    }
    else {
        BDBG_MSG(("autoclose server objects"));
        objdb_client = &server->server_client_state.client;
    }

    /* clean up anything left open */
    for (channel = BLST_D_FIRST(&server->channels); channel; channel = BLST_D_NEXT(channel, link)) {
        struct nexus_driver_module_header *header;
        header = channel->header;
        if (!header || header->module == NEXUS_MODULE_SELF) {
            continue;
        }

        NEXUS_Module_Lock(g_NEXUS_platformHandles.core);
        NEXUS_CoreModule_Uninit_Client_priv(objdb_client);
        NEXUS_Module_Unlock(g_NEXUS_platformHandles.core);
        b_objdb_module_uninit_client_objects(&header->objdb, objdb_client);

        /* deactivate all entries for this client, even if object not in database */
        b_objdb_module_uninit_client_callbacks(&header->objdb, objdb_client);
    }

    if (client && client->pid) {
        nexus_platform_p_set_mmap_access(client, false);
    }

    NEXUS_Platform_P_SweepModules();
}

/* we cannot delete clients or connections in this function. we can only
mark them for deletion. */
static void nexus_unregister_client_lock(NEXUS_ClientHandle client)
{
    struct b_client_module_cxn *cxn;

    BDBG_OBJECT_ASSERT(client, NEXUS_Client);

    BDBG_MSG(("nexus_unregister_client_lock: %p %d", (void *)client, client->fd));
    if (client->fd != -1) {
        close(client->fd);
        client->fd = -1;
    }
    nexus_autoclose_client_objects_lock(client);
    client->deleted = true; /* nexus_cleanup_clients will delete once every connection is deleted */

    for (cxn = BLST_S_FIRST(&client->connections); cxn; cxn = BLST_S_NEXT(cxn, client_link)) {
        BDBG_OBJECT_ASSERT(cxn, b_client_module_cxn);
        cxn->deleted = true; /* nexus_cleanup_channel_connections_lock will delete connections */
        if (cxn->fd != -1) {
            close(cxn->fd);
            cxn->fd = -1;
        }
    }
}

extern int g_NEXUS_driverFd; /* from nexus_platform_os.c */

static void nexus_platform_p_set_mmap_access(NEXUS_ClientHandle client, bool grant)
{
    t_bcm_linux_mmap_access access;
    int rc;

    BKNI_Memset(&access, 0, sizeof(access));

    BDBG_ASSERT(client->pid);
    access.pid = client->pid;

    if (grant) {
        unsigned i,j;
        for (i=0,j=0;i<NEXUS_MAX_HEAPS && j<BCM_MAX_HEAPS;i++) {
            NEXUS_MemoryStatus status;
            if (client->client_state.client.config.heap[i]) {
                rc = NEXUS_Heap_GetStatus_priv(client->client_state.client.config.heap[i], &status);
                if (rc) {rc = BERR_TRACE(rc); continue;}
                /* a heap may be granted for HW access, but not mmap access */
                if (
                    (status.memoryType & NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED) == NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED ||
                    (status.memoryType & NEXUS_MEMORY_TYPE_APPLICATION_CACHED) == NEXUS_MEMORY_TYPE_APPLICATION_CACHED
                    ) {
                    access.heap[j].offset = status.offset;
                    access.heap[j].size = status.size;
                    j++;
                }
            }
        }
    }
    /* if !grant, memset(0) is sufficient to revoke all access. */

    rc = ioctl(g_NEXUS_driverFd, BRCM_IOCTL_SET_MMAP_ACCESS, &access);
    if (rc) rc = BERR_TRACE(rc); /* fall through. client mmap may fail, but server keeps going. */

    return;
}

/**
callbackId is a unique number per callback type and client, it can be used to filter redundant callbacks.
**/
static void nexus_server_send_callback_locked(struct b_server_callback_cxn *callback_cxn, void *interface, unsigned callbackId, const NEXUS_CallbackDesc *pCallback)
{
    int rc;
    struct nexus_callback_queue_entry *queue_entry;

    BSTD_UNUSED(callbackId);
    BDBG_OBJECT_ASSERT(callback_cxn, b_server_callback_cxn);
    BDBG_OBJECT_ASSERT(callback_cxn->client, NEXUS_Client);


    /* if connection has closed, do not send */
    if (callback_cxn->fd == -1) {
        return;
    }

    /* flush all data from the back-channel */
    for(;;) {
        uint8_t num;
        /* non-blocking read */
        rc = read(callback_cxn->fd, &num, sizeof(num));
        if (rc != sizeof(num)) {
            break;
        }
    }

    /* 1. check if this callback already queued locally */
    for(queue_entry = BLST_Q_FIRST(&callback_cxn->callback_queued);queue_entry;queue_entry=BLST_Q_NEXT(queue_entry, link)) {
        if(queue_entry->callbackId == callbackId && queue_entry->data.interface == interface) {
            BDBG_MSG_TRACE(("client %p: found duplicate %p(%p,%d)", callback_cxn->client, pCallback->callback, pCallback->context, pCallback->param));
            return;
        }
    }

    /* 2. Get first empty entry */
    queue_entry = nexus_server_get_callback_queue_entry(callback_cxn->client->server);
    if (!queue_entry) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return;
    }

    /* 3. Fill it up and move to the queue */
    queue_entry->data.interface = interface;
    queue_entry->callbackId = callbackId;
    queue_entry->data.callback = *pCallback;
    BLST_Q_INSERT_TAIL(&callback_cxn->callback_queued, queue_entry, link);

    /* 4. Send one at a time */
    nexus_server_write_callback_locked(callback_cxn);
    return;
}


void nexus_server_send_callback(void *slave, void *interface, unsigned callbackId, const NEXUS_CallbackDesc *pCallback)
{
    struct b_server_callback_cxn *callback_cxn = slave;

    BKNI_AcquireMutex(callback_cxn->lock);
    nexus_server_send_callback_locked(callback_cxn, interface, callbackId, pCallback);
    BKNI_ReleaseMutex(callback_cxn->lock);
    return;
}

NEXUS_Error NEXUS_Platform_GetClientStatus( NEXUS_ClientHandle client, NEXUS_ClientStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->connected = (client->fd != -1);
    pStatus->numJoins = client->numJoins;
    pStatus->pid = client->pid;
    return 0;
}

struct b_objdb_client *nexus_p_platform_objdb_client(NEXUS_ClientHandle client)
{
    return &client->client_state.client;
}

NEXUS_Error NEXUS_Platform_AcquireObject(NEXUS_ClientHandle client, const NEXUS_InterfaceName *type, void *object)
{
    return NEXUS_Platform_P_AcquireObject(nexus_p_platform_objdb_client(client), type, object);
}

void NEXUS_Platform_ReleaseObject(const NEXUS_InterfaceName *type, void *object)
{
    NEXUS_Platform_P_ReleaseObject(type, object);
    return;
}

void NEXUS_Platform_GetClientResources( NEXUS_ClientHandle client, NEXUS_ClientResources *pResources )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    *pResources = client->client_state.client.resources.allowed;
}

NEXUS_Error NEXUS_Platform_SetClientResources( NEXUS_ClientHandle client, const NEXUS_ClientResources *pResources )
{
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    rc = nexus_client_resources_check(pResources, &nexus_p_platform_objdb_client(client)->resources.used);
    if (rc) return BERR_TRACE(rc);
    client->client_state.client.resources.allowed = *pResources;
    return 0;
}

#else /* NEXUS_SERVER_SUPPORT */

/* stub the public API */
NEXUS_Error NEXUS_Platform_P_InitServer(void)
{
    NEXUS_Error rc;
#if NEXUS_AUDIO_DSP_DEBUG
    rc = NEXUS_Platform_P_InitAudioLog();
    if ( rc ) { return BERR_TRACE(rc); }
#endif
    rc = NEXUS_Platform_P_InitSageLog();
    if ( rc ) { return BERR_TRACE(rc); }
    return 0;
}
void NEXUS_Platform_P_UninitServer(void)
{
    NEXUS_Platform_P_UninitAudioLog();
    NEXUS_Platform_P_UninitSageLog();
}
NEXUS_Error NEXUS_Platform_StartServer(const NEXUS_PlatformStartServerSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}
void NEXUS_Platform_StopServer(void)
{
}
NEXUS_ClientHandle NEXUS_Platform_RegisterClient( const NEXUS_ClientSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}
void NEXUS_Platform_UnregisterClient( NEXUS_ClientHandle client )
{
    BSTD_UNUSED(client);
}
NEXUS_Error NEXUS_Platform_GetClientStatus( NEXUS_ClientHandle client, NEXUS_ClientStatus *pStatus )
{
    BSTD_UNUSED(client);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_Platform_AcquireObject(NEXUS_ClientHandle client, const NEXUS_InterfaceName *type, void *object)
{
    BSTD_UNUSED(client);
    BSTD_UNUSED(type);
    BSTD_UNUSED(object);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_Platform_ReleaseObject(const NEXUS_InterfaceName *type, void *object)
{
    BSTD_UNUSED(type);
    BSTD_UNUSED(object);
    (void)BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return;
}

void NEXUS_Platform_GetClientResources( NEXUS_ClientHandle client, NEXUS_ClientResources *pResources )
{
    BSTD_UNUSED(client);
    BSTD_UNUSED(pResources);
}

NEXUS_Error NEXUS_Platform_SetClientResources( NEXUS_ClientHandle client, const NEXUS_ClientResources *pResources )
{
    BSTD_UNUSED(client);
    BSTD_UNUSED(pResources);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

struct b_objdb_client *nexus_p_platform_objdb_client(NEXUS_ClientHandle client)
{
    BSTD_UNUSED(client);
    return NULL;
}
#endif /* NEXUS_SERVER_SUPPORT */

/**
The following client functions only have meaning in the client, not in the server.
They exist here because the server is the superset of the client.
**/
NEXUS_Error NEXUS_Platform_AuthenticatedJoin(const NEXUS_ClientAuthenticationSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    BDBG_ERR(("You are calling client api's with a server-only Nexus library. Have you set NEXUS_MODE=client?"));
    return NEXUS_NOT_SUPPORTED;
}

void NEXUS_Platform_GetDefaultClientAuthenticationSettings( NEXUS_ClientAuthenticationSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
}

void NEXUS_Platform_GetClientConfiguration_driver( NEXUS_ClientConfiguration *pSettings )
{
    NEXUS_PlatformConfiguration *config;

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    /* kernel mode server works like client, so we mimic that in user mode */
    config = BKNI_Malloc(sizeof(*config));
    if (!config) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    else {
        NEXUS_Platform_GetConfiguration(config);
        BKNI_Memcpy(pSettings->heap, config->heap, sizeof(pSettings->heap));
        BKNI_Free(config);
    }
}
