/***************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ***************************************************************/

#include "bstd.h"
#include "bstd_defs.h"
#include "berr.h"
#include "bdbg.h"
#include "bkni.h"
#include "btee.h"
#include "btee_instance.h"
#include "blst_slist.h"

BDBG_OBJECT_ID(BTEE_Instance);
BDBG_OBJECT_ID(BTEE_Client);
BDBG_OBJECT_ID(BTEE_Application);
BDBG_OBJECT_ID(BTEE_Connection);
BDBG_OBJECT_ID(BTEE_File);
BDBG_OBJECT_ID(BTEE_Memory);
BDBG_OBJECT_ID(BTEE_SecureMemory);

BDBG_MODULE(btee);

typedef struct BTEE_Instance
{
    BDBG_OBJECT(BTEE_Instance)
    void *pPrivateData;
    BTEE_InstanceCreateSettings createSettings;
    BLST_S_HEAD(ClientList, BTEE_Client) clientList;
} BTEE_Instance;

typedef struct BTEE_Application
{
    BDBG_OBJECT(BTEE_Application)
    void *pPrivateData;
    BTEE_ClientHandle hClient;
    BLST_S_ENTRY(BTEE_Application) node;
    BLST_S_HEAD(ConnectionList, BTEE_Connection) connectionList;
} BTEE_Application;

typedef struct BTEE_Connection
{
    BDBG_OBJECT(BTEE_Connection)
    void *pPrivateData;
    BTEE_ApplicationHandle hApp;
    BLST_S_ENTRY(BTEE_Connection) node;
} BTEE_Connection;

typedef struct BTEE_File
{
    BDBG_OBJECT(BTEE_File)
    void *pPrivateData;
    BTEE_ClientHandle hClient;
    BLST_S_ENTRY(BTEE_File) node;
} BTEE_File;

typedef struct BTEE_Memory
{
    BDBG_OBJECT(BTEE_Memory)
    void *pMemory;
    uint32_t length;
    BLST_S_ENTRY(BTEE_Memory) node;
} BTEE_Memory;

typedef struct BTEE_SecureMemory
{
    BDBG_OBJECT(BTEE_SecureMemory)
    uint64_t address;
    uint32_t length;
    BLST_S_ENTRY(BTEE_SecureMemory) node;
} BTEE_SecureMemory;

typedef struct BTEE_Client
{
    BDBG_OBJECT(BTEE_Client)
    void *pPrivateData;
    BTEE_InstanceHandle hInstance;
    BLST_S_ENTRY(BTEE_Client) node;
    BLST_S_HEAD(AppList, BTEE_Application) appList;
    BLST_S_HEAD(FileList, BTEE_File) fileList;
    BLST_S_HEAD(MemList, BTEE_Memory) memList;
    BLST_S_HEAD(SecureMemList, BTEE_SecureMemory) secureMemList;
} BTEE_Client;

void BTEE_Client_GetDefaultCreateSettings(
    BTEE_InstanceHandle hInstance,       /* Instance Handle */
    BTEE_ClientCreateSettings *pSettings /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hInstance, BTEE_Instance);
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BTEE_Client_Create(
    BTEE_InstanceHandle hInstance,                  /* Instance Handle */
    const char *pName,                              /* Client Name */
    const BTEE_ClientCreateSettings *pSettings,     /* Client Settings */
    BTEE_ClientHandle *pClient                      /* [out] */
    )
{
    BERR_Code rc = BERR_SUCCESS;
    BTEE_Client *hClient;

    BDBG_OBJECT_ASSERT(hInstance, BTEE_Instance);
    BDBG_ASSERT(NULL != pName);
    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pClient);

    /* Allocate and initialize client strcture */
    hClient = BKNI_Malloc(sizeof(BTEE_Client));
    if ( NULL == hClient )
    {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }
    BKNI_Memset(hClient, 0, sizeof(BTEE_Client));
    BDBG_OBJECT_SET(hClient, BTEE_Client);
    BLST_S_INSERT_HEAD(&hInstance->clientList, hClient, node);
    hClient->hInstance = hInstance;

    /* Call instance constructor */
    rc = hInstance->createSettings.ClientCreate(hInstance->pPrivateData, pName, pSettings, &hClient->pPrivateData);
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto err_create;
    }

    /* Success */
    *pClient = hClient;
    return BERR_SUCCESS;

