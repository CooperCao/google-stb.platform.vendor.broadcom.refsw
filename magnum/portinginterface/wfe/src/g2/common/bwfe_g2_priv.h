/***************************************************************************
 *     Copyright (c) 2005-2013, Broadcom Corporation
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
 
#ifndef BWFE_G2_PRIV_H__
#define BWFE_G2_PRIV_H__

#include "bwfe.h"
#include "bwfe_g2.h"

#if (BCHP_CHIP==4550)
   #include "bwfe_4550_priv.h"
   #define BWFE_HAS_RFFE
   #define BWFE_EXCLUDE_ANALOG_DELAY
   //#define BWFE_EXCLUDE_SPECTRUM_ANALYZER
   #define BWFE_EXCLUDE_LIC_TAP_COMPUTER
#elif (BCHP_CHIP==4538)
   #include "bwfe_4538_priv.h"
   //#define BWFE_EXCLUDE_SPECTRUM_ANALYZER
   //#define BWFE_EXCLUDE_LIC_TAP_COMPUTER
   #define BWFE_RUN_ADC_PHASE_CAL_AT_STARTUP
   #define BWFE_USE_ADC_PHASE_CAL_V2
#else
   #error "unsupported BCHP_CHIP"
#endif


#define BWFE_G2_RELEASE_VERSION 1


struct BWFE_P_ChannelHandle;

typedef BERR_Code (*BWFE_FUNCT)(BWFE_ChannelHandle);


/******************************************************************************
Summary:
   This is the structure for the chip-specific component of the BWFE_Handle. 
******************************************************************************/
typedef struct BWFE_g2_P_Handle
{
   BREG_Handle                   hRegister;           /* register handle */
   BINT_Handle                   hInterrupt;          /* interrupt handle */
   BKNI_EventHandle              hSaDoneEvent;        /* spectrum scan done event handle */
   uint32_t                      adcSampleFreqKhz;    /* ADC rx clock in kHz */
   uint32_t                      dpmPilotFreqKhz;     /* DPM frequency in kHz */
   uint32_t                      dpmQddfsM;           /* QDDFS M */
   uint32_t                      dpmQddfsN;           /* QDDFS N */
#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
   uint32_t                      saFreq;              /* spectrum analyzer current freq */
   uint32_t                      saStep;              /* spectrum analyzer step size */
   uint32_t                      saSamples[BWFE_SA_MAX_SAMPLES];  /* spectrum analyzer samples */
   uint16_t                      saIdx;               /* spectrum analyzer sample index */
   uint8_t                       saState;             /* current state in spectrum analyzer state machine */
   uint8_t                       saSweep;             /* spectrum analyzer sweep count */
#endif
} BWFE_g2_P_Handle;


