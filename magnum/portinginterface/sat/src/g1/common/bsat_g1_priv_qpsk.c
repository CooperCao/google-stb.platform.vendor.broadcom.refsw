/******************************************************************************
* Copyright (C) 2018 Broadcom.
* The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
*
* This program is the proprietary software of Broadcom and/or its licensors,
* and may only be used, duplicated, modified or distributed pursuant to
* the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied),
* right to use, or waiver of any kind with respect to the Software, and
* Broadcom expressly reserves all rights in and to the Software and all
* intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
* THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
* IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1.     This program, including its structure, sequence and organization,
* constitutes the valuable trade secrets of Broadcom, and you shall use all
* reasonable efforts to protect the confidentiality thereof, and to use this
* information only in connection with your use of Broadcom integrated circuit
* products.
*
* 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
* "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
* OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
* RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
* IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
* A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
* ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
* THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
* OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
* INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
* RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
* HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
* EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
* WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
* FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*
* Module Description:
*
*****************************************************************************/
#include "bstd.h"
#include "bmth.h"
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"


BDBG_MODULE(bsat_g1_priv_qpsk);

#define BSAT_DEBUG_QPSK(x) /* x */

/* bit definitions for dtvScanState */
#define BSAT_G1_DTV_SCAN_ON  0x80
#define BSAT_G1_DTV_SCAN_1_2 0x01
#define BSAT_G1_DTV_SCAN_OFF 0x00

#define BSAT_QPSK_CHECK_LOCK_INTERVAL 20000


/* local functions */
static BERR_Code BSAT_g1_P_QpskAcquire1_isr(BSAT_ChannelHandle h);
static BERR_Code BSAT_g1_P_QpskSetOpll_isr(BSAT_ChannelHandle h);


/******************************************************************************
 BSAT_g1_P_QpskEnableLockInterrupts_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_QpskEnableLockInterrupts_isr(BSAT_ChannelHandle h, bool bEnable)
{
   BERR_Code retCode;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (bEnable)
   {
      BSAT_CHK_RETCODE(BINT_EnableCallback_isr(hChn->hQpskLockCb));
      BSAT_CHK_RETCODE(BINT_EnableCallback_isr(hChn->hQpskNotLockCb));
   }
   else
   {
      BSAT_CHK_RETCODE(BINT_DisableCallback_isr(hChn->hQpskLockCb));
      BSAT_CHK_RETCODE(BINT_DisableCallback_isr(hChn->hQpskNotLockCb));
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_QpskInitializeLoopParameters_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskInitializeLoopParameters_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t carrier_acq1_bw, carrier_acq2_bw, carrier_acq3_bw, carrier_trk_bw, baud_acq_bw, baud_trk_bw;
   uint32_t Fb, P_hi, P_lo, Q_hi;
   uint8_t idx = 0;

   /* generated 10-18-2010 10:29:22 */
   /* BW scale = 2097152 (BW is 2 bytes), damp scale = 4 (damp is 1 byte) */
   static const uint16_t qpsk_bw[7][6] =
   {
      /* carrierAcq1, carrierAcq2, carrierAcq3, carrierTrk, baudAcq, baudTrk */
      {0x51EC, 0x051F, 0x03B0, 0x03B0, 0x49BA, 0x0EBF},  /* 0 <= Fb < 2000000 */
      {0x71EC, 0x0C3D, 0x0819, 0x0619, 0x99BA, 0x1A31},  /* 2000000 <= Fb < 4000000 */
      {0x71EC, 0x083D, 0x0219, 0x0119, 0x49BA, 0x0831},  /* 4000000 <= Fb < 12000000 */
      {0x71EC, 0x083D, 0x080C, 0x0619, 0x49BA, 0x0831},  /* 12000000 <= Fb < 17000000 */
      {0x71EC, 0x0C3D, 0x0C3D, 0x0C3D, 0x49BA, 0x0831},  /* 17000000 <= Fb < 23000000 */
      {0x71EC, 0x0C3D, 0x0C3D, 0x0C3D, 0x49BA, 0x0831},  /* 23000000 <= Fb < 27000000 */
      {0x71EC, 0x0C3D, 0x0C3D, 0x0C3D, 0x49BA, 0x0831}   /* 27000000 <= Fb */
   };

   static const uint8_t qpsk_damp[7][6] =
   {
      /* carrierAcq1, carrierAcq2, carrierAcq3, carrierTrk, baudAcq, baudTrk */
      {0x0C, 0x08, 0x18, 0x18, 0x0C, 0x0C},  /* 0 <= Fb < 2000000 */
      {0x0C, 0x08, 0x18, 0x18, 0x08, 0x08},  /* 2000000 <= Fb < 4000000 */
      {0x0C, 0x08, 0x18, 0x18, 0x04, 0x04},  /* 4000000 <= Fb < 12000000 */
      {0x0C, 0x08, 0x18, 0x18, 0x04, 0x04},  /* 12000000 <= Fb < 17000000 */
      {0x0C, 0x08, 0x08, 0x08, 0x0C, 0x0C},  /* 17000000 <= Fb < 23000000 */
      {0x0C, 0x08, 0x18, 0x18, 0x08, 0x04},  /* 23000000 <= Fb < 27000000 */
      {0x0C, 0x08, 0x18, 0x18, 0x04, 0x04}   /* 27000000 <= Fb */
   };

   /* determine symbol rate range */
   Fb = hChn->acqSettings.symbolRate;
   if (Fb >= 27000000)
      idx = 6;
   else if (Fb >= 23000000)
      idx = 5;
   else if (Fb >= 17000000)
      idx = 4;
   else if (Fb >= 12000000)
      idx = 3;
   else if (Fb >= 4000000)
      idx = 2;
   else if (Fb >= 2000000)
      idx = 1;
   else
      idx = 0;

   /* initialize baud/carrier loop parameters based on symbol rate */
   carrier_acq1_bw = qpsk_bw[idx][0];
   carrier_acq2_bw = qpsk_bw[idx][1];
   carrier_acq3_bw = qpsk_bw[idx][2];
   carrier_trk_bw = qpsk_bw[idx][3];
   baud_acq_bw = qpsk_bw[idx][4];
   baud_trk_bw = qpsk_bw[idx][5];

   /* bw scaled 2^21 = 2097152 */
   BMTH_HILO_32TO64_Mul(carrier_acq1_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &carrier_acq1_bw);
   BMTH_HILO_32TO64_Mul(carrier_acq2_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &carrier_acq2_bw);
   BMTH_HILO_32TO64_Mul(carrier_acq3_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &carrier_acq3_bw);
   BMTH_HILO_32TO64_Mul(carrier_trk_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &carrier_trk_bw);
   BMTH_HILO_32TO64_Mul(baud_acq_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &baud_acq_bw);
   BMTH_HILO_32TO64_Mul(baud_trk_bw, Fb, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, 2097152, &Q_hi, &baud_trk_bw);

   hChn->carrierAcq1Bw = carrier_acq1_bw;
   hChn->carrierAcq2Bw = carrier_acq2_bw;
   hChn->carrierAcq3Bw = carrier_acq3_bw;
   hChn->carrierTrkBw = carrier_trk_bw;
   hChn->baudAcqBw = baud_acq_bw;
   hChn->baudTrkBw = baud_trk_bw;

   hChn->carrierAcq1Damp = qpsk_damp[idx][0];
   hChn->carrierAcq2Damp = qpsk_damp[idx][1];
   hChn->carrierAcq3Damp = qpsk_damp[idx][2];
   hChn->carrierTrkDamp = qpsk_damp[idx][3];
   hChn->baudAcqDamp = qpsk_damp[idx][4];
   hChn->baudTrkDamp = qpsk_damp[idx][5];

