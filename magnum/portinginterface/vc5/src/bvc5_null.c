/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#if defined(BVC5_HARDWARE_NONE)

#include "bstd.h"
#include "bvc5.h"
#include "bvc5_priv.h"
#include "bvc5_registers_priv.h"
#include "bvc5_null_priv.h"

#ifdef BVC5_USERMODE
#include <pthread.h>
#include <stdlib.h>
#endif

typedef struct BVC5_P_DelayThreadArgs
{
   BVC5_Handle hVC5;
   uint32_t    uiReason;
   uint32_t    delayUs;
} BVC5_P_DelayThreadArgs;

static void BVC5_P_FakeInterrupt(
   BVC5_Handle hVC5,
   uint32_t    uiReason
)
{
   hVC5->uiInterruptReason |= uiReason;
   BKNI_SetEvent(hVC5->hSchedulerWakeEvent);
}

uint32_t BVC5_P_NullReadRegister(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex,
   uint32_t    uiReg
)
{
   uint32_t ret = 0;

   BSTD_UNUSED(hVC5);
   BSTD_UNUSED(uiCoreIndex);

   switch (uiReg)
   {
   case BCHP_V3D_CTL_IDENT0 : ret = 0x03443356; break;
   case BCHP_V3D_CTL_IDENT1 : ret = 0x41101432; break;
   case BCHP_V3D_CTL_IDENT2 : ret = 0x00078121; break;
   case BCHP_V3D_CTL_IDENT3 : ret = 0x00000000; break;

   default: break;
   }

   return ret;
}

void *BVC5_P_DelayThread(void *a)
{
   BVC5_P_DelayThreadArgs *args = (BVC5_P_DelayThreadArgs *)a;

   usleep(args->delayUs);

   BVC5_P_FakeInterrupt(args->hVC5, args->uiReason);

   BKNI_Free(args);
   pthread_exit(NULL);
}

static void BVC5_P_DelayedFakeInterrupt(
   BVC5_Handle hVC5,
   uint32_t    uiReason,
   bool        bBinner
   )
{
#ifdef BVC5_USERMODE
   /* Hack up a bin delay */
   pthread_t               thread;
   pthread_attr_t          tattr;
   int                     rc;
   BVC5_P_DelayThreadArgs  *args;
   static uint32_t         uiBinDelayUs = 0xFFFFFFFF;
   static uint32_t         uiRdrDelayUs = 0xFFFFFFFF;

   if (uiBinDelayUs == 0xFFFFFFFF)
   {
      char *c;

      uiBinDelayUs = 0;
      uiRdrDelayUs = 0;

      c = getenv("V3D_NULL_BIN_TIME_US");
      if (c != NULL)
         uiBinDelayUs = atoi(c);

      c = getenv("V3D_NULL_RENDER_TIME_US");
      if (c != NULL)
         uiRdrDelayUs = atoi(c);
   }

   if ((bBinner && uiBinDelayUs < 200) || (!bBinner && uiRdrDelayUs < 200))
   {
      BVC5_P_FakeInterrupt(hVC5, uiReason);
      return;
   }

   args = BKNI_Malloc(sizeof(BVC5_P_DelayThreadArgs));
   if (args != NULL)
   {
      args->hVC5 = hVC5;
      args->uiReason = uiReason;
      if (bBinner)
         args->delayUs = uiBinDelayUs;
      else
         args->delayUs = uiRdrDelayUs;

      /* initialized with default attributes */
      pthread_attr_init(&tattr);
      pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
      rc = pthread_create(&thread, &tattr, BVC5_P_DelayThread, (void *)args);
      if (rc)
      {
         /* Failed to create - forget about the delay */
         BVC5_P_FakeInterrupt(hVC5, uiReason);
         BKNI_Free(args);
      }
      pthread_attr_destroy(&tattr);
   }
#else
   BVC5_P_FakeInterrupt(hVC5, uiReason);
#endif
}

void BVC5_P_NullWriteRegister(
   BVC5_Handle hVC5,
   uint32_t    uiCoreIndex,
   uint32_t    uiReg,
   uint32_t    uiValue
)
{
   BSTD_UNUSED(uiCoreIndex);
   BSTD_UNUSED(uiValue);

   switch (uiReg)
   {
   case BCHP_V3D_CLE_CT0QEA  :
   case BCHP_V3D_CLE_CT0EA   :
      BVC5_P_DelayedFakeInterrupt(hVC5, BCHP_V3D_CTL_INT_STS_INT_FLDONE_SET, true);
      break;

   case BCHP_V3D_CLE_CT1QEA  :
   case BCHP_V3D_CLE_CT1EA   :
      BVC5_P_DelayedFakeInterrupt(hVC5, BCHP_V3D_CTL_INT_STS_INT_FRDONE_SET, false);
      break;

   case BCHP_V3D_TFU_TFUICFG :
      /* TODO */
      break;

   case BCHP_V3D_QPS_SRQPC   :
      /* TODO */
      break;

   default:
      break;
   }
}

#endif

/* Avoid compiler warning about empty translation unit */
static void BVC5_P_NullDummy(void)
{
}
