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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/src/bbMailClient.c $
*
* DESCRIPTION:
*       implementation of mailbox client module
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/
/************************* INCLUDES ****************************************************/
#include "private/bbMailPrivateService.h"
#include "private/bbMailPrivateClient.h"
#include "private/bbMailPrivateAdapter.h"

/************************* STATIC FUNCTIONS PROTOTYPES *********************************/
INLINE uint8_t getUniqueId(MailClientDescriptor_t *const client);
INLINE void freePendingTableEntry(MailPendingAPICall_t *const entry);
INLINE void mailClientCancelParcelSending(MailDescriptor_t *const mail, MailPendingAPICall_t *const postponedCall);
INLINE void mailClientStartTimer(MailDescriptor_t *const mail, const uint32_t timeout);
INLINE void mailClientSetParcelState(MailPendingAPICall_t *const postponedCall, const MailState_t state);
INLINE void mailClientCompouseHeaders(MailFifoHeader_t *const fifoHeader, MailWrappedReqHeader_t *const header,
                                      uint16_t fId, uint8_t uId, uint8_t *const parcel);

static MailPendingAPICall_t *findEmptyPendingTableEntry(MailClientDescriptor_t *const client);
static MailPendingAPICall_t *findAppropriatePendingTableEntry(MailClientDescriptor_t *const client, const uint8_t uId);
static MailPendingAPICall_t *mailClientFillPostponeParcel(MailDescriptor_t *const mail,
        const uint16_t fId, uint8_t *const req);
static void mailClientSendParcel(MailDescriptor_t *const mail, MailPendingAPICall_t *const postponedCall);
static bool mailClientQueuePollCommon(MailDescriptor_t *const mail, bool isServiceQueue);
static void mailClientAckTimerFired(SYS_TimeoutTaskServiceField_t *const timeoutService);
/************************* IMPLEMENTATION **********************************************/
/************************************************************************************//**
    \brief initializes mailbox client module
    \param[in] mail - mailbox descriptor.
****************************************************************************************/
void mailClientInit(MailDescriptor_t *const mail)
{
    SYS_QueueResetQueue(&mail->client.parcelQueue);
    pthread_mutex_init(&mail->client.parcelQueueMutex, NULL);
    pthread_mutex_init(&mail->client.pendingTableMutex, NULL);
    SYS_QueueResetQueue(&mail->client.serviceMessageQueue);
    mail->client.uIdCounter = 10U;
    memset(mail->client.pendingTable, 0U,
           sizeof(mail->client.pendingTable[0]) * MAIL_CLIENT_MAX_AMOUNT_PENDING_CALLS);
    mail->client.ackTimer.callback = mailClientAckTimerFired;
}

/************************************************************************************//**
    \brief Gets uniq number.
    \param[in] client - client module descriptor.
****************************************************************************************/
INLINE uint8_t getUniqueId(MailClientDescriptor_t *const client)
{
    return client->uIdCounter++;
}

/************************************************************************************//**
    \brief Clear given entry of postponed table.
    \param[in] entry - pointer to entry to be cleared
****************************************************************************************/
INLINE void freePendingTableEntry(MailPendingAPICall_t *const entry)
{
    SYS_DbgAssertComplex(entry, MAILCLIENT_FREEPENDINGTABLEENTRY_0);
    pthread_mutex_lock(&mailDescriptorPtr->client.pendingTableMutex);
    entry->fId = INCORRECT_REQ_ID;
    pthread_mutex_unlock(&mailDescriptorPtr->client.pendingTableMutex);
}

