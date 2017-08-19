/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GFX_LFMT_BLOCK_H
#define GFX_LFMT_BLOCK_H

#include "lfmt.h"
#include "lfmt_fmt_detail.h"

#include "vcos.h"

EXTERN_C_BEGIN

union gfx_lfmt_block_data
{
   /* Use these when the word size is 64 bits, eg C32C32_C32C32 */
   uint64_t ui64[GFX_LFMT_MAX_BYTES_PER_BLOCK / 8];
   int64_t i64[GFX_LFMT_MAX_BYTES_PER_BLOCK / 8];

   /* Use these when the word size is 32 bits, eg C32_C32 or C8C8C8C8 */
   uint32_t ui32[GFX_LFMT_MAX_BYTES_PER_BLOCK / 4];
   int32_t i32[GFX_LFMT_MAX_BYTES_PER_BLOCK / 4];
   float f[GFX_LFMT_MAX_BYTES_PER_BLOCK / 4];

   /* Use these when the word size is 16 bits, eg C5C6C5 or C16_C16 */
   uint16_t ui16[GFX_LFMT_MAX_BYTES_PER_BLOCK / 2];
   int16_t i16[GFX_LFMT_MAX_BYTES_PER_BLOCK / 2];

   /* Use these when the word size is <= 8 bits, eg C8_C8_C8 or C1 */
   uint8_t ui8[GFX_LFMT_MAX_BYTES_PER_BLOCK];
   int8_t i8[GFX_LFMT_MAX_BYTES_PER_BLOCK];
};

/* Helpers for blocks with a "standard" base format (see gfx_lfmt.h) */
extern void gfx_lfmt_block_set_slot_bits(
   union gfx_lfmt_block_data *block,
   const GFX_LFMT_FMT_DETAIL_T *fd, uint32_t slot_idx, uint32_t bits);
extern uint32_t gfx_lfmt_block_get_slot_bits(
   const union gfx_lfmt_block_data *block,
   const GFX_LFMT_FMT_DETAIL_T *fd, uint32_t slot_idx);

/** GFX_LFMT_BLOCK_T */

/* A single block */
typedef struct
{
   GFX_LFMT_T fmt; /* Must have only format, ie no dims or layout */
   union gfx_lfmt_block_data u;
} GFX_LFMT_BLOCK_T;

/* Like vcos_safe_sprintf.
 * If compact (raw only), output will not contain newlines.
 * Otherwise, output may span multiple lines and will always end with a newline. */
extern size_t gfx_lfmt_sprint_block(char *buf, size_t buf_size, size_t offset,
   const GFX_LFMT_BLOCK_T *block); /* Pick apart */
extern size_t gfx_lfmt_sprint_block_raw(char *buf, size_t buf_size, size_t offset,
   const GFX_LFMT_BLOCK_T *block, bool compact); /* Just show raw bytes */

#define GFX_LFMT_SPRINT_BLOCK(BUF_NAME, BLOCK) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 2048, gfx_lfmt_sprint_block, BLOCK)
#define GFX_LFMT_SPRINT_BLOCK_RAW(BUF_NAME, BLOCK, COMPACT) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 128, gfx_lfmt_sprint_block_raw, BLOCK, COMPACT)

/* Returned strings only valid until next call */
extern const char *gfx_lfmt_desc_block(const GFX_LFMT_BLOCK_T *block);
extern const char *gfx_lfmt_desc_block_raw(const GFX_LFMT_BLOCK_T *block, bool compact);

/** GFX_LFMT_BLOCK_ARR_T */

/* An array of blocks, all with the same format */
typedef struct
{
   GFX_LFMT_T fmt; /* Must have only format, ie no dims or layout */
   uint32_t w, h, d; /* Array dims, in blocks */
   /* Data for block (x,y,z) at u[(z * slice_stride) + (y * stride) + x] */
   union gfx_lfmt_block_data *u;
   uint32_t stride, slice_stride;
} GFX_LFMT_BLOCK_ARR_T;

#define GFX_LFMT_BLOCK_ARR_ALLOCA(ARR, W, H, D)             \
   do                                                       \
   {                                                        \
      GFX_LFMT_BLOCK_ARR_T *arr_ = (ARR);                   \
      arr_->w = (W);                                        \
      arr_->h = (H);                                        \
      arr_->d = (D);                                        \
      arr_->u = (union gfx_lfmt_block_data *)vcos_alloca(   \
         arr_->w * arr_->h * arr_->d * sizeof(*arr_->u));   \
      arr_->stride = arr_->w;                               \
      arr_->slice_stride = arr_->w * arr_->h;               \
   } while (0)

