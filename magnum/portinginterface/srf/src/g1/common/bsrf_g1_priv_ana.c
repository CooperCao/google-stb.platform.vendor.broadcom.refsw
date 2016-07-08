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
#if 1
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER05_DCO, 0x00000100);     /* enable dco sigma-delta */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER24_ADC, 0xD5C00DD0);  /* updated ADC registers */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, 0x010020A5);  /* updated ADC registers */
#endif

   /* override defaults for mixpll */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER12_MPLL, 0x0C07FFFF);    /* ndiv_int=192, ndiv_fract=524287 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER13_MPLL, 0x08032218);    /* configure lock detection */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER14_MPLL, 0xB4E83BA0);    /* Ibias_VCO_slope=16 */
#if 1
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER13_MPLL, 0x00000800);    /* enable continouos pll locking system */
#endif

   /* override default freq for adc pll */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER16_APLL, 0x0AC448C0);    /* pdiv=1, ndiv_int=72, refclk_sel=1 */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER21_APLL, 0x20040000);    /* ch3_mdiv=32 for dco clk, ch2_mdiv=4 for agc clk */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER22_APLL, 0x00000020);    /* ch5_Mdiv=32 for ramp clk */
#if 1
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_WRITER20_APLL, 0x000008E3); /* shut cal tone divider channel */

   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, 0x00200000);     /* faster LDO startup */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER09_MPLL, 0x08000000);    /*faster LDO startup*/
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER10_MPLL, 0x40000000);    /*faster LDO startup*/
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER09_MPLL, 0x0E008001);    /*faster LDO startup*/
#endif

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

   /* wait 200us then release all resets */
   BKNI_Delay(200);

   /* release resets */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER03_DCO, 0x00000001);        /* release dco clock reset */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER05_DCO, ~0x00000400);      /* release dco sigma-delta integrator reset */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER05_DCO, 0x00000001);        /* release global dco digital reset */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER06_AGC, 0x00010000);        /* release agc clock reset */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, ~0x00000040);  /* release mixer div reset */
#if 1
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, 0x00000024);    /*release only the needed MIXERPLL resets*/
#endif
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, 0x0800003C);    /* release lna ramp, mixpll reset */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER19_APLL, 0x00000023);       /* release adc pll reset and power up */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER24_ADC, ~0x0000000C);      /* release adc i and q integrator resets */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER25_ADC, ~0x00600000);      /* release adc i and q data resets */

   /* reset mixpll again */
#if 1
   BKNI_Delay(100);
#endif
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, ~0x0800003C);  /* reset mixpll */
#if 1
   BKNI_Delay(100);
#else
   BKNI_Sleep(1);
#endif
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, 0x0800003C);    /* release mixpll reset */

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

   /* initialize antenna sense */
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR00, 0x00013C08);
   BSRF_P_WriteRegister(h, BCHP_SRFE_ANA_ANT_CTRLR01, 0x900583F8);

   return retCode;
}


/******************************************************************************
 BSRF_g1_Ana_P_PowerDown
******************************************************************************/
BERR_Code BSRF_g1_Ana_P_PowerDown(BSRF_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;

   /* analog power down */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER15_MPLLAGC, ~0x000C0003);  /* power down mixer pll */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER16_APLL, ~0x00000042);     /* power down ldo */
   BSRF_P_OrRegister(h, BCHP_SRFE_ANA_WRITER24_ADC, 0x00380000);        /* power down adc */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER19_APLL, ~0x00000002);     /* power down adc pll */
   BSRF_P_AndRegister(h, BCHP_SRFE_ANA_WRITER00_SYS, ~0x011FBFEF);      /* power down analog blocks */

   return retCode;
}