/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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
 ******************************************************************************
/*****************************************************************************
 *
 * FILENAME: $Workfile: trunk/stack/common/System/include/bbSysMemMan.h $
 *
 * DESCRIPTION:
 *   This is the header file for the Memory Manager component.
 *
 * $Revision: 1887 $
 * $Date: 2014-03-25 09:31:48Z $
 *
 ****************************************************************************************/
#ifndef _BB_SYS_MEM_MAN_H
#define _BB_SYS_MEM_MAN_H

/************************* INCLUDES ***********************************************************************************/
#include "bbSysBasics.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Data type for dynamic memory chunk identifier.
 * \details The identifier of the head block in the sequence is used as the identifier of the dynamic memory chunk.
 */
typedef uint16_t  MM_ChunkId_t;

/**//**
 * \brief   Invalid dynamic memory block identifier.
 */
#define MM_BAD_BLOCK_ID  (0)

/**//**
 * \brief   Data type for size of chunk of dynamic memory.
 */
typedef uint32_t  MM_Size_t;

/**//**
 * \brief   Size of a single block of dynamic memory, in bytes.
 */
#define MM_BLOCK_SIZE  (16)

/************************* VALIDATIONS ********************************************************************************/
/* Validate definition of the single dynamic memory block size, in bytes.
 */
#if (MM_BLOCK_SIZE != 16)
# error The single dynamic memory block size given with MM_BLOCK_SIZE must be equal to 16 and must not be redefined.
# undef  MM_BLOCK_SIZE
# define MM_BLOCK_SIZE    (16)
#endif

/* Validate definition of the dynamic memory pool size, in bytes.
 */
#if !defined(_MEMORY_MANAGER_)
# define MM_POOL_SIZE   (MM_BLOCK_SIZE)
#endif
#if !defined(MM_POOL_SIZE)
# error Missed definition of the dynamic memory pool size. Define it with MM_POOL_SIZE in the project configuration.
# define MM_POOL_SIZE   (MM_BLOCK_SIZE)
#elif DEFINED_EMPTY(MM_POOL_SIZE)
# error The MM_POOL_SIZE must be assigned with a numeric value giving the dynamic memory pool size, in bytes.
# undef  MM_POOL_SIZE
# define MM_POOL_SIZE   (MM_BLOCK_SIZE)
#elif ((MM_POOL_SIZE) / (MM_BLOCK_SIZE) < 1)
# error The dynamic memory pool size given with MM_POOL_SIZE, in bytes, must be at least one block size (16 bytes).
# undef  MM_POOL_SIZE
# define MM_POOL_SIZE   (MM_BLOCK_SIZE)
#elif ((MM_POOL_SIZE) > (MM_BLOCK_SIZE) * (UINT16_MAX))
# warning The dynamic memory pool size given with MM_POOL_SIZE, in bytes, was limited to 1048560 bytes.
#elif ((MM_POOL_SIZE) != (MM_POOL_SIZE) / (MM_BLOCK_SIZE) * (MM_BLOCK_SIZE))
# warning The dynamic memory pool size given with MM_POOL_SIZE, in bytes, was reduced to the nearest multiple of 16.
#endif

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Macro defining the total number of memory blocks in the dynamic memory pool and the maximum value for the
 *  identifier of a dynamic memory block.
 * \details Dynamic memory blocks are enumerated in the range from 1 to MM_MAX_BLOCK_ID. Zero value is used for Null
 *  Block that is not used for storing data.
 * \details For all cases the MM_MAX_BLOCK_ID is limited at least to 65535.
 */
#define MM_MAX_BLOCK_ID    (MIN(((MM_POOL_SIZE) / (MM_BLOCK_SIZE)), UINT16_MAX))

/**//**
 * \brief   Data type representing the packed value of the identifier of a dynamic memory block.
 * \details This data type is used by Memory Manager for linking blocks sequentially into chunks of arbitrary size.
 * \details The last block in a chain points to the Null Block.
 */
