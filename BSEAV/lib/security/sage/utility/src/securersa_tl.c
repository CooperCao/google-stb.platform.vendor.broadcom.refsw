/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <stdio.h>
#include <string.h> /* for strlen */

#include "utility_platform.h"
#include "utility_ids.h"
#include "nexus_memory.h"
#include "bsagelib_types.h"
#include "bsagelib_crypto_types.h"
#include "securersa_tl.h"


#define SECURERSA_UTILITY_DRM_BIN_DEFAULT_FILEPATH "./drm.bin"


BDBG_MODULE(securersa_tl);


struct SecureRsaTl_P_Instance
{
    BDBG_OBJECT(SecureRsaTl_P_Instance)
    SRAI_ModuleHandle moduleHandle;
};

BDBG_OBJECT_ID_DECLARE(SecureRsaTl_P_Instance);
BDBG_OBJECT_ID(SecureRsaTl_P_Instance);


static void _SecureRsaTl_ContextDelete(
    SecureRsaTl_Handle hSecureRsaTl)
{
    if(hSecureRsaTl)
    {
        if(hSecureRsaTl->moduleHandle)
        {
            Utility_ModuleUninit(hSecureRsaTl->moduleHandle);
            hSecureRsaTl->moduleHandle = NULL;
        }

        BDBG_OBJECT_DESTROY(hSecureRsaTl, SecureRsaTl_P_Instance);
        NEXUS_Memory_Free(hSecureRsaTl);
        hSecureRsaTl = NULL;
    }
}


static SecureRsaTl_Handle _SecureRsaTl_ContextNew(
    void)
{
    BERR_Code magnum_rc;
    NEXUS_Error nexus_rc;
    BSAGElib_InOutContainer *container = NULL;
    SecureRsaTl_Handle hSecureRsaTl = NULL;

    nexus_rc = NEXUS_Memory_Allocate(sizeof(*hSecureRsaTl), NULL,
                                     (void **)&hSecureRsaTl);
    if(nexus_rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        magnum_rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }
    hSecureRsaTl->moduleHandle = NULL;

    BDBG_OBJECT_SET(hSecureRsaTl, SecureRsaTl_P_Instance);

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        magnum_rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }

    /* Initialize SAGE Secure Rsa module */
    magnum_rc = Utility_ModuleInit(Utility_ModuleId_eSecureRsa,
                                   NULL,
                                   container,
                                   &hSecureRsaTl->moduleHandle);
    if(magnum_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error initializing Secure Rsa TL module (0x%08x)",
                  BSTD_FUNCTION, container->basicOut[0]));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
        container = NULL;
    }

    if(magnum_rc != BERR_SUCCESS && hSecureRsaTl)
    {
        _SecureRsaTl_ContextDelete(hSecureRsaTl);
        hSecureRsaTl = NULL;
    }

    return hSecureRsaTl;
}


BERR_Code SecureRsaTl_Init(
    SecureRsaTl_Handle *pSecureRsaTlHandle)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(SecureRsaTl_Init);

    if(pSecureRsaTlHandle == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    *pSecureRsaTlHandle = _SecureRsaTl_ContextNew();
    if(*pSecureRsaTlHandle == NULL)
    {
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }

ErrorExit:
    BDBG_LEAVE(SecureRsaTl_Init);
    return rc;
}


void SecureRsaTl_Uninit(
    SecureRsaTl_Handle hSecureRsaTl)
{
    BDBG_ENTER(SecureRsaTl_Uninit);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);
    _SecureRsaTl_ContextDelete(hSecureRsaTl);

    BDBG_LEAVE(SecureRsaTl_Uninit);
}


void SecureRsaTl_GetDefaultLoadRsaPackageSettings(
    SecureRsaTl_LoadRsaPackageSettings *pLoadRsaPackageSettings)
{
    BDBG_ASSERT(pLoadRsaPackageSettings);
    BKNI_Memset((uint8_t *)pLoadRsaPackageSettings, 0x00,
                sizeof(SecureRsaTl_LoadRsaPackageSettings));
    BKNI_Memcpy(pLoadRsaPackageSettings->drm_binfile_path,
                SECURERSA_UTILITY_DRM_BIN_DEFAULT_FILEPATH,
                strlen(SECURERSA_UTILITY_DRM_BIN_DEFAULT_FILEPATH));
}


