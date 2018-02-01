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

#include "nexus_base.h"
#include "nexus_security_module.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"

#include "bhsm.h"
#include "bhsm_misc.h"
#include "bhsm_keyladder.h"
#include "bhsm_keyladder_enc.h"

/**
Summary:
This struct defines the Virtual Key Ladder (VKL) handle structure.

Description:
This structure contains the virtual key ladder handle parameters which will be filled
in NEXUS_Security_AllocateVKL.
calls.

See Also:
NEXUS_Security_AllocateVKL
**/

struct NEXUS_VirtualKeyLadder
{
    NEXUS_OBJECT(NEXUS_VirtualKeyLadder);
    NEXUS_SecurityVirtualKeyladderID      vkl;
    NEXUS_SecurityCustomerSubMode         custSubMode;
    NEXUS_SecurityClientType              client;

};


BDBG_MODULE(nexus_security);


void NEXUS_Security_GetDefaultSessionKeySettings(
    NEXUS_SecurityEncryptedSessionKey  *pSettings
    )
{
    BDBG_ENTER(NEXUS_Security_GetDefaultSessionKeySettings);

    if( !pSettings ){ BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->keyGenCmdID     = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    pSettings->keyladderType   = NEXUS_SecurityKeyladderType_e3Des;
   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    pSettings->keyLadderSelect = NEXUS_SecurityKeyLadderSelect_eFWKL;
   #endif
    pSettings->cusKeySwizzle0aType = NEXUS_SecuritySwizzle0aType_eDisabled;
    pSettings->keySize         = 16;

    BDBG_LEAVE(NEXUS_Security_GetDefaultSessionKeySettings);
    return;
}


void NEXUS_Security_GetDefaultControlWordSettings(
    NEXUS_SecurityEncryptedControlWord  *pSettings
    )
{
    BDBG_ENTER(NEXUS_Security_GetDefaultControlWordSettings);

    if( !pSettings ){ BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->keyGenCmdID       = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    pSettings->keyladderType   = NEXUS_SecurityKeyladderType_e3Des;
  #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    pSettings->keyLadderSelect = NEXUS_SecurityKeyLadderSelect_eFWKL;
    pSettings->sc01Polarity[NEXUS_SecurityPacketType_eGlobal] = NEXUS_SecurityAlgorithmScPolarity_eClear;
    pSettings->sc01Polarity[NEXUS_SecurityPacketType_eRestricted] = NEXUS_SecurityAlgorithmScPolarity_eClear;
    #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,2)
    pSettings->sourceDupleKeyLadderId = NEXUS_SecurityVirtualKeyladderID_eMax;
    #endif
  #endif

    BDBG_LEAVE(NEXUS_Security_GetDefaultControlWordSettings);
    return;
}


static NEXUS_Error NEXUS_Security_GetHsmDestBlkType(
                                    NEXUS_KeySlotHandle                      keyHandle,
                                    NEXUS_SecurityAlgorithmConfigDestination dest,
                                    BCMD_KeyDestBlockType_e                  *pType )
{
    BDBG_ENTER(NEXUS_Security_GetHsmDestBlkType);

    switch( keyHandle->settings.keySlotEngine )
    {
        case NEXUS_SecurityEngine_eCa: { *pType = BCMD_KeyDestBlockType_eCA; break; }
        case NEXUS_SecurityEngine_eM2m: { *pType = BCMD_KeyDestBlockType_eMem2Mem; break; }
        case NEXUS_SecurityEngine_eCaCp:
        {
            if( dest == NEXUS_SecurityAlgorithmConfigDestination_eCa )
            {
                *pType = BCMD_KeyDestBlockType_eCA;
            }
            else if( dest == NEXUS_SecurityAlgorithmConfigDestination_eCpd )
            {
                *pType = BCMD_KeyDestBlockType_eCPDescrambler;
            }
            else if( dest == NEXUS_SecurityAlgorithmConfigDestination_eCps )
            {
                *pType = BCMD_KeyDestBlockType_eCPScrambler;
            }
            break;
        }
        case NEXUS_SecurityEngine_eCp:
        {
            if( dest == NEXUS_SecurityAlgorithmConfigDestination_eCpd )
            {
                 *pType = BCMD_KeyDestBlockType_eCPDescrambler;
            }
            else if( dest == NEXUS_SecurityAlgorithmConfigDestination_eCps )
            {
                *pType = BCMD_KeyDestBlockType_eCPScrambler;
            }
            break;
        }
        case NEXUS_SecurityEngine_eRmx:
        {
            BDBG_ERR(("Remux is not supported on 40nm HSM"));
            *pType = BCMD_KeyDestBlockType_eCA;
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        default:
        {
            /* There is no meaningful default, error. */
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }

    BDBG_LEAVE(NEXUS_Security_GetHsmDestBlkType);
    return NEXUS_SUCCESS;
}



static NEXUS_Error NEXUS_Security_GetHsmDestEntrykType(
    NEXUS_SecurityKeyType   keytype,
    BCMD_KeyDestEntryType_e *pType
    )
{
    switch (keytype)
    {
        case NEXUS_SecurityKeyType_eEven:   *pType = BCMD_KeyDestEntryType_eEvenKey; break;
        case NEXUS_SecurityKeyType_eOdd:    *pType = BCMD_KeyDestEntryType_eOddKey; break;
        case NEXUS_SecurityKeyType_eClear:  *pType = BCMD_KeyDestEntryType_eClearKey; break;
        default:                            *pType = BCMD_KeyDestEntryType_eOddKey; break;
    }
    return NEXUS_SUCCESS;
}

static NEXUS_Error NEXUS_Security_GetHsmDestIVType(
    NEXUS_SecurityKeyIVType keyIVtype,
    BCMD_KeyDestIVType_e    *pType
    )
{
    switch( keyIVtype )
    {
        case NEXUS_SecurityKeyIVType_eNoIV:         *pType = BCMD_KeyDestIVType_eNoIV; break;
        case NEXUS_SecurityKeyIVType_eIV:           *pType = BCMD_KeyDestIVType_eIV; break;
        case NEXUS_SecurityKeyIVType_eAesShortIV:   *pType = BCMD_KeyDestIVType_eAesShortIV; break;
        default:                                    *pType = BCMD_KeyDestIVType_eNoIV; break;
    }
    return NEXUS_SUCCESS;
}


static NEXUS_Error NEXUS_Security_GetKeyLadderType(
    NEXUS_SecurityKeyladderType klType,
    BCMD_KeyLadderType_e        *pType
    )
{
    switch( klType )
    {
        case NEXUS_SecurityKeyladderType_e1Des:     *pType = BCMD_KeyLadderType_e1DES; break;
        case NEXUS_SecurityKeyladderType_e3Des:     *pType = BCMD_KeyLadderType_e3DESABA; break;
        case NEXUS_SecurityKeyladderType_eAes128:   *pType = BCMD_KeyLadderType_eAES128; break;
    #if (BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)) && NEXUS_SUPPORT_HDDTA
        case NEXUS_SecurityKeyladderType_ePKSM:     *pType = BCMD_KeyLadderType_ePKSM; break;
    #endif
        default:                                    *pType = BCMD_KeyLadderType_e3DESABA; break;
    }
    return NEXUS_SUCCESS;
}


static NEXUS_Error NEXUS_Security_GetRootKeySource(
    NEXUS_SecurityRootKeySrc    ksType,
    BCMD_RootKeySrc_e           *pType
    )
{
    switch (ksType)
    {
        case NEXUS_SecurityRootKeySrc_eOtpKeyA:   *pType = BCMD_RootKeySrc_eOTPKeya; break;
        case NEXUS_SecurityRootKeySrc_eOtpKeyB:   *pType = BCMD_RootKeySrc_eOTPKeyb; break;
        case NEXUS_SecurityRootKeySrc_eOtpKeyC:   *pType = BCMD_RootKeySrc_eOTPKeyc; break;
        case NEXUS_SecurityRootKeySrc_eOtpKeyD:   *pType = BCMD_RootKeySrc_eOTPKeyd; break;
        case NEXUS_SecurityRootKeySrc_eOtpKeyE:   *pType = BCMD_RootKeySrc_eOTPKeye; break;
        case NEXUS_SecurityRootKeySrc_eOtpKeyF:   *pType = BCMD_RootKeySrc_eOTPKeyf; break;
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
        case NEXUS_SecurityRootKeySrc_eOtpKeyG:   *pType = BCMD_RootKeySrc_eOTPKeyg; break;
        case NEXUS_SecurityRootKeySrc_eOtpKeyH:   *pType = BCMD_RootKeySrc_eOTPKeyh; break;
        #endif
        case NEXUS_SecurityRootKeySrc_eReserved0: *pType = (BCMD_RootKeySrc_e)BCMD_RootKeySrc_eReserved0; break;
        case NEXUS_SecurityRootKeySrc_eReserved1: *pType = (BCMD_RootKeySrc_e)BCMD_RootKeySrc_eReserved1; break;
        case NEXUS_SecurityRootKeySrc_eReserved2: *pType = (BCMD_RootKeySrc_e)BCMD_RootKeySrc_eReserved2; break;
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
        case NEXUS_SecurityRootKeySrc_eGlobalKey: *pType =  (BCMD_RootKeySrc_e)BCMD_RootKeySrc_eASKMGlobalKey; break;
        #endif
        case NEXUS_SecurityRootKeySrc_eCuskey:
        default:                                  *pType = BCMD_RootKeySrc_eCusKey; break;
    }

    return NEXUS_SUCCESS;
}


static NEXUS_Error NEXUS_Security_GetSwizzleType(
    NEXUS_SecuritySwizzleType   swType,
    BCMD_SwizzleType_e          *pType
    )
{
    switch (swType)
    {
        case NEXUS_SecuritySwizzleType_eNone:       *pType = BCMD_SwizzleType_eNoSwizzle; break;
        case NEXUS_SecuritySwizzleType_eSwizzle1:   *pType = BCMD_SwizzleType_eSwizzle1; break;
        case NEXUS_SecuritySwizzleType_eSwizzle0:   *pType = BCMD_SwizzleType_eSwizzle0; break;
        default:                                    *pType = BCMD_SwizzleType_eNoSwizzle; break;
    }
    return NEXUS_SUCCESS;
}


#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)

static BCMD_XptM2MSecCryptoAlg_e NEXUS_Security_MapNexusAlgorithmToHsm(NEXUS_SecurityAlgorithm algorithm)
{
    BCMD_XptM2MSecCryptoAlg_e rvAlgorithm = algorithm;

    switch( algorithm )
    {
        case NEXUS_SecurityAlgorithm_eDvbCsa2:        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eDVBCSA2; break;
        case NEXUS_SecurityAlgorithm_eMulti2:         rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eMulti2; break;
        case NEXUS_SecurityAlgorithm_eDes:            rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eDes; break;
        case NEXUS_SecurityAlgorithm_e3DesAba:        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_e3DesAba; break;
        case NEXUS_SecurityAlgorithm_e3DesAbc:        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_e3DesAbc; break;
        case NEXUS_SecurityAlgorithm_eAes:            rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eAes128; break;
        case NEXUS_SecurityAlgorithm_eAes192:         rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eAes192; break;
        case NEXUS_SecurityAlgorithm_eC2:             rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eC2; break;
        case NEXUS_SecurityAlgorithm_eM6Ke:           rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eM6KE; break;
        case NEXUS_SecurityAlgorithm_eM6:             rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eM6; break;
        case NEXUS_SecurityAlgorithm_eWMDrmPd:        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eWMDrmPd; break;
        case NEXUS_SecurityAlgorithm_eDvbCsa3:        rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eDVBCSA3; break;
        case NEXUS_SecurityAlgorithm_eAesCounter:     rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eAesCounter0; break;
        case NEXUS_SecurityAlgorithm_eMSMultiSwapMac: rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eMSMULTISWAPMAC; break;
        #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
        case NEXUS_SecurityAlgorithm_eAsa:            rvAlgorithm = BCMD_XptM2MSecCryptoAlg_eReserved19; break;
        #endif
        case NEXUS_SecurityAlgorithm_eReserved0: rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e)0; break;
        case NEXUS_SecurityAlgorithm_eReserved1: rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e)1; break;
        case NEXUS_SecurityAlgorithm_eReserved2: rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e)2; break;
        case NEXUS_SecurityAlgorithm_eReserved3: rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e)3; break;
        case NEXUS_SecurityAlgorithm_eReserved4: rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e)4; break;
        case NEXUS_SecurityAlgorithm_eReserved5: rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e)5; break;
        case NEXUS_SecurityAlgorithm_eReserved6: rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e)6; break;
        case NEXUS_SecurityAlgorithm_eReserved7: rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e)7; break;
        case NEXUS_SecurityAlgorithm_eReserved8: rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e)8; break;
        case NEXUS_SecurityAlgorithm_eReserved9: rvAlgorithm = (BCMD_XptM2MSecCryptoAlg_e)9; break;
        default: break;
    }

    return rvAlgorithm;
}

