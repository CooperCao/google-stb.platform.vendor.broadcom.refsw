/***************************************************************************
*     (c)2008-2013 Broadcom Corporation
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
* API Description:
*
* Revision History:
*
* $brcm_Log: $
*
***************************************************************************/
#include <stdio.h>

#include "archMips.h"
#include "bstd.h"
#include "cpuctrl.h"
#include "bchp_irq0.h"


extern void devinit(void);
extern void bcm_main(void);
extern void main(int argc, char **argv);
extern void dbg_print(char *);
extern void _writeasm(char c);
extern unsigned long _ftext, _fdata, _fbss, _end;


#define INT_LVL_TIMER       SR_IBIT8 /* timer */
#if (BCHP_CHIP==7435)
    #define BCM7420_SR 			(SR_CU0 | SR_CU1 | SR_FR | (SR_IMASK0 & ~(INT_LVL_TIMER | SR_IBIT7 | SR_IBIT6)) | SR_IE)
#else
    #define BCM7420_SR 			(SR_CU0 | SR_CU1 | SR_FR | (SR_IMASK0 & ~INT_LVL_TIMER) | SR_IE)
#endif

/* Cache mode */
#define CACHE_DISABLED      0x00    /* No cache or disabled */
#define CACHE_WRITETHROUGH  0x01    /* Write-through Mode */
#define CACHE_COPYBACK      0x02    /* Copyback Mode */

int cacheDataMode = CACHE_COPYBACK;

#ifdef MIPS_SDE
void romStart(int argc, char **argv)
{
	BSTD_UNUSED(argc);
	BSTD_UNUSED(argv);
	dbg_print("romStart\n");

	/* init tty driver */
    devinit();

	printf("_ftext=%x _fdata=%x _fbss=%x _end=%x\n", (int)&_ftext, (int)&_fdata, (int)&_fbss, (int)&_end);
	CpuStatusSet(BCM7420_SR);

	bcm_main();
}

void dbg_print(char *str)
{
	while(*str)
	{
        if (*str == '\n')
            _writeasm('\r');
 		_writeasm(*str++);
	}
}

void dbg_print_dec32(unsigned int num)
{
    int rem, ddd;
    rem = num;
    ddd = rem / 1000000000; _writeasm(ddd+48);
    rem = num % 1000000000;
    ddd = rem / 100000000; _writeasm(ddd+48);
    rem = num % 100000000;
    ddd = rem / 10000000; _writeasm(ddd+48);
    rem = num % 10000000;
    ddd = rem / 1000000; _writeasm(ddd+48);
    rem = num % 1000000;
    ddd = rem / 100000; _writeasm(ddd+48);
    rem = num % 100000;
    ddd = rem / 10000; _writeasm(ddd+48);
    rem = num % 10000;
    ddd = rem / 1000; _writeasm(ddd+48);
    rem = num % 1000;
    ddd = rem / 100; _writeasm(ddd+48);
    rem = num % 100;
    ddd = rem / 10; _writeasm(ddd+48);
    rem = num % 10;
    _writeasm(rem+48);
}

void dbg_print_hex32(unsigned int num)
{
    unsigned int byte;
    int i;
    void _writeasm(char c);

    _writeasm('0');
    _writeasm('x');
    for (i=0; i<8; i++) {
        /* 28, 24, .. */
        byte = (num >> (7-i)*4) & 0xf;
        if (byte >= 10)
            byte += 87; /*55;*/
        else
            byte += 48;
        _writeasm((char)byte);
    }
}
#endif

/****************************************************************
 * ExceptReport() : print an Exception Context to console and hang
 ****************************************************************/
void ExceptReport(uint32_t *GenRegs, uint32_t *Cp0Regs, uint32_t *Cp1Regs)
{
	printf("\n\r\n\nMIPS Exception\n\r");
	printf("*************************************************************\n\r");
	printf("STATUS=%08x\tBEV=%x\tIMask=%x\tIEnable=%x\n\r",
			Cp0Regs[(12)],
			(Cp0Regs[(12)]>>22)&1,
			(Cp0Regs[(12)]>>8)&0xff,
			(Cp0Regs[(12)])&1);
	printf("CAUSE=%08x ExcCode=%d\n\r",
			Cp0Regs[(13)],(Cp0Regs[(13)]>>2)&0x1f);
	
	printf("-------------------------------------------------------------\n\r");
	printf(" $0=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[0],GenRegs[1],GenRegs[2],GenRegs[3]);

	printf(" $4=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[4],GenRegs[5],GenRegs[6],GenRegs[7]);
	printf(" $8=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[8],GenRegs[9],GenRegs[10],GenRegs[11]);
	printf("$12=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[12],GenRegs[13],GenRegs[14],GenRegs[15]);
	printf("$16=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[16],GenRegs[17],GenRegs[18],GenRegs[19]);
	printf("$20=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[20],GenRegs[21],GenRegs[22],GenRegs[23]);
	printf("$24=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[24],GenRegs[25],GenRegs[26],GenRegs[27]);
	printf("$28=%08x\t%08x\t%08x\t%08x\n\r",
			GenRegs[28],GenRegs[29],GenRegs[30],GenRegs[31]);

	printf("-------------------------------------------------------------\n\r");
	printf("EPC=0x%08x\t%08x\n\r\n\r",
			Cp0Regs[14], *((uint32_t *)Cp0Regs[14]));

	printf("-------------------------------------------------------------\n\r");
	printf("test : %08x\n\r", Cp1Regs[1]);
	printf("floating point CP1 FCR0 : %08x\n\r", Cp1Regs[0]);
	printf("floating point CP1 FCR31 : %08x\n\r\n\r\n\r", Cp1Regs[31]);

}

