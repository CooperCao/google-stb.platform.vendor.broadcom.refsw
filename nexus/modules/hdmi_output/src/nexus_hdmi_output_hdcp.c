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
#include "nexus_hdmi_output_module.h"
#include "priv/nexus_core.h"
#include "nexus_hdmi_types.h"


#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
#include "nexus_sage.h"
#include "bsagelib.h"
#include "bsagelib_client.h"
#include "priv/nexus_sage_priv.h" /* get access to NEXUS_Sage_GetSageLib_priv() */

#if NEXUS_HAS_HDMI_INPUT
#include "priv/nexus_hdmi_input_priv.h"
#include "nexus_hdmi_input_hdcp.h"
#endif

#endif

BDBG_MODULE(nexus_hdmi_output_hdcp);


#if NEXUS_HAS_SECURITY
#include "priv/nexus_security_priv.h"

static void NEXUS_HdmiOutput_P_RiCallback(void *pContext);
static void NEXUS_HdmiOutput_P_PjCallback(void *pContext);
static void NEXUS_HdmiOutput_P_HdcpTimerCallback(void *pContext);
static void NEXUS_HdmiOutput_P_HdcpKeepAliveTimerCallback(void *pContext);
static void NEXUS_HdmiOutput_P_UpdateHdcpState(NEXUS_HdmiOutputHandle handle);
static NEXUS_Error NEXUS_HdmiOutput_P_InitHdcp1x(NEXUS_HdmiOutputHandle output);
NEXUS_Error NEXUS_HdmiOutput_P_SetHdcpVersion(NEXUS_HdmiOutputHandle handle, NEXUS_HdmiOutputHdcpVersion version_select);

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
/* Hdcp 2.2 related Private APIs */

#if NEXUS_HAS_HDMI_INPUT
static void NEXUS_HdmiOutput_P_Hdcp2xUploadDownstreamInfo(NEXUS_HdmiOutputHandle handle);
#endif

static void NEXUS_HdmiOutput_P_Hdcp2xReAuthRequestCallback(void *pContext);
static void NEXUS_HdmiOutput_P_Hdcp2xAuthenticationStatusUpdate(void *pContext);
static void NEXUS_HdmiOutput_P_SageTATerminatedCallback_isr(void);
static void NEXUS_HdmiOutput_P_SageWatchdogEventhandler(void *pContext);
static void NEXUS_HdmiOutput_P_SageIndicationEventHandler(void *pContext);
static void NEXUS_HdmiOutput_P_SageIndicationCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument,
    uint32_t indication_id,
    uint32_t value
);

static void NEXUS_HdmiOutput_P_SageResponseCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument
);

#endif

#define LOCK_SECURITY() NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.security)
#define UNLOCK_SECURITY() NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.security)


#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
static NEXUS_Error NEXUS_HdmiOutput_P_InitHdcp2x(NEXUS_HdmiOutputHandle output)
{
    BHDCPlib_Dependencies hdcpDependencies;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    BKNI_EventHandle hdmEvent;
    BKNI_EventHandle hdcplibEvent;
    NEXUS_SageStatus sageStatus;

    BSAGElib_Handle sagelibHandle;
    BSAGElib_ClientSettings sagelibClientSettings;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

    /* get status so we block until Sage is running */
    rc = NEXUS_Sage_GetStatus(&sageStatus);
    if (rc) return BERR_TRACE(rc);

    /* retrieve Sagelib Handle */
    NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);
    NEXUS_Sage_GetSageLib_priv(&sagelibHandle);
    NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);

    /*****
         Create event to handle watchdog from SAGE
        *****/
    rc = BKNI_CreateEvent(&g_NEXUS_hdmiOutputSageData.eventWatchdogRecv);
    if (rc != BERR_SUCCESS) {
        BDBG_ERR(( "Error creating sage eventWatchdogRecv" ));
        rc = BERR_TRACE(rc);
        goto err_hdcp;
    }

    /* register event */
    g_NEXUS_hdmiOutputSageData.eventWatchdogRecvCallback =
        NEXUS_RegisterEvent(g_NEXUS_hdmiOutputSageData.eventWatchdogRecv,
            NEXUS_HdmiOutput_P_SageWatchdogEventhandler, output);
    if (g_NEXUS_hdmiOutputSageData.eventWatchdogRecvCallback == NULL)
    {
        BDBG_ERR(( "NEXUS_RegisterEvent(eventWatchdogRecv) failed!" ));
        errCode = BERR_TRACE(NEXUS_OS_ERROR);
        goto err_hdcp;
    }

    NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);
    NEXUS_Sage_AddWatchdogEvent_priv(g_NEXUS_hdmiOutputSageData.eventWatchdogRecv);
    NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);


    /* Initialize all SAGE related handles/Data if not yet initialized */
    if (g_NEXUS_hdmiOutputSageData.sagelibClientHandle == NULL)
    {
    /****
        * create event to call BSAGElib_Client_DispatchResponseCallbacks
                when responseRecv_isr callback fire */
        rc = BKNI_CreateEvent(&g_NEXUS_hdmiOutputSageData.eventResponseRecv);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(( "Error creating sage eventResponseRecv" ));
            rc = BERR_TRACE(rc);
            goto err_hdcp;
        }


        /******
        * create event for IndicationRecv_isr callback */
        rc = BKNI_CreateEvent(&g_NEXUS_hdmiOutputSageData.eventIndicationRecv);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(( "Error creating sage eventIndicationRecv" ));
            rc = BERR_TRACE(rc);
            goto err_hdcp;
        }

        /* register event */
        g_NEXUS_hdmiOutputSageData.eventIndicationRecvCallback=
            NEXUS_RegisterEvent(g_NEXUS_hdmiOutputSageData.eventIndicationRecv,
                NEXUS_HdmiOutput_P_SageIndicationEventHandler, &g_NEXUS_hdmiOutputSageData);

        if (g_NEXUS_hdmiOutputSageData.eventIndicationRecvCallback == NULL)
        {
            BDBG_ERR(( "NEXUS_RegisterEvent(eventIndicationRecv) failed!" ));
            errCode = BERR_TRACE(NEXUS_OS_ERROR);
            goto err_hdcp;
        }

        /*****
             Create event to handle TATerminate Int from SAGE
        *****/
        rc = BKNI_CreateEvent(&g_NEXUS_hdmiOutputSageData.eventTATerminated);
        if (rc != BERR_SUCCESS) {
            BDBG_ERR(( "Error creating sage eventTATerminated" ));
            rc = BERR_TRACE(rc);
            goto err_hdcp;
        }

        /* register event - piggy back to SAGE WatchdogEventHandler */
        g_NEXUS_hdmiOutputSageData.eventTATerminatedCallback =
            NEXUS_RegisterEvent(g_NEXUS_hdmiOutputSageData.eventTATerminated,
                NEXUS_HdmiOutput_P_SageWatchdogEventhandler, output);
        if (g_NEXUS_hdmiOutputSageData.eventTATerminatedCallback== NULL)
        {
            BDBG_ERR(( "NEXUS_RegisterEvent(eventTATerminated) failed!" ));
            errCode = BERR_TRACE(NEXUS_OS_ERROR);
            goto err_hdcp;
        }

        /* Open sagelib client */
        BSAGElib_GetDefaultClientSettings(sagelibHandle, &sagelibClientSettings);
        sagelibClientSettings.i_rpc.indicationRecv_isr = NEXUS_HdmiOutput_P_SageIndicationCallback_isr;
        sagelibClientSettings.i_rpc.responseRecv_isr = NEXUS_HdmiOutput_P_SageResponseCallback_isr;
        sagelibClientSettings.i_rpc.taTerminate_isr = (BSAGElib_Rpc_TATerminateCallback) NEXUS_HdmiOutput_P_SageTATerminatedCallback_isr;
        rc = BSAGElib_OpenClient(sagelibHandle, &g_NEXUS_hdmiOutputSageData.sagelibClientHandle,
                                    &sagelibClientSettings);
        if (rc != BERR_SUCCESS)
        {
            BERR_TRACE(rc);
            goto err_hdcp;
        }
    }


    /* Open HDCPlib */
    BHDCPlib_GetDefaultDependencies(&hdcpDependencies);
    hdcpDependencies.hHdm = output->hdmHandle;
    hdcpDependencies.eVersion = BHDM_HDCP_Version_e2_2;
    hdcpDependencies.hTmr = g_pCoreHandles->tmr;
    hdcpDependencies.pHdcpTA = g_hdcpTABlock.buf;
    hdcpDependencies.hdcpTASize = g_hdcpTABlock.len;
    hdcpDependencies.hSagelibClientHandle = g_NEXUS_hdmiOutputSageData.sagelibClientHandle;
    hdcpDependencies.sageResponseReceivedEvent = g_NEXUS_hdmiOutputSageData.eventResponseRecv;

    rc = BHDCPlib_Open(&output->hdcpHandle, &hdcpDependencies);
    BDBG_MSG(("BHDCPlib_Open (for HDCP2.2) <<< TX(%p)", (void *)output->hdcpHandle));
    if (rc != BERR_SUCCESS)
    {
        errCode = BERR_TRACE(rc);
        goto err_hdcp;
    }

    /* get HDCP2x_AuthenticationStatus Event Handle */
    rc = BHDCPlib_Hdcp2x_GetEventHandle(output->hdcpHandle, BHDCPLIB_Hdcp2x_EventIndication, &hdcplibEvent);
    if (rc != BERR_SUCCESS) {
        errCode = BERR_TRACE(rc);
        goto err_hdcp;
    }
    output->hdcp2xAuthenticationStatusCallback = NEXUS_RegisterEvent(hdcplibEvent,
                                NEXUS_HdmiOutput_P_Hdcp2xAuthenticationStatusUpdate, output);
    if (output->hdcp2xAuthenticationStatusCallback == NULL) {
        errCode = BERR_OS_ERROR;
        BERR_TRACE(errCode);
        goto err_hdcp;
    }

    /* get HDCP2x_ReAuthRequest Event Handle */
    errCode = BHDM_GetEventHandle(output->hdmHandle, BHDM_EventHDCP22ReAuthRequest, &hdmEvent);
    if (errCode != BERR_SUCCESS) {
        BERR_TRACE(errCode);
        goto err_hdcp;
    }
    output->hdcp2xReAuthRequestCallback = NEXUS_RegisterEvent(hdmEvent,
                                NEXUS_HdmiOutput_P_Hdcp2xReAuthRequestCallback, output);
    if (output->hdcp2xReAuthRequestCallback == NULL) {
        errCode = BERR_OS_ERROR;
        BERR_TRACE(errCode);
        goto err_hdcp;
    }

    /* Default content stream type to type1 */
    output->hdcpSettings.hdcp2xContentStreamControl = NEXUS_Hdcp2xContentStream_eType1;
    BKNI_EnterCriticalSection();
    output->pendingDisableAuthentication_isr = false;
    BKNI_LeaveCriticalSection();

    return BERR_SUCCESS;


