/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Image conversion using neon code

FILE DESCRIPTION
=============================================================================*/
#include "v3d_imgconv_internal.h"
#include "v3d_parallel.h"

VCOS_LOG_CAT_T v3d_image_conv_neon_log = VCOS_LOG_INIT("v3d_image_conv_neon_log", VCOS_LOG_WARN);
#define VCOS_LOG_CATEGORY (&v3d_image_conv_neon_log)

// Commenting this in will cause extra copies for testing e.g.
// RSO-UIF will do ROS->UIF->RSO->UIF
//
// #define TESTING

#ifdef __arm__
#include <arm_neon.h>
#define USE_NEON_FAST_PATHS
#endif

#ifdef USE_NEON_FAST_PATHS
static uint32_t *address128(uint32_t *dst, uint32_t x, uint32_t y)
{
   uint32_t *dst128 = dst;

   if (x > 1)
      dst128 += 16;

   dst128 += (x & 0x1) << 2;

   if (y > 1)
      dst128 += 32;

   dst128 += (y & 0x1) << 3;

   return dst128;
}

static const uint32_t *address128_const(const uint32_t *src, uint32_t x, uint32_t y)
{
   return address128((uint32_t *)src, x, y);
}

static uint32_t *address64(uint32_t *dst, uint32_t x, uint32_t y)
{
   uint32_t *dst64 = dst;

   if (x > 3)
      dst64 += 16;

   dst64 += (x & 0x3) << 1;

   if (y > 1)
      dst64 += 32;

   dst64 += (y & 0x1) << 3;

   return dst64;
}

static const uint32_t *address64_const(const uint32_t *src, uint32_t x, uint32_t y)
{
   return address64((uint32_t *)src, x, y);
}

static uint32_t *address32(uint32_t *dst, uint32_t x, uint32_t y)
{
   uint32_t *dst32 = dst;

   if (x > 3)
      dst32 += 16;

   dst32 += x & 0x3;

   if (y > 3)
      dst32 += 32;

   dst32 += (y & 0x3) << 2;

   return dst32;
}

static const uint32_t *address32_const(const uint32_t *src, uint32_t x, uint32_t y)
{
   return address32((uint32_t *)src, x, y);
}

static void copy_rows_rso128_to_uif128(const uint8_t *src_ptr, uint32_t *dst_ptr, uint32_t stride)
{
   uint32x4_t   rowl;
   uint32x4_t   rowr;

   const uint8_t  *src_ptr0 = src_ptr;
   const uint8_t  *src_ptr1 = src_ptr0 + stride;

   __builtin_prefetch(&src_ptr0[0x00], 0);
   __builtin_prefetch(&src_ptr1[0x00], 0);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr0[0x00]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr0[0x20]));
   vst1q_u32(&dst_ptr[0x00], rowl);
   vst1q_u32(&dst_ptr[0x10], rowr);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr0[0x10]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr0[0x30]));
   vst1q_u32(&dst_ptr[0x04], rowl);
   vst1q_u32(&dst_ptr[0x14], rowr);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr1[0x00]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr1[0x20]));
   vst1q_u32(&dst_ptr[0x08], rowl);
   vst1q_u32(&dst_ptr[0x18], rowr);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr1[0x10]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr1[0x30]));
   vst1q_u32(&dst_ptr[0x0c], rowl);
   vst1q_u32(&dst_ptr[0x1c], rowr);
}

static void copy_rows_rso64_to_uif64(const uint8_t *src_ptr, uint32_t *dst_ptr, uint32_t stride)
{
   uint32x4_t   rowl;
   uint32x4_t   rowr;

   const uint8_t  *src_ptr0 = src_ptr;
   const uint8_t  *src_ptr1 = src_ptr0 + stride;

   __builtin_prefetch(&src_ptr0[0x00], 0);
   __builtin_prefetch(&src_ptr1[0x00], 0);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr0[0x00]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr0[0x20]));
   vst1q_u32(&dst_ptr[0x00], rowl);
   vst1q_u32(&dst_ptr[0x10], rowr);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr0[0x10]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr0[0x30]));
   vst1q_u32(&dst_ptr[0x04], rowl);
   vst1q_u32(&dst_ptr[0x14], rowr);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr1[0x00]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr1[0x20]));
   vst1q_u32(&dst_ptr[0x08], rowl);
   vst1q_u32(&dst_ptr[0x18], rowr);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr1[0x10]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr1[0x30]));
   vst1q_u32(&dst_ptr[0x0c], rowl);
   vst1q_u32(&dst_ptr[0x1c], rowr);
}

static void copy_rows_rso32_to_uif32(const uint8_t *src_ptr, uint32_t *dst_ptr, uint32_t stride)
{
   uint32x4_t   rowl;
   uint32x4_t   rowr;

   const uint8_t  *src_ptr0 = src_ptr;
   const uint8_t  *src_ptr1 = src_ptr0 + stride;
   const uint8_t  *src_ptr2 = src_ptr1 + stride;
   const uint8_t  *src_ptr3 = src_ptr2 + stride;

   __builtin_prefetch(&src_ptr0[0x00], 0);
   __builtin_prefetch(&src_ptr1[0x00], 0);
   __builtin_prefetch(&src_ptr2[0x00], 0);
   __builtin_prefetch(&src_ptr3[0x00], 0);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr0[0x00]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr0[0x10]));
   vst1q_u32(&dst_ptr[0x00], rowl);
   vst1q_u32(&dst_ptr[0x10], rowr);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr1[0x00]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr1[0x10]));
   vst1q_u32(&dst_ptr[0x04], rowl);
   vst1q_u32(&dst_ptr[0x14], rowr);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr2[0x00]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr2[0x10]));
   vst1q_u32(&dst_ptr[0x08], rowl);
   vst1q_u32(&dst_ptr[0x18], rowr);

   rowl = vreinterpretq_u32_u8(vld1q_u8(&src_ptr3[0x00]));
   rowr = vreinterpretq_u32_u8(vld1q_u8(&src_ptr3[0x10]));
   vst1q_u32(&dst_ptr[0x0c], rowl);
   vst1q_u32(&dst_ptr[0x1c], rowr);
}

static void copy_pixel_rso128_to_uif128(const uint8_t *src, uint32_t *dst, uint32_t x, uint32_t y)
{
   uint32_t *dst_ptr = address128(dst, x, y);

   dst_ptr[0] = ((uint32_t *)src)[0];
   dst_ptr[1] = ((uint32_t *)src)[1];
   dst_ptr[2] = ((uint32_t *)src)[2];
   dst_ptr[3] = ((uint32_t *)src)[3];
}

static void copy_pixel_uif128_to_rso128(const uint32_t *src, uint32_t x, uint32_t y, uint8_t *dst)
{
   const uint32_t *src_ptr = address128_const(src, x, y);

   ((uint32_t *)dst)[0] = src_ptr[0];
   ((uint32_t *)dst)[1] = src_ptr[1];
   ((uint32_t *)dst)[2] = src_ptr[2];
   ((uint32_t *)dst)[3] = src_ptr[3];
}

static void copy_pixel_rso64_to_uif64(const uint8_t *src, uint32_t *dst, uint32_t x, uint32_t y)
{
   uint32_t *dst_ptr = address64(dst, x, y);

   dst_ptr[0] = ((uint32_t *)src)[0];
   dst_ptr[1] = ((uint32_t *)src)[1];
}

