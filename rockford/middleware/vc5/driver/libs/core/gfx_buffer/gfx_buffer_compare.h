/*==============================================================================
 Broadcom Proprietary and Confidential. (c)2012 Broadcom.
 All rights reserved.

 Module   :  gfx_buffer

 $Id: $

 FILE DESCRIPTION
 Interface for image comparision functions
==============================================================================*/
#pragma once

#include "libs/core/lfmt/lfmt.h"
#include "libs/core/lfmt/lfmt_block.h"
#include "gfx_buffer_raw.h"

VCOS_EXTERN_C_BEGIN

/*
 Description of a comparision of two pixels described as GFX_LFMT_BLOCK_T.
 Comparisons are performed on a channel by channel basis.

 GFX_BUFFER_DELTA_TYPE specifies what type of comparison should be performed on each channel.

 GFX_BUFFER_COMPARE_ALPHA specifies if an alpha channel should be ignored (if present).

 GFX_BUFFER_DELTA_CHANNELS_MASK specifies whether the channel slots must match exactly, or
 whether permutated colour channels are permitted.

 Examples:
 GFX_BUFFER_DELTA_EQUAL_CHECK_ALPHA_CHANNELS_STRICT is the strongest comparsion possible.
 If the two pixels have the same lfmt (including channel ordering), but the pixel data one or more
 of the channels differs, (even if it is an alpha channel), then the pixels will be deemed different.

 If the two pixels have the same colour channels, but in a different order, the pixels will be
 deemed different, even if the data in each of the channels is the same.

 For example, when using GFX_BUFFER_DELTA_EQUAL_CHECK_ALPHA_CHANNELS_STRICT
 the following two pixels are deemed to differ:
 GFX_LFMT_R8_G8_B8_A8_UNORM (0xa, 0xb, 0xc, 0xd) and GFX_LFMT_A8_B8_G8_R8_UNORM (0xd, 0xc, 0xb, 0xa)

 GFX_BUFFER_DELTA_EQUAL_CHECK_ALPHA_CHANNELS_PERMUTED
 This allows two pixels of differing lfmts to be deemed equal if they contain the same pixel data
 for the same colour channels, but are merely in a different order.

 For example, when using GFX_BUFFER_DELTA_EQUAL_CHECK_ALPHA_CHANNELS_PERMUTED
 the following two pixels are deemed to be equal:
 GFX_LFMT_R8_G8_B8_A8_UNORM (0xa, 0xb, 0xc, 0xd) and GFX_LFMT_A8_B8_G8_R8_UNORM (0xd, 0xc, 0xb, 0xa)
 */
enum GFX_BUFFER_DELTA_T {
   GFX_BUFFER_DELTA_NONE = 0,
#define GFX_BUFFER_DELTA_TYPE_SHIFT 0
#define GFX_BUFFER_DELTA_TYPE_BITS  2
#define GFX_BUFFER_DELTA_TYPE_MASK (0x3 << 0)
   GFX_BUFFER_DELTA_COMPARE_NONE = 0 << GFX_BUFFER_DELTA_TYPE_SHIFT,
   GFX_BUFFER_DELTA_EQUAL        = 1 << GFX_BUFFER_DELTA_TYPE_SHIFT,
   GFX_BUFFER_DELTA_THRESHOLD    = 2 << GFX_BUFFER_DELTA_TYPE_SHIFT,
   GFX_BUFFER_DELTA_SUBTRACT     = 3 << GFX_BUFFER_DELTA_TYPE_SHIFT,

#define GFX_BUFFER_DELTA_COMPARE_ALPHA_SHIFT 2
#define GFX_BUFFER_DELTA_COMPARE_ALPHA_BITS 1
#define GFX_BUFFER_DELTA_COMPARE_ALPHA_MASK (0x1 << 2)
   GFX_BUFFER_DELTA_NO_ALPHA      = 0 << GFX_BUFFER_DELTA_COMPARE_ALPHA_SHIFT,
   GFX_BUFFER_DELTA_CHECK_ALPHA   = 1 << GFX_BUFFER_DELTA_COMPARE_ALPHA_SHIFT,

#define GFX_BUFFER_DELTA_CHANNELS_SHIFT 3
#define GFX_BUFFER_DELTA_CHANNELS_BITS 1
#define GFX_BUFFER_DELTA_CHANNELS_MASK (0x1 << 3)
   GFX_BUFFER_DELTA_CHANNELS_PERMUTED = 0 << GFX_BUFFER_DELTA_CHANNELS_SHIFT,
   GFX_BUFFER_DELTA_CHANNELS_STRICT   = 1 << GFX_BUFFER_DELTA_CHANNELS_SHIFT,

