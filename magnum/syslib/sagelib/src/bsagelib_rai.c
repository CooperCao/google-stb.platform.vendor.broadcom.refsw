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
#include "bsagelib_types.h"
#include "priv/bsagelib_shared_types.h"

BDBG_MODULE(BSAGElib);

#include "bsagelib_tools.h"
#include "bsagelib.h"
#include "bsagelib_client.h"
#include "bsagelib_priv.h"
#include "bsagelib_rpc.h"
#include "bsagelib_rai.h"

#include "bsagelib_sdl_header.h"

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

#if SAGE_VERSION < SAGE_VERSION_CALC(3,0)
BERR_Code
BSAGElib_Rai_Platform_Install(BSAGElib_ClientHandle hSAGElibClient,
                                        uint32_t platformId,
                                        uint8_t *binBuff,
                                        uint32_t binSize)
{
	BSTD_UNUSED(hSAGElibClient);
    BSTD_UNUSED(platformId);
	BSTD_UNUSED(binBuff);
    BSTD_UNUSED(binSize);

    BDBG_WRN(("This API is valid only for SAGE 3x and later binaries"));

	return BERR_SUCCESS;
}

BERR_Code
BSAGElib_Rai_Platform_UnInstall(BSAGElib_ClientHandle hSAGElibClient,
                                        uint32_t platformId)
{
	BSTD_UNUSED(hSAGElibClient);
	BSTD_UNUSED(platformId);

	BDBG_WRN(("This API is valid only for SAGE 3x and later binaries"));

	return BERR_SUCCESS;
}

BERR_Code
BSAGElib_Rai_Platform_EnableCallbacks(
    BSAGElib_RpcRemoteHandle platform,
    uint32_t *pAsync_id /* [out] */)
{
	BSTD_UNUSED(platform);
    BSTD_UNUSED(pAsync_id);

	BDBG_WRN(("This API is valid only for SAGE 3x and later binaries"));

	return BERR_SUCCESS;
}

#else /* SAGE_3X and later */
static BERR_Code
BSAGELib_Rai_P_WaitForResponse(BSAGElib_ClientHandle hSAGElibClient, uint32_t async_id)
{
    BERR_Code rc = BERR_NOT_AVAILABLE;
    BSAGElib_ResponseData data;

    while(rc == BERR_NOT_AVAILABLE){
        BKNI_Sleep(1);
        rc = BSAGElib_Client_GetResponse(hSAGElibClient, &data);
    }
    if(async_id != data.async_id){
        return BERR_UNKNOWN;
    }

    return rc;

}

static bool
_P_IsSdlValid(
    BSAGElib_Handle hSAGElib,
    BSAGElib_SDLHeader *pHeader)
{
    bool rc = false;
    uint32_t sdlTHLSigShort;

    if ((pHeader->ucSsfVersion[0] | pHeader->ucSsfVersion[1] |
         pHeader->ucSsfVersion[2] | pHeader->ucSsfVersion[3]) != 0) {
      rc = (BKNI_Memcmp(pHeader->ucSsfVersion, hSAGElib->frameworkInfo.version, 4) == 0);
      if (rc != true) {
        BDBG_ERR(("%s: The SDL is compiled using SAGE version %u.%u.%u.%u SDK",
                  __FUNCTION__, pHeader->ucSsfVersion[0],
                  pHeader->ucSsfVersion[1],
                  pHeader->ucSsfVersion[2],
                  pHeader->ucSsfVersion[3]));
        BDBG_ERR(("%s: The SDL is not compiled from the same SDK as the running Framework",
                  __FUNCTION__));
        BDBG_ERR(("%s: The SDL must be recompiled using SAGE version %u.%u.%u.%u SDK",
                  __FUNCTION__, hSAGElib->frameworkInfo.version[0],
                  hSAGElib->frameworkInfo.version[1],
                  hSAGElib->frameworkInfo.version[2],
                  hSAGElib->frameworkInfo.version[3]));
        goto end;
      }
    }
    else {
      BDBG_MSG(("%s: SSF version not found in SDL image, skipping SSF version check", __FUNCTION__));
    }

    sdlTHLSigShort = pHeader->ucSsfThlShortSig[0] | (pHeader->ucSsfThlShortSig[1] << 8) |
                     (pHeader->ucSsfThlShortSig[2] << 16) | (pHeader->ucSsfThlShortSig[3] << 24);

    rc = (sdlTHLSigShort == hSAGElib->frameworkInfo.THLShortSig);
    if (rc != true) {
        BDBG_ERR(("%s: The SDL THL Signature Short (0x%08x) differs from the one inside the loaded SAGE Framework (0x%08x)",
                  __FUNCTION__, sdlTHLSigShort, hSAGElib->frameworkInfo.THLShortSig));
        BDBG_ERR(("%s: The SDL must be linked against the same THL (Thin Layer) as the one inside the SAGE Framework",
                  __FUNCTION__));
        goto end;
    }

end:
    return rc;
}

