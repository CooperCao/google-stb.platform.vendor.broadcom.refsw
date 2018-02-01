/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "dflib.h"
#include "glsl_dataflow.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

#define UNPACK_BYTE_MEDP   0                         // Enable calculation at mediump. Also need to update vitodenf to 10-bit.
#define UNPACK_BYTE_SHIFT  (UNPACK_BYTE_MEDP ? 2u : 0u)

static inline void df_swap(Dataflow **a, Dataflow **b) {
   Dataflow *t = *a;
   *a = *b;
   *b = t;
}

static inline Dataflow *df_reinterp_int(Dataflow *a) {
   return glsl_dataflow_construct_reinterp(a, DF_INT);
}

static inline Dataflow *df_reinterp_uint(Dataflow *a) {
   return glsl_dataflow_construct_reinterp(a, DF_UINT);
}

static inline Dataflow *df_int(unsigned a) {
   return glsl_dataflow_construct_const_int(a);
}

static inline Dataflow *df_uint(unsigned a) {
   return glsl_dataflow_construct_const_uint(a);
}

static inline Dataflow *df_float(float a) {
   return glsl_dataflow_construct_const_float(a);
}

static inline Dataflow *df_min(Dataflow *a, Dataflow *b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_MIN, a, b);
}

static inline Dataflow *df_max(Dataflow *a, Dataflow *b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_MAX, a, b);
}

static inline Dataflow *df_clamp(Dataflow *x, Dataflow *min_val, Dataflow *max_val) {
   return df_min(df_max(x, min_val), max_val);
}

static inline Dataflow *df_add(Dataflow *a, Dataflow *b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, a, b);
}

static inline Dataflow *df_mul(Dataflow *a, Dataflow *b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_MUL, a, b);
}

static inline Dataflow *df_fpack(Dataflow *a, Dataflow *b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_FPACK, a, b);
}

static inline Dataflow *df_shl(Dataflow *a, Dataflow *b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_SHL, a, b);
}

static inline Dataflow *df_shr(Dataflow *a, Dataflow *b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_SHR, a, b);
}

static inline Dataflow *df_vfmul(Dataflow *a, Dataflow *b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_VFMUL, a, b);
}

static inline Dataflow *df_vfmin(Dataflow *a, Dataflow *b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_VFMIN, a, b);
}

static inline Dataflow *df_vfmax(Dataflow *a, Dataflow *b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_VFMAX, a, b);
}

static inline Dataflow *df_vfsat(Dataflow *a) {
   return df_vfmin(df_vfmax(a, df_uint(0)), df_uint(0x3c003c00));
}

static inline Dataflow *df_vfsatn(Dataflow *a) {
   return df_vfmin(df_vfmax(a, df_uint(0xbc00bc00)), df_uint(0x3c003c00));
}

static inline Dataflow *df_shl_const(Dataflow *a, unsigned b) {
   return df_shl(a, df_uint(b));
}

static inline Dataflow *df_shr_const(Dataflow *a, unsigned b) {
   return df_shr(a, df_uint(b));
}

static inline Dataflow *df_and_const(Dataflow *a, unsigned b) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_BITWISE_AND, a, df_uint(b));
}

static inline Dataflow *df_combineu(Dataflow *a, Dataflow *b, unsigned bshift) {
   /* Since we know the channels are disjoint we can use 'add' instead of 'or' */
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, a, df_shl_const(b, bshift));
}

static inline Dataflow *df_combines(Dataflow *a, Dataflow *b, unsigned bshift) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, df_and_const(a, gfx_mask(bshift)), df_shl_const(b, bshift));
}

// Unused bits are zero.
static Dataflow *packu(Dataflow *v[4], unsigned n, bool norm, unsigned num_bits)
{
   // Pack into 32 bits. The last channel may be smaller, but must be 1 bit left.
   assert(num_bits <= 23);
   assert((n-1) * num_bits < 32);

   // Since we know the channels are disjoint we can use 'add' instead of 'or'.
   Dataflow *r = NULL;
   for (unsigned i = 0; i != n; ++i) {
      Dataflow *x = v[i];
      if (norm) {
         x = df_clamp(x, df_float(0.0f), df_float(1.0f));
         x = df_mul(x, df_float(gfx_float_from_bits(gfx_mask(num_bits))));
         x = df_reinterp_uint(x);
      }
      else {
         x = df_min(x, df_uint(gfx_mask(num_bits)));
      }

      r = !i ? x : df_combineu(r, x, i*num_bits);
   }
   return r;
}

