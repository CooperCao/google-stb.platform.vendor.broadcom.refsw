/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#ifdef VK_USE_PLATFORM_WIN32_KHR

#include "V3DPlatformBase.h"

namespace bvk
{

struct SurfaceWin32 : public Allocating
{
   static constexpr uint32_t Magic = 0x86321cd0;

   SurfaceWin32(const VkAllocationCallbacks     *pAllocator,
              const VkWin32SurfaceCreateInfoKHR *pCreateInfo) :
      Allocating(pAllocator),
      hWnd(pCreateInfo->hwnd)
   {
   }

#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
   // When created from vkCreateDisplayPlaneSurfaceKHR (VK_USE_PLATFORM_DISPLAY_KHR)
   SurfaceWin32(const VkAllocationCallbacks         *pAllocator,
                const VkDisplaySurfaceCreateInfoKHR *pCreateInfo);
#endif // VK_USE_PLATFORM_DISPLAY_KHR

   ~SurfaceWin32()
   {
      if (directDisplay)
         DestroyWindow(hWnd);
   }

   bool      directDisplay = false;
   uint32_t  magic = Magic;
   HWND      hWnd;
};

class V3DPlatformWin32 : public V3DPlatformWithFakeDirectDisplay
{
public:
   V3DPlatformWin32();
   virtual ~V3DPlatformWin32();

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
      BITMAPINFO  *m_bmi;
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
   uint32_t  m_curImage = 0;

   uint8_t  *m_rgba8888 = nullptr;
   size_t    m_rgba8888Size = 0;
};

// Whenever V3DPlatform is seen, it means V3DPlatformWin32
using V3DPlatform = V3DPlatformWin32;

// Whenever V3DPlatformSurface is seen, it means SurfaceXcb
using V3DPlatformSurface = SurfaceWin32;

} // namespace bvk

#endif // VK_USE_PLATFORM_WIN32_KHR
