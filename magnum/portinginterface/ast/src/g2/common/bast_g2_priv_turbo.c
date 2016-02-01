/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#include "bstd.h"
#include "bmth.h"
#include "bast.h"
#include "bast_priv.h"
#include "bast_g2_priv.h"

BDBG_MODULE(bast_g2_priv_turbo);

#define BAST_TURBO_INIT_LOCK_FILTER_TIME 1000


#ifndef BAST_EXCLUDE_TURBO
/* local functions */
bool BAST_g2_P_TurboScanDetermineNextMode_isr(BAST_ChannelHandle h);
BERR_Code BAST_g2_P_TurboCheckHP_isr(BAST_ChannelHandle h);

/* external functions */
extern BERR_Code BAST_g2_P_LdpcAcquire1_isr(BAST_ChannelHandle h);


static const uint32_t BAST_turbo_opll[10] =
{
   0x000924d2, /* turbo qpsk 1/2 */
   0x00011c92, /* turbo qpsk 2/3 */
   0x000dbbd2, /* turbo qpsk 3/4 */
   0x000f468c, /* turbo qpsk 5/6 */
   0x00100752, /* turbo qpsk 7/8 */
   0x0003ac00, /* turbo 8psk 2/3 */
   0x0003ee18, /* turbo 8psk 3/4 */
   0x000409a2, /* turbo 8psk 4/5 */
   0x00046744, /* turbo 8psk 5/6 */
   0x00049700, /* turbo 8psk 8/9 */
};


static const uint32_t BAST_turbo_opll2[10] =
{
   0x001d688b, /* turbo qpsk 1/2 */
   0x000264a5, /* turbo qpsk 2/3 */
   0x0018d18b, /* turbo qpsk 3/4 */
   0x001746d1, /* turbo qpsk 5/6 */
   0x0016860b, /* turbo qpsk 7/8 */
   0x0003fdb5, /* turbo 8psk 2/3 */
   0x0003bb9d, /* turbo 8psk 3/4 */
   0x0003a013, /* turbo 8psk 4/5 */
   0x00034271, /* turbo 8psk 5/6 */
   0x000312b5, /* turbo 8psk 8/9 */
};


static const uint32_t BAST_turbo_tssq_qpsk_1_2[2] =
{
   1, 0xDD
};

static const uint32_t BAST_turbo_tssq_qpsk_2_3[4] =
{
   3, 0xDD, 0xDD, 0xEE
};

static const uint32_t BAST_turbo_tssq_qpsk_3_4[5] =
{
   4, 0xED, 0xED, 0xDE, 0xDE
};

static const uint32_t BAST_turbo_tssq_qpsk_5_6[7] =
{
   6, 0xED, 0xDE, 0xDE, 0xEE, 0xEE, 0xED
};

static const uint32_t BAST_turbo_tssq_qpsk_7_8[9] =
{
   8, 0xED, 0xED, 0xDE, 0xDE, 0xEE, 0xEE, 0xEE, 0xEE
};

static const uint32_t BAST_turbo_tssq_8psk_2_3[6] =
{
   5, 0x00, 0x00, 0x00, 0x00, 0x22
};

static const uint32_t BAST_turbo_tssq_8psk_3_4[8] =
{
   7, 0x00, 0x00, 0x20, 0x10, 0x00, 0x01, 0x02
};

static const uint32_t BAST_turbo_tssq_8psk_4_5[6] =
{
   5, 0x10, 0x00, 0x00, 0x01, 0x22
};

static const uint32_t BAST_turbo_tssq_8psk_5_6[6] =
{
   5, 0x10, 0x01, 0x10, 0x01, 0x22
};

static const uint32_t BAST_turbo_tssq_8psk_8_9[7] =
{
   6, 0x10, 0x01, 0x21, 0x10, 0x01, 0x12
};

