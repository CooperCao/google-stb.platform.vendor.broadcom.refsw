/***************************************************************************
 *     Copyright (c) 1999-2010, Broadcom Corporation
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
const uint32_t BRAP_IMG_wmapro_passthru_tables_array[] = {
	0x00000001,
	0x00000002,
	0x00000003,
	0x00000004,
	0x00000005,
	0x00000006,
	0x00000007,
	0x00000008,
	0x00000009,
	0x0000000a,
	0x0000000b,
	0x0000000c,
	0x0000000f,
	0x0000000e,
	0x0000000d,
	0x0000000c,
	0x0000000b,
	0x0000000a,
	0x00000009,
	0x00000008,
	0x00000007,
	0x00000006,
	0x00000005,
	0x00000004,
	0x00000003,
	0x00000002,
	0x00000001,
	0x00000000,
	0x00000000,
	0x00000001,
	0x00000002,
	0x00000003,
	0x00000004,
	0x00000005,
	0x00000006,
	0x00000007,
	0x00000008,
	0x00000009,
	0x0000000a,
	0x0000000b,
	0x0000000c,
	0x0000000d,
	0x0000000e,
	0x0000000f,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};
const uint32_t BRAP_IMG_wmapro_passthru_tables_header [2] = {sizeof(BRAP_IMG_wmapro_passthru_tables_array), 1};
const void * BRAP_IMG_wmapro_passthru_tables [2] = {BRAP_IMG_wmapro_passthru_tables_header, BRAP_IMG_wmapro_passthru_tables_array};
/* End of File */
