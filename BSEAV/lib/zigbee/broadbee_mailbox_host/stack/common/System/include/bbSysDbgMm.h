/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************************/

/*******************************************************************************
 *
 * DESCRIPTION:
 *      Memory Manager debug interface
 *
*******************************************************************************/

#ifndef _SYS_DBG_MM_H
#define _SYS_DBG_MM_H

/************************* INCLUDES *****************************************************/
#include "bbSysBasics.h"

/************************* TYPES ********************************************************/

/**//**
 * \brief SYS_DbgMmGetFreeBlocks descriptor prototype
 */
typedef struct _SYS_DbgMmGetFreeBlocksReqDescr_t SYS_DbgMmGetFreeBlocksReqDescr_t;

/**//**
 * \brief SYS_DbgMmGetFreeBlocks confirmation type
 */
typedef struct _SYS_DbgMmGetFreeBlocksConfParams_t
{
    uint32_t    NumOfFreeBlocks;
} SYS_DbgMmGetFreeBlocksConfParams_t;

/**//**
 * \brief SYS_DbgMmGetFreeBlocks confirmation function type
 */
typedef void SYS_DbgMmGetFreeBlocksConfCallback_t(SYS_DbgMmGetFreeBlocksReqDescr_t *const reqDescr,
        SYS_DbgMmGetFreeBlocksConfParams_t *const conf);

/**//**
 * \brief SYS_DbgMmGetFreeBlocks descriptor structure
 */
struct _SYS_DbgMmGetFreeBlocksReqDescr_t
{
    SYS_DbgMmGetFreeBlocksConfCallback_t    *callback;
};

/************************* PROTOTYPES **************************************************/

/************************************************************************************//**
  \brief Request to get number of available free memory blocks
  \param[in] reqDescr - pointer to the request structure.
****************************************************************************************/
void SYS_DbgMmGetFreeBlocks(SYS_DbgMmGetFreeBlocksReqDescr_t *const req);

#endif /* _SYS_DBG_MM_H */

/* eof bbSysDbgMm.h */