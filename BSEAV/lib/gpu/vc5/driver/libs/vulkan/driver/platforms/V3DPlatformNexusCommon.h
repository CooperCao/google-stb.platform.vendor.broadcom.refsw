/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#ifdef VK_USE_PLATFORM_DISPLAY_KHR

#include "nexus_types.h"
#include "nexus_surface.h"

#include "BVKPlatform.h"
#include "V3DPlatformBase.h"

#include <mutex>
#include <cstring>

struct NEXUS_VideoFormatInfo;

// Set to 1 to turn on very noisy trace of the queue and dequeue
#define LOG_QUEUE_DEQUEUE 0

namespace bvk
{

struct SurfaceDisplay : public Allocating
{
   static constexpr uint32_t Magic = 0x5faced15;

   SurfaceDisplay(const VkAllocationCallbacks         *pAllocator,
                  const VkDisplaySurfaceCreateInfoKHR *pCreateInfo) :
      Allocating(pAllocator),
      displayMode(pCreateInfo->displayMode),
      planeIndex(pCreateInfo->planeIndex),
      planeStackIndex(pCreateInfo->planeStackIndex),
      transform(pCreateInfo->transform),
      globalAlpha(pCreateInfo->globalAlpha),
      alphaMode(pCreateInfo->alphaMode),
      imageExtent(pCreateInfo->imageExtent)
   {
   }

   uint32_t                       magic = Magic;
   VkDisplayModeKHR               displayMode;
   uint32_t                       planeIndex;
   uint32_t                       planeStackIndex;
   VkSurfaceTransformFlagBitsKHR  transform;
   float                          globalAlpha;
   VkDisplayPlaneAlphaFlagBitsKHR alphaMode;
   VkExtent2D                     imageExtent;
};

class V3DPlatformNexusCommon : public V3DPlatformBase
{
public:
   V3DPlatformNexusCommon();
   virtual ~V3DPlatformNexusCommon() = default;

   uint32_t NumRequiredSwapchainBuffers(uint32_t minBuffers) override;

   VkResult GetSurfaceCapabilities(
      VkSurfaceKHR                surface,
      VkSurfaceCapabilitiesKHR   *pSurfaceCapabilities) override;

   VkResult GetSurfaceFormats(
      VkSurfaceKHR          surface,
      uint32_t             *pSurfaceFormatCount,
      VkSurfaceFormatKHR   *pSurfaceFormats) override;

   VkResult GetSurfacePresentModes(
      VkSurfaceKHR       surface,
      uint32_t          *pPresentModeCount,
      VkPresentModeKHR  *pPresentModes) override;

   void GetSurfaceExtent(VkExtent2D *extent, VkSurfaceKHR surf) const;

   VkResult GetDisplayProperties(
      uint32_t                *pPropertyCount,
      VkDisplayPropertiesKHR  *pProperties) override;

   VkResult GetDisplayModeProperties(
      VkDisplayKHR                display,
      uint32_t                   *pPropertyCount,
      VkDisplayModePropertiesKHR *pProperties) override;

   VkResult GetDisplayPlaneSupportedDisplaysKHR(
      uint32_t        planeIndex,
      uint32_t       *pDisplayCount,
      VkDisplayKHR   *pDisplays) override;

   bool HasSurfacePresentationSupport(VkSurfaceKHR surface) override;

   // Return a driver version that can be reported via PhysicalDevice
   uint32_t GetDriverVersion() const override;

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

public:
   // Platform specific image data. A vector of these is held in the swapchain.
   struct ImageData
   {
      // Default values so vector resize doesn't produce uninitialized entries
      bool                acquired = false;
      NEXUS_SurfaceHandle handle = nullptr;
   };

protected:
   // Step methods called by (De)QueueFrame implementations which are
   // provided by the two Nexus derived classes.
   virtual VkResult QueueNexusSurface(
      SwapchainKHR                  *chain,
      NEXUS_SurfaceHandle            nexusSurface,
      SchedDependencies             &deps,
      const VkDisplayPresentInfoKHR *presentInfo) = 0;

   virtual NEXUS_SurfaceHandle DequeueNexusSurface(
      SwapchainKHR *chain,
      uint64_t      timeout) = 0;

   // Helpers for managing NEXUS_SurfaceHandle and the platform
   // image data vector in swapchains containing surface handles. These
   // are used by the two Nexus derived classes and the scoped classes
   // such as V3DPlatformNxClient::NxClientSurface.
   static void AllocateSwapchainImages(
      const VkAllocationCallbacks *pCallbacks,
      Device                      *pDevice,
      SwapchainKHR                *chain);

   static void DestroySwapchainImages(
      const VkAllocationCallbacks *pCallbacks,
      SwapchainKHR *chain);

   static uint32_t FindImageDataIndexFromHandle(
      SwapchainKHR       *chain,
      NEXUS_SurfaceHandle handle);

   static void DestroySurfaceHandle(NEXUS_SurfaceHandle handle);

   static inline void DestroyImageData(ImageData &imageData)
   {
      DestroySurfaceHandle(imageData.handle);
      std::memset(&imageData, 0, sizeof(imageData));
   }

   // Swapchain state helpers
   static VkDisplayPlaneAlphaFlagBitsKHR GetAlphaMode(const SwapchainKHR *chain);
   static void ValidateNewSwapchain(const SwapchainKHR *newChain, const SwapchainKHR *currentChain);

   // Vulkan <-> Nexus display and format helpers
   static void GetDisplayModeInfo(VkDisplayModeKHR mode, NEXUS_VideoFormatInfo *info);
   static NEXUS_PixelFormat VKToNexusFormat(VkFormat format);

   void CommonInitialize();
   void CommonTerminate();

protected:
   std::mutex                 m_displayMutex;
   BVKPlatform                m_bvkPlatform;
   NEXUS_VideoFormat          m_mainDisplayFormat = NEXUS_VideoFormat_eUnknown;

   unsigned                   m_numPlanes;
   VkDisplayPropertiesKHR     m_displayProperties;
   VkDisplayModePropertiesKHR m_displayModeProperties;

   // See the comment in the cpp file for what the following is all about.
protected:
   static std::mutex      s_validContextMutex;
   static std::set<void*> s_validContextPointers;
   static uint8_t         s_contextGeneration;

   static inline void *xorContextPointer(void *p)
   {
      uintptr_t xorVal = (uintptr_t)s_contextGeneration << ((sizeof(uintptr_t)-1) * CHAR_BIT);
      return (void*)((uintptr_t)p ^ xorVal);
   }
};

// Whenever V3DPlatformSurface is seen, it means SurfaceDisplay
using V3DPlatformSurface = SurfaceDisplay;

} // namespace bvk

#endif // EMBEDDED_SETTOP_BOX
