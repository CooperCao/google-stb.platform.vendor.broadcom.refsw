/***************************************************************************
 *     (c)2013 Broadcom Corporation
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

#include "nexus_sage_module.h"

/* magnum basemodules */
#include "berr.h"
#include "bkni.h"

#include "nexus_sage.h"
#include "bsagelib.h"
#include "bsagelib_client.h"
/* #define BSAGELIB_SAGE_MESSAGE_DEBUG 1 */
#include "bsagelib_rpc.h"

BDBG_MODULE(nexus_sage);

/* Internal global context */
static struct NEXUS_Sage_P_State {
    int init;               /* init flag. 0 if module is not yet initialized. */
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


/*
 * Public API
 */

/* semi-private : called from nexus_sage_module */
void NEXUS_Sage_P_Cleanup(void)
{
    if (g_sage.init) {
        g_sage.init = 0;
    }
}

/* semi-private : called from nexus_sage_module */
void NEXUS_Sage_P_Init(void)
{
    BKNI_Memset(&g_sage, 0, sizeof(g_sage));
}

/* BSAGElib_Rpc_ResponseCallback prototypeCallback, see bsagelib.h */
static void NEXUS_Sage_P_ResponseCallback_isr(
    BSAGElib_RpcRemoteHandle handle, void *async_argument, uint32_t async_id, BERR_Code error)
{
    NEXUS_SageChannelHandle channel = (NEXUS_SageChannelHandle)async_argument;

    BSTD_UNUSED(handle);
    BSTD_UNUSED(async_id);

    if (!channel) {
        BDBG_ERR(("%s: retreived channel is NULL", __FUNCTION__));
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

    /* Nexus Sage is also a SAGElib client. */
    {
        BSAGElib_ClientSettings SAGElibClientSettings;
        BERR_Code SAGElib_rc;

        NEXUS_Sage_P_GetSAGElib_ClientSettings(&SAGElibClientSettings);
        SAGElibClientSettings.i_rpc.response_isr = NEXUS_Sage_P_ResponseCallback_isr;
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
                  sizeof(*channel)));
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
    channel->errorCallback = NEXUS_IsrCallback_Create(channel, &callbackSettings);
    if (channel->errorCallback == NULL) {
        BDBG_ERR(("NEXUS_IsrCallback_Create failure for successCallback"));
        (void)BERR_TRACE(NEXUS_OS_ERROR);
        goto end;
    }
    NEXUS_IsrCallback_Set(channel->successCallback, &pSettings->successCallback);
    NEXUS_IsrCallback_Set(channel->errorCallback, &pSettings->errorCallback);

    channel->sage = sage;
    /* Acquire Sage instance will strengthen Channel <- -> Sage dependency:
     * Sage instance cannot be destroyed until any channel remain */
    NEXUS_OBJECT_ACQUIRE(channel, NEXUS_Sage, channel->sage);

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
}

NEXUS_Error NEXUS_Sage_GetLogBuffer(uint8_t *pBuff, uint32_t bufSize,
                                    uint32_t *pBufSize,uint32_t *pWrapBufSize,
                                    uint32_t *pActualBufSize,uint32_t *pActualWrapBufSize)
{
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
}
