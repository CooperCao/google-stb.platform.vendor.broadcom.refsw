/******************************************************************************
 *    (c)2007-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 *****************************************************************************/

#include "nexus_security_module.h"
#include "nexus_keyladder.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "priv/nexus_security_priv.h"
#include "priv/nexus_core.h"

#include "bhsm.h"
#include "bsp_s_commands.h"
#include "bsp_s_misc.h"
#include "bhsm_misc.h"
#include "bsp_s_hw.h"
#include "bsp_s_keycommon.h"
#include "bhsm_keyladder.h"
#include "bhsm_user_cmds.h"
#include "bsp_s_hmac_sha1.h"
#include "bhsm_keyladder_enc.h"

BDBG_MODULE(nexus_security);



struct NEXUS_UserCmdImpl
{
 NEXUS_UserCmdData userCmdData;
}NEXUS_UserCmdImpl;

static void NEXUS_Security_GetHsmDestBlkType(
    NEXUS_KeySlotHandle keyHandle,
    BCMD_KeyDestBlockType_e * pType
    )
{
    switch ( keyHandle->settings.keySlotEngine)
    {
    case NEXUS_SecurityEngine_eCa:
        * pType = BCMD_KeyDestBlockType_eCA;
        break;
    case NEXUS_SecurityEngine_eM2m:
        * pType = BCMD_KeyDestBlockType_eMem2Mem;
        break;
    case NEXUS_SecurityEngine_eCp:
    case NEXUS_SecurityEngine_eRmx:
    default:
        * pType = BCMD_KeyDestBlockType_eRmx;
        break;
    }
}

static void NEXUS_Security_GetHsmDestEntrykType(
    NEXUS_SecurityKeyType keytype,
    BCMD_KeyDestEntryType_e * pType
    )
{
    switch ( keytype )
    {
    case NEXUS_SecurityKeyType_eEven:
        * pType = BCMD_KeyDestEntryType_eEvenKey;
        break;
    case NEXUS_SecurityKeyType_eOdd:
        * pType = BCMD_KeyDestEntryType_eOddKey;
        break;
    case NEXUS_SecurityKeyType_eClear:
        * pType = BCMD_KeyDestEntryType_eReserved0;
        break;
    case NEXUS_SecurityKeyType_eIv:
        * pType = BCMD_KeyDestEntryType_eIV;
        break;
    default:
        * pType = BCMD_KeyDestEntryType_eOddKey;
        break;
    }
}

static void NEXUS_Security_InitGenerateRouteKey(
    BHSM_GenerateRouteKeyIO_t   * generateRouteKeyPtr,
    BCMD_KeyLadderType_e klType,    BCMD_RootKeySrc_e rootKsrc,
    BCMD_SwizzleType_e swizzleType, unsigned char kLow, unsigned char kLowV, unsigned char kHigh, unsigned char kHighV,
    BCMD_KeyRamBuf_e kSrc, BCMD_KeyRamBuf_e kDest,
    BCMD_KeySize_e kSize, bool routeFlag, BCMD_KeyDestEntryType_e oddEvenEntry, unsigned int slotNum,
    bool useCustLow,
        bool useCustHigh,
        bool operateCrption,
        BCMD_KeyDestBlockType_e blockType
    )
{
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

    } else{}
    generateRouteKeyPtr->bUseCustKeyLowDecrypt =  useCustLow;
    generateRouteKeyPtr->bUseCustKeyHighDecrypt = useCustHigh;

#if ((BCHP_CHIP==7420) && (BCHP_VER >= BCHP_VER_A1)) || (BCHP_CHIP == 7340) || \
    (BCHP_CHIP == 7342) || (BCHP_CHIP == 7125) || (BCHP_CHIP == 7468)
	generateRouteKeyPtr->keyLayer           = kDest;
    kSrc = kSrc;
#else
    generateRouteKeyPtr->keySrc= kSrc;
    generateRouteKeyPtr->keyDest= kDest ;
#endif
    generateRouteKeyPtr->bIs3DESDecrypt= operateCrption;

    /*BKNI_Memcpy(generateRouteKeyPtr->aucKeyData, aucKeyData[0], 24); */

    generateRouteKeyPtr->keySize= kSize;

    generateRouteKeyPtr->bIsRouteKeyRequired = routeFlag;
    generateRouteKeyPtr->keyDestBlckType = blockType;       /*fixed */
    generateRouteKeyPtr->keyDestEntryType = oddEvenEntry;
    generateRouteKeyPtr->caKeySlotType = 0;                  /*no matter for m2m */
    generateRouteKeyPtr->unKeySlotNum =  slotNum;

}

