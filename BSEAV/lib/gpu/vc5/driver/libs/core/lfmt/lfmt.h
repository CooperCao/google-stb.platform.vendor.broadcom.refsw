/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GFX_LFMT_H
#define GFX_LFMT_H

#include "libs/core/v3d/v3d_common.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "vcos_string.h"

EXTERN_C_BEGIN

/** Some terminology */

/* An _element_ is a pixel/voxel/texel.
 *
 * An element can have many _slots_. Each slot can have a _type_, which defines
 * how the slot's bits are interpreted (eg integer or float), and can be mapped
 * to a _channel_ (eg red or blue).
 *
 * A _block_ is defined as the smallest n-dimensional rectangle of elements
 * with the constraint that each block is independently represented by an
 * integer number of bytes.
 *
 * So for most formats, a block is just a single element. For YUV formats, a
 * block is often a couple of elements due to chroma subsampling. For the DXTC
 * compressed formats, a block is 4x4 elements. For 1 bpp formats, blocks are
 * 8x1 as blocks must be an integral number of bytes in size.
 *
 * Conceputally, a _buffer_ is an n-dimensional array of elements.
 *
 * In practice, a buffer is made up of blocks which are laid out in memory as
 * defined by the buffer's _layout_.
 *
 * The _format_ of a buffer determines:
 * - the width/height/depth of a block in elements
 * - the size of a block in bytes
 * - how the elements within a block are encoded as bytes
 * - the type & channel of slots within each element
 *
 * Note that a buffer's width/height/depth are always specified in elements,
 * unless explicitly stated otherwise. Note also that the width/height/depth in
 * elements may not be a multiple of the block width/height/depth. In that
 * case, when calculating the width/height/depth in blocks, we always round up
 * and the extra elements are ignored. */

/** Utiles */

/* A utile is always 64 bytes. The width/height in blocks of a utile depends
 * only on the block size in bytes and whether or not the base format is C1 --
 * see gfx_lfmt_base_detail().
 *
 * Within a utile, blocks are stored in raster order.
 *
 * Lineartile swizzling is just utiles in raster order. */

/** UIF */

/* A UIF-block is always 256 bytes and formed from 2x2 utiles in raster order.
 *
 * UIF is made up of 4-UIF-block-wide columns. Within a column, UIF-blocks are
 * stored in raster order.
 *
 * UB-linear (which isn't strictly UIF) is just UIF-blocks in raster order.
 *
 * See http://confluence.broadcom.com/x/a6TtB for more information about UIF. */

/** Sand video */

/* Sand is made up of 128/256-byte wide columns. Within a column, blocks are
 * stored in raster order. */

/** 3D */

/* Currently, all 3D buffers are just formed from multiple independent 2D
 * slices. */

/** Variable naming */

/* block_w/block_h/block_d are dimensions of block in elements
 * ut_w_in_blocks/ut_h_in_blocks/etc are dimensions of utile in blocks */

/** Layout & format definition */

/* The number of dimensions, layout, and format of a buffer are defined by a
 * GFX_LFMT_T value, which has 7 semi-orthogonal fields:
 * - dims
 * - swizzling \ Layout
 * - yflip     /
 * - base (short for base format) \
 * - type                         | Format
 * - channels                     /
 *
 * The dims field indicates how many dimensions the buffer has; so 1D, 2D, or
 * 3D.
 *
 * The swizzling field indicates the swizzling rule used for calculating the
 * address of a block given its coordinates.
 *
 * The yflip field is a single bit. If the bit is set, then the y coordinate of
 * a block is flipped (y_in_blocks = height_in_blocks - y_in_blocks - 1) before
 * applying the swizzling rule. Note that inverting the yflip bit is not
 * necessarily equivalent to vertically flipping the buffer! See
 * http://confluence.broadcom.com/x/54SCBg for why we need this field.
 *
 * The base field indicates how the bytes within a block are interpreted as
 * elements/slots. Note that the size of a block in bytes as well as in
 * elements is fully determined by the base field.
 *
 * The type field gives types to the slots defined by the base field.
 *
 * The channels field maps slots defined by the base field onto channels. */

/* BEGIN AUTO-GENERATED CODE (enum_dims) */
#define GFX_LFMT_DIMS_SHIFT 0
#define GFX_LFMT_DIMS_BITS 2
#define GFX_LFMT_DIMS_MASK (0x3 << 0)

typedef enum gfx_lfmt_dims
{
   GFX_LFMT_DIMS_NONE = 0 << GFX_LFMT_DIMS_SHIFT,
   GFX_LFMT_DIMS_1D   = 1 << GFX_LFMT_DIMS_SHIFT,
   GFX_LFMT_DIMS_2D   = 2 << GFX_LFMT_DIMS_SHIFT,
   GFX_LFMT_DIMS_3D   = 3 << GFX_LFMT_DIMS_SHIFT,
} GFX_LFMT_DIMS_T;
/* END AUTO-GENERATED CODE (enum_dims) */

/* BEGIN AUTO-GENERATED CODE (enum_swizzling) */
#define GFX_LFMT_SWIZZLING_SHIFT 2
#define GFX_LFMT_SWIZZLING_BITS 4
#define GFX_LFMT_SWIZZLING_MASK (0xf << 2)

