/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#if V3D_VER_AT_LEAST(4,0,2,0)

#define GLSL_SAMPLER_NONE 0xFFFFFFF    /* Note 28 bit, not 32 */

static inline uint32_t backend_uniform_get_sampler(uint32_t u) {
   return (u >> 4);
}

static inline uint32_t backend_uniform_get_extra(uint32_t u) {
   return u & 0xF;
}

#else

/* When requesting TMU parameters the compiler provides a sampler index and
 * 'extra' bits that need to be included in the parameters.
 * Both parameters include the image and sampler indices.
 *
 * TMU config 0 is not supported */

/* Image Index   [ 9:13] */
/* Sampler Index [14:18] */
#define IMAGE_SHIFT   9
#define IMAGE_MASK    (0x1F << IMAGE_SHIFT)

#define SAMPLER_SHIFT 14
#define SAMPLER_MASK  (0x1F << SAMPLER_SHIFT)

#define PACK_IMAGE(i)   ( ( (i) << IMAGE_SHIFT   ) & IMAGE_MASK   )
#define PACK_SAMPLER(s) ( ( (s) << SAMPLER_SHIFT ) & SAMPLER_MASK )

#define BACKEND_UNIFORM_MAKE_PARAM(i, s, e) ( (e) | PACK_IMAGE(i) | PACK_SAMPLER(s) )


/* PARAM 0 */

/* LTYPE [0:2] */
#define GLSL_TEXPARAM0_2D         0
#define GLSL_TEXPARAM0_2D_ARRAY   1
#define GLSL_TEXPARAM0_3D         2
#define GLSL_TEXPARAM0_CUBE       3
#define GLSL_TEXPARAM0_1D         4
#define GLSL_TEXPARAM0_1D_ARRAY   5

#define GLSL_TEXPARAM0_FETCH  (1<<3)
#define GLSL_TEXPARAM0_GATHER (1<<4)
#define GLSL_TEXPARAM0_BIAS   (1<<5)
#define GLSL_TEXPARAM0_BSLOD  (1<<6)
#define GLSL_TEXPARAM0_SHADOW (1<<8)

#define GLSL_TEXPARAM0_OFFSET(s,t,r) (((s)&15)<<19 | ((t)&15)<<23 | ((r)&15)<<27)
#define GLSL_TEXPARAM0_PIX_MASK (1<<31)


/* PARAM 1 */

#define GLSL_TEXPARAM1_GATHER_COMP_SHIFT 6
#define GLSL_TEXPARAM1_GATHER_COMP_MASK  (0x3 << GLSL_TEXPARAM1_GATHER_COMP_SHIFT)
#define GLSL_TEXPARAM1_FETCH            (1<<5)
#define GLSL_TEXPARAM1_GATHER           (1<<4)
#define GLSL_TEXPARAM1_WORD_READ_MASK    0xF


static inline uint32_t backend_uniform_get_image(uint32_t u) {
   return (u & IMAGE_MASK) >> IMAGE_SHIFT;
}

static inline uint32_t backend_uniform_get_sampler(uint32_t u) {
   return (u & SAMPLER_MASK) >> SAMPLER_SHIFT;
}

static inline uint32_t backend_uniform_get_extra(uint32_t u) {
   return u & ~(SAMPLER_MASK | IMAGE_MASK);
}

static inline uint32_t backend_uniform_get_extra_param0(uint32_t u) {
   return backend_uniform_get_extra(u);
}

static inline uint32_t backend_uniform_get_extra_param1(uint32_t u) {
   /* Mask out everything but the word reads */
   return backend_uniform_get_extra(u) & GLSL_TEXPARAM1_WORD_READ_MASK;
}

#endif // V3D_VER_AT_LEAST(4,0,2,0)
