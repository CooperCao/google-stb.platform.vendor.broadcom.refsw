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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacFeReqProcScan.h $
*
* DESCRIPTION:
*   MLME-SCAN.request Processor interface.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_FE_REQ_PROC_SCAN_H
#define _BB_MAC_FE_REQ_PROC_SCAN_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacLeRealTimeDisp.h"    /* MAC-LE Real-Time Dispatcher interface. */
#include "private/bbMacCfgFsm.h"            /* MAC layer FSMs integral description. */


/************************* VALIDATIONS **************************************************/
#if defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
# error This header is not for the RF4CE-Controller build.
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts the specified event to be processed with the active MLME-SCAN.request.
 * \param[in]   eventId     Identifier of an event to be accepted by the
 *  MLME-SCAN.request Processor for the currently active request.
 * \param[in]   params      Pointer to auxiliary data passed with the event.
 * \details
 *  This function takes the currently active MLME-SCAN.request and delivers the specified
 *  event with auxiliary data to the MLME-SCAN.request Processor. On such an event the
 *  internal FSM of the Request Processor produces the specified action and switches to
 *  the next state according to its specification.
 * \details
 *  The \p eventId argument specifies the event to be processed by the MLME-SCAN.request
 *  Processor for the currently active MLME-SCAN.request. The MLME-SCAN.request Processor
 *  is able to process the following events:
 *  - START         to start a new request processing. On this event the Request Processor
 *                  validates Request Parameters and decides whether and how to execute
 *                  the new Request,
 *  - TIMEOUT       to get time-out signal to finish the current scan cycle and to start
 *                  the next cycle on the next channel from the set to be scanned,
 *  - COMPLETE      to get confirmation on finished internal requests to the MAC-LE: to
 *                  transmit the Beacon Request MAC Command frame, to perform Energy
 *                  Detection cycle, and to switch the current channel and channel page,
 *  - RECEIVE       to get indication of received Beacon frame from the MAC-LE.
 *
 * \details
 *  The \p params argument, in the case of COMPLETE event, points to the structured
 *  parameters returned by the MAC-LE on the MAC-LE-COMPLETE.response, or, in the case of
 *  RECEIVE event, to the MPDU Surrogate structured object constructed by the MAC-LE
 *  during processing of the MAC-LE-RECEIVE.indication; the \p params argument equals to
 *  NULL in the case of START or TIMEOUT events.
 * \details
 *  This function is called by the MAC-FE Router System for START, COMPLETE and TIMEOUT
 *  events, and by the MAC-FE Indications Dispatcher for RECEIVE event. The START event
 *  is originated by the MAC-FE Requests Dispatcher, COMPLETE and RECEIVE events are
 *  originated by the MAC-LE through the MAC-FE Signal Gates, and the TIMEOUT event is
 *  arranged by this Request Processor to itself and originated by the Symbol Timer.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.1.11.1, 7.1.11.2, 7.5.2.1.
*****************************************************************************************/
MAC_PRIVATE void macFeReqProcScanAcceptEvent(const SYS_FSM_EventId_t       eventId,
                                             MacLeReturnedParams_t *const  params);


/*************************************************************************************//**
 * \brief   Accepts the RESET signal to be processed with the specified MLME-SCAN.request.
 * \param[in]   reqService      Pointer to the service field of the request descriptor of
 *  a particular pending MLME-SCAN.request to be reset.
 * \details
 *  This function finishes the specified pending MLME-SCAN.request without processing and
 *  issues the MLME-SCAN.confirm to the higher layer with RESET status.
 * \details
 *  This function is called by the MAC-FE Requests Dispatcher when performing the
 *  MLME-RESET.request.
*****************************************************************************************/
MAC_PRIVATE void macFeReqProcScanAcceptReset(MacServiceField_t *const reqService);


#endif /* _BB_MAC_FE_REQ_PROC_SCAN_H */