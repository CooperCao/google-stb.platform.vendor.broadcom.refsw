/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 matrix and matrix stack implementation.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_hw.h"

#include "middleware/khronos/gl11/gl11_matrix.h"

#include <string.h>

#include "middleware/khronos/gl11/gl11_matrix_cr.c"

/*
   multiply matrix by row vector

   (x' y' z' w') = (x y z w) B
*/

void gl11_matrix_mult_row(GLfloat *d, const GLfloat *a, const GLfloat *b)
{
   int i;
   for (i = 0; i < 4; i++) {
      GLfloat v = 0.0f;
      int j;
      for (j = 0; j < 4; j++)
         v += a[j] * b[OFFSET(j, i)];

      d[i] = v;
   }
}

/*
   multiply matrix by col vector

   (x') = A (x)
   (y')     (y)
   (z')     (z)
   (w')     (w)
*/

void gl11_matrix_mult_col(GLfloat *d, const GLfloat *a, const GLfloat *b)
{
   int i;
   for (i = 0; i < 4; i++) {
      GLfloat v = 0.0f;
      int j;
      for (j = 0; j < 4; j++)
         v += a[OFFSET(i, j)] * b[j];

      d[i] = v;
   }
}

/*
   gl11_matrix_invert_4x4(GLfloat *d, const GLfloat *a)

   invert a non-singular 4x4 matrix
*/

#define SPLICE(i,x) (((i)<(x)) ? (i) : ((i)+1))

void gl11_matrix_invert_4x4(GLfloat *d, const GLfloat *a)
{
   // A list of all 4-permutations in no particular order except that:
   // - even and odd permutations are listed alternately
   // - the first six elements can also be used as 3-permutations
   static int p[24][4] = {
      {0,1,2,3},{0,2,1,3},{1,2,0,3},{1,0,2,3},{2,0,1,3},{2,1,0,3},
                {1,3,2,0},{2,3,1,0},{2,1,3,0},{3,1,2,0},{3,2,1,0},{1,2,3,0},
      {2,3,0,1},{2,0,3,1},{3,0,2,1},{3,2,0,1},{0,2,3,1},{0,3,2,1},
                {3,1,0,2},{0,1,3,2},{0,3,1,2},{1,3,0,2},{1,0,3,2},{3,0,1,2},
   };
   int n,i,col,row;
   float det = 0;
   for (n = 0; n < 24; n++)
   {
      float product = (n % 2) ? -1.0f : 1.0f;
      for (i = 0; i < 4; i++)
      {
         product *= a[OFFSET(i, p[n][i])];
      }
      det += product;
   }
   for (col = 0; col < 4; col++)
   {
      for (row = 0; row < 4; row++)
      {
         float sum = 0;
         for (n = 0; n < 6; n++)
         {
            float product = ((n+row+col) % 2) ? -1.0f : 1.0f;
            for (i = 0; i < 3; i++)
            {
               product *= a[OFFSET(
                  SPLICE(i, row),
                  SPLICE(p[n][i], col)
               )];
            }
            sum += product;
         }
         d[OFFSET(col,row)] = sum / det;
      }
   }
}

/*
   | a00 a01 a02 |-1             |   a22a11-a21a12  -(a22a01-a21a02)   a12a01-a11a02  |
   | a10 a11 a12 |    =  1/DET * | -(a22a10-a20a12)   a22a00-a20a02  -(a12a00-a10a02) |
   | a20 a21 a22 |               |   a21a10-a20a11  -(a21a00-a20a01)   a11a00-a10a01  |

   with DET  =  a00(a22a11-a21a12)-a10(a22a01-a21a02)+a20(a12a01-a11a02)
*/

