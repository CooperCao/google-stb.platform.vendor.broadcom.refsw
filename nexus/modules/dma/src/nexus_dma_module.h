/***************************************************************************
 *     (c)2007-2012 Broadcom Corporation
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

#include "nexus_base.h"
#include "nexus_dma_thunks.h"
#include "nexus_dma.h"
#include "priv/nexus_dma_priv.h"
#include "priv/nexus_core.h"
#include "nexus_platform_features.h"
#include "blst_slist.h"
#if NEXUS_HAS_XPT_DMA

#else
#include "bmmd.h"
#endif

#ifndef NEXUS_DMA_MODULE_H__
#define NEXUS_DMA_MODULE_H__

#ifdef NEXUS_MODULE_SELF
#error Cant be in two modules at the same time
#endif

#ifndef NEXUS_NUM_SHARF_DMA_CHANNELS
#define NEXUS_NUM_SHARF_DMA_CHANNELS 0
#endif

#define NEXUS_MODULE_NAME dma
#define NEXUS_MODULE_SELF g_NEXUS_dmaModuleData.dmaModule

#if (!NEXUS_HAS_XPT_DMA)
typedef struct NEXUS_DmaModuleData
{
    NEXUS_ModuleHandle dmaModule;
    bool shutdown;
    struct {
        BMMD_Handle mmd;
        BLST_S_HEAD(NEXUS_DmaContexts, NEXUS_Dma) dmaHandles; /* virtualization in nexus allows opening multiple nexus DMA handles against one HW block */
    } channel[NEXUS_NUM_DMA_CHANNELS+NEXUS_NUM_SHARF_DMA_CHANNELS]; /* a channel represents a HW block */
} NEXUS_DmaModuleData;
#else
typedef struct NEXUS_DmaModuleData
{
    NEXUS_ModuleHandle dmaModule;
} NEXUS_DmaModuleData;
#endif

/* global module handle & data */
extern NEXUS_DmaModuleData g_NEXUS_dmaModuleData;

#endif /* #ifndef NEXUS_DMA_MODULE_H__ */

