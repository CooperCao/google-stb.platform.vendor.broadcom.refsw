/***************************************************************************
 *     Copyright (c) 1999-2004, Broadcom Corporation
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
const uint32_t BRAP_IMG_ddp_passthru_interframe_buf_array[] = {
	0x00000000,
};
const uint32_t BRAP_IMG_ddp_passthru_interframe_buf_header [2] = {sizeof(BRAP_IMG_ddp_passthru_interframe_buf_array), 1};
const void * BRAP_IMG_ddp_passthru_interframe_buf [2] = {BRAP_IMG_ddp_passthru_interframe_buf_header, BRAP_IMG_ddp_passthru_interframe_buf_array};
/* End of File */
