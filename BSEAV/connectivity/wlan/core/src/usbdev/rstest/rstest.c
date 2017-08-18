/*
 * Broadcom USB Linux in-kernel remote socket client test
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

#include <linux/config.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/poll.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>		/* for skb free routines */
#include <linux/spinlock.h>

#include "os.h"
#include <bcmrsock/plumb.h>

#include "rstest_cmd.h"

#define USB_RSTEST_MINOR_BASE	144

#define USB_RSTEST_MTU		1536	/* Must be larger than RSOCK_BUS_MTU */

/* 2.4.x compatibility */
#ifndef FILL_BULK_URB
#define FILL_BULK_URB		usb_fill_bulk_urb
#endif
#ifndef FILL_INT_URB
#define FILL_INT_URB		usb_fill_int_urb
#endif
#ifndef USB_QUEUE_BULK
#define USB_QUEUE_BULK		0
#endif

#define DEBUG			0

#if DEBUG
#define trace(format, arg...)	printk(format "\n", ## arg)
#else
#define trace(format, arg...)
#endif

#define RXQ_LEN			10

typedef struct {
	uint32 notification;
	uint32 reserved;
} intr_t;

typedef struct {
	int open_count;		/* Currently can open device only one time */
	int disconnected;	/* Disconnected but still open */

	struct usb_device *usb;
	struct urb intr_urb;
	uint rx_pipe, tx_pipe, intr_pipe;
	struct sk_buff_head rxq, txq;		/* txq currently unneeded */
	wait_queue_head_t intr_wait;
	intr_t intr;
} rst_t;

/*
 * Only one remote socket device can be plugged in at a time.
 * Keep a global pointer to that device, with a mutex to protect it.
 */
static rst_t *rst;
static DECLARE_MUTEX (rst_mutex);

static void rst_tx_complete(struct urb *urb);
static void rst_rx_complete(struct urb *urb);
static int rst_rx_submit(rst_t *rst);

static void rst_disconnect(struct usb_device *usb, void *ptr);

static struct usb_driver rst_driver;

os_pkt_t *
rst_pkt_alloc(int len)
{
	struct sk_buff *skb;

	/* Allocate packet */
	if ((skb = dev_alloc_skb(len)) == NULL) {
		err("rst_data_alloc: dev_alloc_skb failed");
		return NULL;
	}

	skb->len = len;

	return (os_pkt_t *)skb;
}

void
rst_pkt_free(os_pkt_t *pkt)
{
	struct sk_buff *skb = (struct sk_buff *)pkt;

	dev_kfree_skb_any(skb);
}

void *
rst_pkt_data(os_pkt_t *pkt)
{
	struct sk_buff *skb = (struct sk_buff *)pkt;

	return skb->data;
}

int
rst_pkt_len(os_pkt_t *pkt)
{
	struct sk_buff *skb = (struct sk_buff *)pkt;

	return skb->len;
}

static void
rst_tx_complete(struct urb *urb)
{
	struct sk_buff *skb = (struct sk_buff *)urb->context;
	unsigned long flags;

	trace("rst_tx_complete");

	if (rst == NULL) {
		err("rst_tx_complete: device not running!");
		return;
	}

	spin_lock_irqsave(&rst->txq.lock, flags);

	if (skb->list)
		/* Remove from queue */
		__skb_unlink(skb, skb->list);

	spin_unlock_irqrestore(&rst->txq.lock, flags);

	dev_kfree_skb_any(skb);
	usb_free_urb(urb);
}

#if DEBUG
/* pretty hex print a contiguous buffer */
static void
prhex(char *msg, uchar *buf, uint nbytes)
{
	char line[256];
	char *p;
	uint i;

	if (msg && (msg[0] != '\0'))
		printk("%s:\n", msg);

	p = line;
	for (i = 0; i < nbytes; i++) {
		if (i % 16 == 0) {
			p += sprintf(p, "  %04d: ", i);	/* line prefix */
		}
		p += sprintf(p, "%02x ", buf[i]);
		if (i % 16 == 15) {
			printk("%s\n", line);		/* flush line */
			p = line;
		}
	}

	/* flush last partial line */
	if (p != line)
		printk("%s\n", line);
}
#endif

