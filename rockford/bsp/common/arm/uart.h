/***************************************************************************
 *     Copyright (c) 2012-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************/
#ifndef __UART_H__
#define __UART_H__

#define UART_BASE_CLK		81000000
#define UART_SPEED		115200

#define THR_REG			0x00
#define RBR_REG			0x00
#define DLL_REG			0x00

#define DLH_REG			0x04
#define FCR_REG			0x08
#define LCR_REG			0x0c
#define LSR_REG			0x14

#define LSR_DR			BIT(0)
#define LSR_THRE		BIT(5)

void uart_init(void);
int uart_lock(void);
void uart_unlock(int lock);
int uart_putchar(char c);
int uart_getchar(void);

#endif /* __UART_H__ */
