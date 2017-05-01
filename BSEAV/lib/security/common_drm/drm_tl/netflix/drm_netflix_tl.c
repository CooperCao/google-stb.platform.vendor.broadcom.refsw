/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bkni.h"

#include "drm_netflix_tl.h"
#include "drm_data.h"
#include "drm_metadata.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "blst_list.h"
#include "drm_common_tl.h"
/*#include "bsagelib_types.h"*/
#include "sage_srai.h"

#include "nexus_memory.h"

#define nrd_version 1

enum {
    NETFLIX_DECRYPT_DATA     = 0,
    NETFLIX_ENCRYPT_DATA,
    NETFLIX_AES_CTR_INIT,
    NETFLIX_AES_CTR_UNINIT,
    NETFLIX_AES_CTR_ENCRYPT_INIT,
    NETFLIX_AES_CTR_DECRYPT_INIT,
    NETFLIX_AES_CTR_UPDATE,
    NETFLIX_AES_CTR_FINAL,

    NETFLIX_HMAC_INIT,
    NETFLIX_HMAC_UNINIT,
    NETFLIX_HMAC_COMPUTE,

    NETFLIX_DH_INIT,
    NETFLIX_DH_UNINIT,
    NETFLIX_DH_COMPUTE_SHARED_SECRET,
    NETFLIX_DH_GET_PUBKEY_SIZE,
    NETFLIX_DH_GET_PUBKEY,

    NETFLIX_CREATE_CLIENT_KEY,

    NETFLIX_GET_ESN_SIZE,
    NETFLIX_GET_ESN,

    /* NRD 4.1 */
    NETFLIX_IMPORT_KEY,
    NETFLIX_IMPORT_SEALED_KEY,
    NETFLIX_EXPORT_KEY,
    NETFLIX_EXPORT_SEALED_KEY,
    NETFLIX_GET_KEY_INFO,
    NETFLIX_GET_NAMED_KEY,
    NETFLIX_AES_CBC,
    NETFLIX_HMAC,
    NETFLIX_HMAC_VERIFY,
    NETFLIX_DH_GEN_KEYS,
    NETFLIX_DH_DERIVE_KEYS,
    NETFLIX_DELETE_KEY,
    NETFLIX_SECURE_STORE_OP,

    NETFLIX_COMMANDID_MAX

};

BDBG_MODULE(drm_netflix_tl);

#define CHK_RC(_rc)                                      \
{                                                        \
    if(_rc) {                                            \
        rc = _rc;                                        \
        BDBG_ERR(("%s - rc %d", __FUNCTION__, rc));      \
        goto ErrorExit;                                  \
    }                                                    \
}

/* Private (static) variable definitions */
static bool paramStructureSetExternally = false;
static drm_nf_data_t netflix_data_struct;

extern DrmRC DRM_Common_P_GetFileSize(char * filename, uint32_t *filesize);

typedef struct Netflix_SageCommandNode
{
    BSAGElib_InOutContainer                   *pCmd;
    bool                                  inUse;
    BLST_D_ENTRY(Netflix_SageCommandNode) link;
}Netflix_SageCommandNode;

typedef BLST_D_HEAD(Netflix_Sage_Cmd_list, Netflix_SageCommandNode) Netflix_Sage_Cmd_list_t;

typedef struct DrmNetflixSageContext_s
{
    SRAI_ModuleHandle       pNetflixSageModuleHandle;
    netflixContext_t        pSageNetflixCtx;
    BKNI_MutexHandle        mutex;
    Netflix_Sage_Cmd_list_t cmds;

}DrmNetflixSageContext_t;


#define NETFLIX_CONCURRENT_COMMANDS (15)

#define GET_COMMAND_CONTAINER(_pChild, _list, _link, _mutex) do {                                        \
        BKNI_AcquireMutex(_mutex);                                                                       \
        for (_pChild = BLST_D_FIRST(_list); _pChild != NULL; _pChild = BLST_D_NEXT(_pChild, _link)) {    \
            if(_pChild != NULL){                                                                         \
                if(_pChild->inUse == false){                                                             \
                    _pChild->inUse = true;                                                               \
                    BKNI_Memset(_pChild->pCmd, 0, sizeof(BSAGElib_InOutContainer));                          \
                    break;                                                                               \
                }                                                                                        \
                else continue;                                                                           \
            }                                                                                            \
            else break;                                                                                  \
        }                                                                                                \
        BKNI_ReleaseMutex(_mutex);                                                                       \
    } while(0)

#define RELEASE_COMMAND_CONTAINER(_pCmd) do {    \
    _pCmd->inUse = false;                        \
    }while(0)

#define SETUP_SHARED_BLOCK(_blk, _ptr, _len)  do {                        \
    _blk.len = _len;                                                      \
    _blk.data.ptr = SRAI_Memory_Allocate(_len, SRAI_MemoryType_Shared);   \
    if(_blk.data.ptr == NULL){                                            \
        BDBG_ERR(("%s - SRAI mem alloca failed", __FUNCTION__));          \
        CHK_RC(Drm_Err); }                                                \
    BDBG_MSG(("SETUP_SHARED_BLOCK alloc %u to %p",_len,_blk.data.ptr));   \
    BKNI_Memcpy(_blk.data.ptr, _ptr, _len);                               \
    }while(0)


