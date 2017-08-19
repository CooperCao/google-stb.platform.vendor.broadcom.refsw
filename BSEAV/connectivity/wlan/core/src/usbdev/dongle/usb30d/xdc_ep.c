/*
 * xDC EPs configuration
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: xdc_ep.c 406060 2013-06-05 23:13:52Z $
*/

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <usb.h>
#include <usbdev.h>
#include <usbdev_dbg.h>
#include "xdc.h"
#include "xdc_rte.h"


#ifdef USB_XDCI

static void *xdc_align_ctrlpkt(struct xdcd *xdc, struct xdc_ep *ep, void *p, uint32 align);


/** returns address for an internal register for the caller supplied endpoint */
static uint32 xdc_get_EPEXT_Addr(struct xdcd *xdc, int ep_num, int dir)
{
	uint32 regAddr = 0;

	if (xdc->speed == XDC_PORT_SS) { /* internal 'endpoint configuration' register */
		regAddr = ep_num *0x20 + EP_EXTREG_OFFSET;
		if (dir == UE_DIR_OUT) /* direction: host->dongle */
			regAddr += 0x200;

		regAddr += XDC_EXTREG_BASE;
	} else {
		regAddr = ep_num *0x4 + EP_EXT2REG_OFFSET;
		if (dir == UE_DIR_OUT)
			regAddr += 0x40;

		regAddr += XDC_EXT2REG_BASE; /* += 0x2800, 2.0 MAC internal regs base address */
	}

	return regAddr;
}

/** writes to internal 'endpoint configuration' register */
void xdc_set_SeqN(struct xdcd *xdc, int ep_num, int dir, int seqn)
{
	uint32 temp;
	uint32 regAddr = 0;

	xdc_dbg(("ep_num = %d, dir = %x \n", ep_num, dir));

	temp = EP_STATUS_ACTIVE;
	if (xdc->speed == XDC_PORT_SS) {
		temp |= (seqn<<EP_SEQN_BITS)&EP_SEQN_MASK;
		temp |= EP_SEQN_LOAD;
	} else {
		temp |= (seqn<<EP2_SEQN_BITS);
	}

	regAddr = xdc_get_EPEXT_Addr(xdc, ep_num, dir);

	xdc_dbg(("regAddr = %x, seqn = %d, temp = %x \n", regAddr, seqn, temp));
	W_REG(xdc->osh, (uint32 *)XDC_INDIRECT_ADDR, regAddr);
	OSL_DELAY(2);
	W_REG(xdc->osh, (uint32 *)XDC_INDIRECT_DATA, temp);
	OSL_DELAY(2);
}


/**  This is called by ch_init() to initialize 32 endpoints. Do we need so many? */
int xdc_ep_init(struct xdcd *xdc)
{
	uint8 i;
	struct xdc_ep *ep;

	xdc_trace(("xdc_ep_init\n"));

	for (i = 0; i < EP_MAX_IDX; i++) {
		ep = &xdc->ep[i];
		ep->xdc = xdc;
		ep->num = i/2;
		ep->dir = 0;
		ep->index = i;
		ep->state = 0;
		ep->maxpacket = 512;
		ep->ring_reactive = 0;
		ep->urb_head = NULL;
		ep->urb_tail = NULL;
		ep->lb_head = NULL;
		ep->lb_tail = NULL;
		ep->urb = NULL;
		ep->urb_wr = 0;
	}

	xdc->ep[0].evt_fn = xdc_ctrl_event;
	xdc->ep[0].done_fn = xdc_urb_ctrl_done;

	return 0;
}

/** Stop all endpoints except endpoint 0 */
void xdc_stop_eps(struct xdcd *xdc)
{
	uint32 ep_index;
	struct xdc_epctx *epctx;
	struct xdc_ep *ep;

	for (ep_index = 1; ep_index < EP_MAX_IDX; ep_index++) {
		ep = &xdc->ep[ep_index];
		epctx = (struct xdc_epctx *)(xdc->outctx) + 1 + ep_index;

		xdc_dbg(("stop_eps -- epctx->info1 = %d\n", epctx->info1));

		if ((!(ep->state & XDC_EP_ENABLED)) &&
			((epctx->info1&0x7) == EP_DISABLED ||
			(epctx->info1&0x7) == EP_STOPPED))
			continue;

		xdc_dbg(("stop_eps -- epctx->info1 = %d\n", epctx->info1));
		xdc_dbg(("disable ep=%p ep_index=%d \n", ep, ep_index));
		xdc_disable_ep(ep);
	}
}

/** Disable caller provided endpoint. Called after e.g. a USB bus reset */
int xdc_disable_ep(struct xdc_ep *ep)
{
	struct xdcd *xdc;
	struct xdc_epctx *epctx;
	uint32 ret;
	struct xdc_urb *urb;

	xdc_dbg(("xdc_disable_ep ep_index = %d\n", ep->index));

	xdc = ep->xdc;

	epctx = (struct xdc_epctx *)(xdc->outctx) + 1 + ep->index;
#ifdef XDC_DEBUG
	xdc_dump_ctx(xdc, XDC_DEVICE_CTX, ep->index, ep->index);
#endif /* XDC_DEBUG */
	xdc_dbg(("epctx->info1 = %d\n", epctx->info1));

	if ((epctx->info1&0x7) == EP_RUNNING) {
		/* Stop the endpoint ring  at first */
		ep->ring_reactive = 0;
		xdc_dbg(("stop endpoint = %d \n", ep->index));

		ret = xdc_cmd(xdc, ep, STOP_RING_TRB);
	}

#ifdef XDC_DEBUG
	xdc_dump_ctx(xdc, XDC_DEVICE_CTX, ep->index, ep->index);
#endif /* XDC_DEBUG */
	xdc_dbg(("stop ed done  epctx->info1 = %d\n", epctx->info1));

	urb = ep->urb_head;

	while (urb) {
		void *p;

		xdc_dbg(("urb is not empty, free urb list \n"));
		if (urb->trb) {
			xdc_dbg(("remove td from queue....%p\n", urb->trb));
			xdc_noop_trb(xdc, ep->xf_ring, urb);
		}
		p = urb->buf;
		urb = urb->next;
		PKTFREE(xdc->osh, p, FALSE);
	}

	ep->urb_head = ep->urb_tail = NULL;
	ret = xdc_remove_ep_ctx(xdc, ep);

	if (ret) {
		xdc_err(("err ret = %d epctx->info1 = %d\n", ret, epctx->info1));
	}

	xdc_free_ring(xdc, ep->xf_ring, ep->ring_sz);

	if (ep->urb) {
		MFREE(xdc->osh, ep->urb, ep->max_urb*sizeof(struct xdc_urb));
		ep->urb = NULL;
	}

	ep->urb_wr = 0;
	ep->type = 0;
	ep->state = 0;
	ep->xf_ring = 0;

	return ret;
} /* xdc_disable_ep */

