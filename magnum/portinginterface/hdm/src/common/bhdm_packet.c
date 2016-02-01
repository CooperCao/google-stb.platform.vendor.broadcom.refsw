/***************************************************************************
*	  (c)2004-2014 Broadcom Corporation
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
*  1.	  This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*
*  2.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
*  USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.	  TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
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
 ***************************************************************************/

#include "bstd.h"
#include "bhdm.h"
#include "bhdm_priv.h"
#include "bavc_hdmi.h"

#if BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

BDBG_MODULE(BHDM_PACKET) ;

#define BHDM_P_WORDS_PER_RAM_PACKET 9
#define BHDM_P_NUM_CONFIGURABLE_RAM_PACKETS 14

#define BHDM_REFER_TO_STREAM_HEADER 0

static const uint16_t BHDM_Packet_AVIFrameType	  = 0x82 ;
static const uint8_t  BHDM_Packet_AVIFrameVersion = 0x02 ;
static const uint8_t  BHDM_Packet_AVIFrameLength  = 0x0D ;

static const uint16_t BHDM_Packet_SPDFrameType	  = 0x83 ;
static const uint8_t  BHDM_Packet_SPDFrameVersion = 0x01 ;
static const uint8_t  BHDM_Packet_SPDFrameLength  = 25	;

static const uint16_t BHDM_Packet_AudioFrameType	= 0x84 ;
static const uint8_t  BHDM_Packet_AudioFrameVersion = 0x01 ;
static const uint8_t  BHDM_Packet_AudioFrameLength	= 0x0A ;


#if BHDM_CONFIG_HDMI_1_3_SUPPORT
/*
 * As per Chris Pasqualino:
 * ========================
 * This GBD specification based on GBD format 1, 12-bit data, defined as an RGB expression
 * of xvYCC709 coordinates.  This profile provides the min/max non-gamma corrected limited
 * range RGB values in the transmitted stream.
 *
 * Header Byte 1 = 0x81 = 1_000_0001b:
 *	Affected_Gamut_Seq_Num = 1	This is the first gamut provided
 *	GBD_profile = 0 		P0 Profile
 *	Next_Field = 1			This GBD data applies to the next field
 *
 * Header Byte 2 = 0x31 = 0_0_11_0001b:
 *	Current_Gamut_Seq_Num = 1	This gamut number being transmitted.
 *	Packet_Seq = 3			The only packet in the sequence
 *	No_Current_GBD = 0		A GBD is applicable to next video field.
 *
 * From the header, we know this is a profile 0 GBD.  This is important because it means
 * that there are no length parameters stored in the subpacket data. Thus, the MSB of the
 * first byte in subpacket 0 indicates the format of the remainder of the packet.
 *
 * Subpacket 0, Byte 0 = 0x92 = 1_0_0_10_010b:
 *	Format_Flag = 1 		GBD data is a range description
 *	GBD_Color_Precision = 2 	12-bit precision on GBD data
 *	GBD_Color_Space = 2 	RGB Expression of xvYCC709 coordinates
 *
 * The HDMI spec states in section E.4 that for Format_Flag = 1, the Packet_Range_Data
 * shall be stored in the following order:
 *	Min_Red_Data
 *	Max_Red_Data
 *	Min_Green_Data
 *	Max_Green_Data
 *	Min_Blue_Data
 *	Max_Blue_Data
 */
static const uint8_t BHDM_Packet_GamutMetadataType = 0x0A ;
static const uint8_t BHDM_Packet_GamutMetadataHB1  = 0x81 ;
static const uint8_t BHDM_Packet_GamutMetadataHB2  = 0x31 ;
static const uint8_t BHDM_Packet_GamutMetadataSubPkt[] = {
		0x92, 0x9B, 0x52, 0xF4, 0x8D,
		0x72, 0x96, 0x8C, 0xC2, 0x92
};
#endif

/***************************************************************************
BHDM_P_InitializePacketRAM
Summary: Configure/Initialize the Packet RAM with NULL Packets
****************************************************************************/
BERR_Code BHDM_InitializePacketRAM(
   const BHDM_Handle hHDMI		  /* [in] HDMI handle */
)
{
	BERR_Code	rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t	Register, ulOffset ;
	uint32_t RegAddr ;
	uint8_t Packet ;

	/*CP*  13 Enable/Configure RAM Packets - HDMI_TODO Add PUBLIC FUNCTION(s)??? */

	/* This will configure a NULL packet for sending.
	   This is just to get things up and running. */

	/* set	 RAM_ENABLE  */
	/* set	 ACTIVE_RAM_PACKETS = 14'd1   - Enable RAM Packet 0 */

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* Zero (NULL) out all RAM Packets */
	for (Packet = 0 ; Packet < BHDM_P_NUM_CONFIGURABLE_RAM_PACKETS; Packet++)
	{
		for (Register = 0 ; Register < BHDM_P_WORDS_PER_RAM_PACKET ; Register++)
		{
			RegAddr = BCHP_HDMI_RAM_GCP_0
				+ Packet * BHDM_P_WORDS_PER_RAM_PACKET * 4
				+ Register * 4 ;

			BREG_Write32(hRegister, RegAddr + ulOffset, (uint32_t) 0) ;
		}
	}

	/* Enable the Packet Ram */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset) ;
	Register |= BCHP_FIELD_DATA(HDMI_RAM_PACKET_CONFIG, RAM_ENABLE, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset,  Register) ;

	return rc ;
}


BERR_Code BHDM_EnablePacketTransmission(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BHDM_Packet PacketId)
{
	BERR_Code	   rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint32_t XmitPacketN ;
	uint8_t timeoutMs ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* set the transmission bit */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset) ;
	Register |= 1 << (uint8_t) PacketId ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset, Register) ;

	/* wait until RAM Packet Status starts transmitting (RAM_PACKET_X = 0) */
	timeoutMs = 10 ;
	do
	{
		Register = BREG_Read32(hRegister, BCHP_HDMI_RAM_PACKET_STATUS + ulOffset) ;
		XmitPacketN = Register & (1 << (uint8_t) PacketId) ;
		if (XmitPacketN)
			break ;

		BDBG_MSG(("Tx%d: Attempting start of RAM PACKET %d transmission",
			hHDMI->eCoreId, (uint8_t) PacketId)) ;
		BKNI_Sleep(1) ;
	} while ( timeoutMs-- ) ;

	if (!XmitPacketN)
	{
		BDBG_ERR(("Tx%d: Timeout Error starting RAM PACKET %d transmission",
			hHDMI->eCoreId, (uint8_t) PacketId)) ;
		rc = BERR_TRACE(BERR_TIMEOUT) ;
		goto done ;
	}

