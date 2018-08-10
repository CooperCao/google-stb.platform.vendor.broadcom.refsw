/******************************************************************************
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
 *****************************************************************************/
#include "nxclient.h"
#include "nexus_hddvi_input.h"
#include "nexus_simple_video_decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bstd.h"
#include "bkni.h"

BDBG_MODULE(hddvi);
#include "nxapp_prompt.inc"

int main(int argc, char **argv)  {
    NxClient_JoinSettings joinSettings;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
    unsigned connectId;
    int rc;
    NEXUS_HdDviInputHandle hddvi;
    NEXUS_HdDviInputSettings settings;
    NEXUS_SimpleVideoDecoderHandle videoDecoder;

    BSTD_UNUSED(argc);
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) {
        printf("cannot join: %d\n", rc);
        return -1;
    }

    NxClient_GetDefaultAllocSettings(&allocSettings);
    allocSettings.surfaceClient = 1; /* surface client required for video window */
    allocSettings.simpleVideoDecoder = 1;
    rc = NxClient_Alloc(&allocSettings, &allocResults);
    BDBG_ASSERT(!rc);

    videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(allocResults.simpleVideoDecoder[0].id);

    NxClient_GetDefaultConnectSettings(&connectSettings);
    connectSettings.simpleVideoDecoder[0].id = allocResults.simpleVideoDecoder[0].id;
    connectSettings.simpleVideoDecoder[0].surfaceClientId = allocResults.surfaceClient[0].id;
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = 0; /* no decoder needed, just a window */
    connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = 0;
    connectSettings.simpleAudioDecoder.id = allocResults.simpleAudioDecoder.id;
    rc = NxClient_Connect(&connectSettings, &connectId);
    BDBG_ASSERT(!rc);

    /* open alias */
    NEXUS_HdDviInput_GetDefaultSettings(&settings);
    hddvi = NEXUS_HdDviInput_Open(NEXUS_ALIAS_ID + 0, &settings);
    BDBG_ASSERT(hddvi);

    rc = NEXUS_SimpleVideoDecoder_StartHdDviInput(videoDecoder, hddvi, NULL);
    BDBG_ASSERT(!rc);

    nxapp_prompt("exit");

    NEXUS_HdDviInput_Close(hddvi);
    NxClient_Disconnect(connectId);
    NxClient_Free(&allocResults);
    NxClient_Uninit();
    return rc;
}
