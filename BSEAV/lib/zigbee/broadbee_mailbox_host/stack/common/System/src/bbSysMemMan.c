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
 * FILENAME: $Workfile: trunk/stack/common/System/src/bbSysMemMan.c $
 *
 * DESCRIPTION:
 *   This is the source file for the Memory Manager component.
 *
 * $Revision: 8515 $
 * $Date: 2015-09-29 12:22:36Z $
 *
 ****************************************************************************************/

/************************* INCLUDES ****************************************************/
#include "bbSysMemMan.h"

/************************* DEBUG CONFIGURATION ************************************************************************/
/* To configure the Memory Manager debug externally with the -D compiler directive use the following commands:
 * -DMM_DEBUG                           -- enables all debug levels: Failure, Halt (Error), Warning and Information,
 * -DMM_DEBUG=DBG_NONE                  -- disables all debug levels,
 * -DMM_DEBUG=DBG_ALL                   -- enables all debug levels,
 * -DMM_DEBUG=DBG_FAIL                  -- enables only the Failure level, disables all the others,
 * -DMM_DEBUG=DBG_FAIL|DBG_HALT         -- enables Failure and Halt levels, disables Warning and Information,
 * -DMM_DEBUG=                          -- this syntax must not be used (if used, enables all debug level).
 * To configure the Memory Manager debug internally with the #define preprocessor directive use the following commands:
 * #define MM_DEBUG                     -- enables all debug levels: Failure, Halt (Error), Warning and Information,
 * #define MM_DEBUG DBG_NONE            -- disables all debug levels,
 * #define MM_DEBUG DBG_ALL             -- enables all debug levels,
 * #define MM_DEBUG DBG_FAIL            -- enables only the Failure level, disables all the others,
 * #define MM_DEBUG DBG_FAIL|DBG_HALT   -- enables Failure and Halt levels, disables Warning and Information.
 */
#if DEFINED_EMPTY(MM_DEBUG)
# define DF(guard)     DBG_ASSERT_WRAPPER(ENABLED, guard)
# define DH(guard)     DBG_ASSERT_WRAPPER(ENABLED, guard)
# define DW(guard)     DBG_ASSERT_WRAPPER(ENABLED, guard)
# define DI(guard)     DBG_ASSERT_WRAPPER(ENABLED, guard)
#else
# if DBG_IS_FAIL_ENABLED(MM_DEBUG)
#  define DF(guard)    DBG_ASSERT_WRAPPER(ENABLED, guard)
# else
#  define DF(guard)    DBG_ASSERT_WRAPPER(DISABLED, guard)
# endif
# if DBG_IS_HALT_ENABLED(MM_DEBUG)
#  define DH(guard)    DBG_ASSERT_WRAPPER(ENABLED, guard)
# else
#  define DH(guard)    DBG_ASSERT_WRAPPER(DISABLED, guard)
# endif
# if DBG_IS_WARN_ENABLED(MM_DEBUG)
#  define DW(guard)    DBG_ASSERT_WRAPPER(ENABLED, guard)
# else
#  define DW(guard)    DBG_ASSERT_WRAPPER(DISABLED, guard)
# endif
# if DBG_IS_INFO_ENABLED(MM_DEBUG)
#  define DI(guard)    DBG_ASSERT_WRAPPER(ENABLED, guard)
# else
#  define DI(guard)    DBG_ASSERT_WRAPPER(DISABLED, guard)
# endif
#endif

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Data type representing the memory array used for a single dynamic memory block.
 * \details A single dynamic memory block is defined as the continuous array of 16 bytes.
 */
typedef uint8_t  MmBlock_t[MM_BLOCK_SIZE];

/**//**
 * \brief   Macro validating the identifier of a dynamic memory block.
 * \param[in]   blockId     Identifier to be validated.
 * \return  TRUE if the specified block identifier has valid value - i.e., it's in the range from 0 to MM_MAX_BLOCK_ID;
 *  or FALSE otherwise.
 * \note    Take into account that the Null Block Id (zero value) is also considered as the valid Block Id by this
 *  macro. Hence, some operations with blocks allow specifying the Null Block Id as the argument, while others don't. In
 *  the latter case the Null Block Id must be excluded from the allowed range with an additional test expression.
 * \details The expression <tt>(MM_BAD_BLOCK_ID) <= (blockId)</tt> is eliminated because it holds for arbitrary values
 *  of \p blockId having the unsigned data type.
 */
#define mmIsValidBlockId(blockId)    ((blockId) <= (MM_MAX_BLOCK_ID))

/**//**
 * \brief   Data type representing the actual Size of a single dynamic memory block, in bytes, and the block Status.
 * \details A dynamic memory block may be either free (unused) of busy (allocated). Free blocks are marked with zero in
 *  their Size & Status field. It means that a free block has zero size (not allocated). To test if a block is free its
 *  Size & Status field may be tested as a boolean value: it resolves to FALSE for free blocks and non-FALSE for busy
 *  blocks.
 * \details Busy blocks are linked into chains forming dynamic memory chunks of arbitrary size. Each block in a chain
 *  may have its own specific size - from 1 to MM_BLOCK_SIZE bytes (busy block may not have zero size). Each chain has
 *  the leading block (the first one in the chain). The leading block of a chain is not linked to any other block; and
 *  inversely, if a chain involves more than one block, each of the subsequent blocks are linked to the previous one
 *  within the corresponding chain. To distinguish the leading block of a chain from the subsequent blocks its Size &
 *  Status field contains the flag "Leading Block" set to one, while other blocks have this flag cleared. Bit #7 is used
 *  for storing the flag "Leading Block". Notice that all free blocks have the Leading Block flag cleared.
 * \details The actual size of a block, in bytes, is stored in bits #0..#6 of the Size & Status field. For a nonleading
 *  block its actual size is numerically equal to its Size & Status field value (because the bit #7 is assigned to
 *  zero). For a leading block the bit #7 of its Size & Status field must be masked to extract the actual size value.
 */
typedef uint8_t  MmSize_t;

/**//**
 * \brief   Bit mask for the Size value within the Size & Status field.
 * \details The actual Size of a block, in bytes, is stored in bits #0..#6 of the Size & Status field.
 */
#define MM_SIZE_VALUE_MASK    (0x7F)

/**//**
 * \brief   Bit mask for the Leading Block flag within the Size & Status field.
 * \details The Leading Block flag is stored in the bit #7 of the Size & Status field.
 */
#define MM_LEADING_FLAG_MASK    (0x80)

/**//**
 * \brief   Macro returning the actual block size from its Size & Status field value.
 * \param[in]   sizeStatus      Value of the Size & Status field of a block.
 * \return  Actual size of the block, in bytes.
 * \details The actual size of a block, in bytes, is stored in bits #0..#6 of the Size & Status field.
 */
#define mmExtractSize(sizeStatus)    ((sizeStatus) & (MM_SIZE_VALUE_MASK))

/**//**
 * \brief   Macro testing if the given block is the Leading Block of a chain or a subsequent block.
 * \param[in]   sizeStatus      Value of the Size & Status field of a block.
 * \return  TRUE if the given block is the Leading Block of a chain; or FALSE if it's the one of subsequent blocks in
 *  the chain.
 */
#define mmIsLeadingBlock(sizeStatus)    ((sizeStatus) & (MM_LEADING_FLAG_MASK))

/**//**
 * \brief   Macro defining the Size & Status of a free (unused) dynamic memory block.
 * \details All the free blocks are linked into the global queue of free blocks. Nevertheless, each of free blocks is
 *  additionally signed with zero Size & Status in order to speed up validation of each particular block if it's
 *  currently free or busy (busy blocks are signed with nonzero Size & Status). Notice that a free block may not have
 *  the Leading Block flag set to one.
 */
#define MM_FREE_BLOCK    (0)

/**//**
 * \brief   Prohibited Size & Status code.
 * \details This code correspond to the free block with flag Leading Block set to one. Notice that a free block may not
 *  have the Leading Block flag set to one.
 */