#endif

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)

static uint32_t NEXUS_Security_MapNexusSwizzle0aTypeToHsm(NEXUS_SecuritySwizzle0aType swizzle0aType)
{

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    switch( swizzle0aType )
    {
        case NEXUS_SecuritySwizzle0aType_eNoMsp: return BCMD_OwnerIDSelect_eUse1;
        case NEXUS_SecuritySwizzle0aType_eMsp0: return BCMD_OwnerIDSelect_eMSP0;
        case NEXUS_SecuritySwizzle0aType_eMsp1: return BCMD_OwnerIDSelect_eMSP1;
        case NEXUS_SecuritySwizzle0aType_eDisabled:
        default: return BCMD_OwnerIDSelect_eMax;
    }
   #else
    if( swizzle0aType == NEXUS_SecuritySwizzle0aType_eDisabled )
    {
        return BCMD_Swizzle0aType_eMax;  /* Invalid parameter */
    }
   #endif

    return swizzle0aType;  /* Direct mapping between NEXUS and HSM type. */
}

#endif

static void NEXUS_Security_InitGenerateRouteKey(
    BHSM_GenerateRouteKeyIO_t   * generateRouteKeyPtr,
    BCMD_KeyLadderType_e klType,    BCMD_RootKeySrc_e rootKsrc,
    BCMD_SwizzleType_e swizzleType, unsigned char kLow, unsigned char kLowV, unsigned char kHigh, unsigned char kHighV,
    BCMD_KeyRamBuf_e kDest,
    BCMD_KeySize_e kSize, bool routeFlag, BCMD_KeyDestEntryType_e oddEvenEntry, unsigned int slotNum,
    bool useCustLow,
        bool useCustHigh,
        bool operateCrption,
        BCMD_KeyDestBlockType_e blockType
    )
{
    BDBG_ENTER(NEXUS_Security_InitGenerateRouteKey);

    BKNI_Memset(generateRouteKeyPtr, 0, sizeof(BHSM_GenerateRouteKeyIO_t));

    generateRouteKeyPtr->keyLadderType = klType; /* mostly fixed to BCMD_KeyLadderType_e3DESABA */
    generateRouteKeyPtr->rootKeySrc = rootKsrc;
    generateRouteKeyPtr->ucSwizzle1Index = 0;         /*fixed */

    generateRouteKeyPtr->swizzleType = swizzleType; /* valid only if rootkeysrc is custkey */
    if (swizzleType == BCMD_SwizzleType_eSwizzle1){
        generateRouteKeyPtr->ucSwizzle1Index = kLow;   /* swizzle1 needs a valid index, re-define kLow*/
    }else if (swizzleType == BCMD_SwizzleType_eSwizzle0) {
        generateRouteKeyPtr->ucCustKeyLow = kLow;
        generateRouteKeyPtr->ucKeyVarLow= kLowV;
        generateRouteKeyPtr->ucCustKeyHigh = kHigh;
        generateRouteKeyPtr->ucKeyVarHigh= kHighV;

    }
    generateRouteKeyPtr->bUseCustKeyLowDecrypt   =  useCustLow;
    generateRouteKeyPtr->bUseCustKeyHighDecrypt  = useCustHigh;
    generateRouteKeyPtr->keyLayer                = kDest;
    generateRouteKeyPtr->bIsKeyLadder3DESEncrypt = operateCrption;
    generateRouteKeyPtr->keySize= kSize;
    generateRouteKeyPtr->bIsRouteKeyRequired = routeFlag;
    generateRouteKeyPtr->keyDestBlckType = blockType;       /*fixed */
    generateRouteKeyPtr->keyDestEntryType = oddEvenEntry;
    generateRouteKeyPtr->caKeySlotType = 0;                  /*no matter for m2m */
    generateRouteKeyPtr->unKeySlotNum =  slotNum;

    BDBG_LEAVE(NEXUS_Security_InitGenerateRouteKey);
    return;
}

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
static int g_scValues[NEXUS_SecurityAlgorithmScPolarity_eMax] = { 0, 1, 2, 3 };
#endif


