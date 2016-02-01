/***************************************************************************
 *     (c)2012 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#include "bstd.h"
#include "btrc.h"
BTRC_MODULE(idct_simple,ENABLE);

typedef int16_t idct_elem;
typedef struct idct_block {
    idct_elem data[64];
} idct_block;

static const idct_block idct_simple_data[] = {
#include "idct_simple.h"
};

#define IDCT_W1  22725  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define IDCT_W2  21407  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define IDCT_W3  19266  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define IDCT_W4  16383  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define IDCT_W5  12873  //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define IDCT_W6  8867   //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define IDCT_W7  4520   //cos(i*M_PI/16)*sqrt(2)*(1<<14) + 0.5
#define ROW_SCALE 11

#define MUL *
void simple_idct_row(idct_elem *dst, const idct_elem *src)
{
    int a0, a1, a2, a3, b0, b1, b2, b3;
    int dc;

    dc = (IDCT_W4 MUL src[0]) + (1 << (ROW_SCALE - 1));

    a0 = dc + IDCT_W2 MUL src[2];
    a1 = dc + IDCT_W6 MUL src[2];
    a2 = dc - IDCT_W6 MUL src[2];
    a3 = dc - IDCT_W2 MUL src[2];

    b0 = IDCT_W1 MUL src[1] + IDCT_W3 MUL src[3];
    b1 = IDCT_W3 MUL src[1] - IDCT_W7 MUL src[3];
    b2 = IDCT_W5 MUL src[1] - IDCT_W1 MUL src[3];
    b3 = IDCT_W7 MUL src[1] - IDCT_W5 MUL src[3];

    a0 += IDCT_W4 MUL src[4] + IDCT_W6 MUL src[6];
    a1 -= IDCT_W4 MUL src[4] + IDCT_W2 MUL src[6];
    a2 -= IDCT_W4 MUL src[4] - IDCT_W2 MUL src[6];
    a3 += IDCT_W4 MUL src[4] - IDCT_W6 MUL src[6];

    b0 += IDCT_W5 MUL src[5] + IDCT_W7 MUL src[7];
    b1 -= IDCT_W1 MUL src[5] + IDCT_W5 MUL src[7];
    b2 += IDCT_W7 MUL src[5] + IDCT_W3 MUL src[7];
    b3 += IDCT_W3 MUL src[5] - IDCT_W1 MUL src[7];

    dst[0] = (a0 + b0) >> ROW_SCALE;
    dst[1] = (a1 + b1) >> ROW_SCALE;
    dst[2] = (a2 + b2) >> ROW_SCALE;
    dst[3] = (a3 + b3) >> ROW_SCALE;
    dst[4] = (a3 - b3) >> ROW_SCALE;
    dst[5] = (a2 - b2) >> ROW_SCALE;
    dst[6] = (a1 - b1) >> ROW_SCALE;
    dst[7] = (a0 - b0) >> ROW_SCALE;
    return;
}

static void chenwang_idct_row(idct_elem *dst, const idct_elem *src)
{
  int x0, x1, x2, x3, x4, x5, x6, x7, x8;

  x1 = src[4]<<11;
  x2 = src[6];
  x3 = src[2];
  x4 = src[1];
  x5 = src[7];
  x6 = src[5];
  x7 = src[3];

  x0 = (src[0]<<11) + 128; /* for proper rounding in the fourth stage */

  /* first stage */
  x8 = IDCT_W7*(x4+x5);
  x4 = x8 + (IDCT_W1-IDCT_W7)*x4;
  x5 = x8 - (IDCT_W1+IDCT_W7)*x5;
  x8 = IDCT_W3*(x6+x7);
  x6 = x8 - (IDCT_W3-IDCT_W5)*x6;
  x7 = x8 - (IDCT_W3+IDCT_W5)*x7;

  /* second stage */
  x8 = x0 + x1;
  x0 -= x1;
  x1 = IDCT_W6*(x3+x2);
  x2 = x1 - (IDCT_W2+IDCT_W6)*x2;
  x3 = x1 + (IDCT_W2-IDCT_W6)*x3;
  x1 = x4 + x6;
  x4 -= x6;
  x6 = x5 + x7;
  x5 -= x7;

  /* third stage */
  x7 = x8 + x3;
  x8 -= x3;
  x3 = x0 + x2;
  x0 -= x2;
  x2 = (181*(x4+x5)+128)>>8;
  x4 = (181*(x4-x5)+128)>>8;

  /* fourth stage */
  dst[0] = (x7+x1)>>8;
  dst[1] = (x3+x2)>>8;
  dst[2] = (x0+x4)>>8;
  dst[3] = (x8+x6)>>8;
  dst[4] = (x8-x6)>>8;
  dst[5] = (x0-x4)>>8;
  dst[6] = (x3-x2)>>8;
  dst[7] = (x7-x1)>>8;
  return;
}
void simple_idct_partial(idct_block *dst, const idct_block *src)
{
    unsigned i;
    for(i=0;i<8;i++) {
        simple_idct_row(&dst->data[i*8], &src->data[i*8]);
        //chenwang_idct_row(&dst->data[i*8], &src->data[i*8]);
    }
    return;
}

void idct_block_copy(idct_block *dst, const idct_block *src)
{
    unsigned i;
    for(i=0;i<sizeof(dst->data)/sizeof(dst->data[0]);i++) {
        dst->data[i] = src->data[i];
    }
    return;
}

bool idct_block_compare(const idct_block *result, const idct_block *test)
{
    unsigned i;
    for(i=0;i<sizeof(result->data)/sizeof(result->data[0]);i++) {
        if(result->data[i]!=test->data[i]) {
            BDBG_P_PrintString("%u: %d %d\n", i, result->data[i], test->data[i]);
            return false;
        }
    }
    return true;
}

#if B_HAS_BPROFILE
void run_idct_test(void)
{
    unsigned i,j;
    idct_block block;

	/* bunit_alu(); */
    for(i=0;i<sizeof(idct_simple_data)/sizeof(*idct_simple_data);i+=2) {
        for(j=0;j<1024;j++) {
            simple_idct_partial(&block, &idct_simple_data[i]);
        }
        /* idct_block_compare(&block, &idct_simple_data[i+1]); */
    }
    return;
}
#else
void run_idct_test(void)
{
    unsigned i,j;
    idct_block block;

	/* bunit_alu(); */
    for(i=0;i<sizeof(idct_simple_data)/sizeof(*idct_simple_data);i+=2) {
		BTRC_TRACE(idct_simple, START);
        for(j=0;j<1024;j++) {
            simple_idct_partial(&block, &idct_simple_data[i]);
        }
		BTRC_TRACE(idct_simple, STOP);
        /* idct_block_compare(&block, &idct_simple_data[i+1]); */
    }
	BTRC_Module_Report(BTRC_MODULE_HANDLE(idct_simple));
    return;
}
#endif


