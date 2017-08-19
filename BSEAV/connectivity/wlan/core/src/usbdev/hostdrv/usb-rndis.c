/*
 * RNDIS USB device Linux host driver
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
#include <asm/uaccess.h>
#include <asm/unaligned.h>

/* 2.4.x compatibility */
#ifndef FILL_BULK_URB
#define FILL_BULK_URB usb_fill_bulk_urb
#endif /* not FILL_BULK_URB */
#ifndef FILL_INT_URB
#define FILL_INT_URB usb_fill_int_urb
#endif /* not FILL_INT_URB */
#ifndef USB_QUEUE_BULK
#define USB_QUEUE_BULK 0	/* bulk transfer flag bits */
#endif /* not USB_QUEUE_BULK */

#include <bcm_ndis.h>
#include <rndis.h>

#define trace(format, arg...) dbg(format, ## arg)


#define QLEN 10		/* bulk rx and tx queue lengths */
#define RETRIES 2	/* # of retries for submitting ctrl reads & writes */

#include <typedefs.h>
#include <bcmutils.h>
#include <wlioctl.h>

typedef struct {
	uint32 notification;
	uint32 reserved;
} intr_t;

typedef struct {
	struct net_device *net;
	struct net_device_stats stats;
	int up;
	ulong medium;
	ulong drv_version;

	struct usb_device *usb;
	struct urb intr_urb;
	uint rx_pipe, tx_pipe, intr_pipe;
	struct sk_buff_head rxq, txq;
	wait_queue_head_t intr_wait;
	uint RequestId;
	intr_t intr;
	struct semaphore sem;
	spinlock_t lock;
	RNDIS_MESSAGE msg;
	unsigned char buf[1024 - sizeof(RNDIS_MESSAGE)];
} rndis_t;

static void rndis_tx_complete(struct urb *urb);
static void rndis_rx_complete(struct urb *urb);
static int rndis_rx_submit(rndis_t *rndis);
static void rndis_pkt_header(rndis_t *rndis, struct sk_buff *skb);
static void rndis_disconnect(struct usb_device *usb, void *ptr);

static struct usb_device_id rndis_table[];
static struct usb_driver rndis_driver;

static void
rndis_tx_complete(struct urb *urb)
{
	struct sk_buff *skb = (struct sk_buff *) urb->context;
	struct net_device *net = skb->dev;
	rndis_t *rndis = net->priv;
	unsigned long flags;

	trace("rndis_tx_complete");

	spin_lock_irqsave(&rndis->txq.lock, flags);

	if (skb->list) {
		/* Remove from queue */
		__skb_unlink(skb, skb->list);

		/* Flow control */
		if (rndis->up && netif_queue_stopped(net) &&
		    skb_queue_len(&rndis->txq) < QLEN)
			netif_wake_queue(net);
	}

	spin_unlock_irqrestore(&rndis->txq.lock, flags);

	if (urb->status) {
		rndis->stats.tx_errors++;
		err("%s: tx error %d", net->name, urb->status);
	} else
		rndis->stats.tx_bytes += skb->len;

	dev_kfree_skb_any(skb);
	usb_free_urb(urb);
}

static int
rndis_start_xmit(struct sk_buff *skb, struct net_device *net)
{
	rndis_t *rndis = (rndis_t *) net->priv;
	struct urb *urb = NULL;
	int ret = 0;
	unsigned long flags;

	trace("rndis_start_xmit");

	/* Ensure sufficient headroom */
	if (skb_headroom(skb) < RNDIS_MESSAGE_SIZE(RNDIS_PACKET)) {
		struct sk_buff *skb2;

		err("%s: insufficient headroom", net->name);
		skb2 = skb_realloc_headroom(skb, RNDIS_MESSAGE_SIZE(RNDIS_PACKET));
		dev_kfree_skb(skb);
		skb = skb2;
		if (!skb) {
			err("%s: skb_realloc_headroom failed", net->name);
			ret = -ENOMEM;
			goto done;
		}
	}

	/* Push RNDIS header */
	rndis_pkt_header(rndis, skb);

	/* Allocate URB */
	if (!(urb = usb_alloc_urb(0))) {
		err("%s: usb_alloc_urb failed", net->name);
		ret = -ENOMEM;
		goto done;
	}

	skb->dev = net;

	/* Save pointer to URB */
	*((struct urb **) skb->cb) = urb;

	FILL_BULK_URB(urb, rndis->usb, rndis->tx_pipe,
	              skb->data, skb->len, rndis_tx_complete, skb);
	urb->transfer_flags |= USB_QUEUE_BULK;

	spin_lock_irqsave(&rndis->txq.lock, flags);

	if ((ret = usb_submit_urb(urb)))
		err("%s: usb_submit_urb failed with status %d", net->name, ret);
	else {
		/* Enqueue packet */
		__skb_queue_tail(&rndis->txq, skb);

		/* Flow control */
		if (skb_queue_len(&rndis->txq) >= QLEN)
			netif_stop_queue(net);
	}

	spin_unlock_irqrestore(&rndis->txq.lock, flags);

done:
	if (ret) {
		if (skb)
			dev_kfree_skb(skb);
		if (urb)
			usb_free_urb(urb);
		rndis->stats.tx_dropped++;
	}

	/* We've eaten the skb */
	return 0;
}

