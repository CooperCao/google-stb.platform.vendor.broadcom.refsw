/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "monitor.h"
#include "uart.h"

/***********************************************************
 * The mon_printf implementation for all BL stages
 ***********************************************************/

#define get_num_va_args(args, lcount) \
	(((lcount) > 1) ? va_arg(args, long long int) :	\
	((lcount) ? va_arg(args, long int) : va_arg(args, int)))

#define get_unum_va_args(args, lcount) \
	(((lcount) > 1) ? va_arg(args, unsigned long long int) :	\
	((lcount) ? va_arg(args, unsigned long int) : va_arg(args, unsigned int)))

#define get_unum_va_width(lcount) \
	(((lcount) > 1) ? sizeof(unsigned long long int) * 2 :	\
	((lcount) ? sizeof(unsigned long int) * 2 : sizeof(unsigned int) * 2))

static void string_print(const char *str)
{
	while (*str)
		uart_putc(*str++);
}

static void unsigned_num_print(unsigned long long int unum, unsigned int radix, unsigned int width)
{
	/* Just need enough space to store 64 bit decimal integer */
	unsigned char num_buf[20];
	unsigned int i = 0, rem;

	do {
		rem = unum % radix;
		if (rem < 0xa)
			num_buf[i++] = '0' + rem;
		else
			num_buf[i++] = 'a' + (rem - 0xa);
	} while (unum /= radix);

	while (i < width)
		num_buf[i++] = '0';

	while (i-- > 0)
		uart_putc(num_buf[i]);
}

/*******************************************************************
 * Reduced format print for Trusted firmware.
 * The following type specifiers are supported by this print
 * %x - hexadecimal format
 * %s - string format
 * %d or %i - signed decimal format
 * %u - unsigned decimal format
 * %p - pointer format
 *
 * The following length specifiers are supported by this print
 * %l - long int (64-bit on AArch64)
 * %ll - long long int (64-bit on AArch64)
 * %z - size_t sized integer formats (64 bit on AArch64)
 *
 * The print exits on all other formats specifiers other than valid
 * combinations of the above specifiers.
 *******************************************************************/
void mon_printf(const char *fmt, ...)
{
	va_list args;
	int l_count;
        int leading_0;
	long long int num;
	unsigned long long int unum;
        unsigned int width;
	char *str;

	va_start(args, fmt);
	while (*fmt) {
		l_count = 0;
                leading_0 = 0;
                width = 0;

		if (*fmt == '%') {
			fmt++;
			/* Check the format specifier */
loop:
			switch (*fmt) {
			case 'i': /* Fall through to next one */
			case 'd':
				num = get_num_va_args(args, l_count);
				if (num < 0) {
					uart_putc('-');
					unum = (unsigned long long int)-num;
				} else
					unum = (unsigned long long int)num;

				unsigned_num_print(unum, 10, 0);
				break;
			case 's':
				str = va_arg(args, char *);
				string_print(str);
				break;
			case 'p':
				unum = (uintptr_t)va_arg(args, void *);
				if (unum)
					string_print("0x");
				if (leading_0)
					width = get_unum_va_width(l_count);
				unsigned_num_print(unum, 16, width);
				break;
			case 'x':
				unum = get_unum_va_args(args, l_count);
				if (leading_0)
					width = get_unum_va_width(l_count);
				unsigned_num_print(unum, 16, width);
				break;
			case 'z':
				if (sizeof(size_t) == 8)
					l_count = 2;
				fmt++;
				goto loop;
			case 'l':
				l_count++;
				fmt++;
				goto loop;
			case 'u':
				unum = get_unum_va_args(args, l_count);
				unsigned_num_print(unum, 10, 0);
				break;
                        case '0':
				leading_0 = 1;
				fmt++;
				goto loop;
			default:
				/* Exit on any other format specifier */
				goto exit;
			}
			fmt++;
			continue;
		}
		uart_putc(*fmt++);
	}
exit:
	va_end(args);
}
