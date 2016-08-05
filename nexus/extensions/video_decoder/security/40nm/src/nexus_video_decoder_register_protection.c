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

#include "nexus_video_decoder_module.h"
#include "nexus_video_decoder_register_protection.h"
#include "nexus_video_decoder_security.h"
#include "nexus_bsp_config.h"
#include "nexus_keyladder.h"

#include "nexus_video_decoder_register_protection_settings.c"
#if BCHP_CHIP == 7231
#include "nexus_video_decoder_register_protection_signatures_7231.c"
#elif BCHP_CHIP == 7425
#include "nexus_video_decoder_register_protection_signatures_7425.c"
#elif BCHP_CHIP == 7228
#include "nexus_video_decoder_register_protection_signatures_7228.c"
#elif BCHP_CHIP == 7346
#include "nexus_video_decoder_register_protection_signatures_7346.c"
#elif BCHP_CHIP == 7358
#include "nexus_video_decoder_register_protection_signatures_7358.c"
#elif BCHP_CHIP == 7429
#include "nexus_video_decoder_register_protection_signatures_7429.c"
#elif (BCHP_CHIP == 7445)||(BCHP_CHIP==7252)
#include "nexus_video_decoder_register_protection_signatures_7445.c"
#elif BCHP_CHIP == 7250
#include "nexus_video_decoder_register_protection_signatures_7250.c"

#else
#include "nexus_video_decoder_register_protection_signatures.c"
#endif

#if (NEXUS_NUM_XVD_DEVICES)

BDBG_MODULE(nexus_decoder_security);
BDBG_OBJECT_ID(NEXUS_VideoDecoderSecurityRegisterProtection);

static NEXUS_VirtualKeyLadderHandle SecurityAllocateVkl ( NEXUS_SecurityVirtualKeyladderID * vkl )
{
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderHandle vklHandle;
    NEXUS_VirtualKeyLadderInfo vklInfo;

    BDBG_ASSERT ( vkl );

    NEXUS_Security_GetDefaultVKLSettings ( &vklSettings );

	/* For pre Zeus 3.0, set vklSettings.custSubMode */
	vklSettings.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneralPurpose1;

    vklHandle = NEXUS_Security_AllocateVKL ( &vklSettings );

    if ( !vklHandle )
    {
        BDBG_WRN (( "Allocate VKL failed. " ));
        return 0;
    }

    NEXUS_Security_GetVKLInfo ( vklHandle, &vklInfo );
    BDBG_MSG(( "VKL Handle %p is allocated for VKL#%d\n", ( void * ) vklHandle, vklInfo.vkl & 0x7F ));

    *vkl = vklInfo.vkl;

    return vklHandle;
}

