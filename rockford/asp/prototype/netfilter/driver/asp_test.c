/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "asp_driver.h"

char Usage[] = "\
Usage: \n\
    asp_test -a [-options] -- add filter \n\
    asp_test -d [-options] -- delete filter \n\
    asp_test -g -i #       -- get filter\n\
    asp_test -r            -- reset  \n\
Common options:\n\
    -i ##  -- asp channel index\n\
    -u     -- use UDP instead of TCP\n\
    -s ##  -- remote address      \n\
    -h ##  -- my address \n\
    -m ##  --   my port port number \n\
    -p ##  -- remote port number \n\
    -x ##  -- action 0-forward packets to n/w stack\n\
                   1-drop packet\n\
                   2-forward to asp\n\
                   3-forward to app if listening \n\
";

extern char *optarg;

int main(int argc ,char *argv[])
{
    int c=0;
    int fd=0;
    int command=0;
    int ret=0;
    struct asp_ioc_params params;
    memset(&params,0,sizeof(struct asp_ioc_params));

    if (argc < 2) goto usage;

    while ((c = getopt(argc, argv, "adgrui:s:h:p:m:x:")) != -1)
    {
        switch (c)
        {

        case 'a':
            command = ASP_DEV_IOC_ADDFILTER;
            printf("adding rule,");
            break;
        case 'd':
            command = ASP_DEV_IOC_ADDFILTER;
            printf("deleting rule rule,");
        case 'g':
            command = ASP_DEV_IOC_GETFILTER;
            break;
        case 'r':
            command = ASP_DEV_IOC_RESET;
            break;
        case 'i':
            params.asp_ch = atoi(optarg);
            if (params.asp_ch <0 && params.asp_ch > 31)
            {
                printf("channel index i should be between 0-31\n");
                params.asp_ch =0;
            }
            printf("for channel %d\n",params.asp_ch);
            break;
        case 'u':
            params.protocol = 1;
            printf("protocol udp\n");
            break;
        case 's':
            params.src_ip = inet_addr(optarg);
            printf("pkt src ip %s[%llx],",optarg,params.src_ip);
            break;
        case 'h':
            params.dst_ip = inet_addr(optarg);
            printf("pkt dst ip %s[%llx]\n",optarg,params.dst_ip);
            break;
        case 'p':
            params.src_port = atoi(optarg);
            printf("pkt src port %d[%x],",params.src_port,params.src_port);
            break;
        case 'm':
            params.dst_port = atoi(optarg);
            printf("pkt dst port %d[%x],",params.dst_port,params.dst_port);
            break;
        case 'x':
            params.action = atoi(optarg);
            printf("\naction %d\n",params.action);
            break;
        default:
            goto usage;
        }
    }
    fd = open("/dev/brcm_asp", O_RDWR);
    printf("device opened %s %d\n",ASP_DEV_NAME,fd);
    if (fd < 0)
    {
        printf("make sure asp_driver is installed\n");
    }

    ret= ioctl(fd,command,&params);
    if (ret != 0)
    {
        printf("ioctl failed (%d)\n", ret);
        exit(ret);
    }
    close(fd);
    exit(0);
    usage:
    fprintf(stderr,Usage);
    exit(1);
}
