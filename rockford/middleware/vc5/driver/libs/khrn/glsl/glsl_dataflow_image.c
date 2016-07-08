/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_dataflow_image.h"
#include "glsl_dataflow_builder.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_tex_params.h"
#include "glsl_imageunit_swizzling.h"
#include  "libs/core/gfx_buffer/gfx_buffer_uif_config.h"
#include  "libs/core/lfmt/lfmt.h"
#include  "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"

typedef struct
{
   Dataflow *lx_swizzling;
   Dataflow *lx_addr;
   Dataflow *lx_pitch;
   Dataflow *lx_slice_pitch;
}df_img_info;

typedef struct
{
   Dataflow *bytes_per_texel;
   Dataflow *ut_w;
   Dataflow *ut_h;
}df_base_detail;

static GFX_LFMT_T fmt_qualifier_to_fmt(FormatQualifier fmt)
{
   switch (fmt)
   {
   case FMT_RGBA32F:
      return GFX_LFMT_R32_G32_B32_A32_FLOAT;
   case FMT_RGBA16F:
      return GFX_LFMT_R16_G16_B16_A16_FLOAT;
   case FMT_R32F:
      return GFX_LFMT_R32_FLOAT;

   case FMT_RGBA32I:
      return GFX_LFMT_R32_G32_B32_A32_INT;
   case FMT_RGBA16I:
      return GFX_LFMT_R16_G16_B16_A16_INT;
   case FMT_RGBA8I:
      return GFX_LFMT_R8_G8_B8_A8_INT;
   case FMT_R32I:
      return GFX_LFMT_R32_INT;

   case FMT_RGBA32UI:
      return GFX_LFMT_R32_G32_B32_A32_UINT;
   case FMT_RGBA16UI:
      return GFX_LFMT_R16_G16_B16_A16_UINT;
   case FMT_RGBA8UI:
      return GFX_LFMT_R8_G8_B8_A8_UINT;
   case FMT_R32UI:
      return GFX_LFMT_R32_UINT;

   case FMT_RGBA8:
      return GFX_LFMT_R8_G8_B8_A8_UNORM;
   case FMT_RGBA8_SNORM:
      return GFX_LFMT_R8_G8_B8_A8_SNORM;
   default:
      UNREACHABLE();
      break;
   }
}

GLenum glsl_fmt_qualifier_to_gl_enum(FormatQualifier fmt_qual)
{
   GFX_LFMT_T fmt = fmt_qualifier_to_fmt(fmt_qual);
   GLenum internalformat = gfx_sized_internalformat_from_api_fmt_maybe(fmt);
   assert(internalformat != GL_NONE);
   return internalformat;
}


typedef struct
{
   Dataflow *uif_col_w_in_ub;
   Dataflow *uif_ub_size;
   Dataflow *utile_size;
}df_ub_const;

static Dataflow* get_y_or_xor_y_in_ub(Dataflow *x_in_uif_col, Dataflow *img_swizzling,
      Dataflow *y_in_ub)
{
   //odd_col = (x_in_uif_col) % 2;
   Dataflow *odd_col = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, x_in_uif_col ,
         glsl_dataflow_construct_const_uint(1));
   odd_col = glsl_dataflow_convert_type(odd_col, DF_BOOL);

   Dataflow *xor_mode = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, img_swizzling,
                           glsl_dataflow_construct_const_uint(GLSL_IMGUNIT_SWIZZLING_UIF_XOR));

   //y_xor_in_ub = y_in_ub ^ GFX_UIF_XOR_ADDR
   Dataflow *y_xor_in_ub = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_XOR, y_in_ub,
         glsl_dataflow_construct_const_uint(GFX_UIF_XOR_ADDR));

   // (xor_mode && odd_col) ? y_xor_in_ub : y_in_ub
   Dataflow *cond = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, odd_col, xor_mode);
   return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond, y_xor_in_ub, y_in_ub);
}

