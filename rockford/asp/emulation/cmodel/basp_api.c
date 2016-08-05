/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basp_api.h"
#include "basp_aspsim_api.h"


typedef struct BASP_Context
{
    /* Ad a fifo buffer which will hold all asp fw to pi messages.*/
    int numChannelSupported;
    int *pChannelMatrix;        /*!< points to an array which tells us whether an asp channel is available or not.*/

    BASP_ChannelHandle hFirstAspChannelInList;
} BASP_Context;

BASP_Context    gAspContext;
BASP_P_MessageFifo  gAspMessageFifo;

typedef enum BASP_ChannelState
{
    BASP_ChannelState_eOpened,
    BASP_ChannelState_eStartInProgress,
    BASP_ChannelState_eStarted,
    BASP_ChannelState_eStopInProgress,
    BASP_ChannelState_eStopped,
    BASP_ChannelState_eOther,
    BASP_ChannelState_eMax,
} BASP_ChannelState;


typedef struct BASP_Channel
{
    int                         aspChannelNumber;   /*!< Asp channel number based on which channel is unused at the time of asp start.
                                                    /*!< It start from 0 to number of available channel*/
    BASP_ChannelState           channelState;       /*!< channel state. */
    BASP_ChannelOpenSettings    channelOpenSettings;

    SocketState                 sLatestSocketState;  /*!< This will maintain the latest socket state from stop response message.*/
    BASP_ChannelHandle          hNextAspChannelInList;
} BASP_Channel;

typedef enum BASP_ChannelStatus
{
    BASP_ChannelStatus_eAvailable,
    BASP_ChannelStatus_eNotAvailable
} BASP_ChannelStatus;


#define BASP_CHANNELLIST_INSERT(listHeadAddress, listEntry)             \
do                                                                      \
{                                                                       \
    BASP_ChannelHandle *phCurrentNode = NULL;                           \
    phCurrentNode = (listHeadAddress);                                  \
    while(*phCurrentNode != NULL)                                       \
    {                                                                   \
        phCurrentNode = &((*phCurrentNode)->hNextAspChannelInList);     \
    }                                                                   \
    *phCurrentNode = listEntry;                                         \
}while (0)

#define BASP_CHANNELLIST_REMOVE(listHeadAddress, listRemove)            \
do                                                                      \
{                                                                       \
    BASP_ChannelHandle *phCurrentNode = NULL;                           \
    BASP_ChannelHandle hTempNode = NULL;                                \
    phCurrentNode = (listHeadAddress);                                  \
    while(*phCurrentNode != NULL)                                       \
    {                                                                   \
        if( (*phCurrentNode) == listRemove )                            \
        {                                                               \
            hTempNode = (*phCurrentNode);                               \
            *phCurrentNode = (*phCurrentNode)->hNextAspChannelInList;   \
            hTempNode->hNextAspChannelInList = NULL ;/* delinking this node even from pointing to next node.*/                   \
            break;                                                      \
        }                                                               \
        else                                                            \
        {                                                               \
            phCurrentNode = &((*phCurrentNode)->hNextAspChannelInList); \
        }                                                               \
    }                                                                   \
}while (0)


static  BASP_ChannelHandle BASP_GetAspChannelFromListForSpecifiedChannelNumber(
    int channelNumber
    )
{
    BASP_ChannelHandle hChannel = NULL;
    hChannel = gAspContext.hFirstAspChannelInList;
    while(hChannel != NULL) {
        if(hChannel->aspChannelNumber == channelNumber)
        {
            break;
        }
        hChannel = hChannel->hNextAspChannelInList;
    }
    return hChannel;
}

