/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 **************************************************************************/

#include "nexus_sage_module.h"

/* magnum basemodules */
#include "berr.h"
#include "bkni.h"

#include "blst_list.h"
#include "blst_squeue.h"

#include "nexus_sage.h"
#include "bsagelib.h"
#include "bsagelib_client.h"
/* #define BSAGELIB_SAGE_MESSAGE_DEBUG 1 */
#include "bsagelib_rpc.h"

BDBG_MODULE(nexus_sage);

#define SAGE_INDICATION_CONTEXT_INC_SIZE 16

/* a cache of indication context is used accross all instances/channels */
#define NEXUS_SAGE_INDICATION_CONTEXT_CACHE_INC 24
#define NEXUS_SAGE_INDICATION_CONTEXT_CACHE_LOW (NEXUS_SAGE_INDICATION_CONTEXT_CACHE_INC/3)

/* Internal global context */
static struct NEXUS_Sage_P_State {
    /* cache of context to use for indications accross all channels */
    NEXUS_SageIndicationQueueHead indicationsCache;
    uint32_t indicationsCacheCount;
} g_sage;

/*
  Internal contexts are arranged as follow:

  +--------------------------------+
  |     Global g_NEXUS_sageModule  |
  +--|-----------------------------+
     |
     |
  +--+--------+
  | Sage List +--> Instance --> ... --> NULL
  +-----------+       |
                      |
                      |
           +----------+----+
           |  Channel List |--> Channel --> ... --> NULL
           +---------------+
*/


/*
 * Local functions
 */
static void NEXUS_Sage_P_Finalizer(NEXUS_SageHandle sage);
static void NEXUS_SageChannel_P_Finalizer(NEXUS_SageChannelHandle channel);
static void NEXUS_Sage_P_ResponseCallback_isr(
    BSAGElib_RpcRemoteHandle handle, void *async_argument, uint32_t async_id, BERR_Code error);
static void NEXUS_Sage_P_IndicationRecvCallback_isr(
    BSAGElib_RpcRemoteHandle handle, void *async_argument, uint32_t id, uint32_t value);
static BERR_Code NEXUS_Sage_P_CallbackRequestCallback_isr(
    BSAGElib_RpcRemoteHandle handle, void *async_argument);
static void NEXUS_Sage_P_TATerminateCallback_isr(
    BSAGElib_RpcRemoteHandle handle, void *async_argument, uint32_t reason, uint32_t source);
static NEXUS_Error NEXUS_Sage_P_IndicationContextCacheInc(void);


/*
 * Public API
 */


/* semi-private : called from nexus_sage_module */
void NEXUS_Sage_P_Cleanup(void)
{
    NEXUS_SageIndicationContext *pIndicationContext = NULL;
    /* empty the indication context cache */
    while ((pIndicationContext = BLST_SQ_FIRST(&g_sage.indicationsCache)) != NULL) {
        BDBG_ASSERT(g_sage.indicationsCacheCount);
        BLST_SQ_REMOVE_HEAD(&g_sage.indicationsCache, link);
        g_sage.indicationsCacheCount--;
        BKNI_Free(pIndicationContext);
    }
}

/* semi-private : called from nexus_sage_module */
void NEXUS_Sage_P_Init(void)
{
    BKNI_Memset(&g_sage, 0, sizeof(g_sage));

    BLST_SQ_INIT(&g_sage.indicationsCache);
}


static NEXUS_Error NEXUS_Sage_P_IndicationContextCacheInc(void)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned i;
    if (g_sage.indicationsCacheCount <= NEXUS_SAGE_INDICATION_CONTEXT_CACHE_LOW) {
        for (i = 0; i < NEXUS_SAGE_INDICATION_CONTEXT_CACHE_INC; i++) {
            NEXUS_SageIndicationContext *pIndicationContext = BKNI_Malloc(sizeof(*pIndicationContext));
            if (pIndicationContext == NULL) {
                BDBG_ERR(("OOM? Cannot allocate indication context (%u bytes).",
                          (unsigned)sizeof(*pIndicationContext)));
                rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
                goto end;
            }
            BKNI_EnterCriticalSection();
            BLST_SQ_INSERT_HEAD(&g_sage.indicationsCache, pIndicationContext, link);
            g_sage.indicationsCacheCount++;
            BKNI_LeaveCriticalSection();
        }
    }