void gl11_matrix_invert_3x3(GLfloat *d, const GLfloat *a)
{
   float det = a[OFFSET(0, 0)] * (a[OFFSET(1, 1)] * a[OFFSET(2, 2)] - a[OFFSET(1, 2)] * a[OFFSET(2, 1)]) +
               a[OFFSET(1, 0)] * (a[OFFSET(2, 1)] * a[OFFSET(0, 2)] - a[OFFSET(2, 2)] * a[OFFSET(0, 1)]) +
               a[OFFSET(2, 0)] * (a[OFFSET(0, 1)] * a[OFFSET(1, 2)] - a[OFFSET(0, 2)] * a[OFFSET(1, 1)]);

   d[OFFSET(0, 0)] = (a[OFFSET(2, 2)] * a[OFFSET(1, 1)] - a[OFFSET(2, 1)] * a[OFFSET(1, 2)]) / det;
   d[OFFSET(0, 1)] = (a[OFFSET(0, 2)] * a[OFFSET(2, 1)] - a[OFFSET(0, 1)] * a[OFFSET(2, 2)]) / det;
   d[OFFSET(0, 2)] = (a[OFFSET(1, 2)] * a[OFFSET(0, 1)] - a[OFFSET(1, 1)] * a[OFFSET(0, 2)]) / det;
   d[OFFSET(0, 3)] = 0.0f;

   d[OFFSET(1, 0)] = (a[OFFSET(2, 0)] * a[OFFSET(1, 2)] - a[OFFSET(2, 2)] * a[OFFSET(1, 0)]) / det;
   d[OFFSET(1, 1)] = (a[OFFSET(0, 0)] * a[OFFSET(2, 2)] - a[OFFSET(0, 2)] * a[OFFSET(2, 0)]) / det;
   d[OFFSET(1, 2)] = (a[OFFSET(1, 0)] * a[OFFSET(0, 2)] - a[OFFSET(1, 2)] * a[OFFSET(0, 0)]) / det;
   d[OFFSET(1, 3)] = 0.0f;

   d[OFFSET(2, 0)] = (a[OFFSET(2, 1)] * a[OFFSET(1, 0)] - a[OFFSET(2, 0)] * a[OFFSET(1, 1)]) / det;
   d[OFFSET(2, 1)] = (a[OFFSET(0, 1)] * a[OFFSET(2, 0)] - a[OFFSET(0, 0)] * a[OFFSET(2, 1)]) / det;
   d[OFFSET(2, 2)] = (a[OFFSET(1, 1)] * a[OFFSET(0, 0)] - a[OFFSET(1, 0)] * a[OFFSET(0, 1)]) / det;
   d[OFFSET(2, 3)] = 0.0f;

   d[OFFSET(3, 0)] = 0.0f;
   d[OFFSET(3, 1)] = 0.0f;
   d[OFFSET(3, 2)] = 0.0f;
   d[OFFSET(3, 3)] = 1.0f;
}

void gl11_matrix_transpose(GLfloat *d, const GLfloat *a)
{
   int i;
   for (i = 0; i < 4; i++)
   {
      int j;
      for (j = 0; j < 4; j++)
         d[OFFSET(i, j)] = a[OFFSET(j, i)];
   }
}

GLboolean gl11_matrix_is_projective(GLfloat *a)
{
   return a[3] != 0.0f || a[7] != 0.0f || a[11] != 0.0f || a[15] != 1.0f;
}

void gl11_matrix_stack_init(GL11_MATRIX_STACK_T *stack)
{
   stack->body[0][0]  = 1.0f;
   stack->body[0][1]  = 0.0f;
   stack->body[0][2]  = 0.0f;
   stack->body[0][3]  = 0.0f;

   stack->body[0][4]  = 0.0f;
   stack->body[0][5]  = 1.0f;
   stack->body[0][6]  = 0.0f;
   stack->body[0][7]  = 0.0f;

   stack->body[0][8]  = 0.0f;
   stack->body[0][9]  = 0.0f;
   stack->body[0][10] = 1.0f;
   stack->body[0][11] = 0.0f;

   stack->body[0][12] = 0.0f;
   stack->body[0][13] = 0.0f;
   stack->body[0][14] = 0.0f;
   stack->body[0][15] = 1.0f;

   stack->pos = 0;
}
