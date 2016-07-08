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
*   RPC Service common definitions.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_RPC_SERVICE_H
#define _BB_RPC_SERVICE_H


/************************* INCLUDES *****************************************************/
#include "bbRpcPredefined.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Data type for unique identifier of a Service.
 */
typedef uint32_t  RpcServiceId_t;


/**//**
 * \brief   Enumeration of Commands that may be requested from an RPC Service by its
 *  parent RPC Dispatcher within the specified RPC Transaction.
 */
typedef enum _RpcServiceCommand_t
{
    /* Common Commands. */

    RPC_COMMAND_INITIALIZE_SERVICE,                 /*!< Perform Service startup initialization. */

    RPC_COMMAND_KILL_TRANSACTION,                   /*!< Kill Transaction processing unconditionally. */

    RPC_COMMAND_PROCESS_LOWERLEVEL_CONFIRM,         /*!< Process transmission confirmation from the lower-level layer. */

    /* Client (transaction originator) side Commands. */

    RPC_COMMAND_COMPOSE_CLIENT_REQUEST,             /*!< Compose client request frame for transmission. */

    RPC_COMMAND_PARSE_RESPONSE_CANDIDATE,           /*!< Parse received server response candidate frame. */

    RPC_COMMAND_PROCESS_TIMEOUT_EXPIRATION,         /*!< Process response waiting timeout expiration. */

    /* Server (transaction recipient) side Commands. */

    RPC_COMMAND_PARSE_CLIENT_REQUEST,               /*!< Parse received client request frame. */

    RPC_COMMAND_COMPOSE_SERVER_RESPONSE,            /*!< Compose server response frame for transmission. */

    /* Server (transaction originator) side Commands for Unsolicited responses. */

    RPC_COMMAND_COMPOSE_UNSOLICITED_RESPONSE,       /*!< Compose server unsolicited response frame for transmission. */

    /* Client (transaction recipient) side Commands for Unsolicited responses. */

    RPC_COMMAND_PARSE_UNSOLICITED_RESPONSE,         /*!< Parse received server unsolicited response frame. */

} RpcServiceCommand_t;


/**//**
 * \brief   Enumeration of Directives that may be returned on Command completion by the
 *  called RPC Service to its parent RPC Dispatcher.
 */
typedef enum _RpcServiceDirective_t
{
    /* Prohibited code. */

    RPC_DIRECTIVE_PROHIBITED_CODE = 0x0,        /*!< This code is used for the shared variable initialization in order
                                                    not to fall into an uninitialized branch. */
    RPC_NO_DIRECTIVE =
            RPC_DIRECTIVE_PROHIBITED_CODE,      /*!< Directive is not specified. */

    /* Common Directives. */

    RPC_DIRECTIVE_PROCEED,                      /*!< Proceed with Transaction execution. */

    RPC_DIRECTIVE_FINISH_TRANSACTION,           /*!< Finish Transaction as soon as possible. */

    RPC_DIRECTIVE_WAIT_SERVICE,                 /*!< Pause processing and wait for signal from this Service. */

    /* Client side Directives. */

    RPC_DIRECTIVE_LOCAL_FAILURE,                /*!< Failed to serve request from the local upper-level layer. */

    RPC_DIRECTIVE_NO_MEMORY,                    /*!< Retry this Command when lack of dynamic memory is removed. */

    RPC_DIRECTIVE_RESTART_TIMEOUT,              /*!< Restart response waiting timeout and proceed with Transaction. */

    RPC_DIRECTIVE_ALIEN_RESPONSE,               /*!< The response candidate turned to be alien to this Transaction. */

    RPC_DIRECTIVE_UNSOLICITED_RESPONSE,         /*!< The response candidate turned to be unsolicited. */

    RPC_DIRECTIVE_BROKEN_RESPONSE,              /*!< The response candidate turned to have invalid format. */

    /* Server side Directives. */

    RPC_DIRECTIVE_MAKE_RESPONSE_NOW,            /*!< Proceed with making response right after request frame parsing */

    /* Aliases for Directives returned by the client side. */

    RPC_DIRECTIVE_WAIT_RESPONSE =
            RPC_DIRECTIVE_PROCEED,              /*!< Proceed with waiting for response{-s} from server. */

    RPC_DIRECTIVE_DONT_WAIT_RESPONSE =
            RPC_DIRECTIVE_FINISH_TRANSACTION,   /*!< Don't wait {more} response{-s}, finish Transaction. */

    RPC_DIRECTIVE_ABORT_REQUEST =               /*!< Abort issuing the request. */
            RPC_DIRECTIVE_LOCAL_FAILURE,

    /* Aliases for Directives returned by the server side. */

    RPC_DIRECTIVE_SUCCESS =
            RPC_DIRECTIVE_PROCEED,              /*!< Completed successfully, proceed with Transaction execution. */

    RPC_DIRECTIVE_MAKE_RESPONSE =
            RPC_DIRECTIVE_PROCEED,              /*!< Proceed with making {next} server response to client. */

    RPC_DIRECTIVE_DONT_MAKE_RESPONSE =
            RPC_DIRECTIVE_FINISH_TRANSACTION,   /*!< Don't make {more} response{-s}, finish Transaction. */

} RpcServiceDirective_t;