#define MM_PROHIBITED_SIZE    ((MM_LEADING_FLAG_MASK) | (0))

/**//**
 * \brief   Macro validating the size of a dynamic memory block.
 * \param[in]   blockSize       Actual size of a dynamic memory block, in bytes.
 * \return  TRUE if the size value is valid - i.e., it's in the range from 0 to MM_BLOCK_SIZE; or FALSE otherwise.
 * \note    Take into account that zero size is also considered as the valid block size by this macro. Hence, some
 *  operations with blocks allow specifying zero size as the argument, while others don't. In the latter case zero size
 *  must be excluded from the allowed range with an additional test expression.
 * \details The expression <tt>0 <= (blockSize)</tt> is eliminated because it holds for arbitrary values of \p blockSize
 *  having the unsigned data type.
 * \details Using this macro one have to preprocess the Size & Status field of a block to extract the actual size of the
 *  block: for the leading block of a chain its Size & Status field bit #7 must be masked to obtain the actual size
 *  value stored in bits #0..#6; for the subsequent blocks their Size & Status field values may be used directly.
 */
#define mmIsValidSize(blockSize)    ((blockSize) <= (MM_BLOCK_SIZE))

/************************* STATIC FUNCTIONS PROTOTYPES ****************************************************************/
/**//**
 * \brief   Returns identifier of the next block linked to the given one.
 * \param[in]   blockId     Identifier of the block for which need to return the link to the next block.
 * \return  Block Id of the next block linked to the given one; or Null Block Id if the \p blockId specifies the last
 *  block in the chain of linked blocks.
 * \details The \p blockId may also be assigned with the Null Block Id. For this case this function returns identifier
 *  of the first free block in the global queue of free blocks, or zero if there are no free memory.
 */
#if !DF()
SYS_STATIC size_t mmGetNextBlock(const size_t  blockId);
#else
# define mmGetNextBlock(blockId)    ((const MM_Link_t)(mmLinks[blockId]))
#endif

/**//**
 * \brief   Links the second specified block to the first one.
 * \param[in]   blockId         Identifier of the block which to link the second block to.
 * \param[in]   nextBlockId     Identifier of the block which to be linked.
 * \details The \p blockId may also be assigned with the Null Block Id. For such case this function puts the block
 *  \p nextBlockId into the head of the global queue of free blocks. In debug build it is validated that \p nextBlockId
 *  is currently free.
 * \details If the \p nextBlockId is assigned with the Null Block Id, the block \p blockId becomes the last block in the
 *  corresponding chain of blocks.
 * \details If both \p blockId and \p nextBlockId are assigned with the Null Block Id, it makes the global queue of free
 *  blocks empty - i.e., the case of no free memory.
 */
#if !DF()
SYS_STATIC void mmSetNextBlock(const size_t  blockId, const size_t  nextBlockId);
#else
# define mmSetNextBlock(blockId, nextBlockId)    do { mmLinks[blockId] = (nextBlockId); } while (0)
#endif

/**//**
 * \brief   Sets the Leading Block flag for the specified block.
 * \param[in]   blockId         Identifier of the nonleading block block to be made leading.
 * \details The Leading Block flag is stored in the bit #7 of the Size & Status field of the block.
 */
#if !DF()
SYS_STATIC void mmMakeLeading(const size_t  blockId);
#else
# define mmMakeLeading(blockId)    do { mmSizes[blockId] |= (MM_LEADING_FLAG_MASK); } while (0)
#endif

/**//**
 * \brief   Clears the Leading Block flag for the specified block.
 * \param[in]   blockId         Identifier of the leading block block to be made nonleading.
 * \details The Leading Block flag is stored in the bit #7 of the Size & Status field of the block.
 */
#if !DF()
SYS_STATIC void mmMakeNonleading(const size_t  blockId);
#else
# define mmMakeNonleading(blockId)    do { mmSizes[blockId] &= ~(MM_LEADING_FLAG_MASK); } while (0)
#endif

/**//**
 * \brief   Checks if the specified block is linked to any other block.
 * \param[in]   blockId     Identifier of the block for which to return its status.
 * \return  TRUE if the given block is linked to any other block in the pool including the Null Block; or FALSE
 *  otherwise.
 * \details Linked block may not be freed or linked to another block. If a block is linked to a busy block it means that
 *  the first one continues the chain of blocks in a memory chunk; freeing such block would brake that chain and a
 *  multiple linkage will arise. If a block is linked to a free block (or to the Null Block playing the role of the hook
 *  for the global queue of all free blocks) it means that such block belongs to the queue of free blocks; freeing or
 *  forcedly linking such block to a different block would brake the queue of free blocks and originate a linkage loop
 *  or multiple linkage.
 */
#if !DF()
SYS_STATIC bool mmIsLinkedBlock(const size_t  blockId);
#endif

/**//**
 * \brief   Returns the Size & Status field of the given block.
 * \param[in]   blockId     Identifier of the block for which to return the Size & Status field.
 * \return  Size & Status field of the specified block.
 * \details The \p blockId must not be assigned with the Null Block Id which size and status are not defined.
 */
#if !DF()
SYS_STATIC MmSize_t mmGetSizeStatus(const size_t  blockId);
#else
# define mmGetSizeStatus(blockId)    ((const MmSize_t)(mmSizes[blockId]))
#endif

/**//**
 * \brief   Preallocates the specified part of the given block.
 * \param[in]   blockId     Identifier of the free block to be preallocated.
 * \param[in]   size        Size of the preallocated block, in bytes.
 * \details Allocation of a memory chunk (a chain of blocks) involves two steps: (1) preallocation of a chain of blocks
 *  having required total size, and (2) detaching the preallocated chain from the global queue of free blocks. This
 *  function performs the first step in this sequence for a single block.
 * \details The \p size value must be in the range from 1 to MM_BLOCK_SIZE. Allocation of a block having zero size is
 *  not allowed.
 * \details A newly preallocated block receives the Leading Block flag cleared. The first block in the preallocated
 *  chain will be signed as the Leading Block on the second step (when the preallocated chain will be granted).
 * \details Only free blocks may be allocated, busy blocks may not be (re-)allocated. The Null Block is not allowed for
 *  this function.
 */
#if !DF()
SYS_STATIC void mmPreallocBlock(const size_t  blockId, const MM_Size_t  size);
#else
# define mmPreallocBlock(blockId, size)    do { mmSizes[blockId] = (size); } while (0)
#endif

/**//**
 * \brief   Shrinks the specified block.
 * \param[in]   blockId     Identifier of the busy block to be shrunk.
 * \param[in]   newSize     New size to be set for the shrunk block, in bytes.
 * \details The new size must be greater than zero but not greater than the original size of the shrunk block. It's
 *  allowed for the new size to be equal to the original size.
 * \details This function preserves the Leading Block flag of the block.
 * \details Only busy block may be shrunk. The Null Block is not allowed for this function.
 */
#if !DF()
SYS_STATIC void mmShrinkBlock(const size_t  blockId, const MM_Size_t  newSize);
#else
# define mmShrinkBlock(blockId, newSize)\
        do { mmSizes[blockId] = (newSize | (mmSizes[blockId] & (MM_LEADING_FLAG_MASK))); } while (0)
#endif

/**//**
 * \brief   Abandons the specified block.
 * \param[in]   blockId     Identifier of the busy block to be abandoned.
 * \details Freeing of a memory chunk (a chain of blocks) involves two steps: (1) abandoning of the chain of blocks, and
 *  (2) collecting the abandoned blocks chain to the global queue of free blocks. This function performs the first step
 *  in this sequence for a single block.
 * \details Only busy blocks may be freed, an already free block may not be (re-)freed repeatedly. The Null Block is not
 *  allowed for this function.
 */
#if !DF()
SYS_STATIC void mmAbandonBlock(const size_t  blockId);
#else
# define mmAbandonBlock(blockId)    do { mmSizes[blockId] = (MM_FREE_BLOCK); } while (0)
#endif