err_hdcp:
    if (g_NEXUS_hdmiOutputSageData.eventWatchdogRecvCallback) {
        NEXUS_UnregisterEvent(g_NEXUS_hdmiOutputSageData.eventWatchdogRecvCallback);
        g_NEXUS_hdmiOutputSageData.eventWatchdogRecvCallback = NULL;
    }

    if (g_NEXUS_hdmiOutputSageData.eventWatchdogRecv) {
        NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);
        NEXUS_Sage_RemoveWatchdogEvent_priv(g_NEXUS_hdmiOutputSageData.eventWatchdogRecv);
        NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);
        BKNI_DestroyEvent(g_NEXUS_hdmiOutputSageData.eventWatchdogRecv);
        g_NEXUS_hdmiOutputSageData.eventWatchdogRecv = NULL;
    }

    if (g_NEXUS_hdmiOutputSageData.eventTATerminatedCallback) {
        NEXUS_UnregisterEvent(g_NEXUS_hdmiOutputSageData.eventTATerminatedCallback);
        g_NEXUS_hdmiOutputSageData.eventTATerminatedCallback = NULL;
    }

    if (g_NEXUS_hdmiOutputSageData.eventTATerminated) {
        BKNI_DestroyEvent(g_NEXUS_hdmiOutputSageData.eventTATerminated);
        g_NEXUS_hdmiOutputSageData.eventTATerminated= NULL;
    }

    if (g_NEXUS_hdmiOutputSageData.eventResponseRecv) {
        BKNI_DestroyEvent(g_NEXUS_hdmiOutputSageData.eventResponseRecv);
        g_NEXUS_hdmiOutputSageData.eventResponseRecv = NULL;
    }

    if (g_NEXUS_hdmiOutputSageData.eventIndicationRecvCallback) {
        NEXUS_UnregisterEvent(g_NEXUS_hdmiOutputSageData.eventIndicationRecvCallback);
        g_NEXUS_hdmiOutputSageData.eventIndicationRecvCallback = NULL;
    }

    if (g_NEXUS_hdmiOutputSageData.eventIndicationRecv) {
        BKNI_DestroyEvent(g_NEXUS_hdmiOutputSageData.eventIndicationRecv);
        g_NEXUS_hdmiOutputSageData.eventIndicationRecv = NULL;
    }

    if (g_NEXUS_hdmiOutputSageData.sagelibClientHandle) {
        BSAGElib_CloseClient(g_NEXUS_hdmiOutputSageData.sagelibClientHandle);
        g_NEXUS_hdmiOutputSageData.sagelibClientHandle = NULL;
    }

    if (output->hdcp2xAuthenticationStatusCallback != NULL) {
        NEXUS_UnregisterEvent(output->hdcp2xAuthenticationStatusCallback);
        output->hdcp2xAuthenticationStatusCallback = NULL;
    }

    if (output->hdcp2xReAuthRequestCallback != NULL) {
        NEXUS_UnregisterEvent(output->hdcp2xReAuthRequestCallback);
        output->hdcp2xReAuthRequestCallback = NULL;
    }

    return BERR_NOT_SUPPORTED;


}
#endif /* NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT) */

static void NEXUS_HdmiOutput_P_LockSecurity(void)
{
    LOCK_SECURITY();
}

static void NEXUS_HdmiOutput_P_UnlockSecurity(void)
{
    UNLOCK_SECURITY();
}

static NEXUS_Error NEXUS_HdmiOutput_P_InitHdcp1x(NEXUS_HdmiOutputHandle output)
{
    BHDCPlib_Dependencies hdcpDependencies;
    BHDCPlib_Configuration *pHdcpConfig = NULL;
    BKNI_EventHandle hdmEvent;
    NEXUS_Error errCode;
    int i;

    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

    pHdcpConfig = BKNI_Malloc(sizeof(BHDCPlib_Configuration));
    if ( NULL == pHdcpConfig )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Open HDCPlib */
    BHDCPlib_GetDefaultConfiguration(pHdcpConfig);
    BHDCPlib_GetDefaultDependencies(&hdcpDependencies);
    hdcpDependencies.hHdm = output->hdmHandle;
    LOCK_SECURITY();
    NEXUS_Security_GetHsm_priv(&hdcpDependencies.hHsm);
    UNLOCK_SECURITY();
    hdcpDependencies.hTmr = g_pCoreHandles->tmr;
    hdcpDependencies.lockHsm = NEXUS_HdmiOutput_P_LockSecurity;
    hdcpDependencies.unlockHsm = NEXUS_HdmiOutput_P_UnlockSecurity;

    errCode = BHDCPlib_Open(&output->hdcpHandle, &hdcpDependencies);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_hdcp;
    }

    /* Set default HDCP settings (rest are initialized to zero) */
    output->hdcpSettings.anValue = NEXUS_HdmiOutputHdcpAnValue_eRandom;
    output->hdcpSettings.waitForValidVideo = pHdcpConfig->msWaitForValidVideo;
    output->hdcpSettings.waitForRxR0Margin = pHdcpConfig->msWaitForRxR0Margin;
    output->hdcpSettings.waitForKsvFifoMargin = pHdcpConfig->msWaitForKsvFifoMargin;
    output->hdcpSettings.maxDeviceCountSupported = pHdcpConfig->uiMaxDeviceCount;
    output->hdcpSettings.maxDepthSupported = pHdcpConfig->uiMaxDepth;

    /* Retrieve HDCP keys and set HDCP configuration */
    errCode = BHDCPlib_GetKeySet(pHdcpConfig->TxKeySet.TxAksv, pHdcpConfig->TxKeySet.TxKeyStructure);
    if (errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Error retrieving HDCP key set"));
        errCode = BERR_TRACE(errCode);
        goto err_hdcp;
    }

    BKNI_Memcpy(output->hdcpSettings.aksv.data, pHdcpConfig->TxKeySet.TxAksv, BAVC_HDMI_HDCP_KSV_LENGTH);
    for ( i = 0; i < BAVC_HDMI_HDCP_N_PRIVATE_KEYS; i++ )
    {
        BKNI_Memcpy(&output->hdcpSettings.encryptedKeySet[i], &pHdcpConfig->TxKeySet.TxKeyStructure[i], sizeof(NEXUS_HdmiOutputHdcpKey));
    }

    /* get HDCP Ri Event Handle */
    errCode = BHDM_GetEventHandle(output->hdmHandle, BHDM_EventHDCPRiValue, &hdmEvent);
    if ( errCode ) {
        errCode = BERR_TRACE(errCode);
        goto err_ri;
    }
    output->riCallback = NEXUS_RegisterEvent(hdmEvent, NEXUS_HdmiOutput_P_RiCallback, output);
    if ( NULL == output->riCallback ) {
        errCode = BERR_OS_ERROR;
        BERR_TRACE(errCode);
        goto err_ri;
    }

    /* get HDCP Pj Event Handle */
    errCode = BHDM_GetEventHandle(output->hdmHandle, BHDM_EventHDCPPjValue, &hdmEvent);
    if ( errCode ) {
        errCode = BERR_TRACE(errCode);
        goto err_pj;
    }
    output->pjCallback = NEXUS_RegisterEvent(hdmEvent, NEXUS_HdmiOutput_P_PjCallback, output);
    if ( NULL == output->pjCallback ) {
        errCode = BERR_OS_ERROR;
        BERR_TRACE(errCode);
        goto err_pj;
    }

    BKNI_Free(pHdcpConfig);
    return BERR_SUCCESS;

err_pj:
    NEXUS_UnregisterEvent(output->riCallback);
err_ri:
    BHDCPlib_Close(output->hdcpHandle);
    output->hdcpHandle = NULL;

err_hdcp:
    if (pHdcpConfig) {
        BKNI_Free(pHdcpConfig);
    }

    return BERR_NOT_SUPPORTED;

}


NEXUS_Error NEXUS_HdmiOutput_P_InitHdcp(NEXUS_HdmiOutputHandle output)
{
    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

    output->hdcpSettings.transmitEncrypted = true;
    NEXUS_CallbackDesc_Init(&output->hdcpSettings.stateChangedCallback);
    NEXUS_CallbackDesc_Init(&output->hdcpSettings.successCallback);
    NEXUS_CallbackDesc_Init(&output->hdcpSettings.failureCallback);

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    if (output->eHdcpVersion == BHDM_HDCP_Version_e2_2 &&
        output->hdcpVersionSelect != NEXUS_HdmiOutputHdcpVersion_e1_x) {
        return NEXUS_HdmiOutput_P_InitHdcp2x(output);
    }
    else
#endif
    {
        return NEXUS_HdmiOutput_P_InitHdcp1x(output);
    }
}

void NEXUS_HdmiOutput_P_UninitHdcp(NEXUS_HdmiOutputHandle output)
{
    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

    if ( output->hdcpTimer )
    {
        NEXUS_CancelTimer(output->hdcpTimer);
        output->hdcpTimer = NULL;
    }
    if ( output->hdcpKeepAliveTimer )
    {
        NEXUS_CancelTimer(output->hdcpKeepAliveTimer);
        output->hdcpKeepAliveTimer = NULL;
    }

    if (output->eHdcpVersion == BHDM_HDCP_Version_e2_2) {
        (void)NEXUS_HdmiOutput_DisableHdcpEncryption(output);

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
        if (g_NEXUS_hdmiOutputSageData.eventWatchdogRecvCallback) {
            NEXUS_UnregisterEvent(g_NEXUS_hdmiOutputSageData.eventWatchdogRecvCallback);
            g_NEXUS_hdmiOutputSageData.eventWatchdogRecvCallback = NULL;
        }

        if (g_NEXUS_hdmiOutputSageData.eventWatchdogRecv) {
            NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);
            NEXUS_Sage_RemoveWatchdogEvent_priv(g_NEXUS_hdmiOutputSageData.eventWatchdogRecv);
            NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.sage);
            BKNI_DestroyEvent(g_NEXUS_hdmiOutputSageData.eventWatchdogRecv);
            g_NEXUS_hdmiOutputSageData.eventWatchdogRecv = NULL;
        }

        if (g_NEXUS_hdmiOutputSageData.eventTATerminatedCallback) {
            NEXUS_UnregisterEvent(g_NEXUS_hdmiOutputSageData.eventTATerminatedCallback);
            g_NEXUS_hdmiOutputSageData.eventTATerminatedCallback = NULL;
        }

        if (g_NEXUS_hdmiOutputSageData.eventTATerminated) {
            BKNI_DestroyEvent(g_NEXUS_hdmiOutputSageData.eventTATerminated);
            g_NEXUS_hdmiOutputSageData.eventTATerminated= NULL;
        }

        if (output->hdcp2xAuthenticationStatusCallback != NULL) {
            NEXUS_UnregisterEvent(output->hdcp2xAuthenticationStatusCallback);
            output->hdcp2xAuthenticationStatusCallback = NULL;
        }

        if (output->hdcp2xReAuthRequestCallback != NULL) {
            NEXUS_UnregisterEvent(output->hdcp2xReAuthRequestCallback);
            output->hdcp2xReAuthRequestCallback = NULL;
        }
