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
 *      implementation of mailbox adapter module
 *
*******************************************************************************/

/************************* INCLUDES ****************************************************/
#include "private/bbMailPrivateAdapter.h"
#include "private/bbMailPrivateClient.h"
#include "private/bbMailPrivateServer.h"

/************************* STATIC FUNCTIONS PROTOTYPES *********************************/
static void mailAdapterReset(void);
static void mailAdapterFifoOfflineInd(HAL_MailboxDescriptor_t *const mbFifoDescr);
static void mailAdapterReadyToSendInd(HAL_MailboxDescriptor_t *const mbFifoDescr);
static void mailAdapterReceivedInd(HAL_MailboxDescriptor_t *const mbFifoDescr);
static void mailAdapterReadyToSendHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
static void mailAdapterDataReceivedHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

/************************* IMPLEMENTATION **********************************************/
static MailAdapterDescriptor_t adapterMemory;
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
void mailAdapterInit(void)
{
    mailAdapterReset();

    adapterMemory.mbFifoDescr.offlineCallback    = mailAdapterFifoOfflineInd;
    adapterMemory.mbFifoDescr.rtsCallback        = mailAdapterReadyToSendInd;
    adapterMemory.mbFifoDescr.rxCallback         = mailAdapterReceivedInd;

    HAL_MailboxInit(&adapterMemory.mbFifoDescr);
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
void mailAdapterCancelTx(uint8_t *const parcel)
{
    if (parcel == adapterMemory.txOptions.currentParcel)
    {
        mailAdapterInitTxOptions(&adapterMemory.txOptions);
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
static void mailAdapterReset()
{
    adapterMemory.taskDescr.priority = SYS_SCHEDULER_MAILBOX_PRIORITY;
    adapterMemory.taskDescr.handlers = mailAdapterHandlers;
    adapterMemory.pollPriority = 0; /* client's part has more priority by default */

    adapterMemory.readyToSendFlag = false;

    mailAdapterInitTxOptions(&adapterMemory.txOptions);
    SYS_SetEmptyPayload(&adapterMemory.rxOptions.dataPointer);
    mailInitRxOptions(&adapterMemory.rxOptions);
}

/************************************************************************************//**
    \brief Helper function. Get parcel direction.
    \param[in] msgType - pointer to the structure with parcel information.

    \return FROM_CLIENT_TO_SERVER or FROM_SERVER_TO_CLIENT
****************************************************************************************/
INLINE MailAdapterDirection_t mailGetDirection(MailFifoMessageType_t msgType)
{
    return msgType.isConfirm ? FROM_SERVER_TO_CLIENT : FROM_CLIENT_TO_SERVER;
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
    \param[in] data - data to be written to stream.
    \param[in] dataSize - size of current data chunk.
    \param[in] startBlockOffset - starting offset of current type of data in byte stream value.
    \param[in] endBlockOffset - ending offset of current data block in byte stream value.
    \param[in] dataType - data flow type. For metadata, the function doesn't update CS and offset.

    \return true If chunk has been sent fully or false otherwise.
 ****************************************************************************************/
static bool fifoSend(uint8_t *const data, const uint16_t dataSize,
                     const MailAdapterDataType_t dataType,
                     const uint16_t startBlockOffset, const uint16_t endBlockOffset)
{
    uint8_t bytesToSend = MIN(dataSize, adapterMemory.txOptions.reservedSpace);
    bool result = true;

    if (METADATA == dataType)
    {
        HAL_MailboxTx(&adapterMemory.mbFifoDescr, data, bytesToSend);
        adapterMemory.txOptions.reservedSpace -= bytesToSend;
        result = (dataSize == bytesToSend);
        SYS_DbgAssertComplex(result, MAILADAPTER_SEND_METADATA_CAN_NOT_BE_FRAGMENTAD);
    }
    else if (endBlockOffset > adapterMemory.txOptions.offset)
    {
        SYS_DbgAssertComplex(startBlockOffset <= adapterMemory.txOptions.offset, MAILADAPTER_FIFOSEND_0);
        const uint16_t dataOffset = adapterMemory.txOptions.offset - startBlockOffset;
        bytesToSend = MIN(bytesToSend, dataSize - dataOffset);
        HAL_MailboxTx(&adapterMemory.mbFifoDescr, data + dataOffset, bytesToSend);
        adapterMemory.txOptions.reservedSpace -= bytesToSend;
        adapterMemory.txOptions.offset += bytesToSend;

        /* Update Check Sum. */
        if (NOT_DATA_FLOW != dataType)
            updateCheckSum(&(adapterMemory.txOptions.txCheckSum), data + dataOffset, bytesToSend);

        //printf("sent bytes(%d) - %d/%d\n", bytesToSend, adapterMemory.txOptions.offset, endBlockOffset);
        SYS_DbgAssertComplex(endBlockOffset >= adapterMemory.txOptions.offset, MAILADAPTER_FIFOSEND_1);
        result = (endBlockOffset == adapterMemory.txOptions.offset);
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
bool mailAdapterSend(MailFifoHeader_t *fifoHeader, MailWrappedReqHeader_t *header, uint8_t *const params)
{
    bool isMessageSent = true;
    if (!adapterMemory.readyToSendFlag)
        return false;

    if (NO_TRANSMISSION == adapterMemory.txOptions.txDirection)
    {
        adapterMemory.txOptions.txDirection = mailGetDirection(fifoHeader->msgType);
        SYS_DbgAssertComplex(NULL == adapterMemory.txOptions.currentParcel, MAILADAPTER_BROKEN_PARCEL_SEQUENCE);
        adapterMemory.txOptions.currentParcel = params;
    }
    else if (mailGetDirection(fifoHeader->msgType) != adapterMemory.txOptions.txDirection)
        return false;

    /* fifo header */
    {
        const uint16_t fullMessageSize = sizeof(MailFifoPackedHeader_t) + sizeof(*header)
                                         + header->paramLength + header->dataLength + sizeof(adapterMemory.txOptions.txCheckSum);
        const uint16_t remainingMessageLength = CEIL(fullMessageSize - adapterMemory.txOptions.offset, 4) * 4;
        adapterMemory.txOptions.reservedSpace = MIN(HAL_MailboxTxFifoAvailableSize(&adapterMemory.mbFifoDescr), remainingMessageLength);

        /* prepare fifoheader */
        fifoHeader->fragmentNumber = adapterMemory.txOptions.fragmentCounter;
        fifoHeader->isFragment = !(remainingMessageLength <= adapterMemory.txOptions.reservedSpace);

        MailFifoPackedHeader_t fifoPackedHeader;
        fifoPackedHeader.subSystemId    = fifoHeader->msgType.subSystem;
        fifoPackedHeader.messageId      = fifoHeader->msgId;
        fifoPackedHeader.fromStackSide  = fifoHeader->msgType.fromStackSide;
        fifoPackedHeader.isConfirm      = fifoHeader->msgType.isConfirm;
        fifoPackedHeader.fragment       = fifoHeader->isFragment;
        fifoPackedHeader.sequenceNumber = fifoHeader->fragmentNumber;
        fifoPackedHeader.protocolVersion = fifoHeader->msgType.version;
        fifoPackedHeader.messageLength  = adapterMemory.txOptions.reservedSpace / 4 - 1; // /* without fifoheader */ - 1;
        fifoSend((uint8_t *)&fifoPackedHeader, sizeof(fifoPackedHeader), METADATA, 0U, 0U);
    }

    /* wrapped header */
    {
        const bool sendingStatus = fifoSend((uint8_t *)header, sizeof(*header),
                                            DATA_FLOW,
                                            0U,
                                            sizeof(*header));
        SYS_DbgAssertComplex(sendingStatus, MAILADAPTER_MAILADAPTERSEND_1);
    }

    uint8_t dataPointerOffset = (FROM_CLIENT_TO_SERVER == adapterMemory.txOptions.txDirection) ?
                                Mail_ServiceGetFunctionInfo(fifoHeader->msgId)->reqDataPointerOffset :
                                Mail_ServiceGetFunctionInfo(fifoHeader->msgId)->confDataPointerOffset;
    if (MAIL_INVALID_OFFSET == dataPointerOffset)
    {
        if (!isMessageSent
                || !fifoSend(params, header->paramLength,
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
            SYS_DbgAssertComplex(dataPointerOffset <= header->paramLength, MAILADAPTER_MAILADAPTERSEND_2);
            if (!isMessageSent
                    || !fifoSend(params, dataPointerOffset,
                                 DATA_FLOW,
                                 sizeof(*header),
                                 sizeof(*header) + dataPointerOffset)
               )
                isMessageSent = false;
        }

        /* the second part of parameters */
        {
            if (!isMessageSent
                    || !fifoSend(params + dataPointerOffset + sizeof(SYS_DataPointer_t),
                                 header->paramLength - dataPointerOffset,
                                 DATA_FLOW,
                                 sizeof(*header) + dataPointerOffset,
                                 sizeof(*header) + header->paramLength)
               )
                isMessageSent = false;
        }

        /* data frame */
        {
            if (isMessageSent
                    /* Just skip buffers allocation. */
                    && sizeof(*header) + header->paramLength + header->dataLength > adapterMemory.txOptions.offset)
            {
                const SYS_DataLength_t startIndex = adapterMemory.txOptions.offset - sizeof(*header) - header->paramLength;
                const uint8_t bufferSize = MIN(adapterMemory.txOptions.reservedSpace, header->dataLength - startIndex);
                uint8_t *const dataBuffer = ALLOCA(bufferSize);
                SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *)(params + dataPointerOffset);

                SYS_CopyFromPayload(dataBuffer, dataPointer, startIndex, bufferSize);
                if (!fifoSend(dataBuffer, bufferSize,
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
                || !fifoSend((uint8_t *) & (adapterMemory.txOptions.txCheckSum),
                             sizeof(adapterMemory.txOptions.txCheckSum),
                             NOT_DATA_FLOW,
                             sizeof(*header) + header->paramLength + header->dataLength,
                             sizeof(*header) + header->paramLength + header->dataLength + sizeof(adapterMemory.txOptions.txCheckSum))
           )
            isMessageSent = false;
    }

    /* adds zeros for hardware aligh */
    {
        SYS_DbgAssertComplex(sizeof(uint32_t) > adapterMemory.txOptions.reservedSpace, MAILADAPTER_MAILADAPTERSEND_3);
        const uint32_t zeroField = 0;
        fifoSend((uint8_t *)&zeroField, adapterMemory.txOptions.reservedSpace, METADATA, 0U, 0U);
    }

    adapterMemory.readyToSendFlag = false;
    HAL_MailboxTxEnd(&adapterMemory.mbFifoDescr, fifoHeader->msgType.subSystem);

    SYS_DbgAssertComplex(isMessageSent != fifoHeader->isFragment, MAILADAPTER_MAILADAPTERSEND_4);
    if (isMessageSent)
        mailAdapterInitTxOptions(&adapterMemory.txOptions);
    else
        ++adapterMemory.txOptions.fragmentCounter;
    return isMessageSent;
}

/************************************************************************************//**
    \brief Free page indication
    \param[in] mbFifoDescr - shared FIFO descriptor.
****************************************************************************************/
static void mailAdapterReadyToSendInd(HAL_MailboxDescriptor_t *const mbFifoDescr)
{
    (void)mbFifoDescr;
    adapterMemory.readyToSendFlag = true;
    if (NO_TRANSMISSION != adapterMemory.txOptions.txDirection
            || mailClientIsBusy()
            || mailServerIsBusy())
        SYS_SchedulerPostTask(&adapterMemory.taskDescr, READY_TO_SEND_HANDLER_ID);
}

#ifdef _HOST_
void mailAdapterTryToSend()
{
    SYS_SchedulerPostTask(&adapterMemory.taskDescr, READY_TO_SEND_HANDLER_ID);
}
#endif  // _HOST_

static void mailAdapterReadyToSendHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor)
{
    (void)taskDescriptor;
    switch (adapterMemory.txOptions.txDirection)
    {
        case NO_TRANSMISSION:
        {
#define POLL_HIGH_PRIORITY_PART(priority) ((priority) ? mailServerQueuePoll(): mailClientQueuePoll())
#define POLL_LOW_PRIORITY_PART(priority) POLL_HIGH_PRIORITY_PART(!(priority))
#define CHANGE_POLL_PRIORITY_TO_OPPOSITE(priority) (priority) ^= UINT8_MAX

            if (!POLL_HIGH_PRIORITY_PART(adapterMemory.pollPriority))
                POLL_LOW_PRIORITY_PART(adapterMemory.pollPriority);
            CHANGE_POLL_PRIORITY_TO_OPPOSITE(adapterMemory.pollPriority);
            break;

#undef POLL_HIGH_PRIORITY_PART
#undef POLL_LOW_PRIORITY_PART
        }

        case FROM_CLIENT_TO_SERVER:
            mailClientQueuePoll();
            break;

        case FROM_SERVER_TO_CLIENT:
            mailServerQueuePoll();
            break;

        default:
            SYS_DbgAssertComplex(false, MAILADAPTER_MAILREADYTOSENDHANDLER_0);
            break;
    }
}

static void mailAdapterFifoOfflineInd(HAL_MailboxDescriptor_t *const mbFifoDescr)
{
    (void)mbFifoDescr;
    adapterMemory.readyToSendFlag = false;
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
    (void)mbFifoDescr;
    SYS_SchedulerPostTask(&adapterMemory.taskDescr, RECEIVE_HANDLER_ID);
}

/************************************************************************************//**
    \brief Indication about received chunk of parcel.
    \param[in] mbFifoDescr - shared FIFO descriptor.
****************************************************************************************/
static void mailAdapterDataReceivedHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor)
{
    (void)taskDescriptor;
    MailAdapterRxOptions_t *const rxOptions = &adapterMemory.rxOptions;
    MailWrappedReqHeader_t *const header = &rxOptions->header;
    MailFifoHeader_t fifoHeader;
    bool isMessageReceived = false;

    /* receive fifoheader */
    {
        MailFifoPackedHeader_t fifoPackedHeader;
        rxOptions->bytesToReceive = sizeof(MailFifoPackedHeader_t);
        fifoReceive(&adapterMemory.mbFifoDescr, rxOptions, (uint8_t *)&fifoPackedHeader, sizeof(fifoPackedHeader), METADATA, 0U, 0U);

        fifoHeader.msgId                    = fifoPackedHeader.messageId;
        fifoHeader.isFragment               = fifoPackedHeader.fragment;
        fifoHeader.fragmentNumber           = fifoPackedHeader.sequenceNumber;
        fifoHeader.msgType.version          = fifoPackedHeader.protocolVersion;
        fifoHeader.msgType.subSystem        = fifoPackedHeader.subSystemId;
        fifoHeader.msgType.fromStackSide    = fifoPackedHeader.fromStackSide;
        fifoHeader.msgType.isConfirm        = fifoPackedHeader.isConfirm;
        //rxOptions->bytesToReceive = (fifoPackedHeader.messageLength + 1) * 4;
        rxOptions->bytesToReceive = (fifoPackedHeader.messageLength) * 4;
        SYS_DbgAssertComplex(rxOptions->bytesToReceive, MAILADAPTER_MAILRECEIVEIND_1);
    }
    const MailAdapterDirection_t direction = mailGetDirection(fifoHeader.msgType);

    /* Checks the fragments consistency and starts reception of a new parcel if needed  */
    {
        if (rxOptions->nextFragmentNumber != fifoHeader.fragmentNumber
                || (NO_TRANSMISSION != rxOptions->rxDirection
                    && rxOptions->rxDirection != direction))
        {
            SYS_DbgLogId(MAILADAPTER_MAYBE_FRAGMENT_HAS_BEEN_MISSED);
            if (FROM_CLIENT_TO_SERVER == rxOptions->rxDirection)
                mailFreeServerBuffer(rxOptions->params);
            if (SYS_CheckPayload(&rxOptions->dataPointer))
                SYS_FreePayload(&rxOptions->dataPointer);
            mailInitRxOptions(rxOptions);
        }

        /* new message */
        if (0 == fifoHeader.fragmentNumber)
        {
            /* wrapped header */
            const bool receivingStatus = fifoReceive(&adapterMemory.mbFifoDescr, rxOptions,
                                         (uint8_t *)header, sizeof(*header),
                                         DATA_FLOW,
                                         0U,
                                         sizeof(*header));
            SYS_DbgAssertComplex(receivingStatus, MAILADAPTER_MAILRECEIVEIND_3);

            rxOptions->params = (FROM_SERVER_TO_CLIENT == direction) ?
                                mailClientGetMemory(&fifoHeader, header) :
                                mailServerGetMemory(&fifoHeader, header);
            rxOptions->rxDirection = (NULL != rxOptions->params) ?
                                     direction : NO_TRANSMISSION;
        }

        if (NO_TRANSMISSION == rxOptions->rxDirection)
        {
            if (TE_MAILBOX_ACK_FID != fifoHeader.msgId)
                SYS_DbgLogId(NO_ALLOCATED_BUFFER_FOR_INCOMING_PARCEL);
            while (rxOptions->bytesToReceive)
            {
                uint32_t dummy;
                fifoReceive(&adapterMemory.mbFifoDescr, rxOptions, (uint8_t *)&dummy, sizeof(dummy), METADATA, 0U, 0U);
            }
            mailInitRxOptions(rxOptions);
            HAL_MailboxRxEnd(&adapterMemory.mbFifoDescr);
            return;
        }
    }

    const uint8_t dataPointerOffset = (FROM_SERVER_TO_CLIENT == direction) ?
                                Mail_ServiceGetFunctionInfo(fifoHeader.msgId)->confDataPointerOffset :
                                Mail_ServiceGetFunctionInfo(fifoHeader.msgId)->reqDataPointerOffset;
    if (MAIL_INVALID_OFFSET == dataPointerOffset)
    {
        if (isMessageReceived
                || !fifoReceive(&adapterMemory.mbFifoDescr, rxOptions,
                                (uint8_t *)rxOptions->params, header->paramLength,
                                DATA_FLOW,
                                sizeof(*header),
                                sizeof(*header) + header->paramLength)
           )
            isMessageReceived = true;
    }
    else
    {
        SYS_DbgAssertComplex(dataPointerOffset <= header->paramLength, MAILADAPTER_MAILRECEIVEIND_7);
        /* the first part of the parameters */
        {
            if (isMessageReceived
                    || !fifoReceive(&adapterMemory.mbFifoDescr, rxOptions,
                                    (uint8_t *)rxOptions->params, dataPointerOffset,
                                    DATA_FLOW,
                                    sizeof(*header),
                                    sizeof(*header) + dataPointerOffset)
               )
                isMessageReceived = true;
        }

        /* the second part of the parameters */
        {
            if (isMessageReceived
                    || !fifoReceive(&adapterMemory.mbFifoDescr, rxOptions,
                                    (uint8_t *)rxOptions->params + dataPointerOffset + sizeof(SYS_DataPointer_t),
                                    header->paramLength - dataPointerOffset,
                                    DATA_FLOW,
                                    sizeof(*header) + dataPointerOffset,
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
                if (!fifoReceive(&adapterMemory.mbFifoDescr, rxOptions,
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
                || !fifoReceive(&adapterMemory.mbFifoDescr, rxOptions,
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
        fifoReceive(&adapterMemory.mbFifoDescr, rxOptions, (uint8_t *)&zeroField, rxOptions->bytesToReceive, METADATA, 0U, 0U);
    }

    HAL_MailboxRxEnd(&adapterMemory.mbFifoDescr);
    if (!fifoHeader.isFragment)
    {
        /* check the integrity of received data. */
        if (rxOptions->rxCheckSumCalc == rxOptions->rxCheckSum)
        {
            /* merge data pointer to the request */
            if (MAIL_INVALID_OFFSET != dataPointerOffset)
            {
                SYS_DataPointer_t *const dataPointer = (SYS_DataPointer_t *) ((uint8_t *) rxOptions->params + dataPointerOffset);
                memcpy(dataPointer, &rxOptions->dataPointer, sizeof(rxOptions->dataPointer));
                SYS_SetEmptyPayload(&rxOptions->dataPointer);
            }

            if (FROM_CLIENT_TO_SERVER == rxOptions->rxDirection)
                mailServerDataInd(&fifoHeader, header, (uint8_t *) rxOptions->params);
            else
                mailClientDataInd(&fifoHeader, header, (uint8_t *) rxOptions->params);
        }
        else
        {
            if (FROM_CLIENT_TO_SERVER == rxOptions->rxDirection)
                mailFreeServerBuffer(rxOptions->params);
            if (SYS_CheckPayload(&rxOptions->dataPointer))
                SYS_FreePayload(&rxOptions->dataPointer);
        }

        mailInitRxOptions(rxOptions);
    }
    else
        ++rxOptions->nextFragmentNumber;
}

/* eof bbMailAdapter.c */
