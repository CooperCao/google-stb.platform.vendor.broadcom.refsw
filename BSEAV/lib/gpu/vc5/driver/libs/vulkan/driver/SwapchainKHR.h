/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

class SwapchainKHR : public NonCopyable, public Allocating
{
public:
   SwapchainKHR(
      const VkAllocationCallbacks      *pCallbacks,
      bvk::Device                      *pDevice,
      const VkSwapchainCreateInfoKHR   *pCreateInfo);

   ~SwapchainKHR() noexcept;

   VkResult GetSwapchainImagesKHR(
      bvk::Device *device,
      uint32_t    *pSwapchainImageCount,
      VkImage     *pSwapchainImages) noexcept;

   VkResult AcquireNextImageKHR(
      bvk::Device    *device,
      uint64_t        timeout,
      bvk::Semaphore *semaphore,
      bvk::Fence     *fence,
      uint32_t       *pImageIndex) noexcept;

   // Implementation specific from this point on
   const VkSwapchainCreateInfoKHR &CreateInfo() const { return m_info; }

   // Read-only access to the image objects in the chain
   const bvk::vector<Image*> &Images() const { return m_images; }

   // Access to device memory backing blocks
   bvk::vector<DeviceMemory*> &DevMemBacking() { return m_backing; }

   // Access to the platform specific swapchain data
   V3DPlatform::SwapchainData &PlatformSwapchainData() { return m_platformSwapchainData; }

   // Access to the platform specific image data array
   bvk::vector<V3DPlatform::ImageData> &PlatformImageData() { return m_platformImageData; }

private:
   void Cleanup() noexcept;

private:
   Device                    *m_device;
   VkSwapchainCreateInfoKHR   m_info;
   bvk::vector<Image*>        m_images;

   // A platform specific swapchain data
   V3DPlatform::SwapchainData m_platformSwapchainData;

   // A vector of platform specific image data - one for each image in the chain
   bvk::vector<V3DPlatform::ImageData>  m_platformImageData;

   // Not necessarily the same size as m_chain (platform code may just
   // allocate one big chunk of memory for example).
   // Platform dependent, may be empty too.
   bvk::vector<DeviceMemory*> m_backing;
};

} // namespace bvk