#if ((MM_MAX_BLOCK_ID) <= (UINT8_MAX))
typedef uint8_t   MM_Link_t;
#else
typedef uint16_t  MM_Link_t;
SYS_DbgAssertStatic((MM_MAX_BLOCK_ID) <= (UINT16_MAX));
#endif

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Enumeration of directions to copy data between a dynamic memory chunk and a plain array of bytes.
 */
typedef enum _MM_CopyDirection_t {
    MM_COPY_FROM_ARRAY_TO_CHUNK,        /*!< Copy data from the plain array to the dynamic memory chunk. */
    MM_COPY_FROM_CHUNK_TO_ARRAY,        /*!< Copy data from the dynamic memory chunk to the plain array. */
} MM_CopyDirection_t;

/**//**
 * \brief   Copies data from the given plain array to the specified memory chunk.
 * \param[in]   dstChunkId      Identifier of the destination memory chunk.
 * \param[in]   pSrcArray       Pointer to the source plain array of bytes.
 * \param[in]   count           Number of bytes to copy.
 * \param[in]   offset          Offset, in bytes, from the memory chunk origin.
 * \return  The actual number of bytes copied.
 */
#define SYS_MemoryManagerCopyFrom(dstChunkId, pSrcArray, count, offset)\
        SYS_MemoryManagerCopyArray(pSrcArray, dstChunkId, count, offset, MM_COPY_FROM_ARRAY_TO_CHUNK)

/**//**
 * \brief   Copies data to the given plain array from the specified memory chunk.
 * \param[in]   pDstArray       Pointer to the destination plain array of bytes.
 * \param[in]   srcChunkId      Identifier of the source memory chunk.
 * \param[in]   count           Number of bytes to copy.
 * \param[in]   offset          Offset, in bytes, from the memory chunk origin.
 * \return  The actual number of bytes copied.
 */
#define SYS_MemoryManagerCopyTo(pDstArray, srcChunkId, count, offset)\
        SYS_MemoryManagerCopyArray(pDstArray, srcChunkId, count, offset, MM_COPY_FROM_CHUNK_TO_ARRAY)

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \brief   Initializes Memory Manager internal structures at the application startup.
 * \details This function must be called only once at the application startup.
 */
SYS_PUBLIC void SYS_MemoryManagerInit(void);

/**//**
 * \brief   Allocates a dynamic memory chunk of the requested size and returns the identifier of its leading block.
 * \param[in]   size    Size of the requested memory chunk, in bytes.
 * \return  Chunk Id of the allocated memory chunk; or the Null Block Id if no memory was allocated.
 */
SYS_PUBLIC MM_ChunkId_t SYS_MemoryManagerAlloc(const MM_Size_t  size);

/**//**
 * \brief   Allocates a dynamic memory chunk, assigns it with the copy of the given source memory chunk and returns the
 *  identifier of the leading block of the newly allocated chunk.
 * \param[in]   chunkId     Identifier of the source memory chunk.
 * \return  Chunk Id of the allocated memory chunk; or the Null Block Id if no memory was allocated.
 * \details The allocated memory chunk, if allocated, is assigned one-to-one with the content of the given source memory
 *  chunk. No inter-block squeezing is performed.
 */
SYS_PUBLIC MM_ChunkId_t SYS_MemoryManagerDuplicate(const MM_ChunkId_t  chunkId);

/**//**
 * \brief   Frees the given memory chunk.
 * \param[in]   chunkId     Identifier of the memory chunk to be freed.
 * \return  TRUE if operation was performed successfully; FALSE otherwise.
 * \details The \c chunkId must be the leading block of the chunk.
 */
SYS_PUBLIC bool SYS_MemoryManagerFree(const MM_ChunkId_t  chunkId);

/**//**
 * \brief   Frees the head part of the specified size of the given memory chunk preserving and returning its remaining
 *  tail part.
 * \param[in/out]   pChunkId    Pointer to the truncated memory chunk Id; finally is assigned with the chunk Id of the
 *  preserved tail part.
 * \param[in]       headSize    Size of the head part of the chunk to be freed, in bytes.
 * \return  TRUE if operation was performed successfully; FALSE otherwise.
 * \details The \c chunkId must be the leading block of the chunk.
 * \details If the \p headSize equals zero, nothing is deleted.
 * \details If the \p headSize is equal to or greater than the chunk size, the whole chunk is frees. In this case the
 *  \c chunkId is assigned with the Null Block Id.
 */
