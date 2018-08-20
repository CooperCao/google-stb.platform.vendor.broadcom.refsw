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

#include "uart_ns16550a.h"
#include "config.h"
#include "objalloc.h"

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

static ObjCacheAllocator<UartNS16550a> uartAllocator;

SpinLock UartNS16550a::uartLock;
uintptr_t UartNS16550a::uart_base;

void *UartNS16550a::operator new(size_t sz) {
    UNUSED(sz);
    return uartAllocator.alloc();
}

UartNS16550a::~UartNS16550a(){
}

void UartNS16550a::operator delete(void *u){
    UartNS16550a *up = (UartNS16550a *)u;
    uartAllocator.free(up);
}

void UartNS16550a::init(uintptr_t base)
{
    uartAllocator.init();
    spinLockInit(&uartLock);
    uart_base = base;

    if (uart_base != 0) {
        volatile uint8_t *lcr = (uint8_t *)uart_base + UART_SDW_LCR;
        volatile uint8_t *dll = (uint8_t *)uart_base + UART_SDW_DLL;
        volatile uint8_t *dlh = (uint8_t *)uart_base + UART_SDW_DLH;
        volatile uint8_t *fcr = (uint8_t *)uart_base + UART_SDW_FCR;

        volatile uint8_t *rx_status = (uint8_t *)uart_base + UART_SDW_LSR;
        volatile uint8_t *tx_status = (uint8_t *)uart_base + UART_SDW_LSR;

        *lcr = 0x83;
        *dll = UART_DIV_VALUE & 0xff;
        *dlh = UART_DIV_VALUE >> 8;
        *lcr = 0x03;

        /* need to read back */
        (void)*lcr;

        *fcr = 0x07;

        *rx_status = 0;
        *tx_status = 0;
    }
}

void UartNS16550a::puts(const char *str)
{
    spinLockAcquire(&uartLock);

    if (uart_base != 0) {
        while (*str) {
            uint8_t status = 0;
            do {
                volatile uint8_t *reg = (uint8_t *)(uart_base + UART_SDW_LSR);
                status = *reg;
            } while (!(status & THRE));
            volatile uint8_t *txbuf = (uint8_t *)(uart_base + UART_SDW_THR);
            *txbuf = *str++;
        }
    }
    spinLockRelease(&uartLock);
}

void UartNS16550a::putc(const char c)
{
    spinLockAcquire(&uartLock);
    if (uart_base != 0) {
        uint8_t status = 0;

        do {
            volatile uint8_t *tx_status = (uint8_t *)(uart_base + UART_SDW_LSR);
            status = *tx_status;
        } while (!(status & THRE));

        volatile uint8_t *txbuf = (uint8_t *)(uart_base + UART_SDW_THR);
        *txbuf = c;
    }
    spinLockRelease(&uartLock);
}

void UartNS16550a::getc(char *c)
{
    spinLockAcquire(&uartLock);
    if (uart_base != 0) {
        volatile uint8_t *rx_status = (uint8_t *)(uart_base + UART_SDW_LSR);
        volatile uint8_t *rx_data = (uint8_t *)(uart_base + UART_SDW_RBR);
        char inval;

        while (true) {
            volatile uint8_t status = *rx_status;
            if (status & (RXOVFERR | RXPARERR | RXFRAMERR)) {
                inval = *rx_data;
                continue;
            }
            else if (status & RXRDA) {
                *c = *rx_data;
                break;
            }
        }
        UNUSED(inval);
    }
    spinLockRelease(&uartLock);
}

void UartNS16550a::gets(char *str, const unsigned int slen)
{
    spinLockAcquire(&uartLock);

    if (uart_base != 0) {
        volatile uint8_t *rx_status = (uint8_t *)(uart_base + UART_SDW_LSR);
        volatile uint8_t *rx_data = (uint8_t *)(uart_base + UART_SDW_RBR);
        char inval;
        char *end = str + slen - 1;

        if (slen == 0)
            return;

        while (str < end) {
            while (true) {
                volatile uint8_t status = *rx_status;

                if (status & (RXOVFERR | RXPARERR | RXFRAMERR)) {
                    inval = *rx_data;
                    continue;
                }
                else if (status & RXRDA) {
                    *str = *rx_data;
                    break;
                }
            }

            if ((*str == '\r') || (*str == '\n'))
                break;

            str++;
        }
        UNUSED(inval);
        *str = 0;
    }
    spinLockRelease(&uartLock);
}