/**//**
 * \brief   Grants the subchain of preallocated blocks starting with the first found free (preallocated) block and up to
 *  the specified one from the global queue of free blocks.
 * \param[in]   lastBlockId     Identifier of the last block in the subchain of preallocated blocks.
 * \return  Identifier of the leading block of the subchain of granted blocks.
 * \details Allocation of a memory chunk (a chain of blocks) involves two steps: (1) preallocation of a chain of blocks
 *  having required total size, and (2) detaching the preallocated chain from the global queue of free blocks. This
 *  function performs the second step in this sequence.
 * \details The first detached block is the first free block in the pool (the one that is linked to the Null Block),
 *  while the last detached block is given with \p lastBlockId.
 * \details The first block of the detached chain is signed with the Leading Block flag.
 * \note    Do not detach the last block from its subsequent block when calling this function, because that subsequent
 *  block shall become the first free block in the pool after this function completes.
 */
SYS_STATIC size_t mmGrantChain(const size_t  lastBlockId);

/**//**
 * \brief   Collects the subchain of abandoned blocks starting with the first and up to the last specified blocks and
 *  put them into the global queue of free blocks.
 * \param[in]   firstBlockId    Identifier of the first block in the subchain of abandoned blocks.
 * \param[in]   lastBlockId     Identifier of the last block in the subchain of abandoned blocks.
 * \details Freeing of a memory chunk (a chain of blocks) involves two steps: (1) abandoning of the chain of blocks, and
 *  (2) collecting the abandoned blocks chain to the global queue of free blocks. This function performs the second step
 *  in this sequence.
 */
SYS_STATIC void mmCollectChain(const size_t  firstBlockId, const size_t  lastBlockId);

/**//**
 * \brief   Rollbacks the memory chunk allocation.
 * \details This function is called in the case of lack of dynamic memory discovered during the blocks chain
 *  preallocation step. It must be called prior the preallocated chain is detached from the global queue of free blocks.
 */
SYS_STATIC void mmRollbackAlloc(void);

/************************* STATIC VARIABLES ***************************************************************************/
/**//**
 * \brief   Static array of memory blocks used for the dynamic memory pool.
 * \details This array involves MM_MAX_BLOCK_ID dynamic memory blocks. To enumerate them with the range from #1 to
 *  #MM_MAX_BLOCK_ID, the entry point will be shifted to one block. This virtual block #0 will not be used; zero
 *  Block Id is used as the identifier of the Null Block.
 */
SYS_MEMDEF MmBlock_t  mmBlocksArray[MM_MAX_BLOCK_ID];

/**//**
 * \brief   Static array of packed Block Ids used for linking blocks into chains forming memory chunks.
 * \details This array involves MM_MAX_BLOCK_ID plus one packed block identifiers, or Links. Links from #1 to
 *  #MM_MAX_BLOCK_ID are used for storing links of corresponding dynamic memory blocks. For example, if the element #N
 *  of this array is assigned with M, it means that after the block #N the block #M follows. If in the example above the
 *  element #N is assigned with zero, it means that the block #N is the last one in the corresponding memory chunk.
 * \details Link #0, corresponding to the Null Block, is used for linking all the free blocks into the global queue of
 *  free blocks. The Null Block itself is not used and may not be allocated; it means that no busy block may follow it -
 *  so, that's why the Null-Block's link may be used for different purposes as mentioned here. When a new chunk of
 *  memory is allocated the free blocks are picked up starting with that one pointed by the link #0. The last free block
 *  in the pool must point to the Null Block; it means the end of the chain of free blocks (the same as for chains of
 *  busy blocks). If there are no free blocks in the pool, the link #0 is assigned with zero.
 */
SYS_MEMDEF MM_Link_t    mmLinksArray[1 + MM_MAX_BLOCK_ID];

/**//**
 * \brief   Static array of Size & Status fields of blocks.
 * \details This array involves MM_MAX_BLOCK_ID values. To enumerate them with the range from 1 to MM_MAX_BLOCK_ID, the
 *  entry point will be shifted to one block. The value under #0 will not be used, it corresponds to the Null Block
 *  which is never allocated.
 * \details When a block is allocated, its actuals size - from 1 to MM_BLOCK_SIZE - is stored in the Size & Status
 *  field (an allocated block may not have zero size). Additionally, if a block is the leading block of a chunk, its
 *  flag "Leading Block" in the Size & Status field is set to one. Free blocks have zero in their Size & Status field.
 * \details Actual size of a block is stored in the Size & Status field bits #0..#6, while the bit #7 stores the Leading
 *  Block flag.
 */
SYS_MEMDEF MmSize_t  mmSizesArray[MM_MAX_BLOCK_ID];

/************************* STATIC CONSTANTS ***************************************************************************/
/**//**
 * \brief   Entry point to the dynamic memory pool.
 * \details Dynamic memory is indexed by blocks. Block Id must belong to the range from 1 to MM_MAX_BLOCK_ID.
 * \details Index zero must not be used as the Block Id; zero Block Id is used as the Null Block Identifier. There are
 *  no static memory allocated for the Null Block.
 */
SYS_STATIC MmBlock_t *const  mmBlocks = (((MmBlock_t*)mmBlocksArray) - 1);

/**//**
 * \brief   Entry point to the array of inter-block links.
 * \details Links from #1 to #MM_MAX_BLOCK_ID point to blocks following the corresponding working blocks in their
 *  respective memory chunks. The Link field of the trailing block of a chunk (or of the global queue of free blocks) is
 *  assigned with zero.
 * \details Link #0 points to the first free block in the pool. If there are no free blocks, the link #0 is set to zero.
 *
 * And these were written by my son Bogdan - yeah, sometimes he help me to write the code:
 * jkglgbkkdkv;. ;.nlbkvkvkcnjcjmvsssssssssssssaaaasaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
 * aaaaaaaaaaaaaaaaadi5kgt[xllllllxxxxxxxxxxxoz4o4ssssssssss!!!1папаh//lhki/.kllitrT
 */
SYS_STATIC MM_Link_t *const     mmLinks = (mmLinksArray);

/**//*
 * \brief   Entry point to the array of blocks Size & Status fields.
 * \details Size & Status fields from #1 to #MM_MAX_BLOCK_ID correspond to working blocks with the same identifiers.
 *  Index zero must not be used as the Block Id; zero Block Id is used as the Null Block Identifier which is never
 *  allocated and has no Size & Status field.
 * \details Size & Status field shows if the corresponding block is free or busy, if it's the leading block of a chunk
 *  or one of the subsequent blocks in the chain, and contains the actual allocated size of the block, in bytes. For
 *  free blocks the Size & Status field is set to zero. Leading Blocks have bit #7 set to one in their Size & Status
 *  field. The actual size of a block, in bytes, is stored in bits #0..#6.
 */
SYS_STATIC MmSize_t *const  mmSizes = (((MmSize_t*)mmSizesArray) - 1);

/************************* IMPLEMENTATION *****************************************************************************/
/* ---------------------------------------------------------------------------------------------------------------------
 * Initializes Memory Manager internal structures at the application startup.
 */
