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

#ifndef BHDR_PACKET_PRIV_H__
#define BHDR_PACKET_PRIV_H__


#ifdef __cplusplus
extern "C" {
#endif


#define BHDR_P_ENABLE_PACKET_WRITES(hRegister, Register, ulOffset) \
	/* Enable write access to RAM packets  */ \
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CONTROL + ulOffset) ; \
	Register |= BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_CONTROL, ENABLE_RAM_PACKET_WRITES) ;  \
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CONTROL + ulOffset, Register) ;

#define BHDR_P_DISABLE_PACKET_WRITES(hRegister, Register, ulOffset) \
	/* Disable write access to RAM packets  */ \
	Register = BREG_Read32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CONTROL + ulOffset) ; \
	Register &= ~BCHP_MASK(HDMI_RX_0_PACKET_PROCESSOR_CONTROL, ENABLE_RAM_PACKET_WRITES) ;  \
	BREG_Write32(hRegister, BCHP_HDMI_RX_0_PACKET_PROCESSOR_CONTROL + ulOffset, Register) ;

BERR_Code BHDR_P_InitializePacketRAM_isr(BHDR_Handle hHDR) ;
BERR_Code BHDR_P_ProcessReceivedPackets_isr(BHDR_Handle hHDR) ;


BERR_Code BHDR_P_GetPacketRamIndexByType(
	BHDR_Handle hHDR, BAVC_HDMI_PacketType ePacketType,  uint8_t *PacketIndex) ;

void BHDR_P_GetPacketRAM_isr(
	BREG_Handle hRegister, uint32_t PacketRegisterOffset, uint8_t *DataBytes) ;


void BHDR_P_ClearPacketMemory_isr(BHDR_Handle hHDR) ;

BERR_Code BHDR_P_ClearPacketRAM_isr(
   BHDR_Handle hHDR, uint8_t PacketNumber) ;

BERR_Code BHDR_P_ClearPacketRAMLock_isr(BHDR_Handle hHDR, uint8_t PacketNumber) ;


BERR_Code BHDR_P_ParseGeneralControlPacket_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

BERR_Code BHDR_P_ParseAviInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

BERR_Code BHDR_P_ParseAudioInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

BERR_Code BHDR_P_ParseSPDInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

BERR_Code BHDR_P_ParseVendorSpecificInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

BERR_Code BHDR_P_ParseDrmInfoFrameData_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

BERR_Code BHDR_P_ParseAudioContentProtection_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

BERR_Code BHDR_P_ParseAudioClockRegeneration_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;


BERR_Code BHDR_P_GetGamutPacketData_isr(
	BHDR_Handle hHDR, uint8_t PacketNum, BAVC_HDMI_GamutPacket *Packet ) ;

BERR_Code BHDR_P_ParseGamutMetadataPacket_isr(
	BHDR_Handle hHDR, BAVC_HDMI_GamutPacket *Packet) ;

#if BHDR_CONFIG_ISRC_PACKET_SUPPORT
void BHDR_P_ParseISRC1_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

void BHDR_P_ParseISRC2_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;
#endif

BERR_Code BHDR_P_ProcessStoppedPackets_isr(BHDR_Handle hHDR) ;

#ifdef __cplusplus
}
#endif

#endif
/* end bhdr_packet_priv.h */

