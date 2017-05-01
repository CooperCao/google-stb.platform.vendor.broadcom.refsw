/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *
 * [File Description:]
 *
 ***************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"

BDBG_MODULE(bast_g3_priv_turbo);

#define BAST_DEBUG_TURBO(x) /* x */

#define BAST_TURBO_SYNC_TIMEOUT 3000000 /*1500000*/

#ifndef BAST_EXCLUDE_TURBO

/* local functions */
static BERR_Code BAST_g3_P_TurboOnHpLock_isr(BAST_ChannelHandle h);
static BERR_Code BAST_g3_P_TurboOnSyncTimeout_isr(BAST_ChannelHandle h);


/******************************************************************************
 BAST_g3_P_TurboScanTryNextMode_isr()
******************************************************************************/
static bool BAST_g3_P_TurboScanTryNextMode_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   bool bIsTurbo8psk;

   hChn->actualMode = BAST_Mode_eUnknown;

   next_mode:
   if (hChn->turboScanModes == 0)
      return false;

   if (hChn->turboScanCurrMode == 0)
   {
      hChn->turboScanCurrMode = 1;
      hChn->turboScanState &= BAST_TURBO_SCAN_STATE_FIRST_TIME;
      if (!hChn->bBlindScan && ((hChn->turboScanState & BAST_TURBO_SCAN_STATE_FIRST_TIME) == 0))
         hChn->acqCount++;
   }
   else
   {
      hChn->turboScanCurrMode = (hChn->turboScanCurrMode << 1) & BAST_G3_CONFIG_TURBO_SCAN_MODES_MASK;
      if (hChn->turboScanCurrMode == 0)
      {
         hChn->turboScanState &= ~BAST_TURBO_SCAN_STATE_FIRST_TIME;
         if (hChn->bBlindScan)
            return false;
      }
   }
   if (hChn->turboScanCurrMode & hChn->turboScanModes)
   {
      if (hChn->turboScanCurrMode & BAST_G3_CONFIG_TURBO_SCAN_MODES_QPSK_1_2)
         hChn->actualMode = BAST_Mode_eTurbo_Qpsk_1_2;
      else if (hChn->turboScanCurrMode & BAST_G3_CONFIG_TURBO_SCAN_MODES_QPSK_2_3)
         hChn->actualMode = BAST_Mode_eTurbo_Qpsk_2_3;
      else if (hChn->turboScanCurrMode & BAST_G3_CONFIG_TURBO_SCAN_MODES_QPSK_3_4)
         hChn->actualMode = BAST_Mode_eTurbo_Qpsk_3_4;
      else if (hChn->turboScanCurrMode & BAST_G3_CONFIG_TURBO_SCAN_MODES_QPSK_5_6)
         hChn->actualMode = BAST_Mode_eTurbo_Qpsk_5_6;
      else if (hChn->turboScanCurrMode & BAST_G3_CONFIG_TURBO_SCAN_MODES_QPSK_7_8)
         hChn->actualMode = BAST_Mode_eTurbo_Qpsk_7_8;
      else if (hChn->turboScanCurrMode & BAST_G3_CONFIG_TURBO_SCAN_MODES_8PSK_2_3)
         hChn->actualMode = BAST_Mode_eTurbo_8psk_2_3;
      else if (hChn->turboScanCurrMode & BAST_G3_CONFIG_TURBO_SCAN_MODES_8PSK_3_4)
         hChn->actualMode = BAST_Mode_eTurbo_8psk_3_4;
      else if (hChn->turboScanCurrMode & BAST_G3_CONFIG_TURBO_SCAN_MODES_8PSK_4_5)
         hChn->actualMode = BAST_Mode_eTurbo_8psk_4_5;
      else if (hChn->turboScanCurrMode & BAST_G3_CONFIG_TURBO_SCAN_MODES_8PSK_5_6)
         hChn->actualMode = BAST_Mode_eTurbo_8psk_5_6;
      else /* (hChn->turboScanCurrMode & BAST_G3_CONFIG_TURBO_SCAN_MODES_8PSK_8_9) */
         hChn->actualMode = BAST_Mode_eTurbo_8psk_8_9;

      if (hChn->turboScanState & BAST_TURBO_SCAN_STATE_HP_INIT)
      {
         bIsTurbo8psk = BAST_MODE_IS_TURBO_8PSK(hChn->actualMode) ? true : false;
         if (hChn->turboScanState & BAST_TURBO_SCAN_STATE_HP_LOCKED)
         {
            if (bIsTurbo8psk)
            {
               if ((hChn->turboScanState & BAST_TURBO_SCAN_STATE_8PSK_HP_LOCKED) == 0)
                  goto next_mode;
            }
            else if (hChn->turboScanState & BAST_TURBO_SCAN_STATE_8PSK_HP_LOCKED)
               goto next_mode;
         }
         else if (hChn->turboScanState & BAST_TURBO_SCAN_STATE_8PSK_FAILED)
         {
            /* only consider qpsk */
            if (bIsTurbo8psk)
               goto next_mode;
         }
         else
         {
            /* only consider 8psk */
            if (!bIsTurbo8psk)
               goto next_mode;
         }
      }

      switch (hChn->actualMode)
      {
         case BAST_Mode_eTurbo_Qpsk_1_2:
            BDBG_MSG(("trying BAST_Mode_eTurbo_Qpsk_1_2"));
            break;
         case BAST_Mode_eTurbo_Qpsk_2_3:
            BDBG_MSG(("trying BAST_Mode_eTurbo_Qpsk_2_3"));
            break;
         case BAST_Mode_eTurbo_Qpsk_3_4:
            BDBG_MSG(("trying BAST_Mode_eTurbo_Qpsk_3_4"));
            break;
         case BAST_Mode_eTurbo_Qpsk_5_6:
            BDBG_MSG(("trying BAST_Mode_eTurbo_Qpsk_5_6"));
            break;
         case BAST_Mode_eTurbo_Qpsk_7_8:
            BDBG_MSG(("trying BAST_Mode_eTurbo_Qpsk_7_8"));
            break;
         case BAST_Mode_eTurbo_8psk_2_3:
            BDBG_MSG(("trying BAST_Mode_eTurbo_8psk_2_3"));
            break;
         case BAST_Mode_eTurbo_8psk_3_4:
            BDBG_MSG(("trying BAST_Mode_eTurbo_8psk_3_4"));
            break;
         case BAST_Mode_eTurbo_8psk_4_5:
            BDBG_MSG(("trying BAST_Mode_eTurbo_8psk_4_5"));
            break;
         case BAST_Mode_eTurbo_8psk_5_6:
            BDBG_MSG(("trying BAST_Mode_eTurbo_8psk_5_6"));
            break;
         case BAST_Mode_eTurbo_8psk_8_9:
            BDBG_MSG(("trying BAST_Mode_eTurbo_8psk_8_9"));
            break;
         default:
            BDBG_ERR(("BAST_g3_P_TurboScanTryNextMode_isr(): should not get here!"));
            return false;
      }
   }
   else
      goto next_mode;

   return true;
}