BERR_Code SecureRsaTl_LoadRsaPackage(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_LoadRsaPackageSettings *pLoadRsaPackageSettings)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;


    BDBG_ENTER(SecureRsaTl_LoadRsaPackage);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if((pLoadRsaPackageSettings == NULL) ||
       (pLoadRsaPackageSettings->drm_binfile_path == NULL))
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    if(Utility_ModuleLoadDrmBin(pLoadRsaPackageSettings->drm_binfile_path,
                                container, hSecureRsaTl->moduleHandle,
                                SecureRsa_CommandId_eLoadRsaPackage) != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error loading RSA package", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_LoadRsaPackage);
    return rc;
}


void SecureRsaTl_GetDefaultGetStatusSettings(
    SecureRsaTl_GetStatusSettings *pGetStatusSettings)
{
    BDBG_ASSERT(pGetStatusSettings);
    memset(pGetStatusSettings, 0, sizeof(SecureRsaTl_GetStatusSettings));
}


BERR_Code SecureRsaTl_GetStatus(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_GetStatusSettings *pGetStatusSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t *ptr32;


    BDBG_ENTER(SecureRsaTl_GetStatus);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if((pGetStatusSettings == NULL) ||
       (pGetStatusSettings->rsaNumKeys == NULL) ||
       (pGetStatusSettings->key3NumKeys == NULL) ||
       (pGetStatusSettings->kpkNumKeys == NULL))
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = SECURE_RSA_TL_GET_STATUS_NUM_KEY_TYPES * sizeof(uint32_t);
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memset(container->blocks[0].data.ptr, 0, container->blocks[0].len);
    ptr32 = (uint32_t *) container->blocks[0].data.ptr;

    if((*(pGetStatusSettings->keyInfoLen) > 0) &&
       (pGetStatusSettings->keyInfo != NULL))
    {
        container->blocks[1].len = *(pGetStatusSettings->keyInfoLen);
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memset(container->blocks[1].data.ptr, 0, container->blocks[1].len);
    }

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eGetStatus,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

    *(pGetStatusSettings->rsaNumKeys) = ptr32[0];
    *(pGetStatusSettings->key3NumKeys) = ptr32[1];
    *(pGetStatusSettings->kpkNumKeys) = ptr32[2];

    if((*(pGetStatusSettings->keyInfoLen) > 0) &&
       (pGetStatusSettings->keyInfo != NULL))
    {
        BKNI_Memcpy(pGetStatusSettings->keyInfo, container->blocks[1].data.ptr,
                    container->basicOut[1]);
    }

    *(pGetStatusSettings->keyInfoLen) = container->basicOut[1];

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_GetStatus);
    return rc;
}


void SecureRsaTl_GetDefaultRemoveKeySettings(
    SecureRsaTl_RemoveKeySettings *pRemoveKeySettings)
{
    BDBG_ASSERT(pRemoveKeySettings);
    memset(pRemoveKeySettings, 0, sizeof(SecureRsaTl_RemoveKeySettings));
}


BERR_Code SecureRsaTl_RemoveKey(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RemoveKeySettings *pRemoveKeySettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(SecureRsaTl_RemoveKey);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pRemoveKeySettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->basicIn[0] = pRemoveKeySettings->keyType;

    switch(pRemoveKeySettings->keyType)
    {
    case SecureRsaTl_KeyType_eAll:
        container->basicIn[1] = 0;
        break;

    case SecureRsaTl_KeyType_eRsa:
        container->basicIn[1] = pRemoveKeySettings->keyslot.rsaKeySlot;
        break;

    case SecureRsaTl_KeyType_eKey3:
        container->basicIn[1] = pRemoveKeySettings->keyslot.key3KeySlot;
        break;

    case SecureRsaTl_KeyType_eKpk:
        container->basicIn[1] = pRemoveKeySettings->keyslot.kpkKeySlot;
        break;

    default:
        BDBG_ERR(("%s - Invalid key type %x", BSTD_FUNCTION,
                  pRemoveKeySettings->keyType));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eRemoveKey,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_RemoveKey);
    return rc;
}


