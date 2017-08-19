/*
 * Test traps only.
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
#include <osl.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndcpu.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <epivers.h>
#include <bcmutils.h>

#include <rte.h>
#include "rte_priv.h"
#include <rte_trap.h>

#include <hnd_boot.h>

void _c_main(unsigned long ra);

static si_t *sih;
static uint32 clock;
static uint32 ticks;

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
			osl_t	*osh = si_osh(sih);

			/* Clear the interrupt */
			W_REG(osh, &mipsr->intstatus, R_REG(osh, &mipsr->intstatus));
			/* and re-arm it */
			W_REG(osh, &mipsr->timer, ticks);
		}
	}
}

static void
cause_trap(void)
{
	volatile uint32 *p;

	/* Cause a trap, use assembly so the compiler won't move it around */
	p = NULL;
	__asm__ __volatile__(".set\tnoreorder\n\t"
			     "nop\n\t"
			     "lw\t$0,0(%0)\n\t"
			     "nop\n\t"
			     ".set\treorder"
			     :
			     : "r" (p));
}

static void
timer_setup(uint tcks)
{
	uint32 c0reg;
	mips33regs_t *mipsr;

	ticks = tcks;

	/* Enable interrupts and setup the mips timer */
	if ((mipsr = si_setcore(sih, MIPS33_CORE_ID, 0)) != NULL) {
		osl_t *osh = si_osh(sih);

		/* Clear any pending int */
		W_REG(osh, &mipsr->intstatus, R_REG(osh, &mipsr->intstatus));
		c0reg = MFC0(C0_STATUS, 0);

		if (ticks == 0) {
			W_REG(osh, &mipsr->intmask, 0);
			W_REG(osh, &mipsr->timer, 0);
			/* Disable ints */
			c0reg &= ~ST0_IE;
			MTC0(C0_STATUS, 0, c0reg);
		} else {
			W_REG(osh, &mipsr->intmask, 1);
			W_REG(osh, &mipsr->timer, ticks);
			MTC0(C0_CAUSE, 0, MFC0(C0_CAUSE, 0) & ~ST0_IM);
			/* hwint0 for 4712, hwint4 for 4320 */
			MTC0(C0_STATUS, 0, (c0reg | ST0_HWINT0 | ST0_HWINT4 | ST0_IE));
		}
	} else {
		printf("Cannot find the mips core!\n");
	}
}

#endif	/* mips */

#if defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
static void
trap_handler(trap_t *tr)
{
	uint32 type = tr->type;
	volatile void *armr;

	alltraps++;
	if (type >= MAX_TRAP_TYPE)
		type = TR_BAD;
	traps[type]++;
	last_type = type;
	last_epc = tr->epc;
	last_cause = tr->cpsr;
	last_status = tr->spsr;
#if defined(__ARM_ARCH_7M__)
	if (type >= TR_ISR && type < TR_ISR + ARMCM3_NUMINTS) {
#else
	if (type == TR_IRQ) {
#endif
		if ((armr = si_setcore(sih, ARM_CORE_ID, 0)) != NULL) {
#if !defined(__ARM_ARCH_7A__)
			/* Clear the interrupt */
			set_arm_intstatus(get_arm_intstatus());
			/* and re-arm it */
			set_arm_inttimer(ticks);
#endif /* !__ARM_ARCH_7A__ */
		}
	} else {
		printf("\nTrap type 0x%x @ epc 0x%x, cpsr 0x%x, spsr 0x%x\n"
		       "sp 0x%x, lp 0x%x, pc 0x%x\n",
		       tr->type, tr->epc, tr->cpsr, tr->spsr,
		       tr->r13, tr->r14, tr->pc);
	}
}

static void
cause_trap(void)
{
	chipcregs_t *cc;
	uint32 tmp;
	volatile uint32 *p;

	if ((cc = si_setcoreidx(sih, SI_CC_IDX)) == NULL) {
		printf("Cannot find chipc, trying for alignment\n");
		p = &tmp;
		p = (volatile uint32 *)(((uint)p) + 1);
	} else {
		p = (volatile uint32 *)(&cc->uart0data);
	}
	tmp = *p;
}

static void
timer_setup(uint tcks)
{
#if !defined(__ARM_ARCH_7A__)
	volatile void *armr;

	ticks = tcks;

	if ((armr = si_setcore(sih, ARM_CORE_ID, 0)) != NULL) {
		/* Clear any previous timer interrupt */
		set_arm_intstatus(get_arm_intstatus());
		if (ticks == 0) {
			/* Disable ints */
			disable_arm_irq();
			set_arm_inttimer(0);
		} else {
#if defined(__ARM_ARCH_7M__)
			/* Enable ints */
			enable_arm_irq();
#endif
			/* and arm it */
			set_arm_inttimer(ticks);
		}
	}
#else /* !__ARM_ARCH_7A__ */
	BCM_REFERENCE(ticks);
#endif /* !__ARM_ARCH_7A__ */
}

#endif	/* defined(__arm__) || defined(__thumb__) || defined(__thumb2__) */

void
_c_main(unsigned long ra)
{
	uint i, prevtraps, tcks;
	bool vsim, qt;
	char chn[8];

	/* Basic initialization */
	sih = hnd_init();

	i = sih->chippkg;
	vsim = (i == HDLSIM_PKG_ID);
	qt = (i == HWSIM_PKG_ID);

	/* Setup trap handler */
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

	clock = si_clock(sih);
	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       clock / 1000000, si_alp_clock(sih) / 1000000);

	/* Simple trap test */
	prevtraps = alltraps;
	cause_trap();
	OSL_DELAY(vsim ? 10 : 1000);
	if (alltraps == (prevtraps + 1)) {
		printf("Caused and handled a trap ok\n");
		printf("Last trap: type=%d epc=0x%x status=0x%x cause=0x%x\n",
		       last_type, last_epc, last_status, last_cause);
	} else
		printf("Caused a trap, but trap count is incorrect: %d (prev %d)\n",
		       alltraps, prevtraps);

	/* Simple interrupt test */
#ifndef RTE_POLL
	hnd_enable_interrupts();
#endif
	alltraps = prevtraps = 0;

#ifdef mips
	if (si_corerev(sih) < 4)
		tcks = clock;
	else
		tcks = 20000000;
#else
	/* arm */
	tcks = 80000000;
#endif
	tcks = vsim ? 10000 : (qt ? (tcks / 100) : (5 * tcks));
	timer_setup(tcks);
	while (prevtraps < (vsim ? 1 : 10)) {
		uint trapsnow = alltraps;
		if (trapsnow != prevtraps) {
			printf("Traps now: %d\n", trapsnow);
			printf("Last trap: type=%d epc=0x%x status=0x%x cause=0x%x\n",
			       last_type, last_epc, last_status, last_cause);
			prevtraps = trapsnow;
		}
		hnd_poll(sih);
	}
	timer_setup(0);

	printf("traps: Done, bye\n");
}
