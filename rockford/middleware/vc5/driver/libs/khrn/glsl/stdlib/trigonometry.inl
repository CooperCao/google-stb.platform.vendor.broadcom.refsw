static void radians__const_float__const_float(const_float* result, const_float* degrees)
{
   /*
      const float pi_on_180 = 0.01745329252;
      return pi_on_180 * degrees;
   */

   const_float pi_on_180 = float_to_bits((float)atof("0.01745329252"));

   *result = op_f_mul(pi_on_180, *degrees);
}
static void radians__const_vec2__const_vec2(const_vec2* result, const_vec2* degrees)
{
   int i;
   for (i = 0; i < 2; i++)
      radians__const_float__const_float(&(*result)[i], &(*degrees)[i]);
}
static void radians__const_vec3__const_vec3(const_vec3* result, const_vec3* degrees)
{
   int i;
   for (i = 0; i < 3; i++)
      radians__const_float__const_float(&(*result)[i], &(*degrees)[i]);
}
static void radians__const_vec4__const_vec4(const_vec4* result, const_vec4* degrees)
{
   int i;
   for (i = 0; i < 4; i++)
      radians__const_float__const_float(&(*result)[i], &(*degrees)[i]);
}
static void degrees__const_float__const_float(const_float* result, const_float* radians)
{
   /*
      const float _180_on_pi = 57.29577951;
      return _180_on_pi * radians;
   */
   const_float _180_on_pi = float_to_bits((float)atof("57.29577951"));

   *result = op_f_mul(_180_on_pi, *radians);
}
static void degrees__const_vec2__const_vec2(const_vec2* result, const_vec2* radians)
{
   int i;
   for (i = 0; i < 2; i++)
      degrees__const_float__const_float(&(*result)[i], &(*radians)[i]);
}
static void degrees__const_vec3__const_vec3(const_vec3* result, const_vec3* radians)
{
   int i;
   for (i = 0; i < 3; i++)
      degrees__const_float__const_float(&(*result)[i], &(*radians)[i]);
}
static void degrees__const_vec4__const_vec4(const_vec4* result, const_vec4* radians)
{
   int i;
   for (i = 0; i < 4; i++)
      degrees__const_float__const_float(&(*result)[i], &(*radians)[i]);
}
static void sin__const_float__const_float(const_float* result, const_float* angle)
{
   *result = op_sin(*angle);
}
static void sin__const_vec2__const_vec2(const_vec2* result, const_vec2* angle)
{
   int i;
   for (i = 0; i < 2; i++)
      sin__const_float__const_float(&(*result)[i], &(*angle)[i]);
}
static void sin__const_vec3__const_vec3(const_vec3* result, const_vec3* angle)
{
   int i;
   for (i = 0; i < 3; i++)
      sin__const_float__const_float(&(*result)[i], &(*angle)[i]);
}
static void sin__const_vec4__const_vec4(const_vec4* result, const_vec4* angle)
{
   int i;
   for (i = 0; i < 4; i++)
      sin__const_float__const_float(&(*result)[i], &(*angle)[i]);
}
static void cos__const_float__const_float(const_float* result, const_float* angle)
{
   *result = op_cos(*angle);
}
static void cos__const_vec2__const_vec2(const_vec2* result, const_vec2* angle)
{
   int i;
   for (i = 0; i < 2; i++)
      cos__const_float__const_float(&(*result)[i], &(*angle)[i]);
}
static void cos__const_vec3__const_vec3(const_vec3* result, const_vec3* angle)
{
   int i;
   for (i = 0; i < 3; i++)
      cos__const_float__const_float(&(*result)[i], &(*angle)[i]);
}
static void cos__const_vec4__const_vec4(const_vec4* result, const_vec4* angle)
{
   int i;
   for (i = 0; i < 4; i++)
      cos__const_float__const_float(&(*result)[i], &(*angle)[i]);
}
static void tan__const_float__const_float(const_float* result, const_float* angle)
{
   *result = op_tan(*angle);
}
static void tan__const_vec2__const_vec2(const_vec2* result, const_vec2* angle)
{
   int i;
   for (i = 0; i < 2; i++)
      tan__const_float__const_float(&(*result)[i], &(*angle)[i]);
}
static void tan__const_vec3__const_vec3(const_vec3* result, const_vec3* angle)
{
   int i;
   for (i = 0; i < 3; i++)
      tan__const_float__const_float(&(*result)[i], &(*angle)[i]);
}
static void tan__const_vec4__const_vec4(const_vec4* result, const_vec4* angle)
{
   int i;
   for (i = 0; i < 4; i++)
      tan__const_float__const_float(&(*result)[i], &(*angle)[i]);
}
static void asin__const_float__const_float(const_float* result, const_float* x)
{
   const_float one = CONST_FLOAT_ONE;
   const_float t1, t2, t3, t4;

   t1 = op_f_mul(*x, *x);
   t2 = op_f_sub(one, t1);
   t3 = op_rsqrt(t2);
   t4 = op_f_mul(*x, t3);
   atan__const_float__const_float(result, &t4);
}
static void asin__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      asin__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void asin__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      asin__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void asin__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      asin__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void acos__const_float__const_float(const_float* result, const_float* x)
{
   const_float one = CONST_FLOAT_ONE;
   const_float t1, t2, t3;

   t1 = op_f_div(op_f_sub(one, *x), op_f_add(one, *x));
   sqrt__const_float__const_float(&t2, &t1);
   atan__const_float__const_float(&t3, &t2);
   *result = op_f_mul(t3, CONST_FLOAT_TWO);
}
static void acos__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      acos__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void acos__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      acos__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void acos__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      acos__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void atan__const_float__const_float__const_float(const_float* result, const_float* y, const_float* x)
{
   const_float PI = float_to_bits((float)atof("3.1415926535897932384626433832795"));
   const_float quadrant_offset;
   const_float y_on_x;
   const_signed x_sgn_mask = (*x & CONST_INT_SIGN_BIT);
   x_sgn_mask >>= 31;
   const_float y_quad_sign = (*y & CONST_INT_SIGN_BIT) | CONST_FLOAT_ONE;

   const_bool x_inf, y_inf;
   isinf__const_bool__const_float(&x_inf, x);
   isinf__const_bool__const_float(&y_inf, y);
   if (x_inf && y_inf) {
      const_signed sign_bit = (*x ^ *y) & CONST_INT_SIGN_BIT;
      y_on_x = sign_bit | CONST_FLOAT_ONE;
   } else {
      y_on_x = op_f_div(*y, *x);
   }
   quadrant_offset = PI & x_sgn_mask;
   quadrant_offset = op_f_mul(quadrant_offset, y_quad_sign);

   const_float r;
   atan__const_float__const_float(&r, &y_on_x);
   *result = op_f_add(quadrant_offset, r);
}
static void atan__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* y, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      atan__const_float__const_float__const_float(&(*result)[i], &(*y)[i], &(*x)[i]);
}
static void atan__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* y, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      atan__const_float__const_float__const_float(&(*result)[i], &(*y)[i], &(*x)[i]);
}
static void atan__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* y, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      atan__const_float__const_float__const_float(&(*result)[i], &(*y)[i], &(*x)[i]);
}
static void atan__const_float__const_float(const_float* result, const_float* y_over_x)
{
   const_float T3PO8 = float_to_bits((float)atof("2.414213562373"));
   const_float TPO8  = float_to_bits((float)atof("0.414213562373"));
   const_float PO2   = float_to_bits((float)atof("1.570796326794"));
   const_float PO4   = float_to_bits((float)atof("0.785398163397"));
   const_float x     = *y_over_x;
   const_float c     = 0;
   const_float sgn_x = CONST_FLOAT_ONE;

   if (op_f_less_than(x, CONST_FLOAT_ZERO)) {
      x = op_f_negate(x);
      sgn_x = CONST_FLOAT_MINUS_ONE;
   }

   if (op_f_greater_than(x, T3PO8)) {
      x = op_f_negate(op_recip(x));
      c = PO2;
   } else if (op_f_greater_than(x, TPO8)) {
      x = op_f_div(op_f_sub(x, CONST_FLOAT_ONE), op_f_add(x, CONST_FLOAT_ONE));
      c = PO4;
   }

   const_float z = op_f_mul(x,x);
   const_float coeffs[4] = { float_to_bits((float)atof("8.05374449538e-2")),
                             float_to_bits((float)atof("1.38776856032e-1")),
                             float_to_bits((float)atof("1.99777106478e-1")),
                             float_to_bits((float)atof("3.33329491539e-1")) };
   const_float r3 = op_f_mul(op_f_mul(op_f_mul(coeffs[0], z), z), z);
   const_float r2 = op_f_mul(op_f_mul(coeffs[1], z), z);
   const_float r1 = op_f_mul(coeffs[2], z);
   const_float r0 = coeffs[3];
   const_float r = op_f_sub(op_f_add(op_f_sub(r3, r2), r1), r0);
   const_float s = op_f_add(op_f_add(c, x), op_f_mul(op_f_mul(x, z), r));
   *result = op_f_mul(sgn_x, s);
}
static void atan__const_vec2__const_vec2(const_vec2* result, const_vec2* y_over_x)
{
   int i;
   for (i = 0; i < 2; i++)
      atan__const_float__const_float(&(*result)[i], &(*y_over_x)[i]);
}
static void atan__const_vec3__const_vec3(const_vec3* result, const_vec3* y_over_x)
{
   int i;
   for (i = 0; i < 3; i++)
      atan__const_float__const_float(&(*result)[i], &(*y_over_x)[i]);
}
static void atan__const_vec4__const_vec4(const_vec4* result, const_vec4* y_over_x)
{
   int i;
   for (i = 0; i < 4; i++)
      atan__const_float__const_float(&(*result)[i], &(*y_over_x)[i]);
}
