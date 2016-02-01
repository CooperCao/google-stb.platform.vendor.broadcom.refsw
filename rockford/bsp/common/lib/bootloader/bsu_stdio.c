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
//#include <stdio.h>
#include <stdarg.h>
#include "lib_types.h"
//#include "lib_printf.h"
#include "bsu_string.h"

extern int console_xprint(const char *str);

/*  *********************************************************************
    *  Globals								*
    ********************************************************************* */

static const char digits[17] = "0123456789ABCDEF";
static const char ldigits[17] = "0123456789abcdef";

#ifndef PRINTF_BUF_SIZE
#define PRINTF_BUF_SIZE	2048
#endif

/**
 * Check if the current output pointer has reached the last byte of the output
 * buffer. (Need to leave room for the terminating byte.)
 *
 *        start - base of the output buffer
 *        curr - current position within output buffer
 *        size - size of the output buffer
 */
static inline int is_buf_full(const char *start, const char *curr, size_t size)
{
	return (ptrdiff_t)(curr - start) >= (ptrdiff_t)size - 1;
}

/*  *********************************************************************
    *  __atox(buf, num, radix, width, pad, pdigits)
    *
    *  Convert a number to a string
    *
    *  Input Parameters:
    *      buf - where to put characters
    *      num - number to convert
    *      radix - radix to convert number to (usually 10 or 16)
    *      width - width in characters
    *      pad - padding character to use
    *      pdigits - null-terminated character array of hex characters
    *
    *  Return Value:
    *      number of digits placed in output buffer
    ********************************************************************* */
static int __atox(char *buf, unsigned int num, unsigned int radix, int width,
		  char pad, const char *pdigits)
{
	char buffer[16];
	char *op;
	int retval;

	op = &buffer[0];
	retval = 0;

	do {
		*op++ = pdigits[num % radix];
		retval++;
		num /= radix;
	} while (num != 0);

	if (width && (width > retval)) {
		width = width - retval;
		while (width) {
			*op++ = pad;
			retval++;
			width--;
		}
	}

	while (op != buffer) {
		op--;
		*buf++ = *op;
	}

	return retval;
}

/*  *********************************************************************
    *  __atox_n(n, bufstart, buf, num, radix, width, pad, pdigits)
    *
    *  Convert a number to a string
    *
    *  Input Parameters:
    *      n - size of buf
    *      bufstart - pointer to start of buf
    *      buf - where to put characters
    *      radix - radix to convert number to (usually 10 or 16)
    *      width - width in characters
    *      pad - padding character to use
    *      pdigits - null-terminated character array of hex characters
    *
    *  Return Value:
    *      1 if maximum characters reached
    *      0 otherwise
    ********************************************************************* */
static int __atox_n(size_t n, char *bufstart, char *buf, unsigned int num,
		    unsigned int radix, int width, char pad,
		    const char *pdigits)
{
	char buffer[16];
	char *op;
	int retval, retval2;

	op = &buffer[0];
	retval = 0;
	retval2 = 0;

	do {
		*op++ = pdigits[num % radix];
		retval++;
		num /= radix;
	} while (num != 0);

	if (width && (width > retval)) {
		width = width - retval;
		while (width) {
			*op++ = pad;
			retval++;
			width--;
		}
	}

	while (op != buffer) {
		op--;
		*buf++ = *op;
		retval2++;
		if (is_buf_full(bufstart, buf, n))
			break;
	}

	return retval2;
}

/*  *********************************************************************
    *  __llatox(buf, num, radix, width, pad, pdigits)
    *
    *  Convert a long number to a string
    *
    *  Input Parameters:
    *      buf - where to put characters
    *      num - number to convert
    *      radix - radix to convert number to (usually 10 or 16)
    *      width - width in characters
    *      pad - padding character to use
    *      pdigits - null-terminated character array of hex characters
    *
    *  Return Value:
    *      number of digits placed in output buffer
    ********************************************************************* */
static int __llatox(char *buf, unsigned long long num, unsigned int radix,
		    int width, char pad, const char *pdigits)
{
	char buffer[16];
	char *op;
	int retval;

	op = &buffer[0];
	retval = 0;

#ifdef _MIPSREGS32_
	/*
	 * Hack: to avoid pulling in the helper library that isn't necessarily
	 * compatible with PIC code, force radix to 16, use shifts and masks
	 */
	do {
		*op++ = pdigits[num & 0x0F];
		retval++;
		num >>= 4;
	} while (num != 0);
#else
	do {
		*op++ = pdigits[num % radix];

		retval++;
		num /= radix;
	} while (num != 0);
#endif

	if (width && (width > retval)) {
		width = width - retval;
		while (width) {
			*op++ = pad;
			retval++;
			width--;
		}
	}

	while (op != buffer) {
		op--;
		*buf++ = *op;
	}

	return retval;
}