/**
Summary:
Initialize Asp module.All onetime HW and memory
initialization of asp module will happen through this api.
Return:
    -1    : If fails
**/
int BASP_Init( void )
{
    int rc = 0;

    /* Allocagte all global memory and resources required to enable asp module.*/

    /* Populate the init message with appropriate values and send init message thru fifo.*/

    gAspContext.hFirstAspChannelInList = NULL;
    gAspContext.numChannelSupported = BASP_MAX_NUMBER_OF_SUPPORTED_CHANNEL;
    gAspContext.pChannelMatrix = calloc(BASP_MAX_NUMBER_OF_SUPPORTED_CHANNEL,sizeof(int));
    /* whenever a asp channel will be opened the corresponding channelMatrix entry will be set to 1.*/

    /* Initialize the temp message buffer.*/
    memset( &gAspMessageFifo , 0, sizeof(BASP_P_MessageFifo));
    gAspMessageFifo.size = BASP_P_FIFO_SIZE;

    return rc;
}

/**
Summary:
Uninitialize Asp module.
**/
void BASP_UnInit( void )
{
    /* if required,send an unInit message to asp fw.*/
    /* Allocagte all global memory and resources required to enable asp module.*/

    gAspContext.hFirstAspChannelInList = NULL;
    memset( &gAspMessageFifo , 0, sizeof(BASP_P_MessageFifo));
}

/**
Summary:
Get default asp channel settings.
**/
void BASP_Channel_GetDefaultOpenSettings(
    BASP_ChannelOpenSettings *pChannelOpenSettings
    )
{
    memset(pChannelOpenSettings, 0, sizeof(BASP_ChannelOpenSettings));
}

static int BASP_GetAspChannelNumber(
    BASP_Context    *pAspContext
    )
{
    int i=0;

    for( i=0; i<pAspContext->numChannelSupported; i++ )
    {
        /* check which is the available channel grab it and mark it as unavailable.*/
        if( BASP_ChannelStatus_eAvailable == pAspContext->pChannelMatrix[i] )
        {
            pAspContext->pChannelMatrix[i] = BASP_ChannelStatus_eNotAvailable;
            break;
        }
    }

    if(i < pAspContext->numChannelSupported)
    {
        return i;
    }
    else
    {
        /* all channels are occupied.*/
        return -1;
    }
}


/**
Summary:
Allocate memory and initialize the BASP_Channel object.
**/
BASP_ChannelHandle BASP_Channel_Open(
    BASP_ChannelOpenSettings *pChannelOpenSettings
    )
{
    BASP_ChannelHandle hChannel = NULL;

    hChannel = calloc(1,sizeof(BASP_Channel));

    hChannel->channelOpenSettings = *pChannelOpenSettings;

    hChannel->aspChannelNumber = BASP_GetAspChannelNumber(&gAspContext);

    if( hChannel->aspChannelNumber == -1 )
    {
        printf("%s: Failed to open channel \n", __FUNCTION__);
        goto error;
    }
    hChannel->channelState = BASP_ChannelState_eOpened;
    /* add this channel to the channel list.bip_mediaIbip_media_info*/
    BASP_CHANNELLIST_INSERT( &gAspContext.hFirstAspChannelInList, hChannel );

    return hChannel;

error:
    if(hChannel)
    {
        free(hChannel);
    }
    return NULL;
}

static void BASP_ReleaseAspChannelNumber(
     BASP_Context   *pAspContext,
     int            aspChannelNumber
     )
{
    pAspContext->pChannelMatrix[aspChannelNumber] = BASP_ChannelStatus_eAvailable;
}

/**
Summary:
Destroy the BASP_Channel object.
**/
void BASP_Channel_Close(
    BASP_ChannelHandle hChannel
    )
{
    BASP_ReleaseAspChannelNumber(&gAspContext, hChannel->aspChannelNumber);

    BASP_CHANNELLIST_REMOVE( &gAspContext.hFirstAspChannelInList, hChannel);

    free(hChannel);
}

/**
Summary:
Get default asp channel start settings.
**/
void BASP_Channel_GetDefaultStartSettings(
    BASP_ChannelStartSettings *pChannelStartSettings
    )
{
    memset(pChannelStartSettings, 0, sizeof(BASP_ChannelStartSettings));
}

