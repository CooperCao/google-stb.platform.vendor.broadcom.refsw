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

BDBG_MODULE(nexus_hdmi_output_dynrng);

#define IS_AUTO_MODE(X) (\
    (X) == NEXUS_VideoDynamicRangeMode_eDefault \
    || \
    (X) == NEXUS_VideoDynamicRangeMode_eAuto \
    || \
    (X) == NEXUS_VideoDynamicRangeMode_eTrackInput \
)

#if !BDBG_NO_ERR
static const char * NEXUS_HdmiOutput_Dynrng_P_GetName(NEXUS_VideoDynamicRangeMode mode)
{
    static const char * dynrngStrings[] =
    {
        "Default",
        "Auto-select",
        "Track Input",
        "Legacy SDR",
        "SDR",
        "HDR10",
        "HLG",
        "HDR10+",
        "Dolby Vision",
        NULL
    };
    static const char * unknown = "Unknown";

    if (mode < NEXUS_VideoDynamicRangeMode_eMax)
    {
        return dynrngStrings[mode];
    }
    else
    {
        return unknown;
    }
}
#endif

static NEXUS_Error NEXUS_HdmiOutput_Dynrng_P_CheckTxSupport(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_VideoDynamicRangeMode dynamicRangeMode)
{
    if (!IS_AUTO_MODE(dynamicRangeMode))
    {
        /* only check support if not auto */
        if (!hdmiOutput->extraStatus.dynamicRangeModeSupported[dynamicRangeMode].tx)
        {
            return NEXUS_NOT_SUPPORTED;
        }
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_HdmiOutput_Dynrng_P_CheckDepthSupport(NEXUS_VideoDynamicRangeMode dynamicRangeMode, unsigned depth)
{
    if
    (
        depth < 10
        &&
        (
            dynamicRangeMode == NEXUS_VideoDynamicRangeMode_eHdr10Plus
            ||
            dynamicRangeMode == NEXUS_VideoDynamicRangeMode_eHdr10
            ||
            dynamicRangeMode == NEXUS_VideoDynamicRangeMode_eHlg
        )
    )
    {
        return NEXUS_NOT_SUPPORTED;
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_HdmiOutput_Dynrng_P_CheckRxSupport(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_VideoDynamicRangeMode dynamicRangeMode)
{
    if (!IS_AUTO_MODE(dynamicRangeMode))
    {
        /* only check support if not auto */
        if (!hdmiOutput->extraStatus.dynamicRangeModeSupported[dynamicRangeMode].rx)
        {
            return NEXUS_NOT_SUPPORTED;
        }
    }
    return NEXUS_SUCCESS;
}

static void NEXUS_HdmiOutput_Dynrng_P_PrintCaps(const char * tag, NEXUS_HdmiOutputHandle hdmiOutput)
{
    unsigned i;

    BDBG_MSG(("%s", tag));
    for (i = NEXUS_VideoDynamicRangeMode_eDefault; i < NEXUS_VideoDynamicRangeMode_eMax; i++)
    {
        BDBG_MSG(("%s tx: %c; rx: %c",
            NEXUS_HdmiOutput_Dynrng_P_GetName(i),
            hdmiOutput->extraStatus.dynamicRangeModeSupported[i].tx ? 'Y' : 'N',
            hdmiOutput->extraStatus.dynamicRangeModeSupported[i].rx ? 'Y' : 'N'));
    }
}

void NEXUS_HdmiOutput_Dynrng_P_ConnectionChanged(NEXUS_HdmiOutputHandle hdmiOutput)
{
    bool changed = false;
    bool connected = false;

    connected = hdmiOutput->rxState >= NEXUS_HdmiOutputState_eRxSenseCheck;

    if (hdmiOutput->dynrng.connected != connected)
    {
        hdmiOutput->dynrng.connected = connected;
        changed = true;
        BDBG_MSG(("NEXUS_HdmiOutput_Dynrng_P_ConnectionChanged: %s", connected ? "connected" : "disconnected"));
    }

    changed = NEXUS_HdmiOutput_Drmif_P_ConnectionChanged(hdmiOutput, changed) || changed;
#if NEXUS_DBV_SUPPORT
    changed = NEXUS_HdmiOutput_Dbv_P_ConnectionChanged(hdmiOutput, changed) || changed;
#endif

    if (changed)
    {
        NEXUS_HdmiOutput_Dynrng_P_PrintCaps(connected ? "rx connection event" : "rx disconnection event", hdmiOutput);
        /* general HDMI code already knows the connection event occurred and will notify display if we are connected */
    }
}

void NEXUS_HdmiOutput_Dynrng_P_UpdateVendorSpecificInfoFrame(NEXUS_HdmiOutputHandle hdmiOutput, BAVC_HDMI_VendorSpecificInfoFrame * pVSIF)
{
#if NEXUS_DBV_SUPPORT
    NEXUS_HdmiOutput_Dbv_P_UpdateVendorSpecificInfoFrame(hdmiOutput, pVSIF);
#else
    BSTD_UNUSED(hdmiOutput);
    BSTD_UNUSED(pVSIF);
#endif
}

void NEXUS_HdmiOutput_Dynrng_P_UpdateAviInfoFrameSettings(NEXUS_HdmiOutputHandle hdmiOutput, BAVC_HDMI_AviInfoFrame * pAVIIF)
{
#if NEXUS_DBV_SUPPORT
    NEXUS_HdmiOutput_Dbv_P_UpdateAviInfoFrameSettings(hdmiOutput, pAVIIF);
#else
    BSTD_UNUSED(hdmiOutput);
    BSTD_UNUSED(pAVIIF);
#endif
}

void NEXUS_HdmiOutput_Dynrng_P_UpdateAviInfoFrameStatus(NEXUS_HdmiOutputHandle hdmiOutput, BAVC_HDMI_AviInfoFrame * pAVIIF)
{
#if NEXUS_DBV_SUPPORT
    NEXUS_HdmiOutput_Dbv_P_UpdateAviInfoFrameStatus(hdmiOutput, pAVIIF);
#else
    BSTD_UNUSED(hdmiOutput);
    BSTD_UNUSED(pAVIIF);
#endif
}

void NEXUS_HdmiOutput_Dynrng_P_Init(NEXUS_HdmiOutputHandle hdmiOutput)
{
    NEXUS_HdmiOutput_Drmif_P_Init(hdmiOutput);
}

NEXUS_VideoEotf NEXUS_HdmiOutput_Dynrng_P_GetOutputEotf(NEXUS_HdmiOutputHandle hdmiOutput)
{
    return NEXUS_HdmiOutput_Drmif_P_GetOutputEotf(hdmiOutput);
}

void NEXUS_HdmiOutput_Dynrng_P_InitStatus(NEXUS_HdmiOutputHandle hdmiOutput)
{
    unsigned i;
    /* all our transmitters support default through HLG */
    for (i = NEXUS_VideoDynamicRangeMode_eDefault; i <= NEXUS_VideoDynamicRangeMode_eHlg; i++)
    {
        hdmiOutput->extraStatus.dynamicRangeModeSupported[i].tx = true;
    }
    hdmiOutput->extraStatus.dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eLegacy].rx = true;
    /* updated on connect, TODO: could come from module-level capabilities from display */
    hdmiOutput->extraStatus.dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eDolbyVision].tx = false;
#if NEXUS_HDR10PLUS_SUPPORT
    hdmiOutput->extraStatus.dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eHdr10Plus].tx = true;
#endif

    /* all receivers support legacy */
    hdmiOutput->extraStatus.dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eLegacy].rx = true;

    NEXUS_HdmiOutput_Dynrng_P_PrintCaps("capabilities init", hdmiOutput);
}

