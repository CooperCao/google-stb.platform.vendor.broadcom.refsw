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

#include "nexus_sage_module.h"
#include "priv/nexus_core.h" /* get access to g_pCoreHandles */
#include "priv/nexus_security_priv.h"
#include "bhsm.h"
#include "nexus_sage_image.h"
#include "priv/nexus_sage_priv.h"
#include "bsagelib_rai.h"
#include "bsagelib_client.h"
#include "nexus_security_client.h"

#include "bsagelib_boot.h"
#include "bkni.h"

#include "antirollback_module_ids.h"
#include "nexus_sage_image.h"
#include "nexus_sage_types.h"


BDBG_MODULE(nexus_sage_ar);

struct sageARInfo {
    BSAGElib_ClientHandle sagelibClientHandle;
    BSAGElib_RpcRemoteHandle hSagelibRpcPlatformHandle;
    BSAGElib_RpcRemoteHandle hSagelibRpcModuleHandle;
    BSAGElib_InOutContainer *sageContainer;
    uint32_t uiLastAsyncId;
    BKNI_EventHandle response;
    BKNI_EventHandle indication;
    struct
    {
        BSAGElib_RpcRemoteHandle sageRpcHandle;
        uint32_t indication_id;
        uint32_t value;
    } indicationData;

    /* TODO: Need to find a better home for Opening/Closing System Crit Platform*/
    BSAGElib_RpcRemoteHandle hSysCritPlatformHandle;

    uint8_t *pDmaDescriptor;

};

static struct sageARInfo *lHandle;

#define SAGERESPONSE_TIMEOUT 5000 /* in ms */

NEXUS_SageMemoryBlock ar_ta;         /* raw ta binary in memory */
NEXUS_SageMemoryBlock ar_db;         /* AR db binary in memory */

static void NEXUS_Sage_AR_P_SageResponseCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument
)
{
    BSTD_UNUSED(async_argument);
    BSTD_UNUSED(sageRpcHandle);

    BKNI_SetEvent_isr(lHandle->response);
    return;
}

static void NEXUS_Sage_AR_P_SageIndicationCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument,
    uint32_t indication_id,
    uint32_t value
)
{
    BSTD_UNUSED(async_argument);
    /* Save information for later use */
    lHandle->indicationData.sageRpcHandle = sageRpcHandle;
    lHandle->indicationData.indication_id = indication_id;
    lHandle->indicationData.value = value;

    BKNI_SetEvent_isr(lHandle->indication);

    return;
}

