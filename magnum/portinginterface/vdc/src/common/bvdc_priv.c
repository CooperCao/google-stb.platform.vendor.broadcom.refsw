/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
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
 ***************************************************************************/
#include "bstd.h"                 /* standard types */
#include "bkni.h"                 /* memcpy calls */
#include "bvdc.h"                 /* Video display */
#include "bdbg.h"
#include "bavc.h"
#include "bvdc_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_feeder_priv.h"
#include "bvdc_656in_priv.h"

BDBG_MODULE(BVDC_PRIV);

#ifndef BVDC_FOR_BOOTUPDATER
#define BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH     3840
#define BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT    2176

static const BVDC_P_MosaicCanvasCoverage s_full_CCTbl =
    /* 1    2    3    4    5    6    7    8    9   10   11   12  */
    {{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100},
     {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}};

#if 0
/* Legacy table for total area coverage for all mosaics
 * Official number for class 4
 * http://www.sj.broadcom.com/projects/dvt/Chip_Architecture/Video/Released/BVN_Mosaic_Rules_tables.xlsx
 */
static const BVDC_P_MosaicCanvasCoverage sa_MosaicCoverageTble[] =
{
    /* 1    2    3    4    5    6    7    8    9   10   11   12  */
    {{200, 100, 100, 100, 100, 100,  60,  60,  60,  60,  60,  60 }},
    {{100,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50,  50 }},
    {{100,  75,  75,  66,  66,  66,  50,  50,  50,  50,  50,  50 }},
    {{136, 100, 100,  90,  90,  90,  68,  68,  68,  68,  68,  68 }},
    {{100, 100, 100, 100,  75,  66,  66,  50,  50,  50,  50,  50 }}
};
#else
/* derived Legacy table for dimension coverage for each mosaic
 * sa_LegacyMosaicCoverageTble[i] = sqrt(sa_MosaicCoverageTble/n) * 10
 */
static const BVDC_P_MosaicCanvasCoverage sa_LegacyMosaicCoverageTble[] =
{
    /* 1    2    3    4    5    6    7    8    9    10   11   12  */
    {{141,  71,  58,  50,  45,  41,  29,  27,  26,  24,  23,  22 },
     {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }},
    {{100,  50,  41,  35,  32,  29,  27,  25,  24,  22,  21,  20 },
     {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }},
    {{100,  61,  50,  41,  36,  33,  27,  25,  24,  22,  21,  20 },
     {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }},
    {{116,  71,  57,  47,  42,  39,  31,  29,  27,  26,  25,  24 },
     {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }},
    {{100,  71,  57,  50,  39,  33,  31,  25,  24,  22,  21,  20 },
     {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }}
};

#endif

static const BVDC_SourceClassLimits sa_SourceClassTble[] =
{
    /* 1            2             3             4           5           6           7           8           9           10          11          12  */
    /* legacy class */
    {
     {
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}
     },
     {
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}
     },
     {
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}
     }
    },

    /* class 0.0 */
    {
     {
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}
     },
     {
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}
     },
     {
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT},
        {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}, {BVDC_P_SRC_SIZE_LIMIT_MAX_WIDTH, BVDC_P_SRC_SIZE_LIMIT_MAX_HEIGHT}
     }
    },
    /* class 2.1 */
    {
     {{1280,  720}, {352,   240}, {352,   240}, {176, 120}, {176, 120}, {176, 120},},
     {{1920,  540}, {720,   240}, {720,   240},},
     {{720,  1280},}
    },
    /* class 2.2 */
    {
     {{1280,  720}, {352,   288}, {352,   288}, {176, 144}, {176, 144}, {176, 144},},
     {{1920,  540}, {720,   288}, {720,   288},},
     {{720,  1280},}
    },
    /* class 3.1 */
    {
     {{1920, 1080}, {720,   480}, {720,   480}, {352, 240}, {176, 120}, {176, 120}, {176, 120}, {176, 120},},
     {{1920,  540}, {720,   240}, {720,   240}, {720, 240},},
     {{720,  1280},}
    },
    /* class 3.2 */
    {
     {{1920, 1080}, {720,   480}, {720,   480}, {352, 288}, {176, 144}, {176, 144}, {176, 144}, {176, 144},},
     {{1920,  540}, {720,   288}, {720,   288}, {720, 288},},
     {{720,  1280},}
    },
    /* class 4.1 */
    {
     {{3840, 2160}, {1280,  720}, {1280,  720}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 240}, {720, 240}, {720, 240}, {720, 240}, {720, 240}},
     {{3840, 2160}, {1920,  540}, {1920,  540}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 240}, {720, 240}, {720, 240}, {720, 240}, {720, 240}},
     {{1440, 2560},}
    },
    /* class 4.1.2 */
    {
     {{3840, 2160}, {1280,  720}, {1280,  720}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 288}, {720, 288}, {720, 288}, {720, 288}, {720, 288}},
     {{3840, 2160}, {1920,  540}, {1920,  540}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 288}, {720, 288}, {720, 288}, {720, 288}, {720, 288}},
     {{1440, 2560},}
    },
    /* class 4.2 */
    {
     {{3840, 2160}, {1920, 1080}, {1920, 1080}, {1280,  720}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 240}, {720, 240}, {720, 240}},
     {{3840, 2160}, {1920, 1080}, {1920, 1080}, {1920,  540}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 240}, {720, 240}, {720, 240}},
     {{1440, 2560},}
    },
    /* class 4.2.2 */
    {
     {{3840, 2160}, {1920, 1080}, {1920, 1080}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 288}, {720, 288}, {720, 288}},
     {{3840, 2160}, {1920, 1080}, {1920, 1080}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 480}, {720, 288}, {720, 288}, {720, 288}},
     {{1440, 2560},}
    }
};

