/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_common.h"
#include "v3d_cl.h"
#include "libs/core/lfmt/lfmt.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/util/gfx_util/gfx_util_conv.h"
#include <stdio.h>
#include <stdint.h>

bool v3d_prim_mode_is_patch(v3d_prim_mode_t prim_mode)
{
   return prim_mode >= V3D_PRIM_MODE_PATCH1
      && prim_mode <= V3D_PRIM_MODE_PATCH32;
}

uint32_t v3d_prim_mode_num_verts(v3d_prim_mode_t prim_mode, bool tg_enabled)
{
   if(v3d_prim_mode_is_patch(prim_mode))
   {
      return 1 + (prim_mode - V3D_PRIM_MODE_PATCH1);
   }

   switch (prim_mode) {
   case V3D_PRIM_MODE_POINTS:
#if !V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_PRIM_MODE_POINTS_TF:
#endif
      return 1;
   case V3D_PRIM_MODE_LINES:
   case V3D_PRIM_MODE_LINE_LOOP:
   case V3D_PRIM_MODE_LINE_STRIP:
#if !V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_PRIM_MODE_LINES_TF:
   case V3D_PRIM_MODE_LINE_LOOP_TF:
   case V3D_PRIM_MODE_LINE_STRIP_TF:
#endif
      return 2;
   case V3D_PRIM_MODE_TRIS:
   case V3D_PRIM_MODE_TRI_STRIP:
   case V3D_PRIM_MODE_TRI_FAN:
#if !V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_PRIM_MODE_TRIS_TF:
   case V3D_PRIM_MODE_TRI_STRIP_TF:
   case V3D_PRIM_MODE_TRI_FAN_TF:
#endif
      return 3;
   /* When tess and geom aren't enabled we drop adjacency information from
    * with-adjacency prim lists */
   case V3D_PRIM_MODE_LINES_ADJ:
   case V3D_PRIM_MODE_LINE_STRIP_ADJ:
      return tg_enabled ? 4 : 2;
   case V3D_PRIM_MODE_TRIS_ADJ:
   case V3D_PRIM_MODE_TRI_STRIP_ADJ:
      return tg_enabled ? 6 : 3;
   default:
      unreachable();
      return 1;
   }
}

uint32_t v3d_tess_type_num_verts(v3d_cl_tess_type_t tess_type)
{
   switch(tess_type)
   {
      case V3D_CL_TESS_TYPE_TRIANGLE: return 3;
      case V3D_CL_TESS_TYPE_QUAD: return 3;
      case V3D_CL_TESS_TYPE_ISOLINES: return 2;
      default: unreachable();
   }
}

uint32_t v3d_geom_prim_type_num_verts(v3d_cl_geom_prim_type_t type)
{
   switch(type)
   {
      case V3D_CL_GEOM_PRIM_TYPE_POINTS: return 1;
      case V3D_CL_GEOM_PRIM_TYPE_LINE_STRIP: return 2;
      case V3D_CL_GEOM_PRIM_TYPE_TRIANGLE_STRIP: return 3;
      default: unreachable();
   }
}

