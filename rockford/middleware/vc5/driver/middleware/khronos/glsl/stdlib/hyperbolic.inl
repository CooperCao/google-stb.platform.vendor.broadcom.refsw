static void sinh__const_float__const_float(const_float* result, const_float* x)
{
   const_float e = float_to_bits(2.718281828f);
   const_float neg_x, res1, res2;

   neg_x = op_f_sub(0, *x);

   pow__const_float__const_float__const_float(&res1, &e, x);
   pow__const_float__const_float__const_float(&res2, &e, &neg_x);

   res1 = op_f_sub(res1, res2);
   *result = op_f_mul(float_to_bits(0.5f), res1);
}

static void sinh__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      sinh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void sinh__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      sinh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void sinh__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      sinh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void cosh__const_float__const_float(const_float* result, const_float* x)
{
   const_float e = float_to_bits(2.718281828f);
   const_float neg_x, res1, res2;

   neg_x = op_f_sub(0, *x);

   pow__const_float__const_float__const_float(&res1, &e, x);
   pow__const_float__const_float__const_float(&res2, &e, &neg_x);

   res1 = op_f_add(res1, res2);
   *result = op_f_mul(float_to_bits(0.5f), res1);
}
static void cosh__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      cosh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void cosh__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      cosh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void cosh__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      cosh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void tanh__const_float__const_float(const_float* result, const_float* x)
{
   const_float res1, res2;

   sinh__const_float__const_float(&res1, x);
   cosh__const_float__const_float(&res2, x);

   *result = op_f_div(res1, res2);
}
static void tanh__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      tanh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void tanh__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      tanh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void tanh__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      tanh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void asinh__const_float__const_float(const_float* result, const_float* x)
{
   const_float x1, res1;

   x1 = op_f_mul(*x, *x);
   x1 = op_f_add(x1, float_to_bits(1.0f));
   sqrt__const_float__const_float(&res1, &x1);
   x1 = op_f_add(*x, res1);
   log__const_float__const_float(result, &x1);
}
static void asinh__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      asinh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void asinh__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      asinh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void asinh__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      asinh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void acosh__const_float__const_float(const_float* result, const_float* x)
{
   const_float x1, res1;

   x1 = op_f_mul( op_f_sub(*x, CONST_FLOAT_ONE),
                  op_f_add(*x, CONST_FLOAT_ONE));
   sqrt__const_float__const_float(&res1, &x1);
   x1 = op_f_add(*x, res1);
   log__const_float__const_float(result, &x1);
}
static void acosh__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      acosh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void acosh__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      acosh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void acosh__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      acosh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void atanh__const_float__const_float(const_float* result, const_float* x)
{
   const_float xn, xd, x1, res1;

   xd = op_f_sub(float_to_bits(1.0f), *x);
   xd = op_recip(xd);
   xn = op_f_add(float_to_bits(1.0f), *x);
   x1 = op_f_mul(xn, xd);
   log__const_float__const_float(&res1, &x1);
   *result = op_f_mul(res1, float_to_bits(0.5f));
}
static void atanh__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      atanh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void atanh__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      atanh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void atanh__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      atanh__const_float__const_float(&(*result)[i], &(*x)[i]);
}