SYS_PUBLIC void SYS_MemoryManagerInit(void)
{
#if !DF() && !defined(_UNIT_TEST_)
    static bool  isInitialized;         /* Equals to FALSE at startup; then assigned with 0xDA. */
    SYS_DbgAssert(FALSE == isInitialized, FAIL_SYS_MemoryManagerInit_CalledRepeatedly);
    isInitialized = 0xDA;
#endif
    for (size_t blockId = MM_BAD_BLOCK_ID; blockId < MM_MAX_BLOCK_ID; blockId++)
        mmLinks[blockId] = blockId + 1;
    mmLinks[MM_MAX_BLOCK_ID] = MM_BAD_BLOCK_ID;
#if defined(_UNIT_TEST_)
    for (size_t blockId = 1; blockId <= MM_MAX_BLOCK_ID; blockId++)
        mmSizes[blockId] = MM_FREE_BLOCK;
#endif
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Allocates a dynamic memory chunk of the requested size and returns the identifier of its leading block.
 */
SYS_PUBLIC MM_ChunkId_t SYS_MemoryManagerAlloc(const MM_Size_t  size)
{
    SYS_DbgAssertLog(DW(size <= (MM_MAX_BLOCK_ID) * (MM_BLOCK_SIZE)), WARN_SYS_MemoryManagerAlloc_SizeExccedsPool);
    if (0 == size) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerAlloc_SizeIsZero);
        return MM_BAD_BLOCK_ID;
    }

    size_t  eachFreeBlockId = mmGetNextBlock(MM_BAD_BLOCK_ID);  /* Iterates over free blocks being allocated. */
    size_t  lastFreeBlockId = MM_BAD_BLOCK_ID;                  /* Finally will contain the last allocated block Id. */
    MM_Size_t  remSize = size;                      /* The remaining part of the requested size to be allocated. */
    while (remSize > 0 && MM_BAD_BLOCK_ID != eachFreeBlockId) {
        const MM_Size_t  allocSize = MIN(remSize, MM_BLOCK_SIZE);       /* Size of the current block to be allocated. */
        remSize -= allocSize;
        mmPreallocBlock(eachFreeBlockId, allocSize);
        lastFreeBlockId = eachFreeBlockId;
        eachFreeBlockId = mmGetNextBlock(eachFreeBlockId);
    }

    if (0 == remSize) {
        return mmGrantChain(lastFreeBlockId);
    }
    else {
        mmRollbackAlloc();
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerAlloc_NotEnoughMemory);
        return MM_BAD_BLOCK_ID;
    }
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Get number of available free memory blocks
 */
SYS_PUBLIC uint32_t SYS_MemoryManagerAvailableBlocks(void)
{
    MM_ChunkId_t freeBlockId = MM_BAD_BLOCK_ID;
    uint32_t cnt = 0;

    do
        freeBlockId = mmGetNextBlock(freeBlockId);
    while (MM_BAD_BLOCK_ID != freeBlockId && ++cnt);

    return cnt;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Allocates a dynamic memory chunk, assigns it with the copy of the given source memory chunk and returns the
 * identifier of the leading block of the newly allocated chunk.
 */
SYS_PUBLIC MM_ChunkId_t SYS_MemoryManagerDuplicate(const MM_ChunkId_t  chunkId)
{
    if (MM_BAD_BLOCK_ID == chunkId) {
        SYS_DbgAssertLog(DI(), INFO_SYS_MemoryManagerDuplicate_ChunkIsNull);
        return MM_BAD_BLOCK_ID;
    }
    if (!mmIsValidBlockId(chunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerDuplicate_ChunkIdIsInvalid);
        return MM_BAD_BLOCK_ID;
    }
    if (MM_FREE_BLOCK == mmGetSizeStatus(chunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerDuplicate_FirstBlockIsFree);
        return MM_BAD_BLOCK_ID;
    }

#if !DF()
    size_t  countSrcBlocks = MM_MAX_BLOCK_ID;                   /* Down-counter of blocks in the source chunk. */
#endif
    size_t  eachSrcBlockId = chunkId;                           /* Iterates over blocks in the source memory chunk. */
    size_t  eachDstBlockId = mmGetNextBlock(MM_BAD_BLOCK_ID);   /* Iterates over destination blocks being allocated. */
    size_t  lastDstBlockId = MM_BAD_BLOCK_ID;                   /* Finally will contain the last allocated block Id. */
    while (MM_BAD_BLOCK_ID != eachSrcBlockId && MM_BAD_BLOCK_ID != eachDstBlockId) {
        SYS_DbgAssert(DF(countSrcBlocks-- > 0), FAIL_SYS_MemoryManagerDuplicate_LoopFoundInSrcChunk);
        mmPreallocBlock(eachDstBlockId, mmExtractSize(mmGetSizeStatus(eachSrcBlockId)));
        memcpy(mmBlocks[eachDstBlockId], mmBlocks[eachSrcBlockId], MM_BLOCK_SIZE);
        lastDstBlockId = eachDstBlockId;
        eachSrcBlockId = mmGetNextBlock(eachSrcBlockId);
        eachDstBlockId = mmGetNextBlock(eachDstBlockId);
    }

    if (MM_BAD_BLOCK_ID == eachSrcBlockId) {
        return mmGrantChain(lastDstBlockId);
    }
    else {
        mmRollbackAlloc();
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerDuplicate_NotEnoughMemory);
        return MM_BAD_BLOCK_ID;
    }
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Frees the given memory chunk.
 */
SYS_PUBLIC bool SYS_MemoryManagerFree(const MM_ChunkId_t  chunkId)
{
    if (MM_BAD_BLOCK_ID == chunkId) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerFree_ChunkIsNull);
        return TRUE;
    }
    if (!mmIsValidBlockId(chunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerFree_ChunkIdIsInvalid);
        return FALSE;
    }
    const MmSize_t  firstBlockSizeStatus = mmGetSizeStatus(chunkId);        /* Size & Status of the first block. */
    if (MM_FREE_BLOCK == firstBlockSizeStatus) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerFree_FirstBlockIsFree);
        return FALSE;
    }
    if (!mmIsLeadingBlock(firstBlockSizeStatus)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerFree_FirstBlockIsNotLeading);
        return FALSE;
    }
    SYS_DbgAssert(DF(!mmIsLinkedBlock(chunkId)), FAIL_SYS_MemoryManagerFree_LeadingBlockIsLinked);

    size_t  eachBlockId = chunkId;      /* Iterates over blocks in the freed memory chunk. */
    size_t  lastBlockId;                /* Finally will contain the Id of the last freed block. */
    do {
        mmAbandonBlock(eachBlockId);
        lastBlockId = eachBlockId;
        eachBlockId = mmGetNextBlock(eachBlockId);
    } while (MM_BAD_BLOCK_ID != eachBlockId);

    mmCollectChain(chunkId, lastBlockId);
    return TRUE;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Frees the head part of the specified size of the given memory chunk preserving and returning its remaining tail part.
 */