void v3d_pixel_format_to_rt_format(
   V3D_RT_FORMAT_T *rt_format, v3d_pixel_format_t pixel_format)
{
   /* See http://confluence.broadcom.com/x/qwLKB */

#if V3D_VER_AT_LEAST(4,1,34,0)
   rt_format->clamp = V3D_RT_CLAMP_NONE;
#endif

   switch (pixel_format) {

   case V3D_PIXEL_FORMAT_A1_BGR5:
   case V3D_PIXEL_FORMAT_A1_BGR5_AM:
#if V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_PIXEL_FORMAT_RGB5_A1:
#endif
   case V3D_PIXEL_FORMAT_ABGR4:
   case V3D_PIXEL_FORMAT_BGR565:
   case V3D_PIXEL_FORMAT_RGBA8:
   case V3D_PIXEL_FORMAT_RGB8:
   case V3D_PIXEL_FORMAT_RG8:
   case V3D_PIXEL_FORMAT_R8:
#if !V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_PIXEL_FORMAT_RGBX8:
#endif
   case V3D_PIXEL_FORMAT_BSTC:
      rt_format->type = V3D_RT_TYPE_8;
      rt_format->bpp = V3D_RT_BPP_32;
      break;
   case V3D_PIXEL_FORMAT_RGBA8I:
   case V3D_PIXEL_FORMAT_RG8I:
   case V3D_PIXEL_FORMAT_R8I:
      rt_format->type = V3D_RT_TYPE_8I;
      rt_format->bpp = V3D_RT_BPP_32;
      break;
   case V3D_PIXEL_FORMAT_RGBA8UI:
   case V3D_PIXEL_FORMAT_RG8UI:
   case V3D_PIXEL_FORMAT_R8UI:
      rt_format->type = V3D_RT_TYPE_8UI;
      rt_format->bpp = V3D_RT_BPP_32;
      break;

   case V3D_PIXEL_FORMAT_SRGB8_ALPHA8:
   case V3D_PIXEL_FORMAT_SRGB8:
   case V3D_PIXEL_FORMAT_RGB10_A2:
#if !V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_PIXEL_FORMAT_SRGBX8:
#endif
      rt_format->type = V3D_RT_TYPE_16F;
      rt_format->bpp = V3D_RT_BPP_64;
#if V3D_VER_AT_LEAST(4,1,34,0)
      rt_format->clamp = V3D_RT_CLAMP_NORM;
#endif
      break;
   case V3D_PIXEL_FORMAT_R11F_G11F_B10F:
      rt_format->type = V3D_RT_TYPE_16F;
      rt_format->bpp = V3D_RT_BPP_64;
#if V3D_VER_AT_LEAST(4,1,34,0)
      rt_format->clamp = V3D_RT_CLAMP_POS;
#endif
      break;
   case V3D_PIXEL_FORMAT_RGBA16F:
      rt_format->type = V3D_RT_TYPE_16F;
      rt_format->bpp = V3D_RT_BPP_64;
      break;
   case V3D_PIXEL_FORMAT_RG16F:
   case V3D_PIXEL_FORMAT_R16F:
      rt_format->type = V3D_RT_TYPE_16F;
#if V3D_HAS_GFXH1207_FIX
      rt_format->bpp = V3D_RT_BPP_32;
#else
      /* GFXH-1207: Although these are 32bpp, claim they are 64, otherwise the
       * TLB will incorrectly discard alpha output from shaders.
       */
      rt_format->bpp = V3D_RT_BPP_64;
#endif
      break;
   case V3D_PIXEL_FORMAT_RGBA16I:   rt_format->type = V3D_RT_TYPE_16I;  rt_format->bpp = V3D_RT_BPP_64;  break;
   case V3D_PIXEL_FORMAT_RG16I:     rt_format->type = V3D_RT_TYPE_16I;  rt_format->bpp = V3D_RT_BPP_32;  break;
   case V3D_PIXEL_FORMAT_R16I:      rt_format->type = V3D_RT_TYPE_16I;  rt_format->bpp = V3D_RT_BPP_32;  break;
   case V3D_PIXEL_FORMAT_RGB10_A2UI:
   case V3D_PIXEL_FORMAT_RGBA16UI:  rt_format->type = V3D_RT_TYPE_16UI; rt_format->bpp = V3D_RT_BPP_64;  break;
   case V3D_PIXEL_FORMAT_RG16UI:    rt_format->type = V3D_RT_TYPE_16UI; rt_format->bpp = V3D_RT_BPP_32;  break;
   case V3D_PIXEL_FORMAT_R16UI:     rt_format->type = V3D_RT_TYPE_16UI; rt_format->bpp = V3D_RT_BPP_32;  break;

   case V3D_PIXEL_FORMAT_RGBA32F:   rt_format->type = V3D_RT_TYPE_32F;  rt_format->bpp = V3D_RT_BPP_128; break;
   case V3D_PIXEL_FORMAT_RG32F:     rt_format->type = V3D_RT_TYPE_32F;  rt_format->bpp = V3D_RT_BPP_64;  break;
   case V3D_PIXEL_FORMAT_R32F:      rt_format->type = V3D_RT_TYPE_32F;  rt_format->bpp = V3D_RT_BPP_32;  break;
   case V3D_PIXEL_FORMAT_RGBA32I:   rt_format->type = V3D_RT_TYPE_32I;  rt_format->bpp = V3D_RT_BPP_128; break;
   case V3D_PIXEL_FORMAT_RG32I:     rt_format->type = V3D_RT_TYPE_32I;  rt_format->bpp = V3D_RT_BPP_64;  break;
   case V3D_PIXEL_FORMAT_R32I:      rt_format->type = V3D_RT_TYPE_32I;  rt_format->bpp = V3D_RT_BPP_32;  break;
   case V3D_PIXEL_FORMAT_RGBA32UI:  rt_format->type = V3D_RT_TYPE_32UI; rt_format->bpp = V3D_RT_BPP_128; break;
   case V3D_PIXEL_FORMAT_RG32UI:    rt_format->type = V3D_RT_TYPE_32UI; rt_format->bpp = V3D_RT_BPP_64;  break;
   case V3D_PIXEL_FORMAT_R32UI:     rt_format->type = V3D_RT_TYPE_32UI; rt_format->bpp = V3D_RT_BPP_32;  break;

   default:
      unreachable();
   }
}

