/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_dataflow_builder.h"
#include "glsl_errors.h"
#include "glsl_globals.h"
#include "glsl_stringbuilder.h"
#include "glsl_intrinsic_ir_builder.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_stackmem.h"
#include "glsl_dataflow_image.h"

struct intrinsic_ir_info_s {
   glsl_intrinsic_index_t  intrinsic;
   DataflowFlavour         dataflow_flavour;
   int                     args;
};

static const struct intrinsic_ir_info_s intrinsic_ir_info[INTRINSIC_COUNT] = {
   { INTRINSIC_TEXTURE,        DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_TEXTURE_SIZE,   DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_RSQRT,          DATAFLOW_RSQRT,          1 },
   { INTRINSIC_RCP,            DATAFLOW_RCP,            1 },
   { INTRINSIC_LOG2,           DATAFLOW_LOG2,           1 },
   { INTRINSIC_EXP2,           DATAFLOW_EXP2,           1 },
   { INTRINSIC_CEIL,           DATAFLOW_CEIL,           1 },
   { INTRINSIC_FLOOR,          DATAFLOW_FLOOR,          1 },
   { INTRINSIC_TRUNC,          DATAFLOW_TRUNC,          1 },
   { INTRINSIC_NEAREST,        DATAFLOW_NEAREST,        1 },
   { INTRINSIC_MIN,            DATAFLOW_MIN,            2 },
   { INTRINSIC_MAX,            DATAFLOW_MAX,            2 },
   { INTRINSIC_ABS,            DATAFLOW_ABS,            1 },
   { INTRINSIC_FDX,            DATAFLOW_FDX,            1 },
   { INTRINSIC_FDY,            DATAFLOW_FDY,            1 },
   { INTRINSIC_CLZ,            DATAFLOW_CLZ,            1 },
   { INTRINSIC_ROR,            DATAFLOW_ROR,            2 },
   { INTRINSIC_REINTERPF,      DATAFLOW_REINTERP,       1 },
   { INTRINSIC_REINTERPI,      DATAFLOW_REINTERP,       1 },
   { INTRINSIC_REINTERPU,      DATAFLOW_REINTERP,       1 },
   { INTRINSIC_FPACK,          DATAFLOW_FPACK,          2 },
   { INTRINSIC_FUNPACKA,       DATAFLOW_FUNPACKA,       1 },
   { INTRINSIC_FUNPACKB,       DATAFLOW_FUNPACKB,       1 },
   { INTRINSIC_SIN,            DATAFLOW_SIN,            1 },
   { INTRINSIC_COS,            DATAFLOW_COS,            1 },
   { INTRINSIC_TAN,            DATAFLOW_TAN,            1 },
   { INTRINSIC_ATOMIC_LOAD,    DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_ATOMIC_ADD,     DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_ATOMIC_SUB,     DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_ATOMIC_MIN,     DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_ATOMIC_MAX,     DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_ATOMIC_AND,     DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_ATOMIC_OR,      DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_ATOMIC_XOR,     DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_ATOMIC_XCHG,    DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_ATOMIC_CMPXCHG, DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_IMAGE_STORE,    DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_IMAGE_ADD,      DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_IMAGE_MIN,      DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_IMAGE_MAX,      DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_IMAGE_AND,      DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_IMAGE_OR,       DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_IMAGE_XOR,      DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_IMAGE_XCHG,     DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_IMAGE_CMPXCHG,  DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_IMAGE_SIZE,     DATAFLOW_FLAVOUR_COUNT, -1 },
};

static inline Dataflow *pack_const_offsets(const const_value *o) {
   return glsl_dataflow_construct_const_uint((o[0] & 15) | (o[1] & 15) << 4 | (o[2] & 15) << 8);
}

static Dataflow *pack_texture_offsets(int num, Dataflow *const *values)
{
   Dataflow *mask = glsl_dataflow_construct_const_uint(0xf);

   Dataflow *packed = NULL;
   for (int i = 0; i != num; ++i)
   {
      Dataflow *v = values[i];

      /* Mask off high bits. No need to do this for the last value. */
      if (i != (num - 1))
         v = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, v, mask);

      /* Shift into place */
      v = glsl_dataflow_construct_binary_op(DATAFLOW_SHL, v, glsl_dataflow_construct_const_uint(i * 4));

      /* Or into packed value */
      if (packed)
         packed = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_OR, packed, v);
      else
         packed = v;
   }

   return packed;
}

