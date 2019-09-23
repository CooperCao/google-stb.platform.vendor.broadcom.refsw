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

BDBG_MODULE(nexus_hdmi_output_drmif);

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

static void NEXUS_HdmiOutput_Drmif_P_PrintType1StaticMetadata(const NEXUS_HdmiType1DynamicRangeMasteringStaticMetadata * pMetadata)
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

static void NEXUS_HdmiOutput_Drmif_P_PrintType1StaticMetadataChanges(const NEXUS_HdmiType1DynamicRangeMasteringStaticMetadata * pOldMetadata, const NEXUS_HdmiType1DynamicRangeMasteringStaticMetadata * pNewMetadata)
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

static void NEXUS_HdmiOutput_Drmif_P_PrintChanges(const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pOldInfoFrame, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pNewInfoFrame)
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
                NEXUS_HdmiOutput_Drmif_P_PrintType1StaticMetadata(&pOldInfoFrame->metadata.typeSettings.type1);
                break;
            default:
                break;
        }
        BDBG_LOG(("      [new]"));
        switch (pNewInfoFrame->metadata.type)
        {
            case NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1:
                NEXUS_HdmiOutput_Drmif_P_PrintType1StaticMetadata(&pNewInfoFrame->metadata.typeSettings.type1);
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
                NEXUS_HdmiOutput_Drmif_P_PrintType1StaticMetadataChanges(
                    &pOldInfoFrame->metadata.typeSettings.type1,
                    &pNewInfoFrame->metadata.typeSettings.type1);
                break;
            default:
                break;
        }
    }
}
#endif

static void NEXUS_HdmiOutput_Drmif_P_ToMagnum(const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pNexus, BAVC_HDMI_DRMInfoFrame * pMagnum)
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
static void NEXUS_HdmiOutput_Drmif_P_Disable(NEXUS_HdmiOutputHandle hdmiOutput)
{
    BERR_Code rc = BERR_SUCCESS;
    BAVC_HDMI_DRMInfoFrame stDRMInfoFrame ;
    NEXUS_VideoEotf oldEotf;
    oldEotf = hdmiOutput->dynrng.drmif.outputInfoFrame.eotf;
    BDBG_LOG(("    DRMInfoFrame")) ;
    BDBG_LOG(("      eotf: %s -> %s",
        eotfStrings[oldEotf],
        eotfStrings[NEXUS_VideoEotf_eInvalid]));
    hdmiOutput->dynrng.drmif.outputInfoFrame.eotf = NEXUS_VideoEotf_eInvalid;
    BHDM_GetDRMInfoFramePacket(hdmiOutput->hdmHandle, &stDRMInfoFrame) ;
    NEXUS_HdmiOutput_Drmif_P_ToMagnum(&hdmiOutput->dynrng.drmif.outputInfoFrame, &stDRMInfoFrame);
    rc = BHDM_SetDRMInfoFramePacket(hdmiOutput->hdmHandle, &stDRMInfoFrame) ;
    if (rc) { BERR_TRACE(rc); hdmiOutput->dynrng.drmif.outputInfoFrame.eotf = oldEotf; }

    /* notify display that we've changed drminfoframe, but only call this if called from timer below */
    hdmiOutput->displaySettings.valid = false;
    NEXUS_HdmiOutput_P_NotifyDisplay(hdmiOutput);
}

static void NEXUS_HdmiOutput_Drmif_P_DisableTimerExpiration(void * pContext)
{
    NEXUS_HdmiOutputHandle hdmiOutput = pContext;
    if (!hdmiOutput->dynrng.drmif.offTimer) return; /* someone canceled early */
    BDBG_LOG(("DRMIF disable timer expired"));
    hdmiOutput->dynrng.drmif.offTimer = NULL;
    NEXUS_HdmiOutput_Drmif_P_Disable(hdmiOutput);
}

static const NEXUS_HdmiDynamicRangeMasteringInfoFrame DRMIF_ZERO =
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