#if (BCHP_CHIP != 7420) && (BCHP_CHIP != 7340) && (BCHP_CHIP != 7342) && \
    (BCHP_CHIP != 7125) && (BCHP_CHIP != 7468) 
static void NEXUS_Security_GetHsmKeyladderTypeID(
    NEXUS_SecurityKeyladderID keyladderID,
    BCMD_KeyRamBuf_e * pKeyladderID
    )
{
    switch (keyladderID)
    {
    case NEXUS_SecurityKeyladderID_eA:
        * pKeyladderID = BCMD_KeyRamBuf_eKey3KeyLadder1;
        break;
    case NEXUS_SecurityKeyladderID_eB:
        * pKeyladderID = BCMD_KeyRamBuf_eKey3KeyLadder2;
        break;
    case NEXUS_SecurityKeyladderID_eC:
    default:
        * pKeyladderID = BCMD_KeyRamBuf_eKey3KeyLadder3;
        break;
    }
}

#endif

NEXUS_Error NEXUS_Security_GenerateSessionKey(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedSessionKey *pSessionKey
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCMD_KeyDestBlockType_e blockType = BCMD_KeyDestBlockType_eRmx;
    BCMD_KeyDestEntryType_e entryType = BCMD_KeyDestEntryType_eOddKey;
    BHSM_GenerateRouteKeyIO_t   generateRouteKeyIO;
    bool bDecrypt;
    bool bKey2Decrypt;
    BCMD_RootKeySrc_e rootkeySrc = BCMD_RootKeySrc_eCusKey;
    BCMD_KeyRamBuf_e keyladderID;
    BHSM_Handle hHsm;

    NEXUS_Security_GetHsm_priv(&hHsm);
    if (!hHsm) return NEXUS_INVALID_PARAMETER;

#if ((BCHP_CHIP==7420) && (BCHP_VER >= BCHP_VER_A1)) || (BCHP_CHIP == 7340) || \
    (BCHP_CHIP == 7342) || (BCHP_CHIP == 7125) || (BCHP_CHIP == 7468)
	keyladderID = BCMD_KeyRamBuf_eKey3;
#else
	encryptedSessionkey.keyladderID 	= NEXUS_SecurityKeyladderID_eA;
    NEXUS_Security_GetHsmKeyladderTypeID  ( pSessionKey->keyladderID, &keyladderID);
#endif

    switch (pSessionKey->rootKeySrc)
    {
    case NEXUS_SecurityRootKeySrc_eOtpKeyA:
        rootkeySrc = BCMD_RootKeySrc_eOTPKeya;
        break;
    case NEXUS_SecurityRootKeySrc_eOtpKeyB:
        rootkeySrc = BCMD_RootKeySrc_eOTPKeyb;
        break;
    case NEXUS_SecurityRootKeySrc_eOtpKeyC:
        rootkeySrc = BCMD_RootKeySrc_eOTPKeyc;
        break;
#if ((BCHP_CHIP==7420) && (BCHP_VER >= BCHP_VER_A1)) || (BCHP_CHIP == 7340) || \
			(BCHP_CHIP == 7342) || (BCHP_CHIP == 7125) || (BCHP_CHIP == 7468)
	case NEXUS_SecurityRootKeySrc_eAskmSel:
		rootkeySrc = BCMD_RootKeySrc_eASKMSel;
		break;
#endif
    case NEXUS_SecurityRootKeySrc_eCuskey:
    default:
        rootkeySrc = BCMD_RootKeySrc_eCusKey;
        break;
    }

    NEXUS_Security_GetHsmDestBlkType (keyHandle, &blockType);
    NEXUS_Security_GetHsmDestEntrykType(pSessionKey->keyEntryType, &entryType);
    bDecrypt = (pSessionKey->operation==NEXUS_SecurityOperation_eDecrypt)? false: true;
    bKey2Decrypt = (pSessionKey->operationKey2==NEXUS_SecurityOperation_eDecrypt)? true: false;

    NEXUS_Security_InitGenerateRouteKey( &generateRouteKeyIO,
#if (BCHP_CHIP != 3563) && (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556)
                (pSessionKey->keyladderType==NEXUS_SecurityKeyladderType_e3Des)?BCMD_KeyLadderType_e3DESABA:BCMD_KeyLadderType_eAES128,
#else
                BCMD_KeyLadderType_e3DESABA,
#endif 
                rootkeySrc,
                (pSessionKey->swizzleType==NEXUS_SecuritySwizzleType_eNone)?BCMD_SwizzleType_eNoSwizzle: BCMD_SwizzleType_eSwizzle0,
                pSessionKey->cusKeyL, pSessionKey->cusKeyVarL, pSessionKey->cusKeyH, pSessionKey->cusKeyVarH,
#if 0
                0,   /*no matter, e.g.BCMD_KeyRamBuf_eKey3KeyLadder1 */
#else
                keyladderID,
#endif
                keyladderID,
                BCMD_KeySize_e128,
                pSessionKey->bRouteKey,
                entryType,
                keyHandle->keySlotNumber,
                bKey2Decrypt,
                bKey2Decrypt,
                bDecrypt,
                blockType);

#if ((BCHP_CHIP==7420) && (BCHP_VER >= BCHP_VER_A1)) || (BCHP_CHIP == 7340) || \
    (BCHP_CHIP == 7342) || (BCHP_CHIP == 7125) || (BCHP_CHIP == 7468)
	generateRouteKeyIO.virtualKeyLadderID = pSessionKey->virtualKeyLadderID;
	generateRouteKeyIO.customerSubMode    = pSessionKey->custSubMode;
	generateRouteKeyIO.keyMode            = pSessionKey->keyMode;
#else
	generateRouteKeyIO.keyMode            = BCMD_KeyMode_eRegular;	/* always for non-DVB, change for DVB*/
#endif


    generateRouteKeyIO.ucKeyDataLen = 16;
    BKNI_Memset(generateRouteKeyIO.aucKeyData, 0, 16);
    BKNI_Memcpy(generateRouteKeyIO.aucKeyData, pSessionKey->keyData, 16);

    generateRouteKeyIO.unKeySlotNum = keyHandle->keySlotNumber;
    generateRouteKeyIO.caKeySlotType = keyHandle->keyslotType;
    rc= BHSM_GenerateRouteKey (hHsm, &generateRouteKeyIO);
    if (rc != 0) {
        BDBG_ERR(("Generate route Key 3 failed\n"));
        return NEXUS_INVALID_PARAMETER;
    }

    return NEXUS_SUCCESS;
}