NEXUS_Error NEXUS_Security_GenerateSessionKey(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedSessionKey *pSessionKey
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCMD_KeyDestBlockType_e blockType = BCMD_KeyDestBlockType_eCA;
    BCMD_KeyDestEntryType_e entryType = BCMD_KeyDestEntryType_eOddKey;
    BCMD_KeyDestIVType_e    ivType    = BCMD_KeyDestIVType_eIV;
    BHSM_GenerateRouteKeyIO_t   generateRouteKeyIO;
    bool bDecrypt;
    bool bKey2Decrypt;
    BCMD_RootKeySrc_e rootkeySrc = BCMD_RootKeySrc_eCusKey;
    BCMD_KeyRamBuf_e keyLayer;
    BHSM_Handle hHsm;
    BCMD_KeyLadderType_e    keyLadderType;
    BCMD_SwizzleType_e      swizzleType;
    NEXUS_SecurityKeyType keyEntryType;


    BDBG_ENTER(NEXUS_Security_GenerateSessionKey);

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm || !pSessionKey ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }

    keyLayer = BCMD_KeyRamBuf_eKey3;

    keyEntryType = pSessionKey->keyEntryType;

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    if( keyHandle->keyslotType == BCMD_XptSecKeySlot_eType3 )
    {
        /* Clear key entry must be used for Type 3, m2m  */
        keyEntryType = NEXUS_SecurityKeyType_eClear;
    }
