/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "Dispatchable.h"
#include "V3DPlatform.h"
#include "Formats.h"
#include "TileStateMemory.h"

namespace bvk {

// Note: this is a singleton and cannot, therefore, be dispatchable.
// Dispatchable handles must be unique. This gets wrapped in a dispatchable
// wrapper inside Instance.
class PhysicalDevice: public NonCopyable
{
public:
   PhysicalDevice();
   ~PhysicalDevice() noexcept;

   void GetPhysicalDeviceFeatures(
      VkPhysicalDeviceFeatures   *pFeatures) noexcept;

   void GetPhysicalDeviceFormatProperties(
      VkFormat              format,
      VkFormatProperties   *pFormatProperties) noexcept;

   VkResult GetPhysicalDeviceImageFormatProperties(
      VkFormat                 format,
      VkImageType              type,
      VkImageTiling            tiling,
      VkImageUsageFlags        usage,
      VkImageCreateFlags       flags,
      VkImageFormatProperties *pImageFormatProperties) noexcept;

   void GetPhysicalDeviceProperties(
      VkPhysicalDeviceProperties *pProperties) noexcept;

   void GetPhysicalDeviceQueueFamilyProperties(
      uint32_t                *pQueueFamilyPropertyCount,
      VkQueueFamilyProperties *pQueueFamilyProperties) noexcept;

   void GetPhysicalDeviceMemoryProperties(
      VkPhysicalDeviceMemoryProperties *pMemoryProperties) noexcept;

   VkResult EnumerateDeviceExtensionProperties(
      const char              *pLayerName,
      uint32_t                *pPropertyCount,
      VkExtensionProperties   *pProperties) noexcept;

   VkResult EnumerateDeviceLayerProperties(
      uint32_t          *pPropertyCount,
      VkLayerProperties *pProperties) noexcept;

   void GetPhysicalDeviceSparseImageFormatProperties(
      VkFormat                       format,
      VkImageType                    type,
      VkSampleCountFlagBits          samples,
      VkImageUsageFlags              usage,
      VkImageTiling                  tiling,
      uint32_t                      *pPropertyCount,
      VkSparseImageFormatProperties *pProperties) noexcept;

   VkResult GetPhysicalDeviceSurfaceSupportKHR(
      uint32_t     queueFamilyIndex,
      VkSurfaceKHR surface,
      VkBool32    *pSupported) noexcept;

   VkResult GetPhysicalDeviceSurfaceCapabilitiesKHR(
      VkSurfaceKHR                surface,
      VkSurfaceCapabilitiesKHR   *pSurfaceCapabilities) noexcept;

   VkResult GetPhysicalDeviceSurfaceFormatsKHR(
      VkSurfaceKHR          surface,
      uint32_t             *pSurfaceFormatCount,
      VkSurfaceFormatKHR   *pSurfaceFormats) noexcept;

   VkResult GetPhysicalDeviceSurfacePresentModesKHR(
      VkSurfaceKHR       surface,
      uint32_t          *pPresentModeCount,
      VkPresentModeKHR  *pPresentModes) noexcept;

   VkResult GetPhysicalDeviceDisplayPropertiesKHR(
      uint32_t                *pPropertyCount,
      VkDisplayPropertiesKHR  *pProperties) noexcept;

   VkResult GetPhysicalDeviceDisplayPlanePropertiesKHR(
      uint32_t                      *pPropertyCount,
      VkDisplayPlanePropertiesKHR   *pProperties) noexcept;

   VkResult GetDisplayPlaneSupportedDisplaysKHR(
      uint32_t        planeIndex,
      uint32_t       *pDisplayCount,
      VkDisplayKHR   *pDisplays) noexcept;

   VkResult GetDisplayModePropertiesKHR(
      VkDisplayKHR                display,
      uint32_t                   *pPropertyCount,
      VkDisplayModePropertiesKHR *pProperties) noexcept;

   VkResult CreateDisplayModeKHR(
      VkDisplayKHR                      display,
      const VkDisplayModeCreateInfoKHR *pCreateInfo,
      const VkAllocationCallbacks      *pAllocator,
      VkDisplayModeKHR                 *pMode) noexcept;

   VkResult GetDisplayPlaneCapabilitiesKHR(
      VkDisplayModeKHR               mode,
      uint32_t                       planeIndex,
      VkDisplayPlaneCapabilitiesKHR *pCapabilities) noexcept;

#ifdef VK_USE_PLATFORM_XLIB_KHR

   VkBool32 GetPhysicalDeviceXlibPresentationSupportKHR(
      uint32_t  queueFamilyIndex,
      Display  *dpy,
      VisualID  visualID) noexcept;

#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

