/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
   s << (gfx_lfmt_is_3d(img->LFMT()) ? "_S" : "_A");
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

// Helper for the image constructor to translate the Vulkan create info into
// GFX usage flags for gfx_buffer_desc_gen()
static gfx_buffer_usage_t CalcGfxBufferUsage(VkImageCreateFlags flags, GFX_LFMT_T lfmt,
                                             VkImageTiling tiling, VkImageUsageFlags usage)
{
   uint32_t gfxUsage = GFX_BUFFER_USAGE_NONE;

   // Map the Vulkan usage flags into GFX flags
   constexpr uint32_t textureFlags =
      VK_IMAGE_USAGE_SAMPLED_BIT |
      VK_IMAGE_USAGE_STORAGE_BIT |
      VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

   // TODO: fit in transient attachments into the render target configuration
   constexpr uint32_t rtFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   constexpr uint32_t dsFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

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
      if (!gfx_lfmt_is_compressed(lfmt) && gfx_lfmt_has_color(lfmt))
         gfxUsage |= GFX_BUFFER_USAGE_V3D_RENDER_TARGET;

      if (gfx_lfmt_has_depth(lfmt) || gfx_lfmt_has_stencil(lfmt))
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

   GFX_LFMT_T fmt = Formats::GetLFMT(pCreateInfo->format);

   // Map the Vulkan usage and creation flags into GFX flags
   gfx_buffer_usage_t gfxUsage = CalcGfxBufferUsage(pCreateInfo->flags, fmt,
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
                       gfx_lfmt_can_render_format(Formats::GetLFMT(pCreateInfo->format)) &&
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
   /* Valid usage prevents this function from actually being uesd */
   unreachable();
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

static v3d_imgconv_conversion_op CopyAspectToImgconvOp(
   VkImageAspectFlags aspectMask, GFX_LFMT_T srcLFMT, GFX_LFMT_T dstLFMT)
{
   v3d_imgconv_conversion_op dstConversion = V3D_IMGCONV_CONVERSION_FORMAT;
   switch (aspectMask)
   {
      // Combined depth/stencil formats need special treatment when copying
      // just one aspect as the other aspect must be preserved in the
      // destination.
      //
      // Note that we do not support D32S8 in the physical device properites.
      case VK_IMAGE_ASPECT_DEPTH_BIT:
         assert(gfx_lfmt_has_depth(dstLFMT));
         dstConversion = V3D_IMGCONV_CONVERSION_DEPTH_ONLY;
         break;
      case VK_IMAGE_ASPECT_STENCIL_BIT:
         assert(gfx_lfmt_has_stencil(dstLFMT));
         dstConversion = V3D_IMGCONV_CONVERSION_STENCIL_ONLY;
         break;
      case VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT:
         assert(gfx_lfmt_has_depth_stencil(dstLFMT));
         break;
      case VK_IMAGE_ASPECT_COLOR_BIT:
      {
         assert(!gfx_lfmt_has_depth(srcLFMT) && !gfx_lfmt_has_stencil(srcLFMT));
         assert(!gfx_lfmt_has_depth(dstLFMT) && !gfx_lfmt_has_stencil(dstLFMT));
         break;
      }
      default:
         // Any other aspect bit combination is invalid
         unreachable();
   }
   return dstConversion;
}

static void CalculateCopyExtentAndOffsets(
   const VkImageCopy    &region,
   VkSampleCountFlagBits samples,
   GFX_LFMT_T            srcLFMT,
   GFX_LFMT_T            dstLFMT,
   VkExtent2D           *extent,
   VkOffset3D           *srcOffset,
   VkOffset3D           *dstOffset)
{
   // The source and dest sample counts must match, we need to multiply
   // the extent and offsets by 2 for 4x multisampled images.
   int sampleAdjust;
   switch(samples)
   {
      case VK_SAMPLE_COUNT_1_BIT: sampleAdjust = 1; break;
      case VK_SAMPLE_COUNT_4_BIT: sampleAdjust = 2; break;
      default: unreachable();
   }

   extent->width  = region.extent.width  * sampleAdjust;
   extent->height = region.extent.height * sampleAdjust;

   srcOffset->x = region.srcOffset.x * sampleAdjust;
   srcOffset->y = region.srcOffset.y * sampleAdjust;
   srcOffset->z = 0;

   dstOffset->x = region.dstOffset.x * sampleAdjust;
   dstOffset->y = region.dstOffset.y * sampleAdjust;
   dstOffset->z = 0;

   // We cannot trust the height in the extent or the y offsets will be
   // valid values for 1D images.
   //
   // Note: 1-D multisampled images are not allowed by the spec (see 31.4.1
   //       supported sample counts) so the height is set explicitly to 1
   if (gfx_lfmt_is_1d(srcLFMT))
   {
      extent->height = 1;
      srcOffset->y   = 0;
   }

   if (gfx_lfmt_is_1d(dstLFMT))
   {
      extent->height = 1;
      dstOffset->y   = 0;
   }
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
   const auto srcLFMT         = pSrcImg->LFMT();
   const auto dstLFMT         = LFMT();
   const bool srcIs3D         = gfx_lfmt_is_3d(srcLFMT);
   const bool dstIs3D         = gfx_lfmt_is_3d(dstLFMT);

   // Copy the src and dst descriptors so we can modify them for this
   // specific transfer job.
   GFX_BUFFER_DESC_T srcDesc  = pSrcImg->GetDescriptor(region.srcSubresource.mipLevel);
   GFX_BUFFER_DESC_T dstDesc  = GetDescriptor(region.dstSubresource.mipLevel);

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

   VkExtent2D extent;
   VkOffset3D srcOffset, dstOffset;

   CalculateCopyExtentAndOffsets(region, m_samples, srcLFMT, dstLFMT,
                                 &extent, &srcOffset, &dstOffset);

   // Create imgconv targets and adjust the array slice pitch if the original
   // image was 3D.
   struct v3d_imgconv_gmem_tgt srcTarget;
   const uint32_t srcImgOffset = pSrcImg->LayerOffset(region.srcSubresource.baseArrayLayer);

   pSrcImg->InitGMemTarget(&srcTarget, deps, srcDesc, srcImgOffset, srcOffset);
   if (srcIs3D)
      srcTarget.base.array_pitch = srcSlicePitch;

   struct v3d_imgconv_gmem_tgt dstTarget;
   const uint32_t dstImgOffset = LayerOffset(region.dstSubresource.baseArrayLayer);

   this->InitGMemTarget(&dstTarget, deps, dstDesc, dstImgOffset, dstOffset);
   if (dstIs3D)
      dstTarget.base.array_pitch = dstSlicePitch;

   dstTarget.base.conversion =
      CopyAspectToImgconvOp(region.dstSubresource.aspectMask, srcLFMT, dstLFMT);

   JobID jobId = 0;
   // Use v3d_imgconv_memcpy which will handle all compatible color format
   // copies, including compressed<->uncompressed copies, for us when the
   // source is not equal to the destination format.
   //
   // Note: the "memcpy" refers to the image data values, it will still
   //       swizzle between UIF and RSO layouts and honour the depth/stencil
   //       destination write masking.
   bool ret = v3d_imgconv_memcpy(&dstTarget, &srcTarget, &jobId,
                                 extent.width, extent.height, /* depth */ 1,
                                 arrayCount, false /* TODO: secure_context */);
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
   const GFX_LFMT_T srcLFMT = pSrcImg->LFMT();
   const GFX_LFMT_T dstLFMT = LFMT();
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

static bool LogCopyBufferRegion(
   const Image             *image,
   const VkBufferImageCopy &region,
   const char              *prefix)
{
   log_trace("%s: fmt = %s, buffer offset = %#" PRIx64 ", image = %p",
         prefix,
         gfx_lfmt_desc(image->LFMT()),
         region.bufferOffset,
         image);
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
   return true;
}

JobID Image::CopyOneBufferRegion(
      Buffer                  *pSrcBuf,
      const VkBufferImageCopy &region,
      const SchedDependencies &deps)
{
   log_trace_enabled() && LogCopyBufferRegion(this, region, "CopyFromBuffer");

   const bool dstIs1D           = gfx_lfmt_is_1d(LFMT());
   const bool dstIs3D           = gfx_lfmt_is_3d(LFMT());
   // We cannot trust the height in the extent or the y offset will be a
   // valid value for 1D images. There is no need to sanitize depth and Z as
   // their use is guarded by dstIs3D.
   const uint32_t extentHeight  = dstIs1D ? 1 : region.imageExtent.height;
   const int32_t  offsetY       = dstIs1D ? 0 : region.imageOffset.y;

   VkDeviceSize srcMemoryOffset = pSrcBuf->GetBoundMemoryOffset();
   DeviceMemory *srcMemory      = pSrcBuf->GetBoundMemory();
   assert(srcMemory != nullptr);

   GFX_BUFFER_DESC_T dstDesc = GetDescriptor(region.imageSubresource.mipLevel);
   assert(dstDesc.num_planes == 1); // No planar formats yet

   // Start with the lfmt of the destination image and modify it for the
   // correct interpretation of the buffer data.
   GFX_LFMT_T srcLFMT = LFMT();
   AdjustBufferFormatForCopy(region.imageSubresource.aspectMask, &srcLFMT, /* isSrc */ true);

   uint32_t layerCount, dstArraySize;
   if (dstIs3D)
   {
      assert(region.imageSubresource.layerCount == 1);
      assert(region.imageSubresource.baseArrayLayer == 0);

      // Turn 3D slices into array layers for imgconv TFU path
      layerCount   = region.imageExtent.depth;
      dstArraySize = dstDesc.planes[0].slice_pitch;

      Adjust3DDescTo2DSlice(&dstDesc, region.imageOffset.z);
      gfx_lfmt_set_dims(&srcLFMT, GFX_LFMT_DIMS_2D);
   }
   else
   {
      layerCount   = region.imageSubresource.layerCount;
      dstArraySize = m_arrayStride;
   }

   // Generate a buffer desc for the source buffer
   const uint32_t w = (region.bufferRowLength > 0) ? region.bufferRowLength :
                                                     region.imageExtent.width;
   const uint32_t h = (region.bufferImageHeight > 0) ? region.bufferImageHeight :
                                                       extentHeight;
   GFX_BUFFER_DESC_T srcDesc;
   size_t srcArraySize, align;

   gfx_buffer_desc_gen(&srcDesc, &srcArraySize, &align, GFX_BUFFER_USAGE_NONE,
                       w, h, 1, 1, 1, &srcLFMT);

   // Fixup the offset into the memory block
   srcDesc.planes[0].offset = static_cast<uint32_t>(srcMemoryOffset + region.bufferOffset);

   // Create src and dest imgconv targets and do the copy
   struct v3d_imgconv_gmem_tgt srcTarget;
   v3d_imgconv_init_gmem_tgt(&srcTarget, srcMemory->Handle(), 0, &deps, &srcDesc,
                             0, 0, 0, 0, static_cast<unsigned int>(srcArraySize));

   AllocateLazyMemory();

   struct v3d_imgconv_gmem_tgt dstTarget;
   v3d_imgconv_init_gmem_tgt(&dstTarget, m_boundMemory->Handle(),
                             LayerOffset(region.imageSubresource.baseArrayLayer),
                             &deps, &dstDesc,
                             static_cast<unsigned int>(region.imageOffset.x),
                             static_cast<unsigned int>(offsetY),
                             0, 0, static_cast<unsigned int>(dstArraySize));

   dstTarget.base.conversion =
      CopyAspectToImgconvOp(region.imageSubresource.aspectMask, srcLFMT, dstDesc.planes[0].lfmt);

   JobID jobId = 0;
   bool ret = v3d_imgconv_convert(&dstTarget, &srcTarget, &jobId,
                                  region.imageExtent.width, extentHeight, /* depth */ 1,
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
      if (IsValidBufferCopyAspect(regions[i].imageSubresource.aspectMask, m_mipDescs[0].planes[0].lfmt))
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

   const bool srcIs1D = gfx_lfmt_is_1d(LFMT());
   const bool srcIs3D = gfx_lfmt_is_3d(LFMT());

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
      const VkBufferImageCopy &region = regions[i];
      log_trace_enabled() && LogCopyBufferRegion(this, region, "CopyToBuffer");

      if (!IsValidBufferCopyAspect(region.imageSubresource.aspectMask, LFMT()))
         continue;

      // We cannot trust the height and depth in the extent will contain valid
      // values if the image type doesn't need them.
      const uint32_t extentHeight = srcIs1D ? 1 : region.imageExtent.height;
      const uint32_t extentDepth  = srcIs3D ? region.imageExtent.depth : 1;

      GFX_BUFFER_DESC_T srcDesc = GetDescriptor(region.imageSubresource.mipLevel);
      assert(srcDesc.num_planes == 1); // No planar formats yet

      if (srcIs3D)
      {
         assert(region.imageSubresource.layerCount == 1);
         assert(region.imageSubresource.baseArrayLayer == 0);
      }

      GFX_LFMT_T dstLFMT = LFMT();
      AdjustBufferFormatForCopy(region.imageSubresource.aspectMask, &dstLFMT, /* isSrc */ false);

      // Generate the buffer desc for the destination buffer
      GFX_BUFFER_DESC_T dstDesc;
      size_t size, align;

      const uint32_t w = (region.bufferRowLength > 0) ? region.bufferRowLength :
                                                        region.imageExtent.width;
      const uint32_t h = (region.bufferImageHeight > 0) ? region.bufferImageHeight :
                                                          extentHeight;

      gfx_buffer_desc_gen(&dstDesc, &size, &align, GFX_BUFFER_USAGE_NONE,
                          w, h, extentDepth, 1, 1, &dstLFMT);

      // Fixup the offset into the memory block
      dstDesc.planes[0].offset = static_cast<uint32_t>(dstMemoryOffset + region.bufferOffset);

      // Create imgconv source and destination targets and do the copy
      struct v3d_imgconv_gmem_tgt srcTarget;
      const uint32_t srcImgOffset = LayerOffset(region.imageSubresource.baseArrayLayer);
      this->InitGMemTarget(&srcTarget, deps, srcDesc, srcImgOffset, region.imageOffset);

      struct v3d_imgconv_gmem_tgt dstTarget;
      v3d_imgconv_init_gmem_tgt(&dstTarget, dstMemory->Handle(), 0,
                                &deps, &dstDesc, 0, 0, 0, 0, size);

      JobID jobId = 0;
      bool ret = v3d_imgconv_convert(&dstTarget, &srcTarget, &jobId,
                                  region.imageExtent.width,
                                  extentHeight,
                                  extentDepth,
                                  region.imageSubresource.layerCount,
                                  false /* TODO: secure_context */);

      assert(ret);

      if (Options::dumpTransferImages)
      {
         // Note: only the first array layer supported at the moment
         std::ostringstream details;

         WriteImageDetails(details, "src",
            this,
            region.imageSubresource.mipLevel,
            srcIs3D ? region.imageOffset.z : region.imageSubresource.baseArrayLayer);

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
