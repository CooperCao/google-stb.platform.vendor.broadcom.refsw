/*
 * Copyright 2011, Broadcom Corporation
 * All Rights Reserved.
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied or
 * duplicated in any form, in whole or in part, without the prior written
 * permission of Broadcom Corporation.
 *
 * $Id: dpsta_linux.c 704398 2017-06-13 10:58:33Z $
 */

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 14)
#include <linux/module.h>
#endif

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <proto/ethernet.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <dpsta_linux.h>
#include <dpsta.h>
#ifdef HNDCTF
#include <ctf/hndctf.h>
#endif /* HNDCTF */

#define	DPSTA_ERROR(fmt, args...)	printk(fmt, ##args)

#define PSTAFN_IS_DS_STA(pa, sa)	((pa)->is_ds_sta((pa)->psta, (sa)))
#define PSTAFN_BSS_AUTH(pa)		((pa)->bss_auth((pa)->bsscfg))
#define PSTAFN_PSTA_FIND(pa, sa)	((pa)->psta_find((pa)->psta, (sa)))

struct psta_if {
	uint8		name[IFNAMSIZ];		/* Slave or upstream ifname */
	struct net_device *dev;			/* Slave or upstream device */
	psta_if_api_t	api;			/* Slave psta interface api */
	void		*dpsta;			/* Dual proxy sta instance */
};

typedef struct dpsta_info {
	osl_t		*osh;			/* OS handle */
	uint32		unit;			/* Dual proxy sta interface */
	bool		registered;		/* TRUE when both interfaces register */
	bool		enabled;		/* DPSTA Enable/Disable status */
	uint32		policy;			/* Inband or Crossband repeating policy */
	uint32		lan_uif;		/* Upstream interface for lan traffic */
	spinlock_t	lock;			/* Lock for DPSTA dev */
	bool		dev_registered;		/* TRUE if DPSTA interface is registered */
	struct net_device *dev;			/* DPSTA device */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	struct net_device_stats stats;		/* DPSTA counters */
#endif /* KERNEL_VERSION < (2.6.36) */
						/* Slave or upstream interfaces */
	struct psta_if	pstaif[DPSTA_NUM_UPSTREAM_IF];
#ifdef HNDCTF
	ctf_t		*cih;			/* CTF instance handle */
#endif /* HNDCTF */
} dpsta_info_t;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
#define DPSTA_INFO(dev)		((dpsta_info_t *)netdev_priv(dev))
#else
#define DPSTA_INFO(dev)		((dpsta_info_t *)(dev)->priv)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static struct rtnl_link_stats64 *dpsta_get_stats64(struct net_device *dev,
	struct rtnl_link_stats64 *stats);
#else
static struct net_device_stats *dpsta_get_stats(struct net_device *dev);
#endif /* KERNEL_VERSION >= (2.6.36) */
#ifdef HAVE_NET_DEVICE_OPS
static int dpsta_set_mac_address(struct net_device *dev, void *addr);
#endif
static int32 dpsta_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
static int32 dpsta_start(struct sk_buff *skb, struct net_device *dev);
static int32 __init dpsta_init(void);
static void __exit dpsta_exit(void);

#ifdef HNDCTF
static uint32 dpsta_msg_level = 1;
#endif
static dpsta_info_t *g_dpsta;