static const BVDC_WindowClassLimits sa_WindowClassTble[] =
{
    /* legacy class */
    {
        /* bounding boxes */
        {{100, 100, false},},
        /* mosaic rects */
        {
            {{0, 0}, 100,   0}
        }
    },

    /* class 0.0 (all pass) */
    {
        /* bounding boxes */
        {{100, 100, false},},
        /* mosaic rects */
        {
            {{1920, 1080}, 100, 100},
            {{720,   576}, 100, 100},
            {{720,   576}, 100, 100},
            {{720,   576}, 100, 100},
            {{720,   576}, 100, 100},
            {{720,   576}, 100, 100},
            {{  0,     0}, 100, 100},
            {{  0,     0}, 100, 100},
            {{  0,     0}, 100, 100},
            {{  0,     0}, 100, 100},
            {{  0,     0}, 100, 100},
            {{  0,     0}, 100, 100}
        }
    },
    /* class 0.1 (all fail no mosaic support) */
    {
        /* bounding boxes */
        {{100, 100, false},},
        /* mosaic rects */
        {
            {{0, 0}, 0, 0},
        }
    },
    /* class 1.1 */
    {
        /* bounding boxes */
        {{100, 100, false},},
        {
            {{0, 0}, 95,   0},
            {{0, 0}, 63,  39},
            {{0, 0}, 51,  29},
            {{0, 0}, 45,  32},
            {{0, 0}, 41,  29},
            {{0, 0}, 34,  28},
            {{0, 0}, 30,  27},
            {{0, 0}, 29,  25},
            {{0, 0}, 26,  23},
            {{0, 0}, 25,  22},
            {{0, 0}, 23,  20},
            {{0, 0}, 21,  19}
        }
    },
    /* class 1.2 */
    {
        /* bounding boxes */
        {{50, 50, false}, {25, 100, false},},
        {
            {{0, 0}, 48,   0},
            {{0, 0}, 30,  19},
            {{0, 0}, 25,  14},
        }
    },
    /* class 2.1 */
    {
        /* bounding boxes */
        {{100, 100, false},},
        {
            {{720, 576}, 100,  0},
        }
    },
    /* class 3.1 */
    {
        /* bounding boxes */
        {{100, 100, false},},
        {
            {{1920, 1080}, 95,   0},
            {{720,   576}, 61,  35},
            {{720,   576}, 51,  29},
            {{720,   576}, 38,  28},
            {{0, 0}, 32,  26},
            {{0, 0}, 29,  25},
            {{0, 0}, 26,  23},
            {{0, 0}, 23,  21},
            {{0, 0}, 21,  20},
            {{0, 0}, 20,  18},
            {{0, 0}, 18,  16},
            {{0, 0}, 16,  15}
        }
    },
    /* class 3.2 */
    {
        /* bounding boxes */
        {{50, 50, false}, {25, 100, false},},
        {
            {{720,   576}, 38,   0},
            {{720,   576}, 29,  14},
            {{720,   576}, 25,  13},
        }
    },
    /* class 4.1 */
    {
        /* bounding boxes */
        {{100, 100, false},},
        {
            {{1920, 1080}, 95,   0},
            {{720,   576}, 63,  39},
            {{720,   576}, 51,  29},
            {{720,   576}, 45,  30},
            {{0, 0}, 41,  30},
            {{0, 0}, 34,  28},
            {{0, 0}, 30,  27},
            {{0, 0}, 29,  25},
            {{0, 0}, 26,  23},
            {{0, 0}, 25,  22},
            {{0, 0}, 23,  20},
            {{0, 0}, 21,  19}
        }
    },
    /* class 4.2 */
    {
        /* bounding boxes */
        {{50, 50, false}, {25, 100, false},},
        {
            {{1920, 1080}, 48,   0},
            {{720,   576}, 30,  19},
            {{720,   576}, 25,  14},
        }
    },
    /* class 4.2_2 */
    {
        /* bounding boxes */
        {{50, 50, false}, {25, 100, false}, {100, 25, false},},
        {
            {{1920, 1080}, 48,   0},
            {{720,   576}, 30,  19},
            {{720,   576}, 25,  14},
        }
    },

};
#endif

/***************************************************************************
 * Add an NO-OP into RUL.
 *
 */
void BVDC_P_BuildNoOpsRul_isr
    ( BRDC_List_Handle                 hList )
{
    uint32_t ulCurrentEntries;
    uint32_t ulNewEntries;
    uint32_t *pulCurrent;
    uint32_t *pulStart;

    /* Save the current number of entries, will update number for this list
     * at the end. */
    BRDC_List_GetNumEntries_isr(hList, &ulCurrentEntries);

    /* get pointer to list entries */
    pulStart = pulCurrent =
        BRDC_List_GetStartAddress_isr(hList) + ulCurrentEntries;

    /* Valid start address */
    BDBG_ASSERT(pulStart);
    BVDC_P_BUILD_NO_OPS(pulCurrent);

    /* Update entries count */
    ulNewEntries = (uint32_t)(pulCurrent - pulStart);

    BRDC_List_SetNumEntries_isr(hList, ulCurrentEntries + ulNewEntries);
    return;
}


/***************************************************************************
 * Get the currently list pointer and store in pList!  Including the
 * last executed status.
 *
 */
void BVDC_P_ReadListInfo_isr
    ( BVDC_P_ListInfo                 *pList,
      BRDC_List_Handle                 hList )
{
    uint32_t ulNumEntries;

    /* Read list info once!  This prevent calling into RDC multiple times. */
                               BRDC_List_GetNumEntries_isr(hList, &ulNumEntries);
    pList->bLastExecuted     = BRDC_List_GetLastExecStatus_isr(hList);
    pList->pulStart          = BRDC_List_GetStartAddress_isr(hList);
    pList->pulCurrent        = pList->pulStart + ulNumEntries;
    return;
}


/***************************************************************************
 * Update the number of entries from pList to hList!
 *
 */
void BVDC_P_WriteListInfo_isr
    ( const BVDC_P_ListInfo           *pList,
      BRDC_List_Handle                 hList )
{
    BERR_Code eRdcErrCode;
    uint32_t ulNumEntries;

    /* Update the entries again. */
    ulNumEntries = (uint32_t)(pList->pulCurrent - pList->pulStart);
    eRdcErrCode = BRDC_List_SetNumEntries_isr(hList, ulNumEntries);

    /* Got to make sure we allocated enough RUL mem!  Something that must be
     * done in BVDC_Open(), and instrumented with tests runs! */
    BDBG_ASSERT(BERR_SUCCESS == eRdcErrCode);
    BSTD_UNUSED(eRdcErrCode);
    return;
}


/***************************************************************************
 * BVDC_P_Dither_Init
 * This function init the DITHER_LFSR_INIT and DITHER_LFSR_CTRL
 */
#if (BVDC_P_MFD_SUPPORT_10BIT_DITHER)
void BVDC_P_Dither_Setting_isr
    ( BVDC_P_DitherSetting            *pDither,
      bool                             bDitherEn,
      uint32_t                         ulLfsrInitVale,
      uint32_t                         ulScale )
{
    uint32_t ulMode = (bDitherEn) ?
            BCHP_FIELD_ENUM(MFD_0_DITHER_CTRL, MODE, DITHER) :
            BCHP_FIELD_ENUM(MFD_0_DITHER_CTRL, MODE, ROUNDING);

    pDither->ulCtrlReg =
        ulMode |
        BCHP_FIELD_DATA(MFD_0_DITHER_CTRL, OFFSET_CH2,       0) |
        BCHP_FIELD_DATA(MFD_0_DITHER_CTRL, SCALE_CH2,  ulScale) |
        BCHP_FIELD_DATA(MFD_0_DITHER_CTRL, OFFSET_CH1,       0) |
        BCHP_FIELD_DATA(MFD_0_DITHER_CTRL, SCALE_CH1,  ulScale) |
        BCHP_FIELD_DATA(MFD_0_DITHER_CTRL, OFFSET_CH0,       0) |
        BCHP_FIELD_DATA(MFD_0_DITHER_CTRL, SCALE_CH0,  ulScale);

    if(bDitherEn)
    {
        pDither->ulLfsrInitReg =
            BCHP_FIELD_ENUM(MFD_0_DITHER_LFSR_INIT, SEQ,     ONCE_PER_SOP) |
            BCHP_FIELD_DATA(MFD_0_DITHER_LFSR_INIT, VALUE, ulLfsrInitVale);

        pDither->ulLfsrCtrlReg =
            BCHP_FIELD_ENUM(MFD_0_DITHER_LFSR_CTRL, T0,  B3) |
            BCHP_FIELD_ENUM(MFD_0_DITHER_LFSR_CTRL, T1,  B8) |
            BCHP_FIELD_ENUM(MFD_0_DITHER_LFSR_CTRL, T2, B12);
    }
}
#else
void BVDC_P_Dither_Setting_isr
    ( BVDC_P_DitherSetting            *pDither,
      bool                             bDitherEn,
      uint32_t                         ulLfsrInitVale,
      uint32_t                         ulScale )
{
    BSTD_UNUSED(pDither);
    BSTD_UNUSED(bDitherEn);
    BSTD_UNUSED(ulLfsrInitVale);
    BSTD_UNUSED(ulScale);
}
#endif

