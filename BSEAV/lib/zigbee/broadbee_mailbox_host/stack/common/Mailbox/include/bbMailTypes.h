/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: trunk/stack/common/Mailbox/include/bbMailTypes.h $
*
* DESCRIPTION:
*       mailbox types declaration.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/

#ifndef _MAIL_TYPES_H
#define _MAIL_TYPES_H

/************************* INCLUDES ****************************************************/
#include "bbSysBasics.h"
#include "bbSysDbg.h"
#include "bbSysPayload.h"
#include "bbSysTimeoutTask.h"
#include "bbHalSystemTimer.h" // NOTE: system time

/************************* DEFINITIONS *************************************************/
#define MAIL_INVALID_PAYLOAD_OFFSET 0xFFU

/**//**
 * \brief Mailbox delivery time.
 */
#define MAIL_CLIENT_DELIVERY_TIME_MS 400
/************************* TYPES *******************************************************/
/**//**
 * \brief Acknowledgeable status enumeration.
 */
typedef enum _MailState_t
{
    MAIL_PARCEL_AWAIT_FOR_SEND = 0x00U,
    MAIL_ACK_AWAIT_FOR_SEND,
    MAIL_ACK_NOT_NEEDED,
    MAIL_WAITING_FOR_ACK,
    MAIL_ACK_SENT,
    MAIL_WAITING_FOR_CONF,
} MailState_t;

typedef enum _MailAckStatus_t
{
    MAIL_SUCCESSFUL_RECEIVED = 0x00U,
} MailAckStatus_t;

/**//**
 * \brief Enumeration of subsystem codes for shared FIFO.
 */
/* TODO: move to the HAL */
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
 * \brief message direction codes enumeration for shared FIFO.
 */
typedef enum
{
    REQUEST_MSG_TYPE    = 0,
    INDICATION_MSG_TYPE = 1,
    RESPONSE_MSG_TYPE   = 2,
    CONFIRM_MSG_TYPE    = 3,
} MailFifoMessageDirection_t;

/**//**
 * \brief message type codes enumeration.
 */
typedef struct _MailFifoMessageType_t
{
    uint8_t version     : 3; /*!< the protocol version */
    uint8_t subSystem   : 3; /*!< message destination/ source ID */
    uint8_t type        : 2; /*!< type of message (req, conf, ind, resp) */
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
            uint32_t messageType        : 2;
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
    uint8_t dataLength;             /*!< Offset of payload length */
    uint8_t dataPointerOffset;      /*!< Offset of pointer to data field in raw data of request */
} MailWrappedReqHeader_t;

/**//**
 * \brief Type of pointer to the wrapper function.
 */
typedef void (* MailPublicFunction_t)(void *req);

/**//**
 * \brief confirmation callback type.
 */
typedef void(* ConfirmCall_t)(void *req, void *confirm);

/**//**
* \brief predefinition of mail service descriptor
*/
typedef struct _MailDescriptor_t MailDescriptor_t;

/**//**
 * \brief syncronization callback type.
 */
typedef uint16_t MailCheckSumLength_t;

#endif /* _MAIL_TYPES_H */
/* eof bbMailTypes.h */