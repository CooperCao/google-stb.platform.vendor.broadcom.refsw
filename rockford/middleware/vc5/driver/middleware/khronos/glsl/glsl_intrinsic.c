/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_dataflow_builder.h"
#include "glsl_globals.h"
#include "glsl_intrinsic.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_tex_params.h"

struct intrinsic_info_s {
   glsl_intrinsic_index_t  intrinsic;
   const char             *name;
};

static const struct intrinsic_info_s intrinsic_info[INTRINSIC_COUNT] = {
   { INTRINSIC_TEXTURE,       "texture",      },
   { INTRINSIC_TEXTURE_SIZE,  "textureSize",  },
   { INTRINSIC_RSQRT,         "rsqrt",        },
   { INTRINSIC_RCP,           "rcp",          },
   { INTRINSIC_LOG2,          "log2",         },
   { INTRINSIC_EXP2,          "exp2",         },
   { INTRINSIC_CEIL,          "ceil",         },
   { INTRINSIC_FLOOR,         "floor",        },
   { INTRINSIC_TRUNC,         "trunc",        },
   { INTRINSIC_NEAREST,       "nearest",      },
   { INTRINSIC_MIN,           "min",          },
   { INTRINSIC_MAX,           "max",          },
   { INTRINSIC_ABS,           "abs",          },
   { INTRINSIC_FDX,           "fdx",          },
   { INTRINSIC_FDY,           "fdy",          },
   { INTRINSIC_CLZ,           "clz",          },
   { INTRINSIC_REINTERPF,     "reinterpf",    },
   { INTRINSIC_REINTERPI,     "reinterpi",    },
   { INTRINSIC_REINTERPU,     "reinterpu",    },
   { INTRINSIC_FPACK,         "fpack",        },
   { INTRINSIC_FUNPACKA,      "funpacka",     },
   { INTRINSIC_FUNPACKB,      "funpackb",     },
   { INTRINSIC_SIN,           "sin",          },
   { INTRINSIC_COS,           "cos",          },
   { INTRINSIC_TAN,           "tan",          },
};

#ifndef NDEBUG
static unsigned count_sampler_derivs(const PrimSamplerInfo *sampler_info) {
   return sampler_info->coord_count;
}

static unsigned count_sampler_coords(const PrimSamplerInfo *sampler_info) {
   return sampler_info->coord_count + (sampler_info->array ? 1 : 0) + (sampler_info->shadow ? 1 : 0);
}

static void validate_texture_lookup(ExprChain *args) {
   assert(args->count == 4 || args->count == 5 || args->count == 6);

   Expr *bits    = args->first->expr;
   Expr *sampler = args->first->next->expr;
   Expr *coord   = args->first->next->next->expr;
   Expr *lod     = args->first->next->next->next->expr;
   Expr *offset  = NULL, *comp = NULL;

   assert(bits->type->flavour                == SYMBOL_PRIMITIVE_TYPE &&
          bits->type->u.primitive_type.index == PRIM_INT &&
          bits->compile_time_value           != NULL);

   uint32_t texture_bits = *(const_value *)bits->compile_time_value;
   if (texture_bits & GLSL_TEXBITS_GATHER) {
      if (args->count >= 5) comp   = args->first->next->next->next->next->expr;
      if (args->count == 6) offset = args->first->next->next->next->next->next->expr;
   } else {
      if (args->count == 5) offset = args->first->next->next->next->next->expr;
   }

   PrimSamplerInfo *sampler_info = glsl_prim_get_sampler_info(sampler->type->u.primitive_type.index);

   if (texture_bits & GLSL_TEXBITS_FETCH) {
      assert(glsl_prim_is_vector_type(coord->type, PRIM_INT,   count_sampler_coords(sampler_info)));
      assert(lod->type == &primitiveTypes[PRIM_INT]);
   } else {
      assert(glsl_prim_is_vector_type(coord->type, PRIM_FLOAT, count_sampler_coords(sampler_info)));
      assert(lod->type == &primitiveTypes[PRIM_FLOAT]);
   }

   if(offset) {
      assert(count_sampler_derivs(sampler_info) > 0 &&
             count_sampler_derivs(sampler_info) < 4);
      assert(glsl_prim_is_vector_type(offset->type, PRIM_INT, count_sampler_derivs(sampler_info)));

      /* Offset must be constant, but we check that at a later      *
       * stage because here we'll see it as a function parameter.   */
   }

   if (comp) {
      assert(texture_bits & GLSL_TEXBITS_GATHER);
      assert(comp->type == &primitiveTypes[PRIM_INT]);
   }
}