#ifdef LIKE_BUFFER_SLOW_CONV
static Dataflow *get_ublinear_ub_number(const df_base_detail *df_bd, const df_img_info *img, Dataflow *ub_w,
      Dataflow *x_in_ub, Dataflow *y_in_ub)
{
   Dataflow *c_one = glsl_dataflow_construct_const_uint(1);

   //max_ub_w_in_bytes = 2 * ub_w * df_bd->bytes_per_texel
   Dataflow *max_ub_w_in_bytes =
      glsl_dataflow_construct_binary_op(DATAFLOW_SHL,
            glsl_dataflow_construct_binary_op(DATAFLOW_MUL, df_bd->bytes_per_texel, ub_w), c_one);

   // ubnumber = (plane_pitch == max_ub_w_in_bytes)  ? (2 * y_in_ub) + x_in_ub : y_in_ub
   Dataflow *cond = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, img->lx_pitch, max_ub_w_in_bytes);
   return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond,
         glsl_dataflow_construct_binary_op(DATAFLOW_ADD,
            glsl_dataflow_construct_binary_op(DATAFLOW_SHL, y_in_ub, c_one), x_in_ub), y_in_ub);
}

// for uif and uif_xor
static Dataflow *get_uif_ub_number(const df_base_detail *df_bd,
      const df_img_info *img, const df_ub_const *ub_const,
      Dataflow *ub_w, Dataflow *ub_h, Dataflow *x_in_ub, Dataflow *y_in_ub)
{
   Dataflow *c_one = glsl_dataflow_construct_const_uint(1);

   //uif_col_w = ub_w * UIF_COL_W_IN_UB;
   Dataflow *uif_col_w = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, ub_w, ub_const->uif_col_w_in_ub);

   // padded_img_h_in_ub = (img_pitch * uif_col_w)/(UIF_COLW_IN_UB * UIF_UB_SIZE)
   Dataflow *d1 = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, img->lx_pitch, uif_col_w);
   Dataflow *d2 = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, ub_const->uif_col_w_in_ub, ub_const->uif_ub_size);
   Dataflow *padded_img_h_in_ub = glsl_dataflow_construct_binary_op(DATAFLOW_DIV, d1, d2);

   //x_in_uif_col = x_in_ub / UIF_COL_W_IN_UB
   Dataflow *x_in_uif_col = glsl_dataflow_construct_binary_op(DATAFLOW_DIV, x_in_ub, ub_const->uif_col_w_in_ub);

   //ub_number = (x_in_uif_col * (padded_img_h_in_ub - 1) +  get_y_or_xor_y_in_ub()) *  UIF_COL_W_IN_UB +
   //            x_in_ub
   Dataflow *ub_number = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, x_in_uif_col,
         glsl_dataflow_construct_binary_op(DATAFLOW_SUB, padded_img_h_in_ub, c_one));
   ub_number = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, ub_number,
         get_y_or_xor_y_in_ub(x_in_uif_col, img->lx_swizzling, y_in_ub));
   ub_number = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, ub_number, ub_const->uif_col_w_in_ub);
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, ub_number, x_in_ub);
}
#endif

static Dataflow *get_ublinear_ub_offset(const df_base_detail *df_bd,
      const df_img_info *img, const df_ub_const *ub_const,
      Dataflow *ub_h,  Dataflow *x_in_ub, Dataflow *y_in_ub)
{
   //offset_adj_rows_of_ub(in bytes) = ub_h * img_pitch
   Dataflow *offset_adj_rows_of_ub = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, ub_h, img->lx_pitch);

   // offset = y_in_ub * adj_rows_of_ub + x_in_ub * UIF_UB_SIZE
   Dataflow *d1 = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, y_in_ub, offset_adj_rows_of_ub);
   Dataflow *d2 = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, x_in_ub, ub_const->uif_ub_size);
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD,  d1, d2 );
}

