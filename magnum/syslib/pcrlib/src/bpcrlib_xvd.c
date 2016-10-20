/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ***************************************************************************/

#include "bstd.h"
#include "bpcrlib.h"
#include "bpcrlib_xvd.h"
#include "bxvd.h"
#include "bxpt_pcr_offset.h"

BDBG_MODULE(pcrlib);

static BERR_Code 
BPCRlib_Video_GetStc_XVD_isr(void *trp, void *dec, uint32_t *stc)
{
	BSTD_UNUSED(dec);
	BDBG_MSG(("VID_XVD:GetStc %#x %#x", (unsigned)trp, (unsigned)dec));
	*stc = BXPT_PcrOffset_GetStc_isr(trp) + BXPT_PcrOffset_GetOffset_isr(trp);
	return BERR_SUCCESS;
}

static BERR_Code 
BPCRlib_Video_GetPts_XVD_isr(void *dec, BAVC_PTSInfo *pts)
{
	BXVD_PTSInfo			PTSInfo;
	BERR_Code rc;
	const BPCRlib_Xvd_Decoder *xvd=dec;

	BDBG_MSG(("VID_XVD:GetPts %#x", (unsigned)dec));
	rc = BXVD_GetPTS_isr(xvd->dec, &PTSInfo);
	pts->ui32CurrentPTS = PTSInfo.ui32RunningPTS;
	pts->ePTSType = PTSInfo.ePTSType;
	
	return rc;
}

BERR_Code 
BPCRlib_Video_GetCdbLevel_XVD_isr(void *dec, unsigned *level)
{
	const BPCRlib_Xvd_Decoder *xvd=dec;
	BXPT_Rave_BufferInfo buffer_info;
	BERR_Code rc;

	BDBG_ASSERT(xvd->rave);
	rc = BXPT_Rave_GetBufferInfo_isr(xvd->rave, &buffer_info);
	if (rc==BERR_SUCCESS) {
		*level = buffer_info.CdbDepth;
	}
	return rc;
}

static BERR_Code 
BPCRlib_Video_SetStc_XVD_isr(void *trp, void *dec, bool dss,uint32_t stc)
{
	const BPCRlib_Xvd_Decoder *xvd=dec;
	BERR_Code rc = BERR_SUCCESS;
	BSTD_UNUSED(dss);
	BSTD_UNUSED(xvd);

	BDBG_MSG(("VID_XVD:SetStc %#x %#x", (unsigned)trp, (unsigned)dec));
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
	return rc;
}

static BERR_Code 
BPCRlib_Video_UpdateStc_XVD_isr(void *trp, bool is_request_stc)
{
	BDBG_MSG(("XVD::UpdateStc %#x", (unsigned)trp));
	if (is_request_stc) {
		/* If PCR_OFFSET block has non-zero OFFSET_THRESHOLD, then it needs
		to be forced to regenerate an offset message to RAVE. Otherwise the decoder
		may lose a pcr_offset_valid message. */
		BXPT_PcrOffset_RegenOffset_isr(trp);
	}
	return 0;
}


const BPCRlib_StcDecIface BPCRlib_Video_Xvd = {
	BPCRlib_Video_GetPts_XVD_isr,
	BPCRlib_Video_GetStc_XVD_isr,
	BPCRlib_Video_GetCdbLevel_XVD_isr,
	BPCRlib_Video_SetStc_XVD_isr, 
	BPCRlib_Video_UpdateStc_XVD_isr, 
	true
};