NEXUS_Error NEXUS_HdmiOutput_Drmif_P_Set(NEXUS_HdmiOutputHandle hdmiOutput, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    BERR_Code rc = BERR_SUCCESS;
    BAVC_HDMI_DRMInfoFrame stDRMInfoFrame ;

    if (BKNI_Memcmp(&hdmiOutput->dynrng.drmif.outputInfoFrame, pDrmInfoFrame, sizeof(hdmiOutput->dynrng.drmif.outputInfoFrame)))
    {
        if (pDrmInfoFrame->eotf != NEXUS_VideoEotf_eSdr && pDrmInfoFrame->eotf != NEXUS_VideoEotf_eInvalid)
        {
            /*
             * the first time we see a non-SDR, non-Invalid eotf set to this
             * output, we set this flag and print changes from then on
             */
            hdmiOutput->dynrng.printDynrngChanges = true;
        }

#if NEXUS_DBV_SUPPORT
        if (pDrmInfoFrame->eotf == NEXUS_VideoEotf_eInvalid && hdmiOutput->dynrng.drmif.outputInfoFrame.eotf != NEXUS_VideoEotf_eInvalid)
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
            if (hdmiOutput->dynrng.dbv.supported)
            {
                BDBG_LOG(("Disabling DRMIF requires 2 seconds of transmittal with zeroes"));
                hdmiOutput->dynrng.drmif.offTimer = NEXUS_ScheduleTimer(2000, NEXUS_HdmiOutput_Drmif_P_DisableTimerExpiration, hdmiOutput);
                pDrmInfoFrame = &DRMIF_ZERO;
            }
        }
        else if (pDrmInfoFrame->eotf != NEXUS_VideoEotf_eInvalid && hdmiOutput->dynrng.drmif.offTimer)
        {
            NEXUS_CancelTimer(hdmiOutput->dynrng.drmif.offTimer);
            hdmiOutput->dynrng.drmif.offTimer = NULL;
        }
#endif

#if !BDBG_NO_LOG
        if (hdmiOutput->dynrng.printDynrngChanges)
        {
            BDBG_LOG(("NEXUS_HdmiOutput_P_SetDrmInfoFrame")) ;
            NEXUS_HdmiOutput_Drmif_P_PrintChanges(&hdmiOutput->dynrng.drmif.outputInfoFrame, pDrmInfoFrame);
        }
#endif
        BKNI_Memcpy(&hdmiOutput->dynrng.drmif.outputInfoFrame, pDrmInfoFrame, sizeof(hdmiOutput->dynrng.drmif.outputInfoFrame));

        BHDM_GetDRMInfoFramePacket(hdmiOutput->hdmHandle, &stDRMInfoFrame) ;
        NEXUS_HdmiOutput_Drmif_P_ToMagnum(pDrmInfoFrame, &stDRMInfoFrame);
        rc = BHDM_SetDRMInfoFramePacket(hdmiOutput->hdmHandle, &stDRMInfoFrame) ;
        if (rc) { BERR_TRACE(rc); goto error; }
    }

error:
    return rc;
}

static NEXUS_VideoEotf NEXUS_HdmiOutput_Drmif_P_ComputeEotf(
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

static void NEXUS_HdmiOutput_Drmif_P_ApplyEdid(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    /* for now, we only modify the eotf */
    pDrmInfoFrame->eotf = NEXUS_HdmiOutput_Drmif_P_ComputeEotf(pDrmInfoFrame->eotf, &hdmiOutput->dynrng.drmif.hdrdb,
        hdmiOutput->dynrng.processingCaps.typesSupported[NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_ePlm]);
    if (pDrmInfoFrame->eotf == NEXUS_VideoEotf_eSdr)
    {
        BKNI_Memset(&pDrmInfoFrame->metadata, 0, sizeof(pDrmInfoFrame->metadata)) ;
    }
}

static void NEXUS_HdmiOutput_Drmif_P_Build(NEXUS_HdmiDynamicRangeMasteringInfoFrame * pTargetDrmInfoFrame, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pSourceDrmInfoFrame)
{
    BKNI_Memset(pTargetDrmInfoFrame, 0, sizeof(*pTargetDrmInfoFrame));
    if (pSourceDrmInfoFrame->eotf != NEXUS_VideoEotf_eSdr)
    {
        BKNI_Memcpy(pTargetDrmInfoFrame, pSourceDrmInfoFrame, sizeof(*pTargetDrmInfoFrame));
    }
}

void NEXUS_HdmiOutput_Drmif_P_ApplyInput(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_HdmiDynamicRangeMasteringInfoFrame * pTargetDrmInfoFrame)
{
    if (hdmiOutput->dynrng.drmif.inputInfoFrame.eotf != NEXUS_VideoEotf_eInvalid)
    {
        NEXUS_HdmiOutput_Drmif_P_Build(pTargetDrmInfoFrame, &hdmiOutput->dynrng.drmif.inputInfoFrame);

        /* check compat with EDID -> modify as necessary and print debug */
        NEXUS_HdmiOutput_Drmif_P_ApplyEdid(hdmiOutput, pTargetDrmInfoFrame);

#if NEXUS_DBV_SUPPORT
        NEXUS_HdmiOutput_Dbv_P_UpdateDrmInfoFrame(hdmiOutput, pTargetDrmInfoFrame);
#endif
    }
    else
    {
        BDBG_MSG(("No input; Use override to apply HDR settings sans input."));
    }
}

void NEXUS_HdmiOutput_Drmif_P_SetSource(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_HdmiDynamicRangeMasteringInfoFrame * pTargetDrmInfoFrame)
{
    BDBG_ASSERT(hdmiOutput);

    if (hdmiOutput->extraSettings.overrideDynamicRangeMasteringInfoFrame) /* means we listen to what user wants and ignore edid */
    {
        NEXUS_HdmiOutput_Drmif_P_Build(pTargetDrmInfoFrame, &hdmiOutput->extraSettings.dynamicRangeMasteringInfoFrame);
        /*
         * extra settings drmif always uses units of 1 nit for max luma, but
         * internally we use units of 100 micronits where we can, so
         * convert to 100 micronits units from 1 nit units
         */
        pTargetDrmInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max *= 10000;
    }
    else
    {
        NEXUS_HdmiOutput_Drmif_P_ApplyInput(hdmiOutput, pTargetDrmInfoFrame);
    }
}

static void NEXUS_HdmiOutput_Drmif_P_UpdateRxCaps(NEXUS_HdmiOutputHandle hdmiOutput)
{
    hdmiOutput->extraStatus.dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eSdr].rx = hdmiOutput->dynrng.drmif.hdrdb.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eSDR];
    hdmiOutput->extraStatus.dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eHlg].rx = hdmiOutput->dynrng.drmif.hdrdb.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eHLG];
    hdmiOutput->extraStatus.dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eHdr10].rx = hdmiOutput->dynrng.drmif.hdrdb.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eSMPTESt2084];
}