#if !V3D_VER_AT_LEAST(4,1,34,0)
v3d_pixel_format_t v3d_raw_mode_pixel_format(
   v3d_rt_type_t type, v3d_rt_bpp_t bpp)
{
   switch (type) {
   case V3D_RT_TYPE_8I:    assert(bpp == V3D_RT_BPP_32); return V3D_PIXEL_FORMAT_RGBA8I;
   case V3D_RT_TYPE_8UI:   assert(bpp == V3D_RT_BPP_32); return V3D_PIXEL_FORMAT_RGBA8UI;
   case V3D_RT_TYPE_8:     assert(bpp == V3D_RT_BPP_32); return V3D_PIXEL_FORMAT_RGBA8;
   case V3D_RT_TYPE_16I:
      switch (bpp) {
      case V3D_RT_BPP_32:  return V3D_PIXEL_FORMAT_RG16I;
      case V3D_RT_BPP_64:  return V3D_PIXEL_FORMAT_RGBA16I;
      default:             unreachable(); return V3D_PIXEL_FORMAT_INVALID;
      }
   case V3D_RT_TYPE_16UI:
      switch (bpp) {
      case V3D_RT_BPP_32:  return V3D_PIXEL_FORMAT_RG16UI;
      case V3D_RT_BPP_64:  return V3D_PIXEL_FORMAT_RGBA16UI;
      default:             unreachable(); return V3D_PIXEL_FORMAT_INVALID;
      }
   case V3D_RT_TYPE_16F:
      switch (bpp) {
      case V3D_RT_BPP_32:  return V3D_PIXEL_FORMAT_RG16F;
      case V3D_RT_BPP_64:  return V3D_PIXEL_FORMAT_RGBA16F;
      default:             unreachable(); return V3D_PIXEL_FORMAT_INVALID;
      }
   case V3D_RT_TYPE_32I:
      switch (bpp) {
      case V3D_RT_BPP_32:  return V3D_PIXEL_FORMAT_R32I;
      case V3D_RT_BPP_64:  return V3D_PIXEL_FORMAT_RG32I;
      case V3D_RT_BPP_128: return V3D_PIXEL_FORMAT_RGBA32I;
      default:             unreachable(); return V3D_PIXEL_FORMAT_INVALID;
      }
   case V3D_RT_TYPE_32UI:
      switch (bpp) {
      case V3D_RT_BPP_32:  return V3D_PIXEL_FORMAT_R32UI;
      case V3D_RT_BPP_64:  return V3D_PIXEL_FORMAT_RG32UI;
      case V3D_RT_BPP_128: return V3D_PIXEL_FORMAT_RGBA32UI;
      default:             unreachable(); return V3D_PIXEL_FORMAT_INVALID;
      }
   case V3D_RT_TYPE_32F:
      switch (bpp) {
      case V3D_RT_BPP_32:  return V3D_PIXEL_FORMAT_R32F;
      case V3D_RT_BPP_64:  return V3D_PIXEL_FORMAT_RG32F;
      case V3D_RT_BPP_128: return V3D_PIXEL_FORMAT_RGBA32F;
      default:             unreachable(); return V3D_PIXEL_FORMAT_INVALID;
      }
   default:
      unreachable(); return V3D_PIXEL_FORMAT_INVALID;
   }
}
#endif

