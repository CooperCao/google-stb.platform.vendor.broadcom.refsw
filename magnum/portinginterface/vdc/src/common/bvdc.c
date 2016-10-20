/******************************************************************************
 * Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
#include "bstd.h"                /* standard types */
#include "bdbg.h"                /* Dbglib */
#include "bkni.h"                /* malloc */
#include "btmr.h"                /* timer */

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

/* Note: Tricky here!  bavc.h needs bchp_gfd_x.h defininitions.
 * The check here is to see if chips has more than one gfx feeder. */
#include "bchp_gfd_0.h"
#include "bchp_gfd_1.h"

#include "bvdc.h"                /* Video display */
#include "bvdc_priv.h"           /* VDC internal data structures */
#include "bvdc_common_priv.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_bufferheap_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_window_priv.h"  /* only for err msg as time out */
#include "bvdc_display_priv.h"

#include "bchp_fmisc.h"
#include "bchp_mmisc.h"
#include "bchp_bmisc.h"
#include "bchp_timer.h"

#if BVDC_P_SUPPORT_DMISC
#include "bchp_dmisc.h"
#endif
#if BVDC_P_SUPPORT_VIP
#include "bvdc_displayvip_priv.h"
#endif

BDBG_MODULE(BVDC);
BDBG_FILE_MODULE(BVDC_WIN_BUF);
BDBG_OBJECT_ID(BVDC_VDC);

/* This table used to indicate which DACs belong to each group. 0 for unused DAC */
const uint32_t s_aulDacGrouping[BVDC_MAX_DACS] =
{
#if (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_0)
    1, 1, 1, 1, 0, 0, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_1)
    1, 1, 1, 2, 2, 2, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_2)
    /* TODO: Need to confirm if TDAC = DAC[0-2] and QDAC = DAC[3-6] or
       TDAC = DAC[4-6] and QDAC = DAC[0-3] */
    1, 1, 1, 2, 2, 2, 2
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_3)
    1, 1, 1, 2, 2, 2, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_4)
    1, 1, 1, 2, 2, 2, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_5)
    1, 1, 1, 0, 0, 0, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_6)
    1, 1, 1, 2, 2, 2, 2
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_7)
    1, 1, 1, 2, 2, 2, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_8)
    1, 1, 1, 1, 0, 0, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_9)
    1, 1, 1, 1, 0, 0, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_10)
    1, 1, 1, 1, 0, 0, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_11)
    1, 1, 1, 1, 0, 0, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_12)
    1, 1, 1, 1, 0, 0, 0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_13)
    1, 1, 1, 1, 0, 0, 0
#else
#error "Unknown chip!  Not yet supported in VDC."
#endif
};

/* Default settings. */
const BVDC_Settings s_stDefaultSettings =
{
    BFMT_VideoFmt_eNTSC,
    BAVC_FrameRateCode_e59_94,             /* Most HDMI monitors support 60Hz */
    BAVC_MatrixCoefficients_eItu_R_BT_709, /* default HD color matrix */
    BAVC_MatrixCoefficients_eSmpte_170M,   /* default SD color matrix */
    true,                                  /* VEC swap, cmp_0 -> vec_1 */
    false,                                 /* do not support FGT */

    /* Memory controller setttins. */
    {
        /* Quad HD Buffer settings */
        BVDC_P_MAX_4HD_BUFFER_COUNT,           /* default 4HD buffer count */
        BVDC_P_CAP_PIXEL_FORMAT_8BIT422,       /* default capture Pixel format */
        BFMT_VideoFmt_e4096x2160p_24Hz,        /* 4HD buffer is 4096x2160p */
        BVDC_P_DEFAULT_4HD_PIP_BUFFER_COUNT,   /* default 1/4 4HD buffer count */

        /* Double HD Buffer settings */
        BVDC_P_MAX_2HD_BUFFER_COUNT,           /* default 2HD buffer count */
        BVDC_P_CAP_PIXEL_FORMAT_8BIT422,       /* default capture Pixel format */
        BFMT_VideoFmt_e1080p_30Hz,             /* 2HD buffer is 1080p */
        BVDC_P_DEFAULT_2HD_PIP_BUFFER_COUNT,   /* default 1/4 2HD buffer count */

        /* HD buffer settings */
        BVDC_P_MAX_HD_BUFFER_COUNT,            /* default HD buffer count */
        BVDC_P_CAP_PIXEL_FORMAT_8BIT422,                 /* default capture Pixel format */
        BFMT_VideoFmt_e1080i,                  /* HD buffer is 1080i */
        BVDC_P_DEFAULT_HD_PIP_BUFFER_COUNT,    /* default 1/4 HD buffer count */

        /* SD buffer settings */
        BVDC_P_MAX_SD_BUFFER_COUNT,            /* default SD buffer count */
        BVDC_P_CAP_PIXEL_FORMAT_8BIT422,       /* default capture Pixel format */
        BFMT_VideoFmt_ePAL_G,                  /* default SD buffer is PAL */
        BVDC_P_DEFAULT_SD_PIP_BUFFER_COUNT     /* default 1/4 SD buffer count */
    },

    /* New Video DAC bandgap adjust */
    {
#if (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_0)
        BCHP_MISC_QDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_QDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_QDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_QDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        0,
        0,
        0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_1)
        BCHP_MISC_TDAC0_CTRL_REG_BG_PTATADJ_NORM,
        BCHP_MISC_TDAC0_CTRL_REG_BG_PTATADJ_NORM,
        BCHP_MISC_TDAC0_CTRL_REG_BG_PTATADJ_NORM,
        BCHP_MISC_TDAC1_CTRL_REG_BG_PTATADJ_NORM,
        BCHP_MISC_TDAC1_CTRL_REG_BG_PTATADJ_NORM,
        BCHP_MISC_TDAC1_CTRL_REG_BG_PTATADJ_NORM,
        0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_2)
    /* TODO: Need to confirm if TDAC = DAC[0-2] and QDAC = DAC[3-6] or
       TDAC = DAC[4-6] and QDAC = DAC[0-3] */
        BCHP_MISC_TDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_QDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_QDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_QDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_QDAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_3)
        BCHP_MISC_TDAC0_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC0_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC0_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC1_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC1_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC1_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_4)
        BCHP_MISC_TDAC0_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC0_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC0_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC1_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC1_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_TDAC1_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_5)
        BCHP_MISC_DAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_REG_IREF_ADJ_TWENTY_SIX,
        0,
        0,
        0,
        0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_6)
        BCHP_MISC_DAC_BG_CTRL_0_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_0_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_0_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_1_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_1_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_1_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_1_IREF_ADJ_TWENTY_SIX
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_7)
        BCHP_MISC_DAC_BG_CTRL_0_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_0_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_0_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_1_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_1_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_1_IREF_ADJ_TWENTY_SIX,
        0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_8)
        BCHP_MISC_DAC_BG_CTRL_0_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_0_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_0_IREF_ADJ_TWENTY_SIX,
        BCHP_MISC_DAC_BG_CTRL_0_IREF_ADJ_TWENTY_SIX,
        0,
        0,
        0
#elif ((BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_9)  || \
       (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_10) || \
       (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_11) || \
       (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_12))
        240,
        240,
        240,
        240,
        0,
        0,
        0
#elif (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_13)
        201,
        201,
        201,
        201,
        0,
        0,
        0
#else
#error "Unknown chip!  Not yet supported in VDC."
#endif
    },
    BVDC_Mode_eAuto,
    false,
    false,
    false,
    NULL /* box mode */

};

