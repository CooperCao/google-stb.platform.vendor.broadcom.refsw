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
*   API name: Base
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#ifndef NEXUS_BASE_TYPES_H
#define NEXUS_BASE_TYPES_H

#include "bstd_defs.h"
#include "bstd_file.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
nexus_base_types.h provides data types and macros for Base.
It does not provide Base API's.
**/

/**
Summary:
NEXUS_Error is the common return type for Nexus Interfaces.
Any non-zero NEXUS_Error value is a failure. Zero is success.

Description:
An Interface may return one of the standard codes defined below.
It may build a custom code using NEXUS_MAKE_ERR_CODE.
It may propogate an internal non-zero error code whose value is undefined.
**/
typedef unsigned NEXUS_Error;

/**
Summary:
Standard Nexus error codes.
**/
#define NEXUS_SUCCESS              0  /* success (always zero) */
#define NEXUS_NOT_INITIALIZED      1  /* parameter not initialized */
#define NEXUS_INVALID_PARAMETER    2  /* parameter is invalid */
#define NEXUS_OUT_OF_SYSTEM_MEMORY 3  /* out of KNI module memory (aka OS memory) */
#define NEXUS_OUT_OF_DEVICE_MEMORY 4  /* out of MEM module memory (ala heap memory) */
#define NEXUS_TIMEOUT              5  /* reached timeout limit */
#define NEXUS_OS_ERROR             6  /* generic OS error */
#define NEXUS_LEAKED_RESOURCE      7  /* resource being freed has attached resources that haven't been freed */
#define NEXUS_NOT_SUPPORTED        8  /* requested feature is not supported */
#define NEXUS_UNKNOWN              9  /* unknown */
#define NEXUS_NOT_AVAILABLE        10 /* no resource available */

/* These are private macros used by the NEXUS ERR macros below. */
#define NEXUS_P_ERR_ID_MASK   UINT32_C(0xFFFF0000)   /* {private} */
#define NEXUS_P_ERR_ID_SHIFT  16                    /* {private} */
#define NEXUS_P_ERR_NUM_MASK  UINT32_C(0x0000FFFF)  /* {private} */

/**
Summary:
Interfaces can use NEXUS_MAKE_ERR_CODE to build custom error codes.
There is no central repository for ID's in the code.
You can grep the nexus header files for NEXUS_MAKE_ERR_CODE to find existing values.
Please talk with the Nexus architecture team before making use of a new ID.
**/
#define NEXUS_MAKE_ERR_CODE(id, num) \
    (((((NEXUS_Error)(id)) << NEXUS_P_ERR_ID_SHIFT) & NEXUS_P_ERR_ID_MASK) | \
     (((NEXUS_Error)(num)) & NEXUS_P_ERR_NUM_MASK))

#define NEXUS_GET_ERR_NUM(code) \
    (((NEXUS_Error)(code)) & NEXUS_P_ERR_NUM_MASK)

#define NEXUS_GET_ERR_ID(code) \
    ((((NEXUS_Error)(code)) & NEXUS_P_ERR_ID_MASK) >> NEXUS_P_ERR_ID_SHIFT)


/*
Summary:
Every Nexus module is identified by one handle.

Description:
This handle is access by the Platform or application.
It is not used in the module's Interfaces.
*/
typedef struct NEXUS_Module *NEXUS_ModuleHandle;

/**
Summary:
Function prototype used by NEXUS_CallbackDesc.
**/
typedef void (*NEXUS_Callback)(void *context, int param);

/*
Summary:
Standard definition of a callback in Nexus.
*/
typedef struct NEXUS_CallbackDesc {
    NEXUS_Callback callback; /* Function pointer */
    void *context;           /* First parameter to callback function. */
    int param;               /* Second parameter to callback function. */
} NEXUS_CallbackDesc;

#define NEXUS_CALLBACKDESC_INITIALIZER() {NULL, NULL, 0}
#define NEXUS_CALLBACKDESC_INIT(cb) do {(cb)->callback = NULL;(cb)->context=NULL;(cb)->param=0;}while(0)