void v3d_pack_clear_color(uint32_t packed[4], const uint32_t c[4],
   const V3D_RT_FORMAT_T *rt_format)
{
   uint32_t col[4] = { c[0], c[1], c[2], c[3] };
#if V3D_VER_AT_LEAST(4,2,13,0)
   for (int i=0; i<4; i++)
      col[i] = v3d_apply_rt_int_clamp(col[i], rt_format->type, rt_format->clamp);
#endif

   memset(packed, 0, 4 * sizeof(uint32_t));
   switch (rt_format->type)
   {
   case V3D_RT_TYPE_8I:
   case V3D_RT_TYPE_8UI:
      assert(rt_format->bpp == V3D_RT_BPP_32);
      packed[0] = gfx_pack_8888(
         col[0] & 0xff, col[1] & 0xff,
         col[2] & 0xff, col[3] & 0xff);
      break;
   case V3D_RT_TYPE_8:
      assert(rt_format->bpp == V3D_RT_BPP_32);
      packed[0] = gfx_pack_8888(
         gfx_float_to_unorm8(gfx_float_from_bits(col[0])),
         gfx_float_to_unorm8(gfx_float_from_bits(col[1])),
         gfx_float_to_unorm8(gfx_float_from_bits(col[2])),
         gfx_float_to_unorm8(gfx_float_from_bits(col[3])));
      break;
   case V3D_RT_TYPE_16I:
   case V3D_RT_TYPE_16UI:
      switch (rt_format->bpp)
      {
      case V3D_RT_BPP_64:
         packed[1] = gfx_pack_1616(
            col[2] & 0xffff, col[3] & 0xffff);
         /* Fall through */
      case V3D_RT_BPP_32:
         packed[0] = gfx_pack_1616(
            col[0] & 0xffff, col[1] & 0xffff);
         break;
      default:
         unreachable();
      }
      break;
   case V3D_RT_TYPE_16F:
      switch (rt_format->bpp)
      {
      case V3D_RT_BPP_64:
         packed[1] = gfx_pack_1616(
            gfx_floatbits_to_float16(col[2]),
            gfx_floatbits_to_float16(col[3]));
         /* Fall through */
      case V3D_RT_BPP_32:
         packed[0] = gfx_pack_1616(
            gfx_floatbits_to_float16(col[0]),
            gfx_floatbits_to_float16(col[1]));
         break;
      default:
         unreachable();
      }
      break;
   case V3D_RT_TYPE_32I:
   case V3D_RT_TYPE_32UI:
   case V3D_RT_TYPE_32F:
      switch (rt_format->bpp)
      {
      case V3D_RT_BPP_128:
         packed[3] = col[3];
         packed[2] = col[2];
         /* Fall through */
      case V3D_RT_BPP_64:
         packed[1] = col[1];
         /* Fall through */
      case V3D_RT_BPP_32:
         packed[0] = col[0];
         break;
      default:
         unreachable();
      }
      break;
   default:
      unreachable();
   }

#if V3D_VER_AT_LEAST(4,1,34,0)
   for (unsigned i = 0; i != v3d_rt_bpp_words(rt_format->bpp); ++i)
      packed[i] = v3d_apply_rt_clamp(packed[i], rt_format->type, rt_format->clamp);
#endif
}

