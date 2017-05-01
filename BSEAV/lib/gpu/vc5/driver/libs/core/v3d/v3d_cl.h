/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_CL_H
#define V3D_CL_H

#include "v3d_common.h"
#include "v3d_gen.h"
#include "v3d_util.h"
#include "libs/util/gfx_util/gfx_util.h"
#include <string.h>
#include <stdio.h>

VCOS_EXTERN_C_BEGIN

extern bool v3d_prim_mode_is_patch(v3d_prim_mode_t prim_mode);
extern uint32_t v3d_prim_mode_num_verts(v3d_prim_mode_t prim_mode, bool tg_enabled);
extern uint32_t v3d_tess_type_num_verts(v3d_cl_tess_type_t tess_type);
extern uint32_t v3d_geom_prim_type_num_verts(v3d_cl_geom_prim_type_t type);

static inline uint32_t v3d_wireframe_mode_num_verts(v3d_wireframe_mode_t mode)
{
   switch (mode)
   {
   case V3D_WIREFRAME_MODE_LINES:   return 2;
   case V3D_WIREFRAME_MODE_POINTS:  return 1;
   default:                         unreachable(); return 0;
   }
}

static inline uint32_t v3d_index_type_bytes(v3d_index_type_t index_type)
{
   switch (index_type)
   {
   case V3D_INDEX_TYPE_8BIT:  return 1;
   case V3D_INDEX_TYPE_16BIT: return 2;
   case V3D_INDEX_TYPE_32BIT: return 4;
   default:                   unreachable(); return 0;
   }
}

static inline uint32_t v3d_rt_bpp_words(v3d_rt_bpp_t bpp)
{
   switch (bpp)
   {
   case V3D_RT_BPP_32:  return 1;
   case V3D_RT_BPP_64:  return 2;
   case V3D_RT_BPP_128: return 4;
   default:             unreachable(); return 0;
   }
}

static inline bool v3d_rt_formats_equal(
   const V3D_RT_FORMAT_T *a, const V3D_RT_FORMAT_T *b)
{
   return (a->bpp == b->bpp) && (a->type == b->type)
#if V3D_HAS_RT_CLAMP
      && (a->clamp == b->clamp)
#endif
      ;
}

extern void v3d_pixel_format_to_rt_format(
   V3D_RT_FORMAT_T *rt_format, v3d_pixel_format_t pixel_format);

static inline v3d_rt_type_t v3d_pixel_format_to_rt_type(v3d_pixel_format_t pixel_format)
{
   V3D_RT_FORMAT_T rt_format;
   v3d_pixel_format_to_rt_format(&rt_format, pixel_format);
   return rt_format.type;
}

static inline bool v3d_pixel_format_and_rt_format_compatible(
   v3d_pixel_format_t pixel_format, const V3D_RT_FORMAT_T *rt_format)
{
   /* See http://confluence.broadcom.com/x/qwLKB */
   V3D_RT_FORMAT_T from_pf;
   v3d_pixel_format_to_rt_format(&from_pf, pixel_format);
   return (rt_format->type == from_pf.type) && (rt_format->bpp >= from_pf.bpp); // clamp doesn't really matter
}

static inline bool v3d_rt_type_supports_4x_decimate(v3d_rt_type_t type)
{
   switch (type)
   {
   case V3D_RT_TYPE_8:
   case V3D_RT_TYPE_16F:
      return true;
   case V3D_RT_TYPE_8I:
   case V3D_RT_TYPE_8UI:
   case V3D_RT_TYPE_16I:
   case V3D_RT_TYPE_16UI:
   case V3D_RT_TYPE_32I:
   case V3D_RT_TYPE_32UI:
   case V3D_RT_TYPE_32F:
      return false;
   default:
      unreachable();
      return false;
   }
}

static inline bool v3d_pixel_format_supports_4x_decimate(v3d_pixel_format_t pixel_format)
{
   return v3d_rt_type_supports_4x_decimate(v3d_pixel_format_to_rt_type(pixel_format));
}

#if !V3D_VER_AT_LEAST(4,0,2,0)
extern v3d_pixel_format_t v3d_raw_mode_pixel_format(v3d_rt_type_t type, v3d_rt_bpp_t bpp);

static inline v3d_pixel_format_t v3d_pixel_format_raw_mode(v3d_pixel_format_t pixel_format)
{
   V3D_RT_FORMAT_T rt_format;
   v3d_pixel_format_to_rt_format(&rt_format, pixel_format);
   return v3d_raw_mode_pixel_format(rt_format.type, rt_format.bpp);
}
#endif

