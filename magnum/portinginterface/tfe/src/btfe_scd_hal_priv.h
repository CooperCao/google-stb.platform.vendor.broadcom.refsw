/***************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 * [File Description:]
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
