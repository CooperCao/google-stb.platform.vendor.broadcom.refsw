/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
common GL ES 1.1 and 2.0 code for
shaders as dataflow graphs and passing them to the compiler backend.
=============================================================================*/

#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/glsl_dataflow.h"
#include "middleware/khronos/glsl/glsl_backend.h"
#include "middleware/khronos/glsl/2708/glsl_qdisasm_4.h"
#include "middleware/khronos/glsl/2708/glsl_allocator_4.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/include/GLES2/gl2ext.h"
//#include "v3d/verification/sw/tools/v3d/simpenrose/simpenrose.h"
#include "middleware/khronos/glxx/2708/glxx_shader_4.h"

//static functions

static GLXX_VEC4_T * fetch_attribute(uint32_t * vpm_setup_count, int * vpm_spacew,
                                     uint32_t size, GLenum type, bool norm, uint32_t uniform_index, Dataflow **dep);

// constructors

Dataflow *glxx_cfloat(float f)
{
   return glsl_dataflow_construct_const_float(float_to_bits(f));
}

Dataflow *glxx_cint(uint32_t i)
{
   /* TODO: int/float hackiness */
   Dataflow *result = glsl_dataflow_construct_const_float(i);
   return result;
}

Dataflow *glxx_cbool(bool b)
{
   return glsl_dataflow_construct_const_bool(b ? CONST_BOOL_TRUE : CONST_BOOL_FALSE);
}

GLXX_VEC3_T *glxx_vec3(Dataflow *x, Dataflow *y, Dataflow *z)
{
   GLXX_VEC3_T *result = (GLXX_VEC3_T *)malloc_fast(sizeof(GLXX_VEC3_T));

   result->x = x;
   result->y = y;
   result->z = z;

   return result;
}

GLXX_VEC4_T *glxx_vec4(Dataflow *x, Dataflow *y, Dataflow *z, Dataflow *w)
{
   GLXX_VEC4_T *result = (GLXX_VEC4_T *)malloc_fast(sizeof(GLXX_VEC4_T));

   result->x = x;
   result->y = y;
   result->z = z;
   result->w = w;

   return result;
}

GLXX_VEC3_T *glxx_rep3(Dataflow *x)
{
   return glxx_vec3(x, x, x);
}

GLXX_VEC4_T *glxx_rep4(Dataflow *x)
{
   return glxx_vec4(x, x, x, x);
}

GLXX_VEC3_T *glxx_crep3(float f)
{
   Dataflow *c = glxx_cfloat(f);
   return glxx_vec3(c,c,c);
}

GLXX_VEC4_T *glxx_crep4(float f)
{
   return glxx_rep4(glxx_cfloat(f));
}

Dataflow *glxx_ustandard(uint32_t i)
{
   Dataflow *result;
   result = glsl_dataflow_construct_linkable_value(DATAFLOW_UNIFORM, PRIM_FLOAT);
   result->u.linkable_value.row = i;
   return result;
}

Dataflow *glxx_ustandard_int(uint32_t i)
{
   Dataflow *result;
   result = glsl_dataflow_construct_linkable_value(DATAFLOW_UNIFORM, PRIM_INT);
   result->u.linkable_value.row = i;
   return result;
}

Dataflow *glxx_u(uint32_t i, uint32_t j)
{
   return glxx_ustandard(i + j);
}

//these offsets are in bytes
GLXX_VEC3_T *glxx_u3(uint32_t i)
{
   return glxx_vec3(glxx_u(i, 0), glxx_u(i, 4), glxx_u(i, 8));
}

GLXX_VEC3_T *glxx_u3t(uint32_t i)//transpose of glxx_u3
{
   return glxx_vec3(glxx_u(i, 0), glxx_u(i, 16), glxx_u(i, 32));
}

GLXX_VEC4_T *glxx_u4(uint32_t i)
{
   return glxx_vec4(glxx_u(i, 0), glxx_u(i, 4), glxx_u(i, 8), glxx_u(i, 12));
}

GLXX_VEC4_T *glxx_u4t(uint32_t i)//transpose of glxx_u4
{
   return glxx_vec4(glxx_u(i, 0), glxx_u(i, 16), glxx_u(i, 32), glxx_u(i, 48));
}

Dataflow *glxx_fetch_vpm(Dataflow *prev)
{
   Dataflow *result;
   result = glsl_dataflow_construct_linkable_value(DATAFLOW_ATTRIBUTE, PRIM_FLOAT);
   result->u.linkable_value.row = 0;

   glsl_dataflow_add_iodependent(prev, result);
   glsl_dataflow_add_iodependency(result, prev);

   return result;
}

Dataflow *vertex_set(Dataflow *param, DataflowFlavour flavour, Dataflow *dep)
{
   return glsl_dataflow_construct_vertex_set(flavour, param, dep);
}

Dataflow *glxx_vertex_set(Dataflow *param, DataflowFlavour flavour)
{
   return vertex_set(param, flavour, NULL);
}

Dataflow *glxx_vpmw(Dataflow *param, Dataflow *dep)
{
   return vertex_set(param, DATAFLOW_VERTEX_SET, dep);
}

void glxx_iodep(Dataflow *consumer, Dataflow *supplier)
{
   if (consumer != NULL && supplier != NULL)
   {
      glsl_dataflow_add_iodependent(supplier, consumer);
      glsl_dataflow_add_iodependency(consumer, supplier);
   }
}

Dataflow *glxx_vary(uint32_t row, Dataflow *dep)
{
   Dataflow *result;

   result = glsl_dataflow_construct_linkable_value(DATAFLOW_VARYING, PRIM_FLOAT);
   result->u.linkable_value.row = row;
   result = glsl_dataflow_construct_varying_tree(result);
   glxx_iodep(result, dep);

   return result;
}

Dataflow *glxx_vary_non_perspective(uint32_t row, Dataflow *dep)
{
   Dataflow *result;

   result = glsl_dataflow_construct_linkable_value(DATAFLOW_VARYING, PRIM_FLOAT);
   result->u.linkable_value.row = row;
   result = glsl_dataflow_construct_varying_non_perspective_tree(result);
   glxx_iodep(result, dep);

   return result;
}

Dataflow *glxx_get_col(Dataflow *prev)
{
   Dataflow *result = glsl_dataflow_construct_fragment_get(DATAFLOW_FRAG_GET_COL);
   if (prev != NULL)
   {
      glsl_dataflow_add_iodependent(prev, result);
      glsl_dataflow_add_iodependency(result, prev);
   }
   return result;
}

Dataflow *glxx_fragment_get(DataflowFlavour flavour)
{
   return glsl_dataflow_construct_fragment_get(flavour);
}

/* scalar -> scalar (arithmetic) */

Dataflow *glxx_mul(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_MUL, a, b);
}

Dataflow *glxx_add(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, a, b);
}

Dataflow *glxx_iadd(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_INTEGER_ADD, a, b);
}

Dataflow *glxx_abs(Dataflow *a)
{
   //max(-min(0,a),a)
   return glxx_fmax(glxx_mul(glxx_cfloat(-1.0f),glxx_fmin(glxx_cfloat(0.0f),a)),a);
}

