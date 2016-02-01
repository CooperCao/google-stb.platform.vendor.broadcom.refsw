/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "util.h"


#define MAX_MOUNTS 16

static struct {
    struct {
        char device[256];
        char mountpoint[256];
    } mounts[MAX_MOUNTS];
    unsigned total_mounts;
} g_mount_state;


#define SOPASS_MAX	6

struct ethtool_wolinfo {
	unsigned int	cmd;
	unsigned int	supported;
	unsigned int	wolopts;
    unsigned char   sopass[SOPASS_MAX]; /* SecureOn(tm) password */
};

#ifndef SIOCETHTOOL
#define SIOCETHTOOL     0x8946
#endif

#define ETHTOOL_GWOL		0x00000005 /* Get wake-on-lan options. */
#define ETHTOOL_SWOL		0x00000006 /* Set wake-on-lan options. */


struct if_info eth_info, moca_info;
unsigned mocaVer = 0;

int umount_all(void)
{
    FILE *f;

    f = fopen("/proc/mounts", "r+");
    if (!f) return -1;
    while (!feof(f) && g_mount_state.total_mounts < MAX_MOUNTS) {
        char buf[256];
        char *c1, *c2;
        if (fgets(buf, 256, f)) {
            if (strstr(buf, "/dev/sd") == buf || strstr(buf, " nfs ") || strstr(buf, "/dev/mmcblk") ) {
                c1 = strchr(buf, ' ');
                if (!c1) continue;

                *c1 = 0;
                strcpy(g_mount_state.mounts[g_mount_state.total_mounts].device, buf);
                c1++;
                c2 = strchr(c1, ' ');
                if (!c2) continue;

                *c2 = 0;
                strcpy(g_mount_state.mounts[g_mount_state.total_mounts].mountpoint, c1);

                printf("found mount point %d: %s %s\n", g_mount_state.total_mounts, g_mount_state.mounts[g_mount_state.total_mounts].device, g_mount_state.mounts[g_mount_state.total_mounts].mountpoint);

                snprintf(buf, 256, "umount %s", g_mount_state.mounts[g_mount_state.total_mounts].mountpoint);
                if (!system(buf)) {
                    g_mount_state.total_mounts++;
                }
            }
        }
    }
    fclose(f);

    return 0;
}

int wait_for_device( char *device_path, unsigned long timeout_in_ms )
{
    int     rc = 0;

    printf("Wait for device %s\n", device_path);

    if (index( device_path, ':') == NULL   ) {  /* If device has ":", then it's an NFS path... skip it. */
        unsigned long   ms;
        unsigned long   ms_interval = 200;
        unsigned long   ms_max = timeout_in_ms;
        int             rc;
        struct stat     statbuf;

        for (ms=0 ; ms<ms_max ; ms+=ms_interval) {
            rc = stat(device_path, &statbuf);
            if (rc == 0) break;
            if (ms == 0) printf("Waiting for device %s to exist...\n", device_path);
            usleep(ms_interval*1000);
        }
        if (ms >= ms_max) {
            printf("Device %s not found after %lu ms\n", device_path, ms );
            return 1;   /* return error code */
        }
        else if (ms > 0) {
            printf("Device %s appeared after %lu ms\n", device_path, ms );
        }
    }

    return rc;
}

int wait_for_all_devices(void)
{
    int rc;
    unsigned i;

    for (i=0;i<g_mount_state.total_mounts;i++) {
        rc = wait_for_device( g_mount_state.mounts[i].device, 15000 );
        if (rc ) {    /* Device isn't there or not ready... */
            printf("Can't mount device %s\n", g_mount_state.mounts[i].device );
            continue;
        }
    }

    return rc;
}

int mount_all(void)
{
    unsigned i, total_mounts=g_mount_state.total_mounts;

    for (i=0;i<total_mounts;i++) {
        char buf[256];
        int rc;

        rc = wait_for_device( g_mount_state.mounts[i].device, 15000 );
        if (rc ) {    /* Device isn't there or not ready... */
            printf("Can't mount device %s\n", g_mount_state.mounts[i].device );
            continue;
        }

        snprintf(buf, 256, "mount %s %s", g_mount_state.mounts[i].device, g_mount_state.mounts[i].mountpoint);
        printf("%s\n", buf);
        system(buf);
        g_mount_state.total_mounts--;
    }

    return 0;
}

void getWol(char *devname, unsigned int *wolopts)
{
    struct ifreq ifr;
	int fd, err;
    struct ethtool_wolinfo wol;

    /* Setup our control structures. */
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, devname);

    /* Open control socket. */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot get control socket");
		return;
	}

    wol.cmd = ETHTOOL_GWOL;
    ifr.ifr_data = (caddr_t)&wol;
    err = ioctl(fd, SIOCETHTOOL, &ifr);
    if (err < 0)
        perror("Cannot get wake-on-lan settings");

    *wolopts = wol.supported;

    close(fd);
}

void setWol(char *devname, unsigned int wolopts)
{
    struct ifreq ifr;
	int fd, err;
    struct ethtool_wolinfo wol;

	/* Setup our control structures. */
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, devname);

	/* Open control socket. */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot get control socket");
		return;
	}

    wol.wolopts = wolopts;
    wol.cmd = ETHTOOL_SWOL;
    ifr.ifr_data = (caddr_t)&wol;
    err = ioctl(fd, SIOCETHTOOL, &ifr);
    if (err < 0)
        perror("Cannot set wake-on-lan settings");

    close(fd);
}

unsigned getMocaVer(void)
{
    unsigned mocaVer = 0;

    if(!access("/bin/mocactl", F_OK))
        mocaVer = 1;
    if(!access("/bin/mocap", F_OK))
        mocaVer = 2;

    return mocaVer;
}

void getIfName(void)
{
    struct ifaddrs *ifaddr, *ifa;
    struct ifreq s;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        struct if_info *info = NULL;

        if (ifa->ifa_addr == NULL)
            continue;

        if(!strncmp(ifa->ifa_name, "gphy", 4) ||
           !strncmp(ifa->ifa_name, "eth0", 4))
            info = &eth_info;
        else if(!strncmp(ifa->ifa_name, "moca", 4) ||
                !strncmp(ifa->ifa_name, "eth1", 4))
            info = &moca_info;

        if(info) {
            snprintf(info->if_name, IFNAMSIZ, ifa->ifa_name);
            snprintf(s.ifr_name, IFNAMSIZ, ifa->ifa_name);
            if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
                snprintf(info->hw_addr, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
                        (unsigned char) s.ifr_addr.sa_data[0],
                        (unsigned char) s.ifr_addr.sa_data[1],
                        (unsigned char) s.ifr_addr.sa_data[2],
                        (unsigned char) s.ifr_addr.sa_data[3],
                        (unsigned char) s.ifr_addr.sa_data[4],
                        (unsigned char) s.ifr_addr.sa_data[5]);
            }

            if (0 == ioctl(fd, SIOCGIFADDR, &s)) {
                snprintf(info->ip_addr, 32, "%s", inet_ntoa(((struct sockaddr_in *)&s.ifr_addr)->sin_addr));
            }

            info->exists = 1;
        }
    }
    freeifaddrs(ifaddr);
    close(fd);
}
#include <fcntl.h>
int isIpReachable(char *addr, unsigned port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_port   = htons(port);
    inet_pton(AF_INET, addr, &sin.sin_addr);

    if (connect(sockfd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
    {
        printf("Error connecting %s:%d : (%s)\n", addr, port, strerror(errno));
        return 0;
    } else {
        return 1;
    }
}
