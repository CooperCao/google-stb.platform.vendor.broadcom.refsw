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
 * $Id: xdc_cmd.c 397267 2013-04-18 00:45:16Z $
*/

/**
 * Commands are issued using command TRBs inserted into a command ring (CR). Command TRBs are
 * read-only by the xDC controller. After the xDC processed a command, it adds a completion in
 * the event ring and interrupts the ARM. xDCI defines the following command TRBs:
 * - No Op. A CR test mechanism.
 * - Address Device. Configure EP0 and/or set device address.
 * - Address Endpoint
 * - Configure Endpoint. Configure/de-configure selected endpoints.
 * - Reset Endpoint. Remove Stall/Halt condition from an endpoint.
 * - Stop Endpoint. Stop or abort the operation of an endpoint.
 * - Flow Control Endpoint. Set endpoint to NRDY or ERDY. Use discouraged.
 * - Stall Endpoint. Set endpoint to Stall or Halt.
 * - Set Transfer Ring Dequeue Pointer
 * - Force Header Command. Software generated SS LMPs or TPs, including SS notification messages.
 * - Force Event Command
 * 
 * When the CR Doorbell is rung (DBLCMD) by software, xDC will evaluate the Command TRB pointed to
 * by the CR DQP. Once started (with software doorbell write), xDC processes Command TRBs and
 * advances the CR DQP until the ring is empty.
 *
 * The current CR DQP is maintained internally by xDC, and not directly exposed to software. Its
 * value is reported in the Command TRB Pointer field of Command Completion Events (CCEs).
 */
 
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <usb.h>
#include <usbstd.h>
#include <usbdev_dbg.h>
#include "xdc.h"
#include "xdc_rte.h"

#ifdef USB_XDCI

struct xdc_trb *xdc_issue_cmd(struct xdcd *xdc,
	struct xdc_ep *ep, uint32 cmd);
static int wait_cmd_complete(struct xdcd *xdc, struct xdc_ep *ep,
	uint32 cmd, struct xdc_trb *trb);
struct xdc_trb *submit_cmd(struct xdcd *xdc,
	uint32 field0, uint32 field1, uint32 field2, uint32 field3);

void cmd_stop_ring_completion(struct xdc_ep *ep);
static void cmd_set_deq_completion(struct xdc_ep *ep);
static void cmd_reset_ep_completion(struct xdc_ep *ep);
int xdc_function_wake(struct xdcd *xdc);

#ifdef XDC_RESET_CMDRING

static void xdc_reset_cmdring(struct xdcd *xdc, uint32 force_reset)
{
	struct xdc_ring *cmd_ring;
	uint32 val[2];

	cmd_ring = xdc->cmd_ring;

	if (cmd_ring->dequeue == cmd_ring->trbs || force_reset) {
		cmd_ring->enqueue = cmd_ring->trbs;
		cmd_ring->dequeue = cmd_ring->enqueue;
		cmd_ring->cycle = 1;

		val[1] = 0;

		xdc_R_REG64(&val[0], xdc, &xdc->opregs->cmd_ring[0]);

		if (val[0]&B_3) {
			/* cmd ring still running */
			xdc_dbg(("crcrl = %x \n", val[0]));
			OSL_DELAY(10);
			val[0] |= B_1;

			xdc_W_REG64(xdc, val[0], val[1], &xdc->opregs->cmd_ring[0]);

			do {
				xdc_R_REG64(&val[0], xdc, &xdc->opregs->cmd_ring[0]);
				xdc_dbg(("crcrl = %x \n", val[0]));
				OSL_DELAY(2);

			} while ((val[0]&B_3) == 1);
		}
		val[0] = (val[0] & (uint32) CMD_RING_ALIGN_BITS) |
			(((uint32)cmd_ring->trbs)&((uint32) ~ CMD_RING_ALIGN_BITS)) |
			xdc->cmd_ring->cycle;

		xdc_W_REG64(xdc, val[0], val[1], &xdc->opregs->cmd_ring[0]);
		OSL_DELAY(2);
	}
} /* xdc_reset_cmdring */
#endif /* XDC_RESET_CMDRING */

/**
 * Commands such as SET_DEQ_TRB, are queued on a command ring and read out later by the xdc
 * controller. This function waits for command completion.
 */
