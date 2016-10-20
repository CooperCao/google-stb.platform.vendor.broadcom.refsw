/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,37)
#include <ifaddrs.h>
#endif
#if 0
typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
typedef u_int64_t __u64;
typedef u_int32_t __u32;
typedef u_int16_t __u16;
typedef u_int8_t __u8;
#endif
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include <typedefs.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include "wlu.h"
#define DEV_TYPE_LEN 3 /* length for devtype 'wl'/'et' */

extern void wl_find(char *args[], u_int32_t size);
extern int bwl_wl_check(void *bwl);
const char *wlu_av0;

static void
syserr(char *s)
{
    fprintf(stderr, "%s: ", wlu_av0);
    perror(s);
    exit(errno);
}

static int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
    struct ifreq *ifr = (struct ifreq *) wl;
    wl_ioctl_t ioc;
    int ret = 0;
    int s;

    /* open socket to kernel */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        syserr("socket");

    /* do it */
    ioc.cmd = cmd;
    ioc.buf = buf;
    ioc.len = len;
    ioc.set = set;
    ifr->ifr_data = (caddr_t) &ioc;
    if ((ret = ioctl(s, SIOCDEVPRIVATE, ifr)) < 0) {
        if (cmd != WLC_GET_MAGIC) {
#if defined(DHD_WIFI_DRIVER) ||  defined(BISON_WIFI_DRIVER) ||defined(EAGLE_WIFI_DRIVER)
            ret = BCME_IOCTL_ERROR;
#else
            ret = IOCTL_ERROR;
#endif
        }
    }

    /* cleanup */
    close(s);
    return ret;
}

static int
wl_get_dev_type(char *name, void *buf, int len)
{
    int s;
    int ret;
    struct ifreq ifr;
    struct ethtool_drvinfo info;

    /* open socket to kernel */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        syserr("socket");

    /* get device type */
    memset(&info, 0, sizeof(info));
    info.cmd = ETHTOOL_GDRVINFO;
    ifr.ifr_data = (caddr_t)&info;
    strncpy(ifr.ifr_name, name, IFNAMSIZ);
    if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {

        /* print a good diagnostic if not superuser */
        if (errno == EPERM)
            syserr("wl_get_dev_type");

        *(char *)buf = '\0';
    }
    else
        strncpy(buf, info.driver, len);

    close(s);
    return ret;
}

int
wl_get(void *wl, int cmd, void *buf, int len)
{
    return wl_ioctl(wl, cmd, buf, len, FALSE);
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
    return wl_ioctl(wl, cmd, buf, len, TRUE);
}

void
wl_find(char *args[], u_int32_t size)
{
    u_int32_t count = 0;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,37)
    char dev_type[DEV_TYPE_LEN];
    struct ifaddrs *ifaddr, *ifa;


    if (getifaddrs(&ifaddr) < 0) {
        syserr("getifaddrs");
        return;
    }

    for (ifa = ifaddr; (ifa != NULL) && (count < size); ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        if (wl_get_dev_type(ifa->ifa_name, dev_type, DEV_TYPE_LEN) >= 0 && !strncmp(dev_type, "wl", 2)) {
            args[count++] = strdup(ifa->ifa_name);
        }
    }

    freeifaddrs(ifaddr);
#else
    char proc_net_dev[] = "/proc/net/dev";
    FILE *fp;
    char buf[1000], *c, *name;
    char dev_type[DEV_TYPE_LEN];
    struct ifreq ifr;
    (void)size;

    if (!(fp = fopen(proc_net_dev, "r")))
        return;

    /* eat first two lines */
    if (!fgets(buf, sizeof(buf), fp) ||
        !fgets(buf, sizeof(buf), fp)) {
        fclose(fp);
        return;
    }

    while (fgets(buf, sizeof(buf), fp)) {
        c = buf;
        while (isspace(*c))
            c++;
        if (!(name = strsep(&c, ":")))
            continue;
        strncpy(ifr.ifr_name, name, IFNAMSIZ);
        if (wl_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 && !strncmp(dev_type, "wl", 2))
        {
            args[count++] = strdup(name);
        }
    }

    fclose(fp);
#endif
}
