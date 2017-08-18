/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "bhsm.h"
#include "bhsm_priv.h"
#include "bhsm_bsp_msg.h"
#include "bsp_types.h"
#include "bhsm_crypto_sage.h"
#include "bhsm_p_crypto.h"

BDBG_MODULE( BHSM );


BERR_Code _crypt( BHSM_Handle hHsm,
                  BHSM_CryptographicOperation operation,
                  const BHSM_Crypto *pParam );


BERR_Code BHSM_Crypto_Encrypt( BHSM_Handle hHsm, const BHSM_Crypto *pParam )
{
    BERR_Code rc;
    BDBG_ENTER( BHSM_Crypto_Encrypt );

    rc = _crypt( hHsm, BHSM_CryptographicOperation_eEncrypt, pParam );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_Crypto_Encrypt );
    return rc;
}

BERR_Code BHSM_Crypto_Decrypt( BHSM_Handle hHsm, const BHSM_Crypto *pParam )
{
    BERR_Code rc;

    BDBG_ENTER( BHSM_Crypto_Decrypt );

    rc = _crypt( hHsm, BHSM_CryptographicOperation_eDecrypt, pParam );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    BDBG_LEAVE( BHSM_Crypto_Decrypt );
    return rc;
}


BERR_Code _crypt( BHSM_Handle hHsm, BHSM_CryptographicOperation operation, const BHSM_Crypto *pParam )
{
    BERR_Code rc;
    BHSM_P_CryptoAesBulk bspConf;

    BDBG_ENTER( BHSM_Crypto_Decrypt );

    BKNI_Memset( &bspConf, 0, sizeof(bspConf) );

    if( pParam->algorithm != BHSM_CryptographicAlgorithm_eAes128 ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }

    switch( pParam->algorithmMode )
    {
        case BHSM_CryptographicAlgorithmMode_eEcb:
        {
            switch( operation ) {
                case BHSM_CryptographicOperation_eEncrypt: { bspConf.in.aesOperation = Bsp_Crypto_AesBulkOp_eEncryptECB; break; }
                case BHSM_CryptographicOperation_eDecrypt: { bspConf.in.aesOperation = Bsp_Crypto_AesBulkOp_eDecryptECB; break; }
                default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
            }
            break;
        }
        case BHSM_CryptographicAlgorithmMode_eCbc:
        {
            BDBG_CASSERT( sizeof(bspConf.in.aesCbcIv) == sizeof(pParam->iv) );
            BHSM_Mem32cpy( bspConf.in.aesCbcIv, pParam->iv, sizeof(bspConf.in.aesCbcIv) );

            switch( operation ) {
                case BHSM_CryptographicOperation_eEncrypt: { bspConf.in.aesOperation = Bsp_Crypto_AesBulkOp_eEncryptCBC; break; }
                case BHSM_CryptographicOperation_eDecrypt: { bspConf.in.aesOperation = Bsp_Crypto_AesBulkOp_eDecryptCBC; break; }
                default: { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
            }
            break;
        }
        default:
        {   /*unsupported algorithm mode.*/
            return BERR_TRACE( BERR_INVALID_PARAMETER );
        }
    }

    if( pParam->key.keyladder.handle )
    {
        BHSM_KeyLadderInfo keyladderInfo;

        rc = BHSM_KeyLadder_GetInfo( pParam->key.keyladder.handle, &keyladderInfo );
        if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

        bspConf.in.vklId = keyladderInfo.index;
        bspConf.in.keyLayer = pParam->key.keyladder.level;
    }
    else
    {
        BDBG_CASSERT( sizeof(bspConf.in.aesUserKey) == sizeof(pParam->key.softKey) );
        bspConf.in.isUserKey = 1;
        BHSM_Mem32cpy( bspConf.in.aesUserKey, pParam->key.softKey, sizeof(bspConf.in.aesUserKey) );
    }

    bspConf.in.srcAddrMsb    = (pParam->srcOffset >> 32);
    bspConf.in.srcAddr       = (pParam->srcOffset &  0xFFFFFFFF);
    bspConf.in.destAddrMsb   = (pParam->destOffset >> 32);
    bspConf.in.destAddr      = (pParam->destOffset &  0xFFFFFFFF);

    if( pParam->size == 0 ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }
    if( pParam->size % 4  ) { return BERR_TRACE( BERR_INVALID_PARAMETER ); }  /*size must be a multipe of 4.*/
    bspConf.in.dataLengthWds = pParam->size/4;  /*size in words. */

    bspConf.in.enableSha = pParam->calculateSha ? 1 : 0;

    rc = BHSM_P_Crypto_AesBulk( hHsm, &bspConf );
    if( rc != BERR_SUCCESS ) { return BERR_TRACE( rc ); }

    if( pParam->calculateSha )
    {
        BDBG_CASSERT( sizeof(bspConf.out.shaDigest) == sizeof(pParam->sha) );
        BHSM_Mem32cpy( (void*)pParam->sha, (void*)bspConf.out.shaDigest, sizeof(pParam->sha) );
    }

    BDBG_LEAVE( BHSM_Crypto_Decrypt );
    return rc;
}