/* Here is a little consistency check */
#if (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_1)
    #if (BCHP_MISC_TDAC0_CTRL_REG_BG_PTATADJ_NORM != \
         BCHP_MISC_TDAC0_CTRL_REG_BG_CTATADJ_NORM)
        #error bandgap constants not equal
    #endif
#endif

/* Available features
 * INDEX: by compositor id, window id source id */
const BVDC_P_Features s_VdcFeatures =
{
#if (BCHP_CHIP==7422) || (BCHP_CHIP==7425)
    false,
    /* cmp0   cmp1   cmp2   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  true,  true,  false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true,  false, false, false, false, false, false, false, true,  true,  true,  true,  false, false, false, true,  false, false, true,  true,  true,  true,  true,  false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true,  false, false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },
#elif (BCHP_CHIP==7435)
    false,
    /* cmp0   cmp1   cmp2   cmp3   cmp4   cmp5   cmp6  */
    {  true,  true,  true,  true,  true,  true,  false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true,  true,  false, false, false, false, false, false, true,  true,  true,  true,  true,  true,  false, true,  false, false, true,  true,  true,  true,  true,  true,  false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true,  true,  false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },
#elif (BCHP_CHIP==7358) || (BCHP_CHIP==7552) || (BCHP_CHIP==7360) || \
      (BCHP_CHIP==7362) || (BCHP_CHIP==7563) || (BCHP_CHIP==7543) || \
      (BCHP_CHIP==7228) || (BCHP_CHIP==75635) || (BCHP_CHIP==73625) || (BCHP_CHIP==75525)
    false,
    /* cmp0   cmp1   cmpb   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  false, false, false, false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },
#elif (BCHP_CHIP==7344) || (BCHP_CHIP==7346) || (BCHP_CHIP==73465)
    false,
    /* cmp0   cmp1   cmpb   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false, false, true, true,   false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },
#elif (BCHP_CHIP==7231) && (BCHP_VER == BCHP_VER_A0)
    false,
    /* cmp0   cmp1   cmpb   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false,  false, true, true,  false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },
#elif (BCHP_CHIP==7231) || (BCHP_CHIP==7429) || (BCHP_CHIP==7364)|| (BCHP_CHIP==7250) || (BCHP_CHIP==74295)
    false,
    /* cmp0   cmp1   cmpb   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false,  false, true, true,  false, false, false, false, false, true,  false, false, true,  true,  false, false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },
#elif (BCHP_CHIP==7584) || (BCHP_CHIP==75845)
    false,
    /* cmp0   cmp1   cmpb   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false,  false, true, true,  false, false, false, false, false, false, false, false, true,  true,  true,  true,  false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },
#elif (BCHP_CHIP==7445) || (BCHP_CHIP==11360)
    false,
    /* cmp0   cmp1   cmp2   cmp3   cmp4   cmp5   cmp6  */
    {  true,  true,  true,  true,  true,  true,  true  },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true,  true,  true , true,  false, false, false, false, true,  true,  true,  true,  true,  true,  true,  true,  false, false, true,  true,  true,  true,  true,  true,  true,  true  },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true,  true,  true,  true,  false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },

#elif (BCHP_CHIP==7439) && (BCHP_VER>=BCHP_VER_B0)
    false,
    /* cmp0   cmp1   cmp2   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  true,  true,  false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true,  true, false, false, false, false, false, false, true,  true,  true,  true,  false, false, false, true,  false, false, true,  true,  true,  true,  true,  false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true,  true, false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },

#elif (BCHP_CHIP==7366) && (BCHP_VER>=BCHP_VER_B0)
    false,
    /* cmp0   cmp1   cmp2   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  true,  false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true, false, false, false, false, false, false, false, true,  true,  true,  false, false, false, false, false, false, false, true,  true,  true,  true,  false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true, false, false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },

#elif (BCHP_CHIP==7366) ||((BCHP_CHIP==7439) && (BCHP_VER==BCHP_VER_A0)) || \
      ((BCHP_CHIP==74371) && (BCHP_VER==BCHP_VER_A0))
    false,
    /* cmp0   cmp1   cmp2   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  true,  false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false, false, true,  true,  true,  false, false, false, false, true,  false, false, true,  true,  true,  true,  false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },
#elif (BCHP_CHIP==7586)
    false,
    /* cmp0   cmp1   cmpb   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false,  false, true, true,  false, false, false, false, false, true,  false, false, true,  true,  true,  true,  false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  false, false, false, false, false, false, false, false, true,  false, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },
#elif (BCHP_CHIP==7271) || (BCHP_CHIP==7268)
    false,
    /* cmp0   cmp1   cmpb   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, true, false, false,  true,  true,  false, false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true, true, false, false, false, false, false, false, false, false,  true,  true, false, false, false, false, false,  true, false, false,  true,  true, false, false, false, false, false, false },
#elif (BCHP_CHIP==7260)
    false,
    /* cmp0   cmp1   cmpb   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true, false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false, false, false, true,  true,  false, false, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true, true, false, false, false, false, false, false, false, false,  true,  true, false, false, false, false, false, false, false, false,  true,  true, false, false, false, false, false, false },
#elif (BCHP_CHIP==7278)
    false,
    /* cmp0   cmp1   cmpb   cmp3   cmp4   cmp5   cmp6 */
    {  true,  true,  true,  true, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false,  true, false, false,  true,  true,  true,  true, false, false, false, false },

    /* mpg0   mpg1   mpg2   mpg3   mpg4   mpg5   vdec0  vdec1  656_0  656_1  gfx0   gfx1   gfx2   gfx3   gfx4   gfx5   gfx6   dvi0   dvi1   ds 0   vfd0   vfd1   vfd2   vfd3   vfd4   vfd5   vfd6   vfd7  */
    {  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false,  true, false, false,  true,  true,  true,  true, false, false, false, false },
#else
#error "Unknown chip!  Not yet supported in VDC."
#endif
};


/***************************************************************************
 * This function does a soft-reset of each VDC modules.
 */
void BVDC_P_PrintHeapInfo
    ( const BVDC_Heap_Settings        *pHeap )
{
    const BFMT_VideoInfo *pFmtInfo;

    pFmtInfo = BFMT_GetVideoFormatInfoPtr(pHeap->eBufferFormat_4HD);
    if(pFmtInfo == NULL)
        return;

    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("--------4HD---------"));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ulBufferCnt     = %d", pHeap->ulBufferCnt_4HD));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ulBufferCnt_Pip = %d", pHeap->ulBufferCnt_4HD_Pip));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("eBufferFormat   = %s", pFmtInfo->pchFormatStr));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ePixelFormat    = %s", BPXL_ConvertFmtToStr(pHeap->ePixelFormat_4HD)));

    pFmtInfo = BFMT_GetVideoFormatInfoPtr(pHeap->eBufferFormat_2HD);
    if(pFmtInfo == NULL)
        return;
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("--------2HD---------"));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ulBufferCnt     = %d", pHeap->ulBufferCnt_2HD));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ulBufferCnt_Pip = %d", pHeap->ulBufferCnt_2HD_Pip));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("eBufferFormat   = %s", pFmtInfo->pchFormatStr));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ePixelFormat    = %s", BPXL_ConvertFmtToStr(pHeap->ePixelFormat_2HD)));

    pFmtInfo = BFMT_GetVideoFormatInfoPtr(pHeap->eBufferFormat_HD);
    if(pFmtInfo == NULL)
        return;
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("---------HD---------"));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ulBufferCnt     = %d", pHeap->ulBufferCnt_HD));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ulBufferCnt_Pip = %d", pHeap->ulBufferCnt_HD_Pip));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("eBufferFormat   = %s", pFmtInfo->pchFormatStr));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ePixelFormat    = %s", BPXL_ConvertFmtToStr(pHeap->ePixelFormat_HD)));

    pFmtInfo = BFMT_GetVideoFormatInfoPtr(pHeap->eBufferFormat_SD);
    if(pFmtInfo == NULL)
        return;
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("---------SD---------"));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ulBufferCnt     = %d", pHeap->ulBufferCnt_SD));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ulBufferCnt_Pip = %d", pHeap->ulBufferCnt_SD_Pip));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("eBufferFormat   = %s", pFmtInfo->pchFormatStr));
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("ePixelFormat    = %s", BPXL_ConvertFmtToStr(pHeap->ePixelFormat_SD)));

    return;
}


