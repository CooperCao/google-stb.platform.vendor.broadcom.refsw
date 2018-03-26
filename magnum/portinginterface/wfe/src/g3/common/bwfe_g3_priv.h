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
 ******************************************************************************/

#ifndef BWFE_G3_PRIV_H__
#define BWFE_G3_PRIV_H__

#include "bwfe.h"
#include "bwfe_g3.h"

#if (BCHP_CHIP==7366)
   #include "bwfe_7366_priv.h"
   #define BWFE_EXCLUDE_LIC_TAP_COMPUTER
   #define BWFE_EXCLUDE_ANALOG_DELAY
#elif (BCHP_CHIP==7364)
   #include "bwfe_7364_priv.h"
   #define BWFE_EXCLUDE_LIC_TAP_COMPUTER
   #define BWFE_EXCLUDE_ANALOG_DELAY
#elif (BCHP_CHIP==45216)
   #include "bwfe_45216_priv.h"
   #define BWFE_EXCLUDE_LIC_TAP_COMPUTER
   #define BWFE_EXCLUDE_ANALOG_DELAY
#elif (BCHP_CHIP==4552)
   #include "bwfe_4552_priv.h"
   #define BWFE_EXCLUDE_LIC
   #define BWFE_EXCLUDE_LIC_TAP_COMPUTER
   #define BWFE_EXCLUDE_ANALOG_DELAY
#elif (BCHP_CHIP==4554)
   #include "bwfe_4554_priv.h"
   #define BWFE_EXCLUDE_LIC
   #define BWFE_EXCLUDE_LIC_TAP_COMPUTER
   #define BWFE_EXCLUDE_ANALOG_DELAY
#elif (BCHP_CHIP==45308)
   #include "bwfe_45308_priv.h"
   #define BWFE_EXCLUDE_LIC
   #define BWFE_EXCLUDE_LIC_TAP_COMPUTER
   #define BWFE_EXCLUDE_ANALOG_DELAY
#elif (BCHP_CHIP==45402)
   #include "bwfe_45402_priv.h"
   #define BWFE_EXCLUDE_LIC
   #define BWFE_EXCLUDE_LIC_TAP_COMPUTER
   #define BWFE_EXCLUDE_ANALOG_DELAY
#else
   #error "unsupported BCHP_CHIP"
#endif


#define BWFE_G3_RELEASE_VERSION 1


struct BWFE_P_ChannelHandle;

typedef BERR_Code (*BWFE_FUNCT)(BWFE_ChannelHandle);


/******************************************************************************
Summary:
   This is the structure for the chip-specific component of the BWFE_Handle.
******************************************************************************/
typedef struct BWFE_g3_P_Handle
{
   BCHP_Handle                hChip;               /* chip handle */
   BREG_Handle                hRegister;           /* register handle */
   BINT_Handle                hInterrupt;          /* interrupt handle */
   BKNI_EventHandle           hSaDoneEvent;        /* spectrum scan done event handle */
#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
   uint32_t                   saFreq;              /* spectrum analyzer current freq */
   uint32_t                   saStep;              /* spectrum analyzer step size */
   uint32_t                   saSamples[BWFE_SA_MAX_SAMPLES];  /* spectrum analyzer samples */
   uint16_t                   saIdx;               /* spectrum analyzer sample index */
   uint8_t                    saState;             /* current state in spectrum analyzer state machine */
   uint8_t                    saSweep;             /* spectrum analyzer sweep count */
#endif
} BWFE_g3_P_Handle;