/***************************************************************************
 * BVDC_P_CompositorDisplay_isr
 *
 * This get call at every display vsync trigger a slot w/ done execution.
 * We're then building the RUL for the next field/frame.
 *
 * pvCompositorHandle contains hCompositor
 * (iParam2) = BAVC_Polarity_eTopField /BAVC_Polarity_eBotField;
 */
void BVDC_P_CompositorDisplay_isr
    ( void                            *pvCompositorHandle,
      int                              iParam2 )
{
    uint32_t i;
    BRDC_Slot_Handle          hSlot;
    BRDC_List_Handle          hList;
    BVDC_P_ListInfo           stList;
    BVDC_Compositor_Handle    hCompositor = (BVDC_Compositor_Handle)pvCompositorHandle;
    BVDC_Display_Handle       hDisplay;
    BAVC_Polarity             eNextFieldId;

    BDBG_ENTER(BVDC_P_CompositorDisplay_isr);

    /* Get The compositor handle from isr callback */
    eNextFieldId = BVDC_P_NEXT_POLARITY((BAVC_Polarity)(iParam2));

    /* Build Display RUL & Compositor (playback modules) */
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hCompositor->hDisplay, BVDC_DSP);
    BDBG_OBJECT_ASSERT(hCompositor->hVdc, BVDC_VDC);

    /* Display only have T/B slot. */
    BDBG_ASSERT(BAVC_Polarity_eFrame != (BAVC_Polarity)iParam2);

    /* Make sure the BKNI enter/leave critical section works. */
    BVDC_P_CHECK_CS_ENTER_VDC(hCompositor->hVdc);

    hDisplay = hCompositor->hDisplay;
    if((BVDC_P_ItState_eNotActive  == hDisplay->eItState) ||
       (BVDC_P_ItState_eSwitchMode == hDisplay->eItState))
    {
        /* Detected the force execution, or mode switch reset of vec. */
        uint32_t ulVecResetDetected = BRDC_ReadScratch_isrsafe(hCompositor->hVdc->hRegister,
            hDisplay->ulRdcVarAddr);

        /* If ulVecResetDetected it means that the last RDC executed a VEC's
         * reset.  At this point we want to change the state of the vec. */
        if(ulVecResetDetected)
        {
            /* Acknowledge reset. */
            BRDC_WriteScratch_isrsafe(hCompositor->hVdc->hRegister, hDisplay->ulRdcVarAddr, 0);
            BDBG_MSG(("Display[%d]'s state: %s (%s => %d)", hCompositor->eId,
                (BVDC_P_ItState_eNotActive == hDisplay->eItState) ? "eNotActive" :
                (BVDC_P_ItState_eSwitchMode == hDisplay->eItState) ? "eSwitchMode" : "eActive",
                (BAVC_Polarity_eTopField==iParam2)?"T":
                (BAVC_Polarity_eBotField==iParam2)?"B":"F", ulVecResetDetected));
            if((BVDC_P_ItState_eSwitchMode == hDisplay->eItState) ||
               (BVDC_P_ItState_eNotActive  == hDisplay->eItState))
            {
                hDisplay->eItState = BVDC_P_ItState_eActive;
            }

            /* Clean up Top/Bot display slot/list */
            BVDC_P_CMP_NEXT_RUL(hCompositor, BAVC_Polarity_eTopField);
            hList = BVDC_P_CMP_GET_LIST(hCompositor, BAVC_Polarity_eTopField);

            /* disable execution tracking for now to not lose previous critical tracking status.
             */
            BRDC_Slot_UpdateLastRulStatus_isr(hCompositor->ahSlot[BAVC_Polarity_eTopField], hList, false);
            BRDC_Slot_UpdateLastRulStatus_isr(hCompositor->ahSlot[BAVC_Polarity_eBotField], hList, false);
            BRDC_List_SetNumEntries_isr(hList, 0);
            BVDC_P_BuildNoOpsRul_isr(hList);

            /* set same RUL to both display t/b slots */
            BRDC_Slots_SetList_isr(hCompositor->ahSlot, hList, BVDC_P_CMP_MAX_SLOT_COUNT);

            /* Re-enable execution tracking.
             */
            BRDC_Slot_UpdateLastRulStatus_isr(hCompositor->ahSlot[BAVC_Polarity_eTopField], hList, true);
            BRDC_Slot_UpdateLastRulStatus_isr(hCompositor->ahSlot[BAVC_Polarity_eBotField], hList, true);

            /* Clean up Top/Bot/Frame slot/list of sync-lock source. */
            if(hCompositor->hSyncLockSrc)
            {
                BVDC_P_Source_CleanupSlots_isr(hCompositor->hSyncLockSrc);

                /* clean up mpeg PIP source slots as well if it's not locked */
                if(hCompositor->hForceTrigPipSrc)
                {
                    BVDC_P_Source_CleanupSlots_isr(hCompositor->hForceTrigPipSrc);
                }
            }
            /* Re-enable triggers. */
            BVDC_P_Display_EnableTriggers_isr(hDisplay, true);
            goto BVDC_P_CompositorDisplay_isr_Done;
        }
    }

    /* Check if we're doing frame.  If we're doing frame we're use a topfield
     * slot to trigger the frame slot in source isr for sycn lock. */
    if((!hDisplay->stCurInfo.pFmtInfo->bInterlaced)
#if (BVDC_P_SUPPORT_IT_VER >= 2)
       && (2 != hDisplay->stCurInfo.ulTriggerModuloCnt)
#endif
    )
    {
        eNextFieldId = BAVC_Polarity_eFrame;
        BVDC_P_CMP_NEXT_RUL(hCompositor, BAVC_Polarity_eTopField);
        hList = BVDC_P_CMP_GET_LIST(hCompositor, BAVC_Polarity_eTopField);
    }
    else
    {
        /* eNextField can only be either eTopField or eBotField once
         * we get here. The following line fixes a Coverity warning.
         */
        eNextFieldId = (BAVC_Polarity_eBotField == eNextFieldId) ?
                    BAVC_Polarity_eBotField : BAVC_Polarity_eTopField;
        /* Get the approriate slot/list for building RUL. */
        BVDC_P_CMP_NEXT_RUL(hCompositor, eNextFieldId);
        hList = BVDC_P_CMP_GET_LIST(hCompositor, eNextFieldId);
    }
    /* always set RUL to both t/b display slots, so it doesn't matter which slot to pick here for assignment;
       NOTE: the only significance of slot polarity is to tell the trigger/interrupt cadence polarity! */
    hSlot      = BVDC_P_CMP_GET_SLOT(hCompositor, BAVC_Polarity_eTopField);

    /* Reset the RUL entry count and build RUL for backend! */
    /* Always tracking execution will simplify a lot of
     * cases where we check the bLastExecuted flag. It's not
     * to bad since the size of compositor side RUL for sync-locked
     * source path is short anyway.
     *
     */
    BRDC_Slot_UpdateLastRulStatus_isr(hSlot, hList, true);
    BRDC_List_SetNumEntries_isr(hList, 0);
    BVDC_P_ReadListInfo_isr(&stList, hList);
    stList.hSlot = hSlot;

    if((BVDC_P_ItState_eSwitchMode == hDisplay->eItState) ||
        (true == hDisplay->bAlignAdjusting  && !hDisplay->stCurInfo.stAlignCfg.bKeepBvnConnected))
    {
        /* When switching modes or doing VEC alignment, build the Vec only.
         * Do not enable the front-end blocks yet. When doing VEC alignment VEC
         * triggers can be fired earlier than normal cases. This may cause
         * BVN unable to finish processing the picture and result in BVN hang.
         */
        if(BVDC_P_ItState_eSwitchMode == hDisplay->eItState)
        {
            BDBG_MSG(("Vec mode switch[%s]: %s to %s",
                (BAVC_Polarity_eTopField == eNextFieldId) ? "T" :
                (BAVC_Polarity_eBotField == eNextFieldId) ? "B" : "F",
                hDisplay->stCurInfo.pFmtInfo->pchFormatStr,
                hDisplay->stNewInfo.pFmtInfo->pchFormatStr));
        }

        /* Reset VEC as long as it is in switch mode */
        BVDC_P_Vec_BuildRul_isr(hDisplay, &stList, eNextFieldId);
    }
    else
    {
#if BVDC_P_SUPPORT_STG
        bool bSyncSlave = false;
#endif

        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            if(hCompositor->ahWindow[i])
            {
#ifndef BVDC_FOR_BOOTUPDATER
                BVDC_Source_Handle hSource;
#endif

                /* Window detect window destroy done and set event. */
                BVDC_P_Window_UpdateState_isr(hCompositor->ahWindow[i]);

#if BVDC_P_SUPPORT_STG
                /* check if this compositor becomes sync slaved */
                bSyncSlave |= hCompositor->ahWindow[i]->bSyncSlave;
#endif

#ifndef BVDC_FOR_BOOTUPDATER
                /* invoke BVDC_P_Source_VfdGfxDataReady_isr for VFD that inputs gfx
                 * surface and works as an initial source of the video window, to
                 * perform the rectangle adjustment, vnet decision, picture node
                 * setup, and misc stuff that normally done in the source interrupt
                 * _isr, such as MpegDataReady_isr and AnalogDataReady_isr.
                 * note: in this case there is no source interrupt, and one vfd can
                 * not be shared by more than one window. */
                hSource = hCompositor->ahWindow[i]->stCurInfo.hSource;
                if (hSource && hSource->hVfdFeeder &&
                    BVDC_P_STATE_IS_ACTIVE(hSource))
                {
                    BVDC_P_Source_VfdGfxDataReady_isr(
                        hSource, hCompositor->ahWindow[i], &stList, iParam2);
                }
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
            }
        }

