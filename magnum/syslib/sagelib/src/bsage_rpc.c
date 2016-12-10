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

#include "bstd.h"
#include "bkni.h"
#include "bhsi.h"
/* #define BSAGELIB_MESSAGE_DEBUG 1 */
#include "bsagelib.h"
#include "bsage_priv.h"
#include "bsagelib_rpc.h"
#include "bsagelib_rai.h"
#include "priv/bsagelib_rpc_shared.h"
#include "priv/bsagelib_shared_types.h"

BDBG_MODULE(BSAGE);

/* local functions */
static BSAGE_RpcRemoteHandle BSAGE_P_Rpc_FindRemoteById_isr(BSAGE_Handle hSAGE, uint32_t id);
static uint32_t BSAGE_P_Rpc_GenSeqId(BSAGE_Handle hSAGE);
static uint32_t BSAGE_P_Rpc_GenInstanceId(BSAGE_Handle hSAGE);
static void BSAGE_P_Rpc_HandleResponse_isr(BSAGE_Handle hSAGE, BSAGElib_RpcResponse *response);
static void BSAGE_P_Rpc_HandleIndication_isr(BSAGE_Handle hSAGE, BSAGElib_RpcIndication *indication);
static void BSAGE_P_Rpc_HandleTATerminate_isr(BSAGE_Handle hSAGE, BSAGElib_RpcTATerminate *taTerminate);
static BERR_Code BSAGE_P_Rpc_HandleCallbackRequest_isr(BSAGE_Handle hSAGE, BSAGElib_RpcRequest *callbackRequest);
static void BSAGE_P_Rpc_TerminateAllByPlatformById_isrsafe(BSAGE_Handle hSAGE, uint32_t platformId);

static BSAGE_RpcRemoteHandle
BSAGE_P_Rpc_FindRemoteById_isr(
    BSAGE_Handle hSAGE,
    uint32_t id)
{
    BSAGE_RpcRemoteHandle remote;

    for (remote = BLST_S_FIRST(&hSAGE->remotes); remote; remote = BLST_S_NEXT(remote, link)) {
        if (id == remote->message->instanceId) {
            BDBG_MSG(("%s hSAGE=%p found remote=%p id=%u",
                      __FUNCTION__, (void *)hSAGE, (void *)remote, id));
            return remote;
        }
    }

    /* This could happen during S3 transitions */
    BDBG_MSG(("%s hSAGE=%p cannot find id=%u", __FUNCTION__, (void *)hSAGE, id));

    return NULL;
}
static void
BSAGE_P_Rpc_TerminateAllByPlatformById_isrsafe(
    BSAGE_Handle hSAGE,
    uint32_t platformId)
{

    BSAGE_RpcRemoteHandle remote;

    for (remote = BLST_S_FIRST(&hSAGE->remotes); remote; remote = BLST_S_NEXT(remote, link)) {
        if (remote->platformId == platformId) {
            remote->terminated = 1;
            remote->valid = 0;
        }
    }
}
static void
BSAGE_P_Rpc_HandleResponse_isr(
    BSAGE_Handle hSAGE,
    BSAGElib_RpcResponse *response)
{
    BSAGE_RpcRemoteHandle remote;
    BSAGElib_RpcMessage *message;

    message = (BSAGElib_RpcMessage *)BSAGE_iOffsetToAddr(response->messageOffset);

    BDBG_MSG(("%s: message=%p (offset=" BDBG_UINT64_FMT ")",
              __FUNCTION__, (void *)message, BDBG_UINT64_ARG(response->messageOffset)));

    if (!message) {
        BDBG_ERR(("%s: cannot convert message offset=" BDBG_UINT64_FMT " to virtual address",
                  __FUNCTION__, BDBG_UINT64_ARG(response->messageOffset)));
        return;
    }

    /* sync memory with cache (invalidate) */
    BSAGE_iInvalidate((void *)message, sizeof(*message));

    remote = BSAGE_P_Rpc_FindRemoteById_isr(hSAGE, message->instanceId);
    if (!remote) {
        BDBG_WRN(("%s: Cannot find remote bounded to id #%u",
                  __FUNCTION__, message->instanceId));
        return;
    }

    if(remote->rpc.response_isr)
    {
        remote->rpc.response_isr(remote->rpc.responseContext, response->rc);
    }
}