static NEXUS_Error NEXUS_Security_GenerateKey4or5(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedControlWord * pCW,
    bool bKey4
    )
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    BCMD_KeyDestBlockType_e blockType = BCMD_KeyDestBlockType_eRmx;
    BCMD_KeyDestEntryType_e entryType = BCMD_KeyDestEntryType_eOddKey;
    BHSM_GenerateRouteKeyIO_t   generateRouteKeyIO;
    bool bDecrypt;
    BCMD_KeySize_e keySize;
    BCMD_KeyRamBuf_e keyladderID;
    BHSM_Handle hHsm;
#if ((BCHP_CHIP==7420) && (BCHP_VER >= BCHP_VER_A1)) || (BCHP_CHIP == 7340) || \
		(BCHP_CHIP == 7342) || (BCHP_CHIP == 7125) || (BCHP_CHIP == 7468)
    BCMD_RootKeySrc_e rootkeySrc = BCMD_RootKeySrc_eCusKey;
#endif

    NEXUS_Security_GetHsm_priv(&hHsm);
    if (!hHsm) return NEXUS_INVALID_PARAMETER;

    if ( pCW->keySize==8 )
        keySize = BCMD_KeySize_e64;
    else if ( pCW->keySize==16 )
        keySize = BCMD_KeySize_e128;
    else
        keySize = BCMD_KeySize_e192;

#if ((BCHP_CHIP==7420) && (BCHP_VER >= BCHP_VER_A1)) || (BCHP_CHIP == 7340) || \
    (BCHP_CHIP == 7342) || (BCHP_CHIP == 7125) || (BCHP_CHIP == 7468)
	keyladderID = BCMD_KeyRamBuf_eKey3;
    switch (pCW->rootKeySrc)
    {
    	case NEXUS_SecurityRootKeySrc_eOtpKeyA:
        	rootkeySrc = BCMD_RootKeySrc_eOTPKeya;
        	break;
    	case NEXUS_SecurityRootKeySrc_eOtpKeyB:
        	rootkeySrc = BCMD_RootKeySrc_eOTPKeyb;
        	break;
    	case NEXUS_SecurityRootKeySrc_eOtpKeyC:
        	rootkeySrc = BCMD_RootKeySrc_eOTPKeyc;
        	break;
		case NEXUS_SecurityRootKeySrc_eAskmSel:
			rootkeySrc = BCMD_RootKeySrc_eASKMSel;
			break;
	    case NEXUS_SecurityRootKeySrc_eCuskey:
    	default:
        	rootkeySrc = BCMD_RootKeySrc_eCusKey;
        	break;
    }

