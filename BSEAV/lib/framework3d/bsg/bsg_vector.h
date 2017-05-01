/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_VECTOR_H__
#define __BSG_VECTOR_H__

#include "bsg_common.h"

#include <stdint.h>
#include <memory.h>
#include <math.h>

#include <iostream>

namespace bsg
{

/** @addtogroup math Math
The BSG provides classes and methods to help with manipulating geometric objects in 3D.  This includes
support for arbitrarily sized vectors and square matrices, and for quaternions which are a convenient representation
of rotations used internally.
@{
*/

class Vec2;
class Vec3;
class Vec4;
class IVec2;
class IVec3;
class IVec4;

namespace prv
{

struct Vec2Traits
{
   enum { Size = 2 };
   typedef float  Elem_t;
   typedef void   Prev_t;
   typedef Vec2   This_t;
   typedef Vec3   Next_t;
};

struct Vec3Traits
{
   enum { Size = 3 };
   typedef float  Elem_t;
   typedef Vec2   Prev_t;
   typedef Vec3   This_t;
   typedef Vec4   Next_t;
};

struct Vec4Traits
{
   enum { Size = 4 };
   typedef float  Elem_t;
   typedef Vec3   Prev_t;
   typedef Vec4   This_t;
   typedef void   Next_t;
};

struct IVec2Traits
{
   enum { Size = 2 };
   typedef int32_t Elem_t;
   typedef void    Prev_t;
   typedef IVec2   This_t;
   typedef IVec3   Next_t;
};

struct IVec3Traits
{
   enum { Size = 3 };
   typedef int32_t Elem_t;
   typedef IVec2   Prev_t;
   typedef IVec3   This_t;
   typedef IVec4   Next_t;
};

struct IVec4Traits
{
   enum { Size = 4 };
   typedef int32_t Elem_t;
   typedef IVec3   Prev_t;
   typedef IVec4   This_t;
   typedef void    Next_t;
};

}

//! Template base class for vectors.
//!
//! To define a concrete vector class, derived from this base class by supplying Traits such that:
//! - Traits::Elem_t represents the type of vector components
//! - Traits::This_t is the derived class
//! - Traits::Prev_t is the class of a vector one smaller in size (or void)
//! - Traits::Next_t is the class of a vector one larger in size (or void)
//!
//! For example, to define a vector of 3 floats, a suitable Traits class (assuming that Vec2 and Vec4
//! are defined) might be:
//! @code
//! struct Vec3Traits
//! {
//!   enum { Size = 3 };
//!   typedef float  Elem_t;
//!   typedef Vec2   Prev_t;
//!   typedef Vec3   This_t;
//!   typedef Vec4   Next_t;
//! };
//! @endcode
//!
//! Applications should not create or use Vec<Traits> objects directly.  Prefer to use the
//! derived classes Vec2, Vec3 and Vec4 for example.
template <class Traits>
class Vec
{
public:
   typedef typename Traits::Elem_t Elem_t;
   typedef typename Traits::This_t This_t;
   typedef typename Traits::Prev_t Prev_t;
   typedef typename Traits::Next_t Next_t;

   enum { Size = Traits::Size };

   //! Constuct a vector initialised to zero.
   Vec()
   {
      for (uint32_t i = 0; i < Size; ++i)
         m_e[i] = 0;
   }

   //! Construct a vector with all elements initialised to the same value.
   Vec(Elem_t e)
   {
      for (uint32_t i = 0; i < Size; ++i)
         m_e[i] = e;
   }

   //! Construct a vector from an array of values.
   Vec(const Elem_t *init)
   {
      for (uint32_t i = 0; i < Size; ++i)
         m_e[i] = init[i];
   }

   //! Returns the elementwise negation of a vector
   This_t operator-() const
   {
      This_t   res;

      for (uint32_t i = 0; i < Size; ++i)
         res[i] = -m_e[i];

      return res;
   }

   //! Element-wise, adds a vector to this.
   This_t &operator+=(const Vec &rhs)
   {
      for (uint32_t i = 0; i < Size; ++i)
         m_e[i] += rhs.m_e[i];

      return static_cast<This_t &>(*this);
   }

