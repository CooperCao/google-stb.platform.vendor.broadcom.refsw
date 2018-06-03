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

 /* Example AES-ECB encryption record with clearkey
  * This example makes a pair with playback with app "security_keyslot_mpeg_stream_playback.c"
  *
  * The app play a very short clear stream, and then record it to another encrypted stream.
  * This remove the need to setup the frontend and channel information etc.
  */

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_API_VERSION>=2)&& NEXUS_HAS_PLAYBACK && NEXUS_HAS_RECORD && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 4) && NEXUS_HAS_VIDEO_DECODER

#    include <stdio.h>
#    include <stdlib.h>
#    include <string.h>

#    include "nexus_security.h"
#    include "nexus_keyslot.h"
#    include "nexus_security_datatypes.h"
#    include "security_utils.h"

/* Input stream file for playback -> record with encryption. */
#    define CLEAR_STREAM_FILE  "./videos/cnnticker.mpg"

/* Output stream from record. */
#    define ENCRYPTED_STREAM_FILE  "./videos/cnnticker_encrypted.mpg"

/* The main example to setup the encryption or decryption operations */
static int    SecurityExampleSetupClearkeyslot( NEXUS_ExampleSecuritySettings * settings, NEXUS_KeySlotBlockEntry entry );

int main(
    void )
{
    NEXUS_ExampleSecuritySettings settings;

    /* Nexus platform initilisation. */
    SecurityExampleInitPlatform( &settings );

    settings.playfname = CLEAR_STREAM_FILE;
    SecurityExampleSetupPlayback( &settings );

    SecurityExampleSetupDecoders( &settings );

    SecurityExampleSetupPlaybackPidChannels( &settings );

    settings.recfname = ENCRYPTED_STREAM_FILE;
    SecurityExampleSetupRecord4Encrpytion( &settings );

    /* Config CPS encryption. */
    SecurityExampleSetupClearkeyslot( &settings, NEXUS_KeySlotBlockEntry_eCpsClear );

    SecurityExampleStartRecord( &settings );
    SecurityExampleStartPlayback( &settings );

    /* Wait for the two seconds */
    sleep( 2 );
    printf( "Record has been created with file %s. \n", ENCRYPTED_STREAM_FILE );

    /* Shutdown the modules and nexus platform. */
    SecurityExampleShutdown( &settings );

    return 0;
}

/* The main example to setup the encryption or decryption operations */
static int SecurityExampleSetupClearkeyslot(
    NEXUS_ExampleSecuritySettings * settings,
    NEXUS_KeySlotBlockEntry entry )
{
    NEXUS_Error   rc = NEXUS_UNKNOWN;

    NEXUS_KeySlotAllocateSettings keyslotAllocSettings;
    NEXUS_KeySlotSettings keyslotSettings;
    NEXUS_KeySlotEntrySettings keyslotEntrySettings;
    NEXUS_KeySlotKey slotKey;

    /* allocate keyslot */
    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );

    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerSlot;
    settings->videoKeyHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );

    if( !settings->videoKeyHandle ) {
        printf( "\nAllocate video keyslot failed \n" );
        rc = NEXUS_NOT_AVAILABLE;
        SECURITY_CHECK_RC( rc );
    }

    NEXUS_KeySlot_GetSettings( settings->videoKeyHandle, &keyslotSettings );
    rc = NEXUS_KeySlot_SetSettings( settings->videoKeyHandle, &keyslotSettings );
    SECURITY_CHECK_RC( rc );

    NEXUS_KeySlot_GetDefaultAllocateSettings( &keyslotAllocSettings );

    keyslotAllocSettings.owner = NEXUS_SecurityCpuContext_eHost;
    keyslotAllocSettings.slotType = NEXUS_KeySlotType_eIvPerSlot;
    settings->audioKeyHandle = NEXUS_KeySlot_Allocate( &keyslotAllocSettings );

    if( !settings->audioKeyHandle ) {
        printf( "\nAllocate audio keyslot failed \n" );
        rc = NEXUS_NOT_AVAILABLE;
        SECURITY_CHECK_RC( rc );
    }

    NEXUS_KeySlot_GetSettings( settings->audioKeyHandle, &keyslotSettings );

    rc = NEXUS_KeySlot_SetSettings( settings->audioKeyHandle, &keyslotSettings );
    SECURITY_CHECK_RC( rc );

    NEXUS_KeySlot_GetEntrySettings( settings->videoKeyHandle, entry, &keyslotEntrySettings );
    keyslotEntrySettings.algorithm = NEXUS_CryptographicAlgorithm_eAes128;
    keyslotEntrySettings.algorithmMode = NEXUS_CryptographicAlgorithmMode_eEcb;
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    keyslotEntrySettings.rPipeEnable = false;   /* Don't encrypt on r-pipe if display is needed */
    keyslotEntrySettings.gPipeEnable = true;    /* Encrypt on g-pipe that goes to record (default) */

    rc = NEXUS_KeySlot_SetEntrySettings( settings->videoKeyHandle, entry, &keyslotEntrySettings );
    SECURITY_CHECK_RC( rc );

    BKNI_Memcpy( slotKey.key, VIDEO_ENCRYPTION_KEY, 16 );
    slotKey.size = 16;
    rc = NEXUS_KeySlot_SetEntryKey( settings->videoKeyHandle, entry, &slotKey );
    SECURITY_CHECK_RC( rc );

    NEXUS_KeySlot_GetEntrySettings( settings->audioKeyHandle, entry, &keyslotEntrySettings );

    keyslotEntrySettings.algorithm = NEXUS_CryptographicAlgorithm_eAes128;
    keyslotEntrySettings.algorithmMode = NEXUS_CryptographicAlgorithmMode_eEcb;
    keyslotEntrySettings.terminationMode = NEXUS_KeySlotTerminationMode_eClear;
    keyslotEntrySettings.rPipeEnable = false;   /* Don't encrypt on r-pipe if display is needed */
    keyslotEntrySettings.gPipeEnable = true;    /* Encrypt on g-pipe that goes to record (default) */
    rc = NEXUS_KeySlot_SetEntrySettings( settings->audioKeyHandle, entry, &keyslotEntrySettings );
    BKNI_Memcpy( slotKey.key, AUDIO_ENCRYPTION_KEY, 16 );
    slotKey.size = 16;
    rc = NEXUS_KeySlot_SetEntryKey( settings->audioKeyHandle, entry, &slotKey );
    SECURITY_CHECK_RC( rc );

    rc = NEXUS_KeySlot_AddPidChannel( settings->videoKeyHandle, settings->videoPidChannel );
    SECURITY_CHECK_RC( rc );

    rc = NEXUS_KeySlot_AddPidChannel( settings->audioKeyHandle, settings->audioPidChannel );
    SECURITY_CHECK_RC( rc );

  exit:
    return rc;
}

#else
#    include <stdio.h>
int main(
    void )
{
    printf( "This application is not supported on this platform!\n" );
    return -1;
}
#endif