static const uint32_t BAST_turbo_onfn[] =
{
   0x0000001F,  /* turbo qpsk 1/2 */
   0x0000001D,  /* turbo qpsk 2/3 */
   0x0000001C,  /* turbo qpsk 3/4 */
   0x0000001A,  /* turbo qpsk 5/6 */
   0x00000018,  /* turbo qpsk 7/8 */
   0x0000001B,  /* turbo 8psk 2/3 */
   0x00000019,  /* turbo 8psk 3/4 */
   0x0000001B,  /* turbo 8psk 4/5 */
   0x0000001B,  /* turbo 8psk 5/6 */
   0x0000001A,  /* turbo 8psk 8/9 */
};
#endif


/******************************************************************************
 BAST_g2_P_TurboSetOnfn_isr()
******************************************************************************/
void BAST_g2_P_TurboSetOnfn_isr(BAST_ChannelHandle h)
{
#ifndef BAST_EXCLUDE_TURBO
   BAST_g2_P_LdpcConfig_isr(h, BCHP_SDS_ONFN, BAST_turbo_onfn);
#endif
}


/******************************************************************************
 BAST_g2_P_TurboAcquire_isr()
******************************************************************************/
BERR_Code BAST_g2_P_TurboAcquire_isr(BAST_ChannelHandle h)
{
#ifndef BAST_EXCLUDE_TURBO
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;
   BERR_Code BAST_g2_P_LdpcAcquire3_isr(BAST_ChannelHandle h);
   uint32_t val;

   hChn->ldpcScanState = 0;
   if (hChn->acqParams.mode == BAST_Mode_eTurbo_scan)
   {
      if (BAST_g2_P_TurboScanDetermineNextMode_isr(h) == false)
      {
         if (hChn->blindScanStatus == BAST_BlindScanStatus_eAcquire)
            return BAST_g2_P_BlindReacquire_isr(h);
         else
            return false;
      }

      /* check if the next mode is same modulation as previous mode */
      if (hChn->turboScanState & BAST_TURBO_SCAN_HP_LOCKED)
      {
         BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &val);
         if (((val >> 24) & 0x1F) == 17)
         {
            /* HP is still locked */
            val = 0x00;
            BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TFEC_TTUR, &val);
            return BAST_g2_P_LdpcAcquire3_isr(h);
         }
      }
   }
   else
      hChn->actualMode = hChn->acqParams.mode;

   /* use pilot pll settings for turbo acquisition */
   hChn->acqParams.acq_ctl |= (BAST_ACQSETTINGS_LDPC_PILOT_PLL | BAST_ACQSETTINGS_LDPC_PILOT);

   if (hChn->firstTimeLock == 0)
      hChn->lockFilterTime = BAST_TURBO_INIT_LOCK_FILTER_TIME;

   hChn->funct_state = 0;
   retCode = BAST_g2_P_LdpcAcquire1_isr(h);

   return retCode;
#else
   return BERR_NOT_SUPPORTED;
#endif
}


#ifndef BAST_EXCLUDE_TURBO
/******************************************************************************
 BAST_g2_P_TurboMonitor_isr()
******************************************************************************/
BERR_Code BAST_g2_P_TurboMonitor_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   if (hChn->bExternalTuner == false)
      BAST_g2_P_3StageAgc_isr(h, 0);

   BAST_g2_P_TransferFreqOffset_isr(h);
   if (hChn->bForceReacq)
      return BAST_g2_P_LdpcReacquire_isr(h);

#ifndef BAST_EXCLUDE_STATUS_EVENTS
   BAST_g2_P_CheckStatusIndicators_isr(h);
#endif

   return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaudUsec, 100000, BAST_g2_P_TurboMonitor_isr);
}


