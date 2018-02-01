/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "V3DPlatformBase.h"
#include "Common.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "DeviceMemory.h"
#include "Image.h"
#include "Options.h"
#include "Semaphore.h"

#include <algorithm>
#include <sstream>
#include <cstring>

#include "libs/platform/v3d_scheduler.h"
#include "libs/platform/v3d_parallel.h"
#include "libs/util/profile/profile.h"
#include "libs/compute/compute.h"

namespace bvk
{

void V3DPlatformBase::CommonInitialize()
{
   // Read any environment options first
   Options::Initialise();

#if !V3D_VER_AT_LEAST(4,1,34,0)
   throw "You need at least v4.1 VC5 hardware to use Vulkan";
#endif

   if (vcos_init() != VCOS_SUCCESS)
      throw "Failed to initialise V3D platform (vcos_init)";

   profile_init();

   // Bring up the platform
   if (!v3d_platform_init())
      throw "Failed to initialise V3D platform (platform init)";

   if (!v3d_parallel_init(Options::maxWorkerThreads))
      throw "Failed to initialise V3D platform (parallel init)";

   // Interrogate the hardware
   m_hubIdent = *v3d_scheduler_get_hub_identity();
   m_ident = *v3d_scheduler_get_identity();

#if !V3D_USE_CSD
   // Bring up compute runtime.
   compute_init();
#endif

   // Default to using all cores, or number specified by environment.
   // Need to do this after the scheduler session has been brought up.
   uint32_t num_cores = m_hubIdent.num_cores;

   if (Options::renderSubjobs == 0)
      Options::renderSubjobs = num_cores;

   if (Options::binSubjobs == 0)
      Options::binSubjobs = num_cores;

   // Further clamp to max sub-jobs of type
   Options::renderSubjobs = std::min(Options::renderSubjobs, (uint32_t)V3D_MAX_RENDER_SUBJOBS);
   Options::binSubjobs    = std::min(Options::binSubjobs, (uint32_t)V3D_MAX_BIN_SUBJOBS);
}

void V3DPlatformBase::CommonTerminate()
{
   v3d_scheduler_wait_all();
#if !V3D_USE_CSD
   compute_term();
#endif

   // We are done
   v3d_parallel_term();
   v3d_platform_shutdown();
   profile_shutdown();
}

void V3DPlatformBase::MaybeDumpPresentedImage(const Image &img)
{
   if (Options::dumpPresentedImages)
   {
      static uint32_t frameNum = 0;

      std::ostringstream name;
      name << "frame" << frameNum++;

      img.DumpImage(0, 0, name.str());
   }
}

uint32_t V3DPlatformBase::NumRequiredSwapchainBuffers(uint32_t minBuffers)
{
   // Default implementation - return the minimum. Override if required.
   return minBuffers;
}

VkResult V3DPlatformBase::GetSurfaceCapabilities(
   VkSurfaceKHR                surface,
   VkSurfaceCapabilitiesKHR   *pSurfaceCapabilities)
{
   // Defaults for X11/Win32 platforms, hardware platforms will override
   pSurfaceCapabilities->minImageCount           = 1;
   pSurfaceCapabilities->maxImageCount           = 0;
   pSurfaceCapabilities->minImageExtent          = { 1, 1 };
   pSurfaceCapabilities->maxImageExtent          = { 4096, 4096 }; // 4096x2048??
   pSurfaceCapabilities->maxImageArrayLayers     = 1;
   pSurfaceCapabilities->supportedTransforms     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   pSurfaceCapabilities->currentTransform        = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   pSurfaceCapabilities->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
   pSurfaceCapabilities->supportedUsageFlags     = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

   GetSurfaceExtent(&pSurfaceCapabilities->currentExtent, surface);

   return VK_SUCCESS;
}

VkResult V3DPlatformBase::GetSurfaceFormats(
   VkSurfaceKHR          surface,
   uint32_t             *pSurfaceFormatCount,
   VkSurfaceFormatKHR   *pSurfaceFormats)
{
   // Default for X11/Win32 platforms, just support RGBA8888, hardware
   // platforms will override
   const VkSurfaceFormatKHR   formats[] =
   {
      { VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR }
   };

   return sizedListQuery<VkSurfaceFormatKHR>(pSurfaceFormats, pSurfaceFormatCount,
                                             formats, countof(formats));
}

VkResult V3DPlatformBase::GetSurfacePresentModes(
   VkSurfaceKHR       surface,
   uint32_t          *pPresentModeCount,
   VkPresentModeKHR  *pPresentModes)
{
   // Default for X11/Win32 platforms, hardware based platforms will override
   const VkPresentModeKHR  modes[] =
   {
      VK_PRESENT_MODE_IMMEDIATE_KHR,
      VK_PRESENT_MODE_FIFO_KHR
   };

   return sizedListQuery<VkPresentModeKHR>(pPresentModes, pPresentModeCount,
                                           modes, countof(modes));
}

// Dummy implementations of the display extension entrypoints for WSI based
// platforms. Overridden by the hardware platform talking directly to Nexus.
VkResult V3DPlatformBase::GetDisplayProperties(
   uint32_t                *pPropertyCount,
   VkDisplayPropertiesKHR  *pProperties)
{
   *pPropertyCount = 0;
   return VK_SUCCESS;
}

VkResult V3DPlatformBase::GetDisplayPlaneProperties(
   uint32_t                      *pPropertyCount,
   VkDisplayPlanePropertiesKHR   *pProperties)
{
   *pPropertyCount = 0;
   return VK_SUCCESS;
}

VkResult V3DPlatformBase::GetDisplayPlaneSupportedDisplaysKHR(
   uint32_t        planeIndex,
   uint32_t       *pDisplayCount,
   VkDisplayKHR   *pDisplays)
{
   *pDisplayCount = 0;
   return VK_SUCCESS;
}

VkResult V3DPlatformBase::GetDisplayModeProperties(
   VkDisplayKHR                display,
   uint32_t                   *pPropertyCount,
   VkDisplayModePropertiesKHR *pProperties)
{
   *pPropertyCount = 0;
   return VK_SUCCESS;
}

VkResult V3DPlatformBase::GetDisplayPlaneCapabilities(
   VkDisplayModeKHR               mode,
   uint32_t                       planeIndex,
   VkDisplayPlaneCapabilitiesKHR *pCapabilities)
{
   std::memset(pCapabilities, 0, sizeof(*pCapabilities));
   return VK_SUCCESS;
}

bool V3DPlatformBase::HasSurfacePresentationSupport(VkSurfaceKHR surface)
{
   return true;
}

void V3DPlatformBase::ConvertPresentSemaphoresToDeps(
   const VkPresentInfoKHR  *presentInfo,
   SchedDependencies       *deps)
{
   for (uint32_t s = 0; s < presentInfo->waitSemaphoreCount; s++)
   {
      Semaphore *sem = fromHandle<Semaphore>(presentInfo->pWaitSemaphores[s]);
      *deps += sem->ScheduleWait(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
   }
}

// These methods are only used in direct-display mode
#if defined(VK_USE_PLATFORM_DISPLAY_KHR)

static const uint32_t DirectDispW  = 640;
static const uint32_t DirectDispH  = 360;
static const uint32_t DisplayIdent = 1;

V3DPlatformWithFakeDirectDisplay::V3DPlatformWithFakeDirectDisplay() :
   V3DPlatformBase()
{
   m_numPlanes = 1;

   m_displayProperties.display               = (VkDisplayKHR)(uintptr_t)DisplayIdent;
   m_displayProperties.displayName           = "Fake Direct Display";
   m_displayProperties.physicalResolution    = { DirectDispW, DirectDispH };
   m_displayProperties.physicalDimensions    = { 480, 270 };
   m_displayProperties.supportedTransforms   = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
   m_displayProperties.planeReorderPossible  = false;
   m_displayProperties.persistentContent     = false;

   m_displayModeProperties.parameters.visibleRegion = VkExtent2D{ DirectDispW, DirectDispH };
   m_displayModeProperties.parameters.refreshRate   = 60000;
   m_displayModeProperties.displayMode              = (VkDisplayModeKHR)1;
}

VkResult V3DPlatformWithFakeDirectDisplay::GetDisplayProperties(
   uint32_t                *pPropertyCount,
   VkDisplayPropertiesKHR  *pProperties)
{
   return sizedListQuery<VkDisplayPropertiesKHR>(pProperties, pPropertyCount, &m_displayProperties, 1);
}

VkResult V3DPlatformWithFakeDirectDisplay::GetDisplayModeProperties(
   VkDisplayKHR                display,
   uint32_t                   *pPropertyCount,
   VkDisplayModePropertiesKHR *pProperties)
{
   assert(display == m_displayProperties.display);

   return sizedListQuery<VkDisplayModePropertiesKHR>(pProperties, pPropertyCount,
                                                     &m_displayModeProperties, 1);
}

VkResult V3DPlatformWithFakeDirectDisplay::GetDisplayPlaneSupportedDisplaysKHR(
   uint32_t        planeIndex,
   uint32_t       *pDisplayCount,
   VkDisplayKHR   *pDisplays)
{
   assert(planeIndex < m_numPlanes);

   VkDisplayKHR planeDisplays = m_displayProperties.display;

   return sizedListQuery<VkDisplayKHR>(pDisplays, pDisplayCount, &planeDisplays, 1);
}

VkResult V3DPlatformWithFakeDirectDisplay::GetDisplayPlaneProperties(
   uint32_t                    *pPropertyCount,
   VkDisplayPlanePropertiesKHR *pProperties)
{
   if (pProperties == nullptr)
   {
      *pPropertyCount = 1;
      return VK_SUCCESS;
   }

   if (*pPropertyCount == 0)
      return VK_INCOMPLETE;

   *pPropertyCount = 1;
   pProperties->currentDisplay = m_displayProperties.display;
   pProperties->currentStackIndex = 0;
   return VK_SUCCESS;
}

VkResult V3DPlatformWithFakeDirectDisplay::GetDisplayPlaneCapabilities(
   VkDisplayModeKHR mode, uint32_t planeIndex,
   VkDisplayPlaneCapabilitiesKHR *pCapabilities)
{
   assert(planeIndex == 0);

   pCapabilities->supportedAlpha = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;
   pCapabilities->minSrcPosition = VkOffset2D{ 0, 0 };
   pCapabilities->maxSrcPosition = VkOffset2D{ 0, 0 };
   pCapabilities->minSrcExtent   = VkExtent2D{ DirectDispW, DirectDispH };
   pCapabilities->maxSrcExtent   = VkExtent2D{ DirectDispW, DirectDispH };
   pCapabilities->minDstPosition = VkOffset2D{ 0, 0 };
   pCapabilities->maxDstPosition = VkOffset2D{ 0, 0 };
   pCapabilities->minDstExtent   = VkExtent2D{ DirectDispW, DirectDispH };
   pCapabilities->maxDstExtent   = VkExtent2D{ DirectDispW, DirectDispH };
   return VK_SUCCESS;
}

#endif // VK_USE_PLATFORM_DISPLAY_KHR

} // namespace bvk
