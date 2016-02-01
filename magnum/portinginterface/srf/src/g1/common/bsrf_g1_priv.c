/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
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
#include "bsrf.h"
#include "bsrf_priv.h"
#include "bsrf_g1_priv.h"

BDBG_MODULE(bsrf_g1_priv);


/******************************************************************************
 BSRF_g1_P_Open()
******************************************************************************/
BERR_Code BSRF_g1_P_Open(
   BSRF_Handle *h,      /* [out] BSRF handle */
   BCHP_Handle hChip,   /* [in] chip handle */
   void *pReg,          /* [in] pointer to register or i2c handle */
   BINT_Handle hInt,    /* [in] interrupt handle */
   const BSRF_Settings *pDefSettings   /* [in] default settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSRF_Handle hDev;
   BSRF_g1_P_Handle *hG1Dev;
   uint8_t i;

   BDBG_ASSERT(h);
   BDBG_ASSERT(pDefSettings);
   BDBG_ENTER(BSRF_g1_P_Open);

   /* allocate heap memory for device handle */
   hDev = (BSRF_Handle)BKNI_Malloc(sizeof(BSRF_P_Handle));
   BDBG_ASSERT(hDev);
   BKNI_Memset((void*)hDev, 0, sizeof(BSRF_P_Handle));
   hG1Dev = (BSRF_g1_P_Handle *)BKNI_Malloc(sizeof(BSRF_g1_P_Handle));
   BDBG_ASSERT(hG1Dev);
   BKNI_Memset((void*)hG1Dev, 0, sizeof(BSRF_g1_P_Handle));
   hDev->pImpl = (void*)hG1Dev;

   /* allocate heap memory for channel handle pointer */
   hDev->pChannels = (BSRF_P_ChannelHandle **)BKNI_Malloc(BSRF_NUM_CHANNELS * sizeof(BSRF_P_ChannelHandle *));
   BDBG_ASSERT(hDev->pChannels);
   BKNI_Memset((void*)hDev->pChannels, 0, sizeof(BSRF_NUM_CHANNELS * sizeof(BSRF_P_ChannelHandle *)));

   /* initialize device handle */
   BKNI_Memcpy((void*)(&(hDev->settings)), (void*)pDefSettings, sizeof(BSRF_Settings));
   hG1Dev->hRegister = (BREG_Handle)pReg;
   hG1Dev->hInterrupt = hInt;
   hG1Dev->dacFreqHz = 50000000;
   hG1Dev->xsinxCoeff[0] = 0;
   hG1Dev->xsinxCoeff[1] = 0;
   hG1Dev->xsinxCoeff[2] = 0;
   hG1Dev->xsinxCoeff[3] = 0x1000;

   hDev->totalChannels = BSRF_NUM_CHANNELS;
   for (i = 0; i < hDev->totalChannels; i++)
      hDev->pChannels[i] = NULL;

   *h = hDev;
   BDBG_LEAVE(BSRF_g1_P_Open);
   return retCode;
}


/******************************************************************************
 BSRF_g1_P_Close()
******************************************************************************/
BERR_Code BSRF_g1_P_Close(BSRF_Handle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_Close);

   /* free handles */
   BKNI_Free((void*)h->pChannels);
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

   BDBG_LEAVE(BSRF_g1_P_Close);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetTotalChannels()
