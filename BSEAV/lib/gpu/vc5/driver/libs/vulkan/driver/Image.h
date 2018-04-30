/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "SchedDependencies.h"

#include "libs/core/gfx_buffer/gfx_buffer.h"
#include "libs/platform/v3d_imgconv.h"

namespace bvk {

class Image: public NonCopyable, public Allocating
{
public:
   // The standard create image API construction
   Image(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkImageCreateInfo       *pCreateInfo): Image(pCallbacks, pDevice, pCreateInfo, false) {}

   // Internal image construction, e.g. for swapchain images
   Image(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkImageCreateInfo       *pCreateInfo,
      bool                           externalImage);

   ~Image() noexcept;

   VkResult BindImageMemory(
      bvk::Device       *device,
      bvk::DeviceMemory *memory,
      VkDeviceSize       memoryOffset) noexcept;

   void GetImageMemoryRequirements(
      bvk::Device          *device,
      VkMemoryRequirements *pMemoryRequirements) noexcept;

   void GetImageSparseMemoryRequirements(
      bvk::Device                      *device,
      uint32_t                         *pSparseMemoryRequirementCount,
      VkSparseImageMemoryRequirements  *pSparseMemoryRequirements) noexcept;

   void GetImageSubresourceLayout(
      bvk::Device                *device,
      const VkImageSubresource   *pSubresource,
      VkSubresourceLayout        *pLayout) noexcept;

   // Implementation specific from this point on

public:
   void InitGMemTarget(
      struct v3d_imgconv_gmem_tgt   *target,
      const SchedDependencies       &deps,
      uint32_t                      arrayLayer,
      uint32_t                      mipLevel) const
   {
      const GFX_BUFFER_DESC_T &desc = GetDescriptor(mipLevel);
      uint32_t offset = LayerOffset(arrayLayer);
      InitGMemTarget(target, deps, desc, offset, {0, 0, 0 });
   }

   // Calculates the offsets, strides etc for a subresource
   void GetSubresourceLayout(const VkImageSubresource *pSubresource,
                             VkSubresourceLayout      *pLayout);

   // Don't allow external access to the bound memory.
   // Doing so might easily break lazy memory handling.
   DeviceMemory *GetBoundMemory() const = delete;

   // Return a valid physical address for the image memory
   v3d_addr_t PhysAddr() const
   {
      m_boundMemory->AllocateLazyMemory();
      return m_boundMemory->PhysAddr();
   }

   bool HasUnallocatedLazyMemory() const
   {
      return m_transient && m_boundMemory->Handle() == GMEM_HANDLE_INVALID;
   }

   void AllocateLazyMemory() const
   {
      m_boundMemory->AllocateLazyMemory();
   }

   // Helper for vkCmdCopyImage schedule method which sets up one or more imageconv
   // jobs to copy the required image regions from the source. The returned JobID will
   // be for the last job scheduled, if multiple conversion jobs are required.
   JobID CopyFromImage(Image *pSrcImg, uint32_t regionCount, VkImageCopy *regions, const SchedDependencies &deps);

   // Helper for vkCmdCopyBufferToImage schedule method
   JobID CopyFromBuffer(Buffer *pSrcBuf, uint32_t regionCount, VkBufferImageCopy *regions, const SchedDependencies &deps);

   // Helper for vkCmdCopyImageToBuffer schedule method
   JobID CopyToBuffer(Buffer *pDstBuf, uint32_t regionCount, VkBufferImageCopy *regions, const SchedDependencies &deps);

   // Write a PA .txt file
   void DumpImage(uint32_t arrayLayer, uint32_t mipLevel, const std::string &basename) const;

   const GFX_BUFFER_DESC_T &GetDescriptor(uint32_t mipLevel) const { return m_mipDescs[mipLevel]; }

   uint32_t LayerOffset(uint32_t arrayLayer) const { return arrayLayer * m_arrayStride +
                                                     static_cast<uint32_t>(m_boundOffset); }
   size_t   LayerSize()                      const { return m_arrayStride; }

   VkExtent3D            Extent()      const { return m_extent; }
   uint32_t              MipLevels()   const { return m_mipLevels; }
   uint32_t              ArrayLayers() const { return m_arrayLayers; }
   VkSampleCountFlagBits Samples()     const { return m_samples; }
   VkImageUsageFlags     Usage()       const { return m_usage; }
   // This is for when the real Vulkan format is required. Most code should use LFMT().
   VkFormat              Format()      const { return m_format; }

   GFX_LFMT_T      LFMT()         const { return m_mipDescs[0].planes[0].lfmt; }
   GFX_LFMT_DIMS_T GetImageDims() const { return gfx_lfmt_get_dims(&m_mipDescs[0].planes[0].lfmt); }

private:
   void InitGMemTarget(
      struct v3d_imgconv_gmem_tgt  *target,
      const SchedDependencies      &deps,
      const GFX_BUFFER_DESC_T      &desc,
      uint32_t                      baseOffset,
      const VkOffset3D             &offset) const;

   // Image copy private implementation
   JobID CopyOneImageRegion(
      Image                   *pSrcImg,
      const VkImageCopy       &region,
      const SchedDependencies &deps);

   JobID CopyOneBufferRegion(
      Buffer                  *pSrcBuf,
      const VkBufferImageCopy &region,
      const SchedDependencies &deps);

   void UseImageCreateInfoExtension(const VkImageCreateInfo *pci);

   GFX_BUFFER_DESC_T       m_mipDescs[GFX_BUFFER_MAX_MIP_LEVELS];

   VkExtent3D              m_extent;
   uint32_t                m_mipLevels;
   uint32_t                m_arrayLayers;
   VkSampleCountFlagBits   m_samples;
   VkImageUsageFlags       m_usage;
   VkFormat                m_format;

   VkDeviceSize            m_totalSize = 0;
   VkDeviceSize            m_align = 0;

   uint32_t                m_arrayStride;

   DeviceMemory           *m_boundMemory = nullptr;
   VkDeviceSize            m_boundOffset = 0;

   bool                    m_autoDevMemBinding;
   bool                    m_transient = false;
   bool                    m_externalImage;
};

} // namespace bvk
