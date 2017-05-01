/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bstd.h"
#include "bvc5_scheduler_state_priv.h"
#include "bvc5_client_priv.h"
#include "bvc5_hardware_priv.h"

#define CLIENT_HANDLES_INITIAL_CAPACITY   16

BERR_Code BVC5_P_SchedulerStateConstruct(
   BVC5_P_SchedulerState *psState,
   BVC5_ClientMapHandle   hClientMap
)
{
   if (psState == NULL)
      return BERR_INVALID_PARAMETER;

   BKNI_Memset(psState, 0, sizeof(BVC5_P_SchedulerState));

   psState->uiClientsCapacity = CLIENT_HANDLES_INITIAL_CAPACITY;
   psState->phClients         = (BVC5_ClientHandle *)BKNI_Malloc(sizeof(BVC5_ClientHandle) * CLIENT_HANDLES_INITIAL_CAPACITY);

   if (psState->phClients == NULL)
      return BERR_OUT_OF_SYSTEM_MEMORY;

   psState->uiCurrentClient = 0;
   psState->hClientMap      = hClientMap;
   psState->bHoldingPower   = false;

   return BERR_SUCCESS;
}

BERR_Code BVC5_P_SchedulerStateDestruct(
   BVC5_P_SchedulerState *psState
)
{
   if (psState == NULL)
      return BERR_INVALID_PARAMETER;

   if (psState->phClients != NULL)
      BKNI_Free(psState->phClients);

   return BERR_SUCCESS;
}

BERR_Code BVC5_P_SchedulerStateRegisterClient(
   BVC5_P_SchedulerState *psState,
   uint32_t               uiNumClients
)
{
   if (psState == NULL)
      return BERR_INVALID_PARAMETER;

   if (uiNumClients > psState->uiClientsCapacity)
   {
      if (psState->phClients != NULL)
         BKNI_Free(psState->phClients);

      /* Make room for a bunch more clients */
      psState->uiClientsCapacity = uiNumClients * 2;
      psState->phClients         = (BVC5_ClientHandle *)BKNI_Malloc(sizeof(BVC5_ClientHandle) * psState->uiClientsCapacity);

      if (psState->phClients == NULL)
         return BERR_OUT_OF_SYSTEM_MEMORY;
   }

   return BERR_SUCCESS;
}

static BVC5_ClientHandle BVC5_P_GetCurrentClient(
   BVC5_P_SchedulerState *psState
)
{
   if (psState->uiCurrentClient >= BVC5_P_ClientMapSize(psState->hClientMap))
      psState->uiCurrentClient = 0;

   return psState->phClients[psState->uiCurrentClient];
}

/* BVC5_P_SchedulerStateGatherClients

   Copy client handles into schedulerState client array for round robin processing
   of clients.

 */
BVC5_ClientHandle *BVC5_P_SchedulerStateGatherClients(
   BVC5_P_SchedulerState *psState
)
{
   void                *iter = NULL;
   uint32_t             i;
   BVC5_ClientMapHandle hClientMap   = psState->hClientMap;
   BVC5_ClientHandle    hClient      = BVC5_P_ClientMapFirst(hClientMap, &iter);
   uint32_t             uiNumClients = BVC5_P_ClientMapSize(hClientMap);

   for (i = 0; i < uiNumClients; ++i)
   {
      psState->phClients[i] = hClient;
      hClient = BVC5_P_ClientMapNext(hClientMap, &iter);
   }

   return psState->phClients;
}

void BVC5_P_SchedulerStateSetClientWanted(
   BVC5_P_SchedulerState *psState
)
{
   BVC5_ClientHandle hCurrentClient = BVC5_P_GetCurrentClient(psState);

   BVC5_P_ClientSetWanted(hCurrentClient);
}

void BVC5_P_SchedulerStateNextClient(
   BVC5_P_SchedulerState *psState
)
{
   BVC5_ClientHandle hCurrentClient = BVC5_P_GetCurrentClient(psState);

   if (BVC5_P_ClientSatisifed(hCurrentClient))
   {
      psState->uiCurrentClient     = psState->uiCurrentClient + 1;
      hCurrentClient->uiWorkWanted = 0;
      hCurrentClient->uiWorkGiven  = 0;
   }
}

BVC5_ClientHandle BVC5_P_SchedulerStateGetClient(
   BVC5_P_SchedulerState *psState,
   uint32_t               uiClient
)
{
   uint32_t uiActual = uiClient + psState->uiCurrentClient;

   uiActual = uiActual % BVC5_P_ClientMapSize(psState->hClientMap);

   return psState->phClients[uiActual];
}
