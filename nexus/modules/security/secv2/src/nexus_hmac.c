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
#include "nexus_security_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_security_common.h"
#include "nexus_hmac.h"
#include "bhsm_hmac.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_keyladder_priv.h"
#include "priv/nexus_hash_priv.h"
#include "nexus_keyladder.h"


BDBG_MODULE(nexus_hmac);

struct NEXUS_Hmac
{
    NEXUS_OBJECT(NEXUS_Hmac);

    BHSM_HmacHandle hHsmHmac;
    NEXUS_HmacSettings settings;

    NEXUS_HashHmacQueue *queue;
};

static BHSM_HashType _MapHashType( NEXUS_HashType type );

NEXUS_OBJECT_CLASS_MAKE( NEXUS_Hmac, NEXUS_Hmac_Destroy );

NEXUS_HmacHandle NEXUS_Hmac_Create( void )
{
    BHSM_Handle hHsm;
    NEXUS_HmacHandle handle = NULL;

    BDBG_ENTER( NEXUS_Hmac_Create );

    handle = BKNI_Malloc( sizeof(struct NEXUS_Hmac) );
    if( !handle ) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto _error; }

    NEXUS_OBJECT_INIT( NEXUS_Hmac, handle );

    NEXUS_Security_GetHsm_priv( &hHsm );

    handle->hHsmHmac = BHSM_Hmac_Create( hHsm );
    if( !handle->hHsmHmac ) { BERR_TRACE(NEXUS_NOT_AVAILABLE); goto _error; }

    handle->queue = NEXUS_HashHmac_QueueCreate();
    if( handle->queue == NULL) { BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto _error; }

    BDBG_LEAVE( NEXUS_Hmac_Create );
    return handle;

_error:
    if( handle ) { NEXUS_Hmac_P_Finalizer( handle ); }

    return NULL;
}


static void NEXUS_Hmac_P_Finalizer( NEXUS_HmacHandle handle )
{
    BDBG_ENTER( NEXUS_Hmac_P_Finalizer );

    if( handle ){

        NEXUS_HashHmac_QueueDestroy(handle->queue);
        if( handle->hHsmHmac ) { BHSM_Hmac_Destroy ( handle->hHsmHmac ); }

        NEXUS_OBJECT_DESTROY( NEXUS_Hmac, handle );
        BKNI_Free( handle );
    }

    BDBG_LEAVE( NEXUS_Hmac_P_Finalizer );
    return;
}

void NEXUS_Hmac_GetDefaultSettings( NEXUS_HmacSettings *pSettings )
{
    BDBG_ENTER( NEXUS_Hmac_GetDefaultSettings );

    if( !pSettings ) {  BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    BDBG_LEAVE( NEXUS_Hmac_GetDefaultSettings );
    return;
}


NEXUS_Error NEXUS_Hmac_SetSettings( NEXUS_HmacHandle handle, const NEXUS_HmacSettings *pSettings )
{
    BHSM_HmacSettings hsmSettings;
    BERR_Code hsmRc;

    BDBG_ENTER( NEXUS_Hmac_SetSettings );
    NEXUS_OBJECT_ASSERT( NEXUS_Hmac, handle );

    BKNI_Memset( &hsmSettings, 0, sizeof(hsmSettings) );
    hsmSettings.hashType = _MapHashType( pSettings->hashType );

    switch( pSettings->key.source )
    {
        case NEXUS_HmacKeySource_eKeyLadder:
        {
            NEXUS_KeyLadderInfo_priv keyladderInfo;
            NEXUS_KeyLadder_GetInfo_priv( pSettings->key.keyladder.handle, &keyladderInfo );
            hsmSettings.key.keyladder.handle = keyladderInfo.hsmKeyLadderHandle;
            hsmSettings.key.keyladder.level = pSettings->key.keyladder.level;
            hsmSettings.keySource = BHSM_HmacKeySource_eKeyLadder;
            break;
        }
        case NEXUS_HmacKeySource_eSoftwareKey:
        {
            BDBG_CASSERT( sizeof(hsmSettings.key.softKey) == sizeof(pSettings->key.softKey) );
            BKNI_Memcpy( hsmSettings.key.softKey, pSettings->key.softKey, sizeof(hsmSettings.key.softKey) );
            hsmSettings.keySource = BHSM_HmacKeySource_eSoftwareKey;
            break;
        }
        default:
        {
             return BERR_TRACE( NEXUS_INVALID_PARAMETER );
        }
    }

    NEXUS_HashHmac_ResetBuffer(handle->queue);
    hsmRc = BHSM_Hmac_SetSettings( handle->hHsmHmac, &hsmSettings );
    if( hsmRc != BERR_SUCCESS ) { return BERR_TRACE( hsmRc ); }

    handle->settings = *pSettings;

    BDBG_LEAVE( NEXUS_Hmac_SetSettings );
    return NEXUS_SUCCESS;
}



void NEXUS_Hmac_GetDefaultData( NEXUS_HmacData *pData )
{
    BDBG_ENTER( NEXUS_Hmac_GetDefaultData );

    if( !pData ) {  BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    BKNI_Memset( pData, 0, sizeof(*pData) );

    BDBG_LEAVE( NEXUS_Hmac_GetDefaultData );
    return;
}
NEXUS_Error NEXUS_Hmac_SubmitData( NEXUS_HmacHandle handle, const NEXUS_HmacData *pData, NEXUS_HmacResult *pResult )
{
    NEXUS_HashHmac_QueueUp queueUp;
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_ENTER( NEXUS_Hmac_SubmitData );
    NEXUS_OBJECT_ASSERT( NEXUS_Hmac, handle );

    if( !pData ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( pData->last && !pResult ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( pResult ) { pResult->hmacLength = 0; }
    if( pData->dataOffset == 0 && pData->dataSize ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
    if( pData->dataOffset && pData->dataSize == 0 ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    if(pData->dataSize == 0 && pData->last == 0)
    {
        return NEXUS_SUCCESS;
    }

    queueUp.hHsmHandle = (void*)handle->hHsmHmac;
    queueUp.pQueue = handle->queue;
    queueUp.hashNotHmac = false;
    queueUp.dataOffset = pData->dataOffset;
    queueUp.dataSize = pData->dataSize;
    queueUp.last = pData->last;

    rc = NEXUS_HashHmac_QueueUpData(&queueUp);
    if(rc != NEXUS_SUCCESS) { return BERR_TRACE( NEXUS_UNKNOWN ); }

    if( pData->last ) {
        if( queueUp.digestLength > sizeof(pResult->hmac) ) return BERR_TRACE( NEXUS_UNKNOWN );

        BKNI_Memcpy( pResult->hmac, queueUp.digest, queueUp.digestLength );
        pResult->hmacLength = queueUp.digestLength;
    }

    BDBG_LEAVE( NEXUS_Hmac_SubmitData );
    return NEXUS_SUCCESS;
}


static BHSM_HashType _MapHashType( NEXUS_HashType type )
{
    switch( type ) {
        case NEXUS_HashType_e1_160: return BHSM_HashType_e1_160;
        case NEXUS_HashType_e2_224: return BHSM_HashType_e2_224;
        case NEXUS_HashType_e2_256: return BHSM_HashType_e2_256;
        default: BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }
    return (BHSM_HashType)type;
}
