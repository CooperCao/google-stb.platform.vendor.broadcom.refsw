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

#include "bstd.h" 
#include "bwfe.h"
#include "bwfe_priv.h"
#include "bwfe_g3_priv.h"

BDBG_MODULE(bwfe_g3_priv_rffe);


#define BSRF_PGA_LUT_COUNT 256
static const uint32_t pgaLut[BSRF_PGA_LUT_COUNT] =
{
     66640,      66640,      66640,      66640,      66640,      66640,      66640,      66640,
     66640,      66640,      66640,      66640,      66640,      66640,      66640,      66640,
     66640,      66640,      66640,      66640,      66640,      66640,      66640,      66640,
     66640,      66640,      66640,      66640,      66640,      66640,      66640,      66640,
     66640,      66640,      66640,      66640,      66640,      66640,      66640,      66640,
     66640,      66640,      66640,      66640,      66640,      66640,      66640,      66640,
     66640,      66640,      66640,      66640,      66640,      66640,      66640,      66640,
     66640,      66640,      66640,      66640,      66640,      66640,      66640,      66640,
     66640,      66640,      66640,      66640,      66640,      66640,      66640,      66640,
     66640,      66640,      66640,      66640,      66640,      66640,      66640,      66640,
     66640,      66640,      66640,      66640,      66640,      66640,      64592,      62544,
     60496,      58448,      56400,      54352,      52304,      50256,      48208,      46160,
     44112,      42064,      40016,      37968,      35920,      33872,      31824,      29776,
     27728,      25680,      23632,      21584,      19536,      17488,      15440,      13392,
     11344,       9296,       7248,       5200,       3152,       1104,       1096,       1088,
      1080,       1072,       1064,       1056,       1048,       1040,       1032,       1024,
      1016,       1008,       1000,        992,        985,        976,        969,        960,
       953,        944,        937,        928,        927,        913,        904,        902,
       888,        886,        872,        864,        856,        848,        840,        832,
       824,        816,        808,        802,        792,        786,        776,        772,
       762,        756,        746,        742,        734,        726,        716,        707,
       698,        688,        680,        672,        664,        656,        648,        640,
       632,        624,        616,        608,        602,        592,        586,        576,
       570,        560,        554,        544,        536,        528,        520,        512,
       504,        496,        488,        480,        472,        464,        456,        448,
       440,        432,        424,        418,        408,        402,        392,        386,
       376,        372,        362,        353,        345,        336,        328,        320,
       312,        304,        296,        288,        280,        272,        264,        256,
       248,        240,        234,        224,        218,        208,        202,        192,
       186,        178,        170,        161,        153,        144,        136,        128,
       120,        112,        104,         96,         88,         80,         72,         64,
        57,         50,         41,         34,         24,         18,          8,          2
};


/******************************************************************************
 BWFE_Rffe_P_PowerUp()
******************************************************************************/
BERR_Code BWFE_g3_Rffe_P_PowerUp(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
#if (BCHP_CHIP==45308)
   uint32_t i;
#endif

   if (h->bReference)
      return BERR_NOT_SUPPORTED;

   BDBG_MSG(("RFFE%d pwrup", h->channel));
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_SYS_CNTL, 0x80000000);    /* power up RFFE */
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_SYS_CNTL, ~0x40000000);  /* reset RFFE agc */
   BWFE_P_OrRegister(h, BCHP_WFE_ANA_SYS_CNTL, 0x40000000);    /* release RFFE agc reset */

#if (BCHP_CHIP==45308)
   if (BWFE_P_IsAdcOn(h))
   {
      /* enable pga lut micro access */
      BWFE_P_OrRegister(h, BCHP_WFE_CORE_PGALUTA, 0x00000100);

      /* load pga lut */
      for (i = 0; i < BSRF_PGA_LUT_COUNT; i++)
      {
         BWFE_P_ReadModifyWriteRegister(h, BCHP_WFE_CORE_PGALUTA, ~0x000000FF, i & 0xFF);
         BWFE_P_WriteRegister(h, BCHP_WFE_CORE_PGALUTD, pgaLut[i] & 0x1FFFF);
      }

      /* disable pga lut micro access */
      BWFE_P_AndRegister(h, BCHP_WFE_CORE_PGALUTA, ~0x000001FF);
   }
#endif

   return retCode;
}


/******************************************************************************
 BWFE_Rffe_P_PowerDown()
******************************************************************************/
BERR_Code BWFE_g3_Rffe_P_PowerDown(BWFE_ChannelHandle h)
{
   BERR_Code retCode = BERR_SUCCESS;
   
   if (h->bReference)
      return BERR_NOT_SUPPORTED;
   
   BDBG_MSG(("RFFE%d pwrdown", h->channel));
   BWFE_P_AndRegister(h, BCHP_WFE_ANA_SYS_CNTL, ~0x80000000);  /* power down RFFE */
   
   return retCode;
}
