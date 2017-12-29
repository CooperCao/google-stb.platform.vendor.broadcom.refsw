/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "DeviceMemory.h"
#include "Options.h"

#include "libs/core/lfmt/lfmt_translate_v3d.h"

#if KHRN_DEBUG
#include "libs/tools/txtfmt/txtfmt_gfx_buffer.h"
#endif

#if VK_USE_PLATFORM_ANDROID_KHR
#include "vulkan/vk_android_native_buffer.h"
#include <cutils/atomic.h>
#include <gralloc_priv.h>

// For getprogname(3) or program_invocation_short_name.
#if defined(__ANDROID__)
#include <stdlib.h>
#elif defined(__GLIBC__)
#include <errno.h>
#endif

#endif

#include <sstream>
#include <chrono>

LOG_DEFAULT_CAT("bvk::Image");

namespace bvk {

#if defined(__GLIBC__)
const char* getprogname() {
  return program_invocation_short_name;
}
#endif

#ifdef WIN32
static const char* dumpPrefixStr = "bcmvulkan";
#else
static const char *dumpPrefixStr = getprogname();
#endif

static void DumpTransferImage(
      const GFX_BUFFER_DESC_T *desc,
      gmem_handle_t            mem,
      JobID                    job,
      const char              *command,
      const std::string       &postfix)
{
#if KHRN_DEBUG
   std::chrono::time_point<std::chrono::system_clock> t;
   t = std::chrono::system_clock::now();

   auto timestamp =
      std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()).count();

   std::ostringstream filename;
   filename << Options::dumpPath << "/" << dumpPrefixStr;
   filename << "_T" << timestamp << "_" << command << "_J" << job << "_" << postfix;
   filename << ".txt"; // The pa tool needs this extension.

   SchedDependencies deps {job};
   v3d_scheduler_wait_jobs(&deps, V3D_SCHED_DEPS_FINALISED);

   void *data = gmem_map_and_invalidate_buffer(mem);

   txtfmt_store_gfx_buffer(desc, data, 0, filename.str().c_str(), nullptr, nullptr);
#endif
}

static void WriteImageDetails(
      std::ostream      &s,
      const char        *prefix,
      const bvk::Image  *img,
      uint32_t           mipLevel,
      uint32_t           layer)
{
   s << prefix << "_" << toHandle<VkImage>(const_cast<Image*>(img)) << "_M" << mipLevel;
   s << (gfx_lfmt_is_3d(img->NaturalLFMT()) ? "_S" : "_A");
   s << layer;
}

static void WriteBufferDetails(
      std::ostream      &s,
      const char        *prefix,
      const bvk::Buffer *buf,
      uint32_t          offset)
{
   s << prefix << "_" << toHandle<VkBuffer>(const_cast<Buffer*>(buf));
   s << "_off_" << offset;
}

// Public helper used by various command implementations to replace the
// format in a buffer descriptor with a different one suitable for the TLB.
// This is used to allow a wider range of operations and formats to use the
// TLB (for example clears of all TMU supported formats), even though the
// Vulkan image format is not directly supported by the TLB hardware.
void Image::AdjustDescForTLBLfmt(
   GFX_BUFFER_DESC_T *desc,
   GFX_LFMT_T         lfmt,
   bool               multisampled)
{
   // Mask the parts of the lfmt that we need to transplant
   // into the descriptor to get a valid TLB format.
   const auto mask = GFX_LFMT_BASE_MASK | GFX_LFMT_TYPE_MASK | GFX_LFMT_CHANNELS_MASK;
   GFX_LFMT_T srcLFMT;

   if (lfmt == GFX_LFMT_NONE)
      srcLFMT = static_cast<GFX_LFMT_T>(desc->planes[0].lfmt & mask);
   else
      srcLFMT = static_cast<GFX_LFMT_T>(lfmt & mask);

   desc->planes[0].lfmt = static_cast<GFX_LFMT_T>((desc->planes[0].lfmt & ~mask) | srcLFMT);
}

// Helper for the image constructor to translate the Vulkan create info into
// GFX usage flags for gfx_buffer_desc_gen()
static gfx_buffer_usage_t CalcGfxBufferUsage(VkImageCreateFlags flags, VkFormat format,
                                             VkImageTiling tiling, VkImageUsageFlags usage)
{
   // Map the Vulkan usage flags into GFX flags
   constexpr uint32_t textureFlags =
      VK_IMAGE_USAGE_SAMPLED_BIT |
      VK_IMAGE_USAGE_STORAGE_BIT |
      VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

   // TODO: fit in transient attachments into the render target configuration
   constexpr uint32_t rtFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   constexpr uint32_t dsFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

   uint32_t gfxUsage = GFX_BUFFER_USAGE_NONE;

   if ((usage & textureFlags) != 0)
      gfxUsage |= GFX_BUFFER_USAGE_V3D_TEXTURE;

   if ((usage & rtFlags) != 0)
      gfxUsage |= GFX_BUFFER_USAGE_V3D_RENDER_TARGET;

   if ((usage & dsFlags) != 0)
      gfxUsage |= GFX_BUFFER_USAGE_V3D_DEPTH_STENCIL;

   if ((flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) != 0)
      gfxUsage |= GFX_BUFFER_USAGE_V3D_CUBEMAP | GFX_BUFFER_USAGE_V3D_TEXTURE;

   constexpr uint32_t transferFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT;

   if ((usage & transferFlags) != 0)
   {
      // For non compressed color and depth/stencil formats that are being
      // used in transfer functions, we need to say we want to use the TLB
      // for the transfer operation. Not all transfer operations will use the
      // TLB of course, but we need the image layout/alignments etc to be
      // compatible just in case.
      if (!Formats::IsCompressed(format) && Formats::HasColor(format))
         gfxUsage |= GFX_BUFFER_USAGE_V3D_RENDER_TARGET;

      if (Formats::HasDepth(format) || Formats::HasStencil(format))
         gfxUsage |= GFX_BUFFER_USAGE_V3D_DEPTH_STENCIL;

      // Source transfer flags implies that the image may be used as a blit
      // source, which requires the image to be read as a texture. However
      // we do not want to always add texture gfx usage as that would prevent us
      // from having raster swapchain images for efficient display presentation.
      //
      // We will therefore deal with any blits from raster images in the blit
      // code as and when needed.
   }
   return (gfx_buffer_usage_t)gfxUsage;
}