static void copy_pixel_uif64_to_rso64(const uint32_t *src, uint32_t x, uint32_t y, uint8_t *dst)
{
   const uint32_t *src_ptr = address64_const(src, x, y);

   ((uint32_t *)dst)[0] = src_ptr[0];
   ((uint32_t *)dst)[1] = src_ptr[1];
}

static void copy_pixel_rso32_to_uif32(const uint8_t *src, uint32_t *dst, uint32_t x, uint32_t y)
{
   uint32_t *dst_ptr = address32(dst, x, y);

   *dst_ptr = *(uint32_t *)src;
}

static void copy_pixel_uif32_to_rso32(const uint32_t *src, uint32_t x, uint32_t y, uint8_t *dst)
{
   const uint32_t *src_ptr = address32_const(src, x, y);

   *(uint32_t *)dst = *src_ptr;
}

static void copy_rows_rso32_to_uif32_rbswap(const uint8_t *src_ptr, uint32_t *dst_ptr, uint32_t stride)
{
   uint32x4_t   rowl;
   uint32x4_t   rowr;
   const uint8x8_t bgra_swizzle = { 2, 1, 0, 3, 6, 5, 4, 7 };
   uint8x16_t   a, b;

   const uint8_t  *src_ptr0 = src_ptr;
   const uint8_t  *src_ptr1 = src_ptr0 + stride;
   const uint8_t  *src_ptr2 = src_ptr1 + stride;
   const uint8_t  *src_ptr3 = src_ptr2 + stride;

   __builtin_prefetch(&src_ptr0[0x00], 0);
   __builtin_prefetch(&src_ptr1[0x00], 0);
   __builtin_prefetch(&src_ptr2[0x00], 0);
   __builtin_prefetch(&src_ptr3[0x00], 0);

   a = vld1q_u8(&src_ptr0[0x00]);
   b = vld1q_u8(&src_ptr0[0x10]);
   a = vcombine_u8(vtbl1_u8(vget_low_u8(a), bgra_swizzle), vtbl1_u8(vget_high_u8(a), bgra_swizzle));
   b = vcombine_u8(vtbl1_u8(vget_low_u8(b), bgra_swizzle), vtbl1_u8(vget_high_u8(b), bgra_swizzle));
   rowl = vreinterpretq_u32_u8(a);
   rowr = vreinterpretq_u32_u8(b);
   vst1q_u32(&dst_ptr[0x00], rowl);
   vst1q_u32(&dst_ptr[0x10], rowr);

   a = vld1q_u8(&src_ptr1[0x00]);
   b = vld1q_u8(&src_ptr1[0x10]);
   a = vcombine_u8(vtbl1_u8(vget_low_u8(a), bgra_swizzle), vtbl1_u8(vget_high_u8(a), bgra_swizzle));
   b = vcombine_u8(vtbl1_u8(vget_low_u8(b), bgra_swizzle), vtbl1_u8(vget_high_u8(b), bgra_swizzle));
   rowl = vreinterpretq_u32_u8(a);
   rowr = vreinterpretq_u32_u8(b);
   vst1q_u32(&dst_ptr[0x04], rowl);
   vst1q_u32(&dst_ptr[0x14], rowr);

   a = vld1q_u8(&src_ptr2[0x00]);
   b = vld1q_u8(&src_ptr2[0x10]);
   a = vcombine_u8(vtbl1_u8(vget_low_u8(a), bgra_swizzle), vtbl1_u8(vget_high_u8(a), bgra_swizzle));
   b = vcombine_u8(vtbl1_u8(vget_low_u8(b), bgra_swizzle), vtbl1_u8(vget_high_u8(b), bgra_swizzle));
   rowl = vreinterpretq_u32_u8(a);
   rowr = vreinterpretq_u32_u8(b);
   vst1q_u32(&dst_ptr[0x08], rowl);
   vst1q_u32(&dst_ptr[0x18], rowr);

   a = vld1q_u8(&src_ptr3[0x00]);
   b = vld1q_u8(&src_ptr3[0x10]);
   a = vcombine_u8(vtbl1_u8(vget_low_u8(a), bgra_swizzle), vtbl1_u8(vget_high_u8(a), bgra_swizzle));
   b = vcombine_u8(vtbl1_u8(vget_low_u8(b), bgra_swizzle), vtbl1_u8(vget_high_u8(b), bgra_swizzle));
   rowl = vreinterpretq_u32_u8(a);
   rowr = vreinterpretq_u32_u8(b);
   vst1q_u32(&dst_ptr[0x0c], rowl);
   vst1q_u32(&dst_ptr[0x1c], rowr);
}

static uint32_t toBGRA(uint32_t a)
{
   return ((a & 0xFF00FF00) | ((__builtin_bswap32(a) >> 8) & 0x00FF00FF));
}

static void copy_pixel_rso32_to_uif32_rbswap(const uint8_t *src, uint32_t *dst, uint32_t x, uint32_t y)
{
   uint32_t *dst_ptr = address32(dst, x, y);

   *dst_ptr = toBGRA(*(uint32_t *)src);
}

static void copy_row_rso_c8c8c8_to_uif_c8c8c8x8(const uint8_t *src_ptr, uint32_t *dst_ptr, uint32_t src_stride)
{
   uint8x8x3_t  row3;
   uint8x8x4_t  row4;
   uint8x8_t    ff;

   ff = vdup_n_u8(0xff);

   row3 = vld3_u8(src_ptr);

   row4.val[0] = row3.val[0];
   row4.val[1] = row3.val[1];
   row4.val[2] = row3.val[2];
   row4.val[3] = ff;

   vst4_lane_u8((uint8_t *)&dst_ptr[0x00], row4, 0);
   vst4_lane_u8((uint8_t *)&dst_ptr[0x01], row4, 1);
   vst4_lane_u8((uint8_t *)&dst_ptr[0x02], row4, 2);
   vst4_lane_u8((uint8_t *)&dst_ptr[0x03], row4, 3);
   vst4_lane_u8((uint8_t *)&dst_ptr[0x10], row4, 4);
   vst4_lane_u8((uint8_t *)&dst_ptr[0x11], row4, 5);
   vst4_lane_u8((uint8_t *)&dst_ptr[0x12], row4, 6);
   vst4_lane_u8((uint8_t *)&dst_ptr[0x13], row4, 7);
}

static void copy_pixel_rso_c8c8c8_to_uif_c8c8c8x8(const uint8_t *src, uint32_t *dst, uint32_t x, uint32_t y)
{
   uint8_t *dst8 = (uint8_t *)address32(dst, x, y);

   dst8[0] = src[0];
   dst8[1] = src[1];
   dst8[2] = src[2];
   dst8[3] = 0xff;
}

static uint16_t *address16(uint32_t *dst, uint32_t x, uint32_t y)
{
   uint16_t *dst16 = (uint16_t *)dst;

   if (x > 7)
      dst16 += 32;

   if (y > 3)
      dst16 += 64;

   dst16 += x & 0x7;

   dst16 += (y & 0x3) * 8;

   return dst16;
}

static const uint16_t *address16_const(const uint32_t *src, uint32_t x, uint32_t y)
{
   return address16((uint32_t *)src, x, y);
}

static void copy_pixel_rso16_to_uif16(const uint8_t *src, uint32_t *dst, uint32_t x, uint32_t y)
{
   uint16_t *dst16 = address16(dst, x, y);

   *dst16 = *(uint16_t *)src;
}

static void copy_pixel_uif16_to_rso16(const uint32_t *src, uint32_t x, uint32_t y, uint8_t *dst)
{
   const uint16_t *src_ptr = address16_const(src, x, y);

   *(uint16_t *)dst = *src_ptr;
}

