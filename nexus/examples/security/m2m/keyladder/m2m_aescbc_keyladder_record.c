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
 /* Example m2m AES-CBC  encryption  record with keyladder   */
 /* This example makes a pair with  playback using "m2m_aescbc_keyladder_playback.c " */

#if NEXUS_HAS_SECURITY && NEXUS_HAS_PLAYBACK && NEXUS_HAS_RECORD && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nexus_security_examples_setups.h"

/* Input stream file for playback -> record with encryption. */
#define CLEAR_STREAM_FILE               "videos/576i50_2min.ts";

/* Output stream from record. */
#define ENCRYPTED_STREAM_FILE           "videos/recorded_m2m_aescbc_keyladder.mpg"

static int      SecurityExampleSetupM2mKeyladder ( NEXUS_ExampleSecuritySettings * settings,
                                                   NEXUS_SecurityOperation operation );

int main ( void )
{
    NEXUS_ExampleSecuritySettings settings;

    /* Nexus platform initilisation. */
    SecurityExampleInitPlatform ( &settings );

    settings.playfname = CLEAR_STREAM_FILE;
    SecurityExampleSetupPlayback ( &settings );

    SecurityExampleSetupDecoders ( &settings );

    SecurityExampleSetupPlaybackPidChannels ( &settings );

    settings.recfname = ENCRYPTED_STREAM_FILE;
    SecurityExampleSetupRecord4Encrpytion ( &settings );

    /* Config M2M encryption. */
    SecurityExampleSetupM2mKeyladder ( &settings, NEXUS_SecurityOperation_eEncrypt );

    SecurityExampleStartRecord ( &settings );
    SecurityExampleStartPlayback ( &settings );

    /* Wait for the two seconds */
    sleep ( 2 );
    printf ( "Record has been created with file %s. Use playback to inspect on HDMI.\n", ENCRYPTED_STREAM_FILE );

    /* Shutdown the modules and nexus platform. */
    SecurityExampleShutdown ( &settings );

    return 0;
}

static int SecurityExampleAllocateVkl ( NEXUS_SecurityCustomerSubMode custSubMode,
                                        NEXUS_SecurityVirtualKeyladderID * vkl,
                                        NEXUS_VirtualKeyLadderHandle * vklHandle )
{
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderInfo vklInfo;

    BDBG_ASSERT ( vkl );
    BDBG_ASSERT ( vklHandle );

    NEXUS_Security_GetDefaultVKLSettings ( &vklSettings );

    /* For pre Zeus 3.0, please set vklSettings.custSubMode */
    vklSettings.custSubMode = custSubMode;

    *vklHandle = NEXUS_Security_AllocateVKL ( &vklSettings );

    if ( !( *vklHandle ) )
    {
        printf ( "\nAllocate VKL failed \n" );
        return 1;
    }

    NEXUS_Security_GetVKLInfo ( *vklHandle, &vklInfo );
    printf ( "\nVKL Handle %p is allocated for VKL#%d\n", ( void * ) *vklHandle, vklInfo.vkl & 0x7F );

    *vkl = vklInfo.vkl;

    return 0;
}

