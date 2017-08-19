/*
 * Standalone CM3 diags for 43602 CPU subsystem to be run by
 * the 43602_armstest.tcl script.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id$
 */

#include <osl.h>
#include <sbhndcpu.h>
#include <standdiags.h>

#include "arms_43602.h"
#include "pciedma.h"
#include "bme.h"

static sbpcieregs_t *pcieregs = (sbpcieregs_t *)REGBASE_PCIEGEN2;

volatile host_cmd_t *phost_cmd = (host_cmd_t *)HOST_CM3_CMD_OFF;
volatile arm_stat_t *pcm3host_stat = (arm_stat_t *)CM3_HOST_STAT_OFF;


void c_main(uint32 ra);
void wfi_loop_start(uint32 ticks);
void swint_enable(swint_params_t *params);
void set_cr4_swint(uint32 swint);
void doorbell_enable(swint_params_t *params);
void doorbell_set(doorbell_params_t *db_params);
static void trap_handler(trap_t *tr);
void timer_isr_handler(void);
void timer_setup(uint32 ticks);
void pcie_isr_handler(void);
void dma_interrupts_enable(uint32 h2d, uint32 enable, uint32 mask);
void dma_transfer(dma_params_t *dma_params);
void dma_alignment_test(dma_params_t *dma_params);
void tcm_stress_start(stress_params_t *params);
void tcm_stress_dma_start(stress_params_t *params);
void tcm_stress_loop_start(void);
void mem_init(uint32 addr, uint32 size);

static uint32 wfi_ticks = 1000;


/* Exception Handler */
static void trap_handler(trap_t *tr)
{
	register uint32 type = tr->type;
	register uint32 intstatus = ACCESS(REGBASE_ARMCM3 + ARMCM3_NMIISR_STATUS);
	register uint32 core;

	dmb;
	pcm3host_stat->int_count[type] ++;
	dmb;

	for (core = 0; core < 32; core ++)
	{
		if (intstatus & (1 << core))
			pcm3host_stat->core_int_count[core] ++;
	}

	/* CM3 Timer interrupt */
	if (intstatus & ARMCM3_TIMER_INTR_MASK) {
		timer_isr_handler();
		ACCESS(REGBASE_ARMCM3 + ARMCM3_NMIISR_STATUS) = ARMCM3_TIMER_INTR_MASK;
	}

	/* PCIE interrupt */
	if (intstatus & ARMCM3_PCIE2_INTR_MASK) {
		pcie_isr_handler();
		ACCESS(REGBASE_ARMCM3 + ARMCM3_NMIISR_STATUS) = ARMCM3_PCIE2_INTR_MASK;
	}

	/* SW0 interrupt */
	if (intstatus & ARMCM3_SW0_INTR_MASK) {
		ACCESS(REGBASE_ARMCM3 + ARMCM3_NMIISR_STATUS) = ARMCM3_SW0_INTR_MASK;
		dmb;
		pcm3host_stat->swint_count[0] ++;
		dmb;
	}

	/* SW1 interrupt */
	if (intstatus & ARMCM3_SW1_INTR_MASK) {
		ACCESS(REGBASE_ARMCM3 + ARMCM3_NMIISR_STATUS) = ARMCM3_SW1_INTR_MASK;
		dmb;
		pcm3host_stat->swint_count[1] ++;
		dmb;
	}
}

void timer_setup(uint32 ticks)
{

	ACCESS(REGBASE_ARMCM3 + ARMCM3_INTMASK) |= ARMCM3_INTMASK_TIMER;
	ACCESS(REGBASE_ARMCM3 + ARMCM3_ISR_MASK) |= ARMCM3_TIMER_INTR_MASK;

	ACCESS(REGBASE_ARMCM3 + ARMCM3_INTTIMER) = ticks;
}

/* CM3 Timer interrupt handler */
void timer_isr_handler(void)
{

	/* clear interrupt status */
	ACCESS(REGBASE_ARMCM3 + ARMCM3_INTSTATUS) = ARMCM3_INTMASK_TIMER;

	dmb;
	pcm3host_stat->count ++;
	dmb;

	timer_setup(wfi_ticks);
}

