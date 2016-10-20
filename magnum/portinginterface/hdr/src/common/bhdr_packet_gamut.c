/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bhdr_priv.h"

BDBG_MODULE(BHDR_PACKET_GAMUT) ;


#if BHDR_CONFIG_GAMUT_PACKET_SUPPORT
BERR_Code BHDR_P_GetGamutPacketData_isr(
	BHDR_Handle hHDR, uint8_t PacketNum, BAVC_HDMI_GamutPacket *Packet )
{
	BREG_Handle hRegister ;
	uint32_t ulOffset ;

	uint32_t Register ;
	uint32_t PacketRegisterOffset ;

	BDBG_ENTER(BHDR_P_GetGamutPacketData_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister= hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	/* calculate the offset of where the Packet RAM begins */
	PacketRegisterOffset = BCHP_HDMI_RX_0_RAM_PACKET_0_HEADER + ulOffset
		+ BHDR_P_PACKET_WORDS * 4 * PacketNum ;

	/* read the Header Bytes (HBn) first */

	Register = BREG_Read32_isr(hRegister, PacketRegisterOffset) ;

	hHDR->GamutPacket.NextField               = ((Register & 0x00008000) >> 15) ;
	hHDR->GamutPacket.Profile                 = ((Register & 0x00007000) >> 12) ;
	hHDR->GamutPacket.AffectedGamutSeqNumber  = ((Register & 0x00000F00) >>  8) ;
	hHDR->GamutPacket.NoCurrentGamut          = ((Register & 0x00800000) >> 23) ;
	hHDR->GamutPacket.PacketSeq               = ((Register & 0x00300000) >> 20) ;
	hHDR->GamutPacket.CurrentGamutSeqNumber   = ((Register & 0x000F0000) >> 16) ;

	/* advance to the next packet register */
	PacketRegisterOffset +=  4 ;

	/* now read the Packet Bytes (PBn) */
	BHDR_P_GetPacketRAM_isr(hRegister, PacketRegisterOffset, (uint8_t *) Packet->DataBytes) ;

#if BDBG_DEBUG_BUILD
	{
		static char *ucProfile[] = {"P0", "P1", "P2", "P3"} ;
		static char *ucPacketSequence[] =
			{"Intermediate", "First", "Last", "Only"} ;

		BDBG_MSG(("Next Field:           %d", hHDR->GamutPacket.NextField)) ;
		BDBG_MSG(("No Current GBD:       %d", hHDR->GamutPacket.NoCurrentGamut)) ;
		BDBG_MSG(("GBD Profile:          %s", ucProfile[hHDR->GamutPacket.Profile])) ;
		BDBG_MSG(("GBD Affected Gamut Sequence Number:    %d",
			hHDR->GamutPacket.AffectedGamutSeqNumber)) ;
		BDBG_MSG(("GBD Current Gamut Sequence Number:     %d",
			hHDR->GamutPacket.CurrentGamutSeqNumber)) ;
		BDBG_MSG(("GBD Packet Sequence:  %s Packet",
			ucPacketSequence[hHDR->GamutPacket.PacketSeq])) ;

	}
#endif

	BDBG_LEAVE(BHDR_P_GetGamutPacketData_isr) ;
	return BERR_SUCCESS ;
}


BERR_Code BHDR_P_ParseGamutMetadataPacket_isr(
	BHDR_Handle hHDR, BAVC_HDMI_GamutPacket *Packet)
{
	uint8_t temp ;
	uint8_t index ;
	uint8_t i ;

	BDBG_ENTER(BHDR_P_ParseGamutMetadataPacket_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* First, store original contents in the local HDMI structure */
	BKNI_Memcpy_isr(&hHDR->GamutPacket, Packet, sizeof(BAVC_HDMI_Packet));


	/* Format Flag */
	temp = Packet->DataBytes[0] ;
	temp = temp & 0x80 ;
	if (temp)
		hHDR->GamutPacket.Format = BAVC_HDMI_GamutFormat_eRange ;
	else
		hHDR->GamutPacket.Format = BAVC_HDMI_GamutFormat_eVerticesFacets ;

	/* facet mode */
	temp = Packet->DataBytes[0] ;
	hHDR->GamutPacket.bFacets = (bool) (temp & 0x40) ;

	/* color precision */
	temp = Packet->DataBytes[0] & 0x18 ;
	temp = temp >> 3 ;
	hHDR->GamutPacket.eColorPrecision = (BAVC_HDMI_GamutColorPrecision) temp ;

	/* color space */
	temp = Packet->DataBytes[0] & 0x07 ;

	hHDR->GamutPacket.eColorSpace = (BAVC_HDMI_GamutColorspace) temp ;

#if BHDR_CONFIG_DEBUG_PACKET_GAMUT
	{
		static char *ucPacketFormat[] = {"Vertices/Facets", "Range Data"} ;
		static char *ucColorPrecision[] = {"8 Bit", "10 Bit", "12 bit"} ;
		static char *ucColorSpace[] = {
			"ITU-R BT.709 (using RGB)",
			"xvYCC601 (IEC 61966-2-4  SD) (using YCbCr)",
			"xvYCC709 (IEC 61966-2-4  HD) (using YCbCr)",
			"XYZ (see above)" } ;

		BDBG_WRN(("GBD Format: %s %s - %s",
			ucColorPrecision[hHDR->GamutPacket.eColorPrecision],
			ucPacketFormat[hHDR->GamutPacket.Format],
			ucColorSpace[hHDR->GamutPacket.eColorSpace])) ;
	}
#endif


	if (hHDR->GamutPacket.Format == BAVC_HDMI_GamutFormat_eVerticesFacets)
	{
		/* Number_Vertices */
		hHDR->GamutPacket.uiNumberVertices =
			  (uint16_t)  (Packet->DataBytes[1] << 8)
			|(uint16_t)  (Packet->DataBytes[2]) ;

		if (hHDR->GamutPacket.uiNumberVertices != 4)
		{
			BDBG_ERR(("GBD Vertices:        %d invalid; Assuming 4",
				hHDR->GamutPacket.uiNumberVertices)) ;
			hHDR->GamutPacket.uiNumberVertices = 4 ;
		}
#if BHDR_CONFIG_DEBUG_PACKET_GAMUT
		BDBG_LOG(("GBD Vertices:        %d",
			hHDR->GamutPacket.uiNumberVertices )) ;
#endif

		if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e8Bit)
		{
			for (i = 0 ; i < 4 ; i++)  /* for each pixel value black/red/green/blue */
			{
				index = 3 + (3 * i ) ;
				hHDR->GamutPacket.uiY[i]   = Packet->DataBytes[index] ;
				hHDR->GamutPacket.uiCb[i] = Packet->DataBytes[index + 1] ;
				hHDR->GamutPacket.uiCr[i]  = Packet->DataBytes[index + 2] ;
			}
		}
		else if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e10Bit)
		{
			index = 3;

			/* point 0: Black Y, Cb, Cr */
			hHDR->GamutPacket.uiY[0]   =
				  (uint16_t) (Packet->DataBytes[index]	<< 2)
				| (uint16_t) ((Packet->DataBytes[index + 1] & 0xC0) >> 6) ;

			hHDR->GamutPacket.uiCb[0] =
				(uint16_t) ((Packet->DataBytes[index + 1] & 0x3F) << 4)
				| (uint16_t) ((Packet->DataBytes[index + 2] & 0xF0) >> 4) ;

			hHDR->GamutPacket.uiCr[0] =
				(uint16_t) ((Packet->DataBytes[index + 2] & 0x0F) << 6)
				| (uint16_t) ((Packet->DataBytes[index + 3] & 0xFC) >> 2) ;


			/* point 1: Red Y, Cb, Cr	*/
			hHDR->GamutPacket.uiY[1]   =
				  (uint16_t) ((Packet->DataBytes[index + 3] & 0x03) << 8)
				| (uint16_t) (Packet->DataBytes[index + 4]) ;

			hHDR->GamutPacket.uiCb[1] =
				  (uint16_t) (Packet->DataBytes[index+5]  << 2)
				| (uint16_t) ((Packet->DataBytes[index + 6] & 0xC0) >> 6) ;

			hHDR->GamutPacket.uiCr[1] =
				(uint16_t) ((Packet->DataBytes[index + 6] & 0x3F) << 4)
				| (uint16_t) ((Packet->DataBytes[index + 7] & 0xF0) >> 4) ;

			/* point 2: Green Y, Cb, Cr */
			hHDR->GamutPacket.uiY[2] =
				(uint16_t) ((Packet->DataBytes[index + 7] & 0x0F) << 6)
				| (uint16_t) ((Packet->DataBytes[index + 8] & 0xFC) >> 2) ;

			hHDR->GamutPacket.uiCb[2]	=
				  (uint16_t) ((Packet->DataBytes[index + 8] & 0x03) << 8)
				| (uint16_t) (Packet->DataBytes[index + 9]) ;

			hHDR->GamutPacket.uiCr[2] =
				  (uint16_t) (Packet->DataBytes[index+10]  << 2)
				| (uint16_t) ((Packet->DataBytes[index + 11] & 0xC0) >> 6) ;


			/* point 3: Blue Y, Cb, Cr	*/
			hHDR->GamutPacket.uiY[3] =
				(uint16_t) ((Packet->DataBytes[index + 11] & 0x3F) << 4)
				| (uint16_t) ((Packet->DataBytes[index + 12] & 0xF0) >> 4) ;

			hHDR->GamutPacket.uiCb[3] =
				(uint16_t) ((Packet->DataBytes[index + 12] & 0x0F) << 6)
				| (uint16_t) ((Packet->DataBytes[index + 13] & 0xFC) >> 2) ;

			hHDR->GamutPacket.uiCr[3]	=
				  (uint16_t) ((Packet->DataBytes[index + 13] & 0x03) << 8)
				| (uint16_t) (Packet->DataBytes[index + 14]) ;

		}
		else if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e12Bit)
		{
			index = 3;
			for (i = 0 ; i < 4 ; i++)  /* for each pixel value black/red/green/blue */
			{
				hHDR->GamutPacket.uiY[i]   =
					  (uint16_t) (Packet->DataBytes[index]	<< 4)
					| (uint16_t) ((Packet->DataBytes[index+1] & 0xF0) >> 4) ;

				hHDR->GamutPacket.uiCb[i] =
					(uint16_t) ((Packet->DataBytes[index+1] & 0x0F) << 8)
					| (uint16_t) (Packet->DataBytes[index+2]) ;

				hHDR->GamutPacket.uiCr[i]	=
					  (uint16_t) (Packet->DataBytes[index+3]  << 4)
					| (uint16_t) ((Packet->DataBytes[index+4] & 0xF0) >> 4) ;

				hHDR->GamutPacket.uiY[i] =
					(uint16_t) ((Packet->DataBytes[index+4] & 0x0F) << 8)
					| (uint16_t) (Packet->DataBytes[index+5]) ;

				hHDR->GamutPacket.uiCb[i]	=
					  (uint16_t) (Packet->DataBytes[index+6]  << 4)
					| (uint16_t) ((Packet->DataBytes[index+7] & 0xF0) >> 4) ;

				hHDR->GamutPacket.uiCr[i] =
					(uint16_t) ((Packet->DataBytes[index+7] & 0x0F) << 8)
					| (uint16_t) (Packet->DataBytes[index+8]) ;

				index += 9;
			}
		}

#if BHDR_CONFIG_DEBUG_PACKET_GAMUT
		for (i=0; i<4; i++)
		{
			BDBG_LOG(("Point %d Y= %3d	Cb= %3d, Cr= %3d",
				i+1, hHDR->GamutPacket.uiY[i],
				hHDR->GamutPacket.uiCb[i], hHDR->GamutPacket.uiCr[i]))	;
		}
#endif

		/* FACET DATA not supported by HDMI 1.3 */

	}
	else if (hHDR->GamutPacket.Format == BAVC_HDMI_GamutFormat_eRange)
	{
		if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e8Bit)
		{
			/* RED Data */
			hHDR->GamutPacket.uiMinRedData = (uint16_t) Packet->DataBytes[1]  ;
			hHDR->GamutPacket.uiMaxRedData = (uint16_t) Packet->DataBytes[2] ;


			/* GREEN Data */
			hHDR->GamutPacket.uiMinGreenData = (uint16_t) Packet->DataBytes[3] ;
			hHDR->GamutPacket.uiMaxGreenData = (uint16_t) Packet->DataBytes[4] ;


			/* BLUE Data */
			hHDR->GamutPacket.uiMinBlueData = (uint16_t) Packet->DataBytes[5]  ;
			hHDR->GamutPacket.uiMaxBlueData = (uint16_t) Packet->DataBytes[6] ;
		}
		else if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e10Bit)
		{
			/* RED Data */
			hHDR->GamutPacket.uiMinRedData =
				  (uint16_t) (Packet->DataBytes[1]  << 2)
				| (uint16_t) ((Packet->DataBytes[2] & 0xC0) >> 6) ;

			hHDR->GamutPacket.uiMaxRedData =
				  (uint16_t) ((Packet->DataBytes[2] & 0x3F)  << 4)
				| (uint16_t) ((Packet->DataBytes[3] & 0xF0) >> 4) ;


			/* GREEN Data */
			hHDR->GamutPacket.uiMinGreenData =
				  (uint16_t) ((Packet->DataBytes[3] & 0x0F) << 6)
				| (uint16_t) ((Packet->DataBytes[4] & 0xFC) >> 2) ;

			hHDR->GamutPacket.uiMaxGreenData =
				  (uint16_t) ((Packet->DataBytes[4] & 0x03)  << 8)
				| (uint16_t) (Packet->DataBytes[5]) ;


			/* BLUE Data */
			hHDR->GamutPacket.uiMinBlueData =
				  (uint16_t) (Packet->DataBytes[6]  << 2)
				| (uint16_t) ((Packet->DataBytes[7] & 0xC0) >> 6) ;

			hHDR->GamutPacket.uiMaxBlueData =
				  (uint16_t) ((Packet->DataBytes[7] & 0x3F)  << 4)
				| (uint16_t) ((Packet->DataBytes[8] & 0xF0) >> 4) ;

		}
		else if (hHDR->GamutPacket.eColorPrecision == BAVC_HDMI_GamutColorPrecision_e12Bit)
		{
			/* RED Data */
			hHDR->GamutPacket.uiMinRedData =
				  (uint16_t) (Packet->DataBytes[1]  << 4)
				| (uint16_t) ((Packet->DataBytes[2] & 0xF0) >> 4) ;

			hHDR->GamutPacket.uiMaxRedData =
				  (uint16_t) ((Packet->DataBytes[2] & 0x0F)  << 8)
				| (uint16_t) (Packet->DataBytes[3]) ;


			/* GREEN Data */
			hHDR->GamutPacket.uiMinGreenData =
				  (uint16_t) (Packet->DataBytes[4]  << 4)
				| (uint16_t) ((Packet->DataBytes[5] & 0xF0) >> 4) ;

			hHDR->GamutPacket.uiMaxGreenData =
				  (uint16_t) ((Packet->DataBytes[5] & 0x0F)  << 8)
				| (uint16_t) (Packet->DataBytes[6]) ;


			/* BLUE Data */
			hHDR->GamutPacket.uiMinBlueData =
				  (uint16_t) (Packet->DataBytes[7]  << 4)
				| (uint16_t) ((Packet->DataBytes[8] & 0xF0) >> 4) ;

			hHDR->GamutPacket.uiMaxBlueData =
				  (uint16_t) ((Packet->DataBytes[8] & 0x0F)  << 8)
				| (uint16_t) (Packet->DataBytes[9]) ;
		}
		else
		{
			BDBG_ERR(("Unknown Gamut Color Precision Format %d",
				hHDR->GamutPacket.eColorPrecision)) ;
		}