/******************************************************************************
** FUNCTION:
**  DRM_Netflix_Initialize
**
** DESCRIPTION:
**   Reads the bin file specified and pre-loads the confidential info
******************************************************************************/
DrmRC DRM_Netflix_Initialize(char                    *bin_file,
                             DrmNetFlixSageHandle    *netflixSageHandle)
{
    NEXUS_MemoryAllocationSettings  allocSettings;
    NEXUS_Error                     nrc = NEXUS_SUCCESS;
    DrmRC                           rc = Drm_Success;
    BSAGElib_InOutContainer            *container = NULL;
    Netflix_SageCommandNode        *pNode;
    uint32_t                        binfilesize = 0;
    DrmNetflixSageContext_t        *handle=NULL;
    uint32_t                        ii=0;
    DrmCommonInit_TL_t              drmCmnInit;
    ChipType_e                      chip_type;

    BDBG_LOG(("%s - Entered function", __FUNCTION__));
    printf("%s:%d\n",__FUNCTION__,__LINE__);

    NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
    nrc = NEXUS_Memory_Allocate(sizeof(DrmNetflixSageContext_t), &allocSettings, (void *)&handle);
    if(nrc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - NEXUS_Memory_Allocate failed for NetflixSage handle, rc = %d", __FUNCTION__, nrc));
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        rc = nrc;
        goto ErrorExit;
    }

    if ( NULL == handle ) {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto ErrorExit;
    }

    BKNI_Memset(handle, 0, sizeof(DrmNetflixSageContext_t));

    drmCmnInit.drmCommonInit.heap=NULL;

    chip_type = DRM_Common_GetChipType();
#ifdef USE_UNIFIED_COMMON_DRM
    if(chip_type == ChipType_eZS)
    {
        drmCmnInit.ta_bin_file_path = bdrm_get_ta_dev_bin_file_path();
    }
    else
    {
        drmCmnInit.ta_bin_file_path = bdrm_get_ta_bin_file_path();
    }
#else
    drmCmnInit.drmType = BSAGElib_BinFileDrmType_eNetflix;
    if(chip_type == ChipType_eZS)
    {
        drmCmnInit.ta_bin_file_path = bdrm_get_ta_nflx_dev_bin_file_path();
    }
    else
    {
        drmCmnInit.ta_bin_file_path = bdrm_get_ta_nflx_bin_file_path();
    }
#endif

    rc = DRM_Common_TL_Initialize(&drmCmnInit);
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing common drm", __FUNCTION__));
        goto ErrorExit;
    }
    printf("%s:%d DRM_Common_TL_Initialize succeeded\n",__FUNCTION__,__LINE__);

    container = SRAI_Container_Allocate();
    if(container == NULL)
    {
        BDBG_ERR(("%s - Error to allocate SRAI Container", __FUNCTION__));
        rc = Drm_Err;
        goto ErrorExit;
    }

    rc = DRM_Common_P_GetFileSize((char *) bin_file, &binfilesize );
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error determine file size of bin file", __FUNCTION__));
        goto ErrorExit;
    }
    container->blocks[1].len = binfilesize; /* allocate the maximum size as the bin file*/
    container->blocks[1].data.ptr = SRAI_Memory_Allocate(binfilesize, SRAI_MemoryType_Shared);
    if(container->blocks[1].data.ptr == NULL) {
        BDBG_ERR(("%s - Error allocating SRAI memory for PD certificate", __FUNCTION__));
        goto ErrorExit;
    }
    BDBG_MSG(("%s: allocated %u for PD certificate at container->blocks[1].data.ptr %p",__FUNCTION__,binfilesize, container->blocks[1].data.ptr));
    handle->pNetflixSageModuleHandle = NULL;

#ifdef USE_UNIFIED_COMMON_DRM
    rc = DRM_Common_TL_ModuleInitialize(DrmCommon_ModuleId_eNetflix, (char *) bin_file, container, &(handle->pNetflixSageModuleHandle));
#else
    rc = DRM_Common_TL_ModuleInitialize_TA(Common_Platform_Netflix, Netflix_ModuleId_eDRM, (char *) bin_file, container, &(handle->pNetflixSageModuleHandle));
#endif
    if(rc != Drm_Success)
    {
        BDBG_ERR(("%s - Error initializing module (0x%08x)", __FUNCTION__, container->basicOut[0]));
        goto ErrorExit;
    }
printf("%s:%d DRM_Common_TL_ModuleInitialize_TA succeeded, SageModuleHandle 0x%p\n",__FUNCTION__,__LINE__,(void *)handle->pNetflixSageModuleHandle);
    handle->pSageNetflixCtx = (void *)container->basicOut[2];

    /* initialize the SRAI command nodes */
    if( handle->pSageNetflixCtx !=NULL)
    {
        if(BKNI_CreateMutex(&handle->mutex) != BERR_SUCCESS){
            BDBG_ERR(("%s: failed to create mutex",__FUNCTION__));
            goto ErrorExit;
        }
        NEXUS_Memory_GetDefaultAllocationSettings(&allocSettings);
        BLST_D_INIT(&handle->cmds);
        for(ii = 0; ii < NETFLIX_CONCURRENT_COMMANDS; ii++){
            nrc = NEXUS_Memory_Allocate(sizeof(Netflix_SageCommandNode), &allocSettings, (void *)&pNode);
            if(rc != NEXUS_SUCCESS) {
                BDBG_ERR(("%s - NEXUS_Memory_Allocate failed for Netflix handle, rc = %d\n", __FUNCTION__, rc));
                (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                goto ErrorExit;
            }
            if(pNode){
                pNode->pCmd = SRAI_Container_Allocate();
                if(pNode->pCmd){
                    pNode->inUse = false;
                    BLST_D_INSERT_HEAD(&handle->cmds, (pNode), link);
                }
                else {
                    NEXUS_Memory_Free(pNode);
                    goto ErrorExit;
                }
            }
            else goto ErrorExit;
        }

        /*
        GET_COMMAND_CONTAINER(pNode, &handle->cmds, link, handle->mutex);
        if(pNode == NULL){
            BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
            goto ErrorExit;
        }
        */
    }


    /* Clean up the SRAI container */
    if(container->blocks[1].data.ptr != NULL){
        BDBG_MSG(("%s: freeing container->blocks[1].data.ptr %p",__FUNCTION__,container->blocks[1].data.ptr));
        SRAI_Memory_Free(container->blocks[1].data.ptr);
        container->blocks[1].data.ptr = NULL;
    }

    SRAI_Container_Free(container);

    *netflixSageHandle = (DrmNetFlixSageHandle ) handle;

    return Drm_Success;

ErrorExit:

    if(container){
        if(container->blocks[1].data.ptr != NULL){
            SRAI_Memory_Free(container->blocks[1].data.ptr);
            container->blocks[1].data.ptr = NULL;
        }
        SRAI_Container_Free(container);
    }

    if( handle != NULL) {
#ifdef USE_UNIFIED_COMMON_DRM
        DRM_Common_TL_ModuleFinalize(handle->pNetflixSageModuleHandle);
#else
        DRM_Common_TL_ModuleFinalize_TA(Common_Platform_Netflix, handle->pNetflixSageModuleHandle);
#endif
        handle->pNetflixSageModuleHandle = NULL;
        while(!BLST_D_EMPTY(&handle->cmds)){
            pNode = BLST_D_FIRST(&handle->cmds);
            BLST_D_REMOVE(&handle->cmds, pNode, link);
            SRAI_Container_Free(pNode->pCmd);
            NEXUS_Memory_Free(pNode);
        }
        if(handle->mutex)BKNI_DestroyMutex(handle->mutex);
        NEXUS_Memory_Free(handle);
    }


    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}


