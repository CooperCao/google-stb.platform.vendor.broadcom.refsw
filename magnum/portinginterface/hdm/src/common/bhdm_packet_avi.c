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

#include "bhdm.h"
#include "bhdm_priv.h"
#include "bavc.h"

BDBG_MODULE(BHDM_PACKET_AVI) ;


/******************************************************************************
Summary:
Set/Enable the Auxillary Video Information Info frame to be sent to the HDMI Rx
*******************************************************************************/
void BHDM_DisplayAVIInfoFramePacket(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
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

	BDBG_LOG(("*** AVI INFOFRAME")) ;

	/* display original bOverrideDefaults setting */
	BDBG_LOG(("Tx%d: Override Default: %s",
		hHDMI->eCoreId, pstAviInfoFrame->bOverrideDefaults ? "Yes" : "No"));

	BDBG_LOG(("Tx%d: (Y1Y0)     ColorSpace (%d): %s", hHDMI->eCoreId,
		pstAviInfoFrame->ePixelEncoding,
		BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(pstAviInfoFrame->ePixelEncoding)));
	BDBG_LOG(("Tx%d: (A0)       Active Info (%d): %s ", hHDMI->eCoreId,
		pstAviInfoFrame->eActiveInfo, BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(pstAviInfoFrame->eActiveInfo)));
	BDBG_LOG(("Tx%d: (B1B0)     Bar Info (%d): %s ", hHDMI->eCoreId,
		pstAviInfoFrame->eBarInfo, BAVC_HDMI_AviInfoFrame_BarInfoToStr(pstAviInfoFrame->eBarInfo)));
	BDBG_LOG(("Tx%d: (S1S0)     Scan Info (%d): %s", hHDMI->eCoreId,
		pstAviInfoFrame->eScanInfo, BAVC_HDMI_AviInfoFrame_ScanInfoToStr(pstAviInfoFrame->eScanInfo))) ;

	BDBG_LOG(("Tx%d: (C1C0)     Colorimetry (%d): %s", hHDMI->eCoreId,
		pstAviInfoFrame->eColorimetry,
		BAVC_HDMI_AviInfoFrame_ColorimetryToStr(pstAviInfoFrame->eColorimetry))) ;

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	if (pstAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)
	{
		BDBG_LOG(("Tx%d: (EC2..EC0) Extended Colorimetry (%d): %s", hHDMI->eCoreId,
			pstAviInfoFrame->eExtendedColorimetry,
			BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(pstAviInfoFrame->eExtendedColorimetry))) ;
	}
	else
	{
		BDBG_LOG(("Tx%d: (EC2..EC0) Extended Colorimetry Info Invalid", hHDMI->eCoreId)) ;
	}
#endif


	BDBG_LOG(("Tx%d: (M1M0)     Picture AR (%d): %s", hHDMI->eCoreId,
		pstAviInfoFrame->ePictureAspectRatio,
		BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(pstAviInfoFrame->ePictureAspectRatio))) ;

	if ((pstAviInfoFrame->eActiveFormatAspectRatio >= 8)
	&&	(pstAviInfoFrame->eActiveFormatAspectRatio <= 11))
		BDBG_LOG(("Tx%d: (R3..R0)   Active Format AR (%d): %s", hHDMI->eCoreId,
			pstAviInfoFrame->eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(pstAviInfoFrame->eActiveFormatAspectRatio - 8))) ;
	else if  ((pstAviInfoFrame->eActiveFormatAspectRatio > 12)
	&&	(pstAviInfoFrame->eActiveFormatAspectRatio <= 15))
		BDBG_LOG(("Tx%d: (R3..R0)   Active Format AR (%d): %s", hHDMI->eCoreId,
			pstAviInfoFrame->eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(pstAviInfoFrame->eActiveFormatAspectRatio - 9))) ;
	else
		BDBG_LOG(("Tx%d: Active Format AR (%d): Other", hHDMI->eCoreId,
			pstAviInfoFrame->eActiveFormatAspectRatio)) ;

	BDBG_LOG(("Tx%d: (SC1SC0)   Picture Scaling (%d): %s ", hHDMI->eCoreId,
		pstAviInfoFrame->eScaling, BAVC_HDMI_AviInfoFrame_ScalingToStr(pstAviInfoFrame->eScaling))) ;

	BDBG_LOG(("Tx%d: (VIC6..0)  Video ID Code = %d", hHDMI->eCoreId, pstAviInfoFrame->VideoIdCode )) ;
	BDBG_LOG(("Tx%d: (PR3..PR0) Pixel Repeat: %d", hHDMI->eCoreId, pstAviInfoFrame->PixelRepeat)) ;

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	BDBG_LOG(("Tx%d: (ITC)      IT Content (%d): %s", hHDMI->eCoreId, pstAviInfoFrame->eITContent,
		BAVC_HDMI_AviInfoFrame_ITContentToStr(pstAviInfoFrame->eITContent)));
	BDBG_LOG(("Tx%d: (CN1CN0)   IT Content Type (%d): %s", hHDMI->eCoreId, pstAviInfoFrame->eContentType,
		BAVC_HDMI_AviInfoFrame_ContentTypeToStr(pstAviInfoFrame->eContentType)));
	BDBG_LOG(("Tx%d: (Q1Q0)     RGB Quantization Range (%d): %s", hHDMI->eCoreId, pstAviInfoFrame->eRGBQuantizationRange,
		BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(pstAviInfoFrame->eRGBQuantizationRange)));
	BDBG_LOG(("Tx%d: (YQ1YQ0)   Ycc Quantization Range (%d): %s", hHDMI->eCoreId, pstAviInfoFrame->eYccQuantizationRange,
		BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(pstAviInfoFrame->eYccQuantizationRange)));
#endif

	BDBG_LOG(("Tx%d: Top Bar End Line Number:     %d", hHDMI->eCoreId, pstAviInfoFrame->TopBarEndLineNumber)) ;
	BDBG_LOG(("Tx%d: Bottom Bar Stop Line Number: %d", hHDMI->eCoreId, pstAviInfoFrame->BottomBarStartLineNumber)) ;

	BDBG_LOG(("Tx%d: Left Bar End Pixel Number:   %d", hHDMI->eCoreId, pstAviInfoFrame->LeftBarEndPixelNumber )) ;
	BDBG_LOG(("Tx%d: Right Bar End Pixel Number:  %d", hHDMI->eCoreId, pstAviInfoFrame->RightBarEndPixelNumber )) ;
	BDBG_LOG((" ")) ;
#else
	BSTD_UNUSED(hHDMI) ;
	BSTD_UNUSED(pstAviInfoFrame) ;
#endif
}