static BERR_Code GenerateKey3ForSignatureVerification ( NEXUS_SecurityVirtualKeyladderID vklId )
{
    BERR_Code rc = NEXUS_SUCCESS;
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;

    static const unsigned char ucProcInForKey3[16] =
    {
        0x11, 0x11, 0x22, 0x22, 0x12, 0x34, 0x56, 0x78,
        0x98, 0x76, 0x54, 0x32, 0x09, 0x23, 0x45, 0x56,
    };

    NEXUS_KeySlotHandle keyHandle = NULL;
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_SecurityAlgorithmSettings AlgConfig;

    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
    /* Allocate AV keyslots */
    keyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if ( !keyHandle)
    {
        rc = 1;
        goto error;
    }

    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eDvb;
    AlgConfig.dvbScramLevel = NEXUS_SecurityDvbScrambleLevel_eTs;

    AlgConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
    AlgConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
    AlgConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;

#if NEXUS_SECURITY_ZEUS_VERSION_MAJOR < 4
    AlgConfig.caVendorID          = 0x1234;
    AlgConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_3;
    AlgConfig.otpId               = NEXUS_SecurityOtpId_eOtpVal;
#else
    AlgConfig.caVendorID          = 0x8176 /*0x8d92*/;
    AlgConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_15;
    AlgConfig.otpId               = NEXUS_SecurityOtpId_eOneVal;
#endif
    AlgConfig.key2Select      = NEXUS_SecurityKey2Select_eFixedKey;/*1*/

    if ( NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0 )
    {
        return 1;
    }

    BKNI_Memset (&encryptedSessionkey, 0, sizeof(NEXUS_SecurityEncryptedSessionKey));
    encryptedSessionkey.keyladderID     = NEXUS_SecurityKeyladderID_eB;
    encryptedSessionkey.keyladderType     = NEXUS_SecurityKeyladderType_e3Des;
#if NEXUS_SECURITY_ZEUS_VERSION_MAJOR < 4
    encryptedSessionkey.swizzleType        = NEXUS_SecuritySwizzleType_eSwizzle0;
    encryptedSessionkey.rootKeySrc         = NEXUS_SecurityRootKeySrc_eCuskey;
    encryptedSessionkey.operation         = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.operationKey2     = NEXUS_SecurityOperation_eEncrypt;
#else
    encryptedSessionkey.swizzleType        = NEXUS_SecuritySwizzleType_eNone;
    encryptedSessionkey.rootKeySrc         = NEXUS_SecurityRootKeySrc_eGlobalKey;
    encryptedSessionkey.askmGlobalKeyIndex = 0x3E;
    encryptedSessionkey.globalKeyOwnerId = NEXUS_SecurityGlobalKeyOwnerID_eUse1;
    encryptedSessionkey.globalKeyVersion = 0;
    encryptedSessionkey.operation         = NEXUS_SecurityOperation_eDecrypt;
    encryptedSessionkey.operationKey2     = NEXUS_SecurityOperation_eDecrypt;
    encryptedSessionkey.bASKMMode       = true;
#endif
    encryptedSessionkey.cusKeyL         = 0x0a;
    encryptedSessionkey.cusKeyH         = 0x0a;
    encryptedSessionkey.cusKeyVarL         = 0x27;
    encryptedSessionkey.cusKeyVarH         = 0x27;
    encryptedSessionkey.bRouteKey         = false;
    encryptedSessionkey.keyEntryType     = NEXUS_SecurityKeyType_eOdd;

    encryptedSessionkey.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneralPurpose1;
    encryptedSessionkey.virtualKeyLadderID = vklId;
    encryptedSessionkey.keyMode            = NEXUS_SecurityKeyMode_eRegular;
    encryptedSessionkey.cusKeySwizzle0aType  = NEXUS_SecuritySwizzle0aType_eDisabled;

    BKNI_Memcpy(encryptedSessionkey.keyData, ucProcInForKey3, sizeof(ucProcInForKey3));

    rc=NEXUS_Security_GenerateSessionKey(keyHandle, &encryptedSessionkey);

error:
    NEXUS_Security_FreeKeySlot (keyHandle);

    return rc;
}

BERR_Code NEXUS_VideoDecoder_P_EnableRegisterProtection (
    const NEXUS_VideoDecoderRegisterProtectionConfig *pConfig
    )
{
    unsigned int index, i;
    BERR_Code rc = NEXUS_SUCCESS;
    NEXUS_SecurityAVDSRegRangeSettings    avdRegRangeSettings;
    AVDRegRegion * pRegInfo;
    NEXUS_SecurityVirtualKeyladderID vklId;
    NEXUS_VirtualKeyLadderHandle vklHandle;

    BDBG_ENTER(NEXUS_VideoDecoder_P_EnableRegisterProtection);
    BDBG_ASSERT(pConfig);

    index = pConfig->index;
    BDBG_ASSERT(index<=NEXUS_NUM_XVD_DEVICES);

	/* Request for an VKL to use */
	vklHandle = SecurityAllocateVkl ( &vklId );
	if (!vklHandle)
	{
		BDBG_WRN (( "Allocate VKL failed."));
		return BERR_TRACE(NEXUS_NOT_AVAILABLE);
	}

    if ( (rc = GenerateKey3ForSignatureVerification ( vklId )) != NEXUS_SUCCESS )
    {
        BDBG_WRN(("Cannot generate key3c"));
        BERR_TRACE(rc);
        goto exit;
    }

    avdRegRangeSettings.vkl =  vklId;
    avdRegRangeSettings.keyLayer = NEXUS_SecurityKeySource_eKey3;

    if( index == 0 )
    {
        #ifndef NEXUS_VIDEO_DECODER_SECURITY_NO_SVD
        avdRegRangeSettings.VDECId = NEXUS_SecurityAVDType_eSVD;
        #else
        avdRegRangeSettings.VDECId = NEXUS_SecurityAVDType_eAVD;
        #endif
        avdRegRangeSettings.nRange = sizeof(gAVD0_Regions)/sizeof(AVDRegRegion);
        pRegInfo = (AVDRegRegion*)gAVD0_Regions;
    }

    #if (NEXUS_NUM_XVD_DEVICES>1)
    else if( index == 1 )
    {
        avdRegRangeSettings.VDECId = NEXUS_SecurityAVDType_eVDEC1;
        avdRegRangeSettings.nRange = sizeof(gAVD1_Regions)/sizeof(AVDRegRegion);
        pRegInfo = (AVDRegRegion*)gAVD1_Regions;
    }
    #if (NEXUS_NUM_XVD_DEVICES>2)
    else if( index == 2 )
    {
        avdRegRangeSettings.VDECId = NEXUS_SecurityAVDType_eVDEC2;
        avdRegRangeSettings.nRange = sizeof(gAVD2_Regions)/sizeof(AVDRegRegion);
        pRegInfo = (AVDRegRegion*)gAVD2_Regions;
    }
    else
    {
        /* We should never reach here.*/
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto exit;
    }
    #else /* (NEXUS_NUM_XVD_DEVICES>2) */
    else
    {
        /* We should never reach here. */
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto exit;
    }
    #endif /*(NEXUS_NUM_XVD_DEVICES>2) */
    #endif /* (NEXUS_NUM_XVD_DEVICES>1) */

    BDBG_ASSERT( avdRegRangeSettings.nRange <= NEXUS_MAX_AVDSVD_RANGE );

    for ( i=0; i< avdRegRangeSettings.nRange; i++ )
    {
        avdRegRangeSettings.loRange[i] = pRegInfo[i].startPhysicalAddress;
        avdRegRangeSettings.hiRange[i] = pRegInfo[i].endPhysicalAddress;
    }

    for (i=0;i<NEXUS_HMACSHA256_SIGNATURE_SIZE;i++)
    {
        avdRegRangeSettings.signature[i] = gAVD_SetVichSignature[index][i];
    }

    if ( (rc = NEXUS_Security_AVDSRegistersSetUp(&avdRegRangeSettings))!= NEXUS_SUCCESS )
    {
        BDBG_WRN(("Cannot SetVICH %08x\n", rc));
        BERR_TRACE(rc);
        goto exit;
    }

exit:
    NEXUS_Security_FreeVKL(vklHandle);
    return rc;
}

