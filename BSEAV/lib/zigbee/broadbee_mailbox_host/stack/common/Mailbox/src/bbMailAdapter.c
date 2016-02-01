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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/src/bbMailAdapter.c $
*
* DESCRIPTION:
*       implementation of mailbox adapter module
*
* $Revision: 3878 $
* $Date: 2014-10-03 15:15:36Z $
*
****************************************************************************************/
/************************* INCLUDES ****************************************************/
#include "private/bbMailPrivateClient.h"
#include "private/bbMailPrivateServer.h"
#include "private/bbMailPrivateAdapter.h"

/************************* STATIC FUNCTIONS PROTOTYPES *********************************/
static void mailAdapterReset(MailAdapterDescriptor_t *const adapter);
static void mailAdapterFifoOfflineInd(HAL_MailboxDescriptor_t *const mbFifoDescr);
static void mailAdapterReadyToSendInd(HAL_MailboxDescriptor_t *const mbFifoDescr);
static void mailAdapterReceivedInd(HAL_MailboxDescriptor_t *const mbFifoDescr);
static void mailAdapterReadyToSendHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
static void mailAdapterDataReceivedHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************* IMPLEMENTATION **********************************************/
/**//**
 * \brief Adapter handlers list descriptions.
 */
static const SYS_SchedulerTaskHandler_t mailAdapterHandlers[] =
{
    [READY_TO_SEND_HANDLER_ID]  = mailAdapterReadyToSendHandler,
    [RECEIVE_HANDLER_ID]        = mailAdapterDataReceivedHandler,
};

/************************************************************************************//**
    \brief Initialize internal data structures.
    \param[in] adapter - adapter descriptor.
 ****************************************************************************************/
void mailAdapterInit(MailAdapterDescriptor_t *const adapter)
{
    mailAdapterReset(adapter);

    adapter->mbFifoDescr.offlineCallback    = mailAdapterFifoOfflineInd;
    adapter->mbFifoDescr.rtsCallback        = mailAdapterReadyToSendInd;
    adapter->mbFifoDescr.rxCallback         = mailAdapterReceivedInd;

    HAL_MailboxInit(&adapter->mbFifoDescr);
}

/************************************************************************************//**
    \brief Helper function. Reset tx options struct.
    \param[in] rxOptions - pointer.
 ****************************************************************************************/
static void mailAdapterInitTxOptions(MailAdapterTxOptions_t *const txOptions)
{
    txOptions->currentParcel = NULL;
    txOptions->fragmentCounter = 0;
    txOptions->offset = 0;
    txOptions->reservedSpace = 0;
    txOptions->txCheckSum = 0;
    txOptions->txDirection = NO_TRANSMISSION;
}

/************************************************************************************//**
    \brief Stops transactions.
    \param[in] adapter - adapter descriptor.
 ****************************************************************************************/
void mailAdapterCancelTx(MailAdapterDescriptor_t *const adapter, uint8_t *const parcel)
{
    if (parcel == adapter->txOptions.currentParcel)
    {
        mailAdapterInitTxOptions(&adapter->txOptions);
        SYS_DbgLogId(MAILADAPTER_TRANSACTION_WAS_INTERRUPTED_AT_THE_MIDDLE);
    }
}

/************************************************************************************//**
    \brief Helper function. Reset rx options struct.
    \param[in] rxOptions - pointer.
 ****************************************************************************************/
static void mailInitRxOptions(MailAdapterRxOptions_t *const rxOptions)
{
    rxOptions->rxCheckSumCalc = 0;
    rxOptions->offset = 0;
    rxOptions->nextFragmentNumber = 0;
    rxOptions->rxDirection = NO_TRANSMISSION;
    rxOptions->params = NULL;
    SYS_DbgAssertComplex(!SYS_CheckPayload(&rxOptions->dataPointer), MAILADAPTER_INITRXOPTIONS_DAPOINTER_SHALL_BE_FREE);
}
/************************************************************************************//**
    \brief Synchronization event handler.
    \param[in] mbFifoDescr - shared FIFO descriptor.
 ****************************************************************************************/
