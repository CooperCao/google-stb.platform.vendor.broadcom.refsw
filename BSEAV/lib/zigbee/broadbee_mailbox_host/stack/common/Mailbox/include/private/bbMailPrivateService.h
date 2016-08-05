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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/include/private/bbMailPrivateService.h $
*
* DESCRIPTION:
*      private declaration of the mailbox service module.
*
* $Revision: 2000 $
* $Date: 2014-03-31 15:34:06Z $
*
****************************************************************************************/

#ifndef _MAILPRIVATE_SERVICE_H
#define _MAILPRIVATE_SERVICE_H

/************************* INCLUDES ****************************************************/
#include "bbMailService.h"
#include "bbMailAPI.h"

/************************* DEFINITIONS *************************************************/
#define MAILBOX_API_VERSION 1

#define MAIL_INVALID_OFFSET 0xFFU

/**//**
 * \brief Mailbox delivery time.
 */
#define MAIL_CLIENT_DELIVERY_TIME_MS 400

/**//**
 * \brief number of pending client requests
 */
#define MAIL_CLIENT_MAX_AMOUNT_PENDING_CALLS 15

/**//**
 * \brief server buffers amount.
 */
#define MAIL_SERVER_MAX_AMOUNT_PROCESSED_REQUESTS 5

/************************* TYPES *******************************************************/
/**//**
 * \brief Enumeration of subsystem codes for shared FIFO.
 */
typedef enum
{
    UART    = 0,
    HOST0   = 1,
    HOST1   = 2,
    HOST2   = 3,
    LASTSUBSYSTEMID
} MailFifoSubsystemId_t;
SYS_DbgAssertStatic(LASTSUBSYSTEMID <= 7);

/**//**
 * \brief message type codes enumeration.
 */
typedef struct _MailFifoMessageType_t
{
    uint8_t version         : 3; /*!< the protocol version */
    uint8_t subSystem       : 3; /*!< message destination/ source ID */
    uint8_t fromStackSide   : 1; /*!< lo bit from message type field. It set for "request" and "indication" types */
    uint8_t isConfirm       : 1; /*!< hi bit from message type field. It set for "confirm" and "response" types */
} MailFifoMessageType_t;

/**//**
 * \brief Shared FIFO header type.
 */
typedef struct _MailFifoHeader_t
{
    MailFifoMessageType_t msgType;
    uint16_t msgId;                     /*!< current message id (valid only 10 bits) */
    bool isFragment;                    /*!< true if the message is to be continued */
    uint8_t fragmentNumber;             /*!< fragment number */
} MailFifoHeader_t;

/**//**
 * \brief packed FIFO header type
 */
typedef struct _MailFifoPackedHeader_t
{
    union
    {
        struct
        {
            uint32_t messageLength      : 5;
            uint32_t protocolVersion    : 3;
            uint32_t sequenceNumber     : 8;
            uint32_t fromStackSide      : 1;
            uint32_t isConfirm          : 1;
            uint32_t messageId          : 10;
            uint32_t fragment           : 1;
            uint32_t subSystemId        : 3;
        };
        uint8_t part[sizeof(uint32_t)];
    };
} MailFifoPackedHeader_t;

/**//**
 * \brief Type of parcel's header
 */
typedef struct _MailWrappedReqHeader_t
{
    uint8_t uId;                    /*!< request unique ID */
    uint8_t paramLength;            /*!< Wrapped request length */
    uint16_t dataLength;            /*!< Offset of payload length */
} MailWrappedReqHeader_t;

/**//**
 * \brief syncronization callback type.
 */
typedef uint16_t MailCheckSumLength_t;

/**//**
 * \brief Union of all service parameters.
 */
typedef union _MailServiceReq_t
{
    TE_MailboxAckDescr_t        ack;
} MailServiceReq_t;

typedef union _MailServiceConfParams_t
{
    TE_MailboxAckConfParams_t   ack;
} MailServiceConfParams_t;

/**//**
 * \brief Union of all request descriptors.
 */
/* NOTE: used only on server side. Can be move to bbMailServerTable.h */
typedef union _MailReq_t
{
#define MAKE_REQUEST_DSCRIPTOR_LINE__NO_CB(function_name, desc_name)
#define MAKE_REQUEST_DSCRIPTOR_LINE__HAS_CB(function_name, desc_name) desc_name function_name;


#define ANY_DIRECTION(function_name, enum_name, desc_mod, desc_name, params_mod, parameters_name, param_payload, confirm_mod, confirm_name, confirm_payload) \
    MAKE_REQUEST_DSCRIPTOR_LINE__##desc_mod(function_name, desc_name)
#define HOST_TO_STACK ANY_DIRECTION
#define STACK_TO_HOST ANY_DIRECTION
#include "bbMailFunctionList.h"


#undef MAKE_REQUEST_DSCRIPTOR_LINE__HAS_CB
#undef MAKE_REQUEST_DSCRIPTOR_LINE__NO_CB
} MailReq_t;


/**//**
 * \brief Union of all request parameters.
 */
