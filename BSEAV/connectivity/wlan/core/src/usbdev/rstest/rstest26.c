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
#include <linux/version.h>

#include <typedefs.h>

#include <bcmrsock/plumb.h>

#include "rstest_cmd.h"

#define USB_RSTEST_MINOR_BASE	144

#define USB_RSTEST_MTU		1536	/* Must be larger than RSOCK_BUS_MTU */

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

	struct usb_device *dev;
	struct usb_interface *intf;
	struct urb *intr_urb;
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

static void rst_tx_complete(struct urb *urb, struct pt_regs *regs);
static void rst_rx_complete(struct urb *urb, struct pt_regs *regs);
static int rst_rx_submit(rst_t *rst);

static void rst_disconnect(struct usb_interface *intf);

static struct usb_class_driver rst_class;
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
rst_tx_complete(struct urb *urb, struct pt_regs *regs)
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
	if (!(urb = usb_alloc_urb(0, GFP_ATOMIC))) {
		err("rst_data_output: usb_alloc_urb failed");
		dev_kfree_skb(skb);
		return -ENOMEM;
	}

	/* Not using SKBs with actual network device */
	skb->dev = NULL;

	/* Save pointer to URB */
	*((struct urb **)skb->cb) = urb;

	usb_fill_bulk_urb(urb, rst->dev, rst->tx_pipe,
		      skb->data, skb->len, rst_tx_complete, skb);

	spin_lock_irqsave(&rst->txq.lock, flags);

	if ((ret = usb_submit_urb(urb, GFP_ATOMIC)) != 0) {
		spin_unlock_irqrestore(&rst->txq.lock, flags);
		err("rst_data_output: usb_submit_urb failed");
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
rst_rx_complete(struct urb *urb, struct pt_regs *regs)
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

	trace("rst_rx_complete: input data=%p len=%d", skb->data, skb->len);

	rsock_input((os_pkt_t *)skb);

 done:
	usb_free_urb(urb);

	/*
	 * Submit another URB, unless disconnected or error.  On Linux 2.6, when the cable
	 * is disconnected, all RX URBs start completing with status -EILSEQ.  Then when
	 * the host USB core detects the disconnect, they start failing with -ESHUTDOWN.
	 * We don't resubmit on errors since it creates a loop.
	 */

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
		if (!(urb = usb_alloc_urb(0, GFP_ATOMIC))) {
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

		usb_fill_bulk_urb(urb, rst->dev, rst->rx_pipe,
			      skb->data, len, rst_rx_complete, skb);

		if ((ret = usb_submit_urb(urb, GFP_ATOMIC)) != 0) {
			err("rst_rx_submit: usb_submit_urb failed");
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
rst_intr_complete(struct urb *urb, struct pt_regs *regs)
{
	trace("rst_intr_complete");

	if (waitqueue_active(&rst->intr_wait))
		wake_up_interruptible(&rst->intr_wait);

	if (rst != NULL && !rst->disconnected)
		usb_submit_urb(urb, GFP_ATOMIC);

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
	unsigned long flags;
	int n;

	if (rst == NULL) {
		err("rst_delete: fatal; no device!");
		return;
	}

	if (!rst->disconnected) {
		err("rst_delete: fatal; device still connected");
		return;
	}

	/* Finalize rsock */
	rsock_term();

	/* Cancel pending URBs */
	usb_unlink_urb(rst->intr_urb);
	usb_free_urb(rst->intr_urb);
	rst->intr_urb = NULL;

	rst_unlink(&rst->txq);
	rst_unlink(&rst->rxq);

	trace("rst_delete: wait for URB completion");

	/* Wait for outstanding URBs to complete */

	for (;;) {
		n = 0;

		spin_lock_irqsave(&rst->rxq.lock, flags);
		n += skb_queue_len(&rst->rxq);
		spin_unlock_irqrestore(&rst->rxq.lock, flags);

		spin_lock_irqsave(&rst->txq.lock, flags);
		n += skb_queue_len(&rst->txq);
		spin_unlock_irqrestore(&rst->txq.lock, flags);

		if (n == 0)
			break;

		current->state = TASK_INTERRUPTIBLE;
		schedule_timeout(1);
	}

	trace("rst_delete: release data i/f");

	usb_set_intfdata(rst->intf, NULL);

	kfree(rst);

	rst = NULL;

	trace("rst_delete: done");
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
static int
rst_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_interface *ifctrl, *ifdata;
	struct usb_host_interface *ihctrl, *ihdata;
	struct usb_host_config *actconfig;
	struct usb_endpoint_descriptor *endpoint;
	int interval, ret = -EIO, intr_size, i, claimed = 0;

	trace("rst_probe: intf=%p", (void *)intf);

	down(&rst_mutex);

	if (rst != NULL) {
		err("rst_probe: only one device supported");
		up(&rst_mutex);
		return -ENXIO;
	}

	/* Allocate private state */
	if (!(rst = kmalloc(sizeof(rst_t), GFP_KERNEL))) {
		err("rst_probe: kmalloc failed");
		up(&rst_mutex);
		return -ENOMEM;
	}

	memset(rst, 0, sizeof(rst_t));

	rst->dev = dev;
	rst->intf = intf;

	init_waitqueue_head(&rst->intr_wait);

	skb_queue_head_init(&rst->rxq);
	skb_queue_head_init(&rst->txq);

	/* Check that the device supports only one configuration */
	if (dev->descriptor.bNumConfigurations != 1) {
		err("bNumConfigurations %d != 1", dev->descriptor.bNumConfigurations);
		goto fail;
	}

	actconfig = dev->actconfig;

	/* Check that the configuration supports two interfaces */
	if (actconfig->desc.bNumInterfaces != 2) {
		err("bNumInterfaces %d != 1", actconfig->desc.bNumInterfaces);
		goto fail;
	}

	/*
	 * Check control interface.
	 * We should have been probed with it; it is already claimed.
	 */
	ifctrl = actconfig->interface[0];
	ihctrl = &ifctrl->altsetting[0];
	trace("rst_probe: ifctrl=%p ihctrl=%p", (void *)ifctrl, (void *)ihctrl);
	if (ihctrl->desc.bInterfaceClass != 2 ||
	    ihctrl->desc.bInterfaceSubClass != 2 ||
	    ihctrl->desc.bInterfaceProtocol != 0xff ||
	    ihctrl->desc.bNumEndpoints != 1) {
		err("control interface values %d %d %d %d",
		    ihctrl->desc.bInterfaceClass,
		    ihctrl->desc.bInterfaceSubClass,
		    ihctrl->desc.bInterfaceProtocol,
		    ihctrl->desc.bNumEndpoints);
		goto fail;
	}

	/* Check control endpoint */
	endpoint = &ihctrl->endpoint[0].desc;
	if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) != USB_ENDPOINT_XFER_INT) {
		err("control endpoint attr");
		goto fail;
	}

	rst->intr_pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK);

	/* Check data interface */
	ifdata = actconfig->interface[1];
	ihdata = &ifdata->altsetting[0];
	trace("rst_probe: ifdata=%p ihdata=%p", (void *)ifdata, (void *)ihdata);
	if (usb_interface_claimed(ifdata) ||
	    ihdata->desc.bInterfaceClass != 10 ||
	    ihdata->desc.bInterfaceSubClass != 0 ||
	    ihdata->desc.bInterfaceProtocol != 0 ||
	    ihdata->desc.bNumEndpoints != 2) {
		err("data interface values %d %d %d %d",
		    ihdata->desc.bInterfaceClass,
		    ihdata->desc.bInterfaceSubClass,
		    ihdata->desc.bInterfaceProtocol,
		    ihdata->desc.bNumEndpoints);
		goto fail;
	}

	/* Check data endpoints */
	for (i = 0; i < 2; i++) {
		endpoint = &ihdata->endpoint[i].desc;
		if ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) !=
		    USB_ENDPOINT_XFER_BULK) {
			err("data endpoint attr");
			goto fail;
		}

		if ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
			rst->rx_pipe = usb_rcvbulkpipe(dev, endpoint->bEndpointAddress &
			                               USB_ENDPOINT_NUMBER_MASK);
		else
			rst->tx_pipe = usb_sndbulkpipe(dev, endpoint->bEndpointAddress &
			                               USB_ENDPOINT_NUMBER_MASK);
	}

	/* Control interface is already claimed */
	usb_set_intfdata(intf, rst);

	/* Claim data interface */
	usb_driver_claim_interface(&rst_driver, ifdata, rst);
	claimed = 1;

	/* Initialize rsock */
	trace("rst_probe: calling rsock_init");

	if (rsock_init() < 0) {
		err("rst_probe: rsock_init failed");
		goto fail;
	}

	/* Start receiving packets */
	if ((ret = rst_rx_submit(rst)) != 0)
		goto fail;

	/* RNDIS spec says to use an 8-byte intr but our old drivers used 4 bytes for intr */
	if (ihctrl->endpoint[0].desc.wMaxPacketSize == 16)
		intr_size = 8;
	else
		intr_size = 4;

	/* Start polling interrupt endpoint */
	interval = ihctrl->endpoint[0].desc.bInterval;

	if (rst->dev->speed == USB_SPEED_HIGH)
		interval = 1 << (interval - 1);

	trace("rst_probe: intr_size=%d interval=%d rx_pipe=%x tx_pipe=%x intr_pipe=%x",
	      intr_size, interval, rst->rx_pipe, rst->tx_pipe, rst->intr_pipe);

	if (!(rst->intr_urb = usb_alloc_urb(0, GFP_ATOMIC))) {
		err("rst_probe: usb_alloc_urb intr failed");
		ret = -ENOMEM;
		goto fail;
	}

	usb_fill_int_urb(rst->intr_urb, rst->dev, rst->intr_pipe,
	                 &rst->intr, intr_size, rst_intr_complete, rst,
	                 interval);

	trace("rst_probe: submit intr_urb");

	if ((ret = usb_submit_urb(rst->intr_urb, GFP_ATOMIC)) != 0) {
		err("rst_probe: usb_submit_urb failed");
		goto fail;
	}

	if ((ret = usb_register_dev(intf, &rst_class)) != 0) {
		err("rst_probe: usb_register_dev failed");
		goto fail;
	}

	trace("rst_probe: done");

	up(&rst_mutex);
	return 0;

 fail:
	if (claimed)
		usb_driver_release_interface(&rst_driver, ifdata);

	if (ret)
		err("rst_probe: error status %d", ret);

	rst->disconnected = 1;
	rst_delete();

	up(&rst_mutex);

	return ret;
}

