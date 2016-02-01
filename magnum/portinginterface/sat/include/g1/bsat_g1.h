/******************************************************************************
*    (c)2011-2013 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
*****************************************************************************/
#ifndef _BSAT_G1_H_
#define _BSAT_G1_H_

#include "bsat.h"

#define BSAT_G1_MAJOR_VERSION 2
#define BSAT_G1_MINOR_VERSION 0


/* define device configuration parameters */
/* TBD... */

/* define channel configuration parameters */
enum {
   BSAT_g1_CONFIG_PLC_CTL = 0,
   BSAT_g1_CONFIG_PLC_ALT_ACQ_BW,
   BSAT_g1_CONFIG_PLC_ALT_ACQ_DAMP,
   BSAT_g1_CONFIG_PLC_ALT_TRK_BW,
   BSAT_g1_CONFIG_PLC_ALT_TRK_DAMP,
   BSAT_g1_CONFIG_DFT_STATUS,
   BSAT_g1_CONFIG_DFT_FREQ_ESTIMATE,
   BSAT_g1_CONFIG_ACQ_TIME,
   BSAT_g1_CONFIG_HP_CTL,
   BSAT_g1_CONFIG_VLC_GAIN,
   BSAT_g1_CONFIG_EQ_CTL,
   BSAT_g1_CONFIG_ACQ_DAFE_CTL,
   BSAT_g1_CONFIG_TRK_DAFE_CTL,
   BSAT_g1_CONFIG_MAX
};

/* bit definitions for BSAT_g1_CONFIG_ACQ_DAFE_CTL/BSAT_g1_CONFIG_TRK_DAFE_CTL */
#define BSAT_g1_CONFIG_DAFE_CTL_CLDAFECTL          0x0007FFFE /* CLDAFECTL[18:1] */

/* bit definitions for BSAT_g1_CONFIG_EQ_CTL */
#define BSAT_g1_CONFIG_EQ_CTL_OVERRIDE_MU          0x00000001 /* 1=override mu */
#define BSAT_g1_CONFIG_EQ_CTL_OVERRIDE_F0B         0x00000002 /* 1=override main tap initial value */
#define BSAT_g1_CONFIG_EQ_CTL_MAIN_TAP_F0B_MASK    0x0000FF00 /* main tap F0B[31:24], applies when bit 1 is set */
#define BSAT_g1_CONFIG_EQ_CTL_ACQ_FFE_MU_MASK      0x000F0000 /* acquisition ffe_mu, applies when bit 0 is set */
#define BSAT_g1_CONFIG_EQ_CTL_ACQ_FFE_MAIN_MU_MASK 0x00F00000 /* acquisition ffe_main_mu, applies when bit 0 is set */
#define BSAT_g1_CONFIG_EQ_CTL_TRK_FFE_MU_MASK      0x0F000000 /* tracking ffe_mu, applies when bit 0 is set */
#define BSAT_g1_CONFIG_EQ_CTL_TRK_FFE_MAIN_MU_MASK 0xF0000000 /* tracking ffe_main_mu, applies when bit 0 is set */

/* bit definitions for BSAT_g1_CONFIG_PLC_CTL */
#define BSAT_g1_CONFIG_PLC_CTL_AWGN         0x00000001
#define BSAT_g1_CONFIG_PLC_CTL_OVERRIDE_ACQ 0x00000002
#define BSAT_g1_CONFIG_PLC_CTL_OVERRIDE_TRK 0x00000004

/* bit definitions for BSAT_g1_CONFIG_DFT_FREQ_EST_STATUS */
#define BSAT_g1_CONFIG_DFT_STATUS_START     0x01000000 /* DFT freq estimate started */
#define BSAT_g1_CONFIG_DFT_STATUS_COARSE    0x02000000 /* DFT freq estimate coarse search started */
#define BSAT_g1_CONFIG_DFT_STATUS_FINE      0x04000000 /* DFT freq estimate fine search started */
#define BSAT_g1_CONFIG_DFT_STATUS_OQPSK     0x08000000 /* DFT freq estimate for OQPSK */
#define BSAT_g1_CONFIG_DFT_STATUS_PEAK_SCAN 0x10000000 /* peak scan operation */
#define BSAT_g1_CONFIG_DFT_STATUS_DONE      0x80000000 /* DFT freq estimate finished */
#define BSAT_g1_CONFIG_DFT_STATUS_STATE     0x000000FF

