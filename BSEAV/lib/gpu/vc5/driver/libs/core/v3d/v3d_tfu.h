/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_gen.h"
#include "libs/core/gfx_buffer/gfx_buffer_slow_conv.h"

EXTERN_C_BEGIN

static inline v3d_tfu_rgbord_t v3d_tfu_reverse_rgbord(v3d_tfu_rgbord_t rgbord)
{
   switch (rgbord)
   {
   case V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV:   return V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU;
   case V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU:   return V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV;
   case V3D_TFU_RGBORD_ARGB_OR_YYUV:            return V3D_TFU_RGBORD_BGRA_OR_VUYY;
   case V3D_TFU_RGBORD_BGRA_OR_VUYY:            return V3D_TFU_RGBORD_ARGB_OR_YYUV;
   default:                                     unreachable(); return V3D_TFU_RGBORD_INVALID;
   }
}

typedef enum
{
   V3D_TFU_YUV_COL_SPACE_NONE = 0,
   V3D_TFU_YUV_COL_SPACE_REC709,
   V3D_TFU_YUV_COL_SPACE_REC601,
   V3D_TFU_YUV_COL_SPACE_JPEG,
} v3d_tfu_yuv_col_space_t;

static inline v3d_tfu_type_t v3d_tfu_type_yuv_420_2plane(v3d_tfu_yuv_col_space_t col_space)
{
   switch (col_space)
   {
   case V3D_TFU_YUV_COL_SPACE_REC709:  return V3D_TFU_TYPE_YUV_420_2PLANE_REC709;
   case V3D_TFU_YUV_COL_SPACE_REC601:  return V3D_TFU_TYPE_YUV_420_2PLANE_REC601;
   case V3D_TFU_YUV_COL_SPACE_JPEG:    return V3D_TFU_TYPE_YUV_420_2PLANE_JPEG;
   default:                            unreachable(); return V3D_TFU_TYPE_INVALID;
   }
}

static inline v3d_tfu_type_t v3d_tfu_type_yuv_422_2plane(v3d_tfu_yuv_col_space_t col_space)
{
   switch (col_space)
   {
   case V3D_TFU_YUV_COL_SPACE_REC709:  return V3D_TFU_TYPE_YUV_422_2PLANE_REC709;
   case V3D_TFU_YUV_COL_SPACE_REC601:  return V3D_TFU_TYPE_YUV_422_2PLANE_REC601;
   case V3D_TFU_YUV_COL_SPACE_JPEG:    return V3D_TFU_TYPE_YUV_422_2PLANE_JPEG;
   default:                            unreachable(); return V3D_TFU_TYPE_INVALID;
   }
}

static inline v3d_tfu_type_t v3d_tfu_type_yuyv_422_1plane(v3d_tfu_yuv_col_space_t col_space)
{
   switch (col_space)
   {
   case V3D_TFU_YUV_COL_SPACE_REC709:  return V3D_TFU_TYPE_YUYV_422_1PLANE_REC709;
   case V3D_TFU_YUV_COL_SPACE_REC601:  return V3D_TFU_TYPE_YUYV_422_1PLANE_REC601;
   case V3D_TFU_YUV_COL_SPACE_JPEG:    return V3D_TFU_TYPE_YUYV_422_1PLANE_JPEG;
   default:                            unreachable(); return V3D_TFU_TYPE_INVALID;
   }
}

static inline v3d_tfu_type_t v3d_tfu_type_yuv_420_3plane(v3d_tfu_yuv_col_space_t col_space)
{
   switch (col_space)
   {
   case V3D_TFU_YUV_COL_SPACE_REC709:  return V3D_TFU_TYPE_YUV_420_3PLANE_REC709;
   case V3D_TFU_YUV_COL_SPACE_REC601:  return V3D_TFU_TYPE_YUV_420_3PLANE_REC601;
   case V3D_TFU_YUV_COL_SPACE_JPEG:    return V3D_TFU_TYPE_YUV_420_3PLANE_JPEG;
   default:                            unreachable(); return V3D_TFU_TYPE_INVALID;
   }
}

