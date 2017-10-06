/*
 * linuxsim USB support file
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id$
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <proto/ethernet.h>
#include <trxhdr.h>
#include <bcmnvram.h>
#include <osl.h>
#include <bcmendian.h>
#include <sbchipc.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include <dngl_stats.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <dngl_protocol.h>
#include <dngl_dbg.h>

typedef struct dngl {
	void *bus;			/* generic bus handle */
	osl_t *osh;
	si_t *sih;
	void *proto;
	dngl_stats_t stats;
	uint unit;			/* Device index */
	int medium;
	bool up;
	bool devopen;
	struct net_device *dev;
	char name[16];
} dngl_t;

int found = 0;

int dngl_msglevel = DNGL_ERROR;

extern void usbdev_connect(void);
extern void usbdev_selectcfg(uint cfg);

/* Call Broadcom wireless driver private ioctl */
int
_dngl_devioctl(struct dngl *dngl, uint32 cmd, void *buf, int len, int *used, int *needed, bool set)
{
	int ret = -1;

	/* buf must be 4-byte aligned */
	ASSERT(ISALIGNED(buf, 4));

	trace("ioctl %s cmd 0x%x, len %d", set ? "SET" : "QUERY", cmd, len);
	trace("status = %d/0x%x", ret, ret);

	return ret;
}

struct dngl *
BCMINITFN(dngl_attach)(struct dngl_bus *bus, void *drv, si_t *sih, osl_t *osh)
{
	struct dngl *dngl = NULL;

	trace("called");

	if (found >= 8) {
		err("too many units");
		goto fail;
	}

	if (!(dngl = MALLOC(osh, sizeof(dngl_t)))) {
		err("MALLOC failed");
		goto fail;
	}
	memset(dngl, 0, sizeof(dngl_t));
	dngl->bus = bus;
	dngl->osh = osh;
	dngl->sih = sih;
	dngl->unit = found;

	found++;

	return dngl;

fail:
	if (dngl)
		MFREE(osh, dngl, sizeof(dngl_t));
	return NULL;
}

void
BCMATTACHFN(dngl_detach)(struct dngl *dngl)
{
	osl_t *osh = dngl->osh;

	trace("%s", dngl->name);

	proto_detach(dngl->proto);

	MFREE(osh, dngl, sizeof(dngl_t));
}

/* Net interface running faster than USB. Flow-control the net interface */
void
dngl_txstop(struct dngl *dngl)
{
	trace("%s", dngl->name);
	/* overflow case just drops packets at the USB interface */
}

void
dngl_txstart(struct dngl *dngl)
{
	dbg("%s", dngl->name);
	/* overflow case just drops packets at the USB interface */
}

/* flow control called by dev receiving packets from us */
void
dngl_rxflowcontrol(struct dngl *dngl, bool state, int prio)
{
	trace("%s flowctl %s", dngl->name, state == ON ? "on" : "off");
	bus_ops->rxflowcontrol(dngl->bus, state, prio);
}

/* Transmit a stack packet onto the bus */
int
dngl_sendpkt(struct dngl *dngl, void *src, void *p)
{
	trace("%s", dngl->name);

	/* Push protocol-level header */
	if ((p = proto_pkt_header_push(dngl->proto, p))) {
		if (bus_ops->tx(dngl->bus, p)) {
			dngl->stats.tx_bytes += pkttotlen(dngl->osh, p);
			dngl->stats.tx_packets++;
		} else {
			dngl->stats.tx_dropped++;
			err("dropped pkt");
		}
	}
	return 0;
}

/* Transmit a stack packet onto the bus */
int
dngl_sendctl(struct dngl *dngl, void *src, void *p)
{
	return dngl_sendpkt(dngl, src, p);
}

/*
 * Send a packet received from the bus up the stack or queue
 * it for later transmission to the slave
 */
void
dngl_sendup(struct dngl *dngl, void *p)
{
	struct sk_buff *skb;

	trace("%s: pkt len %d", dngl->name, PKTLEN(dngl->osh, p));

	/* Pull protocol-level header */
	if (proto_pkt_header_pull(dngl->proto, p))
		return;

	/* drop if the interface is not up yet */
	if (!netif_device_present(dngl->dev)) {
		err("dngl_sendup: interface not ready");
		PKTFREE(dngl->osh, p, FALSE);
		return;
	}

	/* Convert the packet, mainly detach the pkttag */
	skb = PKTTONATIVE(dngl->osh, p);
	skb->dev = dngl->dev;
	skb->protocol = eth_type_trans(skb, dngl->dev);
	/* ASSERT(ISALIGNED(skb->data, 4)); */

	/* send it up */
	netif_rx(skb);

	dngl->stats.rx_packets++;
}

void
dngl_ctrldispatch(struct dngl *dngl, void *p, uchar *ext_buf)
{
	proto_ctrldispatch(dngl->proto, p, ext_buf);
}

void
dngl_resume(struct dngl *dngl)
{
	trace("%s", dngl->name);

	/* Do nothing */
}

void
dngl_suspend(struct dngl *dngl)
{
	trace("%s", dngl->name);
}

/* bind/enslave to the wireless device */
int
dngl_binddev(struct dngl *dngl, void *bus, void *dev, uint numslaves)
{
	if (dev == NULL) {
		err("dev is NULL");
		return -1;
	}

	dngl->dev = dev;
	dev_put(dev);

	if (!(dngl->proto = proto_attach(dngl->osh, dngl, dngl->bus, dev->name, FALSE))) {
		err("proto_attach failed");
		return -1;
	}

	return 0;
}

void
dngl_opendev(struct dngl *dngl)
{
	trace("%s", dngl->name);
	if (dngl->devopen)
		return;

	usbdev_connect();
	usbdev_selectcfg(1);

	dngl->devopen = TRUE;
}

/* Get device stats */
void
dngl_get_stats(struct dngl *dngl, dngl_stats_t *stats)
{
	trace("%s", dngl->name);
}

bool
dngl_get_netif_stats(struct dngl *dngl, dngl_stats_t *stats)
{
	int ret;

	trace("%s", dngl->name);
	bzero(stats, sizeof(dngl_stats_t));
	/* dngl_stats_t happens to mirror first 8 ulongs in linux net_device_stats */
	if ((ret = dngl_dev_ioctl(dngl, 0, stats, sizeof(dngl_stats_t))) < 0) {
		err("%s: error reading slave addr: %d", dngl->name, ret);
		return TRUE;
	} else
		return FALSE;
}

ulong
dngl_get_netif_mtu(struct dngl *dngl)
{
	uint32 val = 0;
	int ret;

	trace("%s", dngl->name);
	if ((ret = dngl_dev_ioctl(dngl, 1, &val, sizeof(val))) < 0)
		err("%s: error reading slave MTU: %d", dngl->name, ret);
	return val;
}

void
dngl_init(struct dngl *dngl)
{
	err("%s", dngl->name);

	/* check if init called w/o previous halt */
	if (dngl->up)
		return;

	dngl->up = TRUE;
}

void
dngl_halt(struct dngl *dngl)
{
	trace("%s", dngl->name);

	dngl->up = FALSE;

	/* Unregister event handler */
}

void
dngl_reset(struct dngl *dngl)
{
	trace("%s", dngl->name);

	/* purge any stale ctrl & intr packets */
	bus_ops->softreset(dngl->bus);
}

void *
dngl_proto(struct dngl *dngl)
{
	return dngl->proto;
}

void
dngl_keepalive(struct dngl *dngl, uint32 msec)
{
	/* set the watchdog for # msec */
	si_watchdog_ms(dngl->sih, msec);
}
