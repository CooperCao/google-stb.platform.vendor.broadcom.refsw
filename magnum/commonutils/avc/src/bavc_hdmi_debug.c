/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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


#include "bavc.h"
#include "bavc_hdmi.h"
#include "bkni.h"

BDBG_MODULE(BAVC_HDMI_DEBUG) ;

#define BAVC_HDMI_DISPLAY_PREFIX 6

/* macro to setup source port  and debug info */
#define BAVC_HDMI_GET_SOURCE_INFO(stPort, pchSourceString, pstFromPort) \
	if (pstFromPort != NULL) \
	{ \
		BKNI_Memcpy(&stPort, pstFromPort, sizeof(BAVC_HDMI_Port)) ; \
	} \
	else \
	{ \
		stPort.eCoreId = BAVC_HDMI_CoreId_e0 ; \
		stPort.eDevice = BAVC_HDMI_Device_eTx ; \
	} \
\
	BKNI_Snprintf(pchSourceString, BAVC_HDMI_DISPLAY_PREFIX, "%s%d:", \
		(stPort.eDevice == BAVC_HDMI_Device_eTx) ? "Tx" : "Rx", stPort.eCoreId) ;



/* macro to check Rx Packet Status  */
#define BAVC_HDMI_RX_PACKET_STATUS(stPort, packet, pchSourceString, packetName) \
	if (stPort.eDevice == BAVC_HDMI_Device_eRx) \
	{ \
		switch (packet->ePacketStatus) \
		{ \
		case BAVC_HDMI_PacketStatus_eNotReceived : \
			BDBG_LOG(("%s No %s packets have been received", pchSourceString, packetName)) ; \
			goto done ; \
			break ; \
 \
		case BAVC_HDMI_PacketStatus_eStopped: \
			BDBG_LOG(("%s Packets have stopped; Last Valid Packet follows:", pchSourceString)) ; \
			break ; \
 \
		case BAVC_HDMI_PacketStatus_eUpdated: \
			break ; \
 \
		default : \
			BDBG_ERR(("%s Unknown Rx Packet Status: %d", \
				pchSourceString, packet->ePacketStatus)) ; \
			BERR_TRACE(BERR_INVALID_PARAMETER) ; \
			goto done ; \
		} \
	}


