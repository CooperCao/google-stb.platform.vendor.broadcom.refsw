/***************************************************************************
 *     Copyright (c) 2016, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************/
#ifndef BTEE_H__
#define BTEE_H__

/*================== Module Overview =====================================
This module defines an interface to a trusted execution environment

The system will regieter one or more instance handles to define a trusted
execution environment instance.  Any magnum module that wants to use services
from the trusted environment will then create a client handle from that
instance handle.

Using the client handle, you can download an executable file and start an
application from the downloaded file.  The application will then register
one or more service names that you can create a connection to for IPC
purposes.

========================================================================*/

/***************************************************************************
Summary:
    Handle for client calls into BTEE
***************************************************************************/
typedef struct BTEE_Instance *BTEE_InstanceHandle;

/***************************************************************************
Summary:
    Client Handle
***************************************************************************/
typedef struct BTEE_Client *BTEE_ClientHandle;

/***************************************************************************
Summary:
    Application Handle
***************************************************************************/
typedef struct BTEE_Application *BTEE_ApplicationHandle;

/***************************************************************************
Summary:
    Connection Handle
***************************************************************************/
typedef struct BTEE_Connection *BTEE_ConnectionHandle;

/***************************************************************************
Summary:
    File Handle
***************************************************************************/
typedef struct BTEE_File *BTEE_FileHandle;

/***************************************************************************
Summary:
    Client Event Types
***************************************************************************/
typedef enum BTEE_ClientEvent
{
    BTEE_ClientEvent_eMessageReceived,
    BTEE_ClientEvent_eApplicationExit,
    BTEE_ClientEvent_eMax
} BTEE_ClientEvent;

/***************************************************************************
Summary:
    Client Event Callback
***************************************************************************/
typedef void (*BTEE_ClientEventCallback)(BTEE_ClientEvent event, void *pPrivateData);

/***************************************************************************
Summary:
    Client Create Settings
***************************************************************************/
typedef struct BTEE_ClientCreateSettings
{
    BTEE_ClientEventCallback pEventCallback_isr;    /* Event Callback Handler.  This will execute in magnum interrupt context. */
    void *pCallbackData;                            /* Private data passed to event callback */
} BTEE_ClientCreateSettings;

/***************************************************************************
Summary:
    Get Default Client Create Settings
***************************************************************************/
void BTEE_Client_GetDefaultCreateSettings(
    BTEE_InstanceHandle hInstance,       /* Instance Handle */
    BTEE_ClientCreateSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
    Create a client handle
***************************************************************************/
BERR_Code BTEE_Client_Create(
    BTEE_InstanceHandle hInstance,                  /* Instance Handle */
    const char *pName,                              /* Client Name */
    const BTEE_ClientCreateSettings *pSettings,     /* Client Settings */
    BTEE_ClientHandle *pClient                      /* [out] */
    );

/***************************************************************************
Summary:
    Destroy a client handle
***************************************************************************/
void BTEE_Client_Destroy(
    BTEE_ClientHandle hClient
    );

/***************************************************************************
Summary:
    Retrieve a message
***************************************************************************/
BERR_Code BTEE_Client_ReceiveMessage(
    BTEE_ClientHandle hClient,          /* Client Handle */
    BTEE_ConnectionHandle *pConnection, /* Connection that originated the message */
    void *pMessage,                     /* Pointer to buffer for received message */
    size_t maxMessageLength,            /* Length of message buffer in bytes */
    size_t *pMessageLength,             /* Returned message length in bytes */
    int timeoutMsec                     /* Timeout in msec.  Pass 0 for no timeout. */
    );

/***************************************************************************
Summary:
    Allocate Shared Memory
***************************************************************************/
BERR_Code BTEE_Client_AllocateMemory(
    BTEE_ClientHandle hClient,          /* Client Handle */
    size_t allocSize,                   /* Allocation size in bytes */
    void **pMemory                      /* [out] Allocated memory address */
    );

/***************************************************************************
Summary:
    Free Shared Memory
***************************************************************************/
void BTEE_Client_FreeMemory(
    BTEE_ClientHandle hClient,          /* Client Handle */
    void *pMemory                       /* Allocated memory address */
    );

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
    size_t allocSize,                   /* Allocation size in bytes */
    uint32_t *pAddress                  /* [out] Physical address */
    );

