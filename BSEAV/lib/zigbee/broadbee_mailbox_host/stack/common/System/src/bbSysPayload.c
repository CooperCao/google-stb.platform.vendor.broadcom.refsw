/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

/*******************************************************************************
 *
 * DESCRIPTION:
 *      System Payloads implementation.
 *
*******************************************************************************/

/************************* INCLUDES *****************************************************/
#include "bbSysPayload.h"           /* System Payloads interface. */

#ifdef SUBSTITUTE_PAYLOAD_FUNCS
#define NUM_DEBUG_DATA 61
_MY_DebugType_t ____debugData[NUM_DEBUG_DATA];
uint32_t ____iDebugData = 0;

bool ___AddMyDebugData(SYS_DataPointer_t *payload, const char *file, int line)
{
    if (MM_BAD_BLOCK_ID != payload->block)
    {
        if (!payload->isStatic)
        {
            if (____iDebugData < NUM_DEBUG_DATA)
            {
                ____debugData[____iDebugData].blockId = payload->block;
                ____debugData[____iDebugData].file = file;
                ____debugData[____iDebugData++].line = line;
            }
        }
    }
    return true;
}

bool ___RemoveMyDebugData(SYS_DataPointer_t *payload)
{
    if (MM_BAD_BLOCK_ID != payload->block)
    {
        if (!payload->isStatic)
        {
            for (uint32_t i = 0; i < ____iDebugData; ++i)
                if (____debugData[i].blockId == payload->block)
                {
                    memcpy(&____debugData[i], &____debugData[i + 1], (____iDebugData - (i + 1)) * sizeof(_MY_DebugType_t));
                    ____iDebugData--;
                    break;
                }
        }
    }
    return true;
}
#endif /* RF4CE_CONTROLLER */

/************************* IMPLEMENTATION ***********************************************/
/*
 * Initializes the dynamic Payload descriptor as empty.
 */
void SYS_SetEmptyPayload(SYS_DataPointer_t *payload)
{
    SYS_DbgAssert(NULL != payload, SYSPAYLOAD_SETEMPTYPAYLOAD_DA0);

#if defined(_MEMORY_MANAGER_)

    SYS_DbgAssertStatic(sizeof(SYS_DataLength_t) <= 4);
    SYS_DbgAssertStatic(0 == MM_BAD_BLOCK_ID);
    SYS_DbgAssertStatic(2 == sizeof(MM_ChunkId_t));
# if defined(__arc__)
    SYS_DbgAssertStatic(4 == sizeof(SYS_DataPointer_t));
    payload->plain = (uint32_t)MM_BAD_BLOCK_ID;
# else /* ! __arc__ */
    SYS_DbgAssertStatic(8 == sizeof(SYS_DataPointer_t));
    payload->plain = (uint64_t)MM_BAD_BLOCK_ID;
# endif /* ! __arc__ */

#else /* ! _MEMORY_MANAGER_ */
    payload->data = NULL;
    payload->size = 0;
    payload->isStatic = FALSE;

#endif /* ! _MEMORY_MANAGER_ */
}


/*
 * Initializes and links the static Payload descriptor.
 */