Dataflow *glxx_sub(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_SUB, a, b);
}

Dataflow *glxx_fmax(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_INTRINSIC_MAX, a, b);
}

Dataflow *glxx_fmin(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_INTRINSIC_MIN, a, b);
}

Dataflow *glxx_floor(Dataflow *a)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_INTRINSIC_FLOOR, a);
}

Dataflow *glxx_exp2(Dataflow *a)
{
   //TODO HW-1451 workaround?
   return glsl_dataflow_construct_unary_op(DATAFLOW_INTRINSIC_EXP2, a);
}

Dataflow *glxx_log2(Dataflow *a)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_INTRINSIC_LOG2, a);
}

/*Dataflow *glxx_pow1(Dataflow *a, Dataflow *b)
{
   return exp2(glxx_mul(b, log2(a)));
}*/

Dataflow *glxx_rsqrt(Dataflow *a)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_INTRINSIC_RSQRT, a);
}

Dataflow *glxx_recip(Dataflow *a)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_INTRINSIC_RCP, a);
}

Dataflow *glxx_sqrt1(Dataflow *a)
{
   return glxx_recip(glxx_rsqrt(a));
}

Dataflow *glxx_negate(Dataflow *a)
{
   return glxx_sub(glxx_cfloat(0.0f), a);
}

Dataflow *glxx_clamp(Dataflow *a)
{
   return glxx_fmin(glxx_fmax(a, glxx_cfloat(0.0f)), glxx_cfloat(1.0f));
}

Dataflow *glxx_square(Dataflow *a)
{
   return glxx_mul(a, a);
}

/* scalar -> scalar (logic) */

Dataflow *glxx_equal(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL, a, b);
}

Dataflow *glxx_less(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN, a, b);
}

Dataflow *glxx_lequal(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN_EQUAL, a, b);
}

Dataflow *glxx_bitnot(Dataflow *x)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_BITWISE_NOT, x);
}

Dataflow *glxx_bitand(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, a, b);
}

Dataflow *glxx_bitor(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_OR, a, b);
}

Dataflow *glxx_bitxor(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_XOR, a, b);
}

Dataflow *glxx_logicnot(Dataflow *x)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, x);
}

Dataflow *glxx_logicor(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR, a, b);
}

/* scalar -> scalar (v8) */

Dataflow *glxx_v8muld(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_V8MULD, a, b);
}

Dataflow *glxx_v8min(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_V8MIN, a, b);
}

Dataflow *glxx_v8adds(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_V8ADDS, a, b);
}

Dataflow *glxx_v8subs(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_V8SUBS, a, b);
}


/* scalar -> scalar (special) */

Dataflow *glxx_cond(Dataflow *cond, Dataflow *true_value, Dataflow *false_value)
{
   return glsl_dataflow_construct_cond_op(cond, true_value, false_value);
}

Dataflow *glxx_sum3(Dataflow *a, Dataflow *b, Dataflow *c)
{
   return glxx_add(glxx_add(a,b),c);
}

Dataflow *glxx_sum4(Dataflow *a, Dataflow *b, Dataflow *c, Dataflow *d)
{
   return glxx_add(glxx_add(glxx_add(a,b),c),d);
}

/* vector -> scalar */

Dataflow *glxx_dot3(GLXX_VEC3_T *a, GLXX_VEC3_T *b)
{
   return glxx_sum3(glxx_mul(a->x, b->x), glxx_mul(a->y, b->y), glxx_mul(a->z, b->z));
}

Dataflow *glxx_dot4(GLXX_VEC4_T *a, GLXX_VEC4_T *b)
{
   return glxx_sum4(glxx_mul(a->x, b->x), glxx_mul(a->y, b->y), glxx_mul(a->z, b->z), glxx_mul(a->w, b->w));
}

/* vector -> vector */

GLXX_VEC3_T *glxx_add3(GLXX_VEC3_T *a, GLXX_VEC3_T *b)
{
   return glxx_vec3(glxx_add(a->x, b->x), glxx_add(a->y, b->y), glxx_add(a->z, b->z));
}

GLXX_VEC4_T *glxx_add4(GLXX_VEC4_T *a, GLXX_VEC4_T *b)
{
   return glxx_vec4(glxx_add(a->x, b->x), glxx_add(a->y, b->y), glxx_add(a->z, b->z), glxx_add(a->w, b->w));
}

GLXX_VEC3_T *glxx_sub3(GLXX_VEC3_T *a, GLXX_VEC3_T *b)
{
   return glxx_vec3(glxx_sub(a->x, b->x), glxx_sub(a->y, b->y), glxx_sub(a->z, b->z));
}

GLXX_VEC4_T *glxx_sub4(GLXX_VEC4_T *a, GLXX_VEC4_T *b)
{
   return glxx_vec4(glxx_sub(a->x, b->x), glxx_sub(a->y, b->y), glxx_sub(a->z, b->z), glxx_sub(a->w, b->w));
}

GLXX_VEC3_T *glxx_mul3(GLXX_VEC3_T *a, GLXX_VEC3_T *b)
{
   return glxx_vec3(glxx_mul(a->x, b->x), glxx_mul(a->y, b->y), glxx_mul(a->z, b->z));
}

GLXX_VEC4_T *glxx_mul4(GLXX_VEC4_T *a, GLXX_VEC4_T *b)
{
   return glxx_vec4(glxx_mul(a->x, b->x), glxx_mul(a->y, b->y), glxx_mul(a->z, b->z), glxx_mul(a->w, b->w));
}

GLXX_VEC3_T *glxx_scale3(Dataflow *a, GLXX_VEC3_T *b)
{
   return glxx_vec3(glxx_mul(a, b->x), glxx_mul(a, b->y), glxx_mul(a, b->z));
}

GLXX_VEC3_T *glxx_normalize3(GLXX_VEC3_T *a)
{
   return glxx_scale3(glxx_rsqrt(glxx_dot3(a, a)), a);
}

GLXX_VEC3_T *glxx_negate3(GLXX_VEC3_T *a)
{
   return glxx_vec3(glxx_negate(a->x), glxx_negate(a->y), glxx_negate(a->z));
}

GLXX_VEC4_T *glxx_clamp4(GLXX_VEC4_T *a)
{
   return glxx_vec4(glxx_clamp(a->x), glxx_clamp(a->y), glxx_clamp(a->z), glxx_clamp(a->w));
}

GLXX_VEC3_T *glxx_dehomogenize(GLXX_VEC4_T *a)
{
   Dataflow *rw = glxx_recip(a->w);
   return glxx_vec3(glxx_mul(a->x, rw), glxx_mul(a->y, rw), glxx_mul(a->z, rw));
}

GLXX_VEC3_T *glxx_dropw(GLXX_VEC4_T *a)
{
   return glxx_vec3(a->x, a->y, a->z);
}

GLXX_VEC4_T *glxx_nullw(GLXX_VEC3_T *a)
{
   return glxx_vec4(a->x, a->y, a->z, NULL);
}

GLXX_VEC4_T *glxx_interp4(GLXX_VEC4_T *a, GLXX_VEC4_T *b, GLXX_VEC4_T *c)
{
   return glxx_add4(glxx_mul4(a, glxx_sub4(glxx_crep4(1.0f), c)), glxx_mul4(b, c));
}

