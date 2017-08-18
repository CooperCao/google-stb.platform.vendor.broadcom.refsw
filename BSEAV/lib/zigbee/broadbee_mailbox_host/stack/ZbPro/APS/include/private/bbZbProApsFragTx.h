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

/****************************************************************************************
 *
 * DESCRIPTION:
 *      Declaration of the ZigBee PRO APS Frag Tx component
 *
 ****************************************************************************************/

#ifndef _ZBPRO_APS_FRAG_TX_H
#define _ZBPRO_APS_FRAG_TX_H

/************************* INCLUDES ****************************************************/
#include "bbZbProApsCommon.h"
#include "bbZbProApsTx.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Frag Tx descriptor
 */
typedef struct _ZbProApsFragTxDesc_t
{
    SYS_QueueDescriptor_t   queue;
    SYS_TimeoutTask_t       timeoutTask;
} ZbProApsFragTxDesc_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
  \brief Request function to send fragmented or non-fragmented APS transaction

  \param[in] buf - pointer to a Tx buffer
****************************************************************************************/
APS_PRIVATE void zbProApsFragTxReq(ZbProApsTxBuffer_t *buf);

/************************************************************************************//**
  \brief To ensure that changing Fragmentation threshold or windows size is safe at the moment

  \returns true, if at least one fragmented transaction is in progress
****************************************************************************************/
APS_PRIVATE bool zbProApsFragTxIsRunning(void);

/************************************************************************************//**
  \brief Has to be called on receiving extended ACK

  \param[in] ackWindowPosition - blockNumber field value of the Extended Ack
****************************************************************************************/
APS_PRIVATE void zbProApsFragTxGotAck(ZbProApsCounter_t apsCounter,
        ZBPRO_APS_ClusterId_t clusterId, ZBPRO_APS_ProfileId_t profileId,
        ZBPRO_APS_EndpointId_t dstEndpoint, ZBPRO_APS_EndpointId_t srcEndpoint,
        uint8_t ackWindowPosition, uint8_t ackAckMask);

/************************************************************************************//**
 \brief APS Frag Tx callback function. To be called by lower APS Tx FSM on zbProApsTxReq

 \param[in] buf - Tx buffer
 ****************************************************************************************/
APS_PRIVATE void zbProApsFragTxConf(ZbProApsTxBuffer_t *buf);

/************************************************************************************//**
 \brief APS Frag Tx Task Handler
 ****************************************************************************************/
APS_PRIVATE void zbProApsFragTxFsmHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************************************************************************//**
 \brief APS Frag Tx Initialization function
 ****************************************************************************************/
APS_PRIVATE void zbProApsFragTxReset(void);

#endif /* _ZBPRO_APS_FRAG_TX_H */
