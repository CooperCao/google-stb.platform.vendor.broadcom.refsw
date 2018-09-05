/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *****************************************************************************************************/

#include "Formats.h"
#include "Common.h"
#include <cassert>

#include "libs/core/lfmt/lfmt_translate_v3d.h"

namespace bvk
{

#define FMT(comp, scaled, gfx) { VK_FORMAT_##comp, scaled, gfx }

// Vulkan formats
// --------------
// Non-packed formats or compressed formats are described in Vulkan with the leftmost
// component in the least-significant-byte.
// e.g. VK_FORMAT_B8G8R8A8 = B8G8R8A8 in memory from lowest address to highest
//
// Packed formats are in word size units. The leftmost component is in the
// most-significant-bit of the word.
// e.g. VK_FORMAT_A2B10G10R10_*_PACK32 = msb|AABBBBBBBBBBGGGGGGGGGGRRRRRRRRRR|lsb

// GFX_LFMTs
// ---------
// CACBCC means three slots of sizes A/B/C (bits) packed into a word of size
// A + B + C(bits) with A occupying the least significant bits and C occupying
// the most significant bits. This matches Vulkan's non-packed formats, but not the others.
// e.g. GFX_LFMT_R10G10B10A2 = msb|AABBBBBBBBBBGGGGGGGGGGRRRRRRRRRR|lsb

Formats::Format Formats::m_fmts[] =
{
   //  Component,                   Scale, GfxFmt
   FMT (UNDEFINED,                  false, GFX_LFMT_NONE                                        ),

   FMT (R4G4_UNORM_PACK8,           false, GFX_LFMT_NONE                                        ),
   FMT (R4G4B4A4_UNORM_PACK16,      false, GFX_LFMT_A4B4G4R4_UNORM                              ),
   FMT (B4G4R4A4_UNORM_PACK16,      false, GFX_LFMT_A4R4G4B4_UNORM                              ),
   FMT (R5G6B5_UNORM_PACK16,        false, GFX_LFMT_B5G6R5_UNORM                                ),
   FMT (B5G6R5_UNORM_PACK16,        false, GFX_LFMT_R5G6B5_UNORM                                ),
   FMT (R5G5B5A1_UNORM_PACK16,      false, GFX_LFMT_A1B5G5R5_UNORM                              ),
   FMT (B5G5R5A1_UNORM_PACK16,      false, GFX_LFMT_A1R5G5B5_UNORM                              ),
   FMT (A1R5G5B5_UNORM_PACK16,      false, GFX_LFMT_B5G5R5A1_UNORM                              ),
   FMT (R8_UNORM,                   false, GFX_LFMT_R8_UNORM                                    ),
   FMT (R8_SNORM,                   false, GFX_LFMT_R8_SNORM                                    ),
   FMT (R8_USCALED,                 true,  GFX_LFMT_R8_UINT                                     ),
   FMT (R8_SSCALED,                 true,  GFX_LFMT_R8_INT                                      ),
   FMT (R8_UINT,                    false, GFX_LFMT_R8_UINT                                     ),
   FMT (R8_SINT,                    false, GFX_LFMT_R8_INT                                      ),
   FMT (R8_SRGB,                    false, GFX_LFMT_R8_SRGB                                     ),
   FMT (R8G8_UNORM,                 false, GFX_LFMT_R8_G8_UNORM                                 ),
   FMT (R8G8_SNORM,                 false, GFX_LFMT_R8_G8_SNORM                                 ),
   FMT (R8G8_USCALED,               true,  GFX_LFMT_R8_G8_UINT                                  ),
   FMT (R8G8_SSCALED,               true,  GFX_LFMT_R8_G8_INT                                   ),
   FMT (R8G8_UINT,                  false, GFX_LFMT_R8_G8_UINT                                  ),
   FMT (R8G8_SINT,                  false, GFX_LFMT_R8_G8_INT                                   ),
   FMT (R8G8_SRGB,                  false, GFX_LFMT_R8_G8_SRGB                                  ),
   FMT (R8G8B8_UNORM,               false, GFX_LFMT_R8_G8_B8_UNORM                              ),
   FMT (R8G8B8_SNORM,               false, GFX_LFMT_R8_G8_B8_SNORM                              ),
   FMT (R8G8B8_USCALED,             true,  GFX_LFMT_R8_G8_B8_UINT                               ),
   FMT (R8G8B8_SSCALED,             true,  GFX_LFMT_R8_G8_B8_INT                                ),
   FMT (R8G8B8_UINT,                false, GFX_LFMT_R8_G8_B8_UINT                               ),
   FMT (R8G8B8_SINT,                false, GFX_LFMT_R8_G8_B8_INT                                ),
   FMT (R8G8B8_SRGB,                false, GFX_LFMT_R8_G8_B8_SRGB                               ),
   FMT (B8G8R8_UNORM,               false, GFX_LFMT_B8_G8_R8_UNORM                              ),
   FMT (B8G8R8_SNORM,               false, GFX_LFMT_B8_G8_R8_SNORM                              ),
   FMT (B8G8R8_USCALED,             true,  GFX_LFMT_B8_G8_R8_UINT                               ),
   FMT (B8G8R8_SSCALED,             true,  GFX_LFMT_B8_G8_R8_INT                                ),
   FMT (B8G8R8_UINT,                false, GFX_LFMT_B8_G8_R8_UINT                               ),
   FMT (B8G8R8_SINT,                false, GFX_LFMT_B8_G8_R8_INT                                ),
   FMT (B8G8R8_SRGB,                false, GFX_LFMT_B8_G8_R8_SRGB                               ),
   FMT (R8G8B8A8_UNORM,             false, GFX_LFMT_R8_G8_B8_A8_UNORM                           ),
   FMT (R8G8B8A8_SNORM,             false, GFX_LFMT_R8_G8_B8_A8_SNORM                           ),
   FMT (R8G8B8A8_USCALED,           true,  GFX_LFMT_R8_G8_B8_A8_UINT                            ),
   FMT (R8G8B8A8_SSCALED,           true,  GFX_LFMT_R8_G8_B8_A8_INT                             ),
   FMT (R8G8B8A8_UINT,              false, GFX_LFMT_R8_G8_B8_A8_UINT                            ),
   FMT (R8G8B8A8_SINT,              false, GFX_LFMT_R8_G8_B8_A8_INT                             ),
   FMT (R8G8B8A8_SRGB,              false, GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM            ),

   FMT (B8G8R8A8_UNORM,             false, GFX_LFMT_B8_G8_R8_A8_UNORM                           ),
   FMT (B8G8R8A8_SNORM,             false, GFX_LFMT_B8_G8_R8_A8_SNORM                           ),
   FMT (B8G8R8A8_USCALED,           true,  GFX_LFMT_B8_G8_R8_A8_UINT                            ),
   FMT (B8G8R8A8_SSCALED,           true,  GFX_LFMT_B8_G8_R8_A8_INT                             ),
   FMT (B8G8R8A8_UINT,              false, GFX_LFMT_B8_G8_R8_A8_UINT                            ),
   FMT (B8G8R8A8_SINT,              false, GFX_LFMT_B8_G8_R8_A8_INT                             ),
   FMT (B8G8R8A8_SRGB,              false, GFX_LFMT_B8_G8_R8_A8_SRGB_SRGB_SRGB_UNORM            ),
   FMT (A8B8G8R8_UNORM_PACK32,      false, GFX_LFMT_R8_G8_B8_A8_UNORM /* LE only */             ),
   FMT (A8B8G8R8_SNORM_PACK32,      false, GFX_LFMT_R8_G8_B8_A8_SNORM /* LE only */             ),
   FMT (A8B8G8R8_USCALED_PACK32,    true,  GFX_LFMT_R8_G8_B8_A8_UINT                            ),
   FMT (A8B8G8R8_SSCALED_PACK32,    true,  GFX_LFMT_R8_G8_B8_A8_INT                             ),
   FMT (A8B8G8R8_UINT_PACK32,       false, GFX_LFMT_R8_G8_B8_A8_UINT  /* LE only */             ),
   FMT (A8B8G8R8_SINT_PACK32,       false, GFX_LFMT_R8_G8_B8_A8_INT   /* LE only */             ),
   FMT (A8B8G8R8_SRGB_PACK32,       false, GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM/*LE only*/ ),
   FMT (A2R10G10B10_UNORM_PACK32,   false, GFX_LFMT_B10G10R10A2_UNORM                           ),
   FMT (A2R10G10B10_SNORM_PACK32,   false, GFX_LFMT_B10G10R10A2_SNORM                           ),
   FMT (A2R10G10B10_USCALED_PACK32, true,  GFX_LFMT_B10G10R10A2_UINT                            ),
   FMT (A2R10G10B10_SSCALED_PACK32, true,  GFX_LFMT_B10G10R10A2_INT                             ),
   FMT (A2R10G10B10_UINT_PACK32,    false, GFX_LFMT_B10G10R10A2_UINT                            ),
   FMT (A2R10G10B10_SINT_PACK32,    false, GFX_LFMT_B10G10R10A2_INT                             ),
   FMT (A2B10G10R10_UNORM_PACK32,   false, GFX_LFMT_R10G10B10A2_UNORM                           ),
   FMT (A2B10G10R10_SNORM_PACK32,   false, GFX_LFMT_R10G10B10A2_SNORM                           ),
   FMT (A2B10G10R10_USCALED_PACK32, true,  GFX_LFMT_R10G10B10A2_UINT                            ),
   FMT (A2B10G10R10_SSCALED_PACK32, true,  GFX_LFMT_R10G10B10A2_INT                             ),
   FMT (A2B10G10R10_UINT_PACK32,    false, GFX_LFMT_R10G10B10A2_UINT                            ),
   FMT (A2B10G10R10_SINT_PACK32,    false, GFX_LFMT_R10G10B10A2_INT                             ),
   FMT (R16_UNORM,                  false, GFX_LFMT_R16_UNORM                                   ),
   FMT (R16_SNORM,                  false, GFX_LFMT_R16_SNORM                                   ),
   FMT (R16_USCALED,                true,  GFX_LFMT_R16_UINT                                    ),
   FMT (R16_SSCALED,                true,  GFX_LFMT_R16_INT                                     ),
   FMT (R16_UINT,                   false, GFX_LFMT_R16_UINT                                    ),
   FMT (R16_SINT,                   false, GFX_LFMT_R16_INT                                     ),
   FMT (R16_SFLOAT,                 false, GFX_LFMT_R16_FLOAT                                   ),
   FMT (R16G16_UNORM,               false, GFX_LFMT_R16_G16_UNORM                               ),
   FMT (R16G16_SNORM,               false, GFX_LFMT_R16_G16_SNORM                               ),
   FMT (R16G16_USCALED,             true,  GFX_LFMT_R16_G16_UINT                                ),
   FMT (R16G16_SSCALED,             true,  GFX_LFMT_R16_G16_INT                                 ),
   FMT (R16G16_UINT,                false, GFX_LFMT_R16_G16_UINT                                ),
   FMT (R16G16_SINT,                false, GFX_LFMT_R16_G16_INT                                 ),
   FMT (R16G16_SFLOAT,              false, GFX_LFMT_R16_G16_FLOAT                               ),
   FMT (R16G16B16_UNORM,            false, GFX_LFMT_R16_G16_B16_UNORM                           ),
   FMT (R16G16B16_SNORM,            false, GFX_LFMT_R16_G16_B16_SNORM                           ),
   FMT (R16G16B16_USCALED,          true,  GFX_LFMT_R16_G16_B16_UINT                            ),
   FMT (R16G16B16_SSCALED,          true,  GFX_LFMT_R16_G16_B16_INT                             ),
   FMT (R16G16B16_UINT,             false, GFX_LFMT_R16_G16_B16_UINT                            ),
   FMT (R16G16B16_SINT,             false, GFX_LFMT_R16_G16_B16_INT                             ),
   FMT (R16G16B16_SFLOAT,           false, GFX_LFMT_R16_G16_B16_FLOAT                           ),
   FMT (R16G16B16A16_UNORM,         false, GFX_LFMT_R16_G16_B16_A16_UNORM                       ),
   FMT (R16G16B16A16_SNORM,         false, GFX_LFMT_R16_G16_B16_A16_SNORM                       ),
   FMT (R16G16B16A16_USCALED,       true,  GFX_LFMT_R16_G16_B16_A16_UINT                        ),
   FMT (R16G16B16A16_SSCALED,       true,  GFX_LFMT_R16_G16_B16_A16_INT                         ),
   FMT (R16G16B16A16_UINT,          false, GFX_LFMT_R16_G16_B16_A16_UINT                        ),
   FMT (R16G16B16A16_SINT,          false, GFX_LFMT_R16_G16_B16_A16_INT                         ),
   FMT (R16G16B16A16_SFLOAT,        false, GFX_LFMT_R16_G16_B16_A16_FLOAT                       ),
   FMT (R32_UINT,                   false, GFX_LFMT_R32_UINT                                    ),
   FMT (R32_SINT,                   false, GFX_LFMT_R32_INT                                     ),
   FMT (R32_SFLOAT,                 false, GFX_LFMT_R32_FLOAT                                   ),
   FMT (R32G32_UINT,                false, GFX_LFMT_R32_G32_UINT                                ),
   FMT (R32G32_SINT,                false, GFX_LFMT_R32_G32_INT                                 ),
   FMT (R32G32_SFLOAT,              false, GFX_LFMT_R32_G32_FLOAT                               ),
   FMT (R32G32B32_UINT,             false, GFX_LFMT_R32_G32_B32_UINT                            ),
   FMT (R32G32B32_SINT,             false, GFX_LFMT_R32_G32_B32_INT                             ),
   FMT (R32G32B32_SFLOAT,           false, GFX_LFMT_R32_G32_B32_FLOAT                           ),
   FMT (R32G32B32A32_UINT,          false, GFX_LFMT_R32_G32_B32_A32_UINT                        ),
   FMT (R32G32B32A32_SINT,          false, GFX_LFMT_R32_G32_B32_A32_INT                         ),
   FMT (R32G32B32A32_SFLOAT,        false, GFX_LFMT_R32_G32_B32_A32_FLOAT                       ),
   FMT (R64_UINT,                   false, GFX_LFMT_NONE                                        ),
   FMT (R64_SINT,                   false, GFX_LFMT_NONE                                        ),
   FMT (R64_SFLOAT,                 false, GFX_LFMT_NONE                                        ),
   FMT (R64G64_UINT,                false, GFX_LFMT_NONE                                        ),
   FMT (R64G64_SINT,                false, GFX_LFMT_NONE                                        ),
   FMT (R64G64_SFLOAT,              false, GFX_LFMT_NONE                                        ),
   FMT (R64G64B64_UINT,             false, GFX_LFMT_NONE                                        ),
   FMT (R64G64B64_SINT,             false, GFX_LFMT_NONE                                        ),
   FMT (R64G64B64_SFLOAT,           false, GFX_LFMT_NONE                                        ),
   FMT (R64G64B64A64_UINT,          false, GFX_LFMT_NONE                                        ),
   FMT (R64G64B64A64_SINT,          false, GFX_LFMT_NONE                                        ),
   FMT (R64G64B64A64_SFLOAT,        false, GFX_LFMT_NONE                                        ),
   FMT (B10G11R11_UFLOAT_PACK32,    false, GFX_LFMT_R11G11B10_UFLOAT                            ),
   FMT (E5B9G9R9_UFLOAT_PACK32,     false, GFX_LFMT_R9G9B9SHAREDEXP5_UFLOAT                     ),
   FMT (D16_UNORM,                  false, GFX_LFMT_D16_UNORM                                   ),
   FMT (X8_D24_UNORM_PACK32,        false, GFX_LFMT_D24X8_UNORM                                 ),
   FMT (D32_SFLOAT,                 false, GFX_LFMT_D32_FLOAT                                   ),
   FMT (S8_UINT,                    false, GFX_LFMT_NONE /*GFX_LFMT_S8_UINT*/                   ),
   FMT (D16_UNORM_S8_UINT,          false, GFX_LFMT_NONE                                        ),
   FMT (D24_UNORM_S8_UINT,          false, GFX_LFMT_S8D24_UINT_UNORM                            ),
   FMT (D32_SFLOAT_S8_UINT,         false, GFX_LFMT_D32_S8X24_FLOAT_UINT                        ),
   FMT (BC1_RGB_UNORM_BLOCK,        false, GFX_LFMT_BC1_RGBX_UNORM                              ),
   FMT (BC1_RGB_SRGB_BLOCK,         false, GFX_LFMT_BC1_RGBX_SRGB                               ),
   FMT (BC1_RGBA_UNORM_BLOCK,       false, GFX_LFMT_BC1_RGBA_UNORM                              ),
   FMT (BC1_RGBA_SRGB_BLOCK,        false, GFX_LFMT_BC1_RGBA_SRGB_SRGB_SRGB_UNORM               ),
   FMT (BC2_UNORM_BLOCK,            false, GFX_LFMT_BC2_RGBA_UNORM                              ),
   FMT (BC2_SRGB_BLOCK,             false, GFX_LFMT_BC2_RGBA_SRGB_SRGB_SRGB_UNORM               ),
   FMT (BC3_UNORM_BLOCK,            false, GFX_LFMT_BC3_RGBA_UNORM                              ),
   FMT (BC3_SRGB_BLOCK,             false, GFX_LFMT_BC3_RGBA_SRGB_SRGB_SRGB_UNORM               ),
   FMT (BC4_UNORM_BLOCK,            false, GFX_LFMT_NONE                                        ),
   FMT (BC4_SNORM_BLOCK,            false, GFX_LFMT_NONE                                        ),
   FMT (BC5_UNORM_BLOCK,            false, GFX_LFMT_NONE                                        ),
   FMT (BC5_SNORM_BLOCK,            false, GFX_LFMT_NONE                                        ),
   FMT (BC6H_UFLOAT_BLOCK,          false, GFX_LFMT_NONE                                        ),
   FMT (BC6H_SFLOAT_BLOCK,          false, GFX_LFMT_NONE                                        ),
   FMT (BC7_UNORM_BLOCK,            false, GFX_LFMT_NONE                                        ),
   FMT (BC7_SRGB_BLOCK,             false, GFX_LFMT_NONE                                        ),
   FMT (ETC2_R8G8B8_UNORM_BLOCK,    false, GFX_LFMT_ETC2_RGB_UNORM                              ),
   FMT (ETC2_R8G8B8_SRGB_BLOCK,     false, GFX_LFMT_ETC2_RGB_SRGB                               ),
   FMT (ETC2_R8G8B8A1_UNORM_BLOCK,  false, GFX_LFMT_PUNCHTHROUGH_ETC2_RGBA_UNORM                ),
   FMT (ETC2_R8G8B8A1_SRGB_BLOCK,   false, GFX_LFMT_PUNCHTHROUGH_ETC2_RGBA_SRGB_SRGB_SRGB_UNORM ),
   FMT (ETC2_R8G8B8A8_UNORM_BLOCK,  false, GFX_LFMT_ETC2_EAC_RGBA_UNORM                         ),
   FMT (ETC2_R8G8B8A8_SRGB_BLOCK,   false, GFX_LFMT_ETC2_EAC_RGBA_SRGB_SRGB_SRGB_UNORM          ),
   FMT (EAC_R11_UNORM_BLOCK,        false, GFX_LFMT_EAC_R_UNORM                                 ),
   FMT (EAC_R11_SNORM_BLOCK,        false, GFX_LFMT_EAC_R_SNORM                                 ),
   FMT (EAC_R11G11_UNORM_BLOCK,     false, GFX_LFMT_EAC_EAC_RG_UNORM                            ),
   FMT (EAC_R11G11_SNORM_BLOCK,     false, GFX_LFMT_EAC_EAC_RG_SNORM                            ),
   FMT (ASTC_4x4_UNORM_BLOCK,       false, GFX_LFMT_ASTC4X4_RGBA_UNORM                          ),
   FMT (ASTC_4x4_SRGB_BLOCK,        false, GFX_LFMT_ASTC4X4_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   FMT (ASTC_5x4_UNORM_BLOCK,       false, GFX_LFMT_ASTC5X4_RGBA_UNORM                          ),
   FMT (ASTC_5x4_SRGB_BLOCK,        false, GFX_LFMT_ASTC5X4_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   FMT (ASTC_5x5_UNORM_BLOCK,       false, GFX_LFMT_ASTC5X5_RGBA_UNORM                          ),
   FMT (ASTC_5x5_SRGB_BLOCK,        false, GFX_LFMT_ASTC5X5_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   FMT (ASTC_6x5_UNORM_BLOCK,       false, GFX_LFMT_ASTC6X5_RGBA_UNORM                          ),
   FMT (ASTC_6x5_SRGB_BLOCK,        false, GFX_LFMT_ASTC6X5_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   FMT (ASTC_6x6_UNORM_BLOCK,       false, GFX_LFMT_ASTC6X6_RGBA_UNORM                          ),
   FMT (ASTC_6x6_SRGB_BLOCK,        false, GFX_LFMT_ASTC6X6_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   FMT (ASTC_8x5_UNORM_BLOCK,       false, GFX_LFMT_ASTC8X5_RGBA_UNORM                          ),
   FMT (ASTC_8x5_SRGB_BLOCK,        false, GFX_LFMT_ASTC8X5_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   FMT (ASTC_8x6_UNORM_BLOCK,       false, GFX_LFMT_ASTC8X6_RGBA_UNORM                          ),
   FMT (ASTC_8x6_SRGB_BLOCK,        false, GFX_LFMT_ASTC8X6_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   FMT (ASTC_8x8_UNORM_BLOCK,       false, GFX_LFMT_ASTC8X8_RGBA_UNORM                          ),
   FMT (ASTC_8x8_SRGB_BLOCK,        false, GFX_LFMT_ASTC8X8_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   FMT (ASTC_10x5_UNORM_BLOCK,      false, GFX_LFMT_ASTC10X5_RGBA_UNORM                         ),
   FMT (ASTC_10x5_SRGB_BLOCK,       false, GFX_LFMT_ASTC10X5_RGBA_SRGB_SRGB_SRGB_UNORM          ),
   FMT (ASTC_10x6_UNORM_BLOCK,      false, GFX_LFMT_ASTC10X6_RGBA_UNORM                         ),
   FMT (ASTC_10x6_SRGB_BLOCK,       false, GFX_LFMT_ASTC10X6_RGBA_SRGB_SRGB_SRGB_UNORM          ),
   FMT (ASTC_10x8_UNORM_BLOCK,      false, GFX_LFMT_ASTC10X8_RGBA_UNORM                         ),
   FMT (ASTC_10x8_SRGB_BLOCK,       false, GFX_LFMT_ASTC10X8_RGBA_SRGB_SRGB_SRGB_UNORM          ),
   FMT (ASTC_10x10_UNORM_BLOCK,     false, GFX_LFMT_ASTC10X10_RGBA_UNORM                        ),
   FMT (ASTC_10x10_SRGB_BLOCK,      false, GFX_LFMT_ASTC10X10_RGBA_SRGB_SRGB_SRGB_UNORM         ),
   FMT (ASTC_12x10_UNORM_BLOCK,     false, GFX_LFMT_ASTC12X10_RGBA_UNORM                        ),
   FMT (ASTC_12x10_SRGB_BLOCK,      false, GFX_LFMT_ASTC12X10_RGBA_SRGB_SRGB_SRGB_UNORM         ),
   FMT (ASTC_12x12_UNORM_BLOCK,     false, GFX_LFMT_ASTC12X12_RGBA_UNORM                        ),
   FMT (ASTC_12x12_SRGB_BLOCK,      false, GFX_LFMT_ASTC12X12_RGBA_SRGB_SRGB_SRGB_UNORM         ),
};

Formats::Formats()
{
   static_assert(countof(Formats::m_fmts) == VK_FORMAT_RANGE_SIZE,
                 "Format::m_formats size doesn't match VkFormat enums");

   // Check that the entries are all in the order of the VkFormat enums so that
   // we can just lookup using the enum value
   for (VkFormat f = VK_FORMAT_BEGIN_RANGE; f < VK_FORMAT_END_RANGE; f = (VkFormat)(f + 1))
      assert(f == Formats::m_fmts[f].fmtEnum);
}

bool Formats::HasTMUSupport(GFX_LFMT_T lfmt)
{
   if (gfx_lfmt_has_depth(lfmt) && gfx_lfmt_has_stencil(lfmt))
      return false;

   GFX_LFMT_TMU_TRANSLATION_T t;
   if (!gfx_lfmt_maybe_translate_tmu(&t, gfx_lfmt_ds_to_red(lfmt)
#if !V3D_HAS_TMU_R32F_R16_SHAD
      , /*need_depth_type=*/gfx_lfmt_has_depth(lfmt)
#endif
      ))
      return false;

#if V3D_HAS_TMU_R32F_R16_SHAD
   if (gfx_lfmt_has_depth(lfmt) && !v3d_tmu_type_supports_shadow(t.type))
      return false;
#endif

   return true;
}

// Make one so that the constructor (which checks the array) will be called
static Formats s_format;

} // namespace bvk
