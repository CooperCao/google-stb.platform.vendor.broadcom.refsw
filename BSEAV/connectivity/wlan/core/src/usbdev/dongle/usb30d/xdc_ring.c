/*
 * xDC controller driver
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: xdc_ring.c 412056 2013-07-11 18:10:57Z $
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

extern usb_endpoint_descriptor_t xdc_ep0_desc;
extern void rdl_indicate_start(uint32 dlstart);

struct xdc_trb *trb_in_ring(struct xdcd *xdc, struct xdc_trb *trbs, uint32 addr, uint32 ring_sz);
struct xdc_urb *trb_in_ep_urb_list(struct xdc_ep *ep, uint32 trb_addr);
struct xdc_urb *trb_in_urb_array(struct xdc_ep *ep, uint32 trb_addr);
struct xdc_urb *found_urb_via_trb(struct xdc_ring *ring, struct xdc_ep *ep, uint32 trb_addr);

static struct xdc_urb *xdc_unlist_urb(struct xdc_ep *ep, struct xdc_urb *urb);

/** provides dongle->host endpoint with empty buffers */
void xdc_rx_malloc_chk(struct xdcd *xdc, struct xdc_ep *ep)
{
	int ret = 0;

	if (ep == NULL) {
		return;
	}

	if (ep->flow_ctrl) {
		xdc_dbg(("flowctrl\n"));
		return;
	}

	while (ep->malloc_fail) {
		xdc_dbg(("m=%d ", ep->malloc_fail));

		ret = xdc_queue_rxbuf(ep, xdc, NULL);
		if (ret == 0) {
			ep->malloc_fail --;
		} else {
			xdc_dbg(("m=%d \n", ep->malloc_fail));
			break;
		}
	}
}

/** Detects if last entry in ring is about to be filled. If so, updates ring bookkeeping. */
void xdc_enq_is_lktrb(struct xdcd *xdc, struct xdc_ring *ring, uint32 ring_sz)
{
	uint32 link, cycle;

	if (ring->enqueue == &ring->trbs[ring_sz-1]) {
		link = ring->enqueue->flags;
		cycle = link&0x01;
		link &= ~0x01;
		cycle ^= 0x01;
		link |= cycle;

		ring->enqueue->flags =  link;

		xdc_dbg(("ring->enqueue->flags=%x ", ring->enqueue->flags));

		/* Toggle the cycle bit since we just have one ring segment. */
		ring->cycle = (ring->cycle ? 0 : 1);
		ring->enqueue = ring->trbs;

		xdc_dbg(("new ring->cycle=%d ",  ring->cycle));
		xdc_dbg(("new ring->enqueue=%p \n", ring->enqueue));
	}
}

/**
 * The dequeue pointer of a ring points at the next to-be-dequeued item. This pointer must wrap
 * around when the end of the ring is reached.
 */
void xdc_incr_deqptr(struct xdcd *xdc, struct xdc_ring *ring, uint32 ring_sz)
{
	ring->dequeue++;

	if (ring->dequeue == &ring->trbs[ring_sz-1]) {
		ring->dequeue = ring->trbs;
	}
}

/**
 * Notifies the xDCI core that it can start using the ring in host memory associated with the
 * caller supplied endpoint.
 */
void xdc_active_rings(struct xdc_ep *ep)
{
	xdc_dbg(("xdc_active_rings\n"));

	ep->state &= ~(XDC_EP_STALL|XDC_EP_BUSY);

	if (ep->urb_head) {
		/* ring the ep doorbell (DBLDVX) here */
		W_REG(ep->xdc->osh, &ep->xdc->dba->db_dvx, DB_TGT(ep->index, 0));
	}
}

/** Initializes the caller supplied cur_urb struct. The other function parameters are obsolete. */
void xdc_noop_trb(struct xdcd *xdc, struct xdc_ring *xf_ring,
                struct xdc_urb *cur_urb)
{
	struct xdc_trb *cur_trb;

	cur_trb = cur_urb->trb;
	cur_trb->buf_addr = 0;
	cur_trb->buf_addrh = 0;
	cur_trb->status = 0;
	cur_trb->flags &= htol32(TRB_CYCLE);
	cur_trb->flags |= htol32(TRB_TYPE(TR_NOOP_TRB));

}

/**
 * Advances the ring dequeue pointer associated with the caller supplied endpoint.
 * @param ep      [in/out] 
 * @param cur_urb [in]     the item that was just dequeued
 */
void xdc_get_new_deq(struct xdc_ep *ep, struct xdc_urb *cur_urb)
{
	struct xdc_ring *ring;
	struct xdc_trb *trb;
	struct xdc_epctx *ep_ctx;

	ring = ep->xf_ring;

	if (!ring) {
		xdc_err(("ring is empty \n"));
		return;
	}

	ep_ctx = (struct xdc_epctx *)(ep->xdc->outctx) + (ep->index + 1);

	ep->cycle_bit = 0x1 & ltoh32(ep_ctx->deq);
	trb = cur_urb->trb;

	if (trb == &ring->trbs[ep->ring_sz - 1]) {
		trb = ring->trbs;
		ep->cycle_bit ^= 0x1;
	} else {
		trb++;
	}

	ep->dequeue = trb;

	xdc_dbg(("Cycle state = 0x%x\n", ep->cycle_bit));
	xdc_dbg(("New dequeue pointer = %p\n", ep->dequeue));
}



