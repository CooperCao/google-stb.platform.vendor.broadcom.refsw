/*
 * Broadcom CDC/wl USB Linux host driver
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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/random.h>
#include <linux/spinlock.h>
#include <linux/ethtool.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <bcmcdc.h>
#include <bcmendian.h>
#include <epivers.h>
#include <linuxver.h>
#include <proto/bcmevent.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#define KERNEL26
#define USB_ALLOC_URB()		usb_alloc_urb(0, GFP_ATOMIC)
#define USB_SUBMIT_URB(urb)	usb_submit_urb(urb, GFP_ATOMIC)
#define USB_UNLINK_URB(urb)	usb_kill_urb(urb)
#define USB_BUFFER_ALLOC(dev, size, mem, dma) \
				usb_buffer_alloc(dev, size, mem, dma)
#define USB_BUFFER_FREE(dev, size, data, dma) \
				usb_buffer_free(dev, size, data, dma)
#define USB_QUEUE_BULK		0
#define CALLBACK_ARGS		struct urb *urb, struct pt_regs *regs
#define CONFIGDESC(usb)		(&((usb)->actconfig)->desc)
#define IFPTR(usb, idx)		((usb)->actconfig->interface[idx])
#define IFALTS(usb, idx)	(IFPTR((usb), (idx))->altsetting[0])
#define IFDESC(usb, idx)	IFALTS((usb), (idx)).desc
#define IFEPDESC(usb, idx, ep)	(IFALTS((usb), (idx)).endpoint[ep]).desc
#define ALLOC_ETHERDEV()	alloc_etherdev(0)
#define FREE_NETDEV(net)	free_netdev(net)
#define REGISTER_NETDEV(dev)	register_netdev(dev)
#define INTERVAL_DECODE(usb, interval) \
				(interval)	/* usb_fill_int_urb decodes interval in 2.6 */
#else /* 2.4 */
#define USB_ALLOC_URB()		usb_alloc_urb(0)
#define USB_SUBMIT_URB(urb)	usb_submit_urb(urb)
#define USB_UNLINK_URB(urb)	usb_unlink_urb(urb)
#define USB_BUFFER_ALLOC(dev, size, mem, dma) \
				kmalloc(size, mem)
#define USB_BUFFER_FREE(dev, size, data, dma) \
				kfree(data)
#define CALLBACK_ARGS		struct urb *urb
#define CONFIGDESC(usb)		((usb)->actconfig)
#define IFPTR(usb, idx)		(&(usb)->actconfig->interface[idx])
#define IFALTS(usb, idx)	((usb)->actconfig->interface[idx].altsetting[0])
#define IFDESC(usb, idx)	IFALTS((usb), (idx))
#define IFEPDESC(usb, idx, ep)	(IFALTS((usb), (idx)).endpoint[ep])
#define REGISTER_NETDEV(dev)	0
#define ALLOC_ETHERDEV()	init_etherdev(NULL, 0)
#define FREE_NETDEV(net)
#define INTERVAL_DECODE(usb, interval) \
			(((usb)->speed == USB_SPEED_HIGH) ? (1 << ((interval) - 1)) : (interval))
#endif /* 2.4 */

#define trace(format, arg...)	dbg(format, ## arg)
#define SHOW_EVENTS 1

#define RX_QLEN			10	/* rx bulk queue length */
#define TX_QLEN			10	/* tx bulk queue length */
#define TRIES 			2	/* # of tries for submitting ctrl reads & writes */

/* Private data kept in skb */
#define SKB_PRIV(skb, idx)	(&((void **)skb->cb)[idx])
#define SKB_PRIV_URB(skb)	(*(struct urb **)SKB_PRIV(skb, 0))

typedef struct {
	uint32 notification;
	uint32 reserved;
} intr_t;

#define CONTROL_IF		0
#define BULK_IF			0

typedef struct {
	struct net_device *net;
	struct net_device_stats stats;
	int up;

	struct usb_device *usb;
	struct urb *intr_urb;
	struct urb *urb_freelist;
	spinlock_t urb_freelist_lock;
	uint rx_pipe, tx_pipe, intr_pipe;
	struct sk_buff_head rxq, txq;
	wait_queue_head_t intr_wait;
#ifdef KERNEL26
	dma_addr_t hintr;
	dma_addr_t hmsg;
#endif
	intr_t intr;
	int intr_size;
	int interval;
	uint16 reqid;
	struct semaphore sem;
	spinlock_t lock;
	cdc_ioctl_t *msg;
} cdc_t;

static void cdc_tx_complete(CALLBACK_ARGS);
static void cdc_rx_complete(CALLBACK_ARGS);
static int cdc_rx_submit(cdc_t *cdc);
static int wl_host_event(struct sk_buff *skb);
static void cdc_unlink(struct sk_buff_head *q);
#ifdef KERNEL26
static int cdc_probe(struct usb_interface *intf,
                     const struct usb_device_id *id);
static void cdc_disconnect(struct usb_interface *intf);
#else
static void *cdc_probe(struct usb_device *usb, unsigned int ifnum,
                       const struct usb_device_id *id);
static void cdc_disconnect(struct usb_device *usb, void *ptr);
#endif

static int cdc_toe_get(cdc_t *cdc, uint32 *toe_ol);
static int cdc_toe_set(cdc_t *cdc, uint32 toe_ol);

static struct usb_device_id cdc_table[];
static struct usb_driver cdc_driver;

static struct urb *
cdc_urb_get(cdc_t *cdc)
{
	unsigned long flags;
	struct urb *urb;

	spin_lock_irqsave(&cdc->urb_freelist_lock, flags);

	if ((urb = cdc->urb_freelist) != NULL)
		cdc->urb_freelist = urb->context;		/* context is next pointer */

	spin_unlock_irqrestore(&cdc->urb_freelist_lock, flags);

	return urb;
}

static void
cdc_urb_put(cdc_t *cdc, struct urb *urb)
{
	unsigned long flags;

	spin_lock_irqsave(&cdc->urb_freelist_lock, flags);

	urb->context = cdc->urb_freelist;			/* context is next pointer */
	cdc->urb_freelist = urb;

	spin_unlock_irqrestore(&cdc->urb_freelist_lock, flags);
}

