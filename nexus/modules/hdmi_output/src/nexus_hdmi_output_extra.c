/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#include "nexus_hdmi_output_module.h"

BDBG_MODULE(nexus_hdmi_output_extra);

void NEXUS_HdmiOutput_DisplayRxEdid(NEXUS_HdmiOutputHandle handle)
{
#if BDBG_DEBUG_BUILD
    BDBG_Level level ;
    char EDID_MODULE[] = "BHDM_EDID" ;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    RESOLVE_ALIAS(handle);
    /* save the current debug level */
    BDBG_GetModuleLevel(EDID_MODULE, &level) ;
    BDBG_SetModuleLevel(EDID_MODULE, BDBG_eMsg) ;

    BHDM_EDID_Initialize(handle->hdmHandle) ;

    /* restore the debug level */
    BDBG_SetModuleLevel(EDID_MODULE, level) ;

#else
	BSTD_UNUSED(handle);
#endif
}


void NEXUS_HdmiOutput_DisplayRxInfo(NEXUS_HdmiOutputHandle handle)
{
#if BDBG_DEBUG_BUILD

    uint8_t RxSense ;
    uint8_t Attached  ;
    uint8_t RxMonitorName[BHDM_EDID_DESC_ASCII_STRING_LEN] ;

#if NEXUS_HAS_SECURITY
    BDBG_Level level ;
    BERR_Code rc ;
    uint8_t i, IsRepeater;
    uint16_t BStatus ;
    uint16_t DeviceCount, uiRxKsvIndex ;
    uint8_t AttachedKsv[BHDM_HDCP_KSV_LENGTH] ;
    uint8_t *DownstreamKsvsBuffer ;
    uint16_t DownstreamKsvsBufferSize ;
    char HDCP_MODULE[] = "BHDM_HDCP" ;
#endif

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    RESOLVE_ALIAS(handle);
    BHDM_RxDeviceAttached(handle->hdmHandle, &Attached) ;
    BDBG_WRN(("Device Attached: %d", Attached)) ;
    if (!Attached)
        return  ;

    BHDM_GetReceiverSense(handle->hdmHandle, &RxSense) ;
    BDBG_WRN(("Device RxSense: %d", RxSense)) ;

    if (RxSense != 0xF)
    {
        BDBG_WRN(("Rx not powered (%d) ...", RxSense)) ;
        return  ;
    }

    BKNI_Memset(RxMonitorName, 0, BHDM_EDID_DESC_ASCII_STRING_LEN) ;
    BHDM_EDID_GetMonitorName(handle->hdmHandle, RxMonitorName) ;
    BDBG_WRN(("Monitor '%*s':",
        BHDM_EDID_DESC_ASCII_STRING_LEN, RxMonitorName)) ;


#if NEXUS_HAS_SECURITY
    /* save the current debug level */
    BDBG_GetModuleLevel(HDCP_MODULE, &level) ;
    BDBG_SetModuleLevel(HDCP_MODULE, BDBG_eMsg) ;

    /*get/display information for BStatus */
    BHDM_HDCP_GetRxStatus(handle->hdmHandle, &BStatus) ;

    /* retrieve KSV Information */
    rc = BHDM_HDCP_GetRxKsv(handle->hdmHandle, AttachedKsv) ;
    if (rc != BERR_SUCCESS)
    {
        BDBG_WRN(("Error Retrieving Bksv")) ;
        return  ;
    }

    /* display Attached Device KSV */
    BDBG_WRN(("Attached Rx Bksv = %02x %02x %02x %02x %02x",
            AttachedKsv[4], AttachedKsv[3], AttachedKsv[2],
            AttachedKsv[1], AttachedKsv[0]));

    /* switch back to Wrn to remove extra messages when checking for Repeater */
    BDBG_SetModuleLevel(HDCP_MODULE, BDBG_eWrn) ;
        BHDM_HDCP_CheckForRepeater(handle->hdmHandle, &IsRepeater) ;
    BDBG_SetModuleLevel(HDCP_MODULE, BDBG_eMsg) ;
    if (IsRepeater)
    {
        /* retrieve Repeater Ksvs if available */
        DeviceCount = BStatus & 0x7F ;

        /* allocate space for a copy of downstream devices */
        DownstreamKsvsBufferSize  = DeviceCount * BHDM_HDCP_KSV_LENGTH ;
        DownstreamKsvsBuffer = (uint8_t *) BKNI_Malloc(sizeof(uint8_t) * DownstreamKsvsBufferSize) ;

        rc = BHDM_HDCP_GetRepeaterKsvFifo(handle->hdmHandle,
            DownstreamKsvsBuffer, DownstreamKsvsBufferSize) ;
        if (rc != BERR_SUCCESS)
        {
            BDBG_WRN(("Error w/ Repeater/Downstream Devices")) ;
        }
        else
        {
            BDBG_WRN(("Downstream Devices: %d", DeviceCount)) ;
            for (i = 0 ; i < DeviceCount ; i++)                  /* for each RxKsv */
            {
                uiRxKsvIndex = i * BHDM_HDCP_KSV_LENGTH ;

                /* display each KSV to check debugging */
                BDBG_WRN(("Downstream Device %d: %02x %02x %02x %02x %02x",
                    i, DownstreamKsvsBuffer[uiRxKsvIndex + 4],
                    DownstreamKsvsBuffer[uiRxKsvIndex + 3], DownstreamKsvsBuffer[uiRxKsvIndex + 2],
                    DownstreamKsvsBuffer[uiRxKsvIndex + 1], DownstreamKsvsBuffer[uiRxKsvIndex + 0]));
            }
        }

        BKNI_Free(DownstreamKsvsBuffer) ;
    }

    /* restore the debug level */
    BDBG_SetModuleLevel(HDCP_MODULE, level) ;
#endif

#else
	BSTD_UNUSED(handle);
#endif
    return ;
}