void SecureRsaTl_GetDefaultRsaSignSettings(
    SecureRsaTl_RsaSignVerifySettings *pRsaSignVerifySettings)
{
    BDBG_ASSERT(pRsaSignVerifySettings);
    memset(pRsaSignVerifySettings, 0, sizeof(SecureRsaTl_RsaSignVerifySettings));
}


static BERR_Code SecureRsaTl_RsaSignVerify(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaSignVerifySettings *pRsaSignVerifySettings,
    bool signOperation)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t commandId;

    BDBG_ENTER(SecureRsaTl_RsaSignVerify);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pRsaSignVerifySettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = pRsaSignVerifySettings->inputLen;
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[0].data.ptr, pRsaSignVerifySettings->input,
                container->blocks[0].len);

    container->blocks[1].len = *(pRsaSignVerifySettings->signatureLen);
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    if(signOperation)
    {
        BKNI_Memset(container->blocks[1].data.ptr, 0, container->blocks[1].len);
        commandId = SecureRsa_CommandId_eRsaSign;
    }
    else
    {
        BKNI_Memcpy(container->blocks[1].data.ptr,
                    pRsaSignVerifySettings->signature,
                    container->blocks[1].len);
        commandId = SecureRsa_CommandId_eRsaVerify;
    }

    container->basicIn[0] = pRsaSignVerifySettings->rsaKeySlot;
    container->basicIn[1] = pRsaSignVerifySettings->sigPadType;
    container->basicIn[2] = pRsaSignVerifySettings->sigDigestType;
    container->basicIn[3] = pRsaSignVerifySettings->sigPssSaltLen;

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle, commandId,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

    if(signOperation)
    {
        *(pRsaSignVerifySettings->signatureLen) = container->basicOut[1];
        BKNI_Memcpy(pRsaSignVerifySettings->signature,
                    container->blocks[1].data.ptr,
                    *(pRsaSignVerifySettings->signatureLen));
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_RsaSignVerify);
    return rc;
}


BERR_Code SecureRsaTl_RsaSign(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaSignVerifySettings *pRsaSignSettings)
{
    return(SecureRsaTl_RsaSignVerify(hSecureRsaTl,
                                     pRsaSignSettings,
                                     true));
}


BERR_Code SecureRsaTl_RsaVerify(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaSignVerifySettings *pRsaVerifySettings)
{
    return(SecureRsaTl_RsaSignVerify(hSecureRsaTl,
                                     pRsaVerifySettings,
                                     false));
}


void SecureRsaTl_GetDefaultRsaHostUsageSettings(
    SecureRsaTl_RsaHostUsageSettings *pRsaHostUsageSettings)
{
    BDBG_ASSERT(pRsaHostUsageSettings);
    memset(pRsaHostUsageSettings, 0, sizeof(SecureRsaTl_RsaHostUsageSettings));
}


BERR_Code SecureRsaTl_RsaHostUsage(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaHostUsageSettings *pRsaHostUsageSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(SecureRsaTl_RsaHostUsage);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pRsaHostUsageSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = pRsaHostUsageSettings->inputLen;
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[0].data.ptr, pRsaHostUsageSettings->input,
                container->blocks[0].len);

    container->blocks[1].len = *(pRsaHostUsageSettings->outputLen);
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->basicIn[0] = pRsaHostUsageSettings->rsaKeySlot;
    container->basicIn[1] = pRsaHostUsageSettings->rsaKeyType;

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eRsaHostUsage,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

    *(pRsaHostUsageSettings->outputLen) = container->basicOut[1];

    BKNI_Memcpy(pRsaHostUsageSettings->output, container->blocks[1].data.ptr,
                container->blocks[1].len);

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_RsaHostUsage);
    return rc;
}


