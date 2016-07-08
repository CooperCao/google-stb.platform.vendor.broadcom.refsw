/***************************************************************************
 *     Broadcom Proprietary and Confidential. (c)2014 Broadcom.  All rights reserved.
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
 *
 **************************************************************************/

#include "bstd.h"
#include "bvc5_scheduler_state_priv.h"

#define CLIENT_HANDLES_INITIAL_CAPACITY   16

BERR_Code BVC5_P_SchedulerStateConstruct(
   BVC5_P_SchedulerState *psSchedulerState
)
{
   if (psSchedulerState == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_Memset(psSchedulerState, 0, sizeof(BVC5_P_SchedulerState));

   psSchedulerState->uiClientHandlesCapacity = CLIENT_HANDLES_INITIAL_CAPACITY;
   psSchedulerState->phClientHandles         = (BVC5_ClientHandle *)BKNI_Malloc(sizeof(BVC5_ClientHandle) * CLIENT_HANDLES_INITIAL_CAPACITY);

   if (psSchedulerState->phClientHandles == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   return BERR_SUCCESS;
}

BERR_Code BVC5_P_SchedulerStateDestruct(
   BVC5_P_SchedulerState *psSchedulerState
)
{
   if (psSchedulerState == NULL)
      return BERR_INVALID_PARAMETER;

   if (psSchedulerState->phClientHandles != NULL)
      BKNI_Free(psSchedulerState->phClientHandles);

   return BERR_SUCCESS;
}

BERR_Code BVC5_P_SchedulerStateRegisterClient(
   BVC5_P_SchedulerState *psSchedulerState,
   uint32_t               uiNumClients
)
{
   if (psSchedulerState == NULL)
      return BERR_INVALID_PARAMETER;

   if (uiNumClients > psSchedulerState->uiClientHandlesCapacity)
   {
      if (psSchedulerState->phClientHandles != NULL)
         BKNI_Free(psSchedulerState->phClientHandles);

      /* Make room for a bunch more clients */
      psSchedulerState->uiClientHandlesCapacity = uiNumClients * 2;
      psSchedulerState->phClientHandles         = (BVC5_ClientHandle *)BKNI_Malloc(sizeof(BVC5_ClientHandle) * psSchedulerState->uiClientHandlesCapacity);

      if (psSchedulerState->phClientHandles == NULL)
         return BERR_OUT_OF_SYSTEM_MEMORY;
   }

   return BERR_SUCCESS;
}

BERR_Code BVC5_P_SchedulerStateResetFirst(
   BVC5_P_SchedulerState *pSchedulerState
)
{
   if (pSchedulerState == NULL)
      return BERR_INVALID_PARAMETER;

   /* Reset the round-robin counter so it doesn't index off the end of the
      list of clients.
      Client unregister is infrequent, so this should be ok.
   */
   pSchedulerState->uiClientOffset = 0;

   return BERR_SUCCESS;
}
