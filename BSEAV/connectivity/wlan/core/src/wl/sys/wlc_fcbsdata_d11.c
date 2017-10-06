/*
 * FCBS data related to D11 core
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

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <osl.h>
#include <ulp.h>
#include <fcbsutils.h>
#include <fcbs.h>
#include <fcbsdata.h>
#include <d11_addr_space.h>
#include <wlioctl.h>
#include <wlc.h>
#include <wlc_addrmatch.h>
#include <wlc_bmac.h>
#include <hndsoc.h>

#define AMT_ADDR_DATA_PAIR	2
#define FCBS_ULP_NUM_SEQ	6

static const fcbs_ad_sz_t fcbs_ihr[] = {
	{0xE8000540, 2, 0x0000},			/* BMC Control register */
	{0xE80006A8, 2, DS1_43012A0_FAST_PWRUP_DELAY},	/* Fast power up delay */
	{INITVALS_END_MARKER, 0, 0}
};

/* FCBS sequence for enabling MAC clock gating */
static const fcbs_ad_sz_t fcbs_macclk_gating[] = {
	{0xE8000AE0, 2, 0xFFFF},
	{0xE8000AE2, 2, 0xFFFF},
	{0xE8000AE4, 2, 0x03CF},
	{0xE8000AE6, 2, 0x0000},
	{0xE8000AE8, 2, 0x0000},
	{0xE8000AEA, 2, 0x0000},
	{0xE8000AEC, 2, 0x0000},
	{0xE8000AEE, 2, 0x2000},
	{0xE8000AF0, 2, 0x007F},
	{0xE8000AF2, 2, 0x0027},	/* Set MAC ILP clock to 10Mhz */
	{0xE8000AF4, 2, 0x7CEF},
	{0xE8000AF8, 2, 0x0030},
	{0xE8000AFA, 2, 0x0003},
	{0xE8000AFC, 2, 0x0000},
	{0xE8000AFE, 2, 0x0000},
	{0xE8000AF6, 2, 0x0000},
	{INITVALS_END_MARKER, 0, 0}
};

