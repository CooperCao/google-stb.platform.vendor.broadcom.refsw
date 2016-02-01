//
// Angle and Trigonometry Functions
//


float radians(float degrees)
{
   const float pi_on_180 = 0.01745329252;
   return pi_on_180 * degrees;
}


vec2 radians(vec2 degrees)
{
   vec2 r;
   const float pi_on_180 = 0.01745329252;

   r[0] = pi_on_180 * degrees[0];
   r[1] = pi_on_180 * degrees[1];

   return r;
}


vec3 radians(vec3 degrees)
{
   vec3 r;
   const float pi_on_180 = 0.01745329252;

   r[0] = pi_on_180 * degrees[0];
   r[1] = pi_on_180 * degrees[1];
   r[2] = pi_on_180 * degrees[2];

   return r;
}


vec4 radians(vec4 degrees)
{
   vec4 r;
   const float pi_on_180 = 0.01745329252;

   r[0] = pi_on_180 * degrees[0];
   r[1] = pi_on_180 * degrees[1];
   r[2] = pi_on_180 * degrees[2];
   r[3] = pi_on_180 * degrees[3];

   return r;
}


float degrees(float radians)
{
   const float _180_on_pi = 57.29577951;
   return _180_on_pi * radians;
}


vec2 degrees(vec2 radians)
{
   vec2 r;
   const float _180_on_pi = 57.29577951;

   r[0] = _180_on_pi * radians[0];
   r[1] = _180_on_pi * radians[1];

   return r;
}


vec3 degrees(vec3 radians)
{
   vec3 r;
   const float _180_on_pi = 57.29577951;

   r[0] = _180_on_pi * radians[0];
   r[1] = _180_on_pi * radians[1];
   r[2] = _180_on_pi * radians[2];

   return r;
}


vec4 degrees(vec4 radians)
{
   vec4 r;
   const float _180_on_pi = 57.29577951;

   r[0] = _180_on_pi * radians[0];
   r[1] = _180_on_pi * radians[1];
   r[2] = _180_on_pi * radians[2];
   r[3] = _180_on_pi * radians[3];

   return r;
}

float sin(float angle)
{
   return $$sin(angle);
}


vec2 sin(vec2 angle)
{
   return $$sin(angle);
}


vec3 sin(vec3 angle)
{
   return $$sin(angle);
}


vec4 sin(vec4 angle)
{
   return $$sin(angle);
}


float cos(float angle)
{
   return $$cos(angle);
}


vec2 cos(vec2 angle)
{
   return $$cos(angle);
}


vec3 cos(vec3 angle)
{
   return $$cos(angle);
}


vec4 cos(vec4 angle)
{
   return $$cos(angle);
}


float tan(float angle)
{
   return $$tan(angle);
}

vec2 tan(vec2 angle)
{
   return $$tan(angle);
}


vec3 tan(vec3 angle)
{
   return $$tan(angle);
}


vec4 tan(vec4 angle)
{
   return $$tan(angle);
}

float atan(float y, float x)
{
   const float PI = 3.1415926535897932384626433832795;
   float quadrant_offset;
   float y_on_x;

   /* Compute atan2 by working out an offset based on the quadrant and calling
      atan(y/x). This gives correct values unless y/x == #NaN, so resolve those
      cases separately.

      Quadrant offset only gets correct results if -0.0 < +0.0 so we use bitops
      rather than fcmp. Constructing the offset by crazy masking is just because
      it's fast.
   */
   int    x_sgn_mask = ($$reinterpi(x) & 0x80000000) >> 31;
   float y_quad_sign = $$reinterpf(($$reinterpi(y) & 0x80000000) | $$reinterpi(1.0));

   /* Correct for y/x == #NaN. Ignore 0/0 because results are undefined, so we
      only need to correct #Inf/#Inf                                          */
   if (__isinf(x) && __isinf(y)) {
      int sign_bit = ($$reinterpi(x) ^ $$reinterpi(y)) & 0x80000000;
      y_on_x = $$reinterpf(sign_bit | $$reinterpi(1.0));
   } else {
      y_on_x = y / x;
   }

   quadrant_offset = $$reinterpf($$reinterpi(PI) & x_sgn_mask);
   quadrant_offset *= y_quad_sign;

   return quadrant_offset + atan( y_on_x );
}


vec2 atan(vec2 y, vec2 x)
{
   return vec2(atan(y[0], x[0]), atan(y[1], x[1]));
}


vec3 atan(vec3 y, vec3 x)
{
   return vec3(atan(y[0], x[0]), atan(y[1], x[1]), atan(y[2], x[2]));
}


vec4 atan(vec4 y, vec4 x)
{
   return vec4(atan(y[0], x[0]), atan(y[1], x[1]), atan(y[2], x[2]), atan(y[3], x[3]));
}


float atan(float y_over_x)
{
   const float T3PO8 = 2.414213562373;
   const float TPO8  = 0.414213562373;
   const float PO2   = 1.570796326794;
   const float PO4   = 0.785398163397;
   float x = y_over_x;
   float c = 0.0;
   float sgn_x = 1.0;
   if (x < 0.0) {
      x = -x;
      sgn_x = -1.0;
   }

   if (x > T3PO8) {
      x = -1.0/x;
      c = PO2;
   } else if (x > TPO8) {
      x = (x-1.0)/(x+1.0);
      c = PO4;
   }
   float z = x*x;
   return sgn_x * (c + x + x*z*(  8.05374449538e-2*z*z*z
                                - 1.38776856032e-1*z*z
                                + 1.99777106478e-1*z
                                - 3.33329491539e-1));
}


vec2 atan(vec2 y_over_x)
{
   return vec2(atan(y_over_x[0]), atan(y_over_x[1]));
}


vec3 atan(vec3 y_over_x)
{
   return vec3(atan(y_over_x[0]), atan(y_over_x[1]), atan(y_over_x[2]));
}


vec4 atan(vec4 y_over_x)
{
   return vec4(atan(y_over_x[0]), atan(y_over_x[1]), atan(y_over_x[2]), atan(y_over_x[3]));
}


float asin(float x)
{
   return atan(x / sqrt(1.0-x*x));
}


vec2 asin(vec2 x)
{
   return vec2(asin(x[0]), asin(x[1]));
}


vec3 asin(vec3 x)
{
   return vec3(asin(x[0]), asin(x[1]), asin(x[2]));
}


vec4 asin(vec4 x)
{
   return vec4(asin(x[0]), asin(x[1]), asin(x[2]), asin(x[3]));
}


float acos(float x)
{
   return 2.0*atan(sqrt((1.0-x)/(1.0+x)));
}


vec2 acos(vec2 x)
{
   return vec2(acos(x[0]), acos(x[1]));
}


vec3 acos(vec3 x)
{
   return vec3(acos(x[0]), acos(x[1]), acos(x[2]));
}


vec4 acos(vec4 x)
{
   return vec4(acos(x[0]), acos(x[1]), acos(x[2]), acos(x[3]));
}
