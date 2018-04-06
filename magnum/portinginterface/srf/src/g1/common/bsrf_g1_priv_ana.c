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
#include "bsrf.h"
#include "bsrf_priv.h"
#include "bsrf_g1_priv.h"

BDBG_MODULE(bsrf_g1_priv_ana);


/******************************************************************************
 BSRF_g1_Ana_P_PowerUp
******************************************************************************/
BERR_Code BSRF_g1_Ana_P_PowerUp(BSRF_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;
   uint8_t i;
   bool bPllLocked = false;

   /* override defaults for mxrfga and mixpll */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER02_MXRFGA, 0x00005C64);  /* IselAGC=1, ctl_VCM=3, ctl_bias=4 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, 0x05407680); /* cvar_ctnl=84, MIX_div_set=1, Rv3_par=6, LF_Cv3_Par=13 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER10_MPLL, 0x37880A00);    /* Cp_ctrl=0xE */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER11_MPLL, 0x3119C56E);    /*  LODIV_div_ratio_sel=16*/

   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER05_DCO, 0x00000100);     /* enable dco sigma-delta */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER24_ADC, 0xD5C00DD0);  /* updated ADC registers */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, 0x010020A5);  /* updated ADC registers */

   /* override defaults for mixpll LO to 2307.5MHz */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER12_MPLL, 0x0C04AAAB);    /* ndiv_int=192, ndiv_fract=305835 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER13_MPLL, 0x08032220);    /* configure lock detection */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER14_MPLL, 0xB4E83BA0);    /* Ibias_VCO_slope=16 */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER13_MPLL, 0x00000800);       /* enable continouos pll locking system */

   /* override default freq for adc pll */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER16_APLL, 0x0AC448C0);    /* pdiv=1, ndiv_int=72, refclk_sel=1 */
#if (BCHP_CHIP==lonestar)
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER21_APLL, 0x20040000);    /* ch3_mdiv=32 for dco clk, ch2_mdiv=4 for agc clk */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER22_APLL, 0x00000020);    /* ch5_Mdiv=32 for ramp clk */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER20_APLL, 0x000008E3);    /* shut cal tone divider channel */
#else
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER21_APLL, 0x20040020);    /* ch3_mdiv=32 for dco clk, ch2_mdiv=4 for agc clk */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER22_APLL, 0x00000010);    /* ch4_Mdiv=16 for 216MHz leap clk */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER20_APLL, 0x000008A2);    /* shut cal tone divider channel, clock ram generator */
#endif
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, 0x00200000);     /* bypass filter for faster LDO startup */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER09_MPLL, 0x08000000);    /* turn off filter for faster LDO startup*/
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER10_MPLL, 0x40000000);    /* turn off filter for faster LDO startup*/
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER09_MPLL, 0x0E008001);    /* turn off filter for faster LDO startup*/

   /* analog power up sequence */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, ~0x00000010);      /* disable rf test path */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, 0x00001000);        /* power up bandgap */

   /* wait 100us for bandgap current to settle */
   BKNI_Delay(100);

   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, 0x011FAFEF);        /* power up analog blocks */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, ~0x000C0000);  /* power up agc adc */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, 0x00000003);    /* power up mixer pll */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER16_APLL, 0x00000042);       /* power up ldo */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER24_ADC, ~0x00380000);      /* power up adc */

   /* assert resets */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER03_DCO, ~0x00000001);      /* reset dco clock */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER05_DCO, 0x00000400);        /* reset dco sigma-delta integrator */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER05_DCO, ~0x00000001);      /* reset global dco digital */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER06_AGC, ~0x00010000);      /* reset agc clock */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, 0x00000040);    /* reset mixer div */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, ~0x0800003C);  /* reset lna ramp, mixpll */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER19_APLL, ~0x00000C21);     /* reset adc pll */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER24_ADC, 0x0000000C);        /* reset adc i and q integrators */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, 0x00600000);        /* reset adc i and q data */

   /* wait 200us then release all non-mixpll resets */
   BKNI_Delay(200);

   /* release resets */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, ~0x00000040);  /* release mixer div reset */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, 0x08000000);    /* release lna ramp */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER03_DCO, 0x00000007);        /* set max clk div to remove 12MHz IF spur, release dco clock reset */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER05_DCO, ~0x00000400);      /* release dco sigma-delta integrator reset */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER05_DCO, 0x00000001);        /* release global dco digital reset */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER06_AGC, 0x00010000);        /* release agc clock reset */

   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER19_APLL, 0x00000023);       /* release adc pll reset and power up */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER24_ADC, ~0x0000000C);      /* release adc i and q integrator resets */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, ~0x00600000);      /* release adc i and q data resets */

   /* wait 600us then release mixpll resets */
   BKNI_Delay(600);
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, 0x0000003C);    /* release mixpll reset */

   /* wait 10us after resets */
   BKNI_Delay(10);
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER09_MPLL, ~0x08048001);  /* re-enable filters */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, ~0x02200000);   /* re-enable filters */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER10_MPLL, ~0x40000000);  /* re-enable filter */

   /* mixer pll will lock in ~600us after resets are released and digital calibration can start */
   for (i = 0; i < 20; i++)
   {
      /* wait for pll lock */
      BKNI_Delay(50);

      /* check pll status */
      BSRF_P_ReadRegister(h, BCHP_SRFE_ANA_READR03, &val);
      if ((val & 0x6) == 0x2)
      {
         bPllLocked = true;
         break;
      }
   }

   if (!bPllLocked)
   {
      /* pll not locked */
      BDBG_WRN(("mixpll NOT locked!", h->channel));
   }

   /* increase fga gain range */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER07_RAMP, ~0x60000000);

   /* power up antenna sense by default */
   BSRF_g1_Ana_P_PowerUpAntennaSense(h);

   return retCode;
}