#if 0
   BDBG_MSG(("carrierAcq1Bw=%d, carrierAcq1Damp=%d", hChn->carrierAcq1Bw, hChn->carrierAcq1Damp));
   BDBG_MSG(("carrierAcq2Bw=%d, carrierAcq2Damp=%d", hChn->carrierAcq2Bw, hChn->carrierAcq2Damp));
   BDBG_MSG(("carrierAcq3Bw=%d, carrierAcq3Damp=%d", hChn->carrierAcq3Bw, hChn->carrierAcq3Damp));
   BDBG_MSG(("carrierTrkBw=%d, carrierTrkDamp=%d", hChn->carrierTrkBw, hChn->carrierTrkDamp));
   BDBG_MSG(("baudAcqBw=%d, baudAcqDamp=%d", hChn->baudAcqBw, hChn->baudAcqDamp));
   BDBG_MSG(("baudTrkBw=%d, baudTrkDamp=%d", hChn->baudTrkBw, hChn->baudTrkDamp));
#endif
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_QpskSetMode_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskSetMode_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t vtctl, fmod;

   static const uint8_t vtctl_init[0x13] =
   {
      0xF0, /* BSAT_Mode_eDvb_scan */
      0xE0, /* BSAT_Mode_eDvb_1_2 */
      0xE1, /* BSAT_Mode_eDvb_2_3 */
      0xE2, /* BSAT_Mode_eDvb_3_4 */
      0xE3, /* BSAT_Mode_eDvb_5_6 */
      0xE5, /* BSAT_Mode_eDvb_7_8 */
      0xF1, /* BSAT_Mode_eDss_scan */
      0xE0, /* BSAT_Mode_eDss_1_2 */
      0xE1, /* BSAT_Mode_eDss_2_3 */
      0xE4, /* BSAT_Mode_eDss_6_7 */
      0x12, /* BSAT_Mode_eDcii_scan */
      0x09, /* BSAT_Mode_eDcii_1_2 */
      0x0B, /* BSAT_Mode_eDcii_2_3 */
      0x0C, /* BSAT_Mode_eDcii_3_4 */
      0x0E, /* BSAT_Mode_eDcii_5_6 */
      0x0F, /* BSAT_Mode_eDcii_7_8 */
      0x08, /* BSAT_Mode_eDcii_5_11 */
      0x0A, /* BSAT_Mode_eDcii_3_5 */
      0x0D, /* BSAT_Mode_eDcii_4_5 */
   };

   if (BSAT_MODE_IS_DCII(hChn->acqSettings.mode))
   {
      /* DCII */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_V76, 0x0B000A00);   /* 7/8 and 5/6 */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_V54, 0x08000700);   /* 4/5 and 3/4 */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_V32, 0x04000500);   /* 2/3 and 3/5 */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_V10, 0x04500400);   /* 1/2 and 5/11 */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_VINT, 0x25800000);  /* integration period */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FRS, 0x0a0BFFFE);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FSYN, 0x02000805);
   }
   else if (BSAT_MODE_IS_LEGACY_QPSK(hChn->acqSettings.mode))
   {
      /* DVB or DTV */
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_V76, 0x00E600E6);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_V54, 0x09000880);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_V32, 0x08660780);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_V10, 0x06000480);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_VINT, 0x27E70000);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FRS, 0x020B2523);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FSYN, 0x02000805);
   }
   else
   {
      /* non-legacy-qpsk mode */
      return BERR_INVALID_PARAMETER;
   }

   vtctl = (uint32_t)vtctl_init[hChn->acqSettings.mode];
   if (BSAT_MODE_IS_DCII(hChn->acqSettings.mode))
   {
      /* DCII */
      if (hChn->acqSettings.options & BSAT_ACQ_DCII_SPLIT)
      {
         vtctl |= 0x0400;
         if (hChn->acqSettings.options & BSAT_ACQ_DCII_SPLIT_Q)
         {
            vtctl |= 0xA0; /* split Q */
            vtctl &= ~0x40;
         }
         else
         {
            vtctl |= 0x80; /* split I */
            vtctl &= ~0x60;
         }
      }
      else
         vtctl |= 0xE0; /*combine */
   }

   if ((hChn->acqSettings.mode < BSAT_Mode_eDcii_scan) ||
       ((hChn->acqSettings.mode >= BSAT_Mode_eDcii_scan) && ((hChn->acqSettings.options & BSAT_ACQ_DCII_SPLIT) == 0)))
   {
      vtctl &= ~0x60;
      if (hChn->acqSettings.options & BSAT_ACQ_DISABLE_SPINV_SCAN)
      {
         if (hChn->acqSettings.options & BSAT_ACQ_SPINV)
            vtctl |= 0x20; /* or 0x40 */
      }
      else
         vtctl |= 0x60; /* spinv scan */
   }

   if (hChn->acqSettings.options & BSAT_ACQ_OQPSK)
   {
      vtctl |= 0x0200;
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, 0x00020000);
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, 0x00000010);
   }

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, vtctl);

   /* reset viterbi decoder */
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, 0x00004000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, 0x19);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, ~0x00004000);

   /* set FMOD */
   fmod = hChn->qpskSettings.stuffBytes & 0x1F;
   if (BSAT_MODE_IS_DVBS(hChn->acqSettings.mode))
   {
      /* DVB */
      fmod |= 0x5200;
   }
   else if (BSAT_MODE_IS_DTV(hChn->acqSettings.mode))
   {
      /* DTV */
      fmod |= 0x5600;
   }
   else
   {
      /* DCII */
      if (hChn->acqSettings.options & BSAT_ACQ_DCII_SPLIT)
      {
         if (hChn->acqSettings.options & BSAT_ACQ_DCII_SPLIT_Q)
            fmod |= 0x0A00;   /* split Q */
         else
            fmod |= 0x4A00;   /* split I */
      }
      else
         fmod |= 0x5A00; /* combine */
   }
   fmod |= ((hChn->xportSettings.bTei) ? 0x0200 : 0x0000);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FMOD, fmod);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_QpskSetActualMode_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskSetActualMode_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i, j, val, vst_save;
   BERR_Code retCode;

   /* check initial code rate */
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST);
   vst_save = (val >> 24) & 0x0F;

   if ((hChn->acqSettings.mode == BSAT_Mode_eDss_scan) ||
       (hChn->acqSettings.mode == BSAT_Mode_eDvbs_scan) ||
       (hChn->acqSettings.mode == BSAT_Mode_eDcii_scan))
   {
      /* check for stable code rate in scan mode */
      for (i = 10; i > 0; i--)
      {
         for (j = 50; j > 0; j--)
         {
            val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST);
            val = (val >> 24) & 0x0F;
            if (val != vst_save)
               break;
         }
         vst_save = val;
         if (j == 0)
         {
            /* VST is stable */
            break;
         }
      }
   }

   BSAT_CHK_RETCODE(BSAT_g1_P_GetActualMode_isr(h, &(hChn->actualMode)));
   if (hChn->actualMode == BSAT_Mode_eUnknown)
      BERR_TRACE(retCode = BERR_UNKNOWN);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_QpskSetVcos_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskSetVcos_isr(BSAT_ChannelHandle h)
{
   uint32_t vst, val = 0;

   static const uint8_t vcos_byte1[16] =
   {
      0x40, 0x48, 0x50, 0x60,
      0x68, 0x78, 0x40, 0x40,
      0x40, 0x40, 0x48, 0x48,
      0x50, 0x50, 0x60, 0x78
   };

   /* read code rate */
   vst = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST);
   vst = (vst >> 24) & 0x0F;

   /* look up qpsk hard decision level for VLC slicer */
   val = (vcos_byte1[vst] << 8);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VCOS, val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_QpskNarrowBw1_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskNarrowBw1_isr(BSAT_ChannelHandle h)
{
   BERR_Code retCode;
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t cdelta, bw, damp;

   if (hChn->count1 <= 4)
   {
      cdelta = (hChn->carrierAcq2Bw - hChn->carrierAcq3Bw) >> 2;
      bw = hChn->carrierAcq2Bw - hChn->count1 * cdelta;
      cdelta = (hChn->carrierAcq3Damp - hChn->carrierAcq2Damp) >> 2;
      damp = hChn->carrierAcq2Damp + hChn->count1 * cdelta;
      BSAT_g1_P_SetCarrierBw_isr(h, bw, damp);

      hChn->count1++;
      return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, 1000, BSAT_g1_P_QpskNarrowBw1_isr);
   }

   BSAT_g1_P_SetCarrierBw_isr(h, hChn->carrierAcq3Bw, hChn->carrierAcq3Damp);
   retCode = BSAT_g1_P_SetBaudBw_isr(h, hChn->baudTrkBw, hChn->baudTrkDamp);
   if (retCode != BERR_SUCCESS)
   {
      BDBG_WRN(("BSAT_g1_P_SetBaudBw_isr() error 0x%X", retCode));
      return retCode;
   }

   return BSAT_g1_P_QpskAcquire1_isr(h);
}


