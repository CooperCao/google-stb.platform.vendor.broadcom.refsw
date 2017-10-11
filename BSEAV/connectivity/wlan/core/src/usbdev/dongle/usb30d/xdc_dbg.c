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
 * $Id: xdc_dbg.c 387932 2013-02-27 18:15:42Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <bcmstdlib.h>
#include <usbdev_dbg.h>
#include <usbstd.h>
#include "xdc.h"

#ifdef USB_XDCI
#ifdef XDC_DEBUG
static void xdc_dump_cap_regs(struct xdcd *xdc)
{
	uint32 temp;

	xdc_dbg(("xDC capability registers at %p:\n", xdc->cpregs));

	temp = R_REG(xdc->osh, &xdc->cpregs->cpliver);
	xdc_dbg(("cap length and xdc version 0x%x:\n",	(unsigned int) temp));

	temp = R_REG(xdc->osh, &xdc->cpregs->sparams1);
	xdc_dbg(("xdc SPARAMS 1: 0x%x\n", (unsigned int) temp));

	temp = R_REG(xdc->osh, &xdc->cpregs->sparams2);
	xdc_dbg(("xdc SPARAMS 2: 0x%x\n", (unsigned int) temp));

	temp = R_REG(xdc->osh, &xdc->cpregs->sparams3);
	xdc_dbg(("xdc SPARAMS 3 0x%x:\n", (unsigned int) temp));

	temp = R_REG(xdc->osh, &xdc->cpregs->cparams);
	xdc_dbg(("xdc PARAMS 0x%x:\n", (unsigned int) temp));
	temp = R_REG(xdc->osh, &xdc->cpregs->rtreg_off);
	xdc_dbg(("runtime reg offset 0x%x:\n", temp & RT_REGOFF_MASK));
}

void xdc_dump_port(struct xdcd *xdc)
{
	uint32 temp;
	struct xdc_opregs *op_reg;

	op_reg = xdc->opregs;

	temp = R_REG(xdc->osh, &op_reg->portsc);
	xdc_dbg(("PORTSC 0x%x: ", temp));
	temp = R_REG(xdc->osh, &op_reg->portpw);
	xdc_dbg(("PORTPM 0x%x: ", temp));
	temp = R_REG(xdc->osh, &op_reg->portlk);
	xdc_dbg(("PORTLI 0x%x: ", temp));
	temp = R_REG(xdc->osh, &op_reg->portdbg);
	xdc_dbg(("PORTTD 0x%x:\n", temp));
}

static void xdc_dump_opregs(struct xdcd *xdc)
{
	uint32 temp;
	xdc_dbg(("xDC operational registers at %p:\n",	xdc->opregs));

	temp = R_REG(xdc->osh, &xdc->opregs->command);
	xdc_dbg(("USBCMD 0x%x:\n", temp));
	temp = R_REG(xdc->osh, &xdc->opregs->status);
	xdc_dbg(("USBSTS 0x%x:\n", temp));
	xdc_dump_port(xdc);
}

void xdc_dump_irset(struct xdcd *xdc, int set_num)
{
	struct xdc_irreg *irset =  &xdc->rtregs->irset[set_num];
	void *addr;
	uint32 temp;
	uint32 temp_64[2];

	addr = &irset->irq_pend;

	temp = R_REG(xdc->osh, addr);
	if (temp == 0)
		return;

	xdc_dbg((" %p: irset[%i]\n", irset, set_num));

	xdc_dbg((" %p: irset.pending = 0x%x\n", addr,	(unsigned int) temp));

	addr = &irset->irq_ctrl;
	temp = R_REG(xdc->osh, addr);
	xdc_dbg((" %p: irset.control = 0x%x\n", addr,	(unsigned int) temp));

	addr = &irset->erst_sz;
	temp = R_REG(xdc->osh, addr);
	xdc_dbg(("%p: irset.erst_sz = 0x%x\n", addr, (unsigned int) temp));

	addr = &irset->rsv1;
	temp = R_REG(xdc->osh, addr);
	if (temp != 0)
		xdc_dbg((" WARN: %p: irset.rsv = 0x%x\n", addr, (unsigned int) temp));

	addr = &irset->erst_base64[0];
	xdc_R_REG64(&temp_64[0], xdc, addr);
	xdc_dbg((" %p: irset.erst_base = @%08x,%08x\n",
	          addr, temp_64[0], temp_64[1]));

	addr = &irset->erst_deq64[0];
	xdc_R_REG64(&temp_64[0], xdc, addr);
	xdc_dbg(("%p: irset.erst_dequeue = @%08x\n", addr, temp_64[0]));
}