void SYS_LinkStaticPayload(SYS_DataPointer_t *payload, uint8_t *startAddress, SYS_DataLength_t capacity)
{
    SYS_DbgAssertStatic(SYS_STATIC_DATA_LENGTH_MAX <= UINT8_MAX);
    SYS_DbgAssert(NULL != payload, SYSPAYLOAD_LINKSTATICPAYLOAD_DA0);
    SYS_DbgAssert(NULL != startAddress, SYSPAYLOAD_LINKSTATICPAYLOAD_DA1);
    SYS_DbgAssert(capacity > 0, SYSPAYLOAD_LINKSTATICPAYLOAD_DA2);
    SYS_DbgAssert(capacity <= SYS_STATIC_DATA_LENGTH_MAX, SYSPAYLOAD_LINKSTATICPAYLOAD_DA3);

#if defined(_MEMORY_MANAGER_)

# if defined(__arc__)
    SYS_DbgAssertStatic(24 / 8 == offsetof(SYS_DataPointer_t, capacity));
# else /* ! __arc__ */
    SYS_DbgAssertStatic(40 / 8 == offsetof(SYS_DataPointer_t, capacity));
# endif /* ! __arc__ */

# if defined(_DEBUG_)
    {
        uint32_t prjDccmSize = (uint32_t)_PRJ_DCCM_SIZE_;       /* Size of dCCM according to the
                                                                    SVR3 linker command-file. */
        SYS_DbgAssert(prjDccmSize > 0, SYSPAYLOAD_LINKSTATICPAYLOAD_DA4);
        SYS_DbgAssert(startAddress >= _PRJ_DCCM_BASE_, SYSPAYLOAD_LINKSTATICPAYLOAD_DA5);
        SYS_DbgAssert(startAddress < _PRJ_DCCM_BASE_ + prjDccmSize, SYSPAYLOAD_LINKSTATICPAYLOAD_DA6);
        SYS_DbgAssert((uint32_t)capacity < prjDccmSize, SYSPAYLOAD_LINKSTATICPAYLOAD_DA7);
    }
# endif /* _DEBUG_ */

    payload->plain = SYS_STATIC_PAYLOAD_DESCR_INIT(startAddress, capacity);

#else /* ! _MEMORY_MANAGER_ */
    payload->data = startAddress;
    payload->size = 0;
    payload->capacity = capacity;

#endif /* ! _MEMORY_MANAGER_ */
}


/*
 * Returns the Payload size, in bytes; validates the Payload.
 */
SYS_DataLength_t SYS_GetPayloadSize(const SYS_DataPointer_t *payload)
{
    SYS_DbgAssert(NULL != payload, SYSPAYLOAD_GETPAYLOADSIZE_DA0);

    if (payload->isStatic)
    {
        uint32_t capacity = payload->capacity;      /* Payload capacity. */
        uint32_t size = payload->size;              /* Payload size. */
#if defined(_MEMORY_MANAGER_)
        uint32_t staticDataShift = payload->staticDataShift;    /* Payload data. */
#endif

        /* It is true that (payload->capacity > 0). */
        SYS_DbgAssert(capacity <= SYS_STATIC_DATA_LENGTH_MAX, SYSPAYLOAD_GETPAYLOADSIZE_DA1);
#if defined(_MEMORY_MANAGER_)
        SYS_DbgAssert((uint8_t *)capacity < _PRJ_DCCM_SIZE_, SYSPAYLOAD_GETPAYLOADSIZE_DA2);
        SYS_DbgAssert((uint8_t *)staticDataShift < _PRJ_DCCM_SIZE_, SYSPAYLOAD_GETPAYLOADSIZE_DA3);
#else
        SYS_DbgAssert(NULL != payload->data, SYSPAYLOAD_GETPAYLOADSIZE_DA4);  /* Is linked. */
#endif
        SYS_DbgAssert(size <= SYS_STATIC_DATA_LENGTH_MAX, SYSPAYLOAD_GETPAYLOADSIZE_DA5);
        SYS_DbgAssert(size <= capacity, SYSPAYLOAD_GETPAYLOADSIZE_DA6);
    }

    else /* ! payload->isStatic */
    {
#if defined(_MEMORY_MANAGER_)
        SYS_DataLength_t size = payload->size;              /* Payload size. */
        SYS_DbgAssert(0 == size, SYSPAYLOAD_GETPAYLOADSIZE_DA7);
        SYS_DbgAssert(size <= SYS_DYNAMIC_DATA_LENGTH_MAX, SYSPAYLOAD_GETPAYLOADSIZE_DA8);
        return SYS_MemoryManagerGetSize(payload->block);
#endif
    }

    return payload->size;
}


/*
 * Discovers whether the specified Payload is allocated; validates the Payload.
 */
