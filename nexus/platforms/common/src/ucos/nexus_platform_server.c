/***************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * Module Description:
 *
 ************************************************************/
#include "nexus_base.h"
#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "nexus_platform_server.h"
#include "bstd.h"
#include "bkni.h"

#if NEXUS_SERVER_SUPPORT
#include "nexus_driver.h"
#include "server/nexus_ipc_api.h"
#include "server/nexus_server_prologue.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h> /* see SIGPIPE below */
#endif

BDBG_MODULE(nexus_platform_server);

#if NEXUS_SERVER_SUPPORT

#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

/**
Must extern all module-specific functions. We cannot build a master header file of all modules.
**/
#define NEXUS_PLATFORM_P_DRIVER_MODULE(X) \
    extern int nexus_server_##X##_open(struct nexus_driver_module_header **pHeader); \
    extern int nexus_server_##X##_close(void); \
    extern int nexus_server_##X##_process(void *driver_state, void *in_data, unsigned in_data_size, void *out_data, unsigned out_mem_size, unsigned *p_out_data_size);
#include "nexus_ipc_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE

/**
index of g_nexus_server_handlers[] is the module id.
**/
static const struct {
    const char *name;
    int (*open)(struct nexus_driver_module_header **pHeader);
    int (*close)(void);
    int (*process)(void *driver_state, void *in_data, unsigned in_data_size, void *out_data, unsigned out_mem_size, unsigned *p_out_data_size);
} g_nexus_server_handlers[] = {
#define NEXUS_PLATFORM_P_DRIVER_MODULE(X) \
    {#X, nexus_server_##X##_open, nexus_server_##X##_close, nexus_server_##X##_process},
#include "nexus_ipc_modules.h"
#undef NEXUS_PLATFORM_P_DRIVER_MODULE
};
#define NEXUS_PLATFORM_P_NUM_DRIVERS (sizeof(g_nexus_server_handlers)/sizeof(g_nexus_server_handlers[0]))

struct nexus_driver_state nexus_driver_state;
BDBG_OBJECT_ID(nexus_driver_client_state);

BDBG_OBJECT_ID(b_server_callback_cxn);
struct b_server_callback_cxn {
    BDBG_OBJECT(b_server_callback_cxn)
    BLST_S_ENTRY(b_server_callback_cxn) link;
    struct b_server_callback *callback;
    struct NEXUS_Client *client;
    int fd;
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
instead, then channel thread cleans up connections when it is not processing an IPC calls (see nexus_cleanup_channel_connections).
then, the server thread cleans up clients when all of the client connections have been cleaned up (see nexus_cleanup_clients).
**/

BDBG_OBJECT_ID(NEXUS_ServerChannel);
struct NEXUS_ServerChannel
{
    BDBG_OBJECT(NEXUS_ServerChannel)
    BLST_S_ENTRY(NEXUS_ServerChannel) link;
    unsigned moduleId;
    const char *moduleName;
    unsigned dataSize;
    void *in_data; /* size is dataSize */
    void *out_data; /* size is dataSize */

    struct nexus_driver_module_header *header; /* points to local storage in each module's thunk */
    struct NEXUS_Server *server;
    int listen_fd;
    pthread_t thread;
    BKNI_MutexHandle mutex;
    bool done;
    NEXUS_ModulePriority priority;

    /* list of client connections per module. protected by channel->mutex. */
    BLST_S_HEAD(channel_connection_list, b_client_module_cxn) clients;
};

BDBG_OBJECT_ID(NEXUS_Client);
struct NEXUS_Client
{
    BDBG_OBJECT(NEXUS_Client)
    BLST_S_ENTRY(NEXUS_Client) link;
    NEXUS_ClientSettings settings;
    struct NEXUS_Server *server;
    unsigned pid;
    int fd;
    int pollnum;
    struct nexus_driver_client_state client_state; /* generic struct for nexus_driver_callbacks.c/nexus_driver_objects.c */
    bool dynamicallyCreated;
    bool deleted;

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
    pthread_t thread;
    BKNI_MutexHandle mutex;
    bool done;
    struct b_server_callback callback[NEXUS_ModulePriority_eMax];

    BLST_S_HEAD(channel_list, NEXUS_ServerChannel) channels;
    BLST_S_HEAD(client_list, NEXUS_Client) clients;
};

static struct NEXUS_Server *g_server = NULL;

static void *nexus_server_thread(void *context);
static void *nexus_server_channel_thread(void *context);
static void nexus_unregister_client_lock(NEXUS_ClientHandle client);
static void nexus_autoclose_client_objects_lock(NEXUS_ClientHandle client);
static void nexus_cleanup_clients(struct NEXUS_Server *server, bool *p_pending);
static void nexus_cleanup_channel_connections(struct NEXUS_ServerChannel *channel);

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
    g_server = server;

    BKNI_CreateMutex(&server->mutex);
    nexus_driver_state.lock = server->mutex;

    BDBG_OBJECT_SET(&server->server_client_state, nexus_driver_client_state);
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

        (g_nexus_server_handlers[i].open)(&channel->header);

        BDBG_ASSERT(channel->header);
        channel->moduleId = i;
        channel->moduleName = g_nexus_server_handlers[i].name;
        channel->dataSize = 4096; /* max size for single transaction with unix domain sockets */
        channel->in_data = BKNI_Malloc(channel->dataSize);
        if (!channel->in_data) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            BKNI_Free(channel);
            goto error;
        }
        channel->out_data = BKNI_Malloc(channel->dataSize);
        if (!channel->out_data) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            BKNI_Free(channel->in_data);
            BKNI_Free(channel);
            goto error;
        }

        BKNI_CreateMutex(&channel->mutex);
        channel->server = server;

        BKNI_AcquireMutex(server->mutex);
        BLST_S_INSERT_HEAD(&server->channels, channel, link);
        BKNI_ReleaseMutex(server->mutex);
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

    BDBG_OBJECT_ASSERT(server, NEXUS_Server);

    if (server->thread) {
        /* implicit Stop */
        NEXUS_Platform_StopServer();
    }

    /* close all channels */
    while ((channel = BLST_S_FIRST(&server->channels))) {
        BDBG_OBJECT_ASSERT(channel, NEXUS_ServerChannel);
        BDBG_ASSERT(!channel->thread);
        BDBG_ASSERT(channel->server == server);
        BKNI_AcquireMutex(server->mutex);
        BLST_S_REMOVE(&server->channels, channel, NEXUS_ServerChannel, link);
        BKNI_ReleaseMutex(server->mutex);

        BKNI_DestroyMutex(channel->mutex);
        BKNI_Free(channel->in_data);
        BKNI_Free(channel->out_data);
        nexus_driver_callback_uninit(channel->header);

        (g_nexus_server_handlers[channel->moduleId].close)();

        BDBG_OBJECT_DESTROY(channel, NEXUS_ServerChannel);
        BKNI_Free(channel);
    }

    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        BDBG_ASSERT(server->callback[i].listen_fd == -1);
        BDBG_OBJECT_DESTROY(&server->callback[i], b_server_callback);
    }

    nexus_driver_state.lock = NULL;
    BKNI_DestroyMutex(server->mutex);
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

    BDBG_OBJECT_ASSERT(server, NEXUS_Server);

    if (!pSettings) {
        NEXUS_Platform_GetDefaultStartServerSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }
    server->settings = *pSettings;

    BDBG_WRN(("NEXUS_Platform_StartServer %d modules", NEXUS_PLATFORM_P_NUM_DRIVERS));

    rc = pthread_create(&server->thread, NULL, nexus_server_thread, server);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    /* TODO: consolidate some modules on a single thread */
    for (channel = BLST_S_FIRST(&server->channels); channel; channel = BLST_S_NEXT(channel, link)) {
        BDBG_OBJECT_ASSERT(channel, NEXUS_ServerChannel);
        rc = pthread_create(&channel->thread, NULL, nexus_server_channel_thread, channel);
        if (rc) {rc = BERR_TRACE(rc); goto error;}
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

    BDBG_WRN(("NEXUS_Platform_StopServer"));

    /* unregister clients. they will be cleaned up when the channel threads exit. */
    BKNI_AcquireMutex(server->mutex);
    for (client = BLST_S_FIRST(&server->clients); client; client = BLST_S_NEXT(client, link)) {
        nexus_unregister_client_lock(client);
    }
    BKNI_ReleaseMutex(server->mutex);

    /* close all channels first. this stops all communication in-process. */
    for (channel = BLST_S_FIRST(&server->channels); channel; channel = BLST_S_NEXT(channel, link)) {
        BDBG_OBJECT_ASSERT(channel, NEXUS_ServerChannel);
        if (channel->thread) {
            channel->done = true;
            shutdown(channel->listen_fd, SHUT_RDWR); /* cause listening thread to wake up */
            pthread_join(channel->thread, NULL);
            channel->thread = 0;
            BDBG_ASSERT(channel->listen_fd == -1);
        }
    }

    /* terminate main server thread. this stops all new communication and the last thread. */
    if (server->thread) {
        server->done = true;
        shutdown(server->listen_fd, SHUT_RDWR); /* cause listening thread to wake up */
        pthread_join(server->thread, NULL);
        server->thread = 0;
        BDBG_ASSERT(server->listen_fd == -1);
    }
    nexus_cleanup_clients(server, &pending);
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
        *paddr = (void *)NEXUS_AddrToOffset(addr);
    }
    return;
}