int xdc_cmd(struct xdcd *xdc, struct xdc_ep *ep, uint32 cmd)
{
	struct xdc_trb *trb;
	int ret = 0;
	xdc->togo_cmd = 0;

	xdc_trace(("xdc_cmd = %x \n", cmd));
	trb = xdc_issue_cmd(xdc, ep, cmd);

	if (trb == NULL) {
		xdc_dbg(("issue command failed, ret = %d \n", ret));
		/* we will retry this cmd int xdc_cmd_process() in background */
		xdc->togo_cmd = cmd;
		return -1;
	}

	xdc_incr_deqptr(xdc, xdc->cmd_ring, xdc->trbs_per_cmdring);

	ret = wait_cmd_complete(ep->xdc, ep, cmd, trb);

	if (ret != 0) {
		xdc_err(("command exection timeout, ret = %d \n", ret));
		/* TBD, do anything to correct it??  */
		/* This maybe not real fail, maybe ISR is fast than us */
	}

#ifdef XDC_RESET_CMDRING
	xdc_reset_cmdring(xdc, 1);
#endif

	return ret;
} /* xdc_cmd */

/**
 * Posting of command on cmd ring, does not wait for cmd completion. To issue a command, software 
 * - Writes the command TRB in the CR (Command Ring)
 * - Has the command submitted by flipping its Cycle bit.
 * - Writes to register DELCMD with field TGT equal to 0.
 */
struct xdc_trb *xdc_issue_cmd(struct xdcd *xdc,
	struct xdc_ep *ep, uint32 cmd)
{
	uint32 ep_index;
	uint32 dword0, dword3;
	struct xdc_trb *trb;


	dword0 = dword3 = 0;
	ep_index = ep->index;
	xdc->cmd_callback = NULL;

	dword3 = TRB_TYPE(cmd) | EPX_SLOT_ID;

	switch (cmd)
	{
		case STALL_EP_TRB:
			xdc_dbg(("xdc_ep_set_stall  ep_index=%d \n", ep_index));
			dword3 |= EP_ID_TO_TRB(ep_index);
			break;
		case RESET_EP_TRB:
			xdc_dbg(("xdc_ep_clear_stall ep=%p \n", ep));
			dword3 |= EP_ID_TO_TRB(ep_index)|TRB_TSP;
			xdc->cmd_callback = cmd_reset_ep_completion;
			break;
		case STOP_RING_TRB:
			dword3 |= EP_ID_TO_TRB(ep_index);
			dword3 |= SUSPEND_PORT_TO_TRB(FALSE);
			xdc->cmd_callback = cmd_stop_ring_completion;
			break;
		case CONFIG_EP_TRB:
			xdc_trace(("xdc_configure_endpoint\n"));
			dword0 = xdc->inctx_adr;
			break;
		case ADDR_DEV_TRB:
			dword0 = xdc->inctx_adr;
			if (xdc->dev_state == XDC_STATE_CONNETED)
			{
				dword3 |= TRB_BSR;
			}
			break;
		case SET_DEQ_TRB:
			dword0 = (uint32)&(ep->dequeue->buf_addr);
			dword0 |= ep->cycle_bit;
			dword3 |= EP_ID_TO_TRB(ep_index);
			xdc->cmd_callback = cmd_set_deq_completion;
			break;
		default:
			return NULL;
	}

	trb = submit_cmd(xdc, dword0, 0, 0, dword3);

	if (trb) {
		/* ring the command door bell */
		W_REG(xdc->osh, &xdc->dba->db_cmd, 0x00);
	}

	return trb;
} /* xdc_issue_cmd */

/** waits for a command on the command ring to be consumed and completed by the xdc core */
static int wait_cmd_complete(struct xdcd *xdc, struct xdc_ep *ep, uint32 cmd,
	struct xdc_trb *trb)
{
	int status = 0;
	uint32 timeout;
	struct xdc_trb * evt_trb;
	uint32 cycle_bit;
	struct xdc_ring *evt_ring;
	uint32 flags;

	xdc_trace(("wait_cmd_complete \n"));

	timeout = CMD_TIMEOUT;
	evt_ring = xdc->event_ring;
	evt_trb = evt_ring->dequeue;
	cycle_bit = xdc->event_ring->cycle;