SYS_PUBLIC bool SYS_MemoryManagerFreeHead(MM_ChunkId_t *const  pChunkId, const MM_Size_t  headSize)
{
    SYS_DbgAssert(DF(NULL != pChunkId), FAIL_SYS_MemoryManagerFreeHead_PtrChunkIdIsNull);

    const size_t  chunkId = *pChunkId;          /* Identifier of the memory chunk to be truncated. */
    if (MM_BAD_BLOCK_ID == chunkId) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerFreeHead_ChunkIsNull);
        return TRUE;
    }
    if (!mmIsValidBlockId(chunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerFreeHead_ChunkIdIsInvalid);
        return FALSE;
    }
    const MmSize_t  firstBlockSizeStatus = mmGetSizeStatus(chunkId);        /* Size & Status of the first block. */
    if (MM_FREE_BLOCK == firstBlockSizeStatus) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerFreeHead_FirstBlockIsFree);
        return FALSE;
    }
    if (!mmIsLeadingBlock(firstBlockSizeStatus)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerFreeHead_FirstBlockIsNotLeading);
        return FALSE;
    }
    SYS_DbgAssert(DF(!mmIsLinkedBlock(chunkId)), FAIL_SYS_MemoryManagerFreeHead_LeadingBlockIsLinked);

    if (0 == headSize) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerFreeHead_FreedHeadSizeIsZero);
        return TRUE;
    }

    MM_Size_t  remSize = headSize;              /* The remaining part of the requested size to be freed. */
    size_t  eachBlockId = chunkId;              /* Iterates over blocks in the freed head part of the chunk. */
    size_t  lastBlockId = MM_BAD_BLOCK_ID;      /* Finally will contain the Id of the last freed block. */
    do {
        const MmSize_t  blockSizeStatus = mmGetSizeStatus(eachBlockId);         /* Size & Status of current block. */
        SYS_DbgAssert(DF(MM_FREE_BLOCK != blockSizeStatus), FAIL_SYS_MemoryManagerFreeHead_FreeIntermediateBlockFound);
        const MM_Size_t  blockSize = mmExtractSize(blockSizeStatus);            /* Size of the current block. */
        if (remSize >= blockSize) {
            remSize -= blockSize;
            mmAbandonBlock(eachBlockId);
            lastBlockId = eachBlockId;
            eachBlockId = mmGetNextBlock(eachBlockId);
        }
        else {
            const MM_Size_t  preservedSize = (blockSize - remSize);     /* Size of the preserved part of the block. */
            mmShrinkBlock(eachBlockId, preservedSize);
            memmove(mmBlocks[eachBlockId], &(mmBlocks[eachBlockId][remSize]), preservedSize);
            break;
        }
    } while (remSize > 0 && MM_BAD_BLOCK_ID != eachBlockId);
    SYS_DbgAssertLog(DW(!(MM_BAD_BLOCK_ID == eachBlockId && remSize > 0)),
            WARN_SYS_MemoryManagerFreeHead_FreedHeadSizeIsGreaterThanChunkSize);
    SYS_DbgAssertLog(DI(!(MM_BAD_BLOCK_ID == eachBlockId && remSize == 0)),
            INFO_SYS_MemoryManagerFreeHead_FreedHeadSizeEqualsChunkSize);

    if (MM_BAD_BLOCK_ID != lastBlockId) {
        mmCollectChain(chunkId, lastBlockId);
        if (MM_BAD_BLOCK_ID != eachBlockId)
            mmMakeLeading(eachBlockId);
        *pChunkId = eachBlockId;
    }
    return TRUE;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Frees the tail part of the given memory chunk preserving the specified size of its head part.
 */
SYS_PUBLIC bool SYS_MemoryManagerFreeTail(const MM_ChunkId_t  chunkId, const MM_Size_t  headSize)
{
    if (0 == headSize) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerFreeTail_PreservedHeadSizeIsZero);
        return SYS_MemoryManagerFree(chunkId);
    }

    if (MM_BAD_BLOCK_ID == chunkId) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerFreeTail_ChunkIsNull);
        return TRUE;
    }
    if (!mmIsValidBlockId(chunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerFreeTail_ChunkIdIsInvalid);
        return FALSE;
    }
    if (MM_FREE_BLOCK == mmGetSizeStatus(chunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerFreeTail_FirstBlockIsFree);
        return FALSE;
    }

    MM_Size_t  remSize = headSize;          /* The remaining part of the requested size to be preserved. */
    size_t  eachBlockId = chunkId;          /* Iterates over blocks in the preserved head part of the chunk. */
    do {
        const MmSize_t  blockSizeStatus = mmGetSizeStatus(eachBlockId);         /* Size & Status of current block. */
        SYS_DbgAssert(DF(MM_FREE_BLOCK != blockSizeStatus), FAIL_SYS_MemoryManagerFreeTail_FreeIntermediateBlockFound);
        const MM_Size_t  blockSize = mmExtractSize(blockSizeStatus);            /* Size of the current block. */
        if (remSize > blockSize) {
            remSize -= blockSize;
            eachBlockId = mmGetNextBlock(eachBlockId);
        }
        else {
            mmShrinkBlock(eachBlockId, remSize);
            const MM_ChunkId_t  tailBlockId = mmGetNextBlock(eachBlockId);      /* First block Id of the tail part. */
            mmSetNextBlock(eachBlockId, MM_BAD_BLOCK_ID);
            if (MM_BAD_BLOCK_ID != tailBlockId) {
                mmMakeLeading(tailBlockId);
                SYS_MemoryManagerFree(tailBlockId);
            }
            return TRUE;
        }
    } while (MM_BAD_BLOCK_ID != eachBlockId);

    SYS_DbgAssertLog(DW(remSize > 0), WARN_SYS_MemoryManagerFreeTail_PreservedHeadSizeIsGreaterThanChunkSize);
    SYS_DbgAssertLog(DW(remSize == 0), WARN_SYS_MemoryManagerFreeTail_PreservedHeadSizeEqualsThanChunkSize);
    return TRUE;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Splits the given memory chunk at the specified offset and returns Ids of the detached head and tail parts.
 */
SYS_PUBLIC bool SYS_MemoryManagerSplit(MM_ChunkId_t *const  pHeadChunkId, MM_ChunkId_t *const  pTailChunkId,
        const MM_Size_t  headSize)
{
    SYS_DbgAssert(DF(NULL != pTailChunkId && NULL != pHeadChunkId), FAIL_SYS_MemoryManagerSplit_PtrChunkIdIsNull);

    SYS_DbgAssertLog(DW(MM_BAD_BLOCK_ID == *pTailChunkId), WARN_SYS_MemoryManagerSplit_TailChunkIsNotNull);
    *pTailChunkId = MM_BAD_BLOCK_ID;

    const size_t  headChunkId = *pHeadChunkId;          /* Identifier of the memory chunk to be split. */
    if (MM_BAD_BLOCK_ID == headChunkId) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerSplit_HeadChunkIsNull);
        return TRUE;
    }
    if (!mmIsValidBlockId(headChunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerSplit_HeadChunkIdIsInvalid);
        return FALSE;
    }
    const MmSize_t  headFirstBlockSizeStatus = mmGetSizeStatus(headChunkId);    /* Size & Status of first head block. */
    if (MM_FREE_BLOCK == headFirstBlockSizeStatus) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerSplit_HeadFirstBlockIsFree);
        return FALSE;
    }

    if (0 == headSize) {
        if (!mmIsLeadingBlock(headFirstBlockSizeStatus)) {
            SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerSplit_HeadFirstBlockIsNotLeading);
            return FALSE;
        }
        SYS_DbgAssert(DF(!mmIsLinkedBlock(headChunkId)), FAIL_SYS_MemoryManagerSplit_HeadLeadingBlockIsLinked);
        *pTailChunkId = headChunkId;
        *pHeadChunkId = MM_BAD_BLOCK_ID;
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerSplit_HeadSizeIsZero);
        return TRUE;
    }

    size_t  eachBlockId = headChunkId;          /* Identifier of the currently processed block. */
    MM_Size_t  remSize = headSize;              /* Remaining number of bytes to be kept in the head part. */
    do {
        const MmSize_t  blockSizeStatus = mmGetSizeStatus(eachBlockId);         /* Size & Status of current block. */
        SYS_DbgAssert(DF(MM_FREE_BLOCK != blockSizeStatus), FAIL_SYS_MemoryManagerSplit_FreeIntermediateBlockFound);
        const MM_Size_t  blockSize = mmExtractSize(blockSizeStatus);            /* Size of the current block. */
        if (remSize > blockSize) {
            remSize -= blockSize;
            eachBlockId = mmGetNextBlock(eachBlockId);
            continue;
        }
        if (remSize == blockSize) {
            const size_t  tailChunkId = mmGetNextBlock(eachBlockId);    /* Identifier of the first tail chunk block. */
            if (MM_BAD_BLOCK_ID != tailChunkId) {
                *pTailChunkId = tailChunkId;
                mmMakeLeading(tailChunkId);
            }
            else {
                SYS_DbgAssertLog(DI(), INFO_SYS_MemoryManagerSplit_HeadSizeEqualsChunkSize);
            }
        }
        else {
            const MM_Size_t  headPartSize = remSize;                    /* Size of the head part of the split block. */
            const MM_Size_t  tailPartSize = (blockSize - remSize);      /* Size of the tail part of the split block. */
            const size_t  newBlockId = SYS_MemoryManagerAlloc(tailPartSize);    /* Id of the additional tail block. */
            if (MM_BAD_BLOCK_ID == newBlockId) {
                SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerSplit_NotEnoughMemory);
                return FALSE;
            }
            *pTailChunkId = newBlockId;
            memcpy(mmBlocks[newBlockId], &(mmBlocks[eachBlockId][headPartSize]), tailPartSize);
            mmSetNextBlock(newBlockId, mmGetNextBlock(eachBlockId));
            mmShrinkBlock(eachBlockId, headPartSize);
        }
        mmSetNextBlock(eachBlockId, MM_BAD_BLOCK_ID);
        return TRUE;
    } while (MM_BAD_BLOCK_ID != eachBlockId);

    SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerSplit_HeadSizeIsGreaterThanChunkSize);
    return TRUE;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Appends the tail memory chunk to the head memory chunk.
 */