/******************************************************************************
 BAST_g3_P_TurboConfigCl_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboConfigCl_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);
#if 0
   if (hChn->tunerCtl & BAST_G3_CONFIG_TUNER_CTL_PRESERVE_COMMANDED_FREQ)
   {
      val &= ~0x0000FF00;
      if (BAST_MODE_IS_TURBO_QPSK(hChn->actualMode))
         val |= 0x1E00;
      else
         val |= 0x1600;
   }
   else
   {
      if (BAST_MODE_IS_TURBO_QPSK(hChn->actualMode))
         val |= 0x1A00;
      else
         val |= 0x1200;
   }
#else
   val &= ~0x00000A00;
#endif
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, &val);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, &val);
#if 0
   val &= 0x10FFFFFF;
   val |= 0x00006007;
   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
   {
      /* clf_lin_ext = 0 */
      /* set clf_coeff1_ext, clf_pd_ext, clf_int_ext, clb_coeff1_ext, clb_pd_ext, clb_int_ext */
      val |= 0xEE000000;
   }
#else
   val &= ~0xFF000000;
   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val |= 0xEE000000;
#endif
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, &val);

   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLFFCTL, ~0x00000002);
   else
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLFFCTL, 0x00000002);

   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLFBCTL, ~0x000000F8);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboConfigEq_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboConfigEq_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, i;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, &val);
   /* new */
   val &= ~0x0038401F;
   val |= 0x00140000;

   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val |= 0x0000400A; /* err_mode=2, sym_mode=2, set ext_en */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, &val);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, &val);
   val &= 0x0000FF06;
   val |= 0x660C0720;
   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val |= 0x08;
   else
      val |= 0x10;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, &val);

   for (i = 0; i < 24; i++)
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD, &val);
      val &= ~0x1F;
      val |= (0x40 | i);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD, &val);

      val = 0;
      if (i == 0x0C) /* TBD: should get main tap location from EQFFECTL [20:16] instead */
      {
         if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
         {
#ifdef BAST_HAS_WFE
            val = 0x37000000;
#else
            val = 0x39000000; /* orig: 0x45180000 */
#endif
         }
         else
         {
#ifdef BAST_HAS_WFE
            val = 0x25000000;
#else
            val = 0x28600000; /* qpsk setting */
#endif
         }
      }
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_F0B, &val);
   }

   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
   {
#ifdef BAST_HAS_WFE
      val = 0x700A; /* 70% */
#else
      val = 0x670A; /* 0x67 = backoff 15% from 0x79 */
#endif
   }
   else
   {
#ifdef BAST_HAS_WFE
      val = 0x7A0A; /* 70% */
#else
      val = 0x750A;
#endif
   }
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_PLDCTL, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboSetOpll_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboSetOpll_isr(BAST_ChannelHandle h)
{
   static const uint32_t turbo_opll_N[10] =
   {
      38352000, /* turbo qpsk 1/2 */
      51286400, /* turbo qpsk 2/3 */
      57603200, /* turbo qpsk 3/4 */
      64070400, /* turbo qpsk 5/6 */
      67228800, /* turbo qpsk 7/8 */
      77004800, /* turbo 8psk 2/3 */
      82419200, /* turbo 8psk 3/4 */
      84675200, /* turbo 8psk 4/5 */
      92345600, /* turbo 8psk 5/6 */
      96256000  /* turbo 8psk 8/9 */
   };


   static const uint32_t turbo_opll_D[10] =
   {
      161699648, /* turbo qpsk 1/2 */
      161699648, /* turbo qpsk 2/3 */
      161699648, /* turbo qpsk 3/4 */
      161699648, /* turbo qpsk 5/6 */
      161699648, /* turbo qpsk 7/8 */
      160703040, /* turbo 8psk 2/3 */
      160703040, /* turbo 8psk 3/4 */
      160703040, /* turbo 8psk 4/5 */
      160703040, /* turbo 8psk 5/6 */
      160703040  /* turbo 8psk 8/9 */
   };

   static const uint16_t turbo_number_of_bits_in_block[10] =
   {
      2550, /* turbo qpsk 1/2 */
      3410, /* turbo qpsk 2/3 */
      3830, /* turbo qpsk 3/4 */
      4260, /* turbo qpsk 5/6 */
      4470, /* turbo qpsk 7/8 */
      5120, /* turbo 8psk 2/3 */
      5480, /* turbo 8psk 3/4 */
      5630, /* turbo 8psk 4/5 */
      6140, /* turbo 8psk 5/6 */
      6400  /* turbo 8psk 8/9 */
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, i, P_hi, P_lo, Q_hi, Q_lo;

   i = hChn->actualMode - BAST_Mode_eTurbo_Qpsk_1_2;
   hChn->opll_N = turbo_opll_N[i];
   hChn->opll_D = turbo_opll_D[i];

   /* val = HP header symbol */
   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val = 64;
   else
      val = 128;

   BMTH_HILO_32TO64_Mul(hChn->acqParams.symbolRate, turbo_number_of_bits_in_block[i] * 3760 * 4 * 2, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, (val + 10256) * 3893, &Q_hi, &Q_lo);
   hChn->outputBitRate = (Q_lo + 1) >> 1;

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboSetTitr_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboSetTitr_isr(BAST_ChannelHandle h)
{
   static const uint32_t turbo_titr[] =
   {
      0x007F2824,  /* turbo qpsk 1/2 */
      0x00D4C828,  /* turbo qpsk 2/3 */
      0x00BF4830,  /* turbo qpsk 3/4 */
      0x00D4C836,  /* turbo qpsk 5/6 */
      0x00DF483A,  /* turbo qpsk 7/8 */
      0x00FF8024,  /* turbo 8psk 2/3 */
      0x01118028,  /* turbo 8psk 3/4 */
      0x01190030,  /* turbo 8psk 4/5 */
      0x01328036,  /* turbo 8psk 5/6 */
      0x013F803A   /* turbo 8psk 8/9 */
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, i;

   i = hChn->actualMode - BAST_Mode_eTurbo_Qpsk_1_2;
   val = turbo_titr[i];
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_TFEC_TITR, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboSetTtur_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboSetTtur_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, iter, P_hi, P_lo, Q_hi, Q_lo;

   /* 0.5 + 16/256 = 0.5625 = 9/16 */
   /* iter = (uint32_t)((float)fec_freq/(0.5625 * (float)acq_symbol_rate) - 1.0); */
   BMTH_HILO_32TO64_Mul(16, hChn->fecFreq, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 9 * hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
   iter = Q_lo - 1;
   if (iter > 19)
      iter = 19;
   val = 0x00000A03 | (iter << 16);
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_TFEC_TTUR, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboSetTssq_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboSetTssq_isr(BAST_ChannelHandle h)
{
   static const uint8_t turbo_tssq_qpsk_1_2[2] =
   {
      0x01,
      0xDD
   };

   static const uint8_t turbo_tssq_qpsk_2_3[4] =
   {
      0x03,
      0xDD,
      0xDD,
      0xEE
   };

   static const uint8_t turbo_tssq_qpsk_3_4[5] =
   {
      0x04,
      0xED,
      0xED,
      0xDE,
      0xDE
   };

   static const uint8_t turbo_tssq_qpsk_5_6[7] =
   {
      0x06,
      0xED,
      0xDE,
      0xDE,
      0xEE,
      0xEE,
      0xED
   };

   static const uint8_t turbo_tssq_qpsk_7_8[9] =
   {
      0x08,
      0xED,
      0xED,
      0xDE,
      0xDE,
      0xEE,
      0xEE,
      0xEE,
      0xEE
   };

   static const uint8_t turbo_tssq_8psk_2_3[6] =
   {
      0x05,
      0x00,
      0x00,
      0x00,
      0x00,
      0x22
   };

   static const uint8_t turbo_tssq_8psk_3_4[8] =
   {
      0x07,
      0x00,
      0x00,
      0x20,
      0x10,
      0x00,
      0x01,
      0x02
   };

   static const uint8_t turbo_tssq_8psk_4_5[6] =
   {
      0x05,
      0x10,
      0x00,
      0x00,
      0x01,
      0x22
   };

   static const uint8_t turbo_tssq_8psk_5_6[6] =
   {
      0x05,
      0x10,
      0x01,
      0x10,
      0x01,
      0x22
   };

   static const uint8_t turbo_tssq_8psk_8_9[7] =
   {
      0x06,
      0x10,
      0x01,
      0x21,
      0x10,
      0x01,
      0x12
   };

   uint8_t* turbo_tssq_table[10] =
   {
      (uint8_t *)turbo_tssq_qpsk_1_2,
      (uint8_t *)turbo_tssq_qpsk_2_3,
      (uint8_t *)turbo_tssq_qpsk_3_4,
      (uint8_t *)turbo_tssq_qpsk_5_6,
      (uint8_t *)turbo_tssq_qpsk_7_8,
      (uint8_t *)turbo_tssq_8psk_2_3,
      (uint8_t *)turbo_tssq_8psk_3_4,
      (uint8_t *)turbo_tssq_8psk_4_5,
      (uint8_t *)turbo_tssq_8psk_5_6,
      (uint8_t *)turbo_tssq_8psk_8_9
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, i;
   const uint8_t *pTable;
   uint8_t len;

   i = hChn->actualMode - BAST_Mode_eTurbo_Qpsk_1_2;
   pTable = turbo_tssq_table[i];

   len = *pTable++;
   for (i = (uint32_t)len; i; i--)
   {
      val = (uint32_t)(*pTable++);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_TFEC_TSSQ, &val);
   }
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboConfigTfec_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboConfigTfec_isr(BAST_ChannelHandle h)
{
   static const uint32_t script_turbo_2[] =
   {
      BAST_SCRIPT_WRITE(BCHP_TFEC_GR_BRIDGE_SW_INIT_0, 1),
      BAST_SCRIPT_WRITE(BCHP_TFEC_GR_BRIDGE_SW_INIT_0, 0),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TFECTL, 0x80),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_turbo_3[] =
   {
      BAST_SCRIPT_WRITE(BCHP_TFEC_TCIL, 0x00009FCC),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TRSD, 0x00004FCC),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TZPK, 0x03B58F34),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TFMT, 0x00028008),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TSYN, 0x0103FEFE),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TPAK, 0x0009BB47),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_turbo_4[] =
   {
      BAST_SCRIPT_WRITE(BCHP_TFEC_TZSY, 0x0420040F),
      /* BAST_SCRIPT_WRITE(BCHP_TFEC_TFECTL, 0xC0),  */
      BAST_SCRIPT_WRITE(BCHP_TFEC_TFECTL, 0x00),
      BAST_SCRIPT_WRITE(BCHP_TFEC_TFECTL, 0x40),
      BAST_SCRIPT_EXIT
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_turbo_2));
   BAST_CHK_RETCODE(BAST_g3_P_TurboSetTitr_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_turbo_3));
   if (hChn->xportSettings.bTei)
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_TFEC_TPAK, 1<<17);
   BAST_CHK_RETCODE(BAST_g3_P_TurboSetTtur_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_turbo_4));
   BAST_CHK_RETCODE(BAST_g3_P_TurboSetTssq_isr(h));

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_TurboRun_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboRun_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   static const uint32_t script_turbo_6[] =
   {
      BAST_SCRIPT_WRITE(BCHP_TFEC_TFECTL, 0x63), /* reset error counters */
      BAST_SCRIPT_WRITE(BCHP_TFEC_TFECTL, 0x61), /* clear error counter reset */
      BAST_SCRIPT_EXIT
   };

   /* clear turbo interrupts */
   BINT_ClearCallback_isr(hChn->hTurboSyncCb);
   BINT_ClearCallback_isr(hChn->hTurboLockCb);
   BINT_ClearCallback_isr(hChn->hTurboNotLockCb);

   retCode = BAST_g3_P_ProcessScript_isrsafe(h, script_turbo_6);
   if (retCode == BERR_SUCCESS)
      retCode = BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, BAST_TURBO_SYNC_TIMEOUT, BAST_g3_P_TurboOnSyncTimeout_isr);
   return retCode;
}


