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
#include "nexus_base.h"
#include "nexus_display_module.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output_hdcp.h"
#include "priv/nexus_hdmi_output_priv.h"
#endif
BDBG_MODULE(nexus_video_output_hdmi);

#if NEXUS_HAS_HDMI_OUTPUT

void NEXUS_VideoOutputs_P_HdmiCfcHeap(BVDC_Display_HdmiSettings *pSettings)
{
    /* cfc LUT heap */
    if(NEXUS_MAX_HEAPS != g_NEXUS_DisplayModule_State.moduleSettings.cfc.vecHeapIndex[pSettings->ulPortId-BVDC_Hdmi_0]) {
        pSettings->hCfcHeap = g_pCoreHandles->heap[g_NEXUS_DisplayModule_State.moduleSettings.cfc.vecHeapIndex[pSettings->ulPortId-BVDC_Hdmi_0]].mma;
    }
}

static void
NEXUS_VideoOutput_P_HdmiRateChange_isr(NEXUS_DisplayHandle display, void *pParam)
{
    NEXUS_HdmiOutputHandle hdmi = pParam;
    BDBG_ASSERT(NULL != hdmi);
    NEXUS_HdmiOutput_VideoRateChange_isr(hdmi, &display->hdmi.rateInfo);
}

static BERR_Code
NEXUS_VideoOutput_P_HdmiNum_ToMagnum(unsigned index, uint32_t *ulHdmi)
{
    BERR_Code rc = BERR_SUCCESS;
    switch(index) {
    case 0: *ulHdmi = BVDC_Hdmi_0; break;
#if NEXUS_NUM_HDMI_OUTPUTS > 1
    case 1: *ulHdmi = BVDC_Hdmi_1; break;
#endif
    default: rc = BERR_TRACE(BERR_INVALID_PARAMETER); break;
    }
    return rc;
}


/* Set HDMI settings to a legal default configuration; avoids errors due to changed formats */
static void NEXUS_VideoOutput_P_SetDefaultHdmiSettings(NEXUS_DisplayHandle display)
{
    BERR_Code rc ;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    BVDC_Display_GetHdmiSettings(display->displayVdc, &display->hdmi.vdcSettings) ;
    /* disable data to HDMI core */
    display->hdmi.vdcSettings.ulPortId      = display->hdmi.vdcIndex;
    display->hdmi.vdcSettings.eMatrixCoeffs = BAVC_MatrixCoefficients_eUnknown;

    /* color depth */
    rc = BVDC_Display_SetHdmiColorDepth(display->displayVdc, BAVC_HDMI_BitsPerPixel_e24bit) ;
    if (rc) {
        BERR_TRACE(rc) ;
        (void)BVDC_AbortChanges(video->vdc) ;
        goto done ;
    }

    /* color space */
    display->hdmi.vdcSettings.eColorComponent = BAVC_Colorspace_eYCbCr444 ;

    /* cfc LUT heap */
    NEXUS_VideoOutputs_P_HdmiCfcHeap(&display->hdmi.vdcSettings);
    rc = BVDC_Display_SetHdmiSettings(display->displayVdc, &display->hdmi.vdcSettings) ;
    if (rc) {
        BERR_TRACE(rc) ;
        (void)BVDC_AbortChanges(video->vdc) ;
        goto done ;
    }

    rc = BVDC_ApplyChanges(video->vdc);
    rc = BERR_TRACE(rc);

done:
    return ;
}

void NEXUS_VideoOutput_P_SetHdrSettings(NEXUS_HdmiOutputHandle hdmiOutput,
    const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pInputDrmInfoFrame)
{
    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
    NEXUS_HdmiOutput_SetInputDrmInfoFrame_priv(hdmiOutput, pInputDrmInfoFrame);
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
}

static void NEXUS_P_DynamicRangeProcessingCapabilitiesFromMagnum(const BVDC_Window_Capabilities * pVdc, NEXUS_HdmiOutputDisplayDynamicRangeProcessingCapabilities * pNexus)
{
    BKNI_Memset(pNexus, 0, sizeof(*pNexus));
    pNexus->typesSupported[NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_ePlm] = pVdc->bConvHdr10;
    pNexus->typesSupported[NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_eDolbyVision] = pVdc->bDolby;
    pNexus->typesSupported[NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_eTechnicolorPrime] = pVdc->bTchInput;
}

