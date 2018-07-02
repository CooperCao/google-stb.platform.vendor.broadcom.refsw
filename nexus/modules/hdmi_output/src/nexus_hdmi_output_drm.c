/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_hdmi_output_module.h"
#include "bhdm.h"
#include "bhdm_edid.h"

BDBG_MODULE(nexus_hdmi_output_drm);

static BAVC_HDMI_DRM_DescriptorId NEXUS_P_HdmiOutputDrmStaticMetadataType_ToMagnum_isrsafe(NEXUS_HdmiDynamicRangeMasteringStaticMetadataType type)
{
    switch (type) {
    case NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1:         return BAVC_HDMI_DRM_DescriptorId_eType1;
    default: return BAVC_HDMI_DRM_DescriptorId_eMax;
    }
}

#if !BDBG_NO_LOG
static const char * eotfStrings[] =
{
    "sdr",
    "hlg",
    "hdr10",
    "invalid",
    "disabled",
    NULL
};

static const char * metadataTypeStrings[] =
{
    "1",
    NULL
};

static void NEXUS_HdmiOutput_P_PrintType1DrmStaticMetadata(const NEXUS_StaticHdrMetadata * pMetadata)
{
    const NEXUS_MasteringDisplayColorVolume * pMdcv;
    const NEXUS_ContentLightLevel * pCll;

    pMdcv = &pMetadata->masteringDisplayColorVolume;
    pCll = &pMetadata->contentLightLevel;

    BDBG_LOG(("      green: (%d, %d)", pMdcv->greenPrimary.x, pMdcv->greenPrimary.y));
    BDBG_LOG(("      blue: (%d, %d)", pMdcv->bluePrimary.x, pMdcv->bluePrimary.y));
    BDBG_LOG(("      red: (%d, %d)", pMdcv->redPrimary.x, pMdcv->redPrimary.y));
    BDBG_LOG(("      white: (%d, %d)", pMdcv->whitePoint.x, pMdcv->whitePoint.y));
    BDBG_LOG(("      luma: (%d, %d)", pMdcv->luminance.max, pMdcv->luminance.min));
    BDBG_LOG(("      cll: %d", pCll->max));
    BDBG_LOG(("      fal: %d", pCll->maxFrameAverage));
}

static void NEXUS_HdmiOutput_P_PrintType1DrmStaticMetadataChanges(const NEXUS_StaticHdrMetadata * pOldMetadata, const NEXUS_StaticHdrMetadata * pNewMetadata)
{
    const NEXUS_MasteringDisplayColorVolume * pOldMdcv;
    const NEXUS_ContentLightLevel * pOldCll;
    const NEXUS_MasteringDisplayColorVolume * pNewMdcv;
    const NEXUS_ContentLightLevel * pNewCll;

    pOldMdcv = &pOldMetadata->masteringDisplayColorVolume;
    pOldCll = &pOldMetadata->contentLightLevel;
    pNewMdcv = &pNewMetadata->masteringDisplayColorVolume;
    pNewCll = &pNewMetadata->contentLightLevel;

    BDBG_LOG(("      green: (%d, %d) -> (%d, %d)",
        pOldMdcv->greenPrimary.x, pOldMdcv->greenPrimary.y,
        pNewMdcv->greenPrimary.x, pNewMdcv->greenPrimary.y));
    BDBG_LOG(("      blue: (%d, %d) -> (%d, %d)",
        pOldMdcv->bluePrimary.x, pOldMdcv->bluePrimary.y,
        pNewMdcv->bluePrimary.x, pNewMdcv->bluePrimary.y));
    BDBG_LOG(("      red: (%d, %d) -> (%d, %d)",
        pOldMdcv->redPrimary.x, pOldMdcv->redPrimary.y,
        pNewMdcv->redPrimary.x, pNewMdcv->redPrimary.y));
    BDBG_LOG(("      white: (%d, %d) -> (%d, %d)",
        pOldMdcv->whitePoint.x, pOldMdcv->whitePoint.y,
        pNewMdcv->whitePoint.x, pNewMdcv->whitePoint.y));
    BDBG_LOG(("      luma: (%d, %d) -> (%d, %d)",
        pOldMdcv->luminance.max, pOldMdcv->luminance.min,
        pNewMdcv->luminance.max, pNewMdcv->luminance.min));
    BDBG_LOG(("      cll: %d -> %d",
        pOldCll->max,
        pNewCll->max));
    BDBG_LOG(("      fal: %d -> %d",
        pOldCll->maxFrameAverage,
        pNewCll->maxFrameAverage));
}