void xdc_dump_run_regs(struct xdcd *xdc)
{
	uint32 temp;

	xdc_dbg(("xDC runtime registers at %p:\n", xdc->rtregs));
	temp = R_REG(xdc->osh, &xdc->rtregs->mframe_index);
	xdc_dbg(("  %p: Microframe index = 0x%x\n",
	          &xdc->rtregs->mframe_index, (unsigned int) temp));
}

void xdc_dump_registers(struct xdcd *xdc)
{
	xdc_dump_cap_regs(xdc);
	xdc_dump_opregs(xdc);
	/* xdc_dump_port(xdc); */
}

void xdc_dump_trb(struct xdcd *xdc, struct xdc_trb *trb)
{
	uint32 address;
	uint32 type = ltoh32(trb->flags) & TRB_TYPE_BITMASK;

	switch (type) {
	case TRB_TYPE(LINK_TRB):
		xdc_dbg(("Link TRB:\n"));
		address = ltoh32(trb->buf_addr);
		xdc_dbg(("Next ring segment DMA address = 0x%8x\n",	address));

		xdc_dbg(("Interrupter target = 0x%x\n",
			GET_ITGT(ltoh32(trb->status))));
		xdc_dbg(("Cycle bit = %x\n", ltoh32(trb->flags) & TRB_CYCLE));
		xdc_dbg(("Toggle cycle bit = %x\n",	ltoh32(trb->flags) & LINK_CYCLE));
		xdc_dbg(("No Snoop bit = %x\n",	ltoh32(trb->flags) & TRB_NO_SNOOP));
		break;
	case TRB_TYPE(XFER_EVT_TRB):
		address = ltoh32(trb->buf_addr);
		xdc_dbg(("TRB address or buffer contents= %x\n", address));
		break;
	case TRB_TYPE(CMD_COMP_TRB):
		address = ltoh32(trb->buf_addr);
		xdc_dbg(("Command TRB pointer = %8x\n",	address));
		xdc_dbg(("Completion status = %u\n",
		           GET_CC_CODE(ltoh32(trb->status))));
		xdc_dbg(("Flags = 0x%x\n", ltoh32(trb->flags)));
		break;
	default:
		xdc_dbg(("Unknown TRB with TRB type ID %u\n", (unsigned int) type >> 10));
		break;
	}
}

void xdc_dump_segment(struct xdcd *xdc, struct xdc_trb *trbs, uint32 ring_sz)
{
	int i;
	uint32 addr = (uint32)trbs;
	struct xdc_trb *trb = trbs;

	for (i = 0; i < ring_sz; ++i) {
		trb = &trbs[i];
		xdc_dbg(("trb: %08x %08x %08x %08x %08x\n", addr,
			ltoh32(trb->buf_addr),
			0,
			ltoh32(trb->status),
			ltoh32(trb->flags)));
		addr += sizeof(*trb);
	}
}

void xdc_dump_ring_ptrs(struct xdcd *xdc, struct xdc_ring *ring)
{
	xdc_dbg(("Ring deq = %p, 0x%8x \n", ring->dequeue,
		(unsigned int)&ring->dequeue->buf_addr));

	xdc_dbg(("Ring enq = %p, 0x%8x \n",
	        ring->enqueue, (unsigned int)&ring->dequeue->buf_addr));
}