#ifdef HAVE_NET_DEVICE_OPS
static const struct net_device_ops dpsta_netdev_ops = {
	.ndo_start_xmit = dpsta_start,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	.ndo_get_stats64 = dpsta_get_stats64,
#else
	.ndo_get_stats = dpsta_get_stats,
#endif /* KERNEL_VERSION >= (2.6.36) */
	.ndo_set_mac_address = dpsta_set_mac_address,
	.ndo_do_ioctl = dpsta_ioctl,
};
#endif /* HAVE_NET_DEVICE_OPS */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static struct rtnl_link_stats64*
dpsta_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats)
{
	dpsta_info_t *dpsta;
	int32 i;
	struct rtnl_link_stats64 *sstats, tmpstats;
	psta_if_t *pstaif;
	unsigned long flags;

	dpsta = DPSTA_INFO(dev);

	spin_lock_irqsave(&dpsta->lock, flags);

	for (i = 0; i < 2; i++) {
		pstaif = &dpsta->pstaif[i];

		if (pstaif->dev == NULL)
			continue;

#ifndef HAVE_NET_DEVICE_OPS
		sstats = pstaif->dev->get_stats(pstaif->dev);
#else
		sstats = &tmpstats;
		/*
		 * Driver implements either ndo_get_stats64 or ndo_get_stats.
		 * Calling dev_get_stats() instead of ndo_get_stats/ndo_get_stats64
		 * which takes case of both the cases.
		 */
		sstats = dev_get_stats(pstaif->dev, sstats);
#endif

		stats->rx_packets += sstats->rx_packets;
		stats->rx_bytes += sstats->rx_bytes;
		stats->rx_errors += sstats->rx_errors;

		stats->tx_packets += sstats->tx_packets;
		stats->tx_bytes += sstats->tx_bytes;
		stats->tx_errors += sstats->tx_errors;

		stats->collisions += sstats->collisions;

		stats->rx_length_errors += sstats->rx_length_errors;
		stats->rx_over_errors += sstats->rx_over_errors;
		stats->rx_crc_errors += sstats->rx_crc_errors;
		stats->rx_frame_errors += sstats->rx_frame_errors;
		stats->rx_fifo_errors += sstats->rx_fifo_errors;
		stats->rx_missed_errors += sstats->rx_missed_errors;

		stats->tx_fifo_errors += sstats->tx_fifo_errors;
	}

	spin_unlock_irqrestore(&dpsta->lock, flags);
	return stats;
}
#else
static struct net_device_stats *
dpsta_get_stats(struct net_device *dev)
{
	dpsta_info_t *dpsta;
	int32 i;
	struct net_device_stats *stats, *sstats;
	psta_if_t *pstaif;
	unsigned long flags;

	dpsta = DPSTA_INFO(dev);
	stats = &dpsta->stats;
	memset(&dpsta->stats, 0, sizeof(struct net_device_stats));

	spin_lock_irqsave(&dpsta->lock, flags);

	for (i = 0; i < 2; i++) {
		pstaif = &dpsta->pstaif[i];

		if (pstaif->dev == NULL)
			continue;

#ifndef HAVE_NET_DEVICE_OPS
		sstats = pstaif->dev->get_stats(pstaif->dev);
#else
		/* Linux 2.6.36 and up. */
		sstats = pstaif->dev->netdev_ops->ndo_get_stats(pstaif->dev);
#endif

		stats->rx_packets += sstats->rx_packets;
		stats->rx_bytes += sstats->rx_bytes;
		stats->rx_errors += sstats->rx_errors;

		stats->tx_packets += sstats->tx_packets;
		stats->tx_bytes += sstats->tx_bytes;
		stats->tx_errors += sstats->tx_errors;

		stats->collisions += sstats->collisions;

		stats->rx_length_errors += sstats->rx_length_errors;
		stats->rx_over_errors += sstats->rx_over_errors;
		stats->rx_crc_errors += sstats->rx_crc_errors;
		stats->rx_frame_errors += sstats->rx_frame_errors;
		stats->rx_fifo_errors += sstats->rx_fifo_errors;
		stats->rx_missed_errors += sstats->rx_missed_errors;

		stats->tx_fifo_errors += sstats->tx_fifo_errors;
	}

	spin_unlock_irqrestore(&dpsta->lock, flags);
	return stats;
}
#endif /* KERNEL_VERSION >= (2.6.36) */
/*
 * Handles dpsta configuration commands from user space.
 */
