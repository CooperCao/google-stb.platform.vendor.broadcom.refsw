/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
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

#if !BDBG_NO_MSG
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

static void NEXUS_HdmiOutput_P_PrintType1DrmStaticMetadata(const NEXUS_HdmiType1DynamicRangeMasteringStaticMetadata * pMetadata)
{
    const NEXUS_MasteringDisplayColorVolume * pMdcv;
    const NEXUS_ContentLightLevel * pCll;

    pMdcv = &pMetadata->masteringDisplayColorVolume;
    pCll = &pMetadata->contentLightLevel;

    BDBG_MSG(("      green: (%d, %d)", pMdcv->greenPrimary.x, pMdcv->greenPrimary.y));
    BDBG_MSG(("      blue: (%d, %d)", pMdcv->bluePrimary.x, pMdcv->bluePrimary.y));
    BDBG_MSG(("      red: (%d, %d)", pMdcv->redPrimary.x, pMdcv->redPrimary.y));
    BDBG_MSG(("      white: (%d, %d)", pMdcv->whitePoint.x, pMdcv->whitePoint.y));
    BDBG_MSG(("      luma: (%d, %d)", pMdcv->luminance.max, pMdcv->luminance.min));
    BDBG_MSG(("      cll: %d", pCll->max));
    BDBG_MSG(("      fal: %d", pCll->maxFrameAverage));
}

static void NEXUS_HdmiOutput_P_PrintType1DrmStaticMetadataChanges(const NEXUS_HdmiType1DynamicRangeMasteringStaticMetadata * pOldMetadata, const NEXUS_HdmiType1DynamicRangeMasteringStaticMetadata * pNewMetadata)
{
    const NEXUS_MasteringDisplayColorVolume * pOldMdcv;
    const NEXUS_ContentLightLevel * pOldCll;
    const NEXUS_MasteringDisplayColorVolume * pNewMdcv;
    const NEXUS_ContentLightLevel * pNewCll;

    pOldMdcv = &pOldMetadata->masteringDisplayColorVolume;
    pOldCll = &pOldMetadata->contentLightLevel;
    pNewMdcv = &pNewMetadata->masteringDisplayColorVolume;
    pNewCll = &pNewMetadata->contentLightLevel;

    BDBG_MSG(("      green: (%d, %d) -> (%d, %d)",
        pOldMdcv->greenPrimary.x, pOldMdcv->greenPrimary.y,
        pNewMdcv->greenPrimary.x, pNewMdcv->greenPrimary.y));
    BDBG_MSG(("      blue: (%d, %d) -> (%d, %d)",
        pOldMdcv->bluePrimary.x, pOldMdcv->bluePrimary.y,
        pNewMdcv->bluePrimary.x, pNewMdcv->bluePrimary.y));
    BDBG_MSG(("      red: (%d, %d) -> (%d, %d)",
        pOldMdcv->redPrimary.x, pOldMdcv->redPrimary.y,
        pNewMdcv->redPrimary.x, pNewMdcv->redPrimary.y));
    BDBG_MSG(("      white: (%d, %d) -> (%d, %d)",
        pOldMdcv->whitePoint.x, pOldMdcv->whitePoint.y,
        pNewMdcv->whitePoint.x, pNewMdcv->whitePoint.y));
    BDBG_MSG(("      luma: (%d, %d) -> (%d, %d)",
        pOldMdcv->luminance.max, pOldMdcv->luminance.min,
        pNewMdcv->luminance.max, pNewMdcv->luminance.min));
    BDBG_MSG(("      cll: %d -> %d",
        pOldCll->max,
        pNewCll->max));
    BDBG_MSG(("      fal: %d -> %d",
        pOldCll->maxFrameAverage,
        pNewCll->maxFrameAverage));
}