static void bulkout_buffer_refill_cb(struct pktpool *pool, void *arg)
{
	struct xdc_ep *ep = (struct xdc_ep *) arg;
	if (ep->malloc_fail) {
		xdc_rx_malloc_chk(ep->xdc, ep);
	}
}

/** Called e.g. during firmware 'attach' phase */
int xdc_enable_ep(struct xdcd *xdc, const usb_endpoint_descriptor_t *desc,
	const usb_endpoint_companion_descriptor_t *sscmp)
{
	struct xdc_ep *ep;
	uint32 index, dir, num, type, i;
	int ret;

	xdc_trace(("xdc_enable_ep\n"));

	if (!desc || desc->bDescriptorType != UDESC_ENDPOINT /* || (!desc->wMaxPacketSize) */) {
		xdc_err(("invalid desc\n"));
		ASSERT(0);
		return -1;
	}

	xdc_trace(("xdc=%p xdc->speed=%d \n",	xdc, xdc->speed));

	if (xdc->speed == USB_SPEED_UNKNOWN) {
		return -2;
	}

	dir = (uint32)UE_GET_DIR(desc->bEndpointAddress);
	num = (uint32)UE_GET_ADDR(desc->bEndpointAddress);
	type = (uint32)UE_GET_XFERTYPE(desc->bmAttributes);

	index = num * 2;
	if (type != UE_CONTROL) {
		index = index + ((dir) ? 1 : 0) - 1;
	}

	if (index > 31) {
		xdc_err(("ep index is too big!!\n"));
		return -3;
	}

	ep = &xdc->ep[index];

	if (ep->state & XDC_EP_ENABLED) {

		xdc_dbg(("EP %d is already enabled\n", ep->index));
#ifdef XDC_DEBUG
		xdc_dump_ctx(xdc, XDC_DEVICE_CTX, ep->index, ep->index);
#endif /* XDC_DEBUG */
		return index;
	}

	ep->num = num;
	ep->maxpacket = desc->wMaxPacketSize;
	ep->intv = desc->bInterval;
	if (type == UE_INTERRUPT || type == UE_ISOCHRONOUS) {
		if (xdc->speed == XDC_PORT_FS) {
			ep->intv += 3;
		} else {
			if (ep->intv == 0)
				ep->intv = 1;
			ep->intv = 2 << (ep->intv - 1);
		}
	}

	ep->type = type;
	ep->index = index;
	ep->dir = dir;
	if (sscmp) {
	/* simultaneous BULK and endpoint 0 transfers cause skipping of ACK to SETUP status. */
	/* In the transfer ring itself the status ACK is skipped by xDC controller. */
		if ((dir == UE_DIR_OUT) && (ep->type == UE_BULK)) /* direction: host->dongle */
			ep->maxburst = 0;
		else
			ep->maxburst = sscmp->bMaxBurst;

		xdc_dbg(("ep maxburst = %d  ep->index:%d dir:%x ep->num:%x\n", ep->maxburst,
		ep->index, dir, ep->num));
	}

	if (type == UE_BULK || type == UE_INTERRUPT) {
		ep->evt_fn = xdc_bulk_event;
		ep->max_rxrsv_trb = xdc->max_rsv_trbs;

		if (dir == UE_DIR_IN) { /* direction: dongle->host */
			ep->done_fn = xdc_urb_bulkin_done;
			ep->ring_sz = xdc->trbs_per_txring;
			ep->max_urb = TX_MAX_URB;
			ep->blk_sz = MAX_TXBLK_SIZE;
		} else {
			ep->done_fn = xdc_urb_bulkout_done;
			ep->ring_sz = xdc->trbs_per_rxring;
			ep->max_urb = RX_MAX_URB;
			ep->blk_sz = xdc->max_rxbuf_sz;
		}
		if (type == UE_INTERRUPT) {
			ep->ring_sz = xdc->trbs_intr_txring;
			ep->max_urb = INTR_TX_MAX_URB;
			ep->max_rxrsv_trb = xdc->intr_max_rsv_trbs;
			ep->blk_sz = xdc->intr_max_rxbuf_sz;
		}
	} else if (type == UE_CONTROL) {
		ep->evt_fn = xdc_ctrl_event;
		ep->done_fn = xdc_urb_ctrl_done;
		ep->ring_sz = xdc->trbs_per_ctlring;
		ep->max_urb = EP0_MAX_URB;
	} else {
		ep->evt_fn = xdc_bulk_event;
		ep->max_rxrsv_trb = xdc->isoc_max_rsv_trbs;

		if (dir == UE_DIR_IN) { /* direction: dongle->host */
			ep->done_fn = xdc_urb_isocin_done;
			ep->ring_sz = xdc->trbs_iso_txring;
			ep->max_urb = ISOC_TX_MAX_URB;
			ep->blk_sz = xdc->isoc_max_rxbuf_sz;
		} else {
			ep->done_fn = xdc_urb_isocout_done;
			ep->ring_sz = xdc->trbs_iso_rxring;
			ep->max_urb = ISOC_RX_MAX_URB;
			ep->blk_sz = xdc->isoc_max_rxbuf_sz;
		}
	}

	xdc_dbg(("Enable EP index = %d, ep num = %d", ep->index, ep->num));


	/* config SLX and EPX */
	ret = xdc_config_ep(xdc, ep);
	if (ret)
		return -4;

	xdc_dbg(("ep->xf_ring=%p\n", ep->xf_ring));

	/* ep0 is enabled in set address function */
	if (ep->num == 0)
		return index;

	ret = xdc_add_ep_ctx(xdc, ep);
	if (ret)
		return ret;

	ep->state = XDC_EP_ENABLED;

	xdc_dbg(("ep->maxpacket=%d ep->num=%d ep->type=%d ep->state=%d\n",
		ep->maxpacket, ep->num, ep->type, ep->state));

	ep->malloc_fail = 0;
	/* can be freed on bus reset */
	ep->urb = MALLOC_NOPERSIST(xdc->osh, ep->max_urb*sizeof(struct xdc_urb));
	if (ep->urb) {
		bzero(ep->urb, ep->max_urb*sizeof(struct xdc_urb));
	}

	if (ep->dir == UE_DIR_OUT) { /* direction: host->dongle */
		W_REG(xdc->osh, (uint32 *)XDC_WAR_REG10, (uint32)ep);
		xdc_dbg(("prepare OUT xfer buf and TDs ring\n"));
		for (i = 0; i < ep->max_rxrsv_trb; i++) {
			if (xdc_queue_rxbuf(ep, xdc, NULL) != 0) {
				ep->malloc_fail ++;
			}
		}

		/* register buffer notification */
		if (type == UE_BULK || type == UE_ISOCHRONOUS)
		{
			int r;
			r = pktpool_avail_register(SHARED_POOL, bulkout_buffer_refill_cb, ep);
			if (r != 0)
				printf("pktpool_avail_register: FAIL ret %d ep %p\n", r, ep);
		}
	}

	OSL_DELAY(1);
	if (ret == 0)
		ret = index;

	xdc_dbg(("xdc_enable_ep done\n"));

	return ret;
} /* xdc_enable_ep */

