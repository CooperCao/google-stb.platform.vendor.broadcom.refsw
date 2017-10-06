/*
 * Math component interface file.
 * Math definitions.
 *
 * $Copyright (C) 2016 Broadcom Corporation$
 *
 * $Id:$
 */

#ifndef _math_defs_h_
#define _math_defs_h_


#if !defined(USE_MATH_COMPONENT)
#include <bcmutils.h>

#else

typedef int32 math_fixed; /* s15.16 fixed-point */

typedef struct _cint32 {
	math_fixed	q;
	math_fixed	i;
} math_cint32;

typedef math_cint32 cint32;

#ifndef ABS
#define ABS(x) (((x) < 0) ? (-(x)) : (x))
#endif

#endif /* !USE_MATH_COMPONENT */


#endif /* _math_defs_h_ */
