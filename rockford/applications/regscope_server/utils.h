/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
 *   The source code for the stub ikos server.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef UTILS_H__
#define UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define BTST_REG_OFFSET_MASK (~0xF0000000)
void jump_setup (void);
unsigned long deref32 (
	unsigned long seq, unsigned long addr, void* offset, unsigned long* regval);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H__ */
