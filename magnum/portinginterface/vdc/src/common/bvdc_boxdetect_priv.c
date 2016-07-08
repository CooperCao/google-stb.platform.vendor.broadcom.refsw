/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bstd.h"
#include "bkni.h"
#include "brdc.h"
#include "bvdc.h"
#include "bchp_vnet_f.h"
#include "bchp_mmisc.h"
#include "bvdc_boxdetect_priv.h"
#include "bvdc_resource_priv.h"

#if (0 != BVDC_P_SUPPORT_BOX_DETECT)
#include "bchp_lbox_0.h"

BDBG_MODULE(BVDC_BOXDETECT);
BDBG_OBJECT_ID(BVDC_BOX);

#if (BVDC_P_SUPPORT_BOX_DETECT)
static void BVDC_P_BoxDetect_CutInit_isr
    ( BVDC_P_BoxDetectContext       *pBoxDetect,
      uint32_t                       ulSrcWidth,
      uint32_t                       ulSrcHeight );
#endif

static void BVDC_P_BoxDetect_BuildRul_StatisRead_isr
    ( BVDC_P_BoxDetectContext       *pBoxDetect,
      BVDC_P_PicComRulInfo          *pPicComRulInfo,
      const BVDC_P_Rect             *pSrcOut );

static void BVDC_P_BoxDetect_BuildRul_DrainVnet_isr
    ( BVDC_P_BoxDetectContext       *pBoxDetect,
      BVDC_P_ListInfo               *pList );

static void BVDC_P_BoxDetect_BuildRul_SetEnable_isr
    ( BVDC_P_BoxDetectContext       *pBoxDetect,
      BVDC_P_ListInfo               *pList,
      bool                           bEnable );

static void BVDC_P_BoxDetect_BuildRul_VnetInit_isr
    ( BVDC_P_BoxDetectContext       *pBoxDetect,
      BVDC_P_ListInfo               *pList );

static void BVDC_P_BoxDetect_BuildRul_StatisInit_isr
    ( BVDC_P_BoxDetectContext       *pBoxDetect,
      BVDC_P_ListInfo               *pList );

#ifndef BCHP_VNET_F_LBOX_0_SRC_SOURCE_CCIR_656_0
#define BCHP_VNET_F_LBOX_0_SRC_SOURCE_CCIR_656_0 BCHP_VNET_F_SCL_0_SRC_SOURCE_Output_Disabled
#endif

/*--------------------------------------------------------------------
 *  static uint32_t array to hold contiguous hawdware register values
 *  for box detect configuration
 */
#if BVDC_P_USE_BOX_LUM_SUM
#define BVDC_P_BOX_DETECT_MODE                BCHP_LBOX_0_CONFIGURATION_DETECT_MODE_SUM
#else /* use num-threshold */
#define BVDC_P_BOX_DETECT_MODE                BCHP_LBOX_0_CONFIGURATION_DETECT_MODE_NUMBER
#endif  /* BVDC_P_USE_BOX_LUM_SUM */

#define BVDC_P_HORIZONTAL_START_POINT         0       /* 0 - 2047 */
#define BVDC_P_HORIZONTAL_END_POINT           2047    /* 0 - 2047 */
#define BVDC_P_VERTICAL_START_POINT           2       /* 0 - 2047, skip first 2 rows for macrovision */
#define BVDC_P_VERTICAL_END_POINT             2047    /* 0 - 2047 */
#define BVDC_P_BLACK_PIXEL_LEVEL              0x30    /* 0 - 255 */
#define BVDC_P_TB_BLACK_PIXEL_LEVEL           0x30    /* 0 - 255 */
#define BVDC_P_BLACK_ROW_THRESHOLD_NUM        7       /* 0 - 2047 */
#define BVDC_P_BLACK_COL_THRESHOLD_NUM        7       /* 0 - 1023 */
#define BVDC_P_BLACK_LINE_THRESHOLD_L_SUM     0x1b58  /* 0 - 524287 */
#define BVDC_P_WHITE_LINE_THRESHOLD_L_SUM     0x2328  /* 0 - 524287 */

#define BVDC_P_VIDEO_LR_BLACK_PIXEL_LEVEL     0x30    /* 0 - 255 */
#define BVDC_P_VIDEO_WHITE_ROW_THRESHOLD_NUM  0x20    /* 0 - 2047 */
#define BVDC_P_VIDEO_WHITE_COL_THRESHOLD_NUM  0x15    /* 0 - 1023 */

#define BVDC_P_PC_LR_BLACK_PIXEL_LEVEL        0x23    /* 0 - 255 */
#define BVDC_P_PC_WHITE_ROW_THRESHOLD_NUM     0x10    /* 0 - 2047 */
#define BVDC_P_PC_WHITE_COL_THRESHOLD_NUM     0xc     /* 0 - 1023 */



static const uint32_t s_aulBoxDetectRegCfgForVideo[] =
{
    BCHP_FIELD_ENUM( LBOX_0_CONFIGURATION, TB_HORIZONTAL_REGION_ENABLE, DISABLE ) |
    BCHP_FIELD_ENUM( LBOX_0_CONFIGURATION, TB_VERTICAL_REGION_ENABLE,   DISABLE ) |
    BCHP_FIELD_ENUM( LBOX_0_CONFIGURATION, LR_HORIZONTAL_REGION_ENABLE, DISABLE ) |
    BCHP_FIELD_ENUM( LBOX_0_CONFIGURATION, LR_VERTICAL_REGION_ENABLE,   DISABLE ) |
    BCHP_FIELD_DATA( LBOX_0_CONFIGURATION, DETECT_MODE, BVDC_P_BOX_DETECT_MODE ),

    BCHP_FIELD_DATA( LBOX_0_HORIZONTAL_WINDOW_REGION, HORIZONTAL_START_POINT, BVDC_P_HORIZONTAL_START_POINT ) |
    BCHP_FIELD_DATA( LBOX_0_HORIZONTAL_WINDOW_REGION, HORIZONTAL_END_POINT,   BVDC_P_HORIZONTAL_END_POINT ),

    BCHP_FIELD_DATA( LBOX_0_VERTICAL_WINDOW_REGION, VERTICAL_START_POINT, BVDC_P_VERTICAL_START_POINT ) |
    BCHP_FIELD_DATA( LBOX_0_VERTICAL_WINDOW_REGION, VERTICAL_END_POINT,   BVDC_P_VERTICAL_END_POINT ),

    BCHP_FIELD_DATA( LBOX_0_PIXEL_THRESHOLD, BLACK_LEVEL_LUMA_SUM, BVDC_P_BLACK_PIXEL_LEVEL )    |
    BCHP_FIELD_DATA( LBOX_0_PIXEL_THRESHOLD, TB_BLACK_LEVEL_NTA,   BVDC_P_TB_BLACK_PIXEL_LEVEL ) |
    BCHP_FIELD_DATA( LBOX_0_PIXEL_THRESHOLD, LR_BLACK_LEVEL_NTA,   BVDC_P_VIDEO_LR_BLACK_PIXEL_LEVEL ),

    BCHP_FIELD_DATA( LBOX_0_BLACK_LINE_THRESHOLD_NTA, BLACK_LINE_THRESHOLD_SIZE, BVDC_P_BLACK_ROW_THRESHOLD_NUM ),
    BCHP_FIELD_DATA( LBOX_0_BLACK_COL_THRESHOLD_NTA,  BLACK_COL_THRESHOLD_SIZE,  BVDC_P_BLACK_COL_THRESHOLD_NUM ),
    BCHP_FIELD_DATA( LBOX_0_WHITE_LINE_THRESHOLD_NTA, WHITE_LINE_THRESHOLD_SIZE, BVDC_P_VIDEO_WHITE_ROW_THRESHOLD_NUM ),
    BCHP_FIELD_DATA( LBOX_0_WHITE_COL_THRESHOLD_NTA,  WHITE_COL_THRESHOLD_SIZE,  BVDC_P_VIDEO_WHITE_COL_THRESHOLD_NUM ),

    BCHP_FIELD_DATA( LBOX_0_BLACK_LINE_THRESHOLD_LSUM, BLACK_LINE_THRESHOLD_SIZE, BVDC_P_BLACK_LINE_THRESHOLD_L_SUM ),
    BCHP_FIELD_DATA( LBOX_0_WHITE_LINE_THRESHOLD_LSUM, WHITE_LINE_THRESHOLD_SIZE, BVDC_P_WHITE_LINE_THRESHOLD_L_SUM )
};