/******************************************************************************
** FUNCTION:
**  DRM_Netflix_Finalize
**
** DESCRIPTION:
**   Clean up the Netflix module
**
******************************************************************************/
DrmRC DRM_Netflix_Finalize(DrmNetFlixSageHandle   netflixSageHandle)
{
    DrmRC rc = Drm_Success;
    Netflix_SageCommandNode  *pNode;
    DrmNetflixSageContext_t  *ctx = (DrmNetflixSageContext_t *) netflixSageHandle;

    BDBG_MSG(("%s - Entered function", __FUNCTION__));

    BKNI_Memset((uint8_t*)&netflix_data_struct, 0x00, sizeof(drm_nf_data_t));
    /*DRM_Common_Finalize();*/

    paramStructureSetExternally = false;

    if( ctx != NULL) {
#ifdef USE_UNIFIED_COMMON_DRM
        DRM_Common_TL_ModuleFinalize(ctx->pNetflixSageModuleHandle);
#else
        DRM_Common_TL_ModuleFinalize_TA(Common_Platform_Netflix, ctx->pNetflixSageModuleHandle);
#endif
        ctx->pNetflixSageModuleHandle = NULL;
        while(!BLST_D_EMPTY(&ctx->cmds)){
            pNode = BLST_D_FIRST(&ctx->cmds);
            BLST_D_REMOVE(&ctx->cmds, pNode, link);
            SRAI_Container_Free(pNode->pCmd);
            NEXUS_Memory_Free(pNode);
        }
        if(ctx->mutex)BKNI_DestroyMutex(ctx->mutex);
        NEXUS_Memory_Free(ctx);
    }
#ifdef USE_UNIFIED_COMMON_DRM
    DRM_Common_TL_Finalize();
#else
    DRM_Common_TL_Finalize_TA(Common_Platform_Netflix);
#endif
    BDBG_MSG(("%s - Exiting function", __FUNCTION__));
    return rc;
}


/******************************************************************************
** FUNCTION
    Decrypts the stream received in parameter.

** DESCRIPTION:
**   this function is used to decrypt stream. If decryption is successful, the
**   digest for integrity check will be stripped from the returned string.
**
**
** PARAMETERS:
/// @param[in]      pBuf        Pointer to the buffer of encrypted data
/// @param[in]      uiSize      Size of the data to decrypt.
/// @param[out]     pOutSize    Real size of the decrypted string
**
******************************************************************************/
DrmRC DRM_Netflix_Decrypt(DrmNetFlixSageHandle  pHandle,
                          uint8_t              *pBuf,
                          uint32_t              uiSize,
                          uint32_t             *pOutSize)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx != NULL);
    BDBG_ASSERT(pBuf != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = uiSize;
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pBuf, uiSize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_DECRYPT_DATA,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success) {
        *pOutSize = pNode->pCmd->blocks[0].len;
        BKNI_Memcpy(pBuf, pNode->pCmd->blocks[0].data.ptr, pNode->pCmd->blocks[0].len);
    }
    else
    {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, rc));
    }

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

/******************************************************************************
** FUNCTION
    Decrypts the stream received in parameter.

** DESCRIPTION:
**   this function is used to decrypt stream
**
** PARAMETERS:
/// @param[in/out]  pBuf        Pointer to the buffer which must be large enough
///                             to hold the 32-byte digest.
/// @param[in]      uiSize      Actual size of the data to encrypt.
/// @param[out]     pOutSize    Returned size of the encrypted data
**
******************************************************************************/
DrmRC DRM_Netflix_Encrypt(DrmNetFlixSageHandle     pHandle,
                          uint8_t                 *pBuf,
                          uint32_t                 uiSize,
                          uint32_t                *pOutSize)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx != NULL);
    BDBG_ASSERT(pBuf != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pBuf, uiSize);
    pNode->pCmd->blocks[0].len = uiSize;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_ENCRYPT_DATA,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success) {
        *pOutSize = pNode->pCmd->blocks[0].len;
        BKNI_Memcpy(pBuf, pNode->pCmd->blocks[0].data.ptr, pNode->pCmd->blocks[0].len);
    }

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}


/* Initialize the AesCtr Cipher at the SAGE. It supports only EVP_aes_126_cbc only */
DrmRC DRM_Netflix_AesCtr_Init(DrmNetFlixSageHandle       pHandle,
                              NetFlixKeyType_e           authKeyType,
                              NetFlixCipherMode_t        cipherMode,
                              DrmNetflixSageAesCtrCtx_t *pCipherCtx)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (uint32_t)authKeyType;
    pNode->pCmd->basicIn[2] = (uint32_t)cipherMode;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_AES_CTR_INIT,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success) {
       *pCipherCtx = (void *)pNode->pCmd->basicOut[1];
    }
    else
    {
       *pCipherCtx = NULL;
    }


ErrorExit:
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}


DrmRC DRM_Netflix_AesCtr_UnInit(DrmNetFlixSageHandle       pHandle,
                                DrmNetflixSageAesCtrCtx_t  pCipherCtx)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;
    /*uint8_t*                 dmaBuf = NULL; */

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t) pCipherCtx;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_AES_CTR_UNINIT,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success) {
        pCipherCtx = NULL;
    }


ErrorExit:
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}


DrmRC DRM_Netflix_AesCtr_EncryptInit(DrmNetFlixSageHandle       pHandle,
                                     DrmNetflixSageAesCtrCtx_t  pCipherCtx,
                                     uint8_t                   *pEncKey,
                                     uint32_t                   encKeySize,
                                     uint8_t                   *pIV,
                                     uint32_t                   ivSize)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));
    /*printf("%s - Entering\n", __FUNCTION__);*/

    BDBG_ASSERT(pCtx != NULL);
    BDBG_ASSERT(pCipherCtx != NULL);
    /* BDBG_ASSERT(pEncKey != NULL); */
    /* BDBG_ASSERT(pIV != NULL); */

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pCipherCtx;
    /*printf("%s - Key size %d.\n", __FUNCTION__, encKeySize);*/
    if( encKeySize > 0)
    {
        SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pEncKey, encKeySize);
    }

    if( pIV != NULL)
    {
        SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], pIV, ivSize);
    }

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_AES_CTR_ENCRYPT_INIT,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s - AES CTR Encrypt init failed.", __FUNCTION__));
    }


ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[1].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
        pNode->pCmd->blocks[1].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));
    /*printf("%s - Exiting, rc %d.\n", __FUNCTION__, rc);*/

    return rc;
}


DrmRC DRM_Netflix_AesCtr_DecryptInit(DrmNetFlixSageHandle       pHandle,
                                     DrmNetflixSageAesCtrCtx_t  pCipherCtx,
                                     uint8_t                   *pEncKey,
                                     uint32_t                   encKeySize,
                                     uint8_t                   *pIV,
                                     uint32_t                   ivSize)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));
    /*printf("%s - Entering\n", __FUNCTION__);*/

    BDBG_ASSERT(pCtx       != NULL);
    BDBG_ASSERT(pCipherCtx != NULL);
    /*BDBG_ASSERT(pEncKey != NULL); */
    /*BDBG_ASSERT(pIV        != NULL);*/

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pCipherCtx;
    /*printf("%s - Key size %d.\n", __FUNCTION__, encKeySize);*/
    if( encKeySize > 0)
    {
        SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pEncKey, encKeySize);
    }

    if( pIV != NULL)
    {
        SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], pIV, ivSize);
    }

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_AES_CTR_DECRYPT_INIT,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s - AES CTR Decrypt init failed.", __FUNCTION__));
    }


ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[1].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
        pNode->pCmd->blocks[1].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));
    /*printf("%s - Exiting, rc %d.\n", __FUNCTION__, rc);*/

    return rc;
}