err_create:
    BLST_S_REMOVE(&hInstance->clientList, hClient, BTEE_Client, node);
    BDBG_OBJECT_DESTROY(hClient, BTEE_Client);
    BKNI_Free(hClient);
err_malloc:
    *pClient = NULL;
    return rc;
}

void BTEE_Client_Destroy(
    BTEE_ClientHandle hClient
    )
{
    BTEE_ApplicationHandle hApp;
    BTEE_FileHandle hFile;
    BTEE_Memory *pMem;
    BTEE_SecureMemory *pSecureMem;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);

    /* Clean up dependent resources left open */
    for ( hApp = BLST_S_FIRST(&hClient->appList); NULL != hApp; hApp = BLST_S_NEXT(hApp, node) )
    {
        BDBG_WRN(("Application %p leaked on shutdown", (void *)hApp));
    }

    for ( hFile = BLST_S_FIRST(&hClient->fileList); NULL != hFile; hFile = BLST_S_NEXT(hFile, node) )
    {
        BDBG_WRN(("File %p leaked on shutdown", (void *)hFile));
    }

    for ( pMem = BLST_S_FIRST(&hClient->memList); NULL != pMem; pMem = BLST_S_NEXT(pMem, node) )
    {
        BDBG_WRN(("Leaked shared memory allocation %p on shutdown", pMem->pMemory));
    }

    for ( pSecureMem = BLST_S_FIRST(&hClient->secureMemList); NULL != pSecureMem; pSecureMem = BLST_S_NEXT(pSecureMem, node) )
    {
        BDBG_WRN(("Leaked secure memory allocation " BDBG_UINT64_FMT " on shutdown", BDBG_UINT64_ARG(pSecureMem->address)));
    }

    /* Call instance close routine */
    hClient->hInstance->createSettings.ClientDestroy(hClient->pPrivateData);

    /* Cleanup */
    BLST_S_REMOVE(&hClient->hInstance->clientList, hClient, BTEE_Client, node);
    BDBG_OBJECT_DESTROY(hClient, BTEE_Client);
    BKNI_Free(hClient);
}