GLXX_VEC4_T *glxx_blend4(GLXX_VEC4_T *a, GLXX_VEC4_T *b, Dataflow *c)
{
   return glxx_interp4(a, b, glxx_rep4(c));
}

GLXX_VEC3_T *glxx_blend3(GLXX_VEC3_T *a, GLXX_VEC3_T *b, Dataflow *c)
{
   return glxx_add3(glxx_scale3(glxx_sub(glxx_cfloat(1.0f), c), a), glxx_scale3(c, b));
}

GLXX_VEC4_T *glxx_cond4(Dataflow *c, GLXX_VEC4_T *t, GLXX_VEC4_T *f)
{
   return glxx_vec4(glxx_cond(c,t->x,f->x), glxx_cond(c,t->y,f->y), glxx_cond(c,t->z,f->z), glxx_cond(c,t->w,f->w));
}

/* matrix */

GLXX_VEC4_T *glxx_transform4x4(GLXX_VEC4_T *row0, GLXX_VEC4_T *row1, GLXX_VEC4_T *row2, GLXX_VEC4_T *row3, GLXX_VEC4_T *v)
{
   return glxx_vec4(glxx_dot4(row0, v), glxx_dot4(row1, v), glxx_dot4(row2, v), glxx_dot4(row3, v));
}

GLXX_VEC3_T *glxx_transform3x3(GLXX_VEC3_T *row0, GLXX_VEC3_T *row1, GLXX_VEC3_T *row2, GLXX_VEC3_T *v)
{
   return glxx_vec3(glxx_dot3(row0, v), glxx_dot3(row1, v), glxx_dot3(row2, v));
}

Dataflow *glxx_accu_recip(Dataflow *x)
{
   Dataflow *recip_x = glxx_recip(x);
   return glxx_mul(recip_x, glxx_sub(glxx_cfloat(2.0f), glxx_mul(x, recip_x)));
}

Dataflow *glxx_accu_rsqrt(Dataflow *x)
{
/*  y' = (3 - yyx)y / 2  */
   Dataflow *y = glxx_rsqrt(x);
   return glxx_mul(glxx_sub(glxx_cfloat(3.0f),glxx_mul(glxx_mul(y,y),x)), glxx_mul(y, glxx_cfloat(0.5f)));
   //return rsqrt(x);
}

Dataflow *glxx_fix_exp2(Dataflow *x)
{
   return glxx_exp2(x);
}

Dataflow *glxx_accu_pow(Dataflow *a, Dataflow *b)
{
   //return fix_exp2(glxx_mul(b, log2(a)));
   return glxx_fix_exp2(glxx_mul(b, glxx_mul(glxx_log2(glxx_square(glxx_square(glxx_square(a)))), glxx_cfloat(0.125f))));
   //return exp2(glxx_mul(b,log2(a)));
   //return pow1(a, b);
}

void glxx_iodep3(GLXX_VEC3_T *consumer, GLXX_VEC3_T *producer)
{
   glxx_iodep(consumer->x, producer->x); glxx_iodep(consumer->x, producer->y); glxx_iodep(consumer->x, producer->z);
   glxx_iodep(consumer->y, producer->x); glxx_iodep(consumer->y, producer->y); glxx_iodep(consumer->y, producer->z);
   glxx_iodep(consumer->z, producer->x); glxx_iodep(consumer->z, producer->y); glxx_iodep(consumer->z, producer->z);
}

void glxx_iodep4_3(GLXX_VEC4_T *consumer, GLXX_VEC3_T *producer)
{
   glxx_iodep(consumer->x, producer->x); glxx_iodep(consumer->x, producer->y); glxx_iodep(consumer->x, producer->z);
   glxx_iodep(consumer->y, producer->x); glxx_iodep(consumer->y, producer->y); glxx_iodep(consumer->y, producer->z);
   glxx_iodep(consumer->z, producer->x); glxx_iodep(consumer->z, producer->y); glxx_iodep(consumer->z, producer->z);
   glxx_iodep(consumer->w, producer->x); glxx_iodep(consumer->w, producer->y); glxx_iodep(consumer->w, producer->z);
}

void glxx_iodep3_4(GLXX_VEC3_T *consumer, GLXX_VEC4_T *producer)
{
   glxx_iodep(consumer->x, producer->x); glxx_iodep(consumer->x, producer->y); glxx_iodep(consumer->x, producer->z); glxx_iodep(consumer->x, producer->w);
   glxx_iodep(consumer->y, producer->x); glxx_iodep(consumer->y, producer->y); glxx_iodep(consumer->y, producer->z); glxx_iodep(consumer->y, producer->w);
   glxx_iodep(consumer->z, producer->x); glxx_iodep(consumer->z, producer->y); glxx_iodep(consumer->z, producer->z); glxx_iodep(consumer->z, producer->w);
}

Dataflow *glxx_unpack(Dataflow *x, DataflowFlavour flavour)
{
   return glsl_dataflow_construct_unpack(flavour, x);
}

Dataflow *glxx_i_to_f(Dataflow *i)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_ITOF, i);
}

Dataflow *glxx_x_to_f(Dataflow *x)
{
   return glxx_mul(glxx_i_to_f(x), glxx_cfloat(1.0f/65536.0f));
}

Dataflow *glxx_h_to_f(Dataflow *h, uint32_t i)
{
   return glxx_unpack(h, i ? DATAFLOW_UNPACK_16B_F : DATAFLOW_UNPACK_16A_F);
}

Dataflow *glxx_s_to_f(Dataflow *s, uint32_t i, bool norm)
{
   Dataflow *result;
   result = glxx_i_to_f(glxx_unpack(s, i ? DATAFLOW_UNPACK_16B : DATAFLOW_UNPACK_16A));
   if (norm) result = glxx_mul(result, glxx_cfloat(1.0f / 32768.0f));    //TODO: nudge?
   return result;
}

Dataflow *glxx_us_to_f(Dataflow *s, uint32_t i, bool norm)
{
   Dataflow *result;

   if (i)
      result = glxx_i_to_f(glxx_bitand(glxx_unpack(s, DATAFLOW_UNPACK_16B), glxx_cint(0xffff)));
   else
      result = glxx_i_to_f(glxx_bitand(s, glxx_cint(0xffff)));

   if (norm) result = glxx_mul(result, glxx_cfloat(1.0f / 65535.0f));
   return result;
}

/* Assumes input has already been xor'd with 0x80808080 */
Dataflow *glxx_b_to_f_xor(Dataflow *bxor, uint32_t i, bool norm)
{
   Dataflow *result;
   result = glxx_ub_to_f(bxor, i, norm);
   if (norm)
   {
      /* Map interval [0,1] to [-1,1] */
      return glxx_add(glxx_cfloat(-1.0f), glxx_mul(glxx_cfloat(2.0f), result));
   }
   else
   {
      /* Map interval [0,255] to [-128,127] */
      return glxx_add(glxx_cfloat(-128.0f), result);
   }
}

