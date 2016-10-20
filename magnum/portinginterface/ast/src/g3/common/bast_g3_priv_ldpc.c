/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
 *****************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g3_priv.h"

#ifndef BAST_EXCLUDE_LDPC

#define BAST_g3_PD_DATA_QPSK_EXTENSION 16

BDBG_MODULE(bast_g3_priv_ldpc);


/******************************************************************************
 BAST_g3_P_IsLdpc8psk_isrsafe()
******************************************************************************/
bool BAST_g3_P_IsLdpc8psk_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_ENABLED) == BAST_LDPC_SCAN_STATE_ENABLED)
   {
      /* ldpc scan in progress */
      if ((hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_QPSK) == 0)
         return true;
   }
   else if (BAST_MODE_IS_LDPC(hChn->actualMode))
   {
      if (BAST_MODE_IS_LDPC_8PSK(hChn->actualMode))
         return true;
   }
   else if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
      return true;
   return false;
}


/******************************************************************************
 BAST_g3_P_LdpcConfigEq_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcConfigEq_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, eqcfad, f0b, ffe_main_tap, i;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, &val);
   val &= 0x0000FF06;
   val |= 0x22060720;    /* orig: val |= 0x66060720;   */
val &= ~0x500;  /* unfreeze ffe and ffe main tap */
   if ((hChn->acqParams.mode == BAST_Mode_eLdpc_ACM) ||
       (BAST_g3_P_IsLdpcPilotOn_isrsafe(h)))
      val |= 0x18;
   else
      val |= 0x10;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, &val);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, &val);
   val &= ~0x0038401F;
   val |= 0x00140000;
   if ((hChn->acqParams.mode != BAST_Mode_eLdpc_ACM) && BAST_g3_P_IsLdpc8psk_isrsafe(h))
      val |= 0x1A; /* err_mode, sym_mode, dvbs2_8psk_mapping */
   if ((hChn->acqParams.mode == BAST_Mode_eLdpc_ACM) || BAST_g3_P_IsLdpc8psk_isrsafe(h))
      val |= 0x4000;  /* ext_en */
   if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
   {
     /* this is the setting from Michael's script */
     val &= ~0x1F;
     val |= 0x1A;
   }
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, &val);

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, &val);
   ffe_main_tap = (val >> 16) & 0x1F;
   for (i = 0; i < 24; i++)
   {
      val = 0x00;
      if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
      {
         if (i == ffe_main_tap)
            val = 0x20;
      }
      else if (BAST_g3_P_IsLdpc8psk_isrsafe(h))
      {
         /* 8PSK */
         if (i == ffe_main_tap)
         {
#ifdef BAST_HAS_WFE
            val = 0x3A;
#else
            val = 0x20;
#endif
         }
      }
      else
      {
         /* QPSK */
         if (i == ffe_main_tap)
         {
#ifdef BAST_HAS_WFE
            val = 0x29;
#else
            val = 0x18;
#endif
         }
      }

      eqcfad = 0x40 | i;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQCFAD, &eqcfad);

      f0b = (val & 0xFF) << 24;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_F0B, &f0b);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcSetVlctl_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcSetVlctl_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = 0x00040914;
   if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
      val |= 0x00020008; /* set bit 17 (vlc_soft_insel), set bit 3 (vgain_sel) */
   else if (BAST_g3_P_IsLdpc8psk_isrsafe(h))
      val |= 0x020000; /* set bit 17 (vlc_soft_insel) */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCTL, &val);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcSetHardDecisionLevels_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcSetHardDecisionLevels_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = 0x01D901D9;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HD8PSK1, &val);
   val = 0x00C400C4;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HD8PSK2, &val);

   if (hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_ENABLED)
   {
      if (hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_FOUND)
      {
         if (hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_QPSK)
            goto ldpc_set_hd_1;
         else
            goto ldpc_set_hd_0;
      }
      else
      {
         /* assume 8PSK in scan mode */
         goto ldpc_set_hd_0;
      }
   }
   else if ((hChn->acqParams.mode == BAST_Mode_eLdpc_ACM) || BAST_g3_P_IsLdpc8psk_isrsafe(h))
   {
      ldpc_set_hd_0:
      val = 0x016A0000;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, &val);
   }
   else
   {
      /* LDPC QPSK */
      ldpc_set_hd_1:
      val = 0x01000000;
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, &val);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcSetScramblingSeq_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcSetScramblingSeq_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = (hChn->ldpcScramblingSeq[0] << 24) | (hChn->ldpcScramblingSeq[1] << 16) |
         (hChn->ldpcScramblingSeq[2] << 8) | hChn->ldpcScramblingSeq[3];
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_XSEED, &val);
   val = (hChn->ldpcScramblingSeq[4] << 24) | (hChn->ldpcScramblingSeq[5] << 16) |
         (hChn->ldpcScramblingSeq[6] << 8) | hChn->ldpcScramblingSeq[7];
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_PLHDRSCR1, &val);
   val = (hChn->ldpcScramblingSeq[8] << 24) | (hChn->ldpcScramblingSeq[9] << 16) |
         (hChn->ldpcScramblingSeq[10] << 8) | hChn->ldpcScramblingSeq[11];
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_PLHDRSCR2, &val);
   val = (hChn->ldpcScramblingSeq[12] << 24) | (hChn->ldpcScramblingSeq[13] << 16) |
         (hChn->ldpcScramblingSeq[14] << 8) | hChn->ldpcScramblingSeq[15];
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_PLHDRSCR3, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcSetPilotctl_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcSetPilotctl_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = 0;
   if ((hChn->acqParams.mode != BAST_Mode_eLdpc_ACM) && BAST_g3_P_IsLdpcPilotOn_isrsafe(h))
      val |= 0x01; /* pilot_mode */
   if (BAST_g3_P_IsLdpcPilotOn_isrsafe(h) || (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM))
      val |= 0x02; /* pilot_update_mode */
   if ((hChn->acqParams.mode != BAST_Mode_eLdpc_ACM) && (BAST_g3_P_IsLdpcPilotOn_isrsafe(h) == false))
      val |= 0x04; /* phase_adj_en */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_PILOTCTL, &val);
   return BERR_SUCCESS;
}



