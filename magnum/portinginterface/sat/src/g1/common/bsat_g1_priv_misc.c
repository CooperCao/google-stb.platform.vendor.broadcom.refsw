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
#include "bsat.h"
#include "bsat_priv.h"
#include "bsat_g1_priv.h"


BDBG_MODULE(bsat_g1_priv_misc);


/******************************************************************************
 BSAT_g1_P_ReadModifyWriteRegister_isrsafe()
******************************************************************************/
void BSAT_g1_P_ReadModifyWriteRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t and_mask, uint32_t or_mask)
{
   uint32_t val;

   val = (BSAT_g1_P_ReadRegister_isrsafe(h, reg) & and_mask) | or_mask;
   BSAT_g1_P_WriteRegister_isrsafe(h, reg, val);
}


/******************************************************************************
 BSAT_g1_P_OrRegister_isrsafe()
******************************************************************************/
void BSAT_g1_P_OrRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t or_mask)
{
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, reg) | or_mask;
   BSAT_g1_P_WriteRegister_isrsafe(h, reg, val);
}


/******************************************************************************
 BSAT_g1_P_AndRegister_isrsafe()
******************************************************************************/
void BSAT_g1_P_AndRegister_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t and_mask)
{
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, reg) & and_mask;
   BSAT_g1_P_WriteRegister_isrsafe(h, reg, val);
}


/******************************************************************************
 BSAT_g1_P_ToggleBit_isrsafe()
******************************************************************************/
void BSAT_g1_P_ToggleBit_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t mask)
{
   uint32_t val;

   val = BSAT_g1_P_ReadRegister_isrsafe(h, reg) | mask;
   BSAT_g1_P_WriteRegister_isrsafe(h, reg, val);
   val &= ~mask;
   BSAT_g1_P_WriteRegister_isrsafe(h, reg, val);
}


/******************************************************************************
 BSAT_g1_P_GetTraceInfo()
******************************************************************************/
BERR_Code BSAT_g1_P_GetTraceInfo(BSAT_ChannelHandle h, BSAT_TraceInfo *pBuffer)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (pBuffer == NULL)
      return (BERR_TRACE(BERR_INVALID_PARAMETER));

   BKNI_Memcpy((void*)pBuffer, hChn->trace, sizeof(BSAT_TraceInfo));
   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_ResetAcquisitionTimer_isr() - ISR context
******************************************************************************/
BERR_Code BSAT_g1_P_ResetAcquisitionTimer_isr(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BERR_Code retCode;

   hChn->configParam[BSAT_g1_CONFIG_ACQ_TIME] = 0;
   retCode = BSAT_g1_P_EnableTimer_isr(h, BSAT_TimerSelect_eGen3, 0xFFFFFFFF, NULL);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_GetAcquisitionTimerValue_isr()
******************************************************************************/
void BSAT_g1_P_GetAcquisitionTimerValue_isr(BSAT_ChannelHandle h, uint32_t *pVal)
{
   uint32_t lval1;

   lval1 = BSAT_g1_P_ReadRegister_isrsafe(h, BCHP_SDS_MISC_GENTMR3);
   *pVal = (0xFFFFFFFF - lval1);
}


/******************************************************************************
 BSAT_g1_P_ClearTraceBuffer()
******************************************************************************/
BERR_Code BSAT_g1_P_ClearTraceBuffer(BSAT_ChannelHandle h)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t i;

   for (i = 0; i < BSAT_TraceEvent_eMax; i++)
      hChn->trace[i] = 0;

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_LogTraceBuffer_isr()
******************************************************************************/
BERR_Code BSAT_g1_P_LogTraceBuffer_isr(BSAT_ChannelHandle h, BSAT_TraceEvent event)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   if (event < BSAT_TraceEvent_eMax)
   {
      BSAT_g1_P_GetAcquisitionTimerValue_isr(h, &(hChn->trace[event]));
      return BERR_SUCCESS;
   }
   else
      return BERR_INVALID_PARAMETER;
}

/******************************************************************************
 BSAT_g1_P_GetRegisterWriteWaitTime_isrsafe()
******************************************************************************/
void BSAT_g1_P_GetRegisterWriteWaitTime_isrsafe(BSAT_ChannelHandle h, uint32_t reg, uint32_t *wait_time)
{
#if 1 /*ndef BSAT_HAS_LEAP*/
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;

   *wait_time = 0;
   if (hChn->acqSettings.symbolRate < 8000000)
   {
      /* VIT, FEC, CL, and EQ are in the baud clock domain */
      if ((reg >= BCHP_SDS_BL_BLPCTL) && (reg <= BCHP_SDS_MISC_INTR_RAW_STS1))
      {
         if (hChn->acqSettings.symbolRate > 3000000)
            *wait_time = 1;
         else if (hChn->acqSettings.symbolRate > 1500000)
            *wait_time = 2;
         else
            *wait_time = 3;
      }
   }
#else
   BSTD_UNUSED(h);
   BSTD_UNUSED(reg);
   *wait_time = 0;
#endif
}


/******************************************************************************
 BSAT_g1_P_GCF_isrsafe()
******************************************************************************/
uint32_t BSAT_g1_P_GCF_isrsafe(uint32_t m, uint32_t n)
{
   uint32_t L1, L2, L3, L4;

   L1 = m;
   L2 = n;

   while (L2 > 0)
   {
      L3 = L1 / L2;
      L4 = L1 - (L2 * L3);
      L1 = L2;
      L2 = L4;
   }
   return L1;
}