end:
    return rc;
}

/* BSAGElib_Rpc_ResponseCallback prototypeCallback, see bsagelib.h */
static void NEXUS_Sage_P_ResponseCallback_isr(
    BSAGElib_RpcRemoteHandle handle, void *async_argument, uint32_t async_id, BERR_Code error)
{
    NEXUS_SageChannelHandle channel = (NEXUS_SageChannelHandle)async_argument;

    BSTD_UNUSED(handle);
    BSTD_UNUSED(async_id);

    if (!channel) {
        BDBG_ERR(("%s: retrieved channel is NULL", BSTD_FUNCTION));
        return;
    }

    if (error == BERR_SUCCESS) {
        NEXUS_IsrCallback_Fire_isr(channel->successCallback);
    }
    else {
        channel->status.lastError = NEXUS_ERROR_SAGE_SIDE;
        channel->status.lastErrorSage = error;
        NEXUS_IsrCallback_Fire_isr(channel->errorCallback);
    }
}

static void NEXUS_Sage_P_IndicationRecvCallback_isr(
    BSAGElib_RpcRemoteHandle handle, void *async_argument, uint32_t id, uint32_t value)
{
    NEXUS_SageChannelHandle channel = (NEXUS_SageChannelHandle)async_argument;
    NEXUS_SageIndicationContext *pIndicationContext = NULL;

    BSTD_UNUSED(handle);

    if (!channel) {
        BDBG_ERR(("%s: retrieved channel is NULL", BSTD_FUNCTION));
        return;
    }

    if (channel->indicationCallback == NULL) {
        BDBG_ERR(("%s: No callback, channel=%p, remote=%p - indication [id=0x%x, value=0x%x] is lost",
                  BSTD_FUNCTION, (void *)channel, (void *)handle, id, value));
        return;
    }

    pIndicationContext = BLST_SQ_FIRST(&g_sage.indicationsCache);
    if (pIndicationContext == NULL) {
        BDBG_ERR(("%s: indication overflow! no more context available", BSTD_FUNCTION));
        BDBG_ERR(("%s: channel=%p, handle=%p - indication [id=0x%x, value=0x%x] is lost",
                  BSTD_FUNCTION, (void *)channel, (void *)handle, id, value));
        return;
    }

    BDBG_ASSERT(g_sage.indicationsCacheCount);

    /* remove it from the cache  */
    BLST_SQ_REMOVE_HEAD(&g_sage.indicationsCache, link);
    g_sage.indicationsCacheCount--;

    /* set to received values */
    pIndicationContext->id = id;
    pIndicationContext->value = value;

    /* place it inside the channel queue */
    BLST_SQ_INSERT_HEAD(&channel->indications, pIndicationContext, link);

    /* fire the callback to indicate application layer that an indication is available */
    NEXUS_IsrCallback_Fire_isr(channel->indicationCallback);
}

static BERR_Code NEXUS_Sage_P_CallbackRequestCallback_isr(
    BSAGElib_RpcRemoteHandle handle, void *async_argument)
{
    NEXUS_SageChannelHandle channel = (NEXUS_SageChannelHandle)async_argument;
    BERR_Code rc = BERR_SUCCESS;

    BSTD_UNUSED(handle);

    if (!channel) {
        BDBG_ERR(("%s: retrieved channel is NULL", BSTD_FUNCTION));
        rc = BSAGE_ERR_INTERNAL;
        goto end;
    }

    if (channel->status.pendingCallbackRequest) {
        BDBG_ERR(("%s: channel is already processing a callback request", BSTD_FUNCTION));
        rc = BSAGE_ERR_INTERNAL;
        goto end;
    }

    if (channel->callbackRequestRecvCallback) {
        channel->status.pendingCallbackRequest = true;
        NEXUS_IsrCallback_Fire_isr(channel->callbackRequestRecvCallback);
    }
    else {
        BDBG_ERR(("%s: callback requests are not enabled for channel", BSTD_FUNCTION));
        rc = BSAGE_ERR_INTERNAL;
        goto end;
    }

end:
    return rc;
}

