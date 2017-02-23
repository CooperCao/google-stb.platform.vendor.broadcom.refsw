/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#include <string.h> /* for strlen */

#include "utility_platform.h"
#include "utility_ids.h"
#include "rsa_tl.h"
#include "nexus_memory.h"

#define RSA_UTILITY_DRM_BIN_DEFAULT_FILEPATH "./drm.bin"
#define MAX_RSA_PUBLIC_KEY_SIZE             (512)
#define MAX_RSA_PUBLIC_EXPONENT_SIZE        (4)

#define GET_UPPER_16BIT(x) ((x&0xFFFF0000)>>16)
#define GET_LOWER_16BIT(x) ((x&0x0000FFFF))

BDBG_MODULE(rsa_tl);

struct RsaTl_P_Instance
{
    BDBG_OBJECT(RsaTl_P_Instance)
    SRAI_ModuleHandle moduleHandle;
};

BDBG_OBJECT_ID_DECLARE(RsaTl_P_Instance);
BDBG_OBJECT_ID(RsaTl_P_Instance);

static void _RsaTl_ContextDelete(RsaTl_Handle hRsaTl);
static RsaTl_Handle _RsaTl_ContextNew(const char * bin_file_path);

void
RsaTl_GetDefaultSettings(RsaSettings *pRsaModuleSettings)
{
    BDBG_ENTER(RsaTl_GetDefaultSettings);

    BDBG_ASSERT(pRsaModuleSettings);
    BKNI_Memset((uint8_t *)pRsaModuleSettings, 0x00, sizeof(RsaSettings));
    BKNI_Memcpy(pRsaModuleSettings->drm_binfile_path, RSA_UTILITY_DRM_BIN_DEFAULT_FILEPATH, strlen(RSA_UTILITY_DRM_BIN_DEFAULT_FILEPATH));

    BDBG_LEAVE(RsaTl_GetDefaultSettings);
    return;
}

static RsaTl_Handle
_RsaTl_ContextNew(const char * bin_file_path)
{
    BERR_Code magnum_rc;
    NEXUS_Error nexus_rc;
    BSAGElib_InOutContainer *container = NULL;
    RsaTl_Handle hRsaTl = NULL;

    nexus_rc = NEXUS_Memory_Allocate(sizeof(*hRsaTl), NULL, (void **)&hRsaTl);
    if (nexus_rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("%s - cannot allocate memory for RsaTl context", __FUNCTION__));
        magnum_rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }
    hRsaTl->moduleHandle = NULL;

    BDBG_OBJECT_SET(hRsaTl, RsaTl_P_Instance);

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        magnum_rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }


    /* Initialize SAGE Rsa module */
    magnum_rc = Utility_ModuleInit(Utility_ModuleId_eRsa,
                                   bin_file_path,
                                   container,
                                   &hRsaTl->moduleHandle);
    if(magnum_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error initializing Rsa TL module (0x%08x)", __FUNCTION__, container->basicOut[0]));
        goto ErrorExit;
    }

ErrorExit:
    if(container != NULL) {
        SRAI_Container_Free(container);
        container = NULL;
    }

    if (magnum_rc != BERR_SUCCESS && hRsaTl) {
        _RsaTl_ContextDelete(hRsaTl);
        hRsaTl = NULL;
    }

    return hRsaTl;
}

static void
_RsaTl_ContextDelete(RsaTl_Handle hRsaTl)
{
    if (hRsaTl)
    {
        if (hRsaTl->moduleHandle)
        {
            Utility_ModuleUninit(hRsaTl->moduleHandle);
            hRsaTl->moduleHandle = NULL;
        }

        BDBG_OBJECT_DESTROY(hRsaTl, RsaTl_P_Instance);
        NEXUS_Memory_Free(hRsaTl);
        hRsaTl = NULL;
    }
}