   VkBool32 GetPhysicalDeviceXcbPresentationSupportKHR(
      uint32_t           queueFamilyIndex,
      xcb_connection_t  *connection,
      xcb_visualid_t     visual_id) noexcept;

#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

   VkBool32 GetPhysicalDeviceWaylandPresentationSupportKHR(
      uint32_t           queueFamilyIndex,
      struct wl_display *display) noexcept;

#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

   VkBool32 GetPhysicalDeviceWin32PresentationSupportKHR(
      uint32_t  queueFamilyIndex) noexcept;

#endif // VK_USE_PLATFORM_WIN32_KHR

   void GetPhysicalDeviceFeatures2KHR(
      VkPhysicalDeviceFeatures2KHR  *pFeatures) noexcept;

   void GetPhysicalDeviceProperties2KHR(
      VkPhysicalDeviceProperties2KHR   *pProperties) noexcept;

   void GetPhysicalDeviceFormatProperties2KHR(
      VkFormat                 format,
      VkFormatProperties2KHR  *pFormatProperties) noexcept;

   VkResult GetPhysicalDeviceImageFormatProperties2KHR(
      const                          VkPhysicalDeviceImageFormatInfo2KHR*pImageFormatInfo,
      VkImageFormatProperties2KHR   *pImageFormatProperties) noexcept;

   void GetPhysicalDeviceQueueFamilyProperties2KHR(
      uint32_t                      *pQueueFamilyPropertyCount,
      VkQueueFamilyProperties2KHR   *pQueueFamilyProperties) noexcept;

   void GetPhysicalDeviceMemoryProperties2KHR(
      VkPhysicalDeviceMemoryProperties2KHR   *pMemoryProperties) noexcept;

   void GetPhysicalDeviceSparseImageFormatProperties2KHR(
      const                                VkPhysicalDeviceSparseImageFormatInfo2KHR*pFormatInfo,
      uint32_t                            *pPropertyCount,
      VkSparseImageFormatProperties2KHR   *pProperties) noexcept;

   // Implementation specific from this point on
public:
   // Acquire a pointer to the one physical device.
   // These are reference counted.
   static PhysicalDevice *Acquire();

   // Release a reference to the physical device.
   // The physical device will be destroyed when no more references are held.
   void Release();

   const VkPhysicalDeviceLimits &Limits() const { return m_limits; }

   bool ValidateRequestedFeatures(const VkPhysicalDeviceFeatures &test) const;

   uint32_t GetNumMemoryTypes() const
   {
      return m_memProps.memoryTypeCount;
   }

   VkMemoryType GetMemoryType(uint32_t memTypeIndex) const
   {
      assert(memTypeIndex < m_memProps.memoryTypeCount);
      return m_memProps.memoryTypes[memTypeIndex];
   }

#if !V3D_VER_AT_LEAST(4,1,34,0)
   TileStateMemory &TileStateMemoryManager() { return m_tileStateMemory; }
#endif

   // Match the flags to a memory type index.
   // Note: flags must match a valid memory type or a fatal_error exception will occur.
   uint32_t FindSuitableMemoryTypeIndex(VkMemoryPropertyFlags flags) const;

   uint32_t CalculateMemoryTypeBits(VkMemoryPropertyFlags mustHave,
                                    VkMemoryPropertyFlags exclude);

   V3DPlatform &Platform() { return m_platform; }

private:
   void InitFeatures();
   void InitLimits();
   void InitMemoryProps();
   void InitFormatProperties();
   void InitCompilerTables();
   void FPSet(VkFormat fmt, VkFormatProperties flags);

   const VkFormatProperties &FormatProperty(uint32_t indx) const
   {
      // The format list has very large values at the end, outside of the prescribed range.
      // These presumably are extensions, or experimental. We need to ignore them to avoid
      // a huge buffer index.
      if (indx <= VK_FORMAT_END_RANGE)
         return m_formatProperties[indx];

      return m_formatProperties[VK_FORMAT_UNDEFINED];
   }

private:
   V3DPlatform                      m_platform;
   VkPhysicalDeviceFeatures         m_features;
   VkPhysicalDeviceLimits           m_limits;
   VkPhysicalDeviceMemoryProperties m_memProps;
   VkPhysicalDeviceSparseProperties m_sparseProps;
   VkQueueFamilyProperties          m_queueFamilyProps;

   // This should only be accessed via FormatProperty() to avoid index errors
   VkFormatProperties               m_formatProperties[VK_FORMAT_RANGE_SIZE];
#if !V3D_VER_AT_LEAST(4,1,34,0)
   TileStateMemory                  m_tileStateMemory;
#endif
};

} // namespace bvk
