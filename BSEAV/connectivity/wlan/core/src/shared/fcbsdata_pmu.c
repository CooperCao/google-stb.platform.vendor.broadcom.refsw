/*
 * FCBS data for PMU Up Trigger FCBS sequence
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
#include <ulp.h>
#include <fcbsutils.h>
#include <fcbs.h>
#include <fcbsdata.h>
#include <hndpmu.h>
#include <hndsoc.h>

#define FCBS_SR			0xabcdabcd

static fcbs_ad_sz_t fcbs_up_trig[] = {
	{0x18001120, 4, 0x42020400},	/* Enable driver access to IHR */
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	/* BMC initialisations for TID 7 */
	{TXE_BMC_CONFIG, 2, TXE_BMC_MARKER}, /* TXE_BMCConfig */
	{TXE_BMC_CMD, 2, BMC_TPL_IDX}, /* TXE_BMCCmd */
	{TXE_BMC_MAXBUFFERS, 2, TPLBUF}, /* TXE_BMCMaxBuffers */
	{TXE_BMC_MINBUFFERS, 2, TPLBUF}, /* TXE_BMCMinBuffers */
	{TXE_BMC_CMD, 2, TXE_BMC_CMD_VAL}, /* TXE_BMCCmd */
	/* Link the buffers - the polling of bit 0 to become 0 is done in UCODE */
	{TXE_BMC_LINK, 2, TXE_BMC_LINK_VAL},
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	{0x180011ac, 4, FCBS_SR},	/* psm_patchcopy_ctrl [SR] */
	{0x180011ac, 4, FCBS_SR},	/* psm_patchcopy_ctrl [SR] */
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	{0x18001120, 4, 0x42020507},	/* PSM jump to 0 */
	FCBS_DELAY_TUPLE(32),
	{0x18001120, 4, 0x42020503},	/* PSM run */
	{0x18000024, 4, 0x90},	/* Enable GCI interrupt to trigger CC interrupt for SDIO wake up */
	{0x18001496, 4, 0xFF},	/* Configure MAC/Ucode to receive interrupt for SDIO wake up */
	{INITVALS_END_MARKER, 0, 0}
};

int
fcbsdata_pmuuptrigger_populate(si_t *sih)
{
	fcbs_out_t fo;
	fcbs_input_data_t fid;
	fcbs_ad_sz_t *fcbs_up_trigger;
	d11regs_t *regs;
	int i = 0;
	int err = BCME_OK;
	osl_t *osh = si_osh(sih);
	uint origidx = si_coreidx(sih);

	regs = si_setcore(sih, D11_CORE_ID, 0);

	if ((fcbs_up_trigger =
		MALLOCZ(osh, ARRAYSIZE(fcbs_up_trig) * sizeof(fcbs_ad_sz_t))) == NULL) {
		ULP_ERR(("wl: %s: out of mem, malloced %d bytes\n", __func__, MALLOCED(osh)));
		err = BCME_NOMEM;
		goto exit;
	}

	for (i = 0; i < ARRAYSIZE(fcbs_up_trig); i++) {
		fcbs_up_trigger[i].addr = fcbs_up_trig[i].addr;
		fcbs_up_trigger[i].size = fcbs_up_trig[i].size;
		if (fcbs_up_trig[i].value == FCBS_SR) {
			fcbs_up_trigger[i].value = R_REG(sih, (uint32 *)fcbs_up_trig[i].addr);
		}
		else if (fcbs_up_trig[i].value == TXE_BMC_MARKER) {
			uint32 machwcap = R_REG(sih, &regs->machwcap);
			fcbs_up_trigger[i].value = TXFIFO_SZ(machwcap) >>
				(8 + BMC_BUFSIZE_256BLOCK);
		}
		else
			fcbs_up_trigger[i].value = fcbs_up_trig[i].value;
	}

	fid.type = FCBS_BP_ACCESS;
	fid.data_size = 0; /* Size calculated by fcbsutils.c */
	fid.flags = FCBS_DATA_BP_SZ;
	fid.data = (void *)fcbs_up_trigger;

	fo = fcbs_populate(&fid, 1, FCBS_DS1);

	/* Program FCBS UP trigger cmd and dat addresses */
	si_pmu_chipcontrol(sih, PMU_CHIPCTL11,
			~0x0, FCBS_TPL(fo.cmd_start) | (FCBS_TPL(fo.dat_start) << 16));

	/* Enable FCBS UP trigger */
	si_pmu_chipcontrol(sih, PMU_CHIPCTL13,
			PMUCCTL13_43012_FCBS_UP_TRIG_EN, PMUCCTL13_43012_FCBS_UP_TRIG_EN);
exit:
	si_setcoreidx(sih, origidx);
	return err;
}