/** Find an URB given a TRB address. Every endpoint structure in host memory has an array of URBs */
struct xdc_urb *trb_in_urb_array(struct xdc_ep *ep, uint32 trb_addr)
{
	struct xdc_urb *urb;
	struct xdc_trb *trb;
	uint32 i, max;

	if (ep->urb == NULL)
		return NULL;

	if (ep->type != UE_BULK) {
		return NULL;
	}

	max = ep->max_urb;

	urb = (struct xdc_urb*)ep->urb;   // points at an array of URBs
	trb = (struct xdc_trb *)trb_addr;

	for (i = 0; i < max; i++) {
		if (urb->trb == trb && urb->buf) {
			return urb;
		}
		urb++;
	}

	return NULL;
}

/**
 * Find an URB given a TRB address. Every endpoint structure in host memory has a linked list of
 * URBs.
 */
struct xdc_urb *trb_in_ep_urb_list(struct xdc_ep *ep, uint32 trb_addr)
{
	struct xdc_urb *urb;
	struct xdc_trb *trb;

	if (ep->urb_head == NULL)
		return NULL;

	trb = (struct xdc_trb *)trb_addr;

	/* make sure it's still queued on this endpoint */
	for (urb = ep->urb_head; urb != NULL; ) {
		if (urb->trb == trb) {
			/* reconstruct ep->urb_head list */
			xdc_unlist_urb(ep, urb);
			urb->next = NULL;
			if (ep->urb_head == NULL) {
				ep->urb_head = ep->urb_tail = urb;
			} else {
				if (urb != ep->urb_head) {
					urb->next = ep->urb_head;
					ep->urb_head = urb;
				}
			}
			return urb;
		}
		urb = urb->next;
	}

	return NULL;
}

/** returns NULL if addr is not in the range of the caller supplied ring */
struct xdc_trb *trb_in_ring(struct xdcd *xdc, struct xdc_trb *trbs, uint32 addr, uint32 ring_sz)
{
	uint32 i, start_addr;

	if (!trbs || !addr)
		return NULL;

	start_addr = (uint32)trbs;

	for (i = 0; i < ring_sz; i++) {
		if (addr == start_addr + i*sizeof(struct xdc_trb))
			return ((struct xdc_trb *)addr);
	}

	return NULL;
}

/** Attempts to find caller supplied trb_addr in a number of locations, returns URB */
struct xdc_urb *found_urb_via_trb(struct xdc_ring *ring, struct xdc_ep *ep, uint32 trb_addr)
{
	struct xdc_urb *urb;
	struct xdc_trb *trb;

	if ((trb = trb_in_ring(ep->xdc, ring->trbs, trb_addr, ep->ring_sz))) {
		urb = trb_in_ep_urb_list(ep, trb_addr);
		if (urb == NULL) {
			xdc_err(("urb not in ep urb list\n"));
			/* find urb in URB buf */
			urb = trb_in_urb_array(ep, trb_addr);
			if (urb) {
				urb->next = NULL;
				if (ep->urb_head) {
					urb->next = ep->urb_head;
					ep->urb_head = urb;
				} else {
					ep->urb_head = urb;
					ep->urb_tail = urb;
				}
				return urb;
			} else {
				xdc_err(("urb not urb array\n"));
				return NULL;
			}
		}
	} else {
		xdc_err(("trb isnot in ring!\n"));
		return NULL;
	}

	return NULL;
}

/** Enables USB 2.0 Link Power Management */
static void enable_lpm_usb20(struct xdcd *xdc)
{
	uint32 temp;
	temp = R_REG(xdc->osh, &xdc->opregs->portpw20);

	xdc_dbg(("enable_lpm_usb20 and remote wakeup temp=%x\n", temp));
	temp |= XDC_PORTPM_HLE | XDC_PORTPM_RWE;

	W_REG(xdc->osh, &xdc->opregs->portpw20, temp);
}

/** React to port 'connect' event signaled by the xDCI core */
int xdc_connect_event(struct xdcd *xdc, int reset)
{
	uint32 speed, temp;
	struct usbdev_chip *uxdc;
	uint16 xdc_speed[5] = {0, 64, 8, 64, 512};
	int ret;
#ifdef XDC_DEBUG
	struct xdc_slx *slx;

	slx = (struct xdc_slx *)(xdc->outctx);
#endif
	temp = R_REG(xdc->osh, &xdc->opregs->portsc);
	speed = (temp&PORT_SPEED_MASK)>>10;

	if (speed == 0 || speed > XDC_PORT_SS) {
		xdc_err(("port speed is wrong = %d \n", speed));
		ASSERT(0);
		return -1;
	}

	xdc->ctrl_pipe_state = CTRL_PIPE_IDLE;

	xdc_ep0_desc.wMaxPacketSize = htol16(xdc_speed[speed]);
	xdc->ep[0].maxpacket = xdc_speed[speed];

	uxdc = xdc->uxdc;

	if (xdc->speed != speed) {
		struct xdc_ep *ep;

		xdc->speed = speed;
		ep = &xdc->ep[0];

		xdc_disable_ep(ep);
		/* enable ep0 */
		ret = xdc_enable_ep(xdc, &xdc_ep0_desc, NULL);

		if (ret < 0) {
			xdc_err(("change ep0 speed failed \n"));
		}

	}

	if (reset == 0) {
		if (xdc->speed == XDC_PORT_SS) {
			if (uxdc->disconnect == 0) {
				usbdev_attach_xdci(uxdc->bus);
			}
		}
	}

	usbdev_speed(xdc->uxdc->bus,
	             ((xdc->speed >= XDC_PORT_HS) ?
	             (xdc->speed - 2) : 0));

	/* set up ep0 */
	xdc_dbg(("connected ..output-slx->dev_state = %x \n", slx->dev_state));
	xdc_dbg(("connected ..output-slx->info = %x \n", slx->info));
	ret = xdc_set_address(xdc, 0);

	if (xdc->speed != XDC_PORT_LS && xdc->speed != XDC_PORT_SS) {
		enable_lpm_usb20(xdc);
	}

	return ret;
}