#ifndef BVDC_FOR_BOOTUPDATER
        /* A disconnected window causes an mfd source to be orphan and needs a new
         * compositor to drive it.  Since mfd's slots could be driven by t/b/f,
         * we need to make it complete the last execution before transferring
         * the force trigger (driving the mfd job) to this compositor.  The
         * mfd _isr will release the semaphore (ulTransferLock == 0) to signal it's
         * done cleaning up. */
        if(hCompositor->hSrcToBeLocked)
        {
            BDBG_MSG(("cmp[%d] trying to lock new orphan mfd[%d]",
                hCompositor->eId, hCompositor->hSrcToBeLocked->eId));
            if(!hCompositor->hSrcToBeLocked->ulTransferLock)
            {
                BVDC_P_Source_FindLockWindow_isr(hCompositor->hSrcToBeLocked, true);
            }
            else
            {
                /* This is a backup to prevent infinite wait for lock transfer in case
                 * the critical mpeg callback is missing during the synclock transfer process; */
                BDBG_MSG(("Clean up mfd[%d] slots before transfer to new cmp %d",
                    hCompositor->hSrcToBeLocked->eId, hCompositor->eId));
                if(--hCompositor->hSrcToBeLocked->ulTransferLock == 0)
                {
                    /* clean up source slots to get rid of leftover source RULs for old
                       source/windows config; */
                    BVDC_P_Source_CleanupSlots_isr(hCompositor->hSrcToBeLocked);
                }
            }
        }

        /* If Compositor/Display has at least one sync-lock source it will be the
         * source isr that build the RUL.  This return true if it has sync-lock
         * source.  In that case it will just build the RUL to force trigger the
         * source slot. */
        if(hCompositor->hSyncLockSrc)
        {
            /* this is to prevent gfx flash for slip2lock transitioned display */
            if(hCompositor->ulSlip2Lock)
            {
                BDBG_MSG(("cmp[%d] build slip parts for mfd[%d]",
                    hCompositor->eId, hCompositor->hSyncLockSrc->eId));
                BVDC_P_Compositor_BuildSyncSlipRul_isr(hCompositor, &stList, eNextFieldId, true);
            }
            BVDC_P_Compositor_BuildSyncLockRul_isr(hCompositor, &stList, eNextFieldId);
        }
#if BVDC_P_SUPPORT_STG
        else if(bSyncSlave) {/* sync slaved cmp builds nop RUL as its real RUL is built by the sync-locked source isr */
            BVDC_P_BUILD_NO_OPS(stList.pulCurrent);
            if(!hCompositor->bSyncSlave) {
                hCompositor->bSyncSlave = true;
                BDBG_MSG(("cmp[%u] enters sync slave mode.", hCompositor->eId));
            }
        }
#endif
        else
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
        {
            BVDC_P_Compositor_BuildSyncSlipRul_isr(hCompositor, &stList, eNextFieldId, true);
#if BVDC_P_SUPPORT_STG /* first simul sync slaved display grabs the sync-lock needs to build syncLock RUL to trigger source */
            if(hCompositor->hCmpToLock) {/* in case this display isr was the last executed, it needs to build lock RUL for the new sync-locked display */
                BVDC_P_ListInfo stTmpList;
                BRDC_List_Handle hTmpList;

                /* single RUL assigned to both cmp slots. */
                BVDC_P_CMP_NEXT_RUL(hCompositor->hCmpToLock, BAVC_Polarity_eTopField);
                hTmpList = BVDC_P_CMP_GET_LIST(hCompositor->hCmpToLock, BAVC_Polarity_eTopField);

                /* Reset the RUL entry count and build synclock display RUL! */
                BRDC_Slot_UpdateLastRulStatus_isr(hCompositor->hCmpToLock->ahSlot[BAVC_Polarity_eTopField], hTmpList, true);/* top slot */
                BRDC_Slot_UpdateLastRulStatus_isr(hCompositor->hCmpToLock->ahSlot[BAVC_Polarity_eBotField], hTmpList, true);/* bot slot */
                BRDC_List_SetNumEntries_isr(hTmpList, 0);
                BVDC_P_ReadListInfo_isr(&stTmpList, hTmpList);

                BVDC_P_Compositor_BuildSyncLockRul_isr(hCompositor->hCmpToLock, &stTmpList, eNextFieldId);

                /* Updated lists count and assign the RUL to both t/b display slots */
                BVDC_P_WriteListInfo_isr(&stTmpList, hTmpList);
                BRDC_Slots_SetList_isr(hCompositor->hCmpToLock->ahSlot, hTmpList, BVDC_P_CMP_MAX_SLOT_COUNT); /* two slots per display */

                BDBG_MSG(("CMP[%u] transfers sync lock to CMP[%u]", hCompositor->eId, hCompositor->hCmpToLock->eId));
                hCompositor->hCmpToLock = NULL;
            }
#endif
        }

