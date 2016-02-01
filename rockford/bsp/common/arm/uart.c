/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/
#include <common_arm.h>
#include "uart.h"
#include "bchp_common.h"

static unsigned long uart_base = BCHP_PHYSICAL_OFFSET + BCHP_UARTA_REG_START;

void uart_init(void)
{
	/* round to nearest value */
	unsigned int div = (UART_BASE_CLK + UART_SPEED * 8) / (UART_SPEED * 16);

	DEV_WR(uart_base + LCR_REG, 0x83);
	DEV_WR(uart_base + DLL_REG, div & 0xff);
	DEV_WR(uart_base + DLH_REG, div >> 8);
	DEV_WR_RB(uart_base + LCR_REG, 0x03);

	DEV_WR(uart_base + FCR_REG, 0x07);
}

int uart_putchar(char c)
{
	while (!(DEV_RD(uart_base + LSR_REG) & LSR_THRE));
	DEV_WR(uart_base + THR_REG, c);
	return 0;
}

int uart_getchar(void)
{
	int c;
	while (!(DEV_RD(uart_base + LSR_REG) & LSR_DR));
	c = DEV_RD(uart_base + RBR_REG) & 0xff;
	return c;
}

int uart_detchar(void)
{
	int c;
    if (DEV_RD(uart_base + LSR_REG) & LSR_DR)
        c = DEV_RD(uart_base + RBR_REG) & 0xff;
    else
        c = 0;
    return c;
}


void dbg_print(char *str)
{
	while(*str)
	{
        if (*str == '\n')
            uart_putchar('\r');
 		uart_putchar(*str++);
	}
}

void dbg_print_dec32(unsigned int num)
{
    int rem, ddd;
    rem = num;
    ddd = rem / 1000000000; uart_putchar(ddd+48);
    rem = num % 1000000000;
    ddd = rem / 100000000; uart_putchar(ddd+48);
    rem = num % 100000000;
    ddd = rem / 10000000; uart_putchar(ddd+48);
    rem = num % 10000000;
    ddd = rem / 1000000; uart_putchar(ddd+48);
    rem = num % 1000000;
    ddd = rem / 100000; uart_putchar(ddd+48);
    rem = num % 100000;
    ddd = rem / 10000; uart_putchar(ddd+48);
    rem = num % 10000;
    ddd = rem / 1000; uart_putchar(ddd+48);
    rem = num % 1000;
    ddd = rem / 100; uart_putchar(ddd+48);
    rem = num % 100;
    ddd = rem / 10; uart_putchar(ddd+48);
    rem = num % 10;
    uart_putchar(rem+48);
}

void dbg_print_hex32(unsigned int num)
{
    unsigned int byte;
    int i;

    uart_putchar('0');
    uart_putchar('x');
    for (i=0; i<8; i++) {
        /* 28, 24, .. */
        byte = (num >> (7-i)*4) & 0xf;
        if (byte >= 10)
            byte += 87; /*55;*/
        else
            byte += 48;
        uart_putchar((char)byte);
    }
}