static void mailAdapterReset(MailAdapterDescriptor_t *const adapter)
{
    adapter->taskDescr.priority = SYS_SCHEDULER_MAILBOX_PRIORITY;
    adapter->taskDescr.handlers = mailAdapterHandlers;
    adapter->pollPriority = 0; /* client's part has more priority by default */

    adapter->readyToSendFlag = false;

    mailAdapterInitTxOptions(&adapter->txOptions);
    for (uint8_t i = 0; i < ARRAY_SIZE(adapter->rxOptions); ++i)
    {
        SYS_SetEmptyPayload(&adapter->rxOptions[i].dataPointer);
        mailInitRxOptions(&adapter->rxOptions[i]);
    }
}

/************************************************************************************//**
    \brief Helper function. Get parcel direction.
    \param[in] msgType - pointer to the structure with parcel information.

    \return FROM_CLIENT_TO_SERVER or FROM_SERVER_TO_CLIENT
****************************************************************************************/
INLINE MailAdapterDirection_t mailGetDirection(MailFifoMessageType_t msgType)
{
    return (REQUEST_MSG_TYPE == msgType.type
            || INDICATION_MSG_TYPE == msgType.type) ?
           FROM_CLIENT_TO_SERVER :
           FROM_SERVER_TO_CLIENT;
}

/************************************************************************************//**
    \brief Update Check Sum.
    \parem[in] data - data chunk pointer
    \parem[in] length - data chunk length.

    \return NONE.
****************************************************************************************/
static void updateCheckSum(MailCheckSumLength_t *checkSum, uint8_t *data, uint8_t length)
{
    while (length)
    {
        *checkSum += *data++;
        length--;
    }
}

/************************************************************************************//**
    \brief Helper function. Sends a chunk of data to the FIFO while offset isn't equal threshold value or buffer
        is not in full.
    \param[in] adapter - adapter module descriptor.
    \param[in] data - data to be written to stream.
    \param[in] dataSize - size of current data chunk.
    \param[in] startBlockOffset - starting offset of current type of data in byte stream value.
    \param[in] endBlockOffset - ending offset of current data block in byte stream value.
    \param[in] dataType - data flow type. For metadata, the function doesn't update CS and offset.

    \return true If chunk has been sent fully or false otherwise.
 ****************************************************************************************/
static bool fifoSend(MailAdapterDescriptor_t *const adapter,
                     uint8_t *const data, const uint16_t dataSize,
                     const MailAdapterDataType_t dataType,
                     const uint16_t startBlockOffset, const uint16_t endBlockOffset)
{
    uint8_t bytesToSend = MIN(dataSize, adapter->txOptions.reservedSpace);
    bool result = true;

    if (METADATA == dataType)
    {
        HAL_MailboxTx(&adapter->mbFifoDescr, data, bytesToSend);
        adapter->txOptions.reservedSpace -= bytesToSend;
        result = (dataSize == bytesToSend);
        SYS_DbgAssertComplex(result, MAILADAPTER_SEND_METADATA_CAN_NOT_BE_FRAGMENTAD);
    }
    else if (endBlockOffset > adapter->txOptions.offset)
    {
        SYS_DbgAssertComplex(startBlockOffset <= adapter->txOptions.offset, MAILADAPTER_FIFOSEND_0);
        const uint16_t dataOffset = adapter->txOptions.offset - startBlockOffset;
        bytesToSend = MIN(bytesToSend, dataSize - dataOffset);
        HAL_MailboxTx(&adapter->mbFifoDescr, data + dataOffset, bytesToSend);
        adapter->txOptions.reservedSpace -= bytesToSend;
        adapter->txOptions.offset += bytesToSend;

        /* Update Check Sum. */
        if (NOT_DATA_FLOW != dataType)
            updateCheckSum(&(adapter->txOptions.txCheckSum), data + dataOffset, bytesToSend);

        //printf("sent bytes(%d) - %d/%d\n", bytesToSend, adapter->txOptions.offset, endBlockOffset);
        SYS_DbgAssertComplex(endBlockOffset >= adapter->txOptions.offset, MAILADAPTER_FIFOSEND_1);
        result = (endBlockOffset == adapter->txOptions.offset);
    }

    return result;
}

