/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  sfu
=============================================================================*/

#ifndef SFU_H
#define SFU_H

#include "libs/core/v3d/v3d_common.h"
#include "vcos_types.h"
#include <stdbool.h>

VCOS_EXTERN_C_BEGIN

extern unsigned int sfu_exp (unsigned int b, bool prop_nan);
extern unsigned int sfu_log (unsigned int b, bool prop_nan);
extern unsigned int sfu_recipsqrt (unsigned int b, bool prop_nan);
#if V3D_VER_AT_LEAST(3,3,0,0)
extern unsigned int sfu_recipsqrt2 (unsigned int b, bool prop_nan);
#endif
extern unsigned int sfu_recip (unsigned int b, bool prop_nan);
extern unsigned int sfu_sin (unsigned int b, bool prop_nan);

VCOS_EXTERN_C_END

#endif