bool SYS_CheckPayload(const SYS_DataPointer_t *payload)
{
#if defined(_MEMORY_MANAGER_)
    SYS_DbgAssert(NULL != payload, SYSPAYLOAD_CHECKPAYLOAD_DA1);
    return (payload->isStatic) ?
           (0 < SYS_GetPayloadSize(payload)) :
           (MM_BAD_BLOCK_ID != payload->block);
#else
    SYS_DataLength_t size;      /* Size of the specified Payload object allocated memory. */

    size = SYS_GetPayloadSize(payload);
    /* SYS_GetPayloadSize() also validates that (NULL != payload). */

    SYS_DbgAssert(IMP(!payload->isStatic, (0 == size) == (NULL == payload->data)), SYSPAYLOAD_CHECKPAYLOAD_DA0);

    return (size > 0);
#endif
}


/*
 * Allocates dynamic or static memory of the specified size.
 */
bool ___SYS_MemAlloc(SYS_DataPointer_t *payload, SYS_DataLength_t size)
{
    SYS_DbgAssert(NULL != payload, SYSPAYLOAD_MEMALLOC_DA0);
    SYS_DbgAssertComplex(!SYS_CheckPayload(payload), SYSPAYLOAD_MEMALLOC_DA1);

    if (payload->isStatic)
    {
        SYS_DbgAssert(size <= SYS_STATIC_DATA_LENGTH_MAX, SYSPAYLOAD_MEMALLOC_DA2);
        SYS_DbgAssert(size <= (uint32_t)payload->capacity, SYSPAYLOAD_MEMALLOC_DA3);
        payload->size = size;
        return true;
    }

    else /* ! payload->isStatic */
    {
        if (0 == size)
            /* The payload object stays empty (i.e., its size is left equal to 0). */
            return true;

#if defined(_MEMORY_MANAGER_)
        SYS_DbgAssert(size <= SYS_DYNAMIC_DATA_LENGTH_MAX, SYSPAYLOAD_MEMALLOC_DA4);
        {
            MM_ChunkId_t block;     /* Block ID of the dynamic memory chunk
                                        allocated by the Memory Manager. */
            block = SYS_MemoryManagerAlloc(size);
            if (MM_BAD_BLOCK_ID != block)
            {
                payload->block = block;
                /* The size is saved by the Memory Manager in its internal descriptors. */
                return true;
            }
        }

#else /* ! _MEMORY_MANAGER_ */
        {
            void *data;             /* Pointer to the dynamic memory chunk
                                        allocated by the conventional API. */
            data = malloc(size);
            if (NULL != data)
            {
                payload->data = data;
                payload->size = size;
                return true;
            }
            else
                SYS_DbgLogId(SYSPAYLOAD_MEMALLOC_DL1);
        }

#endif /* ! _MEMORY_MANAGER_ */
    }

    return false;
}


/*
 * Frees the specified Payload object.
 */
SYS_PUBLIC void ___SYS_FreePayload(SYS_DataPointer_t *payload)
{
    SYS_DbgAssert(NULL != payload, SYSPAYLOAD_FREEPAYLOAD_DA0);

    if (!SYS_CheckPayload(payload))
    {
        SYS_SetEmptyPayload(payload);
        return;
    }

    if (payload->isStatic)
        payload->size = 0;
    else
    {
#if defined(_MEMORY_MANAGER_)
        const bool isSuccess = SYS_MemoryManagerFree(payload->block);
        SYS_DbgAssert(isSuccess, SYSPAYLOAD_FREEPAYLOAD_DA2);
#else
        free(payload->data);
#endif
        SYS_SetEmptyPayload(payload);
    }
}


/*
 * Copies the array of bytes from arbitrary memory region into the Payload object.
 */
