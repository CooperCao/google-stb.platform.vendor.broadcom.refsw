/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_dataflow_image.h"
#include "glsl_dataflow_builder.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_tex_params.h"
#include "glsl_imageunit_swizzling.h"
#include "dflib.h"
#include "libs/core/gfx_buffer/gfx_buffer_uif_config.h"
#include "libs/core/lfmt/lfmt.h"
#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "libs/khrn/glxx/glxx_int_config.h"

static GFX_LFMT_T fmt_qualifier_to_fmt(FormatQualifier fmt)
{
   switch (fmt)
   {
   case FMT_RGBA32F:      return GFX_LFMT_R32_G32_B32_A32_FLOAT;
   case FMT_RGBA16F:      return GFX_LFMT_R16_G16_B16_A16_FLOAT;
   case FMT_R32F:         return GFX_LFMT_R32_FLOAT;

   case FMT_RGBA8:        return GFX_LFMT_R8_G8_B8_A8_UNORM;
   case FMT_RGBA8_SNORM:  return GFX_LFMT_R8_G8_B8_A8_SNORM;
   /* Extended formats */
   case FMT_RG32F:        return GFX_LFMT_R32_G32_FLOAT;
   case FMT_RG16F:        return GFX_LFMT_R16_G16_FLOAT;
   case FMT_R11G11B10F:   return GFX_LFMT_R11G11B10_UFLOAT;
   case FMT_R16F:         return GFX_LFMT_R16_FLOAT;
   case FMT_RGBA16:       return GFX_LFMT_R16_G16_B16_A16_UNORM;
   case FMT_RGB10A2:      return GFX_LFMT_R10G10B10A2_UNORM;
   case FMT_RG16:         return GFX_LFMT_R16_G16_UNORM;
   case FMT_RG8:          return GFX_LFMT_R8_G8_UNORM;
   case FMT_R16:          return GFX_LFMT_R16_UNORM;
   case FMT_R8:           return GFX_LFMT_R8_UNORM;
   case FMT_RGBA16_SNORM: return GFX_LFMT_R16_G16_B16_A16_SNORM;
   case FMT_RG16_SNORM:   return GFX_LFMT_R16_G16_SNORM;
   case FMT_RG8_SNORM:    return GFX_LFMT_R8_G8_SNORM;
   case FMT_R16_SNORM:    return GFX_LFMT_R16_SNORM;
   case FMT_R8_SNORM:     return GFX_LFMT_R8_SNORM;
   /* End of extended formats */

   case FMT_RGBA32I:      return GFX_LFMT_R32_G32_B32_A32_INT;
   case FMT_RGBA16I:      return GFX_LFMT_R16_G16_B16_A16_INT;
   case FMT_RGBA8I:       return GFX_LFMT_R8_G8_B8_A8_INT;
   case FMT_R32I:         return GFX_LFMT_R32_INT;
   /* Extended formats */
   case FMT_RG32I:        return GFX_LFMT_R32_G32_INT;
   case FMT_RG16I:        return GFX_LFMT_R16_G16_INT;
   case FMT_RG8I:         return GFX_LFMT_R8_G8_INT;
   case FMT_R16I:         return GFX_LFMT_R16_INT;
   case FMT_R8I:          return GFX_LFMT_R8_INT;
   /* End of extended formats */
   case FMT_RGBA32UI:     return GFX_LFMT_R32_G32_B32_A32_UINT;
   case FMT_RGBA16UI:     return GFX_LFMT_R16_G16_B16_A16_UINT;
   case FMT_RGBA8UI:      return GFX_LFMT_R8_G8_B8_A8_UINT;
   case FMT_R32UI:        return GFX_LFMT_R32_UINT;
   /* Extended formats */
   case FMT_RGB10A2UI:    return GFX_LFMT_R10G10B10A2_UINT;
   case FMT_RG32UI:       return GFX_LFMT_R32_G32_UINT;
   case FMT_RG16UI:       return GFX_LFMT_R16_G16_UINT;
   case FMT_RG8UI:        return GFX_LFMT_R8_G8_UINT;
   case FMT_R16UI:        return GFX_LFMT_R16_UINT;
   case FMT_R8UI:         return GFX_LFMT_R8_UINT;
   /* End of extended formats */
   default:
      unreachable();
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

#if !V3D_VER_AT_LEAST(4,1,34,0)

typedef struct {
   Dataflow *lx_swizzling;
   Dataflow *lx_xor_addr;
   Dataflow *lx_pitch;
   Dataflow *lx_slice_pitch;
} df_img_info;

typedef struct {
   Dataflow *bytes_per_texel;
   Dataflow *ut_w;
   Dataflow *ut_h;
} df_base_detail;

typedef struct {
   Dataflow *uif_col_w_in_ub;
   Dataflow *uif_ub_size;
   Dataflow *utile_size;
} df_ub_const;

static Dataflow *get_y_or_xor_y_in_ub(Dataflow *x_in_uif_col, Dataflow *xor_addr,
      Dataflow *y_in_ub)
{
   //odd_col = (x_in_uif_col) % 2;
   Dataflow *odd_col = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, x_in_uif_col,
         glsl_dataflow_construct_const_uint(1));
   odd_col = glsl_dataflow_convert_type(odd_col, DF_BOOL);

   Dataflow *y_xor_in_ub = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_XOR, y_in_ub, xor_addr);

   // (odd_col) ? y_xor_in_ub : y_in_ub
   return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, odd_col, y_xor_in_ub, y_in_ub);
}

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
                     get_y_or_xor_y_in_ub(x_in_uif_col, img->lx_xor_addr, y_in_ub),
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

   //ub_offset = img->lx_swizzling == UBLINEAR ? get_ublinear_ub_offset : get_uif_ub_offset
   Dataflow *cond = glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, img->lx_swizzling,
                           glsl_dataflow_construct_const_uint(GLSL_IMGUNIT_SWIZZLING_UBLINEAR));
   Dataflow *ub_offset = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond,
                             get_ublinear_ub_offset(df_bd, img, ub_const, ub_h, x_in_ub, y_in_ub),
                             get_uif_ub_offset(df_bd, img, ub_const, ub_w, x_in_ub, y_in_ub));

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
   img.lx_xor_addr = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_XOR_ADDR);
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

   // if (uif || uif_xor || ubl) uif_ubl_offset else lt_ut_offset
   Dataflow *uif_ubl_offset = get_uif_ubl_offset(&df_bd, &img, &ub_const, x, y, z, x_in_utiles, y_in_utiles);
   Dataflow *lt_ut_offset = get_lt_ut_offset(&df_bd, &img, &ub_const, x_in_utiles, y_in_utiles);
   Dataflow *cond = glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN, img.lx_swizzling, glsl_dataflow_construct_const_uint(GLSL_IMGUNIT_SWIZZLING_LT));
   Dataflow *offset = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond, uif_ubl_offset, lt_ut_offset);

   //offset += offset_inside_utile
   offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset, get_offset_inside_utile(x, y, &df_bd));

   //offset +=  z * img_slice_pitch;
   if (z != NULL) {
      offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset,
                  glsl_dataflow_construct_binary_op(DATAFLOW_MUL, z,img.lx_slice_pitch));
   }

   // offset += elem_no * arr_stride
   if (elem_no) {
      Dataflow *arr_stride = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_ARR_STRIDE);
      offset = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, offset,
            glsl_dataflow_construct_binary_op(DATAFLOW_MUL, elem_no, arr_stride));
   }

   Dataflow *addr = glsl_dataflow_construct_image_info_param(sampler, IMAGE_INFO_LX_ADDR);
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, addr, offset);
}