static const char * const dynrngProcessingStrings[NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_eMax + 1] =
{
    "PLM",
    "DBV",
    "TCH",
    NULL
};

void NEXUS_VideoOutput_P_InitDisplayDynamicRangeProcessingCapabilities(NEXUS_HdmiOutputDisplayDynamicRangeProcessingCapabilities * pCaps)
{
    unsigned j;
    BKNI_Memset(pCaps, 0, sizeof(*pCaps));
    /* default to supported */
    for (j = 0; j < NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_eMax; j++)
    {
        pCaps->typesSupported[j] = true;
    }
}

NEXUS_Error NEXUS_VideoOutput_P_UpdateDisplayDynamicRangeProcessingCapabilities(NEXUS_DisplayHandle display)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;
    unsigned j;
    bool openWindows;
    NEXUS_HdmiOutputHandle hdmiOutput = display->hdmi.outputNotify;
    NEXUS_HdmiOutputDisplayDynamicRangeProcessingCapabilities mergedCaps;

    if (!hdmiOutput) return NEXUS_SUCCESS;

    NEXUS_VideoOutput_P_InitDisplayDynamicRangeProcessingCapabilities(&mergedCaps);
    openWindows = false;

    for (i = 0; i < NEXUS_NUM_VIDEO_WINDOWS; i++)
    {
        if (display->windows[i].vdcState.window)
        {
            NEXUS_HdmiOutputDisplayDynamicRangeProcessingCapabilities caps;
            openWindows = true;
            NEXUS_P_DynamicRangeProcessingCapabilitiesFromMagnum(&display->windows[i].vdcState.caps, &caps);
            /* if any window active on this display *doesn't* support, then mergedCaps should also not support */
            for (j = 0; j < NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_eMax; j++)
            {
                BDBG_MSG(("window%d %s support: %s", i, dynrngProcessingStrings[j], caps.typesSupported[j] ? "yes" : "no"));
                mergedCaps.typesSupported[j] = mergedCaps.typesSupported[j] && caps.typesSupported[j];
            }
        }
    }

    if (!openWindows)
    {
        BDBG_MSG(("No windows attached to this display, dynrng caps reported based on display capabilities window 0"));
        for (j = 0; j < NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_eMax; j++)
        {
            mergedCaps.typesSupported[j] = g_NEXUS_DisplayModule_State.cap.display[display->index].window[0].dynamicRangeProcessing.typesSupported[j];
        }
    }

    for (j = 0; j < NEXUS_HdmiOutputDisplayDynamicRangeProcessingType_eMax; j++)
    {
        BDBG_MSG(("display %s support: %s", dynrngProcessingStrings[j], mergedCaps.typesSupported[j] ? "yes" : "no"));
    }

    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
        rc = NEXUS_HdmiOutput_SetDisplayDynamicRangeProcessingCapabilities_priv(hdmiOutput, &mergedCaps);
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
    if (rc) { BERR_TRACE(rc); goto error; }

error:
    return rc;
}

static BVDC_DisplayPriorityMode NEXUS_P_PriorityMode_ToMagnum_isrsafe(NEXUS_DisplayPriority priority)
{
    switch (priority) {
    case NEXUS_DisplayPriority_eGraphics: return BVDC_DisplayPriorityMode_eGraphics;
    case NEXUS_DisplayPriority_eVideo:    return BVDC_DisplayPriorityMode_eVideo;
    default:                                                return BVDC_DisplayPriorityMode_eAuto;
    }
}

