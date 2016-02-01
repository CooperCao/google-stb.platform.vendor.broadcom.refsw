/******************************************************************************
 *    (c)2010-2013 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *****************************************************************************/
#ifndef NXCLIENT_CONFIG_H__
#define NXCLIENT_CONFIG_H__

#include "nxclient.h"
#if NEXUS_HAS_INPUT_ROUTER
#include "nexus_input_client.h"
#else
typedef void *NEXUS_InputClientHandle;
#endif
#include "nexus_platform_standby.h"
#include "nexus_platform_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
The following "config" functions allow operations on other clients' resources.
**/

NEXUS_Error NxClient_Config_GetJoinSettings(
    NEXUS_ClientHandle client,
    NxClient_JoinSettings *pSettings
    );
    
void NxClient_Config_GetSurfaceClientComposition(
    NEXUS_ClientHandle client,
    NEXUS_SurfaceClientHandle surfaceClient,
    NEXUS_SurfaceComposition *pComposition
    );

NEXUS_Error NxClient_Config_SetSurfaceClientComposition(
    NEXUS_ClientHandle client,
    NEXUS_SurfaceClientHandle surfaceClient,
    const NEXUS_SurfaceComposition *pComposition
    );
    
typedef struct NxClient_ConnectList
{
    unsigned connectId[NXCLIENT_MAX_IDS];
} NxClient_ConnectList;

NEXUS_Error NxClient_Config_GetConnectList(
    NEXUS_ClientHandle client,
    NxClient_ConnectList *pList
    );
    
NEXUS_Error NxClient_Config_RefreshConnect(
    NEXUS_ClientHandle client,
    unsigned connectId
    );
    
void NxClient_Config_GetConnectSettings(
    NEXUS_ClientHandle client,
    unsigned connectId,
    NxClient_ConnectSettings *pSettings
    );

void NxClient_Config_GetInputClientServerFilter(
    NEXUS_ClientHandle client,
    NEXUS_InputClientHandle inputClient,
    unsigned *pFilter
    );

NEXUS_Error NxClient_Config_SetInputClientServerFilter(
    NEXUS_ClientHandle client,
    NEXUS_InputClientHandle inputClient,
    unsigned filter
    );

/**
An eVerified status client can use this to get status about a particular client
based on the operating system's process id.

If not eVerified, NEXUS_ClientHandle will not be usable.
**/
NEXUS_ClientHandle NxClient_Config_LookupClient(
    unsigned pid /* client's operating system process id */
    );

#ifdef __cplusplus
}
#endif

#endif
