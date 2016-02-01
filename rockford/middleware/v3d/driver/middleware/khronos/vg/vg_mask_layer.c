/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  VG mask layer

FILE DESCRIPTION
VG mask layer object handling implementation.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/vg/vg_int_config.h"
#include "middleware/khronos/vg/vg_mask_layer.h"
#include "middleware/khronos/vg/vg_stem.h"

vcos_static_assert(sizeof(VG_MASK_LAYER_BPRINT_T) <= VG_STEM_SIZE);
vcos_static_assert(alignof(VG_MASK_LAYER_BPRINT_T) <= VG_STEM_ALIGN);

void vg_mask_layer_bprint_term(void *p, uint32_t size)
{
   UNUSED(p);
   UNUSED(size);
}

void vg_mask_layer_bprint_from_stem(
   MEM_HANDLE_T handle,
   int32_t width, int32_t height)
{
   VG_MASK_LAYER_BPRINT_T *mask_layer_bprint;

   vcos_assert(vg_is_stem(handle));
   vcos_assert((width > 0) && (height > 0) &&
      (width <= VG_CONFIG_MAX_IMAGE_WIDTH) && (height <= VG_CONFIG_MAX_IMAGE_HEIGHT) &&
      ((width * height) <= VG_CONFIG_MAX_IMAGE_PIXELS)); /* checks performed on client-side */

   mask_layer_bprint = (VG_MASK_LAYER_BPRINT_T *)mem_lock(handle, NULL);
   mask_layer_bprint->width = width;
   mask_layer_bprint->height = height;
   mem_unlock(handle);

   mem_set_desc(handle, "VG_MASK_LAYER_BPRINT_T");
   mem_set_term(handle, vg_mask_layer_bprint_term);
}

vcos_static_assert(sizeof(VG_MASK_LAYER_T) <= VG_STEM_SIZE);
vcos_static_assert(alignof(VG_MASK_LAYER_T) <= VG_STEM_ALIGN);

void vg_mask_layer_term(void *p, uint32_t size)
{
   VG_MASK_LAYER_T *mask_layer = (VG_MASK_LAYER_T *)p;

   UNUSED(size);

   mem_release(mask_layer->image);
}

bool vg_mask_layer_from_bprint(MEM_HANDLE_T handle)
{
   VG_MASK_LAYER_BPRINT_T *mask_layer_bprint;
   int32_t width, height;
   MEM_HANDLE_T image_handle;
   VG_MASK_LAYER_T *mask_layer;

   vcos_assert(vg_is_mask_layer_bprint(handle));

   /*
      get init params from blueprint
   */

   mask_layer_bprint = (VG_MASK_LAYER_BPRINT_T *)mem_lock(handle, NULL);
   width = mask_layer_bprint->width;
   height = mask_layer_bprint->height;
   mem_unlock(handle);

   /*
      alloc data
   */

   image_handle = khrn_image_create(vg_mask_layer_get_format(width, height), width, height,
      (KHRN_IMAGE_CREATE_FLAG_T)(IMAGE_CREATE_FLAG_ONE | IMAGE_CREATE_FLAG_TEXTURE));
   if (image_handle == MEM_INVALID_HANDLE) {
      return false;
   }

   /*
      fill in struct
   */

   mask_layer = (VG_MASK_LAYER_T *)mem_lock(handle, NULL);
   mask_layer->image = image_handle;
   mask_layer->image_width = width;
   mask_layer->image_height = height;
   mem_unlock(handle);

   mem_set_desc(handle, "VG_MASK_LAYER_T");
   mem_set_term(handle, vg_mask_layer_term);

   return true;
}
