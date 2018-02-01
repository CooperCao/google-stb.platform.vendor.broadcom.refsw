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
#include <string.h>

#include "nexus_memory.h"

#include "bsagelib_types.h"
#include "bsagelib_crypto_types.h"

#include "utility_platform.h"
#include "utility_ids.h"
#include "ssce_tl.h"


BDBG_MODULE(ssce_tl);


struct SsceTl_P_Instance
{
    BDBG_OBJECT(SsceTl_P_Instance)
    SRAI_ModuleHandle moduleHandle;
};

BDBG_OBJECT_ID_DECLARE(SsceTl_P_Instance);
BDBG_OBJECT_ID(SsceTl_P_Instance);


static void _SsceTl_ContextDelete(
    SsceTl_Handle hSsceTl)
{
    if(hSsceTl)
    {
        if(hSsceTl->moduleHandle)
        {
            Utility_ModuleUninit(hSsceTl->moduleHandle);
            hSsceTl->moduleHandle = NULL;
        }

        BDBG_OBJECT_DESTROY(hSsceTl, SsceTl_P_Instance);
        NEXUS_Memory_Free(hSsceTl);
        hSsceTl = NULL;
    }
}


static SsceTl_Handle _SsceTl_ContextNew(
    void)
{
    BERR_Code magnum_rc;
    NEXUS_Error nexus_rc;
    BSAGElib_InOutContainer *container = NULL;
    SsceTl_Handle hSsceTl = NULL;

    nexus_rc = NEXUS_Memory_Allocate(sizeof(*hSsceTl), NULL,
                                     (void **)&hSsceTl);
    if(nexus_rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        magnum_rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }
    hSsceTl->moduleHandle = NULL;

    BDBG_OBJECT_SET(hSsceTl, SsceTl_P_Instance);

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        magnum_rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }

    /* Initialize SSCE module */
    magnum_rc = Utility_ModuleInit(Utility_ModuleId_eSsce,
                                   NULL,
                                   container,
                                   &hSsceTl->moduleHandle);
    if(magnum_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error initializing SSCE TL module (0x%08x)",
                  BSTD_FUNCTION, container->basicOut[0]));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL)
    {
        SRAI_Container_Free(container);
        container = NULL;
    }

    if((magnum_rc != BERR_SUCCESS) && hSsceTl)
    {
        _SsceTl_ContextDelete(hSsceTl);
        hSsceTl = NULL;
    }

    return hSsceTl;
}


