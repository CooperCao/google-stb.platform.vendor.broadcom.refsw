/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_const_operators.h"
#include "glsl_dataflow_simplify.h"
#include "glsl_tex_params.h"

//#define DATAFLOW_DONT_OPTIMISE_CONST_ZERO
//#define DISABLE_CONSTANT_FOLDING

typedef enum {
   FOLDER_CANT_FOLD,
   FOLDER_RET_BOOL,
   FOLDER_RET_FLOAT,
   FOLDER_RET_INT,
   FOLDER_RET_UINT,
   FOLDER_RET_MATCH
} FolderResultType;

typedef const_value (*unary_fold)(const_value);
typedef const_value (*binary_fold)(const_value, const_value);

struct dataflow_fold_info_s {
   DataflowFlavour   flavour; // for checking the table
   FolderResultType  type;
   int               num_args;
   // Folding functions, one for each type
   union {
      unary_fold  uf[4];
      binary_fold bf[4];
   };
};

static const_value const_zero(const_value input) { return 0; }

/* Order: bool, float, int, uint */
static const struct dataflow_fold_info_s dataflow_info[DATAFLOW_FLAVOUR_COUNT] = {
   { DATAFLOW_CONST,               FOLDER_CANT_FOLD, },
   { DATAFLOW_LOAD,                FOLDER_CANT_FOLD, },
   { DATAFLOW_PHI,                 FOLDER_CANT_FOLD, },
   { DATAFLOW_EXTERNAL,            FOLDER_CANT_FOLD, },
   { DATAFLOW_LOGICAL_NOT,         FOLDER_RET_MATCH, 1, .uf = { op_logical_not, NULL, NULL, NULL } },
   { DATAFLOW_CONST_SAMPLER,       FOLDER_CANT_FOLD, },
   { DATAFLOW_FTOI_TRUNC,          FOLDER_RET_INT,   1, .uf = { NULL, op_floattoint_trunc,   NULL, NULL }, },
   { DATAFLOW_FTOI_NEAREST,        FOLDER_RET_INT,   1, .uf = { NULL, op_floattoint_nearest, NULL, NULL }, },
   { DATAFLOW_FTOU,                FOLDER_RET_UINT,  1, .uf = { NULL, op_floattouint,        NULL, NULL }, },

   { DATAFLOW_BITWISE_NOT,         FOLDER_RET_MATCH, 1, .uf = { NULL, NULL, op_bitwise_not, op_bitwise_not }, },
   { DATAFLOW_BITWISE_AND,         FOLDER_RET_MATCH, 2, .bf = { NULL, NULL, op_bitwise_and, op_bitwise_and }, },
   { DATAFLOW_BITWISE_OR,          FOLDER_RET_MATCH, 2, .bf = { NULL, NULL, op_bitwise_or,  op_bitwise_or  }, },
   { DATAFLOW_BITWISE_XOR,         FOLDER_RET_MATCH, 2, .bf = { NULL, NULL, op_bitwise_xor, op_bitwise_xor }, },

   { DATAFLOW_SHL,                 FOLDER_RET_MATCH, 2, .bf = { NULL, NULL, op_bitwise_shl,   op_bitwise_shl   }, },
   { DATAFLOW_SHR,                 FOLDER_RET_MATCH, 2, .bf = { NULL, NULL, op_i_bitwise_shr, op_u_bitwise_shr }, },

   { DATAFLOW_ADDRESS_LOAD,        FOLDER_CANT_FOLD, },
   { DATAFLOW_VECTOR_LOAD,         FOLDER_CANT_FOLD, },
   { DATAFLOW_UNIFORM,             FOLDER_CANT_FOLD, },
   { DATAFLOW_UNIFORM_BUFFER,      FOLDER_CANT_FOLD, },
   { DATAFLOW_STORAGE_BUFFER,      FOLDER_CANT_FOLD, },
   { DATAFLOW_ATOMIC_COUNTER,      FOLDER_CANT_FOLD, },
   { DATAFLOW_IN,                  FOLDER_CANT_FOLD, },
   { DATAFLOW_MUL,                 FOLDER_RET_MATCH, 2, .bf = { NULL, op_f_mul, op_i_mul, op_u_mul }, },
   { DATAFLOW_DIV,                 FOLDER_RET_MATCH, 2, .bf = { NULL, op_f_div, op_i_div, op_u_div }, },
   { DATAFLOW_REM,                 FOLDER_RET_MATCH, 2, .bf = { NULL, NULL,     op_i_rem, op_u_rem }, },
   { DATAFLOW_ADD,                 FOLDER_RET_MATCH, 2, .bf = { NULL, op_f_add, op_i_add, op_i_add }, },
   { DATAFLOW_SUB,                 FOLDER_RET_MATCH, 2, .bf = { NULL, op_f_sub, op_i_sub, op_i_sub }, },
   { DATAFLOW_ARITH_NEGATE,        FOLDER_RET_MATCH, 1, .uf = { NULL, op_f_negate, op_i_negate, op_i_negate }, },
   { DATAFLOW_LESS_THAN,           FOLDER_RET_BOOL,  2, .bf = { NULL, op_f_less_than, op_i_less_than, op_u_less_than }, },
   { DATAFLOW_LESS_THAN_EQUAL,     FOLDER_RET_BOOL,  2, .bf = { NULL, op_f_less_than_equal, op_i_less_than_equal, op_u_less_than_equal }, },
   { DATAFLOW_GREATER_THAN,        FOLDER_RET_BOOL,  2, .bf = { NULL, op_f_greater_than, op_i_greater_than, op_u_greater_than }, },
   { DATAFLOW_GREATER_THAN_EQUAL,  FOLDER_RET_BOOL,  2, .bf = { NULL, op_f_greater_than_equal, op_i_greater_than_equal, op_u_greater_than_equal }, },
   { DATAFLOW_EQUAL,               FOLDER_RET_BOOL,  2, .bf = { op_b_equal, op_f_equal, op_i_equal, op_i_equal }, },
   { DATAFLOW_NOT_EQUAL,           FOLDER_RET_BOOL,  2, .bf = { op_b_not_equal, op_f_not_equal, op_i_not_equal, op_i_not_equal }, },
   { DATAFLOW_LOGICAL_AND,         FOLDER_RET_MATCH, 2, .bf = { op_logical_and, NULL, NULL, NULL }, },
   { DATAFLOW_LOGICAL_XOR,         FOLDER_RET_MATCH, 2, .bf = { op_logical_xor, NULL, NULL, NULL }, },
   { DATAFLOW_LOGICAL_OR,          FOLDER_RET_MATCH, 2, .bf = { op_logical_or,  NULL, NULL, NULL }, },
   { DATAFLOW_CONDITIONAL,         FOLDER_CANT_FOLD, },
   { DATAFLOW_RSQRT,               FOLDER_RET_MATCH, 1, .uf = { NULL, op_rsqrt, NULL, NULL }, },
   { DATAFLOW_RCP,                 FOLDER_RET_MATCH, 1, .uf = { NULL, op_recip, NULL, NULL }, },
   { DATAFLOW_LOG2,                FOLDER_RET_MATCH, 1, .uf = { NULL, op_log2,  NULL, NULL }, },
   { DATAFLOW_EXP2,                FOLDER_RET_MATCH, 1, .uf = { NULL, op_exp2,  NULL, NULL }, },
   { DATAFLOW_SIN,                 FOLDER_RET_MATCH, 1, .uf = { NULL, op_sin,   NULL, NULL }, },
   { DATAFLOW_COS,                 FOLDER_RET_MATCH, 1, .uf = { NULL, op_cos,   NULL, NULL }, },
   { DATAFLOW_TAN,                 FOLDER_RET_MATCH, 1, .uf = { NULL, op_tan,   NULL, NULL }, },
   { DATAFLOW_MIN,                 FOLDER_RET_MATCH, 2, .bf = { NULL, op_f_min, op_i_min, op_u_min }, },
   { DATAFLOW_MAX,                 FOLDER_RET_MATCH, 2, .bf = { NULL, op_f_max, op_i_max, op_u_max }, },
   { DATAFLOW_TRUNC,               FOLDER_RET_MATCH, 1, .uf = { NULL, op_trunc, NULL, NULL }, },
   { DATAFLOW_NEAREST,             FOLDER_RET_MATCH, 1, .uf = { NULL, op_round, NULL, NULL }, },
   { DATAFLOW_CEIL,                FOLDER_RET_MATCH, 1, .uf = { NULL, op_ceil,  NULL, NULL }, },
   { DATAFLOW_FLOOR,               FOLDER_RET_MATCH, 1, .uf = { NULL, op_floor, NULL, NULL }, },
   { DATAFLOW_ABS,                 FOLDER_RET_MATCH, 1, .uf = { NULL, op_f_abs, NULL, NULL }, },
   { DATAFLOW_FDX,                 FOLDER_RET_MATCH, 1, .uf = { NULL, const_zero, NULL, NULL }, },
   { DATAFLOW_FDY,                 FOLDER_RET_MATCH, 1, .uf = { NULL, const_zero, NULL, NULL }, },
   { DATAFLOW_REINTERP,            FOLDER_CANT_FOLD, },     /* Handled in a special way */
   { DATAFLOW_FPACK,               FOLDER_RET_UINT,  2, .bf = { NULL, op_fpack, NULL, NULL }, },
   { DATAFLOW_FUNPACKA,            FOLDER_RET_FLOAT, 1, .uf = { NULL, NULL, NULL, op_funpacka }, },
   { DATAFLOW_FUNPACKB,            FOLDER_RET_FLOAT, 1, .uf = { NULL, NULL, NULL, op_funpackb }, },

   { DATAFLOW_ITOF,                FOLDER_RET_FLOAT, 1, .uf = { NULL, NULL, op_inttofloat, NULL }, },
   { DATAFLOW_UTOF,                FOLDER_RET_FLOAT, 1, .uf = { NULL, NULL, NULL, op_uinttofloat }, },
   { DATAFLOW_CLZ,                 FOLDER_RET_UINT,  1, .uf = { NULL, NULL, NULL, op_clz }, },

   { DATAFLOW_VEC4,                FOLDER_CANT_FOLD, },
   { DATAFLOW_TEXTURE,             FOLDER_CANT_FOLD, },
   { DATAFLOW_TEXTURE_SIZE,        FOLDER_CANT_FOLD, },
   { DATAFLOW_GET_VEC4_COMPONENT,  FOLDER_CANT_FOLD, },
   { DATAFLOW_FRAG_GET_COL,        FOLDER_CANT_FOLD, },
   { DATAFLOW_FRAG_GET_X,          FOLDER_CANT_FOLD, },
   { DATAFLOW_FRAG_GET_Y,          FOLDER_CANT_FOLD, },
   { DATAFLOW_FRAG_GET_X_UINT,     FOLDER_CANT_FOLD, },
   { DATAFLOW_FRAG_GET_Y_UINT,     FOLDER_CANT_FOLD, },
   { DATAFLOW_FRAG_GET_Z,          FOLDER_CANT_FOLD, },
   { DATAFLOW_FRAG_GET_W,          FOLDER_CANT_FOLD, },
   { DATAFLOW_FRAG_GET_FF,         FOLDER_CANT_FOLD, },
   { DATAFLOW_GET_THREAD_INDEX,    FOLDER_CANT_FOLD, },
   { DATAFLOW_IS_HELPER,           FOLDER_CANT_FOLD, },
   { DATAFLOW_GET_VERTEX_ID,       FOLDER_CANT_FOLD, },
   { DATAFLOW_GET_INSTANCE_ID,     FOLDER_CANT_FOLD, },
   { DATAFLOW_GET_POINT_COORD_X,   FOLDER_CANT_FOLD, },
   { DATAFLOW_GET_POINT_COORD_Y,   FOLDER_CANT_FOLD, },
   { DATAFLOW_GET_LINE_COORD,      FOLDER_CANT_FOLD, },
   { DATAFLOW_GET_DEPTHRANGE_NEAR, FOLDER_CANT_FOLD, },
   { DATAFLOW_GET_DEPTHRANGE_FAR,  FOLDER_CANT_FOLD, },
   { DATAFLOW_GET_DEPTHRANGE_DIFF, FOLDER_CANT_FOLD, },
   { DATAFLOW_ADDRESS,             FOLDER_CANT_FOLD, },
};

