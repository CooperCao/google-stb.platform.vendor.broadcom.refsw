/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
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
void BHDR_P_DEBUG_DumpPacketRam_isr(
   BHDR_Handle hHDR, uint8_t PacketNumber, BAVC_HDMI_Packet *Packet
)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t RegAddr ;
	uint32_t ulOffset ;
	uint8_t i ;

	BDBG_ENTER(BHDR_P_DEBUG_DumpPacketRam_isr) ;
	BDBG_OBJECT_ASSERT(hHDR, BHDR_P_Handle) ;

	hRegister = hHDR->hRegister ;
	ulOffset = hHDR->ulOffset;

	BDBG_MSG(("Packet Register Dump:")) ;
	BDBG_MSG((" ")) ;

	RegAddr = BCHP_HDMI_RX_0_RAM_PACKET_0_HEADER  + ulOffset
		+ PacketNumber * 4 * BHDR_P_PACKET_WORDS  ;
	Register = BREG_Read32_isr(hRegister, RegAddr) ;

	BDBG_MSG(("H [%#x] = 0x%08x | Type: 0x%02x Ver: 0x%02x Len: %2d bytes",
		RegAddr, Register, Packet->Type, Packet->Version, Packet->Length)) ;

	for (i = 0 ; i < BHDR_P_PACKET_WORDS; i++)
	{
		Register = BREG_Read32_isr(hRegister, RegAddr) ;
		BDBG_MSG(("%d [%#x] = 0x%08x",
			i, RegAddr, Register)) ;

		RegAddr = RegAddr + 4 ;
	}
        BSTD_UNUSED(Register);

	/* the following code is for debugging received packets; does not need to be enabled */
#if 0
	/* disable dump for kernel mode */
#if BHDR_SPECIAL_DEBUG
	BDBG_LOG(("")) ;
	BDBG_LOG(("Packet Data:")) ;

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

        BSTD_UNUSED(Packet);
	BDBG_LEAVE(BHDR_P_DEBUG_DumpPacketRam_isr) ;
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
