/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "DflowScalars.h"
#include "dflib.h"

namespace bvk {

///////////////////////////////////////////////////////////////////////
// Local function helpers that allow us to write readable code
///////////////////////////////////////////////////////////////////////

static inline DflowScalars constU(uint32_t val, const DflowScalars &anything)
{
   return DflowScalars::ConstantUInt(anything.GetBuilder(), val);
}

static inline DflowScalars constU(uint32_t val, const DflowBuilder &builder)
{
   return DflowScalars::ConstantUInt(builder, val);
}

static inline DflowScalars constI(int32_t val, const DflowScalars &anything)
{
   return DflowScalars::ConstantInt(anything.GetBuilder(), val);
}

static inline DflowScalars constI(int32_t val, const DflowBuilder &builder)
{
   return DflowScalars::ConstantInt(builder, val);
}

static inline DflowScalars constF(float val, const DflowScalars &anything)
{
   return DflowScalars::ConstantFloat(anything.GetBuilder(), val);
}

static inline DflowScalars constF(float val, const DflowBuilder &builder)
{
   return DflowScalars::ConstantFloat(builder, val);
}

static inline DflowScalars fabs(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_ABS, x);
}

static inline DflowScalars max(const DflowScalars &x, const DflowScalars &y)
{
   return DflowScalars::BinaryOp(DATAFLOW_MAX, x, y);
}

static inline DflowScalars max(const DflowScalars &x, float y)
{
   return DflowScalars::BinaryOp(DATAFLOW_MAX, x, DflowScalars::ConstantFloat(x.GetBuilder(), y));
}

static inline DflowScalars max(float x, const DflowScalars &y)
{
   return DflowScalars::BinaryOp(DATAFLOW_MAX, DflowScalars::ConstantFloat(y.GetBuilder(), x), y);
}

static inline DflowScalars max(const DflowScalars &x, int y)
{
   return DflowScalars::BinaryOp(DATAFLOW_MAX, x, DflowScalars::ConstantInt(x.GetBuilder(), y));
}

static inline DflowScalars max(int x, const DflowScalars &y)
{
   return DflowScalars::BinaryOp(DATAFLOW_MAX, DflowScalars::ConstantInt(y.GetBuilder(), x), y);
}

static inline DflowScalars min(const DflowScalars &x, const DflowScalars &y)
{
   return DflowScalars::BinaryOp(DATAFLOW_MIN, x, y);
}

static inline DflowScalars min(const DflowScalars &x, float y)
{
   return DflowScalars::BinaryOp(DATAFLOW_MIN, x, DflowScalars::ConstantFloat(x.GetBuilder(), y));
}

static inline DflowScalars min(float x, const DflowScalars &y)
{
   return DflowScalars::BinaryOp(DATAFLOW_MIN, DflowScalars::ConstantFloat(y.GetBuilder(), x), y);
}

static inline DflowScalars isnan(const DflowScalars &x)
{
   return DflowScalars::IsNaN(x);
}

static inline DflowScalars reinterpi(const DflowScalars &x)
{
   return DflowScalars::Reinterpret(DF_INT, x);
}

static inline DflowScalars reinterpu(const DflowScalars &x)
{
   return DflowScalars::Reinterpret(DF_UINT, x);
}

static inline DflowScalars reinterpf(const DflowScalars &x)
{
   return DflowScalars::Reinterpret(DF_FLOAT, x);
}

static inline DflowScalars recip(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_RCP, x);
}

static inline DflowScalars inversesqrt(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_RSQRT, x);
}

static inline DflowScalars sqrt(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_SQRT, x);
}

static inline DflowScalars trunc(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_TRUNC, x);
}

static inline DflowScalars round(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_NEAREST, x);
}

static inline DflowScalars floor(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_FLOOR, x);
}

static inline DflowScalars ceil(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_CEIL, x);
}

static inline DflowScalars itof(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_ITOF, x);
}

static inline Dflow itof(const Dflow &x)
{
   return Dflow::UnaryOp(DATAFLOW_ITOF, x);
}

static inline DflowScalars utof(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_UTOF, x);
}

static inline DflowScalars ftoi(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_FTOI_TRUNC, x);
}

static inline DflowScalars ftou(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_FTOU, x);
}

static inline DflowScalars bitnot(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_BITWISE_NOT, x);
}

static inline DflowScalars clz(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_CLZ, x);
}

static inline DflowScalars fpack(const DflowScalars &x, const DflowScalars &y)
{
   return DflowScalars::BinaryOp(DATAFLOW_FPACK, x, y);
}

