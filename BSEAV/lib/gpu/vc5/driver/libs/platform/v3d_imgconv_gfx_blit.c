/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Image format conversion

FILE DESCRIPTION
Use the software (slow) path for image conversions
=============================================================================*/
#include "v3d_imgconv_internal.h"

LOG_DEFAULT_CAT("v3d_imgconv_gfx_blit")

/* Slow imgconv time in micro seconds */
#define SLOW_CONV_TIME 10 * 1000
#define MAX_SLOW_CONVERSION_MESSAGES 20

static void init_xform_seq(GFX_BUFFER_XFORM_SEQ_T *seq,
      const GFX_BUFFER_BLIT_TGT_T *dst_bt,
      const GFX_BUFFER_BLIT_TGT_T *src_bt)
{
  gfx_buffer_xform_seq_init(seq, &src_bt->desc);

   GFX_LFMT_T src_fmt_0 = gfx_lfmt_fmt(src_bt->desc.planes[0].lfmt);
   if (gfx_lfmt_has_depth(src_fmt_0) && gfx_lfmt_contains_float(src_fmt_0))
   {
      /* Assume it's the depth channel that has float type... */
      assert(src_bt->desc.num_planes == 1);
      gfx_buffer_xform_seq_add(seq, gfx_buffer_xform_clamp_float_depth, src_fmt_0);
   }

   gfx_buffer_xform_seq_construct_continue_desc(seq, &dst_bt->desc,
      GFX_BUFFER_XFORM_CONVS_REGULAR, NULL);
}

static bool claim_conversion(const struct v3d_imgconv_base_tgt *dst,
      const struct v3d_imgconv_base_tgt *src,
      unsigned int width, unsigned int height, unsigned int depth, const security_info_t *sec_info)
{
   return v3d_imgconv_valid_cpu_sec_info(sec_info);
}

static void convert_now(
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height, unsigned int depth)
{
   GFX_BUFFER_BLIT_TGT_T src_b, dst_b;
   GFX_BUFFER_XFORM_SEQ_T xform_seq;

   src_b.desc = src->desc;
   gfx_buffer_blit_tgt_set_pos(&src_b, src->x, src->y, src->z);
   src_b.p = src_data;

   dst_b.desc = dst->desc;
   gfx_buffer_blit_tgt_set_pos(&dst_b, dst->x, dst->y, dst->z);
   dst_b.p = dst_data;

   unsigned start_us = vcos_getmicrosecs();

   init_xform_seq(&xform_seq, &dst_b, &src_b);
   gfx_buffer_blit(&dst_b, &src_b, &xform_seq, width, height, depth);

   unsigned end_us = vcos_getmicrosecs();
   if (((end_us - start_us > SLOW_CONV_TIME) && log_warn_enabled()) ||
        log_trace_enabled())
   {
      static int slow_conversion_messages = 0;

      if (slow_conversion_messages < MAX_SLOW_CONVERSION_MESSAGES)
      {
         GFX_LFMT_SPRINT(src_lfmt_desc, src_b.desc.planes[0].lfmt);
         GFX_LFMT_SPRINT(dst_lfmt_desc, dst_b.desc.planes[0].lfmt);
         if (log_trace_enabled())
         {
            log_trace("Unaccelerated conversion %ux%u @ (%u,%u) %s -> %ux%u @ (%u,%u) %s took %"PRIu32"us",
                       width, height, src_b.x, src_b.y, src_lfmt_desc,
                       width, height, dst_b.x, dst_b.y, dst_lfmt_desc,
                       end_us - start_us);
         }
         else
         {
            log_warn("Slow unaccelerated conversion %ux%u @ (%u,%u) %s -> %ux%u @ (%u,%u) %s took %"PRIu32"us",
                       width, height, src_b.x, src_b.y, src_lfmt_desc,
                       width, height, dst_b.x, dst_b.y, dst_lfmt_desc,
                       end_us - start_us);

            slow_conversion_messages++;
            if (slow_conversion_messages == MAX_SLOW_CONVERSION_MESSAGES)
               log_warn("Suppressing further slow unaccelerated conversion messages");
         }
      }
   }
}

static v3d_imgconv_methods gfx_blit_path =
{
   .claim         = claim_conversion,
   .convert_async = NULL,
   .convert_sync  = convert_now,
   .convert_prep  = NULL
};

const v3d_imgconv_methods* get_gfx_blit_path(void)
{
   return &gfx_blit_path;
}
