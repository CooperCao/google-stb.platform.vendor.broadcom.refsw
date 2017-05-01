/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