******************************************************************************/
BERR_Code BSRF_g1_P_GetTotalChannels(
   BSRF_Handle h,             /* [in] BSRF handle */
   uint8_t     *totalChannels /* [out] number of channels supported */
)
{
   BSTD_UNUSED(h);
   BDBG_ENTER(BSRF_g1_P_GetTotalChannels);

   *totalChannels = h->totalChannels;

   BDBG_LEAVE(BSRF_g1_P_GetTotalChannels);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_OpenChannel()
******************************************************************************/
BERR_Code BSRF_g1_P_OpenChannel(
   BSRF_Handle                h,               /* [in] BSRF handle */
   BSRF_ChannelHandle         *pChannelHandle, /* [out] BSRF channel handle */
   uint8_t                    chanNum,         /* [in] channel number */
   const BSRF_ChannelSettings *pSettings       /* [in] channel settings */
)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSRF_ChannelSettings chnSettings;
   BSRF_ChannelHandle chnHandle;
   BSRF_g1_P_ChannelHandle *chG1;

   BDBG_ASSERT(h);
   BDBG_ASSERT(pChannelHandle);
   BDBG_ENTER(BSRF_g1_P_OpenChannel);

   /* set channel settings */
   if (pSettings == NULL)
      BSRF_GetChannelDefaultSettings(h, chanNum, &chnSettings);
   else
      chnSettings = *pSettings;

   /* check channel index */
   if (chanNum >= h->totalChannels)
      return BERR_INVALID_PARAMETER;

   /* allocate memory for the channel handle */
   chnHandle = (BSRF_ChannelHandle)BKNI_Malloc(sizeof(BSRF_P_ChannelHandle));
   BDBG_ASSERT(chnHandle);
   BKNI_Memset((void*)chnHandle, 0, sizeof(BSRF_P_ChannelHandle));
   chG1 = (BSRF_g1_P_ChannelHandle *)BKNI_Malloc(sizeof(BSRF_g1_P_ChannelHandle));
   BDBG_ASSERT(chG1);
   BKNI_Memset((void*)chG1, 0, sizeof(BSRF_g1_P_ChannelHandle));

   /* initialize channel handle */
   BKNI_Memcpy((void*)(&(chnHandle->settings)), (void*)&chnSettings, sizeof(BSRF_ChannelSettings));
   chnHandle->pDevice = h;
   chnHandle->channel = chanNum;
   chnHandle->bEnabled = false;
   chnHandle->pImpl = (void*)chG1;

   h->pChannels[chanNum] = chnHandle;

   *pChannelHandle = chnHandle;
   BDBG_LEAVE(BSRF_g1_P_OpenChannel);
   return retCode;
}


/******************************************************************************
 BSRF_g1_P_CloseChannel()
******************************************************************************/
BERR_Code BSRF_g1_P_CloseChannel(BSRF_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_CloseChannel);

   /* free channel handle */
   BKNI_Free((void*)h->pImpl);
   BKNI_Free((void*)h);
   h = NULL;

   done:
   BDBG_LEAVE(BSRF_g1_P_CloseChannel);
   return retCode;
}


/******************************************************************************
 BSRF_g1_P_Reset()
******************************************************************************/
BERR_Code BSRF_g1_P_Reset(BSRF_Handle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSRF_ChannelHandle hChn;
   BSRF_g1_P_Handle *hDev = (BSRF_g1_P_Handle *)h->pImpl;
   int16_t xsinx[] = {0, 0, 0, 0x1000};
   uint8_t i;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_Reset);

   for (i = 0; i < BSRF_NUM_CHANNELS; i++)
   {
      /* skip channel if not opened */
      hChn = h->pChannels[i];
      if (hChn == NULL)
         continue;

      /* toggle rfagc reset */
      BSRF_P_ToggleBit(hChn, BCHP_SRFE_RFAGC_LOOP_RST_CTRL, 0x00000002);

      /* toggle srfe resets */
      BSRF_P_ToggleBit(hChn, BCHP_SRFE_FE_RST_CTRL, 0x00000003);

      /* load rfagc lut and init tuner */
      BSRF_g1_Rfagc_P_Init(hChn);
      BSRF_g1_Tuner_P_Init(hChn);

      /* power down by default */
      BSRF_g1_P_PowerDown(hChn);

      /* default test dac config */
      BSRF_g1_P_ConfigTestDac(h, 50000000, xsinx);
   }

   BDBG_LEAVE(BSRF_g1_P_Reset);
   return retCode;
}


/******************************************************************************
 BSRF_g1_P_GetVersion()
******************************************************************************/
BERR_Code BSRF_g1_P_GetVersion(BSRF_Handle h, BFEC_VersionInfo *pVersion)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetVersion);

   pVersion->majorVersion = BSRF_API_VERSION;
   pVersion->minorVersion = BSRF_G1_RELEASE_VERSION;
   pVersion->buildType = BSRF_G1_BUILD_VERSION;
   pVersion->buildId = 0;

   BDBG_LEAVE(BSRF_g1_P_GetVersion);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_PowerUp
