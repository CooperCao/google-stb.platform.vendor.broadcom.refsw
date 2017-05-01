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


void BMTH_HILO_64TO64_Neg_isrsafe(uint32_t xhi, uint32_t xlo, uint32_t *pouthi, uint32_t *poutlo)
{
	*poutlo = ~xlo + 1;
	*pouthi = ~xhi + ((*poutlo == 0) ? 1 : 0);
}

void BMTH_HILO_32TO64_Mul_isrsafe(uint32_t x, uint32_t y, uint32_t *pouthi, uint32_t *poutlo)
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

void BMTH_HILO_64TO64_Mul_isrsafe(uint32_t xhi, uint32_t xlo, uint32_t yhi, uint32_t ylo, uint32_t *pouthi, uint32_t *poutlo)
{
	BMTH_HILO_32TO64_Mul(xlo, ylo, pouthi, poutlo);
	*pouthi += (xhi * ylo) + (xlo * yhi);
}

void BMTH_HILO_64TO64_Add_isrsafe(uint32_t xhi, uint32_t xlo, uint32_t yhi, uint32_t ylo, uint32_t *pouthi, uint32_t *poutlo)
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

void BMTH_HILO_64TO64_Div32_isrsafe(uint32_t xhi, uint32_t xlo, uint32_t y, uint32_t *pouthi, uint32_t *poutlo)
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

uint32_t BMTH_2560log10_isrsafe(uint32_t x)
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