// for uif and uif_xor
static Dataflow *get_uif_ub_offset(const df_base_detail *df_bd,
      const df_img_info *img, const df_ub_const *ub_const,
      Dataflow *ub_w, Dataflow *x_in_ub, Dataflow *y_in_ub)
{
   //offset_adj_uif_cols(in_bytes) = img_pitch * (ub_w * UIF_COL_W_IN_UB);
   Dataflow *offset_adj_uif_cols = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, img->lx_pitch,
         glsl_dataflow_construct_binary_op(DATAFLOW_MUL, ub_w, ub_const->uif_col_w_in_ub));

   //x_in_uif_col = x_in_ub / UIF_COL_W_IN_UB
   Dataflow *x_in_uif_col = glsl_dataflow_construct_binary_op(DATAFLOW_DIV, x_in_ub, ub_const->uif_col_w_in_ub);

   //x_in_ub % UIF_COL_W_IN_UB
   Dataflow *rem_x_in_ub = glsl_dataflow_construct_binary_op(DATAFLOW_REM,
                  x_in_ub, ub_const->uif_col_w_in_ub);

   //res = x_in_uif_col * offset_adj_uif_cols +
   //      (y_or_xor_y_in_ub * UIF_COL_W_IN_UB + rem_x_in_ub)  * UIF_UB_SIZE
   Dataflow *d1 = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, x_in_uif_col, offset_adj_uif_cols);
   Dataflow *d2 = glsl_dataflow_construct_binary_op(DATAFLOW_MUL,
                     get_y_or_xor_y_in_ub(x_in_uif_col, img->lx_swizzling, y_in_ub),
                     ub_const->uif_col_w_in_ub);
   d2 = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, d2, rem_x_in_ub);
   d2 = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, d2, ub_const->uif_ub_size);
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, d1, d2);
}

static Dataflow *get_offset_inside_utile(Dataflow *x, Dataflow *y, const df_base_detail *df_bd)
{
   //offset_inside_utile = ((y % ut_h) * ut_w + (x % ut_w)) * bytes_per_texel;
   Dataflow *e1 = glsl_dataflow_construct_binary_op(DATAFLOW_MUL,
         glsl_dataflow_construct_binary_op(DATAFLOW_REM, y, df_bd->ut_h),
         df_bd->ut_w);
   Dataflow *e2 = glsl_dataflow_construct_binary_op(DATAFLOW_REM, x, df_bd->ut_w);

   return glsl_dataflow_construct_binary_op(DATAFLOW_MUL,
            glsl_dataflow_construct_binary_op(DATAFLOW_ADD, e1, e2),
            df_bd->bytes_per_texel);
}

static Dataflow *get_uif_ubl_offset(const df_base_detail *df_bd,
      const df_img_info *img, const df_ub_const *ub_const,
      Dataflow *x , Dataflow *y, Dataflow *z, Dataflow *x_in_utiles, Dataflow *y_in_utiles)
{
   //ub_w = ut_w * 2; ub_h = ut_h * 2;
   Dataflow *c_one = glsl_dataflow_construct_const_uint(1);
   Dataflow *ub_w = glsl_dataflow_construct_binary_op(DATAFLOW_SHL, df_bd->ut_w, c_one);
   Dataflow *ub_h = glsl_dataflow_construct_binary_op(DATAFLOW_SHL, df_bd->ut_h, c_one);
   Dataflow *x_in_ub = glsl_dataflow_construct_binary_op(DATAFLOW_DIV, x, ub_w);
   Dataflow *y_in_ub = glsl_dataflow_construct_binary_op(DATAFLOW_DIV, y, ub_h);

   //ub_number = img->lx_swizzling == UBLINEAR ? get_ublinear_ub_number : get_uif
   Dataflow *cond = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, img->lx_swizzling,
                           glsl_dataflow_construct_const_uint(GLSL_IMGUNIT_SWIZZLING_UBLINEAR));
   Dataflow *ub_offset;