#define SUPPORTED(X) \
    hdmiOutput->extraStatus.dynamicRangeModeSupported[X].rx \
    && \
    hdmiOutput->extraStatus.dynamicRangeModeSupported[X].tx

static void NEXUS_HdmiOutput_Dynrng_P_BasicAutoResolution(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_HdmiOutputDisplaySettings * pDisplaySettings)
{
    /*
     * This priority list implements a simple automatic dynamic range mode
     * selection algorithm based on TV/rx support *only*.
     * For examples of automatic selection algorithms involving technologies
     * not in this simple list, please see the nxclient application examples.
     */
    static NEXUS_VideoDynamicRangeMode priorities[] =
    {
        NEXUS_VideoDynamicRangeMode_eHdr10Plus, /* HDR10+ mode falls back to HDR10 if no HDR10+ metadata present */
        NEXUS_VideoDynamicRangeMode_eHdr10, /* HDR10 supports all video sources and usages */
        NEXUS_VideoDynamicRangeMode_eHlg, /* HLG mode is for TVs that support HLG but do not support HDR10. HLG mode is also used in HDR HDMI conformance testing. */
        NEXUS_VideoDynamicRangeMode_eSdr, /* SDR mode is for TVs that have an HDRDB EDID, but only support SDR. SDR mode is also used in HDR HDMI conformance testing. */
        NEXUS_VideoDynamicRangeMode_eLegacy /* Legacy mode is for TVs that don't have HDRDB EDID */
    };
    unsigned i;

    pDisplaySettings->dynamicRangeMode = NEXUS_VideoDynamicRangeMode_eLegacy;

    for (i = 0; i < sizeof(priorities)/sizeof(priorities[0]); i++)
    {
        if (SUPPORTED(priorities[i]))
        {
            if (NEXUS_HdmiOutput_Dynrng_P_CheckDepthSupport(priorities[i], pDisplaySettings->colorDepth) == NEXUS_NOT_SUPPORTED)
            {
                continue;
            }
            pDisplaySettings->dynamicRangeMode = priorities[i];
            break;
        }
    }
}

