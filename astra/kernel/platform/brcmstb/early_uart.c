/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/


#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "uart_boot.h"



extern uintptr_t early_uart_base;

typedef uint32_t SpinLock;
void spinLockInit(SpinLock *lock);
void spinLockAcquire(SpinLock *lock);
void spinLockRelease(SpinLock *lock);
SpinLock earlyUartLock;

/*
 * NS16550A UART
 */

/* Rx status register */
#define RXRDA          0x01
#define RXOVFERR       0x02
#define RXPARERR       0x04
#define RXFRAMERR      0x08

#define THRE                0x20
#define UART_SDW_RBR        0x00
#define UART_SDW_THR        0x00
#define UART_SDW_DLL        0x00
#define UART_SDW_DLH        0x04
#define UART_SDW_IER        0x04
#define UART_SDW_IIR        0x08
#define UART_SDW_FCR        0x08
#define UART_SDW_LCR        0x0c
#define UART_SDW_MCR        0x10
#define UART_SDW_LSR        0x14
#define UART_SDW_MSR        0x18
#define UART_SDW_SCR        0x1c

#define UART_BASE_CLK       81000000
#define UART_BAUD_RATE      115200
#define UART_DIV_VALUE      ((UART_BASE_CLK + UART_BAUD_RATE*8) / (UART_BAUD_RATE*16))

/* PL011 support hacked in here */
/* Flags register */
#define RXFE            0x10
#define TXFF            0x20

#define UART_DR         (0x00 >> 2)
#define UART_FR         (0x18 >> 2)

#define UART_IBRD       0x24
#define UART_FBRD       0x28

eUartType early_uart_type;

void early_uart_init(eUartType uart_type)
{
    spinLockInit(&earlyUartLock);
    early_uart_type = uart_type;
}

static void ns16550a_uart_puts(const char *str)
{
    if (early_uart_base != 0) {
        while (*str) {
            uint8_t status = 0;
            do {
                volatile uint8_t *reg = (uint8_t *)(early_uart_base + UART_SDW_LSR);
                status = *reg;
            } while (!(status & THRE));

            volatile uint8_t *txbuf = (uint8_t *)(early_uart_base + UART_SDW_THR);
            *txbuf = *str++;
        }
    }
}

static void ns16550a_uart_putc(const char c)
{
    if (early_uart_base != 0) {
        uint8_t status = 0;

        do {
            volatile uint8_t *tx_status = (uint8_t *)(early_uart_base + UART_SDW_LSR);
            status = *tx_status;
        } while (!(status & THRE));

        volatile uint8_t *txbuf = (uint8_t *)(early_uart_base + UART_SDW_THR);
        *txbuf = c;
    }
}

static void pl011_uart_puts(const char *str)
{
    if (early_uart_base != 0) {
        while (*str) {
            volatile uint32_t *reg = (volatile uint32_t *)(early_uart_base);
            while(reg[UART_FR] & TXFF);
            if (*str == 0xa) {
                reg[UART_DR] = 0xd;
                while(reg[UART_FR] & TXFF);
            }
            reg[UART_DR] = *str & 0xff;
            str++;
        }
    }
}

static void pl011_uart_putc(const char c)
{
    if (early_uart_base != 0) {
        volatile uint32_t *reg = (volatile uint32_t *)(early_uart_base);
        while(reg[UART_FR] & TXFF);
        if (c == 0xa) {
            reg[UART_DR] = 0xd;
            while(reg[UART_FR] & TXFF);
        }
        reg[UART_DR] = c & 0xff;
    }
}


void early_uart_puts(const char *str)
{
    spinLockAcquire(&earlyUartLock);

    if(early_uart_type == UART_TYPE_NS16550a)
        ns16550a_uart_puts(str);
    else
        pl011_uart_puts(str);

    spinLockRelease(&earlyUartLock);
}

void early_uart_putc(const char c)
{
    spinLockAcquire(&earlyUartLock);

    if(early_uart_type == UART_TYPE_NS16550a)
        ns16550a_uart_putc(c);
    else
        pl011_uart_putc(c);

    spinLockRelease(&earlyUartLock);
}