DrmRC DRM_Netflix_AesCtr_Update(DrmNetFlixSageHandle       pHandle,
                                DrmNetflixSageAesCtrCtx_t  pCipherCtx,
                                const uint8_t             *pInBuf,
                                uint32_t                   inBufSize,
                                uint8_t                   *pOutBuf,
                                uint32_t                   outBufSize,
                                int                       *pOutputLen)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;
    /*uint8_t*                 dmaBuf = NULL; */

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx       != NULL);
    BDBG_ASSERT(pCipherCtx != NULL);
    BDBG_ASSERT(pInBuf     != NULL);
    BDBG_ASSERT(pOutBuf    != NULL);
    BDBG_ASSERT(pOutputLen != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pCipherCtx;
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pInBuf, inBufSize);
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], pOutBuf, outBufSize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_AES_CTR_UPDATE,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[1].len*/
    *pOutputLen = (int) pNode->pCmd->basicOut[1];
    BDBG_MSG(("%s:%d - output from AES CTR Update with len %d.", __FUNCTION__,__LINE__,*pOutputLen));
    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success) {
        BKNI_Memcpy(pOutBuf, pNode->pCmd->blocks[1].data.ptr,*pOutputLen);
    }
    else {
        BDBG_ERR(("%s - AES CTR Update failed.", __FUNCTION__));
    }


ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[1].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
        pNode->pCmd->blocks[1].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_AesCtr_Final(DrmNetFlixSageHandle       pHandle,
                               DrmNetflixSageAesCtrCtx_t  pCipherCtx,
                               uint8_t                   *pOutBuf,
                               uint32_t                   outBufSize,
                               int                       *pOutputLen)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));
    /*printf("%s - Entering\n", __FUNCTION__);*/

    BDBG_ASSERT(pCtx       != NULL);
    BDBG_ASSERT(pCipherCtx != NULL);
    BDBG_ASSERT(pOutBuf    != NULL);
    BDBG_ASSERT(pOutputLen != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    BDBG_MSG(("%s - provided output buffer size %d.", __FUNCTION__,outBufSize));

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pCipherCtx;
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pOutBuf, outBufSize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_AES_CTR_FINAL,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success) {
        /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[1].len*/
        *pOutputLen = (int) pNode->pCmd->basicOut[1];

        /*printf("%s - AES CTR Command success with output buffer size %d\n", __FUNCTION__,*pOutputLen);*/
        BKNI_Memcpy(pOutBuf, pNode->pCmd->blocks[0].data.ptr,*pOutputLen);
    }
    else {
        BDBG_ERR(("%s - AES CTR Decrypt init failed.", __FUNCTION__));
    }


ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));
    /*printf("%s - Exiting, rc %d.\n", __FUNCTION__, rc);*/

    return rc;
}



DrmRC DRM_Netflix_Hmac_Init(DrmNetFlixSageHandle      pHandle,
                            DrmNetflixSageHmacCtx_t  *pHmacCtx,
                            NetFlixKeyType_e          hmacKeyType,
                            NetFlixDigestType_t       digestType)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));
    /*printf("%s - Entering\n", __FUNCTION__);*/

    BDBG_ASSERT(pCtx != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (uint32_t)hmacKeyType;
    pNode->pCmd->basicIn[2] = (uint32_t)digestType;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_HMAC_INIT,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success) {
        *pHmacCtx = (void *)pNode->pCmd->basicOut[1];
    }
    else
    {
        *pHmacCtx = NULL;
    }


ErrorExit:
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));
    /*printf("%s - Exiting, rc %d.\n", __FUNCTION__, rc);*/

    return rc;
}

DrmRC DRM_Netflix_Hmac_UnInit(DrmNetFlixSageHandle      pHandle,
                              DrmNetflixSageHmacCtx_t   pHmacCtx)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;
    /*uint8_t*                 dmaBuf = NULL; */

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pHmacCtx;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_HMAC_UNINIT,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success) {
        pHmacCtx = NULL;
    }


ErrorExit:
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_Hmac_Compute(DrmNetFlixSageHandle      pHandle,
                               DrmNetflixSageHmacCtx_t   pHmacCtx,
                               uint8_t                  *pEncHmacKey,
                               uint32_t                  encHmacKeySize,
                               const uint8_t            *pInput,
                               uint32_t                  inputSize,
                               uint8_t                  *pOutput,
                               uint32_t                 *pOutputSize)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));
    /*printf("%s - Entering\n", __FUNCTION__);*/

    BDBG_ASSERT(pCtx       != NULL);
    BDBG_ASSERT(pHmacCtx   != NULL);
    BDBG_ASSERT(pInput     != NULL);
    BDBG_ASSERT(pOutput    != NULL);
    BDBG_ASSERT(pOutputSize!= NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pHmacCtx;

    /*printf("%s - HmacKey size %d.\n", __FUNCTION__, encHmacKeySize);*/
    if( encHmacKeySize > 0)
    {
        SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pEncHmacKey, encHmacKeySize);
    }

    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], pInput, inputSize);
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[2], pOutput, *pOutputSize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_HMAC_COMPUTE,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s - Hmac Update failed.", __FUNCTION__));
    }

    /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[1].len*/
    *pOutputSize = (int) pNode->pCmd->basicOut[1];

    BKNI_Memcpy(pOutput, pNode->pCmd->blocks[2].data.ptr,*pOutputSize);

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[1].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
        pNode->pCmd->blocks[1].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[2].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[2].data.ptr);
        pNode->pCmd->blocks[2].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));
    /*printf("%s - Exiting, rc %d.\n", __FUNCTION__, rc);*/

    return rc;
}

DrmRC DRM_Netflix_DH_Init(DrmNetFlixSageHandle    pHandle,
                          DrmNetflixSageDHCtx_t  *pDiffHellCtx)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_DH_INIT,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success)
    {
        *pDiffHellCtx = (void *)pNode->pCmd->basicOut[1];
        /*printf("%s:%d - success with DH contxt 0x%p\n",__FUNCTION__,__LINE__,*pDiffHellCtx); */
    }
    else
    {
        *pDiffHellCtx = NULL;
    }


ErrorExit:
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_DH_UnInit(DrmNetFlixSageHandle    pHandle,
                            DrmNetflixSageDHCtx_t   pDiffHellCtx)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pDiffHellCtx;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_DH_UNINIT,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success) {
        pDiffHellCtx = NULL;
    }