void SecureRsaTl_GetDefaultRsaDecryptKey3Settings(
    SecureRsaTl_RsaDecryptAesSettings *pRsaDecryptAesSettings)
{
    BDBG_ASSERT(pRsaDecryptAesSettings);
    memset(pRsaDecryptAesSettings, 0, sizeof(SecureRsaTl_RsaDecryptAesSettings));
}


static BERR_Code SecureRsaTl_RsaDecryptAes(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaDecryptAesSettings *pRsaDecryptAesSettings,
    bool key3Operation)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t *ptr32;
    uint32_t commandId;

    BDBG_ENTER(SecureRsaTl_RsaDecryptAes);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pRsaDecryptAesSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = SECURE_RSA_TL_RSA_DECRYPT_AES_NUM_PARAMETERS * sizeof(uint32_t);
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    ptr32 = (uint32_t *) container->blocks[0].data.ptr;
    ptr32[0] = pRsaDecryptAesSettings->aesKeySlot;
    ptr32[1] = pRsaDecryptAesSettings->aesKeyLen;
    ptr32[2] = pRsaDecryptAesSettings->encKeySlot;
    ptr32[3] = pRsaDecryptAesSettings->encPadType;
    ptr32[4] = pRsaDecryptAesSettings->encDigestType;
    ptr32[5] = pRsaDecryptAesSettings->sigKeySlot;
    ptr32[6] = pRsaDecryptAesSettings->sigPadType;
    ptr32[7] = pRsaDecryptAesSettings->sigDigestType;
    ptr32[8] = pRsaDecryptAesSettings->sigPssSaltLen;

    container->blocks[1].len = pRsaDecryptAesSettings->encKeySettingsLen;
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[1].data.ptr,
                pRsaDecryptAesSettings->encKeySettings,
                container->blocks[1].len);

    container->blocks[2].len = pRsaDecryptAesSettings->encKeyLen;
    container->blocks[2].data.ptr = SRAI_Memory_Allocate(container->blocks[2].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[2].data.ptr, pRsaDecryptAesSettings->encKey,
                container->blocks[2].len);

    if(pRsaDecryptAesSettings->signatureLen > 0)
    {
        container->blocks[3].len = pRsaDecryptAesSettings->signatureLen;
        container->blocks[3].data.ptr = SRAI_Memory_Allocate(container->blocks[3].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[3].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[3].data.ptr,
                    pRsaDecryptAesSettings->signature,
                    container->blocks[3].len);
    }

    if(key3Operation)
    {
        commandId = SecureRsa_CommandId_eRsaDecryptKey3;
    }
    else
    {
        commandId = SecureRsa_CommandId_eRsaDecryptKpk;
    }

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle, commandId,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        if(container->blocks[2].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }

        if(container->blocks[3].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[3].data.ptr);
            container->blocks[3].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_RsaDecryptAes);
    return rc;
}


BERR_Code SecureRsaTl_RsaDecryptKey3(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaDecryptAesSettings *pRsaDecryptAesSettings)
{
    return(SecureRsaTl_RsaDecryptAes(hSecureRsaTl, pRsaDecryptAesSettings,
           true));
}


BERR_Code SecureRsaTl_RsaDecryptKpk(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaDecryptAesSettings *pRsaDecryptAesSettings)
{
    return(SecureRsaTl_RsaDecryptAes(hSecureRsaTl, pRsaDecryptAesSettings,
           false));
}


void SecureRsaTl_GetDefaultRsaLoadPublicKeySettings(
    SecureRsaTl_RsaLoadPublicKeySettings *pRsaLoadPublicKeySettings)
{
    BDBG_ASSERT(pRsaLoadPublicKeySettings);
    memset(pRsaLoadPublicKeySettings, 0,
           sizeof(SecureRsaTl_RsaLoadPublicKeySettings));
}


