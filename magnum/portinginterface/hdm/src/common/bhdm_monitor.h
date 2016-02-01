/***************************************************************************
*     Copyright (c) 2003-2013, Broadcom Corporation
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
#ifndef BHDM_MONITOR_H__
#define BHDM_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BHDM_MONITOR_Status
{
	/* */
	bool RxSense ;

	/* TMDS Signals enabled */
	bool EnabledTMDS_Clock  ;
	bool EnabledTMDS_CH2 ;
	bool EnabledTMDS_CH1 ;
	bool EnabledTMDS_CH0 ;

	uint32_t NumHotPlugChanges ;
	uint32_t NumRxSenseChanges ;
	uint32_t TotalHotPlugChanges ;  /* total since device opened */
	uint32_t TotalRxSenseChanges ;  /* total since device opened */

	/* total count of times an unstable format */
	/* into the HDMI Tx core was detected */
	/* count is reset after each format change */
	uint32_t UnstableFormatDetectedCounter ;

	bool TxHotPlugInterruptDisabled ;

} BHDM_MONITOR_Status ;

void BHDM_MONITOR_P_FormatChanges_isr(BHDM_Handle hHDMI)  ;
void BHDM_MONITOR_P_StatusChanges_isr(BHDM_Handle hHDMI) ;
void BHDM_MONITOR_P_HpdChanges_isr(BHDM_Handle hHDMI) ;
void BHDM_MONITOR_P_HotplugChanges_isr(BHDM_Handle hHDMI) ;

void BHDM_MONITOR_P_StartTimers(BHDM_Handle hHDMI) ;
void BHDM_MONITOR_P_StopTimers_isr(BHDM_Handle hHDMI)  ;

BERR_Code BHDM_MONITOR_GetHwStatusTx(BHDM_Handle hHDMI, BHDM_MONITOR_Status * status) ;



#ifdef __cplusplus
}
#endif



#endif /* BHDM_MONITOR_H__ */