******************************************************************************/
BERR_Code BSRF_g1_P_PowerUp(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_PowerUp);

   /* analog power up sequence */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, ~0x00000010);   /* disable rf test path */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, 0x00001000);     /* power up bandgap */
   BKNI_Delay(100);  /* wait 100us for bandgap current to settle */

   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, 0x011FAFEF);     /* power up analog blocks */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, 0x000C0003); /* power up mixer pll */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER16_APLL, 0x00000042);    /* power up ldo */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER24_ADC, ~0x00380000);   /* power up adc */
   BKNI_Delay(200);  /* wait 200us then release all resets */

   BSRF_P_ToggleBit(h, BCHP_SRFE_ANA_WRITER03_DCO, 0x00000001);      /* reset dco clock */
   BSRF_P_ToggleBit(h, BCHP_SRFE_ANA_WRITER05_DCO, 0x00000400);      /* reset dco sigma-delta integrator */
   BSRF_P_ToggleBit(h, BCHP_SRFE_ANA_WRITER05_DCO, 0x00000001);      /* global dco digital reset */
   BSRF_P_ToggleBit(h, BCHP_SRFE_ANA_WRITER06_AGC, 0x00010000);      /* reset agc clock */
   BSRF_P_ToggleBit(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, 0x0800007C);  /* reset mixer */
   BSRF_P_ToggleBit(h, BCHP_SRFE_ANA_WRITER19_APLL, 0x00000021);     /* reset pll */
   BSRF_P_ToggleBit(h, BCHP_SRFE_ANA_WRITER24_ADC, 0x0000000C);     /* reset adc i and q integrators */
   BSRF_P_ToggleBit(h, BCHP_SRFE_ANA_WRITER25_ADC, 0x00600000);     /* reset adc i and q data */

   /* TBD mixer pll will lock in ~600us after resets are released and digital calibration can start */

#if 0
   /* enable test clock */
   BSRF_P_OrRegister(h, BCHP_SRFE_RFAGC_LOOP_CLK_CTRL, 0x00000001);
#endif

   /* enable rfagc */
   BSRF_P_OrRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL1, 0x40000000);

   /* TBD set manual PGA and LNA Gain Control Code */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_PGA_GCC, 0x00000020);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_LNA_GCC, 0x00000001);

   BDBG_LEAVE(BSRF_g1_P_PowerUp);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_PowerDown
******************************************************************************/
BERR_Code BSRF_g1_P_PowerDown(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_PowerDown);

   /* analog power down */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, ~0x011FBFEF);      /* power down analog blocks */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, ~0x000C0003);  /* power down mixer pll */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER16_APLL, ~0x00000042);     /* power down ldo */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER24_ADC, 0x00380000);        /* power down adc */

#if 0
   /* disable test clock */
   BSRF_P_AndRegister(h, BCHP_SRFE_RFAGC_LOOP_CLK_CTRL, ~0x00000001);
#endif

   /* disable rfagc */
   BSRF_P_AndRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL1, ~0x40000000);

   BDBG_LEAVE(BSRF_g1_P_PowerDown);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_FreezeRfAgc
******************************************************************************/
BERR_Code BSRF_g1_P_FreezeRfAgc(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_FreezeRfAgc);

   /* freeze rfagc */
   BSRF_P_OrRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL1, 0x00020000);

   BDBG_LEAVE(BSRF_g1_P_FreezeRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_UnfreezeRfAgc
******************************************************************************/
BERR_Code BSRF_g1_P_UnfreezeRfAgc(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_UnfreezeRfAgc);

   /* unfreeze rfagc */
   BSRF_P_AndRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL1, ~0x00020000);

   BDBG_LEAVE(BSRF_g1_P_UnfreezeRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_WriteRfAgc
******************************************************************************/
BERR_Code BSRF_g1_P_WriteRfAgc(BSRF_ChannelHandle h, uint32_t val)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_WriteRfAgc);

   /* override rfagc integrator */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LF_INT_WDATA, val);

   BDBG_LEAVE(BSRF_g1_P_WriteRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_ReadRfAgc
******************************************************************************/
BERR_Code BSRF_g1_P_ReadRfAgc(BSRF_ChannelHandle h, uint32_t *pVal)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_ReadRfAgc);

   /* read rfagc integrator */
   BSRF_P_ReadRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LF_INT_WDATA, pVal);

   BDBG_LEAVE(BSRF_g1_P_ReadRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetInputPower
******************************************************************************/
BERR_Code BSRF_g1_P_GetInputPower(BSRF_ChannelHandle h, uint32_t *pPower)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetInputPower);

   /* TBD */

   BDBG_LEAVE(BSRF_g1_P_GetInputPower);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSRF_g1_P_SetRfAgcSettings
******************************************************************************/
BERR_Code BSRF_g1_P_SetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings settings)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_SetRfAgcSettings);

   hChn->settings = settings;
   retCode = BSRF_g1_Rfagc_P_SetSettings(h, settings);

   BDBG_LEAVE(BSRF_g1_P_SetRfAgcSettings);
   return retCode;
}