BERR_Code SecureRsaTl_RsaLoadPublicKey(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_RsaLoadPublicKeySettings *pRsaLoadPublicKeySettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(SecureRsaTl_RsaLoadPublicKey);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pRsaLoadPublicKeySettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = pRsaLoadPublicKeySettings->publicModulusLen;
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[0].data.ptr, pRsaLoadPublicKeySettings->publicModulus,
                container->blocks[0].len);

    container->blocks[1].len = pRsaLoadPublicKeySettings->signatureLen;
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[1].data.ptr,
                pRsaLoadPublicKeySettings->signature,
                container->blocks[1].len);

    container->basicIn[0] = pRsaLoadPublicKeySettings->rsaKeySlot;
    container->basicIn[1] = pRsaLoadPublicKeySettings->sigKeySlot;
    container->basicIn[2] = pRsaLoadPublicKeySettings->sigPadType;
    container->basicIn[3] = pRsaLoadPublicKeySettings->sigDigestType;
    container->basicIn[4] = pRsaLoadPublicKeySettings->sigPssSaltLen;

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eRsaLoadPublicKey,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_RsaLoadPublicKey);
    return rc;
}


void SecureRsaTl_GetDefaultKey3ImportSettings(
    SecureRsaTl_Key3ImportExportSettings *pKey3ImportExportSettings)
{
    BDBG_ASSERT(pKey3ImportExportSettings);
    memset(pKey3ImportExportSettings, 0,
           sizeof(SecureRsaTl_Key3ImportExportSettings));
}


static BERR_Code SecureRsaTl_Key3ImportExport(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3ImportExportSettings *pKey3ImportExportSettings,
    bool importOperation)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t commandId;

    BDBG_ENTER(SecureRsaTl_Key3ImportExport);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pKey3ImportExportSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = *(pKey3ImportExportSettings->encKey3Len);
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    if(importOperation)
    {
        BKNI_Memcpy(container->blocks[0].data.ptr,
                    pKey3ImportExportSettings->encKey3,
                    container->blocks[0].len);
        commandId = SecureRsa_CommandId_eKey3Import;
    }
    else
    {
        commandId = SecureRsa_CommandId_eKey3Export;
    }

    container->basicIn[0] = pKey3ImportExportSettings->key3KeySlot;

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle, commandId,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

    if(!importOperation)
    {
        *(pKey3ImportExportSettings->encKey3Len) = container->basicOut[1];
        BKNI_Memcpy(pKey3ImportExportSettings->encKey3,
                    container->blocks[0].data.ptr,
                    container->blocks[0].len);
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_Key3ImportExport);
    return rc;
}


BERR_Code SecureRsaTl_Key3Import(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3ImportExportSettings *pKey3ImportSettings)
{
    return(SecureRsaTl_Key3ImportExport(hSecureRsaTl,
                                        pKey3ImportSettings,
                                        true));
}


BERR_Code SecureRsaTl_Key3Export(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3ImportExportSettings *pKey3ExportSettings)
{
    return(SecureRsaTl_Key3ImportExport(hSecureRsaTl,
                                        pKey3ExportSettings,
                                        false));
}


void SecureRsaTl_GetDefaultKey3RouteSettings(
    SecureRsaTl_Key3RouteSettings *pKey3RouteSettings)
{
    BDBG_ASSERT(pKey3RouteSettings);
    memset(pKey3RouteSettings, 0, sizeof(SecureRsaTl_Key3RouteSettings));
}