#define IS_DOLBY_VISION(dynrng) \
    ((dynrng) == NEXUS_VideoDynamicRangeMode_eDolbyVision)

void NEXUS_HdmiOutput_Dynrng_P_ResolveMode(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_HdmiOutputDisplaySettings * pDisplaySettings)
{
    NEXUS_VideoDynamicRangeMode desiredDynrng = pDisplaySettings->dynamicRangeMode;

    bool txSupport = (NEXUS_SUCCESS==NEXUS_HdmiOutput_Dynrng_P_CheckTxSupport(hdmiOutput, desiredDynrng));
    bool rxSupport = (NEXUS_SUCCESS==NEXUS_HdmiOutput_Dynrng_P_CheckRxSupport(hdmiOutput, desiredDynrng));
    bool depthSupport = (NEXUS_SUCCESS==NEXUS_HdmiOutput_Dynrng_P_CheckDepthSupport(desiredDynrng, pDisplaySettings->colorDepth));
#if NEXUS_DBV_SUPPORT
    bool dbvSupport = !(IS_DOLBY_VISION(desiredDynrng) && NEXUS_HdmiOutput_Dbv_P_IsUnsupportedFormat(hdmiOutput, pDisplaySettings->format));
#endif

    if (hdmiOutput->dynrng.connected)
    {
        if (!hdmiOutput->dynrng.processingCaps.typesSupported[NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_ePlm])
        {
            if (desiredDynrng == NEXUS_VideoDynamicRangeMode_eDefault || desiredDynrng == NEXUS_VideoDynamicRangeMode_eAuto)
            {
                /* on non-PLM platforms, we want the old-style track-input algo */
                desiredDynrng = NEXUS_VideoDynamicRangeMode_eTrackInput;
            }
        }

        switch (desiredDynrng)
        {
        case NEXUS_VideoDynamicRangeMode_eDefault:
            if (!hdmiOutput->extraSettings.overrideDynamicRangeMasteringInfoFrame)
            {
                pDisplaySettings->dynamicRangeMode = NEXUS_VideoDynamicRangeMode_eLegacy;
            }
            break;
        case NEXUS_VideoDynamicRangeMode_eAuto:
            if (!hdmiOutput->extraSettings.overrideDynamicRangeMasteringInfoFrame)
            {
                NEXUS_HdmiOutput_Dynrng_P_BasicAutoResolution(hdmiOutput, pDisplaySettings);
            }
            break;
        case NEXUS_VideoDynamicRangeMode_eTrackInput:
            /* pass it on to drmif resolution */
            break;
        default:
            if (!txSupport || !rxSupport || !depthSupport
#if NEXUS_DBV_SUPPORT
                || !dbvSupport
#endif
            )
            {
                NEXUS_HdmiOutput_Dynrng_P_BasicAutoResolution(hdmiOutput, pDisplaySettings);
                if (pDisplaySettings->dynamicRangeMode != hdmiOutput->dynrng.outputMode) {
                    if (!txSupport) {
                        BDBG_ERR(("This product does not support dynamic range mode: %s", NEXUS_HdmiOutput_Dynrng_P_GetName(desiredDynrng)));
                    }
                    else if (!rxSupport) {
                        BDBG_ERR(("The attached receiver does not support dynamic range mode: %s", NEXUS_HdmiOutput_Dynrng_P_GetName(desiredDynrng)));
                    }
                    else if (!depthSupport) {
                        BDBG_ERR(("The dynamic range mode %s is not supported in color depth %u", NEXUS_HdmiOutput_Dynrng_P_GetName(desiredDynrng), pDisplaySettings->colorDepth));
                    }
                    else {
                        BDBG_ERR(("The requested dynamic range mode and video format combo is not supported: %s", NEXUS_HdmiOutput_Dynrng_P_GetName(desiredDynrng)));
                    }
                }
            }
            break;
        }
    }
    else {
        pDisplaySettings->dynamicRangeMode = NEXUS_VideoDynamicRangeMode_eLegacy;
    }

    BDBG_MSG(("resolved %s -> %s", NEXUS_HdmiOutput_Dynrng_P_GetName(desiredDynrng), NEXUS_HdmiOutput_Dynrng_P_GetName(pDisplaySettings->dynamicRangeMode)));

#if NEXUS_DBV_SUPPORT
    NEXUS_HdmiOutput_Dbv_P_UpdateDisplaySettings(hdmiOutput, pDisplaySettings);
#endif
}


