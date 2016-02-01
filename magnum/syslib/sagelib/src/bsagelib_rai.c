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
#include "bsagelib_types.h"
#include "bsagelib_shared_types.h"

BDBG_MODULE(BSAGElib);

#include "bsagelib_tools.h"
#include "bsagelib.h"
#include "bsagelib_client.h"
#include "bsagelib_priv.h"
#include "bsagelib_rpc.h"
#include "bsagelib_rai.h"


/* Local functions */
static void BSAGElib_P_Rai_Check_ContainerCache(BSAGElib_ClientHandle hSAGElibClient);
static void BSAGElib_P_Rai_Adjust_ContainerCache(BSAGElib_ClientHandle hSAGElibClient);


/* Used to implement a deferred container cache init
 * Only SAGElib instances using the BSAGElib_Rai API require ContainerCache */
static void
BSAGElib_P_Rai_Check_ContainerCache(
    BSAGElib_ClientHandle hSAGElibClient)
{
    if (!hSAGElibClient->hContainerCache) {
        BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;
        hSAGElibClient->hContainerCache =
            BSAGElib_Tools_ContainerCache_Open(&hSAGElib->i_memory_alloc,
                                               &hSAGElibClient->settings.i_sync_cache);
        BDBG_ASSERT(hSAGElibClient->hContainerCache);
    }
}

/* ContainerCache size = Maximum of (Number of modules, Number of platforms)
 * Update ContainerCache each time a module or a platform is added or removed */
static void
BSAGElib_P_Rai_Adjust_ContainerCache(
    BSAGElib_ClientHandle hSAGElibClient)
{
    uint16_t max = (hSAGElibClient->platformNum > hSAGElibClient->moduleNum) ? hSAGElibClient->platformNum : hSAGElibClient->moduleNum;
    BSAGElib_P_Rai_Check_ContainerCache(hSAGElibClient);
    BSAGElib_Tools_ContainerCache_SetMax(hSAGElibClient->hContainerCache, max);
}

BERR_Code
BSAGElib_Rai_Platform_Open(
    BSAGElib_ClientHandle hSAGElibClient,
    uint32_t platformId,
    BSAGElib_InOutContainer *container /* [in/out] */,
    BSAGElib_RpcRemoteHandle *pPlatform, /* [out] */
    void *async_argument /* [in] */,
    uint32_t *pAsync_id /* [out] */)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_RpcRemoteHandle new_platform;
    BSAGElib_RpcCommand command;
    BSAGElib_Handle hSAGElib;

    BDBG_ENTER(BSAGElib_Rai_Platform_Open);

    BDBG_OBJECT_ASSERT(hSAGElibClient, BSAGElib_P_Client);
    hSAGElib = hSAGElibClient->hSAGElib;

    if (!pPlatform) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: pPlatform is NULL", __FUNCTION__));
        goto end;
    }

    /* Convert and sync virtual <--> physical memory */
    command.containerOffset = BSAGElib_Tools_ContainerAddressToOffset(container,
                                                                      &hSAGElib->i_memory_sync_isrsafe,
                                                                      &hSAGElib->i_memory_map);
    if (!command.containerOffset) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid container %p", __FUNCTION__, container));
        goto end;
    }

    /* Create a communication pipe to SAGE */
    new_platform = BSAGElib_Rpc_AddRemote(hSAGElibClient, platformId, 0, async_argument);
    if (!new_platform) {
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    /* Send a command through associated 'remote' */
    command.containerVAddr = container;
    command.moduleCommandId = 0;
    command.systemCommandId = BSAGElib_SystemCommandId_ePlatformOpen;
    rc = BSAGElib_Rpc_SendCommand(new_platform, &command, pAsync_id);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%u)", __FUNCTION__));
        BSAGElib_Rpc_RemoveRemote(new_platform);
        goto end;
    }

    hSAGElibClient->platformNum++;
    BSAGElib_P_Rai_Adjust_ContainerCache(hSAGElibClient);
    *pPlatform = new_platform;

end:
    BDBG_LEAVE(BSAGElib_Rai_Platform_Open);
    return rc;
}

