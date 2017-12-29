/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/khrn_image.h"
#include "interface/khronos/common/khrn_int_image.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/common/2708/khrn_fmem_4.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/2708/khrn_interlock_priv_4.h"

/*
   is the format ok for rendering to? this is only for color buffer formats, not
   z/stencil formats etc
*/

bool khrn_image_is_ok_for_render_target(KHRN_IMAGE_FORMAT_T format, bool ignore_mem_layout)
{
   assert(khrn_image_is_color(format));

   format = khrn_image_no_layout_format(format);
   format = khrn_image_no_colorspace_format(format);

   switch (format) {
   case ABGR_8888:
   case XBGR_8888:
   case RGBA_8888:
   case RGBX_8888:
   case RGB_565:
      /* need tformat on a0 for vg, but allow rso anyway as it's faster and will
       * only appear when we build with RSO_FRAMEBUFFER defined (in which case
       * we should be aware that some stuff might not work) */
      return ignore_mem_layout || khrn_image_is_tformat(format) || khrn_image_is_rso(format)
         || khrn_image_is_lineartile(format)
         ;
   default:
      return false;
   }
}

/*
   should only be called if khrn_image_is_ok_for_render_target(true) says yes.
   checks the other image properties, eg memory layout, stride, size, alignment.
   again, this is only for color buffers, not z/stencil etc
*/

bool khrn_image_can_use_as_render_target(KHRN_IMAGE_T *image)
{
   assert(khrn_image_is_ok_for_render_target(image->format, true));
   if (/* check the memory layout */
      !khrn_image_is_ok_for_render_target(image->format, false) ||
      (image->stride & (KHRN_HW_TLB_ALIGN - 1)) ||
      (khrn_image_get_align(image) < KHRN_HW_TLB_ALIGN)) {
      assert(!(image->flags & IMAGE_FLAG_RENDER_TARGET));
      return false;
   }
   image->flags |= IMAGE_FLAG_RENDER_TARGET;
   return true;
}

/*
   fudge the image parameters to make it ok for the requested usage. the render
   target usage includes z/stencil/mask buffers as well as color buffers
*/

void khrn_image_platform_fudge(
   KHRN_IMAGE_FORMAT_T *format,
   uint32_t *padded_width, uint32_t *padded_height,
   uint32_t *align, KHRN_IMAGE_CREATE_FLAG_T flags)
{
   if (flags & IMAGE_CREATE_FLAG_TEXTURE) {
      if (!(flags & IMAGE_CREATE_FLAG_RSO_TEXTURE)) {
         *format = khrn_image_to_tf_format(*format);
         if (khrn_image_prefer_lt(*format, *padded_width, *padded_height)) {
            *format = khrn_image_to_lt_format(*format);
         }
      }
      else
      {
         /* height multiple of 4 */
         *padded_height = (*padded_height + 3) & ~0x3;
      }
      *align = _max(*align, KHRN_HW_TEX_ALIGN);
   }
   if (flags & IMAGE_CREATE_FLAG_RENDER_TARGET) {
      /* if it looks like a color buffer, check it's one of the supported
       * formats */
      assert(!khrn_image_is_color(*format) ||
         (*format == A_8_RSO) || /* A_8_RSO used for mask buffers */
         (*format == COL_32_TLBD) || /* COL_32_TLBD used for multisample buffers */
         khrn_image_is_ok_for_render_target(*format, false));
      /* stride needs to be a multiple of KHRN_HW_TLB_ALIGN. this is trivially
       * satisfied for lineartile and tformat as utiles are 16 bytes wide. need
       * to pad raster order widths though... */
      if (khrn_image_is_rso(*format)) {
         *padded_width = round_up(*padded_width, (KHRN_HW_TLB_ALIGN * 8) / khrn_image_get_bpp(*format));
      }
      *align = _max(*align, KHRN_HW_TLB_ALIGN);
   }
   if (flags & IMAGE_CREATE_FLAG_DISPLAY) {
      *align = _max(*align, 4096); /* need 4k alignment for scaler */
   }
}