SYS_PUBLIC bool SYS_MemoryManagerAppend(MM_ChunkId_t *const  pHeadChunkId, const MM_ChunkId_t  tailChunkId)
{
    SYS_DbgAssert(DF(NULL != pHeadChunkId), FAIL_SYS_MemoryManagerAppend_PtrChunkIdIsNull);

    if (MM_BAD_BLOCK_ID == tailChunkId) {
        SYS_DbgAssertLog(DI(), INFO_SYS_MemoryManagerAppend_TailChunkIsNull);
        return TRUE;
    }
    if (!mmIsValidBlockId(tailChunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerAppend_TailChunkIdIsInvalid);
        return FALSE;
    }
    const MmSize_t  tailFirstBlockSizeStatus = mmGetSizeStatus(tailChunkId);    /* Size & Status of first tail block. */
    if (MM_FREE_BLOCK == tailFirstBlockSizeStatus) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerAppend_TailFirstBlockIsFree);
        return FALSE;
    }
    if (!mmIsLeadingBlock(tailFirstBlockSizeStatus)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerAppend_TailFirstBlockIsNotLeading);
        return FALSE;
    }
    SYS_DbgAssert(DF(!mmIsLinkedBlock(tailChunkId)), FAIL_SYS_MemoryManagerAppend_TailLeadingBlockIsLinked);

    const MM_ChunkId_t  headChunkId = *pHeadChunkId;        /* Identifier of the head memory chunk. */
    if (MM_BAD_BLOCK_ID == headChunkId) {
        *pHeadChunkId = tailChunkId;
        SYS_DbgAssertLog(DI(), INFO_SYS_MemoryManagerAppend_HeadChunkIsNull);
        return TRUE;
    }
    if (!mmIsValidBlockId(headChunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerAppend_HeadChunkIdIsInvalid);
        return FALSE;
    }
    const MmSize_t  headFirstBlockSizeStatus = mmGetSizeStatus(headChunkId);    /* Size & Status of first head block. */
    if (MM_FREE_BLOCK == headFirstBlockSizeStatus) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerAppend_HeadFirstBlockIsFree);
        return FALSE;
    }

    if (headChunkId == tailChunkId) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerAppend_JoinedChunksAreSame);
        return FALSE;
    }

#if !DF()
    size_t  countHeadBlocks = MM_MAX_BLOCK_ID;          /* Down-counter of blocks in the head chunk. */
#endif
    size_t  eachHeadBlockId = headChunkId;              /* Iterates over blocks in the head chunk. */
    size_t  lastHeadBlockId;                            /* Finally will contain the head chunk last block Id. */
    do {
        SYS_DbgAssert(DF(countHeadBlocks-- > 0), FAIL_SYS_MemoryManagerAppend_LoopFoundInHeadChunk);
        lastHeadBlockId = eachHeadBlockId;
        eachHeadBlockId = mmGetNextBlock(eachHeadBlockId);
    } while (MM_BAD_BLOCK_ID != eachHeadBlockId);

#if !DF()
    size_t  countTailBlocks = MM_MAX_BLOCK_ID;          /* Down-counter of blocks in the tail chunk. */
#endif
#if !DF() || !DH()
    size_t  eachTailBlockId = tailChunkId;              /* Iterates over blocks in the tail chunk. */
    size_t  lastTailBlockId;                            /* Finally will contain the tail chunk last block Id. */
    do {
        SYS_DbgAssert(DF(countTailBlocks-- > 0), FAIL_SYS_MemoryManagerAppend_LoopFoundInTailChunk);
        if (headChunkId == eachTailBlockId) {
            SYS_DbgAssert(DF(!mmIsLeadingBlock(headFirstBlockSizeStatus)),
                    FAIL_SYS_MemoryManagerAppend_HeadLeadingBlockIsLinked);
            SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerAppend_HeadChunkIsPartOfTailChunk);
        }
        lastTailBlockId = eachTailBlockId;
        eachTailBlockId = mmGetNextBlock(eachTailBlockId);
    } while (MM_BAD_BLOCK_ID != eachTailBlockId);
    SYS_DbgAssert(DF(lastHeadBlockId != lastTailBlockId), FAIL_SYS_MemoryManagerAppend_HeadAndTailChainsIntersect);
#endif

    mmMakeNonleading(tailChunkId);
    mmSetNextBlock(lastHeadBlockId, tailChunkId);
    return TRUE;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Returns the size of the specified memory chunk, in bytes.
 */
SYS_PUBLIC MM_Size_t SYS_MemoryManagerGetSize(const MM_ChunkId_t  chunkId)
{
    if (MM_BAD_BLOCK_ID == chunkId) {
        SYS_DbgAssertLog(DI(), INFO_SYS_MemoryManagerGetSize_ChunkIsNull);
        return 0;
    }
    if (!mmIsValidBlockId(chunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerGetSize_ChunkIdIsInvalid);
        return 0;
    }
    if (MM_FREE_BLOCK == mmGetSizeStatus(chunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerGetSize_FirstBlockIsFree);
        return 0;
    }

#if !DF()
    size_t  countBlocks = MM_MAX_BLOCK_ID;          /* Down-counter of blocks in the chunk. */
#endif
    size_t  eachBlockId = chunkId;
    MM_Size_t  totalSize = 0;           /* Accumulator for the memory chunk total size calculation, in bytes. */
    do {
        SYS_DbgAssert(DF(countBlocks-- > 0), FAIL_SYS_MemoryManagerGetSize_LoopFoundInChunk);
        const MmSize_t  blockSizeStatus = mmGetSizeStatus(eachBlockId);         /* Size & Status of current block. */
        SYS_DbgAssert(DF(MM_FREE_BLOCK != blockSizeStatus), FAIL_SYS_MemoryManagerGetSize_FreeIntermediateBlock);
        totalSize += mmExtractSize(blockSizeStatus);
        eachBlockId = mmGetNextBlock(eachBlockId);
    } while (MM_BAD_BLOCK_ID != eachBlockId);
    return totalSize;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Copies data between the given plain array and the specified memory chunk.
 */
SYS_PUBLIC MM_Size_t SYS_MemoryManagerCopyArray(void *const  pArray, const MM_ChunkId_t  chunkId,
        const MM_Size_t  count, const MM_Size_t  offset, const MM_CopyDirection_t  direction)
{
    SYS_DbgAssert(DF(NULL != (uint8_t*)pArray), FAIL_SYS_MemoryManagerCopyArray_PtrArrayIsNull);

    if (0 == count) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerCopyArray_CountIsZero);
        return 0;
    }

    if (MM_BAD_BLOCK_ID == chunkId) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerCopyArray_ChunkIsNull);
        return 0;
    }
    if (!mmIsValidBlockId(chunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerCopyArray_ChunkIdIsInvalid);
        return 0;
    }
    if (MM_FREE_BLOCK == mmGetSizeStatus(chunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerCopyArray_FirstBlockIsFree);
        return 0;
    }

    uint8_t *curArray = pArray;             /* Pointer to the current byte in the plain array. */
    size_t  eachBlockId = chunkId;          /* Identifier of the currently processed block. */
    MM_Size_t  remCount = count;            /* Remaining number of bytes to be copied. */
    MM_Size_t  remOffset = offset;          /* Remaining part of the initial offset within the chunk. */
    do {
        const MmSize_t  blockSizeStatus = mmGetSizeStatus(eachBlockId);         /* Size & Status of current block. */
        SYS_DbgAssert(DF(MM_FREE_BLOCK != blockSizeStatus), FAIL_SYS_MemoryManagerCopyArray_FreeIntermediateBlockFound);
        MM_Size_t  blockSize = mmExtractSize(blockSizeStatus);                  /* Size of the current block. */
        uint8_t *blockData = mmBlocks[eachBlockId];             /* Pointer to the currently processed block data. */
        eachBlockId = mmGetNextBlock(eachBlockId);
        if (remOffset >= blockSize) {
            remOffset -= blockSize;
            continue;
        }
        blockSize -= remOffset;
        blockData += remOffset;
        remOffset = 0;
        if (remCount < blockSize)
            blockSize = remCount;
        if (MM_COPY_FROM_ARRAY_TO_CHUNK == direction)
            memcpy(blockData, curArray, blockSize);
        else
            memcpy(curArray, blockData, blockSize);
        curArray += blockSize;
        remCount -= blockSize;
        if (0 == remCount)
            return count;
    } while (MM_BAD_BLOCK_ID != eachBlockId);

    SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerCopyArray_CopiedLessBytesThanRequested);
    return (count - remCount);
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Copies data between two memory chunks.
 */
