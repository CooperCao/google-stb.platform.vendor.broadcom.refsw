/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 ****************************************************************************/
 
#ifndef BHDR_DEBUG_H__
#define BHDR_DEBUG_H__

 #include "bhdr.h"

typedef struct
 {
 	uint8_t uiCounter ;
	uint32_t ulEventBitMask31_00 ;
	uint32_t ulEventBitMask63_32 ;
	bool bBchEvent  ;
 } BHDR_DEBUG_P_EventCounter ;

void BHDR_P_DebugMonitorHdmiRx_isr (BHDR_Handle hHDR) ;

#if BHDR_CONFIG_DEBUG_INFO_PACKET_AVI
void BHDR_P_DEBUG_AviInfoFrame(
	BAVC_HDMI_AviInfoFrame *OldAvi, BAVC_HDMI_AviInfoFrame *NewAvi) ;
#endif

#if BHDR_CONFIG_DEBUG_INFO_PACKET_AUDIO
void BHDR_P_DEBUG_AudioInfoFrame(
	BAVC_HDMI_AudioInfoFrame *OldAudioInfoFrame, BAVC_HDMI_AudioInfoFrame*NewAudioInfoframe) ;
#endif

#if BHDR_CONFIG_DEBUG_INFO_PACKET_SPD
void BHDR_P_DEBUG_SpdInfoFrame(
	BAVC_HDMI_SPDInfoFrame *OldSpdInfoFrame,  BAVC_HDMI_SPDInfoFrame *NewSpdInfoFrame) ;
#endif

#if BHDR_CONFIG_DEBUG_INFO_PACKET_VENDOR_SPECIFIC
void BHDR_P_DEBUG_VsInfoFrame(
	BAVC_HDMI_VendorSpecificInfoFrame *OldVsInfoFrame,  BAVC_HDMI_VendorSpecificInfoFrame *VsInfoFrame) ;
#endif

#if BHDR_CONFIG_DEBUG_ACR_PACKET
void BHDR_P_DEBUG_AcrPacket(
	BAVC_HDMI_AudioClockRegenerationPacket *OldAcrPacket,  
	BAVC_HDMI_AudioClockRegenerationPacket *NewAcrPacket) ;
#endif

BERR_Code BHDR_P_DEBUG_DumpPacketRam_isr(
	BHDR_Handle hHDR, uint8_t PacketNumber, BAVC_HDMI_Packet *Packet)  ;

#if BHDR_CONFIG_DEBUG_HDCP_VALUES
void BHDR_P_DebugHdcpValues_isr(BHDR_Handle hHDR) ;
#endif 


void BHDR_DEBUG_P_ConfigureEventCounter(
	BHDR_Handle hHDR, BHDR_DEBUG_P_EventCounter  *pEventCounter) ;

void BHDR_DEBUG_P_ResetAllEventCounters_isr(BHDR_Handle hHDR) ;


#endif
