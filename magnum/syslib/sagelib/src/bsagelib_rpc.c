/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bhsi.h"
#define BSAGELIB_MESSAGE_DEBUG 1
#include "bsagelib.h"
#include "bsagelib_priv.h"
#include "bsagelib_rpc.h"
#include "bsagelib_rpc_shared.h"
#include "bsagelib_rai.h"
#include "bsagelib_shared_types.h"

#define SAGE_HOST_BUF_SIZE 32

BDBG_MODULE(BSAGElib);

BDBG_OBJECT_ID(BSAGElib_P_RpcRemote);

/* local functions */
static BSAGElib_RpcRemoteHandle BSAGElib_P_Rpc_FindRemoteById_isr(BSAGElib_Handle hSAGElib, uint32_t id);
static void BSAGElib_P_Rpc_HSIReceiveCallback_isr(void *context);
static uint32_t BSAGElib_P_Rpc_GenSeqId(BSAGElib_Handle hSAGElib);
static uint32_t BSAGElib_P_Rpc_GenInstanceId(BSAGElib_Handle hSAGElib);
static void BSAGElib_P_Rpc_HandleResponse_isr(BSAGElib_Handle hSAGElib, BSAGElib_RpcResponse *response);
static void BSAGElib_P_Rpc_HandleIndication_isr(BSAGElib_Handle hSAGElib, BSAGElib_RpcIndication *indication);
static void BSAGElib_P_Rpc_ResponseCallbackFree(BSAGElib_ClientHandle hSAGElibClient, BSAGElib_CallbackItem *item);
static BSAGElib_CallbackItem *BSAGElib_P_Rpc_ResponseCallbackAllocate(BSAGElib_ClientHandle hSAGElibClient);


static BSAGElib_RpcRemoteHandle
BSAGElib_P_Rpc_FindRemoteById_isr(
    BSAGElib_Handle hSAGElib,
    uint32_t id)
{
    BSAGElib_ClientHandle hSAGElibClient;

    for (hSAGElibClient = BLST_S_FIRST(&hSAGElib->clients); hSAGElibClient; hSAGElibClient = BLST_S_NEXT(hSAGElibClient, link)) {
        BSAGElib_RpcRemoteHandle remote;

        for (remote = BLST_S_FIRST(&hSAGElibClient->remotes); remote; remote = BLST_S_NEXT(remote, link)) {
            if (id == remote->message->instanceId) {
                BDBG_MSG(("%s hSAGElib=%p hSAGElibClient=%p found remote=%p id=%u",
                          __FUNCTION__, hSAGElib, hSAGElibClient, remote, id));
                return remote;
            }
        }
    }

    /* This could happen during S3 transitions */
    BDBG_MSG(("%s hSAGElib=%p hSAGElibClient=%p cannot find id=%u", __FUNCTION__, hSAGElib, hSAGElibClient, id));

    return NULL;
}

/* This function assumes that given remote is valid. */
void
BSAGElib_P_Rpc_DispatchResponse_isr(
    BSAGElib_RpcRemoteHandle remote,
    BERR_Code response_rc)
{
    BSAGElib_ClientHandle hSAGElibClient = remote->hSAGElibClient;
    BSAGElib_RpcMessage *message = remote->message;

    BDBG_MSG(("%s: remote=%p [id=%u, sequence=%u]: %u",
              __FUNCTION__, remote, message->instanceId, message->sequence, response_rc));

    if (BSAGElib_iRpcResponse_isr) {
        BSAGElib_iRpcResponse_isr(remote, remote->async_arg, message->sequence, response_rc);
        message->sequence = 0;
    }
    else {
        BSAGElib_CallbackItem *callbackItem = remote->callbackItem;
        if (!callbackItem) {
            BDBG_ERR(("%s: error no callback item - drop response", __FUNCTION__));
            goto end;
        }
        remote->callbackItem = NULL;

        callbackItem->remote = remote;
        callbackItem->containerOffset = remote->message->payloadOffset;
        callbackItem->rc = response_rc;
        callbackItem->async_id = message->sequence;
        BLST_SQ_INSERT_TAIL(&hSAGElibClient->responseCallbacks, callbackItem, link);

        BSAGElib_iRpcResponseRecv_isr(remote, remote->async_arg);
    }

end:
    message->payloadOffset = 0;
    return;
}

