/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_MATRIX_H__
#define __BSG_MATRIX_H__

#define _USE_MATH_DEFINES
#include <math.h>
#include <memory.h>

#include "bsg_common.h"
#include "bsg_vector.h"

namespace bsg
{

class Mat2;
class Mat3;
class Mat4;

#ifdef BSG_USE_ES3
class Mat2x3;
class Mat2x4;
class Mat3x2;
class Mat3x4;
class Mat4x2;
class Mat4x3;
#endif

namespace prv
{

// Follows the GLSL convention matCxR
template <int Cols, int Rows> struct MatTraits {};

template <> struct MatTraits<2, 2>
{
   enum
   {
      Rows = 2,
      Cols = 2
   };

   typedef float   Elem_t;
   typedef void    Prev_t;
   typedef Mat2    This_t;
   typedef Mat3    Next_t;
   typedef Vec2    RowVec_t;
   typedef Vec2    ColVec_t;
   typedef Mat2    Trans_t;
};

template <> struct MatTraits<3, 3>
{
   enum
   {
      Rows = 3,
      Cols = 3
   };

   typedef float   Elem_t;
   typedef Mat2    Prev_t;
   typedef Mat3    This_t;
   typedef Mat4    Next_t;
   typedef Vec3    RowVec_t;
   typedef Vec3    ColVec_t;
   typedef Mat3    Trans_t;
};

template <> struct MatTraits<4, 4>
{
   enum
   {
      Rows = 4,
      Cols = 4
   };

   typedef float   Elem_t;
   typedef Mat3    Prev_t;
   typedef Mat4    This_t;
   typedef void    Next_t;
   typedef Vec4    RowVec_t;
   typedef Vec4    ColVec_t;
   typedef Mat4    Trans_t;
};

#ifdef BSG_USE_ES3
template <> struct MatTraits<2, 3>
{
   enum
   {
      Rows = 3,
      Cols = 2
   };

   typedef float   Elem_t;
   typedef void    Prev_t;
   typedef Mat2x3  This_t;
   typedef Mat3x4  Next_t;
   typedef Vec2    RowVec_t;
   typedef Vec3    ColVec_t;
   typedef Mat3x2  Trans_t;
};

template <> struct MatTraits<2, 4>
{
   enum
   {
      Rows = 4,
      Cols = 2
   };

   typedef float   Elem_t;
   typedef void    Prev_t;
   typedef Mat2x4  This_t;
   typedef void    Next_t;
   typedef Vec2    RowVec_t;
   typedef Vec4    ColVec_t;
   typedef Mat4x2  Trans_t;
};

template <> struct MatTraits<3, 2>
{
   enum
   {
      Rows = 2,
      Cols = 3
   };

   typedef float   Elem_t;
   typedef void    Prev_t;
   typedef Mat3x2  This_t;
   typedef Mat4x3  Next_t;
   typedef Vec3    RowVec_t;
   typedef Vec2    ColVec_t;
   typedef Mat2x3  Trans_t;
};

template <> struct MatTraits<3, 4>
{
   enum
   {
      Rows = 4,
      Cols = 3
   };

   typedef float   Elem_t;
   typedef Mat2x3  Prev_t;
   typedef Mat3x4  This_t;
   typedef void    Next_t;
   typedef Vec3    RowVec_t;
   typedef Vec4    ColVec_t;
   typedef Mat4x3  Trans_t;
};

template <> struct MatTraits<4, 2>
{
   enum
   {
      Rows = 2,
      Cols = 4
   };

   typedef float   Elem_t;
   typedef void    Prev_t;
   typedef Mat4x2  This_t;
   typedef void    Next_t;
   typedef Vec4    RowVec_t;
   typedef Vec2    ColVec_t;
   typedef Mat2x4  Trans_t;
};

template <> struct MatTraits<4, 3>
{
   enum
   {
      Rows = 3,
      Cols = 4
   };

   typedef float   Elem_t;
   typedef Mat3x2  Prev_t;
   typedef Mat4x3  This_t;
   typedef void    Next_t;
   typedef Vec4    RowVec_t;
   typedef Vec3    ColVec_t;
   typedef Mat3x4  Trans_t;
};
#endif

// TODO -- this is generic templatery move somewhere common
template <bool B> struct Assert;

template <> struct Assert<true>
{
   static void Dummy() {}
};

}

//! @addtogroup math
//! @{

//! Template base class for rectangular matrices.
//!
//! To define a concrete matrix class, derive from the base class supplying a traits class
//! such that
//! - Traits::Rows is the number of rows in the matrix
//! - Traits::Cols is the number of columns in the matrix
//! - Traits::Elem_t is the type of the matrix elements
//! - Traits::This_t is the type of the derived class
//! - Traits::Prev_t is the derived class with one less row and column
//! - Traits::Next_t is the derived class with side one more row and column
//! - Traits::RowVec_t is the vector type of rows
//! - Traits::ColVec_t is the vector type of columns
//! - Traits::Trans_t is the type of the transpose
//!
//! For example, the traits class for a 2x3 matrix is:
//! @code
//! struct MatTraits<2, 3>
//! {
//!   enum
//!   {
//!      Rows = 3,
//!      Cols = 2
//!   };
//!   typedef float   Elem_t;
//!   typedef void    Prev_t;
//!   typedef Mat2x3  This_t;
//!   typedef Mat3x4  Next_t;
//!   typedef Vec2    RowVec_t;
//!   typedef Vec3    ColVec_t;
//!   typedef Mat3    Trans_t;
//! };
//! @endcode
//!
//! Beware that the naming convention follows the OpenGL MatCxR style which is inconsistent with
//! the preferred mathematical R x C style.
//!
//! Applications should not create or use Mat<Traits> objects directly.  Prefer to use the
//! derived classes Mat2, Mat3, Mat4, Mat2x3 etc.
//!
template <class Traits>
class Mat
{
public:
   typedef typename Traits::Elem_t   Elem_t;
   typedef typename Traits::This_t   This_t;
   typedef typename Traits::Prev_t   Prev_t;
   typedef typename Traits::Next_t   Next_t;
   typedef typename Traits::RowVec_t RowVec_t;
   typedef typename Traits::ColVec_t ColVec_t;
   typedef typename Traits::Trans_t  Trans_t;