/************************************************************************************//**
    \brief Cancel sending procedure and frees payload if possible.
    \param[in] mail - mailbox descriptor.
    \param[in] postponedCall - pointer to entry to be cleared
****************************************************************************************/
INLINE void mailClientCancelParcelSending(MailDescriptor_t *const mail, MailPendingAPICall_t *const postponedCall)
{
    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(postponedCall->fId);

    if (MAIL_PARCEL_AWAIT_FOR_SEND == postponedCall->state)
    {
        pthread_mutex_lock(&mail->client.parcelQueueMutex);
        SYS_QueueRemoveQueueElement(&mail->client.parcelQueue, &postponedCall->elem);
        pthread_mutex_unlock(&mail->client.parcelQueueMutex);
        mailAdapterCancelTx(&mail->adapter, (uint8_t *)&postponedCall->params);
    }

    if (NULL == postponedCall->callback
            && MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)
    {
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(
                (uint8_t *)&postponedCall->params + reqInfo->dataPointerOffset);
        if (SYS_CheckPayload(dataPointer))
            SYS_FreePayload(dataPointer);
    }
}

/************************************************************************************//**
    \brief Starts timer for acknowledgment feature.
    \param[in] mail - mailbox descriptor.
    \param[in] timeout - delay in ms
****************************************************************************************/
INLINE void mailClientStartTimer(MailDescriptor_t *const mail, const uint32_t timeout)
{
    if (!SYS_TimeoutRemain(&mail->client.ackTimer.service))
    {
        mail->client.ackTimer.timeout = timeout;
        SYS_TimeoutSignalStart(&mail->client.ackTimer, TIMEOUT_TASK_ONE_SHOT_MODE);
    }
}

/************************************************************************************//**
    \brief Set state and timeout.
    \param[in] postponedCall - parcel buffer pointer.
    \param[in] state - new state.
****************************************************************************************/
INLINE void mailClientSetParcelState(MailPendingAPICall_t *const postponedCall, const MailState_t state)
{
    postponedCall->state = state;
    switch (state)
    {
        case MAIL_PARCEL_AWAIT_FOR_SEND:
        case MAIL_ACK_AWAIT_FOR_SEND:
        case MAIL_ACK_NOT_NEEDED:
        case MAIL_ACK_SENT:
        case MAIL_WAITING_FOR_CONF:
            /* These states can't be expired and due timestamp should be as far as possible */
            postponedCall->dueTimestamp = HAL_GetSystemTime() - 1;
            break;

        case MAIL_WAITING_FOR_ACK:
            postponedCall->dueTimestamp = HAL_GetSystemTime() + MAIL_CLIENT_DELIVERY_TIME_MS;
            break;
        default:
            break;
    }
}

/************************************************************************************//**
    \brief Find first empty entry in postponed call table.
    \param[in] client - client module descriptor.

    \return Pointer to the found entry.
****************************************************************************************/
static MailPendingAPICall_t *findEmptyPendingTableEntry(MailClientDescriptor_t *const client)
{
    for (uint8_t i = 0; i < MAIL_CLIENT_MAX_AMOUNT_PENDING_CALLS; ++i)
        if (INCORRECT_REQ_ID == client->pendingTable[i].fId)
            return &client->pendingTable[i];
    return NULL;
}

/************************************************************************************//**
    \brief Find first empty entry in postponed call table.
    \param[in] client - client module descriptor.

    \return Pointer to the found entry.
****************************************************************************************/
static MailPendingAPICall_t *findAppropriatePendingTableEntry(MailClientDescriptor_t *const client, const uint8_t uId)
{
    for (uint8_t i = 0; i < MAIL_CLIENT_MAX_AMOUNT_PENDING_CALLS; ++i)
        if (INCORRECT_REQ_ID != client->pendingTable[i].fId
                && uId == client->pendingTable[i].uId)
        {
            if (MAIL_WAITING_FOR_CONF == client->pendingTable[i].state
                    || MAIL_WAITING_FOR_ACK == client->pendingTable[i].state)
                return &client->pendingTable[i];
            else
            {
                SYS_DbgLogId(MAILCLIENT_UNEXPECTED_ANSWER);
                return NULL;
            }
        }
    SYS_DbgLogId(MAILCLIENT_FINDAPPROPRIATEPENDINGTABLEENTRY_0);
    return NULL;
}