BERR_Code BTEE_Client_ReceiveMessage(
    BTEE_ClientHandle hClient,          /* Client Handle */
    BTEE_ConnectionHandle *pConnection, /* Connection that originated the message */
    void *pMessage,                     /* Pointer to buffer for received message */
    uint32_t maxMessageLength,            /* Length of message buffer in bytes */
    uint32_t *pMessageLength,             /* Returned message length in bytes */
    int timeoutMsec                     /* Timeout in msec.  Pass 0 for no timeout. */
    )
{
    BERR_Code rc;
    BTEE_ApplicationHandle hApp;
    BTEE_ConnectionHandle hConnection;
    void *pConnectionPrivate=NULL;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);
    BDBG_ASSERT(NULL != pConnection);
    BDBG_ASSERT(NULL != pMessage);
    BDBG_ASSERT(maxMessageLength > 0);
    BDBG_ASSERT(NULL != pMessageLength);

    rc = hClient->hInstance->createSettings.ClientReceiveMessage(hClient->pPrivateData, &pConnectionPrivate, pMessage, maxMessageLength, pMessageLength, timeoutMsec);
    if ( rc )
    {
        if ( rc != BERR_TIMEOUT )
        {
            return BERR_TRACE(rc);
        }

        /* Fail softly on timeout -> valid error case */
        return BERR_TIMEOUT;
    }

    *pConnection = NULL;

    /* Lookup connection handle */
    if ( pConnectionPrivate )
    {
        for ( hApp = BLST_S_FIRST(&hClient->appList); NULL != hApp; hApp = BLST_S_NEXT(hApp, node) )
        {
            for ( hConnection = BLST_S_FIRST(&hApp->connectionList); NULL != hConnection; hConnection = BLST_S_NEXT(hConnection, node) )
            {
                if ( pConnectionPrivate == hConnection->pPrivateData )
                {
                    *pConnection = hConnection;
                    break;
                }
            }
        }

        if ( NULL == *pConnection )
        {
            BDBG_WRN(("Unable to map received private connection handle %p to BTEE_Connection handle", pConnectionPrivate));
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BTEE_Client_AllocateMemory(
    BTEE_ClientHandle hClient,          /* Client Handle */
    uint32_t allocSize,                   /* Allocation size in bytes */
    void **pMemory                      /* [out] Allocated memory address */
    )
{
    BERR_Code rc = BERR_SUCCESS;
    BTEE_Memory *pMem;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);
    BDBG_ASSERT(NULL != pMemory);

    pMem = BKNI_Malloc(sizeof(BTEE_Memory));
    if ( NULL == pMem )
    {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    BKNI_Memset(pMem, 0, sizeof(BTEE_Memory));
    BDBG_OBJECT_SET(pMem, BTEE_Memory);
    pMem->length = allocSize;
    BLST_S_INSERT_HEAD(&hClient->memList, pMem, node);

    rc = hClient->hInstance->createSettings.ClientAlloc(hClient->pPrivateData, allocSize, &pMem->pMemory);
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto err_client_alloc;
    }

    *pMemory = pMem->pMemory;
    return BERR_SUCCESS;

err_client_alloc:
    BLST_S_REMOVE(&hClient->memList, pMem, BTEE_Memory, node);
    BDBG_OBJECT_DESTROY(pMem, BTEE_Memory);
    BKNI_Free(pMem);
err_malloc:
    return rc;
}

/***************************************************************************
Summary:
    Free Shared Memory
***************************************************************************/
void BTEE_Client_FreeMemory(
    BTEE_ClientHandle hClient,          /* Client Handle */
    void *pMemory                       /* Allocated memory address */
    )
{
    BTEE_Memory *pMem;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);
    BDBG_ASSERT(NULL != pMemory);

    /* Make sure we actually allocated this... */
    for ( pMem = BLST_S_FIRST(&hClient->memList); NULL != pMem; pMem = BLST_S_NEXT(pMem, node) )
    {
        if ( pMem->pMemory == pMemory )
        {
            break;
        }
    }

    if ( NULL == pMem )
    {
        BDBG_ERR(("Attempt to free shared memory block %p not allocated by this client (%p)", pMemory, (void *)hClient));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }

    hClient->hInstance->createSettings.ClientFree(hClient->pPrivateData, pMem->pMemory);

    BLST_S_REMOVE(&hClient->memList, pMem, BTEE_Memory, node);
    BDBG_OBJECT_DESTROY(pMem, BTEE_Memory);
    BKNI_Free(pMem);
}

/***************************************************************************
Summary:
    Allocate Secure Memory

Description:
    This allocates a secure region of memory that is not accessible by
    the host processor.  This can be used to contain decrypted executable
    images or other content that must be hidden from the host processor.
***************************************************************************/
BERR_Code BTEE_Client_AllocateSecureMemory(
    BTEE_ClientHandle hClient,          /* Client Handle */
    uint32_t allocSize,                   /* Allocation size in bytes */
    uint64_t *pAddress                  /* [out] Physical address */
    )
{
    BERR_Code rc = BERR_SUCCESS;
    BTEE_SecureMemory *pMem;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);
    BDBG_ASSERT(NULL != pAddress);

    pMem = BKNI_Malloc(sizeof(BTEE_SecureMemory));
    if ( NULL == pMem )
    {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    BKNI_Memset(pMem, 0, sizeof(BTEE_SecureMemory));
    BDBG_OBJECT_SET(pMem, BTEE_SecureMemory);
    pMem->length = allocSize;
    BLST_S_INSERT_HEAD(&hClient->secureMemList, pMem, node);

    rc = hClient->hInstance->createSettings.ClientSecureAlloc(hClient->pPrivateData, allocSize, &pMem->address);
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto err_client_alloc;
    }

    *pAddress = pMem->address;
    return BERR_SUCCESS;