static void
rndis_rx_complete(struct urb *urb)
{
	struct sk_buff *skb = (struct sk_buff *) urb->context;
	struct net_device *net = skb->dev;
	rndis_t *rndis = (rndis_t *) net->priv;
	RNDIS_MESSAGE *msg = (RNDIS_MESSAGE *) skb->data;
	RNDIS_PACKET *pkt = (RNDIS_PACKET *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	u32 NdisMessageType, MessageLength, DataOffset, DataLength;
	unsigned char *eth;

	trace("rndis_rx_complete");

	/* Remove from queue */
	skb_unlink(skb);

	if (urb->status) {
		err("%s: rx error %d", net->name, urb->status);
		dev_kfree_skb_any(skb);
		goto done;
	}

	/* Check packet */
	NdisMessageType = le32_to_cpu(msg->NdisMessageType);
	MessageLength = le32_to_cpu(msg->MessageLength);
	DataOffset = le32_to_cpu(pkt->DataOffset);
	DataLength = le32_to_cpu(pkt->DataLength);
	if (NdisMessageType != REMOTE_NDIS_PACKET_MSG ||
	    MessageLength > urb->actual_length) {
		err("%s: invalid packet", net->name);
		dev_kfree_skb_any(skb);
		goto done;
	}

	skb_put(skb, MessageLength);
	skb_pull(skb, (uintptr) &pkt->DataOffset - (uintptr) msg);
	skb_pull(skb, DataOffset);

	eth = skb->data;
	skb->protocol = eth_type_trans(skb, net);
	skb->data = eth + ETH_HLEN;
	skb->len = DataLength - ETH_HLEN;
	skb->dev = net;
	net->last_rx = jiffies;
	rndis->stats.rx_bytes += skb->len;

	netif_rx(skb);

done:
	usb_free_urb(urb);
	if (rndis->up)
		rndis_rx_submit(rndis);
}

static int
rndis_rx_submit(rndis_t *rndis)
{
	int len = rndis->net->hard_header_len + rndis->net->mtu;
	struct urb *urb = NULL;
	struct sk_buff *skb = NULL;
	int ret = 0;
	unsigned long flags;

	trace("rndis_rx_submit");

	spin_lock_irqsave(&rndis->rxq.lock, flags);

	while (skb_queue_len(&rndis->rxq) < QLEN) {
		/* Allocate URB for this packet */
		if (!(urb = usb_alloc_urb(0))) {
			err("%s: usb_alloc_urb failed", rndis->net->name);
			ret = -ENOMEM;
			break;
		}

		/* Allocate packet */
		if (!(skb = dev_alloc_skb(len))) {
			err("%s: dev_alloc_skb failed", rndis->net->name);
			usb_free_urb(urb);
			ret = -ENOMEM;
			break;
		}

		skb->dev = rndis->net;

		/* Save pointer to URB */
		*((struct urb **) skb->cb) = urb;

		FILL_BULK_URB(urb, rndis->usb, rndis->rx_pipe,
		              skb->data, len, rndis_rx_complete, skb);
		urb->transfer_flags |= USB_QUEUE_BULK;

		if ((ret = usb_submit_urb(urb))) {
			err("%s: usb_submit_urb failed with status %d", rndis->net->name, ret);
			break;
		}

		/* Enqueue packet */
		__skb_queue_tail(&rndis->rxq, skb);
	}

	spin_unlock_irqrestore(&rndis->rxq.lock, flags);

	if (ret) {
		if (skb)
			dev_kfree_skb_any(skb);
		if (urb)
			usb_free_urb(urb);
	}

	return ret;
}

static void
rndis_intr_complete(struct urb *urb)
{
	rndis_t *rndis = urb->context;

	trace("rndis_intr_complete");

	if (waitqueue_active(&rndis->intr_wait))
		wake_up_interruptible(&rndis->intr_wait);
}

static int
rndis_cmplt(rndis_t *rndis, u32 NdisMessageType)
{
	DECLARE_WAITQUEUE(wait, current);
	int timeout = HZ, ret = 0, retries = 0;
	u32 intr;
	unsigned long flags;
	unsigned long ifnum;

	trace("rndis_cmplt");

	ifnum = cpu_to_le16(rndis->usb->actconfig->interface[0].altsetting[0].bInterfaceNumber);
	do {
		/* Wait for interrupt */
		add_wait_queue(&rndis->intr_wait, &wait);
		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock_irqsave(&rndis->lock, flags);
		while (!(intr = le32_to_cpu(rndis->intr.notification)) &&
		       (!signal_pending(current) && timeout)) {
			spin_unlock_irqrestore(&rndis->lock, flags);
			timeout = schedule_timeout(timeout);
			spin_lock_irqsave(&rndis->lock, flags);
		}
		bzero(&rndis->intr, sizeof(rndis->intr));
		spin_unlock_irqrestore(&rndis->lock, flags);
		remove_wait_queue(&rndis->intr_wait, &wait);
		set_current_state(TASK_RUNNING);

		if (intr) {
			dbg("%s: interrupt", rndis->net->name);
		} else if (timeout == 0) {
			dbg("%s: timeout", rndis->net->name);
		} else if (signal_pending(current)) {
			err("%s: cancelled", rndis->net->name);
			return -ERESTARTSYS;
		}

	retry:
		/* Get response */
		ret = usb_control_msg(rndis->usb, usb_rcvctrlpipe(rndis->usb, 0), 1,
		                      USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		                      cpu_to_le16(0), ifnum, &rndis->msg, 1024,
		                      HZ / RETRIES);
		if (ret == -ETIMEDOUT && ++retries < RETRIES)
			goto retry;
		if (ret < 0) {
			err("%s: %s: usb_control_msg failed with status %d",
			    rndis->net->name, __FUNCTION__, ret);
			return ret;
		}
	} while (le32_to_cpu(rndis->msg.NdisMessageType) != NdisMessageType);

	return ret;
}

static int
rndis_msg(rndis_t *rndis)
{
	int ret, retries = 0;
	unsigned long ifnum;

	trace("rndis_msg");

	ifnum = cpu_to_le16(rndis->usb->actconfig->interface[0].altsetting[0].bInterfaceNumber);
retry:
	/* Send request */
	ret = usb_control_msg(rndis->usb, usb_sndctrlpipe(rndis->usb, 0), 0,
	                      USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
	                      cpu_to_le16(0), ifnum,
	                      &rndis->msg, le32_to_cpu(rndis->msg.MessageLength),
	                      HZ / RETRIES);
	if (ret == -ETIMEDOUT && ++retries < RETRIES)
		goto retry;
	if (ret < 0) {
		err("%s: %s: usb_control_msg failed with status %d",
		    rndis->net->name, __FUNCTION__, ret);
		return ret;
	}

	return 0;
}

static int
rndis_query_oid(rndis_t *rndis, uint oid, void *buf, uint len)
{
	RNDIS_MESSAGE *msg = &rndis->msg;
	void *info;
	int ret = 0, retries = 0;

	trace("rndis_query_oid");

	if ((RNDIS_MESSAGE_SIZE(RNDIS_QUERY_REQUEST) + len) > 1024) {
		err("%s: rndis_query_oid: bad length", rndis->net->name);
		return -EINVAL;
	}

	down(&rndis->sem);

	/* REMOTE_NDIS_QUERY_MSG */
	memset(msg, 0, RNDIS_MESSAGE_SIZE(RNDIS_QUERY_REQUEST));
	msg->NdisMessageType = cpu_to_le32(REMOTE_NDIS_QUERY_MSG);
	msg->MessageLength = cpu_to_le32(RNDIS_MESSAGE_SIZE(RNDIS_QUERY_REQUEST) + len);

	msg->Message.QueryRequest.RequestId = cpu_to_le32(++rndis->RequestId);
	msg->Message.QueryRequest.Oid = cpu_to_le32(oid);
	if (buf) {
		msg->Message.QueryRequest.InformationBufferLength = cpu_to_le32(len);
		msg->Message.QueryRequest.InformationBufferOffset =
		    cpu_to_le32(sizeof(RNDIS_QUERY_REQUEST));
		memcpy((void *)((uintptr) msg + RNDIS_MESSAGE_SIZE(RNDIS_QUERY_REQUEST)), buf, len);
	}
	if ((ret = rndis_msg(rndis)) < 0)
		goto done;

retry:
	/* REMOTE_NDIS_QUERY_CMPLT */
	if ((ret = rndis_cmplt(rndis, REMOTE_NDIS_QUERY_CMPLT)) < 0)
		goto done;
	if (le32_to_cpu(msg->MessageLength) < RNDIS_MESSAGE_SIZE(RNDIS_QUERY_COMPLETE)) {
		err("%s: rndis_query_oid: bad message length %d", rndis->net->name,
		    le32_to_cpu(msg->MessageLength));
		ret = -EINVAL;
		goto done;
	}
	if (le32_to_cpu(msg->Message.QueryComplete.RequestId) <
	    rndis->RequestId && ++retries < RETRIES)
		goto retry;
	if (le32_to_cpu(msg->Message.QueryComplete.RequestId) != rndis->RequestId) {
		err("%s: rndis_query_oid: unexpected request id %d (expected %d)", rndis->net->name,
		    le32_to_cpu(msg->Message.QueryComplete.RequestId), rndis->RequestId);
		ret = -EINVAL;
		goto done;
	}
	if (le32_to_cpu(msg->Message.QueryComplete.Status) != RNDIS_STATUS_SUCCESS) {
		trace("%s: rndis_query_oid: status 0x%x", rndis->net->name,
		      le32_to_cpu(msg->Message.QueryComplete.Status));
		ret = -EINVAL;
		goto done;
	}

	/* Check info buffer */
	info = (void *)((uintptr) &msg->Message.QueryComplete.RequestId +
	                le32_to_cpu(msg->Message.QueryComplete.InformationBufferOffset));
	if (len > le32_to_cpu(msg->Message.QueryComplete.InformationBufferLength))
		len = le32_to_cpu(msg->Message.QueryComplete.InformationBufferLength);
	if (((uintptr) info + len) > ((uintptr) msg + le32_to_cpu(msg->MessageLength))) {
		err("%s: rndis_query_oid: bad message length %d", rndis->net->name,
		    le32_to_cpu(msg->MessageLength));
		ret = -EINVAL;
		goto done;
	}

	/* Copy info buffer */
	if (buf)
		memcpy(buf, info, len);

done:
	up(&rndis->sem);
	return ret;
}

static int
rndis_set_oid(rndis_t *rndis, uint oid, void *buf, uint len)
{
	RNDIS_MESSAGE *msg = &rndis->msg;
	int ret = 0;

	trace("rndis_set_oid");

	if ((RNDIS_MESSAGE_SIZE(RNDIS_SET_REQUEST) + len) > 1024) {
		err("%s: rndis_set_oid: bad length", rndis->net->name);
		return -EINVAL;
	}

	down(&rndis->sem);

	/* REMOTE_NDIS_SET_MSG */
	memset(msg, 0, RNDIS_MESSAGE_SIZE(RNDIS_SET_REQUEST));
	msg->NdisMessageType = cpu_to_le32(REMOTE_NDIS_SET_MSG);
	msg->MessageLength = cpu_to_le32(RNDIS_MESSAGE_SIZE(RNDIS_SET_REQUEST) + len);
	msg->Message.SetRequest.RequestId = cpu_to_le32(++rndis->RequestId);
	msg->Message.SetRequest.Oid = cpu_to_le32(oid);
	msg->Message.SetRequest.InformationBufferLength = len;
	msg->Message.SetRequest.InformationBufferOffset = sizeof(RNDIS_SET_REQUEST);
	memcpy((void *)((uintptr) msg + RNDIS_MESSAGE_SIZE(RNDIS_SET_REQUEST)), buf, len);
	if ((ret = rndis_msg(rndis)) < 0)
		goto done;

	/* REMOTE_NDIS_SET_CMPLT */
	if ((ret = rndis_cmplt(rndis, REMOTE_NDIS_SET_CMPLT)) < 0)
		goto done;
	if (le32_to_cpu(msg->MessageLength) < RNDIS_MESSAGE_SIZE(RNDIS_SET_COMPLETE)) {
		err("%s: rndis_set_oid: bad message length %d", rndis->net->name,
		    le32_to_cpu(msg->MessageLength));
		ret = -EINVAL;
		goto done;
	}
	if (le32_to_cpu(msg->Message.SetComplete.RequestId) != rndis->RequestId) {
		err("%s: rndis_set_oid: unexpected request id %d (expected %d)", rndis->net->name,
		    le32_to_cpu(msg->Message.SetComplete.RequestId), rndis->RequestId);
		ret = -EINVAL;
		goto done;
	}
	if (le32_to_cpu(msg->Message.SetComplete.Status) != RNDIS_STATUS_SUCCESS) {
		trace("%s: rndis_set_oid: status 0x%x", rndis->net->name,
		    le32_to_cpu(msg->Message.SetComplete.Status));
		ret = -EINVAL;
		goto done;
	}

done:
	up(&rndis->sem);
	return ret;
}


static int
rndis_init(rndis_t *rndis)
{
	RNDIS_MESSAGE *msg = &rndis->msg;
	int ret = 0;

	trace("rndis_init");

	down(&rndis->sem);

	/* REMOTE_NDIS_INITIALIZE_MSG */
	memset(msg, 0, RNDIS_MESSAGE_SIZE(RNDIS_INITIALIZE_REQUEST));
	msg->NdisMessageType = cpu_to_le32(REMOTE_NDIS_INITIALIZE_MSG);
	msg->MessageLength = cpu_to_le32(RNDIS_MESSAGE_SIZE(RNDIS_INITIALIZE_REQUEST));
	msg->Message.InitializeRequest.RequestId = cpu_to_le32(++rndis->RequestId);
	msg->Message.InitializeRequest.MajorVersion = cpu_to_le32(RNDIS_MAJOR_VERSION);
	msg->Message.InitializeRequest.MinorVersion = cpu_to_le32(RNDIS_MINOR_VERSION);
	msg->Message.InitializeRequest.MaxTransferSize = cpu_to_le32(0x4000);
	if ((ret = rndis_msg(rndis)) < 0)
		goto done;

	/* REMOTE_NDIS_INITIALIZE_CMPLT */
	if ((ret = rndis_cmplt(rndis, REMOTE_NDIS_INITIALIZE_CMPLT)) < 0)
		goto done;
	if (le32_to_cpu(msg->MessageLength) < RNDIS_MESSAGE_SIZE(RNDIS_INITIALIZE_COMPLETE)) {
		err("%s: rndis_init: bad message length %d", rndis->net->name,
		    le32_to_cpu(msg->NdisMessageType));
		ret = -ENODEV;
		goto done;
	}
	if (le32_to_cpu(msg->Message.InitializeComplete.RequestId) != rndis->RequestId) {
		err("%s: rndis_init: unexpected request id %d (expected %d)", rndis->net->name,
		    le32_to_cpu(msg->Message.InitializeComplete.RequestId), rndis->RequestId);
		ret = -ENODEV;
		goto done;
	}
	if (le32_to_cpu(msg->Message.InitializeComplete.Status) != RNDIS_STATUS_SUCCESS) {
		err("%s: rndis_init: status 0x%x", rndis->net->name,
		    le32_to_cpu(msg->Message.InitializeComplete.Status));
		ret = -ENODEV;
		goto done;
	}

done:
	up(&rndis->sem);
	return ret;
}

static int
rndis_halt(rndis_t *rndis)
{
	RNDIS_MESSAGE *msg = &rndis->msg;
	int ret;

	trace("rndis_halt");

	down(&rndis->sem);

	/* REMOTE_NDIS_HALT_MSG */
	memset(msg, 0, RNDIS_MESSAGE_SIZE(RNDIS_HALT_REQUEST));
	msg->NdisMessageType = cpu_to_le32(REMOTE_NDIS_HALT_MSG);
	msg->MessageLength = cpu_to_le32(RNDIS_MESSAGE_SIZE(RNDIS_HALT_REQUEST));
	msg->Message.HaltRequest.RequestId = cpu_to_le32(++rndis->RequestId);
	ret = rndis_msg(rndis);

	up(&rndis->sem);
	return ret;
}

static struct net_device_stats *
rndis_get_stats(struct net_device *net)
{
	rndis_t *rndis = (rndis_t *) net->priv;

	trace("rndis_get_stats");

	if (rndis->up) {
		rndis_query_oid(rndis, RNDIS_OID_GEN_XMIT_OK,
		                &rndis->stats.tx_packets, sizeof(ulong));
		rndis_query_oid(rndis, RNDIS_OID_GEN_RCV_OK,
		                &rndis->stats.rx_packets, sizeof(ulong));
		rndis_query_oid(rndis, RNDIS_OID_GEN_XMIT_ERROR,
		                &rndis->stats.tx_errors, sizeof(ulong));
		rndis_query_oid(rndis, RNDIS_OID_GEN_RCV_ERROR,
		                &rndis->stats.rx_errors, sizeof(ulong));
		rndis_query_oid(rndis, RNDIS_OID_GEN_RCV_NO_BUFFER,
		                &rndis->stats.rx_dropped, sizeof(ulong));
	}

	return &rndis->stats;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 2)
static int
rndis_ethtool(rndis_t *rndis, void *uaddr)
{
	struct ethtool_drvinfo info;
	uint32 cmd;

	if (copy_from_user(&cmd, uaddr, sizeof (uint32)))
		return (-EFAULT);

	switch (cmd) {
	case ETHTOOL_GDRVINFO:
		memset(&info, 0, sizeof(info));
		info.cmd = cmd;
		if (rndis->medium == NdisPhysicalMediumWirelessLan)
			sprintf(info.driver, "wl");
		else
			sprintf(info.driver, "xx"); /* unknown */
		sprintf(info.version, "%lu", rndis->drv_version);
		if (copy_to_user(uaddr, &info, sizeof(info)))
			return (-EFAULT);
		return (0);
	}

	return (-EOPNOTSUPP);
}
#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 2) */

static int
rndis_ioctl(struct net_device *net, struct ifreq *ifr, int cmd)
{
	rndis_t *rndis = (rndis_t *) net->priv;
	wl_ioctl_t ioc;
	void *buf = NULL;
	uint len = 0;
	int ret = 0;

	trace("rndis_ioctl");

	if (!rndis->up)
		return -ENODEV;

	if (rndis->medium != NdisPhysicalMediumWirelessLan)
		return -EINVAL;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 2)
	if (cmd == SIOCETHTOOL)
		return (rndis_ethtool(rndis, (void*)ifr->ifr_data));
#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2, 4, 2) */

	if (cmd != SIOCDEVPRIVATE)
		return -EINVAL;

	if (copy_from_user(&ioc, ifr->ifr_data, sizeof(wl_ioctl_t)))
		return -EFAULT;

	if (ioc.buf) {
		len = MIN(ioc.len, 1024 - RNDIS_MESSAGE_SIZE(RNDIS_QUERY_REQUEST));

		if (!(buf = kmalloc(len, GFP_KERNEL)))
			return -ENOMEM;

		if (copy_from_user(buf, ioc.buf, len)) {
			ret = -EFAULT;
			goto done;
		}
	}

	if (ioc.set)
		ret = rndis_set_oid(rndis, WL_OID_BASE + ioc.cmd, buf, len);
	else
		ret = rndis_query_oid(rndis, WL_OID_BASE + ioc.cmd, buf, len);

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
rndis_open(struct net_device *net)
{
	rndis_t *rndis = (rndis_t *) net->priv;
	int interval, ret, intr_size;
	ulong ul;

	trace("rndis_open");

	/* Start receiving packets */
	rndis_rx_submit(rndis);

	/* Start polling interrupt endpoint */
	interval = rndis->usb->actconfig->interface[0].altsetting[0].endpoint[0].bInterval;
	if (rndis->usb->speed == USB_SPEED_HIGH)
		interval = 1 << (interval - 1);

	/* RNDIS spec says to use an 8-byte intr but our old drivers used 4 bytes for intr */
	if (rndis->usb->actconfig->interface[0].altsetting[0].endpoint[0].wMaxPacketSize == 16)
		intr_size = 8;
	else
		intr_size = 4;

	FILL_INT_URB(&rndis->intr_urb, rndis->usb, rndis->intr_pipe,
	             &rndis->intr, intr_size, rndis_intr_complete, rndis,
	             interval);
	if ((ret = usb_submit_urb(&rndis->intr_urb))) {
		err("%s: rndis_open: usb_submit_urb failed with status %d", rndis->net->name, ret);
		return ret;
	}

	/* Startup RNDIS */
	rndis_init(rndis);

	rndis_query_oid(rndis, RNDIS_OID_802_3_CURRENT_ADDRESS, net->dev_addr, ETH_ALEN);
	info("%s: mac  %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", rndis->net->name,
	     net->dev_addr[0], net->dev_addr[1], net->dev_addr[2],
	     net->dev_addr[3], net->dev_addr[4], net->dev_addr[5]);

	rndis_query_oid(rndis, RNDIS_OID_GEN_MAXIMUM_FRAME_SIZE, &ul, sizeof(ul));
	info("%s: mtu %lu", rndis->net->name, ul);

	rndis_query_oid(rndis, RNDIS_OID_GEN_VENDOR_DRIVER_VERSION, &ul, sizeof(ul));
	info("%s: drv version %lu", rndis->net->name, ul);
	rndis->drv_version = ul;

	if (rndis_query_oid(rndis, OID_GEN_PHYSICAL_MEDIUM, &ul, sizeof(ulong)) >= 0) {
		rndis->medium = ul;
		info("%s: medium %s", rndis->net->name,
		     (ul == NdisPhysicalMediumWirelessLan) ? "wireless" : "unspecified");
	} else
		rndis->medium = NdisPhysicalMediumUnspecified;


	ul = cpu_to_le32(NDIS_PACKET_TYPE_DIRECTED |
	                 NDIS_PACKET_TYPE_MULTICAST |
	                 NDIS_PACKET_TYPE_BROADCAST);
	rndis_set_oid(rndis, RNDIS_OID_GEN_CURRENT_PACKET_FILTER, &ul, sizeof(ul));
	info("%s: filter %lu", rndis->net->name, ul);

	netif_start_queue(net);
	rndis->up = 1;

	MOD_INC_USE_COUNT;
	return 0;
}

static void
rndis_unlink(struct sk_buff_head *q)
{
	struct sk_buff *skb;
	struct urb *urb;

	while ((skb = skb_dequeue(q))) {
		urb = *((struct urb **) skb->cb);
		usb_unlink_urb(urb);
	}
}

static int
rndis_stop(struct net_device *net)
{
	rndis_t *rndis = (rndis_t *) net->priv;

	trace("rndis_stop");

	rndis->up = 0;
	netif_stop_queue(net);

	/* Stop RNDIS */
	rndis_halt(rndis);

	/* Cancel pending URBs */
	usb_unlink_urb(&rndis->intr_urb);
	rndis_unlink(&rndis->txq);
	rndis_unlink(&rndis->rxq);

	MOD_DEC_USE_COUNT;
	return 0;
}

/* Push RNDIS header onto a data packet */
static void
rndis_pkt_header(rndis_t *rndis, struct sk_buff *skb)
{
	RNDIS_MESSAGE *msg;
	RNDIS_PACKET *pkt;

	trace("rndis_pkt_header");

	/* Push RNDIS header */
	skb_push(skb, RNDIS_MESSAGE_SIZE(RNDIS_PACKET));
	memset(skb->data, 0, RNDIS_MESSAGE_SIZE(RNDIS_PACKET));

	msg = (RNDIS_MESSAGE *) skb->data;
	pkt = (RNDIS_PACKET *) RNDIS_MESSAGE_PTR_TO_MESSAGE_PTR(msg);
	put_unaligned(cpu_to_le32(REMOTE_NDIS_PACKET_MSG), &msg->NdisMessageType);
	put_unaligned(cpu_to_le32(skb->len), &msg->MessageLength);
	put_unaligned(cpu_to_le32(sizeof(RNDIS_PACKET)), &pkt->DataOffset);
	put_unaligned(cpu_to_le32(skb->len - RNDIS_MESSAGE_SIZE(RNDIS_PACKET)), &pkt->DataLength);
}

static void *
rndis_probe(struct usb_device *usb, unsigned int ifnum, const struct usb_device_id *id)
{
	rndis_t *rndis;
	struct usb_interface_descriptor *interface;
	struct usb_endpoint_descriptor *endpoint;
	int i;
	struct net_device *net;

	trace("rndis_probe");

	/* Allocate private state */
	if (!(rndis = kmalloc(sizeof(rndis_t), GFP_KERNEL))) {
		err("kmalloc failed");
		goto fail;
	}
	memset(rndis, 0, sizeof(rndis_t));
	rndis->usb = usb;
	init_waitqueue_head(&rndis->intr_wait);
	init_MUTEX(&rndis->sem);
	spin_lock_init(&rndis->lock);
	skb_queue_head_init(&rndis->rxq);
	skb_queue_head_init(&rndis->txq);

	/* Check that the device supports only one configuration */
	if (usb->descriptor.bNumConfigurations != 1)
		goto fail;

	/* Check that the configuration supports two interfaces */
	if (usb->actconfig->bNumInterfaces != 2)
		goto fail;

	/* Check control interface */
	interface = &usb->actconfig->interface[0].altsetting[0];
	if (usb_interface_claimed(&usb->actconfig->interface[0]) ||
	    interface->bInterfaceClass != 2 ||
	    interface->bInterfaceSubClass != 2 ||
	    interface->bInterfaceProtocol != 0xff ||
	    interface->bNumEndpoints != 1)
		goto fail;

	/* Check control endpoint */
	endpoint = &interface->endpoint[0];
	if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_INT)
		goto fail;
	rndis->intr_pipe =
	    usb_rcvintpipe(usb, endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);

	/* Claim control interface */
	usb_driver_claim_interface(&rndis_driver, &usb->actconfig->interface[0], rndis);

	/* Check data interface */
	interface = &usb->actconfig->interface[1].altsetting[0];
	if (usb_interface_claimed(&usb->actconfig->interface[1]) ||
	    interface->bInterfaceClass != 10 ||
	    interface->bInterfaceSubClass != 0 ||
	    interface->bInterfaceProtocol != 0 ||
	    interface->bNumEndpoints != 2)
		goto fail;

	/* Check data endpoints */
	for (i = 0; i < 2; i++) {
		endpoint = &interface->endpoint[i];
		if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_BULK)
			goto fail;
		if ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
			rndis->rx_pipe =
			    usb_rcvbulkpipe(usb,
			                    endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);
		else
			rndis->tx_pipe =
			    usb_sndbulkpipe(usb,
			                    endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);
	}

	/* Claim data interface */
	usb_driver_claim_interface(&rndis_driver, &usb->actconfig->interface[1], rndis);

	/* Set up network device */
	if (!(net = init_etherdev(NULL, 0))) {
		err("init_etherdev failed");
		goto fail;
	}
	rndis->net = net;

	net->priv = rndis;
	net->open = rndis_open;
	net->stop = rndis_stop;
	net->get_stats = rndis_get_stats;
	net->do_ioctl = rndis_ioctl;
	net->hard_start_xmit = rndis_start_xmit;
	net->hard_header_len = RNDIS_MESSAGE_SIZE(RNDIS_PACKET) + ETH_HLEN;

	usb_inc_dev_use(usb);
	return rndis;