/******************************************************************************
 BAST_g3_P_LdpcGetPdTableIdx_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcGetPdTableIdx_isr(uint32_t i, uint32_t j, uint32_t *pIdx)
{
   uint32_t sval1;

   /* idx = 30*i - i*(i-1)/2 + j - 1; */
   sval1 = (i * (i - 1)) >> 1;
   *pIdx = 30 * i - sval1 + j - 1;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcGeneratePdTable_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcGeneratePdTable_isr(BAST_ChannelHandle h)
{
   static const uint8_t pd_data_qpsk[] =
   {
      0x00,
      0x2E,
      0x54,
      0x70,
      0x81,
      0x8C,
      0x91,
      0x95,
      0x97,
      0x98,
      0x98,
      0x98,
      0x99,
      0x99,
      0x99,
      0x99,
      0x99,
   };

   static const uint8_t pd_data_8psk[] =
   {
      0x00, 0x01, 0x06,
      0x00, 0x01, 0x6F,
      0x00, 0x01, 0x93,
      0x00, 0x01, 0xA1,
      0x00, 0x01, 0xA8,
      0x00, 0x01, 0xAB,
      0x00, 0x01, 0xAD,
      0x00, 0x01, 0xAD,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x00, 0x01, 0xAE,
      0x01, 0x55, 0x54,
      0x01, 0x05, 0x83,
      0x00, 0xD9, 0x99,
      0x00, 0xC3, 0xA4,
      0x00, 0xB7, 0xA9,
      0x00, 0xB3, 0xAC,
      0x00, 0xAF, 0xAD,
      0x00, 0xAF, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x00, 0xAD, 0xAE,
      0x01, 0xC7, 0x5C,
      0x01, 0x79, 0x82,
      0x01, 0x4B, 0x98,
      0x01, 0x31, 0xA3,
      0x01, 0x23, 0xA9,
      0x01, 0x1D, 0xAB,
      0x01, 0x1B, 0xAD,
      0x01, 0x19, 0xAD,
      0x01, 0x19, 0xAE,
      0x01, 0x19, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0x17, 0xAE,
      0x01, 0xF1, 0x5D,
      0x01, 0xA7, 0x81,
      0x01, 0x79, 0x96,
      0x01, 0x61, 0xA2,
      0x01, 0x55, 0xA8,
      0x01, 0x4D, 0xAB,
      0x01, 0x4B, 0xAD,
      0x01, 0x49, 0xAD,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0x49, 0xAE,
      0x01, 0xFF, 0x5D,
      0x01, 0xB9, 0x80,
      0x01, 0x8B, 0x96,
      0x01, 0x73, 0xA2,
      0x01, 0x67, 0xA8,
      0x01, 0x61, 0xAB,
      0x01, 0x5D, 0xAD,
      0x01, 0x5D, 0xAD,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x01, 0x5B, 0xAE,
      0x02, 0x05, 0x5D,
      0x01, 0xBF, 0x80,
      0x01, 0x93, 0x96,
      0x01, 0x7B, 0xA2,
      0x01, 0x6D, 0xA8,
      0x01, 0x67, 0xAB,
      0x01, 0x65, 0xAD,
      0x01, 0x63, 0xAD,
      0x01, 0x63, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x01, 0x61, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC1, 0x80,
      0x01, 0x95, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x6F, 0xA8,
      0x01, 0x69, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x01, 0x63, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC1, 0x80,
      0x01, 0x95, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC1, 0x80,
      0x01, 0x95, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC1, 0x80,
      0x01, 0x95, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x01, 0x65, 0xAE,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x01, 0x67, 0xAD,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x01, 0x67, 0xAD,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x01, 0x6B, 0xAB,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x01, 0x71, 0xA8,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x01, 0x7D, 0xA2,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x01, 0x97, 0x96,
      0x02, 0x07, 0x5D,
      0x01, 0xC3, 0x80,
      0x02, 0x07, 0x5D,
   };

   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val, i, j, idx;
   uint8_t isb_byte1, isb_byte2, isb_byte3;

#if 0
   if ((BAST_g3_P_IsLdpcPilotOn_isrsafe(h) == true) && (BAST_g3_P_IsLdpc8psk_isrsafe(h)) && (hChn->acqParams.symbolRate < 8000000))
   {
      /* don't use soft PD table for 8psk no pilot below 8MBaud */
      return BERR_SUCCESS;
   }
#endif

   /* enable PD LUT memory micro access */
   val = 0x80000000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_LUPA, &val);

   for (i = 0; i < 32; i++)
   {
      for (j = 0; j < 32; j++)
      {
         if (BAST_g3_P_IsLdpc8psk_isrsafe(h))
         {
            /* 8psk case */
            if (i < j)
            {
               /* i < j case */
               BAST_CHK_RETCODE(BAST_g3_P_LdpcGetPdTableIdx_isr(i, j, &idx));
               idx *= 3;
               isb_byte3 = pd_data_8psk[idx];
               isb_byte2 = pd_data_8psk[idx + 1];
               isb_byte1 = pd_data_8psk[idx + 2];
            }
            else if (i > j)
            {
               /* i > j case, must transpose value also */
               BAST_CHK_RETCODE(BAST_g3_P_LdpcGetPdTableIdx_isr(j, i, &idx));
               idx *= 3;
               isb_byte3 = ((pd_data_8psk[idx + 1] << 1) | (pd_data_8psk[idx + 2] >> 7)) & 0x3;
               isb_byte2 = (pd_data_8psk[idx + 2] << 1) | (pd_data_8psk[idx] >> 1);
               isb_byte1 = (pd_data_8psk[idx] << 7) | (pd_data_8psk[idx + 1] >> 1);
            }
            else
            {
               isb_byte1 = isb_byte2 = isb_byte3 = 0;
            }
         }
         else
         {
            /* qpsk case */
            isb_byte1 = pd_data_qpsk[(j < BAST_g3_PD_DATA_QPSK_EXTENSION) ? j : BAST_g3_PD_DATA_QPSK_EXTENSION];
            val = pd_data_qpsk[(i < BAST_g3_PD_DATA_QPSK_EXTENSION) ? i : BAST_g3_PD_DATA_QPSK_EXTENSION];
            isb_byte2 = (val << 1);
            isb_byte3 = (val >> 7);
         }
         val = (isb_byte3 << 24) | (isb_byte2 << 16) | (isb_byte1 << 8);
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_LUPD, &val);
      }
   }

   /* disable PD LUT memory micro access */
   val = 0x00000000;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_LUPA, &val);

   /* deselect CCI phase detect, select soft slicer for carrier loop */
   BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, ~0x0000040, 0x00000020);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_LdpcConfigSnr_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcConfigSnr_isr(BAST_ChannelHandle h)
{
   static const uint32_t LDPC_SNRHT[] =
   {
      0x03c03235, /* LDPC QPSK 1/2 */
      0x02e95977, /* LDPC QPSK 3/5 */
      0x026bf466, /* LDPC QPSK 2/3 */
      0x01ec7290, /* LDPC QPSK 3/4 */
      0x01a3240c, /* LDPC QPSK 4/5 */
      0x01758f45, /* LDPC QPSK 5/6 */
      0x0128ba9e, /* LDPC QPSK 8/9 */
      0x011b5fbc, /* LDPC QPSK 9/10 */
      0x0299deea, /* LDPC 8PSK 3/5 */
      0x0210eb81, /* LDPC 8PSK 2/3 */
      0x01881801, /* LDPC 8PSK 3/4 */
      0x011594c5, /* LDPC 8PSK 5/6 */
      0x00c916fa, /* LDPC 8PSK 8/9 */
      0x00bbaafa  /* LDPC 8PSK 9/10 */
   };

   static const uint32_t LDPC_SNRLT[] =
   {
      0x2581f613, /* LDPC QPSK 1/2 */
      0x2581f613, /* LDPC QPSK 3/5 */
      0x18378c00, /* LDPC QPSK 2/3 */
      0x133c79a2, /* LDPC QPSK 3/4 */
      0x105f6879, /* LDPC QPSK 4/5 */
      0x0e9798ae, /* LDPC QPSK 5/6 */
      0x0b974a29, /* LDPC QPSK 8/9 */
      0x0b11bd5a, /* LDPC QPSK 9/10 */
      0x1a02b525, /* LDPC 8PSK 3/5 */
      0x14a9330f, /* LDPC 8PSK 2/3 */
      0x0f50f00e, /* LDPC 8PSK 3/4 */
      0x0ad7cfb3, /* LDPC 8PSK 5/6 */
      0x07dae5c5, /* LDPC 8PSK 8/9 */
      0x0754adc5  /* LDPC 8PSK 9/10 */
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BAST_g3_P_Handle *hDev = h->pDevice->pImpl;
   BERR_Code retCode;
   uint32_t val, i;

   if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
   {
      /* TBD */
   }
   else
   {
      i = hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2;
      val = LDPC_SNRHT[i];
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRHT, &val);
      val = LDPC_SNRLT[i];
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRLT, &val);
   }

   if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
      val = 0x01546732;
   else if (BAST_g3_P_IsLdpc8psk_isrsafe(h))
   {
      if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_PN)
         val = 0x04623751;
      else
         val = 0x01326754;
   }
   else
      val = 0x04576231;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEM1, &val);

   val = 0x01546732;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEM2, &val);

   BAST_CHK_RETCODE(BAST_g3_P_InitBert_isr(h));

   val = 0x0FFF05FF;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEIT, &val);

   val = 0x88;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRCTL, &val);
   val = 0x0B;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRCTL, &val);

   if (hDev->sdsRevId >= 0x64)
      val = 0xA3; /* alpha=2^-8 */
   else
      val = 0xA4; /* alpha=2^-5 in A0 */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, &val);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_LdpcConfigCycleCnt_isr()
  Algorithm:
  S = frame size = 64800 (or 16200 for short frames)
  R = LDPC slow clock rate = 216 MHz
  L = cycle count adjustment = 50
  Fb = symbol rate
  ACM_CYCLE_CNT_0 = ((R*S/2) / Fb) - L
  ACM_CYCLE_CNT_1 = ((R*S/3) / Fb) - L
  ACM_CYCLE_CNT_2 = ((R*S/4) / Fb) - L
  ACM_CYCLE_CNT_3 = ((R*S/5) / Fb) - L
