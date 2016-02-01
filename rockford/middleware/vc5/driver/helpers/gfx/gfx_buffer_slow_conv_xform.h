/*==============================================================================
 Copyright (c) 2015 Broadcom Europe Limited.
 All rights reserved.
==============================================================================*/
#pragma once

#include "gfx_lfmt_block.h"
#include "gfx_buffer.h"

VCOS_EXTERN_C_BEGIN

/** Individual transform functions */

union gfx_buffer_xform_options;
typedef union gfx_buffer_xform_options GFX_BUFFER_XFORM_OPTIONS_T;

/* dst/src point to arrays of length num_dst/src_planes.
 * dst[p] (including dst[p].fmt) should be setup by the caller.
 * Only *dst[p].u should be modified by the function (which is why dst is
 * const).
 * src[p] should be entirely setup by the caller.
 *
 * The dimensions of all of the dst/src block arrays *in elements* should match.
 * The dimensions should be precisely the lowest common multiple of the block
 * dimensions of all the dst/src formats.
 *
 * So eg if there is one src plane with format ASTC10X5 and two dst planes with
 * formats Y8 and U8_V8_2X2:
 * - src[0] (ASTC10X5) should be 1x2 blocks (10x10 elements)
 * - dst[0] (Y8) should be 10x10 blocks
 * - dst[1] (U8_V8_2X2) should be 5x5 blocks */
typedef void gfx_buffer_xform_func(
   uint32_t num_dst_planes, const GFX_LFMT_BLOCK_ARR_T *dst,
   uint32_t num_src_planes, const GFX_LFMT_BLOCK_ARR_T *src,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts, /* Should be non-NULL iff func supports options */
   uint32_t x, uint32_t y, uint32_t z); /* For eg ordered dither */
typedef gfx_buffer_xform_func *GFX_BUFFER_XFORM_FUNC_T;

extern gfx_buffer_xform_func gfx_buffer_xform_reinterpret;

extern gfx_buffer_xform_func gfx_buffer_xform_clamp_float_depth;
extern gfx_buffer_xform_func gfx_buffer_xform_split_d32s8;

extern gfx_buffer_xform_func gfx_buffer_xform_expand_subsampled; /* eg C8_2X2 --> C8 */

extern gfx_buffer_xform_func gfx_buffer_xform_extract_yuv; /* Needn't do expand_subsampled first */

typedef struct
{
   /* see TFU spec "4.2 YUV to RGB conversion" */
   uint32_t ay, arr, arc, agb, agr, agc, abb, abc;
} GFX_BUFFER_XFORM_OPTIONS_YUV_TO_SRGB_T;

static inline void gfx_buffer_default_yuv_to_srgb_options(
   GFX_BUFFER_XFORM_OPTIONS_YUV_TO_SRGB_T *opts)
{
   /* Rec 709 */
   opts->ay = 0x4a8;
   opts->arr = 0x72c;
   opts->arc = 0x3e0;
   opts->agb = 0x0da;
   opts->agr = 0x222;
   opts->agc = 0x134;
   opts->abb = 0x873;
   opts->abc = 0x484;
}

extern gfx_buffer_xform_func gfx_buffer_xform_yuv_to_srgb;

/* dst[p].fmt must match src[p].fmt. All this does is force X channels to 0;
 * normally their contents are undefined */
extern gfx_buffer_xform_func gfx_buffer_xform_force_xs_to_0;

extern gfx_buffer_xform_func gfx_buffer_xform_clamp_rgb_to_alpha;

extern gfx_buffer_xform_func gfx_buffer_xform_rgba_to_la;

/* C1/C4 <--> C1X7/C4X4 */
extern gfx_buffer_xform_func gfx_buffer_xform_repack_c1_c4;

typedef struct
{
   bool dither_rgb, dither_a;

   /* Clamp when reducing (u)int bit-width. Otherwise just discard high bits. */
   bool clamp_integer;

   /* Convert unorm<->float using gfx_unorm_to_float/float_to_unorm_depth()
    * instead of gfx_unorm_to_float/float_to_unorm() */
   bool depth_unorm_float;
} GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T;

static inline void gfx_buffer_default_transmute_options(
   GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *opts)
{
   opts->dither_rgb = false;
   opts->dither_a = false;
   opts->clamp_integer = true;
   opts->depth_unorm_float = false;
}