static inline DflowScalars funpackA(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_FUNPACKA, x);
}

static inline DflowScalars funpackB(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_FUNPACKB, x);
}

static inline DflowScalars cond(const DflowScalars &c, const DflowScalars &iftrue, const DflowScalars &iffalse)
{
   return DflowScalars::CondOp(c, iftrue, iffalse);
}

static inline DflowScalars cond(const DflowScalars &c, float iftrue, const DflowScalars &iffalse)
{
   return DflowScalars::CondOp(c, DflowScalars::ConstantFloat(c.GetBuilder(), iftrue), iffalse);
}

static inline DflowScalars cond(const DflowScalars &c, const DflowScalars &iftrue, float iffalse)
{
   return DflowScalars::CondOp(c, iftrue, DflowScalars::ConstantFloat(c.GetBuilder(), iffalse));
}

static inline DflowScalars cond(const DflowScalars &c, float iftrue, float iffalse)
{
   return DflowScalars::CondOp(c, DflowScalars::ConstantFloat(c.GetBuilder(), iftrue),
                                  DflowScalars::ConstantFloat(c.GetBuilder(), iffalse));
}

static inline DflowScalars findUMSB(const DflowScalars &x)
{
   // If x is 0, the result is - 1
   auto i31 = constI(31, x);
   return i31 - reinterpi(clz(x));
}

static inline DflowScalars findSMSB(const DflowScalars &x)
{
   // For positive numbers, the result will be the bit number of the most significant 1-bit.
   // For negative numbers, the result will be the bit number of the most significant 0-bit.
   // For a value of 0 or -1, the result is -1.
   auto i0 = constI(0, x);

   auto v = cond(x < i0, bitnot(x), x);
   return findUMSB(reinterpu(v));
}

static inline DflowScalars findLSB(const DflowScalars &x)
{
   return findUMSB(reinterpu(x) & reinterpu(-x));
}

static inline DflowScalars isinf(const DflowScalars &x)
{
   // Inf means exponent is 0xFF or 0xFF and mantissa = 0
   auto inf = reinterpf(constU(0x7F800000, x));
   return fabs(x) == inf;
}

static inline DflowScalars mod(const DflowScalars &x, const DflowScalars &y)
{
   return x - y * floor(x / y);
}

static inline DflowScalars iabs(const DflowScalars &x)
{
   auto xs   = x.Signed();
   auto zero = constI(0, x);

   return cond(xs < zero, -xs, xs);
}

static inline DflowScalars nmax(const DflowScalars &x, const DflowScalars &y)
{
   // Semantics the same as max() on our h/w
   return max(x, y);
}

static inline DflowScalars nmin(const DflowScalars &x, const DflowScalars &y)
{
   // Semantics the same as min() on our h/w
   return min(x, y);
}

static inline DflowScalars clamp(const DflowScalars &x, const DflowScalars &minVal, const DflowScalars &maxVal)
{
   return min(max(x, minVal), maxVal);
}

static inline DflowScalars clamp(const DflowScalars &x, float minVal, const DflowScalars &maxVal)
{
   return min(max(x, minVal), maxVal);
}

static inline DflowScalars clamp(const DflowScalars &x, const DflowScalars &minVal, float maxVal)
{
   return min(max(x, minVal), maxVal);
}

static inline DflowScalars clamp(const DflowScalars &x, float minVal, float maxVal)
{
   return min(max(x, minVal), maxVal);
}

static inline DflowScalars nclamp(const DflowScalars &x, const DflowScalars &minVal, const DflowScalars &maxVal)
{
   return nmin(nmax(x, minVal), maxVal);
}

static inline DflowScalars fmix(const DflowScalars &x, const DflowScalars &y, const DflowScalars &a)
{
   return x + (y - x) * a;
}

static inline DflowScalars imix(const DflowScalars &x, const DflowScalars &y, const DflowScalars &a)
{
   return x + (y - x) * a;
}

static inline DflowScalars step(const DflowScalars &edge, const DflowScalars &x)
{
   return cond(x < edge, 0.0f, 1.0f);
}

static inline DflowScalars smoothstep(const DflowScalars &edge0, const DflowScalars &edge1, const DflowScalars &x)
{
   auto t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
   return t * t * (3.0f - 2.0f * t);
}

static inline DflowScalars log2(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_LOG2, x);
}

static inline DflowScalars log(const DflowScalars &x)
{
   auto x3F317218 = constU(0x3F317218, x);

   auto loge2 = reinterpf(x3F317218);
   return log2(x) * loge2;
}