/** Removes the ep configure from EP CTX and SLOT CTX. Called from ep_disable() */
int xdc_remove_ep_ctx(struct xdcd *xdc, struct xdc_ep *ep)
{
	struct xdc_ctx_icc *icc_ctx;
	struct xdc_epctx *ep_octx;
	struct xdc_epctx *ep_ictx;
	uint32 ep_index, dcf;
	int ret;
#ifdef XDC_DEBUG
	struct xdc_slx *slx;

	slx = (struct xdc_slx *)(xdc->outctx);
#endif
	ep_index = ep->index;
	dcf = 1 << (ep_index + 1);

	if (ep_index == 0)
		return 0;

	xdc_dbg(("xdc_remove_ep_ctx ep=%p \n", ep));
	xdc_dbg(("ep_index=%d  ", ep_index));
	xdc_dbg(("dcf_flag=%x  ", dcf));

	icc_ctx = (struct xdc_ctx_icc *)(xdc->inctx);
	ep_octx = (struct xdc_epctx *)(xdc->outctx) + ep_index + 1;
	ep_ictx = (struct xdc_epctx *)(xdc->inctx) + (ep_index + 2);

	xdc_dbg(("ep_outctx->info1=%x drop_flag =%x  \n",
		ltoh32(ep_octx->info1), dcf));

	xdc_dbg(("output-slx->dev_state = %x \n", slx->dev_state));

	if ((ltoh32(ep_octx->info1)&0x0F) == EP_DISABLED ||
	        ltoh32(icc_ctx->drop_ctx_flag)&dcf) {
		xdc_err(("EP is already disabled %p \n", ep));
		return 0;
	}

	icc_ctx->drop_ctx_flag = htol32(dcf);
	xdc_dbg(("icc_ctx->drop_ctx_flag =%x  \n", icc_ctx->drop_ctx_flag));

	icc_ctx->add_ctx_flag  = 0;
	ep_ictx->info1 = 0;
	ep_ictx->info2 = 0;
	ep_ictx->deq = 0;
	ep_ictx->deqh = 0;

	ret = xdc_cmd(xdc, ep, CONFIG_EP_TRB);

	return ret;
} /* xdc_remove_ep_ctx */

/** Endpoint context is a subset of device context. It is located in dongle memory. */
int xdc_add_ep_ctx(struct xdcd *xdc, struct xdc_ep *ep)
{
	struct xdc_ctx_icc *icc_ctx;
	uint32 acf, ep_index;
	int ret;

	/*         ep0 index = 0,   ep1out index = 1, ep1in index = 2 */
	/* slx = 0,  ep0 ctx = 1,      ep1out = 2,           ep1in = 3 */
	ep_index = ep->index;

	/* ep0 ctx is constructed in xdc_set_addr */
	if (ep_index == 0)
		return 0;

	/* slx = b0001,ep0 = b0010, ep1out = b0100,ep1in = b1000 */
	acf = 1<<(ep_index + 1);
	xdc_dbg(("add_ctx_flag =%x\n", acf));

	icc_ctx = (struct xdc_ctx_icc *)xdc->inctx;

	xdc_dbg(("ep_index=%d old icc_ctx->add_ctx_flag=%x \n", ep_index,
		icc_ctx->add_ctx_flag));

	if (ltoh32(icc_ctx->add_ctx_flag) & acf) {
	      xdc_dbg(("ep already enabled %d\n", ep_index));
		return 0;
	}

	icc_ctx->add_ctx_flag = htol32(acf);
	icc_ctx->drop_ctx_flag = 0;
	xdc_dbg(("new icc_ctx->add_ctx_flag=%x\n",  icc_ctx->add_ctx_flag));

	/* Issue a configure endpoint command and return when it's complete */
	ret = xdc_cmd(xdc, ep, CONFIG_EP_TRB);

	return ret;
} /* xdc_add_ep_ctx */

/** Both NAK and STALL are USB packet types. Function name looks like a misnomer. */
int xdc_send_NAK(struct xdcd *xdc)
{
	int ret;

	if (xdc->ctrl_pipe_state == CTRL_PIPE_IDLE) {
		xdc->ctrl_nak = 1;
		return 0;
	}

	xdc->ctrl_nak = 0;
	 /* Halt the EP0 to send STALL packet */
	ret = xdc_halt_ep(&(xdc->ep[0]), 1);

	xdc->ctrl_pipe_state = CTRL_PIPE_IDLE;

	return ret;
}