ErrorExit:
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_DH_Compute_Shared_Secret(DrmNetFlixSageHandle    pHandle,
                                           DrmNetflixSageDHCtx_t   pDiffHellCtx,
                                           const uint8_t          *pPubKey,
                                           uint32_t                pubKeySize)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx           != NULL);
    BDBG_ASSERT(pDiffHellCtx   != NULL);
    BDBG_ASSERT(pPubKey        != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pDiffHellCtx;
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pPubKey, pubKeySize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_DH_COMPUTE_SHARED_SECRET,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s -  Failed to compute the Diffie-Hellman shared secret.", __FUNCTION__));
    }


ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_DH_Get_PubKeySize(DrmNetFlixSageHandle    pHandle,
                                    DrmNetflixSageDHCtx_t   pDiffHellCtx,
                                    uint32_t               *pubKeySize) /*output*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx           != NULL);
    BDBG_ASSERT(pDiffHellCtx   != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pDiffHellCtx;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_DH_GET_PUBKEY_SIZE,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s - Failed to get Diffie-Hellman public key size.", __FUNCTION__));
    }
    else {
        *pubKeySize = (uint32_t)pNode->pCmd->basicOut[1];
    }

ErrorExit:
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_DH_Get_PubKey(DrmNetFlixSageHandle    pHandle,
                                DrmNetflixSageDHCtx_t   pDiffHellCtx,
                                uint8_t                *pPubKey,  /* output */
                                uint32_t               *pubKeySize)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx           != NULL);
    BDBG_ASSERT(pDiffHellCtx   != NULL);
    BDBG_ASSERT(pPubKey        != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pDiffHellCtx;
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pPubKey, *pubKeySize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_DH_GET_PUBKEY,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s - Failed to get Diffie-Hellman public key.", __FUNCTION__));
    }
    else {
        *pubKeySize = (uint32_t) pNode->pCmd->basicOut[1];
        BKNI_Memcpy(pPubKey, pNode->pCmd->blocks[0].data.ptr,*pubKeySize);
    }


ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

/*
DrmRC DRM_Netflix_test(void)
{

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, 123));
    return Drm_Success;
}
*/

DrmRC DRM_Netflix_ClientKeys_Create(DrmNetFlixSageHandle    pHandle,
                                    DrmNetflixSageDHCtx_t   pDiffHellCtx,
                                    NetFlixKeyType_e        keyTypeToBeCreated,
                                    bool                    dh_pubkey_v2,
                                    uint8_t                *pOutEncKey,
                                    uint32_t               *pOutEncKeySize,
                                    uint8_t                *pOutEncHmac,
                                    uint32_t               *pOutEncHmacSize)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx            != NULL);
    BDBG_ASSERT(pDiffHellCtx    != NULL);
    BDBG_ASSERT(pOutEncKey      != NULL);
    BDBG_ASSERT(pOutEncKeySize  != NULL);
    BDBG_ASSERT(pOutEncHmac     != NULL);
    BDBG_ASSERT(pOutEncHmacSize != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[1] = (int32_t)pDiffHellCtx;
    pNode->pCmd->basicIn[2] = (int32_t)keyTypeToBeCreated;
    pNode->pCmd->basicIn[3] = (int32_t)dh_pubkey_v2==true?1:0;
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pOutEncKey,  *pOutEncKeySize);
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], pOutEncHmac, *pOutEncHmacSize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_CREATE_CLIENT_KEY,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s - Client Keys creation failed.", __FUNCTION__));
    }
    else {
        /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[1].len*/
        *pOutEncKeySize = (uint32_t) pNode->pCmd->basicOut[1];
        BKNI_Memcpy(pOutEncKey, pNode->pCmd->blocks[0].data.ptr,*pOutEncKeySize);
        *pOutEncHmacSize = (uint32_t) pNode->pCmd->basicOut[2];
        BKNI_Memcpy(pOutEncHmac, pNode->pCmd->blocks[1].data.ptr,*pOutEncHmacSize);
    }


ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[1].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
        pNode->pCmd->blocks[1].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}


DrmRC DRM_Netflix_Get_EsnSize(DrmNetFlixSageHandle    pHandle,
                              uint32_t               *pEsnSize) /*output*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx           != NULL);
    BDBG_ASSERT(pEsnSize       != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_GET_ESN_SIZE,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s - Failed to get Diffie-Hellman public key size.", __FUNCTION__));
    }
    else {
        *pEsnSize = (uint32_t)pNode->pCmd->basicOut[1];
    }
    /*printf("%s - Successfully got the size of ESN %d\n", __FUNCTION__,*pEsnSize);*/

ErrorExit:
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_Get_Esn(DrmNetFlixSageHandle    pHandle,
                          uint8_t                *pEsn,       /* output */
                          uint32_t               *pEsnSize)   /* in | out */
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx     != NULL);
    BDBG_ASSERT(pEsn     != NULL);
    BDBG_ASSERT(pEsnSize != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (int32_t)pCtx->pSageNetflixCtx;
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pEsn, *pEsnSize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_GET_ESN,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s - Failed to get ESN.", __FUNCTION__));
        CHK_RC(Drm_Err);
    }
    else {
        /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[1].len*/
        *pEsnSize = (uint32_t) pNode->pCmd->basicOut[1];
        BKNI_Memcpy(pEsn, pNode->pCmd->blocks[0].data.ptr,*pEsnSize);
    }

    /*printf("%s - Got the ESN %d.\n", __FUNCTION__);*/

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}


/*******************************************************************************
 *******************************************************************************
 ******************************* NRD 4.1 APIs  *********************************
 *******************************************************************************
 *******************************************************************************/
#define NFLX_EXTRACTABLE_KEY 0x80

