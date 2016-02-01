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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef _BAST_4528_PRIV_H__
#define _BAST_4528_PRIV_H__

#include "bast_4528.h"
#ifdef BAST_USE_HAB_PI
#include "bhab.h"
#endif

#define BAST_MAX_BBSI_RETRIES 10

#define BAST_4528_MAX_CHANNELS 8

#define BAST_4528_CHK_RETCODE(x) \
   { if ((retCode = (x)) != BERR_SUCCESS) goto done; }

/* interrupt bits in LEAP_HOST_IRQ_STATUS0 */
#define BAST_4528_HIRQ0_INIT_DONE            0x00000001
#define BAST_4528_HIRQ0_HAB_DONE             0x00000002
#define BAST_4528_HIRQ0_HAB_ERROR            0x00000004 /* look at SPARE3[2:0] for error code */
#define BAST_4528_HIRQ0_FW_ERROR             0x00000008 /* look at SPARE3[4:3] for error code */
#define BAST_4528_HIRQ0_FTM_MSG_READY        0x00000010
#define BAST_4528_HIRQ0_FTM_ERROR            0x00000020 /* look at SPARE3[7:5] for FTM error code */
#define BAST_4528_HIRQ0_FTM_TX_BUFFER_AVAIL  0x00000040
#define BAST_4528_HIRQ0_DISEQC_DONE          0x00000080
#define BAST_4528_HIRQ0_DISEQC_OVERVOLTAGE   0x00000100
#define BAST_4528_HIRQ0_DISEQC_UNDERVOLTAGE  0x00000200

/* interrupt bits in LEAP_HOST_IRQ_STATUS1 */
#define BAST_4528_HIRQ1_CHAN0_LOCK_CHANGE    0x00000001
#define BAST_4528_HIRQ1_CHAN0_STATUS_EVENT   0x00000002
#define BAST_4528_HIRQ1_CHAN0_RESERVED       0x00000004
#define BAST_4528_HIRQ1_CHAN0_PEAK_SCAN_DONE 0x00000008
#define BAST_4528_HIRQ1_CHAN1_LOCK_CHANGE    0x00000010
#define BAST_4528_HIRQ1_CHAN1_STATUS_EVENT   0x00000020
#define BAST_4528_HIRQ1_CHAN1_RESERVED       0x00000040
#define BAST_4528_HIRQ1_CHAN1_PEAK_SCAN_DONE 0x00000080
#define BAST_4528_HIRQ1_CHAN2_LOCK_CHANGE    0x00000100
#define BAST_4528_HIRQ1_CHAN2_STATUS_EVENT   0x00000200
#define BAST_4528_HIRQ1_CHAN2_RESERVED       0x00000400
#define BAST_4528_HIRQ1_CHAN2_PEAK_SCAN_DONE 0x00000800
#define BAST_4528_HIRQ1_CHAN3_LOCK_CHANGE    0x00001000
#define BAST_4528_HIRQ1_CHAN3_STATUS_EVENT   0x00002000
#define BAST_4528_HIRQ1_CHAN3_RESERVED       0x00004000
#define BAST_4528_HIRQ1_CHAN3_PEAK_SCAN_DONE 0x00008000
#define BAST_4528_HIRQ1_CHAN4_LOCK_CHANGE    0x00010000
#define BAST_4528_HIRQ1_CHAN4_STATUS_EVENT   0x00020000
#define BAST_4528_HIRQ1_CHAN4_RESERVED       0x00040000
#define BAST_4528_HIRQ1_CHAN4_PEAK_SCAN_DONE 0x00080000
#define BAST_4528_HIRQ1_CHAN5_LOCK_CHANGE    0x00100000
#define BAST_4528_HIRQ1_CHAN5_STATUS_EVENT   0x00200000
#define BAST_4528_HIRQ1_CHAN5_RESERVED       0x00400000
#define BAST_4528_HIRQ1_CHAN5_PEAK_SCAN_DONE 0x00800000
#define BAST_4528_HIRQ1_CHAN6_LOCK_CHANGE    0x01000000
#define BAST_4528_HIRQ1_CHAN6_STATUS_EVENT   0x02000000
#define BAST_4528_HIRQ1_CHAN6_RESERVED       0x04000000
#define BAST_4528_HIRQ1_CHAN6_PEAK_SCAN_DONE 0x08000000
#define BAST_4528_HIRQ1_CHAN7_LOCK_CHANGE    0x10000000
#define BAST_4528_HIRQ1_CHAN7_STATUS_EVENT   0x20000000
#define BAST_4528_HIRQ1_CHAN7_RESERVED       0x40000000
#define BAST_4528_HIRQ1_CHAN7_PEAK_SCAN_DONE 0x80000000