void v3d_cl_rcfg_clear_colors(uint8_t **cl, uint32_t rt,
   const uint32_t col[4],
   const V3D_RT_FORMAT_T *rt_format
#if !V3D_VER_AT_LEAST(4,1,34,0)
   , uint32_t raster_padded_width_or_nonraster_height,
   uint32_t uif_height_in_ub
#endif
   )
{
   uint32_t packed[4];
   v3d_pack_clear_color(packed, col, rt_format);

   v3d_cl_tile_rendering_mode_cfg_clear_colors_part1(cl, rt,
      packed[0], packed[1] & gfx_mask(24));

   if ((rt_format->bpp == V3D_RT_BPP_64) || (rt_format->bpp == V3D_RT_BPP_128))
      v3d_cl_tile_rendering_mode_cfg_clear_colors_part2(cl, rt,
         packed[1] >> 24, packed[2], packed[3] & gfx_mask(16));

#if V3D_VER_AT_LEAST(4,1,34,0)
   if (rt_format->bpp == V3D_RT_BPP_128)
      v3d_cl_tile_rendering_mode_cfg_clear_colors_part3(cl, rt, packed[3] >> 16);
#else
   v3d_cl_tile_rendering_mode_cfg_clear_colors_part3(cl, rt,
      packed[3] >> 16,
      raster_padded_width_or_nonraster_height,
      uif_height_in_ub);
#endif
}

#if V3D_VER_AT_LEAST(4,1,34,0)
# if V3D_VER_AT_LEAST(4,2,13,0)
uint32_t v3d_apply_rt_int_clamp(uint32_t wr, v3d_rt_type_t rt_type, v3d_rt_clamp_t clamp)
{
   /* All non-int clamp modes are handled later */
   if (clamp != V3D_RT_CLAMP_INT)
      return wr;

   switch(rt_type)
   {
   case V3D_RT_TYPE_8I:   return gfx_sclamp(wr, INT8_MIN,  INT8_MAX);
   case V3D_RT_TYPE_8UI:  return gfx_uclamp(wr, 0,         UINT8_MAX);
   case V3D_RT_TYPE_16I:  return gfx_sclamp(wr, INT16_MIN, INT16_MAX);
   case V3D_RT_TYPE_16UI: return gfx_uclamp(wr, 0,         UINT16_MAX);

   case V3D_RT_TYPE_32I:
   case V3D_RT_TYPE_32UI:
      return wr;
   default: unreachable(); return 0;
   }
}
# endif

uint32_t v3d_apply_rt_clamp(uint32_t w, v3d_rt_type_t type, v3d_rt_clamp_t clamp)
{
   switch (clamp)
   {
   case V3D_RT_CLAMP_NONE:
      return w;
   case V3D_RT_CLAMP_NORM:
   case V3D_RT_CLAMP_POS:
   {
      assert(type == V3D_RT_TYPE_16F);
      uint32_t res = 0;
      for (unsigned i = 0; i != 2; ++i)
      {
         float f = gfx_float16_to_float(gfx_pick_16(w, i));
         switch (clamp)
         {
         case V3D_RT_CLAMP_NORM: f = gfx_fclamp(gfx_nan_to_inf(f), 0.0f, 1.0f); break;
         case V3D_RT_CLAMP_POS:  f = gfx_sign_bit_set(f) ? 0.0f : f; break; // Don't use fmaxf; want to preserve +ve NaNs
         default:                unreachable();
         }
         res |= gfx_float_to_float16(f) << (i * 16);
      }
      return res;
   }
#if V3D_VER_AT_LEAST(4,2,13,0)
   case V3D_RT_CLAMP_INT:
      return w;   /* This has been handled earlier */
#endif
   default:
      unreachable();
      return 0;
   }
}
#endif