#endif
    }
    else {
       (void)NEXUS_HdmiOutput_DisableHdcpAuthentication(output);
    }

    if (output->pjCallback != NULL) {
        NEXUS_UnregisterEvent(output->pjCallback);
        output->pjCallback = NULL;
    }

    if (output->riCallback != NULL) {
        NEXUS_UnregisterEvent(output->riCallback);
        output->riCallback = NULL;
    }

    BHDCPlib_Close(output->hdcpHandle);
    output->hdcpHandle = NULL;

    /* Close sagelibClientHandle */
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)

    if (g_NEXUS_hdmiOutputSageData.eventResponseRecv) {
        BKNI_DestroyEvent(g_NEXUS_hdmiOutputSageData.eventResponseRecv);
        g_NEXUS_hdmiOutputSageData.eventResponseRecv = NULL;
    }

    if (g_NEXUS_hdmiOutputSageData.eventIndicationRecvCallback) {
        NEXUS_UnregisterEvent(g_NEXUS_hdmiOutputSageData.eventIndicationRecvCallback);
        g_NEXUS_hdmiOutputSageData.eventIndicationRecvCallback = NULL;
    }

    if (g_NEXUS_hdmiOutputSageData.eventIndicationRecv) {
        BKNI_DestroyEvent(g_NEXUS_hdmiOutputSageData.eventIndicationRecv);
        g_NEXUS_hdmiOutputSageData.eventIndicationRecv = NULL;
    }

    if (g_NEXUS_hdmiOutputSageData.sagelibClientHandle) {
        BSAGElib_CloseClient(g_NEXUS_hdmiOutputSageData.sagelibClientHandle);
        g_NEXUS_hdmiOutputSageData.sagelibClientHandle = NULL;
    }

    BKNI_Memset(&g_NEXUS_hdmiOutputSageData, 0, sizeof(g_NEXUS_hdmiOutputSageData));
#endif
}

void NEXUS_HdmiOutput_P_CloseHdcp(NEXUS_HdmiOutputHandle output)
{
    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

#if NEXUS_HAS_SECURITY
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    if (output->hdcp2xKeys.buffer != NULL) {
        BKNI_Free(output->hdcp2xKeys.buffer);
        output->hdcp2xKeys.buffer = NULL;
        output->hdcp2xKeys.bufferSize = 0;
    }
#endif
    if (output->pRevokedKsvs != NULL) {
        BKNI_Free(output->pRevokedKsvs);
        output->pRevokedKsvs = NULL;
        output->revokedKsvsSize = 0;
   }
#endif
}

void NEXUS_HdmiOutput_P_HdcpNotifyHotplug(NEXUS_HdmiOutputHandle output)
{
    BHDCPlib_Event event = {BHDM_EventHotPlug};

    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

    /* Any hotplug event should stop the state machine */
    if ( output->hdcpTimer )
    {
        NEXUS_CancelTimer(output->hdcpTimer);
        output->hdcpTimer = NULL;
    }

    BHDCPlib_ProcessEvent(output->hdcpHandle, &event);

    if ((output->eHdcpVersion == BHDM_HDCP_Version_e1_1)
    || (output->eHdcpVersion == BHDM_HDCP_Version_e1_1_Optional_Features))
    {
       NEXUS_HdmiOutput_P_UpdateHdcpState(output);
    }
    else
    {
        BKNI_EnterCriticalSection();
            output->pendingDisableAuthentication_isr = false;
        BKNI_LeaveCriticalSection();
    }
}

static void NEXUS_HdmiOutput_P_RiCallback(void *pContext)
{
    NEXUS_HdmiOutputHandle output = (NEXUS_HdmiOutputHandle)pContext;
    BHDCPlib_Event event = {BHDM_EventHDCPRiValue};

    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

    BHDCPlib_ProcessEvent(output->hdcpHandle, &event);

    NEXUS_HdmiOutput_P_UpdateHdcpState(output);
}

static void NEXUS_HdmiOutput_P_PjCallback(void *pContext)
{
    NEXUS_HdmiOutputHandle output = (NEXUS_HdmiOutputHandle)pContext;
    BHDCPlib_Event event = {BHDM_EventHDCPPjValue};

    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

    BHDCPlib_ProcessEvent(output->hdcpHandle, &event);

    NEXUS_HdmiOutput_P_UpdateHdcpState(output);
}


#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
#if NEXUS_HAS_HDMI_INPUT
static void NEXUS_HdmiOutput_P_Hdcp2xUploadDownstreamInfo(NEXUS_HdmiOutputHandle handle)
{
    BERR_Code rc = BERR_SUCCESS;
    NEXUS_Hdcp2xReceiverIdListData stReceiverIdListData;
    BHDCPlib_ReceiverIdListData hdcp2xReceiverIdListData;


    BDBG_MSG(("%s: Upload ReceiverId List to upstream transmitter", BSTD_FUNCTION));

    /* First, get downstream info from hdcplib */
    rc = BHDCPlib_Hdcp2x_Tx_GetReceiverIdList(handle->hdcpHandle, &hdcp2xReceiverIdListData);
    if (rc != BERR_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto done;
    }

    /* save all data */
    stReceiverIdListData.deviceCount = (unsigned) hdcp2xReceiverIdListData.deviceCount;
    stReceiverIdListData.depth = (unsigned) hdcp2xReceiverIdListData.depth;
    stReceiverIdListData.maxDevsExceeded = hdcp2xReceiverIdListData.maxDevsExceeded > 0;
    stReceiverIdListData.maxCascadeExceeded = hdcp2xReceiverIdListData.maxCascadeExceeded > 0;
    stReceiverIdListData.hdcp2xLegacyDeviceDownstream = hdcp2xReceiverIdListData.hdcp2LegacyDeviceDownstream > 0;
    stReceiverIdListData.hdcp1DeviceDownstream = hdcp2xReceiverIdListData.hdcp1DeviceDownstream > 0;

    if (hdcp2xReceiverIdListData.deviceCount + hdcp2xReceiverIdListData.downstreamIsRepeater >= BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT)
    {
        /* ignore the rest of the list, if any */
        BKNI_Memcpy(stReceiverIdListData.rxIdList, &hdcp2xReceiverIdListData.rxIdList,
            BHDCPLIB_HDCP2X_MAX_DEVICE_COUNT * BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);
    }
    else
    {
        /*****************
        * If downstream device is repeater device, the ReceiverIdList will
        * contain 1 additional entry, which is the ReceiverId of the repeater device.
        * This additional entry is not accounted in the device count
        *******************/
        BKNI_Memcpy(stReceiverIdListData.rxIdList, &hdcp2xReceiverIdListData.rxIdList,
            (hdcp2xReceiverIdListData.deviceCount + hdcp2xReceiverIdListData.downstreamIsRepeater)*BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);
    }

    /* Now upload it to the upstream transmitter */
    NEXUS_Module_Lock(g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput);
    rc = NEXUS_HdmiInput_LoadHdcp2xReceiverIdList_priv(handle->hdmiInput, &stReceiverIdListData, hdcp2xReceiverIdListData.downstreamIsRepeater > 0);
    NEXUS_Module_Unlock(g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput);
    if (rc != NEXUS_SUCCESS)
    {
        rc = BERR_TRACE(rc);
        goto done;
    }

done:

    return;
}
#endif


/*
** the following callback is made,
** when the HDCP 2.x Rx requests HDCP re-authentication through the monitored RxStatus bits
*/
static void NEXUS_HdmiOutput_P_Hdcp2xReAuthRequestCallback(void *pContext)
{
    NEXUS_HdmiOutputHandle output = pContext;
    BERR_Code rc = BERR_SUCCESS;
    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

    BDBG_MSG(("%s: Received ReAuth Request from downstream HDCP2.x receiver", BSTD_FUNCTION));

    if (output->hdcpStarted == true)
    {
        output->hdcpMonitor.hdcp22.validReauthReqCounter++ ;
        /* Restart HDCP 2.x authentication */
        rc = NEXUS_HdmiOutput_StartHdcpAuthentication(output);
        if (rc != BERR_SUCCESS)
        {
            BDBG_ERR(("Error restarting HDCP 2.x authentication process"));
            BERR_TRACE(rc);
        }
    }
    else
    {
        output->hdcpMonitor.hdcp22.invalidReauthReqCounter++ ;
        BDBG_WRN(("Received unexpected Rx ReAuth Request...ignored"));
    }

    return;
}