#ifndef BVDC_FOR_BOOTUPDATER
        /* Poll for unplug sources. */
        for(i = 0; i < BVDC_P_MAX_SOURCE_COUNT; i++)
        {
            /* Is this the compositor going to handle polling? */
            if(hCompositor != hCompositor->hVdc->hCmpCheckSource)
            {
                break;
            }

            /* This compositor will check for source that lost triggers or
             * otherwise no longer function. */
            if((BVDC_P_STATE_IS_ACTIVE(hCompositor->hVdc->ahSource[i])) &&
               (hCompositor->hVdc->ahSource[i]->hTrigger0Cb) &&
               (hCompositor->hVdc->ahSource[i]->hTrigger1Cb))
            {
                BVDC_Source_Handle hSource = hCompositor->hVdc->ahSource[i];
                BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
                /* does the source still have control? */
                if((BVDC_P_TriggerCtrl_eSource == hSource->eTrigCtrl) &&
                   (hSource->ulVecTrigger))
                {
                    hSource->ulVecTrigger--;
                    if(!hSource->ulVecTrigger)
                    {
                        uint32_t i;
                        /* Remove the triggers from the slots, because VEC is
                         * going generate artificial triggers. */
                        for(i = 0; i < hSource->ulSlotUsed; i++)
                        {
                            BRDC_Slot_ExecuteOnTrigger_isr(hSource->ahSlot[i],
                                BRDC_Trigger_UNKNOWN, true);
                        }

                        /* start the faked field trigger from display; */
                        hSource->eNextFieldFake = hSource->eNextFieldIntP;

                        /* Make sure the slot are clean, especially if the last
                         * RUL is the one that cause the source to lose triggers. */
                        BVDC_P_Source_CleanupSlots_isr(hSource);

                        /* Turn trigger _isr callback to detect when trigger
                         * cames back. */
                        hSource->eTrigCtrl    = BVDC_P_TriggerCtrl_eDisplay;
                        hSource->ulVecTrigger = BVDC_P_TRIGGER_LOST_THRESHOLD;
                        BINT_ClearCallback_isr(hSource->hTrigger0Cb);
                        BINT_ClearCallback_isr(hSource->hTrigger1Cb);
                        BINT_EnableCallback_isr(hSource->hTrigger0Cb);
                        BINT_EnableCallback_isr(hSource->hTrigger1Cb);
                        BDBG_WRN(("(D) Display[%d] acquires control of source[%d]'s slots",
                            hDisplay->eId, hSource->eId));
                    }
                }
                else if(BVDC_P_TriggerCtrl_eXfering == hSource->eTrigCtrl)
                {
                    hSource->eTrigCtrl = BVDC_P_TriggerCtrl_eSource;
                }
                else if(BVDC_P_TriggerCtrl_eDisplay == hSource->eTrigCtrl)
                {
                    /* Force trigger the current source's format fieldid. */
                    BVDC_P_BUILD_IMM_EXEC_OPS(stList.pulCurrent,
                        hSource->aulImmTriggerAddr[hSource->eNextFieldFake]);
                    /* advance the faked field trigger from display; */
                    hSource->eNextFieldFake = BVDC_P_NEXT_POLARITY(hSource->eNextFieldFake);
                }
            }
        }
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
    }

    /* Updated lists count */
    BVDC_P_WriteListInfo_isr(&stList, hList);

    /* Note: assign the same RUL to both t/b slots of a display at once; */
    BRDC_Slots_SetList_isr(hCompositor->ahSlot, hList, BVDC_P_CMP_MAX_SLOT_COUNT);

#if BVDC_P_SUPPORT_STG
    /* NRT STG host arm if this display isr builds the STG RUL. */
    /* only host arm for sync-slipped display, or the first time when slip transitioned to lock;
     the reason not to host arm the 2nd time during transition is to avoid NRT trigger firing early
     such that when source isr replaces sync-locked display slots with dummy RUL and before
     the source slot is installed with any meaningful RUL the NRT trigger might fire dummy nop
     RUL at both display and source slots which might stop the NRT trigger forever! */
    if(BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg) &&
       (hDisplay->stCurInfo.bStgNonRealTime || (hDisplay->stStgChan.bModeSwitch)))
    {
        if(!hCompositor->hSyncLockSrc || (hCompositor->ulSlip2Lock==1))
        {
            if(hCompositor->ulSlip2Lock==1) hCompositor->ulSlip2Lock++;
            BREG_Write32_isr(hDisplay->hVdc->hRegister, BCHP_VIDEO_ENC_STG_0_HOST_ARM + hDisplay->ulStgRegOffset, 1);
            BDBG_MSG(("Disp[%u] isr wrote host arm, pol[%u]", hDisplay->eId, iParam2));
        }else if(hCompositor->hSyncLockSrc)
        {
            uint32_t ulStgTriggerToBeArmed = hCompositor->hDisplay->ulStgTriggerToBeArmed;

            if(ulStgTriggerToBeArmed)
            {
                /* make sure source isr done before set host arm*/
                BREG_Write32_isr(hDisplay->hVdc->hRegister, BCHP_VIDEO_ENC_STG_0_HOST_ARM + hDisplay->ulStgRegOffset, 1);
                BDBG_MSG(("DISP[%u] isr wrote host arm pol[%u]", hDisplay->eId, iParam2));
                hCompositor->hDisplay->ulStgTriggerToBeArmed = 0;
                hDisplay->stStgChan.bModeSwitch = false;/* clear it if disp isr is later than src isr */
#if BVDC_P_STG_NRT_CADENCE_WORKAROUND /* for STG hw that cannot repeat trigger polarity */
                hCompositor->hSyncLockSrc->bToggleCadence ^= hCompositor->bStgIgnorePicture; /* every ignore picture needs to toggle stg cadence for workaround */
                BDBG_MSG(("Src[%u] bToggleCad = %u", hCompositor->hSyncLockSrc->eId, hCompositor->hSyncLockSrc->bToggleCadence));
#endif
            }
            else {
                /* write bit 0 to indicate display isr done with programming */
                hCompositor->hDisplay->ulStgTriggerToBeArmed = 1;
                BDBG_MSG(("Disp[%u] to host arm", hDisplay->eId));
            }
        }
    }
#endif

BVDC_P_CompositorDisplay_isr_Done:
    BVDC_P_CHECK_CS_LEAVE_VDC(hCompositor->hVdc);
    BDBG_LEAVE(BVDC_P_CompositorDisplay_isr);
    return;
}

