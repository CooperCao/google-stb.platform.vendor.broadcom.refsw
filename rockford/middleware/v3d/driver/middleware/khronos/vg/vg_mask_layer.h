/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  VG mask layer

FILE DESCRIPTION
VG mask layer object handling interface.
=============================================================================*/

#ifndef VG_MASK_LAYER_H
#define VG_MASK_LAYER_H

#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/include/VG/openvg.h"

typedef struct {
   int32_t width;
   int32_t height;
} VG_MASK_LAYER_BPRINT_T;

extern void vg_mask_layer_bprint_from_stem(
   MEM_HANDLE_T handle,
   int32_t width, int32_t height);

extern void vg_mask_layer_bprint_term(void *, uint32_t);

static INLINE bool vg_is_mask_layer_bprint(MEM_HANDLE_T handle)
{
   return mem_get_term(handle) == vg_mask_layer_bprint_term;
}

static INLINE KHRN_IMAGE_FORMAT_T vg_mask_layer_get_format(
   uint32_t width, uint32_t height)
{
   return khrn_image_prefer_lt(A_8_TF, width, height) ? A_8_LT : A_8_TF;
}

typedef struct {
   MEM_HANDLE_T image; /* KHRN_IMAGE_T */
   uint32_t image_width;
   uint32_t image_height;
} VG_MASK_LAYER_T;

extern bool vg_mask_layer_from_bprint(MEM_HANDLE_T handle);

extern void vg_mask_layer_term(void *, uint32_t);

static INLINE bool vg_is_mask_layer(MEM_HANDLE_T handle)
{
   return mem_get_term(handle) == vg_mask_layer_term;
}

#endif
