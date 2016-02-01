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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef _BAST_4538_PRIV_H__
#define _BAST_4538_PRIV_H__

#include "bast_4538.h"
#include "bhab.h"

#define BAST_4538_RELEASE_VERSION 6

#define BAST_MAX_BBSI_RETRIES 10

#define BAST_4538_MAX_CHANNELS 8
#define BAST_4538_DSEC_CHANNELS 4

#define BAST_4538_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }



/***************************************************************************
Summary:
   Structure for chip-specific portion of the BAST handle
Description:
   This is the chip-specific component of the BAST_Handle.
See Also:
   None.
****************************************************************************/
typedef struct BAST_4538_P_Handle
{
   BHAB_Handle      hHab;
   BKNI_EventHandle hFskEvent;                  /* fsk rx ready event handle */
   int              currDiseqcChannel;
} BAST_4538_P_Handle;


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BAST channel handle
Description:
   This is the chip-specific component of the BAST_ChannelHandle.
See Also:
   None.
****************************************************************************/
typedef struct BAST_4538_P_ChannelHandle
{
   BKNI_EventHandle hLockChangeEvent;  /* change of lock status event handle */
   BKNI_EventHandle hStatusEvent;      /* status threshold event handle */
   BKNI_EventHandle hPeakScanEvent;    /* peak scan done event handle */
   BKNI_EventHandle hDiseqcEvent;               /* diseqc event handle */
   BKNI_EventHandle hDiseqcOverVoltageEvent;    /* diseqc overvoltage event handle */
   BKNI_EventHandle hDiseqcUnderVoltageEvent;   /* diseqc undervoltage event handle */
   BAST_Mode        lastMode;          /* most recent acquisition mode */
} BAST_4538_P_ChannelHandle;