/******************************************************************************
 BSAT_g1_P_QpskNarrowBw_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskNarrowBw_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   /* set thresholds for bad headers */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FSYN, 0x04003020);

   hChn->count1 = 1;
   return BSAT_g1_P_QpskNarrowBw1_isr(h);
}


/******************************************************************************
 BSAT_g1_P_QpskSpinvScan1_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskSpinvScan1_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, sts;

   while (1)
   {
      switch (hChn->functState)
      {
         case 0:
            BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, ~0x60, (hChn->count1 % 3) << 5);
            hChn->functState = 1;
            return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, 30000, BSAT_g1_P_QpskSpinvScan1_isr);

         case 1:
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, 0x7F);
            hChn->functState = 2;
            return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, 30000, BSAT_g1_P_QpskSpinvScan1_isr);

         case 2:
            val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_INTR_RAW_STS0);
            sts = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_STATUS);
            sts &= 0x1B;
            val &= 0x1B;
            if (val == 0x09)
            {
               if ((sts == 0) || (sts == 0x09))
               {
/* BDBG_MSG(("spinv scan success")); */
                  goto done;  /* IQ scan successful */
               }
               else
                  goto not_locked;
            }
            else
            {
               not_locked:
/* BDBG_MSG(("spinv scan: RS/Vit not locked")); */
               hChn->count1++;
               if (hChn->count1 < 9)
                  hChn->functState = 0;
               else
               {
                  BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, ~0x60);
                  goto done;
               }
            }
            break;

         default:
            BDBG_ERR(("invalid state"));
            return BSAT_ERR_INVALID_STATE;
      }
   }

   done:
   return BSAT_g1_P_QpskNarrowBw_isr(h);
}


