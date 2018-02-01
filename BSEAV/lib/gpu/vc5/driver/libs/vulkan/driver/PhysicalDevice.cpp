/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "Extensions.h"
#include "DriverLimits.h"
#include "PrimitiveTypes.h"

#include "libs/platform/gmem.h"
#include "libs/platform/v3d_scheduler.h"
#include "libs/util/demand.h"
#include "libs/core/v3d/v3d_ident.h"
#include "libs/core/v3d/v3d_gen.h"

#if VK_USE_PLATFORM_ANDROID_KHR
#include "vulkan/vk_android_native_buffer.h"
#endif

#include <cassert>

LOG_DEFAULT_CAT("bvk::PhysicalDevice");

namespace bvk {

// Supported sample counts for all kinds and uses of images. If, in the future,
// sample counts depend on format or use then something more complex will be needed
static const VkSampleCountFlags v3dSampleCounts = VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_4_BIT;

// There is only one, shared, physicalDevice which is reference counted.
static PhysicalDevice   *s_physicalDevice;
static std::mutex        s_physicalDeviceMutex;
static uint32_t          s_physicalDeviceRefCount;

PhysicalDevice *PhysicalDevice::Acquire()
{
   std::lock_guard<std::mutex> lock(s_physicalDeviceMutex);
   if (s_physicalDevice == NULL)
   {
      assert(s_physicalDeviceRefCount == 0);
      // Note: we don't use allocator callbacks since this is outside instance scope
      s_physicalDevice = ::new PhysicalDevice();
   }
   s_physicalDeviceRefCount++;

   return s_physicalDevice;
}

void PhysicalDevice::Release()
{
   std::lock_guard<std::mutex> lock(s_physicalDeviceMutex);
   assert(s_physicalDeviceRefCount > 0);
   s_physicalDeviceRefCount--;
   if (s_physicalDeviceRefCount == 0)
   {
      ::delete s_physicalDevice;
      s_physicalDevice = nullptr;
   }
}

// Note : PhysicalDevice does not take user-provided allocators.
// We make a single, shared physicalDevice so we don't have appropriate
// allocators to use.
PhysicalDevice::PhysicalDevice()
{
   // Fill our big tables
   InitFeatures();   // Must init features before limits
   InitLimits();
   InitMemoryProps();
   InitFormatProperties();
   InitCompilerTables();

   // Sparse memory not supported
   m_sparseProps = {};

   m_queueFamilyProps.queueCount                  = 1;
   m_queueFamilyProps.timestampValidBits          = 0; // Unsupported initially
   m_queueFamilyProps.minImageTransferGranularity = { 1, 1, 1 };
   m_queueFamilyProps.queueFlags                  = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT |
                                                    VK_QUEUE_TRANSFER_BIT;
}

PhysicalDevice::~PhysicalDevice() noexcept
{
}

void PhysicalDevice::GetPhysicalDeviceFeatures(
   VkPhysicalDeviceFeatures   *pFeatures) noexcept
{
   *pFeatures = m_features;
}

void PhysicalDevice::GetPhysicalDeviceFeatures2KHR(
   VkPhysicalDeviceFeatures2KHR  *pFeatures) noexcept
{
   GetPhysicalDeviceFeatures(&pFeatures->features);
}

void PhysicalDevice::GetPhysicalDeviceFormatProperties(
   VkFormat              format,
   VkFormatProperties   *pFormatProperties) noexcept
{
   *pFormatProperties = FormatProperty(format);
}

void PhysicalDevice::GetPhysicalDeviceFormatProperties2KHR(
   VkFormat                 format,
   VkFormatProperties2KHR  *pFormatProperties) noexcept
{
   GetPhysicalDeviceFormatProperties(format, &pFormatProperties->formatProperties);
}

VkResult PhysicalDevice::GetPhysicalDeviceImageFormatProperties(
   VkFormat                 format,
   VkImageType              type,
   VkImageTiling            tiling,
   VkImageUsageFlags        usage,
   VkImageCreateFlags       flags,
   VkImageFormatProperties *pImageFormatProperties) noexcept
{
   // NOTE: If you decide to change any of the properties or format limitations
   //       in here, be sure that any corresponding asserts are removed/added to
   //       the Image class constructor as well.

   // If the combination of parameters to vkGetPhysicalDeviceImageFormatProperties
   // is not supported by the implementation for use in vkCreateImage, then all
   // members of VkImageFormatProperties will be filled with zero.
   memset(pImageFormatProperties, 0, sizeof(VkImageFormatProperties));

   const bool isLinear = (tiling == VK_IMAGE_TILING_LINEAR);

   // As we cannot texture from any linear format, disallow the creation
   // of linear images with the cube compatible bit.
   if (isLinear && (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   // Also disallow the creation of linear images for input attachments as they
   // are textured from.
   if (isLinear && (usage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT))
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   VkFormatFeatureFlags ff = isLinear ? FormatProperty(format).linearTilingFeatures :
                                        FormatProperty(format).optimalTilingFeatures;

   // Early out if the usage is not supported for the format
   if (ff == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_SAMPLED_BIT) && (ff & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_STORAGE_BIT) && (ff & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) &&
      (ff & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) &&
      (ff & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) && (ff & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   if ((usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) && (ff & VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR) == 0)
      return VK_ERROR_FORMAT_NOT_SUPPORTED;

   switch (type)
   {
   case VK_IMAGE_TYPE_1D :
      pImageFormatProperties->maxExtent = { m_limits.maxImageDimension1D, 1, 1 };
      pImageFormatProperties->maxArrayLayers = m_limits.maxImageArrayLayers;
      break;
   case VK_IMAGE_TYPE_2D :
      pImageFormatProperties->maxExtent = { m_limits.maxImageDimension2D,
                                            m_limits.maxImageDimension2D, 1 };
      pImageFormatProperties->maxArrayLayers = m_limits.maxImageArrayLayers;
      break;
   case VK_IMAGE_TYPE_3D :
      pImageFormatProperties->maxExtent = { m_limits.maxImageDimension3D,
                                            m_limits.maxImageDimension3D,
                                            m_limits.maxImageDimension3D };
      pImageFormatProperties->maxArrayLayers = 1;
      break;
   default:
      unreachable();
   }

   // Spec v1.0.26: 11.3 Images, linear images are only required to be
   // supported with 1 miplevel. As they cannot be used as textures with
   // our devices there is no point supporting more.
   pImageFormatProperties->maxMipLevels =
         isLinear ? 1 : gfx_log2(pImageFormatProperties->maxExtent.width) + 1;

   pImageFormatProperties->maxResourceSize = 0xFFFFFFFF;

   // sampleCounts will be set to VK_SAMPLE_COUNT_1_BIT if at least one of the following conditions is true:
   //  * tiling is VK_IMAGE_TILING_LINEAR
   //  * type is not VK_IMAGE_TYPE_2D
   //  * flags contains VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
   //  * neither VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT or VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
   //    is set in VkFormatProperties::optimalTilingFeatures for the format. (We check ff because LINEAR is
   //    already disallowed).
   //
   // Otherwise, the bits set in sampleCounts will be the intersection of (a
   // superset of) the sample counts supported for the specified values of
   // usage and format.
   VkFormatFeatureFlags attachmentTypes = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                                          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

   if (tiling == VK_IMAGE_TILING_LINEAR ||
       type != VK_IMAGE_TYPE_2D ||
       (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ||
       (ff & attachmentTypes) == 0)
   {
      pImageFormatProperties->sampleCounts = VK_SAMPLE_COUNT_1_BIT;
   }
   else
      pImageFormatProperties->sampleCounts = v3dSampleCounts;

   return VK_SUCCESS;
}

VkResult PhysicalDevice::GetPhysicalDeviceImageFormatProperties2KHR(
   const VkPhysicalDeviceImageFormatInfo2KHR *pImageFormatInfo,
   VkImageFormatProperties2KHR               *pImageFormatProperties) noexcept
{
   return GetPhysicalDeviceImageFormatProperties(pImageFormatInfo->format, pImageFormatInfo->type,
                                                 pImageFormatInfo->tiling, pImageFormatInfo->usage,
                                                 pImageFormatInfo->flags,
                                                 &pImageFormatProperties->imageFormatProperties);
}

static uint32_t BuildDeviceID()
{
   const V3D_HUB_IDENT_T *hubID = v3d_scheduler_get_hub_identity();
   const V3D_IDENT_T     *v3dID = v3d_scheduler_get_identity();

   // Vulkan won't work without an MMU
   demand(hubID->has_mmu);

   // We will encode these bits of the ident registers into the Vulkan DeviceID.
   // The DeviceID is only 32-bits wide, so I've reduced the size of a few
   // fields. I don't think these will cause issues in reality, but gfx_bits
   // will assert if we ever have a case that breaks.
   union DeviceID
   {
      uint32_t m_uint32;
      struct
      {
         uint32_t m_techVersion       : 4;
         uint32_t m_revision          : 4;
         uint32_t m_subRev            : 6;
         uint32_t m_compatRev         : 6;
         uint32_t m_numCoresMinusOne  : 4; // Gives maximum of 16 cores (ident reg allows 16)
         uint32_t m_numSlicesMinusOne : 2; // Gives maximum of 4 slices per core (ident reg allows 15)
         uint32_t m_numQPUSMinusOne   : 2; // Gives maximum of 4 QPUs per slice (ident reg allows 15)
         uint32_t m_numTMUSMinusOne   : 2; // Gives maximum of 4 tmus per core (ident reg allows 15)
         uint32_t m_hasL3C            : 1;
         uint32_t m_hasTFU            : 1;
      } m_bits;
   };

   static_assert(sizeof(DeviceID) <= sizeof(uint32_t), "DeviceID structure is too large");

   DeviceID devID = {};
   devID.m_bits.m_techVersion        = gfx_bits(hubID->tech_version,           4);
   devID.m_bits.m_revision           = gfx_bits(hubID->revision,               4);
   devID.m_bits.m_subRev             = gfx_bits(hubID->sub_rev,                6);
   devID.m_bits.m_compatRev          = gfx_bits(hubID->compat_rev,             6);
   devID.m_bits.m_numCoresMinusOne   = gfx_bits(hubID->num_cores - 1,          4);
   devID.m_bits.m_numSlicesMinusOne  = gfx_bits(v3dID->num_slices - 1,         2);
   devID.m_bits.m_numQPUSMinusOne    = gfx_bits(v3dID->num_qpus_per_slice - 1, 2);
   devID.m_bits.m_numTMUSMinusOne    = gfx_bits(v3dID->num_tmus - 1,           2);
#if V3D_HAS_L3C
   devID.m_bits.m_hasL3C             = hubID->has_l3c ? 1 : 0;
#endif
   devID.m_bits.m_hasTFU             = hubID->has_tfu ? 1 : 0;

   return devID.m_uint32;
}

static void GetUUID(uint8_t uuid[VK_UUID_SIZE])
{
   // UUID represents a universally unique signature that identifies the
   // hardware AND driver combination.

   // TODO : Find a sensible solution for UUID generation. For now, we don't have a
   // pipeline cache that does anything, so a single UUID will work for everything.
   const char *buildUUID = "10c62c25-05c0-4b26-ab83-b43d69070a91";

   log_trace("UUID          = %s", buildUUID);

   char        hexPair[3] = {};
   const char *ptr = buildUUID;
   uint32_t    d = 0;

   // Read each pair of hex digits at a time
   while (*ptr != '\0')
   {
      if (*ptr == '-')  // Skip the '-'' characters
         ptr++;

      hexPair[0] = *ptr++;
      hexPair[1] = *ptr++;

      uuid[d++] = static_cast<uint8_t>(strtoul(hexPair, nullptr, 16));
   }
}

void PhysicalDevice::GetPhysicalDeviceProperties(
   VkPhysicalDeviceProperties *pProperties) noexcept
{
   pProperties->apiVersion       = Instance::SupportedAPIVersion();
   pProperties->driverVersion    = m_platform.GetDriverVersion();
   pProperties->vendorID         = 0x0A5C;   // This is our PCI vendor ID
   pProperties->deviceID         = BuildDeviceID();
   pProperties->deviceType       = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
   pProperties->limits           = m_limits;
   pProperties->sparseProperties = m_sparseProps;

   v3d_sprint_device_name(pProperties->deviceName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE, 0,
      v3d_scheduler_get_identity());

   pProperties->deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE - 1] = '\0';

   // UUID represents a universally unique signature that identifies the
   // hardware AND driver combination.
   GetUUID(pProperties->pipelineCacheUUID);

   log_trace("ApiVersion    = %u",     pProperties->apiVersion);
   log_trace("DriverVersion = %u",     pProperties->driverVersion);
   log_trace("VendorID      = 0x%08X", pProperties->vendorID);
   log_trace("DeviceID      = 0x%08X", pProperties->deviceID);
   log_trace("DeviceName    = %s",     pProperties->deviceName);
}

void PhysicalDevice::GetPhysicalDeviceProperties2KHR(
   VkPhysicalDeviceProperties2KHR   *pProperties) noexcept
{
   GetPhysicalDeviceProperties(&pProperties->properties);
}

void PhysicalDevice::GetPhysicalDeviceQueueFamilyProperties(
   uint32_t                *pQueueFamilyPropertyCount,
   VkQueueFamilyProperties *pQueueFamilyProperties) noexcept
{
   // We only support one queue (as this matches the scheduler queue).
   if (pQueueFamilyProperties != nullptr)
   {
      if (*pQueueFamilyPropertyCount > 0)
      {
         *pQueueFamilyProperties = m_queueFamilyProps;
         *pQueueFamilyPropertyCount = 1;
      }
   }
   else
      *pQueueFamilyPropertyCount = 1;
}

void PhysicalDevice::GetPhysicalDeviceQueueFamilyProperties2KHR(
   uint32_t                     *pQueueFamilyPropertyCount,
   VkQueueFamilyProperties2KHR  *pQueueFamilyProperties) noexcept
{
   if (pQueueFamilyProperties == nullptr)
   {
      GetPhysicalDeviceQueueFamilyProperties(pQueueFamilyPropertyCount, nullptr);
   }
   else if (*pQueueFamilyPropertyCount > 0)
   {
      std::vector<VkQueueFamilyProperties> qfps(*pQueueFamilyPropertyCount);

      GetPhysicalDeviceQueueFamilyProperties(pQueueFamilyPropertyCount, qfps.data());

      uint32_t i = 0;
      for (auto &qfp : qfps)
         pQueueFamilyProperties[i++].queueFamilyProperties = qfp;
   }
}

void PhysicalDevice::GetPhysicalDeviceMemoryProperties(
   VkPhysicalDeviceMemoryProperties *pMemoryProperties) noexcept
{
   *pMemoryProperties = m_memProps;
}

void PhysicalDevice::GetPhysicalDeviceMemoryProperties2KHR(
   VkPhysicalDeviceMemoryProperties2KHR   *pMemoryProperties) noexcept
{
   GetPhysicalDeviceMemoryProperties(&pMemoryProperties->memoryProperties);
}

VkResult PhysicalDevice::EnumerateDeviceExtensionProperties(
   const char              *pLayerName,
   uint32_t                *pPropertyCount,
   VkExtensionProperties   *pProperties) noexcept
{
   static VkExtensionProperties exts[] = {
      { VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_SPEC_VERSION },
      { VK_KHR_MAINTENANCE1_EXTENSION_NAME,                 VK_KHR_MAINTENANCE1_SPEC_VERSION                 },
      { VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,                VK_KHR_BIND_MEMORY_2_SPEC_VERSION                },
      { VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,    VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION    },
      { VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,         VK_KHR_DEDICATED_ALLOCATION_SPEC_VERSION         },
      { VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME, VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_SPEC_VERSION },

      { VK_KHR_RELAXED_BLOCK_LAYOUT_EXTENSION_NAME,         VK_KHR_RELAXED_BLOCK_LAYOUT_SPEC_VERSION         },
#if EMBEDDED_SETTOP_BOX && !defined(VK_USE_PLATFORM_WAYLAND) && !defined(VK_USE_PLATFORM_ANDROID_KHR)
      { VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME,            VK_KHR_DISPLAY_SWAPCHAIN_SPEC_VERSION            },
#endif
#if VK_USE_PLATFORM_ANDROID_KHR
      { VK_ANDROID_NATIVE_BUFFER_EXTENSION_NAME,            VK_ANDROID_NATIVE_BUFFER_SPEC_VERSION            },
#else
      { VK_KHR_SWAPCHAIN_EXTENSION_NAME,                    VK_KHR_SWAPCHAIN_SPEC_VERSION                    },
#endif
   };

   return Extensions::EnumerateExtensionProperties(exts, countof(exts), pLayerName,
                                                   pPropertyCount, pProperties);
}

VkResult PhysicalDevice::EnumerateDeviceLayerProperties(
   uint32_t          *pPropertyCount,
   VkLayerProperties *pProperties) noexcept
{
   *pPropertyCount = 0;
   return VK_SUCCESS;
}

void PhysicalDevice::GetPhysicalDeviceSparseImageFormatProperties(
   VkFormat                       format,
   VkImageType                    type,
   VkSampleCountFlagBits          samples,
   VkImageUsageFlags              usage,
   VkImageTiling                  tiling,
   uint32_t                      *pPropertyCount,
   VkSparseImageFormatProperties *pProperties) noexcept
{
   *pPropertyCount = 0;
}

void PhysicalDevice::GetPhysicalDeviceSparseImageFormatProperties2KHR(
   const VkPhysicalDeviceSparseImageFormatInfo2KHR *pFormatInfo,
   uint32_t                                        *pPropertyCount,
   VkSparseImageFormatProperties2KHR               *pProperties) noexcept
{
   *pPropertyCount = 0;
}

VkResult PhysicalDevice::GetPhysicalDeviceSurfaceSupportKHR(
   uint32_t     queueFamilyIndex,
   VkSurfaceKHR surface,
   VkBool32    *pSupported) noexcept
{
   *pSupported = m_platform.HasSurfacePresentationSupport(surface);
   return VK_SUCCESS;
}

VkResult PhysicalDevice::GetPhysicalDeviceSurfaceCapabilitiesKHR(
   VkSurfaceKHR                surface,
   VkSurfaceCapabilitiesKHR   *pSurfaceCapabilities) noexcept
{
   return m_platform.GetSurfaceCapabilities(surface, pSurfaceCapabilities);
}

VkResult PhysicalDevice::GetPhysicalDeviceSurfaceFormatsKHR(
   VkSurfaceKHR          surface,
   uint32_t             *pSurfaceFormatCount,
   VkSurfaceFormatKHR   *pSurfaceFormats) noexcept
{
   return m_platform.GetSurfaceFormats(surface, pSurfaceFormatCount, pSurfaceFormats);
}

VkResult PhysicalDevice::GetPhysicalDeviceSurfacePresentModesKHR(
   VkSurfaceKHR       surface,
   uint32_t          *pPresentModeCount,
   VkPresentModeKHR  *pPresentModes) noexcept
{
   return m_platform.GetSurfacePresentModes(surface, pPresentModeCount, pPresentModes);
}

VkResult PhysicalDevice::GetPhysicalDeviceDisplayPropertiesKHR(
   uint32_t                *pPropertyCount,
   VkDisplayPropertiesKHR  *pProperties) noexcept
{
   return m_platform.GetDisplayProperties(pPropertyCount, pProperties);
}

VkResult PhysicalDevice::GetPhysicalDeviceDisplayPlanePropertiesKHR(
   uint32_t                      *pPropertyCount,
   VkDisplayPlanePropertiesKHR   *pProperties) noexcept
{
   return m_platform.GetDisplayPlaneProperties(pPropertyCount, pProperties);
}

VkResult PhysicalDevice::GetDisplayPlaneSupportedDisplaysKHR(
   uint32_t        planeIndex,
   uint32_t       *pDisplayCount,
   VkDisplayKHR   *pDisplays) noexcept
{
   return m_platform.GetDisplayPlaneSupportedDisplaysKHR(planeIndex, pDisplayCount, pDisplays);
}

VkResult PhysicalDevice::GetDisplayModePropertiesKHR(
   VkDisplayKHR                display,
   uint32_t                   *pPropertyCount,
   VkDisplayModePropertiesKHR *pProperties) noexcept
{
   return m_platform.GetDisplayModeProperties(display, pPropertyCount, pProperties);
}

VkResult PhysicalDevice::CreateDisplayModeKHR(
   VkDisplayKHR                      display,
   const VkDisplayModeCreateInfoKHR *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkDisplayModeKHR                 *pMode) noexcept
{
   // This driver does not support creating new display modes, it only reports
   // the currently set mode exposed by the underlying Nexus platform for the
   // display. To pass deqp however we have to be able to "create" a mode
   // using the parameters of the first mode we report as supported. This
   // is pants but the spec says nothing about returning the same handle if
   // the mode is already known about.
   //
   // ASIDE: This API appears to be pointless, the visible width/height and
   //        refresh rate is nowhere near enough information to actually
   //        construct a valid display mode timing. You would need the
   //        pixelclock and H&V blanking regions to actually create a usable
   //        display mode timing; so it really isn't clear at all what this API
   //        is intended for.
   VkDisplayModePropertiesKHR mode;
   uint32_t count = 1;
   VkResult err   = m_platform.GetDisplayModeProperties(display, &count, &mode);
   if (err != VK_SUCCESS || count != 1)
      return VK_ERROR_INITIALIZATION_FAILED;

   if (mode.parameters.visibleRegion.width != pCreateInfo->parameters.visibleRegion.width   ||
       mode.parameters.visibleRegion.height != pCreateInfo->parameters.visibleRegion.height ||
       mode.parameters.refreshRate != pCreateInfo->parameters.refreshRate)
      return VK_ERROR_INITIALIZATION_FAILED;

   *pMode = mode.displayMode;
   return VK_SUCCESS;
}

VkResult PhysicalDevice::GetDisplayPlaneCapabilitiesKHR(
   VkDisplayModeKHR               mode,
   uint32_t                       planeIndex,
   VkDisplayPlaneCapabilitiesKHR *pCapabilities) noexcept
{
   return m_platform.GetDisplayPlaneCapabilities(mode, planeIndex, pCapabilities);
}

#ifdef VK_USE_PLATFORM_XLIB_KHR

VkBool32 PhysicalDevice::GetPhysicalDeviceXlibPresentationSupportKHR(
   uint32_t  queueFamilyIndex,
   Display  *dpy,
   VisualID  visualID) noexcept
{
   return VK_TRUE;
}

#endif // VK_USE_PLATFORM_XLIB_KHR

#ifdef VK_USE_PLATFORM_XCB_KHR

VkBool32 PhysicalDevice::GetPhysicalDeviceXcbPresentationSupportKHR(
   uint32_t           queueFamilyIndex,
   xcb_connection_t  *connection,
   xcb_visualid_t     visual_id) noexcept
{
   return VK_TRUE;
}

#endif // VK_USE_PLATFORM_XCB_KHR

#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VkBool32 PhysicalDevice::GetPhysicalDeviceWaylandPresentationSupportKHR(
   uint32_t           queueFamilyIndex,
   struct wl_display *display) noexcept
{
   return VK_TRUE;
}

#endif // VK_USE_PLATFORM_WAYLAND_KHR

#ifdef VK_USE_PLATFORM_WIN32_KHR

VkBool32 PhysicalDevice::GetPhysicalDeviceWin32PresentationSupportKHR(
   uint32_t  queueFamilyIndex) noexcept
{
   return VK_TRUE;
}

#endif // VK_USE_PLATFORM_WIN32_KHR


uint32_t PhysicalDevice::FindSuitableMemoryTypeIndex(VkMemoryPropertyFlags flags) const
{
   for (uint32_t t = 0; t < GetNumMemoryTypes(); t++)
   {
      VkMemoryType memType = GetMemoryType(t);
      if ((memType.propertyFlags & flags) == flags)
         return t;
   }

   demand(false);
   unreachable();
}

uint32_t PhysicalDevice::CalculateMemoryTypeBits(
   VkMemoryPropertyFlags mustHave,
   VkMemoryPropertyFlags exclude)
{
   uint32_t bits = 0;

   // Trawl the list of memory types matching mustHave and exclude
   for (uint32_t t = 0; t < GetNumMemoryTypes(); t++)
   {
      VkMemoryType memType = GetMemoryType(t);

      if ((memType.propertyFlags & mustHave) == mustHave &&
          (memType.propertyFlags & exclude) == 0)
      {
         bits |= 1 << t;
      }
   }

   return bits;
}

void PhysicalDevice::InitFeatures()
{
   m_features.robustBufferAccess                      = true;  // The only mandatory feature
   m_features.fullDrawIndexUint32                     = false;
   m_features.imageCubeArray                          = true;
   m_features.independentBlend                        = true;
   m_features.geometryShader                          = false;
   m_features.tessellationShader                      = false;
   m_features.sampleRateShading                       = true;
   m_features.dualSrcBlend                            = false;
   m_features.logicOp                                 = false;
   m_features.multiDrawIndirect                       = true;
   m_features.drawIndirectFirstInstance               = true;
   m_features.depthClamp                              = false;
   m_features.depthBiasClamp                          = true;
   m_features.fillModeNonSolid                        = true;
   m_features.depthBounds                             = false;
   m_features.wideLines                               = true;
   m_features.largePoints                             = true;
   m_features.alphaToOne                              = false;
   m_features.multiViewport                           = false;
   m_features.samplerAnisotropy                       = true;
   m_features.textureCompressionETC2                  = true;
   m_features.textureCompressionASTC_LDR              = true;
   m_features.textureCompressionBC                    = false;
   m_features.occlusionQueryPrecise                   = true;
   m_features.pipelineStatisticsQuery                 = false;
   m_features.vertexPipelineStoresAndAtomics          = false;
   m_features.fragmentStoresAndAtomics                = true;
   m_features.shaderTessellationAndGeometryPointSize  = false;
   m_features.shaderImageGatherExtended               = true;
   m_features.shaderStorageImageExtendedFormats       = V3D_VER_AT_LEAST(4,2,13,0);
   m_features.shaderStorageImageMultisample           = true;
   m_features.shaderStorageImageReadWithoutFormat     = true;
   m_features.shaderStorageImageWriteWithoutFormat    = false;
   m_features.shaderUniformBufferArrayDynamicIndexing = false;
   m_features.shaderSampledImageArrayDynamicIndexing  = true;
   m_features.shaderStorageBufferArrayDynamicIndexing = false;
   m_features.shaderStorageImageArrayDynamicIndexing  = true;
   m_features.shaderClipDistance                      = false;
   m_features.shaderCullDistance                      = false;
   m_features.shaderFloat64                           = false;
   m_features.shaderInt64                             = false;
   m_features.shaderInt16                             = false;
   m_features.shaderResourceResidency                 = false;
   m_features.shaderResourceMinLod                    = false;
   m_features.sparseBinding                           = false;
   m_features.sparseResidencyBuffer                   = false;
   m_features.sparseResidencyImage2D                  = false;
   m_features.sparseResidencyImage3D                  = false;
   m_features.sparseResidency2Samples                 = false;
   m_features.sparseResidency4Samples                 = false;
   m_features.sparseResidency8Samples                 = false;
   m_features.sparseResidency16Samples                = false;
   m_features.sparseResidencyAliased                  = false;
   m_features.variableMultisampleRate                 = false;

   // We can't really support this the way we have secondary command buffers working right now.
   // When inside a render pass they just make control lists which are branched to from the
   // primary buffer. Supporting this probably requires unrolling a 2ndary when the primary is
   // being recorded, so that it just looks like one big primary. This could be done but would
   // be less efficient when not using queries.
   m_features.inheritedQueries                        = false;
}

bool PhysicalDevice::ValidateRequestedFeatures(const VkPhysicalDeviceFeatures &test) const
{
   static_assert(sizeof(VkPhysicalDeviceFeatures) == 55 * sizeof(VkBool32),
      "VkPhysicalDeviceFeatures does not match expected size in Device::ValidateRequestedFeatures()");

   #define VALIDATE(a) if (test.a && !m_features.a) return false;

   VALIDATE(robustBufferAccess)
   VALIDATE(fullDrawIndexUint32)
   VALIDATE(imageCubeArray)
   VALIDATE(independentBlend)
   VALIDATE(geometryShader)
   VALIDATE(tessellationShader)
   VALIDATE(sampleRateShading)
   VALIDATE(dualSrcBlend)
   VALIDATE(logicOp)
   VALIDATE(multiDrawIndirect)
   VALIDATE(drawIndirectFirstInstance)
   VALIDATE(depthClamp)
   VALIDATE(depthBiasClamp)
   VALIDATE(fillModeNonSolid)
   VALIDATE(depthBounds)
   VALIDATE(wideLines)
   VALIDATE(largePoints)
   VALIDATE(alphaToOne)
   VALIDATE(multiViewport)
   VALIDATE(samplerAnisotropy)
   VALIDATE(textureCompressionETC2)
   VALIDATE(textureCompressionASTC_LDR)
   VALIDATE(textureCompressionBC)
   VALIDATE(occlusionQueryPrecise)
   VALIDATE(pipelineStatisticsQuery)
   VALIDATE(vertexPipelineStoresAndAtomics)
   VALIDATE(fragmentStoresAndAtomics)
   VALIDATE(shaderTessellationAndGeometryPointSize)
   VALIDATE(shaderImageGatherExtended)
   VALIDATE(shaderStorageImageExtendedFormats)
   VALIDATE(shaderStorageImageMultisample)
   VALIDATE(shaderStorageImageReadWithoutFormat)
   VALIDATE(shaderStorageImageWriteWithoutFormat)
   VALIDATE(shaderUniformBufferArrayDynamicIndexing)
   VALIDATE(shaderSampledImageArrayDynamicIndexing)
   VALIDATE(shaderStorageBufferArrayDynamicIndexing)
   VALIDATE(shaderStorageImageArrayDynamicIndexing)
   VALIDATE(shaderClipDistance)
   VALIDATE(shaderCullDistance)
   VALIDATE(shaderFloat64)
   VALIDATE(shaderInt64)
   VALIDATE(shaderInt16)
   VALIDATE(shaderResourceResidency)
   VALIDATE(shaderResourceMinLod)
   VALIDATE(sparseBinding)
   VALIDATE(sparseResidencyBuffer)
   VALIDATE(sparseResidencyImage2D)
   VALIDATE(sparseResidencyImage3D)
   VALIDATE(sparseResidency2Samples)
   VALIDATE(sparseResidency4Samples)
   VALIDATE(sparseResidency8Samples)
   VALIDATE(sparseResidency16Samples)
   VALIDATE(sparseResidencyAliased)
   VALIDATE(variableMultisampleRate)
   VALIDATE(inheritedQueries)

   #undef VALIDATE

   return true;
}

void PhysicalDevice::InitLimits()
{
   constexpr uint64_t _4GB  = UINT64_C(4) * 1024 * 1024 * 1024;
   constexpr uint64_t _64KB = UINT64_C(64) * 1024;

   // TODO: Image sizes are limited by TLB support because of transfer operations. See SWVC5-896
   m_limits.maxImageDimension1D                              = 4096;
   m_limits.maxImageDimension2D                              = 4096;
   m_limits.maxImageDimension3D                              = 4096;
   m_limits.maxImageDimensionCube                            = 4096;
   m_limits.maxImageArrayLayers                              = V3D_MAX_TEXTURE_SIZE;
   m_limits.maxTexelBufferElements                           = 1 << (2 * V3D_MAX_TEXTURE_DIM_BITS);
   m_limits.maxUniformBufferRange                            = DriverLimits::eMaxBufferByte;
   m_limits.maxStorageBufferRange                            = DriverLimits::eMaxBufferByte;
   m_limits.maxPushConstantsSize                             = 0x7FFFFFFF;
   m_limits.maxMemoryAllocationCount                         = _4GB / _64KB; // Number of 64kB pages in 4GB
   m_limits.maxSamplerAllocationCount                        = 64 * 1024;    // Not a hard limit, plucked out of the air
   m_limits.bufferImageGranularity                           = gmem_non_coherent_atom_size();
   m_limits.sparseAddressSpaceSize                           = 0; // When supported 0xFFFFFFFF
   m_limits.maxBoundDescriptorSets                           = DriverLimits::eMaxBoundDescriptorSets;
   m_limits.maxPerStageDescriptorSamplers                    = 16;
   m_limits.maxPerStageDescriptorUniformBuffers              = 12;   // Don't go over 31 -
                                                                     // index 31 is used for push constants
   m_limits.maxPerStageDescriptorStorageBuffers              = 4;
   m_limits.maxPerStageDescriptorSampledImages               = 16;
   m_limits.maxPerStageDescriptorStorageImages               = 4;
   m_limits.maxPerStageDescriptorInputAttachments            = 4;
   m_limits.maxPerStageResources                             = 128;
   m_limits.maxDescriptorSetSamplers                         = 96;
   m_limits.maxDescriptorSetUniformBuffers                   = 72;
   m_limits.maxDescriptorSetUniformBuffersDynamic            = 8;
   m_limits.maxDescriptorSetStorageBuffers                   = 24;
   m_limits.maxDescriptorSetStorageBuffersDynamic            = 4;
   m_limits.maxDescriptorSetSampledImages                    = 96;
   m_limits.maxDescriptorSetStorageImages                    = 24;
   m_limits.maxDescriptorSetInputAttachments                 = 4;
   m_limits.maxVertexInputAttributes                         = V3D_MAX_ATTR_ARRAYS;
   m_limits.maxVertexInputBindings                           = V3D_MAX_ATTR_ARRAYS;
   m_limits.maxVertexInputAttributeOffset                    = 0xFFFFFFFF;
   m_limits.maxVertexInputBindingStride                      = 0xFFFFFFFF;
   m_limits.maxVertexOutputComponents                        = V3D_MAX_VARYING_COMPONENTS;
   m_limits.maxTessellationGenerationLevel                   = 0; // When tessellationShader supported 64;
   m_limits.maxTessellationPatchSize                         = 0; // When tessellationShader supported 32;
   m_limits.maxTessellationControlPerVertexInputComponents   = 0; // When tessellationShader supported 64;
   m_limits.maxTessellationControlPerVertexOutputComponents  = 0; // When tessellationShader supported 64;
   m_limits.maxTessellationControlPerPatchOutputComponents   = 0; // When tessellationShader supported 120;
   m_limits.maxTessellationControlTotalOutputComponents      = 0; // When tessellationShader supported 2048;
   m_limits.maxTessellationEvaluationInputComponents         = 0; // When tessellationShader supported 64;
   m_limits.maxTessellationEvaluationOutputComponents        = 0; // When tessellationShader supported 64;
   m_limits.maxGeometryShaderInvocations                     = 0; // When tessellationShader supported 32;
   m_limits.maxGeometryInputComponents                       = 0; // When tessellationShader supported 64;
   m_limits.maxGeometryOutputComponents                      = 0; // When tessellationShader supported 64;
   m_limits.maxGeometryOutputVertices                        = 0; // When tessellationShader supported 256;
   m_limits.maxGeometryTotalOutputComponents                 = 0; // When tessellationShader supported 1024;
   m_limits.maxFragmentInputComponents                       = V3D_MAX_VARYING_COMPONENTS;
   m_limits.maxFragmentOutputAttachments                     = V3D_MAX_RENDER_TARGETS;
   m_limits.maxFragmentDualSrcAttachments                    = 0; // When dualSrcBlend supported 1
   // Total number of storage buffers, storage images, and output buffers which can be used in fragment stage
   m_limits.maxFragmentCombinedOutputResources               = m_limits.maxPerStageDescriptorStorageBuffers +
                                                               m_limits.maxPerStageDescriptorStorageImages +
                                                               m_limits.maxFragmentOutputAttachments;
   m_limits.maxComputeSharedMemorySize                       = 16384;
   m_limits.maxComputeWorkGroupCount[0]                      = 65535;
   m_limits.maxComputeWorkGroupCount[1]                      = 65535;
   m_limits.maxComputeWorkGroupCount[2]                      = 65535;
#if V3D_USE_CSD
   m_limits.maxComputeWorkGroupInvocations                   = V3D_MAX_CSD_WG_SIZE;
   m_limits.maxComputeWorkGroupSize[0]                       = V3D_MAX_CSD_WG_SIZE;
   m_limits.maxComputeWorkGroupSize[1]                       = V3D_MAX_CSD_WG_SIZE;
   m_limits.maxComputeWorkGroupSize[2]                       = V3D_MAX_CSD_WG_SIZE;
#else
   m_limits.maxComputeWorkGroupInvocations                   = 128;
   m_limits.maxComputeWorkGroupSize[0]                       = 128;
   m_limits.maxComputeWorkGroupSize[1]                       = 128;
   m_limits.maxComputeWorkGroupSize[2]                       = 128;
#endif
   m_limits.subPixelPrecisionBits                            = 4;
   m_limits.subTexelPrecisionBits                            = 8;
   m_limits.mipmapPrecisionBits                              = 8;
   m_limits.maxDrawIndexedIndexValue                         = 0xFFFFFF; // When fullDrawIndexUint32 supported 0xFFFFFFFF;
   m_limits.maxDrawIndirectCount                             = 0x7FFFFFFF;
   m_limits.maxSamplerLodBias                                = 14.0f;
   m_limits.maxSamplerAnisotropy                             = 16.0f;
   m_limits.maxViewports                                     = 1;
   m_limits.maxViewportDimensions[0]                         = V3D_MAX_CLIP_WIDTH;
   m_limits.maxViewportDimensions[1]                         = V3D_MAX_CLIP_HEIGHT;

   uint32_t vpSize = std::max(V3D_MAX_CLIP_WIDTH, V3D_MAX_CLIP_HEIGHT);
   m_limits.viewportBoundsRange[0]                           = -2.0f * vpSize;
   m_limits.viewportBoundsRange[1]                           =  2.0f * vpSize - 1.0f;

   m_limits.viewportSubPixelBits                             = 0;
   m_limits.minMemoryMapAlignment                            = 64;
   m_limits.minTexelBufferOffsetAlignment                    = 256;
   m_limits.minUniformBufferOffsetAlignment                  = 256;
   m_limits.minStorageBufferOffsetAlignment                  = 256;
   m_limits.minTexelOffset                                   = -8;
   m_limits.maxTexelOffset                                   = 7;
   m_limits.minTexelGatherOffset                             = -8;
   m_limits.maxTexelGatherOffset                             = 7;
   m_limits.minInterpolationOffset                           = -0.5f;
   m_limits.maxInterpolationOffset                           = 0.5f;
   m_limits.subPixelInterpolationOffsetBits                  = 4;
   m_limits.maxFramebufferWidth                              = V3D_MAX_CLIP_WIDTH;
   m_limits.maxFramebufferHeight                             = V3D_MAX_CLIP_HEIGHT;
   m_limits.maxFramebufferLayers                             = V3D_MAX_LAYERS;
   m_limits.framebufferColorSampleCounts                     = v3dSampleCounts;
   m_limits.framebufferDepthSampleCounts                     = v3dSampleCounts;
   m_limits.framebufferStencilSampleCounts                   = v3dSampleCounts;
   m_limits.framebufferNoAttachmentsSampleCounts             = v3dSampleCounts;
   m_limits.maxColorAttachments                              = V3D_MAX_RENDER_TARGETS;
   m_limits.sampledImageColorSampleCounts                    = v3dSampleCounts;
   m_limits.sampledImageIntegerSampleCounts                  = v3dSampleCounts;
   m_limits.sampledImageDepthSampleCounts                    = v3dSampleCounts;
   m_limits.sampledImageStencilSampleCounts                  = v3dSampleCounts;
   m_limits.storageImageSampleCounts                         = v3dSampleCounts;
   m_limits.maxSampleMaskWords                               = 1;
   m_limits.timestampComputeAndGraphics                      = false;
   m_limits.timestampPeriod                                  = 0.0f;
   m_limits.maxClipDistances                                 = 0; // When shaderClipDistance supported 8
   m_limits.maxCullDistances                                 = 0; // When shaderCullDistance supported 8
   m_limits.maxCombinedClipAndCullDistances                  = 0; // When shaderCullDistance supported 8
   m_limits.discreteQueuePriorities                          = 2;
   m_limits.pointSizeRange[0]                                = 1.0f;
   m_limits.pointSizeRange[1]                                = V3D_MAX_POINT_SIZE;
   m_limits.lineWidthRange[0]                                = 1.0f;
   m_limits.lineWidthRange[1]                                = V3D_MAX_LINE_WIDTH;
   m_limits.pointSizeGranularity                             = 0.001953125f;
   m_limits.lineWidthGranularity                             = 0.001953125f;
   m_limits.strictLines                                      = false;
   m_limits.standardSampleLocations                          = V3D_HAS_STD_4X_RAST_PATT;
   m_limits.optimalBufferCopyOffsetAlignment                 = 32;  // TODO : what should this be
   m_limits.optimalBufferCopyRowPitchAlignment               = 32;  // TODO : what should this be
   m_limits.nonCoherentAtomSize                              = gmem_non_coherent_atom_size();
}

void PhysicalDevice::InitMemoryProps()
{
   // We will expose a single device memory heap
   m_memProps.memoryHeapCount = 1;
   m_memProps.memoryHeaps[0].size = gmem_heap_size();
   m_memProps.memoryHeaps[0].flags = VK_MEMORY_HEAP_DEVICE_LOCAL_BIT;

   // The heap can expose multiple memory types. Types have to be exposed using an ordering
   // defined in section 10.2 of the spec.
   // "Given two memory types X and Y, the partial order defines X<=Y if:
   //  o The memory property bits set for X are a subset of the memory property bits set for Y. Or,
   //  o The memory property bits set for X are the same as the memory property bits set for Y,
   //    and X uses a memory heap with greater or equal performance (as determined in an
   //    implementation-specific manner)."
   uint32_t idx = 0;
   m_memProps.memoryTypes[idx].heapIndex = 0;
   m_memProps.memoryTypes[idx].propertyFlags =
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |  // Device accessible
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |  // Mappable to host
      VK_MEMORY_PROPERTY_HOST_CACHED_BIT;    // CACHED
   idx++;

   // "It is guaranteed that there is at least one memory type that has its propertyFlags with
   //  the VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT bit set and the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
   //  bit set." In our case, that has to be uncached memory.
   m_memProps.memoryTypes[idx].heapIndex = 0;
   m_memProps.memoryTypes[idx].propertyFlags =
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |     // Device accessible
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |     // Mappable to host
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;     // UNCACHED
   idx++;

   m_memProps.memoryTypes[idx].heapIndex = 0;
   m_memProps.memoryTypes[idx].propertyFlags =
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |     // Device accessible
      VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;  // Lazily backed device only memory
   idx++;

   m_memProps.memoryTypeCount = idx;
}

const uint32_t L = 1;      // Supported for linear images (RSO)
const uint32_t U = 2;      // Supported for optimal images (UIF)
const uint32_t I = L | U;  // Supported for both linear & optimal images
const uint32_t B = 4;      // Supported for buffers
const uint32_t Z = 0;      // Marks the bits which MUST be zero from the spec
                           // i.e. "If format is a block-compression format, then
                           // buffers must not support any features for the format."

// Needed to avoid a narrowing conversion error on Windows
const VkFormatFeatureFlags zero = 0;

static constexpr VkFormatProperties Flags(
   int b1, int b2, int b3, int b4, int b5, int b6,
   int b7, int b8, int b9, int b10, int b11, int b12, int b13)
{
   // Split the table entries into linear, optimal and buffer sections
   return VkFormatProperties
   {
      VkFormatFeatureFlags
      {
         // linearTilingFeatures
         ((b1 & L) ? VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT                : zero) |
         ((b2 & L) ? VK_FORMAT_FEATURE_BLIT_SRC_BIT                     : zero) |
         ((b3 & L) ? VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT  : zero) |
         ((b4 & L) ? VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT                : zero) |
         ((b5 & L) ? VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT         : zero) |
         ((b6 & L) ? VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT             : zero) |
         ((b7 & L) ? VK_FORMAT_FEATURE_BLIT_DST_BIT                     : zero) |
         ((b8 & L) ? VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT       : zero) |
         ((b9 & L) ? VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT     : zero)
      },
      VkFormatFeatureFlags
      {
         // optimalTilingFeatures
         ((b1 & U) ? VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT                : zero) |
         ((b2 & U) ? VK_FORMAT_FEATURE_BLIT_SRC_BIT                     : zero) |
         ((b3 & U) ? VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT  : zero) |
         ((b4 & U) ? VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT                : zero) |
         ((b5 & U) ? VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT         : zero) |
         ((b6 & U) ? VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT             : zero) |
         ((b7 & U) ? VK_FORMAT_FEATURE_BLIT_DST_BIT                     : zero) |
         ((b8 & U) ? VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT       : zero) |
         ((b9 & U) ? VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT     : zero)
      },
      VkFormatFeatureFlags
      {
         // bufferFeatures
         ((b10 & B) ? VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT               : zero) |
         ((b11 & B) ? VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT        : zero) |
         ((b12 & B) ? VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT        : zero) |
         ((b13 & B) ? VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT : zero)
      }
   };
}

void PhysicalDevice::FPSet(VkFormat fmt, VkFormatProperties flags)
{
   // VK_KHR_maintenance1 adds explicit feature flags for SRC and DST
   // transfer (clear & copy). We want this added to any format supporting
   // other hardware functionality. We are not supporting transfer functions
   // on formats that we cannot otherwise use with V3D.
   if (flags.linearTilingFeatures != 0 || flags.optimalTilingFeatures != 0)
   {
      const VkFormatFeatureFlags transferFeatures = VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR |
                                                    VK_FORMAT_FEATURE_TRANSFER_DST_BIT_KHR;

      // Always allow transfer functions in the linear flags for color
      // and compressed formats, we want to be able to copy out of and
      // back into linear images even if no other operation is allowed.
      // However do not allow this for depth/stencil, we do not want to
      // end up supporting the creation of linear depth/stencil images
      // at all.
      const GFX_LFMT_T lfmt = Formats::GetLFMT(fmt);
      if (!gfx_lfmt_has_depth(lfmt) && !gfx_lfmt_has_stencil(lfmt))
         flags.linearTilingFeatures |= transferFeatures;

      // Only allow transfer functions in the optimal flags if there is
      // some other operation supported on optimal images. This prevents
      // the 24bit RGB formats that only have linear color attachment support
      // from gaining any optimal tiling flags, which would break the Image
      // creation code as the formats are not representable in UIF.
      if (flags.optimalTilingFeatures != 0)
         flags.optimalTilingFeatures |= transferFeatures;
   }

   m_formatProperties[fmt] = flags;

   // This checks that anything we've marked as supported actually has an equivalent
   // supported h/w format
   if (flags.linearTilingFeatures || flags.optimalTilingFeatures || flags.bufferFeatures)
   {
      const GFX_LFMT_T lfmt = Formats::GetLFMT(fmt);
      // Although we use the TLB for transfer clears, we do not mention that
      // here as we have to be able to clear formats not natively supported
      // by the TLB.
      const VkFormatFeatureFlags tlbFeatures = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                                               VK_FORMAT_FEATURE_BLIT_DST_BIT;

      if ((flags.linearTilingFeatures  & tlbFeatures) != 0 ||
          (flags.optimalTilingFeatures & tlbFeatures) != 0)
         assert(Formats::HasTLBSupport(lfmt));

      const VkFormatFeatureFlags tmuFeatures = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT |
                                               VK_FORMAT_FEATURE_BLIT_SRC_BIT;

      // No TMU support for linear tiling images.
      assert((flags.linearTilingFeatures & tmuFeatures) == 0);

      if ((flags.optimalTilingFeatures & tmuFeatures) != 0)
         assert(Formats::HasTMUSupport(lfmt));
   }
}

void PhysicalDevice::InitFormatProperties()
{
   // Table 31.13: Mandatory format support: sub-byte channels
   //                                                     ( linear & optimal flags  )(buffer flags)
   //                                                     (       0, I, L or U      )( 0, B or Z  )
   //                                                     (|  |  |  |  |  |  |  |  |)(|  |  |  |  )
   FPSet(VK_FORMAT_UNDEFINED,                        Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R4G4_UNORM_PACK8,                 Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R4G4B4A4_UNORM_PACK16,            Flags(U, U, U, 0, 0, I, I, I, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B4G4R4A4_UNORM_PACK16,            Flags(U, U, U, 0, 0, I, I, I, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_R5G6B5_UNORM_PACK16,              Flags(U, U, U, 0, 0, I, I, I, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_B5G6R5_UNORM_PACK16,              Flags(U, U, U, 0, 0, I, I, I, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_R5G5B5A1_UNORM_PACK16,            Flags(U, U, U, 0, 0, I, I, I, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_B5G5R5A1_UNORM_PACK16,            Flags(U, U, U, 0, 0, I, I, I, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_A1R5G5B5_UNORM_PACK16,            Flags(U, U, U, 0, 0, I, I, I, 0, 0, B, 0, 0));

   // Table 31.14: Mandatory format support: 1-3 byte-sized channels
#if V3D_VER_AT_LEAST(4,2,13,0)
   FPSet(VK_FORMAT_R8_UNORM,                         Flags(U, U, U, U, 0, I, I, I, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8_SNORM,                         Flags(U, U, U, U, 0, 0, 0, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8_USCALED,                       Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8_SSCALED,                       Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8_UINT,                          Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8_SINT,                          Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8_SRGB,                          Flags(U, U, U, 0, 0, 0, 0, 0, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_R8G8_UNORM,                       Flags(U, U, U, U, 0, I, I, I, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8G8_SNORM,                       Flags(U, U, U, U, 0, 0, 0, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8G8_USCALED,                     Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8_SSCALED,                     Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8_UINT,                        Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8G8_SINT,                        Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
#else
   FPSet(VK_FORMAT_R8_UNORM,                         Flags(U, U, U, 0, 0, I, I, I, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R8_SNORM,                         Flags(U, U, U, 0, 0, 0, 0, 0, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R8_USCALED,                       Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8_SSCALED,                       Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8_UINT,                          Flags(U, U, 0, 0, 0, I, I, 0, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R8_SINT,                          Flags(U, U, 0, 0, 0, I, I, 0, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R8_SRGB,                          Flags(U, U, U, 0, 0, 0, 0, 0, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_R8G8_UNORM,                       Flags(U, U, U, 0, 0, I, I, I, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R8G8_SNORM,                       Flags(U, U, U, 0, 0, 0, 0, 0, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R8G8_USCALED,                     Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8_SSCALED,                     Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8_UINT,                        Flags(U, U, 0, 0, 0, I, I, 0, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R8G8_SINT,                        Flags(U, U, 0, 0, 0, I, I, 0, 0, B, B, 0, 0));
#endif
   FPSet(VK_FORMAT_R8G8_SRGB,                        Flags(U, U, U, 0, 0, 0, 0, 0, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_R8G8B8_UNORM,                     Flags(0, 0, 0, 0, 0, L, L, L, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8B8_SNORM,                     Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8B8_USCALED,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8B8_SSCALED,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8B8_UINT,                      Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8B8_SINT,                      Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8B8_SRGB,                      Flags(0, 0, 0, 0, 0, L, L, L, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B8G8R8_UNORM,                     Flags(0, 0, 0, 0, 0, L, L, L, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B8G8R8_SNORM,                     Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B8G8R8_USCALED,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B8G8R8_SSCALED,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B8G8R8_UINT,                      Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B8G8R8_SINT,                      Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B8G8R8_SRGB,                      Flags(0, 0, 0, 0, 0, L, L, L, 0, 0, 0, 0, 0));

   // Table 31.15: Mandatory format support: 4 byte-sized channels
   FPSet(VK_FORMAT_R8G8B8A8_UNORM,                   Flags(U, U, U, U, 0, I, I, I, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8G8B8A8_SNORM,                   Flags(U, U, U, U, 0, 0, 0, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8G8B8A8_USCALED,                 Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8B8A8_SSCALED,                 Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R8G8B8A8_UINT,                    Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8G8B8A8_SINT,                    Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R8G8B8A8_SRGB,                    Flags(U, U, U, 0, 0, I, I, I, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_B8G8R8A8_UNORM,                   Flags(U, U, U, 0, 0, I, I, I, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_B8G8R8A8_SNORM,                   Flags(U, U, U, 0, 0, 0, 0, 0, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_B8G8R8A8_USCALED,                 Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B8G8R8A8_SSCALED,                 Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B8G8R8A8_UINT,                    Flags(U, U, 0, 0, 0, I, I, 0, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_B8G8R8A8_SINT,                    Flags(U, U, 0, 0, 0, I, I, 0, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_B8G8R8A8_SRGB,                    Flags(U, U, U, 0, 0, I, I, I, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_A8B8G8R8_UNORM_PACK32,            Flags(U, U, U, 0, 0, I, I, I, 0, B, B, B, 0));
   FPSet(VK_FORMAT_A8B8G8R8_SNORM_PACK32,            Flags(U, U, U, 0, 0, 0, 0, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_A8B8G8R8_USCALED_PACK32,          Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_A8B8G8R8_SSCALED_PACK32,          Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_A8B8G8R8_UINT_PACK32,             Flags(U, U, 0, 0, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_A8B8G8R8_SINT_PACK32,             Flags(U, U, 0, 0, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_A8B8G8R8_SRGB_PACK32,             Flags(U, U, U, 0, 0, I, I, I, 0, 0, B, 0, 0));

   // Table 31.16: Mandatory format support : 10-bit channels
   FPSet(VK_FORMAT_A2R10G10B10_UNORM_PACK32,         Flags(U, U, U, U, 0, I, I, I, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_A2R10G10B10_SNORM_PACK32,         Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_A2R10G10B10_USCALED_PACK32,       Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_A2R10G10B10_SSCALED_PACK32,       Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_A2R10G10B10_UINT_PACK32,          Flags(U, U, 0, U, 0, I, I, 0, 0, 0, B, 0, 0));
   FPSet(VK_FORMAT_A2R10G10B10_SINT_PACK32,          Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_A2B10G10R10_UNORM_PACK32,         Flags(U, U, U, 0, 0, I, I, I, 0, B, B, B, 0));
   FPSet(VK_FORMAT_A2B10G10R10_SNORM_PACK32,         Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_A2B10G10R10_USCALED_PACK32,       Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_A2B10G10R10_SSCALED_PACK32,       Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
#if V3D_HAS_GFXH1696_FIX
   FPSet(VK_FORMAT_A2B10G10R10_UINT_PACK32,          Flags(U, U, 0, 0, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_A2B10G10R10_SINT_PACK32,          Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
#else
   FPSet(VK_FORMAT_A2B10G10R10_UINT_PACK32,          Flags(U, U, 0, 0, 0, I, I, 0, 0, 0, B, B, 0));
   FPSet(VK_FORMAT_A2B10G10R10_SINT_PACK32,          Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
#endif

   // Table 31.17: Mandatory format support : 16-bit channels
#if V3D_VER_AT_LEAST(4,2,13,0)
   FPSet(VK_FORMAT_R16_UNORM,                        Flags(U, U, U, U, 0, 0, 0, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16_SNORM,                        Flags(U, U, U, U, 0, 0, 0, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16_USCALED,                      Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16_SSCALED,                      Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16_UINT,                         Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16_SINT,                         Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16_SFLOAT,                       Flags(U, U, U, U, 0, I, I, I, 0, B, B, B, 0));
#else
   FPSet(VK_FORMAT_R16_UNORM,                        Flags(U, U, U, 0, 0, 0, 0, 0, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R16_SNORM,                        Flags(U, U, U, 0, 0, 0, 0, 0, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R16_USCALED,                      Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16_SSCALED,                      Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16_UINT,                         Flags(U, U, 0, 0, 0, I, I, 0, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R16_SINT,                         Flags(U, U, 0, 0, 0, I, I, 0, 0, B, B, 0, 0));
   FPSet(VK_FORMAT_R16_SFLOAT,                       Flags(U, U, U, 0, 0, I, I, I, 0, B, B, 0, 0));
#endif
   FPSet(VK_FORMAT_R16G16_UNORM,                     Flags(U, U, U, U, 0, 0, 0, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16G16_SNORM,                     Flags(U, U, U, U, 0, 0, 0, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16G16_USCALED,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16_SSCALED,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16_UINT,                      Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16G16_SINT,                      Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16G16_SFLOAT,                    Flags(U, U, U, U, 0, I, I, I, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16G16B16_UNORM,                  Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16B16_SNORM,                  Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16B16_USCALED,                Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16B16_SSCALED,                Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16B16_UINT,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16B16_SINT,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16B16_SFLOAT,                 Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16B16A16_UNORM,               Flags(U, U, U, U, 0, 0, 0, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16G16B16A16_SNORM,               Flags(U, U, U, U, 0, 0, 0, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16G16B16A16_USCALED,             Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16B16A16_SSCALED,             Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R16G16B16A16_UINT,                Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16G16B16A16_SINT,                Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R16G16B16A16_SFLOAT,              Flags(U, U, U, U, 0, I, I, I, 0, B, B, B, 0));

   // Table 31.18: Mandatory format support : 32-bit channels
   FPSet(VK_FORMAT_R32_UINT,                         Flags(U, U, 0, U, U, I, I, 0, 0, B, B, B, B));
   FPSet(VK_FORMAT_R32_SINT,                         Flags(U, U, 0, U, U, I, I, 0, 0, B, B, B, B));
   FPSet(VK_FORMAT_R32_SFLOAT,                       Flags(U, U, 0, U, U, I, I, 0, 0, B, B, B, B));
   FPSet(VK_FORMAT_R32G32_UINT,                      Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R32G32_SINT,                      Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R32G32_SFLOAT,                    Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R32G32B32_UINT,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R32G32B32_SINT,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R32G32B32_SFLOAT,                 Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, B, 0, 0, 0));
   FPSet(VK_FORMAT_R32G32B32A32_UINT,                Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R32G32B32A32_SINT,                Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));
   FPSet(VK_FORMAT_R32G32B32A32_SFLOAT,              Flags(U, U, 0, U, 0, I, I, 0, 0, B, B, B, 0));

   // Table 31.19: Mandatory format support : 64-bit/uneven channels and depth/stencil
   FPSet(VK_FORMAT_R64_UINT,                         Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64_SINT,                         Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64_SFLOAT,                       Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64G64_UINT,                      Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64G64_SINT,                      Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64G64_SFLOAT,                    Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64G64B64_UINT,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64G64B64_SINT,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64G64B64_SFLOAT,                 Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64G64B64A64_UINT,                Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64G64B64A64_SINT,                Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_R64G64B64A64_SFLOAT,              Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_B10G11R11_UFLOAT_PACK32,          Flags(U, U, U, U, 0, I, I, I, 0, 0, B, B, 0));
   FPSet(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,           Flags(U, U, U, 0, 0, 0, 0, 0, 0, 0, B, 0, 0));
#if V3D_HAS_TMU_R32F_R16_SHAD
   FPSet(VK_FORMAT_D16_UNORM,                        Flags(U, U, U, U, 0, 0, 0, 0, U, B, B, B, 0));
#elif V3D_VER_AT_LEAST(4,2,13,0)
   FPSet(VK_FORMAT_D16_UNORM,                        Flags(U, U, 0, U, 0, 0, 0, 0, U, B, B, B, 0));
#else
   FPSet(VK_FORMAT_D16_UNORM,                        Flags(U, U, 0, 0, 0, 0, 0, 0, U, B, B, 0, 0));
#endif
   FPSet(VK_FORMAT_X8_D24_UNORM_PACK32,              Flags(U, U, 0, U, 0, 0, 0, 0, U, 0, B, 0, 0));
   FPSet(VK_FORMAT_D32_SFLOAT,                       Flags(U, U, 0, U, U, 0, 0, 0, U, B, B, B, B));
   // S8 not fully supported yet, and not required, so disable
   FPSet(VK_FORMAT_S8_UINT,                          Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_D16_UNORM_S8_UINT,                Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
   FPSet(VK_FORMAT_D24_UNORM_S8_UINT,                Flags(0, 0, 0, 0, 0, 0, 0, 0, U, 0, 0, 0, 0));
   FPSet(VK_FORMAT_D32_SFLOAT_S8_UINT,               Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));

   // Table 31.20: Mandatory format support: BC compressed formats with VkImageType VK_IMAGE_TYPE_2D and VK_IMAGE_TYPE_3D
   FPSet(VK_FORMAT_BC1_RGB_UNORM_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC1_RGB_SRGB_BLOCK,               Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC1_RGBA_UNORM_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC1_RGBA_SRGB_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC2_UNORM_BLOCK,                  Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC2_SRGB_BLOCK,                   Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC3_UNORM_BLOCK,                  Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC3_SRGB_BLOCK,                   Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC4_UNORM_BLOCK,                  Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC4_SNORM_BLOCK,                  Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC5_UNORM_BLOCK,                  Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC5_SNORM_BLOCK,                  Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC6H_UFLOAT_BLOCK,                Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC6H_SFLOAT_BLOCK,                Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC7_UNORM_BLOCK,                  Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_BC7_SRGB_BLOCK,                   Flags(0, 0, 0, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));

   // Table 31.21: Mandatory format support: ETC2 and EAC compressed formats with VkImageType VK_IMAGE_TYPE_2D
   FPSet(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,          Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,           Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,        Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,         Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,        Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,         Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_EAC_R11_UNORM_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_EAC_R11_SNORM_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_EAC_R11G11_UNORM_BLOCK,           Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_EAC_R11G11_SNORM_BLOCK,           Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));

   // Table 31.22: Mandatory format support: ASTC LDR compressed formats with VkImageType VK_IMAGE_TYPE_2D
   FPSet(VK_FORMAT_ASTC_4x4_UNORM_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_4x4_SRGB_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_5x4_UNORM_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_5x4_SRGB_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_5x5_UNORM_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_5x5_SRGB_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_6x5_UNORM_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_6x5_SRGB_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_6x6_UNORM_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_6x6_SRGB_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_8x5_UNORM_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_8x5_SRGB_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_8x6_UNORM_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_8x6_SRGB_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_8x8_UNORM_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_8x8_SRGB_BLOCK,              Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_10x5_UNORM_BLOCK,            Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_10x5_SRGB_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_10x6_UNORM_BLOCK,            Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_10x6_SRGB_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_10x8_UNORM_BLOCK,            Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_10x8_SRGB_BLOCK,             Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_10x10_UNORM_BLOCK,           Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_10x10_SRGB_BLOCK,            Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_12x10_UNORM_BLOCK,           Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_12x10_SRGB_BLOCK,            Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_12x12_UNORM_BLOCK,           Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
   FPSet(VK_FORMAT_ASTC_12x12_SRGB_BLOCK,            Flags(U, U, U, 0, 0, 0, 0, 0, 0, Z, Z, Z, Z));
}

void PhysicalDevice::InitCompilerTables()
{
   glsl_prim_init();
}

void PhysicalDeviceDispatchableWrapper::AcquireDevice()
{
   m_physicalDevice = PhysicalDevice::Acquire();
}

PhysicalDeviceDispatchableWrapper::~PhysicalDeviceDispatchableWrapper()
{
   if (m_physicalDevice != nullptr)
      m_physicalDevice->Release();
}

} // namespace bvk