DrmRC DRM_Netflix_Import_Key(DrmNetFlixSageHandle    pHandle,
                             uint8_t                *keyDataPtr,
                             uint32_t                keySize,
                             uint32_t                keyFormat,
                             uint32_t                algorithm,
                             uint32_t                keyUsageFlags,
                             uint32_t               *keyHandlePtr,  /*out*/
                             uint32_t               *keyTypePtr)    /*out*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx         != NULL);
    BDBG_ASSERT(keyHandlePtr != NULL);
    BDBG_ASSERT(keyTypePtr   != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[2] = (uint32_t)keyFormat;
    pNode->pCmd->basicIn[3] = (uint32_t)algorithm;
    pNode->pCmd->basicIn[4] = (uint32_t)keyUsageFlags;

    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], keyDataPtr, keySize);
    pNode->pCmd->blocks[0].len = keySize;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_IMPORT_KEY,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to import key to SAGE.", __FUNCTION__,__LINE__));
        CHK_RC(Drm_Err);
    }
    else {
        *keyHandlePtr = (uint32_t) pNode->pCmd->basicOut[1];
        *keyTypePtr   = (uint32_t) pNode->pCmd->basicOut[2];
    }

    /*printf("%s - success with Handle %d and type %d.\n", __FUNCTION__,*keyHandlePtr,*keyTypePtr);*/

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_Import_Sealed_Key(DrmNetFlixSageHandle    pHandle,
                                    uint8_t                *sealedKeyDataPtr,
                                    uint32_t                sealedKeySize,
                                    uint32_t               *keyHandlePtr,     /*out*/
                                    uint32_t               *algorithmPtr,     /*out*/
                                    uint32_t               *keyUsageFlagsPtr, /*out*/
                                    uint32_t               *keyTypePtr,       /*out*/
                                    uint32_t               *keySizePtr)       /*out*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    /* for output */
    NetflixKeyInfo_t keyInfo;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx               != NULL);
    BDBG_ASSERT(sealedKeyDataPtr   != NULL);
    BDBG_ASSERT(keyHandlePtr       != NULL);
    BDBG_ASSERT(algorithmPtr       != NULL);
    BDBG_ASSERT(keyUsageFlagsPtr   != NULL);
    BDBG_ASSERT(keyTypePtr         != NULL);
    BDBG_ASSERT(keySizePtr         != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;

    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], sealedKeyDataPtr, sealedKeySize);
    pNode->pCmd->blocks[0].len = sealedKeySize;

    /* setup for output */
    BKNI_Memset(&keyInfo, 0x0, sizeof(NetflixKeyInfo_t));
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], &keyInfo, sizeof(NetflixKeyInfo_t));
    pNode->pCmd->blocks[1].len = sizeof(NetflixKeyInfo_t);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_IMPORT_SEALED_KEY,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to import key to SAGE.", __FUNCTION__,__LINE__));
        CHK_RC(Drm_Err);
    }
    else if( pNode->pCmd->blocks[1].len != sizeof(NetflixKeyInfo_t)) {
        BDBG_ERR(("%s%d - incorrect output from SAGE, the output size %d.", __FUNCTION__,__LINE__,pNode->pCmd->blocks[1].len));
        CHK_RC(BSAGE_ERR_INTERNAL);
    }

    /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[1].len*/
    BKNI_Memcpy(&keyInfo, pNode->pCmd->blocks[1].data.ptr, pNode->pCmd->basicOut[2]);

    *keyHandlePtr     = keyInfo.key_handle;
    *keyTypePtr       = keyInfo.key_type;
    *algorithmPtr     = keyInfo.algorithm;
    *keyUsageFlagsPtr = keyInfo.key_usage_flags;
    *keySizePtr       = pNode->pCmd->basicOut[1];

    /*printf("%s:%d - success with key handle %d and type %d algo %d keyUsage %d keySize %d.\n", __FUNCTION__,__LINE__,
            *keyHandlePtr,*keyTypePtr,*algorithmPtr,*keyUsageFlagsPtr,*keySizePtr);
            */

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[1].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
        pNode->pCmd->blocks[1].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_Export_Key(DrmNetFlixSageHandle    pHandle,
                             uint32_t                keyHandle,
                             uint32_t                keyFormat,
                             uint8_t                *keyDataPtr, /*out*/
                             uint32_t               *keySizePtr) /*in|out*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));
    BDBG_LOG(("%s:%d - exporting key with handle %d", __FUNCTION__,__LINE__,keyHandle));

    BDBG_ASSERT(pCtx         != NULL);
    BDBG_ASSERT(keyDataPtr   != NULL);
    BDBG_ASSERT(keySizePtr   != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[2] = (uint32_t)keyHandle;
    pNode->pCmd->basicIn[3] = (uint32_t)keyFormat;

    /* for the output */
    BKNI_Memset(keyDataPtr, 0x0, *keySizePtr);
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], keyDataPtr, *keySizePtr);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_EXPORT_KEY,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to export key from SAGE. key handle %d", __FUNCTION__,__LINE__,keyHandle));
        CHK_RC(Drm_Err);
    }

    BKNI_Memcpy(keyDataPtr, pNode->pCmd->blocks[0].data.ptr, pNode->pCmd->blocks[0].len);

    /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[0].len*/
    *keySizePtr = pNode->pCmd->basicOut[1];

    /*printf("%s:%d - successfully export key for key handle %d keySize %d.\n", __FUNCTION__,__LINE__,keyHandle,*keySizePtr);*/

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_Export_Sealed_Key(DrmNetFlixSageHandle    pHandle,
                                    uint32_t                sealedKeyHandle,
                                    uint8_t                *sealedKeyDataPtr, /*out*/
                                    uint32_t               *maxKeySizePtr)    /*in|out*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx               != NULL);
    BDBG_ASSERT(sealedKeyDataPtr   != NULL);
    BDBG_ASSERT(maxKeySizePtr      != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[2] = (uint32_t)sealedKeyHandle;

    /* for the output */
    BKNI_Memset(sealedKeyDataPtr, 0x0, *maxKeySizePtr);
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], sealedKeyDataPtr, *maxKeySizePtr);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_EXPORT_SEALED_KEY,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to export the sealed key from SAGE. key handle %d", __FUNCTION__,__LINE__,sealedKeyHandle));
        CHK_RC(Drm_Err);
    }

    /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[0].len*/
    BKNI_Memcpy(sealedKeyDataPtr, pNode->pCmd->blocks[0].data.ptr, pNode->pCmd->basicOut[1]);
    *maxKeySizePtr = pNode->pCmd->basicOut[1];

    /*printf("%s:%d - successfully export key for key handle %d keySize %d.\n", __FUNCTION__,__LINE__,sealedKeyHandle,*maxKeySizePtr);*/

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_Get_Key_Info(DrmNetFlixSageHandle    pHandle,
                               uint32_t                keyHandle,
                               uint32_t               *keyTypePtr,        /*out*/
                               bool                   *extractable,       /*out*/
                               uint32_t               *algorithmPtr,      /*out*/
                               uint32_t               *keyUsageFlagsPtr)  /*out*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    /* for output */
    NetflixKeyInfo_t keyInfo;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx               != NULL);
    BDBG_ASSERT(keyTypePtr         != NULL);
    BDBG_ASSERT(extractable        != NULL);
    BDBG_ASSERT(algorithmPtr       != NULL);
    BDBG_ASSERT(keyUsageFlagsPtr   != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[2] = (uint32_t)keyHandle;

    /* for the output */
    BKNI_Memset(&keyInfo, 0x0, sizeof(NetflixKeyInfo_t));
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], &keyInfo, sizeof(NetflixKeyInfo_t));

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_GET_KEY_INFO,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to get the key info from SAGE. key handle %d", __FUNCTION__,__LINE__,keyHandle));
        CHK_RC(Drm_Err);
    }

    /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[1].len*/
    BKNI_Memcpy(&keyInfo, pNode->pCmd->blocks[0].data.ptr, pNode->pCmd->basicOut[1]);
    *keyTypePtr       = keyInfo.key_type;
    *algorithmPtr     = keyInfo.algorithm;
    *keyUsageFlagsPtr = keyInfo.key_usage_flags;
    *extractable      = (keyInfo.key_usage_flags & NFLX_EXTRACTABLE_KEY)?true:false;

    /*printf("%s:%d - key info for key handle (0x%x), keyType (0x%x), algorithm (0x%x), keyUsage (0x%x), extractable (0x%x).\n",
            __FUNCTION__,__LINE__,keyHandle,*keyTypePtr,*algorithmPtr,*keyUsageFlagsPtr,*extractable);
            */

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_Get_Named_Key(DrmNetFlixSageHandle    pHandle,
                                uint8_t                *namedKeyPtr,
                                uint32_t                namedKeySize,
                                uint32_t               *keyHandlePtr) /*out*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx          != NULL);
    BDBG_ASSERT(namedKeyPtr   != NULL);
    BDBG_ASSERT(keyHandlePtr  != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;

    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], namedKeyPtr, namedKeySize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_GET_NAMED_KEY,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to get the named key handle from SAGE", __FUNCTION__,__LINE__));
        CHK_RC(Drm_Err);
    }

    *keyHandlePtr = pNode->pCmd->basicOut[1];

    /*printf("%s:%d - successfully got the named key handle %d.\n", __FUNCTION__,__LINE__,*keyHandlePtr);*/

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }

    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_AES_CBC(DrmNetFlixSageHandle    pHandle,
                          uint32_t                keyHandle,
                          uint32_t                op,
                          uint8_t                *ivPtr,
                          uint32_t                ivDataSize,
                          uint8_t                *inDataPtr,
                          uint32_t                inDataSize,
                          uint8_t                *outDataPtr,   /*out*/
                          uint32_t               *outDataSize)  /*out*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx         != NULL);
    BDBG_ASSERT(ivPtr        != NULL);
    BDBG_ASSERT(inDataPtr    != NULL);
    BDBG_ASSERT(outDataPtr   != NULL);
    BDBG_ASSERT(outDataSize  != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    if( *outDataSize < inDataSize)
    {
        BDBG_ERR(("%s - output data buffer is too small", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[2] = (uint32_t)keyHandle;
    pNode->pCmd->basicIn[3] = (uint32_t)op;

    /* for the IV and the input data */
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], ivPtr, ivDataSize);
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], inDataPtr, inDataSize);
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[2], outDataPtr, *outDataSize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_AES_CBC,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to export key from SAGE. key handle %d", __FUNCTION__,__LINE__,keyHandle));
        CHK_RC(Drm_Err);
    }

    /* the output will be returned in the blocks[1] */
    BKNI_Memset(outDataPtr, 0, *outDataSize );
    *outDataSize = pNode->pCmd->basicOut[1];

    /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[2].len*/
    BKNI_Memcpy(outDataPtr, pNode->pCmd->blocks[2].data.ptr, *outDataSize);


    /*printf("%s:%d - successfully performed aes cbc for key handle %d out data size %d.\n", __FUNCTION__,__LINE__,keyHandle,*outDataSize);*/

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[1].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
        pNode->pCmd->blocks[1].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[2].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[2].data.ptr);
        pNode->pCmd->blocks[2].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_HMAC(DrmNetFlixSageHandle    pHandle,
                       uint32_t                hmacKeyHandle,
                       uint32_t                shaType,
                       uint8_t                *inDataPtr,
                       uint32_t                inDataSize,
                       uint8_t                *outDataPtr,   /*out*/
                       uint32_t               *outDataSize)  /*out*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx         != NULL);
    BDBG_ASSERT(inDataPtr    != NULL);
    BDBG_ASSERT(outDataPtr   != NULL);
    BDBG_ASSERT(outDataSize  != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[2] = (uint32_t)hmacKeyHandle;
    pNode->pCmd->basicIn[3] = (uint32_t)shaType;

    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], inDataPtr, inDataSize);

    /* for the output data */
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], outDataPtr, *outDataSize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_HMAC,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to export key from SAGE. key handle %d", __FUNCTION__,__LINE__,hmacKeyHandle));
        CHK_RC(Drm_Err);
    }

    /* the output will be returned in the blocks[1] */
    /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[1].len*/
    *outDataSize = pNode->pCmd->basicOut[1];
    BKNI_Memset(outDataPtr, 0, *outDataSize );
    BKNI_Memcpy(outDataPtr, pNode->pCmd->blocks[1].data.ptr, *outDataSize);


    BDBG_MSG(("%s:%d successfully performed hmac for the key handle 0x%p out data size %u", __FUNCTION__,__LINE__,hmacKeyHandle,*outDataSize));