static inline DflowScalars exp2(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_EXP2, x);
}

static inline DflowScalars exp(const DflowScalars &x)
{
   auto x3FB8AA3B = constU(0X3FB8AA3B, x);

   auto log2e = reinterpf(x3FB8AA3B);
   return exp2(x * log2e);
}

static inline DflowScalars pow(const DflowScalars &x, const DflowScalars &y)
{
   return exp2(y * log2(x));
}

static inline DflowScalars sin(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_SIN, x);
}

static inline DflowScalars cos(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_COS, x);
}

static inline DflowScalars tan(const DflowScalars &x)
{
   return DflowScalars::UnaryOp(DATAFLOW_TAN, x);
}

static inline DflowScalars sinh(const DflowScalars &x)
{
   return 0.5f * (exp(x) - exp(-x));
}

static inline DflowScalars cosh(const DflowScalars &x)
{
   return 0.5f * (exp(x) + exp(-x));
}

static inline DflowScalars tanh(const DflowScalars &x)
{
   return sinh(x) / cosh(x);
}

static inline DflowScalars asinh(const DflowScalars &x)
{
   return log(x + sqrt(x * x + 1.0f));
}

static inline DflowScalars acosh(const DflowScalars &x)
{
   return log(x + sqrt((x - 1.0f) * (x + 1.0f)));
}

static inline DflowScalars atanh(const DflowScalars &x)
{
   return 0.5f * log((1.0f + x) / (1.0f - x));
}

static inline DflowScalars atan(const DflowScalars &yOverX)
{
   auto &builder = yOverX.GetBuilder();

   const float T3PO8 = 2.414213562373f;
   const float TPO8  = 0.414213562373f;
   const float PO2   = 1.570796326794f;
   const float PO4   = 0.785398163397f;

   auto x80000000 = DflowScalars::ConstantUInt(builder, 0x80000000);

   auto c = DflowScalars::ConstantFloat(builder, 0.0f);

   auto sgnX = reinterpu(yOverX) & x80000000;
   auto x    = fabs(yOverX);

   // if (x > T3PO8)     { x = -1.0 / x;              c = PO2; }
   // else if (x > TPO8) { x = (x - 1.0) / (x + 1.0); c = PO4; }
   auto gtTPO8  = x > TPO8;
   auto xElse = cond(gtTPO8, (x - 1.0f) / (x + 1.0f), x);
   auto cElse = cond(gtTPO8, PO4, c);

   auto gtT3PO8 = x > T3PO8;
   x = cond(gtT3PO8, -recip(x), xElse);
   c = cond(gtT3PO8, PO2, cElse);

   auto z = x * x;
   return reinterpf(sgnX ^ reinterpu(c + x + x * z *
                                     (((8.05374449538e-2f  * z
                                      - 1.38776856032e-1f) * z
                                      + 1.99777106478e-1f) * z
                                      - 3.33329491539e-1f)));
}

DflowScalars inline atan(const DflowScalars &y, const DflowScalars &x)
{
   auto &builder = x.GetBuilder();

   auto PI  = DflowScalars::ConstantFloat(builder, 3.1415926535897932384626433832795f);
   auto c31 = DflowScalars::ConstantUInt(builder, 31);
   auto one = DflowScalars::ConstantFloat(builder, 1.0f);
   auto x80000000 = DflowScalars::ConstantUInt(builder, 0x80000000);

   // Compute atan2 by working out an offset based on the quadrant and calling
   // atan(y/x). This gives correct values unless y/x == #NaN, so resolve those
   // cases separately.
   //
   // Quadrant offset only gets correct results if -0.0 < +0.0 so we use bitops
   // rather than fcmp. Constructing the offset by crazy masking is just because
   // it's fast.
   auto xSgnMask  = reinterpi(x) >> c31;
   auto yQuadSign = reinterpi(y) & x80000000;

   // Correct for y/x == #NaN. Ignore 0/0 because results are undefined, so we
   // only need to correct #Inf/#Inf
   auto signBit = (reinterpi(x) ^ reinterpi(y)) & x80000000;
   auto bothInf = isinf(x) && (fabs(x) == fabs(y));
   auto yOnX = cond(bothInf, reinterpf(signBit + reinterpi(one)), y / x);

   auto quadrantOffset = reinterpi(PI) & xSgnMask;
   quadrantOffset = quadrantOffset ^ yQuadSign;

   return reinterpf(quadrantOffset) + atan(yOnX);
}

