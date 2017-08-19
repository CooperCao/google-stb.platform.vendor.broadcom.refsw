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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      MAC-FE Requests Dispatcher interface.
 *
*******************************************************************************/

#ifndef _BB_MAC_FE_REQ_DISP_H
#define _BB_MAC_FE_REQ_DISP_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacLeRealTimeDisp.h"    /* MAC-LE Real-Time Dispatcher interface. */
#include "private/bbMacMemory.h"            /* MAC Memory interface. */
#include "private/bbMacCfgFsm.h"            /* MAC layer FSMs integral description. */


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts new Requests and Responses received via the MAC-SAP for processing.
 * \param[in]   reqService      Pointer to the newly received Request/Response Descriptor
 *  service field.
 * \details
 *  Puts the new Request/Response into the tail of the MAC-FE Main Requests Queue to be
 *  processed later in the context of MAC tasks (not in the context of the originator
 *  task), and schedules the START task if there is no Active MAC-FE Request/Response
 *  being processed. If a Request/Response is on processing at the moment, then the START
 *  event task for the new Request/Response is not scheduled here, it will be scheduled
 *  when the Currently Active MAC-FE Request/Response finishes.
 * \details
 *  This function also resets transactions flags for the new request/response prior to its
 *  processing by the dedicated MAC-FE Request/Response Processor.
 * \details
 *  This method is intended to be called by the MAC-SAP only. To start processing of the
 *  MLME-BEACON.response use \c macFeReqDispAcceptBeaconsRequest. To restart pending
 *  Request processing use \c macFeReqDispAcceptPendingRequest.
*****************************************************************************************/
MAC_PRIVATE void macFeReqDispAcceptNewRequest(MacServiceField_t *reqService);


#if defined(_MAC_CONTEXT_ZBPRO_)
/*************************************************************************************//**
 * \brief   Accepts pending Requests and Responses received from the MAC-FE Transactions
 *  Dispatcher.
 * \param[in]   reqService      Pointer to the pending Request/Response Descriptor service
 *  field.
 * \details
 *  Puts the activated pending Request/Response into the head of the MAC-FE Main Requests
 *  Queue, and schedules the START task if there is no Active MAC-FE Request/Response
 *  being processed.
 * \details
 *  Indirect Request/Response is put by the corresponding Processor into the tail of the
 *  MAC-FE Pending Requests Queue with the Lifetime timestamp. If such Request/Response is
 *  activated later for transmission, it is transferred to this function.
*****************************************************************************************/
MAC_PRIVATE void macFeReqDispAcceptPendingRequest(MacServiceField_t *reqService);


/*************************************************************************************//**
 * \brief   Accepts expired transactions queue from the MAC-FE Transactions Dispatcher.
 * \param[in]   expiredQueue    Pointer to the expired transactions queue.
 * \details
 *  Appends the expired transactions queue to the tail of the MAC-FE Main Requests Queue,
 *  and schedules the START task if there is no Active MAC-FE Request/Response being
 *  processed.
 * \details
 *  All expired transactions are marked with the ExpiredTransaction flag, which
 *  distinguishes them from new indirect requests and activated indirect requests in the
 *  MAC-FE Main Requests Queue. Expired transactions will be confirmed by their dedicated
 *  MAC-FE Request/Response Processors with TRANSACTION_EXPIRED status.
*****************************************************************************************/
MAC_PRIVATE void macFeReqDispAcceptExpired(const MacMemoryQueueDescr_t *const expiredQueue);
#endif /* _MAC_CONTEXT_ZBPRO_ */


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/*************************************************************************************//**
 * \brief   Accepts new internal Request to transmit Beacons.
 * \param[in]   __givenContextsSet      Set of MAC Contexts that shall transmit their
 *  beacons.
 * \details
 *  Puts one or two MLME-BEACON.response Descriptors into the active state according to
 *  the bitmask \p __givenContextsSet and puts them into the head of the MAC-FE Main
 *  Requests Queue, and schedules the START task if there is no Active MAC-FE
 *  Request/Response being processed.
*****************************************************************************************/
MAC_PRIVATE void macFeReqDispAcceptBeaconsRequest(MAC_WITHIN_GIVEN_CONTEXTS_SET);
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


/*************************************************************************************//**
 * \brief   Accepts and then routes the specified event and its auxiliary data to the
 *  Currently Active MAC-FE Request/Response Processor.
 * \param[in]   eventId     Identifier of an event to be issued to the Currently Active
 *  MAC-FE Request/Response Processor. The following events are supported:
 *  - START     start the Request/Response processing,
 *  - TIMEOUT   signal timeout to the Request/Response Processor,
 *  - COMPLETE  confirms completion of the requested Transmission or ED Scanning.
 *
 * \param[in]   completeParams      Pointer to received parameters of the
 *  MAC-LE-COMPLETE.response for the case of COMPLETE event accepted.
 * \details
 *  Routes the specified event to one of the MAC-FE Request/Response Processors according
 *  to the MAC-SAP Primitive identifier of the Currently Active MAC-FE Request/Response.
*****************************************************************************************/
MAC_PRIVATE void macFeReqDispAcceptEvent(const SYS_FSM_EventId_t                       eventId,
                                         MacLeRealTimeDispCompleteRespParams_t *const  completeParams);


/*************************************************************************************//**
 * \brief   Accepts the finalization signal from the Currently Active MAC-FE Request
 *  Processor.
 * \details
 *  Removes the just finished Request/Response from the MAC-FE Active Request Slot, and
 *  then schedules the START task if there are Requests/Responses waiting in the MAC-FE
 *  Main Requests Queue. If the Queue is empty the task is not scheduled, and in this case
 *  the MAC-FE Requests Dispatcher may be reactivated by one of its accept-request
 *  methods.
 * \details
 *  This method is intended to be called by the Currently Active MAC-FE Request/Response
 *  Processor when it finishes processing (after the confirmation is issued to the higher
 *  layer). This method must be called also after an indirect Request/Response is put from
 *  processing back into the MAC-FE Pending Requests Queue (in the case of unsuccessful
 *  attempt to transmit).
*****************************************************************************************/
MAC_PRIVATE void macFeReqDispFinishProcess(void);


/*************************************************************************************//**
 * \brief   Accepts the reset signal from the currently active MLME-RESET.request
 *  Processor.
 * \details
 *  Parses the MAC-FE Main Requests Queue and confirms all Requests/Responses in it for
 *  the MAC Context that is currently performing the MLME-RESET.request with status RESET.
 *  Then if the currently active MLME-RESET.request is for the ZigBee PRO context, issues
 *  the reset signal to the MAC-FE Transactions Dispatcher.
*****************************************************************************************/
MAC_PRIVATE void macFeReqDispResetActiveContextQueues(void);


#endif /* _BB_MAC_FE_REQ_DISP_H */

/* eof bbMacFeReqDisp.h */