******************************************************************************/
BERR_Code BAST_g3_P_LdpcConfigCycleCnt_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, P_hi, P_lo, Q_hi, Q_lo;

   /* Q_lo = R*S/Fb */
   if (hChn->ldpcCtl & BAST_G3_CONFIG_LDPC_CTL_SHORT_FRAME_DETECTED)
      val = 16200 * 256;
   else
      val = 64800 * 256;
   BMTH_HILO_32TO64_Mul(hChn->fecFreq, val, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);

   val = ((Q_lo / 2) - 50*256) >> 8;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_ACM_CYCLE_CNT_0, &val);

   val *= 3;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_ACM_MISC_1, &val);

   val = ((Q_lo / 3) - 50*256) >> 8;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_ACM_CYCLE_CNT_1, &val);

   val = ((Q_lo / 4) - 50*256) >> 8;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_ACM_CYCLE_CNT_2, &val);

   val = ((Q_lo / 5) - 50*256) >> 8;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_ACM_CYCLE_CNT_3, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcSetMpcfg_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcSetMpcfg_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_MPCFG, &val);
   val &= 0xFFFFFFCF;
   if ((hChn->xportSettings.bchMpegErrorMode == BAST_BchMpegErrorMode_eBch) ||
       (hChn->xportSettings.bchMpegErrorMode == BAST_BchMpegErrorMode_eBchAndCrc8))
      val |= 0x20;
   if ((hChn->xportSettings.bchMpegErrorMode == BAST_BchMpegErrorMode_eCrc8) ||
       (hChn->xportSettings.bchMpegErrorMode == BAST_BchMpegErrorMode_eBchAndCrc8))
      val |= 0x10;
   if (hChn->xportSettings.bTei)
      val |= 0x08;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_MPCFG, &val);

   val = 0x030F0E0F;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_BCH_MPLCK, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcSetModcod_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcSetModcod_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, i;

#if 0
   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_ENABLED) == 0)
   {
      val = 0;
      if (hChn->acqParams.mode != BAST_Mode_eLdpc_ACM)
      {
         val |= 0x20;
         i = hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2;
         val |= (i + 4);
      }
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_ACM_MODCOD_OVERIDE, &val);
   }