done:
	return rc ;
}



BERR_Code BHDM_DisablePacketTransmission(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BHDM_Packet PacketId)
{
	BERR_Code	   rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint32_t XmitPacketN ;
	uint8_t timeoutMs ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* clear the transmission bit */
	Register = BREG_Read32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset) ;
	Register &= ~(1 << (uint8_t) PacketId) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_PACKET_CONFIG + ulOffset, Register) ;

	/* wait until RAM Packet Status stops transmitting (RAM_PACKET_X = 0) */
	timeoutMs = 10 ;
	do
	{
		Register = BREG_Read32(hRegister, BCHP_HDMI_RAM_PACKET_STATUS + ulOffset) ;
		XmitPacketN = Register & (1 << (uint8_t) PacketId) ;
		if (!XmitPacketN)
			break ;

		BDBG_MSG(("Tx%d: Attempting stop of RAM PACKET %d transmission",
			hHDMI->eCoreId, (uint8_t) PacketId)) ;
		BKNI_Sleep(1) ;
	} while ( timeoutMs-- ) ;

	if (XmitPacketN)
	{
		BDBG_ERR(("Tx%d: Timeout Error stopping RAM PACKET %d transmission",
			hHDMI->eCoreId, (uint8_t) PacketId)) ;
		rc = BERR_TRACE(BERR_TIMEOUT) ;
		goto done ;
	}

done:
	return rc ;
}



BERR_Code BHDM_P_WritePacket(
	const BHDM_Handle hHDMI, BHDM_Packet PhysicalHdmiRamPacketId,
	uint8_t PacketType, uint8_t PacketVersion, uint8_t PacketLength,
	uint8_t *PacketBytes)
{
	BERR_Code	   rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;

	uint32_t Register, ulOffset ;

	uint32_t PacketRegisterOffset ;
	uint32_t checksum ;
	uint8_t i ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* zero out the unused packet bytes */
	for (i = PacketLength + 1 ; i < BHDM_NUM_PACKET_BYTES; i++)
		PacketBytes[i] = 0 ;

	/* calculate Packet Checksum only on CEA-861 InfoFrame Packets */
	if (PacketType > BHDM_INFOFRAME_PACKET_TYPE)
	{
		/* calculate checksum */
		checksum = 0 ;
		checksum = PacketType + PacketVersion + PacketLength ;
		for (i = 1 ; i <= PacketLength ; i++)
			checksum = checksum + PacketBytes[i] ;

		PacketBytes[0] = 256 - (uint8_t) (checksum % 256)  ;
#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
		BDBG_MSG(("Tx%d: Infoframe Checksum Byte: %#X",
			hHDMI->eCoreId, PacketBytes[0])) ;
#endif
	}
	else
	{
		/* do not modify PB0; may be used for some other purpose */
		BDBG_WRN(("Tx%d: Writing non-Infoframe Packet Type %#x",
			hHDMI->eCoreId, PacketType)) ;
	}

#ifdef BCHP_PWR_RESOURCE_HDMI_TX
	BCHP_PWR_AcquireResource(hHDMI->hChip, BCHP_PWR_RESOURCE_HDMI_TX);
#endif
	/* disable transmission of the RAM Packet */
	BHDM_CHECK_RC(rc, BHDM_DisablePacketTransmission(hHDMI, PhysicalHdmiRamPacketId)) ;


	/* calculate the offset of where the RAM for Packet ID begins */
	PacketRegisterOffset =
		   BCHP_HDMI_RAM_GCP_0
		+ BHDM_P_WORDS_PER_RAM_PACKET * 4 * PhysicalHdmiRamPacketId ;

	/* create/write the header use HDMI_RAM_PACKET_1_0 for Register field names */
	Register = BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_0, PacketType)
			 | BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_1, PacketVersion)
			 | BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_2, PacketLength) ;
	BREG_Write32(hRegister, PacketRegisterOffset + ulOffset, Register) ;

#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
	BDBG_MSG(("Tx%d: Packet Type %#X Packet %d Loading...", hHDMI->eCoreId,
		PacketType, PhysicalHdmiRamPacketId)) ;
	BDBG_MSG(("Tx%d: Addr [%#x] = %x", hHDMI->eCoreId,
		PacketRegisterOffset, Register)) ;
#endif

	/* load the Packet Bytes */
	for (i = 0 ; i < BHDM_NUM_PACKET_BYTES; i = i + 7)
	{
		PacketRegisterOffset += 4 ;

		/* create/write the subpacket data use HDMI_RAM_PACKET_1_1 for Register field names */
		Register =
			  BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_0, PacketBytes[i])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_1, PacketBytes[i+1])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_2, PacketBytes[i+2])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_3, PacketBytes[i+3]) ;
		BREG_Write32(hRegister, PacketRegisterOffset + ulOffset, Register) ;
#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
		BDBG_MSG(("Tx%d: Addr [%#x] = %#x", hHDMI->eCoreId,
			PacketRegisterOffset, Register)) ;
#endif

		PacketRegisterOffset += 4 ;

		/* create/write the subpacket data use HDMI_RAM_PACKET_1_1 for Register field names */
		Register =
			  BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_2, SUBPACKET_1_BYTE_4, PacketBytes[i+4])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_2, SUBPACKET_1_BYTE_5, PacketBytes[i+5])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_2, SUBPACKET_1_BYTE_6, PacketBytes[i+6]) ;
		BREG_Write32(hRegister, PacketRegisterOffset + ulOffset, Register) ;
#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
		BDBG_MSG(("Tx%d: Addr [%#x] = %#x", hHDMI->eCoreId,
			PacketRegisterOffset, Register)) ;
