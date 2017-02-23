static void pow__const_float__const_float__const_float(const_float* result, const_float* x, const_float* y)
{
   const_float log2x = op_log2(*x);
   const_float temp  = op_f_mul(log2x,*y);
   *result = op_exp2(temp);
}
static void pow__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* x, const_vec2* y)
{
   for (int i = 0; i < 2; i++)
      pow__const_float__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void pow__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* x, const_vec3* y)
{
   for (int i = 0; i < 3; i++)
      pow__const_float__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void pow__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* x, const_vec4* y)
{
   for (int i = 0; i < 4; i++)
      pow__const_float__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void exp__const_float__const_float(const_float* result, const_float* x)
{
   const_float log2_e = 0x3fb8aa3b;
   *result = op_exp2( op_f_mul(*x, log2_e) );
}
static void exp__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   for (int i = 0; i < 2; i++)
      exp__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void exp__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   for (int i = 0; i < 3; i++)
      exp__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void exp__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   for (int i = 0; i < 4; i++)
      exp__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void log__const_float__const_float(const_float* result, const_float* x)
{
   const_float loge_2 = 0x3f317218;
   *result = op_f_mul( op_log2(*x), loge_2);
}
static void log__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   for (int i = 0; i < 2; i++)
      log__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void log__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   for (int i = 0; i < 3; i++)
      log__const_float__const_float(&(*result)[i], &(*x)[i]);
}

static void log__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   for (int i = 0; i < 4; i++)
      log__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void exp2__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_exp2(*x);
}
static void exp2__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   for (int i = 0; i < 2; i++)
      exp2__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void exp2__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   for (int i = 0; i < 3; i++)
      exp2__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void exp2__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   for (int i = 0; i < 4; i++)
      exp2__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void log2__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_log2(*x);
}
static void log2__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   for (int i = 0; i < 2; i++)
      log2__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void log2__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   for (int i = 0; i < 3; i++)
      log2__const_float__const_float(&(*result)[i], &(*x)[i]);
}
static void log2__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   for (int i = 0; i < 4; i++)
      log2__const_float__const_float(&(*result)[i], &(*x)[i]);
}
