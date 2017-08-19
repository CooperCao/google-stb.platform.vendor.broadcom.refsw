/*
 * Test rx offload engine (arm) only.
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: traps.c 277884 2011-08-24 23:36:44Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndcpu.h>
#include <hnddma.h>
#include <sbhnddma.h>
#include <bcmdevs.h>
#include <siutils.h>
#include <epivers.h>
#include <d11.h>
#include <sbhndarm.h>
#include <pcie_core.h>

#define BASE_CCREG 0x18000000
#define BASE_D11REG 0x18001000
#define BASE_CR4REG 0x18002000
#define BASE_PCIE2REG 0x18003000

#define I_CHIPCOMMON 	0x1
#define I_DOT11MAC		0x2
#define I_ARMCR4		0x4
#define I_PCIE			0x8
#define I_USB20D		0x10
#define I_DOT11MACALT 	0x20

#define FIRQ_CHIPCOMMON		0
#define FIRQ_DOT11MAC		1
#define FIRQ_ARMCR4			2
#define FIRQ_PCIE			3
#define FIRQ_USB20D			4
#define FIRQ_DOT11MACALT	5
#define MAX_FIRQ_TYPE		6

/*  shared with sbtopciemailbox */
#define I_SBTOPCIEFUNC0_0		0x100
#define I_SBTOPCIEFUNC0_1		0x200
#define I_SBTOPCIEFUNC1_0		0x400
#define I_SBTOPCIEFUNC1_1		0x800
#define I_SBTOPCIEFUNC2_0		x1000
#define I_SBTOPCIEFUNC2_1		0x2000
#define I_SBTOPCIEFUNC3_0		0x4000
#define I_SBTOPCIEFUNC3_1		0x8000

#define RCV1_OFFSET 0x260

#define RXBUF_SIZE	2048
#define RX_NRXD		16

#ifdef	__arm__

static INLINE uint32
arm_cpsr(void)
{
	uint32 res;

	__asm__ __volatile__("mrs	%0, cpsr" :
	                     "=r" (res));
	return res;
}

static INLINE uint32
arm_spsr(void)
{
	uint32 res;

	__asm__ __volatile__("mrs	%0, spsr" :
	                     "=r" (res));
	return res;
}

#if defined(__ARM_ARCH_7M__)
static INLINE uint32
armcm3_apsr(void)
{
	uint32 res;

	__asm__ __volatile__("mrs	%0, apsr" :
	                     "=r" (res));
	return res;
}
#endif	/* ARM7M */
#endif	/* __arm__ */

void c_main(unsigned long ra);

static si_t *sih;
static osl_t *osh;
static hnddma_t *di;

static volatile uint traps[MAX_TRAP_TYPE];
static volatile uint firqs[MAX_FIRQ_TYPE];
static volatile uint alltraps = 0;
static volatile uint32 last_type = 0;
static volatile uint32 last_epc = 0;
static volatile uint32 last_status = 0;
static volatile uint32 last_cause = 0;
static volatile uint footprint = 0;
static volatile uint last_cpsr = 0;

typedef volatile struct rxoe_hw {
	dma64regs_t *dmarcv; /* dma rcv regs */
	d11regs_t *d11_regs;
	cr4regs_t *cr4_regs;
	sbpcieregs_t *pcie2_regs;
} rxoe_hw_t;

rxoe_hw_t rxoesr;
rxoe_hw_t *rxsr = &rxoesr;

#if defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
static void
pkt_recv(void * p)
{
	/* pkt processing and freeing should go here */

}

static bool
rxoe_recv(rxoe_hw_t *rxsr, hnddma_t *di)
{
	/* need to consider bounded option */
	bool rtn_val = TRUE;
	void *p;
	void *head = NULL;
	void *tail = NULL;
	uint n = 0;

	while ((p =  dma_rx(di))) {

		if (!tail)
			head = tail = p;
		else {
			PKTSETLINK(tail, p);
			tail = p;
		}
		++n;
	}

	dma_rxfill(di);

	while ((p = head) != NULL) {
		head = PKTLINK(head);
		PKTSETLINK(p, NULL);

		pkt_recv(p);
	}

	rtn_val = (n > 1);

	return rtn_val;
}