#endif
	}

	/* reenable the packet transmission */
	BHDM_CHECK_RC(rc, BHDM_EnablePacketTransmission(hHDMI, PhysicalHdmiRamPacketId)) ;
#ifdef BCHP_PWR_RESOURCE_HDMI_TX
	BCHP_PWR_ReleaseResource(hHDMI->hChip, BCHP_PWR_RESOURCE_HDMI_TX);
#endif
done:
	return rc ;
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

	PacketType	  = BHDM_Packet_AVIFrameType ;
	PacketVersion = BHDM_Packet_AVIFrameVersion ;
	PacketLength  = BHDM_Packet_AVIFrameLength ;


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

#if BHDM_CONFIG_MHL_SUPPORT
		if (hHDMI->bMhlMode &&
			(hHDMI->DeviceSettings.eInputVideoFmt == BFMT_VideoFmt_e1080p ||
			 hHDMI->DeviceSettings.eInputVideoFmt == BFMT_VideoFmt_e1080p_50Hz))
		{
			BDBG_MSG(("Forcing 1080p 444 to 422 in MHL mode."));
			Y1Y0 =  BAVC_HDMI_AviInfoFrame_Colorspace_eYCbCr422 ;
		}
#endif

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
			PixelRepeat = BAVC_HDMI_PixelRepetition_e2x;
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

	BDBG_MSG(("Tx%d: (Y1Y0)     ColorSpace (%d): %s", hHDMI->eCoreId,
		newAviInfoFrame.ePixelEncoding,
		BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(newAviInfoFrame.ePixelEncoding)));
	BDBG_MSG(("Tx%d: (C1C0)     Colorimetry (%d): %s", hHDMI->eCoreId,
		newAviInfoFrame.eColorimetry,
		BAVC_HDMI_AviInfoFrame_ColorimetryToStr(newAviInfoFrame.eColorimetry))) ;


#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	if (newAviInfoFrame.eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)
	{
		BDBG_MSG(("Tx%d: Extended Colorimetry (%d): %s", hHDMI->eCoreId,
			newAviInfoFrame.eExtendedColorimetry,
			BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(newAviInfoFrame.eExtendedColorimetry))) ;
	}
	else
	{
		BDBG_MSG(("Tx%d: Extended Colorimetry Info Invalid", hHDMI->eCoreId)) ;
	}
#endif

#if BHDM_CONFIG_DEBUG_AVI_INFOFRAME
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

	BDBG_LOG(("-------------------- NEW  AVI INFOFRAME -------------------")) ;
	BDBG_LOG(("Tx%d: Packet Type: 0x%02x  Version %d  Length: %d",
		hHDMI->eCoreId, PacketType, PacketVersion, PacketLength)) ;

	/* display original bOverrideDefaults setting */
	BDBG_LOG(("Tx%d: Override Default: %s",
		hHDMI->eCoreId, stAviInfoFrame->bOverrideDefaults ? "Yes" : "No"));

	BDBG_LOG(("Tx%d: (Y1Y0)     ColorSpace (%d): %s", hHDMI->eCoreId,
		newAviInfoFrame.ePixelEncoding,
		BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(newAviInfoFrame.ePixelEncoding)));
	BDBG_LOG(("Tx%d: (A0)       Active Info (%d): %s ", hHDMI->eCoreId,
		newAviInfoFrame.eActiveInfo, BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(newAviInfoFrame.eActiveInfo)));
	BDBG_LOG(("Tx%d: (B1B0)     Bar Info (%d): %s ", hHDMI->eCoreId,
		newAviInfoFrame.eBarInfo, BAVC_HDMI_AviInfoFrame_BarInfoToStr(newAviInfoFrame.eBarInfo)));
	BDBG_LOG(("Tx%d: (S1S0)     Scan Info (%d): %s", hHDMI->eCoreId,
		newAviInfoFrame.eScanInfo, BAVC_HDMI_AviInfoFrame_ScanInfoToStr(newAviInfoFrame.eScanInfo))) ;

	BDBG_LOG(("Tx%d: (C1C0)     Colorimetry (%d): %s", hHDMI->eCoreId,
		newAviInfoFrame.eColorimetry,
		BAVC_HDMI_AviInfoFrame_ColorimetryToStr(newAviInfoFrame.eColorimetry))) ;

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	if (newAviInfoFrame.eColorimetry == BAVC_HDMI_AviInfoFrame_Colorimetry_eExtended)
	{
		BDBG_LOG(("Tx%d: (EC2..EC0) Extended Colorimetry (%d): %s", hHDMI->eCoreId,
			newAviInfoFrame.eExtendedColorimetry,
			BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(newAviInfoFrame.eExtendedColorimetry))) ;
	}
	else
	{
		BDBG_LOG(("Tx%d: (EC2..EC0) Extended Colorimetry Info Invalid", hHDMI->eCoreId)) ;
	}