static int SecurityExampleSetupM2mKeyladder ( NEXUS_ExampleSecuritySettings * settings,
                                              NEXUS_SecurityOperation operation )
{
    NEXUS_KeySlotHandle keyHandle;
    NEXUS_SecurityInvalidateKeySettings invSettings;
    NEXUS_SecurityAlgorithmSettings NexusConfig;
    NEXUS_SecurityEncryptedSessionKey encryptedSessionkey;
    NEXUS_SecurityEncryptedControlWord encrytedCW;
    NEXUS_SecurityKeySlotSettings keySettings;
    NEXUS_SecurityVirtualKeyladderID vklId;

    unsigned char   ucProcInForKey3[16] = {
        0x0f, 0x09, 0xa2, 0x06, 0x19, 0x88, 0xb6, 0x89,
        0x28, 0xeb, 0x90, 0x2e, 0xb2, 0x36, 0x18, 0x88
    };

    unsigned char   ucProcInForKey4[16] = {
        0xe4, 0x62, 0x75, 0x1b, 0x5d, 0xd4, 0xbc, 0x02,
        0x27, 0x9e, 0xc9, 0x6c, 0xc8, 0x66, 0xec, 0x10
    };

    NEXUS_SecurityClearKey key;
    uint8_t         ivkeys[16] =
        { 0xad, 0xd6, 0x9e, 0xa3, 0x89, 0xc8, 0x17, 0x72, 0x1e, 0xd4, 0x0e, 0xab, 0x3d, 0xbc, 0x7a, 0xf2 };

    /* Request for an VKL to use */
    if ( SecurityExampleAllocateVkl ( NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4, &vklId, &settings->vklHandle ) )
    {
        printf ( "\nAllocate VKL failed.\n" );
        return 1;
    }

    NEXUS_Security_GetDefaultKeySlotSettings ( &keySettings );
    keySettings.keySlotEngine = NEXUS_SecurityEngine_eM2m;
    keySettings.keySlotType = NEXUS_SecurityKeySlotType_eType2;
    settings->videoKeyHandle = NEXUS_Security_AllocateKeySlot ( &keySettings );
    if ( !settings->videoKeyHandle )
    {
        printf ( "\nAllocate enc keyslot failed \n" );
        return 1;
    }

    /* Invalidate all the keys. */
    NEXUS_Security_GetDefaultInvalidateKeySettings ( &invSettings );
    invSettings.invalidateAllEntries = true;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    NEXUS_Security_InvalidateKey ( settings->videoKeyHandle, &invSettings );

    /* configure the key slot */
    NEXUS_Security_GetDefaultAlgorithmSettings ( &NexusConfig );
    NexusConfig.algorithm = NEXUS_SecurityAlgorithm_eAes;
    NexusConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eCbc;
    NexusConfig.terminationMode = NEXUS_SecurityTerminationMode_eClear;
    NexusConfig.ivMode = NEXUS_SecurityIVMode_eRegular;
    NexusConfig.solitarySelect = NEXUS_SecuritySolitarySelect_eClear;
    NexusConfig.caVendorID = 0x1234;
    NexusConfig.askmModuleID = NEXUS_SecurityAskmModuleID_eModuleID_4;
    NexusConfig.otpId = NEXUS_SecurityOtpId_eOtpVal;
    NexusConfig.key2Select = NEXUS_SecurityKey2Select_eReserved1;
    NexusConfig.operation = operation;
    NexusConfig.keyDestEntryType = NEXUS_SecurityKeyType_eClear;
    NexusConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;

    if ( NEXUS_Security_ConfigAlgorithm ( settings->videoKeyHandle, &NexusConfig ) != 0 )
    {
        printf ( "\nConfigAlg clear keyslot failed \n" );
        return 1;
    }

    /* set up the key ladder to route out the key for (operation) */
    NEXUS_Security_GetDefaultSessionKeySettings ( &encryptedSessionkey );
    encryptedSessionkey.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encryptedSessionkey.swizzleType = NEXUS_SecuritySwizzleType_eNone;
    encryptedSessionkey.cusKeyL = 0x00;
    encryptedSessionkey.cusKeyH = 0x00;
    encryptedSessionkey.cusKeyVarL = 0x00;
    encryptedSessionkey.cusKeyVarH = 0xFF;
    encryptedSessionkey.keyGenCmdID = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encryptedSessionkey.sessionKeyOp = NEXUS_SecuritySessionKeyOp_eNoProcess;
    encryptedSessionkey.bASKMMode = false;
    encryptedSessionkey.rootKeySrc = NEXUS_SecurityRootKeySrc_eOtpKeyA;
    encryptedSessionkey.bSwapAESKey = false;
    encryptedSessionkey.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
    encryptedSessionkey.bRouteKey = false;
    encryptedSessionkey.operation = NEXUS_SecurityOperation_eDecrypt;
    encryptedSessionkey.operationKey2 = NEXUS_SecurityOperation_eEncrypt;
    encryptedSessionkey.keyEntryType = NEXUS_SecurityKeyType_eClear;

    encryptedSessionkey.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encryptedSessionkey.virtualKeyLadderID = vklId;
    encryptedSessionkey.keyMode = NEXUS_SecurityKeyMode_eRegular;
    encryptedSessionkey.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;

    BKNI_Memcpy ( encryptedSessionkey.keyData, ucProcInForKey3, sizeof ( ucProcInForKey3 ) );

    if ( NEXUS_Security_GenerateSessionKey ( settings->videoKeyHandle, &encryptedSessionkey ) != 0 )
    {
        printf ( "\nLoading session key failed \n" );
        return 1;
    }

    /* Load CW - key4 -- routed out */
    NEXUS_Security_GetDefaultControlWordSettings ( &encrytedCW );

    encrytedCW.keyladderType = NEXUS_SecurityKeyladderType_e3Des;
    encrytedCW.keySize = sizeof ( ucProcInForKey4 );
    encrytedCW.keyEntryType = NEXUS_SecurityKeyType_eClear;

    encrytedCW.custSubMode = NEXUS_SecurityCustomerSubMode_eGeneric_CP_128_4;
    encrytedCW.virtualKeyLadderID = vklId;
    encrytedCW.keyMode = NEXUS_SecurityKeyMode_eRegular;

    BKNI_Memcpy ( encrytedCW.keyData, ucProcInForKey4, encrytedCW.keySize );
    encrytedCW.operation = NEXUS_SecurityOperation_eDecrypt;
    encrytedCW.bRouteKey = true;
    encrytedCW.keyDestIVType = NEXUS_SecurityKeyIVType_eNoIV;
    encrytedCW.keyGenCmdID = NEXUS_SecurityKeyGenCmdID_eKeyGen;
    encrytedCW.bSwapAESKey = false;
    encrytedCW.bASKMMode = false;
    encrytedCW.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;

    if ( NEXUS_Security_GenerateControlWord ( settings->videoKeyHandle, &encrytedCW ) )
    {
        printf ( "\nGenerate Control Word failed for Key Slot Handle 0x%p\n", keyHandle );
        return 1;
    }

    /* Load IV. */
    NEXUS_Security_GetDefaultClearKey ( &key );
    key.keySize = 16;
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    key.keyIVType = NEXUS_SecurityKeyIVType_eIV;
    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eMem2Mem;
    BKNI_Memcpy ( key.keyData, ivkeys, key.keySize );
    NEXUS_Security_LoadClearKey ( settings->videoKeyHandle, &key );

    /* Add video PID channel to keyslot */
    BDBG_ASSERT ( settings->videoPidChannel );
    NEXUS_KeySlot_AddPidChannel ( settings->videoKeyHandle, settings->videoPidChannel );

    return ( 0 );
}

#else /* NEXUS_HAS_SECURITY && ... */
#include <stdio.h>
int main ( void )
{
    printf ( "This application is not supported on this platform!\n" );
    return -1;
}
#endif
