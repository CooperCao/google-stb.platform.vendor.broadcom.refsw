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
   return DflowScalars::UInt(anything.GetAllocator(), val);
}

static inline Dflow constU(uint32_t val, const Dflow &)
{
   return Dflow::UInt(val);
}

static inline DflowScalars constU(uint32_t val, const DflowScalars::Allocator &allocator)
{
   return DflowScalars::UInt(allocator, val);
}

static inline DflowScalars constI(int32_t val, const DflowScalars &anything)
{
   return DflowScalars::Int(anything.GetAllocator(), val);
}

static inline Dflow constI(int32_t val, const Dflow &anything)
{
   return Dflow::Int(val);
}

static inline DflowScalars constI(int32_t val, const DflowScalars::Allocator &allocator)
{
   return DflowScalars::Int(allocator, val);
}

static inline DflowScalars constF(float val, const DflowScalars &anything)
{
   return DflowScalars::Float(anything.GetAllocator(), val);
}

static inline Dflow constF(float val, const Dflow &)
{
   return Dflow::Float(val);
}

static inline DflowScalars constF(float val, const DflowScalars::Allocator &allocator)
{
   return DflowScalars::Float(allocator, val);
}

template <typename D> D fabs(const D &x)
{
   return D::UnaryOp(DATAFLOW_ABS, x);
}

template <typename D> D max(const D &x, const D &y)
{
   return D::BinaryOp(DATAFLOW_MAX, x, y);
}

template <typename D> D max(const D &x, float y)
{
   return D::BinaryOp(DATAFLOW_MAX, x, constF(y, x));
}

template <typename D> D max(float x, const D &y)
{
   return D::BinaryOp(DATAFLOW_MAX, constF(x, y), y);
}

template <typename D> D max(const D &x, int y)
{
   return D::BinaryOp(DATAFLOW_MAX, x, constI(y, x));
}

template <typename D> D max(int x, const D &y)
{
   return D::BinaryOp(DATAFLOW_MAX, constI(x, y), y);
}

template <typename D> D min(const D &x, const D &y)
{
   return D::BinaryOp(DATAFLOW_MIN, x, y);
}

template <typename D> D min(const D &x, float y)
{
   return D::BinaryOp(DATAFLOW_MIN, x, constF(y, x));
}

template <typename D> D min(float x, const D &y)
{
   return D::BinaryOp(DATAFLOW_MIN, constF(x, y), y);
}

template <typename D> D min(const D &x, int y)
{
   return D::BinaryOp(DATAFLOW_MIN, x, constI(y, x));
}

template <typename D> D min(int x, const D &y)
{
   return D::BinaryOp(DATAFLOW_MIN, constI(x, y), y);
}

template <typename D> D isnan(const D &x)
{
   return D::UnaryOp(DATAFLOW_ISNAN, x);
}

template <typename D> D reinterpi(const D &x)
{
   return x.As(DF_INT);
}

template <typename D> D reinterpu(const D &x)
{
   return x.As(DF_UINT);
}

template <typename D> D reinterpf(const D &x)
{
   return x.As(DF_FLOAT);
}

template <typename D> D recip(const D &x)
{
   return D::UnaryOp(DATAFLOW_RCP, x);
}

template <typename D> D inversesqrt(const D &x)
{
   return D::UnaryOp(DATAFLOW_RSQRT, x);
}

template <typename D> D sqrt(const D &x)
{
   return D::UnaryOp(DATAFLOW_SQRT, x);
}

template <typename D> D trunc(const D &x)
{
   return D::UnaryOp(DATAFLOW_TRUNC, x);
}

template <typename D> D round(const D &x)
{
   return D::UnaryOp(DATAFLOW_NEAREST, x);
}

template <typename D> D floor(const D &x)
{
   return D::UnaryOp(DATAFLOW_FLOOR, x);
}

template <typename D> D ceil(const D &x)
{
   return D::UnaryOp(DATAFLOW_CEIL, x);
}

template <typename D> D itof(const D &x)
{
   return D::UnaryOp(DATAFLOW_ITOF, x);
}

template <typename D> D ftoi(const D &x)
{
   return D::UnaryOp(DATAFLOW_FTOI_TRUNC, x);
}

template <typename D> D ftou(const D &x)
{
   return D::UnaryOp(DATAFLOW_FTOU, x);
}

template <typename D> D bitnot(const D &x)
{
   return D::UnaryOp(DATAFLOW_BITWISE_NOT, x);
}