static void
BSAGElib_P_Rpc_HandleResponse_isr(
    BSAGElib_Handle hSAGElib,
    BSAGElib_RpcResponse *response)
{
    BSAGElib_RpcRemoteHandle remote;
    BSAGElib_RpcMessage *message;

    message = (BSAGElib_RpcMessage *)BSAGElib_iOffsetToAddr(response->messageOffset);

    BDBG_MSG(("%s: message=%p (offset=%llu)",
              __FUNCTION__, message, response->messageOffset));

    if (!message) {
        BDBG_ERR(("%s: cannot convert message offset=%llu to virtual address",
                  __FUNCTION__, response->messageOffset));
        return;
    }

    /* sync memory with cache (invalidate) */
    BSAGElib_iInvalidate_isrsafe((void *)message, sizeof(*message));

    remote = BSAGElib_P_Rpc_FindRemoteById_isr(hSAGElib, message->instanceId);
    if (!remote) {
        BDBG_WRN(("%s: Cannot find remote bounded to id #%u",
                  __FUNCTION__, message->instanceId));
        return;
    }

    BSAGElib_P_Rpc_DispatchResponse_isr(remote, response->rc);
}

static void
BSAGElib_P_Rpc_HandleIndication_isr(
    BSAGElib_Handle hSAGElib,
    BSAGElib_RpcIndication *indication)
{
    BSAGElib_RpcRemoteHandle remote;
    BSAGElib_ClientHandle hSAGElibClient;

    remote = BSAGElib_P_Rpc_FindRemoteById_isr(hSAGElib,
                                               indication->instanceId);
    if (!remote) {
        BDBG_ERR(("%s: Cannot find remote bounded to id #%u",
                  __FUNCTION__, indication->instanceId));
        return;
    }
    hSAGElibClient = remote->hSAGElibClient;

    if (!BSAGElib_iRpcIndicationRecv_isr) {
        BDBG_WRN(("%s: remote [id=%u] NO INDICATION CALLBACK (indication #%u, val=%u)",
                  __FUNCTION__, indication->instanceId, indication->id, indication->value));
        return;
    }

    BDBG_MSG(("%s: remote [id=%u]: indication #%u, val=%u",
              __FUNCTION__, indication->instanceId, indication->id, indication->value));

    BSAGElib_iRpcIndicationRecv_isr(remote, remote->async_arg, indication->id, indication->value);
}

/* Callback fired by lower layer HSI when receiving a message
 * BHSI_IsrCallbackFunc prototype */
static void
BSAGElib_P_Rpc_HSIReceiveCallback_isr(
    void *context)
{
    BSAGElib_Response rs;
    uint32_t recv_length;
    BERR_Code rc;
    BSAGElib_Handle hSAGElib = (BSAGElib_Handle)context;

    rc = BHSI_Receive_isrsafe(hSAGElib->hHsi, (uint8_t *)&rs, sizeof(rs), &recv_length);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BHSI_Receive failure (%d)",
                  __FUNCTION__, rc));
        return;
    }

    /* TODO Check for reset situation (watchdog) */

    if (recv_length != sizeof(rs)) {
        BDBG_ERR(("%s: BHSI_Receive returns receive length %u != %u",
                  __FUNCTION__, recv_length, sizeof(rs)));
        return;
    }

    switch (rs.responseType) {
    case BSAGElib_Response_eResponse:
        BSAGElib_P_Rpc_HandleResponse_isr(hSAGElib, &rs.response);
        break;
    case BSAGElib_Response_eIndication:
        BSAGElib_P_Rpc_HandleIndication_isr(hSAGElib, &rs.indication);
        break;
    default: return;
    }
}