/************************************************************************************//**
    \brief Sends parcel to FIFO buffer.
    \param[in] adapter - adpter module descriptor.
    \param[in] fifoHeader - FIFO header pointer.
    \param[in] header - pointer to request header.
    \param[in] req - request pointer.

    \return true if the parcel is completely sent.
****************************************************************************************/
bool mailAdapterSend(MailAdapterDescriptor_t *const adapter,
                     MailFifoHeader_t *fifoHeader, MailWrappedReqHeader_t *header, uint8_t *const params)
{
    bool isMessageSent = true;
    if (!adapter->readyToSendFlag)
        return false;

    if (NO_TRANSMISSION == adapter->txOptions.txDirection)
    {
        adapter->txOptions.txDirection = mailGetDirection(fifoHeader->msgType);
        SYS_DbgAssertComplex(NULL == adapter->txOptions.currentParcel, MAILADAPTER_BROKEN_PARCEL_SEQUENCE);
        adapter->txOptions.currentParcel = params;
    }
    else if (mailGetDirection(fifoHeader->msgType) != adapter->txOptions.txDirection)
        return false;

    /* fifo header */
    {
        const uint16_t fullMessageSize = sizeof(MailFifoPackedHeader_t) + sizeof(*header)
                                         + header->paramLength + header->dataLength + sizeof(adapter->txOptions.txCheckSum);
        const uint16_t remainingMessageLength = CEIL(fullMessageSize - adapter->txOptions.offset, 4) * 4;
        adapter->txOptions.reservedSpace = MIN(HAL_MailboxTxFifoAvailableSize(&adapter->mbFifoDescr), remainingMessageLength);

        /* prepare fifoheader */
        fifoHeader->fragmentNumber = adapter->txOptions.fragmentCounter;
        fifoHeader->isFragment = !(remainingMessageLength <= adapter->txOptions.reservedSpace);

        MailFifoPackedHeader_t fifoPackedHeader;
        fifoPackedHeader.subSystemId    = fifoHeader->msgType.subSystem;
        fifoPackedHeader.messageId      = fifoHeader->msgId;
        fifoPackedHeader.messageType    = fifoHeader->msgType.type;
        fifoPackedHeader.fragment       = fifoHeader->isFragment;
        fifoPackedHeader.sequenceNumber = fifoHeader->fragmentNumber;
        fifoPackedHeader.protocolVersion = fifoHeader->msgType.version;
        fifoPackedHeader.messageLength  = adapter->txOptions.reservedSpace / 4 - 1; // /* without fifoheader */ - 1;
        fifoSend(adapter, (uint8_t *)&fifoPackedHeader, sizeof(fifoPackedHeader), METADATA, 0U, 0U);
    }

    /* wrapped header */
    {
        const bool sendingStatus = fifoSend(adapter,
                                            (uint8_t *)header, sizeof(*header),
                                            DATA_FLOW,
                                            0U,
                                            sizeof(*header));
        SYS_DbgAssertComplex(sendingStatus, MAILADAPTER_MAILADAPTERSEND_1);
    }

    if (MAIL_INVALID_PAYLOAD_OFFSET == header->dataPointerOffset)
    {
        if (!isMessageSent
                || !fifoSend(adapter,
                             params, header->paramLength,
                             DATA_FLOW,
                             sizeof(*header),
                             sizeof(*header) + header->paramLength)
           )
            isMessageSent = false;
    }
    else
    {
        /* the first part of parameters */
        {
            SYS_DbgAssertComplex(header->dataPointerOffset <= header->paramLength, MAILADAPTER_MAILADAPTERSEND_2);
            if (!isMessageSent
                    || !fifoSend(adapter,
                                 params, header->dataPointerOffset,
                                 DATA_FLOW,
                                 sizeof(*header),
                                 sizeof(*header) + header->dataPointerOffset)
               )
                isMessageSent = false;
        }

        /* the second part of parameters */
        {
            if (!isMessageSent
                    || !fifoSend(adapter,
                                 params + header->dataPointerOffset + sizeof(SYS_DataPointer_t),
                                 header->paramLength - header->dataPointerOffset,
                                 DATA_FLOW,
                                 sizeof(*header) + header->dataPointerOffset,
                                 sizeof(*header) + header->paramLength)
               )
                isMessageSent = false;
        }

        /* data frame */
        {
            if (isMessageSent
                    /* Just skip buffers allocation. */
                    && sizeof(*header) + header->paramLength + header->dataLength > adapter->txOptions.offset)
            {
                const SYS_DataLength_t startIndex = adapter->txOptions.offset - sizeof(*header) - header->paramLength;
                const uint8_t bufferSize = MIN(adapter->txOptions.reservedSpace, header->dataLength - startIndex);
                uint8_t *const dataBuffer = ALLOCA(bufferSize);
                SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(params + header->dataPointerOffset);

                SYS_CopyFromPayload(dataBuffer, dataPointer, startIndex, bufferSize);
                if (!fifoSend(adapter,
                              dataBuffer, bufferSize,
                              DATA_FLOW,
                              sizeof(*header) + header->paramLength + startIndex,
                              sizeof(*header) + header->paramLength + header->dataLength)
                   )
                    isMessageSent = false;
            }
        }
    }


    /* send the check sum data. */
    {
        if (!isMessageSent
                || !fifoSend(adapter,
                             (uint8_t *) & (adapter->txOptions.txCheckSum),
                             sizeof(adapter->txOptions.txCheckSum),
                             NOT_DATA_FLOW,
                             sizeof(*header) + header->paramLength + header->dataLength,
                             sizeof(*header) + header->paramLength + header->dataLength + sizeof(adapter->txOptions.txCheckSum))
           )
            isMessageSent = false;
    }

    /* adds zeros for hardware aligh */
    {
        SYS_DbgAssertComplex(sizeof(uint32_t) > adapter->txOptions.reservedSpace, MAILADAPTER_MAILADAPTERSEND_3);
        const uint32_t zeroField = 0;
        fifoSend(adapter, (uint8_t *)&zeroField, adapter->txOptions.reservedSpace, METADATA, 0U, 0U);
    }

    adapter->readyToSendFlag = false;
    HAL_MailboxTxEnd(&adapter->mbFifoDescr, fifoHeader->msgType.subSystem);

    SYS_DbgAssertComplex(isMessageSent != fifoHeader->isFragment, MAILADAPTER_MAILADAPTERSEND_4);
    if (isMessageSent)
        mailAdapterInitTxOptions(&adapter->txOptions);
    else
        ++adapter->txOptions.fragmentCounter;
    return isMessageSent;
}

