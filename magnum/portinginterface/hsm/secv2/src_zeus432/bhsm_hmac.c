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


#include "bhsm.h"
#include "bhsm_priv.h"
#include "bhsm_bsp_msg.h"
#include "bsp_s_hmac_sha1.h"
#include "bstd.h"
#include "bhsm.h"
#include "bhsm_hash.h"
#include "bhsm_hash_priv.h"
#include "bhsm_hmac.h"

BDBG_MODULE( BHSM );

BHSM_HmacHandle BHSM_Hmac_Create( BHSM_Handle hHsm )
{
    BHSM_HashHandle hashHandle = NULL;

    BDBG_ENTER( BHSM_Hmac_Create );

    if( !hHsm ){ BERR_TRACE(BERR_INVALID_PARAMETER); return NULL; }

    hashHandle = BHSM_Hash_Create( hHsm );

    BDBG_LEAVE( BHSM_Hmac_Create );
    return hashHandle;
}



void BHSM_Hmac_Destroy( BHSM_HmacHandle handle )
{
    BDBG_ENTER( BHSM_Hmac_Destroy );

    if( !handle ){ BERR_TRACE(BERR_INVALID_PARAMETER); return; }

    BHSM_Hash_Destroy_priv( handle );

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
    BHSM_HashSettings hashSettings;
    BERR_Code rc = BERR_SUCCESS;
    BHSM_P_HashOperation_e operation = BHSM_P_HashOperation_eHmac;

    BDBG_ENTER( BHSM_Hmac_SetSettings );

    if( !handle ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if( !pSettings ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    BHSM_Hash_GetDefaultSettings( &hashSettings );

    hashSettings.hashType = pSettings->hashType;
    hashSettings.appendKey = true;

    switch(  pSettings->keySource )
    {
        case BHSM_HmacKeySource_eKeyLadder:
        {
            hashSettings.key.keyladder.handle = pSettings->key.keyladder.handle;
            hashSettings.key.keyladder.level = pSettings->key.keyladder.level;
            operation = BHSM_P_HashOperation_eHmac;
            break;
        }
        case BHSM_HmacKeySource_eSoftwareKey:
        {
            switch( pSettings->hashType ) {
                case BHSM_HashType_e1_160: { hashSettings.key.softKeySize = 160; break; }
                case BHSM_HashType_e2_224: { hashSettings.key.softKeySize = 224; break; }
                case BHSM_HashType_e2_256: { hashSettings.key.softKeySize = 256; break; }
                default: { return BERR_TRACE(BERR_INVALID_PARAMETER); }
            }

            if( (hashSettings.key.softKeySize/8) > sizeof(hashSettings.key.softKey) ) {
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            BKNI_Memcpy( hashSettings.key.softKey, pSettings->key.softKey, hashSettings.key.softKeySize/8 );
            operation = BHSM_P_HashOperation_eHmac;
            break;
        }
        case BHSM_HmacKeySource_eRpmb:
        {

            operation = BHSM_P_HashOperation_eHmacRpmb;
            break;
        }
        default: { return BERR_TRACE(BERR_INVALID_PARAMETER); }
    }

    rc = BHSM_Hash_SetSettings_priv( handle, &hashSettings, operation );
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE(rc); }

    BDBG_LEAVE( BHSM_Hmac_SetSettings );
    return rc;
}

BERR_Code BHSM_Hmac_SubmitData_1char( BHSM_HmacHandle handle, BHSM_HmacSubmitData_1char *pData )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_P_HmacHashSubmitInfo submitInfo;

    BDBG_ENTER( BHSM_Hash_SubmitData );

    if( !handle ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if( !pData ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    BKNI_Memset( &submitInfo, 0, sizeof(submitInfo) );
    submitInfo.useInlineData = true;
    submitInfo.Data = pData->data;
    submitInfo.dataSize = 1;
    submitInfo.last = true;
    submitInfo.pHash = pData->hmac;
    submitInfo.pHashLength = &pData->hmacLength;

    rc = BHSM_Hash_SubmitData_priv( handle, &submitInfo );
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE(rc); }

    BDBG_LEAVE( BHSM_Hash_SubmitData );
    return rc;
}


BERR_Code BHSM_Hmac_SubmitData( BHSM_HmacHandle handle, BHSM_HmacSubmitData *pData )
{
    BERR_Code rc = BERR_UNKNOWN;
    BHSM_P_HmacHashSubmitInfo submitInfo;

    BDBG_ENTER( BHSM_Hmac_SubmitData );

    if( !handle ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }
    if( !pData ){ return BERR_TRACE(BERR_INVALID_PARAMETER); }

    BKNI_Memset( &submitInfo, 0, sizeof(submitInfo) );
    submitInfo.useInlineData = false;
    submitInfo.dataOffset = pData->dataOffset;
    submitInfo.dataSize = pData->dataSize;
    submitInfo.last = pData->last;
    submitInfo.pHash = pData->hmac;
    submitInfo.pHashLength = &pData->hmacLength;

    rc = BHSM_Hash_SubmitData_priv( handle, &submitInfo );
    if( rc != BERR_SUCCESS ) { rc = BERR_TRACE(rc); }

    BDBG_LEAVE( BHSM_Hmac_SubmitData );
    return rc;
}