static void
cdc_tx_complete(CALLBACK_ARGS)
{
	struct sk_buff *skb = (struct sk_buff *)urb->context;
	struct net_device *net = skb->dev;
	cdc_t *cdc = net->priv;
	unsigned long flags;

	trace("cdc_tx_complete");

	if (!netif_device_present(net)) {
		err("cdc_tx_complete: no net device");
		return;
	}

	spin_lock_irqsave(&cdc->txq.lock, flags);

	/* Remove from queue */
	__skb_unlink(skb, &cdc->txq);

	/* Flow control */
	if (cdc->up && netif_queue_stopped(net) && (skb_queue_len(&cdc->txq) < TX_QLEN))
		netif_wake_queue(net);

	spin_unlock_irqrestore(&cdc->txq.lock, flags);

	if (urb->status) {
		cdc->stats.tx_errors++;
		err("%s: tx error %d", net->name, urb->status);
	} else {
		cdc->stats.tx_packets++;
		cdc->stats.tx_bytes += skb->len;
	}

	cdc_urb_put(cdc, urb);
	dev_kfree_skb_any(skb);
}

static int
cdc_start_xmit(struct sk_buff *skb, struct net_device *net)
{
	cdc_t *cdc = net->priv;
	struct urb *urb;
	int ret;
	unsigned long flags;
	struct bdc_header *h;

	trace("cdc_start_xmit");

	/* Push BDC protocol header */
	if (skb_headroom(skb) < BDC_HEADER_LEN) {
		struct sk_buff *skb2;

		skb2 = skb_realloc_headroom(skb, BDC_HEADER_LEN);
		dev_kfree_skb(skb);
		if ((skb = skb2) == NULL) {
			err("%s: skb_realloc_headroom failed\n", net->name);
			cdc->stats.tx_dropped++;
			return -ENOMEM;
		}
	}

	h = (struct bdc_header *)skb_push(skb, BDC_HEADER_LEN);

	/* Tell device to do checksum if Linux has not done it */
	h->flags = (BDC_PROTO_VER << BDC_FLAG_VER_SHIFT);
	if (skb->ip_summed == CHECKSUM_HW)
		h->flags |= BDC_FLAG_SUM_NEEDED;
	h->priority = (skb->priority & BDC_PRIORITY_MASK);
	h->flags2 = 0;
	h->pad = 0;

	dbg("%s: send: BDC header %02x %02x %02x %02x\n",
	    net->name, h->flags, h->priority, h->flags2, h->pad);

	/* Allocate URB */
	if ((urb = cdc_urb_get(cdc)) == NULL) {
		err("%s: out of URBs for tx", cdc->net->name);
		dev_kfree_skb(skb);
		cdc->stats.tx_dropped++;
		return -ENOMEM;
	}

	/* Save pointer to URB in skb */
	SKB_PRIV_URB(skb) = urb;

	usb_fill_bulk_urb(urb, cdc->usb, cdc->tx_pipe,
	                  skb->data, skb->len, cdc_tx_complete, skb);
	urb->transfer_flags |= USB_QUEUE_BULK;

	/* Send and enqueue atomically */
	spin_lock_irqsave(&cdc->txq.lock, flags);

	if ((ret = USB_SUBMIT_URB(urb))) {
		err("%s: usb_submit_urb tx failed with status %d", net->name, ret);
		spin_unlock_irqrestore(&cdc->txq.lock, flags);
		cdc_urb_put(cdc, urb);
		dev_kfree_skb(skb);
		cdc->stats.tx_dropped++;
		return ret;
	}

	/* Enqueue packet to wait for completion */
	__skb_queue_tail(&cdc->txq, skb);

	/* Flow control */
	if (skb_queue_len(&cdc->txq) >= TX_QLEN)
		netif_stop_queue(net);

	spin_unlock_irqrestore(&cdc->txq.lock, flags);

	/* We've eaten the skb */
	return 0;
}

static void
cdc_rx_complete(CALLBACK_ARGS)
{
	struct sk_buff *skb = (struct sk_buff *)urb->context;
	struct net_device *net = skb->dev;
	cdc_t *cdc = net->priv;
	unsigned long flags;
	struct bdc_header *h;

	trace("cdc_rx_complete");

	if (!netif_device_present(net)) {
		err("cdc_rx_complete: no net device");
		return;
	}

	/* Remove from queue */
	spin_lock_irqsave(&cdc->rxq.lock, flags);
	__skb_unlink(skb, &cdc->rxq);
	spin_unlock_irqrestore(&cdc->rxq.lock, flags);

	/* Handle errors */
	if (urb->status) {
		/*
		 * Linux 2.4 disconnect: -ENOENT or -EILSEQ for CRC error; rmmod: -ENOENT
		 * Linux 2.6 disconnect: -EPROTO; rmmod: -ESHUTDOWN
		 */
		if (urb->status == -ENOENT || urb->status == -ESHUTDOWN || urb->status == -EPROTO)
			cdc->up = 0;
		else {
			err("%s: rx error %d", net->name, urb->status);
			cdc->stats.rx_errors++;
		}
		dev_kfree_skb_any(skb);
		cdc_urb_put(cdc, urb);
		/* On error, don't submit more URBs yet */
		return;
	}

	skb->len = urb->actual_length;

	/* Parse/remove BDC protocol header */
	if (skb->len < BDC_HEADER_LEN) {
		err("%s: runt length %d", net->name, skb->len);
		dev_kfree_skb_any(skb);
		goto done;
	}

	h = (struct bdc_header *)skb->data;

	dbg("%s: recv: BDC header %02x %02x %02x %02x\n",
	    net->name, h->flags, h->priority, h->flags2, h->pad);

	if (((h->flags & BDC_FLAG_VER_MASK) >> BDC_FLAG_VER_SHIFT) != BDC_PROTO_VER) {
		err("%s: non-BDC packet received, flags 0x%x", net->name, h->flags);
		dev_kfree_skb_any(skb);
		goto done;
	}

	skb->priority = (h->priority & BDC_PRIORITY_MASK);

	if (h->flags & BDC_FLAG_SUM_GOOD) {
		dbg("%s: BDC packet received with good rx-csum, flags 0x%x", net->name, h->flags);
		skb->ip_summed = CHECKSUM_UNNECESSARY;
	}

	skb_pull(skb, BDC_HEADER_LEN);

	/* Parse/remove Ethernet protocol header */
	if (skb->len < ETH_HLEN) {
		err("%s: runt length %d", net->name, skb->len);
		dev_kfree_skb_any(skb);
		goto done;
	}

	skb->protocol = eth_type_trans(skb, net);

	/* Process special event packets and then discard them */
	if (ntoh16(skb->protocol) == ETHER_TYPE_BRCM && wl_host_event(skb)) {
		dev_kfree_skb_any(skb);
		goto done;
	}

	net->last_rx = jiffies;

	cdc->stats.rx_packets++;
	cdc->stats.rx_bytes += skb->len;

	netif_rx(skb);

done:
	cdc_urb_put(cdc, urb);

	if (cdc->up)
		cdc_rx_submit(cdc);
}

