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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/src/bbMailServer.c $
*
* DESCRIPTION:
*       implementation of mailbox server module
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/
/************************* INCLUDES ****************************************************/
#include "private/bbMailPrivateService.h"
#include "private/bbMailPrivateServer.h"
#include "private/bbMailPrivateAdapter.h"

/************************* STATIC FUNCTIONS PROTOTYPES *********************************/
INLINE void freeServerBuffer(MailServerBuffer_t *const buffer);
INLINE MailServerBuffer_t *findEmptyServerBuffer(MailServerDescriptor_t *const server);
INLINE MailServerBuffer_t *findAppropriateServerBuffer(MailServerDescriptor_t *const server, uint8_t *reqParam);

static void mailServerSendMessage(MailDescriptor_t *const mail, MailServerBuffer_t *const buffer,
                                  const bool isServiceParcel);
static bool mailServerQueuePollCommon(MailDescriptor_t *const mail, bool isServiceQueue);
/************************* IMPLEMENTATION **********************************************/
/************************************************************************************//**
    \brief initializes mailbox server module
    \param[in] server - server module descriptor.

****************************************************************************************/
void mailServerInit(MailDescriptor_t *const mail)
{
    SYS_QueueResetQueue(&mail->server.finishedQueue);
    SYS_QueueResetQueue(&mail->server.serviceQueue);

    memset(mail->server.buffer, 0U, sizeof(mail->server.buffer[0]) * MAIL_SERVER_MAX_AMOUNT_PROCESSED_REQUESTS);
}

/************************************************************************************//**
    \brief Helper function. Finds empty server buffer.
    \param[in] server - server module descriptor.

    \return a pointer to the free buffer.
****************************************************************************************/
INLINE MailServerBuffer_t *findEmptyServerBuffer(MailServerDescriptor_t *const server)
{
    for (uint8_t i = 0; i < MAIL_SERVER_MAX_AMOUNT_PROCESSED_REQUESTS; i++)
    {
        if (!server->buffer[i].isBusy)
            return &server->buffer[i];
    }
    SYS_DbgLogId(MAILSERVER_FINDEMPTYSERVERBUFFER_0);
    return NULL;
}

/************************************************************************************//**
    \brief Helper function. Free given buffer.
    \param[in] buffer - a pointer to a buffer to be cleared
****************************************************************************************/
INLINE void freeServerBuffer(MailServerBuffer_t *const buffer)
{
    SYS_DbgAssertComplex(buffer, MAILSERVER_FREESERVERBUFFER_0);
    buffer->isBusy = false;
}

INLINE void mailServerFreeBufferPayload(MailWrappedReqHeader_t *const header, uint8_t *const parcel)
{
    if (header->dataLength && MAIL_INVALID_PAYLOAD_OFFSET != header->dataPointerOffset)
    {
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(
                parcel + header->dataPointerOffset);
        SYS_FreePayload(dataPointer);
    }
}

/************************************************************************************//**
    \brief Helper function. Parses the header of a parcel and allocates memory for a parcel.
    \param[in] adapter - adapter module descriptor.
    \param[in] header - request header pointer.
****************************************************************************************/
uint8_t *mailServerGetMemory(MailAdapterDescriptor_t *const adapter,
                             MailFifoHeader_t *fifoHeader,
                             MailWrappedReqHeader_t *const header)
{
    (void)header;
    MailServerDescriptor_t *const server = &GET_PARENT_BY_FIELD(MailDescriptor_t, adapter, adapter)->server;
    MailServerBuffer_t *buffer = findEmptyServerBuffer(server);
    if (NULL != buffer)
    {
        const MailServerParametersTableEntry_t *const reqInfo = mailServerTableGetAppropriateEntry(fifoHeader->msgId);
        buffer->isBusy = true;
        return (uint8_t *)&buffer->parcel + reqInfo->reqParametersOffset;
    }
    return NULL;
}

/************************************************************************************//**
    \brief Helper function. Free given buffer.
    \param[in] adapter - adapter module descriptor.
    \param[in] param - parameter pointer.
****************************************************************************************/
void mailFreeServerBuffer(MailAdapterDescriptor_t *const adapter,
                          uint8_t *param)
{
    MailDescriptor_t *const mail = GET_PARENT_BY_FIELD(MailDescriptor_t, adapter, adapter);
    MailServerDescriptor_t *const server = &mail->server;
    MailServerBuffer_t *const buffer = findAppropriateServerBuffer(server, param);
    if (buffer)
        freeServerBuffer(buffer);
}

