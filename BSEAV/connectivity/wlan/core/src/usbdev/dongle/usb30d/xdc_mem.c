/*
 * xDC memory configuration
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: xdc_mem.c 400352 2013-05-03 22:20:47Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <usb.h>
#include <usbdev_dbg.h>
#include "xdc.h"
#include "xdc_rte.h"

#ifdef USB_XDCI

int get_ep_type(unsigned int ep_type, unsigned int ep_dir);
static void xdc_free_scratchpad(struct xdcd *xdc);
static void xdc_free_eps(struct xdcd *xdc);
static int xdc_set_dcbaa(struct xdcd *xdc);
static int xdc_set_cmd_ring(struct xdcd *xdc);
static int xdc_set_evt_ring(struct xdcd *xdc);
static int xdc_set_dev_ctx(struct xdcd *xdc);
static int xdc_set_ctrl_pipe(struct xdcd *xdc);
static int xdc_set_scratchpad(struct xdcd *xdc);
static void xdc_free_ctx(struct xdcd *xdc);
static int xdc_alloc_ctx(struct xdcd *xdc, uint32 type);

/** called once in firmware lifetime */
int xdc_init_mem(struct xdcd *xdc)
{
	uint32 ret;
	uint32 otp_u1u2;

	xdc_trace(("xdc_init_mem \n"));

	xdc->cparams = R_REG(xdc->osh, &xdc->cpregs->cparams);
	xdc->sp_num = XDC_SCPAD_NUM(R_REG(xdc->osh, &xdc->cpregs->sparams2));

	xdc->page_size = (R_REG(xdc->osh, &xdc->opregs->page_size))<<12;
	ret = R_REG(xdc->osh, &xdc->cpregs->db_off);
	ret &= DBOFF_MASK;
	xdc->dba = (struct xdc_db *)((uint32)xdc->cpregs + ret);
	xdc_dbg(("xdc->dba=%p\n", xdc->dba));

	ret = xdc_set_dcbaa(xdc);      /** Device context base address array */
	ret |= xdc_set_cmd_ring(xdc);
	ret |= xdc_set_evt_ring(xdc);
	ret |= xdc_set_dev_ctx(xdc);
	ret |= xdc_set_scratchpad(xdc);
	ret |= xdc_set_ctrl_pipe(xdc);

	xdc->speed = XDC_PORT_SS;
	xdc->ctrl_pipe_state = CTRL_PIPE_IDLE;
	xdc->dev_state = XDC_STATE_CONNETED;

	if (ret)
		xdc_free_mem(xdc);

	otp_u1u2 = xdci_get_otp_var_u1u2(xdc);

	if (otp_u1u2 & XDCI_OTP_VAR_ENABLE_PIPE_RESET_MASK) {
		uint32 *susptd_reg = (uint32 *) ((char*)xdc->cpregs + 0x42c);
		uint32 susptd_val;
		xdc_dbg_ltssm(("Enable pipe reset on recovery\n"));
		susptd_val = R_REG(xdc->uxdc->osh, susptd_reg);
		xdc_dbg_ltssm(("susptd_read=%08x ", susptd_val));
		susptd_val |= 1 << XDC_SUSPD_PIPE_RESET_BIT;
		xdc_dbg_ltssm(("susptd_write=%08x ", susptd_val));
		W_REG(xdc->uxdc->osh, susptd_reg, susptd_val);
		susptd_val = R_REG(xdc->uxdc->osh, susptd_reg);
		xdc_dbg_ltssm(("susptd_read=%08x \n", susptd_val));
	}

	if (otp_u1u2 & XDCI_OTP_VAR_EXTEND_LFPS_MASK) {
		uint32 *susptd_reg = (uint32 *) ((char*)xdc->cpregs + 0x42c);
		uint32 susptd_val;
		xdc_dbg_ltssm(("Increase LFPS duration\n"));
		susptd_val = R_REG(xdc->uxdc->osh, susptd_reg);
		xdc_dbg_ltssm(("susptd_read=%08x ", susptd_val));
		susptd_val &=  ~ (1 << XDC_SUSPD_LFPS_BIT);
		xdc_dbg_ltssm(("susptd_write=%08x ", susptd_val));
		W_REG(xdc->uxdc->osh, susptd_reg, susptd_val);
		susptd_val = R_REG(xdc->uxdc->osh, susptd_reg);
		xdc_dbg_ltssm(("susptd_read=%08x \n", susptd_val));
	}

	if (otp_u1u2 & XDCI_OTP_VAR_ENABLE_U1_ENTRY_MASK) {
		uint32 portpm;
		portpm = R_REG(xdc->uxdc->osh, &xdc->opregs->portpw);
		xdc_dbg_ltssm(("portmpm_read=%x ", portpm));
		portpm &= ~XDCI_PORT_U1TO(0xff);
		portpm |= XDCI_PORT_U1TO(0xf8);
		portpm |= XDCI_PORT_W1S | XDC_PORTPM_U2A;
		xdc_dbg_ltssm(("portpm_write =%x\n", portpm));
		W_REG(xdc->uxdc->osh, &xdc->opregs->portpw, portpm);
	}

	xdc->test_mode = 0;
	xdc->test_mode_nr = 0;

	return ret;
} /* xdc_init_mem */