static int
cdc_rx_submit(cdc_t *cdc)
{
	int len = cdc->net->hard_header_len + cdc->net->mtu;
	struct urb *urb;
	struct sk_buff *skb;
	int ret = 0;
	unsigned long flags;

	trace("cdc_rx_submit");

	spin_lock_irqsave(&cdc->rxq.lock, flags);

	while (skb_queue_len(&cdc->rxq) < RX_QLEN) {
		/* Allocate URB for this packet */
		if ((urb = cdc_urb_get(cdc)) == NULL) {
			err("%s: out of URBs for rx", cdc->net->name);
			ret = -ENOMEM;
			break;
		}

		/* Allocate a packet buffer */
		if (!(skb = dev_alloc_skb(len))) {
			err("%s: dev_alloc_skb failed; len %d", cdc->net->name, len);
			cdc_urb_put(cdc, urb);
			ret = -ENOMEM;
			break;
		}

		skb->dev = cdc->net;

		/* Ensure headroom for BDC protocol header */
		skb_reserve(skb, BDC_HEADER_LEN);

		/* Save pointer to URB */
		SKB_PRIV_URB(skb) = urb;

		usb_fill_bulk_urb(urb, cdc->usb, cdc->rx_pipe,
		                  skb->data, len, cdc_rx_complete, skb);
		urb->transfer_flags |= USB_QUEUE_BULK;

		if ((ret = USB_SUBMIT_URB(urb))) {
			err("%s: usb_submit_urb rx failed with status %d", cdc->net->name, ret);
			/* 2.6 fails out with -EMSGSIZE if disconnect in progress */
			cdc_urb_put(cdc, urb);
			dev_kfree_skb_any(skb);
			break;
		}

		/* Enqueue packet */
		__skb_queue_tail(&cdc->rxq, skb);
	}

	spin_unlock_irqrestore(&cdc->rxq.lock, flags);

	return ret;
}

static void
cdc_intr_complete(CALLBACK_ARGS)
{
	cdc_t *cdc = urb->context;
#ifdef KERNEL26
	int ret;
#endif

	trace("cdc_intr_complete");

	if (waitqueue_active(&cdc->intr_wait))
		wake_up_interruptible(&cdc->intr_wait);

#ifdef KERNEL26
	/* Resubmit every time (2.6 only) */
	if (cdc->up && (ret = USB_SUBMIT_URB(cdc->intr_urb)))
		err("%s: intr usb_submit_urb failed with status %d", cdc->net->name, ret);
#endif
}

int
cdc_cmplt_getpkt(cdc_t *cdc, uint32 id, uint32 len)
{
	int ret, try;
	int to = cdc->msg->cmd == WLC_SET_SROM ? 8 * HZ : HZ / TRIES;

	for (try = 0; try < TRIES; try++) {
		/* Get response */
		ret = usb_control_msg(cdc->usb, usb_rcvctrlpipe(cdc->usb, 0),
		                      1, USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		                      cpu_to_le16(0),
		                      cpu_to_le16(IFDESC(cdc->usb, CONTROL_IF).bInterfaceNumber),
		                      cdc->msg, len + sizeof(cdc_ioctl_t), to);
		if (ret != -ETIMEDOUT)
			break;
	}

	if (ret < 0)
		err("%s: %s: usb_control_msg failed with status %d",
		    cdc->net->name, __FUNCTION__, ret);

	return ret;
}

static int
cdc_cmplt(cdc_t *cdc, uint32 id, uint32 len)
{
	DECLARE_WAITQUEUE(wait, current);
	int timeout = HZ, ret = 0, attempt = 0;
	uint32 msg_id, msg_flags;
	uint32 intr = 0;
	unsigned long flags;

	trace("cdc_cmplt");

	do {
		/* Wait for interrupt */
		add_wait_queue(&cdc->intr_wait, &wait);
		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock_irqsave(&cdc->lock, flags);
		while (cdc->up &&
		       !(intr = le32_to_cpu(cdc->intr.notification)) &&
		       !signal_pending(current) && timeout) {
			spin_unlock_irqrestore(&cdc->lock, flags);
			timeout = schedule_timeout(timeout);
			spin_lock_irqsave(&cdc->lock, flags);
		}
		if (!cdc->up) {
			spin_unlock_irqrestore(&cdc->lock, flags);
			err("cdc_cmplt: !cdc->up");
			ret = -ENXIO;
			break;
		}
		bzero(&cdc->intr, sizeof(cdc->intr));
		spin_unlock_irqrestore(&cdc->lock, flags);
		remove_wait_queue(&cdc->intr_wait, &wait);
		set_current_state(TASK_RUNNING);

		if (intr)
			dbg("%s: interrupt", cdc->net->name);
		else if (timeout == 0)
			dbg("%s: timeout", cdc->net->name);
		else if (signal_pending(current)) {
			err("%s: cancelled", cdc->net->name);
			return -ERESTARTSYS;
		}

		ret = cdc_cmplt_getpkt(cdc, id, len);

		msg_flags = le32_to_cpu(cdc->msg->flags);
		msg_id = (msg_flags & CDCF_IOC_ID_MASK) >> CDCF_IOC_ID_SHIFT;
	} while ((msg_id != id) && timeout && (attempt++ < 10));

	return ret;
}