static void
trap_handler(trap_t *tr)
{
	uint32 type = tr->type;
	volatile void *armr;
	volatile uint32 intstat = 0;

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
			/* Clear the interrupt */
			intstat = get_arm_firqstatus();

			if (intstat & I_DOT11MACALT) {
				uint altintstat = 0;
				firqs[FIRQ_DOT11MACALT]++;
				altintstat = R_REG(osh, &rxsr->d11_regs->altmacintstatus);
				if (altintstat & MI_DMAINT) {
					rxoe_recv(rxsr, di);
					W_REG(osh, &rxsr->d11_regs->altmacintstatus, MI_DMAINT);
				}

				/* do more dma processing */
			}

			if (intstat & I_PCIE) {
				uint pcieintstat = 0;
				firqs[FIRQ_PCIE]++;
				pcieintstat = R_REG(osh, &rxsr->pcie2_regs->intstatus);
				if (pcieintstat & I_SBTOPCIEFUNC0_0)
					W_REG(osh, &rxsr->pcie2_regs->intstatus, I_SBTOPCIEFUNC0_0);
				/* do more pcie mailbox process */
			}

			set_arm_firqstatus(get_arm_firqstatus());
		}
	} else {
		printf("\nTrap type 0x%x @ epc 0x%x, cpsr 0x%x, spsr 0x%x\n"
		       "sp 0x%x, lp 0x%x, pc 0x%x\n",
		       tr->type, tr->epc, tr->cpsr, tr->spsr,
		       tr->r13, tr->r14, tr->pc);
	}
}

#endif	/* defined(__arm__) || defined(__thumb__) || defined(__thumb2__) */

/* initialize irq related settings */
static void
rxoe_irq_init(void)
{

	/* d11 core int */
	OR_REG(osh, &rxsr->d11_regs->altintmask[1], I_RI);
	OR_REG(osh, &rxsr->d11_regs->altmacintmask, MI_DMAINT);

	/* pcie core int */
	OR_REG(osh, &rxsr->pcie2_regs->intmask, I_SBTOPCIEFUNC0_0 | I_SBTOPCIEFUNC0_1);
	/* cr4 int */
	OR_REG(osh, &rxsr->cr4_regs->isrmask, I_DOT11MACALT | I_PCIE);
}

void
c_main(unsigned long ra)
{
	uint i;
	bool vsim, qt;

	uint msglevel = 2;

	BCMDBG_TRACE(0x545200);

	/* Basic initialization */
	sih = hnd_init();
	osh = si_osh(sih);

	rxsr->d11_regs = (d11regs_t *) BASE_D11REG;
	rxsr->cr4_regs = (cr4regs_t *) BASE_CR4REG;
	rxsr->pcie2_regs = (sbpcieregs_t *) BASE_PCIE2REG;
	rxsr->dmarcv = (dma64regs_t *) (BASE_D11REG + RCV1_OFFSET);

	rxoe_irq_init();

	i = sih->chippkg;
	vsim = (i == HDLSIM_PKG_ID);
	qt = (i == HWSIM_PKG_ID);
	printf("before DMA attach\n");

	di = dma_attach(osh, "rxoe", sih, NULL, rxsr->dmarcv, 0, RX_NRXD, RXBUF_SIZE,
		0, 16, 0, &msglevel);
	if (di == NULL)
		goto dma_attach_fail;
	footprint = 0x545204;

	if (di) {
		dma_rxinit(di);
		dma_rxfill(di);
	}

	/* Setup trap handler */
	for (i = 0; i < MAX_TRAP_TYPE; i++)
		traps[i] = 0;

	for (i = 0; i < MAX_FIRQ_TYPE; i++)
		firqs[i] = 0;

	last_epc = 0;
	last_status = 0;
	last_cause = 0;
	hnd_set_trap(trap_handler);

	enable_arm_irq();
	enable_arm_fiq();

	while (1) {
#if defined(__arm__)
		last_cpsr = arm_cpsr();
#endif
		footprint = 0x545205;
		OSL_DELAY(10);
	}

dma_attach_fail:
	footprint = 0x545206;
	printf("dam_attach failed, exit\n");
}
