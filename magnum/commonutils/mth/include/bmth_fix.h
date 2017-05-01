/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 ******************************************************************************/
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
uint32_t BMTH_FIX_SIGNED_CONVERT_isrsafe(uint32_t x, uint32_t inint, uint32_t infract, uint32_t outint, uint32_t outfract);

#define BMTH_FIX_SIGNED_CONVERT(x, inint, infract, outint, outfract)  \
	BMTH_FIX_SIGNED_CONVERT_isrsafe(x, inint, infract, outint, outfract)
/***************************************************************************
Summary:
	convert one fixed point value to another
***************************************************************************/
int64_t BMTH_FIX_SIGNED_CONVERT_64_isrsafe(int64_t x, uint32_t infract, uint32_t outfract);

#define BMTH_FIX_SIGNED_CONVERT_64(x, infract, outfract)  \
	BMTH_FIX_SIGNED_CONVERT_64_isrsafe(x, infract, outfract)

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
uint32_t BMTH_FIX_SIGNED_MUL_isrsafe(uint32_t x, uint32_t y, uint32_t xint, uint32_t xfract, uint32_t yint, uint32_t yfract, uint32_t outint, uint32_t outfract);

#define BMTH_FIX_SIGNED_MUL(x, y, xint, xfract, yint, yfract, outint, outfract)   \
	BMTH_FIX_SIGNED_MUL_isrsafe(x, y, xint, xfract, yint, yfract, outint, outfract)
/***************************************************************************
Summary:
	fixed point operation multiply
***************************************************************************/
int64_t BMTH_FIX_SIGNED_MUL_64_isrsafe(int64_t x, int64_t y, uint32_t xfract, uint32_t yfract, uint32_t outfract);
#define BMTH_FIX_SIGNED_MUL_64(x, y, xint, xfract, yfract, outfract)   \
	BMTH_FIX_SIGNED_MUL_64_isrsafe(x, y, xint, xfract, yfract, outfract)

/***************************************************************************
Summary:
	fixed point operation divide
***************************************************************************/
#define BMTH_FIX_SIGNED_DIV(x, y, xint, xfract, yint, yfract, outint, outfract) \
( \
	(uint32_t)(((outfract > ((xfract) - (yfract))) ? \
					((int32_t)BMTH_FIX_SIGNED_CONVERT_isrsafe(x, xint, xfract, BMTH_P_FIX_SIGNED_MAX_BITS - (xfract), xfract) << (outfract - (xfract - yfract))) : \
					((int32_t)BMTH_FIX_SIGNED_CONVERT_isrsafe(x, xint, xfract, BMTH_P_FIX_SIGNED_MAX_BITS - (xfract), xfract) >> ((xfract - yfract) - outfract))) / \
			   (int32_t)BMTH_FIX_SIGNED_CONVERT_isrsafe(y, yint, yfract, BMTH_P_FIX_SIGNED_MAX_BITS - yfract, yfract)) \
)


/***************************************************************************
Summary:
	fixed point operation divide
***************************************************************************/
#define BMTH_FIX_SIGNED_DIV_64(x, y, xfract, yfract, outfract) \
( \
	(uint64_t)(((outfract > ((xfract) - (yfract))) ? \
					((int64_t)BMTH_FIX_SIGNED_CONVERT_64_isrsafe(x, xfract, xfract) << (outfract - (xfract - yfract))) : \
					((int64_t)BMTH_FIX_SIGNED_CONVERT_64_isrsafe(x, xfract, xfract) >> ((xfract - yfract) - outfract))) / \
			   (int64_t)BMTH_FIX_SIGNED_CONVERT_64_isrsafe(y, yfract, yfract)) \
)


/***************************************************************************
Summary:
	fixed point operation modulus
***************************************************************************/
uint32_t BMTH_FIX_SIGNED_MOD_isrsafe(uint32_t x, uint32_t y, uint32_t xint, uint32_t xfract, uint32_t yint, uint32_t yfract, uint32_t outint, uint32_t outfract);