/** React to port 'disconnect' event signaled by the xDCI core */
uint32 xdc_disconnect_event(struct xdcd *xdc)
{
	struct usbdev_chip *uxdc;

	uxdc = xdc->uxdc;
	uxdc->disconnect += 1;


	if (xdc->dev_state == XDC_STATE_CONFIGURED) {
		uxdc->disconnect = 0;

		/* reset all bus->ep_xx[n], let setcfg to go throught it */
		usbdev_reset(uxdc->bus);

		if (xdc->speed != XDC_PORT_SS) {
			xdc_stop_eps(xdc);
		} else {
			/* current speed is SS, switch to HS */
			if (uxdc->speed != XDC_PORT_SS) {
				/* Reset endpoints */
				printf("stop eps\n");
				xdc_stop_eps(xdc);

				printf("usbdev_detach_xdci\n");
				/* Free device state */
				usbdev_detach_xdci(uxdc->bus);
			}
		}

		if (uxdc->speed) {
			printf("soft connect\n");

			uxdc->reconnect = 0;
			uxdc->speed = 0;
			xdc->speed = 0;
			xdci_soft_connect(xdc);
		}
	}

	return 1;
}

/**
 * There are transfer, command and port events signaled by the controller. Firmware has received an
 * interrupt because a PORT_EVT_TRB has completed.
 */
int xdc_port_event(struct xdcd *xdc, struct xdc_trb *event)
{
	uint32 portsc, temp;
	uint32 cc;
	int ret = 0;

	xdc_trace(("xdc_port_event\n"));

	cc = GET_CC_CODE(ltoh32(event->status));

	if (cc == CC_SUCCESS) {
#ifdef XDC_DEBUG
	xdc_dump_port(xdc);
#endif /* XDC_DEBUG */

		portsc = R_REG(xdc->osh, &xdc->opregs->portsc);

		xdc_err((".....port event portsc  = %x\n", portsc));
		OSL_DELAY(200);

#ifdef XDCI_MONITOR_LTSSM_WAR
		if (portsc & XDC_PORTSC_SR) {
			xdc_dbg_ltssm(("Port status - start of reset set.\n"));
			if (xdci_get_otp_var_u1u2(xdc) & XDCI_OTP_VAR_ENABLE_LTSSM_SR) {
				xdci_monitor_ltssm(xdc);
			}
		}
#endif

		if (portsc&XDC_PORTSC_CSC) {
			if ((portsc&XDC_PORTSC_CCS)) {
				/* Port Status Changed and Current Connect Status = 1 */

				temp = (portsc & XDC_PORTSC_PLS) >> 5;
				if ((temp == XDC_LINK_STATE_U0) && (xdc->uxdc->suspended))
					xdc->uxdc->suspended = 0;

				ret = xdc_connect_event(xdc, 0);
				printf("Connected speed = %d\n", xdc->speed);
#ifdef XDCI_MONITOR_LTSSM_WAR
				xdci_ltssm_reset_histogram(xdc);
#endif

			} else {
				/* Port Status Changed and Current Connect Status = 0 */
				printf("Disconnected \n");
				if (xdc->dev_state > XDC_STATE_CONNETED) {
					xdc_disconnect_event(xdc);
					xdc->dev_state = XDC_STATE_CONNETED;
				}
			}
		} else if ((portsc&XDC_PORTSC_PRC) && (portsc&XDC_PORTSC_CCS)) {
			printf("Reset devstate = %d\n", xdc->dev_state);
			portsc &= ~(XDC_PORTSC_PRC);
			temp = portsc|XDC_PORTSC_PRC;
			W_REG(xdc->osh, &xdc->opregs->portsc, temp);
				xdc->dev_state = XDC_STATE_CONNETED;
				ret = xdc_connect_event(xdc, 1);
				printf("Reset Done speed= %d, dev_state = %d\n",
					xdc->speed, xdc->dev_state);
		} else if ((portsc & XDC_PORTSC_CCS) && (portsc&XDC_PORTSC_PLC)) {
			temp = (portsc & XDC_PORTSC_PLS) >> 5;
			W_REG(xdc->osh, &xdc->opregs->portsc, portsc);
			printf("Link Statet changed...... = %d \n", temp);
			if (temp == XDC_LINK_STATE_U3) {
#ifdef XDCI_MONITOR_LTSSM_WAR
				xdci_ltssm_dump_histogram(xdc);
				xdci_ltssm_reset_histogram(xdc);
#endif
				/* set suspend flag */
				xdc->uxdc->suspended = 1;

				usbdev_suspend(xdc->uxdc->bus);
			} else
			if ((temp == XDC_LINK_STATE_U0) && (xdc->uxdc->suspended)) {
				printf("(temp == XDC_LINK_STATE_U0) && (xdc->uxdc->suspended)\n");
#ifdef XDCI_MONITOR_LTSSM_WAR
				int call_remote2 = xdci_get_otp_var_u1u2(xdc) &
					XDCI_OTP_VAR_ENABLE_LTSSM_U3_REMOTE;
#endif
				if (xdc->uxdc->remote_wakeup_issued) {
					xdc->uxdc->remote_wakeup_issued = 0;
					if ((xdc->speed == USB_SPEED_SUPER) /* && call_remote2 */ ) {
#if XDCI_MONITOR_LTSSM_WAR
						if (call_remote2)
							xdci_u3exit_remote2(xdc);
#endif
						xdc_function_wake(xdc);
					}
				}
				xdc->uxdc->suspended = 0;

				usbdev_resume(xdc->uxdc->bus);
			}
#if XDCI_MONITOR_LTSSM_WAR
			if (xdci_get_otp_var_u1u2(xdc) & XDCI_OTP_VAR_ENABLE_LTSSM_U3) {
				if (temp == 15) {
					xdci_monitor_ltss_on_u3exit(xdc);
				}
			}
#endif /* XDCI_MONITOR_LTSSM_WAR */
		}

		xdc_dbg(("Clear CSC\n"));
		temp = portsc;
		portsc &= ~(XDC_PORTSC_RWC);

		if (temp&XDC_PORTSC_CSC)
			portsc |= XDC_PORTSC_CSC;

		if (temp&XDC_PORTSC_PRC)
			portsc |= XDC_PORTSC_PRC;

		if (temp&XDC_PORTSC_PPC)
			portsc |= XDC_PORTSC_PPC;

		W_REG(xdc->osh, &xdc->opregs->portsc, portsc);

		return ret;
	}
	return -1;
}

