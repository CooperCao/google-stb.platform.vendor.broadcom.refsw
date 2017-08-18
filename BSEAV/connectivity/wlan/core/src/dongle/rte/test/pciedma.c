/*
 * Test DMA transfers H2D, D2H
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

#include <osl.h>
#include "pcie_core.h"
#include "sbhnddma.h"
#include "bcmutils.h"

#include "arms_43602.h"
#include "pciedma.h"

#define	I2B(index, type)	((index) * sizeof(type))


/* descriptor bumping macros */
#define	XXD(x, n)	((x) & ((n) - 1))	/* faster than %, but n must be power of 2 */
#define	TXD(x)		XXD((x), di->ntxd)
#define	RXD(x)		XXD((x), di->nrxd)
#define	NEXTTXD(i)	TXD((i) + 1)
#define	PREVTXD(i)	TXD((i) - 1)
#define	NEXTRXD(i)	RXD((i) + 1)
#define	PREVRXD(i)	RXD((i) - 1)

#define BYTE_ALIGN_LEN(len)  ((len >> 2) | 1)
#define WORD_ALIGN_LEN(len)  ((len >> 2) | 2)

typedef struct {
	pciedma_t 	di;
	dma64addr_t src;
	dma64addr_t dst;
	uint32		srclen;
	uint32		dstlen;
	uint32		srcnd;
	uint32		dstnd;
	uint32		bytes_count;
	uint32		tcm_stress;
	uint32		dma_alignment;
	volatile arm_stat_t 	*stat;
} dma_context_t;
static dma_context_t dma_h2d_ctx;
static dma_context_t dma_d2h_ctx;

static dma_context_t *pciedma_context_set(dma_params_t *dma_params,
	sbpcieregs_t *pcieregs, volatile arm_stat_t *stat);
static void pciedma_context_transfer(dma_context_t *dma_ctx);

/* DMA interrupts handler */

