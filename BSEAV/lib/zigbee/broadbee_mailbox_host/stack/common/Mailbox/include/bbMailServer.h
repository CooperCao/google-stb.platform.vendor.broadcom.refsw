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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/include/bbMailServer.h $
*
* DESCRIPTION: declaration of the mailbox server module.
*
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/

#ifndef _MAIL_SERVER_H
#define _MAIL_SERVER_H

/************************* INCLUDES ****************************************************/
#include "bbMailTypes.h"
#include "bbMailServerTable.h"
#include "bbMailAPI.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief server buffers amount.
 */
#define MAIL_SERVER_MAX_AMOUNT_PROCESSED_REQUESTS 5

/************************* TYPES *******************************************************/
/**//**
 * \brief Type of server's pool's entry of processed requests.
 */
typedef struct _MailServerPoolEntry_t
{
    /* service field of queue's mechanism */
    SYS_QueueElement_t      elem;
    /* entry status */
    bool                    isBusy;
    /* FIFO header */
    MailFifoHeader_t  fifoHeader;
    /* request header */
    MailWrappedReqHeader_t  header;
    /* buffer for request/confirm/indication */
    union
    {
        MailReq_t           req;
        struct
        {
#ifdef _DEBUG_COMPLEX_
            uint8_t         dbgConfirmOffset[16]; /* NOTE: Addition offset between 'req' and 'conf' fields, to detect wrong behavior.
            Mailbox allocate this buffer -> receives some message to this buffer -> call appropriate handler from stack implementation
            -> handler parse receives message -> handler calls callback function (fakeCallback inserted by mailbox)
            -> into fakeCallback function the mailbox overwrite 'req' field by 'conf' field
            -> !!!! but the low level continues use the 'req' field !!!! (for example remove from queues and etc.) -> undefined behavior
            -> maybe assert or unexpected reset (jump to NULL)
            Be careful and do not use 'request' memory after callback rise.*/
#endif
            MailConfParams_t conf;
        };
        MailIndParams_t     ind;
    } parcel;
    /* service parcel */
    union
    {
        MailServiceConfParams_t conf;
    } serviceParcel;
} MailServerBuffer_t;

/**//**
 * \brief Type of the server module descriptor.
 */
typedef struct _MailServerDescriptor_t
{
    /* Descriptor of queue of finished requests */
    SYS_QueueDescriptor_t finishedQueue;
    /* Descriptor of queue of service requests */
    SYS_QueueDescriptor_t serviceQueue;
    /* Pool of server's requests */
    MailServerBuffer_t buffer[MAIL_SERVER_MAX_AMOUNT_PROCESSED_REQUESTS];
} MailServerDescriptor_t;

#endif /* _MAIL_SERVER_H */