/** callback for (xfer) event on control endpoint */
int xdc_ctrl_event(struct xdcd *xdc, struct xdc_urb *urb,
                           struct xdc_trb *tx_trb,
                           struct xdc_trb *event,
                           uint32 ep_index)
{
	struct xdc_ring *ring;
	struct xdc_ep *ep;
	int32 trb_cc;
	int32 ret;

	xdc_trace(("xdc_ctrl_event\n"));
	ep = &xdc->ep[ep_index];
	ring = ep->xf_ring;

	trb_cc = (int32)urb->trbcc;

	xdc_dbg(("ep_index=%d xf_ring=%p trb_cc=%d\n",
	           ep_index, ring, trb_cc));

	if (trb_cc == CC_SUCCESS || trb_cc == CC_SHORT_TX) {
		urb->actual = urb->length - TRB_LEN(ltoh32(event->status));

		if (xdc->ctrl_pipe_state == WAIT_DATA_XMIT) {
			xdc->ctrl_pipe_state = WAIT_STATUS_START;
		} else if (xdc->ctrl_pipe_state == CTRL_PIPE_PENDING) {
			xdc_setup_status(xdc, NULL);
		} else if (xdc->ctrl_pipe_state == WAIT_STATUS_XMIT) {
			xdc_dbg(("xdc->test_mode:%d xdc->test_mode_nr:%d\n",
				xdc->test_mode, xdc->test_mode_nr));
			if (xdc->test_mode) {
				ret = xdc_set_test_mode(xdc, xdc->test_mode_nr);
				if (ret < 0) {
					xdc_dbg(("Invalid test #%d\n", xdc->test_mode_nr));
					/* TBD?? */
					return -1;
				}
			}

			xdc->ctrl_pipe_state = CTRL_PIPE_IDLE;
		}

		do {
			xdc_incr_deqptr(xdc, ring, ep->ring_sz);
			urb->trb_num -= 1;
		} while (urb->trb_num);

		xdc_dbg(("urb->actual=%d urb->length=%d \n", urb->actual, urb->length));
		if (urb->actual > urb->length) {
			xdc_err(("too much data, urblen=%u, actual=%u\n",
				urb->length, urb->actual));
			urb->actual = 0;
		}
		return 0;
	}

	return -1;
}

/** callback for (xfer) event on bulk endpoint */
int xdc_bulk_event(struct xdcd *xdc, struct xdc_urb *urb,
                                struct xdc_trb *tx_trb,
                                struct xdc_trb *event,
                                uint32 ep_index)
{
	uint32 len;
	struct xdc_ep *ep;

	ep = &xdc->ep[ep_index];

	if (ep->dir == UE_DIR_OUT) { /* direction: host->dongle */
		len = TRB_LEN(ltoh32(event->status));
		if (len < urb->length) {
			urb->actual = urb->length - len;
		} else {
			urb->actual = 0;
		}
		xdc_incr_deqptr(xdc, ep->xf_ring, ep->ring_sz);
	} else {
		do {
			xdc_incr_deqptr(xdc, ep->xf_ring, ep->ring_sz);
			urb->trb_num -= 1;
		} while (urb->trb_num);
	}

	return 0;
}

/**
 * Callback for (xfer) event, called if data has been transfered to/from the xDCI. Specifically:
 * - If the Interrupt on Completion (IOC) bit is set in the transfer TRB.
 * - When a short transfer occurs while processing a transfer TRB and the Interrupt on Short Packet
 *   (ISP) flag is set.
 * - If an error occurs while processing a transfer TRB.
 *
 * Determines endpoint and calls respective handler.
 */