static void copy_rows_rso8_to_uif8(const uint8_t *src_ptr, uint32_t *dst_ptr, uint32_t src_stride)
{
   const uint8_t *src_ptr0;
   const uint8_t *src_ptr1;

   uint32x4_t   row0;
   uint32x4_t   row1;

   src_ptr0 = src_ptr;
   src_ptr1 = src_ptr0 + src_stride;

   row0 = vreinterpretq_u32_u8(vld1q_u8(src_ptr0));
   row1 = vreinterpretq_u32_u8(vld1q_u8(src_ptr1));

   vst1_u32(&dst_ptr[0x00], vget_low_u32(row0));
   vst1_u32(&dst_ptr[0x02], vget_low_u32(row1));
   vst1_u32(&dst_ptr[0x10], vget_high_u32(row0));
   vst1_u32(&dst_ptr[0x12], vget_high_u32(row1));

   src_ptr0 = src_ptr1 + src_stride;
   src_ptr1 = src_ptr0 + src_stride;

   row0 = vreinterpretq_u32_u8(vld1q_u8(src_ptr0));
   row1 = vreinterpretq_u32_u8(vld1q_u8(src_ptr1));

   vst1_u32(&dst_ptr[0x04], vget_low_u32(row0));
   vst1_u32(&dst_ptr[0x06], vget_low_u32(row1));
   vst1_u32(&dst_ptr[0x14], vget_high_u32(row0));
   vst1_u32(&dst_ptr[0x16], vget_high_u32(row1));

   src_ptr0 = src_ptr1 + src_stride;
   src_ptr1 = src_ptr0 + src_stride;

   row0 = vreinterpretq_u32_u8(vld1q_u8(src_ptr0));
   row1 = vreinterpretq_u32_u8(vld1q_u8(src_ptr1));

   vst1_u32(&dst_ptr[0x08], vget_low_u32(row0));
   vst1_u32(&dst_ptr[0x0a], vget_low_u32(row1));
   vst1_u32(&dst_ptr[0x18], vget_high_u32(row0));
   vst1_u32(&dst_ptr[0x1a], vget_high_u32(row1));

   src_ptr0 = src_ptr1 + src_stride;
   src_ptr1 = src_ptr0 + src_stride;

   row0 = vreinterpretq_u32_u8(vld1q_u8(src_ptr0));
   row1 = vreinterpretq_u32_u8(vld1q_u8(src_ptr1));

   vst1_u32(&dst_ptr[0x0c], vget_low_u32(row0));
   vst1_u32(&dst_ptr[0x0e], vget_low_u32(row1));
   vst1_u32(&dst_ptr[0x1c], vget_high_u32(row0));
   vst1_u32(&dst_ptr[0x1e], vget_high_u32(row1));
}

static uint8_t *address8(uint32_t *dst, uint32_t x, uint32_t y)
{
   uint8_t *dst8 = (uint8_t *)dst;

   if (x > 7)
      dst8 += 64;

   if (y > 7)
      dst8 += 128;

   dst8 += x & 0x7;

   dst8 += (y & 0x7) * 8;

   return dst8;
}

static const uint8_t *address8_const(const uint32_t *src, uint32_t x, uint32_t y)
{
   return address8((uint32_t *)src, x, y);
}

static void copy_pixel_rso8_to_uif8(const uint8_t *src, uint32_t *dst, uint32_t x, uint32_t y)
{
   uint8_t  *dst8 = address8(dst, x, y);

   *dst8 = *src;
}

static void copy_pixel_uif8_to_rso8(const uint32_t *src, uint32_t x, uint32_t y, uint8_t *dst)
{
   const uint8_t  *src_ptr = address8_const(src, x, y);

   *dst = *src_ptr;
}

// Forward declaration for typedefs below.
struct CopyAlgorithm_s;

typedef bool (*Copy_t)(
   const struct CopyAlgorithm_s *self,
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height);

typedef bool (*Supports_t)(
   const struct CopyAlgorithm_s *self,
   const struct v3d_imgconv_base_tgt *src,
   const struct v3d_imgconv_base_tgt *dst,
   uint32_t width, uint32_t height);

typedef void (*R2U_CopyNRows_t)(
   const uint8_t *src_ptr, uint32_t *dst_ptr, uint32_t src_stride);

typedef void (*R2U_CopyPixel_t)(
   const uint8_t *src_ptr, uint32_t *dst_ptr, uint32_t x, uint32_t y);

typedef void (*U2R_CopyPixel_t)(
   const uint32_t *src_ptr, uint32_t x, uint32_t y, uint8_t *dst_ptr);

typedef struct CopyAlgorithm_s
{
   // GENERIC FUNCTIONS
   Copy_t            copy;             // Main copying routine
   Supports_t        supports;         // Test that a fast path is supported

   // RSO TO UIF SPECIFIC
   R2U_CopyNRows_t   r2u_copy_n_rows;  // Copy n-rows of data from src to dst
   uint32_t          r2u_n_rows;       // Number of rows copied by function
   R2U_CopyPixel_t   r2u_copy_pixel;   // Copy a pixel for rso to uif

   // UIF TO RSO SPECIFIC
   U2R_CopyPixel_t   u2r_copy_pixel;   // Copy a pixel for uif to rso

   // GENERIC DATA
   uint32_t          block_width;      // Width of a UIF block in pixels
   uint32_t          block_height;     // Height of UIF block in pixels
   uint32_t          src_bpp;          // Bits per pixel in source
   uint32_t          dst_bpp;          // Bits per pixel in source
} CopyAlgorithm_t;

// Only allow copies of complete blocks
// blocks from and to the origin of the images
//
static bool supports_conservative(const CopyAlgorithm_t *self,
                                  const struct v3d_imgconv_base_tgt *dst,
                                  const struct v3d_imgconv_base_tgt *src,
                                  uint32_t width, uint32_t height)
{
   if (((width  % self->block_width)  != 0) ||
       ((height % self->block_height) != 0) ||
       src->x != 0 || src->y != 0 || dst->x != 0 || dst->y != 0)
      return false;

   return true;
}

// Supports all transfers
//
static bool supports_everything(
   const CopyAlgorithm_t *self,
   const struct v3d_imgconv_base_tgt *dst,
   const struct v3d_imgconv_base_tgt *src,
   uint32_t width, uint32_t height)
{
   vcos_unused(self);
   vcos_unused(dst);
   vcos_unused(src);
   vcos_unused(width);
   vcos_unused(height);

   return true;
}

// Supports function that allows the common sub-image case
// which is to copy a source image from (0, 0) to an arbitrary destination
//
#if 0
static bool supports_source_origin(
   const CopyAlgorithm_t *self,
   const struct v3d_imgconv_base_tgt *dst,
   const struct v3d_imgconv_base_tgt *src,
   uint32_t width, uint32_t height)
{
   vcos_unused(self);
   vcos_unused(dst);
   vcos_unused(width);
   vcos_unused(height);

   return src->x == 0 && src->y == 0;
}
#endif

