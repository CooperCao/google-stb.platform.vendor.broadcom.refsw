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
#include "nexus_security_client.h"

#include "bsagelib_boot.h"
#include "bkni.h"

#include "nexus_sage_image.h"
#include "nexus_sage_types.h"

BDBG_MODULE(nexus_sage_secure_log);

typedef enum {
 nexus_sage_secure_log_null = 0,
 nexus_sage_secure_log_TA_found,
 nexus_sage_secure_log_certificate_found,
 nexus_sage_secure_log_module_inited,
 nexus_sage_secure_log_buffer_enabled,
 nexus_sage_secure_log_thread_started,
 nexus_sage_secure_log_capture_start,
 nexus_sage_secure_log_terminated
}nexus_sage_secure_log_status;

struct sageSecureLogInfo {
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
};

uint32_t secure_log_status,secure_log_captured,secure_log_saved,secure_log_capture_fail;
static struct sageSecureLogInfo *lHandle;

#define SAGERESPONSE_TIMEOUT 5000 /* in ms */

NEXUS_SageMemoryBlock secure_log_ta;         /* raw ta binary in memory */
NEXUS_SageMemoryBlock secure_log_certificate;         /* secureLog certificate binary in memory */

static void NEXUS_Sage_SecureLog_P_SageResponseCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument
)
{
    BSTD_UNUSED(async_argument);
    BSTD_UNUSED(sageRpcHandle);

    BKNI_SetEvent_isr(lHandle->response);
    return;
}

