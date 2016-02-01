/***************************************************************************
*     (c)2004-2013 Broadcom Corporation
*  
*  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
* $brcm_Workfile: $
* $brcm_Revision: $
* $brcm_Date: $
*
* API Description:
*   API name: WMDRMPD Module
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/

#include "nexus_wmdrmpd_module.h"
#include "priv/nexus_core.h"

BDBG_MODULE(nexus_wmdrmpd_module);

NEXUS_ModuleHandle g_NEXUS_wmDrmPdModule;
NEXUS_WmDrmPdModuleData g_NEXUS_wmDrmPdModuleData;

static bool g_ioThreadExit=false;
static BKNI_EventHandle g_hIoEvent;
static NEXUS_ThreadHandle g_hIoThread;
static NEXUS_WmDrmPdIoHandle g_hIo;
static unsigned g_io_refcnt;

static void NEXUS_WmDrmPd_P_IoThread(void *pParam);

static NEXUS_Error NEXUS_WmDrmPd_CreateIoThread(
    const NEXUS_ThreadSettings *pSettings
    )
{
    NEXUS_Error errCode;

    errCode = BKNI_CreateEvent(&g_hIoEvent);
    if ( errCode )
    {
        errCode = BERR_TRACE(errCode);
        goto err_event;
    }

    g_ioThreadExit = false;
    g_hIoThread = NEXUS_Thread_Create("WMDRMPD IO", NEXUS_WmDrmPd_P_IoThread, NULL, pSettings);
    if ( NULL == g_hIoThread )
    {
        errCode = BERR_TRACE(BERR_OS_ERROR);
        goto err_thread;
    }

    return NEXUS_SUCCESS;

err_thread:
    BKNI_DestroyEvent(g_hIoEvent);
err_event:
    return errCode;
}

static void NEXUS_WmDrmPd_DestroyIoThread(void)
{
    g_ioThreadExit = true;
    BKNI_SetEvent(g_hIoEvent);
    NEXUS_Thread_Destroy(g_hIoThread);
    BKNI_DestroyEvent(g_hIoEvent);
    g_hIoEvent = NULL;
    g_hIoThread = NULL;
}

static void NEXUS_WmDrmPd_IoJobRequest(void *pParam, int param)
{
    BDBG_MSG(("Entering NEXUS_WmDrmPd_IoJobRequest"));
    BSTD_UNUSED(pParam);
    BSTD_UNUSED(param);
    BKNI_SetEvent(g_hIoEvent);
}

static void NEXUS_WmDrmPd_P_IoThread(void *pParam)
{
    NEXUS_Error errCode;
    NEXUS_WmDrmPdIoHandle hIo;
    NEXUS_WmDrmPdIoRequest request;
    NEXUS_WmDrmPdIoResponse response;

    BSTD_UNUSED(pParam);

    BDBG_MSG(("IO Thread Starting"));

    hIo = g_hIo;

    while ( g_ioThreadExit == false )
    {
        errCode = BKNI_WaitForEvent(g_hIoEvent, BKNI_INFINITE);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            break;
        }
        BDBG_MSG(("IO thread awake"));
        for ( ;; ) 
        {
            errCode = NEXUS_WmDrmPdIo_GetRequest(hIo, &request);
            if ( NEXUS_SUCCESS != errCode ) 
            {
                break;
            }
            BDBG_MSG(("IO Operation Type %u", request.operation));
            if ( request.operation == NEXUS_WmDrmPdIoOperation_eNone ) 
            {
                break;
            }
            NEXUS_WmDrmPdIo_GetDefaultIoResponse(hIo, &request, &response);
            BDBG_MSG(("Processing IO Request %u", request.requestId));
            NEXUS_WmDrmPd_P_ProcessIoRequest(&request, &response);
            BDBG_MSG(("Responding to IO Request %u", request.requestId));
            errCode = NEXUS_WmDrmPdIo_SendResponse(hIo, &response);
            if ( errCode )
            {
                (void)BERR_TRACE(errCode);
            }
        }
    }
    BDBG_MSG(("IO Thread Exiting"));
}

void NEXUS_WmDrmPdModule_GetDefaultSettings(
    NEXUS_WmDrmPdModuleSettings *pSettings    /* [out] */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_WmDrmPdModuleSettings));
    NEXUS_Thread_GetDefaultSettings(&pSettings->ioThreadSettings);
}

NEXUS_ModuleHandle NEXUS_WmDrmPdModule_Init(
    const NEXUS_WmDrmPdModuleSettings *pSettings
    )
{
    NEXUS_WmDrmPdModuleSettings defaults;

    if ( NULL == pSettings )
    {
        NEXUS_WmDrmPdModule_GetDefaultSettings(&defaults);
        pSettings = &defaults;
    }

    if (pSettings->modules.wmdrmpdCore == NULL || pSettings->modules.wmdrmpdIo == NULL) {
        goto err_module;
    }

    g_NEXUS_wmDrmPdModule = NEXUS_Module_Create("wmdrmpd", NULL);
    if ( NULL == g_NEXUS_wmDrmPdModule )
    {
        (void)BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_module;
    }

    NEXUS_UseModule(pSettings->modules.wmdrmpdCore);
    NEXUS_UseModule(pSettings->modules.wmdrmpdIo);
    BKNI_Memset(&g_NEXUS_wmDrmPdModuleData, 0, sizeof(g_NEXUS_wmDrmPdModuleData));
    g_NEXUS_wmDrmPdModuleData.settings = *pSettings;
    return g_NEXUS_wmDrmPdModule;

err_module:
    return NULL;
}

void NEXUS_WmDrmPdModule_Uninit( void )
{
    NEXUS_Module_Destroy(g_NEXUS_wmDrmPdModule);   
    g_NEXUS_wmDrmPdModule = NULL;
}

NEXUS_WmDrmPdIoHandle nexus_wmdrmpd_p_create_io(void)
{
    if (g_io_refcnt++ == 0) {
        NEXUS_WmDrmPdIoSettings ioSettings;
        NEXUS_Error errCode;
        
        NEXUS_WmDrmPdIo_GetDefaultSettings(&ioSettings);
        ioSettings.heapIndex = g_NEXUS_wmDrmPdModuleData.settings.ioHeapIndex;
        ioSettings.jobCallback.callback = NEXUS_WmDrmPd_IoJobRequest;
        g_hIo = NEXUS_WmDrmPdIo_Create(&ioSettings);
        if ( NULL == g_hIo )
        {
            (void)BERR_TRACE(NEXUS_UNKNOWN);
            g_io_refcnt--;
            return NULL;
        }
    
        errCode = NEXUS_WmDrmPd_CreateIoThread(&g_NEXUS_wmDrmPdModuleData.settings.ioThreadSettings);
        if ( errCode )
        {
            (void)BERR_TRACE(errCode);
            NEXUS_WmDrmPdIo_Destroy(g_hIo);
            g_io_refcnt--;
            return NULL;
        }
    }
    return g_hIo;
}

void nexus_wmdrmpd_p_destroy_io(void)
{
    if (--g_io_refcnt == 0) {
        NEXUS_StopCallbacks(g_hIo);
        NEXUS_WmDrmPd_DestroyIoThread();
        NEXUS_WmDrmPdIo_Destroy(g_hIo);
    }
}
