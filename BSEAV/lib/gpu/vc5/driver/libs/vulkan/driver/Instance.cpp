/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "Extensions.h"

#include <string.h>
#include <algorithm>
#include <mutex>

LOG_DEFAULT_CAT("bvk::Instance");

namespace bvk {

// No need to put these statics in the header.
static ProcAddressFinder s_procAddrFinder;

Instance::Instance(
   const VkAllocationCallbacks   *pCallbacks,
   const VkInstanceCreateInfo    *pCreateInfo):
      Allocating(pCallbacks)
{
   // Check api version
   if (pCreateInfo->pApplicationInfo != nullptr)
   {
      if (pCreateInfo->pApplicationInfo->apiVersion != 0)
      {
         APIVersion aVer(pCreateInfo->pApplicationInfo->apiVersion);
         if (!SupportedAPIVersion().CanSupport(aVer))
            throw VK_ERROR_INCOMPATIBLE_DRIVER;
      }
   }

   // Acquire a new reference on the singleton physical device, wrapped in a dispatchable
   // object. Note : we do this after the error checks that might throw.
   m_physicalDeviceDispatchable.AcquireDevice();
}

Instance::~Instance() noexcept
{
}

VkResult Instance::EnumeratePhysicalDevices(
   uint32_t          *pPhysicalDeviceCount,
   VkPhysicalDevice  *pPhysicalDevices) noexcept
{
   if (pPhysicalDevices != nullptr)
   {
      if (*pPhysicalDeviceCount > 0)
      {
         // Just fill in our one device
         *pPhysicalDevices = toHandle<VkPhysicalDevice>(&m_physicalDeviceDispatchable);

         *pPhysicalDeviceCount = 1;
      }
      else
      {
         // We will probably never see this case as we are shielded by the
         // loader, at least on all of the non-Android platforms.
         return VK_INCOMPLETE;
      }
   }
   else
      *pPhysicalDeviceCount = 1;

   return VK_SUCCESS;
}

// This is a static method as instance is allowed to be null.
// The instance is still given in case we want to specialize per instance in the future.
PFN_vkVoidFunction Instance::GetInstanceProcAddr(
   bvk::Instance  *instance,
   const char     *pName) noexcept
{
   // From section 3.1 : vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceLayerProperties or
   // vkCreateInstance are allowed names if instance is null. Those 3 are not allowed if
   // instance is non-null.
   bool onlyNull = false;

   if (!strcmp(pName, "vkCreateInstance") || !strcmp(pName, "vkEnumerateInstanceExtensionProperties") ||
       !strcmp(pName, "vkEnumerateInstanceLayerProperties"))
      onlyNull = true;

   if ((onlyNull && instance != nullptr) || (!onlyNull && instance == nullptr))
      return nullptr;

   return s_procAddrFinder.GetProcAddress(pName);
}

VkResult Instance::EnumerateInstanceExtensionProperties(
   const char              *pLayerName,
   uint32_t                *pPropertyCount,
   VkExtensionProperties   *pProperties) noexcept
{
   static VkExtensionProperties exts[] = {
#if !defined(VK_USE_PLATFORM_ANDROID_KHR)
      { VK_KHR_SURFACE_EXTENSION_NAME,       VK_KHR_SURFACE_SPEC_VERSION         },
#endif

#ifdef VK_USE_PLATFORM_WIN32_KHR
      { VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_SPEC_VERSION   },
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
      { VK_KHR_XCB_SURFACE_EXTENSION_NAME,   VK_KHR_XCB_SURFACE_SPEC_VERSION     },
#endif

#ifdef VK_USE_PLATFORM_DISPLAY_KHR
      { VK_KHR_DISPLAY_EXTENSION_NAME,       VK_KHR_DISPLAY_SPEC_VERSION         },
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
      { VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_SPEC_VERSION },
#endif

      { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_SPEC_VERSION },
   };

   return Extensions::EnumerateExtensionProperties(exts, countof(exts), pLayerName,
                                                   pPropertyCount, pProperties);
}

VkResult Instance::EnumerateInstanceLayerProperties(
   uint32_t          *pPropertyCount,
   VkLayerProperties *pProperties) noexcept
{
   VkResult result;

   result = VK_ERROR_INCOMPATIBLE_DRIVER;

   NOT_IMPLEMENTED_YET;
   return result;
}

#ifndef VK_USE_PLATFORM_ANDROID_KHR

template<typename T>
static VkResult CreatePlatformSurfaceKHR(
   Instance                    *instance,
   const T                     *pCreateInfo,
   const VkAllocationCallbacks *pAllocator,
   VkSurfaceKHR                *pSurface) noexcept
{
   log_trace("CreatePlatformSurfaceKHR");

   V3DPlatformSurface *surf = nullptr;
   try
   {
      surf = createObject<V3DPlatformSurface, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(
                                 pAllocator, instance->GetCallbacks(), pCreateInfo);
   }
   catch (const std::bad_alloc &)   { return VK_ERROR_OUT_OF_HOST_MEMORY;   }
   catch (const bad_device_alloc &) { return VK_ERROR_OUT_OF_DEVICE_MEMORY; }

   log_trace("   surface = %p", surf);

   *pSurface = toHandle<VkSurfaceKHR>(surf);

   return VK_SUCCESS;
}

void Instance::DestroySurfaceKHR(
   VkSurfaceKHR                   surface,
   const VkAllocationCallbacks   *pAllocator) noexcept
{
   V3DPlatformSurface *surf = fromHandle<V3DPlatformSurface>(surface);
   log_trace("DestroySurfaceKHR surface = %p", surf);
   destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(surf, pAllocator);
}

#endif

#ifdef VK_USE_PLATFORM_DISPLAY_KHR

VkResult Instance::CreateDisplayPlaneSurfaceKHR(
   const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
   const VkAllocationCallbacks         *pAllocator,
   VkSurfaceKHR                        *pSurface) noexcept
{
   return CreatePlatformSurfaceKHR<VkDisplaySurfaceCreateInfoKHR>(this, pCreateInfo, pAllocator, pSurface);
}
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR

VkResult Instance::CreateXlibSurfaceKHR(
   const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkSurfaceKHR                     *pSurface) noexcept
{
   VkResult result;

   result = VK_ERROR_INCOMPATIBLE_DRIVER;

   NOT_IMPLEMENTED_YET;
   return result;
}

#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

VkResult Instance::CreateXcbSurfaceKHR(
   const VkXcbSurfaceCreateInfoKHR  *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkSurfaceKHR                     *pSurface) noexcept
{
   return CreatePlatformSurfaceKHR<VkXcbSurfaceCreateInfoKHR>(this, pCreateInfo, pAllocator, pSurface);
}

#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VkResult Instance::CreateWaylandSurfaceKHR(
   const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
   const VkAllocationCallbacks         *pAllocator,
   VkSurfaceKHR                        *pSurface) noexcept
{
   return CreatePlatformSurfaceKHR<VkWaylandSurfaceCreateInfoKHR>(this, pCreateInfo, pAllocator, pSurface);
}