err_client_alloc:
    BLST_S_REMOVE(&hClient->secureMemList, pMem, BTEE_SecureMemory, node);
    BDBG_OBJECT_DESTROY(pMem, BTEE_SecureMemory);
    BKNI_Free(pMem);
err_malloc:
    return rc;
}

/***************************************************************************
Summary:
    Free Secure Memory
***************************************************************************/
void BTEE_Client_FreeSecureMemory(
    BTEE_ClientHandle hClient,          /* Client Handle */
    uint64_t address                    /* Allocated secure address */
    )
{
    BTEE_SecureMemory *pMem;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);

    /* Make sure we actually allocated this... */
    for ( pMem = BLST_S_FIRST(&hClient->secureMemList); NULL != pMem; pMem = BLST_S_NEXT(pMem, node) )
    {
        if ( pMem->address == address )
        {
            break;
        }
    }

    if ( NULL == pMem )
    {
        BDBG_ERR(("Attempt to free secure memory block " BDBG_UINT64_FMT " not allocated by this client (%p)", BDBG_UINT64_ARG(address), (void *)hClient));
        (void)BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }

    hClient->hInstance->createSettings.ClientSecureFree(hClient->pPrivateData, pMem->address);

    BLST_S_REMOVE(&hClient->secureMemList, pMem, BTEE_SecureMemory, node);
    BDBG_OBJECT_DESTROY(pMem, BTEE_SecureMemory);
    BKNI_Free(pMem);
}

/***************************************************************************
Summary:
    Get Physical address for shared memory
***************************************************************************/
BERR_Code BTEE_Client_AddressToOffset(
    BTEE_ClientHandle hClient,          /* Client Handle */
    void *pMemory,                      /* Allocated virtual memory address */
    uint64_t *pOffset                   /* Physical address */
    )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);
    BDBG_ASSERT(NULL != pMemory);
    BDBG_ASSERT(NULL != pOffset);

    rc = hClient->hInstance->createSettings.ClientAddrToOffset(hClient->pPrivateData, pMemory, pOffset);
    if ( rc )
    {
        return BERR_TRACE(rc);
    }

    return rc;
}

/***************************************************************************
Summary:
    Get Physical address for shared memory
***************************************************************************/
BERR_Code BTEE_Client_OffsetToAddress(
    BTEE_ClientHandle hClient,          /* Client Handle */
    uint64_t offset,                    /* Physical address */
    void **pMemory                      /* [out] Virtual address */
    )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);
    BDBG_ASSERT(NULL != pMemory);

    rc = hClient->hInstance->createSettings.ClientOffsetToAddr(hClient->pPrivateData, offset, pMemory);
    if ( rc )
    {
        return BERR_TRACE(rc);
    }

    return rc;
}

/***************************************************************************
Summary:
    Switch execution contexts into trusted environment
***************************************************************************/
BERR_Code BTEE_Client_ContextSwitch(
    BTEE_ClientHandle hClient           /* Client Handle */
    )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);

    rc = hClient->hInstance->createSettings.ClientContextSwitch(hClient->pPrivateData);
    if ( rc )
    {
        return BERR_TRACE(rc);
    }

    return rc;
}

