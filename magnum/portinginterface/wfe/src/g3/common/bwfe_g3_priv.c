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
 
#include "bstd.h"
#include "bint.h"
#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_g3_priv.h"

BDBG_MODULE(bwfe_g3_priv);


/******************************************************************************
 BWFE_g3_P_Open()
******************************************************************************/
BERR_Code BWFE_g3_P_Open(
   BWFE_Handle *h,      /* [out] BWFE handle */
   BCHP_Handle hChip,   /* [in] chip handle */
   void *pReg,          /* [in] pointer to register or i2c handle */
   BINT_Handle hInt,    /* [in] interrupt handle */
   const BWFE_Settings *pDefSettings   /* [in] default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_Handle hDev;
   BWFE_g3_P_Handle *hG3Dev;
   uint8_t i;
   
   BSTD_UNUSED(hChip);
   BDBG_ENTER(BWFE_g3_P_Open);
   
   /* allocate heap memory for device handle */
   hDev = (BWFE_Handle)BKNI_Malloc(sizeof(BWFE_P_Handle));
   BDBG_ASSERT(hDev);
   BKNI_Memset((void*)hDev, 0, sizeof(BWFE_P_Handle));
   hG3Dev = (BWFE_g3_P_Handle *)BKNI_Malloc(sizeof(BWFE_g3_P_Handle));
   BDBG_ASSERT(hG3Dev);
   BKNI_Memset((void*)hG3Dev, 0, sizeof(BWFE_g3_P_Handle));
   hDev->pImpl = (void*)hG3Dev;
   
   /* allocate heap memory for channel handle pointer */
   hDev->pChannels = (BWFE_P_ChannelHandle **)BKNI_Malloc(BWFE_NUM_CHANNELS * sizeof(BWFE_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   BKNI_Memset((void*)hDev->pChannels, 0, BWFE_NUM_CHANNELS * sizeof(BWFE_P_ChannelHandle *));
   
   /* allocate heap memory for reference channel handle pointer */
   hDev->pRefChannels = (BWFE_P_ChannelHandle **)BKNI_Malloc(BWFE_NUM_REF_CHANNELS * sizeof(BWFE_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pRefChannels);
   BKNI_Memset((void*)hDev->pRefChannels, 0, BWFE_NUM_REF_CHANNELS * sizeof(BWFE_P_ChannelHandle *));
   
   /* initialize device handle */
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BWFE_Settings));
#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
   BKNI_Memset((void*)hG3Dev->saSamples, 0, BWFE_SA_MAX_SAMPLES);   /* clear sa samples */
   hG3Dev->saState = 0;
#endif

   hG3Dev->hChip = hChip;
   hG3Dev->hRegister = (BREG_Handle)pReg;
   hG3Dev->hInterrupt = hInt;

   /* create events */
   retCode = BKNI_CreateEvent(&(hG3Dev->hSaDoneEvent));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   
   hDev->totalChannels = BWFE_NUM_CHANNELS;
   for (i = 0; i < BWFE_NUM_CHANNELS; i++)
      hDev->pChannels[i] = NULL;
   
   hDev->totalRefChannels = BWFE_NUM_REF_CHANNELS;
#if (BWFE_NUM_REF_CHANNELS > 0)
   for (i = 0; i < BWFE_NUM_REF_CHANNELS; i++)
      hDev->pRefChannels[i] = NULL;
#endif
   
   *h = hDev;
   BDBG_LEAVE(BWFE_g3_P_Open);
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_Close()
******************************************************************************/
BERR_Code BWFE_g3_P_Close(BWFE_Handle h)
{
   BWFE_g3_P_Handle *hG3Dev;
   
   BDBG_ENTER(BWFE_g3_P_Close);
   
   /* clean up events */
   hG3Dev = (BWFE_g3_P_Handle *)(h->pImpl);
   BKNI_DestroyEvent(hG3Dev->hSaDoneEvent);
   
   /* free handles */
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pRefChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

   BDBG_LEAVE(BWFE_g3_P_Close);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_GetTotalChannels()
******************************************************************************/
BERR_Code BWFE_g3_P_GetTotalChannels(
   BWFE_Handle h,             /* [in] BWFE handle */
   BWFE_ChannelInfo  *pInfo   /* [out] channel availability info */
)
{
   BSTD_UNUSED(h);
   BDBG_ENTER(BWFE_g3_P_GetTotalChannels);
   
   pInfo->maxChannels = BWFE_NUM_CHANNELS;
   pInfo->numChannels = BWFE_NUM_CHANNELS;
   pInfo->availChannelsMask = BWFE_CHANNELS_MASK;
   
   BDBG_LEAVE(BWFE_g3_P_GetTotalChannels);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_GetTotalRefChannels()
******************************************************************************/
BERR_Code BWFE_g3_P_GetTotalRefChannels(
   BWFE_Handle h,                /* [in] BWFE handle */
   uint8_t     *totalRefChannels /* [out] number of channels supported */
)
{
   BSTD_UNUSED(h);
   BDBG_ENTER(BWFE_g3_P_GetTotalRefChannels);
   
   *totalRefChannels = BWFE_NUM_REF_CHANNELS;
   
   BDBG_LEAVE(BWFE_g3_P_GetTotalRefChannels);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_OpenChannel()
******************************************************************************/
BERR_Code BWFE_g3_P_OpenChannel(
   BWFE_Handle                h,               /* [in] BWFE handle */
   BWFE_ChannelHandle         *pChannelHandle, /* [out] BWFE channel handle */
   uint8_t                    chanNum,         /* [in] channel number */
   const BWFE_ChannelSettings *pSettings       /* [in] channel settings */
)
{
   extern const uint32_t BWFE_g3_ChannelIntrID[BWFE_NUM_CHANNELS][BWFE_g3_MaxIntID];
   extern const uint32_t BWFE_g3_RefChannelIntrID[BWFE_NUM_REF_CHANNELS][BWFE_g3_MaxIntID];
   
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_ChannelSettings chnSettings;
   BWFE_ChannelHandle chnHandle;
   BWFE_g3_P_ChannelHandle *chG3;
   BWFE_g3_P_Handle *hDev = h->pImpl;
   
   BDBG_ENTER(BWFE_g3_P_OpenChannel);
   
   /* set channel settings */
   if (pSettings == NULL)
      BWFE_GetChannelDefaultSettings(h, chanNum, &chnSettings);
   else
      chnSettings = *pSettings;
   
   /* check channel index */
#if (BWFE_NUM_REF_CHANNELS > 0)
   if ((chnSettings.bReference) && (chanNum >= BWFE_NUM_REF_CHANNELS))
      return BERR_INVALID_PARAMETER;
#endif
   if (chanNum >= BWFE_NUM_CHANNELS)
      return BERR_INVALID_PARAMETER;
   
   /* allocate heap memory for the channel handle */
   chnHandle = (BWFE_ChannelHandle)BKNI_Malloc(sizeof(BWFE_P_ChannelHandle));
   BDBG_ASSERT(chnHandle);
   BKNI_Memset((void*)chnHandle, 0, sizeof(BWFE_P_ChannelHandle));
   chG3 = (BWFE_g3_P_ChannelHandle *)BKNI_Malloc(sizeof(BWFE_g3_P_ChannelHandle));
   BDBG_ASSERT(chG3);
   BKNI_Memset((void*)chG3, 0, sizeof(BWFE_g3_P_ChannelHandle));
   
   /* initialize channel handle */
   BKNI_Memcpy((void*)(&(chnHandle->settings)), (void*)&chnSettings, sizeof(BWFE_ChannelSettings));
   chnHandle->pDevice = h;
#ifndef BWFE_EXCLUDE_ANALOG_DELAY
   BKNI_Memset((void*)chG3->corrAgc, 0, BWFE_LIC_L);                          /* clear correlator AGC values */
   BKNI_Memset((void*)chG3->corrDelay, 0, BWFE_LIC_L);                        /* clear correlator delays */
   BKNI_Memset((void*)chG3->sliceDelay, 0, BWFE_NUM_SLICES);                  /* clear slice delays */
#endif
#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
   BKNI_Memset((void*)chG3->licCoeff, 0, BWFE_NUM_LANES * BWFE_NUM_LIC_TAPS); /* clear LIC coefficients */
   BKNI_Memset((void*)chG3->licDelay, 0, BWFE_NUM_SLICES * BWFE_NUM_LANES);   /* clear previous LIC tap delays */
#endif
   BKNI_Memset((void*)chG3->equCoeff, 0, BWFE_NUM_SLICES * BWFE_NUM_LANES * BWFE_NUM_MDACS * BWFE_NUM_EQ_TAPS); /* clear EQ coefficients */
   
   chnHandle->channel = chanNum;
   chnHandle->bReference = chnSettings.bReference;
   chnHandle->bEnabled = false;
   chG3->dpmPilotFreqKhz = BWFE_DEF_FC_DPM_KHZ;
   chG3->dpmQddfsM = 0;
   chG3->dpmQddfsN = 0;
   chG3->accSaturateCount = 0;
   chG3->lutSaturateCount = 0;
   chG3->rfAgcInt = 0;
   chG3->dgsClipCtlPing = 0;
   chG3->dgsClipCtlPong = 0;
   chG3->dgsClipCountPing = 0;
   chG3->dgsClipCountPong = 0;
#ifdef BWFE_HYBRID_ADC
   chG3->equRuntimeMs = 20;
   chG3->equTapMask = 0x6;
   chG3->equMdacEnable = 0x1;    /* run mdac1 only */
#else
   chG3->equRuntimeMs = 500;
   chG3->equTapMask = 0xF;
   chG3->equMdacEnable = 0x3;    /* run both mdac2 and mdac1 */
#endif
   chG3->rfAgcThresh = 0xD800;
   chG3->rfAgcK1 = 8;
   chG3->rfAgcBeta = 7;
   chG3->nrAgcK1 = 4;
   chG3->dgsClipSlice = 0;
#ifdef BWFE_HYBRID_ADC
   chG3->equState = 6;  /* final mdac state */
#else
   chG3->equState = 4;  /* final mdac state */
#endif
   chG3->equMainMu1 = 0; /* TBD */
   chG3->equMainMu2 = 0; /* TBD */
   chG3->equMu1 = 0;     /* TBD */
   chG3->equMu2 = 0;     /* TBD */
   chG3->corrMaxBit = 23;
   chG3->corrInputSelect = 0;    /* 0:mdac, 1:dco, 2:agc, 3:lic */
   chG3->corrDelayBeta = 40;     /* beta in 0.05 steps */
   chG3->corrDpmDebug = 0x2;     /* corrDpmDebug[0]: 0=normal corr update, 1=disable corr update corr; corrDpmDebug[2:1]: 0=dpm pulse, 1=on, 2=off */
   chG3->corrRefSlice = 0;       /* use slice0 as default reference slice */
   chG3->bLicTrackingOn = false; /*TBD LIC tracking off for now */
   chG3->bLicReady = false;      /* LIC NR is initially not converged */
   
   /* initialize chip specific parameters e.g. adc sample rate */
   chnHandle->pImpl = (void*)chG3;
   retCode = BWFE_P_InitConfig(chnHandle);

#if (BCHP_CHIP==7364)
   /* acquire aif clock for interrupts */
   BWFE_P_InitAdc(chnHandle);
#endif

   /* setup interrupts */
   if (chnSettings.bReference)
   {
   #if (BWFE_NUM_REF_CHANNELS > 0)
      /* reference channel */
      chnHandle->totalSlices = BWFE_NUM_REF_SLICES;
      retCode = BINT_CreateCallback(&(chG3->hTimer0Cb), hDev->hInterrupt, 
                     BWFE_g3_RefChannelIntrID[chanNum][BWFE_g3_IntID_eTimer0], 
                     BWFE_g3_P_Timer0_isr, (void*)chnHandle, 0);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      
      retCode = BINT_CreateCallback(&(chG3->hTimer1Cb), hDev->hInterrupt, 
                     BWFE_g3_RefChannelIntrID[chanNum][BWFE_g3_IntID_eTimer1], 
                     BWFE_g3_P_Timer1_isr, (void*)chnHandle, 0);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      
      /* no corr_done interrupt on mdac_cal_core */
   #endif
   }
   else
   {
      /* regular wfe channel */
      chnHandle->totalSlices = BWFE_NUM_SLICES;
      retCode = BINT_CreateCallback(&(chG3->hTimer0Cb), hDev->hInterrupt, 
                     BWFE_g3_ChannelIntrID[chanNum][BWFE_g3_IntID_eTimer0], 
                     BWFE_g3_P_Timer0_isr, (void*)chnHandle, 0);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      
      retCode = BINT_CreateCallback(&(chG3->hTimer1Cb), hDev->hInterrupt, 
                     BWFE_g3_ChannelIntrID[chanNum][BWFE_g3_IntID_eTimer1], 
                     BWFE_g3_P_Timer1_isr, (void*)chnHandle, 0);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
      
      retCode = BINT_CreateCallback(&(chG3->hCorrDoneCb), hDev->hInterrupt, 
                       BWFE_g3_ChannelIntrID[chanNum][BWFE_g3_IntID_eCorrDone], 
                       BWFE_g3_Corr_P_CorrDone_isr, (void*)chnHandle, 0);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }

#if (BCHP_CHIP==7364)
   /* release aif clock for interrupts */
   BWFE_P_ShutdownAdc(chnHandle);
#endif

   /* create events */
   retCode = BKNI_CreateEvent(&(chG3->hWfeReady));
   BDBG_ASSERT(retCode == BERR_SUCCESS);
   
   if (chnSettings.bReference)
      h->pRefChannels[chanNum] = chnHandle;
   else
      h->pChannels[chanNum] = chnHandle;
   
   *pChannelHandle = chnHandle;
   BDBG_LEAVE(BWFE_g3_P_OpenChannel);
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_CloseChannel()
******************************************************************************/
BERR_Code BWFE_g3_P_CloseChannel(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *chG3;
   BERR_Code retCode = BERR_SUCCESS;
   
   BDBG_ENTER(BWFE_g3_P_CloseChannel);
   
   /* disable interrupts */
   BWFE_g3_P_DisableChannelInterrupts(h);
   
   /* clean up events */
   chG3 = (BWFE_g3_P_ChannelHandle *)(h->pImpl);
   BKNI_DestroyEvent(chG3->hWfeReady);
   
   /* clean up callbacks */
   BINT_DestroyCallback(chG3->hTimer0Cb);
   BINT_DestroyCallback(chG3->hTimer1Cb);
   if (chG3->hCorrDoneCb)
      BINT_DestroyCallback(chG3->hCorrDoneCb);
   
   /* free channel handle */
   BKNI_Free((void*)h->pImpl); 
   BKNI_Free((void*)h);
   h = NULL;
   
   BDBG_LEAVE(BWFE_g3_P_CloseChannel);
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_ResetChannel()
******************************************************************************/
BERR_Code BWFE_g3_P_ResetChannel(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;

   BWFE_g3_Adc_P_PowerUp(h);        /* power up ADC */
   BWFE_g3_Rffe_P_PowerDown(h);     /* power down RFFE */
   BWFE_P_DisableDpmPilot(h);       /* gate off DPM by default */
   BWFE_g3_Lic_P_PowerUp(h);        /* power up LIC */

#ifndef BWFE_BYPASS_EQU
   /* run pipeline equalization, mdac equ will power up RFFE when finished */
   retCode = BWFE_g3_Adc_P_EqualizeMdac(h);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("pipeline equalization error %X", retCode));
      return retCode;
   }
#else
   BWFE_g3_Rffe_P_PowerUp(h);    /* power up RFFE */
#endif

   /* apply config parameters */
   BWFE_g3_P_UpdateChannelConfig(h);
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_Reset()
******************************************************************************/
BERR_Code BWFE_g3_P_Reset(BWFE_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_g3_P_ChannelHandle *hChn;
   uint32_t freqKhz;
   uint8_t i;
   
   BDBG_ENTER(BWFE_g3_P_Reset);

   for (i = 0; i < h->totalChannels; i++)
   {
      if (h->pChannels[i] != NULL)
      {
      #if (BCHP_CHIP==7364)
         /* acquire aif clock for reset */
         BWFE_P_InitAdc(h->pChannels[i]);
      #endif

      #if 0
         /* configure dpm pilot frequency */
         BWFE_P_SetDpmPilotFreq(h->pChannels[i], BWFE_DEF_FC_DPM_KHZ);
         BWFE_P_GetDpmPilotFreq(h->pChannels[i], &freqKhz);
         if (freqKhz != BWFE_DEF_FC_DPM_KHZ)
         {
            BDBG_WRN(("DPM%d frequency mismatch: %d != %d", i, freqKhz, BWFE_DEF_FC_DPM_KHZ));
         }
      #endif

         /* configure adc sample clock */
         hChn = (BWFE_g3_P_ChannelHandle*)h->pChannels[i]->pImpl;
         BWFE_P_SetAdcSampleFreq(h->pChannels[i], hChn->adcSampleFreqKhz);
         BWFE_P_GetAdcSampleFreq(h->pChannels[i], &freqKhz);
         if (freqKhz != hChn->adcSampleFreqKhz)
         {
            BDBG_WRN(("ADC%d Fs mismatch: %d != %d", i, freqKhz, hChn->adcSampleFreqKhz));
         }

         /* power off input by default */
         BWFE_g3_P_DisableInput(h->pChannels[i]);
      }
   }
   
#if !defined(BWFE_BYPASS_EQU) && !defined(BWFE_HYBRID_ADC)
   /* init reference channels */
   for (i = 0; i < h->totalRefChannels; i++)
   {
      if (h->pRefChannels[i] != NULL)
      {
         /* perform reset logic for each reference ADC if opened */
         BWFE_g3_P_ResetChannel(h->pRefChannels[i]);
      }
   }
#endif
   
   BDBG_LEAVE(BWFE_g3_P_Reset);
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_GetVersion()
******************************************************************************/
BERR_Code BWFE_g3_P_GetVersion(BWFE_Handle h, BFEC_VersionInfo *pVersion)
{  
   BSTD_UNUSED(h);
   BDBG_ENTER(BWFE_g3_P_GetVersion);
   
   pVersion->majorVersion = BWFE_API_VERSION;
   pVersion->minorVersion = BWFE_G3_RELEASE_VERSION;
   pVersion->buildType = BWFE_G3_BUILD_VERSION;
   pVersion->buildId = 0;
   
   BDBG_LEAVE(BWFE_g3_P_GetVersion);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_IsInputEnabled()
******************************************************************************/
BERR_Code BWFE_g3_P_IsInputEnabled(BWFE_ChannelHandle h, bool *bEnabled)
{
   /* return adc power state */
   *bEnabled = BWFE_P_IsAdcOn(h);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_EnableInput()
******************************************************************************/
BERR_Code BWFE_g3_P_EnableInput(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BDBG_ENTER(BWFE_g3_P_EnableInput);
   
   /* check if adc already enabled */
   if (BWFE_P_IsAdcOn(h))
   {
      BKNI_SetEvent(hChn->hWfeReady);
      return BERR_SUCCESS;
   }

   /* reset statuses */
   hChn->bCorrInProgress = false;
   hChn->bLicReady = false;

   /* call channel reset sequence */
   BWFE_g3_P_ResetChannel(h);
   
   BDBG_LEAVE(BWFE_g3_P_EnableInput);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_DisableChannelInterrupts() - non-ISR context
******************************************************************************/
BERR_Code BWFE_g3_P_DisableChannelInterrupts(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;

   /* disable timers */
   BKNI_EnterCriticalSection();
   BWFE_g3_P_DisableTimer(h, BWFE_g3_TimerSelect_e0);
   BWFE_g3_P_DisableTimer(h, BWFE_g3_TimerSelect_e1);
   BKNI_LeaveCriticalSection();
   
   if (!h->bReference)
   {
      /* disable corr interrupt if not reference channel */
      BINT_DisableCallback(hChn->hCorrDoneCb);
      BINT_ClearCallback(hChn->hCorrDoneCb);
   }
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_DisableInput()
******************************************************************************/
BERR_Code BWFE_g3_P_DisableInput(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;

   BDBG_ENTER(BWFE_g3_P_DisableInput);

   if (!BWFE_P_IsAdcOn(h))
      return BERR_SUCCESS;

   /* disable channel interrupts */
   BWFE_g3_P_DisableChannelInterrupts(h);

   /* reset mdac state */
#ifdef BWFE_HYBRID_ADC
   hChn->equState = 6;
#else
   hChn->equState = 4;
#endif

   BWFE_g3_Rffe_P_PowerDown(h);  /* power down RFFE */
   BWFE_g3_Lic_P_PowerDown(h);   /* power down LIC */
   BWFE_g3_Adc_P_PowerDown(h);   /* power down ADC */

   BDBG_LEAVE(BWFE_g3_P_DisableInput);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_GetChannelStatus()
******************************************************************************/
BERR_Code BWFE_g3_P_GetChannelStatus(BWFE_ChannelHandle h, BWFE_ChannelStatus *pStatus)
{
   BWFE_g3_P_Handle *hDev = h->pDevice->pImpl;
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;
   
   BDBG_ENTER(BWFE_g3_P_GetChannelStatus);
   BSTD_UNUSED(hDev);
   BSTD_UNUSED(hChn);
   
   pStatus->bEnabled = BWFE_P_IsAdcOn(h);

   BWFE_P_ReadRegister(h, BCHP_WFE_CORE_AGCCTL1, &val);
   pStatus->bAgcFrozen = (val & 0x00020000) ? true : false;

   /* unused */
   pStatus->freqOffset = 0;
   pStatus->freqLower = 0;
   pStatus->freqUpper = 0;
   pStatus->accSaturateCount = 0;
   pStatus->lutSaturateCount = 0;
   
   BDBG_LEAVE(BWFE_g3_P_GetChannelStatus);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_GetLicTaps() - input 2D array (lanes x taps)
******************************************************************************/
BERR_Code BWFE_g3_P_GetLicTaps(BWFE_ChannelHandle h, uint32_t **pLicCoeff)
{
#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   
   BDBG_ENTER(BWFE_g3_P_GetLicTaps);
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   /* copy lic coefficients to 2D array (lanes x taps) */
   BKNI_Memcpy(pLicCoeff, hChn->licCoeff, sizeof(hChn->licCoeff));
   
   BDBG_LEAVE(BWFE_g3_P_GetLicTaps);
   return BERR_SUCCESS;
#else
   BSTD_UNUSED(h);
   BSTD_UNUSED(pLicCoeff);
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BWFE_g3_P_GetEqTaps() - input 3D array (slices x mdacs x taps)
******************************************************************************/
BERR_Code BWFE_g3_P_GetEqTaps(BWFE_ChannelHandle h, uint32_t ****pEqCoeff)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   
   BDBG_ENTER(BWFE_g3_P_GetEqTaps);
   
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   /* refresh equalizer taps */
   BWFE_g3_Adc_P_ReadEquTaps(h);
   
   /* copy equalizer coefficients to 4D array (slices x lanes x mdacs x taps) */
   BKNI_Memcpy(pEqCoeff, hChn->equCoeff, sizeof(hChn->equCoeff));
   
   BDBG_LEAVE(BWFE_g3_P_GetEqTaps);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_FreezeRfAgc()
******************************************************************************/
BERR_Code BWFE_g3_P_FreezeRfAgc(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BWFE_g3_P_FreezeRfAgc);
   
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   /* freeze RFAGC */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_AGCCTL1, 0x04020000);
   
   BDBG_LEAVE(BWFE_g3_P_FreezeRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_UnfreezeRfAgc()
******************************************************************************/
BERR_Code BWFE_g3_P_UnfreezeRfAgc(BWFE_ChannelHandle h)
{
   BDBG_ENTER(BWFE_g3_P_UnfreezeRfAgc);
   
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   /* unfreeze RFAGC */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x04020000);
   
   BDBG_LEAVE(BWFE_g3_P_UnfreezeRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_RunLaneCorrection()
******************************************************************************/
BERR_Code BWFE_g3_P_RunLaneCorrection(BWFE_ChannelHandle h)
{
#ifndef BWFE_EXCLUDE_LIC_TAP_COMPUTER
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   return BWFE_g3_Lic_P_ComputeTaps(h);
#else
   BSTD_UNUSED(h);
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BWFE_g3_P_CalibrateINL()
******************************************************************************/
BERR_Code BWFE_g3_P_CalibrateINL(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_NOT_SUPPORTED;
   
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   /* TBD... */
   /* RF off, clocks on for INL calibration */
   BWFE_g3_Rffe_P_PowerDown(h);
   /*retCode = BWFE_Adc_P_CalibrateINLv2(h);*/
   BWFE_g3_Rffe_P_PowerUp(h);
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_EqualizePipeline()
******************************************************************************/
BERR_Code BWFE_g3_P_EqualizePipeline(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   /* run pipeline equalization */
   retCode = BWFE_g3_Adc_P_EqualizeMdac(h);
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_SetAdcSampleFreq()
******************************************************************************/
BERR_Code BWFE_g3_P_SetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t freqKhz)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t newFreqKhz;
   
   retCode = BWFE_P_SetAdcSampleFreq(h, freqKhz);
   if (retCode)
   {
      /* error setting sample rate */
      BDBG_ERR(("BWFE_P_SetAdcSampleFreq(%d) error 0x%X", freqKhz, retCode));
   }
   else
   {
      /* update new sample freq */
      BWFE_P_GetAdcSampleFreq(h, &newFreqKhz);
      hChn->adcSampleFreqKhz = newFreqKhz;
   }

   return retCode;
}


/******************************************************************************
 BWFE_g3_P_GetAdcSampleFreq()
******************************************************************************/
BERR_Code BWFE_g3_P_GetAdcSampleFreq(BWFE_ChannelHandle h, uint32_t *freqKhz)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   
   retCode = BWFE_P_GetAdcSampleFreq(h, freqKhz);
   hChn->adcSampleFreqKhz = *freqKhz;
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_GetChannelConfig()
******************************************************************************/
BERR_Code BWFE_g3_P_GetChannelConfig(BWFE_ChannelHandle h, uint16_t id, uint8_t *p, uint8_t len)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   
   BDBG_ENTER(BWFE_g3_P_ReadConfig);
   
   if (!BWFE_P_IsAdcOn(h))
      retCode = BWFE_ERR_POWERED_DOWN;
   
   if ((p == NULL) || (len != 4))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   
   /* initialize data to zero */
   p[0] = p[1] = p[2] = p[3] = 0;
   
   switch (id)
   {
      case BWFE_G3_CONFIG_RF_AGC_THRESH:
         p[0] = (uint8_t)(hChn->rfAgcThresh >> 8);
         p[1] = (uint8_t)(hChn->rfAgcThresh & 0xFF);           
         break;
      
      case BWFE_G3_CONFIG_RF_AGC_K1:
         *p = hChn->rfAgcK1;
         break;
      
      case BWFE_G3_CONFIG_RF_AGC_BETA:
         *p = hChn->rfAgcBeta;
         break;
      
      case BWFE_G3_CONFIG_RF_AGC_INT:
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_ReadRegister(h, BCHP_WFE_CORE_REG_AGC_LF_INT_WDATA, &hChn->rfAgcInt);
         p[0] = (uint8_t)(hChn->rfAgcInt >> 24);
         p[1] = (uint8_t)(hChn->rfAgcInt >> 16);
         p[2] = (uint8_t)(hChn->rfAgcInt >> 8);
         p[3] = (uint8_t)(hChn->rfAgcInt & 0xFF);
         break;
      
      case BWFE_G3_CONFIG_NR_AGC_K1:
         *p = hChn->nrAgcK1;
         break;
      
      case BWFE_G3_CONFIG_EQU_RUNTIME_MS:
         p[0] = (uint8_t)(hChn->equRuntimeMs >> 8);
         p[1] = (uint8_t)(hChn->equRuntimeMs & 0xFF);           
         break;
      
      case BWFE_G3_CONFIG_EQU_MAIN_MU1:
         *p = hChn->equMainMu1;
         break;
      
      case BWFE_G3_CONFIG_EQU_MAIN_MU2:
         *p = hChn->equMainMu2;
         break;
      
      case BWFE_G3_CONFIG_EQU_MU1:
         *p = hChn->equMu1;
         break;
      
      case BWFE_G3_CONFIG_EQU_MU2:
         *p = hChn->equMu2;
         break;
      
      case BWFE_G3_CONFIG_EQU_TAP_MASK:
         *p = hChn->equTapMask;
         break;
      
      case BWFE_G3_CONFIG_DGS_CLIP_SLICE_SELECT:
         *p = hChn->dgsClipSlice;
         break;
      
      case BWFE_G3_CONFIG_DGS_CLIP_CTL_PI:
         p[0] = (uint8_t)(hChn->dgsClipCtlPing >> 24);
         p[1] = (uint8_t)(hChn->dgsClipCtlPing >> 16);
         p[2] = (uint8_t)(hChn->dgsClipCtlPing >> 8);
         p[3] = (uint8_t)(hChn->dgsClipCtlPing & 0xFF);
         break;
      
      case BWFE_G3_CONFIG_DGS_CLIP_CTL_PO:
         p[0] = (uint8_t)(hChn->dgsClipCtlPong >> 24);
         p[1] = (uint8_t)(hChn->dgsClipCtlPong >> 16);
         p[2] = (uint8_t)(hChn->dgsClipCtlPong >> 8);
         p[3] = (uint8_t)(hChn->dgsClipCtlPong & 0xFF);
         break;
      
      case BWFE_G3_CONFIG_DGS_CLIP_COUNT_PI:
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_ReadSliceRegister(h, hChn->dgsClipSlice, BCHP_WFE_CORE_DGSCLPCNT_PI_SLC, &hChn->dgsClipCountPing);
         p[0] = (uint8_t)(hChn->dgsClipCountPing >> 24);
         p[1] = (uint8_t)(hChn->dgsClipCountPing >> 16);
         p[2] = (uint8_t)(hChn->dgsClipCountPing >> 8);
         p[3] = (uint8_t)(hChn->dgsClipCountPing & 0xFF);
         break;
      
      case BWFE_G3_CONFIG_DGS_CLIP_COUNT_PO:
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_ReadSliceRegister(h, hChn->dgsClipSlice, BCHP_WFE_CORE_DGSCLPCNT_PO_SLC, &hChn->dgsClipCountPong);
         p[0] = (uint8_t)(hChn->dgsClipCountPong >> 24);
         p[1] = (uint8_t)(hChn->dgsClipCountPong >> 16);
         p[2] = (uint8_t)(hChn->dgsClipCountPong >> 8);
         p[3] = (uint8_t)(hChn->dgsClipCountPong & 0xFF);
         break;
      
      case BWFE_G3_CONFIG_EQU_MDAC_ENABLE:
         *p = hChn->equMdacEnable;
         break;
      
      case BWFE_G3_CONFIG_CORR_INPUT_SELECT:
         *p = hChn->corrInputSelect;
         break;
      
      case BWFE_G3_CONFIG_CORR_DELAY_BETA:
         *p = hChn->corrDelayBeta;
         break;
      
      case BWFE_G3_CONFIG_CORR_DPM_DEBUG:
         *p = hChn->corrDpmDebug;
         break;
      
      default:
         BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
         break;
   }
   
   BDBG_LEAVE(BWFE_g3_P_ReadConfig);
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_SetChannelConfig()
******************************************************************************/
BERR_Code BWFE_g3_P_SetChannelConfig(BWFE_ChannelHandle h, uint16_t id, uint8_t *p, uint8_t len)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t slice;
   
   BDBG_ENTER(BWFE_g3_P_WriteConfig);
   
   if (!BWFE_P_IsAdcOn(h))
      retCode = BWFE_ERR_POWERED_DOWN;
   
   if ((p == NULL) || (len != 4))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   
   switch (id)
   {
      case BWFE_G3_CONFIG_RF_AGC_THRESH:
         hChn->rfAgcThresh = (p[0] << 8) | p[1];
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x0000FFFF, hChn->rfAgcThresh);
         break;
      
      case BWFE_G3_CONFIG_RF_AGC_K1:
         hChn->rfAgcK1 = *p;
         hChn->rfAgcK1 &= 0xF;
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x003C0000, hChn->rfAgcK1 << 18);
         break;
      
      case BWFE_G3_CONFIG_RF_AGC_BETA:
         hChn->rfAgcBeta = *p;
         hChn->rfAgcBeta &= 0x7;
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x01C00000, hChn->rfAgcBeta << 22);
         break;
      
      case BWFE_G3_CONFIG_RF_AGC_INT:
         hChn->rfAgcInt = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_WriteRegister(h, BCHP_WFE_CORE_REG_AGC_LF_INT_WDATA, hChn->rfAgcInt);
         break;
      
      case BWFE_G3_CONFIG_NR_AGC_K1:
         hChn->nrAgcK1 = *p;
         hChn->nrAgcK1 &= 0xF;
         if (BWFE_P_IsAdcOn(h))
         {
            for (slice = 0; slice < h->totalSlices; slice++)
            {
               /* propagate bandwidth to ping and pong on all slices */
               BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_NRAGCCTL_PI_SLC, ~0x000000F0, hChn->nrAgcK1 << 4);
               BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_NRAGCCTL_PO_SLC, ~0x000000F0, hChn->nrAgcK1 << 4);
            }
         }
         break;
      
      case BWFE_G3_CONFIG_EQU_RUNTIME_MS:
         hChn->equRuntimeMs = (p[0] << 8) | p[1];
         if (hChn->equRuntimeMs > 5000)
            hChn->equRuntimeMs = 5000;  /* limit equalization run-time */
         break;
      
      case BWFE_G3_CONFIG_EQU_MAIN_MU1:
         hChn->equMainMu1 = *p;
         hChn->equMainMu1 &= 0x7;
         break;
      
      case BWFE_G3_CONFIG_EQU_MAIN_MU2:
         hChn->equMainMu2 = *p;
         hChn->equMainMu2 &= 0x7;
         break;
      
      case BWFE_G3_CONFIG_EQU_MU1:
         hChn->equMu1 = *p;
         hChn->equMu1 &= 0x7;
         break;
      
      case BWFE_G3_CONFIG_EQU_MU2:
         hChn->equMu2 = *p;
         hChn->equMu2 &= 0x7;
         break;
      
      case BWFE_G3_CONFIG_EQU_TAP_MASK:
         hChn->equTapMask = *p;
         hChn->equTapMask &= 0xF;
         break;
      
      case BWFE_G3_CONFIG_DGS_CLIP_SLICE_SELECT:
         hChn->dgsClipSlice = *p;
         break;
      
      case BWFE_G3_CONFIG_DGS_CLIP_CTL_PI:
         hChn->dgsClipCtlPing = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
         hChn->dgsClipCtlPing &= 0x1FFF0FFF;  /* mask out reserved bits */
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_WriteSliceRegister(h, hChn->dgsClipSlice, BCHP_WFE_CORE_DGSCLP_PI_SLC, hChn->dgsClipCtlPing);
         break;
      
      case BWFE_G3_CONFIG_DGS_CLIP_CTL_PO:
         hChn->dgsClipCtlPong = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
         hChn->dgsClipCtlPong &= 0x1FFF0FFF;  /* mask out reserved bits */
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_WriteSliceRegister(h, hChn->dgsClipSlice, BCHP_WFE_CORE_DGSCLP_PO_SLC, hChn->dgsClipCtlPong);
         break;
      
      case BWFE_G3_CONFIG_DGS_CLIP_COUNT_PI:
         hChn->dgsClipCountPing = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_WriteSliceRegister(h, hChn->dgsClipSlice, BCHP_WFE_CORE_DGSCLPCNT_PI_SLC, hChn->dgsClipCountPing);
         break;
      
      case BWFE_G3_CONFIG_DGS_CLIP_COUNT_PO:
         hChn->dgsClipCountPong = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
         if (BWFE_P_IsAdcOn(h))
            BWFE_P_WriteSliceRegister(h, hChn->dgsClipSlice, BCHP_WFE_CORE_DGSCLPCNT_PO_SLC, hChn->dgsClipCountPong);
         break;
      
      case BWFE_G3_CONFIG_EQU_MDAC_ENABLE:
         hChn->equMdacEnable = *p;
         hChn->equMdacEnable &= 0x3;
         break;
      
      case BWFE_G3_CONFIG_CORR_INPUT_SELECT:
         hChn->corrInputSelect = *p;
         hChn->corrInputSelect &= 0x3;
         break;
      
      case BWFE_G3_CONFIG_CORR_DELAY_BETA:
         hChn->corrDelayBeta = *p;
         break;
      
      case BWFE_G3_CONFIG_CORR_DPM_DEBUG:
         hChn->corrDpmDebug = *p;
         if (hChn->corrDpmDebug & 0x2)
            BWFE_P_EnableDpmPilot(h);
         else if (hChn->corrDpmDebug & 0x4)
            BWFE_P_DisableDpmPilot(h);
         break;
      
      default:
         BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
         break;
   }
   
   BDBG_LEAVE(BWFE_g3_P_WriteConfig);
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_UpdateChannelConfig()
******************************************************************************/
BERR_Code BWFE_g3_P_UpdateChannelConfig(BWFE_ChannelHandle h)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   uint8_t slice;
   
   /* apply config parameters */
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x0000FFFF, hChn->rfAgcThresh);
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x003C0000, hChn->rfAgcK1 << 18);
   BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x01C00000, hChn->rfAgcBeta << 22);
   
   for (slice = 0; slice < h->totalSlices; slice++)
   {
      /* propagate bandwidth to ping and pong on all slices */
      BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_NRAGCCTL_PI_SLC, ~0x000000F0, hChn->nrAgcK1 << 4);
      BWFE_P_ReadModifyWriteSliceReg(h, slice, BCHP_WFE_CORE_NRAGCCTL_PO_SLC, ~0x000000F0, hChn->nrAgcK1 << 4);
   }
   
   return BERR_SUCCESS;
}

/******************************************************************************
 BWFE_g3_P_ResetDgsLut()
******************************************************************************/
BERR_Code BWFE_g3_P_ResetDgsLut(BWFE_ChannelHandle h)
{
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   /* reset digisum lut */
   BWFE_g3_Adc_P_ResetLutAll(h);
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_ResetEqTaps()
******************************************************************************/
BERR_Code BWFE_g3_P_ResetEqTaps(BWFE_ChannelHandle h)
{
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   /* reset equ taps */
   BWFE_g3_Adc_P_ResetEquTaps(h);
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_ResetLicTaps()
******************************************************************************/
BERR_Code BWFE_g3_P_ResetLicTaps(BWFE_ChannelHandle h)
{
#ifndef BWFE_EXCLUDE_LIC
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   /* reset lic taps */
   BWFE_g3_Lic_P_ResetTaps(h);
   
   return BERR_SUCCESS;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BWFE_g3_P_CancelDCOffset()
******************************************************************************/
BERR_Code BWFE_g3_P_CancelDCOffset(BWFE_ChannelHandle h)
{
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BWFE_g3_P_GetCorrDelayEventHandle()
******************************************************************************/
BERR_Code BWFE_g3_P_GetWfeReadyEventHandle(BWFE_ChannelHandle h, BKNI_EventHandle *hWfeReady)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   
   *hWfeReady = hChn->hWfeReady;
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_CalibrateAnalogDelay()
******************************************************************************/
BERR_Code BWFE_g3_P_CalibrateAnalogDelay(BWFE_ChannelHandle h)
{
#ifndef BWFE_EXCLUDE_ANALOG_DELAY
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   bool bBusy;

   if (h->bReference)
      return BERR_NOT_SUPPORTED;

   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;

   if (hChn->corrDpmDebug & 0x1)
      return BWFE_ERR_POWERED_DOWN;

   /* check if correlator in use */
   BKNI_EnterCriticalSection();
   bBusy = hChn->bCorrInProgress;
   if (!bBusy)
      hChn->bCorrInProgress = true;
   BKNI_LeaveCriticalSection();

   if (bBusy)
      return BWFE_ERR_CORR_BUSY;

   /* select input to correlator */
   BWFE_P_WriteRegister(h, BCHP_WFE_CORE_CORRINSEL, hChn->corrInputSelect & 0x3);

   /* enable DPM tone, disable DPM after delay calculations */
   if ((hChn->corrDpmDebug & 0x6) == 0)
      BWFE_P_EnableDpmPilot(h);

   /* use slice0 as reference */
   hChn->corrRefSlice = 0;
   hChn->postCalcDelayFunct = BWFE_g3_Corr_P_CompensateDelay;
   BWFE_g3_Corr_P_StartCorrelator(h, hChn->dpmPilotFreqKhz * 1000, 0, BWFE_g3_Corr_P_CalcDelay);

   return retCode;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BWFE_g3_P_GetAnalogDelay()
******************************************************************************/
BERR_Code BWFE_g3_P_GetAnalogDelay(BWFE_ChannelHandle h, uint32_t *pCorrDelay)
{
#ifndef BWFE_EXCLUDE_ANALOG_DELAY
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   /* copy correlator delays to 1D array */
   BKNI_Memcpy(pCorrDelay, hChn->corrDelay, sizeof(hChn->corrDelay));
   return retCode;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BWFE_g3_P_GetSaDoneEventHandle()
******************************************************************************/
BERR_Code BWFE_g3_P_GetSaDoneEventHandle(BWFE_Handle h, BKNI_EventHandle *hSaDoneEvent)
{
   BWFE_g3_P_Handle *hDev = h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   
   *hSaDoneEvent = hDev->hSaDoneEvent;
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_ScanSpectrum()
******************************************************************************/
BERR_Code BWFE_g3_P_ScanSpectrum(BWFE_ChannelHandle h, BWFE_SpecAnalyzerParams *pParams)
{
#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   if (!hChn->bLicReady)
      return BWFE_ERR_LIC_BUSY;

   /* validate spec analyzer parameters */
   if (pParams->freqStartHz >= pParams->freqStopHz)
   {
      BDBG_ERR(("freqStart: %d >= freqStop: %d!", pParams->freqStartHz, pParams->freqStopHz));
      return BERR_INVALID_PARAMETER;
   }
   else if ((pParams->numSamples > BWFE_SA_MAX_SAMPLES) || (pParams->numSamples == 0))
   {
      BDBG_ERR(("numSamples must be between [1,%d]!", BWFE_SA_MAX_SAMPLES));
      return BERR_INVALID_PARAMETER;
   }

   /* run spectrum analyzer scan with specified parameters */
   BKNI_Memcpy((void*)&(hChn->saParams), pParams, sizeof(BWFE_SpecAnalyzerParams));
   
   return BWFE_g3_Corr_P_ScanSpectrum(h);
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BWFE_g3_P_GetSaSamples()
******************************************************************************/
BERR_Code BWFE_g3_P_GetSaSamples(BWFE_Handle h, uint32_t *pSaSamples)
{
#ifndef BWFE_EXCLUDE_SPECTRUM_ANALYZER
   BWFE_g3_P_Handle *hDev = h->pImpl;
   
   /* copy spectrum analyzer samples */
   BKNI_Memcpy(pSaSamples, hDev->saSamples, sizeof(hDev->saSamples));
   
   return BERR_SUCCESS;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BWFE_g3_P_MirrorEquRefTaps()
******************************************************************************/
BERR_Code BWFE_g3_P_MirrorEquRefTaps(BWFE_ChannelHandle h, BWFE_ChannelHandle hRef)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BWFE_g3_P_ChannelHandle *hRefChn = (BWFE_g3_P_ChannelHandle *)hRef->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_INVALID_PARAMETER;
   
   if (!hRef->bReference)
      return BERR_INVALID_PARAMETER;
   
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   /* REF0 calibrates ADC0-3 */
   /* REF1 calibrates ADC4-6 */
   /* REF2 calibrates ADC7-9 */
   /* REF3 calibrates ADC10-13 */
   
   /* read taps from reference adc */
   BWFE_g3_Adc_P_ReadEquTaps(hRef);
   
   /* copy equalizer coefficients from specified reference tap */
   BKNI_Memcpy(hChn->equCoeff, hRefChn->equCoeff, sizeof(hRefChn->equCoeff));
   
   /* write taps to target adc */
   BWFE_g3_Adc_P_WriteEquTaps(h);
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_CalibrateAdcPhase()
******************************************************************************/
BERR_Code BWFE_g3_P_CalibrateAdcPhase(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   if (!BWFE_P_IsAdcOn(h))
      return BWFE_ERR_POWERED_DOWN;
   
   retCode = BWFE_g3_Adc_P_CalibratePhase(h);
   
   return retCode;
}


/******************************************************************************
 BWFE_g3_P_DisableTimer() - ISR context
******************************************************************************/
BERR_Code BWFE_g3_P_DisableTimer(BWFE_ChannelHandle h, BWFE_g3_TimerSelect t)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BINT_CallbackHandle hCb = NULL;
   uint32_t reg;
   
   switch (t)
   {
      case BWFE_g3_TimerSelect_e0:
         reg = BCHP_WFE_CORE_TIMERCTL0;
         hCb = hChn->hTimer0Cb;
         hChn->timer0Isr = NULL;
         break;
      
      case BWFE_g3_TimerSelect_e1:
         reg = BCHP_WFE_CORE_TIMERCTL1;
         hCb = hChn->hTimer1Cb;
         hChn->timer1Isr = NULL;
         break;
      
      default:
         BDBG_ERR(("invalid timer!"));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   
   BINT_DisableCallback_isr(hCb);
   BINT_ClearCallback_isr(hCb);
   
   /* disable specified timer */
   BWFE_P_WriteRegister(h, reg, 0);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_EnableTimer() - ISR context
******************************************************************************/
BERR_Code BWFE_g3_P_EnableTimer(BWFE_ChannelHandle h, BWFE_g3_TimerSelect t, uint32_t delay_usec, BWFE_FUNCT func)
{
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BINT_CallbackHandle hCb = NULL;
   uint32_t reg, P_hi, P_lo, Q_hi, Q_lo;
   
   if (delay_usec == 0)
      return BERR_INVALID_PARAMETER;
   
   BWFE_g3_P_DisableTimer(h, t);
   
   switch (t)
   {
      case BWFE_g3_TimerSelect_e0:
         reg = BCHP_WFE_CORE_TIMERCTL0;
         if (func)
         {
            hCb = hChn->hTimer0Cb;
            hChn->timer0Isr = func;
         }
         break;
      
      case BWFE_g3_TimerSelect_e1:
         reg = BCHP_WFE_CORE_TIMERCTL1;
         if (func)
         {
            hCb = hChn->hTimer1Cb;
            hChn->timer1Isr = func;
         }
         break;
      
      default:
         BDBG_ERR(("invalid timer!"));
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   
   if (func)
   {
      /* clear interrupt and enable */
      BINT_ClearCallback_isr(hCb);
      BINT_EnableCallback_isr(hCb);
   }
   
   /* calculate delay period = delay * Fs_adc / 4 / 4 */
   BMTH_HILO_32TO64_Mul(hChn->adcSampleFreqKhz, delay_usec, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 1000 << 4, &Q_hi, &Q_lo);   /* div by factor of 1000 for usec */
   
   if ((Q_hi != 0) || (Q_lo & 0x80000000))
   {
      BDBG_ERR(("timer overflow: delay_usec too large!"));
      return BERR_TRACE(BERR_INVALID_PARAMETER);
   }
   
   /* configure and start specified timer */
   BWFE_P_WriteRegister(h, reg, 0x80000000 | Q_lo) ; 
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_g3_P_Timer0_isr() - ISR context
******************************************************************************/
void BWFE_g3_P_Timer0_isr(void *p, int param)
{
   BWFE_ChannelHandle h = (BWFE_ChannelHandle)p;
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BWFE_FUNCT funct;
   
   BSTD_UNUSED(param);

   /* BDBG_MSG(("in BWFE_P_Timer0_isr()")); */
   funct = hChn->timer0Isr;
   BWFE_g3_P_DisableTimer(h, BWFE_g3_TimerSelect_e0);
   if (funct)
      funct(h);
}


/******************************************************************************
 BWFE_g3_P_Timer1_isr() - ISR context
******************************************************************************/
void BWFE_g3_P_Timer1_isr(void *p, int param)
{
   BWFE_ChannelHandle h = (BWFE_ChannelHandle)p;
   BWFE_g3_P_ChannelHandle *hChn = (BWFE_g3_P_ChannelHandle *)h->pImpl;
   BWFE_FUNCT funct;
   
   BSTD_UNUSED(param);
   
   /* BDBG_MSG(("in BWFE_P_Timer1_isr()")); */
   funct = hChn->timer1Isr;
   BWFE_g3_P_DisableTimer(h, BWFE_g3_TimerSelect_e1);
   if (funct)
      funct(h);
}