Dataflow *glxx_ub_to_f(Dataflow *ub, uint32_t i, bool norm)
{
   if (norm)
   {
      DataflowFlavour flavours[4] = {DATAFLOW_UNPACK_COL_R, DATAFLOW_UNPACK_COL_G, DATAFLOW_UNPACK_COL_B, DATAFLOW_UNPACK_COL_A};
      return glxx_unpack(ub, flavours[i]);
   }
   else
   {
      DataflowFlavour flavours[4] = {DATAFLOW_UNPACK_8A, DATAFLOW_UNPACK_8B, DATAFLOW_UNPACK_8C, DATAFLOW_UNPACK_8D};
      return glxx_i_to_f(glxx_unpack(ub, flavours[i]));
   }
}

Dataflow *glxx_f_to_i(Dataflow *f)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_FTOI_TRUNC, f);
}

static uint32_t calc_vpm_offset(uint32_t size, GLenum type)
{
   switch (type)
   {
   case GL_FLOAT:
   case GL_FIXED:
      return size;
   case GL_HALF_FLOAT_OES:
   case GL_SHORT:
   case GL_UNSIGNED_SHORT:
      return (size+1)/2;
   case GL_BYTE:
   case GL_UNSIGNED_BYTE:
      return 1;
   default:
      UNREACHABLE();
      return 0;
   }
}

static GLXX_VEC4_T * fetch_attribute(uint32_t * vpm_setup_count, int * vpm_space, uint32_t size, GLenum type, bool norm, uint32_t uniform_index, Dataflow **dep)
{
   GLXX_VEC4_T *result;
   Dataflow *vpm;
   uint32_t offset = 0;

   if (type == 0)
   {
      return glxx_u4(uniform_index);
   }

   vcos_assert(size >= 1 && size <= 4);
   result = glxx_vec4(glxx_cfloat(0.0f), glxx_cfloat(0.0f), glxx_cfloat(0.0f), glxx_cfloat(1.0f));

   vpm = *dep;

   //calc new offset
   offset = calc_vpm_offset(size,type);
   *vpm_space -= offset;
   if(*vpm_space<0)
   {
      //read everything from the vpm
      vcos_assert(*vpm_setup_count <= 3);
      *vpm_space = 15 - offset;
      //do another VPM_read_setup
      vpm = vertex_set((glxx_ustandard(BACKEND_UNIFORM_VPM_READ_SETUP + *vpm_setup_count)), DATAFLOW_VPM_READ_SETUP, vpm);
      *vpm_setup_count += 1;
   }
   vcos_assert(vpm!=NULL);

   switch (type)
   {
   case GL_FLOAT:
      if (size >= 1) { vpm = glxx_fetch_vpm(vpm); result->x = vpm; }
      if (size >= 2) { vpm = glxx_fetch_vpm(vpm); result->y = vpm; }
      if (size >= 3) { vpm = glxx_fetch_vpm(vpm); result->z = vpm; }
      if (size >= 4) { vpm = glxx_fetch_vpm(vpm); result->w = vpm; }
      break;
   case GL_FIXED:
      /* TODO: norm is ignored? */
      if (size >= 1) { vpm = glxx_fetch_vpm(vpm); result->x = glxx_x_to_f(vpm); }
      if (size >= 2) { vpm = glxx_fetch_vpm(vpm); result->y = glxx_x_to_f(vpm); }
      if (size >= 3) { vpm = glxx_fetch_vpm(vpm); result->z = glxx_x_to_f(vpm); }
      if (size >= 4) { vpm = glxx_fetch_vpm(vpm); result->w = glxx_x_to_f(vpm); }
      break;
   case GL_HALF_FLOAT_OES:
      #ifdef BIG_ENDIAN_CPU
         if (size >= 1) { vpm = glxx_fetch_vpm(vpm); result->x = glxx_h_to_f(vpm, 1); }
         if (size >= 2) {                       result->y = glxx_h_to_f(vpm, 0); }
         if (size >= 3) { vpm = glxx_fetch_vpm(vpm); result->z = glxx_h_to_f(vpm, 1); }
         if (size >= 4) {                       result->w = glxx_h_to_f(vpm, 0); }
      #else
      if (size >= 1) { vpm = glxx_fetch_vpm(vpm); result->x = glxx_h_to_f(vpm, 0); }
      if (size >= 2) {                       result->y = glxx_h_to_f(vpm, 1); }
      if (size >= 3) { vpm = glxx_fetch_vpm(vpm); result->z = glxx_h_to_f(vpm, 0); }
      if (size >= 4) {                       result->w = glxx_h_to_f(vpm, 1); }
      #endif
      break;
   case GL_SHORT:
      #ifdef BIG_ENDIAN_CPU
         if (size >= 1) { vpm = glxx_fetch_vpm(vpm); result->x = glxx_s_to_f(vpm, 1, norm); }
         if (size >= 2) {                       result->y = glxx_s_to_f(vpm, 0, norm); }
         if (size >= 3) { vpm = glxx_fetch_vpm(vpm); result->z = glxx_s_to_f(vpm, 1, norm); }
         if (size >= 4) {                       result->w = glxx_s_to_f(vpm, 0, norm); }
      #else
      if (size >= 1) { vpm = glxx_fetch_vpm(vpm); result->x = glxx_s_to_f(vpm, 0, norm); }
      if (size >= 2) {                       result->y = glxx_s_to_f(vpm, 1, norm); }
      if (size >= 3) { vpm = glxx_fetch_vpm(vpm); result->z = glxx_s_to_f(vpm, 0, norm); }
      if (size >= 4) {                       result->w = glxx_s_to_f(vpm, 1, norm); }
      #endif
      break;
   case GL_UNSIGNED_SHORT:
      #ifdef BIG_ENDIAN_CPU
         if (size >= 1) { vpm = glxx_fetch_vpm(vpm); result->x = glxx_us_to_f(vpm, 1, norm); }
         if (size >= 2) {                       result->y = glxx_us_to_f(vpm, 0, norm); }
         if (size >= 3) { vpm = glxx_fetch_vpm(vpm); result->z = glxx_us_to_f(vpm, 1, norm); }
         if (size >= 4) {                       result->w = glxx_us_to_f(vpm, 0, norm); }
      #else
      if (size >= 1) { vpm = glxx_fetch_vpm(vpm); result->x = glxx_us_to_f(vpm, 0, norm); }
      if (size >= 2) {                       result->y = glxx_us_to_f(vpm, 1, norm); }
      if (size >= 3) { vpm = glxx_fetch_vpm(vpm); result->z = glxx_us_to_f(vpm, 0, norm); }
      if (size >= 4) {                       result->w = glxx_us_to_f(vpm, 1, norm); }
      #endif
      break;
   case GL_BYTE:
      vpm = glxx_bitxor(glxx_cint(0x80808080), glxx_fetch_vpm(vpm));
      #ifdef BIG_ENDIAN_CPU
         if (size >= 1) {                       result->x = glxx_b_to_f_xor(vpm, 3, norm); }
         if (size >= 2) {                       result->y = glxx_b_to_f_xor(vpm, 2, norm); }
         if (size >= 3) {                       result->z = glxx_b_to_f_xor(vpm, 1, norm); }
         if (size >= 4) {                       result->w = glxx_b_to_f_xor(vpm, 0, norm); }
      #else
         if (size >= 1) {                       result->x = glxx_b_to_f_xor(vpm, 0, norm); }
         if (size >= 2) {                       result->y = glxx_b_to_f_xor(vpm, 1, norm); }
         if (size >= 3) {                       result->z = glxx_b_to_f_xor(vpm, 2, norm); }
         if (size >= 4) {                       result->w = glxx_b_to_f_xor(vpm, 3, norm); }
      #endif
      break;
   case GL_UNSIGNED_BYTE:
      #ifdef BIG_ENDIAN_CPU
         if (size >= 1) { vpm = glxx_fetch_vpm(vpm); result->x = glxx_ub_to_f(vpm, 3, norm); }
         if (size >= 2) {                       result->y = glxx_ub_to_f(vpm, 2, norm); }
         if (size >= 3) {                       result->z = glxx_ub_to_f(vpm, 1, norm); }
         if (size >= 4) {                       result->w = glxx_ub_to_f(vpm, 0, norm); }
      #else
      if (size >= 1) { vpm = glxx_fetch_vpm(vpm); result->x = glxx_ub_to_f(vpm, 0, norm); }
      if (size >= 2) {                       result->y = glxx_ub_to_f(vpm, 1, norm); }
      if (size >= 3) {                       result->z = glxx_ub_to_f(vpm, 2, norm); }
      if (size >= 4) {                       result->w = glxx_ub_to_f(vpm, 3, norm); }
      #endif
      break;
   default:
      UNREACHABLE();
   }
   *dep = vpm;
   return result;
}

