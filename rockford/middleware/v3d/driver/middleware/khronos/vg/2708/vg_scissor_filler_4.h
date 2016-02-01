/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  VG scissor

FILE DESCRIPTION
Platform-specific scissor stuff. This should only be included by
../vg_scissor.h.
=============================================================================*/

#ifndef VG_SCISSOR_FILLER_4_H
#define VG_SCISSOR_FILLER_4_H

#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"

#define VG_SCISSOR_ALIGN KHRN_HW_TEX_ALIGN

static INLINE KHRN_IMAGE_FORMAT_T vg_scissor_get_format(uint32_t width, uint32_t height)
{
   /* hw decides based on size: we must agree */
   return khrn_image_prefer_lt(L_1_TF, width, height) ? L_1_LT : L_1_TF;
}

#endif
