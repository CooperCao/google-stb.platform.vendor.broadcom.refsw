/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#ifndef BMTH_FIX_H__
#define BMTH_FIX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bstd.h"
#include "bmth_fix_priv.h"

/***************************************************************************
Summary:
	convert one fixed point value to another
***************************************************************************/
uint32_t BMTH_FIX_SIGNED_CONVERT(uint32_t x, uint32_t inint, uint32_t infract, uint32_t outint, uint32_t outfract);

/***************************************************************************
Summary:
	convert fixed point to integer
***************************************************************************/
#define BMTH_FIX_SIGNED_FIXTOI(x, intbits, fractbits) \
	(((x) & BMTH_P_FIX_SIGN_BIT(intbits, fractbits)) ? \
		((x) >> (fractbits) | ~BMTH_P_FIX_SIGNED_MASK(intbits, fractbits)) : \
		((x) >> (fractbits)))

/***************************************************************************
Summary:
	convert integer to fixed point
***************************************************************************/
#define BMTH_FIX_SIGNED_ITOFIX(x, intbits, fractbits) \
	(((x) << fractbits) & BMTH_P_FIX_SIGNED_MASK(intbits, fractbits))

/***************************************************************************
Summary:
	convert float to fixed point
***************************************************************************/
#define BMTH_FIX_SIGNED_FTOFIX(x, intbits, fractbits) \
	((int32_t)(((x) * (1 << fractbits)) + ((x > 0) ? 0.5f : -0.5f)) & BMTH_P_FIX_SIGNED_MASK(intbits, fractbits))

/***************************************************************************
Summary:
	convert fixed to float point
***************************************************************************/
#define BMTH_FIX_SIGNED_FIXTOF(x, intbits, fractbits) \
	((int32_t)((BMTH_P_FIX_SIGN_BIT(intbits, fractbits) & x) ? \
		-((~BMTH_P_FIX_SIGN_BIT(intbits, fractbits) & ~x) + 1) : x ) / (float)(1 << fractbits))

/***************************************************************************
Summary:
	fixed point operation multiply
***************************************************************************/
uint32_t BMTH_FIX_SIGNED_MUL(uint32_t x, uint32_t y, uint32_t xint, uint32_t xfract, uint32_t yint, uint32_t yfract, uint32_t outint, uint32_t outfract);

/***************************************************************************
Summary:
	fixed point operation divide
***************************************************************************/
#define BMTH_FIX_SIGNED_DIV(x, y, xint, xfract, yint, yfract, outint, outfract) \
( \
	(uint32_t)(((outfract > ((xfract) - (yfract))) ? \
					((int32_t)BMTH_FIX_SIGNED_CONVERT(x, xint, xfract, BMTH_P_FIX_SIGNED_MAX_BITS - (xfract), xfract) << (outfract - (xfract - yfract))) : \
					((int32_t)BMTH_FIX_SIGNED_CONVERT(x, xint, xfract, BMTH_P_FIX_SIGNED_MAX_BITS - (xfract), xfract) >> ((xfract - yfract) - outfract))) / \
			   (int32_t)BMTH_FIX_SIGNED_CONVERT(y, yint, yfract, BMTH_P_FIX_SIGNED_MAX_BITS - yfract, yfract)) \
)


/***************************************************************************
Summary:
	fixed point operation modulus
***************************************************************************/
uint32_t BMTH_FIX_SIGNED_MOD(uint32_t x, uint32_t y, uint32_t xint, uint32_t xfract, uint32_t yint, uint32_t yfract, uint32_t outint, uint32_t outfract);


/***************************************************************************
Summary:
	fixed point log 2
***************************************************************************/
uint32_t BMTH_FIX_LOG2(uint32_t x);


/***************************************************************************
Summary:
	gets pi in fixed point format
****************************************************************************/
#define BMTH_FIX_SIGNED_GET_PI(intbits, fractbits) \
	 ((BMTH_P_FIX_UNSIGNED_PI >> (BMTH_P_FIX_UNSIGNED_RAD_FRACT_BITS - fractbits)) & \
	  BMTH_P_FIX_SIGNED_MASK(intbits, fractbits))


/***************************************************************************
Summary:
	gets pi/2 in fixed point format
****************************************************************************/
#define BMTH_FIX_SIGNED_GET_HALF_PI(intbits, fractbits) \
	(BMTH_FIX_SIGNED_GET_PI(intbits, fractbits) / 2)


/***************************************************************************
Summary:
	converts a radian to a value between -2 pi and 2 pi
****************************************************************************/
#define BMTH_FIX_SIGNED_RADTO2PI(x, intbits, fractbits, outint, outfract) \
	(BMTH_FIX_SIGNED_MOD(x, (2 * BMTH_FIX_SIGNED_GET_PI(outint, outfract)), \
	                     intbits, fractbits, \
                         BMTH_P_FIX_SIGNED_RAD_INT_BITS, BMTH_P_FIX_SIGNED_RAD_FRACT_BITS, \
                         outint, outfract))


/***************************************************************************
Summary:
	sin, with linear interpolation
****************************************************************************/
uint32_t BMTH_FIX_SIGNED_SIN(uint32_t x, uint32_t xint, uint32_t xfract, uint32_t sinint, uint32_t sinfract);


/***************************************************************************
Summary:
	cos, with linear interpolation
****************************************************************************/
uint32_t BMTH_FIX_SIGNED_COS(uint32_t x, uint32_t xint, uint32_t xfract, uint32_t sinint, uint32_t sinfract);


/***************************************************************************
Summary:
	finds the minimum number of bits needed to represent integer value
****************************************************************************/
uint32_t BMTH_P_FIX_SIGNED_MININTBITS(uint32_t x, uint32_t intbits, uint32_t fractbits);

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef BMTH_FIX_H__ */
/* End of File */
