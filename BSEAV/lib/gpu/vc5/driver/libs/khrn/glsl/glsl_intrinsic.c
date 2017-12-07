/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_dataflow_builder.h"
#include "glsl_globals.h"
#include "glsl_intrinsic.h"
#include "glsl_primitive_types.auto.h"
#include "glsl_dataflow.h"
#include "glsl_fastmem.h"

struct intrinsic_info_s {
   glsl_intrinsic_index_t  intrinsic;
   const char             *name;
};

static const struct intrinsic_info_s intrinsic_info[INTRINSIC_COUNT] = {
   { INTRINSIC_TEXTURE,        "texture"        },
   { INTRINSIC_TEXTURE_SIZE,   "textureSize"    },
   { INTRINSIC_RSQRT,          "rsqrt"          },
   { INTRINSIC_RCP,            "rcp"            },
   { INTRINSIC_LOG2,           "log2"           },
   { INTRINSIC_EXP2,           "exp2"           },
   { INTRINSIC_CEIL,           "ceil"           },
   { INTRINSIC_FLOOR,          "floor"          },
   { INTRINSIC_TRUNC,          "trunc"          },
   { INTRINSIC_NEAREST,        "nearest"        },
   { INTRINSIC_MIN,            "min"            },
   { INTRINSIC_MAX,            "max"            },
   { INTRINSIC_ABS,            "abs"            },
   { INTRINSIC_FDX,            "fdx"            },
   { INTRINSIC_FDY,            "fdy"            },
   { INTRINSIC_CLZ,            "clz"            },
   { INTRINSIC_ROR,            "ror"            },
   { INTRINSIC_REINTERPF,      "reinterpf"      },
   { INTRINSIC_REINTERPI,      "reinterpi"      },
   { INTRINSIC_REINTERPU,      "reinterpu"      },
   { INTRINSIC_FPACK,          "fpack"          },
   { INTRINSIC_FUNPACKA,       "funpacka"       },
   { INTRINSIC_FUNPACKB,       "funpackb"       },
   { INTRINSIC_SIN,            "sin"            },
   { INTRINSIC_COS,            "cos"            },
   { INTRINSIC_TAN,            "tan"            },
   { INTRINSIC_ISNAN,          "isnan"          },
   { INTRINSIC_ATOMIC_LOAD,    "atomic_load"    },
   { INTRINSIC_ATOMIC_ADD,     "atomic_add"     },
   { INTRINSIC_ATOMIC_SUB,     "atomic_sub"     },
   { INTRINSIC_ATOMIC_MIN,     "atomic_min"     },
   { INTRINSIC_ATOMIC_MAX,     "atomic_max"     },
   { INTRINSIC_ATOMIC_AND,     "atomic_and"     },
   { INTRINSIC_ATOMIC_OR,      "atomic_or"      },
   { INTRINSIC_ATOMIC_XOR,     "atomic_xor"     },
   { INTRINSIC_ATOMIC_XCHG,    "atomic_xchg"    },
   { INTRINSIC_ATOMIC_CMPXCHG, "atomic_cmpxchg" },
   { INTRINSIC_IMAGE_STORE,    "imageStore"     },
   { INTRINSIC_IMAGE_ADD,      "imageAtomicAdd" },
   { INTRINSIC_IMAGE_MIN,      "imageAtomicMin" },
   { INTRINSIC_IMAGE_MAX,      "imageAtomicMax" },
   { INTRINSIC_IMAGE_AND,      "imageAtomicAnd" },
   { INTRINSIC_IMAGE_OR,       "imageAtomicOr"  },
   { INTRINSIC_IMAGE_XOR,      "imageAtomicXor" },
   { INTRINSIC_IMAGE_XCHG,     "imageAtomicXchg" },
   { INTRINSIC_IMAGE_CMPXCHG,  "imageAtomicCmpxchg" },
};

#ifndef NDEBUG
static unsigned count_sampler_derivs(const PrimSamplerInfo *sampler_info) {
   return sampler_info->coord_count;
}

static unsigned count_sampler_coords(const PrimSamplerInfo *sampler_info, bool is_image) {
   unsigned count = sampler_info->coord_count;
   /* for image cube arrays do not have another coord */
   if (sampler_info->array && (!is_image || !sampler_info->cube))
      count += 1;
   return count;
}