bool NEXUS_HdmiOutput_Drmif_P_ConnectionChanged(NEXUS_HdmiOutputHandle hdmiOutput, bool force)
{
    BERR_Code rc = BERR_SUCCESS;
    bool changed = false;

    BDBG_MSG(("NEXUS_HdmiOutput_Drmif_P_ConnectionChanged: %s", hdmiOutput->dynrng.connected ? "connected" : "disconnected"));

    if (hdmiOutput->dynrng.connected)
    {
        BHDM_EDID_HDRStaticDB hdrdb;
        rc = BHDM_EDID_GetHdrStaticMetadatadb(hdmiOutput->hdmHandle, &hdrdb);
        if (rc) { BERR_TRACE(rc); goto error; }
        if (BKNI_Memcmp(&hdmiOutput->dynrng.drmif.hdrdb, &hdrdb, sizeof(hdmiOutput->dynrng.drmif.hdrdb)))
        {
            BKNI_Memcpy(&hdmiOutput->dynrng.drmif.hdrdb, &hdrdb, sizeof(hdmiOutput->dynrng.drmif.hdrdb));
            changed = true;
        }
    }
    else
    {
        BKNI_Memset(&hdmiOutput->dynrng.drmif.hdrdb, 0, sizeof(hdmiOutput->dynrng.drmif.hdrdb));
    }

    if (force || changed)
    {
        NEXUS_HdmiOutput_Drmif_P_UpdateRxCaps(hdmiOutput);
    }

error:
    return changed || force;
}

void NEXUS_HdmiOutput_Drmif_P_Init(NEXUS_HdmiOutputHandle hdmiOutput)
{
    /* set DRM InfoFrame eotf to invalid, so DRM InfoFrame will be updated at least once */
    hdmiOutput->dynrng.drmif.inputInfoFrame.eotf = NEXUS_VideoEotf_eInvalid ;
    hdmiOutput->dynrng.drmif.outputInfoFrame.eotf = NEXUS_VideoEotf_eInvalid ;
}

NEXUS_VideoEotf NEXUS_HdmiOutput_Drmif_P_GetOutputEotf(NEXUS_HdmiOutputHandle hdmiOutput)
{
    return hdmiOutput->dynrng.drmif.outputInfoFrame.eotf;
}

void NEXUS_HdmiOutput_Drmif_P_UpdateModeStatus(NEXUS_HdmiOutputHandle output, NEXUS_VideoDynamicRangeMode * pMode)
{
    switch (output->dynrng.drmif.outputInfoFrame.eotf)
    {
    default:
    case NEXUS_VideoEotf_eInvalid:
        *pMode = NEXUS_VideoDynamicRangeMode_eLegacy;
        break;
    case NEXUS_VideoEotf_eSdr:
        *pMode = NEXUS_VideoDynamicRangeMode_eSdr;
        break;
    case NEXUS_VideoEotf_eHlg:
        *pMode = NEXUS_VideoDynamicRangeMode_eHlg;
        break;
    case NEXUS_VideoEotf_ePq:
        *pMode = NEXUS_VideoDynamicRangeMode_eHdr10;
        break;
    }
}

void NEXUS_HdmiOutput_Drmif_P_SetInput(NEXUS_HdmiOutputHandle hdmiOutput,
    const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    if (BKNI_Memcmp(&hdmiOutput->dynrng.drmif.inputInfoFrame, pDrmInfoFrame, sizeof(hdmiOutput->dynrng.drmif.inputInfoFrame)))
    {
        BKNI_Memcpy(&hdmiOutput->dynrng.drmif.inputInfoFrame, pDrmInfoFrame, sizeof(hdmiOutput->dynrng.drmif.inputInfoFrame));
        hdmiOutput->displaySettings.valid = false;
    }
}