/******************************************************************************
 BAST_g3_P_TurboEnableSyncInterrupt_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboEnableSyncInterrupt_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_ClearCallback_isr(hChn->hTurboSyncCb);
   BINT_EnableCallback_isr(hChn->hTurboSyncCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboDisableSyncInterrupt_isr()
******************************************************************************/
BERR_Code BAST_g3_P_TurboDisableSyncInterrupt_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_DisableCallback_isr(hChn->hTurboSyncCb);
   BINT_ClearCallback_isr(hChn->hTurboSyncCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboSync_isr() - callback routine for turbo sync detected
******************************************************************************/
void BAST_g3_P_TurboSync_isr(void *p, int int_id)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BAST_g3_P_DisableTimer_isr(h, BAST_TimerSelect_eBaud); /* disable sync timeout */
   BAST_g3_P_TurboDisableSyncInterrupt_isr(h);

   BAST_g3_P_IncrementInterruptCounter_isr(h, int_id);

   if (hChn->count1 > 0)
   {
      BAST_DEBUG_TURBO(BDBG_WRN(("BAST_g3_P_TurboSync_isr(): resync count=%d", hChn->count1)));
   }

   BAST_g3_P_StartTracking_isr(h);
}

#endif


/******************************************************************************
 BAST_g3_P_TurboAcquire_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboAcquire_isr(BAST_ChannelHandle h)
{
#ifndef BAST_EXCLUDE_TURBO
   static const uint32_t script_turbo_1[] =
   {
#if 1
      /* orig */
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_HD8PSK1, 0x01D901D9),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_HD8PSK2, 0x00C400C4),
#else
      /* 65 nm settings */
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_HD8PSK1, 0x014e01d9),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_HD8PSK2, 0x008b00c4),
#endif
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_HDQPSK, 0x01000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_XTAP1, 0x00000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_XTAP2, 0x00805000),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_PILOTCTL, 0x00000004),
      BAST_SCRIPT_AND(BCHP_SDS_EQ_EQMISCCTL, ~0x60), /* for now, disable soft pd tables */
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_VCOS, 0x76303000),
      BAST_SCRIPT_EXIT
   };

