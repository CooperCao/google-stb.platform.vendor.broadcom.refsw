/***************************************************************************
 *     Copyright (c) 2003-2006, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bpcrlib.h"
#include "bpcrlib_aud.h"
#include "baud.h"
#include "baud_dec.h"
#include "bxpt_directv_pcr.h"

BDBG_MODULE(pcrlib);


static BERR_Code 
BPCRlib_Audio_GetStc_AUD_isr(void *trp, void *dec, uint32_t *stc)
{
	BAUD_DEC_TimeInfo time;
	BERR_Code rc;

	BSTD_UNUSED(trp);

	BDBG_MSG(("AUD_AUD:GetStc %#x %#x", (unsigned)trp, (unsigned)dec));
	rc = BAUD_DEC_GetCurrentTimeInfo_isr(dec, &time);
	if (rc==BERR_SUCCESS) {
		*stc = time.uiStc;
	}
	return rc;
}

static BERR_Code 
BPCRlib_Audio_GetPts_AUD_isr(void *dec, BAVC_PTSInfo *pts)
{
	BAUD_DEC_TimeInfo time;
	BERR_Code rc;

	BDBG_MSG(("AUD_AUD:GetPts %#x", (unsigned)dec));
	rc = BAUD_DEC_GetCurrentTimeInfo_isr(dec, &time);
	if (rc==BERR_SUCCESS) {
		pts->ui32CurrentPTS = time.uiPts;
		pts->ePTSType = BAVC_PTSType_eCoded;
	}
	return rc;
}

static BERR_Code 
BPCRlib_Audio_GetCdbLevel_AUD_isr(void *dec, unsigned *level)
{
	BSTD_UNUSED(dec);
	*level = 0;
	return BERR_SUCCESS;
}

static BERR_Code 
BPCRlib_Audio_SetStc_AUD_isr(void *trp, void *dec, bool dss, uint32_t stc)
{
	BERR_Code rc;

	BSTD_UNUSED(dec);

	if (dss) {
		BDBG_MSG(("updating XPT: DSS stc %#x", stc));
		rc = BXPT_PCR_DirecTv_SendPcr_isr(trp, stc);
		if (rc!=BERR_SUCCESS) { BDBG_ERR(("BXPT_PCR_DirecTv_SendPcr_isr returned error %#x, ignored", rc)); }
	} else {
		BDBG_MSG(("updating XPT: stc %#x", stc));
		rc = BXPT_PCR_SendPcr_isr(trp, stc, 0);
		if (rc!=BERR_SUCCESS) { BDBG_ERR(("BXPT_PCR_SendPcr_isr returned error %#x, ignored", rc)); }
	}
	return rc;
}

static BERR_Code 
BPCRlib_Audio_UpdateStc_AUD_isr(void *trp, bool is_request_stc)
{
	BSTD_UNUSED(is_request_stc);
	BDBG_MSG(("AUD_AUD:Update STC %#x", (unsigned)trp));
	BXPT_PCR_RefreshPcrPid_isr(trp);
	return BERR_SUCCESS;
}


const BPCRlib_StcDecIface BPCRlib_Audio_Aud = {
	BPCRlib_Audio_GetPts_AUD_isr,
	BPCRlib_Audio_GetStc_AUD_isr,
	BPCRlib_Audio_GetCdbLevel_AUD_isr,
	BPCRlib_Audio_SetStc_AUD_isr,
	BPCRlib_Audio_UpdateStc_AUD_isr,
	false
};