/************************************************************************************//**
    \brief Helper function. Finds the first appropriate buffer.
    \param[in] server - server module descriptor.
    \param[in] param - request parameters pointer.

    \return a pointer to the found buffer or NULL if buffer is absent.
****************************************************************************************/
INLINE MailServerBuffer_t *findAppropriateServerBuffer(MailServerDescriptor_t *const server, uint8_t *reqParam)
{
    for (uint8_t i = 0; i < MAIL_SERVER_MAX_AMOUNT_PROCESSED_REQUESTS; i++)
    {
        if (server->buffer[i].isBusy)
            if (reqParam >= (uint8_t *)&server->buffer[i]
                    && reqParam <= ((uint8_t *)&server->buffer[i].parcel + sizeof(server->buffer[i].parcel)))
                return &server->buffer[i];
    }
    SYS_DbgLogId(MAILSERVER_FINDAPPROPRIATESERVERBUFFER_0);
    return NULL;
}

/************************************************************************************//**
    \brief This function called by mailbox adapter when It has some data to the server.
    \param[in] adapter - adapter module descriptor.
    \param[in] fifoHeader - FIFO header pointer.
    \param[in] header - request header pointer.
    \param[in] req - request pointer.
****************************************************************************************/
void mailServerDataInd(MailAdapterDescriptor_t *const adapter,
                       MailFifoHeader_t *fifoHeader,
                       MailWrappedReqHeader_t *header,
                       uint8_t *reqParam)
{
    MailDescriptor_t *const mail = GET_PARENT_BY_FIELD(MailDescriptor_t, adapter, adapter);
    MailServerDescriptor_t *const server = &mail->server;
    MailServerBuffer_t *const buffer = findAppropriateServerBuffer(server, reqParam);
    const MailServerParametersTableEntry_t *const reqInfo = mailServerTableGetAppropriateEntry(fifoHeader->msgId);

    memcpy(&buffer->header, header, sizeof(*header));
    memcpy(&buffer->fifoHeader, fifoHeader, sizeof(*fifoHeader));

    /* set callback */
    if (reqInfo->callbackOffset < reqInfo->reqLength)
    {
        ConfirmCall_t *const callbackPlace = (ConfirmCall_t *)(void *)((uint8_t *)&buffer->parcel
                                             + reqInfo->callbackOffset);
        *callbackPlace = mail->fakeCallback;
    }

    /* send ack */
    {
        buffer->serviceParcel.conf.ack.status = MAIL_SUCCESSFUL_RECEIVED;
        mailServerSendMessage(mail, buffer, true);
    }
}

/************************************************************************************//**
    \brief This function should receive all callback calls from the application/stack
    \param[in] server - server module descriptor.
    \param[in] confirm - a pointer to structure with confirmation.
****************************************************************************************/
void mailServerFakeCallback(MailServerDescriptor_t *const server, void *req, void *confirm)
{
    MailDescriptor_t *const mail = GET_PARENT_BY_FIELD(MailDescriptor_t, server, server);
    MailServerBuffer_t *const buffer = GET_PARENT_BY_FIELD(MailServerBuffer_t, parcel.req, req);
    const MailServerParametersTableEntry_t *const reqInfo =
        mailServerTableGetAppropriateEntry(buffer->fifoHeader.msgId);

    mailServerFreeBufferPayload(&buffer->header, (uint8_t *)&buffer->parcel + reqInfo->reqParametersOffset);

    /* save confirmation instead of request */
    memcpy(&buffer->parcel.conf, confirm, reqInfo->confParametersLength);
#ifdef _DEBUG_COMPLEX_
    memset(&buffer->parcel.dbgConfirmOffset, 0xFD, sizeof(buffer->parcel.dbgConfirmOffset));
#endif
    /* send message to the client side */
    mailServerSendMessage(mail, buffer, false);
}

/************************************************************************************//**
    \brief Checks service state.
    \param[in] mail - mailbox descriptor.
    \return true if service is busy otherwise false.
****************************************************************************************/
bool mailServerIsBusy(MailDescriptor_t *const mail)
{
    return !(SYS_QueueIsEmpty(&mail->server.finishedQueue)
             && SYS_QueueIsEmpty(&mail->server.serviceQueue));
}

