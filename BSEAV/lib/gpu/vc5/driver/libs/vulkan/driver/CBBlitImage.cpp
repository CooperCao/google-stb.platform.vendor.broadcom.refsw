/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Common.h"
#include "CommandBuffer.h"
#include "Command.h"
#include "ColorAspectCommandBuilder.h"
#include "ArenaAllocator.h"
#include "DevMemDataBlock.h"

#include "libs/core/v3d/v3d_shadrec.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_tmu.h"

#include "libs/util/log/log.h"

#include <cstring>
#include <cfloat>

namespace bvk {

LOG_DEFAULT_CAT("bvk::CommandBuffer");

#include "libs/vulkan/nvshaders/vk_blit_shaders.h"

static inline const inline_qasm *GetBlitShader(bool use32, bool signedClamp, uint32_t srcDim)
{
   if (!signedClamp)
   {
      const int writeIdx = use32;
      static const inline_qasm *shader[2][3] = {
         { &blit_f16tlb_1D, &blit_f16tlb_2D, &blit_f16tlb_3D },
         { &blit_f32tlb_1D, &blit_f32tlb_2D, &blit_f32tlb_3D }
      };

      return shader[writeIdx][srcDim-1];
   }
   else
   {
      assert(use32);
      static const inline_qasm *shader[3] = { &blit_s32tlb_1D, &blit_s32tlb_2D, &blit_s32tlb_3D };

      return shader[srcDim-1];
   }
}

struct TexCoord
{
   TexCoord() {}
   TexCoord(float x, float y, float z): u {x}, v {y}, w {z} {}
   TexCoord(int32_t x, int32_t y, int32_t z, const TexCoord &adjustment)
   {
      u = static_cast<float>(x) + adjustment.u;
      v = static_cast<float>(y) + adjustment.v;
      w = static_cast<float>(z) + adjustment.w;
   }

   float u = 0.0f;
   float v = 0.0f;
   float w = 0.0f;
};

using DevMemArena = ArenaAllocator<DevMemDataBlock, DevMemRange>;

class BlitNVShaderHelper
{
public:
   BlitNVShaderHelper(DevMemArena *devDataArena, ControlListBuilder::ControlList *cl, uint32_t dims):
      m_devDataArena {devDataArena}, m_cl {cl}, m_nVaryings {dims}, m_tcoordStride(dims * sizeof(uint32_t))
   {}

   uint32_t NumAttributeArrays() const { return m_nAttrArrays; }

   void SamplerParameter(VkFilter filter, bool use32Bit)
   {
      V3D_TMU_SAMPLER_T sampler;
      std::memset(&sampler, 0, sizeof(sampler));

      if (filter == VK_FILTER_LINEAR)
      {
         sampler.magfilt = V3D_TMU_FILTER_LINEAR;
         sampler.minfilt = V3D_TMU_FILTER_LINEAR;
      }
      else
      {
         sampler.magfilt = V3D_TMU_FILTER_NEAREST;
         sampler.minfilt = V3D_TMU_FILTER_NEAREST;
      }
      sampler.mipfilt = V3D_TMU_MIPFILT_NEAREST;
      sampler.wrap_s = V3D_TMU_WRAP_CLAMP;
      sampler.wrap_t = V3D_TMU_WRAP_CLAMP;
      sampler.wrap_r = V3D_TMU_WRAP_CLAMP;

      NewDevMemRange(&m_samplerMem, V3D_TMU_SAMPLER_PACKED_SIZE, V3D_TMU_SAMPLER_ALIGN);
      v3d_pack_tmu_sampler(static_cast<uint8_t*>(m_samplerMem.Ptr()), &sampler);

      V3D_TMU_PARAM1_T p1;
      p1.sampler_addr = m_samplerMem.Phys();
      p1.unnorm       = true;
      p1.output_32    = use32Bit;
      p1.pix_mask     = false;
      m_tmuCfg[1]     = v3d_pack_tmu_param1(&p1);
   }