/******************************************************************************
Summary:
   This is the structure for the chip-specific component of the BWFE_ChannelHandle. 
******************************************************************************/
typedef struct BWFE_g2_P_ChannelHandle
{
   BWFE_SpecAnalyzerParams saParams;      /* spectrum analyzer parameters */
   BKNI_EventHandle     hWfeReady;        /* wfe ready event */
   BINT_CallbackHandle  hTimer0Cb;        /* callback handle for timer0 interrupt */
   BINT_CallbackHandle  hTimer1Cb;        /* callback handle for timer1 interrupt */
   BINT_CallbackHandle  hCorrDoneCb;      /* callback handle for correlator done interrupt */
   BWFE_FUNCT           timer0Isr;        /* function for handling timer0 interrupt */
   BWFE_FUNCT           timer1Isr;        /* function for handling timer1 interrupt */
   BWFE_FUNCT           corrIdleIsr;      /* function for handling correlator idle interrupt */
   BWFE_FUNCT           postCalcDelayFunct; /* function to call after delay calculations */
   uint32_t             accSaturateCount; /* accumulator saturation count */
   uint32_t             lutSaturateCount; /* lut saturation count */
   uint32_t             rfAgcInt;                  /* rf agc integrator */
   uint32_t             dgsClipCtlPing;            /* DGS clip control */
   uint32_t             dgsClipCtlPong;            /* DGS clip control */
   uint32_t             dgsClipCountPing;          /* DGS clip count */
   uint32_t             dgsClipCountPong;          /* DGS clip count */
   uint32_t             equGainErrPing;            /* equalization gain error for ping lane */
   uint32_t             equGainErrPong;            /* equalization gain error for pong lane */
   uint32_t             corrFreqHz;                /* correlator freq in Hz */
#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
   uint32_t             licCoeff[BWFE_LIC_L1][BWFE_NUM_LIC_TAPS];    /* 2D array of LIC coeff (lanes x taps) */
#endif
#ifndef BWFE_EXCLUDE_ANALOG_DELAY
   uint32_t             corrAgc[BWFE_LIC_L];       /* 1D array of correlator loop filter agcs */
   int32_t              corrDelay[BWFE_LIC_L];     /* 1D array of correlator delays */
   int32_t              sliceDelay[BWFE_NUM_SLICES];  /* 1D array of slice delays */
   int32_t              analogDelay[BWFE_NUM_SLICES]; /* 1D array of delay compensation values */
#endif
   int32_t              calDelay[BWFE_NUM_ADC_PHASES][BWFE_NUM_SLICES];
#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
   int32_t              licDelay[BWFE_NUM_SLICES][BWFE_NUM_LANES];   /* 2D array of previous delay values used to calculate LIC2 taps (slices x ping-pong lanes) */
#endif
   int32_t              equCoeff[BWFE_NUM_SLICES][BWFE_NUM_LANES][BWFE_NUM_MDACS][BWFE_NUM_EQ_TAPS]; /* 4D array of EQ coeff (slices x lanes x mdacs x taps)*/
   uint16_t             equRuntimeMs;              /* digisum equalization run-time in ms */
   uint16_t             rfAgcThresh;               /* rf agc threshold */
   uint8_t              rfAgcK1;                   /* rf agc k1 */
   uint8_t              rfAgcBeta;                 /* rf agc beta */
   uint8_t              nrAgcK1;                   /* lic2 agc k1 */
   uint8_t              dgsClipSlice;              /* DGS clip slice select */
   uint8_t              equState;                  /* current state in mdac eq state machine */
   uint8_t              equCurrSlice;              /* current slice in mdac eq state machine */
   uint8_t              equCurrMdac;               /* current mdac in mdac eq state machine */
   uint8_t              equMainMu1;                /* digisum equalization main tap mu1 */
   uint8_t              equMainMu2;                /* digisum equalization main tap mu2 */
   uint8_t              equMu1;                    /* digisum equalization mu1 */
   uint8_t              equMu2;                    /* digisum equalization mu2 */
   uint8_t              equTapMask;                /* mask to select which eq taps to update */
   uint8_t              equMdacEnable;             /* select which mdac to run during equ */
   uint8_t              corrMaxBit;                /* 2^k accumulation limit based on corr length  */
   uint8_t              corrInputSelect;           /* correlator input select */
   uint8_t              corrDelayBeta;             /* correlator delay tracking beta in 0.05 steps */
   uint8_t              corrDpmDebug;              /* correlator dpm debug controls */
   uint8_t              corrRefSlice;              /* reference slice for correlator delay calculations */
   uint8_t              calState;                  /* current state in phase calibration */
   uint8_t              calCurrPhase;              /* current phase in phase calibration */
   bool                 bLicTrackingOn;            /* toggle LIC tracking mode */
   bool                 bCorrInProgress;           /* true if correlator is busy */
   bool                 bPhaseCalComplete;         /* true if phase calibration finished */
   bool                 bLicTapCalcComplete;       /* true if lic tap calculation finished */
} BWFE_g2_P_ChannelHandle;