Dataflow *glxx_fetch_all_attributes(
   GLXX_ATTRIB_ABSTRACT_T *abstract,
   uint32_t *uniform_index,
   Dataflow **attrib,
   Dataflow *dep,
   uint32_t attribs_live,
   uint32_t *ordering)
{
   uint32_t i,j;
   uint32_t vpm_setup_count = 0;
   int vpm_space = 0;
   //printf("glxx_fetch_all_attributes ");
   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      if(attribs_live & 15<<(4*i) && abstract[i].type == 0)
      {  //live but not enabled, so get from uniform
         attrib[4*i] = glxx_u(uniform_index[i], 0);
         attrib[4*i+1] = glxx_u(uniform_index[i], 4);
         attrib[4*i+2] = glxx_u(uniform_index[i], 8);
         attrib[4*i+3] = glxx_u(uniform_index[i], 12);
      }
      else
         attrib[4*i] = attrib[4*i+1] = attrib[4*i+2] = attrib[4*i+3] = NULL;
   }

   for (j = 0; j < GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2; j++)
   {
      i = ordering[j];
      if(i!=(uint32_t)~0)
      {
         GLXX_VEC4_T *vec;
         //live and enabled
         vcos_assert(attribs_live & 15<<(4*i) && abstract[i].type != 0);
         //printf("i%d: s%d,t%d,n%d,u%d\n",i,abstract[i].size, abstract[i].type, abstract[i].norm, uniform_index[i]);
         vec = fetch_attribute(&vpm_setup_count,&vpm_space, abstract[i].size, abstract[i].type, abstract[i].norm, uniform_index[i], &dep);
         attrib[4*i] = vec->x;
         attrib[4*i+1] = vec->y;
         attrib[4*i+2] = vec->z;
         attrib[4*i+3] = vec->w;
      }
   }
   //printf("\n");

   return dep;
}


static Dataflow *blend_main(GLenum src_function, GLenum dst_function, GLenum equation, Dataflow *dst, Dataflow *src, bool render_alpha)
{
   uint32_t j;
   Dataflow *col;
   Dataflow *result[2];

   col = glxx_ustandard_int(BACKEND_UNIFORM_BLEND_COLOR);

   // Optimization for GL_ZERO, GL_ZERO
   if (dst_function == GL_ZERO && src_function == GL_ZERO)
      return glxx_cint(0);

   for (j = 0; j < 2; j++)
   {
      GLenum function;
      Dataflow *tmp;

      if (j)
         function = dst_function;
      else
         function = src_function;

      switch (function)
      {
      case GL_ZERO:
         tmp = glxx_cint(0);
         result[j] = tmp;
         break;
      case GL_ONE:
         tmp = glxx_cint(0xffffffff);
         result[j] = j ? dst : src;
         break;
      case GL_SRC_COLOR:
         tmp = src;
         break;
      case GL_ONE_MINUS_SRC_COLOR:
         tmp = glxx_bitnot(src);
         break;
      case GL_DST_COLOR:
         tmp = dst;
         break;
      case GL_ONE_MINUS_DST_COLOR:
         tmp = glxx_bitnot(dst);
         break;
      case GL_SRC_ALPHA:
         tmp = glxx_unpack(src, DATAFLOW_UNPACK_8R);
         break;
      case GL_ONE_MINUS_SRC_ALPHA:
         tmp = glxx_bitnot(glxx_unpack(src, DATAFLOW_UNPACK_8R));
         break;
      case GL_DST_ALPHA:
         if (render_alpha)
            tmp = glxx_unpack(dst, DATAFLOW_UNPACK_8R);
         else
            tmp = glxx_cint(0xffffffff);
         break;
      case GL_ONE_MINUS_DST_ALPHA:
         if (render_alpha)
            tmp = glxx_bitnot(glxx_unpack(dst, DATAFLOW_UNPACK_8R));
         else
            tmp = glxx_cint(0x00000000);
         break;
      case GL_CONSTANT_COLOR:
         tmp = col;
         break;
      case GL_ONE_MINUS_CONSTANT_COLOR:
         tmp = glxx_bitnot(col);
         break;
      case GL_CONSTANT_ALPHA:
         tmp = glxx_unpack(col, DATAFLOW_UNPACK_8R);
         break;
      case GL_ONE_MINUS_CONSTANT_ALPHA:
         tmp = glxx_bitnot(glxx_unpack(col, DATAFLOW_UNPACK_8R));
         break;
      case GL_SRC_ALPHA_SATURATE:
         if (render_alpha)
            tmp = glxx_bitor(glxx_cint(0xff000000), glxx_v8min(glxx_unpack(src, DATAFLOW_UNPACK_8R), glxx_bitnot(glxx_unpack(dst, DATAFLOW_UNPACK_8R))));
         else
            tmp = glxx_cint(0xff000000);
         break;
      default:
         UNREACHABLE();
         return 0;
      }

      if ((function != GL_ZERO) && (function != GL_ONE))
      {
         result[j] = glxx_v8muld(tmp, j ? dst : src);
      }
   }

   switch (equation)
   {
   case GL_FUNC_ADD:
      return glxx_v8adds(result[0], result[1]);
   case GL_FUNC_SUBTRACT:
      return glxx_v8subs(result[0], result[1]);
   case GL_FUNC_REVERSE_SUBTRACT:
      return glxx_v8subs(result[1], result[0]);
   default:
      UNREACHABLE();
      return 0;
   }
}