void pciedma_isr_handler(sbpcieregs_t *pcieregs)
{
	/* H2D */
	if (ACCESS(&pcieregs->u.pcie2.h2d_intstat_0) & PCIEGEN2_DE_MASK) {
		dma_h2d_ctx.stat->h2d_de_count ++;
	}
	if (ACCESS(&pcieregs->u.pcie2.h2d_intstat_0) & PCIEGEN2_DA_MASK) {
		dma_h2d_ctx.stat->h2d_da_count ++;
	}
	if (ACCESS(&pcieregs->u.pcie2.h2d_intstat_0) & PCIEGEN2_DP_MASK) {
		dma_h2d_ctx.stat->h2d_dp_count ++;
	}
	if (ACCESS(&pcieregs->u.pcie2.h2d_intstat_0) & PCIEGEN2_RU_MASK) {
		dma_h2d_ctx.stat->h2d_ru_count ++;
	}
	if (ACCESS(&pcieregs->u.pcie2.h2d_intstat_0) & PCIEGEN2_RI_MASK) {
		ACCESS(&pcieregs->u.pcie2.h2d_intstat_0) = PCIEGEN2_RI_MASK;
		dma_h2d_ctx.stat->h2d_ri_count ++;
	}
	if (ACCESS(&pcieregs->u.pcie2.h2d_intstat_0) & PCIEGEN2_XI_MASK) {
		ACCESS(&pcieregs->u.pcie2.h2d_intstat_0) = PCIEGEN2_XI_MASK;
		if (dma_h2d_ctx.dma_alignment) {
			dma_h2d_ctx.src.loaddr += dma_h2d_ctx.bytes_count;
			dma_h2d_ctx.dst.loaddr += dma_h2d_ctx.bytes_count;

			if (dma_h2d_ctx.bytes_count == WORD_ALIGN_LEN(dma_h2d_ctx.srclen)) {
				/* set next transfer to byte aligned length */
				dma_h2d_ctx.bytes_count = BYTE_ALIGN_LEN(dma_h2d_ctx.srclen);
				pciedma_h2d_transfer(&dma_h2d_ctx.di,
					dma_h2d_ctx.src, dma_h2d_ctx.dst,
					dma_h2d_ctx.bytes_count, dma_h2d_ctx.bytes_count,
					dma_h2d_ctx.srcnd, dma_h2d_ctx.dstnd);
			}
			else if (dma_h2d_ctx.bytes_count == BYTE_ALIGN_LEN(dma_h2d_ctx.srclen)) {
				/* remaining bytes */
				dma_h2d_ctx.bytes_count = dma_h2d_ctx.srclen -
					(WORD_ALIGN_LEN(dma_h2d_ctx.srclen) +
					BYTE_ALIGN_LEN(dma_h2d_ctx.srclen));
				pciedma_h2d_transfer(&dma_h2d_ctx.di,
					dma_h2d_ctx.src, dma_h2d_ctx.dst,
					dma_h2d_ctx.bytes_count, dma_h2d_ctx.bytes_count,
					dma_h2d_ctx.srcnd, dma_h2d_ctx.dstnd);
			}

		}
		dma_h2d_ctx.stat->h2d_xi_count ++;
	}
	/* D2H */
	if (ACCESS(&pcieregs->u.pcie2.d2h_intstat_0) & PCIEGEN2_DE_MASK) {
		dma_d2h_ctx.stat->d2h_de_count ++;
	}
	if (ACCESS(&pcieregs->u.pcie2.d2h_intstat_0) & PCIEGEN2_DA_MASK) {
		dma_d2h_ctx.stat->d2h_da_count ++;
	}
	if (ACCESS(&pcieregs->u.pcie2.d2h_intstat_0) & PCIEGEN2_DP_MASK) {
		dma_d2h_ctx.stat->d2h_dp_count ++;
	}
	if (ACCESS(&pcieregs->u.pcie2.d2h_intstat_0) & PCIEGEN2_RU_MASK) {
		dma_d2h_ctx.stat->d2h_ru_count ++;
	}
	if (ACCESS(&pcieregs->u.pcie2.d2h_intstat_0) & PCIEGEN2_RI_MASK) {
		ACCESS(&pcieregs->u.pcie2.d2h_intstat_0) = PCIEGEN2_RI_MASK;
		dma_d2h_ctx.stat->d2h_ri_count ++;
	}
	if (ACCESS(&pcieregs->u.pcie2.d2h_intstat_0) & PCIEGEN2_XI_MASK) {
		ACCESS(&pcieregs->u.pcie2.d2h_intstat_0) = PCIEGEN2_XI_MASK;
		if (dma_d2h_ctx.tcm_stress) {
			pciedma_d2h_transfer(&dma_d2h_ctx.di,
				dma_d2h_ctx.src, dma_d2h_ctx.dst,
				dma_d2h_ctx.srclen, dma_d2h_ctx.dstlen,
				dma_d2h_ctx.srcnd, dma_d2h_ctx.dstnd);
		}
		else if (dma_d2h_ctx.dma_alignment) {
			dma_d2h_ctx.src.loaddr += dma_d2h_ctx.bytes_count;
			dma_d2h_ctx.dst.loaddr += dma_d2h_ctx.bytes_count;

			if (dma_d2h_ctx.bytes_count == WORD_ALIGN_LEN(dma_d2h_ctx.srclen)) {
				/* set next transfer to byte aligned length */
				dma_d2h_ctx.bytes_count = BYTE_ALIGN_LEN(dma_d2h_ctx.srclen);
				pciedma_d2h_transfer(&dma_d2h_ctx.di,
					dma_d2h_ctx.src, dma_d2h_ctx.dst,
					dma_d2h_ctx.bytes_count, dma_d2h_ctx.bytes_count,
					dma_d2h_ctx.srcnd, dma_d2h_ctx.dstnd);
			}
			else if (dma_d2h_ctx.bytes_count == BYTE_ALIGN_LEN(dma_d2h_ctx.srclen)) {
				/* remaining bytes */
				dma_d2h_ctx.bytes_count = dma_d2h_ctx.srclen -
					(WORD_ALIGN_LEN(dma_d2h_ctx.srclen) +
					BYTE_ALIGN_LEN(dma_d2h_ctx.srclen));
				pciedma_d2h_transfer(&dma_d2h_ctx.di,
					dma_d2h_ctx.src, dma_d2h_ctx.dst,
					dma_d2h_ctx.bytes_count, dma_d2h_ctx.bytes_count,
					dma_d2h_ctx.srcnd, dma_d2h_ctx.srcnd);
			}
		}
		dma_d2h_ctx.stat->d2h_xi_count ++;
	}
}


void pciedma_interrupts_enable(sbpcieregs_t *pcieregs, uint32 h2d, uint32 mask)
{

	/* start dma transfer */

	if (h2d) {
		ACCESS(&pcieregs->u.pcie2.h2d_intmask_0) = mask;
	}
	else {
		ACCESS(&pcieregs->u.pcie2.d2h_intmask_0) = mask;
	}
}