/**
 * Device Context Base Address Array. Is always two elements long. DCBAA[0] points at scratchpad
 * buffer, DCBAA[1] at device context (DVX). DVX in its turn it 32 entries long and contains Slot
 * Context (SLX) in DVX[0] and endpoint (EP) related information called Endpoint Context (EPX) in
 * the other 31 entries.
 */
static int xdc_set_dcbaa(struct xdcd *xdc)
{
	void *mem;

	mem = MALLOC_ALIGN(xdc->osh, sizeof(struct xdc_dca), XDC_DCBAA_ALIGN_BITS);
	if (mem == NULL)
		return NULL;

	xdc->dcbaa_ptr = (struct xdc_dca *)mem;

	bzero(xdc->dcbaa_ptr, sizeof(struct xdc_dca));
	xdc->dcbaa_ptr->mem = mem;
	xdc_dbg(("dcbaa_ptr =%p \n", mem));

	xdc_W_REG64(xdc, (uint32) mem, 0, &xdc->opregs->dcbaa[0]);

	return 0;
}

/** Allocates command ring in dongle memory and supply xDC core with its location */
static int xdc_set_cmd_ring(struct xdcd *xdc)
{
	uint32 val[2];

	xdc->cmd_ring = xdc_alloc_ring(xdc, TRUE, xdc->trbs_per_cmdring);
	if (!xdc->cmd_ring)
		return 1;

	xdc_dbg(("Allocated command ring at %p\n", xdc->cmd_ring));
	xdc_dbg(("cmd_ring->trbs is 0x%x\n", (uint32) xdc->cmd_ring->trbs));

	xdc_R_REG64(&val[0], xdc, &xdc->opregs->cmd_ring[0]);
	OSL_DELAY(2);

	val[0] = (val[0] & (uint32) CMD_RING_ALIGN_BITS) |
		(((uint32)xdc->cmd_ring->trbs)&((uint32) ~ CMD_RING_ALIGN_BITS)) |
		xdc->cmd_ring->cycle;
	val[1] = 0;

	xdc_W_REG64(xdc, val[0], val[1], &xdc->opregs->cmd_ring[0]);

	return 0;
}

