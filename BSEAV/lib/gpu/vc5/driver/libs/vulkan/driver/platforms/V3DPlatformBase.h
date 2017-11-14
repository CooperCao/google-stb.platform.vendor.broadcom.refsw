/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Allocating.h"
#include "NonCopyable.h"
#include "ForwardDecl.h"
#include "SchedDependencies.h"

#include "libs/platform/v3d_platform.h"

struct VkSwapchainCreateInfoKHR;

namespace bvk
{

// Interface and common functionality for all platforms
class V3DPlatformBase : public NonCopyable
{
public:
   virtual ~V3DPlatformBase() = default;

   // Return platform specific swapchain count given minimum
   virtual uint32_t NumRequiredSwapchainBuffers(uint32_t minBuffers);

   // Attach platform specific swapchain backing buffers to a set of images
   virtual void AttachSwapchain(
      const VkAllocationCallbacks      *pCallbacks,
      Device                           *pDevice,
      SwapchainKHR                     *chain) = 0;

   // Detach platform specific swapchain stuff
   virtual void DetachSwapchain(
      const VkAllocationCallbacks      *pCallbacks,
      Device                           *pDevice,
      SwapchainKHR                     *chain) = 0;

   // Present a framebuffer for display
   virtual VkResult QueueFrame(
      Device                  *device,
      const VkPresentInfoKHR  *presentInfo) = 0;

   // Get a framebuffer to write into (with semaphores/fences to wait on)
   virtual VkResult DequeueFrame(
      SwapchainKHR   *chain,
      uint64_t        timeout,
      Semaphore      *semaphore,
      Fence          *fence,
      uint32_t       *pImageIndex) = 0;

   // Return a driver version that can be reported via PhysicalDevice
   virtual uint32_t GetDriverVersion() const = 0;

   // Platform virtualized implementation of the physical device surface
   // and display interfaces
   virtual VkResult GetSurfaceCapabilities(
      VkSurfaceKHR                surface,
      VkSurfaceCapabilitiesKHR   *pSurfaceCapabilities);

   virtual VkResult GetSurfaceFormats(
      VkSurfaceKHR          surface,
      uint32_t             *pSurfaceFormatCount,
      VkSurfaceFormatKHR   *pSurfaceFormats);

   virtual VkResult GetSurfacePresentModes(
      VkSurfaceKHR       surface,
      uint32_t          *pPresentModeCount,
      VkPresentModeKHR  *pPresentModes);

   virtual VkResult GetDisplayProperties(
      uint32_t                *pPropertyCount,
      VkDisplayPropertiesKHR  *pProperties);

   virtual VkResult GetDisplayPlaneProperties(
      uint32_t                      *pPropertyCount,
      VkDisplayPlanePropertiesKHR   *pProperties);

   virtual VkResult GetDisplayPlaneSupportedDisplaysKHR(
      uint32_t        planeIndex,
      uint32_t       *pDisplayCount,
      VkDisplayKHR   *pDisplays);

   virtual VkResult GetDisplayModeProperties(
      VkDisplayKHR                display,
      uint32_t                   *pPropertyCount,
      VkDisplayModePropertiesKHR *pProperties);

   virtual VkResult GetDisplayPlaneCapabilities(
      VkDisplayModeKHR               mode,
      uint32_t                       planeIndex,
      VkDisplayPlaneCapabilitiesKHR *pCapabilities);

   virtual bool HasSurfacePresentationSupport(VkSurfaceKHR surface);

   // Hardware version specific information
   uint32_t GetVPMSize() const  { return m_ident.vpm_size_in_multiples_of_8kb * 8192; }
   uint32_t GetNumCores() const { return m_hubIdent.num_cores; }

protected:
   // Initialize the platform
   void CommonInitialize();

   // Terminate the platform
   void CommonTerminate();

   void MaybeDumpPresentedImage(const Image &img);

   // Find the size of a surface
   virtual void GetSurfaceExtent(VkExtent2D *extent, VkSurfaceKHR surf) const = 0;

   // Helper for QueueFrame implementations
   void ConvertPresentSemaphoresToDeps(
      const VkPresentInfoKHR  *presentInfo,
      SchedDependencies       *deps);

protected:
   V3D_HUB_IDENT_T m_hubIdent = {};
   V3D_IDENT_T     m_ident = {};
};

// For simulation platforms that want to support DirectDisplay mode via a native window
class V3DPlatformWithFakeDirectDisplay : public V3DPlatformBase
{
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)
public:
   V3DPlatformWithFakeDirectDisplay();

   VkResult GetDisplayProperties(
      uint32_t               *pPropertyCount,
      VkDisplayPropertiesKHR *pProperties);

   VkResult GetDisplayModeProperties(
      VkDisplayKHR                display,
      uint32_t                   *pPropertyCount,
      VkDisplayModePropertiesKHR *pProperties);

   VkResult GetDisplayPlaneSupportedDisplaysKHR(
      uint32_t        planeIndex,
      uint32_t       *pDisplayCount,
      VkDisplayKHR   *pDisplays);

   VkResult GetDisplayPlaneProperties(
      uint32_t                      *pPropertyCount,
      VkDisplayPlanePropertiesKHR   *pProperties);

   VkResult GetDisplayPlaneCapabilities(
      VkDisplayModeKHR               mode,
      uint32_t                       planeIndex,
      VkDisplayPlaneCapabilitiesKHR *pCapabilities);

private:
   uint32_t                   m_numPlanes = 0;
   VkDisplayPropertiesKHR     m_displayProperties;
   VkDisplayModePropertiesKHR m_displayModeProperties;
#endif
};

template <typename T>
static T *FromSurfaceKHR(VkSurfaceKHR handle)
{
   T *surf = fromHandle<T>(handle);
   assert(surf->magic == T::Magic);
   return surf;
}

} // namespace bvk
