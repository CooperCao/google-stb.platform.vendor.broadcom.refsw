/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/
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
   BKNI_Memset((void*)hDev->pChannels, 0, BSRF_NUM_CHANNELS * sizeof(BSRF_P_ChannelHandle *));

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
   extern const uint32_t BSRF_g1_ChannelIntrID[BSRF_g1_MaxIntID];

   BERR_Code retCode = BERR_SUCCESS;
   BSRF_ChannelSettings chnSettings;
   BSRF_ChannelHandle chnHandle;
   BSRF_g1_P_ChannelHandle *chG1;
   BSRF_g1_P_Handle *hDev = h->pImpl;

#ifdef BSRF_SXM_OVERRIDE
   uint8_t i;

#if (BCHP_CHIP==lonestar)
   /* sxm request */
   #define BSRF_SXM_NUM_AGC_CODES_DELETE 19
   uint32_t idxLutOmit[BSRF_SXM_NUM_AGC_CODES_DELETE] =
   {
      34, 36, 46, 48, 54, 56, 58, 60,
      62, 64, 66, 68, 70, 72, 74, 76,
      77, 78, 79
   };
#else
   /* fabian request */
   #define BSRF_SXM_NUM_AGC_CODES_DELETE 8
   uint32_t idxLutOmit[BSRF_SXM_NUM_AGC_CODES_DELETE] =
   {
      34, 36, 38, 40, 42, 43, 53, 55
   };
#endif
#endif

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

   chG1->rfagcSettings.attackSettings.timeNs = 4000;
   chG1->rfagcSettings.attackSettings.threshold = -1081344;    /* -16.5dBFS scaled 2^16 */
   chG1->rfagcSettings.attackSettings.step = -1536;            /* -0.75dB scaled 2^11 */
   chG1->rfagcSettings.decaySettings.timeNs = 4000;
   chG1->rfagcSettings.decaySettings.threshold = -1081344;     /* -16.5dBFS scaled 2^16 */
   chG1->rfagcSettings.decaySettings.step = 1536;              /* 0.75dB scaled 2^11 */
   chG1->rfagcSettings.fastDecaySettings.timeNs = 4000;
   chG1->rfagcSettings.fastDecaySettings.threshold = -1081344; /* -16.5dBFS scaled 2^16 */
   chG1->rfagcSettings.fastDecaySettings.step = 1536;          /* 0.75dB scaled 2^11 */
   chG1->rfagcSettings.clipWindowSettings.timeNs = 500;
   chG1->rfagcSettings.clipWindowSettings.threshold = 10;      /* percentage of clip window 0 to 100 */
   chG1->rfagcSettings.clipWindowSettings.step = -6144;        /* -3dB scaled 2^11 */
   chG1->rfagcSettings.powerMeasureBw = 0;
   chG1->rfagcSettings.agcSlewRate = 0xBA;

   /* reset omission list to all false */
   BKNI_Memset((void*)(chG1->bOmitRfagcLut), 0, BSRF_RFAGC_LUT_COUNT * sizeof(bool));
   chG1->numRfagcLutOmitted = 0;

   chG1->bEnableFastDecay = false;
   chG1->bAntennaSenseEnabled = false;
   chG1->modeAoc = 2;
   chG1->modeAd = 1;
   chG1->fastDecayGainThr = 0;

#ifdef BSRF_SXM_OVERRIDE
   /* agc codes to omit */
   for (i = 0; i < BSRF_SXM_NUM_AGC_CODES_DELETE; i++)
      chG1->bOmitRfagcLut[idxLutOmit[i]] = true;

   chG1->numRfagcLutOmitted = BSRF_SXM_NUM_AGC_CODES_DELETE;
   chG1->fastDecayGainThr = -32;
   chG1->bEnableFastDecay = true;

   chG1->rfagcSettings.attackSettings.timeNs = 40000;
   chG1->rfagcSettings.attackSettings.threshold = -1268777;    /* -15.5dBFS scaled 2^16 */
   chG1->rfagcSettings.attackSettings.step = -2048;            /* -1dB scaled 2^11 */
   chG1->rfagcSettings.decaySettings.timeNs = 40000;
   chG1->rfagcSettings.decaySettings.threshold = -1268777;     /* -15.5dBFS scaled 2^16 */
   chG1->rfagcSettings.decaySettings.step = 2048;              /* 1dB scaled 2^11 */
   chG1->rfagcSettings.fastDecaySettings.timeNs = 40000;
   chG1->rfagcSettings.fastDecaySettings.threshold = -1268777; /* -15.5dBFS scaled 2^16 */
   chG1->rfagcSettings.fastDecaySettings.step = 2048;          /* 1dB scaled 2^11 */
   chG1->rfagcSettings.clipWindowSettings.timeNs = 2000;
   chG1->rfagcSettings.clipWindowSettings.threshold = 25;      /* percentage of clip window 0 to 100 */
   chG1->rfagcSettings.clipWindowSettings.step = -20480;       /* -10dB scaled 2^11 */
   chG1->rfagcSettings.powerMeasureBw = 5;
   chG1->rfagcSettings.agcSlewRate = 0xB9;