/* Covers:
 * - adding new slots populated with default values based on OpenGL spec.
 * - removal of slots.
 * - reordering of slots.
 * - changing slot type and width */
extern gfx_buffer_xform_func gfx_buffer_xform_transmute;

extern gfx_buffer_xform_func gfx_buffer_xform_float32_to_rgb9e5;
extern gfx_buffer_xform_func gfx_buffer_xform_rgb9e5_to_float32;

/* For TFU mipmap generation.
 * - srgb slots converted to tfu13
 * - unorm slots pass-through
 * but both are called 'UINT' in the output lfmt because there were no suitable
 * lfmt enums handy. */
extern gfx_buffer_xform_func gfx_buffer_xform_srgb_to_tfu13;
extern gfx_buffer_xform_func gfx_buffer_xform_tfu13_to_srgb;

extern gfx_buffer_xform_func gfx_buffer_xform_compress;
extern gfx_buffer_xform_func gfx_buffer_xform_decompress;

union gfx_buffer_xform_options
{
   GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T transmute;
   GFX_BUFFER_XFORM_OPTIONS_YUV_TO_SRGB_T yuv_to_srgb;
};

extern const char *gfx_buffer_xform_func_desc(GFX_BUFFER_XFORM_FUNC_T xform_func);

/* Like vcos_safe_sprintf */
extern size_t gfx_buffer_sprint_xform_options(char *buf, size_t buf_size, size_t offset,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts, GFX_BUFFER_XFORM_FUNC_T xform_func);

/** Transform sequences */

typedef struct
{
   GFX_BUFFER_XFORM_FUNC_T xform_func;
   GFX_LFMT_T dst_fmts[GFX_BUFFER_MAX_PLANES];

   /* LCM of block dimensions of dst_fmts of this xform and previous xform in
    * sequence. xform_func should be called with src/dst arrays of this size
    * (in elements). */
   uint32_t width, height, depth;

   bool has_opts; /* This should be true iff xform_func supports options */
   GFX_BUFFER_XFORM_OPTIONS_T opts;
} GFX_BUFFER_XFORM_T;

/* count is how many steps are in the seq. A valid seq always has >= 1 steps,
 * and the first step is no-op step (the xform_seq_init functions will do this
 * for you). This is so the starting fmts can be stored in this no-op step's
 * dst_fmts. */
typedef struct
{
   GFX_BUFFER_XFORM_T steps[16];
   uint32_t count;
} GFX_BUFFER_XFORM_SEQ_T;

extern void gfx_buffer_xform_seq_add_with_opts_planar(
   GFX_BUFFER_XFORM_SEQ_T *seq,
   GFX_BUFFER_XFORM_FUNC_T xform_func,
   uint32_t dst_num_planes,
   const GFX_LFMT_T *dst_fmts,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts); /* opts may be NULL (meaning use defaults) */

static inline void gfx_buffer_xform_seq_add_with_opts(
   GFX_BUFFER_XFORM_SEQ_T *seq,
   GFX_BUFFER_XFORM_FUNC_T xform_func,
   GFX_LFMT_T dst_fmt,
   const GFX_BUFFER_XFORM_OPTIONS_T *opts) /* opts may be NULL (meaning use defaults) */
{
   gfx_buffer_xform_seq_add_with_opts_planar(seq, xform_func, 1, &dst_fmt, opts);
}

static inline void gfx_buffer_xform_seq_add(
   GFX_BUFFER_XFORM_SEQ_T *seq,
   GFX_BUFFER_XFORM_FUNC_T xform_func,
   GFX_LFMT_T dst_fmt)
{
   gfx_buffer_xform_seq_add_with_opts(seq, xform_func, dst_fmt, NULL);
}

static inline void gfx_buffer_xform_seq_add_planar(
   GFX_BUFFER_XFORM_SEQ_T *seq,
   GFX_BUFFER_XFORM_FUNC_T xform_func,
   uint32_t dst_num_planes,
   const GFX_LFMT_T *dst_fmts)
{
   gfx_buffer_xform_seq_add_with_opts_planar(seq, xform_func, dst_num_planes, dst_fmts, NULL);
}