#if BHDR_CONFIG_DEBUG_PACKET_GAMUT
		BDBG_LOG(("RED RANGE:     %d to %d",
			hHDR->GamutPacket.uiMinRedData, hHDR->GamutPacket.uiMaxRedData)) ;
		BDBG_LOG(("GREEN RANGE:   %d to %d",
			hHDR->GamutPacket.uiMinGreenData, hHDR->GamutPacket.uiMaxGreenData)) ;
		BDBG_LOG(("BLUE RANGE:    %d to %d",
			hHDR->GamutPacket.uiMinBlueData, hHDR->GamutPacket.uiMaxBlueData)) ;
#endif
	}


	/* notify users of gamut change */
	BDBG_WRN(("Gamut Change Notification")) ;

	/* call  the callback functions for Format Change notification  */
	if (hHDR->pfGamutChangeCallback)
	{
		hHDR->pfGamutChangeCallback(hHDR->pvGamutChangeParm1,
			hHDR->iGamutChangeParm2, &hHDR->GamutPacket) ;
	}

	BDBG_LEAVE(BHDR_P_ParseGamutMetadataPacket_isr) ;
	return BERR_SUCCESS ;
 }

#else
BERR_Code BHDR_P_GetGamutPacketData_isr(
	BHDR_Handle hHDR, uint8_t PacketNum, BAVC_HDMI_GamutPacket *Packet )
{
	BSTD_UNUSED(hHDR) ;
	BSTD_UNUSED(PacketNum) ;
	BSTD_UNUSED(Packet) ;

	return BERR_SUCCESS ;
}

BERR_Code BHDR_P_ParseGamutMetadataPacket_isr(
	BHDR_Handle hHDR, BAVC_HDMI_GamutPacket *Packet)
{
	BSTD_UNUSED(hHDR) ;
	BSTD_UNUSED(Packet) ;

	return BERR_SUCCESS ;
}
#endif