	while (timeout) {
		/*
		* just check command done, and go away
		* It will wait for interrupt and xdc_isr() to handle event deq pointer
		*/
		flags = ltoh32(evt_trb->flags);
		if ((flags & TRB_CYCLE) == cycle_bit) {
			/* Got a event, check if this a Command TRB or not */
			if ((flags & TRB_TYPE_BITMASK) ==  TRB_TYPE(CMD_COMP_TRB)) {
				uint32 evttrb_adr;
				uint32 cmdtrb_adr;

				evttrb_adr = (uint32)(evt_trb->buf_addr);
				cmdtrb_adr = (uint32)&(trb->buf_addr);
				xdc_dbg(("evttrb_adr=%x cmdtrb_adr= %08x \n",
					evttrb_adr, cmdtrb_adr));

				if (evttrb_adr == (uint32)cmdtrb_adr) {
					if (TRB_TYPE(cmd) ==
						(ltoh32(trb->flags)&TRB_TYPE_BITMASK)) {
						xdc_dbg(("cmd done and timeout = %d ", timeout));

						/* cmd completion TRB */
						status = GET_CC_CODE(ltoh32(evt_trb->status));
						if (status != CC_SUCCESS) {
							xdc_err(("CMD not completed as normal"));
							xdc_err(("check CC code = %d \n", status));
							/* TBD, do anything to correct it?? */
							return -2;
						}
						status = 0;
						break;
					} else {
					/* cmd TRB different from we were looking for */
						xdc_err(("err cmd done TRB diff from expected"));
						return -3;
					}
				}
			}

			/* Not find match TRB, keep search */
			if (evt_trb == &evt_ring->trbs[xdc->trbs_per_evtring - 1])
			{
				evt_trb = evt_ring->trbs;
				cycle_bit ^= 0x01; /* flip cycle bit */
			} else {
				evt_trb++;
			}

			xdc_dbg(("event_trb=%p cycle=%d \n",
				evt_trb, cycle_bit));
		}
		timeout--;
	}

	if (timeout == 0) {
		status = -4;
	}

	if (status) {
		xdc_err(("ERR cmd failed ret=%d, cmd = %d \n", status, cmd));
	}

#ifdef XDC_DEBUG
	if (ep) {
		xdc_dump_ctx(xdc, XDC_DEVICE_CTX, ep->index, ep->index);
	}
#endif /* XDC_DEBUG */

	return status;
} /* wait_cmd_complete */


/** sets USB address */
int xdc_set_address(struct xdcd *xdc, uint32 addr)
{
	int ret = 0;
	struct xdc_slx *slx;
	struct xdc_ctx_icc *icc_ctx;
	struct xdc_epctx *ep_outctx;
	struct xdc_epctx *ep0_inctx;

	slx = (struct xdc_slx *)(xdc->inctx) + 1;
	ep0_inctx = (struct xdc_epctx *)(xdc->inctx) + 2;

	xdc_dbg(("xdc_set_address addr=%d inslot_ctx->info=%x \n",
	         addr, slx->info));
	xdc_dbg(("inslx->dev_state = %x \n", slx->dev_state));

	if (!slx->info) {
		/* ep0_inctx at inctx[2] */

		slx->info |= htol32(SLX_LCE(1));
		ep0_inctx->info2 = htol32(EPX_EP_TYPE(CTRL_EP));

		xdc_dbg(("slot_ctx->info =%x xdc->speed=%x\n",
			slx->info, xdc->speed));

		if (xdc->speed == XDC_PORT_SS) {
			ep0_inctx->info2 |= htol32(EPX_MPS(512));
		} else if (xdc->speed == XDC_PORT_HS || xdc->speed == XDC_PORT_FS) {
			ep0_inctx->info2 |= htol32(EPX_MPS(64));
		} else if (xdc->speed == XDC_PORT_LS) {
			ep0_inctx->info2 |= htol32(EPX_MPS(8));
		} else {
			xdc_err(("speed unknown!!!!\n"));
			ASSERT(0);
			return -1;
		}

		slx->info |= htol32((xdc->speed)<<20);

		ep0_inctx->info2 |= htol32(EPX_MAX_BURST(0));

		ep0_inctx->deq = htol32(((uint32)(xdc->ep[0].xf_ring->trbs)) |
			xdc->ep[0].xf_ring->cycle);
		ep0_inctx->deqh = 0;
		xdc_dbg(("slx->info =%x xdc->speed=%x\n",
			slx->info, xdc->speed));
		xdc_dbg((" ep0_inctx->info1=%8x, info2 = %8x",
			ep0_inctx->info1, ep0_inctx->info2));
		xdc_dbg((" ep0_inctx->deq =%8x, deqh = %8x \n",
			ep0_inctx->deq, ep0_inctx->deqh));
	} else {
		struct xdc_ring *xf_ring;

		xf_ring = xdc->ep[0].xf_ring;
		ep0_inctx->deq = htol32((uint32)(&(xf_ring->enqueue->buf_addr))
						| xf_ring->cycle);
		xdc_dbg(("ep0_inctx->deq =%8x \n", ep0_inctx->deq));

	}

	icc_ctx = (struct xdc_ctx_icc *)(xdc->inctx);
	icc_ctx->add_ctx_flag = htol32(SLOT_FLAG | EP0_FLAG);
	icc_ctx->drop_ctx_flag = 0;

	slx->dev_state &= ~SLX_ADDR_MASK;
	slx->dev_state |= addr;
#ifdef XDC_DEBUG
	xdc_dump_ctx(xdc, XDC_INPUT_CTX, 0, 0);
#endif /* XDC_DEBUG */
	xdc_cmd(xdc, &xdc->ep[0], ADDR_DEV_TRB);
	slx = (struct xdc_slx *)(xdc->outctx);
	if (xdc->dev_state == XDC_STATE_CONNETED)
		xdc->dev_state = XDC_STATE_DEFAULT;

	if (addr) {
		xdc->dev_state = XDC_STATE_ADDRESS;
		xdc->uxdc->addr = addr;
	}

	ep_outctx =  (struct xdc_epctx *)(xdc->outctx) + 1;

	if ((ep_outctx->info1&0x0F) == EP_HALTED) {
		xdc_halt_ep(&xdc->ep[0], 0);
	}

	return ret;
} /* xdc_set_address */

