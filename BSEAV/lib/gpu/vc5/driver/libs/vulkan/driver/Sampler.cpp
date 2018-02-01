/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

#include "libs/core/v3d/v3d_tmu.h"

namespace bvk {

Sampler::Sampler(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkSamplerCreateInfo     *pCreateInfo) :
      Allocating(pCallbacks)
{
   m_unnormalized = (pCreateInfo->unnormalizedCoordinates ? true : false);

   CreateSamplerRecord(pCreateInfo);
}

Sampler::~Sampler() noexcept
{
}

static v3d_tmu_std_bcol_t TranslateBorderColor(VkBorderColor bc)
{
   switch (bc)
   {
   case VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
   case VK_BORDER_COLOR_INT_TRANSPARENT_BLACK:        return V3D_TMU_STD_BCOL_0000;
   case VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
   case VK_BORDER_COLOR_INT_OPAQUE_BLACK:             return V3D_TMU_STD_BCOL_0001;
   case VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
   case VK_BORDER_COLOR_INT_OPAQUE_WHITE:             return V3D_TMU_STD_BCOL_1111;
   default:                                           unreachable();
   }
}

static v3d_tmu_wrap_t TranslateWrapMode(VkSamplerAddressMode mode)
{
   switch (mode)
   {
   case VK_SAMPLER_ADDRESS_MODE_REPEAT:               return V3D_TMU_WRAP_REPEAT;
   case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:      return V3D_TMU_WRAP_MIRROR;
   case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:        return V3D_TMU_WRAP_CLAMP;
   case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:      return V3D_TMU_WRAP_BORDER;
   case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: return V3D_TMU_WRAP_MIRROR_ONCE;
   default:                                           unreachable();
   }
}

static v3d_tmu_filter_t TranslateFilter(VkFilter filter)
{
   switch (filter)
   {
   case VK_FILTER_NEAREST: return V3D_TMU_FILTER_NEAREST;
   case VK_FILTER_LINEAR:  return V3D_TMU_FILTER_LINEAR;
   default:                unreachable(); return V3D_TMU_FILTER_INVALID;
   }
}

static v3d_tmu_mipfilt_t TranslateMipmapMode(VkSamplerMipmapMode mipmapMode)
{
   switch (mipmapMode)
   {
   case VK_SAMPLER_MIPMAP_MODE_NEAREST:   return V3D_TMU_MIPFILT_NEAREST;
   case VK_SAMPLER_MIPMAP_MODE_LINEAR:    return V3D_TMU_MIPFILT_LINEAR;
   default:                               unreachable(); return V3D_TMU_MIPFILT_INVALID;
   }
}

void Sampler::CreateSamplerRecord(const VkSamplerCreateInfo *pci)
{
   V3D_TMU_SAMPLER_T s = {};

   s.compare_func  = pci->compareEnable ? TranslateCompareFunc(pci->compareOp) : V3D_COMPARE_FUNC_ALWAYS;
   s.srgb_override = false;

   s.max_lod    = gfx_umin(gfx_float_to_uint32(pci->maxLod * 256.0f), (V3D_MAX_MIP_COUNT - 1) << 8);
   s.min_lod    = gfx_umin(gfx_float_to_uint32(pci->minLod * 256.0f), s.max_lod);
   s.fixed_bias = gfx_float_to_int32(pci->mipLodBias * 256.0f);

   s.magfilt   = TranslateFilter(pci->magFilter);
   s.minfilt   = TranslateFilter(pci->minFilter);
   s.mipfilt   = TranslateMipmapMode(pci->mipmapMode);
   s.aniso_en  = pci->anisotropyEnable && pci->maxAnisotropy > 1.0f;
   s.max_aniso = s.aniso_en ? v3d_tmu_translate_max_aniso(pci->maxAnisotropy) : V3D_MAX_ANISO_2;

   s.wrap_s = TranslateWrapMode(pci->addressModeU);
   s.wrap_t = TranslateWrapMode(pci->addressModeV);
   s.wrap_r = TranslateWrapMode(pci->addressModeW);
   s.wrap_i = V3D_TMU_WRAP_I_CLAMP; // Always set to CLAMP in sampler objects. BORDER is for internal use.

   s.std_bcol = TranslateBorderColor(pci->borderColor);

   v3d_pack_tmu_sampler(m_sampState, &s);
}


} // namespace bvk
