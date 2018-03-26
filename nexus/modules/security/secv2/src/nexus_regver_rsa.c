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

#include "bstd.h"
#include "nexus_security_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "priv/nexus_core.h"
#include "nexus_memory.h"
#include "nexus_regver_rsa.h"
#include "priv/nexus_regver_rsa_priv.h"
#include "priv/nexus_security_priv.h"
#include "bhsm_rv_rsa.h"

BDBG_MODULE(nexus_otp_msp);


struct NEXUS_RegVerRsa
{

    BHSM_RvRsaHandle hHsmRvRsaHandle;
    uint8_t* pRsaKey;              /* Points to certificate of region. */
};


void NEXUS_RegVerRsa_GetDefaultAllocateSettings( NEXUS_RegVerRsaAllocateSettings *pSettings )
{
    if( !pSettings ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    return;
}


NEXUS_RegVerRsaHandle NEXUS_RegVerRsa_Allocate( const NEXUS_RegVerRsaAllocateSettings *pSettings )
{
    BHSM_RvRsaAllocateSettings hsmSettings;
    struct NEXUS_RegVerRsa *pInstance = NULL;
    NEXUS_MemoryAllocationSettings memSetting;
    BHSM_Handle hHsm;
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER( NEXUS_RegVerRsa_Allocate );

    if( !pSettings ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); goto error; }

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { BERR_TRACE( NEXUS_NOT_INITIALIZED ); return NULL; }

    pInstance = BKNI_Malloc( sizeof(*pInstance) );
    if( !pInstance ) { BERR_TRACE( NEXUS_OUT_OF_SYSTEM_MEMORY ); goto error; }

    BKNI_Memset( &hsmSettings, 0, sizeof(hsmSettings) );
    hsmSettings.rsaKeyId = pSettings->rsaKeyId;
    pInstance->hHsmRvRsaHandle = BHSM_RvRsa_Allocate( hHsm, &hsmSettings );
    if( !pInstance->hHsmRvRsaHandle ) { BERR_TRACE( NEXUS_NOT_AVAILABLE ); goto error; }

    /* allocate memory for the certificate. */
    NEXUS_Memory_GetDefaultAllocationSettings( &memSetting );
    memSetting.alignment = 32;
    memSetting.heap = g_pCoreHandles->heap[g_pCoreHandles->defaultHeapIndex].nexus;
    rc = NEXUS_Memory_Allocate( NEXUS_REGVER_RSA_KEY_SIZE, &memSetting, (void**)&pInstance->pRsaKey );
    if( rc != NEXUS_SUCCESS ) { BERR_TRACE( NEXUS_OUT_OF_DEVICE_MEMORY ); goto error; }

    BDBG_LEAVE( NEXUS_RegVerRsa_Allocate );
    return (NEXUS_RegVerRsaHandle)pInstance;

error:

    if( pInstance ) {
        if( pInstance->hHsmRvRsaHandle ) { BHSM_RvRsa_Free( pInstance->hHsmRvRsaHandle ); }
        if( pInstance->pRsaKey ) { NEXUS_Memory_Free( pInstance->pRsaKey ); }
        BKNI_Free( pInstance );
    }

    return NULL;
}


void NEXUS_RegVerRsa_Free( NEXUS_RegVerRsaHandle handle )
{
    struct NEXUS_RegVerRsa *pInstance = handle;

    BDBG_ENTER( NEXUS_RegVerRsa_Free );

    if( pInstance->hHsmRvRsaHandle ) { BHSM_RvRsa_Free( pInstance->hHsmRvRsaHandle ); }
    if( pInstance->pRsaKey ) { NEXUS_Memory_Free( pInstance->pRsaKey ); }
    BKNI_Free( pInstance );

    BDBG_LEAVE( NEXUS_RegVerRsa_Free );
    return;
}


NEXUS_Error NEXUS_RegVerRsa_SetSettings( NEXUS_RegVerRsaHandle handle, const NEXUS_RegVerRsaSettings *pSettings )
{
    BERR_Code rc = BERR_SUCCESS;
    struct NEXUS_RegVerRsa *pInstance = handle;
    BHSM_RvRsaSettings rvRsaSettings;

    BDBG_ENTER( NEXUS_RegVerRsa_SetSettings );

    BKNI_Memcpy( pInstance->pRsaKey, pSettings->rsaKey, NEXUS_REGVER_RSA_KEY_SIZE );
    NEXUS_FlushCache( (const void*)pInstance->pRsaKey, NEXUS_REGVER_RSA_KEY_SIZE );

    BKNI_Memset( &rvRsaSettings, 0, sizeof(rvRsaSettings) );
    rvRsaSettings.keyOffset = NEXUS_AddrToOffset( pInstance->pRsaKey );
    rvRsaSettings.multiTier = pSettings->multiTier;
    rvRsaSettings.multiTierSourceKeyId = pSettings->multiTierSourceKeyId;
    BDBG_CASSERT( (unsigned)NEXUS_SigningAuthority_eMax == (unsigned)BHSM_SigningAuthority_eMax );
    rvRsaSettings.rootKey = pSettings->rootKey;

    rc = BHSM_RvRsa_SetSettings( pInstance->hHsmRvRsaHandle, &rvRsaSettings );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    BDBG_LEAVE( NEXUS_RegVerRsa_SetSettings );

    return NEXUS_SUCCESS;
}


NEXUS_Error NEXUS_RegVerRsa_GetInfo( NEXUS_RegVerRsaHandle handle, NEXUS_RegVerRsaInfo *pRvRsaInfo )
{
    BERR_Code rc = BERR_SUCCESS;
    struct NEXUS_RegVerRsa *pInstance = handle;
    BHSM_RvRsaInfo rvRsaInfo;

    BDBG_ENTER( NEXUS_RegVerRsa_GetInfo );

    rc = BHSM_RvRsa_GetInfo( pInstance->hHsmRvRsaHandle, &rvRsaInfo );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    pRvRsaInfo->rsaKeyId = rvRsaInfo.rsaKeyId;

    BDBG_LEAVE( NEXUS_RegVerRsa_GetInfo );

    return NEXUS_SUCCESS;
}



NEXUS_Error NEXUS_RegVerRsa_GetInfo_priv( NEXUS_RegVerRsaHandle handle,
                                          NEXUS_RegVerRsaInfo_priv *pRvRsaInfo )
{
    struct NEXUS_RegVerRsa *pInstance = handle;

    BDBG_ENTER( NEXUS_RegVerRsa_GetInfo_priv );

    BKNI_Memset( pRvRsaInfo, 0, sizeof(*pRvRsaInfo) );

    pRvRsaInfo->rvRsaKeyHandle = pInstance->hHsmRvRsaHandle;

    BDBG_LEAVE( NEXUS_RegVerRsa_GetInfo_priv );

    return NEXUS_SUCCESS;
}
