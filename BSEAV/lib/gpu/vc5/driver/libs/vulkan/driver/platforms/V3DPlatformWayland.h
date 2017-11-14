/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)

#include "BVKPlatform.h"
#include "V3DPlatformBase.h"
#include "wl_client.h"
#include "surface_interface_wl_client.h"

#include <mutex>
#include <condition_variable>
#include <memory>

namespace bvk
{

struct SurfaceWayland : public Allocating
{
   static constexpr uint32_t Magic = 0x03a71a4d;

   SurfaceWayland(const VkAllocationCallbacks         *pAllocator,
                  const VkWaylandSurfaceCreateInfoKHR *pCreateInfo) :
      Allocating(pAllocator),
      display(pCreateInfo->display),
      surface(pCreateInfo->surface)
   {
   }

   uint32_t           magic = Magic;
   struct wl_display *display;
   struct wl_surface *surface;
};

class V3DPlatformWayland : public V3DPlatformBase
{
public:
   V3DPlatformWayland();
   virtual ~V3DPlatformWayland();

   // Attach platform specific swapchain backing buffers to a set of images
   virtual void AttachSwapchain(
      const VkAllocationCallbacks      *pCallbacks,
      Device                           *pDevice,
      SwapchainKHR                     *chain) override;

   // Detach platform specific swapchain stuff
   virtual void DetachSwapchain(
      const VkAllocationCallbacks      *pCallbacks,
      Device                           *pDevice,
      SwapchainKHR                     *chain) override;

   // Present a framebuffer for display
   virtual VkResult QueueFrame(
      Device                  *device,
      const VkPresentInfoKHR  *presentInfo) override;

   // Get a framebuffer to write into (with semaphores/fences to wait on)
   virtual VkResult DequeueFrame(
      SwapchainKHR   *chain,
      uint64_t        timeout,
      Semaphore      *semaphore,
      Fence          *fence,
      uint32_t       *pImageIndex) override;

   virtual VkResult GetSurfaceCapabilities(
      VkSurfaceKHR                surface,
      VkSurfaceCapabilitiesKHR   *pSurfaceCapabilities);

   virtual VkResult GetSurfacePresentModes(
      VkSurfaceKHR       surface,
      uint32_t          *pPresentModeCount,
      VkPresentModeKHR  *pPresentModes) override;

protected:
   virtual void GetSurfaceExtent(
         VkExtent2D *extent,
         VkSurfaceKHR surf) const override;

private:
   // A surface may have more than one swapchain using it (descendants only)
   class WaylandSurface: public NonCopyable, public Allocating
   {
   public:
      WaylandSurface(const VkAllocationCallbacks *pCallbacks,
            struct wl_display *display, struct wl_surface *surface);
      ~WaylandSurface();

      void SetCurrentSwapchain(SwapchainKHR *swapchain);

      void CreateBuffer(
            WaylandClientBuffer &buffer,
            uint32_t width, uint32_t height,
            BEGL_BufferFormat format,
            const struct wl_buffer_listener &listener,
            void *data);

      void DestroyBuffer(WaylandClientBuffer &buffer);

      void QueueBuffer(WaylandClientBuffer &buffer, bool wait);

   private:
      static void FrameCallback(void *data, struct wl_callback *callback,
            uint32_t time);
      void SetFrameCallback();
      int WaitFrameCallback();

   private:
      struct wl_display  *m_display = nullptr;
      struct wl_surface  *m_surface = nullptr;
      WaylandClient       m_client;
      SurfaceInterface    m_surfaceInterface;
      struct wl_callback *m_frameCallback = nullptr;
   };

public:
   // Platform specific image data. A vector of these is held in the swapchain.
   struct ImageData
   {
      V3DPlatformWayland   *m_platform  = nullptr;
      SwapchainKHR         *m_chain     = nullptr;
      WaylandClientBuffer   m_buffer;
      bool                  m_created   = false;
      bool                  m_available = false;

      ImageData()
      {
         memset(&m_buffer, 0, sizeof(m_buffer));
      }
   };

   // Platform specific swapchain data. It is held in the swapchain.
   struct SwapchainData
   {
      std::shared_ptr<WaylandSurface> m_surface = nullptr;
      std::mutex                      m_mutex;
      std::condition_variable         m_released;
      bool                            m_current = false;
   };

private:
   void CreateBuffer(ImageData &data, SwapchainKHR *chain, const VkImageCreateInfo &ci);
   int FindAvailableBuffer(SwapchainKHR *chain);
   static void BufferReleaseCallback(void *data, struct wl_buffer *buffer);
   void BufferRelease(ImageData &image);

private:
   BVKPlatform m_bvkPlatform;
};

// Whenever V3DPlatform is seen, it means V3DPlatformWaylandClient
using V3DPlatform = V3DPlatformWayland;

// Whenever V3DPlatformSurface is seen, it means SurfaceWayland
using V3DPlatformSurface = SurfaceWayland;

} // namespace bvk

#endif // VK_USE_PLATFORM_WAYLAND_KHR