BERR_Code SecureRsaTl_Key3Route(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3RouteSettings *pKey3RouteSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t *ptr32;

    BDBG_ENTER(SecureRsaTl_Key3Route);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pKey3RouteSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = SECURE_RSA_TL_KEY3_ROUTE_NUM_PARAMETERS * sizeof(uint32_t);
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    ptr32 = (uint32_t *) container->blocks[0].data.ptr;
    ptr32[0] = pKey3RouteSettings->key3KeySlot;
    ptr32[1] = pKey3RouteSettings->keySlotUsage;
    ptr32[2] = pKey3RouteSettings->keySlotNum;
    ptr32[3] = pKey3RouteSettings->routeAlgorithm;
    ptr32[4] = pKey3RouteSettings->variant;
    ptr32[5] = pKey3RouteSettings->operation;
    ptr32[6] = pKey3RouteSettings->keyType;
    ptr32[7] = pKey3RouteSettings->terminationMode;
    ptr32[8] = pKey3RouteSettings->counterMode;
    ptr32[9] = pKey3RouteSettings->counterSize;
    ptr32[10] = pKey3RouteSettings->keyLadderAlgorithm;

    if(pKey3RouteSettings->ivLen > 0)
    {
        container->blocks[1].len = pKey3RouteSettings->ivLen;
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr, pKey3RouteSettings->iv,
                    container->blocks[1].len);
    }

    if(pKey3RouteSettings->procInLen > 0)
    {
        container->blocks[2].len = pKey3RouteSettings->procInLen;
        container->blocks[2].data.ptr = SRAI_Memory_Allocate(container->blocks[2].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[2].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[2].data.ptr, pKey3RouteSettings->procIn,
                    container->blocks[2].len);
    }

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eKey3Route,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        if(container->blocks[2].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_Key3Route);
    return rc;
}


void SecureRsaTl_GetDefaultKey3UnrouteSettings(
    SecureRsaTl_Key3UnrouteSettings *pKey3UnrouteSettings)
{
    BDBG_ASSERT(pKey3UnrouteSettings);
    memset(pKey3UnrouteSettings, 0, sizeof(SecureRsaTl_Key3UnrouteSettings));
}


BERR_Code SecureRsaTl_Key3Unroute(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3UnrouteSettings *pKey3UnrouteSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(SecureRsaTl_Key3Unroute);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pKey3UnrouteSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->basicIn[0] = pKey3UnrouteSettings->key3KeySlot;

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eKey3Unroute,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_Key3Unroute);
    return rc;
}


void SecureRsaTl_GetDefaultKey3CalculateHmacSettings(
    SecureRsaTl_Key3CalculateHmacSettings *pKey3CalculateHmacSettings)
{
    BDBG_ASSERT(pKey3CalculateHmacSettings);
    memset(pKey3CalculateHmacSettings, 0, sizeof(SecureRsaTl_Key3CalculateHmacSettings));
}


BERR_Code SecureRsaTl_Key3CalculateHmac(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3CalculateHmacSettings *pKey3CalculateHmacSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(SecureRsaTl_Key3CalculateHmac);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pKey3CalculateHmacSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = pKey3CalculateHmacSettings->inputDataLen;
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[0].data.ptr,
                pKey3CalculateHmacSettings->inputData,
                container->blocks[0].len);

    container->blocks[1].len = *(pKey3CalculateHmacSettings->hmacLen);
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->basicIn[0] = pKey3CalculateHmacSettings->key3KeySlot;
    container->basicIn[1] = pKey3CalculateHmacSettings->hmacType;

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eKey3CalculateHmac,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

    *(pKey3CalculateHmacSettings->hmacLen) = container->basicOut[1];
    BKNI_Memcpy(pKey3CalculateHmacSettings->hmac,
                container->blocks[1].data.ptr,
                *(pKey3CalculateHmacSettings->hmacLen));

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_Key3CalculateHmac);
    return rc;
}


void SecureRsaTl_GetDefaultKey3AppendShaSettings(
    SecureRsaTl_Key3AppendShaSettings *pKey3AppendShaSettings)
{
    BDBG_ASSERT(pKey3AppendShaSettings);
    memset(pKey3AppendShaSettings, 0, sizeof(SecureRsaTl_Key3AppendShaSettings));
}