void
nexus_driver_recv_addr_cached(void **paddr)
{
    void *addr = *paddr;
    /* inplace convert address from physical to virtual */
    if(addr) {
        *paddr = NEXUS_OffsetToCachedAddr((uint32_t)addr);
    }
    return;
}

static ssize_t do_read(int fd, void *data, size_t size)
{
    ssize_t rc;
    do {
        rc = read(fd, data, size);
    } while (rc < 0 && (errno == EAGAIN || errno == EINTR));
    return rc;
}

static ssize_t do_write(int fd, const void *data, size_t size)
{
    ssize_t rc;
    do {
        rc = write(fd, data, size);
    } while (rc < 0 && (errno == EAGAIN || errno == EINTR));
    return rc;
}

static void nexus_cleanup_clients(struct NEXUS_Server *server, bool *p_pending)
{
    NEXUS_ClientHandle client;

    BKNI_AcquireMutex(server->mutex);
    BDBG_OBJECT_ASSERT(server, NEXUS_Server);
    *p_pending = false;

    /* if there are no more connections, then we can remove the client */
    for (client = BLST_S_FIRST(&server->clients); client; ) {
        NEXUS_ClientHandle next;

        BDBG_OBJECT_ASSERT(client, NEXUS_Client);
        next = BLST_S_NEXT(client, link);

        if (client->deleted && !BLST_S_FIRST(&client->connections)) {
            /* we can delete this client */
            unsigned i;

            BLST_S_REMOVE(&server->clients, client, NEXUS_Client, link);
            for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
                struct b_server_callback_cxn *callback_cxn = &client->callback_cxn[i];
                BDBG_OBJECT_ASSERT(callback_cxn, b_server_callback_cxn);
                if (callback_cxn->fd != -1) {
                    close(callback_cxn->fd);
                }
                BLST_S_REMOVE(&callback_cxn->callback->clients, callback_cxn, b_server_callback_cxn, link);
            }

            BDBG_OBJECT_DESTROY(client, NEXUS_Client);
            BKNI_Free(client);
        }
        else if (client->deleted) {
            *p_pending = true;
        }

        client = next;
    }
    BKNI_ReleaseMutex(server->mutex);
}