/* TODO: move to video output and compute from display dynrng status */
static void NEXUS_VideoOutput_P_UpdateVdcDynrngParams(
    NEXUS_DisplayHandle display,
    const NEXUS_VideoOutput_P_FormatChangeParams * pParams,
    BVDC_Display_HdmiSettings * pDisplayHdmiSettings /* in/out - this function may not set all members of this structure */
)
{
    pDisplayHdmiSettings->bDolbyVisionEnabled = false;

    /* dynrng mode status must be set before using this function */
    switch (display->status.dynamicRangeMode)
    {
    case NEXUS_VideoDynamicRangeMode_eMax:
    case NEXUS_VideoDynamicRangeMode_eDefault:
    case NEXUS_VideoDynamicRangeMode_eAuto:
    case NEXUS_VideoDynamicRangeMode_eTrackInput:
    default:
        /* do nothing, we haven't resolved yet */
        break;

    case NEXUS_VideoDynamicRangeMode_eLegacy:
    case NEXUS_VideoDynamicRangeMode_eSdr:
        pDisplayHdmiSettings->eEotf = BAVC_HDMI_DRM_EOTF_eSDR;
        break;
    case NEXUS_VideoDynamicRangeMode_eHdr10Plus:
    case NEXUS_VideoDynamicRangeMode_eHdr10:
        pDisplayHdmiSettings->eEotf = BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084;
        break;
    case NEXUS_VideoDynamicRangeMode_eHlg:
        pDisplayHdmiSettings->eEotf = BAVC_HDMI_DRM_EOTF_eHLG;
        break;
    case NEXUS_VideoDynamicRangeMode_eDolbyVision:
        pDisplayHdmiSettings->bDolbyVisionEnabled = true;
        pDisplayHdmiSettings->eEotf = BAVC_HDMI_DRM_EOTF_eSDR;
        break;
#endif
    }

    pDisplayHdmiSettings->ePriorityMode = NEXUS_P_PriorityMode_ToMagnum_isrsafe(pParams->priority);
    BDBG_MSG(("display priority mode is %d", pDisplayHdmiSettings->ePriorityMode));
    pDisplayHdmiSettings->ulMinLuminance = 0; /* TODO: eventually load from HDMI HDRDB */
    pDisplayHdmiSettings->ulMaxLuminance = 0; /* TODO: eventually load from HDMI HDRDB */
}

static NEXUS_Error NEXUS_VideoOutput_P_ApplyVdcHdmiSettings(
    const NEXUS_VideoOutput_P_FormatChangeParams * pParams,
    const NEXUS_HdmiOutputDisplaySettings * pDisplaySettings
)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_DisplayHandle display ;

    display = pParams->display ;

    rc = BVDC_Display_SetHdmiColorDepth(display->displayVdc, NEXUS_P_HdmiColorDepth_ToMagnum_isrsafe(pDisplaySettings->colorDepth));
    if (rc) return BERR_TRACE(rc);

    display->hdmi.vdcSettings.ulPortId      = display->hdmi.vdcIndex;
    display->hdmi.vdcSettings.eMatrixCoeffs = NEXUS_P_MatrixCoefficients_ToMagnum_isrsafe(pDisplaySettings->colorimetry) ;
    display->hdmi.vdcSettings.eColorComponent = NEXUS_P_ColorSpace_ToMagnum_isrsafe(pDisplaySettings->colorSpace) ;
    display->hdmi.vdcSettings.eColorRange = NEXUS_P_ColorRange_ToMagnum_isrsafe(pDisplaySettings->colorRange) ;
    NEXUS_VideoOutputs_P_HdmiCfcHeap(&display->hdmi.vdcSettings);
    display->hdmi.vdcSettings.bBlendInIpt = true; /* if needed, add to display private API for experimentation later */
    NEXUS_VideoOutput_P_UpdateVdcDynrngParams(display, pParams, &display->hdmi.vdcSettings);
    rc = BVDC_Display_SetHdmiSettings(display->displayVdc, &display->hdmi.vdcSettings) ;
    if (rc) return BERR_TRACE(rc);

    return rc;
}

#define IS_DOLBY_VISION(dynrng) \
    ((dynrng) == NEXUS_VideoDynamicRangeMode_eDolbyVision)