/******************************************************************************
Summary:
Set/Enable the Auxillary Video Information Info frame to be sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_SetAVIInfoFramePacket(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BAVC_HDMI_AviInfoFrame *stAviInfoFrame
)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	BHDM_Packet PhysicalHdmiRamPacketId ;
	BAVC_HDMI_AviInfoFrame newAviInfoFrame ;
	uint8_t PacketType ;
	uint8_t PacketVersion ;
	uint8_t PacketLength ;
	uint8_t M1M0 ;
	uint8_t VideoID = 1 ;
	uint8_t PixelRepeat ;
	uint8_t C1C0 ;
	uint8_t Y1Y0 ;
	uint8_t EC2EC1EC0 = 0 ;
#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	bool yCbCrColorspace ;
	BAVC_ColorRange eColorRange ;
#endif

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* make a local copy of AVI  */
	BKNI_Memcpy(&newAviInfoFrame, stAviInfoFrame,
		sizeof(BAVC_HDMI_AviInfoFrame)) ;

	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	PhysicalHdmiRamPacketId = BHDM_PACKET_eAVI_ID  ;

	PacketType	  = BAVC_HDMI_PacketType_eAviInfoFrame ;
	PacketVersion = BAVC_HDMI_PacketType_AviInfoFrameVersion ;
	PacketLength  = 13 ;

	/* Override AVI infoFrame info. Use settings from the AVI InfoFrame passed in */
	if (stAviInfoFrame->bOverrideDefaults)
	{
		/* Set RGB or YCrCb Colorspace */
		Y1Y0 = stAviInfoFrame->ePixelEncoding;

		/* Set Colorimetry */
		C1C0 = stAviInfoFrame->eColorimetry;

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
		if (stAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)
			EC2EC1EC0 = stAviInfoFrame->eExtendedColorimetry ;
        newAviInfoFrame.eRGBQuantizationRange = stAviInfoFrame->eRGBQuantizationRange ;
        newAviInfoFrame.eYccQuantizationRange = stAviInfoFrame->eYccQuantizationRange ;
#endif
		/* Picture Aspect Ratio*/
		M1M0 = stAviInfoFrame->ePictureAspectRatio;

		/* Video ID Code & Pixel Repetition */
		VideoID = stAviInfoFrame->VideoIdCode;
		PixelRepeat = stAviInfoFrame->PixelRepeat;
	}  /* end overrideDefaults */
	else
	{
		/* Set RGB or YCrCb Colorspace */
		switch (hHDMI->DeviceSettings.stVideoSettings.eColorSpace)
		{
		case BAVC_Colorspace_eRGB :
			Y1Y0 =	BAVC_HDMI_AviInfoFrame_Colorspace_eRGB ;
			break ;

		default :
			/* YCbCr output  */

#if BHDM_HAS_HDMI_20_SUPPORT
			/* all YCbCr encodings are supported in HDMI 2.0 enabled chips */
			Y1Y0 = hHDMI->DeviceSettings.stVideoSettings.eColorSpace ;
 #else
			Y1Y0 =  BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr444 ;
 #endif
			break ;
		}

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
		/* Set RGB or YCrCb Quantization Range (Limited vs Full) */
		switch (hHDMI->DeviceSettings.eColorimetry)
		{
		case BAVC_MatrixCoefficients_eHdmi_RGB :
			newAviInfoFrame.eRGBQuantizationRange = BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eDefault ;
			break ;

		case BAVC_MatrixCoefficients_eDvi_Full_Range_RGB :
			newAviInfoFrame.eRGBQuantizationRange = BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eFullRange ;
			break ;

		case BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr :
			newAviInfoFrame.eYccQuantizationRange = BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eFull ;
			break ;

		default :
		/* YCbCr output */
		case BAVC_MatrixCoefficients_eItu_R_BT_709 :
		case BAVC_MatrixCoefficients_eSmpte_170M :
		case BAVC_MatrixCoefficients_eXvYCC_601 :
		case BAVC_MatrixCoefficients_eXvYCC_709 :
			newAviInfoFrame.eYccQuantizationRange = BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eLimited ;
			break ;
		}


		yCbCrColorspace =
			hHDMI->DeviceSettings.stVideoSettings.eColorSpace != BAVC_Colorspace_eRGB ;


		/* set RGB and YCC quantization (colorRange) to defaults;
		    then update based on the in-use colorspace
		*/
		newAviInfoFrame.eRGBQuantizationRange = BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eDefault ;
		newAviInfoFrame.eYccQuantizationRange = BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eLimited ;
		eColorRange = hHDMI->DeviceSettings.stVideoSettings.eColorRange ;

		if (yCbCrColorspace)
		{
			switch (eColorRange)
			{
			default :
				BDBG_WRN(("Unknown Ycc Quantization Range%d... using Limited Range", eColorRange)) ;

				/**************************/
				/*       FALL THRHOUGH    */
				/* Use Default / Limited */
				/**************************/
			case BAVC_ColorRange_eLimited :
				newAviInfoFrame.eYccQuantizationRange = BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eLimited ;
				break ;

			case BAVC_ColorRange_eFull :
				if (!hHDMI->AttachedEDID.VideoCapabilityDB.bQuantizationSelectatbleYCC)
				{
					BDBG_MSG(("Attached Rx <%s> does not support selectable Ycc Quantization",
						hHDMI->AttachedEDID.MonitorName)) ;
				}
				newAviInfoFrame.eYccQuantizationRange = BAVC_HDMI_AviInfoFrame_YccQuantizationRange_eFull;
				break ;
			}
		}
		else
		{
			switch (eColorRange)
			{
			default :
				BDBG_WRN(("Unknown RGB Quantization Range %d...using Auto Range", eColorRange)) ;

				/**************************/
				/*       FALL THRHOUGH    */
				/* Use Default */
				/**************************/

			case BAVC_ColorRange_eAuto :
				newAviInfoFrame.eRGBQuantizationRange = BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eDefault ;
				break ;

			case BAVC_ColorRange_eLimited :
				if (!hHDMI->AttachedEDID.VideoCapabilityDB.bQuantizationSelectatbleRGB)
				{
					BDBG_MSG(("Attached Rx <%s> does not support selectable RGB Quantization",
						hHDMI->AttachedEDID.MonitorName)) ;
				}
				newAviInfoFrame.eRGBQuantizationRange = BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eLimitedRange ;
				break ;

			case BAVC_ColorRange_eFull :
				newAviInfoFrame.eRGBQuantizationRange = BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eFullRange ;
				break ;
			}
		}
#endif


		/* Set Colorimetry */
		EC2EC1EC0 = 0 ;  /* default extended colorimetry to 0 */
		switch (hHDMI->DeviceSettings.eColorimetry)
		{
		default :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eNoData ;
			break ;

		case BAVC_MatrixCoefficients_eItu_R_BT_709 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eItu709 ;
			break ;

		case BAVC_MatrixCoefficients_eSmpte_170M :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eSmpte170 ;
			break ;

		case BAVC_MatrixCoefficients_eXvYCC_601 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC601 ;
			break ;

		case BAVC_MatrixCoefficients_eXvYCC_709 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC709 ;
			break ;

		case BAVC_MatrixCoefficients_eItu_R_BT_2020_CL :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eItuRBt2020YcCbcCrc ;
			break ;

		case BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_eItuRBt2020YCbCr ;
			break ;
		}


		/* convert specified Aspect Ratio to use for AVI Infoframe values */
		switch (hHDMI->DeviceSettings.eAspectRatio)
		{
		default :
		case BFMT_AspectRatio_eUnknown	 : /* Unknown/Reserved */
			BDBG_WRN(("Tx%d: Unknown AspectRatio <%d> passed in AVI Frame",
				hHDMI->eCoreId, (uint8_t) hHDMI->DeviceSettings.eAspectRatio)) ;
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eNoData ;
			break ;

		case BFMT_AspectRatio_eSquarePxl : /* square pixel */
		case BFMT_AspectRatio_e221_1	 :	   /* 2.21:1 */
			BDBG_WRN(("Tx%d: Specified BAVC Aspect Ratio %d, not compatible with HDMI" ,
				hHDMI->eCoreId, (uint8_t) hHDMI->DeviceSettings.eAspectRatio)) ;
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eNoData ;
			break ;

		case BFMT_AspectRatio_e4_3		 :
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e4_3 ;
			break ;

		case BFMT_AspectRatio_e16_9 	 :
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e16_9 ;
			break ;
		}


		/* As per HDMI1.4, if HD and Aspect Ratio is 4:3, then indicate No-Data. */
		switch (hHDMI->DeviceSettings.eInputVideoFmt)
		{
		case BFMT_VideoFmt_e720p	  :
		case BFMT_VideoFmt_e720p_50Hz :
		case BFMT_VideoFmt_e720p_24Hz :
		case BFMT_VideoFmt_e720p_25Hz :
		case BFMT_VideoFmt_e720p_30Hz :
		case BFMT_VideoFmt_e1080i	  :
		case BFMT_VideoFmt_e1080i_50Hz:
		case BFMT_VideoFmt_e1080p	  :
		case BFMT_VideoFmt_e1080p_24Hz:
		case BFMT_VideoFmt_e1080p_25Hz:
		case BFMT_VideoFmt_e1080p_30Hz:
		case BFMT_VideoFmt_e1080p_50Hz:
			if (hHDMI->DeviceSettings.eAspectRatio == BFMT_AspectRatio_e4_3)
					M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eNoData ;
		break ;
			default :
		break ;
		}


		/* Video Id Code & pixel repetitions */
		BHDM_P_VideoFmt2CEA861Code(hHDMI->DeviceSettings.eInputVideoFmt,
			hHDMI->DeviceSettings.eAspectRatio, hHDMI->DeviceSettings.ePixelRepetition,
			&VideoID) ;

		switch (VideoID)
		{
		case  6 :  	case  7 :
		case 14 :  	case 15 :
		case 21 :  	case 22 :
		case 29 :  	case 30 :
			PixelRepeat = BAVC_HDMI_PixelRepetition_e1x;
			break;

		case 10 :  	case 11 :
		case 25 :  	case 26 :
		case 35 :  	case 36 :
		case 37 :  	case 38 :
			PixelRepeat = BAVC_HDMI_PixelRepetition_e4x;
			break;

		default:
			PixelRepeat = BAVC_HDMI_PixelRepetition_eNone;
			break;
		}
	}

	/* Update derived or overridden AVI fields */

	/* Pixel Encoding  */
	newAviInfoFrame.ePixelEncoding = Y1Y0 ;

	/* Update AVI Colorimetry */
	newAviInfoFrame.eColorimetry = C1C0;