/** Allocates event ring in dongle memory and supply xDC core with its location */
static int xdc_set_evt_ring(struct xdcd *xdc)
{
	struct xdc_erst *er;
	uint32 val, tmp[2];

	xdc->irset = &xdc->rtregs->irset[0];

	xdc_dbg(("Allocating event ring\n"));
	xdc->event_ring = xdc_alloc_ring(xdc, FALSE, xdc->trbs_per_evtring);

	if (!xdc->event_ring)
		return 1;

	xdc->er = (struct xdc_erst *)MALLOC_ALIGN(xdc->osh,
		sizeof(struct  xdc_erst) * ERST_SEG_NUM, XDC_ALIGN_BITS);

	if (!xdc->er) return 1;

	bzero(xdc->er, sizeof(struct xdc_erst)*ERST_SEG_NUM);

	xdc->num_er = ERST_SEG_NUM;
	xdc->erst_addr = (unsigned int) xdc->er;

	er = &xdc->er[0];

	er->seg_addr64[0] = htol32((uint32)xdc->event_ring->trbs);
	er->seg_addr64[1] = 0;

	er->seg_sz = htol32(xdc->trbs_per_evtring);
	er->rsv = 0;

	val = R_REG(xdc->osh, &xdc->irset->erst_sz);
	val &= ERST_SZ_MASK;
	val = er->seg_sz<<16;
	val |= ERST_SEG_NUM;

	W_REG(xdc->osh, &xdc->irset->erst_sz, val);

	xdc_R_REG64(&tmp[0], xdc, &xdc->irset->erst_base64[0]);
	tmp[0] &= ERST_PTR_ALIGN;
	tmp[0] |= (xdc->erst_addr & (uint32) ~ ERST_PTR_ALIGN);

	tmp[1] = 0;
	OSL_DELAY(2);

	xdc_W_REG64(xdc, tmp[0], tmp[1], &xdc->irset->erst_base64[0]);

	val = (uint32)&xdc->event_ring->dequeue->buf_addr;

	ASSERT(val != 0);
	OSL_DELAY(2);

	xdc_R_REG64(&tmp[0], xdc, &xdc->irset->erst_deq64[0]);

	tmp[0] &= ERST_PTR_ALIGN;
	tmp[0] &= ~ERST_EHB_BITS;

	val = (val & (uint32) ~ ERST_PTR_ALIGN) | tmp[0];
	OSL_DELAY(2);

	xdc_W_REG64(xdc, val, 0, &xdc->irset->erst_deq64[0]);
	OSL_DELAY(2);

	return 0;
} /* xdc_set_evt_ring */

/** allocates device and input contexts in host memory */
static int xdc_set_dev_ctx(struct xdcd *xdc)
{
	int ret = 0;

	/* alloc  1 SLX +31 EPX ctx for device output CTX information */
	ret = xdc_alloc_ctx(xdc, XDC_DEVICE_CTX);
	if (ret)
		return 1;

	xdc_dbg(("out_ctx = %p ", xdc->outctx));

	/* alloc 1 ICC + 1 SLX + 31 EPX ctx for SW to alter CTX information */
	ret = xdc_alloc_ctx(xdc, XDC_INPUT_CTX);
	if (ret)
		return 2;

	xdc_dbg(("xdc->in_ctx  = %p ", xdc->inctx));

	/* Point to output device context in dcbaa */
	xdc->dcbaa_ptr->devctx_ptr1[0] = htol32(xdc->outctx_adr);
	xdc->dcbaa_ptr->devctx_ptr1[1] = 0;

	return 0;
}

/** Allocate data structure for endpoint 0. Called once during firmware lifetime. */
static int xdc_set_ctrl_pipe(struct xdcd *xdc)
{
	void *mem;

	/* Allocate setup buf */
	mem = MALLOC_ALIGN(xdc->osh, 512, 4);
	if (mem == NULL)
		return -1;

	xdc->setup_buf = (uint8 *)mem;

	/* allocate LBUF size and copy setup packet data */
	mem = PKTGET(xdc->osh, sizeof(usb_device_request_t), FALSE);

	if (mem == NULL)
		return 2;

	xdc->stppkt_lbuf = mem;
	bzero(xdc->stppkt_lbuf, sizeof(usb_device_request_t));

	xdc->ctrl_xfer = 0;

	return 0;
}