#define CASE_PLATFORM_NAME_TO_STRING(name) case BSAGE_PLATFORM_ID_##name:  platform_name = #name; break

static char *
_P_LookupPlatformName(uint32_t platformId)
{

  char *platform_name = NULL;
  switch (platformId) {
    /* NOTE: system and system_crit platforms are not SDL TAs */
    CASE_PLATFORM_NAME_TO_STRING(COMMONDRM);
    CASE_PLATFORM_NAME_TO_STRING(HDCP22);
    CASE_PLATFORM_NAME_TO_STRING(MANUFACTURING);
    CASE_PLATFORM_NAME_TO_STRING(UTILITY);
    CASE_PLATFORM_NAME_TO_STRING(PLAYREADY_30);
    CASE_PLATFORM_NAME_TO_STRING(ANTIROLLBACK);
    CASE_PLATFORM_NAME_TO_STRING(SECURE_VIDEO);
    CASE_PLATFORM_NAME_TO_STRING(SECURE_LOGGING);
    CASE_PLATFORM_NAME_TO_STRING(ADOBE_DRM);
    CASE_PLATFORM_NAME_TO_STRING(PLAYREADY_25);
    CASE_PLATFORM_NAME_TO_STRING(NETFLIX_ANNEXB);
    CASE_PLATFORM_NAME_TO_STRING(NETFLIX);
    CASE_PLATFORM_NAME_TO_STRING(WIDEVINE);
    CASE_PLATFORM_NAME_TO_STRING(PLAYBACK);
    CASE_PLATFORM_NAME_TO_STRING(DTCP_IP);
    CASE_PLATFORM_NAME_TO_STRING(MIRACAST);
    CASE_PLATFORM_NAME_TO_STRING(MARLIN);
    CASE_PLATFORM_NAME_TO_STRING(EDRM);
    default:
      break;
  }
  return platform_name;
}