#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	newAviInfoFrame.eExtendedColorimetry = EC2EC1EC0;
#endif

	/* Picture Aspect Ratio */
	newAviInfoFrame.ePictureAspectRatio = M1M0 ;


	/* Video ID and Pixel Repeat */
	newAviInfoFrame.VideoIdCode = VideoID;
	newAviInfoFrame.PixelRepeat = PixelRepeat;

	/* always set the bOverrideDefaults to false */
	/* so subsequent calls to Get/Set AVI do not force override of settings */
	/* all overrides must be explicitly forced by the caller */
	newAviInfoFrame.bOverrideDefaults = false ;
/* BANDREWS - need overrides to be remembered, because user is not the only caller */
	newAviInfoFrame.bOverrideDefaults = stAviInfoFrame->bOverrideDefaults ;

	/* Construct AVI InfoFrame pay loads */
	hHDMI->PacketBytes[1] =
		  newAviInfoFrame.ePixelEncoding << 5 /* Y1Y0 Colorspace */
		| newAviInfoFrame.eActiveInfo << 4   /*    A0 Active Information Valid */
		| newAviInfoFrame.eBarInfo << 2      /* B1B0 Bar Info Valid */
		| newAviInfoFrame.eScanInfo ;	      /* S1S0 Scan Information */

	hHDMI->PacketBytes[2] =
		  newAviInfoFrame.eColorimetry << 6   /* C1C0 Colorimetry */
		| newAviInfoFrame.ePictureAspectRatio << 4     /* M1M0 */
		| newAviInfoFrame.eActiveFormatAspectRatio ;  /* R3..R0 */

	hHDMI->PacketBytes[3] = newAviInfoFrame.eScaling ;
	if (newAviInfoFrame.eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)
	{
#if BHDM_CONFIG_HDMI_1_3_SUPPORT
		hHDMI->PacketBytes[3] |=
			newAviInfoFrame.eExtendedColorimetry << 4;  /* EC2EC1EC0 */
#endif
	}

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	hHDMI->PacketBytes[3] |=
		  newAviInfoFrame.eITContent << 7  /* ITC */
		| newAviInfoFrame.eRGBQuantizationRange << 2;  /* Q1Q0 */
