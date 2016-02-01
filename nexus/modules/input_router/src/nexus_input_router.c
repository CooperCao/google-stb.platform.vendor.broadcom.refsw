/***************************************************************************
 *     (c)2011-2013 Broadcom Corporation
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
 **************************************************************************/
#include "nexus_base.h"
#include "nexus_input_router_module.h"
#include "nexus_input_router_impl.h"
#include "nexus_client_resources.h"

BDBG_MODULE(nexus_input_router);
struct nexus_inputrouter_list g_nexus_inputrouter_list;

NEXUS_InputRouterHandle NEXUS_InputRouter_Create( unsigned server_id )
{
    NEXUS_InputRouterHandle router;

    router = BKNI_Malloc(sizeof(*router));
    if (!router) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_InputRouter, router);
    router->server_id = server_id;
    BLST_S_INIT(&router->clients);
    
    BLST_S_INSERT_HEAD(&g_nexus_inputrouter_list, router, link);
    
    return router;
}


static void NEXUS_InputRouter_P_Finalizer( NEXUS_InputRouterHandle router )
{
    NEXUS_OBJECT_ASSERT(NEXUS_InputRouter, router);
    /* autoclean should have destroyed all clients first */
    BDBG_ASSERT(!BLST_S_FIRST(&router->clients));
    BLST_S_REMOVE(&g_nexus_inputrouter_list, router, NEXUS_InputRouter, link);
    NEXUS_OBJECT_DESTROY(NEXUS_InputRouter, router);
    BKNI_Free(router);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_InputRouter, NEXUS_InputRouter_Destroy);
    
NEXUS_InputClientHandle NEXUS_InputRouter_CreateClient( NEXUS_InputRouterHandle router, unsigned client_id )
{
    NEXUS_InputClientHandle client;
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(router, NEXUS_InputRouter);

    client = BKNI_Malloc(sizeof(*client));
    if (!client) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_InputClient, client);
    client->router = router;
    BLST_S_INSERT_HEAD(&client->router->clients, client, link);
    client->client_id = client_id;
    NEXUS_OBJECT_REGISTER(NEXUS_InputClient, client, Create);
    
    /* default settings */
    client->serverSettings.fifoSize = 100;
    client->serverSettings.filterMask = 0xFFFFFFFF; /* everything */
    client->settings.filterMask = 0xFFFFFFFF; /* everything */

    client->codeAvailableCallback = NEXUS_TaskCallback_Create(client, NULL);
    if (!client->codeAvailableCallback) {
        goto error;
    }
    
    /* allow the queue */
    rc = NEXUS_InputRouter_SetClientSettings(router, client, &client->serverSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    return client;

error:
    NEXUS_InputRouter_DestroyClient(client);
    return NULL;
}

static void NEXUS_InputClient_P_Release( NEXUS_InputClientHandle client )
{
    NEXUS_OBJECT_ASSERT(NEXUS_InputClient, client);
    if (client->acquired) {
        NEXUS_InputClient_Release(client);
    }
    NEXUS_OBJECT_UNREGISTER(NEXUS_InputClient, client, Destroy);
}

static void NEXUS_InputClient_P_Finalizer( NEXUS_InputClientHandle client )
{
    NEXUS_OBJECT_ASSERT(NEXUS_InputClient, client);

    BDBG_OBJECT_ASSERT(client->router, NEXUS_InputRouter);
    BLST_S_REMOVE(&client->router->clients, client, NEXUS_InputClient, link);

    if (client->codeAvailableCallback) {
        NEXUS_TaskCallback_Destroy(client->codeAvailableCallback);
    }
    if (client->fifo.data) {
        BKNI_Free(client->fifo.data);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_InputClient, client);
    BKNI_Free(client);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_InputClient, NEXUS_InputRouter_DestroyClient);
    
NEXUS_InputClientHandle NEXUS_InputClient_Acquire( unsigned client_id )
{
    NEXUS_InputRouterHandle router;
    for (router = BLST_S_FIRST(&g_nexus_inputrouter_list); router; router = BLST_S_NEXT(router, link)) {
        NEXUS_InputClientHandle client;
        BDBG_OBJECT_ASSERT(router, NEXUS_InputRouter);
        for (client = BLST_S_FIRST(&router->clients); client; client = BLST_S_NEXT(client, link)) {
            BDBG_OBJECT_ASSERT(client, NEXUS_InputClient);
            if (client->client_id == client_id) {
                int rc;
                if (client->acquired) {
                    BDBG_ERR(("client_id %d already acquired", client_id));
                    return NULL;
                }
                rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(inputClient,IdList,client_id);
                if (rc) { rc = BERR_TRACE(rc); return NULL; }
                client->acquired = true;
                return client;
            }
        }
    }
    BDBG_ERR(("client_id %d not created", client_id));
    BERR_TRACE(NEXUS_NOT_AVAILABLE);
    return NULL;    
}

