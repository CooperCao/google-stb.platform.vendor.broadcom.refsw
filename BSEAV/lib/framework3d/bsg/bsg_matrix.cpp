/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

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
