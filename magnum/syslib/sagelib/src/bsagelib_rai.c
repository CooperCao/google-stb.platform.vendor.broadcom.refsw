/******************************************************************************
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
#include "priv/suif.h"
#include "priv/suif_sdl.h"

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
    if(rc != BERR_SUCCESS)
    {
        return rc;
    }
    if(async_id != data.async_id){
        return BERR_UNKNOWN;
    }

    return data.rc;
}

static bool
_P_IsSdlValid(
    BSAGElib_Handle hSAGElib,
    BSAGElib_SDLHeader *pHeader)
{
    bool rc = false;
    uint32_t sdlTHLSigShort;
    SUIF_CommonHeader *pSUIFHeader = NULL;
    uint32_t magicNumber,*pMagicNumber_BE;

    pSUIFHeader = (SUIF_CommonHeader *)pHeader;
    pMagicNumber_BE = (uint32_t *)pSUIFHeader->magicNumber_BE;
    magicNumber = SUIF_Get32(pMagicNumber,BE);
    if(magicNumber == SUIF_MagicNumber_eSDL)
    {   /* this is SUIF format image */
        const SUIF_SDLSpecificHeader *pSUIFSDLHeader = SUIF_GetSdlHeaderFromPackageHeader((SUIF_PackageHeader *)pHeader);
        bool isAlphaSSF = (hSAGElib->frameworkInfo.version[2] == 0) && (hSAGElib->frameworkInfo.version[3] != 0);
        bool isAlphaSDK = (pSUIFSDLHeader->ssfVersion.revision == 0) && (pSUIFSDLHeader->ssfVersion.branch != 0);
        /* Check that any Alpha SSFs are matched with their SDLs and SDLs built from Alpha SDKs are matched with their SSF */
        if (isAlphaSSF || isAlphaSDK) {
            rc = ((hSAGElib->frameworkInfo.version[0] == pSUIFSDLHeader->ssfVersion.major) &&
                  (hSAGElib->frameworkInfo.version[1] == pSUIFSDLHeader->ssfVersion.minor) &&
                  (hSAGElib->frameworkInfo.version[2] == pSUIFSDLHeader->ssfVersion.revision) &&
                  (hSAGElib->frameworkInfo.version[3] == pSUIFSDLHeader->ssfVersion.branch)    );
            if (rc != true) {
                if (isAlphaSSF) {
                    BDBG_ERR(("%s: The SAGE Alpha Framework does not match the SDK version (%u.%u.%u.%u) of the TA",
                              BSTD_FUNCTION, pSUIFSDLHeader->ssfVersion.major,
                              pSUIFSDLHeader->ssfVersion.minor,
                              pSUIFSDLHeader->ssfVersion.revision,
                              pSUIFSDLHeader->ssfVersion.branch));
                }
                else {
                    BDBG_ERR(("%s: The SAGE TA's Alpha SDK version (%u.%u.%u.%u) does not match the Framework",
                              BSTD_FUNCTION, pSUIFSDLHeader->ssfVersion.major,
                              pSUIFSDLHeader->ssfVersion.minor,
                              pSUIFSDLHeader->ssfVersion.revision,
                              pSUIFSDLHeader->ssfVersion.branch));
                }
                goto end;
            }
        } else {
            rc = true;
        }
    }else
    {
        sdlTHLSigShort = pHeader->ucSsfThlShortSig[0] | (pHeader->ucSsfThlShortSig[1] << 8) |
                         (pHeader->ucSsfThlShortSig[2] << 16) | (pHeader->ucSsfThlShortSig[3] << 24);

        if (sdlTHLSigShort != 0) {
            rc = (sdlTHLSigShort == hSAGElib->frameworkInfo.THLShortSig);
            if (rc != true) {
                if ((pHeader->ucSsfVersion[0] | pHeader->ucSsfVersion[1] |
                     pHeader->ucSsfVersion[2] | pHeader->ucSsfVersion[3]) != 0) {
                    rc = (BKNI_Memcmp(pHeader->ucSsfVersion, hSAGElib->frameworkInfo.version, 4) == 0);
                    if (rc != true) {
                        BDBG_ERR(("%s: The SDL is compiled using SAGE version %u.%u.%u.%u SDK",
                                  BSTD_FUNCTION, pHeader->ucSsfVersion[0],
                                  pHeader->ucSsfVersion[1],
                                  pHeader->ucSsfVersion[2],
                                  pHeader->ucSsfVersion[3]));
                        BDBG_ERR(("%s: The SDL is not compiled from the same SDK as the running Framework",
                                  BSTD_FUNCTION));
                        BDBG_ERR(("%s: The SDL must be recompiled using SAGE version %u.%u.%u.%u SDK",
                                  BSTD_FUNCTION, hSAGElib->frameworkInfo.version[0],
                                  hSAGElib->frameworkInfo.version[1],
                                  hSAGElib->frameworkInfo.version[2],
                                  hSAGElib->frameworkInfo.version[3]));
                        goto end;
                    }
                }
                else {
                    BDBG_MSG(("%s: SSF version not found in SDL image, skipping SSF version check", BSTD_FUNCTION));
                }

                BDBG_ERR(("%s: The SDL THL Signature Short (0x%08x) differs from the one inside the loaded SAGE Framework (0x%08x)",
                          BSTD_FUNCTION, sdlTHLSigShort, hSAGElib->frameworkInfo.THLShortSig));
                BDBG_ERR(("%s: The SDL must be linked against the same THL (Thin Layer) as the one inside the SAGE Framework",
                          BSTD_FUNCTION));
                goto end;
            }
        } else {
            bool isAlphaSSF = (hSAGElib->frameworkInfo.version[2] == 0) && (hSAGElib->frameworkInfo.version[3] != 0);
            bool isAlphaSDK = (pHeader->ucSsfVersion[2] == 0) && (pHeader->ucSsfVersion[3] != 0);
            BDBG_MSG(("%s: Trusted App THL signature indicates Load-Time-Resolution", BSTD_FUNCTION));
            /* Check that any Alpha SSFs are matched with their SDLs and SDLs built from Alpha SDKs are matched with their SSF */
            if (isAlphaSSF || isAlphaSDK) {
                rc = (BKNI_Memcmp(pHeader->ucSsfVersion, hSAGElib->frameworkInfo.version, 4) == 0);
                if (rc != true) {
                    if (isAlphaSSF) {
                        BDBG_ERR(("%s: The SAGE Alpha Framework does not match the SDK version (%u.%u.%u.%u) of the TA",
                                  BSTD_FUNCTION, pHeader->ucSsfVersion[0],
                                  pHeader->ucSsfVersion[1],
                                  pHeader->ucSsfVersion[2],
                                  pHeader->ucSsfVersion[3]));
                    }
                    else {
                        BDBG_ERR(("%s: The SAGE TA's Alpha SDK version (%u.%u.%u.%u) does not match the Framework",
                                  BSTD_FUNCTION, pHeader->ucSsfVersion[0],
                                  pHeader->ucSsfVersion[1],
                                  pHeader->ucSsfVersion[2],
                                  pHeader->ucSsfVersion[3]));
                    }
                    goto end;
                }
            } else {
                rc = true;
            }
        }
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
    CASE_PLATFORM_NAME_TO_STRING(BP3);
    CASE_PLATFORM_NAME_TO_STRING(SARM);
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
    char revstr[9];
    BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;
    uint8_t ucSdlVersion[4];                  /* SDL version */
    uint8_t ucSageSecureBootToolVersion[4];   /* Version of the secure boot tool used to signed the binary */
    SUIF_CommonHeader *pSUIFHeader = NULL;
    uint32_t magicNumber,*pMagicNumber_BE;

    if (binSize < sizeof(BSAGElib_SDLHeader)) {
        BDBG_ERR(("%s: The binary size for TA 0x%X is less than the SDL header structure",
                  BSTD_FUNCTION, platformId));
        goto err;
    }
    pHeader = (BSAGElib_SDLHeader *)binBuff;

    pSUIFHeader = (SUIF_CommonHeader *)binBuff;
    pMagicNumber_BE = (uint32_t *)pSUIFHeader->magicNumber_BE;
    magicNumber = SUIF_Get32(pMagicNumber,BE);
    if(magicNumber == SUIF_MagicNumber_eSDL)
    {   /* this is SUIF format image */
        ucSdlVersion[0] = pSUIFHeader->imageVersion.major;
        ucSdlVersion[1] = pSUIFHeader->imageVersion.minor;
        ucSdlVersion[2] = pSUIFHeader->imageVersion.revision;
        ucSdlVersion[3] = pSUIFHeader->imageVersion.branch;
        ucSageSecureBootToolVersion[0] = pSUIFHeader->signingToolVersion.major;
        ucSageSecureBootToolVersion[1] = pSUIFHeader->signingToolVersion.minor;
        ucSageSecureBootToolVersion[2] = pSUIFHeader->signingToolVersion.revision;
        ucSageSecureBootToolVersion[3] = pSUIFHeader->signingToolVersion.branch;
    }else
    {   /* this is 3.x image */
        ucSdlVersion[0] = pHeader->ucSdlVersion[0];
        ucSdlVersion[1] = pHeader->ucSdlVersion[1];
        ucSdlVersion[2] = pHeader->ucSdlVersion[2];
        ucSdlVersion[3] = pHeader->ucSdlVersion[3];
        ucSageSecureBootToolVersion[0] = pHeader->ucSageSecureBootToolVersion[0];
        ucSageSecureBootToolVersion[1] = pHeader->ucSageSecureBootToolVersion[1];
        ucSageSecureBootToolVersion[2] = pHeader->ucSageSecureBootToolVersion[2];
        ucSageSecureBootToolVersion[3] = pHeader->ucSageSecureBootToolVersion[3];
    }

    container = BSAGElib_Rai_Container_Allocate(hSAGElibClient);
    if(container == NULL)
        goto err;

    if ((ucSdlVersion[2] == 0) && (ucSdlVersion[3] != 0)) {
        BKNI_Snprintf(revstr, sizeof(revstr)-1, "Alpha%u", ucSdlVersion[3]);
    } else {
        BKNI_Snprintf(revstr, sizeof(revstr)-1, "%u.%u",
                      ucSdlVersion[2], ucSdlVersion[3]);
    }
    if (platform_name == NULL) {
        BDBG_LOG(("SAGE TA: [0x%X] [Version=%u.%u.%s, Signing Tool=%u.%u.%u.%u]",
                  platformId,
                  ucSdlVersion[0], ucSdlVersion[1], revstr,
                  ucSageSecureBootToolVersion[0], ucSageSecureBootToolVersion[1],
                  ucSageSecureBootToolVersion[2], ucSageSecureBootToolVersion[3]
                  ));
    }
    else {
        BDBG_LOG(("SAGE TA: %s [Version=%u.%u.%s, Signing Tool=%u.%u.%u.%u]",
                  platform_name,
                  ucSdlVersion[0], ucSdlVersion[1], revstr,
                  ucSageSecureBootToolVersion[0], ucSageSecureBootToolVersion[1],
                  ucSageSecureBootToolVersion[2], ucSageSecureBootToolVersion[3]
                  ));
    }
    BDBG_MSG(("%s: Platform %x %p %d",BSTD_FUNCTION, platformId, binBuff,binSize  ));
    BDBG_MSG(("%s: System Platform %p %p",BSTD_FUNCTION, (void *)hSAGElibClient->system_platform, (void*)hSAGElibClient->system_module ));

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
        BDBG_MSG(("%s: System Platform %p ",BSTD_FUNCTION, (void *)hSAGElibClient->system_platform ));

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
            BDBG_MSG(("%s: System Platform Module %p ",BSTD_FUNCTION, (void *)hSAGElibClient->system_module ));

        }
    }


    BDBG_MSG(("%s: Anti-rollback Platform %p %p",BSTD_FUNCTION, (void *)hSAGElibClient->antirollback_platform, (void*)hSAGElibClient->antirollback_module ));

    if((platformId != BSAGE_PLATFORM_ID_ANTIROLLBACK) && (hSAGElibClient->antirollback_platform == NULL)) {

        /* Open the platform */
        rc = BSAGElib_Rai_Platform_Open(hSAGElibClient,
                                        BSAGE_PLATFORM_ID_ANTIROLLBACK,
                                        container,
                                        &(hSAGElibClient->antirollback_platform),
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
        BDBG_MSG(("%s: Anti-rollback Platform %p ",BSTD_FUNCTION, (void *)hSAGElibClient->antirollback_platform ));

        if(container->basicOut[0] != BSAGElib_State_eInit){

            /* Not yet initialized: send init command*/
            rc = BSAGElib_Rai_Platform_Init(hSAGElibClient->antirollback_platform,
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

        if (hSAGElibClient->antirollback_module == NULL) {
            rc = BSAGElib_Rai_Module_Init(hSAGElibClient->antirollback_platform,
                                        AntiRollback_ModuleId_eAntiRollback,
                                        container,
                                        &(hSAGElibClient->antirollback_module),
                                        (void *)hSAGElibClient,
                                        &async_id);
            if (rc != BERR_SUCCESS) {
                rc = BERR_TRACE(rc);
                goto end;
            }

            rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);
            if(rc == BSAGE_ERR_ALREADY_INITIALIZED)
            {
                /* This error can only occurs on SAGE 3.1.x where we do not need to send the RegisterTa command.  */
                rc = BERR_SUCCESS;
            }
            else if (rc != BERR_SUCCESS) {
                goto err;
            }
            BDBG_MSG(("%s: Anti-rollback Platform Module %p ",BSTD_FUNCTION, (void *)hSAGElibClient->antirollback_module ));

        }
    }


    /* do it after the system platform open so we are sure the SAGE-side is booted
       and the status info are valid */
    if (!_P_IsSdlValid(hSAGElibClient->hSAGElib, pHeader)) {
        BDBG_ERR(("%s: Cannot install incompatible SDL", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto err;
    }


    /* Regiter TA to the Anti-rollback TA (skip when installing AR TA) */
    if((platformId != BSAGE_PLATFORM_ID_ANTIROLLBACK) &&
       (hSAGElibClient->antirollback_module != NULL))
    {
        BKNI_Memset(container, 0, sizeof(*container));
        container->basicIn[0] = platformId;

        rc = BSAGElib_Rai_Module_ProcessCommand(hSAGElibClient->antirollback_module,
                                                AntiRollbackModule_CommandId_eRegisterTa,
                                                container,
                                                &async_id);
        if (rc != BERR_SUCCESS) {
            /* If message to Load failed then SAGE might be reset so clean up Anti-rollback Platform Handles */
            BDBG_ERR(("%s BSAGElib_Rai_Module_ProcessCommand failure %d",BSTD_FUNCTION,rc));
            hSAGElibClient->antirollback_platform = NULL;
            hSAGElibClient->antirollback_module = NULL;
            rc = BERR_TRACE(rc);
            goto err;
        }

        rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);
        if(rc == BSAGE_ERR_MODULE_COMMAND_ID)
        {
            /* In SAGE 3.1.x, AR TA did not support RegisterTa command. Just ignore the request. */
            BDBG_MSG(("%s: Anti-rollback TA does not support the RegisterTa command. Ignoring the error.", BSTD_FUNCTION));
            rc = BERR_SUCCESS;
        }
        else if((rc != BERR_SUCCESS) || (container->basicOut[0] != BERR_SUCCESS))
        {
            rc = BERR_UNKNOWN;
            BDBG_ERR(("%s: Cannot register TA to the Anti-rollback TA\n", BSTD_FUNCTION));
            goto err;
        }
    }

    /* Load TA using the System TA */
    BKNI_Memset(container, 0, sizeof(*container));
    container->basicIn[0] = platformId;

    /* provide the SDL binary that's been pulled from FS */
    container->blocks[0].data.ptr = binBuff;
    container->blocks[0].len = binSize;

    if(magicNumber != SUIF_MagicNumber_eSDL)
    {
        /* this is sage 3.x iamge, not SUIF image */
        BSAGElib_SDLHeader *pHeader = (BSAGElib_SDLHeader *)binBuff;
        if(pHeader->ucSageImageSigningScheme == BSAGELIB_SDL_IMAGE_SIGNING_SCHEME_SINGLE)
        {
            BDBG_MSG(("%s: Single signed image detected ^^^^^", BSTD_FUNCTION)); /* TODO: change to MSG */
            container->basicIn[1] = 0;
        }
        else if(pHeader->ucSageImageSigningScheme == BSAGELIB_SDL_IMAGE_SIGNING_SCHEME_TRIPLE)
        {
            BDBG_MSG(("%s: Triple signed image detected ^^^^^", BSTD_FUNCTION)); /* TODO: change to MSG */
            container->basicIn[1] = 1;
        }
        else{
            BDBG_ERR(("%s: invalid Image Siging Scheme value (0x%02x) detected", BSTD_FUNCTION, pHeader->ucSageImageSigningScheme));
            rc = BERR_INVALID_PARAMETER;
            goto end;
        }
    }

    rc = BSAGElib_Rai_Module_ProcessCommand(hSAGElibClient->system_module,
                                            DynamicLoadModule_CommandId_eLoadSDL,
                                            container,
                                            &async_id);

    if (rc != BERR_SUCCESS) {
        /* If message to Load failed then SAGE might be reset so clean up System Platform Handles */
        BDBG_ERR(("%s BSAGElib_Rai_Module_ProcessCommand failure %d",BSTD_FUNCTION,rc));
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
    BDBG_MSG(("%s Output Status %x",BSTD_FUNCTION,container->basicOut[0]));

    /* If RPC layer had no errors, then pass on the LoadSDL command response error, if applicable */
    if (rc == BERR_SUCCESS) {
        switch (container->basicOut[0]) {
        case BSAGE_ERR_SDL_ALREADY_LOADED:
        case BERR_SUCCESS:
            /* Leave rc as success */
            break;
        default:
            BDBG_ERR(("%s: SAGE failed to Load TA, (rc=0x%08x, %s)", BSTD_FUNCTION, container->basicOut[0], BSAGElib_Tools_ReturnCodeToString(container->basicOut[0])));
            rc = container->basicOut[0];
            break;
        }
    }
    if (rc != BERR_SUCCESS) {
        goto err;
    }

    if(hSAGElib->securelog_module != NULL
      && hSAGElib->securelogContainer != NULL
      && platformId != BSAGE_PLATFORM_ID_SYSTEM
      && platformId != BSAGE_PLATFORM_ID_SYSTEM_CRIT
      && platformId != BSAGE_PLATFORM_ID_ANTIROLLBACK)
    {
        uint32_t async_id;

        hSAGElib->securelogContainer->basicIn[0]=platformId;
        rc = BSAGElib_Rai_Module_ProcessCommand(hSAGElib->securelog_module,
                                            Secure_Log_CommandId_eAttach,
                                            hSAGElib->securelogContainer, &async_id);
        if (rc != BERR_SUCCESS)
        {
            rc = BERR_TRACE(rc);
            goto end;
        }
        rc = BSAGELib_Rai_P_WaitForResponse(hSAGElib->securelog_module->hSAGElibClient, async_id);
        if (rc != BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
            goto end;
        }
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
    BSAGElib_Handle hSAGElib = hSAGElibClient->hSAGElib;


    if((hSAGElibClient->system_module == NULL) || (hSAGElibClient->system_platform == NULL)){
        rc = BERR_INVALID_PARAMETER;
        goto err;
    }

    if(platformId == BSAGE_PLATFORM_ID_SECURE_LOGGING)
    {
        hSAGElib->securelog_module = NULL;
        hSAGElib->securelogContainer = NULL;
    }else
    if(hSAGElib->securelog_module != NULL
      && hSAGElib->securelogContainer != NULL
      && platformId != BSAGE_PLATFORM_ID_SYSTEM
      && platformId != BSAGE_PLATFORM_ID_SYSTEM_CRIT
      && platformId != BSAGE_PLATFORM_ID_ANTIROLLBACK)
    {
        uint32_t async_id;

        hSAGElib->securelogContainer->basicIn[0]=platformId;
        rc = BSAGElib_Rai_Module_ProcessCommand(hSAGElib->securelog_module,
                                            Secure_Log_CommandId_eDetach,
                                            hSAGElib->securelogContainer, &async_id);
        if (rc != BERR_SUCCESS)
        {
            rc = BERR_TRACE(rc);
            goto end;
        }
        rc = BSAGELib_Rai_P_WaitForResponse(hSAGElib->securelog_module->hSAGElibClient, async_id);
        if (rc != BERR_SUCCESS) {
            rc = BERR_TRACE(rc);
        }
    }

    container = BSAGElib_Rai_Container_Allocate(hSAGElibClient);
    if(container == NULL)
        goto err;

    container->basicIn[0] = platformId;

    BDBG_MSG(("%s: System Platform %p %p",BSTD_FUNCTION, (void *)hSAGElibClient->system_platform, (void*)hSAGElibClient->system_module ));

    rc = BSAGElib_Rai_Module_ProcessCommand(hSAGElibClient->system_module,
                                            DynamicLoadModule_CommandId_eUnLoadSDL,
                                            container,
                                            &async_id);

    if (rc != BERR_SUCCESS) {
        /* If message to Unload failed then SAGE might be reset so clean up System Platform Handles */
        BDBG_ERR(("%s BSAGElib_Rai_Module_ProcessCommand failure %d",BSTD_FUNCTION,rc));
        hSAGElibClient->system_platform = NULL;
        hSAGElibClient->system_module = NULL;
        goto err;
    }

    rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);

    if (rc != BERR_SUCCESS) {
        goto err;
    }

    BDBG_MSG(("%s Output Status %d",BSTD_FUNCTION,container->basicOut[0]));

    /* Uninitialize the Anti-Rollback module */
    if (hSAGElibClient->antirollback_module != NULL) {
        BSAGElib_Rai_Module_Uninit(hSAGElibClient->antirollback_module, &async_id);
        rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);
        if (rc != BERR_SUCCESS) {
            goto err;
        }

        BSAGElib_Rpc_RemoveRemote(hSAGElibClient->antirollback_module);
        hSAGElibClient->antirollback_module = NULL;
    }

    /* Uninitialize the Anti-Rollback platform */
    if (hSAGElibClient->antirollback_platform != NULL) {
        BSAGElib_Rai_Platform_Close(hSAGElibClient->antirollback_platform, &async_id);
        rc = BSAGELib_Rai_P_WaitForResponse(hSAGElibClient, async_id);
        if (rc != BERR_SUCCESS) {
            goto err;
        }
        BSAGElib_Rpc_RemoveRemote(hSAGElibClient->antirollback_platform);
        hSAGElibClient->antirollback_platform = NULL;
    }

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
        BDBG_ERR(("%s: already enabled", BSTD_FUNCTION));
        goto end;
    }
#endif
    hSAGElibClient = platform->hSAGElibClient;
    hSAGElib = hSAGElibClient->hSAGElib;

    if (BSAGElib_iRpcCallbackRequest_isr == NULL) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: client does not support callback requests", BSTD_FUNCTION));
        goto end;
    }

    container = BSAGElib_Rai_Container_Allocate(platform->hSAGElibClient);
    if (container == NULL) {
        BDBG_ERR(("%s: BSAGElib_Rai_Container_Allocate() failure (%u)", BSTD_FUNCTION, rc));
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto end;
    }
    message = (BSAGElib_RpcMessage *)BSAGElib_Rai_Memory_Allocate(platform->hSAGElibClient,
                                                                  sizeof(*message),
                                                                  BSAGElib_MemoryType_Global);
    if (message == NULL) {
        BDBG_ERR(("%s: BSAGElib_Rai_Memory_Allocate() failure to allocate message (%u)", BSTD_FUNCTION, rc));
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
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand (%u)", BSTD_FUNCTION, rc));
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

    BDBG_MSG(("%s: Platform %x ",BSTD_FUNCTION, platformId));

    if (!pPlatform) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: pPlatform is NULL", BSTD_FUNCTION));
        goto end;
    }

    /* Convert and sync virtual <--> physical memory */
    command.containerOffset = BSAGElib_Tools_ContainerAddressToOffset(container,
                                                                      &hSAGElib->i_memory_sync_isrsafe,
                                                                      &hSAGElib->i_memory_map);
    if (!command.containerOffset) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: invalid container %p", BSTD_FUNCTION, (void *)container));
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
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%d)", BSTD_FUNCTION, (int)rc));
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

    BDBG_MSG(("%s Platform id %x Module id %x",BSTD_FUNCTION,platform->platformId,platform->moduleId));

    /* Convert and sync virtual <--> physical memory */
    command.containerOffset = BSAGElib_Tools_ContainerAddressToOffset(container,
                                                                      &hSAGElib->i_memory_sync_isrsafe,
                                                                      &hSAGElib->i_memory_map);
    command.containerVAddr = container;
    command.moduleCommandId = 0;
    command.systemCommandId = BSAGElib_SystemCommandId_ePlatformInit;
    rc = BSAGElib_Rpc_SendCommand(platform, &command, pAsync_id);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%u)", BSTD_FUNCTION, rc));
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

    BDBG_MSG(("%s Platform id %x Module id %x",BSTD_FUNCTION,platform->platformId,platform->moduleId));