#endif


	BDBG_LOG(("Tx%d: (M1M0)     Picture AR (%d): %s", hHDMI->eCoreId,
		newAviInfoFrame.ePictureAspectRatio,
		BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(newAviInfoFrame.ePictureAspectRatio))) ;

	if ((newAviInfoFrame.eActiveFormatAspectRatio >= 8)
	&&	(newAviInfoFrame.eActiveFormatAspectRatio <= 11))
		BDBG_LOG(("Tx%d: (R3..R0)   Active Format AR (%d): %s", hHDMI->eCoreId,
			newAviInfoFrame.eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(newAviInfoFrame.eActiveFormatAspectRatio - 8))) ;
	else if  ((newAviInfoFrame.eActiveFormatAspectRatio > 12)
	&&	(newAviInfoFrame.eActiveFormatAspectRatio <= 15))
		BDBG_LOG(("Tx%d: (R3..R0)   Active Format AR (%d): %s", hHDMI->eCoreId,
			newAviInfoFrame.eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(newAviInfoFrame.eActiveFormatAspectRatio - 9))) ;
	else
		BDBG_LOG(("Tx%d: Active Format AR (%d): Other", hHDMI->eCoreId,
			newAviInfoFrame.eActiveFormatAspectRatio)) ;

	BDBG_LOG(("Tx%d: (SC1SC0)   Picture Scaling (%d): %s ", hHDMI->eCoreId,
		newAviInfoFrame.eScaling, BAVC_HDMI_AviInfoFrame_ScalingToStr(newAviInfoFrame.eScaling))) ;

	BDBG_LOG(("Tx%d: (VIC6..0)  Video ID Code = %d", hHDMI->eCoreId, newAviInfoFrame.VideoIdCode )) ;
	BDBG_LOG(("Tx%d: (PR3..PR0) Pixel Repeat: %d", hHDMI->eCoreId, newAviInfoFrame.PixelRepeat)) ;

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	BDBG_LOG(("Tx%d: (ITC)      IT Content (%d): %s", hHDMI->eCoreId, newAviInfoFrame.eITContent,
		BAVC_HDMI_AviInfoFrame_ITContentToStr(newAviInfoFrame.eITContent)));
	BDBG_LOG(("Tx%d: (CN1CN0)   IT Content Type (%d): %s", hHDMI->eCoreId, newAviInfoFrame.eContentType,
		BAVC_HDMI_AviInfoFrame_ContentTypeToStr(newAviInfoFrame.eContentType)));
	BDBG_LOG(("Tx%d: (Q1Q0)     RGB Quantization Range (%d): %s", hHDMI->eCoreId, newAviInfoFrame.eRGBQuantizationRange,
		BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(newAviInfoFrame.eRGBQuantizationRange)));
	BDBG_LOG(("Tx%d: (YQ1YQ0)   Ycc Quantization Range (%d): %s", hHDMI->eCoreId, newAviInfoFrame.eYccQuantizationRange,
		BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(newAviInfoFrame.eYccQuantizationRange)));
