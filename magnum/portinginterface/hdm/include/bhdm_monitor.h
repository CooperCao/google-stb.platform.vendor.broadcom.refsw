/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
******************************************************************************/
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

	unsigned MonitoredHpdChanges ; /* HP changes within BHDM_MONITOR_HP_CHANGE_SECONDS */
	unsigned NumRxSenseChanges ;   /* RxSense changes within BHDM_CONFIG_MONITOR_STATUS_SECONDS */
	unsigned TotalHotPlugChanges ; /* total since device opened */
	unsigned TotalRxSenseChanges ; /* total since device opened */

    struct
    {
		unsigned BCapsReadFailures ;
		unsigned BksvReadFailures ;
		unsigned InvalidBksvFailures ;
    } hdcp1x ;

	/* total count of times an unstable format */
	/* into the HDMI Tx core was detected */
	/* count is reset after each format change */
	uint32_t UnstableFormatDetectedCounter ;

	bool TxHotPlugInterruptDisabled ;

} BHDM_MONITOR_Status ;

void BHDM_MONITOR_P_FormatChanges_isr(BHDM_Handle hHDMI)  ;
void BHDM_MONITOR_P_StatusChanges_isr(BHDM_Handle hHDMI) ;
void BHDM_MONITOR_P_HpdChanges_isr(BHDM_Handle hHDMI) ;
void BHDM_MONITOR_P_ResetHpdChanges_isr(BHDM_Handle hHDMI) ;

void BHDM_MONITOR_P_StartTimers(BHDM_Handle hHDMI) ;
void BHDM_MONITOR_P_StopTimers_isr(BHDM_Handle hHDMI)  ;

BERR_Code BHDM_MONITOR_GetHwStatusTx(BHDM_Handle hHDMI, BHDM_MONITOR_Status * status) ;



#ifdef __cplusplus
}
#endif



#endif /* BHDM_MONITOR_H__ */