static int df_type_index(DataflowType t) {
   switch (t) {
      case DF_BOOL:  return 0;
      case DF_FLOAT: return 1;
      case DF_INT:   return 2;
      case DF_UINT:  return 3;
      default: unreachable(); return -1;
   }
}

static DataflowType get_folder_type(FolderResultType folder_type, DataflowType df_type) {
   switch (folder_type) {
      case FOLDER_RET_MATCH: return df_type;
      case FOLDER_RET_BOOL:  return DF_BOOL;
      case FOLDER_RET_FLOAT: return DF_FLOAT;
      case FOLDER_RET_INT:   return DF_INT;
      case FOLDER_RET_UINT:  return DF_UINT;
      default: unreachable();return DF_INVALID;
   }
}

static Dataflow *simplify_reinterp(Dataflow *dataflow)
{
   Dataflow *operand = dataflow->d.dependencies[0];
   if (dataflow->type == operand->type) return operand;

   if (operand->flavour == DATAFLOW_CONST)
      return glsl_dataflow_construct_const_value(dataflow->type, operand->u.constant.value);

   return NULL;
}

static Dataflow *simplify_texture(Dataflow *dataflow) {
   /* If using normal bias with a bias of 0 then drop it */
   if ( !(dataflow->u.texture.bits & (GLSL_TEXBITS_BSLOD | GLSL_TEXBITS_FETCH)) &&
        dataflow->d.texture.b          != NULL           &&
        dataflow->d.texture.b->flavour == DATAFLOW_CONST &&
        dataflow->d.texture.b->u.constant.value == 0 )
   {
      dataflow->d.texture.b = NULL;
   }
   return dataflow;
}