static void NEXUS_HdmiOutput_P_Hdcp2xAuthenticationStatusUpdate(void *pContext)
{
    NEXUS_HdmiOutputHandle output = pContext;
    BERR_Code rc = BERR_SUCCESS;
    BHDCPlib_Hdcp2x_AuthenticationStatus stAuthenticationStatus;
    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

    rc = BHDCPlib_Hdcp2x_GetAuthenticationStatus(output->hdcpHandle, &stAuthenticationStatus);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("Error retrieving HDCP2.x authentication status"));
        rc = BERR_TRACE(rc);
    }

    BDBG_MSG(("%s: HDCP 2.x authentication status - state: %d, error: %d", BSTD_FUNCTION,
		stAuthenticationStatus.eHdcpState, stAuthenticationStatus.eAuthenticationError));

    /**********
    ** Upload ReceiverID List to upstream transmitter IF
    **    + AUTHENTICATED with downstream devices
    **  OR
    **    + FAILED to authenticate with downstream devices due to errors in downstream ReceiverId List
    **********/
    if ((stAuthenticationStatus.linkAuthenticated)
    || (stAuthenticationStatus.eAuthenticationError == BHDCPlib_HdcpError_eRepeaterDepthExceeded)
    || (stAuthenticationStatus.eAuthenticationError == BHDCPlib_HdcpError_eRxDevicesExceeded)
    || (stAuthenticationStatus.eAuthenticationError == BHDCPlib_HdcpError_eRepeaterAuthenticationError))
    {
#if NEXUS_HAS_HDMI_INPUT
        /* get/upload downstream device list if in repeater mode */
        if (output->hdmiInput)
        {
            NEXUS_HdmiInputHdcpStatus hdmiInputHdcpStatus;
            rc = NEXUS_HdmiInput_HdcpGetStatus(output->hdmiInput, &hdmiInputHdcpStatus) ;
            if (rc) {
                BERR_TRACE(rc);
                return;
            }

            if((hdmiInputHdcpStatus.version == NEXUS_HdcpVersion_e2x)
            && (hdmiInputHdcpStatus.hdcpState != NEXUS_HdmiInputHdcpState_eAuthenticated)
            && (hdmiInputHdcpStatus.hdcpState != NEXUS_HdmiInputHdcpState_eRepeaterAuthenticated))
            {
                output->forceSendRxIdList = true;
            }
            else {
                NEXUS_HdmiOutput_P_Hdcp2xUploadDownstreamInfo(output);
            }
        }
#endif
    }
    else {
        BKNI_Sleep(70);

        /* update HDCP Auth faiure counter if callback is due to an HDCP Auth error */
        if (stAuthenticationStatus.eAuthenticationError != BHDCPlib_HdcpError_eSuccess)
        {
            output->hdcpMonitor.hdcp22.auth.failCounter++ ;
        }
    }

    /* fire stateChange call back */
    NEXUS_TaskCallback_Fire(output->hdcpStateChangedCallback);

    return;
}


/* The ISR callback is registered in HSI and will be fire uppon TA terminated interrupt */
static void NEXUS_HdmiOutput_P_SageTATerminatedCallback_isr(void)
{
    BDBG_WRN(("%s: SAGE TATerminate interrupt", BSTD_FUNCTION));

    BKNI_SetEvent_isr(g_NEXUS_hdmiOutputSageData.eventTATerminated);
}


/* This event handler is called whenever watchdogEvent registered event is set.
 * The watchdogEvent is set inside NEXUS_HdmiOutput_P_SageWatchdogIntHandler_isr() */
static void NEXUS_HdmiOutput_P_SageWatchdogEventhandler(void *pContext)
{
    NEXUS_HdmiOutputHandle output = pContext;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ERR(("%s: SAGE Hdcp2.x Recovery Process - Reopen/initialize HDCPlib and sage rpc handles", BSTD_FUNCTION));

    /* Disable encryption and reset system state */
    (void)NEXUS_HdmiOutput_DisableHdcpEncryption(output);

    output->hdcpMonitor.hdcp22.watchdogCounter++ ;

    /* Reinitialized SAGE RPC handles (now invalid) */
    rc = BHDCPlib_Hdcp2x_ProcessWatchDog(output->hdcpHandle);
    if (rc != BERR_SUCCESS)
    {
        BDBG_ERR(("%s: Error process recovery attempt in HDCPlib", BSTD_FUNCTION));
        rc = BERR_TRACE(rc);
        goto done;

    }

    if (output->hdcpStarted) {
        errCode = NEXUS_HdmiOutput_StartHdcpAuthentication(output);
        if (errCode != NEXUS_SUCCESS)
        {
            BDBG_ERR(("%s: Error restarting HDCP authentication after SAGE Hdcp2.x recovery process", BSTD_FUNCTION));
            errCode = BERR_TRACE(errCode);
            goto done;
        }
    }

done:

    return;
}


static void NEXUS_HdmiOutput_P_SageIndicationEventHandler(void *pContext)
{
    NEXUS_HdmiOutput_SageData *sageData = (NEXUS_HdmiOutput_SageData *) pContext;
    NEXUS_HdmiOutputIndicationData receivedIndication;

    BERR_Code rc = BERR_SUCCESS;
    unsigned i=0;

    while (g_NEXUS_hdmiOutputSageData.indicationReadPtr != g_NEXUS_hdmiOutputSageData.indicationWritePtr)
    {
        i = g_NEXUS_hdmiOutputSageData.indicationReadPtr;
        receivedIndication = sageData->indicationData[i];

        BKNI_EnterCriticalSection();
            if (++g_NEXUS_hdmiOutputSageData.indicationReadPtr == NEXUS_HDMI_OUTPUT_SAGE_INDICATION_QUEUE_SIZE)
            {
                g_NEXUS_hdmiOutputSageData.indicationReadPtr = 0;
            }
        BKNI_LeaveCriticalSection();

        /* Now pass the callback information to HDCPlib */
        rc = BHDCPlib_Hdcp2x_ReceiveSageIndication(
                        receivedIndication.hHDCPlib, &receivedIndication.sageIndication);
        if (rc) BERR_TRACE(rc);
    }

    return;
}


static void NEXUS_HdmiOutput_P_SageIndicationCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument,
    uint32_t indication_id,
    uint32_t value
)
{
    /* Save information for later use */
    unsigned i = g_NEXUS_hdmiOutputSageData.indicationWritePtr;


    g_NEXUS_hdmiOutputSageData.indicationData[i].sageIndication.rpcRemoteHandle = sageRpcHandle;
    g_NEXUS_hdmiOutputSageData.indicationData[i].sageIndication.sessionId = indication_id;
    g_NEXUS_hdmiOutputSageData.indicationData[i].sageIndication.indication_id = value >> 16;
    g_NEXUS_hdmiOutputSageData.indicationData[i].sageIndication.value = value & 0x0000FFFF;
    g_NEXUS_hdmiOutputSageData.indicationData[i].hHDCPlib =
                                    (BHDCPlib_Handle) async_argument;

    if (++g_NEXUS_hdmiOutputSageData.indicationWritePtr == NEXUS_HDMI_OUTPUT_SAGE_INDICATION_QUEUE_SIZE)
    {
        g_NEXUS_hdmiOutputSageData.indicationWritePtr = 0;
    }

    if (g_NEXUS_hdmiOutputSageData.indicationWritePtr == g_NEXUS_hdmiOutputSageData.indicationReadPtr)
    {
        BDBG_ERR(("Indication queue overflow - increase queue size"));
    }

    BKNI_SetEvent_isr(g_NEXUS_hdmiOutputSageData.eventIndicationRecv);
    return;
}

static void NEXUS_HdmiOutput_P_SageResponseCallback_isr(
    BSAGElib_RpcRemoteHandle sageRpcHandle,
    void *async_argument
)
{

    BSTD_UNUSED(async_argument);
    BSTD_UNUSED(sageRpcHandle);

    BKNI_SetEvent_isr(g_NEXUS_hdmiOutputSageData.eventResponseRecv);
    return;
}

NEXUS_Error NEXUS_HdmiOutput_SetHdcp2xBinKeys(
    NEXUS_HdmiOutputHandle handle,
    const uint8_t *pBinFileBuffer,  /* attr{nelem=length} pointer to encrypted key buffer */
    uint32_t length                 /* size of data in pBinFileBuffer in bytes */
)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    if (IS_ALIAS(handle)) {
        errCode = NEXUS_NOT_SUPPORTED ;
        goto done ;
    }

    if (length == 0) {
        BDBG_ERR(("Invalid size provided")) ;
        errCode = NEXUS_INVALID_PARAMETER ;
        goto done ;
    }

    if (length > handle->hdcp2xKeys.bufferSize) {
        if (handle->hdcp2xKeys.buffer != NULL) {
            BKNI_Free(handle->hdcp2xKeys.buffer);
            handle->hdcp2xKeys.buffer = NULL;
            handle->hdcp2xKeys.bufferSize = 0;
        }
    }
    if (handle->hdcp2xKeys.buffer == NULL) {
        handle->hdcp2xKeys.buffer = BKNI_Malloc(length);
        if (handle->hdcp2xKeys.buffer == NULL) {
            errCode = BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY) ;
            goto done;
        }
        handle->hdcp2xKeys.bufferSize = length;
    }
    BKNI_Memset(handle->hdcp2xKeys.buffer, 0, handle->hdcp2xKeys.bufferSize);
    BKNI_Memcpy(handle->hdcp2xKeys.buffer, pBinFileBuffer, length);
    handle->hdcp2xKeys.length = length;

    if (g_NEXUS_hdmiOutputSageData.sagelibClientHandle) {
        errCode = BHDCPlib_Hdcp2x_SetBinKeys(handle->hdcpHandle, handle->hdcp2xKeys.buffer, handle->hdcp2xKeys.length);
    }
    else {
        /* if HDCP 2.x not started, keys must be loaded later anyway */
        BDBG_MSG(("Cannot call BHDCPlib_Hdcp2x_SetBinKeys, g_NEXUS_hdmiOutputSageData.sagelibClientHandle == NULL"));
    }

done :
    if ( errCode ) {
        errCode = BERR_TRACE(errCode);
    }

    return errCode;
}


NEXUS_Error NEXUS_HdmiOutput_SetRepeaterInput(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiInputHandle input
)
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    BHDCPlib_Hdcp2x_AuthenticationStatus stAuthenticationStatus;
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);


