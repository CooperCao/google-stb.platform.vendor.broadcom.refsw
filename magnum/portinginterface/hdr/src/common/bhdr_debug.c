/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 * $brcm_Log: $
 *
  ***************************************************************************/
#if BHDR_SPECIAL_DEBUG
#include "ctype.h"
#endif

#include "bavc_hdmi.h"

#include "bhdr.h"
#include "bhdr_priv.h"

#include "bhdr_debug.h"


#if BHDR_CONFIG_DEBUG_TIMER_S
#include "bchp_hd_dvi_0.h"
#endif

BDBG_MODULE(BHDR_DEBUG) ;


#if BHDR_CONFIG_DEBUG_HDCP_VALUES
void BHDR_P_DebugHdcpValues_isr(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset   ;

	uint8_t AvMute, PacketProcessorAvMute ;
	uint32_t An0, An1, Aksv0, Aksv1;

	BDBG_ENTER(BHDR_P_DebugHdcpValues_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_MISC_STATUS + ulOffset) ;
	AvMute = (uint8_t) BCHP_GET_FIELD_DATA(Register,
		HDMI_RX_0_MISC_STATUS, AV_MUTE) ;

	PacketProcessorAvMute = (uint8_t) BCHP_GET_FIELD_DATA(Register,
		HDMI_RX_0_MISC_STATUS, PKT_PROCESSOR_AV_MUTE) ;

	BDBG_WRN(("CH%d HDCP Auth Request (Load keys)... AvMute: %d ; Packet AvMute: %d",
		hHDR->eCoreId, AvMute, PacketProcessorAvMute)) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_AN_1 + ulOffset) ;
	An1 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_MON_AN_1, HDCP_RX_MON_AN) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_AN_0 + ulOffset) ;
	An0 = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_MON_AN_0, HDCP_RX_MON_AN) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_AKSV_1 + ulOffset) ;
	Aksv1 = 	BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_MON_AKSV_1, HDCP_RX_MON_AKSV) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_HDCP_MON_AKSV_0 + ulOffset) ;
	Aksv0 = 	BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_HDCP_MON_AKSV_0, HDCP_RX_MON_AKSV) ;

	BDBG_WRN((" CH%d Recevied Tx An:   %x%x", hHDR->eCoreId, An1, An0)) ;
	BDBG_WRN((" CH%d Recevied Tx Aksv: %x%x",   hHDR->eCoreId, Aksv1, Aksv0)) ;

	BDBG_LEAVE(BHDR_P_DebugHdcpValues_isr) ;
}
#endif