/**//**
 * \brief   Data type for RPC Service Handler function provided for processing Commands
 *  from its parent RPC Dispatcher with reference to the specified RPC Transaction.
 * \param[in/out]   transaction     Pointer to a Transaction; or NULL if the specified
 *  Command is addressed to the whole Service.
 * \param[in]       command         Numeric identifier of a Command to be executed.
 * \return  Numeric identifier of a Directive from the Service returned to its Dispatcher.
 */
typedef RpcServiceDirective_t  RpcServiceHandler_t(
                RpcTransaction_t   *const  transaction,
                const RpcServiceCommand_t  command);


/**//**
 * \brief   Structure for descriptor of RPC Service.
 * \details
 *  This structure shall be used as-is; it shall not be embedded into a different RPC
 *  Service descriptor.
 */
struct _RpcService_t
{
    /* 32-bit data. */

    RpcServiceId_t       id;            /*!< Unique identifier of this Service; or 0xFFFFFFFF for the trailing record in
                                            the table or Default Service descriptor. */

    RpcServiceHandler_t *handler;       /*!< Entry point to the Service handler function; or NULL for the trailing
                                            record in the table. */
};


/**//**
 * \brief   Constructs single RPC Service descriptor in the form of RPC Services Table
 *  record.
 * \param[in]   serviceId           Unique identifier of this Service; or 0xFFFFFFFF for
 *  the trailing record in the table or Default Service descriptor.
 * \param[in]   serviceHandler      Entry point to the Service handler function; or NULL
 *  for the trailing record in the table.
 * \return  RPC Services Table record representing an RPC Service descriptor.
 */
#define RPC_SERVICE_DESCRIPTOR(serviceId, serviceHandler)\
        {\
            .id = (serviceId),\
            .handler = (serviceHandler),\
        }


/**//**
 * \brief   Special value for the \c id field of the last descriptor in the RPC Services'
 *  Descriptors table.
 * \details
 *  The last element in the RPC Services' Descriptors table must have the \c id assigned
 *  with this special value having the meaning of either End-of-Table or Default Service.
 * \note
 *  If the \c handler field of the last Descriptor in the Table is not equal to NULL, that
 *  Descriptor is considered as the Default Service descriptor and used to process
 *  received Remote Client Requests addressed to services (clusters) that are not
 *  implemented (i.e., not listed in the Services' Descriptors table). Simultaneously this
 *  record is considered as the End-of-Table record.
 * \note
 *  If the \c handler field of the last Descriptor in the Table is equal to NULL, that
 *  record is considered just as the End-of-Table symbol. No Default Service is provided
 *  in this case; and Remote Client Requests addressed to services (clusters) that are not
 *  implemented (i.e., not listed in the Services' Descriptors table) are just rejected.
 */