#if !V3D_HAS_TLB_SWIZZLE
static inline bool v3d_pixel_format_equiv_for_store(
   v3d_pixel_format_t a, v3d_pixel_format_t b)
{
   return (a == b) ||
      /* RGBX8 and RGBA8 literally handled identically by HW when storing... */
      ((a == V3D_PIXEL_FORMAT_SRGBX8) && (b == V3D_PIXEL_FORMAT_SRGB8_ALPHA8)) ||
      ((a == V3D_PIXEL_FORMAT_SRGB8_ALPHA8) && (b == V3D_PIXEL_FORMAT_SRGBX8)) ||
      ((a == V3D_PIXEL_FORMAT_RGBX8) && (b == V3D_PIXEL_FORMAT_RGBA8)) ||
      ((a == V3D_PIXEL_FORMAT_RGBA8) && (b == V3D_PIXEL_FORMAT_RGBX8));
}
#endif

static inline bool v3d_memory_and_pixel_formats_compatible(
   v3d_memory_format_t memory_format, v3d_pixel_format_t pixel_format)
{
   switch (pixel_format)
   {
   case V3D_PIXEL_FORMAT_SRGB8:
   case V3D_PIXEL_FORMAT_RGB8:
      /* 24-bit formats only compatible with raster */
      return memory_format == V3D_MEMORY_FORMAT_RASTER;
#if V3D_VER_AT_LEAST(4,0,2,0)
   case V3D_PIXEL_FORMAT_D32F:
   case V3D_PIXEL_FORMAT_D24:
   case V3D_PIXEL_FORMAT_D16:
   case V3D_PIXEL_FORMAT_D24S8:
   case V3D_PIXEL_FORMAT_S8:
      /* Depth/stencil cannot be stored as raster */
      return memory_format != V3D_MEMORY_FORMAT_RASTER;
#endif
   default:
      /* No restrictions on other pixel formats */
      return true;
   }
}

void v3d_pack_clear_color(uint32_t packed[4], const uint32_t col[4],
   const V3D_RT_FORMAT_T *rt_format);

static inline uint32_t v3d_cl_rcfg_clear_colors_size(v3d_rt_bpp_t bpp)
{
   switch (bpp)
   {
#if V3D_VER_AT_LEAST(4,0,2,0)
   case V3D_RT_BPP_32:  return V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;
   case V3D_RT_BPP_64:  return 2 * V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;
#else
   /* Always have clear colors 3 */
   case V3D_RT_BPP_32:  return 2 * V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;
   case V3D_RT_BPP_64:  return 3 * V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;
#endif
   case V3D_RT_BPP_128: return 3 * V3D_CL_TILE_RENDERING_MODE_CFG_SIZE;
   default:             unreachable(); return 0;
   }
}

void v3d_cl_rcfg_clear_colors(uint8_t **cl, uint32_t rt,
   const uint32_t col[4],
   const V3D_RT_FORMAT_T *rt_format
#if !V3D_VER_AT_LEAST(4,0,2,0)
   , uint32_t raster_padded_width_or_nonraster_height,
   uint32_t uif_height_in_ub
#endif
   );

#if V3D_HAS_RT_CLAMP
extern uint32_t v3d_apply_rt_clamp(uint32_t w, v3d_rt_type_t type, v3d_rt_clamp_t clamp);
#endif
extern float v3d_snap_depth(float depth, v3d_depth_type_t depth_type);

static inline bool v3d_dither_rgb(v3d_dither_t dither)
{
   switch (dither) {
   case V3D_DITHER_OFF:
   case V3D_DITHER_A:
      return false;
   case V3D_DITHER_RGB:
   case V3D_DITHER_RGBA:
      return true;
   default:
      unreachable();
      return false;
   }
}

static inline bool v3d_dither_a(v3d_dither_t dither)
{
   switch (dither) {
   case V3D_DITHER_OFF:
   case V3D_DITHER_RGB:
      return false;
   case V3D_DITHER_A:
   case V3D_DITHER_RGBA:
      return true;
   default:
      unreachable();
      return false;
   }
}

typedef enum {
   V3D_LDST_BUF_CLASS_NONE,
   V3D_LDST_BUF_CLASS_COLOR,
   V3D_LDST_BUF_CLASS_DEPTH_STENCIL,
   V3D_LDST_BUF_CLASS_INVALID
} v3d_ldst_buf_class_t;

