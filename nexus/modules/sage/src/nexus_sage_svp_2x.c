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

#include "nexus_sage_module.h"
#include "priv/nexus_core.h" /* get access to g_pCoreHandles */
#include "priv/nexus_security_priv.h"
#include "bhsm.h"
#include "nexus_sage_image.h"
#include "priv/nexus_sage_priv.h"
#include "bsagelib_rai.h"
#include "bsagelib_client.h"

#include "bsagelib_boot.h"
#include "bkni.h"

#include "secure_video_module_ids.h"
#include "secure_video_command_ids.h"


BDBG_MODULE(nexus_sage_svp);

struct sageSvpInfo {
    BSAGElib_ClientHandle sagelibClientHandle;
    BSAGElib_RpcRemoteHandle hSagelibRpcPlatformHandle;
    BSAGElib_RpcRemoteHandle hSagelibRpcModuleHandle;
    BSAGElib_InOutContainer *sageContainer;
    uint32_t uiLastAsyncId;
    BKNI_EventHandle response;
    BKNI_EventHandle indication;
    BKNI_EventHandle initEvent;
    bool init; /* SVP delayed init complete */
    uint8_t *pCoreList;
    NEXUS_ThreadHandle hThread;
    struct
    {
        BSAGElib_RpcRemoteHandle sageRpcHandle;
        uint32_t indication_id;
        uint32_t value;
    } indicationData;
};

static struct sageSvpInfo *lHandle;

#define SAGERESPONSE_TIMEOUT 5000 /* in ms */


static void NEXUS_Sage_SVP_P_SageResponseCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument
)
{
    BSTD_UNUSED(async_argument);
    BSTD_UNUSED(sageRpcHandle);

    BKNI_SetEvent_isr(lHandle->response);
    return;
}

