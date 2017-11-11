/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

#include "V3DPlatformWayland.h"
#include "Common.h"
#include "Device.h"
#include "Fence.h"
#include "PhysicalDevice.h"
#include "DeviceMemory.h"
#include "Image.h"
#include "Semaphore.h"
#include "SwapchainKHR.h"

#include "libs/platform/v3d_scheduler.h"
#include "libs/platform/v3d_parallel.h"
#include "libs/util/profile/profile.h"

#include "nexus_platform.h"
#include "nxclient.h"
#include "surface_interface_wl_client.h"

#include <string.h>
#include <wayland-client.h>

namespace bvk
{

V3DPlatformWayland::WaylandSurface::WaylandSurface(
      const VkAllocationCallbacks *pCallbacks,
      struct wl_display *display, struct wl_surface *surface):
      Allocating(pCallbacks),
      m_display(display),
      m_surface(surface)
{
   assert(m_display);
   assert(m_surface);

   memset(&m_client, 0, sizeof(m_client));
   memset(&m_surfaceInterface, 0, sizeof(m_surfaceInterface));

   if (!InitWaylandClient(&m_client, m_display))
      throw VK_ERROR_SURFACE_LOST_KHR;

   if (!SurfaceInterface_InitWlClient(&m_surfaceInterface, &m_client))
      throw VK_ERROR_SURFACE_LOST_KHR;
}

V3DPlatformWayland::WaylandSurface::~WaylandSurface()
{
   Interface_Destroy(&m_surfaceInterface.base);
   DestroyWaylandClient(&m_client);
}

void V3DPlatformWayland::WaylandSurface::CreateBuffer(
      WaylandClientBuffer &buffer,
      uint32_t width, uint32_t height,
      BEGL_BufferFormat format,
      const struct wl_buffer_listener &listener,
      void *data)
{
   const bool secure = false;
   if (!SurfaceInterface_Create(&m_surfaceInterface, &buffer,
         width, height, format, secure))
      throw VK_ERROR_OUT_OF_DEVICE_MEMORY;

   if (wl_buffer_add_listener(buffer.buffer, &listener, data) != 0)
   {
      SurfaceInterface_Destroy(&m_surfaceInterface, &buffer);
      throw VK_ERROR_SURFACE_LOST_KHR;
   }

   buffer.buffer_release_callback = listener.release;
}

void V3DPlatformWayland::WaylandSurface::DestroyBuffer(WaylandClientBuffer &buffer)
{
   SurfaceInterface_Destroy(&m_surfaceInterface, &buffer);
}

void V3DPlatformWayland::WaylandSurface::QueueBuffer(
      WaylandClientBuffer &buffer, bool wait)
{
   assert(buffer.buffer_release_callback);

   if (wait)
   {
      WaitFrameCallback();
      SetFrameCallback();
   }

   {
      // attach the new buffer to the surface
      const int32_t dx = 0;
      const int32_t dy = 0;
      wl_surface_attach(m_surface, buffer.buffer, dx, dy);

      wl_surface_damage(m_surface, 0, 0, buffer.info.width, buffer.info.height);

      // send the buffer accross to the server
      wl_surface_commit(m_surface);

      // flush the fd
      wl_display_flush(m_client.display);
   }
}

/*static*/ void V3DPlatformWayland::WaylandSurface::FrameCallback(void *data,
      struct wl_callback *callback, uint32_t time)
{
   WaylandSurface *self = static_cast<WaylandSurface *>(data);
   self->m_frameCallback = nullptr;
   wl_callback_destroy(callback);
}

int V3DPlatformWayland::WaylandSurface::WaitFrameCallback()
{
   int ret = 0;
   // process any pending events
   wl_display_dispatch_queue_pending(m_client.display, m_client.events);
   while (m_frameCallback && ret != -1)
   {
      //wait for any new events
      ret = wl_display_dispatch_queue(m_client.display, m_client.events);
   }
   return ret;
}

void V3DPlatformWayland::WaylandSurface::SetFrameCallback()
{
   assert(!m_frameCallback);
   static const struct wl_callback_listener frame_listener =
   {
      /*.done =*/ FrameCallback,
   };

   m_frameCallback = wl_surface_frame(m_surface);
   wl_callback_add_listener(m_frameCallback, &frame_listener, this);
   wl_proxy_set_queue((wl_proxy *)(m_frameCallback), m_client.events);
}

V3DPlatformWayland::V3DPlatformWayland()
{
   NxClient_JoinSettings joinSettings;

   NxClient_GetDefaultJoinSettings(&joinSettings);
   snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "Vulkan Display");
   joinSettings.mode = NEXUS_ClientMode_eVerified;

