/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/glxx/gl11_int_config.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
   float body[GL11_CONFIG_MAX_STACK_DEPTH][16];

   /*
      Current top of stack

      i.e. body[n] is on the stack iff n <= pos

      Invariant:

      0 <= pos < GL11_CONFIG_MAX_STACK_DEPTH
   */

   int32_t pos;
} GL11_MATRIX_STACK_T;

extern void gl11_matrix_load(float *d, const float *a);
extern void gl11_matrix_mult(float *d, const float *a, const float *b);

/*
   multiply matrix by row vector

   (x' y' z' w') = (x y z w) B
*/

extern void gl11_matrix_mult_row(float *d, const float *a, const float *b);
extern void gl11_matrix_mult_col(float *d, const float *a, const float *b);

/*
   gl11_matrix_invert_4x4(float *d, const float *a)

   invert a non-singular 4x4 matrix
*/

extern void gl11_matrix_invert_4x4(float *d, const float *a);
extern void gl11_matrix_invert_3x3(float *d, const float *a);
extern void gl11_matrix_transpose(float *d, const float *a);
extern bool gl11_matrix_is_projective(float *a);
extern void gl11_matrix_stack_init(GL11_MATRIX_STACK_T *stack);