#else
   val = 0;
   if (hChn->acqParams.mode != BAST_Mode_eLdpc_ACM)
   {
      val |= 0x20;
      i = hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2;
      val |= (i + 4);
   }

   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_ACM_MODCOD_OVERIDE, &val);
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcConfig_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcConfig_isr(BAST_ChannelHandle h)
{
   static const uint32_t script_ldpc_config_0[] =
   {
      BAST_SCRIPT_WRITE(BCHP_AFEC_LDPC_CONFIG_0, 0x05000b02), /* to make lock LDPC easy, was 05000b05 */
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MAX_ITER_OVERIDE, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_1, 0x04c05574),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_2, 0x04c06f84),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_3, 0x04c07cf4),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_4, 0x04c068e5),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_5, 0x04c05f07),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_6, 0x04c05a88),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_7, 0x04c06348),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_8, 0x04c06cd8),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_9, 0x04c071b8),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_10, 0x04c08748),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_11, 0x04c08378),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_12, 0x04c079d8),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_13, 0x04c08748),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_14, 0x04c09b58),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_15, 0x04c0ba18),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_16, 0x04c0be99),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_17, 0x04c0c549),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_18, 0x04c09fd9),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_19, 0x04c0b859),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_20, 0x04c0ca99),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_21, 0x04c0d899),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_22, 0x04c0faf9),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_23, 0x04c101f9),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_24, 0x04c0f659),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_25, 0x04c11199),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_26, 0x04c12679),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_27, 0x04c15a69),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_28, 0x04c16909),
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_ldpc_config_1[] = /* normal frame */
   {
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_SYM_CNT_0, 0x54607E90),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_SYM_CNT_1, 0x3F4832A0),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MISC_0, 0x00010001), /* orig: 0x00110001 */
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_ldpc_config_2[] = /* short frame */
   {
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_SYM_CNT_0, 0x15181FA4),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_SYM_CNT_1, 0x0FD20CA8),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MISC_0, 0x00010009), /* orig: 0x00110009 */
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_ldpc_config_7[] =
   {
      BAST_SCRIPT_WRITE(BCHP_AFEC_BCH_CTRL, 0x00000001),
      BAST_SCRIPT_WRITE(BCHP_AFEC_BCH_BBHDR4, 0x000005E0),
      BAST_SCRIPT_WRITE(BCHP_AFEC_BCH_BBHDR3, 0x0043000C),
      BAST_SCRIPT_WRITE(BCHP_AFEC_BCH_SMCFG, 0xE000005E),
      BAST_SCRIPT_WRITE(BCHP_AFEC_ACM_MODCOD_STATS_CONFIG, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_AFEC_CNTR_CTRL, 0x00000003), /* clear counters */
      BAST_SCRIPT_EXIT
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val;

   retCode = BAST_g3_P_LdpcUpdateBlockCounters_isrsafe(h); /* need to do this because counters are about to be reset */
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BAST_g3_P_LdpcConfig_isr(): BAST_g3_P_LdpcUpdateBlockCounters_isrsafe() error 0x%X", retCode));
   }
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_AFEC_RST, 0x00000001);

   /* ldpc_set_dafe_acq_bw() */
   if (hChn->acqParams.mode != BAST_Mode_eLdpc_ACM)
   {
      if (hChn->actualMode <= BAST_Mode_eLdpc_Qpsk_2_3)
         val = 0x0000406A;
      else if (hChn->actualMode <= BAST_Mode_eLdpc_Qpsk_9_10)
         val = 0x0000506A;
      else if (hChn->actualMode <= BAST_Mode_eLdpc_8psk_3_4)
         val = 0x0000606A;
      else
         val = 0x0000606A; /* was 0x0000708C;  too wide, fell out of lock */
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, 0x60000, val);
   }
   else
   {
      /* TBD: adjust CLDAFECTL for ACM */
   }

   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_ldpc_config_0));
   if (hChn->ldpcCtl & BAST_G3_CONFIG_LDPC_CTL_SHORT_FRAME_DETECTED)
   {
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_ldpc_config_2));
   }
   else
   {
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_ldpc_config_1));
   }

   BAST_CHK_RETCODE(BAST_g3_P_LdpcConfigCycleCnt_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_ldpc_config_7));
   BAST_CHK_RETCODE(BAST_g3_P_LdpcSetMpcfg_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_LdpcSetModcod_isr(h));

   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_AFEC_CNTR_CTRL, 0x00000003); /* clear counters */
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_AFEC_RST, 0x00000100); /* reset data path */

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_LdpcSetPsl_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcSetPsl_isr(BAST_ChannelHandle h)
{
   static const uint32_t AFEC_MODCOD_REGISTER[] =
   {
      BCHP_AFEC_ACM_MODCOD_4, /* LDPC_QPSK_1_2 */
      BCHP_AFEC_ACM_MODCOD_5, /* LDPC_QPSK_3_5 */
      BCHP_AFEC_ACM_MODCOD_6, /* LDPC_QPSK_2_3 */
      BCHP_AFEC_ACM_MODCOD_7, /* LDPC_QPSK_3_4 */
      BCHP_AFEC_ACM_MODCOD_8, /* LDPC_QPSK_4_5 */
      BCHP_AFEC_ACM_MODCOD_9, /* LDPC_QPSK_5_6 */
      BCHP_AFEC_ACM_MODCOD_10, /* LDPC_QPSK_8_9 */
      BCHP_AFEC_ACM_MODCOD_11, /* LDPC_QPSK_9_10 */
      BCHP_AFEC_ACM_MODCOD_12, /* LDPC_8PSK_3_5 */
      BCHP_AFEC_ACM_MODCOD_13, /* LDPC_8PSK_2_3 */
      BCHP_AFEC_ACM_MODCOD_14, /* LDPC_8PSK_3_4 */
      BCHP_AFEC_ACM_MODCOD_15, /* LDPC_8PSK_5_6 */
      BCHP_AFEC_ACM_MODCOD_16, /* LDPC_8PSK_8_9 */
      BCHP_AFEC_ACM_MODCOD_17  /* LDPC_8PSK_9_10 */
   };

   static const uint16_t AFEC_CYCLES_ONE_TIME[] =
   {
      1375, /* LDPC_QPSK_1_2 */
      1725, /* LDPC_QPSK_3_5 */
      1353, /* LDPC_QPSK_2_3 */
      1431, /* LDPC_QPSK_3_4 */
      1479, /* LDPC_QPSK_4_5 */
      1515, /* LDPC_QPSK_5_6 */
      1281, /* LDPC_QPSK_8_9 */
      1286, /* LDPC_QPSK_9_10 */
      1725, /* LDPC_8PSK_3_5 */
      1353, /* LDPC_8PSK_2_3 */
      1431, /* LDPC_8PSK_3_4 */
      1515, /* LDPC_8PSK_5_6 */
      1281, /* LDPC_8PSK_8_9 */
      1286, /* LDPC_8PSK_9_10 */
   };

   static const uint16_t AFEC_CYCLES_PER_ITER[] =
   {
      1287, /* LDPC_QPSK_1_2 */
      1619, /* LDPC_QPSK_3_5 */
      1235, /* LDPC_QPSK_2_3 */
      1298, /* LDPC_QPSK_3_4 */
      1337, /* LDPC_QPSK_4_5 */
      1367, /* LDPC_QPSK_5_6 */
      1123, /* LDPC_QPSK_8_9 */
      1126, /* LDPC_QPSK_9_10 */
      1619, /* LDPC_8PSK_3_5 */
      1235, /* LDPC_8PSK_2_3 */
      1298, /* LDPC_8PSK_3_4 */
      1367, /* LDPC_8PSK_5_6 */
      1123, /* LDPC_8PSK_8_9 */
      1126, /* LDPC_8PSK_9_10 */
   };

#if BCHP_CHIP==4538
   BAST_P_Handle *hDev = (BAST_P_Handle*)h->pDevice;
#endif
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t val, psl_ctl, i, max_iter, thresh, pct, gain, beta;

   if ((hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_ENABLED) ||
       (hChn->ldpcCtl & BAST_G3_CONFIG_LDPC_CTL_DISABLE_PSL))
   {
#if BCHP_CHIP==4538
      disable_psl:
#endif
      psl_ctl = 0x50;
   }
   else
   {
      i = hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2;

      /* calculate max iterations */
      if (BAST_MODE_IS_LDPC_8PSK(hChn->actualMode))
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_AFEC_ACM_CYCLE_CNT_1, &val);
      else
         BAST_g3_P_ReadRegister_isrsafe(h, BCHP_AFEC_ACM_CYCLE_CNT_0, &val);
      max_iter = ((val - AFEC_CYCLES_ONE_TIME[i]) / AFEC_CYCLES_PER_ITER[i]);
      if (max_iter > 150)
         max_iter = 150;

      BAST_g3_P_ReadRegister_isrsafe(h, AFEC_MODCOD_REGISTER[i], &val);
      val &= 0x000FFFFF;
      val |= (max_iter << 20);
      BAST_g3_P_WriteRegister_isrsafe(h, AFEC_MODCOD_REGISTER[i], &val);

#if BCHP_CHIP==4538
   if (hChn->acqParams.symbolRate > 30000000)
      goto disable_psl;
   if (hDev->settings.networkSpec == BAST_NetworkSpec_eDefault)
   {
      pct = 372;
   }
   else
   {
      pct = 298;
   }
#else
      /* set PSL threshold to 68% max iterations for 8psk 3/5
         set PSL threshold to 56% max iterations for all other code rates */
      if (hChn->actualMode == BAST_Mode_eLdpc_8psk_3_5)
         pct = 680;
      else if (hChn->actualMode == BAST_Mode_eLdpc_8psk_3_4)
         pct = 750;
      else
         pct = 560;
#endif
      thresh = ((max_iter * pct) + 500) / 1000;
      thresh = thresh << 2;

      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_PSL_FILTER, &val);
      val &= 0xFFFF001F;
      val |= (thresh << 5);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_PSL_FILTER, &val);

      /* set gain and beta */
      if (hChn->acqParams.symbolRate < 5000000)
      {
         gain = 0x0B;
         beta = 0x0A;
      }
      else
      {
         gain = 0x09;
         beta = 0x07;
      }

      psl_ctl = ((beta << 4) & 0xF0) | 0x05;
      psl_ctl |= ((gain & 0x0F) | (((thresh & 0x0003) << 6) & 0xC0)) << 8;
      psl_ctl |= ((thresh >> 2) & 0xFF) << 16;
      psl_ctl |= ((thresh >> 10) & 0x07) << 24;
   }
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_LDPC_PSL_CTL, &psl_ctl);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcSetOpll_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcSetOpll_isr(BAST_ChannelHandle h)
{
   static const uint16_t LDPC_NUMBER_OF_BITS[] =
   {
      32128, /* 1/2 */
      38608, /* 3/5 */
      42960, /* 2/3 */
      48328, /* 3/4 */
      51568, /* 4/5 */
      53760, /* 5/6 */
      57392, /* 8/9 */
      58112, /* 9/10 */
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t data0, i, lval1, lval2, number_of_bits, number_of_symbols;
   uint32_t P_hi, P_lo, Q_hi, Q_lo;
   BAST_Mode mode;

   if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
      mode = hChn->acmMaxMode;
   else
      mode = hChn->actualMode;

   if (BAST_MODE_IS_LDPC_8PSK(mode))
   {
      /* 8PSK */
      lval1 = 21600;
      if (hChn->actualMode <= BAST_Mode_eLdpc_8psk_3_4)
         data0 = 1;
      else
         data0 = 2;
      lval2 = 504; /* 14*36 */
      i = mode - BAST_Mode_eLdpc_8psk_3_5 + data0;
   }
   else if (BAST_MODE_IS_LDPC_QPSK(mode))
   {
      /* QPSK */
      lval1 = 32400;
      lval2 = 792; /* 22*36 */
      i = mode - BAST_Mode_eLdpc_Qpsk_1_2;
   }
   else
   {
      BDBG_WRN(("Invalid mode: %08X", mode));
      return BERR_INVALID_PARAMETER;
   }

   number_of_bits = (uint32_t)LDPC_NUMBER_OF_BITS[i];

   if (BAST_g3_P_IsLdpcPilotOn_isrsafe(h) || (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM))
      lval1 += lval2;
   number_of_symbols = lval1 + 90;

   /* opll_N = final N */
   hChn->opll_N = (number_of_bits >> 1);

   /* opll_D = final D */
   if (hChn->bUndersample)
      hChn->opll_D = number_of_symbols;
   else
      hChn->opll_D = (number_of_symbols << 1);

   BMTH_HILO_32TO64_Mul(hChn->acqParams.symbolRate, number_of_bits * 2, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, number_of_symbols, &Q_hi, &Q_lo);
   hChn->outputBitRate = (Q_lo + 1) >> 1;

   return BERR_SUCCESS;
}


#if (BCHP_CHIP==4528) || (BCHP_CHIP==4538)
/******************************************************************************
 BAST_g3_P_LdpcEnableDynamicPowerShutDown_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcEnableDynamicPowerShutDown_isrsafe(BAST_ChannelHandle h, bool bEnable)
{
   uint32_t val;

   if (BAST_g3_P_IsLdpcOn_isrsafe(h) == false)
      return BERR_SUCCESS;

   if (bEnable)
   {
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_AFEC_ACM_MISC_0, 1<<20);
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_AFEC_LDPC_MEM_POWER, 1);
   }
   else
   {
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_AFEC_LDPC_MEM_POWER, ~1);
      BAST_g3_P_AndRegister_isrsafe(h, BCHP_AFEC_ACM_MISC_0, ~(1<<20));

      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_STATUS, &val);
      if ((val & (1<<28)) == 0)
      {
         /* power up the memory using correct sequence */
         BAST_g3_P_OrRegister_isrsafe(h, BCHP_AFEC_LDPC_MEM_POWER, 1<<31);
      }
   }

   return BERR_SUCCESS;
}
#endif


