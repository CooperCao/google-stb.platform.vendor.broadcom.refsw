/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Copy buffer

FILE DESCRIPTION
Implementation.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/2708/khrn_copy_buffer_4.h"
#include "middleware/khronos/common/2708/khrn_interlock_priv_4.h"
#include "middleware/khronos/common/2708/khrn_render_state_4.h"
#include "middleware/khronos/common/2708/khrn_prod_4.h"
#include "middleware/khronos/common/khrn_hw.h"
#include "middleware/khronos/common/khrn_image.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/2708/khrn_fmem_4.h"

#define TLB_INVALID_FORMAT 3

static uint32_t get_dest_format(KHRN_IMAGE_FORMAT_T format)
{
   switch (format & IMAGE_FORMAT_PIXEL_LAYOUT_MASK)
   {
   case IMAGE_FORMAT_8888 : return 1;
   case IMAGE_FORMAT_565  : return 2;
   default                : return TLB_INVALID_FORMAT;
   }
}

static uint32_t get_src_format(KHRN_IMAGE_FORMAT_T format)
{
   switch (format & IMAGE_FORMAT_PIXEL_LAYOUT_MASK)
   {
   case IMAGE_FORMAT_8888 : return 0;
   case IMAGE_FORMAT_565  : return 2;
   default                : return TLB_INVALID_FORMAT;
   }
}

static uint32_t get_type_flag(KHRN_IMAGE_FORMAT_T format)
{
   if (khrn_image_is_tformat(format))
      return 1;
   else if (khrn_image_is_lineartile(format))
      return 2;
   else // if (khrn_image_is_rso(format))
      return 0;
}

bool khrn_delayed_copy_buffer(MEM_HANDLE_T dst_handle, MEM_HANDLE_T src_handle)
{
   KHRN_FMEM_T             *fmem;
   uint8_t                 *p;
   uint32_t                i, j;
   bool                    store;
   KHRN_IMAGE_T            *src;
   KHRN_IMAGE_T            *dst;
   uint32_t                xTiles;
   uint32_t                yTiles;
   uint32_t                numTiles;
   uint32_t                numBytes;

   src = (KHRN_IMAGE_T *)mem_lock(src_handle, NULL);
   dst = (KHRN_IMAGE_T *)mem_lock(dst_handle, NULL);

   xTiles = (src->width  + 63) / 64;
   yTiles = (src->height + 63) / 64;

   vcos_assert(src->width == dst->width && src->height == dst->height);

   fmem = khrn_fmem_init(KHRN_INTERLOCK_USER_TEMP);
   if (!fmem)
      return false;

   if (!khrn_fmem_start_render(fmem))
   {
      khrn_fmem_discard(fmem);
      return false;
   }

   numTiles = xTiles * yTiles;
   numBytes = 11 + (numTiles * 11);

   p = khrn_fmem_cle(fmem, numBytes);
   if (p == NULL)
   {
      mem_unlock(dst_handle);
      mem_unlock(src_handle);
   }

   /* set rendering mode config */
   ADD_BYTE(p, KHRN_HW_INSTR_STATE_TILE_RENDERING_MODE);
   khrn_fmem_add_fix(fmem, &p, dst->mh_storage, 0);

   ADD_SHORT(p, src->width);
   ADD_SHORT(p, src->height);
   ADD_BYTE(p,
      (0 << 0) | /* disable ms mode */
      (0 << 1) | /* disable 64-bit color mode */
      (get_dest_format(dst->format) << 2) |
      (0 << 4) | /* No decimation */
      (get_type_flag(dst->format) << 6));

   ADD_BYTE(p,
      (0 << 0) | /* no vg mask */
      (0 << 1) | /* no coverage mode */
      (0 << 2) | /* don't care z-update dir */
      (0 << 3) | /* disable early Z */
      (0 << 4)); /* no double buffer */

   store = false;
   for (j = 0; j != yTiles; ++j)
   {
      for (i = 0; i != xTiles; ++i)
      {
         /* We write the store for the previous tile here as we're not storing all tiles.
          * We can then add the final store with EOF after the loop */
            if (store)
               ADD_BYTE(p, KHRN_HW_INSTR_STORE_SUBSAMPLE);

            ADD_BYTE(p, KHRN_HW_INSTR_LOAD_GENERAL);
            ADD_BYTE(p, (uint8_t)(
               (1 << 0) | /* load color */
               (get_type_flag(src->format) << 4)));

            ADD_BYTE(p, get_src_format(src->format));
            khrn_fmem_add_fix(fmem, &p, src->mh_storage, 0);

            ADD_BYTE(p, KHRN_HW_INSTR_STATE_TILE_COORDS);
            ADD_BYTE(p, (uint8_t)i);
            ADD_BYTE(p, (uint8_t)j);

            store = true;
         }
      }

   /* Add the final store with EOF */
   ADD_BYTE(p, KHRN_HW_INSTR_STORE_SUBSAMPLE_EOF);

   mem_unlock(dst_handle);
   mem_unlock(src_handle);

   khrn_issue_copy_buffer_job(fmem, dst_handle, src_handle, numBytes);

   return true;
}

void khrn_copy_buffer_render_state_flush(KHRN_COPY_BUFFER_RENDER_STATE_T *render_state)
{
   /* Nothing to do */
}