/* Can this be simplified into a table? */
static void validate_intrinsic(glsl_intrinsic_index_t intrinsic, ExprChain *args) {
   assert(intrinsic_info[intrinsic].intrinsic == intrinsic);
   switch (intrinsic) {
   case INTRINSIC_TEXTURE:
      validate_texture_lookup(args);
      break;

   case INTRINSIC_TEXTURE_SIZE:
      /* (ivec2|ivec3) <- intrinsic(genSamplerType) */
      assert(args->count == 1);
      assert(glsl_prim_is_prim_sampler_type(args->first->expr->type));
      break;

   case INTRINSIC_RSQRT:
   case INTRINSIC_RCP:
   case INTRINSIC_LOG2:
   case INTRINSIC_EXP2:
   case INTRINSIC_TRUNC:
   case INTRINSIC_NEAREST:
   case INTRINSIC_CEIL:
   case INTRINSIC_FLOOR:
   case INTRINSIC_FDX:
   case INTRINSIC_FDY:
   case INTRINSIC_SIN:
   case INTRINSIC_COS:
   case INTRINSIC_TAN:
      /* genFtype <- intrinsic(genFtype) */
      assert(args->count == 1);
      assert(glsl_prim_is_prim_with_base_type(args->first->expr->type, PRIM_FLOAT));
      break;

   case INTRINSIC_REINTERPF:
      /* genFtype <- intrinsic(genType) */
      assert(args->count == 1);
      assert(glsl_prim_is_prim_with_same_shape_type(args->first->expr->type, PRIM_FLOAT));
      break;

   case INTRINSIC_REINTERPU:
      /* genUtype <- intrinsic(genType) */
      assert(args->count == 1);
      assert(glsl_prim_is_prim_with_same_shape_type(args->first->expr->type, PRIM_UINT));
      break;

   case INTRINSIC_REINTERPI:
      /* genItype <- intrinsic(genType) */
      assert(args->count == 1);
      assert(glsl_prim_is_prim_with_same_shape_type(args->first->expr->type, PRIM_INT));
      break;

   case INTRINSIC_MIN:
   case INTRINSIC_MAX:
      /* genAType <- intrinsic(genAType,genAType); */
      assert(args->count == 2);
      assert(glsl_prim_is_prim_with_base_type(args->first->expr->type, PRIM_FLOAT) ||
                  glsl_prim_is_prim_with_base_type(args->first->expr->type, PRIM_INT) ||
                  glsl_prim_is_prim_with_base_type(args->first->expr->type, PRIM_UINT));
      assert(args->first->expr->type == args->first->next->expr->type ||
                  glsl_prim_is_base_of_prim_type(args->first->expr->type,       args->first->next->expr->type) ||
                  glsl_prim_is_base_of_prim_type(args->first->next->expr->type, args->first->expr->type));
      break;

   case INTRINSIC_FUNPACKA:
   case INTRINSIC_FUNPACKB:
      /* genFtype <- intrinsic(genUtype); */
      assert(args->count == 1);
      assert(glsl_prim_is_prim_with_base_type      (args->first->expr->type, PRIM_UINT));
      assert(glsl_prim_is_prim_with_same_shape_type(args->first->expr->type, PRIM_FLOAT));
      break;

   case INTRINSIC_FPACK:
      /* genUtype <- intrinsic(genFtype, genFtype); */
      assert(args->count == 2);
      assert(glsl_prim_is_prim_with_base_type(args->first->expr->type, PRIM_FLOAT));
      assert(glsl_prim_is_prim_with_same_shape_type(args->first->expr->type,       PRIM_UINT) &&
                  glsl_prim_is_prim_with_same_shape_type(args->first->next->expr->type, PRIM_UINT));
      assert(args->first->expr->type == args->first->next->expr->type ||
                  glsl_prim_is_base_of_prim_type(args->first->expr->type,       args->first->next->expr->type) ||
                  glsl_prim_is_base_of_prim_type(args->first->next->expr->type, args->first->expr->type));
      break;

   case INTRINSIC_ABS:
      /* genFtype <- intrinsic(genFtype); */
      assert(args->count == 1);
      assert(glsl_prim_is_prim_with_base_type(args->first->expr->type, PRIM_FLOAT));
      break;

   case INTRINSIC_CLZ:
      /* genUtype <- intrinsic(genUType); */
      assert(args->count == 1);
      assert(glsl_prim_is_prim_with_base_type(args->first->expr->type, PRIM_UINT));
      break;

   default:
      UNREACHABLE();
      break;
   }
}
#endif