/************************************************************************************//**
    \brief Helper function. Puts a passed wrapper into the packing queue (the client's queue).
    \param[in] mail - mailbox descriptor.
    \param[in] fId - number of invoked wrapper.
    \param[in] req - request pointer.
****************************************************************************************/
static MailPendingAPICall_t *mailClientFillPostponeParcel(MailDescriptor_t *const mail,
        const uint16_t fId, uint8_t *const req)
{
    MailPendingAPICall_t *const postponedCall = findEmptyPendingTableEntry(&mail->client);
    SYS_DbgAssertComplex(postponedCall, MAILCLIENT_CAN_NOT_POSTPONE_REQUEST);
    if (postponedCall)
    {
        pthread_mutex_lock(&mail->client.pendingTableMutex);
        const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(fId);

        postponedCall->originalReq = req;
        postponedCall->fId = fId;
        postponedCall->uId = getUniqueId(&mail->client);
        postponedCall->callback = (reqInfo->callbackOffset < reqInfo->reqLength) ?
                                  *(ConfirmCall_t *)(req + reqInfo->callbackOffset) :
                                  NULL;
        memcpy(&postponedCall->params, req + reqInfo->paramOffset, reqInfo->paramLength);
        pthread_mutex_unlock(&mail->client.pendingTableMutex);
    }
    return postponedCall;
}

/************************************************************************************//**
    \brief Checks service state.
    \param[in] mail - mailbox descriptor.
    \return true if service is busy otherwise false.
****************************************************************************************/
bool mailClientIsBusy(MailDescriptor_t *const mail)
{
    return !(SYS_QueueIsEmpty(&mail->client.parcelQueue)
             && SYS_QueueIsEmpty(&mail->client.serviceMessageQueue));
}

/************************************************************************************//**
    \brief serializes the received wrapper
    \param[in] mail - mailbox descriptor.
    \param[in] fId - number of invoked wrapper.
    \param[in] req - request pointer.
****************************************************************************************/
void mailClientSerialize(MailDescriptor_t *const mail, const uint16_t fId, uint8_t *const req)
{
    SYS_DbgAssertComplex(req, MAILCLIENT_MAILCLIENTSERIALIZE_0);
    MailPendingAPICall_t *const postponedCall = mailClientFillPostponeParcel(mail, fId, req);
#if 0
    mailClientSendParcel(mail, postponedCall);
#else
    pthread_mutex_lock(&mail->client.parcelQueueMutex);
    SYS_QueuePutQueueElementToTail(&mail->client.parcelQueue, &postponedCall->elem);
    mailClientSetParcelState(postponedCall, MAIL_PARCEL_AWAIT_FOR_SEND);
    pthread_mutex_unlock(&mail->client.parcelQueueMutex);
    SYS_SchedulerPostTask(&mail->adapter.taskDescr, READY_TO_SEND_HANDLER_ID);
#endif
    return;
}

static void mailClientSendParcel(MailDescriptor_t *const mail, MailPendingAPICall_t *const postponedCall)
{
    if (postponedCall)
    {
        const bool wasBusy = mailClientIsBusy(mail);
        pthread_mutex_lock(&mail->client.parcelQueueMutex);
        SYS_QueuePutQueueElementToTail(&mail->client.parcelQueue, &postponedCall->elem);
        pthread_mutex_unlock(&mail->client.parcelQueueMutex);
        mailClientSetParcelState(postponedCall, MAIL_PARCEL_AWAIT_FOR_SEND);
        if (!wasBusy)
            mailClientQueuePollCommon(mail, false);
    }
}

INLINE void mailClientCompouseHeaders(MailFifoHeader_t *const fifoHeader, MailWrappedReqHeader_t *const header,
                                      uint16_t fId, uint8_t uId, uint8_t *const parcel)
{
    const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(fId);

    SYS_DbgAssertStatic(sizeof(fifoHeader->msgType) == sizeof(reqInfo->msgType));
    memcpy(&fifoHeader->msgType, &reqInfo->msgType, sizeof(fifoHeader->msgType));
    fifoHeader->msgId = reqInfo->id;

    header->uId                  = uId;
    header->paramLength          = reqInfo->paramLength;
    header->dataPointerOffset    = reqInfo->dataPointerOffset;
    if (MAIL_INVALID_PAYLOAD_OFFSET != reqInfo->dataPointerOffset)
    {
        SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(parcel + reqInfo->dataPointerOffset);
        header->paramLength     -= sizeof(SYS_DataPointer_t);
        header->dataLength       = SYS_GetPayloadSize(dataPointer);
        SYS_DbgAssertComplex(header->paramLength, MAILCLIENT_QUEUEPOLL_0);
    }
    else
        header->dataLength       = 0;
}