/***************************************************************************
Summary:
    Open a file visible to both execution environments
***************************************************************************/
BERR_Code BTEE_File_Open(
    BTEE_ClientHandle hClient,  /* Client Handle */
    const char *pPath,          /* Path to file */
    int flags,                  /* Standard file flags defined in fcntl.h */
    BTEE_FileHandle *pFile      /* [out] Returned file handle */
    )
{
    BERR_Code rc = BERR_SUCCESS;
    BTEE_File *hFile;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);
    BDBG_ASSERT(NULL != pPath);
    BDBG_ASSERT(NULL != pFile);

    hFile = BKNI_Malloc(sizeof(BTEE_File));
    if ( NULL == hFile )
    {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    BKNI_Memset(hFile, 0, sizeof(BTEE_File));
    BDBG_OBJECT_SET(hFile, BTEE_File);
    hFile->hClient = hClient;
    BLST_S_INSERT_HEAD(&hClient->fileList, hFile, node);

    rc = hClient->hInstance->createSettings.FileOpen(hClient->pPrivateData, pPath, flags, &hFile->pPrivateData);
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto err_open;
    }

    /* Success */
    *pFile = hFile;
    return BERR_SUCCESS;

err_open:
    BLST_S_REMOVE(&hClient->fileList, hFile, BTEE_File, node);
    BDBG_OBJECT_DESTROY(hFile, BTEE_File);
    BKNI_Free(hFile);
err_malloc:
    return rc;
}

/***************************************************************************
Summary:
    Read data from opened file
***************************************************************************/
BERR_Code BTEE_File_Read(
    BTEE_FileHandle hFile,      /* File Handle */
    uint64_t address,           /* Physical address of data buffer obtained from BTEE_AddressToOffset or BMEM_Heap_ConvertAddressToOffset */
    uint32_t bytesToRead,         /* Number of bytes to read */
    uint32_t *pBytesRead          /* Number of bytes actually read */
    )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hFile, BTEE_File);
    BDBG_ASSERT(bytesToRead > 0);
    BDBG_ASSERT(NULL != pBytesRead);

    rc = hFile->hClient->hInstance->createSettings.FileRead(hFile->pPrivateData, address, bytesToRead, pBytesRead);
    if ( rc )
    {
        return BERR_TRACE(rc);
    }

    return rc;
}

/***************************************************************************
Summary:
    Write data to opened file
***************************************************************************/
BERR_Code BTEE_File_Write(
    BTEE_FileHandle hFile,      /* File Handle */
    uint64_t address,           /* Physical address of data buffer obtained from BTEE_AddressToOffset or BMEM_Heap_ConvertAddressToOffset */
    uint32_t bytesToWrite,        /* Number of bytes to write */
    uint32_t *pBytesWritten       /* Number of bytes actually written */
    )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hFile, BTEE_File);
    BDBG_ASSERT(bytesToWrite > 0);
    BDBG_ASSERT(NULL != pBytesWritten);

    rc = hFile->hClient->hInstance->createSettings.FileWrite(hFile->pPrivateData, address, bytesToWrite, pBytesWritten);
    if ( rc )
    {
        return BERR_TRACE(rc);
    }

    return rc;
}

/***************************************************************************
Summary:
    Close file
***************************************************************************/
void BTEE_File_Close(
    BTEE_FileHandle hFile
    )
{
    BDBG_OBJECT_ASSERT(hFile, BTEE_File);

    hFile->hClient->hInstance->createSettings.FileClose(hFile->pPrivateData);

    BLST_S_REMOVE(&hFile->hClient->fileList, hFile, BTEE_File, node);
    BDBG_OBJECT_DESTROY(hFile, BTEE_File);
    BKNI_Free(hFile);
}