static SymbolType *calculate_functionlike_intrinsic_return_type(glsl_intrinsic_index_t intrinsic, ExprChain *args) {
   SymbolType *defining_arg_type = NULL;

   /* The largest number of scalars is the return type, e.g. max(vec4, float) returns vec4 */
   unsigned max_scalars = 0;
   for(ExprChainNode *cur = args->first; cur; cur= cur->next) {
      SymbolType *t = cur->expr->type;
      if(t->scalar_count > max_scalars) {
         max_scalars       = t->scalar_count;
         defining_arg_type = t;
      }
   }

   switch(intrinsic) {
   case INTRINSIC_REINTERPU:
      return glsl_prim_same_shape_type(defining_arg_type, PRIM_UINT);

   case INTRINSIC_REINTERPI:
      return glsl_prim_same_shape_type(defining_arg_type, PRIM_INT);

   case INTRINSIC_REINTERPF:
      return glsl_prim_same_shape_type(defining_arg_type, PRIM_FLOAT);

   case INTRINSIC_FPACK:
      return glsl_prim_same_shape_type(defining_arg_type, PRIM_UINT);

   case INTRINSIC_FUNPACKA:
   case INTRINSIC_FUNPACKB:
      return glsl_prim_same_shape_type(defining_arg_type, PRIM_FLOAT);

   default:
      return defining_arg_type;
   }
}

static SymbolType *calculate_texture_lookup_return_type(ExprChain *args) {
   Expr *bits    = args->first->expr;
   Expr *sampler = args->first->next->expr;

   uint32_t texture_bits = *(const_value *)bits->compile_time_value;

   PrimSamplerInfo *sampler_info = glsl_prim_get_sampler_info(sampler->type->u.primitive_type.index);
   if (texture_bits & GLSL_TEXBITS_GATHER && sampler_info->shadow) return &primitiveTypes[PRIM_VEC4];
   else
      return &primitiveTypes[sampler_info->return_type];
}

static SymbolType *calculate_texture_size_return_type(ExprChain *args) {
   PrimSamplerInfo *sampler_info;
   sampler_info = glsl_prim_get_sampler_info(args->first->expr->type->u.primitive_type.index);
   return glsl_prim_vector_type(PRIM_INT, sampler_info->size_dim);
}

Expr *glsl_intrinsic_construct_expr(glsl_intrinsic_index_t intrinsic, ExprChain *args)
{
#ifndef NDEBUG
   validate_intrinsic(intrinsic, args);
#endif

   Expr *expr               = malloc_fast(sizeof(*expr));
   expr->line_num           = g_LineNumber;
   expr->flavour            = EXPR_INTRINSIC;
   expr->compile_time_value = NULL;

   expr->u.intrinsic.flavour = intrinsic;
   expr->u.intrinsic.args    = args;

   // Set expression type.
   switch (intrinsic) {
   case INTRINSIC_TEXTURE:
      expr->type = calculate_texture_lookup_return_type(args);
      break;
   case INTRINSIC_TEXTURE_SIZE:
      expr->type = calculate_texture_size_return_type(args);
      break;
   default:
      expr->type = calculate_functionlike_intrinsic_return_type(intrinsic, args);
      break;
   }

   return expr;
}

const char *glsl_intrinsic_name(glsl_intrinsic_index_t intrinsic) {
   assert(intrinsic >= 0 && intrinsic < INTRINSIC_COUNT);
   return intrinsic_info[intrinsic].name;
}