/*  *********************************************************************
    *  __llatox_n(n, bufstart, buf, num, radix, width, pad, pdigits)
    *
    *  Convert a long number to a string
    *
    *  Input Parameters:
    *      n - size of buf
    *      bufstart - pointer to start of buf
    *      buf - where to put characters
    *      radix - radix to convert number to (usually 10 or 16)
    *      width - width in characters
    *      pad - padding character to use
    *      pdigits - null-terminated character array of hex characters
    *
    *  Return Value:
    *      number of digits placed in output buffer
    ********************************************************************* */
static int __llatox_n(size_t n, char *bufstart, char *buf,
		      unsigned long long num, unsigned int radix,
		      int width, char pad, const char *pdigits)
{
	char buffer[16];
	char *op;
	int retval, retval2;

	op = &buffer[0];
	retval = 0;
	retval2 = 0;

#ifdef _MIPSREGS32_
	/*
	 * Hack: to avoid pulling in the helper library that isn't necessarily
	 * compatible with PIC code, force radix to 16, use shifts and masks
	 */
	do {
		*op++ = pdigits[num & 0x0F];
		retval++;
		num >>= 4;
	} while (num != 0);
#else
	do {
		*op++ = pdigits[num % radix];

		retval++;
		num /= radix;
	} while (num != 0);
#endif

	if (width && (width > retval)) {
		width = width - retval;
		while (width) {
			*op++ = pad;
			retval++;
			width--;
		}
	}

	while (op != buffer) {
		op--;
		*buf++ = *op;
		retval2++;
		if (is_buf_full(bufstart, buf, n))
			break;
	}

	return retval2;
}

/*  *********************************************************************
    *  xvsprintf(outbuf, format, marker)
    *
    *  Format a string into the output buffer
    *
    *  Input Parameters:
    *      outbuf - output buffer
    *      format - format string
    *      marker - parameters as a va_list
    *
    *  Return Value:
    *      number of characters copied
    ********************************************************************* */
#define isdigit(x) (((x) >= '0') && ((x) <= '9'))
int xvsprintf(char *outbuf, const char *format, va_list marker)
{
	char *optr;
	const char *iptr;
	unsigned char *tmpptr;
	unsigned int x;
	unsigned long long lx;
	int i;
	long long ll;
	int islong;
	int hashash;

	optr = outbuf;
	iptr = format;

	while (*iptr) {
		char pad = ' ';
		int width = 0;
		int precision = 0;

		if (*iptr != '%') {
			*optr++ = *iptr++;
			continue;
		}

		iptr++;

		hashash = 0;
		if (*iptr == '#') {
			hashash = 1;
			iptr++;
		}
		if (*iptr == '-') {
			/* TODO: maybe implement left alignment */
			iptr++;
		}

		if (*iptr == '0') {
			pad = '0';
			iptr++;
		}

		if (*iptr == '*') {
			iptr++;
			width = va_arg(marker, unsigned int);
		}
		while (*iptr && isdigit(*iptr)) {
			width += (*iptr - '0');
			iptr++;
			if (isdigit(*iptr))
				width *= 10;
		}
		if (*iptr == '.') {
			iptr++;
			while (*iptr && isdigit(*iptr)) {
				precision += (*iptr - '0');
				iptr++;
				if (isdigit(*iptr))
					precision *= 10;
			}
		}

		islong = 0;
		if (*iptr == 'l') {
			islong++;
			iptr++;
		}
		if (*iptr == 'l') {
			islong++;
			iptr++;
		}

		if (*iptr == 'z') {
			if (sizeof(size_t) == sizeof(long long))
				islong = 2;
			else
				islong = 1;
			iptr++;
		}

		if (*iptr == 'p')
			hashash = 1;

		if (hashash) {
			*optr++ = '0';
			*optr++ = 'x';
			/* Hash steals from the width */
			width = (width > 2) ? width - 2 : 0;
		}

		switch (*iptr) {
		case 's':
			tmpptr =
			    (unsigned char *)va_arg(marker, unsigned char *);
			if (!tmpptr)
				tmpptr = (unsigned char *)"(null)";
			if ((width == 0) && (precision == 0)) {
				while (*tmpptr)
					*optr++ = *tmpptr++;
				break;
			}
			i = strlen((char *)tmpptr);
			if (width > i)
				i = width - i;
			else
				i = 0;	/* don't need to print blank spaces */

			while (i) {
				*optr++ = ' ';
				i--;
			}
			while (width && *tmpptr) {
				*optr++ = *tmpptr++;
				width--;
			}
			break;
		case 'd':
		case 'i':
			switch (islong) {
			case 0:
			case 1:
				i = va_arg(marker, int);
				if (i < 0) {
					*optr++ = '-';
					i = -i;
				}
				optr += __atox(optr, i, 10, width, pad, digits);
				break;
			case 2:
				ll = va_arg(marker, long long int);
				if (ll < 0) {
					*optr++ = '-';
					ll = -ll;
				}
				optr += __llatox(optr, ll, 10, width, pad,
						 digits);
				break;
			}
			break;
		case 'u':
			switch (islong) {
			case 0:
			case 1:
				x = va_arg(marker, unsigned int);
				optr += __atox(optr, x, 10, width, pad, digits);
				break;
			case 2:
				lx = va_arg(marker, unsigned long long);
				optr += __llatox(optr, lx, 10, width, pad,
						 digits);
				break;
			}
			break;
		case 'X':
		case 'x':
			switch (islong) {
			case 0:
			case 1:
				x = va_arg(marker, unsigned int);
				optr += __atox(optr, x, 16, width, pad,
					       (*iptr ==
						'X') ? digits : ldigits);
				break;
			case 2:
				lx = va_arg(marker, unsigned long long);
				optr += __llatox(optr, lx, 16, width, pad,
						 (*iptr ==
						  'X') ? digits : ldigits);
				break;
			}
			break;
		case 'p':
#ifdef __long64
			lx = va_arg(marker, unsigned long long);
			optr += __llatox(optr, lx, 16, width, pad, ldigits);
#else
			x = va_arg(marker, unsigned int);
			optr += __atox(optr, x, 16, width, pad, ldigits);
#endif
			break;
		case 'c':
			x = va_arg(marker, int);
			*optr++ = x & 0xff;
			break;

		default:
			*optr++ = *iptr;
			break;
		}
		iptr++;
	}

	*optr = '\0';

	return optr - outbuf;
}