   enum
   {
      Rows = Traits::Rows,
      Cols = Traits::Cols
   };

   //! Default constructor.  Square matrices are initialised to the identity.
   Mat()
   {
      for (unsigned int c = 0; c < Cols; ++c)
         for (unsigned int r = 0; r < Rows; ++r)
            m_m[c][r] = 0.0f;

      if (Cols == Rows)
      {
         for (unsigned int c = 0; c < Rows; ++c)
            m_m[c][c] = 1.0f;
      }
   }

   Mat(const float *init)
   {
      for (unsigned int c = 0; c < Cols; ++c)
         for (unsigned int r = 0; r < Rows; ++r)
            m_m[c][r] = init[r * Cols + c];
   }

   //! Swaps rows and columns
   Trans_t Transpose() const
   {
      This_t   res;

      for (unsigned int c = 0; c < Cols; ++c)
         for (unsigned int r = 0; r < Rows; ++r)
            res(r, c) = (*this)(c, r);

      return res;
   }

   //! Access elements by (row, column) indices (non const)
   Elem_t &operator()(int r, int c)       { return m_m[c][r]; }

   //! Access elements by (row, column) indices (const)
   Elem_t operator()(int r, int c) const  { return m_m[c][r]; }

   //! Accumulate a matrix
   This_t &operator+=(const Mat &rhs)
   {
      for (unsigned int c = 0; c < Cols; ++c)
         for (unsigned int r = 0; r < Rows; ++r)
            m_m[c][r] += rhs.m_m[c][r];

      return static_cast<This_t &>(*this);
   }

   //! Add matrices element-wise.
   This_t operator+(const Mat &rhs) const
   {
      return This_t(m_m) += rhs;
   }

   //! Subtract a matrix element-wise from this
   This_t &operator-=(const Mat &rhs)
   {
      for (unsigned int c = 0; c < Cols; ++c)
         for (unsigned int r = 0; r < Rows; ++r)
            m_m[c][r] -= rhs.m_m[c][r];

      return static_cast<This_t &>(*this);
   }

   //! Subtract matrices element-wise
   This_t operator-(const Mat &rhs) const
   {
      return This_t(m_m) -= rhs;
   }