/* LEAP_CTRL_SPARE0 bits */
#define BAST_4528_SPARE0_LOCK0               0x00000001
#define BAST_4528_SPARE0_LOCK1               0x00000002
#define BAST_4528_SPARE0_LOCK2               0x00000004
#define BAST_4528_SPARE0_LOCK3               0x00000008
#define BAST_4528_SPARE0_LOCK4               0x00000010
#define BAST_4528_SPARE0_LOCK5               0x00000020
#define BAST_4528_SPARE0_LOCK6               0x00000040
#define BAST_4528_SPARE0_LOCK7               0x00000080
#define BAST_4528_SPARE0_INIT_DONE           0x80000000

/* LEAP_CTRL_SPARE1 bits */

/* LEAP_CTRL_SPARE2 bits */

/* LEAP_CTRL_SPARE3 bits */
#define BAST_4528_SPARE3_HAB_ERROR_CODE_MASK 0x00000007
#define BAST_4528_SPARE3_FW_ERROR_CODE_MASK  0x00000018
#define BAST_4528_SPARE3_FTM_ERROR_CODE_MASK 0x000000E0
#define BAST_4528_SPARE3_LAST_HAB_CMD_MASK   0x0000FF00


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BAST handle
Description:
   This is the chip-specific component of the BAST_Handle.
See Also:
   None.
****************************************************************************/
typedef struct BAST_4528_P_Handle
{
#ifdef BAST_USE_HAB_PI
   BHAB_Handle      hHab;
#else
   BREG_I2C_Handle  hRegister;         /* handle to the master I2C device used to access registers */
   BKNI_EventHandle hInterruptEvent;   /* interrupt event handle */   
   BKNI_EventHandle hApiEvent;         /* API event handle */   
   BKNI_EventHandle hInitDoneEvent;    /* AP initialization done event handle */
   BKNI_EventHandle hHabDoneEvent;     /* HAB done event handle */
#endif
   BKNI_EventHandle hFskEvent;         /* FSK event handle */
   BKNI_EventHandle hDiseqcEvent;      /* diseqc event handle */
   BKNI_EventHandle hDiseqcOverVoltageEvent;  /* diseqc overvoltage event handle */
   BKNI_EventHandle hDiseqcUnderVoltageEvent; /* diseqc undervoltage event handle */
} BAST_4528_P_Handle;


/***************************************************************************
Summary:
   Structure for chip-specific portion of the BAST channel handle
Description:
   This is the chip-specific component of the BAST_ChannelHandle.
See Also:
   None.
****************************************************************************/
typedef struct BAST_4528_P_ChannelHandle
{
   BKNI_EventHandle hLockChangeEvent;  /* change of lock status event handle */
   BKNI_EventHandle hStatusEvent;      /* status threshold event handle */
   BKNI_EventHandle hPeakScanEvent;    /* peak scan done event handle */
   BAST_Mode        lastMode;          /* most recent acquisition mode */
} BAST_4528_P_ChannelHandle;


#ifndef BAST_USE_HAB_PI
/******************************************************************************
Summary:
   Resets the AP.
Description:
   This function puts the LEAP in reset state.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_4528_P_ResetAp(
   BAST_Handle h   /* [in] BAST handle */
);


/******************************************************************************
Summary:
   This function sends an HAB command.
Description:
   This function writes a command to the HAB, asserts HAB_REQUEST, waits for 
   the LEAP to release the HAB, then returns back the data from the HAB.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_4528_P_SendHabCommand(
   BAST_Handle h,      /* [in] BAST handle */
   uint8_t *write_buf, /* [in] specifies the HAB command to send */
   uint8_t write_len,  /* [in] number of bytes in the HAB command */ 
   uint8_t *read_buf,  /* [out] holds the data read from the HAB after the command has been serviced */ 
   bool    bChecksum   /* [in] true = automatically compute checksum */
);