#if NEXUS_HAS_HDMI_INPUT
    if (input)
    {
        if (!g_NEXUS_hdmiOutputModuleSettings.modules.hdmiInput) {
            errCode = NEXUS_NOT_SUPPORTED;
            BERR_TRACE(errCode);
            goto done;
        }

        if (handle->hdmiInput && handle->hdmiInput != input) {
            NEXUS_OBJECT_RELEASE(handle, NEXUS_HdmiInput, handle->hdmiInput);
            handle->hdmiInput = NULL;
        }

        if (!handle->hdmiInput) {
            NEXUS_OBJECT_ACQUIRE(handle, NEXUS_HdmiInput, input);
        }
    }
    else
    {
        if (!handle->hdmiInput) {
            errCode = NEXUS_SUCCESS;
            goto done;
        }
        NEXUS_OBJECT_RELEASE(handle, NEXUS_HdmiInput, handle->hdmiInput);
    }

    handle->hdmiInput = input;

    if (handle->eHdcpVersion == BHDM_HDCP_Version_e2_2)
    {

        /**********
        ** For backward compatibility for URSR 16.2 - 16.4. API behavior:
        **     + set rx <--> tx link
        **     + do the work to upload the ReceiverId List
        **          -this action is only for pre 17.1 URSR.
        **          -starting from URSR 17.1, the upload take place when the authentication status get updated.
        **
        **********/
        errCode = BHDCPlib_Hdcp2x_GetAuthenticationStatus(handle->hdcpHandle, &stAuthenticationStatus);
        if (errCode != BERR_SUCCESS)
        {
            BDBG_ERR(("Error retrieving HDCP2.x authentication status"));
            errCode = BERR_TRACE(errCode);
            goto done;
        }

        if ((stAuthenticationStatus.linkAuthenticated)
        || (stAuthenticationStatus.eAuthenticationError == BHDCPlib_HdcpError_eRepeaterDepthExceeded)
        || (stAuthenticationStatus.eAuthenticationError == BHDCPlib_HdcpError_eRxDevicesExceeded)
        || (stAuthenticationStatus.eAuthenticationError == BHDCPlib_HdcpError_eRepeaterAuthenticationError)
        || handle->forceSendRxIdList)
        {
            if (handle->hdmiInput) {
                NEXUS_HdmiOutput_P_Hdcp2xUploadDownstreamInfo(handle);
                handle->forceSendRxIdList = false;
            }
        }
    }
    else {
        NEXUS_HdmiHdcpDownStreamInfo downStream  ;
        NEXUS_HdmiHdcpKsv *pKsvs ;
        unsigned returnedDevices;

        /* HDCP 1.x */
        NEXUS_HdmiOutput_HdcpGetDownstreamInfo(handle, &downStream) ;

        /* allocate space to hold ksvs for the downstream devices */
        pKsvs = BKNI_Malloc((downStream.devices) * NEXUS_HDMI_HDCP_KSV_LENGTH) ;
        if (!pKsvs)
        {
            BDBG_ERR(("Unable to allocate memory for downstream KSVs."));
            errCode = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
            goto done;
        }

        errCode = NEXUS_HdmiOutput_HdcpGetDownstreamKsvs(handle, pKsvs, downStream.devices, &returnedDevices) ;
        if (errCode) {
            errCode = BERR_TRACE(errCode);
            BKNI_Free(pKsvs) ;
            goto done;
        }

        /* Now upload it to the upstream transmitter */
        if (handle->hdmiInput)
        {
            errCode = NEXUS_HdmiInput_HdcpLoadKsvFifo(handle->hdmiInput, &downStream, pKsvs, downStream.devices) ;
            if (errCode != NEXUS_SUCCESS)
            {
                errCode = BERR_TRACE(errCode);
                BKNI_Free(pKsvs) ;
                goto done;
            }
        }
        BKNI_Free(pKsvs) ;
    }

#else
    BSTD_UNUSED(input);
    BSTD_UNUSED(stAuthenticationStatus);
    errCode = NEXUS_NOT_SUPPORTED;
    BERR_TRACE(errCode);
    goto done;
#endif

done:
    return errCode;
}


NEXUS_Error NEXUS_HdmiOutput_GetHdcp2xReceiverIdListData(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_Hdcp2xReceiverIdListData *pReceiverIdListData
)
{
    BERR_Code errCode = NEXUS_SUCCESS ;
    BHDCPlib_ReceiverIdListData hdcp2xReceiverIdListData;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    RESOLVE_ALIAS(handle);

    errCode = BHDCPlib_Hdcp2x_Tx_GetReceiverIdList(handle->hdcpHandle, &hdcp2xReceiverIdListData);
    if (errCode != BERR_SUCCESS)
    {
        errCode = BERR_TRACE(errCode);
        goto done;
    }

    pReceiverIdListData->deviceCount = (unsigned) hdcp2xReceiverIdListData.deviceCount;
    pReceiverIdListData->depth = (unsigned) hdcp2xReceiverIdListData.depth;
    pReceiverIdListData->maxDevsExceeded = hdcp2xReceiverIdListData.maxDevsExceeded > 0;
    pReceiverIdListData->maxCascadeExceeded = hdcp2xReceiverIdListData.maxCascadeExceeded > 0;
    pReceiverIdListData->hdcp2xLegacyDeviceDownstream = hdcp2xReceiverIdListData.hdcp2LegacyDeviceDownstream > 0;
    pReceiverIdListData->hdcp1DeviceDownstream = hdcp2xReceiverIdListData.hdcp1DeviceDownstream > 0;

    BKNI_Memcpy(pReceiverIdListData->rxIdList, &hdcp2xReceiverIdListData.rxIdList,
                hdcp2xReceiverIdListData.deviceCount*BHDCPLIB_HDCP2X_RECEIVERID_LENGTH);

done:
    return errCode;
}
#endif  /* NEXUS_HAS_SAGE */

/**
Summary:
Get HDCP Settings
**/
void NEXUS_HdmiOutput_GetHdcpSettings(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputHdcpSettings *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    BDBG_ASSERT(NULL != pSettings);

    RESOLVE_ALIAS(handle);
    *pSettings = handle->hdcpSettings;
}

/**
Summary:
Set HDCP Settings
**/
NEXUS_Error NEXUS_HdmiOutput_SetHdcpSettings(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiOutputHdcpSettings *pSettings
    )
{
    BHDCPlib_Configuration *pHdcpConfig;
    BHDM_HDCP_OPTIONS hdcpOptions;
    NEXUS_HdmiOutputHdcpVersion hdcpVersionSelect = handle->hdcpVersionSelect;
    NEXUS_Error errCode;
    int i;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    if (IS_ALIAS(handle)) return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    BDBG_ASSERT(NULL != pSettings);

    if (handle->hdcpHandle == NULL)
        return BERR_TRACE(NEXUS_NOT_INITIALIZED);

    /* check if HDCP level has changed and reinit if needed */
    NEXUS_HdmiOutput_P_SetHdcpVersion(handle, pSettings->hdcp_version);

    /* Install callback */
    NEXUS_TaskCallback_Set(handle->hdcpStateChangedCallback, &pSettings->stateChangedCallback);
    NEXUS_TaskCallback_Set(handle->hdcpSuccessCallback, &pSettings->successCallback);
    NEXUS_TaskCallback_Set(handle->hdcpFailureCallback, &pSettings->failureCallback);

    pHdcpConfig = BKNI_Malloc(sizeof(*pHdcpConfig));
    if ( NULL == pHdcpConfig )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BDBG_CASSERT(BAVC_HDMI_HDCP_KSV_LENGTH == NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH);
    BDBG_CASSERT(BAVC_HDMI_HDCP_N_PRIVATE_KEYS == NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS);
    BDBG_CASSERT(sizeof(BHDCPlib_EncryptedHdcpKeyStruct) == sizeof(NEXUS_HdmiOutputHdcpKey));

    /* Update HDCP Configuration */
    if (handle->hdcpHandle != NULL) {
        BHDCPlib_GetConfiguration(handle->hdcpHandle, pHdcpConfig);
    }
    BKNI_Memcpy(pHdcpConfig->TxKeySet.TxAksv, pSettings->aksv.data, BAVC_HDMI_HDCP_KSV_LENGTH);
    for ( i = 0; i < BAVC_HDMI_HDCP_N_PRIVATE_KEYS; i++ )
    {
        BKNI_Memcpy(&pHdcpConfig->TxKeySet.TxKeyStructure[i], &pSettings->encryptedKeySet[i], sizeof(NEXUS_HdmiOutputHdcpKey));
    }

    pHdcpConfig->msWaitForValidVideo = pSettings->waitForValidVideo;
    pHdcpConfig->msWaitForRxR0Margin = pSettings->waitForRxR0Margin;
    pHdcpConfig->msWaitForKsvFifoMargin = pSettings->waitForKsvFifoMargin;

    pHdcpConfig->uiMaxDeviceCount = pSettings->maxDeviceCountSupported;
    pHdcpConfig->uiMaxDepth = pSettings->maxDepthSupported;

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    pHdcpConfig->eHdcp2xContentStreamControl =
        (BHDCPlib_Hdcp2xContentStreamType) pSettings->hdcp2xContentStreamControl;
#endif


    errCode = BHDCPlib_SetConfiguration(handle->hdcpHandle, pHdcpConfig);

    BKNI_Free(pHdcpConfig);

    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Set Pj Checking option */
    BHDM_HDCP_GetOptions(handle->hdmHandle, &hdcpOptions);
    hdcpOptions.PjChecking = pSettings->pjCheckEnabled;
    errCode = BHDM_HDCP_SetOptions(handle->hdmHandle, &hdcpOptions);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    /* Store Settings */
    handle->hdcpSettings = *pSettings;

    if ((hdcpVersionSelect == NEXUS_HdmiOutputHdcpVersion_e1_x)
    || ((hdcpVersionSelect == NEXUS_HdmiOutputHdcpVersion_eAuto)
        && (handle->eHdcpVersion < BHDM_HDCP_Version_e2_2)))
    {
        /* May need to enable encryption if the setting changed.  Check now */
        if ( pSettings->transmitEncrypted )
        {
            /* Check if encryption needs to be enabled and fire callback */
            NEXUS_HdmiOutput_P_UpdateHdcpState(handle);
        }
    }

    return BERR_SUCCESS;
}

/**
Summary:
Establish list of Revoked KSV's
**/
NEXUS_Error NEXUS_HdmiOutput_SetHdcpRevokedKsvs(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiOutputHdcpKsv *pRevokedKsvs,    /* attr{nelem=numKsvs;nelem_convert=NEXUS_P_HDMI_OUTPUT_HDCP_KSV_SIZE} array of revoked ksv's */
    uint16_t numKsvs                                /* Number of ksvs in the array provided */
    )
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    BHDCPlib_RevokedKsvList ksvList;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    RESOLVE_ALIAS(handle);


    /* Allocate/Create new list */
    if (numKsvs == 0) {
        /* Any attempt to authenticate should error out
        since the revoked key list can not be determined
        and potentially revoked ksvs are not checked */
        BDBG_ERR(("Number of KSVs is 0; Unknown number of KSVs")) ;
        errCode = NEXUS_INVALID_PARAMETER ;
        goto done ;
    }

    if (numKsvs > handle->revokedKsvsSize) {
        if (handle->pRevokedKsvs != NULL) {
            BKNI_Free(handle->pRevokedKsvs);
            handle->pRevokedKsvs = NULL;
            handle->revokedKsvsSize = 0;
        }
    }
    if (handle->pRevokedKsvs == NULL) {
        handle->pRevokedKsvs = BKNI_Malloc(numKsvs * sizeof(NEXUS_HdmiOutputHdcpKsv));
        if (handle->pRevokedKsvs == NULL) {
            errCode = NEXUS_OUT_OF_DEVICE_MEMORY ;
            goto done;
        }
        handle->revokedKsvsSize = numKsvs;
    }

    BKNI_Memset(handle->pRevokedKsvs, 0, handle->revokedKsvsSize * sizeof(NEXUS_HdmiOutputHdcpKsv));
    BKNI_Memcpy(handle->pRevokedKsvs, pRevokedKsvs, numKsvs * sizeof(NEXUS_HdmiOutputHdcpKsv));
    handle->numRevokedKsvs = numKsvs;

    ksvList.Ksvs = (void *)handle->pRevokedKsvs;
    ksvList.uiNumRevokedKsvs = handle->numRevokedKsvs;

    errCode = BHDCPlib_SetRevokedKSVs(handle->hdcpHandle, &ksvList);