#ifndef LIKE_BUFFER_SLOW_CONV
   ub_offset = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond,
                             get_ublinear_ub_offset(df_bd, img, ub_const, ub_h, x_in_ub, y_in_ub),
                             get_uif_ub_offset(df_bd, img, ub_const, ub_w, x_in_ub, y_in_ub));
#else
   Dataflow *ub_number = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond,
                             get_ublinear_ub_number(df_bd, img, ub_w, x_in_ub, y_in_ub),
                             get_uif_ub_number(df_bd, img, ub_const, ub_w, ub_h, x_in_ub, y_in_ub));

   //ub_number * UIF_UB_SIZE
   ub_offset = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, ub_number, ub_const->uif_ub_size);
#endif
   // ut_x_within_ub = (x_in_utiles) % 2;  ut_y_within_ub = (y_in_utiles) % 2;
   Dataflow *ut_x_within_ub = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, x_in_utiles, c_one);
   Dataflow *ut_y_within_ub = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, y_in_utiles, c_one);

   // ut_offset_within_ub = (ut_y_within_ub * 2 + ut_x_within_ub) * UTILE_SIZE;
   Dataflow *ut_offset_within_ub =
      glsl_dataflow_construct_binary_op(DATAFLOW_MUL,
            glsl_dataflow_construct_binary_op(DATAFLOW_ADD,
              glsl_dataflow_construct_binary_op(DATAFLOW_SHL, ut_y_within_ub, c_one), ut_x_within_ub),
            ub_const->utile_size);

   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, ub_offset, ut_offset_within_ub);
}

static Dataflow *get_lt_ut_offset(const df_base_detail *df_bd,
      const df_img_info *img, const df_ub_const *ub_const,
      Dataflow *x_in_utiles, Dataflow *y_in_utiles)
{
   //ut_offset = (y_in_utiles * ut_h * img_pitch) + (x_in_utiles * UTILE_SIZE);
   Dataflow *d1 = glsl_dataflow_construct_binary_op(DATAFLOW_MUL,
         glsl_dataflow_construct_binary_op(DATAFLOW_MUL, y_in_utiles, df_bd->ut_h), img->lx_pitch);
   Dataflow *d2 = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, x_in_utiles, ub_const->utile_size);
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, d1, d2);
}

