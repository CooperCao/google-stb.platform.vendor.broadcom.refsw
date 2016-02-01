/***************************************************************************
 *     (c)2007-2009 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 **************************************************************************/

#include <stdio.h>
#include "bstd.h"
#include "bkni.h"
#include "nexus_platform.h"
#include "nexus_input_band.h"
#include "nexus_parser_band.h"
#include "nexus_pid_channel_scrambling.h"

BDBG_MODULE(parser_band);

int main(void)
{
    NEXUS_Error rc;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PidChannelHandle pidChannel;
    NEXUS_PidChannelScramblingSettings scramblingSettings;
    NEXUS_PidChannelScramblingStatus status;
    unsigned short pid = 0x31;
    unsigned i;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    rc = NEXUS_Platform_Init(NULL);
    BDBG_ASSERT(!rc);

    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    BDBG_ASSERT(!rc);

    /* must open one to enable parser band */
    pidChannel = NEXUS_PidChannel_Open(parserBand, pid, NULL);
    BDBG_ASSERT(pidChannel);

    /**
    This code shows a standard scrambling check algorithm.
    **/

    NEXUS_PidChannel_GetDefaultScramblingSettings(&scramblingSettings);
    scramblingSettings.pusiOnly = false; /* the app can decide is they want to check only packets with the PUSI bit. */

    rc = NEXUS_PidChannel_StartScramblingCheck(pidChannel, &scramblingSettings);
    BDBG_ASSERT(!rc);

    /* the app must poll the scrambling status. there is no interrupt.
    in a real app, we shouldn't sleep unless we are in a dedicated thread. this can be done asynchronously with other
    channel change processing.
    this app defines that 2 seconds is the longest time to wait */
    for (i=0;i<20;i++) {
        rc = NEXUS_PidChannel_GetScramblingStatus(pidChannel, &status);
        BDBG_ASSERT(!rc);

        if (i >= 10 && !status.pidExists) {
            /* if we've not seen any pid after 1 second, then the pid doesn't exist */
            break;
        }
        if (status.pidExists && status.scrambled) {
            /* if we ever see a scrambled pid, then we know what we have. */
            break;
        }

        BKNI_Sleep(100);
    }
    if (!status.pidExists) {
        printf("program does not exist\n");
    }
    else if (status.scrambled) {
        printf("program exists and is scrambled\n");
    }
    else {
        /* we saw a pid, but it wasn't scrambled. there is no way to prove a universal negative, but
        according to this app's requirements, it's reasonable to now believe that the program is not scrambled. */
        printf("program exists and is not scrambled\n");
    }

    NEXUS_PidChannel_StopScramblingCheck(pidChannel);
    NEXUS_PidChannel_Close(pidChannel);
    NEXUS_Platform_Uninit();

    return 0;
}
