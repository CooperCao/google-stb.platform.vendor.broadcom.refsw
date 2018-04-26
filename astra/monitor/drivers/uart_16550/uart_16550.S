/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>

#include "asm_helpers.h"
#include "uart_16550_priv.h"

	.globl	uart_16550_init
	.globl	uart_16550_putc
	.globl	uart_16550_getc
	.globl	uart_16550_flush

	/* -----------------------------------------------
	 * int uart_16550_init(unsigned long base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the uart without a
	 * C Runtime to print debug information. This
	 * function will be accessed by uart_init and
	 * crash reporting.
	 * In: x0 - uart base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Out: return 0 on success
	 *      return -1 on error
	 * Clobber list: x1, x2, x3
	 * -----------------------------------------------
	 */
func uart_16550_init
	/* Check the input base address */
	cbz	x0, init_fail
	/* Check baud rate and uart clock for sanity */
	cbz	w1, init_fail
	cbz	w2, init_fail

	/* Program the baudrate */
	/* Divisor =  (Uart clock + 8 * baud rate) / (16 * baud rate) */
	lsl	w3, w2, #3
	add	w1, w1, w3
	lsl	w2, w2, #4
	udiv	w2, w1, w2
	and	w1, w2, #0xff		/* w1 = DLL */
	lsr	w2, w2, #8
	and	w2, w2, #0xff		/* w2 = DLLM */
	ldr	w3, [x0, #UARTLCR]
	orr	w3, w3, #UARTLCR_DLAB
	str	w3, [x0, #UARTLCR]	/* enable DLL, DLLM programming */
	str	w1, [x0, #UARTDLL]	/* program DLL */
	str	w2, [x0, #UARTDLLM]	/* program DLLM */
	mov	w2, #~UARTLCR_DLAB
	and	w3, w3, w2
	str	w3, [x0, #UARTLCR]	/* disable DLL, DLLM programming */

	/* 8n1 */
	mov	w3, #3
	str	w3, [x0, #UARTLCR]
	/* no interrupt */
	mov	w3, #0
	str	w3, [x0, #UARTIER]
	/* enable fifo, DMA */
	mov	w3, #(UARTFCR_FIFOEN | UARTFCR_DMAEN)
	str	w3, [x0, #UARTFCR]
	/* DTR + RTS */
	mov	w3, #3
	str	w3, [x0, #UARTMCR]
	mov	w0, #0
	ret
init_fail:
	mov	w0, #-1
	ret
endfunc uart_16550_init

	/* --------------------------------------------------------
	 * int uart_16550_putc(int c, unsigned int base_addr)
	 * Function to output a character over the uart. It
	 * returns the character printed on success or -1 on error.
	 * In: w0 - character to be printed
	 *     x1 - uart base address
	 * Out: return character
	 *      return -1 on error
	 * Clobber list: x2
	 * --------------------------------------------------------
	 */
func uart_16550_putc
	/* Check the input parameter */
	cbz	x1, putc_error

	/* Prepend '\r' to '\n' */
	cmp	w0, #0xA
	b.ne	2f
	/* Check if the transmit FIFO is full */
1:	ldr	w2, [x1, #UARTLSR]
	tbz	w2, #UARTLSR_THRE_SHIFT, 1b
	mov	w2, #0xD		/* '\r' */
	str	w2, [x1, #UARTTX]

	/* Check if the transmit FIFO is full */
2:	ldr	w2, [x1, #UARTLSR]
	tbz	w2, #UARTLSR_THRE_SHIFT, 2b
	str	w0, [x1, #UARTTX]
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc uart_16550_putc

	/* ---------------------------------------------
	 * int uart_16550_getc(void)
	 * Function to get a character from the uart.
	 * It returns the character grabbed on success
	 * or -1 on error.
	 * In: x0 - uart base address
	 * Out: return character
	 *      return -1 on error
	 * Clobber list: x0, x1
	 * ---------------------------------------------
	 */
func uart_16550_getc
	/* Check if the receive FIFO is empty */
1:	ldr	w1, [x0, #UARTLSR]
	tbz	w1, #UARTLSR_RDR_BIT, 1b
	ldr	w0, [x0, #UARTRX]
	ret
getc_error:
	mov	w0, #-1
	ret
endfunc uart_16550_getc

	/* ---------------------------------------------
	 * int uart_16550_flush(uintptr_t base_addr)
	 * Function to force a write of all buffered
	 * data that hasn't been output.
	 * In: x0 - uart base address
	 * Out: return 0 on success
	 *      return -1 on error
	 * Clobber list: x0, x1
	 * ---------------------------------------------
	 */
func uart_16550_flush
	/* Placeholder */
	mov	w0, #0
	ret
endfunc uart_16550_flush
