/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Allocating.h"
#include "NonCopyable.h"
#include "Common.h"
#include "DevMemRange.h"

#include "libs/util/gfx_util/gfx_util.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/platform/gmem.h"

#include <cstdint>

namespace bvk {

// An arbitrary block of device-memory
class DevMemBlock : public Allocating, public NonCopyable
{
public:
   enum
   {
      // Size of each device memory block
      eDefaultBlockSize = 64 * 1024
   };

public:
   DevMemBlock() = delete;

   DevMemBlock(const VkAllocationCallbacks *pCallbacks, bool v3dReadWrite) :
      Allocating(pCallbacks),
      m_v3dReadWrite(v3dReadWrite)
   {
      Initialize(eDefaultBlockSize);
   }

   DevMemBlock(const VkAllocationCallbacks *pCallbacks, bool v3dReadWrite, size_t size) :
      Allocating(pCallbacks),
      m_v3dReadWrite(v3dReadWrite)
   {
      Initialize(size);
   }

   virtual ~DevMemBlock()
   {
      Cleanup();
   }

   // Request size bytes from the buffer
   bool Allocate(DevMemRange *range, size_t size, size_t align);

   // Called when block is acquired from the pool
   void Acquire();

   // Called when block is released back to the pool
   void Release();

   // Flush any cached writes in the block
   void SyncMemory()
   {
      gmem_flush_mapped_buffer(m_handle);
   }

   size_t Size() const { return m_size; }
   size_t Used() const { return m_size - m_free; }
   gmem_handle_t Handle() { return m_handle; }

private:
   void Cleanup();
   void Initialize(size_t size);

private:
   bool           m_v3dReadWrite = false; // false if read-only from V3D side
   uint8_t       *m_curPtr = nullptr;
   v3d_addr_t     m_curPhys = 0;

   gmem_handle_t  m_handle = GMEM_HANDLE_INVALID;
   size_t         m_size = 0;
   size_t         m_free = 0;
   uint8_t       *m_block = nullptr;
};

class DevMemDataBlock : public DevMemBlock
{
public:
   DevMemDataBlock() = delete;

   DevMemDataBlock(const VkAllocationCallbacks *pCallbacks) :
      DevMemBlock(pCallbacks, /*v3dReadWrite=*/false)
   {}

   DevMemDataBlock(const VkAllocationCallbacks *pCallbacks, size_t size) :
      DevMemBlock(pCallbacks, /*v3dReadWrite=*/false, size)
   {}
};

class DevMemQueryBlock : public DevMemBlock
{
public:
   DevMemQueryBlock() = delete;

   DevMemQueryBlock(const VkAllocationCallbacks *pCallbacks) :
      DevMemBlock(pCallbacks, /*v3dReadWrite=*/true)
   {}

   DevMemQueryBlock(const VkAllocationCallbacks *pCallbacks, size_t size) :
      DevMemBlock(pCallbacks, /*v3dReadWrite=*/true, size)
   {}
};

} // namespace bvk
