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
 *
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

#include "sarm_module_ids.h"
#include "nexus_sage_image.h"
#include "nexus_sage_types.h"
#include "sarm_command_ids.h"
#include "priv/nexus_sage_audio.h"
#include "sarm_priv.h"
#include "priv/nexus_rave_priv.h"
#include "bavc_xpt.h"

BDBG_MODULE(nexus_sage_sarm);

struct sageSARMInfo {
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
    uint32_t totalStreams;
};

static struct sageSARMInfo *lHandle;
BDBG_OBJECT_ID(NEXUS_SageAudio_P_Context);

#define SAGERESPONSE_TIMEOUT 10000 /* in ms */

NEXUS_SageMemoryBlock sarm_ta;       /* raw ta binary in memory */

static void NEXUS_Sage_SARM_P_SageResponseCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument
)
{
    BSTD_UNUSED(async_argument);
    BSTD_UNUSED(sageRpcHandle);

    BKNI_SetEvent_isr(lHandle->response);
    return;
}

static void NEXUS_Sage_SARM_P_SageIndicationCallback_isr(
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

static NEXUS_Error NEXUS_Sage_SARM_P_WaitForSage(int timeoutMsec)
{
    NEXUS_Error           rc = NEXUS_SUCCESS;
    BSAGElib_ResponseData data;

    if (lHandle->uiLastAsyncId == 0)
    {
        rc = NEXUS_SUCCESS;
        goto done;
    }

    /* Wait for response from sage  */
    rc = BKNI_WaitForEvent(lHandle->response, timeoutMsec);
    if (rc == NEXUS_TIMEOUT)
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
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto done;
    }

    if(data.rc != NEXUS_SUCCESS)
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

void NEXUS_Sage_P_SARMUninit(void)
{
    NEXUS_Error rc;

    if(!lHandle)
    {
        /* Nothing to do */
        return;
    }

    /* Close SARM Platform */
    if(lHandle->hSagelibRpcPlatformHandle)
    {
        BSAGElib_Rai_Platform_Close(lHandle->hSagelibRpcPlatformHandle, &lHandle->uiLastAsyncId);
        NEXUS_Sage_SARM_P_WaitForSage(SAGERESPONSE_TIMEOUT);
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        lHandle->hSagelibRpcPlatformHandle=NULL;
    }

    if(sarm_ta.buf != NULL){
        /* UnInstall SARM TA bin */
        rc = BSAGElib_Rai_Platform_UnInstall(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SARM);
        if (rc != NEXUS_SUCCESS){
            BDBG_WRN(("Could not UnInstall SARM TA "));
        }
        /* Need to reset event because Install API sends multiple commands to SAGE to install and this triggers multiple response events */
        BKNI_ResetEvent(lHandle->response);

        NEXUS_Memory_Free(sarm_ta.buf);
        sarm_ta.buf = NULL;
        sarm_ta.len = 0;
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


/* Some of the init needs to be delayed until SAGE is running */
NEXUS_Error NEXUS_Sage_P_SARMInit(NEXUS_SageModuleSettings *pSettings)
{
    BSAGElib_ClientSettings  sagelibClientSettings;
    NEXUS_Error              rc;
    void                    *img_context = NULL;
    BIMG_Interface           img_interface;
    NEXUS_SageImageHolder    sarmTAImg   = {"SARM TA",  SAGE_IMAGE_FirmwareID_eSage_TA_SARM,  NULL};
    BSTD_UNUSED(pSettings);

    if(lHandle)
    {
        NEXUS_Sage_P_SARMUninit();
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
    if (rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(( "Error creating event(s)" ));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    /* Initialize IMG interface; used to pull out an image on the file system from the kernel. */
    rc = Nexus_SageModule_P_Img_Create(NEXUS_CORE_IMG_ID_SAGE, &img_context, &img_interface);
    if (rc != NEXUS_SUCCESS) {
        BDBG_ERR(("%s - Cannot Create IMG", BSTD_FUNCTION));
    }
    /* If chip type is ZB or customer specific, then the default IDs stand */
    if (g_NEXUS_sageModule.chipInfo.chipType == BSAGElib_ChipType_eZS) {
        sarmTAImg.id = SAGE_IMAGE_FirmwareID_eSage_TA_SARM_Development;
    }

    sarm_ta.buf = NULL;
    sarm_ta.len = 0;
    sarmTAImg.raw = &sarm_ta;

    /* Load SARM TA into memory */
    rc = NEXUS_SageModule_P_Load(&sarmTAImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) {
        BDBG_WRN(("%s - Cannot Load IMG %s ", BSTD_FUNCTION, sarmTAImg.name));
    }

    /* Open sagelib client */
    NEXUS_Sage_P_GetSAGElib_ClientSettings(&sagelibClientSettings);
    sagelibClientSettings.i_rpc.indicationRecv_isr = NEXUS_Sage_SARM_P_SageIndicationCallback_isr;
    sagelibClientSettings.i_rpc.responseRecv_isr = NEXUS_Sage_SARM_P_SageResponseCallback_isr;
    sagelibClientSettings.i_rpc.response_isr = NULL;
    rc = BSAGElib_OpenClient(g_NEXUS_sageModule.hSAGElib, &lHandle->sagelibClientHandle, &sagelibClientSettings);

    if(sarm_ta.buf != NULL)
    {
        /* Install SARM TA bin */
        rc = BSAGElib_Rai_Platform_Install(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SARM,
                            sarm_ta.buf, sarm_ta.len);
        if (rc != NEXUS_SUCCESS)
        {
            BDBG_WRN(("Could not install SARM TA binary, assuming it is pre-loaded - buff[0x%p], size[%lu]",
                                        sarm_ta.buf, (unsigned long)sarm_ta.len));

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
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto EXIT;
    }

    /* Open SARM platform */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    rc = BSAGElib_Rai_Platform_Open(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SARM,
                    lHandle->sageContainer, &lHandle->hSagelibRpcPlatformHandle,
                    (void *)lHandle, &lHandle->uiLastAsyncId);
    if (rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("Error opening SAGE SARM Platform, [%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SARM_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != NEXUS_SUCCESS)
    {
        if(rc == BSAGE_ERR_PLATFORM_ID)
        {
            /* Note warning, but don't return error (i.e. allow nexus to continue) */
            /* System will run w/ no secure buffers */
            BDBG_WRN(("SARM not running"));
        }
        /* Handle will still be valid even if open failed.. clear handle since cleanup will
                * not know if close will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        lHandle->hSagelibRpcPlatformHandle=NULL;
        goto EXIT;
    }

    BDBG_MSG(("Opened SARM SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* No other consumer of this platform is allowed */
    if(lHandle->sageContainer->basicOut[0]!=BSAGElib_State_eUninit)
    {
        BDBG_ERR(("Platform already open"));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto EXIT;
    }

    /* Initialize platform */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));

    rc = BSAGElib_Rai_Platform_Init(lHandle->hSagelibRpcPlatformHandle, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE SARM platform - error [0x%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(NEXUS_OS_ERROR);
        goto EXIT;
    }
    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SARM_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != NEXUS_SUCCESS)
    {
        goto EXIT;
    }
    else
    {
        BDBG_LOG(("Sage SARM TA running status=%d", lHandle->sageContainer->basicOut[0]));
    }
    BDBG_MSG(("Initialized SARM SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* Initialize SARM Module */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    rc = BSAGElib_Rai_Module_Init (
            lHandle->hSagelibRpcPlatformHandle,
            SARM_ModuleId_eSARM,
            lHandle->sageContainer,
            &lHandle->hSagelibRpcModuleHandle,  /*out */
            (void *) lHandle,
            &lHandle->uiLastAsyncId /*out */);
    if (rc != NEXUS_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE SARM module, error [0x%x] '%s'",
                rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        /* Handle will still be valid even if init failed.. clear handle since cleanup will not know if uninit will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
        lHandle->hSagelibRpcModuleHandle=NULL;
        goto EXIT;
    }
    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SARM_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }
    BDBG_MSG(("Initialized SAGE SARM Module: receivedSageModuleHandle [%p], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, lHandle->uiLastAsyncId));


EXIT:
    BDBG_MSG(("SAGE SARM init complete (0x%x)", rc));
    if (img_context)
    {
        Nexus_SageModule_P_Img_Destroy(img_context);
    }
    return rc;
}

static NEXUS_Error NEXUS_Sage_Sarm_P_WaitForSage(int timeoutMsec)
{
    NEXUS_Error           rc = NEXUS_SUCCESS;
    BSAGElib_ResponseData data;

    if (lHandle->uiLastAsyncId == 0)
    {
        rc = NEXUS_SUCCESS;
        goto done;
    }

    /* Wait for response from sage  */
    rc = BKNI_WaitForEvent(lHandle->response, timeoutMsec);
    if (rc == NEXUS_TIMEOUT)
    {
        BDBG_ERR(("%s: Timeout (%dms) waiting for sage response from previous request",
            BSTD_FUNCTION, timeoutMsec));
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
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto done;
    }

    if(data.rc != NEXUS_SUCCESS)
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
    else
    {
        if (lHandle->sageContainer->basicOut[0] != NEXUS_SUCCESS)
        {
            rc = BERR_TRACE(lHandle->sageContainer->basicOut[0]);
        }
    }

done:

    return rc;
}


/*************************************************
 * SARM APIs - Sage Audio Routing and Monitoring *
 *************************************************/

/*
 * Open/Close routines
 */
/***************************************************
 * Summary:                                        *
 * Get Default SAGE Audio Processing Open Settings *
 ***************************************************/
void NEXUS_SageAudio_GetDefaultOpenSettings_priv(
    NEXUS_SageAudioOpenSettings *pSettings /* out */
    )
{
    if (!pSettings)
    {
        BDBG_ERR(("Null pointer"));
        return;
    }
}

/****************************************************
 * Summary:                                         *
 * Open a SAGE Audio Processor                      *
 * Separate Open for each audio stream is required. *
 ****************************************************/
NEXUS_SageAudioHandle NEXUS_SageAudio_Open_priv(
    const NEXUS_SageAudioOpenSettings *pSettings /* Pass NULL for default settings */
    )
{
    NEXUS_Error              rc         = NEXUS_INVALID_PARAMETER;
    BSAGElib_InOutContainer* container  = lHandle->sageContainer;
    NEXUS_SageAudioHandle    hSageAudio = NULL;

    if (!pSettings)
    {
        BDBG_MSG(("Using default settings"));
    }

    /* Allocate and initialize the handle */
    hSageAudio = (NEXUS_SageAudioHandle)
        BSAGElib_Rai_Memory_Allocate(lHandle->sagelibClientHandle,
                                     sizeof(*hSageAudio),
                                     BSAGElib_MemoryType_Global);
    if (!hSageAudio)
    {
        BDBG_ERR(("Failed to allocate memory!"));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto end;
    }
    BKNI_Memset(hSageAudio, 0, sizeof(*hSageAudio));
    BDBG_OBJECT_SET(hSageAudio, NEXUS_SageAudio_P_Context);

    hSageAudio->sageAudioStatus = (NEXUS_SageAudioStatus*)
        BSAGElib_Rai_Memory_Allocate(lHandle->sagelibClientHandle,
                                     sizeof(*hSageAudio->sageAudioStatus),
                                     BSAGElib_MemoryType_Global);
    if (!hSageAudio->sageAudioStatus)
    {
        BDBG_ERR(("Failed to allocate memory!"));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto end;
    }
    BKNI_Memset(hSageAudio->sageAudioStatus, 0, sizeof(*hSageAudio->sageAudioStatus));

    BKNI_Memset(container, 0, sizeof(*container));
    SARM_CONTAINER_ASYNC(container) = lHandle->uiLastAsyncId;

    /* Pass the shared memory for status update */
    SARM_CONTAINER_STATUS_BLOCK(container).len      = sizeof(*hSageAudio->sageAudioStatus);
    SARM_CONTAINER_STATUS_BLOCK(container).data.ptr = (uint8_t*)hSageAudio->sageAudioStatus;

    BDBG_LOG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x], block length [%d]",
              (void*)lHandle->hSagelibRpcModuleHandle, sarm_CommandId_eAudioOpen, lHandle->uiLastAsyncId, SARM_CONTAINER_STATUS_BLOCK(container).len));
    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
                                            sarm_CommandId_eAudioOpen,
                                            container,
                                            &lHandle->uiLastAsyncId);
    rc = NEXUS_Sage_Sarm_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        BDBG_ERR(("Failed waiting for SARM TA (%d)", rc));
        goto end;
    }

    rc = SARM_CONTAINER_CMD_RC(container);
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        BDBG_ERR(("Failed to Open audio stream to SARM TA (%d)", rc));
        goto end;
    }

    /* Save SARM ID in the handle */
    lHandle->totalStreams = SARM_CONTAINER_STREAM_CNT(container);
    hSageAudio->sarm_id   = SARM_CONTAINER_SARM_ID(container);

    BDBG_LOG(("%s: SARM ID: %d, Total Streams: %d",
              BSTD_FUNCTION, hSageAudio->sarm_id, lHandle->totalStreams));

end:
    if (rc != NEXUS_SUCCESS)
    {
        NEXUS_SageAudio_Close_priv(hSageAudio);
        hSageAudio = NULL;
    }
    return hSageAudio;
}

/************************************************
 * Summary:                                     *
 * Close SAGE Audio Processing                  *
 * It will release all the associated memory    *
 * - Handle allocated during Open               *
 * - Shared status memory allocated during Open *
 * Make sure to NULL the handle after this call *
 ************************************************/
void NEXUS_SageAudio_Close_priv(
    NEXUS_SageAudioHandle hSageAudio
    )
{
    NEXUS_Error              rc        = NEXUS_INVALID_PARAMETER;
    BSAGElib_InOutContainer* container = lHandle->sageContainer;

    BDBG_OBJECT_ASSERT(hSageAudio, NEXUS_SageAudio_P_Context);

    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, sarm_CommandId_eAudioClose, lHandle->uiLastAsyncId));

    /* Pass the close parameters to SAGE */
    BKNI_Memset(container, 0, sizeof(*container));
    SARM_CONTAINER_SARM_ID(container) = hSageAudio->sarm_id;

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
                                            sarm_CommandId_eAudioClose,
                                            container,
                                            &lHandle->uiLastAsyncId);
    rc = NEXUS_Sage_Sarm_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto end;
    }

    rc = SARM_CONTAINER_CMD_RC(container);
    if (rc == NEXUS_SUCCESS)
    {
        BDBG_LOG(("%s: Closed stream with SARM ID: %d, Total Streams: %d",
                  BSTD_FUNCTION, hSageAudio->sarm_id, lHandle->totalStreams));
    }
    else
    {
        BDBG_ERR(("%s: Error (%d) closing stream with SARM ID: %d, Total Streams: %d",
                  BSTD_FUNCTION, rc, hSageAudio->sarm_id, lHandle->totalStreams));
    }
    lHandle->totalStreams = SARM_CONTAINER_STREAM_CNT(container);