void Image::UseImageCreateInfoExtension(const VkImageCreateInfo *pci)
{
   if (pci->pNext)
   {
#if VK_USE_PLATFORM_ANDROID_KHR
      // We know the gralloc buffer our implementation allocated is based
      // on a raster image. The Vulkan usage flags passed to us must therefore
      // be compatible with RSO usage.
      VkNativeBufferANDROID *nativeBufferA = (VkNativeBufferANDROID *)pci->pNext;
      assert(nativeBufferA->sType == VK_STRUCTURE_TYPE_NATIVE_BUFFER_ANDROID);

      auto hnd =  (private_handle_t const*) (nativeBufferA->handle);

      m_autoDevMemBinding = true;

      // Create a gmem handle from the Gralloc buffer handle
      gmem_handle_t external = gmem_from_external_memory(
                     nullptr, nullptr, hnd->nxSurfacePhysicalAddress, nullptr, hnd->oglSize,
                     /*secure=*/false, /*contiguous=*/true, "VK Gralloc Image");

      // Bound the mem to the image
      m_boundMemory = createObject<DeviceMemory, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(
            GetCallbacks(), &g_defaultAllocCallbacks, external);

      assert(m_boundMemory == nullptr);

      m_boundOffset = 0;
      m_externalImage = true;
#endif
   }
}