#if 0
   static const uint32_t script_turbo_2[] = /* this script changes the TFEC clock */
   {
      BAST_SCRIPT_WRITE(BCHP_TFEC_GR_BRIDGE_SW_INIT_0, 1),
      BAST_SCRIPT_WRITE(BCHP_TFEC_GR_BRIDGE_SW_INIT_0, 0),
#if 1
      /* 246 MHz */
      BAST_SCRIPT_WRITE(BCHP_TFEC_MISC_POST_DIV_CTL, 0x0904), /* clock disable, mdiv=9 (246MHz), default is 0xa (221.4MHz) */
      BAST_SCRIPT_WRITE(BCHP_TFEC_MISC_POST_DIV_CTL, 0x0905), /* mdiv load enable */
      BAST_SCRIPT_WRITE(BCHP_TFEC_MISC_POST_DIV_CTL, 0x0904),
      BAST_SCRIPT_WRITE(BCHP_TFEC_MISC_POST_DIV_CTL, 0x0900), /* clock enable */
#else
      /* 201.27 MHz */
      BAST_SCRIPT_WRITE(BCHP_TFEC_MISC_POST_DIV_CTL, 0x0b04), /* clock disable, mdiv=0xb (201.2MHz), default is 0xa (221.4MHz) */
      BAST_SCRIPT_WRITE(BCHP_TFEC_MISC_POST_DIV_CTL, 0x0b05), /* mdiv load enable */
      BAST_SCRIPT_WRITE(BCHP_TFEC_MISC_POST_DIV_CTL, 0x0b04),
      BAST_SCRIPT_WRITE(BCHP_TFEC_MISC_POST_DIV_CTL, 0x0b00), /* clock enable */
#endif
      BAST_SCRIPT_EXIT
   };