BERR_Code
BSAGElib_P_Rpc_Init(BSAGElib_Handle hSAGElib)
{
    BERR_Code rc = BERR_SUCCESS;
    BHSI_Settings HSISettings;

    /* For managing S3 transitions, we restart HSI */
    BSAGElib_P_Rpc_Uninit(hSAGElib);

    BHSI_GetDefaultSettings(&HSISettings);

    HSISettings.responseCallback_isr = BSAGElib_P_Rpc_HSIReceiveCallback_isr;
    HSISettings.responseCallbackContext = (void *)hSAGElib;
    HSISettings.flushCallback_isrsafe = BSAGElib_iFlush_isrsafe;
    HSISettings.invalidateCallback_isrsafe = BSAGElib_iInvalidate_isrsafe;

    hSAGElib->hsi_buffers = BSAGElib_iMalloc(SAGE_HOST_BUF_SIZE*3);
    BDBG_MSG(("%s: allocating buffers @ 0x%08x --> 0x%08x",
              __FUNCTION__, hSAGElib->hsi_buffers,
              (uint32_t)BSAGElib_iAddrToOffset(hSAGElib->hsi_buffers)));
    BSAGElib_iInvalidate_isrsafe(hSAGElib->hsi_buffers, SAGE_HOST_BUF_SIZE*3);

    HSISettings.requestBuf = hSAGElib->hsi_buffers;
    HSISettings.requestBufLen = SAGE_HOST_BUF_SIZE;
    HSISettings.ackBuf = HSISettings.requestBuf+SAGE_HOST_BUF_SIZE;
    HSISettings.ackBufLen = SAGE_HOST_BUF_SIZE;
    HSISettings.responseBuf = HSISettings.ackBuf+SAGE_HOST_BUF_SIZE;
    HSISettings.responseBufLen = SAGE_HOST_BUF_SIZE;

    rc = BHSI_Open(&hSAGElib->hHsi,
                   hSAGElib->core_handles.hReg,
                   hSAGElib->core_handles.hChp,
                   hSAGElib->core_handles.hInt,
                   &HSISettings);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BHSI_Open failure (%d)", __FUNCTION__, rc));
    }

    return rc;
}

void
BSAGElib_P_Rpc_Uninit(BSAGElib_Handle hSAGElib)
{
    if (hSAGElib->hHsi) {
        BHSI_Close(hSAGElib->hHsi);
        hSAGElib->hHsi = NULL;
    }
    if (hSAGElib->hsi_buffers) {
        BSAGElib_iFree(hSAGElib->hsi_buffers);
        hSAGElib->hsi_buffers = NULL;
    }
}

void
BSAGElib_P_Rpc_Reset(BSAGElib_Handle hSAGElib)
{
    hSAGElib->seqIdGen = 0;
    BSAGElib_P_Rpc_Init(hSAGElib);
}

void
BSAGElib_P_Rpc_Reset_isrsafe(BSAGElib_Handle hSAGElib)
{
    BHSI_Reset_isr(hSAGElib->hHsi);
}

static uint32_t
BSAGElib_P_Rpc_GenSeqId(BSAGElib_Handle hSAGElib)
{
    while (!++hSAGElib->seqIdGen); /* 0 is invalid */
    return hSAGElib->seqIdGen;
}

static uint32_t
BSAGElib_P_Rpc_GenInstanceId(BSAGElib_Handle hSAGElib)
{
    while (!++hSAGElib->instanceIdGen); /* 0 is invalid ; could wrap at some point ... */
    return hSAGElib->instanceIdGen;
}

BSAGElib_RpcRemoteHandle
BSAGElib_Rpc_AddRemote(
    BSAGElib_ClientHandle hSAGElibClient,
    uint32_t platformId,
    uint32_t moduleId,
    void *async_argument)
{
    BSAGElib_RpcRemoteHandle remote = NULL;
    BSAGElib_Handle hSAGElib;

    BDBG_OBJECT_ASSERT(hSAGElibClient, BSAGElib_P_Client);
    hSAGElib = hSAGElibClient->hSAGElib;

    if (hSAGElib->resetPending) {
        BDBG_ERR(("%s: cannot add remote until next reset", __FUNCTION__));
        goto end;
    }

    remote = BKNI_Malloc(sizeof(*remote));
    if (!remote) {
        BDBG_ERR(("%s: Cannot allocate remote context", __FUNCTION__));
        goto end;
    }

    BKNI_Memset(remote, 0, sizeof(*remote));
    remote->hSAGElibClient = hSAGElibClient;
    remote->message = BSAGElib_iMalloc(sizeof(*(remote->message)));
    if (!remote->message) {
        BKNI_Free(remote);
        remote = NULL;
        BDBG_ERR(("%s: Cannot allocate message context using malloc interface",
                  __FUNCTION__));
        goto end;
    }

    BKNI_Memset(remote->message, 0, sizeof(*(remote->message)));

    BDBG_OBJECT_SET(remote, BSAGElib_P_RpcRemote);

    remote->platformId = remote->message->platformId = platformId;
    remote->moduleId = remote->message->moduleId = moduleId;
    remote->message->instanceId = BSAGElib_P_Rpc_GenInstanceId(hSAGElib);
    remote->message->version = BSAGELIB_MESSAGE_VERSION;

    remote->async_arg = async_argument;

    remote->valid = 1;

    BDBG_MSG(("%s hSAGElib=%p hSAGElibClient=%p add remote=%p id=%u",
              __FUNCTION__, hSAGElib, hSAGElibClient, remote, remote->message->instanceId));

    BKNI_EnterCriticalSection();
    BLST_S_INSERT_HEAD(&hSAGElibClient->remotes, remote, link);
    BKNI_LeaveCriticalSection();

end:
    return remote;
}

