/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  VG shaders

FILE DESCRIPTION
Shader generation.
=============================================================================*/

#ifndef VG_SHADER_4_H
#define VG_SHADER_4_H

#include "middleware/khronos/common/khrn_mem.h"

extern void vg_shader_init(void);
extern void vg_shader_term(void);

typedef enum {
   VG_SHADER_FD_DRAW_TYPE_PAINT,
   VG_SHADER_FD_DRAW_TYPE_IMAGE,
   VG_SHADER_FD_DRAW_TYPE_MULTIPLY,
   VG_SHADER_FD_DRAW_TYPE_STENCIL
} VG_SHADER_FD_DRAW_TYPE_T;

typedef enum {
   VG_SHADER_FD_PAINT_TYPE_SOLID,
   VG_SHADER_FD_PAINT_TYPE_GRAD_LINEAR,
   VG_SHADER_FD_PAINT_TYPE_GRAD_RADIAL,
   VG_SHADER_FD_PAINT_TYPE_PATTERN
} VG_SHADER_FD_PAINT_TYPE_T;

typedef enum {
   VG_SHADER_FD_BLEND_SRC,
   VG_SHADER_FD_BLEND_SRC_OVER,
   VG_SHADER_FD_BLEND_DST_OVER,
   VG_SHADER_FD_BLEND_SRC_IN,
   VG_SHADER_FD_BLEND_DST_IN,
   VG_SHADER_FD_BLEND_MULTIPLY,
   VG_SHADER_FD_BLEND_SCREEN,
   VG_SHADER_FD_BLEND_DARKEN,
   VG_SHADER_FD_BLEND_LIGHTEN,
   VG_SHADER_FD_BLEND_ADDITIVE
} VG_SHADER_FD_BLEND_T;

typedef struct {
   uint32_t scissor                 : 1;

   uint32_t draw_type               : 2; /* VG_SHADER_FD_DRAW_TYPE_T */

   uint32_t paint_type              : 2; /* VG_SHADER_FD_PAINT_TYPE_T */
   uint32_t paint_pattern_child     : 1;
   uint32_t paint_pattern_tile_fill : 1;
   uint32_t paint_pattern_rgbl      : 1; /* todo: won't need this for b0 */
   uint32_t paint_pattern_pre       : 1;

   uint32_t image_projective        : 1;
   uint32_t image_child             : 1;
   uint32_t image_rgbl              : 1; /* todo: won't need this for b0 */
   uint32_t image_pre               : 1;

   uint32_t color_transform         : 1;
   uint32_t colorspace_convert      : 1;

   uint32_t blend                   : 4; /* VG_SHADER_FD_BLEND_T */
   uint32_t coverage                : 1;
   uint32_t masking                 : 1;

   uint32_t pre                     : 1;
   uint32_t no_alpha                : 1;
} VG_SHADER_FD_KEY_T;

vcos_static_assert(sizeof(VG_SHADER_FD_KEY_T) == sizeof(uint32_t));

extern uint32_t vg_shader_fd_get_extra_thrsws(VG_SHADER_FD_KEY_T key); /* todo: no need for this on b0 */
extern bool vg_shader_fd_need_stencil_a(VG_SHADER_FD_BLEND_T blend);
extern MEM_HANDLE_T vg_shader_fd_get(VG_SHADER_FD_KEY_T key);
extern const uint32_t VG_SHADER_FD_CLEAR_EXTRA_THRSWS;
extern const uint32_t VG_SHADER_FD_CLEAR[];
extern const uint32_t VG_SHADER_FD_CLEAR_SIZE;

/* todo: get rid of the extra thrsws stuff */
extern const uint32_t VG_SHADER_MD_CLEAR_EXTRA_THRSWS;
extern const uint32_t VG_SHADER_MD_CLEAR[];
extern const uint32_t VG_SHADER_MD_CLEAR_SIZE;
extern const uint32_t VG_SHADER_MD_IMAGE_EXTRA_THRSWS;
extern const uint32_t VG_SHADER_MD_IMAGE_SET[];
extern const uint32_t VG_SHADER_MD_IMAGE_SET_SIZE;
extern const uint32_t VG_SHADER_MD_IMAGE_UNION[];
extern const uint32_t VG_SHADER_MD_IMAGE_UNION_SIZE;
extern const uint32_t VG_SHADER_MD_IMAGE_INTERSECT[];
extern const uint32_t VG_SHADER_MD_IMAGE_INTERSECT_SIZE;
extern const uint32_t VG_SHADER_MD_IMAGE_SUBTRACT[];
extern const uint32_t VG_SHADER_MD_IMAGE_SUBTRACT_SIZE;
extern const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_SET[];
extern const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_SET_SIZE;
extern const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_UNION[];
extern const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_UNION_SIZE;
extern const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_INTERSECT[];
extern const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_INTERSECT_SIZE;
extern const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_SUBTRACT[];
extern const uint32_t VG_SHADER_MD_IMAGE_NO_ALPHA_SUBTRACT_SIZE;
extern const uint32_t VG_SHADER_MD_CVG_EXTRA_THRSWS;
extern const uint32_t VG_SHADER_MD_CVG_SET[];
extern const uint32_t VG_SHADER_MD_CVG_SET_SIZE;
extern const uint32_t VG_SHADER_MD_CVG_UNION[];
extern const uint32_t VG_SHADER_MD_CVG_UNION_SIZE;
extern const uint32_t VG_SHADER_MD_CVG_INTERSECT[];
extern const uint32_t VG_SHADER_MD_CVG_INTERSECT_SIZE;
extern const uint32_t VG_SHADER_MD_CVG_SUBTRACT[];
extern const uint32_t VG_SHADER_MD_CVG_SUBTRACT_SIZE;
extern const uint32_t VG_SHADER_MD_CVG_SCISSOR_EXTRA_THRSWS;
extern const uint32_t VG_SHADER_MD_CVG_SCISSOR_SET[];
extern const uint32_t VG_SHADER_MD_CVG_SCISSOR_SET_SIZE;
extern const uint32_t VG_SHADER_MD_CVG_SCISSOR_UNION[];
extern const uint32_t VG_SHADER_MD_CVG_SCISSOR_UNION_SIZE;
extern const uint32_t VG_SHADER_MD_CVG_SCISSOR_INTERSECT[];
extern const uint32_t VG_SHADER_MD_CVG_SCISSOR_INTERSECT_SIZE;
extern const uint32_t VG_SHADER_MD_CVG_SCISSOR_SUBTRACT[];
extern const uint32_t VG_SHADER_MD_CVG_SCISSOR_SUBTRACT_SIZE;

#endif