typedef union _MailReqParams_t
{
#define MAKE_REQUEST_PARAMETERS_LINE__NO_CB__NO_PARAMS(function_name, parameters_name)
#define MAKE_REQUEST_PARAMETERS_LINE__NO_CB__NO_DATA(function_name, parameters_name)
#define MAKE_REQUEST_PARAMETERS_LINE__NO_CB__HAS_DATA(function_name, parameters_name)
#define MAKE_REQUEST_PARAMETERS_LINE__HAS_CB__NO_PARAMS(function_name, parameters_name)
#define MAKE_REQUEST_PARAMETERS_LINE__HAS_CB__NO_DATA(function_name, parameters_name)   parameters_name function_name;
#define MAKE_REQUEST_PARAMETERS_LINE__HAS_CB__HAS_DATA(function_name, parameters_name)  parameters_name function_name;


#define ANY_DIRECTION(function_name, enum_name, desc_mod, desc_name, params_mod, parameters_name, param_payload, confirm_mod, confirm_name, confirm_payload) \
    MAKE_REQUEST_PARAMETERS_LINE__##desc_mod##__##params_mod(function_name, parameters_name)
#define HOST_TO_STACK ANY_DIRECTION
#define STACK_TO_HOST ANY_DIRECTION
#include "bbMailFunctionList.h"


#undef MAKE_REQUEST_PARAMETERS_LINE__HAS_CB__HAS_DATA
#undef MAKE_REQUEST_PARAMETERS_LINE__HAS_CB__NO_DATA
#undef MAKE_REQUEST_PARAMETERS_LINE__HAS_CB__NO_PARAMS
#undef MAKE_REQUEST_PARAMETERS_LINE__NO_CB__HAS_DATA
#undef MAKE_REQUEST_PARAMETERS_LINE__NO_CB__NO_DATA
#undef MAKE_REQUEST_PARAMETERS_LINE__NO_CB__NO_PARAMS
} MailReqParams_t;


/**//**
 * \brief Union of all confirm parameters.
 */
typedef union _MailConfParams_t
{
#define MAKE_CONFIRMATION_PARAMETERS_LINE__NO_CONFIRM(function_name, confirm_name)
#define MAKE_CONFIRMATION_PARAMETERS_LINE__NO_DATA(function_name, confirm_name)     confirm_name function_name;
#define MAKE_CONFIRMATION_PARAMETERS_LINE__HAS_DATA(function_name, confirm_name)    confirm_name function_name;


#define ANY_DIRECTION(function_name, enum_name, desc_mod, desc_name, params_mod, parameters_name, param_payload, confirm_mod, confirm_name, confirm_payload) \
    MAKE_CONFIRMATION_PARAMETERS_LINE__##confirm_mod(function_name, confirm_name)
#define HOST_TO_STACK ANY_DIRECTION
#define STACK_TO_HOST ANY_DIRECTION
#include "bbMailFunctionList.h"


#undef MAKE_CONFIRMATION_PARAMETERS_LINE__HAS_DATA
#undef MAKE_CONFIRMATION_PARAMETERS_LINE__NO_DATA
#undef MAKE_CONFIRMATION_PARAMETERS_LINE__NO_CONFIRM
} MailConfParams_t;

/**//**
 * \brief Union of all indication parameters.
 */
typedef union _MailIndParams_t
{
#define MAKE_INDICATION_PARAMETERS_LINE__NO_CB__NO_PARAMS(function_name, parameters_name)
#define MAKE_INDICATION_PARAMETERS_LINE__NO_CB__NO_DATA(function_name, parameters_name)     parameters_name function_name;
#define MAKE_INDICATION_PARAMETERS_LINE__NO_CB__HAS_DATA(function_name, parameters_name)    parameters_name function_name;
#define MAKE_INDICATION_PARAMETERS_LINE__HAS_CB__NO_PARAMS(function_name, parameters_name)
#define MAKE_INDICATION_PARAMETERS_LINE__HAS_CB__NO_DATA(function_name, parameters_name)
#define MAKE_INDICATION_PARAMETERS_LINE__HAS_CB__HAS_DATA(function_name, parameters_name)


#define ANY_DIRECTION(function_name, enum_name, desc_mod, desc_name, params_mod, parameters_name, param_payload, confirm_mod, confirm_name, confirm_payload) \
    MAKE_INDICATION_PARAMETERS_LINE__##desc_mod##__##params_mod(function_name, parameters_name)
#define HOST_TO_STACK ANY_DIRECTION
#define STACK_TO_HOST ANY_DIRECTION
#include "bbMailFunctionList.h"


#undef MAKE_INDICATION_PARAMETERS_LINE__HAS_CB__HAS_DATA
#undef MAKE_INDICATION_PARAMETERS_LINE__HAS_CB__NO_DATA
#undef MAKE_INDICATION_PARAMETERS_LINE__HAS_CB__NO_PARAMS
#undef MAKE_INDICATION_PARAMETERS_LINE__NO_CB__HAS_DATA
#undef MAKE_INDICATION_PARAMETERS_LINE__NO_CB__NO_DATA
#undef MAKE_INDICATION_PARAMETERS_LINE__NO_CB__NO_PARAMS
} MailIndParams_t;

#endif /* _MAILPRIVATE_SERVICE_H */
/* eof bbMailPrivateService.h */
