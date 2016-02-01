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
 **************************************************************************/
#ifndef NEXUS_INPUT_CLIENT_H__
#define NEXUS_INPUT_CLIENT_H__

#include "nexus_types.h"
#include "nexus_input_router_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
client API
**/

/**
Summary:
**/
NEXUS_InputClientHandle NEXUS_InputClient_Acquire( /* attr{release=NEXUS_InputClient_Release} */
    unsigned client_id /* server app determines ids */
    );

/**
Summary:
**/
void NEXUS_InputClient_Release(
    NEXUS_InputClientHandle client
    );

/**
Summary:
**/
typedef struct NEXUS_InputClientSettings
{
    uint32_t filterMask; /* client-controlled filter mask. if (filter_mask & code.filter_mask) then the code will be received. 
                            see NEXUS_InputRouterCode.filterMask for full discussion. */
    NEXUS_CallbackDesc codeAvailable; /* fired when a NEXUS_InputRouterCode is available. call NEXUS_InputClient_GetCodes. */
} NEXUS_InputClientSettings;

/**
Summary:
**/
void NEXUS_InputClient_GetSettings(
    NEXUS_InputClientHandle handle,
    NEXUS_InputClientSettings *pSettings /* [out] */
    );

/**
Summary:
**/
NEXUS_Error NEXUS_InputClient_SetSettings(
    NEXUS_InputClientHandle handle,
    const NEXUS_InputClientSettings *pSettings
    );

/**
Summary:
Retrieve one or more input codes
**/
NEXUS_Error NEXUS_InputClient_GetCodes(
    NEXUS_InputClientHandle handle,
    NEXUS_InputRouterCode *pCode, /* attr{nelem=numEntries;nelem_out=pNumReturned} [out] */
    unsigned numEntries,
    unsigned *pNumReturned /* [out] */
    );
        
typedef struct NEXUS_InputClientStatus
{
    struct {
        uint32_t filterMask; /* read the server-controlled filter mask */
    } server;
} NEXUS_InputClientStatus;

/**
Summary:
**/
NEXUS_Error NEXUS_InputClient_GetStatus(
    NEXUS_InputClientHandle handle,
    NEXUS_InputClientStatus *pStatus
    );

#ifdef __cplusplus
}
#endif

#endif