static const uint32_t s_aulBoxDetectRegCfgForPc[] =
{
    BCHP_FIELD_ENUM( LBOX_0_CONFIGURATION, TB_HORIZONTAL_REGION_ENABLE, DISABLE ) |
    BCHP_FIELD_ENUM( LBOX_0_CONFIGURATION, TB_VERTICAL_REGION_ENABLE,   DISABLE ) |
    BCHP_FIELD_ENUM( LBOX_0_CONFIGURATION, LR_HORIZONTAL_REGION_ENABLE, DISABLE ) |
    BCHP_FIELD_ENUM( LBOX_0_CONFIGURATION, LR_VERTICAL_REGION_ENABLE,   DISABLE ) |
    BCHP_FIELD_DATA( LBOX_0_CONFIGURATION, DETECT_MODE, BVDC_P_BOX_DETECT_MODE ),

    BCHP_FIELD_DATA( LBOX_0_HORIZONTAL_WINDOW_REGION, HORIZONTAL_START_POINT, BVDC_P_HORIZONTAL_START_POINT ) |
    BCHP_FIELD_DATA( LBOX_0_HORIZONTAL_WINDOW_REGION, HORIZONTAL_END_POINT,   BVDC_P_HORIZONTAL_END_POINT ),

    BCHP_FIELD_DATA( LBOX_0_VERTICAL_WINDOW_REGION, VERTICAL_START_POINT, BVDC_P_VERTICAL_START_POINT ) |
    BCHP_FIELD_DATA( LBOX_0_VERTICAL_WINDOW_REGION, VERTICAL_END_POINT,   BVDC_P_VERTICAL_END_POINT ),

    BCHP_FIELD_DATA( LBOX_0_PIXEL_THRESHOLD, BLACK_LEVEL_LUMA_SUM, BVDC_P_BLACK_PIXEL_LEVEL )    |
    BCHP_FIELD_DATA( LBOX_0_PIXEL_THRESHOLD, TB_BLACK_LEVEL_NTA,   BVDC_P_TB_BLACK_PIXEL_LEVEL ) |
    BCHP_FIELD_DATA( LBOX_0_PIXEL_THRESHOLD, LR_BLACK_LEVEL_NTA,   BVDC_P_PC_LR_BLACK_PIXEL_LEVEL ),

    BCHP_FIELD_DATA( LBOX_0_BLACK_LINE_THRESHOLD_NTA, BLACK_LINE_THRESHOLD_SIZE, BVDC_P_BLACK_ROW_THRESHOLD_NUM ),
    BCHP_FIELD_DATA( LBOX_0_BLACK_COL_THRESHOLD_NTA,  BLACK_COL_THRESHOLD_SIZE,  BVDC_P_BLACK_COL_THRESHOLD_NUM ),
    BCHP_FIELD_DATA( LBOX_0_WHITE_LINE_THRESHOLD_NTA, WHITE_LINE_THRESHOLD_SIZE, BVDC_P_PC_WHITE_ROW_THRESHOLD_NUM ),
    BCHP_FIELD_DATA( LBOX_0_WHITE_COL_THRESHOLD_NTA,  WHITE_COL_THRESHOLD_SIZE,  BVDC_P_PC_WHITE_COL_THRESHOLD_NUM ),

    BCHP_FIELD_DATA( LBOX_0_BLACK_LINE_THRESHOLD_LSUM, BLACK_LINE_THRESHOLD_SIZE, BVDC_P_BLACK_LINE_THRESHOLD_L_SUM ),
    BCHP_FIELD_DATA( LBOX_0_WHITE_LINE_THRESHOLD_LSUM, WHITE_LINE_THRESHOLD_SIZE, BVDC_P_WHITE_LINE_THRESHOLD_L_SUM )
};


#define BVDC_P_NUM_REGS_BOX_DETECT_CONFIG  \
    (sizeof(s_aulBoxDetectRegCfgForVideo) / sizeof(uint32_t))

/* INDEX: BAVC_SourceId to look up LBOX_0_SRC index */
static const uint32_t  s_aulSrcIdToHwIndex[] = {
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_MPEG_Feeder_0,   /* BAVC_SourceId_eMpeg0 = 0,*/
#if (BVDC_P_SUPPORT_BOX_DETECT >= 2)
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_MPEG_Feeder_1,   /* BAVC_SourceId_eMpeg1,    */
#else
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eMpeg1,    */
#endif
#if (BVDC_P_SUPPORT_BOX_DETECT_VER >= 2)
#if (BVDC_P_SUPPORT_BOX_DETECT >= 3)
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_MPEG_Feeder_2,   /* BAVC_SourceId_eMpeg2,    */
#else
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eMpeg2,    */
#endif
#if (BVDC_P_SUPPORT_BOX_DETECT >= 4)
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_MPEG_Feeder_3,   /* BAVC_SourceId_eMpeg3,    */
#else
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eMpeg3,    */
#endif
#if (BVDC_P_SUPPORT_BOX_DETECT >= 5)
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_MPEG_Feeder_4,   /* BAVC_SourceId_eMpeg4,    */
#else
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eMpeg4,    */
#endif
#if (BVDC_P_SUPPORT_BOX_DETECT >= 6)
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_MPEG_Feeder_5,   /* BAVC_SourceId_eMpeg5,    */
#else
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eMpeg5,    */
#endif
#else
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eMpeg2,    */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eMpeg3,    */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eMpeg4,    */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eMpeg5,    */
#endif

    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eVdec0,    */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eVdec1,    */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_CCIR_656_0,      /* BAVC_SourceId_e656In0,   */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_e656In1,   */

    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eGfx0,     */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eGfx1,     */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eGfx2,     */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eGfx3,     */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eGfx4,     */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eGfx5,     */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eGfx6,     */

#if (BVDC_P_SUPPORT_HDDVI == 2)
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_HD_DVI_0,        /* BAVC_SourceId_eHdDvi0,   */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_HD_DVI_1,        /* BAVC_SourceId_eHdDvi1,   */
#elif (BVDC_P_SUPPORT_HDDVI == 1)
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_HD_DVI_0,        /* BAVC_SourceId_eHdDvi0,   */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eHdDvi1,   */
#elif (BVDC_P_SUPPORT_HDDVI == 0)
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eHdDvi0,   */
    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eHdDvi1,   */
#endif

    BCHP_VNET_F_LBOX_0_SRC_SOURCE_Output_Disabled, /* BAVC_SourceId_eDs0,     */
};

#define BVDC_P_SRC_INVALID   (0xffffffff)


#if (BVDC_P_SUPPORT_BOX_DETECT)
/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_Create
 *
 * called by BVDC_Open only
 */
