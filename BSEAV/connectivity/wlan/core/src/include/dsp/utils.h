/*******************************************************************************
 * $Id$
 * utils.h - dsp related macros and inline function utilities
 ******************************************************************************/

#ifndef _DSP_UTILS_H_
#define _DSP_UTILS_H_

#include <math.h> /* this should be the only dependency on libm */
#include "dsp/types.h"

/* prototypes for functions in utils.c */
uint16 dB_ui16(uint16);
uint16 db_power(uint32);
uint32 db_power_inv(uint16);
uint16 mag_approx(int16, int16);
void mag_approx_2(cint16 *, uint32 *, uint16 *, uint32 *, uint8);

#if COUNT_OPS

typedef struct {
    uint32 add;
    uint32 sub;
    uint32 mul;
    uint32 muladd;
    uint32 mulsub;
    uint32 div;
    uint32 add2;
    uint32 sub2;
    uint32 cfloat;
    uint32 c_mag2;
    uint32 c_neg;
    uint32 c_add;
    uint32 c_mpy;
    uint32 c_mpya;
    uint32 rc_mpy;
    uint32 rc_mpya;
    uint32 c_conj;
} opcounts_t;

extern opcounts_t opcounts;

#endif /* COUNT_OPS */

#if defined(_X86_)
#include "x86/proc_utils.h"
#endif /* _X86_ */

/* basic mux operation - can be optimized on several architectures */
#define MUX(pred, true, false) ((pred) ? (true) : (false))

#ifndef MAX
#define MAX(a, b) MUX((a) > (b), (a), (b))
#endif
#ifndef MIN
#define MIN(a, b) MUX((a) < (b), (a), (b))
#endif

/* modulo inc/dec - assumes x E [0, bound - 1] */
#define MODDEC(x, bound) MUX((x) == 0, (bound) - 1, (x) - 1)
#define MODINC(x, bound) MUX((x) == (bound) - 1, 0, (x) + 1)

/* modulo inc/dec, bound = 2^k */
#define MODDEC_POW2(x, bound) (((x) - 1) & ((bound) - 1))
#define MODINC_POW2(x, bound) (((x) + 1) & ((bound) - 1))

/* modulo add/sub - assumes x, y E [0, bound - 1] */
#define MODADD(x, y, bound) \
    MUX((x) + (y) >= (bound), (x) + (y) - (bound), (x) + (y))
#define MODSUB(x, y, bound) \
    MUX(((int)(x)) - ((int)(y)) < 0, (x) - (y) + (bound), (x) - (y))

/* module add/sub, bound = 2^k */
#define MODADD_POW2(x, y, bound) (((x) + (y)) & ((bound) - 1))
#define MODSUB_POW2(x, y, bound) (((x) - (y)) & ((bound) - 1))

/* round towards nearest integer */
//#define ROUND(x, s) (((x) + (1 << ((s) - 1))) >> (s))
#define HWROUND(x, s) (((x) >> (s)) + (((x) >> ((s) - 1)) & 1))
#define HWROUND0(x, s) (((x) >> (s)) + (((x) >> ((s) - 1)) & (s!=0)))

/* convert a fixedpoint number to float */
/* i is the fixedpoint integer, p is the position of the fractional point */
#define FIXED2FLOAT(i, p) ((float)(i)/(1<<(p)))
#define FIXED2DOUBLE(i, p) ((double)(i)/(1<<(p)))

/* absolute value */
#define ABS_I8(x)  ((((x) >>  7) ^ (x)) - ((x) >>  7))
#define ABS_I16(x) ((((x) >> 15) ^ (x)) - ((x) >> 15))
#define ABS_I32(x) ((((x) >> 31) ^ (x)) - ((x) >> 31))

/* limit to [min, max] */
#define LIMIT(x, min, max) \
    ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* limit to  max */
#define LIMIT_TO_MAX(x, max) \
    (((x) > (max) ? (max) : (x)))

/* limit to min */
#define LIMIT_TO_MIN(x, min) \
    (((x) < (min) ? (min) : (x)))

#define R_SAT(x,num_bits) \
    LIMIT(x, -(1<<(num_bits-1)), ((1<<(num_bits-1))-1) )

#define R_SAT64(x,num_bits) \
    LIMIT(x, -(int64(1)<<(num_bits-1)), ((int64(1)<<(num_bits-1))-int64(1)) )

/* abstracted libm functions */
#define FLOOR(x)    floor(x)
#define CEIL(x)     ceil(x)
#define ATAN2(x, y) atan2(x, y)
#define LOG10(x)    log10(x)
#define LOG(x)      log(x)
#define FMOD(x, y) \
    MUX((x) > 0.0, fmod((x), (y)), fmod((x), (y)) + (y))
#define SQRT(x)     sqrt(x)

/* bit level operators */
#define EXTRACT_UI32(x, start, nbits) \
     (((uint32)(x) << (31 - (start))) >> (32 - (nbits)))

#if COUNT_OPS

/* basic arithmetic ops */
#define MUL(x, y)       ( opcounts.mul++, ((x) * (y)) )
#define MULADD(a, x, y) ( opcounts.muladd++, ((a) + (x) * (y)) )
#define MULSUB(a, x, y) ( opcounts.mulsub++, ((a) - (x) * (y)) )
#define DIV(x, y)       ( opcounts.div++, ((x) / (y)) )
#define ADD(x, y)       ( opcounts.add++, ((x) + (y)) )
#define SUB(x, y)       ( opcounts.sub++, ((x) - (y)) )
#define ADD2(x, y)      ( opcounts.add2++, ((x) * (x) + (y) * (y)) )
#define SUB2(x, y)      ( opcounts.sub2++, ((x) * (x) - (y) * (y)) )

