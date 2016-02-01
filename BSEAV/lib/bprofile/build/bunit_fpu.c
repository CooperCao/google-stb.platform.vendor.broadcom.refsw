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
BTRC_MODULE(bunit_filter,ENABLE);

#define FIR_LENGTH  32
#define FILTER_SCALE 10
#define FILTER_COEFF(x) ((x)*(double)(1<<FILTER_SCALE))

typedef struct filter_float {
    float data[FIR_LENGTH];
} filter_float;

typedef struct filter_double {
    double data[FIR_LENGTH];
} filter_double;

typedef struct filter_fixed {
    int data[FIR_LENGTH];
} filter_fixed;


float
filter_float_simple(const filter_float *filter, const filter_float *mem)
{
    unsigned i;
    float d = 0.0;
    for(i=0;i<FIR_LENGTH;i++) {
        d+= filter->data[i]*mem->data[i];
    }
    return d;
}

float
filter_float_unrolled(const filter_float *filter, const filter_float *mem)
{
    unsigned i;
    float d = 0.0;
    for(i=0;i<FIR_LENGTH;i+=4) {
        d+= filter->data[i]*mem->data[i] +
            filter->data[i+1]*mem->data[i+1] +
            filter->data[i+2]*mem->data[i+2] +
            filter->data[i+3]*mem->data[i+3];
    }
    return d;
}

double
filter_double_simple(const filter_double *filter, const filter_double *mem)
{
    unsigned i;
    double d = 0.0;
    for(i=0;i<FIR_LENGTH;i++) {
        d+= filter->data[i]*mem->data[i];
    }
    return d;
}

double
filter_double_unrolled(const filter_double *filter, const filter_double *mem)
{
    unsigned i;
    double d = 0.0;
    for(i=0;i<FIR_LENGTH;i+=4) {
        d+= filter->data[i]*mem->data[i] +
            filter->data[i+1]*mem->data[i+1] +
            filter->data[i+2]*mem->data[i+2] +
            filter->data[i+3]*mem->data[i+3];
    }
    return d;
}

int
filter_fixed_simple(const filter_fixed *filter, const filter_fixed *mem)
{
    unsigned i;
    int d=0;
    for(i=0;i<FIR_LENGTH;i++) {
        d+= filter->data[i]*mem->data[i];
    }
    return d>>FILTER_SCALE;
}


int
filter_fixed_unrolled(const filter_fixed *filter, const filter_fixed *mem)
{
    unsigned i;
    int d = 0;
    for(i=0;i<FIR_LENGTH;i+=4) {
        d+= filter->data[i]*mem->data[i] +
            filter->data[i+1]*mem->data[i+1] +
            filter->data[i+2]*mem->data[i+2] +
            filter->data[i+3]*mem->data[i+3];
    }
    return d>>FILTER_SCALE;
}

int
filter_fixed_scaled_simple(const filter_fixed *filter, const filter_fixed *mem)
{
    unsigned i;
    int d=0;
    for(i=0;i<FIR_LENGTH;i++) {
        d+= (filter->data[i]*mem->data[i])>>FILTER_SCALE;
    }
    return d;
}

int
filter_fixed_scaled_unrolled(const filter_fixed *filter, const filter_fixed *mem)
{
    unsigned i;
    int d=0;
    for(i=0;i<FIR_LENGTH;i+=4) {
        d+= ((filter->data[i]*mem->data[i])>>FILTER_SCALE) +
            ((filter->data[i+1]*mem->data[i+1])>>FILTER_SCALE) +
            ((filter->data[i+2]*mem->data[i+2])>>FILTER_SCALE) +
            ((filter->data[i+3]*mem->data[i+3])>>FILTER_SCALE);
    }
    return d;
}

int
filter_fixed64_simple(const filter_fixed *filter, const filter_fixed *mem)
{
    unsigned i;
    int64_t d=0;
    for(i=0;i<FIR_LENGTH;i++) {
        d+= filter->data[i]*(int64_t)mem->data[i];
    }
    return d>>FILTER_SCALE;
}


int
filter_fixed64_unrolled(const filter_fixed *filter, const filter_fixed *mem)
{
    unsigned i;
    int64_t d = 0;
    for(i=0;i<FIR_LENGTH;i+=4) {
        d+= filter->data[i]*(int64_t)mem->data[i] +
            filter->data[i+1]*(int64_t)mem->data[i+1] +
            filter->data[i+2]*(int64_t)mem->data[i+2] +
            filter->data[i+3]*(int64_t)mem->data[i+3];
    }
    return d>>FILTER_SCALE;
}

int
filter_fixed64_scaled_simple(const filter_fixed *filter, const filter_fixed *mem)
{
    unsigned i;
    int64_t d=0;
    for(i=0;i<FIR_LENGTH;i++) {
        d+= (int)((filter->data[i]*(int64_t)mem->data[i])>>FILTER_SCALE);
    }
    return d;
}

