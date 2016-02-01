/***************************************************************************
*     (c)2004-2008 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to the terms and
*  conditions of a separate, written license agreement executed between you and Broadcom
*  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
*  no license (express or implied), right to use, or waiver of any kind with respect to the
*  Software, and Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
*  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
*  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  
*   
*  Except as expressly set forth in the Authorized License,
*   
*  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
*  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
*  and to use this information only in connection with your use of Broadcom integrated circuit products.
*   
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
*  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
*  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO 
*  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES 
*  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, 
*  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION 
*  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF 
*  USE OR PERFORMANCE OF THE SOFTWARE.
*  
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS 
*  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR 
*  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR 
*  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF 
*  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT 
*  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE 
*  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF 
*  ANY LIMITED REMEDY.
* 
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* Description:
*   API name: OS
*    Library routines for OS abstraction
*
* Revision History:
*
* $brcm_Log: $
* 
***************************************************************************/
#include "b_os_lib.h"
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include "bkni.h"

#include <msgQLib.h>

BDBG_MODULE(b_os_message_queue);

BDBG_OBJECT_ID(B_MessageQueue);

typedef struct B_MessageQueue
{
    BDBG_OBJECT(B_MessageQueue)
    B_MessageQueueSettings settings;
    MSG_Q_ID queue; /* the VxWorks message queue */
} B_MessageQueue;

/***************************************************************************
Summary:
Get Default Message Queue Settings
***************************************************************************/
void B_MessageQueue_GetDefaultSettings(
    B_MessageQueueSettings *pSettings   /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/***************************************************************************
Summary:
Create a message queue
***************************************************************************/
B_MessageQueueHandle B_MessageQueue_Create(
    const B_MessageQueueSettings *pSettings
    )
{
    B_Error errCode;
    B_MessageQueueHandle handle;

    BDBG_ASSERT(NULL != pSettings);

    if ( pSettings->maxMessageSize <= 0 )
    {
        BDBG_ERR(("Invalid message size %u", pSettings->maxMessageSize));
        errCode = BERR_TRACE(B_ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if ( pSettings->maxMessages <= 0 )
    {
        BDBG_ERR(("Invalid max message value %u", pSettings->maxMessages));
        errCode = BERR_TRACE(B_ERROR_INVALID_PARAMETER);
        return NULL;
    }

    handle = B_Os_Calloc(1, sizeof(B_MessageQueue));
    if ( NULL == handle )
    {
        errCode = BERR_TRACE(B_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

    BDBG_OBJECT_SET(handle, B_MessageQueue);

    handle->queue = msgQCreate(pSettings->maxMessages, pSettings->maxMessageSize, MSG_Q_FIFO);
    if ( !handle->queue )
    {
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        goto err_cond;
    }

    /* Save Settings */
    handle->settings = *pSettings;

    return handle;

err_cond:
    B_Os_Free(handle);
    return NULL;
}

/***************************************************************************
Summary:
Destroy a message queue
***************************************************************************/
void B_MessageQueue_Destroy(
    B_MessageQueueHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, B_MessageQueue);
    msgQDelete(handle->queue);
    B_Os_Free(handle);
}

/***************************************************************************
Summary:
Post (Send) a message to a queue

Description:
This function will return B_ERROR_SUCCESS if the message was sent 
successfully or B_ERROR_TIMEOUT if the message queue is full.
***************************************************************************/
B_Error B_MessageQueue_Post(
    B_MessageQueueHandle handle,
    const void *pMessageData,
    size_t messageSize
    )
{
    BDBG_OBJECT_ASSERT(handle, B_MessageQueue);

    if ( messageSize > 0 )
    {
        BDBG_ASSERT(NULL != pMessageData);
    }

    BDBG_ASSERT(messageSize <= handle->settings.maxMessageSize);

    msgQSend(handle->queue, (char*)pMessageData, (UINT)messageSize, NO_WAIT, MSG_PRI_NORMAL);

    /* What should I return if the send fails?? */
    return B_ERROR_SUCCESS;
}

/***************************************************************************
Summary:
Receive a message from a queue

Description:
This function will return B_ERROR_SUCCESS if a message was successfully
received or B_ERROR_TIMEOUT if the specified timeout value was reached.
***************************************************************************/
B_Error B_MessageQueue_Wait(
    B_MessageQueueHandle handle,
    void *pMessageBuffer,
    size_t messageBufferSize,
    size_t *pMessageSizeReceived,
    int timeoutMsec                 /* Values B_WAIT_NONE and B_WAIT_FOREVER may be used */
    )
{
    int rc;

    BDBG_OBJECT_ASSERT(handle, B_MessageQueue);

    BDBG_ASSERT(NULL != pMessageBuffer);
    BDBG_ASSERT(messageBufferSize >= handle->settings.maxMessageSize);
    BDBG_ASSERT(NULL != pMessageSizeReceived);

    rc = msgQReceive(handle->queue, (char*)pMessageBuffer, (UINT)messageBufferSize, 
        timeoutMsec==B_WAIT_NONE? NO_WAIT : timeoutMsec==B_WAIT_FOREVER? WAIT_FOREVER : timeoutMsec);

    if (rc == ERROR) {
        if (timeoutMsec == B_WAIT_FOREVER) 
            return B_ERROR_UNKNOWN;
        return B_ERROR_TIMEOUT;
    }

    *pMessageSizeReceived = rc;
    return B_ERROR_SUCCESS;
}