   NEXUS_Error err = NxClient_Join(&joinSettings);
   if (err != NEXUS_SUCCESS)
      throw std::runtime_error("NxClient_Join() failed\n");

   // Init the abstract platform memory and scheduler interfaces
   m_bvkPlatform.Register();

   // Now bring up the common platform parts
   CommonInitialize();
}

V3DPlatformWayland::~V3DPlatformWayland()
{
   CommonTerminate();

   // Remove memory and scheduler interfaces
   BVK_UnregisterNexusPlatform(m_bvkPlatform);
   m_bvkPlatform = BVK_NULL_HANDLE;

   NxClient_Uninit();
}

void V3DPlatformWayland::AttachSwapchain(
   const VkAllocationCallbacks *pCallbacks,
   Device                      *pDevice,
   SwapchainKHR                *chain)
{
   SwapchainData &psd = chain->PlatformSwapchainData();

   SurfaceWayland *surf = FromSurfaceKHR<SurfaceWayland>(chain->CreateInfo().surface);
   assert(surf->display);
   assert(surf->surface);

   // Use shared pointer to a single Surface object during resize
   // i.e. when a descendant swapchain is created.
   VkSwapchainKHR old = chain->CreateInfo().oldSwapchain;
   if (old)
   {
      SwapchainKHR *oldPtr = bvk::fromHandle<SwapchainKHR>(old);
      assert(oldPtr->CreateInfo().surface == chain->CreateInfo().surface);
      psd.m_surface = oldPtr->PlatformSwapchainData().m_surface;

      oldPtr->PlatformSwapchainData().m_current = false;
   }
   else
   {
      Allocator<WaylandSurface> a(pCallbacks ? pCallbacks : GetCallbacks(),
            VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);
      psd.m_surface =
            std::allocate_shared<WaylandSurface, Allocator<WaylandSurface>>(
                  a, pCallbacks, surf->display, surf->surface);
   }
   psd.m_current = true;

   // all images must have the same size and format
   VKFormat format = chain->CreateInfo().format;
   uint32_t width  = chain->CreateInfo().imageExtent.width;
   uint32_t height = chain->CreateInfo().imageExtent.height;

   assert(chain->DevMemBacking().size() == 0);
   for (size_t i = 0; i < chain->Images().size(); ++i)
   {
      assert(chain->Images()[i]->Extent().width  == width);
      assert(chain->Images()[i]->Extent().height == height);
      assert(chain->Images()[i]->Extent().depth  == 1);
      assert(chain->Images()[i]->Format() == format);

      ImageData &image = chain->PlatformImageData()[i];
      CreateBuffer(image, chain, format, width, height);

      VkMemoryRequirements memReq;
      chain->Images()[i]->GetImageMemoryRequirements(pDevice, &memReq);
      assert(image.m_buffer.info.byteSize >= memReq.size);
      assert((image.m_buffer.info.physicalOffset & (memReq.alignment - 1)) == 0);

      NEXUS_FlushCache(image.m_buffer.info.cachedAddr, memReq.size);

      gmem_handle_t external = gmem_from_external_memory(nullptr, nullptr,
            image.m_buffer.info.physicalOffset, image.m_buffer.info.cachedAddr,
            memReq.size, "Wayland swapchain");
      try
      {
         DeviceMemory *devMem = createObject<DeviceMemory,
               VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(pCallbacks, &g_defaultAllocCallbacks,
                                                    external);
         chain->DevMemBacking().push_back(devMem);
         chain->Images()[i]->BindImageMemory(pDevice, devMem, /*offset=*/0);
      }
      catch(...)
      {
         gmem_free(external);
         throw;
      }
   }
}