   void TextureParameter(
         bvk::Image *img,
         uint32_t    mipLevel,
         uint32_t    arrayLayer,
         bool        use32BitPrecision)
   {
      const auto &imgDesc = img->GetDescriptor(mipLevel);
      const GFX_LFMT_DIMS_T dims = img->GetImageDims();

      GFX_LFMT_TMU_TRANSLATION_T t;

      gfx_lfmt_translate_tmu(&t, img->LFMT()
#if !V3D_HAS_TMU_R32F_R16_SHAD
         , /*need_depth_type=*/false
#endif
         );

      gfx_buffer_uif_cfg uifCfg;
      gfx_buffer_get_tmu_uif_cfg(&uifCfg, &imgDesc, 0);
      assert(!uifCfg.ub_noutile);

      V3D_TMU_TEX_STATE_T tsr;
      std::memset(&tsr, 0, sizeof(tsr));

      tsr.l0_addr = img->PhysAddr() + imgDesc.planes[0].offset + img->LayerOffset(arrayLayer);

      if (dims == GFX_LFMT_DIMS_1D)
         v3d_tmu_get_wh_for_1d_tex_state(&tsr.width, &tsr.height, imgDesc.width);
      else
      {
         tsr.width  = imgDesc.width;
         tsr.height = imgDesc.height;
      }

      tsr.depth = imgDesc.depth;

      if (dims == GFX_LFMT_DIMS_3D)
         tsr.arr_str = imgDesc.planes[0].slice_pitch;

      tsr.type  = t.type;
      tsr.srgb  = t.srgb;

      std::memcpy(&tsr.swizzles, &t.swizzles, sizeof(t.swizzles));

      tsr.extended = uifCfg.force;

      if (tsr.extended)
         NewDevMemRange(&m_tsrMem, V3D_TMU_TEX_STATE_PACKED_SIZE + V3D_TMU_TEX_EXTENSION_PACKED_SIZE , V3D_TMU_EXTENDED_TEX_STATE_ALIGN);
      else
         NewDevMemRange(&m_tsrMem, V3D_TMU_TEX_STATE_PACKED_SIZE, V3D_TMU_TEX_STATE_ALIGN);

      auto ptr = static_cast<uint8_t*>(m_tsrMem.Ptr());

      v3d_pack_tmu_tex_state(ptr, &tsr);

      if (tsr.extended)
      {
         V3D_TMU_TEX_EXTENSION_T e;
         e.ub_pad  = uifCfg.ub_pads[0];
         e.ub_xor  = uifCfg.ub_xor;
         e.uif_top = uifCfg.force;
         e.xor_dis = uifCfg.xor_dis;
         v3d_pack_tmu_tex_extension(ptr + V3D_TMU_TEX_STATE_PACKED_SIZE, &e);
      }

      V3D_TMU_PARAM0_T p0;
      p0.tex_state_addr = m_tsrMem.Phys();
      p0.word_en[0] = true;
      p0.word_en[1] = true;
      p0.word_en[2] = use32BitPrecision;
      p0.word_en[3] = use32BitPrecision;
      m_tmuCfg[0]   = v3d_pack_tmu_param0(&p0);
   }

   void Uniforms(uint32_t tlbConfig, uint32_t clampVal1, uint32_t clampVal2, bool signedClamp)
   {
      m_nUniforms = 5;
      NewDevMemRange(&m_uniformMem, m_nUniforms * sizeof(uint32_t), V3D_SHADREC_ALIGN);
      uint32_t *unifPtr = static_cast<uint32_t*>(m_uniformMem.Ptr());

      uint32_t idx = 0;
      unifPtr[idx++] = m_tmuCfg[0];
      unifPtr[idx++] = m_tmuCfg[1];
      unifPtr[idx++] = clampVal1;
      if (signedClamp)
         unifPtr[idx++] = clampVal2;

      unifPtr[idx++] = tlbConfig;

      if (!signedClamp)
         unifPtr[idx++] = clampVal2;
   }

   void Uniforms(uint32_t tlbConfig)
   {
      m_nUniforms = 3;
      NewDevMemRange(&m_uniformMem, m_nUniforms * sizeof(uint32_t), V3D_SHADREC_ALIGN);
      uint32_t *unifPtr = static_cast<uint32_t*>(m_uniformMem.Ptr());

      uint32_t idx = 0;
      unifPtr[idx++] = m_tmuCfg[0];
      unifPtr[idx++] = m_tmuCfg[1];
      unifPtr[idx++] = tlbConfig;
   }

