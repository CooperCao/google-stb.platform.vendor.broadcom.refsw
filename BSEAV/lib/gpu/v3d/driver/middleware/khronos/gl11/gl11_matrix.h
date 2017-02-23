/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 matrix and matrix stack structure declaration.
=============================================================================*/

#ifndef GL11_MATRIX_H
#define GL11_MATRIX_H

#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/glxx/gl11_int_config.h"

typedef struct {
   GLfloat body[GL11_CONFIG_MAX_STACK_DEPTH][16];

   /*
      Current top of stack

      i.e. body[n] is on the stack iff n <= pos

      Invariant:

      0 <= pos < GL11_CONFIG_MAX_STACK_DEPTH
   */

   int32_t pos;
} GL11_MATRIX_STACK_T;

extern void gl11_matrix_load(GLfloat *d, const GLfloat *a);
extern void gl11_matrix_mult(GLfloat *d, const GLfloat *a, const GLfloat *b);

/*
   multiply matrix by row vector

   (x' y' z' w') = (x y z w) B
*/

extern void gl11_matrix_mult_row(GLfloat *d, const GLfloat *a, const GLfloat *b);
extern void gl11_matrix_mult_col(GLfloat *d, const GLfloat *a, const GLfloat *b);

/*
   gl11_matrix_invert_4x4(GLfloat *d, const GLfloat *a)

   invert a non-singular 4x4 matrix
*/

extern void gl11_matrix_invert_4x4(GLfloat *d, const GLfloat *a);

extern void gl11_matrix_invert_3x3(GLfloat *d, const GLfloat *a);

extern void gl11_matrix_transpose(GLfloat *d, const GLfloat *a);

extern GLboolean gl11_matrix_is_projective(GLfloat *a);

extern void gl11_matrix_stack_init(GL11_MATRIX_STACK_T *stack);

#endif
