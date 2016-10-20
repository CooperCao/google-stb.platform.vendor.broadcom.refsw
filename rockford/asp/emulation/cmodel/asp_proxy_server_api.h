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

#ifndef __ASP_PROXY_SERVER_API_H__
#define __ASP_PROXY_SERVER_API_H__

#include "asp_utils.h"
#include "asp_proxy_server_message_api.h"

/* ASP Proxy Server Interface APIs. */
typedef struct ASP_ProxyServer *ASP_ProxyServerHandle;
typedef struct ASP_ProxyServerCreateSettings
{
    const char *pPayloadListenerPort;
    const char *pRawFrameListenerPort;
    const char *pEventListenerPort;
}ASP_ProxyServerCreateSettings;

#define ASP_ProxyServer_GetDefaultCreateSettings(pSettings) \
    (pSettings)->pPayloadListenerPort = ASP_PROXY_SERVER_PORT_FOR_PAYLOAD; \
    (pSettings)->pRawFrameListenerPort = ASP_PROXY_SERVER_PORT_FOR_RAW_FRAMES; \
    (pSettings)->pEventListenerPort = ASP_PROXY_SERVER_PORT_FOR_EVENTS;

/* ASP ProxyServer APIs that will be called by the top-level loop */

/* API to Create ProxyServer which listens on TCP Sockets for Message Requests from DUT. */
ASP_ProxyServerHandle ASP_ProxyServer_Create(
    ASP_ProxyServerCreateSettings *pSettings
    );

/* API to Destroy ProxyServer. */
void ASP_ProxyServer_Destory(
    ASP_ProxyServerHandle hProxyServer
    );

/* API to process any Requests from the DUT: Reads Requests & invoke corresponding ASP CModel APIs. */
int ASP_ProxyServer_ProcessIo(
    ASP_ProxyServerHandle hProxyServer
    );

int ASP_Nexus_ProcessIo(
    ASP_ProxyServerHandle hProxyServer
    );

int Asp_Simulator_ProcessIo(
    ASP_ProxyServerHandle hProxyServer
    );

#endif /* __ASP_PROXY_SERVER_API_H__ */