static void NEXUS_HdmiOutput_P_PrintDrmInfoFrameChanges(const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pOldInfoFrame, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pNewInfoFrame)
{
    BDBG_LOG(("    DRMInfoFrame")) ;
    BDBG_LOG(("      eotf: %s -> %s",
        eotfStrings[pOldInfoFrame->eotf],
        eotfStrings[pNewInfoFrame->eotf]));
    BDBG_LOG(("      type: %s -> %s",
        metadataTypeStrings[pOldInfoFrame->metadata.type],
        metadataTypeStrings[pNewInfoFrame->metadata.type]));
    if (pOldInfoFrame->metadata.type != pNewInfoFrame->metadata.type)
    {
        BDBG_LOG(("      [old]"));
        switch (pOldInfoFrame->metadata.type)
        {
            case NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1:
                NEXUS_HdmiOutput_P_PrintType1DrmStaticMetadata(&pOldInfoFrame->metadata.typeSettings.type1);
                break;
            default:
                break;
        }
        BDBG_LOG(("      [new]"));
        switch (pNewInfoFrame->metadata.type)
        {
            case NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1:
                NEXUS_HdmiOutput_P_PrintType1DrmStaticMetadata(&pNewInfoFrame->metadata.typeSettings.type1);
                break;
            default:
                break;
        }
    }
    else
    {
        switch (pNewInfoFrame->metadata.type)
        {
            case NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1:
                NEXUS_HdmiOutput_P_PrintType1DrmStaticMetadataChanges(
                    &pOldInfoFrame->metadata.typeSettings.type1,
                    &pNewInfoFrame->metadata.typeSettings.type1);
                break;
            default:
                break;
        }
    }
}
#endif

static void NEXUS_HdmiOutput_P_DrmInfoFrame_ToMagnum(const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pNexus, BAVC_HDMI_DRMInfoFrame * pMagnum)
{
    pMagnum->eEOTF = NEXUS_P_VideoEotf_ToMagnum_isrsafe(pNexus->eotf);
    pMagnum->eDescriptorId = NEXUS_P_HdmiOutputDrmStaticMetadataType_ToMagnum_isrsafe(pNexus->metadata.type);
    switch (pNexus->metadata.type)
    {
        case NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1:
            NEXUS_P_StaticHdrMetadata_ToMagnum_isrsafe(&pNexus->metadata.typeSettings.type1, &pMagnum->stType1);
            break;
        default:
            BDBG_WRN(("Unknown meta data type: %d", pNexus->metadata.type)) ;
            break;
    }
}

#if NEXUS_DBV_SUPPORT
static void NEXUS_HdmiOutput_P_DrmInfoFrameDisable(NEXUS_HdmiOutputHandle output)
{
    BAVC_HDMI_DRMInfoFrame stDRMInfoFrame ;
    BDBG_LOG(("    DRMInfoFrame")) ;
    BDBG_LOG(("      eotf: %s -> %s",
        eotfStrings[output->drm.outputInfoFrame.eotf],
        eotfStrings[NEXUS_VideoEotf_eInvalid]));
    output->drm.outputInfoFrame.eotf = NEXUS_VideoEotf_eInvalid;
    BHDM_GetDRMInfoFramePacket(output->hdmHandle, &stDRMInfoFrame) ;
    NEXUS_HdmiOutput_P_DrmInfoFrame_ToMagnum(&output->drm.outputInfoFrame, &stDRMInfoFrame);
    BHDM_SetDRMInfoFramePacket(output->hdmHandle, &stDRMInfoFrame) ;
    /* notify display that we've changed drminfoframe */
    NEXUS_TaskCallback_Fire(output->notifyDisplay);  /* NEXUS_VideoOutput_P_SetHdmiSettings */
}

static void NEXUS_HdmiOutput_P_DrmInfoFrameDisableTimerExpiration(void * pContext)
{
    NEXUS_HdmiOutputHandle output = pContext;
    if (!output->drm.offTimer) return; /* someone canceled early */
    BDBG_LOG(("DRMIF disable timer expired"));
    output->drm.offTimer = NULL;
    NEXUS_HdmiOutput_P_DrmInfoFrameDisable(output);
}

static const NEXUS_HdmiDynamicRangeMasteringInfoFrame DRM_ZERO =
{
    0,
    {
        0,
        {
            {
                {
                    { 0, 0 },
                    { 0, 0 },
                    { 0, 0 },
                    { 0, 0 },
                    { 0, 0 }
                },
                { 0, 0 }
            }
        }
    }
};
#endif