done :
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
    }
    return errCode;
}

NEXUS_Error NEXUS_HdmiOutput_P_SetHdcpVersion(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputHdcpVersion version_select
    )
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    BHDM_HDCP_Version eHdcpVersion;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    if (IS_ALIAS(handle))
    {
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto done;
    }

    {
        uint8_t deviceAttached;
        errCode = BHDM_RxDeviceAttached(handle->hdmHandle, &deviceAttached);
        if (errCode) return BERR_TRACE(errCode);
        if (!deviceAttached) return NEXUS_SUCCESS;
    }

    if (handle->hdcpVersionSelect != version_select) {
        NEXUS_HdmiOutputHdcpState hdcpState = NEXUS_HdmiOutputHdcpState_eUnauthenticated;

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
        if (handle->eHdcpVersion == BHDM_HDCP_Version_e2_2) {
            BHDCPlib_Hdcp2x_AuthenticationStatus hdcp22_Status;

            errCode = BHDCPlib_Hdcp2x_GetAuthenticationStatus(handle->hdcpHandle, &hdcp22_Status);
            if (errCode)
                errCode = BERR_TRACE(errCode);
            else
                hdcpState = hdcp22_Status.eHdcpState;
        }
        else
#endif
        {
            BHDCPlib_Status hdcpStatus;

            errCode = BHDCPlib_GetHdcpStatus(handle->hdcpHandle, &hdcpStatus);
            if (errCode)
                errCode = BERR_TRACE(errCode);
            else
                hdcpState = hdcpStatus.eAuthenticationState;
        }

        if (hdcpState > NEXUS_HdmiOutputHdcpState_eUnauthenticated) {
            errCode = NEXUS_HdmiOutput_DisableHdcpAuthentication(handle);
            if (errCode)
                errCode = BERR_TRACE(errCode);
        }
        handle->hdcpVersionSelect = version_select;
    }

    if (version_select == NEXUS_HdmiOutputHdcpVersion_e1_x) {
        eHdcpVersion = BHDM_HDCP_Version_e1_1;
    }
    else {
        errCode = BHDM_HDCP_GetHdcpVersion(handle->hdmHandle, &eHdcpVersion);
        /* default to HDCP 1.x if HDCP Version cannot be read from the Rx */
        if (errCode != BERR_SUCCESS) {
            eHdcpVersion = BHDM_HDCP_Version_e1_1;
        }
    }

    /* if HDCP 2.2 support is not available/disabled */
    /*     set supported HDCP version to 1.x */
#if !(NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT))
    if (eHdcpVersion >= BHDM_HDCP_Version_e2_2)
    {
        eHdcpVersion = BHDM_HDCP_Version_e1_1  ;
    }
#endif


    /* close/re-open hdcplib handle for diff hdcp version if needed */
    if (handle->eHdcpVersion != eHdcpVersion)
    {
        NEXUS_HdmiOutput_P_UninitHdcp(handle);

        handle->eHdcpVersion = eHdcpVersion;
        errCode = NEXUS_HdmiOutput_P_InitHdcp(handle);
        if (errCode)
        {
            errCode = BERR_TRACE(errCode);
        }
    }
done:
    return errCode;
}

/**
Summary:
Initiate HDCP authentication

Description:
Calls to NEXUS_HdmiOutput_SetHdcpSettings() and NEXUS_HdmiOutput_SetHdcpRevokedKsvs()
should be made prior to starting authentication.

See Also:
NEXUS_HdmiOutput_SetHdcpSettings
NEXUS_HdmiOutput_SetHdcpRevokedKsvs
**/
NEXUS_Error NEXUS_HdmiOutput_StartHdcpAuthentication(
    NEXUS_HdmiOutputHandle handle)
{
    NEXUS_Error errCode = NEXUS_SUCCESS ;
    NEXUS_HdmiOutputState state;
    bool linkAuthenticated = false;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    if (IS_ALIAS(handle))
    {
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto done ;
    }

    /* Check for device */
    state = NEXUS_HdmiOutput_P_GetState(handle);
    if ( state != NEXUS_HdmiOutputState_ePoweredOn)
    {
        BDBG_ERR(("Can not start authentication; Output State: %d", state));
        errCode = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto done ;
    }

    errCode = BHDM_HDCP_IsLinkAuthenticated(handle->hdmHandle, &linkAuthenticated);
    if (errCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Error checking HDCP link status"));
        errCode = BERR_TRACE(errCode);
    }

    if (handle->eHdcpVersion == BHDM_HDCP_Version_e2_2) {
        /******************/
        /**** HDCP 2.x ****/
        /******************/
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
        handle->hdcpMonitor.hdcp22.auth.attemptCounter++ ;

        if (handle->pendingDisableAuthentication_isr)
        {
            BDBG_LOG(("Pending DisableAuthentication from previous HPD event. Skip authentication"));
            errCode = BERR_SUCCESS;
            goto done;
        }

#endif
        if (linkAuthenticated)
        {
            errCode = NEXUS_HdmiOutput_DisableHdcpEncryption(handle);
            if (errCode != BERR_SUCCESS) {
                BDBG_ERR(("Error disabling HDCP 2.x encryption"));
                errCode = BERR_TRACE(errCode);
                goto done ;
            }

            /* delay start of HDCP Authentication */
            BDBG_MSG(("Delay HDCP Auth Start (allow DisableEncryption to complete)")) ;
            BKNI_Sleep(50);
        }

    }
    else
	{
        handle->hdcpMonitor.hdcp1x.auth.attemptCounter++ ;
        /******************/
        /**** HDCP 1.x ****/
        /******************/
        /* Clean up any pending state */
        errCode = NEXUS_HdmiOutput_DisableHdcpAuthentication(handle) ;
        if (errCode)
        {
            BDBG_ERR(("Unable to Disable HDCP Authentication")) ;
        }

        /* delay start of HDCP Authentication */
        BDBG_MSG(("Delay HDCP Auth Start (allow DisableHdcpAuthentication to complete)")) ;
        BKNI_Sleep(50);
    }

    BDBG_MSG(("Starting HDCP %s Authentication",
        handle->eHdcpVersion == BHDM_HDCP_Version_e2_2 ? "2.2" : "1.x"));

    /* Reset Auth State */
    errCode = BHDCPlib_StartAuthentication(handle->hdcpHandle);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode) ;
        goto done;
    }

    handle->hdcpStarted = true;

    /* NEXUS have to drive the state machine in HDCP 1.x authentication process */
    if (handle->eHdcpVersion != BHDM_HDCP_Version_e2_2)
    {
        /* State Changed */
        NEXUS_HdmiOutput_P_UpdateHdcpState(handle);

        /* Kickstart state machine by faking timer callback */
        NEXUS_HdmiOutput_P_HdcpTimerCallback(handle);
    }


    if (handle->hdcpKeepAliveTimer) {
        NEXUS_CancelTimer(handle->hdcpKeepAliveTimer);
        handle->hdcpKeepAliveTimer = NULL;
    }
    handle->hdcpKeepAliveTimer = NEXUS_ScheduleTimer(20000, NEXUS_HdmiOutput_P_HdcpKeepAliveTimerCallback, handle);

done:
    return errCode ;
}

/**
Summary:
Terminate HDCP authentication
**/
NEXUS_Error NEXUS_HdmiOutput_DisableHdcpAuthentication(
    NEXUS_HdmiOutputHandle handle
    )
{
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    if (IS_ALIAS(handle)) return BERR_TRACE(NEXUS_NOT_SUPPORTED);

    handle->hdcpStarted = false;

    /* Clean up any pending timers */
    if ( NULL != handle->hdcpTimer )
    {
        NEXUS_CancelTimer(handle->hdcpTimer);
        handle->hdcpTimer = NULL;
    }
    if ( handle->hdcpKeepAliveTimer )
    {
        NEXUS_CancelTimer(handle->hdcpKeepAliveTimer);
        handle->hdcpKeepAliveTimer = NULL;
    }

    errCode = BHDCPlib_DisableAuthentication(handle->hdcpHandle);
    if ( errCode )
    {
        return BERR_TRACE(errCode);
    }

    if (handle->eHdcpVersion != BHDM_HDCP_Version_e2_2) {
        NEXUS_HdmiOutput_P_UpdateHdcpState(handle);
    }

    return BERR_SUCCESS;
}