int
rst_pkt_output(os_pkt_t *pkt)
{
	struct sk_buff *skb = (struct sk_buff *)pkt;
	struct urb *urb;
	int ret;
	unsigned long flags;

	trace("rst_data_output");

#if DEBUG
	printk("  len=%d\n", skb->len);
	prhex(NULL, skb->data, skb->len);
#endif

	/*
	 * USB says that if the transmit packet length is a multiple of the endpoint
	 * size (64), we must follow up with a zero-length packet.  The dongle gets
	 * confused if this is not done.  It's easiest just to avoid sending 64-byte
	 * packets since the true length is in the rsock protocol header.
	 */
	if ((skb->len % 64) == 0)
		skb_put(skb, 1);		/* Bump up length */

	if (rst == NULL) {
		err("rst_data_output: rsock not running");
		dev_kfree_skb(skb);
		return -ENODEV;
	}

	/* Allocate URB */
	if (!(urb = usb_alloc_urb(0))) {
		err("rst_data_output: usb_alloc_urb failed");
		dev_kfree_skb(skb);
		return -ENOMEM;
	}

	/* Not using SKBs with actual network device */
	skb->dev = NULL;

	/* Save pointer to URB */
	*((struct urb **)skb->cb) = urb;

	FILL_BULK_URB(urb, rst->usb, rst->tx_pipe,
		      skb->data, skb->len, rst_tx_complete, skb);
	urb->transfer_flags |= USB_QUEUE_BULK;

	spin_lock_irqsave(&rst->txq.lock, flags);

	if ((ret = usb_submit_urb(urb)) != 0) {
		spin_unlock_irqrestore(&rst->txq.lock, flags);
		err("rst_data_output: usb_submit_urb failed with status %d", ret);
		dev_kfree_skb(skb);
		usb_free_urb(urb);
		return ret;
	}

	/* Enqueue packet */
	__skb_queue_tail(&rst->txq, skb);

	spin_unlock_irqrestore(&rst->txq.lock, flags);

	return 0;
}

static ssize_t
rst_write(struct file *file, const char *buffer, size_t count, loff_t *ppos)
{
	char cmd[80];
	int i, rv = count;

	trace("rst_write: count=%d", count);

	down(&rst_mutex);

	/* Verify that the device wasn't unplugged */
	if (rst->disconnected) {
		rv = -ENODEV;
		goto done;
	}

	if (count > sizeof(cmd) - 1)
		count = (int)sizeof(cmd) - 1;

	if (copy_from_user(cmd, buffer, count)) {
		rv = -EFAULT;
		goto done;
	}

	cmd[count] = 0;

	for (i = 0; cmd[i] != 0; i++)
		if (cmd[i] == '\n') {
			cmd[i] = 0;
			break;
		}

	if (rstest_cmd(cmd) < 0)
		rv = -EINVAL;

 done:
	up(&rst_mutex);
	return rv;

}

static ssize_t
rst_read(struct file *file, char *buffer, size_t count, loff_t *ppos)
{
	(void)buffer;
	(void)count;
	(void)ppos;

	down(&rst_mutex);

	/* Verify that the device wasn't unplugged */
	if (rst->disconnected) {
		up(&rst_mutex);
		return -ENODEV;
	}

	up(&rst_mutex);

	return 0;
}

static void
rst_rx_complete(struct urb *urb)
{
	struct sk_buff *skb = (struct sk_buff *)urb->context;
	int ret = 0;

	trace("rst_rx_complete");

	/* Remove from queue */
	skb_unlink(skb);

	if (urb->status) {
		err("rst_rx_complete: rx error %d", urb->status);
		dev_kfree_skb_any(skb);
		ret = urb->status;
		goto done;
	}

	skb->len = urb->actual_length;

	//	printk("len=%d\n",skb->len);
	trace("rst_rx_complete: input data=%p len=%d", skb->data, skb->len);

	rsock_input((os_pkt_t *)skb);

 done:
	usb_free_urb(urb);

	if (!rst->disconnected && ret == 0)
		rst_rx_submit(rst);
}

