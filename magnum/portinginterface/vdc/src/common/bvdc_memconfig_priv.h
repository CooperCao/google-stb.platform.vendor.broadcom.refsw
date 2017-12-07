/***************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef BVDC_MEMCONFIG_PRIV_H__
#define BVDC_MEMCONFIG_PRIV_H__

#include "bvdc.h"
#include "bvdc_common_priv.h"
#include "bvdc_bufferheap_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const BVDC_P_Features s_VdcFeatures;
extern const BVDC_Settings s_stDefaultSettings;

/***************************************************************************
 * Private macros
 ***************************************************************************/
#define BVDC_P_MEMCONFIG_DEINTERLACER_ON(eDeinterlacerMode)   \
    ((eDeinterlacerMode == BVDC_DeinterlacerMode_eBestQuality) || \
     (eDeinterlacerMode == BVDC_DeinterlacerMode_eLowestLatency))

typedef struct
{
    /* Total number available */
    uint32_t                     ulNumCmp;  /* CMP count */
    uint32_t                     ulNumStg;  /* STG count, <= ulNumCmp */
    uint32_t                     ulNumMad;  /* jTotal Deinterlacer count, including ulNumMadr */
    uint32_t                     ulNumMadr; /* Madr count, <= ulNumMad */

    /* Number used */
    uint32_t                     ulNumCmpUsed;
    uint32_t                     ulNumStgUsed;
    uint32_t                     ulNumMadUsed;
    uint32_t                     ulNumMadrUsed;

    bool                         b4kSupported;
    bool                         bEnableShare4Lipsync;
    BVDC_P_BufferHeap_SizeInfo   stHeapSizeInfo;

} BVDC_P_MemConfig_SystemInfo;

typedef struct
{
    /* These flags control capture buffer count */
    bool                     bSyncLock;
    bool                     bCapture;
    bool                     bPip;
    bool                     bLipsync;
    bool                     b5060Convert;
    bool                     bSlave_24_25_30_Display;

    /* This flags control deinterlcer buffer count */
    bool                     bMadr;
    BVDC_DeinterlacerMode    eDeinterlacerMode;

    /* This flags control buffer size */
    bool                     b3d;
    bool                     bMosaicMode;
    BFMT_VideoFmt            eFormat;
    /* TODO: 10bit 422 for 7445 Dx
    bool                     b10Bit422;
    */
    uint32_t                 ulAdditionalBufCnt;
    uint32_t                 aulCapBufCnt[BVDC_P_BufferHeapId_eCount];
    uint32_t                 aulMadBufCnt[BVDC_P_BufferHeapId_eCount];

} BVDC_P_MemConfig_WindowInfo;

/***************************************************************************
 * Memory private functions
 ***************************************************************************/
BERR_Code BVDC_P_MemConfig_GetDefaultRdcSettings
    ( BVDC_RdcMemConfigSettings          *pRdc );

BERR_Code BVDC_P_MemConfig_GetDefaultDisplaySettings
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      uint32_t                            ulDispIndex,
      BVDC_DispMemConfigSettings         *pDisplay );

void BVDC_P_MemConfig_GetDefaultDeinterlacerSettings
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      uint32_t                            ulDispIndex,
      uint32_t                            ulWinIndex,
      BVDC_WinMemConfigSettings          *pWindow,
      bool                                bSd );

BERR_Code BVDC_P_MemConfig_GetDefaultWindowSettings
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      uint32_t                            ulDispIndex,
      uint32_t                            ulWinIndex,
      BVDC_WinMemConfigSettings          *pWindow );

BERR_Code BVDC_P_MemConfigInfo_Init
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo );

BERR_Code BVDC_P_MemConfig_GetBufSize
    ( const BVDC_Heap_Settings           *pHeapSettings,
      BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo );

BERR_Code BVDC_P_MemConfig_Validate
    ( const BVDC_MemConfigSettings       *pMemConfigSettings,
      BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo );

BERR_Code BVDC_P_MemConfig_GetWindowInfo
    ( BVDC_WinMemConfigSettings          *pWindow,
      BVDC_DispMemConfigSettings         *pDisplay,
      BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      uint32_t                            ulDispIndex,
      uint32_t                            ulWinIndex,
      BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo );

BERR_Code BVDC_P_MemConfig_GetWindowBufCnt
    ( BVDC_WinMemConfigSettings          *pWindow,
      BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo,
      uint32_t                            ulDispIndex,
      uint32_t                            ulWinIndex );

BERR_Code BVDC_P_MemConfig_GetWinBufSize
    ( BVDC_P_MemConfig_SystemInfo        *pSystemConfigInfo,
      BVDC_P_MemConfig_WindowInfo        *pWinConfigInfo,
      uint32_t                           *pulCapSize,
      uint32_t                           *pulMadSize );

BERR_Code BVDC_P_MemConfig_SetBufFormat
    ( const BVDC_Heap_Settings           *pHeapSettingsIn,
      BVDC_Heap_Settings                 *pHeapSettingsOut );

BERR_Code BVDC_P_MemConfig_GetRulSize
    ( uint32_t                           *pulRulSize );


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_MEMCONFIG_PRIV_H__*/

/* End of file. */
