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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   ZDO End Device Bind Manager private interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_ZBPRO_ZDO_END_DEVICE_BIND_MANAGER_H
#define _BB_ZBPRO_ZDO_END_DEVICE_BIND_MANAGER_H


/************************* INCLUDES *****************************************************/
#include "private/bbZbProZdpTransaction.h"


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Accepts local indication of new received Remote ZDO End_Device_Bind Request.
 * \param[in]   zdpTransaction      Pointer to ZDP Transaction with received request.
 * \return  TRUE if request was processed synchronously; FALSE if request may not be fully
 *  processed synchronously and will be processed asynchronously.
 * \details
 *  This function delivers new received End_Device_Bind request to the End Device Bind
 *  Manager for processing. If Manager is currently busy with previously started binding
 *  procedure, it will finish this request processing synchronously with the NO_MATCH
 *  status (which is reported back in its ZDP Transaction), returning TRUE. If Manager
 *  encounters no matched clusters between two requests indicated to it (this request is
 *  the second one in this case), it will finish the first request asynchronously with
 *  calling of dedicated response function for its ZDP Transaction and it will finish this
 *  (the second) request synchronously returning TRUE; for both requests statuses of their
 *  ZDP Transactions will be assigned with NO_MATCH. If this is the first request
 *  indicated to MAnager, or if it is the second one and there is at least one match
 *  between clusters of two requests, in both cases this request will be put on
 *  asynchronous processing; this function will return FALSE (i.e., asynchronous
 *  processing) and ZDP Transaction assigned for this request will be activated later by
 *  Manager with call of dedicated response function; status will be reported back in this
 *  request ZDP Transaction parameters.
 * \details
 *  If this function returns TRUE, the End Device Bind Service Handler (the caller) shall
 *  proceed this ZDP Transaction execution. If this function returns FALSE, it shall put
 *  this transaction on hold with directive WAIT_SERVICE returned to the RPC Dispatcher.
 * \details
 *  Parameters of the received Remote ZDO Request may be accessed via the specified
 *  \p zdpTransaction.
 */
bool zbProZdoEndDeviceBindManagerInd(
                ZbProZdpTransaction_t *const  zdpTransaction);


#endif /* _BB_ZBPRO_ZDO_END_DEVICE_BIND_MANAGER_H */