static Dataflow *calculate_store_cond(const PrimSamplerInfo *image_info,
      Dataflow *sampler, Dataflow *x, Dataflow *y, Dataflow *z)
{
   Dataflow *size = glsl_dataflow_construct_texture_size(sampler);

   assert(x != NULL);

   /* expr =  x < width;
    * if (y)
    *    expr = expr && (y < height);
    *  if (z)
    *     expr = expr && (z < depth);
    */
   Dataflow *width = glsl_dataflow_construct_reinterp(glsl_dataflow_construct_get_vec4_component(0, size, DF_INT), DF_UINT);
   Dataflow *expr = glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN, x, width);
   if (y)
   {
      Dataflow *height = glsl_dataflow_construct_reinterp(glsl_dataflow_construct_get_vec4_component(1, size, DF_INT), DF_UINT);
      Dataflow *y_le = glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN, y, height);
      expr = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, expr, y_le);
   }

   if (z)
   {
      Dataflow *depth;
      if (image_info->cube)
      {
         assert(!image_info->array); //we should not have cubemap arrays in this version
         depth = glsl_dataflow_construct_const_uint(6);
      }
      else
         depth = glsl_dataflow_construct_reinterp(glsl_dataflow_construct_get_vec4_component(2, size, DF_INT), DF_UINT);
      Dataflow *z_le = glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN, z, depth);
      expr = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, expr, z_le);
   }

   return expr;
}
#endif