float v3d_snap_depth(float depth, v3d_depth_type_t depth_type)
{
   uint32_t num_bits, unorm;
   float snapped;

   switch (depth_type)
   {
   case V3D_DEPTH_TYPE_32F:   return depth;
   case V3D_DEPTH_TYPE_24:    num_bits = 24; break;
   case V3D_DEPTH_TYPE_16:    num_bits = 16; break;
   default:                   unreachable();
   }

   unorm = gfx_float_to_unorm_depth(depth, num_bits);
   snapped = gfx_unorm_to_float_depth(unorm, num_bits);

   return snapped;
}

#if !V3D_VER_AT_LEAST(4,1,34,0)
const char *v3d_desc_output_format(
   v3d_ldst_buf_t buf, v3d_output_format_t output_format)
{
   switch (v3d_classify_ldst_buf(buf))
   {
   case V3D_LDST_BUF_CLASS_COLOR:
      return v3d_desc_pixel_format(output_format.pixel);
   case V3D_LDST_BUF_CLASS_DEPTH_STENCIL:
      return ((buf == V3D_LDST_BUF_STENCIL) &&
            !v3d_depth_format_has_stencil(output_format.depth)) ?
         "stencil8" : v3d_desc_depth_format(output_format.depth);
   default:
      unreachable();
      return NULL;
   }
}
#endif

void v3d_cl_truncate_source(struct v3d_cl_source *source, size_t size)
{
   if (size > source->parts[0].size)
   {
      size_t p1_size = size - source->parts[0].size;
      assert(p1_size <= source->parts[1].size);
      source->parts[1].size = p1_size;
   }
   else
   {
      source->parts[0].size = size;
      source->parts[1].addr = v3d_cl_source_part_end(&source->parts[0]);
      source->parts[1].size = 0;
   }
}

void v3d_cl_extend_source(struct v3d_cl_source *source, v3d_addr_t addr, size_t size)
{
   if (v3d_cl_source_size(source) == 0)
   {
      v3d_cl_source_single_part(source, addr, size);
      return;
   }
   if (size == 0)
      return;

   if (v3d_cl_source_end(source) == addr)
   {
      if (source->parts[1].size == 0)
      {
         assert(source->parts[1].addr == v3d_cl_source_part_end(&source->parts[0]));
         source->parts[0].size += size;
         source->parts[1].addr = v3d_cl_source_part_end(&source->parts[0]);
      }
      else
         source->parts[1].size += size;
   }
   else
   {
      assert(source->parts[1].size == 0);
      source->parts[1].addr = addr;
      source->parts[1].size = size;
   }
}

void v3d_cl_cat_sources(struct v3d_cl_source *a, const struct v3d_cl_source *b)
{
   for (uint32_t i = 0; i != 2; ++i)
      v3d_cl_extend_source(a, b->parts[i].addr, b->parts[i].size);
}

void v3d_cl_log_cat_bytes(
   struct log_cat *cat, log_level_t level, const char *line_prefix,
   const char *desc, const uint8_t *bytes, const struct v3d_cl_source *source)
{
   if (!log_cat_enabled(cat, level))
      return;

   assert(source->parts[0].size > 0);

   char buf[256];
   size_t offset;

   offset = 0;
   for (uint32_t i = 0; i != 2; ++i)
   {
      const struct v3d_cl_source_part *part = &source->parts[i];
      if (part->size > 0)
         offset = VCOS_SAFE_SPRINTF(buf, offset, "%s%" PRIuSIZE " byte%s @0x%08x",
            (i > 0) ? ", " : "", part->size, (part->size == 1) ? "" : "s", part->addr);
   }
   assert((offset > 0) && (offset < sizeof(buf)));
   log_cat_msg(cat, level, "%s%s [%s]", line_prefix, desc, buf);

   offset = 0;
   for (size_t i = 0; i != v3d_cl_source_size(source); ++i)
      offset = VCOS_SAFE_SPRINTF(buf, offset, "%s%02x",
         (i > 0) ? "," : "", bytes[i]);
   assert((offset > 0) && (offset < sizeof(buf)));
   log_cat_msg(cat, level, "%s%s", line_prefix, buf);
}