BERR_Code SecureRsaTl_Key3AppendSha(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3AppendShaSettings *pKey3AppendShaSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(SecureRsaTl_Key3AppendSha);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pKey3AppendShaSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    if(pKey3AppendShaSettings->inputDataLen > 0)
    {
        container->blocks[0].len = pKey3AppendShaSettings->inputDataLen;
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[0].data.ptr,
                    pKey3AppendShaSettings->inputData,
                    container->blocks[0].len);
    }

    container->blocks[1].len = *(pKey3AppendShaSettings->digestLen);
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->basicIn[0] = pKey3AppendShaSettings->key3KeySlot;
    container->basicIn[1] = pKey3AppendShaSettings->digestType;

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eKey3AppendSha,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

    *(pKey3AppendShaSettings->digestLen) = container->basicOut[1];
    BKNI_Memcpy(pKey3AppendShaSettings->digest,
                container->blocks[1].data.ptr,
                *(pKey3AppendShaSettings->digestLen));

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_Key3AppendSha);
    return rc;
}


void SecureRsaTl_GetDefaultKey3LoadClearIkrSettings(
    SecureRsaTl_Key3LoadClearIkrSettings *pKey3LoadClearIkrSettings)
{
    BDBG_ASSERT(pKey3LoadClearIkrSettings);
    memset(pKey3LoadClearIkrSettings, 0, sizeof(SecureRsaTl_Key3LoadClearIkrSettings));
}


BERR_Code SecureRsaTl_Key3LoadClearIkr(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3LoadClearIkrSettings *pKey3LoadClearIkrSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(SecureRsaTl_Key3LoadClearIkr);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pKey3LoadClearIkrSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = pKey3LoadClearIkrSettings->keyLen;
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[0].data.ptr,
                pKey3LoadClearIkrSettings->key,
                container->blocks[0].len);

    container->basicIn[0] = pKey3LoadClearIkrSettings->key3KeySlot;

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eKey3LoadClearIkr,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_Key3LoadClearIkr);
    return rc;
}


void SecureRsaTl_GetDefaultKey3IkrDecryptIkrSettings(
    SecureRsaTl_Key3IkrDecryptIkrSettings *pKey3IkrDecryptIkrSettings)
{
    BDBG_ASSERT(pKey3IkrDecryptIkrSettings);
    memset(pKey3IkrDecryptIkrSettings, 0, sizeof(SecureRsaTl_Key3IkrDecryptIkrSettings));
}


BERR_Code SecureRsaTl_Key3IkrDecryptIkr(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_Key3IkrDecryptIkrSettings *pKey3IkrDecryptIkrSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(SecureRsaTl_Key3IkrDecryptIkr);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pKey3IkrDecryptIkrSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = pKey3IkrDecryptIkrSettings->encKeyLen;
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[0].data.ptr, pKey3IkrDecryptIkrSettings->encKey,
                container->blocks[0].len);

    if(pKey3IkrDecryptIkrSettings->encIvLen > 0)
    {
        container->blocks[1].len = pKey3IkrDecryptIkrSettings->encIvLen;
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr, pKey3IkrDecryptIkrSettings->encIv,
                    container->blocks[1].len);
    }

    container->basicIn[0] = pKey3IkrDecryptIkrSettings->ikrKeySlot;
    container->basicIn[1] = pKey3IkrDecryptIkrSettings->encKeySlot;
    container->basicIn[2] = pKey3IkrDecryptIkrSettings->variant;
    container->basicIn[3] = pKey3IkrDecryptIkrSettings->enableTransform;

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eKey3IkrDecryptIkr,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_Key3IkrDecryptIkr);
    return rc;
}


void SecureRsaTl_GetDefaultKpkDecryptRsaSettings(
    SecureRsaTl_KpkDecryptRsaSettings *pKpkDecryptRsaSettings)
{
    BDBG_ASSERT(pKpkDecryptRsaSettings);
    memset(pKpkDecryptRsaSettings, 0, sizeof(SecureRsaTl_KpkDecryptRsaSettings));
}


