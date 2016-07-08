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


#include "nexus_video_encoder_module.h"
#include "nexus_video_encoder_register_protection.h"
#include "nexus_video_encoder_security.h"
#include "nexus_bsp_config.h"
#include "nexus_keyladder.h"
#include "nexus_video_encoder_register_protection_settings.c"
#include "nexus_video_encoder_register_protection_signatures.c"


#if (NEXUS_NUM_VCE_DEVICES)

BDBG_MODULE(nexus_encoder_security);
BDBG_OBJECT_ID(NEXUS_VideoEncoderSecurityRegisterProtection);

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
		0x98, 0x76, 0x54, 0x32, 0x09, 0x23, 0x45, 0x56
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

	/* ++++++++ */
	AlgConfig.terminationMode     = NEXUS_SecurityTerminationMode_eClear;
	AlgConfig.ivMode              = NEXUS_SecurityIVMode_eRegular;
	AlgConfig.solitarySelect      = NEXUS_SecuritySolitarySelect_eClear;

#if (BCHP_CHIP!=7445) &&(BCHP_CHIP!=7252)
	AlgConfig.caVendorID          = 0x1234;
	AlgConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_3;
	AlgConfig.otpId               = NEXUS_SecurityOtpId_eOtpVal;
#else
	AlgConfig.caVendorID          = 0x8176 /*0x8d92*/;
    AlgConfig.askmModuleID        = NEXUS_SecurityAskmModuleID_eModuleID_15;
	AlgConfig.otpId               = NEXUS_SecurityOtpId_eOneVal;
#endif

	AlgConfig.testKey2Select      = 2;
	AlgConfig.key2Select      = NEXUS_SecurityKey2Select_eFixedKey;

	/* ++++++++ */

	if ( NEXUS_Security_ConfigAlgorithm (keyHandle, &AlgConfig) != 0 )
	{
		return 1;
	}

	BKNI_Memset (&encryptedSessionkey, 0, sizeof(NEXUS_SecurityEncryptedSessionKey));
	encryptedSessionkey.keyladderID 	= NEXUS_SecurityKeyladderID_eB;
	encryptedSessionkey.keyladderType 	= NEXUS_SecurityKeyladderType_e3Des;


#if (BCHP_CHIP!=7445) &&(BCHP_CHIP!=7252)

	encryptedSessionkey.swizzleType		= NEXUS_SecuritySwizzleType_eSwizzle0;
    encryptedSessionkey.rootKeySrc 		= NEXUS_SecurityRootKeySrc_eCuskey;
	encryptedSessionkey.operation 		= NEXUS_SecurityOperation_eEncrypt;
	encryptedSessionkey.operationKey2 	= NEXUS_SecurityOperation_eEncrypt;
#else
	encryptedSessionkey.swizzleType		= NEXUS_SecuritySwizzleType_eNone;
	encryptedSessionkey.rootKeySrc 		= NEXUS_SecurityRootKeySrc_eGlobalKey;
	encryptedSessionkey.askmGlobalKeyIndex = 0x3E;
	encryptedSessionkey.globalKeyOwnerId = NEXUS_SecurityGlobalKeyOwnerID_eUse1;
	encryptedSessionkey.globalKeyVersion = 0;
	encryptedSessionkey.bASKMMode   	= true;
	encryptedSessionkey.operation 		= NEXUS_SecurityOperation_eDecrypt;
	encryptedSessionkey.operationKey2 	= NEXUS_SecurityOperation_eDecrypt;
#endif

	encryptedSessionkey.cusKeyL 		= 0x0a;
	encryptedSessionkey.cusKeyH 		= 0x0a;
	encryptedSessionkey.cusKeyVarL 		= 0x27;
	encryptedSessionkey.cusKeyVarH 		= 0x27;

	encryptedSessionkey.bRouteKey 		= false;

	encryptedSessionkey.keyEntryType 	= NEXUS_SecurityKeyType_eOdd;

	/* ++++++++ */
	encryptedSessionkey.custSubMode        = NEXUS_SecurityCustomerSubMode_eGeneralPurpose1;
	encryptedSessionkey.virtualKeyLadderID = vklId;
	encryptedSessionkey.keyMode            = NEXUS_SecurityKeyMode_eRegular;
	/* ++++++++ */

	/*encryptedSessionkey.pKeyData 		= ucProcInForKey3;*/
	BKNI_Memcpy(encryptedSessionkey.keyData, ucProcInForKey3, sizeof(ucProcInForKey3));

	rc=NEXUS_Security_GenerateSessionKey(keyHandle, &encryptedSessionkey);

error:

    NEXUS_Security_FreeKeySlot (keyHandle);

	return rc;
}

BERR_Code NEXUS_VideoEncoder_P_EnableRegisterProtection( unsigned deviceId )
{
	unsigned int i;
	BERR_Code rc = NEXUS_SUCCESS;
	NEXUS_SecurityAVDSRegRangeSettings	viceRegRangeSettings;
	ViceRegRegion * pRegInfo;
	NEXUS_SecurityVirtualKeyladderID vklId;
    NEXUS_VirtualKeyLadderHandle vklHandle;

	BDBG_ENTER(NEXUS_VideoEncoder_P_EnableRegisterProtection);

    if( deviceId >= NEXUS_NUM_VCE_DEVICES )
    {
        return BERR_TRACE( NEXUS_INVALID_PARAMETER );
    }

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

	viceRegRangeSettings.vkl = vklId;
	viceRegRangeSettings.keyLayer = NEXUS_SecurityKeySource_eKey3;
	viceRegRangeSettings.VDECId = NEXUS_SecurityAVDType_eVICE;
	viceRegRangeSettings.nRange = sizeof(gVice_Regions)/sizeof(ViceRegRegion);
	pRegInfo = (ViceRegRegion*)gVice_Regions;


	BDBG_ASSERT(viceRegRangeSettings.nRange<=NEXUS_MAX_AVDSVD_RANGE);

	for ( i=0; i< viceRegRangeSettings.nRange; i++ )
	{
		viceRegRangeSettings.loRange[i] = pRegInfo[i].startPhysicalAddress;
		viceRegRangeSettings.hiRange[i] = pRegInfo[i].endPhysicalAddress;
	}

	for (i=0;i<NEXUS_HMACSHA256_SIGNATURE_SIZE;i++)
	{
		viceRegRangeSettings.signature[i] = gViceSetVichSignature[deviceId][i];
	}
	if ( (rc = NEXUS_Security_AVDSRegistersSetUp(&viceRegRangeSettings))!= NEXUS_SUCCESS )
	{
		BDBG_WRN(("Cannot SetVICH %08x\n", rc));
		BERR_TRACE(rc);
		goto exit;
	}

exit:

    NEXUS_Security_FreeVKL(vklHandle);

	return rc;
}

#endif