#endif

    NEXUS_Security_GetHsmDestBlkType (keyHandle, pSessionKey->dest, &blockType);
    NEXUS_Security_GetHsmDestEntrykType( keyEntryType, &entryType );
    NEXUS_Security_GetHsmDestIVType(pSessionKey->keyDestIVType, &ivType);
    bDecrypt = (pSessionKey->operation==NEXUS_SecurityOperation_eDecrypt)? false: true;
    bKey2Decrypt = (pSessionKey->operationKey2==NEXUS_SecurityOperation_eDecrypt)? true: false;

    NEXUS_Security_GetKeyLadderType(pSessionKey->keyladderType, &keyLadderType);
    NEXUS_Security_GetRootKeySource(pSessionKey->rootKeySrc, &rootkeySrc);
    NEXUS_Security_GetSwizzleType(pSessionKey->swizzleType, &swizzleType);
    NEXUS_Security_InitGenerateRouteKey( &generateRouteKeyIO,
                keyLadderType,
                rootkeySrc,
                swizzleType,
                pSessionKey->cusKeyL, pSessionKey->cusKeyVarL, pSessionKey->cusKeyH, pSessionKey->cusKeyVarH,
                keyLayer,
                (pSessionKey->keySize == 8) ? BCMD_KeySize_e64 : BCMD_KeySize_e128,
                pSessionKey->bRouteKey,
                entryType,
                keyHandle->keySlotNumber,
                bKey2Decrypt,
                bKey2Decrypt,
                bDecrypt,
                blockType);

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    generateRouteKeyIO.keyLadderSelect              = pSessionKey->keyLadderSelect;
    generateRouteKeyIO.bASKM3DesKLRootKeySwapEnable = pSessionKey->rootKeySwap;
    generateRouteKeyIO.keyGenMode                   = pSessionKey->bkeyGenBlocked;

    /* For Hardware Key Ladder */
    generateRouteKeyIO.hwklLength                   = (BCMD_HwKeyLadderLength_e)pSessionKey->hwklLen;
    generateRouteKeyIO.hwklDestAlg                  = NEXUS_Security_MapNexusAlgorithmToHsm(pSessionKey->hwklDestAlg);
    BKNI_Memset(generateRouteKeyIO.activationCode, 0, NEXUS_SECURITY_KL_ACTCODE_SIZE);
    BKNI_Memcpy(generateRouteKeyIO.activationCode, pSessionKey->actCode, NEXUS_SECURITY_KL_ACTCODE_SIZE);

    /* BHSM_SUPPORT_HDDTA support */
    generateRouteKeyIO.bResetPKSM      = pSessionKey->pksm.reset;
    generateRouteKeyIO.PKSMInitSize    = pSessionKey->pksm.initState;
    generateRouteKeyIO.PKSMcycle       = pSessionKey->pksm.cycle;
   #endif


   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    generateRouteKeyIO.client = pSessionKey->client;

    generateRouteKeyIO.cusKeySwizzle0aEnable = pSessionKey->cusKeySwizzle0aEnable;
    if (pSessionKey->cusKeySwizzle0aEnable)
    {
        generateRouteKeyIO.cusKeySwizzle0aVariant = NEXUS_Security_MapNexusSwizzle0aTypeToHsm(pSessionKey->cusKeySwizzle0aType);
        generateRouteKeyIO.cusKeySwizzle0aVersion = pSessionKey->cusKeySwizzle0aVer;
    }
    else if(pSessionKey->cusKeySwizzle0aType != NEXUS_SecuritySwizzle0aType_eDisabled)
    {
        BDBG_WRN(("cusKeySwizzle0aEnable must be enabled to activate Swizzle0a"));
    }
    generateRouteKeyIO.sageAskmConfigurationEnable = pSessionKey->sage.askmConfigurationEnable;
    generateRouteKeyIO.sageModuleID                = pSessionKey->sage.moduleID;
    generateRouteKeyIO.sageSTBOwnerID              = (BCMD_STBOwnerID_e)(pSessionKey->sage.otpId);
    generateRouteKeyIO.sageCAVendorID              = pSessionKey->sage.caVendorId;
    generateRouteKeyIO.sageMaskKeySelect           = pSessionKey->sage.maskKeySelect;
   #endif


   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,1)
    generateRouteKeyIO.AskmGlobalKeyIndex    = pSessionKey->askmGlobalKeyIndex;
    generateRouteKeyIO.globalKeyOwnerId      = pSessionKey->globalKeyOwnerId;
    generateRouteKeyIO.globalKeyVersion      = pSessionKey->globalKeyVersion;
   #endif
    generateRouteKeyIO.keyTweak         = pSessionKey->keyTweakOp;
    generateRouteKeyIO.sourceDupleKeyLadderId = pSessionKey->sourceDupleKeyLadderId;

   #if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    /* HWKL CWC setting */
    generateRouteKeyIO.bHWKLVistaKeyGenEnable = pSessionKey->hwklVistaKeyGen;
   #endif

    generateRouteKeyIO.virtualKeyLadderID = pSessionKey->virtualKeyLadderID;
    generateRouteKeyIO.customerSubMode    = pSessionKey->custSubMode;
    generateRouteKeyIO.keyMode            = pSessionKey->keyMode;

    if(pSessionKey->keySize == 8) {
        generateRouteKeyIO.ucKeyDataLen = 8;
        BKNI_Memset(generateRouteKeyIO.aucKeyData, 0, 8);
        BKNI_Memcpy(generateRouteKeyIO.aucKeyData, pSessionKey->keyData, 8);
    }
    else {
        generateRouteKeyIO.ucKeyDataLen       = 16;
        BKNI_Memset(generateRouteKeyIO.aucKeyData, 0, 16);
        BKNI_Memcpy(generateRouteKeyIO.aucKeyData, pSessionKey->keyData, 16);
    }

    generateRouteKeyIO.unKeySlotNum     = keyHandle->keySlotNumber;
    generateRouteKeyIO.caKeySlotType    = keyHandle->keyslotType;
    generateRouteKeyIO.bASKMModeEnabled = pSessionKey->bASKMMode;
    generateRouteKeyIO.bSwapAesKey      = pSessionKey->bSwapAESKey;
    generateRouteKeyIO.key3Op           = pSessionKey->sessionKeyOp;
    generateRouteKeyIO.keyDestIVType    = ivType;
    generateRouteKeyIO.subCmdID         = pSessionKey->keyGenCmdID;
    generateRouteKeyIO.applyKeyContribution = pSessionKey->applyKeyContribution;

    rc= BHSM_GenerateRouteKey (hHsm, &generateRouteKeyIO);
    if( rc ) { return BERR_TRACE( MAKE_HSM_ERR(rc) ); }

    BDBG_LEAVE(NEXUS_Security_GenerateSessionKey);
    return NEXUS_SUCCESS;
}