static int
rst_rx_submit(rst_t *rst)
{
	int len = USB_RSTEST_MTU;
	struct urb *urb;
	struct sk_buff *skb;
	int ret = 0;
	unsigned long flags;

	/* Called from interrupt context */

	trace("rst_rx_submit");

	spin_lock_irqsave(&rst->rxq.lock, flags);

	while (skb_queue_len(&rst->rxq) < RXQ_LEN) {
		/* Allocate URB for this packet */
		if (!(urb = usb_alloc_urb(0))) {
			err("rst_rx_submit: usb_alloc_urb failed");
			ret = -ENOMEM;
			break;
		}

		/* Allocate packet */
		if (!(skb = dev_alloc_skb(len))) {
			err("rst_rx_submit: dev_alloc_skb failed");
			usb_free_urb(urb);
			ret = -ENOMEM;
			break;
		}

		/* Not using SKBs with actual network device */
		skb->dev = NULL;

		/* Save pointer to URB */
		*((struct urb **)skb->cb) = urb;

		FILL_BULK_URB(urb, rst->usb, rst->rx_pipe,
			      skb->data, len, rst_rx_complete, skb);
		urb->transfer_flags |= USB_QUEUE_BULK;

		if ((ret = usb_submit_urb(urb))) {
			err("rst_rx_submit: usb_submit_urb failed with status %d", ret);
			usb_free_urb(urb);
			dev_kfree_skb_any(skb);
			break;
		}

		/* Enqueue packet */
		__skb_queue_tail(&rst->rxq, skb);
	}

	spin_unlock_irqrestore(&rst->rxq.lock, flags);
 
	return ret;
}

static void
rst_intr_complete(struct urb *urb)
{
	trace("rst_intr_complete");

	if (waitqueue_active(&rst->intr_wait))
		wake_up_interruptible(&rst->intr_wait);
}

static int
rst_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	(void)inode;
	(void)cmd;
	(void)arg;

	down(&rst_mutex);

	/* Verify that the device wasn't unplugged */
	if (rst->disconnected) {
		up(&rst_mutex);
		return -ENODEV;
	}

	up(&rst_mutex);

	return -EINVAL;
}

static int
rst_open(struct inode *inode, struct file *file)
{
	trace("rst_open");

	if (MINOR(inode->i_rdev) != USB_RSTEST_MINOR_BASE)
		return -ENODEV;

	down(&rst_mutex);

	if (rst == NULL || rst->disconnected) {
		up(&rst_mutex);
		return -ENODEV;
	}

	rst->open_count++;

	up(&rst_mutex);

	return 0;
}

static void
rst_unlink(struct sk_buff_head *q)
{
	struct sk_buff *skb;
	struct urb *urb;

	trace("rst_unlink: head=%p", (void *)q);

	while ((skb = skb_dequeue(q))) {
		urb = *((struct urb **) skb->cb);
		usb_unlink_urb(urb);
	}
}

static void
rst_delete(void)
{
	if (rst == NULL) {
		err("rst_delete: fatal; no device!");
		return;
	}

	if (!rst->disconnected) {
		err("rst_delete: fatal; device still connected\n");
		return;
	}

	/* Finalize rsock */
	rsock_term();

	/* Cancel pending URBs */
	usb_unlink_urb(&rst->intr_urb);

	rst_unlink(&rst->txq);
	rst_unlink(&rst->rxq);

	usb_driver_release_interface(&rst_driver, &rst->usb->actconfig->interface[0]);
	usb_driver_release_interface(&rst_driver, &rst->usb->actconfig->interface[1]);	
	usb_dec_dev_use(rst->usb);

	kfree(rst);

	rst = NULL;

	MOD_DEC_USE_COUNT;
}

static int
rst_release(struct inode *inode, struct file *file)
{
	trace("rst_release");

	if (rst == NULL)
		return -ENODEV;

	down(&rst_mutex);

	if (rst->open_count == 0) {
		trace("rst_release: dev not open??");
		up(&rst_mutex);
		return -ENODEV;
	}

	rst->open_count--;

	if (rst->disconnected && rst->open_count == 0) {
		/* The device was unplugged before the file was released */
		trace("rst_release: freeing dev previously disconnected");
		rst_delete();
	}

	up(&rst_mutex);

	trace("rst_release: done");

	return 0;
}

/*
 * rsock begins as soon as the USB device is connected.
 * The device is only used to pass in commands.
 */
