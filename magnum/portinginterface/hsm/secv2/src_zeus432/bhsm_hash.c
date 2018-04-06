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
#include "bhsm_priv.h"
#include "bhsm_bsp_msg.h"
#include "bsp_s_hmac_sha1.h"
#include "bsp_s_keycommon.h"
#include "bhsm_hash.h"
#include "bhsm_hash_priv.h"

BDBG_MODULE( BHSM );

#define BHSM_P_HASH_MAX_SIZE   (8*4)
#define BHSM_P_HASH_STATE_SIZE (18*4)
#define BHSM_P_HASH_MIN_KEYSIZE  (20)
#ifdef BHSM_BUILD_HSM_FOR_SAGE
  #define _HASH_CONTEXT_MAX     (1)                                     /* number of simultaneous HASH BFW contexts. */
  #define _HASH_CONTEXT_OFFSET  BPI_HmacSha1_Context_eHmacSha1CtxSCM    /* offset to context. */
#else
  #define _HASH_CONTEXT_MAX     (2)
  #define _HASH_CONTEXT_OFFSET  (BPI_HmacSha1_Context_eHmacSha1Ctx0)
#endif



BERR_Code BHSM_HashHmac_Init( BHSM_Handle hHsm, BHSM_HashHmacModuleSettings *pSettings )
{
    BDBG_ENTER( BHSM_HashHmac_Init );

    if( !hHsm ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    BSTD_UNUSED( pSettings );

    hHsm->modules.pHash = BKNI_Malloc( sizeof(BHSM_P_Hash)*_HASH_CONTEXT_MAX );
    if( !hHsm->modules.pHash ) { return BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); }

    BKNI_Memset( hHsm->modules.pHash, 0, sizeof(BHSM_P_Hash)*_HASH_CONTEXT_MAX );

    BDBG_LEAVE( BHSM_HashHmac_Init );
    return BERR_SUCCESS;
}

void BHSM_HashHmac_Uninit( BHSM_Handle hHsm )
{
    BDBG_ENTER( BHSM_HashHmac_Uninit );

    if( !hHsm ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }
    if( !hHsm->modules.pHash ) { BERR_TRACE( BERR_NOT_INITIALIZED ); return; }

    BKNI_Memset( hHsm->modules.pHash, 0,  sizeof(BHSM_P_Hash)*_HASH_CONTEXT_MAX );
    BKNI_Free( hHsm->modules.pHash );
    hHsm->modules.pHash = NULL;

    BDBG_LEAVE( BHSM_HashHmac_Uninit );
    return;
}

BHSM_HashHandle BHSM_Hash_Create( BHSM_Handle hHsm )
{
    unsigned i;
    BHSM_P_Hash* pInstances = NULL;

    BDBG_ENTER( BHSM_Hash_Create );

    if( !hHsm ) { BERR_TRACE( BERR_INVALID_PARAMETER ); return NULL; }
    if( !hHsm->modules.pHash ) { BERR_TRACE( BERR_NOT_INITIALIZED ); return NULL; }

    pInstances = hHsm->modules.pHash;

    for(i = 0; i < _HASH_CONTEXT_MAX; i++)
    {
        if(pInstances[i].inUse == false)
        {
            break;
        }
    }

    if( i == _HASH_CONTEXT_MAX ) { BERR_TRACE( BERR_OUT_OF_SYSTEM_MEMORY ); return NULL; }

    BKNI_Memset(&pInstances[i], 0, sizeof(BHSM_P_Hash));
    pInstances[i].inUse = true;
    pInstances[i].hHsm = hHsm;
    pInstances[i].state = BHSM_P_HashState_eInitial;
    pInstances[i].index = i;

    BDBG_LEAVE( BHSM_Hash_Create );
    return (BHSM_HashHandle)&pInstances[i];
}

void BHSM_Hash_Destroy( BHSM_HashHandle handle )
{
    BHSM_Hash_Destroy_priv( handle );
}

