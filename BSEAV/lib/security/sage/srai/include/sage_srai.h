/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
/******************************************************************************
 * Module : SAGE - Sage Remote Application Interface (SRAI)
 *          API declaration.
 * 
 * Module Description:
 *          SAGE remote Application Interface (SRAI) is a helper library which allows communication between
 *          Common DRM Application and its counterpart running on the SAGE system.
 *          It provides a platform agnostic method for Common DRM Application to communicate with SAGE from
 *          the Host SoC, in order to trigger routines (process commands).
 *          From the Host, a Common DRM modules remotely manage SAGE in order to initialize platforms,
 *          modules and process module commands.
 * 
 *****************************************************************************/


#ifndef _SAGE_SRAI_H_
#define _SAGE_SRAI_H_

#include "bsagelib_types.h"
#include "bsagelib_tools.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************
 * Type definitions
 **/


/*
 * Opaque Handles, to refer to used instances
 */

/* Platform Handles:
 * retrieved using SRAI_Platform_Open()
 * used in all sub calls to refer to the opened platform
 * ( SRAI_Platform_Close(), SRAI_Platform_Init(), SRAI_Module_Init() )*/
typedef struct SRAI_Platform *SRAI_PlatformHandle;
/* Module Handles:
 * retrieved using SRAI_Module_Init()
 * used in all sub calls to refer to the module instance
 * ( SRAI_Module_Uninit(), SRAI_Module_ProcessCommand() )*/
typedef struct SRAI_Module *SRAI_ModuleHandle;

/*
 * Memory types
 *
 * bitwize mask of flags used to differentiate
 * shared memory blocks (accessible of the host and on SAGE)
 * with secure memory blocks (only accessible by SAGE)
 *
 * See: SRAI_Memory_Allocate()
 */
#define SRAI_MEMORY_SAGE_SIDE_ACCESS 0x1
#define SRAI_MEMORY_HOST_SIDE_ACCESS 0x2
#define SRAI_MEMOTY_SAGE_EXPORT_ACCESS 0x4
#define SRAI_MemoryType_SagePrivate (SRAI_MEMORY_SAGE_SIDE_ACCESS)
#define SRAI_MemoryType_Shared  (SRAI_MEMORY_HOST_SIDE_ACCESS | SRAI_MEMORY_SAGE_SIDE_ACCESS)
#define SRAI_MemoryType_SageExport (SRAI_MEMOTY_SAGE_EXPORT_ACCESS)
typedef uint32_t SRAI_MemoryType;

/*
 * SRAI Settings
 * To be used with SRAI_GetSettings / SRAI_SetSettings
 */

typedef struct SRAI_Settings {
    /* index to use in NEXUS_ClientConfiguration::heap[] array for general memory
     * this type of memory is used to share memory blocks between Host and SAGE */
    uint32_t generalHeapIndex;

    /* index to use in NEXUS_ClientConfiguration::heap[] array for video secure (restricted) memory
     * this type of memory is only accessible from SAGE, decoders and M2M,
     * in order to implement a video secure path feature, preventing the Host CPU to access data in it */
    uint32_t videoSecureHeapIndex;

    /* index to use in NEXUS_ClientConfiguration::heap[] array for export region (restricted) memory
     * this type of memory is only accessible from SAGE and M2M,
     * in order to implement a export region feature, preventing the Host CPU to access data in it */
    uint32_t exportHeapIndex;

} SRAI_Settings;

/*
 * SAGE_* API
 * BSAGElib_InOutContainer, BSAGElib_State, BERR_Code: see bsagelib_types.h header file
 */


/***************************************************************
 * Functions definitions
 **/


/*
 * Set/Get SRAI Settings
 */

