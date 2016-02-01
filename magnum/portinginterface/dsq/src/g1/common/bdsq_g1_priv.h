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

#ifndef BDSQ_G1_PRIV_H__
#define BDSQ_G1_PRIV_H__

#include "bdsq.h"
#include "bdsq_g1.h"

#if (BCHP_CHIP==7366)
   #include "bdsq_7366_priv.h"
#elif (BCHP_CHIP==4550)
	 #include "bdsq_4550_priv.h"
#elif (BCHP_CHIP==4552)
	 #include "bdsq_4552_priv.h"
#elif (BCHP_CHIP==45216)
   #include "bdsq_45216_priv.h"
#elif (BCHP_CHIP==45308)
   #include "bdsq_45308_priv.h"
#elif (BCHP_CHIP==7364)
   #include "bdsq_7364_priv.h"
#elif (BCHP_CHIP==4554)
	 #include "bdsq_4554_priv.h"
#else
   #error "unsupported BCHP_CHIP"
#endif

#define BDSQ_G1_RELEASE_VERSION 1


/******************************************************************************
 bit definitions for BDSQ_g1_P_ChannelHandle.bufStatus
******************************************************************************/
#define BDSQ_BUF_ALL_EMPTY       0x00
#define BDSQ_BUF_TX_NOT_EMPTY    0x01
#define BDSQ_BUF_RX_NOT_EMPTY    0x02


typedef BERR_Code (*BDSQ_FUNCT)(BDSQ_ChannelHandle);

/******************************************************************************
Summary:
   This is the structure for the chip-specific component of the BDSQ_Handle.
******************************************************************************/
typedef struct BDSQ_g1_P_Handle
{
   BCHP_Handle hChip;               /* chip handle */
   BREG_Handle hRegister;           /* register handle */
   BINT_Handle hInterrupt;          /* interrupt handle */
} BDSQ_g1_P_Handle;


/******************************************************************************
Summary:
   This is the structure for the chip-specific component of the BDSQ_ChannelHandle.
******************************************************************************/
typedef struct BDSQ_g1_P_ChannelHandle
{
   BKNI_EventHandle     hTxEvent;         /* diseqc tx event handle */
   BKNI_EventHandle     hRxEvent;         /* diseqc rx event handle */
   BKNI_EventHandle     hVsenseEvent;     /* vsense event handle */
   BKNI_EventHandle     hActivityEvent;   /* activity event handle */
   BKNI_EventHandle     hIdleEvent;       /* idle event handle */
   BINT_CallbackHandle  hTimer1Cb;        /* callback handle for timer1 interrupt */
   BINT_CallbackHandle  hTimer2Cb;        /* callback handle for timer2 interrupt */
   BINT_CallbackHandle  hDiseqcDoneCb;    /* callback handle for diseqc done interrupt */
   BINT_CallbackHandle  hTxAlmostEmptyCb; /* callback handle for tx fifo almost empty interrupt */
   BINT_CallbackHandle  hRxAlmostFullCb;  /* callback handle for rx fifo almost full interrupt */
   BINT_CallbackHandle  hAcwDoneCb;       /* callback handle for acw done interrupt */
   BINT_CallbackHandle  hUnderVoltageCb;  /* callback handle for voltage less than threshold interrupt */
   BINT_CallbackHandle  hOverVoltageCb;   /* callback handle for voltage greater than threshold interrupt */
   BINT_CallbackHandle  hToneRiseCb;      /* callback handle for tone rise/activity detected interrupt */
   BINT_CallbackHandle  hToneFallCb;      /* callback handle for tone fall interrupt */
   BINT_CallbackHandle  hIdleTimeoutCb;   /* callback handle for idle timeout interrupt */
   BDSQ_FUNCT           timer1Isr;        /* function for handling timer1 interrupt */
   BDSQ_FUNCT           timer2Isr;        /* function for handling timer2 interrupt */
   BDSQ_FUNCT           dsecDoneIsr;      /* function for handling diseqc done interrupt */
   BDSQ_Status          dsecStatus;       /* status of last diseqc transaction */
   uint32_t             configParam[BDSQ_g1_CONFIG_MAX]; /* configuration parameters */
   uint16_t             rxFirstBitPw;     /* rx first bit pulse width */
   uint16_t             rxBitTiming;      /* rx bit slice timing */
   uint8_t              bufStatus;        /* status of tx and rx buffers */
   uint8_t              txLen;            /* transmit length */
   uint8_t              txCount;          /* transmit byte count */
   uint8_t              rxCount;          /* receive byte count */
   uint8_t              rxIdx;            /* start idx of rx buffer for multiple reads */
   uint8_t              rxBitCount;       /* rx bit count */
   uint8_t              acw;              /* auto-control word */
   uint8_t              txBuf[BDSQ_MAX_TX_LEN]; /* transmit buffer */
   uint8_t              rxBuf[BDSQ_MAX_RX_LEN]; /* receive buffer */
   bool                 bDiseqcToneOn;    /* true if diseqc tone is enabled */
   bool                 bVsenseEnabled;   /* true if vsense is enabled */
   bool                 bActivityInterruptOn;   /* true if activity interrupt enabled */
} BDSQ_g1_P_ChannelHandle;