// Debug helper for the image constructor to trap invalid API usage
static void CheckValidUsage(const VkImageCreateInfo *ci)
{
   // NOTE: If you change any of the asserts make sure that this is also
   //       reflected in what is returned by:
   //       PhysicalDevice::GetPhysicalDeviceImageFormatProperties()

   // Tiling mode
   switch (ci->tiling)
   {
   case VK_IMAGE_TILING_LINEAR:
      // Spec v1.0.26: 31.4.1 Supported Sample Counts. Linear multi-sampled
      // images are not allowed.
      assert(ci->samples == VK_SAMPLE_COUNT_1_BIT);

      // Spec v1.0.26: 11.3 Images, linear images are only required to be
      // supported with 1 miplevel. As they cannot be used as textures with
      // our devices there is no point supporting more.
      assert(ci->mipLevels == 1);

      // Spec v1.0.32: 11.3 Images, depth/stencil linear images are optional.
      // The physical device image format properties will report these formats
      // as not supported for tiling linear usage.
      assert(!Formats::HasDepth(ci->format) && !Formats::HasStencil(ci->format));
      break;
   case VK_IMAGE_TILING_OPTIMAL:
      assert((ci->samples & VK_SAMPLE_COUNT_1_BIT) != 0 ||
             (ci->samples & VK_SAMPLE_COUNT_4_BIT) != 0);
      break;
   default: unreachable();
   }

   // Trap incompatible cubemap configuration
   if (ci->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
   {
      assert(ci->imageType == VK_IMAGE_TYPE_2D);
      assert(ci->arrayLayers >= 6);
      assert(ci->extent.width == ci->extent.height);
      assert(ci->samples == VK_SAMPLE_COUNT_1_BIT);
      assert(ci->tiling == VK_IMAGE_TILING_OPTIMAL);
   }

   // Multisample config
   if (ci->samples == VK_SAMPLE_COUNT_4_BIT)
   {
      assert(!Formats::IsCompressed(ci->format));

      // Spec v1.0.26:
      // - 31.4.1 Supported Sample Counts. Only 2D, tiling optimal
      assert(ci->imageType == VK_IMAGE_TYPE_2D);
      assert(ci->tiling == VK_IMAGE_TILING_OPTIMAL);
      // - 11.3 Images. Valid usage table for vkCreateImage, mipLevels must be 1
      //        if samples != VK_SAMPLE_COUNT_1_BIT
      assert(ci->mipLevels == 1);
   }

   switch (ci->imageType)
   {
   case VK_IMAGE_TYPE_1D:
      // This is listed as required valid usage on image creation
      assert(ci->extent.height == 1);
      assert(ci->extent.depth  == 1);
      break;
   case VK_IMAGE_TYPE_2D:
      // This is listed as required valid usage on image creation
      assert(ci->extent.depth == 1);
      break;
   case VK_IMAGE_TYPE_3D:
      // No arrays of 3D
      assert(ci->arrayLayers == 1);
      break;
   default:
      unreachable();
   }
}

Image::Image(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkImageCreateInfo       *pCreateInfo,
   bool                           externalImage) :
      Allocating(pCallbacks),
      m_extent(pCreateInfo->extent),
      m_mipLevels(pCreateInfo->mipLevels),
      m_arrayLayers(pCreateInfo->arrayLayers),
      m_samples(pCreateInfo->samples),
      m_usage(pCreateInfo->usage),
      m_format(pCreateInfo->format),
      m_autoDevMemBinding(false),
      m_externalImage(externalImage)
{
   // This may override m_externalImage, e.g. the Android surface extension
   // that auto binds to a gralloc buffer.
   UseImageCreateInfoExtension(pCreateInfo);

   // Map the Vulkan usage and creation flags into GFX flags
   gfx_buffer_usage_t gfxUsage = CalcGfxBufferUsage(pCreateInfo->flags, pCreateInfo->format,
                                                    pCreateInfo->tiling, pCreateInfo->usage);

   // Linear images cannot be transient but the spec does not state that it
   // is actually invalid usage and the CTS tests this case and expects the
   // transient flag to be ignored.
   //
   // Also by definition external images cannot be transient either, so ignore
   // the flag in that case as well.
   if (!m_externalImage && pCreateInfo->tiling != VK_IMAGE_TILING_LINEAR)
      m_transient = pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
   else
      m_transient = false;

   // Map the Vulkan format to the natural LFMT. Any translation into a
   // different LFMT to handle hardware limitations, is done in the command
   // buffer command implementations at the point of use.
   GFX_LFMT_T fmt = Formats::GetLFMT(pCreateInfo->format, Formats::NATURAL);

#ifndef NDEBUG
   CheckValidUsage(pCreateInfo);
#endif

   // Note: we will NEVER set a y-flip on any formats in the Vulkan driver.
   // The first data in memory represents the top-left corner of the image (which matches
   // what Nexus expects). This also works for textures, since tex-coord(0,0) means the
   // first memory location in the texture.

   // Dimension
   switch (pCreateInfo->imageType)
   {
   case VK_IMAGE_TYPE_1D: gfx_lfmt_set_dims(&fmt, GFX_LFMT_DIMS_1D); break;
   case VK_IMAGE_TYPE_2D: gfx_lfmt_set_dims(&fmt, GFX_LFMT_DIMS_2D); break;
   case VK_IMAGE_TYPE_3D: gfx_lfmt_set_dims(&fmt, GFX_LFMT_DIMS_3D); break;
   default:
      unreachable();
   }

   // By default, given no specific guidance, gfx_buffer_gen_desc() will pick
   // UIF as its default swizzling even if it isn't actually required by the
   // gfxUsage flags. Presentable swapchain images are defined to be
   // TILILNG_OPTIMAL, but currently all of our display platforms require RSO
   // images. We do not want to have to bounce presentation images in and out of
   // UIF via an internal intermediate buffer, that would be very inefficient.
   // So if the image is external and likely to be a framebuffer/swapchain
   // image, indicate to gfx_buffer_gen_desc() that we want to use RSO.
   const bool useRSO = m_externalImage &&
                       gfxUsage == GFX_BUFFER_USAGE_V3D_RENDER_TARGET &&
                       Formats::HasTLBSupport(pCreateInfo->format) &&
                       pCreateInfo->mipLevels == 1 &&
                       pCreateInfo->samples == VK_SAMPLE_COUNT_1_BIT;

   if (useRSO || pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR)
      fmt = gfx_lfmt_to_rso(fmt);

   // We need to manipulate the width and height for multi-sampled images.
   VkExtent3D extent = pCreateInfo->extent;

   if (pCreateInfo->samples == VK_SAMPLE_COUNT_4_BIT)
   {
      extent.width  *= 2;
      extent.height *= 2;
   }

   size_t size;
   size_t align;

   gfx_buffer_desc_gen(m_mipDescs, &size, &align, gfxUsage,
      extent.width, extent.height, extent.depth,
      pCreateInfo->mipLevels, /* num planes */1, &fmt);

   align = std::max(align, static_cast<size_t>(gmem_non_coherent_atom_size()));  // Match buffer min alignment

   if (pCreateInfo->arrayLayers > 1)
      size = gfx_zround_up(size, align);

   m_arrayStride = size;
   m_align = static_cast<VkDeviceSize>(align);

   // Round up the total size requirement to be consistent with buffer sizing
   m_totalSize = pCreateInfo->arrayLayers * m_arrayStride;
   m_totalSize = gfx_u64round_up(m_totalSize, gmem_non_coherent_atom_size());

   log_trace("Image %p (%ux%ux%u)[%u] align = %u layerSize = %u totalSize = %u", this,
         (unsigned)extent.width, (unsigned)extent.height,(unsigned)extent.depth,
         (unsigned)pCreateInfo->arrayLayers,(unsigned)m_align,
         (unsigned)m_arrayStride, (unsigned)m_totalSize);
   if (log_trace_enabled())
   {
      for (uint32_t m = 0; m < pCreateInfo->mipLevels; m++)
      {
         log_trace("\tMipLevel%2u (%ux%ux%u): offset %u, pitch %u: %s", m,
               (unsigned)m_mipDescs[m].width,
               (unsigned)m_mipDescs[m].height,
               (unsigned)m_mipDescs[m].depth,
               (unsigned)m_mipDescs[m].planes[0].offset,
               (unsigned)m_mipDescs[m].planes[0].pitch,
               gfx_lfmt_desc(m_mipDescs[m].planes[0].lfmt));
      }
   }

#if !V3D_VER_AT_LEAST(4,1,34,0)
   // Underlying hardware format when the native format isn't supported
   // sufficiently by the hardware on v3.3 and v4.0.2 based devices.
   //
   // NOTE: we are interested in preserving the dimensions and swizzling from
   //       base mipmap lfmt so these can be extracted from either this or the
   //       native format when necessary.
   const GFX_LFMT_T hwFmt  = Formats::GetLFMT(pCreateInfo->format, Formats::HW);
   m_hardwareLFMT  = gfx_lfmt_set_format(m_mipDescs[0].planes[0].lfmt, hwFmt);

   // When the hardware cannot use the native format and the tiling mode is
   // linear (which allows the memory to be directly viewed by the CPU) we need
   // to fix up the handling of color components in clears, blits and copies
   // so that we get the expected Vulkan format even though we are using
   // a different hardware format internally.
   //
   // NOTE: We do not do this for tiling optimal images, to allow us to support
   //       a mandatory sub-byte 1555 color format without needing specialized
   //       shaders. However this is potentially incorrect if someone aliases
   //       the image or uses an image view with a different format. We may have
   //       to revisit this issue again if we need to certify v3.3 HW.
   m_keepVulkanComponentOrder = (m_hardwareLFMT != m_mipDescs[0].planes[0].lfmt) &&
                                (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR);

   // Similarly (but internal to the image/buffer copy implementation) copies to
   // and from images need to be converted in and out of the internal hardware
   // format when it is different from the Vulkan format and the image is
   // created tiling optimal.
   m_needCopyConversions = (m_hardwareLFMT != m_mipDescs[0].planes[0].lfmt) &&
                           (pCreateInfo->tiling == VK_IMAGE_TILING_OPTIMAL);
#endif
}

Image::~Image() noexcept
{
   if (m_autoDevMemBinding)
   {
      destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(m_boundMemory, GetCallbacks());
      m_boundOffset = 0;
   }
}

VkResult Image::BindImageMemory(
   bvk::Device       *device,
   bvk::DeviceMemory *memory,
   VkDeviceSize       memoryOffset) noexcept
{
   // image must not already be backed by a memory object
   assert(m_boundMemory == nullptr);

   // memoryOffset must be an integer multiple of the alignment member of the VkMemoryRequirements
   // structure returned from a call to vkGetImageMemoryRequirements with image
   assert(memoryOffset % m_align == 0);
   assert(m_transient == memory->IsLazy());

   m_boundMemory = memory;
   m_boundOffset = memoryOffset;

   return VK_SUCCESS;
}

void Image::GetImageMemoryRequirements(
   bvk::Device          *device,
   VkMemoryRequirements *pMemoryRequirements) noexcept
{
   pMemoryRequirements->size = m_totalSize;
   pMemoryRequirements->alignment = m_align;
   pMemoryRequirements->memoryTypeBits = 0;

   // We need to create a bitMask where each bit represents one supported
   // memory type for this image, as defined in PhysicalDevice::InitMemoryProps()

   // First work out what heap flags this image wants, then match against the list.
   uint32_t mustHave = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

   uint32_t exclude = 0;
   if (m_transient)
   {
      mustHave |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
   }
   else
   {
      mustHave |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      exclude  |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
   }

   pMemoryRequirements->memoryTypeBits = device->GetPhysicalDevice()->
                                         CalculateMemoryTypeBits(mustHave, exclude);
}

void Image::GetImageSparseMemoryRequirements(
   bvk::Device                      *device,
   uint32_t                         *pSparseMemoryRequirementCount,
   VkSparseImageMemoryRequirements  *pSparseMemoryRequirements) noexcept
{
   NOT_IMPLEMENTED_YET;
}

void Image::GetSubresourceLayout(
   const VkImageSubresource   *pSubresource,
   VkSubresourceLayout        *pLayout)
{
   assert(pSubresource->arrayLayer < m_arrayLayers);
   assert(pSubresource->mipLevel < GFX_BUFFER_MAX_MIP_LEVELS);

   GFX_BUFFER_DESC_T &mip = m_mipDescs[pSubresource->mipLevel];

   assert(mip.num_planes == 1);

   GFX_BUFFER_DESC_PLANE_T &plane = mip.planes[0];

   pLayout->size       = gfx_buffer_size(&mip);
   pLayout->offset     = plane.offset + m_boundOffset + pSubresource->arrayLayer * m_arrayStride;
   pLayout->rowPitch   = plane.pitch;
   pLayout->depthPitch = plane.slice_pitch;
   pLayout->arrayPitch = m_arrayStride;
}

void Image::GetImageSubresourceLayout(
   bvk::Device                *device,
   const VkImageSubresource   *pSubresource,
   VkSubresourceLayout        *pLayout) noexcept
{
   GetSubresourceLayout(pSubresource, pLayout);
}

void Image::InitGMemTarget(
   struct v3d_imgconv_gmem_tgt  *target,
   const SchedDependencies      &deps,
   const GFX_BUFFER_DESC_T      &desc,
   uint32_t                      baseOffset,
   const VkOffset3D             &offset) const
{
   assert(m_boundMemory != nullptr);

   AllocateLazyMemory();

   v3d_imgconv_init_gmem_tgt(target,
      m_boundMemory->Handle(), baseOffset,
      &deps, &desc,
      static_cast<unsigned int>(offset.x),
      static_cast<unsigned int>(offset.y),
      static_cast<unsigned int>(offset.z),
      0, static_cast<unsigned int>(m_arrayStride));
}

//////////////////////////////////////////////////////////////////////////////
// CmdImageCopy implementation
static inline void AdjustDescForCopyCompatibility(
   GFX_BUFFER_DESC_T       *dstDesc,
   const GFX_BUFFER_DESC_T &srcDesc)
{
   dstDesc->planes[0].lfmt = gfx_lfmt_set_format(dstDesc->planes[0].lfmt, srcDesc.planes[0].lfmt);
}

static inline void Adjust3DDescTo2DSlice(GFX_BUFFER_DESC_T *desc, size_t slice)
{
   assert(desc->num_planes == 1);
   assert(gfx_lfmt_get_dims(&desc->planes[0].lfmt) == GFX_LFMT_DIMS_3D);
   assert(desc->planes[0].slice_pitch > 0);

   gfx_lfmt_set_dims(&desc->planes[0].lfmt, GFX_LFMT_DIMS_2D);

   desc->depth = 1;
   desc->planes[0].offset += desc->planes[0].slice_pitch * slice;
   desc->planes[0].slice_pitch = 0;
}

JobID Image::CopyOneImageRegion(
   Image                   *pSrcImg,
   const VkImageCopy       &region,
   const SchedDependencies &deps)
{
   log_trace("CopyImage: [%u, %u, %u] pixels from %u layers",
         region.extent.width,
         region.extent.height,
         region.extent.depth,
         region.srcSubresource.layerCount);

   log_trace("\tsrc: image = %p, miplevel = %u, baselayer = %u, offset [%d, %d, %d]",
         pSrcImg,
         region.srcSubresource.mipLevel,
         region.srcSubresource.baseArrayLayer,
         region.srcOffset.x,
         region.srcOffset.y,
         region.srcOffset.z);

   log_trace("\tdst: image = %p, miplevel = %u, baselayer = %u, offset [%d, %d, %d]",
         this,
         region.dstSubresource.mipLevel,
         region.dstSubresource.baseArrayLayer,
         region.dstOffset.x,
         region.dstOffset.y,
         region.dstOffset.z);

   assert(region.dstSubresource.aspectMask == region.srcSubresource.aspectMask);

   // Pull out some useful info about the image formats
   const auto naturalSrcLFMT  = pSrcImg->NaturalLFMT();
   const auto naturalDstLFMT  = NaturalLFMT();
   const bool srcIsCompressed = gfx_lfmt_is_compressed(naturalSrcLFMT);
   const bool dstIsCompressed = gfx_lfmt_is_compressed(naturalDstLFMT);
   const bool srcIs3D         = gfx_lfmt_is_3d(naturalSrcLFMT);
   const bool dstIs3D         = gfx_lfmt_is_3d(naturalDstLFMT);

   // The source and dest sample counts must match, we need to multiply
   // the extent and offsets by 2 for 4x multisampled images.
   int sampleAdjust;
   switch(m_samples)
   {
      case VK_SAMPLE_COUNT_1_BIT: sampleAdjust = 1; break;
      case VK_SAMPLE_COUNT_4_BIT: sampleAdjust = 2; break;
      default: unreachable();
   }

   // Copy the src and dst descriptors so we can modify them for this
   // specific transfer job.
   GFX_BUFFER_DESC_T srcDesc      = pSrcImg->GetDescriptor(region.srcSubresource.mipLevel);
   uint32_t          srcImgOffset = pSrcImg->LayerOffset(region.srcSubresource.baseArrayLayer);

   GFX_BUFFER_DESC_T dstDesc      = GetDescriptor(region.dstSubresource.mipLevel);
   uint32_t          dstImgOffset = LayerOffset(region.dstSubresource.baseArrayLayer);

   // For a 3D source or destination we convert them to 2D image arrays starting
   // at the slice specified as their respective z offsets. This allows
   // 3D->3D copies to be accelerated by the TFU and it allows us to trivially
   // support copying between 2D array layers and 3D slices which was added
   // in VK_KHR_maintenance1.
   uint32_t arrayCount;

   // Save the original slice pitches of 3D images to use as the array pitch
   uint32_t srcSlicePitch = 0;
   uint32_t dstSlicePitch = 0;

   if (srcIs3D)
   {
      assert(region.srcSubresource.layerCount == 1);
      assert(region.srcSubresource.baseArrayLayer == 0);

      srcSlicePitch = srcDesc.planes[0].slice_pitch;
      arrayCount    = region.extent.depth; // Use the number of 3D slices to copy as the array count
      Adjust3DDescTo2DSlice(&srcDesc, region.srcOffset.z);
   }
   else
   {
      arrayCount = region.srcSubresource.layerCount;
   }

   if (dstIs3D)
   {
      assert(region.dstSubresource.layerCount == 1);
      assert(region.dstSubresource.baseArrayLayer == 0);
      assert(arrayCount == region.extent.depth);

      dstSlicePitch = dstDesc.planes[0].slice_pitch;
      Adjust3DDescTo2DSlice(&dstDesc, region.dstOffset.z);
   }
   else
   {
      assert(arrayCount == region.dstSubresource.layerCount);
   }

   // Working copies which may get adjusted for compatible copies where
   // the image format of the source or destination gets re-interpreted
   VkExtent2D extent = {
         region.extent.width  * sampleAdjust,
         region.extent.height * sampleAdjust
   };

   VkOffset3D srcOffset = {
         region.srcOffset.x * sampleAdjust,
         region.srcOffset.y * sampleAdjust,
         0
   };

   VkOffset3D dstOffset = {
         region.dstOffset.x * sampleAdjust,
         region.dstOffset.y * sampleAdjust,
         0
   };

   // We cannot trust the height in the extent or the y offsets will be
   // valid values for 1D images.
   if (gfx_lfmt_is_1d(naturalSrcLFMT))
   {
      extent.height = 1;
      srcOffset.y   = 0;
   }

   if (gfx_lfmt_is_1d(naturalDstLFMT))
   {
      extent.height = 1;
      dstOffset.y   = 0;
   }

   v3d_imgconv_conversion_op dstConversion = V3D_IMGCONV_CONVERSION_FORMAT;
   switch (region.dstSubresource.aspectMask)
   {
      // Combined depth/stencil formats need special treatment when copying
      // just one aspect as the other aspect must be preserved in the
      // destination.
      //
      // Note that we do not support D32S8 in the physical device properites.
      case VK_IMAGE_ASPECT_DEPTH_BIT:
         assert(gfx_lfmt_has_depth(naturalDstLFMT));
         dstConversion = V3D_IMGCONV_CONVERSION_DEPTH_ONLY;
         break;
      case VK_IMAGE_ASPECT_STENCIL_BIT:
         assert(gfx_lfmt_has_stencil(naturalDstLFMT));
         dstConversion = V3D_IMGCONV_CONVERSION_STENCIL_ONLY;
         break;
      case VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT:
         assert(gfx_lfmt_has_depth_stencil(naturalDstLFMT));
         break;
      case VK_IMAGE_ASPECT_COLOR_BIT:
      {
         assert(!gfx_lfmt_has_depth(naturalSrcLFMT) && !gfx_lfmt_has_stencil(naturalSrcLFMT));
         assert(!gfx_lfmt_has_depth(naturalDstLFMT) && !gfx_lfmt_has_stencil(naturalDstLFMT));
         // If the formats do not match
         if (naturalDstLFMT != naturalSrcLFMT)
         {
            if (!srcIsCompressed && !dstIsCompressed)
            {
#if !V3D_VER_AT_LEAST(4,1,34,0)
               // This is horrible... but it passes the CTS tests for the
               // 1555 and BGRA "special" formats where we have to use
               // a different internal format supported on the TLB.
               if (m_needCopyConversions && pSrcImg->m_needCopyConversions)
               {
                  // We can't really handle this in general if the components
                  // in the source and destination internal format are not in
                  // the same order and the same size.
                  //
                  // That needs a two step slow xform, from the internal
                  // format of the src to its natural and then a reinterpreted
                  // conversion from the dst natural to the dst internal.
                  //
                  // However the only current case of class compatible special
                  // formats would be a copy between VK_FORMAT_B8G8R8A8_UNORM
                  // and VK_FORMAT_B8G8R8A8_SRGB. As the component
                  // ordering is the same in both internal formats
                  // we can just do a "memcpy".
                  log_trace("CopyImage: Compatible copy between internal formats");
                  AdjustDescForCopyCompatibility(&dstDesc, srcDesc);
               }
               else if (m_needCopyConversions)
               {
                  // Reinterpret the src as the dst natural format and slow
                  // convert it to the internal format
                  log_trace("CopyImage: reinterpret src to dst internal copy");
                  AdjustDescForTLBLfmt(&srcDesc, naturalDstLFMT, false);
                  AdjustDescForTLBLfmt(&dstDesc, m_hardwareLFMT, false);
               }
               else if (pSrcImg->m_needCopyConversions)
               {
                  // Reinterpret the dst as the src natural and convert the
                  // src internal format to it.
                  log_trace("CopyImage: reinterpret dst to src internal copy");
                  AdjustDescForTLBLfmt(&srcDesc, pSrcImg->HardwareLFMT(), false);
                  AdjustDescForTLBLfmt(&dstDesc, naturalSrcLFMT, false);
               }
               else
#endif
               {
                  // Easy cases, we just need to do a "memcpy" here.
                  log_trace("CopyImage: Compatible color copy");
                  AdjustDescForCopyCompatibility(&dstDesc, srcDesc);
               }
            }
            else
            {
               // When copying between compatible compressed and uncompressed
               // formats, change the uncompressed descriptor to match the
               // compressed descriptor. We need to do it this way in order
               // to get the correct UIF swizzle for compressed data when
               // going between linear and tiled images.
               //
               // Note: if the uncompressed descriptor is one of the
               //       special formats that do not have native support in the
               //       TLB and is in tiling optimal, we have no idea what we
               //       can or should do, so we are ignoring it.
               GFX_LFMT_BASE_DETAIL_T bd;
               gfx_lfmt_base_detail(&bd, srcDesc.planes[0].lfmt);

               if (srcIsCompressed)
               {
                  AdjustDescForCopyCompatibility(&dstDesc, srcDesc);
                  // If this isn't a copy between two compatible compressed
                  // formats (which should have the same dimensions) adjust
                  // the destination width, height and offsets by the
                  // compressed block size
                  if (!dstIsCompressed)
                  {
                     dstDesc.width  *= bd.block_w;
                     dstDesc.height *= bd.block_h;
                     dstOffset.x    *= static_cast<int32_t>(bd.block_w);
                     dstOffset.y    *= static_cast<int32_t>(bd.block_h);
                  }
               }
               else
               {
                  AdjustDescForCopyCompatibility(&srcDesc, dstDesc);
                  // The src offset and the extent need to be multiplied
                  // by the compressed block size now we are treating it
                  // as compressed data.
                  srcOffset.x   *= static_cast<int32_t>(bd.block_w);
                  srcOffset.y   *= static_cast<int32_t>(bd.block_h);
                  extent.width  *= bd.block_w;
                  extent.height *= bd.block_h;
               }
            }
         }
#if !V3D_VER_AT_LEAST(4,1,34,0)
         else
         {
            // The source and destination image formats are the same, but the
            // images may have different tiling properties. If the format is one
            // of the special ones where we have to convert to an internal
            // format for hardware to use then we need to make sure we do the
            // expected conversions.
            if (pSrcImg->m_needCopyConversions && m_keepVulkanComponentOrder)
            {
               // Do a slow conversion from source in internal hardware format
               // the destination in natural format
               AdjustDescForTLBLfmt(&srcDesc, pSrcImg->m_hardwareLFMT, false);
            }
            else if (m_needCopyConversions && pSrcImg->m_keepVulkanComponentOrder)
            {
               // Do a slow conversion from source natural linear format to
               // the destination in internal TLB format
               AdjustDescForTLBLfmt(&dstDesc, m_hardwareLFMT, false);
            }
            // The other combinations are just straight copies and therefore
            // is doesn't actually matter if we use the native or hardware
            // LFMT as long as it is the same choice for both.
         }
#endif
         break;
      }
      default:
         // Any other aspect bit combination is invalid
         assert(0);
         return 0;
   }

   // Create imgconv targets and adjust the array slice pitch if the original
   // image was 3D.
   struct v3d_imgconv_gmem_tgt srcTarget;
   struct v3d_imgconv_gmem_tgt dstTarget;

   pSrcImg->InitGMemTarget(&srcTarget, deps, srcDesc, srcImgOffset, srcOffset);
   if (srcIs3D)
      srcTarget.base.array_pitch = srcSlicePitch;

   this->InitGMemTarget(&dstTarget, deps, dstDesc, dstImgOffset, dstOffset);
   if (dstIs3D)
      dstTarget.base.array_pitch = dstSlicePitch;

   dstTarget.base.conversion = dstConversion;

   JobID jobId = 0;
   bool ret = v3d_imgconv_convert(
      &dstTarget, &srcTarget, &jobId,
      extent.width,
      extent.height,
      1,
      arrayCount,
      false /* TODO: secure_context */);

   assert(ret);

   if (Options::dumpTransferImages)
   {
      std::ostringstream details;

      WriteImageDetails(details, "src",
         pSrcImg,
         region.srcSubresource.mipLevel,
         srcIs3D ? region.srcOffset.z : region.srcSubresource.baseArrayLayer);

      WriteImageDetails(details, "_dst",
         this,
         region.dstSubresource.mipLevel,
         dstIs3D ? region.dstOffset.z : region.dstSubresource.baseArrayLayer);

      // Note: only the first array layer/ 3D slice supported at the moment
      DumpTransferImage(&dstDesc, m_boundMemory->Handle(), jobId, "CopyImage", details.str());
   }

   return jobId;
}

static inline void ValidateCopyFormats(GFX_LFMT_T srcLFMT, GFX_LFMT_T dstLFMT)
{
   if (gfx_lfmt_has_depth(dstLFMT) || gfx_lfmt_has_stencil(dstLFMT) ||
       gfx_lfmt_has_depth(srcLFMT) || gfx_lfmt_has_stencil(srcLFMT))
   {
      // If the formats have depth/stencil the src and dst must be identical
      assert(dstLFMT == srcLFMT);
   }
   else if (gfx_lfmt_is_compressed(dstLFMT) || gfx_lfmt_is_compressed(srcLFMT))
      assert(gfx_lfmt_bytes_per_block(srcLFMT) == gfx_lfmt_bytes_per_block(dstLFMT));
   else if (dstLFMT != srcLFMT)
      assert(gfx_lfmt_bytes_per_block(srcLFMT) == gfx_lfmt_bytes_per_block(dstLFMT));
}

JobID Image::CopyFromImage(Image *pSrcImg, uint32_t regionCount, VkImageCopy *regions, const SchedDependencies &deps)
{
   assert(m_boundMemory != nullptr);
   assert(pSrcImg->m_boundMemory != nullptr);

   SchedDependencies waitJobs;

   assert(m_samples == pSrcImg->Samples());

#ifndef NDEBUG
   const GFX_LFMT_T srcLFMT = pSrcImg->NaturalLFMT();
   const GFX_LFMT_T dstLFMT = NaturalLFMT();
   ValidateCopyFormats(srcLFMT, dstLFMT);
#endif

   for (uint32_t i = 0; i < regionCount; i++)
      waitJobs += CopyOneImageRegion(pSrcImg, regions[i], deps);

   return waitJobs.Amalgamate();
}

static void AdjustBufferFormatForCopy(
      VkImageAspectFlags aspect,
      GFX_LFMT_T        *lfmt,
      bool               isSrc)
{
   // All buffer data is treated as being in linear (raster) order when
   // being re-interpreted as an image.
   gfx_lfmt_set_swizzling(lfmt, GFX_LFMT_SWIZZLING_RSO);
   switch (aspect)
   {
      case VK_IMAGE_ASPECT_DEPTH_BIT:
      {
         // Nothing to do for the other formats currently supported by the
         // driver (which does not include D32S8).
         if (gfx_lfmt_fmt(*lfmt) == GFX_LFMT_S8D24_UINT_UNORM)
         {
            gfx_lfmt_set_base(lfmt,GFX_LFMT_BASE_C24C8);
            if (isSrc)
            {
               // For source buffers set the stencil bits to undefined.
               gfx_lfmt_set_channels(lfmt, GFX_LFMT_CHANNELS_DX);
               gfx_lfmt_set_type(lfmt, GFX_LFMT_TYPE_UNORM);
            }
            else
            {
               // Copy depth & stencil to destination buffer to make it easier
               // to test. Officially, the stencil bits are "don't care", which
               // is ok.
               gfx_lfmt_set_channels(lfmt, GFX_LFMT_CHANNELS_DS);
               gfx_lfmt_set_type(lfmt, GFX_LFMT_TYPE_UNORM_UINT);
            }
         }
         break;
      }

      case VK_IMAGE_ASPECT_STENCIL_BIT:
      {
         // When copying a stencil aspect the buffer data is always
         // read/written as packed VK_FORMAT_S8_UINT
         gfx_lfmt_set_base(lfmt,GFX_LFMT_BASE_C8);
         gfx_lfmt_set_channels(lfmt, GFX_LFMT_CHANNELS_S);
         gfx_lfmt_set_type(lfmt, GFX_LFMT_TYPE_UINT);
         break;
      }

      default:
         // Nothing to do for color aspects
         break;
   }
}

static inline bool IsValidBufferCopyAspect(
      VkImageAspectFlags aspect,
      GFX_LFMT_T         lfmt)
{
   // Buffer copies must only specify either depth or stencil aspects not
   // both, to copy both you must use two separate copy regions
   assert(aspect!= (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));

   // If the region aspect doesn't exist in the color format just do nothing.
   if (aspect == VK_IMAGE_ASPECT_COLOR_BIT && !gfx_lfmt_has_color(lfmt))
         return false;

   if (aspect == VK_IMAGE_ASPECT_DEPTH_BIT && !gfx_lfmt_has_depth(lfmt))
         return false;

   if (aspect == VK_IMAGE_ASPECT_STENCIL_BIT && !gfx_lfmt_has_stencil(lfmt))
         return false;

   return true;
}

JobID Image::CopyOneBufferRegion(
      Buffer                  *pSrcBuf,
      const VkBufferImageCopy &region,
      const SchedDependencies &deps)
{
   auto srcMemory = pSrcBuf->GetBoundMemory();
   assert(srcMemory != nullptr);

   auto srcMemoryOffset = pSrcBuf->GetBoundMemoryOffset();

   log_trace("CopyFromBuffer: buffer offset = %#" PRIx64 ", image = %p",
         region.bufferOffset,
         this);
   log_trace("\tmiplevel = %u, %u@%u layers",
         region.imageSubresource.mipLevel,
         region.imageSubresource.layerCount,
         region.imageSubresource.baseArrayLayer);
   log_trace("\trowlength = %u, image height = %u",
         region.bufferRowLength,
         region.bufferImageHeight);
   log_trace("\t[%u, %u, %u] pixels @ [%d, %d, %d]",
         region.imageExtent.width,
         region.imageExtent.height,
         region.imageExtent.depth,
         region.imageOffset.x,
         region.imageOffset.y,
         region.imageOffset.z);

   GFX_BUFFER_DESC_T dstDesc = GetDescriptor(region.imageSubresource.mipLevel);
   uint32_t dstImgOffset     = LayerOffset(region.imageSubresource.baseArrayLayer);
   const bool dstIs3D        = gfx_lfmt_is_3d(NaturalLFMT());
   const bool dstIs1D        = gfx_lfmt_is_1d(NaturalLFMT());

   assert(dstDesc.num_planes == 1); // No planar formats yet

   // Start with the lfmt of the destination image and modify it for the
   // correct interpretation of the buffer data.
   GFX_LFMT_T srcLFMT = dstDesc.planes[0].lfmt;
   AdjustBufferFormatForCopy(region.imageSubresource.aspectMask, &srcLFMT, /* isSrc */ true);

   // We cannot trust the height in the extent or the y offset will be a
   // valid value for 1D images.
   uint32_t extentHeight = dstIs1D ? 1 : region.imageExtent.height;
   int32_t  offsetY = dstIs1D ? 0 : region.imageOffset.y;
   uint32_t layerCount;

   if (dstIs3D)
   {
      assert(region.imageSubresource.layerCount == 1);
      assert(region.imageSubresource.baseArrayLayer == 0);
      // We are going to turn 3D slices into array layers for imgconv
      gfx_lfmt_set_dims(&srcLFMT, GFX_LFMT_DIMS_2D);
      layerCount = region.imageExtent.depth;
   }
   else
   {
      layerCount = region.imageSubresource.layerCount;
   }

   // Generate the buffer desc for the source buffer
   auto sw = (region.bufferRowLength > 0) ? region.bufferRowLength :
                                            region.imageExtent.width;
   auto sh = (region.bufferImageHeight > 0) ? region.bufferImageHeight :
                                              extentHeight;
   GFX_BUFFER_DESC_T srcDesc;
   size_t srcArraySize, align;

   gfx_buffer_desc_gen(&srcDesc, &srcArraySize, &align, GFX_BUFFER_USAGE_NONE,
                       sw, sh, 1, 1, 1, &srcLFMT);

   // Fixup the offset into the memory block
   srcDesc.planes[0].offset = static_cast<uint32_t>(srcMemoryOffset + region.bufferOffset);

   uint32_t dstArraySize;
   if (dstIs3D)
   {
      dstArraySize = dstDesc.planes[0].slice_pitch;
      Adjust3DDescTo2DSlice(&dstDesc, region.imageOffset.z);
   }
   else
   {
      dstArraySize = m_arrayStride;
   }

#if !V3D_VER_AT_LEAST(4,1,34,0)
   // For tiling optimal images change the destination descriptor to use the
   // internal TLB format, instead of the natural format. This is so in
   // formats where we use a different internal format, because of hardware
   // restrictions, we get the data swizzled into the correct channels by
   // a slowconv copy.
   if (m_needCopyConversions)
      AdjustDescForTLBLfmt(&dstDesc, m_hardwareLFMT, false);
#endif

   struct v3d_imgconv_gmem_tgt srcTarget;
   struct v3d_imgconv_gmem_tgt dstTarget;

   v3d_imgconv_init_gmem_tgt(&srcTarget, srcMemory->Handle(), 0, &deps, &srcDesc,
                             0, 0, 0, 0, static_cast<unsigned int>(srcArraySize));

   AllocateLazyMemory();

   v3d_imgconv_init_gmem_tgt(&dstTarget, m_boundMemory->Handle(), dstImgOffset,
                             &deps, &dstDesc,
                             static_cast<unsigned int>(region.imageOffset.x),
                             static_cast<unsigned int>(offsetY),
                             0, 0, static_cast<unsigned int>(dstArraySize));

   if (gfx_lfmt_has_depth_stencil(dstDesc.planes[0].lfmt))
   {
      switch(region.imageSubresource.aspectMask)
      {
         case VK_IMAGE_ASPECT_DEPTH_BIT:
            dstTarget.base.conversion = V3D_IMGCONV_CONVERSION_DEPTH_ONLY;
            break;
         case VK_IMAGE_ASPECT_STENCIL_BIT:
            dstTarget.base.conversion = V3D_IMGCONV_CONVERSION_STENCIL_ONLY;
            break;
         default:
            unreachable();
      }
   }

   JobID jobId = 0;
   bool ret = v3d_imgconv_convert(&dstTarget, &srcTarget, &jobId,
                               region.imageExtent.width, extentHeight, 1,
                               layerCount, false /* TODO: secure_context */);

   assert(ret);

   if (Options::dumpTransferImages)
   {
      std::ostringstream details;

      WriteBufferDetails(details, "src", pSrcBuf, srcDesc.planes[0].offset);

      WriteImageDetails(details, "_dst",
         this,
         region.imageSubresource.mipLevel,
         dstIs3D ? region.imageOffset.z : region.imageSubresource.baseArrayLayer);

      // Note: only the first array layer supported at the moment
      DumpTransferImage(&dstDesc, m_boundMemory->Handle(), jobId, "CopyBufferToImage", details.str());
   }

   return jobId;
}

JobID Image::CopyFromBuffer(
      Buffer                  *pSrcBuf,
      uint32_t                 regionCount,
      VkBufferImageCopy       *regions,
      const SchedDependencies &deps)
{
   assert(m_boundMemory != nullptr);
   assert(m_samples == VK_SAMPLE_COUNT_1_BIT); // operation not valid for multisample images

   SchedDependencies waitJobs;

   for (uint32_t i = 0; i < regionCount; i++)
   {
      if (!IsValidBufferCopyAspect(regions[i].imageSubresource.aspectMask, m_mipDescs[0].planes[0].lfmt))
         continue;

      waitJobs += CopyOneBufferRegion(pSrcBuf, regions[i], deps);
   }

   return waitJobs.Amalgamate();
}

// Helper for vkCmdCopyImageToBuffer schedule method
JobID Image::CopyToBuffer(
      Buffer                  *pDstBuf,
      uint32_t                 regionCount,
      VkBufferImageCopy       *regions,
      const SchedDependencies &deps)
{
   assert(m_boundMemory != nullptr);
   assert(m_samples == VK_SAMPLE_COUNT_1_BIT); // operation not valid for multisample images

   const bool srcIs1D = gfx_lfmt_is_1d(NaturalLFMT());
   const bool srcIs3D = gfx_lfmt_is_3d(NaturalLFMT());

   VkDeviceSize  dstMemoryOffset = pDstBuf->GetBoundMemoryOffset();
   DeviceMemory *dstMemory       = pDstBuf->GetBoundMemory();
   assert(dstMemory != nullptr);

   // Note: In this direction we do not try and optimize 3D copies to use
   //       the TFU as the destination buffer must be written in raster layout
   //       and the TFU cannot write raster. Instead on the real platform we
   //       should get an ARM Neon copy most of the time.
   SchedDependencies waitJobs;
   for (uint32_t i = 0; i < regionCount; i++)
   {
      log_trace("CopyToBuffer: buffer offset = %#" PRIx64 ", image = %p, miplevel = %u, %u@%u layers, [%u, %u, %u] pixels @ [%d, %d, %d]",
            regions[i].bufferOffset,
            this,
            regions[i].imageSubresource.mipLevel,
            regions[i].imageSubresource.layerCount,
            regions[i].imageSubresource.baseArrayLayer,
            regions[i].imageExtent.width,
            regions[i].imageExtent.height,
            regions[i].imageExtent.depth,
            regions[i].imageOffset.x,
            regions[i].imageOffset.y,
            regions[i].imageOffset.z);

      GFX_BUFFER_DESC_T srcDesc = GetDescriptor(regions[i].imageSubresource.mipLevel);
      uint32_t srcImgOffset     = LayerOffset(regions[i].imageSubresource.baseArrayLayer);

      assert(srcDesc.num_planes == 1); // No planar formats yet

      if (srcIs3D)
      {
         assert(regions[i].imageSubresource.layerCount == 1);
         assert(regions[i].imageSubresource.baseArrayLayer == 0);
      }

      if (!IsValidBufferCopyAspect(regions[i].imageSubresource.aspectMask, NaturalLFMT()))
         continue;

      GFX_LFMT_T dstLFMT = srcDesc.planes[0].lfmt;
      AdjustBufferFormatForCopy(regions[i].imageSubresource.aspectMask, &dstLFMT, /* isSrc */ false);

      // We cannot trust the height and depth in the extent will contain valid
      // values if the image type doesn't need them.
      uint32_t extentHeight = srcIs1D ? 1 : regions[i].imageExtent.height;
      uint32_t extentDepth = srcIs3D ? regions[i].imageExtent.depth : 1;

      // Generate the buffer desc for the destination buffer
      GFX_BUFFER_DESC_T dstDesc;
      size_t size, align;

      auto sw = (regions[i].bufferRowLength > 0) ? regions[i].bufferRowLength :
                                                   regions[i].imageExtent.width;
      auto sh = (regions[i].bufferImageHeight > 0) ? regions[i].bufferImageHeight :
                                                     extentHeight;

      gfx_buffer_desc_gen(&dstDesc, &size, &align, GFX_BUFFER_USAGE_NONE,
                          sw, sh, extentDepth, 1, 1, &dstLFMT);

      // Fixup the offset into the memory block
      dstDesc.planes[0].offset = static_cast<uint32_t>(dstMemoryOffset + regions[i].bufferOffset);

      // Create imgconv source and destination targets and do the copy
      struct v3d_imgconv_gmem_tgt srcTarget;
      struct v3d_imgconv_gmem_tgt dstTarget;

#if !V3D_VER_AT_LEAST(4,1,34,0)
      // For tiling optimal images change the source descriptor to use the
      // internal TLB format, instead of the natural format. This is so in
      // formats where we use a different internal format, because of hardware
      // restrictions, we get the channels swizzled back to the correct native
      // order in the host visible buffer.
      if (m_needCopyConversions)
         AdjustDescForTLBLfmt(&srcDesc, m_hardwareLFMT, false);
#endif

      this->InitGMemTarget(&srcTarget, deps, srcDesc, srcImgOffset, regions[i].imageOffset);

      v3d_imgconv_init_gmem_tgt(&dstTarget, dstMemory->Handle(), 0,
                                &deps, &dstDesc, 0, 0, 0, 0, size);

      JobID jobId = 0;
      bool ret = v3d_imgconv_convert(&dstTarget, &srcTarget, &jobId,
                                  regions[i].imageExtent.width,
                                  extentHeight,
                                  extentDepth,
                                  regions[i].imageSubresource.layerCount,
                                  false /* TODO: secure_context */);

      assert(ret);

      if (Options::dumpTransferImages)
      {
         // Note: only the first array layer supported at the moment
         std::ostringstream details;

         WriteImageDetails(details, "src",
            this,
            regions[i].imageSubresource.mipLevel,
            srcIs3D ? regions[i].imageOffset.z : regions[i].imageSubresource.baseArrayLayer);

         WriteBufferDetails(details, "_dst", pDstBuf, dstDesc.planes[0].offset);

         if (srcIs3D)
            log_warn("%s: Cannot currently dump 3D images", __FUNCTION__);
         else
            DumpTransferImage(&dstDesc, dstMemory->Handle(), jobId, "CopyImageToBuffer", details.str());
      }

      waitJobs += jobId;
   }

   return waitJobs.Amalgamate();
}

void Image::DumpImage(uint32_t arrayLayer, uint32_t mipLevel, const std::string &basename) const
{
#if KHRN_DEBUG
   if (m_boundMemory->Handle() != GMEM_HANDLE_INVALID)
   {
      GFX_BUFFER_DESC_T desc = GetDescriptor(mipLevel);

      std::ostringstream details;
      details << Options::dumpPath << "/";
      WriteImageDetails(details, basename.c_str(), this, mipLevel, arrayLayer);
      details << ".txt";

      char *data = static_cast<char *>(gmem_map_and_invalidate_buffer(m_boundMemory->Handle())) + LayerOffset(arrayLayer);
      void *d = static_cast<void *>(data);

      txtfmt_store_gfx_buffer(&desc, d, 0, details.str().c_str(), nullptr, nullptr);
   }
#endif
}

} // namespace bvk