static NEXUS_Error NEXUS_HdmiOutput_Dynrng_P_Set(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_VideoDynamicRangeMode dynamicRangeMode)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiDynamicRangeMasteringInfoFrame drmInfoFrame;

    BDBG_ASSERT(hdmiOutput);
    BKNI_Memset(&drmInfoFrame, 0, sizeof(drmInfoFrame));

#if NEXUS_DBV_SUPPORT
    /* must disable DBV first if going out of that mode */
    if (hdmiOutput->dynrng.outputMode == NEXUS_VideoDynamicRangeMode_eDolbyVision && dynamicRangeMode != NEXUS_VideoDynamicRangeMode_eDolbyVision)
    {
        rc = NEXUS_HdmiOutput_Dbv_P_Set(hdmiOutput, false);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }
#endif

    /* explicit dynrng modes take precedence over the override */
    switch (dynamicRangeMode)
    {
    case NEXUS_VideoDynamicRangeMode_eLegacy:
        drmInfoFrame.eotf = NEXUS_VideoEotf_eInvalid;
        break;
    case NEXUS_VideoDynamicRangeMode_eSdr:
        drmInfoFrame.eotf = NEXUS_VideoEotf_eSdr;
        break;
    case NEXUS_VideoDynamicRangeMode_eHdr10:
    case NEXUS_VideoDynamicRangeMode_eHdr10Plus:
        /* copy input metadata if valid */
        NEXUS_HdmiOutput_Drmif_P_ApplyInput(hdmiOutput, &drmInfoFrame);
        /* then override eotf to PQ */
        drmInfoFrame.eotf = NEXUS_VideoEotf_ePq;
        break;
    case NEXUS_VideoDynamicRangeMode_eHlg:
        drmInfoFrame.eotf = NEXUS_VideoEotf_eHlg;
        break;
#if NEXUS_DBV_SUPPORT
    case NEXUS_VideoDynamicRangeMode_eDolbyVision:
        rc = NEXUS_HdmiOutput_Dbv_P_Set(hdmiOutput, true);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
        NEXUS_HdmiOutput_Dbv_P_UpdateDrmInfoFrame(hdmiOutput, &drmInfoFrame);
        break;
#endif
    case NEXUS_VideoDynamicRangeMode_eTrackInput:
        NEXUS_HdmiOutput_Drmif_P_ApplyInput(hdmiOutput, &drmInfoFrame);
        break;
    default:
    case NEXUS_VideoDynamicRangeMode_eDefault: /* if override is set in extra settings it is possible after resolution to still have auto or default */
    case NEXUS_VideoDynamicRangeMode_eAuto:
        NEXUS_HdmiOutput_Drmif_P_SetSource(hdmiOutput, &drmInfoFrame);
        break;
    }

    rc = NEXUS_HdmiOutput_Drmif_P_Set(hdmiOutput, &drmInfoFrame);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    hdmiOutput->dynrng.outputMode = NEXUS_HdmiOutput_GetDynamicRangeModeStatus_priv(hdmiOutput);
    BDBG_MSG(("set %s", NEXUS_HdmiOutput_Dynrng_P_GetName(hdmiOutput->dynrng.outputMode)));

error:
    return rc;
}

#if 0 /* TODO: we always send zeroes for now, may need to resurrect this code later */
static bool NEXUS_HdmiOutput_Dynrng_P_HasChangedMetadata(NEXUS_HdmiOutputHandle hdmiOutput)
{
    bool changed = false;

    switch (hdmiOutput->dynrng.userMode)
    {
    case NEXUS_VideoDynamicRangeMode_eLegacy:
    case NEXUS_VideoDynamicRangeMode_eSdr:
    case NEXUS_VideoDynamicRangeMode_eHlg:
        break;
    default:
        changed =
        (
            !hdmiOutput->extraSettings.overrideDynamicRangeMasteringInfoFrame
            &&
            BKNI_Memcmp(&hdmiOutput->dynrng.drmif.inputInfoFrame.metadata, &hdmiOutput->dynrng.drmif.outputInfoFrame.metadata, sizeof(hdmiOutput->dynrng.drmif.outputInfoFrame.metadata))
        )
        ||
        (
            hdmiOutput->extraSettings.overrideDynamicRangeMasteringInfoFrame
            &&
            BKNI_Memcmp(&hdmiOutput->extraSettings.dynamicRangeMasteringInfoFrame.metadata, &hdmiOutput->dynrng.drmif.outputInfoFrame.metadata, sizeof(hdmiOutput->dynrng.drmif.outputInfoFrame.metadata))
        );
        break;
    }

    return changed;
}
#endif

