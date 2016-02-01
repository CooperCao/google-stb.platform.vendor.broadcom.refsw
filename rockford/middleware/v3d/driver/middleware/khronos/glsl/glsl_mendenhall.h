/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_MENDENHALL_H
#define GLSL_MENDENHALL_H

#define I_CONST_RECIP2PI   0x3e22f983
#define F_CONST_RECIP2PI   0.1591549431
#define I_CONST_TWO        0x40000000
#define F_CONST_TWO        2.0
#define I_COEFF_COS0       0x3f800000
#define F_COEFF_COS0       1.0
#define I_COEFF_COS2       0xbf9de9cf
#define F_COEFF_COS2       -1.2336977925
#define I_COEFF_COS4       0x3e81d8fd
#define F_COEFF_COS4       0.2536086171
#define I_COEFF_COS6       0xbca77008
#define F_COEFF_COS6       -0.0204391631
#define I_COEFF_SIN1       0x3fc90fdb
#define F_COEFF_SIN1       1.5707963235
#define I_COEFF_SIN3       0xbf255ddf
#define F_COEFF_SIN3       -0.645963615
#define I_COEFF_SIN5       0x3da3304e
#define F_COEFF_SIN5       0.0796819754
#define I_COEFF_SIN7       0xbb96fb24
#define F_COEFF_SIN7       -0.0046075748

// For lower accuracy, but a bit faster, we can skip the final error correction.
// Note this also updates definitions in header.shader.c!
// #define SKIP_CORRECTION

extern void glsl_mendenhall_sincospair(uint32_t a, uint32_t *s, uint32_t *c);

#endif // MENDENHALL_H