#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_ANDROID_KHR

VkResult Instance::CreateAndroidSurfaceKHR(
   const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
   const VkAllocationCallbacks         *pAllocator,
   VkSurfaceKHR                        *pSurface) noexcept
{
   VkResult result;

   result = VK_ERROR_INCOMPATIBLE_DRIVER;

   NOT_IMPLEMENTED_YET;
   return result;
}

void Instance::DestroySurfaceKHR(
   VkSurfaceKHR                   surface,
   const VkAllocationCallbacks   *pAllocator) noexcept
{
   NOT_IMPLEMENTED_YET;
}

#endif // VK_USE_PLATFORM_ANDROID_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkResult Instance::CreateWin32SurfaceKHR(
   const VkWin32SurfaceCreateInfoKHR   *pCreateInfo,
   const VkAllocationCallbacks         *pAllocator,
   VkSurfaceKHR                        *pSurface) noexcept
{
   return CreatePlatformSurfaceKHR<VkWin32SurfaceCreateInfoKHR>(this, pCreateInfo, pAllocator, pSurface);
}

#endif // VK_USE_PLATFORM_WIN32_KHR

void Instance::DebugReportMessageEXT(
   VkDebugReportFlagsEXT       flags,
   VkDebugReportObjectTypeEXT  objectType,
   uint64_t                    object,
   size_t                      location,
   int32_t                     messageCode,
   const char                 *pLayerPrefix,
   const char                 *pMessage) noexcept
{
   NOT_IMPLEMENTED_YET;
}

} // namespace bvk
