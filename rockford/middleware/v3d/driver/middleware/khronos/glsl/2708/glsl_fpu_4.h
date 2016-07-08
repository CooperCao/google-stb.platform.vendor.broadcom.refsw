/*=============================================================================
Broadcom Proprietary and Confidential. (c)2007 Broadcom.  All rights reserved.

Project  :  C6357
Module   :

FILE DESCRIPTION
Protoypes and constants for the simulation floating point support.
=============================================================================*/

#ifndef GLSL_FPU_4_H
#define GLSL_FPU_4_H

typedef union
{
   float f;
   uint32_t ui;
} mixed_float;

extern int glsl_fpu_add (unsigned *r, unsigned a, unsigned b);
extern int glsl_fpu_sub (unsigned *r, unsigned a, unsigned b);
extern int glsl_fpu_rsub (unsigned *r, unsigned a, unsigned b);
extern int glsl_fpu_mul (unsigned *r, unsigned a, unsigned b);
extern int glsl_fpu_muln (unsigned *r, unsigned a, unsigned b);
extern int glsl_fpu_min (unsigned *r, unsigned a, unsigned b);
extern int glsl_fpu_max (unsigned *r, unsigned a, unsigned b);
extern int glsl_fpu_floor (unsigned *r, unsigned b);
extern int glsl_fpu_ceil (unsigned *r, unsigned b);
extern int glsl_fpu_log2 (unsigned *r, unsigned b);
extern int glsl_fpu_exp2 (unsigned *r, unsigned b);
extern int glsl_fpu_recip (unsigned *r, unsigned b);
extern int glsl_fpu_rsqrt (unsigned *r, unsigned b);

extern void glsl_fpu_minabs (unsigned *r, unsigned a, unsigned b);
extern void glsl_fpu_maxabs (unsigned *r, unsigned a, unsigned b);
extern void glsl_fpu_floattouintz (unsigned *r, unsigned a, unsigned b);
extern void glsl_fpu_floattouintn (unsigned *r, unsigned a, unsigned b);
extern void glsl_fpu_floattointz (unsigned *r, unsigned a, unsigned b);
extern void glsl_fpu_floattointn (unsigned *r, unsigned a, unsigned b);
extern int glsl_fpu_inttofloat (unsigned *r, unsigned a, unsigned b);
extern int glsl_fpu_uinttofloat (unsigned *r, unsigned a, unsigned b);
extern int glsl_fpu_floattouintq (unsigned *r, unsigned a, unsigned b);

/* These are flags in the p0 register.  They are set whenever the
   corresponding exceptions occur.  */
#define FPE_IO 0x01             /* Invalid Operation.  */
#define FPE_DZ 0x02             /* Divide by Zero.  */
#define FPE_OF 0x04             /* Overflow.  */
#define FPE_UF 0x08             /* Underflow.  */
/* Inexact exceptions are excessively buggy in A0 sillicon, and have
   been dropped from B0 onwards. So we don't try to support them in
   the sim at all.  */
/* #define FPE_I  0x10 */       /* Inexact.  */

/* Check for a NaN.  */
#define IS_NAN(f) ((((unsigned) f) & ~(1U << 31)) > (0xFF << 23))

#endif