static int
cdc_msg(cdc_t *cdc)
{
	int ret, try;
	int len = le32_to_cpu(cdc->msg->len + sizeof(cdc_ioctl_t));

	/*
	 * NOTE: cdc->msg->len holds the desired length of the buffer to be
	 * returned.  Only up to CDC_MAX_MSG_SIZE of this buffer area
	 * is actually sent to the dongle.
	 */

	if (len > CDC_MAX_MSG_SIZE)
		len = CDC_MAX_MSG_SIZE;

	trace("cdc_msg");

	for (try = 0; try < TRIES; try++) {
		/* Send request */
		ret = usb_control_msg(cdc->usb, usb_sndctrlpipe(cdc->usb, 0),
		                      0, USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		                      cpu_to_le16(0),
		                      cpu_to_le16(IFDESC(cdc->usb, CONTROL_IF).bInterfaceNumber),
		                      cdc->msg, len, HZ / TRIES);
		if (ret != -ETIMEDOUT)
			break;
	}

	if (ret < 0)
		err("%s: %s: usb_control_msg failed with status %d",
		    cdc->net->name, __FUNCTION__, ret);

	return ret;
}

static int
cdc_query_ioctl(cdc_t *cdc, uint cmd, void *buf, uint len)
{
	cdc_ioctl_t *msg = cdc->msg;
	void *info;
	int ret = 0, try;
	uint32 flags = 0, id = 0;

	trace("cdc_query_ioctl");

	down(&cdc->sem);

	memset(msg, 0, sizeof(cdc_ioctl_t));

	msg->cmd = cpu_to_le32(cmd);
	msg->len = cpu_to_le32(len);
	flags = (++cdc->reqid << CDCF_IOC_ID_SHIFT);
	msg->flags = cpu_to_le32(flags);

	if (buf)
		memcpy(&msg[1], buf, len);

	if ((ret = cdc_msg(cdc)) < 0)
		goto done;

	for (try = 0; try < TRIES; try++) {
		/* wait for interrupt and get first fragment */
		if ((ret = cdc_cmplt(cdc, cdc->reqid, len)) < 0)
			goto done;

		flags = le32_to_cpu(msg->flags);
		id = (flags & CDCF_IOC_ID_MASK) >> CDCF_IOC_ID_SHIFT;

		if (id == cdc->reqid)
			break;
	}

	if (id != cdc->reqid) {
		err("%s: unexpected request id %d (expected %d)", cdc->net->name, id, cdc->reqid);
		ret = -EINVAL;
		goto done;
	}

	/* Check info buffer */
	info = (void*)&msg[1];

	/* Copy info buffer */
	if (buf)
		memcpy(buf, info, len);

	/* Check the ERROR flag */
	if (flags & CDCF_IOC_ERROR) {
		err("%s: query ioctl(0x%x) error on dongle", cdc->net->name, cmd);
		ret = -EIO;	/* Note: EIO reserved exclusively for dongle side errors */
	}
done:
	up(&cdc->sem);
	return ret;
}

static int
cdc_set_ioctl(cdc_t *cdc, uint cmd, void *buf, uint len)
{
	cdc_ioctl_t *msg = cdc->msg;
	int ret = 0;
	uint32 flags, id;

	trace("cdc_set_ioctl");

	down(&cdc->sem);

	memset(msg, 0, sizeof(cdc_ioctl_t));

	msg->cmd = cpu_to_le32(cmd);
	msg->len = cpu_to_le32(len);
	flags = (++cdc->reqid << CDCF_IOC_ID_SHIFT);
	msg->flags |= cpu_to_le32(flags);

	if (buf)
		memcpy((void *)(&msg[1]), buf, len);

	if ((ret = cdc_msg(cdc)) < 0)
		goto done;

	if ((ret = cdc_cmplt(cdc, cdc->reqid, len)) < 0)
		goto done;

	flags = le32_to_cpu(msg->flags);
	id = (flags & CDCF_IOC_ID_MASK) >> CDCF_IOC_ID_SHIFT;

	if (id != cdc->reqid) {
		err("%s: unexpected request id %d (expected %d)", cdc->net->name,
		    id, cdc->reqid);
		ret = -EINVAL;
		goto done;
	}

	/* Check the ERROR flag */
	if (flags & CDCF_IOC_ERROR) {
		err("%s: set ioctl(0x%x) error on dongle", cdc->net->name, cmd);
		ret = -EIO;	/* Note: EIO reserved exclusively for dongle side errors */
	}

done:
	up(&cdc->sem);
	return ret;
}

static struct net_device_stats *
cdc_get_stats(struct net_device *net)
{
	cdc_t *cdc = net->priv;

	trace("cdc_get_stats");

	return &cdc->stats;
}

/* Retrieve current toe component enables, which are kept as a bitmap in toe_ol iovar */
static int
cdc_toe_get(cdc_t *cdc, uint32 *toe_ol)
{
	char buf[32];
	int ret;

	strcpy(buf, "toe_ol");
	if ((ret = cdc_query_ioctl(cdc, WLC_GET_VAR, buf, sizeof(buf))) < 0) {
		/* Check for older dongle image that doesn't support toe_ol */
		if (ret == -EIO) {
			err("%s: toe not supported by device", cdc->net->name);
			return -EOPNOTSUPP;
		}

		err("%s: could not get toe_ol: ret=%d", cdc->net->name, ret);
		return ret;
	}

	memcpy(toe_ol, buf, sizeof(uint32));
	return 0;
}

/* Set current toe component enables in toe_ol iovar, and set toe global enable iovar */
static int
cdc_toe_set(cdc_t *cdc, uint32 toe_ol)
{
	char buf[32];
	int toe, ret;

	strcpy(buf, "toe_ol");
	memcpy(&buf[sizeof("toe_ol")], &toe_ol, sizeof(uint32));

	if ((ret = cdc_set_ioctl(cdc, WLC_SET_VAR, buf, sizeof(buf))) < 0) {
		err("%s: could not set toe_ol: ret=%d", cdc->net->name, ret);
		return ret;
	}

	/* Enable toe globally only if any components are enabled. */

	toe = (toe_ol != 0);

	strcpy(buf, "toe");
	memcpy(&buf[sizeof("toe")], &toe, sizeof(uint32));

	if ((ret = cdc_set_ioctl(cdc, WLC_SET_VAR, buf, sizeof(buf))) < 0) {
		err("%s: could not set toe: ret=%d", cdc->net->name, ret);
		return ret;
	}

	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 2)
