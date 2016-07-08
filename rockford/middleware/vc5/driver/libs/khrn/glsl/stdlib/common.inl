static void abs__const_float__const_float(const_float* result, const_float* x)
{
   const const_value sign_bit = 0x80000000;
   *result = *x & ~sign_bit;
}
static void abs__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
	abs__const_float__const_float(&(*result)[0], &(*x)[0]);
	abs__const_float__const_float(&(*result)[1], &(*x)[1]);
}
static void abs__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
	abs__const_float__const_float(&(*result)[0], &(*x)[0]);
	abs__const_float__const_float(&(*result)[1], &(*x)[1]);
	abs__const_float__const_float(&(*result)[2], &(*x)[2]);
}
static void abs__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
	abs__const_float__const_float(&(*result)[0], &(*x)[0]);
	abs__const_float__const_float(&(*result)[1], &(*x)[1]);
	abs__const_float__const_float(&(*result)[2], &(*x)[2]);
	abs__const_float__const_float(&(*result)[3], &(*x)[3]);
}
static void abs__const_int__const_int(const_int* result, const_int* x)
{
   if (*(const_signed*)x < 0)
      *result = op_i_negate(*x);
   else
      *result = *x;
}
static void abs__const_ivec2__const_ivec2(const_ivec2* result, const_ivec2* x)
{
   abs__const_int__const_int(&(*result)[0], &(*x)[0]);
   abs__const_int__const_int(&(*result)[1], &(*x)[1]);
}
static void abs__const_ivec3__const_ivec3(const_ivec3* result, const_ivec3* x)
{
   abs__const_int__const_int(&(*result)[0], &(*x)[0]);
   abs__const_int__const_int(&(*result)[1], &(*x)[1]);
   abs__const_int__const_int(&(*result)[2], &(*x)[2]);
}
static void abs__const_ivec4__const_ivec4(const_ivec4* result, const_ivec4* x)
{
   abs__const_int__const_int(&(*result)[0], &(*x)[0]);
   abs__const_int__const_int(&(*result)[1], &(*x)[1]);
   abs__const_int__const_int(&(*result)[2], &(*x)[2]);
   abs__const_int__const_int(&(*result)[3], &(*x)[3]);
}
static void sign__const_float__const_float(const_float* result, const_float* x)
{
   const const_value sign_bit = 0x80000000;
   if((*x & ~sign_bit) == 0) {
      *result = CONST_FLOAT_ZERO;
   } else {
      *result = CONST_FLOAT_ONE | (*x & sign_bit);
   }
}
static void sign__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
	sign__const_float__const_float(&(*result)[0], &(*x)[0]);
	sign__const_float__const_float(&(*result)[1], &(*x)[1]);
}
static void sign__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
	sign__const_float__const_float(&(*result)[0], &(*x)[0]);
	sign__const_float__const_float(&(*result)[1], &(*x)[1]);
	sign__const_float__const_float(&(*result)[2], &(*x)[2]);
}
static void sign__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
	sign__const_float__const_float(&(*result)[0], &(*x)[0]);
	sign__const_float__const_float(&(*result)[1], &(*x)[1]);
	sign__const_float__const_float(&(*result)[2], &(*x)[2]);
	sign__const_float__const_float(&(*result)[3], &(*x)[3]);
}
static void sign__const_int__const_int(const_int* result, const_int* x)
{
   if(*(const_signed*)x > 0) *result = 1;
   else if(*(const_signed*)x < 0) *(const_signed*)result = -1;
   else *result = 0;
}
static void sign__const_ivec2__const_ivec2(const_ivec2* result, const_ivec2* x)
{
   sign__const_int__const_int(&(*result)[0], &(*x)[0]);
   sign__const_int__const_int(&(*result)[1], &(*x)[1]);
}
static void sign__const_ivec3__const_ivec3(const_ivec3* result, const_ivec3* x)
{
   sign__const_int__const_int(&(*result)[0], &(*x)[0]);
   sign__const_int__const_int(&(*result)[1], &(*x)[1]);
   sign__const_int__const_int(&(*result)[2], &(*x)[2]);
}
static void sign__const_ivec4__const_ivec4(const_ivec4* result, const_ivec4* x)
{
   sign__const_int__const_int(&(*result)[0], &(*x)[0]);
   sign__const_int__const_int(&(*result)[1], &(*x)[1]);
   sign__const_int__const_int(&(*result)[2], &(*x)[2]);
   sign__const_int__const_int(&(*result)[3], &(*x)[3]);
}

