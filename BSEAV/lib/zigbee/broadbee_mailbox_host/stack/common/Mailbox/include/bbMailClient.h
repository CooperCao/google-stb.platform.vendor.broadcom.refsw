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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/include/bbMailClient.h $
*
* DESCRIPTION: declaration of the mailbox client module.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/

#ifndef _MAIL_CLIENT_H
#define _MAIL_CLIENT_H

/************************* INCLUDES ****************************************************/
#include "bbMailTypes.h"
#include "bbMailClientTable.h"
#include "bbMailAPI.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief number of pending client requests
 */
#define MAIL_CLIENT_MAX_AMOUNT_PENDING_CALLS 15

/************************* TYPES *******************************************************/
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
    /* The mutex to protect the parcelQueue to support multiple thread */
    pthread_mutex_t         parcelQueueMutex;
    /* The mutex to protect the pending Table to support multiple thread */
    pthread_mutex_t         pendingTableMutex;
    /* Descriptor of queue of service message */
    SYS_QueueDescriptor_t   serviceMessageQueue;
    /* request unique Id counter */
    uint8_t                 uIdCounter;
    /* Table of pending calls */
    MailPendingAPICall_t    pendingTable[MAIL_CLIENT_MAX_AMOUNT_PENDING_CALLS];
    /* Timer for acknowledgment feature */
    SYS_TimeoutSignal_t     ackTimer;
} MailClientDescriptor_t;


#endif /* _MAIL_CLIENT_H */
/* eof bbMailClient.h */