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

#include "uart_pl011.h"
#include "config.h"
#include "objalloc.h"

/* PL011 support hacked in here */
/* Flags register */
#define RXFE        0x10
#define TXFF        0x20

#define UART_DR     (0x00 >> 2)
#define UART_FR     (0x18 >> 2)

#define UART_ECR     0x004

#define UART_IBRD   0x24
#define UART_FBRD   0x28
#define UART_LCR    0x2c
#define UART_CR     0x30

#define CFG_SERIAL_BAUD_RATE    115200
#define UART_BASE_CLK           54000000


#define PL011_UARTCR_CTSEN        (1 << 15)	/* CTS hardware flow control enable */
#define PL011_UARTCR_RTSEN        (1 << 14)	/* RTS hardware flow control enable */
#define PL011_UARTCR_RTS          (1 << 11)	/* Request to send */
#define PL011_UARTCR_DTR          (1 << 10)	/* Data transmit ready. */
#define PL011_UARTCR_RXE          (1 << 9)	/* Receive enable */
#define PL011_UARTCR_TXE          (1 << 8)	/* Transmit enable */
#define PL011_UARTCR_LBE          (1 << 7)	/* Loopback enable */
#define PL011_UARTCR_UARTEN       (1 << 0)	/* UART Enable */

/* FIFO Enabled / No Parity / 8 Data bit / One Stop Bit */
#define PL011_LINE_CONTROL  (PL011_UARTLCR_H_FEN | PL011_UARTLCR_H_WLEN_8)

/* Line Control Register Bits */
#define PL011_UARTLCR_H_SPS       (1 << 7)	/* Stick parity select */
#define PL011_UARTLCR_H_WLEN_8    (3 << 5)
#define PL011_UARTLCR_H_WLEN_7    (2 << 5)
#define PL011_UARTLCR_H_WLEN_6    (1 << 5)
#define PL011_UARTLCR_H_WLEN_5    (0 << 5)
#define PL011_UARTLCR_H_FEN       (1 << 4)	/* FIFOs Enable */
#define PL011_UARTLCR_H_STP2      (1 << 3)	/* Two stop bits select */
#define PL011_UARTLCR_H_EPS       (1 << 2)	/* Even parity select */
#define PL011_UARTLCR_H_PEN       (1 << 1)	/* Parity Enable */
#define PL011_UARTLCR_H_BRK       (1 << 0)	/* Send break */


SpinLock UartPL011::uartLock;
uintptr_t UartPL011::uart_base;

static ObjCacheAllocator<UartPL011> uartAllocator;


void *UartPL011::operator new(size_t sz) {
    UNUSED(sz);
    return uartAllocator.alloc();
}

UartPL011::~UartPL011(){
}

void UartPL011::operator delete(void *u){
    UartPL011 *up = (UartPL011 *)u;
    uartAllocator.free(up);
}

void UartPL011::init(uintptr_t base)
{
    unsigned int div;
    unsigned int tmp;

    uartAllocator.init();
    spinLockInit(&uartLock);
    uart_base = base;

    div = (UART_BASE_CLK << 2) / CFG_SERIAL_BAUD_RATE;

    tmp = *(volatile uint32_t *)(uart_base + UART_CR);
    tmp = tmp & ~PL011_UARTCR_UARTEN;
    *(volatile uint32_t *)(uart_base + UART_CR) = tmp;

    tmp = *(volatile uint32_t *)(uart_base + UART_LCR);

    *(volatile uint32_t *)(uart_base + UART_IBRD) = div >> 6;
    *(volatile uint32_t *)(uart_base + UART_FBRD) = div & 0x3F;
    *(volatile uint32_t *)(uart_base + UART_LCR) = PL011_LINE_CONTROL;
    *(volatile uint32_t *)(uart_base + UART_ECR) = 0;
    *(volatile uint32_t *)(uart_base + UART_CR) = PL011_UARTCR_RXE | PL011_UARTCR_TXE | PL011_UARTCR_UARTEN;
}

void UartPL011::puts(const char *str)
{
    spinLockAcquire(&uartLock);

    if (uart_base != 0) {
        while (*str) {
            volatile uint32_t *reg = (volatile uint32_t *)(uart_base);
            while(reg[UART_FR] & TXFF);
            if (*str == 0xa) {
                reg[UART_DR] = 0xd;
                while(reg[UART_FR] & TXFF);
            }
            reg[UART_DR] = *str & 0xff;
            str++;
        }
    }
    spinLockRelease(&uartLock);
}

void UartPL011::putc(const char c)
{
    spinLockAcquire(&uartLock);
    if (uart_base != 0) {
        volatile uint32_t *reg = (volatile uint32_t *)(uart_base);
        while(reg[UART_FR] & TXFF);
        if (c == 0xa) {
            reg[UART_DR] = 0xd;
            while(reg[UART_FR] & TXFF);
        }
        reg[UART_DR] = c & 0xff;
    }

    spinLockRelease(&uartLock);
}

void UartPL011::getc(char *c)
{
    spinLockAcquire(&uartLock);

    *c = 0;

    spinLockRelease(&uartLock);
}

void UartPL011::gets(char *str, const unsigned int slen)
{
    spinLockAcquire(&uartLock);

    UNUSED(slen);
    *str = 0;

    spinLockRelease(&uartLock);
}