static inline void gfx_buffer_xform_seq_init_from_fmts(GFX_BUFFER_XFORM_SEQ_T *seq,
   uint32_t num_planes, const GFX_LFMT_T *fmts)
{
   uint32_t p;
   for (p = 0; p != num_planes; ++p)
   {
      /* TODO just drop the dims/layout here instead? Or is "explicit better
       * than implicit"? */
      assert(fmts[p] == gfx_lfmt_fmt(fmts[p]));
   }
   seq->count = 0;
   gfx_buffer_xform_seq_add_planar(seq, NULL, num_planes, fmts);
}

static inline void gfx_buffer_xform_seq_init_from_fmt(GFX_BUFFER_XFORM_SEQ_T *seq,
   GFX_LFMT_T fmt)
{
   gfx_buffer_xform_seq_init_from_fmts(seq, 1, &fmt);
}

static inline void gfx_buffer_xform_seq_init(GFX_BUFFER_XFORM_SEQ_T *seq,
   const GFX_BUFFER_DESC_T *src_desc)
{
   GFX_LFMT_T fmts[GFX_BUFFER_MAX_PLANES];
   gfx_fmts_from_desc(fmts, src_desc);
   gfx_buffer_xform_seq_init_from_fmts(seq, src_desc->num_planes, fmts);
}

static inline const GFX_BUFFER_XFORM_T *gfx_buffer_xform_seq_i(
   const GFX_BUFFER_XFORM_SEQ_T *seq, unsigned int i)
{
   assert(i < seq->count);
   return seq->steps + i;
}

static inline const GFX_BUFFER_XFORM_T *gfx_buffer_xform_seq_peek(
   const GFX_BUFFER_XFORM_SEQ_T *seq)
{
   return gfx_buffer_xform_seq_i(seq, seq->count - 1);
}

static inline GFX_LFMT_T gfx_buffer_xform_seq_peek_dst_fmt(
   const GFX_BUFFER_XFORM_SEQ_T *seq)
{
   const GFX_BUFFER_XFORM_T *xform = gfx_buffer_xform_seq_peek(seq);
   assert(gfx_buffer_lfmts_num_planes(xform->dst_fmts) == 1);
   return xform->dst_fmts[0];
}

/* Like vcos_safe_sprintf */
extern size_t gfx_buffer_sprint_xform(char *buf, size_t buf_size, size_t offset,
   const GFX_BUFFER_XFORM_T *xform);
extern size_t gfx_buffer_sprint_xform_seq(char *buf, size_t buf_size, size_t offset,
   const GFX_BUFFER_XFORM_SEQ_T *seq);

#define GFX_BUFFER_SPRINT_XFORM(BUF_NAME, XFORM) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 512, gfx_buffer_sprint_xform, XFORM)
#define GFX_BUFFER_SPRINT_XFORM_SEQ(BUF_NAME, SEQ) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 4096, gfx_buffer_sprint_xform_seq, SEQ)

/* Returned strings only valid until next call */
extern const char* gfx_buffer_desc_xform(const GFX_BUFFER_XFORM_T *xform);
extern const char* gfx_buffer_desc_xform_seq(const GFX_BUFFER_XFORM_SEQ_T *seq);

/** Transform sequence construction helpers */

typedef enum
{
   /* The ordering here is important -- a > b <=> a is a superset of b */

   /* Conversions that are well-defined in the khronos APIs, like unorm<->float */
   GFX_BUFFER_XFORM_CONVS_REGULAR,

   /* A broader set of conversions, including compression/decompression and
    * YUV->RGB */
   GFX_BUFFER_XFORM_CONVS_BROAD,

   /* An even broader set. Some of the conversions are a bit bogus, eg
    * INT->UNORM conversion is done by just reinterpreting the INT data as
    * UNORM. */
   GFX_BUFFER_XFORM_CONVS_DEBUG_VIEW
} gfx_buffer_xform_convs_t;

/* Add a series of xforms to xform_seq to get to dst_fmts, only making use of
 * conversions covered by convs. If this is not possible, this function may
 * assert or you may hit an assertion when trying to use the generated xform
 * seq. */
