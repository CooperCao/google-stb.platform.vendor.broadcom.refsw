/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

namespace bvk {

SwapchainKHR::SwapchainKHR(
   const VkAllocationCallbacks      *pCallbacks,
   bvk::Device                      *pDevice,
   const VkSwapchainCreateInfoKHR   *pCreateInfo) :
      Allocating(pCallbacks),
      m_device(pDevice)
{
   V3DPlatform &platform = pDevice->GetPlatform();

   if (pCreateInfo->oldSwapchain != 0)
   {
      // We don't have to do anything with oldSwapChain, but can free any
      // images that are no longer acquired by the application if we want.
   }

   // Shallow copy the create data and clean it up
   m_info = *pCreateInfo;
   m_info.pNext = nullptr;
   m_info.pQueueFamilyIndices = nullptr;    // We have only one queue

   // Get the number of images we will use for the swap chain
   uint32_t numBuffers = platform.NumRequiredSwapchainBuffers(m_info.minImageCount);

   // Build the create info for our images
   VkImageCreateInfo imgCI;
   imgCI.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
   imgCI.pNext         = nullptr;
   imgCI.flags         = 0;
   imgCI.imageType     = VK_IMAGE_TYPE_2D;
   imgCI.format        = m_info.imageFormat;
   imgCI.extent.width  = m_info.imageExtent.width;
   imgCI.extent.height = m_info.imageExtent.height;
   imgCI.extent.depth  = 1;
   imgCI.mipLevels     = 1;
   imgCI.arrayLayers   = m_info.imageArrayLayers;
   imgCI.samples       = VK_SAMPLE_COUNT_1_BIT;
   imgCI.tiling        = VK_IMAGE_TILING_OPTIMAL; // NOTE: this is mandated by the spec
   imgCI.usage         = m_info.imageUsage;
   imgCI.sharingMode   = m_info.imageSharingMode;
   imgCI.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
   imgCI.pQueueFamilyIndices   = pCreateInfo->pQueueFamilyIndices;
   imgCI.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

   try
   {
      m_images.resize(numBuffers, nullptr);
      m_platformImageData.resize(numBuffers);

      for (uint32_t i = 0; i < numBuffers; i++)
      {
         // Create an image
         m_images[i] = createObject<Image, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
                                  pCallbacks, GetCallbacks(), pDevice, &imgCI,
                                  /* externalImage = */ true);
      }

      // Acquire or Allocate, then bind the backing memory to the image
      platform.AttachSwapchain(pCallbacks, pDevice, this);
   }
   catch (...)
   {
      // If any allocation fails, cleanup and re-throw
      Cleanup();
      throw;
   }
}

void SwapchainKHR::Cleanup() noexcept
{
   V3DPlatform &platform = m_device->GetPlatform();

   // Detach platform specific stuff - this should free up any backing memory
   platform.DetachSwapchain(GetCallbacks(), m_device, this);
   assert(m_backing.size() == 0);

   // Really empty the backing vector
   bvk::vector<DeviceMemory*>().swap(m_backing);

   // Delete the images
   for (Image *img : m_images)
      destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(img, GetCallbacks());

   // Really empty the image vector
   bvk::vector<Image*>().swap(m_images);

   // Really empty any platform data
   bvk::vector<V3DPlatform::ImageData>().swap(m_platformImageData);
}

SwapchainKHR::~SwapchainKHR() noexcept
{
   Cleanup();
}

VkResult SwapchainKHR::GetSwapchainImagesKHR(
   bvk::Device *device,
   uint32_t    *pSwapchainImageCount,
   VkImage     *pSwapchainImages) noexcept
{
   VkResult result = VK_SUCCESS;

   if (pSwapchainImages == nullptr)
      *pSwapchainImageCount = m_images.size();
   else
   {
      if (m_images.size() > *pSwapchainImageCount)
         result = VK_INCOMPLETE;

      *pSwapchainImageCount = std::min((uint32_t)m_images.size(), *pSwapchainImageCount);

      for (uint32_t i = 0; i < *pSwapchainImageCount; i++)
         pSwapchainImages[i] = toHandle<VkImage>(m_images[i]);
   }

   return result;
}

VkResult SwapchainKHR::AcquireNextImageKHR(
   bvk::Device    *device,
   uint64_t        timeout,
   bvk::Semaphore *semaphore,
   bvk::Fence     *fence,
   uint32_t       *pImageIndex) noexcept
{
   V3DPlatform &platform = device->GetPlatform();
   return platform.DequeueFrame(this, timeout, semaphore, fence, pImageIndex);
}

} // namespace bvk