BERR_Code SecureRsaTl_KpkDecryptRsa(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_KpkDecryptRsaSettings *pKpkDecryptRsaSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t *ptr32;

    BDBG_ENTER(SecureRsaTl_KpkDecryptRsa);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pKpkDecryptRsaSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = SECURE_RSA_TL_KPK_DECRYPT_RSA_NUM_PARAMETERS * sizeof(uint32_t);
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    ptr32 = (uint32_t *) container->blocks[0].data.ptr;
    ptr32[0] = pKpkDecryptRsaSettings->rsaKeySlot;
    ptr32[1] = pKpkDecryptRsaSettings->encKeySlot;
    ptr32[2] = pKpkDecryptRsaSettings->variant;
    ptr32[3] = pKpkDecryptRsaSettings->sigKeySlot;
    ptr32[4] = pKpkDecryptRsaSettings->sigPadType;
    ptr32[5] = pKpkDecryptRsaSettings->sigDigestType;
    ptr32[6] = pKpkDecryptRsaSettings->sigPssSaltLen;

    container->blocks[1].len = pKpkDecryptRsaSettings->encKeySettingsLen;
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[1].data.ptr,
                pKpkDecryptRsaSettings->encKeySettings,
                container->blocks[1].len);

    container->blocks[2].len = pKpkDecryptRsaSettings->encKeyLen;
    container->blocks[2].data.ptr = SRAI_Memory_Allocate(container->blocks[2].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[2].data.ptr, pKpkDecryptRsaSettings->encKey,
                container->blocks[2].len);

    if(pKpkDecryptRsaSettings->encIvLen > 0)
    {
        container->blocks[3].len = pKpkDecryptRsaSettings->encIvLen;
        container->blocks[3].data.ptr = SRAI_Memory_Allocate(container->blocks[3].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[3].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[3].data.ptr,
                    pKpkDecryptRsaSettings->encIv,
                    container->blocks[3].len);
    }

    if(pKpkDecryptRsaSettings->signatureLen > 0)
    {
        container->blocks[4].len = pKpkDecryptRsaSettings->signatureLen;
        container->blocks[4].data.ptr = SRAI_Memory_Allocate(container->blocks[4].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[4].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[4].data.ptr,
                    pKpkDecryptRsaSettings->signature,
                    container->blocks[4].len);
    }

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eKpkDecryptRsa,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        if(container->blocks[2].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }

        if(container->blocks[3].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[3].data.ptr);
            container->blocks[3].data.ptr = NULL;
        }

        if(container->blocks[4].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[4].data.ptr);
            container->blocks[4].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_KpkDecryptRsa);
    return rc;
}


void SecureRsaTl_GetDefaultKpkDecryptIkrSettings(
    SecureRsaTl_KpkDecryptIkrSettings *pKpkDecryptIkrSettings)
{
    BDBG_ASSERT(pKpkDecryptIkrSettings);
    memset(pKpkDecryptIkrSettings, 0, sizeof(SecureRsaTl_KpkDecryptIkrSettings));
}


BERR_Code SecureRsaTl_KpkDecryptIkr(
    SecureRsaTl_Handle hSecureRsaTl,
    SecureRsaTl_KpkDecryptIkrSettings *pKpkDecryptIkrSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(SecureRsaTl_KpkDecryptIkr);

    BDBG_OBJECT_ASSERT(hSecureRsaTl, SecureRsaTl_P_Instance);

    if(pKpkDecryptIkrSettings == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }

    container->blocks[0].len = pKpkDecryptIkrSettings->encKeyLen;
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    BKNI_Memcpy(container->blocks[0].data.ptr, pKpkDecryptIkrSettings->encKey,
                container->blocks[0].len);

    if(pKpkDecryptIkrSettings->encIvLen > 0)
    {
        container->blocks[1].len = pKpkDecryptIkrSettings->encIvLen;
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr, pKpkDecryptIkrSettings->encIv,
                    container->blocks[1].len);
    }

    container->basicIn[0] = pKpkDecryptIkrSettings->ikrKeySlot;
    container->basicIn[1] = pKpkDecryptIkrSettings->encKeySlot;
    container->basicIn[2] = pKpkDecryptIkrSettings->variant;

    sage_rc = SRAI_Module_ProcessCommand(hSecureRsaTl->moduleHandle,
                                         SecureRsa_CommandId_eKpkDecryptIkr,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != 0)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(SecureRsaTl_KpkDecryptIkr);
    return rc;
}
