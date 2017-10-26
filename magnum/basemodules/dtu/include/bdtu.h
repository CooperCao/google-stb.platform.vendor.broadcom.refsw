/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef DTU_H__
#define DTU_H__

#include "bchp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DTU is DRAM Translation Unit
It allows large contiguous blocks of device memory to be built by remapping bus addresses. */

typedef struct BDTU *BDTU_Handle;

#define BDTU_INVALID_ADDR (BSTD_DeviceOffset)(0)

typedef struct BDTU_MappingInfo
{
    unsigned memcIndex;
    struct {
        BSTD_DeviceOffset addr; /* base physical address of contiguous addressing region */
        uint64_t size; /* size of contiguous addressing */
    } region[BCHP_MAX_MEMC_REGIONS]; /* supports multiple discontiguous addressing region */
} BDTU_MappingInfo;

typedef struct BDTU_CreateSettings
{
    BREG_Handle reg;
    BDTU_MappingInfo memoryLayout; /* This is HW based mapping, NOT populated memory */
    unsigned ownerId; /* host's GISB ID, used to validate BDTU_Own calls */
} BDTU_CreateSettings;

void BDTU_GetDefaultCreateSettings(
    BDTU_CreateSettings *pSettings
    );

BERR_Code BDTU_Create(
    BDTU_Handle *pHandle,
    const BDTU_CreateSettings *pSettings
    );

void BDTU_Destroy(
    BDTU_Handle handle
    );

#define BDTU_REMAP_LIST_TOTAL 32
typedef struct BDTU_RemapSettings
{
    struct {
        BSTD_DeviceOffset orgPhysAddr; /* original BA. should match either fromPhysAddr or toPhysAddr. */
        BSTD_DeviceOffset fromPhysAddr; /* current BA (bus address). Must be within BDTU_CreateSettings.memoryLayout range and be 2MB aligned. */
        BSTD_DeviceOffset toPhysAddr; /* new BA (bus address). Must be within BDTU_CreateSettings.memoryLayout range and be 2MB aligned. */
    } list[BDTU_REMAP_LIST_TOTAL]; /* list terminates at first entry with devAddr == 0 */
} BDTU_RemapSettings;

void BDTU_GetDefaultRemapSettings(
    BDTU_RemapSettings *pSettings
    );

/*
All memory begins with identity map, where BA == DA.
Remapping means unmapping DA from BA0 then mapping it to BA1.
*/
BERR_Code BDTU_Remap(
    BDTU_Handle handle,
    const BDTU_RemapSettings *pSettings
    );

/* returns the original BA for this remapped BA.
If not remapped, orgPhysAddr will be equal to physAddr.
Function fails with non-zero if BA is not mapped */
BERR_Code BDTU_ReadOriginalAddress(
    BDTU_Handle handle,
    BSTD_DeviceOffset physAddr, /* current BA */
    BSTD_DeviceOffset *orgPhysAddr /* orignal BA */
    );

void BDTU_PrintMap(
    BDTU_Handle handle,
    BSTD_DeviceOffset addr,
    unsigned size
    );

BERR_Code BDTU_Own(
    BDTU_Handle handle,
    BSTD_DeviceOffset addr,
    bool own
    );

BERR_Code BDTU_Scrub(
    BDTU_Handle handle,
    BSTD_DeviceOffset addr,
    unsigned size
    );

typedef struct BDTU_PageInfo
{
    BSTD_DeviceOffset deviceOffset;
    bool valid;
    bool owned;
    bool scrubbing;
    uint8_t ownerID;
} BDTU_PageInfo;

BERR_Code BDTU_ReadInfo(
    BDTU_Handle handle,
    BSTD_DeviceOffset physAddr,
    BDTU_PageInfo *info
    );

typedef enum BDTU_State
{
    BDTU_State_eUnknown,
    BDTU_State_eUnset,
    BDTU_State_eEnabled,
    BDTU_State_eDisabled,
    BDTU_State_eMax
} BDTU_State;

typedef struct BDTU_Status
{
    BDTU_State state;
} BDTU_Status;

BERR_Code BDTU_GetStatus(
    BDTU_Handle handle,
    BDTU_Status *pStatus
    );

#ifdef __cplusplus
}
#endif

#endif