static int
cdc_ethtool(cdc_t *cdc, void *uaddr)
{
	struct ethtool_drvinfo info;
	struct ethtool_value edata;
	uint32 cmd, toe_cmpnt, csum_dir;
	int ret;

	if (copy_from_user(&cmd, uaddr, sizeof (uint32)))
		return -EFAULT;

	switch (cmd) {
	case ETHTOOL_GDRVINFO:
		memset(&info, 0, sizeof(info));
		info.cmd = cmd;

		/* Assumes we are talking to a wl device */
		sprintf(info.driver, "wl");

		/* Null-termination assured by memset above */
		strncpy(info.version, EPI_VERSION_STR, sizeof(info.version) - 1);

		if (copy_to_user(uaddr, &info, sizeof(info)))
			return -EFAULT;
		break;

	/* Get toe offload components from dongle */
	case ETHTOOL_GRXCSUM:
	case ETHTOOL_GTXCSUM:
		if ((ret = cdc_toe_get(cdc, &toe_cmpnt)) < 0)
			return ret;

		csum_dir = (cmd == ETHTOOL_GTXCSUM) ? TOE_TX_CSUM_OL : TOE_RX_CSUM_OL;

		edata.cmd = cmd;
		edata.data = (toe_cmpnt & csum_dir) ? 1 : 0;

		if (copy_to_user(uaddr, &edata, sizeof(edata)))
			return -EFAULT;
		break;

	/* Set toe offload components in dongle */
	case ETHTOOL_SRXCSUM:
	case ETHTOOL_STXCSUM:
		if (copy_from_user(&edata, uaddr, sizeof(edata)))
			return -EFAULT;

		/* Read the current settings, update and write back */
		if ((ret = cdc_toe_get(cdc, &toe_cmpnt)) < 0)
			return ret;

		csum_dir = (cmd == ETHTOOL_STXCSUM) ? TOE_TX_CSUM_OL : TOE_RX_CSUM_OL;

		if (edata.data != 0)
			toe_cmpnt |= csum_dir;
		else
			toe_cmpnt &= ~csum_dir;

		if ((ret = cdc_toe_set(cdc, toe_cmpnt)) < 0)
			return ret;

		/* If setting TX checksum mode, tell Linux the new mode */
		if (cmd == ETHTOOL_STXCSUM) {
			if (edata.data)
				cdc->net->features |= NETIF_F_IP_CSUM;
			else
				cdc->net->features &= ~NETIF_F_IP_CSUM;
		}

		break;
	default:
		return -EOPNOTSUPP;

	}

	return 0;
}
#endif /* Kernel > 2.4.2 */

static int
cdc_ioctl(struct net_device *net, struct ifreq *ifr, int cmd)
{
	cdc_t *cdc = net->priv;
	wl_ioctl_t ioc;
	void *buf = NULL;
	uint len = 0;
	int ret = 0;

	trace("cdc_ioctl");

	if (!cdc->up)
		return -ENXIO;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 2)
	if (cmd == SIOCETHTOOL)
		return cdc_ethtool(cdc, (void*)ifr->ifr_data);
#endif

	if (cmd != SIOCDEVPRIVATE)
		return -EINVAL;

	if (copy_from_user(&ioc, ifr->ifr_data, sizeof(wl_ioctl_t)))
		return -EFAULT;

	if (ioc.buf) {
		/* limit ioctl buf length */
		len = MIN(WLC_IOCTL_MAXLEN, ioc.len);
		if (!(buf = kmalloc(len, GFP_KERNEL)))
			return -ENOMEM;

		if (copy_from_user(buf, ioc.buf, len)) {
			ret = -EFAULT;
			goto done;
		}
	}

	if (ioc.set)
		ret = cdc_set_ioctl(cdc, ioc.cmd, buf, len);
	else
		ret = cdc_query_ioctl(cdc, ioc.cmd, buf, len);

	if (ret < 0)
		goto done;

	/* Too many programs assume ioctl() returns 0 on success */
	ret = 0;

	if (ioc.buf) {
		if (copy_to_user(ioc.buf, buf, len)) {
			ret = -EFAULT;
			goto done;
		}
	}

done:
	if (buf)
		kfree(buf);
	return ret;
}

static int
cdc_open(struct net_device *net)
{
	cdc_t *cdc = net->priv;
	int ret;
	char buf[32];
	uint32 toe_ol;

	trace("cdc_open");

	/* Let completion callbacks know that driver is up */
	cdc->up = 1;

	/* Start polling interrupt endpoint */
	usb_fill_int_urb(cdc->intr_urb, cdc->usb, cdc->intr_pipe,
	                 &cdc->intr, cdc->intr_size, cdc_intr_complete, cdc,
	                 cdc->interval);

	if ((ret = USB_SUBMIT_URB(cdc->intr_urb))) {
		err("%s: usb_submit_urb failed with status %d", cdc->net->name, ret);
		return ret;
	}

	/* Start receiving packets */
	if ((ret = cdc_rx_submit(cdc))) {
		USB_UNLINK_URB(cdc->intr_urb);
		cdc_unlink(&cdc->rxq);
		cdc->up = 0;
		return ret;
	}

	/* Get ethernet address from dongle */
	strcpy(buf, "cur_etheraddr");
	if ((ret = cdc_query_ioctl(cdc, WLC_GET_VAR, buf, sizeof(buf))) < 0) {
		err("%s: could not get MAC address", cdc->net->name);
		USB_UNLINK_URB(cdc->intr_urb);
		cdc_unlink(&cdc->rxq);
		cdc->up = 0;
		return ret;
	}

	/* Get current TOE mode from dongle */
	if (cdc_toe_get(cdc, &toe_ol) >= 0 && (toe_ol & TOE_TX_CSUM_OL) != 0)
		cdc->net->features |= NETIF_F_IP_CSUM;
	else
		cdc->net->features &= ~NETIF_F_IP_CSUM;

	memcpy(net->dev_addr, buf, ETH_ALEN);


	netif_start_queue(net);

	OLD_MOD_INC_USE_COUNT;
	return 0;
}

static void
cdc_unlink(struct sk_buff_head *q)
{
	struct sk_buff *skb;

	trace("cdc_unlink");

	/* Completion function(s) will move unlinked urbs back to the free list */
	while ((skb = skb_peek(q)))
		USB_UNLINK_URB(SKB_PRIV_URB(skb));
}

