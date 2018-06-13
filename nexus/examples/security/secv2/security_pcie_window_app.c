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
#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_API_VERSION==2) && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 5)

#include "nexus_memory.h"
#include "nexus_base_mmap.h"
#include "nexus_security.h"
#include "nexus_pcie_window.h"
#include "nexus_otp_msp.h"
#include "nexus_otp_msp_indexes.h"
#include "security_utils.h"
#include "security_test_vectors_clear_key.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

static long long _getInt( char *p )
{
    int base = 10;

    if( !p ) {
        return BERR_TRACE(0);
    }

    if( p[0] == '0' && ( p[1] == 'x' || p[1] == 'X' ) ) {
        base = 16;
    };

    return (long long)strtoll( p, NULL, base );
}

int main(
    int argc,
    char **argv )
{
    NEXUS_Error   rc = NEXUS_UNKNOWN;
    NEXUS_OtpMspRead mspOtpRead;
    NEXUS_MemoryAllocationSettings memSetting;
    NEXUS_SecurityPciEMaxWindowSizeSettings *maxWinSize = NULL;
    NEXUS_Addr    offset = 0;
    uint8_t      *addr = 0;
    size_t        size = 10 * 1024;
    uint8_t       signedCommand[NEXUS_PCIE_MAX_WINDOW_RAW_COMMAND_SIZE] = { 0 };    /* TODO: replace with the correct command blob. */
    bool          maxWinSizeEnfored = false;
    char ch;


    /* Start NEXUS. */
    securityUtil_PlatformInit( false );

    /* As we are to use signed command to set the PCIE windowing, we check the restrictions. */
    BDBG_LOG( ( "Pcie0 OTP MSP controls are the followings:" ) );

    BKNI_Memset( &mspOtpRead, 0, sizeof( mspOtpRead ) );
    rc = NEXUS_OtpMsp_Read( NEXUS_OTPMSP_PCIE0_HOST_PROTECT, &mspOtpRead );
    SECURITY_CHECK_RC( rc );

    if( !mspOtpRead.data & 1 ) {
        BDBG_LOG( ( "Allow Pcie0 host bridge mode." ) );
    }
    else {
        BDBG_LOG( ( "Pretect Pcie0 host bridge mode." ) );
    }

    BKNI_Memset( &mspOtpRead, 0, sizeof( mspOtpRead ) );
    rc = NEXUS_OtpMsp_Read( NEXUS_OTPMSP_PCIE0_CLIENT_PROTECT, &mspOtpRead );
    SECURITY_CHECK_RC( rc );

    if( !mspOtpRead.data & 1 ) {
        BDBG_LOG( ( "Allow Pcie0 client mode." ) );
    }
    else {
        BDBG_LOG( ( "Pretect Pcie0 client mode." ) );
    }

    BKNI_Memset( &mspOtpRead, 0, sizeof( mspOtpRead ) );
    rc = NEXUS_OtpMsp_Read( NEXUS_OTPMSP_PCIE0_MWIN_RESTRICT_ENABLE, &mspOtpRead );
    SECURITY_CHECK_RC( rc );

    if( !mspOtpRead.data & 1 ) {
        BDBG_LOG( ( "No restriction on Pcie0 memory window access." ) );
    }
    else {
        BDBG_LOG( ( "BSP command memory window set up must complete first before the memory access." ) );
    }

    BKNI_Memset( &mspOtpRead, 0, sizeof( mspOtpRead ) );
    rc = NEXUS_OtpMsp_Read( NEXUS_OTPMSP_PCIE0_GWIN_DISABLE, &mspOtpRead );
    SECURITY_CHECK_RC( rc );

    if( !mspOtpRead.data & 1 ) {
        BDBG_LOG( ( "Pcie0 GISB address window access is allowed." ) );
    }
    else {
        BDBG_LOG( ( "Pcie0 GISB address window access is disallowed." ) );
    }

    BKNI_Memset( &mspOtpRead, 0, sizeof( mspOtpRead ) );
    rc = NEXUS_OtpMsp_Read( NEXUS_OTPMSP_PCIE0_MWIN_SIZE_ENFORCE_ENABLE, &mspOtpRead );
    SECURITY_CHECK_RC( rc );

    if( !mspOtpRead.data & 1 ) {
        BDBG_LOG( ( "No enforcement of maximum Pcie0 memory window size." ) );
    }
    else {
        maxWinSizeEnfored = true;
        BDBG_LOG( ( "Enforcement of maximum Pcie0 memory window size is enabled, exclusive mode only. MWIN_RESTRICT_ENABLE shall be 1. " ) );
    }

    NEXUS_Memory_GetDefaultAllocationSettings( &memSetting );
    memSetting.alignment = 32;

    rc = NEXUS_Memory_Allocate( sizeof( NEXUS_SecurityPciEMaxWindowSizeSettings ), &memSetting,
                                ( void * ) &maxWinSize );
    SECURITY_CHECK_RC( rc );

    if( maxWinSizeEnfored ) {
        NEXUS_Security_GetDefaultPciEMaxWindowSizeSettings( maxWinSize );

        /* Fill in the signed command data ... */
        BKNI_Memcpy( maxWinSize->signedCommand, signedCommand, sizeof( maxWinSize->signedCommand ) );

        maxWinSize->signingAuthority = NEXUS_SigningAuthority_eBroadcom;
        maxWinSize->signedCommandLength = sizeof( maxWinSize->signedCommand );

        rc = NEXUS_Security_SetPciEMaxWindowSize( maxWinSize );
        SECURITY_CHECK_RC( rc );    /* Commenting out to allow it proceed to the next if the settings fail. */
    }

    /* Check size is within the above signed command specified max window size. */
    /* if the MSP is not enfored, and the above call is not made or failed, we expect it can be any size. */
    if( size /* <= maxPcieWinSize */  ) {

        if( argc != 3 ) {
            printf( "\nusage: %s <memory window start offset> <window size> \n"
                    "   e.g., nexus %s 0x80000000 0x2000000 \n\n", argv[0], argv[0] );
            goto exit;
        }

        offset = _getInt( argv[1] );
        size   = _getInt( argv[2] );

        BDBG_LOG(( "\nDo you want to input " \
                " offset [" BDBG_UINT64_FMT "]\n" \
                " size [0x%08X]\n" \
                " y to proceed.\n\n", BDBG_UINT64_ARG(offset), size));

        while( ( ch=getchar( ) ) == '\n'){ }
        if( ch != 'y' && ch != 'Y' ) { goto exit; }

        rc = NEXUS_Security_SetPciERestrictedRange( offset, size, 0 );
        SECURITY_CHECK_RC( rc );

        BDBG_LOG(( "\nPCIE0 can now access to memory " \
                " offset [" BDBG_UINT64_FMT "]\n" \
                " size [0x%08X]\n" \
                " \n", BDBG_UINT64_ARG(offset), size));
    }
    else {
        rc = NEXUS_INVALID_PARAMETER;
        SECURITY_CHECK_RC( rc );
    }

  exit:

    if( maxWinSize ) { NEXUS_Memory_Free( maxWinSize ); }
    if( addr ) { NEXUS_Memory_Free( addr ); }

    securityUtil_PlatformUnInit(  );

    return rc;
}

#else /* NEXUS_HAS_SECURITY */

#    include <stdio.h>
int main(
    void )
{
    printf( "This application is not supported on this platform!\n" );
    return -1;
}

#endif
