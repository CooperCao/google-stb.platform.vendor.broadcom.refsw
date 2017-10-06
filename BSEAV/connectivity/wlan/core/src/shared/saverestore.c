/*
 * Functions for supporting HW save-restore functionality
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id$
 */

#include <typedefs.h>
#include <osl.h>
#include <sbchipc.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <sbsocram.h>
#include <hndpmu.h>

#include <saverestore.h>
#include <sbgci.h>

#if defined(BCMDBG_ERR)
#define	SR_ERROR(args)	printf args
#else
#define	SR_ERROR(args)
#endif	/* BCMDBG_ERR */

static uint32 sr_control(si_t *sih, int sr_core, uint32 n, uint32 mask, uint32 val);

#ifdef SR_DEBUG
static uint32 nvram_min_sr_mask;
static bool min_sr_mask_valid = FALSE;
#endif /* SR_DEBUG */

/* minimal SR functionality to support srvsdb */
#ifndef SR_ESSENTIALS_DISABLED
bool _sr_essentials = TRUE;
#else
bool _sr_essentials = FALSE;
#endif

/* Power save related save/restore support */
#ifndef SAVERESTORE_DISABLED
bool _sr = TRUE;
#else
bool _sr = FALSE;
#endif

#ifdef SRFAST
/* Save/restore fast mode support */
#ifndef SR_FAST_DISABLED
bool _sr_fast = TRUE;
#else
bool _sr_fast = FALSE;
#endif
#endif /* SRFAST */

#define SR_MAX_CLIENT		4
typedef struct {
	sr_save_callback_t cb;
	void *arg;
} sr_s_client_info_t;

typedef struct {
	sr_restore_callback_t cb;
	void *arg;
} sr_r_client_info_t;

#if defined(SAVERESTORE)
static bool g_wakeup_from_deep_sleep = FALSE;
static uint32 g_num_wakeup_from_deep_sleep = 0;
static void sr_update_deep_sleep(si_t *sih);
static sr_r_client_info_t sr_restore_list[SR_MAX_CLIENT];
static sr_s_client_info_t sr_save_list[SR_MAX_CLIENT];
static void sr_wokeup_from_deep_sleep(bool state);
static bool sr_is_wakeup_from_deep_sleep(void);

/* Accessor Functions */
static sr_r_client_info_t* BCMRAMFN(sr_get_restore_info)(uint8 idx)
{
	return &sr_restore_list[idx];
}

static sr_s_client_info_t* BCMRAMFN(sr_get_save_info)(uint8 idx)
{
	return &sr_save_list[idx];
}
#endif /* SAVERESTORE */

