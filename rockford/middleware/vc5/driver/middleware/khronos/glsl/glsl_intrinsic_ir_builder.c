/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_dataflow_builder.h"
#include "glsl_errors.h"
#include "glsl_globals.h"
#include "glsl_stringbuilder.h"
#include "glsl_intrinsic_ir_builder.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_stackmem.h"
#include "glsl_tex_params.h"

struct intrinsic_ir_info_s {
   glsl_intrinsic_index_t  intrinsic;
   DataflowFlavour         dataflow_flavour;
   int                     args;
};

static const struct intrinsic_ir_info_s intrinsic_ir_info[INTRINSIC_COUNT] = {
   { INTRINSIC_TEXTURE,       DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_TEXTURE_SIZE,  DATAFLOW_FLAVOUR_COUNT, -1 },
   { INTRINSIC_RSQRT,         DATAFLOW_RSQRT,          1 },
   { INTRINSIC_RCP,           DATAFLOW_RCP,            1 },
   { INTRINSIC_LOG2,          DATAFLOW_LOG2,           1 },
   { INTRINSIC_EXP2,          DATAFLOW_EXP2,           1 },
   { INTRINSIC_CEIL,          DATAFLOW_CEIL,           1 },
   { INTRINSIC_FLOOR,         DATAFLOW_FLOOR,          1 },
   { INTRINSIC_TRUNC,         DATAFLOW_TRUNC,          1 },
   { INTRINSIC_NEAREST,       DATAFLOW_NEAREST,        1 },
   { INTRINSIC_MIN,           DATAFLOW_MIN,            2 },
   { INTRINSIC_MAX,           DATAFLOW_MAX,            2 },
   { INTRINSIC_ABS,           DATAFLOW_ABS,            1 },
   { INTRINSIC_FDX,           DATAFLOW_FDX,            1 },
   { INTRINSIC_FDY,           DATAFLOW_FDY,            1 },
   { INTRINSIC_CLZ,           DATAFLOW_CLZ,            1 },
   { INTRINSIC_REINTERPF,     DATAFLOW_REINTERP,       1 },
   { INTRINSIC_REINTERPI,     DATAFLOW_REINTERP,       1 },
   { INTRINSIC_REINTERPU,     DATAFLOW_REINTERP,       1 },
   { INTRINSIC_FPACK,         DATAFLOW_FPACK,          2 },
   { INTRINSIC_FUNPACKA,      DATAFLOW_FUNPACKA,       1 },
   { INTRINSIC_FUNPACKB,      DATAFLOW_FUNPACKB,       1 },
   { INTRINSIC_SIN,           DATAFLOW_SIN,            1 },
   { INTRINSIC_COS,           DATAFLOW_COS,            1 },
   { INTRINSIC_TAN,           DATAFLOW_TAN,            1 },
};

static void calculate_dataflow_texture_lookup(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr) {
   Dataflow *sampler_scalar_value;
   Dataflow *coord_scalar_values [4];
   Dataflow *lod_scalar_values   [3];
   Dataflow *offset_scalar_values[3];
   Dataflow *comp_scalar_value;
   PrimSamplerInfo *sampler_info;

   ExprChain *args = expr->u.intrinsic.args;
   Expr *bits      = args->first->expr;
   Expr *sampler   = args->first->next->expr;
   Expr *coord     = args->first->next->next->expr;
   Expr *lod       = args->first->next->next->next->expr;
   Expr *offset    = NULL, *comp = NULL;

   uint32_t bits_value = *(uint32_t  *)bits->compile_time_value;
   if (bits_value & GLSL_TEXBITS_GATHER) {
      if (args->count >= 5) {
         comp = args->first->next->next->next->next->expr;
      }
      if (args->count == 6) {
         offset = args->first->next->next->next->next->next->expr;
      }
   } else {
      if (args->count == 5) {
         offset = args->first->next->next->next->next->expr;
      }
   }


   // Calculate args.
   glsl_expr_calculate_dataflow(ctx, &sampler_scalar_value, sampler);
   glsl_expr_calculate_dataflow(ctx, coord_scalar_values,   coord);
   glsl_expr_calculate_dataflow(ctx, lod_scalar_values,     lod);
   if(offset) {
      glsl_expr_calculate_dataflow(ctx, offset_scalar_values, offset);
   }
   if (comp) {
      glsl_expr_calculate_dataflow(ctx, &comp_scalar_value, comp);
   }

   // Create gadget.
   sampler_info = glsl_prim_get_sampler_info(sampler->type->u.primitive_type.index);

   bits_value |= sampler_info->bits;

   Dataflow *gadget_c[4] = { NULL, };
   Dataflow *gadget_d = NULL;

   for (int i=0; i<sampler_info->coord_count; i++) gadget_c[i] = coord_scalar_values[i];
   if (sampler_info->array)  gadget_c[3] = coord_scalar_values[sampler_info->coord_count];
   if (sampler_info->shadow) gadget_d = coord_scalar_values[sampler_info->coord_count + (sampler_info->array ? 1 : 0)];

   Dataflow *coords = glsl_dataflow_construct_vec4(gadget_c[0], gadget_c[1], gadget_c[2], gadget_c[3]);

   /* Except in fragment shaders all non-Fetch lookups use explicit lod 0 */
   if (g_ShaderFlavour != SHADER_FRAGMENT && !(bits_value & GLSL_TEXBITS_FETCH))
      bits_value |= GLSL_TEXBITS_BSLOD;

   Dataflow *gadget_b = lod_scalar_values[0];

   if (offset) {
      const int n_offsets = offset->type->scalar_count;
      int offsets[3] = {0, };

      for (int i=0; i<n_offsets; i++) {
         if (offset_scalar_values[i]->flavour != DATAFLOW_CONST)
            glsl_compile_error(ERROR_CUSTOM, 25, expr->line_num, NULL);

         offsets[i] = offset_scalar_values[i]->u.constant.value;
      }

      bits_value |= GLSL_TEXBITS_OFFSET(offsets[0], offsets[1], offsets[2]);
   }

   if (comp) {
      if (comp_scalar_value->flavour != DATAFLOW_CONST)
         glsl_compile_error(ERROR_CUSTOM, 40, expr->line_num, NULL);

      bits_value |= (comp_scalar_value->u.constant.value & 0x3) << GLSL_TEXBITS_GATHER_COMP_SHIFT;
   }

   const DataflowType component_type_index = glsl_prim_index_to_df_type(primitiveScalarTypeIndices[sampler_info->return_type]);
   const bool         scalar_result        = expr->type == &primitiveTypes[PRIM_FLOAT];
   glsl_dataflow_construct_texture_gadget(
          &scalar_values[0],
          scalar_result ? NULL : &scalar_values[1],
          scalar_result ? NULL : &scalar_values[2],
          scalar_result ? NULL : &scalar_values[3],
          bits_value, sampler_scalar_value, coords, gadget_d, gadget_b, 0, component_type_index);
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
   ExprChainNode *cur;

   operand_scalar_values = glsl_stack_malloc(sizeof(*operand_scalar_values), arg_count);
   operand_scalar_count  = glsl_stack_malloc(sizeof(*operand_scalar_count),  arg_count);
   dataflow_args         = glsl_stack_malloc(sizeof(*dataflow_args),         arg_count);

   cur = expr->u.intrinsic.args->first;
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
   default:
      assert(arg_count >= 0);
      calculate_dataflow_functionlike_intrinsic(ctx, scalar_values, expr);
      break;
   }
}
