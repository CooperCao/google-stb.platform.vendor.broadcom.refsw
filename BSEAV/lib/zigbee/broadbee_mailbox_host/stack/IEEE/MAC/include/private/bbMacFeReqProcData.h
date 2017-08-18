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
 *      MCPS-DATA.request Processor interface.
 *
*******************************************************************************/

#ifndef _BB_MAC_FE_REQ_PROC_DATA_H
#define _BB_MAC_FE_REQ_PROC_DATA_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacLeRealTimeDisp.h"    /* MAC-LE Real-Time Dispatcher interface. */
#include "private/bbMacCfgFsm.h"            /* MAC layer FSMs integral description. */


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts the specified event to be processed with the active MCPS-DATA.request.
 * \param[in]   eventId     Identifier of an event to be accepted by this Request
 *  Processor for the currently active request.
 * \param[in]   params      Pointer to auxiliary data passed with the event.
 * \details
 *  This function takes the currently active MCPS-DATA.request and delivers the specified
 *  event with auxiliary data to this Request Processor. On such an event the internal FSM
 *  of the Request Processor produces the specified action and switches to the next state
 *  according to its specification.
 * \details
 *  The \p eventId argument specifies the event to be processed by this Request Processor
 *  for the currently active MCPS-DATA.request. This Request Processor is able to process
 *  the following events:
 *  - START         to start a new request processing. On this event the Request Processor
 *                  validates Request Parameters and decides whether and how to execute
 *                  the new Request,
 *  - COMPLETE      to get confirmation on finished internal requests to the MAC-LE: to
 *                  perform data transmission.
 *
 * \details
 *  The \p params argument, in the case of COMPLETE event, points to the structured
 *  parameters returned by the MAC-LE on the MAC-LE-COMPLETE.response; the \p params
 *  argument equals to NULL in the case of START event.
 * \details
 *  This function is called by the MAC-FE Router System for START and COMPLETE events. The
 *  START event is originated by the MAC-FE Requests Dispatcher, and the COMPLETE event is
 *  originated by the MAC-LE through the MAC-FE Signal Gates.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.1.1.1, 7.1.1.2, 7.5.5.
*****************************************************************************************/
MAC_PRIVATE void macFeReqProcDataAcceptEvent(const SYS_FSM_EventId_t eventId, MacLeReturnedParams_t *const params);


/*************************************************************************************//**
 * \brief   Accepts the RESET signal to be processed with the specified MCPS-DATA.request.
 * \param[in]   reqService      Pointer to the service field of the request descriptor of
 *  a particular pending MCPS-DATA.request to be reset.
 * \details
 *  This function finishes the specified pending MCPS-DATA.request without processing and
 *  issues the MCPS-DATA.confirm to the higher layer with RESET status.
 * \details
 *  This function is called by the MAC-FE Requests Dispatcher when performing the
 *  MLME-RESET.request.
*****************************************************************************************/
MAC_PRIVATE void macFeReqProcDataAcceptReset(MacServiceField_t *const reqService);


/*************************************************************************************//**
 * \brief   Accepts the PURGE signal to be processed with the specified MCPS-DATA.request.
 * \param[in]   reqService      Pointer to the service field of the request descriptor of
 *  a particular pending MCPS-DATA.request to be reset.
 * \details
 *  This function finishes the specified pending MCPS-DATA.request without processing and
 *  issues the MCPS-DATA.confirm to the higher layer with PURGED status.
 * \details
 *  This function is called by the MAC-FE Transactions Dispatcher when performing the
 *  MCPS-PURGE.request.
*****************************************************************************************/
MAC_PRIVATE void macFeReqProcDataAcceptPurge(MacServiceField_t *const reqService);


/*************************************************************************************//**
 * \brief   Constructs MPDU Surrogate structure for direct Data frame.
 * \param[in]   mpduSurr    Pointer to an empty MPDU Surrogate to be filled.
 * \details
 *  This function is a callback that is called by the MAC-LE when it is ready to start the
 *  MPDU construction just before the frame transmission. When called it constructs the
 *  MPDU Surrogate structure of the frame to be transmitted according to the active
 *  Request parameters.
 * \details
 *  The MAC-LE caller is responsible for allocating memory for the MPDU Surrogate being
 *  constructed. Pointer to such a placeholder is specified with \p mpduSurr by the MAC-LE
 *  to this function.
 * \details
 *  This function was moved to PRIVATE section from STATIC in order to be accessible
 *  directly for RF4CE Controller build.
*****************************************************************************************/
MAC_PRIVATE void macFeReqProcDataConstructDataDirectMpdu(MacMpduSurr_t *const mpduSurr);


#endif /* _BB_MAC_FE_REQ_PROC_DATA_H */

/* eof bbMacFeReqProcData.h */