/******************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bmth_fix.h"
#include "bstd.h"

/* sin magnitude lookup table in signed 2.10 notation */
static const int32_t s_lFixSinMagTable[] =
{
	0x0000, 0x000d, 0x0019, 0x0026, 0x0033, 0x003f, 0x004c, 0x0059,
	0x0065, 0x0072, 0x007e, 0x008b, 0x0097, 0x00a4, 0x00b0, 0x00bd,
	0x00c9, 0x00d6, 0x00e2, 0x00ee, 0x00fb, 0x0107, 0x0113, 0x011f,
	0x012b, 0x0138, 0x0144, 0x0150, 0x015b, 0x0167, 0x0173, 0x017f,
	0x018b, 0x0196, 0x01a2, 0x01ad, 0x01b9, 0x01c4, 0x01d0, 0x01db,
	0x01e6, 0x01f1, 0x01fc, 0x0207, 0x0212, 0x021d, 0x0228, 0x0232,
	0x023d, 0x0247, 0x0252, 0x025c, 0x0266, 0x0270, 0x027a, 0x0284,
	0x028e, 0x0297, 0x02a1, 0x02ab, 0x02b4, 0x02bd, 0x02c6, 0x02cf,
	0x02d8, 0x02e1, 0x02ea, 0x02f3, 0x02fb, 0x0303, 0x030c, 0x0314,
	0x031c, 0x0324, 0x032c, 0x0333, 0x033b, 0x0342, 0x0349, 0x0351,
	0x0358, 0x035f, 0x0365, 0x036c, 0x0372, 0x0379, 0x037f, 0x0385,
	0x038b, 0x0391, 0x0396, 0x039c, 0x03a1, 0x03a7, 0x03ac, 0x03b1,
	0x03b6, 0x03ba, 0x03bf, 0x03c3, 0x03c7, 0x03cb, 0x03cf, 0x03d3,
	0x03d7, 0x03da, 0x03de, 0x03e1, 0x03e4, 0x03e7, 0x03e9, 0x03ec,
	0x03ee, 0x03f1, 0x03f3, 0x03f5, 0x03f6, 0x03f8, 0x03fa, 0x03fb,
	0x03fc, 0x03fd, 0x03fe, 0x03ff, 0x03ff, 0x0400, 0x0400, 0x0400
};


/*************************************************************************
 * BMTH_FIX_SIGNED_CONVERT_isrsafe
 *
 *************************************************************************/
uint32_t BMTH_FIX_SIGNED_CONVERT_isrsafe(uint32_t x, uint32_t inint, uint32_t infract, uint32_t outint, uint32_t outfract)
{
	uint32_t lFixOut;
	uint32_t ulFixInt;
	uint32_t ulSignBit;

	ulFixInt = (infract > outfract) ? (x >> (infract - outfract)) : (x << (outfract - infract));
	if(inint > outint)
	{
		ulSignBit = 0;
	}
	else
	{
		if(x & BMTH_P_FIX_SIGN_BIT(inint, infract))
		{
			ulSignBit = ~BMTH_P_FIX_SIGNED_MASK(inint, outfract);
		}
		else
		{
			ulSignBit = 0;
		}
	}
	lFixOut = (ulFixInt | ulSignBit) & BMTH_P_FIX_SIGNED_MASK(outint, outfract);

	return lFixOut;
}

/*************************************************************************
 * BMTH_FIX_SIGNED_CONVERT_64
 *
 *************************************************************************/
int64_t BMTH_FIX_SIGNED_CONVERT_64_isrsafe(int64_t x, uint32_t infract, uint32_t outfract)
{
	int64_t lFixOut;

	lFixOut = (infract > outfract) ? (x >> (infract - outfract)) : (x << (outfract - infract));
	return lFixOut;
}

/*************************************************************************
 * BMTH_FIX_SIGNED_MUL
 *
 *************************************************************************/