/******************************************************************************
 BAST_g3_P_LdpcOnHpLock_isr() - called when HP locks
******************************************************************************/
BERR_Code BAST_g3_P_LdpcOnHpLock_isr(BAST_ChannelHandle h)
{
   static const uint32_t script_ldpc_5[] =
   {
      BAST_SCRIPT_AND(BCHP_SDS_EQ_EQMISCCTL, 0xFFFFFBFF),           /* disable CMA */
      BAST_SCRIPT_AND_OR(BCHP_SDS_EQ_EQFFECTL, 0xFFFF00FF, 0x0200), /* unfreze other taps */
      BAST_SCRIPT_EXIT
   };

   static const uint32_t script_ldpc_7[] =
   {
      BAST_SCRIPT_OR(BCHP_SDS_HP_HPOVERRIDE, 0x0000000F),  /* override front carrier loop */
      BAST_SCRIPT_AND(BCHP_SDS_CL_CLCTL1, 0xFFFFFFEF),     /* disable front carrier loop */
      BAST_SCRIPT_OR(BCHP_SDS_CL_CLCTL2, 0x00000004),      /* freeze front carrier loop */
      BAST_SCRIPT_EXIT
   };


   static const uint32_t script_ldpc_6[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_MGAINA, 0x00000000),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_MGAIND, 0x5a383838),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_MGAIND, 0x384e5a5a),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_MGAIND, 0x5a5a5a5a),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_MGAIND, 0x40404040),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_MGAIND, 0x48480000),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_MGAINA, 0x00000000),
      BAST_SCRIPT_EXIT
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val;

   BAST_CHK_RETCODE(BAST_g3_P_LdpcGeneratePdTable_isr(h));

   if (hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_ENABLED)
   {
      BAST_CHK_RETCODE(BAST_g3_P_ConfigPlc_isr(h, true)); /* set acquisition plc to the non-scan plc value */
      BAST_CHK_RETCODE(BAST_g3_P_LdpcSetVlctl_isr(h));
      BAST_CHK_RETCODE(BAST_g3_P_LdpcConfigEq_isr(h));

      if (BAST_MODE_IS_LDPC_QPSK(hChn->actualMode))
      {
         /* we need to reprogram HDQPSK because we assumed 8PSK when acquiring in scan mode */
         val = 0x01000000;
         BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_HDQPSK, &val);
      }
   }
   else if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT_SCAN)
   {
      BAST_CHK_RETCODE(BAST_g3_P_LdpcConfigEq_isr(h));
   }

   BAST_CHK_RETCODE(BAST_g3_P_LdpcSetPilotctl_isr(h));

   /* this causes constellation to be screwed up when fine freq is enabled */
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_ldpc_5));
   if (hChn->bEnableFineFreq == false)
   {
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_ldpc_7));
   }

   if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
   {
      BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_ldpc_6));
   }
   else
   {
      if (hChn->actualMode >= BAST_Mode_eLdpc_8psk_8_9)
         val = 0x24000000;
      else if (BAST_MODE_IS_LDPC_8PSK(hChn->actualMode))
         val = 0x28000000;
      else if (hChn->actualMode == BAST_Mode_eLdpc_Qpsk_1_2)
         val = 0x33000000;
      else if (hChn->actualMode == BAST_Mode_eLdpc_Qpsk_3_5)
         val = 0x3E000000;
      else
         val = 0x43000000;    /* rest of qpsk */
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCI, &val);
      BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCQ, &val);
   }

   BAST_CHK_RETCODE(BAST_g3_P_SetAgcTrackingBw_isr(h));

   val = 0x34;
   if ((hChn->acqParams.mode == BAST_Mode_eLdpc_ACM) || BAST_g3_P_IsLdpcPilotOn_isrsafe(h))
      val |= 0x08;
   if ((hChn->acqParams.mode == BAST_Mode_eLdpc_ACM) || BAST_g3_P_IsLdpc8psk_isrsafe(h))
      val |= 0xC0;
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLFBCTL, &val);

   BAST_CHK_RETCODE(BAST_g3_P_LdpcConfigSnr_isr(h));

   BAST_CHK_RETCODE(BAST_g3_P_LdpcConfig_isr(h));