/******************************************************************************
 BSRF_g1_P_GetRfAgcSettings
******************************************************************************/
BERR_Code BSRF_g1_P_GetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings *pSettings)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetRfAgcSettings);

   /* return saved settings */
   *pSettings = hChn->settings;

   BDBG_LEAVE(BSRF_g1_P_GetRfAgcSettings);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSRF_g1_P_SetFastDecayGainThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_SetFastDecayGainThreshold(BSRF_ChannelHandle h, uint32_t threshold)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_SetFastDecayGainThreshold);

   /* TBD */

   BDBG_LEAVE(BSRF_g1_P_SetFastDecayGainThreshold);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSRF_g1_P_GetFastDecayGainThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_GetFastDecayGainThreshold(BSRF_ChannelHandle h, uint32_t *pThreshold)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetFastDecayGainThreshold);

   /* TBD */

   BDBG_LEAVE(BSRF_g1_P_GetFastDecayGainThreshold);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSRF_g1_P_SetAntennaOverThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_SetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t mode)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_SetAntennaOverThreshold);

   hChn->modeAoc = mode;
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, ~0x000000C0, (mode & 0x3) << 6);

   BDBG_LEAVE(BSRF_g1_P_SetAntennaOverThreshold);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetAntennaOverThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_GetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t *pMode)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetAntennaOverThreshold);

   *pMode = hChn->modeAoc;

   BDBG_LEAVE(BSRF_g1_P_GetAntennaOverThreshold);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_SetAntennaDetectThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_SetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t mode)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_SetAntennaDetectThreshold);

   hChn->modeAd = mode;
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, ~0x00000030, (mode & 0x3) << 4);

   BDBG_LEAVE(BSRF_g1_P_SetAntennaDetectThreshold);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetAntennaDetectThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_GetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t *pMode)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetAntennaDetectThreshold);

   *pMode = hChn->modeAd;

   BDBG_LEAVE(BSRF_g1_P_GetAntennaDetectThreshold);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetAntennaStatus
******************************************************************************/
BERR_Code BSRF_g1_P_GetAntennaStatus(BSRF_ChannelHandle h, BSRF_AntennaStatus *pStatus)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetAntennaStatus);

   /* retrieve antenna status */
   pStatus->modeAoc = hChn->modeAoc;
   pStatus->modeAd = hChn->modeAd;

   BSRF_P_ReadRegister(h, BCHP_SRFE_ANA_ANT_OVC_PAD_1P0, &val);
   if (val & 0x00000001)
      pStatus->bOverCurrent = true;
   else
      pStatus->bOverCurrent = false;

   BSRF_P_ReadRegister(h, BCHP_SRFE_ANA_ANT_DET_PAD_1P0, &val);
   if (val & 0x00000001)
      pStatus->bDetected = true;
   else
      pStatus->bDetected = false;

   BDBG_LEAVE(BSRF_g1_P_GetAntennaStatus);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_Tune
******************************************************************************/
BERR_Code BSRF_g1_P_Tune(BSRF_ChannelHandle h, int32_t freqHz)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_Tune);

   BSRF_g1_Tuner_P_SetFcw(h, freqHz);

   BDBG_LEAVE(BSRF_g1_P_Tune);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetTunerStatus
******************************************************************************/
BERR_Code BSRF_g1_P_GetTunerStatus(BSRF_ChannelHandle h, BSRF_TunerStatus *pStatus)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetTunerStatus);

   /* retrieve status parameters */
   pStatus->tunerFreqHz = hChn->tunerFreq;
   pStatus->bPllLock = true;  /* TBD */

   BDBG_LEAVE(BSRF_g1_P_GetTunerStatus);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_ResetClipCount
******************************************************************************/
BERR_Code BSRF_g1_P_ResetClipCount(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_ResetClipCount);

   /* reset clip counter */
   BSRF_P_ToggleBit(h, BCHP_SRFE_FE_CLIP_CTRL, 0x00100000);

   BDBG_LEAVE(BSRF_g1_P_ResetClipCount);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetClipCount
******************************************************************************/
BERR_Code BSRF_g1_P_GetClipCount(BSRF_ChannelHandle h, uint32_t *pClipCount)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetClipCount);

   /* read clip counter */
   BSRF_P_ReadRegister(h, BCHP_SRFE_FE_CLIP_COUNT, pClipCount);

   BDBG_LEAVE(BSRF_g1_P_GetClipCount);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_ConfigTestMode
******************************************************************************/
BERR_Code BSRF_g1_P_ConfigTestMode(BSRF_ChannelHandle h)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_ConfigTestMode);

   /* TBD */

   BDBG_LEAVE(BSRF_g1_P_ConfigTestMode);
   return BERR_NOT_SUPPORTED;
}


