/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
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

BDBG_MODULE(b_os_message_queue);

BDBG_OBJECT_ID(B_MessageQueue);

typedef struct B_MessageQueue
{
    BDBG_OBJECT(B_MessageQueue)
    B_MessageQueueSettings settings;
    uint8_t *pMemory;
    unsigned readIndex;
    unsigned writeIndex;
    unsigned numAvailable;
    unsigned slotSize;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
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
    int rc;
    size_t messageSize;
    B_Error errCode;
    B_MessageQueueHandle handle;
    pthread_condattr_t attr;

    BDBG_ASSERT(NULL != pSettings);

    if ( pSettings->maxMessageSize <= 0 )
    {
        BDBG_ERR(("Invalid message size %u", (unsigned)pSettings->maxMessageSize));
        errCode = BERR_TRACE(B_ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if ( pSettings->maxMessages <= 0 )
    {
        BDBG_ERR(("Invalid max message value %u", (unsigned)pSettings->maxMessages));
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

    rc = pthread_mutex_init(&handle->mutex, NULL);
    if ( rc )
    {
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        goto err_mutex;
    }

    rc = pthread_condattr_init(&attr);
    if (rc!=0) {
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        goto err_cond;
    }
    
#ifndef B_REFSW_ANDROID
    rc = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (rc!=0) {
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        goto err_cond;
    }
#endif

    rc = pthread_cond_init(&handle->condition, &attr);
    if ( rc )
    {
        errCode = BERR_TRACE(B_ERROR_OS_ERROR);
        goto err_cond;
    }

    /* Save Settings */
    handle->settings = *pSettings;
    messageSize = handle->settings.maxMessageSize;
    /* Add overhead for message size */
    messageSize += sizeof(int);
    /* Round up to integer boundary */
    messageSize = (messageSize + (sizeof(int)-1))&~(sizeof(int)-1);
    handle->slotSize = messageSize;
    handle->pMemory = B_Os_Calloc(messageSize, handle->settings.maxMessages);
    if ( NULL == handle->pMemory )
    {
        errCode = BERR_TRACE(B_ERROR_OUT_OF_MEMORY);
        goto err_memory;
    }

    handle->numAvailable = handle->settings.maxMessages;

    return handle;

err_memory:
err_cond:
err_mutex:
    BKNI_Memset(handle, 0, sizeof(*handle));
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
    B_Os_Free(handle->pMemory);
    BKNI_Memset(handle, 0, sizeof(*handle));
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
    int rc;
    int *pData;

    BDBG_OBJECT_ASSERT(handle, B_MessageQueue);

    if ( messageSize > 0 )
    {
        BDBG_ASSERT(NULL != pMessageData);
    }

    BDBG_ASSERT(messageSize <= handle->settings.maxMessageSize);

    /* Lock Queue */
    rc = pthread_mutex_lock(&handle->mutex);
    if ( rc )
    {
        return BERR_TRACE(B_ERROR_OS_ERROR);
    }

    if ( handle->numAvailable == 0 )
    {
        /* Queue Full */
        pthread_mutex_unlock(&handle->mutex);
        return B_ERROR_TIMEOUT;
    }

    /* Sanity */
    BDBG_ASSERT(handle->writeIndex < handle->settings.maxMessages);

    /* Grab Slot Address */
    pData = (int *)(handle->pMemory+(handle->slotSize*handle->writeIndex));
    /* Write message size */
    pData[0] = messageSize;
    /* Copy Payload */
    if ( messageSize > 0 )
    {
        BKNI_Memcpy(pData+1, pMessageData, messageSize);
    }
    /* Update Write Pointer */
    handle->writeIndex++;
    if ( handle->writeIndex >= handle->settings.maxMessages )
    {
        handle->writeIndex = 0;
    }
    /* Decrement queue space */
    handle->numAvailable--;

    /* Unlock queue */
    pthread_mutex_unlock(&handle->mutex);
    /* Wake Receiver */
    pthread_cond_signal(&handle->condition);

    /* Success */
    return B_ERROR_SUCCESS;
}

static void B_MessageQueue_P_ExtractMessage(
    B_MessageQueueHandle handle,
    void *pMessageBuffer,
    size_t *pMessageSizeReceived
    )
{
    int *pData;
    int messageSize;
    /* All sanity checks have been done before here.  No need to repeat */

    /* Grab Slot Address */
    pData = (int *)(handle->pMemory+(handle->slotSize*handle->readIndex));

    /* Read message size */
    messageSize = pData[0];
    *pMessageSizeReceived = messageSize;
    /* Copy Payload */
    if ( messageSize > 0 )
    {
        BKNI_Memcpy(pMessageBuffer, pData+1, messageSize);
    }
    /* Update Read Pointer */
    handle->readIndex++;
    if ( handle->readIndex >= handle->settings.maxMessages )
    {
        handle->readIndex = 0;
    }
    /* Increment queue space */
    handle->numAvailable++;
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
    B_Error errCode = B_ERROR_SUCCESS;
    struct timespec target;

    BDBG_OBJECT_ASSERT(handle, B_MessageQueue);

    BDBG_ASSERT(NULL != pMessageBuffer);
    BDBG_ASSERT(NULL != pMessageSizeReceived);
    if (messageBufferSize < handle->settings.maxMessageSize) {
        return BERR_TRACE(B_ERROR_INVALID_PARAMETER);
    }

    /* Calculate expiration of timed wait if necessary */
    if ( timeoutMsec!=B_WAIT_NONE && timeoutMsec!=B_WAIT_FOREVER )
    {
        struct timespec now;
        rc = clock_gettime(CLOCK_MONOTONIC, &now);
        if ( rc )
        {
            return BERR_TRACE(B_ERROR_OS_ERROR);
        }
        target.tv_nsec = now.tv_nsec + (timeoutMsec%1000)*1000000;
        target.tv_sec = now.tv_sec + (timeoutMsec/1000);
        if (target.tv_nsec > 1000000000)
        {
            target.tv_nsec -=  1000000000;
            target.tv_sec ++;
        }
    }

    /* Lock Queue */
    rc = pthread_mutex_lock(&handle->mutex);
    if ( rc )
    {
        return BERR_TRACE(B_ERROR_OS_ERROR);
    }

    /* Sanity Checks */
    BDBG_ASSERT(handle->readIndex < handle->settings.maxMessages);
    BDBG_ASSERT(handle->numAvailable <= handle->settings.maxMessages);

    /* Handle waiting */
    if ( timeoutMsec != B_WAIT_NONE )
    {
        /* Loop until timeout or message arrives */
        while ( handle->numAvailable == handle->settings.maxMessages )  /* While queue is empty... */
        {
            if ( timeoutMsec == B_WAIT_FOREVER )
            {
                rc = pthread_cond_wait(&handle->condition, &handle->mutex);
            }
            else
            {
                rc = pthread_cond_timedwait(&handle->condition, &handle->mutex, &target);
                if ( rc == ETIMEDOUT )
                {
                    BDBG_MSG(("B_MessageQueue_Receive(%p): timeout", (void*)handle));
                    errCode = B_ERROR_TIMEOUT;
                    break;
                }
            }

            if ( rc )
            {
                errCode = BERR_TRACE(B_ERROR_OS_ERROR);
                break;
            }
        }
    }

    /* If no error occurred, retrieve first message if available */
    if ( errCode == B_ERROR_SUCCESS )
    {
        if ( handle->numAvailable < handle->settings.maxMessages )
        {
            B_MessageQueue_P_ExtractMessage(handle, pMessageBuffer, pMessageSizeReceived);
            errCode = B_ERROR_SUCCESS;
        }
        else
        {
            /* Queue empty */
            errCode = B_ERROR_TIMEOUT;
        }
    }

    /* Sanity check again */
    BDBG_ASSERT(handle->readIndex < handle->settings.maxMessages);
    BDBG_ASSERT(handle->numAvailable <= handle->settings.maxMessages);

    /* Unlock queue */
    pthread_mutex_unlock(&handle->mutex);
    return errCode;
}