/******************************************************************************
Summary:
Set/Enable the Auxillary Video Information Info frame to be sent to the HDMI Rx
*******************************************************************************/
void BAVC_HDMI_DisplayAVIInfoFramePacket(
	BAVC_HDMI_Port *pstFromPort,
	BAVC_HDMI_AviInfoFrame *pstAviInfoFrame
)
{
#if BDBG_DEBUG_BUILD
/*
		Y0, Y1
			RGB or YCBCR indicator. See EIA/CEA-861B table 8 for details.
		A0
			Active Information Present. Indicates whether field R0..R3 is valid.
			See EIA/CEA-861B table 8 for details.
		B0, B1
			Bar Info data valid. See EIA/CEA-861B table 8 for details.
		S0, S1
			Scan Information (i.e. overscan, underscan). See EIA/CEA-861B table 8
		C0, C1
			Colorimetry (ITU BT.601, BT.709 etc.). See EIA/CEA-861B table 9
		M0, M1
			Picture Aspect Ratio (4:3, 16:9). See EIA/CEA-861B table 9
		R0.R3
			Active Format Aspect Ratio. See EIA/CEA-861B table 10 and Annex H
		VIC0..VIC6
			Video Format Identification Code. See EIA/CEA-861B table 13
		PR0..PR3
			Pixel Repetition factor. See EIA/CEA-861B table 14
		SC1, SC0
			Non-uniform Picture Scaling. See EIA/CEA-861B table 11
		EC2..EC0
			Extended Colorimetry. See EIA/CEA-861D tables 7 & 11
*/

	char pchSourceString[BAVC_HDMI_DISPLAY_PREFIX] ;
	BAVC_HDMI_Port stPort ;

	BAVC_HDMI_GET_SOURCE_INFO(stPort, pchSourceString, pstFromPort) ;
#if BAVC_HDMI_RECEIVER
	BAVC_HDMI_RX_PACKET_STATUS(stPort, pstAviInfoFrame, pchSourceString, "AVI") ;
#endif

	BDBG_LOG(("%s AVI InfoFrame:", pchSourceString)) ;

	if ((stPort.eDevice == BAVC_HDMI_Device_eTx)
	&& (pstAviInfoFrame->bOverrideDefaults))
	{
		/* display bOverrideDefaults setting if enabled */
		BDBG_LOG(("%s Override Default: %s", pchSourceString,
			pstAviInfoFrame->bOverrideDefaults ? "Yes" : "No"));
	}

	BDBG_LOG(("%s (Y1Y0)     ColorSpace (%d): %s",
		pchSourceString, pstAviInfoFrame->ePixelEncoding,
		BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(pstAviInfoFrame->ePixelEncoding)));
	BDBG_LOG(("%s (A0)       Active Info (%d): %s ", pchSourceString,
		pstAviInfoFrame->eActiveInfo,
		BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(pstAviInfoFrame->eActiveInfo)));
	BDBG_LOG(("%s (B1B0)     Bar Info (%d): %s ", pchSourceString,
		pstAviInfoFrame->eBarInfo, BAVC_HDMI_AviInfoFrame_BarInfoToStr(pstAviInfoFrame->eBarInfo)));
	BDBG_LOG(("%s (S1S0)     Scan Info (%d): %s", pchSourceString,
		pstAviInfoFrame->eScanInfo, BAVC_HDMI_AviInfoFrame_ScanInfoToStr(pstAviInfoFrame->eScanInfo))) ;

	BDBG_LOG(("%s (C1C0)     Colorimetry (%d): %s", pchSourceString,
		pstAviInfoFrame->eColorimetry,
		BAVC_HDMI_AviInfoFrame_ColorimetryToStr(pstAviInfoFrame->eColorimetry))) ;

	if (pstAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)
	{
		BDBG_LOG(("%s (EC2..EC0) Extended Colorimetry (%d): %s",
			pchSourceString, pstAviInfoFrame->eExtendedColorimetry,
			BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(pstAviInfoFrame->eExtendedColorimetry))) ;
	}
	else
	{
		BDBG_LOG(("%s (EC2..EC0) Extended Colorimetry Info Invalid", pchSourceString)) ;
	}


	BDBG_LOG(("%s (M1M0)     Picture AR (%d): %s", pchSourceString,
		pstAviInfoFrame->ePictureAspectRatio,
		BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(pstAviInfoFrame->ePictureAspectRatio))) ;

	if ((pstAviInfoFrame->eActiveFormatAspectRatio >= 8)
	&&	(pstAviInfoFrame->eActiveFormatAspectRatio <= 11))
		BDBG_LOG(("%s (R3..R0)   Active Format AR (%d): %s", pchSourceString,
			pstAviInfoFrame->eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(pstAviInfoFrame->eActiveFormatAspectRatio - 8))) ;
	else if  ((pstAviInfoFrame->eActiveFormatAspectRatio > 12)
	&&	(pstAviInfoFrame->eActiveFormatAspectRatio <= 15))
		BDBG_LOG(("%s (R3..R0)   Active Format AR (%d): %s", pchSourceString,
			pstAviInfoFrame->eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(pstAviInfoFrame->eActiveFormatAspectRatio - 9))) ;
	else
		BDBG_LOG(("%s Active Format AR (%d): Other", pchSourceString,
			pstAviInfoFrame->eActiveFormatAspectRatio)) ;

	BDBG_LOG(("%s (SC1SC0)   Picture Scaling (%d): %s ", pchSourceString,
		pstAviInfoFrame->eScaling, BAVC_HDMI_AviInfoFrame_ScalingToStr(pstAviInfoFrame->eScaling))) ;

	BDBG_LOG(("%s (VIC6..0)  Video ID Code = %d", pchSourceString, pstAviInfoFrame->VideoIdCode )) ;
	BDBG_LOG(("%s (PR3..PR0) Pixel Repeat: %d", pchSourceString, pstAviInfoFrame->PixelRepeat)) ;

	BDBG_LOG(("%s (ITC)      IT Content (%d): %s", pchSourceString, pstAviInfoFrame->eITContent,
		BAVC_HDMI_AviInfoFrame_ITContentToStr(pstAviInfoFrame->eITContent)));
	BDBG_LOG(("%s (CN1CN0)   IT Content Type (%d): %s", pchSourceString, pstAviInfoFrame->eContentType,
		BAVC_HDMI_AviInfoFrame_ContentTypeToStr(pstAviInfoFrame->eContentType)));
	BDBG_LOG(("%s (Q1Q0)     RGB Quantization Range (%d): %s", pchSourceString, pstAviInfoFrame->eRGBQuantizationRange,
		BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(pstAviInfoFrame->eRGBQuantizationRange)));
	BDBG_LOG(("%s (YQ1YQ0)   Ycc Quantization Range (%d): %s", pchSourceString, pstAviInfoFrame->eYccQuantizationRange,
		BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(pstAviInfoFrame->eYccQuantizationRange)));

	BDBG_LOG(("%s Top Bar End Line Number:     %d", pchSourceString, pstAviInfoFrame->TopBarEndLineNumber)) ;
	BDBG_LOG(("%s Bottom Bar Stop Line Number: %d", pchSourceString, pstAviInfoFrame->BottomBarStartLineNumber)) ;

	BDBG_LOG(("%s Left Bar End Pixel Number:   %d", pchSourceString, pstAviInfoFrame->LeftBarEndPixelNumber )) ;
	BDBG_LOG(("%s Right Bar End Pixel Number:  %d", pchSourceString, pstAviInfoFrame->RightBarEndPixelNumber )) ;
	BDBG_LOG(("%s END AVI InfoFrame", pchSourceString)) ;

#if BAVC_HDMI_RECEIVER
done: ;
#endif

#else
	BSTD_UNUSED(pstFromPort) ;
	BSTD_UNUSED(pstAviInfoFrame) ;
#endif
}

