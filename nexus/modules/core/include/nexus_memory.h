/***************************************************************************
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
*
* API Description:
*   API name: Memory
*    Specific APIs related to device memory allocation
*
***************************************************************************/

#ifndef NEXUS_MEMORY_H__
#define NEXUS_MEMORY_H__

#include "nexus_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NEXUS_NUM_MEMC is only for internal use. */

/* NEXUS_NUM_MEMC_REGIONS is the maximum number of distinct physical addressing regions on one MEMC */
#define NEXUS_NUM_MEMC_REGIONS 3

/***************************************************************************
Summary:
Memory allocation settings used in NEXUS_Memory_Allocate
***************************************************************************/
typedef struct NEXUS_MemoryAllocationSettings
{
    NEXUS_HeapHandle heap;      /* Heap from which to allocate. If set, type and bankIndex are ignored. */
    size_t alignment;           /* Alignment of this allocation in units of bytes (not in units of 2^alignment bytes).
                                   0 will select default alignment (which may not be byte-aligned). */
} NEXUS_MemoryAllocationSettings;

typedef struct NEXUS_MemoryBlockAllocationSettings {
    unsigned unused;
} NEXUS_MemoryBlockAllocationSettings;

/***************************************************************************
Summary:
Get default memory allocation settings

Description:
This function will initialize a NEXUS_MemoryBlockAllocationSettings structure to
defaults.

See Also:
NEXUS_MemoryBlock_Allocate
***************************************************************************/
void NEXUS_MemoryBlock_GetDefaultAllocationSettings(
    NEXUS_MemoryBlockAllocationSettings *pSettings   /* [out] Default Settings */
    );

/***************************************************************************
Summary:
Allocate memory from a Nexus heap

Description:
Heap memory is distinct from system or OS-controlled memory.
Nexus may implement multiple heaps. For instance, a non-UMA system may have a general heap and special picture heaps.
Some secure systems may have specialized secure heaps.

Call NEXUS_Memory_Free to free whatever memory was allocated.
***************************************************************************/
#if defined BDBG_DEBUG_BUILD && BDBG_DEBUG_BUILD
#define NEXUS_MemoryBlock_Allocate(HEAP,NUMBYTES,ALIGNMENT,PSETTINGS) NEXUS_MemoryBlock_Allocate_tagged(HEAP,NUMBYTES,ALIGNMENT,PSETTINGS, BSTD_FILE, BSTD_LINE)
#else
#define NEXUS_MemoryBlock_Allocate(HEAP,NUMBYTES,ALIGNMENT,PSETTINGS) NEXUS_MemoryBlock_Allocate_tagged(HEAP,NUMBYTES,ALIGNMENT,PSETTINGS, NULL, 0)
#endif

NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_Allocate_tagged( /* attr{local=true} */
    NEXUS_HeapHandle heap,      /* attr{null_allowed=y} Heap from which to allocate. If NULL, default heap is selected. */
    size_t numBytes,            /* Number of bytes to allocate. */
    size_t alignment,           /* Alignment of this allocation in units of bytes (not in units of 2^alignment bytes).
                                   0 will select default alignment (which may not be byte-aligned). */
    const NEXUS_MemoryBlockAllocationSettings *pSettings,   /* attr{null_allowed=y} may be NULL for default settings */
    const char *fileName,
    unsigned lineNumber
    );

/***************************************************************************
Summary:
Free device memory allocated with NEXUS_MemoryBlock_Allocate
***************************************************************************/
void NEXUS_MemoryBlock_Free( /* attr{local=true} */
    NEXUS_MemoryBlockHandle memoryBlock
    );

/***************************************************************************
Summary:
Returns address of allocated block
***************************************************************************/
NEXUS_Error NEXUS_MemoryBlock_Lock( /* attr{local=true}  */
    NEXUS_MemoryBlockHandle memoryBlock,
    void **ppMemory                                     /* [out] attr{memory=cached} pointer to memory */
    );

/***************************************************************************
Summary:
Releases address of allocated block
***************************************************************************/
void NEXUS_MemoryBlock_Unlock( /* attr{local=true}  */
    NEXUS_MemoryBlockHandle memoryBlock
    );

/***************************************************************************
Summary:
Returns existing NEXUS_MemoryBlockHandle based on address
***************************************************************************/
NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_FromAddress( /* attr{local=true}  */
    void *pMemory                                     /* [attr{memory=cached} pointer to memory */
    );


/***************************************************************************
Summary:
Get default memory allocation settings

Description:
This function will initialize a NEXUS_MemoryAllocationSettings structure to
defaults.

See Also:
NEXUS_Memory_Allocate
***************************************************************************/
void NEXUS_Memory_GetDefaultAllocationSettings(
    NEXUS_MemoryAllocationSettings *pSettings   /* [out] Default Settings */
    );

