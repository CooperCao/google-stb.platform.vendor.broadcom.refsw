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

#ifndef BSAGELIB_H_
#define BSAGELIB_H_

#include "breg_mem.h"
#include "bchp.h"
#include "bint.h"
#include "btmr.h"
#include "bhsm.h"

#include "bsagelib_interfaces.h"
#include "bsagelib_types.h"

/* Host to Sage communication buffers size */
#define SAGE_HOST_BUF_SIZE (32)

#ifdef __cplusplus
extern "C" {
#endif


/* This handle is used to store the context of a SAGElib instance */
typedef struct BSAGElib_P_Instance *BSAGElib_Handle;


/*
 * SAGElib instance settings structure
 * Passed upon BSAGElib_Open() call in order to configure the SAGE library
 * - provides corre handles
 * - provides interfaces to integrate the lib in within upper software arcchitecture
 *
 * See BSAGElib_GetDefaultSettings, BSAGElib_Open
*/
typedef struct {

    /* core module handles */
    BREG_Handle hReg;
    BCHP_Handle hChp;
    BINT_Handle hInt;
    BTMR_Handle hTmr;
    BHSM_Handle hHsm;

    /* default interfaces */
    BSAGElib_MemoryMapInterface i_memory_map;
    BSAGElib_MemorySyncInterface i_memory_sync;
    BSAGElib_MemorySyncInterface i_memory_sync_isrsafe;
    BSAGElib_MemoryAllocInterface i_memory_alloc;/* malloc functions should return 16 bytes aligned blocks. */
    BSAGElib_SyncInterface i_sync_sage; /* Sync can be overriden by instance */
    BSAGElib_SyncInterface i_sync_hsm; /* Sync can be overriden by instance */

    uint8_t enablePinmux;
#ifdef SAGE_KO
    BSAGE_func  bsage;
#endif
} BSAGElib_Settings;

/* Get default SAGE library settings structure.
 * Mandatory to use for retro compatibility uppon updates */
void
BSAGElib_GetDefaultSettings(
    BSAGElib_Settings *pSettings /* [in/out] */);


typedef struct {
    BHSM_Handle hHsm;
} BSAGElib_DynamicSettings;

BERR_Code
BSAGElib_GetDynamicSettings(
    BSAGElib_Handle hSAGElib,
    BSAGElib_DynamicSettings *settings);

BERR_Code
BSAGElib_SetDynamicSettings(
    BSAGElib_Handle hSAGElib,
    BSAGElib_DynamicSettings *settings);


/* Initialize SAGE library and retreive main handle */
BERR_Code
BSAGElib_Open(
    BSAGElib_Handle *pSAGElibHandle, /* [out] SAGElib handle */
    BSAGElib_Settings *settings
    );

/* Uninitialize the SAGE library
 * None of other SAGE lib API can be used until BSAGElib_Open is called */
void
BSAGElib_Close(
    BSAGElib_Handle hSAGElib);

/* Chipset type */
typedef enum
{
    BSAGElib_ChipType_eZS = 0,
    BSAGElib_ChipType_eZB,
    BSAGElib_ChipType_eCustomer,
    BSAGElib_ChipType_eCustomer1
} BSAGElib_ChipType_e;

/* Chipset static information, comming from OTP */
typedef struct
{
    BSAGElib_ChipType_e chipType;
} BSAGElib_ChipInfo;

BERR_Code
BSAGElib_GetChipInfo(
    BSAGElib_Handle hSAGElib,
    BSAGElib_ChipInfo *pChipInfo);

typedef struct {
    struct {
        bool secured;
    } urr;
} BSAGElib_Status;
BERR_Code
BSAGElib_GetStatus(
    BSAGElib_Handle hSAGElib,
    BSAGElib_Status *pStatus);

BERR_Code
BSAGElib_GetLogWriteCount(
    BSAGElib_Handle hSAGElib,
    uint64_t *size);
#ifdef __cplusplus
}
#endif


#endif /* BSAGELIB_H_ */
