/***************************************************************************
 * Copyright (C) 2004-2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ************************************************************/
#include "nexus_base.h"
#include "nexus_platform.h"
#include "nexus_platform_priv.h"
#include "nexus_platform_server.h"
#include "nexus_generic_driver_impl.h"
#include "nexus_client_resources.h"

BDBG_MODULE(nexus_platform_server);

struct NEXUS_Client
{
    NEXUS_OBJECT(NEXUS_Client);
    NEXUS_ClientSettings settings;
    BLST_S_ENTRY(NEXUS_Client) link;
    struct nexus_driver_client_state *driver_client;
    unsigned numJoins;
};

static struct {
    BLST_S_HEAD(clientlist, NEXUS_Client) clients;
} g_server;

NEXUS_Error NEXUS_Platform_P_InitServer(void)
{
    int rc;

    if (nexus_driver_state.active) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    rc = nexus_driver_server_postinit();
    if (rc) return BERR_TRACE(rc);

    nexus_driver_state.active = true;
    return 0;
}

void NEXUS_Platform_P_UninitServer(void)
{
    NEXUS_ClientHandle client;
    nexus_driver_state.active = false;
    while ((client = BLST_S_FIRST(&g_server.clients))) {
        NEXUS_Platform_UnregisterClient(client);
    }

    nexus_driver_server_preuninit();
}

NEXUS_Error NEXUS_Platform_StartServer(const NEXUS_PlatformStartServerSettings *pSettings)
{
    NEXUS_PlatformStartServerSettings defaultSettings;

    if (!pSettings) {
        NEXUS_Platform_GetDefaultStartServerSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    nexus_driver_state.serverSettings = *pSettings;
    return 0;
}

void NEXUS_Platform_StopServer(void)
{
#if NEXUS_SERVER_SUPPORT
    /* disable all clients except the server */
    nexus_driver_disable_clients(false);
#endif
}

NEXUS_ClientHandle NEXUS_Platform_RegisterClient( const NEXUS_ClientSettings *pSettings )
{
#if NEXUS_SERVER_SUPPORT
    NEXUS_ClientHandle client;

    BDBG_ASSERT(pSettings);
    client = BKNI_Malloc(sizeof(*client));
    if (!client) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_Client, client);
    client->settings = *pSettings;

    /* populate the driver with this client */
    client->driver_client = nexus_driver_create_client(&pSettings->authentication.certificate, &pSettings->configuration);
    if (!client->driver_client) {
        BKNI_Free(client);
        return NULL;
    }
    BLST_S_INSERT_HEAD(&g_server.clients, client, link);
    NEXUS_OBJECT_REGISTER(NEXUS_Client, client, Create);
    return client;
#else
    BSTD_UNUSED(pSettings);
    return NULL;
#endif
}

static void NEXUS_Client_P_Release( NEXUS_ClientHandle client )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    NEXUS_OBJECT_UNREGISTER(NEXUS_Client, client, Destroy);
}

static void NEXUS_Client_P_Finalizer( NEXUS_ClientHandle client )
{
#if NEXUS_SERVER_SUPPORT
    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    BLST_S_REMOVE(&g_server.clients, client, NEXUS_Client, link);
    nexus_driver_destroy_client(client->driver_client);
    NEXUS_OBJECT_DESTROY(NEXUS_Client, client);
    BKNI_Free(client);
#else
    BSTD_UNUSED(client);
#endif
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Client, NEXUS_Platform_UnregisterClient);

struct b_objdb_client *nexus_p_platform_objdb_client(NEXUS_ClientHandle client)
{
    return &client->driver_client->client;
}

void NEXUS_Platform_GetClientConfiguration_driver( NEXUS_ClientConfiguration *pSettings )
{
    nexus_driver_get_client_configuration(b_objdb_get_client(), pSettings);
}

NEXUS_Error NEXUS_Platform_GetClientStatus( NEXUS_ClientHandle client, NEXUS_ClientStatus *pStatus )
{
#if NEXUS_SERVER_SUPPORT
    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->connected = client->driver_client->joined;
    pStatus->numJoins = client->driver_client->numJoins;
    pStatus->pid = client->driver_client->pid;
    return 0;
#else
    BSTD_UNUSED(client);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

/**
The following client functions only have meaning in the client, not in the server.
They exist here because the server is the superset of the client.
**/
NEXUS_Error NEXUS_Platform_AuthenticatedJoin(const NEXUS_ClientAuthenticationSettings *pSettings)
{
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_Platform_GetDefaultClientAuthenticationSettings( NEXUS_ClientAuthenticationSettings *pSettings )
{
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_Platform_AcquireObject(NEXUS_ClientHandle client, const NEXUS_InterfaceName *type, void *object)
{
    return NEXUS_Platform_P_AcquireObject(&client->driver_client->client, type, object);
}

void NEXUS_Platform_ReleaseObject(const NEXUS_InterfaceName *type, void *object)
{
    NEXUS_Platform_P_ReleaseObject(type, object);
    return;
}

void NEXUS_Platform_GetClientResources( NEXUS_ClientHandle client, NEXUS_ClientResources *pResources )
{
#if NEXUS_SERVER_SUPPORT
    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    *pResources = client->driver_client->client.resources.allowed;
#else
    BSTD_UNUSED(client);
    BSTD_UNUSED(pResources);
#endif
}

NEXUS_Error NEXUS_Platform_SetClientResources( NEXUS_ClientHandle client, const NEXUS_ClientResources *pResources )
{
#if NEXUS_SERVER_SUPPORT
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(client, NEXUS_Client);
    rc = nexus_client_resources_check(pResources, &client->driver_client->client.resources.used);
    if (rc) return BERR_TRACE(rc);
    client->driver_client->client.resources.allowed = *pResources;
    return 0;
#else
    BSTD_UNUSED(client);
    BSTD_UNUSED(pResources);
    return 0;
#endif
}