#ifndef SAGE_KO
    if (platform->valid) {
#endif
        command.containerVAddr = NULL;
        command.containerOffset = 0;
        command.moduleCommandId = 0;
        command.systemCommandId = BSAGElib_SystemCommandId_ePlatformClose;
        rc = BSAGElib_Rpc_SendCommand(platform, &command, pAsync_id);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%u)", BSTD_FUNCTION, rc));
            /* keep going */
        }
#ifndef SAGE_KO
    }
    else {
        BDBG_WRN(("%s: platform is not valid anymore, skip command send", BSTD_FUNCTION));
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
        BDBG_ERR(("%s: pModule is NULL", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto end;
    }

    new_module = BSAGElib_Rpc_AddRemote(platform->hSAGElibClient, platform->platformId, moduleId, async_argument);
    if (!new_module) {
        rc = BERR_INVALID_PARAMETER;
        BDBG_ERR(("%s: BSAGElib_Rpc_AddRemote failure", BSTD_FUNCTION));
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
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand (%u)", BSTD_FUNCTION, rc));
        goto end;
    }

    platform->hSAGElibClient->moduleNum++;
    BSAGElib_P_Rai_Adjust_ContainerCache(platform->hSAGElibClient);
    *pModule = new_module;

    if(new_module->platformId == BSAGE_PLATFORM_ID_SECURE_LOGGING)
    {
        hSAGElib->securelog_module = new_module;
        hSAGElib->securelogContainer = container;
    }

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
    BSAGElib_Handle hSAGElib;

    BDBG_ENTER(BSAGElib_Rai_Module_Uninit);
    hSAGElib = module->hSAGElibClient->hSAGElib;

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
            BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%u)", BSTD_FUNCTION, rc));
            /* keep going */
        }
#ifndef SAGE_KO
    }
    else {
        BDBG_WRN(("%s: module is not valid anymore, skip command send", BSTD_FUNCTION));
    }
#endif
    module->hSAGElibClient->moduleNum--;
    BSAGElib_P_Rai_Adjust_ContainerCache(module->hSAGElibClient);

    if(module->platformId == BSAGE_PLATFORM_ID_SECURE_LOGGING)
    {
        hSAGElib->securelog_module = NULL;
        hSAGElib->securelogContainer = NULL;
    }

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
        BDBG_ERR(("%s: BSAGElib_Rpc_SendCommand failure (%d)", BSTD_FUNCTION, (int)rc));
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

    switch (memoryType) {
    case BSAGElib_MemoryType_Restricted:
        ret = BSAGElib_iMallocRestricted(size);
        break;
    case BSAGElib_MemoryType_Global:
        ret = BSAGElib_iMalloc(size);
        break;
    default:
        BDBG_ERR(("%s: bad memory type %u", BSTD_FUNCTION, memoryType));
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
