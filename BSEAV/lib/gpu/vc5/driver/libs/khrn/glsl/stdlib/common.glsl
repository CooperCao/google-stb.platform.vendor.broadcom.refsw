//
// Common Functions
//


float abs(float x)
{
   return $$fabs(x);
}


vec2 abs(vec2 x)
{
   return $$fabs(x);
}


vec3 abs(vec3 x)
{
   return $$fabs(x);
}


vec4 abs(vec4 x)
{
   return $$fabs(x);
}


int abs(int x)
{
   if(x < 0) return -x;
   else return x;
}


ivec2 abs(ivec2 x)
{
   return ivec2(abs(x[0]), abs(x[1]));
}


ivec3 abs(ivec3 x)
{
   return ivec3(abs(x[0]), abs(x[1]), abs(x[2]));
}


ivec4 abs(ivec4 x)
{
   return ivec4(abs(x[0]), abs(x[1]), abs(x[2]), abs(x[3]));
}


float sign(float x)
{
   uint retval = $$reinterpu(1.0);
   uint y      = $$reinterpu(x);
   uint y_sign = y & 0x80000000u;
   retval |= y_sign;
   retval  = x != 0.0 ? retval : 0u;
   return $$reinterpf(retval);
}


vec2 sign(vec2 x)
{
   return vec2(sign(x[0]), sign(x[1]));
}


vec3 sign(vec3 x)
{
   return vec3(sign(x[0]), sign(x[1]), sign(x[2]));
}


vec4 sign(vec4 x)
{
   return vec4(sign(x[0]), sign(x[1]), sign(x[2]), sign(x[3]));
}


int sign(int x)
{
   if(x > 0) return 1;
   else if(x < 0) return -1;
   else return 0;
}


ivec2 sign(ivec2 x)
{
   return ivec2(sign(x[0]), sign(x[1]));
}


ivec3 sign(ivec3 x)
{
   return ivec3(sign(x[0]), sign(x[1]), sign(x[2]));
}


ivec4 sign(ivec4 x)
{
   return ivec4(sign(x[0]), sign(x[1]), sign(x[2]), sign(x[3]));
}



float floor(float x)
{
   return $$floor(x);
}


vec2 floor(vec2 x)
{
   return $$floor(x);
}


vec3 floor(vec3 x)
{
   return $$floor(x);
}


vec4 floor(vec4 x)
{
   return $$floor(x);
}


float ceil(float x)
{
   return $$ceil(x);
}


vec2 ceil(vec2 x)
{
   return $$ceil(x);
}


vec3 ceil(vec3 x)
{
   return $$ceil(x);
}


vec4 ceil(vec4 x)
{
   return $$ceil(x);
}


float fract(float x)
{
   return x - $$floor(x);
}


vec2 fract(vec2 x)
{
   return vec2(fract(x[0]), fract(x[1]));
}


vec3 fract(vec3 x)
{
   return vec3(fract(x[0]), fract(x[1]), fract(x[2]));
}


vec4 fract(vec4 x)
{
   return vec4(fract(x[0]), fract(x[1]), fract(x[2]), fract(x[3]));
}


float mod(float x, float y)
{
   return x - y * floor(x / y);
}


vec2 mod(vec2 x, float y)
{
   return vec2(mod(x[0], y), mod(x[1], y));
}


vec3 mod(vec3 x, float y)
{
   return vec3(mod(x[0], y), mod(x[1], y), mod(x[2], y));
}


vec4 mod(vec4 x, float y)
{
   return vec4(mod(x[0], y), mod(x[1], y), mod(x[2], y), mod(x[3], y));
}


vec2 mod(vec2 x, vec2 y)
{
   return vec2(mod(x[0], y[0]), mod(x[1], y[1]));
}


vec3 mod(vec3 x, vec3 y)
{
   return vec3(mod(x[0], y[0]), mod(x[1], y[1]), mod(x[2], y[2]));
}


