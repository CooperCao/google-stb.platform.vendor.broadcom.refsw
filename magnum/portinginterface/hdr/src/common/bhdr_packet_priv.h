/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
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
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
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

BERR_Code BHDR_P_ParseAudioContentProtection_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

BERR_Code BHDR_P_ParseAudioClockRegeneration_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;	

	
BERR_Code BHDR_P_GetGamutPacketData_isr(
	BHDR_Handle hHDR, uint8_t PacketNum, BAVC_HDMI_GamutPacket *Packet ) ;

BERR_Code BHDR_P_ParseGamutMetadataPacket_isr(
	BHDR_Handle hHDR, BAVC_HDMI_GamutPacket *Packet) ;

BERR_Code BHDR_P_ParseISRC1_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

BERR_Code BHDR_P_ParseISRC2_isr(
	BHDR_Handle hHDR, BAVC_HDMI_Packet *Packet) ;

BERR_Code BHDR_P_ProcessStoppedPackets_isr(BHDR_Handle hHDR) ;

#ifdef __cplusplus
}
#endif
 
#endif
/* end bhdr_packet_priv.h */

