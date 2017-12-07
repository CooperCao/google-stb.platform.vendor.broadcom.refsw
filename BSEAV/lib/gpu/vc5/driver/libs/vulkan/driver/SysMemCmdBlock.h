/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Allocating.h"
#include "libs/util/gfx_util/gfx_util.h"

#include <cstdint>

namespace bvk {

class PipelineLayout;

// An arbitrary block of system-memory data in a command buffer.
// Essentially an arena allocator for Command object derivatives.
class SysMemCmdBlock : public Allocating
{
public:
   enum
   {
      eDefaultBlockSize = 8192,
      eLargeBlockThreshold = eDefaultBlockSize / 2
   };

public:
   SysMemCmdBlock() = delete;

   SysMemCmdBlock(const VkAllocationCallbacks *pCallbacks, size_t size = eDefaultBlockSize) :
      Allocating(pCallbacks),
      m_used(0)
   {
      m_block = GetObjScopeAllocator<uint8_t>().allocate(size);
      m_curPtr = m_block;
      m_size = size;
   }

   ~SysMemCmdBlock()
   {
      GetObjScopeAllocator<uint8_t>().deallocate((uint8_t*)m_block, m_size);
   }

   bool Allocate(void **ret, size_t size, size_t align = sizeof(uintptr_t))
   {
      // Adjust for alignment requirements
      uint8_t  *aligned = static_cast<uint8_t*>(gfx_align_up(m_curPtr, align));
      size_t    offset = aligned - m_curPtr;

      if (m_used + size + offset < m_size)
      {
         *ret = aligned;
         m_used += offset + size;
         m_curPtr = aligned + size;
         return true;
      }

      *ret = nullptr;
      return false;
   }

   void Acquire() {} // Nothing to do

   void Release()
   {
      m_used = 0;
      m_curPtr = m_block;
   }

   size_t Size() const { return m_size; }
   size_t Used() const { return m_used; }


private:
   uint8_t  *m_block = nullptr;
   uint8_t  *m_curPtr = nullptr;
   size_t    m_used = 0;
   size_t    m_size = 0;
};

} // namespace bvk