static void
BSAGE_P_Rpc_HandleIndication_isr(
    BSAGE_Handle hSAGE,
    BSAGElib_RpcIndication *indication)
{
    BSAGE_RpcRemoteHandle remote;

    remote = BSAGE_P_Rpc_FindRemoteById_isr(hSAGE,
                                               indication->instanceId);
    if (!remote) {
        BDBG_ERR(("%s: Cannot find remote bounded to id #%u",
                  __FUNCTION__, indication->instanceId));
        return;
    }

    BDBG_MSG(("%s: remote [id=%u]: indication #%u, val=%u",
              __FUNCTION__, indication->instanceId, indication->id, indication->value));

    if(remote->rpc.indicationRecv_isr)
    {
        remote->rpc.indicationRecv_isr(remote->rpc.indicationContext, remote->async_arg, indication->id, indication->value);
    }else{
        BDBG_WRN(("%s: remote [id=%u] NO INDICATION CALLBACK (indication #%u, val=%u)",
                  __FUNCTION__, indication->instanceId, indication->id, indication->value));
    }
}

static BERR_Code
BSAGE_P_Rpc_HandleCallbackRequest_isr(
    BSAGE_Handle hSAGE,
    BSAGElib_RpcRequest *callbackRequest)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGE_RpcRemoteHandle remote;
    BSAGElib_RpcMessage *message;

    message = (BSAGElib_RpcMessage *)BSAGE_iOffsetToAddr(callbackRequest->messageOffset);

    if (!message) {
      /*
        BDBG_ERR(("%s: cannot convert message offset=%llu to virtual address",
                  __FUNCTION__, callbackRequest->messageOffset));
      */
        rc = BSAGE_ERR_INTERNAL;
        goto end;
    }

    /* sync memory with cache (invalidate) */
    BSAGE_iInvalidate((void *)message, sizeof(*message));

    BSAGELIB_DUMP_MESSAGE("Rpc CallbackRequest", message, 0);

    remote = BSAGE_P_Rpc_FindRemoteById_isr(hSAGE, message->instanceId);
    if (!remote) {
        BDBG_WRN(("%s: Cannot find remote bounded to id #%u",
                  __FUNCTION__, message->instanceId));
        rc = BSAGE_ERR_INTERNAL;
        goto end;
    }

    /* dispatch callback request */
    {
        BDBG_MSG(("%s: remote=%p [id=%u, sequence=%u]",
                  __FUNCTION__, (void *) remote, message->instanceId, message->sequence));

        if (remote->rpc.callbackRequest_isr) {
            remote->callbacks.message = message;
            rc = remote->rpc.callbackRequest_isr(remote->rpc.callbackRequestContext, remote->async_arg);
            remote->callbacks.sequence = message->sequence;
        }
        else {
            rc = BSAGE_ERR_INTERNAL;
            BDBG_ERR(("%s: drop callbackRequest; not enabled for this client",
                      __FUNCTION__));
            goto end;
        }
    }

end:
    return rc;
}

