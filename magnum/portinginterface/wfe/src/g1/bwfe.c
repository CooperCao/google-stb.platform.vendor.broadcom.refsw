/***************************************************************************
 *     Copyright (c) 2005-2012, Broadcom Corporation
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
#include "bwfe.h"
#include "bwfe_priv.h"

BDBG_MODULE(bwfe);

/* #define BWFE_USE_TIMER_CALINL */


static const BWFE_Settings defDevSettings =
{
   NULL
};

static const BWFE_ChannelSettings defChanSettings =
{
   (uint8_t)0
};


/******************************************************************************
 BWFE_GetDefaultSettings()
******************************************************************************/
BERR_Code BWFE_GetDefaultSettings(BWFE_Settings *pDefSettings)
{
   BDBG_ENTER(BWFE_GetDefaultSettings);
   *pDefSettings = defDevSettings;
   BDBG_LEAVE(BWFE_GetDefaultSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_TimerCallback()
******************************************************************************/
void BWFE_TimerCallback(void *param1, int param2)
{
   BWFE_ChannelHandle h = (BWFE_ChannelHandle)param1;
   
   //printf("!calinl(%d)\n", h->channel);
   BTMR_StopTimer(h->pDevice->tmrCalInl);
   
   /* RF off, clocks on for INL calibration */
   BWFE_Rffe_P_PowerDown(h);
   BWFE_Adc_P_CalibrateINLv2(h);
   BWFE_Rffe_P_PowerUp(h);
   
   BTMR_DestroyTimer(h->pDevice->tmrCalInl);
}


/******************************************************************************
 BWFE_Open()
******************************************************************************/
BERR_Code BWFE_Open(
   BWFE_Handle *h,      /* [out] BWFE handle */
   BCHP_Handle hChip,   /* [in] chip handle */
   void *pReg,          /* [in] pointer to register or i2c handle */
   BINT_Handle hInt,    /* [in] interrupt handle */
   const BWFE_Settings *pDefSettings   /* [in] default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_Handle hDev;
   uint8_t i;
   
   BDBG_ASSERT(h);
   BDBG_ASSERT(pDefSettings);
   BDBG_ENTER(BWFE_Open);
   
   /* allocate heap memory for device handle */
   hDev = (BWFE_Handle)BKNI_Malloc(sizeof(BWFE_P_Handle));
   BDBG_ASSERT(hDev);
   BKNI_Memset((void*)hDev, 0, sizeof(BWFE_P_Handle));
   
   /* allocate heap memory for channel handle pointer */
   hDev->pChannels = (BWFE_P_ChannelHandle **)BKNI_Malloc(BWFE_NUM_CHANNELS * sizeof(BWFE_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   BKNI_Memset((void*)hDev->pChannels, 0, BWFE_NUM_CHANNELS * sizeof(BWFE_P_ChannelHandle *));
   
   /* initialize device handle */
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BWFE_Settings));
   hDev->hRegister = (BREG_Handle)pReg;
   hDev->hInterrupt = hInt;
   hDev->hTimer = hDev->settings.hTimer;
   hDev->adcSampleFreqKhz = BWFE_DEF_FS_ADC_KHZ;
   hDev->totalChannels = BWFE_NUM_CHANNELS;
   
   for (i = 0; i < BWFE_NUM_CHANNELS; i++)
      hDev->pChannels[i] = NULL;
   
   *h = hDev;
   BDBG_LEAVE(BWFE_Open);
   return retCode;
}


/******************************************************************************
 BWFE_Close()
******************************************************************************/
BERR_Code BWFE_Close(BWFE_Handle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BWFE_Close);
   
   /* TBD destroy timers */
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h);
   h = NULL;

   BDBG_LEAVE(BWFE_Close);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_GetTotalChannels()
******************************************************************************/
BERR_Code BWFE_GetTotalChannels(
   BWFE_Handle h,             /* [in] BWFE handle */
   uint8_t     *totalChannels /* [out] number of channels supported */
)
{
   BSTD_UNUSED(h);
   BDBG_ENTER(BWFE_GetTotalChannels);
   
   *totalChannels = BWFE_NUM_CHANNELS;
   
   BDBG_LEAVE(BWFE_GetTotalChannels);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_GetChannelDefaultSettings()
******************************************************************************/
BERR_Code BWFE_GetChannelDefaultSettings(
   BWFE_Handle   h,                      /* [in] BAST handle */
   uint8_t       chanNum,                /* [in] channel number */
   BWFE_ChannelSettings *pChnDefSettings /* [out] default channel settings */
)
{
   BSTD_UNUSED(h);
   BSTD_UNUSED(chanNum);
   BDBG_ENTER(BWFE_GetChannelDefaultSettings);
   
   *pChnDefSettings = defChanSettings
   
   BDBG_LEAVE(BWFE_GetChannelDefaultSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_OpenChannel()
******************************************************************************/
BERR_Code BWFE_OpenChannel(
   BWFE_Handle                h,               /* [in] BWFE handle */
   BWFE_ChannelHandle         *pChannelHandle, /* [out] BWFE channel handle */
   uint32_t                   chanNum,         /* [in] channel number */
   const BWFE_ChannelSettings *pSettings       /* [in] channel settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BWFE_ChannelSettings chnSettings;
   BWFE_ChannelHandle chnHandle;
   uint8_t i;
   
   BDBG_ASSERT(h);
   BDBG_ASSERT(pChannelHandle);
   BDBG_ASSERT(chanNum < BWFE_NUM_CHANNELS);
   BDBG_ENTER(BWFE_OpenChannel);
   
   if (pSettings == NULL)
      BWFE_GetChannelDefaultSettings(h, chanNum, &chnSettings);
   else
      chnSettings = *pSettings;
   
   /* allocate heap memory for the channel handle */
   chnHandle = (BWFE_ChannelHandle)BKNI_Malloc(sizeof(BWFE_P_ChannelHandle));
   BDBG_ASSERT(chnHandle);
   BKNI_Memset((void*)chnHandle, 0, sizeof(BWFE_P_ChannelHandle));
   
   /* initialize channel handle */
   BKNI_Memcpy((void*)(&(chnHandle->settings)), (void*)&chnSettings, sizeof(BWFE_ChannelSettings));
   chnHandle->pDevice = h;
   chnHandle->accSaturateCount = 0;
   chnHandle->lutSaturateCount = 0;
   BKNI_Memset((void*)chnHandle->licCoeff, 0, BWFE_NUM_LANES * BWFE_NUM_LIC_TAPS);     /* clear LIC coefficients */
   BKNI_Memset((void*)chnHandle->equCoeff, 0, BWFE_NUM_SLICES * 3 * BWFE_NUM_EQ_TAPS); /* clear EQ coefficients */
   chnHandle->rfAgcInt = 0;
   chnHandle->dgsClipCtl = 0;
   chnHandle->dgsClipCount = 0;
   chnHandle->licUpdateMask = 0;
   
   /* set up mask for LICCTL2 based on number of taps + dco tap */
   for (i = 0; i < (BWFE_NUM_LIC_TAPS + 1) / 2; i++)
      chnHandle->licUpdateMask |= (1 << i);
   
   chnHandle->licRuntimeMs = 50;
   chnHandle->equRuntimeMs = 20;
   chnHandle->rfAgcThresh = 0xE000;
   chnHandle->rfAgcK1 = 8;
   chnHandle->channel = chanNum;
   chnHandle->totalLanes = BWFE_NUM_LANES;
   chnHandle->dgsClipLane = 0;
   chnHandle->dgsMu1 = 0;  /* mu step size 2^-5 */
   chnHandle->dgsMu2 = 0;  /* mu step size 2^-5 */
   chnHandle->dgsMu3 = 0;  /* mu step size 2^-5 */
   chnHandle->licMu = 3;   /* mu step size 2^-15 */
   chnHandle->licEnableRefLane = 1; /* enable ref lane during lane correction */
   chnHandle->bEnabled = false;
   
   /* initialize chip specific config parameters */
   retCode = BWFE_P_InitConfig(chnHandle);
   
   h->pChannels[chanNum] = chnHandle;
   
   *pChannelHandle = chnHandle;
   BDBG_LEAVE(BWFE_OpenChannel);
   return retCode;
}


/******************************************************************************
 BWFE_CloseChannel()
******************************************************************************/
BERR_Code BWFE_CloseChannel(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   BDBG_ASSERT(h);
   BDBG_ENTER(BWFE_CloseChannel);
   
   BKNI_Free((void*)h);
   h = NULL;
   
   BDBG_LEAVE(BWFE_CloseChannel);
   return retCode;
}


/******************************************************************************
 BWFE_ResetChannel()
******************************************************************************/
BERR_Code BWFE_ResetChannel(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   BWFE_Rffe_P_PowerDown(h);     /* power down RFFE */
   BWFE_Adc_P_PowerUp(h);        /* power up ADC */
   BWFE_Lic_P_PowerUp(h);        /* power up LIC */
   
#ifndef BWFE_BYPASS_INL
   /* run INL calibration */
   retCode = BWFE_Adc_P_CalibrateINL(h);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("INL calibration error"));
      return retCode;
   }
#endif

   BWFE_Lic_P_ResetTaps(h);   /* reset lic taps */
   BWFE_Adc_P_ResetEqTaps(h); /* reset digisum lut */

#ifndef BWFE_BYPASS_EQU
   /* run pipeline equalization */
   retCode = BWFE_Adc_P_EqualizePipeline(h);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_ERR(("pipeline equalization error"));
      return retCode;
   }
#endif
   
   BWFE_Rffe_P_PowerUp(h);    /* power up RFFE */
   return retCode;
}


/******************************************************************************
 BWFE_Reset()
******************************************************************************/
BERR_Code BWFE_Reset(BWFE_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint8_t i;
   
   BDBG_ASSERT(h);
   BDBG_ENTER(BWFE_Reset);
   
   /* configure adc sample clock */
   BWFE_P_SetAdcSampleFreq(h, h->adcSampleFreqKhz);
   BWFE_P_GetAdcSampleFreq(h, &(h->adcSampleFreqKhz));
   
   for (i = 0; i < h->totalChannels; i++)
   {
      /* perform reset logic for each ADC */
      BWFE_ResetChannel(h->pChannels[i]);
   }
   
   /* run LIC lane correction at startup, then schedule periodically */
   retCode = BWFE_Lic_P_RunLaneCorrectionAll(h);
   
   BDBG_LEAVE(BWFE_Reset);
   return retCode;
}


/******************************************************************************
 BWFE_GetVersion()
******************************************************************************/
BERR_Code BWFE_GetVersion(BWFE_Handle h, BWFE_Version *pVersion)
{
   uint32_t val;
   BDBG_ASSERT(h);
   
   val = BREG_Read32(h->hRegister, BCHP_TM_CHIP_ID); 
   pVersion->chipID = (uint16_t)(val & 0xFFFF);
   
   val = BREG_Read32(h->hRegister, BCHP_TM_REV_ID); 
   pVersion->chipRev = (uint16_t)(val & 0xFFFF);
   
   pVersion->verMajor = BWFE_MAJOR_VERSION;
   pVersion->verMinor = BWFE_MINOR_VERSION;
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_EnableInput()
******************************************************************************/
BERR_Code BWFE_EnableInput(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   
   /* call channel reset sequence */
   BWFE_ResetChannel(h);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_DisableInput()
******************************************************************************/
BERR_Code BWFE_DisableInput(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   BWFE_Rffe_P_PowerDown(h);  /* power down RFFE */
   BWFE_Lic_P_PowerDown(h);   /* power down LIC */
   BWFE_Adc_P_PowerDown(h);   /* power down ADC */
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_GetChannelStatus()
******************************************************************************/
BERR_Code BWFE_GetChannelStatus(BWFE_ChannelHandle h, BWFE_ChannelStatus *pStatus)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pStatus);
   
   pStatus->bEnabled = h->bEnabled;

   BWFE_P_ReadRegister(h, BCHP_WFE_CORE_AGCCTL1, &val);
   pStatus->bAgcFrozen = (val & 0x00010000) ? true : false;

   pStatus->freqOffset = 0;
   pStatus->freqLower = 0;
   pStatus->freqUpper = (h->pDevice->adcSampleFreqKhz / 2) * 1000;
   pStatus->accSaturateCount = h->accSaturateCount;
   pStatus->lutSaturateCount = h->lutSaturateCount;
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_GetLicTaps() - input 2D array (lanes x taps)
******************************************************************************/
BERR_Code BWFE_GetLicTaps(BWFE_ChannelHandle h, uint32_t **pLicCoeff)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pLicCoeff);
   
   /* refresh lic taps */
   BWFE_Lic_P_UpdateTaps(h);
   
   /* copy lic coefficients to 2D array (lanes x taps) */
   BKNI_Memcpy(pLicCoeff, h->licCoeff, sizeof(h->licCoeff));
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_GetEqTaps() - input 3D array (slices x mdacs x taps)
******************************************************************************/
BERR_Code BWFE_GetEqTaps(BWFE_ChannelHandle h, uint32_t ***pEqCoeff)
{
   BDBG_ASSERT(h);
   BDBG_ASSERT(pEqCoeff);
   
   /* refresh eq taps */
   BWFE_Adc_P_UpdateEqTaps(h);
   
   /* copy eq coefficients to 3D array (slices x mdacs x taps) */
   BKNI_Memcpy(pEqCoeff, h->equCoeff, sizeof(h->equCoeff));
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_FreezeRfAgc()
******************************************************************************/
BERR_Code BWFE_FreezeRfAgc(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BWFE_FreezeRfAgc);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* freeze RFAGC */
   BWFE_P_OrRegister(h, BCHP_WFE_CORE_AGCCTL1, 0x02010000);
   
   BDBG_LEAVE(BWFE_FreezeRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_UnfreezeRfAgc()
******************************************************************************/
BERR_Code BWFE_UnfreezeRfAgc(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BWFE_UnfreezeRfAgc);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* unfreeze RFAGC */
   BWFE_P_AndRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x02010000);
   
   BDBG_LEAVE(BWFE_UnfreezeRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_RunLaneCorrection()
******************************************************************************/
BERR_Code BWFE_RunLaneCorrection(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   return BWFE_Lic_P_RunLaneCorrection(h);
}


/******************************************************************************
 BWFE_RunLaneCorrectionAll()
******************************************************************************/
BERR_Code BWFE_RunLaneCorrectionAll(BWFE_Handle h)
{
   BDBG_ASSERT(h);
   return BWFE_Lic_P_RunLaneCorrectionAll(h);
}


/******************************************************************************
 BWFE_CalibrateINL()
******************************************************************************/
BERR_Code BWFE_CalibrateINL(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
#ifdef BWFE_USE_TIMER_CALINL
   BTMR_Settings tmrSettings = {BTMR_Type_eCountDown, (BTMR_CallbackFunc)BWFE_TimerCallback, NULL, 0};
#endif

   BDBG_ASSERT(h);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;

#ifdef BWFE_USE_TIMER_CALINL
   if (h->pDevice->hTimer)
   {
      /* create timer if available */
      tmrSettings.pParm1 = (void*)h;
      retCode = BTMR_CreateTimer(h->pDevice->hTimer, &h->pDevice->tmrCalInl, &tmrSettings);
      BDBG_ASSERT(retCode == BERR_SUCCESS);
   }
   
   /* TBD add busy flag to prevent re-entry */
   BTMR_StartTimer(h->pDevice->tmrCalInl, 10);
#else
   /* RF off, clocks on for INL calibration */
   BWFE_Rffe_P_PowerDown(h);
   retCode = BWFE_Adc_P_CalibrateINLv2(h);
   BWFE_Rffe_P_PowerUp(h);
#endif
   
   return retCode;
}


/******************************************************************************
 BWFE_EqualizePipeline()
******************************************************************************/
BERR_Code BWFE_EqualizePipeline(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BDBG_ASSERT(h);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* run pipeline equalization */
   BWFE_Rffe_P_PowerDown(h);
   retCode = BWFE_Adc_P_EqualizePipeline(h);
   BWFE_Rffe_P_PowerUp(h);
   
   return retCode;
}


/******************************************************************************
 BWFE_SetAdcSampleFreq()
******************************************************************************/
BERR_Code BWFE_SetAdcSampleFreq(BWFE_Handle h, uint32_t freqKhz)
{
   BDBG_ASSERT(h);
   
   return BWFE_P_SetAdcSampleFreq(h, freqKhz);
}


/******************************************************************************
 BWFE_GetAdcSampleFreq()
******************************************************************************/
BERR_Code BWFE_GetAdcSampleFreq(BWFE_Handle h, uint32_t *freqKhz)
{
   BDBG_ASSERT(h);
   
   return BWFE_P_GetAdcSampleFreq(h, freqKhz);
}


/******************************************************************************
 BWFE_ReadConfig()
******************************************************************************/
BERR_Code BWFE_ReadConfig(BWFE_ChannelHandle h, uint16_t id, uint8_t *p, uint8_t len)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   BDBG_ASSERT(h);
   BDBG_ENTER(BWFE_ReadConfig);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   if ((p == NULL) || !((len == 1) || (len == 2) || (len == 4)))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   
   switch (id)
   {
      case BWFE_CONFIG_LIC_RUNTIME_MS:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_LIC_RUNTIME_MS);
         p[0] = (uint8_t)(h->licRuntimeMs >> 8);
         p[1] = (uint8_t)(h->licRuntimeMs & 0xFF);           
         break;
         
      case BWFE_CONFIG_RF_AGC_THRESH:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_RF_AGC_THRESH);
         p[0] = (uint8_t)(h->rfAgcThresh >> 8);
         p[1] = (uint8_t)(h->rfAgcThresh & 0xFF);           
         break;
      
      case BWFE_CONFIG_RF_AGC_K1:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_RF_AGC_K1);
         *p = h->rfAgcK1;
         break;
      
      case BWFE_CONFIG_RF_AGC_INT:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_RF_AGC_INT);
         BWFE_P_ReadRegister(h, BCHP_WFE_CORE_REG_AGC_LF_INT_WDATA, &h->rfAgcInt);
         p[0] = (uint8_t)(h->rfAgcInt >> 24);
         p[1] = (uint8_t)(h->rfAgcInt >> 16);
         p[2] = (uint8_t)(h->rfAgcInt >> 8);
         p[3] = (uint8_t)(h->rfAgcInt & 0xFF);
         break;
      
      case BWFE_CONFIG_DGS_CLIP_LANE_SELECT:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_CLIP_LANE_SELECT);
         *p = h->dgsClipLane;
         break;
      
      case BWFE_CONFIG_DGS_CLIP_CTL:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_CLIP_CTL);
         p[0] = (uint8_t)(h->dgsClipCtl >> 24);
         p[1] = (uint8_t)(h->dgsClipCtl >> 16);
         p[2] = (uint8_t)(h->dgsClipCtl >> 8);
         p[3] = (uint8_t)(h->dgsClipCtl & 0xFF);
         break;
      
      case BWFE_CONFIG_DGS_CLIP_COUNT:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_CLIP_COUNT);
         BWFE_P_ReadLaneRegister(h, h->dgsClipLane, BCHP_WFE_CORE_DGSCLPCNT_LN, &h->dgsClipCount);
         p[0] = (uint8_t)(h->dgsClipCount >> 24);
         p[1] = (uint8_t)(h->dgsClipCount >> 16);
         p[2] = (uint8_t)(h->dgsClipCount >> 8);
         p[3] = (uint8_t)(h->dgsClipCount & 0xFF);
         break;
      
      case BWFE_CONFIG_DGS_EQU_RUNTIME_MS:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_EQU_RUNTIME_MS);
         p[0] = (uint8_t)(h->equRuntimeMs >> 8);
         p[1] = (uint8_t)(h->equRuntimeMs & 0xFF);           
         break;
      
      case BWFE_CONFIG_DGS_EQU_MU1:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_EQU_MU1);
         *p = h->dgsMu1;
         break;
      
      case BWFE_CONFIG_DGS_EQU_MU2:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_EQU_MU2);
         *p = h->dgsMu2;
         break;
      
      case BWFE_CONFIG_DGS_EQU_MU3:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_EQU_MU3);
         *p = h->dgsMu3;
         break;
      
      case BWFE_CONFIG_LIC_UPDATE_MASK:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_LIC_UPDATE_MASK);
         p[0] = (uint8_t)(h->licUpdateMask >> 24);
         p[1] = (uint8_t)(h->licUpdateMask >> 16);
         p[2] = (uint8_t)(h->licUpdateMask >> 8);
         p[3] = (uint8_t)(h->licUpdateMask & 0xFF);
         break;
      
      case BWFE_CONFIG_LIC_MU:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_LIC_MU);
         *p = h->licMu;
         break;
      
      case BWFE_CONFIG_LIC_ENABLE_REF_LANE:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_LIC_ENABLE_REF_LANE);
         *p = h->licEnableRefLane;
         break;
      
      default:
         BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
         break;
   }
   
   BDBG_LEAVE(BWFE_ReadConfig);
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_WriteConfig()
******************************************************************************/
BERR_Code BWFE_WriteConfig(BWFE_ChannelHandle h, uint16_t id, uint8_t *p, uint8_t len)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   BDBG_ASSERT(h);
   BDBG_ENTER(BWFE_WriteConfig);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   if ((p == NULL) || !((len == 1) || (len == 2) || (len == 4)))
      return (BERR_TRACE(BERR_INVALID_PARAMETER));
   
   switch (id)
   {
      case BWFE_CONFIG_LIC_RUNTIME_MS:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_LIC_RUNTIME_MS);
         h->licRuntimeMs = (p[0] << 8) | p[1];
         if (h->licRuntimeMs > 5000)
            h->licRuntimeMs = 5000;  /* limit lane correction run-time */
         break;
      
      case BWFE_CONFIG_RF_AGC_THRESH:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_RF_AGC_THRESH);
         h->rfAgcThresh = (p[0] << 8) | p[1];
         BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x0000FFFF, h->rfAgcThresh);
         break;
      
      case BWFE_CONFIG_RF_AGC_K1:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_RF_AGC_K1);
         h->rfAgcK1 = *p;
         h->rfAgcK1 &= 0xF;
         BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_AGCCTL1, ~0x001E0000, h->rfAgcK1 << 17);
         break;
      
      case BWFE_CONFIG_RF_AGC_INT:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_RF_AGC_INT);
         h->rfAgcInt = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
         BWFE_P_WriteRegister(h, BCHP_WFE_CORE_REG_AGC_LF_INT_WDATA, h->rfAgcInt);
         break;
      
      case BWFE_CONFIG_DGS_CLIP_LANE_SELECT:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_CLIP_LANE_SELECT);
         h->dgsClipLane = *p;
         break;
      
      case BWFE_CONFIG_DGS_CLIP_CTL:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_CLIP_CTL);
         h->dgsClipCtl = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
         h->dgsClipCtl &= 0x1FFF0FFF;  /* mask out reserved bits */
         BWFE_P_WriteLaneRegister(h, h->dgsClipLane, BCHP_WFE_CORE_DGSCLP_LN, h->dgsClipCtl);
         break;
      
      case BWFE_CONFIG_DGS_CLIP_COUNT:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_CLIP_COUNT);
         h->dgsClipCount = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
         BWFE_P_WriteLaneRegister(h, h->dgsClipLane, BCHP_WFE_CORE_DGSCLPCNT_LN, h->dgsClipCount);
         break;
      
      case BWFE_CONFIG_DGS_EQU_RUNTIME_MS:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_EQU_RUNTIME_MS);
         h->equRuntimeMs = (p[0] << 8) | p[1];
         if (h->equRuntimeMs > 5000)
            h->equRuntimeMs = 5000;  /* limit equalization run-time */
         break;
      
      case BWFE_CONFIG_DGS_EQU_MU1:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_EQU_MU1);
         h->dgsMu1 = *p;
         h->dgsMu1 &= 0x7;
         break;
      
      case BWFE_CONFIG_DGS_EQU_MU2:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_EQU_MU2);
         h->dgsMu2 = *p;
         h->dgsMu2 &= 0x7;
         break;
      
      case BWFE_CONFIG_DGS_EQU_MU3:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_DGS_EQU_MU3);
         h->dgsMu3 = *p;
         h->dgsMu3 &= 0x7;
         break;
      
      case BWFE_CONFIG_LIC_UPDATE_MASK:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_LIC_UPDATE_MASK);
         h->licUpdateMask = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
         break;
      
      case BWFE_CONFIG_LIC_MU:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_LIC_MU);
         h->licMu = *p;
         h->licMu &= 0xF;
         break;
      
      case BWFE_CONFIG_LIC_ENABLE_REF_LANE:
         BDBG_ASSERT(len == BWFE_CONFIG_LEN_LIC_ENABLE_REF_LANE);
         h->licEnableRefLane = *p;
         break;
      
      default:
         BERR_TRACE(retCode = BERR_INVALID_PARAMETER);
         break;
   }
   
   BDBG_LEAVE(BWFE_WriteConfig);
   return retCode;
}


/******************************************************************************
 BWFE_ResetDgsLut()
******************************************************************************/
BERR_Code BWFE_ResetDgsLut(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* reset digisum lut */
   BWFE_Adc_P_ResetLut(h);
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_ResetEqTaps()
******************************************************************************/
BERR_Code BWFE_ResetEqTaps(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* reset digisum lut */
   BWFE_Adc_P_ResetEqTaps(h);
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_ResetLicTaps()
******************************************************************************/
BERR_Code BWFE_ResetLicTaps(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* reset lic taps */
   BWFE_Lic_P_ResetTaps(h);
   
   return BERR_SUCCESS;
}


/******************************************************************************
 BWFE_CancelDCOffset()
******************************************************************************/
BERR_Code BWFE_CancelDCOffset(BWFE_ChannelHandle h)
{
   BDBG_ASSERT(h);
   
   if (!h->bEnabled)
      return BWFE_ERR_POWERED_DOWN;
   
   /* cancel DC offset */
   BWFE_Adc_P_CancelDCOffset(h);
   
   return BERR_SUCCESS;
}