static dma_context_t *pciedma_context_set(dma_params_t *dma_params,
	sbpcieregs_t *pcieregs, volatile arm_stat_t *stat) {

	dma_context_t *dma_ctx;

	if (dma_params->h2d) {
		dma_ctx = &dma_h2d_ctx;
	}
	else {
		dma_ctx = &dma_d2h_ctx;
	}
	memset(dma_ctx, 0, sizeof(dma_context_t));

	dma_ctx->stat = stat;

	stat->dma_src[0] = dma_params->src.loaddr;
	stat->dma_src[1] = dma_params->src.hiaddr;
	stat->dma_dst[0] = dma_params->dst.loaddr;
	stat->dma_dst[1] = dma_params->dst.hiaddr;

	pciedma_t *di = &dma_ctx->di;
	di->pcieregs = pcieregs;
	di->txout = 0;
	di->rxout = 0;

	di->ddringtx = dma_params->ddringtx;
	di->ddringrx = dma_params->ddringrx;
	di->ntxd = dma_params->ntxd;
	di->nrxd = dma_params->nrxd;
	di->tx_control = dma_params->tx_control;
	di->rx_control = dma_params->rx_control;

	dma_ctx->src.loaddr = dma_params->src.loaddr;
	dma_ctx->src.hiaddr = dma_params->src.hiaddr;
	dma_ctx->dst.loaddr = dma_params->dst.loaddr;
	dma_ctx->dst.hiaddr = dma_params->dst.hiaddr;
	dma_ctx->srclen = dma_params->src_len;
	dma_ctx->dstlen = dma_params->dst_len;
	dma_ctx->srcnd = dma_params->src_nd;
	dma_ctx->dstnd = dma_params->dst_nd;

	dma_ctx->bytes_count = dma_ctx->srclen;

	return dma_ctx;
}

static void pciedma_context_transfer(dma_context_t *dma_ctx)
{
	if ((uint32)dma_ctx == (uint32)&dma_h2d_ctx) {
		pciedma_h2d_init(&dma_ctx->di);
		pciedma_h2d_transfer(&dma_ctx->di, dma_ctx->src, dma_ctx->dst,
			dma_ctx->bytes_count, dma_ctx->bytes_count,
			dma_ctx->srcnd, dma_ctx->dstnd);
	}
	else {
		pciedma_d2h_init(&dma_ctx->di);
		pciedma_d2h_transfer(&dma_ctx->di, dma_ctx->src, dma_ctx->dst,
			dma_ctx->bytes_count, dma_ctx->bytes_count,
			dma_ctx->srcnd, dma_ctx->dstnd);
	}
}

void pciedma_transfer_test(dma_params_t *dma_params,
	sbpcieregs_t *pcieregs, volatile arm_stat_t *stat) {

	dma_context_t *dma_ctx;

	dma_ctx = pciedma_context_set(dma_params, pcieregs, stat);

	/* start dma transfer */
	pciedma_context_transfer(dma_ctx);
}

void pciedma_alignment_test(dma_params_t *dma_params,
	sbpcieregs_t *pcieregs, volatile arm_stat_t *stat) {

	dma_context_t *dma_ctx;

	dma_ctx = pciedma_context_set(dma_params, pcieregs, stat);

	/* set alignment test flag  */
	dma_ctx->dma_alignment = 1;

	/* start from dword length */
	dma_ctx->bytes_count = WORD_ALIGN_LEN(dma_ctx->srclen);

	/* start dma transfer */
	pciedma_context_transfer(dma_ctx);
}

void pciedma_tcm_stress_test(dma_params_t *dma_params,
	sbpcieregs_t *pcieregs, volatile arm_stat_t *stat) {

	dma_context_t *dma_ctx;

	dma_ctx = pciedma_context_set(dma_params, pcieregs, stat);

	/* set tcm stress flag */
	dma_ctx->tcm_stress = 1;

	/* start dma transfer */
	pciedma_context_transfer(dma_ctx);
}

void
pciedma_attach(
	volatile dma64regs_t *dmaregstx, volatile dma64regs_t *dmaregsrx);

void pciedma_init(volatile dma64regs_t *dmaregstx, volatile dma64regs_t *dmaregsrx,
	uint32 tx_ddaddrlow, uint32 tx_ddaddrhigh,
	uint32 rx_ddaddrlow, uint32 rx_ddaddrhigh);
bool
pciedma_txreset(volatile dma64regs_t *dmaregstx);
bool
pciedma_rxreset(volatile dma64regs_t *dmaregsrx);
void
pciedma_dd_upd_64(dma64dd_t *ddring, dma64addr_t pa, uint outidx, uint32 *flags, uint32 ctrl2,
	uint32 bufcount);
