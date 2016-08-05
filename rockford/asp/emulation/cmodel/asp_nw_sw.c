/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include <sys/types.h>
#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "asp_nw_sw_api.h"

#define ASP_NUM_NW_SW_FLOWS   32

ASP_NwSwFlowHandle  ghNwSwFlowHandle = NULL; /* This global pointer is maintained so that asp simulator can call
                                                ASP_NwSwFlow_FeedRawFrameFromAspSimulator api without knowing sw handle.*/

static ASP_CtxEntry g_nwSwCtxList[ASP_NUM_NW_SW_FLOWS];
/**
Summary:
Initialize Network Switch Emulation Module.
**/
int ASP_NwSw_Init(void)
{
    /* Do any one time initialization tasks here. */
    memset(&g_nwSwCtxList, 0, sizeof(g_nwSwCtxList));
    return 0;
}

/**
Summary:
Un-Initialize Network Switch Emulation Module.
**/
void ASP_NwSw_UnInit(void)
{
    /* Do any un-init work here. */
    return;
}

typedef struct ASP_NwSwFlow
{
    uint32_t                        flowId;    /* aka broadcom tag. */
    ASP_SocketInfo                  socketInfo;
    ASP_FeedRawFrameCallbackDesc    rawFrameCallbackDesc;
    bool                            started;
} ASP_NwSwFlow;

/**
Summary:
Open & add Flow Details to Network Switch so that it redirect the incoming traffic to ASP Port.

Note: this Network Switch will not redirect the incoming packets to ASP CModel until
ASP_NwSwFlow_Startis called. This will be called as the last step in the Offload Sequence.

Output:
pFlowId: A unique flow-id that is used to Remove, Enable, Disable that flow.
**/
ASP_NwSwFlowHandle ASP_NwSwFlow_Open(
    ASP_SocketInfo                  *pSocketInfo,
    ASP_FeedRawFrameCallbackDesc    *pRawFrameCallbackDesc,    /* Callback Descriptor for allowing NwSw CModel to send out the Raw Frames back to DUT. */
    uint32_t                        *pFlowId                   /* Output: unique flowId (aka broadcom tag). */
    )
{
    int                 flowId;
    ASP_NwSwFlowHandle  hFlow;

    assert(pRawFrameCallbackDesc);
    assert(pSocketInfo);
    assert(pFlowId);

    /* Find the next free NW SW CFP Entry. */
    flowId = ASP_Utils_AllocFreeCtx(g_nwSwCtxList, ASP_NUM_NW_SW_FLOWS, pRawFrameCallbackDesc);
    if (flowId < 0) { fprintf(stderr, "%s: Failed to find a free NW SW CFP Entry\n", __FUNCTION__); return NULL; }

    hFlow = calloc(1, sizeof(ASP_NwSwFlow));
    assert(hFlow);

    hFlow->flowId = flowId;
    hFlow->socketInfo = *pSocketInfo;
    hFlow->rawFrameCallbackDesc = *pRawFrameCallbackDesc;
    *pFlowId = flowId;

    ghNwSwFlowHandle = hFlow;

    return hFlow;
}

/**
Summary:
Remove a previously added flow.
**/
int ASP_NwSwFlow_Close(
    ASP_NwSwFlowHandle hFlow
    )
{
    assert(hFlow);

    ASP_Utils_FreeCtx(g_nwSwCtxList, hFlow->flowId, ASP_NUM_NW_SW_FLOWS);
    if (hFlow) free(hFlow);

    return 0;
}

/**
Summary:
Enable a particular flow so that N/W Switch can now start forwarding incoming raw frames to ASP CModel.
**/
int ASP_NwSwFlow_Start(
    ASP_NwSwFlowHandle hFlow
    )
{

    assert(hFlow);
    if (!hFlow->started)
    {
        hFlow->started = true;
    }
    return (0);
}

/**
Summary:
Disable a particular flow so that N/W Switch can now stop forwarding incoming raw frames to ASP CModel.
**/
int ASP_NwSwFlow_Stop(
    ASP_NwSwFlowHandle hFlow
    )
{
    assert(hFlow);
    hFlow->started = false;
    return 0;
}

/**
Summary:
API to feed Raw Ethernet Frames to the Network Switch CModel.
**/
int ASP_NwSwFlow_FeedRawFrame(
    ASP_NwSwFlowHandle hFlow,
    void *pRawFrame,
    size_t frameLength
    )
{
    assert(hFlow);
    assert(pRawFrame);
    assert(frameLength);
    /* TODO: Call NwSw CModel API to feed the raw frame (from the network peer) to the NwSw CModel. */
    return 0;
}

/**
Summary:
API to feed Raw Ethernet Frames from asp simulator. This is a
private function between asp simulator and NwSw.
**/
int ASP_NwSwFlow_FeedRawFrameFromAspSimulator(
    void *pRawFrame,
    size_t frameLength
    )
{
    int rc = 0;

    ghNwSwFlowHandle->rawFrameCallbackDesc.callback(
                                            ghNwSwFlowHandle->rawFrameCallbackDesc.pContext,
                                            ghNwSwFlowHandle->rawFrameCallbackDesc.param,
                                            pRawFrame,
                                            frameLength
                                            );

    return rc;
}

/**
Summary:
Dummy function to run the IO loop calls feedRawFrame callback.
**/
void ASP_NwSwFlow_ProcessIo_test(
    uint32_t flowId
    )
{
    ASP_FeedRawFrameCallbackDesc *pFeedRawFrameDesc;

    assert(flowId < ASP_NUM_NW_SW_FLOWS );
    pFeedRawFrameDesc = ASP_Utils_FindCtxUsingCtxNumber(g_nwSwCtxList, flowId, ASP_NUM_NW_SW_FLOWS);
    assert(pFeedRawFrameDesc);

    /* invoke the callback & provide dummy rawFrame data to send out. */
    {
#define RAW_FRAME_LEN 8
        int i;
        int rc;
        uint8_t rawFrame[RAW_FRAME_LEN];

        for (i=0; i < RAW_FRAME_LEN; i++)
            rawFrame[i] = i;
        rc = pFeedRawFrameDesc->callback(pFeedRawFrameDesc->pContext, pFeedRawFrameDesc->param, rawFrame, RAW_FRAME_LEN);
        assert(rc == RAW_FRAME_LEN);
    }
}
