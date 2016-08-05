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
#include "asp_xpt_api.h"

#define ASP_NUM_XPT_MCPB_CHANNELS   32
#define ASP_NUM_XPT_M2MDMA_CHANNELS 32
#define ASP_NUM_XPT_RAVE_CHANNELS   32

static ASP_CtxEntry g_mcpbChannelList[ASP_NUM_XPT_MCPB_CHANNELS];
static ASP_CtxEntry g_m2mDmaChInUseList[ASP_NUM_XPT_M2MDMA_CHANNELS];
static ASP_CtxEntry g_raveChannelList[ASP_NUM_XPT_RAVE_CHANNELS];



ASP_XptMcpb_BuffInfo    gXptMcpbBuffInfo;

/**
Summary:
Initialize All CModels of XPT (MCPB, M2M DMA, RAVE, etc.)
**/
int ASP_Xpt_Init(void)
{
    /* Do any one time initialization tasks here. */
    memset(&g_mcpbChannelList, 0, sizeof(g_mcpbChannelList));
    memset(&g_m2mDmaChInUseList, 0, sizeof(g_mcpbChannelList));
    memset(&g_raveChannelList, 0, sizeof(g_raveChannelList));
    return 0;
}


/**
Summary:
Un-initialize All CModels of XPT (MCPB, M2M DMA, RAVE, etc.)
**/
void ASP_Xpt_UnInit(void)
{
    /* Do any un-init work here. */
    return;
}

/******** XPT MCPB implementation. ********/


typedef struct ASP_XptMcpbChannel
{
    int                     channelNumber;
    bool                    started;
} ASP_XptMcpbChannel;

/**
Summary:
Open a MCPB Channel that will be used to feed the AV Payloads (from App) to be streamed out.
**/
ASP_XptMcpbChannelHandle ASP_XptMcpbChannel_Open( void )
{
    int                         channelNumber;
    ASP_XptMcpbChannelHandle    hXptMcpbChannel;

    /* Find the next free MCPB Channel number. */
    channelNumber = ASP_Utils_AllocFreeCtx(g_mcpbChannelList, ASP_NUM_XPT_MCPB_CHANNELS, NULL);
    if (channelNumber < 0) { fprintf(stderr, "%s: Failed to find a free MCPB Channel\n", __FUNCTION__); return NULL; }

    hXptMcpbChannel = calloc(1, sizeof(ASP_XptMcpbChannel));
    assert(hXptMcpbChannel);

    /* This mimics Mcpb dram buffer where mcpb will put incoming av data for asp.*/
    memset(&gXptMcpbBuffInfo, 0, sizeof(ASP_XptMcpb_BuffInfo));

    hXptMcpbChannel->channelNumber = channelNumber;

    return (hXptMcpbChannel);
}

/**
Summary:
Remove a previously added flow.
**/
int ASP_XptMcpbChannel_Close(
    ASP_XptMcpbChannelHandle hXptMcpbChannel
    )
{
    assert(hXptMcpbChannel);

    ASP_Utils_FreeCtx(g_mcpbChannelList, hXptMcpbChannel->channelNumber, ASP_NUM_XPT_MCPB_CHANNELS);

    if (hXptMcpbChannel) free(hXptMcpbChannel);

    return 0;
}

/**
Summary:
Start MCPB Channel so that ProxyServer can then start feeding it the AV Payloads.
**/
int ASP_XptMcpbChannel_Start(
    ASP_XptMcpbChannelHandle hXptMcpbChannel
    )
{
    assert(hXptMcpbChannel);

    if (!hXptMcpbChannel->started)
    {
        hXptMcpbChannel->started = true;
    }
    return (0);
}

/**
Summary:
Stop a MCPB Channel
**/
int ASP_XptMcpbChannel_Stop(
    ASP_XptMcpbChannelHandle hXptMcpbChannel
    )
{
    assert(hXptMcpbChannel);

    hXptMcpbChannel->started = false;
    return 0;
}

/**
Summary:
Get Xpt Mcpb buffer.
**/
char *ASP_XptMcpbChannel_GetBuffer(
    ASP_XptMcpbChannelHandle hXptMcpbChannel
    )
{
    return gXptMcpbBuffInfo.xptMcpbBuffer;
}

#if 0