int xdc_xfer_event(struct xdcd *xdc, struct xdc_trb *event)
{
	struct xdc_ep *ep;
	struct xdc_ring *ring;
	int ep_index;
	uint32 txtrb_addr;
	struct xdc_trb *tx_trb;
	struct xdc_epctx *ep_ctx;
	struct xdc_urb *urb = NULL;
	uint32 trb_cc;
	int ret = 0;

	tx_trb = NULL;

	ep_index = EP_IDX_OF_TRB(ltoh32(event->flags)) - 1;
	if (ep_index < 0 || ep_index >= EP_MAX_IDX) {
		xdc_err(("xfer event, ep_index is invalid = %d \n", ep_index));
		return -1;
	}

	ep = &xdc->ep[ep_index];
	ring = ep->xf_ring;
	ep_ctx = (struct xdc_epctx *)(xdc->outctx) + (ep_index + 1);

	xdc_dbg(("xdc_xfer_event ep_index=%d\n", ep_index));
	xdc_dbg(("xf_ring=%p ep_ctx->info1=%x\n", ring, ep_ctx->info1));

	if (!ring ||
		(ltoh32(ep_ctx->info1) & 0x0F) == EP_DISABLED) {
		xdc_dbg(("ERR: Xfer evt on disabled ep = %d\n", ep->num));
		return -1;
	}

	txtrb_addr = ltoh32(event->buf_addr);
	trb_cc = GET_CC_CODE(ltoh32(event->status));

	xdc_dbg(("trb_cc =%d ep_index = %d\n", trb_cc, ep->index));
	xdc_dbg(("ep->urb_head =%p\n", ep->urb_head));
	xdc_dbg(("event = %p, trb_addr = %x\n", event, txtrb_addr));

	if (trb_cc == CC_BABBLE) {
		xdc_err(("BABBLE error \n"));
		return 0;
	}

	if (trb_cc == CC_SUCCESS || trb_cc == CC_SHORT_TX) {
		if (txtrb_addr == 0) {
			xdc_err(("err: txtrb_addr = 0\n"));
			return -1;
		}

		if (ep->urb_head) {
			urb = ep->urb_head;
			if (ring->dequeue != urb->trb) {
				xdc_err(("TRB isnot urb_head, ep = %d \n", ep->num));
				urb = found_urb_via_trb(ring, ep, txtrb_addr);
				/* update ring->dequeue pointer? */
				if (urb == NULL)
					return -2;

				ring->dequeue = urb->trb;
			}
		} else {
			xdc_err(("Err: urb head empty! ep = %d\n", ep->num));
			xdc_dbg(("event->buf_addr = %x, flags = %x \n",
				txtrb_addr, event->flags));
			urb = found_urb_via_trb(ring, ep, txtrb_addr);

			if (urb == NULL)
				return -3;

			if (ring->dequeue != urb->trb) {
				ring->dequeue = urb->trb;
			}
		}

		tx_trb = (struct xdc_trb *)txtrb_addr;
		urb->trbcc = trb_cc;
		xdc_dbg(("event_trb=%p trb_addr = %x\n", tx_trb, txtrb_addr));

		/** calls back xdc_bulk_event, xdc_ctrl_event */
		ret = ep->evt_fn(xdc, urb, tx_trb, event, ep_index);

		if (ret == 0) {
			ep->done_fn(xdc, ep_index, urb);
			return 0;
		}
	} else if (trb_cc == CC_DB_ERR) {
		if (ep->malloc_fail) {
			trb_cc = xdc_queue_rxbuf(ep, xdc, NULL);
			if (trb_cc == 0) {
				ep->malloc_fail --;
			}
		}
		return 0;
	} else if (trb_cc >= CC_SETUP_RECV && trb_cc <= CC_STATUS_START) {
		xdc_dbg(("Setup on EP0\n"));
		xdc->setup_fn[trb_cc - CC_SETUP_RECV](xdc, event);
		return 0;
	}
	else {
		if (trb_cc == CC_STOP_INVAL || trb_cc == CC_STOP) {
			xdc_err(("err: trb_cc = %d ep->num = %d \n", trb_cc, ep->num));
			return -trb_cc;
		} else if (trb_cc == CC_STALL) {
			xdc_err(("stalled ep\n"));
			if (urb) {
				ep->state |= XDC_EP_STALL;
				ep->deq_urb = urb;

				xdc_get_new_deq(ep, urb);
				ep->deq_urb = NULL;

				xdc_cmd(ep->xdc, ep, RESET_EP_TRB);
			}
			return -trb_cc;
		} else {
			xdc_err(("CC=%d, ep= %d \n", trb_cc, ep->num));

			if (trb_cc == CC_OVER_RUN || trb_cc == CC_UNDER_RUN) {
				return 0;
			}
			if (ep->type == UE_BULK || trb_cc == CC_MISSED_INT) {
				/* something is wrong, skip this transfer */
				/* TBD clean memory */
				xdc_incr_deqptr(xdc, ep->xf_ring, ep->ring_sz);
			}
		}
	}
	xdc_dbg(("ep->enqueue=%p \n", ring->enqueue));
	xdc_trace((" return xdc_xfer_event\n"));

	return 0;
} /* xdc_xfer_event */