/*  *********************************************************************
    *  xvsnprintf(outbuf, n, format, marker)
    *
    *  Format a string into the output buffer up to n characters
    *
    *  Input Parameters:
    *      outbuf - output buffer
    *      n - size of outbuf
    *      format - format string
    *      marker - parameters as a va_list
    *
    *  Return Value:
    *      number of characters copied
    ********************************************************************* */
int xvsnprintf(char *outbuf, int n, const char *format, va_list marker)
{
	char *optr;
	const char *iptr;
	unsigned char *tmpptr;
	unsigned int x;
	unsigned long long lx;
	int i;
	long long ll;
	int islong;
	int hashash;

	optr = outbuf;
	iptr = format;

	while (!is_buf_full(outbuf, optr, n) && *iptr) {
		char pad = ' ';
		int width = 0;
		int precision = 0;

		if (*iptr != '%') {
			*optr++ = *iptr++;
			continue;
		}

		iptr++;

		hashash = 0;
		if (*iptr == '#') {
			hashash = 1;
			iptr++;
		}

		if (*iptr == '-') {
			/* TODO: maybe implement left alignment */
			iptr++;
		}

		if (*iptr == '0') {
			pad = '0';
			iptr++;
		}

		if (*iptr == '*') {
			iptr++;
			width = va_arg(marker, unsigned int);
		}
		while (*iptr && isdigit(*iptr)) {
			width += (*iptr - '0');
			iptr++;
			if (isdigit(*iptr))
				width *= 10;
		}
		if (*iptr == '.') {
			iptr++;
			precision = 0;
			while (*iptr && isdigit(*iptr)) {
				precision += (*iptr - '0');
				iptr++;
				if (isdigit(*iptr))
					precision *= 10;
			}
		}

		islong = 0;
		if (*iptr == 'l') {
			islong++;
			iptr++;
		}
		if (*iptr == 'l') {
			islong++;
			iptr++;
		}

		if (*iptr == 'z') {
			if (sizeof(size_t) == sizeof(long long))
				islong = 2;
			else
				islong = 1;
			iptr++;
		}

		if (*iptr == 'p')
			hashash = 1;

		if (hashash) {
			*optr++ = '0';
			if (is_buf_full(outbuf, optr, n))
				break;
			*optr++ = 'x';
			/* Hash steals from the width */
			width = (width > 2) ? width - 2 : 0;
			if (is_buf_full(outbuf, optr, n))
				break;
		}

		switch (*iptr) {
		case 's':
			tmpptr = (unsigned char *)
				va_arg(marker, unsigned char *);
			if (!tmpptr)
				tmpptr = (unsigned char *)"(null)";
			if ((width == 0) && (precision == 0)) {
				while (*tmpptr) {
					*optr++ = *tmpptr++;
					if (is_buf_full(outbuf, optr, n))
						break;
				}
				break;
			}
			i = strlen((char *)tmpptr);
			if (width > i)
				i = width - i;
			else
				i = 0;	/* don't need to print blank spaces */

			while (i) {
				*optr++ = ' ';
				if (is_buf_full(outbuf, optr, n))
					break;
				i--;
			}
			while (!is_buf_full(outbuf, optr, n) && width &&
					*tmpptr) {
				*optr++ = *tmpptr++;
				width--;
			}
			break;
		case 'd':
		case 'i':
			switch (islong) {
			case 0:
			case 1:
				i = va_arg(marker, int);
				if (i < 0) {
					*optr++ = '-';
					if (is_buf_full(outbuf, optr, n))
						break;
					i = -i;
				}
				optr += __atox_n(n, outbuf, optr, i, 10, width,
						 pad, digits);
				break;
			case 2:
				ll = va_arg(marker, long long int);
				if (ll < 0) {
					*optr++ = '-';
					ll = -ll;
					if (is_buf_full(outbuf, optr, n))
						break;
				}
				optr += __llatox_n(n, outbuf, optr, ll, 10,
							width, pad, digits);
				break;
			}
			break;
		case 'u':
			switch (islong) {
			case 0:
			case 1:
				x = va_arg(marker, unsigned int);
				optr += __atox_n(n, outbuf, optr, x, 10, width,
						 pad, digits);
				break;
			case 2:
				lx = va_arg(marker, unsigned long long);
				optr += __llatox_n(n, outbuf, optr, lx, 10,
						   width, pad, digits);
				break;
			}
			break;
		case 'X':
		case 'x':
			switch (islong) {
			case 0:
			case 1:
				x = va_arg(marker, unsigned int);
				optr += __atox_n(n, outbuf, optr, x, 16, width,
						 pad, ((*iptr == 'X') ?
						       digits : ldigits));
				break;
			case 2:
				lx = va_arg(marker, unsigned long long);
				optr += __llatox_n(n, outbuf, optr, lx, 16,
						   width, pad,
						   ((*iptr == 'X') ?
						    digits : ldigits));
				break;
			}
			break;
		case 'p':
#ifdef __long64
			lx = va_arg(marker, unsigned long long);
			optr += __llatox_n(n, outbuf, optr, lx, 16, width, pad,
					   ldigits);
#else
			x = va_arg(marker, unsigned int);
			optr += __atox_n(n, outbuf, optr, x, 16, width, pad,
					 ldigits);
#endif
			break;
		case 'c':
			x = va_arg(marker, int);
			*optr++ = x & 0xff;
			break;

		default:
			*optr++ = *iptr;
			break;
		}

		iptr++;
	}

	*optr = '\0';

	return optr - outbuf;
}

