/***************************************************************************
*  Copyright (C) 2004-2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
***************************************************************************/
#ifndef NEXUS_BASE_TYPES_H
#define NEXUS_BASE_TYPES_H

#include "bstd_defs.h"
#include "bstd_file.h"
#include "nexus_base_types_client.h"

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
It may propagate an internal non-zero error code whose value is undefined.
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
#define NEXUS_P_CALLBACK_COOKIE unsigned private_cookie

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

    NEXUS_P_CALLBACK_COOKIE; /* private */
} NEXUS_CallbackDesc;

/**
Summary:
Initialize NEXUS_CallbackDesc structure
**/
void NEXUS_CallbackDesc_Init_isrsafe(
    NEXUS_CallbackDesc *desc /* [out] */
    );


NEXUS_CallbackDesc NEXUS_P_CallbackDescByValue_isrsafe(void);
#define NEXUS_CALLBACKDESC_INITIALIZER() NEXUS_P_CallbackDescByValue_isrsafe()
#define NEXUS_CALLBACKDESC_INIT(cb) NEXUS_CallbackDesc_Init_isrsafe(cb)
#define NEXUS_CallbackDesc_Init NEXUS_CallbackDesc_Init_isrsafe
#define NEXUS_P_CallbackDescByValue NEXUS_P_CallbackDescByValue_isrsafe

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
    NEXUS_ModulePriority_eAlwaysOn,    /* This should be used by modules that remain On during all standby */
    NEXUS_ModulePriority_eCallbackLow, /* scheduler used to route callbacks from the Idle and Low priority modules to application */
    NEXUS_ModulePriority_eCallbackHigh, /* scheduler used to route callbacks from the Default and High priority modules to application */
    NEXUS_ModulePriority_eCallbackLowActiveStandby, /* scheduler used to route callbacks from the Idle and Low priority modules that remain On during all standby to application */
    NEXUS_ModulePriority_eCallbackHighActiveStandby, /* scheduler used to route callbacks from the Default and High priority modules that remain On during all standby to application */
    NEXUS_ModulePriority_eCallbackAlwaysOn, /* scheduler used to route callbacks from the modules that remain On during all standby */
    NEXUS_ModulePriority_eInternal, /* Internal scheduler. This should only be internally to NEXUS and should not be used to route callbacks to application, and remain active during active standby */
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
    NEXUS_TristateEnable_eAuto,  /* this is the tristate */
    NEXUS_TristateEnable_eNotSet=NEXUS_TristateEnable_eAuto, /* backward compatible */
    NEXUS_TristateEnable_eMax
} NEXUS_TristateEnable;


/***************************************************************************
Summary:
Type that is used for physical address
****************************************************************************/
typedef uint64_t NEXUS_Addr;

/***************************************************************************
Summary:
Type that could represent any NEXUS object
****************************************************************************/
typedef void *NEXUS_AnyObject;

/***************************************************************************
Summary:
Type that could represent address in the application address space
****************************************************************************/
typedef void *NEXUS_P_MemoryUserAddr;

/***************************************************************************
Summary:
Unique ID assigned to nexus object
****************************************************************************/
typedef unsigned NEXUS_BaseObjectId;

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_BASE_TYPES_H */