static void NEXUS_Sage_SecureLog_P_SageIndicationCallback_isr(
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

static BERR_Code NEXUS_Sage_SecureLog_P_WaitForSage(int timeoutMsec)
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

void NEXUS_Sage_P_SecureLog_Uninit(void)
{
    NEXUS_Error rc;

    if(!lHandle)
    {
        /* Nothing to do */
        return;
    }

    /* Close securelog module */
    if (lHandle->hSagelibRpcModuleHandle != NULL)
    {
        BSAGElib_Rai_Module_Uninit(lHandle->hSagelibRpcModuleHandle, &lHandle->uiLastAsyncId);
        NEXUS_Sage_SecureLog_P_WaitForSage(SAGERESPONSE_TIMEOUT);
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
        BDBG_MSG(("Uninit & remove SecureLog SAGE Module: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));
        lHandle->hSagelibRpcModuleHandle = NULL;
    }
    /* Close SecureLog Platform */
    if(lHandle->hSagelibRpcPlatformHandle)
    {
        BSAGElib_Rai_Platform_Close(lHandle->hSagelibRpcPlatformHandle, &lHandle->uiLastAsyncId);
        NEXUS_Sage_SecureLog_P_WaitForSage(SAGERESPONSE_TIMEOUT);
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        lHandle->hSagelibRpcPlatformHandle=NULL;
    }

    if(secure_log_ta.buf != NULL){
        /* UnInstall SecureLog TA bin */
        rc = BSAGElib_Rai_Platform_UnInstall(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SECURE_LOGGING);
        if (rc != BERR_SUCCESS){
            BDBG_WRN(("Could not UnInstall SecureLog TA "));
        }
        /* Need to reset event because Install API sends multiple commands to SAGE to install and this triggers multiple response events */
        BKNI_ResetEvent(lHandle->response);

        NEXUS_Memory_Free(secure_log_ta.buf);
        secure_log_ta.buf = NULL;
        secure_log_ta.len = 0;
    }

    if(secure_log_certificate.buf != NULL)
    {
        NEXUS_Memory_Free(secure_log_certificate.buf);
        secure_log_certificate.buf = NULL;
        secure_log_certificate.len = 0;
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

NEXUS_Error NEXUS_Sage_P_SecureLog_Init(const NEXUS_SageModuleSettings *pSettings)
{
    BSAGElib_ClientSettings sagelibClientSettings;
    BERR_Code rc;

    /* Image Interface */
    void * img_context = NULL;
    BIMG_Interface img_interface;

    NEXUS_SageImageHolder secureLogTAImg =
        {"SecureLog TA", SAGE_IMAGE_FirmwareID_eSage_TA_SECURE_LOG, NULL};

    NEXUS_SageImageHolder secureLogCertificateImg =
        {"SecureLog Certificate", SAGE_IMAGE_FirmwareID_eSage_SECURE_LOG_Certificate, NULL};

    if(lHandle){
        NEXUS_Sage_P_SecureLog_Uninit();
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
        BDBG_ERR(("%s - Cannot Create IMG", BSTD_FUNCTION));
        goto EXIT;
    }
    /* If chip type is ZB or customer specific, then the default IDs stand */
    if (g_NEXUS_sageModule.chipInfo.chipType == BSAGElib_ChipType_eZS) {
        secureLogTAImg.id = SAGE_IMAGE_FirmwareID_eSage_TA_SECURE_LOG_Development;
    }

    if(!pSettings->imageExists[secureLogTAImg.id])
    {
        BDBG_MSG(("%s - IMG %s Does not exist", BSTD_FUNCTION, secureLogTAImg.name));
        rc = NEXUS_NOT_AVAILABLE;
        goto EXIT;
    }
    secure_log_status = nexus_sage_secure_log_TA_found;

    if(!pSettings->imageExists[secureLogCertificateImg.id])
    {
        BDBG_MSG(("%s - IMG %s Does not exist", BSTD_FUNCTION, secureLogCertificateImg.name));
        rc = NEXUS_NOT_AVAILABLE;
        goto EXIT;
    }
    secure_log_status = nexus_sage_secure_log_certificate_found;

    secure_log_ta.buf = NULL;
    secure_log_ta.len = 0;
    secureLogTAImg.raw = &secure_log_ta;

    /* Load SecureLog TA into memory */
    rc = NEXUS_SageModule_P_Load(&secureLogTAImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) {
        BDBG_WRN(("%s - Cannot Load IMG %s ", BSTD_FUNCTION, secureLogTAImg.name));
    }

    /* Open sagelib client */
    NEXUS_Sage_P_GetSAGElib_ClientSettings(&sagelibClientSettings);
    sagelibClientSettings.i_rpc.indicationRecv_isr = NEXUS_Sage_SecureLog_P_SageIndicationCallback_isr;
    sagelibClientSettings.i_rpc.responseRecv_isr = NEXUS_Sage_SecureLog_P_SageResponseCallback_isr;
    sagelibClientSettings.i_rpc.response_isr = NULL;
    rc = BSAGElib_OpenClient(g_NEXUS_sageModule.hSAGElib, &lHandle->sagelibClientHandle, &sagelibClientSettings);

    if(secure_log_ta.buf != NULL)
    {
        /* Install secureLog TA bin */
        rc = BSAGElib_Rai_Platform_Install(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SECURE_LOGGING,
                            secure_log_ta.buf, secure_log_ta.len);
        if (rc != BERR_SUCCESS)
        {
            BDBG_WRN(("Could not install SecureLog TA binary, assuming it is pre-loaded - buff[0x%p], size[%lu]",
                                        secure_log_ta.buf, (unsigned long)secure_log_ta.len));

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

    /* Open SecureLog platform */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    rc = BSAGElib_Rai_Platform_Open(lHandle->sagelibClientHandle, BSAGE_PLATFORM_ID_SECURE_LOGGING,
                    lHandle->sageContainer, &lHandle->hSagelibRpcPlatformHandle,
                    (void *)lHandle, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error opening SAGE SecureLog Platform, [%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SecureLog_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        if(rc == BSAGE_ERR_PLATFORM_ID)
        {
            /* Note warning, but don't return error (i.e. allow nexus to continue) */
            /* System will run w/ no secure buffers */
            BDBG_WRN(("SecureLog will not be possible"));
        }
        /* Handle will still be valid even if open failed.. clear handle since cleanup will
                * not know if close will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcPlatformHandle);
        lHandle->hSagelibRpcPlatformHandle=NULL;
        goto EXIT;
    }

    BDBG_MSG(("Opened SecureLog SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* No other consumer of this platform is allowed */
    if(lHandle->sageContainer->basicOut[0]!=BSAGElib_State_eUninit)
    {
        BDBG_ERR(("Platform already open"));
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto EXIT;
    }

    rc = BSAGElib_Rai_Platform_Init(lHandle->hSagelibRpcPlatformHandle, lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE SecureLog platform - error [0x%x] '%s'",
                  rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(BERR_OS_ERROR);
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SecureLog_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        goto EXIT;
    }

    BDBG_MSG(("Initialized SecureLog SAGE platform: assignedAsyncId [0x%x]", lHandle->uiLastAsyncId));

    /* Init SecureLog Monitor Module */
    /* Doesn't seem to be a way to query of a module has been initialized or not.... */
    /* TODO */
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    rc = BSAGElib_Rai_Module_Init(lHandle->hSagelibRpcPlatformHandle, Secure_Log_ModuleId,
        lHandle->sageContainer, &lHandle->hSagelibRpcModuleHandle,  /*out */
        (void *) lHandle, &lHandle->uiLastAsyncId /*out */);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error initializing SAGE SecureLog Monitor module, error [0x%x] '%s'",
                rc, BSAGElib_Tools_ReturnCodeToString(rc)));
        BERR_TRACE(rc);
        /* Handle will still be valid even if init failed.. clear handle since cleanup will
        * not know if uninit will need to be called or not */
        BSAGElib_Rpc_RemoveRemote(lHandle->hSagelibRpcModuleHandle);
        lHandle->hSagelibRpcModuleHandle=NULL;
        goto EXIT;
    }

    /* wait for sage response before proceeding */
    rc = NEXUS_Sage_SecureLog_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }
    secure_log_status = nexus_sage_secure_log_module_inited;

    BDBG_MSG(("Initialized SAGE SecureLog Module: receivedSageModuleHandle [%p], assignedAsyncId [0x%x]",
              (void*)lHandle->hSagelibRpcModuleHandle, lHandle->uiLastAsyncId));

    secure_log_certificate.buf = NULL;
    secure_log_certificate.len = 0;
    secureLogCertificateImg.raw = &secure_log_certificate;

    /* Load SecureLog TA into memory */
    rc = NEXUS_SageModule_P_Load(&secureLogCertificateImg, &img_interface, img_context);
    if(rc != NEXUS_SUCCESS) {
        BDBG_WRN(("%s - Cannot Load IMG %s ", BSTD_FUNCTION, secureLogCertificateImg.name));
    }

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    lHandle->sageContainer->blocks[0].len = secure_log_certificate.len;
    lHandle->sageContainer->blocks[0].data.ptr = (uint8_t* )secure_log_certificate.buf;

    lHandle->sageContainer->basicIn[0] = SAGE_SECURE_LOG_BUFFER_SIZE;

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
                                            Secure_Log_CommandId_eConfigureBuffer,
                                            lHandle->sageContainer,&lHandle->uiLastAsyncId);
    BDBG_MSG(("COnfig secureLog buffer: sageModuleHandle [%p], commandId [%d], assignedAsyncId [0x%x]",
                      (void*)lHandle->hSagelibRpcModuleHandle, Secure_Log_CommandId_eConfigureBuffer, lHandle->uiLastAsyncId));
    if (rc != BERR_SUCCESS)
    {
        rc=BERR_TRACE(rc);
        goto EXIT;
    }
    rc = NEXUS_Sage_SecureLog_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc=BERR_TRACE(rc);
        goto EXIT;
    }
    BDBG_MSG(("Configured secureLog Buffer rc=%d", lHandle->sageContainer->basicOut[0]));

    rc = lHandle->sageContainer->basicOut[0];
    if (rc != BERR_SUCCESS)
    {
        rc=BERR_TRACE(rc);
        goto EXIT;
    }

    secure_log_status = nexus_sage_secure_log_buffer_enabled;

    /* SSF(sage framework) is not a TA, do not have a TA_ID,use 0 as TA_ID in NEXUS_Sage_SecureLog_Attach()
     Attch(0) will attach SSF to sage secure log buffer. */
    NEXUS_Sage_SecureLog_Attach(0);

    NEXUS_Sage_SecureLog_Attach(BSAGE_PLATFORM_ID_SYSTEM);

    NEXUS_Sage_SecureLog_Attach(BSAGE_PLATFORM_ID_SYSTEM_CRIT);

    NEXUS_Sage_SecureLog_Attach(BSAGE_PLATFORM_ID_ANTIROLLBACK);

    NEXUS_Sage_SecureLog_Attach(BSAGE_PLATFORM_ID_SECURE_LOGGING);

    BDBG_MSG(("SAGE SecureLog init complete (0x%x)", rc));
EXIT:
    if(rc != BERR_SUCCESS){
        NEXUS_Sage_P_SecureLog_Uninit();
    }
    if (img_context) {
        Nexus_SageModule_P_Img_Destroy(img_context);
    }
    return rc;
}

NEXUS_Error NEXUS_Sage_SecureLog_Attach(uint32_t TA_Id)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(!lHandle)
    {
        BDBG_MSG(( "%s SecureLog not initialized !",BSTD_FUNCTION));
        rc = NEXUS_NOT_INITIALIZED;
        goto EXIT;
    }

    BKNI_ResetEvent(lHandle->response);

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    lHandle->sageContainer->basicIn[0]=TA_Id;

    BDBG_MSG(("Sending command Secure_Log_CommandId_eAttach: TA_Id 0x%x",TA_Id));

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
                                            Secure_Log_CommandId_eAttach,
                                            lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    rc = NEXUS_Sage_SecureLog_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

EXIT:
    return rc;
}

void NEXUS_Sage_SecureLog_Detach(uint32_t TA_Id)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(!lHandle)
    {
        BDBG_MSG(( "%s SecureLog not initialized !",BSTD_FUNCTION));
        rc = NEXUS_NOT_INITIALIZED;
        goto EXIT;
    }

    BKNI_ResetEvent(lHandle->response);

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));
    lHandle->sageContainer->basicIn[0]=TA_Id;

    BDBG_MSG(("Sending command Secure_Log_CommandId_eDetach: TA_Id 0x%x",TA_Id));

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
                                            Secure_Log_CommandId_eDetach,
                                            lHandle->sageContainer, &lHandle->uiLastAsyncId);

    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    rc = NEXUS_Sage_SecureLog_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