static bool generic_rso_to_uif_internal(
   const CopyAlgorithm_t *self,
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height)
{

   bool yflip = gfx_lfmt_get_yflip(&src->desc.planes[0].lfmt) != 0;

   // Short names for the algorithm details
   uint32_t block_width  = self->block_width;
   uint32_t block_height = self->block_height;
   uint32_t src_bytes_pp = self->src_bpp / 8;
   uint32_t n_rows       = self->r2u_n_rows;

   // Is this aligned to UIF blocks?
   bool block_aligned = dst->x % block_width  == 0 &&
                        dst->y % block_height == 0 &&
                        width  % block_width  == 0 &&
                        height % block_height == 0;

   GFX_LFMT_BASE_DETAIL_T  dst_bd;

   gfx_lfmt_base_detail(&dst_bd, dst->desc.planes[0].lfmt);

   // Block coordinate of start block
   uint32_t block_x_start = dst->x / block_width;
   uint32_t block_y_start = dst->y / block_height;

   // Block coordinate of end block
   uint32_t block_x_end   = (dst->x + width  - 1) / block_width;
   uint32_t block_y_end   = (dst->y + height - 1) / block_height;

   // Setup source details
   uint32_t  src_pitch      = src->desc.planes[0].pitch;
   uint32_t  src_rows_step  = n_rows * src_pitch;

   // Pointer to first row of source image
   uint8_t  *src_origin_ptr = (uint8_t *)src_data + src->desc.planes[0].offset;

   // Adjust for flip
   if (yflip)
   {
      src_origin_ptr += (src->desc.height - 1) * src_pitch;
      src_pitch       = -src_pitch;
      src_rows_step   = -src_rows_step;
   }

   // Adjust for source offset
   src_origin_ptr += src_pitch * src->y;
   src_origin_ptr += src_bytes_pp * src->x;

   // Setup destination details
   uint32_t rows_per_utile = self->block_height / 2;
   uint32_t dst_step       = 16 * n_rows / rows_per_utile;

   // Now copy the pixels
   uint32_t block_x;
   uint32_t block_y;

   for (block_y = block_y_start; block_y <= block_y_end; ++block_y)
   {
      uint32_t  *dst_ptr = NULL;

      for (block_x = block_x_start; block_x <= block_x_end; ++block_x)
      {
         uint32_t  block_pix_x = block_x * block_width;
         uint32_t  block_pix_y = block_y * block_height;

         // 4 horizontal blocks are contiguous, so don't have to recalculate every time.
         if (!block_aligned || dst_ptr == NULL || (block_x & 0x3) == 0)
            dst_ptr = gfx_buffer_block_p(&dst->desc.planes[0], &dst_bd, dst_data,
                                         block_pix_x, block_pix_y, 0,
                                         dst->desc.height);

         if (!block_aligned && (block_pix_x < dst->x                        ||
                                block_pix_y < dst->y                        ||
                                block_pix_x + block_width  > dst->x + width ||
                                block_pix_y + block_height > dst->y + height))
         {
            //////////////////////////////////
            // Slow copy pixel by pixel
            // There is scope to optimize runs
            // or to copy utiles if complete
            //////////////////////////////////
            int32_t   x_begin = vcos_max(block_pix_x, dst->x) - block_pix_x;
            int32_t   y_begin = vcos_max(block_pix_y, dst->y) - block_pix_y;
            // End is first invalid pixel
            int32_t   x_end   = vcos_min(block_pix_x + block_width,  dst->x + width)  - block_pix_x;
            int32_t   y_end   = vcos_min(block_pix_y + block_height, dst->y + height) - block_pix_y;

            uint8_t   *src_ptr = src_origin_ptr + (block_pix_y - dst->y + y_begin) * src_pitch +
                                                  (block_pix_x - dst->x + x_begin) * src_bytes_pp;
            uint8_t   *src_pix = NULL;

            int32_t    x, y;

            for (y = y_begin; y < y_end; ++y)
            {
               src_pix = src_ptr;

               for (x = x_begin; x < x_end; ++x)
               {
                  self->r2u_copy_pixel(src_pix, dst_ptr, x, y);
                  src_pix += src_bytes_pp;
               }

               src_ptr += src_pitch;
            }
         }
         else
         {
            ////////////////////////////
            // Fast copy a full block
            ////////////////////////////
            uint32_t utrow;
            uint32_t row;

            uint8_t  *src_ptr = src_origin_ptr + (block_pix_y - dst->y) * src_pitch +
                                                 (block_pix_x - dst->x) * src_bytes_pp;

            for (utrow = 0; utrow < 2; ++utrow)
            {
               for (row = 0; row  < rows_per_utile; row += n_rows)
               {
                  self->r2u_copy_n_rows(src_ptr, dst_ptr, src_pitch);
                  dst_ptr += dst_step;
                  src_ptr += src_rows_step;
               }

               // We already added 16.
               dst_ptr += 16;
            }
         }
      }
   }

   return true;
}

static bool generic_uif_to_rso(
   const CopyAlgorithm_t *self,
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height)
{
   bool yflip = gfx_lfmt_get_yflip(&src->desc.planes[0].lfmt) != 0;

   // Short names for the algorithm details
   uint32_t block_width  = self->block_width;
   uint32_t block_height = self->block_height;
   uint32_t dst_bytes_pp = self->dst_bpp / 8;

   GFX_LFMT_BASE_DETAIL_T  src_bd;

   gfx_lfmt_base_detail(&src_bd, src->desc.planes[0].lfmt);

   // Block coordinate of start block
   uint32_t block_x_start = src->x / block_width;
   uint32_t block_y_start = src->y / block_height;

   // Block coordinate of end block
   uint32_t block_x_end   = (src->x + width  - 1) / block_width;
   uint32_t block_y_end   = (src->y + height - 1) / block_height;

   // Setup dst details
   uint32_t  dst_pitch    = dst->desc.planes[0].pitch;

   // Pointer to first row of source image
   uint8_t  *dst_origin_ptr = (uint8_t *)dst_data + dst->desc.planes[0].offset;

   // Adjust for flip
   if (yflip)
   {
      dst_origin_ptr += (dst->desc.height - 1) * dst_pitch;
      dst_pitch       = -dst_pitch;
   }

   // Adjust for source offset
   dst_origin_ptr += dst_pitch * dst->y;
   dst_origin_ptr += dst_bytes_pp * dst->x;

   // Now copy the pixels
   uint32_t block_x;
   uint32_t block_y;

   for (block_y = block_y_start; block_y <= block_y_end; ++block_y)
   {
      const uint32_t  *src_ptr = NULL;

      for (block_x = block_x_start; block_x <= block_x_end; ++block_x)
      {
         uint32_t  block_pix_x = block_x * block_width;
         uint32_t  block_pix_y = block_y * block_height;

         src_ptr = gfx_buffer_block_p(&src->desc.planes[0], &src_bd, src_data,
                                      block_pix_x, block_pix_y, 0,
                                      src->desc.height);

         int32_t   x_begin = vcos_max(block_pix_x, src->x) - block_pix_x;
         int32_t   y_begin = vcos_max(block_pix_y, src->y) - block_pix_y;
         // End is first invalid pixel
         int32_t   x_end   = vcos_min(block_pix_x + block_width,  src->x + width)  - block_pix_x;
         int32_t   y_end   = vcos_min(block_pix_y + block_height, src->y + height) - block_pix_y;

         uint8_t   *dst_ptr = dst_origin_ptr + (block_pix_y - src->y + y_begin) * dst_pitch +
                                               (block_pix_x - src->x + x_begin) * dst_bytes_pp;
         uint8_t   *dst_pix = NULL;

         int32_t    x, y;

         for (y = y_begin; y < y_end; ++y)
         {
            dst_pix = dst_ptr;

            for (x = x_begin; x < x_end; ++x)
            {
               self->u2r_copy_pixel(src_ptr, x, y, dst_pix);
               dst_pix += dst_bytes_pp;
            }

            dst_ptr += dst_pitch;
         }
      }
   }

   return true;
}