static inline DflowScalars asin(const DflowScalars &x)
{
   auto &builder = x.GetBuilder();

   auto x80000000 = DflowScalars::ConstantUInt(builder, 0x80000000u);

   auto sgnX = reinterpu(x) & x80000000;

   auto ax = fabs(x);

   auto condition = ax > 0.5f;
   auto c = cond(condition, 1.570796326794f, 0.0f);
   auto f = cond(condition, -2.0f, 1.0f);
   ax = cond(condition, inversesqrt(2.0f / (1.0f - ax)), ax);

   auto z = ax * ax;
   auto r = ((((4.2163199048E-2f  * z
              + 2.4181311049E-2f) * z
              + 4.5470025998E-2f) * z
              + 7.4953002686E-2f) * z
              + 1.6666752422E-1f) * z * ax
              + ax;

   return reinterpf(sgnX ^ reinterpu(c + f * r));
}

static inline DflowScalars acos(const DflowScalars &x)
{
   return 2.0f * atan(inversesqrt((1.0f + x) / (1.0f - x)));
}

static inline DflowScalars degrees(const DflowScalars &radians)
{
   return 57.29577951f * radians;
}

static inline DflowScalars radians(const DflowScalars &degree)
{
   return 0.01745329252f * degree;
}

static inline DflowScalars fsign(const DflowScalars &x)
{
   auto zeroOrOne = cond(x == 0.0f, 0.0f, 1.0f);
   return cond(x < 0.0f, -zeroOrOne, zeroOrOne);
}

static inline DflowScalars ssign(const DflowScalars &x)
{
   auto i0 = DflowScalars::ConstantInt(x.GetBuilder(), 0);
   auto i1  = DflowScalars::ConstantInt(x.GetBuilder(), 1);

   auto zeroOrOne = cond(x == i0, i0, i1);
   return cond(x < i0, -zeroOrOne, zeroOrOne);
}

static inline DflowScalars fract(const DflowScalars &x)
{
   return x - floor(x);
}

static inline DflowScalars fma(const DflowScalars &a, const DflowScalars &b, const DflowScalars &c)
{
   return a * b + c;
}

static inline DflowScalars dot(const DflowScalars &v1, const DflowScalars &v2)
{
   assert(v1.Size() == v2.Size());

   DflowScalars intermediate = v1 * v2;
   Dflow        result       = intermediate[0];

   for (uint32_t i = 1; i < intermediate.Size(); ++i)
      result = result + intermediate[i];

   return DflowScalars(v1.GetBuilder(), result);
}

static inline DflowScalars cross(const DflowScalars &x, const DflowScalars &y)
{
   DflowScalars res(x.GetBuilder(), 3);
   res[0] = x[1] * y[2] - x[2] * y[1];
   res[1] = x[2] * y[0] - x[0] * y[2];
   res[2] = x[0] * y[1] - x[1] * y[0];
   return res;
}

static inline DflowScalars length(const DflowScalars &x)
{
   if (x.Size() == 1)
      return fabs(x);
   else
      return recip(inversesqrt(dot(x, x)));
}

static inline DflowScalars distance(const DflowScalars &p0, const DflowScalars &p1)
{
   return length(p0 - p1);
}

static inline DflowScalars normalize(const DflowScalars &x)
{
   if (x.Size() == 1)
      return fsign(x);
   else
      return x * inversesqrt(dot(x, x));
}

static inline DflowScalars faceforward(const DflowScalars &N, const DflowScalars &I, const DflowScalars &Nref)
{
   return cond(dot(Nref, I) < 0.0f, N, -N);
}

static inline DflowScalars reflect(const DflowScalars &I, const DflowScalars &N)
{
   return I - 2.0f * dot(N, I) * N;
}

static inline DflowScalars refract(const DflowScalars &I, const DflowScalars &N, const DflowScalars &eta)
{
   DflowScalars d = dot(N, I);
   DflowScalars k = 1.0f - eta * eta * (1.0f - d * d);

   return cond(k < 0.0f, 0.0f, eta * I - (eta * d + sqrt(k)) * N);
}

static inline DflowScalars dFdx(const DflowScalars &p)
{
   return DflowScalars::UnaryOp(DATAFLOW_FDX, p);
}

static inline DflowScalars dFdy(const DflowScalars &p)
{
   return DflowScalars::UnaryOp(DATAFLOW_FDY, p);
}

static inline DflowScalars fwidth(const DflowScalars &p)
{
   return fabs(dFdx(p)) + fabs(dFdy(p));
}

