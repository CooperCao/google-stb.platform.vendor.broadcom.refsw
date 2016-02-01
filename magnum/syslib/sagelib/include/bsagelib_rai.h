/******************************************************************************
 * (c) 2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BSAGELIB_RAI_H_
#define BSAGELIB_RAI_H_

#include "bsagelib.h"
#include "bsagelib_rpc.h"
#include "bsagelib_types.h"


#ifdef __cplusplus
extern "C" {
#endif


/* ---------- Application Interface to send commands to SAGE Platforms/Modules
 *
 * All of BSAGElib_Rai_Platform_*(), BSAGElib_Rai_Module_*() are asynchronous calls.
 * i.e. they trigger an action and the final result will come in a callback.
 * When using any of them successfully (returned BERR_SUCCESS),
 * the appropriate request has been sent to SAGE-side counterpart, BUT is not yet processed.
 * Response from SAGE-side will fire BSAGElib_RpcInterface::BSAGElib_Rpc_ResponseCallback
 * registered during BSAGElib instance initialization : BSAGElib_Open().
 *
 * Each time an asynchronous call is realized, an ID is asign and return through 'async_id' param.
 * When the response callback is fired, the exact same value is given back as a parameter 'async_id',
 * allowing the caller to bound the response to a request and/or filter/route accordingly.
 *
 * In addition, each 'remote' i.e., a platform or a module, can be associated with an opaque argument
 * 'async_argument' during it's creation (inside BSAGElib_Rai_Platform_Open, BSAGElib_Rai_Module_Init)
 * The extra 'async_argument' is given back with each response callback associated with the SAGElib instance.
 * This allows the caller to bound the remote to one of his internal context in order to save
 * some search/dispatch time when receiving the response.
 *
 * NOTE: The callback is fired under interrupt and thus is not reentrant.
 *       It's caller's responsibility to add appropriate asynchronous response handling
 *       taking care of returning from the callback ISR before doing any blocking operations.
 */


/*
 * SAGE communication API: platform
 */

/***************************************************************************
Summary:
Open a platform

Description:
On the SAGE-side, only one instance of a platform exists at a time.
While on the Host side, a platform can be opened from multiple
applications at a time; all bound to a dedicated BSAGElib_RpcRemoteHandle.
Thus while opening on the Host-side, initialization of the platform might
already have been done, and the current state is poll/returned by SAGElib.
The returned state is accessible in container->basicOut[0]
State can be
 - BSAGElib_State_eInit   : platform is already initialized
 - BSAGElib_State_eUninit : platform on the SAGE-side is not yet initialized and
                            application must call BSAGElib_Rai_Platform_Init().

See Also:
BSAGElib_Rai_Platform_Close()
BSAGElib_Rai_Platform_Init()
BSAGElib_Rai_Module_Init()
***************************************************************************/
BERR_Code
BSAGElib_Rai_Platform_Open(
    BSAGElib_ClientHandle hSAGElibClient,
    uint32_t platformId,
    BSAGElib_InOutContainer *container /* [in/out] */,
    BSAGElib_RpcRemoteHandle *pPlatform /* [out] */,
    void *async_argument /* [in] */,
    uint32_t *pAsync_id /* [out] */);

/***************************************************************************
Summary:
Close a platform

Description:
Close an opened platform.
If a platform is not opened anymore, it is automatically uninitialized.

Note:
Once called, given RemoteHandle is not valid anymore.

See Also:
BSAGElib_Rai_Platform_Open()
***************************************************************************/
void
BSAGElib_Rai_Platform_Close(
    BSAGElib_RpcRemoteHandle platform,
    uint32_t *pAsync_id /* [out] */);

/***************************************************************************
Summary:
Initialize a platform

Description:
Initialize a platform. To be use if BSAGElib_Rai_Platform_Open() returned
state is SAGE_State_eUninit. Return state will be available in container->basicOut[0]
Initialization parameters are backhauled using a SAGE In/Out container.

See Also:
BSAGElib_Rai_Platform_Open()
***************************************************************************/
BERR_Code
BSAGElib_Rai_Platform_Init(
    BSAGElib_RpcRemoteHandle platform,
    BSAGElib_InOutContainer *container /* [in/out] */,
    uint32_t *pAsync_id /* [out] */);

/*
 * SAGE communication API: module
 */