typedef struct RSOtoUIFWork
{
   const CopyAlgorithm_t       *self;
   struct v3d_imgconv_base_tgt  dst;
   void                        *dst_data;
   struct v3d_imgconv_base_tgt  src;
   void                        *src_data;
   unsigned int                 width;
   unsigned int                 height;
} RSOtoUIFWork;

static void generic_rso_to_uif_sub(void *data)
{
   RSOtoUIFWork *work = (RSOtoUIFWork *)data;

   generic_rso_to_uif_internal(work->self,
                               &work->dst, work->dst_data,
                               &work->src, work->src_data,
                               work->width, work->height);
}

static bool generic_rso_to_uif(
   const CopyAlgorithm_t *self,
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height)
{
   // Check if this transfer has a fast-path implementation
   if (!self->supports(self, dst, src, width, height))
      return false;

   uint32_t block_width  = self->block_width;
   uint32_t block_height = self->block_height;
   uint32_t block_rows   = height / block_height;
   uint32_t block_cols   = width  / block_width;

   /* Special case for parallel copy */
   bool par_copy = dst->x == 0 && dst->y == 0 &&
                   src->x == 0 && src->y == 0 &&
                   width  % block_width  == 0 &&
                   height % block_height == 0 &&
                   block_rows >= 2 &&
                   block_cols >= 4;

   uint32_t numThreads = v3d_parallel_get_num_threads();

   /* Is it sensible to do a multi-threaded copy? */
   if (!par_copy || numThreads < 2)
      return generic_rso_to_uif_internal(self, dst, dst_data, src, src_data, width, height);

   uint32_t usedThreads   = block_rows < numThreads ? block_rows : numThreads;

   /* if (!par_copy)
      usedThreads = 1; */

   uint32_t blockRowsPerThread = block_rows / usedThreads;
   uint32_t blockRowsRemaining = block_rows;

   RSOtoUIFWork   work[V3D_PARALLEL_MAX_THREADS];
   void          *data[V3D_PARALLEL_MAX_THREADS];

   memset(data, 0, sizeof(void *) * V3D_PARALLEL_MAX_THREADS);

   uint32_t  y      = 0;
   uint32_t  y_step = blockRowsPerThread * block_height;

   for (unsigned int t = 0; t < usedThreads; t++)
   {
      RSOtoUIFWork  *workItem = &work[t];
      bool           last     = t == (usedThreads - 1);

      workItem->self     = self;
      workItem->dst      = *dst;
      workItem->dst_data = dst_data;
      workItem->src      = *src;
      workItem->src_data = src_data;
      workItem->width    = width;
      workItem->height   = last ? blockRowsRemaining * block_height : y_step;

      /* Adjust y for this set of blocks */
      /* dst->y and src->y are currenty always 0, but this could change */
      workItem->dst.y = y + dst->y;
      workItem->src.y = y + src->y;

      data[t] = workItem;

      y += block_height * blockRowsPerThread;
      blockRowsRemaining -= blockRowsPerThread;
   }

   v3d_parallel_exec(generic_rso_to_uif_sub, usedThreads, data);

   return true;
}

void copy_ntrows_etc64_rso_to_etc64_uif(uint8_t *src_ptr, uint32_t *dst_ptr)
{
   uint8x16_t   q0, q1, q2, q3;

   q0 = vld1q_u8(&src_ptr[0x00]);
   q1 = vld1q_u8(&src_ptr[0x10]);
   q2 = vld1q_u8(&src_ptr[0x20]);
   q3 = vld1q_u8(&src_ptr[0x30]);

   vst1q_u32(&dst_ptr[0x00], vreinterpretq_u32_u8(q0));
   vst1q_u32(&dst_ptr[0x04], vreinterpretq_u32_u8(q1));
   vst1q_u32(&dst_ptr[0x10], vreinterpretq_u32_u8(q2));
   vst1q_u32(&dst_ptr[0x14], vreinterpretq_u32_u8(q3));
}

static bool etc64rso_to_etc64uif(
   const CopyAlgorithm_t *self,
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height)
{
   if (!self->supports(self, dst, src, width, height))
      return false;

   uint8_t   *src0_ptr   = (uint8_t *)src_data + src->desc.planes[0].offset;
   uint32_t  *dst0_ptr   = dst_data;
   uint32_t   tile_pitch = src->desc.planes[0].pitch;;

   GFX_LFMT_BASE_DETAIL_T  dst_bd;

   gfx_lfmt_base_detail(&dst_bd, dst->desc.planes[0].lfmt);

   // ETC 64-bit format UIF blocks 32 x 16 pixels (8 x 4 nano tiles or 2 ETC blocks)
   uint32_t x_in_blocks = width  / self->block_width;
   uint32_t y_in_blocks = height / self->block_height;

   uint32_t block_x;
   uint32_t block_y;

   for (block_y = 0; block_y < y_in_blocks; ++block_y)
   {
      for (block_x = 0; block_x < x_in_blocks; ++block_x)
      {
         uint8_t  *src_ptr = src0_ptr + block_y * 4 * tile_pitch + 64 * block_x;
         uint32_t *dst_ptr = gfx_buffer_block_p(&dst->desc.planes[0], &dst_bd, dst0_ptr,
                                                block_x * 8, block_y * 4, 0, dst->desc.height); // x,y is nano-tile in ETC case

         uint32_t utrow;
         uint32_t ntrow;

         for (utrow = 0; utrow < 2; ++utrow)
         {
            for (ntrow = 0; ntrow < 2; ++ntrow)
            {
               copy_ntrows_etc64_rso_to_etc64_uif(src_ptr, dst_ptr);
               src_ptr += tile_pitch;
               dst_ptr += 8;
            }

            dst_ptr += 16; /* have already added 16 */
         }
      }
   }

   return true;
}