/***************************************************************************
Summary:
Allocate memory from a Nexus heap

Description:
Heap memory is distinct from system or OS-controlled memory.
Nexus may implement multiple heaps. For instance, a non-UMA system may have a general heap and special picture heaps.
Some secure systems may have specialized secure heaps.

Call NEXUS_Memory_Free to free whatever memory was allocated.
***************************************************************************/
#if defined BDBG_DEBUG_BUILD && BDBG_DEBUG_BUILD
#define NEXUS_Memory_Allocate(NUMBYTES,PSETTINGS,PPMEMORY) NEXUS_Memory_Allocate_tagged(NUMBYTES,PSETTINGS,PPMEMORY, BSTD_FILE, BSTD_LINE)
#else
#define NEXUS_Memory_Allocate(NUMBYTES,PSETTINGS,PPMEMORY) NEXUS_Memory_Allocate_tagged(NUMBYTES,PSETTINGS,PPMEMORY, NULL, 0)
#endif

NEXUS_Error NEXUS_Memory_Allocate_tagged( /* attr{local=true}  */
    size_t numBytes,                                    /* Number of bytes to allocate */
    const NEXUS_MemoryAllocationSettings *pSettings,    /* attr{null_allowed=y} may be NULL for default settings */
    void **ppMemory,                                    /* [out] attr{memory=cached} pointer to memory that has been allocated */
    const char *fileName,
    unsigned lineNumber
    );

/***************************************************************************
Summary:
Free device memory allocated with NEXUS_Memory_Allocate
***************************************************************************/
void NEXUS_Memory_Free( /* attr{local=true}  */
    void *pMemory  /* attr{memory=cached} */
    );

/***************************************************************************
Summary:
Flush the data cache for the specified memory.

Description:
NEXUS_Memory_FlushCache is equivalent to NEXUS_FlushCache, defined in nexus_base_os.h.
Both can be used by the application.
***************************************************************************/
void NEXUS_Memory_FlushCache( /* attr{local=true}  */
    const void *pMemory,/* attr{memory=cached} */
    size_t numBytes     /* Number of bytes to flush */
    );

/***************************************************************************
Summary:
Status of memory heap returned by NEXUS_Memory_GetStatus
***************************************************************************/
typedef struct NEXUS_MemoryStatus
{
    unsigned memoryType; /* see NEXUS_MEMORY_TYPE bitmasks and NEXUS_MemoryType macros in nexus_types.h */
    unsigned heapType;   /* see NEXUS_HEAP_TYPE_XXX macros in nexus_types.h */
    unsigned memcIndex;
    NEXUS_Addr offset; /* physical offset of the heap */
    void    *addr; /*  attr{memory=cached} virtual address of the heap. for internal use, NEXUS_OffsetToCachedAddr(offset) is required. */
    unsigned size; /* total size of the heap in bytes */
    unsigned free; /* total free space in the heap in bytes. be aware that you cannot
                      necessarily allocate this in one block. see largestFreeBlock. */
    unsigned alignment; /* Minimum alignment for this heap in units of bytes (not in units of 2^alignment bytes). */
    unsigned numAllocs; /* total number of allocations in the heap */
    unsigned numFreeBlocks; /* total number of free blocks in the heap */
    unsigned largestFreeBlock; /* the largest free block that can be allocated. this
                      number assumes byte alignment (NEXUS_MemoryAllocationSettings.alignment == 0).
                      any larger alignment will reduce the actual largest allocation possible.  */
    unsigned highWatermark; /* most bytes allocated since system-init or since last NEXUS_Heap_ResetHighWatermark. */
    bool guardBanding; /* [deprecated] returns true if this heap has guardbands. guardbands can help find memory overruns, but at the cost of lower performance. */
    bool dynamic; /* returns true if this heap have had regions added */
} NEXUS_MemoryStatus;

/***************************************************************************
Summary:
Get the current status of a memory heap
***************************************************************************/
NEXUS_Error NEXUS_Heap_GetStatus( /* attr{local=true} */
    NEXUS_HeapHandle heap,
    NEXUS_MemoryStatus *pStatus /* [out] */
    );

