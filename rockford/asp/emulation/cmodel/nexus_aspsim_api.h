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
#ifndef __NEXUS_ASPSIM_API_H__
#define __NEXUS_ASPSIM_API_H__

#include "basp_api.h"
#include "nexus_aspsim.h"

typedef struct NEXUS_AspSim_Channel* NEXUS_AspSim_ChannelHandle;

typedef struct NEXUS_AspSim_ChannelOpenSettings
{
    int unused;
} NEXUS_AspSim_ChannelOpenSettings;

typedef struct NEXUS_AspSim_ChannelStartSettings
{
    NEXUS_AspSim_ConnectionControlInfo connectionControlInfo;
    NEXUS_AspSim_SwitchConfig          switchConfigInfo;
    char                               *pXptMcpbBuffer;         /*!< Later this will be replaced with raveBufferInfo as defined in actual StreamOutStart message.*/
    SocketState socketState; /*TODO: this will be removed */
} NEXUS_AspSim_ChannelStartSettings;

/**
Summary:
Initialize nexus AspSim module.
**/
void NEXUS_AspSim_Init(void);

/**
Summary:
Uninitialize nexus AspSim module.
**/
void NEXUS_AspSim_UnInit(void);

/**
Summary:
Get default nexus asp channel open settings.
**/
void NEXUS_AspSim_Channel_GetDefaultOpenSettings(
    NEXUS_AspSim_ChannelOpenSettings *pNexusAspChannelOpenSettings
    );

/**
Summary:
Open nexus AspSim Context handle.
**/
NEXUS_AspSim_ChannelHandle NEXUS_AspSim_Channel_Open(
    NEXUS_AspSim_ChannelOpenSettings *pNexusAspChannelOpenSettings
    );

/**
Summary:
Close nexus AspSim Context handle.
**/
void NEXUS_AspSim_Channel_Close(
    NEXUS_AspSim_ChannelHandle hNexusAspSimChannel
    );

/**
Summary:
Get default nexus asp channel start settings.
**/
void NEXUS_AspSim_Channel_GetDefaultStartSettings(
    NEXUS_AspSim_ChannelStartSettings *pNexusChannelStartSettings
    );

/**
Summary:
Start nexus AspSim Channel Context.
**/
int NEXUS_AspSim_Channel_Start(
    NEXUS_AspSim_ChannelHandle hNexusAspSimChannel,
    const NEXUS_AspSim_ChannelStartSettings   *pNexusAspSimChannelStartSettings
    );

/**
Summary:
Stop nexus AspSim Channel Context.
**/
int NEXUS_AspSim_Channel_Stop(
    NEXUS_AspSim_ChannelHandle hNexusAspSimChannel
    );

/**
Summary:
Nexus AspSim Process Io.
**/
void Nexus_AspSim_ProcessIo(
    NEXUS_AspSim_ChannelHandle hNexusAspChannel
    );

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
    );

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
    );

/**
Summary:
Provide latest socketState to the caller. This is only useful
once the stop response received from asp-cmodel/simulator .
**/

int NEXUS_AspSim_Channel_GetLatestSocketState(
    NEXUS_AspSim_ChannelHandle hNexusAspChannel,
    SocketState                *pSocketState     /*!< Pass address of the socketState structure which will be populated with latest socket state.*/
    );

#endif /* __NEXUS_ASPSIM_API_H__ */