void SYS_CopyToPayload(SYS_DataPointer_t *payload,
                       SYS_DataLength_t   offset,
                       const void        *src,
                       SYS_DataLength_t   count)
{
    if (0 == count)
        return;
    SYS_DbgAssertComplex(SYS_CheckPayload(payload), HALT_SYS_CopyToPayload_PayloadIsEmpty);
    const SYS_DataLength_t size = SYS_GetPayloadSize(payload);
    SYS_DbgAssert(size > 0, HALT_SYS_CopyToPayload_PayloadSizeIsZero);
    SYS_DbgAssert(offset <= size, HALT_SYS_CopyToPayload_OffsetIsGreaterThanPayloadSize);
    SYS_DbgAssert(count <= size - offset, HALT_SYS_CopyToPayload_CountPlusOffsetExceedsPayloadSize);

#if defined(_MEMORY_MANAGER_)
    if (payload->isStatic)
        memcpy(SYS_STATIC_PAYLOAD_GET_ADDRESS_FROM_SHIFT(payload->staticDataShift) + offset, src, count);
    else {
        const MM_Size_t copied = SYS_MemoryManagerCopyFrom(payload->block, (void*)src, count, offset);
        SYS_DbgAssert(copied == count, HALT_SYS_CopyToPayload_CopiedLessThanRequired);
    }
#else
    memcpy(payload->data + offset, src, count);

#endif
}


/*
 * Copies the array of bytes from the Payload object into arbitrary memory region.
 */
void SYS_CopyFromPayload(void                    *dst,
                         const SYS_DataPointer_t *payload,
                         SYS_DataLength_t         offset,
                         SYS_DataLength_t         count)
{
    if (0 == count)
        return;
    SYS_DbgAssertComplex(SYS_CheckPayload(payload), HALT_SYS_CopyFromPayload_PayloadIsEmpty);

#if defined(_DEBUG_)
    const SYS_DataLength_t size = SYS_GetPayloadSize(payload);
    SYS_DbgAssert(size > 0, HALT_SYS_CopyFromPayload_PayloadSizeIsZero);
    SYS_DbgAssert(offset <= size, HALT_SYS_CopyFromPayload_OffsetIsGreaterThanPayloadSize);
    SYS_DbgAssert(count <= size - offset, HALT_SYS_CopyFromPayload_CountPlusOffsetExceedsPayloadSize);
#endif

#if defined(_MEMORY_MANAGER_)
    if (payload->isStatic)
        memcpy(dst, SYS_STATIC_PAYLOAD_GET_ADDRESS_FROM_SHIFT(payload->staticDataShift) + offset, count);
    else {
        const MM_Size_t copied = SYS_MemoryManagerCopyTo(dst, payload->block, count, offset);
        SYS_DbgAssert(copied == count, HALT_SYS_CopyFromPayload_CopiedLessThanRequired);
    }
#else
    memcpy(dst, payload->data + offset, count);

#endif
}

/*
 * Copies the array of bytes from the Payload object into another Payload object.
 */
SYS_PUBLIC void SYS_CopyPayloadToPayload(SYS_DataPointer_t       *dst,
                                         const SYS_DataPointer_t *src,
                                         SYS_DataLength_t         count)
{
    if (0 == count)
        return;
    SYS_DbgAssertComplex(SYS_CheckPayload(dst) && SYS_CheckPayload(src), HALT_SYS_CopyPayloadToPayload_PayloadIsEmpty);
    const SYS_DataLength_t sizeDst = SYS_GetPayloadSize(dst);
    const SYS_DataLength_t sizeSrc = SYS_GetPayloadSize(src);
    SYS_DbgAssert(sizeDst > 0 && sizeSrc > 0, HALT_SYS_CopyPayloadToPayload_PayloadSizeIsZero);
    SYS_DbgAssert(count <= sizeDst && count <= sizeSrc, HALT_SYS_CopyPayloadToPayload_CountIsGreaterThanPayloadSize);

#if defined(_MEMORY_MANAGER_)
    if (dst->isStatic) {
        if (src->isStatic)
            memcpy(SYS_STATIC_PAYLOAD_GET_ADDRESS_FROM_SHIFT(dst->staticDataShift),
                   SYS_STATIC_PAYLOAD_GET_ADDRESS_FROM_SHIFT(src->staticDataShift), count);
        else
            SYS_CopyFromPayload(SYS_STATIC_PAYLOAD_GET_ADDRESS_FROM_SHIFT(dst->staticDataShift), src, 0, count);
    }
    else {
        if (src->isStatic)
            SYS_CopyToPayload(dst, 0, SYS_STATIC_PAYLOAD_GET_ADDRESS_FROM_SHIFT(src->staticDataShift), count);
        else {
            const MM_Size_t copied = SYS_MemoryManagerCopyChunk(dst->block, src->block, count);
            SYS_DbgAssert(copied == count, HALT_SYS_CopyPayloadToPayload_CopiedLessThanRequired);
        }
    }
#else
    memcpy(dst->data, src->data, count);

#endif
}


