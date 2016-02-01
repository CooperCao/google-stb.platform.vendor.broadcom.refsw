/******************************************************************************
 *    (c)2015 Broadcom Corporation
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
    if(rc!=0) {
        close(fd);
        BERR_TRACE(errno);
        return -1;
    }
    /* allow non-root access */
    chmod(sock_addr.sun_path, 0666);
    return fd;
}