static int
cdc_stop(struct net_device *net)
{
	cdc_t *cdc = net->priv;

	trace("cdc_stop");

	cdc->up = 0;
	netif_stop_queue(net);

	USB_UNLINK_URB(cdc->intr_urb);

	cdc_unlink(&cdc->txq);
	cdc_unlink(&cdc->rxq);

	OLD_MOD_DEC_USE_COUNT;
	return 0;
}

/* All URBs must already have been unlinked before calling cdc_detach */
static void
cdc_detach(cdc_t *cdc)
{
	struct urb *urb;

	if (cdc->msg) {
		USB_BUFFER_FREE(cdc->usb, sizeof(cdc_ioctl_t) + WLC_IOCTL_MAXLEN,
		                cdc->msg, cdc->hmsg);
		cdc->msg = NULL;
	}

	if (cdc->intr_urb) {
		usb_free_urb(cdc->intr_urb);
		cdc->intr_urb = NULL;
	}

	while ((urb = cdc_urb_get(cdc)) != NULL)
		usb_free_urb(urb);

	if (cdc->net) {
		unregister_netdev(cdc->net);
		FREE_NETDEV(cdc->net);
		cdc->net = NULL;
	}
}

#ifdef KERNEL26
static int
cdc_probe(struct usb_interface *intf,
          const struct usb_device_id *id)
#else
static void *
cdc_probe(struct usb_device *usb, unsigned int ifnum,
          const struct usb_device_id *id)
#endif
{
	cdc_t *cdc = NULL;
	struct usb_endpoint_descriptor *endpoint;
	int ep;
	struct net_device *net;
	int ret = 0, i;
#ifdef KERNEL26
	struct usb_device *usb = interface_to_usbdev(intf);
#else
	int if_claimed = 0;
#endif

	trace("cdc_probe");

	/* Allocate private state */
	if (!(cdc = kmalloc(sizeof(cdc_t), GFP_KERNEL))) {
		err("cdc_probe: out of memory");
		ret = -ENOMEM;
		goto fail;
	}

	memset(cdc, 0, sizeof(cdc_t));

#ifdef KERNEL26
	usb_set_intfdata(intf, cdc);
#endif

	if (!usb) {
		err("cdc_probe: failed to get usb device");
		ret = -ENXIO;
		goto fail;
	}

	/* Set up network device */
	if (!(net = ALLOC_ETHERDEV())) {
		err("cdc_probe: alloc_etherdev failed");
		ret = -ENOMEM;
		goto fail;
	}

	cdc->net = net;

	cdc->usb = usb;
	init_waitqueue_head(&cdc->intr_wait);
	init_MUTEX(&cdc->sem);
	spin_lock_init(&cdc->lock);
	spin_lock_init(&cdc->urb_freelist_lock);
	skb_queue_head_init(&cdc->rxq);
	skb_queue_head_init(&cdc->txq);

	/* Default error code while checking device */
	ret = -ENXIO;

	/* Check that the device supports only one configuration */
	if (usb->descriptor.bNumConfigurations != 1) {
		err("cdc_probe: invalid number of configurations %d",
		    usb->descriptor.bNumConfigurations);
		goto fail;
	}

	/*
	 * Currently only the BDC interface configuration is supported:
	 *	Device class: USB_CLASS_VENDOR_SPEC
	 *	if0 class: USB_CLASS_VENDOR_SPEC
	 *	if0/ep0: control
	 *	if0/ep1: bulk in
	 *	if0/ep2: bulk out (ok if swapped with bulk in)
	 */

	if (usb->descriptor.bDeviceClass != USB_CLASS_VENDOR_SPEC) {
		err("cdc_probe: unsupported device class %d\n", usb->descriptor.bDeviceClass);
		goto fail;
	}

	/* Check that the configuration supports one interface */
	if (CONFIGDESC(usb)->bNumInterfaces != 1) {
		err("cdc_probe: invalid number of interfaces %d", CONFIGDESC(usb)->bNumInterfaces);
		goto fail;
	}

	/* Check interface */

#ifndef KERNEL26
	if (usb_interface_claimed(IFPTR(usb, CONTROL_IF))) {
		err("cdc_probe: interface already claimed");
		goto fail;
	}
#endif

	if (IFDESC(usb, CONTROL_IF).bInterfaceClass != USB_CLASS_VENDOR_SPEC ||
	    IFDESC(usb, CONTROL_IF).bInterfaceSubClass != 2 ||
	    IFDESC(usb, CONTROL_IF).bInterfaceProtocol != 0xff) {
		err("cdc_probe: invalid interface 0: class %d; subclass %d; proto %d",
		    IFDESC(usb, CONTROL_IF).bInterfaceClass,
		    IFDESC(usb, CONTROL_IF).bInterfaceSubClass,
		    IFDESC(usb, CONTROL_IF).bInterfaceProtocol);
		goto fail;
	}

	/* Check control endpoint */
	endpoint = &IFEPDESC(usb, CONTROL_IF, 0);
	if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_INT) {
		err("cdc_probe: invalid control endpoint");
		goto fail;
	}

	cdc->intr_pipe = usb_rcvintpipe(usb, endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);

#ifndef KERNEL26
	/* Claim interface */
	usb_driver_claim_interface(&cdc_driver, IFPTR(usb, CONTROL_IF), cdc);
	if_claimed = 1;
