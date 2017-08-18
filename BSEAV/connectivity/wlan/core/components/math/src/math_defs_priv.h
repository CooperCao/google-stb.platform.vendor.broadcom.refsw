/*
 * Math component internal definitions.
 *
 * $Copyright (C) 2016 Broadcom Corporation$
 *
 * $Id:$
 */

#ifndef _math_intern_defs_h_
#define _math_intern_defs_h_


#if defined(USE_MATH_COMPONENT)

#define MASK_32_BITS	(~0)
#define MASK_8_BITS	((1 << 8) - 1)

#define EXTRACT_LOW32(num)	(uint32)(num & MASK_32BITS)
#define EXTRACT_HIGH32(num)	(uint32)(((uint64)num >> 32) & MASK_32BITS)

#define MAXIMUM(a, b) ((a > b) ? a : b)
#define MINIMUM(a, b) ((a < b) ? a : b)
#define LIMIT(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define	CORDIC_AG	39797
#define	CORDIC_NI	18
#define	FIXED(X)	((int32)((X) << 16))
#define	FLOAT(X)	(((X) >= 0) ? ((((X) >> 15) + 1) >> 1) : -((((-(X)) >> 15) + 1) >> 1))

#endif /* !USE_MATH_COMPONENT */


#endif /* _math_intern_defs_h_ */
