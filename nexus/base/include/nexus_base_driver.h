/***************************************************************************
*  Copyright (C) 2017-2018 Broadcom.  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
***************************************************************************/
#ifndef NEXUS_BASE_DRIVER_H__
#define NEXUS_BASE_DRIVER_H__

#include "nexus_base_types.h"
#include "nexus_base_debug.h"

#ifdef __cplusplus
#error Internal header file cannot be built by C++ compiler
#endif

/**
Base API's which need to be called from platform/driver code which doesn't need or have access to nexus_base.h
**/

/**
Summary:
Macro to lock a module with debug tagging.

Description:

See Also:
NEXUS_LockModule
**/
#if NEXUS_P_DEBUG_MODULE_LOCKS
#define NEXUS_Module_Lock(module) NEXUS_Module_Lock_Tagged((module), BSTD_FILE, BSTD_LINE)
#else
#define NEXUS_Module_Lock(module) NEXUS_Module_Lock_Tagged((module), NULL, 0)
#endif

/**
Summary:
Macro to try to lock a module with debug tagging.

Description:

See Also:
NEXUS_TryLockModule
**/
#if NEXUS_P_DEBUG_MODULE_LOCKS
#define NEXUS_Module_TryLock(module) NEXUS_Module_TryLock_Tagged((module), BSTD_FILE, BSTD_LINE)
#else
#define NEXUS_Module_TryLock(module) NEXUS_Module_TryLock_Tagged((module), NULL, 0)
#endif

/**
Summary:
Macro to unlock a module with debug tagging.

Description:

See Also:
NEXUS_UnlockModule
**/
#if NEXUS_P_DEBUG_MODULE_LOCKS
#define NEXUS_Module_Unlock(module) NEXUS_Module_Unlock_Tagged((module), BSTD_FILE, BSTD_LINE)
#else
#define NEXUS_Module_Unlock(module) NEXUS_Module_Unlock_Tagged((module), NULL, 0)
#endif

/**
Summary:
Actual function called by NEXUS_LockModule macro.

Description:

See Also:
NEXUS_LockModule
**/
void NEXUS_Module_Lock_Tagged(
    NEXUS_ModuleHandle module,
    const char *pFileName,
    unsigned lineNumber
    );

/**
Summary:
Actual function called by NEXUS_TryLockModule macro.

Description:

See Also:
NEXUS_TryLockModule
**/
bool NEXUS_Module_TryLock_Tagged(
    NEXUS_ModuleHandle module,
    const char *pFileName,
    unsigned lineNumber
    );

/**
Summary:
Actual function called by NEXUS_UnlockModule macro.

Description:

See Also:
NEXUS_UnlockModule
**/
void NEXUS_Module_Unlock_Tagged(
    NEXUS_ModuleHandle module,
    const char *pFileName,
    unsigned lineNumber
    );

void NEXUS_Module_GetPriority(
    NEXUS_ModuleHandle module,
    NEXUS_ModulePriority *pPriority
    );

void NEXUS_Module_GetCallbackPriority(
    NEXUS_ModuleHandle module,
    NEXUS_ModulePriority *pPriority
    );

/**
Summary:
Returns true if given module could be left active during active standby 
**/
bool NEXUS_Module_ActiveStandyCompatible(
    NEXUS_ModuleHandle module
    );


/**
Summary:
Macro to enable a module with debug tagging.

Description:

See Also:
**/
#if NEXUS_P_DEBUG_MODULE_LOCKS
#define NEXUS_Module_Enable(module) NEXUS_Module_Enable_Tagged((module), BSTD_FILE, BSTD_LINE)
#else
#define NEXUS_Module_Enable(module) NEXUS_Module_Enable_Tagged((module), NULL, 0)
#endif

/**
Summary:
Macro to disable a module with debug tagging.

Description:

See Also:
**/
#if NEXUS_P_DEBUG_MODULE_LOCKS
#define NEXUS_Module_Disable(module) NEXUS_Module_Disable_Tagged((module), BSTD_FILE, BSTD_LINE)
#else
#define NEXUS_Module_Disable(module) NEXUS_Module_Disable_Tagged((module), NULL, 0)
#endif

/**
Summary:
Actual function called by NEXUS_Module_Enable macro.

Description:

See Also:
**/
void NEXUS_Module_Enable_Tagged(
    NEXUS_ModuleHandle module,
    const char *pFileName,
    unsigned lineNumber
    );

/**
Summary:
Actual function called by NEXUS_Module_Disable macro.

Description:

See Also:
**/
void NEXUS_Module_Disable_Tagged(
    NEXUS_ModuleHandle module,
    const char *pFileName,
    unsigned lineNumber
    );

/* local NEXUS_StopCallbacks/NEXUS_StartCallbacks calls into platform code where
it can be routed to client and server base modules. 
these functions cannot call into core module. */

void NEXUS_Platform_P_StopCallbacks(void *interfaceHandle);
void NEXUS_Platform_P_StartCallbacks(void *interfaceHandle);

#if NEXUS_TRACK_STOP_CALLBACKS
void NEXUS_Platform_P_StopCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName);
void NEXUS_Platform_P_StartCallbacks_tagged(void *interfaceHandle, const char *pFileName, unsigned lineNumber, const char *pFunctionName);
#endif

#endif /* !defined NEXUS_BASE_DRIVER_H__ */