/**
 * called back on STOP_RING_TRB command completion. xDC's execution of TDs on an EP was stopped so
 * firmware can:
 * - Temporarily take ownership of TR to alter TDs that had previously been passed to the controller
 * - Stop USB activity prior to powering down the xDC
 *
 * While the endpoint is stopped, software may add, delete, or otherwise rearrange TDs on an
 * associated TR.  Examples include but not limited to:
 * - Insert 'high-priority' TDs at the DQP so they will be executed immediately when the ring is
 *   restarted, or
 * - 'Abort' one or more TDs by removing them from the ring.
 */
void cmd_stop_ring_completion(struct xdc_ep *ep)
{
	struct xdc_ring *xf_ring;
	struct xdcd *xdc;
	struct xdc_trb *cur_trb = NULL;

	xdc_dbg(("cmd_stop_ring_completion \n"));

	xdc = ep->xdc;

	xf_ring = ep->xf_ring;

	if (xf_ring && xf_ring->dequeue)
	{
		cur_trb = xf_ring->dequeue;
		if (ep->deq_urb) {
			if (cur_trb == ep->deq_urb->trb) {
				xdc_get_new_deq(ep, ep->deq_urb);
			} else {
				xdc_noop_trb(xdc, xf_ring, ep->deq_urb);
			}
		}

		/* ep->dequeue is set in xdc_get_new_deq() */
		if (ep->dequeue != cur_trb) {
			xdc_set_dequeue(ep);
		}

		if (ep->ring_reactive) {
			xdc_active_rings(ep);
			ep->ring_reactive = 0;
		}
	}

	ep->deq_urb = NULL;
} /* cmd_stop_ring_completion */

/** called back on SET_DEQ_TRB command completion */
static void cmd_set_deq_completion(struct xdc_ep *ep)
{
	unsigned int ep_index;
	struct xdc_ring *xf_ring;
	struct xdc_epctx *ep_ctx;
	struct xdcd *xdc;

	xdc = ep->xdc;
	ep_index = ep->index;
	xf_ring = ep->xf_ring;

	if (xf_ring) {
		ep_ctx =  (struct xdc_epctx *)(xdc->outctx) + ep_index + 1;

		if (xdc->cmd_status == CC_SUCCESS) {
			xdc_dbg(("Set Deq Ptr done suceed = %x\n",
				ep_ctx->deq));
			if ((uint32)(&ep->dequeue->buf_addr) ==
				(htol32(ep_ctx->deq) & ~(EPX_CYCLE_MASK)))
			{
				xf_ring->dequeue = ep->dequeue;
			} else {
				xdc_err(("Mismatch cycle bit ep deq ptr = %p\n", ep->dequeue));
			}
			xdc_active_rings(ep);
		}
	}

	ep->dequeue = NULL;
} /* cmd_set_deq_completion */

/** called back on RESET_EP_TRB command completion */
static void cmd_reset_ep_completion(struct xdc_ep *ep)
{
	if (ep->xdc->cmd_status == CC_SUCCESS) {
		xdc_active_rings(ep);
	} else {
		xdc_err(("reset ep command failed \n"));
	}
}