static void NEXUS_Sage_P_TATerminateCallback_isr(
    BSAGElib_RpcRemoteHandle handle, void *async_argument, uint32_t reason, uint32_t source)
{
    NEXUS_SageChannelHandle channel = (NEXUS_SageChannelHandle)async_argument;

    BSTD_UNUSED(handle);
    BSTD_UNUSED(reason);
    BSTD_UNUSED(source);

    if (!channel) {
        BDBG_ERR(("%s: retrieved channel is NULL", BSTD_FUNCTION));
        goto end;
    }

    channel->status.lastError = NEXUS_ERROR_SAGE_SIDE;
    channel->status.lastErrorSage = BSAGE_ERR_TA_TERMINATED;

    NEXUS_IsrCallback_Fire_isr(channel->taTerminateCallback);

end:
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Sage, NEXUS_Sage_Close);

/* Get default channel settings */
void NEXUS_Sage_GetDefaultOpenSettings(
    NEXUS_SageOpenSettings *pSettings /* [out] */)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->watchdogCallback);
}

/* semi-private: called inside nexus_sage_priv for standby operation
   Nexus Sage is also a SAGElib client. */
void NEXUS_Sage_P_GetSAGElib_ClientSettings(BSAGElib_ClientSettings *pSAGElibClientSettings)
{
    BSAGElib_GetDefaultClientSettings(g_NEXUS_sageModule.hSAGElib, pSAGElibClientSettings);
    /* Avoid locking SAGE from this instance as we are already in sage module */
    pSAGElibClientSettings->i_sync_sage.lock = NULL;
    pSAGElibClientSettings->i_sync_sage.unlock = NULL;
    pSAGElibClientSettings->i_rpc.response_isr = NEXUS_Sage_P_ResponseCallback_isr;
 }

/* Create a sage instance */
NEXUS_SageHandle NEXUS_Sage_Open(
    unsigned index,
    const NEXUS_SageOpenSettings *pSettings)
{
    NEXUS_SageOpenSettings settings;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_SageHandle sage = NULL;

    BDBG_ENTER(NEXUS_Sage_Open);

    BSTD_UNUSED(index);

    if (pSettings == NULL) {
        NEXUS_Sage_GetDefaultOpenSettings(&settings);
        pSettings = &settings;
    }

    /* check if SAGE has booted */
    if(!NEXUS_Sage_P_CheckSageBooted()) {
        BDBG_ERR(("SAGE boot failure"));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        NEXUS_Sage_P_Cleanup();
        goto end;
    }

    /* Init Sage instance */
    sage = BKNI_Malloc(sizeof(*sage));
    if (sage == NULL) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }
    NEXUS_OBJECT_INIT(NEXUS_Sage, sage);

    /* watchdog initialization cannot be realized before sage OBJECT_INIT */
    rc = NEXUS_Sage_P_WatchdogLink(sage, &pSettings->watchdogCallback);
    if (rc != NEXUS_SUCCESS) { goto end; }

    BLST_D_INIT(&sage->channels);

    /* populate context cache if required */
    rc = NEXUS_Sage_P_IndicationContextCacheInc();
    if (rc != NEXUS_SUCCESS) { goto end; }

    /* Nexus Sage is also a SAGElib client. */
    {
        BSAGElib_ClientSettings SAGElibClientSettings;
        BERR_Code SAGElib_rc;

        NEXUS_Sage_P_GetSAGElib_ClientSettings(&SAGElibClientSettings);
        SAGElibClientSettings.i_rpc.response_isr = NEXUS_Sage_P_ResponseCallback_isr;
        SAGElibClientSettings.i_rpc.callbackRequest_isr = NEXUS_Sage_P_CallbackRequestCallback_isr;
        SAGElibClientSettings.i_rpc.taTerminate_isr = NEXUS_Sage_P_TATerminateCallback_isr;
        SAGElibClientSettings.i_rpc.indicationRecv_isr = NEXUS_Sage_P_IndicationRecvCallback_isr;
        SAGElib_rc = BSAGElib_OpenClient(g_NEXUS_sageModule.hSAGElib,
                                         &sage->hSAGElibClient,
                                         &SAGElibClientSettings);
        if (SAGElib_rc != BERR_SUCCESS) {
            rc = BERR_TRACE(NEXUS_OS_ERROR);
            goto end;
        }
    }

    BKNI_EnterCriticalSection();
    BLST_S_INSERT_HEAD(&g_NEXUS_sageModule.instances, sage, link);
    BKNI_LeaveCriticalSection();

    sage->valid = true;
    BDBG_MSG(("NEW sage=%p", (void *)sage));

