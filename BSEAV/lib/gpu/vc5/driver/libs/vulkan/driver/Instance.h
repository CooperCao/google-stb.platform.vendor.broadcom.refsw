/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "Dispatchable.h"
#include "PhysicalDevice.h"
#include "ProcAddressFinder.h"

namespace bvk {

class Instance: public NonCopyable, public Allocating, public Dispatchable
{
public:
   Instance(
      const VkAllocationCallbacks   *pCallbacks,
      const VkInstanceCreateInfo    *pCreateInfo);

   ~Instance() noexcept;

   VkResult EnumeratePhysicalDevices(
      uint32_t          *pPhysicalDeviceCount,
      VkPhysicalDevice  *pPhysicalDevices) noexcept;

   static PFN_vkVoidFunction GetInstanceProcAddr(
      bvk::Instance  *instance,
      const char     *pName) noexcept;

   static VkResult EnumerateInstanceExtensionProperties(
      const char              *pLayerName,
      uint32_t                *pPropertyCount,
      VkExtensionProperties   *pProperties) noexcept;

   static VkResult EnumerateInstanceLayerProperties(
      uint32_t          *pPropertyCount,
      VkLayerProperties *pProperties) noexcept;

   void DestroySurfaceKHR(
      VkSurfaceKHR                   surface,
      const VkAllocationCallbacks   *pAllocator) noexcept;

   VkResult CreateDisplayPlaneSurfaceKHR(
      const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
      const VkAllocationCallbacks         *pAllocator,
      VkSurfaceKHR                        *pSurface) noexcept;

#ifdef VK_USE_PLATFORM_XLIB_KHR

   VkResult CreateXlibSurfaceKHR(
      const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
      const VkAllocationCallbacks      *pAllocator,
      VkSurfaceKHR                     *pSurface) noexcept;

#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

   VkResult CreateXcbSurfaceKHR(
      const VkXcbSurfaceCreateInfoKHR  *pCreateInfo,
      const VkAllocationCallbacks      *pAllocator,
      VkSurfaceKHR                     *pSurface) noexcept;

#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

   VkResult CreateWaylandSurfaceKHR(
      const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
      const VkAllocationCallbacks         *pAllocator,
      VkSurfaceKHR                        *pSurface) noexcept;

#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

   VkResult CreateAndroidSurfaceKHR(
      const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
      const VkAllocationCallbacks         *pAllocator,
      VkSurfaceKHR                        *pSurface) noexcept;

#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

   VkResult CreateWin32SurfaceKHR(
      const VkWin32SurfaceCreateInfoKHR   *pCreateInfo,
      const VkAllocationCallbacks         *pAllocator,
      VkSurfaceKHR                        *pSurface) noexcept;

#endif // VK_USE_PLATFORM_WIN32_KHR

   void DebugReportMessageEXT(
      VkDebugReportFlagsEXT       flags,
      VkDebugReportObjectTypeEXT  objectType,
      uint64_t                    object,
      size_t                      location,
      int32_t                     messageCode,
      const char                 *pLayerPrefix,
      const char                 *pMessage) noexcept;

   // Implementation specific from this point on
public:
   static const APIVersion &SupportedAPIVersion() { return APIVersion::CurAPIVersion(); }

private:
   PhysicalDeviceDispatchableWrapper  m_physicalDeviceDispatchable;
};

} // namespace bvk