SYS_PUBLIC MM_Size_t SYS_MemoryManagerCopyChunk(const MM_ChunkId_t  dstChunkId, const MM_ChunkId_t  srcChunkId,
        const MM_Size_t  count)
{
    if (0 == count) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerCopyChunk_CountIsZero);
        return 0;
    }

    if (MM_BAD_BLOCK_ID == dstChunkId || MM_BAD_BLOCK_ID == srcChunkId) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerCopyChunk_ChunkIsNull);
        return 0;
    }
    if (!mmIsValidBlockId(dstChunkId) || !mmIsValidBlockId(srcChunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerCopyChunk_ChunkIdIsInvalid);
        return 0;
    }
    if (MM_FREE_BLOCK == mmGetSizeStatus(dstChunkId) || MM_FREE_BLOCK == mmGetSizeStatus(srcChunkId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerCopyChunk_FirstBlockIsFree);
        return 0;
    }

    size_t  dstBlockId = dstChunkId;        /* Identifier of the currently processed destination block. */
    size_t  srcBlockId = srcChunkId;        /* Identifier of the currently processed source block. */
    MM_Size_t  remCount = count;            /* Remaining number of bytes to be copied. */
    MM_Size_t  dstBlockSize = 0;            /* Size of the current destination block. */
    MM_Size_t  srcBlockSize = 0;            /* Size of the current source block. */
    uint8_t *dstBlockData = NULL;           /* Pointer to the currently processed destination block data. */
    uint8_t *srcBlockData = NULL;           /* Pointer to the currently processed source block data. */
    do {
        if (0 == dstBlockSize) {
            if (MM_BAD_BLOCK_ID == dstBlockId) {
                SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerCopyArray_CopiedLessDstBytesThanRequested);
                return (count - remCount);
            }
            const MmSize_t  blockSizeStatus = mmGetSizeStatus(dstBlockId);      /* Size & Status of Dst. block. */
            SYS_DbgAssert(DF(MM_FREE_BLOCK != blockSizeStatus),
                    FAIL_SYS_MemoryManagerCopyArray_FreeIntermediateDstBlockFound);
            dstBlockSize = mmExtractSize(blockSizeStatus);
            dstBlockData = mmBlocks[dstBlockId];
            dstBlockId = mmGetNextBlock(dstBlockId);
        }
        if (0 == srcBlockSize) {
            if (MM_BAD_BLOCK_ID == srcBlockId) {
                SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerCopyArray_CopiedLessSrcBytesThanRequested);
                return (count - remCount);
            }
            const MmSize_t  blockSizeStatus = mmGetSizeStatus(srcBlockId);      /* Size & Status of Src. block. */
            SYS_DbgAssert(DF(MM_FREE_BLOCK != blockSizeStatus),
                    FAIL_SYS_MemoryManagerCopyArray_FreeIntermediateSrcBlockFound);
            srcBlockSize = mmExtractSize(blockSizeStatus);
            srcBlockData = mmBlocks[srcBlockId];
            srcBlockId = mmGetNextBlock(srcBlockId);
        }
        *(dstBlockData++) = *(srcBlockData++);
        dstBlockSize--;
        srcBlockSize--;
        remCount--;
    } while (remCount > 0);
    return count;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Returns the starting address of the dynamic memory block by its Block Id.
 */
SYS_PUBLIC void* SYS_MemoryManagerGetMapBlockIdMemory(const MM_ChunkId_t  blockId)
{
    if (MM_BAD_BLOCK_ID == blockId) {
        SYS_DbgAssertLog(DW(), WARN_SYS_MemoryManagerGetMapBlockIdMemory_BlockIsNull);
        return NULL;
    }
    if (!mmIsValidBlockId(blockId)) {
        SYS_DbgAssert(DH(), HALT_SYS_MemoryManagerGetMapBlockIdMemory_BlockIdIsInvalid);
        return NULL;
    }
    return mmBlocks[blockId];
}

#if !DF()
/* ---------------------------------------------------------------------------------------------------------------------
 * Returns identifier of the next block linked to the given one.
 */