/* Access the save-restore portion of the PMU chipcontrol reg */
uint32
sr_chipcontrol(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43242_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	case BCM43430_CHIP_ID:
	case BCM43018_CHIP_ID:
		/* All 32 bits of pmu chipctl3 are for save-restore.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_chipcontrol(sih, 3, mask, val);
		break;
	CASE_BCM43602_CHIP: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	case BCM43012_CHIP_ID:
	/* intentional fall through */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/* Access the chipcontrol reg 4 */
uint32
sr_chipcontrol4(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	case BCM43430_CHIP_ID:
	case BCM43018_CHIP_ID:
		/* In 43239, all 32 bits are for save-restore chipcontrol.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_chipcontrol(sih, 4, mask, val);
		break;
	case BCM43242_CHIP_ID: /* fall through. pmu chip control #4 reg contains no S/R bits */
	CASE_BCM43602_CHIP: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	case BCM43012_CHIP_ID:
	/* intentional fall through */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/* Access the chipcontrol reg 2 */
uint32
sr_chipcontrol2(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	case BCM43430_CHIP_ID:
	case BCM43018_CHIP_ID:
		/* In 43239, all 32 bits are for save-restore chipcontrol.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_chipcontrol(sih, 2, mask, val);
		break;
	CASE_BCM43602_CHIP: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	case BCM43242_CHIP_ID: /* fall through. pmu chip control #2 reg contains no S/R bits */
	case BCM43012_CHIP_ID:
	/* intentional fall through */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/* Access the chipcontrol reg 5 */
uint32
sr_chipcontrol5(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	case BCM43430_CHIP_ID:
	case BCM43018_CHIP_ID:
		/* In 43239, all 32 bits are for save-restore chipcontrol.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_chipcontrol(sih, 5, mask, val);
		break;
	CASE_BCM43602_CHIP: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	case BCM43242_CHIP_ID: /* fall through. pmu chip control #5 reg contains no S/R bits */
	case BCM43012_CHIP_ID:
	/* intentional fall through */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/* Access the chipcontrol reg 6 */
uint32
sr_chipcontrol6(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	case BCM43430_CHIP_ID:
	case BCM43018_CHIP_ID:
		/* In 43239, all 32 bits are for save-restore chipcontrol.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_chipcontrol(sih, 6, mask, val);
		break;
	CASE_BCM43602_CHIP: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	case BCM43242_CHIP_ID: /* fall through. pmu chip control #6 reg contains no S/R bits */
	case BCM43012_CHIP_ID:
	/* intentional fall through */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/** Access regulator control register 4 */
uint32
sr_regcontrol4(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	case BCM43909_CHIP_ID:
	case BCM43430_CHIP_ID:
	case BCM43018_CHIP_ID:

		/* In 43239, all 32 bits are for save-restore chipcontrol.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_vreg_control(sih, 4, mask, val);
		break;
	case BCM43242_CHIP_ID: /* fall through. pmu reg control #4 is not used in PHOENIX2 branch */
	CASE_BCM43602_CHIP: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	case BCM43012_CHIP_ID:
	/* intentional fall through */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/** register access introduced with 43602a0 */
static uint32
sr_control(si_t *sih, int sr_core, uint32 n, uint32 mask, uint32 val)
{
	uint32 retVal;
	chipcregs_t *cc;
	srregs_t *srr;
	uint origidx = si_coreidx(sih);
	uint intr_val = 0;
	volatile unsigned int *p;
	osl_t *osh = si_osh(sih);

	if (!AOB_ENAB(sih)) {
		ASSERT(0);
		return BCME_NOTFOUND;
	}

	/* Block ints and save current core */
	intr_val = si_introff(sih);
	switch (CHIPID(sih->chip)) {
		case BCM4347_CHIP_GRPID:
			if (sr_core <= 1) {
				sr_core = 1 - sr_core;
			}

			srr = si_setcore(sih, SR_CORE_ID, sr_core);

			if (n == 0) {
				p = &srr->sr_control0;
			} else {
				p = &srr->sr_control1;
			}
			break;

		case BCM43012_CHIP_ID:
			/* 43012 has SR as a seperate core */
			srr = si_setcore(sih, SR_CORE_ID, 0);
			if (n == 0)
				p = &srr->sr_control0;
			else
				p = &srr->sr_control1;

			break;

		case BCM4364_CHIP_ID:
		case BCM4373_CHIP_ID:
			if (si_coreunit(sih) == 0)
				sr_core = 1;

		/* intentional fall through */
		default:
			cc = si_setcoreidx(sih, SI_CC_IDX);

			if (sr_core == 0) {
				if (n == 0)
					p = &cc->sr_control0;
				else
					p = &cc->sr_control1;
			} else {
				if (n == 0)
					p = &cc->sr1_control0;
				else
					p = &cc->sr1_control1;
			}

			break;
	}

	retVal = R_REG(osh, p);

	if (mask != 0) {
		AND_REG(osh, p, ~mask);
		OR_REG(osh, p, val);
	}

	si_setcoreidx(sih, origidx);
	si_intrrestore(sih, intr_val);

	return retVal;
}


/* Function to change the sr memory control to sr engine or driver access */
static int
sr_enable_sr_mem_for(si_t *sih, int opt)
{
	int retVal = 0;
	uint origidx;
	sbsocramregs_t *socramregs;
	osl_t *osh = si_osh(sih);

	origidx = si_coreidx(sih);

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
		/* Switch to SOCRAM core */
		socramregs = si_setcore(sih, SOCRAM_CORE_ID, 0);
		if (!socramregs)
			goto done;

		switch (opt) {
		case SR_HOST:
			W_REG(osh, &socramregs->sr_control,
				(R_REG(osh, &socramregs->sr_control) | 0x800));
			break;
		case SR_ENGINE:
			W_REG(osh, &socramregs->sr_control,
				(R_REG(osh, &socramregs->sr_control) & ~(0x800)));
			break;
		default:
			retVal = -1;
		}
		break;
	case BCM4324_CHIP_ID:
	case BCM43242_CHIP_ID:
		/* Disable all the controls */
		/* Switch to SOCRAM core */
		socramregs = si_setcore(sih, SOCRAM_CORE_ID, 0);
		if (!socramregs)
			goto done;
		W_REG(osh, &socramregs->sr_control,
			(R_REG(osh, &socramregs->sr_control) & ~(0x800)));
		/* Below two operations shall not be combined. */
		sr_chipcontrol(sih, 0x1, 0x0); /* Disable sr mem_clk_en */
		sr_chipcontrol(sih, 0x2, 0x0); /* Disable sr mem_clk_sel */

		switch (opt) {
		case SR_HOST:
			W_REG(osh, &socramregs->sr_control,
				(R_REG(osh, &socramregs->sr_control) | 0x800));
			break;
		case SR_ENGINE:
			/* Below two operations shall not be combined. */
			sr_chipcontrol(sih, 0x2, 0x2); /* Enable sr mem_clk_sel */
			sr_chipcontrol(sih, 0x1, 0x1); /* Enable sr mem_clk_en */
			break;
		default:
			retVal = -1;
		}
		break;
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
		/* setcore d11 */
		socramregs = si_setcore(sih, D11_CORE_ID, 0);
		if (!socramregs)
			goto done;

		/* Below two operation shall not be combined. */
		/* chipctlreg 0x3 [hexpr ([chipctlreg 0x3] & ~(0x1))] */
		sr_chipcontrol(sih, 0x1, 0x0);
		/* chipctlreg 0x3 [hexpr ([chipctlreg 0x3] & ~(0x4))] */
		sr_chipcontrol(sih, 0x4, 0x0);

		switch (opt) {
		case SR_HOST:
			break;
		case SR_ENGINE:
			/* Below two operation shall not be combined. */
			/* chipctlreg 0x3 [hexpr ([chipctlreg 0x3] | (0x1))] */
			sr_chipcontrol(sih, 0x1, 0x1);
			break;
		default:
			retVal = -1;
		}
		break;

	default:
		retVal = -1;
	}

	/* Return to previous core */
	si_setcoreidx(sih, origidx);

done:

	return retVal;
}

#if defined(SAVERESTORE)
/** Interface function used to read/write value from/to sr memory */
uint32
sr_mem_access(si_t *sih, int op, uint32 addr, uint32 data)
{
	uint origidx;
	sbsocramregs_t *socramregs;
	uint32 sr_ctrl;
	osl_t *osh = si_osh(sih);

	BCM_REFERENCE(sr_ctrl);
	origidx = si_coreidx(sih);

	sr_enable_sr_mem_for(sih, SR_HOST);

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM43242_CHIP_ID:
	case BCM4345_CHIP_ID:
		/* Switch to SOCRAM core */
		socramregs = si_setcore(sih, SOCRAM_CORE_ID, 0);
		if (!socramregs)
			goto done;
		sr_ctrl = R_REG(osh, &socramregs->sr_control);
		ASSERT(addr < (SRCTL_BANK_NUM(sr_ctrl) * SRCTL_BANK_SIZE(sr_ctrl)));
		BCM_REFERENCE(sr_ctrl);
		W_REG(osh, &socramregs->sr_address, addr);
		if (op == IOV_SET) {
			W_REG(osh, &socramregs->sr_data, data);
		}
		else {
			data = R_REG(osh, &socramregs->sr_data);
		}
		break;
	default:
		data = 0xFFFFFFFF;
	}

	/* Return to previous core */
	si_setcoreidx(sih, origidx);

done:
	sr_enable_sr_mem_for(sih, SR_ENGINE);

	return data;
}
#endif /* SAVERESTORE */

/** This address is offset to the start of the txfifo bank containing the SR assembly code */
static uint32
BCMATTACHFN(sr_asm_addr)(si_t *sih, int sr_core)
{
	switch (CHIPID(sih->chip)) {
	case BCM4335_CHIP_ID:
		return CC4_4335_SR_ASM_ADDR;
	case BCM4345_CHIP_ID:
	case BCM43909_CHIP_ID:
		return CC4_4345_SR_ASM_ADDR;
		break;

	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		return CC4_4350_SR_ASM_ADDR;
	CASE_BCM43602_CHIP:
		return CC_SR1_43602_SR_ASM_ADDR;
	case BCM4349_CHIP_GRPID:
		return CC_SR1_4349_SR_ASM_ADDR;
	case BCM43012_CHIP_ID:
		/* SR  start from 5K boundary */
		return SR1_43012_SR_ASM_ADDR;
	case BCM4347_CHIP_GRPID:
		if (sr_core == SRENG0) {
			return SR_ASM_ADDR_MAIN_4347;
		} else if (sr_core == SRENG1) {
			return SR_ASM_ADDR_AUX_4347;
		} else if (sr_core == SRENG2) {
			return SR_ASM_ADDR_DIG_4347;
		}
		ASSERT(0);
		break;
	case BCM43242_CHIP_ID: /* fall through: SR mem in this chip is not contained in tx fifo */
	default:
		ASSERT(0);
		break;
	}

	return 0;
}

/** Initialize the save-restore engine during chip attach */
void
BCMATTACHFN(sr_save_restore_init)(si_t *sih)
{
	chipcregs_t *cc;
	uint origidx;
	uint intr_val = 0;
	osl_t *osh = si_osh(sih);
	int i;

	/* 43242 supports SR-VSDB but not SR-PS. Still it needs initialization. */
	if (sr_cap(sih) == FALSE && !(CHIPID(sih->chip) == BCM43242_CHIP_ID))
		return;

	/* Block ints and save current core */
	intr_val = si_introff(sih);
	origidx = si_coreidx(sih);

	cc = si_setcoreidx(sih, SI_CC_IDX);
	UNUSED_PARAMETER(cc);
	ASSERT(cc != NULL);
	BCM_REFERENCE(cc);

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID: {
		sbsocramregs_t *regs;

		/* Switch to SOCRAM core */
		if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
			goto done;

		/* Download the save-restore firmware */
		sr_download_firmware(sih);

		/* Initialize save-trigger controls like polarity, etc */
		W_REG(osh, &regs->sr_control,
			(R_REG(osh, &regs->sr_control)| (0x0A00 << 16)));

		/* Enable the save-restore engine */
		sr_engine_enable(sih, SRENG0, IOV_SET, TRUE);
		break;
	}
	case BCM43242_CHIP_ID: {
		/* Switch to SOCRAM core */
		if (!si_setcore(sih, SOCRAM_CORE_ID, 0))
			goto done;

		/* Disable the save-restore engine */
		sr_engine_enable(sih, SRENG0, IOV_SET, FALSE);

		/* Download the save-restore firmware */
		sr_download_firmware(sih);

		/* Set min_div */
		sr_chipcontrol(sih, 0xF0, (0x30));

		/* Enable the save-restore engine */
		sr_engine_enable(sih, SRENG0, IOV_SET, TRUE);
		break;
	}

	case BCM4335_CHIP_ID: {
		/* Download the save-restore firmware */
		/* Initialize save-trigger controls like polarity, etc */

		/* For 4335, SR binary is stored in TX-FIFO */
		/* Hence it is downloaded in wlc.c */
		sr_chipcontrol4(sih, CC4_SR_INIT_ADDR_MASK,
			sr_asm_addr(sih, SRENG0) << CC4_SR_INIT_ADDR_SHIFT);
		sr_enable_sr_mem_for(sih, SR_HOST);
		sr_enable_sr_mem_for(sih, SR_ENGINE);
		sr_engine_enable(sih, SRENG0, IOV_SET, TRUE);
		break;
	}
	case BCM4345_CHIP_ID:
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	{
		/* Add ASM Init address */
		sr_chipcontrol4(sih, CC4_SR_INIT_ADDR_MASK,
			sr_asm_addr(sih, SRENG0) << CC4_SR_INIT_ADDR_SHIFT);

		/* Turn on MEMLPDO before MEMLPLDO power switch .(bit 159) */
		sr_regcontrol4(sih, VREG4_4350_MEMLPDO_PU_MASK,
			(1 << VREG4_4350_MEMLPDO_PU_SHIFT));

		/* Switch to ALP/HT clk as SR clk is too long */
		sr_chipcontrol4(sih, CC4_4350_EN_SR_CLK_ALP_MASK | CC4_4350_EN_SR_CLK_HT_MASK,
			(1 << CC4_4350_EN_SR_CLK_ALP_SHIFT) | (1 << CC4_4350_EN_SR_CLK_HT_SHIFT));

		/* MEMLPLDO power switch enable */
		sr_chipcontrol2(sih, CC2_4350_MEMLPLDO_PWRSW_EN_MASK,
			(1 << CC2_4350_MEMLPLDO_PWRSW_EN_SHIFT));

		/* VDDM power switch enable */
		sr_chipcontrol2(sih, CC2_4350_VDDM_PWRSW_EN_MASK,
			(1 << CC2_4350_VDDM_PWRSW_EN_SHIFT));
#ifdef SRFAST
		if (SR_FAST_ENAB()) {
			sr_chipcontrol2(sih, CC2_4350_PHY_PWRSW_UPTIME_MASK,
				0x4 << CC2_4350_PHY_PWRSW_UPTIME_SHIFT);
			sr_chipcontrol2(sih, CC2_4350_VDDM_PWRSW_UPDELAY_MASK,
				0x4 << CC2_4350_VDDM_PWRSW_UPDELAY_SHIFT);
		}
#endif /* SRFAST */
		if ((BCM4350_CHIP(sih->chip) &&
			CST4350_CHIPMODE_SDIOD(sih->chipst)) ||
			((CHIPID(sih->chip) == BCM4345_CHIP_ID) &&
			CST4345_CHIPMODE_SDIOD(sih->chipst))) {
			sr_chipcontrol2(sih, CC2_4350_SDIO_AOS_WAKEUP_MASK,
				(1 << CC2_4350_SDIO_AOS_WAKEUP_SHIFT));
		} else if ((BCM4350_CHIP(sih->chip) &&
			CST4350_CHIPMODE_PCIE(sih->chipst)) ||
			((CHIPID(sih->chip) == BCM4345_CHIP_ID) &&
			CST4345_CHIPMODE_PCIE(sih->chipst))) {
			si_pmu_chipcontrol(sih, 6,
				(CC6_4350_PCIE_CLKREQ_WAKEUP_MASK |
				CC6_4350_PMU_WAKEUP_ALPAVAIL_MASK),
				((1 << CC6_4350_PCIE_CLKREQ_WAKEUP_SHIFT) |
				(1 << CC6_4350_PMU_WAKEUP_ALPAVAIL_SHIFT)));
		}

		/* Magic number for chip control 3: 0xF012C33 */
		sr_chipcontrol(sih, ~0, (0x3 << CC3_SR_CLK_SR_MEM_SHIFT |
			0x3 << CC3_SR_MINDIV_FAST_CLK_SHIFT |
			1 << CC3_SR_R23_SR_RISE_EDGE_TRIG_SHIFT |
			1 << CC3_SR_R23_SR_FALL_EDGE_TRIG_SHIFT |
			2 << CC3_SR_NUM_CLK_HIGH_SHIFT |
			1 << CC3_SR_PHY_FUNC_PIC_SHIFT |
			0x7 << CC3_SR_ALLOW_SBC_FUNC_PIC_SHIFT |
			1 << CC3_SR_ALLOW_SBC_STBY_SHIFT));
	} break;

	CASE_BCM43602_CHIP:
		/*
		 * In contrast to 4345/4350/4335 chips, there is no need to touch PMU chip control
		 * 3 and 4 for SR setup.
		 */
		sr_control(sih, SRENG0, 0, 0xFFFFFFFF, (1 << CC_SR_CTL0_EN_SR_ENG_CLK_SHIFT)     |
			(0xc << CC_SR_CTL0_RSRC_TRIGGER_SHIFT)    |
			(3 << CC_SR_CTL0_MIN_DIV_SHIFT)           |
			(1 << CC_SR_CTL0_EN_SBC_STBY_SHIFT)       |
			(1 << CC_SR_CTL0_EN_SR_HT_CLK_SHIFT)      |
			(3 << CC_SR_CTL0_ALLOW_PIC_SHIFT)         |
			(0x10 << CC_SR_CTL0_MAX_SR_LQ_CLK_CNT_SHIFT) |
			(1 << CC_SR_CTL0_EN_MEM_DISABLE_FOR_SLEEP));

		W_REG(osh, &cc->sr_control1, sr_asm_addr(sih, SRENG0)); /* TxFIFO offset */
		break;

	case BCM4349_CHIP_GRPID:
		/*
		 * In contrast to 4345/4350/4335 chips, there is no need to touch PMU chip control
		 * 3 and 4 for SR setup.
		 */
		sr_control(sih, SRENG0, 0, ~0,
			CC_SR0_4349_SR_ENG_CLK_EN |
			CC_SR0_4349_SR_RSRC_TRIGGER |
			CC_SR0_4349_SR_WD_MEM_MIN_DIV |
			CC_SR0_4349_SR_ENABLE_ILP |
			CC_SR0_4349_SR_ENABLE_ALP |
			CC_SR0_4349_SR_ENABLE_HT |
			CC_SR0_4349_SR_ALLOW_PIC |
			CC_SR0_4349_SR_PMU_MEM_DISABLE);
#if !defined(USE_MEMLPLDO)
		sr_control(sih, SRENG0, 0, CC_SR0_4349_SR_MEM_STBY_ALLOW_MSK,
			1 << CC_SR0_4349_SR_MEM_STBY_ALLOW_SHIFT);
#else
		sr_control(sih, SRENG0, 0, CC_SR0_4349_SR_MEM_STBY_ALLOW_MSK,
			0 << CC_SR0_4349_SR_MEM_STBY_ALLOW_SHIFT);
#endif
		W_REG(osh, &cc->sr_control1, sr_asm_addr(sih, SRENG0)); /* TxFIFO offset */

		if (CST4349_CHIPMODE_SDIOD(sih->chipst)) {
			si_pmu_chipcontrol(sih, 2, CC2_4349_SDIO_AOS_WAKEUP_MASK,
				(1 << CC2_4349_SDIO_AOS_WAKEUP_SHIFT));
		} else if (CST4349_CHIPMODE_PCIE(sih->chipst)) {
			si_pmu_chipcontrol(sih, 6,
				(CC6_4349_PCIE_CLKREQ_WAKEUP_MASK |
				CC6_4349_PMU_WAKEUP_ALPAVAIL_MASK),
				((1 << CC6_4349_PCIE_CLKREQ_WAKEUP_SHIFT) |
				(1 << CC6_4349_PMU_WAKEUP_ALPAVAIL_SHIFT)));
		}
		break;

	case BCM43012_CHIP_ID:
		/* Values configured for 43012 sr_control = 0x403800F2 */
		sr_control(sih, SRENG0, 0, ~0,
			SR0_43012_SR_ENG_CLK_EN |
			SR0_43012_SR_RSRC_TRIGGER |
			SR0_43012_SR_WD_MEM_MIN_DIV |
			SR0_43012_SR_ENABLE_HT |
			SR0_43012_SR_ENABLE_ILP |
			SR0_43012_SR_ENABLE_ALP |
			SR0_43012_SR_ALLOW_PIC |
			SR0_43012_SR_PMU_MEM_DISABLE);
#if !defined(USE_MEMLPLDO)
		/* mem standby */
		sr_control(sih, SRENG0, 0, SR0_43012_SR_MEM_STBY_ALLOW_MSK,
			1 << SR0_43012_SR_MEM_STBY_ALLOW_SHIFT);
#else
		sr_control(sih, SRENG0, 0, SR0_43012_SR_MEM_STBY_ALLOW_MSK,
			0 << SR0_43012_SR_MEM_STBY_ALLOW_SHIFT);
#endif
		/* TxFIFO offset */
		sr_control(sih, SRENG0, 1, SR1_43012_SR_INIT_ADDR_MASK, sr_asm_addr(sih, SRENG0));

		/* SDIO AOS wake */
		si_pmu_chipcontrol(sih, 2, CC_43012_SDIO_AOS_WAKEUP_MASK,
			(1 << CC_43012_SDIO_AOS_WAKEUP_SHIFT));

		break;

	case BCM4364_CHIP_ID:
	case BCM4373_CHIP_ID: {
		W_REG(osh, &cc->sr_control0,
			CC_SR0_4364_SR_ENG_CLK_EN |
			CC_SR0_4364_SR_RSRC_TRIGGER |
			CC_SR0_4364_SR_WD_MEM_MIN_DIV |
			CC_SR0_4364_SR_ENABLE_HT |
			CC_SR0_4364_SR_ALLOW_PIC |
			CC_SR0_4364_SR_PMU_MEM_DISABLE);

		W_REG(osh, &cc->sr_control1,
			CC_SR1_4364_SR_CORE0_ASM_ADDR); /* TxFIFO offset */

		W_REG(osh, &cc->sr1_control0,
			CC_SR0_4364_SR_ENG_CLK_EN |
			CC_SR0_4364_SR_RSRC_TRIGGER |
			CC_SR0_4364_SR_WD_MEM_MIN_DIV |
			CC_SR0_4364_SR_INVERT_CLK |
			CC_SR0_4364_SR_ENABLE_HT |
			CC_SR0_4364_SR_ALLOW_PIC |
			CC_SR0_4364_SR_PMU_MEM_DISABLE);

		W_REG(osh, &cc->sr1_control1,
			CC_SR1_4364_SR_CORE1_ASM_ADDR); /* TxFIFO offset */

		if (CST4364_CHIPMODE_SDIOD(sih->chipst)) {
			si_pmu_chipcontrol(sih, 2, CC2_4364_SDIO_AOS_WAKEUP_MASK,
				(1 << CC2_4364_SDIO_AOS_WAKEUP_SHIFT));
		} else if (CST4364_CHIPMODE_PCIE(sih->chipst)) {
			si_pmu_chipcontrol(sih, 6,
				(CC6_4364_PCIE_CLKREQ_WAKEUP_MASK |
				CC6_4364_PMU_WAKEUP_ALPAVAIL_MASK),
				((1 << CC6_4364_PCIE_CLKREQ_WAKEUP_SHIFT) |
				(1 << CC6_4364_PMU_WAKEUP_ALPAVAIL_SHIFT)));
		}
	} break;

	case BCM43430_CHIP_ID: {
		/* Disable the save-restore engine */
		sr_engine_enable(sih, SRENG0, IOV_SET, FALSE);

		/* VDDM power switch enable */
		sr_chipcontrol2(sih, CC2_4350_VDDM_PWRSW_EN_MASK,
			(1 << CC2_4350_VDDM_PWRSW_EN_SHIFT));

		/* Faster wakeup */
		sr_chipcontrol6(sih, CC6_4350_PMU_WAKEUP_ALPAVAIL_MASK,
			1 << CC6_4350_PMU_WAKEUP_ALPAVAIL_SHIFT);

		/* Wakeup from always-on sdio during SR */
		sr_chipcontrol2(sih, CC2_4350_SDIO_AOS_WAKEUP_MASK,
			(1 << CC2_4350_SDIO_AOS_WAKEUP_SHIFT));

		/* Download the save-restore firmware */
		sr_download_firmware(sih);

		/* Setup SR core control registers */
		W_REG(osh, &cc->sr_control0, 0x403900F2);
		W_REG(osh, &cc->sr_control1, CC_SR1_43430_SR_ASM_ADDR);

		/* Enable the save-restore engine */
		sr_engine_enable(sih, SRENG0, IOV_SET, TRUE);

	} break;

	case BCM4347_CHIP_GRPID:
		for (i = 0; i < SRENGMAX; i ++)	{
			uint32 val;

			val = SR0_SR_ENG_CLK_EN |
				SR0_RSRC_TRIGGER |
				SR0_WD_MEM_MIN_DIV |
				SR0_MEM_STBY_ALLOW |
				SR0_ENABLE_SR_HT |
				SR0_ALLOW_PIC |
				SR0_ENB_PMU_MEM_DISABLE;

			if (i == 0 || i == 2) {
				val |= SR0_INVERT_SR_CLK;
			}

			sr_control(sih, i, 0, ~0, val);
			sr_control(sih, i, 1, ~0, sr_asm_addr(sih, i));
		}

		/* retain voltage on the SR memories */
		si_pmu_chipcontrol(sih, 2,
			CC2_4347_VASIP_MEMLPLDO_VDDB_OFF_MASK |
			CC2_4347_MAIN_MEMLPLDO_VDDB_OFF_MASK |
			CC2_4347_AUX_MEMLPLDO_VDDB_OFF_MASK |
			CC2_4347_VASIP_VDDRET_ON_MASK |
			CC2_4347_MAIN_VDDRET_ON_MASK |
			CC2_4347_AUX_VDDRET_ON_MASK,
			(0 << CC2_4347_VASIP_MEMLPLDO_VDDB_OFF_SHIFT) |
			(0 << CC2_4347_MAIN_MEMLPLDO_VDDB_OFF_SHIFT) |
			(0 << CC2_4347_AUX_MEMLPLDO_VDDB_OFF_SHIFT) |
			(1 << CC2_4347_VASIP_VDDRET_ON_SHIFT) |
			(1 << CC2_4347_MAIN_VDDRET_ON_SHIFT) |
			(1 << CC2_4347_AUX_VDDRET_ON_SHIFT));

		if (CST4347_CHIPMODE_SDIOD(sih->chipst)) {
			/* setup wakeup for SDIO save/restore */
			si_pmu_chipcontrol(sih, 2, CC2_4347_SDIO_AOS_WAKEUP_MASK,
				(1 << CC2_4347_SDIO_AOS_WAKEUP_SHIFT));

			if (CHIPREV(sih->chiprev) == 1) {
				/* WAR for A0 */
				si_pmu_chipcontrol(sih, 6, CC6_4347_PWROK_WDT_EN_IN_MASK,
					(1 << CC6_4347_PWROK_WDT_EN_IN_SHIFT));
			}
		}

		LHL_REG(sih, lhl_top_pwrseq_ctl_adr, ~0,
			LHL_4347_SLEEP_ENABLE |
			LHL_4347_PMU_SLEEP_MODE |
			LHL_4347_FINAL_PMU_SLEEP_ENABLE);

		break;

	default:
		break;
	}

	/* Return to previous core */
	si_setcoreidx(sih, origidx);

done:
	si_intrrestore(sih, intr_val);
}

#ifdef SR_ESSENTIALS
/**
 * Load the save-restore firmware into the memory for some chips, other chips use a different
 * function.
 */
void
BCMATTACHFN(sr_download_firmware)(si_t *sih)
{
	uint origidx;
	uint intr_val = 0;
	uint16 i;
	osl_t *osh = si_osh(sih);


	/* Block ints and save current core */
	intr_val = si_introff(sih);
	origidx = si_coreidx(sih);

	switch (CHIPID(sih->chip)) {
	case BCM43430_CHIP_ID: {
		void *p = (void *)SRAM_43430_SR_ASM_ADDR;
		uint32 *sr_source_code = NULL, sr_source_codesz;

		sr_get_source_code_array(sih, SRENG0, &sr_source_code, &sr_source_codesz);
		memcpy(p, sr_source_code, sr_source_codesz);

#ifdef SR_DEBUG
		printf("%s => from %p to %p size %d\n", __func__,
			sr_source_code, p, sr_source_codesz);
		if (!memcmp(p, sr_source_code, sr_source_codesz)) {
			printf("%s => Verified OK!\n", __func__);
		} else {
			printf("%s => Verification FAILED!\n", __func__);
		}
#endif
	}
	break;

	case BCM43242_CHIP_ID:
	case BCM43239_CHIP_ID: {
		sbsocramregs_t *regs;
		uint32 sr_ctrl;
		uint16 count;
		uint32 *sr_source_code = NULL, sr_source_codesz;

		sr_get_source_code_array(sih, SRENG0, &sr_source_code, &sr_source_codesz);

		/* Check if the source code array is properly aligned */
		ASSERT(ISALIGNED(sr_source_codesz, sizeof(uint32)));
		count = (sr_source_codesz/sizeof(uint32));

		/* At least one instruction should be present */
		ASSERT(count > 1);

		/* Switch to SOCRAM core */
		if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
			goto done;

		/* Save the current value of sr_control */
		sr_ctrl = R_REG(osh, &regs->sr_control);

		/* Enable access through SR address and data registers */
		W_REG(osh, &regs->sr_control, (sr_ctrl | 0x800));

		/* Write the firmware using auto-increment from location 0 */
		W_REG(osh, &regs->sr_address, 0);
		for (i = 0; i < count; i++) {
			W_REG(osh, &regs->sr_data, sr_source_code[i]);
		}

		/* Bank size expressed as 32-bit words. i.e. ((x << 10) >> 2) */
		count = ((SRCTL43239_BANK_SIZE(sr_ctrl) + 1) << 8);

		/* Total size expreseed as number of 32-bit words */
		/* Add 1 to take care of invisible bank */
		count = (SRCTL43239_BANK_NUM(sr_ctrl) + 1) * count;

		/* Zero out all the other locations. Resume from previous i */
		for (; i < count; i++) {
			W_REG(osh, &regs->sr_data, 0);
		}

		/* Disable access to SR data. Grant access to SR engine */
		W_REG(osh, &regs->sr_control, (sr_ctrl & ~(0x800)));
		break;
	}
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM4349_CHIP_GRPID:
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	CASE_BCM43602_CHIP:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	case BCM4347_CHIP_GRPID:
	{
		/* Firmware is downloaded in tx-fifo from wlc.c */
	}
	default:
		break;
	}

	/* Return to previous core */
	si_setcoreidx(sih, origidx);

done:
	si_intrrestore(sih, intr_val);
}

/**
 * Either query or enable/disable the save restore engine. Returns a negative value on error.
 * For a query, returns 0 when engine is disabled, positive value when engine is enabled.
 */
int
sr_engine_enable(si_t *sih, int sr_core, bool oper, bool enable)
{
	uint origidx;
	uint intr_val = 0;
	int ret_val = 0;
	osl_t *osh = si_osh(sih);

	/* Block ints and save current core */
	intr_val = si_introff(sih);
	origidx = si_coreidx(sih);

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID: {
		sbsocramregs_t *regs;
		uint32 sr_ctrl;

		/* Switch to SOCRAM core */
		if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
			goto done;

		/* Save current value of SR Control */
		sr_ctrl = R_REG(osh, &regs->sr_control);

		/* In 43239A0, save-restore is controlled through sr_control
		 * and certain bits of chipcontrol(phy power down)
		 */
		if (oper == IOV_GET) {
			ret_val = mboolisset(sr_ctrl, (1 << 20));
			if (ret_val != mboolisset(sr_chipcontrol(sih, 0, 0), (0x7 << 24))) {
				SR_ERROR(("%s: Error unknown saverestore state\n", __FUNCTION__));
				ret_val = BCME_ERROR;
				ASSERT(0);
				break;
			}
			ret_val &= mboolisset(sr_chipcontrol(sih, 0, 0), (0x7 << 24));
		} else if (enable) {
			W_REG(osh, &regs->sr_control, sr_ctrl | (1 << 20));
			sr_chipcontrol(sih, (0x7 << 24), (0x7 << 24));
			ret_val = enable;
		} else {
			W_REG(osh, &regs->sr_control, sr_ctrl & ~(1 << 20));
			sr_chipcontrol(sih, (0x7 << 24), 0);
			ret_val = enable;
		}
		break;
	}
	case BCM4345_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	{
		uint8 on;

		/* Use sr_isenab() for IOV_GET */
		ASSERT(oper == IOV_SET);

		on = enable ? 1 : 0;
		sr_chipcontrol(sih, CC3_SR_ENGINE_ENABLE_MASK, (on << CC3_SR_ENGINE_ENABLE_SHIFT));
	} break;
	case BCM4335_CHIP_ID:{
		if (oper == IOV_GET) {
			ret_val = sr_chipcontrol(sih, 0, 0) & 0x7;
			if (ret_val != 0x7) ret_val = 0;
		}
		else if (enable) {
		  /* regctlreg 4 [hexpr [regctlreg 4] | (1<<(159-32*4))] */
		  uint32 tmp;
		  tmp = (1<<(159-32*4));
		  sr_regcontrol4(sih, tmp, tmp);
		  sr_chipcontrol4(sih, 0xc0000000, (0xc0000000));
		  /* bit 21 = MEMLPLDO pwrsw enable */
		  /* chipctlreg 2 [hexpr ( [chipctlreg 2] | (1 << 21))] */
		  sr_chipcontrol2(sih, 0x200000, 0x200000);
		  ret_val = enable;
		} else {
			/* chipctlreg 3 [hexpr ( [chipctlreg 3] & ~0x4)] */
			sr_chipcontrol(sih, 0x4, 0x0);
			ret_val = enable;
		}
		break;
	}
	case BCM43242_CHIP_ID: {
		if (oper == IOV_GET) {
			/* get clk_en, clk_sel and sr_en bits */
			ret_val = sr_chipcontrol(sih, 0, 0) & 0x7;
			if (ret_val != 0x7)
				ret_val = 0;
		} else if (enable) {
			/* set SR-VSDB bits */
			sr_chipcontrol(sih, 0xFFFFF0F, (0x2030007));
			ret_val = enable;
		} else {
			/* reset SR-VSDB bits */
			sr_chipcontrol(sih, 0xFFFFF0F, 0);
			ret_val = enable;
		}
		break;
	}

	CASE_BCM43602_CHIP:
	case BCM4349_CHIP_GRPID:
		if (oper == IOV_GET) {
			ret_val = (sr_control(sih, sr_core, 0, 0, 0) & CC_SR0_4349_SR_ENG_EN_MASK);
			ret_val >>= CC_SR0_4349_SR_ENG_EN_SHIFT;
		} else {
			if (!BCM43602_CHIP(sih->chip)) {
				si_force_islanding(sih, enable);
			}
			sr_control(sih, sr_core, 0, CC_SR0_4349_SR_ENG_EN_MASK,
				(enable ? 1: 0) << CC_SR0_4349_SR_ENG_EN_SHIFT);
			ret_val = enable;
		}
		break;
	case BCM43012_CHIP_ID:
		if (oper == IOV_GET) {
			ret_val = sr_control(sih, SRENG0, 0, 0, 0);
		} else {
			si_force_islanding(sih, enable);
#ifndef BCMQT
			ret_val = sr_control(sih, SRENG0, 0, CC_SR_CTL0_ENABLE_MASK,
				(enable ? 1: 0) << CC_SR_CTL0_ENABLE_SHIFT);
#endif
			ret_val = enable;
		}
		break;

	case BCM4364_CHIP_ID:
	case BCM4373_CHIP_ID:
	{
		si_setcoreidx(sih, origidx);
		if (oper == IOV_GET) {
			ret_val = (sr_control(sih, SRENG0, 0, 0, 0)
				& CC_SR0_4364_SR_ENG_EN_MASK);
			ret_val >>= CC_SR0_4364_SR_ENG_EN_SHIFT;
		} else {
			sr_control(sih, SRENG0, 0, CC_SR0_4364_SR_ENG_EN_MASK,
				(enable ? 1: 0) << CC_SR0_4364_SR_ENG_EN_SHIFT);
			ret_val = enable;
		}
		break;
	}
	case BCM43430_CHIP_ID:
		if (oper == IOV_GET) {
			ret_val = (sr_control(sih, SRENG0, 0, 0, 0) & CC_SR_CTL0_ENABLE_MASK);
			ret_val >>= CC_SR_CTL0_ENABLE_SHIFT;
		} else {
			uint32 sr_enable = 0;
			uint32 subcore_pw_on = PMU_CC2_FORCE_SUBCORE_PWR_SWITCH_ON;
			if (enable)  {
				sr_enable = (1 << CC_SR_CTL0_ENABLE_SHIFT);
				subcore_pw_on =	0;
			}
			sr_control(sih, SRENG0, 0, CC_SR_CTL0_ENABLE_MASK, sr_enable);
			si_pmu_chipcontrol(sih, CHIPCTRLREG2,
				PMU_CC2_FORCE_SUBCORE_PWR_SWITCH_ON, subcore_pw_on);
			ret_val = enable;
		}
		break;

	case BCM4347_CHIP_GRPID:
		if (oper == IOV_GET) {
			ret_val = (sr_control(sih, sr_core, 0, 0, 0) & SR0_SR_ENG_EN_MASK);
			ret_val >>= SR0_SR_ENG_EN_SHIFT;
		} else {
			sr_control(sih, sr_core, 0, SR0_SR_ENG_EN_MASK,
				(enable ? 1: 0) << SR0_SR_ENG_EN_SHIFT);
			ret_val = enable;
		}
		break;

	default:
		ret_val = BCME_UNSUPPORTED;
		break;
	}

	si_setcoreidx(sih, origidx);

done:
	si_intrrestore(sih, intr_val);
	return ret_val;
} /* sr_engine_enable */
#endif /* SR_ESSENTIALS */

#if defined(SAVERESTORE)
CONST uint32*
BCMPREATTACHFNSR(sr_get_sr_params)(si_t *sih, int sr_core, uint32 *arrsz, uint32 *offset)
{
	uint32 addr;
	uint32 *sr_source_code = NULL;

	if ((CCREV(sih->ccrev) >= 48) && (CCREV(sih->ccrev) != 51)) {
		addr = sr_control(sih, sr_core, 1, 0, 0);
		addr = (addr & CC_SR_CTL1_SR_INIT_MASK) >> CC_SR_CTL1_SR_INIT_SHIFT;
	} else {
		addr = sr_chipcontrol4(sih, 0, 0);
		addr = (addr & CC4_SR_INIT_ADDR_MASK) >> CC4_SR_INIT_ADDR_SHIFT;
	}

	/* convert to txfifo byte offset */
	*offset = addr << 8;

	sr_get_source_code_array(sih, sr_core, &sr_source_code, arrsz);
	return sr_source_code;
}
#endif /* SAVERESTORE */

#if defined(SAVERESTORE)
void
sr_engine_enable_post_dnld(si_t *sih, int sr_core, bool enable)
{
	switch (CHIPID(sih->chip)) {
	case BCM4345_CHIP_ID:
	case BCM4364_CHIP_ID:
	case BCM4373_CHIP_ID:
	case BCM43909_CHIP_ID:
	CASE_BCM43602_CHIP:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	case BCM4349_CHIP_GRPID:
	case BCM43012_CHIP_ID:
	case BCM4347_CHIP_GRPID:
	{
		sr_engine_enable(sih, sr_core, IOV_SET, enable);
	} break;
	case BCM4335_CHIP_ID: {
		si_force_islanding(sih, enable);

		if (enable) {
			/* enable SR */
			/* Don't allow stby from SR (sbc_memStby_allow= bit 27 = 0) */
			/* From RTL sims chipctlreg 3 should have value 0x0F012C37
			 * but it's being changed in driver
			 */
			si_ccreg(sih, CHIPCTRLADDR, ~0, CHIPCTRLREG3);
			si_ccreg(sih, CHIPCTRLDATA, 0x07012C33, 0x07012C33);
			si_ccreg(sih, CHIPCTRLADDR, ~0, CHIPCTRLREG3);
			si_ccreg(sih, CHIPCTRLDATA, 0x4, 0x4);
			/* Increase resource up/dwn time for SR */
			si_ccreg(sih, RSRCTABLEADDR, ~0, 23);
			si_ccreg(sih, RSRCUPDWNTIME, ~0, 0x00200020);
		} else {
			/* SR is disabled */
			/* disable SR */
			si_ccreg(sih, CHIPCTRLADDR, ~0, CHIPCTRLREG3);
			si_ccreg(sih, CHIPCTRLDATA, 0x4, 0x0);
		}
	} break;
	default:
		break;
	}
}
#endif /* SAVERESTORE */

#ifdef SAVERESTORE
/** return TRUE if the power save variant of save/restore is enabled */
bool
sr_isenab(si_t *sih)
{
	bool enab = FALSE;

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		enab = (sr_chipcontrol(sih, 0, 0) & CC3_SR_ENGINE_ENABLE_MASK) ? TRUE : FALSE;
		break;

	CASE_BCM43602_CHIP: /* add chips here when CCREV >= 48 */
	case BCM4349_CHIP_GRPID:
	case BCM4364_CHIP_ID:
	case BCM4373_CHIP_ID:
	enab = ((sr_control(sih, SRENG0, 0, 0, 0) & CC_SR0_4349_SR_ENG_EN_MASK) >>
			CC_SR0_4349_SR_ENG_EN_SHIFT);
		enab = (enab ? TRUE: FALSE);
		break;

	case BCM43018_CHIP_ID:
	case BCM43430_CHIP_ID:
		enab = ((sr_control(sih, SRENG0, 0, 0, 0) & CC_SR_CTL0_ENABLE_MASK) >>
			CC_SR_CTL0_ENABLE_SHIFT);
		enab = (enab ? TRUE: FALSE);
		break;

	case BCM43012_CHIP_ID:
		enab = (sr_control(sih, SRENG0, 0, 0, 0) ? TRUE: FALSE);
		break;

	case BCM4347_CHIP_GRPID:
		enab = (((sr_control(sih, SRENG0, 0, 0, 0) & SR0_SR_ENG_EN_MASK)) &&
			((sr_control(sih, SRENG1, 0, 0, 0) & SR0_SR_ENG_EN_MASK)) &&
			((sr_control(sih, SRENG2, 0, 0, 0) & SR0_SR_ENG_EN_MASK))) ?
			TRUE : FALSE;
		break;

	case BCM43242_CHIP_ID: /* fall through, does not support SR-PS */
	default:
		ASSERT(0);
		break;
	}

	return enab;
}
#endif /* SAVERESTORE */

#ifdef SR_ESSENTIALS
/**
 * return TRUE for chips that support the power save variant of save/restore
 */
bool
sr_cap(si_t *sih)
{
	bool cap = FALSE;

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM43430_CHIP_ID:
	case BCM43018_CHIP_ID:
	case BCM43909_CHIP_ID:
	case BCM43012_CHIP_ID:
	case BCM4349_CHIP_GRPID:
	case BCM4364_CHIP_ID:
	case BCM4373_CHIP_ID:
	case BCM4347_CHIP_GRPID:
	CASE_BCM43602_CHIP:
		cap = TRUE;
		break;
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM4358_CHIP_ID:
		/* 4350A0 is broken */
		if (CHIPREV(sih->chiprev) > 0)
			cap = TRUE;
		break;
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
		cap = TRUE;
		break;
	case BCM43242_CHIP_ID: /* does not support SR-PS, return FALSE */
		break;
	case BCM4335_CHIP_ID:
		if (BUSTYPE(sih->bustype) != PCI_BUS)
			cap = TRUE;
		break;
	default:
		break;
	}

	return cap;
}
#endif /* SR_ESSENTIALS */