/***************************************************************************
Summary:
Get default Display module heap settings
***************************************************************************/
void NEXUS_Heap_GetDefaultDisplayHeapSettings(
    NEXUS_DisplayHeapSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Get current Display module heap settings
***************************************************************************/
void NEXUS_Heap_GetDisplayHeapSettings(
    NEXUS_HeapHandle heap,
    NEXUS_DisplayHeapSettings *pSettings /* [out] */
    );

/***************************************************************************
Summary:
Set new Display module heap settings
***************************************************************************/
NEXUS_Error NEXUS_Heap_SetDisplayHeapSettings(
    NEXUS_HeapHandle heap,
    const NEXUS_DisplayHeapSettings *pSettings
    );

/***************************************************************************
Summary:
Print heap allocations and free blocks to the console
***************************************************************************/
void NEXUS_Heap_Dump(
    NEXUS_HeapHandle heap
    );

/***************************************************************************
Summary:
Reset NEXUS_MemoryStatus.highWatermark
***************************************************************************/
void NEXUS_Heap_ResetHighWatermark(
    NEXUS_HeapHandle heap
    );

/***************************************************************************
Summary:
Prints status of address range checkers
***************************************************************************/
void NEXUS_Memory_PrintStatus(void);

/***************************************************************************
Summary:
Prints status of heaps
***************************************************************************/
void NEXUS_Memory_PrintHeaps(void);

/***************************************************************************
Summary:
Locks allocated memory block in the physical memory
***************************************************************************/
NEXUS_Error NEXUS_MemoryBlock_LockOffset(
                                         NEXUS_MemoryBlockHandle block,
                                         NEXUS_Addr *addr
                                         );

/***************************************************************************
Summary:
Release lock of memory block
***************************************************************************/
void NEXUS_MemoryBlock_UnlockOffset(
                                    NEXUS_MemoryBlockHandle block
                                    );

/**
For system debug, app can call NEXUS_MemoryBlock_CheckIfLocked after it expects
memory block to truly be unlocked. If still locked, it will print an error to
the console. Then app should be debugged.
**/
void NEXUS_MemoryBlock_CheckIfLocked( /* attr{local=true}  */
    NEXUS_MemoryBlockHandle memoryBlock
    );

/**
Relocate unlocked NEXUS_MemoryBlocks from one heap to another without
destroying the NEXUS_MemoryBlockHandle.

The operation is slow and depends on server side virtual memory.
It should be called in a loop to avoid locking the core module for a long time.
For example:

    while (1) {
        unsigned num;
        rc = NEXUS_Memory_MoveUnlockedBlocks(fromHeap, toHeap, 10, &num);
        if (rc || num < 10) break;
    }

This function is not transactional. If an error occurs, whatever number has already been
moved will be reported using pNumMoved; they will not be moved back.
**/
NEXUS_Error NEXUS_Memory_MoveUnlockedBlocks(
    NEXUS_HeapHandle fromHeap,
    NEXUS_HeapHandle toHeap,
    unsigned maxNumber, /* limit the # of moves for this call */
    unsigned *pNumMoved
    );

void NEXUS_Heap_ToString( /* attr{local=true} */
    const NEXUS_MemoryStatus *pStatus,
    char *buf,
    unsigned bufsize
    );

typedef enum NEXUS_HeapLookupType
{
    NEXUS_HeapLookupType_eMain,
    NEXUS_HeapLookupType_eCompressedRegion, /* With Sage, this is the compresed restricted region (CRR) */
    NEXUS_HeapLookupType_eExportRegion, /* With Sage, this is the export restricted region (XRR) */
    NEXUS_HeapLookupType_eMax
} NEXUS_HeapLookupType;

/**
Allows for heap lookup in either server or client context.
The mapping to server index macro and NxClient heap macro is currently as follows:

HeapLookupType    Nexus                   NxClient
eMain             NEXUS_MEMC0_MAIN_HEAP   NXCLIENT_FULL_HEAP
eCompressedRegion NEXUS_VIDEO_SECURE_HEAP NXCLIENT_VIDEO_SECURE_HEAP
eExportRegion     NEXUS_EXPORT_HEAP       NXCLIENT_EXPORT_HEAP
**/
NEXUS_HeapHandle NEXUS_Heap_Lookup(
    NEXUS_HeapLookupType lookupType
    );

#include "nexus_core_priv.h"

typedef struct NEXUS_HeapRuntimeSettings
{
    bool secure;
} NEXUS_HeapRuntimeSettings;

typedef struct NEXUS_MemoryBlockToken *NEXUS_MemoryBlockTokenHandle;

/* NEXUS_MemoryBlock_Clone will return a new NEXUS_MemoryBlockHandle which points
to the same Nexus device memory as the NEXUS_MemoryBlockHandle used to create the token.
A successful Clone will also destroy the token.

Once cloned, the underlying device memory will remain allocated until both the new and
original NEXUS_MemoryBlockHandle are freed.
*/
NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_Clone(
    NEXUS_MemoryBlockTokenHandle token
    );

/* The created token can be used to clone the MemoryBlock one time.
NEXUS_MemoryBlockTokenHandle is created as "shareable", which means any other Nexus process
can use it. The application must communicate NEXUS_MemoryBlockTokenHandle to other processes
using its own IPC.

The token remains valid until either NEXUS_MemoryBlock_Clone or NEXUS_MemoryBlock_DestroyToken
is called.

If the NEXUS_MemoryBlockHandle is destroyed before the token is cloned or destroyed, the
underlying device memory remains allocated.
*/
NEXUS_MemoryBlockTokenHandle NEXUS_MemoryBlock_CreateToken( /* attr{destructor=NEXUS_MemoryBlock_DestroyToken} */
    NEXUS_MemoryBlockHandle memoryBlock
    );

void NEXUS_MemoryBlock_DestroyToken(
    NEXUS_MemoryBlockTokenHandle token
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_MEMORY_H__ */
