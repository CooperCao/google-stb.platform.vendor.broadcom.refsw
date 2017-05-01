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
#ifndef BVC5_SCHEDULER_STATE_H__
#define BVC5_SCHEDULER_STATE_H__

#include "berr.h"
#include "bvc5_client_priv.h"

/* BVC5_P_SchedulerState

   State relevant to the scheduler thread.

 */
typedef struct BVC5_P_SchedulerState
{
   uint32_t           uiClientsCapacity;     /* How big is client handle array (will grow, not shrink) */
   BVC5_ClientHandle *phClients;             /* Array of client handles used in round-robin            */
   uint32_t           uiCurrentClient;       /* The client who is offered work first                   */

   bool               bHoldingPower;         /* Is the scheduler holding power/clocks on?              */

   struct BVC5_P_ClientMap *hClientMap;      /* Copy of the client map for our convenience             */
} BVC5_P_SchedulerState;

/***************************************************************************/

/* BVC5_P_SchedulerStateConstruct

   Initialize scheduler state.

 */
BERR_Code BVC5_P_SchedulerStateConstruct(
   BVC5_P_SchedulerState *psState,
   BVC5_ClientMapHandle   hClientMap
);

/***************************************************************************/

/* BVC5_P_SchedulerStateConstruct

   Destroy scheduler state.

 */
BERR_Code BVC5_P_SchedulerStateDestruct(
   BVC5_P_SchedulerState *psState
);

/***************************************************************************/

/* BVC5_P_SchedulerStateRegisterClient

   Adds a new client to the scheduler state

 */
BERR_Code BVC5_P_SchedulerStateRegisterClient(
   BVC5_P_SchedulerState *psState,
   uint32_t               uiNumClients
);

/***************************************************************************/

/* BVC5_P_SchedulerStateGatherClients

   Build and return the list of clients starting with the preferred client

 */
BVC5_ClientHandle *BVC5_P_SchedulerStateGatherClients(
   BVC5_P_SchedulerState *psState
);

/* BVC5_P_SchedulerStateBegin

   Set up current client requirements and initialise power release count
 */
void BVC5_P_SchedulerStateSetClientWanted(
   BVC5_P_SchedulerState *psState
);

/* BVC5_P_SchedulerStateNextClient

   Move to the next client if the current client has had all its requirements met
 */
void BVC5_P_SchedulerStateNextClient(
   BVC5_P_SchedulerState *psState
);

/* BVC5_P_SchedulerStateGetClient

   Get the nth client where 0 is the current client

 */
BVC5_ClientHandle BVC5_P_SchedulerStateGetClient(
   BVC5_P_SchedulerState *psState,
   uint32_t               uiClient
);

#endif /* BVC5_SCHEDULER_STATE_H__ */