/***************************************************************************
 * This function checks if a callback's dirty bits are dirty.
 */
bool BVDC_P_CbIsDirty_isr
    (void                           *pDirty,
     uint32_t                        ulSize )
{
    bool bIsDirty = false;
    static const uint8_t aulZero[BVDC_P_DIRTY_INT_ARRAY_SIZE * sizeof(uint32_t)]={0};

    BDBG_ASSERT(ulSize <= sizeof(aulZero));

    if (ulSize <= sizeof(aulZero))
    {
        bIsDirty = BKNI_Memcmp_isr(pDirty, aulZero, ulSize)? true : false;
    }

    return bIsDirty;
}

/***************************************************************************
 *
 * Utility function called by BVDC_P_Window_AspectRatioCorrection_isr and
 * BVDC_P_Window_CalcuUserDisplaySize_isr, and BVDC_P_Display_CalPixelAspectRatio_isr
 * to calculate the U4.16 fixed point format aspect ratio of a PIXEL
 *
 * note: pixel asp ratio range is well bounded ( <16, i.e. 4 int bits ), so
 * calcu it first, and also it could have more frac bits than the asp ratio
 * of a sub-rect (that is not well bounded).
 */
void BVDC_P_CalcuPixelAspectRatio_isr(
    BFMT_AspectRatio                 eFullAspectRatio,     /* full asp ratio enum */
    uint32_t                         ulSampleAspectRatioX, /* width of one sampled src pixel */
    uint32_t                         ulSampleAspectRatioY, /* height of one sampled src pixel */
    uint32_t                         ulFullWidth,          /* full asp ratio width */
    uint32_t                         ulFullHeight,         /* full asp ratio height */
    const BVDC_P_ClipRect *          pAspRatCnvsClip,      /* asp rat cnvs clip */
    uintAR_t *                       pulPxlAspRatio,       /* PxlAspR_int.PxlAspR_frac */
    uint32_t *                       pulPxlAspRatio_x_y,   /* PxlAspR_x<<16 | PxlAspR_y */
    BFMT_Orientation                 eOrientation          /* orientation of the input stream  */
)
{
    uint32_t  ulAspRatCnvsWidth, ulAspRatCnvsHeight;
    uintAR_t  ulPixAspRatio = 0;
    uint16_t  uiPixAspR_x=0, uiPixAspR_y=0;
    uint32_t b=0, a=0, m=0, i=0;
    BDBG_ASSERT((NULL != pulPxlAspRatio) && (NULL != pulPxlAspRatio_x_y));
    BDBG_ASSERT(NULL != pAspRatCnvsClip);

    ulFullWidth  <<= (eOrientation == BFMT_Orientation_e3D_LeftRight);
    ulFullHeight <<= (eOrientation == BFMT_Orientation_e3D_OverUnder);
    ulAspRatCnvsWidth  = ulFullWidth  - (pAspRatCnvsClip->ulLeft + pAspRatCnvsClip->ulRight);
    ulAspRatCnvsHeight = ulFullHeight - (pAspRatCnvsClip->ulTop  + pAspRatCnvsClip->ulBottom);

    /* Set default value for unknown aspect ratio. */
    if(BVDC_P_IS_UNKNOWN_ASPR(eFullAspectRatio, ulSampleAspectRatioX, ulSampleAspectRatioY))
    {
        uint32_t ulHVRatio = (ulFullWidth * 100) / ulFullHeight;
        eFullAspectRatio = BVDC_P_EQ_DELTA(ulHVRatio, 130, 25)
            ? BFMT_AspectRatio_e4_3 : BFMT_AspectRatio_eSquarePxl;
    }

    /* Pay attention to overflow, assuming ulAspRatCnvsHeight could be as big as 1080 */
    switch (eFullAspectRatio)
    {
    case BFMT_AspectRatio_eSquarePxl:
        BDBG_MSG(("BFMT_AspectRatio_eSquarePxl"));
        uiPixAspR_x = 1;
        uiPixAspR_y = 1;
        break;
    case BFMT_AspectRatio_e4_3:
        BDBG_MSG(("BFMT_AspectRatio_e4_3"));
        uiPixAspR_x = ulAspRatCnvsHeight * 4;
        uiPixAspR_y = ulAspRatCnvsWidth * 3;
        break;
    case BFMT_AspectRatio_e16_9:
        BDBG_MSG(("BFMT_AspectRatio_e16_9"));
        uiPixAspR_x = ulAspRatCnvsHeight * 16;
        uiPixAspR_y = ulAspRatCnvsWidth * 9;
        break;
    case BFMT_AspectRatio_e221_1:
        BDBG_MSG(("BFMT_AspectRatio_e221_1"));
        uiPixAspR_x = (ulAspRatCnvsHeight * 221) >> 3;
        uiPixAspR_y = (ulAspRatCnvsWidth  * 100) >> 3;
        ulPixAspRatio = ((uintAR_t)ulAspRatCnvsHeight << BVDC_P_ASPR_FRAC_BITS_NUM) * 2 / ulAspRatCnvsWidth +
                        ((uintAR_t)ulAspRatCnvsHeight << BVDC_P_ASPR_FRAC_BITS_NUM) * 21 / (100 * ulAspRatCnvsWidth);
        break;
    case BFMT_AspectRatio_e15_9:
        BDBG_MSG(("BFMT_AspectRatio_e15_9"));
        uiPixAspR_x = ulAspRatCnvsHeight * 15;
        uiPixAspR_y = ulAspRatCnvsWidth * 9;
        break;
    case BFMT_AspectRatio_eSAR:
        BDBG_MSG(("BFMT_AspectRatio_eSAR: %d, %d", ulSampleAspectRatioX, ulSampleAspectRatioY));
        uiPixAspR_x = ulSampleAspectRatioX <<(eOrientation == BFMT_Orientation_e3D_LeftRight);
        uiPixAspR_y = ulSampleAspectRatioY <<(eOrientation == BFMT_Orientation_e3D_OverUnder);
        break;
    default:
        uiPixAspR_x = 1;
        uiPixAspR_y = 1;
        BDBG_ERR(("Bad asp ratio enum %d", eFullAspectRatio));
        break;
    }

    if(uiPixAspR_y == uiPixAspR_x)
    {
        uiPixAspR_y = uiPixAspR_x = 1;
    }
    /* Euclidean gcd algorithm */
    else
    {
        a = uiPixAspR_y > uiPixAspR_x ? uiPixAspR_y:uiPixAspR_x;
        b = uiPixAspR_y > uiPixAspR_x ? uiPixAspR_x:uiPixAspR_y;

        while (b  && (i<10)) { m = a % b; a = b; b = m; i++;}

        if (i<10) {
            uiPixAspR_y/=a;
            uiPixAspR_x/=a;
        }
    }

    if (BFMT_AspectRatio_e221_1 != eFullAspectRatio)
    {
        /* use multiplication to avoid endian dependent error */
        ulPixAspRatio = ((uintAR_t)uiPixAspR_x << BVDC_P_ASPR_FRAC_BITS_NUM) / (uiPixAspR_y);
    }

    *pulPxlAspRatio = ulPixAspRatio;
    *pulPxlAspRatio_x_y = ((uint32_t)uiPixAspR_x <<16) | uiPixAspR_y;

    BDBG_MSG(("pxlAspR: %x %x", (unsigned int)*pulPxlAspRatio, (unsigned int)*pulPxlAspRatio_x_y));
}