static inline v3d_ldst_buf_class_t v3d_classify_ldst_buf(v3d_ldst_buf_t buf)
{
   if (buf <= V3D_LDST_BUF_COLOR7) {
      return V3D_LDST_BUF_CLASS_COLOR;
   }
   switch (buf) {
   case V3D_LDST_BUF_NONE:
      return V3D_LDST_BUF_CLASS_NONE;
   case V3D_LDST_BUF_DEPTH:
   case V3D_LDST_BUF_STENCIL:
   case V3D_LDST_BUF_PACKED_DEPTH_STENCIL:
      return V3D_LDST_BUF_CLASS_DEPTH_STENCIL;
   default:
      unreachable();
      return V3D_LDST_BUF_CLASS_INVALID;
   }
}

static inline uint32_t v3d_ldst_buf_rt(v3d_ldst_buf_t buf)
{
   assert(v3d_classify_ldst_buf(buf) == V3D_LDST_BUF_CLASS_COLOR);
   return buf - V3D_LDST_BUF_COLOR0;
}

static inline v3d_ldst_buf_t v3d_ldst_buf_color(uint32_t i)
{
   v3d_ldst_buf_t buf = (v3d_ldst_buf_t)(V3D_LDST_BUF_COLOR0 + i);
   assert(v3d_classify_ldst_buf(buf) == V3D_LDST_BUF_CLASS_COLOR);
   return buf;
}

static inline bool v3d_ldst_do_depth(v3d_ldst_buf_t buf)
{
   switch (buf) {
   case V3D_LDST_BUF_DEPTH:
   case V3D_LDST_BUF_PACKED_DEPTH_STENCIL:
      return true;
   case V3D_LDST_BUF_STENCIL:
      return false;
   default:
      unreachable();
      return false;
   }
}

static inline bool v3d_ldst_do_stencil(v3d_ldst_buf_t buf)
{
   switch (buf) {
   case V3D_LDST_BUF_DEPTH:
      return false;
   case V3D_LDST_BUF_STENCIL:
   case V3D_LDST_BUF_PACKED_DEPTH_STENCIL:
      return true;
   default:
      unreachable();
      return false;
   }
}

static inline v3d_ldst_buf_t v3d_ldst_buf_ds(bool do_depth, bool do_stencil)
{
   if (do_depth)
      return do_stencil ? V3D_LDST_BUF_PACKED_DEPTH_STENCIL : V3D_LDST_BUF_DEPTH;
   assert(do_stencil);
   return V3D_LDST_BUF_STENCIL;
}

static inline bool v3d_memory_format_is_uif(v3d_memory_format_t memory_format)
{
   return (memory_format == V3D_MEMORY_FORMAT_UIF_NO_XOR) ||
          (memory_format == V3D_MEMORY_FORMAT_UIF_XOR);
}

#if !V3D_VER_AT_LEAST(4,0,2,0)
static inline v3d_memory_format_t v3d_memory_format_from_ldst(
   v3d_ldst_memory_format_t ldst_memory_format)
{
   switch (ldst_memory_format) {
   case V3D_LDST_MEMORY_FORMAT_UIF_NO_XOR:   return V3D_MEMORY_FORMAT_UIF_NO_XOR;
   case V3D_LDST_MEMORY_FORMAT_UIF_XOR:      return V3D_MEMORY_FORMAT_UIF_XOR;
   default:                                  unreachable(); return V3D_MEMORY_FORMAT_INVALID;
   }
}

static inline v3d_ldst_memory_format_t v3d_memory_format_to_ldst(
   v3d_memory_format_t memory_format)
{
   switch (memory_format) {
   case V3D_MEMORY_FORMAT_UIF_NO_XOR:  return V3D_LDST_MEMORY_FORMAT_UIF_NO_XOR;
   case V3D_MEMORY_FORMAT_UIF_XOR:     return V3D_LDST_MEMORY_FORMAT_UIF_XOR;
   default:                            unreachable(); return V3D_LDST_MEMORY_FORMAT_INVALID;
   }
}

static inline bool v3d_depth_format_has_stencil(v3d_depth_format_t depth_format)
{
   switch (depth_format)
   {
   case V3D_DEPTH_FORMAT_32F:
   case V3D_DEPTH_FORMAT_24:
   case V3D_DEPTH_FORMAT_16:
      return false;
   case V3D_DEPTH_FORMAT_24_STENCIL8:
      return true;
   default:
      unreachable();
      return false;
   }
}

typedef union
{
   v3d_pixel_format_t pixel; /* V3D_LDST_BUF_CLASS_COLOR */
   v3d_depth_format_t depth; /* V3D_LDST_BUF_CLASS_DEPTH_STENCIL */
} v3d_output_format_t;