#endif

   /* setup interrupts */
   retCode = BINT_CreateCallback(&(chG1->hAttackCountOvfCb), hDev->hInterrupt,
                  BSRF_g1_ChannelIntrID[BSRF_g1_IntID_eAttackCountOvf],
                  BSRF_g1_P_AttackCountOvf_isr, (void*)chnHandle, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   retCode = BINT_CreateCallback(&(chG1->hDecayCountOvfCb), hDev->hInterrupt,
                  BSRF_g1_ChannelIntrID[BSRF_g1_IntID_eDecayCountOvf],
                  BSRF_g1_P_DecayCountOvf_isr, (void*)chnHandle, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   retCode = BINT_CreateCallback(&(chG1->hFsCountOvfCb), hDev->hInterrupt,
                  BSRF_g1_ChannelIntrID[BSRF_g1_IntID_eFsCountOvf],
                  BSRF_g1_P_FsCountOvf_isr, (void*)chnHandle, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   retCode = BINT_CreateCallback(&(chG1->hWinDetectCb), hDev->hInterrupt,
                  BSRF_g1_ChannelIntrID[BSRF_g1_IntID_eWinDetect],
                  BSRF_g1_P_WinDetect_isr, (void*)chnHandle, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   retCode = BINT_CreateCallback(&(chG1->hRampActiveCb), hDev->hInterrupt,
                  BSRF_g1_ChannelIntrID[BSRF_g1_IntID_eRampActive],
                  BSRF_g1_P_RampActive_isr, (void*)chnHandle, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

   retCode = BINT_CreateCallback(&(chG1->hRampInactiveCb), hDev->hInterrupt,
                  BSRF_g1_ChannelIntrID[BSRF_g1_IntID_eRampInactive],
                  BSRF_g1_P_RampInactive_isr, (void*)chnHandle, 0);
   BDBG_ASSERT(retCode == BERR_SUCCESS);

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
   int16_t xsinx[BSRF_NUM_XSINX_COEFF] = {0, 0, 0, 0x1000};
   uint8_t i;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_Reset);

   for (i = 0; i < BSRF_NUM_CHANNELS; i++)
   {
      /* skip channel if not opened */
      hChn = h->pChannels[i];
      if (hChn == NULL)
         continue;

      /*  must power up plls before digital access */
      BSRF_g1_P_PowerUp(hChn);

      /* init tuner and load rfagc lut */
      BSRF_g1_Tuner_P_Init(hChn);
      BSRF_g1_Rfagc_P_Init(hChn);

#if 0 /* TBD */
      /* power down by default */
      BSRF_g1_P_PowerDown(hChn);
#endif
   }

   /* default test dac config */
   BSRF_g1_P_ConfigTestDac(h, 50000000, xsinx);

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
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_PowerUp);

   /* power up analog */
   BSRF_g1_Ana_P_PowerUp(h);
   BSRF_g1_Ana_P_CalibrateCaps(h);

   /* enable clocks */
   BSRF_P_OrRegister(h, BCHP_SRFE_RFAGC_LOOP_CLK_CTRL, 0x00000001);
   BSRF_P_OrRegister(h, BCHP_SRFE_FE_CLK_CTRL, 0x00000003);

   /* toggle rfagc reset */
   BSRF_P_ToggleBit(h, BCHP_SRFE_RFAGC_LOOP_RST_CTRL, 0x00000003);

   /* toggle srfe resets */
   BSRF_P_ToggleBit(h, BCHP_SRFE_FE_RST_CTRL, 0x00000003);

   /* enable rfagc */
   BSRF_P_OrRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL1, 0x40000000);

   /* TBD set manual PGA and LNA Gain Control Code */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_PGA_GCC, 0x00000020);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_LNA_GCC, 0x00000001);

   /* restore antenna sense thresholds */
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, ~0x000000C0, (hChn->modeAoc & 0x3) << 6);
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, ~0x00000030, (hChn->modeAd & 0x3) << 4);

   /* enable clip interrupt */
   BINT_EnableCallback(hChn->hWinDetectCb);

   h->bEnabled = true;

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

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* set state to powered off */
   h->bEnabled = false;

