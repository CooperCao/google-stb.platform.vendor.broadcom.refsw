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
 *
 *
*******************************************************************************/

#ifndef _MAIL_TESTENGINE_H
#define _MAIL_TESTENGINE_H

#include "bbSysTypes.h"
#include "bbSysPayload.h"
#include "bbMailService.h"

/************************* TYPES *******************************************************/
#ifdef _MAILBOX_WRAPPERS_TEST_ENGINE_
/**//**
 * \Brief firmware type descriptor
*/
typedef struct _FirmwareType_t{
    uint16_t firmareStacks;  // Now 4 bits used, bit mask
    uint16_t firmwareMailboxLevels;  // Bit mask. Mailbox TE is defined as ON always.
    uint16_t firmwareFeatures;  // 2 bits are really used now, bit mask
    uint16_t firmwareProfiles;  // bit mask, 6 profiles are already reserved. can be extended, if need.
} FirmwareType_t;

/**//**
 * \brief command status code enumeration
 */
typedef enum
{
    INVALID_STATUS = 0x00U,
    OK_STATUS,
} TE_ConfirmStatus_t;

/**//**
 * \brief Ping command confirmation type
 */
typedef FirmwareType_t TE_PingCommandConfParams_t;

/**//**
 * \brief Ping command request type
 */
typedef struct _TE_PingCommandReqDescr_t TE_PingCommandReqDescr_t;
typedef struct _TE_PingCommandReqDescr_t
{
    void (*callback)(TE_PingCommandReqDescr_t *, TE_PingCommandConfParams_t *);
} TE_PingCommandReqDescr_t;

/**//**
 * \brief Reset command parameters type
 */
typedef struct _TE_ResetCommandReqParams_t
{
    uint32_t resetType;
} TE_ResetCommandReqParams_t;

/**//**
 * \brief Reset command request type
 */
typedef struct _TE_ResetCommandReqDescr_t
{
    TE_ResetCommandReqParams_t params;
} TE_ResetCommandReqDescr_t;

/**//**
 * \brief Reset command request type
 */
typedef FirmwareType_t TE_HelloCommandIndParams_t;


/**//**
 * \brief Echo command confirmation type
 */
typedef struct _TE_EchoCommandConfParams_t
{
    SYS_DataPointer_t   payload;
    uint8_t             status;
} TE_EchoCommandConfParams_t;

/**//**
 * \brief Echo command parameters type
 */
typedef struct _TE_EchoCommandReqParams_t
{
    SYS_DataPointer_t   payload;
    uint8_t             echoSize;
} TE_EchoCommandReqParams_t;

/**//**
 * \brief Echo command request type
 */
typedef struct _TE_EchoCommandReqDescr_t TE_EchoCommandReqDescr_t;
typedef struct _TE_EchoCommandReqDescr_t
{
    TE_EchoCommandReqParams_t params;
    void (*callback)(TE_EchoCommandReqDescr_t *, TE_EchoCommandConfParams_t *);
} TE_EchoCommandReqDescr_t;

/**//**
 * \brief Assert log command request type.
 */
typedef struct _TE_AssertLogIdCommandIndParams_t
{
    /* log message id number */
    uint32_t number;
} TE_AssertLogIdCommandIndParams_t;

typedef enum _TE_MailboxAckStatus_t
{
    MAIL_SUCCESSFUL_RECEIVED = 0x00U,
} TE_MailboxAckStatus_t;

/**//**
 * \brief Acknowledgment structure descriptor.
 */
typedef struct _TE_MailboxAckConfParams_t
{
    TE_MailboxAckStatus_t status;
} TE_MailboxAckConfParams_t;
typedef struct _TE_MailboxAckReqDescr_t TE_MailboxAckDescr_t;
struct _TE_MailboxAckReqDescr_t
{
    void (*callback)(TE_MailboxAckDescr_t *const , TE_MailboxAckConfParams_t *const);
};


/**//**
 * \brief Enum with statuses which primitive TE_RoutingChangeReq could return in confirmation.
*/
typedef enum _TE_RoutingStatus_t
{
    TE_ROUTING_SUCCESS = 0,
    TE_ROUTING_FAILED,
} TE_RoutingStatus_t;