static int32
dpsta_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	dpsta_info_t *dpsta;
	dpsta_enable_info_t *u_info, k_info;
	struct net_device *tmp;

	if (!dev)
		return -ENETDOWN;

	dpsta = DPSTA_INFO(dev);

	if (dpsta == NULL)
		return -ENETDOWN;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	switch (cmd) {
	case DPSTA_CMD_ENABLE:
		u_info = (dpsta_enable_info_t *)ifr->ifr_data;
		if (copy_from_user(&k_info, u_info, sizeof(dpsta_enable_info_t)))
			return -EFAULT;

		/* Copy input parameters of enable command */
		dpsta->enabled = k_info.enable;
		dpsta->policy = k_info.policy;
		dpsta->lan_uif = k_info.lan_uif;
		strncpy(dpsta->pstaif[0].name, k_info.upstream_if[0], IFNAMSIZ);
		strncpy(dpsta->pstaif[1].name, k_info.upstream_if[1], IFNAMSIZ);

		if (!dpsta->enabled)
			break;

		/* Policy should be either sameband or crossband */
		if ((dpsta->policy != DPSTA_POLICY_SAMEBAND) &&
		    (dpsta->policy != DPSTA_POLICY_CROSSBAND) &&
		    (dpsta->policy != DPSTA_POLICY_AUTO)) {
			dpsta->enabled = FALSE;
			return -EINVAL;
		}

		/* LAN uif should be either 2G or 5G upstream interface or
		 * dynamic.
		 */
		if ((dpsta->lan_uif != DPSTA_LAN_UIF_2G) &&
		    (dpsta->lan_uif != DPSTA_LAN_UIF_5G) &&
		    (dpsta->lan_uif != DPSTA_LAN_UIF_AUTO) &&
		    (dpsta->lan_uif != DPSTA_LAN_UIF_AUTO_5G)) {
			dpsta->enabled = FALSE;
			return -EINVAL;
		}

		/* Save the netdevice addresses */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
		tmp = dev_get_by_name(&init_net, dpsta->pstaif[0].name);
#else
		tmp = dev_get_by_name(dpsta->pstaif[0].name);
#endif
		if (tmp == NULL) {
			dpsta->enabled = FALSE;
			return -ENODEV;
		}
		dpsta->pstaif[0].dev = tmp;

		dev_put(tmp);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
		tmp = dev_get_by_name(&init_net, dpsta->pstaif[1].name);
#else
		tmp = dev_get_by_name(dpsta->pstaif[1].name);
#endif
		if (tmp == NULL) {
			dpsta->enabled = FALSE;
			return -ENODEV;
		}
		dpsta->pstaif[1].dev = tmp;
		dev_put(tmp);

		break;
	default:
		return -EOPNOTSUPP;
	}

	return BCME_OK;
}

#ifdef HAVE_NET_DEVICE_OPS
static int
dpsta_set_mac_address(struct net_device *dev, void *addr)
{
	dpsta_info_t *dpsta;
	struct sockaddr *sa = (struct sockaddr *) addr;

	if (!dev)
		return -ENETDOWN;

	dpsta = DPSTA_INFO(dev);

	if (dpsta == NULL)
		return -ENETDOWN;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	bcopy(sa->sa_data, dev->dev_addr, ETHER_ADDR_LEN);

	return 0;
}
#endif /* HAVE_NET_DEVICE_OPS */

/*
 * Register function called when each upstream interface comes up in
 * psta mode. It exports the api for dpsta.
 */
psta_if_t *
dpsta_register(uint32 unit, psta_if_api_t *pi)
{
	dpsta_info_t *dpsta;

	if (pi == NULL) {
		DPSTA_ERROR("dpsta: %s: registering null proxy sta inst\n",
		            __FUNCTION__);
		return NULL;
	}

	/* Validate interface unit */
	if (unit >= DPSTA_NUM_UPSTREAM_IF) {
		DPSTA_ERROR("dpsta: %s: invalid psta if\n", __FUNCTION__);
		return NULL;
	}

	dpsta = g_dpsta;
	bcopy(pi, &dpsta->pstaif[unit].api, sizeof(psta_if_api_t));

	/* TRUE after both interfaces register */
	dpsta->registered = ((dpsta->pstaif[0].api.is_ds_sta != NULL) &&
	                     (dpsta->pstaif[1].api.is_ds_sta != NULL));

	return &dpsta->pstaif[unit];
}
#ifdef __CONFIG_STBAP__
EXPORT_SYMBOL(dpsta_register);
#endif /* __CONFIG_STBAP__ */
/*
 * Called by drivers when PSTA is enabled. This function is called before
 * sending the packet up to the stack. It gives us opportunity to perform
 * filtering.
 */
int32
dpsta_recv(void *p)
{
	struct sk_buff *skb = (struct sk_buff *)p;
	dpsta_info_t *dpsta;
	psta_if_api_t *pi;

	/* If we are not in dualband proxy sta mode then simply return
	 * error. Also return error if device is not our slave. Packet
	 * will continue its path up to bridge.
	 */
	dpsta = g_dpsta;
	ASSERT(dpsta != NULL);
	if (!dpsta->enabled)
		return BCME_OK;

	/* Not from registered slaves */
	if (!dpsta->registered)
		return BCME_OK;

	/* Filter the frames broadcasted back by bridge of upstream ap.
	 * Look if the received frame belongs to other interface's downstream
	 * clients. If so discard it, otherwise it must be from the upstream
	 * ap's distribution system.
	 */
	if (skb->dev == dpsta->pstaif[0].dev)
		pi = &dpsta->pstaif[1].api;
	else if (skb->dev == dpsta->pstaif[1].dev)
		pi = &dpsta->pstaif[0].api;
	else
		return BCME_OK;

	ASSERT(pi->psta_find != NULL);

	/* Discard bcmc frames that came back */
	if (PSTAFN_PSTA_FIND(pi, (skb->data + ETHER_SRC_OFFSET)))
		return BCME_ERROR;

	/* Fix device before sending up */
	skb->dev = dpsta->dev;

	return BCME_OK;
}
#ifdef __CONFIG_STBAP__
EXPORT_SYMBOL(dpsta_recv);
#endif /* __CONFIG_STBAP__ */
/*
 * Called by bridge layer in transmit direction. Route the frame to either
 * 2G or 5G upstream interface based on the configured policy. If in-band
 * forwarding policy is configured the frames are transmitted in same band.
 * If cross-band forwarding policy is configured frames are always forwarded
 * to opposite band.
 */
