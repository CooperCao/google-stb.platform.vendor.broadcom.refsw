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


BDBG_MODULE(bsat_g1_priv_timer);


#ifdef LEAP_ARC
#pragma Offwarn(254) /* suppress warning message "Casting a data pointer to a function pointer, or vice-versa, is not sanctioned by the C or C++ standards" */
#endif
#ifdef BSAT_HAS_LEAP
extern void * irq_sub_funct;
#endif


/******************************************************************************
 BSAT_g1_P_DisableTimer_isr() - ISR context
******************************************************************************/
BERR_Code BSAT_g1_P_DisableTimer_isr(BSAT_ChannelHandle h, BSAT_TimerSelect t)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   uint32_t reg = BCHP_SDS_MISC_TMRCTL;
   BINT_CallbackHandle hCb = NULL;
   BERR_Code retCode = BERR_SUCCESS;
   uint32_t and_mask = 0xFFFFFFFF;

   switch (t)
   {
      case BSAT_TimerSelect_eBaud:
      case BSAT_TimerSelect_eBaudUsec:
         and_mask = ~0x02;
         hCb = hChn->hBaudTimerCb;
         hChn->baudTimerIsr = NULL;
         break;
      case BSAT_TimerSelect_eBer:
         and_mask = ~0x04;
         hCb = hChn->hBerTimerCb;
         hChn->berTimerIsr = NULL;
         break;
      case BSAT_TimerSelect_eGen1:
         and_mask = ~0x08;
         hCb = hChn->hGen1TimerCb;
         hChn->gen1TimerIsr = NULL;
         break;
      case BSAT_TimerSelect_eGen2:
         and_mask = ~0x10;
         hCb = hChn->hGen2TimerCb;
         hChn->gen2TimerIsr = NULL;
         break;
      case BSAT_TimerSelect_eGen3:
         and_mask = ~0x20;
         hCb = hChn->hGen3TimerCb;
         hChn->gen3TimerIsr = NULL;
         break;
      default:
         BDBG_ERR(("invalid timer specified"));
         BDBG_ASSERT(0); /* should never get here */
         return BERR_TRACE(BERR_INVALID_PARAMETER);
   }

   BINT_DisableCallback_isr(hCb);
   BINT_ClearCallback_isr(hCb);

   BSAT_g1_P_AndRegister_isrsafe(h, reg, and_mask);
   return retCode;
}


/******************************************************************************
 BSAT_g1_P_EnableTimer() - Non-ISR context
******************************************************************************/
BERR_Code BSAT_g1_P_EnableTimer(BSAT_ChannelHandle h, BSAT_TimerSelect t, uint32_t delay, BSAT_g1_FUNCT func)
{
   BERR_Code retCode;

   BKNI_EnterCriticalSection();
   retCode = BSAT_g1_P_EnableTimer_isr(h, t, delay, func);
   BKNI_LeaveCriticalSection();

   return retCode;
}