#if (BCHP_CHIP==4528) || (BCHP_CHIP==4538)
   BAST_CHK_RETCODE(BAST_g3_P_LdpcEnableDynamicPowerShutDown_isrsafe(h, true));
#endif
   BAST_CHK_RETCODE(BAST_g3_P_LdpcSetPsl_isr(h));

   BAST_g3_P_SdsPowerUpOpll_isr(h);
   BAST_CHK_RETCODE(BAST_g3_P_LdpcSetOpll_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_ConfigOif_isr(h));

   retCode = BAST_g3_P_StartTracking_isr(h);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_LdpcAcquire1_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcAcquire1_isr(BAST_ChannelHandle h)
{
   return BAST_g3_P_HpAcquire_isr(h, BAST_g3_P_LdpcOnHpLock_isr);
}


/******************************************************************************
 BAST_g3_P_LdpcAcquire_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcAcquire_isr(BAST_ChannelHandle h)
{
   static const uint32_t script_ldpc_4[] =
   {
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_XTAP1, 0x00000100),
      BAST_SCRIPT_WRITE(BCHP_SDS_EQ_XTAP2, 0x00805000),
      BAST_SCRIPT_EXIT
   };

   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val;

   if (hChn->bHasAfec == false)
   {
      if (hChn->bBlindScan)
      {
         hChn->reacqCause = BAST_ReacqCause_eNoAfec;
         return BAST_g3_P_Reacquire_isr(h);
      }
      else
      {
         /* abort acq */
         BAST_CHK_RETCODE(BAST_g3_P_DisableChannelInterrupts_isr(h, false, false));
         BAST_CHK_RETCODE(BAST_g3_P_SdsDisableOif_isrsafe(h));
         BAST_g3_P_IndicateNotLocked_isrsafe(h);
         hChn->acqTime = 0;
         hChn->acqState = BAST_AcqState_eIdle;
         hChn->reacqCtl &= (BAST_G3_CONFIG_REACQ_CTL_FREQ_DRIFT | BAST_G3_CONFIG_REACQ_CTL_RETUNE);
         return BERR_SUCCESS;
      }
   }

   if (hChn->acqParams.mode == BAST_Mode_eLdpc_scan)
      hChn->ldpcScanState = BAST_LDPC_SCAN_STATE_ENABLED;
   else
      hChn->ldpcScanState = 0;

   /* software pilot scan */
   if ((hChn->acqCount > 0) && (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT_SCAN))
      hChn->acqParams.acq_ctl ^= BAST_ACQSETTINGS_LDPC_PILOT;

#if (BCHP_CHIP==4528) || (BCHP_CHIP==4538)
   BAST_CHK_RETCODE(BAST_g3_P_LdpcEnableDynamicPowerShutDown_isrsafe(h, false));