/** Checks e.g. if there is room for a command on the command ring */
uint32 xdc_chk_ring_space(struct xdcd *xdc, struct xdc_ring *ring,
                 uint32 ep_state, uint32 num_trbs, uint32 ring_sz)
{
	int space;
	uint32 enq_addr, deq_addr;

	xdc_dbg(("chk_ring_space ep_state=%d num_trbs=%d ring=%p \n",
	          ep_state, num_trbs, ring));

	if ((ep_state == EP_DISABLED) || (ep_state == EP_HALTED)) {
		xdc_err(("endpoint is disable\n"));
		/* Do reset or clear HALT */
		return 0;
	}
	if (ring->enqueue == &ring->trbs[ring_sz-1]) {
		enq_addr = (uint32)ring->trbs;
	} else {
		enq_addr = (uint32)ring->enqueue;
	}

	deq_addr = (uint32)ring->dequeue;

	if (enq_addr >= deq_addr) {
		/* sizeof(struct xdc_trb) is 16 byte */
		space = ring_sz - ((enq_addr - deq_addr)>>4);
	} else {
		space = (deq_addr - enq_addr)>>4;
	}

	/* make sure space > = 0 */
	if (space <= 1) {
		space = 0;
	} else {
		space -= 2;
	}

	if (space < num_trbs) {
		xdc_err(("err no room on ring\n"));
	}

	return space;
} /* xdc_chk_ring_space */

#ifndef FLIP_1ST_TRB_CYCLE
#define FLIP_1ST_TRB_CYCLE
#endif

/** submit tx URB (USB Request Block) on endpoints transmit ring, or empty rx URB on receive ring */
int xdc_submit_urb(struct xdc_ep *ep, struct xdc_urb *urb, uint32 trb_type, uint32 dir)
{
	struct xdc_ring *xf_ring;
	void *p;
	uint32 cycle, fst_cycle;
	struct xdc_trb *cur_trb;
	struct xdcd *xdc;
	uint32 len, trb_num;
	uint32 empty_trb;
	int ret = 0;

	xdc = ep->xdc;
	p = NULL;

	xdc_dbg(("submit_urb xdc=%p urb=%p ep_index=%d\n", xdc, urb, ep->index));
	xdc_dbg(("ep=%p ep->xf_ring=%p ", ep, ep->xf_ring));
	xdc_dbg(("urb->buf  =%p \n", urb->buf));

	xf_ring = ep->xf_ring;

	if (urb->lbuf) {
		p = urb->buf;
		trb_num = pktsegcnt(xdc->osh, p);
#ifdef XDC_DEBUG
		if (urb->length != PKTLEN(xdc->osh, p)) {
			xdc_err(("p length is not set\n"));
		}
#endif
	} else {
		trb_num = 1;
	}

	urb->trb_num = trb_num;
	empty_trb = 0;

#ifdef TX_EMPTY_TRB
	if (ep->dir && (ep->index != 0))
	{ /* Send Data to check MaxPacketSize boundry */
		if (urb->lbuf) {
			p = pktlast(xdc->osh, p);
			len = PKTLEN(xdc->osh, p);
			p = urb->buf;
		} else {
			len = urb->length;
		}
		if ((len%ltoh16(ep->maxpacket)) == 0) {
			/* for maxpacket 512/1024.., need to send a 0 packet */
			urb->trb_num += 1;
			empty_trb = 1;
			xdc_dbg(("add a 0 packet for tx \n"));
		}
	}
#endif /* TX_EMPTY_TRB */

	/* if enq is a link trb, it will swap the cycle bit here */
	xdc_enq_is_lktrb(xdc, xf_ring, ep->ring_sz);

	urb->trb = xf_ring->enqueue;
	fst_cycle = xf_ring->cycle;

	do {
		cur_trb = xf_ring->enqueue;
		cycle = xf_ring->cycle;

		if (urb->lbuf) {
			len = PKTLEN(xdc->osh, p);
			cur_trb->buf_addr = htol32((uint32)(PKTDATA(xdc->osh, p)));
		} else {
			len = urb->length;
			cur_trb->buf_addr = htol32((uint32) urb->buf);
		}

		cur_trb->buf_addrh = htol32(0);
		cur_trb->status = htol32(TRB_LEN(len)|TRB_TDS(trb_num-1));

		xdc_dbg(("cur_trb=%p   cycle=%d \n", cur_trb, cycle));
		xdc_dbg(("urb->length=%d ep->wMaxPacketSize=%d \n",
			len, ep->maxpacket));

#ifdef FLIP_1ST_TRB_CYCLE
		if (cur_trb == urb->trb) {
			/* First TRB need to reverse the cycle bit until all TRBs are prepared */
			cur_trb->flags =
				htol32(TRB_TYPE(trb_type)|(cycle^0x01)|dir);
		} else
#endif
		{
			if ((trb_num == 1) && (empty_trb == 0)) {
			/* last trb in TD, set TRB_IOC|TRB_ISP, if zero_trb, */
			/* then zero_trb is last one */
				cur_trb->flags =
					htol32(TRB_TYPE(trb_type)|cycle|TRB_IOC|TRB_ISP|dir);
			} else {
				/* not last trb, need to set TRB_CHAIN */
				cur_trb->flags =
					htol32(TRB_TYPE(trb_type) | cycle | TRB_CHAIN | dir);
			}
		}

#ifdef TX_EMPTY_TRB
		if (empty_trb) {
			xf_ring->enqueue++;

			xdc_enq_is_lktrb(xdc, xf_ring, ep->ring_sz);

			urb->zer_trb = 1;
			cycle = xf_ring->cycle;

			cur_trb = xf_ring->enqueue;
			cur_trb->flags = htol32(TRB_TYPE(trb_type)|cycle|TRB_IOC|TRB_ISP);

			cur_trb->buf_addr = htol32((uint32) urb->buf));
			cur_trb->buf_addrh = htol32(0);
			cur_trb->status = htol32(0);
		}
#endif /* TX_EMPTY_TRB */
		xf_ring->enqueue++;

		trb_num --;
		if (trb_num == 0) {
			break;
		}

#ifdef BCMUSBDEV_COMPOSITE
		if (trb_type == ISOC_TRB) {
			/* for ISOC xfer, 1st trb is ISOC, other is NORMAL */
			trb_type = NORMAL_TRB;
		}
#endif /* BCMUSBDEV_COMPOSITE */

		/* Go to handle next LBUF */
		if (urb->lbuf) {
			p = PKTNEXT(xdc->osh, p);
			if (p == NULL) {
				/* double check lbuf is empty or not */
				if (trb_num) {
					/* error */
					xdc_dbg(("wrong, please check!!\n"));
				}
				break;
			}
		}
		xdc_enq_is_lktrb(xdc, xf_ring, ep->ring_sz);
	} while (trb_num);

	urb->next = NULL;
	if (ep->urb_head == NULL) {
		ep->urb_head = ep->urb_tail = urb;
	} else {
		if (urb != ep->urb_tail) {
			ep->urb_tail->next = urb;
			ep->urb_tail = urb;
		} else {
			xdc_err(("double urb\n"));
		}
	}

	xdc_dbg(("ep->urb_head = %p, ep->urb_tail = %p \n", ep->urb_head, ep->urb_tail));
	xdc_dbg(("urb->next = %p \n", urb->next));

