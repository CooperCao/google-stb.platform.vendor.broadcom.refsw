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
#ifndef __BASP_API_H__
#define __BASP_API_H__

#include <stdint.h>
#include "asp_proxy_server_api.h" /* TODO: Later this will be removed now it is added to access SocketState definition.*/

typedef struct  BASP_Channel* BASP_ChannelHandle;

typedef enum BASP_ChannelStartType
{
    BASP_ChannelStartType_eStreamOut,
    BASP_ChannelStartType_eStreamIn,
    BASP_ChannelStartType_eOther,
    BASP_ChannelStartType_eMax,
}BASP_ChannelStartType;

typedef struct BASP_ChannelOpenSettings
{
    uint32_t unused;
} BASP_ChannelOpenSettings;

typedef struct BASP_ChannelStartSettings
{
    BASP_ChannelStartType       channelStartType;
    SocketState                 sSocketState;
    char                        *pXptMcpbBuffer; /*!< Later this will be replaced with raveBufferInfo as defined in actual StreamOutStart message.*/
} BASP_ChannelStartSettings;

/**
Summary:
Initialize Asp module.All onetime HW and memory
initialization of asp module will happen through this api.
Return:
    -1    : If fails
**/
int BASP_Init();

/**
Summary:
Uninitialize Asp module.
**/
void BASP_UnInit();

/**
Summary:
Get default asp channel open settings.
**/
void BASP_Channel_GetDefaultOpenSettings(
    BASP_ChannelOpenSettings *pChannelOpenSettings
    );

/**
Summary:
Allocate memory and initialize the BASP_Channel object.
**/
BASP_ChannelHandle BASP_Channel_Open(
    BASP_ChannelOpenSettings *pChannelOpenSettings
    );

/**
Summary:
Destroy the BASP_Channel object.
**/
void BASP_Channel_Close(
    BASP_ChannelHandle hChannel
    );

/**
Summary:
Get default asp channel start settings.
**/
void BASP_Channel_GetDefaultStartSettings(
    BASP_ChannelStartSettings *pChannelStartSettings
    );

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
    );

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
    );


/**
Summary:
This api call will invoke the pi code which will process
incoming Fw2Pi messages from Fw2Pi fifo and update the latest
aspChannel state.
Return:
    -1    : If fails
**/
int BASP_Channel_ProcessIo();

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
    );

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
    );

/**
Summary:
Provide latest socketState to the caller. This is only useful
once the stop response received from asp-cmodel/simulator .
**/
int BASP_Channel_GetLatestSocketState(
    BASP_ChannelHandle hAspChannel,
    SocketState        *pSocketState     /*!< Pass address of the socketState structure which will be populated with latest socket state.*/
    );

#if 0
/**
Summary:
Abort asp channel.This will populate the fw abort message and
write to Fw message queue and wait for the ack from fw to
consume this message.
Return:
    -1    : If fails
**/
int BASP_Channel_Abort( BASP_ChannelHandle hAspChannel );

/**
Summary:
Callback Isr function which will be called when asp_pi has got
some response messages from asp_fw. It will update the
hAspChannel context state based on the message.
For example incase of start response message this will change
the AspChannel state from StartInProgress to started.
**/
void BASP_Channel_CallBack_Isr(BASP_ChannelHandle hAspChannel);
#endif

#endif /* __BASP_API_H__ */
