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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/src/bbMailService.c $
*
* DESCRIPTION:
*       implementation of mailbox service module
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/
/************************* INCLUDES ****************************************************/
#include "private/bbMailPrivateService.h"
#include "private/bbMailPrivateAdapter.h"
#include "private/bbMailPrivateClient.h"
#include "private/bbMailPrivateServer.h"

#include "bbMailTestEngine.h"
#include "bbSysDbgProfilingEngine.h"

/************************* IMPLEMENTATION **********************************************/
#ifndef _TEST_HARNESS_ // == STANDARD_DEVICE
MailDescriptor_t *mailDescriptorPtr;
MAIL_SERVER_CREATE_CALLBACK_RECEIVER(mailServerCallbackReceiver, mailDescriptorPtr);

/************************************************************************************//**
    \brief Initialize mail service.
    \param[in] mail - mailbox descriptor.
****************************************************************************************/
void Mail_ServiceInit(MailDescriptor_t *const mail)
{
    mailDescriptorPtr = mail;

    mail->fakeCallback = mailServerCallbackReceiver;
    mailServiceInit(mail);
    Mail_TestEngineSendHello(mail);
}

/************************************************************************************//**
    \brief This function envoke application callback.
    \param[in] mail - mailbox descriptor.
    \param[in] offset - callback offset.
    \param[in] req - request poiter.
    \param[in] confirm - the confirmation structure pointer.
****************************************************************************************/
void mailServiceDeserialize(MailDescriptor_t *const mail, const ConfirmCall_t call, void *req, void *confirm)
{
    SYS_DbgAssertComplex(mail, MAILSERVICE_MAILSERVICEDESERIALIZE_0);
    call(req, confirm);
}

/************************************************************************************//**
    \brief Helper function for initialize mail service. Used at testing.
    \param[in] mail - mailbox descriptor.
    \param[in] handler - handler
****************************************************************************************/
void mailServiceCallRequestHandler(MailDescriptor_t *const mail, MailPublicFunction_t handler, void *req)
{
    SYS_DbgAssertComplex(mail, MAILSERVICE_MAILSERVICECALLREQUESTHANDLER_0);
    handler(req);
}

#endif /* TEST_HARNESS // == STANDARD_DEVICE */
/************************************************************************************//**
    \brief Helper function for initialize mail service. Used at testing.
    \param[in] mail - mailbox descriptor.
****************************************************************************************/
void mailServiceInit(MailDescriptor_t *const mail)
{
    SYS_DbgAssertComplex(mail, MAILSERVICE_MAILSERVICEINIT_0);
    SYS_DbgAssertComplex(mail->fakeCallback, MAILSERVICE_MAILSERVICEINIT_2);

    mailClientInit(mail);
    mailServerInit(mail);
    mailAdapterInit(&mail->adapter);
}

/************************************************************************************//**
    \brief begins the process of serialization.
    \param[in] mail - mailbox descriptor.
    \param[in] fId - request ID (wrapper function number)
    \param[in] req - request pointer.
****************************************************************************************/
void Mail_Serialize(MailDescriptor_t *const mail, uint16_t fId, void *req)
{
    SYS_DbgAssertComplex(mail, MAILSERVICE_SERIALIZE_0);
    mailClientSerialize(mail, fId, (uint8_t *)req);
}
/* eof bbMailService.c */