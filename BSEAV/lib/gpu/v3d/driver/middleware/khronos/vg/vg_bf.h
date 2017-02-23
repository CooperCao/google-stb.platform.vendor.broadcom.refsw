/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG blitting/filtering

FILE DESCRIPTION
VG blitting/filtering interface. Implementation is mostly platform-specific. We
should eventually end up in khrn_bf_* functions, but we might want to do the
heavy lifting (for example) in another thread, which is why we don't just call
the khrn_bf_* functions directly.
=============================================================================*/

#ifndef VG_BF_H
#define VG_BF_H

#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/include/VG/openvg.h"

extern void vg_bf_init(void);
extern void vg_bf_term(void);

extern void vg_bf_clear_region(
   MEM_HANDLE_T handle, uint32_t x, uint32_t y,
   uint32_t width, uint32_t height,
   uint32_t rgba); /* rgba non-lin, unpre */

extern void vg_bf_copy_region(
   MEM_HANDLE_T dst_handle, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   MEM_HANDLE_T src_handle, uint32_t src_x, uint32_t src_y);

extern void vg_bf_copy_scissor_regions(
   MEM_HANDLE_T dst_handle, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   MEM_HANDLE_T src_handle, uint32_t src_x, uint32_t src_y,
   const int32_t *scissor_rects, uint32_t scissor_rects_count);

extern void vg_bf_color_matrix(
   MEM_HANDLE_T dst_handle, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   MEM_HANDLE_T src_handle, uint32_t src_x, uint32_t src_y,
   uint32_t channel_mask, KHRN_IMAGE_FORMAT_T format,
   const float *matrix); /* matrix needs to be clean_float()ed */

extern void vg_bf_lookup(
   MEM_HANDLE_T dst_handle, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   MEM_HANDLE_T src_handle, uint32_t src_x, uint32_t src_y,
   uint32_t channel_mask, KHRN_IMAGE_FORMAT_T format,
   const uint8_t *red_lut, const uint8_t *green_lut, const uint8_t *blue_lut, const uint8_t *alpha_lut,
   KHRN_IMAGE_FORMAT_T output_format);

extern void vg_bf_lookup_single(
   MEM_HANDLE_T dst_handle, uint32_t dst_x, uint32_t dst_y,
   uint32_t width, uint32_t height,
   MEM_HANDLE_T src_handle, uint32_t src_x, uint32_t src_y,
   uint32_t channel_mask, KHRN_IMAGE_FORMAT_T format,
   uint32_t source_channel, const uint32_t *lut,
   KHRN_IMAGE_FORMAT_T output_format);

#define VG_BF_GBLUR_KERNEL_COUNT_MAX 83 /* must be odd. 83 is enough for std dev of 16 with 0.99 tolerance */
#define VG_BF_GBLUR_KERNEL_TOLERANCE 0.99f

extern uint32_t vg_bf_gblur_kernel_gen(int16_t *kernel, float std_dev);

extern void vg_bf_gblur(
   MEM_HANDLE_T dst_handle, uint32_t dst_x, uint32_t dst_y, uint32_t dst_width, uint32_t dst_height,
   MEM_HANDLE_T src_handle, uint32_t src_x, uint32_t src_y, uint32_t src_width, uint32_t src_height,
   uint32_t channel_mask, KHRN_IMAGE_FORMAT_T format,
   VGTilingMode tiling_mode, uint32_t tile_fill_rgba,
   float std_dev_x, float std_dev_y,
   MEM_HANDLE_T state_handle); /* for delayed error reporting */

extern void vg_bf_sconv(
   MEM_HANDLE_T dst_handle, uint32_t dst_x, uint32_t dst_y, uint32_t dst_width, uint32_t dst_height,
   MEM_HANDLE_T src_handle, uint32_t src_x, uint32_t src_y, uint32_t src_width, uint32_t src_height,
   uint32_t channel_mask, KHRN_IMAGE_FORMAT_T format,
   VGTilingMode tiling_mode, uint32_t tile_fill_rgba,
   int32_t shift_x, int32_t shift_y,
   const int16_t *kernel_x, uint32_t kernel_width,
   const int16_t *kernel_y, uint32_t kernel_height,
   float scale, float bias,
   MEM_HANDLE_T state_handle); /* for delayed error reporting */

extern void vg_bf_conv(
   MEM_HANDLE_T dst_handle, uint32_t dst_x, uint32_t dst_y, uint32_t dst_width, uint32_t dst_height,
   MEM_HANDLE_T src_handle, uint32_t src_x, uint32_t src_y, uint32_t src_width, uint32_t src_height,
   uint32_t channel_mask, KHRN_IMAGE_FORMAT_T format,
   VGTilingMode tiling_mode, uint32_t tile_fill_rgba,
   int32_t shift_x, int32_t shift_y,
   const int16_t *kernel, uint32_t kernel_width, uint32_t kernel_height,
   float scale, float bias);

extern void vg_bf_wait(void);

#endif
