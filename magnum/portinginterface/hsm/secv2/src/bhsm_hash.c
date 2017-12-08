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
#include "bhsm_priv.h"
#include "bhsm_hash.h"
#include "bsp_types.h"
#include "bhsm_p_crypto.h"

BDBG_MODULE( BHSM );

#define BHSM_P_HASH_MAX_SIZE   (8*4)
#define BHSM_P_HASH_STATE_SIZE (18*4)

static uint8_t _BspHashType( BHSM_HashType hashType );
static uint8_t _BspKeySize( unsigned keySize /*in bits*/ );
static unsigned _HashLength( BHSM_HashType hashType );

typedef enum{
    BHSM_P_HashState_eInitial,          /* created.     */
    BHSM_P_HashState_eReady,            /* configured.  */
    BHSM_P_HashState_eInprogress,       /* waiting for more data. */
    BHSM_P_HashState_Max
}BHSM_P_HashState;

typedef struct
{
    BHSM_Handle hHsm;
    BHSM_HashSettings settings;
    BHSM_P_HashState state;

    uint8_t bspState[BHSM_P_HASH_STATE_SIZE];

}BHSM_P_Hash;


BHSM_HashHandle BHSM_Hash_Create( BHSM_Handle hHsm )
{
    BHSM_P_Hash *pHandle = NULL;
    BDBG_ENTER( BHSM_Hash_Create );

    pHandle = (BHSM_P_Hash*)BKNI_Malloc( sizeof(BHSM_P_Hash) );
    if( !pHandle ) { BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); return NULL; }

    pHandle->hHsm = hHsm;
    pHandle->state = BHSM_P_HashState_eInitial;

    BDBG_LEAVE( BHSM_Hash_Create );
    return (BHSM_HashHandle)pHandle;
}