/***************************************************************************
 * This function gets number of CMPs.
 */
uint32_t BVDC_P_GetNumCmp
    ( const BVDC_P_Features                 *pFeatures )
{
    uint32_t   i, ulNumCmp = 0;

    for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
    {
        if(pFeatures->abAvailCmp[i])
        {
            ulNumCmp++;
        }
    }

    return ulNumCmp;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_GetCapabilities
    ( BVDC_Handle                      hVdc,
      BVDC_Capabilities               *pCapabilities )
{
    BDBG_ENTER(BVDC_GetCapabilities);

    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    if(pCapabilities)
    {
        BKNI_Memset(pCapabilities, 0x0, sizeof(BVDC_Capabilities));

        pCapabilities->ulNumBox = BVDC_P_SUPPORT_BOX_DETECT;
        pCapabilities->ulNumCmp = BVDC_P_GetNumCmp(hVdc->pFeatures);
        pCapabilities->ulNumMad = BVDC_P_SUPPORT_MCVP;
        pCapabilities->ulNumDnr = BVDC_P_SUPPORT_DNR;
        pCapabilities->ulNumPep = 0;
        pCapabilities->ulNumTab = BVDC_P_SUPPORT_TNT;
        pCapabilities->ulNumDac = BVDC_P_MAX_DACS;
        pCapabilities->ulNumRfm = BVDC_P_SUPPORT_RFM_OUTPUT;
        pCapabilities->ulNumStg = BVDC_P_SUPPORT_STG;
        pCapabilities->ulNumAtg = BVDC_P_NUM_SHARED_IT;
        pCapabilities->ulNumAlgPaths = BVDC_P_NUM_SHARED_VF;
        pCapabilities->bAlgStandAlone = (BVDC_P_SUPPORT_IT_VER >= 3) ? true : false;

        pCapabilities->ulNumCap = BVDC_P_SUPPORT_CAP;
        pCapabilities->ulNumVfd = BVDC_P_SUPPORT_VFD;
        pCapabilities->ulNumMfd = BVDC_P_SUPPORT_MFD;

        pCapabilities->ulNum656Input   = BVDC_P_NUM_656IN_SUPPORT;
        pCapabilities->ulNum656Output  = BVDC_P_SUPPORT_ITU656_OUT;

        pCapabilities->ulNumHdmiInput  = BVDC_P_SUPPORT_HDDVI;
        pCapabilities->ulNumHdmiOutput = BVDC_P_SUPPORT_DVI_OUT;


        pCapabilities->b3DSupport      = hVdc->pFeatures->ab3dSrc[0];   /* mpeg0 support 3d or not */
        pCapabilities->b64BitSupport   = BRDC_64BIT_SUPPORT;
    }

    BDBG_LEAVE(BVDC_GetCapabilities);
    return BERR_SUCCESS;
}


/***************************************************************************
 *
 * Utility function called by BVDC_Source_GetCapabilities
 */
bool  BVDC_P_IsPxlfmtSupported
    (BPXL_Format                       ePxlFmt)
{

    /* Can support all formats with MFD_SUPPORT_BYTE_ORDER */
    return BVDC_P_VALID_PIXEL_FORMAT(ePxlFmt);
}


/***************************************************************************
 * Check Heap settings.
 *
 * 1) SD buffer format is SD
 * 2) HD buffer format is HD
 * 3) 2HD buffer format is  HD
 * 4) SD buffer < HD buffer
 * 5) HD buffer < 2HD buffer
 *
 */
BERR_Code BVDC_P_CheckHeapSettings
    ( const BVDC_Heap_Settings         *pHeapSettings )
{
    BERR_Code  eStatus = BERR_SUCCESS;
    uint32_t   ulWidth, ulHeight;
    uint32_t   ulSDBufSize, ulHDBufSize, ul2HDBufSize, ul4HDBufSize;
    const BVDC_P_FormatInfo  *pVdcFmt;
    const BFMT_VideoInfo     *pFmtInfo;

    /* 0) Check pixel format */
    if(!BVDC_P_VALID_PIXEL_FORMAT(pHeapSettings->ePixelFormat_SD))
    {
        BDBG_ERR(("Not a valid SD Pixel format %s ",
            BPXL_ConvertFmtToStr(pHeapSettings->ePixelFormat_SD)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if(!BVDC_P_VALID_PIXEL_FORMAT(pHeapSettings->ePixelFormat_HD))
    {
        BDBG_ERR(("Not a valid HD Pixel format %s ",
            BPXL_ConvertFmtToStr(pHeapSettings->ePixelFormat_HD)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if(!BVDC_P_VALID_PIXEL_FORMAT(pHeapSettings->ePixelFormat_2HD))
    {
        BDBG_ERR(("Not a valid 2HD Pixel format %s ",
            BPXL_ConvertFmtToStr(pHeapSettings->ePixelFormat_2HD)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if(!BVDC_P_VALID_PIXEL_FORMAT(pHeapSettings->ePixelFormat_4HD))
    {
        BDBG_ERR(("Not a valid 4HD Pixel format %s ",
            BPXL_ConvertFmtToStr(pHeapSettings->ePixelFormat_4HD)));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* 1) SD buffer format is SD */
    pVdcFmt = BVDC_P_GetFormatInfo_isrsafe(pHeapSettings->eBufferFormat_SD);
    if(!pVdcFmt->bSd)
    {
        pFmtInfo = BFMT_GetVideoFormatInfoPtr(pVdcFmt->eVideoFmt);
        BDBG_ERR(("SD buffer format (%s) is not SD", pFmtInfo->pchFormatStr));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* 2) HD buffer format is HD */
    pVdcFmt = BVDC_P_GetFormatInfo_isrsafe(pHeapSettings->eBufferFormat_HD);
    if(!pVdcFmt->bHd)
    {
        pFmtInfo = BFMT_GetVideoFormatInfoPtr(pVdcFmt->eVideoFmt);
        BDBG_ERR(("HD buffer format (%s) is not HD", pFmtInfo->pchFormatStr));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* 3) 2Hd buffer format is HD */
    pVdcFmt = BVDC_P_GetFormatInfo_isrsafe(pHeapSettings->eBufferFormat_2HD);
    if(!pVdcFmt->bHd)
    {
        pFmtInfo = BFMT_GetVideoFormatInfoPtr(pVdcFmt->eVideoFmt);
        BDBG_ERR(("2HD buffer format (%s) is not HD", pFmtInfo->pchFormatStr));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* 4) 4Hd buffer format is HD */
    pVdcFmt = BVDC_P_GetFormatInfo_isrsafe(pHeapSettings->eBufferFormat_4HD);
    if(!pVdcFmt->bHd)
    {
        pFmtInfo = BFMT_GetVideoFormatInfoPtr(pVdcFmt->eVideoFmt);
        BDBG_ERR(("4HD buffer format (%s) is not HD", pFmtInfo->pchFormatStr));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Get SD, HD, 2HD, 4HD buffer size */
    ulSDBufSize = BVDC_P_BufferHeap_GetHeapSize(
        BFMT_GetVideoFormatInfoPtr(pHeapSettings->eBufferFormat_SD),
        pHeapSettings->ePixelFormat_SD, false, NULL, &ulWidth, &ulHeight);
    ulHDBufSize = BVDC_P_BufferHeap_GetHeapSize(
        BFMT_GetVideoFormatInfoPtr(pHeapSettings->eBufferFormat_HD),
        pHeapSettings->ePixelFormat_HD, false, NULL, &ulWidth, &ulHeight);
    ul2HDBufSize = BVDC_P_BufferHeap_GetHeapSize(
        BFMT_GetVideoFormatInfoPtr(pHeapSettings->eBufferFormat_2HD),
        pHeapSettings->ePixelFormat_2HD, false, NULL, &ulWidth, &ulHeight);
    ul4HDBufSize = BVDC_P_BufferHeap_GetHeapSize(
        BFMT_GetVideoFormatInfoPtr(pHeapSettings->eBufferFormat_4HD),
        pHeapSettings->ePixelFormat_4HD, false, NULL, &ulWidth, &ulHeight);

    /* 5) SD buffer < HD buffer */
    if(!(ulSDBufSize < ulHDBufSize))
    {
        BDBG_ERR(("HD buffer (0x%d) is not bigger than SD buffer (0x%x)",
            ulHDBufSize, ulSDBufSize));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* 6) HD buffer < 2HD buffer */
    if(!(ulHDBufSize < ul2HDBufSize))
    {
        BDBG_ERR(("2HD buffer is not bigger than HD buffer"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* 7) 2HD buffer < 4HD buffer */
    if(!(ul2HDBufSize < ul4HDBufSize))
    {
        BDBG_ERR(("4HD buffer is not bigger than 2HD buffer"));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return eStatus;
}

#ifndef BVDC_FOR_BOOTUPDATER
void BVDC_P_SrcWinClass_Init
    ( BVDC_Handle  hVdc )
{
    int i, j, ulCoveragePerDimension;

    hVdc->pstSrcClassTbl = sa_SourceClassTble;
    hVdc->pstWinClassTbl = sa_WindowClassTble;

    /* for backwards compatible API, assume window0's class to prefill msoaic coverage table */
    for(i = 0; i < BVDC_P_MAX_DISPLAY_COUNT; i++)
    {
        BBOX_Vdc_WindowClass       eClass;
        BBOX_Vdc_MosaicModeClass   eLegacyMosaicModeClass;

        eClass = hVdc->stBoxConfig.stVdc.astDisplay[i].astWindow[0].eClass;
        eLegacyMosaicModeClass = hVdc->stBoxConfig.stVdc.astDisplay[i].eMosaicModeClass;

        for(j = 0; j < BAVC_MOSAIC_MAX; j++)
        {
            /* aulCanvasCoverageEqual */
            if(eClass != BBOX_Vdc_WindowClass_eLegacy)
                ulCoveragePerDimension = sa_WindowClassTble[eClass].mosaicRects[j].ulPercentEqual;
            else if(eLegacyMosaicModeClass == BBOX_VDC_DISREGARD)
                ulCoveragePerDimension = s_full_CCTbl.aulCanvasCoverageEqual[j];
            else
                ulCoveragePerDimension = sa_LegacyMosaicCoverageTble[eLegacyMosaicModeClass].aulCanvasCoverageEqual[j];

            hVdc->stMosaicCoverageTbl[i].aulCanvasCoverageEqual[j] = (ulCoveragePerDimension *ulCoveragePerDimension*(j+1))/100;
            hVdc->stMosaicCoverageTbl[i].aulCanvasCoverageEqual[j]=
                BVDC_P_MIN(hVdc->stMosaicCoverageTbl[i].aulCanvasCoverageEqual[j], 100);

            /* aulCanvasCoverageBL */
            if((eClass == BBOX_Vdc_WindowClass_eLegacy) || (j == 0))
                hVdc->stMosaicCoverageTbl[i].aulCanvasCoverageBL[j]= 0;
            else
            {
                ulCoveragePerDimension = sa_WindowClassTble[eClass].mosaicRects[j].ulPercentBigSmall;

                hVdc->stMosaicCoverageTbl[i].aulCanvasCoverageBL[j] =
                    (ulCoveragePerDimension *ulCoveragePerDimension*(j+4))/100;
                hVdc->stMosaicCoverageTbl[i].aulCanvasCoverageBL[j]=
                    BVDC_P_MIN(hVdc->stMosaicCoverageTbl[i].aulCanvasCoverageBL[j], 100);
            }
        }
    }
}
#endif

BERR_Code BVDC_GetWindowClassLimit
    ( BVDC_Handle                      hVdc,
      BVDC_DisplayId                   eDisplayId,
      BVDC_WindowId                    eWinId,
      BVDC_WindowClassLimits          *pClassLimit )
{
#ifndef BVDC_FOR_BOOTUPDATER
    uint32_t  i;
    BBOX_Vdc_WindowClass   eClass;

    eClass = hVdc->stBoxConfig.stVdc.astDisplay[eDisplayId].astWindow[eWinId].eClass;

    if(pClassLimit)
    {
        *pClassLimit = sa_WindowClassTble[eClass];

        if(eClass == BBOX_Vdc_WindowClass_eLegacy)
        {
            BBOX_Vdc_MosaicModeClass   eLegacyMosaicModeClass;
            eLegacyMosaicModeClass = hVdc->stBoxConfig.stVdc.astDisplay[eDisplayId].eMosaicModeClass;

            /* default to class 0 */
            *pClassLimit = sa_WindowClassTble[BBOX_Vdc_WindowClass_e0_0];
            for(i = 0; i < BAVC_MOSAIC_MAX; i++)
            {
                if(eLegacyMosaicModeClass == BBOX_VDC_DISREGARD)
                {
                    (*pClassLimit).mosaicRects[i].ulPercentEqual = s_full_CCTbl.aulCanvasCoverageEqual[i];
                }
                else
                {
                    (*pClassLimit).mosaicRects[i].ulPercentEqual =
                        sa_LegacyMosaicCoverageTble[eLegacyMosaicModeClass].aulCanvasCoverageEqual[i];
                }
                (*pClassLimit).mosaicRects[i].ulPercentBigSmall = 0;
            }
        }
    }
#else
    BSTD_UNUSED(hVdc);
    BSTD_UNUSED(eDisplayId);
    BSTD_UNUSED(eWinId);
    BSTD_UNUSED(pClassLimit);
#endif

    return BERR_SUCCESS;
}

BERR_Code BVDC_GetSourceClassLimit
    ( BVDC_Handle                      hVdc,
      BAVC_SourceId                    eSrcId,
      BVDC_SourceClassLimits          *pClassLimit )
{
#ifndef BVDC_FOR_BOOTUPDATER
    BBOX_Vdc_SourceClass   eClass;

    eClass = hVdc->stBoxConfig.stVdc.astSource[eSrcId].eClass;

    if(pClassLimit)
    {
        *pClassLimit = sa_SourceClassTble[eClass];
    }
#else
    BSTD_UNUSED(hVdc);
    BSTD_UNUSED(eSrcId);
    BSTD_UNUSED(pClassLimit);
#endif

    return BERR_SUCCESS;
}



/* End of file. */