void BHDR_P_DEBUG_AviInfoFrame(
	BAVC_HDMI_AviInfoFrame *OldAvi,
	BAVC_HDMI_AviInfoFrame *NewAvi)
{
	BDBG_LOG(("-------------------- PARSED AVI INFOFRAME -------------------")) ;
	if (OldAvi->ePixelEncoding != NewAvi->ePixelEncoding)
	{
		BDBG_LOG(("(Y1Y0)     PixelEncoding/Colorspace  : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(NewAvi->ePixelEncoding),
			NewAvi->ePixelEncoding,
			BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(OldAvi->ePixelEncoding),
			OldAvi->ePixelEncoding)) ;
	}
	else
	{
		BDBG_LOG(("(Y1Y0)     PixelEncoding/Colorspace  : %s (%d)",
			BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(NewAvi->ePixelEncoding),
			NewAvi->ePixelEncoding)) ;
	}


	if (OldAvi->eActiveInfo != NewAvi->eActiveInfo)
	{
		BDBG_LOG(("(A0)       Active Format Info Valid  : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(NewAvi->eActiveInfo),
			NewAvi->eActiveInfo,
			BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(OldAvi->eActiveInfo),
			OldAvi->eActiveInfo)) ;
	}
	else
	{
		BDBG_LOG(("(A0)       Active Format Info Valid  : %s (%d)",
			BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(NewAvi->eActiveInfo),
			NewAvi->eActiveInfo)) ;
	}


	if (OldAvi->eBarInfo != NewAvi->eBarInfo)
	{
		BDBG_LOG(("(B1B0)     Bar Info Valid            : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_BarInfoToStr(NewAvi->eBarInfo),
			NewAvi->eBarInfo,
			BAVC_HDMI_AviInfoFrame_BarInfoToStr(OldAvi->eBarInfo),
			OldAvi->eBarInfo)) ;
	}
	else
	{
		BDBG_LOG(("(B1B0)     Bar Info Valid            : %s (%d)",
			BAVC_HDMI_AviInfoFrame_BarInfoToStr(NewAvi->eBarInfo),
			NewAvi->eBarInfo)) ;
	}


	if (OldAvi->eScanInfo != NewAvi->eScanInfo)
	{
		BDBG_LOG(("(S1S0)     Scan Information          : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_ScanInfoToStr(NewAvi->eScanInfo),
			NewAvi->eScanInfo,
			BAVC_HDMI_AviInfoFrame_ScanInfoToStr(OldAvi->eScanInfo),
			OldAvi->eScanInfo)) ;
	}
	else
	{
		BDBG_LOG(("(S1S0)     Scan Information          : %s (%d)",
			BAVC_HDMI_AviInfoFrame_ScanInfoToStr(NewAvi->eScanInfo),
			NewAvi->eScanInfo)) ;
	}


	if (OldAvi->eColorimetry != NewAvi->eColorimetry)
	{
		BDBG_LOG(("(C1C0)     Colorimetry               : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_ColorimetryToStr(NewAvi->eColorimetry),
			NewAvi->eColorimetry,
			BAVC_HDMI_AviInfoFrame_ColorimetryToStr(OldAvi->eColorimetry),
			OldAvi->eColorimetry)) ;
	}
	else
	{
		BDBG_LOG(("(C1C0)     Colorimetry               : %s (%d)",
			BAVC_HDMI_AviInfoFrame_ColorimetryToStr(NewAvi->eColorimetry),
			NewAvi->eColorimetry	)) ;
	}


	if (OldAvi->ePictureAspectRatio != NewAvi->ePictureAspectRatio)
	{
		BDBG_LOG(("(M1M0)     Picture AR                : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(NewAvi->ePictureAspectRatio),
			NewAvi->ePictureAspectRatio,
			BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(OldAvi->ePictureAspectRatio),
			OldAvi->ePictureAspectRatio)) ;
	}
	else
	{
		BDBG_LOG(("(M1M0)     Picture AR                : %s (%d)",
			BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(NewAvi->ePictureAspectRatio),
			NewAvi->ePictureAspectRatio)) ;
	}


	if (OldAvi->eActiveFormatAspectRatio != NewAvi->eActiveFormatAspectRatio)
	{
		BDBG_LOG(("(R3..R0)   Active Format AR          : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(NewAvi->eActiveFormatAspectRatio - 8 ),
			NewAvi->eActiveFormatAspectRatio,
			OldAvi->eActiveInfo ?
				BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(OldAvi->eActiveFormatAspectRatio - 8) :
				"No Data",
			OldAvi->eActiveFormatAspectRatio)) ;
	}
	else
	{
		BDBG_LOG(("(R3..R0)   Active Format AR          : %s (%d)",
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(NewAvi->eActiveFormatAspectRatio-8),
			NewAvi->eActiveFormatAspectRatio)) ;
	}


	if (OldAvi->eITContent != NewAvi->eITContent)
	{
		BDBG_LOG(("(ITC)      IT Content                : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_ITContentToStr(NewAvi->eITContent),
			NewAvi->eITContent,
			BAVC_HDMI_AviInfoFrame_ITContentToStr(OldAvi->eITContent),
			OldAvi->eITContent)) ;
	}
	else
	{
		BDBG_LOG(("(ITC)      IT Content                : %s (%d)",
			BAVC_HDMI_AviInfoFrame_ITContentToStr(NewAvi->eITContent),
			NewAvi->eITContent)) ;
	}



	if (OldAvi->eExtendedColorimetry != NewAvi->eExtendedColorimetry)
	{
		BDBG_LOG(("(EC2..EC0) Extended Colorimetry      : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(NewAvi->eExtendedColorimetry),
			NewAvi->eExtendedColorimetry,
			BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(OldAvi->eExtendedColorimetry),
			OldAvi->eExtendedColorimetry)) ;
	}
	else
	{
		BDBG_LOG(("(EC2..EC0) Extended Colorimetry      : %s (%d)",
			BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(NewAvi->eExtendedColorimetry),
			NewAvi->eExtendedColorimetry)) ;
	}


	if (OldAvi->eRGBQuantizationRange != NewAvi->eRGBQuantizationRange)
	{
		BDBG_LOG(("(Q1Q0)     RGB Quantization Range    : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(NewAvi->eRGBQuantizationRange),
			NewAvi->eRGBQuantizationRange,
			BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(OldAvi->eRGBQuantizationRange),
			OldAvi->eRGBQuantizationRange)) ;
	}
	else
	{
		BDBG_LOG(("(Q1Q0)     RGB Quantization Range    : %s (%d)",
			BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(NewAvi->eRGBQuantizationRange),
			NewAvi->eRGBQuantizationRange)) ;
	}


	if (OldAvi->eScaling != NewAvi->eScaling)
	{
		BDBG_LOG(("(SC1SC0)   Picture Scaling           : %s (%d) was %s (%d)",
			BAVC_HDMI_AviInfoFrame_ScalingToStr(NewAvi->eScaling),
			NewAvi->eScaling,
			BAVC_HDMI_AviInfoFrame_ScalingToStr(OldAvi->eScaling),
			OldAvi->eScaling)) ;
	}
	else
	{
		BDBG_LOG(("(SC1SC0)   Picture Scaling           : %s (%d)",
			BAVC_HDMI_AviInfoFrame_ScalingToStr(NewAvi->eScaling),
			NewAvi->eScaling)) ;
	}

/****************************/

	if (OldAvi->VideoIdCode != NewAvi->VideoIdCode)
	{
		BDBG_LOG(("(VIC6..0)  Video Code                : %s (%d) was %s (%d) ",
			BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr(NewAvi->VideoIdCode),
			NewAvi->VideoIdCode,
			BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr(OldAvi->VideoIdCode),
			OldAvi->VideoIdCode)) ;
	}
	else
	{
		BDBG_LOG(("(VIC6..0)  Video Code                : %s (%d) ",
			BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr(NewAvi->VideoIdCode),
			NewAvi->VideoIdCode)) ;
	}


	if (OldAvi->PixelRepeat != NewAvi->PixelRepeat)
	{
		BDBG_LOG(("(PR3..PR0) Pixel Repeat              : %d was %d ",
			NewAvi->PixelRepeat, OldAvi->PixelRepeat)) ;
	}
	else
	{
		BDBG_LOG(("(PR3..PR0) Pixel Repeat              : %d ",
		NewAvi->PixelRepeat)) ;
	}

	if (OldAvi->eContentType != NewAvi->eContentType)
	{
		BDBG_LOG(("(CN1CN0)  IT Content Type            : %s (%d) was %s (%d) ",
			BAVC_HDMI_AviInfoFrame_ContentTypeToStr(NewAvi->eContentType),
			NewAvi->eContentType,
			BAVC_HDMI_AviInfoFrame_ContentTypeToStr(OldAvi->eContentType),
			OldAvi->eContentType)) ;
	}
	else
	{
		BDBG_LOG(("(CN1CN0)  IT Content Type            : %s (%d) ",
			BAVC_HDMI_AviInfoFrame_ContentTypeToStr(NewAvi->eContentType),
			NewAvi->eContentType)) ;
	}


	if (OldAvi->eYccQuantizationRange != NewAvi->eYccQuantizationRange)
	{
		BDBG_LOG(("(YQ1YQ0)  YCC Quantization Range     : %s (%d) was %s (%d) ",
			BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(NewAvi->eYccQuantizationRange),
			NewAvi->eYccQuantizationRange,
			BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(OldAvi->eYccQuantizationRange),
			OldAvi->eYccQuantizationRange)) ;
	}
	else
	{
		BDBG_LOG(("(YQ1YQ0)  YCC Quantization Range     : %s (%d) ",
			BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(NewAvi->eYccQuantizationRange),
			NewAvi->eYccQuantizationRange)) ;
	}


	if (OldAvi->TopBarEndLineNumber != NewAvi->TopBarEndLineNumber)
	{
		BDBG_LOG(("Top Bar End Line #                   : %d was %d ",
			NewAvi->TopBarEndLineNumber, OldAvi->TopBarEndLineNumber )) ;
	}
	else
	{
		BDBG_LOG(("Top Bar End Line #                   : %d ",
		NewAvi->TopBarEndLineNumber)) ;
	}


	if (OldAvi->BottomBarStartLineNumber != NewAvi->BottomBarStartLineNumber)
	{
		BDBG_LOG(("Bottom Bar Start Line #              : %d was %d ",
			NewAvi->BottomBarStartLineNumber, OldAvi->BottomBarStartLineNumber )) ;
	}
	else
	{
		BDBG_LOG(("Bottom Bar Start Line #              : %d ",
			NewAvi->BottomBarStartLineNumber )) ;
	}


	if (OldAvi->LeftBarEndPixelNumber != NewAvi->LeftBarEndPixelNumber)
	{
		BDBG_LOG(("Left Bar End Pixel #                 : %d was %d ",
			NewAvi->LeftBarEndPixelNumber, OldAvi->LeftBarEndPixelNumber )) ;
	}
	else
	{
		BDBG_LOG(("Left Bar End Pixel #                 : %d ",
			NewAvi->LeftBarEndPixelNumber )) ;
	}


	if (OldAvi->RightBarEndPixelNumber != NewAvi->RightBarEndPixelNumber)
	{
		BDBG_LOG(("Right Bar End Pixel #                : %d was %d ",
			NewAvi->RightBarEndPixelNumber, OldAvi->RightBarEndPixelNumber )) ;
	}
	else
	{
		BDBG_LOG(("Right Bar End Pixel #                : %d ",
			NewAvi->RightBarEndPixelNumber)) ;
	}

	BDBG_LOG(("--------------------- END AVI INFOFRAME  -------------------")) ;

}