   //! Multiply (compose) two matrices
   template <typename RHS>
   typename prv::MatTraits<RHS::Cols, Rows>::This_t operator*(const RHS &rhs) const
   {
      typename prv::MatTraits<RHS::Cols, Rows>::This_t   res;

      prv::Assert<(int)Cols == (int)RHS::Rows>::Dummy();

      for (unsigned int r = 0; r < Rows; ++r)
      {
         for (unsigned int c = 0; c < RHS::Cols; ++c)
         {
            float sum = 0.0f;

            for (unsigned int t = 0; t < Cols; ++t)
               sum += (*this)(r, t) * rhs(t, c);

            res(r, c) = sum;
         }
      }

      return res;
   }

   //! Test matrices for equality
   bool operator==(const Mat &rhs) const
   {
      for (int c = Cols - 1; c >= 0; --c)
         for (int r = Rows - 1; r >= 0; --r)
            if (m_m[c][r] != rhs.m_m[c][r])
               return false;

      return true;
   }

   //! Test matrices for inequality
   bool operator!=(const Mat &rhs) const
   {
      return !operator==(rhs);
   }

   //! Compose a vector with a matrix
   ColVec_t operator *(const ColVec_t &rhs) const
   {
      ColVec_t res;

      for (unsigned int r = 0; r < Rows; ++r)
      {
         float sum = 0.0f;

         for (unsigned int t = 0; t < Cols; ++t)
            sum += (*this)(r, t) * rhs[t];

         res[r] = sum;
      }

      return res;
   }

   //! Drops a row and column
   Prev_t Drop() const
   {
      Prev_t   res;

      for (unsigned int c = 0; c < Cols - 1; ++c)
         for (unsigned int r = 0; r < Rows - 1; ++r)
            res(r, c) = (*this)(r, c);

      return res;
   }

   //! Provide access to the raw array of matrix elements (non const)
   Elem_t         *Ptr()        { return &m_m[0][0]; }
   //! Provide access to the raw array of matrix elements (const)
   const Elem_t   *Ptr() const  { return &m_m[0][0]; }

private:
   Elem_t  m_m[Cols][Rows];
};

//! Multiply by scalar
template <typename Traits>
typename Traits::This_t operator*(const Mat<Traits> &m, float f)
{
   typename Traits::This_t res;

   for (unsigned int c = 0; c < Traits::Cols; ++c)
      for (unsigned int r = 0; r < Traits::Rows; ++r)
         res(r, c) = m(r, c) * f;

   return res;
}


//! 2x2 matrix of floats
//!
//! See bsg::Mat.
class Mat2 : public Mat<prv::MatTraits<2, 2> >
{
   typedef Mat<prv::MatTraits<2, 2> > Base;

public:
   Mat2() : Base()
   {}

   Mat2(const float *init) : Base(init)
   {}

   Mat2(float e00, float e01,
        float e10, float e11) : Base()
   {
      Mat2 &m = *this;

      m(0, 0) = e00;  m(0, 1) = e01;
      m(1, 0) = e10;  m(1, 1) = e11;
   }
};

//! 3x3 matrix of floats
class Mat3 : public Mat<prv::MatTraits<3, 3> >
{
   typedef Mat<prv::MatTraits<3, 3> > Base;

public:
   Mat3() : Base()
   {}

   Mat3(const float *init) : Base(init)
   {}

   Mat3(float e00, float e01, float e02,
        float e10, float e11, float e12,
        float e20, float e21, float e22) : Base()
   {
      Mat3 &m = *this;

      m(0, 0) = e00;  m(0, 1) = e01;  m(0, 2) = e02;
      m(1, 0) = e10;  m(1, 1) = e11;  m(1, 2) = e12;
      m(2, 0) = e20;  m(2, 1) = e21;  m(2, 2) = e22;
   }
};

//! 4x4 matrix of floats
class Mat4 : public Mat<prv::MatTraits<4, 4> >
{
   typedef Mat<prv::MatTraits<4, 4> > Base;

public:
   Mat4() : Base()
   {}

   Mat4(const float *init) : Base(init)
   {}