/**
Summary:
Indicates the priority in which events, timers and callbacks from this module will be serviced relative to other modules.

Description:
Inside base, separate threads are created for each priority level.
Call events, timers and callbacks for a given priority will be serviced in FIFO order.
Specifying the correct priority is critical to avoid having a fast interface waiting on a slow interface.

Module priorities do not necessarily map to thread priorities. See Nexus_Architecture for a discussion of thread priorities in Nexus Base.

See Also:
NEXUS_ModuleSettings
**/
typedef enum NEXUS_ModulePriority
{
    NEXUS_ModulePriority_eIdle,    /* Very low priority. This should be used by modules with blocking functions or any work that has minimal performance requirements. */
    NEXUS_ModulePriority_eLow,     /* Low priority. This should be used by slow-executing modules. */
    NEXUS_ModulePriority_eDefault, /* Medium priority. This is used by most modules. */
    NEXUS_ModulePriority_eHigh,    /* The highest priority. This should only be used by fast-executing modules (no slow functions) which require it. */
    NEXUS_ModulePriority_eIdleActiveStandby,    /* Very low priority. This should be used by modules with blocking functions or any work that has minimal performance requirements, but remain to be active during active standby */
    NEXUS_ModulePriority_eLowActiveStandby,     /* Low priority. This should be used by slow-executing modules, but remain to be active during active standby */
    NEXUS_ModulePriority_eDefaultActiveStandby, /* Medium priority. This is used by most modules, but remain to be active during active standby */
    NEXUS_ModulePriority_eHighActiveStandby,    /* The highest priority. This should only be used by fast-executing modules (no slow functions) which require it, and remain active during active standby */
    NEXUS_ModulePriority_eMax
} NEXUS_ModulePriority;

/**
Summary:
Settings used to create a Nexus module

Description:

See Also:
NEXUS_Module_GetDefaultSettings
NEXUS_Module_Create
**/
typedef struct NEXUS_ModuleSettings
{
    NEXUS_ModulePriority priority;  /* Default=NEXUS_ModulePriority_Default */    
    void (*dbgPrint)(void);         /* optional pointer debug callback, this callback should not directly or indirectly call into the nexus Base module */
    const char *dbgModules;         /* BDBG_MODULE or BDBG_FILE_MODULE that will produce the debug output */
    bool passthroughCallbacks;      /* if module supports passthrough callbacks, then NEXUS_CallbackHandler test will be bypassed */
} NEXUS_ModuleSettings;

/**
Summary:
General purpose ID used for dynamic allocation of resources.

Description:
For instance, NEXUS_Playpump_Open(NEXUS_ANY_ID, NULL) will open any available playpump index.
To learn what index was opened, use the interface's GetStatus function.
**/
#define NEXUS_ANY_ID 0xFEFEFEFE

/**
Summary:
Add to ID to open alias of existing resource

Description:
For instance, NEXUS_HdDviInput_Open(NEXUS_ALIAS_ID+0, NULL) will open an alias to an existing NEXUS_HdDviInput_Open(0)
which was opened on the server.
This can only be used if the server's only action is Open, Close and GetConnector.
All settings, callbacks and status are exclusively available to the alias.
**/
#define NEXUS_ALIAS_ID 10000

/**
Summary:
Standard definition of a tristate in Nexus.
**/
typedef enum NEXUS_TristateEnable {
    NEXUS_TristateEnable_eDisable, /* backward compat with boolean false */
    NEXUS_TristateEnable_eEnable,  /* backward compat with boolean true */
    NEXUS_TristateEnable_eNotSet,  /* this is the tristate */
    NEXUS_TristateEnable_eMax
} NEXUS_TristateEnable;

/***************************************************************************
Summary:
Heap handle

Description:
This is used to manage custom memory configurations.
A NEXUS_HeapHandle is created by specifying custom heap options in NEXUS_PlatformSettings.
***************************************************************************/
typedef struct NEXUS_Heap *NEXUS_HeapHandle;