#ifdef FLIP_1ST_TRB_CYCLE
	if (cur_trb != urb->trb) {
		/* we have multiple TRBs set CHAIN bit */
		urb->trb->flags |= TRB_CHAIN;
	} else {
		/* we just have ont TRB */
		urb->trb->flags |= TRB_IOC|TRB_ISP;
	}

	/* revise the first TRB cycle bit */
	if (fst_cycle)
		urb->trb->flags |= htol32(fst_cycle);
	else
		urb->trb->flags &= htol32(~TRB_CYCLE);
#endif /* FLIP_1ST_TRB_CYCLE */


	/* ring doorbell */
	W_REG(ep->xdc->osh, &ep->xdc->dba->db_dvx, DB_TGT(ep->index, 0));

	return ret;
} /* xdc_submit_urb */

/**
 * Removes caller supplied URB from a linked list of URBs that every endpoint has. URB is not
 * necessarily at head nor tail of the linked list.
 */
static struct xdc_urb *xdc_unlist_urb(struct xdc_ep *ep, struct xdc_urb *urb)
{
	struct xdc_urb *temp;

	if (urb == ep->urb_head) {
		ep->urb_head = urb->next;
	} else {
		temp = ep->urb_head;
		while (temp) {
			if (temp->next == urb || temp->next == NULL)
				break;
			temp = temp->next;
		};
		if (temp) {
			/* Found it */
			temp->next = urb->next;
		}
	}

	if (ep->urb_head == NULL) {
		ep->urb_tail = NULL;
	}

	urb->next = NULL;
	xdc_dbg(("unlist ep->urb_head = %p, ep->urb_tail = %p \n", ep->urb_head, ep->urb_tail));

	return urb;
}

#ifdef LOOP_BACK
int send_pkt = 0;
int rcv_pkt = 0;
#endif /* LOOP_BACK */

/** isochronous endpoint specific callback function, called when xfer was succesfully completed */
void xdc_urb_isocin_done(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *urb)
{
	xdc_urb_bulkin_done(xdc, ep_index, urb); /* direction: dongle->host */
}

/** isochronous endpoint specific callback function, called when xfer was succesfully completed */
void xdc_urb_isocout_done(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *urb)
{
	xdc_urb_bulkout_done(xdc, ep_index, urb); /* direction: dongle<-host */
}

/** direction: dongle->host, called when xfer was succesfully completed */
void xdc_urb_bulkin_done(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *urb)
{
	struct xdc_ep *ep;
	void *p;
	int ret = 0;

	ep = &xdc->ep[ep_index];
	xdc_dbg(("xdc_urb_bulkin_done ep_index=%d urb=%p \n", ep->index, urb));

	ep->urb_head = urb->next;

	if (ep->urb_head == NULL) {
		ep->urb_tail = NULL;
	}

	p = (void *)urb->buf;

	urb->trbcc = 0;
	urb->buf = NULL;
	urb->trb = NULL;

#ifdef LOOP_BACK
	send_pkt++;
	printf("FREE pkt %d\n", send_pkt);
	ret = 0;
#endif /* LOOP_BACK */

	if (p) {
		PKTFREE(xdc->osh, p, FALSE);
	}

#ifndef LOOP_BACK
	/* check the tx flow control */
	if (ep->flow_ctrl) {
		ret = xdc_chk_ring_space(xdc, ep->xf_ring, EP_RUNNING, 1, ep->ring_sz);
		if (ret >= (ep->ring_sz - (ep->ring_sz/4))) {
			ep->flow_ctrl = FALSE;
			usbdev_txstart(xdc->uxdc->bus, ep->index);
		}
	}

	/* we have some TX pending */
	p = ep->lb_head;
	ret = xdc_send_lbuf(ep, p);

	/* double check USB RX queue to make sure it is not empty */
	if (ep->type == UE_BULK)
		xdc_rx_malloc_chk(xdc, NULL);
#endif /* LOOP_BACK */
} /* xdc_urb_bulkin_done */