vec4 mod(vec4 x, vec4 y)
{
   return vec4(mod(x[0], y[0]), mod(x[1], y[1]), mod(x[2], y[2]), mod(x[3], y[3]));
}


float min(float x, float y)
{
   return $$min(x, y);
}


vec2 min(vec2 x, vec2 y)
{
   return $$min(x,y);
}


vec3 min(vec3 x, vec3 y)
{
   return $$min(x,y);
}


vec4 min(vec4 x, vec4 y)
{
   return $$min(x,y);
}


vec2 min(vec2 x, float y)
{
   return $$min(x,y);
}


vec3 min(vec3 x, float y)
{
   return $$min(x,y);
}


vec4 min(vec4 x, float y)
{
   return $$min(x,y);
}


int min(int x, int y)
{
   return $$min(x, y);
}


ivec2 min(ivec2 x, ivec2 y)
{
   return $$min(x,y);
}


ivec3 min(ivec3 x, ivec3 y)
{
   return $$min(x,y);
}


ivec4 min(ivec4 x, ivec4 y)
{
   return $$min(x,y);
}


ivec2 min(ivec2 x, int y)
{
   return $$min(x,y);
}


ivec3 min(ivec3 x, int y)
{
   return $$min(x,y);
}


ivec4 min(ivec4 x, int y)
{
   return $$min(x,y);
}


uint min(uint x, uint y)
{
   return $$min(x,y);
}


uvec2 min(uvec2 x, uvec2 y)
{
   return $$min(x,y);
}


uvec3 min(uvec3 x, uvec3 y)
{
   return $$min(x,y);
}


uvec4 min(uvec4 x, uvec4 y)
{
   return $$min(x,y);
}


uvec2 min(uvec2 x, uint y)
{
   return $$min(x,y);
}


uvec3 min(uvec3 x, uint y)
{
   return $$min(x,y);
}


uvec4 min(uvec4 x, uint y)
{
   return $$min(x,y);
}


float max(float x, float y)
{
   return $$max(x, y);
}


vec2 max(vec2 x, vec2 y)
{
   return $$max(x,y);
}


vec3 max(vec3 x, vec3 y)
{
   return $$max(x,y);
}


vec4 max(vec4 x, vec4 y)
{
   return $$max(x,y);
}


vec2 max(vec2 x, float y)
{
   return $$max(x,y);
}


vec3 max(vec3 x, float y)
{
   return $$max(x,y);
}


vec4 max(vec4 x, float y)
{
   return $$max(x,y);
}


int max(int x, int y)
{
   return $$max(x, y);
}


ivec2 max(ivec2 x, ivec2 y)
{
   return $$max(x,y);
}


ivec3 max(ivec3 x, ivec3 y)
{
   return $$max(x,y);
}


ivec4 max(ivec4 x, ivec4 y)
{
   return $$max(x,y);
}


ivec2 max(ivec2 x, int y)
{
   return $$max(x,y);
}


ivec3 max(ivec3 x, int y)
{
   return $$max(x,y);
}


ivec4 max(ivec4 x, int y)
{
   return $$max(x,y);
}


uint max(uint x, uint y)
{
   return $$max(x, y);
}


uvec2 max(uvec2 x, uvec2 y)
{
   return $$max(x,y);
}


uvec3 max(uvec3 x, uvec3 y)
{
   return $$max(x,y);
}


uvec4 max(uvec4 x, uvec4 y)
{
   return $$max(x,y);
}


uvec2 max(uvec2 x, uint y)
{
   return $$max(x,y);
}


uvec3 max(uvec3 x, uint y)
{
   return $$max(x,y);
}


uvec4 max(uvec4 x, uint y)
{
   return $$max(x,y);
}


float clamp(float x, float minVal, float maxVal)
{
   return $$min($$max(x, minVal), maxVal);
}


vec2 clamp(vec2 x, vec2 minVal, vec2 maxVal)
{
   return $$min($$max(x, minVal), maxVal);
}