static void floor__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_floor(*x);
}
static void floor__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
	floor__const_float__const_float(&(*result)[0], &(*x)[0]);
	floor__const_float__const_float(&(*result)[1], &(*x)[1]);
}
static void floor__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
	floor__const_float__const_float(&(*result)[0], &(*x)[0]);
	floor__const_float__const_float(&(*result)[1], &(*x)[1]);
	floor__const_float__const_float(&(*result)[2], &(*x)[2]);
}
static void floor__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
	floor__const_float__const_float(&(*result)[0], &(*x)[0]);
	floor__const_float__const_float(&(*result)[1], &(*x)[1]);
	floor__const_float__const_float(&(*result)[2], &(*x)[2]);
	floor__const_float__const_float(&(*result)[3], &(*x)[3]);
}
static void ceil__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_ceil(*x);
}
static void ceil__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
	ceil__const_float__const_float(&(*result)[0], &(*x)[0]);
	ceil__const_float__const_float(&(*result)[1], &(*x)[1]);
}
static void ceil__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
	ceil__const_float__const_float(&(*result)[0], &(*x)[0]);
	ceil__const_float__const_float(&(*result)[1], &(*x)[1]);
	ceil__const_float__const_float(&(*result)[2], &(*x)[2]);
}
static void ceil__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
	ceil__const_float__const_float(&(*result)[0], &(*x)[0]);
	ceil__const_float__const_float(&(*result)[1], &(*x)[1]);
	ceil__const_float__const_float(&(*result)[2], &(*x)[2]);
	ceil__const_float__const_float(&(*result)[3], &(*x)[3]);
}
static void fract__const_float__const_float(const_float* result, const_float* x)
{
   const_float t1;

   t1 = op_floor(*x);
   *result = op_f_sub(*x, t1);
}
static void fract__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
	fract__const_float__const_float(&(*result)[0], &(*x)[0]);
	fract__const_float__const_float(&(*result)[1], &(*x)[1]);
}
static void fract__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
	fract__const_float__const_float(&(*result)[0], &(*x)[0]);
	fract__const_float__const_float(&(*result)[1], &(*x)[1]);
	fract__const_float__const_float(&(*result)[2], &(*x)[2]);
}
static void fract__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
	fract__const_float__const_float(&(*result)[0], &(*x)[0]);
	fract__const_float__const_float(&(*result)[1], &(*x)[1]);
	fract__const_float__const_float(&(*result)[2], &(*x)[2]);
	fract__const_float__const_float(&(*result)[3], &(*x)[3]);
}
static void mod__const_float__const_float__const_float(const_float* result, const_float* x, const_float* y)
{
   const_float r0, r1, r2, r3;
   r0 = op_recip(*y);
   r1 = op_f_mul(*x, r0);
   r2 = op_floor(r1);
   r3 = op_f_mul(*y, r2);
   *result = op_f_sub(*x, r3);
}
static void mod__const_vec2__const_vec2__const_float(const_vec2* result, const_vec2* x, const_float* y)
{
	mod__const_float__const_float__const_float(&(*result)[0], &(*x)[0], y);
	mod__const_float__const_float__const_float(&(*result)[1], &(*x)[1], y);
}
static void mod__const_vec3__const_vec3__const_float(const_vec3* result, const_vec3* x, const_float* y)
{
	mod__const_float__const_float__const_float(&(*result)[0], &(*x)[0], y);
	mod__const_float__const_float__const_float(&(*result)[1], &(*x)[1], y);
	mod__const_float__const_float__const_float(&(*result)[2], &(*x)[2], y);
}
static void mod__const_vec4__const_vec4__const_float(const_vec4* result, const_vec4* x, const_float* y)
{
	mod__const_float__const_float__const_float(&(*result)[0], &(*x)[0], y);
	mod__const_float__const_float__const_float(&(*result)[1], &(*x)[1], y);
	mod__const_float__const_float__const_float(&(*result)[2], &(*x)[2], y);
	mod__const_float__const_float__const_float(&(*result)[3], &(*x)[3], y);
}
static void mod__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* x, const_vec2* y)
{
	mod__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0]);
	mod__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1]);
}
static void mod__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* x, const_vec3* y)
{
	mod__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0]);
	mod__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1]);
	mod__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*y)[2]);
}
static void mod__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* x, const_vec4* y)
{
	mod__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0]);
	mod__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1]);
	mod__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*y)[2]);
	mod__const_float__const_float__const_float(&(*result)[3], &(*x)[3], &(*y)[3]);
}
static void min__const_float__const_float__const_float(const_float* result, const_float* x, const_float* y)
{
   *result = op_f_min(*x, *y);
}
static void min__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* x, const_vec2* y)
{
	min__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0]);
	min__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1]);
}
static void min__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* x, const_vec3* y)
{
	min__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0]);
	min__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1]);
	min__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*y)[2]);
}
static void min__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* x, const_vec4* y)
{
	min__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0]);
	min__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1]);
	min__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*y)[2]);
	min__const_float__const_float__const_float(&(*result)[3], &(*x)[3], &(*y)[3]);
}
static void min__const_vec2__const_vec2__const_float(const_vec2* result, const_vec2* x, const_float* y)
{
	min__const_float__const_float__const_float(&(*result)[0], &(*x)[0], y);
	min__const_float__const_float__const_float(&(*result)[1], &(*x)[1], y);
}
static void min__const_vec3__const_vec3__const_float(const_vec3* result, const_vec3* x, const_float* y)
{
	min__const_float__const_float__const_float(&(*result)[0], &(*x)[0], y);
	min__const_float__const_float__const_float(&(*result)[1], &(*x)[1], y);
	min__const_float__const_float__const_float(&(*result)[2], &(*x)[2], y);
}
static void min__const_vec4__const_vec4__const_float(const_vec4* result, const_vec4* x, const_float* y)
{
	min__const_float__const_float__const_float(&(*result)[0], &(*x)[0], y);
	min__const_float__const_float__const_float(&(*result)[1], &(*x)[1], y);
	min__const_float__const_float__const_float(&(*result)[2], &(*x)[2], y);
	min__const_float__const_float__const_float(&(*result)[3], &(*x)[3], y);
}
static void min__const_int__const_int__const_int(const_int* result, const_int* x, const_int* y)
{
   *result = op_i_min(*x, *y);
}
static void min__const_ivec2__const_ivec2__const_ivec2(const_ivec2* result, const_ivec2* x, const_ivec2* y)
{
   min__const_int__const_int__const_int(&(*result)[0], &(*x)[0], &(*y)[0]);
   min__const_int__const_int__const_int(&(*result)[1], &(*x)[1], &(*y)[1]);
}
static void min__const_ivec3__const_ivec3__const_ivec3(const_ivec3* result, const_ivec3* x, const_ivec3* y)
{
   min__const_int__const_int__const_int(&(*result)[0], &(*x)[0], &(*y)[0]);
   min__const_int__const_int__const_int(&(*result)[1], &(*x)[1], &(*y)[1]);
   min__const_int__const_int__const_int(&(*result)[2], &(*x)[2], &(*y)[2]);
}
static void min__const_ivec4__const_ivec4__const_ivec4(const_ivec4* result, const_ivec4* x, const_ivec4* y)
{
   min__const_int__const_int__const_int(&(*result)[0], &(*x)[0], &(*y)[0]);
   min__const_int__const_int__const_int(&(*result)[1], &(*x)[1], &(*y)[1]);
   min__const_int__const_int__const_int(&(*result)[2], &(*x)[2], &(*y)[2]);
   min__const_int__const_int__const_int(&(*result)[3], &(*x)[3], &(*y)[3]);
}
static void min__const_ivec2__const_ivec2__const_int(const_ivec2* result, const_ivec2* x, const_int* y)
{
   min__const_int__const_int__const_int(&(*result)[0], &(*x)[0], y);
   min__const_int__const_int__const_int(&(*result)[1], &(*x)[1], y);
}
static void min__const_ivec3__const_ivec3__const_int(const_ivec3* result, const_ivec3* x, const_int* y)
{
   min__const_int__const_int__const_int(&(*result)[0], &(*x)[0], y);
   min__const_int__const_int__const_int(&(*result)[1], &(*x)[1], y);
   min__const_int__const_int__const_int(&(*result)[2], &(*x)[2], y);
}
static void min__const_ivec4__const_ivec4__const_int(const_ivec4* result, const_ivec4* x, const_int* y)
{
   min__const_int__const_int__const_int(&(*result)[0], &(*x)[0], y);
   min__const_int__const_int__const_int(&(*result)[1], &(*x)[1], y);
   min__const_int__const_int__const_int(&(*result)[2], &(*x)[2], y);
   min__const_int__const_int__const_int(&(*result)[3], &(*x)[3], y);
}
static void min__const_uint__const_uint__const_uint(const_uint* result, const_uint* x, const_uint* y)
{
   *result = op_u_min(*x, *y);
}
static void min__const_uvec2__const_uvec2__const_uvec2(const_uvec2* result, const_uvec2* x, const_uvec2* y)
{
   min__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], &(*y)[0]);
   min__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], &(*y)[1]);
}
static void min__const_uvec3__const_uvec3__const_uvec3(const_uvec3* result, const_uvec3* x, const_uvec3* y)
{
   min__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], &(*y)[0]);
   min__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], &(*y)[1]);
   min__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], &(*y)[2]);
}
static void min__const_uvec4__const_uvec4__const_uvec4(const_uvec4* result, const_uvec4* x, const_uvec4* y)
{
   min__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], &(*y)[0]);
   min__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], &(*y)[1]);
   min__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], &(*y)[2]);
   min__const_uint__const_uint__const_uint(&(*result)[3], &(*x)[3], &(*y)[3]);
}
static void min__const_uvec2__const_uvec2__const_uint(const_uvec2* result, const_uvec2* x, const_uint* y)
{
   min__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], y);
   min__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], y);
}
static void min__const_uvec3__const_uvec3__const_uint(const_uvec3* result, const_uvec3* x, const_uint* y)
{
   min__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], y);
   min__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], y);
   min__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], y);
}
static void min__const_uvec4__const_uvec4__const_uint(const_uvec4* result, const_uvec4* x, const_uint* y)
{
   min__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], y);
   min__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], y);
   min__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], y);
   min__const_uint__const_uint__const_uint(&(*result)[3], &(*x)[3], y);
}
static void max__const_float__const_float__const_float(const_float* result, const_float* x, const_float* y)
{
   *result = op_f_max(*x, *y);
}
static void max__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* x, const_vec2* y)
{
	max__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0]);
	max__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1]);
}
static void max__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* x, const_vec3* y)
{
	max__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0]);
	max__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1]);
	max__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*y)[2]);
}
static void max__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* x, const_vec4* y)
{
	max__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0]);
	max__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1]);
	max__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*y)[2]);
	max__const_float__const_float__const_float(&(*result)[3], &(*x)[3], &(*y)[3]);
}
static void max__const_vec2__const_vec2__const_float(const_vec2* result, const_vec2* x, const_float* y)
{
	max__const_float__const_float__const_float(&(*result)[0], &(*x)[0], y);
	max__const_float__const_float__const_float(&(*result)[1], &(*x)[1], y);
}
static void max__const_vec3__const_vec3__const_float(const_vec3* result, const_vec3* x, const_float* y)
{
	max__const_float__const_float__const_float(&(*result)[0], &(*x)[0], y);
	max__const_float__const_float__const_float(&(*result)[1], &(*x)[1], y);
	max__const_float__const_float__const_float(&(*result)[2], &(*x)[2], y);
}
static void max__const_vec4__const_vec4__const_float(const_vec4* result, const_vec4* x, const_float* y)
{
	max__const_float__const_float__const_float(&(*result)[0], &(*x)[0], y);
	max__const_float__const_float__const_float(&(*result)[1], &(*x)[1], y);
	max__const_float__const_float__const_float(&(*result)[2], &(*x)[2], y);
	max__const_float__const_float__const_float(&(*result)[3], &(*x)[3], y);
}
static void max__const_int__const_int__const_int(const_int* result, const_int* x, const_int* y)
{
   *result = op_i_max(*x, *y);
}
static void max__const_ivec2__const_ivec2__const_ivec2(const_ivec2* result, const_ivec2* x, const_ivec2* y)
{
   max__const_int__const_int__const_int(&(*result)[0], &(*x)[0], &(*y)[0]);
   max__const_int__const_int__const_int(&(*result)[1], &(*x)[1], &(*y)[1]);
}
static void max__const_ivec3__const_ivec3__const_ivec3(const_ivec3* result, const_ivec3* x, const_ivec3* y)
{
   max__const_int__const_int__const_int(&(*result)[0], &(*x)[0], &(*y)[0]);
   max__const_int__const_int__const_int(&(*result)[1], &(*x)[1], &(*y)[1]);
   max__const_int__const_int__const_int(&(*result)[2], &(*x)[2], &(*y)[2]);
}
static void max__const_ivec4__const_ivec4__const_ivec4(const_ivec4* result, const_ivec4* x, const_ivec4* y)
{
   max__const_int__const_int__const_int(&(*result)[0], &(*x)[0], &(*y)[0]);
   max__const_int__const_int__const_int(&(*result)[1], &(*x)[1], &(*y)[1]);
   max__const_int__const_int__const_int(&(*result)[2], &(*x)[2], &(*y)[2]);
   max__const_int__const_int__const_int(&(*result)[3], &(*x)[3], &(*y)[3]);
}
static void max__const_ivec2__const_ivec2__const_int(const_ivec2* result, const_ivec2* x, const_int* y)
{
   max__const_int__const_int__const_int(&(*result)[0], &(*x)[0], y);
   max__const_int__const_int__const_int(&(*result)[1], &(*x)[1], y);
}
static void max__const_ivec3__const_ivec3__const_int(const_ivec3* result, const_ivec3* x, const_int* y)
{
   max__const_int__const_int__const_int(&(*result)[0], &(*x)[0], y);
   max__const_int__const_int__const_int(&(*result)[1], &(*x)[1], y);
   max__const_int__const_int__const_int(&(*result)[2], &(*x)[2], y);
}
static void max__const_ivec4__const_ivec4__const_int(const_ivec4* result, const_ivec4* x, const_int* y)
{
   max__const_int__const_int__const_int(&(*result)[0], &(*x)[0], y);
   max__const_int__const_int__const_int(&(*result)[1], &(*x)[1], y);
   max__const_int__const_int__const_int(&(*result)[2], &(*x)[2], y);
   max__const_int__const_int__const_int(&(*result)[3], &(*x)[3], y);
}
static void max__const_uint__const_uint__const_uint(const_uint* result, const_uint* x, const_uint* y)
{
   *result = op_u_max(*x, *y);
}
static void max__const_uvec2__const_uvec2__const_uvec2(const_uvec2* result, const_uvec2* x, const_uvec2* y)
{
   max__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], &(*y)[0]);
   max__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], &(*y)[1]);
}
static void max__const_uvec3__const_uvec3__const_uvec3(const_uvec3* result, const_uvec3* x, const_uvec3* y)
{
   max__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], &(*y)[0]);
   max__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], &(*y)[1]);
   max__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], &(*y)[2]);
}
static void max__const_uvec4__const_uvec4__const_uvec4(const_uvec4* result, const_uvec4* x, const_uvec4* y)
{
   max__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], &(*y)[0]);
   max__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], &(*y)[1]);
   max__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], &(*y)[2]);
   max__const_uint__const_uint__const_uint(&(*result)[3], &(*x)[3], &(*y)[3]);
}
static void max__const_uvec2__const_uvec2__const_uint(const_uvec2* result, const_uvec2* x, const_uint* y)
{
   max__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], y);
   max__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], y);
}
static void max__const_uvec3__const_uvec3__const_uint(const_uvec3* result, const_uvec3* x, const_uint* y)
{
   max__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], y);
   max__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], y);
   max__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], y);
}
static void max__const_uvec4__const_uvec4__const_uint(const_uvec4* result, const_uvec4* x, const_uint* y)
{
   max__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], y);
   max__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], y);
   max__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], y);
   max__const_uint__const_uint__const_uint(&(*result)[3], &(*x)[3], y);
}
static void clamp__const_float__const_float__const_float__const_float(const_float* result, const_float* x, const_float* minVal, const_float* maxVal)
{
   const_float t1;

   t1 = op_f_max(*x, *minVal);
   *result = op_f_min(t1, *maxVal);
}
static void clamp__const_vec2__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* x, const_vec2* minVal, const_vec2* maxVal)
{
	clamp__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*minVal)[0], &(*maxVal)[0]);
	clamp__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*minVal)[1], &(*maxVal)[1]);
}
static void clamp__const_vec3__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* x, const_vec3* minVal, const_vec3* maxVal)
{
	clamp__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*minVal)[0], &(*maxVal)[0]);
	clamp__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*minVal)[1], &(*maxVal)[1]);
	clamp__const_float__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*minVal)[2], &(*maxVal)[2]);
}
static void clamp__const_vec4__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* x, const_vec4* minVal, const_vec4* maxVal)
{
	clamp__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*minVal)[0], &(*maxVal)[0]);
	clamp__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*minVal)[1], &(*maxVal)[1]);
	clamp__const_float__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*minVal)[2], &(*maxVal)[2]);
	clamp__const_float__const_float__const_float__const_float(&(*result)[3], &(*x)[3], &(*minVal)[3], &(*maxVal)[3]);
}
static void clamp__const_vec2__const_vec2__const_float__const_float(const_vec2* result, const_vec2* x, const_float* minVal, const_float* maxVal)
{
	clamp__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], minVal, maxVal);
	clamp__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], minVal, maxVal);
}
static void clamp__const_vec3__const_vec3__const_float__const_float(const_vec3* result, const_vec3* x, const_float* minVal, const_float* maxVal)
{
	clamp__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], minVal, maxVal);
	clamp__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], minVal, maxVal);
	clamp__const_float__const_float__const_float__const_float(&(*result)[2], &(*x)[2], minVal, maxVal);
}
static void clamp__const_vec4__const_vec4__const_float__const_float(const_vec4* result, const_vec4* x, const_float* minVal, const_float* maxVal)
{
	clamp__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], minVal, maxVal);
	clamp__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], minVal, maxVal);
	clamp__const_float__const_float__const_float__const_float(&(*result)[2], &(*x)[2], minVal, maxVal);
	clamp__const_float__const_float__const_float__const_float(&(*result)[3], &(*x)[3], minVal, maxVal);
}
static void clamp__const_int__const_int__const_int__const_int(const_int* result, const_int* x, const_int* minVal, const_int* maxVal)
{
   const_int t1;

   t1 = op_i_max(*x, *minVal);
   *result = op_i_min(t1, *maxVal);
}
static void clamp__const_ivec2__const_ivec2__const_ivec2__const_ivec2(const_ivec2* result, const_ivec2* x, const_ivec2* minVal, const_ivec2* maxVal)
{
   clamp__const_int__const_int__const_int__const_int(&(*result)[0], &(*x)[0], &(*minVal)[0], &(*maxVal)[0]);
   clamp__const_int__const_int__const_int__const_int(&(*result)[1], &(*x)[1], &(*minVal)[1], &(*maxVal)[1]);
}
static void clamp__const_ivec3__const_ivec3__const_ivec3__const_ivec3(const_ivec3* result, const_ivec3* x, const_ivec3* minVal, const_ivec3* maxVal)
{
   clamp__const_int__const_int__const_int__const_int(&(*result)[0], &(*x)[0], &(*minVal)[0], &(*maxVal)[0]);
   clamp__const_int__const_int__const_int__const_int(&(*result)[1], &(*x)[1], &(*minVal)[1], &(*maxVal)[1]);
   clamp__const_int__const_int__const_int__const_int(&(*result)[2], &(*x)[2], &(*minVal)[2], &(*maxVal)[2]);
}
static void clamp__const_ivec4__const_ivec4__const_ivec4__const_ivec4(const_ivec4* result, const_ivec4* x, const_ivec4* minVal, const_ivec4* maxVal)
{
   clamp__const_int__const_int__const_int__const_int(&(*result)[0], &(*x)[0], &(*minVal)[0], &(*maxVal)[0]);
   clamp__const_int__const_int__const_int__const_int(&(*result)[1], &(*x)[1], &(*minVal)[1], &(*maxVal)[1]);
   clamp__const_int__const_int__const_int__const_int(&(*result)[2], &(*x)[2], &(*minVal)[2], &(*maxVal)[2]);
   clamp__const_int__const_int__const_int__const_int(&(*result)[3], &(*x)[3], &(*minVal)[3], &(*maxVal)[3]);
}
static void clamp__const_ivec2__const_ivec2__const_int__const_int(const_ivec2* result, const_ivec2* x, const_int* minVal, const_int* maxVal)
{
   clamp__const_int__const_int__const_int__const_int(&(*result)[0], &(*x)[0], minVal, maxVal);
   clamp__const_int__const_int__const_int__const_int(&(*result)[1], &(*x)[1], minVal, maxVal);
}
static void clamp__const_ivec3__const_ivec3__const_int__const_int(const_ivec3* result, const_ivec3* x, const_int* minVal, const_int* maxVal)
{
   clamp__const_int__const_int__const_int__const_int(&(*result)[0], &(*x)[0], minVal, maxVal);
   clamp__const_int__const_int__const_int__const_int(&(*result)[1], &(*x)[1], minVal, maxVal);
   clamp__const_int__const_int__const_int__const_int(&(*result)[2], &(*x)[2], minVal, maxVal);
}
static void clamp__const_ivec4__const_ivec4__const_int__const_int(const_ivec4* result, const_ivec4* x, const_int* minVal, const_int* maxVal)
{
   clamp__const_int__const_int__const_int__const_int(&(*result)[0], &(*x)[0], minVal, maxVal);
   clamp__const_int__const_int__const_int__const_int(&(*result)[1], &(*x)[1], minVal, maxVal);
   clamp__const_int__const_int__const_int__const_int(&(*result)[2], &(*x)[2], minVal, maxVal);
   clamp__const_int__const_int__const_int__const_int(&(*result)[3], &(*x)[3], minVal, maxVal);
}
static void clamp__const_uint__const_uint__const_uint__const_uint(const_uint* result, const_uint* x, const_uint* minVal, const_uint* maxVal)
{
   const_uint t1;

   t1      = op_u_max(*x, *minVal);
   *result = op_u_min(t1, *maxVal);
}
static void clamp__const_uvec2__const_uvec2__const_uvec2__const_uvec2(const_uvec2* result, const_uvec2* x, const_uvec2* minVal, const_uvec2* maxVal)
{
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], &(*minVal)[0], &(*maxVal)[0]);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], &(*minVal)[1], &(*maxVal)[1]);
}
static void clamp__const_uvec3__const_uvec3__const_uvec3__const_uvec3(const_uvec3* result, const_uvec3* x, const_uvec3* minVal, const_uvec3* maxVal)
{
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], &(*minVal)[0], &(*maxVal)[0]);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], &(*minVal)[1], &(*maxVal)[1]);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], &(*minVal)[2], &(*maxVal)[2]);
}
static void clamp__const_uvec4__const_uvec4__const_uvec4__const_uvec4(const_uvec4* result, const_uvec4* x, const_uvec4* minVal, const_uvec4* maxVal)
{
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], &(*minVal)[0], &(*maxVal)[0]);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], &(*minVal)[1], &(*maxVal)[1]);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], &(*minVal)[2], &(*maxVal)[2]);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[3], &(*x)[3], &(*minVal)[3], &(*maxVal)[3]);
}
static void clamp__const_uvec2__const_uvec2__const_uint__const_uint(const_uvec2* result, const_uvec2* x, const_uint* minVal, const_uint* maxVal)
{
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], minVal, maxVal);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], minVal, maxVal);
}
static void clamp__const_uvec3__const_uvec3__const_uint__const_uint(const_uvec3* result, const_uvec3* x, const_uint* minVal, const_uint* maxVal)
{
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], minVal, maxVal);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], minVal, maxVal);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], minVal, maxVal);
}
static void clamp__const_uvec4__const_uvec4__const_uint__const_uint(const_uvec4* result, const_uvec4* x, const_uint* minVal, const_uint* maxVal)
{
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[0], &(*x)[0], minVal, maxVal);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[1], &(*x)[1], minVal, maxVal);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[2], &(*x)[2], minVal, maxVal);
   clamp__const_uint__const_uint__const_uint__const_uint(&(*result)[3], &(*x)[3], minVal, maxVal);
}
static void mix__const_float__const_float__const_float__const_float(const_float* result, const_float* x, const_float* y, const_float* a)
{
   // return x + a * (y - x)

   const_float t1, t2;

   t1      = op_f_sub(*y, *x);
   t2      = op_f_mul(*a, t1);
   *result = op_f_add(*x, t2);
}
static void mix__const_vec2__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* x, const_vec2* y, const_vec2* a)
{
	mix__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0], &(*a)[0]);
	mix__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1], &(*a)[1]);
}
static void mix__const_vec3__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* x, const_vec3* y, const_vec3* a)
{
	mix__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0], &(*a)[0]);
	mix__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1], &(*a)[1]);
	mix__const_float__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*y)[2], &(*a)[2]);
}
static void mix__const_vec4__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* x, const_vec4* y, const_vec4* a)
{
	mix__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0], &(*a)[0]);
	mix__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1], &(*a)[1]);
	mix__const_float__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*y)[2], &(*a)[2]);
	mix__const_float__const_float__const_float__const_float(&(*result)[3], &(*x)[3], &(*y)[3], &(*a)[3]);
}
static void mix__const_vec2__const_vec2__const_vec2__const_float(const_vec2* result, const_vec2* x, const_vec2* y, const_float* a)
{
	mix__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0], a);
	mix__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1], a);
}
static void mix__const_vec3__const_vec3__const_vec3__const_float(const_vec3* result, const_vec3* x, const_vec3* y, const_float* a)
{
	mix__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0], a);
	mix__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1], a);
	mix__const_float__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*y)[2], a);
}
static void mix__const_vec4__const_vec4__const_vec4__const_float(const_vec4* result, const_vec4* x, const_vec4* y, const_float* a)
{
	mix__const_float__const_float__const_float__const_float(&(*result)[0], &(*x)[0], &(*y)[0], a);
	mix__const_float__const_float__const_float__const_float(&(*result)[1], &(*x)[1], &(*y)[1], a);
	mix__const_float__const_float__const_float__const_float(&(*result)[2], &(*x)[2], &(*y)[2], a);
	mix__const_float__const_float__const_float__const_float(&(*result)[3], &(*x)[3], &(*y)[3], a);
}
static void mix__const_float__const_float__const_float__const_bool(const_float *result, const_float *x, const_float *y, const_bool *b)
{
   if (*b) *result = *y;
   else    *result = *x;
}
static void mix__const_vec2__const_vec2__const_vec2__const_bvec2(const_vec2 *result, const_vec2 *x, const_vec2 *y, const_bvec2 *b)
{
	mix__const_float__const_float__const_float__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_float__const_float__const_float__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
}
static void mix__const_vec3__const_vec3__const_vec3__const_bvec3(const_vec3 *result, const_vec3 *x, const_vec3 *y, const_bvec3 *b)
{
	mix__const_float__const_float__const_float__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_float__const_float__const_float__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
	mix__const_float__const_float__const_float__const_bool(&(*result)[2], &(*x)[2], &(*y)[2], &(*b)[2]);
}
static void mix__const_vec4__const_vec4__const_vec4__const_bvec4(const_vec4 *result, const_vec4 *x, const_vec4 *y, const_bvec4 *b)
{
	mix__const_float__const_float__const_float__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_float__const_float__const_float__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
	mix__const_float__const_float__const_float__const_bool(&(*result)[2], &(*x)[2], &(*y)[2], &(*b)[2]);
	mix__const_float__const_float__const_float__const_bool(&(*result)[3], &(*x)[3], &(*y)[3], &(*b)[3]);
}
static void mix__const_int__const_int__const_int__const_bool(const_int *result, const_int *x, const_int *y, const_bool *b)
{
   if (*b) *result = *y;
   else    *result = *x;
}
static void mix__const_ivec2__const_ivec2__const_ivec2__const_bvec2(const_ivec2 *result, const_ivec2 *x, const_ivec2 *y, const_bvec2 *b)
{
	mix__const_int__const_int__const_int__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_int__const_int__const_int__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
}
static void mix__const_ivec3__const_ivec3__const_ivec3__const_bvec3(const_ivec3 *result, const_ivec3 *x, const_ivec3 *y, const_bvec3 *b)
{
	mix__const_int__const_int__const_int__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_int__const_int__const_int__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
	mix__const_int__const_int__const_int__const_bool(&(*result)[2], &(*x)[2], &(*y)[2], &(*b)[2]);
}
static void mix__const_ivec4__const_ivec4__const_ivec4__const_bvec4(const_ivec4 *result, const_ivec4 *x, const_ivec4 *y, const_bvec4 *b)
{
	mix__const_int__const_int__const_int__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_int__const_int__const_int__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
	mix__const_int__const_int__const_int__const_bool(&(*result)[2], &(*x)[2], &(*y)[2], &(*b)[2]);
	mix__const_int__const_int__const_int__const_bool(&(*result)[3], &(*x)[3], &(*y)[3], &(*b)[3]);
}
static void mix__const_uint__const_uint__const_uint__const_bool(const_uint *result, const_uint *x, const_uint *y, const_bool *b)
{
   if (*b) *result = *y;
   else    *result = *x;
}
static void mix__const_uvec2__const_uvec2__const_uvec2__const_bvec2(const_uvec2 *result, const_uvec2 *x, const_uvec2 *y, const_bvec2 *b)
{
	mix__const_uint__const_uint__const_uint__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_uint__const_uint__const_uint__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
}
static void mix__const_uvec3__const_uvec3__const_uvec3__const_bvec3(const_uvec3 *result, const_uvec3 *x, const_uvec3 *y, const_bvec3 *b)
{
	mix__const_uint__const_uint__const_uint__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_uint__const_uint__const_uint__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
	mix__const_uint__const_uint__const_uint__const_bool(&(*result)[2], &(*x)[2], &(*y)[2], &(*b)[2]);
}
static void mix__const_uvec4__const_uvec4__const_uvec4__const_bvec4(const_uvec4 *result, const_uvec4 *x, const_uvec4 *y, const_bvec4 *b)
{
	mix__const_uint__const_uint__const_uint__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_uint__const_uint__const_uint__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
	mix__const_uint__const_uint__const_uint__const_bool(&(*result)[2], &(*x)[2], &(*y)[2], &(*b)[2]);
	mix__const_uint__const_uint__const_uint__const_bool(&(*result)[3], &(*x)[3], &(*y)[3], &(*b)[3]);
}
static void mix__const_bool__const_bool__const_bool__const_bool(const_bool *result, const_bool *x, const_bool *y, const_bool *b)
{
   if (*b) *result = *y;
   else    *result = *x;
}
static void mix__const_bvec2__const_bvec2__const_bvec2__const_bvec2(const_bvec2 *result, const_bvec2 *x, const_bvec2 *y, const_bvec2 *b)
{
	mix__const_bool__const_bool__const_bool__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_bool__const_bool__const_bool__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
}
static void mix__const_bvec3__const_bvec3__const_bvec3__const_bvec3(const_bvec3 *result, const_bvec3 *x, const_bvec3 *y, const_bvec3 *b)
{
	mix__const_bool__const_bool__const_bool__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_bool__const_bool__const_bool__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
	mix__const_bool__const_bool__const_bool__const_bool(&(*result)[2], &(*x)[2], &(*y)[2], &(*b)[2]);
}
static void mix__const_bvec4__const_bvec4__const_bvec4__const_bvec4(const_bvec4 *result, const_bvec4 *x, const_bvec4 *y, const_bvec4 *b)
{
	mix__const_bool__const_bool__const_bool__const_bool(&(*result)[0], &(*x)[0], &(*y)[0], &(*b)[0]);
	mix__const_bool__const_bool__const_bool__const_bool(&(*result)[1], &(*x)[1], &(*y)[1], &(*b)[1]);
	mix__const_bool__const_bool__const_bool__const_bool(&(*result)[2], &(*x)[2], &(*y)[2], &(*b)[2]);
	mix__const_bool__const_bool__const_bool__const_bool(&(*result)[3], &(*x)[3], &(*y)[3], &(*b)[3]);
}
static void step__const_float__const_float__const_float(const_float* result, const_float* edge, const_float* x)
{
   if (op_f_less_than(*x, *edge))
      *result = CONST_FLOAT_ZERO;
   else
      *result = CONST_FLOAT_ONE;
}
static void step__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* edge, const_vec2* x)
{
	step__const_float__const_float__const_float(&(*result)[0], &(*edge)[0], &(*x)[0]);
	step__const_float__const_float__const_float(&(*result)[1], &(*edge)[1], &(*x)[1]);
}
static void step__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* edge, const_vec3* x)
{
	step__const_float__const_float__const_float(&(*result)[0], &(*edge)[0], &(*x)[0]);
	step__const_float__const_float__const_float(&(*result)[1], &(*edge)[1], &(*x)[1]);
	step__const_float__const_float__const_float(&(*result)[2], &(*edge)[2], &(*x)[2]);
}
static void step__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* edge, const_vec4* x)
{
	step__const_float__const_float__const_float(&(*result)[0], &(*edge)[0], &(*x)[0]);
	step__const_float__const_float__const_float(&(*result)[1], &(*edge)[1], &(*x)[1]);
	step__const_float__const_float__const_float(&(*result)[2], &(*edge)[2], &(*x)[2]);
	step__const_float__const_float__const_float(&(*result)[3], &(*edge)[3], &(*x)[3]);
}
static void step__const_vec2__const_float__const_vec2(const_vec2* result, const_float* edge, const_vec2* x)
{
	step__const_float__const_float__const_float(&(*result)[0], edge, &(*x)[0]);
	step__const_float__const_float__const_float(&(*result)[1], edge, &(*x)[1]);
}
static void step__const_vec3__const_float__const_vec3(const_vec3* result, const_float* edge, const_vec3* x)
{
	step__const_float__const_float__const_float(&(*result)[0], edge, &(*x)[0]);
	step__const_float__const_float__const_float(&(*result)[1], edge, &(*x)[1]);
	step__const_float__const_float__const_float(&(*result)[2], edge, &(*x)[2]);
}
static void step__const_vec4__const_float__const_vec4(const_vec4* result, const_float* edge, const_vec4* x)
{
	step__const_float__const_float__const_float(&(*result)[0], edge, &(*x)[0]);
	step__const_float__const_float__const_float(&(*result)[1], edge, &(*x)[1]);
	step__const_float__const_float__const_float(&(*result)[2], edge, &(*x)[2]);
	step__const_float__const_float__const_float(&(*result)[3], edge, &(*x)[3]);
}
static void smoothstep__const_float__const_float__const_float__const_float(const_float* result, const_float* edge0, const_float* edge1, const_float* x)
{
   /*
      float t;
      t = clamp ((x - edge0) / (edge1 - edge0), 0.0, 1.0);
      return t * t * (3.0 - 2.0 * t);
   */

   const_float zero = CONST_FLOAT_ZERO;
   const_float one = CONST_FLOAT_ONE;
   const_float two = const_float_from_int(2);
   const_float three = const_float_from_int(3);

   const_float t1, t2, t3, t4, t5, t6, t7, t8;

   t1 = op_f_sub(*x, *edge0);
   t2 = op_f_sub(*edge1, *edge0);
   t3 = op_recip(t2);
   t4 = op_f_mul(t1, t3);

   clamp__const_float__const_float__const_float__const_float(&t5, &t4, &zero, &one);

   t6 = op_f_mul(two, t5);
   t7 = op_f_sub(three, t6);
   t8 = op_f_mul(t5, t5);
   *result = op_f_mul(t8, t7);
}
static void smoothstep__const_vec2__const_vec2__const_vec2__const_vec2(const_vec2* result, const_vec2* edge0, const_vec2* edge1, const_vec2* x)
{
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[0], &(*edge0)[0], &(*edge1)[0], &(*x)[0]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[1], &(*edge0)[1], &(*edge1)[1], &(*x)[1]);
}
static void smoothstep__const_vec3__const_vec3__const_vec3__const_vec3(const_vec3* result, const_vec3* edge0, const_vec3* edge1, const_vec3* x)
{
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[0], &(*edge0)[0], &(*edge1)[0], &(*x)[0]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[1], &(*edge0)[1], &(*edge1)[1], &(*x)[1]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[2], &(*edge0)[2], &(*edge1)[2], &(*x)[2]);
}
static void smoothstep__const_vec4__const_vec4__const_vec4__const_vec4(const_vec4* result, const_vec4* edge0, const_vec4* edge1, const_vec4* x)
{
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[0], &(*edge0)[0], &(*edge1)[0], &(*x)[0]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[1], &(*edge0)[1], &(*edge1)[1], &(*x)[1]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[2], &(*edge0)[2], &(*edge1)[2], &(*x)[2]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[3], &(*edge0)[3], &(*edge1)[3], &(*x)[3]);
}
static void smoothstep__const_vec2__const_float__const_float__const_vec2(const_vec2* result, const_float* edge0, const_float* edge1, const_vec2* x)
{
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[0], edge0, edge1, &(*x)[0]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[1], edge0, edge1, &(*x)[1]);
}
static void smoothstep__const_vec3__const_float__const_float__const_vec3(const_vec3* result, const_float* edge0, const_float* edge1, const_vec3* x)
{
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[0], edge0, edge1, &(*x)[0]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[1], edge0, edge1, &(*x)[1]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[2], edge0, edge1, &(*x)[2]);
}
static void smoothstep__const_vec4__const_float__const_float__const_vec4(const_vec4* result, const_float* edge0, const_float* edge1, const_vec4* x)
{
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[0], edge0, edge1, &(*x)[0]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[1], edge0, edge1, &(*x)[1]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[2], edge0, edge1, &(*x)[2]);
	smoothstep__const_float__const_float__const_float__const_float(&(*result)[3], edge0, edge1, &(*x)[3]);
}