#endif

   BAST_CHK_RETCODE(BAST_g3_P_LdpcPowerUp_isrsafe(h));
   BAST_CHK_RETCODE(BAST_g3_P_NonLegacyModeAcquireInit_isr(h));

   BAST_CHK_RETCODE(BAST_g3_P_LdpcConfigEq_isr(h));
   BAST_g3_P_ToggleBit_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 1); /* reset the eq */
   BAST_CHK_RETCODE(BAST_g3_P_LdpcSetVlctl_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_LdpcSetHardDecisionLevels_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_LdpcSetScramblingSeq_isr(h));
   BAST_CHK_RETCODE(BAST_g3_P_ProcessScript_isrsafe(h, script_ldpc_4));

   val = 0x14;
   if ((BAST_g3_P_IsLdpc8psk_isrsafe(h) == 0) && (hChn->acqParams.mode != BAST_Mode_eLdpc_ACM))
   {
      /* DVB-S2 QPSK */
#ifdef BAST_HAS_WFE
      val |= 0x7800; /* 70% backoff */
#else
      val |= 0x8000;
#endif
   }
   else
   {
      /* DVB-S2 8PSK */
#ifdef BAST_HAS_WFE
      val |= 0x5500; /* 70% backoff */
#else
      val |= 0x4000;
#endif
   }
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_PLDCTL, &val);

   BAST_CHK_RETCODE(BAST_g3_P_ConfigPlc_isr(h, true)); /* set acquisition plc */
   BAST_CHK_RETCODE(BAST_g3_P_LdpcSetPilotctl_isr(h));

   /* disable soft pd tables until after HP locks*/
   BAST_g3_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, ~0x00000060);

   /* configure and run the HP */
   /* add delay for eq to adapt before enabling HP */
   retCode = BAST_g3_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 400000, BAST_g3_P_LdpcAcquire1_isr);

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_IsLdpcPilotOn_isrsafe()
******************************************************************************/
bool BAST_g3_P_IsLdpcPilotOn_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   if (hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_ENABLED)
   {
      if (hChn->ldpcScanState & BAST_LDPC_SCAN_STATE_PILOT)
         return true;
   }
   else if (hChn->acqParams.acq_ctl & BAST_ACQSETTINGS_LDPC_PILOT)
      return true;
   return false;
}


/******************************************************************************
 BAST_g3_P_LdpcUpdateBlockCounters_isrsafe()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcUpdateBlockCounters_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   uint32_t corr, bad, total;

   if (BAST_g3_P_IsLdpcOn_isrsafe(h))
   {
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_DECCBLK, &corr);
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_DECBBLK, &bad);
      BAST_g3_P_ReadRegister_isrsafe(h, BCHP_AFEC_BCH_DECNBLK, &total);
      BAST_g3_P_ToggleBit_isrsafe(h, BCHP_AFEC_CNTR_CTRL, 0x04); /* clear BCH counters */
      hChn->ldpcBadBlocks += bad;
      hChn->ldpcCorrBlocks += corr;
      hChn->ldpcTotalBlocks += total;
   }
   return BAST_g3_P_UpdateErrorCounters_isrsafe(h);
}


/******************************************************************************
 BAST_g3_P_LdpcCheckMode_isr() - ISR context; checks if the current LDPC mode is
 one of the modes which we are allowed to lock
******************************************************************************/
BERR_Code BAST_g3_P_LdpcCheckMode_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t i;
   uint16_t mask;

   if (hChn->acqParams.mode == BAST_Mode_eLdpc_ACM)
      goto done;

   if (BAST_MODE_IS_LDPC(hChn->actualMode))
   {
      if (hChn->actualMode == BAST_Mode_eLdpc_scan)
         goto invalid_condition;

      i = hChn->actualMode - BAST_Mode_eLdpc_Qpsk_1_2;
      mask = (uint16_t)(1 << i);
      if ((mask & hChn->ldpcScanModes) == 0)
      {
         hChn->reacqCause = BAST_ReacqCause_eInvalidMode;
         hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE;
      }
   }
   else
   {
      invalid_condition:
      BDBG_ERR(("BAST_g3_P_LdpcCheckMode_isr() - invalid condition"));
      hChn->reacqCause = BAST_ReacqCause_eInvalidCondition1;
      return BAST_g3_P_Reacquire_isr(h);
   }

   done:
   return retCode;
}


#ifndef BAST_HAS_LEAP
/******************************************************************************
 BAST_g3_P_IsLdpcOn_isrsafe()
******************************************************************************/
bool BAST_g3_P_IsLdpcOn_isrsafe(BAST_ChannelHandle h)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_AFEC_GLOBAL_CLK_CNTRL, &val);
   if (val & 0x00800000)
      return false;
   return true;
}


/******************************************************************************
 BAST_g3_P_LdpcPowerUp_isrsafe() - power up the afec core
******************************************************************************/
BERR_Code BAST_g3_P_LdpcPowerUp_isrsafe(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   uint32_t val;

   BAST_g3_P_AndRegister_isrsafe(h, BCHP_AFEC_GLOBAL_CLK_CNTRL, ~0x00800000);

#if (BCHP_CHIP==7346) || (BCHP_CHIP==73465)
   val = 0x00080000 | 4; /* afec mdiv=4 */
#else
   val = 0x00080000 | 6; /* afec mdiv=5 */
#endif
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_GLOBAL_CLK_CNTRL, &val);
   val |= 0x00008000;  /* load a new mdiv value */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_GLOBAL_CLK_CNTRL, &val);
   val &= 0xFF00FFFF; /* enable output and disable new mdiv load */
   BAST_g3_P_WriteRegister_isrsafe(h, BCHP_AFEC_GLOBAL_CLK_CNTRL, &val);

   if ((retCode = BAST_g3_P_GetAfecClock_isrsafe(h, &(hChn->fecFreq))) != BERR_SUCCESS)
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_CMD_FAIL);
      BDBG_ERR(("BAST_g3_P_LdpcPowerUp_isrsafe() failed"));
   }
   return retCode;
}


/******************************************************************************
 BAST_g3_P_LdpcPowerDown() - power down the afec core
******************************************************************************/
BERR_Code BAST_g3_P_LdpcPowerDown(BAST_ChannelHandle h)
{
   BERR_Code retCode;

#if (BCHP_CHIP==4528) || (BCHP_CHIP==4538)
   BAST_g3_P_LdpcEnableDynamicPowerShutDown_isrsafe(h, false);
#endif

   BAST_g3_P_OrRegister_isrsafe(h, BCHP_AFEC_GLOBAL_CLK_CNTRL, 0x00800000);
   if (BAST_g3_P_IsLdpcOn_isrsafe(h) == false)
      retCode = BERR_SUCCESS;
   else
   {
      BERR_TRACE(retCode = BAST_ERR_HAB_CMD_FAIL);
      BDBG_ERR(("BAST_g3_P_LdpcPowerDown() failed"));
   }
   return retCode;
}
#endif