EXIT:
    return;
}

NEXUS_Error NEXUS_Sage_SecureLog_GetBuffer(
                        void       *pSecureLogBuffCtx0,/* attr{memory=cached;null_allowed=y} */
                        uint32_t   logBuffCtxSize,
                        void       *pLogBufferAddr0, /* attr{memory=cached;null_allowed=y} */
                        uint32_t   logBufferSize,
    NEXUS_Sage_SecureLog_BufferId  bufferId
                            )
{
    NEXUS_Sage_Secure_Log_TlBufferContext *pSecureLogBuffCtx = (NEXUS_Sage_Secure_Log_TlBufferContext *)pSecureLogBuffCtx0;
    uint8_t *pLogBufferAddr = pLogBufferAddr0;
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(!lHandle)
    {
        BDBG_ERR(( "%s SecureLog not init !",BSTD_FUNCTION));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return NEXUS_NOT_INITIALIZED;
    }

    if(logBuffCtxSize != sizeof(NEXUS_Sage_Secure_Log_TlBufferContext))
    {
        BDBG_ERR(( "%s logBuffCtxSize(0x%x) wrong, should be 0x%x !",BSTD_FUNCTION,
               logBuffCtxSize,(unsigned int)sizeof(NEXUS_Sage_Secure_Log_TlBufferContext)));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return NEXUS_NOT_INITIALIZED;
    }

    if(logBufferSize != SAGE_SECURE_LOG_BUFFER_SIZE)
    {
        BDBG_ERR(( "%s logBufferSize(0x%x) wrong, should be 0x%x !",BSTD_FUNCTION,
                logBufferSize,SAGE_SECURE_LOG_BUFFER_SIZE));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        return NEXUS_NOT_INITIALIZED;
    }

    BKNI_ResetEvent(lHandle->response);

    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));

    lHandle->sageContainer->basicIn[0] = bufferId;
    lHandle->sageContainer->blocks[0].len = logBuffCtxSize;
    lHandle->sageContainer->blocks[0].data.ptr = (uint8_t*) pSecureLogBuffCtx;

    lHandle->sageContainer->blocks[1].len = logBufferSize;
    lHandle->sageContainer->blocks[1].data.ptr = (uint8_t*)pLogBufferAddr;

    BDBG_MSG(("Sending command Secure_Log_CommandId_eGetBuffer, bufferId %d,"
               "SecureBuffCtx %p, Size 0x%x,"
               "SecureLogBuff %p, Size 0x%x",bufferId,
               (void *)pSecureLogBuffCtx, logBuffCtxSize, pLogBufferAddr,logBufferSize));

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
                                            Secure_Log_CommandId_eGetBuffer,
                                            lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        secure_log_capture_fail++;
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    rc = NEXUS_Sage_SecureLog_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        secure_log_capture_fail++;
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    if(pSecureLogBuffCtx->secHead.secure_logtotal_cnt > 0)
    {
        /* non-empty should be saved to secure_log.bin in nexus_platform_sage_secure_log.c*/
        secure_log_saved++;
    }
