/***************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 **************************************************************************/
#ifndef NEXUS_ASP_MODULE_H__
#define NEXUS_ASP_MODULE_H__

#include "nexus_asp_thunks.h"
#include "nexus_base.h"
#include "nexus_asp.h"
#include "nexus_asp_output.h"
#include "nexus_asp_input.h"
#include "nexus_asp_init.h"
#include "basp.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#define NEXUS_MODULE_NAME asp
#define NEXUS_MODULE_SELF g_NEXUS_aspModule

/* global handle. there is no global data. */
extern NEXUS_ModuleHandle g_NEXUS_aspModule;

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AspChannel);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AspOutput);
NEXUS_OBJECT_CLASS_DECLARE(NEXUS_AspInput);

typedef struct NEXUS_AspBuffer
{
    BMMA_Block_Handle   hBlock;
    void                *pBuffer;
    BMMA_DeviceOffset   offset;
    BMMA_Heap_Handle    hMmaHeap;      /* if non-NULL, then the block is internally-allocated */
    NEXUS_HeapHandle    hNexusHeap;
    unsigned            size;
} NEXUS_AspBuffer;

typedef struct NEXUS_asp_data
{
    NEXUS_AspModuleSettings             settings;
    BASP_Handle                         hAspBaseModule;
    BASP_ContextHandle                  hContext;
    struct NEXUS_AspChannel             *hAspChannelList[BASP_MAX_NUMBER_OF_CHANNEL];
    struct NEXUS_AspOutput              *hAspOutputList[BASP_MAX_NUMBER_OF_CHANNEL];
    struct NEXUS_AspInput               *hAspInputList[BASP_MAX_NUMBER_OF_CHANNEL];
    NEXUS_TimerHandle                   hTimer;
    NEXUS_TimerHandle                   hAspOutputTimer;
    NEXUS_TimerHandle                   hAspInputTimer;
    unsigned                            timerIntervalInMs;
    NEXUS_AspBuffer                     statusBuffer;           /* Buffer contains BASP_FwStatusInfo type. */

    bool enableTcpRetrans;
    bool enableTcpCongestionControl;
    bool enableTcpTimestamps;
    bool enableTcpSack;
    bool enableAch;

} NEXUS_asp_data;

extern  NEXUS_asp_data  g_NEXUS_asp;

#ifdef __cplusplus
}
#endif

#endif
