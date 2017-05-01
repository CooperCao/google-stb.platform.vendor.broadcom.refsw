/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_NUMBER_H__
#define __BSG_NUMBER_H__

#include "bsg_common.h"

namespace bsg
{

// @cond
//! The class for all animatable numeric types (float, int etc).
template <class T>
class Number
{
public:
   Number() {}

   //! Make a T into a Number
   Number(T val) : m_value(val) {}

   //! Make a Number into a T
   operator T() const { return m_value; }

   bool operator!=(T value) { return m_value != value; }
   bool operator==(T value) { return m_value == value; }
   T operator*(T value)     { return m_value * value; }
   T operator/(T value)     { return m_value / value; }
   T operator+(T value)     { return m_value + value; }
   T operator-(T value)     { return m_value - value; }
   Number &operator=(T value)    { m_value = value; return *this; }
   Number &operator+=(T value)   { m_value += value; return *this; }
   Number &operator-=(T value)   { m_value -= value; return *this; }
   Number &operator*=(T value)   { m_value *= value; return *this; }
   Number &operator/=(T value)   { m_value /= value; return *this; }

private:
   T m_value;
};

typedef Number<float>    Float;
typedef Number<int>      Int;

// @endcond

}

#endif /* __BSG_NUMBER_H__ */
