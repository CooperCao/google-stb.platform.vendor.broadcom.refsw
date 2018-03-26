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
 *****************************************************************************/

#include "monitor.h"
#include "memory.h"
#include "platform.h"
#include "uart.h"
#include "uart_16550.h"
#include "brcmstb_priv.h"

#define BCM7268B0_UART_BASE 0xf040c000

uintptr_t uart_base;

void plat_uart_init(uintptr_t base)
{
    uintptr_t uart_page;

    /*
     * For Bolt backward compatibility, if UART base is NOT set in reg
     * x2, try to identify the chip by reading the chip ID register
     * from the default location. This is best effort, and may NOT work
     * for all chips.
     */
    if (!base) {
        switch (MMIO32(BRCMSTB_CHIP_ID_ADDR)) {
        case 0x72680010:
        case 0x72710010:
        case 0x72550000:
            base = BCM7268B0_UART_BASE;
            break;
        default:
            SYS_HALT();
        }
    }

    /* Init UART port */
    uart_init(
        base,
        BRCMSTB_UART_CLK,
        BRCMSTB_BAUD_RATE);

    uart_base = base;
    uart_page = ALIGN_TO_PAGE(base);

    /* Map UART registers page */
    mmap_add_region(
        uart_page,
        uart_page,
        PAGE_SIZE,
        MT_SOC_REG | MT_SECURE);
}

int uart_init(
    uintptr_t base,
    unsigned int uart_clk,
    unsigned int baud_rate)
{
    DBG_ASSERT(base);
    DBG_ASSERT(uart_clk);
    DBG_ASSERT(baud_rate);

    /* Call into 16550 driver */
    return uart_16550_init(
        base,
        uart_clk,
        baud_rate);
}

void uart_uninit(void)
{
    /* TBD */
}

int uart_putc(int c)
{
    DBG_ASSERT(uart_base);

    /* Call into 16550 driver */
    return uart_16550_putc(c, uart_base);
}

int uart_getc(void)
{
    DBG_ASSERT(uart_base);

    /* Call into 16550 driver */
    return uart_16550_getc(uart_base);
}
