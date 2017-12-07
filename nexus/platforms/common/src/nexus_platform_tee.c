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

#include "nexus_platform_priv.h"

#if NEXUS_TEE_SUPPORT

#ifdef NEXUS_BASE_OS_linuxuser
#include "libastra_api.h"
#else
#include "astra_api.h"
#endif

BDBG_OBJECT_ID(NEXUS_TeeClient);
BDBG_MODULE(nexus_platform_tee);

typedef struct NEXUS_TeeClient
{
    BDBG_OBJECT(NEXUS_TeeClient)
    astra_client_handle hAstraClient;
    BTEE_ClientEventCallback pEventCallback_isr;
    void *pEventPrivateData;
} NEXUS_TeeClient;

static void NEXUS_Platform_P_TeeEventHandler(astra_event event, void *pCallbackData, void *pPrivateData)
{
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pPrivateData;

    BDBG_CASSERT(ASTRA_EVENT_MSG_RECEIVED == (int)BTEE_ClientEvent_eMessageReceived);
    BDBG_CASSERT(ASTRA_EVENT_UAPP_EXIT == (int)BTEE_ClientEvent_eApplicationExit);
    BDBG_CASSERT(ASTRA_EVENT_MAX == (int)   BTEE_ClientEvent_eMax);

    BSTD_UNUSED(pCallbackData);

    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);
    BDBG_ASSERT(pClient->pEventCallback_isr != NULL);

    BKNI_EnterCriticalSection();
    pClient->pEventCallback_isr((BTEE_ClientEvent)event, pClient->pEventPrivateData);
    BKNI_LeaveCriticalSection();
}

static BERR_Code NEXUS_Platform_P_TeeClientCreate(void *pInstanceData, const char *pName ,const BTEE_ClientCreateSettings *pSettings, void **pClientPrivate)
{
    BERR_Code rc;

    NEXUS_TeeClient *pClient = BKNI_Malloc(sizeof(NEXUS_TeeClient));
    if ( NULL == pClient )
    {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    BSTD_UNUSED(pInstanceData);
    BKNI_Memset(pClient, 0, sizeof(NEXUS_TeeClient));
    BDBG_OBJECT_SET(pClient, NEXUS_TeeClient);
    pClient->pEventCallback_isr = pSettings->pEventCallback_isr;
    pClient->pEventPrivateData = pSettings->pCallbackData;
    pClient->hAstraClient = astra_client_open(pName, NEXUS_Platform_P_TeeEventHandler, pClient);
    if ( NULL == pClient->hAstraClient )
    {
        rc = BERR_TRACE(BERR_UNKNOWN);
        goto err_astra;
    }

    *pClientPrivate = (void *)pClient;
    return BERR_SUCCESS;

err_astra:
    BDBG_OBJECT_DESTROY(pClient, NEXUS_TeeClient);
    BKNI_Free(pClient);
err_malloc:
    return rc;
}

static void NEXUS_Platform_P_TeeClientDestroy(void *pClientPrivate)
{
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;

    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);
    astra_client_close(pClient->hAstraClient);
    BDBG_OBJECT_DESTROY(pClient, NEXUS_TeeClient);
    BKNI_Free(pClient);
}

static BERR_Code NEXUS_Platform_P_TeeClientReceiveMessage(void *pClientPrivate, void **pConnection, void *pMessage, size_t maxLength, size_t *pLength, int timeoutMsec)
{
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;
    astra_peer_handle hPeer;
    int rc;

    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);

    *pLength = maxLength;
    rc = astra_msg_receive(pClient->hAstraClient, &hPeer, pMessage, pLength, timeoutMsec);
    if ( rc )
    {
        *pLength = 0;
        *pConnection = NULL;
        return rc == ETIMEDOUT ? BERR_TIMEOUT : BERR_TRACE(BERR_UNKNOWN);
    }

    *pConnection = (void *)hPeer;
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_Platform_P_TeeClientAlloc(void *pClientPrivate, size_t size, void **pMemory)
{
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;
    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);
    BDBG_ASSERT(NULL != pMemory);

    *pMemory = astra_mem_alloc(pClient->hAstraClient, size);
    if ( NULL == *pMemory )
    {
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    return BERR_SUCCESS;
}