   //! Element-wise, multiplies this by a scalar
   This_t &operator*=(float scalar)
   {
      for (uint32_t i = 0; i < Size; ++i)
         m_e[i] *= scalar;

      return static_cast<This_t &>(*this);
   }

   //! Adds vectors.
   This_t operator+(const Vec &rhs) const
   {
      This_t  res(m_e);

      res += rhs;

      return  res;
   }

   //! Element-wise, multiplies vector by scalar
   This_t operator*(float scalar) const
   {
      This_t  res(m_e);

      res *= scalar;

      return  res;
   }

   //! Subtracts a vector from this
   This_t &operator-=(const Vec &rhs)
   {
      for (uint32_t i = 0; i < Size; ++i)
         m_e[i] -= rhs.m_e[i];

      return static_cast<This_t &>(*this);
   }

   //! Subtracts vectors
   This_t operator-(const Vec &rhs) const
   {
      This_t  res(m_e);

      res -= rhs;

      return  res;
   }

   //! Multiply a vector element-wise by another
   This_t operator*(const Vec &rhs) const
   {
      This_t   res(m_e);

      for (uint32_t i = 0; i < Size; ++i)
         res[i] *= rhs[i];

      return res;
   }

   //! Divide a vector element-wise by another
   This_t operator/(const Vec &rhs) const
   {
      This_t   res(m_e);

      for (uint32_t i = 0; i < Size; ++i)
         res[i] /= rhs[i];

      return res;
   }

   //! Divides a vector element-wise by a scalar
   This_t operator/(Elem_t rhs) const
   {
      This_t   res;

      for (uint32_t i = 0; i < Size; ++i)
         res[i] = m_e[i] / rhs;

      return res;
   }

   //! Compare two vectors for equality
   bool operator==(const Vec &rhs) const
   {
      for (uint32_t i = 0; i < Size; ++i)
         if (m_e[i] != rhs.m_e[i])
            return false;

      return true;
   }

   //! Compare two vectors for inequality
   bool operator!=(const Vec &rhs) const
   {
      for (uint32_t i = 0; i < Size; ++i)
         if (m_e[i] != rhs.m_e[i])
            return true;

      return false;
   }

   //! Compute the dot product (sum of element-wise products) of vectors
   Elem_t Dot(const Vec &rhs) const
   {
      Elem_t   res = 0.0f;

      for (uint32_t i = 0; i < Size; ++i)
         res += (*this)[i] * rhs[i];

      return res;
   }

   //! Drop an element from the vector, so e.g. Drop() on a bsg::Vec3 returns a bsg::Vec2
   Prev_t Drop() const
   {
      return Prev_t(m_e);
   }

   //! Divide elements of a vector by the last element and drop the last element.
   Prev_t Proj() const
   {
      Prev_t   res;

      for (uint32_t i = 0; i < Prev_t::Size; ++i)
         res[i] = m_e[i] / m_e[Prev_t::Size];

      return res;
   }

   //! Returns a new vector which is the original with a new last element set to val.
   Next_t Lift(Elem_t val) const
   {
      Next_t  res;
      for (uint32_t i = 0; i < Size; ++i)
         res[i] = m_e[i];
      res[Size] = val;

      return res;
   }

   //! Read the i-th element of a vector (non const version)
   Elem_t       &operator[](uint32_t i)       { return m_e[i]; }

   //! Read the i-th element of a vector (const version)
   const Elem_t &operator[](uint32_t i) const { return m_e[i]; }

   //! Obtain raw pointer to element array (const version)
   const Elem_t  *Ptr() const { return m_e; }

   //! Obtain raw pointer to element array (non const version)
   Elem_t        *Ptr()       { return m_e; }

protected:
   Elem_t  m_e[Size];
};

//! Two element vector of floats
class Vec2 : public Vec<prv::Vec2Traits>
{
public:
   typedef Vec<prv::Vec2Traits> Base;

   //! All elements set to zero.
   Vec2() : Base()
   {}

   //! All elements set to argument value.
   Vec2(float e) : Base(e)
   {}

   //! Elements initialised from array.
   Vec2(const float *init) : Base(init)
   {}

   //! Elements initialised from arguments.
   Vec2(float x, float y)
   {
      m_e[0] = x;
      m_e[1] = y;
   }

