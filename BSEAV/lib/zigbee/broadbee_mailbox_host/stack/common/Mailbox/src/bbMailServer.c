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
 *      implementation of mailbox server module
 *
*******************************************************************************/

/************************* INCLUDES ****************************************************/
#include "private/bbMailPrivateServer.h"
#include "private/bbMailPrivateAdapter.h"

/************************* STATIC FUNCTIONS PROTOTYPES *********************************/

INLINE void freeServerBuffer(MailServerBuffer_t *const buffer);
INLINE MailServerBuffer_t *findEmptyServerBuffer(void);
INLINE MailServerBuffer_t *findAppropriateServerBuffer(uint8_t *reqParam);

static void mailServerSendMessage(MailServerBuffer_t *const buffer, const bool isServiceParcel);
static bool mailServerQueuePollCommon(bool isServiceQueue);
/************************* IMPLEMENTATION **********************************************/
static MailServerDescriptor_t serverMemory;
/************************************************************************************//**
    \brief initializes mailbox server module
    \param[in] server - server module descriptor.

****************************************************************************************/
void mailServerInit()
{
    SYS_QueueResetQueue(&serverMemory.finishedQueue);
    SYS_QueueResetQueue(&serverMemory.serviceQueue);

    memset(serverMemory.buffer, 0U, sizeof(serverMemory.buffer[0]) * MAIL_SERVER_MAX_AMOUNT_PROCESSED_REQUESTS);
}

