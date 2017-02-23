/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 matrix and matrix stack structure declaration.
=============================================================================*/

//cr[ 2011-06-16 15:00

#ifndef GL11_MATRIX_H
#define GL11_MATRIX_H

#include "../glxx/gl_public_api.h"
#include "gl11_int_config.h"

typedef struct {
   float body[GL11_CONFIG_MAX_STACK_DEPTH-1][16];

   /*
      Current top of stack

      i.e. body[n] is on the stack iff n <= pos

      Invariant:

      0 <= pos < GL11_CONFIG_MAX_STACK_DEPTH
   */

   int32_t pos;
} GL11_MATRIX_STACK_T;

extern void gl11_matrix_load(float d[16], const float a[16]);
extern void gl11_matrix_mult(float d[16], const float a[16], const float b[16]);

extern void gl11_matrix_mult_row(float d[16], const float a[16], const float b[16]);
extern void gl11_matrix_mult_col(float d[16], const float a[16], const float b[16]);

extern void gl11_matrix_invert_4x4(float d[16], const float a[16]);
extern void gl11_matrix_invert_3x3(float d[16], const float a[16]);

extern void gl11_matrix_transpose(float d[16], const float a[16]);

extern bool gl11_matrix_is_projective(float a[16]);

extern void gl11_matrix_stack_init(GL11_MATRIX_STACK_T *stack);

#endif

//cr]
