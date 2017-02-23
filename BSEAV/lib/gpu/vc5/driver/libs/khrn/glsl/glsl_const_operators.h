/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_CONST_OPERATORS_H
#define GLSL_CONST_OPERATORS_H

#include "glsl_const_types.h"

#include "libs/sim/sfu/sfu.h"
#include "libs/sim/qpu_float/qpu_float.h"

//
// Operators on base types.
//

static inline const_value op_i_negate(const_value operand)
{
   return const_value_from_signed(-const_signed_from_value(operand));
}

static inline const_value op_f_negate(const_value operand)
{
   return operand ^ (1u << (sizeof(const_value) * 8 - 1));
}

static inline const_value op_logical_not(const_value operand)
{
   return !operand;
}

static inline const_value op_bitwise_not(const_value operand)
{
   return ~operand;
}

static inline const_value op_i_mul(const_value left, const_value right)
{
   return const_value_from_signed(const_signed_from_value(left) * const_signed_from_value(right));
}

static inline const_value op_u_mul(const_value left, const_value right)
{
   return left * right;
}

static inline const_value op_f_mul(const_value left, const_value right)
{
   return qpu_fmul(left, right, true);
}

static inline const_value op_i_div(const_value left, const_value right)
{
   if (const_signed_from_value(right) == 0)
      return const_signed_from_value(left) < 0 ? 1 : -1;
   else
      return const_value_from_signed(const_signed_from_value(left) / const_signed_from_value(right));
}

static inline const_value op_u_div(const_value left, const_value right)
{
   if (right == 0)
      return 0xffffffff;
   else
      return left / right;
}

static inline const_value op_f_div(const_value left, const_value right)
{
   return qpu_fmul(left, sfu_recip(right, true), true);
}

static inline const_value op_i_add(const_value left, const_value right)
{
   return left + right;
}
static inline const_value op_f_add(const_value left, const_value right)
{
   return qpu_fadd(left, right, true);
}

static inline const_value op_i_sub(const_value left, const_value right)
{
   return left - right;
}
static inline const_value op_f_sub(const_value left, const_value right)
{
   return qpu_fsub(left, right, true);
}

static inline const_value op_i_less_than(const_value left, const_value right)
{
   return const_signed_from_value(left) < const_signed_from_value(right);
}
static inline const_value op_u_less_than(const_value left, const_value right)
{
   return left < right;
}
static inline const_value op_f_less_than(const_value left, const_value right)
{
   const_value cmpres = qpu_fcmp(left, right, true);
   return (cmpres >= 0x80000001);
}

static inline const_value op_i_less_than_equal(const_value left, const_value right)
{
   return const_signed_from_value(left) <= const_signed_from_value(right);
}
static inline const_value op_u_less_than_equal(const_value left, const_value right)
{
   return left <= right;
}
static inline const_value op_f_less_than_equal(const_value left, const_value right)
{
   const_value cmpres = qpu_fcmp(left, right, true);
   return (cmpres >= 0x80000000) || (cmpres==0);
}

static inline const_value op_i_greater_than(const_value left, const_value right)
{
   return const_signed_from_value(left) > const_signed_from_value(right);
}
static inline const_value op_u_greater_than(const_value left, const_value right)
{
   return left > right;
}
static inline const_value op_f_greater_than(const_value left, const_value right)
{
   const_value cmpres = qpu_fcmp(right, left, true);
   return (cmpres >= 0x80000001);
}

static inline const_value op_i_greater_than_equal(const_value left, const_value right)
{
   return const_signed_from_value(left) >= const_signed_from_value(right);
}
static inline const_value op_u_greater_than_equal(const_value left, const_value right)
{
   return left >= right;
}
static inline const_value op_f_greater_than_equal(const_value left, const_value right)
{
   const_value cmpres = qpu_fcmp(right, left, true);
   return (cmpres >= 0x80000000) || (cmpres==0);
}

static inline const_value op_b_equal(const_value left, const_value right) {
   return left == right;
}

static inline const_value op_f_equal(const_value left, const_value right) {
   const_value cmpres = qpu_fcmp(left, right, true);
   return (cmpres == 0) || (cmpres == 0x80000000);
}

static inline const_value op_i_equal(const_value left, const_value right) {
   return left == right;
}

static inline const_value op_b_not_equal(const_value left, const_value right) {
   return left != right;
}

static inline const_value op_f_not_equal(const_value left, const_value right) {
   const_value cmpres = qpu_fcmp(left, right, true);
   return (cmpres != 0) && (cmpres != 0x80000000);
}

static inline const_value op_i_not_equal(const_value left, const_value right) {
   return left != right;
}

static inline const_value op_logical_and(const_value left, const_value right) {
   return left && right;
}

static inline const_value op_logical_xor(const_value left, const_value right) {
   return (left ? CONST_BOOL_TRUE : CONST_BOOL_FALSE) != (right ? CONST_BOOL_TRUE : CONST_BOOL_FALSE);
}

static inline const_value op_logical_or(const_value left, const_value right) {
   return left || right;
}

static inline const_value op_bitwise_and(const_value lhs, const_value rhs) {
   return lhs & rhs;
}

static inline const_value op_bitwise_xor(const_value lhs, const_value rhs) {
   return lhs ^ rhs;
}

static inline const_value op_bitwise_or(const_value lhs, const_value rhs) {
   return lhs | rhs;
}

static inline const_value op_bitwise_shl(const_value left, const_value right) {
   return const_value_from_signed(const_signed_from_value(left) << (right & 31));
}