/******************************************************************************
 BAST_g2_P_TurboLock_isr()
******************************************************************************/
void BAST_g2_P_TurboLock_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   uint32_t val;
   BSTD_UNUSED(param);

   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eTurboLock);
   BDBG_MSG(("Turbo locked ISR"));

   if (hChn->lockIsrFlag == 1)
   {
      BDBG_MSG(("ignoring lock isr"));
      return;
   }
   hChn->lockIsrFlag = 1;

   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eBaud);

   if (hChn->acqState == BAST_AcqState_eWaitForLock)
   {
      /* ldpc locked for the first time */
      hChn->acqState = BAST_AcqState_eMonitor;

      BAST_g2_P_LdpcSetPlc_isr(h, false);

      /* set final baud bw */
      BAST_g2_P_LdpcSetFinalBaudLoopBw_isr(h);

      val = 0x69;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_EQMU, &val);

      BAST_g2_P_ResyncBert_isr(h);
   }

   BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eGen1, hChn->lockFilterTime, BAST_g2_P_LdpcLockStable_isr);
   BAST_g2_P_TurboMonitor_isr(h);
}


/******************************************************************************
 BAST_g2_P_TurboNotLock_isr()
******************************************************************************/
void BAST_g2_P_TurboNotLock_isr(void *p, int param)
{
   BAST_ChannelHandle h = (BAST_ChannelHandle)p;
   BAST_g2_P_ChannelHandle *hChn = h->pImpl;
   uint32_t val;
   BSTD_UNUSED(param);

   BAST_g2_P_IncrementInterruptCounter_isr(h, BAST_Sds_IntID_eTurboNotLock);
   BAST_g2_P_LdpcUpdateBlockCounters_isr(h);

   if (hChn->lockIsrFlag == 0)
   {
      BDBG_MSG(("ignoring not lock isr"));
      return;
   }
   hChn->lockIsrFlag = 0;

   BDBG_MSG(("Turbo not locked ISR"));

   BAST_g2_P_DisableTimer_isr(h, BAST_TimerSelect_eGen1);
   BAST_g2_P_IndicateNotLocked_isrsafe(h);

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &val);
   if (((val >> 24) & 0x1F) != 17)
   {
      BDBG_MSG(("reacquiring because HP fell out of lock"));
      goto reacquire;
   }

   if (hChn->acqState == BAST_AcqState_eWaitForLock)
      BAST_g2_P_LdpcAcquire6_isr(h);
   else if ((hChn->turboCtl & BAST_G2_TURBO_CTL_DISABLE_FEC_REACQ) == 0)
   {
      BDBG_MSG(("reacquiring because Turbo fell out of lock"));

      reacquire:
      BAST_g2_P_LdpcReacquire_isr(h);
   }
   else
      BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 2000000, BAST_g2_P_TurboCheckHP_isr); /* 100msec */
}


/******************************************************************************
 BAST_g2_P_TurboCheckHP_isr()
******************************************************************************/
BERR_Code BAST_g2_P_TurboCheckHP_isr(BAST_ChannelHandle h)
{
   uint32_t val;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_SDS_STAT, &val);
   if (((val >> 24) & 0x1F) != 17)
   {
      BDBG_MSG(("reacquiring because HP fell out of lock"));
      return BAST_g2_P_LdpcReacquire_isr(h);
   }
   else
      return BAST_g2_P_EnableTimer_isr(h, BAST_TimerSelect_eBaud, 2000000, BAST_g2_P_TurboCheckHP_isr); /* 100msec */
}


