/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __UART_16550_H__
#define __UART_16550_H__

int uart_16550_init(
    uintptr_t base,
    unsigned int uart_clk,
    unsigned int baud_rate);

int uart_16550_putc(
    int c,
    uintptr_t base);

int uart_16550_getc(
    uintptr_t base);

int uart_16550_flush(
    uintptr_t base);

#endif	/* __UART_16550_H__ */