SYS_PUBLIC bool SYS_MemoryManagerFreeHead(MM_ChunkId_t *const  pChunkId, const MM_Size_t  headSize);

/**//**
 * \brief   Frees the tail part of the given memory chunk preserving the specified size of its head part.
 * \param[in]   chunkId     Identifier of the memory chunk to be partially freed.
 * \param[in]   headSize    Size of the head part of the chunk to be preserved, in bytes.
 * \return  TRUE if operation was performed successfully; FALSE otherwise.
 * \details If the \p headSize equals zero, the whole chunk is freed. In this case the \p chunkId must be the leading
 *  block of the chunk.
 * \details If the \p headSize is greater than zero, the \p chunkId does not need necessarily to be the leading block
 *  of the chunk.
 * \details If the \p headSize is equal to or greater than the chunk size (starting with the block \p chunkId), nothing
 *  is deleted.
 */
SYS_PUBLIC bool SYS_MemoryManagerFreeTail(const MM_ChunkId_t  chunkId, const MM_Size_t  headSize);

/**//**
 * \brief   Splits the given memory chunk at the specified offset and returns Ids of the detached head and tail parts.
 * \param[in/out]   pHeadChunkId    Pointer to the split memory chunk Id; finally is assigned with the chunk Id of the
 *  detached head part.
 * \param[out]      pTailChunkId    Pointer to the detached tail memory chunk Id.
 * \param[in]       headSize        Size of the prospective head part of the chunk, in bytes.
 * \return  TRUE if operation was performed successfully; FALSE otherwise.
 * \details If the operation has been completed successfully, the TRUE status is returned, the \c tailChunkId is
 *  assigned with the chunk Id of the detached tail part, and the \c headChunkId is assigned with the chunk Id of the
 *  detached head part. Indeed the \c headChunkId is either preserved with its original value if the head part is not
 *  empty, or is assigned with the Null Block Id otherwise (the second case takes place when the \p headSize equals to
 *  zero and the \c headChunkId is the leading block of split chunk). The \c tailChunkId is assigned either with the
 *  chunk Id of the detached tail part, or with the Null Block Id if the tail part is empty.
 * \details For the split operation to complete successfully one free block may be needed (namely in the case when the
 *  cut point lies inside a block but not on the block boundary). If this additional block is needed and if there are no
 *  free blocks at the moment, the split operation fails and the FALSE status is returned; in this case the
 *  \c tailChunkId is assigned with the Null Block Id, the \c headChunkId preserves its original value and the memory
 *  chunk is kept unchanged.
 */
SYS_PUBLIC bool SYS_MemoryManagerSplit(MM_ChunkId_t *const  pHeadChunkId, MM_ChunkId_t *const  pTailChunkId,
        const MM_Size_t  headSize);

/**//**
 * \brief   Appends the tail memory chunk to the head memory chunk.
 * \param[in/out]   pHeadChunkId        Pointer to the head memory chunk Id.
 * \param[in]       tailChunkId         Identifier of the tail memory chunk.
 * \return  TRUE if operation was performed successfully; FALSE otherwise.
 * \details For two chunks to become joined they must be nonempty and have valid format. Both or one of \c headChunkId
 *  and \p tailChunkId may be assigned with the Null Block Id (the corresponding chunk is considered empty); in such
 *  case respectively the Null Block or the second nonempty chunk is accepted as the resulting (joined) chunk. The first
 *  block Id of the joined chunk is returned in the \c headChunkId pointed by \p pHeadChunkId.
 * \details The \p tailChunkId, if the tail chunk is not empty, must be the leading block of the tail chunk, otherwise
 *  operation fails. Hence, the \c headChunkId is not required to be the leading block of the head chunk. After joining,
 *  if the head chunk was not empty, the formerly leading block of the tail chunk loses its Leading Block status; but it
 *  preserves its Leading Block status if the head chunk was empty.
 * \details The joined head and tail chunks must be different, if they are not empty; otherwise operation fails.
 * \details Both chunks must have busy (allocated) status, if they are not empty; otherwise (i.e., if either
 *  \c headChunkId or \p tailChunkId specifies a free block) operation fails.
 */