BERR_Code BVDC_P_BoxDetect_Create
    ( BVDC_P_BoxDetect_Handle *         phBoxDetect,
      BVDC_P_BoxDetectId                eBoxDetectId,
      BREG_Handle                       hRegister,
      BVDC_P_Resource_Handle            hResource )
{
    uint32_t ulLboxIdx;
    BVDC_P_BoxDetectContext *pBoxDetect;
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_BoxDetect_Create);

    /* Debug level are control by app.  Only needed to turn on for debugging
     * purpose only. */
    /*BDBG_SetModuleLevel("BVDC_BOXDETECT", BDBG_eWrn);*/

    /* in case creation failed */
    BDBG_ASSERT(phBoxDetect);
    BDBG_ASSERT(eBoxDetectId < BVDC_P_BoxDetectId_eUnknown);

    *phBoxDetect = NULL;

    pBoxDetect = (BVDC_P_BoxDetectContext *)
        (BKNI_Malloc(sizeof(BVDC_P_BoxDetectContext)));
    if( pBoxDetect )
    {
        /* init the context */
        BKNI_Memset((void*)pBoxDetect, 0x0, sizeof(BVDC_P_BoxDetectContext));
        BDBG_OBJECT_SET(pBoxDetect, BVDC_BOX);
        pBoxDetect->eId = eBoxDetectId;
        pBoxDetect->hRegister = hRegister;
        pBoxDetect->eSrcId = BVDC_P_SRC_INVALID;
        pBoxDetect->Box.ulWhiteBoxWidth  = BVDC_P_BOX_DETECT_MAX_EDGE;
        pBoxDetect->Box.ulWhiteBoxHeight = BVDC_P_BOX_DETECT_MAX_EDGE;

        ulLboxIdx = (pBoxDetect->eId - BVDC_P_BoxDetectId_eBoxDetect0);

        /* TODO: Check for register layout assumptions below, and reset bit
         * position. */
#if BVDC_P_SUPPORT_NEW_SW_INIT
        pBoxDetect->ulResetAddr = BCHP_MMISC_SW_INIT;
        pBoxDetect->ulResetMask = BCHP_MMISC_SW_INIT_LBOX_0_MASK << (ulLboxIdx);
#else
        pBoxDetect->ulResetAddr = BCHP_MMISC_SOFT_RESET;
        pBoxDetect->ulResetMask = BCHP_MMISC_SOFT_RESET_LBOX_0_MASK << (ulLboxIdx);
#endif

#if (BVDC_P_SUPPORT_BOX_DETECT > 1)
        pBoxDetect->ulRegOffset = (ulLboxIdx) *
            (BCHP_LBOX_1_REG_START - BCHP_LBOX_0_REG_START);
#endif

        /* init the SubRul sub-module */
        BVDC_P_SubRul_Init(&(pBoxDetect->SubRul), BVDC_P_BoxDetect_MuxAddr(pBoxDetect),
            0, BVDC_P_DrainMode_eNone, BVDC_P_BOX_DETECT_NUM_ACCUM_VIDEO, hResource);

        *phBoxDetect = pBoxDetect;
    }

    BDBG_LEAVE(BVDC_P_BoxDetect_Create);
    return BERR_TRACE(eResult);
}
#endif

#if (BVDC_P_SUPPORT_BOX_DETECT)
/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_Destroy
 *
 * called by BVDC_Close only
 */
BERR_Code BVDC_P_BoxDetect_Destroy
    ( BVDC_P_BoxDetect_Handle          hBoxDetect )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_BoxDetect_Destroy);
    BDBG_OBJECT_ASSERT(hBoxDetect, BVDC_BOX);

    BDBG_OBJECT_DESTROY(hBoxDetect, BVDC_BOX);
    /* it is gone afterwards !!! */
    BKNI_Free((void*)hBoxDetect);

    BDBG_LEAVE(BVDC_P_BoxDetect_Destroy);
    return BERR_TRACE(eResult);
}
#endif


#if (BVDC_P_SUPPORT_BOX_DETECT)
/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_AcquireConnect_isr
 *
 * It is called by BVDC_Window_Validate after changing from diableBox to
 * enablingBox .
 */
BERR_Code BVDC_P_BoxDetect_AcquireConnect_isr
    ( BVDC_P_BoxDetect_Handle           hBoxDetect,
      BAVC_SourceId                     eSrcId,
      const BVDC_P_Source_Info         *pCurSrcInfo )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_BoxDetect_AcquireConnect_isr);

    hBoxDetect->eSrcId = eSrcId;
    hBoxDetect->ulHorzBreakCnt = 0;
    hBoxDetect->ulVertBreakCnt = 0;
    hBoxDetect->ulHorzErrCnt = 0;
    hBoxDetect->ulVertErrCnt = 0;
    hBoxDetect->pCurSrcInfo = pCurSrcInfo;

    /* this is the 1st window that acquire connent,  next _GetStatis_isr
     * should not perform cur yet. note that _BoxDetect_GetStatis_isr
     * might come before _BuildRul_isr */
    BVDC_P_BoxDetect_CutInit_isr(hBoxDetect,
        BVDC_P_BOX_DETECT_MAX_EDGE, BVDC_P_BOX_DETECT_MAX_EDGE);

    BVDC_P_SubRul_SetVnet_isr(&(hBoxDetect->SubRul),
        s_aulSrcIdToHwIndex[hBoxDetect->eSrcId],
        BVDC_P_VnetPatch_eNone);

    BDBG_LEAVE(BVDC_P_BoxDetect_AcquireConnect_isr);
    return BERR_TRACE(eResult);
}
#endif


#if BVDC_P_SUPPORT_BOX_DETECT
/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_ReleaseConnect_isr
 *
 * It is called after window decided that box-detect is no-longer used by HW in
 * its vnet mode (i.e. it is really shut down and teared off from vnet).
 */
BERR_Code BVDC_P_BoxDetect_ReleaseConnect_isr
    ( BVDC_P_BoxDetect_Handle         *phBoxDetect )
{
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_BoxDetect_ReleaseConnect_isr);
    BDBG_OBJECT_ASSERT(*phBoxDetect, BVDC_BOX);

    BVDC_P_SubRul_UnsetVnet_isr(&((*phBoxDetect)->SubRul));
    BVDC_P_Resource_ReleaseHandle_isr(
        BVDC_P_SubRul_GetResourceHandle_isr(&(*phBoxDetect)->SubRul),
        BVDC_P_ResourceType_eBoxDetect, (void *)(*phBoxDetect));

    /* this makes win to stop calling box detect code */
    *phBoxDetect = NULL;

    BDBG_LEAVE(BVDC_P_BoxDetect_ReleaseConnect_isr);
    return BERR_TRACE(eResult);
}
#endif

/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_GetStatis_isr
 *
 * BVDC_P_BoxDetect_GetStatis_isr is called by window as adjusting the rect
 * before RUL is built at each vsync. It outputs the box black cut rect by
 * pBoxCut. It also outputs BoxDetectInfo and ulCallBckCntr that is needed
 * for call-back. Window should involve call-back when ulCallBckCntr changes.
 *
 * pBoxCut->ulWidht / ulHeight should contain the full frame src size as
 * input when this function is called.
 */
void BVDC_P_BoxDetect_GetStatis_isr
    ( BVDC_P_BoxDetect_Handle           hBoxDetect,
      BVDC_P_Rect                      *pBoxCut, /* in and out */
      const BVDC_BoxDetectInfo        **ppBoxInfo, /* out */
      uint32_t                         *pulCallBckCntr )  /* out */
{
    BDBG_ENTER(BVDC_P_BoxDetect_GetStatis_isr);

    /* handle validation */
    BDBG_OBJECT_ASSERT(hBoxDetect, BVDC_BOX);
    BDBG_ASSERT( NULL != pBoxCut );
    BDBG_ASSERT( NULL != ppBoxInfo );
    BDBG_ASSERT( NULL != pulCallBckCntr );

    /* return box info to window */
    if ( (hBoxDetect->ulCurSrcWidth  == pBoxCut->ulWidth) &&
         (hBoxDetect->ulCurSrcHeight == pBoxCut->ulHeight) )
    {
        *pBoxCut = hBoxDetect->Cut;
        *ppBoxInfo = (BVDC_P_BOX_DETECT_MAX_EDGE != pBoxCut->ulHeight)?
            &(hBoxDetect->Box) : NULL; /* NULL tell win not to callback */
    }
    else
    {
        /* in case win did not set them */
        pBoxCut->lTop = 0;
        pBoxCut->lLeft = 0;
        pBoxCut->lLeft_R = 0;
        *ppBoxInfo = NULL;
    }
    *pulCallBckCntr = hBoxDetect->ulCallBckCntr;

    BDBG_LEAVE(BVDC_P_BoxDetect_GetStatis_isr);
}

