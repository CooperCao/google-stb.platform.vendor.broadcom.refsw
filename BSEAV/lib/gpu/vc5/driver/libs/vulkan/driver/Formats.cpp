/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Formats.h"
#include "Common.h"
#include <cassert>

#include "libs/core/lfmt/lfmt_translate_v3d.h"

namespace bvk
{

#define FMT(comp, numeric_format, cls, gfx)\
   { VK_FORMAT_##comp##_##numeric_format, comp, numeric_format, cls, NO_FMTTYPE, gfx, gfx }

#define SFMT(comp, numeric_format, cls, suffix, gfx)\
   { VK_FORMAT_##comp##_##numeric_format##suffix, comp, numeric_format, cls, suffix, gfx, gfx }

#if !V3D_VER_AT_LEAST(4,1,34,0)
# define FMT_SPECIAL(comp, numeric_format, cls, naturalFmt, hwFmt)\
    { VK_FORMAT_##comp##_##numeric_format, comp, numeric_format, cls, NO_FMTTYPE, naturalFmt, hwFmt }

# define SFMT_SPECIAL(comp, numeric_format, cls, suffix, naturalFmt, hwFmt)\
    { VK_FORMAT_##comp##_##numeric_format##suffix, comp, numeric_format, cls, suffix, naturalFmt, hwFmt }
#endif


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
   // Special case for undefined
   { VK_FORMAT_UNDEFINED, UNDEFINED, NO_NUMFMT, CLASS_NONE, NO_FMTTYPE, GFX_LFMT_NONE },

   //  Component,            NumericFormat,       CompatibilityClass,  Suffix,    GfxFmt
   SFMT (R4G4,               UNORM,               CLASS_8,             _PACK8,    GFX_LFMT_NONE                                        ),
   SFMT (R4G4B4A4,           UNORM,               CLASS_16,            _PACK16,   GFX_LFMT_A4B4G4R4_UNORM                              ),
   SFMT (B4G4R4A4,           UNORM,               CLASS_16,            _PACK16,   GFX_LFMT_A4R4G4B4_UNORM                              ),
   SFMT (R5G6B5,             UNORM,               CLASS_16,            _PACK16,   GFX_LFMT_B5G6R5_UNORM                                ),
   SFMT (B5G6R5,             UNORM,               CLASS_16,            _PACK16,   GFX_LFMT_R5G6B5_UNORM                                ),
   SFMT (R5G5B5A1,           UNORM,               CLASS_16,            _PACK16,   GFX_LFMT_A1B5G5R5_UNORM                              ),
   SFMT (B5G5R5A1,           UNORM,               CLASS_16,            _PACK16,   GFX_LFMT_A1R5G5B5_UNORM                              ),
#if V3D_VER_AT_LEAST(4,1,34,0)
   SFMT (A1R5G5B5,           UNORM,               CLASS_16,            _PACK16,   GFX_LFMT_B5G5R5A1_UNORM                              ),
#else
   SFMT_SPECIAL
        (A1R5G5B5,           UNORM,               CLASS_16,            _PACK16,   /* NaturalFmt=*/GFX_LFMT_B5G5R5A1_UNORM,
                                                                                  /* HWFmt=*/     GFX_LFMT_A1B5G5R5_UNORM              ),
#endif
   FMT  (R8,                 UNORM,               CLASS_8,                        GFX_LFMT_R8_UNORM                                    ),
   FMT  (R8,                 SNORM,               CLASS_8,                        GFX_LFMT_R8_SNORM                                    ),
   FMT  (R8,                 USCALED,             CLASS_8,                        GFX_LFMT_R8_UINT                                     ),
   FMT  (R8,                 SSCALED,             CLASS_8,                        GFX_LFMT_R8_INT                                      ),
   FMT  (R8,                 UINT,                CLASS_8,                        GFX_LFMT_R8_UINT                                     ),
   FMT  (R8,                 SINT,                CLASS_8,                        GFX_LFMT_R8_INT                                      ),
   FMT  (R8,                 SRGB,                CLASS_8,                        GFX_LFMT_R8_SRGB                                     ),
   FMT  (R8G8,               UNORM,               CLASS_16,                       GFX_LFMT_R8_G8_UNORM                                 ),
   FMT  (R8G8,               SNORM,               CLASS_16,                       GFX_LFMT_R8_G8_SNORM                                 ),
   FMT  (R8G8,               USCALED,             CLASS_16,                       GFX_LFMT_R8_G8_UINT                                  ),
   FMT  (R8G8,               SSCALED,             CLASS_16,                       GFX_LFMT_R8_G8_INT                                   ),
   FMT  (R8G8,               UINT,                CLASS_16,                       GFX_LFMT_R8_G8_UINT                                  ),
   FMT  (R8G8,               SINT,                CLASS_16,                       GFX_LFMT_R8_G8_INT                                   ),
   FMT  (R8G8,               SRGB,                CLASS_16,                       GFX_LFMT_R8_G8_SRGB                                  ),
   FMT  (R8G8B8,             UNORM,               CLASS_24,                       GFX_LFMT_R8_G8_B8_UNORM                              ),
   FMT  (R8G8B8,             SNORM,               CLASS_24,                       GFX_LFMT_R8_G8_B8_SNORM                              ),
   FMT  (R8G8B8,             USCALED,             CLASS_24,                       GFX_LFMT_R8_G8_B8_UINT                               ),
   FMT  (R8G8B8,             SSCALED,             CLASS_24,                       GFX_LFMT_R8_G8_B8_INT                                ),
   FMT  (R8G8B8,             UINT,                CLASS_24,                       GFX_LFMT_R8_G8_B8_UINT                               ),
   FMT  (R8G8B8,             SINT,                CLASS_24,                       GFX_LFMT_R8_G8_B8_INT                                ),
   FMT  (R8G8B8,             SRGB,                CLASS_24,                       GFX_LFMT_R8_G8_B8_SRGB                               ),
   FMT  (B8G8R8,             UNORM,               CLASS_24,                       GFX_LFMT_B8_G8_R8_UNORM                              ),
   FMT  (B8G8R8,             SNORM,               CLASS_24,                       GFX_LFMT_NONE                                        ),
   FMT  (B8G8R8,             USCALED,             CLASS_24,                       GFX_LFMT_NONE                                        ),
   FMT  (B8G8R8,             SSCALED,             CLASS_24,                       GFX_LFMT_NONE                                        ),
   FMT  (B8G8R8,             UINT,                CLASS_24,                       GFX_LFMT_NONE                                        ),
   FMT  (B8G8R8,             SINT,                CLASS_24,                       GFX_LFMT_NONE                                        ),
   FMT  (B8G8R8,             SRGB,                CLASS_24,                       GFX_LFMT_B8_G8_R8_SRGB                               ),
   FMT  (R8G8B8A8,           UNORM,               CLASS_32,                       GFX_LFMT_R8_G8_B8_A8_UNORM                           ),
   FMT  (R8G8B8A8,           SNORM,               CLASS_32,                       GFX_LFMT_R8_G8_B8_A8_SNORM                           ),
   FMT  (R8G8B8A8,           USCALED,             CLASS_32,                       GFX_LFMT_R8_G8_B8_A8_UINT                            ),
   FMT  (R8G8B8A8,           SSCALED,             CLASS_32,                       GFX_LFMT_R8_G8_B8_A8_INT                             ),
   FMT  (R8G8B8A8,           UINT,                CLASS_32,                       GFX_LFMT_R8_G8_B8_A8_UINT                            ),
   FMT  (R8G8B8A8,           SINT,                CLASS_32,                       GFX_LFMT_R8_G8_B8_A8_INT                             ),
   FMT  (R8G8B8A8,           SRGB,                CLASS_32,                       GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM            ),

#if V3D_VER_AT_LEAST(4,1,34,0)
   FMT  (B8G8R8A8,           UNORM,               CLASS_32,                       GFX_LFMT_B8_G8_R8_A8_UNORM                           ),
#else
   FMT_SPECIAL
        (B8G8R8A8,           UNORM,               CLASS_32,       /* NaturalFmt=*/GFX_LFMT_B8_G8_R8_A8_UNORM,
                                                                  /* HWFmt=*/     GFX_LFMT_R8_G8_B8_A8_UNORM                           ),
#endif
   FMT  (B8G8R8A8,           SNORM,               CLASS_32,                       GFX_LFMT_B8_G8_R8_A8_SNORM                           ),
   FMT  (B8G8R8A8,           USCALED,             CLASS_32,                       GFX_LFMT_NONE                                        ),
   FMT  (B8G8R8A8,           SSCALED,             CLASS_32,                       GFX_LFMT_NONE                                        ),
   FMT  (B8G8R8A8,           UINT,                CLASS_32,                       GFX_LFMT_B8_G8_R8_A8_UINT                            ),
   FMT  (B8G8R8A8,           SINT,                CLASS_32,                       GFX_LFMT_B8_G8_R8_A8_INT                             ),
#if V3D_VER_AT_LEAST(4,1,34,0)
   FMT  (B8G8R8A8,           SRGB,                CLASS_32,                       GFX_LFMT_B8_G8_R8_A8_SRGB_SRGB_SRGB_UNORM            ),
#else
   FMT_SPECIAL
        (B8G8R8A8,           SRGB,                CLASS_32,       /* NaturalFmt=*/GFX_LFMT_B8_G8_R8_A8_SRGB_SRGB_SRGB_UNORM,
                                                                  /* HWFmt=*/     GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM            ),
#endif
   SFMT (A8B8G8R8,           UNORM,               CLASS_32,            _PACK32,   GFX_LFMT_R8_G8_B8_A8_UNORM /* LE only */             ),
   SFMT (A8B8G8R8,           SNORM,               CLASS_32,            _PACK32,   GFX_LFMT_R8_G8_B8_A8_SNORM /* LE only */             ),
   SFMT (A8B8G8R8,           USCALED,             CLASS_32,            _PACK32,   GFX_LFMT_R8_G8_B8_A8_UINT                            ),
   SFMT (A8B8G8R8,           SSCALED,             CLASS_32,            _PACK32,   GFX_LFMT_R8_G8_B8_A8_INT                             ),
   SFMT (A8B8G8R8,           UINT,                CLASS_32,            _PACK32,   GFX_LFMT_R8_G8_B8_A8_UINT  /* LE only */             ),
   SFMT (A8B8G8R8,           SINT,                CLASS_32,            _PACK32,   GFX_LFMT_R8_G8_B8_A8_INT   /* LE only */             ),
   SFMT (A8B8G8R8,           SRGB,                CLASS_32,            _PACK32,   GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM/*LE only*/ ),
   SFMT (A2R10G10B10,        UNORM,               CLASS_32,            _PACK32,   GFX_LFMT_B10G10R10A2_UNORM                           ),
   SFMT (A2R10G10B10,        SNORM,               CLASS_32,            _PACK32,   GFX_LFMT_NONE                                        ),
   SFMT (A2R10G10B10,        USCALED,             CLASS_32,            _PACK32,   GFX_LFMT_NONE                                        ),
   SFMT (A2R10G10B10,        SSCALED,             CLASS_32,            _PACK32,   GFX_LFMT_NONE                                        ),
   SFMT (A2R10G10B10,        UINT,                CLASS_32,            _PACK32,   GFX_LFMT_B10G10R10A2_UINT                            ),
   SFMT (A2R10G10B10,        SINT,                CLASS_32,            _PACK32,   GFX_LFMT_NONE                                        ),
   SFMT (A2B10G10R10,        UNORM,               CLASS_32,            _PACK32,   GFX_LFMT_R10G10B10A2_UNORM                           ),
   SFMT (A2B10G10R10,        SNORM,               CLASS_32,            _PACK32,   GFX_LFMT_R10G10B10A2_SNORM                           ),
   SFMT (A2B10G10R10,        USCALED,             CLASS_32,            _PACK32,   GFX_LFMT_R10G10B10A2_UINT                            ),
   SFMT (A2B10G10R10,        SSCALED,             CLASS_32,            _PACK32,   GFX_LFMT_R10G10B10A2_INT                             ),
   SFMT (A2B10G10R10,        UINT,                CLASS_32,            _PACK32,   GFX_LFMT_R10G10B10A2_UINT                            ),
   SFMT (A2B10G10R10,        SINT,                CLASS_32,            _PACK32,   GFX_LFMT_R10G10B10A2_INT                             ),
   FMT  (R16,                UNORM,               CLASS_16,                       GFX_LFMT_R16_UNORM                                   ),
   FMT  (R16,                SNORM,               CLASS_16,                       GFX_LFMT_R16_SNORM                                   ),
   FMT  (R16,                USCALED,             CLASS_16,                       GFX_LFMT_R16_UINT                                    ),
   FMT  (R16,                SSCALED,             CLASS_16,                       GFX_LFMT_R16_INT                                     ),
   FMT  (R16,                UINT,                CLASS_16,                       GFX_LFMT_R16_UINT                                    ),
   FMT  (R16,                SINT,                CLASS_16,                       GFX_LFMT_R16_INT                                     ),
   FMT  (R16,                SFLOAT,              CLASS_16,                       GFX_LFMT_R16_FLOAT                                   ),
   FMT  (R16G16,             UNORM,               CLASS_32,                       GFX_LFMT_R16_G16_UNORM                               ),
   FMT  (R16G16,             SNORM,               CLASS_32,                       GFX_LFMT_R16_G16_SNORM                               ),
   FMT  (R16G16,             USCALED,             CLASS_32,                       GFX_LFMT_R16_G16_UINT                                ),
   FMT  (R16G16,             SSCALED,             CLASS_32,                       GFX_LFMT_R16_G16_INT                                 ),
   FMT  (R16G16,             UINT,                CLASS_32,                       GFX_LFMT_R16_G16_UINT                                ),
   FMT  (R16G16,             SINT,                CLASS_32,                       GFX_LFMT_R16_G16_INT                                 ),
   FMT  (R16G16,             SFLOAT,              CLASS_32,                       GFX_LFMT_R16_G16_FLOAT                               ),
   FMT  (R16G16B16,          UNORM,               CLASS_48,                       GFX_LFMT_R16_G16_B16_UNORM                           ),
   FMT  (R16G16B16,          SNORM,               CLASS_48,                       GFX_LFMT_R16_G16_B16_SNORM                           ),
   FMT  (R16G16B16,          USCALED,             CLASS_48,                       GFX_LFMT_R16_G16_B16_UINT                            ),
   FMT  (R16G16B16,          SSCALED,             CLASS_48,                       GFX_LFMT_R16_G16_B16_INT                             ),
   FMT  (R16G16B16,          UINT,                CLASS_48,                       GFX_LFMT_R16_G16_B16_UINT                            ),
   FMT  (R16G16B16,          SINT,                CLASS_48,                       GFX_LFMT_R16_G16_B16_INT                             ),
   FMT  (R16G16B16,          SFLOAT,              CLASS_48,                       GFX_LFMT_R16_G16_B16_FLOAT                           ),
   FMT  (R16G16B16A16,       UNORM,               CLASS_64,                       GFX_LFMT_R16_G16_B16_A16_UNORM                       ),
   FMT  (R16G16B16A16,       SNORM,               CLASS_64,                       GFX_LFMT_R16_G16_B16_A16_SNORM                       ),
   FMT  (R16G16B16A16,       USCALED,             CLASS_64,                       GFX_LFMT_R16_G16_B16_A16_UINT                        ),
   FMT  (R16G16B16A16,       SSCALED,             CLASS_64,                       GFX_LFMT_R16_G16_B16_A16_INT                         ),
   FMT  (R16G16B16A16,       UINT,                CLASS_64,                       GFX_LFMT_R16_G16_B16_A16_UINT                        ),
   FMT  (R16G16B16A16,       SINT,                CLASS_64,                       GFX_LFMT_R16_G16_B16_A16_INT                         ),
   FMT  (R16G16B16A16,       SFLOAT,              CLASS_64,                       GFX_LFMT_R16_G16_B16_A16_FLOAT                       ),
   FMT  (R32,                UINT,                CLASS_32,                       GFX_LFMT_R32_UINT                                    ),
   FMT  (R32,                SINT,                CLASS_32,                       GFX_LFMT_R32_INT                                     ),
   FMT  (R32,                SFLOAT,              CLASS_32,                       GFX_LFMT_R32_FLOAT                                   ),
   FMT  (R32G32,             UINT,                CLASS_64,                       GFX_LFMT_R32_G32_UINT                                ),
   FMT  (R32G32,             SINT,                CLASS_64,                       GFX_LFMT_R32_G32_INT                                 ),
   FMT  (R32G32,             SFLOAT,              CLASS_64,                       GFX_LFMT_R32_G32_FLOAT                               ),
   FMT  (R32G32B32,          UINT,                CLASS_96,                       GFX_LFMT_R32_G32_B32_UINT                            ),
   FMT  (R32G32B32,          SINT,                CLASS_96,                       GFX_LFMT_R32_G32_B32_INT                             ),
   FMT  (R32G32B32,          SFLOAT,              CLASS_96,                       GFX_LFMT_R32_G32_B32_FLOAT                           ),
   FMT  (R32G32B32A32,       UINT,                CLASS_128,                      GFX_LFMT_R32_G32_B32_A32_UINT                        ),
   FMT  (R32G32B32A32,       SINT,                CLASS_128,                      GFX_LFMT_R32_G32_B32_A32_INT                         ),
   FMT  (R32G32B32A32,       SFLOAT,              CLASS_128,                      GFX_LFMT_R32_G32_B32_A32_FLOAT                       ),
   FMT  (R64,                UINT,                CLASS_64,                       GFX_LFMT_NONE                                        ),
   FMT  (R64,                SINT,                CLASS_64,                       GFX_LFMT_NONE                                        ),
   FMT  (R64,                SFLOAT,              CLASS_64,                       GFX_LFMT_NONE                                        ),
   FMT  (R64G64,             UINT,                CLASS_128,                      GFX_LFMT_NONE                                        ),
   FMT  (R64G64,             SINT,                CLASS_128,                      GFX_LFMT_NONE                                        ),
   FMT  (R64G64,             SFLOAT,              CLASS_128,                      GFX_LFMT_NONE                                        ),
   FMT  (R64G64B64,          UINT,                CLASS_192,                      GFX_LFMT_NONE                                        ),
   FMT  (R64G64B64,          SINT,                CLASS_192,                      GFX_LFMT_NONE                                        ),
   FMT  (R64G64B64,          SFLOAT,              CLASS_192,                      GFX_LFMT_NONE                                        ),
   FMT  (R64G64B64A64,       UINT,                CLASS_256,                      GFX_LFMT_NONE                                        ),
   FMT  (R64G64B64A64,       SINT,                CLASS_256,                      GFX_LFMT_NONE                                        ),
   FMT  (R64G64B64A64,       SFLOAT,              CLASS_256,                      GFX_LFMT_NONE                                        ),
   SFMT (B10G11R11,          UFLOAT,              CLASS_32,            _PACK32,   GFX_LFMT_R11G11B10_UFLOAT                            ),
   SFMT (E5B9G9R9,           UFLOAT,              CLASS_32,            _PACK32,   GFX_LFMT_R9G9B9SHAREDEXP5_UFLOAT                     ),
   FMT  (D16,                UNORM,               CLASS_16,                       GFX_LFMT_D16_UNORM                                   ),
   SFMT (X8_D24,             UNORM,               CLASS_D24,           _PACK32,   GFX_LFMT_D24X8_UNORM                                 ),
   FMT  (D32,                SFLOAT,              CLASS_D32,                      GFX_LFMT_D32_FLOAT                                   ),
   FMT  (S8,                 UINT,                CLASS_S8,                       GFX_LFMT_NONE /*GFX_LFMT_S8_UINT*/                   ),
   FMT  (D16,                UNORM_S8_UINT,       CLASS_D16,                      GFX_LFMT_NONE                                        ),
   FMT  (D24,                UNORM_S8_UINT,       CLASS_D24,                      GFX_LFMT_S8D24_UINT_UNORM                            ),
   FMT  (D32,                SFLOAT_S8_UINT,      CLASS_D32,                      GFX_LFMT_D32_S8X24_FLOAT_UINT                        ),
   SFMT (BC1_RGB,            UNORM,               CLASS_BC1_RGB,       _BLOCK,    GFX_LFMT_BC1_RGBX_UNORM                              ),
   SFMT (BC1_RGB,            SRGB,                CLASS_BC1_RGB,       _BLOCK,    GFX_LFMT_BC1_RGBX_SRGB                               ),
   SFMT (BC1_RGBA,           UNORM,               CLASS_BC1_RGBA,      _BLOCK,    GFX_LFMT_BC1_RGBA_UNORM                              ),
   SFMT (BC1_RGBA,           SRGB,                CLASS_BC1_RGBA,      _BLOCK,    GFX_LFMT_BC1_RGBA_SRGB_SRGB_SRGB_UNORM               ),
   SFMT (BC2,                UNORM,               CLASS_BC2,           _BLOCK,    GFX_LFMT_BC2_RGBA_UNORM                              ),
   SFMT (BC2,                SRGB,                CLASS_BC2,           _BLOCK,    GFX_LFMT_BC2_RGBA_SRGB_SRGB_SRGB_UNORM               ),
   SFMT (BC3,                UNORM,               CLASS_BC3,           _BLOCK,    GFX_LFMT_BC3_RGBA_UNORM                              ),
   SFMT (BC3,                SRGB,                CLASS_BC3,           _BLOCK,    GFX_LFMT_BC3_RGBA_SRGB_SRGB_SRGB_UNORM               ),
   SFMT (BC4,                UNORM,               CLASS_BC4,           _BLOCK,    GFX_LFMT_NONE                                        ),
   SFMT (BC4,                SNORM,               CLASS_BC4,           _BLOCK,    GFX_LFMT_NONE                                        ),
   SFMT (BC5,                UNORM,               CLASS_BC5,           _BLOCK,    GFX_LFMT_NONE                                        ),
   SFMT (BC5,                SNORM,               CLASS_BC5,           _BLOCK,    GFX_LFMT_NONE                                        ),
   SFMT (BC6H,               UFLOAT,              CLASS_BC6H,          _BLOCK,    GFX_LFMT_NONE                                        ),
   SFMT (BC6H,               SFLOAT,              CLASS_BC6H,          _BLOCK,    GFX_LFMT_NONE                                        ),
   SFMT (BC7,                UNORM,               CLASS_BC7,           _BLOCK,    GFX_LFMT_NONE                                        ),
   SFMT (BC7,                SRGB,                CLASS_BC7,           _BLOCK,    GFX_LFMT_NONE                                        ),
   SFMT (ETC2_R8G8B8,        UNORM,               CLASS_ETC2_RGB,      _BLOCK,    GFX_LFMT_ETC2_RGB_UNORM                              ),
   SFMT (ETC2_R8G8B8,        SRGB,                CLASS_ETC2_RGB,      _BLOCK,    GFX_LFMT_ETC2_RGB_SRGB                               ),
   SFMT (ETC2_R8G8B8A1,      UNORM,               CLASS_ETC2_RGBA,     _BLOCK,    GFX_LFMT_PUNCHTHROUGH_ETC2_RGBA_UNORM                ),
   SFMT (ETC2_R8G8B8A1,      SRGB,                CLASS_ETC2_RGBA,     _BLOCK,    GFX_LFMT_PUNCHTHROUGH_ETC2_RGBA_SRGB_SRGB_SRGB_UNORM ),
   SFMT (ETC2_R8G8B8A8,      UNORM,               CLASS_ETC2_EAC_RGBA, _BLOCK,    GFX_LFMT_ETC2_EAC_RGBA_UNORM                         ),
   SFMT (ETC2_R8G8B8A8,      SRGB,                CLASS_ETC2_EAC_RGBA, _BLOCK,    GFX_LFMT_ETC2_EAC_RGBA_SRGB_SRGB_SRGB_UNORM          ),
   SFMT (EAC_R11,            UNORM,               CLASS_EAC_R,         _BLOCK,    GFX_LFMT_EAC_R_UNORM                                 ),
   SFMT (EAC_R11,            SNORM,               CLASS_EAC_R,         _BLOCK,    GFX_LFMT_EAC_R_SNORM                                 ),
   SFMT (EAC_R11G11,         UNORM,               CLASS_EAC_RG,        _BLOCK,    GFX_LFMT_EAC_EAC_RG_UNORM                            ),
   SFMT (EAC_R11G11,         SNORM,               CLASS_EAC_RG,        _BLOCK,    GFX_LFMT_EAC_EAC_RG_SNORM                            ),
   SFMT (ASTC_4x4,           UNORM,               CLASS_ASTC_4x4,      _BLOCK,    GFX_LFMT_ASTC4X4_RGBA_UNORM                          ),
   SFMT (ASTC_4x4,           SRGB,                CLASS_ASTC_4x4,      _BLOCK,    GFX_LFMT_ASTC4X4_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   SFMT (ASTC_5x4,           UNORM,               CLASS_ASTC_5x4,      _BLOCK,    GFX_LFMT_ASTC5X4_RGBA_UNORM                          ),
   SFMT (ASTC_5x4,           SRGB,                CLASS_ASTC_5x4,      _BLOCK,    GFX_LFMT_ASTC5X4_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   SFMT (ASTC_5x5,           UNORM,               CLASS_ASTC_5x5,      _BLOCK,    GFX_LFMT_ASTC5X5_RGBA_UNORM                          ),
   SFMT (ASTC_5x5,           SRGB,                CLASS_ASTC_5x5,      _BLOCK,    GFX_LFMT_ASTC5X5_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   SFMT (ASTC_6x5,           UNORM,               CLASS_ASTC_6x5,      _BLOCK,    GFX_LFMT_ASTC6X5_RGBA_UNORM                          ),
   SFMT (ASTC_6x5,           SRGB,                CLASS_ASTC_6x5,      _BLOCK,    GFX_LFMT_ASTC6X5_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   SFMT (ASTC_6x6,           UNORM,               CLASS_ASTC_6x6,      _BLOCK,    GFX_LFMT_ASTC6X6_RGBA_UNORM                          ),
   SFMT (ASTC_6x6,           SRGB,                CLASS_ASTC_6x6,      _BLOCK,    GFX_LFMT_ASTC6X6_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   SFMT (ASTC_8x5,           UNORM,               CLASS_ASTC_8x5,      _BLOCK,    GFX_LFMT_ASTC8X5_RGBA_UNORM                          ),
   SFMT (ASTC_8x5,           SRGB,                CLASS_ASTC_8x5,      _BLOCK,    GFX_LFMT_ASTC8X5_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   SFMT (ASTC_8x6,           UNORM,               CLASS_ASTC_8x6,      _BLOCK,    GFX_LFMT_ASTC8X6_RGBA_UNORM                          ),
   SFMT (ASTC_8x6,           SRGB,                CLASS_ASTC_8x6,      _BLOCK,    GFX_LFMT_ASTC8X6_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   SFMT (ASTC_8x8,           UNORM,               CLASS_ASTC_8x8,      _BLOCK,    GFX_LFMT_ASTC8X8_RGBA_UNORM                          ),
   SFMT (ASTC_8x8,           SRGB,                CLASS_ASTC_8x8,      _BLOCK,    GFX_LFMT_ASTC8X8_RGBA_SRGB_SRGB_SRGB_UNORM           ),
   SFMT (ASTC_10x5,          UNORM,               CLASS_ASTC_10x5,     _BLOCK,    GFX_LFMT_ASTC10X5_RGBA_UNORM                         ),
   SFMT (ASTC_10x5,          SRGB,                CLASS_ASTC_10x5,     _BLOCK,    GFX_LFMT_ASTC10X5_RGBA_SRGB_SRGB_SRGB_UNORM          ),
   SFMT (ASTC_10x6,          UNORM,               CLASS_ASTC_10x6,     _BLOCK,    GFX_LFMT_ASTC10X6_RGBA_UNORM                         ),
   SFMT (ASTC_10x6,          SRGB,                CLASS_ASTC_10x6,     _BLOCK,    GFX_LFMT_ASTC10X6_RGBA_SRGB_SRGB_SRGB_UNORM          ),
   SFMT (ASTC_10x8,          UNORM,               CLASS_ASTC_10x8,     _BLOCK,    GFX_LFMT_ASTC10X8_RGBA_UNORM                         ),
   SFMT (ASTC_10x8,          SRGB,                CLASS_ASTC_10x8,     _BLOCK,    GFX_LFMT_ASTC10X8_RGBA_SRGB_SRGB_SRGB_UNORM          ),
   SFMT (ASTC_10x10,         UNORM,               CLASS_ASTC_10x10,    _BLOCK,    GFX_LFMT_ASTC10X10_RGBA_UNORM                        ),
   SFMT (ASTC_10x10,         SRGB,                CLASS_ASTC_10x10,    _BLOCK,    GFX_LFMT_ASTC10X10_RGBA_SRGB_SRGB_SRGB_UNORM         ),
   SFMT (ASTC_12x10,         UNORM,               CLASS_ASTC_12x10,    _BLOCK,    GFX_LFMT_ASTC12X10_RGBA_UNORM                        ),
   SFMT (ASTC_12x10,         SRGB,                CLASS_ASTC_12x10,    _BLOCK,    GFX_LFMT_ASTC12X10_RGBA_SRGB_SRGB_SRGB_UNORM         ),
   SFMT (ASTC_12x12,         UNORM,               CLASS_ASTC_12x12,    _BLOCK,    GFX_LFMT_ASTC12X12_RGBA_UNORM                        ),
   SFMT (ASTC_12x12,         SRGB,                CLASS_ASTC_12x12,    _BLOCK,    GFX_LFMT_ASTC12X12_RGBA_SRGB_SRGB_SRGB_UNORM         ),
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

bool Formats::HasTMUSupport(VkFormat fmt)
{
   if (HasDepth(fmt) && HasStencil(fmt))
      return false;

   GFX_LFMT_TMU_TRANSLATION_T t;
   if (!gfx_lfmt_maybe_translate_tmu(&t, gfx_lfmt_ds_to_red(m_fmts[fmt].hwFmt)
#if !V3D_HAS_TMU_R32F_R16_SHAD
      , /*need_depth_type=*/HasDepth(fmt)
#endif
      ))
      return false;

#if V3D_HAS_TMU_R32F_R16_SHAD
   if (HasDepth(fmt) && !v3d_tmu_type_supports_shadow(t.type))
      return false;
#endif

   return true;
}

bool Formats::HasTLBSupport(VkFormat fmt)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_pixel_format_t pf;
   bool rev, swap;
   return gfx_lfmt_maybe_translate_pixel_format(m_fmts[fmt].hwFmt, &pf, &rev, &swap);
#else
   v3d_pixel_format_t pf = gfx_lfmt_maybe_translate_pixel_format(m_fmts[fmt].hwFmt);
   return pf != V3D_PIXEL_FORMAT_INVALID;
#endif
}

v3d_attr_type_t Formats::GetAttributeType(VkFormat fmt)
{
   v3d_attr_type_t type = V3D_ATTR_TYPE_INVALID;

   switch (gfx_lfmt_red_bits(m_fmts[fmt].naturalFmt))
   {
   case 8 :
      type = V3D_ATTR_TYPE_BYTE;
      break;
   case 10:
      type = V3D_ATTR_TYPE_INT2_10_10_10;
      break;
   case 16 :
      if (IsInteger(fmt) || IsNormalized(fmt) || IsScaled(fmt))
         type = V3D_ATTR_TYPE_SHORT;
      else
         type = V3D_ATTR_TYPE_HALF_FLOAT;
      break;
   case 32:
      if (IsInteger(fmt) || IsNormalized(fmt) || IsScaled(fmt))
         type = V3D_ATTR_TYPE_INT;
      else
         type = V3D_ATTR_TYPE_FLOAT;
      break;
   default:
      unreachable();
   }

   return type;
}

// Make one so that the constructor (which checks the array) will be called
static Formats s_format;

} // namespace bvk