/***************************************************************************
 *
 */
static void BVDC_P_ResetBvn
    ( BVDC_P_Context                  *pVdc )
{
    bool bFreeRun;
    uint32_t ulReg;

    /* Note we can not use the SUN_TOP_CTRL_SW_RESET, because it would
     * also reset the RDC, and MEMC memory controller. */
#if (BVDC_P_SUPPORT_CLOCK_GATING)
    bFreeRun = false;
#else
    bFreeRun = true;
#endif

    /* Reset BVN front-end, middle-end, and back-end modules.
     * To reset write a 1 to a bit, and then write a 0.*/
    /*---------------------------*/
    /* FRONT-END & MIDDLE BLOCKS */
    /*---------------------------*/
    ulReg = 0xffffffff;
    ulReg &= ~(
    BCHP_MASK(FMISC_SW_INIT, RDC ));
    BREG_Write32(pVdc->hRegister, BCHP_FMISC_SW_INIT, ulReg);
    BREG_Write32(pVdc->hRegister, BCHP_FMISC_SW_INIT, 0);

#ifdef BCHP_FMISC_BVNF_CLOCK_CTRL
    BREG_Write32(pVdc->hRegister, BCHP_FMISC_BVNF_CLOCK_CTRL,
        BCHP_FIELD_DATA(FMISC_BVNF_CLOCK_CTRL, CLK_FREE_RUN_MODE,
            (bFreeRun || BVDC_P_SUPPORT_CLOCK_GATING_FMISC_FR) ? 1 : 0));
#endif

#if (BVDC_P_SUPPORT_MEM_PWR_GATING)
    BREG_Write32(pVdc->hRegister, BCHP_FMISC_BVN_PDA_CTRL,
        BCHP_FIELD_DATA(FMISC_BVN_PDA_CTRL, DMPG_EN, 1));
#endif
    /*---------------*/
    /* MAD BLOCKS    */
    /*---------------*/
#if BVDC_P_SUPPORT_DMISC
    ulReg = 0xffffffff;
    BREG_Write32(pVdc->hRegister, BCHP_DMISC_SW_INIT, ulReg);
    BREG_Write32(pVdc->hRegister, BCHP_DMISC_SW_INIT, 0);
#endif

#ifdef BCHP_DMISC_BVND_MAD_0_CLOCK_CTRL
    BREG_Write32(pVdc->hRegister, BCHP_DMISC_BVND_MAD_0_CLOCK_CTRL,
        BCHP_FIELD_DATA(DMISC_BVND_MAD_0_CLOCK_CTRL, CLK_FREE_RUN_MODE, bFreeRun));
#endif

    /*---------------*/
    /* MIDDLE BLOCKS */
    /*---------------*/
    ulReg = 0xffffffff;
    BREG_Write32(pVdc->hRegister, BCHP_MMISC_SW_INIT, ulReg);
    BREG_Write32(pVdc->hRegister, BCHP_MMISC_SW_INIT, 0);

#ifdef BCHP_MMISC_BVNM_CLOCK_CTRL
    BREG_Write32(pVdc->hRegister, BCHP_MMISC_BVNM_CLOCK_CTRL,
        BCHP_FIELD_DATA(MMISC_BVNM_CLOCK_CTRL, CLK_FREE_RUN_MODE, bFreeRun));
#endif

    /*------------------*/
    /* BACK-END BLOCKS */
    /*------------------*/
    ulReg = 0xffffffff;
    BREG_Write32(pVdc->hRegister, BCHP_BMISC_SW_INIT, ulReg);
    BREG_Write32(pVdc->hRegister, BCHP_BMISC_SW_INIT, 0);

#ifdef BCHP_BMISC_BVNB_CLOCK_CTRL
    BREG_Write32(pVdc->hRegister, BCHP_BMISC_BVNB_CLOCK_CTRL,
        BCHP_FIELD_DATA(BMISC_BVNB_CLOCK_CTRL, CLK_FREE_RUN_MODE, bFreeRun));
#endif

    return;
}


/***************************************************************************
 * This function does a soft-reset of each VDC modules.
 */
static void BVDC_P_SoftwareReset
    ( BVDC_P_Context                  *pVdc )
{
    /* Reset all BVN modules in BVN_Front, BVN_Middle, BVN_Back */
    BVDC_P_ResetBvn(pVdc);

    /* Reset all Vec modules in Sundry, and initialize vec Misc regs. */
    BVDC_P_ResetVec(pVdc);

    return;
}

#ifndef BVDC_FOR_BOOTUPDATER
/***************************************************************************
 * Check VDC DAC bandgap default settings.
 */