typedef enum gfx_lfmt_swizzling
{
   GFX_LFMT_SWIZZLING_NONE                 = 0 << GFX_LFMT_SWIZZLING_SHIFT,

   /* Valid for 1D, 2D, and 3D */
   GFX_LFMT_SWIZZLING_RSO                  = 1 << GFX_LFMT_SWIZZLING_SHIFT, /* Raster scan order */

   /* Valid for 2D & 3D. For 3D, slices are essentially independent 2D buffers, separated by slice_pitch */
   GFX_LFMT_SWIZZLING_LT                   = 2 << GFX_LFMT_SWIZZLING_SHIFT, /* Lineartile */
   GFX_LFMT_SWIZZLING_UIF                  = 3 << GFX_LFMT_SWIZZLING_SHIFT, /* Unified image format, no XORing */
   GFX_LFMT_SWIZZLING_UIF_XOR              = 4 << GFX_LFMT_SWIZZLING_SHIFT, /* XOR in odd columns */
   GFX_LFMT_SWIZZLING_UBLINEAR             = 5 << GFX_LFMT_SWIZZLING_SHIFT, /* UIF-blocks in raster order */

   /* Only valid for 2D */
   GFX_LFMT_SWIZZLING_SAND_128_MAP2        = 6 << GFX_LFMT_SWIZZLING_SHIFT, /* 128 byte wide stripes, DRAM MAP 2.0 */
   GFX_LFMT_SWIZZLING_SAND_128_MAP5        = 7 << GFX_LFMT_SWIZZLING_SHIFT, /* 128 byte wide stripes, DRAM MAP 5.0 */
   GFX_LFMT_SWIZZLING_SAND_256_MAP2        = 8 << GFX_LFMT_SWIZZLING_SHIFT, /* 256 byte wide stripes, DRAM MAP 2.0 */
   GFX_LFMT_SWIZZLING_SAND_256_MAP5        = 9 << GFX_LFMT_SWIZZLING_SHIFT, /* 256 byte wide stripes, DRAM MAP 5.0 */
   /* Big endian (see GFXH-1344) */
   GFX_LFMT_SWIZZLING_SAND_128_MAP2_BIGEND = 10 << GFX_LFMT_SWIZZLING_SHIFT, /* 128 byte wide stripes, DRAM MAP 2.0 */
   GFX_LFMT_SWIZZLING_SAND_128_MAP5_BIGEND = 11 << GFX_LFMT_SWIZZLING_SHIFT, /* 128 byte wide stripes, DRAM MAP 5.0 */
   GFX_LFMT_SWIZZLING_SAND_256_MAP2_BIGEND = 12 << GFX_LFMT_SWIZZLING_SHIFT, /* 256 byte wide stripes, DRAM MAP 2.0 */
   GFX_LFMT_SWIZZLING_SAND_256_MAP5_BIGEND = 13 << GFX_LFMT_SWIZZLING_SHIFT, /* 256 byte wide stripes, DRAM MAP 5.0 */
   GFX_LFMT_SWIZZLING_SAND_256_MAP8_BIGEND = 14 << GFX_LFMT_SWIZZLING_SHIFT, /* 256 byte wide stripes, DRAM MAP 8.0 */
} GFX_LFMT_SWIZZLING_T;
/* END AUTO-GENERATED CODE (enum_swizzling) */

/* BEGIN AUTO-GENERATED CODE (enum_yflip) */
#define GFX_LFMT_YFLIP_SHIFT 6
#define GFX_LFMT_YFLIP_BITS 1
#define GFX_LFMT_YFLIP_MASK (0x1 << 6)

typedef enum gfx_lfmt_yflip
{
   GFX_LFMT_YFLIP_NOYFLIP = 0 << GFX_LFMT_YFLIP_SHIFT,
   GFX_LFMT_YFLIP_YFLIP   = 1 << GFX_LFMT_YFLIP_SHIFT,
} GFX_LFMT_YFLIP_T;
/* END AUTO-GENERATED CODE (enum_yflip) */

/* Standard base formats are defined solely in terms of slots and padding...
 *
 * CB means a slot of size B (bits).
 * XB means B bits of padding.
 *
 * CACBCC means three slots of sizes A/B/C (bits) packed into a word of size
 * A+B+C (bits) with A occupying the least significant bits and C occupying
 * the most significant bits.
 *
 * CACB_CACB_CACB means three words of size A+B (bits) stored consecutively
 * in memory with the leftmost word having the lowest address.
 *
 * Note that C8_C8 differs from C8C8 in two ways:
 * - C8_C8 is composed of two 1-byte words rather than a single 2-byte word
 *   and so has looser alignment requirements
 * - the bytes in C8C8 are swapped on big-endian systems */

/* BEGIN AUTO-GENERATED CODE (enum_base) */
#define GFX_LFMT_BASE_SHIFT 7
#define GFX_LFMT_BASE_BITS 7
#define GFX_LFMT_BASE_MASK (0x7f << 7)

typedef enum gfx_lfmt_base
{
   GFX_LFMT_BASE_NONE                    = 0 << GFX_LFMT_BASE_SHIFT,

   GFX_LFMT_BASE_C32_C32_C32_C32         = 1 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C32_C32_C32             = 2 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C32_C32                 = 3 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C32                     = 4 << GFX_LFMT_BASE_SHIFT,

   GFX_LFMT_BASE_C16C16C16C16            = 5 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C16C16_C16C16           = 6 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C16_C16_C16_C16         = 7 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C16_C16_C16             = 8 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C16C16                  = 9 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C16_C16                 = 10 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C16                     = 11 << GFX_LFMT_BASE_SHIFT,

   GFX_LFMT_BASE_C8C8C8C8                = 12 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C8_C8_C8_C8             = 13 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C8_C8_C8                = 14 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C8C8                    = 15 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C8_C8                   = 16 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C8                      = 17 << GFX_LFMT_BASE_SHIFT,

   /* Blocks are 2 elements wide */
   GFX_LFMT_BASE_C4                      = 18 << GFX_LFMT_BASE_SHIFT, /* 1st element occupies 4 lsbs (little-endian style) */
   GFX_LFMT_BASE_C4BE                    = 19 << GFX_LFMT_BASE_SHIFT, /* 1st element occupies 4 msbs (big-endian style) */
   GFX_LFMT_BASE_C4X4                    = 20 << GFX_LFMT_BASE_SHIFT, /* Not supported by HW; used internally by gfx_buffer */

   GFX_LFMT_BASE_C1                      = 21 << GFX_LFMT_BASE_SHIFT, /* Blocks are 8 elements wide; 1st element occupies lsb */
   GFX_LFMT_BASE_C1X7                    = 22 << GFX_LFMT_BASE_SHIFT, /* Not supported by HW; used internally by gfx_buffer */

   GFX_LFMT_BASE_C10C10C10C2             = 23 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C11C11C10               = 24 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C5C6C5                  = 25 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C5C5C5C1                = 26 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C1C5C5C5                = 27 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C4C4C4C4                = 28 << GFX_LFMT_BASE_SHIFT,

   GFX_LFMT_BASE_C32_C8X24               = 29 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C24C8                   = 30 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_C8C24                   = 31 << GFX_LFMT_BASE_SHIFT,

   GFX_LFMT_BASE_C15X1_C15X1_C15X1_C15X1 = 32 << GFX_LFMT_BASE_SHIFT, /* TMU blend format for SNORM8 with output_type=32 */

   GFX_LFMT_BASE_C9C9C9SHAREDEXP5        = 33 << GFX_LFMT_BASE_SHIFT, /* Special base for 9_9_9_5 shared exponent */

   GFX_LFMT_BASE_C8_2X2                  = 34 << GFX_LFMT_BASE_SHIFT, /* 3-plane YUV U or V plane 4:2:0, for example YV12 */
   GFX_LFMT_BASE_C8_C8_2X2               = 35 << GFX_LFMT_BASE_SHIFT, /* 2-plane YUV chroma plane 4:2:0 */
   GFX_LFMT_BASE_C8_C8_2X1               = 36 << GFX_LFMT_BASE_SHIFT, /* 2-plane YUV chroma plane 4:2:2 */
   GFX_LFMT_BASE_C8_C8_C8_C8_2X1         = 37 << GFX_LFMT_BASE_SHIFT, /* 1-plane YUV 4:2:2 */

   /* 10-bit YUV */
   GFX_LFMT_BASE_C10                     = 38 << GFX_LFMT_BASE_SHIFT, /* Block is 3 Y values packed into a 32-bit word like y0<<20|y1<<10|y2 */
   GFX_LFMT_BASE_C10C10_2X2              = 39 << GFX_LFMT_BASE_SHIFT, /* Block is 3 UV pairs packed into 2 32-bit words like u0<<20|v0<<10|u1, v1<<20|u2<<10|v2 for the UV ordering */

   /* D3D block compressed formats, AKA S3TC/DXTC */
   GFX_LFMT_BASE_BC1                     = 40 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_BC2                     = 41 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_BC3                     = 42 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_BC4                     = 43 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_BC5                     = 44 << GFX_LFMT_BASE_SHIFT,

   GFX_LFMT_BASE_EAC                     = 45 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_EAC_EAC                 = 46 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ETC2                    = 47 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_PUNCHTHROUGH_ETC2       = 48 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ETC2_EAC                = 49 << GFX_LFMT_BASE_SHIFT,

   GFX_LFMT_BASE_ASTC4X4                 = 50 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC5X4                 = 51 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC5X5                 = 52 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC6X5                 = 53 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC6X6                 = 54 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC8X5                 = 55 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC8X6                 = 56 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC8X8                 = 57 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC10X5                = 58 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC10X6                = 59 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC10X8                = 60 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC10X10               = 61 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC12X10               = 62 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_ASTC12X12               = 63 << GFX_LFMT_BASE_SHIFT,

   GFX_LFMT_BASE_BSTC                    = 64 << GFX_LFMT_BASE_SHIFT, /* Broadcom Standard Texture Compression */
   GFX_LFMT_BASE_BSTCYFLIP               = 65 << GFX_LFMT_BASE_SHIFT, /* BSTC with y-flipped blocks */

   /* gl es1.1 paletted formats. note the P4BE's have 2 element wide blocks with element 0 in the 4 msbs (big-endian style) */
   GFX_LFMT_BASE_P4BE_R8G8B8             = 66 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_P4BE_R8G8B8A8           = 67 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_P4BE_B5G6R5             = 68 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_P4BE_A4B4G4R4           = 69 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_P4BE_A1B5G5R5           = 70 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_P8_R8G8B8               = 71 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_P8_R8G8B8A8             = 72 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_P8_B5G6R5               = 73 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_P8_A4B4G4R4             = 74 << GFX_LFMT_BASE_SHIFT,
   GFX_LFMT_BASE_P8_A1B5G5R5             = 75 << GFX_LFMT_BASE_SHIFT,
} GFX_LFMT_BASE_T;
/* END AUTO-GENERATED CODE (enum_base) */