static void validate_texture_lookup(ExprChain *args) {
   Expr *bits    = args->first->expr;
   Expr *sampler = args->first->next->expr;
   Expr *coord   = args->first->next->next->expr;
   Expr *lod     = args->first->next->next->next->expr;
   Expr *offset  = NULL, *comp = NULL, *dref = NULL;

   PrimSamplerInfo *sampler_info = glsl_prim_get_image_info(sampler->type->u.primitive_type.index);

   assert(bits->type->flavour                == SYMBOL_PRIMITIVE_TYPE &&
          bits->type->u.primitive_type.index == PRIM_INT);

   uint32_t texture_bits = *(const_value *)bits->compile_time_value;
   ExprChainNode *arg = args->first->next->next->next->next;
   if (glsl_prim_sampler_is_shadow(sampler->type->u.primitive_type.index)) {
      dref = arg->expr;
      arg = arg->next;
   }
   if ((texture_bits & DF_TEXBITS_GATHER) && arg != NULL) {
      comp = arg->expr;
      arg = arg->next;
   }
   if (arg != NULL) {
      offset = arg->expr;
      arg = arg->next;
   }

   assert(arg == NULL);

   bool is_image = glsl_prim_is_prim_image_type(sampler->type);
   if (texture_bits & (DF_TEXBITS_FETCH | DF_TEXBITS_SAMPLER_FETCH)) {
      assert(glsl_prim_is_vector_type(coord->type, PRIM_INT, count_sampler_coords(sampler_info, is_image)));
      assert(lod->type == &primitiveTypes[PRIM_INT]);
   } else {
      assert(glsl_prim_is_vector_type(coord->type, PRIM_FLOAT, count_sampler_coords(sampler_info, is_image)));
      assert(lod->type == &primitiveTypes[PRIM_FLOAT]);
   }

   if(offset) {
      assert(count_sampler_derivs(sampler_info) > 0 &&
             count_sampler_derivs(sampler_info) < 4);
      assert((texture_bits & DF_TEXBITS_I_OFF) || glsl_prim_is_vector_type(offset->type, PRIM_INT, count_sampler_derivs(sampler_info)));
   }

   assert(!comp || comp->type == &primitiveTypes[PRIM_INT]);
   assert(!dref || dref->type == &primitiveTypes[PRIM_FLOAT]);
}

