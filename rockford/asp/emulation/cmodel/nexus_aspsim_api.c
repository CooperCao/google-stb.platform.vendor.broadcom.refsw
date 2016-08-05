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
#include "nexus_aspsim_api.h"

typedef struct NEXUS_AspSim_Channel
{
    BASP_ChannelHandle  hAspChannel;

    /* TODO:later add a channel list here like in basp_channel*/
} NEXUS_AspSim_Channel;

/**
Summary:
Initialize nexus AspSim module.
**/
void NEXUS_AspSim_Init(void)
{
    /* Initialize asp module.*/
    BASP_Init();
}

/**
Summary:
Uninitialize nexus AspSim module.
**/
void NEXUS_AspSim_UnInit(void)
{
    /* Initialize asp module.*/
    BASP_UnInit();
}

/**
Summary:
Get default nexus asp channel open settings.
**/
void NEXUS_AspSim_Channel_GetDefaultOpenSettings(
    NEXUS_AspSim_ChannelOpenSettings *pNexusAspChannelOpenSettings
    )
{
    memset( pNexusAspChannelOpenSettings, 0, sizeof(NEXUS_AspSim_ChannelOpenSettings));
}

/**
Summary:
Open Nexus AspSim Context handle.
**/
NEXUS_AspSim_ChannelHandle NEXUS_AspSim_Channel_Open(
    NEXUS_AspSim_ChannelOpenSettings *pNexusAspChannelOpenSettings
    )
{
    NEXUS_AspSim_ChannelHandle hNexusAspChannel = NULL;
    BASP_ChannelOpenSettings channelOpenSettings;

    hNexusAspChannel = calloc(1,sizeof(NEXUS_AspSim_Channel));

    BASP_Channel_GetDefaultOpenSettings(&channelOpenSettings);

    /* Modify the required changes in channelOpenSettings*/

    hNexusAspChannel->hAspChannel = BASP_Channel_Open(&channelOpenSettings);
    if( hNexusAspChannel->hAspChannel == NULL )
    {
        printf("%s: Failed to open Nexus asp channel\n", __FUNCTION__);
        goto error;
    }
    return hNexusAspChannel;

error:
    if(hNexusAspChannel)
    {
        free(hNexusAspChannel);
    }
    return NULL;
}

/**
Summary:
Close nexus AspSim Context handle.
**/
void NEXUS_AspSim_Channel_Close(
    NEXUS_AspSim_ChannelHandle hNexusAspSimChannel
    )
{
    BASP_Channel_Close( hNexusAspSimChannel->hAspChannel );
    free( hNexusAspSimChannel );
}

/**
Summary:
Get default nexus asp channel start settings.
**/
void NEXUS_AspSim_Channel_GetDefaultStartSettings(
    NEXUS_AspSim_ChannelStartSettings *pNexusChannelStartSettings
    )
{
    memset( pNexusChannelStartSettings, 0, sizeof(NEXUS_AspSim_ChannelStartSettings));
}

/**
Summary:
Start nexus AspSim Channel Context.
**/
int NEXUS_AspSim_Channel_Start(
    NEXUS_AspSim_ChannelHandle hNexusAspSimChannel,
    const NEXUS_AspSim_ChannelStartSettings *pNexusAspSimChannelStartSettings
    )
{
    int rc = 0;
    BASP_ChannelStartSettings channelStartSettings;

    BASP_Channel_GetDefaultStartSettings( &channelStartSettings );

    /* modify the settings */
    channelStartSettings.sSocketState = pNexusAspSimChannelStartSettings->socketState;
    channelStartSettings.pXptMcpbBuffer = pNexusAspSimChannelStartSettings->pXptMcpbBuffer;

    rc = BASP_Channel_Start( hNexusAspSimChannel->hAspChannel, &channelStartSettings );
    if(rc == -1)
    {
        printf("%s: BASP_Channel_Start failed.\n");
    }
    return rc;
}

/**
Summary:
Stop nexus AspSim Channel Context.
**/
int NEXUS_AspSim_Channel_Stop(
    NEXUS_AspSim_ChannelHandle hNexusAspSimChannel
    )
{
    int rc = 0;

    rc = BASP_Channel_Stop( hNexusAspSimChannel->hAspChannel );
    assert(rc==0);

error:
    return rc;
}

/**
Summary:
Nexus AspSim Process Io.
In actual case this won't be there since it will be call back
driven from Pi.
**/
void Nexus_AspSim_ProcessIo(NEXUS_AspSim_ChannelHandle hNexusAspChannel)
{
    int rc = 0;

    rc = BASP_Channel_ProcessIo(hNexusAspChannel->hAspChannel);
    assert(rc == 0);

}

/**
Summary:
This api call will check whether a particular nexus asp channel
is stopped or not.
Return:
    1   :If asp chhanel has stopped and a stop response message
         recived from asp-cmodel/simulator.
    0   :Channel is not yet stopped.
**/
int NEXUS_AspSim_Channel_IsStopped(
    NEXUS_AspSim_ChannelHandle hNexusAspChannel
    )
{
    return BASP_Channel_IsStopped( hNexusAspChannel->hAspChannel );
}

/**
Summary:
This api call will check whether a particular nexus asp channel
started or not.
Return:
    1    : If the Asp channel has started.
    0    : If the channel is not yet started (may be because the
           channel start is in progress and the fw is yet to
           send back the start response).
**/
int NEXUS_AspSim_Channel_IsStarted(
    NEXUS_AspSim_ChannelHandle hNexusAspChannel
    )
{
    return BASP_Channel_IsStarted( hNexusAspChannel->hAspChannel );
}

/**
Summary:
Provide latest socketState to the caller. This is only useful
once the stop response received from asp-cmodel/simulator .
**/

int NEXUS_AspSim_Channel_GetLatestSocketState(
    NEXUS_AspSim_ChannelHandle hNexusAspChannel,
    SocketState                *pSocketState     /*!< Pass address of the socketState structure which will be populated with latest socket state.*/
    )
{
    int rc;

    rc = BASP_Channel_GetLatestSocketState( hNexusAspChannel->hAspChannel, pSocketState );

    return rc;
}