#if defined(_MEMORY_MANAGER_)
/*
 * Appends the first Payload with data from the second Payload.
 */
void ___SYS_AppendPayload(SYS_DataPointer_t *dst, SYS_DataPointer_t *src)
{
    SYS_DbgAssert(NULL != dst && NULL != src, FAIL_SYS_AppendPayload_PtrPayloadIsNull);
    SYS_DbgAssert(!dst->isStatic && !src->isStatic, FAIL_SYS_AppendPayload_StaticPayloadIsNotSupported);

    const bool isSuccess = SYS_MemoryManagerAppend(&dst->block, src->block, /*pExhaustBlockId*/ NULL);
    SYS_DbgAssert(isSuccess, HALT_SYS_AppendPayload_Failed);
}


/*
 * Cuts out a specified number of bytes from the Payload starting from its head.
 */
void ___SYS_FreePayloadHead(SYS_DataPointer_t *payload, SYS_DataLength_t count)
{
    SYS_DbgAssert(NULL != payload, SYSPAYLOAD_FREEPAYLAODHEAD_DA0);
    SYS_DbgAssert(!payload->isStatic, SYSPAYLOAD_FREEPAYLAODHEAD_DA1);

    {
        const SYS_DataLength_t inSize = SYS_MemoryManagerGetSize(payload->block);
        SYS_DbgAssert(count <= inSize, SYSPAYLOAD_FREEPAYLAODHEAD_DA2);

        const bool isSuccess = SYS_MemoryManagerFreeHead(&payload->block, count);
        SYS_DbgAssert(isSuccess, SYSPAYLOAD_FREEPAYLAODHEAD_DA3);
    }
}

/*
 * Reduces the size of Payload to the specified value cutting its tail.
 */
void SYS_FreePayloadTail(SYS_DataPointer_t *payload, SYS_DataLength_t newSize)
{
    SYS_DbgAssert(NULL != payload, SYSPAYLOAD_FREEPAYLOADTAIL_DA0);
    SYS_DbgAssert(!payload->isStatic, SYSPAYLOAD_FREEPAYLOADTAIL_DA1);

    SYS_DbgAssert(newSize <= SYS_MemoryManagerGetSize(payload->block), SYSPAYLOAD_FREEPAYLOADTAIL_DA2);
    const bool isSuccess = SYS_MemoryManagerFreeTail(payload->block, newSize);
    SYS_DbgAssert(isSuccess, SYSPAYLOAD_FREEPAYLOADTAIL_DA3);
}


/*
 * Duplicates the given Payload.
 */
bool ___SYS_DuplicatePayload(SYS_DataPointer_t *dst, const SYS_DataPointer_t *src)
{
    SYS_DbgAssert(NULL != dst, SYSPAYLOAD_DUPLICATEPAYLOAD_DA0);
    SYS_DbgAssert(NULL != src, SYSPAYLOAD_DUPLICATEPAYLOAD_DA1);
    SYS_DbgAssert(!dst->isStatic, SYSPAYLOAD_DUPLICATEPAYLOAD_DA2);
    SYS_DbgAssert(!src->isStatic, SYSPAYLOAD_DUPLICATEPAYLOAD_DA3);
    SYS_DbgAssertComplex(!SYS_CheckPayload(dst), SYSPAYLOAD_DUPLICATEPAYLOAD_DA4);

    {
        MM_ChunkId_t  dstBlock;     /* Memory Manager block ID for the duplicate Payload. */
        MM_ChunkId_t  srcBlock;     /* Memory Manager block ID of the original Payload. */

        srcBlock = src->block;
        dst->block = dstBlock = SYS_MemoryManagerDuplicate(srcBlock);
        SYS_DbgAssertLog(IMP(MM_BAD_BLOCK_ID != srcBlock, MM_BAD_BLOCK_ID != dstBlock), SYSPAYLOAD_DUPLICATEPAYLOAD_DL0);
        return IMP(MM_BAD_BLOCK_ID != srcBlock, MM_BAD_BLOCK_ID != dstBlock);
    }
}