/******************************************************************************
Summary:
   This is the structure for the chip-specific component of the BWFE_ChannelHandle.
******************************************************************************/
typedef struct BWFE_g3_P_ChannelHandle
{
   BWFE_SpecAnalyzerParams saParams;      /* spectrum analyzer parameters */
   BKNI_EventHandle     hWfeReady;        /* wfe ready event */
   BKNI_EventHandle     hDelayCalDone;    /* delay calibration done event */
   BINT_CallbackHandle  hTimer0Cb;        /* callback handle for timer0 interrupt */
   BINT_CallbackHandle  hTimer1Cb;        /* callback handle for timer1 interrupt */
   BINT_CallbackHandle  hCorrDoneCb;      /* callback handle for correlator done interrupt */
   BWFE_FUNCT           timer0Isr;        /* function for handling timer0 interrupt */
   BWFE_FUNCT           timer1Isr;        /* function for handling timer1 interrupt */
   BWFE_FUNCT           corrIdleIsr;      /* function for handling correlator idle interrupt */
   BWFE_FUNCT           postCalcDelayFunct;  /* function to call after delay calculations */
   uint32_t             dpmPilotFreqKhz;  /* DPM frequency in kHz */
   uint32_t             dpmQddfsM;        /* QDDFS M */
   uint32_t             dpmQddfsN;        /* QDDFS N */
   uint32_t             adcSampleFreqKhz; /* ADC rx clock in kHz */
   uint32_t             accSaturateCount; /* accumulator saturation count */
   uint32_t             lutSaturateCount; /* lut saturation count */
   uint32_t             rfAgcInt;         /* rf agc integrator */
   uint32_t             dgsClipCtlPing;   /* DGS clip control */
   uint32_t             dgsClipCtlPong;   /* DGS clip control */
   uint32_t             dgsClipCountPing; /* DGS clip count */
   uint32_t             dgsClipCountPong; /* DGS clip count */
   uint32_t             corrFreqHz;       /* correlator freq in Hz */
#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
   uint32_t             licCoeff[BWFE_NUM_LANES][BWFE_NUM_LIC_TAPS];    /* 2D array of LIC coeff (ping-pong lanes x taps) */
#endif
#ifndef BWFE_EXCLUDE_ANALOG_DELAY
   uint32_t             corrAgc[BWFE_LIC_L];          /* 1D array of correlator loop filter agcs */
   uint32_t             adjRight;                     /* S1 delay compensation */
   uint32_t             adjLeft;                      /* S0 delay compensation */
   int32_t              corrDelay[BWFE_LIC_L];        /* 1D array of correlator delays */
   int32_t              sliceDelay[BWFE_NUM_SLICES];  /* 1D array of slice delays */
   int32_t              analogDelay[BWFE_NUM_SLICES]; /* 1D array of delay compensation values */
   int32_t              initDelay;                    /* initial delay for fine adjustment */
   int32_t              prevDelay;                    /* previous measured delay */
#endif
   int32_t              calDelay[BWFE_NUM_ADC_PHASES][BWFE_NUM_SLICES];
#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
   int32_t              licDelay[BWFE_NUM_SLICES][BWFE_NUM_LANES];   /* 2D array of previous delay values used to calculate LIC2 taps (slices x ping-pong lanes) */
#endif
   int32_t              equCoeff[BWFE_NUM_SLICES][BWFE_NUM_LANES][BWFE_NUM_MDACS][BWFE_NUM_EQ_TAPS]; /* 4D array of EQ coeff (slices x lanes x mdacs x taps)*/
   uint16_t             equRuntimeMs;     /* digisum equalization run-time in ms */
   uint16_t             rfAgcThresh;      /* rf agc threshold */
   uint8_t              rfAgcK1;          /* rf agc k1 */
   uint8_t              rfAgcBeta;        /* rf agc beta */
   uint8_t              nrAgcK1;          /* lic2 agc k1 */
   uint8_t              dgsClipSlice;     /* DGS clip slice select */
   uint8_t              equState;         /* current state in mdac eq state machine */
   uint8_t              equCurrSlice;     /* current slice in mdac eq state machine */
   uint8_t              equCurrMdac;      /* current mdac in mdac eq state machine */
   uint8_t              equMainMu1;       /* digisum equalization main tap mu1 */
   uint8_t              equMainMu2;       /* digisum equalization main tap mu2 */
   uint8_t              equMu1;           /* digisum equalization mu1 */
   uint8_t              equMu2;           /* digisum equalization mu2 */
   uint8_t              equTapMask;       /* mask to select which eq taps to update */
   uint8_t              equMdacEnable;    /* select which mdac to run during equ */
   uint8_t              corrMaxBit;       /* 2^k accumulation limit based on corr length  */
   uint8_t              corrInputSelect;  /* correlator input select */
   uint8_t              corrDelayBeta;    /* correlator delay tracking beta in 0.05 steps */
   uint8_t              corrDpmDebug;     /* correlator dpm debug controls */
   uint8_t              corrRefSlice;     /* reference slice for correlator delay calculations */
   uint8_t              calState;         /* current state in delay calibration */
   uint8_t              calCount;         /* current loop count in delay calibration */
   bool                 bLicTrackingOn;   /* toggle LIC tracking mode */
   bool                 bCorrInProgress;  /* true if correlator is busy */
   bool                 bLicReady;        /* lic datapath is ready */
} BWFE_g3_P_ChannelHandle;