/******************************************************************************
 BSAT_g1_P_QpskSpInvScan_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskSpInvScan_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (((hChn->actualMode != BSAT_Mode_eDvbs_5_6) && (hChn->actualMode != BSAT_Mode_eDss_6_7)) || (hChn->acqSettings.options & BSAT_ACQ_DISABLE_SPINV_SCAN))
      return BSAT_g1_P_QpskNarrowBw_isr(h);

   /* do spinv scan */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_FEC_FSYN, ~0xFFFF, 0x0802);

   hChn->functState = 0;
   hChn->count1 = 0;
   return BSAT_g1_P_QpskSpinvScan1_isr(h);
}


/******************************************************************************
 BSAT_g1_P_QpskLockViterbi_isr() - this routine is formerly verify_lock
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskLockViterbi_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, bw, irq, raw;
   BERR_Code retCode;

   while (1)
   {
      switch (hChn->functState)
      {
         case 0:
            /* reset the equalizer */
            BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 1);
            hChn->carrierDelay = (hChn->acqSettings.mode == BSAT_Mode_eDcii_scan) ? 900000 : 700000;

            /* extend delay for small symbol rates, shorten delay for higher symbol rates */
            if (hChn->acqSettings.symbolRate < 5000000)
               hChn->carrierDelay = (hChn->carrierDelay * 3) >> 1;

            /* set baud loop bw */