static void NEXUS_Platform_P_TeeClientFree(void *pClientPrivate, void *pMemory)
{
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;
    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);

    astra_mem_free(pClient->hAstraClient, pMemory);
}

static BERR_Code NEXUS_Platform_P_TeeClientSecureAlloc(void *pClientPrivate, size_t size, uint32_t *pAddress)
{
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;
    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);

    *pAddress = astra_pmem_alloc(pClient->hAstraClient, size);
    if ( 0 == *pAddress )
    {
        return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
    }
    return BERR_SUCCESS;
}

static void NEXUS_Platform_P_TeeClientSecureFree(void *pClientPrivate, uint32_t address)
{
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;
    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);

    astra_pmem_free(pClient->hAstraClient, address);
}

static BERR_Code NEXUS_Platform_P_TeeClientAddrToOffset(void *pClientPrivate, void *pMemory, uint32_t *pOffset)
{
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;
    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);

    *pOffset = astra_vaddr2offset(pClient->hAstraClient, pMemory);
    if ( 0 == *pOffset )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_Platform_P_TeeClientOffsetToAddr(void *pClientPrivate, uint32_t offset, void **pMemory)
{
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;
    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);

    *pMemory = astra_offset2vaddr(pClient->hAstraClient, offset);
    if ( NULL == *pMemory )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_Platform_P_TeeClientContextSwitch(void *pClientPrivate)
{
    int rc;
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;
    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);

    rc = astra_call_smc(pClient->hAstraClient, ASTRA_SMC_CODE_SWITCH);
    if ( rc )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_Platform_P_TeeFileOpen(void *pClientPrivate, const char *pPath, int flags, void **pFilePrivate)
{
    astra_file_handle hFile;
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;
    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);

    *pFilePrivate = NULL;
    hFile = astra_file_open(pClient->hAstraClient, pPath, flags);
    if ( NULL == hFile )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }
    *pFilePrivate = (void *)hFile;
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_Platform_P_TeeFileRead(void *pFilePrivate, uint32_t address, size_t bytesToRead, size_t *pBytesRead)
{
    int rc;

    *pBytesRead = 0;
    rc = astra_file_read(pFilePrivate, address, bytesToRead);
    if ( rc >= 0 )
    {
        *pBytesRead = rc;
        return BERR_SUCCESS;
    }
    return BERR_TRACE(BERR_UNKNOWN);
}

static BERR_Code NEXUS_Platform_P_TeeFileWrite(void *pFilePrivate, uint32_t address, size_t bytesToWrite, size_t *pBytesWritten)
{
    int rc;
    *pBytesWritten = 0;
    rc = astra_file_write(pFilePrivate, address, bytesToWrite);
    if ( rc >= 0 )
    {
        *pBytesWritten = rc;
        return BERR_SUCCESS;
    }
    return BERR_TRACE(BERR_UNKNOWN);
}

static void NEXUS_Platform_P_TeeFileClose(void *pFilePrivate)
{
    astra_file_close(pFilePrivate);
}

static BERR_Code NEXUS_Platform_P_TeeAppOpen(void *pClientPrivate, const char *pName, const char *pPath, void **pAppPrivate)
{
    astra_uapp_handle hApp;
    NEXUS_TeeClient *pClient = (NEXUS_TeeClient *)pClientPrivate;
    BDBG_OBJECT_ASSERT(pClient, NEXUS_TeeClient);

    *pAppPrivate = NULL;
    hApp = astra_uapp_open(pClient->hAstraClient, pName, pPath);
    if ( NULL == hApp )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }
    *pAppPrivate = (void *)hApp;
    return BERR_SUCCESS;
}

static void NEXUS_Platform_P_TeeAppClose(void *pAppPrivate)
{
    astra_uapp_close(pAppPrivate);
}