#if 0
   /* disable test clock */
   BSRF_P_AndRegister(h, BCHP_SRFE_RFAGC_LOOP_CLK_CTRL, ~0x00000001);
#endif

   /* disable rfagc */
   BSRF_P_AndRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL1, ~0x40000000);

   /* power down analog */
   BSRF_g1_Ana_P_PowerDown(h);

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

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

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

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

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

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

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

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* read rfagc integrator */
   BSRF_P_ReadRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LF_INT_WDATA, pVal);

   BDBG_LEAVE(BSRF_g1_P_ReadRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_WriteRfGain
******************************************************************************/
BERR_Code BSRF_g1_P_WriteRfGain(BSRF_ChannelHandle h, uint8_t gain)
{
   uint32_t val;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_WriteRfGain);

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* cap gaincode at 74 */
   if (gain > 74)
      gain = 74;

   /* convert gain to integrator */
   val = (gain - 64) << 24;

   /* override rfagc integrator */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LF_INT_WDATA, val);

   BDBG_LEAVE(BSRF_g1_P_WriteRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_ReadRfGain
******************************************************************************/
BERR_Code BSRF_g1_P_ReadRfGain(BSRF_ChannelHandle h, uint8_t *pGain)
{
   uint32_t val;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_ReadRfGain);

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* read rfagc integrator */
   BSRF_P_ReadRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LF_INT_WDATA, &val);

   /* convert integrator to gain */
   *pGain = (val >> 24) + 64;

   BDBG_LEAVE(BSRF_g1_P_ReadRfAgc);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetInputPower
******************************************************************************/
#define BSRF_AGC_GAIN_MAX 60
#define BSRF_AGC_GAIN_THR 42
BERR_Code BSRF_g1_P_GetInputPower(BSRF_ChannelHandle h, int32_t *pPower)
{
   uint32_t val, laAgcInt, lalog;
   uint8_t agcGain;

   #define BSRF_POWER_B  -1658061 /* -25.3 scaled up 2^16 */
   #define BSRF_POWER_M  -57672   /* -0.88 scaled up 2^16 */
   #define BSRF_POWER_B1 996475   /* 15.205 scaled up 2^16 */
   #define BSRF_POWER_FF -941752  /* -14.37 scaled up 2^16 */  /* mean of 10*log10(la/2^31) */

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetInputPower);

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* read agc gain */
   BSRF_P_ReadRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_STATUS, &val);
   agcGain = val & 0x7F;

   /* read leaky integrator */
   BSRF_P_ReadRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LA_INT_WDATA, &laAgcInt);

   /* lalog = 10*log(la/2^31) */
   lalog = (BMTH_2560log10(laAgcInt) << 8) - 6115774;  /* scaled up 2^16 */
	if (agcGain >= BSRF_AGC_GAIN_MAX)
   {
		*pPower = lalog + BSRF_POWER_B;
   }
	else
   {
      *pPower = BSRF_POWER_M * agcGain + BSRF_POWER_B1 + lalog - BSRF_POWER_FF;

      /* adjustment */
      if (agcGain > BSRF_AGC_GAIN_THR)
         *pPower -= (1 << 16);
	}
   *pPower /= 256;   /* scale down to 2^8 */

   BDBG_LEAVE(BSRF_g1_P_GetInputPower);
   return BERR_SUCCESS;
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

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   hChn->rfagcSettings = settings;
   retCode = BSRF_g1_Rfagc_P_SetSettings(h, settings);

   BDBG_LEAVE(BSRF_g1_P_SetRfAgcSettings);
   return retCode;
}


/******************************************************************************
 BSRF_g1_P_GetRfAgcSettings
******************************************************************************/
BERR_Code BSRF_g1_P_GetRfAgcSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings *pSettings)
{
   BERR_Code retCode = BERR_SUCCESS;
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetRfAgcSettings);

   /* return saved settings */
   *pSettings = hChn->rfagcSettings;

   BDBG_LEAVE(BSRF_g1_P_GetRfAgcSettings);
   return retCode;
}


