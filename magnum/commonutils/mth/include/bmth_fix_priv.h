/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BMTH_FIX_PRIV_H__
#define BMTH_FIX_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bstd.h"
#include "bkni.h"

#define BMTH_P_FIX_MAX_BITS 32
#define BMTH_P_FIX_SIGNED_MAX_BITS (BMTH_P_FIX_MAX_BITS - 1)

#define BMTH_P_FIX_UNSIGNED_PI 0xC90FDAA2 /* pi in 2.30 unsigned fixed notation */
#define BMTH_P_FIX_UNSIGNED_HALF_PI (BMTH_P_FIX_UNSIGNED_PI / 2)
#define BMTH_P_FIX_UNSIGNED_PI_INT_BITS 1
#define BMTH_P_FIX_UNSIGNED_RAD_FRACT_BITS 30

#define BMTH_P_FIX_SINMAG_TABLE_SIZE (sizeof(s_lFixSinMagTable) / sizeof(uint32_t))

#define BMTH_P_FIX_SIGNED_SINMAG_INT_BITS 1
#define BMTH_P_FIX_SIGNED_SINMAG_FRACT_BITS 10
#define BMTH_P_FIX_SIGNED_RAD_INT_BITS 3
#define BMTH_P_FIX_SIGNED_RAD_FRACT_BITS 28
#define BMTH_P_FIX_SIGNED_RAD_INTERP_INT_BITS 1
#define BMTH_P_FIX_SIGNED_RAD_INTERP_FRACT_BITS 15


/* creates a mask with signed bit plus integer bits and fraction bits */
#define BMTH_P_FIX_SIGNED_MASK(intbits, fractbits) \
	(0xFFFFFFFF >> (BMTH_P_FIX_SIGNED_MAX_BITS - (intbits + fractbits)))

/* creates signed bit mask of given fixed point format */
#define BMTH_P_FIX_SIGN_BIT(intbits, fractbits) \
	(1 << (intbits + fractbits))

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef BMTH_FIX_PRIV_H__ */
/* End of File */
