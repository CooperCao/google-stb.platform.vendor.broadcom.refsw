/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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

#include <bsg_matrix.h>
#include <cmath>

// Assumes that 
bsg::Mat3 RotateTo(const bsg::Vec3 &f, const bsg::Vec3 &t)
{
   const float EPSILON = 0.00001f;

   bsg::Vec3  v;
   bsg::Mat3  res;
   float c, h;

   v = Cross(f, t);
   c = f.Dot(t);

   if (fabs(c) < (1.0f - EPSILON))
   {
      h = 1.0f / (1.0f + c);

      res(0, 0) = c + h * v.X() * v.X();
      res(0, 1) = h * v.X() * v.Y() - v.Z();
      res(0, 2) = h * v.X() * v.Z() + v.Y();

      res(1, 0) = h * v.X() * v.Y() + v.Z();
      res(1, 1) = c + h * v.Y() * v.Y();
      res(1, 2) = h * v.Y() * v.Z() - v.X();

      res(2, 0) = h * v.X() * v.Z() - v.Y();
      res(2, 1) = h * v.Y() * v.Z() + v.X();
      res(2, 2) = c + h * v.Z() * v.Z();
   }
   else
   {
      bsg::Vec3  tu, tv;

      bsg::Vec3  x = Abs(f);

      if (x.X() < x.Y())
      {
         if (x.X() < x.Z())
            x = bsg::Vec3(1.0f, 0.0f, 0.0f);
         else
            x = bsg::Vec3(0.0f, 0.0f, 1.0f);
      }
      else
      {
         if (x.Y() < x.Z())
            x = bsg::Vec3(0.0f, 1.0f, 0.0f);
         else
            x = bsg::Vec3(0.0f, 0.0f, 1.0f);
      }

      tu = x - f;
      tv = x - t;

      float c1 = 2.0f / tu.Dot(tu);
      float c2 = 2.0f / tv.Dot(tv);
      float c3 = c1 * c2  * tu.Dot(tv);

      for (uint32_t i = 0; i < 3; i++)
         for (uint32_t j = 0; j < 3; j++)
         {
            if (i == j)
               res(i, j) = 1.0f;
            else
               res(i, j) =  - c1 * tu[i] * tu[j]
                            - c2 * tv[i] * tv[j]
                            + c3 * tv[i] * tu[j];
         }
   }

   return res;
}