static NEXUS_Error NEXUS_Security_GenerateKey4or5(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedControlWord * pCW,
    bool bKey4
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCMD_KeyDestBlockType_e blockType = BCMD_KeyDestBlockType_eCA;
    BCMD_KeyDestEntryType_e entryType = BCMD_KeyDestEntryType_eOddKey;
    BCMD_KeyDestIVType_e    ivType    = BCMD_KeyDestIVType_eIV;
    BHSM_GenerateRouteKeyIO_t   generateRouteKeyIO;
    bool bDecrypt;
    BCMD_KeySize_e keySize;
    BCMD_KeyRamBuf_e keyLayer;
    BHSM_Handle hHsm;
    BCMD_RootKeySrc_e rootkeySrc = BCMD_RootKeySrc_eCusKey;
    BCMD_KeyLadderType_e  keyLadderType;
    NEXUS_SecurityKeyType keyEntryType;

    BDBG_ENTER(NEXUS_Security_GenerateKey4or5);

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm || !pCW ) return BERR_TRACE( NEXUS_INVALID_PARAMETER );

    keyEntryType = pCW->keyEntryType;

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    if( keyHandle->keyslotType == BCMD_XptSecKeySlot_eType3 )
    {
        /* Clear key entry must be used for Type 3, m2m  */
        keyEntryType = NEXUS_SecurityKeyType_eClear;
    }
#endif

    if ( pCW->keySize==8 )      keySize = BCMD_KeySize_e64;
    else if( pCW->keySize==16 ) keySize = BCMD_KeySize_e128;
    else                        keySize = BCMD_KeySize_e192;

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    if(pCW->keyLadderSelect == NEXUS_SecurityKeyLadderSelect_ePKL)
    {
        keyLayer = 10;
    }
    else
    {
        keyLayer = BCMD_KeyRamBuf_eKey4;
        if ( !bKey4 )
            keyLayer ++;
    }
#else
    keyLayer = BCMD_KeyRamBuf_eKey4;
    if ( !bKey4 )
        keyLayer ++;

#endif

    NEXUS_Security_GetHsmDestBlkType (keyHandle, pCW->dest, &blockType);
    NEXUS_Security_GetHsmDestEntrykType( keyEntryType, &entryType );
    NEXUS_Security_GetHsmDestIVType(pCW->keyDestIVType, &ivType);
    bDecrypt = (pCW->operation==NEXUS_SecurityOperation_eDecrypt)? false: true;
    NEXUS_Security_GetKeyLadderType(pCW->keyladderType, &keyLadderType);
    NEXUS_Security_GetRootKeySource(pCW->rootKeySrc, &rootkeySrc);
    NEXUS_Security_InitGenerateRouteKey( &generateRouteKeyIO,
                keyLadderType,
                0,
                0,
                0, 0, 0, 0,
                keyLayer,
                keySize,
                pCW->bRouteKey,
                entryType,
                keyHandle->keySlotNumber,
                bDecrypt,
                bDecrypt,
                bDecrypt,
                blockType);

    generateRouteKeyIO.rootKeySrc         = rootkeySrc;             /* need this for ASKM keyladder distinction */
    generateRouteKeyIO.virtualKeyLadderID = pCW->virtualKeyLadderID;
    generateRouteKeyIO.customerSubMode      = pCW->custSubMode;
    generateRouteKeyIO.keyMode              = pCW->keyMode;
    generateRouteKeyIO.ucKeyDataLen       = pCW->keySize;
    BKNI_Memset(generateRouteKeyIO.aucKeyData, 0,  pCW->keySize);
    if( pCW->keySize > sizeof(generateRouteKeyIO.aucKeyData) ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
    BKNI_Memcpy(generateRouteKeyIO.aucKeyData, pCW->keyData,  pCW->keySize);

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    generateRouteKeyIO.keyLadderSelect                = pCW->keyLadderSelect;
    generateRouteKeyIO.bASKM3DesKLRootKeySwapEnable = pCW->rootKeySwap;
    generateRouteKeyIO.GpipeSC01Val                 = g_scValues[pCW->sc01Polarity[NEXUS_SecurityPacketType_eGlobal]];
    generateRouteKeyIO.RpipeSC01Val                 = g_scValues[pCW->sc01Polarity[NEXUS_SecurityPacketType_eRestricted]];
    generateRouteKeyIO.keyGenMode                    = pCW->bkeyGenBlocked;

    /* For Hardware Key Ladder */
    generateRouteKeyIO.hwklLength                    = (BCMD_HwKeyLadderLength_e)pCW->hwklLen;
    generateRouteKeyIO.hwklDestAlg                    = NEXUS_Security_MapNexusAlgorithmToHsm(pCW->hwklDestAlg);
    BKNI_Memset(generateRouteKeyIO.activationCode, 0, NEXUS_SECURITY_KL_ACTCODE_SIZE);
    BKNI_Memcpy(generateRouteKeyIO.activationCode, pCW->actCode, NEXUS_SECURITY_KL_ACTCODE_SIZE);

#endif

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(3,0)
    generateRouteKeyIO.client             = pCW->client;
    generateRouteKeyIO.SC01ModeMapping    = pCW->sc01GlobalMapping;
#endif
    generateRouteKeyIO.protectionKeyIvSource = pCW->protectionKeyIvSource;
    generateRouteKeyIO.keyTweak         = pCW->keyTweakOp;
    generateRouteKeyIO.sourceDupleKeyLadderId = pCW->sourceDupleKeyLadderId;
    generateRouteKeyIO.unKeySlotNum     = keyHandle->keySlotNumber;
    generateRouteKeyIO.caKeySlotType    = keyHandle->keyslotType;
    generateRouteKeyIO.bASKMModeEnabled = pCW->bASKMMode;
    generateRouteKeyIO.bSwapAesKey      = pCW->bSwapAESKey;
    generateRouteKeyIO.keyDestIVType    = ivType;

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    generateRouteKeyIO.subCmdID         = (BCMD_VKLAssociationQueryFlag_e)pCW->keyGenCmdID;
#else
    generateRouteKeyIO.subCmdID         = pCW->keyGenCmdID;
#endif
    generateRouteKeyIO.applyKeyContribution = pCW->applyKeyContribution;

    rc= BHSM_GenerateRouteKey (hHsm, &generateRouteKeyIO);
    if( rc ) { return BERR_TRACE( MAKE_HSM_ERR(rc) ); }

    BDBG_LEAVE(NEXUS_Security_GenerateKey4or5);
    return NEXUS_SUCCESS;
}




