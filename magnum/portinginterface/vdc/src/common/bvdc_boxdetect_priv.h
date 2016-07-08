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
#ifndef BVDC_BOXDETECT_PRIV_H__
#define BVDC_BOXDETECT_PRIV_H__

#include "bavc.h"
#include "breg_mem.h"      /* Chip register access (memory mapped). */
#include "bvdc_common_priv.h"
#include "bvdc_subrul_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_source_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * {private}
 *
 * BoxDetect Sub-module Overview:
 *
 * When a video content with aspect ratio 16:9 is conveyed on a video stream
 * signal with aspect ratio 4:3, or when 4:3 content is conveyed on 16:9
 * signal, letter box or pillar box algorithm could be used. They patch two
 * black regions on top-bottom or left-right respectively.
 *
 * BoxDetect sub-module is used to detect whether there is letter box or
 * pillar box in the source video stream signal, and the location and size.
 * It is enabled by BVDC_Window_EnableBoxDetect.
 *
 * As other BVDC API functions, box detect enabling and disabling to a
 * window will take effect after BVDC_ApplyChanges, which should call
 * BVDC_P_BoxDetect_ApplyChanges. However, the hardware module is accupied
 * right after BVDC_Window_EnableBoxDetect, and is freed after
 * BVDC_P_BoxDetect_ApplyChanges. BVDC_ApplyChanges does NOT need to call
 * any "validatition" routine for BoxDetect, because after box-detect
 * enabling succeeded for a window, no thing could cause validation failure
 * from BoxDetect sub-module. Two windows that share the same source should
 * use the same box detect module.
 *
 * BVDC_P_BoxDetect_Do_isr is called by window as adjusting the rect before
 * RUL is built at each vsync. It outputs the box black cut rect. It is up
 * to the window to decide whether it is used. BVDC_P_BoxDetect_Do_isr also
 * output BoxDetectInfo and ulCallBckCntr that is needed for call-back.
 * Window should involve call-back when ulCallBckCntr changes.
 *
 * At each source vsync interrupt handler, BVDC_P_BoxDetect_BuildRul_isr
 * should be called when RUL is built. It is OK to called with all window's
 * writer that share the same box detect.
 */

/*-------------------------------------------------------------------------
 * macro used by box-detect sub-module
 */
#define BVDC_P_BOX_DETECT_ID_BIT(id)              (1<<id)
#define BVDC_P_BOX_DETECT_MAX_EDGE                (2047)

#define BVDC_P_BOX_DETECT_FAST_DETECT             1

#ifdef BVDC_P_BOX_DETECT_FAST_DETECT
#define BVDC_P_BOX_DETECT_NUM_ACCUM_VIDEO         2
#define BVDC_P_BOX_DETECT_BREAK_THRESH_VIDEO     20
#define BVDC_P_BOX_DETECT_ERR_THRESH             32
#else
#define BVDC_P_BOX_DETECT_NUM_ACCUM_VIDEO        60
#define BVDC_P_BOX_DETECT_BREAK_THRESH_VIDEO      3
#define BVDC_P_BOX_DETECT_ERR_THRESH              3
#endif
#define BVDC_P_BOX_DETECT_NUM_ACCUM_PC            2
#define BVDC_P_BOX_DETECT_BREAK_THRESH_PC         4

#define BVDC_P_BoxDetect_MuxAddr(hBoxDetect)   (BCHP_VNET_F_LBOX_0_SRC + (hBoxDetect)->eId * sizeof(uint32_t))

#define BVDC_P_BoxDetect_SetVnet_isr(hBoxDetect, ulSrcMuxValue, eVnetPatchMode) \
   BVDC_P_SubRul_SetVnet_isr(&((hBoxDetect)->SubRul), ulSrcMuxValue, eVnetPatchMode)
#define BVDC_P_BoxDetect_UnsetVnet_isr(hBoxDetect) \
   BVDC_P_SubRul_UnsetVnet_isr(&((hBoxDetect)->SubRul))

/*-------------------------------------------------------------------------
 * box detect main context
 */
typedef struct BVDC_P_BoxDetectContext
{
    BDBG_OBJECT(BVDC_BOX)

    /* box detect Id */
    BVDC_P_BoxDetectId               eId;
    uint32_t                         ulRegOffset;
    uint32_t                         ulResetAddr;
    uint32_t                         ulResetMask;

    /* static info from creating */
    BREG_Handle                      hRegister;

    /* */
    BAVC_SourceId                    eSrcId;
    uint32_t                         ulCallBckCntr;
    const BVDC_P_Source_Info        *pCurSrcInfo;

    /* a threshhold of continuous box breaking => real break */
    uint32_t                         ulBreakThresh;
    uint32_t                         ulHorzBreakCnt;
    uint32_t                         ulVertBreakCnt;
    uint32_t                         ulHorzErrCnt;
    uint32_t                         ulVertErrCnt;

    /* current original src size */
    uint32_t                         ulCurSrcWidth;
    uint32_t                         ulCurSrcHeight;

    /* result read from hw and pass to user */
    BVDC_BoxDetectInfo               Box;

    /* cut rectangle if bAutoCutBlack, max if not */
    BVDC_P_Rect                      Cut;

    /* sub-struct to manage vnet and rul build opreations */
    BVDC_P_SubRulContext             SubRul;

} BVDC_P_BoxDetectContext;


/***************************************************************************
 * private functions
***************************************************************************/
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
      BVDC_P_Resource_Handle            hResource );

/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_Destroy
 *
 * called by BVDC_Close only
 */
BERR_Code BVDC_P_BoxDetect_Destroy
    ( BVDC_P_BoxDetect_Handle          hBoxDetect );

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
      const BVDC_P_Source_Info         *pCurSrcInfo );

/***************************************************************************
 * {private}
 *
 * BVDC_P_BoxDetect_ReleaseConnect_isr
 *
 * It is called after window decided that box-detect is no-longer used by HW in
 * its vnet mode (i.e. it is really shut down and teared off from vnet).
 */
BERR_Code BVDC_P_BoxDetect_ReleaseConnect_isr
    ( BVDC_P_BoxDetect_Handle         *phBoxDetect );

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
      uint32_t                         *pulCallBckCntr ); /* out */

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
      bool                           bEnable );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_BOXDETECT_PRIV_H__ */
/* End of file. */