void BHDR_P_DEBUG_AudioInfoFrame(
	BAVC_HDMI_AudioInfoFrame *OldAudioInfoFrame,
	BAVC_HDMI_AudioInfoFrame *NewAudioInfoFrame)
{
	BDBG_LOG(("-------------------- PARSED AUDIO INFOFRAME -------------------")) ;

	if (OldAudioInfoFrame->CodingType != NewAudioInfoFrame->CodingType)
	{
		BDBG_LOG(("(CT3_CT0)       Coding Type          : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_CodingTypeToStr(NewAudioInfoFrame->CodingType),
			NewAudioInfoFrame->CodingType,
			BAVC_HDMI_AudioInfoFrame_CodingTypeToStr(OldAudioInfoFrame->CodingType),
			OldAudioInfoFrame->CodingType)) ;
	}
	else
	{
		BDBG_LOG(("(CT3_CT0)       Coding Type         : %s (%d)",
		BAVC_HDMI_AudioInfoFrame_CodingTypeToStr(NewAudioInfoFrame->CodingType),
		NewAudioInfoFrame->CodingType)) ;
	}


	if (OldAudioInfoFrame->ChannelCount != NewAudioInfoFrame->ChannelCount)
	{
		BDBG_LOG(("(CC2_CC0)       Channel Count       : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(NewAudioInfoFrame->ChannelCount),
			NewAudioInfoFrame->ChannelCount,
			BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(OldAudioInfoFrame->ChannelCount),
			OldAudioInfoFrame->ChannelCount)) ;
	}
	else
	{
		BDBG_LOG(("(CC2_CC0)       Channel Count       : %s (%d)",
			BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(NewAudioInfoFrame->ChannelCount),
			NewAudioInfoFrame->ChannelCount)) ;
	}


	if (OldAudioInfoFrame->SampleFrequency != NewAudioInfoFrame->SampleFrequency)
	{
		BDBG_LOG(("(SF2_SF0)       Sample Frequency    : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_SampleFrequencyToStr(NewAudioInfoFrame->SampleFrequency),
			NewAudioInfoFrame->SampleFrequency,
			BAVC_HDMI_AudioInfoFrame_SampleFrequencyToStr(OldAudioInfoFrame->SampleFrequency),
			OldAudioInfoFrame->SampleFrequency)) ;
	}
	else
	{
		BDBG_LOG(("(SF2_SF0)       Sample Frequency    : %s (%d)",
			BAVC_HDMI_AudioInfoFrame_SampleFrequencyToStr(NewAudioInfoFrame->SampleFrequency),
			NewAudioInfoFrame->SampleFrequency)) ;
	}


	if (OldAudioInfoFrame->SampleSize != NewAudioInfoFrame->SampleSize)
	{
		BDBG_LOG(("(SS1SS0)        Sample Size         : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_SampleSizeToStr(NewAudioInfoFrame->SampleSize),
			NewAudioInfoFrame->SampleSize,
			BAVC_HDMI_AudioInfoFrame_SampleSizeToStr(OldAudioInfoFrame->SampleSize),
			OldAudioInfoFrame->SampleSize)) ;
	}
	else
	{
		BDBG_LOG(("(SS1SS0)        Sample Size         : %s (%d)",
			BAVC_HDMI_AudioInfoFrame_SampleSizeToStr(NewAudioInfoFrame->SampleSize),
			NewAudioInfoFrame->SampleSize)) ;
	}


	if (OldAudioInfoFrame->LevelShift != NewAudioInfoFrame->LevelShift)
	{
		BDBG_LOG(("(LSV3_LSV0)     Level Shift Value   : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_LevelShiftToStr(NewAudioInfoFrame->LevelShift),
			NewAudioInfoFrame->LevelShift,
			BAVC_HDMI_AudioInfoFrame_LevelShiftToStr(OldAudioInfoFrame->LevelShift),
			OldAudioInfoFrame->LevelShift)) ;
	}
	else
	{	BDBG_LOG(("(LSV3_LSV0)     Level Shift Value   : %s (%d)",
		BAVC_HDMI_AudioInfoFrame_LevelShiftToStr(NewAudioInfoFrame->LevelShift),
		NewAudioInfoFrame->LevelShift)) ;
	}


	if (OldAudioInfoFrame->DownMixInhibit != NewAudioInfoFrame->DownMixInhibit)
	{
		BDBG_LOG(("(DM)            Down Mix            : %s (%d) was %s (%d)",
			BAVC_HDMI_AudioInfoFrame_DownMixInhibitToStr(NewAudioInfoFrame->DownMixInhibit),
			NewAudioInfoFrame->DownMixInhibit,
			BAVC_HDMI_AudioInfoFrame_DownMixInhibitToStr(OldAudioInfoFrame->DownMixInhibit),
			OldAudioInfoFrame->DownMixInhibit)) ;
	}
	else
	{
		BDBG_LOG(("(DM)            Down Mix            : %s (%d)",
			BAVC_HDMI_AudioInfoFrame_DownMixInhibitToStr(NewAudioInfoFrame->DownMixInhibit),
			NewAudioInfoFrame->DownMixInhibit)) ;
	}
	BDBG_LOG(("--------------------- END AUDIO INFOFRAME  -------------------")) ;

}