static inline DflowScalars packSnorm2x16(const DflowScalars &v)
{
   assert(v.Size() == 2);
   Dataflow *res = dflib_packSnorm2x16(v[0], v[1]);
   return DflowScalars(v.GetBuilder(), 1, &res);
}

static inline DflowScalars unpackSnorm2x16(const DflowScalars &p)
{
   assert(p.Size() == 1);
   DflowScalars res(p.GetBuilder(), 2);
   dflib_unpackSnorm2x16(res.Data(), p[0]);
   return res;
}

static inline DflowScalars packUnorm2x16(const DflowScalars &v)
{
   assert(v.Size() == 2);
   Dataflow *res = dflib_packUnorm2x16(v[0], v[1]);
   return DflowScalars(v.GetBuilder(), 1, &res);
}

static inline DflowScalars unpackUnorm2x16(const DflowScalars &p)
{
   assert(p.Size() == 1);
   DflowScalars res(p.GetBuilder(), 2);
   dflib_unpackUnorm2x16(res.Data(), p[0]);
   return res;
}

static inline DflowScalars packHalf2x16(const DflowScalars &v)
{
   assert(v.Size() == 2);
   Dataflow *res = dflib_packHalf2x16(v[0], v[1]);
   return DflowScalars(v.GetBuilder(), 1, &res);
}

static inline DflowScalars unpackHalf2x16(const DflowScalars &p)
{
   assert(p.Size() == 1);
   DflowScalars res(p.GetBuilder(), 2);
   dflib_unpackHalf2x16(res.Data(), p[0]);
   return res;
}

static inline DflowScalars packUnorm4x8(const DflowScalars &v)
{
   assert(v.Size() == 4);
   Dataflow *res = dflib_packUnorm4x8(v[0], v[1], v[2], v[3]);
   return DflowScalars(v.GetBuilder(), 1, &res);
}

static inline DflowScalars packSnorm4x8(const DflowScalars &v)
{
   assert(v.Size() == 4);
   Dataflow *res = dflib_packSnorm4x8(v[0], v[1], v[2], v[3]);
   return DflowScalars(v.GetBuilder(), 1, &res);
}

static inline DflowScalars unpackUnorm4x8(const DflowScalars &p)
{
   assert(p.Size() == 1);
   DflowScalars res(p.GetBuilder(), 4);
   dflib_unpackUnorm4x8(res.Data(), p[0]);
   return res;
}

static inline DflowScalars unpackSnorm4x8(const DflowScalars &p)
{
   assert(p.Size() == 1);
   DflowScalars res(p.GetBuilder(), 4);
   dflib_unpackSnorm4x8(res.Data(), p[0]);
   return res;
}

static inline DflowScalars maxInVec(const DflowScalars &v)
{
   if (v.Size() == 1)
      return fabs(v.X());

   std::vector<DflowScalars> arr(v.Size(), DflowScalars(v.GetBuilder()));

   for (uint32_t i = 0; i < v.Size(); i++)
      arr[i] = v.Slice(i, 1);

   DflowScalars ret(v.GetBuilder(), 1);

   uint32_t i = 0;
   for (i = 0; i < v.Size() - 1; i++)
      arr[i + 1] = max(fabs(arr[i]), fabs(arr[i + 1]));

   return arr[i];
}

static inline DflowScalars maxInVec3(const DflowScalars &v)
{
   return max(fabs(v.X()), max(fabs(v.Y()), fabs(v.Z())));
}

static inline DflowScalars prepareCube3Coord(const DflowScalars &coord)
{
   return coord / maxInVec3(coord);
}

// Like the vec3 version but pass the 3-component through. Used for shadow and array
static inline DflowScalars prepareCube4Coord(const DflowScalars &coord)
{
   DflowScalars r(coord.GetBuilder(), 4);

   r.SetXYZ(prepareCube3Coord(coord.XYZ()));
   r.SetW(coord.W());

   return r;
}

static inline DflowScalars cubeProject(const DflowScalars &s, const DflowScalars &m,
                                       const DflowScalars &dsdv, const DflowScalars &dmdv)
{
   return dsdv / (2.0f * m) - s * dmdv / (m * m);
}

static inline DflowScalars lodFromGrads(const DflowScalars &iTexSize,
                                        const DflowScalars &dPdx, const DflowScalars &dPdy)
{
   DflowScalars fTexSize = itof(iTexSize);
   DflowScalars mx = dPdx * fTexSize;
   DflowScalars my = dPdy * fTexSize;

   DflowScalars maxDeriv = max(maxInVec(mx), maxInVec(my));

   return log2(maxDeriv);
}