#endif

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;

   if (hChn->bHasTfec == false)
   {
      turbo_acquire_failed:
      if (hChn->bBlindScan)
      {
         hChn->reacqCause = BAST_ReacqCause_eNoTfec;
         return BAST_g3_P_Reacquire_isr(h);
      }
      else
      {
         /* abort acquisition */
         BAST_g3_P_DisableChannelInterrupts_isr(h, false, false);
         BAST_g3_P_SdsDisableOif_isrsafe(h);
         BAST_g3_P_IndicateNotLocked_isrsafe(h);
         hChn->acqTime = 0;
         hChn->acqState = BAST_AcqState_eIdle;
         hChn->reacqCtl &= (BAST_G3_CONFIG_REACQ_CTL_FREQ_DRIFT | BAST_G3_CONFIG_REACQ_CTL_RETUNE);
         return BERR_SUCCESS;
      }
   }

   if (hChn->acqParams.mode == BAST_Mode_eTurbo_scan)
   {
      if ((hDev->sdsRevId >= 0x68) || ((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_SPINV_IQ_SCAN) != BAST_ACQSETTINGS_SPINV_IQ_SCAN) ||
          (((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_SPINV_IQ_SCAN) == BAST_ACQSETTINGS_SPINV_IQ_SCAN) &&
           (hChn->bTurboSpinv == false)))
      {
         if (BAST_g3_P_TurboScanTryNextMode_isr(h) == false)
            goto turbo_acquire_failed;
      }
   }
   else
      hChn->turboScanState = 0;

#ifndef BAST_EXCLUDE_LDPC
   hChn->ldpcScanState = 0;
#endif

#if 1 /* dynamically turn on/off TFEC clock as needed */
   BAST_CHK_RETCODE(BAST_g3_P_TurboPowerUp_isr(h));
#endif
#if 0 /* enable this to override default tfec clock */
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_turbo_2));
#endif
   BAST_CHK_RETCODE(BAST_g3_P_GetTfecClock_isrsafe(h, &(hChn->fecFreq)));
   BAST_CHK_RETCODE(BAST_g3_P_NonLegacyModeAcquireInit_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_TurboConfigCl_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_TurboConfigEq_isr(h));
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 1); /* reset the eq */

#if 0
   /* 65 nm setting */
   val = 0x00040904;
#else
   val = 0x00000904;
#endif
   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val |= 0x00010000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCTL, &val);

   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_turbo_1));
   BAST_CHK_RETCODE(BAST_g3_P_ConfigPlc_isr(h, true)); /* set acquisition plc */

   /* Chan: before HP lock */
   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val = 0x00005088; /*0x00005066*/ /*5088, 396/211 */
   else
      val = 0x0000406A; /*406a;15, 2048;11*/ /* 6048:18, 606a:14 , 806A:21, 206a;10, 208c; 12*/
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, &val);

   /* configure and run the HP */
   retCode = BAST_g3_P_HpAcquire_isr(h, BAST_g3_P_TurboOnHpLock_isr);

   done:
   return retCode;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