/** calls back cmd_reset_ep_completion, cmd_stop_ring_completion, cmd_set_deq_completion */
int xdc_cmd_event(struct xdcd *xdc, struct xdc_trb *event)
{
	struct xdc_trb *trb;
	unsigned int ep_index;

	xdc_dbg(("xdc_cmd_event \n"));
	xdc->cmd_status = GET_CC_CODE(ltoh32(event->status));

	trb = (struct xdc_trb *)ltoh32(event->buf_addr);
	ep_index = EP_INDEX_OF_TRB(ltoh32(trb->flags));

	xdc_dbg(("xdc_cmd_event cc = %d \n", xdc->cmd_status));
	xdc_dbg(("xdc_cmd_event trb = %p \n", trb));
	xdc_dbg(("xdc_cmd_event pe_index = %d \n", ep_index));

	if (xdc->cmd_callback && ep_index == 0) {
		xdc_dbg(("cmd call back ...\n"));
		xdc->cmd_callback(&xdc->ep[ep_index]);
	}

	return 0;
}

/** enqueues a command on the command ring */
struct xdc_trb *submit_cmd(struct xdcd * xdc,
	uint32 field0, uint32 field1, uint32 field2, uint32 field3)
{
	int ret;
	struct xdc_trb *trb;
	struct xdc_ring *ring;

	ring = xdc->cmd_ring;

	xdc_trace(("submit_cmd  "));

	xdc_enq_is_lktrb(xdc, ring, xdc->trbs_per_cmdring);

	ret = xdc_chk_ring_space(xdc, ring, EP_RUNNING, 1, xdc->trbs_per_cmdring);

	if (ret < 1) {
		xdc_err(("ERR: No room for command on command ring\n"));
		ASSERT(0);
		return NULL;
	}

	trb = ring->enqueue;

	xdc_dbg(("trb=%p field0=%x field1=%x field2=%x field3=%x\n",
		trb, field0, field1, field2, field3));

	trb->buf_addr = htol32(field0);
	trb->buf_addrh = htol32(field1);
	trb->status = htol32(field2);
	trb->flags = htol32(field3)|ring->cycle;

	ring->enqueue++;

	return trb;
} /* submit_cmd */

/**
 * Issues SET_DEQ_TRB command and waits for its completion. It can only be used while the EP is in
 * Error or Stopped state.
 */
void xdc_set_dequeue(struct xdc_ep *ep)
{
	int ret = 0;

	ret = xdc_cmd(ep->xdc, ep, SET_DEQ_TRB);

	if (ret == -1 && ep->xdc->togo_cmd == SET_DEQ_TRB) {
		/* need to find a chance to resend this cmd? */
		return;
	}

	ep->state |= XDC_EP_BUSY;
}

/** Remote wakeup related: host wakes dongle. Called on a port event. */
int xdc_function_wake(struct xdcd *xdc)
{
	int ret;
	uint32	field0, field1;
	struct xdc_trb * trb;

	field0 = 0;
	field1 = 0;

	xdc_dbg_ltssm(("In xdci_function_wake\n"));
	/* Refer to USB 3.1 spec 8.5.6.1 for the format of field0 and field1 */
	field0 |=  USB3_PACKET_TYPE_TRANSACTION << USB3_FW_DN_W0_TYPE_LSB;
	field0 |=  xdc->uxdc->addr << USB3_FW_DN_W0_DEVICE_ADDRESS_LSB;

	field1 |=  UBS3_TRANSACTION_SUB_TYPE_DEV_NOTIFICATION << USB3_FW_DN_W1_SUB_TYPE_LSB;
	field1 |=  USB3_NOTIFICATION_TYPE_FUNCTION_WAKE << USB3_FW_DN_W1_NOTIFICATION_TYPE_LSB;

	xdc_dbg_ltssm(("field0=%08x field1=%08x \n", field0, field1));

	trb = submit_cmd(xdc, field0, field1, 0, TRB_TYPE(FORCE_HEADER_TRB));

	if (trb) {
		/* ring the command door bell */
		W_REG(xdc->osh, &xdc->dba->db_cmd, 0x00);
	}

	xdc_incr_deqptr(xdc, xdc->cmd_ring, xdc->trbs_per_cmdring);

	ret = wait_cmd_complete(xdc, 0, TRB_TYPE(FORCE_HEADER_TRB), trb);
	if (ret != 0) {
		xdc_err(("command execution timeout, ret = %d \n", ret));
		/* TBD, do anything to correct it??  */
		/* This maybe not real fail, maybe ISR is fast than us */
	}

	xdc_dbg_ltssm(("Function wake completed\n"));

	return 0;
} /* xdc_function_wake */

#endif /* USB_XDCI */
