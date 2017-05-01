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
 *****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/select.h>
#include "asp_netfilter_drv.h"

void *testThread( void *data)
{
    int err;
    int fd = (int) data;
    printf("<<<<<<<<<<<<<<<<< removing flow fd=%d\n", fd);
    sleep(1);
    {
        err = ioctl(fd, ASP_CHANNEL_IOC_DEL_SOCKET_5TUPLE_INFO, 0);
        if (err != 0) perror("ioctl: ");
    }
    printf("<<<<<<<<<<<<<<<<< removed fd=%d\n", fd);
}

int main()
{
    int err;
    int fd;
    int fd2;

    /* load the driver. */
    system("rmmod asp_netfilter_drv.ko");
    sleep(1);
    system("insmod asp_netfilter_drv.ko");
    sleep(1);

    fd = open("/dev/brcm_asp", O_RDWR);
    assert(fd >= 0);

    /* invalid ioctl */
    if (1)
    {
        struct ASP_Socket5TupleInfo socket5TupleInfo;

        socket5TupleInfo.l4Protocol = 4;
        err = ioctl(fd, ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo));
        assert(err < 0 && errno == EINVAL);
        socket5TupleInfo.ipVersion = 10;
        socket5TupleInfo.l4Protocol = 1;
        err = ioctl(fd, ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo));
        assert(err < 0 && errno == EINVAL);
    }

    /* add proper socket 5-tuple entry. */
    {
        struct ASP_Socket5TupleInfo socket5TupleInfo;
        struct ASP_Socket5TupleInfo socket5TupleInfo2;
        struct in_addr srcIpAddr;
        struct in_addr dstIpAddr;

        socket5TupleInfo.dstPort = htons(10000);

        err = inet_aton("192.168.2.130", &srcIpAddr);
        assert(err > 0);
        socket5TupleInfo.srcIpAddr[0] = srcIpAddr.s_addr;

        err = inet_aton("192.168.2.11", &dstIpAddr);
        assert(err > 0);
        socket5TupleInfo.dstIpAddr[0] = dstIpAddr.s_addr;

        socket5TupleInfo.l4Protocol = ASP_ChLayer4Protocol_eUDP;
        socket5TupleInfo.ipVersion = ASP_ChLayer3IpVersion_eIpv4;
        err = ioctl(fd, ASP_CHANNEL_IOC_SET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo));
        if (err != 0) perror("ioctl: ");

        err = ioctl(fd, ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo2));
        if (err != 0) perror("ioctl: ");
        if (memcmp(&socket5TupleInfo, &socket5TupleInfo2, sizeof(socket5TupleInfo))) assert(0);

    }

    /* try a non-blocking read. */
    {
#define READ_BUF_SIZE 2048
        char buffer[READ_BUF_SIZE];
        ssize_t bytesRead;
        int flags;
        int rc;

        flags = fcntl(fd, F_GETFL);
        flags |= O_NONBLOCK;
        assert(fcntl(fd, F_SETFL, flags) == 0);

        bytesRead = read(fd, buffer, READ_BUF_SIZE);
        printf(">>>> bytesRead = %d, errno=%d\n", bytesRead, errno);
        assert(bytesRead < 0 && errno == EAGAIN);
        flags &= ~O_NONBLOCK;
        assert(fcntl(fd, F_SETFL, flags) == 0);
    }

    if (0)
    {
        pthread_t threadId;
        pthread_create(&threadId, NULL, testThread,  (void*)fd);
        printf("thread created\n");
    }

    /* select on the fd to know if there is an event. */
    {
        fd_set rfds;
        int rc;

        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        printf(">>>> issue blocking select fd=%d\n", fd);
        rc = select(fd+1, &rfds, NULL, NULL, NULL);
        if (rc < 0)
            perror("select: ");
        else
        {
            assert( rc == 1 && FD_ISSET(fd, &rfds));
            printf("fd=%d is read ready!\n");
        }
    }

    /* do a blocking read. */
    {
#define READ_BUF_SIZE 2048
        char buffer[READ_BUF_SIZE];
        ssize_t bytesRead;

        printf(">>>> issue blocking read for = %d bytes\n", READ_BUF_SIZE);
        bytesRead = read(fd, buffer, READ_BUF_SIZE);
        printf(">>>> bytesRead = %d\n", bytesRead);
        printf(MacAddrPrintFmt , MacAddrPrintArgs(buffer));
        printf("\n");
    }

    printf("Sleeping for sender to send 1 pkt.........\n");
    sleep(10);
    /* test 2nd open. */
    {
        struct ASP_Socket5TupleInfo socket5TupleInfo2;
        fd2 = open("/dev/brcm_asp", O_RDWR);
        assert(fd2 >= 0);
        printf("2nd context successfully opened\n");

        err = ioctl(fd2, ASP_CHANNEL_IOC_GET_SOCKET_5TUPLE_INFO, (unsigned long)(&socket5TupleInfo2));
        assert(err < 0 && errno == EINVAL);
    }
    printf("Sleeping .........\n");
    sleep(30);
    {
        err = ioctl(fd, ASP_CHANNEL_IOC_DEL_SOCKET_5TUPLE_INFO, 0);
        if (err != 0) perror("ioctl: ");
    }

    printf("Check that match is not printed: Sleeping .........\n");
    sleep(30);
    close(fd);
    close(fd2);
    printf("Done ...\n");
    return 0;
}
