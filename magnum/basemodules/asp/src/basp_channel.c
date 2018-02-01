/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/

#include "basp.h"
#include "basp_priv.h"
#include "bdbg.h"

BDBG_MODULE(BASP_CHANNEL);
BDBG_OBJECT_ID(BASP_P_Channel);           /* BASP_ChannelHandle */
BDBG_OBJECT_ID_DECLARE(BASP_P_Context);

/******************************************************************************/
void BASP_Channel_GetDefaultCreateSettings(
  BASP_ChannelCreateSettings *pSettings /* [out] */
  )
{
    BDBG_ASSERT(NULL != pSettings);

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}


/******************************************************************************/
BERR_Code BASP_Channel_Create(
  BASP_ContextHandle hContext,
  const BASP_ChannelCreateSettings *pSettings,
  BASP_ChannelHandle *pHandle /* [out] */
  )
{
    BERR_Code errCode = BERR_SUCCESS;
    BASP_ChannelCreateSettings defaultSettings;
    BASP_ChannelHandle hChannel;

    BDBG_OBJECT_ASSERT(hContext, BASP_P_Context);
    BDBG_ASSERT(NULL != pHandle);

    *pHandle = NULL;

    if ( NULL == pSettings )
    {
        BASP_Channel_GetDefaultCreateSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    /* No settings to validate. */

    hChannel = BKNI_Malloc(sizeof(BASP_P_Channel));
    BDBG_MSG(("hChannel from malloc=%p", (void *)hChannel));
    if ( NULL == hChannel )
    {
        errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    BKNI_Memset(hChannel, 0, sizeof(*hChannel));
    BDBG_OBJECT_SET(hChannel, BASP_P_Channel);

    BKNI_EnterCriticalSection();
    BLST_S_INSERT_HEAD(&hContext->channelList, hChannel, nextChannel);
    BKNI_LeaveCriticalSection();

    hChannel->sCreateSettings = *pSettings;
    hChannel->hContext = hContext;

    hChannel->channelIndex = pSettings->channelNumber;

    *pHandle = hChannel;

error:
    return errCode;
}


/******************************************************************************/

void BASP_Channel_Destroy(
  BASP_ChannelHandle hChannel
  )
{
    BDBG_OBJECT_ASSERT(hChannel, BASP_P_Channel);

    /* Make sure this channel is not running. */
    /* TODO: */

    BDBG_LOG(("hChannel=%p", (void *)hChannel));

    /* Unlink from context's channel list. */
    BKNI_EnterCriticalSection();
    BLST_S_REMOVE(&hChannel->hContext->channelList, hChannel, BASP_P_Channel, nextChannel);
    BKNI_LeaveCriticalSection();
    BDBG_OBJECT_DESTROY(hChannel, BASP_P_Channel);

    /* Free memory. */
    BDBG_LOG(("hChannel freeing=%p", (void *)hChannel));
    BKNI_Free(hChannel);
}


/******************************************************************************/
BASP_ChannelHandle BASP_P_Channel_GetByChannelIndex_isr(
    BASP_Handle  hAsp,
    uint32_t channelIndex
    )
{
    BASP_ContextHandle hContext;
    BASP_ChannelHandle hChannel;

    for ( hContext = BLST_S_FIRST(&hAsp->contextList) ;
        hContext != NULL ;
        hContext = BLST_S_NEXT(hContext, nextContext) )
    {
        /* If this isn't a streaming context, just skip to the next... */
        if (hContext->sCreateSettings.type != BASP_ContextType_eStreaming) { continue;}

        for (hChannel = BLST_S_FIRST(&hContext->channelList) ;
             hChannel != NULL ;
             hChannel = BLST_S_NEXT(hChannel, nextChannel) )
        {
            if (hChannel->channelIndex == channelIndex)
            {
                return hChannel;
            }
        }
    }
    return NULL;    /* Didn't find the desired channelIndex. */
}


/******************************************************************************/
void BASP_P_Channel_FireMessageReadyCallback_isr(
    BASP_ChannelHandle  hChannel
    )
{
    if ( NULL != hChannel->callbacks.messageReady.pCallback_isr )
    {
       BDBG_MSG(("Executing channel[%p] messageReady callback", (void*) hChannel ));
       hChannel->callbacks.messageReady.pCallback_isr(
                       hChannel->callbacks.messageReady.pParam1,
                       hChannel->callbacks.messageReady.param2
                       );
    }

    return;
}


/******************************************************************************/
void BASP_Channel_GetCallbacks(
  BASP_ChannelHandle hChannel,
  BASP_ChannelCallbacks *pCallbacks /* [out] */
  )
{
    BDBG_ENTER( BASP_Channel_GetCallbacks );

    BDBG_OBJECT_ASSERT(hChannel, BASP_P_Channel);
    BDBG_ASSERT( pCallbacks );

    *pCallbacks = hChannel->callbacks;

    BDBG_LEAVE( BASP_Channel_GetCallbacks );
    return;

}


/******************************************************************************/
BERR_Code BASP_Channel_SetCallbacks(
  BASP_ChannelHandle hChannel,
  const BASP_ChannelCallbacks *pCallbacks
  )
{
    BDBG_ENTER( BASP_Channel_SetCallbacks );

    BDBG_OBJECT_ASSERT(hChannel, BASP_P_Channel);
    BDBG_ASSERT( pCallbacks );

    BKNI_EnterCriticalSection();

    hChannel->callbacks = *pCallbacks;

    BKNI_LeaveCriticalSection();

    BDBG_LEAVE( BASP_Channel_SetCallbacks );
    return BERR_TRACE( BERR_SUCCESS );
}


/******************************************************************************/
BERR_Code BASP_Channel_SendMessage(
  BASP_ChannelHandle hChannel,
  BASP_MessageType messageType,
  BASP_ResponseType responseType,
  BASP_Pi2Fw_Message *pMessage
  )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hChannel, BASP_P_Channel);

    pMessage->MessageHeader.MessageType = messageType;
    pMessage->MessageHeader.ResponseType = responseType;
    pMessage->MessageHeader.ui32ChannelIndex = hChannel->channelIndex;

    rc = BASP_Msgqueue_Write(hChannel->hContext->hAsp->hMsgqueueHostToFw, pMessage);

    /* TODO: */

    return (rc);
}


