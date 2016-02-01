/***************************************************************************
 *     Copyright (c) 2003-2009, Broadcom Corporation
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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BSTD_DEFS_H__
#define BSTD_DEFS_H__

#if __KERNEL__
#include <linux/types.h>

#define UINT8_C(c)     c ## U
#define UINT16_C(c)    c ## U
#define UINT32_C(c)    c ## U
#define UINT64_C(c)    c ## ULL

#define INT8_C(c)      c
#define INT16_C(c)     c
#define INT32_C(c)     c
#define INT64_C(c)     c ## LL


#define  INT8_MIN (-128)
#define  INT8_MAX ( 127)
#define UINT8_MAX ( 255)

#define  INT16_MIN (-32768)
#define  INT16_MAX ( 32767)
#define UINT16_MAX ( 65535)

#define  INT32_MIN (-2147483648L)
#define  INT32_MAX ( 2147483647L)
#define UINT32_MAX ( 4294967295UL)
#else
#include <stddef.h>
#include <stdint.h>
#endif
#include <stdbool.h>
#include <stdarg.h>

#define BSTD_UNUSED(x) ((void)x)

/* in linux with non preemptive kernel we shall be safe without locking debug interface */
#define BDBG_LOCK()   
#define BDBG_UNLOCK() 

#endif /* BSTD_DEFS_H__ */