#if defined(SAVERESTORE)
uint32
BCMATTACHFN(sr_register_save)(si_t *sih, sr_save_callback_t cb, void *arg)
{
	uint8 i;
	sr_s_client_info_t *sr_save_info;

	if (sr_cap(sih) == FALSE)
		return BCME_ERROR;

	for (i = 0; i < SR_MAX_CLIENT; i++) {
		sr_save_info = sr_get_save_info(i);
		if (sr_save_info->cb == NULL) {
			sr_save_info->cb = cb;
			sr_save_info->arg = arg;
			return 0;
		}
	}

	return BCME_ERROR;
}

uint32
BCMATTACHFN(sr_register_restore)(si_t *sih, sr_restore_callback_t cb, void *arg)
{
	uint8 i;
	sr_r_client_info_t *sr_restore_info;

	if (sr_cap(sih) == FALSE)
		return BCME_ERROR;

	for (i = 0; i < SR_MAX_CLIENT; i++) {
		sr_restore_info = sr_get_restore_info(i);
		if (sr_restore_info->cb == NULL) {
			sr_restore_info->cb = cb;
			sr_restore_info->arg = arg;
			return 0;
		}
	}

	return BCME_ERROR;
}

void
sr_process_save(si_t *sih)
{
	bool sr_enable_ok = TRUE;
	uint8 i;
	sr_s_client_info_t *sr_save_info;

	if (sr_cap(sih) == FALSE) {
		return;
	}

	for (i = 0; i < SR_MAX_CLIENT; i++) {
		sr_save_info = sr_get_save_info(i);
		if (sr_save_info->cb != NULL && !sr_save_info->cb(sr_save_info->arg)) {
			sr_enable_ok = FALSE;
			break;
		}
	}

	if (sr_isenab(sih) != sr_enable_ok) {
		sr_engine_enable(sih, SRENG0, IOV_SET, sr_enable_ok);
	}
}