/******************************************************************************
 BAST_g2_P_TurboSetOpll_isr()
******************************************************************************/
BERR_Code BAST_g2_P_TurboSetOpll_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, i;

   if ((hChn->actualMode >= BAST_Mode_eTurbo_Qpsk_1_2) && (hChn->actualMode <= BAST_Mode_eTurbo_8psk_8_9))
      i = hChn->actualMode - BAST_Mode_eTurbo_Qpsk_1_2;
   else
      return BERR_INVALID_PARAMETER;

   val = BAST_turbo_opll[i];
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_OPLL, &val);
   val = BAST_turbo_opll2[i];
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_SDS_OPLL2, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_TurboSetTssq_isr()
******************************************************************************/
BERR_Code BAST_g2_P_TurboSetTssq_isr(BAST_ChannelHandle h)
{
   const uint32_t* BAST_turbo_tssq_table[10] =
   {
      BAST_turbo_tssq_qpsk_1_2,
      BAST_turbo_tssq_qpsk_2_3,
      BAST_turbo_tssq_qpsk_3_4,
      BAST_turbo_tssq_qpsk_5_6,
      BAST_turbo_tssq_qpsk_7_8,
      BAST_turbo_tssq_8psk_2_3,
      BAST_turbo_tssq_8psk_3_4,
      BAST_turbo_tssq_8psk_4_5,
      BAST_turbo_tssq_8psk_5_6,
      BAST_turbo_tssq_8psk_8_9
   };

   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val, i, len;
   uint32_t *ptr;

   if ((hChn->actualMode >= BAST_Mode_eTurbo_Qpsk_1_2) && (hChn->actualMode <= BAST_Mode_eTurbo_8psk_8_9))
      i = hChn->actualMode - BAST_Mode_eTurbo_Qpsk_1_2;
   else
      return BERR_INVALID_PARAMETER;

   ptr = (uint32_t*) BAST_turbo_tssq_table[i];

   len = *ptr++;
   for (i = len; i; i--)
   {
      val = *ptr++;
      BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TFEC_TSSQ, &val);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_TurboSetTtur_isr()
******************************************************************************/
BERR_Code BAST_g2_P_TurboSetTtur_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t P_hi, P_lo, Q_hi, Q_lo, val;

   BAST_g2_P_GetFecFreq_isr(h);

   BMTH_HILO_32TO64_Mul(hChn->fecFreq, 10000, &P_hi, &P_lo);
   BMTH_HILO_64TO64_Div32(P_hi, P_lo, hChn->acqParams.symbolRate, &Q_hi, &Q_lo);
   BMTH_HILO_64TO64_Div32(Q_hi, Q_lo, 5625, &Q_hi, &Q_lo);
   Q_lo--;

   if (Q_lo > 19)
      Q_lo = 19;
   val = Q_lo << 16;
   val |= 0x0A03;
/* BDBG_MSG(("BAST_g2_P_TurboSetTtur_isr(): FEC freq=%d, TTUR=0x%X", hChn->fecFreq, val)); */

   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TFEC_TTUR, &val);

   return BERR_SUCCESS;
}


