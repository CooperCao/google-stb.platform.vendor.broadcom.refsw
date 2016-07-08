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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacFeReqProcPurge.h $
*
* DESCRIPTION:
*   MCPS-PURGE.request Processor interface.
*
* $Revision: 2999 $
* $Date: 2014-07-21 13:30:43Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_FE_REQ_PROC_PURGE_H
#define _BB_MAC_FE_REQ_PROC_PURGE_H


/************************* INCLUDES *****************************************************/
#include "bbMacSapService.h"        /* MAC-SAP service data types. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This header shall be compiled only if the ZigBee PRO context is included into the build.
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts the START event to be processed with the active MCPS-PURGE.request.
 * \details
 *  This function takes the currently active MCPS-PURGE.request and delivers the START
 *  event to this Request Processor. On such an event the internal FSM of the Request
 *  Processor produces the specified action and switches to the next state according to
 *  its specification.
 * \details
 *  This Request Processor is able to process the following events:
 *  - START     to start a new request processing. On this event the Request Processor
 *      validates Request Parameters and decides whether and how to execute the new
 *      Request.
 *
 * \details
 *  This function is called by the MAC-FE Router System. The START event is originated by
 *  the MAC-FE Requests Dispatcher.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 7.1.1.4.
*****************************************************************************************/
MAC_PRIVATE void macFeReqProcPurgeAcceptStartEvent(void);


/*************************************************************************************//**
 * \brief   Accepts the RESET signal to be processed with the specified
 *  MCPS-PURGE.request.
 * \param[in]   reqService      Pointer to the service field of the request descriptor of
 *  a particular pending MCPS-PURGE.request to be reset.
 * \details
 *  This function finishes the specified pending MCPS-PURGE.request without processing and
 *  issues the MCPS-PURGE.confirm to the higher layer with RESET status.
 * \details
 *  This function is called by the MAC-FE Requests Dispatcher when performing the
 *  MLME-RESET.request.
*****************************************************************************************/
MAC_PRIVATE void macFeReqProcPurgeAcceptReset(MacServiceField_t *const reqService);


#endif /* _BB_MAC_FE_REQ_PROC_PURGE_H */