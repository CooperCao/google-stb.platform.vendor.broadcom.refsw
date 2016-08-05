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
#include "asp_xpt_api.h"

int testXptMcpb(void)
{
    int rc;
    uint32_t channelNumber;
    ASP_XptMcpbChannelHandle hXptMcpbChannel0;
    ASP_XptMcpbChannelHandle hXptMcpbChannel1;

    ASP_Xpt_Init();

    /* Open, Start, Stop, Close a channel. */
    {
        hXptMcpbChannel0 = ASP_XptMcpbChannel_Open();
        assert(hXptMcpbChannel0);
        rc = ASP_XptMcpbChannel_GetChannelNumber(hXptMcpbChannel0, &channelNumber);
        assert(rc == 0);
        assert(channelNumber == 0);
        rc = ASP_XptMcpbChannel_Start(hXptMcpbChannel0);
        assert(rc == 0);
        rc = ASP_XptMcpbChannel_Start(hXptMcpbChannel0);
        assert(rc == -1);
        rc = ASP_XptMcpbChannel_Stop(hXptMcpbChannel0);
        assert(rc == 0);
        ASP_XptMcpbChannel_Close(hXptMcpbChannel0);
    }

    /* Open, Start, Stop, Close 2 channels. */
    {
        hXptMcpbChannel0 = ASP_XptMcpbChannel_Open();
        assert(hXptMcpbChannel0);
        hXptMcpbChannel1 = ASP_XptMcpbChannel_Open();
        assert(hXptMcpbChannel1);

        rc = ASP_XptMcpbChannel_GetChannelNumber(hXptMcpbChannel0, &channelNumber);
        assert(rc == 0);
        assert(channelNumber == 0);

        rc = ASP_XptMcpbChannel_GetChannelNumber(hXptMcpbChannel1, &channelNumber);
        assert(rc == 0);
        assert(channelNumber == 1);

        rc = ASP_XptMcpbChannel_Start(hXptMcpbChannel0);
        assert(rc == 0);
        rc = ASP_XptMcpbChannel_Start(hXptMcpbChannel0);
        assert(rc == -1);
        rc = ASP_XptMcpbChannel_Stop(hXptMcpbChannel0);
        assert(rc == 0);
        ASP_XptMcpbChannel_Close(hXptMcpbChannel0);
        ASP_XptMcpbChannel_Close(hXptMcpbChannel1);
    }

    return 0;
}

struct
{
    int sockfd; /* socketFd on which to send this payload out. */
} dummyAppCtx;

static int feedPayload(
    void *pContext,
    int param,
    void *pPayloadBuf,
    size_t payloadLength
    )
{
    int i;
    uint8_t *pPayload;

    pPayload = pPayloadBuf;
    assert(pContext == &dummyAppCtx);
    assert(dummyAppCtx.sockfd == 10);
    assert(param == 1);
    assert(payloadLength == 8);
    for (i=0; i < (int)payloadLength; i++)
        assert(pPayload[i] == i);
    fprintf(stdout, "%s: payloadLength=%zu\n", __FUNCTION__, payloadLength);
    return (int)payloadLength;
}

