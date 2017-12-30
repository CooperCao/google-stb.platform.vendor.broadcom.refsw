/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "ProcAddressFinder.h"
#include <vulkan.h>
#include <string>
#include <unordered_map>
#include <utility>

#if VK_USE_PLATFORM_ANDROID_KHR
#ifdef __cplusplus
extern "C" {
#endif

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainGrallocUsageANDROID(
    VkDevice            device,
    VkFormat            format,
    VkImageUsageFlags   imageUsage,
    int*                grallocUsage
);
VKAPI_ATTR VkResult VKAPI_CALL vkAcquireImageANDROID(
    VkDevice            device,
    VkImage             image,
    int                 nativeFenceFd,
    VkSemaphore         semaphore,
    VkFence             fence
);
VKAPI_ATTR VkResult VKAPI_CALL vkQueueSignalReleaseImageANDROID(
    VkQueue             queue,
    uint32_t            waitSemaphoreCount,
    const VkSemaphore*  pWaitSemaphores,
    VkImage             image,
    int*                pNativeFenceFd
);
#ifdef __cplusplus
}
#endif
#endif

namespace bvk {

#if defined(VK_USE_DLADDR)
// Linux, direct link of libbcmvulkan.so (no loader) where vk API
// entrypoints are publically visible. The use of dlsym on ourselves ensures
// we return our version of the entrypoint and not from some other LD_PRELOAD
// library.
#include <dlfcn.h>

// A unique function that we can search for
static void ___uniqueFunctionNameInThisBcmVulkanIcdLibrary___() {}

ProcAddressFinder::ProcAddressFinder() {}

PFN_vkVoidFunction ProcAddressFinder::GetProcAddress(const char *name) const
{
   PFN_vkVoidFunction result;
   Dl_info info;
   dladdr((const void*)___uniqueFunctionNameInThisBcmVulkanIcdLibrary___, &info);

   // Open our own .so
   void *moduleHandle = dlopen(info.dli_fname, RTLD_LAZY | RTLD_NOLOAD);

   // If we happen to fail for any reason, try again with NULL as the handle.
   // This will not ignore LD_PRELOAD, but it's better than failing completely.
   if (moduleHandle == nullptr)
      moduleHandle = dlopen(nullptr, RTLD_LAZY | RTLD_NOLOAD);

   result = (PFN_vkVoidFunction)dlsym(moduleHandle, name);
   dlclose(moduleHandle);
   return result;
}
#endif

#if !defined(VK_USE_DLADDR)
// Win32 and Linux, using the LunarG loader to dynamically install
// libbcmvulkan_icd shared library where only the vk_icd entrypoints are
// publicly visible. In this case the vk API entrypoints are local symbols
// and therefore must be referenced directly as dlsym/GetProcAddress will not
// find them.
ProcAddressFinder::ProcAddressFinder() {}

#define ProcAddressExport(f) ProcAddressItem{ #f, (PFN_vkVoidFunction)f }

PFN_vkVoidFunction ProcAddressFinder::GetProcAddress(const char *name) const
{
   using ProcAddressItem = std::pair<std::string, PFN_vkVoidFunction>;
   static std::unordered_map<std::string, PFN_vkVoidFunction> procAddrMap = {
#include "InstanceProcTable.inc"
#if VK_USE_PLATFORM_ANDROID_KHR
      ProcAddressExport(vkGetSwapchainGrallocUsageANDROID),
      ProcAddressExport(vkAcquireImageANDROID),
      ProcAddressExport(vkQueueSignalReleaseImageANDROID),
#endif
#if VK_USE_PLATFORM_XLIB_KHR
      ProcAddressExport(vkGetPhysicalDeviceXlibPresentationSupportKHR),
#endif
#if VK_USE_PLATFORM_XCB_KHR
      ProcAddressExport(vkGetPhysicalDeviceXcbPresentationSupportKHR),
      ProcAddressExport(vkCreateXcbSurfaceKHR),
#endif
#if VK_USE_PLATFORM_WAYLAND_KHR
      ProcAddressExport(vkGetPhysicalDeviceWaylandPresentationSupportKHR),
      ProcAddressExport(vkCreateWaylandSurfaceKHR),
#endif
#if VK_USE_PLATFORM_WIN32_KHR
      ProcAddressExport(vkGetPhysicalDeviceWin32PresentationSupportKHR),
      ProcAddressExport(vkCreateWin32SurfaceKHR),
#endif
#if VK_USE_PLATFORM_DISPLAY_KHR
      ProcAddressExport(vkCreateDisplayPlaneSurfaceKHR),
#endif
   };

   auto itr = procAddrMap.find(name);
   return (itr != procAddrMap.end()) ? itr->second : nullptr;
}
#endif

} // namespace bvk
