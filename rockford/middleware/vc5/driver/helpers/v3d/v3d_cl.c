/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "helpers/v3d/v3d_common.h"
#include "helpers/v3d/v3d_cl.h"
#include "helpers/gfx/gfx_lfmt.h"
#include "helpers/gfx/gfx_lfmt_translate_v3d.h"
#include <stdio.h>
#include <stdint.h>

v3d_prim_type_t v3d_prim_type_from_mode(v3d_prim_mode_t prim_mode)
{
   switch (prim_mode) {
   case V3D_PRIM_MODE_POINTS:
   case V3D_PRIM_MODE_POINTS_TF:
      return V3D_PRIM_TYPE_POINT;
   case V3D_PRIM_MODE_LINES:
   case V3D_PRIM_MODE_LINES_TF:
   case V3D_PRIM_MODE_LINE_LOOP:
   case V3D_PRIM_MODE_LINE_STRIP:
      return V3D_PRIM_TYPE_LINE;
   case V3D_PRIM_MODE_TRIS:
   case V3D_PRIM_MODE_TRIS_TF:
   case V3D_PRIM_MODE_TRI_STRIP:
   case V3D_PRIM_MODE_TRI_FAN:
      return V3D_PRIM_TYPE_TRI;
   default:
      /* TODO more to come on vc5... */
      not_impl();
      return V3D_PRIM_TYPE_POINT;
   }
}

uint32_t v3d_prim_type_num_verts(v3d_prim_type_t prim_type)
{
   switch (prim_type) {
   case V3D_PRIM_TYPE_POINT:  return 1;
   case V3D_PRIM_TYPE_LINE:   return 2;
   case V3D_PRIM_TYPE_TRI:    return 3;
   default:                   not_impl(); return 0;
   }
}

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

// http://confluence.broadcom.com/display/MobileMultimedia/GFX+VC5+TLB+Design+Specification
//
void v3d_pixel_format_internal_type_and_bpp(
   v3d_rt_type_t *type, v3d_rt_bpp_t *bpp,
   v3d_pixel_format_t pixel_format)
{
   switch (pixel_format) {

   case V3D_PIXEL_FORMAT_A1_BGR5:
   case V3D_PIXEL_FORMAT_A1_BGR5_AM:
   case V3D_PIXEL_FORMAT_ABGR4:
   case V3D_PIXEL_FORMAT_BGR565:
   case V3D_PIXEL_FORMAT_RGBA8:
   case V3D_PIXEL_FORMAT_RGB8:
   case V3D_PIXEL_FORMAT_RG8:
   case V3D_PIXEL_FORMAT_R8:
   case V3D_PIXEL_FORMAT_RGBX8:
   case V3D_PIXEL_FORMAT_BSTC:
      *type = V3D_RT_TYPE_8;
      *bpp = V3D_RT_BPP_32;
      break;
   case V3D_PIXEL_FORMAT_RGBA8I:
   case V3D_PIXEL_FORMAT_RG8I:
   case V3D_PIXEL_FORMAT_R8I:
      *type = V3D_RT_TYPE_8I;
      *bpp = V3D_RT_BPP_32;
      break;
   case V3D_PIXEL_FORMAT_RGBA8UI:
   case V3D_PIXEL_FORMAT_RG8UI:
   case V3D_PIXEL_FORMAT_R8UI:
      *type = V3D_RT_TYPE_8UI;
      *bpp = V3D_RT_BPP_32;
      break;

   case V3D_PIXEL_FORMAT_SRGB8_ALPHA8:
   case V3D_PIXEL_FORMAT_SRGB8:
   case V3D_PIXEL_FORMAT_RGB10_A2:
   case V3D_PIXEL_FORMAT_R11F_G11F_B10F:
   case V3D_PIXEL_FORMAT_RGBA16F:
   case V3D_PIXEL_FORMAT_SRGBX8:
      *type = V3D_RT_TYPE_16F;
      *bpp = V3D_RT_BPP_64;
      break;
   case V3D_PIXEL_FORMAT_RG16F:
   case V3D_PIXEL_FORMAT_R16F:
      /* GFXH-1207: Although these are 32bpp, claim they are 64, otherwise the
       * TLB will incorrectly discard alpha output from shaders.
       */
      *type = V3D_RT_TYPE_16F;
      *bpp = V3D_RT_BPP_64;
      break;
   case V3D_PIXEL_FORMAT_RGBA16I:   *type = V3D_RT_TYPE_16I;   *bpp = V3D_RT_BPP_64;   break;
   case V3D_PIXEL_FORMAT_RG16I:     *type = V3D_RT_TYPE_16I;   *bpp = V3D_RT_BPP_32;   break;
   case V3D_PIXEL_FORMAT_R16I:      *type = V3D_RT_TYPE_16I;   *bpp = V3D_RT_BPP_32;   break;
   case V3D_PIXEL_FORMAT_RGB10_A2UI:
   case V3D_PIXEL_FORMAT_RGBA16UI:  *type = V3D_RT_TYPE_16UI;  *bpp = V3D_RT_BPP_64;   break;
   case V3D_PIXEL_FORMAT_RG16UI:    *type = V3D_RT_TYPE_16UI;  *bpp = V3D_RT_BPP_32;   break;
   case V3D_PIXEL_FORMAT_R16UI:     *type = V3D_RT_TYPE_16UI;  *bpp = V3D_RT_BPP_32;   break;

   case V3D_PIXEL_FORMAT_RGBA32F:   *type = V3D_RT_TYPE_32F;   *bpp = V3D_RT_BPP_128;  break;
   case V3D_PIXEL_FORMAT_RG32F:     *type = V3D_RT_TYPE_32F;   *bpp = V3D_RT_BPP_64;   break;
   case V3D_PIXEL_FORMAT_R32F:      *type = V3D_RT_TYPE_32F;   *bpp = V3D_RT_BPP_32;   break;
   case V3D_PIXEL_FORMAT_RGBA32I:   *type = V3D_RT_TYPE_32I;   *bpp = V3D_RT_BPP_128;  break;
   case V3D_PIXEL_FORMAT_RG32I:     *type = V3D_RT_TYPE_32I;   *bpp = V3D_RT_BPP_64;   break;
   case V3D_PIXEL_FORMAT_R32I:      *type = V3D_RT_TYPE_32I;   *bpp = V3D_RT_BPP_32;   break;
   case V3D_PIXEL_FORMAT_RGBA32UI:  *type = V3D_RT_TYPE_32UI;  *bpp = V3D_RT_BPP_128;  break;
   case V3D_PIXEL_FORMAT_RG32UI:    *type = V3D_RT_TYPE_32UI;  *bpp = V3D_RT_BPP_64;   break;
   case V3D_PIXEL_FORMAT_R32UI:     *type = V3D_RT_TYPE_32UI;  *bpp = V3D_RT_BPP_32;   break;

   default:
      unreachable();
   }
}