BERR_Code
BSAGElib_Rai_Platform_Install(
    BSAGElib_ClientHandle hSAGElibClient,
    uint32_t platformId,
    uint8_t *binBuff,
    uint32_t binSize)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer * container = NULL;
    uint32_t async_id;
    BSAGElib_SDLHeader *pHeader = NULL;
    char *platform_name = _P_LookupPlatformName(platformId);

    if (binSize < sizeof(BSAGElib_SDLHeader)) {
        BDBG_ERR(("%s: The binary size for TA 0x%X is less than the SDL header structure",
                  __FUNCTION__, platformId));
        goto err;
    }
    pHeader = (BSAGElib_SDLHeader *)binBuff;

    container = BSAGElib_Rai_Container_Allocate(hSAGElibClient);
    if(container == NULL)
        goto err;

    if (platform_name == NULL) {
        BDBG_LOG(("SAGE TA: [0x%X] [Version=%u.%u.%u.%u, Signing Tool=%u.%u.%u.%u]",
                  platformId,
                  pHeader->ucSdlVersion[0],pHeader->ucSdlVersion[1],
                  pHeader->ucSdlVersion[2],pHeader->ucSdlVersion[3],
                  pHeader->ucSageSecureBootToolVersion[0],pHeader->ucSageSecureBootToolVersion[1],
                  pHeader->ucSageSecureBootToolVersion[2],pHeader->ucSageSecureBootToolVersion[3]
                  ));
    }
    else {
        BDBG_LOG(("SAGE TA: %s [Version=%u.%u.%u.%u, Signing Tool=%u.%u.%u.%u]",
                  platform_name,
                  pHeader->ucSdlVersion[0],pHeader->ucSdlVersion[1],
                  pHeader->ucSdlVersion[2],pHeader->ucSdlVersion[3],
                  pHeader->ucSageSecureBootToolVersion[0],pHeader->ucSageSecureBootToolVersion[1],
                  pHeader->ucSageSecureBootToolVersion[2],pHeader->ucSageSecureBootToolVersion[3]
                  ));
    }
    BDBG_MSG(("%s: Platform %x %p %d",__FUNCTION__, platformId, binBuff,binSize  ));
    BDBG_MSG(("%s: System Platform %p %p",__FUNCTION__, (void *)hSAGElibClient->system_platform, (void*)hSAGElibClient->system_module ));

    if(hSAGElibClient->system_platform == NULL){

        /* Open the platform first */

        rc = BSAGElib_Rai_Platform_Open(hSAGElibClient,
                                        BSAGE_PLATFORM_ID_SYSTEM,
                                        container,
                                        &(hSAGElibClient->system_platform),
                                        (void *)hSAGElibClient,
                                        &async_id);
        if (rc != BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
            goto err;
        }

        rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);

        if (rc != BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
            goto err;
        }
        BDBG_MSG(("%s: System Platform %p ",__FUNCTION__, (void *)hSAGElibClient->system_platform ));

        if(container->basicOut[0] != BSAGElib_State_eInit){

            /* Not yet initialized: send init command*/
            rc = BSAGElib_Rai_Platform_Init(hSAGElibClient->system_platform,
                                            container,
                                            &async_id);

            if (rc != BERR_SUCCESS) {
                rc = BERR_TRACE(rc);
                goto err;
            }

            rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);

            if (rc != BERR_SUCCESS) {
                rc = BERR_TRACE(rc);
                goto err;
            }
        }

        if (hSAGElibClient->system_module == NULL) {
            rc = BSAGElib_Rai_Module_Init(hSAGElibClient->system_platform,
                                        System_ModuleId_eDynamicLoad,
                                        container,
                                        &(hSAGElibClient->system_module),
                                        (void *)hSAGElibClient,
                                        &async_id);

            if (rc != BERR_SUCCESS) {
                rc = BERR_TRACE(rc);
                goto end;
            }

            rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);

            if (rc != BERR_SUCCESS) {
                goto err;
            }
            BDBG_MSG(("%s: System Platform Module %p ",__FUNCTION__, (void *)hSAGElibClient->system_module ));

        }
    }

    /* do it after the system platform open so we are sure the SAGE-side is booted
       and the status info are valid */
    if (!_P_IsSdlValid(hSAGElibClient->hSAGElib, pHeader)) {
        BDBG_ERR(("%s: Cannot install incompatible SDL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto err;
    }

    container->basicIn[0] = platformId;

    /* provide the SDL binary that's been pulled from FS */
    container->blocks[0].data.ptr = binBuff;
    container->blocks[0].len = binSize;

    rc = BSAGElib_Rai_Module_ProcessCommand(hSAGElibClient->system_module,
                                            DynamicLoadModule_CommandId_eLoadSDL,
                                            container,
                                            &async_id);

    if (rc != BERR_SUCCESS) {
        /* If message to Load failed then SAGE might be reset so clean up System Platform Handles */
        BDBG_ERR(("%s BSAGElib_Rai_Module_ProcessCommand failure %d",__FUNCTION__,rc));
        hSAGElibClient->system_platform = NULL;
        hSAGElibClient->system_module = NULL;
        rc = BERR_TRACE(rc);
        goto err;
    }

    rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);

    if (rc != BERR_SUCCESS) {
        rc = BERR_TRACE(rc);
        goto err;
    }
    BDBG_MSG(("%s Output Status %x",__FUNCTION__,container->basicOut[0]));

    if((rc == BERR_SUCCESS) || (container->basicOut[0] == BSAGE_ERR_SDL_ALREADY_LOADED)){
        rc = BERR_SUCCESS;
        goto end;
    }


err:
end:

    if (container) {
        BSAGElib_Rai_Container_Free(hSAGElibClient, container);
    }
    return rc;
}