#if (BVDC_P_SUPPORT_BOX_DETECT)
/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_BuildRul_isr
 *
 * called by BVDC_Window_BuildRul_isr at every src vsync. It builds RUL for
 * box detect HW module, including initial config and union-white-box-edge
 * resetting, and per-vsync pixel-accept-enabling. It reads HW box detect
 * statistics registers and process the statistics info at every multiple of
 * BVDC_P_BOX_DETECT_NUM_ACCUM vsync.
 *
 * It will reset *phBoxDetect to NULL if the HW module is no longer used by
 * any window.
 *
 * Input:
 *    eVnetState - reader or writer window/vnet state
 *    pPicComRulInfo - the PicComRulInfo that is the shared Picture info by
 *      all sub-modules when they build rul.
 *    bEnable - Whether currently BoxDetect is enabled for the window. This
 *      is passed to handle the case of disabling BoxDetect without vnet
 *      reconfigure.
 */
void BVDC_P_BoxDetect_BuildRul_isr
    ( BVDC_P_BoxDetect_Handle       *phBoxDetect,
      BVDC_P_ListInfo               *pList,
      BVDC_P_State                   eVnetState,
      BVDC_P_PicComRulInfo          *pPicComRulInfo,
      const BVDC_P_Rect             *pSrcOut,
      bool                           bEnable )
{
    BVDC_P_State  eBoxDetectVnetState;
    uint32_t  ulRulOpsFlags;

    BDBG_ENTER(BVDC_P_BoxDetect_BuildRul_isr);

    /* handle validation */
    BDBG_ASSERT(*phBoxDetect);
    BDBG_OBJECT_ASSERT(*phBoxDetect, BVDC_BOX);

    /* init RUL buffer pointers */
    eBoxDetectVnetState = (((BVDC_P_State_eActive          == eVnetState) ||
                            (BVDC_P_State_eShutDownPending == eVnetState))  &&
                           (!bEnable)) ? BVDC_P_State_eShutDownRul : eVnetState;
    ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(&((*phBoxDetect)->SubRul),
        pPicComRulInfo->eWin, eBoxDetectVnetState, pList->bLastExecuted);

    /* if we re-set vnet, or if src video format changes (HdDvi), we need to
     * do SrcInit, such as reset buffer size and stride */
    if (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
        BVDC_P_BoxDetect_BuildRul_VnetInit_isr((*phBoxDetect), pList);

    if ( ((*phBoxDetect)->ulCurSrcWidth  != pSrcOut->ulWidth) ||
         ((*phBoxDetect)->ulCurSrcHeight != pSrcOut->ulHeight) )
    {
        /* start from scratch again if source size change */
        BVDC_P_BoxDetect_CutInit_isr((*phBoxDetect), pSrcOut->ulWidth, pSrcOut->ulHeight);
        BVDC_P_BoxDetect_BuildRul_StatisInit_isr((*phBoxDetect), pList);
    }
    else if (ulRulOpsFlags & BVDC_P_RulOp_eStatisInit)
    {
        /* periodic statistics init */
        BVDC_P_BoxDetect_BuildRul_StatisRead_isr((*phBoxDetect), pPicComRulInfo, pSrcOut);
        BVDC_P_BoxDetect_BuildRul_StatisInit_isr((*phBoxDetect), pList);
    }

    if (ulRulOpsFlags & BVDC_P_RulOp_eEnable)
    {
        BVDC_P_BoxDetect_BuildRul_SetEnable_isr((*phBoxDetect), pList, true);

        /* join in vnet after enable. note: its src mux is initialed as disabled */
        if (ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
        {
            BVDC_P_SubRul_JoinInVnet_isr(&((*phBoxDetect)->SubRul), pList);
        }
    }
    else if (ulRulOpsFlags & BVDC_P_RulOp_eDisable)
    {
        BVDC_P_SubRul_DropOffVnet_isr(&((*phBoxDetect)->SubRul), pList);
        BVDC_P_BoxDetect_BuildRul_SetEnable_isr((*phBoxDetect), pList, false);
        BVDC_P_BoxDetect_BuildRul_DrainVnet_isr((*phBoxDetect), pList);
    }

    /* lbox needs this special handling for the case of disabling boxDetect
     * without vnet reconfiguration */
    if ((!bEnable) && (ulRulOpsFlags & BVDC_P_RulOp_eReleaseHandle))
    {
        BVDC_P_BoxDetect_ReleaseConnect_isr(phBoxDetect);
    }

    BDBG_LEAVE(BVDC_P_BoxDetect_BuildRul_isr);
}
#endif


#if (BVDC_P_SUPPORT_BOX_DETECT)
/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_CutInit_isr
 *
 * It is called to init the cut info which is output to window by
 * BVDC_P_BoxDetect_GetStatis_isr. Notice that BVDC_P_BoxDetect_GetStatis_isr
 * might be called before _BuildRul_isr.
 */
static void BVDC_P_BoxDetect_CutInit_isr
    ( BVDC_P_BoxDetect_Handle        hBoxDetect,
      uint32_t                       ulSrcWidth,
      uint32_t                       ulSrcHeight )
{
    BDBG_MSG(("New src size: (W %d, H %d)", ulSrcWidth, ulSrcHeight));

    /* force to call UpdateCut_isr */
    hBoxDetect->Box.ulWhiteBoxWidth = 0;
    hBoxDetect->Box.ulWhiteBoxHeight = 0;

    hBoxDetect->ulCurSrcWidth  = ulSrcWidth;
    hBoxDetect->ulCurSrcHeight = ulSrcHeight;
    hBoxDetect->Cut.lLeft    = 0;
    hBoxDetect->Cut.lLeft_R  = 0;
    hBoxDetect->Cut.lTop     = 0;
    hBoxDetect->Cut.ulWidth  = ulSrcWidth;  /* ulRight is not in whitebox */
    hBoxDetect->Cut.ulHeight = ulSrcHeight; /* ulBottom is not in whitebox */

    /* tell SubRul to reset AccumCntr to 0 for statistics reading */
    BVDC_P_SubRul_ResetAccumCntr_isr(&(hBoxDetect->SubRul));
}
#endif

/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_BuildRul_VnetInit_isr
 *
 * It is called by BVDC_P_BoxDetect_BuildRul_isr to init HW.
 */
static void BVDC_P_BoxDetect_BuildRul_VnetInit_isr
    ( BVDC_P_BoxDetect_Handle        hBoxDetect,
      BVDC_P_ListInfo               *pList )
{
    uint32_t ulRulOffset;
    int32_t lStatisReadPeriod;
    void *pvBoxDetectRegCfg;

    ulRulOffset = hBoxDetect->ulRegOffset;

    lStatisReadPeriod = BVDC_P_BOX_DETECT_NUM_ACCUM_VIDEO;
    pvBoxDetectRegCfg = (void*) &s_aulBoxDetectRegCfgForVideo[0];
    hBoxDetect->ulBreakThresh = BVDC_P_BOX_DETECT_BREAK_THRESH_VIDEO;

    BVDC_P_SubRul_SetStatisReadPeriod(&(hBoxDetect->SubRul), lStatisReadPeriod);

    /* config the box detect HW module */
    BVDC_P_SUBRUL_START_BLOCK(pList, BCHP_LBOX_0_CONFIGURATION, ulRulOffset,
                              BVDC_P_NUM_REGS_BOX_DETECT_CONFIG);
    BKNI_Memcpy( (void*) pList->pulCurrent, pvBoxDetectRegCfg,
                 4 * BVDC_P_NUM_REGS_BOX_DETECT_CONFIG );
    pList->pulCurrent += BVDC_P_NUM_REGS_BOX_DETECT_CONFIG;

#if (BVDC_P_SUPPORT_BOX_DETECT_VER <= 2)
    /* disable box expanding intrrupt, because sw will check every vsync */
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS( 2 );
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_LBOX_0_TB_INTERRUPT_THRESHOLD) + ulRulOffset;
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA( LBOX_0_TB_INTERRUPT_THRESHOLD, TOP_THRESHOLD,    0) |
        BCHP_FIELD_DATA( LBOX_0_TB_INTERRUPT_THRESHOLD, BOTTOM_THRESHOLD, BVDC_P_BOX_DETECT_MAX_EDGE);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA( LBOX_0_LR_INTERRUPT_THRESHOLD, LEFT_THRESHOLD,    0) |
        BCHP_FIELD_DATA( LBOX_0_LR_INTERRUPT_THRESHOLD, RIGHT_THRESHOLD, BVDC_P_BOX_DETECT_MAX_EDGE);
#endif
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_BuildRul_StatisInit_isr
 *
 * It is called by BVDC_P_BoxDetect_BuildRul_isr to reset statistics
 */
static void BVDC_P_BoxDetect_BuildRul_StatisInit_isr
    ( BVDC_P_BoxDetect_Handle        hBoxDetect,
      BVDC_P_ListInfo               *pList )
{
    uint32_t ulRulOffset;

    ulRulOffset = hBoxDetect->ulRegOffset;

    /* clear accumulated (union) box edges regs */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG( );
    *pList->pulCurrent++ = BRDC_REGISTER( BCHP_LBOX_0_CLEAR_STATUS + ulRulOffset );
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA( LBOX_0_CLEAR_STATUS, CLEAR_EDGES, 1) |
        BCHP_FIELD_DATA( LBOX_0_CLEAR_STATUS, CLEAR_LUMA,  1);
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_BuildRul_StatisInit_isr
 *
 * It is called by BVDC_P_BoxDetect_BuildRul_isr to enable box detect
 */
static void BVDC_P_BoxDetect_BuildRul_SetEnable_isr
    ( BVDC_P_BoxDetect_Handle        hBoxDetect,
      BVDC_P_ListInfo               *pList,
      bool                           bEnable )
{
    uint32_t ulRulOffset;

    ulRulOffset = hBoxDetect->ulRegOffset;

    /* enable/disable the hw to accept pixel data */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG( );
#if (BVDC_P_SUPPORT_BOX_DETECT_VER >= 2)
    *pList->pulCurrent++ = BRDC_REGISTER( BCHP_LBOX_0_CTRL + ulRulOffset );
    if (bEnable)
    {
        *pList->pulCurrent++ = BCHP_FIELD_DATA( LBOX_0_CTRL, ENABLE_ACCEPT,
            BCHP_LBOX_0_CTRL_ENABLE_ACCEPT_EN );
    }
    else
    {
        *pList->pulCurrent++ = BCHP_FIELD_DATA( LBOX_0_CTRL, ENABLE_ACCEPT,
            BCHP_LBOX_0_CTRL_ENABLE_ACCEPT_DIS );
    }
#else
    *pList->pulCurrent++ = BRDC_REGISTER( BCHP_LBOX_0_ENABLE_ACCEPT + ulRulOffset );
    if (bEnable)
    {
        *pList->pulCurrent++ = BCHP_FIELD_DATA( LBOX_0_ENABLE_ACCEPT, ENABLE_ACCEPT,
            BCHP_LBOX_0_ENABLE_ACCEPT_ENABLE_ACCEPT_EN );
    }
    else
    {
        *pList->pulCurrent++ = BCHP_FIELD_DATA( LBOX_0_ENABLE_ACCEPT, ENABLE_ACCEPT,
            BCHP_LBOX_0_ENABLE_ACCEPT_ENABLE_ACCEPT_DIS );
    }
#endif
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_BuildRul_DrainVnet_isr
 *
 * called by BVDC_P_BoxDetect_BuildRul_isr to reset the box detect HW module
 */
static void BVDC_P_BoxDetect_BuildRul_DrainVnet_isr
    ( BVDC_P_BoxDetect_Handle        hBoxDetect,
      BVDC_P_ListInfo               *pList )
{
    /* like cap, it only needs reset */
    BVDC_P_SUBRUL_ONE_REG(pList, hBoxDetect->ulResetAddr, 0, hBoxDetect->ulResetMask);
    BVDC_P_SUBRUL_ONE_REG(pList, hBoxDetect->ulResetAddr, 0, 0);
    return;
}


#define BVDC_P_BOX_DETECT_UPD_SHOW               0
#if (BVDC_P_BOX_DETECT_UPD_SHOW==1)
#define BVDC_P_BOX_UPD_MSG    BDBG_ERR
#else
#define BVDC_P_BOX_UPD_MSG    BDBG_MSG
#endif

#define BVDC_P_BOX_DETECT_MAX_GRAY_WIDTH         3
#define BVDC_P_BOX_DETECT_MIN_ROW_STRNG         50
#define BVDC_P_BOX_DETECT_MIN_COL_STRNG         20
#define BVDC_P_BOX_DETECT_MIN_BLACK_SIZE (2*BVDC_P_BOX_DETECT_MAX_GRAY_WIDTH)
/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_BoxDetect_UpdateCut_isr
    ( BVDC_P_BoxDetect_Handle        hBoxDetect,
      uint32_t                       ulSrcWidth,
      uint32_t                       ulSrcHeight,
      bool                           bHwRowErr,
      bool                           bHwColErr )
{
    uint32_t  ulWhiteTop, ulWhiteBottom, ulWhiteHeight;
    uint32_t  ulMinTop, ulMaxTop, ulMinBottom, ulMaxBottom;
    uint32_t  ulMinHeight, ulMaxHeight;
    uint32_t  ulPreMinCutTop, ulPreMaxCutBot;

    uint32_t  ulWhiteLeft, ulWhiteRight, ulWhiteWidth;
    uint32_t  ulMinLeft, ulMaxLeft, ulMinRight, ulMaxRight;
    uint32_t  ulMinWidth, ulMaxWidth;
    uint32_t  ulPreMinCutLeft, ulPreMaxCutRight;

    uint32_t  ulStandWhiteStart, ulStandWhiteSize, ulTolerance;
    uint32_t  ulCutTop, ulCutLeft, ulCutWidth, ulCutHeight;

    /* In order to stablize the display, specially in the case that MAD is used after
     * box cut, we should adjust the cut basing on statistics and our guess to some
     * standard case. We are likely to see
     *  1).  4:3 content in 16:9 signal with pillar box;
     *  2). 16:9 content in  4:3 signal with letter box;
     *  3).  4:3 content in  4:3 signal as stamp;
     *  4). 16:9 content in 16:9 signal as stamp;
     * In all above cases, the standard white width and/or height are 3/4 of the
     * original src width and/or height.
     */
    ulCutLeft = 0;
    ulCutTop  = 0;
    ulCutWidth  = ulSrcWidth;
    ulCutHeight = ulSrcHeight;

    /* According to hw arch spec, letter box is usually about 75% of the
     * original src height, and is put in the center of the screen.
     * Therefore, MaxTop and MinBot should be at about 1/8 and 7/8 of
     * the src hight respectively. However, we set them to be 2/8 and 6/8
     * for flexibility */
    /* "Notting Hill" shows white rows 52-189, (189-52/240)=.57 */
    ulWhiteTop    = hBoxDetect->Box.ulWhiteBoxTop;
    ulWhiteHeight = hBoxDetect->Box.ulWhiteBoxHeight;
    ulWhiteBottom = ulWhiteTop + ulWhiteHeight; /* need out edge */
    ulMinTop = BVDC_P_BOX_DETECT_MIN_BLACK_SIZE;
    ulMaxTop = (3 * ulSrcHeight) / 8;
    ulMinBottom = (5 * ulSrcHeight) / 8;
    ulMaxBottom = ulSrcHeight - BVDC_P_BOX_DETECT_MIN_BLACK_SIZE; /* row ulMaxBottom is in src rect */
    ulMinHeight = ulSrcHeight / 2;
    ulMaxHeight = (ulSrcHeight * 27) / 32; /* 27/32 = 0.84375 */
    ulPreMinCutTop = hBoxDetect->Cut.lTop - BVDC_P_BOX_DETECT_MAX_GRAY_WIDTH;
    ulPreMaxCutBot = (hBoxDetect->Cut.lTop + hBoxDetect->Cut.ulHeight +
                      BVDC_P_BOX_DETECT_MAX_GRAY_WIDTH);

    /* The following is copied from LBOX_test.bss
     * If (FNBL > 0) And (FNBL < 100) And (FWL < (FNBL + 3)) And (FWLS > 50) Then
     *   print "  Letterbox Top at " & FWL
     * If (LNBL > 100) And (LNBL < 240) And (LWL > (LNBL - 3)) And (LWLS > 50) Then
     *   print "  Letterbox Bottom at " & LWL
     */
    if ( (false == bHwRowErr) &&
         (BVDC_P_BOX_DETECT_MAX_GRAY_WIDTH > hBoxDetect->Box.ulGrayWidthTop)    &&
         (BVDC_P_BOX_DETECT_MAX_GRAY_WIDTH > hBoxDetect->Box.ulGrayWidthBottom) &&
         (BVDC_P_BOX_DETECT_MIN_ROW_STRNG < hBoxDetect->Box.ulEdgeStrengthTop)    &&
         (BVDC_P_BOX_DETECT_MIN_ROW_STRNG < hBoxDetect->Box.ulEdgeStrengthBottom) &&
         (ulMinHeight < ulWhiteHeight) &&
         (ulMaxHeight > ulWhiteHeight) &&
         (ulMinTop    < ulWhiteTop)    &&
         (ulMaxTop    > ulWhiteTop)    &&
         (ulMinBottom < ulWhiteBottom) &&
         (ulMaxBottom > ulWhiteBottom) )
    {
        /* find black strip on top or bottom */
        BVDC_P_BOX_UPD_MSG(("Found L box: White T %d, H %d", ulWhiteTop, ulWhiteHeight));
        ulStandWhiteSize = (ulSrcHeight * 3 + 2) / 4;
        ulStandWhiteStart = (ulSrcHeight - ulStandWhiteSize + 1) / 2;
        ulTolerance = ulSrcHeight / 64;
        if (((ulWhiteTop + ulTolerance) > ulStandWhiteStart) &&
            ((ulWhiteTop + ulWhiteHeight) < (ulStandWhiteStart + ulStandWhiteSize + ulTolerance)))
        {
            /* WhiteTop and WhiteBot are in the tolerantable range of standard white box */
            ulCutTop = ulStandWhiteStart;
            ulCutHeight = ulStandWhiteSize;
        }
        else
        {
            ulCutTop = ulWhiteTop;
            ulCutHeight = ulWhiteHeight;
        }
        hBoxDetect->ulVertBreakCnt = 0;
        hBoxDetect->ulVertErrCnt = 0;
    }
    else if ((false == bHwRowErr) &&
             (ulSrcHeight > hBoxDetect->Cut.ulHeight) && /* last time we found top or bot black */
             (ulPreMinCutTop <= ulWhiteTop) &&
             (ulPreMaxCutBot >= ulWhiteBottom) ) /* new box not expand */
    {
        /* found top or bot black strip last time, not this time, but white box not expand
         * likely no thing changed to box, so don't change cut */
        BVDC_P_BOX_UPD_MSG(("Continue L box cut"));
        ulCutTop = hBoxDetect->Cut.lTop;
        ulCutHeight = hBoxDetect->Cut.ulHeight;
        hBoxDetect->ulVertBreakCnt = 0;
        hBoxDetect->ulVertErrCnt = 0;
    }
    else if ( (hBoxDetect->ulVertBreakCnt < hBoxDetect->ulBreakThresh) &&
              (ulSrcHeight > hBoxDetect->Cut.ulHeight) ) /* last time we found top or bot black */
    {
        /* vert black is broken, but we wait for the break_threash is passed, in case this is
         * caused by noise, so don't change cut yet. If HwRowErr, its is less confident to say
         * that the box is broken, must wait longer to break cut, ulVertErrCnt is used for this */
        BVDC_P_BOX_UPD_MSG(("Wait for L box break thresh hold"));
        ulCutTop = hBoxDetect->Cut.lTop;
        ulCutHeight = hBoxDetect->Cut.ulHeight;
        if ((false == bHwRowErr) ||
            (hBoxDetect->ulVertErrCnt > BVDC_P_BOX_DETECT_ERR_THRESH))
        {
            hBoxDetect->ulVertBreakCnt ++;
            hBoxDetect->ulVertErrCnt = 0;
        }
        else
        {
            hBoxDetect->ulVertErrCnt ++;
        }
    }
    else
    {
        /* vertical black break threshhold is reached, stop cut */
        hBoxDetect->ulVertBreakCnt = 0;
        hBoxDetect->ulVertErrCnt = 0;
#if BDBG_DEBUG_BUILD
        if ((ulCutTop    != (uint32_t)hBoxDetect->Cut.lTop) ||
            (ulCutHeight != hBoxDetect->Cut.ulHeight))
        {
            BVDC_P_BOX_UPD_MSG(("Break cut of L box"));
            if (bHwRowErr)
            {
                BVDC_P_BOX_UPD_MSG(("Found HW row edge reading error"));
            }
        }
#endif
    }

    /* According to hw arch spec, pillar box is usually about 75% of the
     * original src width, and is put in the center of the screen .
     * Therefore, MaxLeft and MinRight should be at about 1/8 and 7/8 of
     * the src width respectively. However, we set them to be 2/8 and 6/8
     * for flexibility */
    ulWhiteLeft  = hBoxDetect->Box.ulWhiteBoxLeft;
    ulWhiteWidth = hBoxDetect->Box.ulWhiteBoxWidth;
    ulWhiteRight = ulWhiteLeft + ulWhiteWidth; /* need out edge */
    ulMinLeft = BVDC_P_BOX_DETECT_MIN_BLACK_SIZE;
    ulMaxLeft = (3 * ulSrcWidth) / 8;
    ulMinRight = (5 * ulSrcWidth) / 8;
    ulMaxRight = ulSrcWidth - BVDC_P_BOX_DETECT_MIN_BLACK_SIZE;  /* col ulMaxRight is in src rect */
    ulMinWidth = ulSrcWidth / 2;
    ulMaxWidth = (ulSrcWidth * 27) / 32; /* 27/32 = 0.84375 */
    ulPreMinCutLeft = hBoxDetect->Cut.lLeft - BVDC_P_BOX_DETECT_MAX_GRAY_WIDTH;
    ulPreMaxCutRight = (hBoxDetect->Cut.lLeft + hBoxDetect->Cut.ulWidth +
                        BVDC_P_BOX_DETECT_MAX_GRAY_WIDTH);

    /* The following is copied from LBOX_test.bss
     * If (FNBC > 0) And (FNBC < 200) And (FWC < (FNBC + 3)) And (FWCS > 20) Then
     *   print "  Pillarbox Left at " & FWC
     * If (LNBC > 500) And (LNBC < 800) And (LWC > (LNBC - 3)) And (LWCS > 20) Then
     *   print "  Pillarbox Right at " & LWC
     */
    if ( (false == bHwColErr) &&
         (BVDC_P_BOX_DETECT_MAX_GRAY_WIDTH > hBoxDetect->Box.ulGrayWidthLeft)  &&
         (BVDC_P_BOX_DETECT_MAX_GRAY_WIDTH > hBoxDetect->Box.ulGrayWidthRight) &&
         (BVDC_P_BOX_DETECT_MIN_COL_STRNG < hBoxDetect->Box.ulEdgeStrengthLeft)  &&
         (BVDC_P_BOX_DETECT_MIN_COL_STRNG < hBoxDetect->Box.ulEdgeStrengthRight) &&
         (ulMinWidth < ulWhiteWidth) &&
         (ulMaxWidth > ulWhiteWidth) &&
         (ulMinLeft  < ulWhiteLeft)  &&
         (ulMaxLeft  > ulWhiteLeft)  &&
         (ulMinRight < ulWhiteRight) &&
         (ulMaxRight > ulWhiteRight) )
    {
        /* find black strip on left and / or right */
        /* column ulLeft should not be in whitebox */
        BVDC_P_BOX_UPD_MSG(("Found P box: White L %d, W %d", ulWhiteLeft, ulWhiteWidth));
        ulStandWhiteSize = (ulSrcWidth * 3 + 2) / 4;
        ulStandWhiteStart = (ulSrcWidth - ulStandWhiteSize + 1) / 2;
        ulTolerance = ulSrcWidth / 64;
        if (((ulWhiteLeft + ulTolerance) > ulStandWhiteStart) &&
            ((ulWhiteLeft + ulWhiteWidth) < (ulStandWhiteStart + ulStandWhiteSize + ulTolerance)))
        {
            /* WhiteLeft and WhiteRight are in the tolerantable range of standard white box */
            ulCutLeft = ulStandWhiteStart;
            ulCutWidth = ulStandWhiteSize;
        }
        else
        {
            ulCutLeft = ulWhiteLeft;
            ulCutWidth = ulWhiteWidth;
        }
        hBoxDetect->ulHorzBreakCnt = 0;
        hBoxDetect->ulHorzErrCnt = 0;
    }
    else if ( (false == bHwColErr) &&
              (ulSrcWidth > hBoxDetect->Cut.ulWidth) && /* last time found left or right black */
              (ulPreMinCutLeft  <= ulWhiteLeft)  &&
              (ulPreMaxCutRight >= ulWhiteRight) ) /* new box not expand */
    {
        /* found left or right black strip last time, not this time, but white box not expand
         * likely no thing changed to box, so don't change cut */
        BVDC_P_BOX_UPD_MSG(("Continue P box cut"));
        ulCutLeft = hBoxDetect->Cut.lLeft;
        ulCutWidth = hBoxDetect->Cut.ulWidth;
        hBoxDetect->ulHorzBreakCnt = 0;
        hBoxDetect->ulHorzErrCnt = 0;
    }
    else if ( (hBoxDetect->ulHorzBreakCnt < hBoxDetect->ulBreakThresh) &&
              (ulSrcWidth > hBoxDetect->Cut.ulWidth) ) /* last time found left or right black */
    {
        /* horz black is broken, but we wait for the break_threash is passed, in case this is
         * caused by noise, so don't change cut yet. If HwColErr, its is less confident to say
         * that the box is broken, must wait longer to break cut, ulHorzErrCnt is used for this */
        BVDC_P_BOX_UPD_MSG(("Wait for P box break thresh hold"));
        ulCutLeft = hBoxDetect->Cut.lLeft;
        ulCutWidth = hBoxDetect->Cut.ulWidth;
        if ((false == bHwColErr) ||
            (hBoxDetect->ulHorzErrCnt >= BVDC_P_BOX_DETECT_ERR_THRESH))
        {
            hBoxDetect->ulHorzBreakCnt ++;
            hBoxDetect->ulHorzErrCnt = 0;
        }
        else
        {
            hBoxDetect->ulHorzErrCnt ++;
        }
    }
    else
    {
        /* horz black break threshhold is reached, stop cut */
        hBoxDetect->ulHorzBreakCnt = 0;
        hBoxDetect->ulHorzErrCnt = 0;
#if BDBG_DEBUG_BUILD
        if ((ulCutLeft  != (uint32_t)hBoxDetect->Cut.lLeft) ||
            (ulCutWidth != hBoxDetect->Cut.ulWidth))
        {
            BVDC_P_BOX_UPD_MSG(("Break cut of P box"));
            if (bHwColErr)
            {
                BVDC_P_BOX_UPD_MSG(("Found HW column edge reading error"));
            }
        }
#endif
    }

    hBoxDetect->Cut.lTop = ulCutTop;
    hBoxDetect->Cut.ulHeight = ulCutHeight;
    hBoxDetect->Cut.lLeft = ulCutLeft;
    hBoxDetect->Cut.lLeft_R = ulCutLeft;
    hBoxDetect->Cut.ulWidth = ulCutWidth;
}

/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_BuildRul_StatisRead_isr
 *
 * It is called by BVDC_P_BoxDetect_BuildRul_isr to read statistics HW registers
 * and then to updates box black cut rect and BoxDetectInfo only if it sees either
 * the white box, outer edge width, or white edge strength change.
 *
 * This happens at every multiple of BVDC_P_BOX_DETECT_NUM_ACCUM fields or
 * frames.
 */
static void BVDC_P_BoxDetect_BuildRul_StatisRead_isr
    ( BVDC_P_BoxDetect_Handle        hBoxDetect,
      BVDC_P_PicComRulInfo          *pPicComRulInfo,
      const BVDC_P_Rect             *pSrcOut )
{
    BREG_Handle  hRegister;
    uint32_t  ulSrcWidth, ulSrcHeight;
    uint32_t  ulRegValue, ulRegOffset;
    uint32_t  ulNnBlkTop, ulNnBlkBot, ulNnBlkLeft, ulNnBlkRight;
    uint32_t  ulWhiteTop, ulWhiteBot, ulWhiteLeft, ulWhiteRight;
    uint32_t  ulWhiteWidth, ulWhiteHeight;
    uint32_t  ulEdgeStrngTop, ulEdgeStrngBot, ulEdgeStrngLeft, ulEdgeStrngRight;
    uint32_t  ulGrayWidthTop, ulGrayWidthBot, ulGrayWidthLeft, ulGrayWidthRight;
    bool  bHwRowErr, bHwColErr, bHwReadChange;

    ulSrcWidth  = pSrcOut->ulWidth;
    ulSrcHeight = pSrcOut->ulHeight;

    /* read hw register for letter box detect: both window need get the cut info,
     * don't care which window read */
    hRegister = hBoxDetect->hRegister;
    ulRegOffset = hBoxDetect->ulRegOffset;
    ulRegValue = BREG_Read32( hRegister, BCHP_LBOX_0_MINMAX_NON_BLACK_LINES + ulRegOffset );
    ulNnBlkTop = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MINMAX_NON_BLACK_LINES, SMALL_NON_BLACK_LINE_NUMBER );
    ulNnBlkBot = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MINMAX_NON_BLACK_LINES, LARGE_NON_BLACK_LINE_NUMBER );
    ulRegValue = BREG_Read32( hRegister, BCHP_LBOX_0_MIN_WHITE_LINE + ulRegOffset );
    ulEdgeStrngTop = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MIN_WHITE_LINE, SMALL_WHITE_LINE_STRENGTH );
    ulWhiteTop = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MIN_WHITE_LINE, SMALL_WHITE_LINE_NUMBER );
    ulRegValue = BREG_Read32( hRegister, BCHP_LBOX_0_MAX_WHITE_LINE + ulRegOffset );
    ulEdgeStrngBot = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MAX_WHITE_LINE, LARGE_WHITE_LINE_STRENGTH );
    ulWhiteBot = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MAX_WHITE_LINE, LARGE_WHITE_LINE_NUMBER );
    ulGrayWidthTop = ulWhiteTop - ulNnBlkTop;
    ulGrayWidthBot = ulNnBlkBot - ulWhiteBot;
    ulWhiteBot += 1; /* need out edge */
    ulNnBlkBot += 1; /* for HW confusing test in the following */
    if ( BAVC_Polarity_eFrame != pPicComRulInfo->eSrcOrigPolarity )
    {
        ulWhiteTop *= 2;
        ulWhiteBot *= 2; /* odd field reaches 2 * ulWhiteBot + 1 */
        ulNnBlkTop *= 2; /* for HW confusing test in the following */
        ulNnBlkBot *= 2; /* for HW confusing test in the following */
    }
    ulWhiteHeight = ulWhiteBot - ulWhiteTop;

    /* read hw register for pillar box detect */
    ulRegValue = BREG_Read32( hRegister, BCHP_LBOX_0_MINMAX_NON_BLACK_COL + ulRegOffset );
    ulNnBlkLeft  = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MINMAX_NON_BLACK_COL, SMALL_NON_BLACK_COL_NUMBER );
    ulNnBlkRight = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MINMAX_NON_BLACK_COL, LARGE_NON_BLACK_COL_NUMBER );
    ulRegValue = BREG_Read32( hRegister, BCHP_LBOX_0_MIN_WHITE_COL + ulRegOffset );
    ulEdgeStrngLeft = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MIN_WHITE_COL, SMALL_WHITE_COL_STRENGTH );
    ulWhiteLeft = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MIN_WHITE_COL, SMALL_WHITE_COL_NUMBER );
    ulRegValue = BREG_Read32( hRegister, BCHP_LBOX_0_MAX_WHITE_COL + ulRegOffset );
    ulEdgeStrngRight = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MAX_WHITE_COL, LARGE_WHITE_COL_STRENGTH );
    ulWhiteRight = BCHP_GET_FIELD_DATA( ulRegValue, LBOX_0_MAX_WHITE_COL, LARGE_WHITE_COL_NUMBER );
    ulGrayWidthLeft  = ulWhiteLeft - ulNnBlkLeft;
    ulGrayWidthRight = ulNnBlkRight - ulWhiteRight;
    ulWhiteRight += 1; /* need out edge */
    ulNnBlkRight += 1; /* for HW confusing test in the following */
    ulWhiteWidth = ulWhiteRight - ulWhiteLeft;

    /* If we use less vsync to accumulate statistics, therefore we need confirm the
     * reading result with a 2nd reading. We used to use BCHP_LBOX_0_FIRST_WHITE_LINE
     * and BCHP_LBOX_0_FIRST_WHITE_COL to confirm the box change For PC input (see
     * PR 36632), but it does not work well for video input */
    bHwRowErr = !(
        (ulNnBlkTop <= ulWhiteTop)  &&
        (ulWhiteTop <  ulWhiteBot)  &&
        (ulWhiteBot <= ulNnBlkBot)  &&
        (ulNnBlkBot <= ulSrcHeight) &&
#if BVDC_P_BOX_DETECT_FAST_DETECT
        (ulWhiteTop    == hBoxDetect->Box.ulWhiteBoxTop)    &&
        (ulWhiteHeight == hBoxDetect->Box.ulWhiteBoxHeight) &&
#endif
        (ulWhiteBot != 1));
    bHwColErr = !(
        (ulNnBlkLeft  <= ulWhiteLeft)  &&
        (ulWhiteLeft  <  ulWhiteRight) &&
        (ulWhiteRight <= ulNnBlkRight) &&
        (ulNnBlkRight <= ulSrcWidth)   &&
#if BVDC_P_BOX_DETECT_FAST_DETECT
        (ulWhiteLeft  == hBoxDetect->Box.ulWhiteBoxLeft)  &&
        (ulWhiteWidth == hBoxDetect->Box.ulWhiteBoxWidth) &&
#endif
        (ulWhiteRight != 1));

    bHwReadChange = (
        (ulWhiteLeft      != hBoxDetect->Box.ulWhiteBoxLeft)       ||
        (ulWhiteTop       != hBoxDetect->Box.ulWhiteBoxTop)        ||
        (ulWhiteWidth     != hBoxDetect->Box.ulWhiteBoxWidth)      ||
        (ulWhiteHeight    != hBoxDetect->Box.ulWhiteBoxHeight)     ||
        (ulGrayWidthLeft  != hBoxDetect->Box.ulGrayWidthLeft)      ||
        (ulGrayWidthTop   != hBoxDetect->Box.ulGrayWidthTop)       ||
        (ulGrayWidthRight != hBoxDetect->Box.ulGrayWidthRight)     ||
        (ulGrayWidthBot   != hBoxDetect->Box.ulGrayWidthBottom)    ||
        (ulEdgeStrngLeft  != hBoxDetect->Box.ulEdgeStrengthLeft)   ||
        (ulEdgeStrngTop   != hBoxDetect->Box.ulEdgeStrengthTop)    ||
        (ulEdgeStrngRight != hBoxDetect->Box.ulEdgeStrengthRight)  ||
        (ulEdgeStrngBot   != hBoxDetect->Box.ulEdgeStrengthBottom) );

    if ( true == bHwReadChange )
    {
        /* update BoxDetect records */
        hBoxDetect->Box.ulWhiteBoxLeft   = ulWhiteLeft;
        hBoxDetect->Box.ulWhiteBoxTop    = ulWhiteTop;
        hBoxDetect->Box.ulWhiteBoxWidth  = ulWhiteWidth;
        hBoxDetect->Box.ulWhiteBoxHeight = ulWhiteHeight;
        hBoxDetect->Box.ulGrayWidthLeft   = ulGrayWidthLeft;
        hBoxDetect->Box.ulGrayWidthTop    = ulGrayWidthTop;
        hBoxDetect->Box.ulGrayWidthRight  = ulGrayWidthRight;
        hBoxDetect->Box.ulGrayWidthBottom = ulGrayWidthBot;
        hBoxDetect->Box.ulEdgeStrengthLeft   = ulEdgeStrngLeft;
        hBoxDetect->Box.ulEdgeStrengthTop    = ulEdgeStrngTop;
        hBoxDetect->Box.ulEdgeStrengthRight  = ulEdgeStrngRight;
        hBoxDetect->Box.ulEdgeStrengthBottom = ulEdgeStrngBot;

        /* window will call callBack if it sees ulCallBckCntr changes */
        hBoxDetect->ulCallBckCntr++;
        if (hBoxDetect->ulCallBckCntr > 0x7fffffff)
            hBoxDetect->ulCallBckCntr = 1;
    }