SYS_STATIC size_t mmGetNextBlock(const size_t  blockId)
{
    SYS_DbgAssert(mmIsValidBlockId(blockId), FAIL_mmGetNextBlock_BlockIdIsInvalid);
    const size_t  nextBlockId = mmLinks[blockId];
    SYS_DbgAssert(mmIsValidBlockId(nextBlockId), FAIL_mmGetNextBlock_NextBlockIdIsBroken);
    SYS_DbgAssert(IMP(MM_BAD_BLOCK_ID != nextBlockId, !mmIsLeadingBlock(mmSizes[nextBlockId])),
            FAIL_mmGetNextBlock_LinkedLeadingBlockFound);
    return nextBlockId;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Links the second specified block to the first one.
 */
SYS_STATIC void mmSetNextBlock(const size_t  blockId, const size_t  nextBlockId)
{
    SYS_DbgAssert(mmIsValidBlockId(blockId), FAIL_mmSetNextBlock_BlockIdIsInvalid);
    SYS_DbgAssert(mmIsValidBlockId(nextBlockId), FAIL_mmSetNextBlock_NextBlockIdIsInvalid);
    if (MM_BAD_BLOCK_ID != nextBlockId) {
        const MmSize_t  nextSizeStatus = mmSizes[nextBlockId];
        SYS_DbgAssert(IMP(MM_BAD_BLOCK_ID == blockId, MM_FREE_BLOCK == nextSizeStatus),
                FAIL_mmSetNextBlock_AttemptToLinkBusyBlockToFreeBlocksQueue);
        SYS_DbgAssert(!mmIsLeadingBlock(nextSizeStatus), FAIL_mmSetNextBlock_LinkedLeadingBlockFound);
    }
    mmLinks[blockId] = nextBlockId;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Sets the Leading Block flag for the specified block.
 */
SYS_STATIC void mmMakeLeading(const size_t  blockId)
{
    SYS_DbgAssert(mmIsValidBlockId(blockId), FAIL_mmMakeLeading_BlockIdIsInvalid);
    SYS_DbgAssert(MM_BAD_BLOCK_ID != blockId, FAIL_mmMakeLeading_NullBlockIsNotAllowed);
    const MmSize_t  sizeStatus = mmSizes[blockId];
    SYS_DbgAssert(MM_FREE_BLOCK != sizeStatus, FAIL_mmMakeLeading_FreeBlockIsNotAllowed);
    SYS_DbgAssert(!mmIsLeadingBlock(sizeStatus), FAIL_mmMakeLeading_BlockIsLeading);
    mmSizes[blockId] = (sizeStatus | (MM_LEADING_FLAG_MASK));
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Clears the Leading Block flag for the specified block.
 */
SYS_STATIC void mmMakeNonleading(const size_t  blockId)
{
    SYS_DbgAssert(mmIsValidBlockId(blockId), FAIL_mmMakeNonleading_BlockIdIsInvalid);
    SYS_DbgAssert(MM_BAD_BLOCK_ID != blockId, FAIL_mmMakeNonleading_NullBlockIsNotAllowed);
    const MmSize_t  sizeStatus = mmSizes[blockId];
    SYS_DbgAssert(mmIsLeadingBlock(sizeStatus), FAIL_mmMakeNonleading_BlockIsNotLeading);
    mmSizes[blockId] = (sizeStatus & ~(MM_LEADING_FLAG_MASK));
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Checks if the specified block is linked to any other block.
 */
SYS_STATIC bool mmIsLinkedBlock(const size_t  blockId)
{
    SYS_DbgAssert(mmIsValidBlockId(blockId), FAIL_mmIsLinkedBlock_BlockIdIsInvalid);
    for (size_t prevBlockId = MM_BAD_BLOCK_ID; prevBlockId <= MM_MAX_BLOCK_ID; prevBlockId++)
        if (blockId == mmLinks[prevBlockId])
            return TRUE;
    return FALSE;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Returns the Size & Status field of the given block.
 */
SYS_STATIC MmSize_t mmGetSizeStatus(const size_t  blockId)
{
    SYS_DbgAssert(mmIsValidBlockId(blockId), FAIL_mmGetSizeStatus_BlockIdIsInvalid);
    SYS_DbgAssert(MM_BAD_BLOCK_ID != blockId, FAIL_mmGetSizeStatus_NullBlockHasNoSize);
    const MmSize_t  sizeStatus = mmSizes[blockId];
    SYS_DbgAssert((MM_PROHIBITED_SIZE) != sizeStatus, FAIL_mmGetSizeStatus_FreeBlockMayNotBeLeading);
    SYS_DbgAssert(mmIsValidSize(mmExtractSize(sizeStatus)), FAIL_mmGetSizeStatus_BlockSizeIsBroken);
    return sizeStatus;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Preallocates the specified part of the given block.
 */
SYS_STATIC void mmPreallocBlock(const size_t  blockId, const MM_Size_t  size)
{
    SYS_DbgAssert(mmIsValidBlockId(blockId), FAIL_mmPreallocBlock_BlockIdIsInvalid);
    SYS_DbgAssert(MM_BAD_BLOCK_ID != blockId, FAIL_mmPreallocBlock_NullBlockMustNotBeUsed);
    SYS_DbgAssert(MM_FREE_BLOCK == mmSizes[blockId], FAIL_mmPreallocBlock_BlockIsBusy);
    SYS_DbgAssert(mmIsValidSize(size), FAIL_mmPreallocBlock_SizeIsInvalidOrAttemptToSetLeadingFlag);
    SYS_DbgAssert(MM_FREE_BLOCK != size, FAIL_mmPreallocBlock_MayNotAllocateZeroSize);
    mmSizes[blockId] = size;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Shrinks the specified block.
 */
SYS_STATIC void mmShrinkBlock(const size_t  blockId, const MM_Size_t  newSize)
{
    SYS_DbgAssert(mmIsValidBlockId(blockId), FAIL_mmShrinkBlock_BlockIdIsInvalid);
    SYS_DbgAssert(MM_BAD_BLOCK_ID != blockId, FAIL_mmShrinkBlock_NullBlockMustNotBeUsed);
    const MmSize_t  sizeStatus = mmSizes[blockId];
    SYS_DbgAssert(MM_FREE_BLOCK != sizeStatus, FAIL_mmShrinkBlock_BlockIsFree);
    const MM_Size_t  size = mmExtractSize(sizeStatus);
    SYS_DbgAssert(mmIsValidSize(size), FAIL_mmShrinkBlock_BlockSizeIsBroken);
    SYS_DbgAssert(MM_FREE_BLOCK != newSize, FAIL_mmShrinkBlock_NewSizeIsZero);
    SYS_DbgAssert(mmIsValidSize(newSize), FAIL_mmShrinkBlock_NewSizeIsInvalidOrAttemptToSetLeadingFlag);
    SYS_DbgAssert(newSize <= size, FAIL_mmShrinkBlock_NewSizeIsGreaterThanCurrent);
    mmSizes[blockId] = (newSize | (sizeStatus & (MM_LEADING_FLAG_MASK)));
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Abandons the specified block.
 */
SYS_STATIC void mmAbandonBlock(const size_t  blockId)
{
    SYS_DbgAssert(mmIsValidBlockId(blockId), FAIL_mmAbandonBlock_BlockIdIsInvalid);
    SYS_DbgAssert(MM_BAD_BLOCK_ID != blockId, FAIL_mmAbandonBlock_NullBlockMustNotBeUsed);
    const MmSize_t  sizeStatus = mmSizes[blockId];
    SYS_DbgAssert(mmIsValidSize(mmExtractSize(sizeStatus)), FAIL_mmAbandonBlock_BlockSizeIsBroken);
    SYS_DbgAssert(MM_FREE_BLOCK != sizeStatus, FAIL_mmAbandonBlock_BlockIsFree);
    mmSizes[blockId] = MM_FREE_BLOCK;
}
#endif /* !DF() */

/* ---------------------------------------------------------------------------------------------------------------------
 * Grants the subchain of preallocated blocks starting with the first found free (preallocated) block and up to the
 * specified one from the global queue of free blocks.
 */
SYS_STATIC size_t mmGrantChain(const size_t  lastBlockId)
{
    const size_t  firstBlockId = mmGetNextBlock(MM_BAD_BLOCK_ID);       /* Id of the first free block in the pool. */
    SYS_DbgAssert(DF(MM_FREE_BLOCK != mmGetSizeStatus(firstBlockId)), FAIL_mmGrantChain_FirstBlockIsFree);
    SYS_DbgAssert(DF(MM_FREE_BLOCK != mmGetSizeStatus(lastBlockId)), FAIL_mmGrantChain_LastBlockIsFree);
    mmSetNextBlock(MM_BAD_BLOCK_ID, mmGetNextBlock(lastBlockId));
    mmSetNextBlock(lastBlockId, MM_BAD_BLOCK_ID);
    mmMakeLeading(firstBlockId);
    return firstBlockId;
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Collects the subchain of abandoned blocks starting with the first and up to the last specified blocks and put them
 * into the global queue of free blocks.
 */
SYS_STATIC void mmCollectChain(const size_t  firstBlockId, const size_t  lastBlockId)
{
    SYS_DbgAssert(DF(MM_FREE_BLOCK == mmGetSizeStatus(firstBlockId)), FAIL_mmCollectChain_FirstBlockIsBusy);
    SYS_DbgAssert(DF(MM_FREE_BLOCK == mmGetSizeStatus(lastBlockId)), FAIL_mmCollectChain_LastBlockIsBusy);
    mmSetNextBlock(lastBlockId, mmGetNextBlock(MM_BAD_BLOCK_ID));
    mmSetNextBlock(MM_BAD_BLOCK_ID, firstBlockId);
}

/* ---------------------------------------------------------------------------------------------------------------------
 * Rollbacks the memory chunk allocation.
 */
SYS_STATIC void mmRollbackAlloc(void)
{
    size_t  eachBlockId = mmGetNextBlock(MM_BAD_BLOCK_ID);          /* Iterates over all the preallocated blocks. */
    while (MM_BAD_BLOCK_ID != eachBlockId) {
        mmAbandonBlock(eachBlockId);
        eachBlockId = mmGetNextBlock(eachBlockId);
    }
}

/************************* DEBUG CONFIGURATION ************************************************************************/
/* Reset definitions of debug assert wrappers.
 */
#undef DF
#undef DH
#undef DW
#undef DI

/* eof bbSysMemMan.c */
