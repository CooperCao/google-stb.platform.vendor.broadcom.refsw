/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "nexus_security_module.h"
#include "nexus_base.h"
#include "nexus_types.h"
#include "nexus_security_common.h"
#include "nexus_hash.h"
#include "bhsm_hash.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_keyladder_priv.h"
#include "priv/nexus_hash_priv.h"
#include "nexus_keyladder.h"


BDBG_MODULE(nexus_hash);

struct NEXUS_Hash
{
    NEXUS_OBJECT(NEXUS_Hash);

    BHSM_HashHandle hHsmHash;
    NEXUS_HashSettings settings;

    NEXUS_HashHmacQueue *queue;

    bool someData;
};

/*
static struct {
    unsigned dummy;
} g_securityHash;
*/

static BHSM_HashType _MapHashType( NEXUS_HashType type );
static BERR_Code _ProcessNullSha( NEXUS_HashResult *pHashResult,  NEXUS_HashType type );

NEXUS_OBJECT_CLASS_MAKE( NEXUS_Hash, NEXUS_Hash_Destroy );

NEXUS_HashHandle NEXUS_Hash_Create( void )
{
    BHSM_Handle hHsm;
    NEXUS_HashHandle handle = NULL;

    BDBG_ENTER( NEXUS_Hash_Create );

    handle = BKNI_Malloc( sizeof(struct NEXUS_Hash) );
    if( !handle ) { BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto _error; }

    NEXUS_OBJECT_INIT( NEXUS_Hash, handle );

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( !hHsm ) { BERR_TRACE( NEXUS_NOT_INITIALIZED ); goto _error; }

    handle->hHsmHash = BHSM_Hash_Create( hHsm );
    if( !handle->hHsmHash ) { BERR_TRACE(NEXUS_NOT_AVAILABLE); goto _error; }

    handle->queue = NEXUS_HashHmac_QueueCreate();
    if( !handle->queue) { BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY); goto _error; }

    BDBG_LEAVE( NEXUS_Hash_Create );
    return handle;

_error:
    if( handle ) { NEXUS_Hash_P_Finalizer( handle ); }

    return NULL;
}


static void NEXUS_Hash_P_Finalizer( NEXUS_HashHandle handle )
{
    BDBG_ENTER( NEXUS_Hash_P_Finalizer );

    if( handle ){

        if( handle->queue ) { NEXUS_HashHmac_QueueDestroy( handle->queue ); }

        if( handle->hHsmHash ) { BHSM_Hash_Destroy ( handle->hHsmHash ); }

        NEXUS_OBJECT_DESTROY( NEXUS_Hash, handle );

        BKNI_Free( handle );
    }

    BDBG_LEAVE( NEXUS_Hash_P_Finalizer );
    return;
}