/* bit definitions for BSAT_g1_CONFIG_HP_CTL */
#define BSAT_g1_CONFIG_HP_CTL_OVERRIDE_DAFE_AVERAGE 0x80000000
#define BSAT_g1_CONFIG_HP_CTL_DAFE_AVERAGE_MASK     0x3F

/* channel interrupt ID enumeration */
typedef enum BSAT_g1_IntID{
   BSAT_g1_IntID_eSdsLock = 0,
   BSAT_g1_IntID_eSdsNotLock,
   BSAT_g1_IntID_eBaudTimer,
   BSAT_g1_IntID_eBerTimer,
   BSAT_g1_IntID_eGenTimer1,
   BSAT_g1_IntID_eGenTimer2,
   BSAT_g1_IntID_eGenTimer3,
   BSAT_g1_IntID_eMi2c,
   BSAT_g1_IntID_eHpFrameBoundary,
   BSAT_g1_IntID_eHpStateMatch,
   BSAT_g1_IntID_eHpStateChange,
   BSAT_g1_IntID_eDftDone,
   BSAT_g1_IntID_eAfecLock,
   BSAT_g1_IntID_eAfecNotLock,
   BSAT_g1_IntID_eAfecMpLock,
   BSAT_g1_IntID_eAfecMpNotLock,
   BSAT_g1_IntID_eTfecLock,
   BSAT_g1_IntID_eTfecNotLock,
   BSAT_g1_IntID_eTfecSync,
   BSAT_g1_MaxIntID
} BSAT_g1_IntID;


/* BSAT G1 public functions */
BERR_Code BSAT_g1_GetDefaultSettings(BSAT_Settings*);
BERR_Code BSAT_g1_GetChannelDefaultSettings(BSAT_Handle, uint32_t, BSAT_ChannelSettings *);
void BSAT_g1_ResetInterruptCounters(BSAT_ChannelHandle h);
BERR_Code BSAT_g1_GetInterruptCount(BSAT_ChannelHandle h, BSAT_g1_IntID idx, uint32_t *pCount);