end:
    /* Release memory */
    if (hSageAudio)
    {
        if (hSageAudio->sageAudioStatus)
        {
            BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle,
                                     (uint8_t*)hSageAudio->sageAudioStatus);
        }
        BDBG_OBJECT_DESTROY(hSageAudio, NEXUS_SageAudio_P_Context);
        BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle,
                                 (uint8_t*)hSageAudio);
    }
    return;
}

/*
 * Start/Stop routines
 */
/*******************************************************
 * Summary:                                            *
 * Get Default SAGE Audio Processing Starting Settings *
 *******************************************************/
void NEXUS_SageAudio_GetDefaultStartSettings_priv(
    NEXUS_SageAudioStartSettings *pSettings /* out */
    )
{
    if (!pSettings)
    {
        BDBG_ERR(("Null pointer"));
        return;
    }

    /* Fill in default values */
    pSettings->routingOnly = false;
    return;
}

NEXUS_Error NEXUS_SageAudio_Start_priv(NEXUS_SageAudioHandle               hSageAudio,
                                       const NEXUS_SageAudioStartSettings* pSettings)
{
    NEXUS_Error                      rc           = NEXUS_INVALID_PARAMETER;
    BSAGElib_InOutContainer         *container    = lHandle->sageContainer;
    _P_NEXUS_SageAudioStartSettings *start_params = NULL;
    NEXUS_RaveStatus                 raveStatus;

    BDBG_OBJECT_ASSERT(hSageAudio, NEXUS_SageAudio_P_Context);

    if (!hSageAudio->sarm_id || !pSettings->codec || ((void*)pSettings->inContext == NULL ) || ((void*)pSettings->outContext == NULL))
    {
        BDBG_ERR(("Missing parameter(s): sarm_id=%d, codec=%d, inContext=%p, outContext=%p",
                  hSageAudio->sarm_id,
                  pSettings->codec,
                  (void*)pSettings->inContext,
                  (void*)pSettings->outContext));
        goto end;
    }

    /* Allocate and initialize the parameters */
    start_params = (_P_NEXUS_SageAudioStartSettings*)
        BSAGElib_Rai_Memory_Allocate(lHandle->sagelibClientHandle,
                                     sizeof(*start_params),
                                     BSAGElib_MemoryType_Global);
    if (!start_params)
    {
        BDBG_ERR(("Failed to allocate memory!"));
        rc = NEXUS_OUT_OF_SYSTEM_MEMORY;
        goto end;
    }

    /* Fill in the start_params */
    BKNI_Memset(start_params, 0, sizeof(*start_params));
    start_params->codec = pSettings->codec;
    start_params->routingOnly = pSettings->routingOnly;

    /* Get the context maps */
    NEXUS_Sage_P_Module_Lock_Transport();
    rc = NEXUS_Rave_GetStatus_priv(pSettings->inContext, &raveStatus);
    NEXUS_Sage_P_Module_Unlock_Transport();
    if (rc)
    {
        rc = BERR_TRACE(rc);
        goto end;

    }
    start_params->inContext = raveStatus.xptContextMap;

    /* Get the context maps */
    NEXUS_Sage_P_Module_Lock_Transport();
    rc = NEXUS_Rave_GetStatus_priv(pSettings->outContext, &raveStatus);
    NEXUS_Sage_P_Module_Unlock_Transport();
    if (rc)
    {
        rc = BERR_TRACE(rc);
        goto end;

    }
    start_params->outContext = raveStatus.xptContextMap;

    /* Pass the start parameters to SAGE */
    BKNI_Memset(container, 0, sizeof(*container));
    SARM_CONTAINER_SARM_ID(container)                     = hSageAudio->sarm_id;
    SARM_CONTAINER_START_PARAMS_BLOCK(container).len      = sizeof(*start_params);
    SARM_CONTAINER_START_PARAMS_BLOCK(container).data.ptr = (uint8_t*)start_params;

    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, sarm_CommandId_eAudioStart, lHandle->uiLastAsyncId));
    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
                                            sarm_CommandId_eAudioStart,
                                            container,
                                            &lHandle->uiLastAsyncId);
    rc = NEXUS_Sage_Sarm_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto end;
    }

