/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#ifdef BUILD_VULKAN_ICD

#include <vulkan.h>

#include <Instance.h>
#include <Common.h>

#ifdef _MSC_VER
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) && (__GNUC__ >= 4)
 /* See https://gcc.gnu.org/wiki/Visibility */
#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_IMPORT __attribute__((visibility("default")))
#else
#define DLL_EXPORT
#define DLL_IMPORT
#endif

LOG_DEFAULT_CAT("vk_icd");

extern "C" {

DLL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(
   VkInstance instance,
   const char *pName
)
{
   log_trace("%s: %s", __FUNCTION__, pName);
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);
   return bvk::Instance::GetInstanceProcAddr(instanceObj, pName);
}

DLL_EXPORT VKAPI_ATTR PFN_vkVoidFunction vk_icdGetPhysicalDeviceProcAddr(
   VkInstance instance,
   const char* pName
)
{
   // We don't expose any physical device extensions yet
   log_trace("%s: %s", __FUNCTION__, pName);
   return nullptr;
}

DLL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(
   uint32_t *pSupportedVersion
)
{
   // From the LunarG LoaderAndLayers documentation:
   //
   // Loader Ver  ICD Ver  Result
   // ==========  =======  ===========================================================================
   // <= 4        <= 4     ICD must fail with VK_ERROR_INCOMPATIBLE_DRIVER for all vkCreateInstance
   //                      calls with apiVersion set to > Vulkan 1.0 because both the loader and ICD
   //                      support interface version <= 4. Otherwise, the ICD should behave as normal.
   // <= 4        >= 5     ICD must fail with VK_ERROR_INCOMPATIBLE_DRIVER for all vkCreateInstance
   //                      calls with apiVersion set to > Vulkan 1.0 because the loader is still at
   //                      interface version <= 4. Otherwise, the ICD should behave as normal.
   // >= 5        <= 4     Loader will fail with VK_ERROR_INCOMPATIBLE_DRIVER if it can't handle the
   //                      apiVersion. ICD may pass for all apiVersions, but since it's interface is
   //                      <= 4, it is best if it assumes it needs to do the work of rejecting anything
   //                      > Vulkan 1.0 and fail with VK_ERROR_INCOMPATIBLE_DRIVER.Otherwise, the ICD
   //                      should behave as normal.
   // >= 5        >= 5     Loader will fail with VK_ERROR_INCOMPATIBLE_DRIVER if it can't handle the
   //                      apiVersion, and ICDs should fail with VK_ERROR_INCOMPATIBLE_DRIVER only if
   //                      they can not support the specified apiVersion. Otherwise, the ICD should
   //                      behave as normal.

   uint32_t icdVer = 5;
   uint32_t loaderVer = *pSupportedVersion;

   log_trace("%s: loader requested version %u", __FUNCTION__, loaderVer);
   log_trace("%s: icd version %u", __FUNCTION__, icdVer);

   if (loaderVer < 5) // We only support loader interface versions >= 5
   {
      log_warn("Your Loader is too old. Version 5 loader interface or greater required.");
      return VK_ERROR_INCOMPATIBLE_DRIVER;
   }

   *pSupportedVersion = std::min(icdVer, loaderVer);

   log_trace("%s: supporting version %u", __FUNCTION__, *pSupportedVersion);

   return VK_SUCCESS;
}

}

#endif
