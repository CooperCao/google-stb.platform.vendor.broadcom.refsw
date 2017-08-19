/*
 * Enhanced Coexistance Interface (ECI) tests.
 *
 * Most tests require the ECI to be in "external loopback" mode
 * with BT firmware's help or a special QT database that echo
 * ECIOutput signals back to ECIInput signals.
 *
 * When run on the special QT database ECIOutput signals and
 * ECIInputL signals are tied together so everything we write
 * to ECIOutput register are written back to ECIInputL register.
 *
 * When run on the real chip it requires BT to run a special
 * firmware which writes ECIOutput bit 29:0 back to ECIInputL
 * bit 29:0 and also duplicate ECIOutput to ECIInputM signals
 * when ECIOutput bit 30 is set (valid flag).
 *
 * How do we test ECIInput 95:64? They are BT hardware signals.
 * We can't test ECIInputH signals yet until BT hardware writes
 * some value and tells us how we should interpret them...
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
#include <epivers.h>
#include <osl.h>
#include <hndsoc.h>
#include <bcmdevs.h>
#include <sbchipc.h>
#include <hndcpu.h>
#include <siutils.h>
#include <bcmutils.h>

#include <rte_chipc.h>
#include <rte.h>
#include "rte_priv.h"

#include <hnd_boot.h>

static si_t *sih;
static osl_t *osh;
static chipcregs_t *cc;

static int intr_type;
static int intr_handled;
static int intr_polarity;
static uint expect_val;
static uint expect_event;

/* bit 30 is valid bit */
#define ECIO_VALID_BIT	30

/* time in us for BT FW to loopback */
#define BTFW_RESP_DELAY	2000

/* # retries to read loopback value */
#define ECIO_LOOPBACK_RETRIES	2

/* time in us for level intr to clear after resetting the output */
#define ECIINT_CLEAR_DELAY	1000

void _c_main(unsigned long ra);

static void
eci_output(uint32 val)
{
	if (sih->chippkg == HDLSIM_PKG_ID || sih->chippkg == HWSIM_PKG_ID) {
		if (sih->ccrev < 35) {
			W_REG(osh, &cc->eci.lt35.eci_output, val);
		}
		else {
			W_REG(osh, &cc->eci.ge35.eci_outputlo, val);
		}
	}
	else {
		if (sih->ccrev < 35) {
			W_REG(osh, &cc->eci.lt35.eci_output, val & ~(1 << ECIO_VALID_BIT));
		}
		else {
			W_REG(osh, &cc->eci.ge35.eci_outputlo, val & ~(1 << ECIO_VALID_BIT));
		}
		OSL_DELAY(BTFW_RESP_DELAY);
		if (sih->ccrev < 35) {
			W_REG(osh, &cc->eci.lt35.eci_output, val | (1 << ECIO_VALID_BIT));
		}
		else {
			W_REG(osh, &cc->eci.ge35.eci_outputlo, val | (1 << ECIO_VALID_BIT));
		}
	}
}

static void
eci_reset_output(void)
{
	if (sih->ccrev < 35) {
		W_REG(osh, &cc->eci.lt35.eci_output, 0);
	}
	else {
		W_REG(osh, &cc->eci.ge35.eci_outputlo, 0);
	}
	if (sih->chippkg == HDLSIM_PKG_ID || sih->chippkg == HWSIM_PKG_ID)
		return;
	OSL_DELAY(BTFW_RESP_DELAY);
}

static int
eci_loopback(uint32 val)
{
	eci_output(val);

	if (sih->chippkg == HDLSIM_PKG_ID || sih->chippkg == HWSIM_PKG_ID) {
		if (sih->ccrev < 35) {
			return R_REG(osh, &cc->eci.lt35.eci_inputlo) != val;
		}
		else {
			return R_REG(osh, &cc->eci.ge35.eci_inputlo) != val;
		}
	}
	else {
		uint32 val1 = val;
		uint32 val2 = val | (1 << ECIO_VALID_BIT);
		int i;
		for (i = 0; i < ECIO_LOOPBACK_RETRIES; i ++) {
			if (sih->ccrev < 35) {
				if (val1 == R_REG(osh, &cc->eci.lt35.eci_inputlo) &&
				    val2 == R_REG(osh, &cc->eci.lt35.eci_inputmi))
				return 0;
			}
			else {
				if (val1 == R_REG(osh, &cc->eci.ge35.eci_inputlo) &&
				    val2 == R_REG(osh, &cc->eci.ge35.eci_inputhi))
				return 0;
			}
			OSL_DELAY(BTFW_RESP_DELAY);
		}
		return 1;
	}
}