static BERR_Code NEXUS_Sage_AR_P_WaitForSage(int timeoutMsec)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_ResponseData data;

    if (lHandle->uiLastAsyncId == 0)
    {
        rc = BERR_SUCCESS;
        goto done;
    }

    /* Wait for response from sage  */
    rc = BKNI_WaitForEvent(lHandle->response, timeoutMsec);
    if (rc == BERR_TIMEOUT)
    {
        BDBG_ERR(("%s: Timeout (%dms) waiting for sage response from previous request",
            __FUNCTION__, SAGERESPONSE_TIMEOUT));
        rc = BERR_TRACE(rc);
        goto done;
    }
    else if (rc)
    {
        rc = BERR_TRACE(rc);
        goto done;
    }

    /* Get Sage response */
    rc = BSAGElib_Client_GetResponse(lHandle->sagelibClientHandle, &data);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto done;
    }

    if(data.rc != BERR_SUCCESS)
    {
        rc = data.rc;
        if(data.rc == BSAGE_ERR_PLATFORM_ID)
        {
            BDBG_WRN(("SAGE WARNING: Unknown Platform"));
        }
        else
        {
            BDBG_ERR(("SAGE ERROR: 0x%x (%s)", rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        }
    }

done:

    return rc;
}

void NEXUS_Sage_P_ARUninit(BSAGElib_eStandbyMode standbyMode)
{
    NEXUS_Error rc;

    if(!lHandle)
    {
        /* Nothing to do */
        return;
    }

    /* Close AR:Monitor module */
    if (lHandle->hSagelibRpcModuleHandle != NULL)
    {
        BSAGElib_Rai_Module_Uninit(lHandle->hSagelibRpcModuleHandle, &lHandle->uiLastAsyncId);
        NEXUS_Sage_AR_P_WaitForSage(SAGERESPONSE_TIMEOUT);
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
        BDBG_MSG(("Uninit & remove AR SAGE Module: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));
        lHandle->hSagelibRpcModuleHandle = NULL;
    }
    /* Close AR Platform */
    if(lHandle->hSagelibRpcPlatformHandle)
    {
        BSAGElib_Rai_Platform_Close(lHandle->hSagelibRpcPlatformHandle, &lHandle->uiLastAsyncId);
        NEXUS_Sage_AR_P_WaitForSage(SAGERESPONSE_TIMEOUT);
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        lHandle->hSagelibRpcPlatformHandle=NULL;
    }

    /***********************************************************************************************************/
    /* TODO: Need to find a better home for Opening/Closing System Crit Platform*/
    /* Close System Crit Platform */
    if(lHandle->hSysCritPlatformHandle)
    {
        /* For S3 transition, we can't close the platform, but we need to "clean resources" */
        /* "BSAGElib_Rpc_RemoveRemote" w/o a matching BSAGElib_Rai_Platform_Close will accomplish this */
        if(standbyMode!=BSAGElib_eStandbyModeDeepSleep)
        {
            BSAGElib_Rai_Platform_Close(lHandle->hSysCritPlatformHandle, &lHandle->uiLastAsyncId);
            NEXUS_Sage_AR_P_WaitForSage(SAGERESPONSE_TIMEOUT);
        }
        BSAGElib_Rpc_RemoveRemote(lHandle->hSysCritPlatformHandle);
        lHandle->hSysCritPlatformHandle=NULL;
    }
    /***********************************************************************************************************/

    if(ar_ta.buf != NULL){
        /* UnInstall AR TA bin */
        rc = BSAGElib_Rai_Platform_UnInstall(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_ANTIROLLBACK);
        if (rc != BERR_SUCCESS){
            BDBG_WRN(("Could not UnInstall AR TA "));
        }
        /* Need to reset event because Install API sends multiple commands to SAGE to install and this triggers multiple response events */
        BKNI_ResetEvent(lHandle->response);

        NEXUS_Memory_Free(ar_ta.buf);
        ar_ta.buf = NULL;
        ar_ta.len = 0;
    }

    if(ar_db.buf != NULL)
    {
        NEXUS_Memory_Free(ar_db.buf);
        ar_db.buf = NULL;
        ar_db.len = 0;
    }

    /* Free dma descriptor */
    if (lHandle->pDmaDescriptor)
    {
        BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle, lHandle->pDmaDescriptor);
        lHandle->pDmaDescriptor = NULL;
    }

    /* Free container */
    if(lHandle->sageContainer)
    {
        BSAGElib_Rai_Container_Free(lHandle->sagelibClientHandle, lHandle->sageContainer);
        lHandle->sageContainer=NULL;
    }

    /* Close BSage client */
    if(lHandle->sagelibClientHandle)
    {
        BSAGElib_CloseClient(lHandle->sagelibClientHandle);
        lHandle->sagelibClientHandle=NULL;
    }

    /* Destroy event(s) */
    if (lHandle->response)
    {
        BKNI_DestroyEvent(lHandle->response);
        lHandle->response = NULL;
    }

    if (lHandle->indication)
    {
        BKNI_DestroyEvent(lHandle->indication);
        lHandle->indication = NULL;
    }

    /* Free local info */
    BKNI_Free(lHandle);
    lHandle=NULL;
}

NEXUS_Error NEXUS_Sage_P_SystemCritRestartCheck(
    void *pSettings)
{
    BSAGElib_RpcRemoteHandle RpcModuleHandle;
    uint8_t *pMem=NULL;
    BERR_Code rc;
    BSAGElib_BootSettings *pBootSettings=(BSAGElib_BootSettings *)pSettings;

    BDBG_ASSERT(pBootSettings);

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));

    lHandle->sageContainer->basicIn[1]=FrameworkModule_CommandId_eHostRestartVerify;

    lHandle->sageContainer->blocks[0].len=pBootSettings->regionMapNum * sizeof(*pBootSettings->pRegionMap);
    pMem = BSAGElib_Rai_Memory_Allocate(lHandle->sagelibClientHandle, lHandle->sageContainer->blocks[0].len, BSAGElib_MemoryType_Global);
    BKNI_Memcpy(pMem, pBootSettings->pRegionMap, lHandle->sageContainer->blocks[0].len);
    lHandle->sageContainer->blocks[0].data.ptr=pMem;

    rc = BSAGElib_Rai_Module_Init(lHandle->hSysCritPlatformHandle, SystemCrit_ModuleId_eFramework,
                                lHandle->sageContainer, &RpcModuleHandle,  /*out */
                                (void *) lHandle, &lHandle->uiLastAsyncId /*out */);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing framework module, error [0x%x] '%s'",
                    rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        /* Handle will still be valid even if init failed.. clear handle since cleanup will
        * not know if uninit will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(RpcModuleHandle);
        RpcModuleHandle=NULL;
        goto EXIT;
    }

    /* wait for sage response */
    rc = NEXUS_Sage_AR_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if(rc!=BERR_SUCCESS)
    {
        goto EXIT;
    }

    rc=lHandle->sageContainer->basicOut[0];

    if(rc==BERR_LEAKED_RESOURCE)
    {
        BDBG_ERR(("Region info has CHANGED since SAGE was booted... SYSTEM CANNOT BE USED!"));
    }

EXIT:
    if(RpcModuleHandle)
    {
        if(rc==BERR_SUCCESS)
        {
            /* Only un-init if the init was succesfull */
            BSAGElib_Rai_Module_Uninit(RpcModuleHandle, &lHandle->uiLastAsyncId);
            NEXUS_Sage_AR_P_WaitForSage(SAGERESPONSE_TIMEOUT);
        }
        BSAGElib_Rpc_RemoveRemote(RpcModuleHandle);
    }

    if(pMem)
    {
        BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle, pMem);
    }

    return rc;
}