extern const char *v3d_desc_output_format(
   v3d_ldst_buf_t buf, v3d_output_format_t output_format);
#endif

struct v3d_tlb_ldst_params
{
   v3d_addr_t addr;
   v3d_memory_format_t memory_format;
#if V3D_VER_AT_LEAST(4,0,2,0)
   v3d_pixel_format_t pixel_format;
# if V3D_HAS_TLB_SWIZZLE
   bool load_alpha_to_one;
   bool chan_reverse;
   bool rb_swap;
# endif
#else
   v3d_output_format_t output_format;
#endif
   v3d_decimate_t decimate;
   v3d_dither_t dither; /* Only used for stores */
   uint32_t stride; /* UIF: height in UIF-blocks. Raster: stride in V3D_VER_AT_LEAST(4,0,2,0) ? bytes : pixels. */
   bool flipy;
   uint32_t flipy_height_px; /* Used only for y-flip */
};

#define V3D_DECIMATE_MAX_X_SCALE 2
#define V3D_DECIMATE_MAX_Y_SCALE 2

#if !V3D_VER_AT_LEAST(4,0,2,0)
static inline v3d_prim_mode_t v3d_prim_mode_remove_tf(bool *tf, v3d_prim_mode_t mode)
{
   bool dummy;
   if(tf == NULL)
   {
      tf = &dummy;
   }

   switch (mode)
   {
   case V3D_PRIM_MODE_POINTS_TF:       *tf = true; return V3D_PRIM_MODE_POINTS;
   case V3D_PRIM_MODE_LINES_TF:        *tf = true; return V3D_PRIM_MODE_LINES;
   case V3D_PRIM_MODE_LINE_LOOP_TF:    *tf = true; return V3D_PRIM_MODE_LINE_LOOP;
   case V3D_PRIM_MODE_LINE_STRIP_TF:   *tf = true; return V3D_PRIM_MODE_LINE_STRIP;
   case V3D_PRIM_MODE_TRIS_TF:         *tf = true; return V3D_PRIM_MODE_TRIS;
   case V3D_PRIM_MODE_TRI_STRIP_TF:    *tf = true; return V3D_PRIM_MODE_TRI_STRIP;
   case V3D_PRIM_MODE_TRI_FAN_TF:      *tf = true; return V3D_PRIM_MODE_TRI_FAN;
   default:                            *tf = false; return mode;
   }
}

static inline bool v3d_cl_store_instr_eof(const V3D_CL_INSTR_T *i)
{
   switch (i->opcode)
   {
   case V3D_CL_STORE_SUBSAMPLE:     return false;
   case V3D_CL_STORE_SUBSAMPLE_EX:  return i->u.store_subsample_ex.eof;
   case V3D_CL_STORE_GENERAL:       return i->u.store_general.eof;
   default:                         unreachable(); return false;
   }
}
#endif

struct v3d_cl_source_part
{
   v3d_addr_t addr;
   size_t size;
};

struct v3d_cl_source
{
   /* In an autochained control list, a single instruction/primitive may be
    * split into two discontiguous parts. parts[0].size should not be 0 if
    * parts[1].size is not 0. If parts[1].size is 0, then parts[1].addr should
    * point to the end of the first part. */
   struct v3d_cl_source_part parts[2];
};

static inline size_t v3d_cl_source_size(const struct v3d_cl_source *source)
{
   return source->parts[0].size + source->parts[1].size;
}

static inline v3d_addr_t v3d_cl_source_part_end(const struct v3d_cl_source_part *part)
{
   return v3d_addr_offset(part->addr, part->size);
}

static inline v3d_addr_t v3d_cl_source_end(const struct v3d_cl_source *source)
{
   return v3d_cl_source_part_end(&source->parts[1]);
}

static inline bool v3d_cl_source_part_equal(
   const struct v3d_cl_source_part *a, const struct v3d_cl_source_part *b)
{
   return (a->size == b->size) && ((a->size == 0) || (a->addr == b->addr));
}

static inline void v3d_cl_source_single_part(struct v3d_cl_source *source,
   v3d_addr_t addr, size_t size)
{
   source->parts[0].addr = addr;
   source->parts[0].size = size;
   source->parts[1].addr = v3d_cl_source_part_end(&source->parts[0]);
   source->parts[1].size = 0;
}

extern void v3d_cl_truncate_source(struct v3d_cl_source *source, size_t size);
extern void v3d_cl_extend_source(struct v3d_cl_source *source, v3d_addr_t addr, size_t size);
extern void v3d_cl_cat_sources(struct v3d_cl_source *a, const struct v3d_cl_source *b);