void xdc_dump_ring(struct xdcd *xdc, struct xdc_ring *ring, uint32 ring_sz)
{
	xdc_dump_segment(xdc, ring->trbs, ring_sz);
}

void xdc_dump_ep_rings(struct xdcd *xdc, unsigned int ep_index)
{
	struct xdc_ep *ep;
	struct xdc_ring *ring;

	ep = &xdc->ep[ep_index];
	ring = ep->xf_ring;
	if (!ring)
		return;
	printf("endpoint %d xf_ring %p \n", ep_index, ring);
	xdc_dump_segment(xdc, ring->trbs, ep->ring_sz);
}

void xdc_dump_erst(struct xdcd *xdc)
{
	uint32 addr = xdc->erst_addr;
	int i;
	struct xdc_erst *er;

	xdc_dbg(("xdc_dump_erst \n"));
	for (i = 0; i < xdc->num_er; ++i) {
		er = &xdc->er[i];
		xdc_dbg(("@%08x %08x %08x %08x %08x\n",
			addr,
			(ltoh32(er->seg_addr64[0])),
			(ltoh32(er->seg_addr64[1])),
			ltoh32(er->seg_sz),
			ltoh32(er->rsv)));
		addr += sizeof(*er);
	}
}

void xdc_dump_cmd_ptrs(struct xdcd *xdc)
{
	uint32 val64[2];

	xdc_R_REG64(&val64[0], xdc, &xdc->opregs->cmd_ring[0]);
	xdc_dbg(("xDC command ring deq ptr low bits + flags = @%08x\n",
		val64[0]));
	xdc_dbg(("xDC command ring deq ptr high bits = @%08x\n",
		val64[1]));
}

static void xdc_dump_slx(struct xdcd *xdc)
{
	struct xdc_slx *in_ctx;
	struct xdc_slx *out_ctx;

	out_ctx = (struct xdc_slx *)xdc->outctx;
	in_ctx = (struct xdc_slx *)(xdc->inctx) + 1;

	printf("Out Ctx: @%p ", &out_ctx->info);
	printf("dev_info1 = %08x ", out_ctx->info);
	printf("dev_info2 = %08x ", out_ctx->itgt);
	printf("dev_state = %08x\n", out_ctx->dev_state);

	printf("In Ctx: @%p ", &in_ctx->info);
	printf("dev_info1 = %08x ", in_ctx->info);
	printf("dev_info2 = %08x ", in_ctx->itgt);
	printf("dev_state = %08x\n", in_ctx->dev_state);
}

static void xdc_dump_ep_ctx(struct xdcd *xdc, uint8 *ctx,
	unsigned int start_ep, unsigned int last_ep)
{
	struct xdc_epctx *ep_ctx;
	int i, last_ctx;
	int adj = 1;

	if (ctx == xdc->inctx) {
		adj = 2;
	}

	last_ctx = last_ep + ((last_ep < 31) ? 1:0);

	for (i = start_ep; i < last_ctx; ++i) {
		ep_ctx = (struct xdc_epctx *)(ctx) + (i + adj);

		printf("ep_index = %d ", i);
		printf("ep_ctx=%p ",	&ep_ctx->info1);
		printf("info1 = %x ", ep_ctx->info1);
		printf("info2 = %x ", ep_ctx->info2);
		printf("deq = %x \n", ep_ctx->deq);
	}
}

void xdc_dump_ctx(struct xdcd *xdc, uint32 type,
	unsigned int start_ep, unsigned int last_ep)
{
	uint8 *ctx;

	if (type == XDC_INPUT_CTX) {
		struct xdc_ctx_icc *icc;

		printf("Input CTX \n");
		icc = (struct xdc_ctx_icc *)xdc->inctx;
		printf("ICC=@%p input_ctrl_ctx: { drop flags = %#08x ",
			icc, icc->drop_ctx_flag);
		printf("add flags = %#08x\n }",	icc->add_ctx_flag);
		ctx = xdc->inctx;
	}
	else {
		printf("Output CTX \n");
		ctx = xdc->outctx;
	}

	xdc_dump_slx(xdc);
	xdc_dump_ep_ctx(xdc, ctx, start_ep, last_ep);
}

