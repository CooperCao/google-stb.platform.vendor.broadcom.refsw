/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

/***************************************************************************************
Interface decription.
***************************************************************************************/
#ifndef BHSM_RV_REGION_H__
#define BHSM_RV_REGION_H__

#include "bstd.h"
#include "bkni.h"
#include "bhsm.h"
#include "berr_ids.h"

#include "bhsm_common.h"
#include "bhsm_rv_rsa.h"
#include "bhsm_keyladder.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BHSM_RV_REGION_ANY_SUBTYPE_INDEX 0xFF

typedef struct BHSM_P_RvRegion* BHSM_RvRegionHandle;

typedef enum
{
    BHSM_RegionId_eCpu0          = 0,
    BHSM_RegionId_eCpu1          = 0x01,
    BHSM_RegionId_eCpu2          = 0x02,
    BHSM_RegionId_eCpu3          = 0x03,
    BHSM_RegionId_eCpu4          = 0x04,
    BHSM_RegionId_eCpu5          = 0x05,
    BHSM_RegionId_eCpu6          = 0x06,
    BHSM_RegionId_eCpu7          = 0x07,
    BHSM_RegionId_eRave0         = 0x08,
    BHSM_RegionId_eAudio0        = 0x09,
    BHSM_RegionId_eRedacted_0x0A = 0x0A,
    BHSM_RegionId_eVdec0_Il2a    = 0x0B,
    BHSM_RegionId_eVdec0_Ila     = 0x0C,
    BHSM_RegionId_eVdec0_Ola     = 0x0D,
    BHSM_RegionId_eAvs           = 0x0E,
    BHSM_RegionId_eRedacted_0x0E = 0x0E,
    BHSM_RegionId_eVice0_Pic     = 0x0F,
    BHSM_RegionId_eVice0_Mb      = 0x10,
    BHSM_RegionId_eRedacted_0x11 = 0x11,
    BHSM_RegionId_eRedacted_0x12 = 0x12,
    BHSM_RegionId_eRedacted_0x13 = 0x13,
    BHSM_RegionId_eRedacted_0x14 = 0x14,
    BHSM_RegionId_eVdec1_Ila     = 0x15,
    BHSM_RegionId_eVdec1_Ola     = 0x16,
    BHSM_RegionId_eRedacted_0x17 = 0x17,
    BHSM_RegionId_eRedacted_0x18 = 0x18,
    BHSM_RegionId_eRedacted_0x19 = 0x19,
    BHSM_RegionId_eRedacted_0x1A = 0x1A,
    BHSM_RegionId_eRedacted_0x1B = 0x1B,
    BHSM_RegionId_eRedacted_0x1C = 0x1C,
    BHSM_RegionId_eRedacted_0x1D = 0x1D,
    BHSM_RegionId_eRedacted_0x1E = 0x1E,
    BHSM_RegionId_eMax
}BHSM_RegionId;

typedef struct
{
    BHSM_RegionId regionId;
} BHSM_RvRegionAllocateSettings;

typedef struct
{
    BHSM_RegionId regionId;
} BHSM_RvRegionInfo;

/* Configuration parameters that apply to a region.  */
typedef struct
{
    struct
    {
        unsigned size;                    /* The size of memory region to be verified. */
        BMMA_DeviceOffset address;        /* The address of the memory region. Valid if "size" is > 0. */
        BMMA_DeviceOffset destAddress;    /* Some regions will need to be decryted into a
                                             different location where it will be authenticated. */
    }range[2];                            /* memory range(s) to be authenticated. */

    struct
    {
        BMMA_DeviceOffset address;        /* Valid if "size" is > 0. */
        unsigned size;                    /* the size of the signature/parameters. */
    }signature;                           /* the signature.  */

    struct
    {
        BMMA_DeviceOffset address;
    }parameters;                          /* the paramters.  */

    BHSM_RvRsaHandle rvRsaHandle;         /* handle to RV RSA key slot. */

    BHSM_KeyLadderHandle keyLadderHandle; /* sage only. */
    unsigned keyLadderLayer;              /* sage only. KeyLadder layer to read key from. */

    unsigned intervalCheckBandwidth;      /* valid values 0x1 to 0x10*/
    bool     resetOnVerifyFailure;
    bool     instrCheck;                  /* sage only. Enables instruction checkers. */
    bool     backgroundCheck;
    bool     allowRegionDisable;
    bool     enforceAuth;                 /* override MSP OTP for region to force authentication. */

} BHSM_RvRegionSettings;


/* The status of a region */
typedef struct
{
    bool configured;
    bool verified;
} BHSM_RvRegionStatus;


/* Allocate a RvRegion. NULL is returned if no resource is available.  */
BHSM_RvRegionHandle BHSM_RvRegion_Allocate( BHSM_Handle hHsm,
                                            const BHSM_RvRegionAllocateSettings *pSettings );

/* Free a RvRegion */
void BHSM_RvRegion_Free( BHSM_RvRegionHandle handle );

/* Get RvRegion configuration.  */
void  BHSM_RvRegion_GetSettings( BHSM_RvRegionHandle handle,
                                 BHSM_RvRegionSettings *pSettings );

/* Set RvRegion configuration.  */
BERR_Code BHSM_RvRegion_SetSettings( BHSM_RvRegionHandle handle,
                                     const BHSM_RvRegionSettings *pSettings );

/* Enable the RvRegion.  */
BERR_Code BHSM_RvRegion_Enable( BHSM_RvRegionHandle handle );


/* Disable the RvRegion.  */
BERR_Code BHSM_RvRegion_Disable( BHSM_RvRegionHandle handle );


/* Return the status of a region.  */
BERR_Code BHSM_RvRegion_GetStatus( BHSM_RvRegionHandle handle,
                                   BHSM_RvRegionStatus *pStatus );

/* Return RvRegion information.  */
BERR_Code BHSM_GetRvRegionInfo( BHSM_RvRegionHandle handle,
                                BHSM_RvRegionInfo *pInfo );





/* ###################   private to HSM   ################# */

typedef struct{
        unsigned dummy;
}BHSM_RvRegionModuleSettings;

/* called internally on platform initialisation */
BERR_Code BHSM_RvRegion_Init( BHSM_Handle hHsm, BHSM_RvRegionModuleSettings *pSettings );

/* called internally on platform un-initialisation */
void BHSM_RvRegion_Uninit( BHSM_Handle hHsm );


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