extern void v3d_cl_log_cat_bytes(
   struct log_cat *cat, log_level_t level, const char *line_prefix,
   const char *desc, const uint8_t *bytes, const struct v3d_cl_source *source);
extern void v3d_cl_log_cat_instr(
   struct log_cat *cat, log_level_t level, const char *line_prefix,
   const uint8_t *packed_instr, const struct v3d_cl_source *source);

#define v3d_cl_log_trace_instr(LINE_PREFIX, PACKED_INSTR, SOURCE) \
   v3d_cl_log_cat_instr(&log_default_cat, LOG_TRACE, LINE_PREFIX, PACKED_INSTR, SOURCE)

static inline bool v3d_cl_plist_fmts_equal(
   const V3D_CL_PRIM_LIST_FORMAT_T *a, const V3D_CL_PRIM_LIST_FORMAT_T *b)
{
   return (a->n_verts == b->n_verts) && (a->xy == b->xy) && (a->d3dpvsf == b->d3dpvsf);
}

extern size_t v3d_cl_sprint_plist_fmt(char *buf, size_t buf_size, size_t offset,
   const V3D_CL_PRIM_LIST_FORMAT_T *plist_fmt);
#define V3D_CL_SPRINT_PLIST_FMT(BUF_NAME, PLIST_FMT) \
   VCOS_SAFE_STRFUNC_TO_LOCAL_BUF(BUF_NAME, 256, v3d_cl_sprint_plist_fmt, PLIST_FMT)

#if V3D_VER_AT_LEAST(4,0,2,0)
static inline uint32_t v3d_cl_vpm_pack_to_width(v3d_cl_vpm_pack_t pack)
{
   switch(pack)
   {
      case V3D_CL_VPM_PACK_X16: return 16;
      case V3D_CL_VPM_PACK_X8: return 8;
      case V3D_CL_VPM_PACK_X4: return 4;
      default: unreachable();
   }
}

static inline uint32_t v3d_cl_geom_pack_to_width(v3d_cl_geom_output_pack_t pack)
{
   switch(pack)
   {
      case V3D_CL_GEOM_OUTPUT_PACK_X16: return 16;
      case V3D_CL_GEOM_OUTPUT_PACK_X8: return 8;
      case V3D_CL_GEOM_OUTPUT_PACK_X4: return 4;
      case V3D_CL_GEOM_OUTPUT_PACK_X1: return 1;
      default: unreachable();
   }
}

static inline uint32_t v3d_cl_per_patch_depth_to_width(uint32_t depth)
{
   assert(depth > 0 && depth <= 16);

   uint32_t words = depth * 8;
   if(words > 32)
   {
      return 1;
   }
   else if(words > 16)
   {
      return 4;
   }
   else if(words > 8)
   {
      return 8;
   }
   else
   {
      return 16;
   }
}
#endif

#if V3D_HAS_POLY_OFFSET_CLAMP
static inline void v3d_cl_set_depth_offset(uint8_t **ptr, float factor, float units, float clamp, uint32_t depth_bits)
#else
static inline void v3d_cl_set_depth_offset(uint8_t **ptr, float factor, float units, uint32_t depth_bits)
#endif
{
   /* The hardware always applies a constant unit of depth offset (2^-24). We must
    * therefore scale the units here according to depth_bits to compensate */
   if (depth_bits == 16)
      units *= (1 << 8);

#if V3D_HAS_POLY_OFFSET_CLAMP
   v3d_cl_depth_offset(ptr, factor, units, clamp);
#else
   v3d_cl_depth_offset(ptr, factor, units);
#endif
}

typedef void (*v3d_cl_flag_set_func)(uint8_t **cl, uint32_t offset,
                                     v3d_flags_action_t lower_action,
                                     v3d_flags_action_t higher_action,
                                     uint32_t flags);

typedef void (*v3d_cl_flag_zero_func)(uint8_t **cl);

/* Set any of the various vary flags. Each uint32_t in flags should contain
 * at most V3D_VARY_FLAGS_PER_WORD bits for flags, starting from the LSB. */
void v3d_cl_write_vary_flags(uint8_t **instr, const uint32_t *flags,
                             v3d_cl_flag_set_func set, v3d_cl_flag_zero_func zero);

extern void v3d_cl_viewport_offset_from_rect(uint8_t **cl,
   int x, int y, unsigned width, unsigned height);

VCOS_EXTERN_C_END

#endif