#endif


	hHDMI->PacketBytes[4] = VideoID ;
	hHDMI->PacketBytes[5] = PixelRepeat ;
#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	hHDMI->PacketBytes[5] |=
		  newAviInfoFrame.eContentType << 4  /* CN1CN0 */
		| newAviInfoFrame.eYccQuantizationRange << 6; /* YQ1YQ0 */
#endif


	hHDMI->PacketBytes[6]  = (uint8_t) (newAviInfoFrame.TopBarEndLineNumber & 0x00FF) ;
	hHDMI->PacketBytes[7]  = (uint8_t) (newAviInfoFrame.TopBarEndLineNumber >> 8) ;

	hHDMI->PacketBytes[8]  = (uint8_t) (newAviInfoFrame.BottomBarStartLineNumber & 0x00FF) ;
	hHDMI->PacketBytes[9]  = (uint8_t) (newAviInfoFrame.BottomBarStartLineNumber >> 8) ;

	hHDMI->PacketBytes[10] = (uint8_t) (newAviInfoFrame.LeftBarEndPixelNumber & 0x00FF) ;
	hHDMI->PacketBytes[11] = (uint8_t) (newAviInfoFrame.LeftBarEndPixelNumber >> 8) ;

	hHDMI->PacketBytes[12] = (uint8_t) (newAviInfoFrame.RightBarEndPixelNumber & 0x00FF) ;
	hHDMI->PacketBytes[13] = (uint8_t) (newAviInfoFrame.RightBarEndPixelNumber >> 8) ;

	BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes) ;

	/* update current device settings with new AviInfoFrame settings */
	BKNI_Memcpy(&(hHDMI->DeviceSettings.stAviInfoFrame), &newAviInfoFrame,
		sizeof(BAVC_HDMI_AviInfoFrame)) ;


