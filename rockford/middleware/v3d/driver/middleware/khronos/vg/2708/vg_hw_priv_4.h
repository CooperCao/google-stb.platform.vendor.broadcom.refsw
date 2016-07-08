/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG backend

FILE DESCRIPTION
Platform-specific VG backend stuff.
=============================================================================*/

#ifndef VG_HW_PRIV_4_H
#define VG_HW_PRIV_4_H

#include "middleware/khronos/vg/2708/vg_tess_priv_4.h"
#include "middleware/khronos/vg/vg_hw.h"
#include "middleware/khronos/common/khrn_image.h"

typedef struct VG_BE_RENDER_STATE {
   MEM_HANDLE_T state; /* so we can report errors */

   MEM_HANDLE_T frame, frame_src, mask;
   uint32_t frame_width;
   uint32_t frame_height;
   KHRN_IMAGE_FORMAT_T frame_format;

   uint32_t buffers_acquired;

   bool frame_clear;
   uint32_t frame_clear_rgba;
   bool mask_clear;
   uint8_t mask_clear_value;

   VG_TESS_HANDLE_T tess;

   KHRN_FMEM_T *fmem;

   /* active state at end of bin */
   uint32_t offset;
   uint32_t clip_xy, clip_wh;
   uint32_t cfg;
   MEM_HANDLE_T shader;
   uint32_t *shader_unifs;

   /* glyph stuff */
   MEM_HANDLE_T glyph_fill_shader, glyph_stroke_shader;
   uint32_t *glyph_fill_unifs, *glyph_stroke_unifs;
   uint32_t glyph_fill_unifs_n, glyph_stroke_unifs_n;
   uint64_t glyph_fill_state_id, glyph_stroke_state_id;

   uint32_t extra_thrsws_min; /* we can drop this many thrsws from the start of all shaders. todo: remove on b0 */
   uint32_t batches_n;
   bool     fence_active;
   int      fence; /* fence_active is for GL, whereas fence is fence from the platform */

} VG_BE_RENDER_STATE_T;

extern void vg_be_render_state_flush(VG_BE_RENDER_STATE_T *render_state);

#endif
