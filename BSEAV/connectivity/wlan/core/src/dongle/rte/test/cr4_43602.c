/*
 * Standalone CR4 diags for 43602 CPU subsystem to be run by
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
#include <sbchipc.h>

#include "arms_43602.h"
#include "pciedma.h"

#define PMU_FLAGS_SHIFT	14

static sbpcieregs_t *pcieregs = (sbpcieregs_t *)REGBASE_PCIEGEN2;

volatile host_cmd_t *phost_cmd = (host_cmd_t *)HOST_CR4_CMD_OFF;
volatile arm_stat_t *pcr4host_stat = (arm_stat_t *)CR4_HOST_STAT_OFF;

volatile chipcregs_t *ccr = (chipcregs_t *)REGBASE_CHIPC;

void c_main(uint32 ra);
void wfi_loop_start(uint32 ticks);
void swint_enable(swint_params_t *params);
void tcm_stress_start(void);
void clock_switch(uint32 n);
void clock_stress(void);
void set_pmu_timer(uint32 ticks);
static void trap_handler(trap_t *tr);
void pcie_isr_handler(void);
void dma_interrupts_enable(uint32 h2d, uint32 enable, uint32 mask);
void dma_transfer(dma_params_t *dma_params);

static uint32 current_clock = 1;

static uint32 wfi_ticks = 1000;

static void
timer_setup(uint tcks)
{
	ACCESS(REGBASE_ARMCR4 + ARMCR4_IRQMASK) |= ARMCR4_TIMER_INTR_MASK;
	set_arm_intmask(ARMCR4_INT_MASK_TIMER);
	set_arm_inttimer(tcks);
}


/* Exception Handler */
static void trap_handler(trap_t *tr)
{
	register uint32 type = tr->type;
	register uint32 intstatus = ACCESS(REGBASE_ARMCR4 + ARMCR4_FIQRSTATUS);
	register uint32 core;

	pcr4host_stat->int_count[type] ++;
	pcr4host_stat->status = intstatus;

	if (type == TR_DAB)
		ACCESS(REGBASE_ARMCR4 + 0x30) = 1; /* trigger */

	for (core = 0; core < 32; core ++)
	{
		if (intstatus & (1 << core))
			pcr4host_stat->core_int_count[core] ++;
	}

	/* check and clear interrupt status */
	/* SW0 */
	if (intstatus & ARMCR4_SW0_INTR_MASK) {
		ACCESS(REGBASE_ARMCR4 + ARMCR4_FIQRSTATUS) = ARMCR4_SW0_INTR_MASK;
		pcr4host_stat->swint_count[0] ++;
	}

	/* SW1 */
	if (intstatus & ARMCR4_SW1_INTR_MASK) {
		ACCESS(REGBASE_ARMCR4 + ARMCR4_FIQRSTATUS) = ARMCR4_SW1_INTR_MASK;
		pcr4host_stat->swint_count[1] ++;
	}

	/* PCIE interrupt */
	if (intstatus & ARMCR4_PCIE2_INTR_MASK) {
		pcie_isr_handler();
		ACCESS(REGBASE_ARMCR4 + ARMCR4_FIQRSTATUS) = ARMCR4_PCIE2_INTR_MASK;
	}

	/* CLOCK SWITCH */
	if (intstatus & ARMCR4_CLOCK_STABLE_INTR_MASK) {
		ACCESS(REGBASE_ARMCR4 + ARMCR4_FIQRSTATUS) = ARMCR4_CLOCK_STABLE_INTR_MASK;
		if (current_clock == 1) {
			pcr4host_stat->clk_sw1_count ++;
			ACCESS(REGBASE_ARMCR4 + 0x30) = 1; /* trigger */
		}
		else if (current_clock == 2) {
			pcr4host_stat->clk_sw2_count ++;
			ACCESS(REGBASE_ARMCR4 + 0x30) = 2; /* trigger */
		}
	}

	/* INT_TIMER */
	if (ACCESS(REGBASE_ARMCR4 + ARMCR4_INTSTATUS) & ARMCR4_INT_MASK_TIMER) {
		ACCESS(REGBASE_ARMCR4 + ARMCR4_INTSTATUS) = ARMCR4_INT_MASK_TIMER;
		pcr4host_stat->count ++;

		timer_setup(wfi_ticks);
	}

	if (intstatus & ARMCR4_CC_INTR_MASK) {
		/* PMU TIMER */
		if (ACCESS(&ccr->res_req_timer) &
			(PRRT_REQ_ACTIVE << PMU_FLAGS_SHIFT)) {
			pcr4host_stat->pmu_int_count ++;
			ACCESS(&ccr->pmustatus) = PST_INTPEND;

			/* setup timer */
			ACCESS(&ccr->res_req_timer) = ((PRRT_HT_REQ | PRRT_INTEN) <<
				PMU_FLAGS_SHIFT) | wfi_ticks;
		}
	}
}