/***************************************************************************
ummary:
NEXUS_ClientResources allows server to limit resources available to untrusted clients

Resources are set by the server in two ways:
1. using NEXUS_ClientSettings.configuration when calling NEXUS_Platform_RegisterClient
2. using NEXUS_PlatformStartServerSettings.unauthenticatedConfiguration when calling NEXUS_Platform_StartServer

Resources are enforced in each module using nexus_client_resources.h macros

Resources can be read by client using NEXUS_Platform_GetClientConfiguration
***************************************************************************/
#define NEXUS_MAX_IDS 16
typedef struct NEXUS_ClientResourceIdList
{
    unsigned id[NEXUS_MAX_IDS]; /* packed array of 'total' elements. */
    unsigned total; /* count of elements in id[] */
} NEXUS_ClientResourceIdList;
typedef struct NEXUS_ClientResourceCount
{
    unsigned total; /* count of resources */
} NEXUS_ClientResourceCount;

typedef struct NEXUS_ClientResources
{
    NEXUS_ClientResourceIdList simpleAudioDecoder;
    NEXUS_ClientResourceIdList simpleVideoDecoder;
    NEXUS_ClientResourceIdList simpleEncoder;
    NEXUS_ClientResourceIdList surfaceClient;
    NEXUS_ClientResourceIdList inputClient;
    NEXUS_ClientResourceIdList audioCapture;
    NEXUS_ClientResourceIdList audioCrc;

    NEXUS_ClientResourceCount dma;
    NEXUS_ClientResourceCount graphics2d;
    NEXUS_ClientResourceCount graphicsv3d;
    NEXUS_ClientResourceCount pictureDecoder;
    NEXUS_ClientResourceCount playpump;
    NEXUS_ClientResourceCount recpump;
    NEXUS_ClientResourceCount simpleAudioPlayback;
    NEXUS_ClientResourceCount simpleStcChannel;
    NEXUS_ClientResourceCount surface;
    struct {
        unsigned sizeLimit;
    } temporaryMemory; /* memory that is allocated for duration of call to hold temporary data */
} NEXUS_ClientResources;

/**
Summary:
Client modes

See nexus/docs/Nexus_MultiProcess.pdf for full discussion of process isolation and multi-process application design.
**/
typedef enum NEXUS_ClientMode
{
    NEXUS_ClientMode_eUnprotected, /* deprecated */
    NEXUS_ClientMode_eVerified,    /* verify handle value, but not owner. unsynchronized caller may compromise nexus settings. */
    NEXUS_ClientMode_eProtected,   /* full handle verification. access to full API. if client crashes, server is protected. */
    NEXUS_ClientMode_eUntrusted,   /* full handle verification. access to limited API. see nexus/build/common/tools/nexus_untrusted_api.txt. if client crashes, server is protected. */
    NEXUS_ClientMode_eMax
} NEXUS_ClientMode;

/* NEXUS_MAX_HEAPS is the maximum number of heaps in the system, pointed to be NEXUS_HeapHandle.
A heap is required for any memory access, whether by HW or SW.
This depends on both HW capabilities & SW configuration. */
#define NEXUS_MAX_HEAPS 16

/* NEXUS_MAX_MEMC is the maximum number of memory controllers in the system. */
#define NEXUS_MAX_MEMC 3

/**
Summary:
Information provided by the server to the client
**/
typedef struct NEXUS_ClientConfiguration
{
    NEXUS_ClientMode mode; /* default is eProtected. */
    NEXUS_HeapHandle heap[NEXUS_MAX_HEAPS]; /* untrusted client will be restricted to these heaps. heap[0] will be its default. */
    NEXUS_ClientResources resources; /* resources granted by the server for untrusted clients */
} NEXUS_ClientConfiguration;

/***************************************************************************
Summary:
Type that is used for physical address
****************************************************************************/
typedef uint64_t NEXUS_Addr;


#ifdef __cplusplus
}
#endif

#endif /* NEXUS_BASE_TYPES_H */