static void
BSAGE_P_Rpc_HandleTATerminate_isr(
    BSAGE_Handle hSAGE,
    BSAGElib_RpcTATerminate *taTerminate)
{
    BSAGE_RpcRemoteHandle remote;

    remote = BSAGE_P_Rpc_FindRemoteById_isr(hSAGE,
                                               taTerminate->instanceId);
    if (!remote) {
        BDBG_ERR(("%s Cannot find remote of instanceId=%u ; terminated TA on SAGE side [.reason=0x%x, .source=0x%x]",
                  __FUNCTION__, taTerminate->instanceId, taTerminate->reason, taTerminate->source));
        return;
    }
    BSAGE_P_Rpc_TerminateAllByPlatformById_isrsafe(hSAGE, remote->platformId);

    BDBG_ERR(("%s hSAGE=%p remote=%p platformId=%u moduleId=%u instanceId=%u is terminated [.reason=0x%x, .source=0x%x]",
              __FUNCTION__, (void *)hSAGE, (void *)remote,
              remote->platformId, remote->moduleId, taTerminate->instanceId, taTerminate->reason, taTerminate->source));

    if (remote->rpc.taTerminate_isr)
    {
        remote->rpc.taTerminate_isr(remote->rpc.taTerminationContext, remote->async_arg, taTerminate->reason, taTerminate->source);
    }
}


/* Callback fired by lower layer HSI when receiving a message
 * BHSI_IsrCallbackFunc prototype */
static void
BSAGElib_P_Rpc_HSIReceiveCallback_isr(
    void *context)
{
    BSAGElib_ResponseChannel rsCh;
    uint32_t recv_length;
    BERR_Code rc;
    BSAGE_Handle hSAGE = (BSAGE_Handle)context;
    BSAGElib_RpcAck ack;
    const uint8_t *pAckBuf = NULL;
    uint32_t ackBufLen = 0;

    rc = BHSI_Receive_isrsafe(hSAGE->hHsi, (uint8_t *)&rsCh, sizeof(rsCh), &recv_length);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BHSI_Receive failure (%d)",
                  __FUNCTION__, rc));
        return;
    }

    /* TODO Check for reset situation (watchdog) */

    if (recv_length != sizeof(rsCh)) {
        BDBG_ERR(("%s: BHSI_Receive returns receive length %u != %u",
                  __FUNCTION__, recv_length,(unsigned) sizeof(rsCh)));
        return;
    }

    switch (rsCh.type) {
    case BSAGElib_RpcMessage_eResponse:
        BSAGE_P_Rpc_HandleResponse_isr(hSAGE, &rsCh.response);
        break;
    case BSAGElib_RpcMessage_eCallbackRequest:
        pAckBuf = (uint8_t *)&ack;
        ackBufLen = sizeof(ack);
        ack.rc = BSAGE_P_Rpc_HandleCallbackRequest_isr(hSAGE, &rsCh.callbackRequest);
        break;
    case BSAGElib_RpcMessage_eIndication:
        BSAGE_P_Rpc_HandleIndication_isr(hSAGE, &rsCh.indication);
        break;
    case BSAGElib_RpcMessage_eTATerminate:
        BSAGE_P_Rpc_HandleTATerminate_isr(hSAGE, &rsCh.taTerminate);
        break;
    default:
        return;
    }

    /* Send Ack */
    BHSI_Ack_isrsafe(hSAGE->hHsi, pAckBuf, ackBufLen);
}
BERR_Code
BSAGE_P_Rpc_Init(BSAGE_Handle hSAGE,uint8_t *hsi_buffers) /* the buffer is SAGE_HOST_BUF_SIZE*4 bytes long */
{
    BERR_Code rc = BERR_SUCCESS;
    BHSI_Settings HSISettings;

    hSAGE->hsi_buffers = hsi_buffers;

    /* For managing S3 transitions, we restart HSI */
    BSAGE_P_Rpc_Uninit(hSAGE);

    BHSI_GetDefaultSettings(&HSISettings);

    HSISettings.responseCallback_isr = BSAGElib_P_Rpc_HSIReceiveCallback_isr;
    HSISettings.responseCallbackContext = (void *)hSAGE;
    HSISettings.flushCallback_isrsafe = BSAGE_iFlush;
    HSISettings.invalidateCallback_isrsafe = BSAGE_iInvalidate;

    BSAGE_iInvalidate(hSAGE->hsi_buffers, SAGE_HOST_BUF_SIZE*4);

    HSISettings.requestBuf = hSAGE->hsi_buffers;
    HSISettings.requestBufLen = SAGE_HOST_BUF_SIZE;
    HSISettings.requestAckBuf = HSISettings.requestBuf+SAGE_HOST_BUF_SIZE;
    HSISettings.requestAckBufLen = SAGE_HOST_BUF_SIZE;
    HSISettings.responseBuf = HSISettings.requestAckBuf+SAGE_HOST_BUF_SIZE;
    HSISettings.responseBufLen = SAGE_HOST_BUF_SIZE;
    HSISettings.responseAckBuf = HSISettings.responseBuf+SAGE_HOST_BUF_SIZE;
    HSISettings.responseAckBufLen = SAGE_HOST_BUF_SIZE;

    rc = BHSI_Open(&hSAGE->hHsi,
                   hSAGE->hReg,
                   hSAGE->hInt,
                   &HSISettings);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BHSI_Open failure (%d)", __FUNCTION__, rc));
    }

    return rc;
}