   void VertexData(const VkOffset3D *dstOffsets, const TexCoord *coords)
   {
      m_vdataMaxIndex = 3;
      NewDevMemRange(&m_vdataMem, m_vdataStride * (m_vdataMaxIndex + 1), V3D_SHADREC_ALIGN);
      uint32_t *vdataPtr = static_cast<uint32_t*>(m_vdataMem.Ptr());

      PackOneVertex(&vdataPtr[0],  dstOffsets[0].x, dstOffsets[0].y);
      PackOneVertex(&vdataPtr[4],  dstOffsets[1].x, dstOffsets[0].y);
      PackOneVertex(&vdataPtr[8],  dstOffsets[1].x, dstOffsets[1].y);
      PackOneVertex(&vdataPtr[12], dstOffsets[0].x, dstOffsets[1].y);

      m_tcoordMaxIndex = 3;
      NewDevMemRange(&m_tcoordMem, m_tcoordStride * (m_tcoordMaxIndex + 1), V3D_SHADREC_ALIGN);
      uint32_t *tcoordPtr = static_cast<uint32_t*>(m_tcoordMem.Ptr());

      PackOneTCoord(&tcoordPtr[0],               coords[0]);
      PackOneTCoord(&tcoordPtr[m_nVaryings],     coords[1]);
      PackOneTCoord(&tcoordPtr[m_nVaryings * 2], coords[2]);
      PackOneTCoord(&tcoordPtr[m_nVaryings * 3], coords[3]);
   }

   void ShaderRecord(const inline_qasm *shader)
   {
      // Install the shader
      DevMemRange fshaderMem;

      NewDevMemRange(&fshaderMem, shader->size * sizeof(uint64_t), V3D_QPU_INSTR_ALIGN);
      std::memcpy(fshaderMem.Ptr(), shader->code, shader->size * sizeof(uint64_t));

      // Allocate the device memory for the shader record
      const uint32_t srSize = V3D_SHADREC_GL_MAIN_PACKED_SIZE +
                              m_nAttrArrays * V3D_SHADREC_GL_ATTR_PACKED_SIZE;

      NewDevMemRange(&m_shadRecMem, srSize, V3D_SHADREC_ALIGN);

      uint8_t *shadRecPtr = static_cast<uint8_t*>(m_shadRecMem.Ptr());
      std::memset(shadRecPtr, 0, srSize);

      // Almost everything in an NV shader record is false or 0 in our current
      // usage.
      V3D_SHADREC_GL_MAIN_T shaderRec = { false };
      V3D_SHADREC_GL_ATTR_T attr[3];

      shaderRec.num_varys = m_nVaryings;

      // Coord and IO VPM segment, measured in units of 8 words
      //
      // - Coordinate shader needs at most 11 words, the clip header,
      //   the vertex coords and the texture coords.
      // - Vertex shader needs at most 7 words, the vertex coords and the
      //   texture coords
      shaderRec.cs_output_size = V3D_OUT_SEG_ARGS_T { 2, 0 };
      shaderRec.cs_input_size  = V3D_IN_SEG_ARGS_T  { /*.sectors = */0, /*.min_req = */1 };
      shaderRec.vs_output_size = V3D_OUT_SEG_ARGS_T { 1, 0 };
      shaderRec.vs_input_size  = V3D_IN_SEG_ARGS_T  { /*.sectors = */0, /*.min_req = */1 };
      shaderRec.fs.threading   = V3D_THREADING_4;
      shaderRec.fs.addr        = fshaderMem.Phys();
      shaderRec.fs.unifs_addr  = m_uniformMem.Phys();
      shaderRec.fs.single_seg  = false;

      // Xc, Yc, Zc, Wc - clipping is disabled so we just replicate the following attribute.
      MakeShaderAttr(&attr[0], 4, 4, 0, m_vdataMem.Phys(), m_vdataStride, m_vdataMaxIndex);

      // Xs, Ys, Zs, 1/Wc
      MakeShaderAttr(&attr[1], 4, 4, 4, m_vdataMem.Phys(), m_vdataStride, m_vdataMaxIndex);

      // 2D or 3D texture coordinates
      MakeShaderAttr(&attr[2], m_nVaryings, m_nVaryings, m_nVaryings,
            m_tcoordMem.Phys(), m_tcoordStride, m_tcoordMaxIndex);

      // This allocation contains both main GL shader record,
      // followed by variable number of attribute records.
      uint8_t *attrs_packed = shadRecPtr + V3D_SHADREC_GL_MAIN_PACKED_SIZE;
      for (uint32_t i = 0; i < m_nAttrArrays; i++)
         v3d_pack_shadrec_gl_attr(reinterpret_cast<uint32_t *>(attrs_packed + i * V3D_SHADREC_GL_ATTR_PACKED_SIZE),
                                  &attr[i]);

      v3d_pack_shadrec_gl_main(reinterpret_cast<uint32_t *>(shadRecPtr), &shaderRec);
   }