/* PCIE interrupts handler */
void pcie_isr_handler(void)
{


	/* DOORBELL0 */
	if (ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) & PCIEGEN2_DB0_MASK) {
		ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) = PCIEGEN2_DB0_MASK;
		dmb;
		pcm3host_stat->doorbell[0] = pcieregs->u.pcie2.dbls[0].host2dev_0;
		dmb;
	}

	/* DOORBELL1 */
	if (ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) & PCIEGEN2_DB1_MASK) {
		ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) = PCIEGEN2_DB1_MASK;
		dmb;
		pcm3host_stat->doorbell[1] = pcieregs->u.pcie2.dbls[0].host2dev_1;
		dmb;
	}

	/* DMA */
	if (ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) & PCIEGEN2_DMA_MASK) {
		pciedma_isr_handler(pcieregs);
		ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) = PCIEGEN2_DMA_MASK;
	}
}

/* WFI LOOP */
void wfi_loop_start(uint32 ticks)
{

	wfi_ticks = ticks;
	timer_setup(ticks);

	dmb;
	pcm3host_stat->status = CM3_WFI_LOOP_STARTED;
	dmb;

	/* do nothing till an interrupt is generated. */
	while (1)
	{
	__asm__ __volatile__("wfi");
	}

}


/* CM3 SW interrupts */
void swint_enable(swint_params_t *params)
{

	if (params->is_nmi) {
		ACCESS(REGBASE_ARMCM3 + ARMCM3_NMI_MASK) |=
			(ARMCM3_SW0_INTR_MASK << params->swint);
		disable_nvic_ints(ARMCM3_INT(ARMCM3_SHARED_INT));
	}
	else {
		ACCESS(REGBASE_ARMCM3 + ARMCM3_ISR_MASK) |=
			(ARMCM3_SW0_INTR_MASK << params->swint);
	}

}

void set_cr4_swint(uint32 swint)
{

	ACCESS(REGBASE_ARMCR4 + ARMCR4_SWINT_REG) = (1 << swint);
}

/* Doorbels */
void doorbell_enable(swint_params_t *params)
{
	/* first mask both */
	ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTMASK2) &=
		~(PCIEGEN2_DB0_MASK | PCIEGEN2_DB1_MASK);

	if (params->is_nmi) {
		ACCESS(REGBASE_ARMCM3 + ARMCM3_NMI_MASK) |= ARMCM3_PCIE2_INTR_MASK;
		ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTMASK2) |=
			(PCIEGEN2_DB0_MASK << params->swint);
		disable_nvic_ints(ARMCM3_INT(ARMCM3_SHARED_INT));
	}
	else {
		ACCESS(REGBASE_ARMCM3 + ARMCM3_ISR_MASK) |= ARMCM3_PCIE2_INTR_MASK;
		ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTMASK2) |=
			(PCIEGEN2_DB0_MASK << params->swint);
	}
}

void doorbell_set(doorbell_params_t *db_params)
{

	ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_DEV2HOST_DB0 + (db_params->db << 2)) = db_params->val;
}

/* DMA */
void dma_interrupts_enable(uint32 h2d, uint32 enable, uint32 mask)
{

	pciedma_interrupts_enable(pcieregs, h2d, mask);

	ACCESS(REGBASE_ARMCM3 + ARMCM3_ISR_MASK) |= ARMCM3_PCIE2_INTR_MASK;
	ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTMASK2) |= PCIEGEN2_DMA_MASK;

	/* Be sure CR4 DMA interrupts disabled */
	ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTMASK) &= ~PCIEGEN2_DMA_MASK;
}

void dma_transfer(dma_params_t *dma_params)
{

	dma_interrupts_enable(dma_params->h2d, 1, dma_params->intmask);

	pciedma_transfer_test(dma_params, pcieregs, pcm3host_stat);
}