#endif

	BDBG_LOG(("Tx%d: Top Bar End Line Number:     %d", hHDMI->eCoreId, newAviInfoFrame.TopBarEndLineNumber)) ;
	BDBG_LOG(("Tx%d: Bottom Bar Stop Line Number: %d", hHDMI->eCoreId, newAviInfoFrame.BottomBarStartLineNumber)) ;

	BDBG_LOG(("Tx%d: Left Bar End Pixel Number:   %d", hHDMI->eCoreId, newAviInfoFrame.LeftBarEndPixelNumber )) ;
	BDBG_LOG(("Tx%d: Right Bar End Pixel Number:  %d", hHDMI->eCoreId, newAviInfoFrame.RightBarEndPixelNumber )) ;
	BDBG_LOG(("--------------------- END AVI INFOFRAME ---------------------")) ;
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
			BDBG_LOG(("Tx%d: Unknown AspectRatio passed in AVI Frame", hHDMI->eCoreId)) ;
			/* fall through */

		case BFMT_AspectRatio_eSquarePxl : /* square pixel */
		case BFMT_AspectRatio_e221_1	 :	   /* 2.21:1 */
			BDBG_LOG(("Tx%d: Specified BAVC Aspect Ratio %d, not compatible with HDMI",
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

	BDBG_LOG(("*** Current AVI Info Frame Settings ***")) ;
	BDBG_LOG(("Tx%d: ColorSpace (%d): %s", hHDMI->eCoreId, Y1Y0,
		BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(Y1Y0))) ;
	BDBG_LOG(("Tx%d: Active Info (%d): %s ",  hHDMI->eCoreId,
		stAviInfoFrame->eActiveInfo,
		BAVC_HDMI_AviInfoFrame_ActiveFormatToStr(stAviInfoFrame->eActiveInfo))) ;
	BDBG_LOG(("Tx%d: Bar Info (%d): %s ", hHDMI->eCoreId,
		stAviInfoFrame->eBarInfo,
		BAVC_HDMI_AviInfoFrame_BarInfoToStr(stAviInfoFrame->eBarInfo))) ;
	BDBG_LOG(("Tx%d: Scan Info (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eScanInfo,
		BAVC_HDMI_AviInfoFrame_ScanInfoToStr(stAviInfoFrame->eScanInfo))) ;
	BDBG_LOG(("Tx%d: Colorimetry (%d): %s", hHDMI->eCoreId,
		C1C0, BAVC_HDMI_AviInfoFrame_ColorimetryToStr(C1C0))) ;
	BDBG_LOG(("Tx%d: Extended Colorimetry (%d): %s", hHDMI->eCoreId, EC2EC1EC0,
		BAVC_HDMI_AviInfoFrame_ExtendedColorimetryToStr(EC2EC1EC0))) ;
	BDBG_LOG(("Tx%d: Picture AR (%d): %s", hHDMI->eCoreId, M1M0,
		BAVC_HDMI_AviInfoFrame_PictureAspectRatioToStr(M1M0))) ;

	if ((stAviInfoFrame->eActiveFormatAspectRatio >= 8)
	&&	(stAviInfoFrame->eActiveFormatAspectRatio <= 11))
		BDBG_LOG(("Tx%d: Active Format AR (%d): %s", hHDMI->eCoreId,
			stAviInfoFrame->eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(stAviInfoFrame->eActiveFormatAspectRatio - 8))) ;
	else if  ((stAviInfoFrame->eActiveFormatAspectRatio > 12)
	&&	(stAviInfoFrame->eActiveFormatAspectRatio <= 15))
		BDBG_LOG(("Tx%d: Active Format AR (%d): %s", hHDMI->eCoreId,
			stAviInfoFrame->eActiveFormatAspectRatio,
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatioToStr(stAviInfoFrame->eActiveFormatAspectRatio - 9))) ;
	else
		BDBG_LOG(("Tx%d: Active Format AR (%d): Other", hHDMI->eCoreId,
			stAviInfoFrame->eActiveFormatAspectRatio)) ;

	BDBG_LOG(("Tx%d: Picture Scaling (%d): %s ", hHDMI->eCoreId,
		stAviInfoFrame->eScaling,
		BAVC_HDMI_AviInfoFrame_ScalingToStr(stAviInfoFrame->eScaling))) ;

	BDBG_LOG(("Tx%d: Video ID Code = %d", hHDMI->eCoreId, VideoID)) ;
	BDBG_LOG(("Tx%d: Pixel Repeat: %d", hHDMI->eCoreId, PixelRepeat)) ;

#if BHDM_CONFIG_HDMI_1_3_SUPPORT
	BDBG_LOG(("Tx%d: IT Content (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eITContent,
		BAVC_HDMI_AviInfoFrame_ITContentToStr(stAviInfoFrame->eITContent)));
	BDBG_LOG(("Tx%d: IT Content Type (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eContentType,
		BAVC_HDMI_AviInfoFrame_ContentTypeToStr(stAviInfoFrame->eContentType)));
	BDBG_LOG(("Tx%d: RGB Quantization Range (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eRGBQuantizationRange,
		BAVC_HDMI_AviInfoFrame_RGBQuantizationRangeToStr(stAviInfoFrame->eRGBQuantizationRange)));
	BDBG_LOG(("Tx%d: YCC Quantization Range (%d): %s", hHDMI->eCoreId,
		stAviInfoFrame->eYccQuantizationRange,
		BAVC_HDMI_AviInfoFrame_YccQuantizationRangeToStr(stAviInfoFrame->eYccQuantizationRange)));
#endif

	BDBG_LOG(("Tx%d: Top Bar End Line Number: %d", hHDMI->eCoreId,
		stAviInfoFrame->TopBarEndLineNumber)) ;
	BDBG_LOG(("Tx%d: Bottom Bar Stop Line Number: %d", hHDMI->eCoreId,
		stAviInfoFrame->BottomBarStartLineNumber)) ;

	BDBG_LOG(("Tx%d: Left Bar End Pixel Number: %d", hHDMI->eCoreId,
		stAviInfoFrame->LeftBarEndPixelNumber )) ;
	BDBG_LOG(("Tx%d: Right Bar End Pixel Number: %d\n", hHDMI->eCoreId,
		stAviInfoFrame->RightBarEndPixelNumber )) ;

#endif

	return rc ;
}


/******************************************************************************
Summary:
Set/Enable the Audio Info Frame packet to be sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_SetAudioInfoFramePacket(
   const BHDM_Handle hHDMI,		  /* [in] HDMI handle */
   BAVC_HDMI_AudioInfoFrame *pstAudioInfoFrame
)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	BHDM_Packet PhysicalHdmiRamPacketId ;

	uint8_t PacketType ;
	uint8_t PacketVersion ;
	uint8_t PacketLength ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* BHDM_SetAudioInfoFramePacket also called from BHDM_EnableDisplay
	    using the AudioInfoFrame stored in the HDMI handle
	*/

	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	PhysicalHdmiRamPacketId = BHDM_PACKET_eAudioFrame_ID ;

	PacketType	  = BHDM_Packet_AudioFrameType ;
	PacketVersion = BHDM_Packet_AudioFrameVersion ;
	PacketLength  = BHDM_Packet_AudioFrameLength ;

	if ((!pstAudioInfoFrame->bOverrideDefaults)
	&& ((pstAudioInfoFrame->CodingType != BAVC_HDMI_AudioInfoFrame_CodingType_eReferToStream)
	||  (pstAudioInfoFrame->SampleFrequency)
	||  (pstAudioInfoFrame->SampleSize)))
	{
		BDBG_WRN((
			"Tx%d: Audio Coding Type, Sample Size, and Frequency are obtained from stream header",
			 hHDMI->eCoreId)) ;

		/* set the audio coding type, sample frequency, and sample size ALL to 0 */
		pstAudioInfoFrame->CodingType =
		pstAudioInfoFrame->SampleFrequency =
		pstAudioInfoFrame->SampleSize
			=  BHDM_REFER_TO_STREAM_HEADER ;
	}


	hHDMI->PacketBytes[1] =
		   pstAudioInfoFrame->CodingType << 4
		| (pstAudioInfoFrame->ChannelCount) ;
	hHDMI->PacketBytes[2] =
		   pstAudioInfoFrame->SampleFrequency << 2
		| pstAudioInfoFrame->SampleSize ;


	hHDMI->PacketBytes[3] = 0 ; /* Per HDMI Spec... Set to 0  */

	hHDMI->PacketBytes[4] =
		pstAudioInfoFrame->SpeakerAllocation ;

	hHDMI->PacketBytes[5] =
		pstAudioInfoFrame->DownMixInhibit << 7
		| pstAudioInfoFrame->LevelShift << 3 ;

	/* adjust the Audio Input Configuration to reflect any changes */
	BHDM_P_ConfigureInputAudioFmt(hHDMI, pstAudioInfoFrame) ;

	BHDM_CHECK_RC(rc, BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes)) ;

	/* update current device settings with new information on AudioInfoFrame (if external) */
		BKNI_Memcpy(&hHDMI->DeviceSettings.stAudioInfoFrame, pstAudioInfoFrame,
			sizeof(BAVC_HDMI_AudioInfoFrame)) ;

#if BHDM_CONFIG_DEBUG_AUDIO_INFOFRAME
	BDBG_LOG(("------------------- NEW  AUDIO INFOFRAME ------------------")) ;
	BDBG_LOG(("Tx%d: Packet Type: 0x%02x  Version %d  Length: %d", hHDMI->eCoreId,
		PacketType, PacketVersion, PacketLength)) ;
	BDBG_LOG(("Tx%d: Checksum            %#02x", hHDMI->eCoreId,
		hHDMI->PacketBytes[0])) ;

	BDBG_LOG(("Tx%d: Audio Coding Type     %s", hHDMI->eCoreId,
		BAVC_HDMI_AudioInfoFrame_CodingTypeToStr(pstAudioInfoFrame->CodingType))) ;

	BDBG_LOG(("Tx%d: Audio Channel Count   %s", hHDMI->eCoreId,
		BAVC_HDMI_AudioInfoFrame_ChannelCountToStr(pstAudioInfoFrame->ChannelCount))) ;

	BDBG_LOG(("Tx%d: Sampling Frequency    %s", hHDMI->eCoreId,
		BAVC_HDMI_AudioInfoFrame_SampleFrequencyToStr(pstAudioInfoFrame->SampleFrequency))) ;

	BDBG_LOG(("Tx%d: Sample Size           %s", hHDMI->eCoreId,
		BAVC_HDMI_AudioInfoFrame_SampleSizeToStr(pstAudioInfoFrame->SampleSize))) ;

	BDBG_LOG(("Tx%d: Speaker Allocation    %02x", hHDMI->eCoreId,
		pstAudioInfoFrame->SpeakerAllocation)) ;

	BDBG_LOG(("Tx%d: Level Shift           %s", hHDMI->eCoreId,
		BAVC_HDMI_AudioInfoFrame_LevelShiftToStr(pstAudioInfoFrame->LevelShift))) ;

	BDBG_LOG(("Tx%d: Down-mix Inhibit Flag %s", hHDMI->eCoreId,
		BAVC_HDMI_AudioInfoFrame_DownMixInhibitToStr(pstAudioInfoFrame->DownMixInhibit))) ;

	{
		uint8_t i ;

		for (i = 1 ; i <= 10 ; i++)
		{
			BDBG_LOG(("Tx%d: Data Byte %02d = %#02x h", hHDMI->eCoreId,
				i, hHDMI->PacketBytes[i])) ;
		}
	}
	BDBG_LOG(("-------------------- END AUDIO INFOFRAME --------------------")) ;
