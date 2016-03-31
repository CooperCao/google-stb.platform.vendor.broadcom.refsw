/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ******************************************************************************/
#ifndef NXCLIENT_STANDBY_H__
#define NXCLIENT_STANDBY_H__

#include "nexus_types.h"
#include "nexus_platform_standby.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxClient_StandbyTransition {
    NxClient_StandbyTransition_eNone,      /* not going into standby, no ack needed */
    NxClient_StandbyTransition_eAckNeeded, /* going into standby, ack needed from this client */
    NxClient_StandbyTransition_ePending,   /* going into standby, ack received from this client */
    NxClient_StandbyTransition_eDone,      /* nexus in standby, client and put linux into standby, no ack needed */
    NxClient_StandbyTransition_eMax
} NxClient_StandbyTransition;

/**
Client polls periodically to get a change ins standby settings
Also same status structure returns the wakeup device status
**/
typedef struct NxClient_StandbyStatus
{
    NEXUS_PlatformStandbySettings settings; /* the desired standby state */
    NEXUS_PlatformStandbyStatus status;     /* wake up status */
    bool standbyTransition;                 /* Deprecated. Use NxClient_StandbyStatus.transition instead. Set to true when transition = NxClient_StandbyTransition_eDone*/
    NxClient_StandbyTransition transition;
} NxClient_StandbyStatus;

/**
Client uses this function to poll for status change
and after wakeup to get the wakeup device status
**/
NEXUS_Error NxClient_GetStandbyStatus(
    NxClient_StandbyStatus *pStatus
    );

/**
Register and unregister that this client wants to acknowledge
standby. If no standby ack is registered, the server views
this client as ignoring standby.

This allows multiple contexts within a single process to register
their wish to acknowledge standby.

For backward compat, NxClient_JoinSettings.ignoreStandbyRequest defaults to false,
which is an implicit call to NxClient_RegisterAcknowledgeStandby.
The first call to NxClient_RegisterAcknowledgeStandby will return that id.
**/
unsigned NxClient_RegisterAcknowledgeStandby(void);
void NxClient_UnregisterAcknowledgeStandby(
    unsigned id
    );

/**
Client acknowledges the standby state using registered id.

If NxClient_RegisterAcknowledgeStandby was not called (implicit id), then the value of id is ignored.
**/
void NxClient_AcknowledgeStandby(
    unsigned id
    );

typedef enum NxClient_StandbyCpuMode {
    NxClient_StandbyCpuMode_eWarmBoot,
    NxClient_StandbyCpuMode_eColdBoot,
    NxClient_StandbyCpuMode_eMax
} NxClient_StandbyCpuMode;

/**
Standby settings structure used by a client
to request one of standby modes
**/
typedef struct NxClient_StandbySetttings
{
    NEXUS_PlatformStandbySettings settings;
    NxClient_StandbyCpuMode mode; /* Deprecated */
} NxClient_StandbySettings;

void NxClient_GetDefaultStandbySettings(
    NxClient_StandbySettings *pSettings
    );

/**
Client uses this api to request a standby mode
**/
NEXUS_Error NxClient_SetStandbySettings(
    const NxClient_StandbySettings *pSettings
    );

#ifdef __cplusplus
}
#endif

#endif /* NXCLIENT_STANDBY_H__ */