void dma_alignment_test(dma_params_t *dma_params)
{

	dma_interrupts_enable(dma_params->h2d, 1, dma_params->intmask);

	pciedma_alignment_test(dma_params, pcieregs, pcm3host_stat);
}


/* TCM Stress */
#define DDRINGTX_ADDR	(MEMBASE_SOCSRAM + 0x4000)
#define DDRINGRX_ADDR	(MEMBASE_SOCSRAM + 0x4100)
#define BME0_SRAM_ADDR	(MEMBASE_SOCSRAM + 0x6000)
#define BME1_SRAM_ADDR	(MEMBASE_SOCSRAM + 0x7000)

#define TCM_DMA_ADDR	(MEMBASE_BTCM_RAM + 0x2000)
#define TCM_CM3_ADDR	(MEMBASE_BTCM_RAM + 0x3000)
#define TCM_BME0_ADDR	(MEMBASE_BTCM_RAM + 0x6000)
#define TCM_BME1_ADDR	(MEMBASE_BTCM_RAM + 0x7000)

#define LAST_WORD(addr, size)	ACCESS((addr) + (size) - 4)

void tcm_stress_dma_start(stress_params_t *params)
{

	dma_params_t dma_params;

	dma_interrupts_enable(0, 1, 0x01013c00);

	dma_params.h2d = 0;
	dma_params.ddringtx = (dma64dd_t *)DDRINGTX_ADDR;
	dma_params.ddringrx = (dma64dd_t *)DDRINGRX_ADDR;
	dma_params.ntxd = 8;
	dma_params.nrxd = 8;
	dma_params.tx_control = 0x00280801;
	dma_params.rx_control = 0x002c0801;
	dma_params.src.loaddr = TCM_DMA_ADDR;
	dma_params.src.hiaddr = 0;
	dma_params.dst.loaddr = params->haddr.loaddr;
	dma_params.dst.hiaddr = params->haddr.hiaddr;
	dma_params.src_len = 512;
	dma_params.dst_len = 512;
	dma_params.src_nd = 2;
	dma_params.dst_nd = 2;

	pciedma_tcm_stress_test(&dma_params, pcieregs, pcm3host_stat);
}

void mem_init(uint32 addr, uint32 size)
{
	uint32 i = 0;

	for (i = 0; i < size; i ++) {
		*((uint8 volatile *)addr++) = i;
	}
}

void tcm_stress_loop_start(void)
{
	uint32 count = 0;
	uint32 bme_status = 0;
	uint32 bme0_copy = 1;
	uint32 bme1_copy = 1;
	uint32 tcm_addr;

	mem_init(BME0_SRAM_ADDR, 2048);
	mem_init(BME1_SRAM_ADDR, 2048);

	/* setup BME transfer for BME0 and BME1 */
	BME_TRANSFER_REQ0(BME0_SRAM_ADDR, TCM_BME0_ADDR, 2048);
	BME_TRANSFER_REQ1(BME1_SRAM_ADDR, TCM_BME1_ADDR, 2048);

	while (1) {
		tcm_addr = TCM_CM3_ADDR + (count & 0x3ff);
		dmb;
		ACCESS(tcm_addr) = count;
		dmb;
		if (ACCESS(tcm_addr) != count) {
			pcm3host_stat->status = 0xffffffff;
			break;
		}
		dmb;
		count ++;

		dmb;
		pcm3host_stat->count = count;
		dmb;

		bme_status = ACCESS(BME_STATUS);

		if (bme_status & (BME_RDERR0 | BME_RDERR1 | BME_WRERR0 | BME_WRERR1)) {
			pcm3host_stat->status = bme_status;
			break;
		}

		if (!(bme_status & BME_BUSY0)) {
			if (bme0_copy) {
				if (LAST_WORD(BME0_SRAM_ADDR, 2048) !=
					LAST_WORD(TCM_BME0_ADDR, 2048)) {
					pcm3host_stat->status = 0x0000ffff;
					break;
				}
				BME_ZEROSET_REQ0(TCM_BME0_ADDR, 2048);
				bme0_copy = 0;
			}
			else {
				if (0 != LAST_WORD(TCM_BME0_ADDR, 2048)) {
					pcm3host_stat->status = 0x0000eeee;
					break;
				}
				BME_TRANSFER_REQ0(BME0_SRAM_ADDR, TCM_BME0_ADDR, 2048);
				bme0_copy = 1;
			}
			pcm3host_stat->bme_cmplt_cnt[0] = count;
		}

		if (!(bme_status & BME_BUSY1)) {
			if (bme1_copy) {
				if (LAST_WORD(BME1_SRAM_ADDR, 2048) !=
					LAST_WORD(TCM_BME1_ADDR, 2048)) {
					pcm3host_stat->status = 0x1111ffff;
					break;
				}
				BME_ZEROSET_REQ1(TCM_BME1_ADDR, 2048);
				bme1_copy = 0;
			}
			else {
				if (0 != LAST_WORD(TCM_BME1_ADDR, 2048)) {
					pcm3host_stat->status = 0x1111eeee;
					break;
				}
				BME_TRANSFER_REQ1(BME1_SRAM_ADDR, TCM_BME1_ADDR, 2048);
				bme1_copy = 1;
			}
			pcm3host_stat->bme_cmplt_cnt[1] = count;
		}
	}
}