template <typename D> D clz(const D &x)
{
   return D::UnaryOp(DATAFLOW_CLZ, x);
}

template <typename D> D fpack(const D&x, const D &y)
{
   return D::BinaryOp(DATAFLOW_FPACK, x, y);
}

template <typename D> D funpackA(const D &x)
{
   return D::UnaryOp(DATAFLOW_FUNPACKA, x);
}

template <typename D> D funpackB(const D &x)
{
   return D::UnaryOp(DATAFLOW_FUNPACKB, x);
}

static inline DflowScalars cond(const DflowScalars &c, const DflowScalars &iftrue, const DflowScalars &iffalse)
{
   return DflowScalars::CondOp(c, iftrue, iffalse);
}

template <typename D> D cond(const Dflow &c, const D &iftrue, const D &iffalse)
{
   return D::CondOp(c, iftrue, iffalse);
}

template <typename D> D cond(const D &c, float iftrue, const D &iffalse)
{
   return D::CondOp(c, constF(iftrue, c), iffalse);
}

template <typename D> D cond(const D &c, const D &iftrue, float iffalse)
{
   return D::CondOp(c, iftrue, constF(iffalse, c));
}

template <typename D> D cond(const D &c, float iftrue, float iffalse)
{
   return D::CondOp(c, constF(iftrue, c), constF(iffalse, c));
}

template <typename D> D findUMSB(const D &x)
{
   // If x is 0, the result is - 1
   auto i31 = constI(31, x);
   return i31 - reinterpi(clz(x));
}

template <typename D> D findSMSB(const D &x)
{
   // For positive numbers, the result will be the bit number of the most significant 1-bit.
   // For negative numbers, the result will be the bit number of the most significant 0-bit.
   // For a value of 0 or -1, the result is -1.
   auto i0 = constI(0, x);

   auto v = cond(x < i0, bitnot(x), x);
   return findUMSB(reinterpu(v));
}