static int xdc_recv_ctrldata(struct xdcd *xdc,
                             usb_device_request_t *ctrl, void *p)
{
	uint32 len, wr, padsize;
	struct xdc_ep *ep;

	xdc_dbg(("xdc_recv_ctrldata\n"));

	if (p == NULL) {
		len = (uint32) htol16(ctrl->wLength);

		padsize = BCMDONGLEPADSZ + BCMDONGLEHDRSZ + sizeof(uint32);
		p = PKTGET(xdc->osh, (len+3 + padsize)&0xFFFFFFFC, FALSE);
		if (p == NULL) {
			xdc_err(("xdc_recv_ctrldata malloc failed \n"));
			return NULL;
		}

		PKTPULL(xdc->osh, p, BCMDONGLEPADSZ);
		PKTSETLEN(xdc->osh, p, len);
	} else {
		len = PKTLEN(xdc->uxdc->osh, p);
	}

	ep = &xdc->ep[0];
	wr = ep->urb_wr;

	xdc->ep0_urb[wr].buf = p; /* point back to lbuf struct */
	xdc->ep0_urb[wr].trb_num = 0;
	xdc->ep0_urb[wr].lbuf = 1;
	xdc->ep0_urb[wr].actual = 0;
	xdc->ep0_urb[wr].trbcc = 0;
	xdc->ep0_urb[wr].trb = NULL;
	xdc->ep0_urb[wr].length = len;

	xdc->ctrl_xfer = 1;

	return 0;
}

/** A Broadcom specific setup packet was received from the host */
static int xdc_sendup_request(struct xdcd *xdc,
           usb_device_request_t *ctrl)
{
	struct xdc_ep *ep;
	void *p, *buf, *p1;
	int err = 0;
	int dir, wr;

	xdc_dbg(("xdc_sendup_request\n"));

	/* allocate LBUF size and copy ctrl to data */
	if (xdc->stppkt_lbuf == NULL) {
		p = PKTGET(xdc->osh, sizeof(usb_device_request_t), FALSE);
		if (p) {
			PKTSETLEN(xdc->osh, p, sizeof(usb_device_request_t));
			xdc->stppkt_lbuf = p;
		} else {
			xdc_err(("%s:PKTGET failed.\n",__FUNCTION__));
		}

	} else {
		p = xdc->stppkt_lbuf;
	}

	if (p == NULL)
		return NULL;

	buf = PKTDATA(xdc->uxdc->osh, p);

	bcopy((void *)ctrl, buf, sizeof(usb_device_request_t));

	p1 = usbdev_setup(xdc->uxdc->bus, 0, p, &err, &dir);

	bzero(xdc->stppkt_lbuf, sizeof(usb_device_request_t));

	xdc->ctrl_xfer = 0;
	/* Error processing request */
	if (!p1) {
		xdc_dbg(("xdc no setup return buffer from cdc = %d\n", err));
	} else if (p1 != p) {
		if (xdc->ep0_in) {
			/* Control IN: Request processed and data to send */
			/* Setup interrupt or successful tx disables or stops tx engine */
			/* Post tx data */
			xdc->ctrl_xfer = 1;
			ep = &xdc->ep[0];

			p1 = xdc_align_ctrlpkt(xdc, ep, p1, USB_3_MAX_CTRL_PACKET);

			buf = PKTDATA(xdc->uxdc->osh, p1);

			wr = ep->urb_wr;
			xdc->ep0_urb[wr].buf = p1; /* point back to lbuf struct */
			xdc->ep0_urb[wr].trb_num = 0;
			xdc->ep0_urb[wr].lbuf = 1;
			xdc->ep0_urb[wr].actual = 0;
			xdc->ep0_urb[wr].trbcc = 0;
			xdc->ep0_urb[wr].trb = NULL;
			xdc->ep0_urb[wr].length = PKTLEN(xdc->osh, p1);
			xdc_dbg(("xdc setup return buffer from cdc = %d\n", err));
		} else {
			xdc_dbg(("control out cdc no buffer\n"));
			/* Control out and High level provide buf */
			xdc_recv_ctrldata(xdc, ctrl, p1);
		}
	} else if (p1 == p) {
		if (err == UT_READ) {
			xdc_dbg(("deferred read \n"));
			xdc->ctrl_xfer = 0x01|err;
			err = 0;
		} else if (xdc->ep0_in == 0) {
			xdc_dbg(("RX no buffer \n"));
			/* Control out and High level provide buf */
			xdc_recv_ctrldata(xdc, ctrl, NULL);
		} else {
			xdc_err(("TX, no buf\n"));
		}
	} else if (!err) {
		if (xdc->ep0_in == 0) {
			/* Control out and High level not provide buf */
			xdc_recv_ctrldata(xdc, ctrl, NULL);
		} else {
			xdc_dbg(("control in no cdc buffer\n"));
		}
		/* OR_REG(ch->osh, &ch->regs->epstatus, EPS_DONE(ep)); */
	}

	return err;
} /* xdc_sendup_request */

/** Received UR_SET_SEL USB request from host on the control endpoint */
static int xdc_set_sel(struct xdcd *xdc, usb_device_request_t *ctrl)
{
	uint32 len, wr;
	struct xdc_ep *ep;

	xdc_trace(("ep0 set_sel\n"));

	ep = &xdc->ep[0];

	len = (uint32) htol16(ctrl->wLength);

	wr = ep->urb_wr;

	xdc->ep0_urb[wr].length = len;
	xdc->ep0_urb[wr].buf = xdc->setup_buf;
	xdc->ep0_urb[wr].lbuf = 0;
	xdc->ep0_urb[wr].trb_num = 0;
	xdc->ep0_urb[wr].actual = 0;
	xdc->ep0_urb[wr].trbcc = 0;
	xdc->ep0_urb[wr].trb = NULL;
	xdc->ctrl_xfer = 1;

	return 0;
}

/** Received UR_SET_CONFIG USB request from host on the control endpoint */
static int xdc_set_config(struct xdcd *xdc, usb_device_request_t *ctrl)
{
	uint32 cfg;
	uint32 *dev_state;
	int ret, nak = 0;

	dev_state = &xdc->dev_state;

	cfg = ltoh16(ctrl->wValue);
	xdc_dbg(("xdc_set_config xdc->dev_state=%d\n", xdc->dev_state));

	if ((*dev_state) == XDC_STATE_ADDRESS) {
		ret = usbdev_setcfg(xdc->uxdc->bus, cfg);
		xdc_dbg(("ret=%d cfg=%d\n", ret, cfg));
		if (cfg == ret) {
			*dev_state = XDC_STATE_CONFIGURED;
			xdc->uxdc->pwmode = 0;
		} else if (cfg) {
			nak = 1;
		}
	} else if ((*dev_state) == XDC_STATE_CONFIGURED) {
		ret = usbdev_getcfg(xdc->uxdc->bus);

		if (ret != cfg) {
			ret = usbdev_setcfg(xdc->uxdc->bus, cfg);
			if ((cfg != ret) && cfg) {
				nak = 1;
			} else {
				xdc->uxdc->pwmode = 0;
			}
		}

		xdc_dbg(("ret=%d recfg=%d\n", ret, cfg));

		if (cfg == 0) {
			*dev_state = XDC_STATE_ADDRESS;
			xdc->uxdc->pwmode = 0;
			/* do we need to deconfig? ..... */
		}
	}

	xdc->ctrl_nak = nak;
	xdc_dbg(("xdc->dev_state =%d \n", xdc->dev_state));

	return 0;
} /* xdc_set_config */