/*
 * Detaches the tail from the Payload at the given offset.
 */
bool ___SYS_SplitPayload(SYS_DataPointer_t *tail, SYS_DataPointer_t *head, SYS_DataLength_t offset)
{
    SYS_DbgAssert(NULL != tail && NULL != head, FAIL_SYS_SplitPayload_PtrPayloadIsNull);
    SYS_DbgAssert(!tail->isStatic && !head->isStatic, FAIL_SYS_SplitPayload_StaticPayloadIsNotSupported);
    SYS_DbgAssertComplex(FALSE == SYS_CheckPayload(tail), HALT_SYS_SplitPayload_TailIsNotEmpty);
    SYS_DbgAssertComplex(offset <= SYS_GetPayloadSize(head), HALT_SYS_SplitPayload_OffsetIsGreaterThanHeadSize);

    const bool isSuccess = SYS_MemoryManagerSplit(&head->block, &tail->block, offset, /*pInterBlockId*/ NULL);
    SYS_DbgAssertLog(isSuccess, WARN_SYS_SplitPayload_Failed);
    return isSuccess;
}


/*
 * Splits the linked chain of memory blocks at the given block id.
 */
void SYS_SplitLinked(SYS_DataPointer_t *pChain, SYS_DataPointer_t *pTrailer)
{
    SYS_DbgAssert(NULL != pChain && NULL != pTrailer, HALT_SYS_SplitLinked__NullPointer);
    SYS_DbgAssertComplex(SYS_CheckPayload(pChain), HALT_SYS_SplitLinked__EmptyChain);
    /* Trailer is allowed to be empty. */

    const SYS_DataLength_t chainLength = SYS_GetPayloadSize(pChain);
    const SYS_DataLength_t trailerLength = SYS_GetPayloadSize(pTrailer);
    SYS_DataPointer_t border = SYS_EMPTY_PAYLOAD;
    const bool result = SYS_SplitPayload(/*tail*/ &border, /*head*/ pChain, /*offset*/ chainLength - trailerLength);
    SYS_DbgAssert(result, HALT_SYS_SplitLinked__FailedToSplit);
    SYS_DbgAssertComplex(SYS_IsEqualPayload(pTrailer, &border), HALT_SYS_SplitLinked__UnexpectedTrailer);
}
#endif /* defined(_MEMORY_MANAGER_) */


/*
 * Resizes the payload size when it does not require additional blocks.
 */
void SYS_ResizePayload(SYS_DataPointer_t *const pPayload, const SYS_DataLength_t size)
{
    SYS_DbgAssert(NULL != pPayload, HALT_SYS_ResizePayload__NullPointer);

#if defined(_MEMORY_MANAGER_)
    const bool result = SYS_MemoryManagerResize(pPayload->block, size);
    SYS_DbgAssert(result, HALT_SYS_ResizePayload__FailedToResize);
#else
    (void)size;
    SYS_DbgHalt(HALT_SYS_ResizePayload__NotSupported);
#endif
}


#if defined(_MEMORY_MANAGER_)
/*
 * Constructs a fragmentation descriptor.
 */
