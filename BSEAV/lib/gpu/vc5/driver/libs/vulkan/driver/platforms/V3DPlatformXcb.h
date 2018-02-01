/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#if defined(VK_USE_PLATFORM_XCB_KHR)

#include "V3DPlatformBase.h"

namespace bvk
{

struct SurfaceXcb : public Allocating
{
   static constexpr uint32_t Magic = 0xcb1cd000;

   // When created from vkCreateXcbSurfaceKHR (VK_USE_PLATFORM_XCB_KHR)
   SurfaceXcb(const VkAllocationCallbacks     *pAllocator,
              const VkXcbSurfaceCreateInfoKHR *pCreateInfo) :
      Allocating(pAllocator),
      connection(pCreateInfo->connection),
      window(pCreateInfo->window)
   {
      directDisplay = false;
   }

#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
   // When created from vkCreateDisplayPlaneSurfaceKHR (VK_USE_PLATFORM_DISPLAY_KHR)
   SurfaceXcb(const VkAllocationCallbacks         *pAllocator,
              const VkDisplaySurfaceCreateInfoKHR *pCreateInfo);
#endif // VK_USE_PLATFORM_DISPLAY_KHR

   ~SurfaceXcb()
   {
      if (directDisplay)
      {
         xcb_destroy_window(connection, window);
         xcb_disconnect(connection);
      }
   }

   bool              directDisplay = false;
   uint32_t          magic         = Magic;
   xcb_connection_t *connection;
   xcb_window_t      window;
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
   xcb_screen_t     *screen;
#endif // VK_USE_PLATFORM_DISPLAY_KHR
};

class V3DPlatformXcb : public V3DPlatformWithFakeDirectDisplay
{
public:
   V3DPlatformXcb();
   virtual ~V3DPlatformXcb();

   // Attach platform specific swapchain backing buffers to a set of images
   void AttachSwapchain(
      const VkAllocationCallbacks      *pCallbacks,
      Device                           *pDevice,
      SwapchainKHR                     *chain);

   // Detach platform specific swapchain stuff
   void DetachSwapchain(
      const VkAllocationCallbacks      *pCallbacks,
      Device                           *pDevice,
      SwapchainKHR                     *chain);

   // Present a framebuffer for display
   VkResult QueueFrame(
      Device                  *device,
      const VkPresentInfoKHR  *presentInfo);

   // Get a framebuffer to write into (with semaphores/fences to wait on)
   VkResult DequeueFrame(
      SwapchainKHR   *chain,
      uint64_t        timeout,
      Semaphore      *semaphore,
      Fence          *fence,
      uint32_t       *pImageIndex);

   void GetSurfaceExtent(VkExtent2D *extent, VkSurfaceKHR surf) const;

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
   static void ConvertTo8888(
      const Image &srcImage,
      void        *dstPtr,
      size_t       dstSize);

   void ResizeDisplayBuffer(uint32_t w, uint32_t h);

   void DisplayFrame(
      Device            *device,
      SwapchainKHR      *chain,
      const Image       &image,
      const ImageData   &data);

private:
   uint32_t  m_numImages = 0;
   uint32_t  m_curImage  = 0;

   uint8_t  *m_rgba8888 = nullptr;
   size_t    m_rgba8888Size = 0;
};

// Whenever V3DPlatform is seen, it means V3DPlatformXcb
using V3DPlatform = V3DPlatformXcb;

// Whenever V3DPlatformSurface is seen, it means SurfaceXcb
using V3DPlatformSurface = SurfaceXcb;

} // namespace bvk

#endif // VK_USE_PLATFORM_XCB_KHR