/************************************************************************************//**
    \brief Free page indication
    \param[in] mbFifoDescr - shared FIFO descriptor.
****************************************************************************************/
static void mailAdapterReadyToSendInd(HAL_MailboxDescriptor_t *const mbFifoDescr)
{
    MailAdapterDescriptor_t *const adapter = GET_PARENT_BY_FIELD(MailAdapterDescriptor_t, mbFifoDescr, mbFifoDescr);
    MailDescriptor_t *const mail = GET_PARENT_BY_FIELD(MailDescriptor_t, adapter, adapter);

    adapter->readyToSendFlag = true;
    if (NO_TRANSMISSION != adapter->txOptions.txDirection
            || mailClientIsBusy(mail)
            || mailServerIsBusy(mail))
        SYS_SchedulerPostTask(&adapter->taskDescr, READY_TO_SEND_HANDLER_ID);
}

static void mailAdapterReadyToSendHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor)
{
    MailAdapterDescriptor_t *const adapter = GET_PARENT_BY_FIELD(MailAdapterDescriptor_t, taskDescr, taskDescriptor);
    MailDescriptor_t *const mail = GET_PARENT_BY_FIELD(MailDescriptor_t, adapter, adapter);

    switch (adapter->txOptions.txDirection)
    {
        case NO_TRANSMISSION:
        {
#define POLL_HIGH_PRIORITY_PART(priority, mail) ((priority) ? mailServerQueuePoll(mail): mailClientQueuePoll(mail))
#define POLL_LOW_PRIORITY_PART(priority, mail) POLL_HIGH_PRIORITY_PART(!(priority), (mail))
#define CHANGE_POLL_PRIORITY_TO_OPPOSITE(priority) (priority) ^= UINT8_MAX

            if (!POLL_HIGH_PRIORITY_PART(adapter->pollPriority, mail))
                POLL_LOW_PRIORITY_PART(adapter->pollPriority, mail);
            CHANGE_POLL_PRIORITY_TO_OPPOSITE(adapter->pollPriority);
            break;

#undef POLL_HIGH_PRIORITY_PART
#undef POLL_LOW_PRIORITY_PART
        }

        case FROM_CLIENT_TO_SERVER:
            mailClientQueuePoll(mail);
            break;

        case FROM_SERVER_TO_CLIENT:
            mailServerQueuePoll(mail);
            break;

        default:
            SYS_DbgAssertComplex(false, MAILADAPTER_MAILREADYTOSENDHANDLER_0);
            break;
    }
}