#endif

done:
	return rc ;
}



/******************************************************************************
Summary:
Get the Audio Information Info frame sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_GetAudioInfoFramePacket(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BAVC_HDMI_AudioInfoFrame *stAudioInfoFrame
)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(stAudioInfoFrame, 0, sizeof(BAVC_HDMI_AudioInfoFrame)) ;

	BKNI_Memcpy(stAudioInfoFrame, &(hHDMI->DeviceSettings.stAudioInfoFrame),
		sizeof(BAVC_HDMI_AudioInfoFrame)) ;

	return rc ;
}



/******************************************************************************
Summary:
Get the length of  a string
*******************************************************************************/
static int _strlen(const unsigned char *s) {
	int i=0;
	while (*s++) i++;
	return i;
}



/******************************************************************************
Summary:
Set/Enable the Source Product Description Info Frame packet to be sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_SetSPDInfoFramePacket(
   const BHDM_Handle hHDMI		  /* [in] HDMI handle */
)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	BHDM_Packet PhysicalHdmiRamPacketId ;

	uint8_t PacketType ;
	uint8_t PacketVersion ;
	uint8_t PacketLength ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* check length of vendor and description */
	PacketLength = _strlen(hHDMI->DeviceSettings.SpdVendorName) ;
	if (PacketLength > BAVC_HDMI_SPD_IF_VENDOR_LEN)
	{
		BDBG_ERR(("Tx%d: SPD Vendor Name Length %d larger than MAX: %d",
			hHDMI->eCoreId, PacketLength, BAVC_HDMI_SPD_IF_VENDOR_LEN)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	PacketLength = _strlen(hHDMI->DeviceSettings.SpdDescription) ;
	if (PacketLength > BAVC_HDMI_SPD_IF_DESC_LEN)
	{
		BDBG_ERR(("Tx%d: SPD Description Length %d larger than MAX: %d",
			hHDMI->eCoreId, PacketLength, BAVC_HDMI_SPD_IF_DESC_LEN)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}


	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	PhysicalHdmiRamPacketId = BHDM_PACKET_eSPD_ID ;

	PacketType	  = BHDM_Packet_SPDFrameType	;
	PacketVersion = BHDM_Packet_SPDFrameVersion ;
	PacketLength  = BHDM_Packet_SPDFrameLength	;

	BKNI_Memcpy(hHDMI->PacketBytes+1, hHDMI->DeviceSettings.SpdVendorName,
		BAVC_HDMI_SPD_IF_VENDOR_LEN) ;
	BKNI_Memcpy(hHDMI->PacketBytes+9, hHDMI->DeviceSettings.SpdDescription,
		BAVC_HDMI_SPD_IF_DESC_LEN) ;
	hHDMI->PacketBytes[25] = hHDMI->DeviceSettings.eSpdSourceDevice ;


	BHDM_CHECK_RC(rc, BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes)) ;

#if BHDM_CONFIG_DEBUG_SPD_INFOFRAME
	BDBG_LOG(("-------------------- NEW  SPD INFOFRAME -------------------")) ;
	BDBG_LOG(("Tx%d: Packet Type: 0x%02x  Version %d  Length: %d",
		hHDMI->eCoreId, PacketType, PacketVersion, PacketLength)) ;

	BDBG_LOG(("Tx%d: Vendor Name: %s", hHDMI->eCoreId,
		hHDMI->DeviceSettings.SpdVendorName)) ;

	BDBG_LOG(("Tx%d: Vendor Description: %s", hHDMI->eCoreId,
		hHDMI->DeviceSettings.SpdDescription)) ;

	BDBG_LOG(("Tx%d: Device Type: %s", hHDMI->eCoreId,
		BAVC_HDMI_SpdInfoFrame_SourceTypeToStr(
			hHDMI->DeviceSettings.eSpdSourceDevice))) ;
	BDBG_LOG(("--------------------- END SPD INFOFRAME ---------------------")) ;
#endif

done:
	return rc ;
}


/******************************************************************************
Summary:
	Get currently transmitted Vendor Specific Info Frame
*******************************************************************************/
void  BHDM_GetVendorSpecificInfoFrame(
	const BHDM_Handle hHDMI,			/* [in] HDMI handle */
	BAVC_HDMI_VendorSpecificInfoFrame *pVendorSpecficInfoFrame)
{
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(pVendorSpecficInfoFrame,
		0, sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;

	BKNI_Memcpy(pVendorSpecficInfoFrame, &(hHDMI->DeviceSettings.stVendorSpecificInfoFrame),
		sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;
}

/******************************************************************************
Summary:
	Create/Set a Vendor Specific Info Frame
*******************************************************************************/
BERR_Code BHDM_SetVendorSpecificInfoFrame(
	const BHDM_Handle hHDMI,			/* [in] HDMI handle */
	const BAVC_HDMI_VendorSpecificInfoFrame *pVendorSpecificInfoFrame)
{
	BERR_Code	   rc = BERR_SUCCESS ;
	uint8_t packetByte5 ;

	BHDM_Packet PhysicalHdmiRamPacketId = BHDM_Packet_eVendorSpecific_ID ;
	BAVC_HDMI_VSInfoFrame_HDMIVideoFormat eHdmiVideoFormat ;
	BAVC_HDMI_VSInfoFrame_HDMIVIC eHdmiVic ;

	BAVC_HDMI_VendorSpecificInfoFrame NewVSI ;

	uint8_t PacketType = BAVC_HDMI_PacketType_eVendorSpecificInfoframe ;
	uint8_t PacketVersion = 0x01 ;
	uint8_t PacketLength = 0 ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_eReserved ;

	/* BHDM_SetVendorSpecificInfoFrame packet also called from BHDM_EnableDisplay
	    using the VSI stored in the HDMI handle
	*/

	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	BKNI_Memcpy(&NewVSI, pVendorSpecificInfoFrame,
		sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;


	/* ready the Packet Data structure used for transmissIon of the Vendor Specific Data */
	/* Copy the IEEE Reg ID; skip over the checksum byte */
	BKNI_Memcpy(hHDMI->PacketBytes + 1, NewVSI.uIEEE_RegId,
		BAVC_HDMI_IEEE_REGID_LEN) ;
	PacketLength = BAVC_HDMI_IEEE_REGID_LEN;

	/* Set the HDMI Video Format */
	if (pVendorSpecificInfoFrame->eHdmiVideoFormat == BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat)
	{
		eHdmiVideoFormat =
			BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat ;
	}
	else
	{
		/* check for non-3D HDMI Video Format e.g. 4K; use current VideoFmt */
		switch(hHDMI->DeviceSettings.eInputVideoFmt)
		{
#if BHDM_HAS_HDMI_20_SUPPORT
		case BFMT_VideoFmt_e3840x2160p_50Hz :
		case BFMT_VideoFmt_e3840x2160p_60Hz :
			eHdmiVideoFormat = BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone ;
			break ;
#endif

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
		case BFMT_VideoFmt_e3840x2160p_30Hz :
			eHdmiVideoFormat =
				BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution ;
			eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_2997_30Hz ;
			break ;

		case BFMT_VideoFmt_e3840x2160p_25Hz :
			eHdmiVideoFormat =
				BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution ;
			eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_25Hz ;

			break ;

		case BFMT_VideoFmt_e3840x2160p_24Hz :
			eHdmiVideoFormat =
				BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution ;
			eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_2398_24Hz ;

			break ;

		case BFMT_VideoFmt_e4096x2160p_24Hz :
			eHdmiVideoFormat =
				BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution ;
			eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_e4Kx2K_SMPTE_24Hz ;

			break ;
#endif

		default :
			eHdmiVideoFormat = BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone ;
			eHdmiVic = BAVC_HDMI_VSInfoFrame_HDMIVIC_eReserved ;
			break ;
		}
	}

	hHDMI->PacketBytes[4] = eHdmiVideoFormat << 5 ;
	PacketLength++ ;


	NewVSI.eHdmiVic = eHdmiVic ;
	NewVSI.eHdmiVideoFormat = eHdmiVideoFormat ;
	/* Set the Extended Resolution or 3D Format */
	switch (eHdmiVideoFormat)
	{
	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone :
	default :
		BDBG_MSG(("Tx%d: HDMI Normal Resolution", hHDMI->eCoreId)) ;
		goto done;

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution :
		BDBG_MSG(("Tx%d: HDMI Extended Resolution", hHDMI->eCoreId )) ;
		packetByte5 = eHdmiVic ;
		break ;
#endif

	case BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat :
		BDBG_MSG(("Tx%d: HDMI 3D Format", hHDMI->eCoreId)) ;
		packetByte5 = NewVSI.e3DStructure << 4 ;
		break ;
	}

	hHDMI->PacketBytes[5] = packetByte5 ;
	PacketLength++ ;

	/* if 3D_Structure = sidebysideHalf, 3D_Ext_Data is added to HDMI VSI for additional info */
	if ((NewVSI.eHdmiVideoFormat == BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat)
	&& (NewVSI.e3DStructure == BAVC_HDMI_VSInfoFrame_3DStructure_eSidexSideHalf))
	{
		hHDMI->PacketBytes[6] = NewVSI.e3DExtData << 4;
		PacketLength++;
	}


done:

#if BHDM_CONFIG_DEBUG_VENDOR_SPECIFIC_INFOFRAME

	BDBG_LOG(("-------------------- NEW  VS  INFOFRAME -------------------")) ;
	BDBG_LOG(("Tx%d: Packet Type: 0x%02x  Version %d  Length: %d",
		hHDMI->eCoreId, PacketType, PacketVersion, PacketLength)) ;

	BDBG_LOG(("Tx%d: IEEE Registration ID 0x%02x%02x%02x", hHDMI->eCoreId,
		NewVSI.uIEEE_RegId[2], NewVSI.uIEEE_RegId[1], NewVSI.uIEEE_RegId[0])) ;


	BDBG_LOG(("Tx%d: HDMI VideoFormat: %s   (PB4: %#x)", hHDMI->eCoreId,
		BAVC_HDMI_VsInfoFrame_HdmiVideoFormatToStr(NewVSI.eHdmiVideoFormat),
		NewVSI.eHdmiVideoFormat)) ;

  	if (NewVSI.eHdmiVideoFormat == BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eExtendedResolution)
  	{
		BDBG_LOG(("Tx%d: HDMI_VIC: 0x%d  %s (PB5: %#x)", hHDMI->eCoreId,
			NewVSI.eHdmiVic,
			BAVC_HDMI_VsInfoFrame_HdmiVicToStr(NewVSI.eHdmiVic),
			NewVSI.eHdmiVic));
  	}
	else if (NewVSI.eHdmiVideoFormat == BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_e3DFormat)
	{
		BDBG_LOG(("Tx%d: HDMI 2D/3D Structure: %s (PB5: %#x)", hHDMI->eCoreId,
			BAVC_HDMI_VsInfoFrame_3DStructureToStr(NewVSI.e3DStructure),
			NewVSI.e3DStructure));

		if (NewVSI.e3DStructure == BAVC_HDMI_VSInfoFrame_3DStructure_eSidexSideHalf)
		{
			BDBG_LOG(("Tx%d: HDMI 3D_Ext_Data: %s (PB6: %#x)", hHDMI->eCoreId,
				BAVC_HDMI_VsInfoFrame_3DExtDataToStr(NewVSI.e3DExtData),
				NewVSI.e3DExtData));
		}
  	}
	BDBG_LOG(("--------------------- END VS  INFOFRAME ---------------------")) ;

#endif

	/* update current device settings with new information on VendorSpecificInfoFrame */
	BKNI_Memcpy(&hHDMI->DeviceSettings.stVendorSpecificInfoFrame, &NewVSI,
		sizeof(BAVC_HDMI_VendorSpecificInfoFrame)) ;

	BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes) ;

	return rc ;
}


/******************************************************************************
Summary:
	Create/Set a user defined packet
*******************************************************************************/
BERR_Code BHDM_SetUserDefinedPacket(
	const BHDM_Handle hHDMI,			/* [in] HDMI handle */
	BHDM_Packet PhysicalHdmiRamPacketId,

	uint8_t PacketType,
	uint8_t PacketVersion,
	uint8_t PacketLength,

	uint8_t *PacketData
)
{
	BERR_Code	   rc = BERR_SUCCESS ;
	uint8_t i ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (PacketLength > BHDM_NUM_PACKET_BYTES)
	{
		BDBG_ERR(("Tx%d: User Defined Packet Length larger than HDMI Packet Size %d",
			hHDMI->eCoreId, BHDM_NUM_PACKET_BYTES)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	if (PhysicalHdmiRamPacketId < BHDM_Packet_eUser1)
	{
		BDBG_ERR(("Tx%d: Pre-defined Packets cannot be modified; use BHDM_Packet_eUserX",
			hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;

	}

	for (i = 0 ; i < BHDM_Packet_eUser1;  i++)
	{
		if (PacketType == (BHDM_Packet) i)
		{
			BDBG_ERR(("Tx%d: User Packet Type cannot be pre-defined Packet Type: %d",
				hHDMI->eCoreId, i)) ;
			rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
			goto done ;
		}
	}


	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;


	/* copy the user defined data */
	for (i = 0 ; i < PacketLength;	i++)
		hHDMI->PacketBytes[i] = PacketData[i] ;

	BDBG_MSG(("Tx%d: Packet Bytes: %s", hHDMI->eCoreId, hHDMI->PacketBytes)) ;

	BHDM_P_WritePacket(
		hHDMI, PhysicalHdmiRamPacketId,
		PacketType, PacketVersion, PacketLength, hHDMI->PacketBytes) ;

done:
	return rc ;
}


#if BHDM_CONFIG_HDMI_1_3_SUPPORT
/******************************************************************************
Summary:
Set/Enable the Gamut Metadata packet to be sent to the HDMI Rx
*******************************************************************************/
BERR_Code BHDM_P_SetGamutMetadataPacket(
	const BHDM_Handle hHDMI		  /* [in] HDMI handle */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;

	BHDM_Packet PhysicalHdmiRamPacketId ;
	uint32_t PacketRegisterOffset ;
	uint32_t Register, ulOffset;
	uint32_t i;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* initialize packet information to zero */
	BKNI_Memset(hHDMI->PacketBytes, 0, BHDM_NUM_PACKET_BYTES) ;

	PhysicalHdmiRamPacketId = BHDM_PACKET_eGamutMetadata_ID;

	hHDMI->PacketBytes[0] = BHDM_Packet_GamutMetadataType;
	hHDMI->PacketBytes[1] = BHDM_Packet_GamutMetadataHB1;
	hHDMI->PacketBytes[2] = BHDM_Packet_GamutMetadataHB2;

	for (i=0; i<sizeof(BHDM_Packet_GamutMetadataSubPkt); i++) {
		hHDMI->PacketBytes[i+3] = BHDM_Packet_GamutMetadataSubPkt[i];
	}

	/* disable transmission of the RAM Packet */
	BHDM_CHECK_RC(rc, BHDM_DisablePacketTransmission(hHDMI, PhysicalHdmiRamPacketId)) ;

	/* calculate the offset of where the RAM for Packet ID begins */
	PacketRegisterOffset =
		  BCHP_HDMI_RAM_GCP_0
		+ BHDM_P_WORDS_PER_RAM_PACKET * 4 * PhysicalHdmiRamPacketId ;

	/* load the Packet Bytes */
	for (i = 0 ; i < BHDM_NUM_PACKET_BYTES; i = i + 7)
	{
		/* create/write the subpacket data use HDMI_RAM_PACKET_1_0 for Register field names */
		Register =
			  BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_0, hHDMI->PacketBytes[i+0])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_1, hHDMI->PacketBytes[i+1])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_0, HEADER_BYTE_2, hHDMI->PacketBytes[i+2]) ;
		BREG_Write32(hRegister, PacketRegisterOffset + ulOffset, Register) ;

#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
		BDBG_MSG(("Tx%d: Addr [%#x] = %#x", hHDMI->eCoreId, PacketRegisterOffset, Register)) ;
#endif
		PacketRegisterOffset += 4 ;

		/* create/write the subpacket data use HDMI_RAM_PACKET_1_1 for Register field names */
		Register =
			  BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_0, hHDMI->PacketBytes[i+3])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_1, hHDMI->PacketBytes[i+4])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_2, hHDMI->PacketBytes[i+5])
			| BCHP_FIELD_DATA(HDMI_RAM_PACKET_1_1, SUBPACKET_1_BYTE_3, hHDMI->PacketBytes[i+6]) ;
		BREG_Write32(hRegister, PacketRegisterOffset + ulOffset, Register) ;

#if BHDM_CONFIG_DEBUG_HDMI_PACKETS
		BDBG_MSG(("Tx%d: Addr [%#x] = %#x", hHDMI->eCoreId, PacketRegisterOffset, Register)) ;
#endif
		PacketRegisterOffset += 4 ;
	}

	/* reenable the packet transmission */
	BHDM_CHECK_RC(rc, BHDM_EnablePacketTransmission(hHDMI, PhysicalHdmiRamPacketId)) ;

done:
	return rc ;
}

#endif
