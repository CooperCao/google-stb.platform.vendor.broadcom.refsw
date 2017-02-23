static void lessThan__const_bool__const_float__const_float(const_bool* result, const_float* x, const_float* y)
{
   *result = op_f_less_than(*x, *y);
}
static void lessThan__const_bvec2__const_vec2__const_vec2(const_bvec2* result, const_vec2* x, const_vec2* y)
{
   for (int i = 0; i < 2; i++)
      lessThan__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThan__const_bvec3__const_vec3__const_vec3(const_bvec3* result, const_vec3* x, const_vec3* y)
{
   for (int i = 0; i < 3; i++)
      lessThan__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThan__const_bvec4__const_vec4__const_vec4(const_bvec4* result, const_vec4* x, const_vec4* y)
{
   for (int i = 0; i < 4; i++)
      lessThan__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThan__const_bool__const_int__const_int(const_bool* result, const_int* x, const_int* y)
{
   if (*(const_signed*)x < *(const_signed*)y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void lessThan__const_bvec2__const_ivec2__const_ivec2(const_bvec2* result, const_ivec2* x, const_ivec2* y)
{
   for (int i = 0; i < 2; i++)
      lessThan__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThan__const_bvec3__const_ivec3__const_ivec3(const_bvec3* result, const_ivec3* x, const_ivec3* y)
{
   for (int i = 0; i < 3; i++)
      lessThan__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThan__const_bvec4__const_ivec4__const_ivec4(const_bvec4* result, const_ivec4* x, const_ivec4* y)
{
   for (int i = 0; i < 4; i++)
      lessThan__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThan__const_bool__const_uint__const_uint(const_bool* result, const_uint* x, const_uint* y)
{
   if (*x < *y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void lessThan__const_bvec2__const_uvec2__const_uvec2(const_bvec2* result, const_uvec2* x, const_uvec2* y)
{
   for (int i = 0; i < 2; i++)
      lessThan__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThan__const_bvec3__const_uvec3__const_uvec3(const_bvec3* result, const_uvec3* x, const_uvec3* y)
{
   for (int i = 0; i < 3; i++)
      lessThan__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThan__const_bvec4__const_uvec4__const_uvec4(const_bvec4* result, const_uvec4* x, const_uvec4* y)
{
   for (int i = 0; i < 4; i++)
      lessThan__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThanEqual__const_bool__const_float__const_float(const_bool* result, const_float* x, const_float* y)
{
   *result = op_f_less_than_equal(*x, *y);
}
static void lessThanEqual__const_bvec2__const_vec2__const_vec2(const_bvec2* result, const_vec2* x, const_vec2* y)
{
   for (int i = 0; i < 2; i++)
      lessThanEqual__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThanEqual__const_bvec3__const_vec3__const_vec3(const_bvec3* result, const_vec3* x, const_vec3* y)
{
   for (int i = 0; i < 3; i++)
      lessThanEqual__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThanEqual__const_bvec4__const_vec4__const_vec4(const_bvec4* result, const_vec4* x, const_vec4* y)
{
   for (int i = 0; i < 4; i++)
      lessThanEqual__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThanEqual__const_bool__const_int__const_int(const_bool* result, const_int* x, const_int* y)
{
   if (*(const_signed*)x <= *(const_signed*)y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void lessThanEqual__const_bvec2__const_ivec2__const_ivec2(const_bvec2* result, const_ivec2* x, const_ivec2* y)
{
   for (int i = 0; i < 2; i++)
      lessThanEqual__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThanEqual__const_bvec3__const_ivec3__const_ivec3(const_bvec3* result, const_ivec3* x, const_ivec3* y)
{
   for (int i = 0; i < 3; i++)
      lessThanEqual__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThanEqual__const_bvec4__const_ivec4__const_ivec4(const_bvec4* result, const_ivec4* x, const_ivec4* y)
{
   for (int i = 0; i < 4; i++)
      lessThanEqual__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThanEqual__const_bool__const_uint__const_uint(const_bool* result, const_uint* x, const_uint* y)
{
   if (*x <= *y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void lessThanEqual__const_bvec2__const_uvec2__const_uvec2(const_bvec2* result, const_uvec2* x, const_uvec2* y)
{
   for (int i = 0; i < 2; i++)
      lessThanEqual__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThanEqual__const_bvec3__const_uvec3__const_uvec3(const_bvec3* result, const_uvec3* x, const_uvec3* y)
{
   for (int i = 0; i < 3; i++)
      lessThanEqual__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void lessThanEqual__const_bvec4__const_uvec4__const_uvec4(const_bvec4* result, const_uvec4* x, const_uvec4* y)
{
   for (int i = 0; i < 4; i++)
      lessThanEqual__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThan__const_bool__const_float__const_float(const_bool* result, const_float* x, const_float* y)
{
   *result = op_f_greater_than(*x, *y);
}
static void greaterThan__const_bvec2__const_vec2__const_vec2(const_bvec2* result, const_vec2* x, const_vec2* y)
{
   for (int i = 0; i < 2; i++)
      greaterThan__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThan__const_bvec3__const_vec3__const_vec3(const_bvec3* result, const_vec3* x, const_vec3* y)
{
   for (int i = 0; i < 3; i++)
      greaterThan__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThan__const_bvec4__const_vec4__const_vec4(const_bvec4* result, const_vec4* x, const_vec4* y)
{
   for (int i = 0; i < 4; i++)
      greaterThan__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThan__const_bool__const_int__const_int(const_bool* result, const_int* x, const_int* y)
{
   if (*(const_signed*)x > *(const_signed*)y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void greaterThan__const_bvec2__const_ivec2__const_ivec2(const_bvec2* result, const_ivec2* x, const_ivec2* y)
{
   for (int i = 0; i < 2; i++)
      greaterThan__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThan__const_bvec3__const_ivec3__const_ivec3(const_bvec3* result, const_ivec3* x, const_ivec3* y)
{
   for (int i = 0; i < 3; i++)
      greaterThan__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThan__const_bvec4__const_ivec4__const_ivec4(const_bvec4* result, const_ivec4* x, const_ivec4* y)
{
   for (int i = 0; i < 4; i++)
      greaterThan__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThan__const_bool__const_uint__const_uint(const_bool* result, const_uint* x, const_uint* y)
{
   if (*(const_signed*)x > *(const_signed*)y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void greaterThan__const_bvec2__const_uvec2__const_uvec2(const_bvec2* result, const_uvec2* x, const_uvec2* y)
{
   for (int i = 0; i < 2; i++)
      greaterThan__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThan__const_bvec3__const_uvec3__const_uvec3(const_bvec3* result, const_uvec3* x, const_uvec3* y)
{
   for (int i = 0; i < 3; i++)
      greaterThan__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThan__const_bvec4__const_uvec4__const_uvec4(const_bvec4* result, const_uvec4* x, const_uvec4* y)
{
   for (int i = 0; i < 4; i++)
      greaterThan__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThanEqual__const_bool__const_float__const_float(const_bool* result, const_float* x, const_float* y)
{
   *result = op_f_greater_than_equal(*x, *y);
}
static void greaterThanEqual__const_bvec2__const_vec2__const_vec2(const_bvec2* result, const_vec2* x, const_vec2* y)
{
   for (int i = 0; i < 2; i++)
      greaterThanEqual__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThanEqual__const_bvec3__const_vec3__const_vec3(const_bvec3* result, const_vec3* x, const_vec3* y)
{
   for (int i = 0; i < 3; i++)
      greaterThanEqual__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThanEqual__const_bvec4__const_vec4__const_vec4(const_bvec4* result, const_vec4* x, const_vec4* y)
{
   for (int i = 0; i < 4; i++)
      greaterThanEqual__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThanEqual__const_bool__const_int__const_int(const_bool* result, const_int* x, const_int* y)
{
   if (*(const_signed*)x >= *(const_signed*)y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void greaterThanEqual__const_bvec2__const_ivec2__const_ivec2(const_bvec2* result, const_ivec2* x, const_ivec2* y)
{
   for (int i = 0; i < 2; i++)
      greaterThanEqual__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThanEqual__const_bvec3__const_ivec3__const_ivec3(const_bvec3* result, const_ivec3* x, const_ivec3* y)
{
   for (int i = 0; i < 3; i++)
      greaterThanEqual__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThanEqual__const_bvec4__const_ivec4__const_ivec4(const_bvec4* result, const_ivec4* x, const_ivec4* y)
{
   for (int i = 0; i < 4; i++)
      greaterThanEqual__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThanEqual__const_bool__const_uint__const_uint(const_bool* result, const_uint* x, const_uint* y)
{
   if (*x >= *y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void greaterThanEqual__const_bvec2__const_uvec2__const_uvec2(const_bvec2* result, const_uvec2* x, const_uvec2* y)
{
   for (int i = 0; i < 2; i++)
      greaterThanEqual__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThanEqual__const_bvec3__const_uvec3__const_uvec3(const_bvec3* result, const_uvec3* x, const_uvec3* y)
{
   for (int i = 0; i < 3; i++)
      greaterThanEqual__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void greaterThanEqual__const_bvec4__const_uvec4__const_uvec4(const_bvec4* result, const_uvec4* x, const_uvec4* y)
{
   for (int i = 0; i < 4; i++)
      greaterThanEqual__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bool__const_float__const_float(const_bool* result, const_float* x, const_float* y)
{
   *result = op_f_equal(*x, *y);
}
static void equal__const_bvec2__const_vec2__const_vec2(const_bvec2* result, const_vec2* x, const_vec2* y)
{
   for (int i = 0; i < 2; i++)
      equal__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bvec3__const_vec3__const_vec3(const_bvec3* result, const_vec3* x, const_vec3* y)
{
   for (int i = 0; i < 3; i++)
      equal__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bvec4__const_vec4__const_vec4(const_bvec4* result, const_vec4* x, const_vec4* y)
{
   for (int i = 0; i < 4; i++)
      equal__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bool__const_int__const_int(const_bool* result, const_int* x, const_int* y)
{
   if (*x == *y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void equal__const_bvec2__const_ivec2__const_ivec2(const_bvec2* result, const_ivec2* x, const_ivec2* y)
{
   for (int i = 0; i < 2; i++)
      equal__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bvec3__const_ivec3__const_ivec3(const_bvec3* result, const_ivec3* x, const_ivec3* y)
{
   for (int i = 0; i < 3; i++)
      equal__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bvec4__const_ivec4__const_ivec4(const_bvec4* result, const_ivec4* x, const_ivec4* y)
{
   for (int i = 0; i < 4; i++)
      equal__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bool__const_uint__const_uint(const_bool* result, const_uint* x, const_uint* y)
{
   if (*x == *y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void equal__const_bvec2__const_uvec2__const_uvec2(const_bvec2* result, const_uvec2* x, const_uvec2* y)
{
   for (int i = 0; i < 2; i++)
      equal__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bvec3__const_uvec3__const_uvec3(const_bvec3* result, const_uvec3* x, const_uvec3* y)
{
   for (int i = 0; i < 3; i++)
      equal__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bvec4__const_uvec4__const_uvec4(const_bvec4* result, const_uvec4* x, const_uvec4* y)
{
   for (int i = 0; i < 4; i++)
      equal__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bool__const_bool__const_bool(const_bool* result, const_bool* x, const_bool* y)
{
   if (*x == *y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void equal__const_bvec2__const_bvec2__const_bvec2(const_bvec2* result, const_bvec2* x, const_bvec2* y)
{
   for (int i = 0; i < 2; i++)
      equal__const_bool__const_bool__const_bool(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bvec3__const_bvec3__const_bvec3(const_bvec3* result, const_bvec3* x, const_bvec3* y)
{
   for (int i = 0; i < 3; i++)
      equal__const_bool__const_bool__const_bool(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void equal__const_bvec4__const_bvec4__const_bvec4(const_bvec4* result, const_bvec4* x, const_bvec4* y)
{
   for (int i = 0; i < 4; i++)
      equal__const_bool__const_bool__const_bool(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bool__const_float__const_float(const_bool* result, const_float* x, const_float* y)
{
   *result = op_f_not_equal(*x, *y);
}
static void notEqual__const_bvec2__const_vec2__const_vec2(const_bvec2* result, const_vec2* x, const_vec2* y)
{
   for (int i = 0; i < 2; i++)
      notEqual__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bvec3__const_vec3__const_vec3(const_bvec3* result, const_vec3* x, const_vec3* y)
{
   for (int i = 0; i < 3; i++)
      notEqual__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bvec4__const_vec4__const_vec4(const_bvec4* result, const_vec4* x, const_vec4* y)
{
   for (int i = 0; i < 4; i++)
      notEqual__const_bool__const_float__const_float(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bool__const_int__const_int(const_bool* result, const_int* x, const_int* y)
{
   if (*x != *y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void notEqual__const_bvec2__const_ivec2__const_ivec2(const_bvec2* result, const_ivec2* x, const_ivec2* y)
{
   for (int i = 0; i < 2; i++)
      notEqual__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bvec3__const_ivec3__const_ivec3(const_bvec3* result, const_ivec3* x, const_ivec3* y)
{
   for (int i = 0; i < 3; i++)
      notEqual__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bvec4__const_ivec4__const_ivec4(const_bvec4* result, const_ivec4* x, const_ivec4* y)
{
   for (int i = 0; i < 4; i++)
      notEqual__const_bool__const_int__const_int(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bool__const_uint__const_uint(const_bool* result, const_uint* x, const_uint* y)
{
   if (*x != *y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void notEqual__const_bvec2__const_uvec2__const_uvec2(const_bvec2* result, const_uvec2* x, const_uvec2* y)
{
   for (int i = 0; i < 2; i++)
      notEqual__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bvec3__const_uvec3__const_uvec3(const_bvec3* result, const_uvec3* x, const_uvec3* y)
{
   for (int i = 0; i < 3; i++)
      notEqual__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bvec4__const_uvec4__const_uvec4(const_bvec4* result, const_uvec4* x, const_uvec4* y)
{
   for (int i = 0; i < 4; i++)
      notEqual__const_bool__const_uint__const_uint(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bool__const_bool__const_bool(const_bool* result, const_bool* x, const_bool* y)
{
   if (*x != *y)
      *result = CONST_BOOL_TRUE;
   else
      *result = CONST_BOOL_FALSE;
}
static void notEqual__const_bvec2__const_bvec2__const_bvec2(const_bvec2* result, const_bvec2* x, const_bvec2* y)
{
   for (int i = 0; i < 2; i++)
      notEqual__const_bool__const_bool__const_bool(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bvec3__const_bvec3__const_bvec3(const_bvec3* result, const_bvec3* x, const_bvec3* y)
{
   for (int i = 0; i < 3; i++)
      notEqual__const_bool__const_bool__const_bool(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void notEqual__const_bvec4__const_bvec4__const_bvec4(const_bvec4* result, const_bvec4* x, const_bvec4* y)
{
   for (int i = 0; i < 4; i++)
      notEqual__const_bool__const_bool__const_bool(&(*result)[i], &(*x)[i], &(*y)[i]);
}
static void any__const_bool__const_bvec2(const_bool* result, const_bvec2* x)
{
   *result = (*x)[0] | (*x)[1];
}
static void any__const_bool__const_bvec3(const_bool* result, const_bvec3* x)
{
   *result = (*x)[0] | (*x)[1] | (*x)[2];
}
static void any__const_bool__const_bvec4(const_bool* result, const_bvec4* x)
{
   *result = (*x)[0] | (*x)[1] | (*x)[2] | (*x)[3];
}
static void all__const_bool__const_bvec2(const_bool* result, const_bvec2* x)
{
   *result = (*x)[0] & (*x)[1];
}
static void all__const_bool__const_bvec3(const_bool* result, const_bvec3* x)
{
   *result = (*x)[0] & (*x)[1] & (*x)[2];
}
static void all__const_bool__const_bvec4(const_bool* result, const_bvec4* x)
{
   *result = (*x)[0] & (*x)[1] & (*x)[2] & (*x)[3];
}
static void not__const_bool__const_bool(const_bool* result, const_bool* x)
{
   *result = !(*x);
}
static void not__const_bvec2__const_bvec2(const_bvec2* result, const_bvec2* x)
{
   for (int i = 0; i < 2; i++)
      not__const_bool__const_bool(&(*result)[i], &(*x)[i]);
}
static void not__const_bvec3__const_bvec3(const_bvec3* result, const_bvec3* x)
{
   for (int i = 0; i < 3; i++)
      not__const_bool__const_bool(&(*result)[i], &(*x)[i]);
}
static void not__const_bvec4__const_bvec4(const_bvec4* result, const_bvec4* x)
{
   for (int i = 0; i < 4; i++)
      not__const_bool__const_bool(&(*result)[i], &(*x)[i]);
}