void v3d_cl_log_cat_instr(
   struct log_cat *cat, log_level_t level, const char *line_prefix,
   const uint8_t *packed_instr, const struct v3d_cl_source *source)
{
   if (!log_cat_enabled(cat, level))
      return;

   v3d_cl_opcode_t opcode = (v3d_cl_opcode_t)packed_instr[0];
   struct v3d_cl_source truncated_source = *source;
   v3d_cl_truncate_source(&truncated_source, v3d_cl_instr_size(opcode));
   v3d_cl_log_cat_bytes(cat, level, line_prefix,
      v3d_desc_cl_opcode(opcode), packed_instr, &truncated_source);

   char buf[256];
   size_t offset = VCOS_SAFE_SPRINTF(buf, 0, "%s   ", line_prefix);
   assert(offset < sizeof(buf));
   struct v3d_basic_log_cat_printer p;
   v3d_basic_log_cat_printer_init(&p, cat, level, buf);
   v3d_cl_print_instr(packed_instr, &p.base.base);
}

size_t v3d_cl_sprint_plist_fmt(char *buf, size_t buf_size, size_t offset,
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt)
{
   assert(!plist_fmt->xy || !plist_fmt->d3dpvsf);
   return vcos_safe_sprintf(buf, buf_size, offset, "n_verts=%u%s",
      plist_fmt->n_verts,
      plist_fmt->xy ? " xy" : plist_fmt->d3dpvsf ? " d3dpvsf" : "");
}

void v3d_cl_write_vary_flags(uint8_t **instr, const uint32_t *flags, v3d_cl_flag_set_func set, v3d_cl_flag_zero_func zero)
{
   v3d_flags_action_t lower_action = flags[0] ? V3D_FLAGS_ACTION_SET : V3D_FLAGS_ACTION_ZERO;
   bool higher_set = lower_action == V3D_FLAGS_ACTION_SET;

   for (uint32_t i = 0; i < V3D_MAX_VARY_FLAG_WORDS; i++)
   {
      uint32_t mask = gfx_mask(gfx_umin(V3D_VARY_FLAGS_PER_WORD,
         V3D_MAX_VARYING_COMPONENTS - (i * V3D_VARY_FLAGS_PER_WORD)));
      assert((flags[i] & ~mask) == 0);

      if (flags[i] != (higher_set ? mask : 0))
      {
         if (i != (V3D_MAX_VARY_FLAG_WORDS - 1))
            higher_set = flags[i + 1] != 0;
         set(instr, i, lower_action, higher_set ? V3D_FLAGS_ACTION_SET : V3D_FLAGS_ACTION_ZERO, flags[i]);
         lower_action = V3D_FLAGS_ACTION_KEEP;
      }
   }

   switch (lower_action)
   {
   case V3D_FLAGS_ACTION_ZERO:
      zero(instr);
      break;
   case V3D_FLAGS_ACTION_SET:
      set(instr, 0, V3D_FLAGS_ACTION_SET, V3D_FLAGS_ACTION_SET, gfx_mask(V3D_VARY_FLAGS_PER_WORD));
      break;
   case V3D_FLAGS_ACTION_KEEP:
      break;
   default:
      unreachable();
   }
}

void v3d_cl_viewport_offset_from_rect(uint8_t **cl,
   int x, int y, unsigned width, unsigned height)
{
   int off_x = ((2 * x) + (int)width) * 128;
   int off_y = ((2 * y) + (int)height) * 128;
#if V3D_VER_AT_LEAST(4,1,34,0)
   int coarse_off_x = (off_x - (int)(width * 128)) >> (6 + 8);
   int coarse_off_y = (off_y - (int)(height * 128)) >> (6 + 8);
   v3d_cl_viewport_offset(cl,
      off_x - (coarse_off_x << (6 + 8)), coarse_off_x,
      off_y - (coarse_off_y << (6 + 8)), coarse_off_y);
#else
   v3d_cl_viewport_offset(cl, off_x, off_y);
#endif
}