/*
three entry points:
1) NEXUS_HdmiOutput_SetSettings
        where display->cfg is unchanged, called via notifyDisplay callback then NEXUS_VideoOutput_P_SetHdmiSettings
2) NEXUS_Display_AddOutput
        see NEXUS_VideoOutput_P_ConnectHdmi,
        also called via NEXUS_VideoOutput_P_SetHdmiSettings but with a forced format change
3) NEXUS_Display_SetSettings
        where NEXUS_HdmiOutput_SetSettings is unchanged, but new format is passed as params

the params struct could contain current or new display settings. use display->cfg to test for change.
*/
static BERR_Code
NEXUS_VideoOutput_P_ApplyHdmiSettings(void *output, const NEXUS_VideoOutput_P_FormatChangeParams * pParams)
{
    BERR_Code rc;
    NEXUS_HdmiOutputHandle hdmiOutput = output;
    NEXUS_HdmiOutputSettings settings;
    NEXUS_HdmiOutputStatus *hdmiOutputStatus = NULL;

    BFMT_VideoFmt videoFmt = BFMT_VideoFmt_eNTSC, hdmiFmt = BFMT_VideoFmt_eNTSC;
    BFMT_AspectRatio aspectRatioVdc = BFMT_AspectRatio_eUnknown;
    bool hdmiMasterMode = false;
    bool hdmiFormatChange = false;
    BVDC_Display_DvoSettings dvoSettings;
    bool reconnect_outputs = false;
    NEXUS_HdmiOutputDisplaySettings oldDisplaySettings;
    NEXUS_HdmiOutputDisplaySettings newDisplaySettings;
    bool postFormatChangeNeeded = false;
    NEXUS_DisplayHandle display = NULL;

    display = pParams->display;
    hdmiOutputStatus = &g_NEXUS_DisplayModule_State.functionData.NEXUS_VideoOutput_P_SetHdmiFormat.hdmiOutputStatus;

    if (display->hdmi.rateChangeCb_isr == NULL)
    {
        BDBG_MSG(("HDMI output is disconnected from display. Settings not applied to HDMI device")) ;
        return NEXUS_SUCCESS;
    }
    BDBG_ASSERT(display->hdmi.outputNotify == hdmiOutput);

    BVDC_Display_GetHdmiSettings(display->displayVdc, &display->hdmi.vdcSettings) ;
    NEXUS_HdmiOutput_GetSettings(hdmiOutput, &settings);
    rc = NEXUS_HdmiOutput_GetStatus(hdmiOutput, hdmiOutputStatus);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    hdmiMasterMode = display->timingGenerator == NEXUS_DisplayTimingGenerator_eHdmiDvo ;

    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
        NEXUS_HdmiOutput_GetDisplaySettings_priv(hdmiOutput, &oldDisplaySettings);
        BKNI_Memcpy(&newDisplaySettings, &oldDisplaySettings, sizeof(newDisplaySettings));
        newDisplaySettings.format = pParams->format;
        newDisplaySettings.dynamicRangeMode = pParams->dynamicRangeMode;
        rc = NEXUS_HdmiOutput_ResolveDisplaySettings_priv(hdmiOutput,
            &settings, display->cfg.xvYccEnabled, hdmiMasterMode,
            g_NEXUS_DisplayModule_State.requiredOutputSystem, &newDisplaySettings);
        if (rc)
        {
            /* If there is no HDMI Rx connected to the Tx; disable the HDMI Output */
            /* reset VDC HDMI settings to values compatible with analog settings */

            /* 7563[5] has a clockgen bug which requires fixed usage of displays and timing generators. */
            /* This is handled using the DISPLAY_OPEN_REQUIRES_HDMI_OUTPUT define and g_NEXUS_DisplayModule_State.requiredOutput. */
            /* Calling this on start up on 7563[5] causes the video window creation to fail, or errors when we pretend to remove HDMI. */

            if (!hdmiOutputStatus->connected && !g_NEXUS_DisplayModule_State.requiredOutputSystem && !hdmiMasterMode)
            {
                BDBG_MSG(("No HDMI Rx connected.. high-compatibility settings will be applied")) ;
                /* new HDMI settings will be applied when Rx device is connected  */
                newDisplaySettings.colorDepth = 8;
                newDisplaySettings.colorRange = NEXUS_ColorRange_eLimited;
                newDisplaySettings.colorSpace = NEXUS_ColorSpace_eYCbCr444;
                newDisplaySettings.colorimetry = NEXUS_MatrixCoefficients_eDvi_Full_Range_RGB; /* TODO: this is what was used in previous code, is it still valid? */
                newDisplaySettings.dynamicRangeMode = NEXUS_VideoDynamicRangeMode_eDefault;
                newDisplaySettings.format = NEXUS_VideoFormat_eNtsc;
            }
            else
            {
                rc = BERR_TRACE(rc); goto unlock;
            }
        }
        rc = NEXUS_HdmiOutput_SetDisplaySettings_priv(hdmiOutput, &newDisplaySettings) ;
        if (rc) { rc = BERR_TRACE(rc); goto unlock; }
        display->status.dynamicRangeMode = NEXUS_HdmiOutput_GetDynamicRangeModeStatus_priv(hdmiOutput);
        NEXUS_HdmiOutput_GetDynrngEdidBytes_priv(hdmiOutput, display->hdmi.vdcSettings.aucVsvdbBytes);
unlock:
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
#if 0
    if (rc) {
        /*
         * keep going, there is a condition that is not covered by above code where
         * we must apply *some* VDC settings on 7563(5) that we will fail if we
         * return here early.
         * TODO: find out exact condition required and modify 7563(5) code above to implement it
         */
    }
#endif

    /* certain settings, like color depth and color space, ought to trigger a format change */
    hdmiFormatChange =
    (
        display->hdmi.forceFormatChange
        ||
        pParams->format != display->cfg.format
        ||
        pParams->aspectRatio != display->cfg.aspectRatio
        ||
        pParams->_3dOrientationChange
        ||
        newDisplaySettings.colorSpace != oldDisplaySettings.colorSpace
        ||
        newDisplaySettings.colorDepth != oldDisplaySettings.colorDepth
        ||
        (
            /* Dolby Vision change is treated as a format change */
            (display->cfg.dynamicRangeMode != pParams->dynamicRangeMode)
            &&
            (
                IS_DOLBY_VISION(pParams->dynamicRangeMode)
                ||
                IS_DOLBY_VISION(display->cfg.dynamicRangeMode)
            )
        )
    );

    /* self-clear this after use */
    display->hdmi.forceFormatChange = false;

    BDBG_MSG(("hdmi Format Change: %s", hdmiFormatChange ? "Yes" : "No")) ;

    if (hdmiFormatChange && hdmiOutputStatus->connected && hdmiOutputStatus->rxPowered)
    {
        bool contentChangeOnly;
        bool displaySyncOnly ;

        rc = BVDC_Display_GetHdmiSyncOnly(display->displayVdc, &displaySyncOnly) ;
        if (rc) {
            rc = BERR_TRACE(rc);
            /* Keep going, they asked for it... */
        }

        /* If only content (i.e. aspect ratio, displaySync) is changing and not timing,
             set the contentChangeOnly flag for later use in hdmi_output module*/
        contentChangeOnly =
            (((display->cfg.aspectRatio != pParams->aspectRatio) && (display->cfg.format == pParams->format) && (!pParams->_3dOrientationChange))
            || (displaySyncOnly != settings.syncOnly)) ;

        postFormatChangeNeeded = true;
        NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
            NEXUS_HdmiOutput_PreFormatChange_priv(hdmiOutput, contentChangeOnly);
        NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
    }

    rc = NEXUS_VideoOutput_P_ApplyVdcHdmiSettings(pParams, &newDisplaySettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    rc = NEXUS_P_DisplayAspectRatio_ToMagnum(pParams->aspectRatio, pParams->format, &aspectRatioVdc);
    if (rc) {
        aspectRatioVdc = BFMT_AspectRatio_eUnknown; /* don't proceed with uninitialized value. */
    }

    rc = BVDC_Display_SetHdmiSyncOnly(display->displayVdc, settings.syncOnly);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = BVDC_Display_SetMpaaDecimation(display->displayVdc, BVDC_MpaaDeciIf_eHdmi, 1, settings.mpaaDecimationEnabled);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    BVDC_Display_GetDvoConfiguration(display->displayVdc, &dvoSettings);
    dvoSettings.stSpreadSpectrum.bEnable = settings.spreadSpectrum.enable;
    dvoSettings.stSpreadSpectrum.ulFrequency  = settings.spreadSpectrum.frequency ;
    dvoSettings.stSpreadSpectrum.ulDelta = settings.spreadSpectrum.delta ;
    rc = BVDC_Display_SetDvoConfiguration(display->displayVdc, &dvoSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(newDisplaySettings.format, &videoFmt);
    if (rc) {
        videoFmt = BFMT_VideoFmt_eNTSC; /* don't proceed with uninitialized value. */
    }

    /* Check if any hdmi upscale change or hdmi format change */
    if ((display->hdmi.outputFormat != settings.outputFormat || hdmiFormatChange) && hdmiOutputStatus->connected) {
        if(settings.outputFormat != NEXUS_VideoFormat_eUnknown)
        {
            reconnect_outputs = true;
        }
        nexus_display_p_disconnect_outputs(display, &display->cfg, settings.outputFormat);

        rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(settings.outputFormat, &hdmiFmt);
        if (rc) {
            hdmiFmt = BFMT_VideoFmt_eMaxCount;
        }

        rc = BVDC_Display_SetHdmiFormat(display->displayVdc, hdmiFmt, videoFmt);
        if (rc) {rc = BERR_TRACE(rc); goto error;}

        display->hdmi.outputFormat = settings.outputFormat;
    }

    rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    if (reconnect_outputs) {
        nexus_display_p_connect_outputs(display);
        rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }

    /* if upscale, use upscale format to set HDMI instead of videoFmt */
    if(display->hdmi.outputFormat != NEXUS_VideoFormat_eUnknown) {
        rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(display->hdmi.outputFormat, &hdmiFmt);
    }
    else {
        hdmiFmt = videoFmt;
    }

    /* post-ApplyChanges format change */
    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
        (void)NEXUS_HdmiOutput_SetDisplayParams_priv(hdmiOutput, hdmiFmt,
            NEXUS_P_MatrixCoefficients_ToMagnum_isrsafe(newDisplaySettings.colorimetry),
            aspectRatioVdc, hdmiMasterMode) ;

        if (postFormatChangeNeeded)
        {
            NEXUS_HdmiOutput_PostFormatChange_priv(hdmiOutput);
        }
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);

    return NEXUS_SUCCESS;

error:
    BDBG_ASSERT(rc);
    return rc;
}

/* called as a result of NEXUS_HdmiOutput_SetSettings */
void NEXUS_VideoOutput_P_SetHdmiSettings(void *context)
{
    NEXUS_Error rc;
    NEXUS_DisplayHandle display = context;
    NEXUS_VideoOutput_P_FormatChangeParams params;

    BKNI_Memset(&params, 0, sizeof(params));
    params.display = display;
    params.format = display->cfg.format;
    params.aspectRatio = display->cfg.aspectRatio;
    params._3dOrientationChange = false;
    params.dynamicRangeMode = display->cfg.dynamicRangeMode;
    params.priority = display->cfg.priority;
    /* call NEXUS_VideoOutput_P_ApplyHdmiSettings with current display settings */
    rc = NEXUS_VideoOutput_P_ApplyHdmiSettings((void *)display->hdmi.outputNotify, &params);
    if (rc) BERR_TRACE(rc);
}

static BERR_Code
NEXUS_VideoOutput_P_ConnectHdmi(void *output,  NEXUS_DisplayHandle display)
{
    BERR_Code rc;
    NEXUS_HdmiOutputHandle hdmi = output;
    NEXUS_HdmiOutputStatus *hdmiOutputStatus;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    BDBG_ASSERT(NULL != hdmi);
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);

    if(display->hdmi.rateChangeCb_isr)
    {
        /* already connected */
        return BERR_SUCCESS;
    }

    rc = BKNI_CreateEvent(&display->hdmi.notifyDisplayEvent);
    if (rc) {BERR_TRACE(rc); goto err_createevent;}
    display->hdmi.notifyDisplayHandler = NEXUS_RegisterEvent(display->hdmi.notifyDisplayEvent, NEXUS_VideoOutput_P_SetHdmiSettings, display);
    if (!display->hdmi.notifyDisplayHandler) {rc = BERR_TRACE(NEXUS_UNKNOWN); goto err_regevent;}

    /* Enable/connect HDMI Core   */
    NEXUS_Module_Lock(video->modules.hdmiOutput) ;
        rc = NEXUS_HdmiOutput_Connect_priv(hdmi, display->hdmi.notifyDisplayEvent) ;
    NEXUS_Module_Unlock(video->modules.hdmiOutput) ;
    if (rc) return BERR_TRACE(rc);

    hdmiOutputStatus = &g_NEXUS_DisplayModule_State.functionData.NEXUS_VideoOutput_P_SetHdmiFormat.hdmiOutputStatus;
    rc = NEXUS_HdmiOutput_GetStatus(hdmi, hdmiOutputStatus);
    if (rc) {BERR_TRACE(rc); goto err_getstatus;}
    rc = NEXUS_VideoOutput_P_HdmiNum_ToMagnum(hdmiOutputStatus->index, &display->hdmi.vdcIndex);
    if (rc) {BERR_TRACE(rc); goto err_getstatus;}

    /* set this before any isr callbacks that might indirectly trigger its use */
    BDBG_ASSERT(!display->hdmi.outputNotify);
    display->hdmi.outputNotify = hdmi;

    BKNI_EnterCriticalSection();
    display->hdmi.rateChangeCb_isr = NEXUS_VideoOutput_P_HdmiRateChange_isr;
    display->hdmi.vsync_isr = NEXUS_HdmiOutput_P_Vsync_isr;
    display->hdmi.pCbParam = hdmi;
    if ( display->hdmi.rateInfoValid )
    {
        NEXUS_VideoOutput_P_HdmiRateChange_isr(display, hdmi);
    }
    BKNI_LeaveCriticalSection();

    /* On NEXUS_Display_AddOutput, we need to apply settings just like a NEXUS_HdmiOutput_SetSettings. */
    display->hdmi.forceFormatChange = true;
    NEXUS_VideoOutput_P_SetHdmiSettings(display);

    return BERR_SUCCESS;

err_getstatus:
    NEXUS_UnregisterEvent(display->hdmi.notifyDisplayHandler);
err_regevent:
    BKNI_DestroyEvent(display->hdmi.notifyDisplayEvent);
err_createevent:
    return rc;
}

