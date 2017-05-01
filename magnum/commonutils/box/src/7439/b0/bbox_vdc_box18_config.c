/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
******************************************************************************/

#include "bstd.h"                /* standard types */
#include "bkni.h"
#include "bdbg.h"                /* Debug message */
#include "bbox.h"
#include "bbox_priv_modes.h"
#include "bbox_vdc.h"
#include "bbox_vdc_priv.h"
#include "bbox_vdc_box18_config.h"

BDBG_MODULE(BBOX_VDC_PRIV_BOX18);
BDBG_OBJECT_ID(BBOX_VDC_BOX_PRIV_BOX18);

/* Memc Index for box mode 18. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box18 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 0,       Invalid,       1),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(1,       Invalid, 0,       Invalid,       1),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC,
   BBOX_MK_DRAM_REFRESH_RATE(1x)
};

void BBOX_P_Vdc_SetBox18SourceCapabilities
    ( BBOX_Vdc_Source_Capabilities *pSourceCap )
{
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, Mpeg0,  MTG_DISABLE,  BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Disregard, 10bit,     false);
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, Mpeg1,  MTG_DISABLE,  BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Disregard, 10bit,     false);
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, HdDvi0, MTG_DISABLE,  3840,               BFMT_2160P_HEIGHT,  Disregard, Disregard, false);
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, Gfx2,   MTG_DISABLE,  BFMT_1080P_WIDTH,   BFMT_1080P_HEIGHT,  RGB,       8bit,      false);
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, Gfx3,   MTG_DISABLE,  BFMT_1080P_WIDTH,   BFMT_1080P_HEIGHT,  RGB,       8bit,      false);
}

void BBOX_P_Vdc_SetBox18DisplayCapabilities
    ( BBOX_Vdc_Display_Capabilities *pDisplayCap )
{
    BBOX_P_VDC_SET_DISPLAY_LIMIT(pDisplayCap, Display2, 1080p_30Hz, 1080p_30Hz, 1, 0, 1, Disregard);
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display2, Video0, DISREGARD, Disregard, Disregard, Disregard, BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Auto);
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display2, Gfx0,   INVALID,   Disregard, Disregard, Disregard, BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Disregard);

    BBOX_P_VDC_SET_DISPLAY_LIMIT(pDisplayCap, Display3, 1080p_30Hz, 1080p_30Hz, 0, 0, 0, Disregard);
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display3, Video0, DISREGARD, Disregard, Disregard, Disregard, BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Auto);
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display3, Gfx0,   INVALID,   Disregard, Disregard, Disregard, BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Disregard);
}

void BBOX_P_Vdc_SetBox18DeinterlacerCapabilities
    ( BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap )
{
    BBOX_P_VDC_SET_DEINTERLACER_LIMIT(pDeinterlacerCap, Deinterlacer2, BFMT_1080I_WIDTH, BFMT_1080I_HEIGHT, 1920);
    BBOX_P_VDC_SET_DEINTERLACER_LIMIT(pDeinterlacerCap, Deinterlacer3, BFMT_1080I_WIDTH, BFMT_1080I_HEIGHT, 1920);
}

void BBOX_P_Vdc_SetBox18XcodeCapabilities
    ( BBOX_Vdc_Xcode_Capabilities *pXcodeCap )
{
    BBOX_P_VDC_SET_XCODE_LIMIT(pXcodeCap, BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD);
}

void BBOX_P_GetBox18MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    *pBoxMemConfig = stBoxMemConfig_7439B0_box18;
    pBoxMemConfig->ulNumMemc = stBoxRts_7252_headless_dualxcode_box18.ulNumMemc;
}

void BBOX_P_GetBox18Rts
    ( BBOX_Rts *pBoxRts )
{
    *pBoxRts = stBoxRts_7252_headless_dualxcode_box18;
}
