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
#ifndef DTU_H__
#define DTU_H__

#ifdef __cplusplus
extern "C" {
#endif

/* DTU is DRAM Translation Unit
It allows large contiguous blocks of device memory to be built by remapping bus addresses. */

typedef struct BDTU *BDTU_Handle;

typedef struct BDTU_CreateSettings
{
    BREG_Handle reg;
    unsigned memcIndex;
    BSTD_DeviceOffset physAddrBase; /* physical base address of MEMC */
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

/*
All memory begins with identity map, where BA == DA.
Remapping means unmapping DA from BA0 then mapping it to BA1.
*/
BERR_Code BDTU_Remap(
    BDTU_Handle handle,
    BSTD_DeviceOffset devAddr, /* DA (device address). Starts from 0 on every MEMC. Must be 2MB aligned and within actual populated DRAM. */
    BSTD_DeviceOffset fromPhysAddr, /* current BA (bus address). Must be within BDTU_CreateSettings.physAddr range and be 2MB aligned. */
    BSTD_DeviceOffset toPhysAddr /* new BA (bus address). Must be within BDTU_CreateSettings.physAddr range and be 2MB aligned. */
    );

/* returns the DA of BA.
Function fails with non-zero if BA is not mapped */
BERR_Code BDTU_ReadDeviceAddress(
    BDTU_Handle handle,
    BSTD_DeviceOffset physAddr, /* BA */
    BSTD_DeviceOffset *devAddr /* returns DA to which this BA is currently mapped */
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