static Dataflow  *construct_dataflow_img_addr(Dataflow *sampler, GFX_LFMT_T fmt,
      Dataflow *x, Dataflow *y, Dataflow *z, Dataflow *elem_no)
{

   df_img_info img;
   img.lx_swizzling = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_SWIZZLING);
   img.lx_addr = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_LX_ADDR);
   img.lx_pitch = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_LX_PITCH);
   if (z != NULL)
      img.lx_slice_pitch = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_LX_SLICE_PITCH);

   GFX_LFMT_BASE_DETAIL_T bd;
   df_base_detail df_bd;
   gfx_lfmt_base_detail(&bd, fmt);
   assert(bd.block_w == 1 && bd.block_h == 1 &&  bd.block_d == 1);
   df_bd.bytes_per_texel = glsl_dataflow_construct_const_uint(bd.bytes_per_block);
   df_bd.ut_w = glsl_dataflow_construct_const_uint(bd.ut_w_in_blocks_2d);
   df_bd.ut_h = glsl_dataflow_construct_const_uint(bd.ut_h_in_blocks_2d);

   df_ub_const ub_const;
   ub_const.uif_col_w_in_ub = glsl_dataflow_construct_const_uint(GFX_UIF_COL_W_IN_UB);
   ub_const.uif_ub_size = glsl_dataflow_construct_const_uint(GFX_UIF_UB_SIZE);
   ub_const.utile_size = glsl_dataflow_construct_const_uint(GFX_LFMT_UTILE_SIZE);

   Dataflow *x_in_utiles = glsl_dataflow_construct_binary_op(DATAFLOW_DIV, x, df_bd.ut_w);
   Dataflow *y_in_utiles = glsl_dataflow_construct_binary_op(DATAFLOW_DIV, y, df_bd.ut_h);

   Dataflow *c_uif_ubl =
      glsl_dataflow_construct_const_uint(GLSL_IMGUNIT_SWIZZLING_UIF | GLSL_IMGUNIT_SWIZZLING_UIF_XOR | GLSL_IMGUNIT_SWIZZLING_UBLINEAR);

   // if (uif | uif_xor | ubl) uif_ubl_offset else lt_ut_offset
   Dataflow *cond = glsl_dataflow_convert_type(
         glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, img.lx_swizzling, c_uif_ubl),
         DF_BOOL);
   Dataflow *uif_ubl_offset = get_uif_ubl_offset(&df_bd, &img, &ub_const, x, y, z, x_in_utiles, y_in_utiles);
   Dataflow *lt_ut_offset = get_lt_ut_offset(&df_bd, &img, &ub_const, x_in_utiles, y_in_utiles);
   Dataflow *offset = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond, uif_ubl_offset, lt_ut_offset);

   //offset += offset_inside_utile
   offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, get_offset_inside_utile(x, y, &df_bd));

   //offset +=  z * img_slice_pitch;
   if (z != NULL)
   {
      offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset,
               glsl_dataflow_construct_binary_op(DATAFLOW_MUL, z,img.lx_slice_pitch));
   }

   // offset += elem_no * arr_stride
   if (elem_no)
   {
      Dataflow *arr_stride = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_ARR_STRIDE);
      offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset,
            glsl_dataflow_construct_binary_op(DATAFLOW_MUL, elem_no, arr_stride));
   }
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, img.lx_addr, offset);
}

Dataflow *just_pack_uint_4x8(Dataflow *data[4])
{
   Dataflow *res = data[0];
   for (unsigned i = 1; i < 4; ++i)
   {
      //data[i] = data [i] * i << 8;
      data[i] = glsl_dataflow_construct_binary_op(DATAFLOW_SHL, data[i],
            glsl_dataflow_construct_const_uint(i * 8));
      //res = res | data[i]
      res = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_OR, res, data[i]);
   }
   return res;
}

Dataflow *just_pack_uint_2x16(Dataflow *data[2])
{
   // data[0] | data[1] << 16
   data[1] = glsl_dataflow_construct_binary_op(DATAFLOW_SHL, data[1],
               glsl_dataflow_construct_const_uint(16));
   return glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_OR, data[1], data[0]);
}

static Dataflow *clamp(Dataflow *x, Dataflow *min_val, Dataflow *max_val)
{
   //min(max(x, minVal), maxVal);\n"
   return glsl_dataflow_construct_binary_op(DATAFLOW_MIN,
             glsl_dataflow_construct_binary_op(DATAFLOW_MAX, x, min_val),
             max_val);
}

static Dataflow *pack_unorm_4x8(Dataflow *data[4])
{
   for (unsigned i = 0; i < 4; ++i)
   {
      //data[i] = uint(uint(round(clamp(float_val, 0, +1) * 255)))
      data[i] = clamp(data[i], glsl_dataflow_construct_const_float(0),
            glsl_dataflow_construct_const_float(1));
      data[i] = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, data[i],
            glsl_dataflow_construct_const_float(255));
      data[i] = glsl_dataflow_construct_unary_op(DATAFLOW_NEAREST, data[i]);
      data[i] = glsl_dataflow_convert_type(data[i], DF_INT);
      data[i] = glsl_dataflow_convert_type(data[i], DF_UINT);

   }
   return just_pack_uint_4x8(data);
}

