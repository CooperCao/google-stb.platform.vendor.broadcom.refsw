/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_imgconv_internal.h"

LOG_DEFAULT_CAT("v3d_imgconv_gfx_blit")

/* Slow imgconv time in micro seconds */
#define SLOW_CONV_TIME 10 * 1000
#define MAX_SLOW_CONVERSION_MESSAGES 20

static void init_xform_seq(GFX_BUFFER_XFORM_SEQ_T *seq,
      const GFX_BUFFER_BLIT_TGT_FUNC_T *dst_bt,
      const GFX_BUFFER_BLIT_TGT_FUNC_T *src_bt,
      v3d_imgconv_conversion_op dst_conversion)
{
   gfx_buffer_xform_seq_init(seq, &src_bt->desc);

   if (dst_conversion == V3D_IMGCONV_CONVERSION_CLAMP_DEPTH && gfx_buffer_any_float_depth(&src_bt->desc))
   {
      assert(src_bt->desc.num_planes == 1);
      gfx_buffer_xform_seq_add(seq, gfx_buffer_xform_clamp_float_depth, gfx_lfmt_fmt(src_bt->desc.planes[0].lfmt));
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

static inline void masked_32bit_write(size_t dst_offset, const void *src, size_t size, void *p, uint32_t mask)
{
   assert(size == 4);
   uint32_t *dst = (uint32_t *)((char *)p + dst_offset);
   *dst = (*((uint32_t *)src) & ~mask) | (*dst & mask);
}

static void masked_x8d24_write(size_t dst_offset, const void *src, size_t size, void *p)
{
   masked_32bit_write(dst_offset, src, size, p, 0xff);
}

static void masked_s8x24_write(size_t dst_offset, const void *src, size_t size, void *p)
{
   masked_32bit_write(dst_offset, src, size, p, 0xffffff00);
}

static void convert_now(
   const struct v3d_imgconv_base_tgt *dst, void *dst_data,
   const struct v3d_imgconv_base_tgt *src, void *src_data,
   unsigned int width, unsigned int height, unsigned int depth)
{
   GFX_BUFFER_BLIT_TGT_FUNC_T src_b, dst_b;
   GFX_BUFFER_XFORM_SEQ_T xform_seq;

   src_b.desc = src->desc;
   src_b.x = src->x;
   src_b.y = src->y;
   src_b.z = src->z;
   src_b.p = src_data;
   src_b.read = NULL;
   src_b.write = NULL;

   dst_b.desc = dst->desc;
   dst_b.x = dst->x;
   dst_b.y = dst->y;
   dst_b.z = dst->z;
   dst_b.p = dst_data;
   dst_b.read = NULL;
   dst_b.write = NULL;

   /* Handle destination masking for Vulkan depth/stencil aspect copies for
    * S8D24 destinations. It is allowed for the conversions to be specified
    * on depth only or stencil only formats, which requires no special
    * treatment.
    *
    * Note: the Vulkan driver does not advertise support for D32S8 at this
    *       time so that case is not implemented.
    */
   if (dst->conversion == V3D_IMGCONV_CONVERSION_DEPTH_ONLY)
   {
      if (gfx_lfmt_fmt(dst->desc.planes[0].lfmt) == GFX_LFMT_S8D24_UINT_UNORM)
         dst_b.write = masked_x8d24_write;
      else
         assert(gfx_lfmt_has_depth(dst->desc.planes[0].lfmt) && !gfx_lfmt_has_stencil(dst->desc.planes[0].lfmt));
   }

   if (dst->conversion == V3D_IMGCONV_CONVERSION_STENCIL_ONLY)
   {
      if (gfx_lfmt_fmt(dst->desc.planes[0].lfmt) == GFX_LFMT_S8D24_UINT_UNORM)
         dst_b.write = masked_s8x24_write;
      else
         assert(gfx_lfmt_has_stencil(dst->desc.planes[0].lfmt) && !gfx_lfmt_has_depth(dst->desc.planes[0].lfmt));
   }

   unsigned start_us = vcos_getmicrosecs();

   init_xform_seq(&xform_seq, &dst_b, &src_b, dst->conversion);
   gfx_buffer_blit_func(&dst_b, &src_b, &xform_seq, width, height, depth);

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