EXIT:
    secure_log_status = nexus_sage_secure_log_capture_start;
    secure_log_captured++;
    return rc;
}

NEXUS_Error
NEXUS_Sage_SecureLog_StartCaptureOK(void)
{
    if(!lHandle)
    {
        return NEXUS_NOT_INITIALIZED;
    }

    if(secure_log_status < nexus_sage_secure_log_buffer_enabled)
    {
        return NEXUS_NOT_INITIALIZED;
    }

    if(secure_log_status == nexus_sage_secure_log_buffer_enabled)
    {
        /* this call should be from NEXUS_Platform_P_SageSecureLog
         checking if secure log is enabled.
         so we can say NEXUS_Platform_P_SageSecureLog is up */
         secure_log_status = nexus_sage_secure_log_thread_started;
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Sage_SecureLog_P_GetAttachedTAs(uint32_t *pPlatFormId,uint32_t *pPlatformIdSize,uint32_t bufferId)
{
    NEXUS_Error rc = NEXUS_SUCCESS;

    if(!lHandle)
    {
        BDBG_ERR(( "%s SecureLog not init !",BSTD_FUNCTION));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    if (pPlatFormId == NULL)
    {
        BDBG_ERR(( "%s pPlatform is NULL !",BSTD_FUNCTION));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    if (*pPlatformIdSize == 0)
    {
        BDBG_ERR(( "%s platform size is 0 !",BSTD_FUNCTION));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    BKNI_ResetEvent(lHandle->response);
    BKNI_Memset(lHandle->sageContainer, 0, sizeof(*lHandle->sageContainer));

    lHandle->sageContainer->basicIn[0] = bufferId;
    lHandle->sageContainer->blocks[0].data.ptr = (uint8_t *)pPlatFormId;
    lHandle->sageContainer->blocks[0].len      = *pPlatformIdSize;

    BDBG_MSG(("Sending command Secure_Log_CommandId_eGetBufferTA: bufferId %d,"
              "pPlatFormIds %p platformSize %d",bufferId,(void *)pPlatFormId,*pPlatformIdSize));

    rc = BSAGElib_Rai_Module_ProcessCommand(lHandle->hSagelibRpcModuleHandle,
                                            Secure_Log_CommandId_eGetBufferTA,
                                            lHandle->sageContainer, &lHandle->uiLastAsyncId);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    rc = NEXUS_Sage_SecureLog_P_WaitForSage(SAGERESPONSE_TIMEOUT);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    *pPlatformIdSize = lHandle->sageContainer->basicOut[1];
    rc = lHandle->sageContainer->basicOut[0];

EXIT:
    return rc;
}

BDBG_FILE_MODULE(nexus_sage_module);

void NEXUS_Sage_SecureLog_P_PrintAttachedTAs(void)
{
#define PRINT_SIZE 200
    NEXUS_Error rc = NEXUS_SUCCESS;
    uint32_t *pPlatFormId = NULL;
    uint32_t platformIdSize = 100,platformIdCnt = 100, i;
    char output[PRINT_SIZE];
    int cnt;

    if(!lHandle)
    {
        BDBG_ERR(( "%s SecureLog not init !",BSTD_FUNCTION));
        rc = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto EXIT;
    }

    pPlatFormId = (uint32_t *)BSAGElib_Rai_Memory_Allocate(
            lHandle->sagelibClientHandle,
            platformIdSize,
            BSAGElib_MemoryType_Global);
    if (pPlatFormId == NULL)
    {
        BDBG_ERR(("failed to allocate mem for pPlatform"));
        rc = NEXUS_NOT_AVAILABLE;
        goto EXIT;
    }

    rc = NEXUS_Sage_SecureLog_P_GetAttachedTAs(pPlatFormId,&platformIdCnt,NEXUS_Sage_SecureLog_BufferId_eFirstAvailable);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto EXIT;
    }

    cnt = BKNI_Snprintf(output,PRINT_SIZE,"Attached %d TAs:",platformIdCnt);
    for(i=0;i<platformIdSize && i < platformIdCnt;i++)
    {
        cnt += BKNI_Snprintf(output + cnt,PRINT_SIZE-cnt," 0x%x,",pPlatFormId[i]);
    }
    BDBG_MODULE_LOG(nexus_sage_module, ("%s",output));
EXIT:
    if(pPlatFormId)
    {
        BSAGElib_Rai_Memory_Free(lHandle->sagelibClientHandle, (uint8_t *)pPlatFormId);
    }
    return;
}

void NEXUS_Sage_P_PrintSecureLog(void)
{
    switch(secure_log_status)
    {
    case nexus_sage_secure_log_null:
        BDBG_MODULE_LOG(nexus_sage_module, ("SecureLog not initialized, to look for securelog TA!"));
        break;
    case nexus_sage_secure_log_TA_found:
        BDBG_MODULE_LOG(nexus_sage_module, ("SecureLog TA found, looking for certificate"));
        break;
    case nexus_sage_secure_log_certificate_found:
        BDBG_MODULE_LOG(nexus_sage_module, ("SecureLog certificate found, to init module!"));
        break;
    case nexus_sage_secure_log_module_inited:
        BDBG_MODULE_LOG(nexus_sage_module, ("SecureLog module inited, to enabled logging!"));
        break;
    case nexus_sage_secure_log_buffer_enabled:
        BDBG_MODULE_LOG(nexus_sage_module, ("SecureLog logging enabled, to start logging thread!"));
        break;
    case nexus_sage_secure_log_thread_started:
        BDBG_MODULE_LOG(nexus_sage_module, ("SecureLog log thread started, to create log file!"));
        break;
    case nexus_sage_secure_log_capture_start:
        BDBG_MODULE_LOG(nexus_sage_module, ("SecureLog captured started!, captured %d, saved %d failed %d",
                secure_log_captured,secure_log_saved,secure_log_capture_fail));
        NEXUS_Sage_SecureLog_P_PrintAttachedTAs();
        break;
    case nexus_sage_secure_log_terminated:
        BDBG_MODULE_LOG(nexus_sage_module, ("SecureLog log terminated!"));
        break;
    default:
        BDBG_MODULE_LOG(nexus_sage_module, ("SecureLog log unknow status %d!",secure_log_status));
        break;
    }
}
