/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

namespace bvk {

static inline bool IsImageType(VkDescriptorType t)
{
   return t == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE          ||
          t == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE          ||
          t == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
          t == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
}
static inline bool IsBufferType(VkDescriptorType t)
{
   return t == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
          t == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
          t == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
          t == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
}
static inline bool IsTexelBufferType(VkDescriptorType t)
{
   return t == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
          t == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
}

DescriptorSet::DescriptorSet(const DescriptorSetLayout *layout,
                             void *data, size_t dataSize,
                             gmem_handle_t devMem, size_t devMemOffset,
                             size_t devMemSize) :
   m_layout(layout->State()), m_data(static_cast<uint8_t*>(data)),
   m_dataSize(dataSize), m_devMemHandle(devMem),
   m_devMemOffset(devMemOffset), m_devDataSize(devMemSize)
{
   // Write the immutable sampler records into the pool now.
   // We may never get another chance.
   if (m_devMemHandle != GMEM_HANDLE_INVALID)
   {
      bool flush = m_layout->WriteImmutableSamplerRecords(m_data, m_devMemOffset, m_devMemHandle);
      if (flush)
         gmem_flush_mapped_range(m_devMemHandle, m_devMemOffset, m_devDataSize);
   }
}

DescriptorSet::~DescriptorSet() noexcept
{
}

void DescriptorSet::WriteImage(const VkWriteDescriptorSet *writeInfo,
                               size_t sysOffset, size_t devOffset)
{
   size_t elemSize = DescriptorPool::CalcDescriptorTypeBytes(writeInfo->descriptorType);
   size_t size     = elemSize * writeInfo->descriptorCount;
   assert(sysOffset + size <= m_dataSize);

   size_t elemDevSize = DescriptorPool::CalcDescriptorTypeDevMemBytes(writeInfo->descriptorType);
   size_t devSize     = elemDevSize * writeInfo->descriptorCount;
   assert(devOffset + devSize <= m_devDataSize);

   for (uint32_t i = 0; i < writeInfo->descriptorCount; i++)
   {
      auto data       = reinterpret_cast<DescriptorPool::ImageInfo*>(m_data + sysOffset + i * elemSize);
      data->imageView = fromHandle<ImageView>(writeInfo->pImageInfo[i].imageView);

      size_t   physOffset = m_devMemOffset + devOffset + i * elemDevSize;
      uint8_t *devPtr     = static_cast<uint8_t*>(gmem_get_ptr(m_devMemHandle));

      if (writeInfo->descriptorType != VK_DESCRIPTOR_TYPE_SAMPLER)
      {
         // Everything but Sampler needs a texture state record, so write this first
         data->imageView->WriteTextureStateRecord(devPtr + physOffset, writeInfo->descriptorType);
         physOffset += DescriptorPool::GetDevMemRecordSize();
      }

      if (writeInfo->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
          writeInfo->descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)
      {
         // Only write the sampler if it's not an immutable one. Immutable ones are written
         // during construction.
         if (!m_layout->HasImmutableSamplers(writeInfo->dstBinding))
         {
            Sampler *s = fromHandle<Sampler>(writeInfo->pImageInfo[i].sampler);
            s->WriteSamplerRecord(devPtr + physOffset);

            V3D_TMU_PARAM1_T p1 = {};
            p1.unnorm       = s->UnnormalizedCoordinates();
            p1.sampler_addr = gmem_get_addr(m_devMemHandle) + physOffset;

            data->samplerParam = v3d_pack_tmu_param1(&p1);
         }
      }
   }

   gmem_flush_mapped_range(m_devMemHandle, m_devMemOffset + devOffset, devSize);
}

uint32_t DescriptorSet::GetImageParam(uint32_t binding, uint32_t element) const
{
   assert(IsImageType(m_layout->BindingType(binding)) ||
          IsTexelBufferType(m_layout->BindingType(binding)));

   // Returns the physical address of the texture state record
   size_t devOffset = m_layout->DeviceOffsetFor(binding, element);

   V3D_TMU_PARAM0_T p0 = {};
   p0.tex_state_addr = gmem_get_addr(m_devMemHandle) + m_devMemOffset + devOffset;

   return v3d_pack_tmu_param0(&p0);
}

// Returns the sampler parameter for the binding and element
uint32_t DescriptorSet::GetSamplerParam(uint32_t binding, uint32_t element) const
{
   assert(m_layout->BindingType(binding) == VK_DESCRIPTOR_TYPE_SAMPLER ||
          m_layout->BindingType(binding) == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

   // Find the offset into the data block for the binding and element
   size_t offset = m_layout->OffsetFor(binding, element);
   return reinterpret_cast<DescriptorPool::ImageInfo*>(m_data + offset)->samplerParam;
}

void DescriptorSet::WriteTexelBuffer(const VkWriteDescriptorSet *writeInfo,
                                     size_t sysOffset, size_t devOffset)
{
   size_t elemSize = DescriptorPool::CalcDescriptorTypeBytes(writeInfo->descriptorType);
   size_t size     = elemSize * writeInfo->descriptorCount;
   assert(sysOffset + size <= m_dataSize);

   size_t elemDevSize = DescriptorPool::CalcDescriptorTypeDevMemBytes(writeInfo->descriptorType);
   size_t devSize     = elemDevSize * writeInfo->descriptorCount;
   assert(devOffset + devSize <= m_devDataSize);

   for (uint32_t i = 0; i < writeInfo->descriptorCount; i++)
   {
      size_t   physOffset = m_devMemOffset + devOffset + i * elemDevSize;
      uint8_t *devPtr     = static_cast<uint8_t*>(gmem_get_ptr(m_devMemHandle)) + physOffset;

      // Write the texture state record
      BufferView *bv = fromHandle<BufferView>(writeInfo->pTexelBufferView[i]);
      bv->WriteTextureStateRecord(devPtr);

      auto data = reinterpret_cast<uint32_t *>(m_data + sysOffset + i * elemSize);
      *data     = bv->GetNumElems();
   }

   gmem_flush_mapped_range(m_devMemHandle, m_devMemOffset + devOffset, devSize);
}

void DescriptorSet::WriteBuffer(const VkWriteDescriptorSet *writeInfo,
                                size_t sysOffset, size_t devOffset)
{
   // The size is the amount of bytes in the buffer after taking the
   // bufInfo->offset into account. The range may be further restricted. The
   // range does not include any dynamicOffset.
   // [<--bufInfo.offset--><---------------size------------->]
   // [<--bufInfo.offset--><--dynamicOffset--><--range-->----]
   auto data = reinterpret_cast<DescriptorPool::BufferInfo *>(m_data + sysOffset);

   for (uint32_t i = 0; i < writeInfo->descriptorCount; i++)
   {
      Buffer      *b      = fromHandle<Buffer>(writeInfo->pBufferInfo[i].buffer);
      VkDeviceSize offset = writeInfo->pBufferInfo[i].offset;
      assert(offset <= b->Size());

      data[i].addr = b->CalculateBufferOffsetAddr(offset);
      data[i].size = static_cast<uint32_t>(b->Size() - offset);

      if (writeInfo->pBufferInfo[i].range == VK_WHOLE_SIZE)
         data[i].range = data[i].size;
      else
         data[i].range = std::min(static_cast<uint32_t>(writeInfo->pBufferInfo[i].range), data[i].size);
   }
}

void DescriptorSet::Write(const VkWriteDescriptorSet *writeInfo)
{
   // Find the offsets into the data block for the binding and element
   size_t sysOffset = m_layout->OffsetFor(writeInfo->dstBinding, writeInfo->dstArrayElement);
   size_t devOffset = m_layout->DeviceOffsetFor(writeInfo->dstBinding, writeInfo->dstArrayElement);

   assert(m_layout->BindingType(writeInfo->dstBinding) == writeInfo->descriptorType);

   // Copy the data
   switch (writeInfo->descriptorType)
   {
   case VK_DESCRIPTOR_TYPE_SAMPLER                :
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER :
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE          :
   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE          :
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT       :
      WriteImage(writeInfo, sysOffset, devOffset);
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER         :
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER         :
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC :
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC :
      WriteBuffer(writeInfo, sysOffset, devOffset);
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   :
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   :
      WriteTexelBuffer(writeInfo, sysOffset, devOffset);
      break;

   default:
      unreachable();
   }
}

void DescriptorSet::Copy(const VkCopyDescriptorSet *copyInfo)
{
   assert(m_layout->BindingType(copyInfo->srcBinding) == m_layout->BindingType(copyInfo->dstBinding));

   DescriptorSet *srcDS = fromHandle<DescriptorSet>(copyInfo->srcSet);

   // First the system memory
   if (m_dataSize > 0)
   {
      // Find the offsets into the data block for the binding and element
      size_t srcOffset = m_layout->OffsetFor(copyInfo->srcBinding, copyInfo->srcArrayElement);
      size_t dstOffset = m_layout->OffsetFor(copyInfo->dstBinding, copyInfo->dstArrayElement);

      uint8_t *srcPtr = srcDS->m_data + srcOffset;
      uint8_t *dstPtr = m_data + dstOffset;

      // Work out the copy size
      size_t elemSize = DescriptorPool::CalcDescriptorTypeBytes(m_layout->BindingType(copyInfo->srcBinding));
      size_t size     = elemSize * copyInfo->descriptorCount;

      assert(srcOffset + size <= srcDS->m_dataSize);
      assert(dstOffset + size <= m_dataSize);

      // Do the copy
      memcpy(dstPtr, srcPtr, size);
   }

   // Now the device memory
   if (m_devDataSize > 0)
   {
      size_t srcOffset = m_layout->DeviceOffsetFor(copyInfo->srcBinding, copyInfo->srcArrayElement);
      size_t dstOffset = m_layout->DeviceOffsetFor(copyInfo->dstBinding, copyInfo->dstArrayElement);

      uint8_t *srcPtr = static_cast<uint8_t*>(gmem_get_ptr(srcDS->m_devMemHandle)) +
                                              srcDS->m_devMemOffset + srcOffset;
      uint8_t *dstPtr = static_cast<uint8_t*>(gmem_get_ptr(m_devMemHandle)) +
                                              m_devMemOffset + dstOffset;

      // Work out the copy size
      size_t elemSize = DescriptorPool::CalcDescriptorTypeDevMemBytes(m_layout->BindingType(
                                                                      copyInfo->srcBinding));
      size_t size     = elemSize * copyInfo->descriptorCount;

      assert(srcOffset + size <= srcDS->m_devDataSize);
      assert(dstOffset + size <= m_devDataSize);

      // Do the copy
      memcpy(dstPtr, srcPtr, size);

      gmem_flush_mapped_range(m_devMemHandle, m_devMemOffset + dstOffset, size);
   }
}

// Returns the full imageinfo associated with the binding and element
const ImageView *DescriptorSet::GetImageView(uint32_t binding, uint32_t element) const
{
   assert(IsImageType(m_layout->BindingType(binding)));
   size_t offset = m_layout->OffsetFor(binding, element);
   return reinterpret_cast<DescriptorPool::ImageInfo*>(m_data + offset)->imageView;
}

v3d_addr_t DescriptorSet::GetBufferAddress(uint32_t binding, uint32_t element) const
{
   assert(IsBufferType(m_layout->BindingType(binding)));
   size_t offset = m_layout->OffsetFor(binding, element);
   return reinterpret_cast<DescriptorPool::BufferInfo*>(m_data + offset)->addr;
}

uint32_t DescriptorSet::GetBufferSize(uint32_t binding, uint32_t element) const
{
   assert(IsBufferType(m_layout->BindingType(binding)));
   size_t offset = m_layout->OffsetFor(binding, element);
   return reinterpret_cast<DescriptorPool::BufferInfo*>(m_data + offset)->size;
}

uint32_t DescriptorSet::GetBufferRange(uint32_t binding, uint32_t element) const
{
   assert(IsBufferType(m_layout->BindingType(binding)));
   size_t offset = m_layout->OffsetFor(binding, element);
   return reinterpret_cast<DescriptorPool::BufferInfo*>(m_data + offset)->range;
}

uint32_t DescriptorSet::GetNumLevels(uint32_t binding, uint32_t element) const
{
   switch (m_layout->BindingType(binding))
   {
   case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
   case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      return GetImageView(binding, element)->GetNumLevels();

   case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
   case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return 1;
   default:
      unreachable();
   }
}

VkExtent3D DescriptorSet::GetTextureSize(uint32_t binding, uint32_t element) const
{
   assert(IsImageType(m_layout->BindingType(binding)) ||
          IsTexelBufferType(m_layout->BindingType(binding)));

   if (IsImageType(m_layout->BindingType(binding)))
      return GetImageView(binding, element)->GetBaseExtent();
   else
   {
      size_t offset = m_layout->OffsetFor(binding, element);
      const uint32_t *size = reinterpret_cast<const uint32_t *>(m_data + offset);
      return VkExtent3D{ *size, 1, 1 };
   }
}

} // namespace bvk