static void trunc__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_trunc(*(unsigned int *)x);
}

static void trunc__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   trunc__const_float__const_float(&(*result)[0], &(*x)[0]);
   trunc__const_float__const_float(&(*result)[1], &(*x)[1]);
}

static void trunc__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   trunc__const_float__const_float(&(*result)[0], &(*x)[0]);
   trunc__const_float__const_float(&(*result)[1], &(*x)[1]);
   trunc__const_float__const_float(&(*result)[2], &(*x)[2]);
}

static void trunc__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   trunc__const_float__const_float(&(*result)[0], &(*x)[0]);
   trunc__const_float__const_float(&(*result)[1], &(*x)[1]);
   trunc__const_float__const_float(&(*result)[2], &(*x)[2]);
   trunc__const_float__const_float(&(*result)[3], &(*x)[3]);
}

static void round__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_round(*(unsigned int *)x);
}

static void round__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   round__const_float__const_float(&(*result)[0], &(*x)[0]);
   round__const_float__const_float(&(*result)[1], &(*x)[1]);
}

static void round__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   round__const_float__const_float(&(*result)[0], &(*x)[0]);
   round__const_float__const_float(&(*result)[1], &(*x)[1]);
   round__const_float__const_float(&(*result)[2], &(*x)[2]);
}

