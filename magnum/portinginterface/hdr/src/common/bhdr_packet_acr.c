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


#if BHDR_CONFIG_DEBUG_PACKET_ACR

#define BHDR_P_ACR_PACKET_LOCATION 1

static void BHDR_P_DEBUG_AcrPacket_isr(
	BAVC_HDMI_AudioClockRegenerationPacket *OldACR,
	BAVC_HDMI_AudioClockRegenerationPacket *NewACR)
{
	BSTD_UNUSED(OldACR) ;

	BDBG_MSG(("=== PARSED ACR PACKET ===")) ;
	BDBG_MSG(("ACR:       N= %8d        CTS= %8d", NewACR->N, NewACR->CTS)) ;

	/* use a marker to separate packet data */
	BDBG_MSG(("=== END ACR PACKET ===")) ;
}
#endif


BERR_Code BHDR_P_ParseAudioClockRegeneration_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet)
{

#if BHDR_CONFIG_DEBUG_PACKET_ACR
	BAVC_HDMI_AudioClockRegenerationPacket stNewAudioClockRegenerationPacket ;
#endif
	uint32_t N, CTS ;

	BDBG_ENTER(BHDR_P_ParseAudioClockRegeneration_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	/* Keep a raw copy of the HDMI Packet structure  */
	BKNI_Memcpy_isr(&hHDR->AudioClockRegenerationPacket.stPacket, Packet,
		sizeof(BAVC_HDMI_Packet)) ;

	/* display modified N and CTS values for debug purposes */
	CTS =
		  (uint32_t) ((Packet->DataBytes[1] & 0xF) << 16)
		|(uint32_t) ((Packet->DataBytes[2] << 8))
		|(uint32_t) ((Packet->DataBytes[3])) ;

	N =
		  (uint32_t) ((Packet->DataBytes[4] & 0xF) << 16)
		|(uint32_t) ((Packet->DataBytes[5] << 8))
		|(uint32_t) ((Packet->DataBytes[6])) ;


	/* For audio sample freq, CTS and N from packet are more exact than the value from register. */
	if ((hHDR->AudioClockRegenerationPacket.N != N)
	|| (hHDR->AudioClockRegenerationPacket.CTS > (CTS + 20))
	|| ((hHDR->AudioClockRegenerationPacket.CTS + 20) < CTS))
	{
#if BHDR_CONFIG_DEBUG_PACKET_ACR
		BDBG_MSG(("RAM_PACKET_%02d received '%s' Packet (0x%02x)",
			BHDR_P_ACR_PACKET_LOCATION,
			BAVC_HDMI_PacketTypeToStr_isrsafe(Packet->Type), Packet->Type)) ;

		stNewAudioClockRegenerationPacket.CTS = CTS ;
		stNewAudioClockRegenerationPacket.N = N ;
		/* always report significant changes in audio clock recovery packet */
		BHDR_P_DEBUG_AcrPacket_isr(
			&hHDR->AudioClockRegenerationPacket,
			&stNewAudioClockRegenerationPacket) ;
#endif

		hHDR->AudioClockRegenerationPacket.CTS = CTS ;
		hHDR->AudioClockRegenerationPacket.N = N ;
	}

	BDBG_LEAVE(BHDR_P_ParseAudioClockRegeneration_isr) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
Summary:
*******************************************************************************/
BERR_Code BHDR_GetAudioClockRegenerationData(BHDR_Handle hHDR,
	BAVC_HDMI_AudioClockRegenerationPacket * AudioClockRegeneraionPacket)
{
	BKNI_Memcpy(AudioClockRegeneraionPacket, &hHDR->AudioClockRegenerationPacket, sizeof(BAVC_HDMI_AudioClockRegenerationPacket)) ;
	return BERR_SUCCESS ;
}
