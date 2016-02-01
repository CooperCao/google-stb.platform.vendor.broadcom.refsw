/***************************************************************************
*     (c)2004-2012 Broadcom Corporation
*
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: Platform (private)
*    Common part of all kernel servers
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "bstd.h"
#include "server/nexus_ipc_api.h"
#include "nexus_platform_local_priv.h"
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/un.h>
#include <stdio.h>
#include <fcntl.h>

BDBG_MODULE(nexus_socket);

/* copied from nexus/lib/ipc/bipc_util.c */
ssize_t b_nexus_read(int fd, void *buf, size_t buf_size)
{
    int result;
    for(result=0;;) {
        int rc;
        if(buf_size==0) {
            break;
        }
        rc = read(fd, buf, buf_size);
        if(rc>0) {
            BDBG_ASSERT((unsigned)rc<=buf_size);
            result += rc;
/* TODO: current impl allows read of <buf_size. this works with unix domain sockets.
but for general impl, required for internet domain, we need to know expected size to read. 
ipc must be reworked so that variable size params can be loaded into fixed size header.
then read can be done in two passes: fixed size, variable size.
*/
#if 0
            if((unsigned)rc==buf_size) {
                break;
            }
            BDBG_ASSERT((unsigned)rc<=buf_size);
            buf_size -= rc;
            buf = (uint8_t *)buf + rc;
#else
            break;
#endif
        } else {
            if(rc==0) {
                result = -1;
                break;
            }
            rc = errno;
            if(rc==EINTR) {
                continue;
            } else if((rc==EAGAIN || rc==EWOULDBLOCK) && result==0) {
                break;
            } else {
                result = -rc;
                break;
            }
        }
    }
    return result;
}

/* blocking write. any non-blocking write should be handled directly with write().
returns amount written. anything other than 'size' is an error. */
ssize_t b_nexus_write(int fd, const void *buf, size_t buf_size)
{
    int result;

    for(result=0;;) {
        int rc;
        if(buf_size==0) {
            break;
        }
        rc = write(fd, buf, buf_size);
        if(rc>0) {
            BDBG_ASSERT((unsigned)rc<=buf_size);
            result += rc;
            if((unsigned)rc==buf_size) {
                break;
            }
            BDBG_ASSERT((unsigned)rc<=buf_size);
            buf_size -= rc;
            buf = (uint8_t *)buf + rc;
        } else {
            rc = errno;
            if(rc==EINTR) {
                continue;
            } else if((rc==EAGAIN || rc==EWOULDBLOCK) && result==0) {
                break;
            } else {
                result = -rc;
                break;
            }
        }
    }
    return result;
}

/* for nexus ipc socket close, always do shutdown;close */
int b_nexus_socket_close(int fd)
{
    shutdown(fd, SHUT_RDWR);
    return close(fd);
}

static const char *nexus_p_read_and_validate_nexus_ipc_dir(const char *str)
{
    static const char *whitelist[] = {"/tmp", "/shared", NULL};
    const char **whitelist_ptr;

    whitelist_ptr = whitelist;

    if (!str || *str == 0)
        goto whitelist_done;

    do {
        if (strcmp(*whitelist_ptr, str) == 0)
            break;

       whitelist_ptr++;
    } while (*whitelist_ptr != NULL);

    if (*whitelist_ptr == NULL) {
        /* No match found, use default */
        whitelist_ptr = whitelist;
    }

whitelist_done:
    return *whitelist_ptr;
}