#ifdef BSAT_EXCLUDE_API_TABLE
#include "bsat_g1_priv.h"
#define BSAT_Open BSAT_g1_P_Open
#define BSAT_Close BSAT_g1_P_Close
#define BSAT_GetTotalChannels BSAT_g1_P_GetTotalChannels
#define BSAT_GetChannelDefaultSettings BSAT_g1_GetChannelDefaultSettings
#define BSAT_OpenChannel BSAT_g1_P_OpenChannel
#define BSAT_CloseChannel BSAT_g1_P_CloseChannel
#define BSAT_GetDevice BSAT_g1_P_GetDevice
#define BSAT_GetVersionInfo BSAT_g1_P_GetVersionInfo
#define BSAT_Reset BSAT_g1_P_Reset
#define BSAT_PowerDownChannel BSAT_g1_P_PowerDownChannel
#define BSAT_PowerUpChannel BSAT_g1_P_PowerUpChannel
#define BSAT_IsChannelOn BSAT_g1_P_IsChannelOn
#define BSAT_Acquire BSAT_g1_P_Acquire
#define BSAT_GetLockStatus BSAT_g1_P_GetLockStatus
#define BSAT_GetChannelStatus BSAT_g1_P_GetChannelStatus
#define BSAT_ResetChannelStatus BSAT_g1_P_ResetChannelStatus
#define BSAT_GetSoftDecisions BSAT_g1_P_GetSoftDecisions
#define BSAT_ResetChannel BSAT_g1_P_ResetChannel
#define BSAT_SetBertSettings BSAT_g1_P_SetBertSettings
#define BSAT_GetBertSettings BSAT_g1_P_GetBertSettings
#define BSAT_GetBertStatus BSAT_g1_P_GetBertStatus
#define BSAT_SetSearchRange BSAT_g1_P_SetSearchRange
#define BSAT_GetSearchRange BSAT_g1_P_GetSearchRange
#define BSAT_SetAmcScramblingSeq BSAT_g1_P_SetAmcScramblingSeq
#define BSAT_SetNetworkSpec BSAT_g1_P_SetNetworkSpec
#define BSAT_GetNetworkSpec BSAT_g1_P_GetNetworkSpec_isrsafe
#define BSAT_SetOutputTransportSettings BSAT_g1_P_SetOutputTransportSettings
#define BSAT_GetOutputTransportSettings BSAT_g1_P_GetOutputTransportSettings
#define BSAT_GetInitDoneEventHandle BSAT_g1_P_GetInitDoneEventHandle
#define BSAT_GetLockStateChangeEventHandle BSAT_g1_P_GetLockStateChangeEventHandle
#define BSAT_GetAcqDoneEventHandle BSAT_g1_P_GetAcqDoneEventHandle
#define BSAT_GetSignalNotificationEventHandle BSAT_g1_P_GetSignalNotificationEventHandle
#define BSAT_GetReadyEventHandle BSAT_g1_P_GetReadyEventHandle
#define BSAT_SetAcqDoneEventSettings BSAT_g1_P_SetAcqDoneEventSettings
#define BSAT_SetSignalNotificationSettings BSAT_g1_P_SetSignalNotificationSettings
#define BSAT_GetSignalNotificationSettings BSAT_g1_P_GetSignalNotificationSettings
#define BSAT_StartToneDetect BSAT_g1_P_StartToneDetect
#define BSAT_GetToneDetectStatus BSAT_g1_P_GetToneDetectStatus
#define BSAT_StartSymbolRateScan BSAT_g1_P_StartSymbolRateScan
#define BSAT_GetSymbolRateScanStatus BSAT_g1_P_GetSymbolRateScanStatus
#define BSAT_StartSignalDetect BSAT_g1_P_StartSignalDetect
#define BSAT_GetSignalDetectStatus BSAT_g1_P_GetSignalDetectStatus
#define BSAT_SetConfig BSAT_g1_P_SetConfig
#define BSAT_GetConfig BSAT_g1_P_GetConfig
#define BSAT_SetChannelConfig BSAT_g1_P_SetChannelConfig
#define BSAT_GetChannelConfig BSAT_g1_P_GetChannelConfig
#define BSAT_GetLegacyQpskAcqSettings BSAT_g1_P_GetLegacyQpskAcqSettings
#define BSAT_SetLegacyQpskAcqSettings BSAT_g1_P_SetLegacyQpskAcqSettings
#define BSAT_GetDvbs2AcqSettings BSAT_g1_P_GetDvbs2AcqSettings
#define BSAT_SetDvbs2AcqSettings BSAT_g1_P_SetDvbs2AcqSettings
#define BSAT_GetTurboAcqSettings BSAT_g1_P_GetTurboAcqSettings
#define BSAT_SetTurboAcqSettings BSAT_g1_P_SetTurboAcqSettings
#define BSAT_GetExtAcqSettings BSAT_g1_P_GetExtAcqSettings
#define BSAT_SetExtAcqSettings BSAT_g1_P_SetExtAcqSettings
#define BSAT_SetACIBandwidth BSAT_g1_P_SetACIBandwidth
#define BSAT_SetNotchSettings BSAT_g1_P_SetNotchSettings
#define BSAT_GetNotchSettings BSAT_g1_P_GetNotchSettings
#define BSAT_GetTraceInfo BSAT_g1_P_GetTraceInfo
#define BSAT_SetCwc BSAT_g1_P_SetCwc
#define BSAT_GetDvbs2xAcqSettings BSAT_g1_P_GetDvbs2xAcqSettings
#define BSAT_SetDvbs2xAcqSettings BSAT_g1_P_SetDvbs2xAcqSettings
#define BSAT_StartPsdScan BSAT_g1_P_StartPsdScan
#define BSAT_GetPsdScanStatus BSAT_g1_P_GetPsdScanStatus
#define BSAT_SetAcmSettings BSAT_g1_P_SetAcmSettings
#define BSAT_GetAcmSettings BSAT_g1_P_GetAcmSettings
#define BSAT_GetStreamList BSAT_g1_P_GetStreamList
#define BSAT_GetStreamStatus BSAT_g1_P_GetStreamStatus
#endif /* BSAT_EXCLUDE_API_TABLE */

#endif /* _BSAT_G1_H_ */
