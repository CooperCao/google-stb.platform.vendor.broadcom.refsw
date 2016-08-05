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
#ifndef __ASP_NW_SW_API_H__
#define __ASP_NW_SW_API_H__

#include "asp_utils.h"

typedef enum ASP_IpTransportProtocol
{
    ASP_IpTransportProtocol_eTcp,
    ASP_IpTransportProtocol_eUdp,
    ASP_IpTransportProtocol_eMax
} ASP_IpTransportProtocol;

typedef struct ASP_SocketInfo
{
    int                         sockFd;
    ASP_IpTransportProtocol     protocol;
    uint32_t                    remotePort;
    uint32_t                    remoteIpAddr[4];
    uint32_t                    localPort;
    uint32_t                    localIpAddr[4];
    int                         noIpFragments;      /* True: In addition to above 5-type, CFP rule should also specify IP Header packets w/ Dont Fragment bit. */
} ASP_SocketInfo;

/**
Summary:
Callback function to feed payload to DUT.
**/
typedef int (*ASP_FeedRawFrame)(
    void *pContext,
    int param,
    void *pRawFrame,
    size_t frameLength
    );
typedef struct ASP_FeedRawFrameCallbackDesc
{
    ASP_FeedRawFrame            callback;       /* Function pointer */
    void                        *pContext;      /* First parameter to callback function. */
    int                         param;          /* Second parameter to callback function. */
} ASP_FeedRawFrameCallbackDesc;


/**
Summary:
Initialize Network Switch Emulation Module.
**/
int ASP_NwSw_Init(void);

/**
Summary:
Un-Initialize Network Switch Emulation Module.
**/
void ASP_NwSw_UnInit(void);

typedef struct ASP_NwSwFlow *ASP_NwSwFlowHandle;

/**
Summary:
Open & add Flow Details to Network Switch so that it redirect the incoming traffic to ASP Port.

Note: this Network Switch will not redirect the incoming packets to ASP CModel until
ASP_NwSwFlow_Startis called. This will be called as the last step in the Offload Sequence.

Returns:
A unique flow-id that is used to Remove, Enable, Disable that flow.
**/
ASP_NwSwFlowHandle ASP_NwSwFlow_Open(
    ASP_SocketInfo                  *pSocketInfo,
    ASP_FeedRawFrameCallbackDesc    *pRawFrameCallbackDesc,    /* Callback Descriptor for allowing NwSw CModel to send out the Raw Frames back to DUT. */
    uint32_t                        *pFlowId                   /* Output: unique flowId (aka broadcom tag). */
    );

/**
Summary:
Remove a previously added flow.
**/
int ASP_NwSwFlow_Close(
    ASP_NwSwFlowHandle  hFlow
    );

/**
Summary:
Enable a particular flow so that N/W Switch can now start forwarding incoming raw frames to ASP CModel.
**/
int ASP_NwSwFlow_Start(
    ASP_NwSwFlowHandle  hFlow
    );

/**
Summary:
Disable a particular flow so that N/W Switch can now stop forwarding incoming raw frames to ASP CModel.
**/
int ASP_NwSwFlow_Stop(
    ASP_NwSwFlowHandle hFlow
    );

/**
Summary:
API to feed Raw Ethernet Frames to the Network Switch CModel.
**/
int ASP_NwSwFlow_FeedRawFrame(
    ASP_NwSwFlowHandle hFlow,
    void *pRawFrame,
    size_t frameLength
    );

/**
Summary:
API to feed Raw Ethernet Frames from asp simulator.
**/
int ASP_NwSwFlow_FeedRawFrameFromAspSimulator(
    void *pRawFrame,
    size_t frameLength
    );

#endif /* __ASP_NW_SW_API_H__ */