static inline const_value op_i_bitwise_shr(const_value left, const_value right) {
   return const_value_from_signed(const_signed_from_value(left) >> (right & 31));
}

static inline const_value op_u_bitwise_shr(const_value left, const_value right) {
   return left >> (right & 31);
}

static inline const_value op_bitwise_ror(const_value left, const_value right) {
   right &= 31;
   return right == 0 ? left : (left >> right) | (left << (32 - right));
}

static inline const_value op_i_min(const_value left, const_value right) {
   return (const_signed_from_value(left) < const_signed_from_value(right)) ? left : right;
}

static inline const_value op_u_min(const_value left, const_value right) {
   return left < right ? left : right;
}

static inline const_value op_f_min(const_value left, const_value right) {
   return qpu_fmin(left, right, true);
}

static inline const_value op_i_max(const_value left, const_value right)
{
   return (const_signed_from_value(left) > const_signed_from_value(right)) ? left : right;
}
static inline const_value op_u_max(const_value left, const_value right)
{
   return left > right ? left : right;
}
static inline const_value op_f_max(const_value left, const_value right)
{
   return qpu_fmax(left, right, true);
}

static inline const_value op_f_abs(const_value operand)
{
   return qpu_fabs(operand);
}

//
// Linear algebraic multiply.
//
static inline void op_mul__const_matXxY__const_matZxY__const_matXxZ(const int X, const int Y, const int Z, const_value *result, const_value *left, const_value *right)
{
   const_value mul;

   for (int i = 0; i < X; i++)
   {
      for (int j = 0; j < Y; j++)
      {
         result[j + i*Y] = CONST_FLOAT_ZERO;

         for (int k = 0; k < Z; k++)
         {
            mul = qpu_fmul(left[k*Y + j], right[i*Z + k], true);
            result[j + i*Y] = qpu_fadd(result[j + i*Y], mul, true);
         }
      }
   }
}

static inline const_value op_rsqrt(const_value operand)
{
   return sfu_recipsqrt(operand, true);
}

static inline const_value op_sqrt(const_value operand)
{
#if V3D_VER_AT_LEAST(3,3,0,0)
   return qpu_fmul(operand, sfu_recipsqrt2(operand, true), true);
#else
   return sfu_recip(op_rsqrt(operand), true);
#endif
}

static inline const_value op_recip(const_value operand)
{
   return sfu_recip(operand, true);
}

static inline const_value op_log2(const_value operand)
{
   return sfu_log(operand, true);
}

static inline const_value op_exp2(const_value operand)
{
   return sfu_exp(operand, true);
}

static inline const_value op_ceil(const_value operand)
{
   return qpu_fceil(operand, true);
}

static inline const_value op_floor(const_value operand)
{
   return qpu_ffloor(operand, true);
}

static inline const_value op_trunc(const_value operand)
{
   return qpu_ftrunc(operand, true);
}

static inline const_value op_round(const_value operand)
{
   return qpu_fround(operand, true);
}

static inline const_value op_floattoint_trunc(const_value operand)
{
   bool carry; // ignored
   return qpu_ftoiz(operand, &carry);
}

static inline const_value op_floattouint(const_value operand)
{
   bool carry; // ignored
   return qpu_ftouz(operand, &carry);
}

static inline const_value op_floattoint_nearest(const_value operand)
{
   bool carry; // ignored
   return qpu_ftoin(operand, &carry);
}

static inline const_value op_inttofloat(const_value operand)
{
   return qpu_itof(operand);
}

static inline const_value op_uinttofloat(const_value operand)
{
   return qpu_utof(operand);
}

static inline const_value op_i_rem(const_value left, const_value right)
{
   if (const_signed_from_value(right) == 0)
      return left;
   else
      return const_value_from_signed(const_signed_from_value(left) % const_signed_from_value(right));
}
static inline const_value op_u_rem(const_value left, const_value right)
{
   if (right == 0)
      return left;
   else
      return left % right;
}

static inline const_value op_fpack(const_value f1, const_value f2) {
   const_value lo = qpu_float_pack16(f1);
   const_value hi = qpu_float_pack16(f2);
   return (hi << 16) | lo;
}

static inline const_value op_funpacka(const_value ui) {
   return qpu_float_unpack16(ui & 0xFFFF);
}

static inline const_value op_funpackb(const_value ui) {
   return qpu_float_unpack16(ui >> 16);
}

static inline const_value op_clz(const_value ui) {
   if (ui == 0) return 32;

   unsigned ret = 0;
   while (!(ui & 0x800000000)) {
      ret++;
      ui <<= 1;
   }
   return ret;
}

static inline const_value op_sin(const_value operand) {
   const_value one_on_pi = 0x3ea2f983;
   const_value x = op_f_mul(operand, one_on_pi);
   const_value y = op_round(x);
   x = op_f_sub(x, y);
   const_value sfu_res = sfu_sin(x, true);
   const_value i = op_floattoint_nearest(y);
   const_value il31 = (i << 31);
   return sfu_res ^ il31;
}

static inline const_value op_cos(const_value operand) {
   const_value sin_angle = op_f_add(operand, 0x3fc90fdb /* pi/2 */);
   return op_sin(sin_angle);
}

static inline const_value op_tan(const_value operand) {
   const_value sin = op_sin(operand);
   const_value cos = op_cos(operand);
   const_value one_on_cos = op_recip(cos);
   return op_f_mul(sin, one_on_cos);
}

#endif // CONST_OPERATORS_H
