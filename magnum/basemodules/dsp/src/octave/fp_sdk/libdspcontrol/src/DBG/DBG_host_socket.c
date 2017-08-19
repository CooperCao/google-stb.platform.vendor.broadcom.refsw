/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

/*
 * Implementation of the host side of the DBG module that opens
 * a TCP socket and serves one client at a time.
 *
 * TODO: use the poll syscall instead of the select one
 * TODO: use SO_REUSEPORT on the server socket where available
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include "libdspcontrol/CHIP.h"
#include "fp_sdk_config.h"

#include "libdspcontrol/DSPLOG.h"
#include "libdspcontrol/OS.h"

#include "DBG_interfaces.h"



#if !FEATURE_IS(DBG_HOST_IF, SOCKET)
#  error "This source is for the socket variant of the DBG host interface"
#endif
#if !FEATURE_IS(SOCKETS, SUPPORTED)
#  error "This module requires support for sockets"
#endif


static void DBG_checkNewConnections(DBG_HOST *p_dbg_host);


DBG_RET DBG_hostInit(DBG_HOST *p_dbg_host, DBG_PARAMETERS *parameters)
{
    int ret;
    struct sockaddr_in socket_address;
    socklen_t socket_address_len = sizeof(struct sockaddr_in *);

    p_dbg_host->server_fd = -1;
    p_dbg_host->data_fd = -1;
    p_dbg_host->server_port = 0;
    p_dbg_host->id = parameters->id;

    DSPLOG_INFO("DBG(%u): trying to open server on port %"PRIu16,
                p_dbg_host->id, parameters->socket_port);

#define DBG_hostInit_FAIL(...)      \
    do {                            \
        if(p_dbg_host->server_fd != -1) {  \
            OS_close(p_dbg_host->server_fd);  \
            p_dbg_host->server_fd = -1;    \
        }                           \
        DSPLOG_ERROR(__VA_ARGS__);  \
        return DBG_FAILURE;         \
    } while (0)

    /*  create the server socket */
    p_dbg_host->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(p_dbg_host->server_fd == -1)
        DBG_hostInit_FAIL("DBG(%u): unable to create a server socket: \"%s\" (%d)",
                          p_dbg_host->id, strerror(errno), errno);

    /*  enable the REUSEADDR option */
    ret = setsockopt(p_dbg_host->server_fd, SOL_SOCKET, SO_REUSEADDR, (void *) &ret, sizeof(ret));
    if(ret == -1)
        DBG_hostInit_FAIL("DBG(%u): unable to call setsockopt on server socket: \"%s\" (%d)",
                          p_dbg_host->id, strerror(errno), errno);

    /*  set socket in non-blocking mode */
    ret = fcntl(p_dbg_host->server_fd, F_GETFL);
    ret = fcntl(p_dbg_host->server_fd, F_SETFL, ret | O_NONBLOCK);
    if(ret == -1)
        DBG_hostInit_FAIL("DBG(%u): unable to call fcntl on server socket: %s (%d)",
                          p_dbg_host->id, strerror(errno), errno);

    /*  bind & listen */
    memset(&socket_address, 0, sizeof(struct sockaddr_in));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(parameters->socket_port);

    ret = bind(p_dbg_host->server_fd, (struct sockaddr *) &socket_address, sizeof(struct sockaddr_in));
    if(ret == -1)
        DBG_hostInit_FAIL("DBG(%u): unable to call bind on server socket: \"%s\" (%d)",
                          p_dbg_host->id, strerror(errno), errno);

    ret = listen(p_dbg_host->server_fd, 1);
    if(ret == -1)
        DBG_hostInit_FAIL("DBG(%u): unable to listen on server socket: \"%s\" (%d)",
                          p_dbg_host->id, strerror(errno), errno);

    if(getsockname(p_dbg_host->server_fd, (struct sockaddr *) &socket_address, &socket_address_len) == -1)
        DBG_hostInit_FAIL("DBG(%u): error upon calling getsockname on server socket: \"%s\" (%d)",
                          p_dbg_host->id, strerror(errno), errno);
    else
        p_dbg_host->server_port = ntohs(socket_address.sin_port);

    DSPLOG_INFO("DBG(%u): server open on port %"PRIu16", socket fd is %d",
                p_dbg_host->id, p_dbg_host->server_port, p_dbg_host->server_fd);

    return DBG_SUCCESS;
}


void DBG_hostFinish(DBG_HOST *p_dbg_host)
{
    if(p_dbg_host->data_fd != -1)
    {
        DSPLOG_DEBUG("DBG(%u): closing client connection", p_dbg_host->id);
        OS_close(p_dbg_host->data_fd);
        p_dbg_host->data_fd = -1;
    }

    if(p_dbg_host->server_fd != -1)
    {
        DSPLOG_DEBUG("DBG(%u): closing server socket", p_dbg_host->id);
        OS_close(p_dbg_host->server_fd);
        p_dbg_host->server_fd = -1;
    }
}