/******************************************************************************
 BSRF_g1_Ana_P_PowerDown
******************************************************************************/
BERR_Code BSRF_g1_Ana_P_PowerDown(BSRF_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;

   /* power down antenna sense */
   BSRF_g1_Ana_P_PowerDownAntennaSense(h);

   /* analog power down */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, ~0x000C0003);  /* power down mixer pll */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER16_APLL, ~0x00000042);     /* power down ldo */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER24_ADC, 0x00380000);        /* power down adc */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER19_APLL, ~0x00000002);     /* power down adc pll */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, ~0x011FBFEF);      /* power down analog blocks */

   return retCode;
}


/******************************************************************************
 BSRF_g1_Ana_P_PowerUpAntennaSense
******************************************************************************/
BERR_Code BSRF_g1_Ana_P_PowerUpAntennaSense(BSRF_ChannelHandle h)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   hChn->bAntennaSenseEnabled = true;

   /* initialize antenna sense */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, 0x40013C98);
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR01, 0x900583F8);
#if (BCHP_CHIP==89730)
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR02, 0x00000001);
#endif

   /* antenna power up */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, ~0x10000000);
   return retCode;
}


/******************************************************************************
 BSRF_g1_Ana_P_PowerDownAsense
******************************************************************************/
BERR_Code BSRF_g1_Ana_P_PowerDownAntennaSense(BSRF_ChannelHandle h)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;

   hChn->bAntennaSenseEnabled = false;

   /* antenna power down */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, 0x10000000);
#if (BCHP_CHIP==89730)
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR02, 0x00000000);
#endif
   return retCode;
}


/******************************************************************************
 BSRF_g1_Ana_P_CalibrateCaps
******************************************************************************/
BERR_Code BSRF_g1_Ana_P_CalibrateCaps(BSRF_ChannelHandle h)
{
   BSRF_g1_P_ChannelHandle *hChn = (BSRF_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, Q_hi, Q_lo;
   uint8_t i, cap[5];

   /* frc_n(ndx) = [4.46401e6,  4.26822e6,  4.20207e6, 4.17966e6, 4.01148e6] */
#if (BCHP_CHIP==89730)
   /* count nominal = M * frc_n(ndx) / fclk, where M=2^16-1, fclk=192e6 */
   uint32_t countNominal[5] = {99857361, 95477008, 93997280, 93495985, 89733919};      /* scaled 16.16 */
#else
   /* count nominal = M * frc_n(ndx) / fclk, where M=2^16-1, fclk=96e6 */
   uint32_t countNominal[5] = {199713379, 190954017, 187994561, 186991970, 179467839}; /* scaled 16.16 */
#endif

   /* set timer to max count @ 96MHz */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_RC_CAL_TMR, 0x0000FFFF);
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, 0x00800000);  /* power up rc calibration */

   for (i = 0; i < 5; i++)
   {
      /* select cap for calibration */
      BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, ~0x1F000000, (1 << i) << 24);
      BSRF_P_AndRegister(h, BCHP_SRFE_ANA_RC_CAL_TMR, ~0x00010000);  /* clear osc count */
      BSRF_P_OrRegister(h, BCHP_SRFE_ANA_RC_CAL_TMR, 0x00010000);    /* enable timer */

      /* read rc oscillation count */
      BKNI_Sleep(2);
      BSRF_P_ReadRegister(h, BCHP_SRFE_ANA_RC_OSC_CNT, &val);  /* actual count */

      /* calculate capv(ndxc) = Clng(cnt_act(ndxc)/cnt_nom(ndxc) * (15+capv_nom(ndxc)) - 14) */
      BMTH_HILO_64TO64_Div32(val, 0, countNominal[i], &Q_hi, &Q_lo); /* count scaled up 32.32 for division */
      val = (Q_lo * 23) - 917504;   /* scaled 16.16 after divide, times 15+capv=23, minus 14 */
      val = (val + 32768) >> 16;    /* round */
      cap[i] = val & 0xF;           /* 4-bit cap values */

      //BKNI_Printf("cap%d=%02X\n", i, cap[i]);
   }

   /* program cap values */
   val = (cap[0] << 28) | (cap[1] << 12) | (cap[2] << 8) | (cap[3] << 4) | cap[4];
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER23_ADC, val);
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, ~0x000000F0, cap[1] << 4);  /* same as calC_int1 */

   /* power down rc calibration */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, ~0x00800000);

   /* delay state calibration */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, 0x80000000);  /* power up delay contrroller */
   BSRF_P_ReadRegister(h, BCHP_SRFE_ANA_DCTRL_STATE, &val);
   BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, ~0x0000000F, ((val & 0x3) << 2) | (val & 0x3));
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, ~0x80000000);   /* power down delay contrroller */

   return retCode;
}


/******************************************************************************
 BSRF_g1_Ana_P_BoostMixPllCurrent
******************************************************************************/
BERR_Code BSRF_g1_Ana_P_BoostMixPllCurrent(BSRF_ChannelHandle h, bool bBoost)
{
   if (bBoost)
      BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER14_MPLL, ~0x0000007F, 0x0000002F);
   else
      BSRF_P_ReadModifyWriteRegister(h, BCHP_SRFE_ANA_WRITER14_MPLL, ~0x0000007F, 0x00000020);

   return BERR_SUCCESS;
}
