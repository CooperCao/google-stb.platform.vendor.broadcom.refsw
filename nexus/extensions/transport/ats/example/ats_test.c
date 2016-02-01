/******************************************************************************
 *    (c)2008-2015 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *
 * $brcm_Workfile: $
 * $brcm_Revision:  $
 * $brcm_Date:  $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 *****************************************************************************/

#include "nexus_platform.h"
#include "nexus_ats.h"

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "bchp_xpt_fe.h"

BDBG_MODULE( ats_test );

int main(void)
{
    uint32_t firstAts, secondAts, atsAfterReset, userAts;
    NEXUS_PlatformSettings platformSettings;

    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    /* Get an ATS, wait 10 mS, then get another. The second should be ahead of the first. */
    NEXUS_Ats_SetInternal();    /* Optional. HW default is internal. */
    NEXUS_Ats_ResetCounter();
    firstAts = NEXUS_Ats_GetBinaryTimestamp();
    BKNI_Sleep( 10 );
    secondAts = NEXUS_Ats_GetBinaryTimestamp();
    BDBG_ASSERT( secondAts > firstAts );

    /* Reset the counter and get another ATS. This one should be less than the secondAts, above. */
    NEXUS_Ats_ResetCounter();
    atsAfterReset = NEXUS_Ats_GetBinaryTimestamp();
    BDBG_ASSERT( secondAts > atsAfterReset );

    /* Write our own timestamp, then read it back. The upper half of the two values should match
       since we're reading back almost immediately.
    */
    #define TEST_AST_VALUE  0x12345000
    #define TEST_AST_MASK   0xFFFFF000
    NEXUS_Ats_ResetCounter();
    NEXUS_Ats_SetBinaryTimestamp( TEST_AST_VALUE );
    userAts = NEXUS_Ats_GetBinaryTimestamp();
    printf("userAts 0x%08lX\n", (unsigned long) userAts );
    BDBG_ASSERT( TEST_AST_VALUE == (userAts & TEST_AST_MASK) );

    printf( "ATS tests passed\n" );
    NEXUS_Platform_Uninit();
    return 0;
}