#else
	NEXUS_Security_GetHsmKeyladderTypeID  ( pCW->keyladderID, &keyladderID);
#endif

    if ( !bKey4 )
        keyladderID ++;

    NEXUS_Security_GetHsmDestBlkType (keyHandle, &blockType);
    NEXUS_Security_GetHsmDestEntrykType(pCW->keyEntryType, &entryType);
    bDecrypt = (pCW->operation==NEXUS_SecurityOperation_eDecrypt)? false: true;
    NEXUS_Security_InitGenerateRouteKey( &generateRouteKeyIO,
#if (BCHP_CHIP != 3563) && (BCHP_CHIP != 3548) && (BCHP_CHIP != 3556)
                (pCW->keyladderType==NEXUS_SecurityKeyladderType_e3Des)?BCMD_KeyLadderType_e3DESABA:BCMD_KeyLadderType_eAES128,
#else
		BCMD_KeyLadderType_e3DESABA,
#endif
                0,
                0,
                0, 0, 0, 0,
                keyladderID,
                keyladderID+1,
                keySize,
                pCW->bRouteKey,
                entryType,
                keyHandle->keySlotNumber,
                bDecrypt,
                bDecrypt,
                bDecrypt,
                blockType);

#if ((BCHP_CHIP==7420) && (BCHP_VER >= BCHP_VER_A1)) || (BCHP_CHIP == 7340) || \
    (BCHP_CHIP == 7342) || (BCHP_CHIP == 7125) || (BCHP_CHIP == 7468)
	generateRouteKeyIO.rootKeySrc         = rootkeySrc;             /* need this for ASKM keyladder distinction */
	generateRouteKeyIO.virtualKeyLadderID = pCW->virtualKeyLadderID;
	generateRouteKeyIO.customerSubMode	  = pCW->custSubMode;
	generateRouteKeyIO.keyMode			  = pCW->keyMode;
#else
	generateRouteKeyIO.keyMode			  = BCMD_KeyMode_eRegular;	/* always for non-DVB, change for DVB*/