bool SYS_FragDescrConstruct(SYS_FragDescr_t *const pFragDescr)
{
    SYS_DbgAssert(NULL != pFragDescr, HALT_SYS_FragDescrConstruct_NullFragDescr);

    memset(pFragDescr, 0, sizeof(*pFragDescr));

    size_t i = 0;       /* Index over the array of preallocated blocks. */
    do {
        /* Try to allocate blocks each of size 1 byte. The size of 1 byte guarantees that the allocated chunk consists
         * of exactly one block. */
        if (MM_BAD_BLOCK_ID == (pFragDescr->interBlock[i] = SYS_MemoryManagerAlloc(1)))
            break;
    } while (++i < ARRAY_SIZE(pFragDescr->interBlock));

    if (i < ARRAY_SIZE(pFragDescr->interBlock)) {
        /* If not all blocks were successfully allocated, free all the preallocated blocks and return failure status.
         * Notice that if all block were allocated, 'i' will be equal to the number of allocated blocks; otherwise, 'i'
         * will be equal to the index of the first block that was not allocated, and blocks with indexes 0..(i-1) must
         * be dismissed (as soon as they all were allocated). */
        while (i > 0)
            SYS_MemoryManagerFree(pFragDescr->interBlock[--i]);
        return FALSE;
    }

    return TRUE;
}


/*
 * Destructs a fragmentation descriptor.
 */
void SYS_FragDescrDestruct(SYS_FragDescr_t *const pFragDescr)
{
    SYS_DbgAssert(NULL != pFragDescr, HALT_SYS_FragDescrDestruct_NullFragDescr);
# ifdef _DEBUG_PAYLOAD_FRAG_
    SYS_DbgAssert(MM_BAD_BLOCK_ID == pFragDescr->fragment, HALT_SYS_FragDescrDestruct_BusyFragDescr);
# endif

    for (size_t i = 0; i < ARRAY_SIZE(pFragDescr->interBlock); i++)
        SYS_MemoryManagerFree(pFragDescr->interBlock[i]);
}


/*
 * Extracts a fragment from a payload.
 */
void SYS_FragmentExtract(SYS_DataPointer_t *const pPayload, SYS_DataPointer_t *const pFragment,
        const SYS_DataLength_t offset, const SYS_DataLength_t len, SYS_FragDescr_t *const pFragDescr)
{
    SYS_DbgAssert(NULL != pPayload && NULL != pFragment && NULL != pFragDescr, HALT_SYS_FragmentExtract_NullParam);
    SYS_DbgAssertComplex(SYS_CheckPayload(pPayload), HALT_SYS_FragmentExtract_EmptyPayload);
    SYS_DbgAssertComplex(!SYS_CheckPayload(pFragment), HALT_SYS_FragmentExtract_BusyFragment);
    SYS_DbgAssert(len > 0, HALT_SYS_FragmentExtract_ZeroLen);
    SYS_DbgAssertComplex(offset + len <= SYS_GetPayloadSize(pPayload), HALT_SYS_FragmentExtract_OffsetOverflow);
# ifdef _DEBUG_PAYLOAD_FRAG_
    SYS_DbgAssert(MM_BAD_BLOCK_ID == pFragDescr->fragment, HALT_SYS_FragmentExtract_BusyFragDescr);
# endif

    MM_ChunkId_t tailId = MM_BAD_BLOCK_ID;      /* Chunk Id of the temporary tail part. */
    if (!(  /* All the called functions shall return TRUE. Otherwise it's a failure. */
            SYS_MemoryManagerSplit(&pPayload->block, &pFragment->block, offset, &pFragDescr->interBlock[0]) &&
            SYS_MemoryManagerSplit(&pFragment->block, &tailId, len, &pFragDescr->interBlock[1]) &&
            SYS_MemoryManagerAppend(&pPayload->block, tailId, /*pExhaustBlockId*/ NULL)))
        SYS_DbgHalt(HALT_SYS_FragmentExtract_Failed);

# ifdef _DEBUG_PAYLOAD_FRAG_
    pFragDescr->payload = pPayload->block;
    pFragDescr->fragment = pFragment->block;
    pFragDescr->offset = offset;
    SYS_DbgAssertComplex(IMP(!SYS_CheckPayload(pPayload), 0 == offset), HALT_SYS_FragmentExtract_EmptyRemainingPayload);
# endif
}


/*
 * Returns a fragment back into the payload from which it was previously extracted.
 */