vec3 clamp(vec3 x, vec3 minVal, vec3 maxVal)
{
   return $$min($$max(x, minVal), maxVal);
}


vec4 clamp(vec4 x, vec4 minVal, vec4 maxVal)
{
   return $$min($$max(x, minVal), maxVal);
}


vec2 clamp(vec2 x, float minVal, float maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


vec3 clamp(vec3 x, float minVal, float maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


vec4 clamp(vec4 x, float minVal, float maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}

int clamp(int x, int minVal, int maxVal)
{
   return $$min($$max(x, minVal), maxVal);
}


ivec2 clamp(ivec2 x, ivec2 minVal, ivec2 maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


ivec3 clamp(ivec3 x, ivec3 minVal, ivec3 maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


ivec4 clamp(ivec4 x, ivec4 minVal, ivec4 maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


ivec2 clamp(ivec2 x, int minVal, int maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


ivec3 clamp(ivec3 x, int minVal, int maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


ivec4 clamp(ivec4 x, int minVal, int maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


uint clamp(uint x, uint minVal, uint maxVal)
{
   return $$min($$max(x, minVal), maxVal);
}


uvec2 clamp(uvec2 x, uvec2 minVal, uvec2 maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


uvec3 clamp(uvec3 x, uvec3 minVal, uvec3 maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


uvec4 clamp(uvec4 x, uvec4 minVal, uvec4 maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


uvec2 clamp(uvec2 x, uint minVal, uint maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


uvec3 clamp(uvec3 x, uint minVal, uint maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


uvec4 clamp(uvec4 x, uint minVal, uint maxVal)
{
   return $$min($$max(x,minVal), maxVal);
}


float mix(float x, float y, float a)
{
   return x + (y - x) * a;
}


vec2 mix(vec2 x, vec2 y, vec2 a)
{
   return vec2(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]));
}


vec3 mix(vec3 x, vec3 y, vec3 a)
{
   return vec3(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]), mix(x[2], y[2], a[2]));
}


vec4 mix(vec4 x, vec4 y, vec4 a)
{
   return vec4(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]), mix(x[2], y[2], a[2]), mix(x[3], y[3], a[3]));
}


vec2 mix(vec2 x, vec2 y, float a)
{
   return vec2(mix(x[0], y[0], a), mix(x[1], y[1], a));
}


vec3 mix(vec3 x, vec3 y, float a)
{
   return vec3(mix(x[0], y[0], a), mix(x[1], y[1], a), mix(x[2], y[2], a));
}


vec4 mix(vec4 x, vec4 y, float a)
{
   return vec4(mix(x[0], y[0], a), mix(x[1], y[1], a), mix(x[2], y[2], a), mix(x[3], y[3], a));
}

float mix(float x, float y, bool a)
{
   if (a)
      return y;
   else
      return x;
}

vec2 mix(vec2 x, vec2 y, bvec2 a)
{
   return vec2(mix(x[0],y[0],a[0]), mix(x[1],y[1],a[1]));
}

vec3 mix(vec3 x, vec3 y, bvec3 a)
{
   return vec3(mix(x[0],y[0],a[0]), mix(x[1],y[1],a[1]), mix(x[2],y[2],a[2]));
}

vec4 mix(vec4 x, vec4 y, bvec4 a)
{
   return vec4(mix(x[0],y[0],a[0]), mix(x[1],y[1],a[1]), mix(x[2],y[2],a[2]), mix(x[3],y[3],a[3]));
}

int mix(int x, int y, bool a) {
   if (a) return y;
   else return x;
}

ivec2 mix(ivec2 x, ivec2 y, bvec2 a) {
   return ivec2(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]));
}

ivec3 mix(ivec3 x, ivec3 y, bvec3 a) {
   return ivec3(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]), mix(x[2], y[2], a[2]));
}

ivec4 mix(ivec4 x, ivec4 y, bvec4 a) {
   return ivec4(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]), mix(x[2], y[2], a[2]), mix(x[3], y[3], a[3]));
}

uint mix(uint x, uint y, bool a) {
   if (a) return y;
   else return x;
}

uvec2 mix(uvec2 x, uvec2 y, bvec2 a) {
   return uvec2(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]));
}