static BERR_Code BSAGElib_P_Rpc_RemoveRemoteResponses( BSAGElib_ClientHandle hSAGElibClient, BSAGElib_RpcRemoteHandle remote)
{

    BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;
    BSAGElib_CallbackItem *itemNext;

    BDBG_MSG(("%s: remote %p ",__FUNCTION__, remote));

    BKNI_EnterCriticalSection();
    itemNext = BLST_SQ_FIRST(&hSAGElibClient->responseCallbacks);
    BKNI_LeaveCriticalSection();

    while (itemNext) {
        BSAGElib_CallbackItem *item = itemNext; /* set current item */

        BKNI_EnterCriticalSection();
        itemNext = BLST_SQ_NEXT(item, link); /* save next item */

        if (item->remote == remote) {
            BDBG_MSG(("%s: remote %p response removed %x '%s'",__FUNCTION__, item->remote, item->rc, BSAGElib_Tools_ReturnCodeToString(item->rc)));

            BLST_SQ_REMOVE(&hSAGElibClient->responseCallbacks, item, BSAGElib_P_CallbackItem, link);
            BKNI_LeaveCriticalSection();

            if (item->containerOffset) {

                BSAGElib_Tools_ContainerOffsetToAddress(item->containerOffset,
                                                        &hSAGElib->i_memory_sync_isrsafe,
                                                        &hSAGElib->i_memory_map);
            }
            BSAGElib_P_Rpc_ResponseCallbackFree(hSAGElibClient, item);
        }else{
            BKNI_LeaveCriticalSection();
        }
    }

    return BERR_SUCCESS;
}


void
BSAGElib_P_Rpc_RemoveRemote(
    BSAGElib_RpcRemoteHandle remote)
{
    BSAGElib_Handle hSAGElib;
    BERR_Code rc = BERR_SUCCESS;

    hSAGElib = remote->hSAGElibClient->hSAGElib;
    BDBG_MSG(("%s hSAGElib=%p hSAGElibClient=%p remove remote=%p id=%u",
              __FUNCTION__, hSAGElib, remote->hSAGElibClient, remote, remote->message->instanceId));

    if (remote->open) {
        BDBG_ERR(("%s hSAGElib=%p hSAGElibClient=%p remote=%p platformId=%u moduleId=%u instanceId=%u is open",
                  __FUNCTION__, hSAGElib, remote->hSAGElibClient, remote,
                  remote->platformId, remote->moduleId, remote->message->instanceId));

        if (remote->platformId != 0) {
            BSAGElib_Rai_Platform_Close(remote, NULL);
        }
    }
    BKNI_EnterCriticalSection();
    BLST_S_REMOVE(&remote->hSAGElibClient->remotes, remote, BSAGElib_P_RpcRemote, link);
    BKNI_LeaveCriticalSection();

    /* Remove all the pending response messages from the list for the specific client */
    rc = BSAGElib_P_Rpc_RemoveRemoteResponses(remote->hSAGElibClient, remote);
    if(rc == BERR_SUCCESS){
        BDBG_MSG(("%s Removed all Remote Response hSAGElib=%p hSAGElibClient=%p remove remote=%p",
                  __FUNCTION__, hSAGElib, remote->hSAGElibClient, remote));
    }

    if (remote->callbackItem) {
        BSAGElib_P_Rpc_ResponseCallbackFree(remote->hSAGElibClient, remote->callbackItem);
        remote->callbackItem = NULL;
    }
    else {
        BSAGElib_iFree(remote->message);
        remote->message = NULL;
    }

    BDBG_OBJECT_DESTROY(remote, BSAGElib_P_RpcRemote);
    BKNI_Free(remote);
}

