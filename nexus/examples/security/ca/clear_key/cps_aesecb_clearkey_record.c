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
/* Example record CPS AES-ECB encryption with clearkey - same key for m2m decryption playback */
/* This example makes a pair with  playback using "cps_aesecb_clearkey_playback.c " */
#if NEXUS_HAS_SECURITY && NEXUS_HAS_PLAYBACK && NEXUS_HAS_RECORD && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nexus_security_examples_setups.h"

/* Input stream file for playback -> record with encryption. */
#define CLEAR_STREAM_FILE               "videos/576i50_2min.ts";

/* Output stream from record. */
#define ENCRYPTED_STREAM_FILE           "videos/recorded_aesecb_clearKey.mpg"

static NEXUS_Error SecurityExampleSecuritySetups ( NEXUS_ExampleSecuritySettings * settings );
static NEXUS_Error SecurityExampleSetupKeySlotForCPS ( NEXUS_KeySlotHandle keyHandle, unsigned const char *pKeyOdd );

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

    /* Config CA scrambler */
    SecurityExampleSecuritySetups ( &settings );

    SecurityExampleStartRecord ( &settings );
    SecurityExampleStartPlayback ( &settings );

    /* Wait for the two seconds */
    sleep ( 2 );
    printf ( "Record has been created with file %s. Use playback to inspect on HDMI.\n", ENCRYPTED_STREAM_FILE );

    /* Shutdown the modules and nexus platform. */
    SecurityExampleShutdown ( &settings );

    return 0;
}

static NEXUS_Error SecurityExampleSecuritySetups ( NEXUS_ExampleSecuritySettings * settings )
{

    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_SecurityInvalidateKeySettings invSettings;

    /*      Config CA descrambler */
    NEXUS_Security_GetDefaultKeySlotSettings ( &keySlotSettings );
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCaCp; /* For encryption of the stream. */

    /* Allocate AV keyslots */
    settings->videoKeyHandle = NEXUS_Security_AllocateKeySlot ( &keySlotSettings );
    if ( !settings->videoKeyHandle )
    {
        printf ( "\nAllocate CACP video keyslot failed \n" );
        return NEXUS_NOT_INITIALIZED;
    }

    settings->audioKeyHandle = NEXUS_Security_AllocateKeySlot ( &keySlotSettings );
    if ( !settings->audioKeyHandle )
    {
        printf ( "\nAllocate CACP audio keyslot failed \n" );
        return NEXUS_NOT_INITIALIZED;
    }

    /* Invalidate all the keys. */
    NEXUS_Security_GetDefaultInvalidateKeySettings ( &invSettings );
    invSettings.invalidateAllEntries = true;
    invSettings.invalidateKeyType = NEXUS_SecurityInvalidateKeyFlag_eDestKeyOnly;
    NEXUS_Security_InvalidateKey ( settings->videoKeyHandle, &invSettings );

    printf ( "\n CPS scrambler settings.\n" );

    if ( SecurityExampleSetupKeySlotForCPS ( settings->videoKeyHandle, VIDEO_ENCRYPTION_KEY ) != 0 )
    {
        printf ( "\nConfig video CPD Algorithm failed for video keyslot 1 \n" );
        return NEXUS_NOT_INITIALIZED;
    }

    printf ( "\nSecurity Config video CW OK\n" );
    if ( SecurityExampleSetupKeySlotForCPS ( settings->audioKeyHandle, AUDIO_ENCRYPTION_KEY ) != 0 )
    {
        printf ( "\nConfig audio CPD Algorithm failed for audio keyslot 1 \n" );
        return NEXUS_NOT_INITIALIZED;
    }

    printf ( "\nSecurity Config audio CW OK\n" );

    /* Add video PID channel to keyslot */
    BDBG_ASSERT ( settings->videoPidChannel );
    NEXUS_KeySlot_AddPidChannel ( settings->videoKeyHandle, settings->videoPidChannel );

    /* Add video PID channel to keyslot */
    BDBG_ASSERT ( settings->audioPidChannel );
    NEXUS_KeySlot_AddPidChannel ( settings->audioKeyHandle, settings->audioPidChannel );

    printf ( "\nSecurity Config OK.\n" );
    return NEXUS_SUCCESS;
}

static NEXUS_Error SecurityExampleSetupKeySlotForCPS ( NEXUS_KeySlotHandle keyHandle, unsigned const char *pKeyOdd )
{
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityClearKey key;

    /* Change the output stream packet SC bits to Odd polarity, as an example. */
    NEXUS_SecurityAlgorithmScPolarity outputSCBits = NEXUS_SecurityAlgorithmScPolarity_eOdd;

    NEXUS_Security_GetDefaultAlgorithmSettings ( &AlgConfig );
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eAes128;
    AlgConfig.algorithmVar = NEXUS_SecurityAlgorithmVariant_eEcb;
    AlgConfig.terminationMode = NEXUS_SecurityTerminationMode_eCipherStealing;
    AlgConfig.dest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
    AlgConfig.operation = NEXUS_SecurityOperation_eEncrypt;
    AlgConfig.keyDestEntryType = NEXUS_SecurityKeyType_eClear;

    AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eRestricted] = true;
    AlgConfig.modifyScValue[NEXUS_SecurityPacketType_eGlobal] = true;
    AlgConfig.scValue[NEXUS_SecurityPacketType_eRestricted] = outputSCBits;
    AlgConfig.scValue[NEXUS_SecurityPacketType_eGlobal] = outputSCBits;

    NEXUS_Security_ConfigAlgorithm ( keyHandle, &AlgConfig );

    /* Load AV keys */
    NEXUS_Security_GetDefaultClearKey ( &key );
    key.keySize = 16;

    key.dest = NEXUS_SecurityAlgorithmConfigDestination_eCps;
    key.keyIVType = NEXUS_SecurityKeyIVType_eNoIV;
    memcpy ( key.keyData, pKeyOdd, key.keySize );

    key.sc01Polarity[NEXUS_SecurityPacketType_eGlobal] = outputSCBits;
    key.sc01Polarity[NEXUS_SecurityPacketType_eRestricted] = outputSCBits;
    /* key.sc01GlobalMapping = outputSCBits; */
    key.keyEntryType = NEXUS_SecurityKeyType_eClear;
    NEXUS_Security_LoadClearKey ( keyHandle, &key );
    return 0;

}

#else /* #if NEXUS_HAS_SECURITY  && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER */

#include <stdio.h>
int main ( void )
{
    printf ( "This application is not supported on this platform!\n" );
    return -1;
}
#endif