BERR_Code
BSAGE_Rpc_Init(uint32_t HSIBufferOffset) /* the buffer is SAGE_HOST_BUF_SIZE*4 bytes long */
{
    uint8_t *hsi_buffers;
    BERR_Code rc = BERR_SUCCESS;
    BSAGE_Handle hSAGE = hSAGE_Global;

    if (!hSAGE) {
        rc = BERR_NOT_INITIALIZED;
        BDBG_ERR(("%s: SAGE is not open yet, hSAGE is NULL", __FUNCTION__));
        goto err;
    }

    if (!HSIBufferOffset) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: HSIBufferOffset is NULL", __FUNCTION__));
        goto err;
    }

    hsi_buffers = BSAGE_iOffsetToAddr(HSIBufferOffset);

    rc = BSAGE_P_Rpc_Init(hSAGE,hsi_buffers);
err:
    return rc;
}

void
BSAGE_P_Rpc_Uninit(BSAGE_Handle hSAGE)
{
    if (hSAGE->hHsi) {
        BHSI_Close(hSAGE->hHsi);
        hSAGE->hHsi = NULL;
    }
/*    if (hSAGE->hsi_buffers) {
        BSAGElib_iFree(hSAGE->hsi_buffers);
        hSAGE->hsi_buffers = NULL;
    }*/
}

void
BSAGE_P_Rpc_Reset(BSAGE_Handle hSAGE)
{
    hSAGE->seqIdGen = 0;
    BSAGE_P_Rpc_Init(hSAGE,hSAGE->hsi_buffers);
}

void
BSAGE_P_Rpc_Reset_isrsafe(BSAGE_Handle hSAGE)
{
    BHSI_Reset_isr(hSAGE->hHsi);
}

static uint32_t
BSAGE_P_Rpc_GenSeqId(BSAGE_Handle hSAGE)
{
    while (!++hSAGE->seqIdGen); /* 0 is invalid */
    return hSAGE->seqIdGen;
}

static uint32_t
BSAGE_P_Rpc_GenInstanceId(BSAGE_Handle hSAGE)
{
    while (!++hSAGE->instanceIdGen); /* 0 is invalid ; could wrap at some point ... */
    return hSAGE->instanceIdGen;
}