#ifndef BAST_EXCLUDE_TURBO
/******************************************************************************
 BAST_g3_P_TurboOnSyncTimeout_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboOnSyncTimeout_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BAST_CHK_RETCODE(BAST_g3_P_TurboDisableSyncInterrupt_isr(h));

   hChn->count1++;
   if (BAST_g3_P_IsHpLocked_isrsafe(h))
   {
      if (hChn->count1 < 1)
      {
         /* reprogram the TFEC */
         BAST_CHK_RETCODE(BAST_g3_P_TurboConfigTfec_isr(h));
         BAST_CHK_RETCODE(BAST_g3_P_TurboEnableSyncInterrupt_isr(h));
         retCode = BAST_g3_P_TurboRun_isr(h);
      }
      else
      {
         hChn->count2++;
         if ((hChn->acqParams.mode == BAST_Mode_eTurbo_scan) && (hChn->count2 > 1))
         {
            hChn->reacqCause = BAST_ReacqCause_eTurboSyncTimeout;
            goto reacquire;
         }
         BAST_DEBUG_TURBO(BDBG_WRN(("reacquiring HP since TFEC can't lock")));
         BAST_CHK_RETCODE(BAST_g3_P_HpEnable_isr(h, false));
         retCode = BAST_g3_P_TurboAcquire_isr(h); /* reacquire HP */
      }
   }
   else
   {
      hChn->reacqCause = BAST_ReacqCause_eHpLostLock2;

      reacquire:
      if ((hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_SPINV_IQ_SCAN) == BAST_ACQSETTINGS_SPINV_IQ_SCAN)
         hChn->bTurboSpinv = false;
      retCode = BAST_g3_P_Reacquire_isr(h);
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_TurboOnHpLock_isr() - called when HP locks
******************************************************************************/
static BERR_Code BAST_g3_P_TurboOnHpLock_isr(BAST_ChannelHandle h)
{
   static const uint32_t script_turbo_5[] =
   {
      BAST_SCRIPT_AND(BCHP_SDS_EQ_EQMISCCTL, 0xFFFFFBFF),           /* disable CMA */
      BAST_SCRIPT_AND_OR(BCHP_SDS_EQ_EQFFECTL, 0xFFFF00FF, 0x0200), /* unfreze other taps */
#if 0
      BAST_SCRIPT_OR(BCHP_SDS_HP_HPOVERRIDE, 0x0000000F),           /* override front carrier loop */
#endif
      BAST_SCRIPT_AND(BCHP_SDS_CL_CLCTL1, 0xFFFFFFEF),              /* disable front carrier loop */
      BAST_SCRIPT_OR(BCHP_SDS_CL_CLCTL2, 0x00000004),               /* freeze front carrier loop */
      BAST_SCRIPT_EXIT
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   uint32_t val;
   BERR_Code retCode;

   if (hChn->acqParams.mode == BAST_Mode_eTurbo_scan)
   {
      hChn->turboScanState |= BAST_TURBO_SCAN_STATE_HP_LOCKED;
      if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
         hChn->turboScanState |= BAST_TURBO_SCAN_STATE_8PSK_HP_LOCKED;
   }

   /* Chan: after HP locks */
   if (hChn->actualMode <= BAST_Mode_eTurbo_Qpsk_2_3)
      val = 0x0000406A; /*406A; 17, 206A;15, 606A;12,20, 4048 no good, 408C:BAD */
   else if (hChn->actualMode <= BAST_Mode_eTurbo_Qpsk_7_8)
      val = 0x0000406A;
   else if (hChn->actualMode <= BAST_Mode_eTurbo_8psk_3_4)
      val = 0x0000408A; /*, was 0x406A, need to make DAFE converge fast *//* 4066, 50AA, 5066, 8PSK never lock, 406A is OK,  */
   else
      val = 0x0000408A;  /* was 506A */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, &val);

   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_turbo_5));
   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val = 0x91;
   else
      val = 0x81;

   if (hDev->sdsRevId >= 0x64)
      val |= 0x08;

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, &val);
   BAST_g3_P_InitBert_isr(h);
   BAST_g3_P_SdsPowerUpOpll_isr(h);
   BAST_CHK_RETCODE(BAST_g3_P_TurboSetOpll_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_ConfigOif_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_SetAgcTrackingBw_isr(h));

   /* set up the TFEC */
   BAST_CHK_RETCODE(BAST_g3_P_TurboConfigTfec_isr(h));

   if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val = 0x00004066; /*5066; *//* 3066; bad for 8PSK, never lock*/ /* 406A */
   else
      val = 0x00003063; /* 3066; *//*3038;16, 5034;17, 5056 */ /* 406A fell out of lock QPSK 1/2, 6A fell out of lock */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, &val);

   /* set tracking PLC lock */
   if (hChn->bPlcTracking == false)
   {
      BAST_g3_P_ConfigPlc_isr(h, false); /* set tracking PLC */
   }

   /* wait for sync with timeout */
   hChn->count1 = 0; /* count1=number of times we failed to get sync */
   BAST_CHK_RETCODE(BAST_g3_P_TurboEnableSyncInterrupt_isr(h));

   /* need a delay here for some reason... */
   /* retCode = BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 20000, BAST_g3_P_TurboRun_isr);    */
   retCode = BAST_g3_P_TurboRun_isr(h);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_TurboUpdateErrorCounters_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_TurboUpdateErrorCounters_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   if (BAST_g3_P_IsTurboOn_isrsafe(h))
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TFEC_TNBLK, &val);
      hChn->turboTotalBlocks += val;

      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TFEC_TCBLK, &val);
      hChn->turboCorrBlocks += val;

      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TFEC_TBBLK, &val);
      hChn->turboBadBlocks += val;

      /* reset the FEC error counters */
      BAST_g3_P_ToggleBit_isrsafe(h, BCHP_TFEC_TFECTL, 0x06);
   }
   return BAST_g3_P_UpdateErrorCounters_isrsafe(h);
}