void
BSAGElib_Rpc_RemoveRemote(
    BSAGElib_RpcRemoteHandle remote)
{
    BDBG_OBJECT_ASSERT(remote, BSAGElib_P_RpcRemote);

    BSAGElib_P_Rpc_RemoveRemote(remote);
}

BERR_Code
BSAGElib_Rpc_SendCommand(
    BSAGElib_RpcRemoteHandle remote,
    BSAGElib_RpcCommand *command,
    uint32_t *pAsync_id)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_RpcRequest request;
    BSAGElib_RpcAck ack;
    uint32_t ack_len;
    uint32_t seqId;
    BSAGElib_Handle hSAGElib;
    BSAGElib_ClientHandle hSAGElibClient;

    BDBG_OBJECT_ASSERT(remote, BSAGElib_P_RpcRemote);
    BDBG_ASSERT(command);

    hSAGElibClient = remote->hSAGElibClient;
    hSAGElib = hSAGElibClient->hSAGElib;

    if (remote->message->sequence) {
        BDBG_ERR(("%s: remote is busy", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (!remote->valid) {
        BDBG_ERR(("%s: remote is invalid (watchdog reset)", __FUNCTION__));
        rc = BSAGE_ERR_RESET;
        goto end;
    }

    if (command->containerVAddr && !command->containerOffset) {
        BDBG_ERR(("%s: container VAddr and offset inconsistency", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (!BSAGElib_iRpcResponse_isr) {
        remote->callbackItem = BSAGElib_P_Rpc_ResponseCallbackAllocate(remote->hSAGElibClient);
        if (!remote->callbackItem) {
            BDBG_ERR(("%s: cannot allocate callback item", __FUNCTION__));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto end;
        }
    }

    remote->container = command->containerVAddr;

    remote->message->systemCommandId = command->systemCommandId;
    remote->message->moduleCommandId = command->moduleCommandId;
    remote->message->payloadOffset = command->containerOffset;

    /* lock/unlock sage to ensure consistent message/sequence IDs
     * (no new seq can be generated until that one is consumed) */
    BSAGElib_iLockSage;

    seqId = BSAGElib_P_Rpc_GenSeqId(hSAGElib);

    remote->message->sequence = seqId;
    BTMR_ReadTimer(hSAGElib->hTimer, &remote->message->timestamp);

    request.messageOffset = BSAGElib_iAddrToOffset(remote->message);
    BSAGElib_iFlush_isrsafe(remote->message, sizeof(*remote->message));

    rc = BHSI_Send(hSAGElib->hHsi,
                   (uint8_t *)&request,
                   sizeof(request),
                   (uint8_t *)&ack,
                   sizeof(ack),
                   &ack_len);

    BSAGElib_iUnlockSage;

    /* if any host-related error occur beyond this, most likely sequence won't be in sync anymore */

    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("BHSI_Send failure (%d)", rc));
        goto end;
    }

    if (ack_len != sizeof(ack)) {
        BDBG_ERR(("ack_len is not the right size (%u != %u)", ack_len, sizeof(ack)));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (ack.rc) {
        BDBG_WRN(("An error occurred on the SAGE side (%u)", ack.rc));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    switch (command->systemCommandId) {
    case BSAGElib_SystemCommandId_ePlatformOpen:
        remote->open = true;
        break;
    case BSAGElib_SystemCommandId_ePlatformClose:
        remote->open = false;
        break;
    default:
        break;
    }

    if (pAsync_id) {
        *pAsync_id = seqId;
    }

end:
    if (rc) {
        remote->message->sequence = 0;
        if (remote->callbackItem) {
            BSAGElib_P_Rpc_ResponseCallbackFree(hSAGElibClient, remote->callbackItem);
            remote->callbackItem = NULL;
        }
    }
    return rc;
}


static void
BSAGElib_P_Rpc_ResponseCallbackFree(
    BSAGElib_ClientHandle hSAGElibClient,
    BSAGElib_CallbackItem *item)
{
    /* always put back in cache */
    BKNI_EnterCriticalSection();
    BLST_SQ_INSERT_HEAD(&hSAGElibClient->responseCallbackCache, item, link);
    BKNI_LeaveCriticalSection();
}

static BSAGElib_CallbackItem *
BSAGElib_P_Rpc_ResponseCallbackAllocate(
    BSAGElib_ClientHandle hSAGElibClient)
{
    BSAGElib_CallbackItem *item;

    /* allocate from cache first, if empty allocate from system memory */
    BKNI_EnterCriticalSection();
    item = BLST_SQ_FIRST(&hSAGElibClient->responseCallbackCache);
    if (item) {
        BLST_SQ_REMOVE_HEAD(&hSAGElibClient->responseCallbackCache, link);
    }
    BKNI_LeaveCriticalSection();

    if (!item) {
        item = BKNI_Malloc(sizeof(*item));
        if (!item) {
            BDBG_ERR(("%s: cannot allocate memory for Callback Item context", __FUNCTION__));
        }
    }

    return item;
}

static BSAGElib_CallbackItem *
BSAGElib_P_Rpc_PopResponseCallback(
    BSAGElib_ClientHandle hSAGElibClient, int prep_to_fire)
{
    BSAGElib_CallbackItem *item;
    BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;
    BKNI_EnterCriticalSection();
    item = BLST_SQ_FIRST(&hSAGElibClient->responseCallbacks);
    if (item) {
        BLST_SQ_REMOVE_HEAD(&hSAGElibClient->responseCallbacks, link);
    }
    BKNI_LeaveCriticalSection();
    if (item) {
        if (item->rc) {
            BDBG_ERR(("%s: remote %p error %x '%s'",
                      __FUNCTION__, item->remote, item->rc, BSAGElib_Tools_ReturnCodeToString(item->rc)));
        }

        if (prep_to_fire) {
            if (item->containerOffset) {
                BSAGElib_Tools_ContainerOffsetToAddress(item->containerOffset,
                                                        &hSAGElib->i_memory_sync_isrsafe,
                                                        &hSAGElib->i_memory_map);
            }
        }

        /* only from here the remote can be used */
        item->remote->message->sequence = 0;
    }
    return item;
}

BERR_Code BSAGElib_P_Rpc_GetResponse( BSAGElib_ClientHandle hSAGElibClient, struct BSAGElib_ResponseData *data)
{
    BSAGElib_CallbackItem *item;
    item = BSAGElib_P_Rpc_PopResponseCallback(hSAGElibClient, 1);
    if (item) {
        data->remote = item->remote;
        data->async_arg = item->remote->async_arg;
        data->rc = item->rc;
        data->async_id = item->async_id;

        /* we've copied, so free now */
        BSAGElib_P_Rpc_ResponseCallbackFree(hSAGElibClient, item);
        return BERR_SUCCESS;
    }
    else {
        return BERR_NOT_AVAILABLE;
    }
}

BERR_Code
BSAGElib_P_Rpc_ConsumeResponseCallbacks(
    BSAGElib_ClientHandle hSAGElibClient,
    int fire)
{
    BERR_Code rc = BERR_NOT_AVAILABLE; /* return special code if we get nothing, this enables polling */
    BSTD_UNUSED(fire);

    for (;;) {
        BSAGElib_CallbackItem *item;

        item = BSAGElib_P_Rpc_PopResponseCallback(hSAGElibClient, 0);
        if (!item) {
            break;
        }
        rc = BERR_SUCCESS; /* we have at least one */

        BSAGElib_P_Rpc_ResponseCallbackFree(hSAGElibClient, item);
    }

    return rc;
}

void
BSAGElib_P_Rpc_ResponseCallbackCacheFree(
    BSAGElib_ClientHandle hSAGElibClient)
{
    for (;;) {
        BSAGElib_CallbackItem *item = BLST_SQ_FIRST(&hSAGElibClient->responseCallbackCache);
        if (!item) {
            break;
        }
        BLST_SQ_REMOVE_HEAD(&hSAGElibClient->responseCallbackCache, link);
        BKNI_Free(item);
    }
}