static BERR_Code
NEXUS_VideoOutput_P_DisconnectHdmi(void *output, NEXUS_DisplayHandle display)
{
    BERR_Code rc;
    NEXUS_HdmiOutputHandle hdmi = output;
    NEXUS_HdmiOutputSettings settings;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    BDBG_ASSERT(NULL != hdmi);
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);

    if(g_NEXUS_DisplayModule_State.updateMode != NEXUS_DisplayUpdateMode_eAuto) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);}

    /* HDMI is being removed from the Display; */
    /* reset VDC HDMI settings to values compatible with analog settings */
    NEXUS_VideoOutput_P_SetDefaultHdmiSettings(display) ;

    BKNI_EnterCriticalSection();
        BDBG_ASSERT(NULL != display->hdmi.rateChangeCb_isr);
        display->hdmi.rateChangeCb_isr = NULL;
        display->hdmi.vsync_isr = NULL;
        display->hdmi.pCbParam = NULL;
    BKNI_LeaveCriticalSection();
    display->hdmi.outputNotify = NULL;

    (void) NEXUS_HdmiOutput_GetSettings(hdmi, &settings);
    if (settings.mpaaDecimationEnabled) {
        settings.mpaaDecimationEnabled = false;
        rc = BVDC_Display_SetMpaaDecimation(display->displayVdc, BVDC_MpaaDeciIf_eHdmi, 1, settings.mpaaDecimationEnabled);
        if (rc) {
            rc = BERR_TRACE(rc); /* fall through */
        }
        else {
            /* two ApplyChanges are required: this one and the standard one */
            rc = BVDC_ApplyChanges(video->vdc);
            if (rc) {rc = BERR_TRACE(rc);} /* fall through */
        }
    }

    /* Turn off the transmitter itself */
    BDBG_MSG(("Remove HDMI from nexus display...")) ;
    NEXUS_Module_Lock(video->modules.hdmiOutput);
    rc = NEXUS_HdmiOutput_Disconnect_priv(hdmi);
    NEXUS_Module_Unlock(video->modules.hdmiOutput);
    if (rc) {BERR_TRACE(rc);} /* fall through */

    NEXUS_UnregisterEvent(display->hdmi.notifyDisplayHandler);
    BKNI_DestroyEvent(display->hdmi.notifyDisplayEvent);
    display->hdmi.notifyDisplayEvent = NULL;

    return BERR_SUCCESS;
}

NEXUS_VideoOutput_P_Link *
NEXUS_VideoOutput_P_OpenHdmi(NEXUS_VideoOutputHandle output)
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_VideoOutput_P_Iface iface;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    BDBG_ASSERT(output->type == NEXUS_VideoOutputType_eHdmi);

    BKNI_Memset(&iface, 0, sizeof(iface));
    iface.connect = NEXUS_VideoOutput_P_ConnectHdmi;
    iface.disconnect = NEXUS_VideoOutput_P_DisconnectHdmi;
    iface.formatChange = NEXUS_VideoOutput_P_ApplyHdmiSettings;
    iface.checkSettings = NULL;
    link = NEXUS_VideoOutput_P_CreateLink(output, &iface, false);
    if (link) {
        link->displayOutput = BVDC_DisplayOutput_eDvo;
    }
    return link;
}

/* End of file */