/******************************************************************************/
BERR_Code BASP_Channel_ReadMessage_isr(
  BASP_ChannelHandle hChannel,
  BASP_MessageType *pType,
  BASP_Fw2Pi_Message *pMessage,
  unsigned *pMessageLength
  )
{
    BERR_Code rc = BERR_SUCCESS;

    BKNI_ASSERT_ISR_CONTEXT();
    BDBG_OBJECT_ASSERT(hChannel, BASP_P_Channel);

    BSTD_UNUSED(pType);
    BSTD_UNUSED(pMessageLength);

    {
        BASP_Fw2Pi_Message *pMsg;
        BASP_MsgqueueHandle hMsgqueue;
        size_t size = 0;

        hMsgqueue = hChannel->hContext->hAsp->hMsgqueueFwToHost;

        /* Try to get the first (oldest) message in the queue. */
        rc = BASP_Msgqueue_GetReadData_isr(hMsgqueue, (void *)&pMsg, &size);
        if (BERR_SUCCESS != rc)
        {
            if (BERR_NOT_AVAILABLE == rc)
            {
                BDBG_MSG(("%s : %d : Msgqueue %p is empty", BSTD_FUNCTION, __LINE__, (void*)hChannel ));
            }
            else
            {
                BERR_TRACE(rc);
            }
            return rc;
        }

        /* We have a message, see if it's for the caller's channel. */
        if (pMsg->MessageHeader.ui32ChannelIndex != hChannel->channelIndex)
        {
            BDBG_MSG(("%s : %d : No message for channel=%u have channel=%u instead", BSTD_FUNCTION, __LINE__, hChannel->channelIndex, pMsg->MessageHeader.ui32ChannelIndex ));
            BDBG_MSG(("%s: pMsg->MessageHeader.MessageType=%u, pMsg->MessageHeader.ui32MessageCounter=%u", BSTD_FUNCTION, pMsg->MessageHeader.MessageType, pMsg->MessageHeader.ui32MessageCounter));

            return BERR_NOT_AVAILABLE;
        }   /* Oldest message isn't for requested channel. */

        /* Give MessageType back to caller. */
        *pType = pMsg->MessageHeader.MessageType;

        /* Copy the message to the caller's buffer. */
        if (size > sizeof (BASP_Fw2Pi_Message)) {size = sizeof (BASP_Fw2Pi_Message);}
        BKNI_Memcpy_isr(pMessage, pMsg, size);

        /* Indicate that message can be removed from queue. */
        rc = BASP_Msgqueue_ReadComplete_isr(hChannel->hContext->hAsp->hMsgqueueFwToHost);
        BERR_TRACE(rc);
    }

    return (rc);

}


BERR_Code BASP_Channel_ReadMessage(
  BASP_ChannelHandle hChannel,
  BASP_MessageType *pType,
  BASP_Fw2Pi_Message *pMessage,
  unsigned *pMessageLength
  )
{
    BERR_Code   errCode;

    BDBG_OBJECT_ASSERT(hChannel, BASP_P_Channel);

    BKNI_EnterCriticalSection();
    errCode = BASP_Channel_ReadMessage_isr(hChannel, pType, pMessage, pMessageLength);
    BKNI_LeaveCriticalSection();

    return errCode;
}