#if 0 /* this bw is way too high */
            BSAT_g1_P_SetBaudBw_isr(h, hChn->baudAcqBw, hChn->baudAcqDamp);
#endif
            hChn->functState = 1;
            break;

         case 1:
            /* set carrier loop bw */
            BSAT_g1_P_SetCarrierBw_isr(h, hChn->carrierAcq1Bw, hChn->carrierAcq1Damp);

            /* initialize counters */
            hChn->count1 = 0;
            hChn->maxCount1 = hChn->carrierDelay / BSAT_QPSK_CHECK_LOCK_INTERVAL;
            if (hChn->maxCount1 == 0)
               hChn->maxCount1 = 1;

            hChn->functState = 2;
            break;

         case 2:
            /* gradually narrow the carrier bw from carrierAcq1Bw to carrierAcq2Bw */
            bw = hChn->carrierAcq1Bw - ((hChn->count1 * (hChn->carrierAcq1Bw - hChn->carrierAcq2Bw)) / hChn->maxCount1);
            BSAT_g1_P_SetCarrierBw_isr(h, bw, hChn->carrierAcq1Damp);
            hChn->count1++;

            if (hChn->count1 < hChn->maxCount1)
               return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, BSAT_QPSK_CHECK_LOCK_INTERVAL, BSAT_g1_P_QpskLockViterbi_isr);

            /* narrow to tracking bw */
            BSAT_g1_P_SetCarrierBw_isr(h, hChn->carrierAcq2Bw, hChn->carrierAcq2Damp);

            hChn->functState = 3;
            break;

         case 3:
            /* calculate time to wait for viterbi lock */
            hChn->count1 = hChn->carrierDelay / BSAT_QPSK_CHECK_LOCK_INTERVAL;
            if (hChn->count1 < 1)
               hChn->count1 = 1;

            val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_INTR_RAW_STS0);
            hChn->count3 = (val & 0x08); /* count3=prev_state */

            /* clear sticky vit lock status bit */
            BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, 0x08);

            hChn->functState = 4;
            break;

         case 4:
            /* wait for viterbi lock */
            if ((hChn->count1 > 0) || (hChn->count3 == 0x08))
            {
               hChn->functState = 5;
               return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, BSAT_QPSK_CHECK_LOCK_INTERVAL, BSAT_g1_P_QpskLockViterbi_isr);
            }
            hChn->functState = 6; /* vit NOT locked after carrierDelay */
            break;

         case 5:
            /* make sure real time status is locked before checking sticky bits */
            raw = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_INTR_RAW_STS0) & 0x08;
            irq = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_STATUS) & 0x08;
            /* BDBG_MSG(("raw=0x%X, irq=0x%X, count1=%u", raw, irq, hChn->count1)); */

            if (raw == 0x08) /* BCHP_SDS_INTR2_0_CPU_STATUS_vit_in_sync_MASK */
            {
               BSAT_DEBUG_QPSK(BDBG_MSG(("vit raw sts is locked")));

               /* check sticky bit status for lock stability */
               if ((irq == 0) && (hChn->count3 == 0x08))
               {
                  hChn->bFlag1 = true; /* bFlag1=viterbi lock indication */
                  hChn->functState = 7;  /* vit locked */
                  break;
               }
               hChn->count3 = 0x08;
            }
            else
               hChn->count3 = 0; /* count3=prev_state */
            if (irq != 0)
               BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, irq);

            if (hChn->count1 > 0)
               hChn->count1--;
            hChn->functState = 4;  /* wait again */
            break;

         case 6:
            /* viterbi NOT locked */
            hChn->bFlag1 = false; /* bFlag1=viterbi lock indication */

            /* manually handle dtv scan */
            if (hChn->dtvScanState & BSAT_G1_DTV_SCAN_ON)
            {
               hChn->dtvScanState ^= BSAT_G1_DTV_SCAN_1_2;

               if (hChn->dtvScanState & BSAT_G1_DTV_SCAN_1_2)
               {
                  /* trying dtv 1/2 */
                  BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, ~0x0000001F, 0x00004000);
                  BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL, ~0x00004000);  /* deassert vit soft rst */

                  hChn->functState = 1;
                  break;
               }
            }
            hChn->functState = 7;
            break;

         case 7:
            /* reacquire if unable to lock viterbi */
            if (hChn->bFlag1 == false)
            {
               BDBG_MSG(("Vit could not lock"));
               hChn->reacqCause = BSAT_ReacqCause_eVitNotLock;
               return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
            }

            BDBG_MSG(("vit locked"));
            BSAT_CHK_RETCODE(BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eViterbiLocked));

            /* determine stabilized code rate */
            if (BSAT_g1_P_QpskSetActualMode_isr(h) != BERR_SUCCESS)
            {
               BDBG_MSG(("unable to set actual mode"));
               hChn->reacqCause = BSAT_ReacqCause_eUnableToDetermineActualMode;
               return BSAT_g1_P_GetModeFunct_isrsafe(hChn->acqType)->Reacquire(h);
            }
            BSAT_CHK_RETCODE(BSAT_g1_P_QpskSetVcos_isr(h));
            return BSAT_g1_P_QpskSpInvScan_isr(h);

         default:
            BDBG_ERR(("invalid state"));
            BERR_TRACE(retCode = BSAT_ERR_INVALID_STATE);
            break;
      }
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_QpskIsSpinv_isrsafe()
******************************************************************************/
bool BSAT_g1_P_QpskIsSpinv_isrsafe(BSAT_ChannelHandle h)
{
   uint32_t val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST) & 0x30000000;
   return val ? true : false;
}


