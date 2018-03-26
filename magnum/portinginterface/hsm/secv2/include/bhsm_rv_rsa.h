/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef BHSM_RV_RSA_H__
#define BHSM_RV_RSA_H__

#include "bstd.h"
#include "bkni.h"
#include "bhsm.h"
#include "berr_ids.h"

#include "bhsm_common.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct BHSM_P_RvRsa* BHSM_RvRsaHandle;

typedef struct
{
    unsigned rsaKeyId;
} BHSM_RvRsaAllocateSettings;

typedef struct{
    unsigned rsaKeyId;
}BHSM_RvRsaInfo;

/*
Description:
    Configuration parameters that apply to a region.
*/
typedef struct
{
    bool multiTier;
    unsigned  multiTierSourceKeyId;     /* valid if multiTier is true. */

    BHSM_SigningAuthority rootKey;      /* valid if multiTier is NOT true. */

    BSTD_DeviceOffset keyOffset;        /* offset to key/certificate. */

} BHSM_RvRsaSettings;


/*
Description:
    Allocate a RvRsa. NULL is returned if no resource is available.
*/
BHSM_RvRsaHandle BHSM_RvRsa_Allocate( BHSM_Handle hHsm,
                                      const BHSM_RvRsaAllocateSettings *pSettings );

/*
Description:
    Free a RvRsa and clear the associated RSA KEY ID.
*/
void BHSM_RvRsa_Free( BHSM_RvRsaHandle handle );


/*
Description:
    Set RvRsa configuration.
*/
BERR_Code BHSM_RvRsa_SetSettings( BHSM_RvRsaHandle handle,
                                  const BHSM_RvRsaSettings *pSettings );


BERR_Code BHSM_RvRsa_GetInfo( BHSM_RvRsaHandle handle,
                              BHSM_RvRsaInfo *pRvRsaInfo );


/* ###################   private to HSM   ################# */
typedef struct{
    unsigned dummy;
}BHSM_RvRsaModuleSettings;

/* called internally on platform initialisation */
BERR_Code BHSM_RvRsa_Init( BHSM_Handle hHsm, BHSM_RvRsaModuleSettings *pSettings );


/* called internally on platform un-initialisation */
void BHSM_RvRsa_Uninit( BHSM_Handle hHsm );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
