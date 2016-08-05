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
#ifndef __ASP_XPT_API_H__
#define __ASP_XPT_API_H__

#include "asp_utils.h"

#define ASP_XPT_MCPB_BUFFER_SIZE    4096*2 /* Bytes*/

typedef struct  ASP_XptMcpb_BuffInfo{
    size_t  payloadSize;     /*!< This informs the datasize that written to the mcpb buffer.*/
    char    xptMcpbBuffer[ASP_XPT_MCPB_BUFFER_SIZE]; /*!< This mimics Mcpb dram buffer where mcpb will put incoming av data for asp.*/
} ASP_XptMcpb_BuffInfo;

/**
Summary:
Initialize All CModels of XPT (MCPB, M2M DMA, RAVE, etc.)
**/
int ASP_Xpt_Init(void);

/**
Summary:
Un-initialize All CModels of XPT (MCPB, M2M DMA, RAVE, etc.)
**/
void ASP_Xpt_UnInit(void);

typedef struct ASP_XptMcpbChannel *ASP_XptMcpbChannelHandle;

/**
Summary:
Open a MCPB Channel that will be used to feed the AV Payloads (from App) to be streamed out.
**/
ASP_XptMcpbChannelHandle ASP_XptMcpbChannel_Open( void );

/**
Summary:
Close a channel.
**/
int ASP_XptMcpbChannel_Close(
    ASP_XptMcpbChannelHandle  hXptMcpbChannel
    );

/**
Summary:
Get the associated channel number.
**/
int ASP_XptMcpbChannel_GetChannelNumber(
    ASP_XptMcpbChannelHandle  hXptMcpbChannel,
    uint32_t                  *pChannelNumber   /* output: */
    );

/**
Summary:
Start MCPB Channel so that ProxyServer can then start feeding it the AV Payloads.
**/
int ASP_XptMcpbChannel_Start(
    ASP_XptMcpbChannelHandle  hXptMcpbChannel
    );

/**
Summary:
Stop a channel.
**/
int ASP_XptMcpbChannel_Stop(
    ASP_XptMcpbChannelHandle hXptMcpbChannel
    );

/**
Summary:
Get Xpt Mcpb buffer.
**/
char *ASP_XptMcpbChannel_GetBuffer(
    ASP_XptMcpbChannelHandle hXptMcpbChannel
    );

#if 0

/**
Summary:
Get Xpt Mcpb payloadSize. This will be called by asp_simulator
while reading data from mcpb buffer.
**/
size_t ASP_XptMcpbChannel_GetPayloadSize(
    ASP_XptMcpbChannelHandle hXptMcpbChannel
    );
#endif

/**
Summary:
API to feed AV Payload to the MCBP CModel.
**/
int ASP_XptMcpbChannel_FeedPayload(
    ASP_XptMcpbChannelHandle hXptMcpbChannel,
    void *pPayload,
    size_t payloadLength
    );

typedef struct ASP_XptM2mDmaChannel *ASP_XptM2mDmaChannelHandle;

/**
Summary:
Callback function to feed payload to DUT.
**/
typedef int (*ASP_FeedPayload)(
    void *pContext,
    int param,
    void *pPayload,
    size_t payloadLength
    );
typedef struct ASP_FeedPayloadCallbackDesc
{
    ASP_FeedPayload             callback;       /* Function pointer */
    void                        *pContext;      /* First parameter to callback function. */
    int                         param;          /* Second parameter to callback function. */
} ASP_FeedPayloadCallbackDesc;

/**
Summary:
Open a MCPB Channel that will be used to feed the AV Payloads (from App) to be streamed out.
**/
ASP_XptM2mDmaChannelHandle ASP_XptM2mDmaChannel_Open(
    ASP_FeedPayloadCallbackDesc *pFeedPayloadDesc
    );

/**
Summary:
Close a channel.
**/
int ASP_XptM2mDmaChannel_Close(
    ASP_XptM2mDmaChannelHandle  hXptM2mDmaChannel
    );

/**
Summary:
Get the associated channel number.
**/
int ASP_XptM2mDmaChannel_GetChannelNumber(
    ASP_XptM2mDmaChannelHandle  hXptM2mDmaChannel,
    uint32_t                    *pChannelNumber   /* output: */
    );

/**
Summary:
Start MCPB Channel so that ProxyServer can then start feeding it the AV Payloads.
**/
int ASP_XptM2mDmaChannel_Start(
    ASP_XptM2mDmaChannelHandle  hXptM2mDmaChannel
    );

/**
Summary:
Stop a channel.
**/
int ASP_XptM2mDmaChannel_Stop(
    ASP_XptM2mDmaChannelHandle hXptM2mDmaChannel
    );

typedef struct ASP_XptRaveChannel *ASP_XptRaveChannelHandle;

/**
Summary:
Open a RAVE Channel that will be used to feed the AV Payloads (from App) to be streamed out.
**/
ASP_XptRaveChannelHandle ASP_XptRaveChannel_Open( void );

/**
Summary:
Close a channel.
**/
int ASP_XptRaveChannel_Close(
    ASP_XptRaveChannelHandle  hXptRaveChannel
    );

/**
Summary:
Get the associated channel number.
**/
int ASP_XptRaveChannel_GetChannelNumber(
    ASP_XptRaveChannelHandle  hXptRaveChannel,
    uint32_t                  *pChannelNumber   /* output: */
    );

/**
Summary:
Start RAVE Channel. This causes XPT to release AV data from Playback Channel into the RAVE FIFO.
**/
int ASP_XptRaveChannel_Start(
    ASP_XptRaveChannelHandle  hXptRaveChannel
    );

/**
Summary:
Stop a channel.
**/
int ASP_XptRaveChannel_Stop(
    ASP_XptRaveChannelHandle hXptRaveChannel
    );

#endif /* __ASP_XPT_API_H__ */