static void round__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   round__const_float__const_float(&(*result)[0], &(*x)[0]);
   round__const_float__const_float(&(*result)[1], &(*x)[1]);
   round__const_float__const_float(&(*result)[2], &(*x)[2]);
   round__const_float__const_float(&(*result)[3], &(*x)[3]);
}

static void roundEven__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_round(*(unsigned int *)x);
}

static void roundEven__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
   roundEven__const_float__const_float(&(*result)[0], &(*x)[0]);
   roundEven__const_float__const_float(&(*result)[1], &(*x)[1]);
}

static void roundEven__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
   roundEven__const_float__const_float(&(*result)[0], &(*x)[0]);
   roundEven__const_float__const_float(&(*result)[1], &(*x)[1]);
   roundEven__const_float__const_float(&(*result)[2], &(*x)[2]);
}

static void roundEven__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
   roundEven__const_float__const_float(&(*result)[0], &(*x)[0]);
   roundEven__const_float__const_float(&(*result)[1], &(*x)[1]);
   roundEven__const_float__const_float(&(*result)[2], &(*x)[2]);
   roundEven__const_float__const_float(&(*result)[3], &(*x)[3]);
}

static void modf__const_float__const_float__const_out_float(const_float* result, const_float* x, const_float* i)
{
   trunc__const_float__const_float(i, x);
   *result = op_f_sub(*x, *i);
}

