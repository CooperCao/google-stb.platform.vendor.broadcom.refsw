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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacFeRespProcBeacon.h $
*
* DESCRIPTION:
*   MLME-BEACON.response Processor interface.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_FE_RESP_PROC_BEACON_H
#define _BB_MAC_FE_RESP_PROC_BEACON_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapTypesBeacon.h"            /* MLME-BEACON service data types. */
#include "private/bbMacLeRealTimeDisp.h"    /* MAC-LE Real-Time Dispatcher interface. */
#include "private/bbMacCfgFsm.h"            /* MAC layer FSMs integral description. */


/************************* VALIDATIONS **************************************************/
#if defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
# error This header is not for the RF4CE-Controller build.
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts the specified event to be processed with the active
 *  MLME-BEACON.response.
 * \param[in]   eventId     Identifier of an event to be accepted by the
 *  MLME-BEACON.response Processor for the currently active response.
 * \param[in]   params      Pointer to auxiliary data passed with the event.
 * \details
 *  This function takes the currently active MLME-BEACON.response and delivers the
 *  specified event with auxiliary data to the MLME-BEACON.response Processor. On such an
 *  event the internal FSM of the Response Processor produces the specified action and
 *  switches to the next state according to its specification.
 * \details
 *  The \p eventId argument specifies the event to be processed by the
 *  MLME-BEACON.response Processor for the currently active MLME-BEACON.response. The
 *  MLME-BEACON.response Processor is able to process following events:
 *  - START         to start a new response processing. On this event the Response
 *                  Processor verifies whether it is allowed to send a beacon (i.e., if
 *                  this devices is in a PAN or not),
 *  - COMPLETE      to get confirmation on finished internal request to the MAC-LE to
 *                  transmit the Beacon frame.
 *
 * \details
 *  The \p confParams argument points to the MAC-LE Confirmation Parameters object when
 *  this function is called for the CONFIRM event; and it must equals NULL for START
 *  event. This argument is used only in the MAC Tester build configuration to construct
 *  the MLME-BEACON.confirm primitive parameters; this argument is omitted in the
 *  conventional build configuration because there is no MLME-BEACON.confirm primitive
 *  and the macBeaconTxTime attribute is not implemented for nonbeacon-enabled PANs.
 * \details
 *  This function is called by the MAC-FE Router System for START and CONFIRM events. The
 *  START event is originated by the MAC-FE Requests Dispatcher when a Beacon Request MAC
 *  Command is received, and the CONFIRM event is originated by the MAC-LE through the
 *  MAC-FE Confirmation Signal Gate.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.5.2.4.
*****************************************************************************************/
MAC_PRIVATE void macFeRespProcBeaconAcceptEvent(const SYS_FSM_EventId_t       eventId
#if defined(_MAC_TESTER_)
                                              , MacLeReturnedParams_t *const  params
#endif
                                                                                    );


/*************************************************************************************//**
 * \brief   Accepts the RESET signal to be processed with the specified
 *  MLME-BEACON.response.
 * \param[in]   respService     Pointer to the service field of the response descriptor of
 *  a particular pending MLME-BEACON.response to be reset.
 * \details
 *  This function finishes the specified pending MLME-BEACON.response without processing.
 *  In the case of MAC Tester build configuration it also issues the MLME-BEACON.confirm
 *  to the higher layer with RESET status.
 * \details
 *  This function is called by the MAC-FE Requests Dispatcher when performing the
 *  MLME-RESET.request.
*****************************************************************************************/
MAC_PRIVATE void macFeRespProcBeaconAcceptReset(MacServiceField_t *const respService);


#endif /* _BB_MAC_FE_RESP_PROC_BEACON_H */