void v3d_pack_clear_color(uint32_t packed[4], const uint32_t col[4],
                          v3d_rt_type_t type, v3d_rt_bpp_t bpp)
{
   memset(packed, 0, 4 * sizeof(uint32_t));

   switch (type) {
   case V3D_RT_TYPE_8I:
   case V3D_RT_TYPE_8UI:
      assert(bpp == V3D_RT_BPP_32);
      packed[0] = gfx_pack_8888(
         col[0] & 0xff, col[1] & 0xff,
         col[2] & 0xff, col[3] & 0xff);
      break;
   case V3D_RT_TYPE_8:
      assert(bpp == V3D_RT_BPP_32);
      {
         float f[4];
         int i;
         for (i=0; i<4; i++) f[i] = gfx_float_from_bits(col[i]);

         packed[0] = gfx_float4_to_unorm8888(f);
      }
      break;
   case V3D_RT_TYPE_16I:
   case V3D_RT_TYPE_16UI:
      switch (bpp) {
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
      switch (bpp) {
      case V3D_RT_BPP_64:
         packed[1] = gfx_pack_1616(
            gfx_bits_to_float16(col[2]),
            gfx_bits_to_float16(col[3]));
         /* Fall through */
      case V3D_RT_BPP_32:
         packed[0] = gfx_pack_1616(
            gfx_bits_to_float16(col[0]),
            gfx_bits_to_float16(col[1]));
         break;
      default:
         unreachable();
      }
      break;
   case V3D_RT_TYPE_32I:
   case V3D_RT_TYPE_32UI:
   case V3D_RT_TYPE_32F:
      switch (bpp) {
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
}

void v3d_cl_rcfg_clear_colors(uint8_t **cl, uint32_t rt,
   const uint32_t col[4],
   v3d_rt_type_t type, v3d_rt_bpp_t bpp,
   uint32_t pad,
   uint32_t clear3_raster_padded_width_or_nonraster_height,
   uint32_t clear3_uif_height_in_ub)
{
   uint32_t packed[4];

   v3d_pack_clear_color(packed, col, type, bpp);

   v3d_cl_tile_rendering_mode_cfg_clear_colors_part1(cl, rt,
      packed[0], packed[1] & gfx_mask(24));

   if ((bpp == V3D_RT_BPP_64) || (bpp == V3D_RT_BPP_128)) {
      v3d_cl_tile_rendering_mode_cfg_clear_colors_part2(cl, rt,
         packed[1] >> 24, packed[2], packed[3] & gfx_mask(16));
   }

   if ((bpp == V3D_RT_BPP_128) || (pad == 15)) {
      v3d_cl_tile_rendering_mode_cfg_clear_colors_part3(cl, rt,
         packed[3] >> 16,
         clear3_raster_padded_width_or_nonraster_height,
         clear3_uif_height_in_ub);
   }
}

v3d_depth_type_t v3d_depth_format_internal_type(v3d_depth_format_t depth_format)
{
   switch (depth_format) {
   case V3D_DEPTH_FORMAT_32F:
      return V3D_DEPTH_TYPE_32F;
   case V3D_DEPTH_FORMAT_24:
   case V3D_DEPTH_FORMAT_24_STENCIL8:
      return V3D_DEPTH_TYPE_24;
   case V3D_DEPTH_FORMAT_16:
      return V3D_DEPTH_TYPE_16;
   default:
      unreachable();
      return V3D_DEPTH_TYPE_INVALID;
   }
}

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

void v3d_cl_logc_bytes(
   VCOS_LOG_CAT_T *log_cat, VCOS_LOG_LEVEL_T level, const char *line_prefix,
   const char *desc, const uint8_t *bytes, const struct v3d_cl_source *source)
{
   if (!vcos_is_log_enabled(log_cat, level))
      return;

   assert(source->parts[0].size > 0);

   char buf[256];
   size_t offset;

   offset = 0;
   for (uint32_t i = 0; i != 2; ++i)
   {
      const struct v3d_cl_source_part *part = &source->parts[i];
      if (part->size > 0)
         offset = VCOS_SAFE_SPRINTF(buf, offset, "%s%zu byte%s @0x%08x",
            (i > 0) ? ", " : "", part->size, (part->size == 1) ? "" : "s", part->addr);
   }
   assert((offset > 0) && (offset < sizeof(buf)));
   _VCOS_LOG_X(log_cat, level, "%s%s [%s]", line_prefix, desc, buf);

   offset = 0;
   for (size_t i = 0; i != v3d_cl_source_size(source); ++i)
      offset = VCOS_SAFE_SPRINTF(buf, offset, "%s%02x",
         (i > 0) ? "," : "", bytes[i]);
   assert((offset > 0) && (offset < sizeof(buf)));
   _VCOS_LOG_X(log_cat, level, "%s%s", line_prefix, buf);
}

void v3d_cl_logc_instr(
   VCOS_LOG_CAT_T *log_cat, VCOS_LOG_LEVEL_T level, const char *line_prefix,
   const uint8_t *packed_instr, const struct v3d_cl_source *source)
{
   if (!vcos_is_log_enabled(log_cat, level))
   {
      return;
   }

   v3d_cl_opcode_t opcode = (v3d_cl_opcode_t)packed_instr[0];
   struct v3d_cl_source truncated_source = *source;
   v3d_cl_truncate_source(&truncated_source, v3d_cl_instr_size(opcode));
   v3d_cl_logc_bytes(log_cat, level, line_prefix,
      v3d_desc_cl_opcode(opcode), packed_instr, &truncated_source);

   char buf[256];
   size_t offset = VCOS_SAFE_SPRINTF(buf, 0, "%s   ", line_prefix);
   assert(offset < sizeof(buf));
   vcos_unused_in_release(offset);
   struct v3d_basic_logc_printer p;
   v3d_basic_logc_printer_init(&p, log_cat, level, buf);
   v3d_cl_print_instr(packed_instr, &p.base.base);
}

size_t v3d_cl_sprint_plist_fmt(char *buf, size_t buf_size, size_t offset,
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt)
{
   assert(!plist_fmt->xy || !plist_fmt->d3dpvsf);
   return vcos_safe_sprintf(buf, buf_size, offset, "prim_type=%s%s",
      v3d_desc_prim_type(plist_fmt->prim_type),
      plist_fmt->xy ? " xy" : plist_fmt->d3dpvsf ? " d3dpvsf" : "");
}