BERR_Code
BSAGElib_Rai_Platform_Init(
    BSAGElib_RpcRemoteHandle platform,
    BSAGElib_InOutContainer *container, /* [in/out] */
    uint32_t *pAsync_id /* [out] */)
{
    BERR_Code rc;
    BSAGElib_RpcCommand command;
    BSAGElib_Handle hSAGElib;

    BDBG_ENTER(BSAGElib_Rai_Platform_Init);

    BDBG_OBJECT_ASSERT(platform, BSAGElib_P_RpcRemote);
    hSAGElib = platform->hSAGElibClient->hSAGElib;

    /* Convert and sync virtual <--> physical memory */
    command.containerOffset = BSAGElib_Tools_ContainerAddressToOffset(container,
                                                                      &hSAGElib->i_memory_sync_isrsafe,
                                                                      &hSAGElib->i_memory_map);
    command.containerVAddr = container;
    command.moduleCommandId = 0;
    command.systemCommandId = BSAGElib_SystemCommandId_ePlatformInit;
    rc = BSAGElib_Rpc_SendCommand(platform, &command, pAsync_id);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%u)", __FUNCTION__, rc));
        goto end;
    }

end:
    BDBG_LEAVE(BSAGElib_Rai_Platform_Init);
    return rc;
}

void
BSAGElib_Rai_Platform_Close(
    BSAGElib_RpcRemoteHandle platform,
    uint32_t *pAsync_id /* [out] */)
{
    BERR_Code rc;
    BSAGElib_RpcCommand command;

    BDBG_ENTER(BSAGElib_Rai_Platform_Close);

    BDBG_OBJECT_ASSERT(platform, BSAGElib_P_RpcRemote);

    if (platform->valid) {
        command.containerVAddr = NULL;
        command.containerOffset = 0;
        command.moduleCommandId = 0;
        command.systemCommandId = BSAGElib_SystemCommandId_ePlatformClose;
        rc = BSAGElib_Rpc_SendCommand(platform, &command, pAsync_id);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%u)", __FUNCTION__, rc));
            /* keep going */
        }
    }
    else {
        BDBG_WRN(("%s: platform is not valid anymore, skip command send", __FUNCTION__));
    }
    platform->hSAGElibClient->platformNum--;
    BSAGElib_P_Rai_Adjust_ContainerCache(platform->hSAGElibClient);

    BDBG_LEAVE(BSAGElib_Rai_Platform_Close);
}

BERR_Code
BSAGElib_Rai_Module_Init(
    BSAGElib_RpcRemoteHandle platform,
    uint32_t moduleId,
    BSAGElib_InOutContainer *container, /* [in/out] */
    BSAGElib_RpcRemoteHandle *pModule, /* [out] */
    void *async_argument /* [in] */,
    uint32_t *pAsync_id /* [out] */)
{
    BSAGElib_RpcCommand command;
    BERR_Code rc;
    BSAGElib_RpcRemoteHandle new_module;
    BSAGElib_Handle hSAGElib;

    BDBG_ENTER(BSAGElib_Rai_Module_Init);

    BDBG_OBJECT_ASSERT(platform, BSAGElib_P_RpcRemote);
    hSAGElib = platform->hSAGElibClient->hSAGElib;

    if (!pModule) {
        BDBG_ERR(("%s: pModule is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    new_module = BSAGElib_Rpc_AddRemote(platform->hSAGElibClient, platform->platformId, moduleId, async_argument);
    if (!new_module) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: BSAGElib_Rpc_AddRemote failure", __FUNCTION__));
        goto end;
    }

    command.containerOffset = BSAGElib_Tools_ContainerAddressToOffset(container,
                                                                      &hSAGElib->i_memory_sync_isrsafe,
                                                                      &hSAGElib->i_memory_map);
    command.containerVAddr = container;
    command.moduleCommandId = 0;
    command.systemCommandId = BSAGElib_SystemCommandId_eModuleInit;
    rc = BSAGElib_Rpc_SendCommand(new_module, &command, pAsync_id);
    if (rc != BERR_SUCCESS) {
        BSAGElib_Rpc_RemoveRemote(new_module);
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand (%u)", __FUNCTION__, rc));
        goto end;
    }

    platform->hSAGElibClient->moduleNum++;
    BSAGElib_P_Rai_Adjust_ContainerCache(platform->hSAGElibClient);
    *pModule = new_module;

end:
    BDBG_LEAVE(BSAGElib_Rai_Module_Init);
    return rc;
}

void
BSAGElib_Rai_Module_Uninit(
    BSAGElib_RpcRemoteHandle module,
    uint32_t *pAsync_id /* [out] */)
{
    BERR_Code rc;
    BSAGElib_RpcCommand command;

    BDBG_ENTER(BSAGElib_Rai_Module_Uninit);

    BDBG_OBJECT_ASSERT(module, BSAGElib_P_RpcRemote);

    if (module->valid) {
        command.containerOffset = 0;
        command.moduleCommandId = 0;
        command.containerVAddr = NULL;
        command.systemCommandId = BSAGElib_SystemCommandId_eModuleUninit;
        rc = BSAGElib_Rpc_SendCommand(module, &command, pAsync_id);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%u)", __FUNCTION__, rc));
            /* keep going */
        }
    }
    else {
        BDBG_WRN(("%s: module is not valid anymore, skip command send", __FUNCTION__));
    }

    module->hSAGElibClient->moduleNum--;
    BSAGElib_P_Rai_Adjust_ContainerCache(module->hSAGElibClient);

    BDBG_LEAVE(BSAGElib_Rai_Module_Uninit);
}

