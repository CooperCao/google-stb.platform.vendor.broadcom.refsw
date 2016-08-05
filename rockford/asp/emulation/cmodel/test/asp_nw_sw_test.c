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

#include <stdio.h>
#include <assert.h>
#include "asp_nw_sw_api.h"

struct
{
    int sockfd; /* socketFd on which to send this payload out. */
} dummyAppCtx;

static int feedRawFrame(
    void *pContext,
    int param,
    void *pRawFrameBuf,
    size_t rawFrameLength
    )
{
    int i;
    uint8_t *pRawFrame;

    pRawFrame = pRawFrameBuf;
    assert(pContext == &dummyAppCtx);
    assert(dummyAppCtx.sockfd == 10);
    assert(param == 1);
    assert(rawFrameLength == 8);
    for (i=0; i < (int)rawFrameLength; i++)
        assert(pRawFrame[i] == i);
    fprintf(stdout, "%s: rawFrameLength=%zu\n", __FUNCTION__, rawFrameLength);
    return (int)rawFrameLength;
}

extern int ASP_NwSwFlow_ProcessIo_test(int flowId);
int main(void)
{
    int rc;
    ASP_NwSwFlowHandle hNwSwFlow0;
    ASP_NwSwFlowHandle hNwSwFlow1;
    ASP_FeedRawFrameCallbackDesc feedRawFrameDesc;
    ASP_SocketInfo socketInfo;
    uint32_t flowId;

    ASP_NwSw_Init();

    {
        dummyAppCtx.sockfd = 10;
        feedRawFrameDesc.callback = feedRawFrame;
        feedRawFrameDesc.pContext = &dummyAppCtx;
        feedRawFrameDesc.param = 1;
    }

    /* Open, Start, Stop, Close a channel. */
    {
        hNwSwFlow0 = ASP_NwSwFlow_Open(&socketInfo, &feedRawFrameDesc, &flowId);
        assert(hNwSwFlow0);
        assert(flowId == 0);

        rc = ASP_NwSwFlow_Start(hNwSwFlow0);
        assert(rc == 0);
        rc = ASP_NwSwFlow_Start(hNwSwFlow0);
        assert(rc == -1);

        ASP_NwSwFlow_ProcessIo_test(flowId);

        rc = ASP_NwSwFlow_Stop(hNwSwFlow0);
        assert(rc == 0);
        ASP_NwSwFlow_Close(hNwSwFlow0);
    }

    /* Open, Start, Stop, Close 2 flows. */
    {
        hNwSwFlow0 = ASP_NwSwFlow_Open(&socketInfo, &feedRawFrameDesc, &flowId);
        assert(hNwSwFlow0);
        assert(flowId == 0);

        hNwSwFlow1 = ASP_NwSwFlow_Open(&socketInfo, &feedRawFrameDesc, &flowId);
        assert(hNwSwFlow1);
        assert(flowId == 1);

        rc = ASP_NwSwFlow_Start(hNwSwFlow0);
        assert(rc == 0);
        rc = ASP_NwSwFlow_Start(hNwSwFlow0);
        assert(rc == -1);

        ASP_NwSwFlow_ProcessIo_test(flowId);

        rc = ASP_NwSwFlow_Stop(hNwSwFlow0);
        assert(rc == 0);
        rc = ASP_NwSwFlow_Stop(hNwSwFlow1);
        assert(rc == 0);
        ASP_NwSwFlow_Close(hNwSwFlow0);
        ASP_NwSwFlow_Close(hNwSwFlow1);
    }

    ASP_NwSw_UnInit();
    return 0;
}