/******************************************************************************
 BSRF_g1_P_ConfigOutput
******************************************************************************/
BERR_Code BSRF_g1_P_ConfigOutput(BSRF_Handle h, BSRF_OutputSelect output, bool bSlewEdges, uint8_t driveStrength_ma)
{
   BSRF_ChannelHandle hChn = h->pChannels[0];
   uint32_t reg;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_ConfigOutput);

   if (driveStrength_ma > 7)
      return BERR_INVALID_PARAMETER;

   if (output == BSRF_OutputSelect_eGpio)
      reg = BCHP_TOP_CTRL_GPIO_PAD_CTRL;
   else if (output == BSRF_OutputSelect_eTestport)
      reg = BCHP_TOP_CTRL_TP_OUT_PAD_CTRL;
   else if (output == BSRF_OutputSelect_eClock)
      reg = BCHP_TOP_CTRL_SRFE_CLK_OUT_PAD_CTRL;
   else if (output == BSRF_OutputSelect_eDataI)
      reg = BCHP_TOP_CTRL_SRFE_DATA_OUT_I_PAD_CTRL;
   else
      reg = BCHP_TOP_CTRL_SRFE_DATA_OUT_Q_PAD_CTRL;

   /* program output slew rate and drive strength */
   BSRF_P_ReadModifyWriteRegister(hChn, reg, ~0x0000000F, (bSlewEdges ? 0x8 : 0) | driveStrength_ma & 0x7);

   BDBG_LEAVE(BSRF_g1_P_ConfigOutput);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_ConfigTestDac
******************************************************************************/
BERR_Code BSRF_g1_P_ConfigTestDac(BSRF_Handle h, int32_t freqHz, int16_t *pCoeff)
{
   BSRF_g1_P_Handle *hDev = (BSRF_g1_P_Handle *)h->pImpl;
   BSRF_ChannelHandle hChn = h->pChannels[0];
   uint32_t freq, P_hi, P_lo, Q_hi, Q_lo;
   uint8_t i;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_ConfigTestDac);

   hDev->dacFreqHz = freqHz;
   if (freqHz < 0)
      freq = -freqHz;
   else
      freq = freqHz;

   /* calculate FCW = (2^32)*12Fc/Fs */
   BMTH_HILO_32TO64_Mul(12 * freq, 4294967295UL, &P_hi, &P_lo);  /* 12Fc*(2^32) */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, BSRF_DAC_SAMPLE_FREQ_KHZ, &Q_hi, &Q_lo);   /* div by Fs */
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, 1000, &Q_hi, &Q_lo);

   /* program test dac fcw */
   if (freqHz < 0) Q_lo = -Q_lo;
   BSRF_P_WriteRegister(hChn, BCHP_SRFE_FE_TEST_FCW, Q_lo);

   /* program xsinx filter taps */
   for (i = 0; i < BSRF_NUM_XSINX_COEFF; i++)
   {
      hDev->xsinxCoeff[i] = pCoeff[i];
      BSRF_P_WriteRegister(hChn, BCHP_SRFE_FE_TEST_XSINX_H0+4*i, pCoeff[i]);
   }

   BDBG_LEAVE(BSRF_g1_P_ConfigTestDac);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_EnableTestDac
******************************************************************************/
BERR_Code BSRF_g1_P_EnableTestDac(BSRF_Handle h)
{
   BSRF_ChannelHandle hChn = h->pChannels[0];

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_EnableTestDac);

   BSRF_P_AndRegister(hChn, BCHP_SRFE_FE_TEST_DAC_PWRDN, ~0x00000001);

   BDBG_LEAVE(BSRF_g1_P_EnableTestDac);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_DisableTestDac
******************************************************************************/
BERR_Code BSRF_g1_P_DisableTestDac(BSRF_Handle h)
{
   BSRF_ChannelHandle hChn = h->pChannels[0];

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_DisableTestDac);

   BSRF_P_OrRegister(hChn, BCHP_SRFE_FE_TEST_DAC_PWRDN, 0x00000001);

   BDBG_LEAVE(BSRF_g1_P_DisableTestDac);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_EnableTestDacTone
******************************************************************************/
BERR_Code BSRF_g1_P_EnableTestDacTone(BSRF_Handle h, bool bToneOn, uint16_t toneAmpl)
{
   BSRF_ChannelHandle hChn = h->pChannels[0];

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_EnableTestDacTone);

   /* limit tone amplitude to 14 bits */
   toneAmpl &= 0x3FFF;

   if (bToneOn)
      BSRF_P_WriteRegister(hChn, BCHP_SRFE_FE_TEST_TONE, (toneAmpl << 16) | toneAmpl);
   else
      BSRF_P_WriteRegister(hChn, BCHP_SRFE_FE_TEST_TONE, 0);

   BDBG_LEAVE(BSRF_g1_P_EnableTestDacTone);
   return BERR_SUCCESS;
}
