/*
 * ====================================================
 *
 * Portions of file Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * ====================================================
 */
/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
#include "bstd.h"
#include "bmth.h"
#include "bmth_fix.h"


/*******************************************************************************
*
*	This file was implemented using the code from previous project.  To
*	minimize changes, the code has not been reformated.
*	
*
*******************************************************************************/

/* BDBG_MODULE(bmth); enable when adding debug prints */


void BMTH_HILO_64TO64_Neg(uint32_t xhi, uint32_t xlo, uint32_t *pouthi, uint32_t *poutlo)
{
	*poutlo = ~xlo + 1;
	*pouthi = ~xhi + ((*poutlo == 0) ? 1 : 0);
}

void BMTH_HILO_32TO64_Mul(uint32_t x, uint32_t y, uint32_t *pouthi, uint32_t *poutlo)
{
	uint32_t xhi, xlo, yhi, ylo, ahi, alo, bhi, blo;

	xhi = (x >> 16) & 0xffff;
	xlo = x & 0xffff;
	yhi = (y >> 16) & 0xffff;
	ylo = y & 0xffff;

	*poutlo = xlo * ylo;

	BMTH_HILO_64TO64_Add(0, (xhi * ylo), 0, (yhi * xlo), &ahi, &alo);
	BMTH_HILO_64TO64_Add(0, ((*poutlo >> 16) & 0xffff), ahi, alo, &bhi, &blo);

	*poutlo = (*poutlo & 0xffff) | ((blo & 0xffff) << 16);
	*pouthi = ((blo >> 16) & 0xffff) | ((bhi &0xffff) << 16);
	*pouthi += xhi * yhi;
}

void BMTH_HILO_64TO64_Mul(uint32_t xhi, uint32_t xlo, uint32_t yhi, uint32_t ylo, uint32_t *pouthi, uint32_t *poutlo)
{
	BMTH_HILO_32TO64_Mul(xlo, ylo, pouthi, poutlo);
	*pouthi += (xhi * ylo) + (xlo * yhi);
}

void BMTH_HILO_64TO64_Add(uint32_t xhi, uint32_t xlo, uint32_t yhi, uint32_t ylo, uint32_t *pouthi, uint32_t *poutlo)
{
	uint32_t tempxlo = xlo;
	uint32_t tempxhi = xhi;
	uint32_t tempylo = ylo;
	uint32_t tempyhi = yhi;

	*poutlo = tempxlo + tempylo;
	*pouthi = tempxhi + tempyhi + (*poutlo < tempylo);
}

#if 0
void BMTH_HILO_64TO64_Div32(uint32_t xhi, uint32_t xlo, uint32_t y, uint32_t *pouthi, uint32_t *poutlo)
{
	uint32_t newxhi = xhi;
	uint32_t newxlo = xlo;
	uint32_t newq1 = 0;
	uint32_t r = 0;
	uint32_t curshift = 32; /* shift value of current 32 bits being divided */
	uint32_t rbits = 0;     /* remainder bits */
	uint32_t xlowbits = 0;  /* bits to concatenate from xlo with r to form new xhi*/
	*pouthi = 0;
	*poutlo = 0;
	uint32_t loop = 0;

	while (1)
	{
		newq1 = (newxhi / y);
		r = newxhi % y;

		BMTH_HILO_64TO64_Add(*pouthi, *poutlo,
			                 (curshift > 0)?  (newq1 >> (32 - curshift)) : 0,
							 (curshift < 32)? (newq1 << curshift) : 0,
							 pouthi, poutlo);

		if (curshift == 0)
		{
			break;
		}

		rbits = BMTH_FIX_LOG2(r) + 1;
		xlowbits = 32 - rbits;

		if (xlowbits > curshift)
		{
			xlowbits = curshift;
		}

		newxhi = (r << xlowbits) | (newxlo >> (32 - xlowbits));
		newxlo = newxlo << xlowbits;
		curshift -= xlowbits;
	}
	printf("loop %d\n", loop);
}
#else
#define BMTH_PRINTF(x)

void BMTH_HILO_64TO64_Div32(uint32_t xhi, uint32_t xlo, uint32_t y, uint32_t *pouthi, uint32_t *poutlo)
{
    /* this algo uses 64 bit add/subtract/bitshift, but not 64 bit multiply/divide */
    uint64_t xx = (uint64_t)xhi << 32 | xlo;
    uint64_t yy = y;
    uint64_t result = 0;

    if (y == 0) {
        *pouthi = 0;
        *poutlo = 0;
        return;
    }

    while (1) {
        uint64_t prev_yy = 0;
        uint64_t temp_yy = yy;
        uint64_t shift = 0;

        BMTH_PRINTF(("%llx / %llx = \n", xx, yy));
        while (xx >= temp_yy) {
            if (shift == 0)
                shift = 1;
            else
                shift <<= 1;
            prev_yy = temp_yy; /* if we fail the next test, prev_yy is it */
            temp_yy <<= 1;
        }
        if (!shift) break;

        BMTH_PRINTF(("%llx -= %llx, %llx += %llx\n", xx, prev_yy, result, shift));
        xx -= prev_yy;
        result += shift;
    }
    *pouthi = (uint32_t)(result >> 32);
    *poutlo = (uint32_t)(result & 0xFFFFFFFF);
    BMTH_PRINTF(("result = %08x%08x\n", *pouthi, *poutlo));
}
#endif

uint32_t BMTH_2560log10(uint32_t x)
{
  int32_t  x1;
  int32_t  x2;
  int32_t  x3;
  int32_t  x4;

  if (x == 0)
    return 0;

  x1 = 31;
  while (!((x >> x1) & 1))
    x1 = x1-1;
  x1 = x1+1;

  if (x1 > 20)
    x2 = (int32_t)(x >> (x1 - 8));
  else
    x2 = (int32_t)((x << 8) >> x1);
   
  x3 = -24381739 + x2*(62348 + (x2 << 7));
  x4 = 5907991 + (x2 << 16);

  return (unsigned)((770*(x3/(x4 >> 8) + (x1 << 8))) >> 8);
}
