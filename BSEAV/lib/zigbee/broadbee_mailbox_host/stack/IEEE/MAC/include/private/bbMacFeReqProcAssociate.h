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
 *      MLME-ASSOCIATE.request Processor interface.
 *
*******************************************************************************/

#ifndef _BB_MAC_FE_REQ_PROC_ASSOCIATE_H
#define _BB_MAC_FE_REQ_PROC_ASSOCIATE_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapTypesAssociate.h"         /* MLME-ASSOCIATE service data types. */
#include "private/bbMacLeRealTimeDisp.h"    /* MAC-LE Real-Time Dispatcher interface. */
#include "private/bbMacCfgFsm.h"            /* MAC layer FSMs integral description. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This file requires the MAC Context for ZigBee PRO to be included into the build.
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
  \brief
    Accepts the specified event to be processed with the active MLME-ASSOCIATE.request.
  \param    eventId
    Identifier of the event to be accepted by this Request Processor for the currently
    active Request entity and its stored data.
  \param    params
    Pointer to the auxiliary data transmitted with event. For this function this parameter
    is either NULL or pointer to the confirmation/indication parameters from MAC-LE.
  \details
    When called, this function issues corresponding event to this Request Processor
    internal FSM for the active Request. On such an event FSM produces the specified
    action and switches to the next state according to FSM specification.
  \details
    This function is to be called by MAC-FE Router System to deliver events to the active
    Request being processed.
  \details
    This Request Processor accepts the following events with \p eventId:
    \li START       is originated by MAC-FE Requests Dispatcher to start processing of
                    next request/response from the Dispatcher requests queue;
    \li TIMEOUT     is originated by MAC-FE Events System for the timeout previously
                    appointed by this Request Processor to itself;
    \li COMPLETE    is originated by MAC-FE Events System on the task previously scheduled
                    by MAC-FE Confirm Gate on received confirmation from MAC-LE being
                    accomplished the request/response execution. This command has
                    auxiliary data transmitted into this Request Processor via
                    \p unionConfInd.confParams - MAC-LE Confirmation parameters;
    \li RECEIVE     is originated by MAC-FE Events System on the task previously scheduled
                    by MAC-FE Indication Gate on received indication from MAC-LE upon
                    reception of an MPDU. This command has auxiliary data transmitted into
                    this Request Processor via \p unionConfInd.indMpduSurr - MAC-LE
                    Indication MPDU surrogate.
*****************************************************************************************/
MAC_PRIVATE void macFeReqProcAssociateAcceptEvent(const SYS_FSM_EventId_t eventId, MacLeReturnedParams_t *const params);


/*************************************************************************************//**
 * \brief   Accepts the RESET signal to be processed with the specified
 *  MLME-ASSOCIATE.request.
 * \param[in]   reqService      Pointer to the service field of the request descriptor of
 *  a particular pending MLME-ASSOCIATE.request to be reset.
 * \details
 *  This function finishes the specified pending MLME-ASSOCIATE.request without processing
 *  and issues the MLME-ASSOCIATE.confirm to the higher layer with RESET status.
 * \details
 *  This function is called by the MAC-FE Requests Dispatcher when performing the
 *  MLME-RESET.request.
*****************************************************************************************/
MAC_PRIVATE void macFeReqProcAssociateAcceptReset(MacServiceField_t *const reqService);


#endif /* _BB_MAC_FE_REQ_PROC_ASSOCIATE_H */

/* eof bbMacFeReqProcAssociate.h */