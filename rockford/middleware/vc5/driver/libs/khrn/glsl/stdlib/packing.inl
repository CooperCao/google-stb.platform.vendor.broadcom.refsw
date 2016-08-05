static void packSnorm2x16__const_highp_uint__const_vec2(const_uint* result, const_vec2* v)
{
   const_float pone = CONST_FLOAT_ONE;
   const_float mone = CONST_FLOAT_MINUS_ONE;
   const_float max  = gfx_float_to_bits(32767.0);
   const_float clamped, multiplied, rounded;

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[0], &mone, &pone);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result = const_int_from_float(rounded) & 0xFFFF;

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[1], &mone, &pone);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result |= const_int_from_float(rounded) << 0x10;
}

static void unpackSnorm2x16__const_highp_vec2__const_highp_uint(const_vec2* result, const_uint* p)
{
   const_float  pone = CONST_FLOAT_ONE;
   const_float  mone = CONST_FLOAT_MINUS_ONE;
   const_float  max  = gfx_float_to_bits(32767.0);
   const_signed lo   = *p & 0xFFFF;
   const_signed hi   = *p >> 0x10;
   const_float  flo  = const_float_from_int((lo & 0x7FFF) - (lo & 0x8000));
   const_float  fhi  = const_float_from_int((hi & 0x7FFF) - (hi & 0x8000));
   const_float clamped, divided;

   divided = op_f_div(flo, max);
   clamp__const_float__const_float__const_float__const_float(&clamped, &divided, &mone, &pone);
   (*result)[0] = clamped;

   divided = op_f_div(fhi, max);
   clamp__const_float__const_float__const_float__const_float(&clamped, &divided, &mone, &pone);
   (*result)[1] = clamped;
}

static void packUnorm2x16__const_highp_uint__const_vec2(const_uint* result, const_vec2* v)
{
   const_float one  = CONST_FLOAT_ONE;
   const_float zero = CONST_FLOAT_ZERO;
   const_float max  = gfx_float_to_bits(65535.0);
   const_float clamped, multiplied, rounded;

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[0], &zero, &one);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result = const_uint_from_float(rounded);

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[1], &zero, &one);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result |= (const_uint_from_float(rounded) << 0x10);
}

static void unpackUnorm2x16__const_highp_vec2__const_highp_uint(const_vec2* result, const_uint* p)
{
   const_float max  = gfx_float_to_bits(65535.0);
   const_float low  = const_float_from_uint(*p & 0xFFFF);
   const_float high = const_float_from_uint(*p >> 0x10);

   (*result)[0] = op_f_div(low, max);
   (*result)[1] = op_f_div(high, max);
}

static void packHalf2x16__const_highp_uint__const_mediump_vec2(const_uint* result, const_vec2* v)
{
   *result = op_fpack((*v)[0], (*v)[1]);
}

static void unpackHalf2x16__const_mediump_vec2__const_highp_uint(const_vec2* result, const_uint* p)
{
   (*result)[0] = op_funpacka(*p);
   (*result)[1] = op_funpackb(*p);
}

static void packUnorm4x8__const_highp_uint__const_mediump_vec4(const_uint *result, const_vec4 *v)
{
   const_float one  = CONST_FLOAT_ONE;
   const_float zero = CONST_FLOAT_ZERO;
   const_float max  = gfx_float_to_bits(255.0);
   const_float clamped, multiplied, rounded;

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[0], &zero, &one);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result = const_uint_from_float(rounded);

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[1], &zero, &one);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result |= (const_uint_from_float(rounded) << 8);

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[2], &zero, &one);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result |= (const_uint_from_float(rounded) << 16);

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[3], &zero, &one);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result |= (const_uint_from_float(rounded) << 24);
}

static void packSnorm4x8__const_highp_uint__const_mediump_vec4(const_uint *result, const_vec4 *v)
{
   const_float pone = CONST_FLOAT_ONE;
   const_float mone = CONST_FLOAT_MINUS_ONE;
   const_float max  = gfx_float_to_bits(127.0);
   const_float clamped, multiplied, rounded;

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[0], &mone, &pone);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result = const_int_from_float(rounded) & 0xFF;

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[1], &mone, &pone);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result |= (const_int_from_float(rounded) & 0xFF) << 8;

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[2], &mone, &pone);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result |= (const_int_from_float(rounded) & 0xFF) << 16;

   clamp__const_float__const_float__const_float__const_float(&clamped, &(*v)[3], &mone, &pone);
   multiplied = op_f_mul(clamped, max);
   round__const_float__const_float(&rounded, &multiplied);
   *result |= const_int_from_float(rounded) << 24;
}

static void unpackUnorm4x8__const_mediump_vec4__const_highp_uint(const_vec4 *result, const_uint *v)
{
   const_float max  = gfx_float_to_bits(255.0);
   const_float a = const_float_from_uint((*v      ) & 0xFF);
   const_float b = const_float_from_uint((*v >> 8 ) & 0xFF);
   const_float c = const_float_from_uint((*v >> 16) & 0xFF);
   const_float d = const_float_from_uint((*v >> 24));

   (*result)[0] = op_f_div(a, max);
   (*result)[1] = op_f_div(b, max);
   (*result)[2] = op_f_div(c, max);
   (*result)[3] = op_f_div(d, max);
}

static void unpackSnorm4x8__const_mediump_vec4__const_highp_uint(const_vec4 *result, const_uint *v)
{
   const_float  pone = CONST_FLOAT_ONE;
   const_float  mone = CONST_FLOAT_MINUS_ONE;
   const_float  max  = gfx_float_to_bits(127.0);
   const_signed a   =  *v        & 0xFF;
   const_signed b   = (*v >> 8 ) & 0xFF;
   const_signed c   = (*v >> 16) & 0xFF;
   const_signed d   = (*v >> 24) & 0xFF;
   const_float  fa  = const_float_from_int((a & 0x7F) - (a & 0x80));
   const_float  fb  = const_float_from_int((b & 0x7F) - (b & 0x80));
   const_float  fc  = const_float_from_int((c & 0x7F) - (c & 0x80));
   const_float  fd  = const_float_from_int((d & 0x7F) - (d & 0x80));
   const_float clamped, divided;

   divided = op_f_div(fa, max);
   clamp__const_float__const_float__const_float__const_float(&clamped, &divided, &mone, &pone);
   (*result)[0] = clamped;

   divided = op_f_div(fb, max);
   clamp__const_float__const_float__const_float__const_float(&clamped, &divided, &mone, &pone);
   (*result)[1] = clamped;

   divided = op_f_div(fc, max);
   clamp__const_float__const_float__const_float__const_float(&clamped, &divided, &mone, &pone);
   (*result)[2] = clamped;

   divided = op_f_div(fd, max);
   clamp__const_float__const_float__const_float__const_float(&clamped, &divided, &mone, &pone);
   (*result)[3] = clamped;
}