/***************************************************************************
Summary:
Override SRAI default settings.

Description:
SRAI_GetSettings will return current settings.
SRAI_SetSettings will save new settings.
Settings are used in SRAI at the appropriate time regarding library life cycle and internal mechanics.
The SRAI_GetSettings/SRAI_SetSettings can be done at any time, regardless of Nexus or application state.

Example:
  {
      SRAI_Settings appSettings;

      // Get Current Settings
      SRAI_GetSettings(&appSettings);

      // customize appSettings, for example if designed to use NxClient API:
      appSettings.generalHeapIndex = NXCLIENT_FULL_HEAP;
      appSettings.videoSecureHeapIndex = NXCLIENT_VIDEO_SECURE_HEAP;

      // Save/Apply new settings
      SRAI_SetSettings(&appSettings);
  }
***************************************************************************/
void SRAI_GetSettings(SRAI_Settings *pSettings /* [out] */);
BERR_Code SRAI_SetSettings(SRAI_Settings *pSettings);


/***************************************************************************
Summary:
We are done using SRAI, try to clean-up any cache or temp memory
***************************************************************************/
void SRAI_Cleanup(void);

/*
 * Memory/Container Allocation API
 */

/**************************************************************************************************
Summary:
Allocate a memory block

Description:
This function allocates a memory block of given size.
Memory block location and accessibility depend on memoryType parameter:

 - SRAI_MemoryType_Shared: Memory block is allocated from GLR (GLobal Region).
                           It is accessible from both the Host and the SAGE systems.

 - SRAI_MemoryType_SagePrivate: Memory block is allocated from CRR (Compressed Restricted Region).
                                It is accessible on the SAGE system only and is typically used
                                to handle compressed content for display (without PVR and export).

 - SRAI_MemoryType_SageExport: Memory block is allocated from XRR (eXport Restricted Region).
                               It is accessible on the SAGE system only and is typically used
                               to handle compressed content where PVR and export are allowed.

See Also:
SRAI_Memory_Free()
**************************************************************************************************/
uint8_t *SRAI_Memory_Allocate(uint32_t size, SRAI_MemoryType memoryType);

/***************************************************************************
Summary:
Free a memory block

Description:
This function frees a memory block allocated using SRAI_Memory_Allocate()

See Also:
SRAI_Memory_Allocate()
***************************************************************************/
void SRAI_Memory_Free(uint8_t *pMemory);

/***************************************************************************
Summary:
Allocate an In/Out container

Description:
This function allocates a SAGE In/Out Container used to backhaul parameters
during Host to SAGE communication.
Returned container is zeroed ( memset(0) ).

Note:
If any, container memory blocks needs to be allocated using SRAI_Memory_Allocate()

See Also:
SRAI_Container_Free()
SRAI_Platform_Init()
SRAI_Module_Init()
SRAI_Module_ProcessCommand()
***************************************************************************/
BSAGElib_InOutContainer *SRAI_Container_Allocate(void);

/***************************************************************************
Summary:
Free an In/Out container

Description:
This function frees an In/Out container allocated using SRAI_Container_Allocate()

Note:
All container memory blocks previously allocated with SRAI_Memory_Allocate()
have to be freed with SRAI_Memory_Free()

See Also:
SRAI_Container_Allocate()
***************************************************************************/
void SRAI_Container_Free(BSAGElib_InOutContainer *container);

/*
 * SAGE communication API: platform
 */

/***************************************************************************
Summary:
Insall a Platform/TA

Description:
SAGE side Platform/TA can by dynamically loaded from the Host.
Host application can use this API to install any Platform/TA on the SAGE side.
Once the Platform/TA has been installed on the SAGE side successfully, Host can then call:
SRAI_Platform_XXX APIs to Open and Init the Platform and Modules in the Platform.


See Also:
SRAI_Platform_Open()
SRAI_Platform_Init()
SRAI_Module_Init()
***************************************************************************/
BERR_Code SRAI_Platform_Install(uint32_t platformId,
                             uint8_t *binBuff,
                             uint32_t binSize );