#else /* !COUNT_OPS */

/* basic arithmetic ops */
#define MUL(x, y)       ((x) * (y))
#define MULADD(a, x, y) ((a) + (x) * (y))
#define MULSUB(a, x, y) ((a) - (x) * (y))
#define DIV(x, y)       ((x) / (y))
#define ADD(x, y)       ((x) + (y))
#define SUB(x, y)       ((x) - (y))
#define ADD2(x, y)      ((x) * (x) + (y) * (y))
#define SUB2(x, y)      ((x) * (x) - (y) * (y))

#endif /* COUNT_OPS */

static INLINE cfloat_t
CFLOAT(
    float_t i,
    float_t q
)
{
    cfloat_t z;
    z.i = i;
    z.q = q;
    return(z);
}

static INLINE float_t
C_MAG2(
    cfloat_t x
)
{
#if COUNT_OPS
    opcounts.c_mag2++;
#endif
    return(x.i*x.i + x.q*x.q);
}

static INLINE cfloat_t
C_NEG(
    cfloat_t x
)
{
    cfloat_t z;

    z.i = -x.i;
    z.q = -x.q;
    return(z);
}

static INLINE cfloat_t
C_MPY(
    cfloat_t x,
    cfloat_t y
)
{
    cfloat_t z;

    z.i = x.i * y.i - x.q * y.q;
    z.q = x.i * y.q + x.q * y.i;
#if COUNT_OPS
    opcounts.c_mpy++;
#endif
    return(z);
}

static INLINE void
C_MPYA(
    cfloat_t x,
    cfloat_t y,
    cfloat_t *acc
)
{
    acc->i += x.i * y.i - x.q * y.q;
    acc->q += x.i * y.q + x.q * y.i;
#if COUNT_OPS
    opcounts.c_mpya++;
#endif
}

static INLINE void
C_MPYS(
    cfloat_t x,
    cfloat_t y,
    cfloat_t *acc
)
{
    acc->i -= x.i * y.i - x.q * y.q;
    acc->q -= x.i * y.q + x.q * y.i;
#if COUNT_OPS
    opcounts.c_mpya++;
#endif
}

static INLINE cfloat_t
RC_MPY(
    float_t x,
    cfloat_t y
)
{
    cfloat_t z;
    z.i = x * y.i;
    z.q = x * y.q;
#if COUNT_OPS
    opcounts.rc_mpy++;
#endif
    return(z);
}

static INLINE void
RC_MPYA(
    float_t x,
    cfloat_t y,
    cfloat_t *acc
)
{
    acc->i += x * y.i;
    acc->q += x * y.q;
#if COUNT_OPS
    opcounts.rc_mpya++;
#endif
}

static INLINE cfloat_t
C_CONJ(
    cfloat_t x
)
{
    cfloat_t z;

    z.i = x.i;
    z.q = -x.q;
    return(z);
}

static INLINE cfloat_t
CLOAD(
    float_t i,
    float_t q
)
{
    cfloat_t z;
    z.i = i;
    z.q = q;
    return(z);
}

static INLINE cint8
CLOAD8(
    int8 i,
    int8 q
)
{
    cint8 z;
    z.i = i;
    z.q = q;
    return(z);
}

static INLINE cint16
CLOAD16(
    int16 i,
    int16 q
)
{
    cint16 z;
    z.i = i;
    z.q = q;
    return(z);
}

static INLINE cint32
CLOAD32(
    int32 i,
    int32 q
)
{
    cint32 z;
    z.i = i;
    z.q = q;
    return(z);
}

static INLINE cint64
CLOAD64(
    int64 i,
    int64 q
)
{
    cint64 z;
    z.i = i;
    z.q = q;
    return(z);
}

#if defined(FIXED_POINT)
static INLINE uint16
C_MAG2_FXPT(
    cint16 x,
    uint16 nfracbits
)
{
#if COUNT_OPS
    opcounts.c_mag2++;
#endif
    return(HWROUND((x.i * x.i + x.q * x.q), nfracbits));
}
#endif



#if defined(FIXED_POINT)
static INLINE cint16
C_NEG16(
    cint16 x
)
{
    cint16 z;

    z.i = -x.i;
    z.q = -x.q;
    return(z);
}
#endif

/*******************************************************************
 * unsigned 16 x unsigned 16 bit multiply, returns rounded 16 bits
 ********************************************************************/

#if defined(FIXED_POINT)
static INLINE uint16
MULT16(
    uint16 x,
    uint16 y,
    uint nbits
)
{
    uint16 z;

    z = HWROUND(x * y, nbits);
#if COUNT_OPS
    opcounts.mul++;
#endif
    return (z);
}
#endif


#if defined(FIXED_POINT)
static INLINE cint16
C16C16_MPY(
    cint16 x,
    cint16 y,
    uint16 nfracbits
)
{
    cint16 z;

    z.i = HWROUND( (x.i*y.i - x.q*y.q),nfracbits);
    z.q = HWROUND( (x.i*y.q + x.q*y.i),nfracbits);
#if COUNT_OPS
    opcounts.c_mpy++;
#endif
    return(z);
}
#endif

/* compute log2(x) where x = 2^k */
static INLINE int
LOG2(
    uint32 x
)
{
    int log2_x = 0;
    while ((x >>= 1) != 0)
        log2_x++;
    return log2_x;
}

static INLINE int
INT64_LOG2(
    uint64 x
)
{
    int log2_x = 0;
    while ((x >>= 1) != 0)
        log2_x++;
    return log2_x;
}



#endif /* _DSP_UTILS_H_ */