   //! @name Element accessors with traditional names
   //! @{
   float X() const    { return (*this)[0]; }
   float Y() const    { return (*this)[1]; }

   float &X()         { return (*this)[0]; }
   float &Y()         { return (*this)[1]; }
   //! @}
};

//! Three element vector of floats
class Vec3 : public Vec<prv::Vec3Traits>
{
public:
   typedef Vec<prv::Vec3Traits> Base;

   //! All elements set to zero.
   Vec3() : Base()
   {}

   //! All elements set to argument value.
   Vec3(float e) : Base(e)
   {}

   //! Elements initialised from array.
   Vec3(const float *init) : Base(init)
   {}

   //! Elements initialised from arguments.
   Vec3(float x, float y, float z)
   {
      m_e[0] = x;
      m_e[1] = y;
      m_e[2] = z;
   }

   //! @name Element accessors with traditional names
   //! @{
   float X() const    { return (*this)[0]; }
   float Y() const    { return (*this)[1]; }
   float Z() const    { return (*this)[2]; }

   float &X()         { return (*this)[0]; }
   float &Y()         { return (*this)[1]; }
   float &Z()         { return (*this)[2]; }
   //! @}
};

//! Four element vector of floats
class Vec4 : public Vec<prv::Vec4Traits>
{
public:
   typedef Vec<prv::Vec4Traits> Base;

   //! All elements set to zero.
   Vec4() : Base()
   {}

   //! All elements set to argument value.
   Vec4(float e) : Base(e)
   {}

   //! Elements initialised from array.
   Vec4(const float *init) : Base(init)
   {}

   //! Elements initialised from arguments.
   Vec4(float x, float y, float z, float w)
   {
      m_e[0] = x;
      m_e[1] = y;
      m_e[2] = z;
      m_e[3] = w;
   }

   //! @name Element accessors with traditional names
   //! @{
   float X() const    { return (*this)[0]; }
   float Y() const    { return (*this)[1]; }
   float Z() const    { return (*this)[2]; }
   float W() const    { return (*this)[3]; }

   float &X()         { return (*this)[0]; }
   float &Y()         { return (*this)[1]; }
   float &Z()         { return (*this)[2]; }
   float &W()         { return (*this)[3]; }
   //! @}
};

//! Two element vector of integers
class IVec2 : public Vec<prv::IVec2Traits>
{
public:
   typedef Vec<prv::IVec2Traits> Base;

   //! All elements set to zero.
   IVec2() : Base()
   {}

   //! All elements set to argument value.
   IVec2(int32_t e) : Base(e)
   {}

   //! Elements initialised from array.
   IVec2(const int32_t *init) : Base(init)
   {}

   //! Elements initialised from arguments.
   IVec2(int32_t x, int32_t y)
   {
      m_e[0] = x;
      m_e[1] = y;
   }

   //! @name Element accessors with traditional names
   //! @{
   int32_t X() const    { return (*this)[0]; }
   int32_t Y() const    { return (*this)[1]; }

   int32_t &X()         { return (*this)[0]; }
   int32_t &Y()         { return (*this)[1]; }
   //! @}
};

//! Three element vector of integers
class IVec3 : public Vec<prv::IVec3Traits>
{
public:
   typedef Vec<prv::IVec3Traits> Base;

   //! All elements set to zero.
   IVec3() : Base()
   {}

   //! All elements set to argument value.
   IVec3(int32_t e) : Base(e)
   {}

   //! Elements initialised from array.
   IVec3(const int32_t *init) : Base(init)
   {}

   //! Elements initialised from arguments.
   IVec3(int32_t x, int32_t y, int32_t z)
   {
      m_e[0] = x;
      m_e[1] = y;
      m_e[2] = z;
   }

   //! @name Element accessors with traditional names
   //! @{
   int32_t X() const    { return (*this)[0]; }
   int32_t Y() const    { return (*this)[1]; }
   int32_t Z() const    { return (*this)[2]; }

   int32_t &X()         { return (*this)[0]; }
   int32_t &Y()         { return (*this)[1]; }
   int32_t &Z()         { return (*this)[2]; }
   //! @}
};

//! Four element vector of integers
class IVec4 : public Vec<prv::IVec4Traits>
{
public:
   typedef Vec<prv::IVec4Traits> Base;

