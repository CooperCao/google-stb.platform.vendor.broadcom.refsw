/******************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*****************************************************************************/
#ifndef _BSAT_45402_PRIV_H_
#define _BSAT_45402_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "bsat_45402.h"
#include "bhab.h"

#define BSAT_45402_MAX_CHANNELS 2
#define BSAT_45402_MAX_SYMRATE 52000000

#define BSAT_45402_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BSAT handle
Description:
   This is the chip-specific component of the BSAT_Handle.
See Also:
   None.
****************************************************************************/
typedef struct BSAT_45402_P_Handle
{
   BHAB_Handle      hHab;
   BKNI_EventHandle hInitDoneEvent;
} BSAT_45402_P_Handle;


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BSAT channel handle
Description:
   This is the chip-specific component of the BSAT_ChannelHandle.
See Also:
   None.
****************************************************************************/
typedef struct BSAT_45402_P_ChannelHandle
{
   BKNI_EventHandle hLockChangeEvent;  /* change of lock status event handle */
   BKNI_EventHandle hAcqDoneEvent;     /* acquisition done event handle */
   BKNI_EventHandle hSignalNotificationEvent;  /* signal notification event handle */
   BKNI_EventHandle hReadyEvent;       /* ready event handle */
   BSAT_Mode        lastMode;          /* most recent acquisition mode */
} BSAT_45402_P_ChannelHandle;