static void
eci_disable_edge_interrupts(void)
{
	if (sih->ccrev < 35) {
		W_REG(osh, &cc->eci.lt35.eci_eventmaskhi, 0x0);
		W_REG(osh, &cc->eci.lt35.eci_eventmaskmi, 0x0);
		W_REG(osh, &cc->eci.lt35.eci_eventmasklo, 0x0);
	}
	else {
		W_REG(osh, &cc->eci.ge35.eci_eventmaskhi, 0x0);
		W_REG(osh, &cc->eci.ge35.eci_eventmasklo, 0x0);
	}
}

static void
eci_enable_edge_interrupts(void)
{
	if (sih->ccrev < 35) {
		W_REG(osh, &cc->eci.lt35.eci_eventmaskmi, 0xffffffff & ~(1 << ECIO_VALID_BIT));
		W_REG(osh, &cc->eci.lt35.eci_eventmasklo, 0xffffffff & ~(1 << ECIO_VALID_BIT));
	}
	else {
		W_REG(osh, &cc->eci.ge35.eci_eventmaskhi, 0xffffffff & ~(1 << ECIO_VALID_BIT));
		W_REG(osh, &cc->eci.ge35.eci_eventmasklo, 0xffffffff & ~(1 << ECIO_VALID_BIT));
	}
}

static int
eci_clear_edge_interrupts(void)
{
	eci_reset_output();

	if (sih->ccrev < 35) {
		W_REG(osh, &cc->eci.lt35.eci_eventlo, R_REG(osh, &cc->eci.lt35.eci_eventlo));
		W_REG(osh, &cc->eci.lt35.eci_eventmi, R_REG(osh, &cc->eci.lt35.eci_eventmi));
	}
	else {
		W_REG(osh, &cc->eci.lt35.eci_eventlo, R_REG(osh, &cc->eci.ge35.eci_eventlo));
		W_REG(osh, &cc->eci.lt35.eci_eventhi, R_REG(osh, &cc->eci.ge35.eci_eventhi));
	}

	return (R_REG(osh, &cc->intstatus) & CI_ECI) ? 1 : 0;
}

static void
eci_disable_level_interrupts(void)
{
	if (sih->ccrev < 35) {
		W_REG(osh, &cc->eci.lt35.eci_intmasklo, 0);
		W_REG(osh, &cc->eci.lt35.eci_intmaskmi, 0);
		W_REG(osh, &cc->eci.lt35.eci_intmaskhi, 0);
	}
	else {
		W_REG(osh, &cc->eci.ge35.eci_intmasklo, 0);
		W_REG(osh, &cc->eci.ge35.eci_intmaskhi, 0);
	}
}

static void
eci_enable_level_interrupts(void)
{
	if (sih->ccrev < 35) {
		W_REG(osh, &cc->eci.lt35.eci_inputintpolaritylo, intr_polarity ? 0 : 0xffffffff);
		W_REG(osh, &cc->eci.lt35.eci_inputintpolaritymi, intr_polarity ? 0 : 0xffffffff);
		W_REG(osh, &cc->eci.lt35.eci_intmasklo, 0xffffffff & ~(3 << ECIO_VALID_BIT));
		W_REG(osh, &cc->eci.lt35.eci_intmaskmi, 0xffffffff & ~(3 << ECIO_VALID_BIT));
	}
	else {
		W_REG(osh, &cc->eci.ge35.eci_inputintpolaritylo, intr_polarity ? 0 : 0xffffffff);
		W_REG(osh, &cc->eci.ge35.eci_inputintpolarityhi, intr_polarity ? 0 : 0xffffffff);
		W_REG(osh, &cc->eci.ge35.eci_intmasklo, 0xffffffff & ~(3 << ECIO_VALID_BIT));
		W_REG(osh, &cc->eci.ge35.eci_intmaskhi, 0xffffffff & ~(3 << ECIO_VALID_BIT));
	}
}

static int
eci_clear_level_interrupts(void)
{
	eci_output(intr_polarity ? 0 : (0xffffffff & ~(1 << ECIO_VALID_BIT)));

	SPINWAIT(R_REG(osh, &cc->intstatus) & CI_ECI, ECIINT_CLEAR_DELAY);
	return (R_REG(osh, &cc->intstatus) & CI_ECI) ? 1 : 0;
}

static void
eci_init(void)
{
	eci_output(0);
	eci_disable_edge_interrupts();
	eci_disable_level_interrupts();
}