static bool is_imagebuffer(const PrimSamplerInfo *sampler)
{
   return (sampler->type == PRIM_IMAGEBUFFER || sampler->type == PRIM_UIMAGEBUFFER ||
         sampler->type == PRIM_IIMAGEBUFFER);
}

static bool is_samplerbuffer(const PrimSamplerInfo *sampler)
{
   return (sampler->type == PRIM_SAMPLERBUFFER || sampler->type == PRIM_USAMPLERBUFFER ||
         sampler->type == PRIM_ISAMPLERBUFFER);
}

/* translate input coord into an index in array + s coord;  */
static void texbuffer_translate_coord(Dataflow *sampler, Dataflow *coord,
      Dataflow **x, Dataflow **elem_no)
{
   Dataflow *log2_arr_elem_w = glsl_construct_texbuffer_info_param(
         sampler, TEXBUFFER_INFO_LOG2_ARR_ELEM_W);
   Dataflow *arr_elem_w_minus_1 = glsl_construct_texbuffer_info_param(
         sampler, TEXBUFFER_INFO_ARR_ELEM_W_MINUS_1);

   *elem_no = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, coord, log2_arr_elem_w);
   *x = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, coord, arr_elem_w_minus_1);
}

static void calculate_dataflow_texture_lookup(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr) {
   Dataflow *sampler_scalar_value;
   Dataflow *coord_scalar_values [4];
   Dataflow *lod_scalar;
   Dataflow *comp_scalar;
   Dataflow *dref_scalar = NULL;
   PrimSamplerInfo *sampler_info;

   ExprChain *args = expr->u.intrinsic.args;
   Expr *bits      = args->first->expr;
   Expr *sampler   = args->first->next->expr;
   Expr *coord     = args->first->next->next->expr;
   Expr *lod       = args->first->next->next->next->expr;
   Expr *offset    = NULL, *comp = NULL, *dref = NULL;

   bool is_image = glsl_prim_is_prim_image_type(sampler->type);
   if (is_image)
      sampler_info = glsl_prim_get_image_info(sampler->type->u.primitive_type.index);
   else
      sampler_info = glsl_prim_get_sampler_info(sampler->type->u.primitive_type.index);

   ExprChainNode *arg = args->first->next->next->next->next;
   if (sampler_info->shadow) {
      dref = arg->expr;
      arg = arg->next;
   }

   uint32_t bits_value = *(uint32_t  *)bits->compile_time_value;
   if ((bits_value & DF_TEXBITS_GATHER) && arg != NULL) {
      comp = arg->expr;
      arg = arg->next;
   }

   if (arg != NULL) {
      offset = arg->expr;
   }

   // Calculate args.
   glsl_expr_calculate_dataflow(ctx, &sampler_scalar_value, sampler);
   glsl_expr_calculate_dataflow(ctx, coord_scalar_values,   coord);
   glsl_expr_calculate_dataflow(ctx, &lod_scalar,           lod);
   if (comp) glsl_expr_calculate_dataflow(ctx, &comp_scalar, comp);
   if (dref) glsl_expr_calculate_dataflow(ctx, &dref_scalar, dref);

   int non_idx_coords = sampler_info->coord_count;
   bool array = sampler_info->array;
   bool cube = sampler_info->cube;
   if (is_image && cube)
   {
      /* Cube and cubemap arrays images are treated as 2D arrays... */
      assert(non_idx_coords == 3);
      non_idx_coords = 2;
      array = true;
      cube = false;
   }

   if (cube) bits_value |= DF_TEXBITS_CUBE;

   Dataflow *gadget_c[4] = { NULL, };
   for (int i=0; i<non_idx_coords; i++) gadget_c[i] = coord_scalar_values[i];
   if (array) gadget_c[3] = coord_scalar_values[non_idx_coords];

   if (is_samplerbuffer(sampler_info) || is_imagebuffer(sampler_info))
   {
      Dataflow *pos = gadget_c[0];
      if (is_image)
         glsl_imgbuffer_translate_coord(sampler_scalar_value, pos, &gadget_c[0], &gadget_c[3]);
      else
         texbuffer_translate_coord(sampler_scalar_value, pos, &gadget_c[0], &gadget_c[3]);
   }

   Dataflow *coords = glsl_dataflow_construct_vec4(gadget_c[0], gadget_c[1], gadget_c[2], gadget_c[3]);

   /* Except in fragment shaders all non-Fetch lookups use explicit lod 0 */
   if (g_ShaderFlavour != SHADER_FRAGMENT && !glsl_dataflow_tex_cfg_implies_bslod(bits_value))
      bits_value |= DF_TEXBITS_BSLOD;

   Dataflow *gadget_offset = NULL;
   if (offset) {
      /* For constant non-independent offsets we pack them separately in order to
       * support them on old hardware that didn't allow variable offsets. The constant
       * folder could do it but we shouldn't rely on optimisation for correctness */
      if (!(bits_value & DF_TEXBITS_I_OFF) && offset->compile_time_value)
         gadget_offset = pack_const_offsets(offset->compile_time_value);
      else {
         const int n_offsets = offset->type->scalar_count;
         Dataflow **offset_scalar_values = glsl_stack_malloc(sizeof(Dataflow *), n_offsets);
         glsl_expr_calculate_dataflow(ctx, offset_scalar_values, offset);
         gadget_offset = pack_texture_offsets(n_offsets, offset_scalar_values);
         glsl_stack_free(offset_scalar_values);
      }
   }

   if (comp) {
      assert(comp_scalar->flavour == DATAFLOW_CONST);
      bits_value |= (comp_scalar->u.constant.value & 0x3) << DF_TEXBITS_GATHER_COMP_SHIFT;
   }

   const DataflowType component_type_index = glsl_prim_index_to_df_type(primitiveScalarTypeIndices[sampler_info->return_type]);
   const bool         scalar_result        = expr->type == &primitiveTypes[PRIM_FLOAT];
   glsl_dataflow_construct_texture_gadget(
          &scalar_values[0],
          scalar_result ? NULL : &scalar_values[1],
          scalar_result ? NULL : &scalar_values[2],
          scalar_result ? NULL : &scalar_values[3],
          bits_value, sampler_scalar_value, coords, dref_scalar, lod_scalar, gadget_offset,
          0, component_type_index);

   assert(!is_image || !scalar_result);      /* No image load functions return scalars */

#if !V3D_VER_AT_LEAST(3,3,0,0)
   if (is_image) {
      Dataflow *ok = NULL;
      ImageInfoParam size_param[3] = { IMAGE_INFO_LX_WIDTH, IMAGE_INFO_LX_HEIGHT, IMAGE_INFO_LX_DEPTH };
      for (int i=0; i<non_idx_coords; i++) {
         Dataflow *size = glsl_dataflow_construct_image_info_param(sampler_scalar_value, size_param[i]);
         Dataflow *c_ok = glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN,
                                glsl_dataflow_construct_reinterp(coord_scalar_values[i], DF_UINT), size);

         if (ok == NULL) ok = c_ok;
         else ok = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, ok, c_ok);
      }

      Dataflow *zero = glsl_dataflow_construct_const_value(component_type_index, 0);
      for (int i=0; i<4; i++)
         scalar_values[i] = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, ok, scalar_values[i], zero);
   }