/***************************************************************************
Summary:
    Launch an application.  If the application is already running, an
    internal reference count will be updated.
***************************************************************************/
BERR_Code BTEE_Application_Open(
    BTEE_ClientHandle hClient,              /* Client Handle */
    const char *pName,                      /* Application Name */
    const char *pExecutablePath,            /* Path to executable image */
    BTEE_ApplicationHandle *pApplication    /* [out] Handle to launched application */
    )
{
    BERR_Code rc = BERR_SUCCESS;
    BTEE_Application *hApp;

    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);
    BDBG_ASSERT(NULL != pExecutablePath);
    BDBG_ASSERT(NULL != pApplication);

    hApp = BKNI_Malloc(sizeof(BTEE_Application));
    if ( NULL == hApp )
    {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    BKNI_Memset(hApp, 0, sizeof(BTEE_Application));
    BDBG_OBJECT_SET(hApp, BTEE_Application);
    hApp->hClient = hClient;
    BLST_S_INSERT_HEAD(&hClient->appList, hApp, node);

    rc = hClient->hInstance->createSettings.AppOpen(hClient->pPrivateData, pName, pExecutablePath, &hApp->pPrivateData);
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto err_open;
    }

    *pApplication = hApp;
    return BERR_SUCCESS;

err_open:
    BLST_S_REMOVE(&hClient->appList, hApp, BTEE_Application, node);
    BDBG_OBJECT_DESTROY(hApp, BTEE_Application);
    BKNI_Free(hApp);

err_malloc:
    return rc;
}

/***************************************************************************
Summary:
    Shutdown an application.  If the application has been started by more
    than one client an internal reference count will be updated.
***************************************************************************/
void BTEE_Application_Close(
    BTEE_ApplicationHandle hApp /* Application Handle */
    )
{
    BDBG_OBJECT_ASSERT(hApp, BTEE_Application);

    hApp->hClient->hInstance->createSettings.AppClose(hApp->pPrivateData);

    BLST_S_REMOVE(&hApp->hClient->appList, hApp, BTEE_Application, node);
    BDBG_OBJECT_DESTROY(hApp, BTEE_Application);
    BKNI_Free(hApp);
}

/***************************************************************************
Summary:
    Get Default Connection Settings
***************************************************************************/
void BTEE_Connection_GetDefaultSettings(
    BTEE_ClientHandle hClient,
    BTEE_ConnectionSettings *pSettings  /* [out] */
    )
{
    BDBG_OBJECT_ASSERT(hClient, BTEE_Client);
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(BTEE_ConnectionSettings));
}

/***************************************************************************
Summary:
    Connect to an application service by name
***************************************************************************/
BERR_Code BTEE_Connection_Open(
    BTEE_ApplicationHandle hApplication,
    const char *pServiceName,
    const BTEE_ConnectionSettings *pSettings,   /* */
    BTEE_ConnectionHandle *pConnection      /* [out] Connection Handle */
    )
{
    BERR_Code rc = BERR_SUCCESS;
    BTEE_ConnectionHandle hConnection;
    BTEE_ConnectionSettings defaults;

    BDBG_OBJECT_ASSERT(hApplication, BTEE_Application);
    BDBG_ASSERT(NULL != pServiceName);
    BDBG_ASSERT(NULL != pConnection);

    if ( NULL == pSettings )
    {
        BTEE_Connection_GetDefaultSettings(hApplication->hClient, &defaults);
        pSettings = &defaults;
    }

    hConnection = BKNI_Malloc(sizeof(BTEE_Connection));
    if ( NULL == hConnection )
    {
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto err_malloc;
    }

    BKNI_Memset(hConnection, 0, sizeof(BTEE_Connection));
    BDBG_OBJECT_SET(hConnection, BTEE_Connection);
    hConnection->hApp = hApplication;
    BLST_S_INSERT_HEAD(&hApplication->connectionList, hConnection, node);

    rc = hApplication->hClient->hInstance->createSettings.ConnectionOpen(hApplication->pPrivateData, pServiceName, pSettings, &hConnection->pPrivateData);
    if ( rc )
    {
        rc = BERR_TRACE(rc);
        goto err_open;
    }

    *pConnection = hConnection;
    return BERR_SUCCESS;

err_open:
    BLST_S_REMOVE(&hApplication->connectionList, hConnection, BTEE_Connection, node);
    BDBG_OBJECT_DESTROY(hConnection, BTEE_Connection);
    BKNI_Free(hConnection);
err_malloc:
    return rc;
}

