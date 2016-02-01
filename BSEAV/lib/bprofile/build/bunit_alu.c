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

BDBG_MODULE(bunit_alu);

BTRC_MODULE(bunit_alu,ENABLE);

BSTD_INLINE unsigned b_mult(unsigned a, unsigned b)
{
    unsigned c;
	__asm__ __volatile__(
		"	mul %0, %1, %2				\n"
		: "=r" (c)
		: "Ir" (a), "Ir" (b)
		);
    return c;
}
#define b_nop() __asm__ __volatile__( \
		".set\tpush\n\t"  \
		".set\tnoreorder\n\t"       \
		"	nop	\n"  \
		".set\tpop\n")

#define b_nop2() __asm__ __volatile__( \
		".set\tpush\n\t"  \
		".set\tnoreorder\n\t"       \
		"	nop	\n" \
		"	nop	\n"  \
		".set\tpop\n")

#define b_nop3() __asm__ __volatile__( \
		".set\tpush\n\t"  \
		"	nop	\n" \
		"	nop	\n" \
		"	nop	\n"  \
		".set\tpop\n")

#define b_nop4() b_nop2();b_nop2()
#define b_nop6() b_nop2();b_nop2();b_nop2()

static unsigned 
b_unit_mult_0_test(unsigned a,unsigned b)
{
    unsigned i;
    unsigned c=a;
    for(i=0;i<256;i++) {
        c = b_mult(c, b);
        c = b_mult(c, a);
        c = b_mult(c, b);
        c = b_mult(c, a);
        c = b_mult(c, b);
        c = b_mult(c, a);
        c = b_mult(c, b);
        c = b_mult(c, a);
    }
    return c;
}

static unsigned 
b_unit_mult_pipe_test(unsigned a,unsigned b)
{
    unsigned i;
    unsigned c=a;
    for(i=0;i<256;i++) {
        unsigned t0,t1,t2,t3,t4,t5,t6;
        t0 = b_mult(c, b);
        t1 = b_mult(c, a);
        t2 = b_mult(c, b);
        t3 = b_mult(c, a);
        t4 = b_mult(c, b);
        t5 = b_mult(t0, t1);
        t6 = b_mult(t2, t3);
        c = b_mult(t4, t5);
    }
    return c;
}

static unsigned 
b_unit_mult_pipe2_test(unsigned a,unsigned b)
{
    unsigned i;
    unsigned c=a;
    for(i=0;i<256;i++) {
        unsigned t0,t1,t2,t3,t4,t5,t6;
        t0 = b_mult(c, b);
        t1 = b_mult(c, a);
        t2 = b_mult(c, b);
        b_nop2();
        c = b_mult(a, a);
        a = b_mult(b, b);
        b = b_mult(t0, t0);
        b_nop2();
        t0 = b_mult(t1, t1);
        t1 = b_mult(t2, t2);
        b_nop4();
        t0 = b_mult(c, a);
        b_nop4();

        t1 = b_mult(c, a);
        b_nop4();
        t2 = b_mult(c, b);
        b_nop4();
        c = b_mult(a, a);
        b_nop4();

        a = b_mult(b, b);
        b_nop4();
        b = b_mult(t0, t0);
        b_nop4();
        t0 = b_mult(t1, t1);
        b_nop4();
    }
    return c;
}

static unsigned 
b_unit_mult_2_test(unsigned a,unsigned b)
{
    unsigned i;
    unsigned c=a;
    for(i=0;i<256;i++) {
        c = b_mult(c, b);
        b_nop2();
        c = b_mult(c, a);
        b_nop2();
        c = b_mult(c, b);
        b_nop2();
        c = b_mult(c, a);
        b_nop2();
        c = b_mult(c, b);
        b_nop2();
        c = b_mult(c, a);
        b_nop2();
        c = b_mult(c, b);
        b_nop2();
        c = b_mult(c, a);
        b_nop2();
    }
    return c;
}

static unsigned 
b_unit_mult_3_test(unsigned a,unsigned b)
{
    unsigned i;
    unsigned c=a;
    for(i=0;i<356;i++) {
        c = b_mult(c, b);
        b_nop3();
        c = b_mult(c, a);
        b_nop3();
        c = b_mult(c, b);
        b_nop3();
        c = b_mult(c, a);
        b_nop3();
        c = b_mult(c, b);
        b_nop3();
        c = b_mult(c, a);
        b_nop3();
        c = b_mult(c, b);
        b_nop3();
        c = b_mult(c, a);
        b_nop3();
    }
    return c;
}


static unsigned 
b_unit_mult_4_test(unsigned a,unsigned b)
{
    unsigned i;
    unsigned c=a;
    for(i=0;i<256;i++) {
        c = b_mult(c, b);
        b_nop4();
        c = b_mult(c, a);
        b_nop4();
        c = b_mult(c, b);
        b_nop4();
        c = b_mult(c, a);
        b_nop4();
        c = b_mult(c, b);
        b_nop4();
        c = b_mult(c, a);
        b_nop4();
        c = b_mult(c, b);
        b_nop4();
        c = b_mult(c, a);
        b_nop4();
    }
    return c;
}

static unsigned 
b_unit_mult_6_test(unsigned a,unsigned b)
{
    unsigned i;
    unsigned c=a;
    for(i=0;i<256;i++) {
        c = b_mult(c, b);
        b_nop6();
        c = b_mult(c, a);
        b_nop6();
        c = b_mult(c, b);
        b_nop6();
        c = b_mult(c, a);
        b_nop6();
        c = b_mult(c, b);
        b_nop6();
        c = b_mult(c, a);
        b_nop6();
        c = b_mult(c, b);
        b_nop6();
        c = b_mult(c, a);
        b_nop6();
    }
    return c;
}

static void 
b_unit_alu_run(const char *name, unsigned a, unsigned b, unsigned (*test)(unsigned , unsigned))
{
	unsigned i;
	BKNI_EnterCriticalSection();
	for(i=0;i<8;i++) {
		BTRC_TRACE(bunit_alu, START);
		test(a, b); 
		BTRC_TRACE(bunit_alu, STOP);
	}
	BKNI_LeaveCriticalSection();
	BDBG_LOG(("test '%s'", name));
	BTRC_Module_Report(BTRC_MODULE_HANDLE(bunit_alu));
	BTRC_Module_Reset(BTRC_MODULE_HANDLE(bunit_alu));
	BKNI_Sleep(50);
	return;
}

void
bunit_alu(void)
{
    b_unit_alu_run("mult_0", 17, 31, b_unit_mult_0_test);
    b_unit_alu_run("mult_2", 17, 31, b_unit_mult_2_test);
    b_unit_alu_run("mult_3", 17, 31, b_unit_mult_3_test);
    b_unit_alu_run("mult_4", 17, 31, b_unit_mult_4_test);
    b_unit_alu_run("mult_6", 17, 31, b_unit_mult_6_test);
    b_unit_alu_run("mult_pipe", 17, 31, b_unit_mult_pipe_test);
    b_unit_alu_run("mult_pipe2", 17, 31, b_unit_mult_pipe2_test);
    return;
}

