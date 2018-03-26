/***************************************************************************
* Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*
* Module Description:
*
***************************************************************************/

#include "nexus_base.h"
#include "nexus_cec_thunks.h"
#include "nexus_cec.h"
#include "priv/nexus_cec_priv.h"
#include "nexus_cec_init.h"
#include "bcec.h"
#include "nexus_platform_features.h"
#include "nexus_hdmi_output.h"

#ifndef NEXUS_CEC_MODULE_H__
#define NEXUS_CEC_MODULE_H__

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME cec
#define NEXUS_MODULE_SELF g_NEXUS_cecModule

/* Global Types */
typedef enum NEXUS_CecLogicalAddrSearch
{
    NEXUS_CecLogicalAddrSearch_eInit,
    NEXUS_CecLogicalAddrSearch_eNext,
    NEXUS_CecLogicalAddrSearch_eReady
} NEXUS_CecLogicalAddrSearch;

typedef struct NEXUS_Cec
{
    NEXUS_OBJECT(NEXUS_Cec);
    bool opened;
    BCEC_Handle cecHandle;
    NEXUS_CecSettings cecSettings;
    NEXUS_CecStatus status;
    NEXUS_EventCallbackHandle cecTransmittedEventCallback;
    NEXUS_EventCallbackHandle cecReceivedEventCallback;

    NEXUS_CecLogicalAddrSearch searchState;
    unsigned logAddrSearchIndex;
    unsigned logAddrStartIndex;

    NEXUS_TaskCallbackHandle messageTransmittedCallback;
    NEXUS_TaskCallbackHandle messageReceivedCallback;
    NEXUS_TaskCallbackHandle logicalAddressAcquiredCallback;

    NEXUS_HdmiOutputHandle hdmiOutput;
    BKNI_EventHandle cecHotplugEvent;
    NEXUS_EventCallbackHandle cecHotplugEventCallback;
} NEXUS_Cec;

/* Global module handle & data */
extern NEXUS_ModuleHandle g_NEXUS_cecModule;
extern NEXUS_CecModuleSettings g_NEXUS_cecModuleSettings;


/* Internal Private Routines */
void NEXUS_Cec_P_Shutdown(void);
void NEXUS_Cec_P_TransmittedCallback(void *pContext);
void NEXUS_Cec_P_ReceivedCallback(void *pContext);

void NEXUS_CecModule_Print(void);


#endif /* #ifndef NEXUS_CEC_MODULE_H__ */





