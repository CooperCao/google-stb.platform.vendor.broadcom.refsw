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
#include "bsagelib_priv.h"
#include "bsagelib_rpc.h"
#include "bsagelib_rai.h"
#include "priv/bsagelib_rpc_shared.h"
#include "priv/bsagelib_shared_types.h"

BDBG_MODULE(BSAGElib);

BDBG_OBJECT_ID(BSAGElib_P_RpcRemote);

/* local functions */
static void BSAGElib_P_Rpc_ResponseCallbackFree(BSAGElib_ClientHandle hSAGElibClient, BSAGElib_CallbackItem *item);
static BSAGElib_CallbackItem *BSAGElib_P_Rpc_ResponseCallbackAllocate(BSAGElib_ClientHandle hSAGElibClient);

/* This function assumes that given remote is valid. */
void
BSAGElib_P_Rpc_DispatchResponse_isr(
    BSAGElib_RpcRemoteHandle remote,
    BERR_Code response_rc)
{
    BSAGElib_ClientHandle hSAGElibClient = remote->hSAGElibClient;
    BSAGElib_RpcMessage *message = remote->message;

    BDBG_MSG(("%s: remote=%p [id=%u, sequence=%u]: %u",
              __FUNCTION__, (void *)remote, message->instanceId, message->sequence, response_rc));

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

    remote->platformId = platformId;
    remote->moduleId = moduleId;

    remote->async_arg = async_argument;

    remote->hRemote = hSAGElib->bsage.Rpc_AddRemote(platformId,moduleId,async_argument,BSAGElib_iAddrToOffset(remote->message));
/*    remote->hRemote = BSAGE_Rpc_AddRemote(platformId,moduleId,async_argument,BSAGElib_iAddrToOffset(remote->message));*/
    if (!remote->hRemote) {
        BKNI_Free(remote);
        remote = NULL;
        BDBG_ERR(("%s: Cannot Add Sage Remote",
                  __FUNCTION__));
        goto end;
    }

    hSAGElib->bsage.RegisterCallback(BSAGE_Event_msgIndication,     hSAGElibClient->settings.i_rpc.indicationRecv_isr, (void *)remote,remote->hRemote);
    hSAGElib->bsage.RegisterCallback(BSAGE_Event_msgResponse,       BSAGElib_P_Rpc_DispatchResponse_isr,               (void *)remote,remote->hRemote);
    hSAGElib->bsage.RegisterCallback(BSAGE_Event_msgCallbackRequest,hSAGElibClient->settings.i_rpc.callbackRequest_isr,(void *)remote,remote->hRemote);
    hSAGElib->bsage.RegisterCallback(BSAGE_Event_msgTATermination,  hSAGElibClient->settings.i_rpc.taTerminate_isr,    (void *)remote,remote->hRemote);
/*
    BSAGE_RegisterCallback(BSAGE_Event_msgIndication,     hSAGElibClient->settings.i_rpc.indicationRecv_isr, (void *)remote,remote->hRemote);
    BSAGE_RegisterCallback(BSAGE_Event_msgResponse,       BSAGElib_P_Rpc_DispatchResponse_isr,               (void *)remote,remote->hRemote);
    BSAGE_RegisterCallback(BSAGE_Event_msgCallbackRequest,hSAGElibClient->settings.i_rpc.callbackRequest_isr,(void *)remote,remote->hRemote);
    BSAGE_RegisterCallback(BSAGE_Event_msgTATermination,  hSAGElibClient->settings.i_rpc.taTerminate_isr,    (void *)remote,remote->hRemote);
*/
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

    BDBG_MSG(("%s: remote %p ",__FUNCTION__, (void *)remote));

    BKNI_EnterCriticalSection();
    itemNext = BLST_SQ_FIRST(&hSAGElibClient->responseCallbacks);
    BKNI_LeaveCriticalSection();

    while (itemNext) {
        BSAGElib_CallbackItem *item = itemNext; /* set current item */

        BKNI_EnterCriticalSection();
        itemNext = BLST_SQ_NEXT(item, link); /* save next item */

        if (item->remote == remote) {
            BDBG_MSG(("%s: remote %p response removed %x '%s'",__FUNCTION__, (void *)item->remote, item->rc, BSAGElib_Tools_ReturnCodeToString(item->rc)));

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
              __FUNCTION__, (void *)hSAGElib, (void *)remote->hSAGElibClient, (void *)remote, remote->message->instanceId));

    if (remote->open) {
        /* Special case for system crit platform and S3 */
        /* When going to S3, BSAGElib_P_Rpc_RemoveRemote will be intentionally called without
        * closing the platform in order to free resources, but leave the platform running (as
        * it is needed for S3) */
        if(remote->platformId != BSAGE_PLATFORM_ID_SYSTEM_CRIT)
        {
            BDBG_ERR(("%s hSAGElib=%p hSAGElibClient=%p remote=%p platformId=%u moduleId=%u instanceId=%u is open",
                      __FUNCTION__, (void *)hSAGElib, (void *)remote->hSAGElibClient, (void *)remote,
                      remote->platformId, remote->moduleId, remote->message->instanceId));

            if (remote->moduleId != 0) {
                BSAGElib_Rai_Module_Uninit(remote, NULL);
            }
            else if (remote->platformId != 0) {
                BSAGElib_Rai_Platform_Close(remote, NULL);
            }
        }
    }
    BKNI_EnterCriticalSection();
    BLST_S_REMOVE(&remote->hSAGElibClient->remotes, remote, BSAGElib_P_RpcRemote, link);
    BKNI_LeaveCriticalSection();

    /* Remove all the pending response messages from the list for the specific client */
    rc = BSAGElib_P_Rpc_RemoveRemoteResponses(remote->hSAGElibClient, remote);
    if(rc == BERR_SUCCESS){
        BDBG_MSG(("%s Removed all Remote Response hSAGElib=%p hSAGElibClient=%p remove remote=%p",
                  __FUNCTION__, (void *)hSAGElib, (void *)remote->hSAGElibClient, (void *)remote));
    }

    if (remote->callbackItem) {
        BSAGElib_P_Rpc_ResponseCallbackFree(remote->hSAGElibClient, remote->callbackItem);
        remote->callbackItem = NULL;
    }
    else {
        BSAGElib_iFree(remote->message);
        remote->message = NULL;
    }

    hSAGElib->bsage.Rpc_RemoveRemote(remote->hRemote);
/*    BSAGE_Rpc_RemoveRemote(remote->hRemote);*/

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
    uint32_t seqId;
    BSAGElib_Handle hSAGElib;
    BSAGElib_ClientHandle hSAGElibClient;
    BDBG_OBJECT_ASSERT(remote, BSAGElib_P_RpcRemote);
    BDBG_ASSERT(command);

    hSAGElibClient = remote->hSAGElibClient;
    hSAGElib = hSAGElibClient->hSAGElib;

    if (!BSAGElib_iRpcResponse_isr) {
        remote->callbackItem = BSAGElib_P_Rpc_ResponseCallbackAllocate(remote->hSAGElibClient);
        if (!remote->callbackItem) {
            BDBG_ERR(("%s: cannot allocate callback item", __FUNCTION__));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto end;
        }
    }

    rc = hSAGElib->bsage.Rpc_SendCommand(remote->hRemote,command,&seqId);
/*    rc = BSAGE_Rpc_SendCommand(remote->hRemote,command,&seqId);*/
    /* if any host-related error occur beyond this, most likely sequence won't be in sync anymore */

    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("BHSI_Send failure (%d)", rc));
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
        if (remote->callbackItem) {
            BSAGElib_P_Rpc_ResponseCallbackFree(hSAGElibClient, remote->callbackItem);
            remote->callbackItem = NULL;
        }
    }
    return rc;
}