uvec3 mix(uvec3 x, uvec3 y, bvec3 a) {
   return uvec3(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]), mix(x[2], y[2], a[2]));
}

uvec4 mix(uvec4 x, uvec4 y, bvec4 a) {
   return uvec4(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]), mix(x[2], y[2], a[2]), mix(x[3], y[3], a[3]));
}

bool mix(bool x, bool y, bool a) {
   if (a) return y;
   else return x;
}

bvec2 mix(bvec2 x, bvec2 y, bvec2 a) {
   return bvec2(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]));
}

bvec3 mix(bvec3 x, bvec3 y, bvec3 a) {
   return bvec3(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]), mix(x[2], y[2], a[2]));
}

bvec4 mix(bvec4 x, bvec4 y, bvec4 a) {
   return bvec4(mix(x[0], y[0], a[0]), mix(x[1], y[1], a[1]), mix(x[2], y[2], a[2]), mix(x[3], y[3], a[3]));
}


float step(float edge, float x)
{
   if (x < edge)
      return 0.0;
   else
      return 1.0;
}


vec2 step(vec2 edge, vec2 x)
{
   return vec2(
      step(edge[0], x[0]),
      step(edge[1], x[1]));
}


vec3 step(vec3 edge, vec3 x)
{
   return vec3(
      step(edge[0], x[0]),
      step(edge[1], x[1]),
      step(edge[2], x[2]));
}


vec4 step(vec4 edge, vec4 x)
{
   return vec4(
      step(edge[0], x[0]),
      step(edge[1], x[1]),
      step(edge[2], x[2]),
      step(edge[3], x[3]));
}


vec2 step(float edge, vec2 x)
{
   return vec2(
      step(edge, x[0]),
      step(edge, x[1]));
}


vec3 step(float edge, vec3 x)
{
   return vec3(
      step(edge, x[0]),
      step(edge, x[1]),
      step(edge, x[2]));
}


vec4 step(float edge, vec4 x)
{
   return vec4(
      step(edge, x[0]),
      step(edge, x[1]),
      step(edge, x[2]),
      step(edge, x[3]));
}


float smoothstep(float edge0, float edge1, float x)
{
   highp float t;
   t = clamp ((x - edge0) / (edge1 - edge0), 0.0, 1.0);
   return t * t * (3.0 - 2.0 * t);
}


vec2 smoothstep(vec2 edge0, vec2 edge1, vec2 x)
{
   return vec2(
      smoothstep(edge0[0], edge1[0], x[0]),
      smoothstep(edge0[1], edge1[1], x[1]));
}


vec3 smoothstep(vec3 edge0, vec3 edge1, vec3 x)
{
   return vec3(
      smoothstep(edge0[0], edge1[0], x[0]),
      smoothstep(edge0[1], edge1[1], x[1]),
      smoothstep(edge0[2], edge1[2], x[2]));
}


vec4 smoothstep(vec4 edge0, vec4 edge1, vec4 x)
{
   return vec4(
      smoothstep(edge0[0], edge1[0], x[0]),
      smoothstep(edge0[1], edge1[1], x[1]),
      smoothstep(edge0[2], edge1[2], x[2]),
      smoothstep(edge0[3], edge1[3], x[3]));
}


vec2 smoothstep(float edge0, float edge1, vec2 x)
{
   return vec2(
      smoothstep(edge0, edge1, x[0]),
      smoothstep(edge0, edge1, x[1]));
}


vec3 smoothstep(float edge0, float edge1, vec3 x)
{
   return vec3(
      smoothstep(edge0, edge1, x[0]),
      smoothstep(edge0, edge1, x[1]),
      smoothstep(edge0, edge1, x[2]));
}


