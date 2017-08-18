/*
 * FCBS data related to SDIO core
 *
 * Broadcom Proprietary and Confidential. Copyright (C) 2017,
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom.
 *
 * $Id: sdpcmdev.c 611926 2016-01-12 13:11:32Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <osl.h>
#include <sdpcmdev.h>
#include <ulp.h>
#include <fcbsutils.h>
#include <fcbs.h>
#include <sbsdpcmdev.h>
#include <fcbsdata.h>
#include <hndsoc.h>

#define SDIO_FCBS_NUM_SEQ	5
#define FCBS_SR			0xFF

typedef struct fcbs_input_fbr {
	uint16 fbr;
	uint8 offset;
	uint8 data;
} fcbs_input_fbr_t;

/* SDIO CCCR, FBR and Function registers */
static const fcbs_input_fbr_t fi_fbr[] = {
	{SDA_CCCR_SPACE, 0x02, 0x02},
	{SDA_CCCR_SPACE, 0x03, 0x02},
	{SDA_CCCR_SPACE, 0x04, 0x03},
	{SDA_CCCR_SPACE, 0x07, FCBS_SR},
	{SDA_CCCR_SPACE, 0x08, FCBS_SR},
	{SDA_CCCR_SPACE, 0x13, FCBS_SR},
	{SDA_CCCR_SPACE, 0x15, FCBS_SR},
	{SDA_CCCR_SPACE, 0x16, FCBS_SR},
	{SDA_CCCR_SPACE, 0xF0, FCBS_SR},
	{SDA_CCCR_SPACE, 0xF1, FCBS_SR},
	{SDA_CCCR_SPACE, 0xF2, FCBS_SR},

	{SDA_F1_FBR_SPACE, 0x10, FCBS_SR},
	{SDA_F1_FBR_SPACE, 0x11, FCBS_SR},

	{SDA_F2_FBR_SPACE, 0x10, FCBS_SR},
	{SDA_F2_FBR_SPACE, 0x11, FCBS_SR},

	{SDA_F3_FBR_SPACE, 0x10, FCBS_SR},
	{SDA_F3_FBR_SPACE, 0x11, FCBS_SR},

	{SDA_F1_REG_SPACE, 0x01, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x05, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x06, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x07, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x08, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x09, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x0d, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x0f, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x1d, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x1e, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x20, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x21, FCBS_SR},
	{SDA_F1_REG_SPACE, 0x22, FCBS_SR},
};

/* SDIO Enumeration registers */
static const uint32 enum_reg_offsets[] = {
	OFFSETOF(sdpcmd_regs_t, corecontrol),
	OFFSETOF(sdpcmd_regs_t, hostintmask),
	OFFSETOF(sdpcmd_regs_t, intmask),
	OFFSETOF(sdpcmd_regs_t, sbintmask),
	OFFSETOF(sdpcmd_regs_t, funcintmask),
	OFFSETOF(sdpcmd_regs_t, MiscHostAccessIntEn),
	OFFSETOF(sdpcmd_regs_t, intrcvlazy),
	OFFSETOF(sdpcmd_regs_t, clockctlstatus),
	OFFSETOF(sdpcmd_regs_t, powerctl)
};

/* SDIO core initialization */
static const fcbs_ad_sz_t fcbs_sdio_init[] = {
	{0x18102800, 4, 0x00000001},	/* SDIO backplane reset */
	FCBS_DELAY_TUPLE(0),
	{0x18102408, 4, 0x00000005},	/* SDIO clock domain reset */
	FCBS_DELAY_TUPLE(0),
	{0x18102800, 4, 0x00000000},	/* SDIO back plane reset removal */
	FCBS_DELAY_TUPLE(0),
	{0x18002300, 4, 0x0000001D},	/* Assert sync_stop, assert ht/alp/ilp clock selects */
	FCBS_DELAY_TUPLE(0),
	{0x18002300, 4, 0x0000001F},	/* Assert async_stop */
	FCBS_DELAY_TUPLE(0),
	{0x18002300, 4, 0x0000001D},	/* De-assert async_stop */
	FCBS_DELAY_TUPLE(0),
	{0x18002300, 4, 0x0000001C},	/* De-assert sync_stop */
	FCBS_DELAY_TUPLE(0),
	{0x18002300, 4, 0x0000007C},	/* Enable Fast OTP and Fast SDIO init enables and
					 * set HT clock as SDIO clock
					 */
	FCBS_DELAY_TUPLE(0),
	{0x18002304, 4, 0x00009100},	/* OTP Status shawdow register.
					 *  - Bit '8' indicates OTP present. Disable this bit
					 *    to bypass OTP.
					 *  - So for OTP bypass, program this register as 0x9000
					 */
	{0x18002308, 4, 0x9011B200},	/* OTP Layout shawdow register */
	{0x1800230C, 4, 0xFFFF0000},	/* OTP Shadow register 1 */
	{0x18002310, 4, 0xFFFF6000},	/* OTP Shadow register 2 */
	{0x18002314, 4, 0xCAFEBEAD},	/* OTP Shadow register 3 */
	{0x18102408, 4, 0x00000001},	/* SDIO clock domain reset removal */
	FCBS_DELAY_TUPLE(0),
	{0x18102408, 4, 0x00000001},	/* Same as above, duplicated here for some extra delay */
	{INITVALS_END_MARKER, 0, 0}
};

static const fcbs_ad_sz_t fcbs_sdio_init_post[] = {
	{0x18002300, 4, 0x20003060},	/* Force SW Ready */
	{INITVALS_END_MARKER, 0, 0}
};

