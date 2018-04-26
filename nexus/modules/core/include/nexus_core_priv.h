/***************************************************************************
*  Copyright (C) 2018 Broadcom.
*  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
*  and may only be used, duplicated, modified or distributed pursuant to
*  the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied),
*  right to use, or waiver of any kind with respect to the Software, and
*  Broadcom expressly reserves all rights in and to the Software and all
*  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
*  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
*  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
*  Except as expressly set forth in the Authorized License,
*
*  1.     This program, including its structure, sequence and organization,
*  constitutes the valuable trade secrets of Broadcom, and you shall use all
*  reasonable efforts to protect the confidentiality thereof, and to use this
*  information only in connection with your use of Broadcom integrated circuit
*  products.
*
*  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
*  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
*  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
*  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
*  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
*  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
*  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
*  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
*  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
*  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
*  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
*  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
***************************************************************************/
#ifndef NEXUS_CORE_PRIV_H__
#define NEXUS_CORE_PRIV_H__

#include "nexus_types.h"
#include "nexus_memory.h"

/**
These are internal functions, but are still proxied.
Applications should not call them directly.
**/

#ifdef __cplusplus
extern "C" {
#endif

/**
Summary:
Settings for NEXUS_Platform_CreateHeap
**/
typedef struct NEXUS_PlatformCreateHeapSettings
{
    NEXUS_Addr offset; /* physical address */
    unsigned size; /* in bytes */
    NEXUS_MemoryType memoryType; /* requested memory mapping */
    unsigned heapType;
    unsigned alignment; /* required alignment (in bytes) of allocations in this region */
    bool locked; /* if true, nexus is not allowed to allocate from this heap */
    unsigned userAddress; /* deprecated */
} NEXUS_PlatformCreateHeapSettings;

NEXUS_HeapHandle NEXUS_Heap_CreateInternal ( /* attr{destructor=NEXUS_Heap_DestroyInternal } */
        const NEXUS_PlatformCreateHeapSettings *pSettings
        );

void NEXUS_Heap_DestroyInternal(
        NEXUS_HeapHandle heap
        );

typedef struct NEXUS_MemoryBlockProperties
{
    unsigned memoryType; /* see NEXUS_MEMORY_TYPE bitmasks */
    size_t size; /* size of the allocated block */
} NEXUS_MemoryBlockProperties;
typedef struct NEXUS_MemoryBlockTag
{
    char fileName[64];
    unsigned lineNumber;
} NEXUS_MemoryBlockTag;

NEXUS_MemoryBlockHandle NEXUS_MemoryBlock_Allocate_driver( /* attr{destructor=NEXUS_MemoryBlock_Free_driver} */
    NEXUS_HeapHandle heap,
    size_t numBytes,
    size_t alignment,
    const NEXUS_MemoryBlockAllocationSettings *pSettings,  /* attr{null_allowed=y} */
    const NEXUS_MemoryBlockTag *pTag,
    NEXUS_MemoryBlockProperties *pProperties
    );

void NEXUS_MemoryBlock_Free_driver(
    NEXUS_MemoryBlockHandle memoryBlock
    );

void NEXUS_MemoryBlock_GetProperties(
    NEXUS_MemoryBlockHandle memoryBlock,
    NEXUS_MemoryBlockProperties *pProperties
    );

typedef struct NEXUS_MemoryBlockUserState {
    NEXUS_P_MemoryUserAddr state;
} NEXUS_MemoryBlockUserState;

NEXUS_Error NEXUS_MemoryBlock_GetUserState(
    NEXUS_MemoryBlockHandle memoryBlock,
    NEXUS_MemoryBlockUserState *userState
    );

void NEXUS_MemoryBlock_SetUserState(
    NEXUS_MemoryBlockHandle memoryBlock,
    const NEXUS_MemoryBlockUserState *userState /* attr{null_allowed=y}  */
    );

void NEXUS_MemoryBlock_Free_local ( /* attr{local=true} */
    NEXUS_MemoryBlockHandle memoryBlock
    );

NEXUS_FrontendConnectorHandle NEXUS_FrontendConnector_Create( /* attr{destructor=NEXUS_FrontendConnector_Destroy} */
    void);

NEXUS_FrontendConnectorHandle NEXUS_FrontendConnector_Acquire( /* attr{release=NEXUS_FrontendConnector_Release} */
    unsigned index
    );

void NEXUS_FrontendConnector_Release(
    NEXUS_FrontendConnectorHandle handle
    );

void NEXUS_FrontendConnector_Destroy(
    NEXUS_FrontendConnectorHandle handle
    );

NEXUS_VideoInputHandle NEXUS_VideoInput_Create( /* attr{destructor=NEXUS_VideoInput_Destroy } */
    void
    );

void NEXUS_VideoInput_Destroy(
    NEXUS_VideoInputHandle videoInput
    );

NEXUS_VideoOutputHandle NEXUS_VideoOutput_Create( /* attr{destructor=NEXUS_VideoOutput_Destroy } */
    void
    );

void NEXUS_VideoOutput_Destroy(
    NEXUS_VideoOutputHandle videoOutput
    );

NEXUS_AudioOutputHandle NEXUS_AudioOutput_Create( /* attr{destructor=NEXUS_AudioOutput_Destroy } */
    void
    );

void NEXUS_AudioOutput_Destroy(
    NEXUS_AudioOutputHandle audioOutput
    );

NEXUS_AudioInputHandle NEXUS_AudioInput_Create( /* attr{destructor=NEXUS_AudioInput_Destroy } */
    void
    );

void NEXUS_AudioInput_Destroy(
    NEXUS_AudioInputHandle audioInput
    );

/* returns bounds of CRR. returns 0 if it does not exist. */
void NEXUS_Memory_GetVideoSecureHeap(
    NEXUS_Addr *pOffset,
    unsigned *pSize
    );

/* Returns everything except NEXUS_MemoryStatus.addr, which can be locally obtained with NEXUS_OffsetToCacheAddr. See NEXUS_Heap_GetStatus. */
NEXUS_Error NEXUS_Heap_GetStatus_driver(
    NEXUS_HeapHandle heap,
    NEXUS_MemoryStatus *pStatus /* [out] */
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef NEXUS_MEMORY_H__ */