   v3d_addr_t ShaderRecPhys() { return m_shadRecMem.Phys(); }

private:
   void NewDevMemRange(DevMemRange *range, size_t size, size_t align)
   {
      m_devDataArena->Allocate(range, size, align);
   }

   void PackOneVertex(uint32_t *addr, uint32_t x, uint32_t y)
   {
      const uint32_t one = gfx_float_to_bits(1.0f);
      addr[0] = x << 8; // Xs in 24.8 fixed point
      addr[1] = y << 8; // Ys in 24.8 fixed point
      addr[2] = one;    // Zs in 32bit float (dummy value)
      addr[3] = one;    // 1/Wc in 32bit float (dummy value)

      log_trace("\tVertex: [%u,%u] -> [%#x,%#x]", x, y, addr[0], addr[1]);
   }

   void PackOneTCoord(uint32_t *addr, const TexCoord &coord)
   {
      int i = 0;
      if (m_nVaryings > 1) addr[i++] = gfx_float_to_bits(coord.v);
      if (m_nVaryings > 2) addr[i++] = gfx_float_to_bits(coord.w);

      addr[i] = gfx_float_to_bits(coord.u);

      log_trace("\tTCoord: [%f,%f,%f] -> [%#x,%#x,%#x]",
            coord.u, coord.v, coord.w, addr[0], addr[1], addr[2]);
   }

   void MakeShaderAttr(
         V3D_SHADREC_GL_ATTR_T *attr,
         int size, int csReads, int vsReads, v3d_addr_t addr, int stride, uint32_t max_index)
   {
      std::memset(attr, 0, sizeof(*attr));
      attr->addr         = addr;
      attr->size         = size;
      attr->type         = V3D_ATTR_TYPE_FLOAT;
      attr->cs_num_reads = csReads;
      attr->vs_num_reads = vsReads;
      attr->stride       = stride;
      attr->max_index    = max_index;
   }

   DevMemArena                     *m_devDataArena;
   ControlListBuilder::ControlList *m_cl;

   DevMemRange                      m_tsrMem;
   DevMemRange                      m_samplerMem;
   uint32_t                         m_tmuCfg[2];

   DevMemRange                      m_uniformMem;
   uint32_t                         m_nUniforms;

   DevMemRange                      m_vdataMem;
   const uint32_t                   m_vdataStride = 16;
   uint32_t                         m_vdataMaxIndex;

   DevMemRange                      m_tcoordMem;
   const uint32_t                   m_nVaryings;
   const uint32_t                   m_tcoordStride;
   uint32_t                         m_tcoordMaxIndex;