static void BASP_SendMessage2Fw(
    BASP_P_MessageHeader *pMsgHeader,
    void *pMessagePayload,
    int payLoadSize,
    BASP_P_MessageFifo  *pMsgFifo
    )
{
    /* write start message to the Pi2Fw fifo.*/
    int msgHeaderSize = sizeof(BASP_P_MessageHeader);
    memcpy(pMsgFifo->pi2FwMessage, pMsgHeader, msgHeaderSize);

    memcpy((pMsgFifo->pi2FwMessage+msgHeaderSize), pMessagePayload, payLoadSize);
    pMsgFifo->pi2FwMessageAvailable = true;
}

/**
Summary:
Start asp channel.This will populate the fw start message and
write to Fw message queue and wait for the ack from fw to
consume this message.
Return:
    -1    : If fails
**/
int BASP_Channel_Start(
    BASP_ChannelHandle hAspChannel,
    const BASP_ChannelStartSettings *pChannelStartSettings
    )
{
    int                         rc =0;
    BASP_P_MessageHeader        sMessageHdr;
    BASP_P_ChannelStartMessage  sStartMessage;

    memset(&sMessageHdr, 0, sizeof(BASP_P_MessageHeader));
    memset(&sStartMessage, 0, sizeof(BASP_P_ChannelStartMessage));

    /* populate the simulated start message.*/
    sMessageHdr.channelIndex = hAspChannel->aspChannelNumber;
    if(pChannelStartSettings->channelStartType == BASP_ChannelStartType_eStreamOut)
    {
        sMessageHdr.messageType = BASP_P_MessageType_PI2FW_eChannelStartStreamOut;
    }
    else
    {
        sMessageHdr.messageType = BASP_P_MessageType_PI2FW_eChannelStartStreamIn;
    }

    printf("\n%s: asp channelIndex = %d, sMessageHdr.messageType=%d\n", __FUNCTION__, sMessageHdr.channelIndex, sMessageHdr.messageType);

    sStartMessage.sSocketState =  pChannelStartSettings->sSocketState;
    sStartMessage.pXptMcpbBuffer = pChannelStartSettings->pXptMcpbBuffer;
    /* for time being here we will copy the data to another global buffer(may not be exact fifo) from which ixia dut will consume the data.
       I assume that this receives the ack immidiately.*/
    BASP_SendMessage2Fw(&sMessageHdr, &sStartMessage, sizeof(BASP_P_ChannelStartMessage), &gAspMessageFifo);

    hAspChannel->channelState = BASP_ChannelState_eStartInProgress;
    return rc;
}

/**
Summary:
Stop asp channel.This will populate the fw stop message and
write to Fw message queue and wait for the ack from fw to
consume this message.
Return:
    -1    : If fails
**/
int BASP_Channel_Stop(
    BASP_ChannelHandle hAspChannel
    )
{
    int                         rc =0;
    BASP_P_MessageHeader        sMessageHdr;
    BASP_P_ChannelStopMessage   sStopMsg;

    /* populated the stop message. */
    sMessageHdr.channelIndex = hAspChannel->aspChannelNumber;
    sMessageHdr.messageType = BASP_P_MessageType_PI2FW_eChannelStop;

    /* send simulated stop message.I assume that this receives the ack immidiately.*/
    BASP_SendMessage2Fw(&sMessageHdr, &sStopMsg, sizeof(BASP_P_ChannelStopMessage), &gAspMessageFifo);

    hAspChannel->channelState = BASP_ChannelState_eStopInProgress;

    return 0;
}

