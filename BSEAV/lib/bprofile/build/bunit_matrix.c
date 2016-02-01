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
#include "bkni.h"

BTRC_MODULE(bunit_matrix,ENABLE);

#define FILTER_SCALE 10
#define FILTER_COEFF(x) ((x)*(double)(1<<FILTER_SCALE))
#define FILTER_ROW(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11) {FILTER_COEFF(x0),FILTER_COEFF(x1),FILTER_COEFF(x2),FILTER_COEFF(x3),FILTER_COEFF(x4),FILTER_COEFF(x5),FILTER_COEFF(x6),FILTER_COEFF(x7),FILTER_COEFF(x8),FILTER_COEFF(x9),FILTER_COEFF(x10),FILTER_COEFF(x11)}

typedef struct matrix_12x12 {
    uint16_t data[12][12];
} matrix_12x12;

typedef struct row_12 {
    uint16_t data[12];
} row_12;

static const matrix_12x12 filter = {{
    FILTER_ROW(  1 , 0.5,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  0 ),
    FILTER_ROW( 0.5,  1 , 0.5,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  0 ),
    FILTER_ROW(  1 , 0.5,  1 , 0.5,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  0 ),
    FILTER_ROW(  1 ,  1 , 0.5,  1 , 0.5,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  0 ),
    FILTER_ROW(  1 ,  1 ,  1 , 0.5,  1 , 0.5,  1 ,  1 ,  1 ,  1 ,  1 ,  0 ),
    FILTER_ROW(  1 ,  1 ,  1 ,  1 , 0.5,  1 , 0.5,  1 ,  1 ,  1 ,  1 ,  0 ),
    FILTER_ROW(  1 ,  1 ,  1 ,  1 ,  1 , 0.5,  1 , 0.5,  1 ,  1 ,  1 ,  0 ),
    FILTER_ROW(  1 ,  1 ,  1 ,  1 ,  1 ,  1 , 0.5,  1 , 0.5,  1 ,  1 ,  0 ),
    FILTER_ROW(  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 , 0.5,  1 , 0.5,  1 ,  0 ),
    FILTER_ROW(  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 , 0.5,  1 , 0.5,  0 ),
    FILTER_ROW(  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 , 0.5,  1 , 0.5),
    FILTER_ROW(  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 ,  1 , 0.5,  1 )
}};


void 
simple_mult(const matrix_12x12 *matrix, const row_12 *src, uint32_t *dst)
{
    unsigned i,j;
    for(i=0;i<12;i++) {
        uint32_t acc;
        for(acc=0,j=0;j<12;j++) {
            acc += matrix->data[i][j] * (uint32_t)src->data[j];
        }
        dst[i] = acc;
    }
    return;
}

void 
unrolled_mult(const matrix_12x12 *matrix, const row_12 *src, uint32_t *dst)
{
    unsigned i;
    for(i=0;i<12;i++) {
        const uint16_t *row = matrix->data[i];
        dst[i] = 
        row[0] * (uint32_t)src->data[0] +
        row[1] * (uint32_t)src->data[1] +
        row[2] * (uint32_t)src->data[2] +
        row[3] * (uint32_t)src->data[3] +
        row[4] * (uint32_t)src->data[4] +
        row[5] * (uint32_t)src->data[4] +
        row[6] * (uint32_t)src->data[6] +
        row[7] * (uint32_t)src->data[7] +
        row[8] * (uint32_t)src->data[8] +
        row[9] * (uint32_t)src->data[9] +
        row[10] * (uint32_t)src->data[10] +
        row[11] * (uint32_t)src->data[11];
    }
    return;
}

static void 
b_unit_matrix_run(const char *name, void (*test)(const matrix_12x12 *matrix, const row_12 *src, uint32_t *dst))
{
    unsigned i;
    static const row_12 src={{1,1,1,1, 1,1,1,1, 1,1,1,1}};
    uint32_t result[12];

    BTRC_TRACE(bunit_matrix, START);
    for(i=0;i<1000;i++) {
        test(&filter, &src, result);
    }
    BTRC_TRACE(bunit_matrix, STOP);
	BKNI_Printf("test '%s'\n", name);
	BTRC_Module_Report(BTRC_MODULE_HANDLE(bunit_matrix));
	BTRC_Module_Reset(BTRC_MODULE_HANDLE(bunit_matrix));
}

void
bunit_matrix(void)
{
    b_unit_matrix_run("simple_mult", simple_mult);
    b_unit_matrix_run("unrolled_mult", unrolled_mult);
    return;
}