/** USB standard specifies a 'wIndex' field in every setup packet */
static struct xdc_ep *get_ep_from_wIndex(struct xdcd *xdc, uint16 windex)
{
	struct xdc_ep *ep;
	uint32 num;
	uint32 ep_index;

	num = (windex & UE_ADDR);

	xdc_dbg(("get_ep_from_wIndex num=%d  windex=%d", num, windex));

	if (num == 0) {
		ep_index = 0;
	} else {
		ep_index = num * 2 - 1;
		if ((windex & UE_DIR_IN) == UE_DIR_IN) /* direction: dongle->host */
			ep_index++;
	}

	if (ep_index < EP_MAX_IDX)
		ep = &xdc->ep[ep_index];
	else
		ep = NULL;

	return ep;
}

/**
 * By USB standard, every endpoint has a 'halt' bit. When set, endpoint will return a stall pkt.
 * Stall and Halt are of the same semantics on a USB device. The condition is persistent until the
 * EP is reset with Reset Endpoint Command, re-configured, or reception of a SETUP packet (EP0).
 * @param[in/out] ep    endpoint
 * @param[in/out] value 0==resume endpoint, 1==halt endpoint
 */
int xdc_halt_ep(struct xdc_ep *ep, int value)
{
	struct xdcd *xdc;
	int ret;

	xdc = ep->xdc;
	xdc_dbg(("xdc_halt_ep num = %d\n", ep->num));

	if (value) {
		/* Halt */
		if (ep->state&XDC_EP_STALL) {
			return 0;
		}

		if (ep->num == 0)
			xdc->ctrl_pipe_state = CTRL_PIPE_IDLE;

		ret = xdc_cmd(xdc, ep, STALL_EP_TRB);
		if (ret && ret != -4)
			xdc_err(("failed to stall on %d\n", ep->index));
		else
			ep->state |= XDC_EP_STALL;
	} else {
		/* Reset Endpoint command is used by software to recover from a Stall/Halted
		 * condition for an EP, which happens when host issues a request on CLEAR_FEATURE
		 * (Endpoint Halt).
		 */
		if ((ep->state&XDC_EP_STALL) == 0) {
			return 0;
		}
		ret = xdc_cmd(xdc, ep, RESET_EP_TRB);
		if (ret && ret != -4)
			xdc_err(("failed to reset %d\n", ep->index));
		else
			ep->state &= ~XDC_EP_STALL;
	}
	return ret;
} /* xdc_halt_ep */

/** USB request 'UR_GET_STATUS' was received on the control endpoint */
static int xdc_get_status(struct xdcd *xdc, usb_device_request_t *ctrl)
{
	struct xdc_ep *ep;
	uint32 type, wr;
	uint16 status = 0;
	uint16 *rsp;
	uint32 num;

	type = ctrl->bmRequestType & 0x1f;

	if (type > UT_ENDPOINT)
		return -1;

	if (type == UT_DEVICE) {
		status |= xdc->uxdc->self_power<<XDC_SELF_POWERED;
		status |= xdc->uxdc->remote_wakeup<< XDC_REMOTE_WAKEUP;
		status |= xdc->uxdc->pwmode;
	} else if (type == UT_INTERFACE) {
		if (ctrl->wIndex == 0) {
			status = 1  << USB3_IGS_REMOTE_WAKEUP_CAPABLE_LSB;
			status |= xdc->uxdc->remote_wakeup << USB3_IGS_REMOTE_WAKEUP_LSB;
		}
	} else if (type == UT_ENDPOINT) {
		ep = get_ep_from_wIndex(xdc, ltoh16(ctrl->wIndex));
		if (!ep) {
			xdc_err(("not a valid EP?"));
			return -1;
		}
		num = (ltoh16(ctrl->wIndex) & UE_ADDR);
		if (ep->state & XDC_EP_STALL) {
			/* Experimental change */
			if (num != 0)
				status = 1 << UF_ENDPOINT_HALT;
		}
	}

	rsp = (uint16 *) xdc->setup_buf;

	/* prepare xdc_urb for the status transfer */
	ep = &xdc->ep[0];
	wr = ep->urb_wr;

	*rsp = htol16(status);
	xdc->ep0_urb[wr].length = 2;
	xdc->ep0_urb[wr].buf = xdc->setup_buf;
	xdc->ep0_urb[wr].trb_num = 0;
	xdc->ep0_urb[wr].lbuf = 0;
	xdc->ep0_urb[wr].actual = 0;
	xdc->ep0_urb[wr].trbcc = 0;
	xdc->ep0_urb[wr].trb = NULL;
	xdc->ctrl_xfer = 1;

	return 0;
} /* xdc_get_status */

/** received feature number 'UF_TEST_MODE' from host */
int xdc_set_test_mode(struct xdcd *xdc, int mode)
{
	uint32 portpm = 0;
	uint32 value  = 0;

	xdc_dbg(("\n xdc_set_test_mode=%08x", mode));
	portpm = R_REG(xdc->osh, &xdc->opregs->portpw20);
	printf("portpm=%08x\n", portpm);
	portpm &= ~USB_2_PORTPM_TESTMODE_MASK;

	switch (mode) {
	case USB_2_TEST_J:
	case USB_2_TEST_K:
	case USB_2_TEST_PACKET:
	case USB_2_TEST_FORCE_EN:
		portpm |=  mode << USB_2_PORTPM_TESTMODE_SHIFT;
		break;
	case USB_2_TEST_SE0_NAK:

		//offset
		W_REG(xdc->osh, (uint32 *)XDC_INDIRECT_ADDR, XDC_DEBUGCTL0);

		//data
		value = R_REG(xdc->osh, (uint32 *)XDC_INDIRECT_DATA);

		value |= 1<<FORCE_UTMIST;

		value &= UTMIFSMST_MASK;
		value |= UTMIFSMST_15<<UTMIFSMST;

		printf("DEBUGCTRL0 set as %08x \n", value);
		W_REG(xdc->osh, (uint32 *)XDC_INDIRECT_DATA, value);

		//offset
		W_REG(xdc->osh, (uint32 *)XDC_INDIRECT_ADDR, XDC_DEVADDR);

		//data
		value = XDC_DEVVALUE;
		W_REG(xdc->osh, (uint32 *)XDC_INDIRECT_DATA, value);

		portpm |=  mode << USB_2_PORTPM_TESTMODE_SHIFT;
		break;
	default:
		return -1;
	}

	xdc_dbg(("portpm=%x\n", portpm));
	W_REG(xdc->osh, &xdc->opregs->portpw20, portpm);
	return 0;
} /* xdc_set_test_mode */

