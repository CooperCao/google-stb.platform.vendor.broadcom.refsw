/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#ifdef VK_USE_PLATFORM_ANDROID_KHR

#include <hardware/gralloc.h>

#include "V3DPlatformBase.h"
#include "V3DPlatformAndroid.h"

#include <cutils/log.h>

#include "v3d_scheduler.h"
#include "Fence.h"
#include "Semaphore.h"

namespace bvk
{

V3DPlatformAndroid::V3DPlatformAndroid(): V3DPlatformBase()
{
   m_bvkPlatform.Register();
   CommonInitialize();
}

V3DPlatformAndroid::~V3DPlatformAndroid()
{
   CommonTerminate();
}

#ifdef __cplusplus
extern "C" {
#endif

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainGrallocUsageANDROID(
    VkDevice            device,
    VkFormat            format,
    VkImageUsageFlags   imageUsage,
    int*                grallocUsage
)
{
   // TODO: Use image format to select correct gralloc usage
   // Format returned by GetPhysicalDeviceSurfaceFormatsKHR in the loader
   // VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R5G6B5_UNORM_PACK16

   if (imageUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
      *grallocUsage |= GRALLOC_USAGE_SW_READ_MASK | GRALLOC_USAGE_HW_RENDER;

   if (imageUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
      *grallocUsage |= GRALLOC_USAGE_SW_WRITE_MASK | GRALLOC_USAGE_HW_RENDER;

   if (imageUsage & VK_IMAGE_USAGE_SAMPLED_BIT)
      *grallocUsage |= GRALLOC_USAGE_HW_TEXTURE;

   if (imageUsage & VK_IMAGE_USAGE_STORAGE_BIT)
      *grallocUsage |= GRALLOC_USAGE_SW_WRITE_MASK | GRALLOC_USAGE_HW_TEXTURE;

   if (imageUsage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
      *grallocUsage |= GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_SW_WRITE_MASK;

   if (imageUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
      *grallocUsage |= GRALLOC_USAGE_HW_RENDER | GRALLOC_USAGE_SW_WRITE_MASK;

   if (imageUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)
      *grallocUsage |= GRALLOC_USAGE_SW_WRITE_MASK;

   if (imageUsage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
      *grallocUsage |= GRALLOC_USAGE_HW_TEXTURE;

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireImageANDROID(
    VkDevice            device,
    VkImage             image,
    int                 nativeFenceFd,
    VkSemaphore         semaphore,
    VkFence             fence
)
{
   JobID jobid;

   if ((semaphore != VK_NULL_HANDLE) || (fence != VK_NULL_HANDLE))
   {
      jobid = v3d_scheduler_submit_wait_fence(nativeFenceFd);
   }

   if (fence != VK_NULL_HANDLE)
   {
      auto fenceObj = bvk::fromHandle<bvk::Fence>(fence);
      fenceObj->ScheduleSignal(jobid);
   }

   if (semaphore != VK_NULL_HANDLE)
   {
      auto semaphoreObj = bvk::fromHandle<bvk::Semaphore>(semaphore);
      semaphoreObj->ScheduleSignal(jobid);
   }

   return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSignalReleaseImageANDROID(
    VkQueue             queue,
    uint32_t            waitSemaphoreCount,
    const VkSemaphore*  pWaitSemaphores,
    VkImage             image,
    int*                pNativeFenceFd
)
{
   SchedDependencies deps;

   for (unsigned int index = 0; index < waitSemaphoreCount; index++)
   {
      auto semaphoreObj = bvk::fromHandle<bvk::Semaphore>(pWaitSemaphores[index]);
      deps += semaphoreObj->ScheduleWait(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
   }

   *pNativeFenceFd = v3d_scheduler_create_fence(&deps, V3D_SCHED_DEPS_COMPLETED, false);
   return VK_SUCCESS;
}

#ifdef __cplusplus
}
#endif

}  // namespace bvk

#endif // VK_USE_PLATFORM_ANDROID_KHR