/* PCIE interrupts handler */
void pcie_isr_handler(void)
{


	/* DOORBELL0 */
	if (ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) & PCIEGEN2_DB0_MASK) {
		ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) = PCIEGEN2_DB0_MASK;
		pcr4host_stat->doorbell[0] = pcieregs->u.pcie2.dbls[0].host2dev_0;
	}

	/* DOORBELL1 */
	if (ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) & PCIEGEN2_DB1_MASK) {
		ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) = PCIEGEN2_DB1_MASK;
		pcr4host_stat->doorbell[1] = pcieregs->u.pcie2.dbls[0].host2dev_1;
	}

	/* DMA */
	if (ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) & PCIEGEN2_DMA_MASK) {
		pciedma_isr_handler(pcieregs);
		ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTSTATUS) = PCIEGEN2_DMA_MASK;
	}
}

/* DMA */
void dma_interrupts_enable(uint32 h2d, uint32 enable, uint32 mask)
{

	pciedma_interrupts_enable(pcieregs, h2d, mask);

	ACCESS(REGBASE_ARMCR4 + ARMCR4_IRQMASK) |= ARMCR4_PCIE2_INTR_MASK;
	ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTMASK) |= PCIEGEN2_DMA_MASK;

	/* Be sure CM3 DMA interrupts disabled */
	ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTMASK2) &= ~PCIEGEN2_DMA_MASK;
}

void dma_transfer(dma_params_t *dma_params)
{

	dma_interrupts_enable(dma_params->h2d, 1, dma_params->intmask);

	pciedma_transfer_test(dma_params, pcieregs, pcr4host_stat);
}

/* WFI LOOP */
void wfi_loop_start(uint32 ticks)
{

	wfi_ticks = ticks;
	timer_setup(wfi_ticks);

	pcr4host_stat->status = CR4_WFI_LOOP_STARTED;

	/* do nothing till an interrupt is generated. */
	while (1)
	{
	__asm__ __volatile__("wfi");
	}

}


void swint_enable(swint_params_t *params)
{
	if (params->is_nmi) {
		ACCESS(REGBASE_ARMCR4 + ARMCR4_FIQMASK) |= (ARMCR4_SW0_INTR_MASK << params->swint);
		enable_arm_fiq();
	}
	else {
		ACCESS(REGBASE_ARMCR4 + ARMCR4_IRQMASK) |= (ARMCR4_SW0_INTR_MASK << params->swint);
	}
}

void set_cm3_swint(uint32 swint);

void set_cm3_swint(uint32 swint)
{
		ACCESS(REGBASE_ARMCM3 + ARMCM3_SWINT_REG) = (1 << swint);
}

void doorbell_enable(swint_params_t *params);

void doorbell_enable(swint_params_t *params)
{
	/* first mask both */
	ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTMASK) &= ~(PCIEGEN2_DB0_MASK | PCIEGEN2_DB1_MASK);

	if (params->is_nmi) {
		ACCESS(REGBASE_ARMCR4 + ARMCR4_FIQMASK) |= ARMCR4_PCIE2_INTR_MASK;
		ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTMASK) |=
			(PCIEGEN2_DB0_MASK << params->swint);
		enable_arm_fiq();
	}
	else {
		ACCESS(REGBASE_ARMCR4 + ARMCR4_IRQMASK) |= ARMCR4_PCIE2_INTR_MASK;
		ACCESS(REGBASE_PCIEGEN2 + PCIEGEN2_INTMASK) |=
			(PCIEGEN2_DB0_MASK << params->swint);
	}
}