static bool rso_b5g6r5_to_rso_r8g8b8a8(
   const CopyAlgorithm_t *self,
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height)
{
   if (!self->supports(self, dst, src, width, height))
      return false;

   bool  yflip = gfx_lfmt_get_yflip(&src->desc.planes[0].lfmt) != 0;

   uint8_t  *src0_ptr  = (uint8_t *)src_data + src->desc.planes[0].offset;
   uint8_t  *dst0_ptr  = (uint8_t *)dst_data + dst->desc.planes[0].offset;

   uint32_t src0_pitch = src->desc.planes[0].pitch;
   uint32_t dst0_pitch = dst->desc.planes[0].pitch;

   int32_t x;
   int32_t y;

   bool  use_scalar = ((width & 0x3) != 0);

   for (y = 0; y < height; ++y)
   {
      uint32_t yd = y + dst->y;
      uint32_t ys = y + src->y;

      if (yflip)
         ys = src->desc.height - 1 - ys;

      uint16_t *src_ptr = (uint16_t *)(src0_ptr + ys * src0_pitch) + src->x;
      uint32_t *dst_ptr = (uint32_t *)(dst0_ptr + yd * dst0_pitch) + dst->x;

      if (use_scalar)
      {
         // Use scalar loop
         for (x = 0; x < width; ++x, ++src_ptr, ++dst_ptr)
         {
            uint16_t    pixel = *src_ptr;
            uint32_t    r, g, b, a;
            uint32_t    rgba;

            b = (pixel & 0x001f) >> 0;  b = b << 3 | b >> 2;
            g = (pixel & 0x07e0) >> 5;  g = g << 2 | g >> 4;
            r = (pixel & 0xf800) >> 11; r = r << 3 | r >> 2;
            a = 0xff;

            rgba = r | g << 8 | b << 16 | a << 24;

            *dst_ptr = rgba;
         }
      }
      else
      {
         uint8x8x4_t  res;

         res.val[3] = vdup_n_u8(0xff);

         // Use Neon vector loop
         for (x = 0; x < width; x += 4)
         {
            uint8x8_t   rgb, r, g, b;
            uint16x4_t  rgb16, r16, g16, b16;

            rgb   = vld1_u8((uint8_t *)src_ptr);
            rgb16 = vreinterpret_u16_u8(rgb);

            src_ptr += 4;

            r16 = vshr_n_u16(rgb16, 8);
            r   = vreinterpret_u8_u16(r16);
            r   = vsri_n_u8(r, r, 5);

            g16 = vshl_n_u16(rgb16, 5);
            g16 = vshr_n_u16(g16, 8);
            g   = vreinterpret_u8_u16(g16);
            g   = vsri_n_u8(g, g, 6);

            b16 = vshl_n_u16(rgb16, 11);
            b16 = vshr_n_u16(b16, 8);
            b   = vreinterpret_u8_u16(b16);
            b   = vsri_n_u8(b, b, 5);

            res.val[0] = r;
            res.val[1] = g;
            res.val[2] = b;

            vst4_lane_u8((uint8_t *)&dst_ptr[0x00], res, 0);
            vst4_lane_u8((uint8_t *)&dst_ptr[0x01], res, 2);
            vst4_lane_u8((uint8_t *)&dst_ptr[0x02], res, 4);
            vst4_lane_u8((uint8_t *)&dst_ptr[0x03], res, 6);

            dst_ptr += 4;
         }
      }
   }

   return true;
}

static bool rso32_to_rso32(
   const CopyAlgorithm_t *self,
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height)
{
   if (!self->supports(self, dst, src, width, height))
      return false;

   bool  yflip = gfx_lfmt_get_yflip(&src->desc.planes[0].lfmt) != 0;

   uint8_t  *src0_ptr  = (uint8_t *)src_data + src->desc.planes[0].offset;
   uint8_t  *dst0_ptr  = (uint8_t *)dst_data + dst->desc.planes[0].offset;

   uint32_t src0_pitch = src->desc.planes[0].pitch;
   uint32_t dst0_pitch = dst->desc.planes[0].pitch;

   int32_t y;

   for (y = 0; y < height; ++y)
   {
      uint32_t yd = y + dst->y;
      uint32_t ys = y + src->y;

      if (yflip)
         ys = src->desc.height - 1 - ys;

      uint32_t *src_ptr = (uint32_t *)(src0_ptr + ys * src0_pitch) + src->x;
      uint32_t *dst_ptr = (uint32_t *)(dst0_ptr + yd * dst0_pitch) + dst->x;

      memcpy(dst_ptr, src_ptr, width * 4);
   }

   return true;
}

static bool rso_r8g8b8x8_to_rso_r8g8b8a8(
   const CopyAlgorithm_t *self,
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height)
{
   if (!self->supports(self, dst, src, width, height))
      return false;

   bool  yflip = gfx_lfmt_get_yflip(&src->desc.planes[0].lfmt) != 0;

   uint8_t  *src0_ptr  = (uint8_t *)src_data + src->desc.planes[0].offset;
   uint8_t  *dst0_ptr  = (uint8_t *)dst_data + dst->desc.planes[0].offset;

   uint32_t src0_pitch = src->desc.planes[0].pitch;
   uint32_t dst0_pitch = dst->desc.planes[0].pitch;

   int32_t x, y;

   for (y = 0; y < height; ++y)
   {
      uint32_t yd = y + dst->y;
      uint32_t ys = y + src->y;

      if (yflip)
         ys = src->desc.height - 1 - ys;

      uint32_t *src_ptr = (uint32_t *)(src0_ptr + ys * src0_pitch) + src->x;
      uint32_t *dst_ptr = (uint32_t *)(dst0_ptr + yd * dst0_pitch) + dst->x;

      for (x = 0; x < width; ++x, ++dst_ptr, ++src_ptr)
         *dst_ptr = *src_ptr | 0xff000000;
   }

   return true;
}

static CopyAlgorithm_t copy_rso128_to_uif128 =
{
   .copy       = generic_rso_to_uif,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = copy_rows_rso128_to_uif128,
   .r2u_n_rows       = 2,
   .r2u_copy_pixel   = copy_pixel_rso128_to_uif128,

   .u2r_copy_pixel   = NULL,

   .block_width  = 4,
   .block_height = 4,
   .src_bpp      = 128,
   .dst_bpp      = 128
};

static CopyAlgorithm_t copy_uif128_to_rso128 =
{
   .copy       = generic_uif_to_rso,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = NULL,
   .r2u_n_rows       = 0,
   .r2u_copy_pixel   = NULL,

   .u2r_copy_pixel   = copy_pixel_uif128_to_rso128,

   .block_width  = 4,
   .block_height = 4,
   .src_bpp      = 128,
   .dst_bpp      = 128
};

static CopyAlgorithm_t copy_rso64_to_uif64 =
{
   .copy         = generic_rso_to_uif,
   .supports     = supports_everything,

   .r2u_copy_n_rows  = copy_rows_rso64_to_uif64,
   .r2u_n_rows       = 2,
   .r2u_copy_pixel   = copy_pixel_rso64_to_uif64,

   .u2r_copy_pixel   = NULL,

   .block_width  = 8,
   .block_height = 4,
   .src_bpp      = 64,
   .dst_bpp      = 64
};

static CopyAlgorithm_t copy_uif64_to_rso64 =
{
   .copy       = generic_uif_to_rso,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = NULL,
   .r2u_n_rows       = 0,
   .r2u_copy_pixel   = NULL,

   .u2r_copy_pixel   = copy_pixel_uif64_to_rso64,

   .block_width  = 8,
   .block_height = 4,
   .src_bpp      = 64,
   .dst_bpp      = 64
};

static CopyAlgorithm_t copy_rso32_to_uif32 =
{
   .copy       = generic_rso_to_uif,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = copy_rows_rso32_to_uif32,
   .r2u_n_rows       = 4,
   .r2u_copy_pixel   = copy_pixel_rso32_to_uif32,

   .u2r_copy_pixel   = NULL,

   .block_width  = 8,
   .block_height = 8,
   .src_bpp      = 32,
   .dst_bpp      = 32
};

static CopyAlgorithm_t copy_uif32_to_rso32 =
{
   .copy       = generic_uif_to_rso,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = NULL,
   .r2u_n_rows       = 0,
   .r2u_copy_pixel   = NULL,

   .u2r_copy_pixel   = copy_pixel_uif32_to_rso32,

   .block_width  = 8,
   .block_height = 8,
   .src_bpp      = 32,
   .dst_bpp      = 32
};

static CopyAlgorithm_t copy_rso32_to_uif32_rbswap =
{
   .copy       = generic_rso_to_uif,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = copy_rows_rso32_to_uif32_rbswap,
   .r2u_n_rows       = 4,
   .r2u_copy_pixel   = copy_pixel_rso32_to_uif32_rbswap,

   .u2r_copy_pixel   = NULL,

   .block_width  = 8,
   .block_height = 8,
   .src_bpp      = 32,
   .dst_bpp      = 32
};

