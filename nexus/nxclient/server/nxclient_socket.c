/******************************************************************************
 * Copyright (C) 2015-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 *****************************************************************************/
#include "bstd.h"
#include "nexus_types.h"
#include "nexus_base_os.h"
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#if !defined(NEXUS_HAS_SOCKET_DRIVER)
static void nxclient_p_print_devicenode(char *path, unsigned pathlen)
{
    const char *str;
    str = NEXUS_GetEnv("nxclient_ipc_node");
    if (str) {
        snprintf(path, pathlen, "%s", str);
    }
    else {
        str = NEXUS_GetEnv("nexus_ipc_dir");
        if (str) {
            snprintf(path, pathlen, "%s/%s", str, "nxserver_ipc");
        }
        else {
            snprintf(path, pathlen, "/tmp/nxserver_ipc");
        }
    }
}

int b_nxclient_client_connect(void)
{
    int fd;
    struct sockaddr_un sock_addr;
    int rc;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd < 0) {
        return fd;
    }
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    nxclient_p_print_devicenode(sock_addr.sun_path, sizeof(sock_addr.sun_path));
    rc = connect(fd, (struct sockaddr *)&sock_addr, strlen(sock_addr.sun_path)+sizeof(sock_addr.sun_family));
    if(rc<0) {
        close(fd);
        return rc;
    }
    return fd;
}

int b_nxclient_socket_listen(void)
{
    int fd;
    struct sockaddr_un sock_addr;
    int rc;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(fd < 0) {
        BERR_TRACE(errno);
        return fd;
    }
    BKNI_Memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    nxclient_p_print_devicenode(sock_addr.sun_path, sizeof(sock_addr.sun_path));
    unlink(sock_addr.sun_path);
    fcntl(fd, F_SETFL, O_NONBLOCK | FD_CLOEXEC);
    rc = bind(fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr.sun_family)+strlen(sock_addr.sun_path));
    if(rc!=0) { (void)BERR_TRACE(errno); goto err_bind;}

    /* allow non-root access */
    chmod(sock_addr.sun_path, 0666);
    rc = listen(fd, 10);
    if(rc!=0) { (void)BERR_TRACE(errno); goto err_listen; }
    return fd;

err_listen:
err_bind:
    perror("");
    close(fd);
    return -1;
}

int b_nxclient_get_client_pid(int client_fd, unsigned *pid)
{
    int rc;
    struct ucred credentials;
    socklen_t ucred_length = sizeof(struct ucred);

    rc = getsockopt(client_fd, SOL_SOCKET, SO_PEERCRED, &credentials, &ucred_length);
    if (rc) {return BERR_TRACE(rc);}
    *pid = credentials.pid;
    return 0;
}

int b_nxclient_socket_accept(int listen_fd)
{
    int fd;

    fd = accept(listen_fd, NULL, NULL);
    return fd;
}
#else /* #if !defined(NEXUS_HAS_SOCKET_DRIVER) */

BDBG_MODULE(nxclient_socket);

#include <stdlib.h>
#include <sys/ioctl.h>
#include "nexus_platform_socket_ioctl.h"

#define BDBG_MSG_TRACE(x) BDBG_MSG(x)

static const char *b_nxclient_socket_devName(void)
{
    const char *devName = getenv("NEXUS_DEVICE_NODE");
    if (!devName) {
        devName = "/dev/nexus";
    }
    return devName;
}

static int b_nxclient_socket_open(void)
{
    int fd;
    const char *devName =  b_nxclient_socket_devName();
    fd = open(devName, O_RDWR);
    if(fd<0) {
        perror(devName);
    }
    return fd;
}


int b_nxclient_socket_listen(void)
{
    int rc;
    int fd = b_nxclient_socket_open();
    if(fd<0) { (void)BERR_TRACE(errno);  goto err_open; }

    rc = ioctl(fd, NEXUS_PLATFORM_SOCKET_LISTEN, NULL);
    BDBG_MSG_TRACE(("Listen -> %d(%d)", fd, rc));
    if(rc!=0) { (void)BERR_TRACE(errno);  goto err_ioctl; }

    return fd;

err_ioctl:
    close(fd);
err_open:
    perror("");
    return -1;
}

int b_nxclient_client_connect(void)
{
    int rc;
    int fd = b_nxclient_socket_open();
    if(fd<0) { (void)BERR_TRACE(errno);  goto err_open; }


    rc = ioctl(fd, NEXUS_PLATFORM_SOCKET_CONNECT, NULL);
    BDBG_MSG_TRACE(("Connect -> %d(%d)", fd, rc));
    if(rc!=0) { (void)BERR_TRACE(errno);  goto err_ioctl; }

    return fd;

err_ioctl:
    close(fd);
err_open:
    perror("");
    return -1;
}

int b_nxclient_socket_accept(int listen_fd)
{
    int rc;
    int fd = b_nxclient_socket_open();
    if(fd<0) { (void)BERR_TRACE(errno);  goto err_open; }


    rc = ioctl(fd, NEXUS_PLATFORM_SOCKET_ACCEPT, NULL);
    BDBG_MSG_TRACE(("accept -> %d(%d)", fd, rc));
    if(rc!=0) { (void)BERR_TRACE(errno);  goto err_ioctl; }

    BSTD_UNUSED(listen_fd);
    return fd;

err_ioctl:
    close(fd);
err_open:
    perror("");
    return -1;
}

int b_nxclient_get_client_pid(int client_fd, unsigned *pid)
{
    return ioctl(client_fd, NEXUS_PLATFORM_SOCKET_GET_PID, pid);
}


#endif /* #else #if !defined(NEXUS_HAS_SOCKET_DRIVER) */
