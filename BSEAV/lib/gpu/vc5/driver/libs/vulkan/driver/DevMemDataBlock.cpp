/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "DevMemDataBlock.h"
#include "libs/core/v3d/v3d_util.h"

namespace bvk {

void DevMemBlock::Initialize(size_t size)
{
   m_size = size;
   m_free = size;

   try
   {
      gmem_usage_flags_t   usage = GMEM_USAGE_V3D_READ;
      if (m_v3dReadWrite)
         usage |= GMEM_USAGE_V3D_WRITE;

      m_handle = gmem_alloc(size, 4096, usage, "DeviceDataBlock");
      if (m_handle == GMEM_HANDLE_INVALID)
         throw bvk::bad_device_alloc();
   }
   catch (...)
   {
      Cleanup();
      throw;
   }
}

void DevMemBlock::Cleanup()
{
   if (m_handle != GMEM_HANDLE_INVALID)
   {
      assert(m_block == nullptr);
      gmem_free(m_handle);
   }
}

bool DevMemBlock::Allocate(DevMemRange *range, size_t size, size_t align)
{
   // Adjust for alignment requirements
   v3d_addr_t aligned = v3d_addr_align_up(m_curPhys, align);
   size_t     offset = aligned - m_curPhys;

   if (size + offset <= m_free)
   {
      m_curPhys = aligned;
      m_curPtr  += offset;
      m_free    -= offset;

      range->Set(this->Handle(), m_curPhys, m_curPtr, size);

      m_curPhys += size;
      m_curPtr  += size;
      m_free    -= size;
      return true;
   }
   return false;
}

void DevMemBlock::Acquire()
{
   // Map into writable CPU address
   assert(m_block == nullptr);
   m_block = static_cast<uint8_t*>(gmem_map_and_invalidate_buffer(m_handle));
   if (m_block == nullptr)
      throw bvk::bad_device_alloc();

   m_curPtr  = m_block;
   m_curPhys = gmem_get_addr(m_handle);
}

void DevMemBlock::Release()
{
   m_block = nullptr;
   m_free  = m_size;
}

} // namespace bvk