void NEXUS_HdmiOutput_GetVideoFormatFromDetailTiming(NEXUS_HdmiOutputHandle handle, uint8_t detailTimingNumber)
{
#if BDBG_DEBUG_BUILD

	BDBG_Level level ;
	BHDM_EDID_DetailTiming hdmiEDIDDetailTiming ;
	BFMT_VideoFmt videoFormat ;
	char EDID_MODULE[] = "BHDM_EDID" ;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	RESOLVE_ALIAS(handle);
	/* save the current debug level */
	BDBG_GetModuleLevel(EDID_MODULE, &level) ;
	BDBG_SetModuleLevel(EDID_MODULE, BDBG_eMsg) ;

	BHDM_EDID_GetDetailTiming(handle->hdmHandle, detailTimingNumber, &hdmiEDIDDetailTiming, &videoFormat) ;

	/* restore the debug level */
	BDBG_SetModuleLevel(EDID_MODULE, level) ;

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(detailTimingNumber);
#endif

	return;
}


void NEXUS_HdmiOutput_SetAviInfoFrameColorimetry(NEXUS_HdmiOutputHandle handle, uint8_t colorimetry)
{
#if BDBG_DEBUG_BUILD

	BDBG_Level level ;
	char HDM_MODULE[] = "BHDM" ;
	BHDM_Settings settings;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}
	/* save the current debug level */
	BDBG_GetModuleLevel(HDM_MODULE, &level) ;
	BDBG_SetModuleLevel(HDM_MODULE, BDBG_eMsg) ;

	BHDM_GetHdmiSettings(handle->hdmHandle, &settings) ;
	switch (colorimetry)
	{
		default :
		case 1 : settings.eColorimetry = BAVC_MatrixCoefficients_eHdmi_RGB ; break ;
		case 2 : settings.eColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_709 ; break ;
		case 3 : settings.eColorimetry = BAVC_MatrixCoefficients_eSmpte_170M   ; break ;
		case 0 : settings.eColorimetry = BAVC_MatrixCoefficients_eUnknown; break ;
	}
	BDBG_WRN(("Switch Colorimetry to %d", settings.eColorimetry)) ;
	BHDM_EnableDisplay(handle->hdmHandle, &settings);

	/* restore the debug level */
	BDBG_SetModuleLevel(HDM_MODULE, level) ;

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(colorimetry);
#endif

	return;
}