NEXUS_Error NEXUS_HdmiOutput_EnableHdcpEncryption(
    NEXUS_HdmiOutputHandle handle
    )
{
    NEXUS_HdmiOutputState state;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    if (IS_ALIAS(handle)) return BERR_TRACE(NEXUS_NOT_SUPPORTED);

    /* Check for device */
    state = NEXUS_HdmiOutput_P_GetState(handle);
    if ( state != NEXUS_HdmiOutputState_ePoweredOn)
    {
        BDBG_ERR(("Can not enable encryption; Output State: %d", state));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ((handle->eHdcpVersion == BHDM_HDCP_Version_e1_1)
    || (handle->eHdcpVersion == BHDM_HDCP_Version_e1_1_Optional_Features))
    {
        rc = NEXUS_HdmiOutput_StartHdcpAuthentication(handle);
        goto done;
    }

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    BDBG_WRN(("Enabling HDCP2.2 encryption"));
    rc = BHDCPlib_Hdcp2x_EnableEncryption(handle->hdcpHandle, true);
#endif

done:
    return rc;
}


NEXUS_Error NEXUS_HdmiOutput_DisableHdcpEncryption(
    NEXUS_HdmiOutputHandle handle
    )
{
    NEXUS_HdmiOutputState state;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    if (IS_ALIAS(handle)) return BERR_TRACE(NEXUS_NOT_SUPPORTED);

    /* Check for device */
    state = NEXUS_HdmiOutput_P_GetState(handle);
    if ( state != NEXUS_HdmiOutputState_ePoweredOn )
        goto done;

    if ((handle->eHdcpVersion == BHDM_HDCP_Version_e1_1)
    || (handle->eHdcpVersion == BHDM_HDCP_Version_e1_1_Optional_Features))
    {
        return NEXUS_HdmiOutput_DisableHdcpAuthentication(handle);
    }

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    BDBG_WRN(("Disabling HDCP2.2 encryption"));
    BHDCPlib_Hdcp2x_EnableEncryption(handle->hdcpHandle, false);
#endif

done:

    return BERR_SUCCESS;
}



BHDCPlib_State NEXUS_HdmiOutput_P_GetCurrentHdcplibState(
    NEXUS_HdmiOutputHandle handle)
{
#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    BERR_Code errCode = BERR_SUCCESS;

    if (handle->eHdcpVersion == BHDM_HDCP_Version_e2_2)
    {
        BHDCPlib_Hdcp2x_AuthenticationStatus stAuthenticationStatus;

        errCode = BHDCPlib_Hdcp2x_GetAuthenticationStatus(handle->hdcpHandle, &stAuthenticationStatus);
        if (errCode != BERR_SUCCESS)
        {
            return BHDCPlib_State_eUnauthenticated;
        }
        else {
            return stAuthenticationStatus.eHdcpState;
        }
    }
    else
    {
        return handle->hdcp1xState;
    }
#else
    return handle->hdcp1xState;
#endif


}

static void NEXUS_HdmiOutput_P_HdcpKeepAliveTimerCallback(void *pContext)
{
    NEXUS_HdmiOutputHandle handle = (NEXUS_HdmiOutputHandle)pContext;
    NEXUS_HdmiOutputHdcpStatus hdcpStatus;
    int rc;

    handle->hdcpKeepAliveTimer = NULL;

    /* we stop the timer if we are authenticated */
    rc = NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdcpStatus);
    if (rc) BERR_TRACE(rc);

    if (hdcpStatus.hdcpState == NEXUS_HdmiOutputHdcpState_eEncryptionEnabled) {
        return;
    }

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    handle->hdcpMonitor.hdcp22.timeoutCounter++ ;
#endif

    /* otherwise, restart authentication */
    NEXUS_TaskCallback_Fire(handle->hdcpFailureCallback);

    /* fire stateChange call back */
    NEXUS_TaskCallback_Fire(handle->hdcpStateChangedCallback);
}

static void NEXUS_HdmiOutput_P_HdcpTimerCallback(void *pContext)
{
    NEXUS_HdmiOutputHandle output = (NEXUS_HdmiOutputHandle)pContext;
    BHDCPlib_Status hdcpStatus;
    NEXUS_Error errCode;

    BDBG_OBJECT_ASSERT(output, NEXUS_HdmiOutput);

#if 0
    BDBG_MSG(("HDCP State Machine Timer"));
#endif

    /* Mark timer as expired */
    output->hdcpTimer = NULL;

    errCode = BHDCPlib_ProcessAuthentication(output->hdcpHandle, &hdcpStatus);

    if ( errCode )
    {
        /* All failures will eventually wind up here */
        errCode = BERR_TRACE(errCode);
        output->hdcpMonitor.hdcp1x.auth.failCounter++ ;

        switch(hdcpStatus.eHdcpError)
        {
        default :
            BDBG_WRN(("HDCP Auth Error: %d", hdcpStatus.eHdcpError)) ;
        }

        BDBG_ERR(("HDCP error occurred, aborting authentication"));
        NEXUS_HdmiOutput_P_UpdateHdcpState(output);
        NEXUS_TaskCallback_Fire(output->hdcpFailureCallback);
        return;
    }

    NEXUS_HdmiOutput_P_UpdateHdcpState(output);

    if ( hdcpStatus.msRecommendedWaitTime > 0 )
    {
#if 0
        BDBG_MSG(("Arming HDCP State Machine Timer for %u ms", hdcpStatus.msRecommendedWaitTime));
#endif
        output->hdcpTimer = NEXUS_ScheduleTimer(hdcpStatus.msRecommendedWaitTime,
                                                NEXUS_HdmiOutput_P_HdcpTimerCallback,
                                                pContext);
        /* This is basically impossible to recover from */
        BDBG_ASSERT(NULL != output->hdcpTimer);
    }
    else
    {
        BDBG_WRN(("HDCP recommended no wait time.   Disabling Hdcp Authentication."));
        NEXUS_HdmiOutput_DisableHdcpAuthentication(output);
    }
}

static void NEXUS_HdmiOutput_P_UpdateHdcpState(NEXUS_HdmiOutputHandle handle)
{
    bool ready;
    BHDCPlib_Status hdcpStatus;
    BHDCPlib_State prev_state;
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);

    BHDCPlib_GetHdcpStatus(handle->hdcpHandle, &hdcpStatus);
    ready = BHDCPlib_LinkReadyForEncryption(handle->hdcpHandle);

    /* Save last HDCP error */
    handle->hdcp1xError = hdcpStatus.eHdcpError;
    prev_state = NEXUS_HdmiOutput_P_GetCurrentHdcplibState(handle);
    if (prev_state != hdcpStatus.eAuthenticationState) {
        BDBG_MSG(("Updating HDCP State from %d -> %d", prev_state, hdcpStatus.eAuthenticationState));
    }
    if ( hdcpStatus.eAuthenticationState != prev_state)
    {
        handle->hdcp1xState = hdcpStatus.eAuthenticationState;
        NEXUS_TaskCallback_Fire(handle->hdcpStateChangedCallback);

        if ( ready )
        {
            BDBG_MSG(("Authentication complete"));

            if ( handle->hdcpSettings.transmitEncrypted )
            {
                NEXUS_Error errCode;

                BDBG_MSG(("Enabling Encryption"));

                errCode = BHDCPlib_TransmitEncrypted(handle->hdcpHandle);
                if ( errCode )
                {
                    errCode = BERR_TRACE(errCode);
                }
                else {
                    prev_state = handle->hdcp1xState;
                    BHDCPlib_GetHdcpStatus(handle->hdcpHandle, &hdcpStatus);
                    if (prev_state != hdcpStatus.eAuthenticationState) {
                        BDBG_MSG(("Updating HDCP State from %d -> %d", prev_state, hdcpStatus.eAuthenticationState));
                    }
                    handle->hdcp1xState = hdcpStatus.eAuthenticationState;
                }
            }

            handle->hdcpMonitor.hdcp1x.auth.passCounter++ ;
            NEXUS_TaskCallback_Fire(handle->hdcpSuccessCallback);
        }
    }
}


NEXUS_Error NEXUS_HdmiOutput_HdcpGetDownstreamInfo(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiHdcpDownStreamInfo *pDownstream
    )
{
    BERR_Code rc = BERR_SUCCESS ;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_HdmiOutputHdcpStatus hdmiAttachedRxStatus ;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    RESOLVE_ALIAS(handle);
    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdmiAttachedRxStatus) ;
    BKNI_Memset(pDownstream, 0, sizeof(NEXUS_HdmiHdcpDownStreamInfo));

    if (handle->eHdcpVersion == BHDM_HDCP_Version_e2_2) {
        BDBG_ERR(("This API %s is not applicable for HDCP2.x", BSTD_FUNCTION));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto done;
    }

    /* if only one device is downstream, return with its KSV */
    if (!hdmiAttachedRxStatus.isHdcpRepeater)
    {
        /* only 1 rx device is attached */
        pDownstream->devices = 1 ;
        pDownstream->depth = 1 ;

        goto done ;
    }

    if (hdmiAttachedRxStatus.hdcpState == NEXUS_HdmiOutputHdcpState_eRepeaterAuthenticationFailure)
    {
        pDownstream->maxDepthExceeded=
            (hdmiAttachedRxStatus.hdcpError == NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded);

        pDownstream->maxDevicesExceeded=
            (hdmiAttachedRxStatus.hdcpError == NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded);
    }

    pDownstream->isRepeater = hdmiAttachedRxStatus.isHdcpRepeater;
    BKNI_Memcpy(&pDownstream->repeaterKsv, &hdmiAttachedRxStatus.bksv, sizeof(NEXUS_HdmiHdcpKsv));

    /* multiple devices are downstream  */
    /* determine downstream depth levels and device count */
    rc = BHDM_HDCP_GetRepeaterDepth(handle->hdmHandle, &pDownstream->depth) ;
    rc = BERR_TRACE(rc) ;

    rc = BHDM_HDCP_GetRepeaterDeviceCount(handle->hdmHandle, &pDownstream->devices) ;
    rc = BERR_TRACE(rc) ;


done:
    return errCode ;
}


NEXUS_Error NEXUS_HdmiOutput_HdcpGetDownstreamKsvs(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiHdcpKsv *pKsvs, /* attr{nelem=numDevices;nelem_out=pNumRead} */
    unsigned numDevices,
    unsigned *pNumRead
    )
{
    BERR_Code rc = BERR_SUCCESS ;
    NEXUS_Error errCode = NEXUS_SUCCESS;
    NEXUS_HdmiOutputHdcpStatus hdmiAttachedRxStatus ;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    RESOLVE_ALIAS(handle);
    /* Coverity: 32292  */
    if( pKsvs == NULL || (numDevices ==0))
    {
        errCode = BERR_TRACE(NEXUS_NOT_INITIALIZED);
        goto done;
    }

    if (handle->eHdcpVersion == BHDM_HDCP_Version_e2_2) {
        BDBG_ERR(("This API %s is not applicable for HDCP2.x", BSTD_FUNCTION));
        errCode = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto done;
    }

    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdmiAttachedRxStatus) ;


    /* if only one device is downstream, return with its KSV */
    if (!hdmiAttachedRxStatus.isHdcpRepeater)
    {

        /* only 1 rx device is attached */
        *pNumRead = 1 ;
        BKNI_Memcpy(&pKsvs->data, &hdmiAttachedRxStatus.bksv, sizeof(NEXUS_HdmiHdcpKsv)) ;

        goto done ;
    }
    else {
        *pNumRead = numDevices;
    }

    /* retrieve KSVs for each downtream device */
    rc = BHDM_HDCP_GetRepeaterKsvFifo(handle->hdmHandle,
        (uint8_t *) pKsvs, (uint16_t) ((uint8_t) numDevices * sizeof(NEXUS_HdmiHdcpKsv))) ;
    rc = BERR_TRACE(rc) ;