ErrorExit:
    if (pNode) {
        if(pNode->pCmd->blocks[0].data.ptr) {
            SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
            pNode->pCmd->blocks[0].data.ptr = NULL;
        }
        if(pNode->pCmd->blocks[1].data.ptr) {
            SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
            pNode->pCmd->blocks[1].data.ptr = NULL;
        }
        RELEASE_COMMAND_CONTAINER(pNode);
    }
    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));
    return rc;
}

DrmRC DRM_Netflix_HMAC_Verify(DrmNetFlixSageHandle    pHandle,
                              uint32_t                hmacKeyHandle,
                              uint32_t                shaType,
                              uint8_t                *inDataPtr,
                              uint32_t                inDataSize,
                              uint8_t                *hmacDataPtr,
                              uint32_t                hmacDataSize,
                              bool                   *verified)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx         != NULL);
    BDBG_ASSERT(inDataPtr    != NULL);
    BDBG_ASSERT(hmacDataPtr  != NULL);
    BDBG_ASSERT(verified     != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[2] = (uint32_t)hmacKeyHandle;
    pNode->pCmd->basicIn[3] = (uint32_t)shaType;

    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], inDataPtr, inDataSize);
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], hmacDataPtr, hmacDataSize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_HMAC_VERIFY,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to export key from SAGE. key handle %d", __FUNCTION__,__LINE__,hmacKeyHandle));
        CHK_RC(Drm_Err);
    }

    /* the output will be returned in the blocks[1] */
    *verified = (bool)pNode->pCmd->basicOut[1];

    /*printf("%s:%d - successfully performed HMAC Verify for the key handle %d Verified %d.\n",
            __FUNCTION__,__LINE__,hmacKeyHandle,*verified);
            */

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[1].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
        pNode->pCmd->blocks[1].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_DH_Gen_Keys(DrmNetFlixSageHandle    pHandle,
                              /*uint32_t                generator,*/
                              uint8_t                *generatorPtr,
                              uint32_t                generatorSize,
                              uint8_t                *primePtr,
                              uint32_t                primeSize,
                              uint32_t               *pubKeyHandle,   /*out*/
                              uint32_t               *privKeyHandle)  /*out*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx          != NULL);
    BDBG_ASSERT(generatorPtr  != NULL);
    BDBG_ASSERT(primePtr      != NULL);
    BDBG_ASSERT(pubKeyHandle  != NULL);
    BDBG_ASSERT(privKeyHandle != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    /*pNode->pCmd->basicIn[2] = (uint32_t)generator;*/

    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], primePtr, primeSize);
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], generatorPtr, generatorSize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_DH_GEN_KEYS,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to generate DH keys from SAGE rc = (%d)", __FUNCTION__,__LINE__,rc));
        CHK_RC(Drm_Err);
    }

    /*
       basically, SAGE will create only a single key handle for both dh public and private
       therefore, we just return the same key handle for both parameters
     */
    *pubKeyHandle   = pNode->pCmd->basicOut[1];
    *privKeyHandle  = pNode->pCmd->basicOut[1];

    /*printf("%s:%d - successfully generated the DH keys, handle (%d)\n",
            __FUNCTION__,__LINE__,*pubKeyHandle);
            */

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[1].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
        pNode->pCmd->blocks[1].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_DH_Derive_Keys(DrmNetFlixSageHandle    pHandle,
                                 uint32_t                dhPrivKeyHandle,
                                 uint32_t                derivationKeyHandle,
                                 uint8_t                *peerPubKeyPtr,
                                 uint32_t                peerPubKeySize,
                                 uint32_t               *encKeyHandlePtr,   /*out*/
                                 uint32_t               *hmacKeyHandlePtr,  /*out*/
                                 uint32_t               *wrapKeyHandlePtr)  /*out*/
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx             != NULL);
    BDBG_ASSERT(peerPubKeyPtr    != NULL);
    BDBG_ASSERT(encKeyHandlePtr  != NULL);
    BDBG_ASSERT(hmacKeyHandlePtr != NULL);
    BDBG_ASSERT(wrapKeyHandlePtr != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[2] = (uint32_t)dhPrivKeyHandle;
    pNode->pCmd->basicIn[3] = (uint32_t)derivationKeyHandle;

    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], peerPubKeyPtr, peerPubKeySize);

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_DH_DERIVE_KEYS,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to derive keys from SAGE rc = (%d)", __FUNCTION__,__LINE__,rc));
        CHK_RC(Drm_Err);
    }

    *encKeyHandlePtr   = pNode->pCmd->basicOut[1];
    *hmacKeyHandlePtr  = pNode->pCmd->basicOut[2];
    *wrapKeyHandlePtr  = pNode->pCmd->basicOut[3];

    /*printf("%s:%d - successfully derived key Handles %d, %d and %d\n",
            __FUNCTION__,__LINE__,*encKeyHandlePtr,*hmacKeyHandlePtr,*wrapKeyHandlePtr);
            */

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