template <typename D> D findLSB(const D &x)
{
   return findUMSB(reinterpu(x) & reinterpu(-x));
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

static inline DflowScalars bitCount(const DflowScalars &value)
{
   const DflowScalars u1        = constU(1, value);
   const DflowScalars u2        = constU(2, value);
   const DflowScalars u4        = constU(4, value);
   const DflowScalars u24       = constU(24, value);
   const DflowScalars x33333333 = constU(0x33333333, value);
   const DflowScalars x55555555 = constU(0x55555555, value);
   const DflowScalars x0F0F0F0F = constU(0x0F0F0F0F, value);
   const DflowScalars x01010101 = constU(0x01010101, value);

   DflowScalars res = value - ((value >> u1) & x55555555);  // 2 bit counters
   res = (res & x33333333) + ((res >> u2) & x33333333); // 4 bit counters
   res = (res + (res >> u4)) & x0F0F0F0F;               // 8 bit counters
   return lsr(res * x01010101, u24);
}

template <typename D> D isinf(const D &x)
{
   // Inf means exponent is 0xFF or 0xFF and mantissa = 0
   auto inf = reinterpf(constU(0x7F800000, x));
   return fabs(x) == inf;
}

template <typename D> D mod(const D &x, const D &y)
{
   return x - y * floor(x / y);
}

template <typename D> D iabs(const D &x)
{
   auto xs   = x.Signed();
   auto zero = constI(0, x);

   return cond(xs < zero, -xs, xs);
}

template <typename D> D nmax(const D &x, const D &y)
{
   // Semantics the same as max() on our h/w
   return max(x, y);
}

template <typename D> D nmin(const D &x, const D &y)
{
   // Semantics the same as min() on our h/w
   return min(x, y);
}

template <typename D> D clamp(const D &x, const D &minVal, const D &maxVal)
{
   return min(max(x, minVal), maxVal);
}

template <typename D> D clamp(const D &x, float minVal, const D &maxVal)
{
   return min(max(x, minVal), maxVal);
}

template <typename D> D clamp(const D &x, const D &minVal, float maxVal)
{
   return min(max(x, minVal), maxVal);
}

template <typename D> D clamp(const D &x, float minVal, float maxVal)
{
   return min(max(x, minVal), maxVal);
}

template <typename D> D nclamp(const D &x, const D &minVal, const D &maxVal)
{
   return nmin(nmax(x, minVal), maxVal);
}

template <typename D> D fmix(const D &x, const D &y, const D &a)
{
   return x + (y - x) * a;
}

template <typename D> D imix(const D &x, const D &y, const D &a)
{
   return x + (y - x) * a;
}

template <typename D> D step(const D &edge, const D &x)
{
   return cond(x < edge, 0.0f, 1.0f);
}

template <typename D> D smoothstep(const D &edge0, const D &edge1, const D &x)
{
   auto t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
   return t * t * (3.0f - 2.0f * t);
}

template <typename D> D log2(const D &x)
{
   return D::UnaryOp(DATAFLOW_LOG2, x);
}

template <typename D> D log(const D &x)
{
   auto x3F317218 = constU(0x3F317218, x);
   auto loge2     = reinterpf(x3F317218);

   return log2(x) * loge2;
}

template <typename D> D exp2(const D &x)
{
   return D::UnaryOp(DATAFLOW_EXP2, x);
}

template <typename D> D exp(const D &x)
{
   auto x3FB8AA3B = constU(0X3FB8AA3B, x);
   auto log2e     = reinterpf(x3FB8AA3B);

   return exp2(x * log2e);
}

template <typename D> D pow(const D &x, const D &y)
{
   return exp2(y * log2(x));
}

template <typename D> D sin(const D &x)
{
   return D::UnaryOp(DATAFLOW_SIN, x);
}

template <typename D> D cos(const D &x)
{
   return D::UnaryOp(DATAFLOW_COS, x);
}

template <typename D> D tan(const D &x)
{
   return D::UnaryOp(DATAFLOW_TAN, x);
}

template <typename D> D sinh(const D &x)
{
   return 0.5f * (exp(x) - exp(-x));
}

template <typename D> D cosh(const D &x)
{
   return 0.5f * (exp(x) + exp(-x));
}

template <typename D> D tanh(const D &x)
{
   return sinh(x) / cosh(x);
}

template <typename D> D asinh(const D &x)
{
   return log(x + sqrt(x * x + 1.0f));
}

template <typename D> D acosh(const D &x)
{
   return log(x + sqrt((x - 1.0f) * (x + 1.0f)));
}

template <typename D> D atanh(const D &x)
{
   return 0.5f * log((1.0f + x) / (1.0f - x));
}

template <typename D> D atan(const D &yOverX)
{
   const float T3PO8 = 2.414213562373f;
   const float TPO8  = 0.414213562373f;
   const float PO2   = 1.570796326794f;
   const float PO4   = 0.785398163397f;

   auto x80000000 = constU(0x80000000, yOverX);
   auto c         = constF(0.0f, yOverX);

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

template <typename D> D atan(const D &y, const D &x)
{
   auto PI        = constF(3.1415926535897932384626433832795f, x);
   auto c31       = constU(31, x);
   auto one       = constF(1.0f, x);
   auto x80000000 = constU(0x80000000, x);

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

template <typename D> D asin(const D &x)
{
   auto x80000000 = constU(0x80000000u, x);

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

template <typename D> D acos(const D &x)
{
   return 2.0f * atan(inversesqrt((1.0f + x) / (1.0f - x)));
}

template <typename D> D degrees(const D &radians)
{
   return 57.29577951f * radians;
}

template <typename D> D radians(const D&degree)
{
   return 0.01745329252f * degree;
}

template <typename D> D fsign(const D &x)
{
   auto zeroOrOne = cond(x == 0.0f, 0.0f, 1.0f);
   return cond(x < 0.0f, -zeroOrOne, zeroOrOne);
}

template <typename D> D ssign(const D &x)
{
   auto i0 = constI(0, x);
   auto i1 = constI(1, x);

   auto zeroOrOne = cond(x == i0, i0, i1);
   return cond(x < i0, -zeroOrOne, zeroOrOne);
}

template <typename D> D fract(const D &x)
{
   return x - floor(x);
}

template <typename D> D fma(const D &a, const D &b, const D &c)
{
   return a * b + c;
}

static inline DflowScalars dot(const DflowScalars &v1, const DflowScalars &v2)
{
   assert(v1.Size() == v2.Size());

   Dflow result = (v1 * v2).Fold([](const Dflow &l, const Dflow &r) { return l + r; });

   return DflowScalars(v1.GetAllocator(), result);
}

static inline DflowScalars cross(const DflowScalars &x, const DflowScalars &y)
{
   return DflowScalars(x.GetAllocator(),
                       { x[1] * y[2] - x[2] * y[1],
                         x[2] * y[0] - x[0] * y[2],
                         x[0] * y[1] - x[1] * y[0] });
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
   return DflowScalars(v.GetAllocator(), 1, &res);
}

static inline DflowScalars unpackSnorm2x16(const DflowScalars &p)
{
   assert(p.Size() == 1);
   DflowScalars res(p.GetAllocator(), 2);
   dflib_unpackSnorm2x16(res.Data(), p[0]);
   return res;
}

static inline DflowScalars packUnorm2x16(const DflowScalars &v)
{
   assert(v.Size() == 2);
   Dataflow *res = dflib_packUnorm2x16(v[0], v[1]);
   return DflowScalars(v.GetAllocator(), 1, &res);
}

static inline DflowScalars unpackUnorm2x16(const DflowScalars &p)
{
   assert(p.Size() == 1);
   DflowScalars res(p.GetAllocator(), 2);
   dflib_unpackUnorm2x16(res.Data(), p[0]);
   return res;
}

static inline DflowScalars packHalf2x16(const DflowScalars &v)
{
   assert(v.Size() == 2);
   Dataflow *res = dflib_packHalf2x16(v[0], v[1]);
   return DflowScalars(v.GetAllocator(), 1, &res);
}

static inline DflowScalars unpackHalf2x16(const DflowScalars &p)
{
   assert(p.Size() == 1);
   DflowScalars res(p.GetAllocator(), 2);
   dflib_unpackHalf2x16(res.Data(), p[0]);
   return res;
}

static inline DflowScalars packUnorm4x8(const DflowScalars &v)
{
   assert(v.Size() == 4);
   Dataflow *res = dflib_packUnorm4x8(v[0], v[1], v[2], v[3]);
   return DflowScalars(v.GetAllocator(), 1, &res);
}

static inline DflowScalars packSnorm4x8(const DflowScalars &v)
{
   assert(v.Size() == 4);
   Dataflow *res = dflib_packSnorm4x8(v[0], v[1], v[2], v[3]);
   return DflowScalars(v.GetAllocator(), 1, &res);
}

static inline DflowScalars unpackUnorm4x8(const DflowScalars &p)
{
   assert(p.Size() == 1);
   DflowScalars res(p.GetAllocator(), 4);
   dflib_unpackUnorm4x8(res.Data(), p[0]);
   return res;
}

static inline DflowScalars unpackSnorm4x8(const DflowScalars &p)
{
   assert(p.Size() == 1);
   DflowScalars res(p.GetAllocator(), 4);
   dflib_unpackSnorm4x8(res.Data(), p[0]);
   return res;
}

static inline Dflow maxInVec(const DflowScalars &v)
{
   Dflow res = fabs(v[0]);

   for (uint32_t i = 1; i < v.Size(); ++i)
      res = max(res, fabs(v[i]));

   return res;
}

static inline Dflow maxInVec3(const DflowScalars &v)
{
   return max(fabs(v.x()), max(fabs(v.y()), fabs(v.z())));
}

static inline DflowScalars prepareCube3Coord(const DflowScalars &coord)
{
   return coord / DflowScalars(coord.GetAllocator(), coord.Size(), maxInVec3(coord));
}

// Like the vec3 version but pass the 3-component through. Used for shadow and array
static inline DflowScalars prepareCube4Coord(const DflowScalars &coord)
{
   DflowScalars xyz = prepareCube3Coord(coord.XYZ());

   return DflowScalars(coord.GetAllocator(),
                       { xyz.x(), xyz.y(), xyz.z(), coord.w() });
}

static inline Dflow cubeProject(const DflowScalars &scalars)
{
   const Dflow &s    = scalars.x();
   const Dflow &m    = scalars.y();
   const Dflow &dsdv = scalars.z();
   const Dflow &dmdv = scalars.w();

   return dsdv / (2.0f * m) - s * dmdv / (m * m);
}

static inline Dflow lodFromGrads(const DflowScalars &iTexSize,
                                 const DflowScalars &dPdx, const DflowScalars &dPdy)
{
   DflowScalars fTexSize = itof(iTexSize);
   DflowScalars mx = dPdx * fTexSize;
   DflowScalars my = dPdy * fTexSize;

   Dflow maxDeriv = max(maxInVec(mx), maxInVec(my));

   return log2(maxDeriv);
}

static inline Dflow lodFromCubeGrads(const DflowScalars &iTexSize, const DflowScalars &P,
                                     const DflowScalars &dPdx, const DflowScalars &dPdy)
{
   assert(iTexSize.Size() == 2);
   assert(P.Size() == 3);
   assert(dPdx.Size() == 3);
   assert(dPdy.Size() == 3);

   const DflowScalars::Allocator &alloc = P.GetAllocator();

   DflowScalars fTexSize      = itof(iTexSize);
   Dflow        maxPComponent = maxInVec(P);
   DflowScalars absP          = fabs(P);

   DflowScalars xArgs0(alloc, { P.z(), P.x(), dPdx.z(), dPdx.x() } );
   DflowScalars xArgs1(alloc, { P.z(), P.x(), dPdy.z(), dPdy.x() } );
   DflowScalars xArgs2(alloc, { P.y(), P.x(), dPdx.y(), dPdx.x() } );
   DflowScalars xArgs3(alloc, { P.y(), P.x(), dPdy.y(), dPdy.x() } );

   DflowScalars yArgs0(alloc, { P.x(), P.y(), dPdx.x(), dPdx.y() } );
   DflowScalars yArgs1(alloc, { P.x(), P.y(), dPdy.x(), dPdy.y() } );
   DflowScalars yArgs2(alloc, { P.z(), P.y(), dPdx.z(), dPdx.y() } );
   DflowScalars yArgs3(alloc, { P.z(), P.y(), dPdy.z(), dPdy.y() } );

   DflowScalars args0(alloc, { P.x(), P.z(), dPdx.x(), dPdx.z() } );
   DflowScalars args1(alloc, { P.x(), P.z(), dPdy.x(), dPdy.z() } );
   DflowScalars args2(alloc, { P.y(), P.z(), dPdx.y(), dPdx.z() } );
   DflowScalars args3(alloc, { P.y(), P.z(), dPdy.y(), dPdy.z() } );

   Dflow xCondition = (absP.x() == maxPComponent);
   Dflow yCondition = (absP.y() == maxPComponent);

   args0 = cond(xCondition, xArgs0, cond(yCondition, yArgs0, args0));
   args1 = cond(xCondition, xArgs1, cond(yCondition, yArgs1, args1));
   args2 = cond(xCondition, xArgs2, cond(yCondition, yArgs2, args2));
   args3 = cond(xCondition, xArgs3, cond(yCondition, yArgs3, args3));

   DflowScalars dPdxProj(alloc, {cubeProject(args0), cubeProject(args2)} );
   DflowScalars dPdyProj(alloc, {cubeProject(args1), cubeProject(args3)} );

   dPdxProj = dPdxProj * fTexSize;
   dPdyProj = dPdyProj * fTexSize;

   Dflow maxDeriv = max(maxInVec(dPdxProj), maxInVec(dPdyProj));

   return log2(maxDeriv);
}

static inline Dflow calculateLod(const DflowScalars &texSize, const DflowScalars &texCoord,
                                 const DflowScalars &dPdx, const DflowScalars &dPdy,
                                 bool isCube)
{
   if (isCube)
      return lodFromCubeGrads(texSize.XY(), texCoord.XYZ(), dPdx, dPdy);
   else
      return lodFromGrads(texSize, dPdx, dPdy);
}

template <typename D> D rem(const D &op1, const D &op2)
{
   return D::BinaryOp(DATAFLOW_REM, op1, op2);
}

static inline DflowScalars modf(const DflowScalars &x)
{
   auto whole = trunc(x);

   return DflowScalars::Join(whole, x - whole);
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

template <typename D> D ldexp(const D &sig, const D &exp)
{
   D ep = reinterpf(max((exp + constI(127, exp)), 0) << constI(23, exp));

   return sig * ep;
}

template <typename D> D low16(const D &x)
{
   return x & constU(0xFFFF, x);
}

template <typename D> D high16(const D &x)
{
   return x >> constU(16, x);
}

template <typename D> void uMulExtended(const D &x, const D &y, D &lsb, D &msb)
{
   D mul0 = low16(x)  * low16(y);
   D mul1 = low16(x)  * high16(y);
   D mul2 = high16(x) * low16(y);
   D mul3 = high16(x) * high16(y);
   D tmp1 = high16(mul0) + mul1;
   D tmp2 = low16(tmp1) + mul2;

   lsb = low16(mul0);
   msb = high16(tmp1);
   lsb = lsb + (tmp2 << constU(16, lsb));
   msb = msb + high16(tmp2);
   msb = msb + mul3;
}

template <typename D> D umod(const D &x, const D &y)
{
   return x - y * (x / y);
}

template <typename D> D smod(const D &x, const D &y)
{
   D ax = iabs(x.Signed());
   D ay = iabs(y.Signed());

   return ssign(y) * (ax - ay * (ax / ay));
}

static inline DflowScalars shuffle(const DflowScalars &op1, const DflowScalars &op2,
                                   const spv::vector<uint32_t> &indices)
{
   auto     result = DflowScalars(op1.GetAllocator(), indices.size());
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

template <typename D> D quantizeToF16(const D &value)
{
   // 2^-14 (min normal representable value in FP16)
   // e = 127 - 14 = 113 = 1110001 giving
   // seee eeee emmm mmmm ...
   // 0011 1000 1000 0000 ...
   //    3    8    8    0 ...
   D minRep     = reinterpf(constU(0x38800000, value));
   D signedZero = reinterpf(reinterpu(value) & constU(0x80000000, value));
   D result     = funpackA(fpack(value, value));

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

static inline DflowScalars getSampleOffset(const DflowScalars::Allocator &allocator, const Dflow &sampleIndex)
{
   DflowScalars offset(allocator, 2);

   Dflow sampleIndexFloat = itof(sampleIndex);

   // See v3d_sample_x_offset()
#if V3D_VER_AT_LEAST(4,2,14,0)
   // sampleIndex=0,1,2,3 ==> offset[0]=-0.125,0.375,-0.375,0.125
   offset[0] = Dflow::Float(-0.125f) + (sampleIndexFloat * Dflow::Float(0.5f));
   offset[0] = Dflow::CondOp(sampleIndex >= Dflow::Int(2),
      offset[0] - Dflow::Float(1.25f), offset[0]);
#else
   // sampleIndex=0,1,2,3 ==> offset[0]=0.125,-0.375,0.375,-0.125
   offset[0] = 1.375f - (sampleIndexFloat * 0.5f);
   offset[0] = Dflow::CondOp(sampleIndex < Dflow::Int(2),
      offset[0] - Dflow::Float(1.25f), offset[0]);
#endif

   // See v3d_sample_y_offset()
   offset[1] = -0.375f + (sampleIndexFloat * 0.25f);

   return offset;
}

static inline DflowScalars getCentroidOffset(const DflowScalars::Allocator &allocator)
{
   // From get_centroid_offset() in fep.c
   auto i0 = Dflow::Int(0);
   auto i1 = Dflow::Int(1);
   auto i2 = Dflow::Int(2);
   auto i3 = Dflow::Int(3);
   auto i4 = Dflow::Int(4);
   auto i8 = Dflow::Int(8);

   // Get the sampleMask
   Dflow sm = Dflow::NullaryOp(DATAFLOW_SAMPLE_MASK);

   Dflow s0 = ((sm & i1) == i1);
   Dflow s1 = ((sm & i2) == i2);
   Dflow s2 = ((sm & i4) == i4);
   Dflow s3 = ((sm & i8) == i8);

   Dflow        sampleIndx = Dflow::CondOp(s0, i0, Dflow::CondOp(s2, i2, Dflow::CondOp(s1, i1, i3)));
   DflowScalars offset     = getSampleOffset(allocator, sampleIndx);

   DflowScalars condition(allocator, (sm == i0) || (s0 && s3) || (s1 && s2));
   offset = cond(condition, 0.0f, offset);

   return offset;
}

static inline DflowScalars getValueAtOffset(const DflowScalars &p, const DflowScalars &offset,
                                            InterpolationQualifier qual)
{
   if (qual == INTERP_FLAT)
      return p;

   // Adjust for offset (from centre of pixel)
   DflowScalars samplePos = DflowScalars::NullaryOp(p.GetAllocator(),
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
      Dflow        w0 = Dflow::NullaryOp(DATAFLOW_FRAG_GET_W);
      DflowScalars w(p.GetAllocator(), p.Size());
      for (uint32_t i = 0; i < p.Size(); i++)
         w[i] = w0;

      DflowScalars pOverW  = p / w;
      DflowScalars wInterp = w + (dFdx(w) * scale.X()) + (dFdy(w) * scale.Y());

      res = pOverW + (dFdx(pOverW) * scale.X()) + (dFdy(pOverW) * scale.Y());
      res = res * wInterp;
   }

   return res;
}

} // namespace bvk