/******************************************************************************
 BSAT_g1_P_QpskIsValidMode_isr()
******************************************************************************/
bool BSAT_g1_P_QpskIsValidMode_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_Mode acq_mode = hChn->acqSettings.mode;
   uint8_t base, idx, enable_mask, mode_mask;

   if ((acq_mode == BSAT_Mode_eDss_scan) || (acq_mode == BSAT_Mode_eDvbs_scan) ||
       (acq_mode == BSAT_Mode_eDcii_scan))
   {
      if (BSAT_MODE_IS_DVBS(hChn->actualMode))
      {
         base = BSAT_Mode_eDvbs_1_2;
         enable_mask = hChn->qpskSettings.dvbsScanModes;
      }
      else if (BSAT_MODE_IS_DTV(hChn->actualMode))
      {
         base = BSAT_Mode_eDss_1_2;
         enable_mask = hChn->qpskSettings.dtvScanModes;
      }
      else
      {
         base = BSAT_Mode_eDcii_1_2;
         enable_mask = hChn->qpskSettings.dciiScanModes;
      }
      idx = hChn->actualMode - base;
      mode_mask = 1 << idx;
      if ((mode_mask & enable_mask) == 0)
         return false;
   }

   return true;
}


/******************************************************************************
 BSAT_g1_P_QpskIsLocked_isr()
******************************************************************************/
bool BSAT_g1_P_QpskIsLocked_isr(BSAT_ChannelHandle h)
{
   uint32_t val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_INTR_RAW_STS0) & 0x60;
   return (val == 0x20) ? true : false; /* test rvb_in_lock */
}


/******************************************************************************
 BSAT_g1_P_QpskDisableFreqDetector_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskDisableFreqDetector_isr(BSAT_ChannelHandle h)
{
   BSAT_DEBUG_QPSK(BDBG_MSG(("disabling freq detector")));
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~0x00400000);
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_QpskOnLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_QpskOnLock_isr(BSAT_ChannelHandle h)
{
   /* disable freq detector 30 msecs after lock */
   BSAT_DEBUG_QPSK(BDBG_MSG(("starting 30 msec timer for freq detector disable")));
   return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaudUsec, 30000, BSAT_g1_P_QpskDisableFreqDetector_isr);
}


/******************************************************************************
 BSAT_g1_P_QpskOnLostLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_QpskOnLostLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eBaudUsec);

   /* re-enable frequency detector */
   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0x00400000);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_QpskOnStableLock_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_QpskOnStableLock_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BSAT_Mode prev_mode = hChn->actualMode;
   BSAT_Mode curr_mode;
   BERR_Code retCode;

   if (hChn->bPlcTracking == false)
      BSAT_g1_P_ConfigPlc_isr(h, false); /* set tracking PLC */

   BSAT_g1_P_QpskDisableFreqDetector_isr(h);

   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 0x00FFFFFF, 0x69000000); /* set mu for tracking */

   BSAT_CHK_RETCODE(BSAT_g1_P_GetActualMode_isr(h, &curr_mode));
   if (prev_mode != curr_mode)
   {
      /* different code rate now detected */
      BDBG_MSG(("diff code rate detected"));
      hChn->bForceReacq = true;
      hChn->reacqCause = BSAT_ReacqCause_eCodeRateChanged;
   }
   else if (hChn->bEverStableLock == false)
   {
      BSAT_g1_P_QpskUpdateErrorCount_isrsafe(h);
      hChn->qpskStatus.rsCorrCount = 0;
      hChn->qpskStatus.rsUncorrCount = 0;
      hChn->qpskStatus.preVitErrCount = 0;
   }

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_QpskUpdateErrorCount_isrsafe() - updates qpskStatus
******************************************************************************/
BERR_Code BSAT_g1_P_QpskUpdateErrorCount_isrsafe(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

#ifdef BCHP_SDS_FEC_FERR
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_FEC_FERR);
   hChn->qpskStatus.rsCorrCount += ((val >> 16) & 0xFFFF);
   hChn->qpskStatus.rsUncorrCount += (val & 0xFFFF);