SYS_PUBLIC bool SYS_MemoryManagerAppend(MM_ChunkId_t *const  pHeadChunkId, const MM_ChunkId_t  tailChunkId);

/**//**
 * \brief   Returns the size of the specified memory chunk, in bytes.
 * \param[in]   chunkId     Identifier of the chunk.
 * \return  Size of the chunk, in bytes; or zero if chunk is not allocated.
 * \details If a chunk is allocated, its size may not be equal to zero.
 */
SYS_PUBLIC MM_Size_t SYS_MemoryManagerGetSize(const MM_ChunkId_t  chunkId);

/**//**
 * \brief   Copies data between the given plain array and the specified memory chunk.
 * \param[in]   pArray          Pointer to the plain array of bytes.
 * \param[in]   chunkId         Identifier of the memory chunk.
 * \param[in]   count           Number of bytes to copy.
 * \param[in]   offset          Offset, in bytes, from the memory chunk origin.
 * \param[in]   direction       Direction of the copy operation, either from the plain array to the dynamic memory chunk
 *  or in the reverse direction.
 * \return  The actual number of bytes copied.
 * \details The specified dynamic memory chunk must be allocated prior to perform data copying.
 * \details If the memory chunk includes blocks that were allocated partially, this function takes into account the
 *  actually allocated size of each particular block in the chain and preserves the chunk's structure.
 * \details Offset is applied only to the dynamic memory chunk but not to the plain array.
 * \details If the total size of the memory chunk decremented for the \p offset bytes is less than the number of bytes
 *  to be copied, the copy operation is performed partially until the memory chunk size allows copying. In this case the
 *  actual number of bytes returned by this function will be less than the \p count. The plain array of bytes pointed
 *  with \p pArray is considered to have size greater or equal to the \p count.
 */
SYS_PUBLIC MM_Size_t SYS_MemoryManagerCopyArray(void *const  pArray, const MM_ChunkId_t  chunkId,
        const MM_Size_t  count, const MM_Size_t  offset, const MM_CopyDirection_t  direction);

/**//**
 * \brief   Copies data between two memory chunks.
 * \param[in]   dstChunkId      Identifier of the destination memory chunk.
 * \param[in]   srcChunkId      Identifier of the source memory chunk.
 * \param[in]   count           Number of bytes to copy.
 * \return  The actual number of bytes copied.
 * \details Both the destination and the source dynamic memory chunks must be allocated prior to perform data copying.
 * \details If a memory chunk includes blocks that were allocated partially, this function takes into account the
 *  actually allocated size of each particular block in the chain - i.e., this function preserves the chunks' structure.
 * \details If the total size of either of two memory chunks is less than the number of bytes to be copied, the copy
 *  operation is performed partially until the memory chunk size allows copying. In this case the actual number of bytes
 *  returned by this function will be less than the \p count.
 */
SYS_PUBLIC MM_Size_t SYS_MemoryManagerCopyChunk(const MM_ChunkId_t  dstChunkId, const MM_ChunkId_t  srcChunkId,
        const MM_Size_t  count);

/**//**
 * \brief   Returns the starting address of the dynamic memory block by its Block Id.
 * \param[in]   blockId         Identifier of the dynamic memory block.
 * \return  Address of the first 1-byte cell of the dynamic memory block by its Block Id.
 * \details The \p blockId must be valid block Id and must not be the Null Block Id; otherwise NULL is returned.
 */
SYS_PUBLIC void* SYS_MemoryManagerGetMapBlockIdMemory(const MM_ChunkId_t  blockId);

/**//**
 * \brief   Get number of available free memory blocks
 * \return  number of available free memory blocks
 */
SYS_PUBLIC uint32_t SYS_MemoryManagerAvailableBlocks(void);

#endif /* _BB_SYS_MEM_MAN_H */