uint32_t BMTH_FIX_SIGNED_MUL_isrsafe(uint32_t x, uint32_t y, uint32_t xint, uint32_t xfract, uint32_t yint, uint32_t yfract, uint32_t outint, uint32_t outfract)
{
	int64_t lFixOut; /* 64-bit to hold intermediate multiplied value of two 32-bit values */

#if 0
	/* used to debug overflows */
	int32_t lhi, llo;
	int32_t lExtendedX = (int32_t)BMTH_FIX_SIGNED_CONVERT_isrsafe(x, xint, xfract, BMTH_P_FIX_SIGNED_MAX_BITS - xfract, xfract);
	int32_t lExtendedY = (int32_t)BMTH_FIX_SIGNED_CONVERT_isrsafe(y, yint, yfract, BMTH_P_FIX_SIGNED_MAX_BITS - yfract, yfract);
	int32_t lSignMask = BMTH_P_FIX_SIGN_BIT(BMTH_P_FIX_SIGNED_MAX_BITS, 0);

	BMTH_HILO_32TO64_Mul (lExtendedX, lExtendedY, &lhi, &llo);

	if (lhi)
	{
		if ((!(lExtendedX & lSignMask) && !(lExtendedY & lSignMask)) ||
			(!(llo & lSignMask) && (((lExtendedX & lSignMask) && !(lExtendedY & lSignMask)) ||
			                         (!(lExtendedX & lSignMask) && (lExtendedY & lSignMask)))) ||
			((llo & lSignMask) && (lExtendedX & lSignMask) && (lExtendedY & lSignMask)))
		{
			printf("BMTH_FIX_SIGNED_MUL OVERFLOW! x: 0x%x, y: 0x%x, lhi: 0x%x, llo: 0x%x\n", lExtendedX, lExtendedY, lhi, llo);
		}
	}

	if(outfract > (xfract + yfract))
	{
		lFixOut = llo  << (outfract - (xfract + yfract));
	}
	else
	{
		lFixOut = llo >> ((xfract + yfract) - outfract);
	}

#else
	if(outfract > (xfract + yfract))
	{
		lFixOut = ((int64_t)(int32_t)BMTH_FIX_SIGNED_CONVERT_isrsafe(x, xint, xfract, BMTH_P_FIX_SIGNED_MAX_BITS - xfract, xfract) *
				            (int32_t)BMTH_FIX_SIGNED_CONVERT_isrsafe(y, yint, yfract, BMTH_P_FIX_SIGNED_MAX_BITS - yfract, yfract))
				  << (outfract - (xfract + yfract));
	}
	else
	{
		lFixOut = ((int64_t)(int32_t)BMTH_FIX_SIGNED_CONVERT_isrsafe(x, xint, xfract, BMTH_P_FIX_SIGNED_MAX_BITS - xfract, xfract) *
				            (int32_t)BMTH_FIX_SIGNED_CONVERT_isrsafe(y, yint, yfract, BMTH_P_FIX_SIGNED_MAX_BITS - yfract, yfract))
				  >> ((xfract + yfract) - outfract);
	}

#endif

	return (uint32_t)(lFixOut & (int32_t)BMTH_P_FIX_SIGNED_MASK(outint, outfract));
}

/*************************************************************************
 * BMTH_FIX_SIGNED_MUL_64
 *
 *************************************************************************/
int64_t BMTH_FIX_SIGNED_MUL_64_isrsafe(int64_t x, int64_t y, uint32_t xfract, uint32_t yfract, uint32_t outfract)
{
	int64_t lFixOut;

	if(outfract > (xfract + yfract))
	{
		lFixOut = ((int64_t)BMTH_FIX_SIGNED_CONVERT_64_isrsafe(x, xfract, xfract) *
				            BMTH_FIX_SIGNED_CONVERT_64_isrsafe(y, yfract, yfract))
				  << (outfract - (xfract + yfract));
	}
	else
	{
		lFixOut = ((int64_t)BMTH_FIX_SIGNED_CONVERT_64_isrsafe(x, xfract, xfract) *
				            BMTH_FIX_SIGNED_CONVERT_64_isrsafe(y, yfract, yfract))
				  >> ((xfract + yfract) - outfract);
	}

	return lFixOut;
}

/*************************************************************************
 * BMTH_FIX_LOG2
 *
 *************************************************************************/