void BHDR_P_DEBUG_SpdInfoFrame(
	BAVC_HDMI_SPDInfoFrame *OldSpdInfoFrame,
	BAVC_HDMI_SPDInfoFrame *NewSpdInfoFrame)
{
	BSTD_UNUSED(OldSpdInfoFrame) ;

	BDBG_LOG(("-------------------- PARSED SPD INFOFRAME -------------------")) ;
	BDBG_LOG(("Vendor Name: <%s>", NewSpdInfoFrame->VendorName)) ;
	BDBG_LOG(("Description: <%s>",  NewSpdInfoFrame->ProductDescription)) ;
	BDBG_LOG(("Type: %s (%d)",
		BAVC_HDMI_SpdInfoFrame_SourceTypeToStr(NewSpdInfoFrame->SourceDeviceInfo),
		NewSpdInfoFrame->SourceDeviceInfo)) ;

	/* use a marker to separate packet data */
	BDBG_LOG(("--------------------- END SPD INFOFRAME ---------------------")) ;
}


void BHDR_P_DEBUG_VsInfoFrame(
	BAVC_HDMI_VendorSpecificInfoFrame *OldVsInfoFrame,
	BAVC_HDMI_VendorSpecificInfoFrame *NewVsInfoFrame)
{
	BSTD_UNUSED(OldVsInfoFrame) ;

	BDBG_LOG(("-------------------- PARSED VS INFOFRAME -------------------")) ;

	BDBG_LOG(("IEEE RegId: <0x%02x%02X%02X>",
		NewVsInfoFrame->uIEEE_RegId[2],
		NewVsInfoFrame->uIEEE_RegId[1],
		NewVsInfoFrame->uIEEE_RegId[0])) ;

	BDBG_LOG(("HDMI Video Format: <%s> (%d)",
		BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr(
			NewVsInfoFrame->eHdmiVideoFormat),
			NewVsInfoFrame->eHdmiVideoFormat)) ;

	switch (NewVsInfoFrame->eHdmiVideoFormat)
	{
	default :
	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone :
		break ;

	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution:
		BDBG_LOG(("HDMI VIC: <%s> (%d)",
			BAVC_HDMI_VsInfoFrame_HdmiVicToStr(NewVsInfoFrame->eHdmiVic),
				NewVsInfoFrame->eHdmiVic)) ;
		break ;

	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat :
		BDBG_LOG(("3D Struct: <%s> (%d)",
			BAVC_HDMI_VsInfoFrame_3DStructureToStr(NewVsInfoFrame->e3DStructure),
			NewVsInfoFrame->e3DStructure)) ;

		if ((NewVsInfoFrame->e3DStructure <=  BAVC_HDMI_VSInfoFrame_3DStructure_eSidexSideHalf)
		&& (NewVsInfoFrame->e3DStructure !=  BAVC_HDMI_VSInfoFrame_3DStructure_eReserved))
		{
			BDBG_LOG(("3D Ext Data: <%s> (%d)",
				BAVC_HDMI_VsInfoFrame_3DExtDataToStr(NewVsInfoFrame->e3DExtData),
				NewVsInfoFrame->e3DExtData)) ;
		}
		else
		{
			BDBG_LOG(("3D Ext Data: %d is not used", NewVsInfoFrame->e3DExtData)) ;
		}
		break ;

	}

	/* use a marker to separate packet data */
	BDBG_LOG(("--------------------- END VS INFOFRAME ---------------------")) ;
}