vec4 smoothstep(float edge0, float edge1, vec4 x)
{
   return vec4(
      smoothstep(edge0, edge1, x[0]),
      smoothstep(edge0, edge1, x[1]),
      smoothstep(edge0, edge1, x[2]),
      smoothstep(edge0, edge1, x[3]));
}


float trunc(float x)
{
  return $$trunc(x);
}


vec2 trunc(vec2 x)
{
   return $$trunc(x);
}


vec3 trunc(vec3 x)
{
   return $$trunc(x);
}


vec4 trunc(vec4 x)
{
   return $$trunc(x);
}


float round(float x)
{
   return $$nearest(x);
}


vec2 round(vec2 x)
{
   return $$nearest(x);
}


vec3 round(vec3 x)
{
   return $$nearest(x);
}


vec4 round(vec4 x)
{
   return $$nearest(x);
}


float roundEven(float x)
{
  return $$nearest(x);
}


vec2 roundEven(vec2 x)
{
   return $$nearest(x);
}


vec3 roundEven(vec3 x)
{
   return $$nearest(x);
}


vec4 roundEven(vec4 x)
{
   return $$nearest(x);
}


float modf(float x, out float i)
{
   i = trunc(x);
   return x - i;
}


vec2 modf(vec2 x, out vec2 i)
{
  return vec2(modf(x[0], i[0]), modf(x[1], i[1]));
}


vec3 modf(vec3 x, out vec3 i)
{
  return vec3(modf(x[0], i[0]), modf(x[1], i[1]), modf(x[2], i[2]));
}


vec4 modf(vec4 x, out vec4 i)
{
  return vec4(modf(x[0], i[0]), modf(x[1], i[1]), modf(x[2], i[2]), modf(x[3], i[3]));
}


bool __isinf(float x)
{
   highp float inf = $$reinterpf(0x7f800000);
   return $$fabs(x) == inf;
}

bool isinf(float x)
{
   return __isinf(x);
}

bvec2 isinf(vec2 x)
{
  return bvec2(isinf(x[0]), isinf(x[1]));
}


bvec3 isinf(vec3 x)
{
  return bvec3(isinf(x[0]), isinf(x[1]), isinf(x[2]));
}


bvec4 isinf(vec4 x)
{
  return bvec4(isinf(x[0]), isinf(x[1]), isinf(x[2]), isinf(x[3]));
}


bool isnan(float x)
{
   int xi = $$reinterpi(x);
   int exponent = xi & (0xff << 23);
   int mantissa = xi & ((1<<23) - 1);
   return (exponent == (0xff << 23) && mantissa != 0);
}


bvec2 isnan(vec2 x)
{
  return bvec2(isnan(x[0]), isnan(x[1]));
}


bvec3 isnan(vec3 x)
{
  return bvec3(isnan(x[0]), isnan(x[1]), isnan(x[2]));
}


bvec4 isnan(vec4 x)
{
  return bvec4(isnan(x[0]), isnan(x[1]), isnan(x[2]), isnan(x[3]));
}


int floatBitsToInt(float value)
{
  return $$reinterpi(value);
}


ivec2 floatBitsToInt(vec2 value)
{
  return $$reinterpi(value);
}


ivec3 floatBitsToInt(vec3 value)
{
  return $$reinterpi(value);
}


ivec4 floatBitsToInt(vec4 value)
{
  return $$reinterpi(value);
}


float intBitsToFloat(int value)
{
  return $$reinterpf(value);
}


vec2 intBitsToFloat(ivec2 value)
{
  return $$reinterpf(value);
}


vec3 intBitsToFloat(ivec3 value)
{
  return $$reinterpf(value);
}


vec4 intBitsToFloat(ivec4 value)
{
  return $$reinterpf(value);
}


uint floatBitsToUint(float value)
{
   return $$reinterpu(value);
}


