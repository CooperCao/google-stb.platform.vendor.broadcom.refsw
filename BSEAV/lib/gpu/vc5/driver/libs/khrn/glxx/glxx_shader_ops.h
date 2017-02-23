/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
common GLES 1.1 and 2.0 dataflow graph functions
=============================================================================*/

#ifndef GL11_SHADER_4_OPS_H
#define GL11_SHADER_4_OPS_H

#include "../glsl/glsl_dataflow.h"

typedef struct {
   Dataflow *x;
   Dataflow *y;
   Dataflow *z;
} GLXX_VEC3_T;

typedef struct {
   Dataflow *x;
   Dataflow *y;
   Dataflow *z;
   Dataflow *w;
} GLXX_VEC4_T;

typedef struct {
      GLXX_VEC4_T row[4];
} GLXX_MAT4_T;

typedef struct {
      GLXX_VEC3_T row[3];
} GLXX_MAT3_T;

static inline Dataflow *glxx_cfloat(float f)   { return glsl_dataflow_construct_const_float(f); }
static inline Dataflow *glxx_cint(uint32_t i)  { return glsl_dataflow_construct_const_int(i); }
static inline Dataflow *glxx_cbool(bool b)     { return glsl_dataflow_construct_const_bool(b); }
static inline Dataflow *glxx_u(uint32_t i)     { return glsl_dataflow_construct_buffer(DATAFLOW_UNIFORM, DF_FLOAT, i, 0); }
static inline Dataflow *glxx_u_int(uint32_t i) { return glsl_dataflow_construct_buffer(DATAFLOW_UNIFORM, DF_INT,   i, 0); }

static inline Dataflow *glxx_fragment_get(DataflowFlavour flavour) { return glsl_dataflow_construct_nullary_op(flavour); }

static inline Dataflow *glxx_mul(Dataflow *a, Dataflow *b)  { return glsl_dataflow_construct_binary_op(DATAFLOW_MUL, a, b); }
static inline Dataflow *glxx_add(Dataflow *a, Dataflow *b)  { return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, a, b); }
static inline Dataflow *glxx_sub(Dataflow *a, Dataflow *b)  { return glsl_dataflow_construct_binary_op(DATAFLOW_SUB, a, b); }
static inline Dataflow *glxx_fmax(Dataflow *a, Dataflow *b) { return glsl_dataflow_construct_binary_op(DATAFLOW_MAX, a, b); }
static inline Dataflow *glxx_fmin(Dataflow *a, Dataflow *b) { return glsl_dataflow_construct_binary_op(DATAFLOW_MIN, a, b); }
static inline Dataflow *glxx_abs(Dataflow *a)     { return glsl_dataflow_construct_unary_op(DATAFLOW_ABS,   a); }
static inline Dataflow *glxx_floor(Dataflow *a)   { return glsl_dataflow_construct_unary_op(DATAFLOW_FLOOR, a); }
static inline Dataflow *glxx_exp2(Dataflow *a)    { return glsl_dataflow_construct_unary_op(DATAFLOW_EXP2,  a); }
static inline Dataflow *glxx_log2(Dataflow *a)    { return glsl_dataflow_construct_unary_op(DATAFLOW_LOG2,  a); }
static inline Dataflow *glxx_rsqrt(Dataflow *a)   { return glsl_dataflow_construct_unary_op(DATAFLOW_RSQRT, a); }
static inline Dataflow *glxx_recip(Dataflow *a)   { return glsl_dataflow_construct_unary_op(DATAFLOW_RCP,   a); }
static inline Dataflow *glxx_sqrt(Dataflow *a)    { return glsl_dataflow_construct_unary_op(DATAFLOW_SQRT,  a); }
static inline Dataflow *glxx_negate(Dataflow *a)  { return glsl_dataflow_construct_unary_op(DATAFLOW_ARITH_NEGATE, a); }

static inline Dataflow *glxx_equal(Dataflow *a, Dataflow *b)  { return glsl_dataflow_construct_binary_op(DATAFLOW_EQUAL,       a, b); }
static inline Dataflow *glxx_less(Dataflow *a, Dataflow *b)   { return glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN,   a, b); }
static inline Dataflow *glxx_lequal(Dataflow *a, Dataflow *b) { return glsl_dataflow_construct_binary_op(DATAFLOW_LESS_THAN_EQUAL, a, b); }
static inline Dataflow *glxx_bitnot(Dataflow *x)              { return glsl_dataflow_construct_unary_op(DATAFLOW_BITWISE_NOT, x); }
static inline Dataflow *glxx_bitand(Dataflow *a, Dataflow *b) { return glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, a, b); }
static inline Dataflow *glxx_bitor(Dataflow *a, Dataflow *b)  { return glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_OR,  a, b); }
static inline Dataflow *glxx_bitxor(Dataflow *a, Dataflow *b) { return glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_XOR, a, b); }