/***************************************************************************
Summary:
    Free Secure Memory
***************************************************************************/
void BTEE_Client_FreeSecureMemory(
    BTEE_ClientHandle hClient,          /* Client Handle */
    uint32_t address                    /* Allocated secure address */
    );

/***************************************************************************
Summary:
    Get Physical address for shared memory
***************************************************************************/
BERR_Code BTEE_Client_AddressToOffset(
    BTEE_ClientHandle hClient,          /* Client Handle */
    void *pMemory,                      /* Allocated virtual memory address */
    uint32_t *pOffset                   /* Physical address */
    );

/***************************************************************************
Summary:
    Get Physical address for shared memory
***************************************************************************/
BERR_Code BTEE_Client_OffsetToAddress(
    BTEE_ClientHandle hClient,          /* Client Handle */
    uint32_t offset,                    /* Physical address */
    void **pMemory                      /* [out] Virtual address */
    );

/***************************************************************************
Summary:
    Switch execution contexts into trusted environment
***************************************************************************/
BERR_Code BTEE_Client_ContextSwitch(
    BTEE_ClientHandle hClient           /* Client Handle */
    );

/***************************************************************************
Summary:
    Open a file visible to both execution environments
***************************************************************************/
BERR_Code BTEE_File_Open(
    BTEE_ClientHandle hClient,  /* Client Handle */
    const char *pPath,          /* Path to file */
    int flags,                  /* Standard file flags defined in fcntl.h */
    BTEE_FileHandle *pFile      /* [out] Returned file handle */
    );

/***************************************************************************
Summary:
    Read data from opened file
***************************************************************************/
BERR_Code BTEE_File_Read(
    BTEE_FileHandle hFile,      /* File Handle */
    uint32_t address,           /* Physical address of data buffer obtained from BTEE_AddressToOffset or BMEM_Heap_ConvertAddressToOffset */
    size_t bytesToRead,         /* Number of bytes to read */
    size_t *pBytesRead          /* Number of bytes actually read */
    );

/***************************************************************************
Summary:
    Write data to opened file
***************************************************************************/
BERR_Code BTEE_File_Write(
    BTEE_FileHandle hFile,      /* File Handle */
    uint32_t address,           /* Physical address of data buffer obtained from BTEE_AddressToOffset or BMEM_Heap_ConvertAddressToOffset */
    size_t bytesToWrite,        /* Number of bytes to write */
    size_t *pBytesWritten       /* Number of bytes actually written */
    );

/***************************************************************************
Summary:
    Close file
***************************************************************************/
void BTEE_File_Close(
    BTEE_FileHandle hFile
    );

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
    );

/***************************************************************************
Summary:
    Shutdown an application.  If the application has been started by more
    than one client an internal reference count will be updated.
***************************************************************************/
void BTEE_Application_Close(
    BTEE_ApplicationHandle hApp /* Application Handle */
    );

/***************************************************************************
Summary:
    Connection Settings
***************************************************************************/
typedef struct BTEE_ConnectionSettings
{
    int tbd;        /* Placeholder for future expansion */
} BTEE_ConnectionSettings;

/***************************************************************************
Summary:
    Get Default Connection Settings
***************************************************************************/
void BTEE_Connection_GetDefaultSettings(
    BTEE_ClientHandle hClient,
    BTEE_ConnectionSettings *pSettings  /* [out] */
    );

/***************************************************************************
Summary:
    Connect to an application service by name
***************************************************************************/
BERR_Code BTEE_Connection_Open(
    BTEE_ApplicationHandle hApplication,
    const char *pServiceName,
    const BTEE_ConnectionSettings *pSettings,   /* */
    BTEE_ConnectionHandle *pConnection      /* [out] Connection Handle */
    );

/***************************************************************************
Summary:
    Send a message to a connected service
***************************************************************************/
BERR_Code BTEE_Connection_SendMessage(
    BTEE_ConnectionHandle hConnection,
    const void *pMessage,
    size_t messageLength
    );

/***************************************************************************
Summary:
    Close a connection
***************************************************************************/
void BTEE_Connection_Close(
    BTEE_ConnectionHandle hConnection
    );

#endif /* BTEE_H__ */
