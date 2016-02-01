/***************************************************************************
 *     Copyright (c) 2005-2011, Broadcom Corporation
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
#include "bpcrlib_ape.h"
#include "bxpt_pcr_offset.h"

BDBG_MODULE(pcrlib);

static BERR_Code 
BPCRlib_Audio_GetStc_Ape_isr(void *trp, void *dec, uint32_t *stc)
{
    BSTD_UNUSED(dec);
    BDBG_MSG(("APE:GetStc %#x %#x", (unsigned)trp, (unsigned)dec));
    *stc = BXPT_PcrOffset_GetStc_isr(trp) + BXPT_PcrOffset_GetOffset_isr(trp);
    return BERR_SUCCESS;
}

static BERR_Code 
BPCRlib_Audio_GetPts_Ape_isr(void *dec, BAVC_PTSInfo *pts)
{
    BAPE_DecoderTsmStatus tsmStatus;
    BERR_Code rc;
    const BPCRlib_Ape_Decoder *ape=dec;

    BDBG_MSG(("APE:GetPts %#x", (unsigned)dec));
    rc = BAPE_Decoder_GetTsmStatus_isr(ape->dec, &tsmStatus);
    *pts = tsmStatus.ptsInfo;
    
    return rc;
}

static BERR_Code 
BPCRlib_Audio_GetCdbLevel_Ape_isr(void *dec, unsigned *level)
{
    const BPCRlib_Ape_Decoder *ape=dec;
    BXPT_Rave_BufferInfo buffer_info;
    BERR_Code rc;

    BDBG_ASSERT(ape->rave);
    rc = BXPT_Rave_GetBufferInfo_isr(ape->rave, &buffer_info);
    if (rc==BERR_SUCCESS) {
        *level = buffer_info.CdbDepth;
    }
    return rc;
}


static BERR_Code 
BPCRlib_Audio_SetStc_Ape_isr(void *trp, void *dec, bool dss, uint32_t stc)
{
    BERR_Code  rc = BERR_SUCCESS;
    const BPCRlib_Ape_Decoder *ape=dec;
    uint32_t current_stc;
    int32_t diff;
    BSTD_UNUSED(dss);

    BDBG_MSG(("APE:SetStc %#x %#x", (unsigned)trp, (unsigned)dec));

    current_stc = BXPT_PcrOffset_GetStc_isr(trp);
    diff = (int32_t)current_stc - (int32_t)stc;
    /* If the STC is already very close, there's no point in resetting it. This 
    prevents PTS Errors from raptor which tries to follow a tight TSM threshold. */
    if (diff > 100 || diff < -100) {
        /* assume that PCR_OFFSET offset was set to 0 by host before starting PVR. */
        rc = BXPT_PcrOffset_SetStc_isr(trp, stc);
    }
    
    if (rc==BERR_SUCCESS) {
        rc = BAPE_Decoder_SetStcValid_isr(ape->dec);
    }
    return rc;
}

static BERR_Code 
BPCRlib_Audio_UpdateStc_Ape_isr(void *trp, bool is_request_stc)
{
    BDBG_MSG(("APE::UpdateStc %#x", (unsigned)trp));
    if (is_request_stc) {
        /* If PCR_OFFSET block has non-zero OFFSET_THRESHOLD, then it needs
        to be forced to regenerate an offset message to RAVE. Otherwise the decoder
        may lose a pcr_offset_valid message. */
        BXPT_PcrOffset_RegenOffset_isr(trp);
    }
    return 0;
}

const BPCRlib_StcDecIface BPCRlib_Audio_Ape = {
    BPCRlib_Audio_GetPts_Ape_isr,
    BPCRlib_Audio_GetStc_Ape_isr,
    BPCRlib_Audio_GetCdbLevel_Ape_isr,
    BPCRlib_Audio_SetStc_Ape_isr, 
    BPCRlib_Audio_UpdateStc_Ape_isr,  /* no update STC */
    true
};