static CopyAlgorithm_t copy_rso_c8c8c8_to_uif_c8c8c8x8 =
{
   .copy       = generic_rso_to_uif,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = copy_row_rso_c8c8c8_to_uif_c8c8c8x8,
   .r2u_n_rows       = 1,
   .r2u_copy_pixel   = copy_pixel_rso_c8c8c8_to_uif_c8c8c8x8,

   .u2r_copy_pixel   = NULL,

   .block_width  = 8,
   .block_height = 8,
   .src_bpp      = 24,
   .dst_bpp      = 32
};

static CopyAlgorithm_t copy_rso16_to_uif16 =
{
   .copy         = generic_rso_to_uif,
   .supports     = supports_everything,

   .r2u_copy_n_rows  = copy_rows_rso32_to_uif32, /* Just a dumb copy */
   .r2u_n_rows       = 4,
   .r2u_copy_pixel   = copy_pixel_rso16_to_uif16,

   .u2r_copy_pixel   = NULL,

   .block_width  = 16,
   .block_height = 8,
   .src_bpp      = 16,
   .dst_bpp      = 16
};

static CopyAlgorithm_t copy_uif16_to_rso16 =
{
   .copy         = generic_uif_to_rso,
   .supports     = supports_everything,

   .r2u_copy_n_rows  = NULL,
   .r2u_n_rows       = 0,
   .r2u_copy_pixel   = NULL,

   .u2r_copy_pixel   = copy_pixel_uif16_to_rso16,

   .block_width  = 16,
   .block_height = 8,
   .src_bpp      = 16,
   .dst_bpp      = 16
};

static CopyAlgorithm_t copy_rso8_to_uif8 =
{
   .copy       = generic_rso_to_uif,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = copy_rows_rso8_to_uif8,
   .r2u_n_rows       = 8,
   .r2u_copy_pixel   = copy_pixel_rso8_to_uif8,

   .u2r_copy_pixel   = NULL,

   .block_width  = 16,
   .block_height = 16,
   .src_bpp      = 8,
   .dst_bpp      = 8
};

static CopyAlgorithm_t copy_uif8_to_rso8 =
{
   .copy         = generic_uif_to_rso,
   .supports     = supports_everything,

   .r2u_copy_n_rows  = NULL,
   .r2u_n_rows       = 0,
   .r2u_copy_pixel   = NULL,

   .u2r_copy_pixel   = copy_pixel_uif8_to_rso8,

   .block_width  = 16,
   .block_height = 16,
   .src_bpp      = 8,
   .dst_bpp      = 8
};

static CopyAlgorithm_t copy_etc64rso_to_etc64uif =
{
   .copy       = etc64rso_to_etc64uif,
   .supports   = supports_conservative,

   .r2u_copy_n_rows  = NULL,
   .r2u_n_rows       = 0,
   .r2u_copy_pixel   = NULL,

   .u2r_copy_pixel   = NULL,

   .block_width  = 32,
   .block_height = 16,
   .src_bpp      = 4,
   .dst_bpp      = 4
};

static CopyAlgorithm_t copy_rso_b5g6r5_to_rso_r8g8b8a8 =
{
   .copy       = rso_b5g6r5_to_rso_r8g8b8a8,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = NULL,
   .r2u_n_rows       = 0,
   .r2u_copy_pixel   = NULL,

   .u2r_copy_pixel   = NULL,

   .block_width  = 1,
   .block_height = 1,
   .src_bpp      = 16,
   .dst_bpp      = 16
};

static CopyAlgorithm_t copy_rso32_to_rso32 =
{
   .copy       = rso32_to_rso32,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = NULL,
   .r2u_n_rows       = 0,
   .r2u_copy_pixel   = NULL,

   .u2r_copy_pixel   = NULL,

   .block_width  = 1,
   .block_height = 1,
   .src_bpp      = 32,
   .dst_bpp      = 32
};

static CopyAlgorithm_t copy_rso_r8g8b8x8_to_rso_r8g8b8a8 =
{
   .copy       = rso_r8g8b8x8_to_rso_r8g8b8a8,
   .supports   = supports_everything,

   .r2u_copy_n_rows  = NULL,
   .r2u_n_rows       = 0,
   .r2u_copy_pixel   = NULL,

   .u2r_copy_pixel   = NULL,

   .block_width  = 1,
   .block_height = 1,
   .src_bpp      = 32,
   .dst_bpp      = 32
};

static bool is_rso_to_uif_match(GFX_LFMT_T src_lfmt, GFX_LFMT_T dst_lfmt)
{
   return
      gfx_lfmt_is_rso(src_lfmt)                                    &&           /* source is rso      */
      (gfx_lfmt_is_uif(dst_lfmt) || gfx_lfmt_is_uif_xor(dst_lfmt)) &&           /* dst is uif variant */
      (!gfx_lfmt_get_yflip(&dst_lfmt))                             &&           /* dst is not flipped */
      ((dst_lfmt & GFX_LFMT_FORMAT_MASK) == (src_lfmt & GFX_LFMT_FORMAT_MASK)); /* formats match      */
}

static bool is_uif_to_rso_match(GFX_LFMT_T src_lfmt, GFX_LFMT_T dst_lfmt)
{
   return
      gfx_lfmt_is_rso(dst_lfmt)                                    &&           /* source is rso      */
      (gfx_lfmt_is_uif(src_lfmt) || gfx_lfmt_is_uif_xor(src_lfmt)) &&           /* dst is uif variant */
      (!gfx_lfmt_get_yflip(&src_lfmt))                             &&           /* dst is not flipped */
      ((src_lfmt & GFX_LFMT_FORMAT_MASK) == (dst_lfmt & GFX_LFMT_FORMAT_MASK)); /* formats match      */
}

static bool is_clamping(GFX_LFMT_T lfmt)
{
   return gfx_lfmt_has_depth(lfmt); // || gfx_lfmt_contains_snorm(lfmt);  Shouldn't need to clamp snorms
}

static uint32_t lfmt_bpp(GFX_LFMT_T lfmt)
{
   switch (gfx_lfmt_get_base(&lfmt))
   {
   /* 128 bpp */
   case GFX_LFMT_BASE_C32_C32_C32_C32   :
      return 128;

   /* 64 bpp */
   case GFX_LFMT_BASE_C32_C32           :
   case GFX_LFMT_BASE_C16C16C16C16      :
   case GFX_LFMT_BASE_C16_C16_C16_C16   :
      return 64;

   /* 32 bpp */
   case GFX_LFMT_BASE_C32               :
   case GFX_LFMT_BASE_C16C16            :
   case GFX_LFMT_BASE_C16_C16           :
   case GFX_LFMT_BASE_C8C8C8C8          :
   case GFX_LFMT_BASE_C8_C8_C8_C8       :
   case GFX_LFMT_BASE_C10C10C10C2       :
   case GFX_LFMT_BASE_C11C11C10         :
   case GFX_LFMT_BASE_C24C8             :
   case GFX_LFMT_BASE_C8C24             :
   case GFX_LFMT_BASE_C9C9C9SHAREDEXP5  :
      return 32;

   /* 16 bpp */
   case GFX_LFMT_BASE_C16               :
   case GFX_LFMT_BASE_C8C8              :
   case GFX_LFMT_BASE_C8_C8             :
   case GFX_LFMT_BASE_C5C6C5            :
   case GFX_LFMT_BASE_C5C5C5C1          :
   case GFX_LFMT_BASE_C1C5C5C5          :
   case GFX_LFMT_BASE_C4C4C4C4          :
      return 16;

   case GFX_LFMT_BASE_C8                :
      return 8;

   default:
      break;
   }

   return 0;
}