end:
    if (rc != NEXUS_SUCCESS) {
        if (sage) {
            NEXUS_Sage_P_WatchdogUnlink(sage);
            NEXUS_OBJECT_DESTROY(NEXUS_Sage, sage);
            BKNI_Free(sage);
            sage = NULL;
        }
    }
    BDBG_LEAVE(NEXUS_Sage_Open);
    return sage;
}

/* NEXUS_Sage_Close's destructor.
 * Called by NEXUS_Sage_Close upper API and
 *        by NEXUS garbage collecting */
static void NEXUS_Sage_P_Finalizer(NEXUS_SageHandle sage)
{
    BDBG_ENTER(NEXUS_Sage_P_Finalizer);

    NEXUS_OBJECT_ASSERT(NEXUS_Sage, sage);

    BDBG_MSG(("DEL sage=%p", (void *)sage));

    /* NEXUS_OBJECT_ACQUIRE guarantees that no channels remain */
    BDBG_ASSERT(!BLST_D_FIRST(&sage->channels));

    BKNI_EnterCriticalSection();
    BLST_S_REMOVE(&g_NEXUS_sageModule.instances, sage, NEXUS_Sage, link);
    BKNI_LeaveCriticalSection();

    BSAGElib_CloseClient(sage->hSAGElibClient);
    sage->hSAGElibClient = NULL;

    NEXUS_Sage_P_WatchdogUnlink(sage);

    /* CANNOT cleanup Sage as watchdog is always active */
    /* sage context is zeroed in NEXUS_OBJECT_DESTROY */
    NEXUS_OBJECT_DESTROY(NEXUS_Sage, sage);

    BKNI_Free(sage);

    BDBG_LEAVE(NEXUS_Sage_P_Finalizer);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_SageChannel, NEXUS_Sage_DestroyChannel);

/* Get default channel settings */
void NEXUS_SageChannel_GetDefaultSettings(
    NEXUS_SageChannelSettings *pSettings /* [out] */)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->successCallback);
    NEXUS_CallbackDesc_Init(&pSettings->errorCallback);
    NEXUS_CallbackDesc_Init(&pSettings->callbackRequestRecvCallback);
    NEXUS_CallbackDesc_Init(&pSettings->taTerminateCallback);
    NEXUS_CallbackDesc_Init(&pSettings->indicationCallback);
}