#endif

    generateRouteKeyIO.ucKeyDataLen = pCW->keySize;
    BKNI_Memset(generateRouteKeyIO.aucKeyData, 0,  pCW->keySize);
    BKNI_Memcpy(generateRouteKeyIO.aucKeyData, pCW->keyData,  pCW->keySize);

    generateRouteKeyIO.unKeySlotNum = keyHandle->keySlotNumber;
    generateRouteKeyIO.caKeySlotType = keyHandle->keyslotType;
    rc= BHSM_GenerateRouteKey (hHsm, &generateRouteKeyIO);
    if (rc != 0) {
        BDBG_ERR(("Generate route Key 4 or 5 failed\n"));
        return NEXUS_INVALID_PARAMETER;
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_Security_GenerateControlWord(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedControlWord * pCW
    )
{
    return NEXUS_Security_GenerateKey4or5 (keyHandle, pCW, true);
}

NEXUS_Error NEXUS_Security_GenerateKey5(
    NEXUS_KeySlotHandle keyHandle,
    const NEXUS_SecurityEncryptedControlWord * pCW
    )
{
    return NEXUS_Security_GenerateKey4or5 (keyHandle, pCW, false);
}

void NEXUS_Security_GetUserCmdSettings(
    NEXUS_UserCmdHandle userCmdhandle,
    NEXUS_UserCmdData *pUserData /* [out] */
    ){

	unsigned int i;
		for (i = 0; i < NEXUS_SHA1_DIGEST_SIZE; i++)
		{
			pUserData->hmacSha1Buf[i] = userCmdhandle->userCmdData.hmacSha1Buf[i];
		}
	
	return ;
}
 

NEXUS_Error NEXUS_Security_SetUserCmdSettings(
    NEXUS_UserCmdHandle userCmdHandle,
    const NEXUS_UserCmdData *pData
    ){
 
		unsigned int i;
		for (i = 0; i < NEXUS_SHA1_DIGEST_SIZE; i++)
		{
			userCmdHandle->userCmdData.hmacSha1Buf[i] = pData->hmacSha1Buf[i]  ;
		}
	
	return BERR_SUCCESS;
}

NEXUS_UserCmdHandle NEXUS_Security_CreateUserCmd(void){
	NEXUS_UserCmdHandle  userCmdHandle;
	NEXUS_UserCmdData *userCmdData;
	
	NEXUS_Memory_Allocate(sizeof(NEXUS_UserCmdImpl), NULL, (void *)&userCmdHandle);
	NEXUS_Memory_Allocate(sizeof(NEXUS_UserCmdData), NULL, (void *)&userCmdData);
	BKNI_Memset(userCmdData, 0, sizeof(NEXUS_UserCmdData));
	NEXUS_Security_SetUserCmdSettings(userCmdHandle, userCmdData);

	return userCmdHandle; 
}
    
void NEXUS_Security_DestroyUserCmd(
    NEXUS_UserCmdHandle userCmd
    ){

	NEXUS_Memory_Free(userCmd);
	  
	return;
}
 

NEXUS_Error NEXUS_Security_SetVich (
	void
	)
{

#if defined(BHSM_AUTO_TEST) && defined(BHSM_SETVICH_PATCH)

    NEXUS_Error rc = NEXUS_SUCCESS;
	BHSM_Handle hHsm;
	BERR_Code	errCode;

	/* Only raw command can be used due to auto-increment of sequence number */
	static const unsigned int incmdBuffer[NEXUS_SECURITY_SETVICH_CMD_IN_BUFFER_SIZE] =
	{
		0x00000010,
		0x00000088,
		0xABCDEF00,
		0xD455AA2B,		/* SESSION_SET_VICH_REG_PAR */
		0x789A0038,
		0x00000009,
		0x10800C00,		/* DECODE_SINT_0_REG_SINT_DMA_ADDR */
		0x10800C0F,		/* DECODE_SINT_0_REG_SINT_DMA_END +3 */
		0x10801800,		/* DECODE_CPUDMA_0_REG_DMA0_SD_ADDR */
		0x10801A03,		/* DECODE_DMAMEM_0_DMA_MEM+3 */
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,

#if 1
		/* Signature: ae ad eb 4d cb 54 0b 34 9e 0d c3 25 57 c8 8f 2f 91 78 80 99 */
		0xaeadeb4d,
		0xcb540b34,
		0x9e0dc325,
		0x57c88f2f,
		0x91788099,
#else
		/* f7 b6 65 e6 f4 86 62 1a 2a 2a 86 2e bd 60 e4 b7 26 61 02 3d */
		0x600a2287,
		0xd9e39b85,
		0x999bf1d2,
		0xee9ea296,
		0x7123d745,
#endif

	};

	unsigned int 	outCmdLength = 0;
	unsigned int 	outCmdBuffer[NEXUS_SECURITY_SETVICH_CMD_OUT_BUFFER_SIZE];
	static bool		bSetVichCalled = false;

	if ( bSetVichCalled )
		return NEXUS_SUCCESS;

	NEXUS_Security_GetHsm_priv (&hHsm);
    if (!hHsm) 
	{
		BDBG_ERR(("Error calling NEXUS_Security_GetHsm_priv()\n"));	
		return NEXUS_INVALID_PARAMETER;
    }
	
	BKNI_Memset(outCmdBuffer, 0, sizeof(unsigned int)*NEXUS_SECURITY_SETVICH_CMD_OUT_BUFFER_SIZE);
	errCode = BHSM_SubmitRawCommand (hHsm, 1, NEXUS_SECURITY_SETVICH_CMD_IN_BUFFER_SIZE, 
									(unsigned int *)incmdBuffer, &outCmdLength, outCmdBuffer);
	/* Error code shall be ignored */
	if ( errCode == BERR_SUCCESS )
	{
		if ( outCmdBuffer [5]== 0 )
		{
			if ( outCmdBuffer [6]== 0 )
			{
				BDBG_MSG (("SetVich successful\n"));
			}
			else if (outCmdBuffer [6]== 0xab )
			{
				BDBG_MSG (("SetVich signature failure\n"));
			}
		}
		else
		{
			BDBG_MSG (("SetVich might have been called already\n"));
		}
	}
	else
	{
		BDBG_MSG (("Unknown error for SetVich\n"));
	}

	bSetVichCalled = true;
	return NEXUS_SUCCESS;
#else
	return NEXUS_SUCCESS;
#endif


}