static const CopyAlgorithm_t *find_fast_algorithm(
   const struct v3d_imgconv_base_tgt *dst, const struct v3d_imgconv_base_tgt *src,
   unsigned int width, unsigned int height, unsigned int depth)
{
   const CopyAlgorithm_t   *alg = NULL;

   if (depth != 1)
      return NULL;

   GFX_LFMT_T dst_lfmt = dst->desc.planes[0].lfmt;
   GFX_LFMT_T src_lfmt = src->desc.planes[0].lfmt;

   /* Only handling 2D conversions */
   if (!gfx_lfmt_is_2d(dst_lfmt) ||
       !gfx_lfmt_is_2d(src_lfmt))
       return NULL;

   /* First check for exact format matches
      for rso->uif conversions and use the generic n-bit routines
      dst must not be flipped
    */
   if (is_rso_to_uif_match(src_lfmt, dst_lfmt) && !is_clamping(src_lfmt))
   {
      switch (lfmt_bpp(src_lfmt))
      {
      case 128 :  alg = &copy_rso128_to_uif128; break;
      case 64  :  alg = &copy_rso64_to_uif64;   break;
      case 32  :  alg = &copy_rso32_to_uif32;   break;
      case 16  :  alg = &copy_rso16_to_uif16;   break;
      case 8   :  alg = &copy_rso8_to_uif8;     break;
      default  :  break;
      }
   }
   else if (is_uif_to_rso_match(src_lfmt, dst_lfmt))
   {
      switch (lfmt_bpp(src_lfmt))
      {
      case 128 :  alg = &copy_uif128_to_rso128; break;
      case 64  :  alg = &copy_uif64_to_rso64;   break;
      case 32  :  alg = &copy_uif32_to_rso32;   break;
      case 16  :  alg = &copy_uif16_to_rso16;   break;
      case 8   :  alg = &copy_uif8_to_rso8;     break;
      default  :  break;
      }
   }

   if (alg != NULL)
      return alg;

   /* Special cases with fast paths */
   switch ((uint32_t)src_lfmt)
   {
   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP,  BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_BGRA, PRE_NONPRE):
   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_YFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_BGRA, PRE_NONPRE):
      switch ((uint32_t)dst_lfmt)
      {
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_UIF_XOR, YFLIP_NOYFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBA, PRE_NONPRE):
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_UIF,     YFLIP_NOYFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBA, PRE_NONPRE):
         alg = &copy_rso32_to_uif32_rbswap;
         break;
      }
      break;

   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP,  BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBA, PRE_NONPRE):
   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_YFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBA, PRE_NONPRE):
      switch ((uint32_t)dst_lfmt)
      {
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBA, PRE_NONPRE):
         alg = &copy_rso32_to_rso32;
         break;
      }
      break;

   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP,  BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBX, PRE_NONPRE):
   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_YFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBX, PRE_NONPRE):
      switch ((uint32_t)dst_lfmt)
      {
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBX, PRE_NONPRE):
         alg = &copy_rso32_to_rso32;
         break;

      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBA, PRE_NONPRE):
         alg = &copy_rso_r8g8b8x8_to_rso_r8g8b8a8;
         break;
      }
      break;

   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP,  BASE_C8_C8_C8, TYPE_UNORM, CHANNELS_RGB, PRE_NONPRE):
   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_YFLIP, BASE_C8_C8_C8, TYPE_UNORM, CHANNELS_RGB, PRE_NONPRE):
      switch ((uint32_t)dst_lfmt)
      {
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_UIF_XOR, YFLIP_NOYFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBX, PRE_NONPRE):
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_UIF,     YFLIP_NOYFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBX, PRE_NONPRE):
         alg = &copy_rso_c8c8c8_to_uif_c8c8c8x8;
         break;
      }
      break;

   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP,  BASE_C5C6C5, TYPE_UNORM, CHANNELS_BGR, PRE_NONPRE):
   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_YFLIP, BASE_C5C6C5, TYPE_UNORM, CHANNELS_BGR, PRE_NONPRE):
      switch ((uint32_t)dst_lfmt)
      {
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP, BASE_C8_C8_C8_C8, TYPE_UNORM, CHANNELS_RGBA, PRE_NONPRE):
         alg = &copy_rso_b5g6r5_to_rso_r8g8b8a8;
         break;
      }
      break;

   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP, BASE_ETC1, TYPE_UNORM, CHANNELS_RGB, PRE_NONPRE):
   case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_RSO, YFLIP_NOYFLIP, BASE_ETC2, TYPE_UNORM, CHANNELS_RGB, PRE_NONPRE):
      switch ((uint32_t)dst_lfmt)
      {
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_UIF_XOR, YFLIP_NOYFLIP, BASE_ETC1, TYPE_UNORM, CHANNELS_RGB, PRE_NONPRE):
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_UIF,     YFLIP_NOYFLIP, BASE_ETC1, TYPE_UNORM, CHANNELS_RGB, PRE_NONPRE):
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_UIF_XOR, YFLIP_NOYFLIP, BASE_ETC2, TYPE_UNORM, CHANNELS_RGB, PRE_NONPRE):
      case GFX_LFMT_MAKE(DIMS_2D, SWIZZLING_UIF,     YFLIP_NOYFLIP, BASE_ETC2, TYPE_UNORM, CHANNELS_RGB, PRE_NONPRE):
         alg = &copy_etc64rso_to_etc64uif;
         break;
      }
      break;
   }

   return alg;
}

static bool claim_fast_path(
   const struct v3d_imgconv_base_tgt *dst, const struct v3d_imgconv_base_tgt *src,
   unsigned int width, unsigned int height, unsigned int depth)
{
   const CopyAlgorithm_t   *alg = find_fast_algorithm(dst, src, width, height, depth);

   return alg != NULL && alg->supports(alg, dst, src, width, height);
}

static bool fast_path_convert(
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height, unsigned int depth)
{
   bool                     ok;
   const CopyAlgorithm_t   *alg = find_fast_algorithm(dst, src, width, height, depth);

   if (vcos_is_log_enabled(VCOS_LOG_CATEGORY, VCOS_LOG_INFO) && alg != NULL)
   {
      GFX_LFMT_SPRINT(src_desc, src->desc.planes[0].lfmt);
      GFX_LFMT_SPRINT(dst_desc, dst->desc.planes[0].lfmt);
      vcos_log_info("Neon conversion : %ux%u, srcPitch=%u, dstPitch=%u, from(%s)->to(%s)",
                    width, height, src->desc.planes[0].pitch, dst->desc.planes[0].pitch,
                    src_desc, dst_desc);
   }

   ok = alg != NULL;

   if (ok)
      ok = alg->copy(alg, dst, dst_data, src, src_data, width, height);

#ifdef TESTING
   {
      const CopyAlgorithm_t   *alg2 = find_fast_algorithm(src, dst, width, height, depth);

      if (ok && alg2 != NULL)
      {
         alg2->copy(alg2, src, src_data, dst, dst_data, width, height);
         alg->copy(alg, dst, dst_data, src, src_data, width, height);
      }
   }
#endif

   return ok;
}

static v3d_imgconv_methods neon_path =
{
   .claim         = claim_fast_path,
   .convert_async = NULL,
   .convert_sync  = fast_path_convert,
   .convert_prep  = NULL
};
#endif

const v3d_imgconv_methods* get_neon_path(void)
{
#ifdef USE_NEON_FAST_PATHS
   return &neon_path;
#else
   return NULL;
#endif
}