/**
Summary:
Get Xpt Mcpb payloadSize. This will be called by asp_simulator
while reading data from mcpb buffer.
**/
size_t ASP_XptMcpbChannel_GetPayloadSize(
    ASP_XptMcpbChannelHandle hXptMcpbChannel
    )
{
    return hXptMcpbChannel->pXptMcpbBuffInfo->payloadSize;
}
#endif

/**
Summary:
API to feed AV Payload to the MCBP CModel.
**/
int ASP_XptMcpbChannel_FeedPayload(
    ASP_XptMcpbChannelHandle    hXptMcpbChannel,
    void                        *pPayload,
    size_t                      payloadLength
    )
{
    assert(hXptMcpbChannel);
    assert(pPayload);
    assert(payloadLength);
    /* TODO: Call XptMcpb CModel API to feed the payload (from the network peer) to the XptMcpb CModel. */
    /* Copy data to mcpb buffer and update the payload size.*/
    memcpy(gXptMcpbBuffInfo.xptMcpbBuffer, pPayload, payloadLength);
    gXptMcpbBuffInfo.payloadSize = payloadLength;

    return 0;
}

/**
Summary:
Get the associated channel number.
**/
int ASP_XptMcpbChannel_GetChannelNumber(
    ASP_XptMcpbChannelHandle  hXptMcpbChannel,
    uint32_t                  *pChannelNumber   /* output: */
    )
{
    assert(hXptMcpbChannel);

    *pChannelNumber = hXptMcpbChannel->channelNumber;
    return (0);
}

/******** XPT M2M DMA implementation. ********/

typedef struct ASP_XptM2mDmaChannel
{
    int         channelNumber;
    bool        started;
    ASP_FeedPayloadCallbackDesc feedPayloadDesc;
} ASP_XptM2mDmaChannel;

/**
Summary:
Open a MCPB Channel that will be used to feed the AV Payloads (from App) to be streamed out.
**/
ASP_XptM2mDmaChannelHandle ASP_XptM2mDmaChannel_Open(
    ASP_FeedPayloadCallbackDesc *pFeedPayloadDesc
    )
{
    int                         channelNumber;
    ASP_XptM2mDmaChannelHandle    hXptM2mDmaChannel;

    assert(pFeedPayloadDesc);

    /* Find the next free MCPB Channel number. */
    channelNumber = ASP_Utils_AllocFreeCtx(g_m2mDmaChInUseList, ASP_NUM_XPT_M2MDMA_CHANNELS, pFeedPayloadDesc);
    if (channelNumber < 0) { fprintf(stderr, "%s: Failed to find a free MCPB Channel\n", __FUNCTION__); return NULL; }

    hXptM2mDmaChannel = calloc(1, sizeof(ASP_XptM2mDmaChannel));
    assert(hXptM2mDmaChannel);

    hXptM2mDmaChannel->channelNumber = channelNumber;
    hXptM2mDmaChannel->feedPayloadDesc = *pFeedPayloadDesc;

    return (hXptM2mDmaChannel);
}

/**
Summary:
Close a channel.
**/
int ASP_XptM2mDmaChannel_Close(
    ASP_XptM2mDmaChannelHandle  hXptM2mDmaChannel
    )
{
    assert(hXptM2mDmaChannel);

    ASP_Utils_FreeCtx(g_m2mDmaChInUseList, hXptM2mDmaChannel->channelNumber, ASP_NUM_XPT_M2MDMA_CHANNELS);

    if (hXptM2mDmaChannel) free(hXptM2mDmaChannel);

    return 0;
}

/**
Summary:
Get the associated channel number.
**/
int ASP_XptM2mDmaChannel_GetChannelNumber(
    ASP_XptM2mDmaChannelHandle  hXptM2mDmaChannel,
    uint32_t                    *pChannelNumber   /* output: */
    )
{
    assert(hXptM2mDmaChannel);

    *pChannelNumber = hXptM2mDmaChannel->channelNumber;
    return (0);
}

/**
Summary:
Start MCPB Channel so that ProxyServer can then start feeding it the AV Payloads.
**/
int ASP_XptM2mDmaChannel_Start(
    ASP_XptM2mDmaChannelHandle  hXptM2mDmaChannel
    )
{
    int rc = -1;

    assert(hXptM2mDmaChannel);

    if (!hXptM2mDmaChannel->started)
    {
        hXptM2mDmaChannel->started = true;
        rc = 0;
    }
    return (rc);
}