/* Types apply to slots in textual order, eg:
 * C32_C8X24 | FLOAT_UINT = FLOAT32_UINT8X24 */

/* BEGIN AUTO-GENERATED CODE (enum_type) */
#define GFX_LFMT_TYPE_SHIFT 14
#define GFX_LFMT_TYPE_BITS 4
#define GFX_LFMT_TYPE_MASK (0xf << 14)

typedef enum gfx_lfmt_type
{
   GFX_LFMT_TYPE_NONE                 = 0 << GFX_LFMT_TYPE_SHIFT,

   /* all slots have the same type */
   GFX_LFMT_TYPE_UFLOAT               = 1 << GFX_LFMT_TYPE_SHIFT, /* unsigned floating-point (5.11 for 16-bit) */
   GFX_LFMT_TYPE_FLOAT                = 2 << GFX_LFMT_TYPE_SHIFT, /* signed floating-point (1.8.23 for 32-bit, 1.5.10 for 16-bit) */
   GFX_LFMT_TYPE_UINT                 = 3 << GFX_LFMT_TYPE_SHIFT, /* unsigned integer */
   GFX_LFMT_TYPE_INT                  = 4 << GFX_LFMT_TYPE_SHIFT, /* signed 2's complement integer */
   GFX_LFMT_TYPE_UNORM                = 5 << GFX_LFMT_TYPE_SHIFT, /* normalised unsigned integer [0, 1] */
   GFX_LFMT_TYPE_SRGB                 = 6 << GFX_LFMT_TYPE_SHIFT, /* like UNORM, but in sRGB space instead of linear space */
   GFX_LFMT_TYPE_SNORM                = 7 << GFX_LFMT_TYPE_SHIFT, /* normalised signed 2's complement integer [-1, 1] */

   /* slots have differing types */
   GFX_LFMT_TYPE_FLOAT_UINT           = 8 << GFX_LFMT_TYPE_SHIFT,
   GFX_LFMT_TYPE_UINT_FLOAT           = 9 << GFX_LFMT_TYPE_SHIFT,
   GFX_LFMT_TYPE_FLOAT_UNORM          = 10 << GFX_LFMT_TYPE_SHIFT,
   GFX_LFMT_TYPE_UNORM_FLOAT          = 11 << GFX_LFMT_TYPE_SHIFT,
   GFX_LFMT_TYPE_UNORM_UINT           = 12 << GFX_LFMT_TYPE_SHIFT,
   GFX_LFMT_TYPE_UINT_UNORM           = 13 << GFX_LFMT_TYPE_SHIFT,
   GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM = 14 << GFX_LFMT_TYPE_SHIFT,
   GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB = 15 << GFX_LFMT_TYPE_SHIFT,
} GFX_LFMT_TYPE_T;
/* END AUTO-GENERATED CODE (enum_type) */

/* Channels map to slots in textual order, eg:
 * C8_C8_C8_C8 | RGBX = R8_G8_B8_X8 */

/* BEGIN AUTO-GENERATED CODE (enum_channels) */
#define GFX_LFMT_CHANNELS_SHIFT 18
#define GFX_LFMT_CHANNELS_BITS 6
#define GFX_LFMT_CHANNELS_MASK (0x3f << 18)