uint32_t BMTH_FIX_LOG2(uint32_t x)
{
	const uint32_t bitarray[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
	const uint32_t shiftarray[] = {1, 2, 4, 8, 16};
	int32_t i = 0;
	uint32_t log = 0;
	for (i = 4; i >= 0; i--)
	{
		if (x & bitarray[i])
		{
			x >>= shiftarray[i];
			log |= shiftarray[i];
		}
	}
	return log;
}

/*************************************************************************
 * BMTH_P_FIX_SIGNED_MININTBITS
 *
 *************************************************************************/
uint32_t BMTH_P_FIX_SIGNED_MININTBITS(uint32_t x, uint32_t intbits, uint32_t fractbits)
{
	uint32_t ulMinIntBits;
	int32_t ulFixInput = BMTH_FIX_SIGNED_CONVERT_isrsafe(x, intbits, fractbits, BMTH_P_FIX_SIGNED_MAX_BITS, 0);

	if(x & BMTH_P_FIX_SIGN_BIT(intbits, fractbits))
	{
		ulMinIntBits = BMTH_FIX_LOG2((uint32_t)((int32_t)ulFixInput * (-1)));
	}
	else
	{
		ulMinIntBits = BMTH_FIX_LOG2(ulFixInput) + 1;
	}
	return ulMinIntBits;
}


/*************************************************************************
 * BMTH_FIX_SIGNED_MOD
 *
 *************************************************************************/
uint32_t BMTH_FIX_SIGNED_MOD(uint32_t x, uint32_t y, uint32_t xint, uint32_t xfract, uint32_t yint, uint32_t yfract, uint32_t outint, uint32_t outfract)
{
	int32_t  lFixX;
	int32_t  lFixY;
	int32_t  lFixMod;
	uint32_t ulTempIntBits = 0;
	uint32_t ulTempFractBits = 0;

#if 1
	uint32_t ulTempXIntBits = 0;
	uint32_t ulTempYIntBits = 0;

	ulTempXIntBits = BMTH_P_FIX_SIGNED_MININTBITS(x, xint, xfract);
	ulTempYIntBits = BMTH_P_FIX_SIGNED_MININTBITS(y, yint, yfract);
	ulTempIntBits = (ulTempXIntBits > ulTempYIntBits) ? ulTempXIntBits : ulTempYIntBits;

	if (ulTempIntBits < outint)
	{
		ulTempIntBits = outint;
	}
#else
	ulTempIntBits = outint;
#endif

	ulTempFractBits = BMTH_P_FIX_SIGNED_MAX_BITS - ulTempIntBits;

	/* convert to common format */
	lFixX = BMTH_FIX_SIGNED_CONVERT_isrsafe(x, xint, xfract,
		                            ulTempIntBits, ulTempFractBits);
	lFixY = BMTH_FIX_SIGNED_CONVERT_isrsafe(y, yint, yfract,
		                            ulTempIntBits, ulTempFractBits);
	lFixMod = lFixX % lFixY;

	return BMTH_FIX_SIGNED_CONVERT_isrsafe(lFixMod, ulTempIntBits, ulTempFractBits, outint, outfract);
}


/***************************************************************************
 * {private}
 *
 */
uint32_t BMTH_P_FIX_SIGNED_SIN_CALC(uint32_t x, uint32_t xint, uint32_t xfract, uint32_t sinint, uint32_t sinfract)
{
	uint32_t ulRadIntBits      = xint;
	uint32_t ulRadFractBits    = xfract;
	uint32_t ulSinMagIntBits   = BMTH_P_FIX_SIGNED_SINMAG_INT_BITS;
	uint32_t ulSinMagFractBits = BMTH_P_FIX_SIGNED_SINMAG_FRACT_BITS;
	uint32_t ulInterpIntBits   = BMTH_P_FIX_SIGNED_RAD_INTERP_INT_BITS;
	uint32_t ulInterpFractBits = BMTH_P_FIX_SIGNED_RAD_INTERP_FRACT_BITS;

	int32_t ulFixX = x;
	int32_t ulFixXModPi;
	int32_t ulFixFirstQuadX;
	int32_t ulFixPi = BMTH_FIX_SIGNED_GET_PI(ulRadIntBits, ulRadFractBits);
	int32_t ulFixHalfPi = ulFixPi / 2;

	uint32_t ulRadShiftBits = 0;
	uint32_t ulSinMagTableSizeBits = 0;
	uint32_t ulTotalBits = 0;
	uint32_t ulSinMagIdx = 0;
	int32_t ulFixSinMag = 0;
	int32_t ulFixSinMagNext = 0;

	int32_t ulFixInterpNum = 0;
	int32_t ulFixInterpDem = 0;
	uint32_t ulFixInterpCoeff = 0;


	/* convert to positive angle */
	ulFixX = (ulFixX & BMTH_P_FIX_SIGN_BIT(ulRadIntBits, ulRadFractBits)) ?
			 (ulFixX + (2 * ulFixPi)) : ulFixX;
	ulFixX &= BMTH_P_FIX_SIGNED_MASK(ulRadIntBits, ulRadFractBits);

	ulFixXModPi = BMTH_FIX_SIGNED_MOD(ulFixX, ulFixPi,
	                                    ulRadIntBits, ulRadFractBits,
	                                    ulRadIntBits, ulRadFractBits,
	                                    ulRadIntBits, ulRadFractBits);

	/* convert to first quadrant equivalent with same sin value */
	ulFixFirstQuadX = (ulFixXModPi <= ulFixHalfPi) ?
			ulFixXModPi : (ulFixPi - ulFixXModPi);

	/* get sin magnitude table index */
	ulSinMagTableSizeBits = BMTH_P_FIX_SIGNED_MININTBITS((BMTH_P_FIX_SINMAG_TABLE_SIZE - 1), BMTH_P_FIX_SIGNED_MAX_BITS, 0);
	ulTotalBits = ulRadIntBits + ulRadFractBits + ulSinMagTableSizeBits;

	if (ulTotalBits > BMTH_P_FIX_SIGNED_MAX_BITS)
	{
		ulRadShiftBits = ulTotalBits - BMTH_P_FIX_SIGNED_MAX_BITS;
	}

	ulSinMagIdx = ((ulFixFirstQuadX >> ulRadShiftBits) * (BMTH_P_FIX_SINMAG_TABLE_SIZE - 1)) /
		           (ulFixHalfPi >> ulRadShiftBits);
	ulFixSinMag = s_lFixSinMagTable[ulSinMagIdx];
	ulFixSinMagNext = (ulSinMagIdx < (BMTH_P_FIX_SINMAG_TABLE_SIZE - 1)) ?
		(s_lFixSinMagTable[ulSinMagIdx + 1]) : (s_lFixSinMagTable[ulSinMagIdx]);


	ulFixInterpNum = ulFixFirstQuadX - (((ulSinMagIdx * (ulFixHalfPi >> ulRadShiftBits)) /
	                                     (BMTH_P_FIX_SINMAG_TABLE_SIZE - 1)) << ulRadShiftBits);
	ulFixInterpDem = ulFixHalfPi / BMTH_P_FIX_SINMAG_TABLE_SIZE;


	ulFixInterpNum = BMTH_FIX_SIGNED_CONVERT_isrsafe(ulFixInterpNum,
											 ulRadIntBits, ulRadFractBits,
											 ulInterpIntBits, ulInterpFractBits);

	ulFixInterpDem = BMTH_FIX_SIGNED_CONVERT_isrsafe(ulFixInterpDem,
											 ulRadIntBits, ulRadFractBits,
											 ulInterpIntBits, ulInterpFractBits);

	ulFixInterpCoeff = BMTH_FIX_SIGNED_DIV(ulFixInterpNum, ulFixInterpDem,
								           ulInterpIntBits, ulInterpFractBits,
								           ulInterpIntBits, ulInterpFractBits,
								           ulInterpIntBits, ulInterpFractBits);

	ulFixSinMag = BMTH_FIX_SIGNED_CONVERT_isrsafe(ulFixSinMag,
										  ulSinMagIntBits, ulSinMagFractBits,
										  ulInterpIntBits, ulInterpFractBits);

	ulFixSinMagNext = BMTH_FIX_SIGNED_CONVERT_isrsafe(ulFixSinMagNext,
											  ulSinMagIntBits, ulSinMagFractBits,
											  ulInterpIntBits, ulInterpFractBits);

	return (BMTH_FIX_SIGNED_CONVERT_isrsafe(ulFixSinMag,
									ulInterpIntBits, ulInterpFractBits,
									sinint, sinfract) +
		    BMTH_FIX_SIGNED_MUL_isrsafe((ulFixSinMagNext - ulFixSinMag), ulFixInterpCoeff,
							    ulInterpIntBits, ulInterpFractBits,
								ulInterpIntBits, ulInterpFractBits,
	                            sinint, sinfract)) *
		   ((ulFixX > ulFixPi) ? -1 : 1);  /* add the sign back */
}


/*************************************************************************
 * BMTH_FIX_SIGNED_COS
 *
 *************************************************************************/
uint32_t BMTH_FIX_SIGNED_COS_isrsafe(uint32_t x, uint32_t xint, uint32_t xfract, uint32_t sinint, uint32_t sinfract)
{
	uint32_t ulFixX;
	uint32_t ulRadIntBits      = BMTH_P_FIX_SIGNED_RAD_INT_BITS;
	uint32_t ulRadFractBits    = BMTH_P_FIX_SIGNED_RAD_FRACT_BITS;

	ulFixX = BMTH_FIX_SIGNED_RADTO2PI(x, xint, xfract, ulRadIntBits, ulRadFractBits);
	ulFixX += BMTH_FIX_SIGNED_GET_HALF_PI(ulRadIntBits, ulRadFractBits);

	return BMTH_P_FIX_SIGNED_SIN_CALC(ulFixX, ulRadIntBits, ulRadFractBits, sinint, sinfract);
}


/*************************************************************************
 * BMTH_FIX_SIGNED_SIN
 *
 *************************************************************************/
uint32_t BMTH_FIX_SIGNED_SIN_isrsafe(uint32_t x, uint32_t xint, uint32_t xfract, uint32_t sinint, uint32_t sinfract)
{
	uint32_t ulFixX;
	uint32_t ulRadIntBits      = BMTH_P_FIX_SIGNED_RAD_INT_BITS;
	uint32_t ulRadFractBits    = BMTH_P_FIX_SIGNED_RAD_FRACT_BITS;

	ulFixX = BMTH_FIX_SIGNED_RADTO2PI(x, xint, xfract, ulRadIntBits, ulRadFractBits);

	return BMTH_P_FIX_SIGNED_SIN_CALC(ulFixX, ulRadIntBits, ulRadFractBits, sinint, sinfract);
}


/* end of file */