void BHDR_P_DEBUG_AcrPacket(
	BAVC_HDMI_AudioClockRegenerationPacket *OldACR,
	BAVC_HDMI_AudioClockRegenerationPacket *NewACR)
{
	BSTD_UNUSED(OldACR) ;

	BDBG_LOG(("-------------------- PARSED ACR PACKET -------------------")) ;
	BDBG_LOG(("ACR:       N= %8d        CTS= %8d", NewACR->N, NewACR->CTS)) ;

	/* use a marker to separate packet data */
	BDBG_LOG(("--------------------- END ACR PACKET ---------------------")) ;
}

#if BHDR_CONFIG_DEBUG_TIMER_S
/******************************************************************************
void BHDR_P_DebugMonitorHdmiRx_isr
Summary: Debug Monitor function to display information at BHDR_CONFIG_DEBUG_TIMER_S
intervals
*******************************************************************************/
void BHDR_P_DebugMonitorHdmiRx_isr (BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;

	BDBG_ENTER(BHDR_P_DebugMonitorHdmiRx_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	ulOffset = hHDR->ulOffset ;
	hRegister = hHDR->hRegister ;


	Register = BREG_Read32(hRegister, BCHP_HD_DVI_0_V0_COEFF_C01_C00 + ulOffset) ;
	if (hHDR->ulVoCooef_C01_C00 != Register)
	{
		BDBG_WRN(("HD_DVI_0_V0_COEFF_C01_C00	changed from %x to %x",
			hHDR->ulVoCooef_C01_C00, Register)) ;
		hHDR->ulVoCooef_C01_C00 = Register;
	}

	Register = BREG_Read32(hRegister, BCHP_HD_DVI_0_INPUT_CNTRL  + ulOffset) ;
	Register = BCHP_GET_FIELD_DATA(Register, HD_DVI_0_INPUT_CNTRL , USE_RGB_TO_YCRCB) ;
	if (hHDR->ulUseRgbToRcbCr != Register)
	{
		BDBG_WRN(("USE RGB TO YCBCR changed from %x",
			hHDR->ulUseRgbToRcbCr, Register)) ;
		hHDR->ulUseRgbToRcbCr = (uint8_t) Register ;
	}

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_MISC_STATUS + ulOffset) ;
	Register = BCHP_GET_FIELD_DATA(Register, HDMI_RX_0_MISC_STATUS, AV_MUTE) ;
	if (hHDR->ulAvMute != Register)
	{
		BDBG_WRN(("AV MUTE STATUS changed from %x to %x",
			hHDR->ulAvMute, Register)) ;
		hHDR->ulAvMute = Register;
	}
	BDBG_LEAVE(BHDR_P_DebugMonitorHdmiRx_isr) ;

}