   DevMemRange                      m_shadRecMem;
   const uint32_t                   m_nAttrArrays = 3;
};

///////////////////////////////////////////////////////////////////////////////
// Inline helpers for BlitImageRegion

static inline void GetClampValues(
   GFX_LFMT_T format,
   uint32_t *clampVal1, uint32_t *clampVal2, bool *signedClamp)
{
   // Determine the unsigned integer clamping value to be applied to each
   // output component in the F(I)32 and S32 shaders.
   if (gfx_lfmt_has_color(format) && gfx_lfmt_contains_int(format))
   {
      const uint32_t redBits = gfx_lfmt_red_bits(format);
      const uint32_t alphaBits = gfx_lfmt_alpha_bits(format);

      if (gfx_lfmt_contains_int_signed(format))
      {
         *clampVal1 = gfx_mask(redBits-1); // max +ve value
         *clampVal2 = ~(*clampVal1);       // max -ve value
         *signedClamp = true;
      }
      else
      {
         *clampVal1 = gfx_mask(redBits);
         *clampVal2 = gfx_mask(alphaBits); // For VK_FORMAT_A2B10G10R10_UINT_PACK32
         *signedClamp = false;
      }
   }
   else
   {
      *clampVal1 = 0xffffffff;
      *clampVal2 = 0xffffffff;
      *signedClamp = false;
   }
}

static inline bool IsEmptyRegion(const VkOffset3D offsets[2])
{
   return offsets[0].x == offsets[1].x
       || offsets[0].y == offsets[1].y
       || offsets[0].z == offsets[1].z;
}

static inline void SanitizeDestinationRegion(
      bvk::Image      *dstImage,
      const VkOffset3D dstOffsets[2],
      VkOffset3D       updatedOffsets[2])
{
   // We have to have a valid destination y-range [0,1] for 1D images to
   // correctly set up the quad we are drawing and the render target
   // parameters. It also helps to make sure there is a valid z-range
   // for 2D destinations when copying from a 3D source.
   std::memcpy(updatedOffsets, dstOffsets, sizeof(VkOffset3D)*2);

   GFX_LFMT_DIMS_T dims = dstImage->GetImageDims();
   if (dims == GFX_LFMT_DIMS_1D)
   {
      updatedOffsets[0].y = 0;
      updatedOffsets[1].y = 1;
   }

   if (dims != GFX_LFMT_DIMS_3D)
   {
      updatedOffsets[0].z = 0;
      updatedOffsets[1].z = 1;
   }
}

static inline TexCoord TexCoordAdjustment(
      Image    *srcImage,
      VkFilter  filter)
{
   // vkCmdBlitImage cannot use a multi-sampled source, but this code is
   // also used by VkCmdResolveImage when we cannot use the TLB directly.
   if (srcImage->Samples() != VK_SAMPLE_COUNT_1_BIT)
   {
      if (filter == VK_FILTER_NEAREST)
      {
         // Get the top left sample instead of bottom right which is what
         // you would normally get with the standard setup and a downscale by 2
         return TexCoord {-0.5, -0.5f, 0.0 };
      }
   }
   else
   {
      // Previously this path was used to add a small offset in order to pass
      // broken dEQP blit tests with linear sampling. That is no longer
      // required as the tests have now been fixed, but if something similar
      // is needed in the future put it here.
   }

   return TexCoord { 0.0, 0.0, 0.0 };
}

static inline void CreateTexCoords(
      const TexCoord   &adjustment,
      const VkOffset3D *srcOffsets,
      const VkOffset3D *dstOffsets,
      TexCoord          coords[4])
{
   coords[0] = TexCoord {srcOffsets[0].x, srcOffsets[0].y, srcOffsets[0].z, adjustment};
   coords[1] = TexCoord {srcOffsets[1].x, srcOffsets[0].y, srcOffsets[0].z, adjustment};
   coords[2] = TexCoord {srcOffsets[1].x, srcOffsets[1].y, srcOffsets[0].z, adjustment};
   coords[3] = TexCoord {srcOffsets[0].x, srcOffsets[1].y, srcOffsets[0].z, adjustment};
}

// Support for updating the w coord for 3D slices. The scale and coordinate
// maths are taken directly from the spec.
static inline float CalculateScaleW(
      const VkOffset3D srcOffsets[2],
      const VkOffset3D dstOffsets[2])
{
   // Note: the destination range has already been checked and sanitized
   //       so we will not have zero destination coverage specified here.
   return static_cast<float>(srcOffsets[1].z - srcOffsets[0].z) /
          static_cast<float>(dstOffsets[1].z - dstOffsets[0].z);
}

static inline void UpdateWTexCoordFor3DSlice(
      int32_t           k,
      float             scalew,
      const TexCoord   &adjustment,
      const VkOffset3D &srcOffset,
      const VkOffset3D &dstOffset,
      TexCoord         coords[4])
{
   // It appears we have to follow the maths in the spec here and add the 0.5
   // ourselves in order to get the correct sample point for the slice.
   float w = ((static_cast<float>(k - dstOffset.z) + 0.5f) * scalew) + srcOffset.z + adjustment.w;
   coords[0].w = coords[1].w = coords[2].w = coords[3].w = w;
}

void CommandBuffer::BlitImageRegion(
   bvk::Image        *srcImage,
   bvk::Image        *dstImage,
   const VkImageBlit &region,
   VkFilter           filter)
{
   const bool dstIs3D = gfx_lfmt_is_3d(dstImage->LFMT());
   const bool srcIs3D = gfx_lfmt_is_3d(srcImage->LFMT());
   const uint32_t srcMipLevel = region.srcSubresource.mipLevel;
   const uint32_t dstMipLevel = region.dstSubresource.mipLevel;

   const auto &srcOffsets = region.srcOffsets;

   VkOffset3D dstOffsets[2];
   SanitizeDestinationRegion(dstImage, region.dstOffsets, dstOffsets);
   // The spec doesn't say that specifying a destination with no coverage
   // is invalid usage, so if we get one just ignore it otherwise the
   // TLB setup will assert.
   if (IsEmptyRegion(dstOffsets))
      return;

   float scalew = CalculateScaleW(srcOffsets, dstOffsets);
   TexCoord adjustment = TexCoordAdjustment(srcImage, filter);
   TexCoord coords[4];

   CreateTexCoords(adjustment, srcOffsets, dstOffsets, coords);

   GFX_LFMT_T tlbLFMT = dstImage->LFMT();

   uint32_t clampVal1,clampVal2;
   bool signedClamp;
   GetClampValues(tlbLFMT, &clampVal1, &clampVal2, &signedClamp);

   const int32_t layerCount  =
         dstIs3D ? std::abs(dstOffsets[1].z - dstOffsets[0].z) : region.dstSubresource.layerCount;

   const int32_t layerStep = dstIs3D && (dstOffsets[1].z < dstOffsets[0].z) ? -1 : 1;

   int32_t dstBaseLayer;
   // For 3D destination images we draw into a slice at a time and move the
   // source z texture coordinate appropriately using the scale.
   if (dstIs3D)
   {
      dstBaseLayer = (dstOffsets[1].z < dstOffsets[0].z) ? region.dstOffsets[0].z - 1 : region.dstOffsets[0].z;
   }
   else
   {
      dstBaseLayer = region.dstSubresource.baseArrayLayer;
      // If copying from a 3D image to a 2D source then calculate the source
      // sample point in the w coordinate based on a destination "slice 0". This
      // will give us correct linear filtering between the 3D source slices.
      if (srcIs3D)
         UpdateWTexCoordFor3DSlice(0, scalew, adjustment, srcOffsets[0], dstOffsets[0], coords);
   }

   for (int32_t l = 0; l < layerCount; l++)
   {
      ColorAspectCommandBuilder cb {GetCallbacks(), this};

      // Force source base layer to 0 if the destination is 3D, it must be 0
      // in the subresource and there can only be one array layer copied
      // if the source is 3D and the destination is a 2D array, otherwise it
      // is invalid usage.
      uint32_t srcBaseLayer = dstIs3D ? 0 : region.srcSubresource.baseArrayLayer + l;

      log_trace("\tBlitNVShader bin/render job for:");
      log_trace("\tsrcMipLevel  = %u dstMipLevel = %u", srcMipLevel, dstMipLevel);
      log_trace("\tsrcBaseLayer = %u dstBaseLayer/slice = %d", srcBaseLayer, dstBaseLayer);
      log_trace("\tfilter       = %s", filter == VK_FILTER_NEAREST ?  "nearest" : "linear");

      if (dstIs3D)
         UpdateWTexCoordFor3DSlice(dstBaseLayer, scalew, adjustment, srcOffsets[0], dstOffsets[0], coords);

      if (dstOffsets[0].x != 0 || dstOffsets[0].y != 0)
         cb.SetLoadDestination(); // We need to preserve what we are not drawing to

      cb.SetImageSubresources(nullptr, dstImage, tlbLFMT,
            0, dstMipLevel, 0, dstBaseLayer, /* layerCount = */ 1,
            std::max(dstOffsets[0].x, dstOffsets[1].x),
            std::max(dstOffsets[0].y, dstOffsets[1].y));

      ControlListBuilder::ControlList drawList;

      cb.SetCurrentControlList(&drawList);
      drawList.SetStart(*cb.m_curDeviceBlock);

      uint32_t srcDim = gfx_lfmt_dims_from_enum(srcImage->GetImageDims());
      BlitNVShaderHelper nv { &m_devDataArena, &drawList, srcDim };

      /* TODO: The !IsInt is to force the old behaviour on new hardware until shaders, etc. catch up */
      const bool use16  = cb.TLBUseWRCfg16() && !cb.TLBIsInt();
      const bool useInt = cb.TLBIsInt();

      const inline_qasm *shader = GetBlitShader(!use16, signedClamp, srcDim);

      nv.TextureParameter(srcImage, srcMipLevel, srcBaseLayer, !use16);
      nv.SamplerParameter(filter, !use16);
      nv.VertexData(dstOffsets, coords);

      uint32_t tlbConfig = 0xffffff00 | v3d_tlb_config_color(0, use16, useInt, use16 ? 2 : 4, false);

      if (use16)
         nv.Uniforms(tlbConfig);
      else
         nv.Uniforms(tlbConfig, clampVal1, clampVal2, signedClamp);

      nv.ShaderRecord(shader);

      v3d_cl_nv_shader(cb.CLPtr(), nv.NumAttributeArrays(), nv.ShaderRecPhys());
      v3d_cl_vertex_array_prims(cb.CLPtr(), V3D_PRIM_MODE_TRI_FAN, 4, 0);
      v3d_cl_return(cb.CLPtr());

      drawList.SetEnd(*cb.m_curDeviceBlock);
      cb.SetCurrentControlList(nullptr);
      cb.SetDrawList(&drawList);

      auto cmd = NewObject<CmdBinRenderJobObj>(m_device->GetPhysicalDevice());
      const v3d_barrier_flags binSyncFlags = V3D_BARRIER_NO_ACCESS; // Control list builder defaults are OK for blit binning
      const v3d_barrier_flags renderSyncFlags = V3D_BARRIER_TMU_CONFIG_READ | V3D_BARRIER_TMU_DATA_READ;

      cb.CreateMasterControlLists(cmd, binSyncFlags, renderSyncFlags);

      m_commandList.push_back(cmd);
      dstBaseLayer += layerStep;
   }
}


class CmdReleaseBounceImageObj : public CPUCommand
{
public:
   void Execute() const
   {
      destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(m_image, nullptr);
      destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(m_memory, nullptr);
   }