void
sr_process_restore(si_t *sih)
{
	uint origidx;
	uint intr_val = 0;
	uint8 i;
	sr_r_client_info_t *sr_restore_info;

	if (sr_cap(sih) == FALSE)
		return;

	sr_update_deep_sleep(sih);

	/* Block ints, save current core and switch to common core */
	intr_val = si_introff(sih);
	origidx = si_coreidx(sih);

	/* If it is a 4334 or a variant we can test if the hardware actually
	   went into deep sleep.  If so clear indication/log bit for next time
	   before letting any registered modules do their restore processing.
	 */
	if (!sr_is_wakeup_from_deep_sleep()) goto exit;

	/* Note that PSM/ucode will resume execution at the same time as
	 * the ARM and you need to consider any possible race conditions.
	 *
	 * Any wl related registered, restore function should consider
	 * whether the PSM might use resources it is restoring before we
	 * are done restoring, e.g.. RCMTA table.
	 *
	 * Similarly, you also have to consider whether we update, use
	 * or otherwise depend on resources which PSM might be restoring.
	 * If so it might make sense to first synchronize with the PSM by
	 * "suspending the MAC".
	 */
	for (i = 0; i < SR_MAX_CLIENT; i++) {
		sr_restore_info = sr_get_restore_info(i);
		if (sr_restore_info->cb != NULL) {
			sr_restore_info->cb(sr_restore_info->arg);
		}
	}

exit:
	si_setcoreidx(sih, origidx);
	si_intrrestore(sih, intr_val);
}

static void sr_update_deep_sleep(si_t *sih)
{
	osl_t *osh = si_osh(sih);

	sr_wokeup_from_deep_sleep(FALSE);

	if (si_pmu_reset_ret_sleep_log(sih, osh))
		sr_wokeup_from_deep_sleep(TRUE);
}

static void sr_wokeup_from_deep_sleep(bool state)
{
	g_wakeup_from_deep_sleep = state;
	if (state) {
		g_num_wakeup_from_deep_sleep ++;
#ifdef SR_DEBUG
		if (!(g_num_wakeup_from_deep_sleep % 10))
			printf("g_num_wakeup_from_deep_sleep = %d\n", g_num_wakeup_from_deep_sleep);
#endif
	}
}

static bool sr_is_wakeup_from_deep_sleep()
{
	return g_wakeup_from_deep_sleep;
}

#endif /* SAVERESTORE */
