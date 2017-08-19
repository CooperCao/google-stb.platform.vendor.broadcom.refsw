/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 **************************************************************************/
#if NEXUS_HAS_FRONTEND
#include "nxclient.h"
#include "nexus_frontend.h"
#include "nexus_parser_band.h"
#include "nexus_simple_video_decoder.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

BDBG_MODULE(tune_qam);
#include "nxapp_prompt.inc"

/* the following define the input file and its characteristics -- these will vary by input file */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11

int main(void)
{
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    NEXUS_FrontendHandle frontend;
    NEXUS_FrontendAcquireSettings frontendAcquireSettings;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;
    NEXUS_SimpleVideoDecoderStartSettings videoProgram;
    NEXUS_SimpleStcChannelHandle stcChannel;
    NEXUS_Error rc;
    unsigned freq = 765;
    
    rc = NxClient_Join(NULL);
    if (rc) return -1;

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.simpleVideoDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    if (rc) return BERR_TRACE(rc);

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);
    BDBG_ASSERT(videoDecoder);
    
    stcChannel = NEXUS_SimpleStcChannel_Create(NULL);
    
    parserBand = NEXUS_ParserBand_Open(NEXUS_ANY_ID);
    
    NEXUS_Frontend_GetDefaultAcquireSettings(&frontendAcquireSettings);
    frontendAcquireSettings.capabilities.qam = true;
    frontend = NEXUS_Frontend_Acquire(&frontendAcquireSettings);
    if (!frontend) {
        BDBG_WRN(("Unable to find QAM-capable frontend"));
        return -1;
    }
    
    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = freq * 1000000;
    qamSettings.mode = NEXUS_FrontendQamMode_e64;
    qamSettings.symbolRate = 5056900;
    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    if (userParams.isMtsif) {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend); /* NEXUS_Frontend_TuneXyz() will connect this frontend to this parser band */
    }
    else {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;  /* Platform initializes this to input band */
    }
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    rc = NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);
    BDBG_ASSERT(!rc);

    rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
    BDBG_ASSERT(!rc);
            
    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    if (rc) return BERR_TRACE(rc);
    
    NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.settings.pidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);
    videoProgram.settings.codec = VIDEO_CODEC;

    if (videoProgram.settings.pidChannel) {
        NEXUS_SimpleVideoDecoder_SetStcChannel(videoDecoder, stcChannel);
    }
    if (videoProgram.settings.pidChannel) {
        NEXUS_SimpleVideoDecoder_Start(videoDecoder, &videoProgram);
    }
    
    nxapp_prompt("exit");

    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return 0;
}
#else
#include <stdio.h>
int main(void)
{
    printf("This application is not supported on this platform!\n");
    return 0;
}
#endif