void xdc_dump_rings(uint32 arg, uint argc, char *argv[])
{
	struct usbdev_chip *uxdc;
	struct xdcd *xdc;
	uint32 ep_index = 0;
	uint32 ring = 0;

	uxdc = (struct usbdev_chip *)arg;

	if (uxdc == NULL)
		return;

	xdc = uxdc->xdc;

	if (argc > 1) {
		ring = atoi(argv[1]);
		if (argc > 2)
			ep_index = atoi(argv[2]);
	}

	if (ring == 0) {
		printf("dump command ring\n");
		xdc_dump_ring(xdc, xdc->cmd_ring, xdc->trbs_per_cmdring);
	}

	if (ring == 1) {
		printf("dump event ring\n");
		xdc_dump_ring(xdc, xdc->event_ring, xdc->trbs_per_evtring);
	}

	if (ring == 2) {
		printf("dump EP->xf_ring \n");
		xdc_dump_ep_rings(xdc, ep_index);
	}
}
void xdc_dump_pointer(uint32 arg, uint argc, char *argv[])
{
	struct usbdev_chip *uxdc;
	struct xdcd *xdc;

	uxdc = (struct usbdev_chip *)arg;

	if (uxdc == NULL)
		return;

	xdc = uxdc->xdc;

	printf("uxdc = %p \n", uxdc);
	printf("uxdc->drv = %p\n", uxdc->drv);
	printf("uxdc->osh = %p\n", uxdc->osh);
	printf("uxdc->bus = %p\n", uxdc->bus);
	printf("uxdc->ish = %p\n", uxdc->sih);

	printf("xdc = %p\n", uxdc->xdc);
	printf("xdc->cpregs = %p\n", xdc->cpregs);
	printf("xdc->opregs = %p\n", xdc->opregs);
	printf("xdc->rtregs = %p\n", xdc->rtregs);
	printf("xdc->ir_regs = %p\n", xdc->irset);
	printf("uxdc->shim_regs = %p\n", uxdc->regs);

	printf("xdc->dba = %p\n", xdc->dba);
	printf("xdc->dcbaa_ptr = %p\n", xdc->dcbaa_ptr);
	printf("xdc->cmd_ring = %p\n", xdc->cmd_ring);
	printf("xdc->event_ring = %p\n", xdc->event_ring);

	printf("xdc->erst = %x\n", xdc->erst_addr);
	printf("xdc->inctx = %p ", xdc->inctx);
	printf("input ctx = %x\n", xdc->inctx_adr);
	printf("xdc->out_ctx = %p ", xdc->outctx);
	printf("output ctx = %x\n", xdc->outctx_adr);

	printf("ep address = %p\n", &xdc->ep[0]);
}

void xdc_dump_ctxs(uint32 arg, uint argc, char *argv[])
{
	struct usbdev_chip *uxdc;
	struct xdcd *xdc;
	int first_ep = 0;
	int last_ep = 31;

	uxdc = (struct usbdev_chip *)arg;

	if (uxdc == NULL)
		return;

	if (argc > 1) {
		first_ep = atoi(argv[1]);
		if (argc > 2)
			last_ep = atoi(argv[2]);
	}

	xdc = uxdc->xdc;

	xdc_dump_ctx(xdc, XDC_INPUT_CTX, first_ep, last_ep);
	xdc_dump_ctx(xdc, XDC_DEVICE_CTX, first_ep, last_ep);
}
#endif /* XDC_DEBUG */
#endif /* USB_XDCI */