/** received USB standard defined 'set feature' control packet from host */
static int xdc_set_feature(struct xdcd *xdc,
       usb_device_request_t *ctrl, uint32 set)
{
	struct xdc_ep *ep;
	uint32 type, wValue, wIndex;
	uint32 portpw, portpr, timeout;
	int ret;

	wValue = ltoh16(ctrl->wValue);
	wIndex = ltoh16(ctrl->wIndex);
	type = ctrl->bmRequestType & 0x1f;
	timeout = 60;
	xdc->ctrl_nak = 0;

	xdc_dbg(("xdc_set_feature wValue=%d wIndex=%d type=%d \n",
	          wValue, wIndex, type));

	switch (type) {
	case UT_DEVICE:
	{
		/* This is for EP0 in SS		 */
		switch (wValue) {
		case UF_DEVICE_REMOTE_WAKEUP:
			if (set) {
				xdc_dbg(("set remote wake up\n"));
				xdc->uxdc->remote_wakeup = 1;
			} else {
				xdc_dbg(("clear remote wake up\n"));
				xdc->uxdc->remote_wakeup = 0;
			}
			break;
		case UF_DEVICE_U1_ENABLE:
			xdc_dbg(("set U1 Enable, dev_state = %d, set = %d\n", xdc->dev_state, set));
			if (xdc->dev_state <= XDC_STATE_ADDRESS) {
				xdc->ctrl_nak = 1;
				break;
			}
			portpw = R_REG(xdc->osh, &xdc->opregs->portpw);

			xdc_dbg(("portpw=%x \n", portpw));

			if (xdc->u1u2_enable == 0)
				set = 0;

			portpw &= ~(XDC_PORTPM_U1T);

			if (set) {
				/* Device will never issue U1 Entry */
				portpw |= XDC_PORTPM_U1T;

				xdc_dbg(("portpw=%x \n", portpw));
				portpw |= XDC_PORTPM_U1E;
				portpw |= XDC_PORTPM_W1S;
				xdc->uxdc->pwmode |= (1 << XDC_U1_ENABLED);
				set = XDC_PORTPM_U1E|0xFF;
			} else {
				portpw &= ~(XDC_PORTPM_U1T);
				portpw &= ~XDC_PORTPM_U1E;
				portpw |= XDC_PORTPM_W1S;
				xdc->uxdc->pwmode &= ~(1 << XDC_U1_ENABLED);
			}

			do {
				W_REG(xdc->osh, &xdc->opregs->portpw, portpw);
				OSL_DELAY(2);
				portpr = R_REG(xdc->osh, &xdc->opregs->portpw);
				portpr &= XDC_PORTPM_U1E|XDC_PORTPM_U1T;
				timeout--;
				if (timeout == 0)
					break;
			} while (portpr != set);

			xdc_dbg(("u1 portpm=%x pwmode=%x set = %x\n",
				portpw, xdc->uxdc->pwmode, set));
			break;

		case UF_DEVICE_U2_ENABLE:
			xdc_dbg(("set U2 Enable, dev_state = %d, set = %d\n", xdc->dev_state, set));
			if (xdc->dev_state <= XDC_STATE_ADDRESS) {
				xdc->ctrl_nak = 1;
				break;
			}

			portpw = R_REG(xdc->osh, &xdc->opregs->portpw);

			xdc_dbg((" portpw=%x \n", portpw));

			/* WAR: disable U2 since it will cause link since HW bug */
			if (xdc->u1u2_enable == 0)
				set = 0;

			if (set) {
				portpw |= XDC_PORTPM_U2A;
				portpw |= XDC_PORTPM_U2E;
				xdc->uxdc->pwmode |= (1 << XDC_U2_ENABLED);
				set = XDC_PORTPM_U2E;
			} else {
				portpw &= ~(XDC_PORTPM_U2T);
				portpw &= ~(XDC_PORTPM_U2E|XDC_PORTPM_U2A);
				portpw |= XDC_PORTPM_W2S;
				xdc->uxdc->pwmode &= ~(1 << XDC_U2_ENABLED);
			}

			do {
				W_REG(xdc->osh, &xdc->opregs->portpw, portpw);
				OSL_DELAY(2);
				portpr = R_REG(xdc->osh, &xdc->opregs->portpw);
				portpr &= XDC_PORTPM_U2E;
				timeout--;
				if (timeout == 0)
					break;

			} while (portpr != set);
			xdc_dbg(("u2 portpm=%x pwmode=%x set = %x\n",
				portpw, xdc->uxdc->pwmode, set));
			break;

		case UF_DEVICE_LTM_ENABLE:
			xdc_dbg(("USB device LTM enable \n"));
			break;

		case UF_TEST_MODE:
			xdc_dbg(("\n USB_DEVICE_TEST_MODE"));
			if ((wIndex & 0xff) != 0)
				return -1;
			if (!set)
				return -1;

			xdc_dbg(("\n USB_DEVICE_TEST_MODE=%08x", wIndex));
			xdc->test_mode_nr = wIndex >> UF_TEST_MODE_SHIFT;
			xdc->test_mode = 1;
			break;

		default:
			xdc->ctrl_nak = 1;
			return -1;
		}
		break;
	}

	case UT_INTERFACE:
	{
		if (wValue == UF_INTRF_FUNC_SUSPEND) {
			if (wIndex&UF_INTRF_FUNC_SUSP_LP) {
				/* Enable Low Power Suspend */
			}
			if (wIndex & UF_INTRF_FUNC_SUSP_RW) {
				xdc_dbg_ltssm(("Enabling USB3 remote wakeup\n"));
				xdc->uxdc->remote_wakeup = 1;
			} else {
				xdc_dbg_ltssm(("Disabling USB3 remote wakeup\n"));
				xdc->uxdc->remote_wakeup = 0;
			}
		}
		break;
	}

	case UT_ENDPOINT:
	{
		if (wValue == UF_ENDPOINT_HALT) {
			xdc_dbg(("usb set ep halt set=%d \n", set));
			ep =  get_ep_from_wIndex(xdc, wIndex);
			if (!ep)
				return -1;
			ret = xdc_halt_ep(ep, set);
			if (ret)
				return -1;
		} else {
			return -1;
		}
		break;
	}

	}

	return 0;
} /* xdc_set_feature */