typedef enum gfx_lfmt_channels
{
   GFX_LFMT_CHANNELS_NONE = 0 << GFX_LFMT_CHANNELS_SHIFT,

   GFX_LFMT_CHANNELS_R    = 1 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_G    = 2 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_B    = 3 << GFX_LFMT_CHANNELS_SHIFT, /* used for internal planar R32_G32_B32 formats */
   GFX_LFMT_CHANNELS_A    = 4 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_RG   = 5 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_GR   = 6 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_RA   = 7 << GFX_LFMT_CHANNELS_SHIFT, /* for CL_RA */
   GFX_LFMT_CHANNELS_AR   = 8 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_RGB  = 9 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_BGR  = 10 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_RGBA = 11 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_BGRA = 12 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_ARGB = 13 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_ABGR = 14 << GFX_LFMT_CHANNELS_SHIFT,

   GFX_LFMT_CHANNELS_X    = 15 << GFX_LFMT_CHANNELS_SHIFT, /* for planar R32_FLOAT_X8X24_TYPELESS or X32_TYPELESS_G8X24_UINT */
   GFX_LFMT_CHANNELS_RX   = 16 << GFX_LFMT_CHANNELS_SHIFT, /* for R32F_X8X24, CL_Rx */
   GFX_LFMT_CHANNELS_XR   = 17 << GFX_LFMT_CHANNELS_SHIFT, /* for DEPTH24_STENCIL8 in tmu in non-compare mode (it ignores stencil) */
   GFX_LFMT_CHANNELS_XG   = 18 << GFX_LFMT_CHANNELS_SHIFT, /* for X32_G8X24 */
   GFX_LFMT_CHANNELS_GX   = 19 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_RGX  = 20 << GFX_LFMT_CHANNELS_SHIFT, /* for CL_RGx */
   GFX_LFMT_CHANNELS_XGR  = 21 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_RGBX = 22 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_BGRX = 23 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_XRGB = 24 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_XBGR = 25 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_RXXX = 26 << GFX_LFMT_CHANNELS_SHIFT, /* for DEPTH24_STENCIL8 and depth_stencil_texture_mode = stencil index (ignores depth) */
   GFX_LFMT_CHANNELS_XXXR = 27 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_RAXX = 28 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_XXAR = 29 << GFX_LFMT_CHANNELS_SHIFT,

   /* luminance/alpha formats */
   GFX_LFMT_CHANNELS_L    = 30 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_LA   = 31 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_AL   = 32 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_LX   = 33 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_XL   = 34 << GFX_LFMT_CHANNELS_SHIFT,

   /* depth/stencil formats */
   GFX_LFMT_CHANNELS_D    = 35 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_S    = 36 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_DS   = 37 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_SD   = 38 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_DX   = 39 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_XD   = 40 << GFX_LFMT_CHANNELS_SHIFT, /* for DEPTH24_STENCIL8 in tmu (it ignores stencil) */
   GFX_LFMT_CHANNELS_SX   = 41 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_XS   = 42 << GFX_LFMT_CHANNELS_SHIFT,

   /* yuv formats */
   GFX_LFMT_CHANNELS_Y    = 43 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_U    = 44 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_V    = 45 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_UV   = 46 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_VU   = 47 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_YUV  = 48 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_VUY  = 49 << GFX_LFMT_CHANNELS_SHIFT,
   GFX_LFMT_CHANNELS_YUYV = 50 << GFX_LFMT_CHANNELS_SHIFT, /* v3d 2.2 YUYV422R = Y8_U8_Y8_V8 */
   GFX_LFMT_CHANNELS_VYUY = 51 << GFX_LFMT_CHANNELS_SHIFT, /* See TFU RGBORD */
   GFX_LFMT_CHANNELS_YYUV = 52 << GFX_LFMT_CHANNELS_SHIFT, /* See TFU RGBORD */
   GFX_LFMT_CHANNELS_VUYY = 53 << GFX_LFMT_CHANNELS_SHIFT, /* See TFU RGBORD */
} GFX_LFMT_CHANNELS_T;
/* END AUTO-GENERATED CODE (enum_channels) */

/* Macros for constructing a GFX_LFMT_T. For picking apart a GFX_LFMT_T,
 * just & with the appropriate mask defines, eg:
 * (GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP, BASE_BC1, TYPE_UNORM, CHANNELS_RGBA) & GFX_LFMT_BASE_MASK) == GFX_LFMT_BC1 */