   //! All elements set to zero.
   IVec4() : Base()
   {}

   //! All elements set to argument value.
   IVec4(int32_t e) : Base(e)
   {}

   //! Elements initialised from array.
   IVec4(const int32_t *init) : Base(init)
   {}

   //! Elements initialised from arguments.
   IVec4(int32_t x, int32_t y, int32_t z, int32_t w)
   {
      m_e[0] = x;
      m_e[1] = y;
      m_e[2] = z;
      m_e[3] = w;
   }

   //! @name Element accessors with traditional names
   //! @{
   int32_t X() const    { return (*this)[0]; }
   int32_t Y() const    { return (*this)[1]; }
   int32_t Z() const    { return (*this)[2]; }
   int32_t W() const    { return (*this)[3]; }

   int32_t &X()         { return (*this)[0]; }
   int32_t &Y()         { return (*this)[1]; }
   int32_t &Z()         { return (*this)[2]; }
   int32_t &W()         { return (*this)[3]; }
   //! @}
};

//! Calculates the square of the distance between two points.
template <class Traits>
float Distance2(const Vec<Traits> &lhs, const Vec<Traits> &rhs)
{
   float sum = 0.0f;

   for (uint32_t i = 0; i < Vec<Traits>::Size; ++i)
      sum += (lhs[i] - rhs[i]) * (lhs[i] - rhs[i]);

   return sum;
}

//! Calculates the pythagorean distance between two points.
template <class Traits>
float Distance(const Vec<Traits> &lhs, const Vec<Traits> &rhs)
{
   return sqrtf(Distance2(lhs, rhs));
}

//! Calculates the square of the length of a vector
template <class Traits>
float LengthSquared(const Vec<Traits> &vec)
{
   float sum = 0.0f;

   for (uint32_t i = 0; i < Vec<Traits>::Size; ++i)
      sum += vec[i] * vec[i];

   return sum;
}

//! Calculates the length of a vector
template <class Traits>
float Length(const Vec<Traits> &vec)
{
   return sqrtf(LengthSquared(vec));
}

//! Returns a the result of normalizing a vector (i.e. dividing all components)
template <class Traits>
typename Vec<Traits>::This_t Normalize(const Vec<Traits> &vec)
{
   typename Vec<Traits>::This_t res;

   float len = Length(vec);
   if (len != 0.0f)
   {
      float rlen = 1.0f / len;
      res = vec * rlen;
   }
   return res;
}

//! Returns the component-wise absolute value of the argument.
template <class Traits>
typename Traits::This_t Abs(const Vec<Traits> &vec)
{
   typename Vec<Traits>::This_t res;

   for (uint32_t i = 0; i < Traits::Size; ++i)
      res[i] = fabs(vec[i]);

   return res;
}

//! Calculates the vector cross product.
inline Vec3 Cross(const Vec3 &lhs, const Vec3 &rhs)
{
   return Vec3(lhs.Y() * rhs.Z() - lhs.Z() * rhs.Y(),
               lhs.Z() * rhs.X() - lhs.X() * rhs.Z(),
               lhs.X() * rhs.Y() - lhs.Y() * rhs.X());
}

//! Read a Vec3
inline std::istream &operator>>(std::istream &is, Vec3 &v)
{
    return is >> v.X() >> v.Y() >> v.Z();
}

// Read a Vec2
inline std::istream &operator>>(std::istream &is, Vec2 &v)
{
    return is >> v.X() >> v.Y();
}

//! Write a Vec4
inline std::ostream &operator<<(std::ostream &os, const Vec4 &v)
{
   return os << v.X() << ", " << v.Y() << ", " << v.Z() << ", " << v.W() << '\n';
}

//! Write a Vec3
inline std::ostream &operator<<(std::ostream &os, const Vec3 &v)
{
   return os << v.X() << ", " << v.Y() << ", " << v.Z() << '\n';
}

// Write a Vec2
inline std::ostream &operator<<(std::ostream &os, const Vec2 &v)
{
   return os << v.X() << ", " << v.Y() << '\n';
}

//! @}

}

#endif