static Dataflow *pack_snorm_4x8(Dataflow *data[4])
{
   for (unsigned i = 0; i < 4; ++i)
   {
      //data[i] = uint(uint(round(clamp(float_val, -1, +1) * 127))) & 0xFFu
      data[i] = clamp(data[i], glsl_dataflow_construct_const_float(-1),
            glsl_dataflow_construct_const_float(1));
      data[i] = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, data[i],
            glsl_dataflow_construct_const_float(127));
      data[i] = glsl_dataflow_construct_unary_op(DATAFLOW_NEAREST, data[i]);
      data[i] = glsl_dataflow_convert_type(data[i], DF_INT);
      data[i] = glsl_dataflow_convert_type(data[i], DF_UINT);

      data[i] = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, data[i],
            glsl_dataflow_construct_const_uint(0xff));

   }

   return just_pack_uint_4x8(data);
}


static Dataflow *construct_dataflow_pack_data(GFX_LFMT_T fmt, Dataflow *data[4]) {
   Dataflow *v[4] = { NULL, };

   /* we always have a channel red */
   unsigned channel_size = gfx_lfmt_red_bits(fmt);
   assert(channel_size != 0);

   unsigned n_channels = gfx_lfmt_num_slots_from_channels(fmt);
   assert(n_channels == 1 || n_channels == 4);

   assert(gfx_lfmt_num_slots_from_type(fmt) == 1);
   GFX_LFMT_TYPE_T fmt_type = fmt & GFX_LFMT_TYPE_MASK;

   if (channel_size == 32)
   {
      for (unsigned i = 0; i < n_channels; ++i)
         v[i] = glsl_dataflow_construct_reinterp(data[i], DF_UINT);
   }
   else
   {
      switch(fmt_type)
      {
      case GFX_LFMT_TYPE_INT:
      case GFX_LFMT_TYPE_UINT:
         {
            assert(n_channels == 4);
            if (fmt_type == GFX_LFMT_TYPE_INT)
            {
               // -1 * 2^(channel_size-1) to 2^(channel_size-1) - 1;
               unsigned val = 1u << (channel_size - 1);
               for (unsigned i = 0; i < 4; ++i)
               {
                  data[i] = clamp(data[i],
                        glsl_dataflow_construct_const_int(-1 * val),
                        glsl_dataflow_construct_const_int(val - 1));
                  unsigned mask = (1u << channel_size) - 1;
                  data[i] = glsl_dataflow_construct_reinterp(data[i], DF_UINT);
                  data[i] = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, data[i],
                                glsl_dataflow_construct_const_uint(mask));
               }
            }
            else
            {
               // 0 to 2^(channel_size)-1;
               unsigned max_val = (1 << channel_size) - 1 ;
               for (unsigned i = 0; i < 4; ++i)
                  data[i] = glsl_dataflow_construct_binary_op(DATAFLOW_MIN, data[i],
                       glsl_dataflow_construct_const_uint(max_val));
            }

            switch (channel_size)
            {
            case 16:
               v[0] = just_pack_uint_2x16(data);
               v[1] = just_pack_uint_2x16(&data[2]);
               break;
            case 8:
               v[0] = just_pack_uint_4x8(data);
               break;
            default:UNREACHABLE();
            }
         }
         break;
      case GFX_LFMT_TYPE_FLOAT:
         assert (channel_size == 16);
         assert(n_channels == 4);
         v[0] = glsl_dataflow_construct_binary_op(DATAFLOW_FPACK, data[0], data[1]);
         v[1] = glsl_dataflow_construct_binary_op(DATAFLOW_FPACK, data[2], data[3]);
         break;
      case GFX_LFMT_TYPE_UNORM:
         assert(channel_size == 8);
         assert(n_channels == 4);
         v[0] = pack_unorm_4x8(data);
         break;
      case GFX_LFMT_TYPE_SNORM:
         assert(channel_size == 8);
         assert(n_channels == 4);
         v[0] = pack_snorm_4x8(data);
         break;
      default:
         UNREACHABLE();
      }
   }

   return glsl_dataflow_construct_vec4(v[0], v[1], v[2], v[3]);
}