static inline Dataflow *glxx_logicnot(Dataflow *x)             { return glsl_dataflow_construct_unary_op(DATAFLOW_LOGICAL_NOT, x); }
static inline Dataflow *glxx_logicor(Dataflow *a, Dataflow *b) { return glsl_dataflow_construct_binary_op(DATAFLOW_LOGICAL_OR, a, b); }

static inline Dataflow *glxx_i_to_f(Dataflow *i)  { return glsl_dataflow_construct_unary_op(DATAFLOW_ITOF,         i); }
static inline Dataflow *glxx_f_to_iz(Dataflow *f) { return glsl_dataflow_construct_unary_op(DATAFLOW_FTOI_TRUNC,   f); }
static inline Dataflow *glxx_f_to_in(Dataflow *f) { return glsl_dataflow_construct_unary_op(DATAFLOW_FTOI_NEAREST, f); }

Dataflow *glxx_clamp(Dataflow *a);
Dataflow *glxx_square(Dataflow *a);
Dataflow *glxx_pow(Dataflow *a, Dataflow *b);
Dataflow *glxx_cond(Dataflow *cond, Dataflow *true_value, Dataflow *false_value);
Dataflow *glxx_sum3(Dataflow *a, Dataflow *b, Dataflow *c);
Dataflow *glxx_sum4(Dataflow *a, Dataflow *b, Dataflow *c, Dataflow *d);
Dataflow *glxx_dot3(const GLXX_VEC3_T *a, const GLXX_VEC3_T *b);
Dataflow *glxx_dot4(const GLXX_VEC4_T *a, const GLXX_VEC4_T *b);

void glxx_m3_upper_left(GLXX_MAT3_T *m3, const GLXX_MAT4_T *m4);
void glxx_m3_transpose(GLXX_MAT3_T *m3);
void glxx_m3_transform(GLXX_VEC3_T *result, const GLXX_MAT3_T *m, const GLXX_VEC3_T *v);

void glxx_m4_transform(GLXX_VEC4_T *result, const GLXX_MAT4_T *m, const GLXX_VEC4_T *v);
void glxx_m4_u(GLXX_MAT4_T *m, int loc);

void glxx_v_vec3(GLXX_VEC3_T *result, Dataflow *x, Dataflow *y, Dataflow *z);
void glxx_v_vec4(GLXX_VEC4_T *result, Dataflow *x, Dataflow *y, Dataflow *z, Dataflow *w);
void glxx_v_rep3(GLXX_VEC3_T *result, Dataflow *x);
void glxx_v_rep4(GLXX_VEC4_T *result, Dataflow *x);
void glxx_v_crep3(GLXX_VEC3_T *result, float f);
void glxx_v_crep4(GLXX_VEC4_T *result, float f);
void glxx_v_u3(GLXX_VEC3_T *result, uint32_t i);
void glxx_v_u3t(GLXX_VEC3_T *result, uint32_t i);
void glxx_v_u4(GLXX_VEC4_T *result, uint32_t i);
void glxx_v_u4t(GLXX_VEC4_T *result, uint32_t i);
void glxx_v_add3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b);
void glxx_v_add4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b);
void glxx_v_sub3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b);
void glxx_v_sub4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b);
void glxx_v_mul3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b);
void glxx_v_mul4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b);
void glxx_v_scale3(GLXX_VEC3_T *result, Dataflow *a, const GLXX_VEC3_T *b);
void glxx_v_normalize3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a);
void glxx_v_negate3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a);
void glxx_v_clamp4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a);
void glxx_v_dehomogenize(GLXX_VEC3_T *result, const GLXX_VEC4_T *a);
void glxx_v_dropw(GLXX_VEC3_T *result, const GLXX_VEC4_T *a);
void glxx_v_nullw(GLXX_VEC4_T *result, const GLXX_VEC3_T *a);
void glxx_v_interp4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b, const GLXX_VEC4_T *c);
void glxx_v_blend4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b, Dataflow *c);
void glxx_v_blend3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b, Dataflow *c);
void glxx_v_cond4(GLXX_VEC4_T *result, Dataflow *c, const GLXX_VEC4_T *t, const GLXX_VEC4_T *f);
void glxx_v_transform4x4(GLXX_VEC4_T *result, const GLXX_VEC4_T *row0, const GLXX_VEC4_T *row1, const GLXX_VEC4_T *row2, const GLXX_VEC4_T *row3, const GLXX_VEC4_T *v);
void glxx_v_transform3x3(GLXX_VEC3_T *result, const GLXX_VEC3_T *row0, const GLXX_VEC3_T *row1, const GLXX_VEC3_T *row2, const GLXX_VEC3_T *v);

#endif
