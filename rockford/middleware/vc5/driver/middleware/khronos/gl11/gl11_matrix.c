/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 matrix and matrix stack implementation.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#include "middleware/khronos/gl11/gl11_matrix.h"

#include <string.h>

//cr[ 2011-06-16 15:00

/* Assumes that pointers are in the same memory alias */
#define OVERLAPPING(ptr_a, size_a, ptr_b, size_b) ( ( (size_a) > 0 && (size_b) > 0) && \
                                                    ( ( (char *)(ptr_a) <= (char *)(ptr_b) && (char *)(ptr_a)+(size_a) > (char *)(ptr_b) ) || \
                                                      ( (char *)(ptr_b) <= (char *)(ptr_a) && (char *)(ptr_b)+(size_b) > (char *)(ptr_a) )      )  )

/*
   Copies a matrix from one place to another. (source and destination
   should not be identical or overlap).

   Khronos documentation [See GL1.1 S2.10.2 - Matrices]:

   LoadMatrix takes a pointer to a 4 × 4 matrix stored in column-major order as 16
   consecutive fixed- or floating-point values, i.e. as

   a0  a4  a8  a12
   a1  a5  a9  a13
   a2  a6  a10 a14
   a3  a7  a11 a15

   (This differs from the standard row-major C ordering for matrix elements. If the
   standard ordering is used, all of the subsequent transformation equations are transposed,
   and the columns representing vectors become rows.)

   Implementation notes:

   so we have a_ij is at a[i + j * 4]

   when multiplying we have d_01 = a_00 * b_01 + a_01 * b_11 + a_02 * b_21 + a_03 * b_31

   We use the same internal representation of a matrix as GL does, therefore
   we can use this to move matrices from client land into our internal stuff
   directly. (Hence why we quoted the above thing about LoadMatrix).

   Preconditions:

   a, d do not overlap

   Postconditions:

   -
*/

void gl11_matrix_load(float d[16], const float a[16])
{
   assert(!OVERLAPPING(d, 16*sizeof(float), a, 16*sizeof(float)));

   memcpy(d, a, 16 * sizeof(float));
}

/*
   Returns the offset into a column-major matrix, in units of sizeof(float)
   where i is the row number and j is the column number.
   Preconditions:
      0 <= i < 4
      0 <= j < 4
   Postconditions:
      0 <= result < 16
*/
#define OFFSET(i, j) ((i) + (j) * 4)


/*
   Calculates the matrix product ab

   Implementation notes:

   It's OK for any of the pointers to overlap.

   Preconditions:

   -

   Postconditions:

   -
*/

void gl11_matrix_mult(float d[16], const float a[16], const float b[16])
{
   int32_t i;
   float t[16];

   for (i = 0; i < 4; i++)
   {
      int32_t j;
      for (j = 0; j < 4; j++) {
         float v = 0.0f;
         int32_t k;
         for (k = 0; k < 4; k++)
            v += a[OFFSET(i, k)] * b[OFFSET(k, j)];

         t[OFFSET(i, j)] = v;
      }
   }

   gl11_matrix_load(d, t);
}

/*
   multiply matrix by row vector

   (x' y' z' w') = (x y z w) B

   Preconditions:

   d does not overlap a, b
*/
void gl11_matrix_mult_row(float d[4], const float a[4], const float b[16])
{
   int32_t i;
   for (i = 0; i < 4; i++) {
      float v = 0.0f;
      int32_t j;
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

   Preconditions:

   d does not overlap a, b
*/

void gl11_matrix_mult_col(float d[4], const float a[16], const float b[4])
{
   int32_t i;
   for (i = 0; i < 4; i++) {
      float v = 0.0f;
      int32_t j;
      for (j = 0; j < 4; j++)
         v += a[OFFSET(i, j)] * b[j];

      d[i] = v;
   }
}


/*
   Manipulate the index i so that row/column x is omitted from the view of a matrix

   Preconditions:

   0 <= i < 3
   0 <= x < 4

   Postconditions:

   0 <= Result < 4
*/
#define SPLICE(i,x) (((i)<(x)) ? (i) : ((i)+1))

/*
   invert a non-singular 4x4 matrix

   Preconditions:

   d, a must not overlap
*/
void gl11_matrix_invert_4x4(float d[16], const float a[16])
{
   // A list of all 4-permutations in no particular order except that:
   // - even and odd permutations are listed alternately
   // - the first six elements can also be used as 3-permutations
   /* For all 0 <= n < 24, 0 <= i < 4: 0 <= p[n][i] < 4 */
   /* For all 0 <= n < 6,  0 <= i < 3: 0 <= p[n][i] < 3 */
   static int32_t p[24][4] = {
      {0,1,2,3},{0,2,1,3},{1,2,0,3},{1,0,2,3},{2,0,1,3},{2,1,0,3},
                {1,3,2,0},{2,3,1,0},{2,1,3,0},{3,1,2,0},{3,2,1,0},{1,2,3,0},
      {2,3,0,1},{2,0,3,1},{3,0,2,1},{3,2,0,1},{0,2,3,1},{0,3,2,1},
                {3,1,0,2},{0,1,3,2},{0,3,1,2},{1,3,0,2},{1,0,3,2},{3,0,1,2},
   };
   int32_t n,i,col,row;
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
   Invert a non-singular 3x3 portion of a 4x4 matrix. Pad the result to 4x4 with 0s and 1s as appropriate

   Implementation notes:

   Uses the same algorithm as gl11_matrix_invert_4x4 above but unrolled to give
      | a00 a01 a02 |-1             |   a22a11-a21a12  -(a22a01-a21a02)   a12a01-a11a02  |
      | a10 a11 a12 |    =  1/DET * | -(a22a10-a20a12)   a22a00-a20a02  -(a12a00-a10a02) |
      | a20 a21 a22 |               |   a21a10-a20a11  -(a21a00-a20a01)   a11a00-a10a01  |

   with DET  =  a00(a22a11-a21a12)-a10(a22a01-a21a02)+a20(a12a01-a11a02)

   Preconditions:

   d, a must not overlap
*/

void gl11_matrix_invert_3x3(float d[16], const float a[16])
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

/*
   Transpose a matrix

   Preconditions:

   d, a must not overlap
*/
void gl11_matrix_transpose(float d[16], const float a[16])
{
   int32_t i;
   for (i = 0; i < 4; i++)
   {
      int32_t j;
      for (j = 0; j < 4; j++)
         d[OFFSET(i, j)] = a[OFFSET(j, i)];
   }
}

/*
   Determine whether a matrix is projective, that is whether w is transformed by
   the matrix rather than simply copied.
*/
bool gl11_matrix_is_projective(float a[16])
{
   return a[3] != 0.0f || a[7] != 0.0f || a[11] != 0.0f || a[15] != 1.0f;
}


/*
   Initialise a matrix stack by marking it as empty. Just set the position because
   the head of the stack is stored separately

   Preconditions:

   stack need not be valid

   Postconditions:

   stack is valid
*/
void gl11_matrix_stack_init(GL11_MATRIX_STACK_T *stack)
{
   stack->pos = 0;
}

//cr]
