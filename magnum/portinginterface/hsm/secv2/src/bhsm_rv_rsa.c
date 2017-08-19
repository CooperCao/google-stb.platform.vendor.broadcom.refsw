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
#include "bkni.h"
#include "bhsm.h"
#include "berr_ids.h"
#include "bhsm_common.h"
#include "bhsm_rv_rsa.h"
#include "bhsm_priv.h"
#include "bhsm_p_rv.h"

BDBG_MODULE(BHSM);


#define BHSM_MAX_RV_RSA_KEYS 4


typedef struct BHSM_P_RvRsa{

    BHSM_Handle hHsm;
    bool allocated;

    unsigned rsaKeyId;

}BHSM_P_RvRsa;

typedef struct BHSM_RvRsaModule
{
    BHSM_P_RvRsa rsaSlots[BHSM_MAX_RV_RSA_KEYS];

}BHSM_RvRsaModule;


static BHSM_Handle _GetHsmHandle( BHSM_RvRsaHandle handle );


BERR_Code BHSM_RvRsa_Init( BHSM_Handle hHsm, BHSM_RvRsaModuleSettings *pSettings )
{
    BDBG_ENTER( BHSM_RvRsa_Init );

    BSTD_UNUSED( pSettings );

    hHsm->modules.pRvRsa = (BHSM_RvRsaModule*)BKNI_Malloc( sizeof(BHSM_RvRsaModule) );
    if( !hHsm->modules.pRvRsa ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( hHsm->modules.pRvRsa, 0, sizeof(BHSM_RvRsaModule) );

    BDBG_LEAVE( BHSM_RvRsa_Init );
    return BERR_SUCCESS;
}

void BHSM_RvRsa_Uninit( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_RvRsa_Uninit );

    if( !hHsm->modules.pRvRsa ){
        BERR_TRACE( BERR_INVALID_PARAMETER );
        return;  /* function called out of sequence.  */
    }

    BKNI_Free( hHsm->modules.pRvRsa );
    hHsm->modules.pRvRsa = NULL;

    BDBG_LEAVE( BHSM_RvRsa_Uninit );
    return;
}