BERR_Code
RsaTl_Init(RsaTl_Handle *pRsaTlHandle,
           RsaSettings *pRsaModuleSettings)
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(RsaTl_Init);

    if(pRsaModuleSettings == NULL)
    {
        BDBG_ERR(("%s - Parameter settings are NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    if (pRsaTlHandle == NULL)
    {
        BDBG_ERR(("%s - Parameter pHandle is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    *pRsaTlHandle = _RsaTl_ContextNew(pRsaModuleSettings->drm_binfile_path);
    if (*pRsaTlHandle == NULL)
    {
        rc = BERR_OUT_OF_DEVICE_MEMORY;
        goto ErrorExit;
    }

ErrorExit:

    BDBG_LEAVE(RsaTl_Init);
    return rc;
}

void
RsaTl_Uninit(RsaTl_Handle hRsaTl)
{
    BDBG_ENTER(RsaTl_Uninit);

    BDBG_OBJECT_ASSERT(hRsaTl, RsaTl_P_Instance);
    _RsaTl_ContextDelete(hRsaTl);

    BDBG_LEAVE(RsaTl_Uninit);
    return;
}


BERR_Code
RsaTl_EncryptDecrypt(RsaTl_Handle hRsaTl,
                     uint32_t commandId,
                     uint8_t *pSrcData,
                     uint32_t srcLength,
                     uint8_t *pDstData,
                     uint32_t *pDstLength,
                     uint32_t padding,
                     uint32_t rsaKeyIndex)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_InOutContainer *container = NULL;
    uint32_t padding_actual;
    uint32_t padding_supplemental;

    BDBG_ENTER(RsaTl_EncryptDecrypt);

    BDBG_OBJECT_ASSERT(hRsaTl, RsaTl_P_Instance);

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }

    if (!pSrcData || !pDstData || !pDstLength)
    {
        BDBG_ERR(("%s - Invalid input", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }

    padding_actual = GET_LOWER_16BIT(padding);
    padding_supplemental = GET_UPPER_16BIT(padding);
    switch (commandId) {
    case Rsa_CommandId_ePublicDecrypt:
        BDBG_LOG(("%s: Decrypt data using Public key", __FUNCTION__));
        break;
    case Rsa_CommandId_ePublicEncrypt:
        BDBG_LOG(("%s: Encrypt data using Public key", __FUNCTION__));
        break;
    case Rsa_CommandId_ePrivateDecrypt:
        BDBG_LOG(("%s: Decrypt data using Private key", __FUNCTION__));
        break;
    case Rsa_CommandId_ePrivateEncrypt:
        BDBG_LOG(("%s: Encrypt data using Private key", __FUNCTION__));
        break;
    default:
        BDBG_ERR(("%s - Invalid command ID (%u)", __FUNCTION__, commandId));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }

    /* allocated memory for source */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(srcLength, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("\t allocate '%u' bytes of memory ^^^^^^^^^^^^^^^^^^^", srcLength));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }
    container->blocks[0].len = srcLength;
    BKNI_Memcpy(container->blocks[0].data.ptr, pSrcData, srcLength);

    /* allocated memory for destination */
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(*pDstLength, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("\t allocate '%u' bytes of memory ^^^^^^^^^^^^^^^^^^^", srcLength));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }
    container->blocks[1].len = *pDstLength;

    container->basicIn[0] = rsaKeyIndex;/* index of private key to sign */
    container->basicIn[1] = padding_actual;

    if(padding_actual == RsaTl_PaddingType_eOAEP)
    {
        /* padding_supplemental shall includes hash algorithm and below 2 is supported. */
        /* - BSAGElib_Crypto_ShaVariant_eSha256 */
        /* - BSAGElib_Crypto_ShaVariant_eSha1 */
        container->basicIn[2] = (int32_t)padding_supplemental;
    }

    sage_rc = SRAI_Module_ProcessCommand(hRsaTl->moduleHandle, commandId, container);
    if ((sage_rc != BERR_SUCCESS) || (container->basicOut[0] != 0))
    {
        BDBG_ERR(("\t Enc/Dec data ^^^^^^^^^^^^^^^^^^"));
        rc = sage_rc;
        goto handle_error;
    }

    BDBG_LOG(("%s - Result copied back to caller  ------------------", __FUNCTION__));
    /* how to return the info when returned dst size does not match src size*/
    BDBG_LOG(("%s: returned size: %u", __FUNCTION__, container->basicOut[2]));
    *pDstLength = container->basicOut[2];
    BKNI_Memcpy(pDstData, container->blocks[1].data.ptr, *pDstLength);

handle_error:

    if(container != NULL)
    {
        /* overwrite bin file */
        if (container->basicOut[1] != BERR_SUCCESS)
        {
            /* Display error code received from SAGE side */
            BDBG_LOG(("The following SAGE error occurred during the DRM binfile validation process (0x%08x):", container->basicOut[1]));
            BDBG_LOG(("\t%s\n", BSAGElib_Tools_ReturnCodeToString(container->basicOut[1])));
        }

        if(container->blocks[0].data.ptr != NULL)
        {
            BKNI_Memset(container->blocks[0].data.ptr, 0x00, container->blocks[0].len);
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            BKNI_Memset(container->blocks[1].data.ptr, 0x00, container->blocks[1].len);
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(RsaTl_EncryptDecrypt);
    return rc;
}

BERR_Code
RsaTl_SignVerify(RsaTl_Handle hRsaTl,
                 uint32_t commandId,
                 uint8_t *pSrcData,
                 uint32_t srcLength,
                 uint32_t rsaKeyIndex,
                 BSAGElib_Crypto_ShaVariant_e shaVariant,
                 uint8_t *pSignature,
                 uint32_t signatureLength)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    uint8_t *ptr = NULL;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(RsaTl_SignVerify);

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        rc = BSAGE_ERR_CONTAINER_REQUIRED;
        goto handle_error;
    }

    if(commandId == Rsa_CommandId_eSign)
    {
        BDBG_LOG(("%s - Signing operation --- data length = '%u'", __FUNCTION__, srcLength));
    }
    else if(commandId == Rsa_CommandId_eVerify)
    {
        BDBG_LOG(("%s - Verify operation ------------------", __FUNCTION__));
    }
    else
    {
        BDBG_ERR(("%s - Invalid command ID (%u)", __FUNCTION__, commandId));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }

    /* allocated memory for signature (max possible size is 384) */
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(signatureLength, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
   {
        BDBG_ERR(("\t### Cannot allocate '%u' bytes of memory ^^^^^^^^^^^^^^^^^^^", signatureLength));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }
    container->blocks[0].len = signatureLength;
    if(commandId == Rsa_CommandId_eVerify)
    {
        BKNI_Memcpy(container->blocks[0].data.ptr, pSignature, signatureLength);
    }
    else
    {
        BKNI_Memset(container->blocks[0].data.ptr, 0x00, signatureLength);
    }

    /* allocated memory for source data */
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(srcLength, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("\t### Cannot allocate '%u' bytes of memory ^^^^^^^^^^^^^^^^^^^", srcLength));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }
    container->blocks[1].len = srcLength;
    BKNI_Memcpy(container->blocks[1].data.ptr, pSrcData, srcLength);

    container->basicIn[0] = rsaKeyIndex;/* index of private key to sign */
    container->basicIn[1] = shaVariant;
    sage_rc = SRAI_Module_ProcessCommand(hRsaTl->moduleHandle, commandId, container);
    if ((sage_rc != BERR_SUCCESS) || (container->basicOut[0] != 0))
    {
        BDBG_ERR(("\t### Error Signing data ^^^^^^^^^^^^^^^^^^"));
        rc = sage_rc;
        goto handle_error;
    }

    ptr = container->blocks[0].data.ptr;
    BDBG_MSG(("\tComputed/verified signature = %02x %02x %02x %02x ...", ptr[0], ptr[1], ptr[2], ptr[3]));

    if(commandId == Rsa_CommandId_eSign)
    {
        BDBG_LOG(("%s - Signature copied back to caller  ------------------", __FUNCTION__));
        BKNI_Memcpy(pSignature, container->blocks[0].data.ptr, signatureLength);
    }

    BDBG_LOG(("\t*** command successfully returned from SAGE, analyzing result....\n"));

handle_error:

    if(container != NULL)
    {
        /* overwrite bin file */
        if (container->basicOut[1] != BERR_SUCCESS)
        {
            /* Display error code received from SAGE side */
            BDBG_LOG(("The following SAGE error occurred during the DRM binfile validation process (0x%08x):", container->basicOut[1]));
            BDBG_LOG(("\t%s", BSAGElib_Tools_ReturnCodeToString(container->basicOut[1])));
            BDBG_LOG(("\t%s\n", BSAGElib_Tools_ReturnCodeToString(container->basicOut[1])));
        }

        if(container->blocks[0].data.ptr != NULL)
        {
            BKNI_Memset(container->blocks[0].data.ptr, 0x00, container->blocks[0].len);
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }

        if(container->blocks[1].data.ptr != NULL)
        {
            BKNI_Memset(container->blocks[1].data.ptr, 0x00, container->blocks[1].len);
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }


        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(RsaTl_SignVerify);
    return rc;
}

BERR_Code
RsaTl_GetPublicKey(RsaTl_Handle hRsaTl,
                   uint32_t rsaKeyIndex,
                   uint8_t  *pModulus,
                   uint32_t *pModulusLength,
                   uint8_t  *pPublicExponent)
{
    BERR_Code sage_rc;
    BERR_Code rc = BERR_SUCCESS;
    uint8_t *ptr = NULL;
    BSAGElib_InOutContainer *container = NULL;

    BDBG_ENTER(RsaTl_GetPublicKey);

    BDBG_OBJECT_ASSERT(hRsaTl, RsaTl_P_Instance);

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }

    /* validate params */
    if (pModulus == NULL)
    {
        BDBG_ERR(("%s - Pointer to Modulus is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }

    if (pModulusLength == NULL)
    {
        BDBG_ERR(("%s - Pointer to Modulus length is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }

    if (pPublicExponent == NULL)
    {
        BDBG_ERR(("%s - Pointer to Public Exponent buffer is NULL", __FUNCTION__));
        rc = BERR_INVALID_PARAMETER;
        goto handle_error;
    }

    BDBG_LOG(("%s - Sending command to return public key for index '%u'", __FUNCTION__, rsaKeyIndex));

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(MAX_RSA_PUBLIC_KEY_SIZE, SRAI_MemoryType_Shared);
    if(container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate '%u' bytes of memory ^^^^^^^^^^^^^^^^^^^", __FUNCTION__, MAX_RSA_PUBLIC_KEY_SIZE));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }
    container->blocks[0].len = MAX_RSA_PUBLIC_KEY_SIZE;

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(MAX_RSA_PUBLIC_EXPONENT_SIZE, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Cannot allocate '%u' bytes of memory ^^^^^^^^^^^^^^^^^^^", __FUNCTION__, MAX_RSA_PUBLIC_EXPONENT_SIZE));
        rc = BERR_OUT_OF_SYSTEM_MEMORY;
        goto handle_error;
    }
    container->blocks[1].len = MAX_RSA_PUBLIC_EXPONENT_SIZE;

    container->basicIn[0] = rsaKeyIndex;
    sage_rc = SRAI_Module_ProcessCommand(hRsaTl->moduleHandle, Rsa_CommandId_eGetPublicKey, container);
    if ((sage_rc != BERR_SUCCESS) || (container->basicOut[0] != 0))
    {
        BDBG_ERR(("%s - Error fetching public key info data fo rindex '%u'", __FUNCTION__, rsaKeyIndex));
        rc = sage_rc;
        goto handle_error;
    }

    BKNI_Memcpy(pModulus, container->blocks[0].data.ptr, container->blocks[0].len);
    (*pModulusLength)= container->basicOut[1];
    ptr = container->blocks[0].data.ptr;
    BDBG_LOG(("%s - public modulus returned (len = %u) = %02x %02x %02x %02x ...", __FUNCTION__, container->basicOut[1], ptr[0], ptr[1], ptr[2], ptr[3]));

    ptr = container->blocks[1].data.ptr;
    BKNI_Memcpy(pPublicExponent, container->blocks[1].data.ptr, MAX_RSA_PUBLIC_EXPONENT_SIZE);
    BDBG_LOG(("%s - public exponent returned (len = 4) = %02x %02x %02x %02x ...", __FUNCTION__, ptr[0], ptr[1], ptr[2], ptr[3]));

handle_error:

    if(container != NULL)
    {
        if(container->blocks[0].data.ptr != NULL)
        {
            BKNI_Memset(container->blocks[0].data.ptr, 0x00, container->blocks[0].len);
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if(container->blocks[1].data.ptr != NULL)
        {
            BKNI_Memset(container->blocks[1].data.ptr, 0x00, container->blocks[1].len);
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }

        SRAI_Container_Free(container);
        container = NULL;
    }

    BDBG_LEAVE(RsaTl_GetPublicKey);
    return rc;
}