static int float_as_small_int(const_value f) {
   if (f == 0x40000000) return 2;
   if (f == 0x40400000) return 3;
   if (f == 0x40800000) return 4;
   return -1;
}

static Dataflow *simplify_small_pow(Dataflow *log, Dataflow *pow) {
   Dataflow *x = log->d.unary_op.operand;
   int p = float_as_small_int(pow->u.constant.value);
   if (p == -1) return NULL;      /* A power we don't optimise */

   Dataflow *xx = glsl_dataflow_construct_binary_op(DATAFLOW_MUL, x, x);
   if (p == 2) return xx;
   if (p == 3) return glsl_dataflow_construct_binary_op(DATAFLOW_MUL, xx, x);
   if (p == 4) return glsl_dataflow_construct_binary_op(DATAFLOW_MUL, xx, xx);
   return NULL;
}

static Dataflow *simplify_unary_op (DataflowFlavour flavour, Dataflow *operand) {
   if (operand->flavour == DATAFLOW_CONST) {
      const struct dataflow_fold_info_s *info = &dataflow_info[flavour];
      assert(info->flavour == flavour);
      if (info->type != FOLDER_CANT_FOLD) {
         assert(info->num_args == 1);
         int id = df_type_index(operand->type);
         unary_fold folding_func = info->uf[id];
         DataflowType t = get_folder_type(info->type, operand->type);
         return glsl_dataflow_construct_const_value(t, folding_func(operand->u.constant.value));
      }
   }

   if (flavour == operand->flavour) {
      switch(flavour) {
      /* Self-inverting operations */
      case DATAFLOW_RCP:
      case DATAFLOW_LOGICAL_NOT:
      case DATAFLOW_BITWISE_NOT:
      case DATAFLOW_ARITH_NEGATE:
         return operand->d.unary_op.operand;
      /* Idempotent operations */
      case DATAFLOW_TRUNC:
      case DATAFLOW_NEAREST:
      case DATAFLOW_CEIL:
      case DATAFLOW_FLOOR:
      case DATAFLOW_ABS:
         return operand;
      default:
         break;
      }
   }
   if ( (flavour == DATAFLOW_EXP2 && operand->flavour == DATAFLOW_LOG2) ||
        (flavour == DATAFLOW_LOG2 && operand->flavour == DATAFLOW_EXP2)  )
      return operand->d.unary_op.operand;

   /* Optimise expressions that resolve to small powers */
   if (flavour == DATAFLOW_EXP2 && operand->flavour == DATAFLOW_MUL) {
      Dataflow *l = operand->d.binary_op.left;
      Dataflow *r = operand->d.binary_op.right;
      if (l->flavour == DATAFLOW_LOG2 && r->flavour == DATAFLOW_CONST)
         return simplify_small_pow(l, r);
      if (r->flavour == DATAFLOW_LOG2 && l->flavour == DATAFLOW_CONST)
         return simplify_small_pow(r, l);
   }

   return NULL;
}

