/******************************************************************************
 *    (c)2008-2012 Broadcom Corporation
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
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
******************************************************************************/
#include "nexus_platform.h"
#include "nexus_parser_band.h"
#include "nexus_playpump.h"
#include "nexus_mpod.h"
#include "bstd.h"
#include "bkni.h"
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_MpodHandle mpod;
    NEXUS_MpodInputHandle mpodInput, mpodInputPb;
    NEXUS_MpodOpenSettings mpodOpenSettings;
    NEXUS_MpodInputSettings mpodInputSettings;
    int rc;

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;

    /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
    NEXUS_Platform_GetStreamerInputBand(0, &inputBand);

    /* Map a parser band to the streamer input band. */
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    parserBandSettings.sourceTypeSettings.inputBand = inputBand;
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_Mpod_GetDefaultOpenSettings(&mpodOpenSettings);
    mpodOpenSettings.mpodMode = NEXUS_MpodMode_eSpod;
    mpodOpenSettings.enableBandAssociation = true;
    mpodOpenSettings.bandType = NEXUS_MpodBandType_eParserBand;
    mpodOpenSettings.band.parserBand = NEXUS_ParserBand_e0;
    mpod = NEXUS_Mpod_Open(0, &mpodOpenSettings); /* Index always 0 */
    BDBG_ASSERT(mpod);

    NEXUS_MpodInput_GetDefaultSettings(&mpodInputSettings);
    mpodInputSettings.bandType = NEXUS_MpodBandType_eParserBand;
    mpodInputSettings.band.parserBand = NEXUS_ParserBand_e0;
    mpodInputSettings.allPass = true;
    mpodInput = NEXUS_MpodInput_Open(mpod, &mpodInputSettings);
    BDBG_ASSERT(mpodInput);

    printf("Press ENTER to route playback to Mpod....\n");
    getchar();
    
    playpump = NEXUS_Playpump_Open(1, NULL);

    NEXUS_MpodInput_GetDefaultSettings(&mpodInputSettings);
    mpodInputSettings.bandType = NEXUS_MpodBandType_ePlaypump;
    mpodInputSettings.band.playpump = playpump;
    mpodInputPb = NEXUS_MpodInput_Open(mpod, &mpodInputSettings);
    BDBG_ASSERT(mpodInputPb);

    printf("Press ENTER to end program....\n");
    getchar();
    
    NEXUS_MpodInput_Close(mpodInputPb);
    NEXUS_Playpump_Close(playpump);
    NEXUS_MpodInput_Close(mpodInput);
    NEXUS_Mpod_Close(mpod);
    NEXUS_Platform_Uninit();
    
    return 0;
}
