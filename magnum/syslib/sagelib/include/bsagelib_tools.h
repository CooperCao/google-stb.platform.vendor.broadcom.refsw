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

#ifndef BSAGELIB_TOOLS_H_
#define BSAGELIB_TOOLS_H_

#include "bsagelib_types.h"
#include "bsagelib_interfaces.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Convert API.
 * Used to convert Virtual to Offset addresses and sync physical memory
 * for container (and sub resources) allowing easier exchange between Host and SAGE sides.
 */

/* OffsetToAddress, for container and all sub memory blocks:
   - Convert offsets (physical addresses) into cached virtual addresses
   - invalidate memory areas to force next read from physical memory */
BSAGElib_InOutContainer *
BSAGElib_Tools_ContainerOffsetToAddress(
    uint64_t containerOffset,
    BSAGElib_MemorySyncInterface *i_memory_sync,
    BSAGElib_MemoryMapInterface *i_memory_map);

/* AddressToOffset, for container and all sub memory blocks:
   - Convert virtual addresses into offsets (physical addresses)
   - flush memory to sync physical memory with CPU cache */
uint64_t
BSAGElib_Tools_ContainerAddressToOffset(
    BSAGElib_InOutContainer *containerAddr,
    BSAGElib_MemorySyncInterface *i_memory_sync,
    BSAGElib_MemoryMapInterface *i_memory_map);


/*
 * Container Cache API
 * Used to manipulate a container cache and avoid memory fragmentation
 */

/* This handle is used to store the context of a SAGElib ContainerCache */
typedef struct BSAGElib_P_Tools_ContainerCache *BSAGElib_Tools_ContainerCacheHandle;

/* Initialize a Container Cache context
   - i_memory_alloc is a memory allocation interface to alloc/free the containers
   - i_sync_cache is a sync interface to lock/unlock access to the cache
     (if only one thread is accessing to the cache, this can be set to NULL) */

BSAGElib_Tools_ContainerCacheHandle
BSAGElib_Tools_ContainerCache_Open(
    BSAGElib_MemoryAllocInterface *i_memory_alloc,
    BSAGElib_SyncInterface *i_sync_cache);

/* Uninitialize the Container Cache
   - will also garbage collect all pending containers. */
void
BSAGElib_Tools_ContainerCache_Close(
    BSAGElib_Tools_ContainerCacheHandle hContainerCache);

/* Set Cache maximum size
   - set the cache maximum size, any extra container will be freed instead of inserting back to the cache */
void
BSAGElib_Tools_ContainerCache_SetMax(
    BSAGElib_Tools_ContainerCacheHandle hContainerCache,
    uint32_t max);

/* Allocate a container
   - take care of either allocating the memory for it or taking it from the cache if available */
BSAGElib_InOutContainer *
BSAGElib_Tools_ContainerCache_Allocate(
    BSAGElib_Tools_ContainerCacheHandle hContainerCache);

/* Freeing a container
   - take care of either putting the container resource back into the cache or freeing it */
void
BSAGElib_Tools_ContainerCache_Free(
    BSAGElib_Tools_ContainerCacheHandle hContainerCache,
    BSAGElib_InOutContainer *container);

/* Return a string describing the SAGE side return code*/
const char *
BSAGElib_Tools_ReturnCodeToString(BERR_Code returnCode);


#ifdef __cplusplus
}
#endif


#endif /* BSAGELIB_TOOLS_H_ */