// Unused bits are undefined.
static Dataflow *packs(Dataflow *v[4], unsigned n, bool norm, unsigned num_bits)
{
   // Pack into 32 bits. The last channel may be smaller, but must be 1 bit left.
   assert((n-1) * num_bits < 32);

   // Since we know the channels are disjoint we can use 'add' instead of 'or'.
   Dataflow *r = NULL;
   for (unsigned i = 0; i != n; ++i) {
      Dataflow *x = v[i];
      if (norm) {
         x = df_clamp(x, df_float(-1.0f), df_float(1.0f));
         x = df_mul(x, df_float((float)gfx_mask(num_bits - 1)));
         x = glsl_dataflow_construct_unary_op(DATAFLOW_FTOI_NEAREST, x);
      }
      else {
         x = df_clamp(x, df_int(~gfx_mask(num_bits-1)), df_int(gfx_mask(num_bits-1)));
      }
      x = df_reinterp_uint(x);

      // Mask off any upper sign bits of the result when combining a new channel. Any unused bits will be undefined.
      r = !i ? x : df_combines(r, x, i*num_bits);
   }
   return r;
}

static Dataflow *pack_unorm10(Dataflow *v[4]) {

   // [31:16] = x, [15:0] = z
   Dataflow *xz = df_fpack(v[2], v[0]);
   xz = df_vfmul(df_vfsat(xz), df_uint(0x03ff03ff));
   xz = df_add(df_shl_const(xz, 20), df_shr_const(xz, 16));

   // [31:16] = b, [15:0] = a
   Dataflow *yw = df_fpack(v[3], v[1]);
   yw = df_vfmul(df_vfsat(yw), df_uint(0x03ff0003));
   yw = df_add(df_shl_const(yw, 30), df_shr_const(yw, 6));

   return df_add(xz, yw);
}

static Dataflow *pack_ufloat11(Dataflow *v[3]) {

   Dataflow *zero = df_float(0.0f);
   Dataflow *r = df_shr_const(df_fpack(df_max(v[0], zero), zero), 4);
   Dataflow *yz = df_vfmax(df_fpack(v[1], v[2]), df_uint(0));
   r = df_combineu(r, df_and_const(yz, 0x00007ff0), 11 - 4);
   r = df_combineu(r, df_and_const(yz, 0x7fe00000), 22 - 16 - 5);
   return r;
}

static Dataflow *pack_unorm8_xz(Dataflow *x, Dataflow *z) {
   return df_vfmul(df_vfsat(df_fpack(x, z)), df_uint(0x00FF00FF));
}

static Dataflow *pack_snorm8_xz(Dataflow *x, Dataflow *z) {
   return glsl_dataflow_construct_binary_op(DATAFLOW_VFMULDENFTOI, df_vfsatn(df_fpack(x, z)), df_uint(0x007F007F));
}

static Dataflow *unpack_unorm(Dataflow *p, unsigned num_bits) {
   Dataflow *r;
   r = glsl_dataflow_construct_unary_op(DATAFLOW_UTOF, p);
   r = df_mul(r, df_float(1.0f / (float)gfx_mask(num_bits)));
   return r;
}

static Dataflow *unpack_snorm(Dataflow *p, unsigned num_bits) {
   Dataflow *r;
   r = glsl_dataflow_construct_unary_op(DATAFLOW_ITOF, df_reinterp_int(p));
   r = df_max(r, df_float((-(float)gfx_mask(num_bits - 1))));
   r = df_mul(r, df_float(1.0f / (float)gfx_mask(num_bits - 1)));
   return r;
}

static void unpack_unorm8_xz(Dataflow *r[2], Dataflow *p) {

   const unsigned mask = 0x00ff00ffu << UNPACK_BYTE_SHIFT;
   const float scale = (float)(1u << 24u >> UNPACK_BYTE_SHIFT) / 255.0f;

   p = df_and_const(p, mask);
   if (UNPACK_BYTE_MEDP) {
      const uint32_t f16_scale = gfx_float_to_float16(scale);
      p = df_vfmul(p, df_uint(f16_scale << 16 | f16_scale));
   }

   dflib_unpackHalf2x16(r, p);

   if (!UNPACK_BYTE_MEDP) {
      Dataflow *df_scale = df_float(scale);
      for (unsigned i = 0; i != 2; ++i)
         r[i] = df_mul(r[i], df_scale);
   }
}