bool DBG_hostExchangeData(DBG_HOST *p_dbg_host, uint8_t *send_buffer, size_t *send_length,
                          uint8_t *receive_buffer, size_t *receive_length)
{
    size_t receive_available_space = *receive_length;
    size_t data_to_send = *send_length;
    int result;
    ssize_t size;

    DBG_checkNewConnections(p_dbg_host);

    /* initialise return values */
    *receive_length = 0;
    *send_length = 0;

    if(p_dbg_host->data_fd == -1)
    {
        /* no client is connected, there's not much we can do */
        return false;
    }
    else
    {
        /*  check if we can read or write */
        fd_set read_set, write_set;
        struct timeval timeout = {0, 0};  /*  just poll */

        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_SET(p_dbg_host->data_fd, &read_set);
        FD_SET(p_dbg_host->data_fd, &write_set);
        result = select(p_dbg_host->data_fd + 1, &read_set, &write_set, NULL, &timeout);
        if(result < 0)
        {
            DSPLOG_ERROR("DBG: select returned %d, \"%s\" (%d)", result, strerror(errno), errno);
            return false;
        }

        if(FD_ISSET(p_dbg_host->data_fd, &read_set) && receive_available_space > 0)
        {
            size = OS_read(p_dbg_host->data_fd, receive_buffer, receive_available_space);
            if(size <= 0)
            {
                if(size == 0)
                    DSPLOG_DETAIL("DBG(%u): client disconnected", p_dbg_host->id);
                if(size < 0)
                    DSPLOG_ERROR("DBG(%u): error while reading data from client, "
                                 "read returned %zd, disconnecting", p_dbg_host->id, size);
                OS_close(p_dbg_host->data_fd);
                p_dbg_host->data_fd = -1;

                return false;   /*  nothing else to do, really */
            }
            else
            {
                *receive_length = (size_t) size;
                DSPLOG_DEBUG("DBG(%u): received %zu bytes from the client", p_dbg_host->id, *receive_length);
            }
        }

        if(FD_ISSET(p_dbg_host->data_fd, &write_set) && data_to_send > 0)
        {
            size = OS_write(p_dbg_host->data_fd, send_buffer, data_to_send);
            if(size <= 0)
            {
                if(size == 0)
                    DSPLOG_DETAIL("DBG(%u): client disconnected", p_dbg_host->id);
                if(size < 0)
                    DSPLOG_ERROR("DBG(%u): error while writing data to client, "
                                 "write returned %zd, disconnecting", p_dbg_host->id, size);
                OS_close(p_dbg_host->data_fd);
                p_dbg_host->data_fd = -1;
            }
            else
            {
                *send_length = (size_t) size;
                DSPLOG_DEBUG("DBG(%u): sent %zu bytes to the client", p_dbg_host->id, *send_length);
            }
        }

        return false;   /*  it's not worth issuing another select */
    }
}


void DBG_checkNewConnections(DBG_HOST *p_dbg_host)
{
    struct timeval timeout = { 0, 0 };  /*  just poll */
    int result;
    int new_client_fd;
    struct sockaddr_in new_client;
    unsigned size;
    char host_name[NI_MAXHOST], service_name[NI_MAXSERV];
    int ret;
    fd_set fdset;

    if(p_dbg_host->server_fd == -1)
        return;

    FD_ZERO(&fdset);
    FD_SET(p_dbg_host->server_fd, &fdset);
    result = select(p_dbg_host->server_fd + 1, &fdset, NULL, NULL, &timeout);
    if(result < 0)
    {
        DSPLOG_ERROR("DBG: select returned %d, \"%s\" (%d)", result, strerror(errno), errno);
        return;
    }

    if(FD_ISSET(p_dbg_host->server_fd, &fdset))
    {
        do
        {
            size = sizeof(struct sockaddr_in);
            new_client_fd = accept(p_dbg_host->server_fd, (struct sockaddr *) &new_client, &size);
        } while((new_client_fd == -1) && (errno == EINTR));

        if(new_client_fd == -1)
        {
            DSPLOG_ERROR("DBG(%u): error while accepting a new connection: \"%s\" (%d)",
                         p_dbg_host->id, strerror(errno), errno);
            return;
        }

       if(getnameinfo((struct sockaddr *) &new_client, sizeof(struct sockaddr_in),
                      host_name, sizeof(host_name),
                      service_name, sizeof(service_name), 0) == 0)
           DSPLOG_DETAIL("DBG(%u): client connected from %s:%s",
                        p_dbg_host->id, host_name, service_name);
       else
           DSPLOG_DETAIL("DBG(%u): client connected", p_dbg_host->id);

       /* Following the inclusion of the debug server, the stratergy for the host socket is
        * altered.
        * In this scheme, if there is a new connection, it is accepted and the old
        * socket is dropped.
        */
       if(p_dbg_host->data_fd != -1)
       {
           OS_close(p_dbg_host->data_fd);
           DSPLOG_DEBUG("DBG(%u): old client dropped, only one connection at a time is supported", p_dbg_host->id);
       }
       /*  accept the connection */
       p_dbg_host->data_fd = new_client_fd;

       /*  disable Nagle's algorithm */
       ret = setsockopt(p_dbg_host->data_fd, SOL_TCP, TCP_NODELAY, &ret, sizeof(ret));
       if(ret == -1)
       {
           DSPLOG_ERROR("DBG(%u): unable to call setsockopt on socket: \"%s\" (%d), "
                        "closing connection", p_dbg_host->id, strerror(errno), errno);
           OS_close(p_dbg_host->data_fd);
           p_dbg_host->data_fd = -1;
       }

    }
}