   Mat4(float e00, float e01, float e02, float e03,
        float e10, float e11, float e12, float e13,
        float e20, float e21, float e22, float e23,
        float e30, float e31, float e32, float e33) : Base()
   {
      Mat4 &m = *this;

      m(0, 0) = e00;  m(0, 1) = e01;  m(0, 2) = e02;  m(0, 3) = e03;
      m(1, 0) = e10;  m(1, 1) = e11;  m(1, 2) = e12;  m(1, 3) = e13;
      m(2, 0) = e20;  m(2, 1) = e21;  m(2, 2) = e22;  m(2, 3) = e23;
      m(3, 0) = e30;  m(3, 1) = e31;  m(3, 2) = e32;  m(3, 3) = e33;
   }
};

#ifdef BSG_USE_ES3
//! Rectangular matrices
class Mat2x3 : public Mat<prv::MatTraits<2, 3> >
{
   typedef Mat<prv::MatTraits<2, 3> > Base;

public:
   Mat2x3() : Base()
   {}

   Mat2x3(const float *init) : Base(init)
   {}

   Mat2x3(float e00, float e01,
          float e10, float e11,
          float e20, float e21) : Base()
   {
      Mat2x3 &m = *this;

      m(0, 0) = e00;  m(0, 1) = e01;
      m(1, 0) = e10;  m(1, 1) = e11;
      m(2, 0) = e20;  m(2, 1) = e21;
   }
};

class Mat2x4 : public Mat<prv::MatTraits<2, 4> >
{
   typedef Mat<prv::MatTraits<2, 4> > Base;

public:
   Mat2x4() : Base()
   {}

   Mat2x4(const float *init) : Base(init)
   {}

   Mat2x4(float e00, float e01,
          float e10, float e11,
          float e20, float e21,
          float e30, float e31) : Base()
   {
      Mat2x4 &m = *this;

      m(0, 0) = e00;  m(0, 1) = e01;
      m(1, 0) = e10;  m(1, 1) = e11;
      m(2, 0) = e20;  m(2, 1) = e21;
      m(3, 0) = e30;  m(3, 1) = e31;
   }
};

class Mat3x2 : public Mat<prv::MatTraits<3, 2> >
{
   typedef Mat<prv::MatTraits<3, 2> > Base;

public:
   Mat3x2() : Base()
   {}

   Mat3x2(const float *init) : Base(init)
   {}

   Mat3x2(float e00, float e01, float e02,
          float e10, float e11, float e12) : Base()
   {
      Mat3x2 &m = *this;

      m(0, 0) = e00;  m(0, 1) = e01;  m(0, 2) = e02;
      m(1, 0) = e10;  m(1, 1) = e11;  m(1, 2) = e12;

   }
};

class Mat3x4 : public Mat<prv::MatTraits<3, 4> >
{
   typedef Mat<prv::MatTraits<3, 4> > Base;

public:
   Mat3x4() : Base()
   {}

   Mat3x4(const float *init) : Base(init)
   {}

   Mat3x4(float e00, float e01, float e02,
          float e10, float e11, float e12,
          float e20, float e21, float e22,
          float e30, float e31, float e32) : Base()
   {
      Mat3x4 &m = *this;

      m(0, 0) = e00;  m(0, 1) = e01;  m(0, 2) = e02;
      m(1, 0) = e10;  m(1, 1) = e11;  m(1, 2) = e12;
      m(2, 0) = e20;  m(2, 1) = e21;  m(2, 2) = e22;
      m(3, 0) = e30;  m(3, 1) = e31;  m(3, 2) = e32;
   }
};

class Mat4x2 : public Mat<prv::MatTraits<4, 2> >
{
   typedef Mat<prv::MatTraits<4, 2> > Base;

public:
   Mat4x2() : Base()
   {}

   Mat4x2(const float *init) : Base(init)
   {}

   Mat4x2(float e00, float e01, float e02, float e03,
          float e10, float e11, float e12, float e13) : Base()
   {
      Mat4x2 &m = *this;

      m(0, 0) = e00;  m(0, 1) = e01;  m(0, 2) = e02;  m(0, 3) = e03;
      m(1, 0) = e10;  m(1, 1) = e11;  m(1, 2) = e12;  m(1, 3) = e13;
   }
};

class Mat4x3 : public Mat<prv::MatTraits<4, 3> >
{
   typedef Mat<prv::MatTraits<4, 3> > Base;

public:
   Mat4x3() : Base()
   {}

