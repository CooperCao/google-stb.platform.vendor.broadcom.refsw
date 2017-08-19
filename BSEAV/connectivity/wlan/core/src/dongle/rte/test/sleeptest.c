/*
 * Initialization and support routines for self-booting
 * compressed image.
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

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <sbchipc.h>
#include <hndcpu.h>
#include <siutils.h>
#include <epivers.h>
#include <bcmutils.h>

#include <rte.h>
#include "rte_priv.h"
#include <rte_trap.h>

#include <hnd_boot.h>

void _c_main(unsigned long ra);

static si_t *sih;

static int intr_handled;

#ifndef MAX_TRAP_TYPE
#define MAX_TRAP_TYPE 16
#endif

static volatile uint traps[MAX_TRAP_TYPE];
static volatile uint alltraps = 0;
static volatile uint32 last_type = 0;
static volatile uint32 last_epc = 0;
static volatile uint32 last_status = 0;
static volatile uint32 last_cause = 0;

#ifdef mips

static void
trap_handler(trap_t *tr)
{
	uint32 type = tr->type;
	uint32 cause = tr->cause;
	uint32 status = tr->status;
	uint32 exc;
	mips33regs_t *mipsr;
	osl_t	*osh;

	osh = si_osh(sih);


	alltraps++;
	traps[type >> TRAP_TYPE_SH]++;
	last_type = tr->type;
	last_epc = tr->epc;
	last_status = status;
	last_cause = cause;

	exc = (cause & C_EXC) >> C_EXC_SHIFT;
	if (exc != EXC_INT) {
		if ((cause & C_BD) == 0) {
			tr->epc += 4;
		} else {
			/* Trap in a delay slot! harder to ignore :-( */
		}
	} else {
		if ((mipsr = si_setcore(sih, MIPS33_CORE_ID, 0)) != NULL) {
			/* Clear the interrupt */
			W_REG(osh, &mipsr->intstatus, R_REG(osh, &mipsr->intstatus));
			intr_handled = 1;
		}
	}
}

#endif	/* mips */

static uint max_timer_dur;
static uint flags_shift;
static uint intrcnt = 0;

#if defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
static volatile void *armr;

static void
trap_handler(trap_t *tr)
{
	uint32 type = tr->type;
	osl_t *osh;

	osh = si_osh(sih);

	alltraps++;
	if (type >= MAX_TRAP_TYPE)
		type = TR_BAD;
	traps[type]++;
	last_type = type;
	last_epc = tr->epc;
	last_cause = tr->cpsr;
	last_status = tr->spsr;
	if (FALSE ||
#if defined(__ARM_ARCH_7M__)
	    (type >= TR_ISR && type < TR_ISR + ARMCM3_NUMINTS) ||
#endif
	    FALSE) {

		chipcregs_t *ccr = si_setcore(sih, CC_CORE_ID, 0);
		sbconfig_t *sb = (sbconfig_t *)((volatile uint8 *)ccr + SBCONFIGOFF);
		uint32 sbflagst = R_REG(osh, &sb->sbflagst);
		if (sbflagst & (1 << SI_CC_IDX)) {
			if (R_REG(osh, &ccr->res_req_timer) &
			    (PRRT_REQ_ACTIVE << flags_shift)) {
				W_REG(osh, &ccr->pmustatus, PST_INTPEND);
				AND_REG(osh, ARMREG(armr, clk_ctl_st), ~CCS_FORCEHWREQOFF);
				SPINWAIT(((R_REG(osh, ARMREG(armr, clk_ctl_st)) & CCS_HTAVAIL)
				          == 0), PMU_MAX_TRANSITION_DLY);
				ASSERT(R_REG(osh, ARMREG(armr, clk_ctl_st)) & CCS_HTAVAIL);
				OSL_DELAY(31);
				intrcnt ++;
				intr_handled = 1;
				OR_REG(osh, ARMREG(armr, clk_ctl_st), CCS_FORCEHWREQOFF);
			}
		}
	} else {
		/* Ignore traps */
		printf("\nTrap type 0x%x @ epc 0x%x, cpsr 0x%x, spsr 0x%x\n"
		       "sp 0x%x, lp 0x%x, rpc 0x%x\n",
		       tr->type, tr->epc, tr->cpsr, tr->spsr, tr->r13, tr->r14, tr->pc);
	}
}
#endif	/* defined(__arm__) || defined(__thumb__) || defined(__thumb2__) */

void
_c_main(unsigned long ra)
{
	int i, cnt, wait;
	bool vsim, qt;
	osl_t *osh;
	uint timestamp, now, delta;
	char chn[8];
	chipcregs_t *ccr;

	BCMDBG_TRACE(0x434d0000);

	/* Basic initialization */
	sih = hnd_init();
	osh = si_osh(sih);

	BCMDBG_TRACE(0x434d0001);

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

	for (i = 0; i < MAX_TRAP_TYPE; i++)
		traps[i] = 0;
	last_epc = 0;
	last_status = 0;
	last_cause = 0;
	hnd_set_trap(trap_handler);

#ifndef	RTE_UNCACHED
	/* Initialize and turn caches on */
	caches_on();
#endif

	BCMDBG_TRACE(0x434d0002);

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	BCMDBG_TRACE(0x434d0003);

	if ((sih->cccaps & CC_CAP_PMU) == 0) {
		printf("Chipcommon does not have a PMU\n");
		return;
	}

	if (sih->pmurev > 0) {
		/* max_timer_dur = (1 << 24) - 1; */
		max_timer_dur = (1 << 10) - 1;
		flags_shift = 14;
	}
	else {
		max_timer_dur = (1 << 10) - 1;
		flags_shift = 0;
	}

	hnd_enable_interrupts();

#if defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
	armr = si_setcore(sih, ARM_CORE_ID, 0);
	OR_REG(osh, ARMREG(armr, clk_ctl_st), CCS_FORCEHWREQOFF);
#endif

	ccr = si_setcore(sih, CC_CORE_ID, 0);

	cnt = vsim ? 100 : 1000;
	wait = vsim ? 5 : (qt ? 320 : max_timer_dur);
	timestamp = R_REG(osh, &ccr->pmutimer);
	for (i = 0; i < cnt; i++) {
		intr_handled = 0;
		W_REG(osh, &ccr->res_req_timer_sel, 0);
		W_REG(osh, &ccr->res_req_timer,
		      ((PRRT_HT_REQ | PRRT_INTEN) << flags_shift) | wait);
		do {
			hnd_wait_irq(sih);
		}
		while (intr_handled == 0);
	}
	now = R_REG(osh, &ccr->pmutimer);
	if (now < timestamp)
		delta = 0xffffffff - timestamp + now;
	else
		delta = now - timestamp;
	printf("Trapped %d times in %u ticks\n",
	       alltraps, delta);
	printf("Woke up %u times at interval %u(%u) ticks\n",
	       intrcnt, delta/intrcnt, max_timer_dur);
}
