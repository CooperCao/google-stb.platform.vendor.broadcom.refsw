/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bsrf.h"
#include "bsrf_priv.h"
#include "bsrf_g1_priv.h"

BDBG_MODULE(bsrf_g1_priv_tuner);


static const int16_t aciCoeff[BSRF_NUM_ACI_COEFF_TAPS] =
{
   6, 32, 25, -7, -15, 11, 12, -16,
   -10, 21, 6, -26, -1, 31, -7, -34,
   16, 35, -27, -34, 40, 29, -53, -20,
   66, 6, -76, 13, 83, -35, -84, 61,
   79, -89, -66, 117, 44, -142, -13, 162,
   -27, -174, 75, 175, -128, -162, 185, 133,
   -242, -87, 293, 21, -334, 63, 359, -165,
   -363, 282, 339, -411, -280, 547, 179, -687,
   -27, 823, -190, -951, 498, 1065, -950,-1160,
   1685, 1231, -3230, -1275, 10347, 17674
};

static int16_t iEquCoeff[BSRF_NUM_IQEQ_COEFF_TAPS] =
{
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int16_t qEquCoeff[BSRF_NUM_IQEQ_COEFF_TAPS] =
{
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/******************************************************************************
 BSRF_g1_Tuner_P_Init
******************************************************************************/
BERR_Code BSRF_g1_Tuner_P_Init(BSRF_ChannelHandle h)
{
   BSRF_IqEqSettings iqeqSettings;
   uint32_t val;
   uint32_t bbAgcThresh = 0xE8000;
   uint8_t bbAgcBandwidth = 12;
   uint8_t iqImbBandwidth = 6;
   uint8_t dcoBandwidth = 12;
   uint8_t notchBandwidth = 15;
   uint8_t i;

   /* load aci coeffs */
   for (i = 0; i < BSRF_NUM_ACI_COEFF_TAPS; i++)
      BSRF_P_WriteRegister(h, BCHP_SRFE_TABLE_ACI_DATAi_ARRAY_BASE+4*i, aciCoeff[i] & 0xFFFF);

   /* load default IQ imbalance equ coeffs */
   BSRF_g1_Tuner_P_SetIqEqCoeff(h, iEquCoeff, qEquCoeff);

   /* set IQ imbalance equ settings */
   iqeqSettings.bBypass = false;
   iqeqSettings.bFreeze = false;
   iqeqSettings.bandwidth = 6;
   iqeqSettings.delay = 0;
   BSRF_g1_Tuner_P_SetIqEqSettings(h, iqeqSettings);

   /* setup bbagc */
   val = 0xB0000000 | (bbAgcBandwidth << 20) | (bbAgcThresh & 0xFFFFF);
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_AGC_CTRL, val);  /* frz=1, byp=1, select aci output */

   /* setup dco */
   val = 0x00000400 | (dcoBandwidth & 0x1F);    /* winfrz=1 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_DCO_CTRL, val);

   /* setup notch */
   val = 0x00000400 | (notchBandwidth & 0x1F);  /* winfrz=1 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_NOTCH_CTRL, val);

   /* set default center freq */
#if 1
   BSRF_g1_P_Tune(h, BSRF_DEFAULT_FC_HZ);
#else
   BSRF_g1_Tuner_P_SetNotchFcw(h, BSRF_NOTCH_FREQ_HZ);
   BSRF_g1_Tuner_P_SetFcw(h, -22500000 - BSRF_NOTCH_FREQ_HZ);
#endif

   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_Tuner_P_SetFcw
******************************************************************************/
BERR_Code BSRF_g1_Tuner_P_SetFcw(BSRF_ChannelHandle h, int32_t freqHz)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   uint32_t freq, P_hi, P_lo, Q_hi, Q_lo, fcw = 0x743B8424;

   if (freqHz < 0)
      freq = -freqHz;
   else
      freq = freqHz;

   /* calculate fcw = (2^32)*12Fc/Fs */
   BMTH_HILO_32TO64_Mul(freq, 4294967295UL, &P_hi, &P_lo);  /* Fc*(2^32) */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, BSRF_FCW_SAMPLE_FREQ_KHZ, &Q_hi, &Q_lo);   /* div by Fs_fcw */
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, 1000, &Q_hi, &Q_lo);

   /* program fcw */
   if (freqHz < 0) Q_lo = -Q_lo;
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_NOTCH_FCW, Q_lo);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_Tuner_P_SetNotchFcw
******************************************************************************/
BERR_Code BSRF_g1_Tuner_P_SetNotchFcw(BSRF_ChannelHandle h, int32_t freqHz)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   uint32_t freq, P_hi, P_lo, Q_hi, Q_lo;

   hChn->notchFreq = freqHz;
   if (freqHz < 0)
      freq = -freqHz;
   else
      freq = freqHz;

   /* calculate fcw = (2^32)*12Fc/Fs */
   BMTH_HILO_32TO64_Mul(freq, 4294967295UL, &P_hi, &P_lo);  /* Fc*(2^32) */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, BSRF_FCW_SAMPLE_FREQ_KHZ, &Q_hi, &Q_lo);   /* div by Fs_fcw */
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, 1000, &Q_hi, &Q_lo);

   /* program fcw */
   if (freqHz < 0) Q_lo = -Q_lo;
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_FCW, Q_lo);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_Tuner_P_SetIqEqCoeff
******************************************************************************/
BERR_Code BSRF_g1_Tuner_P_SetIqEqCoeff(BSRF_ChannelHandle h, int16_t *iTaps, int16_t *qTaps)
{
   uint8_t i;

   /* program IQ imbalance equalization taps */
   for (i = 0; i < BSRF_NUM_IQEQ_COEFF_TAPS; i++)
   {
      BSRF_P_WriteRegister(h, BCHP_SRFE_TABLE_IQEQ_I_DATAi_ARRAY_BASE+4*i, iTaps[i]);
      BSRF_P_WriteRegister(h, BCHP_SRFE_TABLE_IQEQ_Q_DATAi_ARRAY_BASE+4*i, qTaps[i]);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_Tuner_P_ConfigIqEq
******************************************************************************/
BERR_Code BSRF_g1_Tuner_P_SetIqEqSettings(BSRF_ChannelHandle h, BSRF_IqEqSettings settings)
{
   uint32_t val;

   /* setup IQ imbalance equ */
   val = 0x03040040 | ((settings.bandwidth & 0xF) << 12) | (settings.bandwidth & 0xF);
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_IQ_IMB_CTRL, val);  /* winfrz=1, cal=1, phs_byp_nrm=1, amp_byp_nrm=1 */

   if (settings.bBypass)
      BSRF_P_OrRegister(h, BCHP_SRFE_FE_IQ_IMB_CTRL, 0x00020020);
   else
      BSRF_P_AndRegister(h, BCHP_SRFE_FE_IQ_IMB_CTRL, ~0x00020020);

   if (settings.bFreeze)
      BSRF_P_OrRegister(h, BCHP_SRFE_FE_IQ_IMB_CTRL, 0x00010010);
   else
      BSRF_P_AndRegister(h, BCHP_SRFE_FE_IQ_IMB_CTRL, ~0x00010010);

   /* IQ imbalance equalization delay */
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_IQ_EQ_DLY, settings.delay & 0xF);

   return BERR_SUCCESS;
}
