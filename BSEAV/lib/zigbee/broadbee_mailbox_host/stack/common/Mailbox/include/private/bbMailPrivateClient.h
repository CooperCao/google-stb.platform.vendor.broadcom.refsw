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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/include/private/bbMailPrivateClient.h $
*
* DESCRIPTION: private declaration of the mailbox client module.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/

#ifndef _MAILPRIVATE_CLIENT_H
#define _MAILPRIVATE_CLIENT_H

/************************* INCLUDES ****************************************************/
#include "bbMailService.h"

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief initializes mailbox client module
    \param[in] mail - mailbox descriptor.
****************************************************************************************/
void mailClientInit(MailDescriptor_t *const mail);

/************************************************************************************//**
    \brief serializes the received wrapper
    \param[in] mail - mailbox descriptor.
    \param[in] fId - number of invoked wrapper.
    \param[in] req - request pointer.
****************************************************************************************/
void mailClientSerialize(MailDescriptor_t *const mail, const uint16_t fId, uint8_t *const req);

/************************************************************************************//**
    \brief Helper function. Checks service state.
    \param[in] mail - mailbox descriptor.
    \return true if service is busy otherwise false.
****************************************************************************************/
bool mailClientIsBusy(MailDescriptor_t *const mail);

/************************************************************************************//**
    \brief This function called by the mailbox adapter when the adapter checks
        the client's sending queue.
    \param[in] mail - mailbox descriptor.

    \return true if the server's queue is not empty and false otherwise.
****************************************************************************************/
bool mailClientQueuePoll(MailDescriptor_t *const mail);

/************************************************************************************//**
    \brief This function should be invoked by the adapter when it has a parcel
        for the mailbox client.
    \param[in] client - client module descriptor.
    \param[in] fifoHeader - FIFO header pointer.
    \param[in] header - request header pointer.
    \param[in] req - request pointer.
****************************************************************************************/
void mailClientDataInd(MailAdapterDescriptor_t *const adapter,
                       MailFifoHeader_t *const fifoHeader,
                       MailWrappedReqHeader_t *const header, uint8_t *req);

/************************************************************************************//**
    \brief Helper function. Parses the parcel header and allocates memory for a parcel.
    \param[in] client - client module descriptor.
    \param[in] header - request header pointer.
****************************************************************************************/
uint8_t *mailClientGetMemory(MailAdapterDescriptor_t *const adapter,
                             MailFifoHeader_t *fifoHeader,
                             MailWrappedReqHeader_t *const header);

#endif /* _MAILPRIVATE_CLIENT_H */