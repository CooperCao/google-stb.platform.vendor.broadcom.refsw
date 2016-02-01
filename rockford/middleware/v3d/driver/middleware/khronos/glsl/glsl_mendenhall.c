/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#include "middleware/khronos/glsl/glsl_common.h"

#include "middleware/khronos/glsl/glsl_mendenhall.h"

#include "middleware/khronos/glsl/2708/glsl_fpu_4.h"

// Generates a sine-cosine pair in 50 cycles, (including 25 stalls)
// NB.  Cycle counts should be regarded as estimates, and the comments
// may be out of date by now.  TO-DO - recalculate cycle count more accurately
void glsl_mendenhall_sincospair(uint32_t a, uint32_t *s, uint32_t *c)
{
   unsigned x,xint,xx,xxxx,sinpart,cospart,sinsemi,cossemi,sinfinal,cosfinal;
   unsigned cospart2,sinpart2,cos2part,sin2part,cos2semi,sin2semi,sincospart,sincossemi;
#ifndef SKIP_CORRECTION
   unsigned sinuncorr,cosuncorr,hypsq,errormul;
#endif

   glsl_fpu_mul(&x, a, I_CONST_RECIP2PI);              // 1 cycle (+2?)
   glsl_fpu_floattointn(&xint, x, 0);                // 3 cycles 2 stalls
   glsl_fpu_inttofloat(&xint, xint, 0);              // 3 cycles 2 stalls
   glsl_fpu_sub(&x, x,xint);                         // 3 cycles 2 stalls
   glsl_fpu_mul(&xx, x,x);                           // 3 cycles 2 stalls
   glsl_fpu_mul(&xxxx, xx,xx);                       // 3 cycles 2 stalls
   glsl_fpu_mul(&sinpart, xx,I_COEFF_SIN7);            // 2 cycles 1 stall
   glsl_fpu_mul(&cospart, xx,I_COEFF_COS6);            // 1 cycle
   glsl_fpu_add(&sinpart, sinpart,I_COEFF_SIN5);       // 2 cycles 1 stall
   glsl_fpu_add(&cospart, cospart,I_COEFF_COS4);       // 2 cycles 1 stall
   glsl_fpu_mul(&sinpart2, xx,I_COEFF_SIN3);           // 1 cycle
   glsl_fpu_mul(&sinpart, sinpart,xxxx);             // 1 cycle
   glsl_fpu_mul(&cospart2, xx,I_COEFF_COS2);           // 1 cycle
   glsl_fpu_mul(&cospart, cospart,xxxx);             // 1 cycle
   glsl_fpu_add(&sinpart, sinpart,sinpart2);         // 1 cycle
   glsl_fpu_add(&cospart, cospart,cospart2);         // 2 cycles 1 stall
   glsl_fpu_add(&sinpart, sinpart,I_COEFF_SIN1);       // 2 cycles 1 stall
   glsl_fpu_add(&cospart, cospart,I_COEFF_COS0);       // 2 cycles 1 stall
   glsl_fpu_mul(&sinpart, sinpart,x);                // 2 cycles 1 stall
   glsl_fpu_mul(&cos2part, cospart,cospart);         // 2 cycles 1 stall
   glsl_fpu_mul(&sin2part, sinpart,sinpart);         // 2 cycles 1 stall
   glsl_fpu_mul(&sincospart, sinpart,cospart);       // 1 cycle
   glsl_fpu_sub(&cossemi, cos2part,sin2part);        // 2 cycles 1 stall
   sinsemi = sincospart+0x00800000; // multiply by two in one cycle - ignoring denormals, zeroes, nans, infinities... none of which we care about

   glsl_fpu_mul(&sin2semi, sinsemi,sinsemi);         // 1 cycle (because no need to wait for previous result)
   glsl_fpu_mul(&cos2semi, cossemi,cossemi);         // 1 cycle
   glsl_fpu_mul(&sincossemi, sinsemi,cossemi);       // 1 cycle

#ifndef SKIP_CORRECTION
   glsl_fpu_add(&hypsq,cos2semi,sin2semi);           // 2 cycles 1 stall
   glsl_fpu_sub(&cosuncorr, cos2semi,sin2semi);      // 1 cycle
   glsl_fpu_sub(&errormul, I_CONST_TWO, hypsq);       // 2 cycles 1 stall
   sinuncorr = sincossemi+0x00800000; // again, a cheap *2 // 1 cycle
   glsl_fpu_mul(&cosfinal,errormul,cosuncorr);       // 2 cycles 1 stall
   glsl_fpu_mul(&sinfinal,errormul,sinuncorr);       // 1 cycle
#else
   glsl_fpu_sub(&cosfinal, cos2semi,sin2semi);       // 2 cycles 1 stall
   sinfinal = sincossemi+0x00800000; // again, a cheap *2 // 1 cycle
#endif

   *c = cosfinal;
   *s = sinfinal;
}