int
fcbsdata_sdio_populate(si_t *sih)
{
	sdpcmd_regs_t *regs;
	fcbs_input_bp_t *fa, *fb;
	fcbs_input_data_t *fcbs_sdio_input_data;
	int i = 0;
	int err = BCME_OK;
	uint32 reg_val;
	osl_t *osh = si_osh(sih);
	uint origidx = si_coreidx(sih);

	regs = si_setcore(sih, SDIOD_CORE_ID, 0);

	if ((fcbs_sdio_input_data =
		MALLOCZ(osh, SDIO_FCBS_NUM_SEQ * sizeof(fcbs_input_data_t))) == NULL) {
		ULP_ERR(("wl: %s: out of mem, malloced %d bytes\n", __func__, MALLOCED(osh)));
		err = BCME_NOMEM;
		goto exit;
	}

	if ((fa = MALLOCZ(osh,
		(sizeof(fcbs_input_bp_t) + ARRAYSIZE(fi_fbr) * sizeof(fcbs_input_ad_t)))) == NULL) {
		ULP_ERR(("wl: %s: out of mem, malloced %d bytes\n", __func__, MALLOCED(osh)));
		err = BCME_NOMEM;
		goto exit_fa;
	}

	if ((fb = MALLOCZ(osh,
		(sizeof(fcbs_input_bp_t) +
		ARRAYSIZE(enum_reg_offsets) * sizeof(fcbs_input_ad_t)))) == NULL) {
		ULP_ERR(("wl: %s: out of mem, malloced %d bytes\n", __func__, MALLOCED(osh)));
		err = BCME_NOMEM;
		goto exit_fb;
	}

	fa->addr_hi = ADDR_HI((int)&regs->sdioaccess);
	fa->data = (fcbs_ad_t *)(fa + 1);

	/* Save SDIO F0 registers */
	for (i = 0; i < ARRAYSIZE(fi_fbr); i++) {
		fa->data[i].addr = (int)&regs->sdioaccess;
	}

	for (i = 0; i < ARRAYSIZE(fi_fbr); i++) {
		if (fi_fbr[i].data == FCBS_SR) {
		sdpcmd_sdioaccess(&fa->data[i].data, fi_fbr[i].fbr, fi_fbr[i].offset,
			SDA_READ, osh, regs);
		} else {
			fa->data[i].data = 0x81000000 | ((fi_fbr[i].fbr + fi_fbr[i].offset)<< 8) |
				fi_fbr[i].data;
		}
	}

	ULP_DBG(("\n SDIO CCCR, FBR and Function registers:\n"));
	for (i = 0; i < ARRAYSIZE(fi_fbr); i++) {
		ULP_DBG(("[0x%02x] [0x%02x] fi->func_addr[%d] = %x\n",
				fi_fbr[i].fbr, fi_fbr[i].offset, i, fa->data[i].data));
	}

	/* Save SDIO Enumeration registers */
	fb->addr_hi = ADDR_HI((int)&regs->sdioaccess);
	fb->data = (fcbs_ad_t *)(fb + 1);

	for (i = 0; i < ARRAYSIZE(enum_reg_offsets); i++) {
		reg_val = R_REG(osh, (uint32*)((uint32)regs + enum_reg_offsets[i]));

		/* Clearing F2 ready bit */
		if (enum_reg_offsets[i] == OFFSETOF(sdpcmd_regs_t, corecontrol)) {
			reg_val &= ~0x4;
		}

		fcbs_create_addr_data_pairs(fb, i, (int)regs + enum_reg_offsets[i], reg_val);
	}

	ULP_DBG(("SDIO Enumeration Registers:\n"));
	for (i = 0; i < ARRAYSIZE(enum_reg_offsets); i++) {
		ULP_DBG(("addr = 0x%04x, data = 0x%04x\n", fb->data[i].addr, fb->data[i].data));
	}

	i = 0;

	/* FCBS input data packing */
	fcbs_data_bp(fcbs_sdio_input_data, i++, 0, FCBS_DATA_BP_SZ, (void *)fcbs_sdio_init);
	fcbs_data_bp(fcbs_sdio_input_data, i++, ARRAYSIZE(fi_fbr), 0, fa);
	fcbs_data_bp(fcbs_sdio_input_data, i++, ARRAYSIZE(enum_reg_offsets), 0, fb);
	fcbs_data_bp(fcbs_sdio_input_data, i++, 0, FCBS_DATA_BP_SZ, (void *)fcbs_sdio_init_post);
	fcbs_data_end(fcbs_sdio_input_data, i++);

	if (i > SDIO_FCBS_NUM_SEQ) {
		ULP_ERR(("%s: Allocated insufficient memory!\n", __func__));
		err = BCME_ERROR;
		goto exit_free_all;
	}

	ulp_fcbs_add_dynamic_seq(fcbs_sdio_input_data, FCBS_DS1_EXIT_BLOCK,
		FCBS_DS1_EXIT_BLOCK_SEQ0);
	goto exit;

exit_free_all:
	MFREE(osh, fb, (sizeof(fcbs_input_bp_t) +
			ARRAYSIZE(enum_reg_offsets) * sizeof(fcbs_input_ad_t)));
exit_fb:
	MFREE(osh, fa, (sizeof(fcbs_input_bp_t) + ARRAYSIZE(fi_fbr) * sizeof(fcbs_input_ad_t)));
exit_fa:
	MFREE(osh, fcbs_sdio_input_data, (SDIO_FCBS_NUM_SEQ * sizeof(fcbs_input_data_t)));
exit:
	si_setcoreidx(sih, origidx);
	return err;
}