/******************************************************************************
Summary:
Set/Enable the Audio Info Frame packet to be sent to the HDMI Rx
*******************************************************************************/
void BAVC_HDMI_DisplayAudioInfoFramePacket(
	BAVC_HDMI_Port *pstFromPort,
	BAVC_HDMI_AudioInfoFrame *pstAudioInfoFrame
)
{
#if !BDBG_NO_LOG

	char pchSourceString[BAVC_HDMI_DISPLAY_PREFIX] ;
	BAVC_HDMI_Port stPort ;

	BAVC_HDMI_GET_SOURCE_INFO(stPort, pchSourceString, pstFromPort) ;
#if BAVC_HDMI_RECEIVER
	BAVC_HDMI_RX_PACKET_STATUS(stPort, pstAudioInfoFrame, pchSourceString, "Audio IF") ;
#endif

	BDBG_LOG(("%s Audio InfoFrame:", pchSourceString)) ;

	BDBG_LOG(("%s Audio Coding Type     %s", pchSourceString,
		BAVC_HDMI_AudioInfoFrame_CodingTypeToStr(pstAudioInfoFrame->CodingType))) ;

	BDBG_LOG(("%s Audio Channel Count   %s", pchSourceString,
		BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(pstAudioInfoFrame->ChannelCount))) ;

	BDBG_LOG(("%s Sampling Frequency    %s", pchSourceString,
		BAVC_HDMI_AudioInfoFrame_SampleFrequencyToStr(pstAudioInfoFrame->SampleFrequency))) ;

	BDBG_LOG(("%s Sample Size           %s", pchSourceString,
		BAVC_HDMI_AudioInfoFrame_SampleSizeToStr(pstAudioInfoFrame->SampleSize))) ;

	BDBG_LOG(("%s Speaker Allocation    %02x", pchSourceString,
		pstAudioInfoFrame->SpeakerAllocation)) ;

	BDBG_LOG(("%s Level Shift           %s", pchSourceString,
		BAVC_HDMI_AudioInfoFrame_LevelShiftToStr(pstAudioInfoFrame->LevelShift))) ;

	BDBG_LOG(("%s Down-mix Inhibit Flag %s", pchSourceString,
		BAVC_HDMI_AudioInfoFrame_DownMixInhibitToStr(pstAudioInfoFrame->DownMixInhibit))) ;

	BDBG_LOG(("%s END Audio InfoFrame", pchSourceString)) ;

#if BAVC_HDMI_RECEIVER
	done: ;
#endif

#else
	BSTD_UNUSED(pstFromPort) ;
	BSTD_UNUSED(pstAudioInfoFrame) ;
#endif
}