#endif


/******************************************************************************
Summary: Debug Function to dump Packet Contents at isr time
*******************************************************************************/
BERR_Code BHDR_P_DEBUG_DumpPacketRam_isr(
   BHDR_Handle hHDR, uint8_t PacketNumber, BAVC_HDMI_Packet *Packet
)
{
	BREG_Handle hRegister ;
	uint32_t rc = BERR_SUCCESS ;
	uint32_t Register ;
	uint32_t RegAddr ;
	uint32_t ulOffset ;
	uint8_t i ;

	BDBG_ENTER(BHDR_P_DEBUG_DumpPacketRam_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	BDBG_LOG(("Packet Register Dump:")) ;
	BDBG_LOG(("")) ;

	RegAddr = BCHP_HDMI_RX_0_RAM_PACKET_0_HEADER  + ulOffset
		+ PacketNumber * 4 * BHDR_P_PACKET_WORDS  ;
	Register = BREG_Read32_isr(hRegister, RegAddr) ;

	BDBG_LOG(("H [%#x] = 0x%08x | Type: 0x%02x Ver: 0x%02x Len: %2d bytes",
		RegAddr, Register, Packet->Type, Packet->Version, Packet->Length)) ;

	for (i = 0 ; i < BHDR_P_PACKET_WORDS; i++)
	{
		Register = BREG_Read32_isr(hRegister, RegAddr) ;
		BDBG_LOG(("%d [%#x] = 0x%08x",
			i, RegAddr, Register)) ;

		RegAddr = RegAddr + 4 ;
	}

	/* the following code is for debugging received packets; does not need to be enabled */
#if 0
	/* disable dump for kernel mode */
#if BHDR_SPECIAL_DEBUG
	BDBG_WRN(("")) ;
	BDBG_WRN(("Packet Data:")) ;

	for (i = 0 ; i < BHDR_P_PACKET_BYTES ; i = i + 4)
	{
		BDBG_LOG(("Bytes %02d..%02d : %02x %02x %02x %02x       %c%c%c%c",
			i + 3, i,
			Packet->DataBytes[i+3], Packet->DataBytes[i+2], Packet->DataBytes[i+1], Packet->DataBytes[i],
			isprint(Packet->DataBytes[i+3]) ? Packet->DataBytes[i+3] : '.',
			isprint(Packet->DataBytes[i+2]) ? Packet->DataBytes[i+2] : '.',
			isprint(Packet->DataBytes[i+1]) ? Packet->DataBytes[i+1] : '.',
			isprint(Packet->DataBytes[i])    ? Packet->DataBytes[i]   : '.')) ;
	}
#endif
#endif

	BDBG_LEAVE(BHDR_P_DEBUG_DumpPacketRam_isr) ;
	return rc ;
}


void BHDR_DEBUG_P_ConfigureEventCounter(
	BHDR_Handle hHDR, BHDR_DEBUG_P_EventCounter  *pEventCounter)
{
	BREG_Handle hRegister ;
	uint32_t Register,
		EventStatsEnable_n0_Addr ,  /* Bits 00..31 */
		EventStatsEnable_n1_Addr ;  /* Bits 32..63 */

	uint32_t ulOffset ;

	BDBG_ENTER(BHDR_DEBUG_P_ConfigureEventCounter) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset ;

	/* configure for Counters for BCH Events */
	Register = BREG_Read32(hHDR->hRegister, BCHP_HDMI_RX_0_EVENT_STATS_CONFIG + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_0_EVENT_STATS_CONFIG, SELECT_BCH_EVENTS) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_EVENT_STATS_CONFIG, SELECT_BCH_EVENTS,
		pEventCounter->bBchEvent) ;
	BREG_Write32(hHDR->hRegister,  BCHP_HDMI_RX_0_EVENT_STATS_CONFIG + ulOffset, Register) ;

	EventStatsEnable_n0_Addr =
		BCHP_HDMI_RX_0_EVENT_STATS_ENABLE_00 + pEventCounter->uiCounter * 8 ;
	EventStatsEnable_n1_Addr =
		BCHP_HDMI_RX_0_EVENT_STATS_ENABLE_01 + pEventCounter->uiCounter * 8 ;

	Register = BREG_Read32(hRegister, EventStatsEnable_n0_Addr + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_0_EVENT_STATS_ENABLE_00, BITS_31_0) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_EVENT_STATS_ENABLE_00, BITS_31_0,
		pEventCounter->ulEventBitMask31_00) ;
	BREG_Write32(hRegister,  EventStatsEnable_n0_Addr+ ulOffset, Register) ;

	Register = BREG_Read32(hHDR->hRegister, EventStatsEnable_n1_Addr + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_0_EVENT_STATS_ENABLE_01, BITS_63_32) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_EVENT_STATS_ENABLE_01, BITS_63_32,
		pEventCounter->ulEventBitMask63_32) ;
	BREG_Write32(hHDR->hRegister,  EventStatsEnable_n1_Addr + ulOffset, Register) ;

	BDBG_LEAVE(BHDR_DEBUG_P_ConfigureEventCounter) ;

}