static inline v3d_tfu_yuv_col_space_t v3d_yuv_col_space_from_tfu_type(v3d_tfu_type_t tfu_type)
{
   switch (tfu_type)
   {
   case V3D_TFU_TYPE_YUV_420_3PLANE_REC709:
   case V3D_TFU_TYPE_YUV_420_2PLANE_REC709:
   case V3D_TFU_TYPE_YUV_422_2PLANE_REC709:
   case V3D_TFU_TYPE_YUYV_422_1PLANE_REC709:
      return V3D_TFU_YUV_COL_SPACE_REC709;
   case V3D_TFU_TYPE_YUV_420_3PLANE_REC601:
   case V3D_TFU_TYPE_YUV_420_2PLANE_REC601:
   case V3D_TFU_TYPE_YUV_422_2PLANE_REC601:
   case V3D_TFU_TYPE_YUYV_422_1PLANE_REC601:
      return V3D_TFU_YUV_COL_SPACE_REC601;
   case V3D_TFU_TYPE_YUV_420_3PLANE_JPEG:
   case V3D_TFU_TYPE_YUV_420_2PLANE_JPEG:
   case V3D_TFU_TYPE_YUV_422_2PLANE_JPEG:
   case V3D_TFU_TYPE_YUYV_422_1PLANE_JPEG:
      return V3D_TFU_YUV_COL_SPACE_JPEG;
   default:
      return V3D_TFU_YUV_COL_SPACE_NONE;
   }
}

static inline void v3d_tfu_yuv_coeff_set(GFX_BUFFER_XFORM_OPTIONS_T *opts,
   uint32_t ay,
   uint32_t arr,
   uint32_t arc,
   uint32_t agb,
   uint32_t agr,
   uint32_t agc,
   uint32_t abb,
   uint32_t abc)
{
   opts->yuv_to_srgb.ay  = ay;
   opts->yuv_to_srgb.arr = arr;
   opts->yuv_to_srgb.arc = arc;
   opts->yuv_to_srgb.agb = agb;
   opts->yuv_to_srgb.agr = agr;
   opts->yuv_to_srgb.agc = agc;
   opts->yuv_to_srgb.abb = abb;
   opts->yuv_to_srgb.abc = abc;
}

static inline v3d_memory_format_t v3d_tfu_iformat_to_memory_format(
   v3d_tfu_iformat_t tfu_iformat)
{
   switch (tfu_iformat)
   {
   case V3D_TFU_IFORMAT_RASTER:      return V3D_MEMORY_FORMAT_RASTER;
   case V3D_TFU_IFORMAT_LINEARTILE:  return V3D_MEMORY_FORMAT_LINEARTILE;
   case V3D_TFU_IFORMAT_UBLINEAR_1:  return V3D_MEMORY_FORMAT_UBLINEAR_1;
   case V3D_TFU_IFORMAT_UBLINEAR_2:  return V3D_MEMORY_FORMAT_UBLINEAR_2;
   case V3D_TFU_IFORMAT_UIF_NO_XOR:  return V3D_MEMORY_FORMAT_UIF_NO_XOR;
   case V3D_TFU_IFORMAT_UIF_XOR:     return V3D_MEMORY_FORMAT_UIF_XOR;
   default: unreachable(); return V3D_MEMORY_FORMAT_INVALID;
   }
}