void tcm_stress_start(stress_params_t *params)
{
	/* 1. Start PCIE DMA TCM->DDR */
	tcm_stress_dma_start(params);

	/* 2. Start CM3<->TCM/BME SOCSRAM->TCM loop */
	tcm_stress_loop_start();
}

/* Get cmd from host and run the test */
void c_main(uint32 ra)
{

	swint_params_t *swint_params = NULL;
	dma_params_t *dma_params = NULL;
	bme_params_t *bme_params = NULL;
	doorbell_params_t *db_params = NULL;
	stress_params_t *stress_params = NULL;
	uint32 ticks = 0;

	dmb;
	memset((void*)pcm3host_stat, 0, sizeof(arm_stat_t));
	dmb;

	dmb;
	pcm3host_stat->status = CM3_MAIN_ENTERED;
	dmb;

	hnd_set_trap(trap_handler);
	enable_nvic_ints(ARMCM3_INT(ARMCM3_SHARED_INT));
	enable_arm_irq();

	/* check host cmd */
	switch (phost_cmd->cmd) {
		default:
		case HOST_CMD_START_WFI:
			ticks = phost_cmd->params[0];
			if (ticks)
				wfi_loop_start(ticks);
			else
				wfi_loop_start(wfi_ticks);
			break;
		case HOST_CMD_SWINT_ENABLE:
			swint_params = (swint_params_t *)phost_cmd->params;
			swint_enable(swint_params);
			wfi_loop_start(1000);
			break;
		case HOST_CMD_SET_SWINT:
			set_cr4_swint(phost_cmd->params[0]);
			break;
		case HOST_CMD_DOORBELL_ENABLE:
			swint_params = (swint_params_t *)phost_cmd->params;
			doorbell_enable(swint_params);
			wfi_loop_start(1000);
			break;
		case HOST_CMD_DOORBELL_SET:
			db_params = (doorbell_params_t *)phost_cmd->params;
			doorbell_set(db_params);
			wfi_loop_start(1000);
			break;
		case HOST_CMD_DMA_TRANSFER:
			dma_params = (dma_params_t *)phost_cmd->params;
			dma_transfer(dma_params);
			wfi_loop_start(1000);
			break;
		case HOST_CMD_DMA_ALIGNMENT:
			dma_params = (dma_params_t *)phost_cmd->params;
			dma_alignment_test(dma_params);
			wfi_loop_start(1000);
			break;
		case HOST_CMD_BME_TRANSFER:
			bme_params = (bme_params_t *)phost_cmd->params;
			bme_transfer(bme_params, pcm3host_stat);
			wfi_loop_start(1000);
			break;
		case HOST_CMD_TCM_STRESS:
			stress_params = (stress_params_t *)phost_cmd->params;
			tcm_stress_start(stress_params);
			break;
	}
}