void tcm_stress_start(void)
{
	uint32 count = 0;
	uint32 tcm_addr;

	while (1) {
		tcm_addr = TCM_CR4_ADDR + (count & 0x3ff);
		ACCESS(tcm_addr) = count;
		if (ACCESS(tcm_addr) != count) {
			pcr4host_stat->status = 0xffffffff;
			break;
		}
		count ++;

		pcr4host_stat->count = count;
	}
}
void clock_switch(uint32 n)
{
	uint32 reg_val;

	/* Check ARM status */
	reg_val = ACCESS(REGBASE_ARMCR4 + ARMCR4_CORECTL);
	ACCESS(REGBASE_ARMCR4 + ARMCR4_CORECTL) = reg_val | 0x4; /* Bit-2 */

	/* Request HT and Fast clock and wait to get */
	ACCESS(REGBASE_ARMCR4 + ARMCR4_CLOCK_CONTROL_REG) = 0x102;

	while (!(ACCESS(REGBASE_ARMCR4 + ARMCR4_CLOCK_CONTROL_REG) & ((1<<24) | (1<<19)))) {}

	/* Disable IRQ_COMMON/NMI interrupt generation */
	ACCESS(REGBASE_ARMCR4 + ARMCR4_FIQMASK) = 0;
	ACCESS(REGBASE_ARMCR4 + ARMCR4_IRQMASK) = 0;

	/* Clear any previous interrupt generated by SHIM */
	ACCESS(REGBASE_ARMCR4 + ARMCR4_FIQRSTATUS) = 0xffffffff;

	/* select interrupt routing in SHIM */
	ACCESS(REGBASE_ARMCR4 + ARMCR4_IRQMASK) = ARMCR4_CLOCK_STABLE_INTR_MASK;

	reg_val = ACCESS(REGBASE_ARMCR4 + ARMCR4_CORECTL);
	ACCESS(REGBASE_ARMCR4 + ARMCR4_CORECTL) = reg_val | 0x8; /* Bit-3 */

	current_clock = n;
	if (n == 1)
		ACCESS(REGBASE_ARMCR4 + ARMCR4_CORECTL) = reg_val & 0xff; /* set clock 1:1 */
	if (n == 2)
		ACCESS(REGBASE_ARMCR4 + ARMCR4_CORECTL) = reg_val | 0x400; /* set clock 2:1 */

	/* Put ARM to sleep, dont poll for int, test will fail if sleep didnt happen */
	asm("wfi"); /* Wait For Interrupt */
}

void clock_stress(void)
{

	while (1) {

		clock_switch(2);
		for (pcr4host_stat->count = 0; pcr4host_stat->count < 1000;
			pcr4host_stat->count ++);
		clock_switch(1);
		for (pcr4host_stat->count = 0; pcr4host_stat->count < 1000;
			pcr4host_stat->count ++);
	}
}

void set_pmu_timer(uint32 ticks)
{

	wfi_ticks = ticks;

	/* select interrupt routing in SHIM */
	ACCESS(REGBASE_ARMCR4 + ARMCR4_IRQMASK) = ARMCR4_CC_INTR_MASK;
	/* Enable PMU interrupts in chipcommon */
	ACCESS(&ccr->intmask) = CI_PMU;

	ACCESS(&ccr->res_req_timer_sel) = 0;
	ACCESS(&ccr->res_req_timer) = ((PRRT_HT_REQ | PRRT_INTEN) << PMU_FLAGS_SHIFT) | wfi_ticks;
	/* do nothing till an interrupt is generated. */
	while (1)
	{
		__asm__ __volatile__("wfi");
	}
}

/* Get cmd from host and run test */
void c_main(uint32 ra)
{

	uint32 ticks = 0;
	swint_params_t *swint_params = NULL;
	dma_params_t *dma_params = NULL;

	memset((void*)pcr4host_stat, 0, sizeof(arm_stat_t));

	pcr4host_stat->status = CR4_MAIN_ENTERED;

	hnd_set_trap(trap_handler);
	/* Enable ints in cortex */
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
			wfi_loop_start(0xffffffff);
			break;
		case HOST_CMD_SET_SWINT:
			set_cm3_swint(phost_cmd->params[0]);
			break;
		case HOST_CMD_DOORBELL_ENABLE:
			swint_params = (swint_params_t *)phost_cmd->params;
			doorbell_enable(swint_params);
			wfi_loop_start(0xffffffff);
			break;
		case HOST_CMD_TCM_STRESS:
			tcm_stress_start();
			break;
		case HOST_CMD_CLOCK_SWITCH:
			clock_switch(phost_cmd->params[0]);
			wfi_loop_start(0xffffffff);
			break;
		case HOST_CMD_CLOCK_STRESS:
			clock_stress();
			break;
		case HOST_CMD_PMU_TIMER:
			set_pmu_timer(500);
			break;
		case HOST_CMD_DMA_TRANSFER:
			dma_params = (dma_params_t *)phost_cmd->params;
			dma_transfer(dma_params);
			wfi_loop_start(0xffffffff);
			break;
	}
}