static inline v3d_memory_format_t v3d_tfu_oformat_to_memory_format(
   v3d_tfu_oformat_t tfu_oformat)
{
   switch (tfu_oformat)
   {
   case V3D_TFU_OFORMAT_LINEARTILE:  return V3D_MEMORY_FORMAT_LINEARTILE;
   case V3D_TFU_OFORMAT_UBLINEAR_1:  return V3D_MEMORY_FORMAT_UBLINEAR_1;
   case V3D_TFU_OFORMAT_UBLINEAR_2:  return V3D_MEMORY_FORMAT_UBLINEAR_2;
   case V3D_TFU_OFORMAT_UIF_NO_XOR:  return V3D_MEMORY_FORMAT_UIF_NO_XOR;
   case V3D_TFU_OFORMAT_UIF_XOR:     return V3D_MEMORY_FORMAT_UIF_XOR;
   default: unreachable(); return V3D_MEMORY_FORMAT_INVALID;
   }
}

static inline v3d_tfu_iformat_t v3d_tfu_iformat_from_memory_format(
   v3d_memory_format_t memory_format)
{
   switch (memory_format)
   {
   case V3D_MEMORY_FORMAT_RASTER:      return V3D_TFU_IFORMAT_RASTER;
   case V3D_MEMORY_FORMAT_LINEARTILE:  return V3D_TFU_IFORMAT_LINEARTILE;
   case V3D_MEMORY_FORMAT_UBLINEAR_1:  return V3D_TFU_IFORMAT_UBLINEAR_1;
   case V3D_MEMORY_FORMAT_UBLINEAR_2:  return V3D_TFU_IFORMAT_UBLINEAR_2;
   case V3D_MEMORY_FORMAT_UIF_NO_XOR:  return V3D_TFU_IFORMAT_UIF_NO_XOR;
   case V3D_MEMORY_FORMAT_UIF_XOR:     return V3D_TFU_IFORMAT_UIF_XOR;
   default: unreachable(); return V3D_TFU_IFORMAT_INVALID;
   }
}

static inline v3d_tfu_oformat_t v3d_tfu_oformat_from_memory_format(
   v3d_memory_format_t memory_format)
{
   switch (memory_format)
   {
   case V3D_MEMORY_FORMAT_LINEARTILE:  return V3D_TFU_OFORMAT_LINEARTILE;
   case V3D_MEMORY_FORMAT_UBLINEAR_1:  return V3D_TFU_OFORMAT_UBLINEAR_1;
   case V3D_MEMORY_FORMAT_UBLINEAR_2:  return V3D_TFU_OFORMAT_UBLINEAR_2;
   case V3D_MEMORY_FORMAT_UIF_NO_XOR:  return V3D_TFU_OFORMAT_UIF_NO_XOR;
   case V3D_MEMORY_FORMAT_UIF_XOR:     return V3D_TFU_OFORMAT_UIF_XOR;
   default: unreachable(); return V3D_TFU_OFORMAT_INVALID;
   }
}

static inline bool v3d_tfu_iformat_vertical_pitch(
   v3d_tfu_iformat_t tfu_iformat)
{
   switch (tfu_iformat)
   {
   case V3D_TFU_IFORMAT_SAND_128:
   case V3D_TFU_IFORMAT_SAND_256:
      return true;
   case V3D_TFU_IFORMAT_RASTER:
      return false;
   default:
      unreachable(); return false;
   }
}

typedef struct {
   V3D_TFUSU_T su;

   struct
   {
      V3D_TFUICFG_T cfg;
      v3d_addr_t ia;
      v3d_addr_t ca; /* 2-plane YUV: UV, 3-plane YUV: V */
      v3d_addr_t ua; /* 3-plane YUV: U */
      V3D_TFUIIS_T is;
      V3D_TFUIOA_T oa;
      V3D_TFUIOS_T os;
   } i;

   V3D_TFUCOEF0_T coef0;
   V3D_TFUCOEF1_T coef1;
   V3D_TFUCOEF2_T coef2;
   V3D_TFUCOEF3_T coef3;
} V3D_TFU_REGS_T;

typedef struct {
   uint32_t su;
   uint32_t icfg;
   v3d_addr_t iia;
   v3d_addr_t ica;
   v3d_addr_t iua;
   uint32_t iis;
   uint32_t ioa;
   uint32_t ios;
   uint32_t coef0;
   uint32_t coef1;
   uint32_t coef2;
   uint32_t coef3;
} V3D_TFU_REGS_PACKED_T;