static void mailServerSendMessage(MailDescriptor_t *const mail, MailServerBuffer_t *const buffer,
                                  const bool isServiceParcel)
{
    const bool wasBusy = mailServerIsBusy(mail);
    SYS_QueueDescriptor_t *const queue = (isServiceParcel) ?
                                         &mail->server.serviceQueue :
                                         &mail->server.finishedQueue;
    SYS_QueuePutQueueElementToTail(queue, &buffer->elem);
    if (!wasBusy)
        mailServerQueuePollCommon(mail, isServiceParcel);
}

/************************************************************************************//**
    \brief This function called by the mailbox adapter when the adapter checks
        the server's sending queue.
    \param[in] mail - mailbox descriptor.

    \return true if the server's queue is not empty and false otherwise.
****************************************************************************************/
bool mailServerQueuePoll(MailDescriptor_t *const mail)
{
    if (!mailServerQueuePollCommon(mail, true))
        return mailServerQueuePollCommon(mail, false);
    return true;
}

INLINE void mailServerComposeHeaders(MailFifoHeader_t *const fifoHeader, MailWrappedReqHeader_t *const header,
                                     uint8_t *const parcel, MailFifoHeader_t *const originalFifoHeader, const uint16_t fId, const uint8_t uId)
{
    const MailServerParametersTableEntry_t *const reqInfo = mailServerTableGetAppropriateEntry(fId);
    /* compose fifo header */
    {
        memcpy(fifoHeader, originalFifoHeader, sizeof(*fifoHeader));
        fifoHeader->msgType.type    = (REQUEST_MSG_TYPE == originalFifoHeader->msgType.type) ?
                                      CONFIRM_MSG_TYPE : RESPONSE_MSG_TYPE;
        fifoHeader->msgId = fId;
    }

    /* compose wrapped header */
    {
        /*  header->uId same for all confirmations */
        header->uId                 = uId;
        header->paramLength         = reqInfo->confParametersLength;
        header->dataPointerOffset   = reqInfo->confDataPointerOffset;

        if (MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->confDataPointerOffset)
        {
            header->paramLength     -= sizeof(SYS_DataPointer_t);
            header->dataLength      = SYS_GetPayloadSize((SYS_DataPointer_t *)(
                                          parcel + reqInfo->confDataPointerOffset));
            SYS_DbgAssertComplex(header->paramLength, MAILSERVER_FAKECALLBACK_0);
        }
        else
            header->dataLength      = 0;
    }
}

static bool mailServerQueuePollCommon(MailDescriptor_t *const mail, bool isServiceQueue)
{
    SYS_QueueDescriptor_t *const queueToPoll = (isServiceQueue) ?
            &mail->server.serviceQueue :
            &mail->server.finishedQueue;

    if (!SYS_QueueIsEmpty(queueToPoll))
    {
        MailFifoHeader_t fifoHeader;
        MailWrappedReqHeader_t header;
        MailServerBuffer_t *const buffer = GET_PARENT_BY_FIELD(MailServerBuffer_t, elem,
                                           SYS_QueueGetQueueHead(queueToPoll));
        const uint16_t fId = (isServiceQueue) ?
                             MAIL_ACK_FID :
                             buffer->fifoHeader.msgId;
        uint8_t *const parcel = (isServiceQueue) ?
                                (uint8_t *)&buffer->serviceParcel :
                                (uint8_t *)&buffer->parcel.conf;
        mailServerComposeHeaders(&fifoHeader, &header, parcel, &buffer->fifoHeader, fId, buffer->header.uId);
        if (mailAdapterSend(&mail->adapter, &fifoHeader, &header, parcel))
        {
            SYS_QueueRemoveHeadElement(queueToPoll);
            if (isServiceQueue)
            {
                const MailServerParametersTableEntry_t *const reqInfo =
                    mailServerTableGetAppropriateEntry(buffer->fifoHeader.msgId);
                MailPublicFunction_t publicFunction = reqInfo->function;
                mailServiceCallRequestHandler(mail, publicFunction, &buffer->parcel);

                if (reqInfo->callbackOffset == reqInfo->reqLength)
                {
                    /* NOTE: In this case application should clear payload. */
                    // mailServerFreeBufferPayload(&buffer->header,
                    //                             (uint8_t *)&buffer->parcel + reqInfo->reqParametersOffset);
                    freeServerBuffer(buffer);
                }
            }
            else
            {
                mailServerFreeBufferPayload(&header, parcel);
                freeServerBuffer(buffer);
            }
        }
        return true;
    }
    return false;
}