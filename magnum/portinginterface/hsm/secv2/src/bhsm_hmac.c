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
#include "bhsm.h"
#include "bhsm_hmac.h"
#include "bhsm_p_crypto.h"
#include "bhsm_priv.h"
#include "bsp_types.h"

BDBG_MODULE( BHSM );

#define BHSM_P_HMAC_MAX_SIZE   (8*4)
#define BHSM_P_HMAC_STATE_SIZE (18*4)

static uint8_t _BspHashType( BHSM_HashType hashType );
static unsigned _HashLength( BHSM_HashType hashType );

typedef enum{
    BHSM_P_HmacState_eInitial,       /* the instance has been created.    */
    BHSM_P_HmacState_eReady,         /* the instance has been configured. */
    BHSM_P_HmacState_eInprogress,    /* waiting for further submits.  */

    BHSM_P_HmacState_Max
}BHSM_P_HmacState;

typedef struct
{
    BHSM_Handle hHsm;
    BHSM_HmacSettings settings;
    BHSM_P_HmacState state;

    uint8_t bspState[BHSM_P_HMAC_STATE_SIZE];

}BHSM_P_Hmac;


BHSM_HmacHandle BHSM_Hmac_Create( BHSM_Handle hHsm )
{
    BHSM_P_Hmac *pHandle = NULL;
    BDBG_ENTER( BHSM_Hmac_Create );

    pHandle = (BHSM_P_Hmac*)BKNI_Malloc( sizeof(BHSM_P_Hmac) );
    if( !pHandle ) { BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); return NULL; }

    pHandle->hHsm = hHsm;
    pHandle->state = BHSM_P_HmacState_eInitial;

    BDBG_LEAVE( BHSM_Hmac_Create );
    return (BHSM_HmacHandle)pHandle;
}



void BHSM_Hmac_Destroy( BHSM_HmacHandle handle )
{
    BHSM_P_Hmac *pInstance = (BHSM_P_Hmac*)handle;
    BDBG_ENTER( BHSM_Hmac_Destroy );

    if( !pInstance ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    BKNI_Memset( pInstance, 0, sizeof(*pInstance) );
    BKNI_Free( pInstance );

    BDBG_LEAVE( BHSM_Hmac_Destroy );
    return;
}


void BHSM_Hmac_GetDefaultSettings( BHSM_HmacSettings *pSettings )
{
    BDBG_ENTER( BHSM_Hmac_GetDefaultSettings );

    if( !pSettings ){ BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    BDBG_LEAVE( BHSM_Hmac_GetDefaultSettings );
    return;
}

BERR_Code BHSM_Hmac_SetSettings( BHSM_HmacHandle handle, const BHSM_HmacSettings *pSettings )
{
    BHSM_P_Hmac *pInstance = (BHSM_P_Hmac*)handle;

    BDBG_ENTER( BHSM_Hmac_SetSettings );

    if( !pSettings ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    pInstance->settings = *pSettings;
    pInstance->state = BHSM_P_HmacState_eReady;

    BDBG_LEAVE( BHSM_Hmac_SetSettings );
    return BERR_SUCCESS;
}


BERR_Code BHSM_Hmac_SubmitData( BHSM_HmacHandle handle, BHSM_HmacSubmitData *pData )
{
    BHSM_P_Hmac *pInstance = (BHSM_P_Hmac*)handle;
    BHSM_P_CryptoHmac bspConfig;
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER( BHSM_Hmac_SubmitData );

    if( pInstance->state == BHSM_P_HmacState_eInitial ) { return BERR_TRACE(BHSM_STATUS_STATE_ERROR); }
        /* ready and inprogress allowed.  */

    BDBG_MSG(("%s dataSize[%u] last[%x]", BSTD_FUNCTION, pData->dataSize, pData->last ));

    BKNI_Memset( &bspConfig, 0, sizeof(bspConfig) );
    bspConfig.in.dataAddrHi = (pData->dataOffset >> 32) & 0xFF;
    bspConfig.in.dataAddrLo = pData->dataOffset & 0xFFFFFFFF;
    bspConfig.in.dataLengthBytes = pData->dataSize;
    bspConfig.in.shaType = _BspHashType( pInstance->settings.hashType );

    switch( pInstance->settings.keySource )
    {
        case BHSM_HmacKeySource_eKeyLadder:
        {
            BHSM_KeyLadderInfo keyladderInfo;
            rc = BHSM_KeyLadder_GetInfo( pInstance->settings.key.keyladder.handle, &keyladderInfo );
            if( rc != BERR_SUCCESS ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }
            bspConfig.in.vklId = keyladderInfo.index;
            bspConfig.in.keyLayer = pInstance->settings.key.keyladder.level;
            bspConfig.in.keyType = Bsp_Crypto_HmacKeyType_eVkl;
            break;
        }
        case BHSM_HmacKeySource_eSoftwareKey:
        {
            unsigned keySize = 0;

            switch( pInstance->settings.hashType ) {
                case BHSM_HashType_e1_160: { keySize = 20; break; }
                case BHSM_HashType_e2_224: { keySize = 28; break; }
                case BHSM_HashType_e2_256: { keySize = 32; break; }
                default: { return BERR_TRACE(BERR_INVALID_PARAMETER); }
            }

            BDBG_CASSERT( sizeof(pInstance->settings.key.softKey) <= sizeof(bspConfig.in.userHmacKey) );
            if( keySize > sizeof(pInstance->settings.key.softKey) ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
            BHSM_MemcpySwap( bspConfig.in.userHmacKey, pInstance->settings.key.softKey, keySize );
            bspConfig.in.keyType = Bsp_Crypto_HmacKeyType_eUser;
            break;
        }
        case BHSM_HmacKeySource_eRpmb:
        {
            bspConfig.in.keyType = Bsp_Crypto_HmacKeyType_eRpmb;
            break;
        }
        default: { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    }

    bspConfig.in.isFirstDataBlock = (pInstance->state == BHSM_P_HmacState_eReady)?1:0;
    bspConfig.in.isFinalDataBlock = pData->last?1:0;

    if( pInstance->state == BHSM_P_HmacState_eInprogress ){
        BDBG_CASSERT( sizeof(pInstance->bspState) == sizeof(bspConfig.in.hmac_State) );
        BKNI_Memcpy( bspConfig.in.hmac_State, pInstance->bspState, sizeof(bspConfig.in.hmac_State) );
    }

    rc = BHSM_P_Crypto_Hmac( pInstance->hHsm, &bspConfig );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    if( pData->last ) {
        BDBG_CASSERT( sizeof(pData->hmac) == sizeof(bspConfig.out.hmac_Signature) );
        pData->hmacLength = _HashLength( pInstance->settings.hashType );
        BHSM_MemcpySwap( pData->hmac, bspConfig.out.hmac_Signature, pData->hmacLength );
        pInstance->state = BHSM_P_HmacState_eReady;
    }
    else {
        BDBG_CASSERT( sizeof(pInstance->bspState) == sizeof(bspConfig.out.hmac_State) );
        BKNI_Memcpy( pInstance->bspState, bspConfig.out.hmac_State, sizeof(pInstance->bspState) );
        pInstance->state = BHSM_P_HmacState_eInprogress;
    }

    BDBG_LEAVE( BHSM_Hmac_SubmitData );
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