/******************************************************************************
Summary:
   Waits for BBSI transaction to complete. 
Description:
   This function waits for BBSI busy bit to be cleared or any of the error flag
   to be set.
Returns:
   BERR_Code
******************************************************************************/
BERR_Code BAST_4528_P_WaitForBbsiDone(
   BAST_Handle h  /* [in] BAST handle */
);
#endif /* BAST_USE_HAB_PI */


/* 4528 implementation of API functions */
BERR_Code BAST_4528_P_Open(BAST_Handle *, BCHP_Handle, void*, BINT_Handle, const BAST_Settings *pDefSettings);
BERR_Code BAST_4528_P_Close(BAST_Handle);
BERR_Code BAST_4528_P_GetTotalChannels(BAST_Handle, uint32_t *);
BERR_Code BAST_4528_P_OpenChannel(BAST_Handle, BAST_ChannelHandle *, uint32_t chnNo, const BAST_ChannelSettings *pChnDefSettings);
BERR_Code BAST_4528_P_CloseChannel(BAST_ChannelHandle);
BERR_Code BAST_4528_P_GetDevice(BAST_ChannelHandle, BAST_Handle *);
BERR_Code BAST_4528_P_InitAp(BAST_Handle, const uint8_t *);
BERR_Code BAST_4528_P_SoftReset(BAST_Handle);
BERR_Code BAST_4528_P_GetApStatus(BAST_Handle, BAST_ApStatus *);
BERR_Code BAST_4528_P_GetApVersion(BAST_Handle, uint16_t*, uint8_t*, uint32_t*, uint8_t*, uint8_t*);
BERR_Code BAST_4528_P_ConfigGpio(BAST_Handle, uint32_t, uint32_t);
BERR_Code BAST_4528_P_SetGpio(BAST_Handle, uint32_t, uint32_t);
BERR_Code BAST_4528_P_GetGpio(BAST_Handle, uint32_t, uint32_t *);
BERR_Code BAST_4528_P_TuneAcquire(BAST_ChannelHandle, const uint32_t, const BAST_AcqSettings *);
BERR_Code BAST_4528_P_GetChannelStatus(BAST_ChannelHandle, BAST_ChannelStatus *);
BERR_Code BAST_4528_P_GetLockStatus(BAST_ChannelHandle, bool *);
BERR_Code BAST_4528_P_ResetStatus(BAST_ChannelHandle);
BERR_Code BAST_4528_P_SetDiseqcTone(BAST_ChannelHandle, bool);
BERR_Code BAST_4528_P_GetDiseqcTone(BAST_ChannelHandle, bool*);
BERR_Code BAST_4528_P_SetDiseqcVoltage(BAST_ChannelHandle, bool);
BERR_Code BAST_4528_P_GetDiseqcVoltage(BAST_ChannelHandle, uint8_t*);
BERR_Code BAST_4528_P_EnableVsenseInterrupts(BAST_ChannelHandle, bool);
BERR_Code BAST_4528_P_SendDiseqcCommand(BAST_ChannelHandle, const uint8_t *pSendBuf, uint8_t sendBufLen);
BERR_Code BAST_4528_P_GetDiseqcStatus(BAST_ChannelHandle, BAST_DiseqcStatus *pStatus);
BERR_Code BAST_4528_P_ResetDiseqc(BAST_ChannelHandle, uint8_t options);
BERR_Code BAST_4528_P_ResetFsk(BAST_Handle);
BERR_Code BAST_4528_P_ReadFsk(BAST_Handle, uint8_t *pBuf, uint8_t *n);
BERR_Code BAST_4528_P_WriteFsk(BAST_Handle, uint8_t *pBuf, uint8_t n);
BERR_Code BAST_4528_P_PowerDownFsk(BAST_Handle);
BERR_Code BAST_4528_P_PowerUpFsk(BAST_Handle);
BERR_Code BAST_4528_P_WriteMi2c(BAST_ChannelHandle, uint8_t, uint8_t*, uint8_t);
BERR_Code BAST_4528_P_ReadMi2c(BAST_ChannelHandle, uint8_t, uint8_t*, uint8_t, uint8_t *, uint8_t);
BERR_Code BAST_4528_P_GetSoftDecisionBuf(BAST_ChannelHandle, int16_t*, int16_t*);
BERR_Code BAST_4528_P_FreezeEq(BAST_ChannelHandle, bool);
BERR_Code BAST_4528_P_PowerDown(BAST_ChannelHandle, uint32_t);
BERR_Code BAST_4528_P_PowerUp(BAST_ChannelHandle, uint32_t);
BERR_Code BAST_4528_P_ReadRegister(BAST_ChannelHandle, uint32_t, uint32_t*);
BERR_Code BAST_4528_P_WriteRegister(BAST_ChannelHandle, uint32_t, uint32_t*);
BERR_Code BAST_4528_P_ReadConfig(BAST_ChannelHandle, uint16_t, uint8_t*, uint8_t);
BERR_Code BAST_4528_P_WriteConfig(BAST_ChannelHandle, uint16_t, uint8_t*, uint8_t);
BERR_Code BAST_4528_P_GetLockStateChangeEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
BERR_Code BAST_4528_P_GetFskEventHandle(BAST_Handle, BKNI_EventHandle*);
BERR_Code BAST_4528_P_GetDiseqcEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
BERR_Code BAST_4528_P_GetDiseqcVsenseEventHandles(BAST_ChannelHandle, BKNI_EventHandle*, BKNI_EventHandle*);
BERR_Code BAST_4528_P_AbortAcq(BAST_ChannelHandle);
BERR_Code BAST_4528_P_DeepSleep(BAST_Handle h);
BERR_Code BAST_4528_P_PeakScan(BAST_ChannelHandle, uint32_t);
BERR_Code BAST_4528_P_GetPeakScanStatus(BAST_ChannelHandle, BAST_PeakScanStatus*);
BERR_Code BAST_4528_P_GetPeakScanEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
BERR_Code BAST_4528_P_EnableStatusInterrupts(BAST_ChannelHandle, bool);
BERR_Code BAST_4528_P_GetStatusEventHandle(BAST_ChannelHandle, BKNI_EventHandle*);
BERR_Code BAST_4528_P_ConfigBcm3445(BAST_Handle h, BAST_Bcm3445Settings *pSettings);
BERR_Code BAST_4528_P_MapBcm3445ToTuner(BAST_ChannelHandle h, BAST_Mi2cChannel mi2c, BAST_Bcm3445OutputChannel out);
BERR_Code BAST_4528_P_GetBcm3445Status(BAST_ChannelHandle h, BAST_Bcm3445Status *pStatus);
BERR_Code BAST_4528_P_SetSearchRange(BAST_Handle, uint32_t);
BERR_Code BAST_4528_P_GetSearchRange(BAST_Handle, uint32_t*);
BERR_Code BAST_4528_P_SetAmcScramblingSeq(BAST_ChannelHandle, uint32_t xseed, uint32_t plhdrscr1, uint32_t plhdrscr2, uint32_t plhdrscr3);
BERR_Code BAST_4528_P_SetTunerFilter(BAST_ChannelHandle, uint32_t);
BERR_Code BAST_4528_P_GetSignalDetectStatus(BAST_ChannelHandle, BAST_SignalDetectStatus*);
BERR_Code BAST_4528_P_SetOutputTransportSettings(BAST_ChannelHandle, BAST_OutputTransportSettings*);
BERR_Code BAST_4528_P_GetOutputTransportSettings(BAST_ChannelHandle, BAST_OutputTransportSettings*);
BERR_Code BAST_4528_P_SetNetworkSpec(BAST_Handle, BAST_NetworkSpec);
BERR_Code BAST_4528_P_GetNetworkSpec(BAST_Handle, BAST_NetworkSpec*);
BERR_Code BAST_4528_P_ResetChannel(BAST_ChannelHandle, bool);
BERR_Code BAST_4528_P_EnableDiseqcLnb(BAST_ChannelHandle h, bool bEnable);
BERR_Code BAST_4528_P_SetDiseqcSettings(BAST_ChannelHandle h, BAST_DiseqcSettings *pSettings);
BERR_Code BAST_4528_P_GetDiseqcSettings(BAST_ChannelHandle h, BAST_DiseqcSettings *pSettings);
#ifndef BAST_USE_HAB_PI
BERR_Code BAST_4528_P_ProcessInterruptEvent(BAST_Handle);
BERR_Code BAST_4528_P_HandleInterrupt_isr(BAST_Handle);
BERR_Code BAST_4528_P_GetInterruptEventHandle(BAST_Handle, BKNI_EventHandle*);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BAST_4528_P_PRIV_H__ */

