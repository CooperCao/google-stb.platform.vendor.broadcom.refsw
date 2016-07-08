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

typedef struct
{
   Dataflow *x;
   Dataflow *y;
   Dataflow *z;
} GLXX_VEC3_T;

typedef struct
{
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

Dataflow *glxx_cfloat(float f);
Dataflow *glxx_cint(uint32_t i);
Dataflow *glxx_cbool(bool b);
void glxx_v_vec3(GLXX_VEC3_T *result, Dataflow *x, Dataflow *y, Dataflow *z);
void glxx_v_vec4(GLXX_VEC4_T *result, Dataflow *x, Dataflow *y, Dataflow *z, Dataflow *w);
void glxx_v_rep3(GLXX_VEC3_T *result, Dataflow *x);
void glxx_v_rep4(GLXX_VEC4_T *result, Dataflow *x);
void glxx_v_crep3(GLXX_VEC3_T *result, float f);
void glxx_v_crep4(GLXX_VEC4_T *result, float f);
void glxx_m3_upper_left(GLXX_MAT3_T *m3, const GLXX_MAT4_T *m4);
void glxx_m3_transpose(GLXX_MAT3_T *m3);
void glxx_m4_transform(GLXX_VEC4_T *result, const GLXX_MAT4_T *m, const GLXX_VEC4_T *v);
void glxx_m3_transform(GLXX_VEC3_T *result, const GLXX_MAT3_T *m, const GLXX_VEC3_T *v);
Dataflow *glxx_u(uint32_t i);
Dataflow *glxx_u_int(uint32_t i);
Dataflow *glxx_u_off(uint32_t i, uint32_t j);
Dataflow *glxx_u_off_int(uint32_t i, uint32_t j);
void glxx_v_u3(GLXX_VEC3_T *result, uint32_t i);
void glxx_v_u3t(GLXX_VEC3_T *result, uint32_t i);
void glxx_v_u4(GLXX_VEC4_T *result, uint32_t i);
void glxx_v_u4t(GLXX_VEC4_T *result, uint32_t i);
void glxx_m4_u(GLXX_MAT4_T *m, int loc);
Dataflow *glxx_fetch_vpm(Dataflow *prev);
Dataflow *glxx_fragment_get(DataflowFlavour flavour);
Dataflow *glxx_mul(Dataflow *a, Dataflow *b);
Dataflow *glxx_add(Dataflow *a, Dataflow *b);
Dataflow *glxx_sub(Dataflow *a, Dataflow *b);
Dataflow *glxx_abs(Dataflow *a);
Dataflow *glxx_fmax(Dataflow *a, Dataflow *b);
Dataflow *glxx_fmin(Dataflow *a, Dataflow *b);
Dataflow *glxx_floor(Dataflow *a);
Dataflow *glxx_exp2(Dataflow *a);
Dataflow *glxx_log2(Dataflow *a);
Dataflow *glxx_rsqrt(Dataflow *a);
Dataflow *glxx_recip(Dataflow *a);
Dataflow *glxx_sqrt1(Dataflow *a);
Dataflow *glxx_negate(Dataflow *a);
Dataflow *glxx_clamp(Dataflow *a);
Dataflow *glxx_square(Dataflow *a);
Dataflow *glxx_equal(Dataflow *a, Dataflow *b);
Dataflow *glxx_less(Dataflow *a, Dataflow *b);
Dataflow *glxx_lequal(Dataflow *a, Dataflow *b);
Dataflow *glxx_bitnot(Dataflow *x);
Dataflow *glxx_bitand(Dataflow *a, Dataflow *b);
Dataflow *glxx_bitor(Dataflow *a, Dataflow *b);
Dataflow *glxx_bitxor(Dataflow *a, Dataflow *b);
Dataflow *glxx_logicnot(Dataflow *x);
Dataflow *glxx_logicor(Dataflow *a, Dataflow *b);
Dataflow *glxx_cond(Dataflow *cond, Dataflow *true_value, Dataflow *false_value);
Dataflow *glxx_sum3(Dataflow *a, Dataflow *b, Dataflow *c);
Dataflow *glxx_sum4(Dataflow *a, Dataflow *b, Dataflow *c, Dataflow *d);
Dataflow *glxx_dot3(const GLXX_VEC3_T *a, const GLXX_VEC3_T *b);
Dataflow *glxx_dot4(const GLXX_VEC4_T *a, const GLXX_VEC4_T *b);
void glxx_v_add3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b);
void glxx_v_add4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b);
void glxx_v_sub3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b);
void glxx_v_sub4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b);
void glxx_v_mul3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a, const GLXX_VEC3_T *b);
void glxx_v_mul4(GLXX_VEC4_T *result, const GLXX_VEC4_T *a, const GLXX_VEC4_T *b);
void glxx_v_scale3(GLXX_VEC3_T *result, Dataflow *a, const GLXX_VEC3_T *b);
void glxx_v_normalize3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a);
void glxx_v_accu_normalize3(GLXX_VEC3_T *result, const GLXX_VEC3_T *a);
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
Dataflow *glxx_accu_recip(Dataflow *x);
Dataflow *glxx_accu_rsqrt(Dataflow *x);
Dataflow *glxx_accu_pow(Dataflow *a, Dataflow *b);
Dataflow *glxx_i_to_f(Dataflow *i);
Dataflow *glxx_f_to_i(Dataflow *f);
Dataflow *glxx_f_to_i_nearest(Dataflow *f);

#endif
