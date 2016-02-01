/***************************************************************************
 *     (c)2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
#ifndef BVC5_SCHEDULER_STATE_H__
#define BVC5_SCHEDULER_STATE_H__

#include "berr.h"
#include "bvc5_client_priv.h"

/* BVC5_P_SchedulerState

   State relevant to the scheduler thread.

 */
typedef struct BVC5_P_SchedulerState
{
   uint32_t           uiClientHandlesCapacity;  /* How big is client handle array (will grow, not shrink)               */
   BVC5_ClientHandle *phClientHandles;          /* Array of client handles used in round-robin                          */
   uint32_t           uiClientOffset;           /* Which client is considered first in the schedule                     */
} BVC5_P_SchedulerState;

/***************************************************************************/

/* BVC5_P_SchedulerStateConstruct

   Initialize scheduler state.

 */
BERR_Code BVC5_P_SchedulerStateConstruct(
   BVC5_P_SchedulerState *phSchedulerState
);

/***************************************************************************/

/* BVC5_P_SchedulerStateConstruct

   Destroy scheduler state.

 */
BERR_Code BVC5_P_SchedulerStateDestruct(
   BVC5_P_SchedulerState *hSchedulerState
);

/***************************************************************************/

/* BVC5_P_SchedulerStateRegisterClient

   Adds a new client to the scheduler state

 */
BERR_Code BVC5_P_SchedulerStateRegisterClient(
   BVC5_P_SchedulerState *psSchedulerState,
   uint32_t               uiNumClients
);

/***************************************************************************/

/* BVC5_P_SchedulerStateResetFirst

   Removes client from the scheduler state e.g. on process termination

 */
BERR_Code BVC5_P_SchedulerStateResetFirst(
   BVC5_P_SchedulerState *psSchedulerState
);

#endif /* BVC5_SCHEDULER_STATE_H__ */