static void NEXUS_Sage_SVP_P_SageIndicationCallback_isr(
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

static BERR_Code NEXUS_Sage_SVP_P_WaitForSage(void)
{
    BERR_Code rc = BERR_SUCCESS;
    BSAGElib_ResponseData data;

    if (lHandle->uiLastAsyncId == 0)
    {
        rc = BERR_SUCCESS;
        goto done;
    }

    /* Wait for response from sage  */
    rc = BKNI_WaitForEvent(lHandle->response, SAGERESPONSE_TIMEOUT);
    if (rc == BERR_TIMEOUT)
    {
        BDBG_ERR(("%s: Timeout (%dms) waiting for sage response from previous request",
            BSTD_FUNCTION, SAGERESPONSE_TIMEOUT));
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

void NEXUS_Sage_P_SvpUninit(bool reset)
{
    BDBG_ASSERT(lHandle);

    if (lHandle->hThread)
    {
        NEXUS_Thread_Destroy(lHandle->hThread);
        lHandle->hThread = NULL;
    }

    /* Close SVP:Monitor module */
    if (lHandle->hSagelibRpcModuleHandle != NULL)
    {
        if(!reset) /* On a SAGE reset, don't try to communicate */
        {
            BSAGElib_Rai_Module_Uninit(lHandle->hSagelibRpcModuleHandle, &lHandle->uiLastAsyncId);
            NEXUS_Sage_SVP_P_WaitForSage();
            BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
            BDBG_MSG(("Uninit & remove SVP SAGE Module: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));
        }
        lHandle->hSagelibRpcModuleHandle = NULL;
    }

    /* Close SVP Platform */
    if(lHandle->hSagelibRpcPlatformHandle)
    {
        if(!reset) /* On a SAGE reset, don't try to communicate */
        {
            BSAGElib_Rai_Platform_Close(lHandle->hSagelibRpcPlatformHandle, &lHandle->uiLastAsyncId);
            NEXUS_Sage_SVP_P_WaitForSage();
            BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        }
        lHandle->hSagelibRpcPlatformHandle=NULL;
    }

    /* Free memory */
    if(lHandle->pCoreList)
    {
        BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle, lHandle->pCoreList);
        lHandle->pCoreList = NULL;
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

    if (lHandle->initEvent)
    {
        BKNI_DestroyEvent(lHandle->initEvent);
        lHandle->initEvent = NULL;
    }

    /* Free local info */
    BKNI_Free(lHandle);
    lHandle=NULL;
}

/* Some of the init needs to be delayed until SAGE is running */
/* TODO: Move some of this (platform open/init, module open/init, into
* more generic functions that can be used across nexus */
void NEXUS_Sage_P_SvpInitDelayed(void *pParam)
{
    BSAGElib_ClientSettings sagelibClientSettings;
    BERR_Code rc;

    BSTD_UNUSED(pParam);

    NEXUS_LockModule();

    /* Wait for sage to be up */
    rc = NEXUS_Sage_P_CheckSageBooted();

    if(!rc)
    {
        BDBG_ERR(("SAGE boot failure"));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    /* Open sagelib client */
    NEXUS_Sage_P_GetSAGElib_ClientSettings(&sagelibClientSettings);
    sagelibClientSettings.i_rpc.indicationRecv_isr = NEXUS_Sage_SVP_P_SageIndicationCallback_isr;
    sagelibClientSettings.i_rpc.responseRecv_isr = NEXUS_Sage_SVP_P_SageResponseCallback_isr;
    sagelibClientSettings.i_rpc.response_isr = NULL;
    rc = BSAGElib_OpenClient(g_NEXUS_sageModule.hSAGElib, &lHandle->sagelibClientHandle, &sagelibClientSettings);

    /* Allocate a single container and reuse */
    lHandle->sageContainer = BSAGElib_Rai_Container_Allocate(lHandle->sagelibClientHandle);
    if (lHandle->sageContainer == NULL)
    {
        BDBG_ERR(("Error allocating BSAGElib_InOutContainer"));
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto EXIT;
    }

    /* Open SVP platform */
    rc = BSAGElib_Rai_Platform_Open(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SECURE_VIDEO,
                    lHandle->sageContainer, &lHandle->hSagelibRpcPlatformHandle,
                    (void *)lHandle, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error opening SAGE SVP Platform, [%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SVP_P_WaitForSage();
    if (rc != BERR_SUCCESS)
    {
        if(rc == BSAGE_ERR_PLATFORM_ID)
        {
            /* Note warning, but don't return error (i.e. allow nexus to continue) */
            /* System will run w/ no secure buffers */
            BDBG_WRN(("SVP will not be possible"));
        }
        else
        {
            rc = BERR_TRACE(rc);
        }
        /* Handle will still be valid even if open failed.. clear handle since cleanup will
        * not know if close will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        lHandle->hSagelibRpcPlatformHandle=NULL;
        goto EXIT;
    }

    BDBG_MSG(("Opened SVP SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* No other consumer of this platform is allowed */
    if(lHandle->sageContainer->basicOut[0]!=BSAGElib_State_eUninit)
    {
        BDBG_ERR(("Platform already open"));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto EXIT;
    }

    /* Initialize platform */
    rc = BSAGElib_Rai_Platform_Init(lHandle->hSagelibRpcPlatformHandle, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE SVP platform - error [0x%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(BERR_OS_ERROR);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SVP_P_WaitForSage();
    if (rc != BERR_SUCCESS)
    {
        if(rc == BSAGE_ERR_SVP_INVALID_URR)
        {
            /* Note error, but don't return error (i.e. allow nexus to continue) */
            /* System will run w/ no access to CRR */
            BDBG_ERR(("INVALID URR(s) specified! Current SAGE binary requires secure picture buffers!"));
            BDBG_ERR(("Please correctly configure heaps to allow for SVP."));
            BDBG_ERR(("ALL ACCESS to NEXUS_VIDEO_SECURE_HEAP blocked. Video playback NOT possible!"));
        }
        else
        {
            rc = BERR_TRACE(rc);
        }
        goto EXIT;
    }
    BDBG_MSG(("Initialized SVP SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* Init SVP Monitor Module */
    /* Doesn't seem to be a way to query of a module has been initialized or not.... */
    /* TODO */
    rc = BSAGElib_Rai_Module_Init(lHandle->hSagelibRpcPlatformHandle, secure_video_ModuleId_ebvn_monitor,
        lHandle->sageContainer, &lHandle->hSagelibRpcModuleHandle,  /*out */
        (void *) lHandle, &lHandle->uiLastAsyncId /*out */);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE SVP Monitor module, error [0x%x] '%s'",
                rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        /* Handle will still be valid even if init failed.. clear handle since cleanup will
        * not know if uninit will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
        lHandle->hSagelibRpcModuleHandle=NULL;
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SVP_P_WaitForSage();
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    BDBG_MSG(("Initialized SAGE SVP Module: receivedSageModuleHandle [%p], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, lHandle->uiLastAsyncId));

    /* Allocate some shared memory */
    lHandle->pCoreList = BSAGElib_Rai_Memory_Allocate(lHandle->sagelibClientHandle, sizeof(BAVC_CoreList), BSAGElib_MemoryType_Global);

EXIT:
    /* Init complete */
    lHandle->init = true;
    BKNI_SetEvent(lHandle->initEvent);

    BDBG_MSG(("SAGE SVP init complete (0x%x)", rc));

    NEXUS_UnlockModule();
    return;
}

NEXUS_Error NEXUS_Sage_P_SvpInit(NEXUS_SageModuleInternalSettings *internalSettings)
{
    NEXUS_ThreadSettings thSettings;
    BERR_Code rc;

    BSTD_UNUSED(internalSettings);

    if(lHandle)
    {
        /* Must be a reset, sage-less cleanup */
        NEXUS_Sage_P_SvpUninit(true);
    }

    lHandle=BKNI_Malloc(sizeof(*lHandle));
    if(!lHandle)
    {
        return NEXUS_NOT_AVAILABLE;
    }

    BKNI_Memset(lHandle, 0, sizeof(*lHandle));

    rc = BKNI_CreateEvent(&lHandle->response);
    rc |= BKNI_CreateEvent(&lHandle->indication);
    rc |= BKNI_CreateEvent(&lHandle->initEvent);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(( "Error creating event(s)" ));
        rc = BERR_TRACE(rc);
        goto ERROR_EXIT;
    }

    /* Rest of init must wait for SAGE to be running */
    NEXUS_Thread_GetDefaultSettings(&thSettings);
    lHandle->hThread = NEXUS_Thread_Create("SAGE SVP Init", NEXUS_Sage_P_SvpInitDelayed,
                                           NULL, &thSettings);
    if (!lHandle->hThread)
    {
        BDBG_ERR(("NEXUS_Thread_Create(SAGE SVP Init) failed"));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto ERROR_EXIT;
    }

    return NEXUS_SUCCESS;

ERROR_EXIT:
    NEXUS_Sage_P_SvpUninit(false);

    return NEXUS_UNKNOWN;
}

/* Not to be called from within SAGE module itself */
NEXUS_Error NEXUS_Sage_AddSecureCores(const BAVC_CoreList *pCoreList, NEXUS_SageUrrType type)
{
	uint32_t coreListSize;
    NEXUS_Error rc = NEXUS_SUCCESS;

    NEXUS_LockModule();
    coreListSize=sizeof(*pCoreList);

    BSTD_UNUSED(type);

#ifdef NEXUS_SAGE_SVP_TEST
    NEXUS_Sage_P_SecureCores_test(pCoreList, true);
#endif

    if(!lHandle)
    {
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    if(!lHandle->init)
    {
        rc = BKNI_WaitForEvent(lHandle->initEvent, SAGERESPONSE_TIMEOUT);
        if (rc == BERR_TIMEOUT)
        {
            BDBG_ERR(("%s: Timeout (%dms) waiting for SVP Init",
                BSTD_FUNCTION, SAGERESPONSE_TIMEOUT));
            rc = BERR_TRACE(rc);
            goto EXIT;
        }
        else if (rc)
        {
            rc = BERR_TRACE(rc);
            goto EXIT;
        }
    }

    if(!lHandle->hSagelibRpcModuleHandle)
    {
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto EXIT;
    }

    coreListSize-=(2*sizeof(__typeof__(pCoreList->aeCores[0])));
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    BKNI_Memcpy(lHandle->pCoreList, pCoreList, sizeof(*pCoreList));
    lHandle->sageContainer->blocks[0].len = coreListSize;
    lHandle->sageContainer->basicIn[0]=SECURE_VIDEO_VER_ID;
    lHandle->sageContainer->basicIn[1]=true;
    lHandle->sageContainer->blocks[0].data.ptr = lHandle->pCoreList;
    /* TODO: Add some version # for debug/verification */

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
            bvn_monitor_CommandId_eSetCores, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, bvn_monitor_CommandId_eSetCores, lHandle->uiLastAsyncId));
    rc = NEXUS_Sage_SVP_P_WaitForSage();
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
    }

EXIT:
    NEXUS_UnlockModule();

    return rc;
}

/* Not to be called from within SAGE module itself */
void NEXUS_Sage_RemoveSecureCores(const BAVC_CoreList *pCoreList, NEXUS_SageUrrType type)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t coreListSize=sizeof(*pCoreList);

    NEXUS_LockModule();

    BSTD_UNUSED(type);
#ifdef NEXUS_SAGE_SVP_TEST
    NEXUS_Sage_P_SecureCores_test(pCoreList, false);
#endif

    if(!lHandle)
    {
        BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    if(!lHandle->init)
    {
        rc = BKNI_WaitForEvent(lHandle->initEvent, SAGERESPONSE_TIMEOUT);
        if (rc == BERR_TIMEOUT)
        {
            BDBG_ERR(("%s: Timeout (%dms) waiting for SVP Init",
                BSTD_FUNCTION, SAGERESPONSE_TIMEOUT));
            BERR_TRACE(NEXUS_NOT_INITIALIZED);
            goto EXIT;
        }
        else if (rc)
        {
            BERR_TRACE(NEXUS_NOT_INITIALIZED);
            goto EXIT;
        }
    }

    if(!lHandle->hSagelibRpcModuleHandle)
    {
        BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto EXIT;
    }

    coreListSize-=(2*sizeof(__typeof__(pCoreList->aeCores[0])));
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    BKNI_Memcpy(lHandle->pCoreList, pCoreList, sizeof(*pCoreList));
    lHandle->sageContainer->blocks[0].len = coreListSize;
    lHandle->sageContainer->basicIn[0]=SECURE_VIDEO_VER_ID;
    lHandle->sageContainer->basicIn[1]=false;
    lHandle->sageContainer->blocks[0].data.ptr = lHandle->pCoreList;
    /* TODO: Add some version # for debug/verification */

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
            bvn_monitor_CommandId_eSetCores, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, bvn_monitor_CommandId_eSetCores, lHandle->uiLastAsyncId));
    rc = NEXUS_Sage_SVP_P_WaitForSage();
    if (rc != BERR_SUCCESS)
    {
        BERR_TRACE(rc);
    }

EXIT:
    NEXUS_UnlockModule();
}

/* Dummy function for 3.x compatibility */
NEXUS_Error NEXUS_Sage_UpdateHeaps(void)
{
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Sage_P_SvpEnterS3(void)
{
    return NEXUS_SUCCESS;
}
void NEXUS_Sage_P_SvpStop(bool reset)
{
    BSTD_UNUSED(reset);
    return;
}
void NEXUS_Sage_P_ARUninit(BSAGElib_eStandbyMode standbyMode)
{
    BSTD_UNUSED(standbyMode);
}
/* Dummy function for 3.x compatibility */
NEXUS_Error NEXUS_Sage_SecureRemap(unsigned memcIndex, const BDTU_RemapSettings *pSettings)
{
    BSTD_UNUSED(memcIndex);
    BSTD_UNUSED(pSettings);
    return NEXUS_SUCCESS;
}

void NEXUS_Sage_P_BP3Uninit(void)
{
    return;
}

NEXUS_Error NEXUS_Sage_EnableHvd(void)
{
    BDBG_ERR(("%s: NOT SUPPORTED in SAGE 2.x", __FUNCTION__));
    return NEXUS_SUCCESS;
}

void NEXUS_Sage_DisableHvd(void)
{
    BDBG_ERR(("%s: NOT SUPPORTED in SAGE 2.x", __FUNCTION__));
    return;
}