   Image        *m_image  = nullptr;
   DeviceMemory *m_memory = nullptr;
};

static void CreateBounceImage(const Image *img, Device *dev, CmdReleaseBounceImageObj *bounce)
{
   VkImageCreateInfo info = {
      /* .sType =                 */ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      /* .pNext =                 */ nullptr,
      /* .flags =                 */ 0,
      /* .imageType =             */ img->GetImageDims() == GFX_LFMT_DIMS_2D ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D,
      /* .format =                */ img->Format(),
      /* .extent =                */ img->Extent(),
      /* .mipLevels =             */ img->MipLevels(),
      /* .arrayLayers =           */ img->ArrayLayers(),
      /* .samples =               */ img->Samples(),
      /* .tiling =                */ VK_IMAGE_TILING_OPTIMAL,
      /* .usage =                 */ VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                     VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      /* .sharingMode =           */ VK_SHARING_MODE_EXCLUSIVE,
      /* .queueFamilyIndexCount = */ 0,
      /* .pQueueFamilyIndices =   */ nullptr,
      /* .initialLayout =         */ VK_IMAGE_LAYOUT_UNDEFINED,
   };

   bounce->m_image = bvk::createObject<Image, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      nullptr, dev->GetCallbacks(), dev, &info);

   VkMemoryRequirements memReq;
   bounce->m_image->GetImageMemoryRequirements(dev, &memReq);

