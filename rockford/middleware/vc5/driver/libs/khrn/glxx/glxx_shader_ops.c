/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
common GL ES 1.1 and 2.0 code for
shaders as dataflow graphs and passing them to the compiler backend.
=============================================================================*/

#include "../glsl/glsl_common.h"
#include "../glsl/glsl_dataflow.h"

#include "glxx_shader_ops.h"

// constructors

Dataflow *glxx_cfloat(float f)
{
   return glsl_dataflow_construct_const_float(f);
}

Dataflow *glxx_cint(uint32_t i)
{
   Dataflow *result = glsl_dataflow_construct_const_int(i);
   return result;
}

Dataflow *glxx_cbool(bool b)
{
   return glsl_dataflow_construct_const_bool(b ? CONST_BOOL_TRUE : CONST_BOOL_FALSE);
}

void glxx_v_vec3(GLXX_VEC3_T *result, Dataflow *x, Dataflow *y, Dataflow *z)
{
   result->x = x;
   result->y = y;
   result->z = z;
}

void glxx_v_vec4(GLXX_VEC4_T *result, Dataflow *x, Dataflow *y, Dataflow *z, Dataflow *w)
{
   result->x = x;
   result->y = y;
   result->z = z;
   result->w = w;
}

void glxx_v_rep3(GLXX_VEC3_T *result, Dataflow *x)
{
   glxx_v_vec3(result, x, x, x);
}

void glxx_v_rep4(GLXX_VEC4_T *result, Dataflow *x)
{
   glxx_v_vec4(result, x, x, x, x);
}

void glxx_v_crep3(GLXX_VEC3_T *result, float f)
{
   Dataflow *c = glxx_cfloat(f);
   glxx_v_vec3(result, c,c,c);
}

void glxx_v_crep4(GLXX_VEC4_T *result, float f)
{
   glxx_v_rep4(result, glxx_cfloat(f));
}

void glxx_m3_upper_left(GLXX_MAT3_T *m3, const GLXX_MAT4_T *m4) {
   glxx_v_dropw(&m3->row[0], &m4->row[0]);
   glxx_v_dropw(&m3->row[1], &m4->row[1]);
   glxx_v_dropw(&m3->row[2], &m4->row[2]);
}

void glxx_m3_transpose(GLXX_MAT3_T *m3) {
   Dataflow *tmp;
   tmp = m3->row[0].y;
   m3->row[0].y = m3->row[1].x;
   m3->row[1].x = tmp;
   tmp = m3->row[0].z;
   m3->row[0].z = m3->row[2].x;
   m3->row[2].x = tmp;
   tmp = m3->row[1].z;
   m3->row[1].z = m3->row[2].y;
   m3->row[2].y = tmp;
}

void glxx_m4_transform(GLXX_VEC4_T *result, const GLXX_MAT4_T *m, const GLXX_VEC4_T *v) {
   glxx_v_transform4x4(result, &m->row[0], &m->row[1], &m->row[2], &m->row[3], v);
}

void glxx_m3_transform(GLXX_VEC3_T *result, const GLXX_MAT3_T *m, const GLXX_VEC3_T *v) {
   glxx_v_transform3x3(result, &m->row[0], &m->row[1], &m->row[2], v);
}

Dataflow *glxx_u(uint32_t i)
{
   Dataflow *result;
   result = glsl_dataflow_construct_linkable_value(DATAFLOW_UNIFORM, DF_FLOAT);
   result->u.linkable_value.row = i;
   return result;
}

Dataflow *glxx_u_int(uint32_t i)
{
   Dataflow *result;
   result = glsl_dataflow_construct_linkable_value(DATAFLOW_UNIFORM, DF_INT);
   result->u.linkable_value.row = i;
   return result;
}

/* Offsets are in whole values, ie. words */
Dataflow *glxx_u_off(uint32_t i, uint32_t j)
{
   return glxx_u(i + j);
}

Dataflow *glxx_u_off_int(uint32_t i, uint32_t j) {
   return glxx_u_int(i + j);
}

void glxx_v_u3(GLXX_VEC3_T *result, uint32_t i)
{
   glxx_v_vec3(result, glxx_u_off(i, 0), glxx_u_off(i, 1), glxx_u_off(i, 2));
}