/************************************************************************************//**
    \brief Helper function. Finds empty server buffer.
    \param[in] server - server module descriptor.

    \return a pointer to the free buffer.
****************************************************************************************/
INLINE MailServerBuffer_t *findEmptyServerBuffer(void)
{
    for (uint8_t i = 0; i < MAIL_SERVER_MAX_AMOUNT_PROCESSED_REQUESTS; i++)
    {
        if (!serverMemory.buffer[i].isBusy)
            return &serverMemory.buffer[i];
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
    buffer->isBusy = false;
}

/************************************************************************************//**
    \brief Helper function. Parses the header of a parcel and allocates memory for a parcel.
****************************************************************************************/
uint8_t *mailServerGetMemory(MailFifoHeader_t *fifoHeader, MailWrappedReqHeader_t *const header)
{
    (void)header;
    MailServerBuffer_t *buffer = findEmptyServerBuffer();
    if (NULL != buffer)
    {
        const MailServiceFunctionInfo_t *const reqInfo = Mail_ServiceGetFunctionInfo(fifoHeader->msgId);
        buffer->isBusy = true;
        return (uint8_t *)&buffer->parcel + reqInfo->reqParametersOffset;
    }
    return NULL;
}

/************************************************************************************//**
    \brief Helper function. Free given buffer.
    \param[in] param - parameter pointer.
****************************************************************************************/
void mailFreeServerBuffer(uint8_t *param)
{
    MailServerBuffer_t *const buffer = findAppropriateServerBuffer(param);
    if (buffer)
        freeServerBuffer(buffer);
}

/************************************************************************************//**
    \brief Helper function. Finds the first appropriate buffer.
    \param[in] server - server module descriptor.
    \param[in] param - request parameters pointer.

    \return a pointer to the found buffer or NULL if buffer is absent.
****************************************************************************************/
INLINE MailServerBuffer_t *findAppropriateServerBuffer(uint8_t *reqParam)
{
    for (uint8_t i = 0; i < MAIL_SERVER_MAX_AMOUNT_PROCESSED_REQUESTS; i++)
    {
        if (serverMemory.buffer[i].isBusy)
            if (reqParam >= (uint8_t *)&serverMemory.buffer[i].parcel
                    && reqParam <= ((uint8_t *)&serverMemory.buffer[i].parcel + sizeof(serverMemory.buffer[i].parcel)))
                return &serverMemory.buffer[i];
    }
    SYS_DbgLogId(MAILSERVER_FINDAPPROPRIATESERVERBUFFER_0);
    return NULL;
}

/************************************************************************************//**
    \brief This function should receive all callback calls from the application/stack
    \param[in] server - server module descriptor.
    \param[in] confirm - a pointer to structure with confirmation.
****************************************************************************************/
void mailServerFakeCallback(void *req, void *confirm)
{
    MailServerBuffer_t *const buffer = GET_PARENT_BY_FIELD(MailServerBuffer_t, parcel.req, req);
    const MailServiceFunctionInfo_t *const reqInfo = Mail_ServiceGetFunctionInfo(buffer->fifoHeader.msgId);

    if (buffer->header.dataLength && MAIL_INVALID_OFFSET != reqInfo->reqDataPointerOffset)
        SYS_FreePayload((SYS_DataPointer_t *)((uint8_t *)&buffer->parcel
                                              + reqInfo->reqParametersOffset + reqInfo->reqDataPointerOffset));

    /* save confirmation instead of request */
    memcpy(&buffer->parcel.conf, confirm, reqInfo->confParametersLength);
#ifdef _DEBUG_COMPLEX_
    memset(&buffer->parcel.dbgConfirmOffset, 0xFD, sizeof(buffer->parcel.dbgConfirmOffset));
#endif
    /* send message to the client side */
    mailServerSendMessage(buffer, false);
}

/************************************************************************************//**
    \brief This function called by mailbox adapter when It has some data to the server.
    \param[in] fifoHeader - FIFO header pointer.
    \param[in] header - request header pointer.
    \param[in] req - request pointer.
****************************************************************************************/
void mailServerDataInd(MailFifoHeader_t *fifoHeader, MailWrappedReqHeader_t *header, uint8_t *reqParam)
{
    MailServerBuffer_t *const buffer = findAppropriateServerBuffer(reqParam);
    const MailServiceFunctionInfo_t *const reqInfo = Mail_ServiceGetFunctionInfo(fifoHeader->msgId);

    memcpy(&buffer->header, header, sizeof(*header));
    memcpy(&buffer->fifoHeader, fifoHeader, sizeof(*fifoHeader));

    /* set callback */
    if (MAIL_INVALID_OFFSET != reqInfo->reqCallbackOffset)
    {
        ConfirmCall_t *const callbackPlace = (ConfirmCall_t *)(void *)((uint8_t *)&buffer->parcel
                                             + reqInfo->reqCallbackOffset);
        *callbackPlace = mailServerFakeCallback;
    }

    /* send ack */
    {
        buffer->serviceParcel.conf.ack.status = MAIL_SUCCESSFUL_RECEIVED;
        mailServerSendMessage(buffer, true);
    }
}

/************************************************************************************//**
    \brief Checks service state.
    \param[in] mail - mailbox descriptor.
    \return true if service is busy otherwise false.
****************************************************************************************/
bool mailServerIsBusy(void)
{
    return !(SYS_QueueIsEmpty(&serverMemory.finishedQueue)
             && SYS_QueueIsEmpty(&serverMemory.serviceQueue));
}

static void mailServerSendMessage(MailServerBuffer_t *const buffer,
                                  const bool isServiceParcel)
{
    const bool wasBusy = mailServerIsBusy();
    SYS_QueueDescriptor_t *const queue = (isServiceParcel) ?
                                         &serverMemory.serviceQueue :
                                         &serverMemory.finishedQueue;
    SYS_DbgAssertComplex(!SYS_QueueFindParentElement(queue, &buffer->elem), SYS_QUEUE_PUT_MAIL_SERVER);
    SYS_QueuePutQueueElementToTail(queue, &buffer->elem);
    if (!wasBusy)
        mailServerQueuePollCommon(isServiceParcel);
}

/************************************************************************************//**
    \brief This function called by the mailbox adapter when the adapter checks
        the server's sending queue.
    \param[in] mail - mailbox descriptor.

    \return true if the server's queue is not empty and false otherwise.
****************************************************************************************/
bool mailServerQueuePoll(void)
{
    if (!mailServerQueuePollCommon(true))
        return mailServerQueuePollCommon(false);
    return true;
}

INLINE void mailServerComposeHeaders(MailFifoHeader_t *const fifoHeader, MailWrappedReqHeader_t *const header,
                                     uint8_t *const parcel, MailFifoHeader_t *const originalFifoHeader, const uint16_t fId, const uint8_t uId)
{
    const MailServiceFunctionInfo_t *const reqInfo = Mail_ServiceGetFunctionInfo(fId);
    /* compose fifo header */
    {
        memcpy(fifoHeader, originalFifoHeader, sizeof(*fifoHeader));
        fifoHeader->msgId = fId;
        fifoHeader->msgType.isConfirm = 1;

#ifdef MAILBOX_HOST_SIDE
        fifoHeader->msgType.fromStackSide = 0;
#else
        fifoHeader->msgType.fromStackSide = 1;
#endif
    }

    /* compose wrapped header */
    {
        /*  header->uId same for all confirmations */
        header->uId                 = uId;
        header->paramLength         = reqInfo->confParametersLength;

        if (MAIL_INVALID_OFFSET != reqInfo->confDataPointerOffset)
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

static bool mailServerQueuePollCommon(bool isServiceQueue)
{
    SYS_QueueDescriptor_t *const queueToPoll = (isServiceQueue) ?
            &serverMemory.serviceQueue :
            &serverMemory.finishedQueue;

    if (!SYS_QueueIsEmpty(queueToPoll))
    {
        MailFifoHeader_t fifoHeader;
        MailWrappedReqHeader_t header;
        MailServerBuffer_t *const buffer = GET_PARENT_BY_FIELD(MailServerBuffer_t, elem,
                                           SYS_QueueGetQueueHead(queueToPoll));
        const uint16_t fId = (isServiceQueue) ?
                             TE_MAILBOX_ACK_FID :
                             buffer->fifoHeader.msgId;
        uint8_t *const parcel = (isServiceQueue) ?
                                (uint8_t *)&buffer->serviceParcel :
                                (uint8_t *)&buffer->parcel.conf;
        mailServerComposeHeaders(&fifoHeader, &header, parcel, &buffer->fifoHeader, fId, buffer->header.uId);
        if (mailAdapterSend(&fifoHeader, &header, parcel))
        {
            const MailServiceFunctionInfo_t *const reqInfo = Mail_ServiceGetFunctionInfo(buffer->fifoHeader.msgId);
            SYS_QueueRemoveHeadElement(queueToPoll);
            if (isServiceQueue)
            {
                MailPublicFunction_t publicFunction = reqInfo->function;
                Mail_RequestHandler(buffer->fifoHeader.msgId, publicFunction, &buffer->parcel);

                if (MAIL_INVALID_OFFSET == reqInfo->reqCallbackOffset)
                {
                    /* NOTE: In this case application should clear payload. */
                    freeServerBuffer(buffer);
                }
            }
            else
            {
                if (header.dataLength && MAIL_INVALID_OFFSET != reqInfo->confDataPointerOffset)
                    SYS_FreePayload((SYS_DataPointer_t *)((uint8_t *)parcel + reqInfo->confDataPointerOffset));
                freeServerBuffer(buffer);
            }
        }
        return true;
    }
    return false;
}

/* eof bbMailServer.c */