/** A setup packet was received from the host */
static int xdc_std_request(struct xdcd *xdc, usb_device_request_t *ctrl)
{
	int ret;
	uint32 request;

	ret = 0;

	request = ctrl->bRequest;

	if ((ctrl->bmRequestType & (UT_CLASS|UT_VENDOR)) != UT_STANDARD)
	{
		ret = xdc_sendup_request(xdc, ctrl);
		return ret;
	}

	xdc_dbg(("xdc_std_request ctrl->bRequest=%x\n", ctrl->bRequest));
	if (request == UR_GET_STATUS) {
		ret = xdc_get_status(xdc, ctrl);
	} else if (request == UR_CLEAR_FEATURE || request == UR_SET_FEATURE) {
		ret = xdc_set_feature(xdc, ctrl, request>>1);
	} else if (request == UR_SET_ADDRESS) {
		uint32 addr;
		addr = (uint32)ltoh16(ctrl->wValue);
		if (xdc->dev_state == XDC_STATE_DEFAULT ||
			xdc->dev_state == XDC_STATE_ADDRESS) {
			ret = xdc_set_address(xdc, addr);
		}
	} else if (request == UR_SET_CONFIG) {
		ret = xdc_set_config(xdc, ctrl);
	} else if (request == UR_SET_SEL) {
		ret = xdc_set_sel(xdc, ctrl);
	} else if (request == UR_SET_ISODELAY) {
		ret = 0;
	} else {
		ret = xdc_sendup_request(xdc, ctrl);
	}

	return ret;
} /* xdc_std_request */

/*
 * All USB devices respond to requests from the host on the device's Default Control Pipe (EP0).
 * These requests are made using Control Transfers. At the USB packet level, a Control Transfer
 * consists of multiple transactions partitioned into stages: a setup stage, an optional data stage,
 * and a terminating status stage. Under xDCI:
 * - SETUP packet: the setup stage of a control transfer is delivered by xDC via a Transfer Event
 *   with field SPR set to 1B. The packet is carried by fields TPL and TPH (as immediately data).
 * - The data stage of a control transfer, if there is one, is prepared by software in response to
 *   the SETUP, as a TD in EP0's TR, which is headed by a Data Stage TRB, and followed by zero or
 *   more normal TRBs (and Event Data TRBs). The Data Stage TRB indicates the direction of the
 *   transfer.  No more than one Data Stage TD may be defined to go with a Status Stage TD.
 * - A TE for the Status Stage TRB will be generated by xDC. The control transfer is considered
 *   successful if the TE for the Status Stage TD has CC equal to Success. Otherwise, the control
 *   transfer is considered aborted by host.
 */

/** callback, 'setup packet received' event signaled by xDC core */
void xdc_get_setup_pkt(struct xdcd *xdc,
	struct xdc_trb *event)
{
	usb_device_request_t *setup_pkt;
	uint32 len;
	int ret;

	/* save received setup packet */
	setup_pkt = &xdc->setup_pkt;
	memcpy(setup_pkt, &event->buf_addr, sizeof(*setup_pkt));

	len = ltoh16(setup_pkt->wLength);

	if (len == 0) {
		xdc->ep0_in = 0;
		xdc->ctrl_pipe_state = WAIT_STATUS_START;
	} else {
		xdc->ep0_in = ((setup_pkt->bmRequestType&UE_DIR_IN)>>7)&0x01;
		xdc->ctrl_pipe_state = WAIT_DATA_START;
	}

	xdc_dbg(("xdc->ep0_in=%x\n", xdc->ep0_in));
	xdc_dbg(("bmRequestType=%d, bRequest=%d, wValue=%d, wIndex=%d, wLength=%d\n",
		setup_pkt->bmRequestType,
		setup_pkt->bRequest,
		setup_pkt->wValue,
		setup_pkt->wIndex,
		setup_pkt->wLength));
	ret = xdc_std_request(xdc, setup_pkt);

	if (ret) {
		xdc_trace(("Request Data transfer error and give it up\n"));
		if (xdc->ctrl_xfer == 0) {
			xdc->ctrl_nak = 1;
		}
	}
} /* xdc_get_setup_pkt */

