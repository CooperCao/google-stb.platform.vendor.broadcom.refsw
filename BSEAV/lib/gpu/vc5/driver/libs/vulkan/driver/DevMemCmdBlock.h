/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Allocating.h"
#include "NonCopyable.h"
#include "Common.h"

#include "libs/util/gfx_util/gfx_util.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/core/v3d/v3d_limits.h"
#include "libs/platform/gmem.h"

#include <cstdint>

namespace bvk {

// An arbitrary block of device-memory-backed data in a command buffer
class DevMemCmdBlock : public Allocating, public NonCopyable
{
public:
   enum
   {
      // Size of each device memory block
      eDefaultBlockSize = 64 * 1024
   };

private:
   enum
   {
      // Tail region of block - big enough to hold a branch, plus CLE read-ahead
      eTailSize = V3D_CL_BRANCH_SIZE + V3D_MAX_CLE_READAHEAD
   };

public:
   DevMemCmdBlock() = delete;
   DevMemCmdBlock(const VkAllocationCallbacks *pCallbacks);
   DevMemCmdBlock(const VkAllocationCallbacks *pCallbacks, v3d_size_t size);

   ~DevMemCmdBlock();

   // Called when block is acquired from the pool
   void Acquire();

   // Called when block is released back to the pool
   void Release();

   void Cleanup();

   bool CanFit(v3d_size_t size) const
   {
      return m_curPtr + size >= m_tailGuard;
   }

   bool CanFitFinal(v3d_size_t size) const
   {
      return size <= V3D_CL_BRANCH_SIZE;
   }

   void AddJumpTo(DevMemCmdBlock *newBlock)
   {
      // We are guaranteed to have room for this.
      assert(v3d_size_t(m_curPtr - m_block) + V3D_CL_BRANCH_SIZE < m_size);
      v3d_cl_branch(&m_curPtr, gmem_get_addr(newBlock->m_handle));
   }

   // Note: this allows m_curPtr to be modified externally.
   // Used by the control list generators.
   uint8_t **CurDataPtr() { return &m_curPtr; }

   // Return the physical address for the current block pointer
   v3d_addr_t CurPhys() const { return gmem_get_addr(m_handle) + (m_curPtr - m_block); }

   v3d_size_t OffsetOfPtr(uint8_t* ptr) const
   {
      assert(v3d_size_t(ptr - m_block) < m_size);
      return ptr - m_block;
   }

   // Flush any cached writes in the block
   void SyncMemory()
   {
      gmem_flush_mapped_buffer(m_handle);
   }

   v3d_size_t    Size() const { return m_size; }
   v3d_size_t    Used() const { return static_cast<v3d_size_t>(m_curPtr - m_block); }
   gmem_handle_t Handle()     { return m_handle; }

private:
   void Initialize(v3d_size_t size);

private:
   uint8_t       *m_block = nullptr;
   uint8_t       *m_curPtr = nullptr;
   uint8_t       *m_tailGuard = nullptr;

   gmem_handle_t  m_handle = GMEM_HANDLE_INVALID;
   v3d_size_t     m_size = 0;
};

} // namespace bvk