Dataflow *glxx_perform_blend(GLXX_HW_BLEND_T blend, Dataflow *dst, Dataflow *src, bool render_alpha)
{
   if (!render_alpha || (
      blend.equation == blend.equation_alpha &&
      blend.src_function == blend.src_function_alpha &&
      blend.dst_function == blend.dst_function_alpha))
   {
      return blend_main(blend.src_function, blend.dst_function, blend.equation, dst, src, render_alpha);
   }
   else
   {
      Dataflow *rgb, *a;

      rgb = blend_main(blend.src_function, blend.dst_function, blend.equation, dst, src, render_alpha);
      a = blend_main(blend.src_function_alpha, blend.dst_function_alpha, blend.equation_alpha, dst, src, render_alpha);
      return glxx_bitor(glxx_bitand(rgb, glxx_cint(0x00ffffff)), glxx_bitand(a, glxx_cint(0xff000000)));
   }
}

Dataflow *glxx_perform_logic_op(GLenum operation, Dataflow *s, Dataflow *d)
{
   switch (operation)
   {
   case GL_CLEAR:
      return glxx_cint(0);
   case GL_AND:
      return glxx_bitand(s, d);
   case GL_AND_REVERSE:
      return glxx_bitand(s, glxx_bitnot(d));
   case GL_COPY:
      return s;
   case GL_AND_INVERTED:
      return glxx_bitand(glxx_bitnot(s), d);
   case GL_NOOP:
      return d;
   case GL_XOR:
      return glxx_bitxor(s, d);
   case GL_OR:
      return glxx_bitor(s, d);
   case GL_NOR:
      return glxx_bitnot(glxx_bitor(s, d));
   case GL_EQUIV:
      return glxx_bitnot(glxx_bitxor(s, d));
   case GL_INVERT:
      return glxx_bitnot(d);
   case GL_OR_REVERSE:
      return glxx_bitor(s, glxx_bitnot(d));
   case GL_COPY_INVERTED:
      return glxx_bitnot(s);
   case GL_OR_INVERTED:
      return glxx_bitor(glxx_bitnot(s), d);
   case GL_NAND:
      return glxx_bitnot(glxx_bitand(s, d));
   case GL_SET:
      return glxx_cint(0xffffffff);
   default:
      UNREACHABLE();
      return NULL;
   }
}

Dataflow *glxx_perform_color_mask(uint32_t mask, Dataflow *s, Dataflow *d)
{
   return glxx_bitor(glxx_bitand(s, glxx_cint(mask)), glxx_bitand(d, glxx_cint(~mask)));
}

Dataflow *glxx_shr(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_SHIFT_RIGHT, a, b);
}

Dataflow *glxx_888_to_565(Dataflow *d)
{
   Dataflow *s5 = glxx_bitand(glxx_shr(d, glxx_cint(5)), glxx_cint(0x00070007));
   Dataflow *s6 = glxx_bitand(glxx_shr(d, glxx_cint(6)), glxx_cint(0x00000300));

   d = glxx_v8subs(d, s5);
   d = glxx_v8subs(d, s6);
   d = glxx_v8adds(d, glxx_cint(0x00040204));

   return glxx_bitand(d, glxx_cint(0x00f8fcf8));
}

Dataflow *glxx_565_to_888(Dataflow *d)
{
   return glxx_bitor(glxx_bitand(d, glxx_cint(0x00f8fcf8)), glxx_cint(0x00030103));
}

Dataflow *glxx_alpha_to_one(Dataflow *d)
{
   return glsl_dataflow_construct_pack(DATAFLOW_PACK_COL_A, glxx_cfloat(1.0f), d);
   //return glxx_bitor(d, glxx_cint(0xff000000));
}

#define COVERAGE_PATTERN_PIXEL_DIM 2

static Dataflow * alpha_coverage_pattern(Dataflow * alpha, Dataflow * index)
{
#if COVERAGE_PATTERN_PIXEL_DIM == 1
   /*a 2x2 ordered dither (see wikipedia)
      1/5 [ 1 3 ]
          [ 4 2 ] */

   vcos_assert(COVERAGE_PATTERN_PIXEL_DIM == 1);
   return glxx_less(glxx_mul(alpha,glxx_cfloat(5.0f)),
         glxx_cond(glxx_less(index,glxx_cfloat(1.0f)),glxx_cfloat(1.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(2.0f)),glxx_cfloat(3.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(3.0f)),glxx_cfloat(4.0f),
                                                      glxx_cfloat(2.0f)))));
#else

   /* a 4x4 ordered dither
      1/17 [1  9  3 11]
           [13 5 15  7]
           [4 12  2 10]
           [16 8 14  6]
   */
   vcos_assert(COVERAGE_PATTERN_PIXEL_DIM == 2);
   return glxx_less(glxx_mul(alpha,glxx_cfloat(17.0f)),
         glxx_cond(glxx_less(index,glxx_cfloat(1.0f)),glxx_cfloat(1.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(2.0f)),glxx_cfloat(9.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(3.0f)),glxx_cfloat(3.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(4.0f)),glxx_cfloat(11.0f),

         glxx_cond(glxx_less(index,glxx_cfloat(5.0f)),glxx_cfloat(13.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(6.0f)),glxx_cfloat(5.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(7.0f)),glxx_cfloat(15.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(8.0f)),glxx_cfloat(7.0f),

         glxx_cond(glxx_less(index,glxx_cfloat(9.0f)),glxx_cfloat(4.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(10.0f)),glxx_cfloat(12.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(11.0f)),glxx_cfloat(2.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(12.0f)),glxx_cfloat(10.0f),

         glxx_cond(glxx_less(index,glxx_cfloat(13.0f)),glxx_cfloat(16.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(14.0f)),glxx_cfloat(8.0f),
         glxx_cond(glxx_less(index,glxx_cfloat(15.0f)),glxx_cfloat(14.0f),
                                                      glxx_cfloat(6.0f)))))))))))))))));
#endif
}

static Dataflow * alpha_to_coverage(Dataflow * dst, Dataflow * src, int sub_pixel, Dataflow * coverage)
{
   Dataflow *dim, *alpha, *x, *y, *off;
   dim = glxx_cfloat((float)COVERAGE_PATTERN_PIXEL_DIM);
   alpha = coverage==NULL ? glxx_unpack(src, DATAFLOW_UNPACK_COL_A) : coverage;
   x = glsl_dataflow_construct_fragment_get(DATAFLOW_FRAG_GET_X);
   y = glsl_dataflow_construct_fragment_get(DATAFLOW_FRAG_GET_Y);
   x = glxx_bitand(x,glxx_cint(COVERAGE_PATTERN_PIXEL_DIM -1)); /* x = x % dim */
   y = glxx_bitand(y,glxx_cint(COVERAGE_PATTERN_PIXEL_DIM -1));

   /* off  = sub_pixel + 4*(x + dim*y) */
   off = glxx_add(glxx_cfloat((float)sub_pixel),
                  glxx_mul(glxx_cfloat(4.0f),
                           glxx_add(glxx_i_to_f(x),
                              glxx_mul(dim,glxx_i_to_f(y)))));

   return glxx_cond(alpha_coverage_pattern(alpha,off), dst, src);
}

static bool is_logic_nop(GLenum op) {
   return (op == 0 || op == GL_COPY);
}

static uint8_t col_to_byte(float in) {
   float out = in*255.0f;
   if (out < 0.0f) return 0;
   if (out > 255.0f) return 255;
   return (uint8_t)out;
}