static NEXUS_Error NEXUS_HdmiOutput_P_SetDrmInfoFrame(NEXUS_HdmiOutputHandle output, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    BERR_Code rc = BERR_SUCCESS;
    BAVC_HDMI_DRMInfoFrame stDRMInfoFrame ;

    if (BKNI_Memcmp(&output->drm.outputInfoFrame, pDrmInfoFrame, sizeof(output->drm.outputInfoFrame)))
    {
        if (pDrmInfoFrame->eotf != NEXUS_VideoEotf_eSdr && pDrmInfoFrame->eotf != NEXUS_VideoEotf_eInvalid)
        {
            /*
             * the first time we see a non-SDR, non-Invalid eotf set to this
             * output, we set this flag and print changes from then on
             */
            output->drm.printDrmInfoFrameChanges = true;
        }

#if NEXUS_DBV_SUPPORT
        if (pDrmInfoFrame->eotf == NEXUS_VideoEotf_eInvalid && output->drm.outputInfoFrame.eotf != NEXUS_VideoEotf_eInvalid)
        {
            /*
             * NOTE: The HDMI 2.1 spec requires that when we disable the DRMIF
             * transmission, we must first send an all-zero DRMIF for 2 seconds
             * and then we can disable the packet transmission altogether.
             * However, there are currently TVs out there that can't handle this
             * and instead expect the packet to transition immediately from
             * DRMIF(HDR) to no DRMIF sent.  As such, for non-Dolby-Vision
             * receivers, we will use the non-spec-compliant behavior of
             * immediate transition with no 2-second, all-zero DRMIF.
             * Since Dolby Vision compliance requires that we match the
             * HDMI spec, we enable spec-compliant behavior for any Dolby Vision
             * capable receiver.
             */
            if (output->dbv.supported)
            {
                BDBG_LOG(("Disabling DRMIF requires 2 seconds of transmittal with zeroes"));
                output->drm.offTimer = NEXUS_ScheduleTimer(2000, NEXUS_HdmiOutput_P_DrmInfoFrameDisableTimerExpiration, output);
                pDrmInfoFrame = &DRM_ZERO;
            }
        }
        else if (pDrmInfoFrame->eotf != NEXUS_VideoEotf_eInvalid && output->drm.offTimer)
        {
            NEXUS_CancelTimer(output->drm.offTimer);
            output->drm.offTimer = NULL;
        }
#endif

#if !BDBG_NO_LOG
        if (output->drm.printDrmInfoFrameChanges)
        {
            BDBG_LOG(("NEXUS_HdmiOutput_P_SetDrmInfoFrame")) ;
            NEXUS_HdmiOutput_P_PrintDrmInfoFrameChanges(&output->drm.outputInfoFrame, pDrmInfoFrame);
        }
#endif
        BKNI_Memcpy(&output->drm.outputInfoFrame, pDrmInfoFrame, sizeof(output->drm.outputInfoFrame));

        BHDM_GetDRMInfoFramePacket(output->hdmHandle, &stDRMInfoFrame) ;
        NEXUS_HdmiOutput_P_DrmInfoFrame_ToMagnum(pDrmInfoFrame, &stDRMInfoFrame);
        rc = BHDM_SetDRMInfoFramePacket(output->hdmHandle, &stDRMInfoFrame) ;
        if (rc) { BERR_TRACE(rc); goto error; }

        /* notify display that we've changed drminfoframe */
        NEXUS_TaskCallback_Fire(output->notifyDisplay);  /* NEXUS_VideoOutput_P_SetHdmiSettings */
    }

error:
    return rc;
}

