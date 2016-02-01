/***************************************************************************
 *     Copyright (c) 2005-2008, Broadcom Corporation
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
#include "bpcrlib_rap.h"
#ifndef BCHP_7411_VER
#include "bxpt_pcr_offset.h"
#endif

#if BRAP_VER >= 2
#define BRAP_DSPCHN_GetCurrentPTS_isr(x,Y) BRAP_GetCurrentPTS_isr(x,Y)
#define BRAP_DSPCHN_SetStcValidFlag_isr(x) BRAP_SetStcValidFlag_isr(x) 
#endif

BDBG_MODULE(pcrlib);

static BERR_Code 
BPCRlib_Audio_GetStc_Rap_isr(void *trp, void *dec, uint32_t *stc)
{
    BSTD_UNUSED(dec);
    BDBG_MSG(("RAP:GetStc %#x %#x", (unsigned)trp, (unsigned)dec));
#ifdef BCHP_7411_VER
    return BARC_GetSTC_isr(trp, stc);
#else
    *stc = BXPT_PcrOffset_GetStc_isr(trp) + BXPT_PcrOffset_GetOffset_isr(trp);
    return BERR_SUCCESS;
#endif
}

static BERR_Code 
BPCRlib_Audio_GetPts_Rap_isr(void *dec, BAVC_PTSInfo *pts)
{
    BRAP_DSPCHN_PtsInfo PTSInfo;
    BERR_Code rc;
    const BPCRlib_Rap_Decoder *rap=dec;

    BDBG_MSG(("RAP:GetPts %#x", (unsigned)dec));
    rc = BRAP_DSPCHN_GetCurrentPTS_isr(rap->dec, &PTSInfo);
    pts->ui32CurrentPTS = PTSInfo.ui32RunningPts;
    pts->ePTSType = PTSInfo.ePtsType;
    
    return rc;
}

static BERR_Code 
BPCRlib_Audio_GetCdbLevel_Rap_isr(void *dec, unsigned *level)
{
    const BPCRlib_Rap_Decoder *rap=dec;
#ifdef BCHP_7411_VER
    return BRAP_TRANS_GetCdbOccupancy_isr(rap->trans,  level);
#else
    BXPT_Rave_BufferInfo buffer_info;
    BERR_Code rc;

    BDBG_ASSERT(rap->rave);
    rc = BXPT_Rave_GetBufferInfo_isr(rap->rave, &buffer_info);
    if (rc==BERR_SUCCESS) {
        *level = buffer_info.CdbDepth;
    }
    return rc;
#endif
}


static BERR_Code 
BPCRlib_Audio_SetStc_Rap_isr(void *trp, void *dec, bool dss, uint32_t stc)
{
    BERR_Code  rc = BERR_SUCCESS;
    const BPCRlib_Rap_Decoder *rap=dec;
    BSTD_UNUSED(dss);

    BDBG_MSG(("RAP:SetStc %#x %#x", (unsigned)trp, (unsigned)dec));

#ifdef BCHP_7411_VER
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
    
#endif
    if (rc==BERR_SUCCESS) {
        rc = BRAP_DSPCHN_SetStcValidFlag_isr(rap->dec);
    }
    return rc;
}

static BERR_Code 
BPCRlib_Audio_UpdateStc_Rap_isr(void *trp, bool is_request_stc)
{
    BDBG_MSG(("RAP::UpdateStc %#x", (unsigned)trp));
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

const BPCRlib_StcDecIface BPCRlib_Audio_Rap = {
    BPCRlib_Audio_GetPts_Rap_isr,
    BPCRlib_Audio_GetStc_Rap_isr,
    BPCRlib_Audio_GetCdbLevel_Rap_isr,
    BPCRlib_Audio_SetStc_Rap_isr, 
    BPCRlib_Audio_UpdateStc_Rap_isr,  /* no update STC */
    true
};