static int processFw2PiMessage(
    BASP_P_MessageFifo *pMessageFifo
    )
{
    int rc = 0;
    BASP_P_MessageHeader        sMessageHdr;
    BASP_P_ChannelStartResponse sSatrtResponse;
    BASP_P_ChannelStopResponse  sStopResponse;
    BASP_ChannelHandle hAspChannel = NULL;
    int headerSize = sizeof(BASP_P_MessageHeader);

    memcpy(&sMessageHdr,pMessageFifo->fw2PiMessage, headerSize);

    fprintf(stdout,"\n%s: Got sMessageHdr.messageType=%d ========================\n", __FUNCTION__, sMessageHdr.messageType);
    switch(sMessageHdr.messageType)
    {
    case BASP_P_MessageType_FW2PI_eChannelStartStreamOutResponse:
    case BASP_P_MessageType_FW2PI_eChannelStartStreamInResponse:
        memcpy(&sSatrtResponse, (pMessageFifo->fw2PiMessage + headerSize), sizeof(BASP_P_ChannelStartResponse) );
        pMessageFifo->fw2PiMessageAvailable = false;
        if(sSatrtResponse.success == 1)
        {
            /* Find out the channel from the list. */
            hAspChannel = BASP_GetAspChannelFromListForSpecifiedChannelNumber(sMessageHdr.channelIndex);
            if(hAspChannel != NULL)
            {
                hAspChannel->channelState = BASP_ChannelState_eStarted;
            }
            else
            {
                fprintf(stdout,"\n%s: Received startResponse for a aspChannel with channelIndex=%d that doesn't exist in the list.\n", __FUNCTION__, sMessageHdr.channelIndex);
                rc = -1;
            }
        }
        break;

    case BASP_P_MessageType_FW2PI_eChannelStopResponse:
        memcpy(&sStopResponse, (pMessageFifo->fw2PiMessage + headerSize), sizeof(BASP_P_ChannelStopResponse) );
        pMessageFifo->fw2PiMessageAvailable = false;
        if(sStopResponse.success == 1)
        {
            /* Find out the channel from the list. */
            hAspChannel = BASP_GetAspChannelFromListForSpecifiedChannelNumber(sMessageHdr.channelIndex);
            if(hAspChannel != NULL)
            {
                hAspChannel->channelState = BASP_ChannelState_eStopped;
                hAspChannel->sLatestSocketState = sStopResponse.sSocketState;
            }
            else
            {
                fprintf(stdout,"\n%s: Received stopResponse for a aspChannel with channelIndex=%d that doesn't exist in the list.\n", __FUNCTION__, sMessageHdr.channelIndex);
                rc = -1;
            }
        }

        break;
    }
    return rc;
}

/**
Summary:
This api call will invoke the pi code which will process
incoming Fw2Pi messages from Fw2Pi fifo and update the latest
aspChannel state.
Return:
    -1    : If fails
**/
int BASP_Channel_ProcessIo( void )
{
    int rc = 0;

    /* Check if there is any Fw2Pi Response handle them and update the asp channel state.*/
    if( gAspMessageFifo.fw2PiMessageAvailable )
    {
        rc = processFw2PiMessage( &gAspMessageFifo );
    }

    return rc;
}

/**
Summary:
This api call will check whether a particular asp channel
started or not.
Return:
    1    : If the Asp channel has started.
    0    : If the channel is not yet started (may be because the
           channel start is in progress and the fw is yet to
           send back the start response).
**/
int BASP_Channel_IsStarted(
    BASP_ChannelHandle hAspChannel
    )
{
   if(hAspChannel->channelState == BASP_ChannelState_eStarted)
   {
       return 1;
   }
   else
   {
       return 0;
   }
}

/**
Summary:
This api call will check whether a particular asp channel
stopped or not.
Return:
    1    : If the Asp channel has started.
    0    : If the channel is not yet stopped.
**/
int BASP_Channel_IsStopped(
    BASP_ChannelHandle hAspChannel
    )
{
   if(hAspChannel->channelState == BASP_ChannelState_eStopped)
   {
       return 1;
   }
   else
   {
       return 0;
   }
}

/**
Summary:
Provide latest socketState to the caller. This is only useful
once the stop response received from asp-cmodel/simulator .
**/

int BASP_Channel_GetLatestSocketState(
    BASP_ChannelHandle hAspChannel,
    SocketState        *pSocketState     /*!< Pass address of the socketState structure which will be populated with latest socket state.*/
    )
{
    int rc;

    if(hAspChannel->channelState == BASP_ChannelState_eStopped)
    {
        *pSocketState = hAspChannel->sLatestSocketState;
        rc = 0;
    }
    else
    {
        printf("\n %s: Trying to get latestSocketState for channelNumber = %d ,though the corresponding asp channel is not yet stopped.", __FUNCTION__, hAspChannel->aspChannelNumber);
        rc = -1;
    }

    return rc;
}