void NEXUS_Hash_GetDefaultSettings( NEXUS_HashSettings *pSettings )
{
    BDBG_ENTER( NEXUS_Hash_GetDefaultSettings );

    if( !pSettings ) {  BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    BDBG_LEAVE( NEXUS_Hash_GetDefaultSettings );
    return;
}


NEXUS_Error NEXUS_Hash_SetSettings( NEXUS_HashHandle handle, const NEXUS_HashSettings *pSettings )
{
    BHSM_HashSettings hsmSettings;
    BERR_Code hsmRc;

    BDBG_ENTER( NEXUS_Hash_SetSettings );
    NEXUS_OBJECT_ASSERT( NEXUS_Hash, handle );

    BKNI_Memset( &hsmSettings, 0, sizeof(hsmSettings) );
    hsmSettings.hashType = _MapHashType( pSettings->hashType );
    hsmSettings.appendKey = pSettings->appendKey;

    if( pSettings->appendKey ) {

        if( pSettings->key.keyladder.handle ) {
            NEXUS_KeyLadderInfo_priv keyladderInfo;

            NEXUS_KeyLadder_GetInfo_priv( pSettings->key.keyladder.handle, &keyladderInfo );

            hsmSettings.key.keyladder.handle = keyladderInfo.hsmKeyLadderHandle;
            hsmSettings.key.keyladder.level = pSettings->key.keyladder.level;
        }
        else{
            if( pSettings->key.softKeySize/8 > BHSM_HASH_MAX_LENGTH ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }

            BKNI_Memcpy( hsmSettings.key.softKey, pSettings->key.softKey, pSettings->key.softKeySize/8 );
            hsmSettings.key.softKeySize = pSettings->key.softKeySize;
        }
    }

    NEXUS_HashHmac_ResetBuffer(handle->queue);
    hsmRc = BHSM_Hash_SetSettings( handle->hHsmHash, &hsmSettings );
    if( hsmRc != BERR_SUCCESS ) { return BERR_TRACE( hsmRc ); }

    handle->settings = *pSettings;

    BDBG_LEAVE( NEXUS_Hash_SetSettings );
    return NEXUS_SUCCESS;
}



void NEXUS_Hash_GetDefaultData( NEXUS_HashData *pData )
{
    BDBG_ENTER( NEXUS_Hash_GetDefaultData );

    if( !pData ) {  BERR_TRACE(NEXUS_INVALID_PARAMETER); return; }

    BKNI_Memset( pData, 0, sizeof(*pData) );

    BDBG_LEAVE( NEXUS_Hash_GetDefaultData );
    return;
}

static BERR_Code _ProcessNullSha(NEXUS_HashResult *pHashResult,  NEXUS_HashType type)
{
    const uint8_t *pNullSha = 0;

    /* unfortunately the BSP has a bug with null string SHA so I need to hardcode */
    static const uint8_t gNullSha1[] =
    {
        0xDA,0x39,0xA3,0xEE,0x5E,0x6B,0x4B,0x0D,
        0x32,0x55,0xBF,0xEF,0x95,0x60,0x18,0x90,
        0xAF,0xD8,0x07,0x09
    };

    static const uint8_t gNullSha224[] =
    {
        0xd1,0x4a,0x02,0x8c,0x2a,0x3a,0x2b,0xc9,
        0x47,0x61,0x02,0xbb,0x28,0x82,0x34,0xc4,
        0x15,0xa2,0xb0,0x1f,0x82,0x8e,0xa6,0x2a,
        0xc5,0xb3,0xe4,0x2f
    };

    static const uint8_t gNullSha256[] =
    {
        0xe3,0xb0,0xc4,0x42,0x98,0xfc,0x1c,0x14,
        0x9a,0xfb,0xf4,0xc8,0x99,0x6f,0xb9,0x24,
        0x27,0xae,0x41,0xe4,0x64,0x9b,0x93,0x4c,
        0xa4,0x95,0x99,0x1b,0x78,0x52,0xb8,0x55
    };

    if( !pHashResult ) { return BERR_TRACE(BERR_INVALID_PARAMETER ); }

    switch (type)
    {
        case NEXUS_HashType_e1_160:
            pNullSha = gNullSha1;
            pHashResult->hashLength = sizeof(gNullSha1);
            break;
        case NEXUS_HashType_e2_224:
            pNullSha = gNullSha224;
            pHashResult->hashLength = sizeof(gNullSha224);
            break;
        case NEXUS_HashType_e2_256:
            pNullSha = gNullSha256;
            pHashResult->hashLength = sizeof(gNullSha256);
            break;
        default:
            return BERR_TRACE( BERR_INVALID_PARAMETER );
    }

    if( pHashResult->hashLength > sizeof(pHashResult->hash)) { return BERR_TRACE(BERR_INVALID_PARAMETER ); }

    BKNI_Memcpy(pHashResult->hash, pNullSha,  pHashResult->hashLength);
    return BERR_SUCCESS;
}


NEXUS_Error NEXUS_Hash_SubmitData( NEXUS_HashHandle handle, const NEXUS_HashData *pData, NEXUS_HashResult *pResult )
{
    BERR_Code hsmRc = BERR_UNKNOWN;
    NEXUS_HashHmac_QueueUp queueUp;

    BDBG_ENTER( NEXUS_Hash_SubmitData );
    NEXUS_OBJECT_ASSERT( NEXUS_Hash, handle );

    if( !pData ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( pData->last && !pResult ) { return BERR_TRACE(NEXUS_INVALID_PARAMETER); }
    if( pResult ) { pResult->hashLength = 0; }
    if( pData->dataOffset == 0 && pData->dataSize ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
    if( pData->dataOffset && pData->dataSize == 0 ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    if(pData->dataSize)
    {
        handle->someData = true;
    }
    else if(pData->last == 0)
    {
        return NEXUS_SUCCESS;
    }

    if(pData->last == 1 && handle->someData == false)
    {
        hsmRc = _ProcessNullSha(pResult, handle->settings.hashType);
        if( hsmRc != BERR_SUCCESS ) { return BERR_TRACE( hsmRc ); }
    }
    else
    {
        NEXUS_Error rc = NEXUS_SUCCESS;
        queueUp.hHsmHandle = (void*)handle->hHsmHash;
        queueUp.pQueue = handle->queue;
        queueUp.hashNotHmac = true;
        queueUp.dataOffset = pData->dataOffset;
        queueUp.dataSize = pData->dataSize;
        queueUp.last = pData->last;

        rc = NEXUS_HashHmac_QueueUpData(&queueUp);
        if(rc != NEXUS_SUCCESS) { return BERR_TRACE( NEXUS_UNKNOWN ); }

        if( pData->last ) {
            if( queueUp.digestLength > sizeof(pResult->hash) ) return BERR_TRACE( NEXUS_UNKNOWN );

            BKNI_Memcpy( pResult->hash, queueUp.digest, queueUp.digestLength );
            pResult->hashLength = queueUp.digestLength;
            handle->someData = false;
        }
    }
    BDBG_LEAVE( NEXUS_Hash_SubmitData );
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