#define V3D_TFU_MAX_SRC_PLANES 3

typedef struct {
   bool interrupt_on_complete;
   bool disable_main_texture_write;
   bool use_programmable_yuv_coef;
   v3d_tfu_rgbord_t src_channel_order;
   bool flip_y;
   bool srgb;
   uint32_t num_mip_levels; /* includes level 0 */
   v3d_tfu_type_t src_ttype;
   v3d_tfu_iformat_t src_memory_format;
   v3d_tfu_oformat_t dst_memory_format;
   v3d_addr_t src_base_addrs[V3D_TFU_MAX_SRC_PLANES];
   uint32_t src_strides[2];
   uint32_t dst_pad_in_uif_blocks;
   v3d_addr_t dst_base_addr;
   uint32_t width;
   uint32_t height;
   GFX_BUFFER_XFORM_OPTIONS_T yuv_coef;
} V3D_TFU_COMMAND_T;

extern void v3d_tfu_regs_collect(
   V3D_TFU_COMMAND_T *cmd,
   const V3D_TFU_REGS_T *r);

extern void v3d_tfu_regs_uncollect(
   V3D_TFU_REGS_T *r,
   const V3D_TFU_COMMAND_T *cmd);

void v3d_tfu_regs_pack_command(
   V3D_TFU_REGS_PACKED_T *pr,
   const V3D_TFU_COMMAND_T *cmd);

static inline bool v3d_is_tfu_ttype_yuv(v3d_tfu_type_t ttype)
{
   switch (ttype)
   {
   case V3D_TFU_TYPE_YUV_420_3PLANE_REC709:
   case V3D_TFU_TYPE_YUV_420_3PLANE_REC601:
   case V3D_TFU_TYPE_YUV_420_3PLANE_JPEG:
   case V3D_TFU_TYPE_YUV_420_2PLANE_REC709:
   case V3D_TFU_TYPE_YUV_420_2PLANE_REC601:
   case V3D_TFU_TYPE_YUV_420_2PLANE_JPEG:
   case V3D_TFU_TYPE_YUV_422_2PLANE_REC709:
   case V3D_TFU_TYPE_YUV_422_2PLANE_REC601:
   case V3D_TFU_TYPE_YUV_422_2PLANE_JPEG:
   case V3D_TFU_TYPE_YUYV_422_1PLANE_REC709:
   case V3D_TFU_TYPE_YUYV_422_1PLANE_REC601:
   case V3D_TFU_TYPE_YUYV_422_1PLANE_JPEG:
      return true;
   default:
      return false;
   }
}

static inline bool v3d_tfu_cmd_uses_yuv_coef(const V3D_TFU_COMMAND_T *cmd)
{
   return v3d_is_tfu_ttype_yuv(cmd->src_ttype) &&
          cmd->use_programmable_yuv_coef;
}

bool v3d_build_tfu_cmd(V3D_TFU_COMMAND_T *cmd,
      const GFX_BUFFER_DESC_T *src_desc,
      const GFX_BUFFER_DESC_T *dst_desc,
      unsigned num_dst_levels, bool skip_dst_level_0,
      const v3d_addr_t src_base_addr[GFX_BUFFER_MAX_PLANES],
      v3d_addr_t dst_base_addr);

void v3d_tfu_calc_src_desc(
   v3d_addr_t *base_addr, GFX_BUFFER_DESC_T *desc,
   v3d_tfu_yuv_col_space_t *yuv_col_space, // May be NULL
   const V3D_TFU_COMMAND_T *cmd, unsigned dram_map_version);

// descs[0] shouldn't be used if cmd->disable_main_texture_write
void v3d_tfu_calc_dst_descs(
   v3d_addr_t *base_addr, GFX_BUFFER_DESC_T *descs,
   const V3D_TFU_COMMAND_T *cmd);

EXTERN_C_END