static void validate_image_atomic(glsl_intrinsic_index_t index, ExprChain *args)
{
   assert( (index != INTRINSIC_IMAGE_CMPXCHG && args->count == 3) ||
           (index == INTRINSIC_IMAGE_CMPXCHG && args->count == 4)  );

   Expr *sampler = args->first->expr;
   Expr *coord   = args->first->next->expr;
   Expr *data    = args->first->next->next->expr;

   PrimSamplerInfo *sampler_info;
   sampler_info = glsl_prim_get_image_info(sampler->type->u.primitive_type.index);
   assert(glsl_prim_is_vector_type(coord->type, PRIM_INT, count_sampler_coords(sampler_info, true)));
   assert(data->type->flavour == SYMBOL_PRIMITIVE_TYPE);

   if (index != INTRINSIC_IMAGE_STORE) {
      //validate that data type is uint or int
      assert(data->type->scalar_count == 1);
      assert(glsl_prim_is_base_of_prim_type(&primitiveTypes[sampler_info->return_type], data->type));
   } else {
      //validate that data type is ivec4, uvec4 or uvec4 depending on the sampler's return_type
      assert(data->type->scalar_count == 4);
      assert(data->type->u.primitive_type.index == sampler_info->return_type);
   }

   if (args->count == 4) {
      Expr *cmp_expr = args->first->next->next->next->expr;
      assert(cmp_expr->type->scalar_count == 1);
      assert(cmp_expr->type == data->type);
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
      /* (ivec2|ivec3) <- intrinsic(genImageType) */
      assert(args->count == 1);
      assert(glsl_prim_is_prim_comb_sampler_type(args->first->expr->type) ||
             glsl_prim_is_prim_image_type(args->first->expr->type));
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
   case INTRINSIC_ABS:
   case INTRINSIC_ISNAN:
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

   case INTRINSIC_CLZ:
      /* genUtype <- intrinsic(genUType); */
      assert(args->count == 1);
      assert(glsl_prim_is_prim_with_base_type(args->first->expr->type, PRIM_UINT));
      break;

   case INTRINSIC_ROR:
      /* genUtype <- intrinsic(genUType, uint); */
      assert(args->count == 2);
      assert(glsl_prim_is_prim_with_base_type(args->first->expr->type, PRIM_UINT));
      assert(args->first->next->expr->type == &primitiveTypes[PRIM_UINT]);
      break;

   case INTRINSIC_ATOMIC_LOAD:
      assert(args->count == 1);
      assert(glsl_prim_is_prim_atomic_type(args->first->expr->type));
      break;

   case INTRINSIC_ATOMIC_ADD:
   case INTRINSIC_ATOMIC_SUB:
   case INTRINSIC_ATOMIC_MIN:
   case INTRINSIC_ATOMIC_MAX:
   case INTRINSIC_ATOMIC_AND:
   case INTRINSIC_ATOMIC_OR:
   case INTRINSIC_ATOMIC_XOR:
   case INTRINSIC_ATOMIC_XCHG:
      assert(args->count == 2);
      if (glsl_prim_is_prim_atomic_type(args->first->expr->type))
         assert(args->first->next->expr->type == &primitiveTypes[PRIM_UINT]);
      else {
         assert(args->first->expr->type == args->first->next->expr->type &&
                 (args->first->expr->type == &primitiveTypes[PRIM_INT] ||
                  args->first->expr->type == &primitiveTypes[PRIM_UINT]  ) );
      }
      break;

   case INTRINSIC_ATOMIC_CMPXCHG:
      assert(args->count == 3);
      assert(args->first->expr->type == args->first->next->expr->type       &&
             args->first->expr->type == args->first->next->next->expr->type &&
              (args->first->expr->type == &primitiveTypes[PRIM_INT] ||
               args->first->expr->type == &primitiveTypes[PRIM_UINT]  ) );
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
      validate_image_atomic(intrinsic, args);
      break;
   default:
      unreachable();
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

   case INTRINSIC_ATOMIC_LOAD:
   case INTRINSIC_ATOMIC_ADD:
   case INTRINSIC_ATOMIC_SUB:
   case INTRINSIC_ATOMIC_MIN:
   case INTRINSIC_ATOMIC_MAX:
   case INTRINSIC_ATOMIC_AND:
   case INTRINSIC_ATOMIC_OR:
   case INTRINSIC_ATOMIC_XOR:
   case INTRINSIC_ATOMIC_XCHG:
   case INTRINSIC_ATOMIC_CMPXCHG:
      if (glsl_prim_is_prim_atomic_type(args->first->expr->type))
         return &primitiveTypes[PRIM_UINT];
      else
         return args->first->expr->type;
   default:
      return defining_arg_type;
   }
}

static SymbolType *calculate_texture_lookup_return_type(ExprChain *args) {
   Expr *bits    = args->first->expr;
   Expr *sampler = args->first->next->expr;

   uint32_t texture_bits = *(const_value *)bits->compile_time_value;

   PrimSamplerInfo *sampler_info = glsl_prim_get_image_info(sampler->type->u.primitive_type.index);

   if (glsl_prim_sampler_is_shadow(sampler->type->u.primitive_type.index) && !(texture_bits & DF_TEXBITS_GATHER))
      return &primitiveTypes[PRIM_FLOAT];
   else
      return &primitiveTypes[sampler_info->return_type];
}

static SymbolType *calculate_texture_size_return_type(ExprChain *args) {
   PrimSamplerInfo *sampler_info = glsl_prim_get_image_info(args->first->expr->type->u.primitive_type.index);
   return glsl_prim_vector_type(PRIM_INT, sampler_info->size_dim);
}

Expr *glsl_intrinsic_construct_expr(int line_num, glsl_intrinsic_index_t intrinsic, ExprChain *args)
{
#ifndef NDEBUG
   validate_intrinsic(intrinsic, args);
#endif

   Expr *expr               = malloc_fast(sizeof(*expr));
   expr->flavour            = EXPR_INTRINSIC;
   expr->line_num           = line_num;
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
   case INTRINSIC_IMAGE_ADD:
   case INTRINSIC_IMAGE_MIN:
   case INTRINSIC_IMAGE_MAX:
   case INTRINSIC_IMAGE_AND:
   case INTRINSIC_IMAGE_OR:
   case INTRINSIC_IMAGE_XOR:
   case INTRINSIC_IMAGE_XCHG:
   case INTRINSIC_IMAGE_CMPXCHG:
      expr->type = args->first->next->next->expr->type;  /* Matches data argument */
      break;
   case INTRINSIC_IMAGE_STORE:
      expr->type = &primitiveTypes[PRIM_VOID];
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