void glxx_v_u3t(GLXX_VEC3_T *result, uint32_t i)//transpose of glxx_v_u3
{
   glxx_v_vec3(result, glxx_u_off(i, 0), glxx_u_off(i, 4), glxx_u_off(i, 8));
}

void glxx_v_u4(GLXX_VEC4_T *result, uint32_t i)
{
   glxx_v_vec4(result, glxx_u_off(i, 0), glxx_u_off(i, 1), glxx_u_off(i, 2), glxx_u_off(i, 3));
}

void glxx_v_u4t(GLXX_VEC4_T *result, uint32_t i)//transpose of glxx_v_u4
{
   glxx_v_vec4(result, glxx_u_off(i, 0), glxx_u_off(i, 4), glxx_u_off(i, 8), glxx_u_off(i, 12));
}

void glxx_m4_u(GLXX_MAT4_T *m, int loc) {
   glxx_v_u4t(&m->row[0], loc);
   glxx_v_u4t(&m->row[1], loc + 1);
   glxx_v_u4t(&m->row[2], loc + 2);
   glxx_v_u4t(&m->row[3], loc + 3);
}

Dataflow *glxx_fragment_get(DataflowFlavour flavour)
{
   return glsl_dataflow_construct_nullary_op(flavour);
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
   return glsl_dataflow_construct_binary_op(DATAFLOW_MAX, a, b);
}

Dataflow *glxx_fmin(Dataflow *a, Dataflow *b)
{
   return glsl_dataflow_construct_binary_op(DATAFLOW_MIN, a, b);
}

Dataflow *glxx_floor(Dataflow *a)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_FLOOR, a);
}

Dataflow *glxx_exp2(Dataflow *a)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_EXP2, a);
}

Dataflow *glxx_log2(Dataflow *a)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_LOG2, a);
}

Dataflow *glxx_rsqrt(Dataflow *a)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_RSQRT, a);
}

Dataflow *glxx_recip(Dataflow *a)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_RCP, a);
}

Dataflow *glxx_sqrt1(Dataflow *a)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_SQRT, a);
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

/* scalar -> scalar (special) */

Dataflow *glxx_cond(Dataflow *cond, Dataflow *true_value, Dataflow *false_value)
{
   return glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond, true_value, false_value);
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

Dataflow *glxx_dot3(const GLXX_VEC3_T *a, const GLXX_VEC3_T *b)
{
   return glxx_sum3(glxx_mul(a->x, b->x), glxx_mul(a->y, b->y), glxx_mul(a->z, b->z));
}

Dataflow *glxx_dot4(const GLXX_VEC4_T *a, const GLXX_VEC4_T *b)
{
   return glxx_sum4(glxx_mul(a->x, b->x), glxx_mul(a->y, b->y), glxx_mul(a->z, b->z), glxx_mul(a->w, b->w));
}

/* vector -> vector */

void glxx_v_add3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b)
{
   glxx_v_vec3(result, glxx_add(a->x, b->x), glxx_add(a->y, b->y), glxx_add(a->z, b->z));
}

void glxx_v_add4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b)
{
   glxx_v_vec4(result, glxx_add(a->x, b->x), glxx_add(a->y, b->y), glxx_add(a->z, b->z), glxx_add(a->w, b->w));
}

void glxx_v_sub3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b)
{
   glxx_v_vec3(result, glxx_sub(a->x, b->x), glxx_sub(a->y, b->y), glxx_sub(a->z, b->z));
}

void glxx_v_sub4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b)
{
   glxx_v_vec4(result, glxx_sub(a->x, b->x), glxx_sub(a->y, b->y), glxx_sub(a->z, b->z), glxx_sub(a->w, b->w));
}

void glxx_v_mul3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b)
{
   glxx_v_vec3(result, glxx_mul(a->x, b->x), glxx_mul(a->y, b->y), glxx_mul(a->z, b->z));
}

void glxx_v_mul4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b)
{
   glxx_v_vec4(result, glxx_mul(a->x, b->x), glxx_mul(a->y, b->y), glxx_mul(a->z, b->z), glxx_mul(a->w, b->w));
}

void glxx_v_scale3(GLXX_VEC3_T *result, Dataflow *a, const GLXX_VEC3_T *b)
{
   glxx_v_vec3(result, glxx_mul(a, b->x), glxx_mul(a, b->y), glxx_mul(a, b->z));
}