static void modf__const_vec2__const_vec2__const_out_vec2(const_vec2* result, const_vec2* x, const_vec2* i)
{
   modf__const_float__const_float__const_out_float(&(*result)[0], &(*x)[0], &(*i)[0]);
   modf__const_float__const_float__const_out_float(&(*result)[1], &(*x)[1], &(*i)[1]);
}

static void modf__const_vec3__const_vec3__const_out_vec3(const_vec3* result, const_vec3* x, const_vec3* i)
{
   modf__const_float__const_float__const_out_float(&(*result)[0], &(*x)[0], &(*i)[0]);
   modf__const_float__const_float__const_out_float(&(*result)[1], &(*x)[1], &(*i)[1]);
   modf__const_float__const_float__const_out_float(&(*result)[2], &(*x)[2], &(*i)[2]);
}

static void modf__const_vec4__const_vec4__const_out_vec4(const_vec4* result, const_vec4* x, const_vec4* i)
{
   modf__const_float__const_float__const_out_float(&(*result)[0], &(*x)[0], &(*i)[0]);
   modf__const_float__const_float__const_out_float(&(*result)[1], &(*x)[1], &(*i)[1]);
   modf__const_float__const_float__const_out_float(&(*result)[2], &(*x)[2], &(*i)[2]);
   modf__const_float__const_float__const_out_float(&(*result)[3], &(*x)[3], &(*i)[3]);
}

