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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/include/bbMailTestEngine.h $
*
* DESCRIPTION:
*         test engine definitions.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/

#ifndef _MAIL_TESTENGINE_H
#define _MAIL_TESTENGINE_H

#include "bbMailTypes.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Acknowledgment structure descriptor.
 */
typedef struct _MailAckConfParams_t
{
    MailAckStatus_t status;
} MailAckConfParams_t;
typedef struct _MailAckReqDescr_t MailAckDescr_t;
struct _MailAckReqDescr_t
{
    void (*callback)(MailAckDescr_t *const , MailAckConfParams_t *const);
};

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
 * \brief Set echo delay command parameters type
 */
typedef struct _TE_SetEchoDelayCommandReqParams_t
{
    uint32_t delayTime;
} TE_SetEchoDelayCommandReqParams_t;

typedef struct _TE_SetEchoDelayCommandConfParams_t TE_SetEchoDelayCommandConfParams_t;
typedef struct _TE_SetEchoDelayCommandReqDescr_t  TE_SetEchoDelayCommandReqDescr_t;

/**//**
 * \brief set echo delay command request type
 */
typedef struct _TE_SetEchoDelayCommandReqDescr_t
{
    TE_SetEchoDelayCommandReqParams_t params;
    void (*callback)(TE_SetEchoDelayCommandReqDescr_t *, TE_SetEchoDelayCommandConfParams_t *);
} TE_SetEchoDelayCommandReqDescr_t;

typedef struct _TE_SetEchoDelayCommandConfParams_t
{
    uint32_t delayTime;
    uint8_t status;
} TE_SetEchoDelayCommandConfParams_t;

/**//**
 * \brief Assert log command request type.
 */
typedef struct _TE_AssertLogIdCommandIndParams_t
{
    /* log message id number */
    uint32_t number;
} TE_AssertLogIdCommandIndParams_t;
/************************* PROTOTYPES **************************************************/

/************************************************************************************//**
    \brief Ping request implementation.
    \param[in] req - request pointer.
****************************************************************************************/
void Mail_TestEnginePing(TE_PingCommandReqDescr_t *const req);

/************************************************************************************//**
    \brief Echo request implementation.
    \param[in] req - request pointer.
****************************************************************************************/
void Mail_TestEngineEcho(TE_EchoCommandReqDescr_t *const req);

/************************************************************************************//**
    \brief Reset request implementation
    \param[in] req - request pointer.
****************************************************************************************/
void Mail_TestEngineReset(TE_ResetCommandReqDescr_t *const req);

/************************************************************************************//**
    \brief Sent greeting.
    \param[in] mail - mailbox descriptor
****************************************************************************************/
void Mail_TestEngineSendHello(MailDescriptor_t *const mail);

/************************************************************************************//**
    \brief Greeting indication.
    \param[in] indParams - indication parameters.
****************************************************************************************/
void Mail_TestEngineHelloInd(TE_HelloCommandIndParams_t *const indParams);

/************************************************************************************//**
    \brief Greeting indication.
    \param[in] indParams - indication parameters.
****************************************************************************************/
void Mail_TestEngineAssertLogIdInd(TE_AssertLogIdCommandIndParams_t *const indParams);

/*************************************************************************************
    \brief Set the echo delay timer
    \param[in] req - request pointer
*************************************************************************************/
void Mail_SetEchoDelay(TE_SetEchoDelayCommandReqDescr_t *const req);

#endif /* _MAILBOX_WRAPPERS_TEST_ENGINE_ */
#endif /* _MAIL_TESTENGINE_H */