BERR_Code
BSAGElib_Rai_Platform_UnInstall(BSAGElib_ClientHandle hSAGElibClient,
                                        uint32_t platformId)
{

    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer * container = NULL;
    uint32_t async_id;


    if((hSAGElibClient->system_module == NULL) || (hSAGElibClient->system_platform == NULL)){
        rc = BERR_INVALID_PARAMETER;
        goto err;
    }

    container = BSAGElib_Rai_Container_Allocate(hSAGElibClient);
    if(container == NULL)
        goto err;

    container->basicIn[0] = platformId;

    BDBG_MSG(("%s: System Platform %p %p",__FUNCTION__, (void *)hSAGElibClient->system_platform, (void*)hSAGElibClient->system_module ));

    rc = BSAGElib_Rai_Module_ProcessCommand(hSAGElibClient->system_module,
                                            DynamicLoadModule_CommandId_eUnLoadSDL,
                                            container,
                                            &async_id);

    if (rc != BERR_SUCCESS) {
        /* If message to Unload failed then SAGE might be reset so clean up System Platform Handles */
        BDBG_ERR(("%s BSAGElib_Rai_Module_ProcessCommand failure %d",__FUNCTION__,rc));
        hSAGElibClient->system_platform = NULL;
        hSAGElibClient->system_module = NULL;
        goto err;
    }

    rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);

    if (rc != BERR_SUCCESS) {
        goto err;
    }

    BDBG_MSG(("%s Output Status %d",__FUNCTION__,container->basicOut[0]));

    if (hSAGElibClient->system_module != NULL) {
        BSAGElib_Rai_Module_Uninit(hSAGElibClient->system_module,&async_id);
        rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);
        if (rc != BERR_SUCCESS) {
            goto err;
        }

        BSAGElib_Rpc_RemoveRemote(hSAGElibClient->system_module);
        hSAGElibClient->system_module = NULL;
    }

    if (hSAGElibClient->system_platform != NULL) {
        BSAGElib_Rai_Platform_Close(hSAGElibClient->system_platform, &async_id);
        rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);
        if (rc != BERR_SUCCESS) {
            goto err;
        }
        BSAGElib_Rpc_RemoveRemote(hSAGElibClient->system_platform);
        hSAGElibClient->system_platform = NULL;
    }
    goto end;

err:
end:
    if (container) {
        BSAGElib_Rai_Container_Free(hSAGElibClient, container);
    }
    return rc;


}

BERR_Code
BSAGElib_Rai_Platform_EnableCallbacks(
    BSAGElib_RpcRemoteHandle platform,
    uint32_t *pAsync_id /* [out] */)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_RpcCommand command;
    BSAGElib_InOutContainer *container = NULL;
    BSAGElib_RpcMessage *message = NULL;
    BSAGElib_Handle hSAGElib;
    BSAGElib_ClientHandle hSAGElibClient;

    BDBG_ENTER(BSAGElib_Rai_Platform_EnableCallbacks);

    BDBG_OBJECT_ASSERT(platform, BSAGElib_P_RpcRemote);
#ifndef SAGE_KO
    if (platform->callbacks.container != NULL) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: already enabled", __FUNCTION__));
        goto end;
    }