#define GFX_LFMT_MAKE_NOCAST(DIMS, SWIZZLING, YFLIP, BASE, TYPE, CHANNELS) \
   (GFX_LFMT_##DIMS | GFX_LFMT_##SWIZZLING | GFX_LFMT_##YFLIP | GFX_LFMT_##BASE | GFX_LFMT_##TYPE | GFX_LFMT_##CHANNELS)
#define GFX_LFMT_MAKE(DIMS, SWIZZLING, YFLIP, BASE, TYPE, CHANNELS) \
   ((GFX_LFMT_T)GFX_LFMT_MAKE_NOCAST(DIMS, SWIZZLING, YFLIP, BASE, TYPE, CHANNELS))

#define GFX_LFMT_LAYOUT_MASK (GFX_LFMT_SWIZZLING_MASK | GFX_LFMT_YFLIP_MASK)
#define GFX_LFMT_FORMAT_MASK (GFX_LFMT_BASE_MASK | GFX_LFMT_TYPE_MASK | GFX_LFMT_CHANNELS_MASK)

typedef enum gfx_lfmt
{
   GFX_LFMT_NONE = 0, /* For use as a sentinel */

   /* A bunch of explicitly defined enum values */
#include "lfmt_nice_enum.h"
} GFX_LFMT_T;

/* Getters and setters for the fields inside GFX_LFMT_T */
/* BEGIN AUTO-GENERATED CODE (get_set_funcs) */
static inline GFX_LFMT_DIMS_T gfx_lfmt_get_dims(const GFX_LFMT_T *lfmt)
{ return (GFX_LFMT_DIMS_T)(*lfmt & GFX_LFMT_DIMS_MASK); }
static inline GFX_LFMT_T * gfx_lfmt_set_dims(GFX_LFMT_T *lfmt, GFX_LFMT_DIMS_T dims)
{ *lfmt = (GFX_LFMT_T)((*lfmt & ~GFX_LFMT_DIMS_MASK) | dims); return lfmt; }

static inline GFX_LFMT_SWIZZLING_T gfx_lfmt_get_swizzling(const GFX_LFMT_T *lfmt)
{ return (GFX_LFMT_SWIZZLING_T)(*lfmt & GFX_LFMT_SWIZZLING_MASK); }
static inline GFX_LFMT_T * gfx_lfmt_set_swizzling(GFX_LFMT_T *lfmt, GFX_LFMT_SWIZZLING_T swizzling)
{ *lfmt = (GFX_LFMT_T)((*lfmt & ~GFX_LFMT_SWIZZLING_MASK) | swizzling); return lfmt; }

static inline GFX_LFMT_YFLIP_T gfx_lfmt_get_yflip(const GFX_LFMT_T *lfmt)
{ return (GFX_LFMT_YFLIP_T)(*lfmt & GFX_LFMT_YFLIP_MASK); }
static inline GFX_LFMT_T * gfx_lfmt_set_yflip(GFX_LFMT_T *lfmt, GFX_LFMT_YFLIP_T yflip)
{ *lfmt = (GFX_LFMT_T)((*lfmt & ~GFX_LFMT_YFLIP_MASK) | yflip); return lfmt; }

static inline GFX_LFMT_BASE_T gfx_lfmt_get_base(const GFX_LFMT_T *lfmt)
{ return (GFX_LFMT_BASE_T)(*lfmt & GFX_LFMT_BASE_MASK); }
static inline GFX_LFMT_T * gfx_lfmt_set_base(GFX_LFMT_T *lfmt, GFX_LFMT_BASE_T base)
{ *lfmt = (GFX_LFMT_T)((*lfmt & ~GFX_LFMT_BASE_MASK) | base); return lfmt; }

static inline GFX_LFMT_TYPE_T gfx_lfmt_get_type(const GFX_LFMT_T *lfmt)
{ return (GFX_LFMT_TYPE_T)(*lfmt & GFX_LFMT_TYPE_MASK); }
static inline GFX_LFMT_T * gfx_lfmt_set_type(GFX_LFMT_T *lfmt, GFX_LFMT_TYPE_T type)
{ *lfmt = (GFX_LFMT_T)((*lfmt & ~GFX_LFMT_TYPE_MASK) | type); return lfmt; }

static inline GFX_LFMT_CHANNELS_T gfx_lfmt_get_channels(const GFX_LFMT_T *lfmt)
{ return (GFX_LFMT_CHANNELS_T)(*lfmt & GFX_LFMT_CHANNELS_MASK); }
static inline GFX_LFMT_T * gfx_lfmt_set_channels(GFX_LFMT_T *lfmt, GFX_LFMT_CHANNELS_T channels)
{ *lfmt = (GFX_LFMT_T)((*lfmt & ~GFX_LFMT_CHANNELS_MASK) | channels); return lfmt; }
/* END AUTO-GENERATED CODE (get_set_funcs) */

/** Get/parse human-readable strings describing lfmts */

/* Like vcos_safe_sprintf */
extern size_t gfx_lfmt_sprint(char *buf, size_t buf_size, size_t offset, GFX_LFMT_T lfmt);

#define GFX_LFMT_SPRINT(BUF_NAME, LFMT) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 256, gfx_lfmt_sprint, LFMT)

/* Returned pointer is only valid until next call... */
extern const char *gfx_lfmt_desc(GFX_LFMT_T lfmt);

extern bool gfx_lfmt_maybe_from_desc(GFX_LFMT_T *lfmt, const char *desc);
extern GFX_LFMT_T gfx_lfmt_from_desc(const char *desc);

/** Trivial functions for dealing with dims field */

static inline GFX_LFMT_DIMS_T gfx_lfmt_dims_to_enum(uint32_t dims)
{
   switch (dims)
   {
   case 1:  return GFX_LFMT_DIMS_1D;
   case 2:  return GFX_LFMT_DIMS_2D;
   case 3:  return GFX_LFMT_DIMS_3D;
   default: unreachable(); return GFX_LFMT_DIMS_NONE;
   }
}

static inline GFX_LFMT_T gfx_lfmt_to_1d(GFX_LFMT_T lfmt) { return *gfx_lfmt_set_dims(&lfmt, GFX_LFMT_DIMS_1D); }
static inline GFX_LFMT_T gfx_lfmt_to_2d(GFX_LFMT_T lfmt) { return *gfx_lfmt_set_dims(&lfmt, GFX_LFMT_DIMS_2D); }
static inline GFX_LFMT_T gfx_lfmt_to_3d(GFX_LFMT_T lfmt) { return *gfx_lfmt_set_dims(&lfmt, GFX_LFMT_DIMS_3D); }

static inline uint32_t gfx_lfmt_dims_from_enum(GFX_LFMT_DIMS_T dims)
{
   switch (dims)
   {
   case GFX_LFMT_DIMS_1D: return 1;
   case GFX_LFMT_DIMS_2D: return 2;
   case GFX_LFMT_DIMS_3D: return 3;
   default: unreachable(); return 0;
   }
}

static inline bool gfx_lfmt_is_1d(GFX_LFMT_T lfmt) { return gfx_lfmt_get_dims(&lfmt) == GFX_LFMT_DIMS_1D; }
static inline bool gfx_lfmt_is_2d(GFX_LFMT_T lfmt) { return gfx_lfmt_get_dims(&lfmt) == GFX_LFMT_DIMS_2D; }
static inline bool gfx_lfmt_is_3d(GFX_LFMT_T lfmt) { return gfx_lfmt_get_dims(&lfmt) == GFX_LFMT_DIMS_3D; }

/** Trivial functions for dealing with layout fields */

/* BEGIN AUTO-GENERATED CODE (to_is_swizzling_funcs) */
static inline GFX_LFMT_T gfx_lfmt_to_rso(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_RSO); }
static inline bool gfx_lfmt_is_rso(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_RSO; }

static inline GFX_LFMT_T gfx_lfmt_to_lt(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_LT); }
static inline bool gfx_lfmt_is_lt(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_LT; }

static inline GFX_LFMT_T gfx_lfmt_to_uif(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_UIF); }
static inline bool gfx_lfmt_is_uif(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_UIF; }

static inline GFX_LFMT_T gfx_lfmt_to_uif_xor(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_UIF_XOR); }
static inline bool gfx_lfmt_is_uif_xor(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_UIF_XOR; }

static inline GFX_LFMT_T gfx_lfmt_to_ublinear(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_UBLINEAR); }
static inline bool gfx_lfmt_is_ublinear(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_UBLINEAR; }

static inline GFX_LFMT_T gfx_lfmt_to_sand_128_map2(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_SAND_128_MAP2); }
static inline bool gfx_lfmt_is_sand_128_map2(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_SAND_128_MAP2; }

static inline GFX_LFMT_T gfx_lfmt_to_sand_128_map5(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_SAND_128_MAP5); }
static inline bool gfx_lfmt_is_sand_128_map5(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_SAND_128_MAP5; }

static inline GFX_LFMT_T gfx_lfmt_to_sand_256_map2(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_SAND_256_MAP2); }
static inline bool gfx_lfmt_is_sand_256_map2(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_SAND_256_MAP2; }

static inline GFX_LFMT_T gfx_lfmt_to_sand_256_map5(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_SAND_256_MAP5); }
static inline bool gfx_lfmt_is_sand_256_map5(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_SAND_256_MAP5; }

static inline GFX_LFMT_T gfx_lfmt_to_sand_128_map2_bigend(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_SAND_128_MAP2_BIGEND); }
static inline bool gfx_lfmt_is_sand_128_map2_bigend(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_SAND_128_MAP2_BIGEND; }