extern void gfx_buffer_xform_seq_construct_continue(
   GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   uint32_t dst_num_planes, const GFX_LFMT_T *dst_fmts,
   gfx_buffer_xform_convs_t convs,
   /* transmute_options may be NULL (meaning use defaults) */
   const GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *transmute_options);

static inline void gfx_buffer_xform_seq_construct(
   GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   uint32_t dst_num_planes, const GFX_LFMT_T *dst_fmts,
   uint32_t src_num_planes, const GFX_LFMT_T *src_fmts,
   gfx_buffer_xform_convs_t convs,
   /* transmute_options may be NULL (meaning use defaults) */
   const GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *transmute_options)
{
   gfx_buffer_xform_seq_init_from_fmts(xform_seq, src_num_planes, src_fmts);
   gfx_buffer_xform_seq_construct_continue(xform_seq, dst_num_planes, dst_fmts,
      convs, transmute_options);
}

static inline void gfx_buffer_xform_seq_construct_continue_desc(
   GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   const GFX_BUFFER_DESC_T *dst,
   gfx_buffer_xform_convs_t convs,
   /* transmute_options may be NULL (meaning use defaults) */
   const GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *transmute_options)
{
   GFX_LFMT_T dst_fmts[GFX_BUFFER_MAX_PLANES];
   gfx_fmts_from_desc(dst_fmts, dst);
   gfx_buffer_xform_seq_construct_continue(xform_seq,
      dst->num_planes, dst_fmts, convs, transmute_options);
}

static inline void gfx_buffer_xform_seq_construct_desc(
   GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   const GFX_BUFFER_DESC_T *dst, const GFX_BUFFER_DESC_T *src,
   gfx_buffer_xform_convs_t convs,
   /* transmute_options may be NULL (meaning use defaults) */
   const GFX_BUFFER_XFORM_OPTIONS_TRANSMUTE_T *transmute_options)
{
   gfx_buffer_xform_seq_init(xform_seq, src);
   gfx_buffer_xform_seq_construct_continue_desc(xform_seq,
      dst, convs, transmute_options);
}

/** Transform sequence application */

extern void gfx_buffer_get_xform_seq_quantum(
   uint32_t *width, uint32_t *height, uint32_t *depth,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq);

/* out should point to an array of length
 * gfx_buffer_lfmts_num_planes(xform_seq->steps[xform_seq->count - 1].dst_fmts).
 * out[p] (including out[p].fmt) should be setup by the caller.
 * out[p].fmt should match xform_seq->steps[xform_seq->count - 1].dst_fmts[p].
 * Only *out[p].u will be written by this function (which is why out is const).
 *
 * in should point to an array of length
 * gfx_buffer_lfmts_num_planes(xform_seq->steps[0].dst_fmts).
 * in[p] should be entirely setup by the caller.
 * in[p].fmt should match xform_seq->steps[0].dst_fmts[p].
 *
 * The dimensions of all of the in/out block arrays *in elements* should match.
 * The dimensions should be a multiple of the dimensions returned by
 * gfx_buffer_get_xform_seq_quantum(). */
extern void gfx_buffer_apply_xform_seq(
   const GFX_LFMT_BLOCK_ARR_T *out,
   const GFX_LFMT_BLOCK_ARR_T *in,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   uint32_t x, uint32_t y, uint32_t z);

static inline void gfx_buffer_apply_xform_seq_single_block(
   GFX_LFMT_BLOCK_T *out, const GFX_LFMT_BLOCK_T *in,
   const GFX_BUFFER_XFORM_SEQ_T *xform_seq,
   uint32_t x, uint32_t y, uint32_t z)
{
   GFX_LFMT_BLOCK_ARR_T out_arr;
   GFX_LFMT_BLOCK_ARR_ALLOCA(&out_arr, 1, 1, 1);
   out_arr.fmt = gfx_buffer_xform_seq_peek_dst_fmt(xform_seq);

   GFX_LFMT_BLOCK_ARR_T in_arr;
   GFX_LFMT_BLOCK_ARR_ALLOCA(&in_arr, 1, 1, 1);
   gfx_lfmt_block_arr_single_set(&in_arr, in);

   gfx_buffer_apply_xform_seq(&out_arr, &in_arr, xform_seq, x, y, z);

   gfx_lfmt_block_arr_single_get(out, &out_arr);
}

VCOS_EXTERN_C_END