BERR_Code
BSAGElib_Rpc_SendCallbackResponse(
    BSAGElib_RpcRemoteHandle remote,
    uint32_t sequenceId,
    BERR_Code retCode)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_Handle hSAGElib;
    BSAGElib_ClientHandle hSAGElibClient;

    BDBG_OBJECT_ASSERT(remote, BSAGElib_P_RpcRemote);

    hSAGElibClient = remote->hSAGElibClient;
    hSAGElib = hSAGElibClient->hSAGElib;

    rc = hSAGElib->bsage.Rpc_SendCallbackResponse(remote->hRemote,sequenceId,retCode);
/*    rc = BSAGE_Rpc_SendCallbackResponse(remote->hRemote,sequenceId,retCode);*/
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BHSI_Send failure (%d)",
                  __FUNCTION__, rc));
        goto end;
    }

end:
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
            /* If the command was openPlatform, and return was platform unknown,
            * change the open flag to false to allow for proper cleanup */
            if((item->rc==BSAGE_ERR_PLATFORM_ID)&&item->remote&&item->remote->message
                &&(item->remote->message->systemCommandId==BSAGElib_SystemCommandId_ePlatformOpen))
            {
                item->remote->open = false;
                BDBG_WRN(("%s: remote %p warning %x '%s'",
                      __FUNCTION__, (void *)item->remote, item->rc, BSAGElib_Tools_ReturnCodeToString(item->rc)));
            }
            else
            {
                BDBG_ERR(("%s: remote %p error %x '%s'",
                      __FUNCTION__, (void *)item->remote, item->rc, BSAGElib_Tools_ReturnCodeToString(item->rc)));
            }
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