NEXUS_Error NEXUS_Security_GenerateControlWord(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedControlWord * pCW
    )
{
    NEXUS_Error retVal;

    BDBG_ENTER(NEXUS_Security_GenerateControlWord);

    retVal =  NEXUS_Security_GenerateKey4or5 (keyHandle, pCW, true);
    if( retVal ) { return BERR_TRACE( MAKE_HSM_ERR(retVal) ); }

    BDBG_LEAVE(NEXUS_Security_GenerateControlWord);
    return retVal;
}

NEXUS_Error NEXUS_Security_GenerateKey5(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedControlWord * pCW
    )
{
    NEXUS_Error retVal;

    BDBG_ENTER(NEXUS_Security_GenerateKey5);

    retVal =  NEXUS_Security_GenerateKey4or5 (keyHandle, pCW, false);

    BDBG_LEAVE(NEXUS_Security_GenerateKey5);
    return retVal;

}


void NEXUS_Security_GetDefaultVKLSettings(
    NEXUS_SecurityVKLSettings *pVKLSettings
    )
{
    BDBG_ENTER(NEXUS_Security_GetDefaultVKLSettings);

    if( !pVKLSettings ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset( pVKLSettings, 0, sizeof(*pVKLSettings) );
    pVKLSettings->client = NEXUS_SecurityClientType_eHost;
    pVKLSettings->newVKLCustSubModeAssoc = false;

    BDBG_LEAVE(NEXUS_Security_GetDefaultVKLSettings);
}



NEXUS_VirtualKeyLadderHandle NEXUS_Security_AllocateVKL(
    const NEXUS_SecurityVKLSettings *pVKLSettings
    )
{
    NEXUS_Error                   rc = NEXUS_SUCCESS;
    BHSM_AllocateVKLIO_t          allocateVKLIO;
    BHSM_Handle                   hHsm;
    NEXUS_VirtualKeyLadderHandle  pVKLHandle = NULL;

    BDBG_ENTER(NEXUS_Security_AllocateVKL);

    BDBG_CASSERT( (int)BHSM_ClientType_eHost == (int)NEXUS_SecurityClientType_eHost );
    BDBG_CASSERT( (int)BHSM_ClientType_eSAGE == (int)NEXUS_SecurityClientType_eSage );

    NEXUS_Security_GetHsm_priv( &hHsm );
    if( hHsm == NULL || !pVKLSettings ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return NULL; }

    allocateVKLIO.client                  = pVKLSettings->client;
    allocateVKLIO.customerSubMode         = pVKLSettings->custSubMode;
    allocateVKLIO.bNewVKLCustSubModeAssoc = pVKLSettings->newVKLCustSubModeAssoc;

    rc= BHSM_AllocateVKL( hHsm, &allocateVKLIO );
    if( rc ) { BERR_TRACE( MAKE_HSM_ERR(rc) ); return NULL; }

    /*  Start acquiring VKL handle and managing VKL handle list */
    pVKLHandle = (NEXUS_VirtualKeyLadderHandle)BKNI_Malloc( sizeof( *pVKLHandle ) );
    if( pVKLHandle == NULL )
    {
       BHSM_FreeVKL( hHsm, allocateVKLIO.allocVKL );
       return NULL;
    }

    NEXUS_OBJECT_INIT(NEXUS_VirtualKeyLadder, pVKLHandle);

    pVKLHandle->client      = pVKLSettings->client;
    pVKLHandle->custSubMode = pVKLSettings->custSubMode;
    pVKLHandle->vkl         = allocateVKLIO.allocVKL;

    BDBG_LEAVE( NEXUS_Security_AllocateVKL );
    return pVKLHandle;
}

NEXUS_OBJECT_CLASS_MAKE( NEXUS_VirtualKeyLadder, NEXUS_Security_FreeVKL );


static void NEXUS_VirtualKeyLadder_P_Finalizer( NEXUS_VirtualKeyLadderHandle vklHandle )
{
    BHSM_Handle hHsm;
    BERR_Code hsmRc = BERR_SUCCESS;
    BHSM_InvalidateVkl_t invalidate;

    BDBG_ENTER( NEXUS_VirtualKeyLadder_P_Finalizer );

    NEXUS_OBJECT_ASSERT( NEXUS_VirtualKeyLadder, vklHandle );

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset( &invalidate, 0, sizeof(invalidate) );
    invalidate.bInvalidateVkl = true;           /* for Zeus4+*/
    invalidate.keyLayer = BCMD_KeyRamBuf_eKey3; /* For Zeus1/2/3 */
    invalidate.virtualKeyLadderID = vklHandle->vkl;

    hsmRc = BHSM_InvalidateVKL( hHsm, &invalidate );
    if( hsmRc != BERR_SUCCESS ){
        BDBG_ERR(("Failed to invalidate VKL"));  /* continue, best effort. */
    }

    BHSM_FreeVKL( hHsm, vklHandle->vkl );

    NEXUS_OBJECT_DESTROY( NEXUS_VirtualKeyLadder, vklHandle );
    BKNI_Free( vklHandle );

    BDBG_ENTER( NEXUS_VirtualKeyLadder_P_Finalizer );
    return;
}


void NEXUS_Security_GetVKLInfo(
    const NEXUS_VirtualKeyLadderHandle        vklHandle,
    NEXUS_VirtualKeyLadderInfo                *pVKLInfo
    )
{
    BDBG_ENTER(NEXUS_Security_GetVKLInfo);

    NEXUS_OBJECT_ASSERT( NEXUS_VirtualKeyLadder, vklHandle );

    if( !pVKLInfo ) { BERR_TRACE( NEXUS_INVALID_PARAMETER ); return; }

    BKNI_Memset( pVKLInfo, 0, sizeof(*pVKLInfo) );
    pVKLInfo->client      = vklHandle->client;
    pVKLInfo->custSubMode = vklHandle->custSubMode;
    pVKLInfo->vkl         = vklHandle->vkl;

    BDBG_LEAVE(NEXUS_Security_GetVKLInfo);
    return;
}



NEXUS_Error NEXUS_Security_GenerateNextLayerKey(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedControlWord * pCW
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCMD_KeyDestBlockType_e blockType = BCMD_KeyDestBlockType_eCA;
    BCMD_KeyDestEntryType_e entryType = BCMD_KeyDestEntryType_eOddKey;
    BCMD_KeyDestIVType_e    ivType    = BCMD_KeyDestIVType_eIV;
    BHSM_GenerateRouteKeyIO_t   generateRouteKeyIO;
    bool bDecrypt;
    BCMD_KeySize_e keySize;
    BCMD_KeyRamBuf_e keyLayer;
    BHSM_Handle hHsm;
    BCMD_RootKeySrc_e rootkeySrc = BCMD_RootKeySrc_eCusKey;
    BCMD_KeyLadderType_e    keyLadderType;
    NEXUS_SecurityKeyType keyEntryType;

    BDBG_ENTER(NEXUS_Security_GenerateNextLayerKey);

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm  || !pCW ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); };

    if( pCW->keySize==8 )       keySize = BCMD_KeySize_e64;
    else if( pCW->keySize==16 ) keySize = BCMD_KeySize_e128;
    else                        keySize = BCMD_KeySize_e192;

    keyEntryType = pCW->keyEntryType;

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(4,0)
    if( keyHandle->keyslotType == BCMD_XptSecKeySlot_eType3 )
    {
        /* Clear key entry must be used for Type 3, m2m  */
        keyEntryType = NEXUS_SecurityKeyType_eClear;
    }