fail:
	usb_driver_release_interface(&rndis_driver, &usb->actconfig->interface[0]);
	usb_driver_release_interface(&rndis_driver, &usb->actconfig->interface[1]);
	if (rndis->net)
		unregister_netdev(rndis->net);
	if (rndis)
		kfree(rndis);
	return NULL;
}

static struct usb_device_id rndis_table[] = {
	{ USB_DEVICE(0x0a5c, 0xd11b) },
	{ USB_DEVICE(0x0565, 0x0041) },
	{ USB_DEVICE(0x04f9, 0x01a1) },
	{ }
};
MODULE_DEVICE_TABLE(usb, rndis_table);

static struct usb_driver rndis_driver = {
	name:		"rndis",
	probe:		rndis_probe,
	disconnect:	rndis_disconnect,
	id_table:	rndis_table
};

static void
rndis_disconnect(struct usb_device *usb, void *ptr)
{
	rndis_t *rndis = (rndis_t *) ptr;

	trace("rndis_disconnect");

	unregister_netdev(rndis->net);

	usb_driver_release_interface(&rndis_driver, &usb->actconfig->interface[0]);
	usb_driver_release_interface(&rndis_driver, &usb->actconfig->interface[1]);

	kfree(rndis);
	usb_dec_dev_use(usb);

	info("rndis: disconnect");
}

static int __init
rndis_module_init(void)
{
	trace("rndis_module_init");

	return usb_register(&rndis_driver);
}

static void __exit
rndis_module_cleanup(void)
{
	trace("rndis_module_cleanup");

	usb_deregister(&rndis_driver);
}

module_init(rndis_module_init);
module_exit(rndis_module_cleanup);