/* Create a channel on a sage instance */
NEXUS_SageChannelHandle NEXUS_Sage_CreateChannel(
    NEXUS_SageHandle sage,
    const NEXUS_SageChannelSettings *pSettings)
{
    NEXUS_SageChannelHandle ret = NULL;
    NEXUS_SageChannelHandle channel = NULL;

    NEXUS_CallbackSettings callbackSettings;

    BDBG_ENTER(NEXUS_Sage_CreateChannel);

    BDBG_ASSERT( pSettings );
    NEXUS_OBJECT_ASSERT(NEXUS_Sage, sage);

    if (!pSettings->successCallback.callback) {
        BDBG_ERR(("successCallback is a mandatory parameter"));
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    if (!pSettings->errorCallback.callback) {
        BDBG_ERR(("errorCallback is a mandatory parameter"));
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }
    if (!pSettings->taTerminateCallback.callback) {
        BDBG_ERR(("taTerminateCallback is a mandatory parameter"));
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }
#if 0 /* UNUSED */
    if (!NEXUS_Sage_P_IsHeapValid(pSettings->heap)) {
        BDBG_ERR(("given heap is not valid"));
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }
#endif

    channel = BKNI_Malloc(sizeof(*channel));
    if (channel == NULL) {
        BDBG_ERR(("OOM? Cannot allocate channel instance context (%u bytes).",
                  (unsigned)sizeof(*channel)));
        (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto end;
    }

    NEXUS_OBJECT_INIT(NEXUS_SageChannel, channel);

    /* bound to channel if channel is a NEXUS_OBJECT */
    NEXUS_Callback_GetDefaultSettings(&callbackSettings);
    channel->successCallback = NEXUS_IsrCallback_Create(channel, &callbackSettings);
    if (channel->successCallback == NULL) {
        BDBG_ERR(("NEXUS_IsrCallback_Create failure for successCallback"));
        (void)BERR_TRACE(NEXUS_OS_ERROR);
        goto end;
    }
    NEXUS_IsrCallback_Set(channel->successCallback, &pSettings->successCallback);

    NEXUS_Callback_GetDefaultSettings(&callbackSettings);
    channel->errorCallback = NEXUS_IsrCallback_Create(channel, &callbackSettings);
    if (channel->errorCallback == NULL) {
        BDBG_ERR(("NEXUS_IsrCallback_Create failure for errorCallback"));
        (void)BERR_TRACE(NEXUS_OS_ERROR);
        goto end;
    }
    NEXUS_IsrCallback_Set(channel->errorCallback, &pSettings->errorCallback);

    NEXUS_Callback_GetDefaultSettings(&callbackSettings);
    channel->taTerminateCallback = NEXUS_IsrCallback_Create(channel, &callbackSettings);
    if (channel->taTerminateCallback == NULL) {
        BDBG_ERR(("NEXUS_IsrCallback_Create failure for taTerminateCallback"));
        (void)BERR_TRACE(NEXUS_OS_ERROR);
        goto end;
    }
    NEXUS_IsrCallback_Set(channel->taTerminateCallback, &pSettings->taTerminateCallback);

    if (pSettings->callbackRequestRecvCallback.callback) {
        NEXUS_Callback_GetDefaultSettings(&callbackSettings);
        channel->callbackRequestRecvCallback = NEXUS_IsrCallback_Create(channel, &callbackSettings);
        if (channel->callbackRequestRecvCallback == NULL) {
            BDBG_ERR(("NEXUS_IsrCallback_Create failure for callbackRequestRecvCallback"));
            (void)BERR_TRACE(NEXUS_OS_ERROR);
            goto end;
        }
        NEXUS_IsrCallback_Set(channel->callbackRequestRecvCallback, &pSettings->callbackRequestRecvCallback);
    }

    if (pSettings->indicationCallback.callback) {
        NEXUS_Callback_GetDefaultSettings(&callbackSettings);
        channel->indicationCallback = NEXUS_IsrCallback_Create(channel, &callbackSettings);
        if (channel->indicationCallback == NULL) {
            BDBG_ERR(("NEXUS_IsrCallback_Create failure for indicationCallback"));
            (void)BERR_TRACE(NEXUS_OS_ERROR);
            goto end;
        }
        NEXUS_IsrCallback_Set(channel->indicationCallback, &pSettings->indicationCallback);
    }

    channel->sage = sage;
    /* Acquire Sage instance will strengthen Channel <- -> Sage dependency:
     * Sage instance cannot be destroyed until any channel remain */
    NEXUS_OBJECT_ACQUIRE(channel, NEXUS_Sage, channel->sage);

    BLST_SQ_INIT(&channel->indications);

    /* insert head in order to act as a LIFO for garbage management */
    BKNI_EnterCriticalSection();
    if (sage->valid) {
        BLST_D_INSERT_HEAD(&sage->channels, channel, link);
        BKNI_LeaveCriticalSection();

        /* success */
        ret = channel;
    }
    else {
        BKNI_LeaveCriticalSection();
        BDBG_ERR(("this sage instance is not valid anymore (watchdog reset?)"));
    }

end:
    if (ret == NULL) {
        /* error cleanup */
        if (channel != NULL) {
            NEXUS_Sage_DestroyChannel(channel);
        }
    }
    BDBG_LEAVE(NEXUS_Sage_CreateChannel);
    return ret;
}

/* Destroy a channel created by NEXUS_Sage_CreateChannel */
static void NEXUS_SageChannel_P_Finalizer(NEXUS_SageChannelHandle channel)
{
    BDBG_ENTER(NEXUS_SageChannel_P_Finalizer);

    NEXUS_OBJECT_ASSERT(NEXUS_SageChannel, channel);

    BKNI_EnterCriticalSection();
    BLST_D_REMOVE(&channel->sage->channels, channel, link);

    /* remove any pending indication
       keep the critical section because we are accessing the indicationsCache */
    {
        NEXUS_SageIndicationContext *pIndicationContext = NULL;
        while ((pIndicationContext = BLST_SQ_FIRST(&channel->indications)) != NULL) {
            /* place it back inside the cache */
            BLST_SQ_REMOVE_HEAD(&channel->indications, link);
            BLST_SQ_INSERT_HEAD(&g_sage.indicationsCache, pIndicationContext, link);
            g_sage.indicationsCacheCount++;
        }
    }

    BKNI_LeaveCriticalSection();

    if (channel->sagelib_remote) {
        BSAGElib_Rpc_RemoveRemote(channel->sagelib_remote);
        channel->sagelib_remote = NULL;
    }

    NEXUS_OBJECT_RELEASE(channel, NEXUS_Sage, channel->sage);

    if (channel->successCallback) {
        NEXUS_IsrCallback_Destroy(channel->successCallback);
    }
    if (channel->errorCallback) {
        NEXUS_IsrCallback_Destroy(channel->errorCallback);
    }
    if (channel->taTerminateCallback) {
        NEXUS_IsrCallback_Destroy(channel->taTerminateCallback);
    }
    if (channel->callbackRequestRecvCallback) {
        NEXUS_IsrCallback_Destroy(channel->callbackRequestRecvCallback);
    }
    if (channel->indicationCallback) {
        NEXUS_IsrCallback_Destroy(channel->indicationCallback);
    }

    /* channel context is zeroed in NEXUS_OBJECT_DESTROY */
    NEXUS_OBJECT_DESTROY(NEXUS_SageChannel, channel);

    BKNI_Free(channel);

    BDBG_LEAVE(NEXUS_SageChannel_P_Finalizer);
}

NEXUS_Error NEXUS_SageChannel_GetStatus(
    NEXUS_SageChannelHandle channel,
    NEXUS_SageChannelStatus *pStatus /* [out] */
    )
{
    BDBG_ENTER(NEXUS_SageChannel_GetStatus);

    NEXUS_OBJECT_ASSERT(NEXUS_SageChannel, channel);

    BDBG_ASSERT(pStatus);

    BKNI_EnterCriticalSection();
    *pStatus = channel->status;
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(NEXUS_SageChannel_GetStatus);
    return NEXUS_SUCCESS;
}

/* Send a command to SAGE-side through a channel */
NEXUS_Error NEXUS_SageChannel_SendCommand(
    NEXUS_SageChannelHandle channel,
    const NEXUS_SageCommand *pCommand)
{
    NEXUS_Error ret = NEXUS_SUCCESS;
    BERR_Code SAGElib_rc;
    BSAGElib_RpcCommand SAGElib_command;

    BDBG_ENTER(NEXUS_SageChannel_SendCommand);

    NEXUS_OBJECT_ASSERT(NEXUS_SageChannel, channel);
    BDBG_ASSERT(pCommand);

    /* check if SAGE booted : covers the Watchdog event */
    if(!NEXUS_Sage_P_CheckSageBooted()) {
        BDBG_ERR(("SAGE (re)boot failure"));
        ret = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        NEXUS_Sage_P_Cleanup();
        goto end;
    }

    if (!channel->sagelib_remote) {
        channel->sagelib_remote = BSAGElib_Rpc_AddRemote(channel->sage->hSAGElibClient,
                                                         pCommand->platformId,
                                                         pCommand->moduleId,
                                                         channel);
        if (!channel->sagelib_remote) {
            BDBG_ERR(("Cannot bound channel to SAGElib remote"));
            ret = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto end;
        }
    }
    channel->status.lastError = NEXUS_SUCCESS;
    channel->status.lastErrorSage = 0;

    SAGElib_command.systemCommandId = pCommand->systemCommandId;
    SAGElib_command.moduleCommandId = pCommand->moduleCommandId;
    SAGElib_command.containerOffset = (uint64_t)pCommand->payloadOffset;
    SAGElib_command.containerVAddr = NULL;/* Containers are tramsformed in Application context. See SRAI. */
    SAGElib_rc = BSAGElib_Rpc_SendCommand(channel->sagelib_remote, &SAGElib_command, NULL);
    if (SAGElib_rc != BERR_SUCCESS) {
        if (SAGElib_rc == BSAGE_ERR_RESET) {
            ret = BERR_TRACE(NEXUS_ERROR_SAGE_WATCHDOG);
        }
        else {
            ret = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        channel->status.lastError = ret;
        BDBG_ERR(("BSAGElib_Rpc_SendCommand() failure %d", SAGElib_rc));
        goto end;
    }

end:
    BDBG_LEAVE(NEXUS_SageChannel_SendCommand);
    return ret;
}

NEXUS_Error NEXUS_Sage_GetStatus(
    NEXUS_SageStatus *pStatus)
{
    NEXUS_Error ret = NEXUS_SUCCESS;

    BDBG_ENTER(NEXUS_Sage_GetStatus);

    if(!NEXUS_Sage_P_CheckSageBooted()) {
        BDBG_ERR(("SAGE failed to boot, failing status."));
        ret = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto end;
    }

    ret = NEXUS_Sage_P_Status(pStatus);

end:
    BDBG_LEAVE(NEXUS_Sage_GetStatus);
    return ret;
}

NEXUS_Error NEXUS_Sage_GetEncKey(uint8_t *pKeyBuff, uint32_t keySize, uint32_t *pOutKeySize)
{
#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
    NEXUS_Error ret = NEXUS_SUCCESS;

    BDBG_ENTER(NEXUS_Sage_GetEncKey);

    if(!NEXUS_Sage_P_CheckSageBooted()) {
        BDBG_ERR(("SAGE failed to boot, failing status."));
        ret = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto end;
    }

    ret = NEXUS_Sage_P_GetEncKey(pKeyBuff, keySize, pOutKeySize);

end:
    BDBG_LEAVE(NEXUS_Sage_GetEncKey);
    return ret;
#else
    BSTD_UNUSED(pKeyBuff);
    BSTD_UNUSED(keySize);
    BSTD_UNUSED(pOutKeySize);
    BDBG_ERR(("NEXUS_Sage_GetEncKey is not supported in SAGE 3.x and later"));
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error NEXUS_Sage_GetLogBuffer(uint8_t *pBuff, uint32_t bufSize,
                                    uint32_t *pBufSize,uint32_t *pWrapBufSize,
                                    uint32_t *pActualBufSize,uint32_t *pActualWrapBufSize)
{
#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
    NEXUS_Error ret = NEXUS_SUCCESS;

    BDBG_ENTER(NEXUS_Sage_GetLogBuffer);

    if(!NEXUS_Sage_P_CheckSageBooted()) {
        BDBG_ERR(("SAGE failed to boot, failing status."));
        ret = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto end;
    }

    ret = NEXUS_Sage_P_GetLogBuffer(pBuff, bufSize, pBufSize, pWrapBufSize, pActualBufSize, pActualWrapBufSize);

end:
    BDBG_LEAVE(NEXUS_Sage_GetLogBuffer);
    return ret;
#else
    BSTD_UNUSED(pBuff);
    BSTD_UNUSED(bufSize);
    BSTD_UNUSED(pBufSize);
    BSTD_UNUSED(pWrapBufSize);
    BSTD_UNUSED(pActualBufSize);
    BSTD_UNUSED(pActualWrapBufSize);
    BDBG_ERR(("NEXUS_Sage_GetLogBuffer is not supported in SAGE 3.x and later"));
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

/* Send a response (to a callback request) to SAGE-side through a channel
 * Note: the request came from BSAGElib and was propagated up through NEXUS_SageChannelHandle::callbackRequestRecvCallback */
NEXUS_Error NEXUS_SageChannel_SendResponse(
    NEXUS_SageChannelHandle channel,
    const NEXUS_SageResponse *pResponse)
{
    NEXUS_Error ret = NEXUS_SUCCESS;
    BERR_Code SAGElib_rc;

    BDBG_ENTER(NEXUS_SageChannel_SendResponse);

    NEXUS_OBJECT_ASSERT(NEXUS_SageChannel, channel);
    BDBG_ASSERT(pResponse);

    /* check if SAGE booted : covers the Watchdog event */
    if(!NEXUS_Sage_P_CheckSageBooted()) {
        BDBG_ERR(("SAGE (re)boot failure"));
        ret = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        NEXUS_Sage_P_Cleanup();
        goto end;
    }

    if (!channel->sagelib_remote) {
        BDBG_ERR(("Cannot bound channel to SAGElib remote"));
        ret = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto end;
    }

    if (!channel->status.pendingCallbackRequest) {
        BDBG_ERR(("No pending callback request"));
        ret = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto end;
    }

    channel->status.pendingCallbackRequest = false;

    SAGElib_rc = BSAGElib_Rpc_SendCallbackResponse(channel->sagelib_remote,
                                                   pResponse->sequenceId,
                                                   pResponse->returnCode);
    if (SAGElib_rc != BERR_SUCCESS) {
        if (SAGElib_rc == BSAGE_ERR_RESET) {
            ret = BERR_TRACE(NEXUS_ERROR_SAGE_WATCHDOG);
        }
        else {
            ret = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        BDBG_ERR(("BSAGElib_Rpc_SendResponse() failure %d", SAGElib_rc));
        goto end;
    }

end:
    BDBG_LEAVE(NEXUS_SageChannel_SendResponse);
    return ret;
}

NEXUS_Error NEXUS_SageChannel_GetNextIndication(
    NEXUS_SageChannelHandle channel,
    uint32_t *pId,
    uint32_t *pValue)
{
    NEXUS_Error rc = NEXUS_NOT_AVAILABLE;
    NEXUS_SageIndicationContext *pIndicationContext = NULL;

    NEXUS_OBJECT_ASSERT(NEXUS_SageChannel, channel);
    BDBG_ASSERT(pId && pValue);

    /* pop first (oldest) element of the Queue */
    /* those queues are accessed from an ISR, use critical section */
    BKNI_EnterCriticalSection();

    pIndicationContext = BLST_SQ_FIRST(&channel->indications);
    if (pIndicationContext) {
        *pId = pIndicationContext->id;
        *pValue = pIndicationContext->value;
        /* remove it from the channel queue */
        BLST_SQ_REMOVE_HEAD(&channel->indications, link);
        /* place it back inside the cache
           HEAD/TAIL doesn't matter for the cache */
        BLST_SQ_INSERT_HEAD(&g_sage.indicationsCache, pIndicationContext, link);
        g_sage.indicationsCacheCount++;
        rc = NEXUS_SUCCESS;
    }

    BKNI_LeaveCriticalSection();

  /*  BDBG_ERR(("Returning Indication %d %d",*pId, *pValue ));*/
    /* populate context cache if required
       ! do not overwrite rc */
    if (NEXUS_Sage_P_IndicationContextCacheInc() != NEXUS_SUCCESS) {
        BDBG_WRN(("Cannot increase context cache as required (OOM?)"));
    }

    return rc;
}