/*  *********************************************************************
    *  xsprintf(buf, format, params..)
    *
    *  format messages from format string into a buffer.
    *
    *  Input Parameters:
    *      buf - output buffer
    *      format - format string
    *      params... parameters
    *
    *  Return Value:
    *      number of bytes copied to buffer
    ********************************************************************* */
int xsprintf(char *buf, const char *format, ...)
{
	va_list marker;
	int count;

	va_start(marker, format);
	count = xvsprintf(buf, format, marker);
	va_end(marker);

	return count;
}

/*  *********************************************************************
    *  xprintf(format,...)
    *
    *  A miniature printf.
    *
    *      %s - unpacked string, null terminated
    *      %x - hex word (machine size)
    *
    *  Return value:
    *	   number of bytes written
    ********************************************************************* */
int printf(const char *format, ...)
{
	va_list marker;
	int count;
	char buffer[PRINTF_BUF_SIZE];

	va_start(marker, format);
	count = xvsnprintf(buffer, PRINTF_BUF_SIZE, format, marker);
	va_end(marker);

	console_xprint(buffer);

	return count;
}

/*  *********************************************************************
    *  xvprintf(format,...)
    *
    *  A miniature vprintf.
    *
    *      %s - unpacked string, null terminated
    *      %x - hex word (machine size)
    *
    *  Return value:
    *	   number of bytes written
    ********************************************************************* */
int xvprintf(const char *format, va_list marker)
{
	int count;
	char buffer[PRINTF_BUF_SIZE];

	count = xvsnprintf(buffer, PRINTF_BUF_SIZE, format, marker);

	console_xprint(buffer);

	return count;
}