   Mat4x3(const float *init) : Base(init)
   {}

   Mat4x3(float e00, float e01, float e02, float e03,
          float e10, float e11, float e12, float e13,
          float e20, float e21, float e22, float e23) : Base()
   {
      Mat4x3 &m = *this;

      m(0, 0) = e00;  m(0, 1) = e01;  m(0, 2) = e02;  m(0, 3) = e03;
      m(1, 0) = e10;  m(1, 1) = e11;  m(1, 2) = e12;  m(1, 3) = e13;
      m(2, 0) = e20;  m(2, 1) = e21;  m(2, 2) = e22;  m(2, 3) = e23;
   }
};
#endif

//
// Utilities
//

//! Return the translation matrix T(tx, ty, tz)
inline Mat4 Translate(float tx, float ty, float tz)
{
   Mat4  m;

   m(0, 3) = tx;
   m(1, 3) = ty;
   m(2, 3) = tz;
   m(3, 3) = 1.0f;

   return m;
}

//! Set the translation components in a matrix T(tx, ty, tz)
inline void SetTranslation(Mat4 &m, const Vec3 &t)
{
   m(0, 3) = t.X();
   m(1, 3) = t.Y();
   m(2, 3) = t.Z();
}

//! Return the translation vector from a matrix
inline Vec3 GetTranslation(const Mat4 &mat)
{
   return Vec3(mat(0, 3), mat(1, 3), mat(2, 3));
}

//! Return the translation matrix T(t.X(), t.Y(), t.Z())
inline Mat4 Translate(const Vec3 &t)
{
   return Translate(t.X(), t.Y(), t.Z());
}

//! Return the X vector from the matrix i.e. the result of xform * Vec3(1.0, 0.0, 0.0, 0.0)
inline Vec3 GetXVector(const Mat4 &mat)
{
   return Vec3(mat(0, 0), mat(1, 0), mat(2, 0));
}

//! Return the Y vector from the matrix i.e. the result of xform * Vec3(0.0, 1.0, 0.0, 0.0)
inline Vec3 GetYVector(const Mat4 &mat)
{
   return Vec3(mat(0, 1), mat(1, 1), mat(2, 1));
}

//! Return the Z vector from the matrix i.e. the result of xform * Vec3(0.0, 0.0, 1.0, 0.0)
inline Vec3 GetZVector(const Mat4 &mat)
{
   return Vec3(mat(0, 2), mat(1, 2), mat(2, 2));
}

//! Return the scale matrix S(sx, sy, sz)
inline Mat4 Scale(float sx, float sy, float sz)
{
   Mat4  m;

   m(0, 0) = sx;
   m(1, 1) = sy;
   m(2, 2) = sz;
   m(3, 3) = 1.0f;

   return m;
}

//! Return the scale matrix S(s.X(), s.Y(), s.Z())
inline Mat4 Scale(const Vec3 &s)
{
   return Scale(s.X(), s.Y(), s.Z());
}

//! Return a rotation matrix by angle a (degrees) about axis (x, y, z)
inline Mat4 Rotate(float a, float x, float y, float z)
{
   Mat4  m;

   float mag = sqrtf(x * x + y * y + z * z);

   if (mag > 0.0f)
   {
      float sina = sinf(a * (float)M_PI / 180.0f);
      float cosa = cosf(a * (float)M_PI / 180.0f);

      x /= mag;  y /= mag;  z /= mag;

      float xx = x * x,  yy = y * y, zz = z * z;
      float xy = x * y,  yz = y * z, zx = z * x;
      float xs = x * sina;
      float ys = y * sina;
      float zs = z * sina;
      float oneMinusCos = 1.0f - cosa;

      m(0, 0) = (oneMinusCos * xx) + cosa;
      m(0, 1) = (oneMinusCos * xy) - zs;
      m(0, 2) = (oneMinusCos * zx) + ys;
      m(0, 3) = 0.0f;

      m(1, 0) = (oneMinusCos * xy) + zs;
      m(1, 1) = (oneMinusCos * yy) + cosa;
      m(1, 2) = (oneMinusCos * yz) - xs;
      m(1, 3) = 0.0f;

      m(2, 0) = (oneMinusCos * zx) - ys;
      m(2, 1) = (oneMinusCos * yz) + xs;
      m(2, 2) = (oneMinusCos * zz) + cosa;
      m(2, 3) = 0.0f;

      m(3, 0) = 0.0f;
      m(3, 1) = 0.0f;
      m(3, 2) = 0.0f;
      m(3, 3) = 1.0f;
   }

   return m;
}

//! Return a rotation matrix by angle a (degrees) about axis
inline Mat4 Rotate(float a, const Vec3 &axis)
{
   return Rotate(a, axis.X(), axis.Y(), axis.Z());
}

//! Return the perspective projection matrix with fovy (degrees) field of view, aspect ratio and zNear and zFar planes as specified.
inline Mat4 Perspective(float fovy, float aspect, float zNear, float zFar)
{
   Mat4     m;
   float    sine, cot, deltaZ;
   float    radians = fovy / 2.0f * (float)M_PI / 180.0f;

   deltaZ = zFar - zNear;
   sine = sinf(radians);
   if ((deltaZ == 0) || (sine == 0) || (aspect == 0))
      return m;

   cot = cosf(radians) / sine;

   m(0, 0) = cot / aspect; m(1, 0) =   0; m(2, 0) =                          0; m(3, 0) =  0;
   m(0, 1) =            0; m(1, 1) = cot; m(2, 1) =                          0; m(3, 1) =  0;
   m(0, 2) =            0; m(1, 2) =   0; m(2, 2) =   -(zFar + zNear) / deltaZ; m(3, 2) = -1;
   m(0, 3) =            0; m(1, 3) =   0; m(2, 3) = -2 * zNear * zFar / deltaZ; m(3, 3) =  0;

   return m;
}

//! Return the projection matrix for view frustum of given dimensions.
inline Mat4 Frustum(float left, float right, float bottom, float top, float nearZ, float farZ)
{
   Mat4  m;

   float deltaX = right - left;
   float deltaY = top - bottom;
   float deltaZ = farZ - nearZ;

   if (nearZ  <= 0.0f || farZ <= 0.0f   ||
       deltaX <= 0.0f || deltaY <= 0.0f || deltaZ <= 0.0f)
      return m;

   m(0, 0) = 2.0f * nearZ / deltaX;
   m(1, 0) = 0.0f;
   m(2, 0) = 0.0f;
   m(3, 0) = 0.0f;

   m(1, 1) = 2.0f * nearZ / deltaY;
   m(0, 1) = 0.0f;
   m(2, 1) = 0.0f;
   m(3, 1) = 0.0f;

   m(0, 2) = (right + left) / deltaX;
   m(1, 2) = (top + bottom) / deltaY;
   m(2, 2) = -(nearZ + farZ) / deltaZ;
   m(3, 2) = -1.0f;

   m(2, 3) = -2.0f * nearZ * farZ / deltaZ;
   m(0, 3) = 0.0f;
   m(1, 3) = 0.0f;
   m(3, 3) = 0.0f;

   return m;
}

//! Return the orthographic (parallel) projection matrix for the view volume of the given dimensions
inline Mat4 Ortho(float left, float right, float bottom, float top, float nearZ, float farZ)
{
   Mat4  m;
   float deltaX = right - left;
   float deltaY = top - bottom;
   float deltaZ = farZ - nearZ;

   if ((deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f))
      return m;

   m(0, 0) = 2.0f / deltaX;
   m(0, 3) = -(right + left) / deltaX;
   m(1, 1) = 2.0f / deltaY;
   m(1, 3) = -(top + bottom) / deltaY;
   m(2, 2) = -2.0f / deltaZ;
   m(2, 3) = -(nearZ + farZ) / deltaZ;

   return m;
}

//! Return the inverse of an affine matrix
Mat4 Invert(const Mat4 &in);

//! Return the inverse.
Mat3 Invert(const Mat3 &in);

//! Generate a "lookat" matrix
Mat4 LookAt(const Vec3 &location, const Vec3 &lookAt, const Vec3 &up, Vec3 *realUp = 0);

//! @}
}

#endif