/** called every time an empty buffer is provided to the xDC core for host->dongle traffic */
inline struct xdc_urb *xdc_alloc_rx_urb(struct xdcd *xdc, struct xdc_ep *ep, void *p)
{
	struct xdc_urb *urb;
	uint32 len, max_wr;

	len = ep->blk_sz;
	max_wr = ep->max_urb;

	if (p == NULL) {
#if defined(BCMPKTPOOL_ENABLED)
		p = pktpool_get(SHARED_POOL);
#else
		p = PKTGET(xdc->osh, len, FALSE);
#endif
	}

	if (p == NULL) {
		xdc_dbg(("err: rx alloc faile= %d\n", len));
		return NULL;
	}

	PKTSETLEN(xdc->osh, p, len);
	PKTSETNEXT(xdc->osh, p, NULL);

	urb = (struct xdc_urb *)ep->urb + ep->urb_wr;
	if (urb->buf) {
		xdc_err(("urb ovwr\n"));
		PKTFREE(xdc->osh, p, FALSE);
		return NULL;
	}

	ep->urb_wr = (ep->urb_wr + 1) % max_wr;

	urb->buf = p; /* point back to lbuf struct */
	urb->lbuf = 1;
	urb->trb_num = 0;
	urb->length = len;
	urb->actual = 0;
	urb->trbcc = 0;

	return urb;
} /* xdc_alloc_rx_urb */

/**
 * Make the last TRB as link trb and loop back to first trb. Link TRBs provides support for sizing
 * and non-contiguous Transfer and Command Rings. 
 * - A Link TRB indicates the end of a Ring by providing a pointer to the beginning of the ring
 * - If contiguous Pages cannot be allocated by system software to form a large Transfer Ring, then
 *   Link TRBs may be used to link together multiple memory Pages to form a single Transfer Ring.
 */
static void xdc_loop_trbs(struct xdcd *xdc, struct xdc_ring *ring, bool link_trbs, uint32 ring_sz)
{
	uint32 val;
	if (!ring)
		return;

	if (link_trbs) {
		/* Not Event Ring */
		ring->trbs[ring_sz - 1].buf_addr =  htol32((uint32)(ring->trbs));
		ring->trbs[ring_sz - 1].buf_addrh = 0;

		val = ltoh32(ring->trbs[ring_sz - 1].flags);
		val &= ~TRB_TYPE_BITMASK;
		val |= TRB_TYPE(LINK_TRB);
		ring->trbs[ring_sz - 1].flags = htol32(val);
	}
}

/** Called after e.g. a USB bus reset */
void xdc_free_ring(struct xdcd *xdc, struct xdc_ring *ring, int trbs_num)
{
	if (!ring)
		return;

	if (ring->trbs)
		MFREE(xdc->osh, (void *)ring->trbs, trbs_num*16);
	ring->trbs = 0;
	MFREE(xdc->osh, ring, sizeof(struct xdc_ring));
}

/** Allocates cmd, event and transfer rings */
struct xdc_ring *xdc_alloc_ring(struct xdcd *xdc, bool link_trbs, int trbs_num)
{
	struct xdc_ring *ring;

	/* xdc_trace(("xdc_alloc_ring\n")); */

	/* can be freed on bus reset */
	ring = (struct xdc_ring *) MALLOC_NOPERSIST(xdc->osh, sizeof(struct xdc_ring));

	if (!ring)
		return NULL;

	ring->trbs = (struct xdc_trb *)
		MALLOC_ALIGN(xdc->osh, trbs_num*16, XDC_ALIGN_BITS + 2);

	if (ring->trbs) {
		bzero((void *)ring->trbs, trbs_num*16);
		/* make the last trb as link trb and loop back to first trb */
		xdc_loop_trbs(xdc, ring, link_trbs, trbs_num);

		if (link_trbs) {
			ring->trbs[trbs_num - 1].flags |=  htol32(LINK_CYCLE);
		}

		ring->enqueue = ring->trbs;
		ring->dequeue = ring->enqueue;
		ring->cycle = 1;

		return ring;
	}

	xdc_free_ring(xdc, ring, trbs_num);

	return NULL;
} /* xdc_alloc_ring */