static inline GFX_LFMT_T gfx_lfmt_to_sand_128_map5_bigend(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_SAND_128_MAP5_BIGEND); }
static inline bool gfx_lfmt_is_sand_128_map5_bigend(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_SAND_128_MAP5_BIGEND; }

static inline GFX_LFMT_T gfx_lfmt_to_sand_256_map2_bigend(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_SAND_256_MAP2_BIGEND); }
static inline bool gfx_lfmt_is_sand_256_map2_bigend(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_SAND_256_MAP2_BIGEND; }

static inline GFX_LFMT_T gfx_lfmt_to_sand_256_map5_bigend(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_SAND_256_MAP5_BIGEND); }
static inline bool gfx_lfmt_is_sand_256_map5_bigend(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_SAND_256_MAP5_BIGEND; }

static inline GFX_LFMT_T gfx_lfmt_to_sand_256_map8_bigend(GFX_LFMT_T lfmt)
{ return *gfx_lfmt_set_swizzling(&lfmt, GFX_LFMT_SWIZZLING_SAND_256_MAP8_BIGEND); }
static inline bool gfx_lfmt_is_sand_256_map8_bigend(GFX_LFMT_T lfmt)
{ return gfx_lfmt_get_swizzling(&lfmt) == GFX_LFMT_SWIZZLING_SAND_256_MAP8_BIGEND; }
/* END AUTO-GENERATED CODE (to_is_swizzling_funcs) */

/** Misc helpers */

static inline GFX_LFMT_YFLIP_T gfx_lfmt_invert_yflip(GFX_LFMT_YFLIP_T yflip)
{
   return yflip ? GFX_LFMT_YFLIP_NOYFLIP : GFX_LFMT_YFLIP_YFLIP;
}

/* BEGIN AUTO-GENERATED CODE (misc_defines) */
#define GFX_LFMT_MAX_BYTES_PER_BLOCK 32
#define GFX_LFMT_CHAN_A_BIT 0x1
#define GFX_LFMT_CHAN_B_BIT 0x2
#define GFX_LFMT_CHAN_D_BIT 0x4
#define GFX_LFMT_CHAN_G_BIT 0x8
#define GFX_LFMT_CHAN_L_BIT 0x10
#define GFX_LFMT_CHAN_R_BIT 0x20
#define GFX_LFMT_CHAN_S_BIT 0x40
#define GFX_LFMT_CHAN_U_BIT 0x80
#define GFX_LFMT_CHAN_V_BIT 0x100
#define GFX_LFMT_CHAN_X_BIT 0x200
#define GFX_LFMT_CHAN_Y_BIT 0x400
/* END AUTO-GENERATED CODE (misc_defines) */