static NEXUS_VideoEotf NEXUS_HdmiOutput_P_ComputeEotf(
    NEXUS_VideoEotf preferredEotf,
    const BHDM_EDID_HDRStaticDB * pHdrDataBlock,
    bool plmSupported)
{
    NEXUS_VideoEotf eotf = NEXUS_VideoEotf_eSdr;

    if (!pHdrDataBlock->valid && ((preferredEotf == NEXUS_VideoEotf_eHlg) || (preferredEotf == NEXUS_VideoEotf_eHdr10)))
    {
        eotf = NEXUS_VideoEotf_eInvalid; /* legacy SDR */
        if (!plmSupported) { BDBG_WRN(("Receiver does not support HDR data block, no DRMInfoFrame sent. Displaying all video as SDR. Content may not appear correctly.")); }
    }
    else if (preferredEotf == NEXUS_VideoEotf_eHlg)
    {
        if (pHdrDataBlock->bEotfSupport[BAVC_HDMI_DRM_EOTF_eHLG])
        {
            eotf = NEXUS_VideoEotf_eHlg;
            BDBG_MSG(("Using HLG eotf"));
        }
        else
        {
            if (!plmSupported) { BDBG_WRN(("Input prefers HLG eotf; receiver does not support it; falling back to SDR")); }
        }
    }
    else if (preferredEotf == NEXUS_VideoEotf_eHdr10)
    {
        if (pHdrDataBlock->bEotfSupport[BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084])
        {
            eotf = NEXUS_VideoEotf_eHdr10;
            BDBG_MSG(("Using HDR10 eotf"));
        }
        else
        {
            if (!plmSupported) { BDBG_WRN(("Input prefers HDR10 eotf; receiver does not support it; falling back to SDR. Content may not appear correctly.")); }
        }
    }

    return eotf;
}

static void NEXUS_HdmiOutput_P_DrmInfoFrame_ApplyEdid(NEXUS_HdmiOutputHandle output, NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    /* for now, we only modify the eotf */
    pDrmInfoFrame->eotf = NEXUS_HdmiOutput_P_ComputeEotf(pDrmInfoFrame->eotf, &output->drm.hdrdb,
        output->drm.processingCaps.typesSupported[NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_ePlm]);
    if (pDrmInfoFrame->eotf == NEXUS_VideoEotf_eSdr)
    {
        BKNI_Memset(&pDrmInfoFrame->metadata, 0, sizeof(pDrmInfoFrame->metadata)) ;
    }
}

static void NEXUS_HdmiOutput_P_BuildDrmInfoFrame(NEXUS_HdmiDynamicRangeMasteringInfoFrame * pTargetDrmInfoFrame, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pSourceDrmInfoFrame)
{
    BKNI_Memset(pTargetDrmInfoFrame, 0, sizeof(*pTargetDrmInfoFrame));
    if (pSourceDrmInfoFrame->eotf != NEXUS_VideoEotf_eSdr)
    {
        BKNI_Memcpy(pTargetDrmInfoFrame, pSourceDrmInfoFrame, sizeof(*pTargetDrmInfoFrame));
    }
}

static NEXUS_Error NEXUS_HdmiOutput_P_ApplyInputDrmInfoFrame(NEXUS_HdmiOutputHandle output)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiDynamicRangeMasteringInfoFrame drmInfoFrame;

    if (output->drm.inputInfoFrame.eotf != NEXUS_VideoEotf_eInvalid)
    {
        NEXUS_HdmiOutput_P_BuildDrmInfoFrame(&drmInfoFrame, &output->drm.inputInfoFrame);

        /* check compat with EDID -> modify as necessary and print debug */
        NEXUS_HdmiOutput_P_DrmInfoFrame_ApplyEdid(output, &drmInfoFrame);

#if NEXUS_DBV_SUPPORT
        NEXUS_HdmiOutput_P_DbvUpdateDrmInfoFrame(output, &drmInfoFrame);
#endif

        /* then set */
        rc = NEXUS_HdmiOutput_P_SetDrmInfoFrame(output, &drmInfoFrame);
        if (rc) { BERR_TRACE(rc); goto error; }
    }
    else
    {
        BDBG_MSG(("No input; Use override to apply HDR settings sans input."));
    }

error:
    return rc;
}

NEXUS_Error NEXUS_HdmiOutput_P_ApplyDrmInfoFrameSource(NEXUS_HdmiOutputHandle output)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(output);

    if (output->drm.connected)
    {
        if (output->extraSettings.overrideDynamicRangeMasteringInfoFrame) /* means we listen to what user wants and ignore edid */
        {
            NEXUS_HdmiDynamicRangeMasteringInfoFrame drmInfoFrame;
            NEXUS_HdmiOutput_P_BuildDrmInfoFrame(&drmInfoFrame, &output->extraSettings.dynamicRangeMasteringInfoFrame);
            /*
             * extra settings drmif always uses units of 1 nit for max luma, but
             * internally we use units of 100 micronits where we can, so
             * convert to 100 micronits units from 1 nit units
             */
            drmInfoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max *= 10000;
#if NEXUS_DBV_SUPPORT
            NEXUS_HdmiOutput_P_DbvUpdateDrmInfoFrame(output, &drmInfoFrame);
#endif
            rc = NEXUS_HdmiOutput_P_SetDrmInfoFrame(output, &drmInfoFrame);
            if (rc) { BERR_TRACE(rc); goto error; }
        }
        else
        {
            rc = NEXUS_HdmiOutput_P_ApplyInputDrmInfoFrame(output);
            if (rc) { BERR_TRACE(rc); goto error; }
        }
    }
    else
    {
        BDBG_MSG(("No receiver connected. HDR irrelevant."));
    }