/* Test ECIOutput register write/read */
static void
test_eci_output(void)
{
	int i;
	uint32 val;

	printf("testing ECIOutput register read/write...");

	/* test walking one */
	for (i = 0; i < 32; i ++) {
		val = 1 << i;
		if (sih->ccrev < 35) {
			W_REG(osh, &cc->eci.lt35.eci_output, val);
			if (val != R_REG(osh, &cc->eci.lt35.eci_output)) {
				printf("fail (0x%08x)\n", val);
				return;
			}
		}
		else {
			W_REG(osh, &cc->eci.ge35.eci_outputlo, val);
			if (val != R_REG(osh, &cc->eci.ge35.eci_outputlo)) {
				printf("fail (0x%08x)\n", val);
				return;
			}
		}
	}

	/* test pattern 0xa5 */
	val = 0xa5a5a5a5;
	if (sih->ccrev < 35) {
		W_REG(osh, &cc->eci.lt35.eci_output, val);
		if (val != R_REG(osh, &cc->eci.lt35.eci_output)) {
			printf("fail (0x%08x)\n", val);
			return;
		}
	}
	else {
		W_REG(osh, &cc->eci.ge35.eci_outputlo, val);
		if (val != R_REG(osh, &cc->eci.ge35.eci_outputlo)) {
			printf("fail (0x%08x)\n", val);
			return;
		}
	}

	/* test pattern 0x5a */
	val = 0x5a5a5a5a;
	if (sih->ccrev < 35) {
		W_REG(osh, &cc->eci.lt35.eci_output, val);
		if (val != R_REG(osh, &cc->eci.lt35.eci_output)) {
			printf("fail (0x%08x)\n", val);
			return;
		}
	}
	else {
		W_REG(osh, &cc->eci.ge35.eci_outputlo, val);
		if (val != R_REG(osh, &cc->eci.ge35.eci_outputlo)) {
			printf("fail (0x%08x)\n", val);
			return;
		}
	}

	printf("pass\n");
}

/* Test ECIOutput->ECIInput loopback (with BT firmware's help) */
static void
test_eci_loopback(void)
{
	int i;

	printf("test_eci_loopback...");

	eci_init();

	/* test pattern 0x5a */
	if (eci_loopback(0x1a5a5a5a)) {
		printf("fail (0x1a5a5a5a)\n");
		return;
	}

	/* test pattern 0xa5 */
	if (eci_loopback(0x25a5a5a5)) {
		printf("fail (0x25a5a5a5)\n");
		return;
	}

	/* test walking one */
	for (i = 0; i < ECIO_VALID_BIT; i ++) {
		if (eci_loopback(1 << i)) {
			printf("fail (%d)\n", 1 << i);
			return;
		}
	}

	printf("pass\n");
}

/* Test ECI interrupts (with BT firmware's help) */
static void
test_eci_edge_interrupts(int enable)
{
	int i;
	uint val;

	intr_type = 1;

	printf("test_eci_edge_interrupts %s...", enable ? "enable" : "disable");

	eci_init();
	eci_clear_edge_interrupts();
	if (enable == 1)
		eci_enable_edge_interrupts();
	else
		eci_disable_edge_interrupts();

	for (i = 0; i < ECIO_VALID_BIT; i ++) {
		if (R_REG(osh, &cc->intstatus) & CI_ECI) {
			printf("error : intstatus is still on ");
			break;
		}

		intr_handled = -1;

		expect_val = val = 1 << i;
		if (i == 0)
			expect_event = expect_val;
		else
			expect_event = 3 << (i - 1);
		eci_output(val);

		SPINWAIT((hnd_poll(sih), intr_handled == -1), BTFW_RESP_DELAY);
		if (enable == 1 && intr_handled == -1) {
			printf("error : intstatus not set for value 0x%08x ", val);
			break;
		} else if ((enable == 1 && intr_handled == 0) ||
		           (enable == 0 && (R_REG(osh, &cc->intstatus) & CI_ECI))) {
			printf("error : intstatus not clear for value 0x%08x ", val);
			break;
		}
	}

	if (i == ECIO_VALID_BIT)
		printf("pass\n");
	else
		printf("fail\n");
}

static void
test_eci_level_interrupts(int enable, int polarity)
{
	int i;
	uint val;

	intr_type = 2;
	intr_polarity = polarity;

	printf("test_eci_level_interrupts %s polarity %d...",
	       enable ? "enable" : "disable", polarity);

	eci_init();
	eci_clear_level_interrupts();
	if (enable == 1)
		eci_enable_level_interrupts();
	else
		eci_disable_level_interrupts();

	for (i = 0; i < ECIO_VALID_BIT; i ++) {
		if (R_REG(osh, &cc->intstatus) & CI_ECI) {
			printf("error : intstatus is still on ");
			break;
		}

		intr_handled = -1;

		if (polarity == 1)
			val = 1 << i;
		else
			val = ~(1 << i) & ~(3 << ECIO_VALID_BIT);
		expect_val = val;
		eci_output(val);

		SPINWAIT((hnd_poll(sih), intr_handled == -1), BTFW_RESP_DELAY);
		if (enable == 1 && intr_handled == -1) {
			printf("error : intstatus not set for value 0x%08x ", val);
			break;
		} else if ((enable == 1 && intr_handled == 0) ||
		           (enable == 0 && (R_REG(osh, &cc->intstatus) & CI_ECI))) {
			printf("error : intstatus not clear for value 0x%08x ", val);
			break;
		}
	}

	if (i == ECIO_VALID_BIT)
		printf("pass\n");
	else
		printf("fail\n");
}