void glxx_v_accu_normalize3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a)
{
   glxx_v_scale3(result, glxx_accu_rsqrt(glxx_dot3(a, a)), a);
}

void glxx_v_normalize3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a)
{
   glxx_v_scale3(result, glxx_rsqrt(glxx_dot3(a, a)), a);
}

void glxx_v_negate3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a)
{
   glxx_v_vec3(result, glxx_negate(a->x), glxx_negate(a->y), glxx_negate(a->z));
}

void glxx_v_clamp4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a)
{
   glxx_v_vec4(result, glxx_clamp(a->x), glxx_clamp(a->y), glxx_clamp(a->z), glxx_clamp(a->w));
}

void glxx_v_dehomogenize(GLXX_VEC3_T *result, const GLXX_VEC4_T *a)
{
   Dataflow *rw = glxx_recip(a->w);
   glxx_v_vec3(result, glxx_mul(a->x, rw), glxx_mul(a->y, rw), glxx_mul(a->z, rw));
}

void glxx_v_dropw(GLXX_VEC3_T *result, const GLXX_VEC4_T *a)
{
   glxx_v_vec3(result, a->x, a->y, a->z);
}

void glxx_v_nullw(GLXX_VEC4_T *result, const GLXX_VEC3_T *a)
{
   glxx_v_vec4(result, a->x, a->y, a->z, NULL);
}

void glxx_v_interp4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b, const GLXX_VEC4_T *c)
{
   GLXX_VEC4_T tmp0, tmp1;
   glxx_v_crep4(&tmp0, 1.0f);
   glxx_v_sub4 (&tmp0, &tmp0, c);
   glxx_v_mul4 (&tmp0, a, &tmp0);
   glxx_v_mul4 (&tmp1, b, c);
   glxx_v_add4 (result, &tmp0, &tmp1);
}

void glxx_v_blend4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b, Dataflow *c)
{
   GLXX_VEC4_T tmp;
   glxx_v_rep4(&tmp, c);
   glxx_v_interp4(result, a, b, &tmp);
}

void glxx_v_blend3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b, Dataflow *c)
{
   GLXX_VEC3_T tmp0, tmp1;
   glxx_v_scale3(&tmp0, glxx_sub(glxx_cfloat(1.0f), c), a);
   glxx_v_scale3(&tmp1, c, b);
   glxx_v_add3(result, &tmp0, &tmp1);
}

void glxx_v_cond4(GLXX_VEC4_T *result, Dataflow *c, const GLXX_VEC4_T *t, const GLXX_VEC4_T *f)
{
   glxx_v_vec4(result, glxx_cond(c,t->x,f->x), glxx_cond(c,t->y,f->y), glxx_cond(c,t->z,f->z), glxx_cond(c,t->w,f->w));
}

/* matrix */

void glxx_v_transform4x4(GLXX_VEC4_T *result, const GLXX_VEC4_T *row0, const GLXX_VEC4_T *row1, const GLXX_VEC4_T *row2, const GLXX_VEC4_T *row3, const GLXX_VEC4_T *v)
{
   glxx_v_vec4(result, glxx_dot4(row0, v), glxx_dot4(row1, v), glxx_dot4(row2, v), glxx_dot4(row3, v));
}

void glxx_v_transform3x3(GLXX_VEC3_T *result, const GLXX_VEC3_T *row0, const GLXX_VEC3_T *row1, const GLXX_VEC3_T *row2, const GLXX_VEC3_T *v)
{
   glxx_v_vec3(result, glxx_dot3(row0, v), glxx_dot3(row1, v), glxx_dot3(row2, v));
}

Dataflow *glxx_accu_rsqrt(Dataflow *x)
{
   return glxx_rsqrt(x);
}

Dataflow *glxx_accu_pow(Dataflow *a, Dataflow *b)
{
   return glxx_exp2(glxx_mul(b, glxx_log2(a)));
}

Dataflow *glxx_i_to_f(Dataflow *i)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_ITOF, i);
}

Dataflow *glxx_f_to_i(Dataflow *f)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_FTOI_TRUNC, f);
}

Dataflow *glxx_f_to_i_nearest(Dataflow *f)
{
   return glsl_dataflow_construct_unary_op(DATAFLOW_FTOI_NEAREST, f);
}