/* API implementation functions */
BERR_Code BSAT_45402_P_Open(BSAT_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInterrupt, const BSAT_Settings *pSettings);
BERR_Code BSAT_45402_P_Close(BSAT_Handle h);
BERR_Code BSAT_45402_P_GetTotalChannels(BSAT_Handle h, uint32_t *totalChannels);
BERR_Code BSAT_45402_P_OpenChannel(BSAT_Handle h, BSAT_ChannelHandle *pChannelHandle, uint32_t chan, const BSAT_ChannelSettings *pSettings);
BERR_Code BSAT_45402_P_CloseChannel(BSAT_ChannelHandle h);
BERR_Code BSAT_45402_P_GetDevice(BSAT_ChannelHandle h, BSAT_Handle *pDev);
BERR_Code BSAT_45402_P_GetVersionInfo(BSAT_Handle h, BFEC_VersionInfo *pVersion);
BERR_Code BSAT_45402_P_Reset(BSAT_Handle h);
BERR_Code BSAT_45402_P_PowerDownChannel(BSAT_ChannelHandle h);
BERR_Code BSAT_45402_P_PowerUpChannel(BSAT_ChannelHandle h);
BERR_Code BSAT_45402_P_IsChannelOn(BSAT_ChannelHandle, bool*);
BERR_Code BSAT_45402_P_Acquire(BSAT_ChannelHandle h, BSAT_AcqSettings *pParams);
BERR_Code BSAT_45402_P_GetLockStatus(BSAT_ChannelHandle h, bool *pbLocked, bool *pbAcqDone);
BERR_Code BSAT_45402_P_GetChannelStatus(BSAT_ChannelHandle h, BSAT_ChannelStatus *pStatus);
BERR_Code BSAT_45402_P_ResetChannelStatus(BSAT_ChannelHandle h);
BERR_Code BSAT_45402_P_GetSoftDecisions(BSAT_ChannelHandle h, uint32_t n, int16_t *pI, int16_t *pQ);
BERR_Code BSAT_45402_P_ResetChannel(BSAT_ChannelHandle h, bool bDisableDemod);
BERR_Code BSAT_45402_P_SetBertSettings(BSAT_ChannelHandle h, BSAT_BertSettings *pSettings);
BERR_Code BSAT_45402_P_GetBertSettings(BSAT_ChannelHandle h, BSAT_BertSettings *pSettings);
BERR_Code BSAT_45402_P_GetBertStatus(BSAT_ChannelHandle h, BSAT_BertStatus *pStatus);
BERR_Code BSAT_45402_P_SetSearchRange(BSAT_ChannelHandle h, uint32_t searchRange);
BERR_Code BSAT_45402_P_GetSearchRange(BSAT_ChannelHandle h, uint32_t *pSearchRange);
BERR_Code BSAT_45402_P_SetAmcScramblingSeq(BSAT_ChannelHandle h, BSAT_AmcScramblingSeq *pSeq);
BERR_Code BSAT_45402_P_SetNetworkSpec(BSAT_Handle h, BSAT_NetworkSpec networkSpec);
BERR_Code BSAT_45402_P_GetNetworkSpec(BSAT_Handle h, BSAT_NetworkSpec *pNetworkSpec);
BERR_Code BSAT_45402_P_SetOutputTransportSettings(BSAT_ChannelHandle h, BSAT_OutputTransportSettings *pSettings);
BERR_Code BSAT_45402_P_GetOutputTransportSettings(BSAT_ChannelHandle h, BSAT_OutputTransportSettings *pSettings);
BERR_Code BSAT_45402_P_GetInitDoneEventHandle(BSAT_Handle h, BKNI_EventHandle *hEvent);
BERR_Code BSAT_45402_P_GetLockStateChangeEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BSAT_45402_P_GetAcqDoneEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BSAT_45402_P_GetSignalNotificationEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BSAT_45402_P_GetReadyEventHandle(BSAT_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BSAT_45402_P_StartToneDetect(BSAT_ChannelHandle h, uint32_t freq, uint32_t adcSelect);
BERR_Code BSAT_45402_P_GetToneDetectStatus(BSAT_ChannelHandle h, BSAT_ToneDetectStatus *pStatus);
BERR_Code BSAT_45402_P_StartSymbolRateScan(BSAT_ChannelHandle h, uint32_t freq, uint32_t minSymbolRate, uint32_t maxSymbolRate, uint32_t adcSelect);
BERR_Code BSAT_45402_P_GetSymbolRateScanStatus(BSAT_ChannelHandle h, BSAT_SymbolRateScanStatus *pStatus);
BERR_Code BSAT_45402_P_SetConfig(BSAT_Handle h, uint32_t addr, uint32_t val);
BERR_Code BSAT_45402_P_GetConfig(BSAT_Handle h, uint32_t addr, uint32_t *pVal);
BERR_Code BSAT_45402_P_SetChannelConfig(BSAT_ChannelHandle h, uint32_t addr, uint32_t val);
BERR_Code BSAT_45402_P_GetChannelConfig(BSAT_ChannelHandle h, uint32_t addr, uint32_t *pVal);
BERR_Code BSAT_45402_P_SetAcqDoneEventSettings(BSAT_Handle h, uint8_t reacqs);
BERR_Code BSAT_45402_P_SetSignalNotificationSettings(BSAT_ChannelHandle h, BSAT_SignalNotificationSettings *pSettings);
BERR_Code BSAT_45402_P_GetSignalNotificationSettings(BSAT_ChannelHandle h, BSAT_SignalNotificationSettings *pSettings);
BERR_Code BSAT_45402_P_SetACIBandwidth(BSAT_ChannelHandle h, uint32_t bw);
BERR_Code BSAT_45402_P_StartSignalDetect(BSAT_ChannelHandle h, uint32_t symbolRate, uint32_t freq, uint32_t adcSelect);
BERR_Code BSAT_45402_P_GetSignalDetectStatus(BSAT_ChannelHandle h, BSAT_SignalDetectStatus *pStatus);
BERR_Code BSAT_45402_P_GetLegacyQpskAcqSettings(BSAT_ChannelHandle h, BSAT_LegacyQpskAcqSettings *pSettings);
BERR_Code BSAT_45402_P_SetLegacyQpskAcqSettings(BSAT_ChannelHandle h, BSAT_LegacyQpskAcqSettings *pSettings);
BERR_Code BSAT_45402_P_GetDvbs2AcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2AcqSettings *pSettings);
BERR_Code BSAT_45402_P_SetDvbs2AcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2AcqSettings *pSettings);
BERR_Code BSAT_45402_P_GetDvbs2xAcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2xAcqSettings *pSettings);
BERR_Code BSAT_45402_P_SetDvbs2xAcqSettings(BSAT_ChannelHandle h, BSAT_Dvbs2xAcqSettings *pSettings);
BERR_Code BSAT_45402_P_GetTurboAcqSettings(BSAT_ChannelHandle h, BSAT_TurboAcqSettings *pSettings);
BERR_Code BSAT_45402_P_SetTurboAcqSettings(BSAT_ChannelHandle h, BSAT_TurboAcqSettings *pSettings);
BERR_Code BSAT_45402_P_GetExtAcqSettings(BSAT_ChannelHandle h, BSAT_ExtAcqSettings *pSettings);
BERR_Code BSAT_45402_P_SetExtAcqSettings(BSAT_ChannelHandle h, BSAT_ExtAcqSettings *pSettings);
BERR_Code BSAT_45402_P_GetTraceInfo(BSAT_ChannelHandle h, BSAT_TraceInfo*);
BERR_Code BSAT_45402_P_SetExternalBertSettings(BSAT_Handle h, BSAT_ExternalBertSettings*);
BERR_Code BSAT_45402_P_GetExternalBertSettings(BSAT_Handle h, BSAT_ExternalBertSettings*);
BERR_Code BSAT_45402_P_StartPsdScan(BSAT_ChannelHandle h, uint32_t freq, uint32_t adcSelect);
BERR_Code BSAT_45402_P_GetPsdScanStatus(BSAT_ChannelHandle h, BSAT_PsdScanStatus *pStatus);
BERR_Code BSAT_45402_P_SetAcmSettings(BSAT_ChannelHandle h, BSAT_AcmSettings *pSettings);
BERR_Code BSAT_45402_P_GetAcmSettings(BSAT_ChannelHandle h, BSAT_AcmSettings *pSettings);
BERR_Code BSAT_45402_P_GetStreamList(BSAT_ChannelHandle h, int bufsize, int *pNumStreams, uint8_t *pStreamIdList);
BERR_Code BSAT_45402_P_GetStreamStatus(BSAT_ChannelHandle h, uint8_t streamId, BSAT_StreamStatus *pStatus);
BERR_Code BSAT_45402_P_ScanSpectrum(BSAT_ChannelHandle h, BSAT_ScanSpectrumSettings *pSettings);
BERR_Code BSAT_45402_P_GetSpectrumStatus(BSAT_ChannelHandle h, BSAT_SpectrumStatus *pStatus, uint8_t *pSamples);

/* other functions */
BERR_Code BSAT_45402_P_SendCommand(BHAB_Handle h, uint32_t *pBuf, uint32_t n);

#ifdef __cplusplus
}
#endif

#endif /* _BSAT_45402_PRIV_H_ */