void NEXUS_HdmiOutput_SetAviInfoFrameAspectRatio(NEXUS_HdmiOutputHandle handle, uint8_t aspectRatio)
{
#if BDBG_DEBUG_BUILD

	BDBG_Level level ;
	char HDM_MODULE[] = "BHDM" ;
	BHDM_Settings settings;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}
	/* save the current debug level */
	BDBG_GetModuleLevel(HDM_MODULE, &level) ;
	BDBG_SetModuleLevel(HDM_MODULE, BDBG_eMsg) ;

	BHDM_GetHdmiSettings(handle->hdmHandle, &settings);
	switch (aspectRatio)
	{
		default :
		case 1 : settings.eAspectRatio = BFMT_AspectRatio_e4_3; break ;
		case 0 : settings.eAspectRatio = BFMT_AspectRatio_e16_9; break ;
	}
	BDBG_WRN(("Switch AR to %d", settings.eAspectRatio)) ;
	BHDM_EnableDisplay(handle->hdmHandle, &settings);

	/* restore the debug level */
	BDBG_SetModuleLevel(HDM_MODULE, level) ;

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(aspectRatio);
#endif

	return;
}


void NEXUS_HdmiOutput_EnablePjChecking(NEXUS_HdmiOutputHandle handle, bool enable)
{
#if BDBG_DEBUG_BUILD && NEXUS_HAS_SECURITY

	BDBG_Level level ;
	char HDCP_MODULE[] = "BHDM_HDCP" ;
	BHDM_HDCP_OPTIONS HdcpOptions ;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}
	/* save the current debug level */
	BDBG_GetModuleLevel(HDCP_MODULE, &level) ;
	BDBG_SetModuleLevel(HDCP_MODULE, BDBG_eMsg) ;

	BHDM_HDCP_GetOptions(handle->hdmHandle, &HdcpOptions) ;
	HdcpOptions.PjChecking = enable ;
	BHDM_HDCP_SetOptions(handle->hdmHandle, &HdcpOptions) ;
	BDBG_WRN(("Pj Checking: %d", (int) HdcpOptions.PjChecking)) ;

	/* restore the debug level */
	BDBG_SetModuleLevel(HDCP_MODULE, level) ;

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(enable);
#endif

	return;
}

void NEXUS_HdmiOutput_ForceVideoPixel(NEXUS_HdmiOutputHandle handle, uint8_t pixelValue)
{
#if BDBG_DEBUG_BUILD && NEXUS_HAS_SECURITY

	BDBG_Level level ;
	char HDCP_MODULE[] = "BHDM_HDCP" ;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}
	/* save the current debug level */
	BDBG_GetModuleLevel(HDCP_MODULE, &level) ;
	BDBG_SetModuleLevel(HDCP_MODULE, BDBG_eMsg) ;

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

	/* restore the debug level */
	BDBG_SetModuleLevel(HDCP_MODULE, level) ;

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(pixelValue);
#endif

	return;
}


void NEXUS_HdmiOutput_EnablePacketTransmission(NEXUS_HdmiOutputHandle handle, uint8_t packetChoice)
{
#if BDBG_DEBUG_BUILD

	BDBG_Level level ;
	char HDM_MODULE[] = "BHDM" ;
	uint8_t packetType;
	BHDM_Packet packetId;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}
	/* save the current debug level */
	BDBG_GetModuleLevel(HDM_MODULE, &level) ;
	BDBG_SetModuleLevel(HDM_MODULE, BDBG_eMsg) ;

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

	/* restore the debug level */
	BDBG_SetModuleLevel(HDM_MODULE, level) ;

#else
	BSTD_UNUSED(handle);
	BSTD_UNUSED(packetChoice);
#endif

	return;
}


void NEXUS_HdmiOutput_DisablePacketTransmission(NEXUS_HdmiOutputHandle handle, uint8_t packetChoice)
{
#if BDBG_DEBUG_BUILD

	BDBG_Level level ;
	char HDM_MODULE[] = "BHDM" ;
	uint8_t packetType;
	BHDM_Packet packetId;

	BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
	if (IS_ALIAS(handle)) {BERR_TRACE(NEXUS_NOT_SUPPORTED);return;}
	/* save the current debug level */
	BDBG_GetModuleLevel(HDM_MODULE, &level) ;
	BDBG_SetModuleLevel(HDM_MODULE, BDBG_eMsg) ;

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

	/* restore the debug level */
	BDBG_SetModuleLevel(HDM_MODULE, level) ;

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