#endif

	/* Check data endpoints and get pipes */
	for (ep = 1; ep <= 2; ep++) {
		endpoint = &IFEPDESC(usb, BULK_IF, ep);
		if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) !=
		    USB_ENDPOINT_XFER_BULK) {
			err("cdc_probe: invalid data endpoint %d", ep);
			goto fail;
		}

		if ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
			cdc->rx_pipe = usb_rcvbulkpipe(usb,
			                               endpoint->bEndpointAddress &
			                               USB_ENDPOINT_NUMBER_MASK);
		else
			cdc->tx_pipe = usb_sndbulkpipe(usb,
			                               endpoint->bEndpointAddress &
			                               USB_ENDPOINT_NUMBER_MASK);
	}

	/* Register network device */
	net->priv = cdc;
	net->open = cdc_open;
	net->stop = cdc_stop;
	net->get_stats = cdc_get_stats;
	net->do_ioctl = cdc_ioctl;
	net->hard_start_xmit = cdc_start_xmit;
	net->hard_header_len = ETH_HLEN;
	net->mtu = ETH_DATA_LEN + 4;

	SET_NETDEV_DEV(net, &IFPTR(usb, CONTROL_IF)->dev);

	if (REGISTER_NETDEV(net) != 0) {
		err("register_netdev failed");
		goto fail;
	}

	/* alloc the IOCTL BUFFER */
	cdc->msg = (cdc_ioctl_t *)USB_BUFFER_ALLOC(usb, sizeof(cdc_ioctl_t) +
	                                           WLC_IOCTL_MAXLEN,
	                                           GFP_KERNEL, &cdc->hmsg);

	/* Allocate interrupt URB and data buffer */
	/* RNDIS says 8-byte intr, our old drivers used 4-byte */
	cdc->intr_size = (IFEPDESC(usb, CONTROL_IF, 0).wMaxPacketSize == 16) ? 8 : 4;

	cdc->interval = INTERVAL_DECODE(usb, IFEPDESC(usb, CONTROL_IF, 0).bInterval);

	if (!(cdc->intr_urb = USB_ALLOC_URB())) {
		err("%s: usb_alloc_urb failed", net->name);
		ret = -ENOMEM;
		goto fail;
	}

	/*
	 * Allocate static pool of URBs for bulk tx/rx.  On Linux 2.6 (at least) it seems
	 * that if the RX completion callback frees a URB that is being unlinked, the
	 * kernel can hang.  We'll avoid allocating/freeing them dynamically.
	 */

	for (i = 0; i < TX_QLEN + RX_QLEN; i++) {
		struct urb *urb;

		if (!(urb = USB_ALLOC_URB())) {
			err("%s: usb_alloc_urb failed", net->name);
			ret = -ENOMEM;
			goto fail;
		}

		cdc_urb_put(cdc, urb);
	}

	/* Success */
#ifdef KERNEL26
	return 0;
#else
	usb_inc_dev_use(usb);
	return cdc;
#endif

fail:
	err("cdc_probe: failed errno %d", ret);

#ifndef KERNEL26
	if (if_claimed)
		usb_driver_release_interface(&cdc_driver, IFPTR(usb, CONTROL_IF));
#endif

	cdc_detach(cdc);
	kfree(cdc);

#ifdef KERNEL26
	usb_set_intfdata(intf, NULL);
	return ret;
#else
	return NULL;
#endif
}

static struct usb_device_id cdc_table[] = {
	{ USB_DEVICE(0x0a5c, 0x0bdc) },
	{ }
};

MODULE_DEVICE_TABLE(usb, cdc_table);

static struct usb_driver cdc_driver = {
	name:		"cdc",
	probe:		cdc_probe,
	disconnect:	cdc_disconnect,
	id_table:	cdc_table
};

#ifdef KERNEL26
static void
cdc_disconnect(struct usb_interface *intf)
#else
static void
cdc_disconnect(struct usb_device *usb, void *ptr)
#endif
{
#ifdef KERNEL26
	cdc_t *cdc = usb_get_intfdata(intf);
	struct usb_device *usb = interface_to_usbdev(intf);
#else
	cdc_t *cdc = (cdc_t *)ptr;
#endif

	trace("cdc_disconnect");

	if (cdc == NULL || usb == NULL) {
		err("cdc_disconnect: null structure cdc=%p usb=%p", cdc, usb);
		return;
	}

	cdc_stop(cdc->net);
	cdc_detach(cdc);
	kfree(cdc);

#ifndef KERNEL26
	usb_driver_release_interface(&cdc_driver, IFPTR(usb, CONTROL_IF));
	usb_dec_dev_use(usb);
#endif

	info("cdc: disconnect");
}

static int __init
cdc_module_init(void)
{
	trace("cdc_module_init");

	return usb_register(&cdc_driver);
}

static void __exit
cdc_module_cleanup(void)
{
	trace("cdc_module_cleanup");

	usb_deregister(&cdc_driver);
}

module_init(cdc_module_init);
module_exit(cdc_module_cleanup);