#else
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_FEC_FECTL, BCHP_SDS_FEC_0_FECTL_rs_cnt_snap_MASK);
   hChn->qpskStatus.rsCorrCount += BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_FEC_CSYM);
   hChn->qpskStatus.rsUncorrCount += BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_FEC_BBLK);
#endif

   /* get pre-Viterbi error count */
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_VIT_VTCTL, BCHP_SDS_VIT_0_VTCTL_vit_rec_cnt_snap_MASK); /* take snapshot of VRCV+VREC */
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VREC);
   hChn->qpskStatus.preVitErrCount += val;
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_QpskSetOqpsk_isr() - from qpsk_set_clctl_3() in 4506
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskSetOqpsk_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1);
   if (hChn->acqSettings.options & BSAT_ACQ_OQPSK)
      val = (val & 0xFFFFFF70) | 0x0C;
   else
      val = (val & 0xFFFFFF30) | 0x0C;
   val |= 0x0C080000;
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_QpskSetFinalFlBw_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskSetFinalFlBw_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t vst, fllc, flic;

   vst = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST);
   vst = (vst >> 24) & 0x0F;

   if (BSAT_MODE_IS_DVBS(hChn->acqSettings.mode))
   {
      /* DVB */
      if (vst <= 1)
      {
         /* code rate 1/2 or 2/3 */
         fllc = 0x01000100;
         flic = 0x01000000;
      }
      else
      {
         fllc = 0x01000100;
         flic = 0x01000000;
      }
   }
   else if (BSAT_MODE_IS_DTV(hChn->acqSettings.mode))
   {
      /* DTV */
      if (vst <= 1)
      {
         /* code rate 1/2 or 2/3 */
         fllc = 0x02880300;
         flic = 0x029B0200;
      }
      else
      {
         fllc = 0x04F20200;
         flic = 0x01B20200;
      }
   }
   else
   {
      /* DCII */
      fllc = 0x02000200;
      flic = 0x01000300;
   }
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLLC, fllc);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLIC, flic);

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_QpskAcquire_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_QpskAcquire_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   if (hChn->acqSettings.mode == BSAT_Mode_eDss_scan)
      hChn->dtvScanState = BSAT_G1_DTV_SCAN_ON;
   else
      hChn->dtvScanState = BSAT_G1_DTV_SCAN_OFF;

   /* set qpsk mode and init bert */
   BSAT_g1_P_QpskSetMode_isr(h);
   BSAT_g1_P_InitBert_isr(h);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEM1, 0x01546732);

   BSAT_g1_P_QpskInitializeLoopParameters_isr(h);
   BSAT_CHK_RETCODE(BSAT_g1_P_LogTraceBuffer_isr(h, BSAT_TraceEvent_eStartViterbi));

   if (hChn->acqSettings.options & BSAT_ACQ_OQPSK)
   {
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0x0E08D25C);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, 0x00006003);
   }

   hChn->functState = 0;
   return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, 20000, BSAT_g1_P_QpskLockViterbi_isr);

   done:
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_QpskAcquire1_isr() - ISR context
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskAcquire1_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val;
#ifdef BSAT_HAS_CONTEXT_SWITCH
   int32_t pli, fli;
   uint32_t P_hi, P_lo;
#endif

   /* set agc tracking bw */
   BSAT_g1_P_SetAgcTrackingBw_isr(h);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_FEC_FSYN, 0x04003020);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_EQMISCCTL, ~0x0000FF00);  /* EQBLND=0 */
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLTD, 0x16000000);

   BSAT_g1_P_ConfigPlc_isr(h, false); /* set tracking PLC */

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_VLCTL, 0x00040704);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLTD, 0x28000000);

   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, ~0x000000EF, 0x000000A1);  /* CLMISC=0xA1 but retain bit 4 */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~0x0CFFFF00, 0x08481000);
   BSAT_g1_P_QpskSetOqpsk_isr(h);

#ifdef BSAT_HAS_CONTEXT_SWITCH
   /* enable the front carrier loop */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, ~0x00801000, BCHP_SDS_CL_0_CLCTL1_clen_MASK); /* fenfd =0, flfsel=0, clen=1 */

   /* move back carrier loop's integrator to front carrier loop's integrator */
   pli = (int32_t)(BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_CL_PLI));
   if (pli < 0)
      val = -pli;
   else
      val = pli;

   BMTH_HILO_32TO64_Mul(val, hChn->acqSettings.symbolRate, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->sampleFreq, &P_hi, &P_lo);

   if (pli >= 0)
      fli = (int32_t)P_lo;
   else
      fli = (int32_t)-P_lo;

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_FLI, fli);

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLI, 0x00000000);
#endif /* BSAT_HAS_CONTEXT_SWITCH */

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_PLI, 0x00000000); /* clear back carrier loop's integrator */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, ~0x0000FF00, 0x00007100);  /*CLMISC2=0x71 */

   if (hChn->acqSettings.options & BSAT_ACQ_OQPSK)
   {
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL1, 0x0E08D25C);
      BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_CL_CLCTL2, 0x00006003);
   }

   BSAT_g1_P_QpskSetFinalFlBw_isr(h);

