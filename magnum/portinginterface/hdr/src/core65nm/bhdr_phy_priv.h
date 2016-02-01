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

#ifndef BHDR_PHY_PRIV_H__
#define BHDR_PHY_PRIV_H__

#if BHDR_CONFIG_SYMBOL_LOSS_SM
void BHDR_P_HdmiRxCoreReset_isr(BHDR_Handle hHDR) ;
void BHDR_P_ResetFrontEndAnalogChannels_isr(BHDR_Handle hHDR) ;

const char * BHDR_P_SymbolLossSM_StateToStr(uint8_t state ) ;
void	BHDR_P_SymbolLossSM_Initialize(BHDR_Handle hHDR) ;
void BHDR_P_SymbolLossSM_HdcpInit_isr(BHDR_Handle hHDR) ;
void BHDR_P_SymbolLossSM_ConfigureAfterClockChange_isr(BHDR_Handle hHDR, bool bClockStatus) ;
void BHDR_P_SymbolLossSM_isr(BHDR_Handle hHDR)  ;
#endif

void BHDR_P_VerifyHdcpMasterSlave_isr(BHDR_Handle hHDR) ;


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif
 
#endif
/* end bhdr_packet_priv.h */