void BHSM_Hash_Destroy_priv( BHSM_HashHandle handle )
{
    BHSM_P_Hash *pInstance = handle;
    BDBG_ENTER( BHSM_Hash_Destroy );

    if( !handle ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }
    if( !pInstance ) { BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    if( pInstance->state == BHSM_P_HashState_eInprogress )
    {
        uint8_t digest[BHSM_P_HASH_MAX_SIZE] = {0};
        unsigned hashLength;
        BHSM_P_HmacHashSubmitInfo submitInfo;
        BERR_Code rc = BERR_UNKNOWN;

        BDBG_WRN(( "[%s]: Destroy out of sequence.", BSTD_FUNCTION ));

        BKNI_Memset( &submitInfo, 0, sizeof(submitInfo) );
        submitInfo.useInlineData = true;
        submitInfo.Data = 0x01;
        submitInfo.dataSize = 1;
        submitInfo.last = true;
        submitInfo.pHash = digest;
        submitInfo.pHashLength = &hashLength;

        rc = BHSM_Hash_SubmitData_priv( handle, &submitInfo );
        if( rc != BERR_SUCCESS ) {  BERR_TRACE(rc); /* continue, best effort. */ }
    }
    pInstance->inUse = false;
    BKNI_Memset( pInstance, 0, sizeof(*pInstance) );

    BDBG_LEAVE( BHSM_Hash_Destroy );
    return;
}

void BHSM_Hash_GetDefaultSettings( BHSM_HashSettings *pSettings )
{
    BDBG_ENTER( BHSM_Hash_GetDefaultSettings );

    if( !pSettings ) {  BERR_TRACE(BERR_INVALID_PARAMETER); return; }
    BKNI_Memset( pSettings, 0, sizeof(*pSettings) );

    BDBG_LEAVE( BHSM_Hash_GetDefaultSettings );
    return;
}


BERR_Code BHSM_Hash_SetSettings( BHSM_HashHandle handle,
                                 const BHSM_HashSettings *pSettings )
{
    return BHSM_Hash_SetSettings_priv( handle, pSettings, BHSM_P_HashOperation_eHash );
}

BERR_Code BHSM_Hash_SetSettings_priv( BHSM_HashHandle handle,
                                      const BHSM_HashSettings *pSettings,
                                      BHSM_P_HashOperation_e operation )
{
    BHSM_P_Hash *pInstance = handle;
    BERR_Code rc = BERR_UNKNOWN;

    BDBG_ENTER( BHSM_Hash_SetSettings );

    if( pInstance->state == BHSM_P_HashState_eInprogress )
    {
        /* Flush the BFW engine.*/
        uint8_t digest[BHSM_P_HASH_MAX_SIZE] = {0};
        unsigned hashLength;
        BHSM_P_HmacHashSubmitInfo submitInfo;

        BDBG_WRN(( "[%s]: Set Settings out of sequence.", BSTD_FUNCTION ));

        BKNI_Memset( &submitInfo, 0, sizeof(submitInfo) );
        submitInfo.useInlineData = true;
        submitInfo.Data = 0x01;
        submitInfo.dataSize = 1;
        submitInfo.last = true;
        submitInfo.pHash = digest;
        submitInfo.pHashLength = &hashLength;

        rc = BHSM_Hash_SubmitData_priv( handle, &submitInfo );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }
    }
    pInstance->settings = *pSettings;
    pInstance->operation = operation;
    pInstance->state = BHSM_P_HashState_eReady;

    BDBG_LEAVE( BHSM_Hash_SetSettings );
    return BERR_SUCCESS;
}