int
filter_fixed64_scaled_unrolled(const filter_fixed *filter, const filter_fixed *mem)
{
    unsigned i;
    int d=0;
    for(i=0;i<FIR_LENGTH;i+=4) {
        d+= (int)(((filter->data[i]*(int64_t)mem->data[i])>>FILTER_SCALE)) +
            (int)(((filter->data[i+1]*(int64_t)mem->data[i+1])>>FILTER_SCALE)) +
            (int)(((filter->data[i+2]*(int64_t)mem->data[i+2])>>FILTER_SCALE)) +
            (int)(((filter->data[i+3]*(int64_t)mem->data[i+3])>>FILTER_SCALE));
    }
    return d;
}

static const filter_float sample_float = {{
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
}};

static const filter_double sample_double = {{
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 
    1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
}};

static const filter_fixed sample_fixed = {{
    FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0),
    FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0),
    FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0),
    FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0),
    FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0),
    FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0),
    FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0),
    FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0), FILTER_COEFF(1.0)
}};

static void
b_unit_filter_float_simple_run(void)
{
    filter_float_simple(&sample_float, &sample_float);
    return;
}

static void
b_unit_filter_float_unrolled_run(void)
{
    filter_float_unrolled(&sample_float, &sample_float);
    return;
}

static void
b_unit_filter_double_simple_run(void)
{
    filter_double_simple(&sample_double, &sample_double);
}

static void
b_unit_filter_double_unrolled_run(void)
{
    filter_double_unrolled(&sample_double, &sample_double);
}

static void
b_unit_filter_fixed_simple_run(void)
{
    filter_fixed_simple(&sample_fixed, &sample_fixed);
}

static void
b_unit_filter_fixed_unrolled_run(void)
{
    filter_fixed_unrolled(&sample_fixed, &sample_fixed);
}

static void
b_unit_filter_fixed_scaled_simple_run(void)
{
    filter_fixed_scaled_simple(&sample_fixed, &sample_fixed);
}

static void
b_unit_filter_fixed_scaled_unrolled_run(void)
{
    filter_fixed_scaled_unrolled(&sample_fixed, &sample_fixed);
}

static void
b_unit_filter_fixed64_simple_run(void)
{
    filter_fixed64_simple(&sample_fixed, &sample_fixed);
}

static void
b_unit_filter_fixed64_unrolled_run(void)
{
    filter_fixed64_unrolled(&sample_fixed, &sample_fixed);
}

static void
b_unit_filter_fixed64_scaled_simple_run(void)
{
    filter_fixed64_scaled_simple(&sample_fixed, &sample_fixed);
}

static void
b_unit_filter_fixed64_scaled_unrolled_run(void)
{
    filter_fixed64_scaled_unrolled(&sample_fixed, &sample_fixed);
}

static void 
b_unit_filter_run(const char *name, void (*test)(void))
{
    unsigned i;
    unsigned j;

    for(j=0;j<10;j++) {
        BTRC_TRACE(bunit_filter, START);
        for(i=0;i<1000;i++) {
            test();
        }
        BTRC_TRACE(bunit_filter, STOP);
    }
	BKNI_Printf("test '%s'\n", name);
	BTRC_Module_Report(BTRC_MODULE_HANDLE(bunit_filter));
	BTRC_Module_Reset(BTRC_MODULE_HANDLE(bunit_filter));
}

void
bunit_fpu(void)
{
    b_unit_filter_run("float_simple", b_unit_filter_float_simple_run);
    b_unit_filter_run("double_simple", b_unit_filter_double_simple_run);
    b_unit_filter_run("fixed_simple", b_unit_filter_fixed_simple_run);
    b_unit_filter_run("fixed_scaled_simple", b_unit_filter_fixed_scaled_simple_run);
    b_unit_filter_run("fixed64_simple", b_unit_filter_fixed64_simple_run);
    b_unit_filter_run("fixed64_scaled_simple", b_unit_filter_fixed64_scaled_simple_run);

    b_unit_filter_run("float_unrolled", b_unit_filter_float_unrolled_run);
    b_unit_filter_run("double_unrolled", b_unit_filter_double_unrolled_run);
    b_unit_filter_run("fixed_unrolled", b_unit_filter_fixed_unrolled_run);
    b_unit_filter_run("fixed_scaled_unrolled", b_unit_filter_fixed_scaled_unrolled_run);
    b_unit_filter_run("fixed64_unrolled", b_unit_filter_fixed64_unrolled_run);
    b_unit_filter_run("fixed64_scaled_unrolled", b_unit_filter_fixed64_scaled_unrolled_run);

    return;
}