/******************************************************************************
 BSAT_g1_P_EnableTimer_isr() - ISR context
******************************************************************************/
BERR_Code BSAT_g1_P_EnableTimer_isr(BSAT_ChannelHandle h, BSAT_TimerSelect t, uint32_t delay, BSAT_g1_FUNCT func)
{
   BSAT_g1_P_ChannelHandle *hChn = (BSAT_g1_P_ChannelHandle *)h->pImpl;
   BINT_CallbackHandle hCb = NULL;
   uint32_t tmrctl_reg = BCHP_SDS_MISC_TMRCTL;
   uint32_t tmrctl_set = 0, tmrctl_clr = 0;
   uint32_t count_reg, val;

   BDBG_ASSERT(h);

   BSAT_g1_P_DisableTimer_isr(h, t);

   if (delay == 0)
   {
      BDBG_ERR(("BSAT_g1_P_EnableTimer_isr(%d): delay=0, t=%d, func=0x%X", h->channel, t, (unsigned int)func));
      return BERR_INVALID_PARAMETER;
   }

   switch (t)
   {
      case BSAT_TimerSelect_eBaud:
      case BSAT_TimerSelect_eBaudUsec:
         count_reg = BCHP_SDS_MISC_BTMR;
         if (func)
         {
            hChn->baudTimerIsr = func;
            hCb = hChn->hBaudTimerCb;
         }
         else
            hChn->baudTimerIsr = NULL;
         if (t == BSAT_TimerSelect_eBaudUsec)
            tmrctl_set |= 0x01;
         else
            tmrctl_clr |= 0x01;
         tmrctl_set |= 0x02; /* timer start */
         break;

      case BSAT_TimerSelect_eBer:
         count_reg = BCHP_SDS_MISC_BERTMR;
         if (func)
         {
            hChn->berTimerIsr = func;
            hCb = hChn->hBerTimerCb;
         }
         else
            hChn->berTimerIsr = NULL;
         tmrctl_set |= 0x04;
         break;

      case BSAT_TimerSelect_eGen1:
         count_reg = BCHP_SDS_MISC_GENTMR1;
         if (func)
         {
            hChn->gen1TimerIsr = func;
            hCb = hChn->hGen1TimerCb;
         }
         else
            hChn->gen1TimerIsr = NULL;
         tmrctl_set |= 0x08;
         break;

      case BSAT_TimerSelect_eGen2:
         count_reg = BCHP_SDS_MISC_GENTMR2;
         if (func)
         {
            hChn->gen2TimerIsr = func;
            hCb = hChn->hGen2TimerCb;
         }
         else
            hChn->gen2TimerIsr = NULL;
         tmrctl_set |= 0x10;
         break;

      case BSAT_TimerSelect_eGen3:
         count_reg = BCHP_SDS_MISC_GENTMR3;
         if (func)
         {
            hChn->gen3TimerIsr = func;
            hCb = hChn->hGen3TimerCb;
         }
         else
            hChn->gen3TimerIsr = NULL;
         tmrctl_set |= 0x20;
         break;

      default:
         BDBG_ERR(("invalid timer specified"));
         BDBG_ASSERT(0); /* should never get here */
         return BERR_INVALID_PARAMETER;
   }

   if (func)
   {
      /* clear interrupt and enable */
      BINT_ClearCallback_isr(hCb);
      BINT_EnableCallback_isr(hCb);
   }

   /* load timer value */
   BSAT_g1_P_WriteRegister_isrsafe(h, count_reg, delay);
   BSAT_g1_P_WriteRegister_isrsafe(h, count_reg, delay);

   /* start the timer */
   BSAT_g1_P_ReadModifyWriteRegister_isrsafe(h, tmrctl_reg, ~tmrctl_clr, tmrctl_set);

   if ((func == NULL) && (delay < 1000))
   {
      do
      {
         val = BSAT_g1_P_ReadRegister_isrsafe(h, count_reg);
      } while (val > 0);
   }

   return BERR_SUCCESS;
}


/******************************************************************************
 BSAT_g1_P_BaudTimer_isr()
******************************************************************************/
void BSAT_g1_P_BaudTimer_isr(void *p, int param)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = h->pImpl;
   BSAT_g1_FUNCT funct = hChn->baudTimerIsr;

   BSTD_UNUSED(param);

   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eBaudUsec);
   if (funct)
   {
#ifdef BSAT_HAS_LEAP
      irq_sub_funct = (void*)funct; /* for debug only */
#endif
      funct(h);
   }
}


/******************************************************************************
 BSAT_g1_P_BerTimer_isr()
******************************************************************************/
void BSAT_g1_P_BerTimer_isr(void *p, int param)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = h->pImpl;
   BSAT_g1_FUNCT funct = hChn->berTimerIsr;

   BSTD_UNUSED(param);

   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eBer);
   if (funct)
   {
#ifdef BSAT_HAS_LEAP
      irq_sub_funct = (void*)funct; /* for debug only */
#endif
      funct(h);
   }
}