#define RPC_SERVICE_ID_LAST_RECORD      0xFFFFFFFF

/*
 * Validate coincidence of sizes of the End-of-Table symbol and the Service Identifier.
 */
SYS_DbgAssertStatic(sizeof(RPC_SERVICE_ID_LAST_RECORD) == sizeof(RpcServiceId_t));


/**//**
 * \brief   Constructs RPC Service descriptor for Default Service.
 * \param[in]   defaultServiceHandler       Entry point to the Default Service handler
 *  function.
 * \return  RPC Services Table record representing the Default Service descriptor.
 * \note
 *  If \p defaultServiceHandler is assigned with NULL, this macro will return the
 *  End-of-Table record.
 */
#define RPC_SERVICE_DESCRIPTOR_DEFAULT(defaultServiceHandler)\
        RPC_SERVICE_DESCRIPTOR(RPC_SERVICE_ID_LAST_RECORD, (defaultServiceHandler))


/**//**
 * \brief   Constructs the RPC Services' Descriptor Table End-of-Table record.
 * \return  RPC Services Table record representing the End-of-Table special record.
 * \note
 *  The End-of-Table record is denoted as the Default Service record but with omitted
 *  (i.e., assigned with NULL) handler.
 */
#define RPC_SERVICE_DESCRIPTOR_EOT\
        RPC_SERVICE_DESCRIPTOR_DEFAULT(NULL)


/**//**
 * \brief   Checks the specified Service Descriptor record if it is the last record in the
 *  RPC Services' Descriptorss Table - i.e., the End-of-Table record or the Default
 *  Service Descriptor record.
 * \param[in]   serviceDescr    Pointer to a Service Descriptor.
 * \return  TRUE if this is the last record; FALSE if this is an ordinary Service
 *  Descriptor record, but not the end of table.
 * \details
 *  The last record has the \c id field set to RPC_SERVICE_ID_LAST_RECORD (0xFFFFFFFF) and
 *  the \c handler field set either to NULL (for the End-of-Table record) or non-NULL (for
 *  the Default Service descriptor record). Ordinary Services' Descriptors have different
 *  \c id value.
 */
#define RPC_IS_SERVICES_TABLE_EOT(serviceDescr)\
        (RPC_SERVICE_ID_LAST_RECORD == (serviceDescr)->id)


/**//**
 * \brief   Returns handler of the specified Service.
 * \param[in]   serviceDescr    Pointer to a Service Descriptor.
 * \return  Entry point to the Service callback handler function; or NULL if no handler
 *  provided for Service (the case of the EOT record).
 */
#define RPC_GET_SERVICE_HANDLER(serviceDescr)\
        ((serviceDescr)->handler)


/**//**
 * \brief   Checks the specified Service Descriptor record if it is the Default Service
 *  Descriptor record or an Ordinary Service Descriptor record.
 * \param[in]   serviceDescr    Pointer to a Service Descriptor.
 * \return  TRUE if this is the Default Service Descriptor record or an Ordinary Service
 *  Descriptor record; FALSE if this is the End-of-Table record.
 * \details
 *  The End-of-Table record has the \c id field set to RPC_SERVICE_ID_LAST_RECORD
 *  (0xFFFFFFFF) and the \c handler field set to NULL. Ordinary Services' Descriptors and
 *  the Default Service Descriptor record have the \c handler field assigned with the
 *  handling function entry point that is not NULL.
 */
#define RPC_IS_VALID_SERVICE_DESCRIPTOR(serviceDescr)\
        (NULL != RPC_GET_SERVICE_HANDLER(serviceDescr))


#endif /* _BB_RPC_SERVICE_H */
