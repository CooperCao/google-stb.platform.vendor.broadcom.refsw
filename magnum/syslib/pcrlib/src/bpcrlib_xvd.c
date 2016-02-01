/***************************************************************************
 *     Copyright (c) 2003-2007, Broadcom Corporation
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
#include "bpcrlib_xvd.h"
#include "bxvd.h"
#ifndef BCHP_7411_VER
#include "bxpt_pcr_offset.h"
#endif

BDBG_MODULE(pcrlib);

static BERR_Code 
BPCRlib_Video_GetStc_XVD_isr(void *trp, void *dec, uint32_t *stc)
{
	BSTD_UNUSED(dec);
	BDBG_MSG(("VID_XVD:GetStc %#x %#x", (unsigned)trp, (unsigned)dec));
#ifdef BCHP_7411_VER
	return BARC_GetSTC_isr(trp, stc);
#else
	*stc = BXPT_PcrOffset_GetStc_isr(trp) + BXPT_PcrOffset_GetOffset_isr(trp);
	return BERR_SUCCESS;
#endif
}

static BERR_Code 
BPCRlib_Video_GetPts_XVD_isr(void *dec, BAVC_PTSInfo *pts)
{
	BXVD_PTSInfo			PTSInfo;
	BERR_Code rc;
	const BPCRlib_Xvd_Decoder *xvd=dec;

	BDBG_MSG(("VID_XVD:GetPts %#x", (unsigned)dec));
	rc = BXVD_GetPTS_isr(xvd->dec, &PTSInfo);
#ifdef BCHP_7411_VER
	pts->ui32CurrentPTS = PTSInfo.ulCurrentPTS;
#else
	pts->ui32CurrentPTS = PTSInfo.ui32RunningPTS;
#endif
	pts->ePTSType = PTSInfo.ePTSType;
	
	return rc;
}

BERR_Code 
BPCRlib_Video_GetCdbLevel_XVD_isr(void *dec, unsigned *level)
{
	const BPCRlib_Xvd_Decoder *xvd=dec;
#ifdef BCHP_7411_VER
	unsigned size;
	return BXVD_GetCPBInfo_isr(xvd->dec, &size, level);
#else
	BXPT_Rave_BufferInfo buffer_info;
	BERR_Code rc;

	BDBG_ASSERT(xvd->rave);
	rc = BXPT_Rave_GetBufferInfo_isr(xvd->rave, &buffer_info);
	if (rc==BERR_SUCCESS) {
		*level = buffer_info.CdbDepth;
	}
	return rc;
#endif
}

static BERR_Code 
BPCRlib_Video_SetStc_XVD_isr(void *trp, void *dec, bool dss,uint32_t stc)
{
	const BPCRlib_Xvd_Decoder *xvd=dec;
	BERR_Code rc = BERR_SUCCESS;
	BSTD_UNUSED(dss);
	BSTD_UNUSED(xvd);

	BDBG_MSG(("VID_XVD:SetStc %#x %#x", (unsigned)trp, (unsigned)dec));
#ifdef BCHP_7411_VER
	BSTD_UNUSED(dec);
	BSTD_UNUSED(xvd);
	rc = BARC_SetSTC_isr(trp, stc);
#else
	{
		uint32_t current_stc;
		int32_t diff;
	
		current_stc = BXPT_PcrOffset_GetStc_isr(trp);
		diff = (int32_t)current_stc - (int32_t)stc;
		/* If the STC is already very close, there's no point in resetting it. This	
		prevents PTS Errors from raptor which tries to follow a tight TSM threshold. */
		if (diff > 100 || diff < -100) {
			/* assume that PCR_OFFSET offset was set to 0 by host before starting PVR. */
			rc = BXPT_PcrOffset_SetStc_isr(trp, stc);
		}
	}
	
	/* notify decoders that STC is now valid */
	if (rc==BERR_SUCCESS) {
		rc = BXVD_SetSTCInvalidFlag_isr(xvd->dec, false);
	}
#endif

	return rc;
}

static BERR_Code 
BPCRlib_Video_UpdateStc_XVD_isr(void *trp, bool is_request_stc)
{
	BDBG_MSG(("XVD::UpdateStc %#x", (unsigned)trp));
#ifdef BCHP_7411_VER
	BSTD_UNUSED(is_request_stc);
	return BARC_UpdateSTC_isr(trp);
#else
	if (is_request_stc) {
		/* If PCR_OFFSET block has non-zero OFFSET_THRESHOLD, then it needs
		to be forced to regenerate an offset message to RAVE. Otherwise the decoder
		may lose a pcr_offset_valid message. */
		BXPT_PcrOffset_RegenOffset_isr(trp);
	}
	return 0;
#endif
}


const BPCRlib_StcDecIface BPCRlib_Video_Xvd = {
	BPCRlib_Video_GetPts_XVD_isr,
	BPCRlib_Video_GetStc_XVD_isr,
	BPCRlib_Video_GetCdbLevel_XVD_isr,
	BPCRlib_Video_SetStc_XVD_isr, 
	BPCRlib_Video_UpdateStc_XVD_isr, 
	true
};