BERR_Code SsceTl_Init(
    SsceTl_Handle *pSsceTlHandle)
{
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER(SsceTl_Init);

    if(pSsceTlHandle == NULL)
    {
        BDBG_ERR(("%s - NULL parameter", BSTD_FUNCTION));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    *pSsceTlHandle = _SsceTl_ContextNew();
    if(*pSsceTlHandle == NULL)
    {
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }

    rc = BERR_SUCCESS;

ErrorExit:
    BDBG_LEAVE(SsceTl_Init);
    return rc;
}


void SsceTl_Uninit(
    SsceTl_Handle hSsceTl)
{
    BDBG_ENTER(SsceTl_Uninit);

    BDBG_OBJECT_ASSERT(hSsceTl, SsceTl_P_Instance);
    _SsceTl_ContextDelete(hSsceTl);

    BDBG_LEAVE(SsceTl_Uninit);
}


void SsceTl_GetDefaultCreateKeySettings(
    SsceTl_CreateKeySettings *pCreateKeySettings)
{
    BDBG_ASSERT(pCreateKeySettings);
    memset(pCreateKeySettings, 0, sizeof(SsceTl_CreateKeySettings));

    pCreateKeySettings->rsaSize = SsceTl_RsaSize_e2048;
    pCreateKeySettings->rsaPublicExponent = RSA_PUBLIC_EXPONENT_DEFAULT;
    pCreateKeySettings->eccCurve = SsceTl_EccCurve_eprime256v1;
}


BERR_Code SsceTl_CreateKey(
    SsceTl_Handle hSsceTl,
    SsceTl_CreateKeySettings *pCreateKeySettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_UNKNOWN;
    BSAGElib_InOutContainer *container = NULL;


    BDBG_ENTER(SsceTl_CreateKey);

    BDBG_OBJECT_ASSERT(hSsceTl, SsceTl_P_Instance);

    if((pCreateKeySettings == NULL) ||
       (pCreateKeySettings->label == NULL) ||
       (pCreateKeySettings->keyContainerLen == NULL))
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

    container->blocks[0].len = 1 + strlen(pCreateKeySettings->label);
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    strcpy((char *) container->blocks[0].data.ptr, pCreateKeySettings->label);

    if((*(pCreateKeySettings->keyContainerLen) > 0) &&
       (pCreateKeySettings->keyContainer != NULL))
    {
        container->blocks[1].len = *(pCreateKeySettings->keyContainerLen);
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
    }

    container->basicIn[0] = pCreateKeySettings->keyType;
    container->basicIn[1] = pCreateKeySettings->rsaSize;
    container->basicIn[2] = pCreateKeySettings->rsaPublicExponent;
    container->basicIn[3] = pCreateKeySettings->eccCurve;

    sage_rc = SRAI_Module_ProcessCommand(hSsceTl->moduleHandle,
                                         Ssce_CommandId_eCreateKey,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

    if((*(pCreateKeySettings->keyContainerLen) > 0) &&
       (pCreateKeySettings->keyContainer != NULL))
    {
        BKNI_Memcpy(pCreateKeySettings->keyContainer,
                    container->blocks[1].data.ptr,
                    container->basicOut[1]);
    }

    *(pCreateKeySettings->keyContainerLen) = container->basicOut[1];

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

    BDBG_LEAVE(SsceTl_CreateKey);
    return rc;
}


void SsceTl_GetDefaultLoadKeySettings(
    SsceTl_LoadKeySettings *pLoadKeySettings)
{
    BDBG_ASSERT(pLoadKeySettings);
    memset(pLoadKeySettings, 0, sizeof(SsceTl_LoadKeySettings));
}


BERR_Code SsceTl_LoadKey(
    SsceTl_Handle hSsceTl,
    SsceTl_LoadKeySettings *pLoadKeySettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_UNKNOWN;
    BSAGElib_InOutContainer *container = NULL;


    BDBG_ENTER(SsceTl_LoadKey);

    BDBG_OBJECT_ASSERT(hSsceTl, SsceTl_P_Instance);

    if((pLoadKeySettings == NULL) ||
       (pLoadKeySettings->label == NULL))
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

    container->blocks[0].len = 1 + strlen(pLoadKeySettings->label);
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    strcpy((char *) container->blocks[0].data.ptr, pLoadKeySettings->label);

    if((pLoadKeySettings->keyContainerLen > 0) &&
       (pLoadKeySettings->keyContainer != NULL))
    {
        container->blocks[1].len = pLoadKeySettings->keyContainerLen;
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr,
                    pLoadKeySettings->keyContainer,
                    container->blocks[1].len);
    }

    sage_rc = SRAI_Module_ProcessCommand(hSsceTl->moduleHandle,
                                         Ssce_CommandId_eLoadKey,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != BERR_SUCCESS)
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

    BDBG_LEAVE(SsceTl_LoadKey);
    return rc;
}


void SsceTl_GetDefaultUpdateCertificateSettings(
    SsceTl_UpdateCertificateSettings *pUpdateCertificateSettings)
{
    BDBG_ASSERT(pUpdateCertificateSettings);
    memset(pUpdateCertificateSettings, 0, sizeof(SsceTl_UpdateCertificateSettings));
}


BERR_Code SsceTl_UpdateCertificate(
    SsceTl_Handle hSsceTl,
    SsceTl_UpdateCertificateSettings *pUpdateCertificateSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_UNKNOWN;
    BSAGElib_InOutContainer *container = NULL;


    BDBG_ENTER(SsceTl_UpdateCertificate);

    BDBG_OBJECT_ASSERT(hSsceTl, SsceTl_P_Instance);

    if((pUpdateCertificateSettings == NULL) ||
       (pUpdateCertificateSettings->label == NULL) ||
       (pUpdateCertificateSettings->keyContainerLen == NULL))
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

    container->blocks[0].len = 1 + strlen(pUpdateCertificateSettings->label);
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    strcpy((char *) container->blocks[0].data.ptr, pUpdateCertificateSettings->label);

    if((pUpdateCertificateSettings->certificateLen > 0) &&
       (pUpdateCertificateSettings->certificate != NULL))
    {
        container->blocks[1].len = pUpdateCertificateSettings->certificateLen;
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr,
                    pUpdateCertificateSettings->certificate,
                    container->blocks[1].len);
    }

    if((*(pUpdateCertificateSettings->keyContainerLen) > 0) &&
       (pUpdateCertificateSettings->keyContainer != NULL))
    {
        container->blocks[2].len = *(pUpdateCertificateSettings->keyContainerLen);
        container->blocks[2].data.ptr = SRAI_Memory_Allocate(container->blocks[2].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[2].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
    }

    sage_rc = SRAI_Module_ProcessCommand(hSsceTl->moduleHandle,
                                         Ssce_CommandId_eUpdateCertificate,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

    if((*(pUpdateCertificateSettings->keyContainerLen) > 0) &&
       (pUpdateCertificateSettings->keyContainer != NULL))
    {
        BKNI_Memcpy(pUpdateCertificateSettings->keyContainer,
                    container->blocks[2].data.ptr,
                    container->basicOut[1]);
    }

    *(pUpdateCertificateSettings->keyContainerLen) = container->basicOut[1];

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

    BDBG_LEAVE(SsceTl_UpdateCertificate);
    return rc;
}


void SsceTl_GetDefaultRetrieveSettings(
    SsceTl_RetrieveSettings *pRetrieveSettings)
{
    BDBG_ASSERT(pRetrieveSettings);
    memset(pRetrieveSettings, 0, sizeof(SsceTl_RetrieveSettings));
}


BERR_Code SsceTl_Retrieve(
    SsceTl_Handle hSsceTl,
    SsceTl_RetrieveSettings *pRetrieveSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_UNKNOWN;
    BSAGElib_InOutContainer *container = NULL;


    BDBG_ENTER(SsceTl_Retrieve);

    BDBG_OBJECT_ASSERT(hSsceTl, SsceTl_P_Instance);

    if((pRetrieveSettings == NULL) ||
       (pRetrieveSettings->label == NULL) ||
       (pRetrieveSettings->outputLen == NULL))
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

    container->blocks[0].len = 1 + strlen(pRetrieveSettings->label);
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    strcpy((char *) container->blocks[0].data.ptr, pRetrieveSettings->label);

    if((*(pRetrieveSettings->outputLen) > 0) &&
       (pRetrieveSettings->output != NULL))
    {
        container->blocks[1].len = *(pRetrieveSettings->outputLen);
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
    }

    container->basicIn[0] = pRetrieveSettings->retrieveType;

    sage_rc = SRAI_Module_ProcessCommand(hSsceTl->moduleHandle,
                                         Ssce_CommandId_eRetrieve,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

    if((*(pRetrieveSettings->outputLen) > 0) &&
       (pRetrieveSettings->output != NULL))
    {
        BKNI_Memcpy(pRetrieveSettings->output, container->blocks[1].data.ptr,
                    container->basicOut[1]);
    }

    *(pRetrieveSettings->outputLen) = container->basicOut[1];

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

    BDBG_LEAVE(SsceTl_Retrieve);
    return rc;
}


void SsceTl_GetDefaultSignSettings(
    SsceTl_SignSettings *pSignSettings)
{
    BDBG_ASSERT(pSignSettings);
    memset(pSignSettings, 0, sizeof(SsceTl_SignSettings));


    pSignSettings->shaVariant = BSAGElib_Crypto_ShaVariant_eSha256;
    pSignSettings->rsaSignAlgorithm = SsceTl_RsaSignAlgorithm_ePss;
    pSignSettings->eccSignAlgorithm = SsceTl_EccSignAlgorithm_eEcdsa;
}


BERR_Code SsceTl_Sign(
    SsceTl_Handle hSsceTl,
    SsceTl_SignSettings *pSignSettings)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_UNKNOWN;
    BSAGElib_InOutContainer *container = NULL;


    BDBG_ENTER(SsceTl_Sign);

    BDBG_OBJECT_ASSERT(hSsceTl, SsceTl_P_Instance);

    if((pSignSettings == NULL) ||
       (pSignSettings->label == NULL) ||
       (pSignSettings->signatureLen == NULL))
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

    container->blocks[0].len = 1 + strlen(pSignSettings->label);
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(container->blocks[0].len,
                                                         SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto ErrorExit;
    }
    strcpy((char *) container->blocks[0].data.ptr, pSignSettings->label);

    if((pSignSettings->digestLen > 0) &&
       (pSignSettings->digest != NULL))
    {
        container->blocks[1].len = pSignSettings->digestLen;
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(container->blocks[1].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
        BKNI_Memcpy(container->blocks[1].data.ptr, pSignSettings->digest,
                    container->blocks[1].len);
    }

    if((*(pSignSettings->signatureLen) > 0) &&
       (pSignSettings->signature != NULL))
    {
        container->blocks[2].len = *(pSignSettings->signatureLen);
        container->blocks[2].data.ptr = SRAI_Memory_Allocate(container->blocks[2].len,
                                                             SRAI_MemoryType_Shared);
        if(container->blocks[2].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Cannot allocate memory", BSTD_FUNCTION));
            rc = BERR_OUT_OF_SYSTEM_MEMORY;
            goto ErrorExit;
        }
    }

    container->basicIn[0] = pSignSettings->shaVariant;
    container->basicIn[1] = pSignSettings->rsaSignAlgorithm;
    container->basicIn[2] = pSignSettings->rsaPssSaltLen;
    container->basicIn[3] = pSignSettings->eccSignAlgorithm;

    sage_rc = SRAI_Module_ProcessCommand(hSsceTl->moduleHandle, Ssce_CommandId_eSign,
                                         container);
    if(sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during operation", BSTD_FUNCTION));
        rc = BERR_UNKNOWN;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if(rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Command was sent successfully but actual operation failed (0x%08x)",
                  BSTD_FUNCTION, rc));
        goto ErrorExit;
    }

    if((*(pSignSettings->signatureLen) > 0) &&
       (pSignSettings->signature != NULL))
    {
        BKNI_Memcpy(pSignSettings->signature,
                    container->blocks[2].data.ptr,
                    container->basicOut[1]);
    }

    *(pSignSettings->signatureLen) = container->basicOut[1];

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

    BDBG_LEAVE(SsceTl_Sign);
    return rc;
}