BSAGE_RpcRemoteHandle
BSAGE_Rpc_AddRemote(
    uint32_t platformId,
    uint32_t moduleId,
    void *async_argument,
    uint64_t messageOffset)
{
    BSAGE_RpcRemoteHandle remote = NULL;
    BSAGE_Handle hSAGE = hSAGE_Global;

    if (!hSAGE) {
        BDBG_ERR(("%s: SAGE is not open yet, hSAGE is NULL", __FUNCTION__));
        goto end;
    }

    if (!messageOffset) {
        BDBG_ERR(("%s: message buffer for remote is NULL", __FUNCTION__));
        goto end;
    }

    if (hSAGE->resetPending) {
        BDBG_ERR(("%s: cannot add remote until next reset", __FUNCTION__));
        goto end;
    }

    remote = BKNI_Malloc(sizeof(*remote));
    if (!remote) {
        BDBG_ERR(("%s: Cannot allocate remote context", __FUNCTION__));
        goto end;
    }

    BKNI_Memset(remote, 0, sizeof(*remote));
    remote->hSAGE = hSAGE;
    remote->message = BSAGE_iOffsetToAddr(messageOffset);

    BKNI_Memset(remote->message, 0, sizeof(*(remote->message)));

    remote->platformId = remote->message->platformId = platformId;
    remote->moduleId = remote->message->moduleId = moduleId;
    remote->message->instanceId = BSAGE_P_Rpc_GenInstanceId(hSAGE);
    remote->message->version = BSAGELIB_MESSAGE_VERSION;

    remote->async_arg = async_argument;

    remote->valid = 1;
    remote->terminated = 0;

    BDBG_MSG(("%s hSAGE=%p add remote=%p id=%u",
              __FUNCTION__, (void *)hSAGE, (void *)remote, remote->message->instanceId));

    BKNI_EnterCriticalSection();
    BLST_S_INSERT_HEAD(&hSAGE->remotes, remote, link);
    BKNI_LeaveCriticalSection();

end:
    return remote;
}


void
BSAGE_Rpc_RemoveRemote(
    BSAGE_RpcRemoteHandle remote)
{
    BSAGE_Handle hSAGE;

    hSAGE = remote->hSAGE;
    BDBG_MSG(("%s hSAGE=%p remove remote=%p id=%u",
              __FUNCTION__, (void *)hSAGE, (void *)remote, remote->message->instanceId));

    BKNI_EnterCriticalSection();
    BLST_S_REMOVE(&remote->hSAGE->remotes, remote, BSAGE_P_RpcRemote, link);
    BKNI_LeaveCriticalSection();

    BKNI_Free(remote);
}
BERR_Code
BSAGE_Rpc_SendCommand(
    BSAGE_RpcRemoteHandle remote,
    BSAGElib_RpcCommand *command,
    uint32_t *pAsync_id)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_RequestChannel rqCh;
    BSAGElib_RpcAck ack;
    uint32_t ack_len;
    uint32_t seqId;
    BSAGE_Handle hSAGE;
    BDBG_ASSERT(command);

    hSAGE = remote->hSAGE;