static bool is_pow2(uint32_t i) {
   return (i != 0) && ((i & (i - 1)) == 0);
}

static unsigned log2i(uint32_t y) {
   for (unsigned i=0; i<32; i++) {
      if (y == (1u<<i)) return i;
   }
   assert(0);
   return 0;
}

#define OR_BUF_SIZE 50
static bool push_ors(Dataflow *it, Dataflow **ors, int *n_ors);

static Dataflow *simplify_binary_op (DataflowFlavour flavour, Dataflow *left, Dataflow *right) {
   if (left->flavour == DATAFLOW_CONST && right->flavour == DATAFLOW_CONST) {
      const struct dataflow_fold_info_s *info = &dataflow_info[flavour];
      assert(info->flavour == flavour);
      if (info->type != FOLDER_CANT_FOLD) {
         assert(info->num_args == 2);
         int id = df_type_index(left->type);
         binary_fold folding_func = info->bf[id];
         DataflowType t = get_folder_type(info->type, left->type);
         return glsl_dataflow_construct_const_value(t, folding_func(left->u.constant.value, right->u.constant.value));
      }
   }

   if (flavour == DATAFLOW_DIV) {
      if (left->type == DF_FLOAT) {
         /* Canonicalise to multiplication by reciprocal. Better for CSE */
         return glsl_dataflow_construct_binary_op(DATAFLOW_MUL, left,
                              glsl_dataflow_construct_unary_op(DATAFLOW_RCP, right));
      }

      /* TODO: For cases of undefined behaviour these optimisations don't give
       *       the same results. Is this OK?                                   */
      if ((right->type == DF_INT || right->type == DF_UINT) && right->flavour == DATAFLOW_CONST) {
         if (right->u.constant.value == 0) {
            return glsl_dataflow_construct_const_value(right->type, 0xFFFFFFFFu);
         }

         if (is_pow2(right->u.constant.value)) {
            if (right->type == DF_INT) {
               /* For signed ints, fix up rounding by adding (l>>31) & (r-1) */
               /* This doesn't overflow, since we only add anything if l < 0 */
               /* (since r is 2^k it is either > 1 or INT_MIN so r-1 > 0) */
               /* NOTE: The rounding correction assumes signed integer wrapping on underflow */
               Dataflow *c31 = glsl_dataflow_construct_const_int(31);
               Dataflow *sign_mask = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, left, c31);
               Dataflow *right_m_1 = glsl_dataflow_construct_const_int(right->u.constant.value-1);
               Dataflow *round_fix = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, sign_mask, right_m_1);
               left = glsl_dataflow_construct_binary_op(DATAFLOW_ADD, left, round_fix);

               if (right->u.constant.value == 0x80000000u) {
                  /* The rounding correction has left us with -1 if the input
                   * was INT_MIN, or something +ve. Shift out all +ve to 0 */
                  left = glsl_dataflow_construct_binary_op(DATAFLOW_SHR, left, c31);
                  /* Negate the result */
                  return glsl_dataflow_construct_unary_op(DATAFLOW_ARITH_NEGATE, left);
               }
            }

            Dataflow *l2 = glsl_dataflow_construct_const_value(right->type, log2i(right->u.constant.value));
            return glsl_dataflow_construct_binary_op(DATAFLOW_SHR, left, l2);
         }
      }
   }

   if (flavour == DATAFLOW_MUL && (left->type == DF_INT || left->type == DF_UINT)) {
      if (left->flavour == DATAFLOW_CONST && is_pow2(left->u.constant.value)) {
         Dataflow *c = glsl_dataflow_construct_const_value(left->type, log2i(left->u.constant.value));
         return glsl_dataflow_construct_binary_op(DATAFLOW_SHL, right, c);
      }
      if (right->flavour == DATAFLOW_CONST && is_pow2(right->u.constant.value)) {
         Dataflow *c = glsl_dataflow_construct_const_value(right->type, log2i(right->u.constant.value));
         return glsl_dataflow_construct_binary_op(DATAFLOW_SHL, left, c);
      }
   }


   if (flavour == DATAFLOW_REM) {
      /* Results here are undefined if either left or right are negative, so do
       * the simplest thing. The results are pretty wildly wrong if right == INT_MIN */
      if ((right->type == DF_INT || right->type == DF_UINT) && right->flavour == DATAFLOW_CONST) {
         if (right->u.constant.value == 0)
            return glsl_dataflow_construct_const_value(right->type, 0);

         if(is_pow2(right->u.constant.value)) {
            Dataflow *rm1 = glsl_dataflow_construct_const_value(right->type, right->u.constant.value-1);
            return glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, left, rm1);
         }
      }
   }

   /* Reassociate integer add/sub expressions. Turn (x+c1)+c2 into ((c1+c2) + x), and
    * similarly for -. Pull the constants out, recording signs, leaving the non-constant
    * nc part. Reconstruct the final expression, applying the correct signs. */
   if (glsl_dataflow_is_integral_type(left) && (flavour == DATAFLOW_ADD || flavour == DATAFLOW_SUB)) {
      Dataflow *c1 = NULL, *c2 = NULL, *nc = NULL;
      bool sub_c1, sub_nc;
      if (left->flavour == DATAFLOW_CONST) {
         sub_c1 = false; sub_nc = (flavour == DATAFLOW_SUB);
         c1 = left;
         nc = right;
      } else if (right->flavour == DATAFLOW_CONST) {
         sub_c1 = (flavour == DATAFLOW_SUB); sub_nc = false;
         c1 = right;
         nc = left;
      }

      if (c1 != NULL && (nc->flavour == DATAFLOW_ADD || nc->flavour == DATAFLOW_SUB)) {
         bool sub_c2;
         if (nc->d.binary_op.left->flavour == DATAFLOW_CONST) {
            sub_c2 = sub_nc; sub_nc = sub_nc ^ (nc->flavour == DATAFLOW_SUB);
            c2 = nc->d.binary_op.left;
            nc = nc->d.binary_op.right;
         } else if (nc->d.binary_op.right->flavour == DATAFLOW_CONST) {
            sub_c2 = sub_nc ^ (nc->flavour == DATAFLOW_SUB);
            c2 = nc->d.binary_op.right;
            nc = nc->d.binary_op.left;
         }

         if (c2 != NULL) {
            const_value c_val = (sub_c1 ^ sub_c2) ? op_i_sub(c1->u.constant.value, c2->u.constant.value) :
                                                    op_i_add(c1->u.constant.value, c2->u.constant.value);
            if (sub_c1) c_val = op_i_negate(c_val);
            Dataflow *c = glsl_dataflow_construct_const_value(left->type, c_val);
            DataflowFlavour f = sub_nc ? DATAFLOW_SUB : DATAFLOW_ADD;
            return glsl_dataflow_construct_binary_op(f, c, nc);
         }
      }
   }

   if (left->flavour == DATAFLOW_CONST) {
      if (left->type == DF_FLOAT) {
#if !defined(DATAFLOW_DONT_OPTIMISE_CONST_ZERO)
         if (left->u.constant.value == CONST_FLOAT_ZERO || left->u.constant.value == CONST_FLOAT_MINUS_ZERO) {
            if (flavour == DATAFLOW_MUL) return left;
            if (flavour == DATAFLOW_ADD) return right;
         }
         if (left->u.constant.value == CONST_FLOAT_ONE) {
            if (flavour == DATAFLOW_MUL) return right;
         }
#endif
      }
      if (left->type == DF_INT || left->type == DF_UINT) {
         if (left->u.constant.value == 0) {
            if (flavour == DATAFLOW_DIV) return left;
            if (flavour == DATAFLOW_REM) return left;
            if (flavour == DATAFLOW_MUL) return left;
            if (flavour == DATAFLOW_ADD) return right;
            if (flavour == DATAFLOW_SHL) return left;
            if (flavour == DATAFLOW_SHR) return left;
            if (flavour == DATAFLOW_BITWISE_AND) return left;
            if (flavour == DATAFLOW_BITWISE_OR ) return right;
            if (flavour == DATAFLOW_BITWISE_XOR) return right;
         }
         if (left->u.constant.value == 1) {
            if (flavour == DATAFLOW_MUL) return right;
         }
      }
      if (left->type == DF_BOOL) {
         if (flavour == DATAFLOW_LOGICAL_AND) {
            if (left->u.constant.value)
               return right;
            else
               return left;
         }
         if (flavour == DATAFLOW_LOGICAL_OR) {
            if (left->u.constant.value)
               return left;
            else
               return right;
         }
      }
   }

   if (right->flavour == DATAFLOW_CONST) {
      if (right->type == DF_FLOAT) {
#if !defined(DATAFLOW_DONT_OPTIMISE_CONST_ZERO)
         if (right->u.constant.value == CONST_FLOAT_ZERO || right->u.constant.value == CONST_FLOAT_MINUS_ZERO) {
            if (flavour == DATAFLOW_MUL) return right;
            if (flavour == DATAFLOW_ADD) return left;
            if (flavour == DATAFLOW_SUB) return left;
         }
         if (right->u.constant.value == CONST_FLOAT_ONE) {
            if (flavour == DATAFLOW_MUL) return left;
         }
#endif
      }
      if (right->type == DF_INT || right->type == DF_UINT) {
         if (right->u.constant.value == 0) {
            if (flavour == DATAFLOW_MUL) return right;
            if (flavour == DATAFLOW_ADD) return left;
            if (flavour == DATAFLOW_SHL) return left;
            if (flavour == DATAFLOW_SHR) return left;
            if (flavour == DATAFLOW_BITWISE_AND) return right;
            if (flavour == DATAFLOW_BITWISE_OR ) return left;
            if (flavour == DATAFLOW_BITWISE_XOR) return left;
            if (flavour == DATAFLOW_SUB) return left;
         }
         if (right->u.constant.value == 1) {
            if (flavour == DATAFLOW_MUL) return left;
         }
      }
      if (right->type == DF_BOOL) {
         if (flavour == DATAFLOW_LOGICAL_AND) {
            if (right->u.constant.value)
               return left;
            else
               return right;
         }
         if (flavour == DATAFLOW_LOGICAL_OR) {
            if (right->u.constant.value)
               return right;
            else
               return left;
         }
      }
   }

   if (left == right) {
      /* These operations are NaN-correct */
      if (flavour == DATAFLOW_BITWISE_AND) return left;
      if (flavour == DATAFLOW_BITWISE_OR)  return left;
      if (flavour == DATAFLOW_BITWISE_XOR) return glsl_dataflow_construct_const_value(left->type, 0);
      if (flavour == DATAFLOW_MIN)         return left;
      if (flavour == DATAFLOW_MAX)         return left;
      if (flavour == DATAFLOW_LOGICAL_AND) return left;
      if (flavour == DATAFLOW_LOGICAL_XOR) return glsl_dataflow_construct_const_bool(0);
      if (flavour == DATAFLOW_LOGICAL_OR)  return left;
      /* These are not */
      if (flavour == DATAFLOW_SUB)                return glsl_dataflow_construct_const_value(left->type, 0);
      if (flavour == DATAFLOW_LESS_THAN)          return glsl_dataflow_construct_const_bool(0);
      if (flavour == DATAFLOW_LESS_THAN_EQUAL)    return glsl_dataflow_construct_const_bool(1);
      if (flavour == DATAFLOW_GREATER_THAN)       return glsl_dataflow_construct_const_bool(0);
      if (flavour == DATAFLOW_GREATER_THAN_EQUAL) return glsl_dataflow_construct_const_bool(1);
      if (flavour == DATAFLOW_EQUAL)              return glsl_dataflow_construct_const_bool(1);
      if (flavour == DATAFLOW_NOT_EQUAL)          return glsl_dataflow_construct_const_bool(0);
   }

   // simplify common guard expressions
   if (flavour == DATAFLOW_LOGICAL_OR) {
      if (right->flavour == DATAFLOW_LOGICAL_NOT && left == right->d.unary_op.operand) {
         // a || ~a -> 1
         return glsl_dataflow_construct_const_bool(true);
      }
      if (left->flavour == DATAFLOW_LOGICAL_NOT && left->d.unary_op.operand == right) {
         // ~a || a -> 1
         return glsl_dataflow_construct_const_bool(true);
      }
      // ( a && b ) || ( a && c ) --> a && (b || c)
      if (left->flavour == DATAFLOW_LOGICAL_AND && right->flavour == DATAFLOW_LOGICAL_AND) {
         Dataflow* a1 = left->d.binary_op.left;
         Dataflow* b1 = left->d.binary_op.right;
         Dataflow* a2 = right->d.binary_op.left;
         Dataflow* b2 = right->d.binary_op.right;
         if (a1 == a2) {
            return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, a1, glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR, b1, b2));
         }
         if (b1 == b2) {
            return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, b1, glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR, a1, a2));
         }
      }

      {
         /* XXX Try to reassociate boolean expressions to eliminate complex guard expressions */
         Dataflow *res;
         Dataflow *ors[OR_BUF_SIZE] = { NULL, };
         int n_ors = 0;
         bool changed = false;

         /* Create a list of the things that are or-ed together */
         if (left->flavour  != DATAFLOW_LOGICAL_OR) ors[n_ors++] = left;
         if (right->flavour != DATAFLOW_LOGICAL_OR) ors[n_ors++] = right;

         if (left->flavour == DATAFLOW_LOGICAL_OR) {
            if (!push_ors(left, ors, &n_ors)) return NULL;
         }
         if (right->flavour == DATAFLOW_LOGICAL_OR) {
            if (!push_ors(right, ors, &n_ors)) return NULL;
         }

         /* Now scan for pairs that want to be placed together */
         for (int i=0; i<n_ors-1; i++) {
            if (ors[i]->flavour == DATAFLOW_LOGICAL_AND) {
               Dataflow *a = ors[i]->d.binary_op.left;
               Dataflow *b = ors[i]->d.binary_op.right;
               for (int j=i+1; j<n_ors; j++) {
                  if (ors[j]->flavour == DATAFLOW_LOGICAL_AND) {
                     if (ors[j]->d.binary_op.left  == a || ors[j]->d.binary_op.right == a ||
                         ors[j]->d.binary_op.left  == b || ors[j]->d.binary_op.right == b   )
                     {
                        if (ors[i]->d.binary_op.left == ors[j]->d.binary_op.left) {
                           int k;
                           ors[i] = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, ors[j]->d.binary_op.left, glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR, ors[i]->d.binary_op.right, ors[j]->d.binary_op.right));
                           for (k=j; k < n_ors-1; k++) {
                              ors[k] = ors[k+1];
                           }
                           n_ors--;
                           changed = true;
                           break;
                        }
                     }
                  }
               }
            }
         }
         if (changed) {
            assert(n_ors > 1);
            res = ors[0];
            for (int i=1; i<n_ors; i++) {
               res = glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR, res, ors[i]);
            }

            return res;
         }
      }
   }

   return NULL;
}