BERR_Code NEXUS_VideoDecoder_P_AVDResetCallback( void * pPrivateData )
{
    NEXUS_VideoDecoderSecureBootContext * pContext = (NEXUS_VideoDecoderSecureBootContext *)pPrivateData;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ENTER(AVDResetCallback);
    BDBG_ASSERT( pPrivateData );

    BDBG_MSG(("\n$$$$$$$$ DeviceID is %d $$$$$$$$\n", pContext->deviceID));

    if ( pContext->deviceID == 0 )
    {
        NEXUS_VideoDecoderRegisterProtectionConfig config;
        config.index = pContext->deviceID;
        rc = NEXUS_VideoDecoder_P_SecureReset (&config);
        if ( rc != BERR_SUCCESS)
        {
            BDBG_ERR(("##### Cannot secure reset AVD0 #####\n"));
            /* Stop IL AVD ARC */
           #if NEXUS_SECURITY_ZEUS_VERSION_MAJOR < 4
            BREG_Write32(pContext->hReg, BCHP_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG, 1);
            BREG_Write32(pContext->hReg, BCHP_DECODE_CPUAUX_0_CPUAUX_REG+0x28, 1);
            BREG_Write32(pContext->hReg, BCHP_DECODE_IND_SDRAM_REGS2_0_REG_CPU_DBG, 1);  /* Stop OL AVD ARC */
            BREG_Write32(pContext->hReg, BCHP_DECODE_CPUAUX2_0_CPUAUX_REG+0x28, 1);

            /* Stop BL SVD ARC */
            #ifndef NEXUS_VIDEO_DECODER_SECURITY_NO_BLD
            BREG_Write32( pContext->hReg, BCHP_BLD_DECODE_IND_SDRAM_REGS_0_REG_CPU_DBG, 1);
            BREG_Write32( pContext->hReg, BCHP_BLD_DECODE_CPUAUX_0_CPUAUX_REG+0x28, 1);
            #endif
           #endif
        }

    }

    #if (NEXUS_NUM_XVD_DEVICES==2)
    else if ( pContext->deviceID == 1 )
    {
        NEXUS_VideoDecoderRegisterProtectionConfig config;
        config.index = pContext->deviceID;
        rc = NEXUS_VideoDecoder_P_SecureReset (&config);

        #if NEXUS_SECURITY_ZEUS_VERSION_MAJOR < 4
        if ( rc != BERR_SUCCESS)
        {
            BDBG_ERR(("##### Cannot secure reset AVD1 #####\n"));
            /* Stop IL AVD ARC */
            BREG_Write32(pContext->hReg, BCHP_DECODE_IND_SDRAM_REGS_1_REG_CPU_DBG, 1); /* Stop IL AVD ARC */
            BREG_Write32(pContext->hReg, BCHP_DECODE_CPUAUX_1_CPUAUX_REG+0x28, 1);
        }
        /* Stop OL AVD ARC */
        BREG_Write32(pContext->hReg, BCHP_DECODE_IND_SDRAM_REGS2_1_REG_CPU_DBG, 1);
        BREG_Write32(pContext->hReg, BCHP_DECODE_CPUAUX2_1_CPUAUX_REG+0x28, 1);
        #endif
    }
    #endif

    BDBG_LEAVE(AVDResetCallback);
    return BERR_SUCCESS;
}