static inline DflowScalars lodFromCubeGrads(const DflowScalars &iTexSize, const DflowScalars &P,
                                            const DflowScalars &dPdx, const DflowScalars &dPdy)
{
   assert(iTexSize.Size() == 2);
   assert(P.Size() == 3);
   assert(dPdx.Size() == 3);
   assert(dPdy.Size() == 3);

   DflowScalars dPdxProj(P.GetBuilder(), 2);
   DflowScalars dPdyProj(P.GetBuilder(), 2);

   DflowScalars xArgs0(P.GetBuilder(), 4);
   DflowScalars xArgs1(P.GetBuilder(), 4);
   DflowScalars xArgs2(P.GetBuilder(), 4);
   DflowScalars xArgs3(P.GetBuilder(), 4);

   DflowScalars yArgs0(P.GetBuilder(), 4);
   DflowScalars yArgs1(P.GetBuilder(), 4);
   DflowScalars yArgs2(P.GetBuilder(), 4);
   DflowScalars yArgs3(P.GetBuilder(), 4);

   DflowScalars args0(P.GetBuilder(), 4);
   DflowScalars args1(P.GetBuilder(), 4);
   DflowScalars args2(P.GetBuilder(), 4);
   DflowScalars args3(P.GetBuilder(), 4);

   DflowScalars fTexSize = itof(iTexSize);
   DflowScalars maxPComponent = maxInVec(P);
   DflowScalars absP = fabs(P);

   xArgs0.Set4(P.Z(), P.X(), dPdx.Z(), dPdx.X());
   xArgs1.Set4(P.Z(), P.X(), dPdy.Z(), dPdy.X());
   xArgs2.Set4(P.Y(), P.X(), dPdx.Y(), dPdx.X());
   xArgs3.Set4(P.Y(), P.X(), dPdy.Y(), dPdy.X());

   yArgs0.Set4(P.X(), P.Y(), dPdx.X(), dPdx.Y());
   yArgs1.Set4(P.X(), P.Y(), dPdy.X(), dPdy.Y());
   yArgs2.Set4(P.Z(), P.Y(), dPdx.Z(), dPdx.Y());
   yArgs3.Set4(P.Z(), P.Y(), dPdy.Z(), dPdy.Y());

   args0.Set4(P.X(), P.Z(), dPdx.X(), dPdx.Z());
   args1.Set4(P.X(), P.Z(), dPdy.X(), dPdy.Z());
   args2.Set4(P.Y(), P.Z(), dPdx.Y(), dPdx.Z());
   args3.Set4(P.Y(), P.Z(), dPdy.Y(), dPdy.Z());

   DflowScalars xCondition = (absP.X() == maxPComponent);
   DflowScalars yCondition = (absP.Y() == maxPComponent);

   args0 = cond(xCondition, xArgs0, cond(yCondition, yArgs0, args0));
   args1 = cond(xCondition, xArgs1, cond(yCondition, yArgs1, args1));
   args2 = cond(xCondition, xArgs2, cond(yCondition, yArgs2, args2));
   args3 = cond(xCondition, xArgs3, cond(yCondition, yArgs3, args3));

   dPdxProj.SetX(cubeProject(args0.X(), args0.Y(), args0.Z(), args0.W()));
   dPdyProj.SetX(cubeProject(args1.X(), args1.Y(), args1.Z(), args1.W()));
   dPdxProj.SetY(cubeProject(args2.X(), args2.Y(), args2.Z(), args2.W()));
   dPdyProj.SetY(cubeProject(args3.X(), args3.Y(), args3.Z(), args3.W()));

   dPdxProj = dPdxProj * fTexSize;
   dPdyProj = dPdyProj * fTexSize;

   DflowScalars maxDeriv = max(maxInVec(dPdxProj), maxInVec(dPdyProj));
   return log2(maxDeriv);
}

static inline DflowScalars calculateLod(const DflowScalars &texSize, const DflowScalars &texCoord,
                                        const DflowScalars &dPdx, const DflowScalars &dPdy,
                                        bool isCube)
{
   DflowScalars lod;
   if (isCube)
      lod = lodFromCubeGrads(texSize.XY(), texCoord.XYZ(), dPdx, dPdy);
   else
      lod = lodFromGrads(texSize, dPdx, dPdy);

   return lod;
}