/******************************************************************************
 BAST_g3_P_TurboCheckMode_isr() - ISR context
******************************************************************************/
static BERR_Code BAST_g3_P_TurboCheckMode_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i;
   uint16_t mask;

   if (BAST_MODE_IS_TURBO(hChn->actualMode))
   {
      if (hChn->actualMode == BAST_Mode_eTurbo_scan)
         goto invalid_condition;

      i = hChn->actualMode - BAST_Mode_eTurbo_Qpsk_1_2;
      mask = (uint16_t)(1 << i);
      if ((mask & hChn->turboScanModes) == 0)
      {
         hChn->reacqCause = BAST_ReacqCause_eInvalidMode;
         hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE;
      }
   }
   else
   {
      invalid_condition:
      BDBG_ERR(("BAST_g3_P_TurboCheckMode_isr() - invalid condition"));
      BERR_TRACE(retCode = BAST_ERR_AP_UNKNOWN);
   }

   return retCode;
}


/******************************************************************************
 BAST_g3_P_TurboOnLock_isr() - ISR context
******************************************************************************/
static BERR_Code BAST_g3_P_TurboOnLock_isr(BAST_ChannelHandle h)
{
   BSTD_UNUSED(h);

   BAST_DEBUG_TURBO(BDBG_MSG(("TFEC locked")));
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboOnLostLock_isr() - ISR context
******************************************************************************/
static BERR_Code BAST_g3_P_TurboOnLostLock_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BAST_CHK_RETCODE(BAST_g3_P_TurboUpdateErrorCounters_isrsafe(h));

   if (BAST_g3_P_IsHpLocked_isrsafe(h) == false)
   {
      hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE;
      hChn->reacqCause = BAST_ReacqCause_eHpLostLock4;
      BAST_DEBUG_TURBO(BDBG_MSG(("HP lost lock")));
   }
   else
   {
      BAST_DEBUG_TURBO(BDBG_MSG(("TFEC lost lock")));
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_TurboOnStableLock_isr() - ISR context
******************************************************************************/
static BERR_Code BAST_g3_P_TurboOnStableLock_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   BERR_Code retCode;
   uint32_t val;

#if 0
   /* narrow DAFE loop bw */
BDBG_ERR(("set dafectl"));
  if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      val = 0x00004066; /* 3066; bad for 8PSK, never lock*/
   else
      val = 0x00003063; /* 3066; *//*3038;16, 5034;17, 5056 */ /* 406A fell out of lock QPSK 1/2, 6A fell out of lock */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, &val);
#endif

   if (hDev->sdsRevId < 0x64)
   {
      if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
         val = 0x98;
      else
         val = 0x8C;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, &val);
   }

   /* set tracking baud loop bw */
   BAST_CHK_RETCODE(BAST_g3_P_SetBaudBw_isr(h, hChn->acqParams.symbolRate / 400, 4));

   if (hChn->bEverStableLock == false)
   {
      BAST_CHK_RETCODE(BAST_g3_P_TurboUpdateErrorCounters_isrsafe(h));
      hChn->turboTotalBlocks = 0;
      hChn->turboCorrBlocks = 0;
      hChn->turboBadBlocks = 0;
   }

   done:
   return retCode;
}


#ifndef BAST_HAS_LEAP
/******************************************************************************
 BAST_g3_P_GetTfecClock_isrsafe() - updates fecFreq
******************************************************************************/
BERR_Code BAST_g3_P_GetTfecClock_isrsafe(BAST_ChannelHandle h, uint32_t *pFreq)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;
   BERR_Code retCode = BERR_SUCCESS;

   if (BAST_g3_P_IsTurboOn_isrsafe(h))
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TFEC_MISC_POST_DIV_CTL, &val);
      *pFreq = hChn->vcoRefClock / ((val >> 8) & 0x000000FF);
   }
   else
   {
      *pFreq = 0;
      BDBG_ERR(("BAST_g3_P_GetTfecClock_isrsafe(): TFEC not powered"));
      BERR_TRACE(retCode = BAST_ERR_POWERED_DOWN);
   }
   return retCode;
}


/******************************************************************************
 BAST_g3_P_IsTurboOn_isrsafe()
******************************************************************************/
bool BAST_g3_P_IsTurboOn_isrsafe(BAST_ChannelHandle h)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_TFEC_MISC_POST_DIV_CTL, &val);
   if (val & 0x04)
      return false;
   return true;
}


/******************************************************************************
 BAST_g3_P_TurboPowerUp_isr() - power up the tfec core
******************************************************************************/
BERR_Code BAST_g3_P_TurboPowerUp_isr(BAST_ChannelHandle h)
{
   BERR_Code retCode;

   BAST_g3_P_AndRegister_isrsafe(h, BCHP_TFEC_MISC_POST_DIV_CTL, ~0x4);
   if (BAST_g3_P_IsTurboOn_isrsafe(h))
      retCode = BERR_SUCCESS;
   else
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_CMD_FAIL);
      BDBG_ERR(("BAST_g3_P_TurboPowerUp_isr() failed"));
   }
   return retCode;
}


/******************************************************************************
 BAST_g3_P_TurboPowerDown() - power down the tfec core
******************************************************************************/
BERR_Code BAST_g3_P_TurboPowerDown(BAST_ChannelHandle h)
{
   BERR_Code retCode;

   BAST_g3_P_OrRegister_isrsafe(h, BCHP_TFEC_MISC_POST_DIV_CTL, 0x4);
   if (BAST_g3_P_IsTurboOn_isrsafe(h) == false)
      retCode = BERR_SUCCESS;
   else
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_CMD_FAIL);
      BDBG_ERR(("BAST_g3_P_TurboPowerDown() failed"));
   }
   return retCode;
}
#endif


