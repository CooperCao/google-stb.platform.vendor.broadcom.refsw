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
 *      private declaration of the mailbox client module.
 *
*******************************************************************************/

#ifndef _MAILPRIVATE_CLIENT_H
#define _MAILPRIVATE_CLIENT_H

/************************* INCLUDES ****************************************************/
#include "bbSysTimeoutTask.h"
#include "bbHalSystemTimer.h"
#include "private/bbMailPrivateService.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Acknowledgeable status enumeration.
 */
typedef enum _MailState_t
{
#ifndef _HOST_
    MAIL_ENTRY_IDLE = 0x00U,
    MAIL_ENTRY_ALLOCATED,
    MAIL_PARCEL_AWAIT_FOR_SEND,
    MAIL_WAITING_FOR_ACK,
    MAIL_WAITING_FOR_CONF,
#else   // _HOST_
    MAIL_PARCEL_AWAIT_FOR_SEND = 0x00U,
    MAIL_ACK_AWAIT_FOR_SEND,
    MAIL_ACK_NOT_NEEDED,
    MAIL_WAITING_FOR_ACK,
    MAIL_ACK_SENT,
    MAIL_WAITING_FOR_CONF,
#endif  // _HOST_
} MailState_t;

/**//**
 * \brief Type of entry of client's delayed requests
 */
typedef struct _MailPendingAPICall_t
{
    /* Pointer to the next task in a queue. */
    SYS_QueueElement_t elem;
    /* Original Request pointer */
    void *originalReq;
    /* function number that has been invoked */
    uint16_t fId;
    /* Request unique number */
    uint8_t uId;
    uint8_t shipmentsCounter;
    /* Request parameters */
    union
    {
        MailReqParams_t  req;
        MailConfParams_t conf;
        MailIndParams_t  ind;
    } params;
    /* service parcel */
    union
    {
        MailServiceReq_t        req;
        MailServiceConfParams_t conf;
    } serviceParcel;
    /* Callback pointer */
    ConfirmCall_t callback;
    /* When timestamp will expired client tries to repeat a parcel transfer */
    HAL_SystemTimestamp_t dueTimestamp;
    /* Acknowledgment state */
    MailState_t state;
} MailPendingAPICall_t;

/**//**
 * \brief Type of client module descriptor.
 */
typedef struct _MailClientDescriptor_t
{
    /* Descriptor of queue of postpone calls */
    SYS_QueueDescriptor_t   parcelQueue;
#ifdef _HOST_
    /* The mutex to protect the parcelQueue to support multiple thread */
    pthread_mutex_t         parcelQueueMutex;
    /* The mutex to protect the pending Table to support multiple thread */
    pthread_mutex_t         pendingTableMutex;
#endif  // _HOST_
    /* Descriptor of queue of service message */
    SYS_QueueDescriptor_t   serviceMessageQueue;
    /* request unique Id counter */
    uint8_t                 uIdCounter;
    /* Table of pending calls */
    MailPendingAPICall_t    pendingTable[MAIL_CLIENT_MAX_AMOUNT_PENDING_CALLS];
    /* Timer for acknowledgment feature */
    SYS_TimeoutSignal_t     ackTimer;
} MailClientDescriptor_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief initializes mailbox client module
****************************************************************************************/
void mailClientInit(void);

/************************************************************************************//**
    \brief Find first empty entry in postponed call table.
    \return Pointer to the found entry.
****************************************************************************************/
MailPendingAPICall_t *mailClientFindEmptyPendingTableEntry(void);

/************************************************************************************//**
    \brief serializes the received wrapper
    \param[in] mail - mailbox descriptor.
    \param[in] fId - number of invoked wrapper.
    \param[in] req - request pointer.
****************************************************************************************/
void mailClientSerialize(const uint16_t fId, uint8_t *const req);

/************************************************************************************//**
    \brief Helper function. Checks service state.
    \param[in] mail - mailbox descriptor.
    \return true if service is busy otherwise false.
****************************************************************************************/
bool mailClientIsBusy(void);

/************************************************************************************//**
    \brief This function called by the mailbox adapter when the adapter checks
        the client's sending queue.
    \param[in] mail - mailbox descriptor.

    \return true if the server's queue is not empty and false otherwise.
****************************************************************************************/
bool mailClientQueuePoll(void);

/************************************************************************************//**
    \brief This function should be invoked by the adapter when it has a parcel
        for the mailbox client.
    \param[in] fifoHeader - FIFO header pointer.
    \param[in] header - request header pointer.
    \param[in] req - request pointer.
****************************************************************************************/
void mailClientDataInd(MailFifoHeader_t *fifoHeader, MailWrappedReqHeader_t *const header, uint8_t *req);

/************************************************************************************//**
    \brief Helper function. Parses the parcel header and allocates memory for a parcel.
    \param[in] header - request header pointer.
****************************************************************************************/
uint8_t *mailClientGetMemory(MailFifoHeader_t *fifoHeader, MailWrappedReqHeader_t *const header);

#endif /* _MAILPRIVATE_CLIENT_H */

/* eof bbMailPrivateClient.h */