static void NEXUS_HdmiOutput_P_PrintDrmInfoFrameChanges(const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pOldInfoFrame, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pNewInfoFrame)
{
    BDBG_MSG(("    DRMInfoFrame")) ;
    BDBG_MSG(("      eotf: %s -> %s",
        eotfStrings[pOldInfoFrame->eotf],
        eotfStrings[pNewInfoFrame->eotf]));
    BDBG_MSG(("      type: %s -> %s",
        metadataTypeStrings[pOldInfoFrame->metadata.type],
        metadataTypeStrings[pNewInfoFrame->metadata.type]));
    if (pOldInfoFrame->metadata.type != pNewInfoFrame->metadata.type)
    {
        BDBG_MSG(("      [old]"));
        switch (pOldInfoFrame->metadata.type)
        {
            case NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1:
                NEXUS_HdmiOutput_P_PrintType1DrmStaticMetadata(&pOldInfoFrame->metadata.typeSettings.type1);
                break;
            default:
                break;
        }
        BDBG_MSG(("      [new]"));
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

static void NEXUS_HdmiOutput_P_DrmInfoFrame_ToMagnum(BAVC_HDMI_DRMInfoFrame * magnum, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * nexus)
{
    magnum->eEOTF = NEXUS_P_VideoEotf_ToMagnum_isrsafe(nexus->eotf);
    magnum->eDescriptorId = NEXUS_P_HdmiOutputDrmStaticMetadataType_ToMagnum_isrsafe(nexus->metadata.type);
    switch (nexus->metadata.type)
    {
        case NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1:
            magnum->Type1.DisplayPrimaries[0].X = nexus->metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.x;
            magnum->Type1.DisplayPrimaries[0].Y = nexus->metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.y;
            magnum->Type1.DisplayPrimaries[1].X = nexus->metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.x;
            magnum->Type1.DisplayPrimaries[1].Y = nexus->metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.y;
            magnum->Type1.DisplayPrimaries[2].X = nexus->metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.x;
            magnum->Type1.DisplayPrimaries[2].Y = nexus->metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.y;
            magnum->Type1.WhitePoint.X = nexus->metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.x;
            magnum->Type1.WhitePoint.Y = nexus->metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.y;
            magnum->Type1.DisplayMasteringLuminance.Max = nexus->metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max;
            magnum->Type1.DisplayMasteringLuminance.Min = nexus->metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.min;
            magnum->Type1.MaxContentLightLevel = nexus->metadata.typeSettings.type1.contentLightLevel.max;
            magnum->Type1.MaxFrameAverageLightLevel = nexus->metadata.typeSettings.type1.contentLightLevel.maxFrameAverage;
            break;
        default:
            break;
    }
}

static NEXUS_Error NEXUS_HdmiOutput_P_SetDrmInfoFrame(NEXUS_HdmiOutputHandle output, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    BERR_Code rc = BERR_SUCCESS;
    BAVC_HDMI_DRMInfoFrame stDRMInfoFrame ;

    BHDM_GetDRMInfoFramePacket(output->hdmHandle, &stDRMInfoFrame) ;
        NEXUS_HdmiOutput_P_DrmInfoFrame_ToMagnum(&stDRMInfoFrame, pDrmInfoFrame);
    rc = BHDM_SetDRMInfoFramePacket(output->hdmHandle, &stDRMInfoFrame) ;
    if (rc) { BERR_TRACE(rc); goto error; }

    if (BKNI_Memcmp(&output->drm.outputDrmInfoFrame, pDrmInfoFrame, sizeof(output->drm.outputDrmInfoFrame)))
    {
        /* TODO: is formatChange necessary? */
        output->formatChangeUpdate = true;
#if !BDBG_NO_MSG
        BDBG_MSG(("NEXUS_HdmiOutput_P_SetDrmInfoFrame")) ;
        NEXUS_HdmiOutput_P_PrintDrmInfoFrameChanges(&output->drm.outputDrmInfoFrame, pDrmInfoFrame);
#endif
        BKNI_Memcpy(&output->drm.outputDrmInfoFrame, pDrmInfoFrame, sizeof(output->drm.outputDrmInfoFrame));
        /* notify display that we've changed drminfoframe */
        NEXUS_TaskCallback_Fire(output->notifyDisplay);  /* NEXUS_VideoOutput_P_SetHdmiSettings */
    }

error:
    return rc;
}

static NEXUS_VideoEotf NEXUS_HdmiOutput_P_ComputeEotf(
    NEXUS_VideoEotf preferredEotf,
    const NEXUS_HdmiOutputEdidRxHdrdb * pHdrDataBlock)
{
    NEXUS_VideoEotf eotf = NEXUS_VideoEotf_eSdr;

    if (!pHdrDataBlock->valid)
    {
        BDBG_WRN(("Receiver does not support HDR data block, no DRMInfoFrame sent. Displaying all video as SDR. Content may not appear correctly."));
    }
    else if (preferredEotf == NEXUS_VideoEotf_eHlg)
    {
        if (pHdrDataBlock->eotfSupported[NEXUS_VideoEotf_eHlg])
        {
            eotf = NEXUS_VideoEotf_eHlg;
            BDBG_MSG(("Using HLG eotf"));
        }
        else
        {
            BDBG_WRN(("Input prefers HLG eotf; receiver does not support it; falling back to SDR"));
        }
    }
    else if (preferredEotf == NEXUS_VideoEotf_eHdr10)
    {
        if (pHdrDataBlock->eotfSupported[NEXUS_VideoEotf_eHdr10])
        {
            eotf = NEXUS_VideoEotf_eHdr10;
            BDBG_MSG(("Using HDR10 eotf"));
        }
        else
        {
            /* TODO: if chip supports HDR->SDR conversion, give a nicer message */
            BDBG_WRN(("Input prefers HDR10 eotf; receiver does not support it; falling back to SDR. Content may not appear correctly."));
        }
    }

    return eotf;
}

static void NEXUS_HdmiOutput_P_DrmInfoFrame_ApplyEdid(NEXUS_HdmiOutputHandle output, NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    /* for now, we only modify the eotf */
    pDrmInfoFrame->eotf = NEXUS_HdmiOutput_P_ComputeEotf(pDrmInfoFrame->eotf, &output->drm.hdrdb);
}

static NEXUS_Error NEXUS_HdmiOutput_P_ApplyInputDrmInfoFrame(NEXUS_HdmiOutputHandle output)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiDynamicRangeMasteringInfoFrame drmInfoFrame;

    if (output->drm.inputDrmInfoFrameValid)
    {
        /* start with copy */
        BKNI_Memcpy(&drmInfoFrame, &output->drm.inputDrmInfoFrame, sizeof(drmInfoFrame));

        /* check compat with EDID -> modify as necessary and print debug */
        NEXUS_HdmiOutput_P_DrmInfoFrame_ApplyEdid(output, &drmInfoFrame);

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
            rc = NEXUS_HdmiOutput_P_SetDrmInfoFrame(output, &output->extraSettings.dynamicRangeMasteringInfoFrame);
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

void NEXUS_HdmiOutput_GetDefaultDrmInfoFrame_priv(NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    BKNI_Memset(pDrmInfoFrame, 0, sizeof(*pDrmInfoFrame));
    pDrmInfoFrame->metadata.type = NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1;
}

void NEXUS_HdmiOutput_P_DrmInfoFrameConnectionChanged(NEXUS_HdmiOutputHandle output)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputEdidData edid;
    bool changed = false;

    rc = NEXUS_HdmiOutput_GetStatus(output, &status);
    if (rc) { BERR_TRACE(rc); goto error; }

    if (output->drm.connected != status.connected)
    {
        output->drm.connected = status.connected;
        changed = true;
    }

    BDBG_MSG(("NEXUS_HdmiOutput_P_DrmInfoFrameConnectionChanged: %s", status.connected ? "connected" : "disconnected"));

    if (status.connected)
    {
        rc = NEXUS_HdmiOutput_GetEdidData(output, &edid);
        if (rc) { BERR_TRACE(rc); goto error; }
        if (BKNI_Memcmp(&output->drm.hdrdb, &edid.hdrdb, sizeof(output->drm.hdrdb)))
        {
            BKNI_Memcpy(&output->drm.hdrdb, &edid.hdrdb, sizeof(output->drm.hdrdb));
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

NEXUS_Error NEXUS_HdmiOutput_SetInputDrmInfoFrame_priv(NEXUS_HdmiOutputHandle output, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bool changed = false;

    if (BKNI_Memcmp(&output->drm.inputDrmInfoFrame, pDrmInfoFrame, sizeof(output->drm.inputDrmInfoFrame)))
    {
        BKNI_Memcpy(&output->drm.inputDrmInfoFrame, pDrmInfoFrame, sizeof(output->drm.inputDrmInfoFrame));
        output->drm.inputDrmInfoFrameValid = true;
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
    if (hdmiOutput->drm.outputDrmInfoFrame.eotf == NEXUS_VideoEotf_eInvalid)
    {
        /* user has requested to turn off packet transmission */
        if (hdmiOutput->drm.inputDrmInfoFrameValid)
        {
            /* if input is valid, tell display that output is input (no conversion) */
            *pEotf = hdmiOutput->drm.inputDrmInfoFrame.eotf;
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
        *pEotf = hdmiOutput->drm.outputDrmInfoFrame.eotf;
    }

    return true;
}