static int
wl_host_event(struct sk_buff *skb)
{
	/* check whether packet is a BRCM event pkt */
	bcm_event_t *pvt_data = (bcm_event_t *)skb->data;
	uint i, msg;
	struct ether_addr *addr;
	uint status, reason, auth_type, datalen;
	bool group = FALSE, flush_txq = FALSE, link = FALSE;
	char *auth_str, *event_name;
	uchar *buf;
	char err_msg[256], eabuf[ETHER_ADDR_STR_LEN];
	static struct {
		uint event;
		char event_name[16];
	} event_names[] = {
		{ WLC_E_JOIN, "JOIN" },
		{ WLC_E_START, "START" },
		{ WLC_E_AUTH, "AUTH" },
		{ WLC_E_AUTH_IND, "AUTH_IND" },
		{ WLC_E_DEAUTH, "DEAUTH" },
		{ WLC_E_DEAUTH_IND, "DEAUTH_IND" },
		{ WLC_E_ASSOC, "ASSOC" },
		{ WLC_E_ASSOC_IND, "ASSOC_IND" },
		{ WLC_E_REASSOC, "REASSOC" },
		{ WLC_E_REASSOC_IND, "REASSOC_IND" },
		{ WLC_E_DISASSOC, "DISASSOC" },
		{ WLC_E_DISASSOC_IND, "DISASSOC_IND" },
		{ WLC_E_SET_SSID, "SET_SSID" },
		{ WLC_E_QUIET_START, "START_QUIET" },
		{ WLC_E_QUIET_END, "END_QUIET" },
		{ WLC_E_BEACON_RX, "BEACON_RX" },
		{ WLC_E_LINK, "LINK" },
		{ WLC_E_NDIS_LINK, "NDIS_LINK" },
		{ WLC_E_PMKID_CACHE, "PMKID_CACHE" },
		{ WLC_E_TXFAIL, "TXFAIL" },
		{ WLC_E_ROAM, "ROAM" }
	};

	if (bcmp(BRCM_OUI, &pvt_data->bcm_hdr.oui[0], DOT11_OUI_LEN))
		return 0;

	if (ntoh16(pvt_data->bcm_hdr.usr_subtype) != BCMILCP_BCM_SUBTYPE_EVENT)
		return 0;


	msg = ntoh32(pvt_data->event.event_type);
	addr = (struct ether_addr *)&(pvt_data->event.addr);
	status = ntoh32(pvt_data->event.status);
	reason = ntoh32(pvt_data->event.reason);
	auth_type = ntoh32(pvt_data->event.auth_type);
	datalen = ntoh32(pvt_data->event.datalen);

#if SHOW_EVENTS
	/* debug dump of event messages */
	if (addr != NULL) {
		sprintf(eabuf, "%02x:%02x:%02x:%02x:%02x:%02x",
			(uchar)pvt_data->event.addr.octet[0] & 0xff,
		        (uchar)pvt_data->event.addr.octet[1] & 0xff,
		        (uchar)pvt_data->event.addr.octet[2] & 0xff,
			(uchar)pvt_data->event.addr.octet[3] & 0xff,
		        (uchar)pvt_data->event.addr.octet[4] & 0xff,
		        (uchar)pvt_data->event.addr.octet[5] & 0xff);
	} else
		strcpy(eabuf, "<NULL>");

	event_name = "UNKNOWN";

	for (i = 0; i < ARRAYSIZE(event_names); i++) {
		if (event_names[i].event == msg)
			event_name = event_names[i].event_name;
	}

	if (ntoh16(pvt_data->event.flags) & WLC_EVENT_MSG_LINK)
		link = TRUE;
	if (ntoh16(pvt_data->event.flags) & WLC_EVENT_MSG_GROUP)
		group = TRUE;
	if (ntoh16(pvt_data->event.flags) & WLC_EVENT_MSG_FLUSHTXQ)
		flush_txq = TRUE;

	switch (msg) {
	case WLC_E_START:
	case WLC_E_DEAUTH:
	case WLC_E_ASSOC_IND:
	case WLC_E_REASSOC_IND:
	case WLC_E_DISASSOC:
		printk("MACEVENT: %s, MAC %s\n", event_name, eabuf);
		break;

	case WLC_E_ASSOC:
	case WLC_E_REASSOC:
		if (status == WLC_E_STATUS_SUCCESS) {
			printk("MACEVENT: %s, MAC %s, SUCCESS\n", event_name, eabuf);
		} else if (status == WLC_E_STATUS_TIMEOUT) {
			printk("MACEVENT: %s, MAC %s, TIMEOUT\n", event_name, eabuf);
		} else if (status == WLC_E_STATUS_FAIL) {
			printk("MACEVENT: %s, MAC %s, FAILURE, reason %d\n",
			       event_name, eabuf, (int)reason);
		} else {
			printk("MACEVENT: %s, MAC %s, unexpected status %d\n",
			       event_name, eabuf, (int)status);
		}
		break;

	case WLC_E_DEAUTH_IND:
	case WLC_E_DISASSOC_IND:
		printk("MACEVENT: %s, MAC %s, reason %d\n", event_name, eabuf, (int)reason);
		break;

	case WLC_E_AUTH:
	case WLC_E_AUTH_IND:
		if (auth_type == DOT11_OPEN_SYSTEM)
			auth_str = "Open System";
		else if (auth_type == DOT11_SHARED_KEY)
			auth_str = "Shared Key";
		else {
			sprintf(err_msg, "AUTH unknown: %d", (int)auth_type);
			auth_str = err_msg;
		}
		if (msg == WLC_E_AUTH_IND) {
			printk("MACEVENT: %s, MAC %s, %s\n",
			       event_name, eabuf, auth_str);
		} else if (status == WLC_E_STATUS_SUCCESS) {
			printk("MACEVENT: %s, MAC %s, %s, SUCCESS\n",
			       event_name, eabuf, auth_str);
		} else if (status == WLC_E_STATUS_TIMEOUT) {
			printk("MACEVENT: %s, MAC %s, %s, TIMEOUT\n",
			       event_name, eabuf, auth_str);
		} else if (status == WLC_E_STATUS_FAIL) {
			printk("MACEVENT: %s, MAC %s, %s, FAILURE, reason %d\n",
			       event_name, eabuf, auth_str, (int)reason);
		}

		break;

	case WLC_E_JOIN:
	case WLC_E_ROAM:
	case WLC_E_SET_SSID:
		if (status == WLC_E_STATUS_SUCCESS) {
			printk("MACEVENT: %s, MAC %s\n", event_name, eabuf);
		} else if (status == WLC_E_STATUS_FAIL) {
			printk("MACEVENT: %s, failed\n", event_name);
		} else if (status == WLC_E_STATUS_NO_NETWORKS) {
			printk("MACEVENT: %s, no networks found\n", event_name);
		} else {
			printk("MACEVENT: %s, unexpected status %d\n", event_name, (int)status);
		}
		break;

	case WLC_E_BEACON_RX:
		if (status == WLC_E_STATUS_SUCCESS) {
			printk("MACEVENT: %s, SUCCESS\n", event_name);
		} else if (status == WLC_E_STATUS_FAIL) {
			printk("MACEVENT: %s, FAIL\n", event_name);
		} else {
			printk("MACEVENT: %s, status %d\n", event_name, status);
		}
		break;

	case WLC_E_LINK:
		printk("MACEVENT: %s %s\n", event_name, link? "UP" : "DOWN");
		break;

	case WLC_E_MIC_ERROR:
		printk("MACEVENT: %s, MAC %s, Group %d, Flush %d\n",
		       event_name, eabuf, group, flush_txq);
		break;

	case WLC_E_TXFAIL:
		printk("MACEVENT: %s, RA %s\n", event_name, eabuf);
		break;

	case WLC_E_PMKID_CACHE:
		printk("MACEVENT: %s\n", event_name);
		break;

	default:
		printk("MACEVENT: UNKNOWN %d, MAC %s, status %d, reason %d, auth %d\n",
		       msg, eabuf, (int)status, (int)reason, (int)auth_type);
		break;
	}

	/* show any appended data */
	if (datalen) {
		buf = (char*)(pvt_data + 1);
		printk(" data (%d) : ", datalen);
		for (i = 0; i < datalen; i++)
			printk(" 0x%02x ", *buf++);
		printk("\n");
	}
#endif /* SHOW_EVENTS */

	return 1;
}
