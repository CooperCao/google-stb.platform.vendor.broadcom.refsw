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
#include <libgen.h>
#include <stdlib.h>
#include <pthread.h>
#include "bstd.h"
#include "tr69clib.h"
#include "tr69clib_priv.h"

BDBG_MODULE(tr69c);

struct b_tr69c
{
	int socket_fd;
	pthread_t thread_id;
	tr69c_callback callback;
	void *callback_context;
};

static void b_tr69c_get_nxclient_identity(char *identity)
{
	char name[MAX_LEN_STR] = {'\0'};
    char pid[MAX_LEN_STR] = {'\0'};

    readlink("/proc/self/exe", name, sizeof(name));
    readlink("/proc/self", pid, sizeof(pid));
    sprintf(identity, "%s-%s", basename(name), pid);

	return;
}

static void *b_tr69c_monitor(void *context)
{
	b_tr69c_t tr69c;
	int optVal = 1;
	socklen_t optLen = sizeof(optVal);
	int socket_fd;
	tr69c_callback callback;
	void *callback_context;
    struct sockaddr_un address;
	struct b_tr69c_data tr69c_data;
    void *recv_data = (void *)&tr69c_data;
	int recv_data_len = sizeof(struct b_tr69c_data);
    void *send_data;
	int send_data_len;
	int bytes_read;
    char identity[MAX_LEN_STR] = {'\0'};
    char buffer[MAX_LEN_STR] = {'\0'};
    int rc = 0;

    tr69c = (b_tr69c_t)context;
	callback = tr69c->callback;
	callback_context = tr69c->callback_context;

    do {
        if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        {
            BDBG_ERR(("[Client] socket creation error"));
            return NULL;
        }
        tr69c->socket_fd = socket_fd;

        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&optVal, optLen) == -1)
        {
            BDBG_ERR(("[Client] setsockopt error"));
            return NULL;
        }

        memset(&address, 0, sizeof(struct sockaddr_un));
        address.sun_family = AF_UNIX;
        sprintf(address.sun_path, "/tmp/tr69c_socket");

        do {
			unsigned i;
            if (connect(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0)
            {
                BDBG_MSG(("[Client] connect() failed"));
            }
            else
            {
                BDBG_MSG(("[Client] connected"));
                break;
            }
			for (i=0; i < 50 && (tr69c->socket_fd != -1); i++)
				BKNI_Sleep(100);
        }while (tr69c->socket_fd != -1);

		if (tr69c->socket_fd == -1)
			break;

        b_tr69c_get_nxclient_identity(identity);
        if (send(socket_fd, identity, strlen(identity), 0) == -1)
        {
            BDBG_MSG(("[Client] send failed"));
        }

        bytes_read = recv(socket_fd, buffer, MAX_LEN_STR, 0);
        if (bytes_read > 0)
        {
            BDBG_MSG(("[Client] received: %s, %d bytes read", buffer, bytes_read));
        }
        else
        {
            if (bytes_read < 0)
            {
                BDBG_MSG(("[Client] recv failed"));
            }
            else
            {
                BDBG_MSG(("[Client] connection closed"));
            }
        }

        while (1)
        {
            BDBG_MSG(("[Client] waiting to receive data"));
            bytes_read = recv(socket_fd, recv_data, recv_data_len, 0);
            BDBG_MSG(("[Client] received: %d bytes (%d)", bytes_read, recv_data_len));
            if (bytes_read > 0)
            {
                BDBG_MSG(("[Client] receive data type: %d", tr69c_data.type));

                switch (tr69c_data.type)
                {
                    case b_tr69c_type_get_playback_ip_status:
                    case b_tr69c_type_get_playback_status:
                    case b_tr69c_type_get_video_decoder_status:
                    case b_tr69c_type_get_video_decoder_start_settings:
                    case b_tr69c_type_get_audio_decoder_status:
                    case b_tr69c_type_get_audio_decoder_settings:
                        memset(&tr69c_data.info, 0, sizeof(union b_tr69c_info));
                        break;
                    case b_tr69c_type_set_video_decoder_mute:
                        break;
                    default:
                        BDBG_WRN(("[Client] data type not supported"));
                }

                rc = callback(callback_context, tr69c_data.type, &tr69c_data.info);

                switch (tr69c_data.type)
                {
                    case b_tr69c_type_get_playback_ip_status:
                    case b_tr69c_type_get_playback_status:
                    case b_tr69c_type_get_video_decoder_status:
                    case b_tr69c_type_get_video_decoder_start_settings:
                    case b_tr69c_type_get_audio_decoder_status:
                    case b_tr69c_type_get_audio_decoder_settings:
                        send_data = (void *)&tr69c_data.info;
                        send_data_len = sizeof(union b_tr69c_info);
                        break;
                    case b_tr69c_type_set_video_decoder_mute:
                    case b_tr69c_type_set_audio_decoder_mute:
                        if (tr69c_data.type == b_tr69c_type_set_video_decoder_mute)
                        {
                            BDBG_MSG(("[Client] set data object: Video Decoder Mute: %s", tr69c_data.info.video_decoder_mute ? "yes" : "no"));
                        }
                        else if (tr69c_data.type == b_tr69c_type_set_audio_decoder_mute)
                        {
                            BDBG_MSG(("[Client] set data object: Audio Decoder Mute: %s", tr69c_data.info.audio_decoder_mute ? "yes" : "no"));
                        }
                        memset(buffer, 0, MAX_LEN_STR);
                        sprintf(buffer, (rc == 0) ? "success" : "fail");
                        send_data = (void *)buffer;
                        send_data_len = strlen(buffer);
                        break;
                    default:
                        BDBG_WRN(("[Client] data type not supported"));
                }
                if (send(socket_fd, send_data, send_data_len, 0) == -1)
                {
                    BDBG_MSG(("[Client] send failed"));
                }
                else
                {
                    BDBG_MSG(("[Client] sent: %d bytes", send_data_len));
                }
            }
            else
            {
                if (bytes_read < 0)
                {
                    BDBG_MSG(("[Client] recv failed"));
                }
                else
                {
                    BDBG_MSG(("[Client] connection closed"));
                    close(socket_fd);
                    tr69c->socket_fd = -1;
                    break;
                }
            }
        }
    } while (tr69c->socket_fd != -1);

	return NULL;
}

b_tr69c_t b_tr69c_init(
    tr69c_callback callback, /* universal callback to get status/settings and set settings */
    void *context /* pass into callback for the caller's use */
    )
{
    NEXUS_Error rc = 0;
	b_tr69c_t tr69c;

	tr69c = malloc(sizeof(*tr69c));
	if (!tr69c)
	{
		return NULL;
	}
    memset(tr69c, 0, sizeof(*tr69c));

	tr69c->callback = callback;
	tr69c->callback_context = context;

	rc = pthread_create(&tr69c->thread_id, NULL, b_tr69c_monitor, (void *)tr69c);
	BDBG_ASSERT(!rc);

	return tr69c;
}

void b_tr69c_uninit(
    b_tr69c_t tr69c
    )
{
	int socket_fd = tr69c->socket_fd;
	tr69c->socket_fd = -1;
	close(socket_fd);
    pthread_join(tr69c->thread_id, NULL);
	free(tr69c);
}