/** direction: dongle<-host, called when xfer was succesfully completed */
void xdc_urb_bulkout_done(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *urb)
{
	struct xdc_ep *ep;
	void *p;
	int trbcc;

	ep = &xdc->ep[ep_index];
	xdc_dbg(("xdc_urb_bulkout_done ep=%p urb=%p \n", ep, urb));

	ep->urb_head = urb->next;

	if (ep->urb_head == NULL) {
		ep->urb_tail = NULL;
	}

	p = (void *)urb->buf;
	trbcc = urb->trbcc;
	urb->trbcc = 0;
	urb->buf = NULL;

	/* urb->actual is the received data length */
	xdc_trace(("bulk out urbrequest done, send it up\n"));
	if (p == NULL) {
		xdc_err(("bout p NULL \n"));
		return;
	}

	PKTSETLEN(xdc->osh, p, urb->actual);
	PKTSETNEXT(xdc->osh, p, NULL);

#ifndef LOOP_BACK
	if (urb->actual) {
		if (ep->lb_tail) {
			PKTSETNEXT(xdc->osh, ep->lb_tail, p);
			ep->lb_tail = p;
			xdc_trace(("set lb_tail link \n"));
		} else {
			ep->lb_head = p;
			ep->lb_tail = p;
		}
		p = NULL;
	}


	if (trbcc == CC_SHORT_TX || (urb->actual == 0) ||
		urb->actual < ep->blk_sz) {
		xdc_rx(xdc->uxdc, ep->index, ep->lb_head);

		ep->lb_head = NULL;
		ep->lb_tail = NULL;
	}
#else

	rcv_pkt++;
	printf("RCV pkt %d\n", rcv_pkt);
	xdc_tx_urb(xdc, &xdc->ep[4], p);
	p = NULL;
	ep->lb_head = NULL;
	ep->lb_tail = NULL;
#endif /* LOOP_BACK */

	if (ep->flow_ctrl == FALSE) {
		/* fill 1 more TDs to the ring, if 0  packet, will reuse p */
		trbcc = xdc_queue_rxbuf(ep, xdc, p);
		if (trbcc == 0)
			return;
	} else {
		if (p) {
			PKTFREE(xdc->osh, p, FALSE);
		}
	}

	ep->malloc_fail++;
	if (ep->malloc_fail >= ep->max_rxrsv_trb)
		ep->malloc_fail = ep->max_rxrsv_trb;
} /* xdc_urb_bulkout_done */

/** control endpoint specific callback function, called when xfer was succesfully completed */
void xdc_urb_ctrl_done(struct xdcd *xdc, uint32 ep_index, struct xdc_urb *urb)
{
	void *p;
	struct xdc_ep *ep;
	ep = &xdc->ep[ep_index];

	xdc_dbg(("xdc_urb_ctrl_done ep=%p urb=%p \n", ep, urb));
	ASSERT(urb);

	urb = xdc_unlist_urb(ep, urb);

	urb->trbcc = 0;

	/* EP0 request is fixed location */
	if (urb->buf == NULL) {
		/* This is for status xfer urbuest */
		if (xdc->uxdc->reconnect) {
			xdc->uxdc->reconnect = 0;
			xdci_force_speed(xdc->uxdc);
		}
	} else if (urb->buf == xdc->setup_buf) {
		/* for control setup stage or status stage */
		if (xdc->setup_pkt.bRequest == UR_SET_SEL) {
			struct lpm_save *lpm = (struct lpm_save *)xdc->setup_buf;
			xdc->lpm.u1pel = lpm->u1pel;
			xdc->lpm.u1sel = lpm->u1sel;
			xdc->lpm.u2pel = lpm->u2pel;
			xdc->lpm.u2sel = lpm->u2sel;
			xdc_dbg(("LMP { u1pel=%x, u1sel=%x, u2pel= %x, u2sel= %x}\n",
				lpm->u1pel, lpm->u1sel, lpm->u2pel, lpm->u2sel));
			/* TBD, need to set the register for those values */
		}
		urb->buf = NULL;
		xdc->ctrl_xfer = 0;
	} else {
		p = (void *)urb->buf;
		if (xdc->ep0_in) {
			/* For control xfer IN data stage */
			PKTFREE(xdc->osh, p, FALSE);
			urb->buf = NULL;
		} else {
			/* For control xfer out data, send up */
			if (urb->length && urb->actual) {
				PKTSETLEN(xdc->uxdc->osh, p, urb->actual);
				usbdev_rx(xdc->uxdc->bus, ep->index, p);
			}
			urb->buf = NULL;
		}
		xdc->ctrl_xfer = 0;
	}

	/* double check RX buffer */
	xdc_rx_malloc_chk(xdc, NULL);
} /* xdc_urb_ctrl_done */

#endif /* USB_XDCI */