static void unpack_snorm8_xz(Dataflow *r[2], Dataflow *p) {

   const unsigned mask = 0x00ff00ffu << UNPACK_BYTE_SHIFT;
   const float scale = (float)(1u << 24u >> UNPACK_BYTE_SHIFT) / 127.0f;

   p = df_and_const(p, mask);
   p = glsl_dataflow_construct_unary_op(DATAFLOW_VITODENF, p);
   p = df_vfmax(p, df_uint(0x80008000u | 0x007f007fu << UNPACK_BYTE_SHIFT));  // make 0x80(-128) byte equivalent to 0x81(-127).

   if (UNPACK_BYTE_MEDP) {
      const uint32_t f16_scale = gfx_float_to_float16(scale);
      p = df_vfmul(p, df_uint(f16_scale << 16 | f16_scale));
   }

   dflib_unpackHalf2x16(r, p);

   if (!UNPACK_BYTE_MEDP) {
      Dataflow *df_scale = df_float(scale);
      for (unsigned i = 0; i != 2; ++i)
         r[i] = df_mul(r[i], df_scale);
   }
}

Dataflow *dflib_packHalf2x16(Dataflow *x, Dataflow *y) {
   return df_fpack(x, y);
}

Dataflow *dflib_packUnorm2x16(Dataflow *x, Dataflow *y) {
   Dataflow *v[2] = { x, y };
   return packu(v, 2, true, 16);
}

Dataflow *dflib_packSnorm2x16(Dataflow *x, Dataflow *y) {
   Dataflow *v[2] = { x, y };
   return packs(v, 2, true, 16);
}

// Upper 16-bits are undefined.
static Dataflow *packUnorm2x8_undef(Dataflow *x, Dataflow *y) {
   Dataflow *pxy = pack_unorm8_xz(x, y);
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, pxy, df_shr_const(pxy, 8));
}

Dataflow *dflib_packUnorm4x8(Dataflow *x, Dataflow *y, Dataflow *z, Dataflow *w) {
   Dataflow *pxz = pack_unorm8_xz(x, z);
   Dataflow *pyw = pack_unorm8_xz(y, w);
   return df_combineu(pxz, pyw, 8);
}

// Upper 16-bits are undefined.
static Dataflow *packSnorm2x8_undef(Dataflow *x, Dataflow *y) {
   Dataflow *pxy = pack_snorm8_xz(x, y);
   return glsl_dataflow_construct_binary_op(DATAFLOW_ADD, pxy, df_shr_const(pxy, 8));
}

Dataflow *dflib_packSnorm4x8(Dataflow *x, Dataflow *y, Dataflow *z, Dataflow *w) {
   Dataflow *pxz = pack_snorm8_xz(x, z);
   Dataflow *pyw = pack_snorm8_xz(y, w);
   return df_combineu(pxz, pyw, 8); // Deliberate use of combineu since pack_snorm8_xz already cleared unwanted sign bits.
}

