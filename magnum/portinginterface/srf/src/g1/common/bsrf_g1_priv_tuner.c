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

static const int16_t iEquCoeff[BSRF_NUM_IQEQ_COEFF_TAPS] =
{
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int16_t qEquCoeff[BSRF_NUM_IQEQ_COEFF_TAPS] =
{
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/******************************************************************************
 BSRF_g1_Tuner_P_Init
******************************************************************************/
BERR_Code BSRF_g1_Tuner_P_Init(BSRF_ChannelHandle h)
{
   uint32_t val;
   uint32_t bbAgcThresh = 0;
   uint8_t bbAgcBandwidth = 0;
   uint8_t iqImbBandwidth = 6;
   uint8_t dcoBandwidth = 12;
   uint8_t notchBandwidth = 12;
   uint8_t i;

   /* override defaults for mxrfga and mpll */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER02_MXRFGA, 0x00001C64);  /* ctl_VCM=3, ctl_bias=4 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER10_MPLL, 0xB7880A00);    /* Cp_ctrl=0xE */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER11_MPLL, 0xB119C56E);    /*  LODIV_test_port_sel=1, LODIV_div_ratio_sel=3*/

   /* override default freq for mpll */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER12_MPLL, 0x0AB1C71C); /* ndiv_int=171, ndiv_fract=116508 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER14_MPLL, 0xB4E83BA0); /* Ibias_VCO_slope=16 */

   /* override default freq for adc pll */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER16_APLL, 0x0AC440C0); /* pdiv=1, ndiv_int=64, refclk_sel=1 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER21_APLL, 0x20040000); /* ch3_mdiv=32 for dco clk, ch2_mdiv=4 for agc clk */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER22_APLL, 0x00000020); /* ch5_Mdiv=32 for ramp clk */

   /* load aci coeffs */
   for (i = 0; i < BSRF_NUM_ACI_COEFF_TAPS; i++)
      BSRF_P_WriteRegister(h, BCHP_SRFE_TABLE_ACI_DATAi_ARRAY_BASE+4*i, aciCoeff[i] & 0xFFFF);

   /* load IQ imbalance equ coeffs */
   for (i = 0; i < BSRF_NUM_IQEQ_COEFF_TAPS; i++)
   {
      BSRF_P_WriteRegister(h, BCHP_SRFE_TABLE_IQEQ_I_DATAi_ARRAY_BASE+4*i, iEquCoeff[i] & 0xFFFF);
      BSRF_P_WriteRegister(h, BCHP_SRFE_TABLE_IQEQ_Q_DATAi_ARRAY_BASE+4*i, qEquCoeff[i] & 0xFFFF);
   }

   /* setup IQ imbalance equ */
   val = 0x03000000 | ((iqImbBandwidth & 0xF) << 12) | (iqImbBandwidth & 0xF);
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_IQ_IMB_CTRL, val);  /* winfrz, cal, bw=6 */

   /* setup bbagc */
   val = 0xA0000000 | (bbAgcBandwidth << 20) | (bbAgcThresh & 0xFFFFF);
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_AGC_CTRL, val);  /* TBD bypass and freeze for bring-up */

   /* setup dco */
   val = 0x00000300 | dcoBandwidth & 0x1F;   /* TBD bypass and freeze for bring-up */
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_DCO_CTRL, val);

   /* setup notch */
   val = 0x00000300 | notchBandwidth & 0x1F; /* TBD bypass and freeze for bring-up */
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_NOTCH_CTRL, val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSRF_g1_Tuner_P_SetFcw
******************************************************************************/
BERR_Code BSRF_g1_Tuner_P_SetFcw(BSRF_ChannelHandle h, int32_t freqHz)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   uint32_t freq, P_hi, P_lo, Q_hi, Q_lo, fcw = 0x743B8424;

   hChn->tunerFreq = freqHz;
   if (freqHz < 0)
      freq = -freqHz;
   else
      freq = freqHz;

   /* calculate FCW = (2^32)*12Fc/Fs */
   BMTH_HILO_32TO64_Mul(12 * freq, 4294967295UL, &P_hi, &P_lo);  /* 12Fc*(2^32) */
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, BSRF_ADC_SAMPLE_FREQ_KHZ, &Q_hi, &Q_lo);   /* div by Fs */
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, 1000, &Q_hi, &Q_lo);

   /* program fcw */
   if (freqHz < 0) Q_lo = -Q_lo;
   BSRF_P_WriteRegister(h, BCHP_SRFE_FE_FCW, Q_lo);

   return BERR_SUCCESS;
}