/** Allocates a device or input context */
static int xdc_alloc_ctx(struct xdcd *xdc, uint32 type)
{
	/* we should just 32 bytes address */
	if (type == XDC_INPUT_CTX) {
		xdc->inctx_size = 1024 + 32;
		xdc->inctx = (uint8 *)MALLOC_ALIGN(xdc->osh,
			xdc->inctx_size, XDC_DCBAA_ALIGN_BITS);

		if (xdc->inctx == NULL)	return -1;

		xdc->inctx_adr = (uint32)xdc->inctx;
		bzero(xdc->inctx, xdc->inctx_size);
		W_REG(xdc->osh, (uint32 *)XDC_WAR_REG2, xdc->inctx_adr);
	} else {
		xdc->outctx_size = 1024;
		xdc->outctx = (uint8 *)MALLOC_ALIGN(xdc->osh,
			xdc->outctx_size, XDC_DCBAA_ALIGN_BITS);

		if (xdc->outctx == NULL)	return -1;

		xdc->outctx_adr = (uint32)xdc->outctx;
		bzero(xdc->outctx, xdc->outctx_size);
		W_REG(xdc->osh, (uint32 *)XDC_WAR_REG3, xdc->outctx_adr);
	}

	return 0;
}

static void xdc_free_ctx(struct xdcd *xdc)
{
	if (xdc->inctx) {
		MFREE(xdc->osh, xdc->inctx, xdc->inctx_size);
		xdc->inctx = NULL;
	}

	if (xdc->outctx) {
		MFREE(xdc->osh, xdc->outctx, xdc->outctx_size);
		xdc->outctx = NULL;
	}
}

/** returns e.g. (CTRL_EP << 3) */
int get_ep_type(unsigned int ep_type, unsigned int ep_dir)
{
	int in;
	uint32 type;

	if (ep_type > 3)
		return 0;

	in = ep_dir>>7;

	type = EPX_EP_TYPE(CTRL_EP);
	if (ep_type) {
		if (in) {
			type = EPX_EP_TYPE(CTRL_EP + ep_type);
		} else {
			type = EPX_EP_TYPE(ep_type);
		}
	}

	return type;
}

/**
 * Allocates and initializes an endpoint ring in dongle memory with one ring segment. Segments are
 * used for non-contiguous rings.
 */
int xdc_config_ep(struct xdcd *xdc, struct xdc_ep *ep)
{
	uint32 ep_index;
	struct xdc_epctx *ep_ctx;
	struct xdc_ring *xf_ring;
	uint32 max_packet = 64;

	ep_index = ep->index;
	ep_ctx = (struct xdc_epctx *)(xdc->inctx) + (ep_index + 2);

	xdc_dbg(("xdc_config_ep ep_index=%d\n", ep_index));

	ep->xf_ring = xdc_alloc_ring(xdc, TRUE, ep->ring_sz);

	if (!ep->xf_ring) {
		return -1;
	}

	xf_ring = ep->xf_ring;
	xdc_dbg(("ep=%p xf_ring=%p\n", ep, xf_ring));
	ep_ctx->deq = htol32(((uint32)xf_ring->trbs) | xf_ring->cycle);

	ep_ctx->info1 = htol32(EPX_ITV(ep->intv));
	ep_ctx->info2 |= htol32(get_ep_type(ep->type, ep->dir));

	xdc_dbg(("xdc->speed=%8x\n", xdc->speed));

	/* Set the max packet size and max burst */
	if (xdc->speed == 0) {
		xdc_err(("speed err \n"));
	}

	max_packet = ltoh16(ep->maxpacket);
	ep_ctx->info2 |= htol32(EPX_MPS(max_packet));

	if (xdc->speed == 	XDC_PORT_SS) {
		ep_ctx->info2 |= htol32(EPX_MAX_BURST(ep->maxburst));
	}
	/* for HS/FS ep->maxburst always set to be 0 */

	xdc_dbg(("ep_ctx=%p, max_packtsize=%d, ep_ctx->info2 = %x \n",
	         ep_ctx, max_packet, ep_ctx->info2));

	return 0;
} /* xdc_config_ep */

/**
 * Scratchpad area in dongle memory is used by xDC core for bookkeeping. Not accessed by firmware.
 */