/******************************************************************************
 BSAT_g1_P_Gen1Timer_isr()
******************************************************************************/
void BSAT_g1_P_Gen1Timer_isr(void *p, int param)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = h->pImpl;
   BSAT_g1_FUNCT funct = hChn->gen1TimerIsr;

   BSTD_UNUSED(param);

   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eGen1);
   if (funct)
   {
#ifdef BSAT_HAS_LEAP
      irq_sub_funct = (void*)funct; /* for debug only */
#endif
      funct(h);
   }
}


/******************************************************************************
 BSAT_g1_P_Gen2Timer_isr()
******************************************************************************/
void BSAT_g1_P_Gen2Timer_isr(void *p, int param)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = h->pImpl;
   BSAT_g1_FUNCT funct = hChn->gen2TimerIsr;

   BSTD_UNUSED(param);

   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eGen2);
   if (funct)
   {
#ifdef BSAT_HAS_LEAP
      irq_sub_funct = (void*)funct; /* for debug only */
#endif
      funct(h);
   }
}


/******************************************************************************
 BSAT_g1_P_Gen3Timer_isr()
******************************************************************************/
void BSAT_g1_P_Gen3Timer_isr(void *p, int param)
{
   BSAT_ChannelHandle h = (BSAT_ChannelHandle)p;
   BSAT_g1_P_ChannelHandle *hChn = h->pImpl;
   BSAT_g1_FUNCT funct = hChn->gen3TimerIsr;

   BSTD_UNUSED(param);

   BSAT_g1_P_DisableTimer_isr(h, BSAT_TimerSelect_eGen3);
   if (funct)
   {
#ifdef BSAT_HAS_LEAP
      irq_sub_funct = (void*)funct; /* for debug only */
#endif
      funct(h);
   }
}


/******************************************************************************
 BSAT_g1_P_TimerIsRunning()
******************************************************************************/
bool BSAT_g1_P_IsTimerRunning_isr(BSAT_ChannelHandle h, BSAT_TimerSelect t)
{
   uint32_t mask = 0, reg = BCHP_SDS_MISC_TMRCTL;

   switch (t)
   {
      case BSAT_TimerSelect_eBaud:
      case BSAT_TimerSelect_eBaudUsec:
         mask = 0x02;
         break;
      case BSAT_TimerSelect_eBer:
         mask = 0x04;
         break;
      case BSAT_TimerSelect_eGen1:
         mask = 0x08;
         break;
      case BSAT_TimerSelect_eGen2:
         mask = 0x10;
         break;
      case BSAT_TimerSelect_eGen3:
         mask = 0x20;
         break;
      default:
         /* BDBG_ERR(("invalid timer specified in BSAT_g1_P_TimerIsRunning()")); */
         BDBG_ASSERT(0);
         return BERR_INVALID_PARAMETER;
   }

   return (BSAT_g1_P_ReadRegister_isrsafe(h, reg) & mask) ? true : false;
}


/******************************************************************************
 BSAT_g1_P_TimerIsExpired()
******************************************************************************/
bool BSAT_g1_P_IsTimerExpired_isr(BSAT_ChannelHandle h, BSAT_TimerSelect t)
{
   uint32_t count_reg;

   if (BSAT_g1_P_IsTimerRunning_isr(h, t))
   {
      switch (t)
      {
         case BSAT_TimerSelect_eBaud:
         case BSAT_TimerSelect_eBaudUsec:
            count_reg = BCHP_SDS_MISC_BTMR;
            break;
         case BSAT_TimerSelect_eBer:
            count_reg = BCHP_SDS_MISC_BERTMR;
            break;
         case BSAT_TimerSelect_eGen1:
            count_reg = BCHP_SDS_MISC_GENTMR1;
            break;
         case BSAT_TimerSelect_eGen2:
            count_reg = BCHP_SDS_MISC_GENTMR2;
            break;
         case BSAT_TimerSelect_eGen3:
            count_reg = BCHP_SDS_MISC_GENTMR3;
            break;
         default:
            BDBG_ASSERT(0); /* should never get here */
            return BERR_INVALID_PARAMETER;
      }
      if (BSAT_g1_P_ReadRegister_isrsafe(h, count_reg) == 0)
         return true;
   }
   return false;
}