Dataflow *dflib_pack_format(FormatQualifier f, Dataflow *v[4]) {

   Dataflow *r[4] = { NULL, };
   switch (f) {

   case FMT_RGBA32F:
   case FMT_RGBA32UI:
   case FMT_RGBA32I:
      for (unsigned i = 0; i != 4; ++i)
         r[i] = df_reinterp_uint(v[i]);
      break;

   case FMT_RG32F:
   case FMT_RG32UI:
   case FMT_RG32I:
      for (unsigned i = 0; i != 2; ++i)
         r[i] = df_reinterp_uint(v[i]);
      break;

   case FMT_R32F:
   case FMT_R32UI:
   case FMT_R32I:
      r[0] = df_reinterp_uint(v[0]);
      break;

   case FMT_R11G11B10F:    r[0] = pack_ufloat11(v); break;

   case FMT_RGBA16F:       r[0] = df_fpack(v[0], v[1]);
                           r[1] = df_fpack(v[2], v[3]); break;
   case FMT_RG16F:         r[0] = df_fpack(v[0], v[1]); break;
   case FMT_R16F:          r[0] = df_fpack(v[0], v[0]); break;


   case FMT_RGBA16:        r[0] = packu(v, 2, true,  16); r[1] = packu(v + 2, 2, true,  16); break;
   case FMT_RGBA16_SNORM:  r[0] = packs(v, 2, true,  16); r[1] = packs(v + 2, 2, true,  16); break;
   case FMT_RGBA16UI:      r[0] = packu(v, 2, false, 16); r[1] = packu(v + 2, 2, false, 16); break;
   case FMT_RGBA16I:       r[0] = packs(v, 2, false, 16); r[1] = packs(v + 2, 2, false, 16); break;

   case FMT_RG16:          r[0] = packu(v, 2, true,  16); break;
   case FMT_RG16_SNORM:    r[0] = packs(v, 2, true,  16); break;
   case FMT_RG16UI:        r[0] = packu(v, 2, false, 16); break;
   case FMT_RG16I:         r[0] = packs(v, 2, false, 16); break;

   case FMT_R16:           r[0] = packu(v, 1, true,  16); break;
   case FMT_R16_SNORM:     r[0] = packs(v, 1, true,  16); break;
   case FMT_R16UI:         r[0] = packu(v, 1, false, 16); break;
   case FMT_R16I:          r[0] = packs(v, 1, false, 16); break;

   case FMT_RGBA8:         r[0] = dflib_packUnorm4x8(v[0], v[1], v[2], v[3]); break;
   case FMT_RGBA8_SNORM:   r[0] = dflib_packSnorm4x8(v[0], v[1], v[2], v[3]); break;
   case FMT_RGBA8UI:       r[0] = packu(v, 4, false, 8); break;
   case FMT_RGBA8I:        r[0] = packs(v, 4, false, 8); break;

   case FMT_RG8:           r[0] = packUnorm2x8_undef(v[0], v[1]); break; // upper 16-bits truncated by TMU store.
   case FMT_RG8_SNORM:     r[0] = packSnorm2x8_undef(v[0], v[1]); break;
   case FMT_RG8UI:         r[0] = packu(v, 2, false, 8); break;
   case FMT_RG8I:          r[0] = packs(v, 2, false, 8); break;

   case FMT_R8:            r[0] = packu(v, 1, true,  8); break;
   case FMT_R8_SNORM:      r[0] = packs(v, 1, true,  8); break;
   case FMT_R8UI:          r[0] = packu(v, 1, false, 8); break;
   case FMT_R8I:           r[0] = packs(v, 1, false, 8); break;

   case FMT_RGB10A2:       r[0] = pack_unorm10(v); break;
   case FMT_RGB10A2UI:     r[0] = packu(v, 4, false, 10); break;

   default:
      unreachable();
   }

   return glsl_dataflow_construct_vec4(r[0], r[1], r[2], r[3]);
}


void dflib_unpackHalf2x16(Dataflow *r[2], Dataflow *p) {
   r[0] = glsl_dataflow_construct_unary_op(DATAFLOW_FUNPACKA, p);
   r[1] = glsl_dataflow_construct_unary_op(DATAFLOW_FUNPACKB, p);
}

void dflib_unpackUnorm2x16(Dataflow *r[2], Dataflow *p) {
   r[0] = unpack_unorm(df_and_const(p, 0xffff), 16);
   r[1] = unpack_unorm(df_shr_const(p, 16), 16);
}

void dflib_unpackSnorm2x16(Dataflow *r[2], Dataflow *p) {
   Dataflow *u16 = df_uint(16);
   p = df_reinterp_int(p);
   r[0] = unpack_snorm(df_shr(df_shl(p, u16), u16), 16);
   r[1] = unpack_snorm(df_shr(p,              u16), 16);
}

void dflib_unpackUnorm4x8(Dataflow *r[4], Dataflow *p) {
   unpack_unorm8_xz(&r[0], UNPACK_BYTE_SHIFT ? df_shl_const(p, UNPACK_BYTE_SHIFT) : p);
   unpack_unorm8_xz(&r[2],                     df_shr_const(p, 8 - UNPACK_BYTE_SHIFT));
   df_swap(&r[1], &r[2]);
}

void dflib_unpackSnorm4x8(Dataflow *r[4], Dataflow *p) {
   unpack_snorm8_xz(&r[0], UNPACK_BYTE_SHIFT ? df_shl_const(p, UNPACK_BYTE_SHIFT) : p);
   unpack_snorm8_xz(&r[2],                     df_shr_const(p, 8 - UNPACK_BYTE_SHIFT));
   df_swap(&r[1], &r[2]);
}