/* bwfe_g2_priv.c */
BERR_Code BWFE_g2_P_Open(BWFE_Handle *h, BCHP_Handle hChip, void *pReg, BINT_Handle hInt, const BWFE_Settings *pDefSettings);
BERR_Code BWFE_g2_P_Close(BWFE_Handle h);
BERR_Code BWFE_g2_P_GetTotalChannels(BWFE_Handle h, BWFE_ChannelInfo *pInfo);
BERR_Code BWFE_g2_P_GetTotalRefChannels(BWFE_Handle h, uint8_t *totalRefChannels);
BERR_Code BWFE_g2_P_OpenChannel(BWFE_Handle h, BWFE_ChannelHandle *pChannelHandle, uint8_t chanNum, const BWFE_ChannelSettings *pSettings);
BERR_Code BWFE_g2_P_CloseChannel(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_Reset(BWFE_Handle h);
BERR_Code BWFE_g2_P_GetVersion(BWFE_Handle h, BFEC_VersionInfo *pVersion);
BERR_Code BWFE_g2_P_IsInputEnabled(BWFE_ChannelHandle h, bool *bEnabled);
BERR_Code BWFE_g2_P_EnableInput(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_DisableInput(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_GetChannelStatus(BWFE_ChannelHandle h, BWFE_ChannelStatus *pStatus);
BERR_Code BWFE_g2_P_GetLicTaps(BWFE_ChannelHandle h, uint32_t **pLicCoeff);
BERR_Code BWFE_g2_P_GetEqTaps(BWFE_ChannelHandle h, uint32_t ****pEqCoeff);
BERR_Code BWFE_g2_P_FreezeRfAgc(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_UnfreezeRfAgc(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_RunLaneCorrection(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_CalibrateINL(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_EqualizePipeline(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_SetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t freqKhz);
BERR_Code BWFE_g2_P_GetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t *freqKhz);
BERR_Code BWFE_g2_P_ReadConfig(BWFE_ChannelHandle h, uint16_t id, uint8_t *p, uint8_t len);
BERR_Code BWFE_g2_P_WriteConfig(BWFE_ChannelHandle h, uint16_t id, uint8_t *p, uint8_t len);
BERR_Code BWFE_g2_P_UpdateConfig(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_ResetDgsLut(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_ResetEqTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_ResetLicTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_CancelDCOffset(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_GetWfeReadyEventHandle(BWFE_ChannelHandle h, BKNI_EventHandle *hWfeReady);
BERR_Code BWFE_g2_P_CalibrateAnalogDelay(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_GetAnalogDelay(BWFE_ChannelHandle h, uint32_t *pCorrDelay);
BERR_Code BWFE_g2_P_GetSaDoneEventHandle(BWFE_Handle h, BKNI_EventHandle *hSaDoneEvent);
BERR_Code BWFE_g2_P_ScanSpectrum(BWFE_ChannelHandle h, BWFE_SpecAnalyzerParams *pParams);
BERR_Code BWFE_g2_P_GetSaSamples(BWFE_Handle h, uint32_t *pSaSamples);
BERR_Code BWFE_g2_P_MirrorEquRefTaps(BWFE_ChannelHandle h, BWFE_ChannelHandle hRef);
BERR_Code BWFE_g2_P_CalibrateAdcPhase(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_P_DisableTimer(BWFE_ChannelHandle h, BWFE_g2_TimerSelect t);
BERR_Code BWFE_g2_P_EnableTimer(BWFE_ChannelHandle h, BWFE_g2_TimerSelect t, uint32_t delay_usec, BWFE_FUNCT func);
void BWFE_g2_P_Timer0_isr(void *p, int param);
void BWFE_g2_P_Timer1_isr(void *p, int param);

/* bwfe_g2_priv_adc.c */
bool BWFE_g2_Adc_P_IsAdcOn(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Adc_P_PowerUp(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Adc_P_PowerDown(BWFE_ChannelHandle h);
#ifdef BWFE_USE_ADC_PHASE_CAL_V2
BERR_Code BWFE_g2_Adc_P_FindUnusedChan(BWFE_ChannelHandle h, uint8_t *unusedChan);
BERR_Code BWFE_g2_Adc_P_ConfigChan(BWFE_ChannelHandle h, uint8_t chan);
BERR_Code BWFE_g2_Adc_P_CalibratePhaseV2(BWFE_ChannelHandle h, uint8_t chan);
#else
BERR_Code BWFE_g2_Adc_P_CalibratePhase(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Adc_P_CalibratePhase1(BWFE_ChannelHandle h);
#endif
BERR_Code BWFE_g2_Adc_P_Accumulate(BWFE_ChannelHandle h, uint8_t slice, uint8_t lane, uint8_t interval, int32_t *acc);
BERR_Code BWFE_g2_Adc_P_CalibrateINL(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Adc_P_EqualizePipeline(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Adc_P_EqualizeMdac(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Adc_P_EqualizeMdac1(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Adc_P_ResetLut(BWFE_ChannelHandle h, uint8_t slice, uint8_t mdac);
BERR_Code BWFE_g2_Adc_P_ResetLutAll(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Adc_P_ResetEquTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Adc_P_ReadEquTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Adc_P_WriteEquTaps(BWFE_ChannelHandle h);

/* bwfe_g2_priv_corr.c */
void BWFE_g2_Corr_P_CorrDone_isr(void *p, int param);
BERR_Code BWFE_g2_Corr_P_StartCorrelator(BWFE_ChannelHandle h, uint32_t freqHz, uint32_t binSizeHz, BWFE_FUNCT func);
#ifndef BWFE_EXCLUDE_ANALOG_DELAY
BERR_Code BWFE_g2_Corr_P_CalcDelay(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Corr_P_AccumulateOffset(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Corr_P_CompensateDelay(BWFE_ChannelHandle h, int32_t *delay);
#endif
#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
BERR_Code BWFE_g2_Corr_P_ScanSpectrum(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Corr_P_ScanSpectrum1(BWFE_ChannelHandle h);
#endif

/* bwfe_g2_priv_lic.c */
BERR_Code BWFE_g2_Lic_P_PowerUp(BWFE_ChannelHandle h, bool bBypassLic, bool bEnableNrLic);
BERR_Code BWFE_g2_Lic_P_PowerDown(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Lic_P_ResetTaps(BWFE_ChannelHandle h);
#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
BERR_Code BWFE_g2_Lic_P_ReadTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Lic_P_WriteTaps(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Lic_P_ComputeTaps(BWFE_ChannelHandle h);
#endif

/* bwfe_g2_priv_rffe.c */
BERR_Code BWFE_g2_Rffe_P_PowerUp(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Rffe_P_PowerDown(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Rffe_P_EnableDpmMode(BWFE_ChannelHandle h);
BERR_Code BWFE_g2_Rffe_P_DisableDpmMode(BWFE_ChannelHandle h);


#endif /* BWFE_G2_PRIV_H__ */