#if BDBG_DEBUG_BUILD
    if ( bHwRowErr && bHwReadChange )
    {
        BDBG_MSG(("Box NBlkT %d, WhiteT %d, WhiteB %d, NBlkB %d, SrcH %d",
            ulNnBlkTop, ulWhiteTop, ulWhiteBot, ulNnBlkBot, ulSrcHeight));
    }
    if ( bHwColErr && bHwReadChange )
    {
        BDBG_MSG(("Box NBlkL %d, WhiteL %d, WhiteR %d, NBlkR %d, SrcW %d",
            ulNnBlkLeft, ulWhiteLeft, ulWhiteRight, ulNnBlkRight, ulSrcWidth));
    }
#endif

    /* update black patch clip rect */
    BVDC_P_BoxDetect_UpdateCut_isr(
        hBoxDetect, ulSrcWidth, ulSrcHeight, bHwRowErr, bHwColErr );
}


/***************************************************************************/
/* No support for any box detect */
#else

#include "bvdc_errors.h"

BDBG_MODULE(BVDC_BOXDETECT);

void BVDC_P_BoxDetect_GetStatis_isr
    ( BVDC_P_BoxDetect_Handle           hBoxDetect,
      BVDC_P_Rect                      *pBoxCut, /* out */
      const BVDC_BoxDetectInfo        **ppBoxInfo, /* out */
      uint32_t                         *pulCallBckCntr ) /* out */
{
    BSTD_UNUSED(hBoxDetect);
    BSTD_UNUSED(pBoxCut);
    BSTD_UNUSED(ppBoxInfo);
    BSTD_UNUSED(pulCallBckCntr);
    return;
}

#endif  /* #if (0 != BVDC_P_SUPPORT_BOX_DETECT) */

/* End of file. */