   VkMemoryAllocateInfo alloc = {
         /* .sType =           */ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
         /* .pNext =           */ nullptr,
         /* .allocationSize =  */ memReq.size,
         /* .memoryTypeIndex = */ 0,
   };

   // If this fails the image creation will get cleaned up as part of the
   // bounce object destruction.
   bounce->m_memory = createObject<DeviceMemory, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE>(
                                       nullptr, dev->GetCallbacks(), dev, &alloc);

   bounce->m_image->BindImageMemory(dev, bounce->m_memory, 0);
}

void CommandBuffer::CmdBlitImage(
   bvk::Image        *srcImage,
   VkImageLayout      srcImageLayout,
   bvk::Image        *dstImage,
   VkImageLayout      dstImageLayout,
   uint32_t           regionCount,
   const VkImageBlit *pRegions,
   VkFilter           filter) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(!InRenderPass());

   // Blitting MS images is not allowed by the spec
   assert(srcImage->Samples() == VK_SAMPLE_COUNT_1_BIT);
   assert(dstImage->Samples() == VK_SAMPLE_COUNT_1_BIT);

   log_trace("CmdBlitImage: srcImage = %p dstImage = %p", srcImage, dstImage);

   CmdReleaseBounceImageObj *bounceReleaseCmd = nullptr;

   try
   {
      // We can hit this case for example when using blit to take a snapshot
      // from a swapchain image, which is in RSO for efficient display interop.
      GFX_LFMT_T tmuLFMT = srcImage->LFMT();
      if (!gfx_lfmt_is_1d(tmuLFMT) && gfx_lfmt_is_rso(tmuLFMT))
      {
         // We should never have a 2D or 3D mipmapped image in an RSO layout.
         assert(srcImage->MipLevels() == 1);

         // Create a new image with memory bound to it to bounce a UIF version
         // of the image into (using the TFU). This image currently lives for
         // the lifetime of the command buffer as the physical address is
         // required to construct the blit bin/render job command.
         bounceReleaseCmd = NewObject<CmdReleaseBounceImageObj>();
         CreateBounceImage(srcImage, m_device, bounceReleaseCmd);

         VkImageCopy region = {
            /* .srcSubresources = */ { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, srcImage->ArrayLayers() },
            /* .srcOffset =       */ { 0, 0, 0 },
            /* .dstSubresources = */ { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, srcImage->ArrayLayers() },
            /* .dstOffset =       */ { 0, 0, 0 },
            /* .extent =          */ srcImage->Extent()
         };

         // Insert an image copy and barrier into the command buffer and then
         // switch to using the bounce buffer as the blit source.
         //
         // NOTE: if you are getting confused, we cannot actually issue the TFU
         //       job right now; this command buffer isn't being executed yet,
         //       we are just recording the actions that are needed. So we are
         //       leveraging the transfer API copy command code to help us out.
         CmdCopyImage(srcImage, srcImageLayout, bounceReleaseCmd->m_image, VK_IMAGE_LAYOUT_GENERAL, 1, &region);
         InsertExecutionBarrier();
         srcImage = bounceReleaseCmd->m_image;
      }

      for (uint32_t i = 0; i < regionCount; i++)
      {
         assert(pRegions[i].srcSubresource.aspectMask == pRegions[i].dstSubresource.aspectMask);
         assert(pRegions[i].srcSubresource.layerCount == pRegions[i].dstSubresource.layerCount);
         if (gfx_lfmt_is_3d(dstImage->LFMT()))
         {
            assert(pRegions[i].dstSubresource.layerCount == 1);
            assert(pRegions[i].dstSubresource.baseArrayLayer == 0);
            // The spec does not allow blitting from a non-zero 1D or 2D array
            // layer into a 3D slice
            assert(pRegions[i].srcSubresource.baseArrayLayer == 0);
         }

         // We need to add our own execution barriers between regions as each
         // job may be writing to portions of the same memory. On the multi-core
         // simulator this will cause rendering errors as tile writes fight
         // against each other.
         if (i != 0)
            InsertExecutionBarrier();

         BlitImageRegion(srcImage, dstImage, pRegions[i], filter);
      }

      if (bounceReleaseCmd)
      {
         // We have to ensure that a second execution of this command
         // buffer does not reuse the bounce image before a blit from it
         // has completed.
         InsertExecutionBarrier();
         m_cleanupCommandList.push_back(bounceReleaseCmd);
      }
   }
   catch (...)
   {
      // something went wrong, if we created a bounce image we need to
      // release it and the bound device memory immediately
      if (bounceReleaseCmd)
         bounceReleaseCmd->Execute();

      throw;
   }
   CMD_END
}

} // namespace bvk