static int32
dpsta_start(struct sk_buff *skb, struct net_device *dev)
{
	dpsta_info_t *dpsta;
	psta_if_api_t *pa0, *pa1;
	struct ether_addr *sa;

	dpsta = DPSTA_INFO(dev);
	ASSERT(dpsta != NULL);
	ASSERT(dpsta == g_dpsta);

	if (!dpsta->enabled)
		return BCME_OK;

	/* Depending on the configured policy send the frame to appropriate
	 * upstream interface.
	 */
	sa = (struct ether_addr *)(skb->data + ETHER_SRC_OFFSET);
	pa0 = &dpsta->pstaif[0].api;
	pa1 = &dpsta->pstaif[1].api;

	/* Frames from apps directly attached to interfaces (nas, wps,...)
	 * are forwarded to respective interfaces.
	 */
	if (memcmp(dpsta->pstaif[0].dev->dev_addr, sa, ETHER_ADDR_LEN) == 0) {
		skb->dev = dpsta->pstaif[0].dev;
		goto devxmit;
	} else if (memcmp(dpsta->pstaif[1].dev->dev_addr, sa, ETHER_ADDR_LEN) == 0) {
		skb->dev = dpsta->pstaif[1].dev;
		goto devxmit;
	}

	/* Slaves haven't registered yet */
	if (!dpsta->registered) {
		PKTFRMNATIVE(dpsta->osh, skb);
		PKTFREE(dpsta->osh, skb, TRUE);
		return BCME_OK;
	}

	/* Try sending downstream wireless traffic on same band. If uplink
	 * connection is down try the other band.
	 */
	if (PSTAFN_IS_DS_STA(pa0, sa)) {
		if (dpsta->policy == DPSTA_POLICY_AUTO)
			skb->dev = PSTAFN_BSS_AUTH(pa0) ?
			           dpsta->pstaif[0].dev :
			           dpsta->pstaif[1].dev;
		else
			skb->dev = (dpsta->policy == DPSTA_POLICY_SAMEBAND ?
			            dpsta->pstaif[0].dev : dpsta->pstaif[1].dev);
	} else if (PSTAFN_IS_DS_STA(pa1, sa)) {
		if (dpsta->policy == DPSTA_POLICY_AUTO)
			skb->dev = PSTAFN_BSS_AUTH(pa1) ?
			           dpsta->pstaif[1].dev :
			           dpsta->pstaif[0].dev;
		else
			skb->dev = (dpsta->policy == DPSTA_POLICY_SAMEBAND ?
			            dpsta->pstaif[1].dev : dpsta->pstaif[0].dev);
	} else {
		bool wl0_auth, wl1_auth;

		/* Host traffic will use 2G if both interfaces are connected.
		 * Otherwise it will use whichever interface is connected.
		 */
		if (memcmp(dpsta->dev->dev_addr, sa, ETHER_ADDR_LEN) == 0)
			goto lanuif_auto;

		/* Upstream interface to use for LAN traffic */
		if (dpsta->lan_uif == DPSTA_LAN_UIF_2G)
			skb->dev = dpsta->pstaif[0].dev;
		else if (dpsta->lan_uif == DPSTA_LAN_UIF_5G)
			skb->dev = dpsta->pstaif[1].dev;
		else {
			/* Frame from LAN, if 2G interface is connected send over it.
			 * Otherwise send over 5G. If both are connected just use 2G.
			 */
lanuif_auto:
			wl0_auth = PSTAFN_BSS_AUTH(pa0);
			wl1_auth = PSTAFN_BSS_AUTH(pa1);

			if (wl0_auth && wl1_auth) {
				if (dpsta->lan_uif == DPSTA_LAN_UIF_AUTO_5G)
					skb->dev = dpsta->pstaif[1].dev;
				else
					skb->dev = dpsta->pstaif[0].dev;
			}
			else if (wl0_auth)
				skb->dev = dpsta->pstaif[0].dev;
			else if (wl1_auth)
				skb->dev = dpsta->pstaif[1].dev;
			else {
				PKTFRMNATIVE(dpsta->osh, skb);
				PKTFREE(dpsta->osh, skb, TRUE);
				return BCME_OK;
			}
		}
	}

devxmit:
	dev_queue_xmit(skb);
	return BCME_OK;
}