/************************************************************************************//**
    \brief This function called by the mailbox adapter when the adapter checks
        the client's sending queue.
    \param[in] mail - mailbox descriptor.

    \return true if the server's queue is not empty and false otherwise.
****************************************************************************************/
bool mailClientQueuePoll(MailDescriptor_t *const mail)
{
    return /* mailClientQueuePollCommon(mail, true) */
        /* || */ mailClientQueuePollCommon(mail, false);
}

static bool mailClientQueuePollCommon(MailDescriptor_t *const mail, bool isServiceQueue)
{
    SYS_QueueDescriptor_t *const queueToPoll = (isServiceQueue) ?
            &mail->client.serviceMessageQueue :
            &mail->client.parcelQueue;

    if (!SYS_QueueIsEmpty(queueToPoll))
    {
        MailFifoHeader_t fifoHeader;
        MailWrappedReqHeader_t header;
        MailPendingAPICall_t *const postponedCall = GET_PARENT_BY_FIELD(MailPendingAPICall_t, elem,
                SYS_QueueGetQueueHead(queueToPoll));
        uint8_t *const parcel = (isServiceQueue) ?
                                (uint8_t *)&postponedCall->serviceParcel :
                                (uint8_t *)&postponedCall->params;

        mailClientCompouseHeaders(&fifoHeader, &header, postponedCall->fId, postponedCall->uId, parcel);

        if (mailAdapterSend(&mail->adapter, &fifoHeader, &header, parcel))
        {
            pthread_mutex_lock(&mail->client.parcelQueueMutex);
            SYS_QueueRemoveHeadElement(queueToPoll);
            pthread_mutex_unlock(&mail->client.parcelQueueMutex);

            mailClientSetParcelState(postponedCall, MAIL_WAITING_FOR_ACK);
            //mailClientStartTimer(mail, MAIL_CLIENT_DELIVERY_TIME_MS);
        }
        return true;
    }
    return false;
}

/************************************************************************************//**
    \brief Helper function. Parses the parcel header and allocates memory for a parcel.
    \param[in] adapter - adapter module descriptor.
    \param[in] header - request header pointer.
****************************************************************************************/
uint8_t *mailClientGetMemory(MailAdapterDescriptor_t *const adapter,
                             MailFifoHeader_t *fifoHeader,
                             MailWrappedReqHeader_t *const header)
{
    SYS_DbgAssertComplex(adapter, MAILCLIENT_GET_MEMORY);
    MailClientDescriptor_t *const client = &GET_PARENT_BY_FIELD(MailDescriptor_t, adapter, adapter)->client;
    MailPendingAPICall_t *const postponedCall = findAppropriatePendingTableEntry(client, header->uId);
    if (postponedCall)
        return (FIRST_SERVICE_FID <= fifoHeader->msgId) ?
               (uint8_t *)&postponedCall->serviceParcel :
               (uint8_t *)&postponedCall->params;
    return NULL;
}