end:
    if (start_params)
    {
        BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle,
                                 (uint8_t*)start_params);
    }
    return rc;
}

NEXUS_Error NEXUS_SageAudio_Stop_priv(NEXUS_SageAudioHandle hSageAudio)
{
    NEXUS_Error              rc        = NEXUS_INVALID_PARAMETER;
    BSAGElib_InOutContainer* container = lHandle->sageContainer;

    BDBG_OBJECT_ASSERT(hSageAudio, NEXUS_SageAudio_P_Context);

    BDBG_MSG(("Sending command to SAGE: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, sarm_CommandId_eAudioStop, lHandle->uiLastAsyncId));

    /* Pass the stop parameters to SAGE */
    BKNI_Memset(container, 0, sizeof(*container));
    SARM_CONTAINER_SARM_ID(container) = hSageAudio->sarm_id;

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
                                            sarm_CommandId_eAudioStop,
                                            container,
                                            &lHandle->uiLastAsyncId);
    rc = NEXUS_Sage_Sarm_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto end;
    }

end:
    return rc;
}

/*
 * Get SARM Status
 */
NEXUS_Error NEXUS_SageAudio_GetStatus_priv(NEXUS_SageAudioHandle  hSageAudio,
                                           NEXUS_SageAudioStatus* sageAudioStatus)
{
    NEXUS_Error rc = NEXUS_INVALID_PARAMETER;

    if ((!hSageAudio) || (!sageAudioStatus))
    {
        BDBG_ERR(("Invalid Parameters! handle=%p, status=%p",
                  (void*)hSageAudio, (void*)sageAudioStatus));
        goto end;
    }
    BDBG_OBJECT_ASSERT(hSageAudio, NEXUS_SageAudio_P_Context);

    /* Copy the status memory */
    *sageAudioStatus = *hSageAudio->sageAudioStatus;
end:
    return rc;
}