/* 4538 implementation of API functions */
BERR_Code BAST_4538_P_Open(BAST_Handle *, BCHP_Handle, void*, BINT_Handle, const BAST_Settings *pDefSettings);
BERR_Code BAST_4538_P_Close(BAST_Handle);
BERR_Code BAST_4538_P_GetTotalChannels(BAST_Handle, uint32_t *);
BERR_Code BAST_4538_P_OpenChannel(BAST_Handle, BAST_ChannelHandle *, uint32_t chnNo, const BAST_ChannelSettings *pChnDefSettings);
BERR_Code BAST_4538_P_CloseChannel(BAST_ChannelHandle);
BERR_Code BAST_4538_P_GetDevice(BAST_ChannelHandle, BAST_Handle *);
BERR_Code BAST_4538_P_InitAp(BAST_Handle, const uint8_t *);
BERR_Code BAST_4538_P_SoftReset(BAST_Handle);
BERR_Code BAST_4538_P_GetApStatus(BAST_Handle, BAST_ApStatus *);
BERR_Code BAST_4538_P_GetApVersion(BAST_Handle, uint16_t*, uint8_t*, uint32_t*, uint8_t*, uint8_t*);
BERR_Code BAST_4538_P_ConfigGpio(BAST_Handle, uint32_t, uint32_t);
BERR_Code BAST_4538_P_SetGpio(BAST_Handle, uint32_t, uint32_t);
BERR_Code BAST_4538_P_GetGpio(BAST_Handle, uint32_t, uint32_t *);
BERR_Code BAST_4538_P_TuneAcquire(BAST_ChannelHandle, const uint32_t, const BAST_AcqSettings *);
BERR_Code BAST_4538_P_GetChannelStatus(BAST_ChannelHandle, BAST_ChannelStatus *);
BERR_Code BAST_4538_P_GetLockStatus(BAST_ChannelHandle, bool *);
BERR_Code BAST_4538_P_ResetStatus(BAST_ChannelHandle);
BERR_Code BAST_4538_P_SetDiseqcTone(BAST_ChannelHandle, bool);
BERR_Code BAST_4538_P_GetDiseqcTone(BAST_ChannelHandle, bool*);
BERR_Code BAST_4538_P_SetDiseqcVoltage(BAST_ChannelHandle, bool);
BERR_Code BAST_4538_P_GetDiseqcVoltage(BAST_ChannelHandle, uint8_t*);
BERR_Code BAST_4538_P_EnableVsenseInterrupts(BAST_ChannelHandle, bool);
BERR_Code BAST_4538_P_SendDiseqcCommand(BAST_ChannelHandle, const uint8_t *pSendBuf, uint8_t sendBufLen);
BERR_Code BAST_4538_P_GetDiseqcStatus(BAST_ChannelHandle, BAST_DiseqcStatus *pStatus);
BERR_Code BAST_4538_P_SendACW(BAST_ChannelHandle, uint8_t);
BERR_Code BAST_4538_P_ResetDiseqc(BAST_ChannelHandle, uint8_t options);
BERR_Code BAST_4538_P_ResetFsk(BAST_Handle);
BERR_Code BAST_4538_P_ReadFsk(BAST_Handle, uint8_t *pBuf, uint8_t *n);
BERR_Code BAST_4538_P_WriteFsk(BAST_Handle, uint8_t *pBuf, uint8_t n);
BERR_Code BAST_4538_P_PowerDownFsk(BAST_Handle);
BERR_Code BAST_4538_P_PowerUpFsk(BAST_Handle);
BERR_Code BAST_4538_P_GetSoftDecisionBuf(BAST_ChannelHandle, int16_t*, int16_t*);
BERR_Code BAST_4538_P_FreezeEq(BAST_ChannelHandle, bool);
BERR_Code BAST_4538_P_PowerDown(BAST_ChannelHandle, uint32_t);
BERR_Code BAST_4538_P_PowerUp(BAST_ChannelHandle, uint32_t);
BERR_Code BAST_4538_P_ReadRegister(BAST_ChannelHandle, uint32_t, uint32_t*);
BERR_Code BAST_4538_P_WriteRegister(BAST_ChannelHandle, uint32_t, uint32_t*);
BERR_Code BAST_4538_P_ReadConfig(BAST_ChannelHandle, uint16_t, uint8_t*, uint8_t);
BERR_Code BAST_4538_P_WriteConfig(BAST_ChannelHandle, uint16_t, uint8_t*, uint8_t);
BERR_Code BAST_4538_P_GetLockStateChangeEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
BERR_Code BAST_4538_P_GetDiseqcEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
BERR_Code BAST_4538_P_GetDiseqcVsenseEventHandles(BAST_ChannelHandle, BKNI_EventHandle*, BKNI_EventHandle*);
BERR_Code BAST_4538_P_AbortAcq(BAST_ChannelHandle);
BERR_Code BAST_4538_P_PeakScan(BAST_ChannelHandle, uint32_t);
BERR_Code BAST_4538_P_GetPeakScanStatus(BAST_ChannelHandle, BAST_PeakScanStatus*);
BERR_Code BAST_4538_P_GetPeakScanEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
BERR_Code BAST_4538_P_EnableStatusInterrupts(BAST_ChannelHandle, bool);
BERR_Code BAST_4538_P_GetStatusEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
BERR_Code BAST_4538_P_SetSearchRange(BAST_Handle, uint32_t);
BERR_Code BAST_4538_P_GetSearchRange(BAST_Handle, uint32_t*);
BERR_Code BAST_4538_P_SetTunerFilter(BAST_ChannelHandle, uint32_t);
BERR_Code BAST_4538_P_GetSignalDetectStatus(BAST_ChannelHandle, BAST_SignalDetectStatus*);
BERR_Code BAST_4538_P_SetOutputTransportSettings(BAST_ChannelHandle, BAST_OutputTransportSettings*);
BERR_Code BAST_4538_P_GetOutputTransportSettings(BAST_ChannelHandle, BAST_OutputTransportSettings*);
BERR_Code BAST_4538_P_SetNetworkSpec(BAST_Handle, BAST_NetworkSpec);
BERR_Code BAST_4538_P_GetNetworkSpec(BAST_Handle, BAST_NetworkSpec*);
BERR_Code BAST_4538_P_ResetChannel(BAST_ChannelHandle, bool);
BERR_Code BAST_4538_P_EnableDiseqcLnb(BAST_ChannelHandle h, bool bEnable);
BERR_Code BAST_4538_P_SetDiseqcSettings(BAST_ChannelHandle h, BAST_DiseqcSettings *pSettings);
BERR_Code BAST_4538_P_GetDiseqcSettings(BAST_ChannelHandle h, BAST_DiseqcSettings *pSettings);
BERR_Code BAST_4538_P_SetPeakScanSymbolRateRange(BAST_ChannelHandle, uint32_t, uint32_t);
BERR_Code BAST_4538_P_GetPeakScanSymbolRateRange(BAST_ChannelHandle, uint32_t*, uint32_t*); 
BERR_Code BAST_4538_P_EnableSpurCanceller(BAST_ChannelHandle h, uint8_t n, BAST_SpurCancellerConfig *pConfig);
BERR_Code BAST_4538_P_GetFskEventHandle(BAST_Handle h, BKNI_EventHandle *hEvent);
BERR_Code BAST_4538_P_SetAmcScramblingSeq(BAST_ChannelHandle, uint32_t xseed, uint32_t plhdrscr1, uint32_t plhdrscr2, uint32_t plhdrscr3);
BERR_Code BAST_4538_P_SetAdcSelect(BAST_ChannelHandle, uint8_t);
BERR_Code BAST_4538_P_GetAdcSelect(BAST_ChannelHandle, uint8_t*, uint8_t*);
BERR_Code BAST_4538_P_GetVersionInfo(BAST_Handle, FEC_DeviceVersionInfo*);


#ifdef __cplusplus
}
#endif

#endif /* BAST_4538_P_PRIV_H__ */

