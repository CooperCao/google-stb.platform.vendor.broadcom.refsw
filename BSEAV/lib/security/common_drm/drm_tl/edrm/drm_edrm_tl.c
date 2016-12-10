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

#include "bstd.h"
#include "bdbg.h"

#include "drm_common.h"
#include "drm_common_swcrypto.h"
#include "drm_common_tl.h"
#include "drm_common_command_ids.h"
#include "drm_edrm_tl.h"
#include "drm_data.h"

#include "bsagelib_types.h"
#include "sage_srai.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

BDBG_MODULE(drm_edrm_tl);

#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define DUMP_SIZE 32
#define DUMP_EDRM_DATA(string,data,size) {\
    char tmp[DUMP_SIZE*4]={0};\
    uint32_t i=0, j=0;\
    BDBG_LOG(("%s",string));\
    BDBG_LOG(("--------------------------------"));\
    while( i<size ) {\
        if(i !=0 && i%DUMP_SIZE == 0) {j=0; BDBG_LOG(("%s",tmp));}\
        sprintf(tmp+3*j++," %02x", data[i++]);}\
    BDBG_LOG(("%s",tmp));\
    BDBG_LOG(("--------------------------------"));\
}

typedef struct DRM_DrmEdrmTl_P_Context_s {
    SRAI_ModuleHandle moduleHandle;
}DRM_DrmEdrmTl_P_Context_t;

DrmRC
DRM_EdrmTl_Initialize(
    char *key_file,
    DRM_EdrmTlHandle *hEdrmTl)
{
    DrmRC rc = Drm_Success;
    DrmCommonInit_TL_t drmCmnInit;
    DRM_DrmEdrmTl_P_Context_t        *handle=NULL;
    BSAGElib_InOutContainer *container = NULL;
    ChipType_e chip_type;

    BDBG_ENTER(DRM_EdrmTl_Initialize);
    if ( !hEdrmTl || !key_file )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", __FUNCTION__  ));
        goto ErrorExit;
    }

    rc = DRM_Common_MemoryAllocate((uint8_t**)&handle, sizeof(DRM_DrmEdrmTl_P_Context_t));
    if (rc != Drm_Success || handle == NULL)
    {
        rc = Drm_MemErr;
        BDBG_ERR(("%s -  Error Allocating drm Memory for context", __FUNCTION__));
        goto ErrorExit;
    }
    drmCmnInit.drmCommonInit.heap = NULL;
#ifdef USE_UNIFIED_COMMON_DRM
    drmCmnInit.drmType = 0;
#else
    drmCmnInit.drmType = BSAGElib_BinFileDrmType_eEdrm;
#endif
    chip_type = DRM_Common_GetChipType();
#if USE_UNIFIED_COMMON_DRM
    if(chip_type == ChipType_eZS)
    {
        drmCmnInit.ta_bin_file_path = bdrm_get_ta_dev_bin_file_path();
    }
    else
    {
        drmCmnInit.ta_bin_file_path = bdrm_get_ta_bin_file_path();
    }
#else
    if(chip_type == ChipType_eZS)
    {
        drmCmnInit.ta_bin_file_path = bdrm_get_ta_edrm_dev_bin_file_path();
    }
    else
    {
        drmCmnInit.ta_bin_file_path = bdrm_get_ta_edrm_bin_file_path();
    }
#endif
    BDBG_MSG(("%s TA bin file %s ",__FUNCTION__, drmCmnInit.ta_bin_file_path));

    rc = DRM_Common_TL_Initialize(&drmCmnInit);
    if (rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module", __FUNCTION__));
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    /* Initialize SAGE Edrm module */
    handle->moduleHandle = NULL;
#if USE_UNIFIED_COMMON_DRM
    rc = DRM_Common_TL_ModuleInitialize(DrmCommon_ModuleId_eEdrm,(char *)key_file, container, &(handle->moduleHandle));
#else
    rc = DRM_Common_TL_ModuleInitialize_TA(Common_Platform_eDrm, Edrm_ModuleId_eDRM, (char *)key_file, container, &(handle->moduleHandle));
#endif
    if (rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module (0x%08x)", __FUNCTION__, container->basicOut[0]));
        goto ErrorExit;
    }

    *hEdrmTl = (DRM_EdrmTlHandle)handle;
    handle = NULL;

ErrorExit:
    if (container != NULL)
    {
        SRAI_Container_Free(container);
    }
    if (handle != NULL)
    {
        DRM_Common_MemoryFree((uint8_t *)handle);
    }

    BDBG_LEAVE(DRM_EdrmTl_Initialize);
    return rc;
}

void DRM_EdrmTl_Finalize(DRM_EdrmTlHandle hEdrmTl)
{
    if ( !hEdrmTl )
    {
        BDBG_ERR(("%s Invalid Parameter ", __FUNCTION__  ));
        return;
    }

    DRM_DrmEdrmTl_P_Context_t  *ctx = (DRM_DrmEdrmTl_P_Context_t *) hEdrmTl;

    if (ctx != NULL)
    {

#ifdef USE_UNIFIED_COMMON_DRM
        DRM_Common_TL_ModuleFinalize(ctx->moduleHandle);
#else
        DRM_Common_TL_ModuleFinalize_TA(Common_Platform_eDrm,ctx->moduleHandle);
#endif
        DRM_Common_MemoryFree((uint8_t *)ctx);
#ifdef USE_UNIFIED_COMMON_DRM
        DRM_Common_TL_Finalize();
#else
        DRM_Common_TL_Finalize_TA(Common_Platform_eDrm);
#endif
    }
}

