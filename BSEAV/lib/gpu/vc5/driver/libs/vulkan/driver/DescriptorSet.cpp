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

void DescriptorSet::WriteImageEntry(const VkDescriptorImageInfo *srcImageInfo,
                                    VkDescriptorType descType, uint32_t binding,
                                    DescriptorPool::ImageInfo *dstSysData, uint8_t *dstDevData,
                                    v3d_addr_t devAddr)
{
   size_t physOffset = 0;

   dstSysData->imageView = fromHandle<ImageView>(srcImageInfo->imageView);

   if (descType != VK_DESCRIPTOR_TYPE_SAMPLER)
   {
      // Everything but Sampler needs a texture state record, so write this first
      dstSysData->imageView->WriteTextureStateRecord(dstDevData, descType);
      physOffset += DescriptorPool::GetDevMemRecordSize();
   }

   if (descType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
       descType == VK_DESCRIPTOR_TYPE_SAMPLER)
   {
      // Only write the sampler if it's not an immutable one. Immutable ones are written
      // during construction.
      if (!m_layout->HasImmutableSamplers(binding))
      {
         Sampler *s = fromHandle<Sampler>(srcImageInfo->sampler);
         s->WriteSamplerRecord(dstDevData + physOffset);

         V3D_TMU_PARAM1_T p1 = {};
         p1.unnorm = s->UnnormalizedCoordinates();
         p1.sampler_addr = devAddr + physOffset;

         dstSysData->samplerParam = v3d_pack_tmu_param1(&p1);
      }
   }
}

