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
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <net/if.h>
#include <pthread.h>

#include "bstd.h"
#include "mocalib.h"
#include "moca2.h"

#undef DEBUG
#ifdef DEBUG
#define DBGPRINT(X) fprintf X
#else
#define DBGPRINT(X)
#endif

struct link_state_info
{
    void *ctx;
    pthread_t id;
    unsigned int since;
};

static struct link_state_info g_link_state_info;

static unsigned int get_time_sec(void)
{
   static int error = 0;

   struct sysinfo info;

   if (!error)
   {
      error = sysinfo(&info);

      if (error)
         DBGPRINT((stdout, "WARNING: calling sysinfo, using time() as fallback\n"));
      else
         return((unsigned int)info.uptime);
   }

   if (error)
   {
      return((unsigned int)time(NULL));
   }

   return 0;
}

static void link_cb(void * userarg, uint32_t status)
{
    struct link_state_info *info = (struct link_state_info *)userarg;

    if (status == 0)
    {
        DBGPRINT((stdout, "Link is DOWN\n"));
        info->since = 0;
    }
    else
    {
        DBGPRINT((stdout, "Link is UP\n"));
        info->since = get_time_sec();
    }
}

static void *link_state_monitor_thread(void * context)
{
    struct link_state_info *info = (struct link_state_info *)context;

    if (info->ctx)
    {
        moca_register_link_up_state_cb(info->ctx, link_cb, context);
        moca_event_loop(info->ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is mocad running?\n"));
    }

    return NULL;
}

int moca_link_state_monitor_init(void)
{
    int ret = 0;
    struct moca_interface_status ifstatus;

    g_link_state_info.since = 0;
    g_link_state_info.ctx = moca_open(NULL);

    if (g_link_state_info.ctx)
    {
        ret = moca_get_interface_status(g_link_state_info.ctx, &ifstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            if (ifstatus.link_status == MOCA_LINK_UP)
            {
                g_link_state_info.since = get_time_sec();
            }
        }
    }

    ret = pthread_create(&g_link_state_info.id, NULL, link_state_monitor_thread, &g_link_state_info);
    if (ret != 0)
    {
        DBGPRINT((stderr, "pthread_create() failed\n"));
    }

    return ret;
}

int moca_link_state_monitor_uninit(void)
{
    moca_cancel_event_loop(g_link_state_info.ctx);
    moca_close(g_link_state_info.ctx);
    pthread_join(g_link_state_info.id, NULL);
    return 0;
}

/* Device.Moca.Interface.{i}.Stats. */
int moca_if_stats_bytes_sent(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_tx_total_bytes;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_bytes_received(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_rx_total_bytes;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_packets_sent(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_tx_total_pkts;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_packets_received(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_rx_total_pkts;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_errors_sent(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_tx_error_drop_pkts;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_errors_received(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_error_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_error_stats(ctx, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.rx_uc_crc_error + stats.rx_uc_crc_error_sec_ch + stats.rx_bc_crc_error + stats.rx_map_crc_error +
                     stats.rx_beacon_crc_error + stats.rx_rr_crc_error + stats.rx_ofdma_rr_crc_error + stats.rx_lc_uc_crc_error +
                     stats.rx_lc_bc_crc_error + stats.rx_probe1_error + stats.rx_probe1_error_sec_ch + stats.rx_probe2_error +
                     stats.rx_probe3_error + stats.rx_probe1_gcd_error + stats.rx_plp_crc_error + stats.rx_acf_crc_error;

            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_unicast_packets_sent(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_tx_ucast_pkts;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_unicast_packets_received(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_rx_ucast_pkts;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}


int moca_if_stats_discard_packets_sent(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_tx_buff_drop_pkts +
                     stats.ecl_tx_mcast_drops +
                     stats.ecl_tx_ucast_drops;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_discard_packets_received(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_rx_ucast_drops;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_multicast_packets_sent(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_tx_mcast_pkts;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_multicast_packets_received(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_rx_mcast_pkts;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_broadcast_packets_sent(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_tx_bcast_pkts;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_stats_broadcast_packets_received(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_gen_stats stats;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_gen_stats(ctx, 0, &stats);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = stats.ecl_rx_bcast_pkts;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

#if 0 /* NOT SUPPORT */
int moca_if_stats_unknown_proto_packets_received(int *value)
{
}
#endif

/* Device.Ethernet.Interface.{i}. */
static int ioctl_get(int command, struct ifreq *ifrq, const char* name)
{
    int fd = -1, rc = 0;

    if (name == NULL)
	{
        DBGPRINT((stderr, "invalid moca interface name"));
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
	{
        DBGPRINT((stderr, "couldn't create socket"));
        return -2;
    }

    strncpy(ifrq->ifr_name, name, sizeof(ifrq->ifr_name));
    ifrq->ifr_name[ sizeof(ifrq->ifr_name)-1 ] = 0;
    rc = ioctl(fd, command, ifrq);
    if (rc < 0)
	{
        DBGPRINT((stderr, "ioctl %d returned %d", command, rc));
        rc = -3;
    }

    if (fd >= 0)
    {
        close(fd);
    }

    return rc;
}

static int ioctl_set(int flags, int and_complement, const char* name)
{
	struct ifreq ifrq;
	int fd = -1, rc = 0;

	if (name == NULL)
	{
        DBGPRINT((stderr, "invalid moca interface name"));
	    return -1;
	}

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
	{
        DBGPRINT((stderr, "couldn't create socket"));
        return -2;
    }

	strncpy(ifrq.ifr_name, name, sizeof(ifrq.ifr_name));
	ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1 ] = 0;
	rc = ioctl(fd, SIOCGIFFLAGS, &ifrq);
	if(rc < 0)
	{
	    DBGPRINT((stderr, "error getting flags"));
	    close(fd);
	    return -3;
	}
	if (and_complement == 0)
	{
	    ifrq.ifr_flags |= flags;
	}
	else
	{
	    ifrq.ifr_flags &= ~flags;
	}

	rc = ioctl(fd, SIOCSIFFLAGS, &ifrq);
	if (rc < 0)
	{
	    close(fd);
	    DBGPRINT((stderr, "error setting flags"));
	    return -4;
	}

	if (fd >= 0)
	{
	    close(fd);
	}

	return 0;
}


/* Device.Moca.Interface.{i}. */
int moca_if_get_enable(int *value)
{
    void *ctx;
    struct moca_drv_info info;
    struct ifreq ifrq;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_drv_info(ctx, &info);

        if (ret == 0)
        {
            ret = ioctl_get(SIOCGIFFLAGS, &ifrq, info.ifname);

            if (ret == 0)
            {
                *value = (ifrq.ifr_flags & IFF_UP) ? 1 : 0;
                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_set_enable(int value)
{
    void *ctx;
    struct moca_drv_info info;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_drv_info(ctx, &info);

        if (ret == MOCA_API_SUCCESS)
        {
            ret = ioctl_set(IFF_UP, value, info.ifname);
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_status(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_drv_info info;
    struct ifreq ifrq;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_drv_info(ctx, &info);

        if (ret == 0)
        {
            ret = ioctl_get(SIOCGIFFLAGS, &ifrq, info.ifname);

            if (ret == MOCA_API_SUCCESS)
            {
                if (ifrq.ifr_flags & IFF_UP)
                {
                    if (ifrq.ifr_flags & IFF_RUNNING)
                    {
                        *value = MOCA_IF_STATUS_UP;
                    }
                    else
                    {
                        *value = MOCA_IF_STATUS_DOWN;
                    }
                }
                else
                {
                    *value = MOCA_IF_STATUS_DOWN;
                }
                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_name(char *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_drv_info info;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_drv_info(ctx, &info);

        if (ret == MOCA_API_SUCCESS)
        {
            sprintf(value, "%s", info.ifname);
            DBGPRINT((stdout, "%s: %s\n", __FUNCTION__, value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_last_change(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    unsigned int time_diff;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            if (ifstatus.link_status == MOCA_LINK_UP)
            {
                time_diff = get_time_sec() - g_link_state_info.since;
                *value = (time_diff > 0) ? time_diff : 0xffffffff - time_diff;
            }
            else
            {
                *value = 0;
            }
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_mac_addr(unsigned char *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_mac_addr mac_addr;
    int ret = 0, i;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_mac_addr(ctx, &mac_addr);

        if (ret == MOCA_API_SUCCESS)
        {
            for (i = 0; i < 6; i++)
                value[i] = mac_addr.val.addr[i];
            DBGPRINT((stdout, "%s: %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
                value[0], value[1], value[2], value[3], value[4], value[5]));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_firmware_version(char *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_fw_version fw_version;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_fw_version(ctx, &fw_version);

        if (ret == MOCA_API_SUCCESS)
        {
            sprintf(value, "%d.%d.%dp%d",
                fw_version.version_moca, fw_version.version_major,
                fw_version.version_minor, fw_version.version_patch);
            DBGPRINT((stdout, "%s: %s\n", __FUNCTION__, value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_max_bitrate(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    unsigned int max_phy_rate;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_max_phy_rate(ctx, &max_phy_rate);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = max_phy_rate;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_max_ingress_bandwidth(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns_local, gns_remote;
    struct moca_pqos_create_flow_in pqosc_in;
    struct moca_pqos_create_flow_out pqosc_out;
    struct moca_pqos_delete_flow_out pqos_delete_rsp;
    unsigned int altnode;
    unsigned int altnodemask;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            if (moca_count_bits(netstatus.connected_nodes) < 2)
            {
                DBGPRINT((stderr, "error! only one node currently exists\n"));
                moca_close(ctx);
                return(-3);
            }

            ret = moca_get_gen_node_status(ctx, netstatus.node_id, &gns_local);

            if (ret != MOCA_API_SUCCESS)
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                moca_close(ctx);
                return(-4);
            }

            altnode = 0;
            altnodemask = netstatus.connected_nodes;

            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    ret = moca_get_gen_node_status(ctx, altnode, &gns_remote);

                    if (ret == MOCA_API_SUCCESS)
                    {
                        break;
                    }
                    else
                    {
                        DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        moca_close(ctx);
                        return(-5);
                    }
                }

                altnode++;
            }

            memset(&pqosc_in, 0, sizeof(pqosc_in));
            MOCA_SET_PQOS_CREATE_FLOW_FLOW_ID_DEF(pqosc_in.flow_id);
            MOCA_SET_PQOS_CREATE_FLOW_PACKET_DA_DEF(pqosc_in.packet_da);
            pqosc_in.packet_size = MOCA_PQOS_CREATE_FLOW_PACKET_SIZE_DEF;
            pqosc_in.flow_tag = MOCA_PQOS_CREATE_FLOW_FLOW_TAG_DEF;
            pqosc_in.lease_time = MOCA_PQOS_CREATE_FLOW_LEASE_TIME_DEF;
            pqosc_in.burst_size = MOCA_PQOS_CREATE_FLOW_BURST_SIZE_DEF;
            pqosc_in.vlan_id = MOCA_PQOS_CREATE_FLOW_VLAN_ID_DEF;
            pqosc_in.max_latency = MOCA_PQOS_CREATE_FLOW_MAX_LATENCY_DEF;
            pqosc_in.short_term_avg_ratio = MOCA_PQOS_CREATE_FLOW_SHORT_TERM_AVG_RATIO_DEF;
            pqosc_in.max_retry = MOCA_PQOS_CREATE_FLOW_MAX_RETRY_DEF;
            pqosc_in.flow_per = MOCA_PQOS_CREATE_FLOW_FLOW_PER_DEF;
            pqosc_in.in_order_delivery = MOCA_PQOS_CREATE_FLOW_IN_ORDER_DELIVERY_DEF;
            pqosc_in.traffic_protocol = MOCA_PQOS_CREATE_FLOW_TRAFFIC_PROTOCOL_DEF;
            pqosc_in.ingr_class_rule = MOCA_PQOS_CREATE_FLOW_INGR_CLASS_RULE_DEF;
            pqosc_in.vlan_tag = MOCA_PQOS_CREATE_FLOW_VLAN_TAG_DEF;
            pqosc_in.dscp_moca = MOCA_PQOS_CREATE_FLOW_DSCP_MOCA_DEF;
            pqosc_in.ingress_node = gns_local.eui;
            pqosc_in.egress_node = gns_remote.eui;
            pqosc_in.peak_data_rate = 0xFFFFFF;

            ret = moca_do_pqos_create_flow(ctx, &pqosc_in, &pqosc_out);

            if (ret == MOCA_API_SUCCESS)
            {
                if (pqosc_out.response_code == MOCA_L2_SUCCESS)
                {
                    *value = pqosc_out.peak_data_rate;
                    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

                    ret = moca_do_pqos_delete_flow(ctx, pqosc_in.flow_id, &pqos_delete_rsp);

                    if (ret != MOCA_API_SUCCESS && pqos_delete_rsp.response_code != MOCA_L2_SUCCESS)
                    {
                        DBGPRINT((stderr, "%s: fail to delete flow: %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, MOCA_DISPLAY_MAC(pqosc_in.flow_id)));
                        DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqos_delete_rsp.response_code, moca_l2_error_name(pqos_delete_rsp.response_code)));
                    }
                }
                else
                {
                    DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqosc_out.response_code, moca_l2_error_name(pqosc_out.response_code)));
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_max_egress_bandwidth(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns_local, gns_remote;
    struct moca_pqos_create_flow_in pqosc_in;
    struct moca_pqos_create_flow_out pqosc_out;
    struct moca_pqos_delete_flow_out pqos_delete_rsp;
    unsigned int altnode;
    unsigned int altnodemask;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            if (moca_count_bits(netstatus.connected_nodes) < 2)
            {
                DBGPRINT((stderr, "error! only one node currently exists\n"));
                moca_close(ctx);
                return(-3);
            }

            ret = moca_get_gen_node_status(ctx, netstatus.node_id, &gns_local);

            if (ret != MOCA_API_SUCCESS)
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                moca_close(ctx);
                return(-4);
            }

            altnode = 0;
            altnodemask = netstatus.connected_nodes;

            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    ret = moca_get_gen_node_status(ctx, altnode, &gns_remote);

                    if (ret == MOCA_API_SUCCESS)
                    {
                        break;
                    }
                    else
                    {
                        DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        moca_close(ctx);
                        return(-5);
                    }
                }

                altnode++;
            }

            memset(&pqosc_in, 0, sizeof(pqosc_in));
            MOCA_SET_PQOS_CREATE_FLOW_FLOW_ID_DEF(pqosc_in.flow_id);
            MOCA_SET_PQOS_CREATE_FLOW_PACKET_DA_DEF(pqosc_in.packet_da);
            pqosc_in.packet_size = MOCA_PQOS_CREATE_FLOW_PACKET_SIZE_DEF;
            pqosc_in.flow_tag = MOCA_PQOS_CREATE_FLOW_FLOW_TAG_DEF;
            pqosc_in.lease_time = MOCA_PQOS_CREATE_FLOW_LEASE_TIME_DEF;
            pqosc_in.burst_size = MOCA_PQOS_CREATE_FLOW_BURST_SIZE_DEF;
            pqosc_in.vlan_id = MOCA_PQOS_CREATE_FLOW_VLAN_ID_DEF;
            pqosc_in.max_latency = MOCA_PQOS_CREATE_FLOW_MAX_LATENCY_DEF;
            pqosc_in.short_term_avg_ratio = MOCA_PQOS_CREATE_FLOW_SHORT_TERM_AVG_RATIO_DEF;
            pqosc_in.max_retry = MOCA_PQOS_CREATE_FLOW_MAX_RETRY_DEF;
            pqosc_in.flow_per = MOCA_PQOS_CREATE_FLOW_FLOW_PER_DEF;
            pqosc_in.in_order_delivery = MOCA_PQOS_CREATE_FLOW_IN_ORDER_DELIVERY_DEF;
            pqosc_in.traffic_protocol = MOCA_PQOS_CREATE_FLOW_TRAFFIC_PROTOCOL_DEF;
            pqosc_in.ingr_class_rule = MOCA_PQOS_CREATE_FLOW_INGR_CLASS_RULE_DEF;
            pqosc_in.vlan_tag = MOCA_PQOS_CREATE_FLOW_VLAN_TAG_DEF;
            pqosc_in.dscp_moca = MOCA_PQOS_CREATE_FLOW_DSCP_MOCA_DEF;
            pqosc_in.ingress_node = gns_remote.eui;
            pqosc_in.egress_node = gns_local.eui;
            pqosc_in.peak_data_rate = 0xFFFFFF;

            ret = moca_do_pqos_create_flow(ctx, &pqosc_in, &pqosc_out);

            if (ret == MOCA_API_SUCCESS)
            {
                if (pqosc_out.response_code == MOCA_L2_SUCCESS)
                {
                    *value = pqosc_out.peak_data_rate;
                    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

                    ret = moca_do_pqos_delete_flow(ctx, pqosc_in.flow_id, &pqos_delete_rsp);

                    if (ret != MOCA_API_SUCCESS && pqos_delete_rsp.response_code != MOCA_L2_SUCCESS)
                    {
                        DBGPRINT((stderr, "%s: fail to delete flow: %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, MOCA_DISPLAY_MAC(pqosc_in.flow_id)));
                        DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqos_delete_rsp.response_code, moca_l2_error_name(pqos_delete_rsp.response_code)));
                    }
                }
                else
                {
                    DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqosc_out.response_code, moca_l2_error_name(pqosc_out.response_code)));
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_highest_version(char *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_fw_version fw_version;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_fw_version(ctx, &fw_version);

        if (ret == MOCA_API_SUCCESS)
        {
            sprintf(value, "%d.0", fw_version.version_moca);
            DBGPRINT((stdout, "%s: %s\n", __FUNCTION__, value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_current_version(char *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status status;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &status);

        if (ret == MOCA_API_SUCCESS)
        {
            switch (status.network_moca_version)
            {
                case 0x10:
                    sprintf(value, "%s", "1.0");
                    break;
                case 0x11:
                    sprintf(value, "%s", "1.1");
                    break;
                case 0x20:
                    sprintf(value, "%s", "2.0");
                    break;
                default:
                    DBGPRINT((stderr, "%s: unknown self_moca_version\n", __FUNCTION__));
                    sprintf(value, "%s", "0.0");
                    break;

            }
            DBGPRINT((stdout, "%s: %s\n", __FUNCTION__, value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_network_coordinator(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status status;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &status);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = status.nc_node_id;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_node_id(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status status;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &status);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = status.node_id;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_preferred_nc(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    unsigned int is_preferred_nc;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_preferred_nc(ctx, &is_preferred_nc);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = is_preferred_nc;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_set_preferred_nc(int value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_set_preferred_nc(ctx, value);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            moca_close(ctx);
            return(-3);
        }

        ret = moca_set_restart(ctx);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error!  unable to restart moca\n"));
            moca_close(ctx);
            return(-4);
        }

        ret = moca_set_persistent(ctx);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error!  unable to save persistent parameters\n"));
            moca_close(ctx);
            return(-5);
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_backup_nc(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status status;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &status);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = status.backup_nc_id;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_privacy_enabled_setting(int *value)
{
    void *ctx;
    unsigned int privacy;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_privacy_en(ctx, &privacy);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = privacy;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_set_privacy_enabled_setting(int value)
{
    void *ctx;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_set_privacy_en(ctx, value);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}


int moca_if_get_privacy_enabled(int *value)
{
    void *ctx;
    unsigned int privacy;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_privacy_en(ctx, &privacy);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = privacy;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_freq_capability_mask(int *value)
{
    void *ctx;
    unsigned int mask;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_listening_freq_mask(ctx, &mask);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = mask;
            DBGPRINT((stdout, "%s: 0x%08X\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_freq_current_mask_setting(int *value)
{
    void *ctx;
    unsigned int mask;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_listening_freq_mask(ctx, &mask);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = mask;
            DBGPRINT((stdout, "%s: 0x%08X\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_set_freq_current_mask_setting(int value)
{
    void *ctx;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_set_listening_freq_mask(ctx, value);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_freq_current_mask(int *value)
{
    void *ctx;
    unsigned int mask;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_listening_freq_mask(ctx, &mask);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = mask;
            DBGPRINT((stdout, "%s: 0x%08X\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_current_operational_frequency(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        if (ret == MOCA_API_SUCCESS)
        {
            *value = ifstatus.rf_channel;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_last_operational_frequency(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    unsigned int lof;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_lof(ctx, &lof);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = lof;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_key_passphase(char *value)
{
    void *ctx;
    struct moca_password passphase;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_password(ctx, &passphase);

        if (ret == MOCA_API_SUCCESS)
        {
            memcpy(value, passphase.password, 32);
            DBGPRINT((stdout, "%s: %s\n", __FUNCTION__, value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_set_key_passphase(char *value)
{
    void *ctx;
    struct moca_password passphase;
    unsigned int privacy;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_privacy_en(ctx, &privacy);

        if (ret == MOCA_API_SUCCESS)
        {
            if (privacy)
            {
                memcpy(passphase.password, value, 32);

                ret = moca_set_password(ctx, &passphase);

                if (ret != MOCA_API_SUCCESS)
                {
                   DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                   moca_close(ctx);
                   return(-1);
                }
            }
        }

        ret = moca_set_restart(ctx);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error!  unable to restart moca\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_set_persistent(ctx);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error!  unable to save persistent parameters\n"));
            moca_close(ctx);
            return(-3);
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_max_tx_power(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    int power;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_max_tx_power(ctx, &power);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = power;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_set_max_tx_power(int value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_set_max_tx_power(ctx, value);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            moca_close(ctx);
            return(-3);
        }

        ret = moca_set_restart(ctx);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error!  unable to restart moca\n"));
            moca_close(ctx);
            return(-4);
        }

        ret = moca_set_persistent(ctx);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error!  unable to save persistent parameters\n"));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_power_conrtol_target_phy_rate(int *value)
{
    void *ctx;
    unsigned int qam256_capable = 0;
    unsigned int rate = 0;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_qam256_capability(ctx, &qam256_capable);

        if (ret == MOCA_API_SUCCESS)
        {
            if (qam256_capable)
            {
                ret = moca_get_target_phy_rate_qam256(ctx, &rate);
            }
            else
            {
                ret = moca_get_target_phy_rate_qam128(ctx, &rate);
            }

            if (ret == MOCA_API_SUCCESS)
            {
                *value = rate;
                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_set_power_conrtol_target_phy_rate(int value)
{
    void *ctx;
    struct moca_drv_info info;
    unsigned int qam256_capable = 0;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_drv_info(ctx, &info);

        if (ret == MOCA_API_SUCCESS)
        {
            ret = ioctl_set(IFF_UP, 0, info.ifname); /* disable moca interface */
        }
        else
        {
           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
           moca_close(ctx);
           return(-1);
        }

        ret = moca_get_qam256_capability(ctx, &qam256_capable);

        if (ret == MOCA_API_SUCCESS)
        {
            if (qam256_capable)
            {
                ret = moca_set_target_phy_rate_qam256(ctx, value);
            }
            else
            {
                ret = moca_set_target_phy_rate_qam128(ctx, value);
            }

            if (ret != MOCA_API_SUCCESS)
            {
               DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
               moca_close(ctx);
               return(-2);
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            moca_close(ctx);
            return(-3);
        }

        ret = moca_set_restart(ctx);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error!  unable to restart moca\n"));
            moca_close(ctx);
            return(-4);
        }

        ret = moca_set_persistent(ctx);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error!  unable to save persistent parameters\n"));
            moca_close(ctx);
            return(-5);
        }

        ret = ioctl_set(IFF_UP, 1, info.ifname); /* enable moca interface */

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            moca_close(ctx);
            return(-6);
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_beacon_power_reduction(int *value)
{
    void *ctx;
    unsigned int power_reduction_enable = 0;
    unsigned int power = 0;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_beacon_pwr_reduction_en(ctx, &power_reduction_enable);

        if (ret == MOCA_API_SUCCESS)
        {
            if (power_reduction_enable)
            {
                ret = moca_get_beacon_pwr_reduction(ctx, &power);
            }
            else
            {
                DBGPRINT((stderr, "beacon power reduction is disabled\n"));
                moca_close(ctx);
                return(-1);
            }

            if (ret == MOCA_API_SUCCESS)
            {
                *value = power;
                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
            }
            else
            {
               DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_set_beacon_power_reduction(int value)
{
    void *ctx;
    struct moca_drv_info info;
    unsigned int power_reduction_enable = 0;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_drv_info(ctx, &info);

        if (ret == MOCA_API_SUCCESS)
        {
            ret = ioctl_set(IFF_UP, 0, info.ifname); /* disable moca interface */
        }
        else
        {
           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
           moca_close(ctx);
           return(-1);
        }

        ret = moca_get_beacon_pwr_reduction_en(ctx, &power_reduction_enable);

        if (ret == MOCA_API_SUCCESS)
        {
            if (power_reduction_enable)
            {
                ret = moca_set_beacon_pwr_reduction(ctx, value);

                if (ret != MOCA_API_SUCCESS)
                {
                    DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                    moca_close(ctx);
                    return(-2);
                }
            }
            else
            {
                DBGPRINT((stderr, "beacon power reduction is disabled\n"));
                moca_close(ctx);
                return(-3);
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            moca_close(ctx);
            return(-4);
        }

        ret = moca_set_restart(ctx);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error!  unable to restart moca\n"));
            moca_close(ctx);
            return(-5);
        }

        ret = moca_set_persistent(ctx);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error!  unable to save persistent parameters\n"));
            moca_close(ctx);
            return(-6);
        }

        ret = ioctl_set(IFF_UP, 1, info.ifname); /* enable moca interface */

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            moca_close(ctx);
            return(-7);
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_network_taboo_mask(int *value)
{
    void *ctx;
    struct moca_network_status status;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_network_status(ctx, &status);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = status.network_taboo_mask;
            DBGPRINT((stdout, "%s: 0x%08X\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_node_taboo_mask(int *value)
{
    void *ctx;
    struct moca_taboo_channels taboo_channel;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_taboo_channels(ctx, &taboo_channel);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = taboo_channel.taboo_fixed_channel_mask;
            DBGPRINT((stdout, "%s: 0x%08X\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

static int get_phy_rate(void * ctx, unsigned int node, unsigned int profile, unsigned int *phy_rate)
{
    struct moca_gen_node_ext_status gns_ext;
    struct moca_gen_node_ext_status_in gns_ext_in;
    int ret;

    gns_ext_in.index = node;
    gns_ext_in.profile_type = profile;
    ret = moca_get_gen_node_ext_status(ctx, &gns_ext_in, &gns_ext);
    if (ret != MOCA_API_SUCCESS)
    {
        DBGPRINT((stderr, "error!  mocalib phyrate failure\n"));
        return(-1);
    }

    *phy_rate = gns_ext.phy_rate;

    return(0);
}

int moca_if_get_tx_broadcast_phy_rate(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int gcdTxRate = 0;
    unsigned int gcdTxNperRate = 0;
    unsigned int gcdTxVlperRate = 0;
    unsigned int node;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }
        ret = moca_get_network_status(ctx, &netstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! mocalib net failure\n"));
            moca_close(ctx);
            return(-3);
        }

        node = netstatus.node_id;

        ret = moca_get_gen_node_status(ctx, node, &gns);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! mocalib net failure\n"));
            moca_close(ctx);
            return(-4);
        }

        if (gns.active_moca_version < MoCA_VERSION_2_0)
        {
            ret = get_phy_rate(ctx, node, MOCA_EXT_STATUS_PROFILE_TX_BCAST, &gcdTxRate);

            if (ret == MOCA_API_SUCCESS)
            {
                *value = gcdTxRate;
                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
            }
            else
            {
                moca_close(ctx);
                return(-5);
            }
        }
        else
        {
            ret = get_phy_rate(ctx, node, MOCA_EXT_STATUS_PROFILE_TX_BC_NPER, &gcdTxNperRate );

            if (ret == MOCA_API_SUCCESS)
            {
                *value = gcdTxNperRate;
                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
            }
            else
            {
                moca_close(ctx);
                return(-6);
            }

            ret = get_phy_rate(ctx, node, MOCA_EXT_STATUS_PROFILE_TX_BC_VLPER, &gcdTxVlperRate );

            if (ret == MOCA_API_SUCCESS)
            {
                #if 0
                *value = gcdTxVlperRate;
                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                #endif
            }
            else
            {
                moca_close(ctx);
                return(-7);
            }
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_tx_broadcast_power_reduction(int *value)
{
    void *ctx;
    unsigned int tx_gcd_power_reduction;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_phy_status(ctx, &tx_gcd_power_reduction);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = tx_gcd_power_reduction;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_qam256_capable(int *value)
{
    void *ctx;
    unsigned int qam256_capable = 0;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_qam256_capability(ctx, &qam256_capable);

        if (ret == MOCA_API_SUCCESS)
        {
            if (ret == MOCA_API_SUCCESS)
            {
                *value = qam256_capable;
                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

static int get_packet_aggregation_capability(void *ctx, unsigned int node, unsigned int *cap)
{
    struct moca_gen_node_status gns;
    struct moca_dd_init_out dd_info;
    unsigned int nodemask, i;
    int ret = 0;

    ret = moca_get_gen_node_status(ctx, node, &gns);

    if (ret == MOCA_API_SUCCESS)
    {
        if (gns.active_moca_version < MoCA_VERSION_2_0)
        {
            *cap = ((gns.protocol_support >> 7) & 0x3) == 0x2 ? 10 : 6;
            DBGPRINT((stdout, "%s: %s\n", __FUNCTION__, *cap));
        }
        else
        {
            nodemask = 1;
            for (i = 1; i <= node; i++)
            {
                nodemask = nodemask << 1;
            }

            if ((nodemask == 0) || (nodemask > 0xFFFF))
            {
                DBGPRINT((stderr, "%s: rrror! invalid parameter - bitmask\n", __FUNCTION__));
                moca_close(ctx);
                return(-1);
            }

            ret = moca_do_dd_init(ctx, nodemask, &dd_info);

            if (ret == MOCA_API_SUCCESS)
            {
                if (dd_info.responsecode == MOCA_L2_SUCCESS)
                {
                    if ((nodemask & (1 << node)) && (dd_info.responded_nodemask & (1 << node)))
                    {
                        *cap = dd_info.aggr_size[node];
                        /* DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *cap)); */
                    }
                }
                else
                {
                    DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, dd_info.responsecode));
                    return(-2);
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                return(-3);
            }
        }
    }
    else
    {
        DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        return(-4);
    }

    return ret;
}


int moca_if_get_packet_aggregation_capability (int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status status;
    unsigned int cap;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &status);

        if (ret == MOCA_API_SUCCESS)
        {
            ret = get_packet_aggregation_capability(ctx, status.node_id, &cap);

            if (ret == MOCA_API_SUCCESS)
            {
                *value = cap;
                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;

}

int moca_if_get_associated_device_number_of_entries(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = moca_count_bits(netstatus.connected_nodes) - 1;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

static int get_tx_power_control_reduction(void * ctx, unsigned int node, unsigned int profile, int *power)
{
    struct moca_gen_node_ext_status gns_ext;
    struct moca_gen_node_ext_status_in gns_ext_in;
    int ret;

    gns_ext_in.index = node;
    gns_ext_in.profile_type = profile;
    ret = moca_get_gen_node_ext_status(ctx, &gns_ext_in, &gns_ext);
    if (ret != MOCA_API_SUCCESS)
    {
        DBGPRINT((stderr, "error!  mocalib tx power failure\n"));
        return(-1);
    }

    *power = gns_ext.tx_backoff;

    return(0);
}

static int get_rx_power_level(void * ctx, unsigned int node, unsigned int profile, int *power)
{
    struct moca_gen_node_ext_status gns_ext;
    struct moca_gen_node_ext_status_in gns_ext_in;
    int ret;

    gns_ext_in.index = node;
    gns_ext_in.profile_type = profile;
    ret = moca_get_gen_node_ext_status(ctx, &gns_ext_in, &gns_ext);
    if (ret != MOCA_API_SUCCESS)
    {
        DBGPRINT((stderr, "error!  mocalib rx power failure\n"));
        return(-1);
    }

    *power = gns_ext.rx_power;

    return(0);
}

static int get_rx_sn_ratio(void * ctx, unsigned int node, unsigned int profile, unsigned int *snr)
{
    struct moca_gen_node_ext_status gns_ext;
    struct moca_gen_node_ext_status_in gns_ext_in;
    int ret;

    gns_ext_in.index = node;
    gns_ext_in.profile_type = profile;
    ret = moca_get_gen_node_ext_status(ctx, &gns_ext_in, &gns_ext);
    if (ret != MOCA_API_SUCCESS)
    {
        DBGPRINT((stderr, "error!  mocalib rx power failure\n"));
        return(-1);
    }

    *snr = gns_ext.avg_snr;

    return(0);
}

/* Device.Moca.Interface.{i}.AssociatedDevice.{i}. */
int moca_if_get_associated_device_mac_address(int instance_id, unsigned char *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    int ret = 0, i;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = moca_get_gen_node_status(ctx, altnode, &gns);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            for (i = 0; i < 6; i++)
                                value[i] = gns.eui.addr[i];
                            DBGPRINT((stdout, "%s: %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
                                value[0], value[1], value[2], value[3], value[4], value[5]));
                        }
                        else
                        {
                            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_node_id(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        *value = altnode;
                        DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));

                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_preferred_nc(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = moca_get_gen_node_status(ctx, altnode, &gns);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            *value = (gns.protocol_support & 0x20) ? 1 : 0;
                            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                        }
                        else
                        {
                           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_phy_tx_rate(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    unsigned int phytxrate;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = moca_get_gen_node_status(ctx, altnode, &gns);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            if (gns.active_moca_version < MoCA_VERSION_2_0)
                            {
                                ret = get_phy_rate(ctx, altnode, MOCA_EXT_STATUS_PROFILE_TX_UCAST, &phytxrate);
                            }
                            else
                            {
                                ret = get_phy_rate(ctx, altnode, MOCA_EXT_STATUS_PROFILE_TX_UC_NPER, &phytxrate);
                            }

                            if (ret == MOCA_API_SUCCESS)
                            {
                                *value = phytxrate;
                                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                            }
                            else
                            {
                                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                            }
                        }
                        else
                        {
                            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_phy_rx_rate(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    unsigned int phyrxrate;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = moca_get_gen_node_status(ctx, altnode, &gns);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            if (gns.active_moca_version < MoCA_VERSION_2_0)
                            {
                                ret = get_phy_rate(ctx, altnode, MOCA_EXT_STATUS_PROFILE_RX_UCAST, &phyrxrate);
                            }
                            else
                            {
                                ret = get_phy_rate(ctx, altnode, MOCA_EXT_STATUS_PROFILE_RX_UC_NPER, &phyrxrate);
                            }

                            if (ret == MOCA_API_SUCCESS)
                            {
                                *value = phyrxrate;
                                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                            }
                            else
                            {
                                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                            }
                        }
                        else
                        {
                           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_tx_power_control_reduction(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    int txpower;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = moca_get_gen_node_status(ctx, altnode, &gns);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            if (gns.active_moca_version < MoCA_VERSION_2_0)
                            {
                                ret = get_tx_power_control_reduction(ctx, altnode, MOCA_EXT_STATUS_PROFILE_TX_UCAST, &txpower);
                            }
                            else
                            {
                                ret = get_tx_power_control_reduction(ctx, altnode, MOCA_EXT_STATUS_PROFILE_TX_UC_NPER, &txpower);
                            }

                            if (ret == MOCA_API_SUCCESS)
                            {
                                *value = txpower;
                                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                            }
                            else
                            {
                                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                            }
                        }
                        else
                        {
                           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_rx_power_level(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    int rxpower;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = moca_get_gen_node_status(ctx, altnode, &gns);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            if (gns.active_moca_version < MoCA_VERSION_2_0)
                            {
                                ret = get_rx_power_level(ctx, altnode, MOCA_EXT_STATUS_PROFILE_RX_UCAST, &rxpower);
                            }
                            else
                            {
                                ret = get_rx_power_level(ctx, altnode, MOCA_EXT_STATUS_PROFILE_RX_UC_NPER, &rxpower);
                            }

                            if (ret == MOCA_API_SUCCESS)
                            {
                                *value = abs(rxpower);
                                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                            }
                            else
                            {
                                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                            }
                        }
                        else
                        {
                           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_tx_broadcast_rate(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    unsigned int txbcastrate;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = moca_get_gen_node_status(ctx, altnode, &gns);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            if (gns.active_moca_version < MoCA_VERSION_2_0)
                            {
                                ret = get_phy_rate(ctx, altnode, MOCA_EXT_STATUS_PROFILE_TX_BCAST, &txbcastrate);
                            }
                            else
                            {
                                ret = get_phy_rate(ctx, altnode, MOCA_EXT_STATUS_PROFILE_TX_BC_NPER, &txbcastrate);
                            }

                            if (ret == MOCA_API_SUCCESS)
                            {
                                *value = txbcastrate;
                                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                            }
                            else
                            {
                                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                            }
                        }
                        else
                        {
                           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_rx_broadcast_power_level(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    int rxbcastpower;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = moca_get_gen_node_status(ctx, altnode, &gns);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            if (gns.active_moca_version < MoCA_VERSION_2_0)
                            {
                                ret = get_rx_power_level(ctx, altnode, MOCA_EXT_STATUS_PROFILE_RX_BCAST, &rxbcastpower);
                            }
                            else
                            {
                                ret = get_rx_power_level(ctx, altnode, MOCA_EXT_STATUS_PROFILE_RX_BC_NPER, &rxbcastpower);
                            }

                            if (ret == MOCA_API_SUCCESS)
                            {
                                *value = abs(rxbcastpower);
                                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                            }
                            else
                            {
                                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                            }
                        }
                        else
                        {
                            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_tx_packets(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_node_stats nodestats;
    struct moca_node_stats_in in;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        in.index = altnode;
                        in.reset_stats = 0;
                        ret = moca_get_node_stats(ctx, &in, &nodestats);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            *value = nodestats.tx_packets;
                            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                        }
                        else
                        {
                            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_rx_packets(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_node_stats nodestats;
    struct moca_node_stats_in in;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        in.index = altnode;
                        in.reset_stats = 0;
                        ret = moca_get_node_stats(ctx, &in, &nodestats);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            *value = nodestats.rx_packets;
                            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                        }
                        else
                        {
                            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_rx_errored_and_missed_packets(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_node_stats_ext nodestatsext;
    struct moca_node_stats_ext_in ext_in;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ext_in.index = altnode;
                        ext_in.reset_stats = 0;
                        ret = moca_get_node_stats_ext(ctx, &ext_in, &nodestatsext);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            *value = nodestatsext.rx_uc_crc_error + nodestatsext.rx_uc_timeout_error + nodestatsext.rx_bc_crc_error + nodestatsext.rx_bc_timeout_error;
                            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                        }
                        else
                        {
                            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_qam256_capable(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = moca_get_gen_node_status(ctx, altnode, &gns);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            *value = (gns.protocol_support & 0x8) ? 1 : 0;
                            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                        }
                        else
                        {
                            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}


int moca_if_get_associated_device_packet_aggregation_capability(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    unsigned int cap;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = get_packet_aggregation_capability(ctx, altnode, &cap);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            *value = cap;
                            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                        }
                        else
                        {
                            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
         DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_get_associated_device_rx_snr(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_network_status netstatus;
    struct moca_gen_node_status gns;
    unsigned int altnode;
    unsigned int altnodemask;
    int nth_remote_node;
    unsigned int rxsnr;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_network_status(ctx, &netstatus);

        if (ret == MOCA_API_SUCCESS)
        {
            altnode = 0;
            altnodemask = netstatus.connected_nodes;
            nth_remote_node = 1;
            while (altnode < 16)
            {
                if (((altnodemask & (1 << altnode)) != 0) && (altnode != netstatus.node_id))
                {
                    DBGPRINT((stdout, "%s: nth node: %d, instance id: %d\n", __FUNCTION__, nth_remote_node, instance_id));

                    if (nth_remote_node == instance_id)
                    {
                        DBGPRINT((stdout, "%s: remote node: %d, local node: %d\n", __FUNCTION__, altnode, netstatus.node_id));

                        ret = moca_get_gen_node_status(ctx, altnode, &gns);

                        if (ret == MOCA_API_SUCCESS)
                        {
                            if (gns.active_moca_version < MoCA_VERSION_2_0)
                            {
                                ret = get_rx_sn_ratio(ctx, altnode, MOCA_EXT_STATUS_PROFILE_RX_UCAST, &rxsnr);
                            }
                            else
                            {
                                ret = get_rx_sn_ratio(ctx, altnode, MOCA_EXT_STATUS_PROFILE_RX_UC_NPER, &rxsnr);
                            }

                            if (ret == MOCA_API_SUCCESS)
                            {
                                *value = rxsnr;
                                DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                            }
                            else
                            {
                                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                            }
                        }
                        else
                        {
                            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
                        }
                        break;
                    }
                    nth_remote_node++;
                }
                altnode++;
            }
        }
        else
        {
           DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
         DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

#if 0 /* NOT SUPPORT */
int moca_if_get_associated_device_active(int instance_id, int *value)
{
}
#endif

int moca_if_qos_egress_num_flows(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    unsigned int pqos_egress_numflows;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        ret = moca_get_pqos_egress_numflows(ctx, &pqos_egress_numflows);

        if (ret == MOCA_API_SUCCESS)
        {
            *value = pqos_egress_numflows;
            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_qos_ingress_num_flows(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_mac_addr mac_addr;
    struct moca_pqos_list_in pqos_list_in;
    struct moca_pqos_list_out pqos_list_out;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        memset (&pqos_list_in, 0x00, sizeof(pqos_list_in)) ;
        memset (&pqos_list_out, 0x00, sizeof(pqos_list_out)) ;

        ret = moca_get_mac_addr(ctx, &mac_addr);

        if (ret == MOCA_API_SUCCESS)
        {
            pqos_list_in.ingr_node_id = 0xFFFFFFFF;
            pqos_list_in.ingr_node_mac = mac_addr.val;
            pqos_list_in.flow_start_index = MOCA_PQOS_LIST_FLOW_START_INDEX_DEF;
            pqos_list_in.flow_max_return = MOCA_PQOS_LIST_FLOW_MAX_RETURN_DEF;

            ret = moca_do_pqos_list(ctx, &pqos_list_in, &pqos_list_out);

            if (ret == MOCA_API_SUCCESS)
            {
                if (pqos_list_out.response_code == MOCA_L2_SUCCESS)
                {
                    *value = pqos_list_out.num_ret_flow_ids;
                    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                }
                else
                {
                    DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqos_list_out.response_code, moca_l2_error_name(pqos_list_out.response_code)));
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_qos_flow_stats_number_of_entries(int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_mac_addr mac_addr;
    struct moca_pqos_list_in pqos_list_in;
    struct moca_pqos_list_out pqos_list_out;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        memset (&pqos_list_in, 0x00, sizeof(pqos_list_in)) ;
        memset (&pqos_list_out, 0x00, sizeof(pqos_list_out)) ;

        ret = moca_get_mac_addr(ctx, &mac_addr);

        if (ret == MOCA_API_SUCCESS)
        {
            pqos_list_in.ingr_node_id = 0xFFFFFFFF;
            pqos_list_in.ingr_node_mac = mac_addr.val;
            pqos_list_in.flow_start_index = MOCA_PQOS_LIST_FLOW_START_INDEX_DEF;
            pqos_list_in.flow_max_return = MOCA_PQOS_LIST_FLOW_MAX_RETURN_DEF;

            ret = moca_do_pqos_list(ctx, &pqos_list_in, &pqos_list_out);

            if (ret == MOCA_API_SUCCESS)
            {
                if (pqos_list_out.response_code == MOCA_L2_SUCCESS)
                {
                    *value = pqos_list_out.num_ret_flow_ids;
                    DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                }
                else
                {
                    DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqos_list_out.response_code, moca_l2_error_name(pqos_list_out.response_code)));
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_qos_flow_stats_flow_id(int instance_id, unsigned char *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_mac_addr mac_addr;
    struct moca_pqos_list_in pqos_list_in;
    struct moca_pqos_list_out pqos_list_out;
    macaddr_t pqos_query_in;
    struct moca_pqos_query_out pqos_query_out;
    int ret = 0, i;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        memset (&pqos_list_in, 0x00, sizeof(pqos_list_in)) ;
        memset (&pqos_list_out, 0x00, sizeof(pqos_list_out)) ;

        ret = moca_get_mac_addr(ctx, &mac_addr);

        if (ret == MOCA_API_SUCCESS)
        {
            pqos_list_in.ingr_node_id = 0xFFFFFFFF;
            pqos_list_in.ingr_node_mac = mac_addr.val;
            pqos_list_in.flow_start_index = MOCA_PQOS_LIST_FLOW_START_INDEX_DEF;
            pqos_list_in.flow_max_return = MOCA_PQOS_LIST_FLOW_MAX_RETURN_DEF;

            ret = moca_do_pqos_list(ctx, &pqos_list_in, &pqos_list_out);

            if (ret == MOCA_API_SUCCESS)
            {
                if (pqos_list_out.response_code == MOCA_L2_SUCCESS)
                {
                    if ((unsigned int)instance_id <= pqos_list_out.num_ret_flow_ids)
                    {
                        pqos_query_in = pqos_list_out.flowid[instance_id - 1];

                        ret = moca_do_pqos_query(ctx, pqos_query_in, &pqos_query_out);
                        if (ret == MOCA_API_SUCCESS && pqos_query_out.response_code == MOCA_L2_SUCCESS)
                        {
                            for (i = 0; i < 6; i++)
                                value[i] = pqos_query_out.flow_id.addr[i];
                            DBGPRINT((stdout, "%s: %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
                                value[0], value[1], value[2], value[3], value[4], value[5]));
                        }
                    }
                }
                else
                {
                    DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqos_list_out.response_code, moca_l2_error_name(pqos_list_out.response_code)));
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_qos_flow_stats_packet_da(int instance_id, unsigned char *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_mac_addr mac_addr;
    struct moca_pqos_list_in pqos_list_in;
    struct moca_pqos_list_out pqos_list_out;
    macaddr_t pqos_query_in;
    struct moca_pqos_query_out pqos_query_out;
    int ret = 0, i;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        memset (&pqos_list_in, 0x00, sizeof(pqos_list_in)) ;
        memset (&pqos_list_out, 0x00, sizeof(pqos_list_out)) ;

        ret = moca_get_mac_addr(ctx, &mac_addr);

        if (ret == MOCA_API_SUCCESS)
        {
            pqos_list_in.ingr_node_id = 0xFFFFFFFF;
            pqos_list_in.ingr_node_mac = mac_addr.val;
            pqos_list_in.flow_start_index = MOCA_PQOS_LIST_FLOW_START_INDEX_DEF;
            pqos_list_in.flow_max_return = MOCA_PQOS_LIST_FLOW_MAX_RETURN_DEF;

            ret = moca_do_pqos_list(ctx, &pqos_list_in, &pqos_list_out);

            if (ret == MOCA_API_SUCCESS)
            {
                if (pqos_list_out.response_code == MOCA_L2_SUCCESS)
                {
                    if ((unsigned int)instance_id <= pqos_list_out.num_ret_flow_ids)
                    {
                        pqos_query_in = pqos_list_out.flowid[instance_id - 1];

                        ret = moca_do_pqos_query(ctx, pqos_query_in, &pqos_query_out);
                        if (ret == MOCA_API_SUCCESS && pqos_query_out.response_code == MOCA_L2_SUCCESS)
                        {
                            for (i = 0; i < 6; i++)
                                value[i] = pqos_query_out.packet_da.addr[i];
                            DBGPRINT((stdout, "%s: %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
                                value[0], value[1], value[2], value[3], value[4], value[5]));
                        }
                    }
                }
                else
                {
                    DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqos_list_out.response_code, moca_l2_error_name(pqos_list_out.response_code)));
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_qos_flow_stats_max_rate(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_mac_addr mac_addr;
    struct moca_pqos_list_in pqos_list_in;
    struct moca_pqos_list_out pqos_list_out;
    macaddr_t pqos_query_in;
    struct moca_pqos_query_out pqos_query_out;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        memset (&pqos_list_in, 0x00, sizeof(pqos_list_in)) ;
        memset (&pqos_list_out, 0x00, sizeof(pqos_list_out)) ;

        ret = moca_get_mac_addr(ctx, &mac_addr);

        if (ret == MOCA_API_SUCCESS)
        {
            pqos_list_in.ingr_node_id = 0xFFFFFFFF;
            pqos_list_in.ingr_node_mac = mac_addr.val;
            pqos_list_in.flow_start_index = MOCA_PQOS_LIST_FLOW_START_INDEX_DEF;
            pqos_list_in.flow_max_return = MOCA_PQOS_LIST_FLOW_MAX_RETURN_DEF;

            ret = moca_do_pqos_list(ctx, &pqos_list_in, &pqos_list_out);

            if (ret == MOCA_API_SUCCESS)
            {
                if (pqos_list_out.response_code == MOCA_L2_SUCCESS)
                {
                    if ((unsigned int)instance_id <= pqos_list_out.num_ret_flow_ids)
                    {
                        pqos_query_in = pqos_list_out.flowid[instance_id - 1];

                        ret = moca_do_pqos_query(ctx, pqos_query_in, &pqos_query_out);
                        if (ret == MOCA_API_SUCCESS && pqos_query_out.response_code == MOCA_L2_SUCCESS)
                        {
                            *value = pqos_query_out.peak_data_rate;
                            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                        }
                    }
                }
                else
                {
                    DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqos_list_out.response_code, moca_l2_error_name(pqos_list_out.response_code)));
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_qos_flow_stats_max_burst_size(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_mac_addr mac_addr;
    struct moca_pqos_list_in pqos_list_in;
    struct moca_pqos_list_out pqos_list_out;
    macaddr_t pqos_query_in;
    struct moca_pqos_query_out pqos_query_out;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        memset (&pqos_list_in, 0x00, sizeof(pqos_list_in)) ;
        memset (&pqos_list_out, 0x00, sizeof(pqos_list_out)) ;

        ret = moca_get_mac_addr(ctx, &mac_addr);

        if (ret == MOCA_API_SUCCESS)
        {
            pqos_list_in.ingr_node_id = 0xFFFFFFFF;
            pqos_list_in.ingr_node_mac = mac_addr.val;
            pqos_list_in.flow_start_index = MOCA_PQOS_LIST_FLOW_START_INDEX_DEF;
            pqos_list_in.flow_max_return = MOCA_PQOS_LIST_FLOW_MAX_RETURN_DEF;

            ret = moca_do_pqos_list(ctx, &pqos_list_in, &pqos_list_out);

            if (ret == MOCA_API_SUCCESS)
            {
                if (pqos_list_out.response_code == MOCA_L2_SUCCESS)
                {
                    if ((unsigned int)instance_id <= pqos_list_out.num_ret_flow_ids)
                    {
                        pqos_query_in = pqos_list_out.flowid[instance_id - 1];

                        ret = moca_do_pqos_query(ctx, pqos_query_in, &pqos_query_out);
                        if (ret == MOCA_API_SUCCESS && pqos_query_out.response_code == MOCA_L2_SUCCESS)
                        {
                            *value = pqos_query_out.burst_size;
                            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                        }
                    }
                }
                else
                {
                    DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqos_list_out.response_code, moca_l2_error_name(pqos_list_out.response_code)));
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_qos_flow_stats_lease_time(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_mac_addr mac_addr;
    struct moca_pqos_list_in pqos_list_in;
    struct moca_pqos_list_out pqos_list_out;
    macaddr_t pqos_query_in;
    struct moca_pqos_query_out pqos_query_out;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        memset (&pqos_list_in, 0x00, sizeof(pqos_list_in)) ;
        memset (&pqos_list_out, 0x00, sizeof(pqos_list_out)) ;

        ret = moca_get_mac_addr(ctx, &mac_addr);

        if (ret == MOCA_API_SUCCESS)
        {
            pqos_list_in.ingr_node_id = 0xFFFFFFFF;
            pqos_list_in.ingr_node_mac = mac_addr.val;
            pqos_list_in.flow_start_index = MOCA_PQOS_LIST_FLOW_START_INDEX_DEF;
            pqos_list_in.flow_max_return = MOCA_PQOS_LIST_FLOW_MAX_RETURN_DEF;

            ret = moca_do_pqos_list(ctx, &pqos_list_in, &pqos_list_out);

            if (ret == MOCA_API_SUCCESS)
            {
                if (pqos_list_out.response_code == MOCA_L2_SUCCESS)
                {
                    if ((unsigned int)instance_id <= pqos_list_out.num_ret_flow_ids)
                    {
                        pqos_query_in = pqos_list_out.flowid[instance_id - 1];

                        ret = moca_do_pqos_query(ctx, pqos_query_in, &pqos_query_out);
                        if (ret == MOCA_API_SUCCESS && pqos_query_out.response_code == MOCA_L2_SUCCESS)
                        {
                            *value = pqos_query_out.lease_time;
                            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                        }
                    }
                }
                else
                {
                    DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqos_list_out.response_code, moca_l2_error_name(pqos_list_out.response_code)));
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

int moca_if_qos_flow_stats_lease_time_left(int instance_id, int *value)
{
    void *ctx;
    struct moca_interface_status ifstatus;
    struct moca_mac_addr mac_addr;
    struct moca_pqos_list_in pqos_list_in;
    struct moca_pqos_list_out pqos_list_out;
    macaddr_t pqos_query_in;
    struct moca_pqos_query_out pqos_query_out;
    int ret = 0;

    ctx = moca_open(NULL);

    if (ctx)
    {
        ret = moca_get_interface_status(ctx, &ifstatus);

        if (ret != MOCA_API_SUCCESS)
        {
            DBGPRINT((stderr, "error! interface failure\n"));
            moca_close(ctx);
            return(-1);
        }

        if (ifstatus.link_status != MOCA_LINK_UP)
        {
            DBGPRINT((stderr, "error! no link\n"));
            moca_close(ctx);
            return(-2);
        }

        memset (&pqos_list_in, 0x00, sizeof(pqos_list_in)) ;
        memset (&pqos_list_out, 0x00, sizeof(pqos_list_out)) ;

        ret = moca_get_mac_addr(ctx, &mac_addr);

        if (ret == MOCA_API_SUCCESS)
        {
            pqos_list_in.ingr_node_id = 0xFFFFFFFF;
            pqos_list_in.ingr_node_mac = mac_addr.val;
            pqos_list_in.flow_start_index = MOCA_PQOS_LIST_FLOW_START_INDEX_DEF;
            pqos_list_in.flow_max_return = MOCA_PQOS_LIST_FLOW_MAX_RETURN_DEF;

            ret = moca_do_pqos_list(ctx, &pqos_list_in, &pqos_list_out);

            if (ret == MOCA_API_SUCCESS)
            {
                if (pqos_list_out.response_code == MOCA_L2_SUCCESS)
                {
                    if ((unsigned int)instance_id <= pqos_list_out.num_ret_flow_ids)
                    {
                        pqos_query_in = pqos_list_out.flowid[instance_id - 1];

                        ret = moca_do_pqos_query(ctx, pqos_query_in, &pqos_query_out);
                        if (ret == MOCA_API_SUCCESS && pqos_query_out.response_code == MOCA_L2_SUCCESS)
                        {
                            *value = pqos_query_out.lease_time_left;
                            DBGPRINT((stdout, "%s: %d\n", __FUNCTION__, *value));
                        }
                    }
                }
                else
                {
                    DBGPRINT((stderr, "MoCA L2 response : 0x%x (%s)\n", pqos_list_out.response_code, moca_l2_error_name(pqos_list_out.response_code)));
                }
            }
            else
            {
                DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
            }
        }
        else
        {
            DBGPRINT((stderr, "%s: error %d\n", __FUNCTION__, ret));
        }

        moca_close(ctx);
    }
    else
    {
        DBGPRINT((stderr, "cannot connect to mocad.  Is it running?\n"));
    }

    return ret;
}

#if 0 /* NOT SUPPORT */
int moca_if_qos_flow_stats_flow_packets(int instance_id, int *value)
{
}
#endif