void BHSM_Hash_Destroy( BHSM_HashHandle handle )
{
    BHSM_P_Hash *pInstance = (BHSM_P_Hash*)handle;
    BDBG_ENTER( BHSM_Hash_Destroy );

    if( !pInstance ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    BKNI_Memset( pInstance, 0, sizeof(*pInstance) );
    BKNI_Free( pInstance );

    BDBG_LEAVE( BHSM_Hash_Destroy );
    return;
}


void BHSM_Hash_GetDefaultSettings( BHSM_HashSettings *pSettings )
{
    BDBG_ENTER( BHSM_Hash_GetDefaultSettings );

    if( !pSettings ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    BDBG_LEAVE( BHSM_Hash_GetDefaultSettings );
    return;
}


BERR_Code BHSM_Hash_SetSettings( BHSM_HashHandle handle, const BHSM_HashSettings *pSettings )
{
    BHSM_P_Hash *pInstance = (BHSM_P_Hash*)handle;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER( BHSM_Hash_SetSettings );

    if( !pSettings ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    pInstance->settings = *pSettings;
    pInstance->state = BHSM_P_HashState_eReady;

    BDBG_LEAVE( BHSM_Hash_SetSettings );
    return rc;
}

BERR_Code BHSM_Hash_SubmitData( BHSM_HashHandle handle, BHSM_HashSubmitData  *pData )
{
    BHSM_P_Hash *pInstance = (BHSM_P_Hash*)handle;
    BHSM_P_CryptoSha bspConfig;
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER( BHSM_Hash_SubmitData );

    if( pInstance->state == BHSM_P_HashState_eInitial ) { return BERR_TRACE(BHSM_STATUS_STATE_ERROR); }
    if( !pData ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    BDBG_MSG(("%s dataSize[%u] last[%x]", BSTD_FUNCTION, pData->dataSize, pData->last ));

    pData->hashLength = 0;

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );
    bspConfig.in.dataAddrHi = (pData->dataOffset >> 32) & 0xFF;
    bspConfig.in.dataAddrLo = pData->dataOffset & 0xFFFFFFFF;
    bspConfig.in.dataLengthBytes = pData->dataSize;
    bspConfig.in.shaType = _BspHashType( pInstance->settings.hashType );

    if( pInstance->settings.appendKey ) {
        unsigned keySize = 0;

        if( pInstance->settings.key.keyladder.handle ) { return BERR_TRACE(BERR_INVALID_PARAMETER); } /*not in zues5*/
        if( pInstance->settings.key.softKeySize % 8 ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
        keySize = pInstance->settings.key.softKeySize / 8;
        if( keySize > sizeof(pInstance->settings.key.softKey) ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
        if( keySize > sizeof(bspConfig.in.userKey) ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

        switch( pInstance->settings.key.softKeySize ) { /* check that the keysize is supported. */
            case 128: break;
            case 192: break;
            case 256: break;
            default: { return BERR_TRACE(BERR_INVALID_PARAMETER); }
        }

        BHSM_MemcpySwap( bspConfig.in.userKey, pInstance->settings.key.softKey, keySize );
        bspConfig.in.userKeySize = _BspKeySize( pInstance->settings.key.softKeySize );
        bspConfig.in.isKeyAppend = 1;
    }

    bspConfig.in.isFirstDataBlock = (pInstance->state == BHSM_P_HashState_eReady)?1:0;
    bspConfig.in.isFinalDataBlock = pData->last?1:0;

    if( pInstance->state == BHSM_P_HashState_eInprogress ){
        BDBG_CASSERT( sizeof(pInstance->bspState) == sizeof(bspConfig.in.sha_State) );
        BKNI_Memcpy( bspConfig.in.sha_State, pInstance->bspState, sizeof(bspConfig.in.sha_State) );
    }

    BDBG_MSG(("%s state[%x] last[%x] appendKey[%x] dataSize[%u]"
                                    , BSTD_FUNCTION, pInstance->state, pData->last
                                    , pInstance->settings.appendKey, pData->dataSize ));

    rc = BHSM_P_Crypto_Sha( pInstance->hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    if( pData->last ) {
        BDBG_CASSERT( sizeof(pData->hash) == sizeof(bspConfig.out.sha_Digest) );
        BHSM_MemcpySwap( (void*)pData->hash, (void*)bspConfig.out.sha_Digest, sizeof(pData->hash) );
        pData->hashLength = _HashLength( pInstance->settings.hashType );
        pInstance->state = BHSM_P_HashState_eReady; /* Finished. revert to configured state. */
    }
    else {
        BDBG_CASSERT( sizeof(pInstance->bspState) == sizeof(bspConfig.out.sha_State) );
        BKNI_Memcpy( pInstance->bspState, bspConfig.out.sha_State, sizeof(pInstance->bspState) );
        pInstance->state = BHSM_P_HashState_eInprogress;
    }

    BDBG_LEAVE( BHSM_Hash_SubmitData );
    return rc;
}


static uint8_t _BspHashType( BHSM_HashType hashType )
{
    switch( hashType ) {
        case BHSM_HashType_e1_160: { return 0;}
        case BHSM_HashType_e2_224: { return 1;}
        case BHSM_HashType_e2_256: { return 2;}
        default: { BERR_TRACE(BERR_INVALID_PARAMETER); }
    }

    return (uint8_t)hashType;
}

static uint8_t _BspKeySize( unsigned keySize /*in bits*/ )
{
    switch( keySize )
    {
        case  64: return Bsp_KeySize_e64;
        case 128: return Bsp_KeySize_e128;
        case 192: return Bsp_KeySize_e192;
        case 256: return Bsp_KeySize_e256;
        default: { BDBG_ERR(("%s size[%u]", BSTD_FUNCTION, keySize )); BERR_TRACE(BERR_INVALID_PARAMETER); }
    }
    return Bsp_KeySize_eMax;
}

/* return the length of a hash in bytes   */
static unsigned _HashLength( BHSM_HashType hashType )
{
    switch( hashType ) {
        case BHSM_HashType_e1_160: { return (160/8);}
        case BHSM_HashType_e2_224: { return (224/8);}
        case BHSM_HashType_e2_256: { return (256/8);}
        default: { BERR_TRACE(BERR_INVALID_PARAMETER); }
    }

    return (0); /* default size! */
}