/***************************************************************************
Summary:
    Send a message to a connected service
***************************************************************************/
BERR_Code BTEE_Connection_SendMessage(
    BTEE_ConnectionHandle hConnection,
    const void *pMessage,
    uint32_t messageLength
    )
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hConnection, BTEE_Connection);
    BDBG_ASSERT(NULL != pMessage);
    BDBG_ASSERT(messageLength > 0);

    rc = hConnection->hApp->hClient->hInstance->createSettings.ConnectionSendMessage(hConnection->pPrivateData, pMessage, messageLength);
    if ( rc )
    {
        return BERR_TRACE(rc);
    }

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Close a connection
***************************************************************************/
void BTEE_Connection_Close(
    BTEE_ConnectionHandle hConnection
    )
{
    BDBG_OBJECT_ASSERT(hConnection, BTEE_Connection);

    hConnection->hApp->hClient->hInstance->createSettings.ConnectionClose(hConnection->pPrivateData);

    BLST_S_REMOVE(&hConnection->hApp->connectionList, hConnection, BTEE_Connection, node);
    BDBG_OBJECT_DESTROY(hConnection, BTEE_Connection);
    BKNI_Free(hConnection);
}

void BTEE_Instance_GetDefaultCreateSettings(
    BTEE_InstanceCreateSettings *pSettings /* [out] default settings */
    )
{
    BDBG_ASSERT(NULL != pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/***************************************************************************
Summary:
    Create a TEE instance
***************************************************************************/
BERR_Code BTEE_Instance_Create(
    const BTEE_InstanceCreateSettings *pSettings,
    BTEE_InstanceHandle *pInstance /* [out] handle to instance */
    )
{
    BTEE_Instance *hInstance;

    BDBG_ASSERT(NULL != pSettings);
    BDBG_ASSERT(NULL != pInstance);

    /* Sanity check callbacks exist once when creating instance to avoid checking later */
    if ( NULL == pSettings->ClientCreate ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ClientDestroy ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ClientReceiveMessage ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ClientAlloc ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ClientFree ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ClientSecureAlloc ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ClientSecureFree ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ClientAddrToOffset ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ClientOffsetToAddr ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ClientContextSwitch ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->FileOpen ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->FileRead ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->FileWrite ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->FileClose ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->AppOpen ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->AppClose ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ConnectionOpen ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ConnectionSendMessage ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->ConnectionClose ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if ( NULL == pSettings->GetStatus ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    hInstance = BKNI_Malloc(sizeof(BTEE_Instance));
    if ( NULL == hInstance )
    {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    BKNI_Memset(hInstance, 0, sizeof(BTEE_Instance));
    BDBG_OBJECT_SET(hInstance, BTEE_Instance);
    hInstance->createSettings = *pSettings;
    *pInstance = hInstance;

    return BERR_SUCCESS;
}

/***************************************************************************
Summary:
    Destroy a TEE instance
***************************************************************************/
void BTEE_Instance_Destroy(
    BTEE_InstanceHandle hInstance
    )
{
    BTEE_ClientHandle hClient;

    BDBG_OBJECT_ASSERT(hInstance, BTEE_Instance);

    for ( hClient = BLST_S_FIRST(&hInstance->clientList); NULL != hClient; hClient = BLST_S_NEXT(hClient, node) )
    {
        BDBG_WRN(("Leaked client %p on instance shutdown", (void *)hClient));
    }

    BDBG_OBJECT_DESTROY(hInstance, BTEE_Instance);
    BKNI_Free(hInstance);
}

/***************************************************************************
Summary:
    Get TEE Instance Status
***************************************************************************/
BERR_Code BTEE_Instance_GetStatus(
    BTEE_InstanceHandle hInstance,
    BTEE_InstanceStatus *pStatus
    )
{
    BDBG_OBJECT_ASSERT(hInstance, BTEE_Instance);

    if ( pStatus == NULL )
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return hInstance->createSettings.GetStatus(pStatus);
}
