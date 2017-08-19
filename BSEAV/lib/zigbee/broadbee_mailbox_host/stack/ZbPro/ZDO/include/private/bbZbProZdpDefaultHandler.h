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
*   ZDP Default Service private interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZDP_DEFAULT_HANDLER_H
#define _BB_ZBPRO_ZDP_DEFAULT_HANDLER_H


/************************* INCLUDES *****************************************************/
#include "bbRpc.h"
#include "bbZbProZdoCommon.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the Default ZDO Response.
 * \details
 *  This structure involves only one field - destination address substructure - that is
 *  assigned with the source address of the received unsupported request. It is used by
 *  the Default Service to hold address of the remote device to which it shall send the
 *  default response with the NOT_SUPPORTED status.
 */
typedef struct _ZbProZdoDefaultRespParams_t
{
    /* Structured data, aligned at 32 bits. */

    ZBPRO_ZDO_Address_t  zdpDstAddress;         /*!< Destination address. It is assigned with the unicast source address
                                                    of the device that originated the request that is not supported on
                                                    this node. */
} ZbProZdoDefaultRespParams_t;


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Processes Commands from ZDP Dispatcher with reference to the specified ZDP
 *  Server Transaction in the case when the requested command is not implemented.
 * \param[in/out]   rpcTransaction      Pointer to a Server Transaction; or NULL if the
 *  specified Command is addressed to the whole Service.
 * \param[in]       rpcCommand          Numeric identifier of a Command to be executed.
 * \return  Numeric identifier of a Directive from the Service returned to its Dispatcher.
 * \note
 *  This Default Service is able to serve only the Server side transactions assigned to
 *  remote requests received via the media; it doesn't serve the application requests.
 */
RpcServiceDirective_t zbProZdpDefaultServiceHandler(
                RpcTransaction_t   *const  rpcTransaction,
                const RpcServiceCommand_t  rpcCommand);


#endif /* _BB_ZBPRO_ZDP_DEFAULT_HANDLER_H */

/* eof bbZbProZdpDefaultHandler.h */