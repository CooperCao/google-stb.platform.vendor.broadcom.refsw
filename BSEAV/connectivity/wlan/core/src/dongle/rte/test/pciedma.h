/*
 * PCIe DMA definitions
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

#ifndef _PCIEDMA_H
#define _PCIEDMA_H

typedef struct {
	sbpcieregs_t 	*pcieregs;
	dma64dd_t		*ddringtx;
	dma64dd_t		*ddringrx;
	uint32			ntxd;
	uint32			nrxd;
	uint32			tx_control;
	uint32			rx_control;
	uint32			txout;
	uint32			rxout;
} pciedma_t;

void pciedma_d2h_init(pciedma_t *di);
void pciedma_h2d_init(pciedma_t *di);
void pciedma_d2h_transfer(pciedma_t *di, dma64addr_t src, dma64addr_t dst,
	uint32 src_len, uint32 dst_len, uint32 src_nd, uint32 dst_nd);
void pciedma_h2d_transfer(pciedma_t *di, dma64addr_t src, dma64addr_t dst,
	uint32 src_len, uint32 dst_len, uint32 src_nd, uint32 dst_nd);

void pciedma_isr_handler(sbpcieregs_t *pcieregs);
void pciedma_interrupts_enable(sbpcieregs_t *pcieregs, uint32 h2d, uint32 mask);

void pciedma_transfer_test(dma_params_t *dma_params,
	sbpcieregs_t *pcieregs, volatile arm_stat_t *stat);
void pciedma_alignment_test(dma_params_t *dma_params,
	sbpcieregs_t *pcieregs, volatile arm_stat_t *stat);
void pciedma_tcm_stress_test(dma_params_t *dma_params,
	sbpcieregs_t *pcieregs, volatile arm_stat_t *stat);

#endif /*  _PCIEDMA_H */