DrmRC DRM_Netflix_Delete_Key(DrmNetFlixSageHandle    pHandle,
                             uint32_t                keyHandle)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx             != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[2] = (uint32_t)keyHandle;

    BDBG_MSG(("%s - deleting key with handle (%d)", __FUNCTION__,keyHandle));
    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_DELETE_KEY,
                                         pNode->pCmd);
    if( sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc != Drm_Success) {
        BDBG_ERR(("%s%d - Failed to derive DH keys from SAGE rc = (%d)", __FUNCTION__,__LINE__,rc));
        CHK_RC(Drm_Err);
    }

    /*printf("%s:%d - successfully deleted the key with handle (%d)\n", __FUNCTION__,__LINE__,keyHandle);*/

ErrorExit:

    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    BDBG_MSG(("%s - Exiting, rc %d.", __FUNCTION__, rc));

    return rc;
}

/*******************************************************************************
// FUNCTION:
//  DRM_Netflix_Secure_Store_Op
//
// DESCRIPTION:
//  Perform SAGE Secure Store Operations, which is either encrypt or decrypt
//  the data in SAGE using SAGE SecureStore API.
//
// @param[in]         *pBuf       Pointer to the buffer of the data
// @param[in]          uiSize     Size of the data for the operations
// @param[out]       *pOutBuf     Pointer to the output buffer
// @param[in|out]    *pOutSize    Size of the encrypted string
//
// @return Drm_Success if the operation is successful or an error.
//
********************************************************************************/
DrmRC DRM_Netflix_Secure_Store_Op(DrmNetFlixSageHandle   pHandle,
                                  uint8_t               *pBuf,
                                  uint32_t               uiSize,
                                  uint8_t               *pOutBuf,
                                  uint32_t              *pOutSize,
                                  uint32_t               op)
{
    DrmRC                    rc = Drm_Success;
    Netflix_SageCommandNode *pNode;
    BERR_Code               sage_rc;
    DrmNetflixSageContext_t *pCtx = (DrmNetflixSageContext_t *)pHandle;

    BDBG_MSG(("%s - Entering", __FUNCTION__));

    BDBG_ASSERT(pCtx != NULL);
    BDBG_ASSERT(pBuf != NULL);
    BDBG_ASSERT(pOutBuf != NULL);
    BDBG_ASSERT(pOutSize != NULL);

    GET_COMMAND_CONTAINER(pNode, &pCtx->cmds, link, pCtx->mutex);

    if(pNode == NULL){
        BDBG_ERR(("%s - Out of SRAI command container", __FUNCTION__));
        CHK_RC(Drm_Err);
    }

    if( (op != e_NETFLIX_TL_ENCRYPT) && (op != e_NETFLIX_TL_DECRYPT))
    {
        BDBG_ERR(("%s - invalid operation 0x%x", __FUNCTION__,op));
        CHK_RC(Drm_Err);
    }

    pNode->pCmd->basicIn[0] = (uint32_t)nrd_version;
    pNode->pCmd->basicIn[1] = (uint32_t)pCtx->pSageNetflixCtx;
    pNode->pCmd->basicIn[2] = (uint32_t)op;

    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[0], pBuf, uiSize);
    pNode->pCmd->blocks[0].len = uiSize;
    SETUP_SHARED_BLOCK(pNode->pCmd->blocks[1], pOutBuf, *pOutSize);
    pNode->pCmd->blocks[1].len = *pOutSize;

    sage_rc = SRAI_Module_ProcessCommand(pCtx->pNetflixSageModuleHandle,
                                         NETFLIX_SECURE_STORE_OP,
                                         pNode->pCmd);
    if (sage_rc != BERR_SUCCESS) {
        BDBG_ERR(("%s - SRAI_Module_ProcessCommand() failed, %x", __FUNCTION__, sage_rc));
        CHK_RC(Drm_SraiModuleError);
    }

    rc = (DrmRC)pNode->pCmd->basicOut[0];
    if(rc == Drm_Success) {
        /*SWSECDRM-1658:sage informs the actual length in container->basicout[1] instead of  altering pNode->pCmd->blocks[1].len*/
        *pOutSize = pNode->pCmd->basicOut[1];
        BKNI_Memcpy(pOutBuf, pNode->pCmd->blocks[1].data.ptr, *pOutSize);
    }

ErrorExit:
    if(pNode->pCmd->blocks[0].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[0].data.ptr);
        pNode->pCmd->blocks[0].data.ptr = NULL;
    }
    if(pNode->pCmd->blocks[1].data.ptr) {
        SRAI_Memory_Free(pNode->pCmd->blocks[1].data.ptr);
        pNode->pCmd->blocks[1].data.ptr = NULL;
    }
    if(pNode) RELEASE_COMMAND_CONTAINER(pNode);

    return rc;
}