DrmRC DRM_EdrmTl_GetDeviceCertificate(
    DRM_EdrmTlHandle hEdrmTl,
    uint8_t *cert,
    uint32_t certLength
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hEdrmTl || !cert )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", __FUNCTION__  ));
        goto ErrorExit;
    }

    DRM_DrmEdrmTl_P_Context_t *pCtx = (DRM_DrmEdrmTl_P_Context_t *)hEdrmTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    if (certLength != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(certLength, SRAI_MemoryType_Shared);
        if (container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating buffer for certificate", __FUNCTION__));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[0].len = certLength;
    }

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmEdrmTl_CommandId_GetDeviceCertificate,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during GetDeviceCertificate operation", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if (rc == BERR_SUCCESS) {
        BKNI_Memcpy(cert, container->blocks[0].data.ptr, container->blocks[0].len);

#ifdef B_DRM_EDRM_DEBUG
        DUMP_EDRM_DATA("nCert: ", cert, container->blocks[0].len);
#endif
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", __FUNCTION__, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

DrmRC DRM_EdrmTl_VerifyPublicKey(
    DRM_EdrmTlHandle hEdrmTl,
    uint32_t *result
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;
    DrmCommon_RsaKey_t  rsa_key;
    X509* pCertificate = NULL;
    uint8_t *pPublicKey = NULL, *pCert = NULL;

    *result = 0; /* set result as invalid at first */

    if ( !hEdrmTl )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", __FUNCTION__  ));
        goto ErrorExit;
    }

    DRM_DrmEdrmTl_P_Context_t *pCtx = (DRM_DrmEdrmTl_P_Context_t *)hEdrmTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(MAX(EDRM_CERT_SIZE,EDRM_PUBLIC_KEY_SIZE), SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating buffer for certificate and pubkey", __FUNCTION__));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    /* Get pubkey from eDRM data section */
    container->blocks[0].len = EDRM_PUBLIC_KEY_SIZE;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->moduleHandle, DrmEdrmTl_CommandId_GetDevicePublicKey, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during GetDevicePublicKey operation", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }
    rc = container->basicOut[0];
    if (rc == BERR_SUCCESS && container->blocks[0].len == EDRM_PUBLIC_KEY_SIZE) {
        rc = DRM_Common_MemoryAllocate(&pPublicKey, container->blocks[0].len);
        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error allocating buffer for pubkey", __FUNCTION__));
            goto ErrorExit;
        }
        BKNI_Memcpy(pPublicKey, container->blocks[0].data.ptr, container->blocks[0].len);

#ifdef B_DRM_EDRM_DEBUG
        DUMP_EDRM_DATA("PubKey: ", pPublicKey, container->blocks[0].len);
#endif
    }
    else
    {
        BDBG_ERR(("%s - GetDevicePublicKey was sent succuessfully but actual operation failed (0x%08x)", __FUNCTION__, rc));
        goto ErrorExit;
    }

    /* Get cert from eDRM data section */
    container->blocks[0].len = EDRM_CERT_SIZE;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->moduleHandle, DrmEdrmTl_CommandId_GetDeviceCertificate, container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during GetDeviceCertificate operation", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = container->basicOut[0];
    if (rc == BERR_SUCCESS && container->blocks[0].len == EDRM_CERT_SIZE) {
        rc = DRM_Common_MemoryAllocate(&pCert, container->blocks[0].len);
        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error allocating buffer for cert", __FUNCTION__));
            goto ErrorExit;
        }

        BKNI_Memcpy(pCert, container->blocks[0].data.ptr, container->blocks[0].len);

#ifdef B_DRM_EDRM_DEBUG
        DUMP_EDRM_DATA("nCert: ", pCert, container->blocks[0].len);
#endif

        rc = DRM_Common_Swx509ASN1DerDecode(pCert, container->blocks[0].len, &pCertificate);
        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error converting cert to internal form", __FUNCTION__));
            goto ErrorExit;
        }

        rc = DRM_Common_Swx509GetRsaPublicKey(pCertificate, &rsa_key);
        if (rc != Drm_Success)
        {
            BDBG_ERR(("%s - Error extracting pubkey from cert", __FUNCTION__));
            goto ErrorExit;
        }

#ifdef B_DRM_EDRM_DEBUG
        BDBG_LOG(("modulus_size=%d pub_exponent_size=%d", rsa_key.n.len, rsa_key.e.len));
        DUMP_EDRM_DATA("nModulus: ", rsa_key.n.pData, rsa_key.n.len);
        DUMP_EDRM_DATA("nExponent: ", rsa_key.e.pData, rsa_key.e.len);
#endif


    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", __FUNCTION__, rc));
        goto ErrorExit;
    }

    if(rsa_key.n.len == EDRM_PUBLIC_KEY_SIZE)
        if(BKNI_Memcmp(pPublicKey, rsa_key.n.pData, rsa_key.n.len) == 0)
        {
            *result = 1; /* pubkey is valid */
        }

ErrorExit:
    if (pCertificate)
        DRM_Common_Swx509Free(pCertificate);
    if (pPublicKey)
        DRM_Common_MemoryFree(pPublicKey);
    if (pCert)
        DRM_Common_MemoryFree(pCert);
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}
