/***************************************************************************
*  Broadcom Proprietary and Confidential. (c)2010-2016 Broadcom. All rights reserved.
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
*
* API Description:
*   API name: Platform (private)
*    Common part of all kernel servers
*
***************************************************************************/
#ifndef NEXUS_PLATFORM_SERVER_H__
#define NEXUS_PLATFORM_SERVER_H__

#include "nexus_platform_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************
Server management
******************************/

/**
Summary:
**/
typedef struct NEXUS_PlatformStartServerSettings
{
    bool allowUnprotectedClientsToCrash; /* deprecated */
    bool allowUnauthenticatedClients; /* default is false. if false, the server must call NEXUS_Platform_RegisterClient to allow clients to join.
                                 if true, unregistered clients can connect and unauthenticatedConfiguration will determine their status. */
    NEXUS_ClientConfiguration unauthenticatedConfiguration;
} NEXUS_PlatformStartServerSettings;

/**
Summary:
**/
void NEXUS_Platform_GetDefaultStartServerSettings(
    NEXUS_PlatformStartServerSettings *pSettings
    );

/**
Summary:
**/
NEXUS_Error NEXUS_Platform_StartServer(
    const NEXUS_PlatformStartServerSettings *pSettings /* attr{null_allowed=y} */
    );

/**
Summary:
**/
void NEXUS_Platform_StopServer(void);

/******************************
server-side client management

The following API's allow the server to manage which clients can connect
and what resources they are granted.
******************************/

/**
Summary:
Server-side settings per client.
**/
typedef struct NEXUS_ClientSettings
{
    NEXUS_ClientAuthenticationSettings authentication;
    NEXUS_ClientConfiguration configuration;
} NEXUS_ClientSettings;

/**
Summary:
**/
void NEXUS_Platform_GetDefaultClientSettings(
    NEXUS_ClientSettings *pSettings /* [out] */
    );

/**
Summary:
Server allows this client to connect
**/
NEXUS_ClientHandle NEXUS_Platform_RegisterClient( /* attr{destructor=NEXUS_Platform_UnregisterClient} */
    const NEXUS_ClientSettings *pSettings
    );

/**
Summary:
Server disallows this client. If connected, the client will be disconnected immediately.
**/
void NEXUS_Platform_UnregisterClient(
    NEXUS_ClientHandle client
    );

/**
Summary:
**/
typedef struct NEXUS_ClientStatus
{
    bool connected; /* is the client currently connected? */
    unsigned numJoins; /* total number of times the client has joined */
    unsigned pid; /* OS process id */
} NEXUS_ClientStatus;

/**
Summary:
**/
NEXUS_Error NEXUS_Platform_GetClientStatus(
    NEXUS_ClientHandle client,
    NEXUS_ClientStatus *pStatus /* [out] */
    );


/*
Summary:
Acquires object to prevent its destruction

Description:
Verifies that object is a valid instance of said type and it's accessible by a given client, and if test passes increases the object reference count
*/
NEXUS_Error NEXUS_Platform_AcquireObject(NEXUS_ClientHandle client, const NEXUS_InterfaceName *name, NEXUS_PlatformAnyObject object);

/*
Summary:
Releases object to allow its destruction

Description:
Verifies that object is a valid instance of said type, and if test passes decreases the object reference count
*/
void NEXUS_Platform_ReleaseObject(const NEXUS_InterfaceName *name, NEXUS_PlatformAnyObject object);

void NEXUS_Platform_GetClientResources(
    NEXUS_ClientHandle client,
    NEXUS_ClientResources *pResources
    );
    
NEXUS_Error NEXUS_Platform_SetClientResources(
    NEXUS_ClientHandle client,
    const NEXUS_ClientResources *pResources
    );

#ifdef __cplusplus
}
#endif

#endif