static void
eci_isr(void* cbdata, uint32 ccintst)
{
	uint lo;
	uint mi;
	uint elo, emi;

	if (sih->ccrev < 35) {
		lo = R_REG(osh, &cc->eci.lt35.eci_inputlo);
		mi = R_REG(osh, &cc->eci.lt35.eci_inputmi);
	}
	else {
		lo = R_REG(osh, &cc->eci.ge35.eci_inputlo);
		mi = R_REG(osh, &cc->eci.ge35.eci_inputhi);
	}

	if (expect_val != lo ||
	    expect_val != (mi & ~(1 << ECIO_VALID_BIT)))
		printf("\ninputlo = 0x%08x inputmi = 0x%08x", lo, mi);

	switch (intr_type) {
	case 1:
		if (sih->ccrev < 35) {
			elo = R_REG(osh, &cc->eci.lt35.eci_eventlo);
			emi = R_REG(osh, &cc->eci.lt35.eci_eventmi);
		}
		else {
			elo = R_REG(osh, &cc->eci.ge35.eci_eventlo);
			emi = R_REG(osh, &cc->eci.ge35.eci_eventhi);
		}
		if (expect_event != elo ||
		    expect_event != (emi & ~(1 << ECIO_VALID_BIT)))
			printf("\neventlo = 0x%08x eventmi = 0x%08x", elo, emi);
		intr_handled = eci_clear_edge_interrupts() == 0;
		break;

	case 2:
		intr_handled = eci_clear_level_interrupts() == 0;
		break;

	default:
		printf("Unknown interrupt type %d\n", intr_type);
		break;
	}
}

static void
test_eci_interrupts(void)
{
	eci_init();

	si_cc_register_isr(sih, eci_isr, CI_ECI, NULL);
#ifndef RTE_POLL
	hnd_enable_interrupts();
#endif	/* RTE_POLL */
	test_eci_edge_interrupts(1);
	OSL_DELAY(5000);
	test_eci_edge_interrupts(0);
	OSL_DELAY(5000);
	test_eci_level_interrupts(1, 1);
	OSL_DELAY(5000);
	test_eci_level_interrupts(0, 1);
	OSL_DELAY(5000);
	test_eci_level_interrupts(1, 0);
	OSL_DELAY(5000);
	test_eci_level_interrupts(0, 0);
	OSL_DELAY(5000);
}

void
_c_main(unsigned long ra)
{
	bool vsim, qt;
	char chn[8];

	caches_on();

	sih = hnd_init();
	ASSERT(sih);

	osh = si_osh(sih);
	ASSERT(osh);

	vsim = sih->chippkg == HDLSIM_PKG_ID;
	qt = sih->chippkg == HWSIM_PKG_ID;

	printf("%s ver %s compiled at %s on %s\n",
	       __FILE__, EPI_VERSION_STR, __TIME__, __DATE__);
	printf("Running on a %s%s Rev. %d @ %d/%d/%d/%d Mhz.\n",
	       bcm_chipname(sih->chip, chn, 8), vsim ? "(vsim)" : (qt ? "(qt)" : ""),
	       sih->chiprev, si_cpu_clock(sih) / 1000000, si_mem_clock(sih) / 1000000,
	       si_clock(sih) / 1000000, si_alp_clock(sih) / 1000000);

	if (sih->ccrev < 21 || (sih->cccaps & CC_CAP_ECI) == 0) {
		printf("chipc corerev < 21, or no ECI\n");
		return;
	}

	cc = si_setcoreidx(sih, SI_CC_IDX);
	ASSERT(cc);
	AND_REG(osh, &cc->intmask, ~CI_ECI);

#if defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
{
	volatile void *arm = si_setcore(sih, ARM_CORE_ID, 0);
	ASSERT(arm);
	OR_REG(osh, ARMREG(arm, clk_ctl_st), CCS_FORCEHT);
}
#endif

	/* Test ECIOutput register read/write */
	test_eci_output();

	/* Test ECIOutput->ECIInput loopback (with BT firmware's help) */
	test_eci_loopback();

	/* Test ECI interrupts */
	test_eci_interrupts();

	printf("ECI tests are done\n");
}