#define BMTH_FIX_SIGNED_MOD(x, y, xint, xfract, yint, yfract, outint, outfract)   \
	BMTH_FIX_SIGNED_MOD_isrsafe(x, y, xint, xfract, yint, yfract, outint, outfract)


/***************************************************************************
Summary:
	fixed point operation modulus
***************************************************************************/
int64_t BMTH_FIX_SIGNED_MOD_64_isrsafe(int64_t x, int64_t y, uint32_t xint, uint32_t xfract, uint32_t yint, uint32_t yfract, uint32_t outint, uint32_t outfract);

#define BMTH_FIX_SIGNED_MOD_64(x, y, xint, xfract, yint, yfract, outint, outfract)   \
	BMTH_FIX_SIGNED_MOD_64_isrsafe(x, y, xint, xfract, yint, yfract, outint, outfract)


/***************************************************************************
Summary:
	fixed point log 2
***************************************************************************/
uint32_t BMTH_FIX_LOG2_isrsafe(uint32_t x);
#define BMTH_FIX_LOG2(x)      \
	BMTH_FIX_LOG2_isrsafe(x)


/***************************************************************************
Summary:
	fixed point log 2
***************************************************************************/
uint32_t BMTH_FIX_LOG2_64_isrsafe(uint64_t x);
#define BMTH_FIX_LOG2_64(x)      \
	BMTH_FIX_LOG2_64_isrsafe(x)


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
	converts a radian to a value between -2 pi and 2 pi
****************************************************************************/
#define BMTH_FIX_SIGNED_RADTO2PI_64(x, intbits, fractbits, outint, outfract) \
	(BMTH_FIX_SIGNED_MOD_64(x, (2 * BMTH_FIX_SIGNED_GET_PI(outint, outfract)), \
	                     intbits, fractbits, \
                         BMTH_P_FIX_SIGNED_RAD_INT_BITS, BMTH_P_FIX_SIGNED_RAD_FRACT_BITS, \
                         outint, outfract))


/***************************************************************************
Summary:
	sin, with linear interpolation
****************************************************************************/
uint32_t BMTH_FIX_SIGNED_SIN_isrsafe(uint32_t x, uint32_t xint, uint32_t xfract, uint32_t sinint, uint32_t sinfract);
#define BMTH_FIX_SIGNED_SIN(x, xint, xfract, sinint, sinfract)   \
	BMTH_FIX_SIGNED_SIN_isrsafe(x, xint, xfract, sinint, sinfract)


/***************************************************************************
Summary:
	sin, with linear interpolation
****************************************************************************/
int64_t BMTH_FIX_SIGNED_SIN_64_isrsafe(int64_t x, uint32_t xint, uint32_t xfract, uint32_t sinfract);
#define BMTH_FIX_SIGNED_SIN_64(x, xint, xfract, sinfract)   \
	BMTH_FIX_SIGNED_SIN_64_isrsafe(x, xint, xfract, sinfract)


/***************************************************************************
Summary:
	cos, with linear interpolation
****************************************************************************/
uint32_t BMTH_FIX_SIGNED_COS_isrsafe(uint32_t x, uint32_t xint, uint32_t xfract, uint32_t sinint, uint32_t sinfract);
#define BMTH_FIX_SIGNED_COS(x, xint, xfract, sinint, sinfract)   \
	BMTH_FIX_SIGNED_COS_isrsafe(x, xint, xfract, sinint, sinfract)


/***************************************************************************
Summary:
	cos, with linear interpolation
****************************************************************************/
int64_t BMTH_FIX_SIGNED_COS_64_isrsafe(int64_t x, uint32_t xint, uint32_t xfract, uint32_t sinfract);
#define BMTH_FIX_SIGNED_COS_64(x, xint, xfract, sinfract)   \
	BMTH_FIX_SIGNED_COS_64_isrsafe(x, xint, xfract, sinfract)


/***************************************************************************
Summary:
	finds the minimum number of bits needed to represent integer value
****************************************************************************/
uint32_t BMTH_P_FIX_SIGNED_MININTBITS_isrsafe(uint32_t x, uint32_t intbits, uint32_t fractbits);

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef BMTH_FIX_H__ */
/* End of File */
