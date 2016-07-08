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
 ******************************************************************************
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/ZbPro/NWK/include/private/bbZbProNwkReset.h $
*
* DESCRIPTION:
*   Network Layer Management Entity Reset primitive PRIVATE declarations.
*
* $Revision: 2720 $
* $Date: 2014-06-24 14:43:25Z $
*
****************************************************************************************/

#ifndef _ZBPRO_NWK_RESET_PRIVATE_H
#define _ZBPRO_NWK_RESET_PRIVATE_H

/************************* INCLUDES ****************************************************/
#include "bbZbProNwkCommon.h"

/************************* DEFINITIONS *************************************************/
#define NWK_RESET_HANDLER_DESCR_INITIALIZER { \
                                                .queue = SYS_QUEUE_INITIALIZER, \
                                                .state = S_IDLE \
                                            }

/************************* TYPES *******************************************************/
/**//**
 * \brief Internal memory of Reset handler.
 */
typedef struct _ZbProNwkResetHandlerDescr_t
{
    SYS_QueueDescriptor_t         queue;      /**< Queue of requests from other NWK components. */
    MAC_ResetReqDescr_t           macReset;   /**< MLME-RESET.request descriptor. */
    SYS_FSM_StateId_t             state;      /**< Current state of the Reset Handler FSM. */
} ZbProNwkResetHandlerDescr_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief Reset task handler.
 ****************************************************************************************/
NWK_PRIVATE void zbProResetHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
    \brief Reset handler reset routine
****************************************************************************************/
NWK_PRIVATE void zbProNwkResetHandlerReset(void);

#endif /* _ZBPRO_NWK_RESET_PRIVATE_H */