static void isinf__const_bool__const_float(const_bool* result, const_float* x)
{
   *result = gfx_is_inf_bits(*x) ? 1 : 0;
}

static void isinf__const_bvec2__const_vec2(const_bvec2* result, const_vec2* x)
{
   isinf__const_bool__const_float(&(*result)[0], &(*x)[0]);
   isinf__const_bool__const_float(&(*result)[1], &(*x)[1]);
}

static void isinf__const_bvec3__const_vec3(const_bvec3* result, const_vec3* x)
{
   isinf__const_bool__const_float(&(*result)[0], &(*x)[0]);
   isinf__const_bool__const_float(&(*result)[1], &(*x)[1]);
   isinf__const_bool__const_float(&(*result)[2], &(*x)[2]);
}

static void isinf__const_bvec4__const_vec4(const_bvec4* result, const_vec4* x)
{
   isinf__const_bool__const_float(&(*result)[0], &(*x)[0]);
   isinf__const_bool__const_float(&(*result)[1], &(*x)[1]);
   isinf__const_bool__const_float(&(*result)[2], &(*x)[2]);
   isinf__const_bool__const_float(&(*result)[3], &(*x)[3]);
}

static void isnan__const_bool__const_float(const_bool* result, const_float* x)
{
   *result = gfx_is_nan_bits(*x) ? 1 : 0;
}