void NEXUS_InputClient_Release( NEXUS_InputClientHandle client )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_InputClient);
    client->acquired = false;
    NEXUS_CLIENT_RESOURCES_RELEASE(inputClient,IdList,client->client_id);

    /* must clear out client's state */
    client->settings.codeAvailable.callback = NULL;
    NEXUS_InputClient_SetSettings(client, &client->settings);
}

void NEXUS_InputRouter_GetClientSettings( NEXUS_InputRouterHandle router, NEXUS_InputClientHandle client, NEXUS_InputRouterClientSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(router, NEXUS_InputRouter);
    BDBG_OBJECT_ASSERT(client, NEXUS_InputClient);
    if (client->router != router) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    }
    else {
        *pSettings = client->serverSettings;
    }
}
    
NEXUS_Error NEXUS_InputRouter_SetClientSettings( NEXUS_InputRouterHandle router, NEXUS_InputClientHandle client, const NEXUS_InputRouterClientSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(router, NEXUS_InputRouter);
    BDBG_OBJECT_ASSERT(client, NEXUS_InputClient);
    if (client->router != router) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (client->serverSettings.fifoSize != pSettings->fifoSize) {
        BKNI_Free(client->fifo.data);
        client->fifo.data = NULL;
        client->fifo.rptr = client->fifo.wptr = 0;
    }
    if (pSettings->fifoSize && !client->fifo.data) {
        client->fifo.data = BKNI_Malloc(sizeof(*client->fifo.data) * pSettings->fifoSize);
        if (!client->fifo.data) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
    }
    
    client->serverSettings = *pSettings;
    return 0;
}

void NEXUS_InputClient_GetSettings( NEXUS_InputClientHandle client, NEXUS_InputClientSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_InputClient);
    *pSettings = client->settings;
}

NEXUS_Error NEXUS_InputClient_SetSettings( NEXUS_InputClientHandle client, const NEXUS_InputClientSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(client, NEXUS_InputClient);
    NEXUS_TaskCallback_Set(client->codeAvailableCallback, &pSettings->codeAvailable);
    client->settings = *pSettings;
    return 0;
}

void NEXUS_InputRouter_GetDefaultCode( NEXUS_InputRouterCode *pCode )
{
    BKNI_Memset(pCode, 0, sizeof(*pCode));
    pCode->filterMask = 0xFFFFFFFF;
}

void NEXUS_InputRouter_SendCode( NEXUS_InputRouterHandle router, const NEXUS_InputRouterCode *pCode )
{
    NEXUS_InputClientHandle client;
    BDBG_OBJECT_ASSERT(router, NEXUS_InputRouter);
    for (client = BLST_S_FIRST(&router->clients); client; client = BLST_S_NEXT(client, link)) {
        /* apply server mask here */
        if (client->serverSettings.filterMask & pCode->filterMask) {
            NEXUS_InputRouter_SendClientCode(router, client, pCode);
        }
    }
}

void NEXUS_InputRouter_SendClientCode( NEXUS_InputRouterHandle router, NEXUS_InputClientHandle client, const NEXUS_InputRouterCode *pCode )
{
    if (client->router != router) return;

    /* do not send to unacquired client. apply client mask here */
    if (client->acquired &&
        (client->settings.filterMask & pCode->filterMask)) {
        if (!client->fifo.data) {
            BDBG_WRN(("client %d dropped data because it has no fifo", client->client_id));
            return;
        }
        client->fifo.data[client->fifo.wptr++] = *pCode;
        if (client->fifo.wptr == client->serverSettings.fifoSize) {
            client->fifo.wptr = 0;
        }
        if (client->fifo.wptr == client->fifo.rptr) {
            BDBG_WRN(("client %d overflowed", client->client_id));
        }
        NEXUS_TaskCallback_Fire(client->codeAvailableCallback);
    }
}

NEXUS_Error NEXUS_InputClient_GetCodes( NEXUS_InputClientHandle client, NEXUS_InputRouterCode *pCode, unsigned numEntries, unsigned *pNumReturned )
{
    unsigned n = 0;
    BDBG_OBJECT_ASSERT(client, NEXUS_InputClient);
    if (!pCode) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    while (n < numEntries && client->fifo.rptr != client->fifo.wptr) {
        pCode[n++] = client->fifo.data[client->fifo.rptr++];
        if (client->fifo.rptr == client->serverSettings.fifoSize) {
            client->fifo.rptr = 0;
        }
    }
    *pNumReturned = n;
    return 0;
}

NEXUS_Error NEXUS_InputClient_GetStatus( NEXUS_InputClientHandle handle, NEXUS_InputClientStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_InputClient);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->server.filterMask = handle->serverSettings.filterMask;
    return NEXUS_SUCCESS;
}