Dataflow *glxx_backend(GLXX_HW_BLEND_T blend, GLXX_VEC4_T *color, Dataflow *discard,
                       unsigned int stencil_config, bool use_depth, bool render_alpha,
                       bool rgb565, bool fb_rb_swap)
{
   bool use_stencil = (bool)(stencil_config & 0x1);
   Dataflow *sbwait;
   Dataflow *data_z = NULL;
   Dataflow *result;
   Dataflow *packed;
   Dataflow *stencil = NULL;
   uint32_t color_mask = blend.color_mask;

   /* Arrange the colour channels accoring to the framebuffer byte ordering. Either RGBA or (if fb_rb_swap) BGRA */
   Dataflow *fb_8a = fb_rb_swap ? color->z : color->x;
   Dataflow *fb_8b = color->y;
   Dataflow *fb_8c = fb_rb_swap ? color->x : color->z;
   Dataflow *fb_8d = color->w;

   if (fb_rb_swap)
      color_mask = (color_mask & 0xff00ff00) | ((color_mask & 0xff0000) >> 16) | ((color_mask & 0xff) << 16);

   if (use_stencil)
   {
      stencil = glsl_dataflow_construct_fragment_submit(DATAFLOW_FRAG_SUBMIT_STENCIL, glxx_ustandard_int(BACKEND_UNIFORM_STENCIL_FRONT), stencil, NULL);
      stencil = glsl_dataflow_construct_fragment_submit(DATAFLOW_FRAG_SUBMIT_STENCIL, glxx_ustandard_int(BACKEND_UNIFORM_STENCIL_BACK), stencil, NULL);
      stencil = glsl_dataflow_construct_fragment_submit(DATAFLOW_FRAG_SUBMIT_STENCIL, glxx_ustandard_int(BACKEND_UNIFORM_STENCIL_MASK), stencil, NULL);
   }

   sbwait = glsl_dataflow_construct_scoreboard_wait(stencil);
   if (use_depth || use_stencil || (discard && !(discard->flavour == DATAFLOW_CONST_BOOL && discard->u.const_bool.value == CONST_BOOL_FALSE)))
   {
      data_z = glsl_dataflow_construct_fragment_get(DATAFLOW_FRAG_GET_Z);
      data_z = glsl_dataflow_construct_fragment_submit(DATAFLOW_FRAG_SUBMIT_Z, data_z, sbwait, discard);
   }
   else
   {
      data_z = sbwait;
   }

   if (color_mask == 0 &&
       blend.ms == 0)
   {
      /* Read the color value without changing it.
       * If you just try result = data_z; asserts can occur in generate(...) in glsl_allocator_4.c.
       * The generated code for this is the same length, and this doesn't assert, so we'll run with it.
       */
      Dataflow *read = glxx_get_col(data_z);
      result = read;
   }
   else if (!glxx_hw_blend_enabled(blend) && is_logic_nop(blend.logic_op) && color_mask == 0xffffffff
      && !blend.sample_alpha_to_coverage
      && !blend.sample_coverage)
   {
      if(render_alpha)
      {
         if ((fb_8a->flavour == DATAFLOW_CONST_FLOAT) &&
             (fb_8b->flavour == DATAFLOW_CONST_FLOAT) &&
             (fb_8c->flavour == DATAFLOW_CONST_FLOAT) &&
             (fb_8d->flavour == DATAFLOW_CONST_FLOAT))
         {
            float r, g, b, a;
            unsigned int packed_color;
            r = float_from_bits(fb_8a->u.const_float.value);
            g = float_from_bits(fb_8b->u.const_float.value);
            b = float_from_bits(fb_8c->u.const_float.value);
            a = float_from_bits(fb_8d->u.const_float.value);
            packed_color = ((uint32_t)col_to_byte(a) << 24) | ((uint32_t)col_to_byte(b) << 16) | ((uint32_t)col_to_byte(g) << 8) | col_to_byte(r);
            packed = glxx_cint((int)packed_color);
         }
         else
            packed = glsl_dataflow_construct_pack_col(fb_8a, fb_8b, fb_8c, fb_8d);
      }
      else
      {
         if ((fb_8a->flavour == DATAFLOW_CONST_FLOAT) &&
             (fb_8b->flavour == DATAFLOW_CONST_FLOAT) &&
             (fb_8c->flavour == DATAFLOW_CONST_FLOAT))
         {
            float r, g, b;
            unsigned int packed_color;
            r = float_from_bits(fb_8a->u.const_float.value);
            g = float_from_bits(fb_8b->u.const_float.value);
            b = float_from_bits(fb_8c->u.const_float.value);
            packed_color = 0xFF000000 | ((uint32_t)col_to_byte(b) << 16) | ((uint32_t)col_to_byte(g) << 8) | col_to_byte(r);
            packed = glxx_cint((int)packed_color);
         }
         else
         {
            packed = glsl_dataflow_construct_pack_col_rgb(fb_8a, fb_8b, fb_8c);
#ifdef MUST_SET_ALPHA
            /* Some platforms require a sensible alpha value, so make sure it's 1.0 in the case of XBGR mode */
            //packed = glsl_dataflow_construct_pack_col(-1, color->x, color->y, color->z, glxx_cfloat(1.0f));
            //packed = glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_OR, packed, glxx_cint((int)0xFF000000));
            packed = glxx_alpha_to_one(packed);
#endif
         }
      }
      /* Write opaque color value */
      result = glsl_dataflow_construct_fragment_submit(
         DATAFLOW_FRAG_SUBMIT_ALL,
         packed, data_z, NULL);
   }
   else
   {
      /* Perform some combination of blend, logic op and color mask */
      Dataflow *read = NULL;
      Dataflow *write = data_z;
      Dataflow *value;
      Dataflow *coverage = NULL;
      uint32_t i;
      uint32_t samples = blend.ms ? GLXX_CONFIG_SAMPLES : 1;

      if (blend.sample_coverage && blend.ms)
      {
         vcos_assert(0.0f <= blend.sample_coverage_v.value && 1.0f>=blend.sample_coverage_v.value);
         coverage = blend.sample_coverage_v.invert ? glxx_cfloat(1.0f - blend.sample_coverage_v.value) : glxx_cfloat(blend.sample_coverage_v.value);
      }

      packed = glsl_dataflow_construct_pack_col(fb_8a, fb_8b, fb_8c, fb_8d);

      for (i = 0; i < samples; i++)
      {
         DataflowFlavour target;

         target = blend.ms ? DATAFLOW_FRAG_SUBMIT_MS : DATAFLOW_FRAG_SUBMIT_ALL;

         read = glxx_get_col(read);
         if (i == 0)
            glxx_iodep(read, data_z);
         value = packed;

         if (blend.sample_alpha_to_coverage && blend.ms)
            value = alpha_to_coverage(read, value, i, NULL);

         if (blend.sample_coverage && blend.ms)
            value = alpha_to_coverage(read, value, i, coverage);

         if (glxx_hw_blend_enabled(blend))
            value = glxx_perform_blend(blend, read, value, render_alpha);

         if (!is_logic_nop(blend.logic_op))
         {
            if (rgb565)
            {
               value = glxx_565_to_888(glxx_perform_logic_op(blend.logic_op, glxx_888_to_565(value), glxx_888_to_565(read)));
            }
            else
            {
               value = glxx_perform_logic_op(blend.logic_op, value, read);
            }
         }

         if (color_mask != 0xffffffff)
            value = glxx_perform_color_mask(color_mask, value, read);

#ifdef MUST_SET_ALPHA
         /* We MUST set alpha to one if no dest alpha plane is present */
         if (!render_alpha)
            value = glxx_alpha_to_one(value);
#endif

         if (khrn_options.old_glsl_sched)
         {
            /*XXX TODO this is a hack */
            if (i == 0 && value != packed)
               glxx_iodep(value, packed);
         }

         write = glsl_dataflow_construct_fragment_submit(target, value, write, NULL);
      }
      result = write;
   }

   return result;
}