#endif
#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (is_image && array)
   {
      /* HW clamps array index. We need border behaviour.
       * Replace result with 0 if array index was out of bounds... */

      Dataflow *num_elems;

      /* we should not see cube map arrays here */
      assert( (sampler_info->cube && !sampler_info->array) ||
              (!sampler_info->cube && sampler_info->array));

      if (sampler_info->cube && !sampler_info->array)
         num_elems = glsl_dataflow_construct_const_uint(6);
      else
         num_elems = glsl_dataflow_construct_image_info_param(
            sampler_scalar_value, IMAGE_INFO_LX_DEPTH);

      Dataflow *idx_ok = glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN,
         glsl_dataflow_construct_reinterp(gadget_c[3], DF_UINT), num_elems);

      Dataflow *zero = glsl_dataflow_construct_const_value(component_type_index, 0);
      for (int i=0; i<4; i++)
         scalar_values[i] = glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, idx_ok,
            scalar_values[i], zero);
   }
#endif
}

static void calculate_dataflow_texture_size(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr) {
   /* Should have only a sampler parameter. Lod was handled already */
   assert(expr->u.intrinsic.args->count == 1);
   Expr *sampler_expr = expr->u.intrinsic.args->first->expr;

   Dataflow *sampler;
   glsl_expr_calculate_dataflow(ctx, &sampler, sampler_expr);

   Dataflow *size = glsl_dataflow_construct_texture_size(sampler);

   for (unsigned i=0; i<expr->type->scalar_count; i++) {
      scalar_values[i] = glsl_dataflow_construct_get_vec4_component(i, size, DF_INT);
   }
}