BERR_Code BHSM_Hash_SubmitData_priv( BHSM_HashHandle handle,
                                     BHSM_P_HmacHashSubmitInfo *pData )
{
    BHSM_P_Hash *pInstance = handle;
    BHSM_BspMsg_h hMsg = NULL;
    BERR_Code rc = BERR_UNKNOWN;
    unsigned _eAddressOrData = BCMD_HmacSha1_InCmdField_eAddressOrData;
    unsigned _eDataLength = BCMD_HmacSha1_InCmdField_eDataLen;
    uint8_t status = ~0;
    BHSM_BspMsgHeader_t header;
    unsigned keySize = BHSM_P_HASH_MIN_KEYSIZE;
    bool writekey = false;

    BDBG_ENTER( BHSM_Hash_SubmitData );

    if( pInstance->state == BHSM_P_HashState_eInitial ) { return BERR_TRACE(BHSM_STATUS_STATE_ERROR); }
    if( !pData ) { return BERR_TRACE(BERR_INVALID_PARAMETER); }

    *pData->pHashLength = 0;

    rc = BHSM_BspMsg_Create( pInstance->hHsm, &hMsg );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BHSM_BspMsg_GetDefaultHeader( &header );

    if( pData->last )
    {
        if( pInstance->state == BHSM_P_HashState_eInprogress )
        {
            header.continualMode = BHSM_HMACSHA_ContinualMode_eLastSeg;
        }
        else
        {
            header.continualMode = BHSM_HMACSHA_ContinualMode_eAllSeg;
        }
    }
    else
    {
        header.continualMode = BHSM_HMACSHA_ContinualMode_eMoreSeg;
    }
    BHSM_BspMsg_Header( hMsg, BCMD_cmdType_eUSER_SHA1, &header );

    if( pInstance->operation == BHSM_P_HashOperation_eHmacRpmb )
    {
        BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eVKL, BCMD_VKLID_eRPMBHmacKey );
    }
    else
    {
        if( pInstance->settings.appendKey )
        {
            if( pInstance->settings.key.keyladder.handle == NULL )
            {
                unsigned shaKeysize = 0;
                keySize = pInstance->settings.key.softKeySize  / 8;

                switch( pInstance->settings.hashType )
                {
                    case BHSM_HashType_e1_160: shaKeysize = 20; break;
                    case BHSM_HashType_e2_224: shaKeysize = 28; break;
                    case BHSM_HashType_e2_256: shaKeysize = 32; break;
                    default: { rc = BERR_TRACE( BERR_INVALID_PARAMETER ); goto BHSM_P_DONE_LABEL; }
                }

                if(keySize > shaKeysize) { rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto BHSM_P_DONE_LABEL;  }

                _eDataLength = BCMD_HmacSha1_InCmdField_eKeyData + keySize;
                _eAddressOrData = _eDataLength + 4;
                writekey = true;
            }
            else
            {
                BHSM_KeyLadderInfo info;

                rc = BHSM_KeyLadder_GetInfo( pInstance->settings.key.keyladder.handle,  &info );
                if( rc != BERR_SUCCESS ) { rc = BERR_TRACE(rc); goto BHSM_P_DONE_LABEL; }

                BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eVKL, info.index );
                BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eKeyLayer, pInstance->settings.key.keyladder.level );
            }
        }
    }

    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eOperation,  (pInstance->operation == BHSM_P_HashOperation_eHash) ?
                                                                           BPI_HmacSha1_Op_eSha1 : BPI_HmacSha1_Op_eHmac );
    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eIncludeKey, (pInstance->settings.appendKey & (pInstance->operation == BHSM_P_HashOperation_eHash)) ? 1 : 0 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eIsAddress,  (pData->useInlineData) ? 0 : 1 );
    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eShaType,    pInstance->settings.hashType );
    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eContextId,  (_HASH_CONTEXT_OFFSET + pInstance->index) );

    BHSM_BspMsg_Pack8( hMsg, BCMD_HmacSha1_InCmdField_eKeyLen, keySize );

    if( writekey )
    {
        BHSM_BspMsg_PackArray( hMsg, BCMD_HmacSha1_InCmdField_eKeyData, pInstance->settings.key.softKey, keySize );
    }

    BHSM_BspMsg_Pack32( hMsg, _eDataLength, pData->dataSize );
    if( pData->useInlineData )
    {
        BHSM_BspMsg_Pack8( hMsg, _eAddressOrData, pData->Data );
    }
    else
    {
        BHSM_BspMsg_Pack32( hMsg, _eAddressOrData, pData->dataOffset );
    }

    rc = BHSM_BspMsg_SubmitCommand( hMsg );
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE(rc); goto BHSM_P_DONE_LABEL; }

    BHSM_BspMsg_Get8( hMsg, BCMD_CommonBufferFields_eStatus, &status );
    if(    ( status != 0 )
        && ( status != 0xA8 )    /* more data expected. */ )
    {
        BDBG_ERR(( "[%s] failed status[0x%02X]", BSTD_FUNCTION, status ));
        rc = BHSM_STATUS_FAILED;
        goto BHSM_P_DONE_LABEL;
    }

    if( pData->last ) {
        uint16_t            digestSize = 0;
        BHSM_BspMsg_Get16( hMsg, BCMD_CommonBufferFields_eParamLen, &digestSize );
        digestSize -= 4; /* remove status paramter bytes from length */

        if( digestSize > BHSM_HASH_MAX_LENGTH ) { rc = BERR_TRACE( BERR_INVALID_PARAMETER ); goto BHSM_P_DONE_LABEL; }

        BHSM_BspMsg_GetArray( hMsg, BCMD_HmacSha1_OutCmdField_eDigest, pData->pHash, digestSize );

        *pData->pHashLength = digestSize;
        pInstance->state = BHSM_P_HashState_eReady;
    }
    else {
        pInstance->state = BHSM_P_HashState_eInprogress;
    }

BHSM_P_DONE_LABEL:

    (void)BHSM_BspMsg_Destroy( hMsg );

    BDBG_LEAVE( BHSM_Hash_SubmitData );
    return rc;
}

BERR_Code BHSM_Hash_SubmitData_1char( BHSM_HashHandle handle,
                                      BHSM_HashSubmitData_1char *pData )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_P_HmacHashSubmitInfo submitInfo;

    BDBG_ENTER( BHSM_Hash_SubmitData );

    BKNI_Memset( &submitInfo, 0, sizeof(submitInfo) );
    submitInfo.useInlineData = true;
    submitInfo.Data = pData->data;
    submitInfo.dataSize = 1;
    submitInfo.last = true;
    submitInfo.pHash = pData->hash;
    submitInfo.pHashLength = &pData->hashLength;

    rc = BHSM_Hash_SubmitData_priv( handle, &submitInfo );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    BDBG_LEAVE( BHSM_Hash_SubmitData );
    return BERR_SUCCESS;
}


BERR_Code BHSM_Hash_SubmitData( BHSM_HashHandle handle,
                                BHSM_HashSubmitData  *pData )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_P_HmacHashSubmitInfo submitInfo;

    BDBG_ENTER( BHSM_Hash_SubmitData );

    BKNI_Memset( &submitInfo, 0, sizeof(submitInfo) );
    submitInfo.useInlineData = false;
    submitInfo.dataOffset = pData->dataOffset;
    submitInfo.dataSize = pData->dataSize;
    submitInfo.last = pData->last;
    submitInfo.pHash = pData->hash;
    submitInfo.pHashLength = &pData->hashLength;

    rc = BHSM_Hash_SubmitData_priv( handle, &submitInfo );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE(rc); }

    BDBG_LEAVE( BHSM_Hash_SubmitData );
    return BERR_SUCCESS;
}