static int32 __init
dpsta_init(void)
{
	dpsta_info_t *dpsta;
	struct net_device *dev;
	uint8 name[IFNAMSIZ] = "dpsta";
	osl_t *osh;

	osh = osl_attach(NULL, PCI_BUS, FALSE);
	ASSERT(osh);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29))
	if (!(dpsta = MALLOC(osh, sizeof(dpsta_info_t)))) {
		DPSTA_ERROR("dpsta: %s: out of memory\n", __FUNCTION__);
		return -ENOMEM;
	}
	bzero(dpsta, sizeof(dpsta_info_t));
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
	if (!(dev = MALLOC(osh, sizeof(struct net_device)))) {
		MFREE(osh, dpsta, sizeof(dpsta_info_t));
		DPSTA_ERROR("dpsta: %s: out of memory\n", __FUNCTION__);
		return -ENOMEM;
	}
	bzero(dev, sizeof(struct net_device));
	ether_setup(dev);
	strncpy(dev->name, name, IFNAMSIZ);
#else
	/* Allocate net device, including space for private structure */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
        dev = alloc_netdev(sizeof(dpsta_info_t), name, NET_NAME_ENUM, ether_setup);
#else
        dev = alloc_netdev(sizeof(dpsta_info_t), name, ether_setup);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0) */
	dpsta = netdev_priv(dev);
	if (!dev) {
#else
	dev = alloc_netdev(0, name, ether_setup);
	if (!dev) {
		MFREE(osh, dpsta, sizeof(dpsta_info_t));
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29) */
		DPSTA_ERROR("dpsta: %s: out of memory, alloc_netdev\n", __FUNCTION__);
		return -ENOMEM;
	}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22) */

	dpsta->osh = osh;
	dpsta->dev = dev;
	dpsta->unit = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29)
	dev->priv = dpsta;
#endif

	/* Virtual device, no queueing */
	dev->tx_queue_len = 0;

	/* Initialize dev fn pointers */
#ifndef HAVE_NET_DEVICE_OPS
	dev->hard_start_xmit = dpsta_start;
	dev->do_ioctl = dpsta_ioctl;
	dev->get_stats = dpsta_get_stats;
#else
	/* Linux 2.6.36 and up. */
	dev->netdev_ops = &dpsta_netdev_ops;
#endif /* HAVE_NET_DEVICE_OPS */

	if (register_netdev(dev)) {
		DPSTA_ERROR("dpsta: %s, register_netdev failed for \"%s\"\n",
		            name, __FUNCTION__);
		free_netdev(dev);
		MFREE(osh, dpsta, sizeof(dpsta_info_t));
		return -EIO;
	} else
		dpsta->dev_registered = TRUE;

	dpsta->pstaif[0].dpsta = dpsta;
	dpsta->pstaif[1].dpsta = dpsta;

#ifdef HNDCTF
	dpsta->cih = ctf_attach(osh, dev->name, &dpsta_msg_level, NULL, NULL);

	if ((ctf_dev_register(dpsta->cih, dev, FALSE) != BCME_OK) ||
	    (ctf_enable(dpsta->cih, dev, TRUE, NULL) != BCME_OK)) {
		DPSTA_ERROR("dpsta: ctf_dev_register() failed\n");
		free_netdev(dev);
		MFREE(osh, dpsta, sizeof(dpsta_info_t));
		return -ENOMEM;
	}
#endif /* HNDCTF */


	g_dpsta = dpsta;

	return 0;
}

static void __exit
dpsta_exit(void)
{
	dpsta_info_t *dpsta;

	dpsta = g_dpsta;

#ifdef HNDCTF
	if (dpsta->cih) {
		ctf_dev_unregister(dpsta->cih, dpsta->dev);
		ctf_detach(dpsta->cih);
	}
#endif /* HNDCTF */

	free_netdev(dpsta->dev);
	MFREE(dpsta->osh, dpsta, sizeof(dpsta_info_t));
}

module_init(dpsta_init);
module_exit(dpsta_exit);