static void calculate_dataflow_atomic_load(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr) {
   assert(expr->u.intrinsic.args->count == 1);
   Expr *counter = expr->u.intrinsic.args->first->expr;

   Dataflow *addr;
   glsl_expr_calculate_dataflow(ctx, &addr, counter);
   Dataflow *data = glsl_dataflow_construct_vector_load(DF_UINT, addr);
   scalar_values[0] = glsl_dataflow_construct_get_vec4_component(0, data, DF_UINT);
}

static void calculate_dataflow_atomic_op(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr) {
   assert(expr->u.intrinsic.args->count == 2 || expr->u.intrinsic.args->count == 3);
   Expr *mem   = expr->u.intrinsic.args->first->expr;
   Expr *data0 = expr->u.intrinsic.args->first->next->expr;

   Dataflow *m, *d[2] = { NULL, NULL};
   glsl_expr_calculate_dataflow(ctx, &m, mem);
   glsl_expr_calculate_dataflow(ctx, &d[0], data0);
   if (expr->u.intrinsic.args->count == 3) {
      Expr *data1 = expr->u.intrinsic.args->first->next->next->expr;
      glsl_expr_calculate_dataflow(ctx, &d[1], data1);
   }
   Dataflow *out_data = glsl_dataflow_construct_vec4(d[0], d[1], NULL, NULL);
   /* TODO: This is a hack to work around the fact that we spuriously loaded. Don't do that */
   Dataflow *addr;
   if (m->flavour == DATAFLOW_GET_VEC4_COMPONENT) {
      assert(m->d.dependencies[0]->flavour == DATAFLOW_VECTOR_LOAD);
      addr = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, m->d.dependencies[0]->d.dependencies[0], glsl_dataflow_construct_const_uint(4*m->u.get_vec4_component.component_index));
   }
   else addr = m;

   DataflowFlavour f;
   switch (expr->u.intrinsic.flavour) {
      case INTRINSIC_ATOMIC_ADD:     f = DATAFLOW_ATOMIC_ADD;     break;
      case INTRINSIC_ATOMIC_SUB:     f = DATAFLOW_ATOMIC_SUB;     break;
      case INTRINSIC_ATOMIC_MIN:     f = DATAFLOW_ATOMIC_MIN;     break;
      case INTRINSIC_ATOMIC_MAX:     f = DATAFLOW_ATOMIC_MAX;     break;
      case INTRINSIC_ATOMIC_AND:     f = DATAFLOW_ATOMIC_AND;     break;
      case INTRINSIC_ATOMIC_OR:      f = DATAFLOW_ATOMIC_OR;      break;
      case INTRINSIC_ATOMIC_XOR:     f = DATAFLOW_ATOMIC_XOR;     break;
      case INTRINSIC_ATOMIC_XCHG:    f = DATAFLOW_ATOMIC_XCHG;    break;
      case INTRINSIC_ATOMIC_CMPXCHG: f = DATAFLOW_ATOMIC_CMPXCHG; break;
      default: unreachable(); f = 0; break;
   }
   assert(expr->type == &primitiveTypes[PRIM_UINT] || expr->type == &primitiveTypes[PRIM_INT]);
   DataflowType t;
   if (expr->type == &primitiveTypes[PRIM_UINT]) t = DF_UINT;
   else t = DF_INT;

   scalar_values[0] = glsl_dataflow_construct_atomic(f, t, addr, out_data, NULL, ctx->memory_head);
   ctx->memory_head = scalar_values[0];
}

static DataflowType intrinsic_df_return_type(glsl_intrinsic_index_t intrinsic,
                                             Dataflow ***operands,
                                             int index)
{
   switch(intrinsic) {
   case INTRINSIC_FUNPACKA:
   case INTRINSIC_FUNPACKB:
   case INTRINSIC_REINTERPF:
      return DF_FLOAT;
   case INTRINSIC_FPACK:
   case INTRINSIC_REINTERPU:
      return DF_UINT;
   case INTRINSIC_REINTERPI:
      return DF_INT;
   default:
      return operands[0][index]->type;
   }
}