void DescriptorSet::TemplateWriteImage(const VkDescriptorUpdateTemplateEntry *entry,
                                       const void *pData, size_t sysOffset, size_t devOffset)
{
   size_t elemSize    = DescriptorPool::CalcDescriptorTypeBytes(entry->descriptorType);
   size_t elemDevSize = DescriptorPool::CalcDescriptorTypeDevMemBytes(entry->descriptorType);
   size_t devSize     = elemDevSize * entry->descriptorCount;
   assert(devOffset + devSize <= m_devDataSize);

   uint8_t   *dstSysData = m_data + sysOffset;
   size_t     physOffset = m_devMemOffset + devOffset;
   uint8_t   *devPtr     = static_cast<uint8_t*>(gmem_get_ptr(m_devMemHandle)) + physOffset;
   v3d_addr_t devAddr    = gmem_get_addr(m_devMemHandle) + physOffset;
   auto       srcPtr     = static_cast<const uint8_t *>(pData) + entry->offset;

   for (uint32_t i = 0; i < entry->descriptorCount; i++)
   {
      auto dstImageInfo = reinterpret_cast<DescriptorPool::ImageInfo*>(dstSysData);
      auto srcData      = reinterpret_cast<const VkDescriptorImageInfo *>(srcPtr);

      WriteImageEntry(srcData, entry->descriptorType, entry->dstBinding,
                      dstImageInfo, devPtr, devAddr);

      dstSysData += elemSize;
      devPtr     += elemDevSize;
      devAddr    += elemDevSize;
      srcPtr     += entry->stride;
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

void DescriptorSet::WriteTexelBufferEntry(VkBufferView srcData,
                                          uint8_t *dstSysData, uint8_t *dstDevData)
{
   // Write the texture state record
   const BufferView *bv = fromHandle<const BufferView>(srcData);
   bv->WriteTextureStateRecord(dstDevData);

   // And the sysmem data
   auto dstSysData32 = reinterpret_cast<uint32_t *>(dstSysData);
   *dstSysData32 = bv->GetNumElems();
}

void DescriptorSet::TemplateWriteTexelBuffer(const VkDescriptorUpdateTemplateEntry *entry,
                                             const void *pData, size_t sysOffset, size_t devOffset)
{
   size_t elemSize    = DescriptorPool::CalcDescriptorTypeBytes(entry->descriptorType);
   size_t elemDevSize = DescriptorPool::CalcDescriptorTypeDevMemBytes(entry->descriptorType);
   size_t totDevSize  = elemDevSize * entry->descriptorCount;
   assert(devOffset + totDevSize <= m_devDataSize);

   uint8_t *devPtr = static_cast<uint8_t*>(gmem_get_ptr(m_devMemHandle)) + m_devMemOffset + devOffset;
   uint8_t *sysPtr = m_data + sysOffset;
   auto     srcPtr = static_cast<const uint8_t *>(pData) + entry->offset;

   for (uint32_t i = 0; i < entry->descriptorCount; i++)
   {
      auto srcData = reinterpret_cast<const VkBufferView *>(srcPtr);

      WriteTexelBufferEntry(*srcData, sysPtr, devPtr);

      srcPtr += entry->stride;
      sysPtr += elemSize;
      devPtr += elemDevSize;
   }

   gmem_flush_mapped_range(m_devMemHandle, m_devMemOffset + devOffset, totDevSize);
}

void DescriptorSet::WriteBufferEntry(const VkDescriptorBufferInfo *srcData,
                                     DescriptorPool::BufferInfo   *dstData)
{
   // The size is the amount of bytes in the buffer after taking the
   // bufInfo->offset into account. The range may be further restricted. The
   // range does not include any dynamicOffset.
   // [<--bufInfo.offset--><---------------size------------->]
   // [<--bufInfo.offset--><--dynamicOffset--><--range-->----]

   Buffer      *b = fromHandle<Buffer>(srcData->buffer);
   VkDeviceSize offset = srcData->offset;
   assert(offset <= b->Size());

   dstData->addr = b->CalculateBufferOffsetAddr(offset);
   dstData->size = static_cast<uint32_t>(b->Size() - offset);

   if (srcData->range == VK_WHOLE_SIZE)
      dstData->range = dstData->size;
   else
      dstData->range = std::min(static_cast<uint32_t>(srcData->range), dstData->size);
}

void DescriptorSet::TemplateWriteBuffer(const VkDescriptorUpdateTemplateEntry *entry,
                                        const void *pData, size_t sysOffset)
{
   auto dstData = reinterpret_cast<DescriptorPool::BufferInfo *>(m_data + sysOffset);
   auto srcPtr = static_cast<const uint8_t *>(pData) + entry->offset;

   for (uint32_t i = 0; i < entry->descriptorCount; i++)
   {
      auto srcData = reinterpret_cast<const VkDescriptorBufferInfo*>(srcPtr);
      WriteBufferEntry(srcData, &dstData[i]);
      srcPtr += entry->stride;
   }
}

void DescriptorSet::Write(const VkWriteDescriptorSet *writeInfo)
{
   VkDescriptorUpdateTemplateEntry e;
   e.dstBinding      = writeInfo->dstBinding;
   e.dstArrayElement = writeInfo->dstArrayElement;
   e.descriptorCount = writeInfo->descriptorCount;
   e.descriptorType  = writeInfo->descriptorType;
   e.offset          = 0;

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
      e.stride = sizeof(VkDescriptorImageInfo);
      TemplateWriteImage(&e, writeInfo->pImageInfo, sysOffset, devOffset);
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER         :
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER         :
   case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC :
   case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC :
      e.stride = sizeof(VkDescriptorBufferInfo);
      TemplateWriteBuffer(&e, writeInfo->pBufferInfo, sysOffset);
      break;

   case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   :
   case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   :
      e.stride = sizeof(VkBufferView);
      TemplateWriteTexelBuffer(&e, writeInfo->pTexelBufferView, sysOffset, devOffset);
      break;

   default:
      unreachable();
   }
}

void DescriptorSet::UpdateDescriptorSetWithTemplate(
   bvk::Device                   *device,
   bvk::DescriptorUpdateTemplate *updateTemplate,
   const void                    *pData) noexcept
{
   assert(updateTemplate->GetTemplateType() == VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET);

   for (auto &entry : updateTemplate->GetUpdateEntries())
   {
      // Find the offsets into the data block for the binding and element
      size_t sysOffset = m_layout->OffsetFor(entry.dstBinding, entry.dstArrayElement);
      size_t devOffset = m_layout->DeviceOffsetFor(entry.dstBinding, entry.dstArrayElement);

      assert(m_layout->BindingType(entry.dstBinding) == entry.descriptorType);

      // Copy the data
      switch (entry.descriptorType)
      {
      case VK_DESCRIPTOR_TYPE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
         TemplateWriteImage(&entry, pData, sysOffset, devOffset);
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
         TemplateWriteBuffer(&entry, pData, sysOffset);
         break;

      case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
         TemplateWriteTexelBuffer(&entry, pData, sysOffset, devOffset);
         break;

      default:
         unreachable();
      }
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
