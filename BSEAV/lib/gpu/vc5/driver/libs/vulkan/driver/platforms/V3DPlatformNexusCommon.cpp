/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#if EMBEDDED_SETTOP_BOX && defined(VK_USE_PLATFORM_DISPLAY_KHR)

#include <cstring>

#include "V3DPlatformBase.h"
#include "Common.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DeviceMemory.h"
#include "Image.h"
#include "SwapchainKHR.h"
#include "Semaphore.h"
#include "Fence.h"

#include "libs/platform/v3d_scheduler.h"
#include "libs/platform/v3d_parallel.h"
#include "libs/util/profile/profile.h"

#include "nexus_memory.h"
#include "nexus_platform.h"
#include "nexus_core_utils.h"

LOG_DEFAULT_CAT("bvk::V3DPlatform");

namespace bvk
{
// We believe, from the GL driver development, that pending NEXUS callbacks
// can arrive after we have told them to stop. This is obviously a problem
// because a raw context pointer referring to the object the callback is for
// cannot be guaranteed to still be valid if we are destroying the platform.
//
//****************************************************************************
// The strategy is to xor the context pointers with a generation identifier
// which is incremented each time the platform is created as part of the
// physical device singleton. A set of current valid (non-xor'd) context
// pointers will be maintained and the callbacks will xor the passed context
// with the current generation identifier and check that the resulting pointer
// is in the valid set.
//
// This is being very over cautious, but the intent is to prevent any possible
// issue with the app destroying the instance, re-creating it immediately and
// old pointers getting re-used [because of the behaviour of an application
// provided memory allocator], then getting a late NEXUS callback meant for an
// object in the previous instance.
//
// Having said all of this, while implementing this scheme we can see that
// for the recycle callbacks at least, the NEXUS surface client implementation
// blocks waiting for at least currently executing callbacks to complete
// when you change the settings to disable them. So it is entirely possible
// that none of this is necessary for this specific implementation.
std::mutex      V3DPlatformNexusCommon::s_validContextMutex {};
std::set<void*> V3DPlatformNexusCommon::s_validContextPointers {};
uint8_t         V3DPlatformNexusCommon::s_contextGeneration = 0;

V3DPlatformNexusCommon::V3DPlatformNexusCommon()
{
   std::lock_guard<std::mutex> contextLock { s_validContextMutex };
   s_contextGeneration++;
}

void V3DPlatformNexusCommon::CommonInitialize()
{
   // Init the abstract platform memory and scheduler interfaces
   m_bvkPlatform.Register();

   // Now bring up the common platform parts
   V3DPlatformBase::CommonInitialize();

   std::lock_guard<std::mutex> contextLock { s_validContextMutex };
   // Allow registered vsync/display callbacks to now call into this object
   s_validContextPointers.insert(this);
}

void V3DPlatformNexusCommon::CommonTerminate()
{
   V3DPlatformBase::CommonTerminate();
   // Invalidate all context pointers so any pending callbacks
   // during the rest of the cleanup will safely no nothing.
   std::lock_guard<std::mutex> contextLock { s_validContextMutex };
   s_validContextPointers.clear();
}

uint32_t V3DPlatformNexusCommon::NumRequiredSwapchainBuffers(
   uint32_t minBuffers)
{
   // We at least need double buffering
   return std::max(minBuffers, 2u);
}

VkResult V3DPlatformNexusCommon::GetSurfaceCapabilities(
   VkSurfaceKHR                surface,
   VkSurfaceCapabilitiesKHR   *pSurfaceCapabilities)
{
   pSurfaceCapabilities->minImageCount             = 2;
   pSurfaceCapabilities->maxImageCount             = 0; // No maximum, just limited by memory
   pSurfaceCapabilities->minImageExtent            = { 1, 1 };
   pSurfaceCapabilities->maxImageExtent            = { 4096, 4096 };

   // Vulkan only supports 3D in HDMI "frame packing" style by having two full
   // size image arrays attached to each swapchain image.
   //
   // If the application wants to do side-by-side or top/bottom it would have
   // to do it itself inside a single swapchain image.
   //
   // TODO: Does Nexus support these HDMI modes? And do we care about 3D anyway?
   //
   pSurfaceCapabilities->maxImageArrayLayers       = 1;
   pSurfaceCapabilities->supportedTransforms       = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   pSurfaceCapabilities->currentTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

   pSurfaceCapabilities->supportedCompositeAlpha   = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR |
                                                     VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR |
                                                     VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
                                                     VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

   pSurfaceCapabilities->supportedUsageFlags       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | // Direct render
                                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |     // Blit, Explicit MS resolve into display buffer
                                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT;      // Copy out (read pixels)

   GetSurfaceExtent(&pSurfaceCapabilities->currentExtent, surface);

   return VK_SUCCESS;
}

VkResult V3DPlatformNexusCommon::GetSurfaceFormats(
   VkSurfaceKHR          surface,
   uint32_t             *pSurfaceFormatCount,
   VkSurfaceFormatKHR   *pSurfaceFormats)
{
   const VkSurfaceFormatKHR formats[] =
   {
      { VK_FORMAT_R8G8B8A8_UNORM,        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
      { VK_FORMAT_A8B8G8R8_UNORM_PACK32, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
      { VK_FORMAT_R8G8B8A8_SRGB,         VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
      { VK_FORMAT_A8B8G8R8_SRGB_PACK32,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
      { VK_FORMAT_R5G6B5_UNORM_PACK16,   VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
      { VK_FORMAT_R4G4B4A4_UNORM_PACK16, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
      { VK_FORMAT_R5G5B5A1_UNORM_PACK16, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
   };

   return sizedListQuery<VkSurfaceFormatKHR>(pSurfaceFormats, pSurfaceFormatCount,
                                             formats, countof(formats));
}

VkResult V3DPlatformNexusCommon::GetSurfacePresentModes(
   VkSurfaceKHR       surface,
   uint32_t          *pPresentModeCount,
   VkPresentModeKHR  *pPresentModes)
{
   const VkPresentModeKHR  modes[] =
   {
      VK_PRESENT_MODE_FIFO_KHR,
      VK_PRESENT_MODE_MAILBOX_KHR
   };

   return sizedListQuery<VkPresentModeKHR>(pPresentModes, pPresentModeCount,
                                           modes, countof(modes));
}

NEXUS_PixelFormat V3DPlatformNexusCommon::VKToNexusFormat(VkFormat format)
{
   switch (format)
   {
      // NOTE: Vulkan does not have formats with "don't care" alpha
      //       and Nexus does not currently support R10G10B10A2 surfaces. So
      //       there are not very many presentable formats.
      //
      // This mapping assumes a Little endian system and that we will never
      // support big endian.
      case VK_FORMAT_R8G8B8A8_UNORM:
      case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
      case VK_FORMAT_R8G8B8A8_SRGB:
      case VK_FORMAT_A8B8G8R8_SRGB_PACK32:  return NEXUS_PixelFormat_eA8_B8_G8_R8;
      case VK_FORMAT_R5G6B5_UNORM_PACK16:   return NEXUS_PixelFormat_eR5_G6_B5;
      case VK_FORMAT_R4G4B4A4_UNORM_PACK16: return NEXUS_PixelFormat_eR4_G4_B4_A4;
      case VK_FORMAT_R5G5B5A1_UNORM_PACK16: return NEXUS_PixelFormat_eR5_G5_B5_A1;
      default:                              break;
   }
   return NEXUS_PixelFormat_eUnknown;
}

void V3DPlatformNexusCommon::GetSurfaceExtent(
   VkExtent2D  *extent,
   VkSurfaceKHR surf) const
{
   const auto *displaySurface = FromSurfaceKHR<SurfaceDisplay>(surf);
   *extent = displaySurface->imageExtent;
}

VkResult V3DPlatformNexusCommon::GetDisplayProperties(
   uint32_t                *pPropertyCount,
   VkDisplayPropertiesKHR  *pProperties)
{
   std::lock_guard<std::mutex> lock { m_displayMutex };

   return sizedListQuery<VkDisplayPropertiesKHR>(pProperties, pPropertyCount, &m_displayProperties, 1);
}

VkResult V3DPlatformNexusCommon::GetDisplayModeProperties(
   VkDisplayKHR                display,
   uint32_t                   *pPropertyCount,
   VkDisplayModePropertiesKHR *pProperties)
{
   std::lock_guard<std::mutex> lock { m_displayMutex };

   assert(display == m_displayProperties.display);

   return sizedListQuery<VkDisplayModePropertiesKHR>(pProperties, pPropertyCount,
                                                     &m_displayModeProperties, 1);
}

VkResult V3DPlatformNexusCommon::GetDisplayPlaneSupportedDisplaysKHR(
   uint32_t        planeIndex,
   uint32_t       *pDisplayCount,
   VkDisplayKHR   *pDisplays)
{
   std::lock_guard<std::mutex> lock { m_displayMutex };

   assert(planeIndex < m_numPlanes);

   VkDisplayKHR planeDisplays = m_displayProperties.display;

   return sizedListQuery<VkDisplayKHR>(pDisplays, pDisplayCount, &planeDisplays, 1);
}

bool V3DPlatformNexusCommon::HasSurfacePresentationSupport(VkSurfaceKHR surface)
{
   const auto *displaySurface = fromHandle<SurfaceDisplay>(surface);
   if (displaySurface->magic != SurfaceDisplay::Magic)
      return false;

   // Note: we are not allowing the Vulkan API to change the display mode
   // in exclusive mode at this point.
   return displaySurface->planeIndex < m_numPlanes &&
          displaySurface->displayMode == m_displayModeProperties.displayMode;
}

uint32_t V3DPlatformNexusCommon::GetDriverVersion() const
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

void V3DPlatformNexusCommon::AllocateSwapchainImages(
      const VkAllocationCallbacks *pCallbacks,
      Device                      *pDevice,
      SwapchainKHR                *chain)
{
   const auto ci = chain->CreateInfo();

   NEXUS_SurfaceCreateSettings createSettings;
   NEXUS_Surface_GetDefaultCreateSettings(&createSettings);

   createSettings.pixelFormat       = VKToNexusFormat(ci.imageFormat);
   createSettings.width             = ci.imageExtent.width;
   createSettings.height            = ci.imageExtent.height;
   createSettings.pixelMemoryOffset = 0;

   log_trace("AllocateSwapchainImages: %zu (%ux%u) images NEXUS format = %u",
      chain->Images().size(),
      createSettings.width, createSettings.height,
      createSettings.pixelFormat);

   assert(chain->PlatformImageData().size() == chain->Images().size());

   try
   {
      auto imageDataItr = chain->PlatformImageData().begin();

      for (auto image: chain->Images())
      {
         VkMemoryRequirements memReq;
         image->GetImageMemoryRequirements(pDevice, &memReq);

         // Framebuffer heap 0 is always GFD accessible
         //
         // TODO: allow secure framebuffers
         constexpr bool secure = false;
         NEXUS_HeapHandle heap = NEXUS_Platform_GetFramebufferHeap(
               secure ? NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE : 0);

         createSettings.pixelMemory = NEXUS_MemoryBlock_Allocate(
               heap, memReq.size, 4096, nullptr);

         if (createSettings.pixelMemory == nullptr)
            throw bvk::bad_device_alloc();

         imageDataItr->handle = NEXUS_Surface_Create(&createSettings);
         if (imageDataItr->handle == nullptr)
         {
            NEXUS_MemoryBlock_Free(createSettings.pixelMemory);
            createSettings.pixelMemory = nullptr;
            throw std::bad_alloc();
         }

         log_trace("   Created NEXUS_Surface: %p", (void*)imageDataItr->handle);

         imageDataItr->acquired = false;

         NEXUS_Addr offset;
         NEXUS_MemoryBlock_LockOffset(createSettings.pixelMemory, &offset);
         void *cpuAddr = nullptr;

         // Secure memory can't be mapped as the ARM will try to access it
         if (!secure)
         {
            NEXUS_MemoryBlock_Lock(createSettings.pixelMemory, &cpuAddr);
            NEXUS_FlushCache(cpuAddr, memReq.size);
         }

         gmem_handle_t external = gmem_from_external_memory(
               nullptr, nullptr, offset, cpuAddr, memReq.size,
               "VkSwapChainKHR Image");

         if (external == GMEM_HANDLE_INVALID)
            throw bvk::bad_device_alloc();

         DeviceMemory *devMem = createObject<DeviceMemory, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(
               pCallbacks, &g_defaultAllocCallbacks, external);

         chain->DevMemBacking().push_back(devMem);

         ++imageDataItr;
      }
   }
   catch(...)
   {
      for (auto devMem: chain->DevMemBacking())
         destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(devMem, pCallbacks);

      // Clear the vector so we do not destroy these again when detach is called
      // as part of the Swapchain creation exception handling.
      chain->DevMemBacking().clear();

      for (auto imageData: chain->PlatformImageData())
         DestroyImageData(imageData);

      throw;
   }

   // Now everything is allocated successfully, bind the memory objects
   // to the swapchain images.
   auto devMemItr = chain->DevMemBacking().begin();
   for (auto image: chain->Images())
      image->BindImageMemory(pDevice, *devMemItr++, 0);
}

void V3DPlatformNexusCommon::DestroySwapchainImages(
      const VkAllocationCallbacks *pCallbacks,
      SwapchainKHR                *chain)
{
   log_trace("DestroySwapchainImages: chain = %p", chain);

   for (auto devMem: chain->DevMemBacking())
      destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(devMem, pCallbacks);

   // We must clear the vector as this is asserted for
   // in SwapchainKHR::Cleanup()
   chain->DevMemBacking().clear();

   // The image data cleanup is not so straight forward. Some images may have
   // already been cleaned up as part of another swapchain bind if it was a
   // descendant of this one. And we have to not disturb images from this
   // swapchain currently presented on the display.
   //
   // So destroy application acquired images now..
   for (auto imageData: chain->PlatformImageData())
   {
      if (imageData.acquired)
         DestroyImageData(imageData);
      else
         std::memset(&imageData, 0, sizeof(imageData));
   }
}

uint32_t V3DPlatformNexusCommon::FindImageDataIndexFromHandle(
      SwapchainKHR       *chain,
      NEXUS_SurfaceHandle handle)
{
   uint32_t index = 0;
   for(auto i: chain->PlatformImageData())
   {
      if (i.handle == handle)
         break;

      index++;
   }

   assert(index < chain->PlatformImageData().size());
   return index;
}

void V3DPlatformNexusCommon::DestroySurfaceHandle(NEXUS_SurfaceHandle handle)
{
   if (handle != nullptr)
   {
      NEXUS_SurfaceMemoryProperties memProperties;
      NEXUS_Surface_GetMemoryProperties(handle, &memProperties);

      log_trace("   Destroying NEXUS_Surface: %p", (void*)handle);
      NEXUS_Surface_Destroy(handle);

      // This will also unmap and unlock the memory block so no need
      // to do that explicitly
      NEXUS_MemoryBlock_Free(memProperties.pixelMemory);
   }
}

void V3DPlatformNexusCommon::GetDisplayModeInfo(
      VkDisplayModeKHR mode,
      NEXUS_VideoFormatInfo *info)
{
   // NOTE: a deliberate C style cast here to cope with the different
   // definitions of VkDisplayModeKHR in 32 and 64 bit builds. The
   // two step cast is necessary to avoid warnings in this case.
   const auto format = (NEXUS_VideoFormat)((uintptr_t)mode);
   assert(format != NEXUS_VideoFormat_eUnknown);

   NEXUS_VideoFormat_GetInfo(format, info);
}

VkResult V3DPlatformNexusCommon::QueueFrame(
   Device                 *device,
   const VkPresentInfoKHR *presentInfo)
{
   SchedDependencies deps {};
   ConvertPresentSemaphoresToDeps(presentInfo, &deps);

   auto dpi = static_cast<const VkDisplayPresentInfoKHR*>(presentInfo->pNext);

   VkResult res = VK_SUCCESS;

   for (uint32_t c = 0; c < presentInfo->swapchainCount; c++)
   {
      SwapchainKHR *chain = fromHandle<SwapchainKHR>(presentInfo->pSwapchains[c]);
      uint32_t imageIndex = presentInfo->pImageIndices[c];

#if LOG_QUEUE_DEQUEUE
      const auto *displaySurface = FromSurfaceKHR<SurfaceDisplay>(chain->CreateInfo().surface);
      log_trace("QueueFrame: chain = %p imageIndex = %u planeIndex = %u",
         chain, imageIndex, displaySurface->planeIndex);
#endif

      auto nexusSurface = chain->PlatformImageData()[imageIndex].handle;
      VkResult err = QueueNexusSurface(chain, nexusSurface, deps, dpi);

      if (presentInfo->pResults != nullptr)
         presentInfo->pResults[c] = err;

      // There is a defined preference of which error code to return
      // from the API if different swapchains return different errors.
      switch (res)
      {
         case VK_ERROR_DEVICE_LOST:
            break;
         case VK_ERROR_SURFACE_LOST_KHR:
            if (err == VK_ERROR_DEVICE_LOST)
               res = err;
            break;
         case VK_ERROR_OUT_OF_DATE_KHR:
            if (err == VK_ERROR_DEVICE_LOST || err == VK_ERROR_SURFACE_LOST_KHR)
               res = err;
            break;
         case VK_SUBOPTIMAL_KHR:
            if (err != VK_SUCCESS)
               res = err;
            break;
         case VK_SUCCESS:
            res = err;
            break;
         default:
            unreachable();
      }

      // Mark the image as no longer acquired. If the image was out of date
      // it's underlying resource will have been destroyed by the Nexus surface
      // queue call and it will never be able to be re-acquired by
      // the application.
      chain->PlatformImageData()[imageIndex].acquired  = false;
   }

   return res;
}

VkResult V3DPlatformNexusCommon::DequeueFrame(
   SwapchainKHR *chain,
   uint64_t      timeout,
   Semaphore    *semaphore,
   Fence        *fence,
   uint32_t     *pImageIndex)
{
#if LOG_QUEUE_DEQUEUE
   const auto *displaySurface = FromSurfaceKHR<SurfaceDisplay>(chain->CreateInfo().surface);
   log_trace("DequeueFrame: chain = %p planeIndex = %u", chain, displaySurface->planeIndex);
#endif

   NEXUS_SurfaceHandle h = DequeueNexusSurface(chain, timeout);

   if (h == nullptr)
      return timeout == 0u ? VK_NOT_READY: VK_TIMEOUT;

   uint32_t index = FindImageDataIndexFromHandle(chain, h);

   chain->PlatformImageData()[index].acquired  = true;
   *pImageIndex = index;

   if (fence != nullptr)
      fence->Signal();

   if (semaphore != nullptr)
      semaphore->SignalNow();

   return VK_SUCCESS;
}

VkDisplayPlaneAlphaFlagBitsKHR V3DPlatformNexusCommon::GetAlphaMode(
   const SwapchainKHR *chain)
{
   const auto *displaySurface = FromSurfaceKHR<SurfaceDisplay>(chain->CreateInfo().surface);

   // We use the swapchain alpha composition if there is one, if not then
   // use the surface's mode again if there is one, failing all that fall
   // back to pre-multiplied per-pixel alpha.
   VkDisplayPlaneAlphaFlagBitsKHR alphaMode;
   switch (chain->CreateInfo().compositeAlpha)
   {
      case VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR:
         alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR; break;
      case VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR:
         alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR; break;
      case VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR:
         alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR; break;
      case VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR:
      default:
         alphaMode = displaySurface->alphaMode; break;
   }

   // The spec allows the ICD display surface alpha mode to be 0, in which
   // case we should pick a sensible platform default.
   if (static_cast<uint32_t>(alphaMode) == 0u)
      alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_PREMULTIPLIED_BIT_KHR;

   return alphaMode;
}

void V3DPlatformNexusCommon::ValidateNewSwapchain(
   const SwapchainKHR *newChain,
   const SwapchainKHR *currentChain)
{
   const auto ci = newChain->CreateInfo();
   if (ci.oldSwapchain != VK_NULL_HANDLE)
   {
      auto *oldChain = fromHandle<SwapchainKHR>(ci.oldSwapchain);

      log_trace("%s: chain = %p, oldChain = %p", __FUNCTION__, newChain, oldChain);

      // Spec 1.0.32, section 29.6 WSI Swapchain:
      // Description of vkCreateSwapchainKHR states that the old and new chains
      // must have been created from the same surface.
      assert(oldChain->CreateInfo().surface == ci.surface);

      // Spec 1.0.32, section 29.6 WSI Swapchain:
      // Description of vkCreateSwapchainKHR states that the
      // underlying surface must either be associated with oldSwapchain
      // or not associated with any swapchain at this point.
      //
      // This is sort of couched as valid usage implying it is up the to
      // application to ensure this is the case, however this API can
      // return an error if the "window" is in use and that seems appropriate
      // for this case.
      if (currentChain != nullptr && currentChain != oldChain)
         throw VK_ERROR_NATIVE_WINDOW_IN_USE_KHR; // Caught in entrypoints

   }
   else if (currentChain != nullptr)
   {
      // Spec 1.0.32, section 29.6 WSI Swapchain:
      // The underlying surface must not be associated with any other swapchain
      // or a non-Vulkan API.
      throw VK_ERROR_NATIVE_WINDOW_IN_USE_KHR; // Caught in entrypoints
   }
   else
   {
      log_trace("%s: chain = %p, oldChain = (NULL)", __FUNCTION__, newChain);
   }

   assert(ci.presentMode == VK_PRESENT_MODE_FIFO_KHR || ci.presentMode == VK_PRESENT_MODE_MAILBOX_KHR);

   log_trace("   PresentMode = %s",
      ci.presentMode == VK_PRESENT_MODE_FIFO_KHR ? "FIFO" :
         ci.presentMode == VK_PRESENT_MODE_MAILBOX_KHR ? "MAILBOX" : "UNSUPPORTED");
}

} // namespace bvk

#endif // EMBEDDED_SETTOP_BOX
