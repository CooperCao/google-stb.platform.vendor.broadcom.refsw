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

#if NEXUS_HAS_SECURITY && NEXUS_SECURITY_API_VERSION==2

#include "nexus_platform.h"
#include "nexus_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "nexus_otp_msp.h"

static int _getInt( char *p )
{
    int base = 10;

    if( !p ) {
        return BERR_TRACE(0);
    }

    if( p[0] == '0' && ( p[1] == 'x' || p[1] == 'X' ) ) {
        base = 16;
    };

    return (int)strtol( p, NULL, base );
}



int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_OtpMspWrite mspOtpWrite;
    unsigned index;
    NEXUS_Error rc;
    char ch;

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    BKNI_Memset( &mspOtpWrite, 0,sizeof(mspOtpWrite) );

    BKNI_Sleep( 500 ); /* let test pass through */
    if( argc != 4 ) {
        printf( "\nusage: %s <msp otp index> <data> <mask>\n"
                "   i.e., %s 201 0x1001 0xFFFF\n\n", argv[0], argv[0] );
        goto clean_up;
    }

    index             = _getInt( argv[1] );
    mspOtpWrite.data  = _getInt( argv[2] );
    mspOtpWrite.mask  = _getInt( argv[3] );

    printf( "\nDo you want to write to" \
            " OTP MSP [%d]:-\n" \
            " - data[0x%08X]\n" \
            " - mask[0x%08X]\n" \
            " y to proceed.\n\n", index, mspOtpWrite.data, mspOtpWrite.mask );

    while( ( ch=getchar( ) ) == '\n'){ }
    if( ch != 'y' && ch != 'Y' ) { goto clean_up; }


    rc = NEXUS_OtpMsp_Write( index, &mspOtpWrite );
    if( rc != NEXUS_SUCCESS ) {   BERR_TRACE(rc); goto clean_up; }

clean_up:

    NEXUS_Platform_Uninit();

    return 0;
}

#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported!\n");
    return -1;
}
#endif
