/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef BMXT_DCBG_H__
#define BMXT_DCBG_H__

#include "bstd.h"
#include "breg_mem.h"
#include "bchp.h"
#include "bmem.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BMXT_MAX_NUM_DCBG 8

typedef struct BMXT_P_DcbgHandle *BMXT_Dcbg_Handle;

typedef struct BMXT_Dcbg_OpenSettings
{
    unsigned dummy;
}
BMXT_Dcbg_OpenSettings;

unsigned BMXT_Dcbg_GetFreeIndex(
    BMXT_Handle hMxt
    );

void BMXT_Dcbg_GetDefaultOpenSettings(
    BMXT_Dcbg_OpenSettings *pSettings
    );

BERR_Code BMXT_Dcbg_Open(
    BMXT_Handle hMxt,
    BMXT_Dcbg_Handle *phDcbg, /* [out] handle for new group */
    unsigned index,           /* [in] HW index */
    BMXT_Dcbg_OpenSettings *pSettings
    );

void BMXT_Dcbg_Close(
    BMXT_Dcbg_Handle hDcbg
    );

typedef struct BMXT_Dcbg_Settings
{
    unsigned primaryBand;      /* primary parser band number */
} BMXT_Dcbg_Settings;

void BMXT_Dcbg_GetSettings(
    BMXT_Dcbg_Handle hDcbg,
    BMXT_Dcbg_Settings *pSettings
    );

BERR_Code BMXT_Dcbg_SetSettings(
    BMXT_Dcbg_Handle hDcbg,
    const BMXT_Dcbg_Settings *pSettings
    );

BERR_Code BMXT_Dcbg_AddParser(
    BMXT_Dcbg_Handle hDcbg,
    unsigned parserNum
    );

BERR_Code BMXT_Dcbg_RemoveParser(
    BMXT_Dcbg_Handle hDcbg,
    unsigned parserNum
    );

BERR_Code BMXT_Dcbg_RemoveAllParsers(
    BMXT_Dcbg_Handle hDcbg
    );

void BMXT_Dcbg_GetDefaultSettings(
    BMXT_Dcbg_Handle hDcbg,
    BMXT_Dcbg_Settings *pSettings
    );

BERR_Code BMXT_Dcbg_Start(
    BMXT_Dcbg_Handle hDcbg,
    const BMXT_Dcbg_Settings *pSettings
    );

void BMXT_Dcbg_Stop(
    BMXT_Dcbg_Handle hDcbg
    );

typedef struct BMXT_Dcbg_Status
{
    bool locked; /* if true, chunk sequencer is locked and reading out all band data in sequence */
} BMXT_Dcbg_Status;

BERR_Code BMXT_Dcbg_GetStatus(
    BMXT_Dcbg_Handle hDcbg,
    BMXT_Dcbg_Status *pStatus
    );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BMXT_DCBG_H__ */