static void *
rst_probe(struct usb_device *usb, unsigned int ifnum,
	    const struct usb_device_id *id)
{
	struct usb_interface_descriptor *interface;
	struct usb_endpoint_descriptor *endpoint;
	int interval, ret, intr_size;
	int i;

	trace("rst_probe");

	down(&rst_mutex);

	if (rst != NULL) {
		err("rst_probe: only one device supported\n");
		up(&rst_mutex);
		return NULL;
	}

	/* Allocate private state */
	if (!(rst = kmalloc(sizeof(rst_t), GFP_KERNEL))) {
		err("rst_probe: kmalloc failed");
		up(&rst_mutex);
		return NULL;
	}

	memset(rst, 0, sizeof(rst_t));

	usb_inc_dev_use(usb);

	rst->usb = usb;
	init_waitqueue_head(&rst->intr_wait);

	skb_queue_head_init(&rst->rxq);
	skb_queue_head_init(&rst->txq);

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
	rst->intr_pipe = usb_rcvintpipe(usb, endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);

	/* Claim control interface */
	usb_driver_claim_interface(&rst_driver, &usb->actconfig->interface[0], rst);

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
			rst->rx_pipe = usb_rcvbulkpipe(usb, endpoint->bEndpointAddress &
			                               USB_ENDPOINT_NUMBER_MASK);
		else
			rst->tx_pipe = usb_sndbulkpipe(usb, endpoint->bEndpointAddress &
			                               USB_ENDPOINT_NUMBER_MASK);
	}

	/* Claim data interface */
	usb_driver_claim_interface(&rst_driver, &usb->actconfig->interface[1], rst);

	/* Initialize rsock */
	trace("rst_probe: calling rsock_init");

	if (rsock_init() < 0) {
		err("rst_probe: rsock_init failed");
		goto fail;
	}

	/* Start receiving packets */
	if (rst_rx_submit(rst))
		goto fail;

	/* RNDIS spec says to use an 8-byte intr but our old drivers used 4 bytes for intr */
	if (rst->usb->actconfig->interface[0].altsetting[0].endpoint[0].wMaxPacketSize == 16)
		intr_size = 8;
	else
		intr_size = 4;

	/* Start polling interrupt endpoint */
	interval = rst->usb->actconfig->interface[0].altsetting[0].endpoint[0].bInterval;

	if (rst->usb->speed == USB_SPEED_HIGH)
		interval = 1 << (interval - 1);

	FILL_INT_URB(&rst->intr_urb, rst->usb, rst->intr_pipe,
		     &rst->intr, intr_size, rst_intr_complete, rst,
		     interval);

	trace("rst_probe: submit intr_urb");

	if ((ret = usb_submit_urb(&rst->intr_urb))) {
		err("rst_probe: usb_submit_urb failed with status %d", ret);
		goto fail;
	}

	trace("rst_probe: done");

	MOD_INC_USE_COUNT;

	up(&rst_mutex);
	return rst;

 fail:
	rst->disconnected = 1;
	rst_delete();
	up(&rst_mutex);
	return NULL;
}

static void
rst_disconnect(struct usb_device *usb, void *ptr)
{
	(void)ptr;	/* Using global rst */

	trace("rst_disconnect");

	down(&rst_mutex);

	rst->disconnected = 1;

	if (rst->open_count > 0) {
		trace("rst_disconnect: still open");
		up(&rst_mutex);
		/* rst_release will finish cleanup when device is closed */
		return;
	}

	/* Device is not opened; clean up now */
	trace("rst_disconnect: freeing now");

	rst_delete();

	up(&rst_mutex);
}

static struct usb_device_id cdc_table[] = {
	{ USB_DEVICE(0x0a5c, 0x0cdc) },
	{ }
};

MODULE_DEVICE_TABLE(usb, cdc_table);

static struct
file_operations rst_fops = {
	.owner =	THIS_MODULE,
	.read =		rst_read,
	.write =	rst_write,
	.ioctl =	rst_ioctl,
	.open =		rst_open,
	.release =	rst_release,
};

static struct usb_driver rst_driver = {
	name:		"rst",
	probe:		rst_probe,
	disconnect:	rst_disconnect,
	id_table:	cdc_table,
	fops:		&rst_fops,
	minor:		USB_RSTEST_MINOR_BASE,
};

static int __init
rst_module_init(void)
{
	trace("rst_module_init");

	return usb_register(&rst_driver);
}

static void __exit
rst_module_cleanup(void)
{
	trace("rst_module_cleanup");

	usb_deregister(&rst_driver);
}

module_init(rst_module_init);
module_exit(rst_module_cleanup);