static void *nexus_server_thread(void *context)
{
    struct NEXUS_Server *server = context;
    struct sockaddr_un sock_addr;
    int rc;
    unsigned i;

    /* set up server's main listener socket */
    server->listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server->listen_fd == -1) {
        rc = BERR_TRACE(errno);
        return 0;
    }

    /* TODO: set filesystem permissions on the socket. don't require root. */
    BKNI_Memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strcpy(sock_addr.sun_path, B_UNIX_MAIN_SOCKET_STR);
    unlink(sock_addr.sun_path);
    rc = bind(server->listen_fd, (struct sockaddr *)&sock_addr, strlen(sock_addr.sun_path)+sizeof(sock_addr.sun_family));
    if (rc == -1) {
        rc = BERR_TRACE(errno);
        goto done;
    }

    rc = listen(server->listen_fd, 10);
    if (rc == -1) {
        rc = BERR_TRACE(errno);
        goto done;
    }

    /* set up callback listener sockets */
    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct b_server_callback *callback = &server->callback[i];
        BDBG_OBJECT_ASSERT(callback, b_server_callback);

        callback->listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (callback->listen_fd == -1) {
            rc = BERR_TRACE(errno);
            return 0;
        }

        /* TODO: set filesystem permissions on the socket. don't require root. */
        BKNI_Memset(&sock_addr, 0, sizeof(sock_addr));
        sock_addr.sun_family = AF_UNIX;
        sprintf(sock_addr.sun_path, B_UNIX_SCHEDULER_SOCKET_FMT_STR, i);
        unlink(sock_addr.sun_path);
        rc = bind(callback->listen_fd, (struct sockaddr *)&sock_addr, strlen(sock_addr.sun_path)+sizeof(sock_addr.sun_family));
        if (rc == -1) {
            rc = BERR_TRACE(errno);
            goto done;
        }

        rc = listen(callback->listen_fd, 10);
        if (rc == -1) {
            rc = BERR_TRACE(errno);
            goto done;
        }
    }