static BERR_Code NEXUS_VideoDecoder_P_SecureModify(
    const NEXUS_VideoDecoderRegisterProtectionConfig *pConfig,
    bool bReset
    )
{
    unsigned int i;
    BERR_Code rc = NEXUS_SUCCESS;
    NEXUS_SecurityAVDSRegModifySettings avdModifySettings;
    AVDRegAddrValue *pRegAddrValue;
    unsigned char *pSignature;
    NEXUS_SecurityVirtualKeyladderID vklId;
    NEXUS_VirtualKeyLadderHandle vklHandle;

    BDBG_ENTER(NEXUS_VideoDecoder_P_SecureStart);
    BDBG_ASSERT(pConfig);

    BDBG_ASSERT(pConfig->index<=NEXUS_NUM_XVD_DEVICES);

	/* Request for an VKL to use */
	vklHandle = SecurityAllocateVkl ( &vklId );
	if (!vklHandle)
	{
		BDBG_WRN (( "Allocate VKL failed."));
		return BERR_TRACE(NEXUS_NOT_AVAILABLE);
	}

    if ( (rc = GenerateKey3ForSignatureVerification ( vklId )) != NEXUS_SUCCESS )
    {
        BDBG_WRN(("Cannot generate key3c"));
        BERR_TRACE(rc);
        goto exit;
    }

    avdModifySettings.vkl = vklId;
    avdModifySettings.keyLayer = NEXUS_SecurityKeySource_eKey3;

    #ifndef NEXUS_VIDEO_DECODER_SECURITY_NO_SVD

    switch (pConfig->index)
    {
        case 0:
            avdModifySettings.avdID = NEXUS_SecurityAVDType_eSVD;
            break;
        case 1:
            avdModifySettings.avdID = NEXUS_SecurityAVDType_eVDEC1;
            break;
        case 2:
            avdModifySettings.avdID = NEXUS_SecurityAVDType_eVDEC2;
            break;
        default:
		return	BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    #else
    avdModifySettings.avdID = (pConfig->index==0)? NEXUS_SecurityAVDType_eAVD:NEXUS_SecurityAVDType_eAVD;
    #endif

    if ( bReset )
    {
        pRegAddrValue = (AVDRegAddrValue *)&gResetAVDRegAddrValues[pConfig->index][0];
        pSignature = (unsigned char *)&gAVD_ResetAvdSignature[pConfig->index][0];
        avdModifySettings.nAVDReg = (pConfig->index==0)? NEXUS_VIDEO_REG_PROTECTION_NUM_RESET_AVD0_REGISTERS:\
                        NEXUS_VIDEO_REG_PROTECTION_NUM_RESET_AVD1_REGISTERS;
    }
    else
    {
        pRegAddrValue = (AVDRegAddrValue *)&gStartAVDRegAddrValues[pConfig->index][0];
        pSignature = (unsigned char *)&gAVD_StartAvdSignature[pConfig->index][0];
        avdModifySettings.nAVDReg = NEXUS_VIDEO_REG_PROTECTION_NUM_START_AVD_REGISTERS;
    }

    for ( i=0; i<avdModifySettings.nAVDReg; i++ )
    {
        avdModifySettings.regAddr[i] = pRegAddrValue[i].regPhysicalAddress;
        avdModifySettings.regVal[i] = pRegAddrValue[i].regValue;
    }

    for (i=0; i<NEXUS_HMACSHA256_SIGNATURE_SIZE; i++ )
    {
        avdModifySettings.signature[i] = pSignature[i];
    }

    if ( (rc = NEXUS_Security_AVDSRegistersModify(&avdModifySettings))!= NEXUS_SUCCESS )
    {
        BDBG_WRN(("Cannot StartAVD"));
        BERR_TRACE(rc);
        goto exit;
    }

exit:
    NEXUS_Security_FreeVKL(vklHandle);
    return rc;
}


BERR_Code NEXUS_VideoDecoder_P_SecureStart(
    const NEXUS_VideoDecoderRegisterProtectionConfig *pConfig
    )
{

    return NEXUS_VideoDecoder_P_SecureModify (pConfig, false);
}

BERR_Code NEXUS_VideoDecoder_P_SecureReset(
    const NEXUS_VideoDecoderRegisterProtectionConfig *pConfig
    )
{
    return NEXUS_VideoDecoder_P_SecureModify (pConfig, true);
}

#endif