/* BEGIN AUTO-GENERATED CODE (misc_func_decls) */
extern bool gfx_lfmt_dims_and_swizzling_compatible(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_uif_family(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_sand_family(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_bigend_sand_family(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_compressed(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_paletted(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_etc_family(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_astc_family(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_bc_family(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_bstc_family(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_std(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_is_std_with_subsample(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_contains_srgb(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_contains_int(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_contains_int_signed(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_contains_int_unsigned(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_contains_float(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_contains_unorm(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_contains_snorm(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_present_channels(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_red(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_red_bits(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_green(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_green_bits(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_blue(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_blue_bits(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_alpha(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_alpha_bits(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_depth(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_depth_bits(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_stencil(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_stencil_bits(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_y(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_y_bits(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_u(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_u_bits(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_v(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_v_bits(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_x(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_x_bits(GFX_LFMT_T lfmt);
extern GFX_LFMT_TYPE_T gfx_lfmt_red_type(GFX_LFMT_T lfmt);
extern unsigned gfx_lfmt_red_index(GFX_LFMT_T lfmt);
extern GFX_LFMT_TYPE_T gfx_lfmt_green_type(GFX_LFMT_T lfmt);
extern unsigned gfx_lfmt_green_index(GFX_LFMT_T lfmt);
extern GFX_LFMT_TYPE_T gfx_lfmt_blue_type(GFX_LFMT_T lfmt);
extern unsigned gfx_lfmt_blue_index(GFX_LFMT_T lfmt);
extern GFX_LFMT_TYPE_T gfx_lfmt_alpha_type(GFX_LFMT_T lfmt);
extern unsigned gfx_lfmt_alpha_index(GFX_LFMT_T lfmt);
extern GFX_LFMT_TYPE_T gfx_lfmt_depth_type(GFX_LFMT_T lfmt);
extern unsigned gfx_lfmt_depth_index(GFX_LFMT_T lfmt);
extern GFX_LFMT_TYPE_T gfx_lfmt_stencil_type(GFX_LFMT_T lfmt);
extern unsigned gfx_lfmt_stencil_index(GFX_LFMT_T lfmt);
extern GFX_LFMT_TYPE_T gfx_lfmt_y_type(GFX_LFMT_T lfmt);
extern unsigned gfx_lfmt_y_index(GFX_LFMT_T lfmt);
extern GFX_LFMT_TYPE_T gfx_lfmt_u_type(GFX_LFMT_T lfmt);
extern unsigned gfx_lfmt_u_index(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_depth_to_x(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_stencil_to_x(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_alpha_to_x(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_color(GFX_LFMT_T lfmt); /* has any color channel (R, G, B, L, Y, U, V) */
extern bool gfx_lfmt_has_depth_stencil(GFX_LFMT_T lfmt);
extern bool gfx_lfmt_has_repeated_channel(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_reverse_channels(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_reverse_type(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_valid_chan_mask(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_num_slots_from_base(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_num_slots_from_type(GFX_LFMT_T lfmt);
extern uint32_t gfx_lfmt_num_slots_from_channels(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_drop_subsample_size(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_ds_to_l(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_ds_to_red(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_ds_to_rg(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_int_to_norm(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_snorm_to_unorm(GFX_LFMT_T lfmt);
extern GFX_LFMT_T gfx_lfmt_srgb_to_unorm(GFX_LFMT_T lfmt);
/* END AUTO-GENERATED CODE (misc_func_decls) */

static inline void gfx_lfmt_check_num_slots_eq(GFX_LFMT_T lfmt, uint32_t num_slots)
{
   assert(num_slots >= 1);

   if (gfx_lfmt_get_base(&lfmt) != GFX_LFMT_BASE_NONE)
      assert(gfx_lfmt_num_slots_from_base(lfmt) == num_slots);

   if (gfx_lfmt_get_type(&lfmt) != GFX_LFMT_TYPE_NONE)
   {
      uint32_t num_slots_from_type = gfx_lfmt_num_slots_from_type(lfmt);
      assert((num_slots_from_type == 1) || (num_slots_from_type == num_slots));
   }

   if (gfx_lfmt_get_channels(&lfmt) != GFX_LFMT_CHANNELS_NONE)
      assert(gfx_lfmt_num_slots_from_channels(lfmt) == num_slots);
}

static inline bool gfx_lfmt_all_planes(bool (*pred)(GFX_LFMT_T),
   uint32_t num_planes, const GFX_LFMT_T *lfmts)
{
   for (uint32_t i = 0; i != num_planes; ++i)
      if (!pred(lfmts[i]))
         return false;
   return true;
}

static inline bool gfx_lfmt_any_plane(bool (*pred)(GFX_LFMT_T),
   uint32_t num_planes, const GFX_LFMT_T *lfmts)
{
   for (uint32_t i = 0; i != num_planes; ++i)
      if (pred(lfmts[i]))
         return true;
   return false;
}

static inline bool gfx_lfmt_dims_and_layout_compatible(GFX_LFMT_T lfmt)
{
   if (gfx_lfmt_is_1d(lfmt) && gfx_lfmt_get_yflip(&lfmt))
      return false;
   return gfx_lfmt_dims_and_swizzling_compatible(lfmt);
}

static inline GFX_LFMT_T gfx_lfmt_set_format(GFX_LFMT_T to, GFX_LFMT_T from)
{
   return (GFX_LFMT_T)((to & ~GFX_LFMT_FORMAT_MASK) | (from & GFX_LFMT_FORMAT_MASK));
}

static inline GFX_LFMT_T gfx_lfmt_copy_layout_same_dims(GFX_LFMT_T to, GFX_LFMT_T from)
{
   assert(gfx_lfmt_get_dims(&to) == gfx_lfmt_get_dims(&from));
   return gfx_lfmt_set_format(from, to);
}

static inline GFX_LFMT_T gfx_lfmt_to_2d_rso(GFX_LFMT_T lfmt)
{
   return gfx_lfmt_to_2d(gfx_lfmt_to_rso(lfmt));
}

static inline unsigned gfx_lfmt_sandcol_w_in_bytes(GFX_LFMT_SWIZZLING_T swizzling)
{
   switch (swizzling)
   {
   case GFX_LFMT_SWIZZLING_SAND_128_MAP2:
   case GFX_LFMT_SWIZZLING_SAND_128_MAP5:
   case GFX_LFMT_SWIZZLING_SAND_128_MAP2_BIGEND:
   case GFX_LFMT_SWIZZLING_SAND_128_MAP5_BIGEND:   return 128;
   case GFX_LFMT_SWIZZLING_SAND_256_MAP2:
   case GFX_LFMT_SWIZZLING_SAND_256_MAP5:
   case GFX_LFMT_SWIZZLING_SAND_256_MAP2_BIGEND:
   case GFX_LFMT_SWIZZLING_SAND_256_MAP5_BIGEND:
   case GFX_LFMT_SWIZZLING_SAND_256_MAP8_BIGEND:   return 256;
   default:                                        unreachable(); return 0;
   }
}

static inline unsigned gfx_lfmt_dram_map_version(GFX_LFMT_SWIZZLING_T swizzling)
{
   switch (swizzling)
   {
   case GFX_LFMT_SWIZZLING_SAND_128_MAP2:
   case GFX_LFMT_SWIZZLING_SAND_256_MAP2:
   case GFX_LFMT_SWIZZLING_SAND_128_MAP2_BIGEND:
   case GFX_LFMT_SWIZZLING_SAND_256_MAP2_BIGEND:   return 2;
   case GFX_LFMT_SWIZZLING_SAND_128_MAP5:
   case GFX_LFMT_SWIZZLING_SAND_256_MAP5:
   case GFX_LFMT_SWIZZLING_SAND_128_MAP5_BIGEND:
   case GFX_LFMT_SWIZZLING_SAND_256_MAP5_BIGEND:   return 5;
   case GFX_LFMT_SWIZZLING_SAND_256_MAP8_BIGEND:   return 8;
   default:                                        unreachable(); return 0;
   }
}

static inline bool gfx_lfmt_pitch_is_vertical(GFX_LFMT_T lfmt)
{
   if (gfx_lfmt_is_sand_family(lfmt))
      return true;
   switch (gfx_lfmt_get_swizzling(&lfmt))
   {
   case GFX_LFMT_SWIZZLING_RSO:
   case GFX_LFMT_SWIZZLING_LT:
   case GFX_LFMT_SWIZZLING_UBLINEAR:
      return false;
   case GFX_LFMT_SWIZZLING_UIF:
   case GFX_LFMT_SWIZZLING_UIF_XOR:
      return true;
   default:
      unreachable();
      return false;
   }
}

static inline GFX_LFMT_T gfx_lfmt_fmt(GFX_LFMT_T lfmt)
{
   return (GFX_LFMT_T)(lfmt & GFX_LFMT_FORMAT_MASK);
}

static inline bool gfx_lfmt_fmts_equal(
   uint32_t lhs_num_lfmts, const GFX_LFMT_T *lhs_lfmts,
   uint32_t rhs_num_lfmts, const GFX_LFMT_T *rhs_lfmts)
{
   uint32_t i;

   if (lhs_num_lfmts != rhs_num_lfmts)
      return false;

   for (i = 0; i != lhs_num_lfmts; ++i)
      if (gfx_lfmt_fmt(lhs_lfmts[i]) != gfx_lfmt_fmt(rhs_lfmts[i]))
         return false;

   return true;
}

static inline bool gfx_lfmts_equal(
   uint32_t lhs_num_lfmts, const GFX_LFMT_T *lhs_lfmts,
   uint32_t rhs_num_lfmts, const GFX_LFMT_T *rhs_lfmts)
{
   uint32_t i;

   if (lhs_num_lfmts != rhs_num_lfmts)
      return false;

   for (i = 0; i != lhs_num_lfmts; ++i)
      if (lhs_lfmts[i] != rhs_lfmts[i])
         return false;

   return true;
}

static inline GFX_LFMT_T gfx_lfmt_fill_in_blanks(GFX_LFMT_T lfmt, GFX_LFMT_T reference)
{
   if (!gfx_lfmt_get_dims(&lfmt))      gfx_lfmt_set_dims(     &lfmt, gfx_lfmt_get_dims(&reference));
   if (!gfx_lfmt_get_swizzling(&lfmt)) gfx_lfmt_set_swizzling(&lfmt, gfx_lfmt_get_swizzling(&reference));
   if (!gfx_lfmt_get_base(&lfmt))      gfx_lfmt_set_base(     &lfmt, gfx_lfmt_get_base(&reference));
   if (!gfx_lfmt_get_type(&lfmt))      gfx_lfmt_set_type(     &lfmt, gfx_lfmt_get_type(&reference));
   if (!gfx_lfmt_get_channels(&lfmt))  gfx_lfmt_set_channels( &lfmt, gfx_lfmt_get_channels(&reference));
   return lfmt;
}

/** Base detail */

typedef struct
{
   uint32_t bytes_per_block;
   uint32_t block_w, block_h, block_d; /* Block dimensions */

   /* Dimensions of utiles (not always valid -- may be 0) */
   uint32_t ut_w_in_blocks_1d;
   /* TODO Actually valid for 3D as well as 2D -- utiles are never deeper than
    * 1 block? Ditto the UIF _2d stuff below */
   uint32_t ut_w_in_blocks_2d, ut_h_in_blocks_2d;

   // Dimensions of UIF-blocks (not always valid -- may be 0)
   uint32_t ub_w_in_blocks_2d, ub_h_in_blocks_2d;
} GFX_LFMT_BASE_DETAIL_T;

/* Only looks at base format -- other lfmt fields are ignored */
extern void gfx_lfmt_base_detail(GFX_LFMT_BASE_DETAIL_T *bd, GFX_LFMT_T lfmt);

static inline uint32_t gfx_lfmt_bytes_per_block(GFX_LFMT_T lfmt)
{
   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, lfmt);
   return bd.bytes_per_block;
}

static inline bool gfx_lfmt_block_dims_equal(GFX_LFMT_T lfmt,
   uint32_t w, uint32_t h, uint32_t d)
{
   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, lfmt);
   return (bd.block_w == w) && (bd.block_h == h) && (bd.block_d == d);
}

static inline bool gfx_lfmt_block_is_single_elem(GFX_LFMT_T lfmt)
{
   return gfx_lfmt_block_dims_equal(lfmt, 1, 1, 1);
}

/* utiles */

#define GFX_LFMT_UTILE_SIZE 64

static inline bool gfx_lfmt_have_ut_dims(const GFX_LFMT_BASE_DETAIL_T *bd)
{
   if (!bd->ut_w_in_blocks_1d) {
      assert(!bd->ut_w_in_blocks_2d);
      assert(!bd->ut_h_in_blocks_2d);
      return false;
   }
   assert(bd->ut_w_in_blocks_2d);
   assert(bd->ut_h_in_blocks_2d);
   return true;
}

static inline uint32_t gfx_lfmt_ut_w_2d(const GFX_LFMT_BASE_DETAIL_T *bd)
{
   return bd->ut_w_in_blocks_2d * bd->block_w;
}

static inline uint32_t gfx_lfmt_ut_h_2d(const GFX_LFMT_BASE_DETAIL_T *bd)
{
   return bd->ut_h_in_blocks_2d * bd->block_h;
}

/* uifblocks */

#define GFX_UIF_COL_W_IN_UB 4
#define GFX_UIF_UB_SIZE 256

static inline uint32_t gfx_lfmt_ub_w_2d(const GFX_LFMT_BASE_DETAIL_T *bd)
{
   return bd->ub_w_in_blocks_2d * bd->block_w;
}

static inline uint32_t gfx_lfmt_ub_h_2d(const GFX_LFMT_BASE_DETAIL_T *bd)
{
   return bd->ub_h_in_blocks_2d * bd->block_h;
}

/* uifcolumns */

static inline uint32_t gfx_lfmt_ucol_w_in_blocks_2d(const GFX_LFMT_BASE_DETAIL_T *bd)
{
   return bd->ub_w_in_blocks_2d * GFX_UIF_COL_W_IN_UB;
}

static inline uint32_t gfx_lfmt_ucol_w_2d(const GFX_LFMT_BASE_DETAIL_T *bd)
{
   return gfx_lfmt_ucol_w_in_blocks_2d(bd) * bd->block_w;
}

/* sand columns */

static inline uint32_t gfx_lfmt_sandcol_w_in_blocks_2d(
   const GFX_LFMT_BASE_DETAIL_T *bd, GFX_LFMT_SWIZZLING_T swizzling)
{
   return gfx_udiv_exactly(gfx_lfmt_sandcol_w_in_bytes(swizzling), bd->bytes_per_block);
}

static inline uint32_t gfx_lfmt_sandcol_w_2d(
   const GFX_LFMT_BASE_DETAIL_T *bd, GFX_LFMT_SWIZZLING_T swizzling)
{
   return gfx_lfmt_sandcol_w_in_blocks_2d(bd, swizzling) * bd->block_w;
}

/** Y-flip */

static inline bool gfx_lfmt_yflip_base_is_nop(GFX_LFMT_BASE_T base)
{
   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, (GFX_LFMT_T)base);
   if (bd.block_h == 1)
      return true;

   switch (base)
   {
   case GFX_LFMT_BASE_C8_C8_2X2:
   case GFX_LFMT_BASE_C8_2X2:
      /* All elements within a block the same */
      return true;
   default:
      return false;
   }
}

static inline GFX_LFMT_BASE_T gfx_lfmt_try_yflip_base(GFX_LFMT_BASE_T base)
{
   if (gfx_lfmt_yflip_base_is_nop(base))
      return base;

   switch (base)
   {
   case GFX_LFMT_BASE_BSTC:      return GFX_LFMT_BASE_BSTCYFLIP;
   case GFX_LFMT_BASE_BSTCYFLIP: return GFX_LFMT_BASE_BSTC;
   default:                      return GFX_LFMT_BASE_NONE;
   }
}

static inline bool gfx_lfmt_can_yflip_base(GFX_LFMT_BASE_T base)
{
   return gfx_lfmt_try_yflip_base(base) != GFX_LFMT_BASE_NONE;
}

static inline GFX_LFMT_BASE_T gfx_lfmt_yflip_base(GFX_LFMT_BASE_T base)
{
   base = gfx_lfmt_try_yflip_base(base);
   assert(base != GFX_LFMT_BASE_NONE);
   return base;
}

static inline bool gfx_lfmt_base_is_yflipped(GFX_LFMT_BASE_T base)
{
   return base == GFX_LFMT_BASE_BSTCYFLIP;
}

static inline bool gfx_lfmt_yflip_consistent(GFX_LFMT_T lfmt)
{
   GFX_LFMT_BASE_T base = gfx_lfmt_get_base(&lfmt);
   if (gfx_lfmt_get_yflip(&lfmt))
      return gfx_lfmt_yflip_base_is_nop(base) || gfx_lfmt_base_is_yflipped(base);
   else
      return !gfx_lfmt_base_is_yflipped(base);
}

EXTERN_C_END

#endif