#if 0
    /* BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 0x001F0000, 0x22000271);  per Xiaofen */
    BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, 0x22060271);
    BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, ~0x00000001);
#else
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_update_rate, 3);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_reset, 1);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, val);
   BCHP_SET_FIELD_DATA(val, SDS_EQ_0_EQFFECTL, ffe_reset, 0);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_EQ_EQFFECTL, val);
#endif

   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNRCTL, 0x03);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, 0x8A);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, 0x0A);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_SNR_SNORECTL, 0x8A);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_BERT_BEIT, 0x00FF005F);

   /* configure output pll */
   return BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eBaud, 1000, BSAT_g1_P_QpskSetOpll_isr);
}


/******************************************************************************
 BSAT_g1_P_QpskSetOpll_isr()
******************************************************************************/
static BERR_Code BSAT_g1_P_QpskSetOpll_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t val, fmod, rs_mode, vst, data0, rs_info_symbols, rs_symbols, data3, numerator, denominator;
   uint32_t n, d, gcf, P_hi, P_lo, Q_hi, Q_lo;

   static const uint8_t qpsk_opll_numerator[16] =
   {
      0x01, 0x02, 0x03, 0x05,
      0x06, 0x07, 0x01, 0x01,
      0x05, 0x01, 0x03, 0x02,
      0x03, 0x04, 0x05, 0x07
   };

   static const uint8_t qpsk_opll_denominator[16] =
   {
      0x02, 0x03, 0x04, 0x06,
      0x07, 0x08, 0x01, 0x01,
      0x0B, 0x02, 0x05, 0x03,
      0x04, 0x05, 0x06, 0x08
   };

   if (hChn->xportSettings.bOpllBypass == false)
      BSAT_g1_P_PowerUpOpll_isr(h);

   /* reset output interface */
   BSAT_g1_P_ToggleBit_isrsafe(h, BCHP_SDS_OI_OIFCTL00, 0x00000001);

   fmod = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_FEC_FMOD);
   rs_mode = (fmod & 0x0C00);

   if ((rs_mode == 0x0800) || (rs_mode == 0x0000))
   {
      /* DVB and DCII */
      rs_info_symbols = 188; /* RS info symbols */
      rs_symbols = 204; /* RS symbols */
      data3 = 188;
   }
   else
   {
      /* DTV */
      rs_info_symbols = 130; /* RS info symbols */
      rs_symbols = 147; /* RS symbols */
      data3 = 147;
   }

   /* read code rate */
   vst = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VST);
   vst = (vst >> 24) & 0x0F;
   numerator = qpsk_opll_numerator[vst];
   denominator = qpsk_opll_denominator[vst];

   n = rs_info_symbols * 2 * numerator;
   d = rs_symbols * denominator;
   d = d << (hChn->bUndersample ? 1 : 2);

   /* are we in split mode? */
   val = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_VIT_VTCTL);
   if (val & 0x0400)
      d = d << 1;

   gcf = BSAT_g1_P_GCF_isrsafe(n, d);
   n /= gcf;
   d /= gcf;

   BMTH_HILO_32TO64_Mul(hChn->acqSettings.symbolRate, n * (hChn->bUndersample ? 2 : 4), &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, d, &Q_hi, &Q_lo);

   if (hChn->xportSettings.bDelHeader)
   {
      if (hChn->xportSettings.bHead4)
         data0 = 4;
      else
         data0 = 1;
      BMTH_HILO_32TO64_Mul(Q_lo, data3 - data0, &P_hi, &P_lo);
      BMTH_HILO_64TO64_Div32(P_hi, P_lo, data3, &Q_hi, &Q_lo);
   }
   hChn->outputBitrate = Q_lo;

   /* config output interface */
   hChn->opll_N = n;
   hChn->opll_D = d;
   BSAT_g1_P_ConfigOif_isr(h);

   if (hChn->qpskSettings.bRsDisable)
      BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_FEC_FMOD, 0x00000100);
   else
      BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_FEC_FMOD, ~0x00000100);

   BSAT_g1_P_OrRegister_isrsafe(h, BCHP_SDS_FEC_FECTL, 0x00000001);
   BSAT_g1_P_WriteRegister_isrsafe(h, BCHP_SDS_INTR2_0_CPU_CLEAR, 0x7F);
   BSAT_g1_P_AndRegister_isrsafe(h, BCHP_SDS_FEC_FECTL, ~0x00000001);

   /* start tracking */
   return BSAT_g1_P_StartTracking_isr(h);
}