#endif

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    if( pCW->keyLadderSelect == NEXUS_SecurityKeyLadderSelect_ePKL)
    {
        keyLayer = 10;
    }
    else
#endif
    {
        keyLayer = NEXUS_Security_P_mapNexus2Hsm_KeyLayer( pCW->keylayer );
    }

    NEXUS_Security_GetHsmDestBlkType (keyHandle, pCW->dest, &blockType);
    NEXUS_Security_GetHsmDestEntrykType( keyEntryType, &entryType );
    NEXUS_Security_GetHsmDestIVType(pCW->keyDestIVType, &ivType);
    bDecrypt = (pCW->operation==NEXUS_SecurityOperation_eDecrypt)? false: true;
    NEXUS_Security_GetKeyLadderType(pCW->keyladderType, &keyLadderType);
    NEXUS_Security_GetRootKeySource(pCW->rootKeySrc, &rootkeySrc);
    NEXUS_Security_InitGenerateRouteKey( &generateRouteKeyIO,
                keyLadderType,
                0,
                0,
                0, 0, 0, 0,
                keyLayer,
                keySize,
                pCW->bRouteKey,
                entryType,
                keyHandle->keySlotNumber,
                bDecrypt,
                bDecrypt,
                bDecrypt,
                blockType);


    generateRouteKeyIO.protectionKeyIvSource = pCW->protectionKeyIvSource;
    generateRouteKeyIO.keyTweak         = pCW->keyTweakOp; /* needed for tweaks */
    generateRouteKeyIO.rootKeySrc         = rootkeySrc;             /* need this for ASKM keyladder distinction */
    generateRouteKeyIO.virtualKeyLadderID = pCW->virtualKeyLadderID;
    generateRouteKeyIO.customerSubMode      = pCW->custSubMode;
    generateRouteKeyIO.keyMode              = pCW->keyMode;
    generateRouteKeyIO.ucKeyDataLen       = pCW->keySize;
    BKNI_Memset(generateRouteKeyIO.aucKeyData, 0,  pCW->keySize);
    if( pCW->keySize > sizeof(generateRouteKeyIO.aucKeyData) ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
    BKNI_Memcpy(generateRouteKeyIO.aucKeyData, pCW->keyData,  pCW->keySize);

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    generateRouteKeyIO.keyLadderSelect                = pCW->keyLadderSelect;
    generateRouteKeyIO.bASKM3DesKLRootKeySwapEnable = pCW->rootKeySwap;
    generateRouteKeyIO.GpipeSC01Val                 = g_scValues[pCW->sc01Polarity[NEXUS_SecurityPacketType_eGlobal]];
    generateRouteKeyIO.RpipeSC01Val                 = g_scValues[pCW->sc01Polarity[NEXUS_SecurityPacketType_eRestricted]];
    generateRouteKeyIO.keyGenMode                    = pCW->bkeyGenBlocked;

    /* For Hardware Key Ladder */
    generateRouteKeyIO.hwklLength                    = (BCMD_HwKeyLadderLength_e)pCW->hwklLen;
    generateRouteKeyIO.hwklDestAlg                    = NEXUS_Security_MapNexusAlgorithmToHsm(pCW->hwklDestAlg);
    BKNI_Memset(generateRouteKeyIO.activationCode, 0, NEXUS_SECURITY_KL_ACTCODE_SIZE);
    BKNI_Memcpy(generateRouteKeyIO.activationCode, pCW->actCode, NEXUS_SECURITY_KL_ACTCODE_SIZE);

#endif

    generateRouteKeyIO.unKeySlotNum     = keyHandle->keySlotNumber;
    generateRouteKeyIO.caKeySlotType    = keyHandle->keyslotType;
    generateRouteKeyIO.bASKMModeEnabled = pCW->bASKMMode;
    generateRouteKeyIO.bSwapAesKey      = pCW->bSwapAESKey;
    generateRouteKeyIO.keyDestIVType    = ivType;

#if BHSM_ZEUS_VERSION >= BHSM_ZEUS_VERSION_CALC(2,0)
    generateRouteKeyIO.subCmdID         = (BCMD_VKLAssociationQueryFlag_e)pCW->keyGenCmdID;
#else
    generateRouteKeyIO.subCmdID         = pCW->keyGenCmdID;
#endif
    generateRouteKeyIO.applyKeyContribution = pCW->applyKeyContribution;

    rc= BHSM_GenerateRouteKey (hHsm, &generateRouteKeyIO);
    if (rc != 0) {
        BDBG_ERR(("Generate next layer Key %d failed", pCW->keylayer));
        return NEXUS_INVALID_PARAMETER;
    }

    BDBG_LEAVE(NEXUS_Security_GenerateNextLayerKey);
    return NEXUS_SUCCESS;
}