/******************************************************************************
 BAST_g3_P_TurboEnableLockInterrupts_isr()
******************************************************************************/
static BERR_Code BAST_g3_P_TurboEnableLockInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_ClearCallback_isr(hChn->hTurboLockCb);
   BINT_ClearCallback_isr(hChn->hTurboNotLockCb);

   BINT_EnableCallback_isr(hChn->hTurboLockCb);
   BINT_EnableCallback_isr(hChn->hTurboNotLockCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboDisableLockInterrupts_isr()
******************************************************************************/
BERR_Code BAST_g3_P_TurboDisableLockInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_DisableCallback_isr(hChn->hTurboLockCb);
   BINT_DisableCallback_isr(hChn->hTurboNotLockCb);

   BINT_ClearCallback_isr(hChn->hTurboLockCb);
   BINT_ClearCallback_isr(hChn->hTurboNotLockCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboMonitorLock_isr()
******************************************************************************/
/* #define BAST_DELAY_DAFE_TRK_BW */
static BERR_Code BAST_g3_P_TurboMonitorLock_isr(BAST_ChannelHandle h)
{
#if 0
#ifdef BAST_DELAY_DAFE_TRK_BW
   static uint32_t t0_Fb_table[8] = {5500000, 5000000, 4500000, 4000000, 3000000, 2500000, 2100000, 0};
   static uint32_t t0_table[8] =    {0,       1,       2,       4,       5,       8      , 10,      10};
   static uint32_t t1_Fb_table[6] = {5000000, 4500000, 4200000, 2500000, 2100000, 0};
   static uint32_t t1_table[6] =    {0,       1,       2,       4,       10,      10};
#endif

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, t0, t1;
#ifdef BAST_DELAY_DAFE_TRK_BW
   uint32_t i;
#endif

   if (BAST_g3_P_IsHpLocked_isrsafe(h) == false)
   {
      BDBG_MSG(("HP fell out of lock"));
      hChn->reacqCause = BAST_ReacqCause_eHpLostLock4;
      hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE;
   }
   else
   {
      /* determine the time for setting cldafectl */
#ifdef BAST_DELAY_DAFE_TRK_BW
      if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      {
         /* turbo 8psk */
#if 0
         t0 = hChn->debug1;
         t1 = hChn->debug2;
#else
         for (i = 0; i < 7; i++)
         {
            if (hChn->acqParams.symbolRate > t0_Fb_table[i])
               break;
         }
         if ((i <= 1) || (i >= 7))
            t0 = t0_table[i];
         else
         {
            /* interpolate */
            t0 = ((t0_table[i] - t0_table[i-1]) * (hChn->acqParams.symbolRate - t0_Fb_table[i]) * 2) / (t0_Fb_table[i-1] - t0_Fb_table[i]);
            t0 = (t0 + 1) >> 1;
            t0 = t0_table[i] - t0;
         }

         /* determine the time after t0 to set tracking plc */
         for (i = 0; i < 5; i++)
         {
            if (hChn->acqParams.symbolRate > t1_Fb_table[i])
               break;
         }
         if ((i <= 1) || (i >= 5))
            t1 = t1_table[i];
         else
         {
            /* interpolate */
            t1 = ((t1_table[i] - t1_table[i-1]) * (hChn->acqParams.symbolRate - t1_Fb_table[i]) * 2) / (t1_Fb_table[i-1] - t1_Fb_table[i]);
            t1 = (t1 + 1) >> 1;
            t1 = t1_table[i] - t1;
         }
#endif
      }
      else
#endif
         t0 = t1 = 0;

      if (hChn->timeSinceStableLock == t0) /* 5Mbaud=2, */
      {
/* BDBG_MSG(("setting cldafectl, t0=%d, t1=%d", t0, t1));  */
         if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
            val = 0x00004066; /*5066; *//* 3066; bad for 8PSK, never lock*/ /* 406A */
         else
            val = 0x00003063; /* 3066; *//*3038;16, 5034;17, 5056 */ /* 406A fell out of lock QPSK 1/2, 6A fell out of lock */
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, &val);
      }

      /* set tracking PLC lock */
      if (hChn->bPlcTracking == false)
      {
         if (hChn->timeSinceStableLock > (t0 + t1))
         {
/* BDBG_MSG(("set tracking plc, t=%d", t0+t1)); */
            BAST_g3_P_ConfigPlc_isr(h, false); /* set tracking PLC */
         }
      }
   }
#else
   BSTD_UNUSED(h);
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_TurboSetFunctTable_isr()
******************************************************************************/
BERR_Code BAST_g3_P_TurboSetFunctTable_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   hChn->acqFunct = BAST_g3_P_TurboAcquire_isr;
   hChn->checkModeFunct = BAST_g3_P_TurboCheckMode_isr;
   hChn->onLockFunct = BAST_g3_P_TurboOnLock_isr;
   hChn->onLostLockFunct = BAST_g3_P_TurboOnLostLock_isr;
   hChn->onStableLockFunct = BAST_g3_P_TurboOnStableLock_isr;
   hChn->onMonitorLockFunct = BAST_g3_P_TurboMonitorLock_isr;
   hChn->enableLockInterrupts = BAST_g3_P_TurboEnableLockInterrupts_isr;
   hChn->disableLockInterrupts = BAST_g3_P_TurboDisableLockInterrupts_isr;
   hChn->getLockStatusFunct = NULL;

   return BERR_SUCCESS;
}

#endif