static void calculate_dataflow_functionlike_intrinsic(BasicBlock* ctx, Dataflow **scalar_values, Expr *expr) {
   const glsl_intrinsic_index_t intrinsic   = expr->u.intrinsic.flavour;
   const int                    arg_count   = intrinsic_ir_info[intrinsic].args;
   const DataflowFlavour        flavour     = intrinsic_ir_info[intrinsic].dataflow_flavour;
   Dataflow ***operand_scalar_values;
   int        *operand_scalar_count;
   Dataflow  **dataflow_args;

   operand_scalar_values = glsl_stack_malloc(sizeof(*operand_scalar_values), arg_count);
   operand_scalar_count  = glsl_stack_malloc(sizeof(*operand_scalar_count),  arg_count);
   dataflow_args         = glsl_stack_malloc(sizeof(*dataflow_args),         arg_count);

   ExprChainNode *cur = expr->u.intrinsic.args->first;
   for(int i=0;i<arg_count;++i) {
      Expr *operand = cur->expr;
      cur = cur->next;
      operand_scalar_values[i] = glsl_stack_malloc(sizeof(*operand_scalar_values[i]), operand->type->scalar_count);
      operand_scalar_count [i] = operand->type->scalar_count;
      glsl_expr_calculate_dataflow(ctx, operand_scalar_values[i], operand);
   }

   for(unsigned i=0;i<expr->type->scalar_count;++i) {
      for(int j=0;j<arg_count;++j) {
         const int operand_scalar_index = i % operand_scalar_count[j];
         dataflow_args[j] = operand_scalar_values[j][operand_scalar_index];
      }

      if(flavour == DATAFLOW_REINTERP) {
         scalar_values[i] = glsl_dataflow_construct_reinterp(dataflow_args[0],
                                                             intrinsic_df_return_type(intrinsic,operand_scalar_values,i));
      } else {
         scalar_values[i] = glsl_dataflow_construct_op(flavour, arg_count, dataflow_args);
      }
   }

   for(int i=arg_count-1;i>=0;--i) {
      glsl_stack_free(operand_scalar_values[i]);
   }

   glsl_stack_free(dataflow_args);
   glsl_stack_free(operand_scalar_count);
   glsl_stack_free(operand_scalar_values);
}

void glsl_intrinsic_ir_calculate_dataflow(BasicBlock* ctx, Dataflow **scalar_values, Expr *expr) {
   const glsl_intrinsic_index_t intrinsic = expr->u.intrinsic.flavour;
   const int                    arg_count = intrinsic_ir_info[intrinsic].args;

   assert(expr->flavour == EXPR_INTRINSIC && intrinsic >= 0 && intrinsic < INTRINSIC_COUNT);

   switch(intrinsic) {
   case INTRINSIC_TEXTURE:
      calculate_dataflow_texture_lookup(ctx, scalar_values, expr);
      break;
   case INTRINSIC_TEXTURE_SIZE:
      calculate_dataflow_texture_size(ctx, scalar_values, expr);
      break;
   case INTRINSIC_ATOMIC_LOAD:
      calculate_dataflow_atomic_load(ctx, scalar_values, expr);
      break;
   case INTRINSIC_ATOMIC_ADD:
   case INTRINSIC_ATOMIC_SUB:
   case INTRINSIC_ATOMIC_MIN:
   case INTRINSIC_ATOMIC_MAX:
   case INTRINSIC_ATOMIC_AND:
   case INTRINSIC_ATOMIC_OR:
   case INTRINSIC_ATOMIC_XOR:
   case INTRINSIC_ATOMIC_XCHG:
   case INTRINSIC_ATOMIC_CMPXCHG:
      calculate_dataflow_atomic_op(ctx, scalar_values, expr);
      break;
   case INTRINSIC_IMAGE_ADD:
   case INTRINSIC_IMAGE_MIN:
   case INTRINSIC_IMAGE_MAX:
   case INTRINSIC_IMAGE_AND:
   case INTRINSIC_IMAGE_OR:
   case INTRINSIC_IMAGE_XOR:
   case INTRINSIC_IMAGE_XCHG:
   case INTRINSIC_IMAGE_CMPXCHG:
   case INTRINSIC_IMAGE_STORE:
      glsl_calculate_dataflow_image_atomic(ctx, scalar_values, expr);
      break;
   case INTRINSIC_IMAGE_SIZE:
      glsl_calculate_dataflow_image_size(ctx, scalar_values, expr);
      break;
   default:
      assert(arg_count >= 0);
      calculate_dataflow_functionlike_intrinsic(ctx, scalar_values, expr);
      break;
   }
}
