/***************************************************************************
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
 **************************************************************************/

#include "bstd.h"
#include "bdbg.h"
#include "nexus_base_os.h"

#include "drm_common.h"
#include "drm_data.h"
#include "drm_common_tl.h"
#include "drm_common_command_ids.h"
#include "drm_dtcp_ip_tl.h"

#include "bsagelib_types.h"
#include "sage_srai.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

BDBG_MODULE(drm_dtcpip_tl);

#define HOST_ACCESSIBLE_HEAP 1 /* Make this 0 when running in SVP mode, since HOST does not have access to XRR heap */

#define DTCP_AES_KEY_SIZE  16
#define DTCP_AES_IV_SIZE   16

#define DUMP_DTCP_DATA(string,data,size) {        \
   char tmp[4096*2]= "\0";                          \
   uint32_t i=0, l=strlen(string);               \
   sprintf(tmp,"%s",string);                     \
   while( i<size && l < 4096*2) {                   \
    sprintf(tmp+l," %02x", data[i]); ++i; l+=3;} \
   /*printf(tmp); printf("\n");*/                \
   BDBG_LOG((tmp));                              \
}

typedef struct DRM_DrmDtcpIpTl_P_Context_s {
    SRAI_ModuleHandle moduleHandle;
}DRM_DrmDtcpIpTl_P_Context_t;

DrmRC
DRM_DtcpIpTl_Initialize(
    char *key_file,
    uint32_t mode,
    DRM_DtcpIpTlHandle *hDtcpIpTl)
{
    DrmRC rc = Drm_Success;
    DrmCommonInit_TL_t drmCmnInit;
    DRM_DrmDtcpIpTl_P_Context_t        *handle=NULL;
    BSAGElib_InOutContainer *container = NULL;
    ChipType_e chip_type;

    BDBG_ENTER(DRM_DtcpIpTl_Initialize);
    if ( !hDtcpIpTl || !key_file )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    rc = DRM_Common_MemoryAllocate((uint8_t**)&handle, sizeof(DRM_DrmDtcpIpTl_P_Context_t));
    if (rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module", BSTD_FUNCTION));
        goto ErrorExit;
    }

    if (handle == NULL)
    {
        rc = Drm_MemErr;
        BDBG_ERR(("%s -  Error Allocating drm Memory for context", BSTD_FUNCTION));
        goto ErrorExit;
    }
    drmCmnInit.drmCommonInit.heap = NULL;
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
        drmCmnInit.ta_bin_file_path = bdrm_get_ta_dtcp_dev_bin_file_path();
    }
    else
    {
        drmCmnInit.ta_bin_file_path = bdrm_get_ta_dtcp_bin_file_path();
    }
#endif

    BDBG_MSG(("%s TA bin file %s ",BSTD_FUNCTION, drmCmnInit.ta_bin_file_path));
#ifdef USE_UNIFIED_COMMON_DRM
    drmCmnInit.drmType = 0;
#else
    drmCmnInit.drmType = BSAGElib_BinFileDrmType_eDtcpIp;
#endif

    rc = DRM_Common_TL_Initialize(&drmCmnInit);
    if (rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module", BSTD_FUNCTION));
        goto ErrorExit;
    }

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->basicIn[1] = mode;

    /* Initialize SAGE Dtcp-IP module */
    handle->moduleHandle = NULL;
#ifdef USE_UNIFIED_COMMON_DRM
    rc = DRM_Common_TL_ModuleInitialize(DrmCommon_ModuleId_eDtcpIp, (char *)key_file, container, &(handle->moduleHandle));
#else
    rc = DRM_Common_TL_ModuleInitialize_TA(Common_Platform_DtcpIp, DtcpIp_ModuleId_eDRM, (char *)key_file, container, &(handle->moduleHandle));
#endif
    if (rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module (0x%08x)", BSTD_FUNCTION, container->basicOut[0]));
        goto ErrorExit;
    }

    *hDtcpIpTl = (DRM_DtcpIpTlHandle)handle;
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

    BDBG_LEAVE(DRM_DtcpIpTl_Initialize);
    return rc;
}

