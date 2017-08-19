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

#include "bstd.h"
#include "bhsm.h"
#include "bhsm_checker_biu.h"


BDBG_MODULE( BHSM );


BERR_Code BHSM_CheckerBiu_Init( BHSM_Handle hHsm )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_CheckerBiu_Init );
    BSTD_UNUSED( hHsm );

    BDBG_LEAVE( BHSM_CheckerBiu_Init );
    return rc;
}


BERR_Code BHSM_CheckerBiu_Uninit( BHSM_Handle hHsm )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_CheckerBiu_Uninit );
    BSTD_UNUSED( hHsm );


    BDBG_LEAVE( BHSM_CheckerBiu_Uninit );
    return rc;
}


BERR_Code BHSM_CheckerBiu_Allocate( BHSM_Handle hHsm, BHSM_CheckerBiuHandle_t *phChecker )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_CheckerBiu_Allocate );
    BSTD_UNUSED( hHsm );
    BSTD_UNUSED( phChecker );


    BDBG_LEAVE( BHSM_CheckerBiu_Allocate );
    return rc;
}


BERR_Code BHSM_CheckerBiu_Free( BHSM_CheckerBiuHandle_t hChecker )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_CheckerBiu_Free );
    BSTD_UNUSED( hChecker );


    BDBG_LEAVE( BHSM_CheckerBiu_Free );
    return rc;
}


BERR_Code BHSM_CheckerBiu_Enable( BHSM_CheckerBiuHandle_t hChecker,
                                   BHSM_CheckerBiu_Configuration_t *pConfig )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_CheckerBiu_Enable );
    BSTD_UNUSED( hChecker );
    BSTD_UNUSED( pConfig );


    BDBG_LEAVE( BHSM_CheckerBiu_Enable );
    return rc;
}


BERR_Code BHSM_CheckerBiu_Disable( BHSM_CheckerBiuHandle_t hChecker )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_CheckerBiu_Disable );
    BSTD_UNUSED( hChecker );


    BDBG_LEAVE( BHSM_CheckerBiu_Disable );
    return rc;
}


BERR_Code BHSM_CheckerBiu_GetConfiguration( BHSM_CheckerBiuHandle_t hChecker,
                                            BHSM_CheckerBiu_Configuration_t *pConfig )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_CheckerBiu_GetConfiguration );
    BSTD_UNUSED( hChecker );
    BSTD_UNUSED( pConfig );


    BDBG_LEAVE( BHSM_CheckerBiu_GetConfiguration );
    return rc;
}
