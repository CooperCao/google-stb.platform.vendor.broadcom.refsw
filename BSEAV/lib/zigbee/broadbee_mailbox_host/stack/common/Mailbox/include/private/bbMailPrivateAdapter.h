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
* FILENAME: $Workfile: trunk/stack/common/Mailbox/include/private/bbMailPrivateAdapter.h $
*
* DESCRIPTION: private declaration of the mailbox abstract module.
*
* $Revision: 3612 $
* $Date: 2014-09-17 09:29:25Z $
*
****************************************************************************************/
#ifndef _MAILPRIVATE_ADAPTER_H
#define _MAILPRIVATE_ADAPTER_H

/************************* INCLUDES ****************************************************/
#include "bbMailAdapter.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Internal adapter data types.
 */
typedef enum _MailAdapterDataType_t
{
    METADATA,
    NOT_DATA_FLOW,
    DATA_FLOW,
} MailAdapterDataType_t;

typedef enum _MailAdapterHandlers_t
{
    READY_TO_SEND_HANDLER_ID,
    RECEIVE_HANDLER_ID,
} MailAdapterHandlers_t;

/************************* PROTOTYPES **************************************************/
/************************************************************************************//**
    \brief Initialize internal data structures.
    \param[in] adapter - adapter descriptor.
 ****************************************************************************************/
void mailAdapterInit(MailAdapterDescriptor_t *const adapter);

/************************************************************************************//**
    \brief Stops transactions.
    \param[in] adapter - adapter descriptor.
 ****************************************************************************************/
void mailAdapterCancelTx(MailAdapterDescriptor_t *const adapter, uint8_t *const parcel);

/************************************************************************************//**
    \brief Sends parcel to FIFO buffer.
    \param[in] adapter - adapter descriptor.
    \param[in] fifoHeader - FIFO header pointer.
    \param[in] header - pointer to request header.
    \param[in] req - request pointer.

    \return true if the parcel is completely sent.
****************************************************************************************/
bool mailAdapterSend(MailAdapterDescriptor_t *const adapter,
                     MailFifoHeader_t *fifoHeader, MailWrappedReqHeader_t *header, uint8_t *req);

#endif /* _MAILPRIVATE_ADAPTER_H */
/* eof bbMailPrivateAdapter.h */