error:
    return rc;
}

void NEXUS_HdmiOutput_P_DrmInfoFrameConnectionChanged(NEXUS_HdmiOutputHandle output)
{
    BERR_Code rc = BERR_SUCCESS;
    bool changed = false;
    bool connected = false;

    connected = output->rxState >= NEXUS_HdmiOutputState_eRxSenseCheck;

    if (output->drm.connected != connected)
    {
        output->drm.connected = connected;
        changed = true;
    }

    BDBG_MSG(("NEXUS_HdmiOutput_P_DrmInfoFrameConnectionChanged: %s", connected ? "connected" : "disconnected"));

    if (connected)
    {
        BHDM_EDID_HDRStaticDB hdrdb;
        rc = BHDM_EDID_GetHdrStaticMetadatadb(output->hdmHandle, &hdrdb);
        if (rc) { BERR_TRACE(rc); goto error; }
        if (BKNI_Memcmp(&output->drm.hdrdb, &hdrdb, sizeof(output->drm.hdrdb)))
        {
            BKNI_Memcpy(&output->drm.hdrdb, &hdrdb, sizeof(output->drm.hdrdb));
            changed = true;
        }
    }
    else
    {
        BKNI_Memset(&output->drm.hdrdb, 0, sizeof(output->drm.hdrdb));
    }

    if (changed)
    {
        rc = NEXUS_HdmiOutput_P_ApplyDrmInfoFrameSource(output);
        if (rc) { BERR_TRACE(rc); goto error; }
    }

error:
    return;
}

NEXUS_Error NEXUS_HdmiOutput_SetInputDrmInfoFrame_priv(NEXUS_HdmiOutputHandle output,
    const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bool changed = false;

    if (BKNI_Memcmp(&output->drm.inputInfoFrame, pDrmInfoFrame, sizeof(output->drm.inputInfoFrame)))
    {
        BKNI_Memcpy(&output->drm.inputInfoFrame, pDrmInfoFrame, sizeof(output->drm.inputInfoFrame));
        changed = true;
    }

    if (changed)
    {
        rc = NEXUS_HdmiOutput_P_ApplyDrmInfoFrameSource(output);
        if (rc) { BERR_TRACE(rc); goto error; }
    }

error:
    return rc;
}

NEXUS_Error NEXUS_HdmiOutput_SetDisplayDynamicRangeProcessingCapabilities_priv(NEXUS_HdmiOutputHandle output,
    const NEXUS_HdmiOutputDisplayDynamicRangeProcessingCapabilities * pCaps)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bool changed = false;

    if (BKNI_Memcmp(&output->drm.processingCaps, pCaps, sizeof(output->drm.processingCaps)))
    {
        BKNI_Memcpy(&output->drm.processingCaps, pCaps, sizeof(output->drm.processingCaps));
        changed = true;
    }

    if (changed)
    {
        rc = NEXUS_HdmiOutput_P_ApplyDrmInfoFrameSource(output);
        if (rc) { BERR_TRACE(rc); goto error; }
    }

error:
    return rc;
}

bool NEXUS_HdmiOutput_GetEotf_priv(
    NEXUS_HdmiOutputHandle hdmiOutput,
    NEXUS_VideoEotf *pEotf
)
{
    if (hdmiOutput->drm.outputInfoFrame.eotf == NEXUS_VideoEotf_eInvalid)
    {
        /* output invalid means user has requested to turn off packet transmission or we are in init state */
        if (!hdmiOutput->drm.processingCaps.typesSupported[NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_ePlm]
            && hdmiOutput->drm.inputInfoFrame.eotf != NEXUS_VideoEotf_eInvalid)
        {
            /* if input is valid, tell display that output is input (no conversion on non-plm chips) */
            *pEotf = hdmiOutput->drm.inputInfoFrame.eotf;
        }
        else
        {
            /* otherwise just use SDR */
            *pEotf = NEXUS_VideoEotf_eSdr;
        }
    }
    else
    {
        /* grab normal computed output eotf */
        *pEotf = hdmiOutput->drm.outputInfoFrame.eotf;
    }

    return true;
}