/******************************************************************************
 BAST_g2_P_TurboScanDetermineNextMode_isr()
******************************************************************************/
bool BAST_g2_P_TurboScanDetermineNextMode_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;

   hChn->actualMode = BAST_Mode_eUnknown;
   if (hChn->turboScanModes == 0)
      return false;

   next_mode:
   if (hChn->turboScanCurrMode == 0)
   {
      hChn->turboScanCurrMode = 1;
      hChn->turboScanState = 0;
   }
   else
   {
      hChn->turboScanCurrMode = (hChn->turboScanCurrMode << 1) & BAST_G2_TURBO_SCAN_MASK;

      if ((hChn->blindScanStatus == BAST_BlindScanStatus_eAcquire) && (hChn->turboScanCurrMode == 0))
         return false;
   }

   if (hChn->turboScanCurrMode & hChn->turboScanModes)
   {
      if (hChn->turboScanCurrMode & BAST_G2_TURBO_QPSK_SCAN_1_2)
         hChn->actualMode = BAST_Mode_eTurbo_Qpsk_1_2;
      else if (hChn->turboScanCurrMode & BAST_G2_TURBO_QPSK_SCAN_2_3)
         hChn->actualMode = BAST_Mode_eTurbo_Qpsk_2_3;
      else if (hChn->turboScanCurrMode & BAST_G2_TURBO_QPSK_SCAN_3_4)
         hChn->actualMode = BAST_Mode_eTurbo_Qpsk_3_4;
      else if (hChn->turboScanCurrMode & BAST_G2_TURBO_QPSK_SCAN_5_6)
         hChn->actualMode = BAST_Mode_eTurbo_Qpsk_5_6;
      else if (hChn->turboScanCurrMode & BAST_G2_TURBO_QPSK_SCAN_7_8)
         hChn->actualMode = BAST_Mode_eTurbo_Qpsk_7_8;
      else if (hChn->turboScanCurrMode & BAST_G2_TURBO_8PSK_SCAN_2_3)
         hChn->actualMode = BAST_Mode_eTurbo_8psk_2_3;
      else if (hChn->turboScanCurrMode & BAST_G2_TURBO_8PSK_SCAN_3_4)
         hChn->actualMode = BAST_Mode_eTurbo_8psk_3_4;
      else if (hChn->turboScanCurrMode & BAST_G2_TURBO_8PSK_SCAN_4_5)
         hChn->actualMode = BAST_Mode_eTurbo_8psk_4_5;
      else if (hChn->turboScanCurrMode & BAST_G2_TURBO_8PSK_SCAN_5_6)
         hChn->actualMode = BAST_Mode_eTurbo_8psk_5_6;
      else /* (hChn->turboScanCurrMode & BAST_G2_TURBO_8PSK_SCAN_8_9) */
         hChn->actualMode = BAST_Mode_eTurbo_8psk_8_9;

      if (hChn->turboScanState & BAST_TURBO_SCAN_HP_INIT)
      {
         if (hChn->turboScanState & BAST_TURBO_SCAN_HP_LOCKED)
         {
            if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
            {
               if ((hChn->turboScanState & BAST_TURBO_SCAN_8PSK_HP_LOCKED) == 0)
                  goto next_mode;
            }
            else if (hChn->turboScanState & BAST_TURBO_SCAN_8PSK_HP_LOCKED)
               goto next_mode;
         }
         else
         {
            if (hChn->turboScanState & BAST_TURBO_SCAN_8PSK_FAILED)
            {
               /* only consider qpsk */
               if (BAST_MODE_IS_TURBO_8PSK(hChn->actualMode))
                  goto next_mode;
            }
            else
            {
               /* only consider 8psk */
               if (!(BAST_MODE_IS_TURBO_8PSK(hChn->actualMode)))
                  goto next_mode;
            }
         }
      }
   }
   else
      goto next_mode;

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
         BDBG_ERR(("BAST_g2_P_TurboScanDetermineNextMode_isr(): should not get here!"));
         return false;
   }

   return true;
}


/******************************************************************************
 BAST_g2_P_TurboUpdateErrorCounters()
******************************************************************************/
void BAST_g2_P_TurboUpdateErrorCounters(BAST_ChannelHandle h)
{
   BKNI_EnterCriticalSection();
   BAST_g2_P_TurboUpdateErrorCounters_isr(h);
   BKNI_LeaveCriticalSection();
}


/******************************************************************************
 BAST_g2_P_TurboUpdateErrorCounters_isr()
******************************************************************************/
void BAST_g2_P_TurboUpdateErrorCounters_isr(BAST_ChannelHandle h)
{
   BAST_g2_P_ChannelHandle *hChn = (BAST_g2_P_ChannelHandle *)h->pImpl;
   uint32_t val;

#if (BCHP_CHIP==7340) || (BCHP_CHIP==7342)
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_TFEC_TNBLK, &val);
   hChn->turboTotalBlocks += val;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_TFEC_TCBLK, &val);
   hChn->turboCorrBlocks += val;

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_TFEC_TBBLK, &val);
   hChn->turboBadBlocks += val;

   /* reset the FEC error counters */
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_TFEC_TFECTL, &val);
   val |= 0x06;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, &val);
   val &= ~0x06;
   BAST_g2_P_WriteRegister_isrsafe(h, BCHP_TFEC_TFECTL, &val);
#else
   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_TFEC_TCOR, &val);
   hChn->turboTotalBlocks += ((val >> 16) & 0xFFFF);
   hChn->turboCorrBlocks += (val & 0xFFFF);

   BAST_g2_P_ReadRegister_isrsafe(h, BCHP_TFEC_TERR, &val);
   hChn->turboBadBlocks += (val & 0xFFFF);
#endif
}

#endif