static void get_x_y_z_elem_no(const PrimSamplerInfo *image_info, Dataflow *coord[3],
      Dataflow **x, Dataflow **y, Dataflow **z, Dataflow **elem_no)
{
   //coords from int to uint
   for (unsigned i = 0; i < 3 && coord[i]; ++i)
      coord[i] = glsl_dataflow_construct_reinterp(coord[i], DF_UINT);

   *y = *z = *elem_no = NULL;
   unsigned pos = 0;

   assert(image_info->size_dim >= 1);
   *x = coord[pos++];
   if (image_info->size_dim > 1)
      *y = coord[pos++];
   if (image_info->array || image_info->cube)
      *elem_no = coord[pos++];
   else if (image_info->coord_count > 2)
      *z = coord[2];
}

static DataflowFlavour df_atomic_from_intrinsic(glsl_intrinsic_index_t f) {
   switch(f) {
      case INTRINSIC_IMAGE_ADD:     return DATAFLOW_ATOMIC_ADD;
      case INTRINSIC_IMAGE_MIN:     return DATAFLOW_ATOMIC_MIN;
      case INTRINSIC_IMAGE_MAX:     return DATAFLOW_ATOMIC_MAX;
      case INTRINSIC_IMAGE_AND:     return DATAFLOW_ATOMIC_AND;
      case INTRINSIC_IMAGE_OR:      return DATAFLOW_ATOMIC_OR;
      case INTRINSIC_IMAGE_XOR:     return DATAFLOW_ATOMIC_XOR;
      case INTRINSIC_IMAGE_XCHG:    return DATAFLOW_ATOMIC_XCHG;
      case INTRINSIC_IMAGE_CMPXCHG: return DATAFLOW_ATOMIC_CMPXCHG;
      case INTRINSIC_IMAGE_STORE:   return DATAFLOW_ADDRESS_STORE;
      default: unreachable(); return 0;
   }
}

void glsl_calculate_dataflow_image_atomic(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr)
{
   ExprChain *args = expr->u.intrinsic.args;
   Expr *expr_sampler = args->first->expr;
   Expr *expr_coord   = args->first->next->expr;
   Expr *expr_data    = args->first->next->next->expr;

   Dataflow *sampler;
   Dataflow *coord[3] = { NULL, };
   Dataflow *data[4] = { NULL };
   Dataflow *cmp = NULL;

   glsl_expr_calculate_dataflow(ctx, &sampler, expr_sampler);
   glsl_expr_calculate_dataflow(ctx, coord, expr_coord);
   glsl_expr_calculate_dataflow(ctx, data, expr_data);
   if (expr->u.intrinsic.args->count == 4)
      glsl_expr_calculate_dataflow(ctx, &cmp, args->first->next->next->next->expr);

   PrimSamplerInfo *image_info = glsl_prim_get_image_info(expr_sampler->type->u.primitive_type.index);

   Dataflow *x , *y, *z, *elem_no;
   get_x_y_z_elem_no(image_info, coord, &x, &y, &z, & elem_no);

   assert(sampler->u.load.fmt_valid);

   Dataflow *values;
   if (expr->u.intrinsic.flavour == INTRINSIC_IMAGE_STORE)
      values = dflib_pack_format(sampler->u.load.fmt, data);
   else
      values = glsl_dataflow_construct_vec4(data[0], cmp, NULL, NULL);

#if V3D_VER_AT_LEAST(4,1,34,0)
   /* We use border wrap mode, which will cause the TMU to skip writes which
    * are outside of the image. So no need for an explicit condition. */
   Dataflow *cond = NULL;
   Dataflow *addr = glsl_dataflow_construct_texture_addr(sampler, x, y, z, elem_no);
#else
   //if coordinates are outside image size --> don't do any stores;
   Dataflow *cond = calculate_store_cond(image_info, sampler, x, y, z ? z : elem_no);
   GFX_LFMT_T fmt = fmt_qualifier_to_fmt(sampler->u.load.fmt);
   /* Image buffer aren't supported on v3.3, so this must be a normal image */
   Dataflow *addr = construct_dataflow_img_addr(sampler, fmt, x, y, z, elem_no);
#endif

   DataflowType t = expr->u.intrinsic.flavour == INTRINSIC_IMAGE_STORE ? DF_VOID :
                        glsl_prim_index_to_df_type(expr_data->type->u.primitive_type.index);
   DataflowFlavour f = df_atomic_from_intrinsic(expr->u.intrinsic.flavour);
   Dataflow *s = glsl_dataflow_construct_atomic(f, t, addr, values, cond, ctx->memory_head);
   ctx->memory_head = s;

   if (expr->u.intrinsic.flavour != INTRINSIC_IMAGE_STORE) scalar_values[0] = s;
}