static BERR_Code BVDC_P_CheckBandgapDefSettings
    ( const BVDC_Settings             *pDefSettings )
{
    BERR_Code  eStatus = BERR_SUCCESS;
    uint32_t   id;

    for(id = 0; id < BVDC_MAX_DACS; id++)
    {
        BDBG_MSG(("DAC %d BG setting = %d", id + 1, pDefSettings->aulDacBandGapAdjust[id]));
    }

    for(id = 1; id < BVDC_MAX_DACS; id++)
    {
        if(s_aulDacGrouping[id] != 0 &&
           s_aulDacGrouping[id-1] == s_aulDacGrouping[id] &&
           pDefSettings->aulDacBandGapAdjust[id-1] != pDefSettings->aulDacBandGapAdjust[id])
        {
            BDBG_ERR(("BG setting for DAC %d = %d should be same as DAC %d = %d",
                id, pDefSettings->aulDacBandGapAdjust[id],
                id - 1, pDefSettings->aulDacBandGapAdjust[id-1]));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    return eStatus;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_GetDefaultSettings
    ( const BBOX_Handle                hBox,
      BVDC_Settings                   *pDefSettings )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_GetDefaultSettings);

    if(pDefSettings)
    {
        *pDefSettings = s_stDefaultSettings;

        pDefSettings->hBox = hBox;

        /* Disable  bVecSwap                                                  */
        /* so that DISP_0 (main display) can be created with CMP_0 because    */
        /* currently the HD_SRC is assumed to be with the VEC that connects   */
        /* to CMP_0.  This can be removed after the assumption is removed     */
        pDefSettings->bVecSwap = false;
    }

    BDBG_LEAVE(BVDC_GetDefaultSettings);
    return err;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
BERR_Code BVDC_GetMaxCompositorCount
    ( const BVDC_Handle                hVdc,
      uint32_t                        *pulCompositorCount )
{
    BDBG_ENTER(BVDC_GetMaxCompositorCount);
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    /* set return value */
    if(pulCompositorCount)
    {
        *pulCompositorCount = BVDC_P_MAX_COMPOSITOR_COUNT;
    }

    BDBG_LEAVE(BVDC_GetMaxCompositorCount);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 * BVDC_Open()
 *
 */
#ifndef BVDC_FOR_BOOTUPDATER
BERR_Code BVDC_Open
    ( BVDC_Handle                     *phVdc,
      BCHP_Handle                      hChip,
      BREG_Handle                      hRegister,
      BMMA_Heap_Handle                 hMemory,
      BINT_Handle                      hInterrupt,
      BRDC_Handle                      hRdc,
      BTMR_Handle                      hTmr,
      const BVDC_Settings             *pDefSettings )
{
    BVDC_P_Context *pVdc = NULL;
    BERR_Code eStatus = BERR_SUCCESS;
    BTMR_Settings sTmrSettings;
    uint32_t i;

    BDBG_ENTER(BVDC_Open);
    BDBG_ASSERT(phVdc);
    BDBG_ASSERT(hChip);
    BDBG_ASSERT(hRegister);
    BDBG_ASSERT(hInterrupt);
    BDBG_ASSERT(hRdc);

    /* The handle will be NULL if create fails. */
    *phVdc = NULL;

    /* check VDC settings */
    if(pDefSettings)
    {
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("------VDC main heap settings------"));
        BVDC_P_PrintHeapInfo(&pDefSettings->stHeapSettings);
        eStatus = BERR_TRACE(BVDC_P_CheckHeapSettings(&pDefSettings->stHeapSettings));
        if( eStatus != BERR_SUCCESS )
        {
            goto BVDC_Open_Done;
        }

        eStatus = BERR_TRACE(BVDC_P_CheckBandgapDefSettings(pDefSettings));
        if( eStatus != BERR_SUCCESS )
        {
            goto BVDC_Open_Done;
        }
    }

    /* (1) Alloc the main VDC context. */
    pVdc = (BVDC_P_Context*)(BKNI_Malloc(sizeof(BVDC_P_Context)));
    if(NULL == pVdc)
    {
        eStatus = BERR_OUT_OF_SYSTEM_MEMORY;
        goto BVDC_Open_Done;
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pVdc, 0x0, sizeof(BVDC_P_Context));
    BDBG_OBJECT_SET(pVdc, BVDC_VDC);

    /* Store the hChip, hRegister, hMemory, and hRdc for later use. */
    pVdc->hChip      = hChip;
    pVdc->hRegister  = hRegister;
    pVdc->hMemory    = hMemory;
    pVdc->hInterrupt = hInterrupt;
    pVdc->hRdc       = hRdc;
    pVdc->hTmr       = hTmr;

    /* (1.1) Power managment */
#ifdef BCHP_PWR_RESOURCE_BVN
    BCHP_PWR_AcquireResource(pVdc->hChip, BCHP_PWR_RESOURCE_BVN);
#endif
#ifdef BCHP_PWR_RESOURCE_BVN_SRAM
    BCHP_PWR_AcquireResource(pVdc->hChip, BCHP_PWR_RESOURCE_BVN_SRAM);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
    BCHP_PWR_AcquireResource(pVdc->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC_SRAM
    BCHP_PWR_AcquireResource(pVdc->hChip, BCHP_PWR_RESOURCE_VDC_VEC_SRAM);
#endif

    /* (2) Initalize and start timer */
    if(!pVdc->hTimer)
    {
        BTMR_GetDefaultTimerSettings(&sTmrSettings);
        sTmrSettings.type = BTMR_Type_eSharedFreeRun;
        sTmrSettings.cb_isr = NULL;
        sTmrSettings.pParm1 = NULL;
        sTmrSettings.parm2 = 0;
        sTmrSettings.exclusive = true;

        eStatus = BTMR_CreateTimer(hTmr, &pVdc->hTimer, &sTmrSettings);
        if (eStatus != BERR_SUCCESS)
        {
            goto BVDC_Open_Done;
        }
    }

    /* Take in feature, this should be the centralize place to discover about
     * chip information and features. */
    pVdc->pFeatures = &s_VdcFeatures;

    /* Take in default settings. */
    pVdc->stSettings = (pDefSettings) ? *pDefSettings : s_stDefaultSettings;

    /* Initialize box modes */
    eStatus = BBOX_GetConfig(pVdc->stSettings.hBox, &pVdc->stBoxConfig);
    if (eStatus != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to get Box settings."));
    }

    /* Do we need to swap the CMP/VEC. */
    pVdc->bSwapVec = (
        (pVdc->stSettings.bVecSwap) &&
        (pVdc->pFeatures->abAvailCmp[BVDC_CompositorId_eCompositor1]));

    /* (3) Allocate Buffer Heap (VDC Internal) */
    if(pVdc->hMemory) {
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("--------------------------------- "));
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("BVDC_Open: create VDC main heap"));
        BVDC_P_BufferHeap_Create(pVdc, &pVdc->hBufferHeap, pVdc->hMemory,
            &pVdc->stSettings.stHeapSettings);
    }

    /* (4) Create resource */
    BVDC_P_Resource_Create(&pVdc->hResource, pVdc);

    /* Get mosaic coverage table */
    for(i = 0; i < BVDC_P_MAX_DISPLAY_COUNT; i++)
    {
        BVDC_P_MosaicCoverage_Init(&pVdc->stBoxConfig,
            i, &pVdc->stMosaicCoverageTbl[i]);
    }

#if BVDC_P_SUPPORT_VIP
    for(i = 0; i < BVDC_P_SUPPORT_VIP; i++)
    {
        BVDC_P_Vip_Create(&pVdc->ahVip[i], i, pVdc);
    }
#endif

    /* (5) Put Hardware into a known state. */
    BVDC_P_SoftwareReset((BVDC_Handle)pVdc);

    /* (6) Register BVN error recovery handler. and assert enum defines */
#if (BVDC_SUPPORT_BVN_DEBUG)
    eStatus = BVDC_P_CreateErrCb(pVdc);
    if(BERR_SUCCESS != eStatus)
    {
        goto BVDC_Open_Done;
    }
#endif

    /* (7) allocate drain buffer (2 pixels deep) for mosaic mode support */
#if BVDC_P_SUPPORT_MOSAIC_MODE
    if(pVdc->hMemory) {
        pVdc->hVdcMosaicMmaBlock = BMMA_Alloc(pVdc->hMemory,
            16, /* 2 pixels wide, in case 10-bit 4:2:2 capture rounding; */
            16,  /* 16 bytes aligned for capture engine */
            NULL  );

        if(!pVdc->hVdcMosaicMmaBlock)
        {
            BDBG_ERR(("Not enough device memory"));
            BDBG_ASSERT(0);
            return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        }

        pVdc->ullVdcNullBufOffset = BMMA_LockOffset(pVdc->hVdcMosaicMmaBlock);
    }
#endif

    /* (8) Initialize all DACs to unused */
    for (i = 0; i < BVDC_P_MAX_DACS; i++ )
    {
        pVdc->aDacOutput[i] = BVDC_DacOutput_eUnused;
        pVdc->aulDacSyncSource[i] = i;
    }
    pVdc->aulDacGrouping = s_aulDacGrouping;
    /* Default Auto = Off */
    pVdc->bDacDetectionEnable = (pVdc->stSettings.eDacDetection == BVDC_Mode_eOn) ? true : false;

    /* Reset used Xcode GFD counter */
    pVdc->ulXcodeGfd = 0;

    /* Get memory info */
    BCHP_GetMemoryInfo(pVdc->hChip, &pVdc->stMemInfo);

    /* All done. now return the new fresh context to user. */
    *phVdc = (BVDC_Handle)pVdc;

BVDC_Open_Done:
    BDBG_LEAVE(BVDC_Open);

    if ((BERR_SUCCESS != eStatus) && (NULL != pVdc))
    {
#ifdef BCHP_PWR_RESOURCE_BVN
        BCHP_PWR_ReleaseResource(pVdc->hChip, BCHP_PWR_RESOURCE_BVN);
#endif
#ifdef BCHP_PWR_RESOURCE_BVN_SRAM
        BCHP_PWR_ReleaseResource(pVdc->hChip, BCHP_PWR_RESOURCE_BVN_SRAM);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
        BCHP_PWR_ReleaseResource(pVdc->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC_SRAM
        BCHP_PWR_ReleaseResource(pVdc->hChip, BCHP_PWR_RESOURCE_VDC_VEC_SRAM);
#endif
        BDBG_OBJECT_DESTROY(pVdc, BVDC_VDC);
        BKNI_Free((void*)pVdc);
    }

    return BERR_TRACE(eStatus);
}


/***************************************************************************
 *
 */
BERR_Code BVDC_Close
    ( BVDC_Handle                      hVdc )
{
    uint32_t i;
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_Close);

    /* Return if trying to free a NULL handle. */
    if(!hVdc)
    {
        goto done;
    }

    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    /* [9] free drain buffer */
#if BVDC_P_SUPPORT_MOSAIC_MODE
    if(hVdc->hMemory) {
        BMMA_UnlockOffset(hVdc->hVdcMosaicMmaBlock, hVdc->ullVdcNullBufOffset);
        BMMA_Free(hVdc->hVdcMosaicMmaBlock);
    }
#endif

    /* [8] Un-Register BVN error recovery handler. */
#if (BVDC_SUPPORT_BVN_DEBUG)
    BVDC_P_DestroyErrCb(hVdc);
#endif

#if defined(BVDC_GFX_PERSIST)
    return BERR_SUCCESS;
#endif

    /* [7] Make sure we disable capture before we exit so that it would
     * not write to memory that potential contain heap bookeeping
     * next time we create. */
    BVDC_P_ResetBvn(hVdc);

#if BVDC_P_SUPPORT_VIP
    for(i = 0; i < BVDC_P_SUPPORT_VIP; i++)
    {
        BVDC_P_Vip_Destroy(hVdc->ahVip[i]);
    }
#endif

    /* [6] Free compositor handles . */
    for(i  = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
    {
        if(hVdc->pFeatures->abAvailCmp[i])
        {
            BVDC_P_Display_Destroy(hVdc->ahDisplay[i]);
            BVDC_P_Compositor_Destroy(hVdc->ahCompositor[i]);
        }
    }

    /* [5] Free sources handles. */
    for(i  = 0; i < BVDC_P_MAX_SOURCE_COUNT; i++)
    {
        if(hVdc->pFeatures->abAvailSrc[i])
        {
            /* SW7435-80: Skip these sources to save RUL memory */
            if((BVDC_P_SRC_IS_HDDVI(i) && hVdc->stSettings.bDisableHddviInput) ||
               (BVDC_P_SRC_IS_ITU656(i) && hVdc->stSettings.bDisable656Input))
            {
                BDBG_MSG(("User disabled SRC[%d] to save memory!", i));
                continue;
            }
            BVDC_P_Source_Destroy(hVdc->ahSource[i]);
        }
    }

    /* [4] destroy resource */
    BVDC_P_Resource_Destroy(hVdc->hResource);

    /* [3] Release Buffer Heap */
    if(hVdc->hBufferHeap) {
        BVDC_P_BufferHeap_Destroy(hVdc->hBufferHeap);
    }

    /* [2] Destroy Timer */
    if(hVdc->hTimer)
    {
        eStatus = BTMR_DestroyTimer(hVdc->hTimer);
        if (eStatus != BERR_SUCCESS)
        {
            return BERR_TRACE(eStatus);
        }
        hVdc->hTimer = NULL;
    }

    /* [1.1] Power managment */
#ifdef BCHP_PWR_RESOURCE_BVN
    BCHP_PWR_ReleaseResource(hVdc->hChip, BCHP_PWR_RESOURCE_BVN);
#endif
#ifdef BCHP_PWR_RESOURCE_BVN_SRAM
    BCHP_PWR_ReleaseResource(hVdc->hChip, BCHP_PWR_RESOURCE_BVN_SRAM);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
    BCHP_PWR_ReleaseResource(hVdc->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC_SRAM
    BCHP_PWR_ReleaseResource(hVdc->hChip, BCHP_PWR_RESOURCE_VDC_VEC_SRAM);
#endif

    /* [1] Release main context.   User will be responsible for destroying
     * compositors, sources, windows, and displays prior. */
    BDBG_OBJECT_DESTROY(hVdc, BVDC_VDC);
    BKNI_Free((void*)hVdc);

done:
    BDBG_LEAVE(BVDC_Close);
    return eStatus;
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

#if !B_REFSW_MINIMAL
/***************************************************************************
 *
 */
void BVDC_GetDefaultStandbySettings
    ( BVDC_StandbySettings            *pStandbypSettings )
{
    BSTD_UNUSED(pStandbypSettings);
    return;
}
#endif

/***************************************************************************
 *
 */
BERR_Code BVDC_Standby
    ( BVDC_Handle                      hVdc,
      const BVDC_StandbySettings      *pStandbypSettings )
{
    uint32_t i, j;
    bool bActive = false;
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    BSTD_UNUSED(pStandbypSettings);

    for(i = 0; i < BVDC_P_MAX_SOURCE_COUNT && !bActive; i++)
    {
        if(NULL != hVdc->ahSource[i])
        {
            bActive |= BVDC_P_STATE_IS_ACTIVE(hVdc->ahSource[i]);
            BDBG_MSG(("hVdc->ahSource[%d]=%d", i, BVDC_P_STATE_IS_ACTIVE(hVdc->ahSource[i])));
        }
    }

    for(i = 0; i < BVDC_P_MAX_DISPLAY_COUNT && !bActive; i++)
    {
        if(NULL!= hVdc->ahDisplay[i])
        {
            bActive |= BVDC_P_STATE_IS_ACTIVE(hVdc->ahDisplay[i]);
            BDBG_MSG(("hVdc->ahDisplay[%d]=%d", i, BVDC_P_STATE_IS_ACTIVE(hVdc->ahDisplay[i])));
        }
    }

    for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT && !bActive; i++)
    {
        if(NULL != hVdc->ahCompositor[i])
        {
            bActive |= BVDC_P_STATE_IS_ACTIVE(hVdc->ahCompositor[i]);
            BDBG_MSG(("hVdc->ahCompositor[%d]=%d", i, BVDC_P_STATE_IS_ACTIVE(hVdc->ahCompositor[i])));

            if(hVdc->pFeatures->abAvailCmp[i])
            {
                for(j = 0; j < BVDC_P_MAX_WINDOW_COUNT && !bActive; j++)
                {
                    if(NULL != hVdc->ahCompositor[i]->ahWindow[j])
                    {
                        bActive |= BVDC_P_STATE_IS_ACTIVE(hVdc->ahCompositor[i]->ahWindow[j]);
                        BDBG_MSG(("hVdc->ahCompositor[%d]->ahWindow[%d]=%d",
                            i, j, BVDC_P_STATE_IS_ACTIVE(hVdc->ahCompositor[i]->ahWindow[j])));
                    }
                }
            }
        }
    }

    BDBG_MSG(("Power Standby %s ready!", !bActive ? "is" : "is not"));

    if(bActive) {
        BDBG_ERR(("Cannot enter standby due to VDC in use"));
        return BERR_UNKNOWN;
    }
    /* if we get to this point, then nothing is in use and we can power down */
    if(!hVdc->bStandby)
    {
        BERR_Code eStatus = BERR_SUCCESS;

        /* Destroy Timer */
        if(hVdc->hTimer)
        {
            eStatus = BTMR_DestroyTimer(hVdc->hTimer);
            if (eStatus != BERR_SUCCESS)
            {
                return BERR_TRACE(eStatus);
            }
            hVdc->hTimer = NULL;
        }

#ifdef BCHP_PWR_RESOURCE_BVN
        BCHP_PWR_ReleaseResource(hVdc->hChip, BCHP_PWR_RESOURCE_BVN);
#endif
#ifdef BCHP_PWR_RESOURCE_BVN_SRAM
        BCHP_PWR_ReleaseResource(hVdc->hChip, BCHP_PWR_RESOURCE_BVN_SRAM);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
        BCHP_PWR_ReleaseResource(hVdc->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC_SRAM
        BCHP_PWR_ReleaseResource(hVdc->hChip, BCHP_PWR_RESOURCE_VDC_VEC_SRAM);
#endif
        hVdc->bStandby = true;
        BDBG_MSG(("Entering standby mode!"));
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_Resume
    ( BVDC_Handle                      hVdc )
{
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    if(!hVdc->bStandby)
    {
        BDBG_ERR(("Not in standby"));
        return BERR_UNKNOWN;
    }
    else
    {
#ifdef BCHP_PWR_RESOURCE_BVN
        BCHP_PWR_AcquireResource(hVdc->hChip, BCHP_PWR_RESOURCE_BVN);
#endif
#ifdef BCHP_PWR_RESOURCE_BVN_SRAM
        BCHP_PWR_AcquireResource(hVdc->hChip, BCHP_PWR_RESOURCE_BVN_SRAM);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC
        BCHP_PWR_AcquireResource(hVdc->hChip, BCHP_PWR_RESOURCE_VDC_VEC);
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_VEC_SRAM
        BCHP_PWR_AcquireResource(hVdc->hChip, BCHP_PWR_RESOURCE_VDC_VEC_SRAM);
#endif

        if(!hVdc->hTimer)
        {
            BERR_Code eStatus = BERR_SUCCESS;
            BTMR_Settings sTmrSettings;

            /* Initalize and start timer */
            BTMR_GetDefaultTimerSettings(&sTmrSettings);
            sTmrSettings.type = BTMR_Type_eSharedFreeRun;
            sTmrSettings.cb_isr = NULL;
            sTmrSettings.pParm1 = NULL;
            sTmrSettings.parm2 = 0;
            sTmrSettings.exclusive = true;

            eStatus = BTMR_CreateTimer(hVdc->hTmr, &hVdc->hTimer, &sTmrSettings);
            if (eStatus != BERR_SUCCESS)
            {
                return BERR_TRACE(eStatus);
            }
        }

        BVDC_P_SoftwareReset(hVdc);
        hVdc->bStandby = false;

        BDBG_MSG(("Leaving standby mode!"));
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
BERR_Code BVDC_GetMaxMosaicCoverage
    ( BVDC_Handle                      hVdc,
      BVDC_DisplayId                   eDispId,
      uint32_t                         ulRectsCount,
      uint32_t                        *pulCoverage )
{
    uint32_t  ulCoverage = 100;

    if(ulRectsCount)
    {
        BVDC_P_MosaicCanvasCoverage     *pCoverageTbl;

        pCoverageTbl = &hVdc->stMosaicCoverageTbl[eDispId];
        ulCoverage = pCoverageTbl->ulCanvasCoverage[ulRectsCount-1];
    }

    if(pulCoverage)
        *pulCoverage = ulCoverage;

    BDBG_MSG(("Disp[%d] ulRectsCount: %d, coverage: %d",
        eDispId, ulRectsCount, ulCoverage));

    return BERR_SUCCESS;
}

/***************************************************************************
 *
 */
#if (BDBG_DEBUG_BUILD)
static void BVDC_P_CheckDisplayAlignAdjustedStatus
    ( BVDC_Handle                      hVdc )
{
    uint32_t k;

    BDBG_ENTER(BVDC_P_CheckDisplayAlignAdjustStatus);
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    for (k = 0; k < BVDC_P_MAX_COMPOSITOR_COUNT; k++)
    {
        /* Bypass the inactive ones. */
        if(!hVdc->ahDisplay[k] || !hVdc->ahDisplay[k]->hCompositor)
        {
            continue;
        }

        if (hVdc->ahDisplay[k]->bAlignAdjusting)
        {
            BDBG_ERR(("Display %d is in the process of VEC alignment", k));
        }
    }

    BDBG_ENTER(BVDC_P_CheckDisplayAlignAdjustStatus);
    return;
}
#endif

/***************************************************************************
 *
 */
static BERR_Code BVDC_P_CheckApplyChangesStatus
    ( BVDC_Handle                      hVdc,
      bool                             bSynchronous )
{
    uint32_t i, j;
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_CheckApplyChangesStatus);
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    /* Active various debug message at runtime */
#if (BDBG_DEBUG_BUILD)
    {
        uint32_t ulReg = BREG_Read32(hVdc->hRegister, BCHP_BMISC_SCRATCH_0);
        hVdc->bForcePrint = (ulReg & (1 << 0)); /* BMISC_SCRATCH_0[00:00] */
                                                /* BMISC_SCRATCH_0[31:01] - avail */
    }
#endif

    for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
    {
        /* Bypass the inactive ones. */
        if(!hVdc->ahDisplay[i] || !hVdc->ahDisplay[i]->hCompositor)
        {
            continue;
        }

        BDBG_OBJECT_ASSERT(hVdc->ahDisplay[i], BVDC_DSP);
        BDBG_OBJECT_ASSERT(hVdc->ahDisplay[i]->hCompositor, BVDC_CMP);

        /* Wait for compositor/display to be applied/destroyed. */
        if((BVDC_P_STATE_IS_ACTIVE(hVdc->ahDisplay[i]) &&
            BVDC_P_STATE_IS_ACTIVE(hVdc->ahDisplay[i]->hCompositor) &&
            hVdc->ahDisplay[i]->bSetEventPending))
        {
            BDBG_MSG(("Waiting for Display%d to be applied", hVdc->ahDisplay[i]->eId));
            eStatus = BKNI_WaitForEvent(hVdc->ahDisplay[i]->hAppliedDoneEvent,
                BVDC_P_MAX_VEC_APPLY_WAIT_TIMEOUT);
            if(BERR_TIMEOUT == eStatus)
            {
                BDBG_ERR(("Display%d apply times out", hVdc->ahDisplay[i]->eId));
                return BERR_TRACE(eStatus);
            }
        }
#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY0
        if(hVdc->ahDisplay[i]->ulHdmiPwrRelease)
        {
            BDBG_MSG(("HDMI: release BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY"));
            BCHP_PWR_ReleaseResource(hVdc->hChip, hVdc->ahDisplay[i]->ulHdmiPwrId);
            hVdc->ahDisplay[i]->ulHdmiPwrRelease = 0;
        }
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_STG0
        if(hVdc->ahDisplay[i]->ulStgPwrRelease)
        {
            BDBG_MSG(("STG: release BCHP_PWR_RESOURCE_VDC_STG"));
            BCHP_PWR_ReleaseResource(hVdc->hChip, hVdc->ahDisplay[i]->ulStgPwrId);
            hVdc->ahDisplay[i]->ulStgPwrRelease = 0;
        }
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_656_OUT
        if(hVdc->ahDisplay[i]->ul656PwrRelease)
        {
            BDBG_MSG(("656: release BCHP_PWR_RESOURCE_VDC_656_OUT"));
            BCHP_PWR_ReleaseResource(hVdc->hChip, BCHP_PWR_RESOURCE_VDC_656_OUT);
            hVdc->ahDisplay[i]->ul656PwrRelease = 0;
        }
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_DAC
        if(hVdc->ahDisplay[i]->stAnlgChan_0.ulDacPwrAcquire == 0 &&
           hVdc->ahDisplay[i]->stAnlgChan_1.ulDacPwrAcquire == 0 &&
           (hVdc->ahDisplay[i]->stAnlgChan_0.ulDacPwrRelease ||
            hVdc->ahDisplay[i]->stAnlgChan_1.ulDacPwrRelease))
        {
            BDBG_MSG(("DAC: release BCHP_PWR_RESOURCE_VDC_DAC"));
            BCHP_PWR_ReleaseResource(hVdc->hChip, BCHP_PWR_RESOURCE_VDC_DAC);
            hVdc->ahDisplay[i]->ulDacPwrRelease = 0;
            hVdc->ahDisplay[i]->ulDacPwrAcquire = 0;
            hVdc->ahDisplay[i]->stAnlgChan_0.ulDacPwrRelease = 0;
            hVdc->ahDisplay[i]->stAnlgChan_1.ulDacPwrRelease = 0;
        }
#endif
#if BVDC_P_SUPPORT_VIP
        if(hVdc->ahDisplay[i]->hVip && !hVdc->ahDisplay[i]->stCurInfo.hVipHeap)
        {
            BVDC_P_Vip_FreeBuffer(hVdc->ahDisplay[i]->hVip);
        }
#endif

        /* Wait for window to applied/destroy */
        for(j = 0; j < BVDC_P_MAX_WINDOW_COUNT; j++)
        {
            if((BVDC_P_STATE_IS_SHUTDOWNPENDING(hVdc->ahDisplay[i]->hCompositor->ahWindow[j]) ||
                BVDC_P_STATE_IS_SHUTDOWNRUL(hVdc->ahDisplay[i]->hCompositor->ahWindow[j]) ||
                BVDC_P_STATE_IS_SHUTDOWN(hVdc->ahDisplay[i]->hCompositor->ahWindow[j]) ||
                BVDC_P_STATE_IS_INACTIVE(hVdc->ahDisplay[i]->hCompositor->ahWindow[j])) &&
               (hVdc->ahDisplay[i]->hCompositor->ahWindow[j]->bSetDestroyEventPending))
            {
                BVDC_Window_Handle hWindow = hVdc->ahDisplay[i]->hCompositor->ahWindow[j];
                BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
                BDBG_MSG(("Waiting for window%d to be destroyed", hWindow->eId));
                eStatus = BKNI_WaitForEvent(hWindow->hDestroyDoneEvent,
                    BVDC_P_MAX_DESTROY_WAIT_TIMEOUT);
                if(BERR_TIMEOUT == eStatus)
                {
#if (BDBG_DEBUG_BUILD)
                    const BVDC_P_Window_DirtyBits  *pDirty = &(hWindow->stCurInfo.stDirty);
                    const BVDC_P_VnetMode  *pVntMd = &(hWindow->stVnetMode);
                    BVDC_P_CheckDisplayAlignAdjustedStatus(hVdc);
                    BDBG_ERR(("Window%d (Src%d) destroy times out", hWindow->eId,
                        hWindow->stCurInfo.hSource->eId));
                    BDBG_ERR(("VnetMode 0x%08lx, readerState %d, writerState %d",
                        *(long unsigned int *) pVntMd, hWindow->stCurInfo.eReaderState, hWindow->stCurInfo.eWriterState));
                    BDBG_ERR(("   bShutDown %d, bRecfgVnet %d, bSrcPending %d, dirty 0x%08lx",
                        pDirty->stBits.bShutdown, pDirty->stBits.bReConfigVnet, pDirty->stBits.bSrcPending,
                        (long unsigned int)pDirty->aulInts[0]));
#endif
                    return BERR_TRACE(eStatus);
                }
            }
            else if((BVDC_P_STATE_IS_ACTIVE(hVdc->ahDisplay[i]->hCompositor->ahWindow[j]) &&
                    hVdc->ahDisplay[i]->hCompositor->ahWindow[j]->bSetAppliedEventPending) && bSynchronous)
            {
                BVDC_Window_Handle hWindow = hVdc->ahDisplay[i]->hCompositor->ahWindow[j];
                BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
                BDBG_MSG(("Waiting for window%d to be applied", hWindow->eId));
                eStatus = BKNI_WaitForEvent(hWindow->hAppliedDoneEvent,
                    BVDC_P_MAX_APPLY_WAIT_TIMEOUT);
                if(BERR_TIMEOUT == eStatus)
                {
#if (BDBG_DEBUG_BUILD)
                    const BVDC_P_Window_DirtyBits *pDirty = &(hWindow->stCurInfo.stDirty);
                    const BVDC_P_VnetMode  *pVntMd = &(hWindow->stVnetMode);
                    BVDC_P_CheckDisplayAlignAdjustedStatus(hVdc);
                    BDBG_ERR(("Window%d (Src%d) apply times out", hWindow->eId,
                        hWindow->stCurInfo.hSource->eId));
                    BDBG_ERR(("VnetMode 0x%08lx, readerState %d, writerState %d",
                        *(long unsigned int *) pVntMd, hWindow->stCurInfo.eReaderState, hWindow->stCurInfo.eWriterState));
                    BDBG_ERR(("   bShutDown %d, bRecfgVnet %d, bSrcPending %d, dirty 0x%08lx",
                        pDirty->stBits.bShutdown, pDirty->stBits.bReConfigVnet, pDirty->stBits.bSrcPending, (long unsigned int)pDirty->aulInts[0]));
#endif
                    return BERR_TRACE(eStatus);
                }
            }
        }
    }

    for(i = 0; i < BVDC_P_MAX_SOURCE_COUNT; i++)
    {
        if((BVDC_P_STATE_IS_ACTIVE(hVdc->ahSource[i]) &&
            hVdc->ahSource[i]->bUserAppliedChanges) && bSynchronous)
        {
            BDBG_OBJECT_ASSERT(hVdc->ahSource[i], BVDC_SRC);
            BDBG_MSG(("Waiting for Source%d to be applied", hVdc->ahSource[i]->eId));
            eStatus = BKNI_WaitForEvent(hVdc->ahSource[i]->hAppliedDoneEvent,
                BVDC_P_MAX_APPLY_WAIT_TIMEOUT);
            if(BERR_TIMEOUT == eStatus)
            {
#if (BDBG_DEBUG_BUILD)
                BVDC_P_CheckDisplayAlignAdjustedStatus(hVdc);
                BDBG_ERR(("Source%d apply times out", hVdc->ahSource[i]->eId));
#endif
                return BERR_TRACE(eStatus);
            }
        }

#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_RX_CLK0
        if((hVdc->ahSource[i]) &&
           (hVdc->ahSource[i]->ulHdmiPwrRelease))
        {
            BDBG_MSG(("SRC[%d]: release BCHP_PWR_RESOURCE_VDC_HDMI_RX_CLK = 0x%08x",
                i, hVdc->ahSource[i]->ulHdmiPwrId));
            BCHP_PWR_ReleaseResource(hVdc->hChip, hVdc->ahSource[i]->ulHdmiPwrId);
            hVdc->ahSource[i]->ulHdmiPwrRelease = 0;
        }
#endif
    }

    BDBG_LEAVE(BVDC_P_CheckApplyChangesStatus);
    return eStatus;
}


/***************************************************************************
 * BVDC_AbortChanges
 *
 */
BERR_Code BVDC_AbortChanges
    ( BVDC_Handle                      hVdc )
{
    uint32_t i;

    BDBG_ENTER(BVDC_AbortChanges);
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    for(i = 0; i < BVDC_P_MAX_SOURCE_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(hVdc->ahSource[i]) ||
           BVDC_P_STATE_IS_CREATE(hVdc->ahSource[i]) ||
           BVDC_P_STATE_IS_DESTROY(hVdc->ahSource[i]))
        {
            BVDC_P_Source_AbortChanges(hVdc->ahSource[i]);
        }
    }

    for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
    {
        if((BVDC_P_STATE_IS_ACTIVE(hVdc->ahCompositor[i]) &&
            BVDC_P_STATE_IS_ACTIVE(hVdc->ahDisplay[i])) ||
           (BVDC_P_STATE_IS_CREATE(hVdc->ahCompositor[i]) &&
            BVDC_P_STATE_IS_CREATE(hVdc->ahDisplay[i])) ||
           (BVDC_P_STATE_IS_DESTROY(hVdc->ahCompositor[i]) &&
            BVDC_P_STATE_IS_DESTROY(hVdc->ahDisplay[i])))
        {
            BVDC_P_Display_AbortChanges(hVdc->ahDisplay[i]);
            BVDC_P_Compositor_AbortChanges(hVdc->ahCompositor[i]);
        }
    }

    BDBG_LEAVE(BVDC_AbortChanges);
    return BERR_SUCCESS;
}


/***************************************************************************
 * BVDC_ApplyChanges
 *
 * Validate/Apply User's new changes.  This function will not result in
 * RUL building, but rather taken the new changes.  The ISR will be
 * responsible for building the RUL.
 */
BERR_Code BVDC_ApplyChanges
    ( BVDC_Handle                      hVdc )
{
    uint32_t i;
    BERR_Code eStatus = BERR_SUCCESS;

    BDBG_ENTER(BVDC_ApplyChanges);
    BDBG_OBJECT_ASSERT(hVdc, BVDC_VDC);

    BDBG_MSG(("-------------ApplyChanges(%d)------------", hVdc->ulApplyCnt++));

    hVdc->ulXcodeWinCap = 0;

    /* +------------------------------------- +
     * | WAIT FOR CHANGES APPLIED (next vsync)|
     * +--------------------------------------+
     * After the changes are put in RUL in _isr it will set event to notify
     * that changes will be hardware on next trigger. */
    eStatus = BERR_TRACE(BVDC_P_CheckApplyChangesStatus(hVdc, true));
    if(BERR_SUCCESS != eStatus)
    {
        return BERR_TRACE(eStatus);
    }

    /* +------------------+
     * | VALIDATE CHANGES |
     * +------------------+
     * User's new settings reject if we have bad new setting with
     * approriate error status.  No new settings will be used.
     * Frontend   Things that are going to cause failures should be caught
     * here.   Other failures may able to detect early in the set function
     * where error checking is not depended on other settings.   Our build
     * RULs and ApplyChanges should not failed anymore. */
    eStatus = BERR_TRACE(BVDC_P_Source_ValidateChanges(hVdc->ahSource));
    if(BERR_SUCCESS != eStatus)
    {
        return BERR_TRACE(eStatus);
    }

    eStatus = BERR_TRACE(BVDC_P_Display_ValidateChanges(hVdc->ahDisplay));
    if(BERR_SUCCESS != eStatus)
    {
        return BERR_TRACE(eStatus);
    }

    eStatus = BERR_TRACE(BVDC_P_Compositor_ValidateChanges(hVdc->ahCompositor));
    if(BERR_SUCCESS != eStatus)
    {
        return BERR_TRACE(eStatus);
    }

    /* +-------------- +
     * | APPLY CHANGES |
     * +---------------+
     * Apply user's new settings it will be include in the next RUL building*/
    BKNI_EnterCriticalSection();
    BVDC_P_CHECK_CS_ENTER_VDC(hVdc);

    for(i = 0; i < BVDC_P_MAX_SOURCE_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(hVdc->ahSource[i]) ||
           BVDC_P_STATE_IS_CREATE(hVdc->ahSource[i]) ||
           BVDC_P_STATE_IS_DESTROY(hVdc->ahSource[i]))
        {
            BVDC_P_Source_ApplyChanges_isr(hVdc->ahSource[i]);
        }
    }

    BVDC_P_Vec_Update_OutMuxes_isr(hVdc);

    /* Note, since display has id now, which might be different from compositor
       id, we need to loop them separately in case cmp/win logic depends on its
       display's context changes; */
    for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(hVdc->ahDisplay[i]) ||
           BVDC_P_STATE_IS_CREATE(hVdc->ahDisplay[i]) ||
           BVDC_P_STATE_IS_DESTROY(hVdc->ahDisplay[i]))
        {
            BVDC_P_Display_ApplyChanges_isr(hVdc->ahDisplay[i]);
        }
    }
    for(i = 0; i < BVDC_P_MAX_COMPOSITOR_COUNT; i++)
    {
        if(BVDC_P_STATE_IS_ACTIVE(hVdc->ahCompositor[i]) ||
           BVDC_P_STATE_IS_CREATE(hVdc->ahCompositor[i]) ||
           BVDC_P_STATE_IS_DESTROY(hVdc->ahCompositor[i]))
        {
            BVDC_P_Compositor_ApplyChanges_isr(hVdc->ahCompositor[i]);
        }
    }

    BVDC_P_CHECK_CS_LEAVE_VDC(hVdc);
    BKNI_LeaveCriticalSection();

    /* +------------------------------------- +
     * | WAIT FOR CHANGES APPLIED (next vsync)|
     * +--------------------------------------+
     * After the changes are put in RUL in _isr it will set event to notify
     * that changes will be hardware on next trigger. */
#ifndef BVDC_FOR_BOOTUPDATER
    eStatus = BERR_TRACE(BVDC_P_CheckApplyChangesStatus(hVdc, false));
    if(BERR_SUCCESS != eStatus)
    {
        return BERR_TRACE(eStatus);
    }
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

    BDBG_LEAVE(BVDC_ApplyChanges);
    return BERR_SUCCESS;
}

/* End of File */
