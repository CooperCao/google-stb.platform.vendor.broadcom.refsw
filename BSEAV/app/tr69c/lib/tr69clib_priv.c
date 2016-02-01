/******************************************************************************
 *    (c)2014 Broadcom Corporation
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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include "bstd.h"
#include "tr69clib.h"
#include "tr69clib_priv.h"

BDBG_MODULE(tr69c);

static b_tr69c_server_t g_tr69c_server = NULL;

#undef RECV_TIMEOUT
#ifdef RECV_TIMEOUT
static int recv_timeout (int socket_fd, void *recv_data, int recv_data_len, int flags, struct timeval *timeout)
{
    fd_set set;
    int rv;
    int bytes_read = -1;

    FD_ZERO(&set);
    FD_SET(socket_fd, &set);

    rv = select(socket_fd + 1, &set, NULL, NULL, timeout);
    if (rv == -1)
    {
        BDBG_WRN(("[Server] select() failed"));
    }
    else if (rv == 0)
    {
        BDBG_WRN(("[Server] recv() timeout"));
    }
    else
    {
        bytes_read = recv(socket_fd, recv_data, recv_data_len, flags);
    }

    return bytes_read;
}
#endif

static void get_client_process_arg_list (char *pid, char *arglist)
{
    int fd;
    char filename[64] = {'\0'};
    char buf[MAX_LEN_ARG] = {'\0'};
    size_t length;
    char *next_arg;

    snprintf(filename, sizeof (filename), "/proc/%s/cmdline", pid);

    fd = open(filename, O_RDONLY);
    length = read(fd, buf, sizeof(buf));
    close(fd);

    buf[length] = '\0';

    next_arg = buf;
    while (next_arg < buf + length)
    {
        strcat(arglist, next_arg);
        next_arg += strlen(next_arg) + 1;
        if (next_arg) strcat(arglist, " ");
    }
    return;
}

static void *connection_handler(void *context)
{
    struct b_tr69c_client *curr = (struct b_tr69c_client *)context;
    int bytes_read, i;
    char buffer[MAX_LEN_STR] = {'\0'};
    char *str, *token, *saveptr;

#ifndef RECV_TIMEOUT
    bytes_read = recv(curr->connection_fd, buffer, MAX_LEN_STR, 0);
#else
    struct timeval timeout = { 10, 0 };
    bytes_read = recv_timeout(curr->connection_fd, buffer, MAX_LEN_STR, 0, &timeout);
#endif
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        for (i = 0, str = buffer; ;i++, str = NULL)
        {
			token = strtok_r(str, "-", &saveptr);
			if (i == 0)
				BKNI_Snprintf(curr->name, strlen(token)+1, "%s", token);
			if (i == 1)
				BKNI_Snprintf(curr->pid, strlen(token)+1, "%s", token);
            if (token == NULL)
                break;
        }
        BDBG_MSG(("[Server] connected client: %s(%s)", curr->name, curr->pid));

        get_client_process_arg_list(curr->pid, curr->arglist);
        BDBG_MSG(("[Server] connected process argument list: %s", curr->arglist));
    }
    else
    {
        if (bytes_read < 0)
        {
            BDBG_MSG(("[Server] recv() failed"));
        }
        else
        {
            BDBG_MSG(("[Server] connection closed"));
        }
    }

    sprintf(buffer, "connected. tr69c <-> %s(%s)", curr->name, curr->pid);
    if (send(curr->connection_fd, buffer, strlen(buffer), 0) == -1)
    {
        BDBG_MSG(("[Server] send() failed"));
    }

    while (1)
    {
        BKNI_WaitForEvent(curr->request_event, BKNI_INFINITE);
        BKNI_AcquireMutex(curr->connection_mutex);
        if (send(curr->connection_fd, curr->send_data, curr->send_data_len, 0) == -1)
        {
            BDBG_MSG(("[Server] send() failed"));
            BKNI_ReleaseMutex(curr->connection_mutex);
            break;
        }

        BDBG_MSG(("[Server] send data type: %d", ((struct b_tr69c_data *)curr->send_data)->type));
        switch (((struct b_tr69c_data *)curr->send_data)->type)
        {
            case b_tr69c_type_get_playback_ip_status:
            case b_tr69c_type_get_playback_status:
            case b_tr69c_type_get_video_decoder_status:
            case b_tr69c_type_get_video_decoder_start_settings:
            case b_tr69c_type_get_audio_decoder_status:
            case b_tr69c_type_get_audio_decoder_settings:
#ifndef RECV_TIMEOUT
                bytes_read = recv(curr->connection_fd, curr->recv_data, curr->recv_data_len, 0);
#else
                bytes_read = recv_timeout(curr->connection_fd, curr->recv_data, curr->recv_data_len, 0, &timeout);
#endif
                if (bytes_read > 0)
                {
                    BDBG_MSG(("[Server] received: %d bytes (%d)", bytes_read, curr->recv_data_len));
                    BKNI_SetEvent(curr->response_event);
                }
                else
                {
                    if (bytes_read < 0)
                    {
                        BDBG_MSG(("[Server] recv() failed"));
                    }
                    else
                    {
                        BDBG_MSG(("[Server] connection closed"));
                    }
                }
                break;
            case b_tr69c_type_set_video_decoder_mute:
            case b_tr69c_type_set_audio_decoder_mute:
                memset(buffer, 0, MAX_LEN_STR);
#ifndef RECV_TIMEOUT
                bytes_read = recv(curr->connection_fd, buffer, MAX_LEN_STR, 0);
#else
                bytes_read = recv_timeout(curr->connection_fd, buffer, MAX_LEN_STR, 0, &timeout);
#endif
                if (bytes_read > 0)
                {
                    BDBG_MSG(("[Server] set to client: %s. %d bytes (%d)", buffer, bytes_read, sizeof(buffer)));
                    BKNI_SetEvent(curr->response_event);
                }
                else
                {
                    if (bytes_read < 0)
                    {
                        BDBG_MSG(("[Server] recv() failed"));
                    }
                    else
                    {
                        BDBG_MSG(("[Server] connection closed"));
                    }
                }
                break;
            default:
                break;
        }
        BKNI_ReleaseMutex(curr->connection_mutex);
    }

    return NULL;
}

static void *connection_listener(void *context)
{
    NEXUS_Error rc = 0;
    b_tr69c_server_t tr69c_server = (b_tr69c_server_t)context;
    int connection_fd;
    socklen_t address_length;
    struct b_tr69c_client *curr;

    while (1)
    {
        if ((connection_fd = accept(tr69c_server->socket_fd, (struct sockaddr *) &tr69c_server->address, &address_length)) > -1)
        {
            tr69c_server->num_client += 1;
            tr69c_server->client_registry = realloc(tr69c_server->client_registry, sizeof(struct b_tr69c_client_registry));
            if (!tr69c_server->client_registry)
            {
                return NULL;
            }
            curr = malloc(sizeof(*curr));
            if (!curr)
            {
                return NULL;
            }
            memset(curr, 0, sizeof(*curr));
            (tr69c_server->client_registry + tr69c_server->num_client - 1)->client = curr;
            BKNI_CreateEvent(&curr->request_event);
            BKNI_CreateEvent(&curr->response_event);
            BKNI_CreateMutex(&curr->connection_mutex);
            curr->connection_fd = connection_fd;
            rc = pthread_create(&curr->connection_id, NULL, connection_handler, (void *)curr);
            BDBG_ASSERT(!rc);
        }
        else
        {
            BDBG_MSG(("[Server] accept() failed"));
        }
    }

    return NULL;
}

b_tr69c_server_t b_tr69c_server_init(void)
{
    NEXUS_Error rc = 0;
	b_tr69c_server_t tr69c_server;
	int optVal = 1;
	socklen_t optLen = sizeof(optVal);

	tr69c_server = malloc(sizeof(*tr69c_server));
	if (!tr69c_server)
	{
		return NULL;
	}
    memset(tr69c_server, 0, sizeof(*tr69c_server));

    if ((tr69c_server->socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        BDBG_MSG(("[Server] socket() failed"));
        free(tr69c_server);
		return NULL;
    }

	if (setsockopt(tr69c_server->socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&optVal, optLen) == -1)
	{
        BDBG_MSG(("[Server] setsockopt error"));
        close(tr69c_server->socket_fd);
        free(tr69c_server);
		return NULL;
	}

    unlink("/tmp/tr69c_socket");

    memset(&tr69c_server->address, 0, sizeof(struct sockaddr_un));
    tr69c_server->address.sun_family = AF_UNIX;
    sprintf(tr69c_server->address.sun_path, "/tmp/tr69c_socket");

    if (bind(tr69c_server->socket_fd, (struct sockaddr *) &tr69c_server->address, sizeof(struct sockaddr_un)) != 0)
    {
        BDBG_MSG(("[Server] bind() failed"));
        close(tr69c_server->socket_fd);
        free(tr69c_server);
		return NULL;
    }

    if (listen(tr69c_server->socket_fd, 5) != 0)
    {
        BDBG_MSG(("[Server] listen() failed"));
        close(tr69c_server->socket_fd);
        free(tr69c_server);
		return NULL;
    }

	rc = pthread_create(&tr69c_server->listener_id, NULL, connection_listener, (void *)tr69c_server);
	BDBG_ASSERT(!rc);

    g_tr69c_server = tr69c_server;
	return tr69c_server;
}

int b_tr69c_server_get_client(b_tr69c_client_t curr, void *send_data, int send_data_len, void *recv_data, int recv_data_len)
{
    NEXUS_Error rc = 0;
    BDBG_ASSERT(curr != NULL);
    BDBG_ASSERT(send_data != NULL);
    BDBG_ASSERT(send_data_len != 0);
    BDBG_ASSERT(recv_data != NULL);
    BDBG_ASSERT(recv_data_len != 0);

    curr->send_data = send_data;
    curr->send_data_len = send_data_len;
    curr->recv_data = recv_data;
    curr->recv_data_len = recv_data_len;
    BKNI_SetEvent(curr->request_event);
    rc = BKNI_WaitForEvent(curr->response_event, BKNI_INFINITE);
    if (rc != BERR_SUCCESS) return -1;
    else return 0;
}

int b_tr69c_server_set_client(b_tr69c_client_t curr, void *send_data, int send_data_len)
{
    NEXUS_Error rc = 0;
    BDBG_ASSERT(curr != NULL);
    BDBG_ASSERT(send_data != NULL);
    BDBG_ASSERT(send_data_len != 0);

    curr->send_data = send_data;
    curr->send_data_len = send_data_len;
    curr->recv_data = NULL;
    curr->recv_data_len = 0;
    BKNI_SetEvent(curr->request_event);
    rc = BKNI_WaitForEvent(curr->response_event, BKNI_INFINITE);
    if (rc != BERR_SUCCESS) return -1;
    else return 0;
}

int b_tr69c_get_client_registry(b_tr69c_client_registry_t *registry)
{
    if (g_tr69c_server && g_tr69c_server->num_client !=0)
    {
        *registry = g_tr69c_server->client_registry;
		return BERR_SUCCESS;
    }
	else
	{
		BDBG_ERR(("Can't get client registry\n"));
		return -1;
	}
}

void b_tr69c_get_client_count(int *count)
{
    if (g_tr69c_server)
    {
        *count = g_tr69c_server->num_client;
    }
}

void b_tr69c_server_uninit(
    b_tr69c_server_t tr69c_server
    )
{
    int i;

    if (tr69c_server->client_registry)
    {
        for (i = 0; i < tr69c_server->num_client; i++)
        {
            if ((tr69c_server->client_registry + i)->client)
            {
                pthread_join((tr69c_server->client_registry + i)->client->connection_id, NULL);
                close((tr69c_server->client_registry + i)->client->connection_fd);
                BKNI_DestroyEvent((tr69c_server->client_registry + i)->client->request_event);
                BKNI_DestroyEvent((tr69c_server->client_registry + i)->client->response_event);
                BKNI_DestroyMutex((tr69c_server->client_registry + i)->client->connection_mutex);
            }
        }
        free(tr69c_server->client_registry);
    }

    pthread_join(tr69c_server->listener_id, NULL);
    close(tr69c_server->socket_fd);
    unlink("/tmp/tr69c_socket");

	if (tr69c_server)
	{
		free(tr69c_server);
	}
    g_tr69c_server = NULL;

    return;
}