void DRM_DtcpIpTl_Finalize(DRM_DtcpIpTlHandle hDtcpIpTl)
{
    if ( !hDtcpIpTl )
    {
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        return;
    }

    DRM_DrmDtcpIpTl_P_Context_t  *ctx = (DRM_DrmDtcpIpTl_P_Context_t *) hDtcpIpTl;

    if (ctx != NULL)
    {
#ifdef USE_UNIFIED_COMMON_DRM
        DRM_Common_TL_ModuleFinalize(ctx->moduleHandle);
#else
        DRM_Common_TL_ModuleFinalize_TA(Common_Platform_DtcpIp, ctx->moduleHandle);
#endif
        DRM_Common_MemoryFree((uint8_t *)ctx);
#ifdef USE_UNIFIED_COMMON_DRM
        DRM_Common_TL_Finalize();
#else
        DRM_Common_TL_Finalize_TA(Common_Platform_DtcpIp);
#endif
    }
}

DrmRC DRM_DtcpIpTl_GetRNG(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t *r,
    uint32_t len
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !r )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    if (len != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(len, SRAI_MemoryType_Shared);
        if (container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[0].len = len;
    }

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_GetRNG,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during GetRNG operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        if (r != NULL)
        {
            BKNI_Memcpy(r, container->blocks[0].data.ptr, container->blocks[0].len);
        }
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
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

DrmRC DRM_DtcpIpTl_GetRNGMax(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t *r,
    uint8_t *max,
    uint32_t len
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !r || !max)
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    if (len != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(len, SRAI_MemoryType_Shared);
        if (container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[0].len = len;
        BKNI_Memcpy(container->blocks[0].data.ptr, r, len);
    }

    if (len != 0)
    {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(len, SRAI_MemoryType_Shared);
        if (container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[1].len = len;
        BKNI_Memcpy(container->blocks[1].data.ptr, max, len);
    }

    container->basicIn[0] = len;

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_GetRNGMax,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during GetRNG operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        if (r != NULL)
        {
            BKNI_Memcpy(r, container->blocks[0].data.ptr, container->blocks[0].len);
        }
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

DrmRC DRM_DtcpIpTl_GetDeviceCertificate(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t *cert,
    uint32_t certLength,
    uint8_t *dtlaPublicKey
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !cert || !dtlaPublicKey)
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    if (certLength != 0)
    {
        container->blocks[0].data.ptr = SRAI_Memory_Allocate(certLength, SRAI_MemoryType_Shared);
        if (container->blocks[0].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[0].len = certLength;
    }

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(DTCP_PUBLIC_KEY_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[1].len = DTCP_PUBLIC_KEY_SIZE;

    container->basicIn[0] = certLength;

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_GetDeviceCertificate,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during GetRNG operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        if (cert != NULL && dtlaPublicKey != NULL)
        {
            BKNI_Memcpy(cert, container->blocks[0].data.ptr, container->blocks[0].len);
            BKNI_Memcpy(dtlaPublicKey, container->blocks[1].data.ptr, container->blocks[1].len);
        }
#ifdef B_DRM_DTCP_DEBUG
        DUMP_DTCP_DATA("Certificate before copy in TL : ", container->blocks[0].data.ptr, container->blocks[0].len);
        DUMP_DTCP_DATA("Certificate after copy in TL : ", cert, container->blocks[0].len);
#endif
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

DrmRC DtcpIpTl_EncDecOperation(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t *pSrc,
    uint32_t src_length,
    uint8_t *pDst,
    NEXUS_KeySlotHandle dtcpIpKeyHandle,
    bool scatterGatherStart,
    bool scatterGatherEnd,
    NEXUS_DmaHandle dtcpIpDmaHandle
    )
{
    DrmRC rc = Drm_Success;
    NEXUS_Error retCode = NEXUS_SUCCESS;
    NEXUS_DmaJobSettings  dmaJobSetting;
    NEXUS_DmaJobBlockSettings blockSettings;
    NEXUS_DmaJobHandle hDmaJob;
    NEXUS_DmaJobStatus jobStatus;

    BDBG_MSG(("%s - Entering function", BSTD_FUNCTION));
    BSTD_UNUSED(hDtcpIpTl);

    if ( !dtcpIpDmaHandle || !dtcpIpKeyHandle || !pSrc || !pDst || !src_length )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("Invalid Parameter during Enc/Dec operation %p %p %p %p %u",(void *)dtcpIpDmaHandle,(void *)dtcpIpKeyHandle, (void *)pSrc, (void *)pDst, src_length));
        goto ErrorExit;
    }

    if(src_length % 16 != 0)
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s - Source length  of '%u' bytes is invalid for crypto operation, must be 16-byte aligned", BSTD_FUNCTION, src_length));
        goto ErrorExit;
    }

    NEXUS_DmaJob_GetDefaultSettings (&dmaJobSetting);
    dmaJobSetting.numBlocks = 1;
    dmaJobSetting.completionCallback.callback = NULL;
    dmaJobSetting.dataFormat = NEXUS_DmaDataFormat_eBlock;
    dmaJobSetting.keySlot = dtcpIpKeyHandle;

    if ( (hDmaJob = NEXUS_DmaJob_Create (dtcpIpDmaHandle, &dmaJobSetting)) == NULL )
    {
        BDBG_ERR(("%s - NEXUS_DmaJob_Create failed\n", BSTD_FUNCTION));
        rc = Drm_NexusErr;
        goto ErrorExit;
    }

#if HOST_ACCESSIBLE_HEAP
    NEXUS_FlushCache(pSrc, src_length);
#endif

    NEXUS_FlushCache(pDst, src_length);
    NEXUS_DmaJob_GetDefaultBlockSettings (&blockSettings);
    blockSettings.pSrcAddr = pSrc;
    blockSettings.pDestAddr = pDst;
    blockSettings.blockSize = src_length;
    blockSettings.resetCrypto = true;
    blockSettings.scatterGatherCryptoStart = scatterGatherStart;
    blockSettings.scatterGatherCryptoEnd = scatterGatherEnd;
    blockSettings.cached = false;

    retCode = NEXUS_DmaJob_ProcessBlocks(hDmaJob, &blockSettings, 1);
    if (retCode != NEXUS_SUCCESS)
    {
        if (retCode == NEXUS_DMA_QUEUED)
        {
            for (;;)
            {
                retCode = NEXUS_DmaJob_GetStatus(hDmaJob, &jobStatus);
                 if(retCode != NEXUS_SUCCESS) {
                    BDBG_ERR(("%s - NEXUS_DmaJob_ProcessBlocks failed, retCode = %d\n", BSTD_FUNCTION, retCode));
                    rc = Drm_NexusErr;
                    goto ErrorExit;
                }

                NEXUS_FlushCache(pDst, src_length);
                if(jobStatus.currentState == NEXUS_DmaJobState_eComplete)
                {
                    break;
                }
                BKNI_Delay(1);
            }
        }
    }

ErrorExit:
    if(hDmaJob != NULL) NEXUS_DmaJob_Destroy (hDmaJob);

    BDBG_MSG(("%s - Exiting function", BSTD_FUNCTION));
    return rc;
}

DrmRC DRM_DtcpIpTl_UpdateKeyIv(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t *keyIv,
    uint32_t keyslotID
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !keyIv )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(DTCP_AES_IV_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[0].len = DTCP_AES_IV_SIZE;
    BKNI_Memcpy(container->blocks[0].data.ptr, keyIv, DTCP_AES_IV_SIZE);

    container->basicIn[0] = keyslotID;

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_UpdateKeyIv,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during DrmDtcpIpTl_CommandId_UpdateKeyIv operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        BDBG_ERR(("%s - KeyIv updated", BSTD_FUNCTION));
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
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

DrmRC DRM_DtcpIpTl_ModAdd(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * r,
    uint8_t * a,
    uint8_t * b,
    uint8_t * m,
    uint32_t size_a,
    uint32_t size_b,
    uint32_t size_m
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !r  || !a || !b || !m)
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(DTCP_RTT_MK_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[0].len = DTCP_RTT_MK_SIZE;

    if (size_a != 0)
    {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(size_a, SRAI_MemoryType_Shared);
        if (container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[1].len = size_a;
        BKNI_Memcpy(container->blocks[1].data.ptr, a, size_a);
    }

    if (size_b != 0)
    {
        container->blocks[2].data.ptr = SRAI_Memory_Allocate(size_b, SRAI_MemoryType_Shared);
        if (container->blocks[2].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[2].len = size_b;
        BKNI_Memcpy(container->blocks[2].data.ptr, b, size_b);
    }

    if (size_m != 0)
    {
        container->blocks[3].data.ptr = SRAI_Memory_Allocate(size_m, SRAI_MemoryType_Shared);
        if (container->blocks[3].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[3].len = size_m;
        BKNI_Memcpy(container->blocks[3].data.ptr, m, size_m);
    }

    container->basicIn[0] = size_a;
    container->basicIn[1] = size_b;
    container->basicIn[2] = size_m;
#ifdef B_DRM_DTCP_DEBUG
    DUMP_DTCP_DATA("Input a : ", a, size_a);
    DUMP_DTCP_DATA("Input b : ", b, size_b);
    DUMP_DTCP_DATA("Input m : ", m, size_m);
#endif

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_ModAdd,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during DRM_DtcpIpTl_ModAdd operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        if (r != NULL)
        {
            BKNI_Memcpy(r, container->blocks[0].data.ptr, container->blocks[0].len);
        }
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        if (container->blocks[2].data.ptr) {
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }
        if (container->blocks[3].data.ptr) {
            SRAI_Memory_Free(container->blocks[3].data.ptr);
            container->blocks[3].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

DrmRC DRM_DtcpIpTl_ComputeRttMac(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * AuthKey,
    uint8_t * RttN,
    uint8_t * MacValue
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !AuthKey  || !RttN || !MacValue )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(DTCP_AUTH_KEY_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[0].len = DTCP_RTT_MK_SIZE;
    BKNI_Memcpy(container->blocks[0].data.ptr, AuthKey, DTCP_AUTH_KEY_SIZE);

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(DTCP_RTT_N_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[1].len = DTCP_RTT_N_SIZE;
    BKNI_Memcpy(container->blocks[1].data.ptr, RttN, DTCP_RTT_N_SIZE);

    container->blocks[2].data.ptr = SRAI_Memory_Allocate(DTCP_RTT_MAC_DATA_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[2].len = DTCP_RTT_MAC_DATA_SIZE;

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_ComputeRttMac,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during DRM_DtcpIpTl_ComputeRttMac operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        if (MacValue != NULL)
        {
            BKNI_Memcpy(MacValue, container->blocks[2].data.ptr, container->blocks[2].len);
        }
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        if (container->blocks[2].data.ptr) {
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

DrmRC DRM_DtcpIpTl_ComputeRttMac_Alt(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * AuthKey,
    uint8_t * RttN,
    uint32_t RttN_sz,
    uint8_t * MacValue
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !AuthKey  || !RttN || !MacValue )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(DTCP_EXCHANGE_KEY_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[0].len = DTCP_EXCHANGE_KEY_SIZE;
    BKNI_Memcpy(container->blocks[0].data.ptr, AuthKey, DTCP_EXCHANGE_KEY_SIZE);

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(DTCP_RTT_MK_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[1].len = DTCP_RTT_MK_SIZE;
    BKNI_Memcpy(container->blocks[1].data.ptr, RttN, DTCP_RTT_MK_SIZE);

    container->blocks[2].data.ptr = SRAI_Memory_Allocate(DTCP_CONT_KEY_CONF_MAC_DATA_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[2].len = DTCP_CONT_KEY_CONF_MAC_DATA_SIZE;

    container->basicIn[0] = RttN_sz;

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_ComputeRttMac_2,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during DrmDtcpIpTl_CommandId_ComputeRttMac_2 operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        if (MacValue != NULL)
        {
            BKNI_Memcpy(MacValue, container->blocks[2].data.ptr, container->blocks[2].len);
        }
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        if (container->blocks[2].data.ptr) {
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}


DrmRC DRM_DtcpIpTl_CheckOverFlow(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * a,
    uint8_t * b,
    uint32_t size,
    bool *retVal
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !a  || !b || !retVal )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(DTCP_CONTENT_KEY_NONCE_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[0].len = DTCP_CONTENT_KEY_NONCE_SIZE;
    BKNI_Memcpy(container->blocks[0].data.ptr, a, DTCP_CONTENT_KEY_NONCE_SIZE);

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(DTCP_CONT_KEY_CONF_R_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[1].len = DTCP_CONT_KEY_CONF_R_SIZE;
    BKNI_Memcpy(container->blocks[1].data.ptr, b, DTCP_CONT_KEY_CONF_R_SIZE);

    container->basicIn[0] = size;

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_CheckOverFlow,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during B_DTCP_IP_CheckOverFlow operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        *retVal = (bool)container->basicOut[1];
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}


DrmRC DRM_DtcpIpTl_GetFirstPhaseValue(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * pXv,
    uint8_t * pXk
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !pXv  || !pXk )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[0].len = DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE;

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(DTCP_DH_FIRST_PHASE_SECRET_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[1].len = DTCP_DH_FIRST_PHASE_SECRET_SIZE;

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_GetFirstPhaseValue,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during DRM_DtcpIpTl_GetFirstPhaseValue operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        if (pXv != NULL && pXk != NULL)
        {
            BKNI_Memcpy(pXv, container->blocks[0].data.ptr, container->blocks[0].len);
            BKNI_Memcpy(pXk, container->blocks[1].data.ptr, container->blocks[1].len);
        }
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

DrmRC DRM_DtcpIpTl_GetSharedSecret(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * pKauth,
    uint8_t * pXk,
    uint8_t *pYv
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !pYv  || !pXk || !pKauth)
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(DTCP_AUTH_KEY_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[0].len = DTCP_AUTH_KEY_SIZE;

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(DTCP_DH_FIRST_PHASE_SECRET_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[1].len = DTCP_DH_FIRST_PHASE_SECRET_SIZE;
    BKNI_Memcpy(container->blocks[1].data.ptr, pXk, DTCP_DH_FIRST_PHASE_SECRET_SIZE);

    container->blocks[2].data.ptr = SRAI_Memory_Allocate(DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[2].len = DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE;
    BKNI_Memcpy(container->blocks[2].data.ptr, pYv, DTCP_EC_DH_FIRST_PHASE_VALUE_SIZE);

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_GetSharedSecret,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during DRM_DtcpIpTl_GetSharedSecret operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        if (pKauth != NULL)
        {
            BKNI_Memcpy(pKauth, container->blocks[0].data.ptr, container->blocks[0].len);
        }
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        if (container->blocks[2].data.ptr) {
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

DrmRC DRM_DtcpIpTl_SignData_BinKey(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint8_t * pSignature,
    uint8_t * pBuffer,
    uint32_t len
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !pSignature  || !pBuffer || !len )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(DTCP_SIGNATURE_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[0].len = DTCP_SIGNATURE_SIZE;

    if (len != 0)
    {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(len, SRAI_MemoryType_Shared);
        if (container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[1].len = len;
        BKNI_Memcpy(container->blocks[1].data.ptr, pBuffer, len);
    }

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_SignData_BinKey,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during DRM_DtcpIpTl_SignData_BinKey operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        if (pSignature != NULL && pBuffer != NULL)
        {
            BKNI_Memcpy(pSignature, container->blocks[0].data.ptr, DTCP_SIGNATURE_SIZE);
        }
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}


DrmRC DRM_DtcpIpTl_VerifyData_BinKey(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint32_t *valid,
    uint8_t * pSignature,
    uint8_t * pBuffer,
    uint32_t len,
    uint8_t * BinKey
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !pSignature  || !pBuffer || !len || !BinKey || !valid )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->blocks[0].data.ptr = SRAI_Memory_Allocate(DTCP_SIGNATURE_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[0].len = DTCP_SIGNATURE_SIZE;
    BKNI_Memcpy(container->blocks[0].data.ptr, pSignature, DTCP_SIGNATURE_SIZE);

    if (len != 0)
    {
        container->blocks[1].data.ptr = SRAI_Memory_Allocate(len, SRAI_MemoryType_Shared);
        if (container->blocks[1].data.ptr == NULL)
        {
            BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
            rc = Drm_MemErr;
            goto ErrorExit;
        }
        container->blocks[1].len = len;
        BKNI_Memcpy(container->blocks[1].data.ptr, pBuffer, len);
    }

    container->blocks[2].data.ptr = SRAI_Memory_Allocate(DTCP_PUBLIC_KEY_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[2].len = DTCP_PUBLIC_KEY_SIZE;
    BKNI_Memcpy(container->blocks[2].data.ptr, BinKey, DTCP_PUBLIC_KEY_SIZE);
#ifdef B_DRM_DTCP_DEBUG
    DUMP_DTCP_DATA("DTLA PUBLIC KEY that goes in the container block : ", container->blocks[2].data.ptr, DTCP_PUBLIC_KEY_SIZE);
    DUMP_DTCP_DATA("Other device Signature that goes in container block : ", container->blocks[0].data.ptr, DTCP_SIGNATURE_SIZE);
    DUMP_DTCP_DATA("Other decive cert that goes in container bloack : ", container->blocks[1].data.ptr, len);
#endif

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_VerifyData_BinKey,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during DRM_DtcpIpTl_VerifyData_BinKey operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        *valid = (uint32_t)container->basicOut[1];
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        if (container->blocks[2].data.ptr) {
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    return rc;
}

DrmRC DRM_DtcpIpTl_CreateContentKey(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint32_t aExtendedEmi,
    uint8_t *aExchangeKey,
    uint8_t *aNonce,
    uint8_t *aCipherKey,
    uint8_t *aCipherIv,
    uint32_t keyslotID
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl || !aExchangeKey  || !aNonce || !aCipherKey || !aCipherIv )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    BSTD_UNUSED(aCipherIv);
    BSTD_UNUSED(aCipherKey);

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->basicIn[0] = aExtendedEmi;
    container->blocks[0].data.ptr = SRAI_Memory_Allocate(DTCP_EXCHANGE_KEY_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[0].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[0].len = DTCP_EXCHANGE_KEY_SIZE;
    BKNI_Memcpy(container->blocks[0].data.ptr, aExchangeKey, DTCP_EXCHANGE_KEY_SIZE);

    container->blocks[1].data.ptr = SRAI_Memory_Allocate(DTCP_CONTENT_KEY_NONCE_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[1].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[1].len = DTCP_CONTENT_KEY_NONCE_SIZE;
    BKNI_Memcpy(container->blocks[1].data.ptr, aNonce, DTCP_CONTENT_KEY_NONCE_SIZE);

    #if 0
    container->blocks[2].data.ptr = SRAI_Memory_Allocate(DTCP_AES_KEY_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[2].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[2].len = DTCP_AES_KEY_SIZE;

    container->blocks[3].data.ptr = SRAI_Memory_Allocate(DTCP_AES_IV_SIZE, SRAI_MemoryType_Shared);
    if (container->blocks[3].data.ptr == NULL)
    {
        BDBG_ERR(("%s - Error allocating src buffer", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }
    container->blocks[3].len = DTCP_AES_IV_SIZE;
    #endif
    container->basicIn[1] = keyslotID;

    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_CreateContentKey,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during DRM_DtcpIpTl_VerifyData_BinKey operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = (DrmRC)container->basicOut[0];
    if (rc == Drm_Success) {
        #if 0 /* key loading is done on SAGE no neeed to bring the keys on HOST*/
        if (aCipherKey != NULL && aCipherIv != NULL)
        {
            BKNI_Memcpy(aCipherKey, container->blocks[2].data.ptr, DTCP_AES_KEY_SIZE);
            BKNI_Memcpy(aCipherIv, container->blocks[3].data.ptr, DTCP_AES_IV_SIZE);
        }
        #endif
    }
    else
    {
        BDBG_ERR(("%s - Command was sent succuessfully but actual operation failed (0x%08x)", BSTD_FUNCTION, rc));
    }

ErrorExit:
    if (container)
    {
        if (container->blocks[0].data.ptr) {
            SRAI_Memory_Free(container->blocks[0].data.ptr);
            container->blocks[0].data.ptr = NULL;
        }
        if (container->blocks[1].data.ptr) {
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        #if 0
        if (container->blocks[2].data.ptr) {
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[2].data.ptr = NULL;
        }
        if (container->blocks[3].data.ptr) {
            SRAI_Memory_Free(container->blocks[2].data.ptr);
            container->blocks[3].data.ptr = NULL;
        }
        #endif
        SRAI_Container_Free(container);
    }

    return rc;
}

DrmRC DRM_DtcpIpTl_FreeKeySlot(
    DRM_DtcpIpTlHandle hDtcpIpTl,
    uint32_t keyslotID
    )
{
    DrmRC rc = Drm_Success;
    BERR_Code sage_rc;
    BSAGElib_InOutContainer *container = NULL;

    if ( !hDtcpIpTl )
    {
        rc = Drm_InvalidParameter;
        BDBG_ERR(("%s Invalid Parameter ", BSTD_FUNCTION  ));
        goto ErrorExit;
    }

    DRM_DrmDtcpIpTl_P_Context_t *pCtx = (DRM_DrmDtcpIpTl_P_Context_t *)hDtcpIpTl;

    container = SRAI_Container_Allocate();
    if (container == NULL)
    {
        BDBG_ERR(("%s - Error allocating container", BSTD_FUNCTION));
        rc = Drm_MemErr;
        goto ErrorExit;
    }

    container->basicIn[0] = keyslotID;
    sage_rc = SRAI_Module_ProcessCommand(
        pCtx->moduleHandle,
        DrmDtcpIpTl_CommandId_FreeKeySlot,
        container);
    if (sage_rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s - Error during DrmDtcpIpTl_CommandId_FreeKeySlot operation", BSTD_FUNCTION));
        rc = Drm_Err;
        goto ErrorExit;
    }

ErrorExit:
    if (container)
    {
        SRAI_Container_Free(container);
    }
    return rc;
}