/* Some of the init needs to be delayed until SAGE is running */
/* TODO: Move some of this (platform open/init, module open/init, into
* more generic functions that can be used across nexus */
NEXUS_Error NEXUS_Sage_P_ARInit(NEXUS_SageModuleSettings *pSettings)
{
    BSAGElib_ClientSettings sagelibClientSettings;
    BERR_Code rc;

    /* Image Interface */
    void * img_context = NULL;
    BIMG_Interface img_interface;

    uint8_t *pDmaMemoryPool = NULL;
    unsigned size = 8*1024;

    NEXUS_SageImageHolder arTAImg =
        {"AR TA", SAGE_IMAGE_FirmwareID_eSage_TA_AR, NULL};

    NEXUS_SageImageHolder arDBImg =
        {"AR DB", SAGE_IMAGE_FirmwareID_eSage_AR_DB, NULL};

    if(lHandle){
        NEXUS_Sage_P_ARUninit(BSAGElib_eStandbyModeOn);
    }

    lHandle=BKNI_Malloc(sizeof(*lHandle));
    if(!lHandle)
    {
        BDBG_ERR(( "Error creating event(s)" ));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return NEXUS_NOT_INITIALIZED;
    }

    BKNI_Memset(lHandle, 0, sizeof(*lHandle));

    rc = BKNI_CreateEvent(&lHandle->response);
    rc |= BKNI_CreateEvent(&lHandle->indication);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(( "Error creating event(s)" ));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    /* Initialize IMG interface; used to pull out an image on the file system from the kernel. */
    rc = Nexus_SageModule_P_Img_Create(NEXUS_CORE_IMG_ID_SAGE, &img_context, &img_interface);
    if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - Cannot Create IMG", __FUNCTION__));
    }
    /* If chip type is ZB or customer specific, then the default IDs stand */
    if (g_NEXUS_sageModule.chipInfo.chipType == BSAGElib_ChipType_eZS) {
        arTAImg.id = SAGE_IMAGE_FirmwareID_eSage_TA_AR_Development;
    }

    ar_ta.buf = NULL;
    ar_ta.len = 0;
    arTAImg.raw = &ar_ta;

    /* Load AR TA into memory */
    rc = NEXUS_SageModule_P_Load(&arTAImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) {
        BDBG_WRN(("%s - Cannot Load IMG %s ", __FUNCTION__, arTAImg.name));
    }

    /* Open sagelib client */
    NEXUS_Sage_P_GetSAGElib_ClientSettings(&sagelibClientSettings);
    sagelibClientSettings.i_rpc.indicationRecv_isr = NEXUS_Sage_AR_P_SageIndicationCallback_isr;
    sagelibClientSettings.i_rpc.responseRecv_isr = NEXUS_Sage_AR_P_SageResponseCallback_isr;
    sagelibClientSettings.i_rpc.response_isr = NULL;
    rc = BSAGElib_OpenClient(g_NEXUS_sageModule.hSAGElib, &lHandle->sagelibClientHandle, &sagelibClientSettings);

    if(ar_ta.buf != NULL)
    {
        /* Install AR TA bin */
        rc = BSAGElib_Rai_Platform_Install(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_ANTIROLLBACK,
                            ar_ta.buf, ar_ta.len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_WRN(("Could not install AR TA binary, assuming it is pre-loaded - buff[0x%p], size[%lu]",
                                        ar_ta.buf, (unsigned long)ar_ta.len));

            /* fall through, assuming TA was already loaded*/
        }
        /* Need to reset event because Install API sends multiple commands to SAGE to install and this triggers multiple response events */
        BKNI_ResetEvent(lHandle->response);
    }

    /* Allocate a single container and reuse */
    lHandle->sageContainer = BSAGElib_Rai_Container_Allocate(lHandle->sagelibClientHandle);
    if (lHandle->sageContainer == NULL)
    {
        BDBG_ERR(("Error allocating BSAGElib_InOutContainer"));
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto EXIT;
    }

    /* Open AR platform */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    rc = BSAGElib_Rai_Platform_Open(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_ANTIROLLBACK,
                    lHandle->sageContainer, &lHandle->hSagelibRpcPlatformHandle,
                    (void *)lHandle, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error opening SAGE AR Platform, [%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_AR_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        if(rc == BSAGE_ERR_PLATFORM_ID)
        {
            /* Note warning, but don't return error (i.e. allow nexus to continue) */
            /* System will run w/ no secure buffers */
            BDBG_WRN(("AR will not be possible"));
        }
        /* Handle will still be valid even if open failed.. clear handle since cleanup will
                * not know if close will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        lHandle->hSagelibRpcPlatformHandle=NULL;
        goto EXIT;
    }

    BDBG_MSG(("Opened AR SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* No other consumer of this platform is allowed */
    if(lHandle->sageContainer->basicOut[0]!=BSAGElib_State_eUninit)
    {
        BDBG_ERR(("Platform already open"));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto EXIT;
    }

    /* Initialize platform */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));

    if(pSettings->imageExists[arDBImg.id])
    {
        ar_db.buf = NULL;
        ar_db.len = 0;
        arDBImg.raw = &ar_db;

        /* Load AR DB into memory */
        rc = NEXUS_SageModule_P_Load(&arDBImg, &img_interface, img_context);
        if(rc != NEXUS_SUCCESS) {
            BDBG_LOG(("%s - Cannot load AR database %s, AR will use builtin database ", __FUNCTION__, arDBImg.name));
        }
        else
        {
            /*If db exists, pass info to platform init*/
            lHandle->sageContainer->blocks[0].len = arDBImg.raw->len;
            lHandle->sageContainer->blocks[0].data.ptr = arDBImg.raw->buf;
        }
    }
    else
    {
        BDBG_LOG(("%s - Skipping AR Database load, file does not exist...", __FUNCTION__));
    }

    rc = BSAGElib_Rai_Platform_Init(lHandle->hSagelibRpcPlatformHandle, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE AR platform - error [0x%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(BERR_OS_ERROR);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_AR_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        goto EXIT;
    }
    else
    {
        BDBG_LOG(("Sage AR TA running with %s database version %u",
                lHandle->sageContainer->basicOut[1]?"Loadable":"Builtin",
                  lHandle->sageContainer->basicOut[0]));
    }
    BDBG_MSG(("Initialized AR SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /***********************************************************************************************************/
    /* TODO: Need to find a better home for Opening/Closing System Crit Platform*/
    /* Open System Critical  platform */

    /* Allocate enough memory for DMA descriptors to pass to SAGE */
    pDmaMemoryPool = BSAGElib_Rai_Memory_Allocate(lHandle->sagelibClientHandle, size, BSAGElib_MemoryType_Restricted);
    if (pDmaMemoryPool == NULL)
    {
        rc = NEXUS_UNKNOWN;
        BDBG_ERR(("%s - Error calling BSAGElib_Rai_Memory_Allocate()", __FUNCTION__));
        goto EXIT;
    }

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    lHandle->sageContainer->blocks[0].data.ptr = pDmaMemoryPool;
    lHandle->sageContainer->blocks[0].len = size;

    rc = BSAGElib_Rai_Platform_Open(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SYSTEM_CRIT,
                    lHandle->sageContainer, &lHandle->hSysCritPlatformHandle,
                    (void *)lHandle, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error opening SAGE System Crit Platform, [%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        goto EXIT;
    }

    /* Store the potiner, free it until Uninit() */
    lHandle->pDmaDescriptor = pDmaMemoryPool;
    pDmaMemoryPool = NULL;

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_AR_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        if(rc == BSAGE_ERR_PLATFORM_ID)
        {
            /* Note warning, but don't return error (i.e. allow nexus to continue) */
            BDBG_WRN(("Standby will not be possible"));
        }
        /* Handle will still be valid even if open failed.. clear handle since cleanup will
                * not know if close will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSysCritPlatformHandle);
        lHandle->hSysCritPlatformHandle=NULL;
        goto EXIT;
    }

    BDBG_MSG(("Opened System Crit SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* No other consumer of this platform is allowed */
    if(lHandle->sageContainer->basicOut[0]!=BSAGElib_State_eUninit)
    {
        BDBG_ERR(("Platform already open"));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto EXIT;
    }

    /* Initialize platform */
    rc = BSAGElib_Rai_Platform_Init(lHandle->hSysCritPlatformHandle, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE System Crit platform - error [0x%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(BERR_OS_ERROR);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_AR_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        goto EXIT;
    }
    BDBG_MSG(("Initialized System Crit SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));
    /***********************************************************************************************************/

    /* Init AR Monitor Module */
    /* Doesn't seem to be a way to query of a module has been initialized or not.... */
    /* TODO */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    rc = BSAGElib_Rai_Module_Init(lHandle->hSagelibRpcPlatformHandle, antirollback_ModuleId,
        lHandle->sageContainer, &lHandle->hSagelibRpcModuleHandle,  /*out */
        (void *) lHandle, &lHandle->uiLastAsyncId /*out */);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE AR Monitor module, error [0x%x] '%s'",
                rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        /* Handle will still be valid even if init failed.. clear handle since cleanup will
        * not know if uninit will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
        lHandle->hSagelibRpcModuleHandle=NULL;
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_AR_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    BDBG_MSG(("Initialized SAGE AR Module: receivedSageModuleHandle [%p], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, lHandle->uiLastAsyncId));

EXIT:

    BDBG_MSG(("SAGE AR init complete (0x%x)", rc));
    if (img_context) {
        Nexus_SageModule_P_Img_Destroy(img_context);
    }

    if (pDmaMemoryPool) {
        BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle, pDmaMemoryPool);
    }

    if (rc != BERR_SUCCESS) {
        if (lHandle->pDmaDescriptor) {
            BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle, lHandle->pDmaDescriptor);
            lHandle->pDmaDescriptor = NULL;
        }
    }

    return rc;
}
