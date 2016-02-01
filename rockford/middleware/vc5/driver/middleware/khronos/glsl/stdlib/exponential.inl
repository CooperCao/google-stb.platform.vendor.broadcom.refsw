static void pow__const_float__const_float__const_float(const_float* result, const_float* x, const_float* y)
{
   unsigned log2x,temp;
   log2x = op_log2(*x);
   temp  = op_f_mul(log2x,*y);
   *result = op_exp2(temp);
}
static void pow__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* x, const_vec2* y)
{
   int i;
   for (i = 0; i < 2; i++)
      pow__const_float__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void pow__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* x, const_vec3* y)
{
   int i;
   for (i = 0; i < 3; i++)
      pow__const_float__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void pow__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* x, const_vec4* y)
{
   int i;
   for (i = 0; i < 4; i++)
      pow__const_float__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void exp__const_float__const_float(const_float* result, const_float* x)
{
   const_float e = float_to_bits((float)atof("2.718281828"));

   pow__const_float__const_float__const_float(result, &e, x);
}
static void exp__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      exp__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void exp__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      exp__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void exp__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      exp__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void log__const_float__const_float(const_float* result, const_float* x)
{
   const_float rlog2e;
   const_float log2x;

   rlog2e = float_to_bits((float)atof("0.6931471805"));

   log2__const_float__const_float(&log2x, x);
   *result = op_f_mul((unsigned)log2x,rlog2e);
}
static void log__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      log__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void log__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      log__const_float__const_float(&(*result)[i], &(*x)[i]);
}

static void log__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      log__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void exp2__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_exp2(*x);
}
static void exp2__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   int i;
   for (i = 0; i < 2; i++)
      exp2__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void exp2__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   int i;
   for (i = 0; i < 3; i++)
      exp2__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void exp2__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   int i;
   for (i = 0; i < 4; i++)
      exp2__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void log2__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_log2(*x);
}
static void log2__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   log2__const_float__const_float(&(*result)[0], &(*x)[0]);
   log2__const_float__const_float(&(*result)[1], &(*x)[1]);
}
static void log2__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   log2__const_float__const_float(&(*result)[0], &(*x)[0]);
   log2__const_float__const_float(&(*result)[1], &(*x)[1]);
   log2__const_float__const_float(&(*result)[2], &(*x)[2]);
}
static void log2__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   log2__const_float__const_float(&(*result)[0], &(*x)[0]);
   log2__const_float__const_float(&(*result)[1], &(*x)[1]);
   log2__const_float__const_float(&(*result)[2], &(*x)[2]);
   log2__const_float__const_float(&(*result)[3], &(*x)[3]);
}
