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

/* Nexus example app: OTP/Key field program */

#include <stdio.h>
#include "nexus_platform.h"
#if NEXUS_SECURITY_API_VERSION == 2
int main ( void )
{
    printf ("otpprogkey unsupported\n");
}
#else

#include "nexus_prog_otp_key.h"
#include "nexus_keyladder.h"

BDBG_MODULE ( OTPPROGKEG );

static int Security_AllocateVkl ( NEXUS_SecurityVirtualKeyladderID * vkl, NEXUS_VirtualKeyLadderHandle * pVklHandle )
{
    NEXUS_SecurityVKLSettings vklSettings;
    NEXUS_VirtualKeyLadderInfo vklInfo;
    NEXUS_VirtualKeyLadderHandle vklHandle;

    BDBG_ASSERT ( vkl );

    NEXUS_Security_GetDefaultVKLSettings ( &vklSettings );

    /* For pre Zeus 3.0, please set vklSettings.custSubMode */

    vklHandle = NEXUS_Security_AllocateVKL ( &vklSettings );

    if ( !vklHandle )
    {
        printf ( "\nAllocate VKL failed \n" );
        return 1;
    }

    NEXUS_Security_GetVKLInfo ( vklHandle, &vklInfo );
    printf ( "\nVKL Handle %p is allocated for VKL#%d\n", ( void * ) vklHandle, vklInfo.vkl & 0x7F );

    /* For Zeus 4.2 or later
     * if ((vklInfo.vkl & 0x7F ) >= NEXUS_SecurityVirtualKeyLadderID_eMax)
     * {
     * printf ( "\nAllocate VKL failed with invalid VKL Id.\n" );
     * return 1;
     * }
     */

    *vkl = vklInfo.vkl;
    *pVklHandle = vklHandle;

    return 0;
}

int main ( void )
{
    NEXUS_Error     rc = BERR_SUCCESS;
    NEXUS_SecurityCapabilities securityCapabilities;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformStatus platStatus;
    NEXUS_ProgramOtpKey otpKey;
    NEXUS_SecurityVirtualKeyladderID vklId;
    NEXUS_VirtualKeyLadderHandle vklHandle;

    /* Setup key data here. */
    uint8_t         keyData[NEXUS_MAX_OTP_KEY_LENGTH] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    NEXUS_Platform_GetDefaultSettings ( &platformSettings );
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init ( &platformSettings );

    NEXUS_Platform_GetConfiguration ( &platformConfig );

    NEXUS_Security_GetDefaultProgramOtpKey ( &otpKey );

    /* request for an VKL to use */
    if ( Security_AllocateVkl ( &vklId, &vklHandle ) )
    {
        printf ( "\nAllocate VKL failed.\n" );
        goto EXIT;
    }

    otpKey.keyType = NEXUS_OtpKeyType_eA;
    otpKey.virtualKeyLadderID = vklId;
    otpKey.keyDataSize = sizeof ( keyData );
    otpKey.keyLayer = NEXUS_SecurityKeySource_eKey3;

    BKNI_Memcpy ( &otpKey.keyData, &keyData, otpKey.keyDataSize );

    rc = NEXUS_Security_ProgramOtpKey ( &otpKey );
    if ( rc != NEXUS_SUCCESS )
    {
        printf ( "Error: BHSM_ProgOTPKey() failed. Error code: %x\n", rc );
        goto EXIT;
    }

    NEXUS_GetSecurityCapabilities ( &securityCapabilities );
    NEXUS_Platform_GetStatus ( &platStatus );
    printf ( "Chip Family........[%X]\n", platStatus.familyId );
    printf ( "Chip Version.......[%X]\n", platStatus.chipRevision );
    printf ( "Chip Id............[%X]\n", platStatus.chipId );
    printf ( "Zeus Version.......[%d.%d]\n", NEXUS_ZEUS_VERSION_MAJOR ( securityCapabilities.version.zeus ),
             NEXUS_ZEUS_VERSION_MINOR ( securityCapabilities.version.zeus ) );
    printf ( "Firmware Version...[%d.%d]\n", ( securityCapabilities.version.firmware >> 16 ),
             ( securityCapabilities.version.firmware & 0x0000FFFF ) );

  EXIT:

    NEXUS_Security_FreeVKL ( vklHandle );

    NEXUS_Platform_Uninit (  );

    return rc;
}
#endif