extern int ASP_XptM2mDmaChannel_ProcessIo_test(int channelNumber);
int testXptM2mDma(void)
{
    int rc;
    uint32_t channelNumber;
    ASP_XptM2mDmaChannelHandle hXptM2mDmaChannel0;
    ASP_XptM2mDmaChannelHandle hXptM2mDmaChannel1;
    ASP_FeedPayloadCallbackDesc feedPayloadDesc;

    {
        dummyAppCtx.sockfd = 10;
        feedPayloadDesc.callback = feedPayload;
        feedPayloadDesc.pContext = &dummyAppCtx;
        feedPayloadDesc.param = 1;
    }

    /* Open, Start, Stop, Close a channel. */
    {
        hXptM2mDmaChannel0 = ASP_XptM2mDmaChannel_Open(&feedPayloadDesc);
        assert(hXptM2mDmaChannel0);
        rc = ASP_XptM2mDmaChannel_GetChannelNumber(hXptM2mDmaChannel0, &channelNumber);
        assert(rc == 0);
        assert(channelNumber == 0);
        rc = ASP_XptM2mDmaChannel_Start(hXptM2mDmaChannel0);
        assert(rc == 0);
        rc = ASP_XptM2mDmaChannel_Start(hXptM2mDmaChannel0);
        assert(rc == -1);

        ASP_XptM2mDmaChannel_ProcessIo_test(channelNumber);

        rc = ASP_XptM2mDmaChannel_Stop(hXptM2mDmaChannel0);
        assert(rc == 0);
        ASP_XptM2mDmaChannel_Close(hXptM2mDmaChannel0);
    }

    /* Open, Start, Stop, Close 2 channels. */
    {
        hXptM2mDmaChannel0 = ASP_XptM2mDmaChannel_Open(&feedPayloadDesc);
        assert(hXptM2mDmaChannel0);
        hXptM2mDmaChannel1 = ASP_XptM2mDmaChannel_Open(&feedPayloadDesc);
        assert(hXptM2mDmaChannel1);

        rc = ASP_XptM2mDmaChannel_GetChannelNumber(hXptM2mDmaChannel0, &channelNumber);
        assert(rc == 0);
        assert(channelNumber == 0);

        rc = ASP_XptM2mDmaChannel_GetChannelNumber(hXptM2mDmaChannel1, &channelNumber);
        assert(rc == 0);
        assert(channelNumber == 1);

        rc = ASP_XptM2mDmaChannel_Start(hXptM2mDmaChannel0);
        assert(rc == 0);
        rc = ASP_XptM2mDmaChannel_Start(hXptM2mDmaChannel0);
        assert(rc == -1);

        ASP_XptM2mDmaChannel_ProcessIo_test(channelNumber);

        rc = ASP_XptM2mDmaChannel_Stop(hXptM2mDmaChannel0);
        assert(rc == 0);
        ASP_XptM2mDmaChannel_Close(hXptM2mDmaChannel0);
        ASP_XptM2mDmaChannel_Close(hXptM2mDmaChannel1);
    }

    return 0;
}

int testXptRave(void)
{
    int rc;
    uint32_t channelNumber;
    ASP_XptRaveChannelHandle hXptRaveChannel0;
    ASP_XptRaveChannelHandle hXptRaveChannel1;

    /* Open, Start, Stop, Close a channel. */
    {
        hXptRaveChannel0 = ASP_XptRaveChannel_Open();
        assert(hXptRaveChannel0);
        rc = ASP_XptRaveChannel_GetChannelNumber(hXptRaveChannel0, &channelNumber);
        assert(rc == 0);
        assert(channelNumber == 0);
        rc = ASP_XptRaveChannel_Start(hXptRaveChannel0);
        assert(rc == 0);
        rc = ASP_XptRaveChannel_Start(hXptRaveChannel0);
        assert(rc == -1);
        rc = ASP_XptRaveChannel_Stop(hXptRaveChannel0);
        assert(rc == 0);
        ASP_XptRaveChannel_Close(hXptRaveChannel0);
    }

    /* Open, Start, Stop, Close 2 channels. */
    {
        hXptRaveChannel0 = ASP_XptRaveChannel_Open();
        assert(hXptRaveChannel0);
        hXptRaveChannel1 = ASP_XptRaveChannel_Open();
        assert(hXptRaveChannel1);

        rc = ASP_XptRaveChannel_GetChannelNumber(hXptRaveChannel0, &channelNumber);
        assert(rc == 0);
        assert(channelNumber == 0);

        rc = ASP_XptRaveChannel_GetChannelNumber(hXptRaveChannel1, &channelNumber);
        assert(rc == 0);
        assert(channelNumber == 1);

        rc = ASP_XptRaveChannel_Start(hXptRaveChannel0);
        assert(rc == 0);
        rc = ASP_XptRaveChannel_Start(hXptRaveChannel0);
        assert(rc == -1);
        rc = ASP_XptRaveChannel_Stop(hXptRaveChannel0);
        assert(rc == 0);
        ASP_XptRaveChannel_Close(hXptRaveChannel0);
        ASP_XptRaveChannel_Close(hXptRaveChannel1);
    }

    return 0;
}

int main(void)
{

    ASP_Xpt_Init();

    testXptMcpb();

    testXptM2mDma();

    testXptRave();

    ASP_Xpt_UnInit();

    return 0;
}