   GFX_BUFFER_DELTA_EQUAL_CHECK_ALPHA_CHANNELS_STRICT = (GFX_BUFFER_DELTA_EQUAL | GFX_BUFFER_DELTA_CHECK_ALPHA | GFX_BUFFER_DELTA_CHANNELS_STRICT),
   GFX_BUFFER_DELTA_EQUAL_NO_ALPHA_CHANNELS_STRICT = (GFX_BUFFER_DELTA_EQUAL | GFX_BUFFER_DELTA_NO_ALPHA | GFX_BUFFER_DELTA_CHANNELS_STRICT),
   GFX_BUFFER_DELTA_EQUAL_NO_ALPHA_CHANNELS_PERMUTED = (GFX_BUFFER_DELTA_EQUAL | GFX_BUFFER_DELTA_NO_ALPHA | GFX_BUFFER_DELTA_CHANNELS_PERMUTED),
   GFX_BUFFER_DELTA_EQUAL_CHECK_ALPHA_CHANNELS_PERMUTED = (GFX_BUFFER_DELTA_EQUAL | GFX_BUFFER_DELTA_CHECK_ALPHA | GFX_BUFFER_DELTA_CHANNELS_PERMUTED),
   GFX_BUFFER_DELTA_THRESHOLD_CHECK_ALPHA_CHANNELS_STRICT = (GFX_BUFFER_DELTA_THRESHOLD | GFX_BUFFER_DELTA_CHECK_ALPHA | GFX_BUFFER_DELTA_CHANNELS_STRICT),
   GFX_BUFFER_DELTA_SUBTRACT_CHECK_ALPHA_CHANNELS_STRICT = (GFX_BUFFER_DELTA_SUBTRACT | GFX_BUFFER_DELTA_CHECK_ALPHA | GFX_BUFFER_DELTA_CHANNELS_STRICT),
// Not implemented:
};

typedef struct {
   enum GFX_BUFFER_DELTA_T delta_type;
   GFX_LFMT_BLOCK_T diff_colour;
   GFX_LFMT_BLOCK_T equality_colour;
   GFX_LFMT_BLOCK_T threshold_colour;
   uint32_t threshold;
} GFX_BUFFER_DELTA_DESC_T;

typedef enum
{
   GFX_BUFFER_CMP_RESULT_EQUAL,
   GFX_BUFFER_CMP_RESULT_WITHIN_THRESHOLD, /* Only when using GFX_BUFFER_DELTA_THRESHOLD */
   GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD, /* Means "not equal" when not using GFX_BUFFER_DELTA_THRESHOLD */
   GFX_BUFFER_CMP_RESULT_INVALID
} gfx_buffer_cmp_result_t;

/***************************************************************************//**
Compares two images, placing comparision result in a third image.

For a given pair of pixels at a given position in the candidate images
   - Sets pixel at this position in result image to diff_colour if pixels differ
   - Sets pixel at this position in result image to same_colour if pixels equal

Returns
   - GFX_BUFFER_CMP_RESULT_EQUAL if images are the same
   - GFX_BUFFER_CMP_RESULT_WITHIN_THRESHOLD if using GFX_BUFFER_DELTA_THRESHOLD
     and all differences are within the threshold
   - GFX_BUFFER_CMP_RESULT_BEYOND_THRESHOLD if images differ in
     dimensions/lfmt, or if pixel differences are not within the threshold

@param  *result      Return parameter containing comparision result
@param  *lhs         First candidate image for comparision
@param  *rhs         Second candidate image for comparision
@param  *delta_desc  Description of type of comparison to be performed
********************************************************************************/
extern gfx_buffer_cmp_result_t gfx_buffer_compare_raw_images(GFX_BUFFER_RAW_T *result,
     const GFX_BUFFER_RAW_T *lhs, const GFX_BUFFER_RAW_T *rhs,
     const GFX_BUFFER_DELTA_DESC_T *delta_desc);

VCOS_EXTERN_C_END