/***************************************************************************
Summary:
UnInsall a platform/TA

Description:
This API is used to Un-install the Platform/TA that has been installed by SRAI_Platform_Install API


See Also:
SRAI_Platform_Close()
***************************************************************************/
BERR_Code SRAI_Platform_UnInstall(uint32_t platformId);


/***************************************************************************
Summary:
Open a platform

Description:
On the SAGE-side, only one instance of a platform exists at a time.
While on the Host side, a platform can be opened from multiple
applications at a time; all bound to a dedicated SRAI_PlatformHandle.
Thus while opening on the Host-side, initialization of the platform might
already have been done, and the current state is poll/returned by SRAI.
If returned state is BSAGElib_State_eUninit, platform on the SAGE-side is
not yet initialized and application must call SRAI_Platform_Init().

See Also:
SRAI_Platform_Close()
SRAI_Platform_Init()
SRAI_Module_Init()
***************************************************************************/
BERR_Code SRAI_Platform_Open(uint32_t platformId,
                             BSAGElib_State *state /* [out] */,
                             SRAI_PlatformHandle *pPlatform /* [out] */);

/***************************************************************************
Summary:
Close a platform

Description:
Close an opened platform.
If a platform is not opened anymore, it is automatically uninitialized.

Note:
Once called, given PlatformHandle is not valid anymore.

See Also:
SRAI_Platform_Open()
***************************************************************************/
void SRAI_Platform_Close(SRAI_PlatformHandle platform);

/***************************************************************************
Summary:
Initialize a platform

Description:
Initialize a platform. To be use if SRAI_Platform_Open() returned
state is BSAGElib_State_eUninit.
Initialization parameters are backhauled using a SAGE In/Out container.

See Also:
SRAI_Platform_Open()
***************************************************************************/
BERR_Code SRAI_Platform_Init(SRAI_PlatformHandle platform,
                             BSAGElib_InOutContainer *container /* [in/out] */);


/*
 * SAGE communication API: module
 */

/***************************************************************************
Summary:
Initialize a module instance

Description:
Initialize a module instance. This module identified by moduleId has to be
available within the platform associated with given PlatformHandle.
Initialization parameters are backhauled using a SAGE In/Out container.

Note:
Multiple instances of the same module could run concurrently
Example with two application using the same module, each
of them will use its own dedicated instance.

See Also:
SRAI_Module_Uninit()
SRAI_Module_ProcessCommands()
***************************************************************************/
BERR_Code SRAI_Module_Init(SRAI_PlatformHandle platform,
                           uint32_t moduleId,
                           BSAGElib_InOutContainer *container /* [in/out] */,
                           SRAI_ModuleHandle *pModule /* [out] */);

/***************************************************************************
Summary:
Un-initialize a module instance

Note:
Once called, given ModuleHandle is not valid anymore.

See Also:
SRAI_Module_Init()
***************************************************************************/
void SRAI_Module_Uninit(SRAI_ModuleHandle module);

/***************************************************************************
Summary:
Process a module's command on the SAGE side

Description:
Process a command on the SAGE side of the module instance associated
with given ModuleHandle.
Command parameters are backhauled using a SAGE In/Out container.

See Also:
SRAI_Module_Init()
***************************************************************************/
BERR_Code SRAI_Module_ProcessCommand(SRAI_ModuleHandle module,
                                     uint32_t commandId,
                                     BSAGElib_InOutContainer *container /* [in/out] */);


/***************************************************************/


/*
 * Management API
 */

typedef void (*SRAI_Callback)(void);
typedef void (*SRAI_TATerminateCallback)(uint32_t platformId);

typedef struct SRAI_ManagementInterface {
    SRAI_Callback watchdog_callback;
    SRAI_TATerminateCallback ta_terminate_callback;
} SRAI_ManagementInterface;

BERR_Code SRAI_Management_Register(SRAI_ManagementInterface *interface);
BERR_Code SRAI_Management_Unregister(SRAI_ManagementInterface *interface);

#ifdef __cplusplus
}
#endif

#endif /* _SAGE_SRAI_H_ */