int
fcbsdata_d11_populate(si_t *sih, void *wlc_hw, void *wlc, bool mac_clkgating)
{
	int i, k = 0;
	struct ether_addr ea;
	uint16 attr;
	uint32 word[2];
	struct ether_addr prev_addr;
	uint16 prev_attr  = 0;
	int amt_count;
	d11regs_t *regs;
	osl_t *osh = si_osh(sih);
	fcbs_input_bp_t *fcbs_amt, *fcbs_bcn_int, *fcbs_chipc;
	fcbs_input_data_t *fcbs_ulp_data;

	int err = BCME_OK;
	uint32 axi_base = si_get_d11_slaveport_addr(sih,
		D11_AXI_SP_IDX, CORE_BASE_ADDR_0, 0);
	amt_count = wlc_get_valid_amt_count(wlc);
	regs = si_setcore(sih, D11_CORE_ID, 0);

	if ((fcbs_ulp_data =
		MALLOCZ(osh, FCBS_ULP_NUM_SEQ * sizeof(fcbs_input_data_t))) == NULL) {
		ULP_ERR(("wl: %s: (fcbs_amt_data) out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		err = BCME_NOMEM;
		goto exit;
	}

	if ((fcbs_amt = MALLOCZ(osh,
		(sizeof(fcbs_input_bp_t) +
		(amt_count * AMT_ADDR_DATA_PAIR * sizeof(fcbs_ad_t))))) == NULL) {
		ULP_ERR(("wl: %s: (fcbs_amt) out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		err = BCME_NOMEM;
		goto exit_fcbs_amt;
	}

	if ((fcbs_bcn_int = MALLOCZ(osh,
		(sizeof(fcbs_input_bp_t) + sizeof(fcbs_ad_t)))) == NULL) {
		ULP_ERR(("wl: %s: (fcbs_bcn_int) out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		err = BCME_NOMEM;
		goto exit_fcbs_bcn_int;
	}

	if ((fcbs_chipc = MALLOCZ(osh,
		(sizeof(fcbs_input_bp_t) + sizeof(fcbs_ad_t)))) == NULL) {
		ULP_ERR(("wl: %s: (fcbs_chipc) out of mem, malloced %d bytes\n",
			__FUNCTION__, MALLOCED(osh)));
		err = BCME_NOMEM;
		goto exit_fcbs_chipc;
	}

	/* FCBS data: AMT related */
	fcbs_amt->addr_hi = ADDR_HI(axi_base);
	fcbs_amt->data = (fcbs_ad_t *)(fcbs_amt + 1);
	for (i = 0; i < ((wlc_info_t *)wlc)->pub->max_addrma_idx; i++) {
		wlc_get_addrmatch(wlc, i, &ea, &attr);
		if (ETHER_ISNULLADDR(&ea) && !(attr & AMT_ATTR_VALID))
			continue;
		wlc_bmac_read_amt(wlc_hw, i, &prev_addr, &prev_attr);

		if (attr & AMT_ATTR_VALID) {
			attr |= prev_attr;
		} else {
			attr = 0;
		}

		word[0] = (ea.octet[3] << 24) |
		        (ea.octet[2] << 16) |
		        (ea.octet[1] << 8) |
		        ea.octet[0];
		word[1] = (attr << 16) |
		        (ea.octet[5] << 8) |
		        ea.octet[4];
		fcbs_create_addr_data_pairs(fcbs_amt, k++,
			AXISL_AMT_BASE + ((int)(i * 2) << 2), (int)word[0]);
		fcbs_create_addr_data_pairs(fcbs_amt, k++,
			AXISL_AMT_BASE +((int)(i * 2) << 2) + sizeof(int), (int)word[1]);
	}

	/* FCBS data: Beacon related */
	fcbs_bcn_int->addr_hi = 0x1800;
	fcbs_bcn_int->data = (fcbs_ad_t *)(fcbs_bcn_int + 1);
	fcbs_create_addr_data_pairs(fcbs_bcn_int, 0,
		(int)regs + OFFSETOF(d11regs_t, tsf_cfprep),
		R_REG(osh, (uint32*)((uint32)regs + OFFSETOF(d11regs_t, tsf_cfprep))));

	/* FCBS data: ChipCommon related */
	fcbs_chipc->addr_hi = 0x1800;
	fcbs_chipc->data = (fcbs_ad_t *)(fcbs_chipc + 1);
	fcbs_create_addr_data_pairs(fcbs_chipc, 0, (int)0x01E0, 0x00000040);

	i = 0;
	/* FCBS input data packing */
	fcbs_data_bp(fcbs_ulp_data, i, AMT_ADDR_DATA_PAIR * amt_count, 0, fcbs_amt);
	fcbs_data_bp(fcbs_ulp_data, ++i, 1, 0, fcbs_bcn_int);
	fcbs_data_bp(fcbs_ulp_data, ++i, 0, FCBS_DATA_BP_SZ, (void *)fcbs_ihr);
	if (mac_clkgating) {
		fcbs_data_bp(fcbs_ulp_data, ++i, 0, FCBS_DATA_BP_SZ, (void *)fcbs_macclk_gating);
	}
	fcbs_data_bp(fcbs_ulp_data, ++i, 1, 0, fcbs_chipc);
	fcbs_data_end(fcbs_ulp_data, ++i);
	if (i > FCBS_ULP_NUM_SEQ) {
		ULP_ERR(("%s: Allocated insufficient memory!\n", __func__));
		err = BCME_ERROR;
		goto exit_free_all;
	}

	ulp_fcbs_add_dynamic_seq(fcbs_ulp_data, FCBS_DS1_MAC_INIT_BLOCK,
			FCBS_DYN_MAC_INIT_BLK_AMT_SEQ);
	goto exit;

exit_free_all:
	MFREE(osh, fcbs_chipc, (sizeof(fcbs_input_bp_t) + sizeof(fcbs_ad_t)));
exit_fcbs_chipc:
	MFREE(osh, fcbs_bcn_int, (sizeof(fcbs_input_bp_t) + sizeof(fcbs_ad_t)));
exit_fcbs_bcn_int:
	MFREE(osh, fcbs_amt, (sizeof(fcbs_input_bp_t) +
			(amt_count * AMT_ADDR_DATA_PAIR * sizeof(fcbs_ad_t))));
exit_fcbs_amt:
	MFREE(osh, fcbs_ulp_data, (FCBS_ULP_NUM_SEQ * sizeof(fcbs_input_data_t)));
exit:
	return err;
}