bool glxx_schedule(Dataflow *root, uint32_t type, MEM_HANDLE_T *mh_code, MEM_HANDLE_T *mh_uniform_map, bool *threaded, uint32_t *vary_map, uint32_t *vary_count)
{
   if (!khrn_options.old_glsl_sched)
   {
      if (!glsl_bcg_backend_schedule(root, type, mh_code, mh_uniform_map, threaded, vary_map, vary_count))
         return false;
      return true;
   }
   else
   {
      uint32_t code_size, uniform_map_size;
      MEM_HANDLE_T hcode, huniform_map;

      if (!glsl_backend_schedule(root, type, threaded))
         return false;

      code_size = glsl_allocator_get_shader_size();
      hcode = mem_alloc_ex(code_size, 8, MEM_FLAG_DIRECT, "shader code", MEM_COMPACT_DISCARD);

      uniform_map_size = 8 * glsl_allocator_get_unif_count();
      huniform_map = mem_alloc_ex(uniform_map_size, 4, 0, "uniform map", MEM_COMPACT_DISCARD);

      if (hcode == MEM_INVALID_HANDLE || huniform_map == MEM_INVALID_HANDLE)
      {
         if (hcode != MEM_INVALID_HANDLE) mem_release(hcode);
         if (huniform_map != MEM_INVALID_HANDLE) mem_release(huniform_map);
         return false;
      }

      MEM_ASSIGN(*mh_code, hcode);
      MEM_ASSIGN(*mh_uniform_map, huniform_map);

      khrn_memcpy(mem_lock(hcode, NULL), glsl_allocator_get_shader_pointer(), code_size);
      mem_unlock(hcode);
      mem_release(hcode);

      khrn_memcpy(mem_lock(huniform_map, NULL), glsl_allocator_get_unif_pointer(), uniform_map_size);
      mem_unlock(huniform_map);
      mem_release(huniform_map);

      if (type & GLSL_BACKEND_TYPE_FRAGMENT)
      {
         *vary_count = glsl_allocator_get_varying_count();
         khrn_memcpy(vary_map, glsl_allocator_get_varying_pointer(), 4 * glsl_allocator_get_varying_count());
         /*
         {
         uint32_t i;
         printf("================= Varying count = %d\n", glsl_allocator_get_varying_count());
         for (i = 0; i < glsl_allocator_get_varying_count(); i++)
         printf("v%2d = %x\n", i, vary_map[i]);
         }
         */
      }
      return true;
   }
}

Dataflow *glxx_vertex_backend(Dataflow *x, Dataflow *y, Dataflow *z, Dataflow *w,
                              Dataflow *point_size,
                              bool write_clip_header,
                              bool write_varyings,
                              Dataflow **vertex_vary,
                              uint32_t *vary_map,
                              uint32_t vary_count,
                              bool egl_output)
{
   Dataflow *recip_w, *temp_x0, *temp_y0, *temp_z, *xy, *dep;
   Dataflow *temp_x1, *temp_y1;
   Dataflow *plus_half, *minus_half, *zero;
   Dataflow *adjustment_x, *adjustment_y;
   uint32_t i;

   recip_w = glxx_accu_recip(w);

   plus_half = glxx_cfloat(0.5f);
   minus_half = glxx_cfloat(-0.5f);
   zero = glxx_cfloat(0.0f);

   temp_x0 = glxx_mul(glxx_mul(x, glxx_ustandard(GLXX_STATE_OFFSET(viewport.internal[9]))), recip_w);
   adjustment_x = glxx_cond(
      glxx_less(temp_x0, zero),
      minus_half, plus_half);

   temp_x1 = glxx_f_to_i(glxx_add(temp_x0, adjustment_x));
   temp_y0 = glxx_mul(glxx_mul(y, glxx_ustandard(GLXX_STATE_OFFSET(viewport.internal[10]))), recip_w);
   adjustment_y = glxx_cond(
      glxx_less(temp_y0, zero),
      minus_half, plus_half);

   if ((khrn_workarounds.FB_BOTTOM_UP || khrn_workarounds.FB_TOP_DOWN) && egl_output)
      /* scale_y needs to be pushed as an actual uniform, so it can be altered depending on the framebuffer attatchment */
      temp_y1 = glxx_f_to_i(glxx_sub(adjustment_y, temp_y0));
   else
      temp_y1 = glxx_f_to_i(glxx_add(temp_y0, adjustment_y));

   xy = glsl_dataflow_construct_pack_int16(temp_x1, temp_y1);

   temp_z = glxx_mul(glxx_mul(z, glxx_ustandard(GLXX_STATE_OFFSET(viewport.internal[4]))), recip_w);
   temp_z = glxx_add(temp_z, glxx_ustandard(GLXX_STATE_OFFSET(viewport.internal[5])));

   dep = glxx_vertex_set(glxx_ustandard(BACKEND_UNIFORM_VPM_WRITE_SETUP), DATAFLOW_VPM_WRITE_SETUP);
   if (write_clip_header)
   {
      dep = glxx_vpmw(x, dep);
      dep = glxx_vpmw(y, dep);
      dep = glxx_vpmw(z, dep);
      dep = glxx_vpmw(w, dep);

      if (point_size != NULL)
      {
         recip_w = glxx_cond(glxx_equal(w, glxx_cfloat(0.0f)),
                                       glxx_cfloat(0.0f),
                                       recip_w);
         point_size = glxx_cond(glxx_equal(w, glxx_cfloat(0.0f)),
                                       glxx_cfloat(0.0f),
                                       point_size);
      }
   }
   dep = glxx_vpmw(xy, dep);
   dep = glxx_vpmw(temp_z, dep);
   dep = glxx_vpmw(recip_w, dep);

   if (point_size != NULL)
   {
      if (khrn_workarounds.HW2903)
      {
         if (write_clip_header)  /* Only needs adjusting in coordinate shaders */
         {
            point_size = glxx_fmax(point_size, glxx_cfloat(0.125f));
         }
      }
      dep = glxx_vpmw(point_size, dep);
   }

   if (write_varyings)
   {
      for (i = 0; i < vary_count; i++)
      {
         vcos_assert(vertex_vary[vary_map[i]] != NULL);
         dep = glxx_vpmw(vertex_vary[vary_map[i]], dep);
      }
   }

   return dep;
}