#if BDBG_DEBUG_BUILD
	{
		BDBG_Level level ;

		BDBG_GetModuleLevel("BHDM_PACKET_AVI", &level) ;
		if (level == BDBG_eMsg)
		{
			BDBG_LOG(("Tx%d: AVI Packet Type: 0x%02x  Version %d  Length: %d",
				hHDMI->eCoreId, PacketType, PacketVersion, PacketLength)) ;

			BHDM_DisplayAVIInfoFramePacket( hHDMI, &newAviInfoFrame) ;
		}
	}
#endif

	return rc ;
}



/******************************************************************************
Summary:
Get the Auxillary Video Information Info frame sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_GetAVIInfoFramePacket(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BAVC_HDMI_AviInfoFrame *stAviInfoFrame
)
{
	BERR_Code	   rc = BERR_SUCCESS ;
#if BHDM_CONFIG_DEBUG_AVI_INFOFRAME
	uint8_t
		Y1Y0, C1C0, M1M0,
		EC2EC1EC0=2,
		VideoID,
		PixelRepeat;
#endif

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(stAviInfoFrame, 0, sizeof(BAVC_HDMI_AviInfoFrame)) ;
	BKNI_Memcpy(stAviInfoFrame, &(hHDMI->DeviceSettings.stAviInfoFrame),
		sizeof(BAVC_HDMI_AviInfoFrame)) ;


/* translate to AVI Data for debug/display purposes */
#if BHDM_CONFIG_DEBUG_AVI_INFOFRAME

	if (stAviInfoFrame->bOverrideDefaults)
	{
		/* RGB or YCrCb Colorspace */
		Y1Y0 = stAviInfoFrame->ePixelEncoding;

		/* Colorimetry */
		C1C0 = stAviInfoFrame->eColorimetry;

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
		if (stAviInfoFrame->eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended) {
			EC2EC1EC0 = stAviInfoFrame->eExtendedColorimetry;
		}
#endif

		/* Picture Aspect Ratio*/
		M1M0 = stAviInfoFrame->ePictureAspectRatio;

		/* Video ID Code & Pixel Repetition */
		VideoID = stAviInfoFrame->VideoIdCode;
		PixelRepeat = stAviInfoFrame->PixelRepeat;
	}
	else
	{
		Y1Y0 = hHDMI->DeviceSettings.stAviInfoFrame.ePixelEncoding ;

		/* Set Colorimetry */
		switch (hHDMI->DeviceSettings.eColorimetry)
		{
		default :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eNoData ;
			break ;

		case BAVC_MatrixCoefficients_eItu_R_BT_709 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eItu709 ;
			break ;

		case BAVC_MatrixCoefficients_eSmpte_170M :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eSmpte170 ;
			break ;

		case BAVC_MatrixCoefficients_eXvYCC_601 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC601;
			break ;

		case BAVC_MatrixCoefficients_eXvYCC_709 :
			C1C0 = BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended ;
			EC2EC1EC0 = BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC709;
		}


		/* convert specified Aspect Ratio to use for AVI Infoframe values */
		switch (hHDMI->DeviceSettings.eAspectRatio)
		{
		default :
		case BFMT_AspectRatio_eUnknown	 : /* Unknown/Reserved */
			BDBG_WRN(("Tx%d: Unknown AspectRatio passed in AVI Frame", hHDMI->eCoreId)) ;
			/* fall through */

		case BFMT_AspectRatio_eSquarePxl : /* square pixel */
		case BFMT_AspectRatio_e221_1	 :	   /* 2.21:1 */
			BDBG_WRN(("Tx%d: Specified BAVC Aspect Ratio %d, not compatible with HDMI",
				hHDMI->eCoreId,	hHDMI->DeviceSettings.eAspectRatio)) ;
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_eNoData ;
			break ;

		case BFMT_AspectRatio_e4_3		 :
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e4_3 ;
			break ;

		case BFMT_AspectRatio_e16_9 	 :
			M1M0 = BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e16_9 ;
			break ;
		}

		BHDM_P_VideoFmt2CEA861Code(hHDMI->DeviceSettings.eInputVideoFmt,
			hHDMI->DeviceSettings.eAspectRatio, hHDMI->DeviceSettings.ePixelRepetition,
			&VideoID) ;

		PixelRepeat = (uint8_t) hHDMI->DeviceSettings.ePixelRepetition;
	}

	BDBG_MSG(("*** Current AVI Info Frame Settings ***")) ;
	BDBG_MSG(("Tx%d: ColorSpace (%d): %s", hHDMI->eCoreId, Y1Y0,
		BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(Y1Y0))) ;
	BDBG_MSG(("Tx%d: Active Info (%d): %s ",  hHDMI->eCoreId,
		stAviInfoFrame->eActiveInfo,
		BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(stAviInfoFrame->eActiveInfo))) ;
	BDBG_MSG(("Tx%d: Bar Info (%d): %s ", hHDMI->eCoreId,
		stAviInfoFrame->eBarInfo,
		BAVC_HDMI_AviInfoFrame_BarInfoToStr(stAviInfoFrame->eBarInfo))) ;
	BDBG_MSG(("Tx%d: Scan Info (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eScanInfo,
		BAVC_HDMI_AviInfoFrame_ScanInfoToStr(stAviInfoFrame->eScanInfo))) ;
	BDBG_MSG(("Tx%d: Colorimetry (%d): %s", hHDMI->eCoreId,
		C1C0, BAVC_HDMI_AviInfoFrame_ColorimetryToStr(C1C0))) ;
	BDBG_MSG(("Tx%d: Extended Colorimetry (%d): %s", hHDMI->eCoreId, EC2EC1EC0,
		BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(EC2EC1EC0))) ;
	BDBG_MSG(("Tx%d: Picture AR (%d): %s", hHDMI->eCoreId, M1M0,
		BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(M1M0))) ;

	if ((stAviInfoFrame->eActiveFormatAspectRatio >= 8)
	&&	(stAviInfoFrame->eActiveFormatAspectRatio <= 11))
		BDBG_MSG(("Tx%d: Active Format AR (%d): %s", hHDMI->eCoreId,
			stAviInfoFrame->eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(stAviInfoFrame->eActiveFormatAspectRatio - 8))) ;
	else if  ((stAviInfoFrame->eActiveFormatAspectRatio > 12)
	&&	(stAviInfoFrame->eActiveFormatAspectRatio <= 15))
		BDBG_MSG(("Tx%d: Active Format AR (%d): %s", hHDMI->eCoreId,
			stAviInfoFrame->eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(stAviInfoFrame->eActiveFormatAspectRatio - 9))) ;
	else
		BDBG_MSG(("Tx%d: Active Format AR (%d): Other", hHDMI->eCoreId,
			stAviInfoFrame->eActiveFormatAspectRatio)) ;

	BDBG_MSG(("Tx%d: Picture Scaling (%d): %s ", hHDMI->eCoreId,
		stAviInfoFrame->eScaling,
		BAVC_HDMI_AviInfoFrame_ScalingToStr(stAviInfoFrame->eScaling))) ;

	BDBG_MSG(("Tx%d: Video ID Code = %d", hHDMI->eCoreId, VideoID)) ;
	BDBG_MSG(("Tx%d: Pixel Repeat: %d", hHDMI->eCoreId, PixelRepeat)) ;

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	BDBG_MSG(("Tx%d: IT Content (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eITContent,
		BAVC_HDMI_AviInfoFrame_ITContentToStr(stAviInfoFrame->eITContent)));
	BDBG_MSG(("Tx%d: IT Content Type (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eContentType,
		BAVC_HDMI_AviInfoFrame_ContentTypeToStr(stAviInfoFrame->eContentType)));
	BDBG_MSG(("Tx%d: RGB Quantization Range (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eRGBQuantizationRange,
		BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(stAviInfoFrame->eRGBQuantizationRange)));
	BDBG_MSG(("Tx%d: YCC Quantization Range (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eYccQuantizationRange,
		BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(stAviInfoFrame->eYccQuantizationRange)));
#endif

	BDBG_MSG(("Tx%d: Top Bar End Line Number: %d", hHDMI->eCoreId,
		stAviInfoFrame->TopBarEndLineNumber)) ;
	BDBG_MSG(("Tx%d: Bottom Bar Stop Line Number: %d", hHDMI->eCoreId,
		stAviInfoFrame->BottomBarStartLineNumber)) ;

	BDBG_MSG(("Tx%d: Left Bar End Pixel Number: %d", hHDMI->eCoreId,
		stAviInfoFrame->LeftBarEndPixelNumber )) ;
	BDBG_MSG(("Tx%d: Right Bar End Pixel Number: %d\n", hHDMI->eCoreId,
		stAviInfoFrame->RightBarEndPixelNumber )) ;

#endif

	return rc ;
}