NEXUS_Error NEXUS_Security_GenerateProcOut(
    NEXUS_KeySlotHandle                      keyHandle,
    const NEXUS_SecurityKLProcOutParm        *inProcOutParm,
    NEXUS_SecurityKLProcOutOutput            *outProcOut
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHSM_Handle hHsm;
    BHSM_ProcOutCmdIO_t procOutIO;

    BDBG_ENTER(NEXUS_Security_GenerateProcOut);

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm || !keyHandle || !inProcOutParm || !outProcOut ) {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

    BKNI_Memset( &procOutIO, 0, sizeof(procOutIO)  );
    procOutIO.virtualKeyLadderID = inProcOutParm->virtualKeyLadderID;
    procOutIO.unProcInLen        = inProcOutParm->procInLen;
    if( inProcOutParm->procInLen > sizeof(procOutIO.aucProcIn) ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
    BKNI_Memcpy( procOutIO.aucProcIn, inProcOutParm->procIn, inProcOutParm->procInLen );

    rc = BHSM_ProcOutCmd(hHsm, &procOutIO);
    if( rc ) { return BERR_TRACE( MAKE_HSM_ERR(rc)); }

    outProcOut->procOutLen = procOutIO.unProcOutLen;

    if( procOutIO.unProcOutLen > sizeof(outProcOut->procOut) ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
    BKNI_Memcpy( outProcOut->procOut, procOutIO.aucProcOut, procOutIO.unProcOutLen );

    BDBG_LEAVE(NEXUS_Security_GenerateProcOut);
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SecurityChallengeResponse(
    const NEXUS_SecurityChallengeResponseParm *crParm,
    NEXUS_SecurityChallengeResponseOutput *crOutput  /* [out] */
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BHSM_Handle hHsm;
    BHSM_KladChallengeIO_t kladChallengeIO;
    BHSM_KladResponseIO_t  kladResponseIO;

    BDBG_ENTER(NEXUS_SecurityChallengeResponse);

    NEXUS_Security_GetHsm_priv(&hHsm);
    if( !hHsm || !crParm || !crOutput ) { return  BERR_TRACE( NEXUS_INVALID_PARAMETER ); };

    /* Step 1: Get Challenge */
    BKNI_Memset(&kladChallengeIO, 0, sizeof(BHSM_KladChallengeIO_t));
    kladChallengeIO.keyId = crParm->otpKeyId;

    rc = BHSM_KladChallenge(hHsm,&kladChallengeIO);
    if( rc ) { return BERR_TRACE( MAKE_HSM_ERR(rc)); }

    /* Step 2: Send Response */
    BKNI_Memset(&kladResponseIO, 0, sizeof(BHSM_KladResponseIO_t));
    kladResponseIO.kladMode = crParm->kladMode;
    kladResponseIO.keyLayer = BCMD_KeyRamBuf_eKey3;
    kladResponseIO.virtualKeyLadderID = crParm->vkl;
    if( crParm->challengeSize > sizeof(kladResponseIO.aucNonce) ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
    BKNI_Memcpy( kladResponseIO.aucNonce, crParm->data, crParm->challengeSize );

    rc = BHSM_KladResponse(hHsm, &kladResponseIO);
    if( rc ) { return BERR_TRACE( MAKE_HSM_ERR(rc)); }

    crOutput->responseSize = BHSM_MAX_NONCE_DATA_LEN;
    if( crOutput->responseSize > sizeof(crOutput->data) ) { return BERR_TRACE( NEXUS_INVALID_PARAMETER ); }
    BKNI_Memcpy( crOutput->data, kladResponseIO.aucReponse, crOutput->responseSize );

    BDBG_LEAVE(NEXUS_SecurityChallengeResponse);
    return NEXUS_SUCCESS;
}