/**//**
 * \brief Parameters of the TE_RoutingChangeReq primitive.
*/
typedef struct _TE_RoutingChangeReqParams_t
{
    MailFID_t                fid;
    MailRouteDirection_t     routeDirection;
} TE_RoutingChangeReqParams_t;

/**//**
 * \brief Request descriptor for the TE_RoutingChangeReq primitive.
*/
typedef struct _TE_RoutingChangeReqDescr_t TE_RoutingChangeReqDescr_t;

/**//**
 * \brief Confirmation for TE_RoutingChangeReq primitive.
*/
typedef struct _TE_RoutingChangeConfParams_t
{
    TE_RoutingStatus_t status;
} TE_RoutingChangeConfParams_t;

/**//**
 * \brief Callback type of the ChangeRouting primitive.
*/
typedef void (*TE_RoutingChangeCb_t)(TE_RoutingChangeReqDescr_t *reqDescr, TE_RoutingChangeConfParams_t *conf);

/**//**
 * \brief Request descriptor for the ChangeRouting primitive.
*/
struct _TE_RoutingChangeReqDescr_t
{
    TE_RoutingChangeReqParams_t   params;
    TE_RoutingChangeCb_t          callback;
};

typedef struct _Mail_UartSendConfParams_t
{
    uint8_t             status;
} Mail_UartSendConfParams_t;

typedef struct _Mail_UartSendReqParams_t
{
    SYS_DataPointer_t   payload;
    uint8_t             payloadSize;
} Mail_UartSendReqParams_t;

typedef struct _Mail_UartSendReqDescr_t Mail_UartSendReqDescr_t;
typedef struct _Mail_UartSendReqDescr_t
{
    Mail_UartSendReqParams_t  params;
    void (*callback)(Mail_UartSendReqDescr_t *, Mail_UartSendConfParams_t *);
} Mail_UartSendReqDescr_t;

typedef struct _Mail_UartRecvIndParams_t
{
    SYS_DataPointer_t   payload;
    uint8_t             payloadSize;
}Mail_UartRecvIndParams_t;

typedef struct _Mail_UartRecvRespParams_t
{
    uint8_t    status;
} Mail_UartRecvRespParams_t;

typedef struct _Mail_UartRecvIndDescr_t  Mail_UartRecvIndDescr_t;
typedef void (*Mail_UartRecvResp_t)(Mail_UartRecvIndDescr_t *orgInd, Mail_UartRecvRespParams_t *resp);

typedef struct _Mail_UartRecvIndDescr_t
{
    Mail_UartRecvIndParams_t  params;
    Mail_UartRecvResp_t       callback;
}Mail_UartRecvIndDescr_t;

/**//**
 * \brief Public primitive for changing direction of the indication routing.
*/
void TE_RoutingChangeReq(TE_RoutingChangeReqDescr_t *const reqDescr);

/************************* PROTOTYPES **************************************************/
void Mail_TestEnginePing(TE_PingCommandReqDescr_t *const req);
void Mail_TestEngineEcho(TE_EchoCommandReqDescr_t *const req);
void Mail_TestEngineReset(TE_ResetCommandReqDescr_t *const req);
void Mail_TestEngineMailboxAckReq(TE_MailboxAckDescr_t *const req);
void Mail_TestEngineSendHello(void);
void Mail_TestEngineHelloInd(TE_HelloCommandIndParams_t *const indParams);
void Mail_TestEngineAssertLogIdInd(TE_AssertLogIdCommandIndParams_t *const indParams);
void Mail_TestEngineHaltInd(TE_AssertLogIdCommandIndParams_t *const indParams);
#ifdef _USE_ASYNC_UART_
void Mail_UartSendReq(Mail_UartSendReqDescr_t *const req);
void Mail_UartRecvInd(Mail_UartRecvIndDescr_t *const ind);
#endif
#endif /* _MAILBOX_WRAPPERS_TEST_ENGINE_ */
#endif /* _MAIL_TESTENGINE_H */

/* eof bbExtTestEngine.h */