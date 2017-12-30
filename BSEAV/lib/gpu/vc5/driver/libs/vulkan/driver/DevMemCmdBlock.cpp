/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "DevMemCmdBlock.h"

namespace bvk {

void DevMemCmdBlock::Initialize(v3d_size_t size)
{
   m_size = size;

   try
   {
      m_handle = gmem_alloc(size, 4096, GMEM_USAGE_V3D_READ, "DeviceCmdBlock");
      if (m_handle == GMEM_HANDLE_INVALID)
         throw bvk::bad_device_alloc();
   }
   catch (...)
   {
      Cleanup();
      throw;
   }
}

DevMemCmdBlock::DevMemCmdBlock(const VkAllocationCallbacks *pCallbacks) :
   Allocating(pCallbacks)
{
   Initialize(eDefaultBlockSize);
}

DevMemCmdBlock::DevMemCmdBlock(const VkAllocationCallbacks *pCallbacks, v3d_size_t size) :
   Allocating(pCallbacks)
{
   Initialize(size);
}

DevMemCmdBlock::~DevMemCmdBlock()
{
   Cleanup();
}

void DevMemCmdBlock::Cleanup()
{
   if (m_handle != GMEM_HANDLE_INVALID)
   {
      assert(m_block == nullptr);
      gmem_free(m_handle);
   }
}

void DevMemCmdBlock::Acquire()
{
   // Map into writable CPU address
   assert(m_block == nullptr);
   m_block = static_cast<uint8_t*>(gmem_map_and_invalidate_buffer(m_handle));
   if (m_block == nullptr)
      throw bvk::bad_device_alloc();

   m_curPtr    = m_block;
   m_tailGuard = m_block + m_size - eTailSize;
}

void DevMemCmdBlock::Release()
{
   m_block = nullptr;
}

} // namespace bvk