/* bdsq_g1_priv.c */
BERR_Code BDSQ_g1_P_Open(BDSQ_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInterrupt, const BDSQ_Settings *pSettings);
BERR_Code BDSQ_g1_P_Close(BDSQ_Handle h);
BERR_Code BDSQ_g1_P_GetTotalChannels(BDSQ_Handle h, uint32_t *totalChannels);
BERR_Code BDSQ_g1_P_OpenChannel(BDSQ_Handle h, BDSQ_ChannelHandle *pChannelHandle, uint32_t chan, const BDSQ_ChannelSettings *pSettings);
BERR_Code BDSQ_g1_P_CloseChannel(BDSQ_ChannelHandle h);
BERR_Code BDSQ_g1_P_GetDevice(BDSQ_ChannelHandle h, BDSQ_Handle *pDev);
BERR_Code BDSQ_g1_P_GetVersionInfo(BDSQ_Handle h, BFEC_VersionInfo *pVersion);
BERR_Code BDSQ_g1_P_Reset(BDSQ_Handle h);
BERR_Code BDSQ_g1_P_ResetChannel(BDSQ_ChannelHandle h);
BERR_Code BDSQ_g1_P_PowerDownChannel(BDSQ_ChannelHandle h);
BERR_Code BDSQ_g1_P_PowerUpChannel(BDSQ_ChannelHandle h);
BERR_Code BDSQ_g1_P_IsChannelOn(BDSQ_ChannelHandle h, bool *pOn);
BERR_Code BDSQ_g1_P_PowerDownVsense(BDSQ_ChannelHandle h);
BERR_Code BDSQ_g1_P_PowerUpVsense(BDSQ_ChannelHandle h);
BERR_Code BDSQ_g1_P_IsVsenseOn(BDSQ_ChannelHandle h, bool *pOn);
BERR_Code BDSQ_g1_P_SetTone(BDSQ_ChannelHandle h, bool bEnable);
BERR_Code BDSQ_g1_P_GetTone(BDSQ_ChannelHandle h, bool *pbTone);
BERR_Code BDSQ_g1_P_SetVoltage(BDSQ_ChannelHandle h, bool bVtop);
BERR_Code BDSQ_g1_P_GetVoltage(BDSQ_ChannelHandle h, uint16_t *pVoltage);
BERR_Code BDSQ_g1_P_EnableVsenseInterrupt(BDSQ_ChannelHandle h, bool bEnable);
BERR_Code BDSQ_g1_P_EnableActivityInterrupt(BDSQ_ChannelHandle h, bool bEnable);
BERR_Code BDSQ_g1_P_EnableIdleInterrupt(BDSQ_ChannelHandle h, bool bEnable);
BERR_Code BDSQ_g1_P_Write(BDSQ_ChannelHandle h, const uint8_t *pBuf, uint8_t n);
BERR_Code BDSQ_g1_P_EnableRx(BDSQ_ChannelHandle h, bool bEnable);
BERR_Code BDSQ_g1_P_EnableLnb(BDSQ_ChannelHandle h, bool bEnable);
BERR_Code BDSQ_g1_P_Read(BDSQ_ChannelHandle h, uint8_t *pBuf, uint8_t nBufMax, uint8_t *n, uint8_t *nNotRead);
BERR_Code BDSQ_g1_P_SendAcw(BDSQ_ChannelHandle h, uint8_t acw);
BERR_Code BDSQ_g1_P_GetStatus(BDSQ_ChannelHandle h, BDSQ_Status *pStatus);
BERR_Code BDSQ_g1_P_GetActivityStatus(BDSQ_ChannelHandle h, BDSQ_ActivityStatus *pActivityStatus);
BERR_Code BDSQ_g1_P_ResetActivityStatus(BDSQ_ChannelHandle h);
BERR_Code BDSQ_g1_P_SetChannelSettings(BDSQ_ChannelHandle h, BDSQ_ChannelSettings *pSettings);
BERR_Code BDSQ_g1_P_GetChannelSettings(BDSQ_ChannelHandle h, BDSQ_ChannelSettings *pSettings);
BERR_Code BDSQ_g1_P_SetConfig(BDSQ_Handle h, uint32_t addr, uint32_t val);
BERR_Code BDSQ_g1_P_GetConfig(BDSQ_Handle h, uint32_t addr, uint32_t *pVal);
BERR_Code BDSQ_g1_P_SetChannelConfig(BDSQ_ChannelHandle h, uint32_t addr, uint32_t val);
BERR_Code BDSQ_g1_P_GetChannelConfig(BDSQ_ChannelHandle h, uint32_t addr, uint32_t *pVal);
BERR_Code BDSQ_g1_P_GetTxEventHandle(BDSQ_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BDSQ_g1_P_GetRxEventHandle(BDSQ_ChannelHandle h, BKNI_EventHandle *hEvent);
BERR_Code BDSQ_g1_P_GetVsenseEventHandle(BDSQ_ChannelHandle h, BKNI_EventHandle *hVsenseEvent);
BERR_Code BDSQ_g1_P_GetActivityEventHandle(BDSQ_ChannelHandle h, BKNI_EventHandle *hVsenseEvent);
BERR_Code BDSQ_g1_P_GetIdleEventHandle(BDSQ_ChannelHandle h, BKNI_EventHandle *hIdleEvent);
BERR_Code BDSQ_g1_P_DisableTimer_isr(BDSQ_ChannelHandle h, BDSQ_g1_TimerSelect t);
BERR_Code BDSQ_g1_P_EnableTimer_isr(BDSQ_ChannelHandle h, BDSQ_g1_TimerSelect t, uint32_t delay_usec, BDSQ_FUNCT func);
void BDSQ_g1_P_Timer1_isr(void *p, int param);
void BDSQ_g1_P_Timer2_isr(void *p, int param);
void BDSQ_g1_P_UnderVoltage_isr(void *p, int param);
void BDSQ_g1_P_OverVoltage_isr(void *p, int param);
void BDSQ_g1_P_ToneRise_isr(void *p, int param);
void BDSQ_g1_P_ToneFall_isr(void *p, int param);
void BDSQ_g1_P_IdleTimeout_isr(void *p, int param);

/* bdsq_g1_priv_txn.c */
BERR_Code BDSQ_g1_Txn_P_UpdateSettings(BDSQ_ChannelHandle h);
BERR_Code BDSQ_g1_Txn_P_Transmit_isr(BDSQ_ChannelHandle h);
void BDSQ_g1_Txn_P_TxAlmostEmpty_isr(void *p, int param);
void BDSQ_g1_Txn_P_RxAlmostFull_isr(void *p, int param);
void BDSQ_g1_Txn_P_DiseqcDone_isr(void *p, int param);
BERR_Code BDSQ_g1_Txn_P_SendAcw(BDSQ_ChannelHandle h, uint8_t acw);
void BDSQ_g1_Txn_P_AcwDone_isr(void *p, int param);

#endif /* BDSQ_G1_PRIV_H__ */
