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

#ifndef BSAGELIB_INTERFACES_H_
#define BSAGELIB_INTERFACES_H_


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Common interfaces to SAGElib and SAGElib_Tools
 */

/* Memory Allocation interface includes a malloc and a free callbacks */
typedef void* (*BSAGElib_MemoryAlloc_MallocCallback)(size_t size);
typedef void  (*BSAGElib_MemoryAlloc_FreeCallback)(void *mem);

typedef struct {
    BSAGElib_MemoryAlloc_MallocCallback malloc;
    BSAGElib_MemoryAlloc_MallocCallback malloc_restricted;
    BSAGElib_MemoryAlloc_FreeCallback free;
} BSAGElib_MemoryAllocInterface;

/*
 * virtual address < -- > physical address (offset) conversion
 */
#if BSAGELIB_HARDEN_MMAP
/* Not compiled by default */
    typedef void* (*BSAGElib_MemoryMap_OffsetToAddrCallback)(uint64_t offset, uint64_t length);
    typedef uint64_t (*BSAGElib_MemoryMap_AddrToOffsetCallback)(const void *address, uint64_t length);
#else
    typedef void* (*BSAGElib_MemoryMap_OffsetToAddrCallback)(uint64_t offset);
    typedef uint64_t (*BSAGElib_MemoryMap_AddrToOffsetCallback)(const void *address);
#endif

typedef struct {
    BSAGElib_MemoryMap_OffsetToAddrCallback offset_to_addr;
    BSAGElib_MemoryMap_AddrToOffsetCallback addr_to_offset;
} BSAGElib_MemoryMapInterface;

/*
 * CPU cache < -- > physical memory synchronization
 */

typedef void (*BSAGElib_MemorySync_FlushCallback)(const void *address, size_t size);
typedef BSAGElib_MemorySync_FlushCallback BSAGElib_MemorySync_InvalidateCallback;

typedef struct {
    BSAGElib_MemorySync_FlushCallback flush;
    BSAGElib_MemorySync_InvalidateCallback invalidate;
} BSAGElib_MemorySyncInterface;

/*
 * sync interface for concurrent access to ressources
 */

typedef void (*BSAGElib_Sync_LockCallback)(void);
typedef BSAGElib_Sync_LockCallback BSAGElib_Sync_UnlockCallback;

typedef struct {
    BSAGElib_Sync_LockCallback lock;
    BSAGElib_Sync_UnlockCallback unlock;
} BSAGElib_SyncInterface;


#ifdef __cplusplus
}
#endif


#endif /* BSAGELIB_INTERFACES_H_ */
