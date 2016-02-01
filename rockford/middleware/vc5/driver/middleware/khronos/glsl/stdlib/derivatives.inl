// Derivative Functions
static void dFdx__const_float__const_float(const_float* result, const_float* p)
{
   *result = CONST_FLOAT_ZERO;
}

static void dFdx__const_vec2__const_vec2(const_vec2* result, const_vec2* p)
{
   (*result)[0] = CONST_FLOAT_ZERO;
   (*result)[1] = CONST_FLOAT_ZERO;
}

static void dFdx__const_vec3__const_vec3(const_vec3* result, const_vec3* p)
{
   (*result)[0] = CONST_FLOAT_ZERO;
   (*result)[1] = CONST_FLOAT_ZERO;
   (*result)[2] = CONST_FLOAT_ZERO;
}

static void dFdx__const_vec4__const_vec4(const_vec4* result, const_vec4* p)
{
   (*result)[0] = CONST_FLOAT_ZERO;
   (*result)[1] = CONST_FLOAT_ZERO;
   (*result)[2] = CONST_FLOAT_ZERO;
   (*result)[3] = CONST_FLOAT_ZERO;
}

static void dFdy__const_float__const_float(const_float* result, const_float* p)
{
   *result = CONST_FLOAT_ZERO;
}

static void dFdy__const_vec2__const_vec2(const_vec2* result, const_vec2* p)
{
   (*result)[0] = CONST_FLOAT_ZERO;
   (*result)[1] = CONST_FLOAT_ZERO;
}

static void dFdy__const_vec3__const_vec3(const_vec3* result, const_vec3* p)
{
   (*result)[0] = CONST_FLOAT_ZERO;
   (*result)[1] = CONST_FLOAT_ZERO;
   (*result)[2] = CONST_FLOAT_ZERO;
}

static void dFdy__const_vec4__const_vec4(const_vec4* result, const_vec4* p)
{
   (*result)[0] = CONST_FLOAT_ZERO;
   (*result)[1] = CONST_FLOAT_ZERO;
   (*result)[2] = CONST_FLOAT_ZERO;
   (*result)[3] = CONST_FLOAT_ZERO;
}

static void fwidth__const_float__const_float(const_float* result, const_float* p)
{
   *result = CONST_FLOAT_ZERO;
}

static void fwidth__const_vec2__const_vec2(const_vec2* result, const_vec2* p)
{
   (*result)[0] = CONST_FLOAT_ZERO;
   (*result)[1] = CONST_FLOAT_ZERO;
}

static void fwidth__const_vec3__const_vec3(const_vec3* result, const_vec3* p)
{
   (*result)[0] = CONST_FLOAT_ZERO;
   (*result)[1] = CONST_FLOAT_ZERO;
   (*result)[2] = CONST_FLOAT_ZERO;
}

static void fwidth__const_vec4__const_vec4(const_vec4* result, const_vec4* p)
{
   (*result)[0] = CONST_FLOAT_ZERO;
   (*result)[1] = CONST_FLOAT_ZERO;
   (*result)[2] = CONST_FLOAT_ZERO;
   (*result)[3] = CONST_FLOAT_ZERO;
}
