/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include "nexus_hdmi_output_module.h"

BDBG_MODULE(nexus_hdmi_output_extra);

void NEXUS_HdmiOutput_DisplayRxEdid(NEXUS_HdmiOutputHandle handle)
{
    BSTD_UNUSED(handle) ;
    NEXUS_HdmiOutput_PrintRxEdid() ;
}


void NEXUS_HdmiOutput_DisplayRxInfo(NEXUS_HdmiOutputHandle handle)
{
    BSTD_UNUSED(handle) ;
    NEXUS_HdmiOutputModule_Print() ;
}


void NEXUS_HdmiOutput_GetVideoFormatFromDetailTiming(NEXUS_HdmiOutputHandle handle, uint8_t detailTimingNumber)
{
#if BDBG_DEBUG_BUILD

    BHDM_EDID_DetailTiming hdmiEDIDDetailTiming ;
    BFMT_VideoFmt videoFormat ;
    const BFMT_VideoInfo *info;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    RESOLVE_ALIAS(handle);

    BHDM_EDID_GetDetailTiming(handle->hdmHandle, detailTimingNumber, &hdmiEDIDDetailTiming, &videoFormat) ;
    info = BFMT_GetVideoFormatInfoPtr_isrsafe(videoFormat) ;
    BDBG_LOG(("Detailed Timing Format # %d is %s", detailTimingNumber, info->pchFormatStr)) ;

#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(detailTimingNumber);
#endif

	return;
}


void NEXUS_HdmiOutput_SetAviInfoFrameColorimetry(NEXUS_HdmiOutputHandle handle, uint8_t colorimetry)
{
#if BDBG_DEBUG_BUILD

	BHDM_Settings settings;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}

	BHDM_GetHdmiSettings(handle->hdmHandle, &settings) ;
	switch (colorimetry)
	{
		default :
		case 1 : settings.eColorimetry = BAVC_MatrixCoefficients_eHdmi_RGB ; break ;
		case 2 : settings.eColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_709 ; break ;
		case 3 : settings.eColorimetry = BAVC_MatrixCoefficients_eSmpte_170M   ; break ;
		case 0 : settings.eColorimetry = BAVC_MatrixCoefficients_eUnknown; break ;
	}
	BDBG_LOG(("Switching Colorimetry to %d", settings.eColorimetry)) ;
	NEXUS_HdmiOutput_P_EnableDisplay(handle, &settings);

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(colorimetry);
#endif

	return;
}


void NEXUS_HdmiOutput_SetAviInfoFrameAspectRatio(NEXUS_HdmiOutputHandle handle, uint8_t aspectRatio)
{
#if BDBG_DEBUG_BUILD

	BHDM_Settings settings;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}

	BHDM_GetHdmiSettings(handle->hdmHandle, &settings);
	switch (aspectRatio)
	{
		default :
		case 1 : settings.eAspectRatio = BFMT_AspectRatio_e4_3; break ;
		case 0 : settings.eAspectRatio = BFMT_AspectRatio_e16_9; break ;
	}
	BDBG_LOG(("Switching AR to %d", settings.eAspectRatio)) ;
	NEXUS_HdmiOutput_P_EnableDisplay(handle, &settings);

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(aspectRatio);
#endif

	return;
}


void NEXUS_HdmiOutput_EnablePjChecking(NEXUS_HdmiOutputHandle handle, bool enable)
{
#if BDBG_DEBUG_BUILD && NEXUS_HAS_SECURITY

	BHDM_HDCP_OPTIONS HdcpOptions ;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}

	BHDM_HDCP_GetOptions(handle->hdmHandle, &HdcpOptions) ;
	HdcpOptions.PjChecking = enable ;
	BHDM_HDCP_SetOptions(handle->hdmHandle, &HdcpOptions) ;
	BDBG_WRN(("Pj Checking: %d", (int) HdcpOptions.PjChecking)) ;

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(enable);
#endif

	return;
}

void NEXUS_HdmiOutput_ForceVideoPixel(NEXUS_HdmiOutputHandle handle, uint8_t pixelValue)
{
#if BDBG_DEBUG_BUILD && NEXUS_HAS_SECURITY

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}

	switch (pixelValue)
	{
	case 0 :
	case 1 :
		BHDM_HDCP_P_DEBUG_PjForceVideo(handle->hdmHandle, pixelValue) ;
		break ;

	default :
		BHDM_HDCP_P_DEBUG_PjCleanVideo(handle->hdmHandle, pixelValue) ;
		break ;
	}

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(pixelValue);
#endif

	return;
}


void NEXUS_HdmiOutput_EnablePacketTransmission(NEXUS_HdmiOutputHandle handle, uint8_t packetChoice)
{
#if BDBG_DEBUG_BUILD

	uint8_t packetType;
	BHDM_Packet packetId;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}

	switch (packetChoice)
	{
		default :
		case 0 :
			packetId = BHDM_PACKET_eAVI_ID;
			packetType = BAVC_HDMI_PacketType_eAviInfoFrame;
			break;

		case 1 :
			packetId = BHDM_Packet_eVendorSpecific_ID;
			packetType = BAVC_HDMI_PacketType_eVendorSpecificInfoframe;
			break;

		case 2 :
			packetId = BHDM_PACKET_eAudioFrame_ID;
			packetType = BAVC_HDMI_PacketType_eAudioInfoFrame;
			break;

		case 3 :
			packetId = BHDM_PACKET_eSPD_ID;
			packetType = BAVC_HDMI_PacketType_eSpdInfoFrame;
			break;
	}

	BDBG_WRN(("Enable transmission of '%s' packets", BAVC_HDMI_PacketTypeToStr_isrsafe(packetType))) ;
	BHDM_EnablePacketTransmission(handle->hdmHandle, packetId);

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(packetChoice);
#endif

	return;
}