static bool push_ors(Dataflow *it, Dataflow **ors, int *n_ors) {
   assert(it->flavour == DATAFLOW_LOGICAL_OR);

   if (it->d.binary_op.left->flavour == DATAFLOW_LOGICAL_OR) {
      if (!push_ors(it->d.binary_op.left, ors, n_ors)) return false;
   } else {
      if (*n_ors == OR_BUF_SIZE) return false;
      ors[(*n_ors)++] = it->d.binary_op.left;
   }

   if (it->d.binary_op.right->flavour == DATAFLOW_LOGICAL_OR) {
      if (!push_ors(it->d.binary_op.right, ors, n_ors)) return false;
   } else {
      if (*n_ors == OR_BUF_SIZE) return false;
      ors[(*n_ors)++] = it->d.binary_op.right;
   }

   return true;
}

static Dataflow *simplify_ternary_op (DataflowFlavour flavour, Dataflow *cond, Dataflow *true_value, Dataflow *false_value) {
   if(flavour != DATAFLOW_CONDITIONAL) {
      return NULL;
   }

   if (true_value == false_value) {
      return true_value;
   }

   if (cond->flavour == DATAFLOW_CONST) {
      if (cond->u.constant.value) {
         return true_value;
      } else {
         return false_value;
      }
   }

   if (true_value->flavour == DATAFLOW_CONST && false_value->flavour == DATAFLOW_CONST) {
      if (true_value->u.constant.value == false_value->u.constant.value) {
         return true_value;
      }
   }

   if (true_value->type == DF_BOOL) {
      if (true_value == cond) {
         return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR, cond, false_value);
      }
      if (false_value == cond) {
         return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, true_value);
      }
      if (true_value->flavour == DATAFLOW_CONST && false_value->flavour == DATAFLOW_CONST) {
         if (true_value->u.constant.value) {
            assert(!false_value->u.constant.value);
            // c ? 1 : 0 -> c
            return cond;
         } else {
            assert(false_value->u.constant.value);
            // c ? 0 : 1 -> ~c
            return glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, cond);
         }
      }
      if (true_value->flavour == DATAFLOW_CONST) {
         if (true_value->u.constant.value) {
            // c ? 1 : f ->  c | f
            return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR, cond, false_value);
         } else {
            // c ? 0 : f -> ~c & f
            return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, cond), false_value);
         }
      }
      if (false_value->flavour == DATAFLOW_CONST) {
         if (false_value->u.constant.value) {
            // c ? t : 1 -> ~c | t
            return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR, glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, cond), true_value);
         } else {
            // c ? t : 0 ->  c & t
            return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_AND, cond, true_value);
         }
      }
   }

   if (cond->flavour == DATAFLOW_LOGICAL_NOT) {
      return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL,
                                                cond->d.unary_op.operand,
                                                false_value,
                                                true_value);
   }

   /* TODO: consider replacing this with proper tracking of guarding predicates in back end */

   if (true_value->flavour == DATAFLOW_CONDITIONAL && true_value->d.cond_op.cond == cond)
      return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL,
                                                cond,
                                                true_value->d.cond_op.true_value,
                                                false_value);
   if (false_value->flavour == DATAFLOW_CONDITIONAL && false_value->d.cond_op.cond == cond)
      return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL,
                                                cond,
                                                true_value,
                                                false_value->d.cond_op.false_value);

   return NULL;
}

Dataflow *glsl_dataflow_simplify(Dataflow *dataflow) {
#ifndef DISABLE_CONSTANT_FOLDING
   if (dataflow->flavour == DATAFLOW_REINTERP)
      return simplify_reinterp(dataflow);
   if (dataflow->flavour == DATAFLOW_TEXTURE)
      return simplify_texture(dataflow);

   switch(dataflow->dependencies_count) {
   case 0:
      return NULL;
   case 1:
      return simplify_unary_op  (dataflow->flavour,
                                 dataflow->d.dependencies[0]);
   case 2:
      return simplify_binary_op (dataflow->flavour,
                                 dataflow->d.dependencies[0],
                                 dataflow->d.dependencies[1]);
   case 3:
      return simplify_ternary_op(dataflow->flavour,
                                 dataflow->d.dependencies[0],
                                 dataflow->d.dependencies[1],
                                 dataflow->d.dependencies[2]);
   default:
      return NULL;
   }
#endif

   return NULL;
}
