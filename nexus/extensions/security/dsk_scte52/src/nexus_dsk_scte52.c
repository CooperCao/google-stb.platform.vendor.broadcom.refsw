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
#include <stdlib.h>
#include <stdio.h>
#include "nexus_base.h"
#include "nexus_security_module.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"
#include "bhsm.h"
#include "bhsm_keyslots.h"
#include "bhsm_keyladder.h"

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
#include "bhsm_restricted_dsk_scte52.h"
#include "bsp_restricted_keycommon_ip1_arris.h"
#endif

BDBG_MODULE( nexus_security );

#define NEXUS_SECURITY_SCTE52_KEY_IV_LENGTH  (48)

void NEXUS_DskScte52_GetDefaultLicenseConfig( NEXUS_DskScte52LicenseConfig *pConfig )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    BKNI_Memset( pConfig, 0, sizeof(*pConfig) );

    pConfig->keyladderType = NEXUS_SecurityKeyladderType_eAes128;
    pConfig->operation = NEXUS_SecurityOperation_eDecrypt;
    pConfig->globalKeyOwnerId = NEXUS_SecurityGlobalKeyOwnerID_eMSP1;
    pConfig->askmGlobalKeyIndex = 0;  /* 0 is the selected global key DONOT CHANGE */
    pConfig->caVendorId = 0xAB1F;
    pConfig->otpId = NEXUS_SecurityOtpId_eOneVal;

    return;
#else
    BSTD_UNUSED(pConfig);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return;
#endif
}