/**
Summary:
Stop a channel.
**/
int ASP_XptM2mDmaChannel_Stop(
    ASP_XptM2mDmaChannelHandle hXptM2mDmaChannel
    )
{
    assert(hXptM2mDmaChannel);

    hXptM2mDmaChannel->started = false;
    return 0;
}

/**
Summary:
Dummy function to run the IO loop calls feedPayload callback.
**/
void ASP_XptM2mDmaChannel_ProcessIo_test(
    uint32_t channelNumber
    )
{
    ASP_FeedPayloadCallbackDesc *pFeedPayloadDesc;

    assert(channelNumber < ASP_NUM_XPT_M2MDMA_CHANNELS );
    pFeedPayloadDesc = ASP_Utils_FindCtxUsingCtxNumber(g_m2mDmaChInUseList, channelNumber, ASP_NUM_XPT_M2MDMA_CHANNELS);
    assert(pFeedPayloadDesc);

    /* invoke the callback & provide dummy payload data to send out. */
    {
#define PAYLOAD_LEN 8
        int i;
        int rc;
        uint8_t payload[PAYLOAD_LEN];

        for (i=0; i < PAYLOAD_LEN; i++)
            payload[i] = i;
        rc = pFeedPayloadDesc->callback(pFeedPayloadDesc->pContext, pFeedPayloadDesc->param, payload, PAYLOAD_LEN);
        assert(rc == PAYLOAD_LEN);
    }
}

/******** XPT RAVE implementation. ********/

typedef struct ASP_XptRaveChannel
{
    int         channelNumber;
    bool        started;
} ASP_XptRaveChannel;

/**
Summary:
Open a MCPB Channel that will be used to feed the AV Payloads (from App) to be streamed out.
**/
ASP_XptRaveChannelHandle ASP_XptRaveChannel_Open( void )
{
    int                         channelNumber;
    ASP_XptRaveChannelHandle    hXptRaveChannel;

    /* Find the next free MCPB Channel number. */
    channelNumber = ASP_Utils_AllocFreeCtx(g_raveChannelList, ASP_NUM_XPT_RAVE_CHANNELS, NULL);
    if (channelNumber < 0) { fprintf(stderr, "%s: Failed to find a free RAVE Channel\n", __FUNCTION__); return NULL; }

    hXptRaveChannel = calloc(1, sizeof(ASP_XptRaveChannel));
    assert(hXptRaveChannel);

    hXptRaveChannel->channelNumber = channelNumber;

    return (hXptRaveChannel);
}

/**
Summary:
Remove a previously added flow.
**/
int ASP_XptRaveChannel_Close(
    ASP_XptRaveChannelHandle hXptRaveChannel
    )
{
    assert(hXptRaveChannel);

    ASP_Utils_FreeCtx(g_raveChannelList, hXptRaveChannel->channelNumber, ASP_NUM_XPT_RAVE_CHANNELS);

    if (hXptRaveChannel) free(hXptRaveChannel);

    return 0;
}

/**
Summary:
Start MCPB Channel so that ProxyServer can then start feeding it the AV Payloads.
**/
int ASP_XptRaveChannel_Start(
    ASP_XptRaveChannelHandle hXptRaveChannel
    )
{
    int rc = -1;

    assert(hXptRaveChannel);

    if (!hXptRaveChannel->started)
    {
        hXptRaveChannel->started = true;
        rc = 0;
    }
    return (rc);
}

/**
Summary:
Stop a MCPB Channel
**/
int ASP_XptRaveChannel_Stop(
    ASP_XptRaveChannelHandle hXptRaveChannel
    )
{
    assert(hXptRaveChannel);

    hXptRaveChannel->started = false;
    return 0;
}

/**
Summary:
Get the associated channel number.
**/
int ASP_XptRaveChannel_GetChannelNumber(
    ASP_XptRaveChannelHandle  hXptRaveChannel,
    uint32_t                  *pChannelNumber   /* output: */
    )
{
    assert(hXptRaveChannel);

    *pChannelNumber = hXptRaveChannel->channelNumber;
    return (0);
}