BHSM_RvRsaHandle BHSM_RvRsa_Allocate( BHSM_Handle hHsm, const BHSM_RvRsaAllocateSettings *pSettings )
{
    BHSM_P_RvRsa *pRvRsa = NULL;
    BHSM_RvRsaModule *pModule;
    unsigned i;
    unsigned rsaKeyId;

    BDBG_ENTER( BHSM_RvRsa_Allocate );

    pModule = hHsm->modules.pRvRsa;

    /* valid rsaKeyId are from 1 to BHSM_MAX_RV_RSA_KEYS */
    if( pSettings->rsaKeyId < 1 ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    if( (pSettings->rsaKeyId > BHSM_MAX_RV_RSA_KEYS) && (pSettings->rsaKeyId != BHSM_ANY_ID) )
    {
        BERR_TRACE( BERR_INVALID_PARAMETER );
        return NULL;
    }

    if( pSettings->rsaKeyId <= BHSM_MAX_RV_RSA_KEYS )
    {
        /* check if requested slot is available */
        if( pModule->rsaSlots[pSettings->rsaKeyId-1].allocated )
        {
            BERR_TRACE( BERR_NOT_AVAILABLE );
            goto error;
        }
        pRvRsa = &pModule->rsaSlots[pSettings->rsaKeyId-1];
        rsaKeyId = pSettings->rsaKeyId;
    }
    else
    {
        /* search for a free slot. */
        for( i = 0; i < BHSM_MAX_RV_RSA_KEYS; i++ )
        {
            if( pModule->rsaSlots[i].allocated == false )
            {
                pRvRsa = &pModule->rsaSlots[i];
                rsaKeyId = i+1; /*rsaId run from 1 to BHSM_MAX_RV_RSA_KEYS */
                break;
            }
        }

        if( !pRvRsa )
        {
            BERR_TRACE( BERR_NOT_AVAILABLE );
            goto error;
        }
    }

    BKNI_Memset( pRvRsa, 0, sizeof(*pRvRsa) );

    pRvRsa->allocated = true;
    pRvRsa->hHsm = hHsm;
    pRvRsa->rsaKeyId = rsaKeyId;

    BDBG_LEAVE( BHSM_RvRsa_Allocate );
    return (BHSM_RvRsaHandle)pRvRsa;

error:

    return NULL;
}


void BHSM_RvRsa_Free( BHSM_RvRsaHandle handle )
{
    BHSM_P_RvRsa *pRvRsa = (BHSM_P_RvRsa*)handle;
    BHSM_P_RvRsaKeyClear bspRsaClear;
    BERR_Code rc;

    BDBG_ENTER( BHSM_RvRsa_Free );

    if( pRvRsa->allocated )
    {
        BKNI_Memset( &bspRsaClear, 0, sizeof(bspRsaClear) );

        bspRsaClear.in.rsaKeyId = pRvRsa->rsaKeyId;

        rc = BHSM_P_Rv_RsaKeyClear( _GetHsmHandle(handle), &bspRsaClear );
        if( rc != BERR_SUCCESS ) { BERR_TRACE( rc ); } /* continue, best effort. */

        BKNI_Memset( pRvRsa, 0, sizeof(*pRvRsa) );
    }

    BDBG_LEAVE( BHSM_RvRsa_Free );

    return;
}


BERR_Code BHSM_RvRsa_SetSettings( BHSM_RvRsaHandle handle, const BHSM_RvRsaSettings *pSettings )
{
    BHSM_P_RvRsa *pRvRsa = (BHSM_P_RvRsa*)handle;
    BHSM_P_RvRsaKeySet bspRsaSet;
    BERR_Code rc;

    BDBG_ENTER( BHSM_RvRsa_SetSettings );

    BKNI_Memset( &bspRsaSet, 0, sizeof(bspRsaSet) );

    bspRsaSet.in.rsaKeyId = pRvRsa->rsaKeyId;

    if( pSettings->multiTier )
    {
        bspRsaSet.in.rootKeySrc = pSettings->multiTierSourceKeyId;
        bspRsaSet.in.multiTierKey = 1;
    }
    else
    {
        switch( pSettings->rootKey ) {
            case BHSM_RvRsaRootKey_e0Prime:{ bspRsaSet.in.rootKeySrc = 0; break;} /* Bsp_CmdRv_RsaRootKeyId_eKey0Prime */
            case BHSM_RvRsaRootKey_e0:     { bspRsaSet.in.rootKeySrc = 1; break;} /* Bsp_CmdRv_RsaRootKeyId_eKey0      */
            default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
        }
    }

    bspRsaSet.in.addressMsb = (pSettings->keyOffset >> 32);
    bspRsaSet.in.address    = (pSettings->keyOffset & 0xFFFFFFFF);

    rc = BHSM_P_Rv_RsaKeySet( _GetHsmHandle(handle), &bspRsaSet );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_RvRsa_SetSettings );

    return BERR_SUCCESS;
}


BERR_Code BHSM_RvRsa_GetInfo( BHSM_RvRsaHandle handle, BHSM_RvRsaInfo *pRvRsaInfo )
{
    BHSM_P_RvRsa *pRvRsa = (BHSM_P_RvRsa*)handle;

    BDBG_ENTER( BHSM_RvRsa_GetInfo );

    BKNI_Memset( pRvRsaInfo, 0, sizeof(*pRvRsaInfo) );

    pRvRsaInfo->rsaKeyId = pRvRsa->rsaKeyId;

    BDBG_LEAVE( BHSM_RvRsa_GetInfo );
    return BERR_SUCCESS;
}


static BHSM_Handle _GetHsmHandle( BHSM_RvRsaHandle handle )
{
    return handle->hHsm;
}
