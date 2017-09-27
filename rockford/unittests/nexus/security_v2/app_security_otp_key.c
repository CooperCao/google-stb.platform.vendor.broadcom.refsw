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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "nexus_platform.h"
#include "nexus_security_datatypes.h"
#include "nexus_security.h"
#include "nexus_otp_key.h"

#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) {             \
    int x_offset;                                                       \
    printf("[%s][%d]", description_txt, in_size );                      \
    for( x_offset = 0; x_offset < (int)(in_size); x_offset++ )          \
    {                                                                   \
        if( x_offset%16 == 0 ) { printf("\n"); }                        \
        printf("%02X ", in_ptr[x_offset] );                             \
    }                                                                   \
    printf("\n");                                                       \
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_OtpKeyInfo otpKeyInfo;
    NEXUS_Error rc;
    unsigned i;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    BKNI_Sleep(1000);

    if( argc == 1  ) {
        /* print info for all keys */

        for( i = 0; i <= 7; i++ ) {

            BKNI_Memset( &otpKeyInfo, 0,sizeof(otpKeyInfo) );
            rc = NEXUS_OtpKey_GetInfo( i, &otpKeyInfo );
            if( rc == NEXUS_SUCCESS )
            {
                DEBUG_PRINT_ARRAY("hash", (int)sizeof(otpKeyInfo.hash), otpKeyInfo.hash );
                DEBUG_PRINT_ARRAY("id  ", (int)sizeof(otpKeyInfo.id), otpKeyInfo.id );
                printf( " blackBoxId[0x%X]\n",     otpKeyInfo.blackBoxId  );
                printf( " caKeyLadderAllow[%s]\n", otpKeyInfo.caKeyLadderAllow?"true":"false"  );
                printf( " cpKeyLadderAllow[%s]\n", otpKeyInfo.cpKeyLadderAllow?"true":"false"  );
                printf( " gp1KeyLadderAllow[%s]\n",   otpKeyInfo.gp1KeyLadderAllow?"true":"false"  );
                printf( " gp2KeyLadderAllow[%s]\n",   otpKeyInfo.gp2KeyLadderAllow?"true":"false"  );
                printf( " customerMode[0x%X]\n",   otpKeyInfo.customerMode  );
            }
            else
            {
                printf( "Not able to acccess OTP %d Key info\n", i );
            }
        }
    }
    else{

        i = atoi( argv[1] );

        rc = NEXUS_OtpKey_GetInfo( i, &otpKeyInfo );
        if( rc == NEXUS_SUCCESS )
        {
            DEBUG_PRINT_ARRAY("hash", (int)sizeof(otpKeyInfo.hash), otpKeyInfo.hash );
            DEBUG_PRINT_ARRAY("id  ", (int)sizeof(otpKeyInfo.id), otpKeyInfo.id );
            printf( " blackBoxId[0x%X]\n",     otpKeyInfo.blackBoxId  );
            printf( " caKeyLadderAllow[%s]\n", otpKeyInfo.caKeyLadderAllow?"true":"false"  );
            printf( " cpKeyLadderAllow[%s]\n", otpKeyInfo.cpKeyLadderAllow?"true":"false"  );
            printf( " gp1KeyLadderAllow[%s]\n",   otpKeyInfo.gp1KeyLadderAllow?"true":"false"  );
            printf( " gp2KeyLadderAllow[%s]\n",   otpKeyInfo.gp2KeyLadderAllow?"true":"false"  );
            printf( " customerMode[0x%X]\n",   otpKeyInfo.customerMode  );
        }
        else
        {
            printf( "Not able to acccess OTP %d Key info\n", i );
        }
    }

    NEXUS_Platform_Uninit();

    return 0;
}

#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return -1;
}
#endif