int
pciedma_rxfast(pciedma_t *di, volatile dma64regs_t *dmaregsrx, dma64dd_t *ddring,
	dma64addr_t p, uint32 len, uint32 nd);
int
pciedma_txfast(pciedma_t *di, volatile dma64regs_t *dmaregsrx, dma64dd_t *ddring,
	dma64addr_t p, uint32 len, uint32 nd);

void
pciedma_attach(
	volatile dma64regs_t *dmaregstx, volatile dma64regs_t *dmaregsrx)
{
	uint32 reg_val;

	if (dmaregsrx) {

			/* first disable the dma if not already done */
			reg_val = ACCESS(&dmaregsrx->control);
			if (reg_val & 1) {
				reg_val &= ~1;
				ACCESS(&dmaregsrx->control) = reg_val;
				ACCESS(&dmaregsrx->control) = reg_val;
			}
	}
	if (dmaregstx) {

			/* first disable the dma if not already done */
			reg_val = ACCESS(&dmaregsrx->control);
			if (reg_val & 1) {
				reg_val &= ~1;
				ACCESS(&dmaregsrx->control) = reg_val;
				ACCESS(&dmaregsrx->control) = reg_val;
			}
	}

}

void pciedma_init(volatile dma64regs_t *dmaregstx, volatile dma64regs_t *dmaregsrx,
	uint32 tx_ddaddrlow, uint32 tx_ddaddrhigh,
	uint32 rx_ddaddrlow, uint32 rx_ddaddrhigh) {

	ACCESS(&dmaregstx->addrlow) =  tx_ddaddrlow;
	ACCESS(&dmaregstx->addrhigh) =  tx_ddaddrhigh;
	ACCESS(&dmaregsrx->addrlow) =  rx_ddaddrlow;
	ACCESS(&dmaregsrx->addrhigh) =  rx_ddaddrhigh;
}

bool
pciedma_txreset(volatile dma64regs_t *dmaregstx)
{
	uint32 status;

	/* suspend tx DMA first */
	ACCESS(&dmaregstx->control) = D64_XC_SE;
	SPINWAIT(((status = (ACCESS(&dmaregstx->status0) & D64_XS0_XS_MASK)) !=
	          D64_XS0_XS_DISABLED) &&
	         (status != D64_XS0_XS_IDLE) &&
	         (status != D64_XS0_XS_STOPPED),
	         10000);

	ACCESS(&dmaregstx->control) = 0;
	SPINWAIT(((status = (ACCESS(&dmaregstx->status0) & D64_XS0_XS_MASK)) !=
	          D64_XS0_XS_DISABLED),
	         10000);

	/* We should be disabled at this point */
	return (status == D64_XS0_XS_DISABLED);
}


bool
pciedma_rxreset(volatile dma64regs_t *dmaregsrx)
{
	uint32 status;

	ACCESS(&dmaregsrx->control) =  0;
	SPINWAIT(((status = (ACCESS(&dmaregsrx->status0) & D64_RS0_RS_MASK)) !=
	          D64_RS0_RS_DISABLED), 10000);

	return (status == D64_RS0_RS_DISABLED);
}

void
pciedma_dd_upd_64(dma64dd_t *ddring, dma64addr_t pa, uint outidx, uint32 *flags, uint32 ctrl2,
        uint32 bufcount)
{
	ctrl2 |= (bufcount & D64_CTRL2_BC_MASK);

	ACCESS(&ddring[outidx].addrlow) = pa.loaddr;
	ACCESS(&ddring[outidx].addrhigh) = pa.hiaddr;
	ACCESS(&ddring[outidx].ctrl1) = *flags;
	ACCESS(&ddring[outidx].ctrl2) = ctrl2;
}

int
pciedma_rxfast(pciedma_t *di, volatile dma64regs_t *dmaregsrx, dma64dd_t *ddring,
	dma64addr_t p, uint32 len, uint32 nd)
{
	uint32 chunk;
	uint32 flags = 0;
	uint32 ctrl2 = 0;
	uint32 idx = 0;

	if ((nd == 0) || (nd > di->nrxd) || (len%nd != 0))
		return -1;

	chunk = len/nd;

	for (idx = 0; idx < nd; idx ++) {
		/* reset flags for each descriptor */
		flags = 0;
		if (di->rxout == (di->nrxd - 1))
			flags |= D64_CTRL1_EOT;

		/* Update descriptor */
		pciedma_dd_upd_64(ddring, p, di->rxout, &flags, ctrl2, chunk);
		p.loaddr += chunk;
		di->rxout = NEXTRXD(di->rxout);
	}

	/* update the chip lastdscr pointer */
	ACCESS(&dmaregsrx->ptr) = (uint32)ddring + I2B(di->rxout, dma64dd_t);
	return 0;
}