#define B_MAX_CLIENTS 16 /* required for poll() */

    BDBG_MSG(("server listening..."));
    while (!server->done) {
        struct pollfd fds[1+NEXUS_ModulePriority_eMax+B_MAX_CLIENTS];
        unsigned num = 0;
        struct NEXUS_Client *client;

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

        /* wake up every second to see if app is exiting or client list has changed */
        while (!server->done) {
            bool pending;
            unsigned timeout = 1000;
            nexus_cleanup_clients(server, &pending);
            if (pending) {
                timeout = 1; /* very short timeout if channel threads are cleaning up */
            }

            /* add clients to detect disconnects. note that client->pollnum cannot be zero if included in the poll. */
            BKNI_AcquireMutex(server->mutex);
            for (client = BLST_S_FIRST(&server->clients); num<1+NEXUS_ModulePriority_eMax+B_MAX_CLIENTS && client; client = BLST_S_NEXT(client, link)) {
                if (client->fd != -1) {
                    client->pollnum = num;
                    fds[num].fd = client->fd;
                    fds[num].events = POLLIN;
                    num++;
                }
            }
            BKNI_ReleaseMutex(server->mutex);

            rc = poll(fds, num, timeout);
            if (rc < 0) {
                rc = BERR_TRACE(errno);
                goto done;
            }
            else if (rc > 0) {
                break;
            }
        }

        /* check if a client has disconnected */
        BKNI_AcquireMutex(server->mutex);
        for (client = BLST_S_FIRST(&server->clients); client; ) {
            struct NEXUS_Client *next;
            BDBG_OBJECT_ASSERT(client, NEXUS_Client);
            next = BLST_S_NEXT(client, link);
            if (client->fd != -1) {
                if (fds[client->pollnum].revents & (POLLERR|POLLHUP)) {
                    BDBG_MSG(("close client %p", client));
                    close(client->fd);
                    client->fd = -1;
                    /* disconnect callbacks */
                    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
                        struct b_server_callback_cxn *callback_cxn = &client->callback_cxn[i];
                        BDBG_OBJECT_ASSERT(callback_cxn, b_server_callback_cxn);
                        if (callback_cxn->fd != -1) {
                            BDBG_MSG(("disconnect client(%p) from callback(%d)", client, i));
                            close(callback_cxn->fd);
                            callback_cxn->fd = -1;
                        }
                    }
                    nexus_autoclose_client_objects_lock(client);
                    if (client->dynamicallyCreated) {
                        nexus_unregister_client_lock(client);
                    }
                }
            }
            client = next;
        }
        BKNI_ReleaseMutex(server->mutex);

        /* check listener for a new client or client callback connection */
        for (i=0;i<1+NEXUS_ModulePriority_eMax;i++) {
            if (fds[i].revents) {
                int fd;
                struct ucred credentials;
                socklen_t ucred_length = sizeof(struct ucred);

                fd = accept(i == 0 ? server->listen_fd : server->callback[i-1].listen_fd, NULL, NULL);
                if (fd < 0) {
                    if (!server->done) {
                        rc = BERR_TRACE(errno);
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
                        NEXUS_ClientAuthenticationSettings auth;
                        struct nexus_client_init_data init_data;

                        /* some manual ipc that matches nexus_platform_client.c code */
                        rc = do_read(fd, &auth, sizeof(auth));
                        if (rc != sizeof(auth)) {
                            BDBG_ERR(("unable to read authentication settings: %d", rc));
                            close(fd);
                            continue;
                        }

                        /* search already registered clients */
                        BKNI_AcquireMutex(server->mutex);
                        for (client = BLST_S_FIRST(&server->clients); client; client = BLST_S_NEXT(client, link)) {
                            if (client->fd == -1) {
                                if (!BKNI_Memcmp(&client->settings.authentication.certificate, &auth.certificate, sizeof(client->settings.authentication.certificate))) {
                                    /* found it */
                                    break;
                                }
                            }
                        }
                        BKNI_ReleaseMutex(server->mutex);

                        /* if not found, and if untrusted clients are allowed, create a new one */
                        if (!client && server->settings.allowUnauthenticatedClients) {
                            NEXUS_ClientSettings clientSettings;
                            NEXUS_Platform_GetDefaultClientSettings(&clientSettings);
                            clientSettings.configuration.trusted = server->settings.trustUnauthenticatedClients;
                            client = NEXUS_Platform_RegisterClient(&clientSettings);
                            if (client) {
                                client->dynamicallyCreated = true;
                            }
                        }

                        /* unable to authenticate or allow this client */
                        if (!client) {
                            BDBG_ERR(("rejected unauthenticated client %d connection", credentials.pid));
                            close(fd);
                            continue;
                        }

                        BKNI_Memset(&init_data, 0, sizeof(init_data));
                        init_data.config = client->settings.configuration;
                        for (i=0;i<NEXUS_MAX_HEAPS;i++) {
                            if (init_data.config.heap[i]) {
                                NEXUS_MemoryStatus status;
                                init_data.heap[i].heap = init_data.config.heap[i];
                                rc = NEXUS_Heap_GetStatus(init_data.heap[i].heap, &status);
                                if (rc) continue;
                                init_data.heap[i].offset  = status.offset;
                                init_data.heap[i].size = status.size;
                                init_data.heap[i].memoryType = status.memoryType;
                            }
                        }
                        rc = do_write(fd, &init_data, sizeof(init_data));
                        if (rc != sizeof(init_data)) {
                            BDBG_ERR(("unable to confirm client: %d", rc));
                            close(fd);
                            if (client->dynamicallyCreated) {
                                NEXUS_Platform_UnregisterClient(client);
                            }
                            continue;
                        }

                        /* this client is ready */
                        client->pid = credentials.pid;
                        client->fd = fd;
                    }
                    else {
                        struct NEXUS_Client *client;
                        unsigned priority = i-1;

                        /* new callback connection */
                        /* find the client */
                        BKNI_AcquireMutex(server->mutex);
                        for (client = BLST_S_FIRST(&server->clients); client; client = BLST_S_NEXT(client, link)) {
                            if (client->pid == (unsigned)credentials.pid) {
                                if (client->callback_cxn[priority].fd != -1) {
                                    BDBG_ERR(("client(%p) callback(%d) already connected", client, priority));
                                    client = NULL;
                                }
                                break;
                            }
                        }
                        BKNI_ReleaseMutex(server->mutex);
                        if (client) {
                            /* client has connected */
                            BDBG_MSG(("connect client(%p) to callback(%d)", client, priority));
                            client->callback_cxn[priority].fd = fd;
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
    close(server->listen_fd);
    server->listen_fd = -1;

    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct b_server_callback *callback = &server->callback[i];
        BDBG_OBJECT_ASSERT(callback, b_server_callback);
        close(callback->listen_fd);
        callback->listen_fd = -1;
    }

    return 0;
}

static void nexus_cleanup_channel_connections(struct NEXUS_ServerChannel *channel)
{
    struct b_client_module_cxn *cxn;

    BDBG_OBJECT_ASSERT(channel, NEXUS_ServerChannel);
    BKNI_AcquireMutex(channel->server->mutex);
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
    BKNI_ReleaseMutex(channel->server->mutex);
}

/* this thread never acquires the server mutex */
static void *nexus_server_channel_thread(void *context)
{
    struct NEXUS_ServerChannel *channel = context;
    struct sockaddr_un sock_addr;
    int rc;

    channel->listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (channel->listen_fd == -1) {
        rc = BERR_TRACE(errno);
        return 0;
    }

    BKNI_Memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    sprintf(sock_addr.sun_path, B_UNIX_SOCKET_FORMAT_STR, channel->moduleId);

    unlink(sock_addr.sun_path);
    rc = bind(channel->listen_fd, (struct sockaddr *)&sock_addr, strlen(sock_addr.sun_path)+sizeof(sock_addr.sun_family));
    if (rc == -1) {
        rc = BERR_TRACE(errno);
        goto done;
    }

    rc = listen(channel->listen_fd, 10);
    if (rc == -1) {
        rc = BERR_TRACE(errno);
        goto done;
    }

    BDBG_MSG(("%s(%d) server channel listening...", channel->moduleName, channel->moduleId));
    while (!channel->done) {
        struct pollfd fds[B_MAX_CLIENTS+1];
        unsigned num = 0;
        struct b_client_module_cxn *cxn;

        /* add listener */
        fds[num].fd = channel->listen_fd;
        fds[num].events = POLLIN;
        num++;

        /* wake up every second to see if app is exiting */
        while (!channel->done) {
            nexus_cleanup_channel_connections(channel);

            /* add clients. note that client->pollnum cannot be zero if included in the poll. */
            BKNI_AcquireMutex(channel->mutex);
            for (cxn = BLST_S_FIRST(&channel->clients); num<B_MAX_CLIENTS && cxn; cxn = BLST_S_NEXT(cxn, channel_link)) {
                BDBG_OBJECT_ASSERT(cxn, b_client_module_cxn);
                if (cxn->fd != -1) {
                    cxn->pollnum = num;
                    fds[num].fd = cxn->fd;
                    fds[num].events = POLLIN;
                    num++;
                }
            }
            BKNI_ReleaseMutex(channel->mutex);

            rc = poll(fds, num, 1000);
            if (rc < 0) {
                rc = BERR_TRACE(errno);
                goto done;
            }
            else if (rc > 0) {
                break;
            }
        }

        /* check clients */
        BKNI_AcquireMutex(channel->mutex);
        for (cxn = BLST_S_FIRST(&channel->clients); cxn; cxn = BLST_S_NEXT(cxn, channel_link)) {
            BDBG_OBJECT_ASSERT(cxn, b_client_module_cxn);
            if (!cxn->deleted && cxn->fd != -1) {
                if (fds[cxn->pollnum].revents & (POLLERR|POLLHUP)) {
                    BDBG_OBJECT_ASSERT(cxn->channel, NEXUS_ServerChannel);
                    BDBG_OBJECT_ASSERT(cxn->client, NEXUS_Client);
                    BDBG_MSG(("disconnect client(%p) from module(%d)", cxn->client, cxn->channel->moduleId));
                    close(cxn->fd);
                    cxn->fd = -1;
                }
                else if (fds[cxn->pollnum].revents) {
                    /* we're going to make an IPC call. we must unlock, but this should be safe
                    because the cxn and client are guaranteed to not be destroyed here. */

                    BKNI_ReleaseMutex(channel->mutex);

                    /* process client message */
                    rc = do_read(cxn->fd, channel->in_data, channel->dataSize);
                    if (rc < 0) {
                        /* not necessarily an error. client may have closed, which is normal. */
                        BDBG_MSG(("read failed: %d %d", rc, errno));
                        close(cxn->fd);
                        cxn->fd = -1;
                    }
                    else {
                        unsigned out_size;

                        /*BDBG_MSG_TRACE(("read %d", rc));*/
                        rc = (*g_nexus_server_handlers[channel->moduleId].process)(&cxn->client_module_state, channel->in_data, rc, channel->out_data, channel->dataSize, &out_size);
                        if (rc) {
                            BDBG_ERR(("client(%p,%p,%d) module(%d) call failed: %d", cxn->client, cxn, cxn->fd, cxn->channel->moduleId, rc));
                            close(cxn->fd);
                            cxn->fd = -1;
                            /* fall through */
                        }
                        else {
                            rc = do_write(cxn->fd, channel->out_data, out_size);
                            if (rc < 0) {
                                BDBG_MSG(("write failed: %d %d", rc, errno));
                                close(cxn->fd);
                                cxn->fd = -1;
                            }
                            /*BDBG_MSG_TRACE(("wrote %d out of %d", rc, channel->dataSize));*/
                        }
                    }

                    BKNI_AcquireMutex(channel->mutex);
                }
            }
        }
        BKNI_ReleaseMutex(channel->mutex);

        /* check listener for client module connection */
        if (fds[0].revents) {
            int fd;
            struct ucred credentials;
            socklen_t ucred_length = sizeof(struct ucred);

            fd = accept(channel->listen_fd, NULL, NULL);
            if (fd < 0) {
                if (!channel->done) {
                    rc = BERR_TRACE(errno);
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
                            BDBG_ERR(("client(%p) module(%d) already connected", cxn->client, cxn->channel->moduleId));
                            cxn = NULL;
                        }
                        break;
                    }
                }
                if (cxn) {
                    /* client has connected */
                    BDBG_MSG(("connect client(%p) to module(%d)", cxn->client, cxn->channel->moduleId));
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
    nexus_cleanup_channel_connections(channel);
    return NULL;
}

NEXUS_ClientHandle NEXUS_Platform_RegisterClient(const NEXUS_ClientSettings *pSettings)
{
    NEXUS_ClientHandle client;
    struct NEXUS_Server *server = g_server;
    struct NEXUS_ServerChannel *channel;
    unsigned i;
    NEXUS_ClientSettings defaultSettings;

    BDBG_OBJECT_ASSERT(server, NEXUS_Server);

    if (!pSettings) {
        NEXUS_Platform_GetDefaultClientSettings(&defaultSettings);
        defaultSettings.configuration.trusted = true;
        pSettings = &defaultSettings;
    }

    client = BKNI_Malloc(sizeof(*client));
    if (!client) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    BKNI_Memset(client, 0, sizeof(*client));
    BDBG_OBJECT_SET(client, NEXUS_Client);
    client->pid = 0;
    client->fd = -1;
    client->server = server;
    client->settings = *pSettings;
    BDBG_OBJECT_SET(&client->client_state, nexus_driver_client_state);
    client->client_state.client.trusted = client->settings.configuration.trusted;

    for (i=0;i<NEXUS_MAX_HEAPS;i++) {
        if (client->settings.configuration.heap[i]) break;
    }
    if (client->client_state.client.trusted && i == NEXUS_MAX_HEAPS) {
        /* if trusted and no heaps specified, give all heaps */
        NEXUS_PlatformConfiguration config;
        NEXUS_Platform_GetConfiguration(&config);
        BDBG_CASSERT(sizeof(client->client_state.heap) == sizeof(config.heap));
        BKNI_Memcpy(client->settings.configuration.heap, config.heap, sizeof(client->client_state.heap));
    }
    /* load client heaps into ipc thunk */
    BKNI_Memcpy(client->client_state.heap, client->settings.configuration.heap, sizeof(client->client_state.heap));

    BKNI_AcquireMutex(server->mutex);
    BLST_S_INSERT_HEAD(&server->clients, client, link);

    /* pre-create all callback connections */
    for (i=0;i<NEXUS_ModulePriority_eMax;i++) {
        struct b_server_callback_cxn *callback_cxn = &client->callback_cxn[i];
        BDBG_OBJECT_SET(callback_cxn, b_server_callback_cxn);
        callback_cxn->fd = -1;
        callback_cxn->client = client;
        callback_cxn->callback = &server->callback[i];
        BLST_S_INSERT_HEAD(&callback_cxn->callback->clients, callback_cxn, link);
        BDBG_OBJECT_ASSERT(callback_cxn->callback, b_server_callback);
    }

    /* pre-create all module connections */
    for (channel = BLST_S_FIRST(&server->channels); channel; channel = BLST_S_NEXT(channel, link)) {
        struct b_client_module_cxn *cxn;
        NEXUS_ModuleSettings moduleSettings;

        cxn = BKNI_Malloc(sizeof(*cxn));
        if (!cxn) {
            BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto err_malloc_cxn;
        }
        BKNI_Memset(cxn, 0, sizeof(*cxn));
        BDBG_OBJECT_SET(cxn, b_client_module_cxn);
        cxn->fd = -1; /* will listen for this */
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

    BKNI_ReleaseMutex(server->mutex);

    BDBG_MSG(("registered client(%p)", client));

    return client;

err_malloc_cxn:
    nexus_unregister_client_lock(client);
    BKNI_ReleaseMutex(server->mutex);
err_malloc:
    return NULL;
}

/* NEXUS_Platform_UnregisterClient can be called for explicitly and dynamically registered clients */
void NEXUS_Platform_UnregisterClient( NEXUS_ClientHandle client )
{
    struct NEXUS_Server *server = g_server;
    BKNI_AcquireMutex(server->mutex);
    nexus_unregister_client_lock(client);
    BKNI_ReleaseMutex(server->mutex);
}

static void nexus_autoclose_client_objects_lock(NEXUS_ClientHandle client)
{
    /* auto-close all objects associated with this client */
    struct NEXUS_Server *server = g_server;
    struct NEXUS_ServerChannel *channel;

    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    BDBG_MSG(("autoclose client(%p)", client));

    /* clean up anything left open */
    for (channel = BLST_S_FIRST(&server->channels); channel; channel = BLST_S_NEXT(channel, link)) {
        struct nexus_driver_module_header *header;
        header = channel->header;
        if (!header) {
            continue;
        }
        while (1) {
            struct b_objdb_entry *entry;
            struct b_objdb_class *p_class;
            if (!b_objdb_get_newest_entry(&header->objdb, &client->client_state.client, &p_class, &entry)) {
                b_objdb_uninit_entry(p_class, entry, &client->client_state.client);
            }
            else {
                break;
            }
        }
    }
}

/* we cannot delete clients or connections in this function. we can only
mark them for deletion. */
static void nexus_unregister_client_lock(NEXUS_ClientHandle client)
{
    struct b_client_module_cxn *cxn;

    BDBG_OBJECT_ASSERT(client, NEXUS_Client);

    BDBG_MSG(("nexus_unregister_client_lock: %p %d", client, client->fd));
    if (client->fd != -1) {
        close(client->fd);
        client->fd = -1;
    }
    nexus_autoclose_client_objects_lock(client);
    client->deleted = true; /* nexus_cleanup_clients will delete once every connection is deleted */

    for (cxn = BLST_S_FIRST(&client->connections); cxn; cxn = BLST_S_NEXT(cxn, client_link)) {
        BDBG_OBJECT_ASSERT(cxn, b_client_module_cxn);
        cxn->deleted = true; /* nexus_cleanup_channel_connections will delete connections */
        if (cxn->fd != -1) {
            close(cxn->fd);
            cxn->fd = -1;
        }
    }
}

void nexus_server_send_callback(void *slave, void *interface, const NEXUS_CallbackDesc *pCallback)
{
    struct nexus_callback_data data;
    struct b_server_callback_cxn *callback_cxn = slave;
    int rc;

    BDBG_OBJECT_ASSERT(callback_cxn, b_server_callback_cxn);
    BDBG_OBJECT_ASSERT(callback_cxn->client, NEXUS_Client);

    /* if connection has closed, do not send */
    if (callback_cxn->fd == -1) {
        return;
    }

    BDBG_MSG_TRACE(("send callback %p(%p,%d)", pCallback->callback, pCallback->context, pCallback->param));
    data.interface = interface;
    data.callback = *pCallback;
    /* for unix domain sockets, writes < 4K are atomic. no mutex needed. */
    rc = do_write(callback_cxn->fd, &data, sizeof(data));
    if (rc != sizeof(data)) {
        BDBG_WRN(("nexus_server_send_callback write failed: %d", rc, errno));
    }
}

#else /* NEXUS_SERVER_SUPPORT */

/* stub the public API */
NEXUS_Error NEXUS_Platform_P_InitServer(void)
{
    return 0;
}
void NEXUS_Platform_P_UninitServer(void)
{
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
#endif /* NEXUS_SERVER_SUPPORT */

/**
The following client functions only have meaning in the client, not in the server.
They exist here because the server is the superset of the client.
**/
NEXUS_Error NEXUS_Platform_AuthenticatedJoin(const NEXUS_ClientAuthenticationSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    BDBG_ERR(("You are calling client api's with a server-only Nexus library. Have you set NEXUS_MODE=client?"));
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_Platform_GetClientConfiguration( NEXUS_ClientConfiguration *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

}