/******************************************************************************
 BSRF_g1_P_EnableFastDecayMode
******************************************************************************/
BERR_Code BSRF_g1_P_EnableFastDecayMode(BSRF_ChannelHandle h, bool bEnable)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_EnableFastDecayMode);

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   hChn->bEnableFastDecay = bEnable;

   if (bEnable)
      BSRF_P_OrRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL2, 0x80000000);
   else
      BSRF_P_AndRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_CTRL2, ~0x80000000)

   BDBG_LEAVE(BSRF_g1_P_EnableFastDecayMode);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_SetFastDecayGainThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_SetFastDecayGainThreshold(BSRF_ChannelHandle h, int8_t threshold)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_SetFastDecayGainThreshold);

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* configure fast decay gain threshold */
   hChn->fastDecayGainThr = threshold;
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FDECAY_REF, hChn->fastDecayGainThr & 0x7F);

   BDBG_LEAVE(BSRF_g1_P_SetFastDecayGainThreshold);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetFastDecayGainThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_GetFastDecayGainThreshold(BSRF_ChannelHandle h, int8_t *pThreshold)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetFastDecayGainThreshold);

   /* read fast decay gain threshold */
   BSRF_P_ReadRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FDECAY_REF, &val);
   if (val & 0x40)
      val += 0xFFFFFF80;   /* sign extension */
   hChn->fastDecayGainThr = val;
   *pThreshold = hChn->fastDecayGainThr;

   BDBG_LEAVE(BSRF_g1_P_GetFastDecayGainThreshold);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_SetAntennaOverThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_SetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t mode)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_SetAntennaOverThreshold);

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;
   if (!hChn->bAntennaSenseEnabled)
      return BSRF_ERR_POWERED_DOWN;

   hChn->modeAoc = mode & 0x3;
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, ~0x000000C0, hChn->modeAoc << 6);

   BDBG_LEAVE(BSRF_g1_P_SetAntennaOverThreshold);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetAntennaOverThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_GetAntennaOverThreshold(BSRF_ChannelHandle h, uint8_t *pMode)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetAntennaOverThreshold);

   BSRF_P_ReadRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, &val);
   hChn->modeAoc = (val >> 6) & 0x3;
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

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;
   if (!hChn->bAntennaSenseEnabled)
      return BSRF_ERR_POWERED_DOWN;

   hChn->modeAd = mode & 0x3;
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, ~0x00000030, hChn->modeAd << 4);

   BDBG_LEAVE(BSRF_g1_P_SetAntennaDetectThreshold);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetAntennaDetectThreshold
******************************************************************************/
BERR_Code BSRF_g1_P_GetAntennaDetectThreshold(BSRF_ChannelHandle h, uint8_t *pMode)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetAntennaDetectThreshold);

   BSRF_P_ReadRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, &val);
   hChn->modeAd = (val >> 4) & 0x3;
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

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;
   if (!hChn->bAntennaSenseEnabled)
      return BSRF_ERR_POWERED_DOWN;

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
BERR_Code BSRF_g1_P_Tune(BSRF_ChannelHandle h, uint32_t freqHz)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_Tune);

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* verify range of center freq */
   if ((freqHz > (BSRF_DEFAULT_FC_HZ + 50000000)) || (freqHz < (BSRF_DEFAULT_FC_HZ - 50000000)))
      return BERR_INVALID_PARAMETER;

   /* save center freq if valid */
   hChn->tunerFreq = freqHz;

   BSRF_g1_Tuner_P_SetNotchFcw(h, BSRF_NOTCH_FREQ_HZ);
   BSRF_g1_Tuner_P_SetFcw(h, 2310000000 - freqHz - BSRF_NOTCH_FREQ_HZ);

   BDBG_LEAVE(BSRF_g1_P_Tune);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_GetTunerStatus
******************************************************************************/
BERR_Code BSRF_g1_P_GetTunerStatus(BSRF_ChannelHandle h, BSRF_TunerStatus *pStatus)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_GetTunerStatus);

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* retrieve status parameters */
   pStatus->tunerFreqHz = hChn->tunerFreq;

   BSRF_P_ReadRegister(h, BCHP_SRFE_ANA_READR03, &val);
   if ((val & 0x6) == 0x2)
      pStatus->bPllLock = true;
   else
      pStatus->bPllLock = false;

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

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

