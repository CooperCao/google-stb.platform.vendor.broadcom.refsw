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

#ifndef BHSM_CHECKER_GISB_H__
#define BHSM_CHECKER_GISB_H__

#include "bhsm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define  BHSM_MAX_CHECKERS_GISB          (32)

typedef struct BHSM_P_CheckerGisb_s * BHSM_CheckerGisbHandle_t;

typedef struct
{
    uint32_t control;
    uint64_t startAddress;
    uint64_t endAddress;
    BHSM_CheckerGisb_Client_t client;
}BHSM_CheckerGisb_Configuration_t;


/* Called from BHSM_Open on system initialisation. */
BERR_Code BHSM_CheckerGisb_Init( BHSM_Handle hHsm );

/* Called from BHSM_Close on system shutdowm */
BERR_Code BHSM_CheckerGisb_Uninit( BHSM_Handle hHsm );

/* Allocate a GISB Checker */
BERR_Code BHSM_CheckerGisb_Allocate( BHSM_Handle hHsm, BHSM_CheckerGisbHandle_t *phChecker /* out: pointer to sandbox handle */ );

/* Free a GISB Checker */
BERR_Code BHSM_CheckerGisb_Free( BHSM_CheckerGisbHandle_t hChecker );

/* Configure and enable a GISB Checker */
BERR_Code BHSM_CheckerGisb_Enable( BHSM_CheckerGisbHandle_t hChecker,
                                   BHSM_CheckerGisb_Configuration_t *pConfig  /* in */
                                 );

/* Disable a GISB Checker */
BERR_Code BHSM_CheckerGisb_Disable( BHSM_CheckerGisbHandle_t hChecker );

/* Return a GISB Checker's current configuration. */
BERR_Code BHSM_CheckerGisb_GetConfiguration( BHSM_CheckerGisbHandle_t hChecker,
                                             BHSM_CheckerGisb_Configuration_t *pConfig  /* out */
                                           );

#ifdef __cplusplus
}
#endif

#endif /* BHSM_CHECKER_GISB_H__ */
