/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#ifdef VK_USE_PLATFORM_ANDROID_KHR

#include <hardware/gralloc.h>

#include "V3DPlatformBase.h"
#include "V3DPlatformAndroid.h"

#include <log/log.h>

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
   // The format is not important to the gralloc usage on our platform.
   (void)format;
   (void)device;

   // The current (N) swapchain code does zero this before calling, but
   // zero it here in case their code changes in the future.
   *grallocUsage = 0;

   // NOTE: The following flags are not advertised as valid for WSI surfaces by
   //       the Android swapchain implementation provided by Google.
   //       VK_USAGE_DEPTH_STENCIL_BIT        - swapchain images must be color
   //       VK_USAGE_TRANSIENT_ATTACHMENT_BIT - swapchain images cannot be
   //          transient memory as they are backed by gralloc buffers

   // If we ever implement UIF gralloc buffers as an interop format with
   // the display composition then add GRALLOC_USAGE_HW_TEXTURE for TRANSFER_SRC
   // to allow for optimized blits from swapchain images (i.e. screen snapshot).
   //
   // NOTE: we have no sensible flag available to indicate the TFU might read
   //       this, so just specify HW_RENDER to make sure the memory is visible
   //       to the v3d hardware in general. Although a swapchain image usage
   //       without either TRANSFER_DST or COLOR_ATTACHMENT as well would be
   //       pretty useless.
   if (imageUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
      *grallocUsage |= GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_RENDER;

   if (imageUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
      *grallocUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_RENDER;

   if (imageUsage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
      *grallocUsage |= GRALLOC_USAGE_HW_RENDER;

   // These require UIF compatible gralloc buffers, which are currently not
   // supported in our Android implementation. We would prefer Google allowed
   // vendors the choice to decide if certain usage was valid for swapchain
   // images, but currently their layer hardcodes it.
   //
   // NOTE: we are not flagging any error here, we are relying on the
   //       surface window's creation of the buffers failing when our
   //       gralloc implementation sees the GRALLOC_USAGE_HW_TEXTURE flag.
   if (imageUsage & VK_IMAGE_USAGE_SAMPLED_BIT)
      *grallocUsage |= GRALLOC_USAGE_HW_TEXTURE;

   if (imageUsage & VK_IMAGE_USAGE_STORAGE_BIT)
      *grallocUsage |= GRALLOC_USAGE_HW_TEXTURE;

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