BERR_Code
BSAGElib_Rai_Module_ProcessCommand(
    BSAGElib_RpcRemoteHandle module,
    uint32_t commandId,
    BSAGElib_InOutContainer *container, /* [in/out] */
    uint32_t *pAsync_id /* [out] */)
{
    BERR_Code rc;
    BSAGElib_RpcCommand command;
    BSAGElib_Handle hSAGElib;

    BDBG_ENTER(BSAGElib_Rai_Module_ProcessCommand);

    BDBG_OBJECT_ASSERT(module, BSAGElib_P_RpcRemote);
    hSAGElib = module->hSAGElibClient->hSAGElib;

    command.containerOffset = BSAGElib_Tools_ContainerAddressToOffset(container,
                                                                      &hSAGElib->i_memory_sync_isrsafe,
                                                                      &hSAGElib->i_memory_map);
    command.containerVAddr = container;
    command.moduleCommandId = commandId;
    command.systemCommandId = BSAGElib_SystemCommandId_eModuleProcessCommand;
    rc = BSAGElib_Rpc_SendCommand(module, &command, pAsync_id);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%u)", __FUNCTION__));
        goto end;
    }

end:
    BDBG_LEAVE(BSAGElib_Rai_Module_ProcessCommand);
    return rc;
}

uint8_t *
BSAGElib_Rai_Memory_Allocate(
    BSAGElib_ClientHandle hSAGElibClient,
    size_t size,
    BSAGElib_Rai_MemoryType memoryType)
{
    BSAGElib_Handle hSAGElib;
    uint8_t *ret;

    BDBG_ENTER(BSAGElib_Rai_Memory_Allocate);

    BDBG_OBJECT_ASSERT(hSAGElibClient, BSAGElib_P_Client);
    hSAGElib = hSAGElibClient->hSAGElib;

    /* size is rounded up to a multiple of 16 bytes for SAGE-side flush concerns. */
    if (size & 0x1F) {
        size = (size | 0x1F) + 1;
    }

    switch (memoryType) {
    case BSAGElib_MemoryType_Restricted:
        ret = BSAGElib_iMallocRestricted(size);
        break;
    case BSAGElib_MemoryType_Global:
        ret = BSAGElib_iMalloc(size);
        break;
    default:
        BDBG_ERR(("%s: bad memory type %u", __FUNCTION__, memoryType));
        ret = NULL;
        break;
    }

    BDBG_LEAVE(BSAGElib_Rai_Memory_Allocate);
    return ret;
}

void
BSAGElib_Rai_Memory_Free(
    BSAGElib_ClientHandle hSAGElibClient,
    uint8_t *pMemory)
{
    BSAGElib_Handle hSAGElib;
    BDBG_ENTER(BSAGElib_Rai_Memory_Free);

    BDBG_OBJECT_ASSERT(hSAGElibClient, BSAGElib_P_Client);
    hSAGElib = hSAGElibClient->hSAGElib;

    BSAGElib_iFree(pMemory);

    BDBG_LEAVE(BSAGElib_Rai_Memory_Free);
}

BSAGElib_InOutContainer *
BSAGElib_Rai_Container_Allocate(
    BSAGElib_ClientHandle hSAGElibClient)
{
    BSAGElib_InOutContainer * container;

    BDBG_ENTER(BSAGElib_Rai_Container_Allocate);

    BDBG_OBJECT_ASSERT(hSAGElibClient, BSAGElib_P_Client);

    BSAGElib_P_Rai_Check_ContainerCache(hSAGElibClient);
    container = BSAGElib_Tools_ContainerCache_Allocate(hSAGElibClient->hContainerCache);

    BDBG_LEAVE(BSAGElib_Rai_Container_Allocate);
    return container;
}

void
BSAGElib_Rai_Container_Free(
    BSAGElib_ClientHandle hSAGElibClient,
    BSAGElib_InOutContainer *container)
{
    BDBG_ENTER(BSAGElib_Rai_Container_Free);

    BDBG_OBJECT_ASSERT(hSAGElibClient, BSAGElib_P_Client);

    BSAGElib_P_Rai_Check_ContainerCache(hSAGElibClient);
    BSAGElib_Tools_ContainerCache_Free(hSAGElibClient->hContainerCache, container);

    BDBG_LEAVE(BSAGElib_Rai_Container_Free);
}