static void
rst_disconnect(struct usb_interface *intf)
{
	trace("rst_disconnect: intf=%p", (void *)intf);

	down(&rst_mutex);

	if (rst == NULL || intf != rst->intf) {
		/* This routine is called once for control I/F and once for data I/F */
		up(&rst_mutex);
		return;
	}

	rst->disconnected = 1;

	if (rst->open_count > 0) {
		trace("rst_disconnect: still open");
		up(&rst_mutex);
		/* rst_release will finish cleanup when device is closed */
		return;
	}

	/* give back our minor */
	trace("rst_disconnect: deregister dev");
	usb_deregister_dev(intf, &rst_class);

	/* Device is not opened; clean up now */
	trace("rst_disconnect: freeing now");

	rst_delete();

	up(&rst_mutex);

	trace("rst_disconnect: done");
}

static struct usb_device_id cdc_table[] = {
	{ USB_DEVICE(0x0a5c, 0x0cdc) },
	{ }
};

MODULE_DEVICE_TABLE(usb, cdc_table);

static struct file_operations rst_fops = {
	owner:		THIS_MODULE,
	read:		rst_read,
	write:		rst_write,
	ioctl:		rst_ioctl,
	open:		rst_open,
	release:	rst_release,
};

static struct usb_class_driver rst_class = {
	name:		"usb/rst%d",
	fops:		&rst_fops,
	mode:		S_IFCHR | S_IRUGO | S_IWUGO,
	minor_base:	USB_RSTEST_MINOR_BASE,
};

static struct usb_driver rst_driver = {
	owner:		THIS_MODULE,
	name:		"rst",
	probe:		rst_probe,
	disconnect:	rst_disconnect,
	id_table:	cdc_table,
};

static int __init
rst_module_init(void)
{
	int en;

	trace("rst_module_init");

	init_MUTEX(&rst_mutex);

	if ((en = usb_register(&rst_driver)) != 0) {
		err("rst_module_init: usb_register failed (errno=%d)", en);
		return en;
	}

	return 0;
}

static void __exit
rst_module_exit(void)
{
	trace("rst_module_exit");

	usb_deregister(&rst_driver);
}

module_init(rst_module_init);
module_exit(rst_module_exit);

MODULE_AUTHOR("Broadcom Corp.");
MODULE_DESCRIPTION("Remote Sockets Test");