/******************************************************************************
 BAST_g3_P_LdpcEnableLockInterrupts_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcEnableLockInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_ClearCallback_isr(hChn->hLdpcLockCb);
   BINT_ClearCallback_isr(hChn->hLdpcNotLockCb);

   BINT_EnableCallback_isr(hChn->hLdpcLockCb);
   BINT_EnableCallback_isr(hChn->hLdpcNotLockCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcDisableLockInterrupts_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcDisableLockInterrupts_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   BINT_DisableCallback_isr(hChn->hLdpcLockCb);
   BINT_DisableCallback_isr(hChn->hLdpcNotLockCb);

   BINT_ClearCallback_isr(hChn->hLdpcLockCb);
   BINT_ClearCallback_isr(hChn->hLdpcNotLockCb);
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcOnLock_isr() - This function is called when lock status transitions
                          from not_locked to locked.
******************************************************************************/
BERR_Code BAST_g3_P_LdpcOnLock_isr(BAST_ChannelHandle h)
{
   BSTD_UNUSED(h);

   /* do nothing */
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcOnLostLock_isr() - This function is called when lock status
                              transitions from locked to not_locked.
******************************************************************************/
BERR_Code BAST_g3_P_LdpcOnLostLock_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   BAST_CHK_RETCODE(BAST_g3_P_LdpcUpdateBlockCounters_isrsafe(h));

   if (BAST_g3_P_IsHpLocked_isrsafe(h) == false)
   {
      hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE;
      hChn->reacqCause = BAST_ReacqCause_eHpLostLock3;
      goto done;
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_LdpcIsMpegLocked_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcIsMpegLocked_isr(BAST_ChannelHandle h, bool *bLocked)
{
   uint32_t val;

   BAST_g3_P_ReadRegister_isrsafe(h, BCHP_AFEC_LDPC_STATUS, &val);
   if ((val & 0xC0000000) != 0xC0000000)
      *bLocked = false;
   else
      *bLocked = true;
   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcOnStableLock_isr() - ISR context
******************************************************************************/
BERR_Code BAST_g3_P_LdpcOnStableLock_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t val;
   bool bMpLocked;

   BAST_g3_P_LdpcIsMpegLocked_isr(h, &bMpLocked);
   if (!bMpLocked)
   {
#if 0
      /* MP is not locked, so force reacquire */
      BDBG_MSG(("BAST_g3_P_LdpcOnStableLock_isr: mpeg not locked, reacquiring..."));
      hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE;
      return BERR_SUCCESS;
#else
      hChn->count1 = 1;
#endif
   }

   if (hChn->bPlcTracking == false)
   {
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 0x00FFFFFF, 0x66000000); /* Chan: update mu */

      BAST_g3_P_ConfigPlc_isr(h, false); /* set tracking PLC */

#ifdef BAST_FROF2_WORKAROUND
      BAST_g3_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, 0x00000004);   /* freeze front carrier loop */
#endif
   }

   if (hChn->bEnableFineFreq == false)
   {
      /* narrow DAFE loop bw */
      if (BAST_g3_P_IsLdpcPilotOn_isrsafe(h)) /* all pilot on modes, except LDPC 8PSK 3/4 */
      {
         if (hChn->actualMode != BAST_Mode_eLdpc_8psk_3_4)
         {
            BAST_g3_P_ReadRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, &val);
            val |= 0x04; /* Use Pilot for FROF3 estimation to avoid error at the edge of freq. drift */
            BAST_g3_P_WriteRegister_isrsafe(h, BCHP_SDS_HP_HPCONTROL, &val);
            val = 0x00005044;
         }
         else
            val = 0x00005056;  /* 8psk 3/4 pilot  on, do not use pilot for FROF3 est. */
      }
      else
         val = 0x00005056;
      BAST_g3_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLDAFECTL, 0x60000, val);
   }

   /* set tracking baud loop bw */
   BAST_CHK_RETCODE(BAST_g3_P_SetBaudBw_isr(h, hChn->acqParams.symbolRate / 400, 4));

   if (hChn->bEverStableLock == false)
   {
      BAST_CHK_RETCODE(BAST_g3_P_LdpcUpdateBlockCounters_isrsafe(h));
      hChn->ldpcBadBlocks = 0;
      hChn->ldpcCorrBlocks = 0;
      hChn->ldpcTotalBlocks = 0;
   }

   done:
   return retCode;
}


/******************************************************************************
 BAST_g3_P_LdpcMonitorLock_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcMonitorLock_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;
   bool bMpLocked;

   BAST_g3_P_LdpcIsMpegLocked_isr(h, &bMpLocked);
   if (!bMpLocked)
   {
      hChn->count1++;
      if (hChn->count1 > 2)
      {
         /* MP is not locked, so force reacquire */
         BDBG_MSG(("BAST_g3_P_LdpcMonitorLock_isr: mpeg not locked, reacquiring..."));
         hChn->reacqCtl |= BAST_G3_CONFIG_REACQ_CTL_FORCE;
      }
   }
   else
      hChn->count1 = 0;
    return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g3_P_LdpcSetFunctTable_isr()
******************************************************************************/
BERR_Code BAST_g3_P_LdpcSetFunctTable_isr(BAST_ChannelHandle h)
{
   BAST_g3_P_ChannelHandle *hChn = (BAST_g3_P_ChannelHandle *)h->pImpl;

   hChn->acqFunct = BAST_g3_P_LdpcAcquire_isr;
   hChn->checkModeFunct = BAST_g3_P_LdpcCheckMode_isr;
   hChn->onLockFunct = BAST_g3_P_LdpcOnLock_isr;
   hChn->onLostLockFunct = BAST_g3_P_LdpcOnLostLock_isr;
   hChn->onStableLockFunct = BAST_g3_P_LdpcOnStableLock_isr;
   hChn->onMonitorLockFunct = BAST_g3_P_LdpcMonitorLock_isr;
   hChn->enableLockInterrupts = BAST_g3_P_LdpcEnableLockInterrupts_isr;
   hChn->disableLockInterrupts = BAST_g3_P_LdpcDisableLockInterrupts_isr;
   hChn->getLockStatusFunct = NULL;
   return BERR_SUCCESS;
}

#endif
