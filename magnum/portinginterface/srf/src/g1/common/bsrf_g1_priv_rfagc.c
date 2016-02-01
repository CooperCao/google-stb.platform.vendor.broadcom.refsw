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
#include "bsrf.h"
#include "bsrf_priv.h"
#include "bsrf_g1_priv.h"

BDBG_MODULE(bsrf_g1_priv_rfagc);


static const uint32_t rfagcLut[BSRF_RFAGC_LUT_COUNT] =
{
   36,   100,  164,  228,  292,  356,  420,  484,
   548,  612,  676,  740,  804,  868,  932,  996,
   1060, 1124, 1188, 1252, 1316, 1380, 1444, 1508,
   1572, 1636, 1700, 1764, 1828, 1892, 1956, 2020,
   2084, 2148, 2212, 2276, 2340, 2404, 2468, 2532,
   2596, 2660, 2724, 2788, 2852, 2916, 2980, 3044,
   3108, 3172, 3236, 3300, 3364, 3428, 3429, 3430,
   3431, 3432, 3433, 3434, 3435, 3436, 3437, 3438,
   3439, 3440, 3441, 3442, 3443, 3444, 3445, 3446,
   3447, 3448, 3449, 3449, 3449, 3449, 3449, 3449,
   3449, 3449, 3449, 3449, 3449, 3449, 3449, 3449
};


/******************************************************************************
 BSRF_g1_Rfagc_P_Init
******************************************************************************/
BERR_Code BSRF_g1_Rfagc_P_Init(BSRF_ChannelHandle h)
{
   uint8_t i;

   /* load rfagc lut */
   for (i = 0; i < BSRF_RFAGC_LUT_COUNT; i++)
      BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LUT_DATAi_ARRAY_BASE+4*i, rfagcLut[i]);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_Rfagc_P_SetSettings
******************************************************************************/
BERR_Code BSRF_g1_Rfagc_P_SetSettings(BSRF_ChannelHandle h, BSRF_RfAgcSettings settings)
{
   BERR_Code retCode = BERR_SUCCESS;

   /* program attack settings */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_ATTACK_CNT, settings.attackSettings.timeNs);
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_ATTACK_CTRL, ~0x0000FFFF, settings.attackSettings.threshold & 0xFFFF);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_ATTACK_INC, settings.attackSettings.step & 0x1FFFF);

   /* program decay settings */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_DECAY_CNT, settings.decaySettings.timeNs);
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_DECAY_CTRL, ~0x0000FFFF, settings.decaySettings.threshold & 0xFFFF);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_DECAY_INC, settings.decaySettings.step & 0x1FFFF);

   /* program fast decay settings */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FDECAY_CNT, settings.fastDecaySettings.timeNs);
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FDECAY_CTRL, ~0x0000FFFF, settings.fastDecaySettings.threshold & 0xFFFF);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_FDECAY_INC, settings.fastDecaySettings.step & 0x1FFFF);

   /* program clip window settings */
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_WIN_N, settings.clipWindowSettings.timeNs);
   BSRF_P_WriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_WIN_M, settings.clipWindowSettings.threshold);
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_RFAGC_LOOP_REG_WIN_CTRL, ~0x00FFFF80, (settings.clipWindowSettings.step & 0x1FFFF) << 7);

   /* program other settings */
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER07_RAMP, ~0x000000FF, settings.agcSlewRate & 0xFF);
#if 0
   /* TBD */
   SRFE_RFAGC_LOOP_REG_AGC_LF_INT_WDATA
   SRFE_RFAGC_LOOP_REG_AGC_LA_INT_WDATA
   settings.powerMeasureBw;
#endif

   return retCode;
}