#endif
    hSAGElibClient = platform->hSAGElibClient;
    hSAGElib = hSAGElibClient->hSAGElib;

    if (BSAGElib_iRpcCallbackRequest_isr == NULL) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: client does not support callback requests", __FUNCTION__));
        goto end;
    }

    container = BSAGElib_Rai_Container_Allocate(platform->hSAGElibClient);
    if (container == NULL) {
        BDBG_ERR(("%s: BSAGElib_Rai_Container_Allocate() failure (%u)", __FUNCTION__, rc));
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto end;
    }
    message = (BSAGElib_RpcMessage *)BSAGElib_Rai_Memory_Allocate(platform->hSAGElibClient,
                                                                  sizeof(*message),
                                                                  BSAGElib_MemoryType_Global);
    if (message == NULL) {
        BDBG_ERR(("%s: BSAGElib_Rai_Memory_Allocate() failure to allocate message (%u)", __FUNCTION__, rc));
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto end;
    }
    container->blocks[0].data.ptr = (uint8_t *)message;
    container->blocks[0].len = sizeof(*message);
    command.containerOffset = BSAGElib_Tools_ContainerAddressToOffset(container,
                                                                      &hSAGElib->i_memory_sync_isrsafe,
                                                                      &hSAGElib->i_memory_map);
    command.containerVAddr = container;
    command.moduleCommandId = 0;
    command.systemCommandId = BSAGElib_SystemCommandId_ePlatformEnableCallbacks;
    rc = BSAGElib_Rpc_SendCommand(platform, &command, pAsync_id);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand (%u)", __FUNCTION__, rc));
        goto end;
    }
#ifndef SAGE_KO
    platform->callbacks.message = message;
#endif
    message = NULL;

#ifndef SAGE_KO
    platform->callbacks.container = container;
#endif
    container = NULL;

end:
    if (container) {
        BSAGElib_Rai_Container_Free(platform->hSAGElibClient, container);
    }
    if (message) {
        BSAGElib_Rai_Memory_Free(platform->hSAGElibClient, (uint8_t *)message);
    }
    BDBG_LEAVE(BSAGElib_Rai_Platform_EnableCallbacks);
    return rc;
}


#endif
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

    BDBG_MSG(("%s: Platform %x ",__FUNCTION__, platformId));

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
        BDBG_ERR(("%s: invalid container %p", __FUNCTION__, (void *)container));
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
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%d)", __FUNCTION__, (int)rc));
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

    BDBG_MSG(("%s Platform id %x Module id %x",__FUNCTION__,platform->platformId,platform->moduleId));

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

    BDBG_MSG(("%s Platform id %x Module id %x",__FUNCTION__,platform->platformId,platform->moduleId));

#ifndef SAGE_KO
    if (platform->valid) {
#endif
        command.containerVAddr = NULL;
        command.containerOffset = 0;
        command.moduleCommandId = 0;
        command.systemCommandId = BSAGElib_SystemCommandId_ePlatformClose;
        rc = BSAGElib_Rpc_SendCommand(platform, &command, pAsync_id);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%u)", __FUNCTION__, rc));
            /* keep going */
        }
#ifndef SAGE_KO
    }
    else {
        BDBG_WRN(("%s: platform is not valid anymore, skip command send", __FUNCTION__));
    }
#endif
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

#ifndef SAGE_KO
    if (module->valid) {
#endif
        command.containerOffset = 0;
        command.moduleCommandId = 0;
        command.containerVAddr = NULL;
        command.systemCommandId = BSAGElib_SystemCommandId_eModuleUninit;
        rc = BSAGElib_Rpc_SendCommand(module, &command, pAsync_id);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%u)", __FUNCTION__, rc));
            /* keep going */
        }
#ifndef SAGE_KO
    }
    else {
        BDBG_WRN(("%s: module is not valid anymore, skip command send", __FUNCTION__));
    }
#endif
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
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%d)", __FUNCTION__, (int)rc));
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
