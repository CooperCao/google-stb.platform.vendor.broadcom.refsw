/***************************************************************************
 *     Copyright (c) 2006-2008, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description: stub file for net dma functions which now reside in netaccel driver
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/skbuff.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/inet.h>
#include <asm/atomic.h>
#include <net/tcp.h>
#include <net/udp.h>
#include "bsettop_ip.h"
#include "netif_dma.h"

#define BSTD_UNUSED(x) ((void)x)
bnetif_dma_cb_t netif_cb;

int bcm_netif_sethook(bnetif_dma_cb_t *hook)
{
	if (hook)
    	memcpy(&netif_cb, hook, sizeof(bnetif_dma_cb_t));
	else
    	memset(&netif_cb, 0, sizeof(bnetif_dma_cb_t));
    return 0;
}

int 
bnetif_dma_data(unsigned *puser_len, struct bnetif_user_block *user, unsigned net_len, const struct bnetif_net_block *net, unsigned *noverflows)
{
    if (netif_cb.bnetif_dma_data)
        return netif_cb.bnetif_dma_data(puser_len, user, net_len, net, noverflows);
    else
        printk("*********%s() Error: Please load the Net DMA Driver for IP Support*********\n", __FUNCTION__);
	return -1;
}

struct bnetif_filter *
bnetif_dma_add_filter(struct bnetif_dma_user_filter *desc, struct bnetif_dma_user_cookie *cookie)
{
    if (netif_cb.bnetif_dma_add_filter)
        return netif_cb.bnetif_dma_add_filter(desc, cookie);
    else
        printk("*********%s() Error: Please load the Net DMA Driver for IP Support*********\n", __FUNCTION__);
	return 0;
}

int
bnetif_dma_delete_filter(struct bnetif_filter *filter)
{
    BSTD_UNUSED(filter);
    if (netif_cb.bnetif_dma_delete_filter)
        return netif_cb.bnetif_dma_delete_filter(filter);
    else
        printk("*********%s() Error: Please load the Net DMA Driver for IP Support*********\n", __FUNCTION__);
	return -1;
}

int 
init_bnetif_dma(void)
{
	return 0;
}

void 
cleanup_bnetif_dma(void)
{
	return;
}

int 
bnetif_ioctl(struct inode *inode, struct file * file, unsigned int cmd, unsigned long arg)
{
    if (netif_cb.bnetif_ioctl)
        return netif_cb.bnetif_ioctl(inode, file, cmd, arg);
    else
        printk("*********%s() Error: Please load the Net DMA Driver for IP Support*********\n", __FUNCTION__);
	return 0;
}

void 
bnetif_dma_stop(void)
{
    if (netif_cb.bnetif_dma_stop)
        return netif_cb.bnetif_dma_stop();
    else
        printk("*********%s() Error: Please load the Net DMA Driver for IP Support*********\n", __FUNCTION__);
}

EXPORT_SYMBOL(bcm_netif_sethook);