/** Called in the context of responding to the host, because the host sent a setup packet */
static void *xdc_align_ctrlpkt(struct xdcd *xdc, struct xdc_ep *ep, void *p, uint32 align)
{
	void *p0, *p1, *p2;
	uchar *dst, *src;
	int wrp, len, tot;

	xdc_dbg(("xdc_align_ctrlpkt p = %p\n", p));
	/* just do this for SS speed */
	if (ep->maxpacket != USB_3_MAX_CTRL_PACKET)
		return p;

	if (p == NULL || (PKTNEXT(xdc->osh, p)) == NULL)
		return p;

	p0 = p;
	p1 = NULL;

	p2 = PKTNEXT(xdc->osh, p0);
	do {
		tot = PKTLEN(xdc->osh, p0);
		xdc_dbg(("p0 len = %d \n", tot));

		if (tot < USB_3_MAX_CTRL_PACKET) {
			/* not handle this case */
			xdc_dbg(("tot = %d < 512 \n", tot));
			PKTSETNEXT(xdc->osh, p0, NULL);
			PKTFREE(xdc->osh, p2, FALSE);
			break;
		}

		if (tot%align) {
			if (p1 == NULL) {
				p1 = PKTGET(xdc->osh, USB_3_MAX_CTRL_PACKET + 16, FALSE);
				if (p1 == NULL) {
					xdc_dbg(("alloc failed for align %p\n", p1));
					PKTSETNEXT(xdc->osh, p0, NULL);
					PKTFREE(xdc->osh, p2, FALSE);
					return p;
				}
				wrp = 0;
			} else {
				xdc_dbg(("wrong !!!!!!!!!!!!!!!!!!!!\n"));
			}

			xdc_dbg(("\np0 = %p; p1 = %p, p2 = %p wrp = %d\n", p0, p1, p2, wrp));

			/* copy front part */
			len = tot - (tot/align)*align;
			src = (uchar *) PKTDATA(xdc->osh, p0) + len;
			dst = (uchar *) PKTDATA(xdc->osh, p1) + wrp;
			bcopy(src, dst, len);

			PKTSETLEN(xdc->osh, p0, tot-len);

			/* copy back part */
			wrp = len;
			len = USB_3_MAX_CTRL_PACKET - wrp;
			src = (uchar *) PKTDATA(xdc->osh, p2);
			tot = PKTLEN(xdc->osh, p2);
			dst = (uchar *) PKTDATA(xdc->osh, p1) + wrp;


			if (tot > len) {
				bcopy(src, dst, len);
				PKTPULL(xdc->osh, p2, len);
				PKTSETLEN(xdc->osh, p2, tot-len);
				PKTSETLEN(xdc->osh, p1, USB_3_MAX_CTRL_PACKET);

				PKTSETNEXT(xdc->osh, p0, p1);
				PKTSETNEXT(xdc->osh, p1, p2);

				p0 = p1;
				p1 = NULL;
				p2 = PKTNEXT(xdc->osh, p0);
			} else {
				/* This should be last packet */
				xdc_dbg(("!p2 copy back part tot = %d < len = %d \n", tot, len));
				len = tot;
				bcopy(src, dst+wrp, len);
				PKTSETNEXT(xdc->osh, p2, NULL);
				PKTFREE(xdc->osh, p2, FALSE);
				p2 = NULL;
				PKTSETLEN(xdc->osh, p1, USB_3_MAX_CTRL_PACKET);
				PKTSETNEXT(xdc->osh, p0, p1);
				PKTSETNEXT(xdc->osh, p1, p2);
				break;
			}
		} else {
			xdc_dbg(("p0 len is aligned \n"));

			p0 = p2;
			p1 = NULL;
			p2 = PKTNEXT(xdc->osh, p0);
		}
	} while (p2);

	xdc_dbg(("check done\n"));

	return p;
} /* xdc_align_ctrlpkt */

/** callback, called on 'data stage' event for a control transfer, formulates answer to host */
void xdc_setup_data(struct xdcd *xdc, struct xdc_trb *event)
{
	struct xdc_slx *slx;
	struct xdc_epctx *ep0_ctx;

	slx = (struct xdc_slx *)(xdc->outctx);
	ep0_ctx = (struct xdc_epctx *)(xdc->outctx) + 1;

	if ((((slx->dev_state)>>27)&0x1f) > XDC_STATE_CONFIGURED) {
		xdc_err(("dev is dead = %x \n", slx->dev_state));
		return;
	}

	if ((ep0_ctx->info1&0x07) == EP_DISABLED) {
		xdc_err(("EP0 state is disabled = %d \n", ep0_ctx->info1));
		return;
	}

	if (xdc->ep[0].state & XDC_EP_STALL)
	{
			xdc_halt_ep(&(xdc->ep[0]), 0);
	}

	if (xdc->ctrl_nak == 1)	{
		xdc_send_NAK(xdc);
		return;
	}

	xdc_trace(("xdc_setup_data xfer = %d......\n", xdc->ctrl_xfer));
	xdc_trace(("xdc->ep0_in=%x\n", xdc->ep0_in));

	if (xdc->ctrl_xfer == 1) {
		uint32 wr;
		struct xdc_urb *urb;

		wr =  xdc->ep[0].urb_wr;
		urb = &xdc->ep0_urb[wr];

		xdc_submit_urb(&xdc->ep[0], urb, DATA_TRB,
			((xdc->ep0_in) ? TRB_DIR_IN:0));

		xdc->ep[0].urb_wr = (wr + 1)%EP0_MAX_URB;

		xdc->ctrl_pipe_state = WAIT_DATA_XMIT;
	} else if (xdc->ctrl_xfer&UT_READ) {
		xdc->ctrl_pipe_state = WAIT_DATA_XMIT;
	} else {
		xdc_dbg(("stall the EP0 \n"));
		xdc_send_NAK(xdc);
		xdc_dbg(("EP0 state is stalled = %d \n", ep0_ctx->info1));
	}
} /* xdc_setup_data */

/** callback, called on 'status stage' event for a control transfer */
void xdc_setup_status(struct xdcd *xdc,
	struct xdc_trb *event)
{
	int ret = 0;
	uint32 trb_dir;
	uint32 len;
	usb_device_request_t *setup_pkt;

	setup_pkt = &xdc->setup_pkt;
	len = ltoh16(setup_pkt->wLength);

	xdc_dbg(("xdc_setup_status ctrl_pipe_state = %d......\n", xdc->ctrl_pipe_state));

	if (xdc->ctrl_nak == 1)	{
		xdc_send_NAK(xdc);
		return;
	}

	/* NAK, no status to send */
	if (xdc->ctrl_pipe_state == CTRL_PIPE_IDLE) {
		return;
	}

	if (xdc->ctrl_pipe_state == WAIT_DATA_XMIT) {
		xdc->ctrl_pipe_state = CTRL_PIPE_PENDING;
		return;
	}

	if (xdc->ep[0].state&XDC_EP_STALL)
		xdc_halt_ep(&(xdc->ep[0]), 0);

	trb_dir = 0;
	if (len == 0) {
		xdc_dbg(("no data stage\n"));
		trb_dir = TRB_DIR_IN;
	}

	xdc->ctrl_pipe_state = WAIT_STATUS_XMIT;

	/* send a status trb */
	xdc->status_urb.buf = NULL;
	xdc->status_urb.length = 0;
	xdc->status_urb.trbcc = ret;
	xdc->status_urb.trb_num = 0;
	xdc->status_urb.lbuf = 0;
	ret = xdc_submit_urb(&xdc->ep[0], &xdc->status_urb, STATUS_TRB, trb_dir);
} /* xdc_setup_status */

#endif /* USB_XDCI */
