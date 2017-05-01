/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <limits>

#include "bsg_exception.h"
#include "bsg_matrix.h"

namespace bsg
{

// Invert an affine matrix
//
// Matrix is assume to be of the form:
//
// | A C |
// | 0 1 |
//
// Where A is a 3x3 submatrix
//
// The inverse is
//
// | inv(A) -inv(A)'C |
// | 0          1     |
//
Mat4 Invert(const Mat4 &in)
{
   Mat4  res;

   float det_1;
   float abssum;
   float temp;

   // Calculate the determinant of 3x3 submatrix
   temp =  in(0, 0) * in(1, 1) * in(2, 2);
   det_1 = temp; abssum = fabsf(temp);
   temp =  in(0, 1) * in(1, 2) * in(2, 0);
   det_1 += temp; abssum += fabsf(temp);
   temp =  in(0, 2) * in(1, 0) * in(2, 1);
   det_1 += temp; abssum += fabsf(temp);
   temp = -in(0, 2) * in(1, 1) * in(2, 0);
   det_1 += temp; abssum += fabsf(temp);
   temp = -in(0, 1) * in(1, 0) * in(2, 2);
   det_1 += temp; abssum += fabsf(temp);
   temp = -in(0, 0) * in(1, 2) * in(2, 1);
   det_1 += temp; abssum += fabsf(temp);

   // Is the submatrix singular?
   if (det_1 == 0.0f || (fabsf(det_1 / abssum) < std::numeric_limits<float>::min()))
      BSG_THROW("Singular matrix");

   // Calculate inv(A) = adj(A) / det(A)
   det_1 = 1.0f / det_1;
   res(0, 0) =   (in(1, 1) * in(2, 2) - in(1, 2) * in(2, 1)) * det_1;
   res(1, 0) = - (in(1, 0) * in(2, 2) - in(1, 2) * in(2, 0)) * det_1;
   res(2, 0) =   (in(1, 0) * in(2, 1) - in(1, 1) * in(2, 0)) * det_1;
   res(0, 1) = - (in(0, 1) * in(2, 2) - in(0, 2) * in(2, 1)) * det_1;
   res(1, 1) =   (in(0, 0) * in(2, 2) - in(0, 2) * in(2, 0)) * det_1;
   res(2, 1) = - (in(0, 0) * in(2, 1) - in(0, 1) * in(2, 0)) * det_1;
   res(0, 2) =   (in(0, 1) * in(1, 2) - in(0, 2) * in(1, 1)) * det_1;
   res(1, 2) = - (in(0, 0) * in(1, 2) - in(0, 2) * in(1, 0)) * det_1;
   res(2, 2) =   (in(0, 0) * in(1, 1) - in(0, 1) * in(1, 0)) * det_1;

   // Calculate - inv(A) * C
   res(0, 3) = - (in(0, 3) * res(0, 0) +
                  in(1, 3) * res(0, 1) +
                  in(2, 3) * res(0, 2));
   res(1, 3) = - (in(0, 3) * res(1, 0) +
                  in(1, 3) * res(1, 1) +
                  in(2, 3) * res(1, 2));
   res(2, 3) = - (in(0, 3) * res(2, 0) +
                  in(1, 3) * res(2, 1) +
                  in(2, 3) * res(2, 2));

   /* Fill in last row */
   res(3, 0) = 0.0f;
   res(3, 1) = 0.0f;
   res(3, 2) = 0.0f;
   res(3, 3) = 1.0f;

   return res;
}

Mat3 Invert(const Mat3 &in)
{
   Mat3  res;

   float det_1;
   float abssum;
   float temp;

   // Calculate the determinant of 3x3 submatrix
   temp =  in(0, 0) * in(1, 1) * in(2, 2);
   det_1 = temp; abssum = fabsf(temp);
   temp =  in(0, 1) * in(1, 2) * in(2, 0);
   det_1 += temp; abssum += fabsf(temp);
   temp =  in(0, 2) * in(1, 0) * in(2, 1);
   det_1 += temp; abssum += fabsf(temp);
   temp = -in(0, 2) * in(1, 1) * in(2, 0);
   det_1 += temp; abssum += fabsf(temp);
   temp = -in(0, 1) * in(1, 0) * in(2, 2);
   det_1 += temp; abssum += fabsf(temp);
   temp = -in(0, 0) * in(1, 2) * in(2, 1);
   det_1 += temp; abssum += fabsf(temp);

   // Is the submatrix singular?
   if (det_1 == 0.0f || (fabsf(det_1 / abssum) < std::numeric_limits<float>::min()))
      BSG_THROW("Singular matrix");

   // Calculate inv(A) = adj(A) / det(A)
   det_1 = 1.0f / det_1;
   res(0, 0) =   (in(1, 1) * in(2, 2) - in(1, 2) * in(2, 1)) * det_1;
   res(1, 0) = - (in(1, 0) * in(2, 2) - in(1, 2) * in(2, 0)) * det_1;
   res(2, 0) =   (in(1, 0) * in(2, 1) - in(1, 1) * in(2, 0)) * det_1;
   res(0, 1) = - (in(0, 1) * in(2, 2) - in(0, 2) * in(2, 1)) * det_1;
   res(1, 1) =   (in(0, 0) * in(2, 2) - in(0, 2) * in(2, 0)) * det_1;
   res(2, 1) = - (in(0, 0) * in(2, 1) - in(0, 1) * in(2, 0)) * det_1;
   res(0, 2) =   (in(0, 1) * in(1, 2) - in(0, 2) * in(1, 1)) * det_1;
   res(1, 2) = - (in(0, 0) * in(1, 2) - in(0, 2) * in(1, 0)) * det_1;
   res(2, 2) =   (in(0, 0) * in(1, 1) - in(0, 1) * in(1, 0)) * det_1;

   return res;
}

Mat4 LookAt(const Vec3 &location, const Vec3 &lookAt, const Vec3 &upAsk, Vec3 *realUp)
{
   Vec3 z = Normalize(location - lookAt);
   Vec3 x = Normalize(Cross(upAsk, z));
   Vec3 y = Normalize(Cross(z, x));

   if (realUp != 0)
      *realUp = y;

   Mat4         result;

   result(0, 0) = x[0];
   result(1, 0) = x[1];
   result(2, 0) = x[2];

   result(0, 1) = y[0];
   result(1, 1) = y[1];
   result(2, 1) = y[2];

   result(0, 2) = z[0];
   result(1, 2) = z[1];
   result(2, 2) = z[2];

   result(0, 3) = location[0];
   result(1, 3) = location[1];
   result(2, 3) = location[2];

   return result;
}

}