int xdc_set_scratchpad(struct xdcd *xdc)
{
	int i;
	int sp_num = xdc->sp_num;

	xdc_dbg(("scratchpad buffers = %d\n", sp_num));

	if (!sp_num)
		return 0;

	xdc->sp_tbl = (uint32 *)MALLOC_ALIGN(xdc->osh,
		sp_num*sizeof(uint32), XDC_DCBAA_ALIGN_BITS);
	xdc->sp_addr = (uint32)xdc->sp_tbl;

	if (!xdc->sp_tbl)
		return 1;

	xdc->dcbaa_ptr->devctx_ptr0[0] = htol32(xdc->sp_addr);
	xdc->dcbaa_ptr->devctx_ptr0[1] = 0;

	for (i = 0; i < sp_num; i++) {
		uint32 addr;
		void *buf = MALLOC(xdc->osh, xdc->page_size);
		addr = (uint32)buf;

		if (!buf)
			return 2;

		xdc->sp_tbl[i] = addr;
	}

	return 0;
} /* xdc_set_scratchpad */

static void xdc_free_scratchpad(struct xdcd *xdc)
{
	int sp_num;
	int i;

	sp_num = xdc->sp_num;

	if (xdc->sp_addr == 0)
		return;

	for (i = 0; i < sp_num; i++) {
		MFREE(xdc->osh,  (void *)xdc->sp_tbl[i], xdc->page_size);
	}

	MFREE(xdc->osh, xdc->sp_tbl, sp_num * sizeof(uint32));
	xdc->sp_tbl = NULL;
	xdc->sp_addr = 0;
}

/** frees multiple end point rings allocated in dongle memory */
static void xdc_free_eps(struct xdcd *xdc)
{
	uint32 ep_index;
	struct xdc_ep *ep;

	for (ep_index = 0; ep_index < EP_MAX_IDX; ep_index++) {
		ep = &xdc->ep[ep_index];
		if (ep->xf_ring) {
			xdc_free_ring(xdc, ep->xf_ring, ep->ring_sz);
			ep->xf_ring = NULL;
		}
	}
}

/** frees all xDC related data structures allocated in dongle memory */
void xdc_free_mem(struct xdcd *xdc)
{
	int size;

	if (xdc->irset) {
		W_REG(xdc->osh, &xdc->irset->erst_sz, 0);
		xdc_W_REG64(xdc, 0, 0, &xdc->irset->erst_base64[0]);
		xdc_W_REG64(xdc, 0, 0, &xdc->irset->erst_deq64[0]);
	}
	xdc_dbg(("Free MEM\n"));

	size = sizeof(struct xdc_erst) * (xdc->num_er);
	if (xdc->er) MFREE(xdc->osh, xdc->er, size);

	xdc->er = NULL;

	if (xdc->event_ring)
		xdc_free_ring(xdc, xdc->event_ring, xdc->trbs_per_evtring);
	xdc->event_ring = NULL;


	xdc_W_REG64(xdc, 0, 0, &xdc->opregs->cmd_ring[0]);
	if (xdc->cmd_ring)
		xdc_free_ring(xdc, xdc->cmd_ring, xdc->trbs_per_cmdring);

	xdc->cmd_ring = NULL;

	xdc_free_eps(xdc);

	xdc_free_ctx(xdc);


	xdc_W_REG64(xdc, 0, 0, &xdc->opregs->dcbaa[0]);
	if (xdc->dcbaa_ptr)
		MFREE(xdc->osh, xdc->dcbaa_ptr->mem, (sizeof(*xdc->dcbaa_ptr)));
	xdc->dcbaa_ptr = NULL;

	xdc_free_scratchpad(xdc);

	if (xdc->setup_buf)
		MFREE(xdc->osh, xdc->setup_buf, 512);
	if (xdc->stppkt_lbuf)
		PKTFREE(xdc->osh, xdc->stppkt_lbuf, FALSE);

	xdc->page_size = 0;
} /* xdc_free_mem */

#endif /* USB_XDCI */
