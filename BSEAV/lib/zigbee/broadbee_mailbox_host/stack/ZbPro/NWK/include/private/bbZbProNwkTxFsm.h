/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

/******************************************************************************
*
* DESCRIPTION:
*       ZigBee PRO Network layer TX Fsm definition.
*
*******************************************************************************/

#ifndef _BB_ZB_PRO_NWK_TX_FSM_H
#define _BB_ZB_PRO_NWK_TX_FSM_H

/************************* INCLUDES *****************************************************/
#include "bbSysTimeoutTask.h"
#include "bbZbProNwkNeighbor.h"
#include "private/bbZbProNwkServices.h"

/************************* DEFINITIONS **************************************************/

/**//**
 * \brief TX FSM Memory descriptor definition.
 */
typedef struct _ZbProTxFsmDescr_t
{
    SYS_QueueDescriptor_t   nwkWaitingForTxQueueDescr;  /*!< Queue declaration for outgoing network packets. */
    SYS_TimeoutTask_t       timeoutTask;                /*!< Internal timer. Is used to perform a delay
                                                             before/after transmission attempt. */
    ZBPRO_NWK_Neighbor_t    *neighbor;                  /*!< Extended state information. */
} ZbProTxFsmDescr_t;

/************************* FUNCTION PROTOTYPES ******************************************/
/*************************************************************************************//**
  \brief Initialization routine for Network TX FSM.
*****************************************************************************************/
NWK_PRIVATE void zbProNwkTxFsmReset(void);

/************************************************************************************//**
  \brief Initializes the network output buffer service fields.
  \param[in] outBuf - pointer to an output buffer structure.
****************************************************************************************/
NWK_PRIVATE void zbProNwkInitializeOutputBuffer(ZbProNwkOutputBuffer_t *const outBuf);

/************************************************************************************//**
  \brief Initiates transmission of the network packet.
  \param[in] outBuf - pointer to an output buffer structure.
****************************************************************************************/
NWK_PRIVATE void zbProNwkTransmit(ZbProNwkOutputBuffer_t *const outBuf);

/************************************************************************************//**
  \brief Cancels transmission associated with a given output buffer.
  \param[in] outBuf - pointer to an output buffer structure.
****************************************************************************************/
INLINE void zbProNwkCancelRequest(ZbProNwkOutputBuffer_t *const outBuf)
{
    // TODO: implement me.
    (void)outBuf;
}
/************************************************************************************//**
  \brief Callback which is called if Tx Status has been updated asynchronously
  \param[in] txStatus - pointer to the TX Status structure.
****************************************************************************************/
NWK_PRIVATE void zbProNwkTxFsmStatusUpdated(ZbProNwkTxStatus_t *const txStatus);

/************************************************************************************//**
  \brief Network transmitter task handler.
  \param[in] taskDescriptor - pointer to the task descriptor which the handler is belonged to.
****************************************************************************************/
NWK_PRIVATE void zbProNwkTxTimeoutTaskHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
  \brief The route record command has been sent.
  \param[in]  outBuf - pointer to an output buffer structure.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkTxFsmRouteRecordIsSent(ZbProNwkOutputBuffer_t *const outBuf);

#endif /* _BB_ZB_PRO_NWK_TX_FSM_H */

/* eof bbZbProNwkTxFsm.h */