static inline DflowScalars rem(const DflowScalars &op1, const DflowScalars &op2)
{
   return DflowScalars::BinaryOp(DATAFLOW_REM, op1, op2);
}

static inline DflowScalars modf(const DflowScalars &x)
{
   auto whole = trunc(x);

   return DflowScalars::Join(whole, x - whole);
}

static inline DflowScalars asr(const DflowScalars &bits, const DflowScalars &shift)
{
   // The backend selects the type of shift according to the type of the result.
   return DflowScalars::TypedBinaryOp(DF_INT, DATAFLOW_SHR, bits, shift);
}

static inline DflowScalars lsr(const DflowScalars &bits, const DflowScalars &shift)
{
   // The backend selects the type of shift according to the type of the result.
   return DflowScalars::TypedBinaryOp(DF_UINT, DATAFLOW_SHR, bits, shift);
}

static inline DflowScalars frexp(const DflowScalars &x)
{
   DflowScalars eq0       = x == 0.0f;
   DflowScalars x807FFFFF = constI(0x807FFFFF, x);
   DflowScalars x3F000000 = constI(0x3F000000, x);
   DflowScalars xFF       = constI(0xFF, x);
   DflowScalars bias      = constI(126, x);

   DflowScalars bits      = reinterpi(x);
   DflowScalars exp       = asr(bits, constI(23, x)) & xFF;

   DflowScalars exponent    = cond(eq0, constI(0, x), exp - bias);
   DflowScalars significand = cond(eq0, x, reinterpf((bits & x807FFFFF) | x3F000000));

   return DflowScalars::Join(significand, exponent);
}

static inline DflowScalars ldexp(const DflowScalars &sig, const DflowScalars &exp)
{
   DflowScalars ep = reinterpf(max((exp + constI(127, exp)), 0) << constI(23, exp));

   return sig * ep;
}

static inline DflowScalars low16(const DflowScalars &x)
{
   DflowScalars   xFFFF = constU(0xFFFF, x);

   return x & xFFFF;
}

static inline DflowScalars high16(const DflowScalars &x)
{
   DflowScalars   u16 = constU(16, x);

   return x >> u16;
}

static inline void uMulExtended(const DflowScalars &x, const DflowScalars &y, DflowScalars &lsb, DflowScalars &msb)
{
   DflowScalars mul0 = low16(x)  * low16(y);
   DflowScalars mul1 = low16(x)  * high16(y);
   DflowScalars mul2 = high16(x) * low16(y);
   DflowScalars mul3 = high16(x) * high16(y);
   DflowScalars tmp1 = high16(mul0) + mul1;
   DflowScalars tmp2 = low16(tmp1) + mul2;

   lsb = low16(mul0);
   msb = high16(tmp1);
   lsb = lsb + (tmp2 << constU(16, lsb));
   msb = msb + high16(tmp2);
   msb = msb + mul3;
}

static inline DflowScalars umod(const DflowScalars &x, const DflowScalars &y)
{
   return x - y * (x / y);
}

static inline DflowScalars smod(const DflowScalars &x, const DflowScalars &y)
{
   auto ax = iabs(x.Signed());
   auto ay = iabs(y.Signed());

   return ssign(y) * (ax - ay * (ax / ay));
}

static inline DflowScalars shuffle(const DflowScalars &op1, const DflowScalars &op2,
                                   const spv::vector<uint32_t> &indices)
{
   auto     result = DflowScalars(op1.GetBuilder(), indices.size());
   uint32_t size1  = op1.Size();

   uint32_t i = 0;
   for (uint32_t l : indices)
   {
      if (l != 0xffffffff)
         result[i] = l < size1 ? op1[l] : op2[l - size1];
      i++;
   }

   return result;
}

static inline DflowScalars quantizeToF16(const DflowScalars &value)
{
   // 2^-14 (min normal representable value in FP16)
   // e = 127 - 14 = 113 = 1110001 giving
   // seee eeee emmm mmmm ...
   // 0011 1000 1000 0000 ...
   //    3    8    8    0 ...
   DflowScalars minRep     = reinterpf(constU(0x38800000, value));
   DflowScalars signedZero = reinterpf(reinterpu(value) & constU(0x80000000, value));
   DflowScalars result     = funpackA(fpack(value, value));

   return cond(fabs(value) < minRep, signedZero, result);
}

static DataflowType TypeOfIntOp(const DflowScalars &a, const DflowScalars &b)
{
   // Vectors should be homogeneous
   return a[0].IsUnsigned() || b[0].IsUnsigned() ? DF_UINT : DF_INT;
}

