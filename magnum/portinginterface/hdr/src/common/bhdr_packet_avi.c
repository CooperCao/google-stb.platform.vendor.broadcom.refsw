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
#include "bavc_hdmi.h"

#include "bhdr.h"
#include "bhdr_priv.h"

BDBG_MODULE(BHDR_PACKET_AVI) ;


#if BHDR_CONFIG_DEBUG_PACKET_AVI
static void BHDR_P_DEBUG_AviInfoFrame(
	BAVC_HDMI_AviInfoFrame *OldAvi,
	BAVC_HDMI_AviInfoFrame *NewAvi)
{
	BDBG_LOG(("=== NEW AVI IF ===")) ;
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

	BDBG_LOG(("=== END AVI IF ===")) ;

}
#endif


/******************************************************************************
Summary:
Parse AVI Info Frame data from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseAviInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{

	BERR_Code rc = BERR_SUCCESS ;
	BAVC_HDMI_AviInfoFrame stNewAviInfoFrame ;
	uint8_t temp ;

	BDBG_ENTER(BHDR_P_ParseAviInfoFrameData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;


	if (hHDR->DeviceSettings.bParseAVI == false)
	{
		/* return with just a raw copy of the HDMI Packet structure  */
		BKNI_Memcpy_isr(&hHDR->AviInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;
		return rc ;
	}

	BKNI_Memset_isr(&stNewAviInfoFrame, 0, sizeof(BAVC_HDMI_AviInfoFrame)) ;

	/* Keep a raw copy of the HDMI Packet structure  */
	BKNI_Memcpy_isr(&stNewAviInfoFrame.stPacket, Packet, sizeof(BAVC_HDMI_Packet)) ;


	/* parse all fields and store in the local HDMI structure */

	/* AVI Infoframe Data Byte 1 */
	/* AviInfoFrame.ePixelEncoding */
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x60 ;
	temp = temp >> 5 ;
	stNewAviInfoFrame.ePixelEncoding = temp ;

	/* AviInfoFrame.eActiveInfo*/
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x10 ;
	temp = temp >> 4 ;
	stNewAviInfoFrame.eActiveInfo = temp ;

	/* AviInfoFrame.eBarInfo */
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x0C ;
	temp = temp >> 2 ;
	stNewAviInfoFrame.eBarInfo = temp ;

	/* AviInfoFrame.eScanInfo */
	temp = Packet->DataBytes[1] ;
	temp = temp & 0x03 ;
	stNewAviInfoFrame.eScanInfo = temp ;


	/* AVI Infoframe Data Byte 2 */
	/* AviInfoFrame.eColorimetry */
	temp = Packet->DataBytes[2];
	temp = temp & 0xC0 ;
	temp = temp >> 6 ;
	stNewAviInfoFrame.eColorimetry = temp ;

	/* AviInfoFrame.ePictureAspectRatio */
	temp = Packet->DataBytes[2];
	temp = temp & 0x30 ;
	temp = temp >> 4 ;
	stNewAviInfoFrame.ePictureAspectRatio = temp ;

	/* AviInfoFrame.eActiveFormatAspectRatio */
	temp = Packet->DataBytes[2] ;
	temp = temp & 0x0F ;
	stNewAviInfoFrame.eActiveFormatAspectRatio = temp ;

	/* AVI Infoframe Data Byte 3 */
	/* AviInfoFrame.eITContent */
	temp = Packet->DataBytes[3] ;
	temp = temp & 0x80 ;
	temp = temp >> 7 ;
	stNewAviInfoFrame.eITContent= temp ;

	/* AVI Infoframe Data Byte 3 */
	/* AviInfoFrame.eExtendedColorimetry */
	temp = Packet->DataBytes[3] ;
	temp = temp & 0x70 ;
	temp = temp >> 4 ;
	stNewAviInfoFrame.eExtendedColorimetry= temp ;


	/* AVI Infoframe Data Byte 3 */
	/* AviInfoFrame.eRGBQuantizationRange */
	temp = Packet->DataBytes[3] ;
	temp = temp & 0x0c ;
	temp = temp >> 2 ;
	stNewAviInfoFrame.eRGBQuantizationRange= temp ;

	/* AVI Infoframe Data Byte 3 */
	/* AviInfoFrame.eScaling */
	temp = Packet->DataBytes[3] ;
	temp = temp & 0x03 ;
	stNewAviInfoFrame.eScaling = temp ;


	/* AVI Infoframe Data Byte 4 */
	/* AviInfoFrame.VideoIdCode */
	temp = Packet->DataBytes[4] ;
	temp = temp & 0x7F ;
	stNewAviInfoFrame.VideoIdCode = temp ;


	/* AVI Infoframe Data Byte 5 */
	/* AviInfoFrame.PixelRepeat */
	temp = Packet->DataBytes[5] ;
	temp = temp & 0x0F ;
	stNewAviInfoFrame.PixelRepeat = temp ;

	/* AVI Infoframe Data Byte 5 */
	/* AviInfoFrame.eContentType */
	temp = Packet->DataBytes[5] ;
	temp = temp & 0x30 ;
	temp = temp >> 4 ;
	stNewAviInfoFrame.eContentType = temp ;

	/* AVI Infoframe Data Byte 5 */
	/* AviInfoFrame.eYccQuantizationRange */
	temp = Packet->DataBytes[5] ;
	temp = temp & 0xC0 ;
	temp = temp >> 6 ;
	stNewAviInfoFrame.eYccQuantizationRange = temp ;

	if (hHDR->AviInfoFrame.eBarInfo != BAVC_HDMI_AviInfoFrame_BarInfo_eInvalid)
	{
		/* get bar info */
		stNewAviInfoFrame.TopBarEndLineNumber =
			(uint16_t) ((Packet->DataBytes[7] << 8) | Packet->DataBytes[6]) ;

		stNewAviInfoFrame.BottomBarStartLineNumber =
			(uint16_t) ((Packet->DataBytes[9] << 8) | Packet->DataBytes[8]) ;

		stNewAviInfoFrame.LeftBarEndPixelNumber =
			(uint16_t) ((Packet->DataBytes[11] << 8) | Packet->DataBytes[10]) ;

		stNewAviInfoFrame.RightBarEndPixelNumber =
			(uint16_t) ((Packet->DataBytes[13] << 8) | Packet->DataBytes[12]) ;
	}


#if BHDR_CONFIG_DEBUG_PACKET_AVI
	BHDR_P_DEBUG_AviInfoFrame(&hHDR->AviInfoFrame, &stNewAviInfoFrame) ;
#endif


	/* copy the new packet */
	BKNI_Memcpy_isr(&hHDR->AviInfoFrame, &stNewAviInfoFrame, sizeof(BAVC_HDMI_AviInfoFrame)) ;

	/* call  the callback functions for Format Change notification  */
	if (hHDR->pfVideoFormatChangeCallback)
	{
		hHDR->pfVideoFormatChangeCallback(hHDR->pvVideoFormatChangeParm1,
			hHDR->iVideoFormatChangeParm2, &hHDR->AviInfoFrame) ;
	}

	BDBG_LEAVE(BHDR_P_ParseAviInfoFrameData_isr) ;
	return rc ;
}

/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_GetAviInfoFrameData(BHDR_Handle hHDR,
	BAVC_HDMI_AviInfoFrame *AviInfoFrame)
{
	BKNI_Memcpy(AviInfoFrame, &hHDR->AviInfoFrame, sizeof(BAVC_HDMI_AviInfoFrame)) ;
	return BERR_SUCCESS ;
}
