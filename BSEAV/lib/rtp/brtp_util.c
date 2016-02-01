/***************************************************************************
 *     Copyright (c) 2006-2013, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * RTP parser library
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "brtp_util.h"

static int
b_get_hex_one(char c)
{
	if(c>='0' && c<='9') {
		return c-'0';
	} else if (c>='A' && c<='F') {
		return 0x0a+c-'A';
	} else if (c>='a' && c<='f') {
		return 0x0a+c-'A';
	} else {
		return -1;
	}
}

int
bhex_decode(const char *src, size_t src_len, uint8_t *dest, size_t dst_len)
{
	size_t src_off;
	size_t dst_off;

	for(src_off=dst_off=0;(src_off+2)<=src_len && dst_off<dst_len;src_off+=2, dst_off++) {
		int ch1, ch2;

		ch1 = b_get_hex_one(src[src_off]);
		ch2 = b_get_hex_one(src[src_off+1]);
		if (ch1<0 || ch2<0) {
			return -1;
		}
		dest[dst_off] = (uint8_t)((ch1<<4)|ch2);
	}
	return dst_off;
}