void V3DPlatformWayland::DetachSwapchain(
   const VkAllocationCallbacks *pCallbacks,
   Device                      *pDevice,
   SwapchainKHR                *chain)
{
   for (DeviceMemory *devMem: chain->DevMemBacking())
      destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(
            devMem, pCallbacks);
   chain->DevMemBacking().clear();

   SwapchainData &psd = chain->PlatformSwapchainData();
   for (ImageData &image: chain->PlatformImageData())
   {
      if (image.m_created)
      {
         psd.m_surface->DestroyBuffer(image.m_buffer);
         image.m_created = false;
      }
   }
   psd.m_current = false;
   psd.m_surface = nullptr;
}

VkResult V3DPlatformWayland::QueueFrame(
   Device                 *device,
   const VkPresentInfoKHR *presentInfo)
{
   // Wait for the waitSemaphores to fire
   for (uint32_t s = 0; s < presentInfo->waitSemaphoreCount; s++)
   {
      Semaphore *sem = fromHandle<Semaphore>(presentInfo->pWaitSemaphores[s]);
      sem->WaitNow();
   }

   for (uint32_t c = 0; c < presentInfo->swapchainCount; c++)
   {
      SwapchainKHR *chain = fromHandle<SwapchainKHR>(presentInfo->pSwapchains[c]);
      SwapchainData &psd = chain->PlatformSwapchainData();
      uint32_t      indx = presentInfo->pImageIndices[c];
      ImageData &image = chain->PlatformImageData()[indx];
      assert(image.m_buffer.buffer_release_callback == BufferReleaseCallback);
      bool wait = chain->CreateInfo().presentMode == VK_PRESENT_MODE_FIFO_KHR;
      psd.m_surface->QueueBuffer(image.m_buffer, wait);
   }

   return VK_SUCCESS;
}

VkResult V3DPlatformWayland::DequeueFrame(
   SwapchainKHR *chain,
   uint64_t      timeout,
   Semaphore    *semaphore,
   Fence        *fence,
   uint32_t     *pImageIndex)
{
   SwapchainData &psd = chain->PlatformSwapchainData();
   if (!psd.m_current)
      return VK_ERROR_OUT_OF_DATE_KHR;

   std::unique_lock<std::mutex> lock { psd.m_mutex };

   int found;
   auto available = [this, chain, &found]{ return (found = FindAvailableBuffer(chain)) >= 0; };
   if (timeout == UINT64_MAX)
      psd.m_released.wait(lock, available);
   else
      psd.m_released.wait_for(lock, std::chrono::nanoseconds(timeout), available);

   if (found >= 0)
   {
      *pImageIndex = found;
      if (fence != nullptr)
         fence->Signal();

      if (semaphore != nullptr)
         semaphore->SignalNow();

      return VK_SUCCESS;
   }
   else
   {
      return timeout > 0 ? VK_TIMEOUT : VK_NOT_READY;
   }
}

VkResult V3DPlatformWayland::GetSurfaceCapabilities(
   VkSurfaceKHR                surface,
   VkSurfaceCapabilitiesKHR   *pSurfaceCapabilities)
{
   VkResult result = V3DPlatformBase::GetSurfaceCapabilities(surface,
         pSurfaceCapabilities);
   if (result == VK_SUCCESS)
   {
      pSurfaceCapabilities->minImageCount = 3;
      pSurfaceCapabilities->maxImageCount = 0;
      // keep pSurfaceCapabilities->minImageExtent
      // keep pSurfaceCapabilities->maxImageExtent
      // keep pSurfaceCapabilities->maxImageArrayLayers
      // keep pSurfaceCapabilities->supportedTransforms
      // keep pSurfaceCapabilities->currentTransform
      pSurfaceCapabilities->supportedCompositeAlpha = //match Weston GL renerer
            //VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
      pSurfaceCapabilities->supportedUsageFlags =
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   }
   return result;
}