/******************************************************************************
Summary:
	Display  Vendor Specific Info Frame
*******************************************************************************/
void BAVC_HDMI_DisplayVendorSpecificInfoFrame(
	BAVC_HDMI_Port *pstFromPort,
	const BAVC_HDMI_VendorSpecificInfoFrame *pstVSI)
{
#if BDBG_DEBUG_BUILD

	char pchSourceString[BAVC_HDMI_DISPLAY_PREFIX] ;
	BAVC_HDMI_Port stPort ;

	BAVC_HDMI_GET_SOURCE_INFO(stPort, pchSourceString, pstFromPort) ;
#if BAVC_HDMI_RECEIVER
	BAVC_HDMI_RX_PACKET_STATUS(stPort, pstVSI, pchSourceString, "VSI") ;
#endif

	BDBG_LOG(("%s Vendor Specific InfoFrame:", pchSourceString)) ;
	BDBG_LOG(("%s IEEE Reg ID 0x%02x%02x%02x", pchSourceString,
		pstVSI->uIEEE_RegId[2], pstVSI->uIEEE_RegId[1], pstVSI->uIEEE_RegId[0])) ;


	BDBG_LOG(("%s HDMI VideoFormat: %s   (PB4: %#x)", pchSourceString,
		BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr(pstVSI->eHdmiVideoFormat),
		pstVSI->eHdmiVideoFormat)) ;

	if (pstVSI->eHdmiVideoFormat == BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution)
	{
		BDBG_LOG(("%s HDMI_VIC: 0x%d  %s (PB5: %#x)", pchSourceString,
			pstVSI->eHdmiVic,
			BAVC_HDMI_VsInfoFrame_HdmiVicToStr(pstVSI->eHdmiVic),
			pstVSI->eHdmiVic));
	}
	else if (pstVSI->eHdmiVideoFormat == BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat)
	{
		BDBG_LOG(("%s HDMI 2D/3D Structure: %s (PB5: %#x)", pchSourceString,
			BAVC_HDMI_VsInfoFrame_3DStructureToStr(pstVSI->e3DStructure),
			pstVSI->e3DStructure));

		if (pstVSI->e3DStructure == BAVC_HDMI_VSInfoFrame_3DStructure_eSidexSideHalf)
		{
			BDBG_LOG(("%s HDMI 3D_Ext_Data: %s (PB6: %#x)", pchSourceString,
				BAVC_HDMI_VsInfoFrame_3DExtDataToStr(pstVSI->e3DExtData),
				pstVSI->e3DExtData));
		}
	}
	BDBG_LOG(("%s END Vendor Specific InfoFrame", pchSourceString)) ;

#if BAVC_HDMI_RECEIVER
done: ;
#endif

#else
	BSTD_UNUSED(pstFromPort) ;
	BSTD_UNUSED(pstVSI) ;
#endif
}


/******************************************************************************
Summary:
Display the DRM Info Frame packet to be sent to the HDMI Rx
*******************************************************************************/
void BAVC_HDMI_DisplayDRMInfoFramePacket(
	BAVC_HDMI_Port *pstFromPort,
	BAVC_HDMI_DRMInfoFrame *pstDRMInfoFrame
)
{
#if BDBG_DEBUG_BUILD

	char pchSourceString[BAVC_HDMI_DISPLAY_PREFIX] ;
	BAVC_HDMI_Port stPort ;

	BAVC_HDMI_GET_SOURCE_INFO(stPort, pchSourceString, pstFromPort) ;
#if BAVC_HDMI_RECEIVER
	BAVC_HDMI_RX_PACKET_STATUS(stPort, pstDRMInfoFrame, pchSourceString, "DRM") ;
#endif

	BDBG_LOG(("%s DRM InfoFrame:", pchSourceString)) ;

	BDBG_LOG(("%s DRM EOTF:   %s", pchSourceString,
		BAVC_HDMI_DRMInfoFrame_EOTFToStr(pstDRMInfoFrame->eEOTF))) ;

	BDBG_LOG(("%s DRM Descriptor ID   %s", pchSourceString,
		BAVC_HDMI_DRMInfoFrame_DescriptorIdToStr(pstDRMInfoFrame->eDescriptorId))) ;

	/* TODO Add parse/display of fields within the packet */
	BDBG_LOG(("%s END DRM InfoFrame", pchSourceString)) ;
#if BAVC_HDMI_RECEIVER
	done: ;
#endif

#else
	BSTD_UNUSED(pstFromPort) ;
	BSTD_UNUSED(pstDRMInfoFrame) ;
#endif
}


/******************************************************************************
Summary:
Display the Audio Clock Regeneration values
*******************************************************************************/
void BAVC_HDMI_DisplayACRData(
	BAVC_HDMI_Port *pstFromPort,
	BAVC_HDMI_AudioClockRegenerationPacket *pstACR)
{
#if BDBG_DEBUG_BUILD

	char pchSourceString[BAVC_HDMI_DISPLAY_PREFIX] ;
	BAVC_HDMI_Port stPort ;

	BAVC_HDMI_GET_SOURCE_INFO(stPort, pchSourceString, pstFromPort) ;
#if BAVC_HDMI_RECEIVER
	BAVC_HDMI_RX_PACKET_STATUS(stPort, pstACR, pchSourceString, "ACR") ;
#endif

	BDBG_LOG(("%s ACR Data Packet:", pchSourceString)) ;
	BDBG_LOG(("%s ACR:    N= %8d        CTS= %8d", pchSourceString,
		pstACR->N, pstACR->CTS)) ;
	BDBG_LOG(("%s END ACR Data Packet", pchSourceString)) ;

#if BAVC_HDMI_RECEIVER
done: ;
#endif
#else
	BSTD_UNUSED(pstFromPort) ;
	BSTD_UNUSED(pstACR) ;
#endif

}
