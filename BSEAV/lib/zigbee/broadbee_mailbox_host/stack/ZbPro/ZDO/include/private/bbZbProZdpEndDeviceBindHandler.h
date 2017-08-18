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
*       ZDP End_Device_Bind Service private interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZDP_END_DEVICE_BIND_HANDLER_H
#define _BB_ZBPRO_ZDP_END_DEVICE_BIND_HANDLER_H


/************************* INCLUDES *****************************************************/
#include "private/bbZbProZdpTransaction.h"


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Processes Commands from ZDP Dispatcher with reference to the specified ZDP
 *  End_Device_Bind Service Transaction.
 * \param[in/out]   rpcTransaction      Pointer to a Transaction; or NULL if the specified
 *  Command is addressed to the whole Service.
 * \param[in]       rpcCommand          Numeric identifier of a Command to be executed.
 * \return  Numeric identifier of a Directive from the Service returned to its Dispatcher.
 */
RpcServiceDirective_t zbProZdpEndDeviceBindServiceHandler(
                RpcTransaction_t   *const  rpcTransaction,
                const RpcServiceCommand_t  rpcCommand);


/**//**
 * \brief   Accepts local response from ZDO End Device Bind Manager to proceed with the
 *  specified ZDP End_Device_Bind Service Transaction.
 * \param[in]   zdpTransaction      Pointer to the Transaction to proceed with.
 * \note
 *  The ZDO End Device Bind Manager shall call this function in a different execution flow
 *  with that in which it was called by the ZDP End_Device_Bind Service Handler.
 * \details
 *  All the data necessary for composing the ZDO End_Device_Bind Server Response may be
 *  accessed via the specified \p zdpTransaction.
 */
void zbProZdpEndDeviceBindManagerResp(
                ZbProZdpTransaction_t *const  zdpTransaction);


#endif /* _BB_ZBPRO_ZDP_END_DEVICE_BIND_HANDLER_H */

/* eof bbZbProZdpEndDeviceBindHandler.h */