#if (BCHP_CHIP==lonestar)
   /* setup clip count, sig = 6 for IQ EQ output (tc1.15), thres = 32112 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_CLIP_CTRL, 0x00067D70);

   /* reset clip counter */
   BSRF_P_ToggleBit(h, BCHP_SRFE_FE_CLIP_CTRL, 0x00100000);
#else
   /* reset clip counter for 89730 */
   BSRF_P_ToggleBit(h, BCHP_SRFE_RFAGC_LOOP_REG_WIN_CTRL, 0x02000000);
#endif

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

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* read clip counter */
#if (BCHP_CHIP==lonestar)
   BSRF_P_ReadRegister(h, BCHP_SRFE_FE_CLIP_COUNT, pClipCount);
#else
   BSRF_P_ReadRegister(h, BCHP_SRFE_RFAGC_LOOP_CLIP_CNT, pClipCount);
#endif

   BDBG_LEAVE(BSRF_g1_P_GetClipCount);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_ConfigTestMode
******************************************************************************/
BERR_Code BSRF_g1_P_ConfigTestMode(BSRF_ChannelHandle h, BSRF_TestportSelect tp, bool bEnable)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_ConfigTestMode);

   if (tp > BSRF_TestportSelect_eIfOutput)
      return BERR_INVALID_PARAMETER;

   if (tp == BSRF_TestportSelect_eLoPll)
   {
      /* to enable pll lo */
      if (bEnable)
         BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER11_MPLL, ~0x30000000, 0x80000000);
      else
         BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER11_MPLL, ~0x80000000, 0x30000000);
   }

   if (tp == BSRF_TestportSelect_eIfInput)
   {
      /* to enable if testport as output */
      if (bEnable)
      {
         BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER08_BGTST, ~0x00FFFC00, 0x00004400);
         BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, ~0x00000023);
         BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, ~0x0000003C, 0x00000040);
      }
      else
      {
         BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER08_BGTST, ~0x00FFFC00);
         BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, 0x00000023);
         BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, ~0x00000040, 0x0000003C);
      }
   }

   if (tp == BSRF_TestportSelect_eIfOutput)
   {
      /* to enable if testport as output */
      if (bEnable)
         BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER08_BGTST, ~0x00FFFC00, 0x00004400);
      else
         BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER08_BGTST, ~0x00FFFC00);
   }

   BDBG_LEAVE(BSRF_g1_P_ConfigTestMode);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_ConfigOutput
******************************************************************************/
BERR_Code BSRF_g1_P_ConfigOutput(BSRF_Handle h, BSRF_OutputSelect output, bool bSlewEdges, uint8_t driveStrength_ma)
{
#if (BCHP_CHIP==lonestar)
   BSRF_ChannelHandle hChn = h->pChannels[0];
   uint32_t reg;
   uint8_t driveStrength = 0;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_ConfigOutput);

   if (driveStrength_ma > 16)
      return BERR_INVALID_PARAMETER;

   if (driveStrength_ma > 0)
      driveStrength = (driveStrength_ma + 1) / 2 - 1;

   if (output == BSRF_OutputSelect_eGpio)
      reg = BCHP_TOP_CTRL_GPIO_PAD_CTRL;
   else if (output == BSRF_OutputSelect_eTestport)
      reg = BCHP_TOP_CTRL_TP_OUT_PAD_CTRL;
   else if (output == BSRF_OutputSelect_eClock)
      reg = BCHP_TOP_CTRL_SRFE_CLK_OUT_PAD_CTRL;
   else if (output == BSRF_OutputSelect_eDataI)
      reg = BCHP_TOP_CTRL_SRFE_DATA_OUT_I_PAD_CTRL;
   else if (output == BSRF_OutputSelect_eDataQ)
      reg = BCHP_TOP_CTRL_SRFE_DATA_OUT_Q_PAD_CTRL;
   else
      return BERR_INVALID_PARAMETER;

   /* program output slew rate and drive strength */
   BSRF_P_ReadModifyWriteRegister(hChn, reg, ~0x0000000F, (bSlewEdges ? 0x8 : 0) | driveStrength & 0x7);

   BDBG_LEAVE(BSRF_g1_P_ConfigOutput);
   return BERR_SUCCESS;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BSRF_g1_P_ConfigTestDac