/***************************************************************************
Summary:
Initialize a module instance

Description:
Initialize a module instance. This module identified by moduleId has to be
available within the platform associated with given RemoteHandle.
Initialization parameters are backhauled using a SAGE In/Out container.

Note:
Multiple instances of the same module could run concurrently
Example with two applications using the same module, each
of them will use its own dedicated instance.

See Also:
BSAGElib_Rai_Module_Uninit()
BSAGElib_Rai_Module_ProcessCommands()
***************************************************************************/

BERR_Code
BSAGElib_Rai_Module_Init(
    BSAGElib_RpcRemoteHandle platform,
    uint32_t moduleId,
    BSAGElib_InOutContainer *container, /* [in/out] */
    BSAGElib_RpcRemoteHandle *pModule, /* [out] */
    void *async_argument, /* [in] */
    uint32_t *pAsync_id /* [out] */);

/***************************************************************************
Summary:
Un-initialize a module instance

Note:
Once called, given RemoteHandle is not valid anymore.

See Also:
BSAGElib_Rai_Module_Init()
***************************************************************************/
void
BSAGElib_Rai_Module_Uninit(
    BSAGElib_RpcRemoteHandle module,
    uint32_t *pAsync_id /* [out] */);

/***************************************************************************
Summary:
Process a module's command on the SAGE side

Description:
Process a command on the SAGE side of the module instance associated
with given RemoteHandle.
Command parameters are backhauled using a SAGE In/Out container.

See Also:
BSAGElib_Rai_Module_Init()
***************************************************************************/
BERR_Code
BSAGElib_Rai_Module_ProcessCommand(
    BSAGElib_RpcRemoteHandle module,
    uint32_t commandId,
    BSAGElib_InOutContainer *container, /* [in/out] */
    uint32_t *pAsync_id /* [out] */);

/* ---------------------------------------- */

/*
 * SAGE Memory related API
 * As SAGE-side has some memory constrains, all exchanged blocks
 * and containers must be allocated using the following API.
 */

/*
 * Memory types
 * Restricted: Accessible by restricted components. NOT by Host CPU.
 * Global: Accessible by all components, including Host-CPU
 */
#define BSAGElib_MemoryType_Restricted 0x1
#define BSAGElib_MemoryType_Global     0x3
typedef uint32_t BSAGElib_Rai_MemoryType;

/***************************************************************************
Summary:
Allocate a memory block

Description:
This function allocates a memory block of given size.
Memory block accessibility depends on memoryType parameter, see BSAGElib_Rai_MemoryType.

See Also:
BSAGElib_Rai_Memory_Free()
***************************************************************************/
uint8_t *
BSAGElib_Rai_Memory_Allocate(
    BSAGElib_ClientHandle hSAGElibClient,
    size_t size,
    BSAGElib_Rai_MemoryType memoryType);

/***************************************************************************
Summary:
Free a memory block

Description:
This function frees a memory block allocated using BSAGElib_Rai_Memory_Allocate()

See Also:
BSAGElib_Rai_Memory_Allocate()
***************************************************************************/
void
BSAGElib_Rai_Memory_Free(
    BSAGElib_ClientHandle hSAGElibClient,
    uint8_t *pMemory);

/***************************************************************************
Summary:
Allocate an In/Out container

Description:
This function allocates a SAGE In/Out Container used to backhaul parameters
during Host to SAGE communication.
Returned container is zeroed ( memset(0) ).

Note:
If any, container memory blocks needs to be allocated using BSAGElib_Rai_Memory_Allocate()

See Also:
BSAGElib_Rai_Container_Free()
BSAGElib_Rai_Platform_Open()
BSAGElib_Rai_Platform_Init()
BSAGElib_Rai_Module_Init()
BSAGElib_Rai_Module_ProcessCommand()
***************************************************************************/
BSAGElib_InOutContainer *
BSAGElib_Rai_Container_Allocate(
    BSAGElib_ClientHandle hSAGElibClient);

/***************************************************************************
Summary:
Free an In/Out container

Description:
This function frees an In/Out container allocated using BSAGElib_Rai_Container_Allocate()

Note:
All container memory blocks previously allocated with BSAGElib_Rai_Memory_Allocate()
have to be freed with BSAGElib_Rai_Memory_Free()
This function is NOT doing any garbage collecting!

See Also:
BSAGElib_Rai_Container_Allocate()
***************************************************************************/
void
BSAGElib_Rai_Container_Free(
    BSAGElib_ClientHandle hSAGElibClient,
    BSAGElib_InOutContainer *container);


#ifdef __cplusplus
}
#endif


#endif /* BSAGELIB_RAI_H_ */