static inline union gfx_lfmt_block_data *gfx_lfmt_block_arr_elem(
   const GFX_LFMT_BLOCK_ARR_T *arr, uint32_t x, uint32_t y, uint32_t z)
{
   assert((x < arr->w) && (y < arr->h) && (z < arr->d));
   return &arr->u[(z * arr->slice_stride) + (y * arr->stride) + x];
}

static inline void gfx_lfmt_block_arr_window(GFX_LFMT_BLOCK_ARR_T *arr,
   uint32_t x, uint32_t y, uint32_t z, /* In blocks */
   uint32_t w, uint32_t h, uint32_t d) /* In blocks */
{
   assert(((x + w) <= arr->w) && ((y + h) <= arr->h) && ((z + d) <= arr->d));
   arr->w = w;
   arr->h = h;
   arr->d = d;
   arr->u += (z * arr->slice_stride) + (y * arr->stride) + x;
}

static inline void gfx_lfmt_block_arr_extract(GFX_LFMT_BLOCK_T *block,
   const GFX_LFMT_BLOCK_ARR_T *arr, uint32_t x, uint32_t y, uint32_t z)
{
   block->fmt = arr->fmt;
   block->u = *gfx_lfmt_block_arr_elem(arr, x, y, z);
}

static inline size_t gfx_lfmt_block_arr_sprint_elem_raw(char *buf, size_t buf_size, size_t offset,
   const GFX_LFMT_BLOCK_ARR_T *arr, uint32_t x, uint32_t y, uint32_t z, bool compact)
{
   GFX_LFMT_BLOCK_T block;
   gfx_lfmt_block_arr_extract(&block, arr, x, y, z);
   return gfx_lfmt_sprint_block_raw(buf, buf_size, offset, &block, compact);
}

#define GFX_LFMT_BLOCK_ARR_SPRINT_ELEM_RAW(BUF_NAME, ARR, X, Y, Z, COMPACT) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 128, gfx_lfmt_block_arr_sprint_elem_raw, ARR, X, Y, Z, COMPACT)

static inline bool gfx_lfmt_block_arr_is_single(const GFX_LFMT_BLOCK_ARR_T *arr)
{
   return (arr->w == 1) && (arr->h == 1) && (arr->d == 1);
}

static inline void gfx_lfmt_block_arr_single_get(GFX_LFMT_BLOCK_T *block,
   const GFX_LFMT_BLOCK_ARR_T *arr)
{
   assert(gfx_lfmt_block_arr_is_single(arr));
   gfx_lfmt_block_arr_extract(block, arr, 0, 0, 0);
}

static inline void gfx_lfmt_block_arr_single_set(GFX_LFMT_BLOCK_ARR_T *arr,
   const GFX_LFMT_BLOCK_T *block)
{
   assert(gfx_lfmt_block_arr_is_single(arr));
   arr->fmt = block->fmt;
   *gfx_lfmt_block_arr_elem(arr, 0, 0, 0) = block->u;
}

static inline bool gfx_lfmt_block_arr_dims_match(
   const GFX_LFMT_BLOCK_ARR_T *a, const GFX_LFMT_BLOCK_ARR_T *b)
{
   return (a->w == b->w) && (a->h == b->h) && (a->d == b->d);
}

static inline void gfx_lfmt_block_arr_copy_data(
   const GFX_LFMT_BLOCK_ARR_T *dst, const GFX_LFMT_BLOCK_ARR_T *src)
{
   assert(gfx_lfmt_block_arr_dims_match(dst, src));
   for (uint32_t x = 0; x != dst->w; ++x)
      for (uint32_t y = 0; y != dst->h; ++y)
         for (uint32_t z = 0; z != dst->d; ++z)
            *gfx_lfmt_block_arr_elem(dst, x, y, z) =
               *gfx_lfmt_block_arr_elem(src, x, y, z);
}

static inline void gfx_lfmt_block_arr_dims_in_elems(
   uint32_t *width, uint32_t *height, uint32_t *depth,
   const GFX_LFMT_BLOCK_ARR_T *arr)
{
   GFX_LFMT_BASE_DETAIL_T bd;
   gfx_lfmt_base_detail(&bd, arr->fmt);

   *width = arr->w * bd.block_w;
   *height = arr->h * bd.block_h;
   *depth = arr->d * bd.block_d;
}

static inline bool gfx_lfmt_block_arr_dims_in_elems_equals(
   const GFX_LFMT_BLOCK_ARR_T *arr, uint32_t width, uint32_t height, uint32_t depth)
{
   uint32_t arr_width, arr_height, arr_depth;
   gfx_lfmt_block_arr_dims_in_elems(&arr_width, &arr_height, &arr_depth, arr);
   return (arr_width == width) && (arr_height == height) && (arr_depth == depth);
}

EXTERN_C_END

#endif
