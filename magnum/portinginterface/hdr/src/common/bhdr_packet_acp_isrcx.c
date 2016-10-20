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

BDBG_MODULE(BHDR_PACKET_ACP_ISRCX) ;


/******************************************************************************
Summary:
Parse Audio Content Protection  from received packet
*******************************************************************************/
BERR_Code BHDR_P_ParseAudioContentProtection_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{

	uint8_t i ;

	BDBG_ENTER(BHDR_P_ParseAudioContentProtection_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* Firstly don't  parse all fields but store in the local HDMI structure */
	BKNI_Memcpy_isr(&hHDR->AudioContentProtection.stPacket, Packet, sizeof(BAVC_HDMI_Packet));


	hHDR->AudioContentProtection.ACP_Type = Packet->Version;
	for (i =0; i<BAVC_HDMI_PB_LENGTH; i++)
	{
		hHDR->AudioContentProtection.ACP_Type_Dependent[i] = Packet->DataBytes[i] ;
	}

	BDBG_LEAVE(BHDR_P_ParseAudioContentProtection_isr) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
*******************************************************************************/
void BHDR_GetAudioContentProtection(BHDR_Handle hHDR,
	BAVC_HDMI_ACP *AudioContentProtection)
{
	BKNI_Memcpy(AudioContentProtection, &hHDR->AudioContentProtection, sizeof(BAVC_HDMI_ACP)) ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_GetISRCData(BHDR_Handle hHDR,
	BAVC_HDMI_ISRC *pISRC)
{
#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
	BKNI_Memcpy(pISRC, &hHDR->ISRC, sizeof(pISRC)) ;
	return BERR_SUCCESS ;
#else
	BSTD_UNUSED(hHDR) ;
	BSTD_UNUSED(pISRC) ;
	BDBG_WRN(("ISRC Support disabled...")) ;
	return BERR_NOT_SUPPORTED  ;
#endif
}

#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
/******************************************************************************
Summary:
Parse ISRC1 from received packet
*******************************************************************************/
void BHDR_P_ParseISRC1_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{

	uint8_t i ;

	BDBG_ENTER(BHDR_P_ParseISRC1_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* Firstly don't  parse all fields but store in the local HDMI structure */
	BKNI_Memcpy_isr(&hHDR->ISRC.stISRC1_Packet, Packet, sizeof(BAVC_HDMI_Packet));

	hHDR->ISRC.ISRC1_PacketReceived = true  ;

	hHDR->ISRC.ISRC_Cont = Packet->Version & 0x80 ;
	hHDR->ISRC.ISRC_Valid = Packet->Version & 0x40 ;
	hHDR->ISRC.ISRC_Status = Packet->Version & 0x07 ;

	for (i =0; i < BAVC_HDMI_PB_LENGTH; i++)
	{
		hHDR->ISRC.ISRC_UPC_EAN[i] = Packet->DataBytes[i] ;
	}

	/* call callback if there is no ISRC2 packet, otherwise wait for the Recd ISRC2 packet to call the cb */
	if (!hHDR->ISRC.ISRC_Cont)
	{
		/* call  the callback functions for ISRC Change notification  */
		if (hHDR->pfPacketChangeCallback)
		{
			hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
				hHDR->RamPacket.Type, &hHDR->ISRC);
		}
	}

	BDBG_LEAVE(BHDR_P_ParseISRC1_isr) ;
}


/******************************************************************************
Summary:
Parse ISRC2 from received packet
*******************************************************************************/
void BHDR_P_ParseISRC2_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{

	uint8_t i ;

	BDBG_ENTER(BHDR_P_ParseISRC2_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* Store a copy of the packet first before parsing the fields  */
	BKNI_Memcpy_isr(&hHDR->ISRC.stISRC2_Packet, Packet, sizeof(BAVC_HDMI_Packet));


	/* there is no Header data in ISRC2; only packet byte data */

	for (i =0; i < BAVC_HDMI_PB_LENGTH; i++)
	{
		hHDR->ISRC.ISRC_UPC_EAN[BAVC_HDMI_IRSC2_PACKET_OFFSET + i]
			= Packet->DataBytes[i] ;
	}


	/* always call  the packet change callback for ISRC Change notification  when 2nd part of ISRC packet is sent */
	if (hHDR->pfPacketChangeCallback)
	{
		hHDR->pfPacketChangeCallback(hHDR->pvPacketChangeParm1,
			hHDR->RamPacket.Type, &hHDR->ISRC);
	}

	BDBG_LEAVE(BHDR_P_ParseISRC2_isr) ;
}
#endif
