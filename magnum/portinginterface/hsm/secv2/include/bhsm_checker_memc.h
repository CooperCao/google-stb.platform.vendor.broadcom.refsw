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

#ifndef BHSM_CHECKER_MEMC_H__
#define BHSM_CHECKER_MEMC_H__

#include "bhsm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  BHSM_SANDBOX_MAX_MEMCARCH      (12)
#define  BHSM_SANDBOX_MEMARCH_MAX_CLIENTS  (8)

typedef struct BHSM_P_CheckerMemc_t* BHSM_CheckerMemcHandle_t;

/* TODO .. consider replaceing this with a 256 bit array. */
typedef struct BHSM_CheckerMemc_Client_t {
    uint32_t read;
    uint32_t write;
}BHSM_CheckerMemc_Client_t;


typedef struct
{
    uint32_t control;
    uint64_t startAddress;
    uint64_t endAddress;
    BHSM_CheckerMemc_Client_t client[BHSM_SANDBOX_MEMARCH_MAX_CLIENTS];
    bool enableBackgroundCheck;

    bool xptRegionAssociated;       /* if true, an XPT region is associated with this arch. Region paramters in following xptRegion struct */
    struct{
        uint8_t number;             /* region number    */
        bool psubCrossing;          /* true to enalbe   */
        bool mcpb0Crossing;         /* true to enalbe   */
        bool memdmaMcpbCrossing;    /* true to enalbe   */
    }xptRegion;

}BHSM_CheckerMemc_Configuration_t;


#define BHSM_SANDBOX_MEMARCH_UPDATE_CLIENTS (1<<0)   /* update MEM_ARCH clients */
#define BHSM_SANDBOX_MEMARCH_UPDATE_RANGE   (1<<1)   /* update MEM_ARCH range   */

typedef struct
{
    uint32_t  updateSelect;   /* select what paramters to update, use  BHSM_SANDBOX_MEMARCH_UPDATE_*  */
    BHSM_CheckerMemc_Client_t client[BHSM_SANDBOX_MEMARCH_MAX_CLIENTS];  /* arch clients. used if "updateSelect | BHSM_SANDBOX_MEMARCH_UPDATE_CLIENTS"  */
    struct{                                                          /* arch range.   used if "updateSelect | BHSM_SANDBOX_MEMARCH_UPDATE_RANGE" */
        uint64_t startAddress;
        uint64_t endAddress;
    }range;
}BHSM_CheckerMemc_Update_t;


/* Called from BHSM_Open on platform initialisation. This function will poll BSP for a list of open MEMC Checker, i
   and close them if the exist. */
BERR_Code BHSM_CheckerMemc_Init( BHSM_Handle hHsm );

/* Called from BHSM_Close on platform termination. */
BERR_Code BHSM_CheckerMemc_Uninit( BHSM_Handle hHsm );

/* Allocate a CheckerMemc */
BERR_Code BHSM_CheckerMemc_Allocate( BHSM_Handle hHsm,
                                     BHSM_CheckerMemcHandle_t *phChecker      /* out: pointer to MEMC Checker */
                                   );

BERR_Code BHSM_CheckerMemc_Free( BHSM_CheckerMemcHandle_t hChecker );

/* Configure and activate CheckerMemc. */
BERR_Code BHSM_CheckerMemc_Enable( BHSM_CheckerMemcHandle_t hChecker,
                                   const BHSM_CheckerMemc_Configuration_t *pConfig  /* in */
                                 );


/* Deactivate the CheckerMemc. */
BERR_Code BHSM_CheckerMemc_Disable( BHSM_CheckerMemcHandle_t hChecker);

/* Update the CheckerMemc's sandbox configuration. */
BERR_Code BHSM_CheckerMemc_Update( BHSM_CheckerMemcHandle_t hChecker,
                                   const BHSM_CheckerMemc_Update_t *pUpdate         /* in */
                                 );

/* Get the CheckerMemc's current configuration. */
BERR_Code BHSM_CheckerMemc_GetConfiguration( BHSM_CheckerMemcHandle_t hChecker,
                                             BHSM_CheckerMemc_Configuration_t *pConfig  /* out */
                                           );

#ifdef __cplusplus
}
#endif

#endif /* BHSM_CHECKER_MEMC_H__ */
