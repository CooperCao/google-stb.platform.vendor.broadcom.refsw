/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

#if NEXUS_HAS_SECURITY

#include "nexus_platform.h"
#include "nexus_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "security_main.h"
#include "security_util.h"
#include "security_skey.h"

BDBG_MODULE(security_framework);

int test_keyslot_open(
    securityTestConfig * pArgs )
{
    NEXUS_Error   rc = NEXUS_SUCCESS;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyslot );
        SECURITY_FRAMEWORK_SET_NUMBER( 1 );
        SECURITY_FRAMEWORK_SET_NAME( "open" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "check that keyslots can be open" );
        return 1;
    }

    verboseLog( "EXECUTING  [%s]\n", __FUNCTION__ );

    rc = securityTest_Keyslot_open_close_all( pArgs->group, NULL );

    return rc;
}

int test_keyslot_noKey(
    securityTestConfig * pArgs )
{
    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyslot );
        SECURITY_FRAMEWORK_SET_NUMBER( 2 );
        SECURITY_FRAMEWORK_SET_NAME( "noKey" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "check the clear data passes DMA when no key is set" );
        return 1;
    }

    verboseLog( "EXECUTING  [%s]\n", __FUNCTION__ );

    return 0;
}

int test_keyslot_aes(
    securityTestConfig * pArgs )
{
    char         *op;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyslot );
        SECURITY_FRAMEWORK_SET_NUMBER( 3 );
        SECURITY_FRAMEWORK_SET_NAME( "AES" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "Check AES combinations." );
        return 1;
    }

    op = "encryption";
    verboseLog( "EXECUTING  [%s] AES %s.\n", __FUNCTION__, op );
    rc = securityTest_Keyslot_crypto_AES( pArgs->number, &op );

    op = "decryption";
    verboseLog( "EXECUTING  [%s] AES %s.\n", __FUNCTION__, op );
    rc = securityTest_Keyslot_crypto_AES( pArgs->number, &op );

    return rc;
}

int test_keyslot_des(
    securityTestConfig * pArgs )
{
    char         *op;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyslot );
        SECURITY_FRAMEWORK_SET_NUMBER( 4 );
        SECURITY_FRAMEWORK_SET_NAME( "DES" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "Check DES combinations." );
        return 1;
    }

    op = "encryption";
    verboseLog( "EXECUTING  [%s] DES %s.\n", __FUNCTION__, op );
    rc = securityTest_Keyslot_crypto_DES( pArgs->number, &op );

    op = "decryption";
    verboseLog( "EXECUTING  [%s] DES %s.\n", __FUNCTION__, op );
    rc = securityTest_Keyslot_crypto_DES( pArgs->number, &op );

    return rc;
}

int test_keyslot_tdes(
    securityTestConfig * pArgs )
{
    char         *op;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyslot );
        SECURITY_FRAMEWORK_SET_NUMBER( 5 );
        SECURITY_FRAMEWORK_SET_NAME( "TDES" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "Check 3DES ABA combinations." );
        return 1;
    }

    op = "encryption";
    verboseLog( "EXECUTING  [%s] TDES %s.\n", __FUNCTION__, op );
    rc = securityTest_Keyslot_crypto_TDES( pArgs->number, &op );

    op = "decryption";
    verboseLog( "EXECUTING  [%s] DES %s.\n", __FUNCTION__, op );
    rc = securityTest_Keyslot_crypto_TDES( pArgs->number, &op );
    return rc;
}

int test_keyslot_ext_iv_key(
    securityTestConfig * pArgs )
{
    char         *algo;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyslot );
        SECURITY_FRAMEWORK_SET_NUMBER( 6 );
        SECURITY_FRAMEWORK_SET_NAME( "EXTERNAL_IV_KEY" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( "Check the use of external IV and key." );
        return 1;
    }

    algo = "AES_CBC";
    verboseLog( "EXECUTING  [%s] EXTERNAL_IV_KEY.\n", __FUNCTION__ );
    rc = securityTest_Keyslot_external_iv_key( pArgs->number, &algo );

    return rc;
}

int test_all_keyslots_ext_iv_key(
    securityTestConfig * pArgs )
{
    char         *algo;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyslot );
        SECURITY_FRAMEWORK_SET_NUMBER( 7 );
        SECURITY_FRAMEWORK_SET_NAME( "EXTERNAL_IV_KEYS" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION
            ( "Use of all the external IV and key slots with three different alternated keys simutanousily." );
        return 1;
    }

    algo = "AES_CBC";
    verboseLog( "EXECUTING  [%s] EXTERNAL_IV_KEYS.\n", __FUNCTION__ );
    rc = securityTest_Keyslot_all_external_iv_keys( pArgs->number, &algo );

    return rc;
}

int test_keyslot_SC_bits_DMA (
    securityTestConfig * pArgs )
{
    char         *algo;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyslot );
        SECURITY_FRAMEWORK_SET_NUMBER( 8 );
        SECURITY_FRAMEWORK_SET_NAME( "SC_BITS_DMA" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( ( "Using DMA to check the key related to SC bits and change the output TS packet SC value." ) );
        return 1;
    }

    algo = "AES_ECB";
    verboseLog( "EXECUTING  [%s] SC_BITS.\n", __FUNCTION__ );
    rc = securityTest_Keyslot_SC_bits( pArgs->number, &algo );

    return rc;
}

int test_keyslot_SC_bits_rec_playback(
    securityTestConfig * pArgs )
{
    char         *algo;
    NEXUS_Error   rc = NEXUS_SUCCESS;

    if( pArgs->enquire ) {
        SECURITY_FRAMEWORK_SET_GROUP( securityTestGroup_keyslot );
        SECURITY_FRAMEWORK_SET_NUMBER( 9 );
        SECURITY_FRAMEWORK_SET_NAME( "SC_BITS_REC_PLAYBACK" );
        SECURITY_FRAMEWORK_SET_DESCRIPTION( ( "Using DMA to check the key related to SC bits and change the output TS packet SC value." ) );
        return 1;
    }

    algo = "AES_ECB";
    verboseLog( "EXECUTING  [%s] SC_BITS.\n", __FUNCTION__ );
    /* TODO: to be enabled, pending to JIRA SW7272-4 */
    /* rc = securityTest_Keyslot_SC_bits_rec_playback( pArgs->number, &algo ); */

    return rc;
}

/* initalise the keyslot tests. */
void initTestGroup_keyslot(
    void )
{
    /* register group */
    securityFramework_registerGroup( securityTestGroup_keyslot, "KEYSLOT" );

    /* register keyskot tests.  */
    securityFramework_registerTest( test_keyslot_SC_bits_rec_playback);
    securityFramework_registerTest( test_keyslot_SC_bits_DMA );
    securityFramework_registerTest( test_all_keyslots_ext_iv_key );
    securityFramework_registerTest( test_keyslot_ext_iv_key );
    securityFramework_registerTest( test_keyslot_tdes );
    securityFramework_registerTest( test_keyslot_des );
    securityFramework_registerTest( test_keyslot_aes );
    securityFramework_registerTest( test_keyslot_noKey );
    securityFramework_registerTest( test_keyslot_open );

    return;
}

#endif