retry:
    if (remote->message->sequence) {
        BDBG_ERR(("%s: remote(%p) is busy", __FUNCTION__,remote));
        BKNI_Sleep(10);
        goto retry;
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if(command->systemCommandId == BSAGElib_SystemCommandId_ePlatformEnableCallbacks)
    {
        if (remote->callbacks.container != NULL) {
            rc = BERR_INVALID_PARAMETER;
            BDBG_ERR(("%s: already enabled", __FUNCTION__));
            goto end;
        }
    }


    if (!remote->valid) {
        if (remote->terminated) {
            BDBG_ERR(("%s: remote is invalid (TA terminated)", __FUNCTION__));
            rc = BSAGE_ERR_TA_TERMINATED;
        }
        else {
            BDBG_ERR(("%s: remote is invalid (sage watchdog reset)", __FUNCTION__));
            rc = BSAGE_ERR_RESET;
        }
        goto end;
    }

    if (command->containerVAddr && !command->containerOffset) {
        BDBG_ERR(("%s: container VAddr and offset inconsistency", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if(!hSAGE->bBootPostCalled)
    {
        BDBG_ERR(("%s: cannot send RPC command, SAGE Boot not completed", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    remote->container = command->containerVAddr;

    remote->message->systemCommandId = command->systemCommandId;
    remote->message->moduleCommandId = command->moduleCommandId;
    remote->message->payloadOffset = command->containerOffset;

    BSAGELIB_DUMP_MESSAGE("MESSAGE", remote->message, 0);

    /* lock/unlock sage to ensure consistent message/sequence IDs
     * (no new seq can be generated until that one is consumed) */
    BSAGE_iLockSage;

    seqId = BSAGE_P_Rpc_GenSeqId(hSAGE);

    remote->message->sequence = seqId;
    BTMR_ReadTimer(hSAGE->hTimer, &remote->message->timestamp);

    rqCh.type = BSAGElib_RpcMessage_eRequest;
    rqCh.request.messageOffset = BSAGE_iAddrToOffset(remote->message);
    BSAGE_iFlush(remote->message, sizeof(*remote->message));

    rc = BHSI_Send(hSAGE->hHsi,
                   (uint8_t *)&rqCh,
                   sizeof(rqCh),
                   (uint8_t *)&ack,
                   sizeof(ack),
                   &ack_len);

    BSAGE_iUnlockSage;

    /* if any host-related error occur beyond this, most likely sequence won't be in sync anymore */

    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("BHSI_Send failure (%d)", rc));
        goto end;
    }

    if (ack_len != sizeof(ack)) {
        BDBG_ERR(("ack_len is not the right size (%u != %u)", ack_len, (unsigned)sizeof(ack)));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (ack.rc) {
        BDBG_WRN(("An error occurred on the SAGE side (%u)", ack.rc));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if(command->systemCommandId == BSAGElib_SystemCommandId_ePlatformEnableCallbacks)
    {
        remote->callbacks.message = (BSAGElib_RpcMessage *)remote->container->blocks[0].data.ptr;

        remote->callbacks.container = command->containerVAddr;
    }

    if (pAsync_id) {
        *pAsync_id = seqId;
    }

end:
    if (rc) {
        remote->message->sequence = 0;
    }
    return rc;
}


BERR_Code
BSAGE_Rpc_SendCallbackResponse(
    BSAGE_RpcRemoteHandle remote,
    uint32_t sequenceId,
    BERR_Code retCode)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_RequestChannel rqCh;
    BSAGElib_RpcAck ack;
    uint32_t ack_len;
    BSAGE_Handle hSAGE;

    hSAGE = remote->hSAGE;

    if (!remote->valid) {
        if (remote->terminated) {
            BDBG_ERR(("%s: remote is invalid (TA terminated)", __FUNCTION__));
            rc = BSAGE_ERR_TA_TERMINATED;
        }
        else {
            BDBG_ERR(("%s: remote is invalid (sage watchdog reset)", __FUNCTION__));
            rc = BSAGE_ERR_RESET;
        }
        goto end;
    }

    if (remote->callbacks.sequence == 0 || remote->callbacks.sequence != sequenceId) {
        BDBG_ERR(("%s: current sequenceId %u != %u given sequence Id",
                  __FUNCTION__, remote->callbacks.sequence, sequenceId));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }
    rqCh.callbackResponse.type = BSAGElib_RpcMessage_eCallbackResponse;
    rqCh.callbackResponse.rc = retCode;

    BSAGELIB_DUMP_MESSAGE("Rpc SendCallbackResponse", remote->callbacks.message, 0);
    rqCh.callbackResponse.messageOffset = BSAGE_iAddrToOffset(remote->callbacks.message);
    BSAGE_iFlush(remote->callbacks.message, sizeof(*remote->callbacks.message));

    remote->callbacks.sequence = 0;

    BSAGE_iLockSage;

    rc = BHSI_Send(hSAGE->hHsi,
                   (uint8_t *)&rqCh,
                   sizeof(rqCh),
                   (uint8_t *)&ack,
                   sizeof(ack),
                   &ack_len);

    BSAGE_iUnlockSage;

    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BHSI_Send failure (%d)",
                  __FUNCTION__, rc));
        goto end;
    }

    if (ack_len != sizeof(ack)) {
        BDBG_ERR(("%s: ack_len is not the right size (%u != %lu)",
                  __FUNCTION__, ack_len, (unsigned long)sizeof(ack)));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    if (ack.rc) {
        BDBG_WRN(("%s: An error occurred on the SAGE side (%u)",
                  __FUNCTION__, ack.rc));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

end:
    return rc;
}