int
pciedma_txfast(pciedma_t *di, volatile dma64regs_t *dmaregstx, dma64dd_t *ddring,
	dma64addr_t p, uint32 len, uint32 nd)
{
	uint32 chunk;
	uint32 flags = 0;
	uint32 ctrl2 = 0;
	uint32 idx = 0;

	if ((nd == 0) || (nd > di->ntxd) || (len%nd != 0))
		return -1;

	chunk = len/nd;

	for (idx = 0; idx < nd; idx ++) {
		/* reset flags for each descriptor */
		flags = D64_CTRL1_IOC;
		if (idx == 0)
			flags |= D64_CTRL1_SOF;
		if (idx == (nd - 1))
			flags |= D64_CTRL1_EOF;
		if (di->txout == (di->nrxd - 1))
			flags |= D64_CTRL1_EOT;

		/* Update descriptor */
		pciedma_dd_upd_64(ddring, p, di->txout, &flags, ctrl2, chunk);
		p.loaddr += chunk;
		di->txout = NEXTRXD(di->txout);
	}

	/* kick the chip */
	ACCESS(&dmaregstx->ptr) = (uint32)ddring + I2B(di->txout, dma64dd_t);
	return (0);
}


void pciedma_d2h_init(pciedma_t *di)
{
	dma64regs_t *dmaregstx = &di->pcieregs->u.pcie2.d2h0_dmaregs.tx;
	dma64regs_t *dmaregsrx = &di->pcieregs->u.pcie2.d2h0_dmaregs.rx;

	memset((void*)di->ddringtx, 0, sizeof(dma64dd_t)*di->ntxd);
	memset((void*)di->ddringrx, 0, sizeof(dma64dd_t)*di->nrxd);

	pciedma_attach(dmaregstx, dmaregsrx);

	pciedma_txreset(dmaregstx);
	pciedma_rxreset(dmaregsrx);

	pciedma_init(dmaregstx, dmaregsrx, (uint32)di->ddringtx, 0, (uint32)di->ddringrx, 0);

	ACCESS(&dmaregstx->control) = di->tx_control;
	ACCESS(&dmaregsrx->control) = di->rx_control;
}

void pciedma_h2d_init(pciedma_t *di)
{

	dma64regs_t *dmaregstx = &di->pcieregs->u.pcie2.h2d0_dmaregs.tx;
	dma64regs_t *dmaregsrx = &di->pcieregs->u.pcie2.h2d0_dmaregs.rx;

	memset((void*)di->ddringtx, 0, sizeof(dma64dd_t)*di->ntxd);
	memset((void*)di->ddringrx, 0, sizeof(dma64dd_t)*di->nrxd);

	pciedma_attach(dmaregstx, dmaregsrx);

	pciedma_txreset(dmaregstx);
	pciedma_rxreset(dmaregsrx);

	pciedma_init(dmaregstx, dmaregsrx, (uint32)di->ddringtx, 0, (uint32)di->ddringrx, 0);

	ACCESS(&dmaregstx->control) = di->tx_control;
	ACCESS(&dmaregsrx->control) = di->rx_control;
}

void pciedma_d2h_transfer(pciedma_t *di, dma64addr_t src, dma64addr_t dst,
	uint32 src_len, uint32 dst_len, uint32 src_nd, uint32 dst_nd) {

	dma64regs_t *dmaregstx = &di->pcieregs->u.pcie2.d2h0_dmaregs.tx;
	dma64regs_t *dmaregsrx = &di->pcieregs->u.pcie2.d2h0_dmaregs.rx;

	pciedma_rxfast(di, dmaregsrx, di->ddringrx, dst, dst_len, dst_nd);
	pciedma_txfast(di, dmaregstx, di->ddringtx, src, src_len, src_nd);
}

void pciedma_h2d_transfer(pciedma_t *di, dma64addr_t src, dma64addr_t dst,
	uint32 src_len, uint32 dst_len, uint32 src_nd, uint32 dst_nd) {

	dma64regs_t *dmaregstx = &di->pcieregs->u.pcie2.h2d0_dmaregs.tx;
	dma64regs_t *dmaregsrx = &di->pcieregs->u.pcie2.h2d0_dmaregs.rx;

	pciedma_rxfast(di, dmaregsrx, di->ddringrx, dst, dst_len, dst_nd);
	pciedma_txfast(di, dmaregstx, di->ddringtx, src, src_len, src_nd);
}