static BERR_Code NEXUS_Platform_P_TeeConnectionOpen(void *pAppPrivate, const char *pServiceName, const BTEE_ConnectionSettings *pSettings, void **pConnectionPrivate)
{
    astra_peer_handle hPeer;

    BSTD_UNUSED(pSettings);

    *pConnectionPrivate = NULL;
    hPeer = astra_peer_open(pAppPrivate, pServiceName);
    if ( NULL == hPeer )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    *pConnectionPrivate = (void *)hPeer;
    return BERR_SUCCESS;
}

static BERR_Code NEXUS_Platform_P_TeeConnectionSendMessage(void *pConnectionPrivate, const void *pMessage, size_t messageLength)
{
    int rc;

    rc = astra_msg_send(pConnectionPrivate, pMessage, messageLength);
    if ( rc )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }
    return BERR_SUCCESS;
}

static void NEXUS_Platform_P_TeeConnectionClose(void *pConnectionPrivate)
{
    astra_peer_close(pConnectionPrivate);
}

static BERR_Code NEXUS_Platform_P_TeeGetStatus(BTEE_InstanceStatus *pStatus)
{
    astra_status astraStatus;
    astra_version astraVersion;

    if ( NULL == pStatus )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (astra_status_get(&astraStatus) != 0)
    {
        BDBG_WRN(("Unable to query Astra status"));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }
    if (astra_version_get(&astraVersion) != 0)
    {
        BDBG_WRN(("Unable to query Astra version"));
        return BERR_TRACE(BERR_NOT_INITIALIZED);
    }

    pStatus->enabled = astraStatus.up;
    pStatus->version.major = astraVersion.major;
    pStatus->version.minor = astraVersion.minor;
    pStatus->version.build = astraVersion.build;
    return BERR_SUCCESS;
}


BTEE_InstanceHandle NEXUS_Platform_P_CreateTeeInstance(void)
{
    BERR_Code rc;
    BTEE_InstanceHandle hInstance;
    BTEE_InstanceCreateSettings createSettings;

    BTEE_Instance_GetDefaultCreateSettings(&createSettings);
    createSettings.pInstanceData = NULL;    /* No global data required */
    createSettings.ClientCreate = NEXUS_Platform_P_TeeClientCreate;
    createSettings.ClientDestroy = NEXUS_Platform_P_TeeClientDestroy;
    createSettings.ClientReceiveMessage = NEXUS_Platform_P_TeeClientReceiveMessage;
    createSettings.ClientAlloc = NEXUS_Platform_P_TeeClientAlloc;
    createSettings.ClientFree = NEXUS_Platform_P_TeeClientFree;
    createSettings.ClientSecureAlloc = NEXUS_Platform_P_TeeClientSecureAlloc;
    createSettings.ClientSecureFree = NEXUS_Platform_P_TeeClientSecureFree;
    createSettings.ClientAddrToOffset = NEXUS_Platform_P_TeeClientAddrToOffset;
    createSettings.ClientOffsetToAddr = NEXUS_Platform_P_TeeClientOffsetToAddr;
    createSettings.ClientContextSwitch = NEXUS_Platform_P_TeeClientContextSwitch;
    createSettings.FileOpen = NEXUS_Platform_P_TeeFileOpen;
    createSettings.FileRead = NEXUS_Platform_P_TeeFileRead;
    createSettings.FileWrite = NEXUS_Platform_P_TeeFileWrite;
    createSettings.FileClose = NEXUS_Platform_P_TeeFileClose;
    createSettings.AppOpen = NEXUS_Platform_P_TeeAppOpen;
    createSettings.AppClose = NEXUS_Platform_P_TeeAppClose;
    createSettings.ConnectionOpen = NEXUS_Platform_P_TeeConnectionOpen;
    createSettings.ConnectionSendMessage = NEXUS_Platform_P_TeeConnectionSendMessage;
    createSettings.ConnectionClose = NEXUS_Platform_P_TeeConnectionClose;
    createSettings.GetStatus = NEXUS_Platform_P_TeeGetStatus;

    rc = BTEE_Instance_Create(&createSettings, &hInstance);
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        return NULL;
    }
    return hInstance;
}

void NEXUS_Platform_P_DestroyTeeInstance(BTEE_InstanceHandle teeHandle)
{
    BTEE_Instance_Destroy(teeHandle);
}

#endif