static void mailAdapterFifoOfflineInd(HAL_MailboxDescriptor_t *const mbFifoDescr)
{
    MailAdapterDescriptor_t *const adapter = GET_PARENT_BY_FIELD(MailAdapterDescriptor_t, mbFifoDescr, mbFifoDescr);
    adapter->readyToSendFlag = false;
}

/************************************************************************************//**
    \brief Helper function. Received a chunk of data from the FIFO while offset don't equal threshold value
        or buffer is not clear.
    \param[in] mbFifoDescr - shared FIFO descriptor.
    \param[in] rxOptions - pointer to Rx options set
    \param[in] buffer - buffer pointer
    \param[in] bufferSize - size of the buffer
    \param[in] startBlockOffset - starting offset of current type of data in byte stream value.
    \param[in] endBlockOffset - ending offset of current type of data in byte stream value.
    \param[in] dataType - data flow type. For metadata, the function doesn't update CS and offset.

    \return true If chunk has been received fully or false otherwise.
 ****************************************************************************************/
static bool fifoReceive(HAL_MailboxDescriptor_t *mbFifoDescr, MailAdapterRxOptions_t *const rxOptions,
                        uint8_t *const buffer, uint16_t bufferSize,
                        const MailAdapterDataType_t dataType,
                        const uint16_t startBlockOffset, const uint16_t endBlockOffset)
{
    uint8_t bytesToReceive = MIN(bufferSize, rxOptions->bytesToReceive);
    bool result = true;

    if (METADATA == dataType)
    {
        HAL_MailboxRx(mbFifoDescr, buffer, bytesToReceive);
        rxOptions->bytesToReceive -= bytesToReceive;
        result = (bufferSize == bytesToReceive);
        SYS_DbgAssertComplex(result, MAILADAPTER_RECEIVE_METADATA_CAN_NOT_BE_FRAGMENTAD);
    }
    else if (endBlockOffset > rxOptions->offset)
    {
        SYS_DbgAssertComplex(startBlockOffset <= rxOptions->offset, MAILADAPTER_FIFORECEIVE_0);
        const uint16_t bufferOffset = rxOptions->offset - startBlockOffset;
        bytesToReceive = MIN(bytesToReceive, bufferSize - bufferOffset);
        HAL_MailboxRx(mbFifoDescr, buffer + bufferOffset, bytesToReceive);
        rxOptions->bytesToReceive -= bytesToReceive;
        rxOptions->offset += bytesToReceive;

        /* Update Check Sum. */
        if (NOT_DATA_FLOW != dataType)
        updateCheckSum(&(rxOptions->rxCheckSumCalc), buffer + bufferOffset, bytesToReceive);

        //printf("received bytes(%d) - %d/%d\n", bytesToReceive, rxOptions->offset, endBlockOffset);
        SYS_DbgAssertComplex(endBlockOffset >= rxOptions->offset, MAILADAPTER_FIFORECEIVE_1);
        result = (endBlockOffset == rxOptions->offset);
    }
    return result;
}

static void mailAdapterReceivedInd(HAL_MailboxDescriptor_t *const mbFifoDescr)
{
    MailAdapterDescriptor_t *const adapter = GET_PARENT_BY_FIELD(MailAdapterDescriptor_t, mbFifoDescr, mbFifoDescr);

    SYS_SchedulerPostTask(&adapter->taskDescr, RECEIVE_HANDLER_ID);
}

