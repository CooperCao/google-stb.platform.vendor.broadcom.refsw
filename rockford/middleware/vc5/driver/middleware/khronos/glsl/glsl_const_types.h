/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_CONST_TYPES_H
#define GLSL_CONST_TYPES_H

#include "interface/khronos/common/khrn_int_util.h"   // Shouldn't be here, but needed for other things. Fix later
#include "tools/v3d/qpu_float/qpu_float.h"
#include "helpers/gfx/gfx_util.h"

typedef uint32_t const_value;  // for internal representation of constant values

typedef uint32_t const_unsigned;  // for internal representation of unsigned values
typedef int32_t  const_signed;    // for internal use whenever signed values are needed

#define CONST_BOOL_TRUE 1
#define CONST_BOOL_FALSE 0

#define CONST_INT_SIGN_BIT 0x80000000

#define CONST_FLOAT_MINUS_ONE  0xbf800000
#define CONST_FLOAT_TWO        0x40000000
#define CONST_FLOAT_ONE        0x3f800000
#define CONST_FLOAT_ZERO       0x00000000
#define CONST_FLOAT_MINUS_ZERO 0x80000000

static inline const_value const_float_from_int(const_value i)
{
	return qpu_itof(i);
}

static inline const_value const_float_from_uint(const_value ui)
{
   return qpu_utof(ui);
}

static inline const_value const_int_from_float(const_value f)
{
   // "When constructors are used to convert a float to an int,
   // the fractional part of the floating-point value is dropped."
   // -- I take this to mean truncation, aka round to zero.
   bool carry; // ignored
   return qpu_ftoiz(f, &carry);
}

static inline const_value const_uint_from_float(const_value f)
{
   bool carry; // ignored
   return qpu_ftouz(f, &carry);
}

// the following two function should be safe in the sense that they preserve bit patterns
static inline const_signed const_signed_from_value(const_value ui)
{
   return (const_signed)ui;
}

static inline const_value const_value_from_signed(const_signed i)
{
   return (const_value)i;
}
#endif // CONST_TYPES_H
