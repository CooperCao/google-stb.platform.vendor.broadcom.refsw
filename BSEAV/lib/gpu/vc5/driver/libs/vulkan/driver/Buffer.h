/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

#include "libs/platform/gmem.h"

namespace bvk {

class Buffer: public NonCopyable, public Allocating
{
public:
   Buffer(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkBufferCreateInfo      *pCreateInfo);

   ~Buffer() noexcept;

   VkResult BindBufferMemory(
      bvk::Device       *device,
      bvk::DeviceMemory *memory,
      VkDeviceSize       memoryOffset) noexcept;

   void GetBufferMemoryRequirements(
      bvk::Device          *device,
      VkMemoryRequirements *pMemoryRequirements) noexcept;

   // Implementation specific from this point on
public:
   DeviceMemory *GetBoundMemory() const { return m_boundMemory; }
   VkDeviceSize GetBoundMemoryOffset() const { return m_boundOffset; }
   VkDeviceSize Size() const { return m_size; }

   v3d_addr_t CalculateBufferOffsetAddr(VkDeviceSize offset) const
   {
      return m_boundMemory->PhysAddr() + static_cast<v3d_addr_t>(m_boundOffset) +
                                         static_cast<v3d_addr_t>(offset);
   }

   // CPU based implementations of various buffer commands
   void CopyFromBuffer(Buffer *srcBuffer, uint32_t regionCount, VkBufferCopy *regions);
   void UpdateBuffer(VkDeviceSize dstOffset, VkDeviceSize dataSize, void *data);
   void FillBuffer(VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);

private:
   DeviceMemory        *m_boundMemory = nullptr;
   VkDeviceSize         m_boundOffset = 0;
   VkDeviceSize         m_size = 0;
   VkDeviceSize         m_align = 0;
};

} // namespace bvk