******************************************************************************/
BERR_Code BSRF_g1_P_ConfigTestDac(BSRF_Handle h, int32_t freqHz, int16_t *pCoeff)
{
#if (BCHP_CHIP==lonestar)
   BSRF_g1_P_Handle *hDev = (BSRF_g1_P_Handle *)h->pImpl;
   BSRF_ChannelHandle hChn = h->pChannels[0];
   uint32_t freq, P_hi, P_lo, Q_hi, Q_lo;
   uint8_t i;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_ConfigTestDac);

   if (!hChn->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   hDev->dacFreqHz = freqHz;
   if (freqHz < 0)
      freq = -freqHz;
   else
      freq = freqHz;

   /* calculate FCW = (2^32)*6Fc/Fs */
   BMTH_HILO_32TO64_Mul(freq, 4294967295UL, &P_hi, &P_lo);  /* Fc*(2^32) */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, BSRF_DAC_SAMPLE_FREQ_KHZ, &Q_hi, &Q_lo);   /* div by Fs_dac */
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
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BSRF_g1_P_EnableTestDac
******************************************************************************/
BERR_Code BSRF_g1_P_EnableTestDac(BSRF_Handle h)
{
#if (BCHP_CHIP==lonestar)
   BSRF_ChannelHandle hChn = h->pChannels[0];

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_EnableTestDac);

   if (!hChn->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* offset binary format */
   BSRF_P_OrRegister(hChn, BCHP_SRFE_FE_TEST_CTRL, 0x00000004);

   /* set dac output power and release reset */
   BSRF_P_WriteRegister(hChn, BCHP_SRFE_FE_TEST_DAC_CTRL0, 0x18100000); /* Ifs_ctrl=192, rstb_crc_lfsr_saw=1 */
   BSRF_P_AndRegister(hChn, BCHP_SRFE_FE_TEST_DAC_PWRDN, ~0x00000001);

   BDBG_LEAVE(BSRF_g1_P_EnableTestDac);
   return BERR_SUCCESS;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BSRF_g1_P_DisableTestDac
******************************************************************************/
BERR_Code BSRF_g1_P_DisableTestDac(BSRF_Handle h)
{
#if (BCHP_CHIP==lonestar)
   BSRF_ChannelHandle hChn = h->pChannels[0];

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_DisableTestDac);

   if (!hChn->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   BSRF_P_OrRegister(hChn, BCHP_SRFE_FE_TEST_DAC_PWRDN, 0x00000001);

   BDBG_LEAVE(BSRF_g1_P_DisableTestDac);
   return BERR_SUCCESS;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BSRF_g1_P_EnableTestDacTone
******************************************************************************/
BERR_Code BSRF_g1_P_EnableTestDacTone(BSRF_Handle h, bool bToneOn, uint16_t toneAmpl)
{
#if (BCHP_CHIP==lonestar)
   BSRF_ChannelHandle hChn = h->pChannels[0];

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_EnableTestDacTone);

   if (!hChn->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   if (bToneOn)
   {
      BSRF_P_OrRegister(hChn, BCHP_SRFE_FE_TEST_CTRL, 0x00000002);
      BSRF_P_WriteRegister(hChn, BCHP_SRFE_FE_TEST_TONE, toneAmpl & 0x3FFF);
   }
   else
   {
      BSRF_P_AndRegister(hChn, BCHP_SRFE_FE_TEST_CTRL, ~0x00000002);
      BSRF_P_WriteRegister(hChn, BCHP_SRFE_FE_TEST_TONE, 0);
   }

   BDBG_LEAVE(BSRF_g1_P_EnableTestDacTone);
   return BERR_SUCCESS;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BSRF_g1_P_RunDataCapture
******************************************************************************/
#define BSRF_DATA_CAPT_SIZE 8192 /* default to max capt size */
BERR_Code BSRF_g1_P_RunDataCapture(BSRF_Handle h)
{
#if (BCHP_CHIP==lonestar)
   BSRF_ChannelHandle hChn = h->pChannels[0];
   uint32_t waitTimeMs;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_EnableDataCapture);

   if (!hChn->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* adc data sent to diag testport, select attack integrator, assert fe_fifo_rst, select fe datapath */
   BSRF_P_WriteRegister(hChn, BCHP_SRFE_RFAGC_LOOP_TP_OUT_CTRL, 0x00000112);

   /* digital data, select aci output, output i/q pair, 3-bit 2's comp */
   BSRF_P_WriteRegister(hChn, BCHP_SRFE_FE_TP_CTRL, 0x00000153);

   BSRF_P_WriteRegister(hChn, BCHP_DIAG_CAPT_NEW_SIZE, BSRF_DATA_CAPT_SIZE);     /* capt_size=8192 */
   BSRF_P_WriteRegister(hChn, BCHP_DIAG_CAPT_NEW_CONTROL, 0x00030100);  /* 64-bit capt width */
   BSRF_P_WriteRegister(hChn, BCHP_DIAG_CAPT_NEW_ENABLE, 0x00000001);   /* enable diag capture */

   /* enable rfagc testport, release fifo reset */
   BSRF_P_ReadModifyWriteRegister(hChn, BCHP_SRFE_RFAGC_LOOP_TP_OUT_CTRL, ~0x00000002, 0x00000004);

   /* wait for data capture to complete */
   waitTimeMs = (BSRF_DATA_CAPT_SIZE  * 1100 / (BSRF_ADC_SAMPLE_FREQ_KHZ/8)) + 2;
   BKNI_Sleep(waitTimeMs);

   /* re-assert fifo reset */
   BSRF_P_OrRegister(hChn, BCHP_SRFE_RFAGC_LOOP_TP_OUT_CTRL, 0x00000002);

   /* reset data buffer index */
   BSRF_P_WriteRegister(hChn, BCHP_DIAG_CAPT_NEW_DATA_RD_IDX, 0);

   BDBG_LEAVE(BSRF_g1_P_EnableDataCapture);
   return BERR_SUCCESS;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


/******************************************************************************
 BSRF_g1_P_DeleteAgcLutCodes
******************************************************************************/
BERR_Code BSRF_g1_P_DeleteAgcLutCodes(BSRF_Handle h, uint32_t *pIdx, uint32_t n)
{
   BSRF_ChannelHandle hChn = h->pChannels[0];
   BSRF_g1_P_ChannelHandle *hChnImpl = (BSRF_g1_P_ChannelHandle *)hChn->pImpl;
   uint8_t i;

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_DeleteAgcLutCodes);

   if (n > BSRF_RFAGC_LUT_COUNT)
      return BERR_INVALID_PARAMETER;

   /* reset omission list to all false */
   BKNI_Memset((void*)(hChnImpl->bOmitRfagcLut), 0, BSRF_RFAGC_LUT_COUNT * sizeof(bool));

   for (i = 0; i < n; i++)
      hChnImpl->bOmitRfagcLut[pIdx[i]] = true;

   hChnImpl->numRfagcLutOmitted = n;

   /* reload rfagc lut */
   BSRF_g1_Rfagc_P_Init(hChn);

   BDBG_LEAVE(BSRF_g1_P_DeleteAgcLutCodes);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_ConfigOutputClockPhase
******************************************************************************/
BERR_Code BSRF_g1_P_ConfigOutputClockPhase(BSRF_Handle h, uint8_t phase, bool bDisableOutput)
{
   BSRF_ChannelHandle hChn = h->pChannels[0];

   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_ConfigOutputClockPhase);

   if (!hChn->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   if (phase > 5)
      return BERR_INVALID_PARAMETER;

   /* shift clock phase by phase/6 of the period */
   BSRF_P_ReadModifyWriteRegister(hChn, BCHP_SRFE_FE_OI_CTRL, ~0x00000007, phase & 0x7);

   if (bDisableOutput)
      BSRF_P_OrRegister(hChn, BCHP_SRFE_FE_OI_CTRL, 0x00000080);
   else
      BSRF_P_AndRegister(hChn, BCHP_SRFE_FE_OI_CTRL, ~0x00000080);

   BDBG_LEAVE(BSRF_g1_P_ConfigOutputClockPhase);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_SetIqEqCoeff
******************************************************************************/
BERR_Code BSRF_g1_P_SetIqEqCoeff(BSRF_ChannelHandle h, int16_t *iTaps, int16_t *qTaps)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_SetIqEqCoeff);

   if (!h->bEnabled)
      return BSRF_ERR_POWERED_DOWN;

   /* program IQ imbalance equalization taps */
   BSRF_g1_Tuner_P_SetIqEqCoeff(h, iTaps, qTaps);

   BDBG_LEAVE(BSRF_g1_P_SetIqEqCoeff);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_SetRfAgcSettings
******************************************************************************/
BERR_Code BSRF_g1_P_SetIqEqSettings(BSRF_ChannelHandle h, BSRF_IqEqSettings settings)
{
   BDBG_ASSERT(h);
   BDBG_ENTER(BSRF_g1_P_SetIqEqSettings);

   /* program IQ imbalance equalization settings */
   BSRF_g1_Tuner_P_SetIqEqSettings(h, settings);

   BDBG_LEAVE(BSRF_g1_P_SetIqEqSettings);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_P_AttackCountOvf_isr() - ISR context
******************************************************************************/
void BSRF_g1_P_AttackCountOvf_isr(void *p, int param)
{
   BSRF_ChannelHandle h = (BSRF_ChannelHandle)p;
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   BSTD_UNUSED(param);
   BINT_DisableCallback(hChn->hAttackCountOvfCb);
}


/******************************************************************************
 BSRF_g1_P_DecayCountOvf_isr() - ISR context
******************************************************************************/
void BSRF_g1_P_DecayCountOvf_isr(void *p, int param)
{
   BSRF_ChannelHandle h = (BSRF_ChannelHandle)p;
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   BSTD_UNUSED(param);
   BINT_DisableCallback(hChn->hDecayCountOvfCb);
}


/******************************************************************************
 BSRF_g1_P_FsCountOvf_isr() - ISR context
******************************************************************************/
void BSRF_g1_P_FsCountOvf_isr(void *p, int param)
{
   BSRF_ChannelHandle h = (BSRF_ChannelHandle)p;
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   BSTD_UNUSED(param);
   BINT_DisableCallback(hChn->hFsCountOvfCb);
}


/******************************************************************************
 BSRF_g1_P_WinDetect_isr() - ISR context
******************************************************************************/
#define BSRF_AGC_TRK_M -27820    /* -8.291e-4 scaled -9.25 */
#define BSRF_AGC_TRK_B -65206496 /* -1.9433 scaled -9.25 */
void BSRF_g1_P_WinDetect_isr(void *p, int param)
{
   BSRF_ChannelHandle h = (BSRF_ChannelHandle)p;
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
#ifdef BSRF_TEST_RFAGC_CLIP_CONVERGE
   uint32_t val, x;  /* TBD is log2 always positive? */
   int32_t y;
   uint8_t gain;
#endif

   BSTD_UNUSED(param);

#ifdef BSRF_TEST_RFAGC_CLIP_CONVERGE
   BKNI_Printf("!BSRF_g1_P_WinDetect_isr: ");

   BSRF_P_ReadRegister(h, BCHP_SRFE_RFAGC_LOOP_AGC_STATUS, &val); /* log2 in 16.16 format */
   gain = val & 0x7F;   /* gain is 7 bits */
   x = val >> 16;       /* drop non-existent frac bits for 16.0 format */

   BKNI_Printf("status=%08X\n", val);
   y = BSRF_AGC_TRK_M * x + BSRF_AGC_TRK_B;  /* 7.25 format */
   BKNI_Printf("%08X*(%08X)+%08X=%08X | ", BSRF_AGC_TRK_M, x, BSRF_AGC_TRK_B, y);
   BKNI_Printf("%08X>>25=%08X+%02X=", y, y >> 25, gain, y);
   y = (y >> 25) + gain;
   BKNI_Printf("%02X\n", y);

   BSRF_P_ReadRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LF_INT_WDATA, &val);   /* tc8.24 format */
   BKNI_Printf("LF_INT:%08X->", val);
   val = (val + (y << 24)) | 0xC0000000;
   BKNI_Printf("%08X\n", val);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LF_INT_WDATA, val);   /* tc8.24 format */
#else
   /* clear leaky averager */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_AGC_LA_INT_WDATA, 0);
#endif
}


/******************************************************************************
 BSRF_g1_P_RampActive_isr() - ISR context
******************************************************************************/
void BSRF_g1_P_RampActive_isr(void *p, int param)
{
   BSRF_ChannelHandle h = (BSRF_ChannelHandle)p;
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   BSTD_UNUSED(param);
   BINT_DisableCallback(hChn->hRampActiveCb);
}


/******************************************************************************
 BSRF_g1_P_RampInactive_isr() - ISR context
******************************************************************************/
void BSRF_g1_P_RampInactive_isr(void *p, int param)
{
   BSRF_ChannelHandle h = (BSRF_ChannelHandle)p;
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   BSTD_UNUSED(param);
   BINT_DisableCallback(hChn->hRampInactiveCb);
}