static void get_x_y_z_elem_no(const PrimSamplerInfo *sampler_info, Dataflow *coord[3],
      Dataflow **x, Dataflow **y, Dataflow **z, Dataflow **elem_no)
{
   //coords from int to uint
   for (unsigned i = 0; i < 3 && coord[i]; ++i)
      coord[i] = glsl_dataflow_construct_reinterp(coord[i], DF_UINT);
   assert(sampler_info->size_dim >= 2);
   *x = coord[0];
   *y = coord[1];
   if (sampler_info->array || sampler_info->cube) {
      *elem_no = coord[2];
      *z = NULL;
   } else if (sampler_info->coord_count > 2) {
      *z = coord[2];
      *elem_no = NULL;
   } else {
      *z = NULL;
      *elem_no = NULL;
   }
}


static Dataflow *calculate_store_cond(Dataflow *sampler, Dataflow *x, Dataflow *y, Dataflow *z)
{
   Dataflow *width = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_LX_WIDTH);
   Dataflow *height = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_LX_HEIGHT);

   assert(x != NULL && y!= NULL);

   /* expr =  x < width && y < height;
      if (z)
         expr = expr && (z < depth);
   */
   Dataflow *expr = glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN, x, width);
   Dataflow *y_le = glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN, y, height);
   expr = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, expr, y_le);

   if (z)
   {
      Dataflow *depth = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_LX_DEPTH);
      Dataflow *z_le = glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN, z, depth);
      expr = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, expr, z_le);
   }

   return expr;
}

void glsl_calculate_dataflow_image_store(BasicBlock *ctx, Expr *expr)
{
   assert(expr->u.intrinsic.args->count == 3);
   ExprChain *args = expr->u.intrinsic.args;
   Expr *expr_sampler = args->first->expr;
   Expr *expr_coord =   args->first->next->expr;
   Expr *expr_data =    args->first->next->next->expr;

   Dataflow *sampler;
   Dataflow *coord[3] = { NULL, };
   Dataflow *data[4] = { NULL };

   glsl_expr_calculate_dataflow(ctx, &sampler, expr_sampler);
   glsl_expr_calculate_dataflow(ctx, coord, expr_coord);
   glsl_expr_calculate_dataflow(ctx, data, expr_data);

   PrimSamplerInfo *sampler_info = glsl_prim_get_image_info(expr_sampler->type->u.primitive_type.index);

   Dataflow *x , *y, *z, *elem_no;
   get_x_y_z_elem_no(sampler_info, coord, &x, &y, &z, & elem_no);

   //if coordinates are outside image size --> don't do any stores
   Dataflow *cond = calculate_store_cond(sampler, x, y, z ? z : elem_no);

   assert(sampler->u.load.fmt_valid);
   GFX_LFMT_T fmt = fmt_qualifier_to_fmt(sampler->u.load.fmt);
   Dataflow *addr = construct_dataflow_img_addr(sampler, fmt, x, y, z, elem_no);

   //pack data from gvec4 to the destination format;
   Dataflow *values = construct_dataflow_pack_data(fmt, data);

   Dataflow *s = glsl_dataflow_construct_atomic(DATAFLOW_ADDRESS_STORE, DF_VOID,
         addr, values, cond, ctx->memory_head);
   ctx->memory_head = s;
}

void glsl_calculate_dataflow_image_size(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   Expr *expr_sampler = expr->u.intrinsic.args->first->expr;

   Dataflow *sampler;
   glsl_expr_calculate_dataflow(ctx, &sampler, expr_sampler);

   assert(expr->type->scalar_count >= 2);
   scalar_values[0] = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_LX_WIDTH);
   scalar_values[1] = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_LX_HEIGHT);
   if (expr->type->scalar_count == 3)
      scalar_values[2] = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_LX_DEPTH);
   // uint -> int
   for (unsigned i = 0; i < expr->type->scalar_count; ++i)
      scalar_values[i] = glsl_dataflow_construct_reinterp(scalar_values[i], DF_INT);
}
