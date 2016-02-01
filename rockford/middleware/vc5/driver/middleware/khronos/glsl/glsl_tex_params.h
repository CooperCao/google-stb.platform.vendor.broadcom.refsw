/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION

Generic texture parameter getting/setting for the compiler
=============================================================================*/
#ifndef GLSL_TEX_PARAMS_H
#define GLSL_TEX_PARAMS_H

/* LTYPE [0:2] */
#define GLSL_TEXBITS_2D         0
#define GLSL_TEXBITS_2D_ARRAY   1
#define GLSL_TEXBITS_3D         2
#define GLSL_TEXBITS_CUBE       3
#define GLSL_TEXBITS_1D         4
#define GLSL_TEXBITS_1D_ARRAY   5
#define GLSL_TEXBITS_CUBE_ARRAY 6

#define GLSL_TEXBITS_FETCH  (1<<3)
#define GLSL_TEXBITS_GATHER (1<<4)
#define GLSL_TEXBITS_BIAS   (1<<5)
#define GLSL_TEXBITS_BSLOD  (1<<6)
#define GLSL_TEXBITS_SHADOW (1<<7)

#define GLSL_TEXBITS_GATHER_COMP_SHIFT 17
#define GLSL_TEXBITS_GATHER_COMP_MASK  (0x3 << GLSL_TEXBITS_GATHER_COMP_SHIFT)

#define GLSL_TEXBITS_OFFSET(s,t,r) (((s)&15)<<19 | ((t)&15)<<23 | ((r)&15)<<27)
#define GLSL_TEXBITS_PIX_MASK (1<<31)



#if V3D_HAS_NEW_TMU_CFG

static inline uint32_t backend_uniform_get_sampler(uint32_t u) {
   return (u >> 4);
}

static inline uint32_t backend_uniform_get_extra(uint32_t u) {
   return u & 0xF;
}

#else

/* When requesting TMU parameters the compiler provides a sampler index and
 * 'extra' bits that need to be included in the parameters.  The bit pattern
 * is based on param 0, with the sampler index and param 1 information packed
 * into the gap [8:18]. TMU config 0 is not supported */

/* Sampler Index [8:12] */
#define SAMPLER_SHIFT   8
#define SAMPLER_MASK    (0x1F << SAMPLER_SHIFT)

#define GLSL_TEXBITS_WORD_READS_SHIFT  13
#define GLSL_TEXBITS_WORD_READS_MASK   (0xf << GLSL_TEXBITS_WORD_READS_SHIFT)

#define PACK_SAMPLER(s) ( ( (s) << SAMPLER_SHIFT ) & SAMPLER_MASK )
#define BACKEND_UNIFORM_MAKE_PARAM(s, e) ( (e) | PACK_SAMPLER(s) )

static inline uint32_t backend_uniform_get_sampler(uint32_t u)
{
   return (u & SAMPLER_MASK) >> SAMPLER_SHIFT;
}

static inline uint32_t backend_uniform_get_extra(uint32_t u)
{
   return u & ~SAMPLER_MASK;
}

static inline uint32_t backend_uniform_get_extra_param0(uint32_t u)
{
   uint32_t extra = backend_uniform_get_extra(u);
   uint32_t shadow_bit = (extra & GLSL_TEXBITS_SHADOW) << 1;
   extra &= ~(GLSL_TEXBITS_WORD_READS_MASK | GLSL_TEXBITS_GATHER_COMP_MASK | GLSL_TEXBITS_SHADOW);
   return extra | shadow_bit;
}

static inline uint32_t backend_uniform_get_extra_param1(uint32_t u)
{
   /* We only care about word enables */
   uint32_t extra = backend_uniform_get_extra(u);
   return (extra & GLSL_TEXBITS_WORD_READS_MASK) >> GLSL_TEXBITS_WORD_READS_SHIFT;
}

#endif // V3D_HAS_NEW_TMU_CFG

#endif //GLSL_TEX_PARAMS_H