uvec2 floatBitsToUint(vec2 value)
{
   return $$reinterpu(value);
}


uvec3 floatBitsToUint(vec3 value)
{
   return $$reinterpu(value);
}


uvec4 floatBitsToUint(vec4 value)
{
   return $$reinterpu(value);
}


float uintBitsToFloat(uint value)
{
   return $$reinterpf(value);
}


vec2 uintBitsToFloat(uvec2 value)
{
   return $$reinterpf(value);
}


vec3 uintBitsToFloat(uvec3 value)
{
   return $$reinterpf(value);
}


vec4 uintBitsToFloat(uvec4 value)
{
   return $$reinterpf(value);
}


float sqrt(float x)
{
   return $$rcp($$rsqrt(x));
}


vec2 sqrt(vec2 x)
{
   return $$rcp($$rsqrt(x));
}


vec3 sqrt(vec3 x)
{
   return $$rcp($$rsqrt(x));
}


vec4 sqrt(vec4 x)
{
   return $$rcp($$rsqrt(x));
}


float inversesqrt(float x)
{
   return $$rsqrt(x);
}


vec2 inversesqrt(vec2 x)
{
   return $$rsqrt(x);
}


vec3 inversesqrt(vec3 x)
{
   return $$rsqrt(x);
}


vec4 inversesqrt(vec4 x)
{
   return $$rsqrt(x);
}

highp float frexp(highp float x, out highp int e)
{
   if (x == 0.0) {
      e = 0;
      return x;
   }
   uint bits = floatBitsToUint(x);
   e = (int(bits >> 23) & 0xFF) - 126;
   return uintBitsToFloat( (bits & 0x807FFFFFu) | 0x3F000000u );
}

highp vec2 frexp(highp vec2 x, out highp ivec2 e)
{
   return vec2(frexp(x.x, e.x), frexp(x.y, e.y));
}

highp vec3 frexp(highp vec3 x, out highp ivec3 e)
{
   return vec3(frexp(x.x, e.x), frexp(x.y, e.y), frexp(x.z, e.z));
}

highp vec4 frexp(highp vec4 x, out highp ivec4 e)
{
   return vec4(frexp(x.x, e.x), frexp(x.y, e.y),
               frexp(x.z, e.z), frexp(x.w, e.w));
}

highp float ldexp(highp float x, highp int e)
{
   /* Deal with overflow, push e >= 128 down into range */
   if (e >= 128) {
      const highp float _2exp127 = uintBitsToFloat(0x7F000000u);
      x *= _2exp127;
      e -= 127;
   }

   highp float e_p = intBitsToFloat(max(e + 127, 0) << 23);
   return x * e_p;
}

highp vec2 ldexp(highp vec2 x, highp ivec2 e)
{
   return vec2(ldexp(x.x, e.x), ldexp(x.y, e.y));
}

highp vec3 ldexp(highp vec3 x, highp ivec3 e)
{
   return vec3(ldexp(x.x, e.x), ldexp(x.y, e.y), ldexp(x.z, e.z));
}

highp vec4 ldexp(highp vec4 x, highp ivec4 e)
{
   return vec4(ldexp(x.x, e.x), ldexp(x.y, e.y),
               ldexp(x.z, e.z), ldexp(x.w, e.w));
}

float fma(float a, float b, float c) {
   return a * b + c; }
vec2  fma(vec2  a, vec2  b, vec2  c) {
   return vec2(fma(a.x, b.x, c.x), fma(a.y, b.y, c.y));
}
vec3  fma(vec3  a, vec3  b, vec3  c) {
   return vec3(fma(a.x, b.x, c.x), fma(a.y, b.y, c.y), fma(a.z, b.z, c.z));
}
vec4  fma(vec4  a, vec4  b, vec4  c) {
   return vec4(fma(a.x, b.x, c.x), fma(a.y, b.y, c.y), fma(a.z, b.z, c.z), fma(a.w, b.w, c.w));
}