NEXUS_Error NEXUS_HdmiOutput_Dynrng_P_ReapplyVsif(NEXUS_HdmiOutputHandle hdmiOutput)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiVendorSpecificInfoFrame vsif;
    NEXUS_HdmiOutput_GetVendorSpecificInfoFrame(hdmiOutput, &vsif);
    rc = NEXUS_HdmiOutput_SetVendorSpecificInfoFrame(hdmiOutput, &vsif);
    if (rc) { rc = BERR_TRACE(rc); goto end; }
end:
    return rc;
}

NEXUS_Error NEXUS_HdmiOutput_Dynrng_P_SetMode(NEXUS_HdmiOutputHandle hdmiOutput, NEXUS_VideoDynamicRangeMode dynamicRangeMode)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ASSERT(hdmiOutput);

    if (dynamicRangeMode == hdmiOutput->dynrng.outputMode && (dynamicRangeMode != NEXUS_VideoDynamicRangeMode_eTrackInput))
    {
        rc = NEXUS_SUCCESS;
        goto end;
    }

    rc = NEXUS_HdmiOutput_Dynrng_P_Set(hdmiOutput, dynamicRangeMode);
    if (rc) { rc = BERR_TRACE(rc); goto end; }

end:
    return rc;
}

NEXUS_Error NEXUS_HdmiOutput_SetDisplayDynamicRangeProcessingCapabilities_priv(NEXUS_HdmiOutputHandle output,
    const NEXUS_HdmiOutputDisplayDynamicRangeProcessingCapabilities * pCaps)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if (BKNI_Memcmp(&output->dynrng.processingCaps, pCaps, sizeof(output->dynrng.processingCaps)))
    {
        BKNI_Memcpy(&output->dynrng.processingCaps, pCaps, sizeof(output->dynrng.processingCaps));
        output->extraStatus.dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eDolbyVision].tx =
            output->dynrng.processingCaps.typesSupported[NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_eDolbyVision];
#if NEXUS_HAS_SAGE
        /* HasLicensedFeature uses magnum return code instead of bool, so boolean logic is inverted -> better to check against return code itself */
        output->extraStatus.dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eDolbyVision].tx =
            output->extraStatus.dynamicRangeModeSupported[NEXUS_VideoDynamicRangeMode_eDolbyVision].tx
            &&
            (BCHP_HasLicensedFeature_isrsafe(g_pCoreHandles->chp, BCHP_LicensedFeature_eDolbyVision) == BERR_SUCCESS);
#endif

        NEXUS_HdmiOutput_Dynrng_P_PrintCaps("display capabilities change event", output);

        /* ensure we don't make any decisions based on display settings until display updates them */
        NEXUS_HdmiOutput_P_SetDisplaySettingsValidity(output, false);
        /* notify display that HDMI knows that display caps have been updated, which should re-set the display settings */
        NEXUS_HdmiOutput_P_NotifyDisplay(output);
    }

    return rc;
}

NEXUS_VideoDynamicRangeMode NEXUS_HdmiOutput_GetDynamicRangeModeStatus_priv(NEXUS_HdmiOutputHandle hdmiOutput)
{
    NEXUS_VideoDynamicRangeMode mode;
    NEXUS_HdmiOutput_Drmif_P_UpdateModeStatus(hdmiOutput, &mode);
#if NEXUS_DBV_SUPPORT
    NEXUS_HdmiOutput_Dbv_P_UpdateModeStatus(hdmiOutput, &mode);
#endif
    return mode;
}

void NEXUS_HdmiOutput_SetInputDrmInfoFrame_priv(NEXUS_HdmiOutputHandle hdmiOutput,
    const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pDrmInfoFrame)
{
    NEXUS_HdmiOutput_Drmif_P_SetInput(hdmiOutput, pDrmInfoFrame);
}

void NEXUS_HdmiOutput_GetDynrngEdidBytes_priv(NEXUS_HdmiOutputHandle hdmiOutput, uint8_t * bytes)
{
#if NEXUS_DBV_SUPPORT
    NEXUS_HdmiOutput_Dbv_P_GetEdidBytes(hdmiOutput, bytes);
#else
    BSTD_UNUSED(hdmiOutput);
    BSTD_UNUSED(bytes);
#endif
}
