/***************************************************************************
 *     Copyright (c) 2006-2008, Broadcom Corporation
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
 * BASE64 encoder/decoder
 * 
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *******************************************************************************/
#include "bstd.h"
#include "bbase64.h"

BDBG_MODULE(bbase64);

static const char b_base64_set[65]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int
bbase64_encode(const uint8_t *src, size_t src_len, char *dest, size_t dst_len)
{
	size_t src_off;
	size_t dst_off;
	unsigned acc;

	for(src_off=dst_off=0;(src_off+3)<=src_len && (dst_off+4)<=dst_len; src_off+=3,dst_off+=4) {
		acc =  (((unsigned)src[src_off+0])<<16) | (((unsigned)src[src_off+1])<<8) | src[src_off+2];
		dest[dst_off+0] = b_base64_set[(acc>>(24-6))&0x3F];
		dest[dst_off+1] = b_base64_set[(acc>>(24-12))&0x3F];
		dest[dst_off+2] = b_base64_set[(acc>>(24-18))&0x3F];
		dest[dst_off+3] = b_base64_set[acc&0x3F];
	}
	switch(src_len - src_off) {
	default:
		return -1;
	case 0: 
		return  (int)dst_off;
	case 1:
		if ((dst_off+4)>dst_len) {
			return -1;
		}
		acc =  (((unsigned)src[src_off+0])<<16);
		dest[dst_off+0] = b_base64_set[(acc>>(24-6))&0x3F];
		dest[dst_off+1] = b_base64_set[(acc>>(24-12))&0x3F];
		dest[dst_off+2] = '=';
		dest[dst_off+3] = '=';
		dst_off+=4;
		break;
	case 2:
		if ((dst_off+4)>dst_len) {
			return -1;
		}
		acc =  (((unsigned)src[src_off+0])<<16) | (((unsigned)src[src_off+1])<<8);
		dest[dst_off+0] = b_base64_set[(acc>>(24-6))&0x3F];
		dest[dst_off+1] = b_base64_set[(acc>>(24-12))&0x3F];
		dest[dst_off+2] = b_base64_set[(acc>>(24-18))&0x3F];
		dest[dst_off+3] = '=';
		dst_off+=4;
		break;
	}
	return  (int)dst_off;
}

static int
bbase64_decode_unit(char ch)
{
	int index;

	if (ch >= 'A' && ch <= 'Z') {
		index = ch - 'A';
	} else if (ch >= 'a' && ch <= 'z') {
		index = 26+(ch - 'a');
	} else if (ch >= '0' && ch <= '9') {
		index = 52+(ch - '0');
	} else if (ch=='+') {
		index = 62;
	} else if (ch=='/') {
		index = 63;
	} else if (ch=='=') {
		return 0;
	} else {
		return -1;
	}
	BDBG_MSG(("%c -> %d", ch, index));
	BDBG_ASSERT(b_base64_set[index]==ch);
	return index;
}

int
bbase64_decode(const char *src, size_t src_len, uint8_t *dest, size_t dst_len)
{
	size_t src_off;
	size_t dst_off;
	unsigned acc;

	for(src_off=dst_off=0;(src_off+4)<=src_len && (dst_off+3)<=dst_len; src_off+=4,dst_off+=3) {
		int index0,index1,index2,index3;
		if (src[src_off+0]=='=' || src[src_off+1]=='=') {
			return -1;
		}
		index0 = bbase64_decode_unit(src[src_off+0]);
		index1 = bbase64_decode_unit(src[src_off+1]);
		index2 = bbase64_decode_unit(src[src_off+2]);
		index3 = bbase64_decode_unit(src[src_off+3]);
		if(index0<0 || index1<0 || index2<0 || index3<0) {
			return -1;
		}
		acc = (index0 <<(3*6)) | (index1 <<(2*6)) | (index2 <<(1*6)) |  index3;
		BDBG_MSG(("%#x", acc));
		dest[dst_off+0] = acc>>16;
		if(src[src_off+2]=='=') {
			return dst_off+1;
		}
		dest[dst_off+1] = (acc>>8);
		if(src[src_off+3]=='=') {
			return dst_off+2;
		}
		dest[dst_off+2] = acc;
	}
	return dst_off;
}