static inline DflowScalars intEqual(const DflowScalars &a, const DflowScalars &b)
{
   DataflowType type = TypeOfIntOp(a, b);

   return a.As(type) == b.As(type);
}

static inline DflowScalars intNotEqual(const DflowScalars &a, const DflowScalars &b)
{
   DataflowType type = TypeOfIntOp(a, b);

   return a.As(type) != b.As(type);
}

static inline DflowScalars getSampleOffset(const DflowBuilder &builder, const Dflow &sampleIndex)
{
   DflowScalars offset(builder, 2);

   Dflow sampleIndexFloat = itof(sampleIndex);

   // See v3d_sample_x_offset()
#if V3D_HAS_STD_4X_RAST_PATT
   // sampleIndex=0,1,2,3 ==> offset[0]=-0.125,0.375,-0.375,0.125
   offset[0] = Dflow::ConstantFloat(-0.125f) + (sampleIndexFloat * Dflow::ConstantFloat(0.5f));
   offset[0] = Dflow::CondOp(sampleIndex >= Dflow::ConstantInt(2),
      offset[0] - Dflow::ConstantFloat(1.25f), offset[0]);
#else
   // sampleIndex=0,1,2,3 ==> offset[0]=0.125,-0.375,0.375,-0.125
   offset[0] = Dflow::ConstantFloat(1.375f) - (sampleIndexFloat * Dflow::ConstantFloat(0.5f));
   offset[0] = Dflow::CondOp(sampleIndex < Dflow::ConstantInt(2),
      offset[0] - Dflow::ConstantFloat(1.25f), offset[0]);
#endif

   // See v3d_sample_y_offset()
   offset[1] = Dflow::ConstantFloat(-0.375f) + (sampleIndexFloat * Dflow::ConstantFloat(0.25f));

   return offset;
}

static inline DflowScalars getCentroidOffset(const DflowBuilder &builder)
{
   // From get_centroid_offset() in fep.c
   auto i0 = Dflow::ConstantInt(0);
   auto i1 = Dflow::ConstantInt(1);
   auto i2 = Dflow::ConstantInt(2);
   auto i3 = Dflow::ConstantInt(3);
   auto i4 = Dflow::ConstantInt(4);
   auto i8 = Dflow::ConstantInt(8);

   // Get the sampleMask
   Dflow sm = Dflow::NullaryOp(DATAFLOW_SAMPLE_MASK);

   Dflow s0 = ((sm & i1) == i1);
   Dflow s1 = ((sm & i2) == i2);
   Dflow s2 = ((sm & i4) == i4);
   Dflow s3 = ((sm & i8) == i8);

   Dflow        sampleIndx = Dflow::CondOp(s0, i0, Dflow::CondOp(s2, i2, Dflow::CondOp(s1, i1, i3)));
   DflowScalars offset     = getSampleOffset(builder, sampleIndx);

   DflowScalars condition(builder, (sm == i0) || (s0 && s3) || (s1 && s2));
   offset = cond(condition, 0.0f, offset);

   return offset;
}

static inline DflowScalars getValueAtOffset(const DflowScalars &p, const DflowScalars &offset,
                                            InterpolationQualifier qual)
{
   if (qual == INTERP_FLAT)
      return p;

   // Adjust for offset (from centre of pixel)
   DflowScalars samplePos = DflowScalars::NullaryOp(p.GetBuilder(),
                                                   { DATAFLOW_SAMPLE_POS_X, DATAFLOW_SAMPLE_POS_Y });
   DflowScalars offsetToCentre = 0.5f - samplePos;
   DflowScalars scale = offsetToCentre + offset;
   DflowScalars res;

   if (qual == INTERP_NOPERSPECTIVE)
   {
      res = p   + (dFdx(p) * scale.X());
      res = res + (dFdy(p) * scale.Y());
   }
   else // INTERP_SMOOTH
   {
      DflowScalars w0 = DflowScalars::NullaryOp(p.GetBuilder(), { DATAFLOW_FRAG_GET_W });
      DflowScalars w(p.GetBuilder(), p.Size());
      for (uint32_t i = 0; i < p.Size(); i++)
         w[i] = w0[0];

      DflowScalars pOverW  = p / w;
      DflowScalars wInterp = w + (dFdx(w) * scale.X()) + (dFdy(w) * scale.Y());

      res = pOverW + (dFdx(pOverW) * scale.X()) + (dFdy(pOverW) * scale.Y());
      res = res * wInterp;
   }

   return res;
}

} // namespace bvk
