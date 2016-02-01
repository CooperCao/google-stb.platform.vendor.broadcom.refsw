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
 *
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

/****************************************************************************/
/*
 *  SCD Hardware Abstraction Layer header for T314 MEV board
 */
/****************************************************************************/

#ifndef SCD_HAL_H
#define SCD_HAL_H

/****************************************************************************/

#if BTFE_BBS
#ifdef _MSC_VER
	#if _MSC_VER<=1200 /* Only use for Visual C++, 32-bit, version 6.0         1200 or lower*/
		#pragma warning(disable:4115) /* disable warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses */
			#include <windows.h>
		#pragma warning(default:4115) /* enable warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses */
	#else
		#include <windows.h>
	#endif
/*#elif */
/*	#include <windows.h> */
#endif
#endif

/****************************************************************************/
/* constants */
/****************************************************************************/

/* maximum number of chips supported */
#define SCD_MAX_CHIP  4

/* maximum number of FATs supported */
#define SCD_MAX_FAT   8

/* maximum number of FDCs supported */
#define SCD_MAX_FDC   4

/* chip lock semaphore timeout */
#define CHIP_LOCK_TIMEOUT 1000

/****************************************************************************/
/* prototypes */
/****************************************************************************/

SCD_RESULT BTFE_P_HalInitialize(uint32_t flags, void *reg_handle);
SCD_RESULT BTFE_P_HalCleanup(void);

SCD_RESULT BTFE_P_HalOpenChip(uint32_t instance);
SCD_RESULT BTFE_P_HalWriteChip(uint32_t offset, uint32_t length, uint8_t *buffer);
SCD_RESULT BTFE_P_HalReadChip(uint32_t offset, uint32_t length, uint8_t *buffer);
SCD_RESULT BTFE_P_HalCloseChip(uint32_t instance);

/****************************************************************************/

#endif /* SCD_HAL_H */

/****************************************************************************/