NEXUS_Error NEXUS_DskScte52_LoadLicenseConfig( const NEXUS_DskScte52LicenseConfig *pConfig )
{
#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)

    BHSM_Handle hHsm;
    BERR_Code hsmRc;
    NEXUS_Error rc;
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderInfo vklInfo;
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_KeySlotHandle keyslotHandle;
    NEXUS_SecurityAlgorithmSettings  keyslotSettings;
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCw;
    NEXUS_VirtualKeyLadderHandle vklHandle;
    BHSM_DskScte52LicenseIv_t scte52Config;
    NEXUS_SecurityClearKey key;
    unsigned char iv[16]={0};

#define DEBUG
#ifdef DEBUG
    uint8_t procInKey3[16] = { 0xf1, 0x50, 0xb2, 0x5e, 0x79, 0xd7, 0xde, 0x2c,\
                               0x10, 0x3c, 0x93, 0x7a, 0xec, 0xa6, 0x15, 0xef};  /* fake procin, TO remove */

    uint8_t procInKey4[16] = { 0xFB, 0xFB, 0x39, 0xE7, 0xF9, 0xE3, 0xCB, 0xF1,\
                               0x44, 0x12, 0x92, 0xC9, 0x35, 0x93, 0xEB, 0x7A }; /* fake procin, TO remove */

    uint8_t encBlob[NEXUS_SECURITY_SCTE52_KEY_IV_LENGTH] = {
                               0xCD, 0x1F, 0x4D, 0x0D, 0x03, 0xC6, 0x9F, 0x45,\
                               0x41, 0x66, 0x3C, 0x80, 0xDA, 0x34, 0xBE, 0xE4,\
                               0xB7, 0x77, 0x33, 0xB5, 0x6E, 0x13, 0xF8, 0xFC,\
                               0x7E, 0xA2, 0xDE, 0x93, 0x32, 0x16, 0x2F, 0x84,\
                               0xD6, 0x59, 0xC1, 0x67, 0x49, 0x58, 0x44, 0x0C,\
                               0xC4, 0xFE, 0xB7, 0x8B, 0x31, 0xCE, 0x36, 0x22};
#else
    uint8_t procInKey3[16] = { 0x37, 0x7C, 0xDF, 0xF8, 0x84, 0xC5, 0xC3, 0xD5,
                               0x08, 0x75, 0x7A, 0x50, 0xDB, 0x3B, 0x61, 0xC1 };  /* REAL procin, DO NOT CHANGE */
    uint8_t procInKey4[16] = { 0xBD, 0xEC, 0xBA, 0x0A, 0x5B, 0x94, 0x39, 0x30,
                               0x10, 0xEA, 0x81, 0x1E, 0x5D, 0x08, 0x9D, 0xAF };  /* REAL procin, DO NOT CHANGE */
    uint8_t encBlob[NEXUS_SECURITY_SCTE52_KEY_IV_LENGTH] = {
                               0x47, 0xBA, 0x4D, 0x41, 0xE8, 0xA6, 0xF1, 0x0A,
                               0xDC, 0xF5, 0x19, 0xAF, 0x05, 0x6B, 0x66, 0xA3,
                               0xED, 0x38, 0x68, 0xA9, 0xE7, 0xAF, 0x5D, 0x99,
                               0x4F, 0x44, 0xD2, 0x2B, 0x9F, 0x55, 0xAA, 0x1C,
                               0x56, 0x18, 0x73, 0x78, 0xC8, 0xB1, 0xB8, 0x5C,
                               0xB2, 0x34, 0x02, 0x9E, 0xC6, 0x06, 0x54, 0xBE };
#endif

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( hHsm == NULL ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    NEXUS_Security_GetDefaultVKLSettings ( &vklSettings );
    vklHandle = NEXUS_Security_AllocateVKL ( &vklSettings );
    if( !vklHandle ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    NEXUS_Security_GetVKLInfo ( vklHandle, &vklInfo );

    /* Allocate AV keyslots */
    NEXUS_Security_GetDefaultKeySlotSettings( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keyslotHandle = NEXUS_Security_AllocateKeySlot( &keySettings );
    if( !keyslotHandle )  { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    /* Set up key for keyladder  */
    NEXUS_Security_GetDefaultAlgorithmSettings(&keyslotSettings);
    keyslotSettings.algorithm           = NEXUS_SecurityAlgorithm_eAes;
    keyslotSettings.algorithmVar        = NEXUS_SecurityAlgorithmVariant_eCbc;
    /* ++++++++ */
    keyslotSettings.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
    keyslotSettings.ivMode              = NEXUS_SecurityIVMode_eRegular;
    keyslotSettings.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;
    keyslotSettings.caVendorID          = pConfig->caVendorId;
    keyslotSettings.askmModuleID        = 0x23;
    keyslotSettings.otpId               = pConfig->otpId;
    /* -------- */
    keyslotSettings.key2Select          = NEXUS_SecurityKey2Select_eFixedKey;
    keyslotSettings.dest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
    /* ++++++++ */
    keyslotSettings.operation           = NEXUS_SecurityOperation_eDecrypt;
    keyslotSettings.keyDestEntryType    = NEXUS_SecurityKeyType_eClear;

    rc = NEXUS_Security_ConfigAlgorithm( keyslotHandle, &keyslotSettings );
    if ( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }

    /* Load session key */
    NEXUS_Security_GetDefaultSessionKeySettings( &encryptedSessionkey );
    encryptedSessionkey.keyladderID     = NEXUS_SecurityKeyladderID_eA;
    encryptedSessionkey.keyladderType   = pConfig->keyladderType;
    encryptedSessionkey.swizzleType     = NEXUS_SecuritySwizzleType_eNone;
    /* ++++++++ */
    encryptedSessionkey.keyGenCmdID     = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encryptedSessionkey.sessionKeyOp    = NEXUS_SecuritySessionKeyOp_eNoProcess;
    encryptedSessionkey.bASKMMode       = true;
    /* ++++++++ */
    encryptedSessionkey.rootKeySrc          = NEXUS_SecurityRootKeySrc_eGlobalKey;
    encryptedSessionkey.globalKeyOwnerId    = pConfig->globalKeyOwnerId;
    encryptedSessionkey.askmGlobalKeyIndex  = (uint8_t)pConfig->askmGlobalKeyIndex;
    /* ++++++++ */
    encryptedSessionkey.bSwapAESKey     = false;
    encryptedSessionkey.keyDestIVType   = NEXUS_SecurityKeyIVType_eNoIV;
    /* ++++++++ */
    encryptedSessionkey.bRouteKey       = false;
    encryptedSessionkey.operation       = pConfig->operation;
    encryptedSessionkey.operationKey2   = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.keyEntryType    = NEXUS_SecurityKeyType_eClear;
    /* ++++++++ */
    encryptedSessionkey.custSubMode        = BCMD_CustomerSubMode_eSCTE52_DecProtectionKey;
    encryptedSessionkey.virtualKeyLadderID = vklInfo.vkl;
    encryptedSessionkey.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    /* ++++++++ */
    BKNI_Memcpy(encryptedSessionkey.keyData, procInKey3, sizeof(procInKey3));

    rc = NEXUS_Security_GenerateSessionKey( keyslotHandle, &encryptedSessionkey );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }

    /* Generate Key 4 */
    NEXUS_Security_GetDefaultControlWordSettings(&encrytedCw);
    encrytedCw.keyladderID = NEXUS_SecurityKeyladderID_eA;
    encrytedCw.keyladderType = pConfig->keyladderType;
    encrytedCw.keySize = sizeof(procInKey4);
    encrytedCw.keyEntryType = NEXUS_SecurityKeyType_eClear;

    encrytedCw.custSubMode        = BCMD_CustomerSubMode_eSCTE52_DecProtectionKey;
    encrytedCw.virtualKeyLadderID = vklInfo.vkl;
    encrytedCw.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    encrytedCw.bASKMMode          = true;
    encrytedCw.operation          = pConfig->operation;

    encrytedCw.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
    encrytedCw.keyGenCmdID     = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encrytedCw.bSwapAESKey     = false;

    BKNI_Memcpy( encrytedCw.keyData, procInKey4, sizeof(procInKey4) );

    encrytedCw.keylayer = NEXUS_SecurityKeyLayer_eKey4;
    encrytedCw.bRouteKey = false;  /* no route, stay BSP internal  */
    encrytedCw.protectionKeyIvSource = true;

    rc = NEXUS_Security_GenerateNextLayerKey( keyslotHandle, &encrytedCw );
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }

    NEXUS_Security_GetDefaultClearKey(&key);
    key.keySize = sizeof(iv);
    key.keyEntryType = NEXUS_SecurityKeyType_eIv;
    key.keyIVType = NEXUS_SecurityKeyIVType_eIV;
    BKNI_Memcpy(key.keyData, iv, sizeof(iv));

    rc = NEXUS_Security_LoadClearKey (keyslotHandle, &key);
    if( rc != NEXUS_SUCCESS ) { return BERR_TRACE( rc ); }

    scte52Config.virtualKeyLadderID = vklInfo.vkl;
    BKNI_Memcpy( scte52Config.encBlob, encBlob, NEXUS_SECURITY_SCTE52_KEY_IV_LENGTH );
    hsmRc = BHSM_SetDskScte52LicenseIv( hHsm, &scte52Config );
    if( hsmRc != BERR_SUCCESS ) { return BERR_TRACE( MAKE_HSM_ERR(hsmRc) ); }

    NEXUS_Security_FreeKeySlot( keyslotHandle );
    NEXUS_Security_FreeVKL( vklHandle );

    return NEXUS_SUCCESS;
#else

    BSTD_UNUSED(pConfig);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}
