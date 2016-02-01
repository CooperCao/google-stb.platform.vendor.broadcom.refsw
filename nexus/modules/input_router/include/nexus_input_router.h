/***************************************************************************
 *     (c)2011 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
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
 **************************************************************************/
#ifndef NEXUS_INPUT_ROUTER_H__
#define NEXUS_INPUT_ROUTER_H__

#include "nexus_input_router_types.h"
#include "nexus_input_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
server API
**/

/**
Summary:
**/
typedef struct NEXUS_InputRouter *NEXUS_InputRouterHandle;

/**
Summary:
**/
NEXUS_InputRouterHandle NEXUS_InputRouter_Create( /* attr{destructor=NEXUS_InputRouter_Destroy} */
    unsigned server_id
    );

/**
Summary:
**/
void NEXUS_InputRouter_Destroy(
    NEXUS_InputRouterHandle handle
    );
    
/**
Summary:
**/
NEXUS_InputClientHandle NEXUS_InputRouter_CreateClient( /* attr{destructor=NEXUS_InputRouter_DestroyClient} */
    NEXUS_InputRouterHandle handle,
    unsigned client_id
    );

/**
Summary:
**/
void NEXUS_InputRouter_DestroyClient(
    NEXUS_InputClientHandle client
    );
    
typedef struct NEXUS_InputRouterClientSettings
{
    unsigned fifoSize; /* size of NEXUS_InputRouterCode queue */
    uint32_t filterMask; /* server-controlled filter mask. if (filterMask & code.filterMask) then the code will be sent.
                            see NEXUS_InputRouterCode.filterMask for full discussion. */
    NEXUS_Rect position; /* in main display coordinates. location of concern for pointer device. */
} NEXUS_InputRouterClientSettings;

/**
Summary:
**/
void NEXUS_InputRouter_GetClientSettings(
    NEXUS_InputRouterHandle handle,
    NEXUS_InputClientHandle client,
    NEXUS_InputRouterClientSettings *pSettings /* [out] */
    );
    
/**
Summary:
server-controlled client settings
**/
NEXUS_Error NEXUS_InputRouter_SetClientSettings(
    NEXUS_InputRouterHandle handle,
    NEXUS_InputClientHandle client,
    const NEXUS_InputRouterClientSettings *pSettings
    );

/**
Summary:
**/
void NEXUS_InputRouter_GetDefaultCode(
    NEXUS_InputRouterCode *pCode /* [out] */
    );
    
/**
Summary:
send a code to all enabled clients
**/
void NEXUS_InputRouter_SendCode(
    NEXUS_InputRouterHandle handle,
    const NEXUS_InputRouterCode *pCode
    );
        
/**
Summary:
Send a code to one client

Description:
This bypasses the server mask but still honors the client mask.
**/
void NEXUS_InputRouter_SendClientCode(
    NEXUS_InputRouterHandle handle,
    NEXUS_InputClientHandle client,
    const NEXUS_InputRouterCode *pCode
    );

#ifdef __cplusplus
}
#endif

#endif

