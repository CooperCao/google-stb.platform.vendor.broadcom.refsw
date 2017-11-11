/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include <algorithm>
#include <cstring>

#include "AllObjects.h"
#include "Common.h"

namespace bvk {

Buffer::Buffer(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkBufferCreateInfo      *pCreateInfo) :
      Allocating(pCallbacks)
{
   m_align = gmem_non_coherent_atom_size();  // TODO: Simple, worst case scenario alignment for now
   m_size = gfx_u64round_up(pCreateInfo->size, m_align);
}

Buffer::~Buffer() noexcept
{
}

VkResult Buffer::BindBufferMemory(
   bvk::Device       *device,
   bvk::DeviceMemory *memory,
   VkDeviceSize       memoryOffset) noexcept
{
   // buffer must not already be backed by a memory object
   assert(m_boundMemory == nullptr);

   // memoryOffset must be an integer multiple of the alignment member of the VkMemoryRequirements
   // structure returned from a call to vkGetBufferMemoryRequirements with buffer
   assert(memoryOffset % m_align == 0);

   m_boundMemory = memory;
   m_boundOffset = memoryOffset;
   return VK_SUCCESS;
}

void Buffer::GetBufferMemoryRequirements(
   bvk::Device          *device,
   VkMemoryRequirements *pMemoryRequirements) noexcept
{
   pMemoryRequirements->size = m_size;
   pMemoryRequirements->alignment = m_align;

   // We need to create a bitMask where each bit represents one supported
   // memory type for this buffer, as defined in PhysicalDevice::InitMemoryProps()

   uint32_t mustHave = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

   // TODO : Exclude lazy alloc memory for now - do we want it later?
   uint32_t exclude = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;

   pMemoryRequirements->memoryTypeBits = device->GetPhysicalDevice()->
                                         CalculateMemoryTypeBits(mustHave, exclude);
}

void Buffer::CopyFromBuffer(Buffer *srcBuffer, uint32_t regionCount, VkBufferCopy *regions)
{
   for(uint32_t i = 0; i < regionCount; i++)
   {
      size_t size = static_cast<size_t>(regions[i].size);

      void *src = gmem_map_and_invalidate_range(
            srcBuffer->m_boundMemory->Handle(),
            static_cast<size_t>(srcBuffer->m_boundOffset + regions[i].srcOffset),
            size);

      void *dst = gmem_map_and_invalidate_range(
            m_boundMemory->Handle(),
            static_cast<size_t>(m_boundOffset + regions[i].dstOffset),
            size);

      // Overlapping src and dst regions, where the same buffer or aliases
      // to the same memory resource are being used, is not a valid usage.
      std::memcpy(dst, src, size);

      gmem_flush_mapped_range(
            m_boundMemory->Handle(),
            static_cast<size_t>(m_boundOffset + regions[i].dstOffset),
            size);
   }
}

void Buffer::UpdateBuffer(VkDeviceSize dstOffset, VkDeviceSize dataSize, void *data)
{
   size_t size = static_cast<size_t>(dataSize);
   size_t offset = static_cast<size_t>(m_boundOffset + dstOffset);

   void *dst = gmem_map_and_invalidate_range(
                  m_boundMemory->Handle(), offset,
                  size);

   std::memcpy(dst, data, size);

   gmem_flush_mapped_range(
         m_boundMemory->Handle(), offset,
         size);
}


void Buffer::FillBuffer(VkDeviceSize dstOffset, VkDeviceSize devSize, uint32_t data)
{
   size_t count;
   size_t size = static_cast<size_t>(devSize);

   assert((dstOffset % sizeof(uint32_t)) == 0);
   assert(size == VK_WHOLE_SIZE || (size % sizeof(uint32_t)) == 0);

   if (m_size != VK_WHOLE_SIZE)
   {
      count = size / sizeof(uint32_t);
   }
   else
   {
      // If m_size is not a multiple of 4 then we have to fill as many uint32_t
      // values as we can and leave the remainder unchanged.
      assert(dstOffset < m_size);
      count = static_cast<size_t>((m_size - dstOffset) / sizeof(uint32_t));
   }

   // Adjust the passed in size to be the required map size
   size = count * sizeof(uint32_t);

   size_t offset = static_cast<size_t>(m_boundOffset + dstOffset);

   auto dst = static_cast<uint32_t *>(
                 gmem_map_and_invalidate_range(
                    m_boundMemory->Handle(), offset,
                    size));

   std::fill_n(dst, count, data);

   gmem_flush_mapped_range(
         m_boundMemory->Handle(), offset,
         size);
}

} // namespace bvk