static void isnan__const_bvec2__const_vec2(const_bvec2* result, const_vec2* x)
{
   isnan__const_bool__const_float(&(*result)[0], &(*x)[0]);
   isnan__const_bool__const_float(&(*result)[1], &(*x)[1]);
}

static void isnan__const_bvec3__const_vec3(const_bvec3* result, const_vec3* x)
{
   isnan__const_bool__const_float(&(*result)[0], &(*x)[0]);
   isnan__const_bool__const_float(&(*result)[1], &(*x)[1]);
   isnan__const_bool__const_float(&(*result)[2], &(*x)[2]);
}

static void isnan__const_bvec4__const_vec4(const_bvec4* result, const_vec4* x)
{
   isnan__const_bool__const_float(&(*result)[0], &(*x)[0]);
   isnan__const_bool__const_float(&(*result)[1], &(*x)[1]);
   isnan__const_bool__const_float(&(*result)[2], &(*x)[2]);
   isnan__const_bool__const_float(&(*result)[3], &(*x)[3]);
}

static void floatBitsToInt__const_int__const_float(const_int* result, const_float* x)
{
   *result = *x;
}

static void floatBitsToInt__const_ivec2__const_vec2(const_ivec2* result, const_vec2* x)
{
   floatBitsToInt__const_int__const_float(&(*result)[0], &(*x)[0]);
   floatBitsToInt__const_int__const_float(&(*result)[1], &(*x)[1]);
}

static void floatBitsToInt__const_ivec3__const_vec3(const_ivec3* result, const_vec3* x)
{
   floatBitsToInt__const_int__const_float(&(*result)[0], &(*x)[0]);
   floatBitsToInt__const_int__const_float(&(*result)[1], &(*x)[1]);
   floatBitsToInt__const_int__const_float(&(*result)[2], &(*x)[2]);

}
static void floatBitsToInt__const_ivec4__const_vec4(const_ivec4* result, const_vec4* x)
{
   floatBitsToInt__const_int__const_float(&(*result)[0], &(*x)[0]);
   floatBitsToInt__const_int__const_float(&(*result)[1], &(*x)[1]);
   floatBitsToInt__const_int__const_float(&(*result)[2], &(*x)[2]);
   floatBitsToInt__const_int__const_float(&(*result)[3], &(*x)[3]);
}

static void intBitsToFloat__const_float__const_int(const_float* result, const_int* x)
{
   *result = *x;
}

static void intBitsToFloat__const_vec2__const_ivec2(const_vec2* result, const_ivec2* x)
{
   intBitsToFloat__const_float__const_int(&(*result)[0], &(*x)[0]);
   intBitsToFloat__const_float__const_int(&(*result)[1], &(*x)[1]);
}

static void intBitsToFloat__const_vec3__const_ivec3(const_vec3* result, const_ivec3* x)
{
   intBitsToFloat__const_float__const_int(&(*result)[0], &(*x)[0]);
   intBitsToFloat__const_float__const_int(&(*result)[1], &(*x)[1]);
   intBitsToFloat__const_float__const_int(&(*result)[2], &(*x)[2]);
}

static void intBitsToFloat__const_vec4__const_ivec4(const_vec4* result, const_ivec4* x)
{
   intBitsToFloat__const_float__const_int(&(*result)[0], &(*x)[0]);
   intBitsToFloat__const_float__const_int(&(*result)[1], &(*x)[1]);
   intBitsToFloat__const_float__const_int(&(*result)[2], &(*x)[2]);
   intBitsToFloat__const_float__const_int(&(*result)[3], &(*x)[3]);
}

