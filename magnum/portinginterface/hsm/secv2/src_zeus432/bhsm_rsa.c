/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bhsm_priv.h"
#include "bhsm_rsa.h"

BDBG_MODULE( BHSM );


typedef struct BHSM_Rsa
{
    BHSM_Handle hHsm;
    bool inUse;    /* only one instance allowed.  */
}BHSM_Rsa;

BERR_Code BHSM_Rsa_Init( BHSM_Handle hHsm, BHSM_RsaModuleSettings *pSettings )
{
    BHSM_Rsa *pRsa;

    BDBG_ENTER( BHSM_Rsa_Init );
    BSTD_UNUSED( pSettings );

    if( hHsm->modules.pRsa != NULL ) { return BERR_TRACE(BERR_UNKNOWN); }

    pRsa = (BHSM_Rsa*)BKNI_Malloc( sizeof(BHSM_Rsa) );
    if( !pRsa ){ return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY); }

    BKNI_Memset( pRsa, 0, sizeof(*pRsa) );
    pRsa->hHsm = hHsm;
    hHsm->modules.pRsa = (void*)pRsa;

    BDBG_ENTER( BHSM_Rsa_Init );
    return BERR_SUCCESS;
}

void BHSM_Rsa_Uninit( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_Rsa_Uninit );

    if( !hHsm->modules.pRsa ) { BERR_TRACE(BERR_UNKNOWN); return; }

    BKNI_Free( hHsm->modules.pRsa );
    hHsm->modules.pRsa = NULL;

    BDBG_ENTER( BHSM_Rsa_Uninit );
    return;
}

BHSM_RsaHandle BHSM_Rsa_Open( BHSM_Handle hHsm )
{
    BHSM_Rsa *pRsa;

    BDBG_ENTER( BHSM_Rsa_Open );

    pRsa = hHsm->modules.pRsa; /* there is only one RSA instance in the system/chip. */
    if( !pRsa ) { BERR_TRACE(BERR_UNKNOWN); return NULL; }

    if( pRsa->inUse == true) { BERR_TRACE(BERR_NOT_AVAILABLE); return NULL; }

    pRsa->inUse = true;

    BDBG_LEAVE( BHSM_Rsa_Open );
    return (BHSM_RsaHandle)pRsa;
}


void BHSM_Rsa_Close( BHSM_RsaHandle handle )
{
    BHSM_Rsa *pRsa = handle;

    BDBG_ENTER( BHSM_Rsa_Close );

    if( !pRsa->inUse ) { BERR_TRACE(BERR_NOT_INITIALIZED); return; }

    pRsa->inUse = false;

    BDBG_LEAVE( BHSM_Rsa_Close );
    return;
}


void  BHSM_Rsa_GetDefaultExponentiateSettings( BHSM_RsaHandle handle, BHSM_RsaExponentiateSettings *pSettings )
{

    BDBG_ENTER( BHSM_Rsa_GetDefaultExponentiateSettings );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pSettings );

    BDBG_LEAVE( BHSM_Rsa_GetDefaultExponentiateSettings );
    return;
}



BERR_Code BHSM_Rsa_Exponentiate( BHSM_RsaHandle handle, const BHSM_RsaExponentiateSettings *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_Rsa_Exponentiate );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pSettings );

    BDBG_LEAVE( BHSM_Rsa_Exponentiate );
    return rc;
}



void BHSM_Rsa_GetDefaultResult( BHSM_RsaExponentiateResult *pResult )
{
    BDBG_ENTER( BHSM_Rsa_GetDefaultResult );
    BSTD_UNUSED( pResult );

    BDBG_LEAVE( BHSM_Rsa_GetDefaultResult );
    return;
}



BERR_Code BHSM_Rsa_GetResult( BHSM_RsaHandle handle, BHSM_RsaExponentiateResult *pResult )
{
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_Rsa_GetResult );
    BSTD_UNUSED( handle );
    BSTD_UNUSED( pResult );

    BDBG_LEAVE( BHSM_Rsa_GetResult );
    return rc;
}
