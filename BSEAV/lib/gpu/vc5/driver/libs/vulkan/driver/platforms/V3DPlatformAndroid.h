/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#ifdef VK_USE_PLATFORM_ANDROID_KHR

#include "V3DPlatformBase.h"
#include "BVKPlatform.h"

namespace bvk
{

class V3DPlatformAndroid : public V3DPlatformBase
{
public:
   V3DPlatformAndroid();
   virtual ~V3DPlatformAndroid();

   // Attach platform specific swapchain backing buffers to a set of images
   void AttachSwapchain(
      const VkAllocationCallbacks      *pCallbacks,
      Device                           *pDevice,
      SwapchainKHR                     *chain) { };

   // Detach platform specific swapchain stuff
   void DetachSwapchain(
      const VkAllocationCallbacks      *pCallbacks,
      Device                           *pDevice,
      SwapchainKHR                     *chain) { };

   // Present a framebuffer for display
   VkResult QueueFrame(
      Device                  *device,
      const VkPresentInfoKHR  *presentInfo) { return VK_SUCCESS; };

   // Get a framebuffer to write into (with semaphores/fences to wait on)
   VkResult DequeueFrame(
      SwapchainKHR   *chain,
      uint64_t        timeout,
      Semaphore      *semaphore,
      Fence          *fence,
      uint32_t       *pImageIndex) {  return VK_SUCCESS; };

   void GetSurfaceExtent(VkExtent2D *extent, VkSurfaceKHR surf) const { };

   // Return a dummy driver version. This platform is only used for simulation.
   uint32_t GetDriverVersion() const { return 0; };

public:
   // Platform specific swapchain data. It is held in the swapchain.
   struct SwapchainData
   {
      // No platform specific data needed
   };

   // Platform specific image data. A vector of these is held in the swapchain.
   struct ImageData
   {
      // No platform specific data needed
   };

private:
   BVKPlatform m_bvkPlatform;
};

// Whenever V3DPlatform is seen, it means V3DPlatformAndroid
using V3DPlatform = V3DPlatformAndroid;

} // namespace bvk


#ifdef __cplusplus
extern "C" {
#endif

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainGrallocUsageANDROID(
    VkDevice            device,
    VkFormat            format,
    VkImageUsageFlags   imageUsage,
    int*                grallocUsage
);

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireImageANDROID(
    VkDevice            device,
    VkImage             image,
    int                 nativeFenceFd,
    VkSemaphore         semaphore,
    VkFence             fence
);
VKAPI_ATTR VkResult VKAPI_CALL vkQueueSignalReleaseImageANDROID(
    VkQueue             queue,
    uint32_t            waitSemaphoreCount,
    const VkSemaphore*  pWaitSemaphores,
    VkImage             image,
    int*                pNativeFenceFd
);

#ifdef __cplusplus
}
#endif


#endif // VK_USE_PLATFORM_ANDROID_KHR