static void floatBitsToUint__const_uint__const_float(const_uint* result, const_float* x)
{
   *result = *x;
}

static void floatBitsToUint__const_uvec2__const_vec2(const_uvec2* result, const_vec2* x)
{
   floatBitsToUint__const_uint__const_float(&(*result)[0], &(*x)[0]);
   floatBitsToUint__const_uint__const_float(&(*result)[1], &(*x)[1]);
}

static void floatBitsToUint__const_uvec3__const_vec3(const_uvec3* result, const_vec3* x)
{
   floatBitsToUint__const_uint__const_float(&(*result)[0], &(*x)[0]);
   floatBitsToUint__const_uint__const_float(&(*result)[1], &(*x)[1]);
   floatBitsToUint__const_uint__const_float(&(*result)[2], &(*x)[2]);

}
static void floatBitsToUint__const_uvec4__const_vec4(const_uvec4* result, const_vec4* x)
{
   floatBitsToUint__const_uint__const_float(&(*result)[0], &(*x)[0]);
   floatBitsToUint__const_uint__const_float(&(*result)[1], &(*x)[1]);
   floatBitsToUint__const_uint__const_float(&(*result)[2], &(*x)[2]);
   floatBitsToUint__const_uint__const_float(&(*result)[3], &(*x)[3]);
}

static void uintBitsToFloat__const_float__const_uint(const_float* result, const_uint* x)
{
   *result = *x;
}

static void uintBitsToFloat__const_vec2__const_uvec2(const_vec2* result, const_uvec2* x)
{
   uintBitsToFloat__const_float__const_uint(&(*result)[0], &(*x)[0]);
   uintBitsToFloat__const_float__const_uint(&(*result)[1], &(*x)[1]);
}

static void uintBitsToFloat__const_vec3__const_uvec3(const_vec3* result, const_uvec3* x)
{
   uintBitsToFloat__const_float__const_uint(&(*result)[0], &(*x)[0]);
   uintBitsToFloat__const_float__const_uint(&(*result)[1], &(*x)[1]);
   uintBitsToFloat__const_float__const_uint(&(*result)[2], &(*x)[2]);
}

static void uintBitsToFloat__const_vec4__const_uvec4(const_vec4* result, const_uvec4* x)
{
   uintBitsToFloat__const_float__const_uint(&(*result)[0], &(*x)[0]);
   uintBitsToFloat__const_float__const_uint(&(*result)[1], &(*x)[1]);
   uintBitsToFloat__const_float__const_uint(&(*result)[2], &(*x)[2]);
   uintBitsToFloat__const_float__const_uint(&(*result)[3], &(*x)[3]);
}

static void sqrt__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_sqrt(*x);
}
static void sqrt__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
	sqrt__const_float__const_float(&(*result)[0], &(*x)[0]);
	sqrt__const_float__const_float(&(*result)[1], &(*x)[1]);
}
static void sqrt__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
	sqrt__const_float__const_float(&(*result)[0], &(*x)[0]);
	sqrt__const_float__const_float(&(*result)[1], &(*x)[1]);
	sqrt__const_float__const_float(&(*result)[2], &(*x)[2]);
}
static void sqrt__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
	sqrt__const_float__const_float(&(*result)[0], &(*x)[0]);
	sqrt__const_float__const_float(&(*result)[1], &(*x)[1]);
	sqrt__const_float__const_float(&(*result)[2], &(*x)[2]);
	sqrt__const_float__const_float(&(*result)[3], &(*x)[3]);
}
static void inversesqrt__const_float__const_float(const_float* result, const_float* x)
{
   *result = op_rsqrt(*x);
}
static void inversesqrt__const_vec2__const_vec2(const_vec2* result, const_vec2* x)
{
	inversesqrt__const_float__const_float(&(*result)[0], &(*x)[0]);
	inversesqrt__const_float__const_float(&(*result)[1], &(*x)[1]);
}
static void inversesqrt__const_vec3__const_vec3(const_vec3* result, const_vec3* x)
{
	inversesqrt__const_float__const_float(&(*result)[0], &(*x)[0]);
	inversesqrt__const_float__const_float(&(*result)[1], &(*x)[1]);
	inversesqrt__const_float__const_float(&(*result)[2], &(*x)[2]);
}
static void inversesqrt__const_vec4__const_vec4(const_vec4* result, const_vec4* x)
{
	inversesqrt__const_float__const_float(&(*result)[0], &(*x)[0]);
	inversesqrt__const_float__const_float(&(*result)[1], &(*x)[1]);
	inversesqrt__const_float__const_float(&(*result)[2], &(*x)[2]);
	inversesqrt__const_float__const_float(&(*result)[3], &(*x)[3]);
}

static void ldexp__const_highp_float__const_highp_float__const_highp_int(const_float *result, const_float *x, const_int *e)
{
   const_float x_i = *x;
   const_int e_i = *e;
   if (const_signed_from_value(e_i) >= 128) {
      x_i = op_f_mul(x_i, 0x7F000000);
      e_i = op_i_sub(e_i, 127);
   }

   const_value e_p = op_bitwise_shl(op_i_max(op_i_add(e_i, 127), 0), 23);
   *result = op_f_mul(x_i, e_p);
}

static void ldexp__const_highp_vec2__const_highp_vec2__const_highp_ivec2(const_vec2 *result, const_vec2 *x, const_ivec2 *e)
{
   for (int i=0; i<2; i++)
      ldexp__const_highp_float__const_highp_float__const_highp_int(&(*result)[i], &(*x)[i], &(*e)[i]);
}

static void ldexp__const_highp_vec3__const_highp_vec3__const_highp_ivec3(const_vec3 *result, const_vec3 *x, const_ivec3 *e)
{
   for (int i=0; i<3; i++)
      ldexp__const_highp_float__const_highp_float__const_highp_int(&(*result)[i], &(*x)[i], &(*e)[i]);
}

static void ldexp__const_highp_vec4__const_highp_vec4__const_highp_ivec4(const_vec4 *result, const_vec4 *x, const_ivec4 *e)
{
   for (int i=0; i<4; i++)
      ldexp__const_highp_float__const_highp_float__const_highp_int(&(*result)[i], &(*x)[i], &(*e)[i]);
}

static void fma__const_float__const_float__const_float__const_float(const_float *result, const_float *a, const_float *b, const_float *c) {
      *result = op_f_add(op_f_mul(*a, *b), *c);
}

static void fma__const_vec2__const_vec2__const_vec2__const_vec2(const_vec2 *result, const_vec2 *a, const_vec2 *b, const_vec2 *c) {
   for (int i=0; i<2; i++)
      (*result)[i] = op_f_add(op_f_mul((*a)[i], (*b)[i]), (*c)[i]);
}

static void fma__const_vec3__const_vec3__const_vec3__const_vec3(const_vec3 *result, const_vec3 *a, const_vec3 *b, const_vec3 *c) {
   for (int i=0; i<3; i++)
      (*result)[i] = op_f_add(op_f_mul((*a)[i], (*b)[i]), (*c)[i]);
}

static void fma__const_vec4__const_vec4__const_vec4__const_vec4(const_vec4 *result, const_vec4 *a, const_vec4 *b, const_vec4 *c) {
   for (int i=0; i<4; i++)
      (*result)[i] = op_f_add(op_f_mul((*a)[i], (*b)[i]), (*c)[i]);
}
