//
// Angle and Trigonometry Functions
//


float radians(float degrees) {
   const highp float pi_on_180 = 0.01745329252;
   return pi_on_180 * degrees;
}

vec2 radians(vec2 degrees) {
   const highp float pi_on_180 = 0.01745329252;
   return pi_on_180 * degrees;
}

vec3 radians(vec3 degrees) {
   const highp float pi_on_180 = 0.01745329252;
   return pi_on_180 * degrees;
}

vec4 radians(vec4 degrees) {
   const highp float pi_on_180 = 0.01745329252;
   return pi_on_180 * degrees;
}

float degrees(float radians) {
   const highp float _180_on_pi = 57.29577951;
   return _180_on_pi * radians;
}


vec2 degrees(vec2 radians) {
   const highp float _180_on_pi = 57.29577951;
   return _180_on_pi * radians;
}

vec3 degrees(vec3 radians) {
   const highp float _180_on_pi = 57.29577951;
   return _180_on_pi * radians;
}

vec4 degrees(vec4 radians) {
   const highp float _180_on_pi = 57.29577951;
   return _180_on_pi * radians;
}

float sin(float angle) { return $$sin(angle); }
vec2  sin(vec2  angle) { return $$sin(angle); }
vec3  sin(vec3  angle) { return $$sin(angle); }
vec4  sin(vec4  angle) { return $$sin(angle); }

float cos(float angle) { return $$cos(angle); }
vec2  cos(vec2  angle) { return $$cos(angle); }
vec3  cos(vec3  angle) { return $$cos(angle); }
vec4  cos(vec4  angle) { return $$cos(angle); }

float tan(float angle) { return $$tan(angle); }
vec2  tan(vec2  angle) { return $$tan(angle); }
vec3  tan(vec3  angle) { return $$tan(angle); }
vec4  tan(vec4  angle) { return $$tan(angle); }

float atan(float y, float x)
{
   const highp float PI = 3.1415926535897932384626433832795;

   /* Compute atan2 by working out an offset based on the quadrant and calling
      atan(y/x). This gives correct values unless y/x == #NaN, so resolve those
      cases separately.

      Quadrant offset only gets correct results if -0.0 < +0.0 so we use bitops
      rather than fcmp. Constructing the offset by crazy masking is just because
      it's fast.
   */
   int x_sgn_mask = $$reinterpi(x) >> 31;
   highp int y_quad_sign = $$reinterpi(y) & 0x80000000;

   /* Correct for y/x == #NaN. Ignore 0/0 because results are undefined, so we
      only need to correct #Inf/#Inf                                          */
   highp float y_on_x;
   if (__isinf(x) && abs(x) == abs(y)) {
      int sign_bit = ($$reinterpi(x) ^ $$reinterpi(y)) & 0x80000000;
      y_on_x = $$reinterpf(sign_bit + $$reinterpi(1.0));
   } else {
      y_on_x = y / x;
   }

   highp int quadrant_offset = $$reinterpi(PI) & x_sgn_mask;
   quadrant_offset ^= y_quad_sign;

   return $$reinterpf(quadrant_offset) + atan( y_on_x );
}

vec2 atan(vec2 y, vec2 x) {
   return vec2(atan(y[0], x[0]), atan(y[1], x[1]));
}

vec3 atan(vec3 y, vec3 x) {
   return vec3(atan(y[0], x[0]), atan(y[1], x[1]), atan(y[2], x[2]));
}

vec4 atan(vec4 y, vec4 x) {
   return vec4(atan(y[0], x[0]), atan(y[1], x[1]), atan(y[2], x[2]), atan(y[3], x[3]));
}


float atan(float y_over_x)
{
   const highp float T3PO8 = 2.414213562373;
   const highp float TPO8  = 0.414213562373;
   const highp float PO2   = 1.570796326794;
   const highp float PO4   = 0.785398163397;
   highp float x = y_over_x;
   highp float c = 0.0;
   highp uint sgn_x = $$reinterpu(x) & 0x80000000u;

   x = abs(x);

   if (x > T3PO8) {
      x = -1.0/x;
      c = PO2;
   } else if (x > TPO8) {
      x = (x-1.0)/(x+1.0);
      c = PO4;
   }
   highp float z  = x*x;
   return $$reinterpf(sgn_x ^ $$reinterpu(c + x + x*z*(((  8.05374449538e-2  * z
                                                         - 1.38776856032e-1) * z
                                                         + 1.99777106478e-1) * z
                                                         - 3.33329491539e-1)));
}

vec2 atan(vec2 y_over_x) {
   return vec2(atan(y_over_x[0]), atan(y_over_x[1]));
}

vec3 atan(vec3 y_over_x) {
   return vec3(atan(y_over_x[0]), atan(y_over_x[1]), atan(y_over_x[2]));
}

vec4 atan(vec4 y_over_x) {
   return vec4(atan(y_over_x[0]), atan(y_over_x[1]), atan(y_over_x[2]), atan(y_over_x[3]));
}

float asin(float x) {
   const highp float PO2 = 1.570796326794;
   highp float c = 0.0;
   highp float f = 1.0;
   highp uint sgn_x = $$reinterpu(x) & 0x80000000u;

   x = abs(x);

   if (x > 0.5) {
      c = PO2;
      f = -2.0;
      x = inversesqrt(2.0/(1.0-x));
   }
   highp float z  = x * x;
   highp float r = ((((  4.2163199048E-2  * z
                       + 2.4181311049E-2) * z
                       + 4.5470025998E-2) * z
                       + 7.4953002686E-2) * z
                       + 1.6666752422E-1) * z * x
                       + x;

   return $$reinterpf(sgn_x ^ $$reinterpu(c + f * r));
}

vec2 asin(vec2 x) {
   return vec2(asin(x[0]), asin(x[1]));
}

vec3 asin(vec3 x) {
   return vec3(asin(x[0]), asin(x[1]), asin(x[2]));
}

vec4 asin(vec4 x) {
   return vec4(asin(x[0]), asin(x[1]), asin(x[2]), asin(x[3]));
}


float acos(float x) {
   return 2.0*atan(inversesqrt((1.0+x)/(1.0-x)));
}

vec2 acos(vec2 x) {
   return vec2(acos(x[0]), acos(x[1]));
}

vec3 acos(vec3 x) {
   return vec3(acos(x[0]), acos(x[1]), acos(x[2]));
}

vec4 acos(vec4 x) {
   return vec4(acos(x[0]), acos(x[1]), acos(x[2]), acos(x[3]));
}