/************************************************************************************//**
    \brief This function should be invoked by the adapter when it has a parcel
        for the mailbox client.
    \param[in] adapter - adapter module descriptor.
    \param[out] fifoHeader - FIFO header pointer.
    \param[out] header - request header pointer.
    \param[out] confirm - confirmation parameters pointer.
****************************************************************************************/
void mailClientDataInd(MailAdapterDescriptor_t *const adapter, MailFifoHeader_t *const fifoHeader,
                       MailWrappedReqHeader_t *const header, uint8_t *confirm)
{
    (void)header;
    SYS_DbgAssertComplex(NULL != confirm, MAILCLIENT_DATAIND_CONFIRM_SHALL_BE_NOT_NULL);
    MailDescriptor_t *const mail = GET_PARENT_BY_FIELD(MailDescriptor_t, adapter, adapter);

    switch (fifoHeader->msgId)
    {
        case MAIL_ACK_FID:
        {
            MailPendingAPICall_t *const postponedCall = GET_PARENT_BY_FIELD(MailPendingAPICall_t,
                    serviceParcel , confirm);
            SYS_DbgAssertComplex(MAIL_SUCCESSFUL_RECEIVED == postponedCall->serviceParcel.conf.ack.status,
                          MAILCLIENT_DATAIND_0);

            mailClientCancelParcelSending(mail, postponedCall);
            #if defined(RF4CE_ZRC_WAKEUP_ACTION_CODE_SUPPORT) && !defined(_HOST_)
            RF4CE_PMSetHostWakingUp(false);
            #endif
            if (!postponedCall->callback)
                freePendingTableEntry(postponedCall);
            else
                mailClientSetParcelState(postponedCall, MAIL_WAITING_FOR_CONF);
            break;
        }
        case TE_ASSERT_LOGID_FID:
        case TE_ASSERT_ERRID_FID:
            SYS_DbgAssertComplex(false, MAILCLIENT_DATAIND_LOGID_AND_ERRID_MESSAGES_DO_NOT_HAVE_AN_APPROPRIATE_RESPONSE);
            break;
        default:
        {
            MailPendingAPICall_t *const postponedCall = GET_PARENT_BY_FIELD(MailPendingAPICall_t, params, confirm);
#ifdef _DEBUG_COMPLEX_
            {
                SYS_DbgAssertLog(MAIL_WAITING_FOR_ACK != postponedCall->state, MAILCLIENT_DATAIND_MAYBE_ACK_WAS_MISSED);
                const MailClientParametersTableEntry_t *const reqInfo = mailClientTableGetAppropriateEntry(fifoHeader->msgId);
                SYS_DbgAssertComplex(reqInfo->callbackOffset < reqInfo->reqLength, MAILCLIENT_DATAIND_COMMAND_DO_NOT_HAVE_RESPONSE_OR_CONFIRM);
                SYS_DbgAssertComplex(FIRST_SERVICE_FID > fifoHeader->msgId, MAILCLIENT_DATAIND_THIS_IS_SHALL_BE_NOT_A_SERVICE_MESSAGE);
                SYS_DbgAssertComplex(NULL != postponedCall->callback, MAILCLIENT_DATAIND_CALLBACK_SHALL_BE_NOT_NULL);
            }
#endif /* _DEBUG_COMPLEX_ */
            /* call application callback */
            mailServiceDeserialize(mail, postponedCall->callback, postponedCall->originalReq, confirm);
            mailClientCancelParcelSending(mail, postponedCall);
            freePendingTableEntry(postponedCall);
        }
    }
}

static void mailClientAckTimerFired(SYS_TimeoutTaskServiceField_t *const timeoutService)
{
    MailDescriptor_t *const mail = GET_PARENT_BY_FIELD(MailDescriptor_t,
                                   client.ackTimer.service, timeoutService);
    const MailPendingAPICall_t *nextToExpire = NULL;
    for (uint8_t i = 0; i < MAIL_CLIENT_MAX_AMOUNT_PENDING_CALLS; ++i)
    {
        MailPendingAPICall_t *const entry = &mail->client.pendingTable[i];
        if (INCORRECT_REQ_ID != entry->fId)
        {
            if (MAIL_WAITING_FOR_ACK != entry->state)
                continue;

            if (HAL_GetSystemTime() >= entry->dueTimestamp)
                mailClientSendParcel(mail, entry);
            else if (NULL == nextToExpire
                     || nextToExpire->dueTimestamp > entry->dueTimestamp)
                nextToExpire = entry;
        }
    }

    if (NULL != nextToExpire)
    {
        const SYS_Time_t currentTime = HAL_GetSystemTime();
        const uint32_t timeout = (nextToExpire->dueTimestamp > currentTime) ?
                                 nextToExpire->dueTimestamp - currentTime : 0;
        mailClientStartTimer(mail, timeout);
    }
}