done:

    return errCode ;
}



/**
Summary:
Get HDCP Status
**/
NEXUS_Error NEXUS_HdmiOutput_GetHdcpStatus(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputHdcpStatus *pStatus /* [out] */
    )
{
    NEXUS_Error errCode = NEXUS_SUCCESS;
    BHDCPlib_RxInfo rxInfo;
    NEXUS_HdmiOutputState state;
    BHDM_HDCP_Version eHdcpVersion;

    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    RESOLVE_ALIAS(handle);
    BDBG_ASSERT(NULL != pStatus);

    BDBG_CASSERT(NEXUS_HdmiOutputHdcpState_eMax == (BHDCPlib_State_ePjLinkIntegrityFailure+1));
    BDBG_CASSERT(NEXUS_HdmiOutputHdcpError_eMax == (NEXUS_HdmiOutputHdcpError)BHDCPlib_HdcpError_eCount);

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->hdcpState = NEXUS_HdmiOutputHdcpState_eUnauthenticated ;
    pStatus->hdcpError = NEXUS_HdmiOutputHdcpError_eRxBksvError;

    state = NEXUS_HdmiOutput_P_GetState(handle);
    if ( state != NEXUS_HdmiOutputState_ePoweredOn )
    {
        errCode = NEXUS_NOT_AVAILABLE ;
        goto done;
    }

    errCode = BHDM_HDCP_GetHdcpVersion(handle->hdmHandle, &eHdcpVersion);
    /* default to HDCP 1.x if HDCP Version cannot be read from the Rx */
    if (errCode != BERR_SUCCESS) {
        eHdcpVersion = BHDM_HDCP_Version_e1_1;
    }
    pStatus->rxMaxHdcpVersion = (eHdcpVersion == BHDM_HDCP_Version_e2_2) ?
        NEXUS_HdcpVersion_e2x : NEXUS_HdcpVersion_e1x;

    pStatus->selectedHdcpVersion = (handle->eHdcpVersion == BHDM_HDCP_Version_e2_2) ?
        NEXUS_HdcpVersion_e2x : NEXUS_HdcpVersion_e1x;

#if NEXUS_HAS_SAGE && defined(NEXUS_HAS_HDCP_2X_SUPPORT)
    {
        BHDCPlib_Hdcp2x_AuthenticationStatus stAuthenticationStatus;

        if (handle->eHdcpVersion == BHDM_HDCP_Version_e2_2) {
            errCode = BHDCPlib_Hdcp2x_GetAuthenticationStatus(handle->hdcpHandle, &stAuthenticationStatus);
            if (errCode != BERR_SUCCESS)
            {
                BDBG_ERR(("Error retrieving HDCP2.x authentication status"));
                errCode = BERR_TRACE(errCode);
                goto done;
            }

            pStatus->hdcpState = stAuthenticationStatus.eHdcpState;
            pStatus->hdcpError = stAuthenticationStatus.eAuthenticationError;

            pStatus->linkReadyForEncryption = (stAuthenticationStatus.eHdcpState == BHDCPlib_State_eEncryptionEnabled)
                                            || (stAuthenticationStatus.eHdcpState == BHDCPlib_State_eLinkAuthenticated);
            pStatus->transmittingEncrypted = (stAuthenticationStatus.eHdcpState == BHDCPlib_State_eEncryptionEnabled);
            pStatus->hdcp2_2Features = true ;
            pStatus->hdcp2_2RxInfo.hdcp1_xDeviceDownstream = stAuthenticationStatus.hdcp1DeviceDownstream;
            pStatus->isHdcpRepeater = stAuthenticationStatus.downstreamIsRepeater;

            if ((pStatus->hdcpState == NEXUS_HdmiOutputHdcpState_eEncryptionEnabled) ||
                (pStatus->hdcpState == NEXUS_HdmiOutputHdcpState_eLinkAuthenticated)) {
                BDBG_MSG(("HDCP 2.2 authentication successful"));
            }
            goto done;
        }
    }
#endif

    /* HDCP 1.x */
    {
        BHDCPlib_Status hdcpStatus;
        BHDCPlib_GetHdcpStatus(handle->hdcpHandle, &hdcpStatus);

        pStatus->hdcpState = hdcpStatus.eAuthenticationState;
        pStatus->hdcpError = hdcpStatus.eHdcpError;
        pStatus->linkReadyForEncryption = BHDCPlib_LinkReadyForEncryption(handle->hdcpHandle);
        pStatus->transmittingEncrypted = (hdcpStatus.eAuthenticationState == BHDCPlib_State_eEncryptionEnabled);

        errCode = BHDCPlib_GetReceiverInfo(handle->hdcpHandle, &rxInfo);
        if ( BERR_SUCCESS == errCode )
        {
            pStatus->isHdcpRepeater = rxInfo.bIsHdcpRepeater;
            pStatus->ksvFifoReady = (rxInfo.uiRxBCaps & BHDM_HDCP_RxCaps_eKsvFifoReady)?true:false;
            pStatus->i2c400Support = (rxInfo.uiRxBCaps & BHDM_HDCP_RxCaps_eI2c400KhzSupport)?true:false;
            pStatus->hdcp1_1Features = (rxInfo.uiRxBCaps & BHDM_HDCP_RxCaps_eHDCP_1_1_Features)?true:false;
            pStatus->fastReauthentication = (rxInfo.uiRxBCaps & BHDM_HDCP_RxCaps_eFastReauth)?true:false;
            BKNI_Memcpy(&pStatus->bksv.data, &rxInfo.RxBksv[0], sizeof(rxInfo.RxBksv));

            if ((pStatus->hdcpState == NEXUS_HdmiOutputHdcpState_eEncryptionEnabled) ||
                (pStatus->hdcpState == NEXUS_HdmiOutputHdcpState_eLinkAuthenticated)) {
                BDBG_MSG(("HDCP 1.x authentication successful"));
            }
        }
        else
        {
            errCode = BERR_TRACE(errCode);
        }

    }
done:

    return NEXUS_SUCCESS;
}

#else /* NEXUS_HAS_SECURITY */
/* No security - use stubs for HDCP */
NEXUS_Error NEXUS_HdmiOutput_P_InitHdcp(NEXUS_HdmiOutputHandle output)
{
    BSTD_UNUSED(output);
    return BERR_SUCCESS;
}

void NEXUS_HdmiOutput_P_UninitHdcp(NEXUS_HdmiOutputHandle output)
{
    BSTD_UNUSED(output);
}

void NEXUS_HdmiOutput_P_CloseHdcp(NEXUS_HdmiOutputHandle output)
{
    BSTD_UNUSED(output);
}

void NEXUS_HdmiOutput_P_HdcpNotifyHotplug(NEXUS_HdmiOutputHandle output)
{
    BSTD_UNUSED(output);
    return;
}

void NEXUS_HdmiOutput_GetHdcpSettings(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputHdcpSettings *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_CallbackDesc_Init(&pSettings->stateChangedCallback);
    NEXUS_CallbackDesc_Init(&pSettings->successCallback);
    NEXUS_CallbackDesc_Init(&pSettings->failureCallback);
}

NEXUS_Error NEXUS_HdmiOutput_SetHdcpSettings(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiOutputHdcpSettings *pSettings
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    BSTD_UNUSED(pSettings);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_HdmiOutput_SetHdcpRevokedKsvs(
    NEXUS_HdmiOutputHandle handle,
    const NEXUS_HdmiOutputHdcpKsv *pRevokedKsvs,    /* attr{nelem=numKsvs;nelem_convert=NEXUS_P_HDMI_OUTPUT_HDCP_KSV_SIZE} array of revoked ksv's */
    uint16_t numKsvs                                /* Number of keys in the array provided */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    BSTD_UNUSED(pRevokedKsvs);
    BSTD_UNUSED(numKsvs);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_HdmiOutput_StartHdcpAuthentication(
    NEXUS_HdmiOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_HdmiOutput_DisableHdcpAuthentication(
    NEXUS_HdmiOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    /* silent, is already disabled */
    return NEXUS_SUCCESS ;
}

NEXUS_Error NEXUS_HdmiOutput_EnableHdcpEncryption(
    NEXUS_HdmiOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_HdmiOutput_DisableHdcpEncryption(
    NEXUS_HdmiOutputHandle handle
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    /* silent, is already disabled */
    return NEXUS_SUCCESS ;
}

NEXUS_Error NEXUS_HdmiOutput_GetHdcpStatus(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiOutputHdcpStatus *pStatus /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_HdmiOutput_HdcpGetDownstreamInfo(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiHdcpDownStreamInfo *pDownstream
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    BSTD_UNUSED(pDownstream);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_HdmiOutput_HdcpGetDownstreamKsvs(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiHdcpKsv *pKsvs, /* attr{nelem=numDevices;nelem_out=pNumRead} */
    unsigned numDevices,
    unsigned *pNumRead
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    BSTD_UNUSED(pKsvs);
    BSTD_UNUSED(numDevices);
    BSTD_UNUSED(pNumRead);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

#endif /* NEXUS_HAS_SECURITY */


#if !NEXUS_HAS_SECURITY || !NEXUS_HAS_SAGE || !defined(NEXUS_HAS_HDCP_2X_SUPPORT)
NEXUS_Error NEXUS_HdmiOutput_SetHdcp2xBinKeys(
    NEXUS_HdmiOutputHandle handle,
    const uint8_t *pBinFileBuffer,  /* attr{nelem=length} pointer to encrypted key buffer */
    uint32_t length                 /* size of data in pBinFileBuffer in bytes */
)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    BSTD_UNUSED(pBinFileBuffer);
    BSTD_UNUSED(length);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}

NEXUS_Error NEXUS_HdmiOutput_SetRepeaterInput(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_HdmiInputHandle input
)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    return input ? BERR_TRACE(BERR_NOT_SUPPORTED) : NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_HdmiOutput_GetHdcp2xReceiverIdListData(
    NEXUS_HdmiOutputHandle handle,
    NEXUS_Hdcp2xReceiverIdListData *pReceiverIdListData
)
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_HdmiOutput);
    BSTD_UNUSED(pReceiverIdListData);
    return BERR_TRACE(BERR_NOT_SUPPORTED);
}
#endif