/* bwfe_g3_priv.c */
BERR_Code BWFE_g3_P_Open(BWFE_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInt, const BWFE_Settings *pDefSettings);
BERR_Code BWFE_g3_P_Close(BWFE_Handle h);
BERR_Code BWFE_g3_P_GetTotalChannels(BWFE_Handle h, BWFE_ChannelInfo *pInfo);
BERR_Code BWFE_g3_P_GetTotalRefChannels(BWFE_Handle h, uint8_t *totalRefChannels);
BERR_Code BWFE_g3_P_OpenChannel(BWFE_Handle h, BWFE_ChannelHandle *pChannelHandle, uint8_t chanNum, const BWFE_ChannelSettings *pSettings);
BERR_Code BWFE_g3_P_CloseChannel(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_Reset(BWFE_Handle h);
BERR_Code BWFE_g3_P_GetVersion(BWFE_Handle h, BFEC_VersionInfo *pVersion);
BERR_Code BWFE_g3_P_IsInputEnabled(BWFE_ChannelHandle h, bool *bEnabled);
BERR_Code BWFE_g3_P_DisableChannelInterrupts(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_EnableInput(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_DisableInput(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_GetChannelStatus(BWFE_ChannelHandle h, BWFE_ChannelStatus *pStatus);
BERR_Code BWFE_g3_P_GetLicTaps(BWFE_ChannelHandle h, uint32_t **pLicCoeff);
BERR_Code BWFE_g3_P_GetEqTaps(BWFE_ChannelHandle h, uint32_t ****pEqCoeff);
BERR_Code BWFE_g3_P_FreezeRfAgc(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_UnfreezeRfAgc(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_RunLaneCorrection(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_CalibrateINL(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_EqualizePipeline(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_SetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t freqKhz);
BERR_Code BWFE_g3_P_GetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t *freqKhz);
BERR_Code BWFE_g3_P_GetChannelConfig(BWFE_ChannelHandle h, uint16_t id, uint8_t *p, uint8_t len);
BERR_Code BWFE_g3_P_SetChannelConfig(BWFE_ChannelHandle h, uint16_t id, uint8_t *p, uint8_t len);
BERR_Code BWFE_g3_P_UpdateChannelConfig(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_ResetDgsLut(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_ResetEqTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_ResetLicTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_CancelDCOffset(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_GetWfeReadyEventHandle(BWFE_ChannelHandle h, BKNI_EventHandle *hWfeReady);
BERR_Code BWFE_g3_P_CalibrateAnalogDelay(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_GetAnalogDelay(BWFE_ChannelHandle h, uint32_t *pCorrDelay);
BERR_Code BWFE_g3_P_CompensateDelay(BWFE_ChannelHandle h, uint32_t *pSliceAdjust);
BERR_Code BWFE_g3_P_GetSaDoneEventHandle(BWFE_Handle h, BKNI_EventHandle *hSaDoneEvent);
BERR_Code BWFE_g3_P_ScanSpectrum(BWFE_ChannelHandle h, BWFE_SpecAnalyzerParams *pParams);
BERR_Code BWFE_g3_P_GetSaSamples(BWFE_Handle h, uint32_t *pSaSamples);
BERR_Code BWFE_g3_P_MirrorEquRefTaps(BWFE_ChannelHandle h, BWFE_ChannelHandle hRef);
BERR_Code BWFE_g3_P_CalibrateAdcPhase(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_P_DisableTimer_isr(BWFE_ChannelHandle h, BWFE_g3_TimerSelect t);
BERR_Code BWFE_g3_P_EnableTimer_isr(BWFE_ChannelHandle h, BWFE_g3_TimerSelect t, uint32_t delay_usec, BWFE_FUNCT func);
void BWFE_g3_P_Timer0_isr(void *p, int param);
void BWFE_g3_P_Timer1_isr(void *p, int param);

/* bwfe_g3_priv_adc.c */
BERR_Code BWFE_g3_Adc_P_PowerUp(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_PowerDown(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_CalibratePhase(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_CalibratePhase1(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_Accumulate(BWFE_ChannelHandle h, uint8_t slice, uint8_t lane, uint8_t interval, int32_t *acc);
BERR_Code BWFE_g3_Adc_P_CalibrateINL(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_EqualizePipeline(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_EqualizeMdac(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_EqualizeMdac1(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_ResetLut(BWFE_ChannelHandle h, uint8_t slice, uint8_t mdac);
BERR_Code BWFE_g3_Adc_P_ResetLutAll(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_ResetEquTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_ReadEquTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Adc_P_WriteEquTaps(BWFE_ChannelHandle h);

/* bwfe_g3_priv_corr.c */
void BWFE_g3_Corr_P_CorrDone_isr(void *p, int param);
BERR_Code BWFE_g3_Corr_P_StartCorrelator(BWFE_ChannelHandle h, uint32_t freqHz, uint32_t binSizeHz, BWFE_FUNCT func);
#ifndef BWFE_EXCLUDE_ANALOG_DELAY
BERR_Code BWFE_g3_Corr_P_CalcDelay(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Corr_P_CoarseAdjust(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Corr_P_FineAdjust(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Corr_P_FineAdjust1(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Corr_P_CompensateDelay(BWFE_ChannelHandle h, uint32_t adjRight, uint32_t adjLeft);
#endif
#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
BERR_Code BWFE_g3_Corr_P_ScanSpectrum(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Corr_P_ScanSpectrum1(BWFE_ChannelHandle h);
#endif

/* bwfe_g3_priv_lic.c */
BERR_Code BWFE_g3_Lic_P_PowerUp(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Lic_P_NrDcoTracking1(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Lic_P_NrDcoTracking2(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Lic_P_PowerDown(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Lic_P_ResetTaps(BWFE_ChannelHandle h);
#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
BERR_Code BWFE_g3_Lic_P_ReadTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Lic_P_WriteTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Lic_P_ComputeTaps(BWFE_ChannelHandle h);
#endif

/* bwfe_g3_priv_rffe.c */
BERR_Code BWFE_g3_Rffe_P_PowerUp(BWFE_ChannelHandle h);
BERR_Code BWFE_g3_Rffe_P_PowerDown(BWFE_ChannelHandle h);


#endif /* BWFE_g3_PRIV_H__ */