int b_nexus_socket_connect(nexus_socket_type socket_type, unsigned index)
{
    int fd;
    struct sockaddr_un sock_addr;
    int rc;
    const char *ipc_dir;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        return BERR_TRACE(-1);
    }

    rc = fcntl(fd, F_SETFD, FD_CLOEXEC);
    if (rc == -1) {
        BDBG_MSG(("fcntl failed on connect_fd: %d %d", rc, errno));
        close(fd);
        return BERR_TRACE(-1);
    }

    ipc_dir = nexus_p_read_and_validate_nexus_ipc_dir(NEXUS_GetEnv("nexus_ipc_dir"));

    BKNI_Memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    switch (socket_type) {
    case nexus_socket_type_main:
        snprintf(sock_addr.sun_path, sizeof(sock_addr.sun_path), "%s/%s", ipc_dir, B_UNIX_MAIN_SOCKET_STR);
        break;
    case nexus_socket_type_module:
        snprintf(sock_addr.sun_path, sizeof(sock_addr.sun_path), "%s/%s%d", ipc_dir, B_UNIX_SOCKET_FORMAT_STR, index);
        break;
    case nexus_socket_type_scheduler:
        snprintf(sock_addr.sun_path, sizeof(sock_addr.sun_path), "%s/%s%d", ipc_dir, B_UNIX_SCHEDULER_SOCKET_FMT_STR, index);
        break;
    }
    rc = connect(fd, (struct sockaddr *)&sock_addr, strlen(sock_addr.sun_path)+sizeof(sock_addr.sun_family));
    if (rc == -1) {
        BDBG_ERR(("unable to connect to %s. check server console for error messages.", sock_addr.sun_path));
        close(fd);
        return -1; /* no BERR_TRACE */
    }
    return fd;
}

int b_nexus_socket_listen(nexus_socket_type socketType, unsigned index)
{
    int rc;
    int listen_fd;
    struct sockaddr_un sock_addr;
    const char *ipc_dir;

    /* set up server's main listener socket */
    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        BERR_TRACE(NEXUS_UNKNOWN);
        return -1;
    }
    rc = fcntl(listen_fd, F_SETFD, FD_CLOEXEC);
    if (rc == -1) {
        BDBG_MSG(("fcntl failed on listen_fd: %d %d", rc, errno));
    }
    ipc_dir = nexus_p_read_and_validate_nexus_ipc_dir(NEXUS_GetEnv("nexus_ipc_dir"));
    rc = mkdir(ipc_dir, 0777);
    if (rc == -1) {
        BDBG_MSG(("mkdir failed (\"%s\"): %d %d", ipc_dir, rc, errno));
    }
    rc = chmod(ipc_dir, 0777);
    if (rc == -1) {
        BDBG_MSG(("chmod failed (\"%s\"): %d %d", ipc_dir, rc, errno));
    }

    BKNI_Memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    switch (socketType) {
    case nexus_socket_type_main:
        snprintf(sock_addr.sun_path, sizeof(sock_addr.sun_path), "%s/%s", ipc_dir, B_UNIX_MAIN_SOCKET_STR);
        break;
    case nexus_socket_type_module:
        snprintf(sock_addr.sun_path, sizeof(sock_addr.sun_path), "%s/%s%d", ipc_dir, B_UNIX_SOCKET_FORMAT_STR, index);
        break;
    case nexus_socket_type_scheduler:
        snprintf(sock_addr.sun_path, sizeof(sock_addr.sun_path), "%s/%s%d", ipc_dir, B_UNIX_SCHEDULER_SOCKET_FMT_STR, index);
        break;
    }
    rc = unlink(sock_addr.sun_path);
    if (rc == -1) {
        BDBG_MSG(("unlink failed (\"%s\"): %d %d", sock_addr.sun_path, rc, errno));
    }

    rc = bind(listen_fd, (struct sockaddr *)&sock_addr, strlen(sock_addr.sun_path)+sizeof(sock_addr.sun_family));
    if (rc == -1) {
        close(listen_fd);
        BERR_TRACE(NEXUS_UNKNOWN);
        return -1;
    }

    /* allow non-root access */
    rc = chmod(sock_addr.sun_path, 0666);
    if (rc == -1) {
        BDBG_MSG(("chmod failed (\"%s\"): %d %d", sock_addr.sun_path, rc, errno));
    }

    rc = listen(listen_fd, 10);
    if (rc == -1) {
        close(listen_fd);
        BERR_TRACE(NEXUS_UNKNOWN);
        return -1;
    }

    return listen_fd;
}