void BHDR_DEBUG_P_ResetAllEventCounters_isr(BHDR_Handle hHDR)
{
   	BREG_Handle hRegister  ;
	uint32_t Register ;
	uint32_t ulOffset  ;

	BDBG_ENTER(BHDR_DEBUG_P_ResetAllEventCounters_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	/* reset all counters */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PERT_CONFIG + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_0_PERT_CONFIG, REGISTER_AND_CLEAR_EVENT_COUNTERS) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_PERT_CONFIG, REGISTER_AND_CLEAR_EVENT_COUNTERS, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PERT_CONFIG + ulOffset, Register) ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PERT_CONFIG + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_RX_0_PERT_CONFIG, REGISTER_AND_CLEAR_EVENT_COUNTERS) ;
	Register |= BCHP_FIELD_DATA(HDMI_RX_0_PERT_CONFIG, REGISTER_AND_CLEAR_EVENT_COUNTERS, 0) ;
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PERT_CONFIG + ulOffset, Register) ;

	BDBG_LEAVE(BHDR_DEBUG_P_ResetAllEventCounters_isr) ;
}


#if BDBG_DEBUG_BUILD
#if HDMI_RX_GEN != 7445
BERR_Code BHDR_DEBUG_EnableHdmiRxStandaloneMode(BHDR_Handle hHDR)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset   ;

	BDBG_ENTER(BHDR_DEBUG_EnableHdmiRxStandaloneMode) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	ulOffset = hHDR->ulOffset ;
	hRegister = hHDR->hRegister ;

	BDBG_WRN(("CH%d HDMI Rx is in standalone mode... video data will not display",
				hHDR->eCoreId)) ;

	Register = BREG_Read32(hRegister, BCHP_DVP_HR_DVP_FLOW_CONTROL + ulOffset) ;
	Register |= BCHP_MASK(DVP_HR_DVP_FLOW_CONTROL, HDMI_RX_0_ALWAYS_ACCEPT) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_DVP_FLOW_CONTROL + ulOffset, Register) ;

	/* ALWAYS ACCEPT should not be set for normal operation */
	Register = BREG_Read32(hRegister, BCHP_DVP_HR_DVP_FLOW_CONTROL + ulOffset) ;
	Register |= BCHP_MASK(DVP_HR_DVP_FLOW_CONTROL, HD_DVI_0_ALWAYS_ACCEPT) ;
	BREG_Write32(hRegister, BCHP_DVP_HR_DVP_FLOW_CONTROL + ulOffset, Register) ;


	BDBG_LEAVE(BHDR_DEBUG_EnableHdmiRxStandaloneMode) ;
	return BERR_SUCCESS ;
}
#endif
#endif