void NEXUS_HdmiOutput_DisablePacketTransmission(NEXUS_HdmiOutputHandle handle, uint8_t packetChoice)
{
#if BDBG_DEBUG_BUILD

	uint8_t packetType;
	BHDM_Packet packetId;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}

	switch (packetChoice)
	{
		default :
		case 0 :
			packetId = BHDM_PACKET_eAVI_ID;
			packetType = BAVC_HDMI_PacketType_eAviInfoFrame;
			break;

		case 1 :
			packetId = BHDM_Packet_eVendorSpecific_ID;
			packetType = BAVC_HDMI_PacketType_eVendorSpecificInfoframe;
			break;

		case 2 :
			packetId = BHDM_PACKET_eAudioFrame_ID;
			packetType = BAVC_HDMI_PacketType_eAudioInfoFrame;
			break;

		case 3 :
			packetId = BHDM_PACKET_eSPD_ID;
			packetType = BAVC_HDMI_PacketType_eSpdInfoFrame;
			break;
	}

	BDBG_WRN(("Disable transmission of '%s' packets", BAVC_HDMI_PacketTypeToStr_isrsafe(packetType))) ;
	BHDM_DisablePacketTransmission(handle->hdmHandle, packetId);

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(packetChoice);
#endif

	return;
}


NEXUS_Error NEXUS_HdmiOutput_ResetScrambling(NEXUS_HdmiOutputHandle hdmiOutput)
{
    BERR_Code rc = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(hdmiOutput, NEXUS_HdmiOutput);
    RESOLVE_ALIAS(hdmiOutput);

#if BHDM_HAS_HDMI_20_SUPPORT
    BDBG_WRN(("Reset the HDMI scrambling configuration")) ;
    rc = BHDM_SCDC_ConfigureScrambling(hdmiOutput->hdmHandle) ;
#endif

    if (rc) BERR_TRACE(rc) ;
    return rc ;
}

/**
Summary:
Get current lightly-used extra settings
**/
void NEXUS_HdmiOutput_GetExtraSettings(
    NEXUS_HdmiOutputHandle output,
    NEXUS_HdmiOutputExtraSettings *pSettings    /* [out] Settings */
    )
{
    BDBG_ASSERT(output);
    if (pSettings)
    {
        *pSettings = output->extraSettings;
    }
}

/**
Summary:
Apply new lightly-used extra settings
**/
NEXUS_Error NEXUS_HdmiOutput_SetExtraSettings(
    NEXUS_HdmiOutputHandle output,
    const NEXUS_HdmiOutputExtraSettings *pSettings
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    bool drmChanged = false;
    bool dbvChanged = false;

    BDBG_ASSERT(output);
    BDBG_ASSERT(pSettings);

    if
    (
        (output->extraSettings.overrideDynamicRangeMasteringInfoFrame != pSettings->overrideDynamicRangeMasteringInfoFrame)
        ||
        (
            pSettings->overrideDynamicRangeMasteringInfoFrame
            &&
            BKNI_Memcmp(&output->extraSettings.dynamicRangeMasteringInfoFrame, &pSettings->dynamicRangeMasteringInfoFrame, sizeof(pSettings->dynamicRangeMasteringInfoFrame))
        )
    )
    {
        if (output->extraSettings.overrideDynamicRangeMasteringInfoFrame != pSettings->overrideDynamicRangeMasteringInfoFrame)
        {
            BDBG_WRN(("%s drm info frame override", pSettings->overrideDynamicRangeMasteringInfoFrame ? "Engaging" : "Releasing"));
        }
        drmChanged = true;
    }

    if (output->extraSettings.dolbyVision.outputMode != pSettings->dolbyVision.outputMode ||
        output->extraSettings.dolbyVision.blendInIpt != pSettings->dolbyVision.blendInIpt ||
        output->extraSettings.dolbyVision.priorityMode != pSettings->dolbyVision.priorityMode)
    {
        dbvChanged = true;
    }

    BKNI_Memcpy(&output->extraSettings, pSettings, sizeof(*pSettings));

    if (drmChanged)
    {
        rc = NEXUS_HdmiOutput_P_ApplyDrmInfoFrameSource(output);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }

#if NEXUS_DBV_SUPPORT
    if (dbvChanged)
    {
        rc = NEXUS_HdmiOutput_P_SetDbvMode(output);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }
    /* need to update all other info, like eotf if not dolby */
    NEXUS_TaskCallback_Fire(output->notifyDisplay);
#else
    BSTD_UNUSED(dbvChanged);
#endif

error:
    return rc;
}

void NEXUS_HdmiOutput_GetExtraStatus(
    NEXUS_HdmiOutputHandle output,
    NEXUS_HdmiOutputExtraStatus *pStatus
    )
{
    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);
    RESOLVE_ALIAS(output);
    if (pStatus)
    {
        BKNI_Memset(pStatus, 0, sizeof(*pStatus));
#if NEXUS_DBV_SUPPORT
        pStatus->dolbyVision.supported = output->dbv.supported;
        pStatus->dolbyVision.enabled = output->dbv.enabled;
#endif
        pStatus->phyChangeRequestCounter = output->phyChangeRequestCounter ;
    }
}