void SYS_FragmentReturn(SYS_DataPointer_t *const pPayload, SYS_DataPointer_t *const pFragment,
        const SYS_DataLength_t offset, SYS_FragDescr_t *const pFragDescr)
{
    SYS_DbgAssert(NULL != pPayload && NULL != pFragment && NULL != pFragDescr, HALT_SYS_FragmentReturn_NullParam);
    SYS_DbgAssertComplex(IMP(!SYS_CheckPayload(pPayload), 0 == offset), HALT_SYS_FragmentReturn_EmptyRemainingPayload);
    SYS_DbgAssertComplex(SYS_CheckPayload(pFragment), HALT_SYS_FragmentReturn_EmptyFragment);
    SYS_DbgAssertComplex(offset <= SYS_GetPayloadSize(pPayload), HALT_SYS_FragmentReturn_OffsetOverflow);
# ifdef _DEBUG_PAYLOAD_FRAG_
    SYS_DbgAssert(MM_BAD_BLOCK_ID != pFragDescr->fragment, HALT_SYS_FragmentExtract_IdleFragDescr);
    SYS_DbgAssert(pPayload->block == pFragDescr->payload && pFragment->block == pFragDescr->fragment &&
            offset == pFragDescr->offset , HALT_SYS_FragmentReturn_InvalidFragDescr);
# endif

    MM_ChunkId_t tailId = MM_BAD_BLOCK_ID;                      /* Chunk Id of the temporary tail part. */
    MM_ChunkId_t dontAllocateInterBlock = MM_BAD_BLOCK_ID;      /* Instructs Split not to allocate the inter-block. */
    if (!(  /* All the called functions shall return TRUE. Otherwise it's a failure. */
            SYS_MemoryManagerSplit(&pPayload->block, &tailId, offset, /*pInterBlockId*/ &dontAllocateInterBlock) &&
            SYS_MemoryManagerAppend(&pPayload->block, pFragment->block, &pFragDescr->interBlock[0]) &&
            SYS_MemoryManagerAppend(&pPayload->block, tailId, &pFragDescr->interBlock[1])))
        SYS_DbgHalt(HALT_SYS_FragmentReturn_Failed);
    *pFragment = SYS_EMPTY_PAYLOAD;

# ifdef _DEBUG_PAYLOAD_FRAG_
    pFragDescr->payload = MM_BAD_BLOCK_ID;
    pFragDescr->fragment = MM_BAD_BLOCK_ID;
    pFragDescr->offset = 0;
# endif
}


/*
 * Inserts a newly received fragment into the payload composed from previously received fragments.
 */
void SYS_FragmentInsert(SYS_DataPointer_t *const pPayload, SYS_DataPointer_t *const pFragment,
        const SYS_DataLength_t offset)
{
    SYS_DbgAssert(NULL != pPayload && NULL != pFragment, HALT_SYS_FragmentInsert_NullParam);
    SYS_DbgAssertComplex(IMP(!SYS_CheckPayload(pPayload), 0 == offset), HALT_SYS_FragmentInsert_EmptyPayload);
    SYS_DbgAssertComplex(SYS_CheckPayload(pFragment), HALT_SYS_FragmentInsert_EmptyFragment);
    SYS_DbgAssertComplex(offset <= SYS_GetPayloadSize(pPayload), HALT_SYS_FragmentInsert_OffsetOverflow);

    MM_ChunkId_t tailId = MM_BAD_BLOCK_ID;                      /* Chunk Id of the temporary tail part. */
    MM_ChunkId_t dontAllocateInterBlock = MM_BAD_BLOCK_ID;      /* Instructs Split not to allocate the inter-block. */
    if (!(  /* All the called functions shall return TRUE. Otherwise it's a failure. */
            SYS_MemoryManagerSplit(&pPayload->block, &tailId, offset, /*pInterBlockId*/ &dontAllocateInterBlock) &&
            SYS_MemoryManagerAppend(&pPayload->block, pFragment->block, /*pExhaustBlockId*/ NULL) &&
            SYS_MemoryManagerAppend(&pPayload->block, tailId, /*pExhaustBlockId*/ NULL)))
        SYS_DbgHalt(HALT_SYS_FragmentInsert_Failed);
    *pFragment = SYS_EMPTY_PAYLOAD;
}

#endif /* _MEMORY_MANAGER_ */


/* eof bbSysPayload.c */