/************************************************************************************//**
    \brief Indication about received chunk of parcel.
    \param[in] mbFifoDescr - shared FIFO descriptor.
****************************************************************************************/
static void mailAdapterDataReceivedHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor)
{
    MailAdapterDescriptor_t *const adapter = GET_PARENT_BY_FIELD(MailAdapterDescriptor_t, taskDescr, taskDescriptor);
    MailAdapterRxOptions_t *const rxOptions = &adapter->rxOptions[0];
    MailWrappedReqHeader_t *const header = &rxOptions->header;
    MailFifoHeader_t fifoHeader;
    bool isMessageReceived = false;

    /* receive fifoheader */
    {
        MailFifoPackedHeader_t fifoPackedHeader;
        rxOptions->bytesToReceive = sizeof(MailFifoPackedHeader_t);
        fifoReceive(&adapter->mbFifoDescr, rxOptions, (uint8_t *)&fifoPackedHeader, sizeof(fifoPackedHeader), METADATA, 0U, 0U);

        fifoHeader.msgId            = fifoPackedHeader.messageId;
        fifoHeader.isFragment       = fifoPackedHeader.fragment;
        fifoHeader.fragmentNumber   = fifoPackedHeader.sequenceNumber;
        fifoHeader.msgType.version  = fifoPackedHeader.protocolVersion;
        fifoHeader.msgType.subSystem = fifoPackedHeader.subSystemId;
        fifoHeader.msgType.type     = fifoPackedHeader.messageType;
        //rxOptions->bytesToReceive = (fifoPackedHeader.messageLength + 1) * 4;
        rxOptions->bytesToReceive = (fifoPackedHeader.messageLength) * 4;
        SYS_DbgAssertComplex(rxOptions->bytesToReceive, MAILADAPTER_MAILRECEIVEIND_1);
    }

    /* Checks the fragments consistency and starts reception of a new parcel if needed  */
    {
        MailAdapterDirection_t direction = mailGetDirection(fifoHeader.msgType);
        if (rxOptions->nextFragmentNumber != fifoHeader.fragmentNumber
                || (NO_TRANSMISSION != rxOptions->rxDirection
                    && rxOptions->rxDirection != direction))
        {
            SYS_DbgLogId(MAILADAPTER_MAYBE_FRAGMENT_HAS_BEEN_MISSED);
            if (FROM_CLIENT_TO_SERVER == rxOptions->rxDirection)
                mailFreeServerBuffer(adapter, rxOptions->params);
            if (SYS_CheckPayload(&rxOptions->dataPointer))
                SYS_FreePayload(&rxOptions->dataPointer);
            mailInitRxOptions(rxOptions);
        }

        /* new message */
        if (0 == fifoHeader.fragmentNumber)
        {
            /* wrapped header */
            const bool receivingStatus = fifoReceive(&adapter->mbFifoDescr, rxOptions,
                                         (uint8_t *)header, sizeof(*header),
                                         DATA_FLOW,
                                         0U,
                                         sizeof(*header));
            SYS_DbgAssertComplex(receivingStatus, MAILADAPTER_MAILRECEIVEIND_3);

            rxOptions->params = (FROM_SERVER_TO_CLIENT == direction) ?
                                mailClientGetMemory(adapter, &fifoHeader, header) :
                                mailServerGetMemory(adapter, &fifoHeader, header);
            rxOptions->rxDirection = (NULL != rxOptions->params) ?
                                     direction : NO_TRANSMISSION;

        }

        if (NO_TRANSMISSION == rxOptions->rxDirection)
        {
            SYS_DbgLogId(NO_ALLOCATED_BUFFER_FOR_INCOMING_PARCEL);
            while (rxOptions->bytesToReceive)
            {
                uint32_t dummy;
                fifoReceive(&adapter->mbFifoDescr, rxOptions, (uint8_t *)&dummy, sizeof(dummy), METADATA, 0U, 0U);
            }
            mailInitRxOptions(rxOptions);
            HAL_MailboxRxEnd(&adapter->mbFifoDescr);
            return;
        }
    }

    if (MAIL_INVALID_PAYLOAD_OFFSET == header->dataPointerOffset)
    {
        if (isMessageReceived
                || !fifoReceive(&adapter->mbFifoDescr, rxOptions,
                                (uint8_t *)rxOptions->params, header->paramLength,
                                DATA_FLOW,
                                sizeof(*header),
                                sizeof(*header) + header->paramLength)
           )
            isMessageReceived = true;
    }
    else
    {
        SYS_DbgAssertComplex(header->dataPointerOffset <= header->paramLength, MAILADAPTER_MAILRECEIVEIND_7);
        /* the first part of the parameters */
        {
            if (isMessageReceived
                    || !fifoReceive(&adapter->mbFifoDescr, rxOptions,
                                    (uint8_t *)rxOptions->params, header->dataPointerOffset,
                                    DATA_FLOW,
                                    sizeof(*header),
                                    sizeof(*header) + header->dataPointerOffset)
               )
                isMessageReceived = true;
        }

        /* the second part of the parameters */
        {
            if (isMessageReceived
                    || !fifoReceive(&adapter->mbFifoDescr, rxOptions,
                                    (uint8_t *)rxOptions->params + header->dataPointerOffset + sizeof(SYS_DataPointer_t),
                                    header->paramLength - header->dataPointerOffset,
                                    DATA_FLOW,
                                    sizeof(*header) + header->dataPointerOffset,
                                    sizeof(*header) + header->paramLength)
               )
                isMessageReceived = true;
        }

        /* data frame */
        {
            if (!isMessageReceived
                    /* Just skip buffers allocation. */
                    && sizeof(*header) + header->paramLength + header->dataLength > rxOptions->offset)
            {
                const SYS_DataLength_t startIndex = rxOptions->offset - sizeof(*header) - header->paramLength;
                const uint8_t bufferSize = MIN(rxOptions->bytesToReceive, header->dataLength - startIndex);
                uint8_t *const dataBuffer = ALLOCA(bufferSize);
                if (!fifoReceive(&adapter->mbFifoDescr, rxOptions,
                                 dataBuffer, bufferSize,
                                 DATA_FLOW,
                                 sizeof(*header) + header->paramLength + startIndex,
                                 sizeof(*header) + header->paramLength + header->dataLength)
                   )
                    isMessageReceived = true;

                if (!SYS_CheckPayload(&rxOptions->dataPointer))
                    SYS_DbgAssert(SYS_MemAlloc(&rxOptions->dataPointer, header->dataLength), MAILADAPTER_MAILRECEIVEIND_9);

                SYS_CopyToPayload(&rxOptions->dataPointer, startIndex, dataBuffer, bufferSize);
            }
        }
    }

    /* get the check sum data. */
    {
        if (isMessageReceived
                || !fifoReceive(&adapter->mbFifoDescr, rxOptions,
                                (uint8_t *) & (rxOptions->rxCheckSum),
                                sizeof(rxOptions->rxCheckSum),
                                NOT_DATA_FLOW,
                                sizeof(*header) + header->paramLength + header->dataLength,
                                sizeof(*header) + header->paramLength + header->dataLength + sizeof(rxOptions->rxCheckSum))
           )
            isMessageReceived = true;
    }

    /* read addition bytes, uses for aligh */
    {
        SYS_DbgAssertComplex(sizeof(uint32_t) > rxOptions->bytesToReceive, MAILADAPTER_MAILRECEIVEIND_10);
        const uint32_t zeroField = 0;
        fifoReceive(&adapter->mbFifoDescr, rxOptions, (uint8_t *)&zeroField, rxOptions->bytesToReceive, METADATA, 0U, 0U);
    }

    HAL_MailboxRxEnd(&adapter->mbFifoDescr);
    if (!fifoHeader.isFragment)
    {
        /* check the integrity of received data. */
        if (rxOptions->rxCheckSumCalc == rxOptions->rxCheckSum)
        {
            /* merge data pointer to the request */
            if (MAIL_INVALID_PAYLOAD_OFFSET != header->dataPointerOffset)
            {
                SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *) ((uint8_t *) rxOptions->params + header->dataPointerOffset);
                memcpy(dataPointer, &rxOptions->dataPointer, sizeof(rxOptions->dataPointer));
                SYS_SetEmptyPayload(&rxOptions->dataPointer);
            }

            if (FROM_CLIENT_TO_SERVER == rxOptions->rxDirection)
                mailServerDataInd(adapter, &fifoHeader, header, (uint8_t *) rxOptions->params);
            else
                mailClientDataInd(adapter, &fifoHeader, header, (uint8_t *) rxOptions->params);
        }
        else
        {
            if (FROM_CLIENT_TO_SERVER == rxOptions->rxDirection)
                mailFreeServerBuffer(adapter, rxOptions->params);
            if (SYS_CheckPayload(&rxOptions->dataPointer))
                SYS_FreePayload(&rxOptions->dataPointer);
        }

        mailInitRxOptions(rxOptions);
    }
    else
        ++rxOptions->nextFragmentNumber;
}
