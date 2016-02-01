/***************************************************************************
 *     Copyright (c) 1999-2006, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *
 * THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 * AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 * EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 ***************************************************************************/

#include "bchp.h"
const uint32_t BRAP_IMG_mp3e_scratch_array[] = {
	0x00000000,
	0x00000000,
	0x09010000,
	0x06201411,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};
const uint32_t BRAP_IMG_mp3e_scratch_header [2] = {sizeof(BRAP_IMG_mp3e_scratch_array), 1};
const void * BRAP_IMG_mp3e_scratch [2] = {BRAP_IMG_mp3e_scratch_header, BRAP_IMG_mp3e_scratch_array};
/* End of File */