VkResult V3DPlatformWayland::GetSurfacePresentModes(
   VkSurfaceKHR       surface,
   uint32_t          *pPresentModeCount,
   VkPresentModeKHR  *pPresentModes)
{
   const VkPresentModeKHR  modes[] =
   {
      VK_PRESENT_MODE_MAILBOX_KHR,
      VK_PRESENT_MODE_FIFO_KHR
   };

   return sizedListQuery<VkPresentModeKHR>(pPresentModes, pPresentModeCount,
                                           modes, countof(modes));
}

void V3DPlatformWayland::GetSurfaceExtent(
   VkExtent2D  *extent,
   VkSurfaceKHR surface) const
{
   SurfaceWayland *surf = FromSurfaceKHR<SurfaceWayland>(surface);

   // Wayland surfaces have no size. The size of the surface is calculated
   // based on the attached buffer size transformed by the inverse
   // buffer_transform and the inverse buffer_scale.
   extent->width  = 0xFFFFFFFF;
   extent->height = 0xFFFFFFFF;
}

static BEGL_BufferFormat ToBeglFormat(VkFormat vkFormat)
{
   switch (vkFormat)
   {
   case VK_FORMAT_R8G8B8A8_UNORM:
   case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
      return BEGL_BufferFormat_eA8B8G8R8;
   default:
      return BEGL_BufferFormat_INVALID;
   }
}

void V3DPlatformWayland::CreateBuffer(ImageData &image, SwapchainKHR *chain,
      VKFormat format, uint32_t width, uint32_t height)
{
   assert(!image.m_created);
   image.m_platform   = this;
   image.m_chain      = chain;
   SwapchainData &psd = chain->PlatformSwapchainData();

   const BEGL_BufferFormat format = ToBeglFormat(format);
   assert(format != BEGL_BufferFormat_INVALID);

   memset(&image.m_buffer, 0, sizeof(image.m_buffer));

   static const struct wl_buffer_listener listener =
   {
      .release = BufferReleaseCallback,
   };

   psd.m_surface->CreateBuffer(image.m_buffer, width, height,
         format, listener, &image);

   image.m_created   = true;
   image.m_available = true;
}

int V3DPlatformWayland::FindAvailableBuffer(SwapchainKHR *chain)
{
   for (size_t i = 0; i < chain->PlatformImageData().size(); i++)
   {
      ImageData &image = chain->PlatformImageData()[i];
      if (image.m_available)
      {
         return i;
      }
   }
   return -1; // not found;
}

/*static*/ void V3DPlatformWayland::BufferReleaseCallback(void *data,
      struct wl_buffer *buffer)
{
   ImageData *image = static_cast<ImageData *>(data);
   assert(image->m_buffer.buffer == buffer);
   image->m_platform->BufferRelease(*image);
}

void V3DPlatformWayland::BufferRelease(ImageData &image)
{
   SwapchainData &psd = image.m_chain->PlatformSwapchainData();
   {
      std::unique_lock<std::mutex> lock { psd.m_mutex };
      image.m_available = true;
   }
   psd.m_released.notify_one();
}

uint32_t V3DPlatformWayland::GetDriverVersion() const
{
   char relStr[128];
   NEXUS_Platform_GetReleaseVersion(relStr, 128);

   // The release string looks like "97271 A0 17.2" - we want the 17.2 bit
   uint32_t maj, min;
   sscanf(relStr, "%*s %*s %u.%u", &maj, &min);

   assert(maj >= 17);   // Vulkan driver didn't exist before 2017

   // Encode, but leave human readable
   return maj * 10000 + min * 100;
}

} // namespace bvk

#endif // VK_USE_PLATFORM_WAYLAND_KHR
