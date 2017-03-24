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
#include "bbox_vdc_box7_config.h"

BDBG_MODULE(BBOX_VDC_PRIV_BOX7);
BDBG_OBJECT_ID(BBOX_VDC_BOX_PRIV_BOX7);

/* Memc Index for box mode 7. BBOX_MemcIndex_Invalid means it's not used */
static const BBOX_MemConfig stBoxMemConfig_7439B0_box7 =
{
   {
      BBOX_MK_RDC_MEMC_IDX(0),       /* RDC */
      BBOX_MK_DVI_CFC_MEMC_IDX(Invalid), /* HDMI display CFC */
      {
         BBOX_MK_WIN_MEMC_IDX(1,       1,       1,       1,       1      ),  /* disp 0 */
         BBOX_MK_WIN_MEMC_IDX(1,       0,       Invalid, Invalid, 1      ),  /* disp 1 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 2 */
         BBOX_MK_WIN_MEMC_IDX(0,       Invalid, 0,       Invalid, Invalid),  /* disp 3 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 4 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 5 */
         BBOX_MK_WIN_MEMC_IDX(Invalid, Invalid, Invalid, Invalid, Invalid),  /* disp 6 */
      }
   },
   BBOX_INVALID_NUM_MEMC
};

void BBOX_P_Vdc_SetBox7SourceCapabilities
    ( BBOX_Vdc_Source_Capabilities *pSourceCap )
{
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, Mpeg0,  MTG_DISABLE,   BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Disregard, 10bit,     false);
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, Mpeg1,  MTG_DISABLE,   BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Disregard, 8bit,      false);
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, Mpeg2,  MTG_DISABLE,   BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Disregard, 8bit,      false);
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, Mpeg3,  MTG_DISABLE,   BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Disregard, 8bit,      false);
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, HdDvi0, MTG_DISABLE,   3840,               BFMT_2160P_HEIGHT,  Disregard, Disregard, false);
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, Gfx0,   MTG_DISABLE,   BFMT_1080P_WIDTH,   BFMT_1080P_HEIGHT,  RGB,       8bit,      false);
    BBOX_P_VDC_SET_SRC_LIMIT(pSourceCap, Gfx1,   MTG_DISABLE,   BFMT_576P_WIDTH,    BFMT_576P_HEIGHT,   RGB,       8bit,      false);
}

void BBOX_P_Vdc_SetBox7DisplayCapabilities
    ( BBOX_Vdc_Display_Capabilities *pDisplayCap )
{
    BBOX_P_VDC_SET_DISPLAY_LIMIT(pDisplayCap, Display0, 3840x2160p_50Hz, 3840x2160p_50Hz, Invalid, Invalid, Invalid, Class2);
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display0, Video0, DISREGARD, BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, SclBeforeCap);
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display0, Video1, DISREGARD, 1,                  1,                  SclBeforeCap);  /* can be full size if decode is 1080p25/50i or less. If larger, then 1/2 x 1/2. */
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display0, Gfx0,   INVALID,   BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Disregard);

    BBOX_P_VDC_SET_DISPLAY_LIMIT(pDisplayCap, Display1, PAL_G,           PAL_G,           Invalid, Invalid, Invalid, Class0);
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display1, Video0, INVALID,   BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, SclBeforeCap);
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display1, Video1, INVALID,   1,                  1,                  SclBeforeCap); /* can be full size if decode is 1080p25/50i or less. If larger, then 1/2 x 1/2. */
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display1, Gfx0,   INVALID,   BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, Disregard);

    BBOX_P_VDC_SET_DISPLAY_LIMIT(pDisplayCap, Display2, 720p_25Hz,       720p_25Hz,       1,       0,       1,       Disregard);
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display2, Video0, DISREGARD, BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, AutoDisable);

    BBOX_P_VDC_SET_DISPLAY_LIMIT(pDisplayCap, Display3, 720p_25Hz,       720p_25Hz,       0,       0,       0,       Disregard);
        BBOX_P_VDC_SET_WINDOW_LIMIT(pDisplayCap, Display3, Video0, DISREGARD, BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD, AutoDisable);
}

void BBOX_P_Vdc_SetBox7DeinterlacerCapabilities
    ( BBOX_Vdc_Deinterlacer_Capabilities *pDeinterlacerCap )
{
    BBOX_P_VDC_SET_DEINTERLACER_LIMIT(pDeinterlacerCap, Deinterlacer0, BFMT_1080I_WIDTH, BFMT_1080I_HEIGHT, BBOX_VDC_DISREGARD);
    BBOX_P_VDC_SET_DEINTERLACER_LIMIT(pDeinterlacerCap, Deinterlacer1, BFMT_1080I_WIDTH, BFMT_1080I_HEIGHT, BBOX_VDC_DISREGARD);
    BBOX_P_VDC_SET_DEINTERLACER_LIMIT(pDeinterlacerCap, Deinterlacer2, 1280, BFMT_1080I_HEIGHT, 1280);
    BBOX_P_VDC_SET_DEINTERLACER_LIMIT(pDeinterlacerCap, Deinterlacer3, 1280, BFMT_1080I_HEIGHT, 1280);
}

void BBOX_P_Vdc_SetBox7XcodeCapabilities
    ( BBOX_Vdc_Xcode_Capabilities *pXcodeCap )
{
    BBOX_P_VDC_SET_XCODE_LIMIT(pXcodeCap, BBOX_VDC_DISREGARD, BBOX_VDC_DISREGARD);
}

void BBOX_P_GetBox7MemConfig
    ( BBOX_MemConfig                *pBoxMemConfig )
{
    *pBoxMemConfig = stBoxMemConfig_7439B0_box7;
    pBoxMemConfig->ulNumMemc = stBoxRts_7439_4Kstb2t50_box7.ulNumMemc;
}

void BBOX_P_GetBox7Rts
    ( BBOX_Rts *pBoxRts )
{
    *pBoxRts = stBoxRts_7439_4Kstb2t50_box7;
}
