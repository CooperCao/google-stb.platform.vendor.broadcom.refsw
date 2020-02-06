/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bvdc.h"
#include "brdc.h"
#include "bmth.h"
#include "bvdc_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_source_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_capture_priv.h"
#include "bvdc_scaler_priv.h"
#include "bvdc_xsrc_priv.h"
#include "bvdc_vfc_priv.h"
#include "bvdc_hscaler_priv.h"
#include "bvdc_dnr_priv.h"
#include "bvdc_vnetcrc_priv.h"
#include "bvdc_feeder_priv.h"
#include "bvdc_vnet_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_hddvi_priv.h"
#include "bvdc_gfxfeeder_priv.h"
#include "bvdc_boxdetect_priv.h"
#include "bvdc_bufferheap_priv.h"
#include "bvdc_pep_priv.h"
#include "bvdc_656in_priv.h"
#include "bvdc_anr_priv.h"
#include "bvdc_mcvp_priv.h"
#include "bvdc_mcdi_priv.h"
#include "bvdc_tnt_priv.h"
#include "bvdc_tntd_priv.h"
#include "bvdc_adjrect_priv.h"

BDBG_MODULE(BVDC_WIN);
BDBG_FILE_MODULE(BVDC_TIMESTAMP);
BDBG_FILE_MODULE(BVDC_DCXM_PICSIZE);
BDBG_FILE_MODULE(BVDC_MADR_PICSIZE);
BDBG_FILE_MODULE(BVDC_ASP_RAT);
BDBG_FILE_MODULE(BVDC_WIN_VNET);
BDBG_FILE_MODULE(BVDC_WIN_BUF);
BDBG_FILE_MODULE(BVDC_WIN_BUF_SIZE);
BDBG_FILE_MODULE(BVDC_BAR);
BDBG_FILE_MODULE(BVDC_WIN_COV);
BDBG_FILE_MODULE(BVDC_REPEATPOLARITY);
BDBG_FILE_MODULE(BVDC_DEINTERLACER_MOSAIC);
BDBG_FILE_MODULE(BVDC_MTGW);
BDBG_FILE_MODULE(BVDC_MTGR);
BDBG_FILE_MODULE(BVDC_MTGR__);
BDBG_FILE_MODULE(BVDC_CFC_2); /* print CFC configure */
BDBG_FILE_MODULE(BVDC_CFC_5); /* print flooded msgs */
BDBG_OBJECT_ID(BVDC_WIN);

/* SW3556-886 */
#define BVDC_FORCE_VFD_TOP_CLIP            BVDC_P_VDEC_SECAM_TTD_BYPASS_FILTER_IN_VBI

/* SW7405-3068 */
#define BVDC_P_WIN_MOSAIC_MODE_BVN_WORKAROUND    (1)


/* Short hand for make resoure! */
#define BVDC_P_MAKE_RES(win_id, cap, vfd, scl, mad, pep,                    \
    cap_id, vfd_id, scl_id, mad_id)                                         \
{                                                                           \
    cap, vfd, scl, mad, pep,                                                \
    BVDC_P_CaptureId_e##cap_id,                                             \
    BVDC_P_FeederId_e##vfd_id,                                              \
    BVDC_P_ScalerId_e##scl_id,                                              \
    BVDC_P_MadId_e##mad_id,                                                 \
    BVDC_P_WindowId_e##win_id,                                              \
}

#ifndef BVDC_FOR_BOOTUPDATER
/* INDEX: by BVDC_P_WindowId. */
static const BVDC_P_ResourceRequire s_aResourceRequireTable[] =
{
    /*              win_id    c  f  s  m  p  ids... */
#if (BCHP_CHIP==7358) || (BCHP_CHIP==7552) || \
    (BCHP_CHIP==7360) || (BCHP_CHIP==7255) || \
    (BCHP_CHIP==7563) || (BCHP_CHIP==7543) || (BCHP_CHIP==7362) || \
    (BCHP_CHIP==7228) || (BCHP_CHIP==75635) || (BCHP_CHIP==73625)
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 1, Cap0, Vfd0, Scl0, Unknown),
    BVDC_P_MAKE_RES(Comp0_V1, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp1_V0, 1, 1, 1, 0, 0, Cap1, Vfd1, Scl1, Unknown),
    BVDC_P_MAKE_RES(Comp1_V1, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp2_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp3_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp4_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
#elif (BCHP_CHIP==7211)
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 0, Cap0, Vfd0, Scl0, Unknown),
    BVDC_P_MAKE_RES(Comp0_V1, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp1_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp1_V1, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp2_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp3_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp4_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
#elif (BCHP_CHIP==7364) || (BCHP_CHIP==7250) || (BCHP_CHIP==7271) || \
      (BCHP_CHIP==7268) || (BCHP_CHIP==7260)
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 1, Cap0, Vfd0, Scl0, Unknown),
    BVDC_P_MAKE_RES(Comp0_V1, 1, 1, 0, 0, 0, Cap1, Vfd1, Scl1, Unknown), /* shared w/ Comp1_V0 */
    BVDC_P_MAKE_RES(Comp1_V0, 1, 1, 0, 0, 0, Cap1, Vfd1, Scl1, Unknown), /* shared w/ Comp0_V1 */
    BVDC_P_MAKE_RES(Comp1_V1, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp2_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp3_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp4_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),

#elif (BCHP_CHIP==7231)  || (BCHP_CHIP==7344)  || (BCHP_CHIP==7346) || \
      (BCHP_CHIP==7429)  || (BCHP_CHIP==7584)  || (BCHP_CHIP==7586) || \
      (BCHP_CHIP==75845) || (BCHP_CHIP==74295) || (BCHP_CHIP==73465)
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 1, Cap0, Vfd0, Scl0, Unknown),
#if (BVDC_SUPPORT_DUAL_HD)
    BVDC_P_MAKE_RES(Comp0_V1, 1, 1, 1, 0, 0, Cap2, Vfd2, Scl2, Unknown),
    BVDC_P_MAKE_RES(Comp1_V0, 1, 1, 1, 0, 0, Cap1, Vfd1, Scl1, Unknown),
#else
    BVDC_P_MAKE_RES(Comp0_V1, 1, 1, 1, 0, 0, Cap1, Vfd1, Scl1, Unknown),
    BVDC_P_MAKE_RES(Comp1_V0, 1, 1, 1, 0, 0, Cap2, Vfd2, Scl2, Unknown),
#endif
    BVDC_P_MAKE_RES(Comp1_V1, 1, 1, 1, 0, 0, Cap3, Vfd3, Scl3, Unknown),
    BVDC_P_MAKE_RES(Comp2_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp3_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp4_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),

#elif ((BCHP_CHIP==7439) && (BCHP_VER==BCHP_VER_B0))
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 1, Cap0, Vfd0, Scl0, Unknown),
    BVDC_P_MAKE_RES(Comp0_V1, 1, 1, 0, 0, 0, Cap1, Vfd1, Scl1, Unknown),
    BVDC_P_MAKE_RES(Comp1_V0, 1, 1, 1, 0, 0, Cap2, Vfd2, Scl2, Unknown),
    BVDC_P_MAKE_RES(Comp1_V1, 1, 1, 1, 0, 0, Cap3, Vfd3, Scl3, Unknown),
    BVDC_P_MAKE_RES(Comp2_V0, 1, 1, 1, 0, 0, Cap4, Vfd4, Scl4, Unknown),
    BVDC_P_MAKE_RES(Comp3_V0, 1, 1, 1, 0, 0, Cap5, Vfd5, Scl5, Unknown),
    BVDC_P_MAKE_RES(Comp4_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),

#elif (BCHP_CHIP==7366) || ((BCHP_CHIP==7439)&& (BCHP_VER==BCHP_VER_A0))|| \
      (BCHP_CHIP==74371)
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 1, Cap0,    Vfd0,    Scl0,    Unknown),
    BVDC_P_MAKE_RES(Comp0_V1, 1, 1, 0, 0, 0, Cap1,    Vfd1,    Scl1,    Unknown), /* shared w/ Comp2_V0 */
    BVDC_P_MAKE_RES(Comp1_V0, 1, 1, 1, 0, 0, Cap2,    Vfd2,    Scl2,    Unknown),
    BVDC_P_MAKE_RES(Comp1_V1, 1, 1, 1, 0, 0, Cap3,    Vfd3,    Scl3,    Unknown),
    BVDC_P_MAKE_RES(Comp2_V0, 1, 1, 0, 0, 0, Cap1,    Vfd1,    Scl1,    Unknown), /* shared w/ Comp0_V1 */
    BVDC_P_MAKE_RES(Comp3_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp4_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),

#elif (BCHP_CHIP==7422) || (BCHP_CHIP==7425)
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 1, Cap0, Vfd0, Scl0, Unknown),
    BVDC_P_MAKE_RES(Comp0_V1, 1, 1, 0, 0, 0, Cap1, Vfd1, Scl1, Unknown),    /* shared w/ Comp2_V0 */
    BVDC_P_MAKE_RES(Comp1_V0, 1, 1, 1, 0, 0, Cap2, Vfd2, Scl2, Unknown),
    BVDC_P_MAKE_RES(Comp1_V1, 1, 1, 1, 0, 0, Cap3, Vfd3, Scl3, Unknown),
    BVDC_P_MAKE_RES(Comp2_V0, 1, 1, 0, 0, 0, Cap1, Vfd1, Scl1, Unknown),    /* shared w/ Comp0_V1 */
    BVDC_P_MAKE_RES(Comp3_V0, 1, 1, 1, 0, 0, Cap4, Vfd4, Scl4, Unknown),
    BVDC_P_MAKE_RES(Comp4_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),

#elif (BCHP_CHIP==7435)
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 1, Cap0, Vfd0, Scl0,    Unknown),
    BVDC_P_MAKE_RES(Comp0_V1, 1, 1, 1, 0, 0, Cap1, Vfd1, Scl1,    Unknown), /* shared w/ Comp2_V0 */
    BVDC_P_MAKE_RES(Comp1_V0, 1, 1, 1, 0, 0, Cap2, Vfd2, Scl2,    Unknown),
    BVDC_P_MAKE_RES(Comp1_V1, 1, 1, 0, 0, 0, Cap3, Vfd3, Unknown, Unknown), /* shared w/ Comp3_V0 */
    BVDC_P_MAKE_RES(Comp2_V0, 1, 1, 1, 0, 0, Cap1, Vfd1, Scl1,    Unknown), /* shared w/ Comp0_V1 */
    BVDC_P_MAKE_RES(Comp3_V0, 1, 1, 0, 0, 0, Cap3, Vfd3, Unknown, Unknown), /* shared w/ Comp1_V1 */
    BVDC_P_MAKE_RES(Comp4_V0, 1, 1, 1, 0, 0, Cap4, Vfd4, Scl4,    Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 1, 1, 1, 0, 0, Cap5, Vfd5, Scl5,    Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),

#elif (BCHP_CHIP==7445) || (BCHP_CHIP==11360)
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 1, Cap0, Vfd0, Scl0,    Unknown),
    BVDC_P_MAKE_RES(Comp0_V1, 1, 1, 0, 0, 0, Cap1, Vfd1, Scl1,    Unknown), /* shared w/ Comp2_V0 */
    BVDC_P_MAKE_RES(Comp1_V0, 1, 1, 1, 0, 0, Cap2, Vfd2, Scl2,    Unknown),
    BVDC_P_MAKE_RES(Comp1_V1, 1, 1, 1, 0, 0, Cap3, Vfd3, Scl3,    Unknown),
    BVDC_P_MAKE_RES(Comp2_V0, 1, 1, 0, 0, 0, Cap1, Vfd1, Scl1,    Unknown), /* shared w/ Comp0_V1 */
    BVDC_P_MAKE_RES(Comp3_V0, 1, 1, 1, 0, 0, Cap4, Vfd4, Scl4,    Unknown),
    BVDC_P_MAKE_RES(Comp4_V0, 1, 1, 1, 0, 0, Cap5, Vfd5, Scl5,    Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 1, 1, 1, 0, 0, Cap6, Vfd6, Scl6,    Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 1, 1, 1, 0, 0, Cap7, Vfd7, Scl7,    Unknown),

#elif (BCHP_CHIP==7278)
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 1, Cap0, Vfd0, Scl0, Unknown),
    BVDC_P_MAKE_RES(Comp0_V1, 1, 1, 1, 0, 0, Cap1, Vfd1, Scl1, Unknown),    /* shared w/ Comp2_V0 */
    BVDC_P_MAKE_RES(Comp1_V0, 1, 1, 1, 0, 0, Cap2, Vfd2, Scl2, Unknown),
    BVDC_P_MAKE_RES(Comp1_V1, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp2_V0, 1, 1, 1, 0, 0, Cap1, Vfd1, Scl1, Unknown),    /* shared w/ Comp0_V1 */
    BVDC_P_MAKE_RES(Comp3_V0, 1, 1, 1, 0, 0, Cap3, Vfd3, Scl3, Unknown),
    BVDC_P_MAKE_RES(Comp4_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),

#elif (BCHP_CHIP==7216)
    BVDC_P_MAKE_RES(Comp0_V0, 1, 1, 1, 0, 1, Cap0, Vfd0, Scl0, Unknown),
    BVDC_P_MAKE_RES(Comp0_V1, 1, 1, 1, 0, 0, Cap1, Vfd1, Scl1, Unknown),
    BVDC_P_MAKE_RES(Comp1_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp1_V1, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp2_V0, 1, 1, 1, 0, 0, Cap1, Vfd1, Scl2, Unknown),
    BVDC_P_MAKE_RES(Comp3_V0, 1, 1, 1, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp4_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp5_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp6_V0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),

#else
#error "Unknown chip!  Not yet supported in VDC."
#endif

    BVDC_P_MAKE_RES(Comp0_G0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp0_G1, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp0_G2, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp1_G0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp2_G0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp3_G0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp4_G0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp5_G0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
    BVDC_P_MAKE_RES(Comp6_G0, 0, 0, 0, 0, 0, Unknown, Unknown, Unknown, Unknown),
};
#endif

#define FTR_SD     (BVDC_P_Able_eSd)
#define FTR_HD     (BVDC_P_Able_eHd)
#define FTR_M0     (BVDC_P_Able_eMem0)
#define FTR_M1     (BVDC_P_Able_eMem1)
#define FTR_M2     (BVDC_P_Able_eMem2)
#define FTR_M01    (BVDC_P_Able_eMem0 | BVDC_P_Able_eMem1)
#define FTR_HD_M0  (BVDC_P_Able_eMem0 | BVDC_P_Able_eHd)
#define FTR_HD_M1  (BVDC_P_Able_eMem1 | BVDC_P_Able_eHd)
#define FTR_HD_M2  (BVDC_P_Able_eMem2 | BVDC_P_Able_eHd)
#define FTR_HD_MR0 (BVDC_P_Able_eHd | BVDC_P_Able_eMadr0)
#define FTR_HD_MR1 (BVDC_P_Able_eHd | BVDC_P_Able_eMadr1)
#define FTR_HD_MR2 (BVDC_P_Able_eHd | BVDC_P_Able_eMadr2)
#define FTR_HD_MR3 (BVDC_P_Able_eHd | BVDC_P_Able_eMadr3)
#define FTR_HD_MR4 (BVDC_P_Able_eHd | BVDC_P_Able_eMadr4)
#define FTR_HD_MR5 (BVDC_P_Able_eHd | BVDC_P_Able_eMadr5)

/* this will cause acquire to fail */
#define FTR___     (BVDC_P_Able_eInvalid)

#ifndef BVDC_FOR_BOOTUPDATER
/* INDEX: by BVDC_P_WindowId. */
static const BVDC_P_ResourceFeature s_aResourceFeatureTable[] =
{
#if (BCHP_CHIP==7358) || (BCHP_CHIP==7552) || (BCHP_CHIP==7360) || \
    (BCHP_CHIP==7362) || (BCHP_CHIP==7228) || (BCHP_CHIP==73625)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD, FTR___ },
    /*Comp0_V1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD, FTR___ },
    /*Comp1_V1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp2_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp3_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },

#elif (BCHP_CHIP==7563) || (BCHP_CHIP==7543) || (BCHP_CHIP==75635)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD, FTR___ },
    /*Comp0_V1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD, FTR___ },
    /*Comp1_V1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp2_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp3_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },

#elif ((BCHP_CHIP==7366) && (BCHP_VER >= BCHP_VER_B0))
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD,     FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD,     FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_HD_MR1, FTR___ },
    /*Comp1_V1*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp2_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR0, FTR___ },
    /*Comp3_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },

#elif ((BCHP_CHIP==7439) && (BCHP_VER >= BCHP_VER_B0))
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD,     FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR0, FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp1_V1*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp2_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR1, FTR___ },
    /*Comp3_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR2, FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },

#elif (BCHP_CHIP==7366) || \
      ((BCHP_CHIP==7439)  && (BCHP_VER==BCHP_VER_A0))|| \
      ((BCHP_CHIP==74371) && (BCHP_VER==BCHP_VER_A0))
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD,     FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD,     FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp1_V1*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp2_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR0, FTR___ },
    /*Comp3_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },

#elif (BCHP_CHIP==7422) || (BCHP_CHIP==7425)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD,     FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD,     FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp1_V1*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp2_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR1, FTR___ },
    /*Comp3_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR0, FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },

#elif (BCHP_CHIP==7231)  || \
      (BCHP_CHIP==7344)  || (BCHP_CHIP==7346)  || (BCHP_CHIP==7429)  || \
      (BCHP_CHIP==7584)  || (BCHP_CHIP==7586)  || (BCHP_CHIP==75845) || \
      (BCHP_CHIP==74295) || (BCHP_CHIP==73465)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD, FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD, FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD, FTR___ },
    /*Comp1_V1*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD, FTR___ },
    /*Comp2_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD, FTR___ },
    /*Comp3_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },

#elif (BCHP_CHIP==7271) || (BCHP_CHIP==7268) || (BCHP_CHIP==7260)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD, FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD, FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD, FTR___ },
    /*Comp1_V1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp2_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp3_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },

#elif (BCHP_CHIP==7255)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD, FTR___ }, /* sharing with Comp1_V0 */
    /*Comp0_V1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD, FTR___ }, /* sharing with Comp0_V0 */
    /*Comp1_V1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp2_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp3_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },

#elif (BCHP_CHIP==7211)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR___, FTR___ }, /* sharing with Comp1_V0 */
    /*Comp0_V1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp1_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ }, /* sharing with Comp0_V0 */
    /*Comp1_V1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp2_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp3_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },

#elif (BCHP_CHIP==7364) || (BCHP_CHIP==7250)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD, FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD, FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD, FTR___ },
    /*Comp1_V1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp2_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp3_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
#elif (BCHP_CHIP==7435)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD,     FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_SD,     FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp1_V1*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp2_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR2, FTR___ },
    /*Comp3_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_HD_MR3, FTR___ },
    /*Comp4_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR1, FTR___ },
    /*Comp5_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR0, FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },

#elif (BCHP_CHIP==7445) || (BCHP_CHIP==11360)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD,     FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR0, FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp1_V1*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp2_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_HD_MR0, FTR___ },
    /*Comp3_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR1, FTR___ },
    /*Comp4_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR2, FTR___ },
    /*Comp5_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR3, FTR___ },
    /*Comp6_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR4, FTR___ },

#elif (BCHP_CHIP==7278)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD,     FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR0, FTR___ },
    /*Comp1_V0*/{ FTR_M0, FTR_M0, FTR_SD, FTR_SD,     FTR___ },
    /*Comp1_V1*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp2_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR1, FTR___ },
    /*Comp3_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR2, FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },

#elif (BCHP_CHIP==7216)
    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR0, FTR___ },
    /*Comp0_V1*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR1, FTR___ },
    /*Comp1_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp1_V1*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp2_V0*/{ FTR_M0, FTR_M0, FTR_HD, FTR_HD_MR1, FTR___ },
    /*Comp3_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp4_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp5_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },
    /*Comp6_V0*/{ FTR___, FTR___, FTR___, FTR___,     FTR___ },

#else
#error "Unknown chip!  Not yet supported in VDC."
#endif

    /*            ulCap;  ulVfd;  ulScl;  ulMad;  ulAnr; */
    /*Comp0_G0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp0_G1*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp0_G2*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp1_G0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp2_G0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp3_G0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp4_G0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp5_G0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ },
    /*Comp6_G0*/{ FTR___, FTR___, FTR___, FTR___, FTR___ }
};

#define BVDC_P_RESOURCE_TABLE_COUNT \
    (sizeof(s_aResourceRequireTable) / sizeof(BVDC_P_ResourceRequire))

#define BVDC_P_RESOURCE_FEATURE_TABLE_COUNT \
    (sizeof(s_aResourceFeatureTable) / sizeof(BVDC_P_ResourceFeature))
#endif

#if (BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT < 2)
#undef  BCHP_VNET_B_CMP_0_V1_SRC
#define BCHP_VNET_B_CMP_0_V1_SRC  0
#endif

#if (BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT < 1)
#undef  BCHP_VNET_B_CMP_1_V0_SRC
#define BCHP_VNET_B_CMP_1_V0_SRC  0
#endif

#if (BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT < 2)
#undef  BCHP_VNET_B_CMP_1_V1_SRC
#define BCHP_VNET_B_CMP_1_V1_SRC  0
#endif

#if (BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT < 1)
#undef  BCHP_VNET_B_CMP_2_V0_SRC
#define BCHP_VNET_B_CMP_2_V0_SRC  0
#else
#ifndef BCHP_VNET_B_CMP_2_V0_SRC
#define BCHP_VNET_B_CMP_2_V0_SRC  BCHP_VNET_B_BP_0_SRC
#endif
#endif

#if (BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT < 1)
#undef  BCHP_VNET_B_CMP_3_V0_SRC
#define BCHP_VNET_B_CMP_3_V0_SRC  0
#else
#ifndef BCHP_VNET_B_CMP_3_V0_SRC
#define BCHP_VNET_B_CMP_3_V0_SRC  BCHP_VNET_B_BP_0_SRC
#endif
#endif

#if (BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT < 1)
#undef  BCHP_VNET_B_CMP_4_V0_SRC
#define BCHP_VNET_B_CMP_4_V0_SRC  0
#else
#ifndef BCHP_VNET_B_CMP_4_V0_SRC
#define BCHP_VNET_B_CMP_4_V0_SRC  BCHP_VNET_B_BP_0_SRC
#endif
#endif

#if (BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT < 1)
#undef  BCHP_VNET_B_CMP_5_V0_SRC
#define BCHP_VNET_B_CMP_5_V0_SRC  0
#else
#ifndef BCHP_VNET_B_CMP_5_V0_SRC
#define BCHP_VNET_B_CMP_5_V0_SRC  BCHP_VNET_B_BP_0_SRC
#endif
#endif

#if (BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT < 1)
#undef  BCHP_VNET_B_CMP_6_V0_SRC
#define BCHP_VNET_B_CMP_6_V0_SRC  0
#else
#ifndef BCHP_VNET_B_CMP_6_V0_SRC
#define BCHP_VNET_B_CMP_6_V0_SRC  BCHP_VNET_B_BP_0_SRC
#endif
#endif

#ifndef BVDC_FOR_BOOTUPDATER
/* compositor's win src mux addr: index by eWinId */
static const uint32_t s_aulWinOutMuxAddr[] =
{
    BCHP_VNET_B_CMP_0_V0_SRC,  /* BVDC_P_WindowId_eComp0_V0 = 0 */
    BCHP_VNET_B_CMP_0_V1_SRC,  /* BVDC_P_WindowId_eComp0_V1 */
    BCHP_VNET_B_CMP_1_V0_SRC,  /* BVDC_P_WindowId_eComp1_V0 */
    BCHP_VNET_B_CMP_1_V1_SRC,  /* BVDC_P_WindowId_eComp1_V1 */
    BCHP_VNET_B_CMP_2_V0_SRC,  /* BVDC_P_WindowId_eComp2_V0 */
    BCHP_VNET_B_CMP_3_V0_SRC,  /* BVDC_P_WindowId_eComp3_V0 */
    BCHP_VNET_B_CMP_4_V0_SRC,  /* BVDC_P_WindowId_eComp4_V0 */
    BCHP_VNET_B_CMP_5_V0_SRC,  /* BVDC_P_WindowId_eComp5_V0 */
    BCHP_VNET_B_CMP_6_V0_SRC,  /* BVDC_P_WindowId_eComp6_V0 */
    0,                         /* BVDC_P_WindowId_eComp0_G0 */
    0,                         /* BVDC_P_WindowId_eComp0_G1 */
    0,                         /* BVDC_P_WindowId_eComp0_G2 */
    0,                         /* BVDC_P_WindowId_eComp1_G0 */
    0,                         /* BVDC_P_WindowId_eComp2_G0 */
    0,                         /* BVDC_P_WindowId_eComp3_G0 */
    0,                         /* BVDC_P_WindowId_eComp4_G0 */
    0,                         /* BVDC_P_WindowId_eComp5_G0 */
    0                          /* BVDC_P_WindowId_eComp6_G0 */
};

/* blender address tables */
static const uint32_t s_aulBlendAddr[] =
{
    BCHP_CMP_0_BLEND_0_CTRL,
    BCHP_CMP_0_BLEND_1_CTRL,
#if (BVDC_P_CMP_0_MAX_WINDOW_COUNT > 2)
    BCHP_CMP_0_BLEND_2_CTRL,
#if (BVDC_P_CMP_0_MAX_WINDOW_COUNT > 3)
    BCHP_CMP_0_BLEND_3_CTRL,
#if (BVDC_P_CMP_0_MAX_WINDOW_COUNT > 4)
    BCHP_CMP_0_BLEND_4_CTRL
#endif
#endif
#endif
};
#endif

/***************************************************************************
 * Window Id lookup table with compostior id, and source id.  This
 * only lookup window id for the first gfx or video window.
 ***************************************************************************/
typedef struct BVDC_P_Window_SelectedId
{
    bool bValid;
    BVDC_P_WindowId eWindowId;
} BVDC_P_Window_SelectedId;

#ifndef BVDC_FOR_BOOTUPDATER
/* INDEX: by BAVC_SourceId & BVDC_P_CompositorId. */
static const BVDC_P_Window_SelectedId s_aaWindowIdSelectTable
    [BVDC_P_MAX_COMPOSITOR_COUNT /* compositor id (0,1,bypass) */]
    [BVDC_P_MAX_SOURCE_COUNT     /* source (mpeg, gfx, etc)    */] =
{
    /* compositor 0: could take any video source or gfx0 */
    {{true,  BVDC_P_WindowId_eComp0_V0},  /* Mpeg0 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* Mpeg1 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* Mpeg2 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* Mpeg3 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* Mpeg4 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* Mpeg5 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* Vdec0 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* Vdec1 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* 656In0 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* 656In1 */
     {true,  BVDC_P_WindowId_eComp0_G0},  /* Gfx0 */
     {false, BVDC_P_WindowId_eComp0_G0},  /* Gfx1 */
     {false, BVDC_P_WindowId_eComp0_G0},  /* Gfx2 */
     {false, BVDC_P_WindowId_eComp0_G0},  /* Gfx3 */
     {false, BVDC_P_WindowId_eComp0_G0},  /* Gfx4 */
     {false, BVDC_P_WindowId_eComp0_G0},  /* Gfx5 */
     {false, BVDC_P_WindowId_eComp0_G0},  /* Gfx6 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* HdDvi0 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* HdDvi1 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* Ds0 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* Vfd0 */
     {true,  BVDC_P_WindowId_eComp0_V0},  /* Vfd1 */
     {false, BVDC_P_WindowId_eComp0_V0},  /* Vfd2 */
     {false, BVDC_P_WindowId_eComp0_V0},  /* Vfd3 */
     {false, BVDC_P_WindowId_eComp0_V0},  /* Vfd4 */
     {false, BVDC_P_WindowId_eComp0_V0},  /* Vfd5 */
     {false, BVDC_P_WindowId_eComp0_V0},  /* Vfd6 */
     {false, BVDC_P_WindowId_eComp0_V0}}, /* Vfd7 */

    /* compositor 1: could take any video source or gfx1 */
    {{true,  BVDC_P_WindowId_eComp1_V0},  /* Mpeg0 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* Mpeg1 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* Mpeg2 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* Mpeg3 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* Mpeg4 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* Mpeg5 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* Vdec0 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* Vdec1 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* 656In0 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* 656In1 */
     {false, BVDC_P_WindowId_eComp1_G0},  /* Gfx0 */
     {true,  BVDC_P_WindowId_eComp1_G0},  /* Gfx1 */
     {false, BVDC_P_WindowId_eComp1_G0},  /* Gfx2 */
     {false, BVDC_P_WindowId_eComp1_G0},  /* Gfx3 */
     {false, BVDC_P_WindowId_eComp1_G0},  /* Gfx4 */
     {false, BVDC_P_WindowId_eComp1_G0},  /* Gfx5 */
     {false, BVDC_P_WindowId_eComp1_G0},  /* Gfx6 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* HdDvi0 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* HdDvi1 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* Ds0 */
     {false, BVDC_P_WindowId_eComp1_V0},  /* Vfd0 */
     {false, BVDC_P_WindowId_eComp1_V0},  /* Vfd1 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* Vfd2 */
     {true,  BVDC_P_WindowId_eComp1_V0},  /* Vfd3 */
     {false, BVDC_P_WindowId_eComp1_V0},  /* Vfd4 */
     {false, BVDC_P_WindowId_eComp1_V0},  /* Vfd5 */
     {false, BVDC_P_WindowId_eComp1_V0},  /* Vfd6 */
     {false, BVDC_P_WindowId_eComp1_V0}}, /* Vfd7 */

    /* bypass 0: intended for analog PVR; enforce analog video source */
    {{true,  BVDC_P_WindowId_eComp2_V0},  /* Mpeg0 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Mpeg1 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Mpeg2 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Mpeg3 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Mpeg4 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Mpeg5 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Vdec0 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Vdec1 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* 656In0 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* 656In1 */
     {false, BVDC_P_WindowId_eComp2_G0},  /* Gfx0 */
     {false, BVDC_P_WindowId_eComp2_G0},  /* Gfx1 */
     {true,  BVDC_P_WindowId_eComp2_G0},  /* Gfx2 */
     {false, BVDC_P_WindowId_eComp2_G0},  /* Gfx3 */
     {false, BVDC_P_WindowId_eComp2_G0},  /* Gfx4 */
     {false, BVDC_P_WindowId_eComp2_G0},  /* Gfx5 */
     {false, BVDC_P_WindowId_eComp2_G0},  /* Gfx6 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* HdDvi0 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* HdDvi1 */
     {false, BVDC_P_WindowId_eComp2_V0},  /* Ds0 */
     {false, BVDC_P_WindowId_eComp2_V0},  /* Vfd0 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Vfd1 */
     {false, BVDC_P_WindowId_eComp2_V0},  /* Vfd2 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Vfd3 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Vfd4 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Vfd5 */
     {true,  BVDC_P_WindowId_eComp2_V0},  /* Vfd6 */
     {true,  BVDC_P_WindowId_eComp2_V0}}, /* Vfd7 */

    /* VICE */
    {{true,  BVDC_P_WindowId_eComp3_V0},  /* Mpeg0 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* Mpeg1 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* Mpeg2 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* Mpeg3 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* Mpeg4 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* Mpeg5 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* Vdec0 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* Vdec1 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* 656In0 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* 656In1 */
     {false, BVDC_P_WindowId_eComp3_G0},  /* Gfx0 */
     {false, BVDC_P_WindowId_eComp3_G0},  /* Gfx1 */
     {false, BVDC_P_WindowId_eComp3_G0},  /* Gfx2 */
     {true,  BVDC_P_WindowId_eComp3_G0},  /* Gfx3 */
     {false, BVDC_P_WindowId_eComp3_G0},  /* Gfx4 */
     {false, BVDC_P_WindowId_eComp3_G0},  /* Gfx5 */
     {false, BVDC_P_WindowId_eComp3_G0},  /* Gfx6 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* HdDvi0 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* HdDvi1 */
     {false, BVDC_P_WindowId_eComp3_V0},  /* Ds0 */
     {false, BVDC_P_WindowId_eComp3_V0},  /* Vfd0 */
     {false, BVDC_P_WindowId_eComp3_V0},  /* Vfd1 */
     {false, BVDC_P_WindowId_eComp3_V0},  /* Vfd2 */
     {false, BVDC_P_WindowId_eComp3_V0},  /* Vfd3 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* Vfd4 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* Vfd5 */
     {true,  BVDC_P_WindowId_eComp3_V0},  /* Vfd6 */
     {true,  BVDC_P_WindowId_eComp3_V0}}, /* Vfd7 */

    /* VICE */
    {{true,  BVDC_P_WindowId_eComp4_V0},  /* Mpeg0 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* Mpeg1 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* Mpeg2 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* Mpeg3 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* Mpeg4 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* Mpeg5 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* Vdec0 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* Vdec1 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* 656In0 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* 656In1 */
     {false, BVDC_P_WindowId_eComp4_G0},  /* Gfx0 */
     {false, BVDC_P_WindowId_eComp4_G0},  /* Gfx1 */
     {false, BVDC_P_WindowId_eComp4_G0},  /* Gfx2 */
     {false, BVDC_P_WindowId_eComp4_G0},  /* Gfx3 */
     {true,  BVDC_P_WindowId_eComp4_G0},  /* Gfx4 */
     {false, BVDC_P_WindowId_eComp4_G0},  /* Gfx5 */
     {false, BVDC_P_WindowId_eComp4_G0},  /* Gfx6 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* HdDvi0 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* HdDvi1 */
     {false, BVDC_P_WindowId_eComp4_V0},  /* Ds0 */
     {false, BVDC_P_WindowId_eComp4_V0},  /* Vfd0 */
     {false, BVDC_P_WindowId_eComp4_V0},  /* Vfd1 */
     {false, BVDC_P_WindowId_eComp4_V0},  /* Vfd2 */
     {false, BVDC_P_WindowId_eComp4_V0},  /* Vfd3 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* Vfd4 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* Vfd5 */
     {true,  BVDC_P_WindowId_eComp4_V0},  /* Vfd6 */
     {true,  BVDC_P_WindowId_eComp4_V0}}, /* Vfd7 */

    /* VICE */
    {{true,  BVDC_P_WindowId_eComp5_V0},  /* Mpeg0 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* Mpeg1 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* Mpeg2 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* Mpeg3 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* Mpeg4 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* Mpeg5 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* Vdec0 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* Vdec1 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* 656In0 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* 656In1 */
     {false, BVDC_P_WindowId_eComp5_G0},  /* Gfx0 */
     {false, BVDC_P_WindowId_eComp5_G0},  /* Gfx1 */
     {false, BVDC_P_WindowId_eComp5_G0},  /* Gfx2 */
     {false, BVDC_P_WindowId_eComp5_G0},  /* Gfx3 */
     {false, BVDC_P_WindowId_eComp5_G0},  /* Gfx4 */
     {true,  BVDC_P_WindowId_eComp5_G0},  /* Gfx5 */
     {false, BVDC_P_WindowId_eComp5_G0},  /* Gfx6 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* HdDvi0 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* HdDvi1 */
     {false, BVDC_P_WindowId_eComp5_V0},  /* Ds0 */
     {false, BVDC_P_WindowId_eComp5_V0},  /* Vfd0 */
     {false, BVDC_P_WindowId_eComp5_V0},  /* Vfd1 */
     {false, BVDC_P_WindowId_eComp5_V0},  /* Vfd2 */
     {false, BVDC_P_WindowId_eComp5_V0},  /* Vfd3 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* Vfd4 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* Vfd5 */
     {true,  BVDC_P_WindowId_eComp5_V0},  /* Vfd6 */
     {true,  BVDC_P_WindowId_eComp5_V0}}, /* Vfd7 */

    /* VICE */
    {{true,  BVDC_P_WindowId_eComp6_V0},  /* Mpeg0 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* Mpeg1 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* Mpeg2 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* Mpeg3 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* Mpeg4 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* Mpeg5 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* Vdec0 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* Vdec1 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* 656In0 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* 656In1 */
     {false, BVDC_P_WindowId_eComp6_G0},  /* Gfx0 */
     {false, BVDC_P_WindowId_eComp6_G0},  /* Gfx1 */
     {false, BVDC_P_WindowId_eComp6_G0},  /* Gfx2 */
     {false, BVDC_P_WindowId_eComp6_G0},  /* Gfx3 */
     {false, BVDC_P_WindowId_eComp6_G0},  /* Gfx4 */
     {false, BVDC_P_WindowId_eComp6_G0},  /* Gfx5 */
     {true,  BVDC_P_WindowId_eComp6_G0},  /* Gfx6 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* HdDvi0 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* HdDvi1 */
     {false, BVDC_P_WindowId_eComp6_V0},  /* Ds0 */
     {false, BVDC_P_WindowId_eComp6_V0},  /* Vfd0 */
     {false, BVDC_P_WindowId_eComp6_V0},  /* Vfd1 */
     {false, BVDC_P_WindowId_eComp6_V0},  /* Vfd2 */
     {false, BVDC_P_WindowId_eComp6_V0},  /* Vfd3 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* Vfd4 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* Vfd5 */
     {true,  BVDC_P_WindowId_eComp6_V0},  /* Vfd6 */
     {true,  BVDC_P_WindowId_eComp6_V0}}, /* Vfd7 */
};
#endif

#define BVDC_P_CMP_GET_REG_ADDR_IDX(reg) \
    ((reg- BCHP_CMP_0_REG_START) / sizeof(uint32_t))

/* Get/Set reg data */
#define BVDC_P_CMP_GET_REG_ADDR_DATA(reg) \
    (hCompositor->aulRegs[BVDC_P_CMP_GET_REG_ADDR_IDX(reg)])

#if BVDC_ENABLE_50HZ_60HZ_FRAME_CAPTURE
    /* Enabling frame capture for 50Hz source to 60Hz display
     * will improve picture quality. However, it will require
     * more capture memory and system bandwidth. By default we
     * turn it off. Cross reference SW7425-2140 */
#define BVDC_P_DO_PULLDOWN(sourceVertRate, displayVertRate) \
    (displayVertRate > sourceVertRate)
#elif BVDC_ENABLE_60HZ_50HZ_FRAME_CAPTURE
    /* Enabling frame capture for 60Hz source to 50Hz display
     * will improve picture quality. However, it will require
     * more capture memory and system bandwidth. By default we
     * turn it off. Cross reference SWSTB-7037 */
#define BVDC_P_DO_PULLDOWN(sourceVertRate, displayVertRate) \
    (displayVertRate < sourceVertRate)
#else
#define BVDC_P_DO_PULLDOWN(sourceVertRate, displayVertRate) \
    (((displayVertRate * 10) / (sourceVertRate)) >= ((50 * 10) / 30) ? 1 : 0)
#endif

#define BVDC_P_FrameRate50Hz(eFrameRate) \
   (BAVC_FrameRateCode_e25 == eFrameRate || BAVC_FrameRateCode_e50 == eFrameRate ||\
    BAVC_FrameRateCode_e100 == eFrameRate || BAVC_FrameRateCode_e12_5 == eFrameRate )

#define BVDC_P_FrameRate60Hz(eFrameRate) \
   (BAVC_FrameRateCode_e29_97 == eFrameRate || BAVC_FrameRateCode_e30 == eFrameRate || \
    BAVC_FrameRateCode_e59_94 == eFrameRate || BAVC_FrameRateCode_e60 == eFrameRate ||\
    BAVC_FrameRateCode_e119_88== eFrameRate || BAVC_FrameRateCode_e120 == eFrameRate ||\
    BAVC_FrameRateCode_e14_985 == eFrameRate || BAVC_FrameRateCode_e15 == eFrameRate ||\
    BAVC_FrameRateCode_e7_493 == eFrameRate || BAVC_FrameRateCode_e7_5 == eFrameRate ||\
    BAVC_FrameRateCode_e9_99 == eFrameRate || BAVC_FrameRateCode_e10 == eFrameRate ||\
    BAVC_FrameRateCode_e19_98 == eFrameRate || BAVC_FrameRateCode_e20 == eFrameRate)

#define BVDC_P_CAL_HRZ_SRC_STEP(InW, OutW) \
    BVDC_P_DIV_ROUND_NEAR(InW << BVDC_P_NRM_SRC_STEP_F_BITS, OutW)

#if (BDBG_DEBUG_BUILD)
/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Window_DumpRects_isr
    ( const BVDC_Window_Handle         hWindow,
      const BVDC_P_PictureNode        *pPicture )
{
    bool bForcePrint;
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor->hVdc, BVDC_VDC);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);

    bForcePrint = hWindow->hCompositor->hVdc->bForcePrint;

    if(bForcePrint)
    {
        BDBG_ERR(("---------------Window[%d] chan[%d]'s VnetMode: 0x%04x-------------",
            hWindow->eId, pPicture->ulPictureIdx, *(unsigned int *)&hWindow->stVnetMode));
        BVDC_P_PRINT_CLIP("SrcClip*", &hWindow->stCurInfo.stSrcClip, bForcePrint);
        BVDC_P_PRINT_RECT("DstRect*", &hWindow->stCurInfo.stDstRect, bForcePrint, 0, 0);
        BVDC_P_PRINT_RECT("SclOut* ", &hWindow->stCurInfo.stScalerOutput, bForcePrint, 0, 0);
    }
    else
    {
        BDBG_MSG(("---------------Window[%d] chan[%d]'s VnetMode: 0x%x--------------",
            hWindow->eId, pPicture->ulPictureIdx, *(unsigned int *)&pPicture->stVnetMode));
    }

    BVDC_P_PRINT_RECT("ScanOut", &hWindow->stCurInfo.hSource->stScanOut, bForcePrint, 0, 0);
    BVDC_P_PRINT_RECT("pSrcCnt", &(hWindow->stSrcCnt), bForcePrint, BVDC_P_16TH_PIXEL_SHIFT, BVDC_P_16TH_PIXEL_MASK);

    BVDC_P_PRINT_RECT("pSrcOut", pPicture->pSrcOut, bForcePrint, 0, 0);
    if(BVDC_P_VNET_USED_DNR_AT_WRITER(pPicture->stVnetMode))
    {
        BVDC_P_PRINT_RECT("pDnrIn ", pPicture->pDnrIn, bForcePrint, 0, 0);
        BVDC_P_PRINT_RECT("pDnrOut", pPicture->pDnrOut, bForcePrint, 0, 0);
    }
    if(BVDC_P_VNET_USED_XSRC_AT_WRITER(pPicture->stVnetMode))
    {
        BVDC_P_PRINT_RECT("pXsrcIn ", pPicture->pXsrcIn, bForcePrint, 0, 0);
        BVDC_P_PRINT_RECT("pXsrcOut", pPicture->pXsrcOut, bForcePrint, 0, 0);
    }

    if(BVDC_P_VNET_USED_VFC_AT_WRITER(pPicture->stVnetMode))
    {
        BVDC_P_PRINT_RECT("pVfcIn ", pPicture->pVfcIn, bForcePrint, 0, 0);
        BVDC_P_PRINT_RECT("pVfcOut", pPicture->pVfcOut, bForcePrint, 0, 0);
    }

    if(BVDC_P_VNET_USED_MVP_AT_WRITER(pPicture->stVnetMode))
    {
        /* not bypass */
        if(BVDC_P_MVP_USED_HSCL(pPicture->stMvpMode))
        {
            BVDC_P_PRINT_RECT("pHsclIn ", pPicture->pHsclIn, bForcePrint, 0, 0);
            BVDC_P_PRINT_SCLCUT_RECT("stHsclCut", &pPicture->stHsclCut, bForcePrint);
            BVDC_P_PRINT_RECT("pHsclOut", pPicture->pHsclOut, bForcePrint, 0, 0);
        }
        if(BVDC_P_MVP_USED_ANR(pPicture->stMvpMode))
        {
            BVDC_P_PRINT_RECT("pAnrIn ", pPicture->pAnrIn, bForcePrint, 0, 0);
            BVDC_P_PRINT_RECT("pAnrOut", pPicture->pAnrOut, bForcePrint, 0, 0);
        }
        if(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode))
        {
            BVDC_P_PRINT_RECT("pMadIn ", pPicture->pMadIn, bForcePrint, 0, 0);
            BVDC_P_PRINT_RECT("pMadOut", pPicture->pMadOut, bForcePrint, 0, 0);
        }
    }
    if(BVDC_P_VNET_USED_SCALER(pPicture->stVnetMode))
    {
        if(BVDC_P_VNET_USED_SCALER_AT_READER(pPicture->stVnetMode) &&
           BVDC_P_VNET_USED_CAPTURE(pPicture->stVnetMode))
        {
            BVDC_P_PRINT_RECT("pCapIn ", pPicture->pCapIn, bForcePrint, 0, 0);
            BVDC_P_PRINT_RECT("pCapOut", pPicture->pCapOut, bForcePrint, 0, 0);
            BVDC_P_PRINT_RECT("pVfdIn ", pPicture->pVfdIn, bForcePrint, 0, 0);
            BVDC_P_PRINT_RECT("pVfdOut", pPicture->pVfdOut, bForcePrint, 0, 0);
        }

        if(BVDC_P_VNET_USED_DNR_AT_READER(pPicture->stVnetMode))
        {
            BVDC_P_PRINT_RECT("pDnrIn ", pPicture->pDnrIn, bForcePrint, 0, 0);
            BVDC_P_PRINT_RECT("pDnrOut", pPicture->pDnrOut, bForcePrint, 0, 0);
        }
        if(BVDC_P_VNET_USED_XSRC_AT_READER(pPicture->stVnetMode))
        {
            BVDC_P_PRINT_RECT("pXsrcIn ", pPicture->pXsrcIn, bForcePrint, 0, 0);
            BVDC_P_PRINT_RECT("pXsrcOut", pPicture->pXsrcOut, bForcePrint, 0, 0);
        }

        if(BVDC_P_VNET_USED_VFC_AT_READER(pPicture->stVnetMode))
        {
            BVDC_P_PRINT_RECT("pVfcIn ", pPicture->pVfcIn, bForcePrint, 0, 0);
            BVDC_P_PRINT_RECT("pVfcOut", pPicture->pVfcOut, bForcePrint, 0, 0);
        }

        if(BVDC_P_VNET_USED_MVP_AT_READER(pPicture->stVnetMode))
        {
            /* not bypass */
            if(BVDC_P_MVP_USED_HSCL(pPicture->stMvpMode))
            {
                BVDC_P_PRINT_RECT("pHsclIn ", pPicture->pHsclIn, bForcePrint, 0, 0);
                BVDC_P_PRINT_SCLCUT_RECT("stHsclCut", &pPicture->stHsclCut, bForcePrint);
                BVDC_P_PRINT_RECT("pHsclOut", pPicture->pHsclOut, bForcePrint, 0, 0);
            }
            if(BVDC_P_MVP_USED_ANR(pPicture->stMvpMode))
            {
                BVDC_P_PRINT_RECT("pAnrIn ", pPicture->pAnrIn, bForcePrint, 0, 0);
                BVDC_P_PRINT_RECT("pAnrOut", pPicture->pAnrOut, bForcePrint, 0, 0);
            }
            if(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode))
            {
                BVDC_P_PRINT_RECT("pMadIn ", pPicture->pMadIn, bForcePrint, 0, 0);
                BVDC_P_PRINT_RECT("pMadOut", pPicture->pMadOut, bForcePrint, 0, 0);
            }
        }

        BVDC_P_PRINT_RECT("pSclIn ", pPicture->pSclIn, bForcePrint, 0, 0);
        BVDC_P_PRINT_SCLCUT_RECT("pSclCut", &(pPicture->stSclCut), bForcePrint);
        BVDC_P_PRINT_RECT("pSclOut", pPicture->pSclOut, bForcePrint, 0, 0);
    }

    if(BVDC_P_VNET_USED_CAPTURE(pPicture->stVnetMode) &&
       !BVDC_P_VNET_USED_SCALER_AT_READER(pPicture->stVnetMode))
    {
        BVDC_P_PRINT_RECT("pCapIn ", pPicture->pCapIn, bForcePrint, 0, 0);
        BVDC_P_PRINT_RECT("pCapOut", pPicture->pCapOut, bForcePrint, 0, 0);
        BVDC_P_PRINT_RECT("pVfdIn ", pPicture->pVfdIn, bForcePrint, 0, 0);
        BVDC_P_PRINT_RECT("pVfdOut", pPicture->pVfdOut, bForcePrint, 0, 0);
    }

    BVDC_P_PRINT_RECT("pWinIn ", pPicture->pWinIn, bForcePrint, 0, 0);
    BVDC_P_PRINT_RECT("pWinOut", pPicture->pWinOut, bForcePrint, 0, 0);

    bForcePrint
        ? BDBG_ERR(("--------------------------------------------------------"))
        : BDBG_MSG(("--------------------------------------------------------"));

    return;
}
#endif

void BVDC_P_Window_SetInvalidVnetMode_isr
    ( BVDC_P_VnetMode                  *pVnetMode )
{
    BVDC_P_CLEAN_ALL_DIRTY(pVnetMode);
    pVnetMode->stBits.bInvalid = BVDC_P_ON;
}

#if (BVDC_P_SUPPORT_HIST)
static bool BVDC_P_Hist_Level_Cmp
    ( const uint32_t                   *pulNewThres,
      const uint32_t                   *pulCurThres )
{
    uint32_t id;
    for(id = 0; id < BVDC_LUMA_HISTOGRAM_LEVELS; id++)
    {
        if(*(pulNewThres + id) != *(pulCurThres + id))
            return false;
    }
    return true;
}
#endif

#ifndef BVDC_FOR_BOOTUPDATER
/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Window_GetPrivHandle
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_WindowId                    eWinId,
      BAVC_SourceId                    eSrcId,
      BVDC_Window_Handle              *phWindow,
      BVDC_P_WindowId                 *peWindowId )
{
    BVDC_Window_Handle hWindow;
    BVDC_P_WindowId eWindowId;

    /* Check if this window allow on this compositor?  Some compositor does not
     * support graphics window that crosses to other compositor. */
    if(!s_aaWindowIdSelectTable[hCompositor->eId][eSrcId].bValid)
    {
        BDBG_ERR(("Window is not available."));
        return BERR_TRACE(BVDC_ERR_WINDOW_NOT_AVAILABLE);
    }

    /* Determine what window id to use. */
    eWindowId = s_aaWindowIdSelectTable[hCompositor->eId][eSrcId].eWindowId;
    if(BVDC_P_SRC_IS_VIDEO(eSrcId) && (eWinId != BVDC_WindowId_eAuto))
    {
        eWindowId += eWinId;
    }

    /* Need to select V0 or V1 for comp0/comp1. */
    if((BVDC_P_SRC_IS_VIDEO(eSrcId)) && (eWinId == BVDC_WindowId_eAuto) &&
       (hCompositor->pFeatures->ulMaxVideoWindow > 1))
    {
        uint32_t ulIndex;
        BVDC_Window_Handle hV0, hV1;

        ulIndex = (hCompositor->eId == BVDC_CompositorId_eCompositor0)
            ? BVDC_P_WindowId_eComp0_V0 : BVDC_P_WindowId_eComp1_V0;
        hV0 = hCompositor->ahWindow[ulIndex];
        hV1 = hCompositor->ahWindow[ulIndex+1];

        /* Check if V0 or V1 is available */
        hWindow = (NULL != hV0)?hV0:hV1;
    }
    else
    {
        hWindow = hCompositor->ahWindow[eWindowId];
    }

    if(phWindow)
    {
        *phWindow = hWindow;
        BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    }

    if(peWindowId)
    {
        *peWindowId = (hWindow) ? hWindow->eId : eWindowId;
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Window_Create
    ( BVDC_Compositor_Handle            hCompositor,
      BVDC_Window_Handle               *phWindow,
      BAVC_SourceId                     eSrcId,
      BVDC_WindowId                     eWinId )
{
    BVDC_P_WindowContext *pWindow;
    BVDC_P_WindowId      eWindowId;
    uint32_t ulBoxWinId;
    const BBOX_Vdc_Capabilities *pBoxVdc;
    const BBOX_Vdc_ResourceFeature *pBoxVdcResource;

    BDBG_ENTER(BVDC_P_Window_Create);
    BDBG_ASSERT(phWindow);

    /* Make sure the table have enough elements. */
    BDBG_CASSERT(BVDC_P_RESOURCE_TABLE_COUNT == BVDC_P_MAX_WINDOW_COUNT);
    BDBG_CASSERT(BVDC_P_RESOURCE_FEATURE_TABLE_COUNT == BVDC_P_MAX_WINDOW_COUNT);

    /* Ascertain that VDC window ID correctly mirrors BOX's */
    BDBG_CASSERT(BBOX_Vdc_Window_eGfx2 == (BBOX_Vdc_WindowId)BVDC_WindowId_eGfx2);
    /* Ascertain that VDC SclCap enum correctly mirrors BOX's */
    BDBG_CASSERT(BBOX_Vdc_SclCapBias_eSclAfterCap == (BBOX_Vdc_SclCapBias)BVDC_SclCapBias_eSclAfterCap);
    /* Ascertain that VDC resource defines correctly mirrors BOX's */
    BDBG_CASSERT(BBOX_Vdc_Resource_eHdmi1 == (BBOX_Vdc_Resource)BVDC_P_Able_eHdmi1);
    BDBG_CASSERT(BBOX_FTR_HD_MR5 == FTR_HD_MR5);

    /* Get relate context. */
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hCompositor->hVdc, BVDC_VDC);

    /* Determine what window id to use. */
    eWindowId = s_aaWindowIdSelectTable[hCompositor->eId][eSrcId].eWindowId;

    if((BVDC_P_SRC_IS_VIDEO(eSrcId) && (eWinId != BVDC_WindowId_eAuto) && ((eWinId - BVDC_WindowId_eVideo0) >= hCompositor->pFeatures->ulMaxVideoWindow)) ||
       (BVDC_P_SRC_IS_GFX(eSrcId)   && (eWinId != BVDC_WindowId_eAuto) && ((eWinId - BVDC_WindowId_eGfx0)   >= hCompositor->pFeatures->ulMaxGfxWindow)))
    {
        BDBG_LEAVE(BVDC_P_Window_Create);
        return BERR_TRACE(BVDC_ERR_WINDOW_NOT_AVAILABLE);
    }
    if(BVDC_P_SRC_IS_VIDEO(eSrcId) && (eWinId != BVDC_WindowId_eAuto))
    {
        eWindowId += eWinId;
    }

    if(hCompositor->ahWindow[eWindowId]!=NULL)
    {
        BDBG_MSG(("cmp[%d] win[%d] has been created state %d", hCompositor->eId, eWinId, hCompositor->ahWindow[eWindowId]->eState));
        *phWindow = hCompositor->ahWindow[eWindowId];
        BDBG_LEAVE(BVDC_P_Window_Create);
        return BERR_SUCCESS;
    }

    /* (1) Alloc the context. */
    pWindow = (BVDC_P_WindowContext*)
        (BKNI_Malloc(sizeof(BVDC_P_WindowContext)));
    if(!pWindow)
    {
        BDBG_LEAVE(BVDC_P_Window_Create);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* Clear out the context and set defaults. */
    BKNI_Memset((void*)pWindow, 0x0, sizeof(BVDC_P_WindowContext));
    BDBG_OBJECT_SET(pWindow, BVDC_WIN);

    /* Initialize window context */
    pWindow->eId          = eWindowId;
    pWindow->ulRegOffset  = BVDC_P_WIN_GET_REG_OFFSET(eWindowId);
    pWindow->hCompositor  = hCompositor;
    pWindow->stResourceRequire  = s_aResourceRequireTable[eWindowId];
    pWindow->stResourceFeature  = s_aResourceFeatureTable[eWindowId];

#if (BVDC_P_SUPPORT_TNT)
    if(BVDC_P_WindowId_eComp0_V0 ==eWindowId)
    {
        pWindow->bTntAvail = true;
        pWindow->ulTntRegOffset = 0;
    }
    if(BVDC_P_SUPPORT_TNT == 2)
    {
        /* either TNT_CMP_0_V0 and TNT_CMP_0_V1 or TNT_CMP_1_V0 */
        pWindow->ulTntRegOffset =
#if BCHP_TNT_CMP_0_V1_REG_START
            (BVDC_P_WindowId_eComp0_V1==(eWindowId)) ? BCHP_TNT_CMP_0_V1_REG_START - BCHP_TNT_CMP_0_V0_REG_START :
#elif BCHP_TNT_CMP_1_V0_REG_START
            (BVDC_P_WindowId_eComp1_V0==(eWindowId)) ? BCHP_TNT_CMP_1_V0_REG_START - BCHP_TNT_CMP_0_V0_REG_START :
#endif
            0;
        pWindow->bTntAvail =
            (BVDC_P_WindowId_eComp0_V0==(eWindowId)) ? true :
#if BCHP_TNT_CMP_0_V1_REG_START
            (BVDC_P_WindowId_eComp0_V1==(eWindowId)) ? true :
#elif BCHP_TNT_CMP_1_V0_REG_START
            (BVDC_P_WindowId_eComp1_V0==(eWindowId)) ? true :
#endif
            false;
    }
#endif

#if BVDC_P_SUPPORT_MASK_DITHER
    if(BVDC_P_WindowId_eComp0_V0 ==eWindowId)
    {
        pWindow->bMaskAvail = true;
        pWindow->ulMaskRegOffset = 0;
    }
    if(BVDC_P_SUPPORT_MASK_DITHER == 2)
    {
        pWindow->ulMaskRegOffset =
#if BCHP_MASK_1_REG_START
            (BVDC_P_WindowId_eComp1_V0==(eWindowId)) ? BCHP_MASK_1_REG_START - BCHP_MASK_0_REG_START :
#endif
            0;
        pWindow->bMaskAvail =
            (BVDC_P_WindowId_eComp0_V0==(eWindowId)) ? true :
#if BCHP_MASK_1_REG_START
            (BVDC_P_WindowId_eComp1_V0==(eWindowId)) ? true :
#endif
            false;
    }
#endif

    /* Check if BOX has specific deinterlacer allocation */
    pBoxVdc = &hCompositor->hVdc->stBoxConfig.stVdc;
    ulBoxWinId = BVDC_P_GetBoxWindowId_isrsafe(eWindowId);
    BDBG_ASSERT(ulBoxWinId < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY);

    pBoxVdcResource = &pBoxVdc->astDisplay[hCompositor->eId].astWindow[ulBoxWinId].stResource;

    /* Change default MAD allocation to BOX's deinterlacer allocation */
    if (pBoxVdcResource->ulMad != BBOX_VDC_DISREGARD)
    {
        pWindow->stResourceFeature.ulMad = pBoxVdcResource->ulMad;

        BDBG_MSG(("Win[%d]: BOX is overriding default deinterlacer resource with deinterlacer 0x%x", pWindow->eId, pWindow->stResourceFeature.ulMad));
    }

    /* (2) Alloc capture & capture memory for this window. */
    if(pWindow->stResourceRequire.bRequireCapture)
    {
        if ((pBoxVdcResource->eCap != BBOX_VDC_DISREGARD) && (pBoxVdcResource->eCap != BBOX_Vdc_Resource_Capture_eUnknown))
        {
            pWindow->stResourceRequire.eCapture = pBoxVdcResource->eCap;
            BDBG_MSG(("Win[%d]: BOX is overriding default capture resource with capture 0x%x", pWindow->eId, pWindow->stResourceRequire.eCapture));
        }

        /* Get the register handle this will be needed capture to setup the
         * trigger with host write.  Memory handle is for conveting address. */
        BVDC_P_Capture_Create(&pWindow->stNewResource.hCapture, hCompositor->hVdc->hRdc,
            hCompositor->hVdc->hRegister, pWindow->stResourceRequire.eCapture,
#if (!BVDC_P_USE_RDC_TIMESTAMP)
            hCompositor->hVdc->hTimer,
#endif
            hCompositor->hVdc->hResource);

        pWindow->stNewResource.hCapture->hWindow = pWindow;

#if (!BVDC_P_USE_RDC_TIMESTAMP) /** { **/
        if ((hCompositor->hVdc->ahSource[eSrcId]) &&(BVDC_P_SRC_IS_VFD(eSrcId)))
        {
            /* the vfd can not be both in cap/playback pair and an gfx src */
            pWindow->stNewResource.hCapture->ulTimestampRegAddr =
                hCompositor->hVdc->ahSource[eSrcId]->hVfdFeeder->stGfxSurface.ulSurAddrReg[0];
        }
        else
        {
            pWindow->stNewResource.hCapture->ulTimestampRegAddr = BRDC_AllocScratchReg(hCompositor->hVdc->hRdc);
            BDBG_MSG(("AllocScratchReg 0x%x for window[%d] capture %d",
                pWindow->stNewResource.hCapture->ulTimestampRegAddr, pWindow->eId, pWindow->stResourceRequire.eCapture));
        }
#endif /** } !BVDC_P_USE_RDC_TIMESTAMP **/

        /* Create buffer list for managing multi-buffering.  And create device
         * memory for capture engine. */
        BVDC_P_Buffer_Create(pWindow, &pWindow->hBuffer);

    }

    /* (3) Alloc playback block */
    if(pWindow->stResourceRequire.bRequirePlayback)
    {
        if ((pBoxVdcResource->eVfd != BBOX_VDC_DISREGARD) && (pBoxVdcResource->eVfd != BBOX_Vdc_Resource_Feeder_eUnknown))
        {
            pWindow->stResourceRequire.ePlayback = (BVDC_P_FeederId)pBoxVdcResource->eVfd;
            BDBG_MSG(("Win[%d]: BOX is overriding default feeder resource with feeder 0x%x", pWindow->eId, pWindow->stResourceRequire.ePlayback));
        }

        BVDC_P_Feeder_Create(&pWindow->stNewResource.hPlayback, hCompositor->hVdc->hRdc,
            hCompositor->hVdc->hRegister, pWindow->stResourceRequire.ePlayback,
#if (!BVDC_P_USE_RDC_TIMESTAMP)
            hCompositor->hVdc->hTimer,
#endif
            NULL, hCompositor->hVdc->hResource, false);

        pWindow->stNewResource.hPlayback->hWindow = pWindow;

#if (!BVDC_P_USE_RDC_TIMESTAMP) /** { **/
        if ((hCompositor->hVdc->ahSource[eSrcId])&&(BVDC_P_SRC_IS_VFD(eSrcId)))
        {
            /* the vfd can not be both a playback and an gfx src */
            pWindow->stNewResource.hPlayback->ulTimestampRegAddr =
                hCompositor->hVdc->ahSource[eSrcId]->hVfdFeeder->stGfxSurface.ulVsyncCntrReg;
        }
        else
        {
            pWindow->stNewResource.hPlayback->ulTimestampRegAddr = BRDC_AllocScratchReg(hCompositor->hVdc->hRdc);
            BDBG_MSG(("AllocScratchReg 0x%x for window[%d] playback %d",
                pWindow->stNewResource.hPlayback->ulTimestampRegAddr, pWindow->eId, pWindow->stResourceRequire.ePlayback));
        }
#endif /** } !BVDC_P_USE_RDC_TIMESTAMP **/

    }

    /* (4) Alloc scaler block */
    if(pWindow->stResourceRequire.bRequireScaler)
    {
        if ((pBoxVdcResource->eScl != BBOX_VDC_DISREGARD) && (pBoxVdcResource->eScl != BBOX_Vdc_Resource_Scaler_eUnknown))
        {
            pWindow->stResourceRequire.eScaler = (BVDC_P_ScalerId)pBoxVdcResource->eScl;
            BDBG_MSG(("Win[%d]: BOX is overriding default scaler resource with feeder 0x%x", pWindow->eId, pWindow->stResourceRequire.eScaler));
        }

        BVDC_P_Scaler_Create(&pWindow->stNewResource.hScaler, pWindow->stResourceRequire.eScaler,
            hCompositor->hVdc->hResource, hCompositor->hVdc->hRegister);
    }

    /* (5) Alloc PEP block */
    if(pWindow->stResourceRequire.bRequirePep)
    {
        BVDC_P_Pep_Create(&pWindow->stNewResource.hPep, pWindow->stResourceRequire.eWinId, hCompositor->hVdc->hRegister);
    }

    /* (6) create a DestroyDone event. */
    BKNI_CreateEvent(&pWindow->hDestroyDoneEvent);

    /* (7) create a AppliedDone event. */
    BKNI_CreateEvent(&pWindow->hAppliedDoneEvent);

    /* (8) Init SubRul that manage compositer' win src vnet setting */
    BVDC_P_SubRul_Init(&(pWindow->stWinOutVnet),
        s_aulWinOutMuxAddr[pWindow->eId], 0, BVDC_P_DrainMode_eNone, 0,
        hCompositor->hVdc->hResource);

    /* (9) Sync up stNewResource and stCurResource. For historical reasons, these resources are
    * not allocated via the resource management library. Weird, but that's how it has been.
    */
    pWindow->stCurResource = pWindow->stNewResource;

    /* (10) Added this compositor to hVdc */
    hCompositor->ahWindow[pWindow->eId] = (BVDC_Window_Handle)pWindow;

    /* All done. now return the new fresh context to user. */
    *phWindow = (BVDC_Window_Handle)pWindow;

    BDBG_LEAVE(BVDC_P_Window_Create);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Window_Destroy
    ( BVDC_Window_Handle               hWindow )
{
#if (!BVDC_P_USE_RDC_TIMESTAMP)
    BAVC_SourceId  eSrcId;
#endif

    BDBG_ENTER(BVDC_P_Window_Destroy);
    if(!hWindow)
    {
        BDBG_LEAVE(BVDC_P_Window_Destroy);
        return;
    }

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Get the compositor that this window belongs to.   The blender
     * registers are spread out in the compositor. */
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

#if BVDC_P_SUPPORT_XCODE_WIN_CAP
    /* [9] Decrement xcode window capture counter */
    if(BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg))
    {
        hWindow->hCompositor->hVdc->ulXcodeWinCap -= hWindow->stCurInfo.bForceCapture;
    }
#endif

    /* [8] Added this compositor to hVdc */
    hWindow->hCompositor->ahWindow[hWindow->eId] = NULL;

    /* [7] Destroy event */
    BKNI_DestroyEvent(hWindow->hAppliedDoneEvent);

    /* [6] Destroy event */
    BKNI_DestroyEvent(hWindow->hDestroyDoneEvent);

    /* [5] Free PEP block */
    if(hWindow->stResourceRequire.bRequirePep)
    {
        BVDC_P_Pep_Destroy(hWindow->stCurResource.hPep);
    }

    /* [4] Free scaler block */
    if(hWindow->stResourceRequire.bRequireScaler)
    {
        BVDC_P_Scaler_Destroy(hWindow->stCurResource.hScaler);
    }

    /* [3] Free playback block */
#if (!BVDC_P_USE_RDC_TIMESTAMP)
    eSrcId =
        BAVC_SourceId_eVfd0 +
            (hWindow->stResourceRequire.ePlayback - BVDC_P_FeederId_eVfd0);
#endif
    if(hWindow->stResourceRequire.bRequirePlayback)
    {
#if (!BVDC_P_USE_RDC_TIMESTAMP)
        if (NULL == hWindow->hCompositor->hVdc->ahSource[eSrcId])
        {
            BDBG_MSG(("FreeScratchReg 0x%x for window[%d] playback %d",
                hWindow->stNewResource.hPlayback->ulTimestampRegAddr,
                hWindow->eId, hWindow->stResourceRequire.ePlayback));

            BRDC_FreeScratchReg(hWindow->hCompositor->hVdc->hRdc,
                hWindow->stNewResource.hPlayback->ulTimestampRegAddr);
        }
#endif

        BVDC_P_Feeder_Destroy(hWindow->stCurResource.hPlayback);
    }

    /* [2] Free capture & capture memory for this window. */
    if(hWindow->stResourceRequire.bRequireCapture)
    {
#if (!BVDC_P_USE_RDC_TIMESTAMP)
        if (NULL == hWindow->hCompositor->hVdc->ahSource[eSrcId])
        {
            BDBG_MSG(("FreeScratchReg 0x%x for window[%d] capture %d",
                hWindow->stNewResource.hCapture->ulTimestampRegAddr,
                hWindow->eId, hWindow->stResourceRequire.eCapture));
            BRDC_FreeScratchReg(hWindow->hCompositor->hVdc->hRdc,
                hWindow->stNewResource.hCapture->ulTimestampRegAddr);
        }
#endif
        BVDC_P_Buffer_Destroy(hWindow->hBuffer);
        BVDC_P_Capture_Destroy(hWindow->stCurResource.hCapture);
    }

    BDBG_OBJECT_DESTROY(hWindow, BVDC_WIN);
    /* [1] Free context in system memory */
    BKNI_Free((void*)hWindow);

    BDBG_LEAVE(BVDC_P_Window_Destroy);
    return;
}

/***************************************************************************
 * {private}
 *
 * PLATFORMS: Specifics override that either take adavantage of bvn doc
 * assumptions or otherwise default to support ref rts.  Otherwise can
 * be overwritten by public API
 */
void BVDC_P_Window_Rts_Init
    (
      bool                             bCmpXcode,
      bool                             bDispNrtStg,
      bool                            *pbForceCapture,
      BVDC_SclCapBias                 *peSclCapBias,
      uint32_t                        *pulBandwidthDelta )
{
    bool             bForceCapture;
    uint32_t         ulBandwidthDelta;
    BVDC_SclCapBias  eSclCapBias;

    bForceCapture    =!(bCmpXcode || bDispNrtStg);
    eSclCapBias      = BVDC_SclCapBias_eAuto;
    ulBandwidthDelta = BVDC_P_BW_DEFAULT_DELTA;

    if(pbForceCapture)
        *pbForceCapture = bForceCapture;

    if(peSclCapBias)
        *peSclCapBias = eSclCapBias;

    if(pulBandwidthDelta)
        *pulBandwidthDelta = ulBandwidthDelta;

}

void BVDC_P_Window_Compression_Init_isr
    ( bool                             bIs10BitCore,
      bool                             bSupportDcxm,
      BVDC_P_Compression_Settings     *pCapCompSetting,
      BVDC_P_Compression_Settings     *pMadCompSetting,
      BVDC_P_MvpDcxCore                eDcxCore)
{

    /* Default setting for capture compression */
    if(pCapCompSetting)
    {
        if(bSupportDcxm)
        {
            /* enable compression by default for 10 bits datapath*/
            pCapCompSetting->bEnable = true;
            pCapCompSetting->ulPixelPerGroup = BVDC_DCX_PIXEL_PER_GROUP;
            pCapCompSetting->ulBitsPerGroup = BVDC_40BITS_PER_GROUP;
        }
        else
        {
            pCapCompSetting->bEnable = false;
            pCapCompSetting->ulPixelPerGroup = 0;
            pCapCompSetting->ulBitsPerGroup = 0;
        }
        pCapCompSetting->ulPredictionMode = 0;
    }

    /* Default setting for deinterlacer compression */
    if(pMadCompSetting)
    {
        pMadCompSetting->bEnable = true;
        pMadCompSetting->ulPixelPerGroup = BVDC_DCX_PIXEL_PER_GROUP;
        pMadCompSetting->ulBitsPerGroup =
            (bIs10BitCore && (eDcxCore==BVDC_P_Mvp_Dcxs2))?BVDC_37BITS_PER_GROUP: BVDC_36BITS_PER_GROUP;
        pMadCompSetting->ulPredictionMode = 0;
    }

    return;
}

/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Window_Init
    ( BVDC_Window_Handle               hWindow,
      BVDC_Source_Handle               hSource )
{
    BVDC_P_Window_Info *pNewInfo, *pCurInfo;
    BVDC_P_Window_DirtyBits *pNewDirty;
    /* coverity[result_independent_of_operands: FALSE] */
    const BBOX_Vdc_Capabilities *pBoxVdcCap;
    const BBOX_Vdc_Display_Capabilities *pBoxVdcDispCap;
    uint32_t ulBoxWinId;
    BVDC_DisplayId   eDisplayId;

    BDBG_ENTER(BVDC_P_Window_Init);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    /* Init to default state. */
    hWindow->eState               = BVDC_P_State_eInactive;
    hWindow->bUserAppliedChanges  = false;
    hWindow->bSyncLockSrc         = false;
    hWindow->ulTotalBvnDelay      = 0;
    BVDC_P_Window_SetInvalidVnetMode_isr(&(hWindow->stVnetMode));

    /* MAD delay sw pipeline reset */
    hWindow->bResetMadDelaySwPipe = false;

    /* Use to determine vnet modes. */
    hWindow->bCapture             = false;
    hWindow->eScanoutMode         = BVDC_P_ScanoutMode_eLive;
    hWindow->eBufferHeapIdRequest = BVDC_P_BufferHeapId_eUnknown;
    hWindow->eBufferHeapIdPrefer  = BVDC_P_BufferHeapId_eUnknown;
    hWindow->ulBufCntNeeded       = 0;
    hWindow->ulBufCntAllocated    = 0;
    hWindow->eBufAllocMode        = BVDC_P_BufHeapAllocMode_eLeftOnly;
    hWindow->ePrevBufAllocMode    = BVDC_P_BufHeapAllocMode_eLeftOnly;
    hWindow->bRightBufAllocated   = false;
    hWindow->uiAvailCaptureBuffers = 0;
    hWindow->ulBufDelay            = 0;
    hWindow->ulPrevBufCntNeeded    = 0;
    hWindow->bBufferCntDecremented = false;
    hWindow->bBufferCntDecrementedForPullDown = false;
    hWindow->bBufferCntIncremented = false;
    hWindow->ulBlenderMuteCnt      = 0;
    hWindow->bSlipBuiltWriter      = false;

    /* adjusted rectangles */
    hWindow->bAdjRectsDirty        = true;
    hWindow->bMuteBypass           = false;

    /* MosaicMode mosaic rect set */
    hWindow->ulMosaicRectSet       = ~0; /* to update rect setting */
    hWindow->bClearRectSupport     = hWindow->stSettings.ulMaxMosaicRect
        ? true : false;

    hWindow->eSrcOrientation       = BFMT_Orientation_e2D;
    hWindow->eDispOrientation      = BFMT_Orientation_e2D;

    /* Reset done events */
    hWindow->bSetDestroyEventPending = false;
    hWindow->bSetAppliedEventPending = false;
    BKNI_ResetEvent(hWindow->hDestroyDoneEvent);
    BKNI_ResetEvent(hWindow->hAppliedDoneEvent);

    /* Initialize cadence handling related fields */
    hWindow->stCadHndl.bForceAltCap = false;
    hWindow->stCadHndl.eLastCapPolarity = BAVC_Polarity_eInvalid;
    hWindow->stCadHndl.bReaderCadMatching = true;
    hWindow->stCadHndl.bDecoderRateCov = false;
    hWindow->stCadHndl.bTrickMode = false;

    hWindow->ulDropCntNonIgnoredPics = 0;
#if BVDC_P_SUPPORT_STG
    {
        uint32_t i, j;
        for (i = 0; i < BAVC_MOSAIC_MAX; i++)
        {
            for(j=0; j< BVDC_MADDELAY_BUF_COUNT; j++)
            {
                hWindow->stMadDelayed[i][j].bIgnorePicture = true;
                hWindow->stMadDelayed[i][j].bChannelChange = true;
            }
        }
    }
    if(BVDC_P_SRC_IS_MPEG(hSource->eId)) {
        BRDC_WriteScratch(hSource->hVdc->hRegister, hSource->ulScratchPolReg, 0);
#if BVDC_P_STG_NRT_CADENCE_WORKAROUND
        hSource->bToggleCadence = false;
#endif
    }
    BDBG_MODULE_MSG(BVDC_REPEATPOLARITY, ("win[%d] clear src[%d] scratch pol reg ",hWindow->eId, hSource->eId));
#endif

#if BVDC_P_SUPPORT_MOSAIC_MODE
    {
        uint32_t i;

        for (i = 0; i < BAVC_MOSAIC_MAX; i++)
            hWindow->aulMosaicZOrderIndex[i] = i;
    }
#endif

    hWindow->pMainCfc = &hWindow->astMosaicCfc[0];
    hWindow->pDemoCfc = &hWindow->astMosaicCfc[1];

    /* Initial new/current public states */
    pNewInfo = &hWindow->stNewInfo;
    pCurInfo = &hWindow->stCurInfo;

    /* Clear out user's states. */
    BKNI_Memset((void*)pNewInfo, 0x0, sizeof(BVDC_P_Window_Info));
    BKNI_Memset((void*)pCurInfo, 0x0, sizeof(BVDC_P_Window_Info));

    /* Init dirty bits so there will executed at least once. */
    pNewDirty = &pNewInfo->stDirty;
    BDBG_CASSERT(sizeof(pNewDirty->stBits) <= sizeof(pNewDirty->aulInts));
    pNewDirty->stBits.bCscAdjust      = BVDC_P_DIRTY;
    pNewDirty->stBits.bColorKeyAdjust  = BVDC_P_DIRTY;

    /* Disconnecting/Connecting Source */
    hWindow->stNewInfo.hSource     = hSource;

    /* Misc flags. */
    pNewInfo->bUseSrcFrameRate     = false;
    pNewInfo->bDeinterlace         = false;
    pNewInfo->bVisible             = false;
    pNewInfo->ucZOrder             = 0;
    pNewInfo->ucAlpha              = BVDC_ALPHA_MAX;
    pNewInfo->ucConstantAlpha      = BVDC_ALPHA_MAX;
    pNewInfo->eFrontBlendFactor    = BVDC_BlendFactor_eSrcAlpha;
    pNewInfo->eBackBlendFactor     = BVDC_BlendFactor_eOneMinusSrcAlpha;
    pNewInfo->bCscRgbMatching      = false;
    pNewInfo->uiVsyncDelayOffset   = 0;
    pNewInfo->eReaderState         = BVDC_P_State_eInactive;
    pNewInfo->eWriterState         = BVDC_P_State_eInactive;

    /* Where on the canvas */
    pNewInfo->stDstRect.lLeft    = 0;
    pNewInfo->stDstRect.lLeft_R  = 0;
    pNewInfo->stDstRect.lTop     = 0;
    pNewInfo->stDstRect.ulWidth  = hWindow->hCompositor->stCurInfo.pFmtInfo->ulWidth;
    pNewInfo->stDstRect.ulHeight = hWindow->hCompositor->stCurInfo.pFmtInfo->ulHeight;

    pNewInfo->lRWinXOffsetDelta  = 0;

    /* Where on the scaler's output. */
    pNewInfo->stScalerOutput     = pNewInfo->stDstRect;

    /* AFD */
    pNewInfo->stAfdSettings.eMode = BVDC_AfdMode_eDisabled;
    pNewInfo->stAfdSettings.eClip = BVDC_AfdClip_eNominal;
    pNewInfo->stAfdSettings.ulAfd = 0;

    /* Pan Scan */
    pNewInfo->ePanScanType     = BVDC_PanScanType_eDisable;
    pNewInfo->lUserHorzPanScan = 0;
    pNewInfo->lUserVertPanScan = 0;

    /* Aspect Ratio Correct */
    pNewInfo->eAspectRatioMode       = BVDC_AspectRatioMode_eBypass;
    pNewInfo->ulNonlinearSrcWidth    = 0;
    pNewInfo->ulNonlinearSclOutWidth = 0;

    /* scale factor rounding */
    pNewInfo->ulHrzSclFctRndToler = 0;
    pNewInfo->ulVrtSclFctRndToler = 0;

    /* Scaler configuration */
    pNewInfo->stSclSettings.bSclVertDejagging       = true;
    pNewInfo->stSclSettings.bSclHorzLumaDeringing   = true;
    pNewInfo->stSclSettings.bSclVertLumaDeringing   = true;
    pNewInfo->stSclSettings.bSclHorzChromaDeringing = true;
    pNewInfo->stSclSettings.bSclVertChromaDeringing = true;
    pNewInfo->stSclSettings.bSclVertPhaseIgnore     = false;
    pNewInfo->stSclSettings.ulSclDejaggingHorz      = 4;
    pNewInfo->stSclSettings.ulSclDejaggingGain      = 2;
    pNewInfo->stSclSettings.ulSclDejaggingCore      = 0;

    /* Sharpness */
    pNewInfo->bSharpnessEnable       = false;
    pNewInfo->bUserSharpnessConfig   = false;

    /* Mosaic mode */
    pNewInfo->bClearRect            = false;
    pNewInfo->bMosaicMode           = false;
    pNewInfo->bClearRectByMaskColor = true;
    pNewInfo->ulClearRectAlpha      = 255;
    BPXL_ConvertPixel_RGBtoYCbCr(BPXL_eA8_Y8_Cb8_Cr8, BPXL_eA8_R8_G8_B8,
        BPXL_MAKE_PIXEL(BPXL_eA8_R8_G8_B8, 0x00, 255, 255, 255),
        (unsigned int*)&pNewInfo->ulMaskColorYCrCb);

    /* Demo mode */
    pNewInfo->stSplitScreenSetting.eHue             = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eContrast        = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eAutoFlesh       = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eBrightness      = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eColorTemp       = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eSharpness       = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eBlueBoost       = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eGreenBoost      = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eCms             = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eContrastStretch = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eBlueStretch     = BVDC_SplitScreenMode_eDisable;
    pNewInfo->stSplitScreenSetting.eFgt             = BVDC_SplitScreenMode_eDisable;

    /* box detect */
    pNewInfo->bBoxDetect = false;
    pNewInfo->bAutoCutBlack = false;
    pNewInfo->BoxDetectCallBack = NULL;
    pNewInfo->pvBoxDetectParm1 = NULL;
    pNewInfo->iBoxDetectParm2 = 0;

    /* Init mad default settings */
    BVDC_P_Mvp_Init_Default(
        &pNewInfo->stMadSettings.eGameMode,
        &pNewInfo->stMadSettings.ePixelFmt,
        &pNewInfo->stMadSettings.ePqEnhancement,
        &pNewInfo->stMadSettings.bShrinkWidth,
        &pNewInfo->stMadSettings.bReverse32Pulldown,
        &pNewInfo->stMadSettings.bReverse22Pulldown,
        &pNewInfo->stMadSettings.stChromaSettings,
        &pNewInfo->stMadSettings.stMotionSettings);

    /* Init mad custom settings */
    BVDC_P_Mvp_Init_Custom(
        &pNewInfo->stMadSettings.stUpSampler,
        &pNewInfo->stMadSettings.stDnSampler,
        &pNewInfo->stMadSettings.stLowAngles);

    pNewInfo->bChromaCustom = false;
    pNewInfo->bMotionCustom = false;
    pNewInfo->bRev32Custom  = false;
    pNewInfo->bRev22Custom  = false;

    /* init ANR default settings */
    /* only vwin0 has ANR as default; */
    pNewInfo->bAnr                  = false;
    pNewInfo->stAnrSettings.eMode   = BVDC_FilterMode_eDisable;
    pNewInfo->stAnrSettings.ePxlFormat = BVDC_P_CAP_PIXEL_FORMAT_8BIT422;

    /* Color adjustment attributes */
    pNewInfo->sContrast             = 0;
    pNewInfo->sSaturation           = 0;
    pNewInfo->sHue                  = 0;
    pNewInfo->sBrightness           = 0;
    pNewInfo->sSharpness            = 0;
    pNewInfo->sColorTemp            = 0;
    pNewInfo->lAttenuationR         = BVDC_P_CFC_ITOFIX(1);
    pNewInfo->lAttenuationG         = BVDC_P_CFC_ITOFIX(1);
    pNewInfo->lAttenuationB         = BVDC_P_CFC_ITOFIX(1);
    pNewInfo->lOffsetR              = 0;
    pNewInfo->lOffsetG              = 0;
    pNewInfo->lOffsetB              = 0;

    pNewInfo->sHdrPeakBrightness    = BVDC_P_TCH_DEFAULT_HDR_BRIGHTNESS;
    pNewInfo->sSdrPeakBrightness    = BVDC_P_TCH_DEFAULT_SDR_BRIGHTNESS;

    /* Color key attributes */
    pNewInfo->stColorKey.bLumaKey            = false;
    pNewInfo->stColorKey.ucLumaKeyMask       = 0;
    pNewInfo->stColorKey.ucLumaKeyHigh       = 0xff;
    pNewInfo->stColorKey.ucLumaKeyLow        = 0;
    pNewInfo->stColorKey.bChromaRedKey       = false;
    pNewInfo->stColorKey.ucChromaRedKeyMask  = 0;
    pNewInfo->stColorKey.ucChromaRedKeyHigh  = 0xff;
    pNewInfo->stColorKey.ucChromaRedKeyLow   = 0;
    pNewInfo->stColorKey.bChromaBlueKey      = false;
    pNewInfo->stColorKey.ucChromaBlueKeyMask = 0;
    pNewInfo->stColorKey.ucChromaBlueKeyHigh = 0xff;
    pNewInfo->stColorKey.ucChromaBlueKeyLow  = 0;

    /* User capture buffers */
    pNewInfo->uiCaptureBufCnt       = 0;
    pNewInfo->pBufferFromUser       = NULL;

    /* Set default pixel format of buffer */
#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_LITTLE)
    pNewInfo->ePixelFormat          = BVDC_P_CAP_PIXEL_FORMAT_8BIT422;
#else
    pNewInfo->ePixelFormat          = BVDC_P_CAP_PIXEL_FORMAT_8BIT422_BE;
#endif

    /* Luma avg rect set */
    pNewInfo->stLumaRect.stRegion.ulLeft   = 0;
    pNewInfo->stLumaRect.stRegion.ulTop    = 0;
    pNewInfo->stLumaRect.stRegion.ulRight  = 0;
    pNewInfo->stLumaRect.stRegion.ulBottom = 0;
    pNewInfo->stLumaRect.aulLevelThres[0]  =  100;
    pNewInfo->stLumaRect.aulLevelThres[1]  =  500;
    pNewInfo->stLumaRect.aulLevelThres[2]  = 9000;
    pNewInfo->stLumaRect.aulLevelThres[3]  = 9500;
    pNewInfo->stLumaRect.eNumBins = BVDC_HistBinSelect_e16_Bins;

    /* Pulldown capture */
    hWindow->bDoPulldown     = false;
    hWindow->bFrameCapture   = false;

    /* Callback setting (from user) */
    pNewInfo->stCbSettings.ulLipSyncTolerance = BVDC_P_LIP_SYNC_TOLERANCE;
    pNewInfo->stCbSettings.ulGameModeReadWritePhaseDiff = BVDC_P_LIP_SYNC_TOLERANCE;
    pNewInfo->stCbSettings.ulGameModeTolerance = BVDC_P_LIP_SYNC_RESET_DELAY;

    /* CbData internal window data */
    hWindow->pCurWriterNode  = NULL;
    hWindow->pCurReaderNode  = NULL;
    hWindow->bRepeatCurrReader = false;
    hWindow->stCbData.ulVsyncDelay = BVDC_P_LIP_SYNC_RESET_DELAY;
    hWindow->stCbData.ulDriftDelay = BVDC_P_LIP_SYNC_RESET_DELAY;
    hWindow->stCbData.ulGameModeDelay = BVDC_P_LIP_SYNC_RESET_DELAY;
    hWindow->stCbData.stOutputRect.lLeft    = pNewInfo->stDstRect.lLeft;
    hWindow->stCbData.stOutputRect.lTop     = pNewInfo->stDstRect.lTop;
    hWindow->stCbData.stOutputRect.ulWidth  = pNewInfo->stDstRect.ulWidth;
    hWindow->stCbData.stOutputRect.ulHeight = pNewInfo->stDstRect.ulHeight;
    hWindow->stCbData.stMask.bVsyncDelay = BVDC_P_DIRTY;
    hWindow->stCbData.stMask.bDriftDelay = BVDC_P_DIRTY;
    hWindow->stCbData.stMask.bRectAdjust = BVDC_P_DIRTY;
    hWindow->stCbData.stMask.bSyncLock   = BVDC_P_DIRTY;
    hWindow->stCbData.stMask.bGameModeDelay = BVDC_P_DIRTY;
    hWindow->stCbData.stMask.bCrc = BVDC_P_CLEAN;

    /* Game mode delay setting from user */
    pNewInfo->stGameDelaySetting.bEnable           = false;
    pNewInfo->stGameDelaySetting.bForceCoarseTrack = false;
    pNewInfo->stGameDelaySetting.ulBufferDelayTarget = BVDC_P_LIP_SYNC_RESET_DELAY;
    pNewInfo->stGameDelaySetting.ulBufferDelayTolerance = BVDC_P_LIP_SYNC_RESET_DELAY;

    /* Game data internal data */
    hWindow->ulCurBufDelay     = 0;
    hWindow->lCurGameModeLag   = 0;
    hWindow->bAdjGameModeClock = false;
    hWindow->bCoarseAdjClock   = false;
    hWindow->bFastAdjClock     = false;

    /* PLATFORMS: Specifics override that either take adavantage of bvn doc
     * assumptions, BOX modes or otherwise default to support ref rts.  Otherwise can
     * be overwritten by public API. */
    /* Check if destination rectangle is bigger than BOX limits */
    BVDC_P_Window_Rts_Init(
        BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg),
        BVDC_P_DISPLAY_NRT_STG(hWindow->hCompositor->hDisplay),
        &pNewInfo->bForceCapture, &pNewInfo->eSclCapBias, &pNewInfo->ulBandwidthDelta);

    pBoxVdcCap     = &hWindow->hCompositor->hVdc->stBoxConfig.stVdc;
    pBoxVdcDispCap = &pBoxVdcCap->astDisplay[hWindow->hCompositor->eId];

    ulBoxWinId = BVDC_P_GetBoxWindowId_isrsafe(hWindow->eId);
    eDisplayId = hWindow->hCompositor->hDisplay->eId;

    hWindow->bSrcSideDeinterlace = pBoxVdcDispCap->astWindow[ulBoxWinId].stResource.bSrcSideDeinterlacer;
    hWindow->eBoxSclCapBias = pBoxVdcDispCap->astWindow[ulBoxWinId].eSclCapBias;

    hWindow->eWindowClass = pBoxVdcCap->astDisplay[eDisplayId].astWindow[ulBoxWinId].eClass;
    if(hWindow->eWindowClass != BBOX_Vdc_WindowClass_eLegacy)
        hWindow->pWindowClassLimit = &hWindow->hCompositor->hDisplay->hVdc->pstWinClassTbl[hWindow->eWindowClass];
    else
        hWindow->pWindowClassLimit = NULL;

    if(hWindow->stResourceRequire.bRequireScaler)
    {
        BVDC_P_Scaler_Init_isr(hWindow->stCurResource.hScaler, hWindow);
    }

    if(hWindow->stResourceRequire.bRequirePlayback)
    {
        BVDC_P_Feeder_Init(hWindow->stCurResource.hPlayback, hWindow, false, false);
    }

    if(hWindow->stResourceRequire.bRequireCapture)
    {
        BVDC_P_Buffer_Init(hWindow->hBuffer);
        BVDC_P_Capture_Init(hWindow->stCurResource.hCapture, hWindow);
        hWindow->pCurWriterNode =
            BVDC_P_Buffer_GetCurWriterNode_isr(hWindow->hBuffer);
        hWindow->pCurReaderNode =
            BVDC_P_Buffer_GetCurReaderNode_isr(hWindow->hBuffer);
    }

    if(hWindow->stResourceRequire.bRequirePep)
    {
        BVDC_P_Pep_Init(hWindow->stCurResource.hPep);
    }

    /* Init compression settings. Needs hWindow->bIs10BitCore, call
     * after BVDC_P_Scaler_Init_isr */
    BVDC_P_Window_Compression_Init(hWindow->bIs10BitCore, hWindow->bSupportDcxm,
        &hWindow->stCapCompression, NULL, BVDC_P_Mvp_Dcxs);

    /* Clear out user's states. */
    BKNI_Memcpy(pCurInfo, pNewInfo, sizeof(BVDC_P_Window_Info));

    /* Default CFC */
    BVDC_P_Window_InitCfcs(hWindow);

    return;
}

static void BVDC_P_Window_GetCoverageInfo
    ( const BVDC_Window_Handle         hWindow,
      BVDC_P_Window_CoverageInfo      *pCoverageInfo )
{
    uint32_t    i, ulMosaicCount;
    int32_t     lLeft, lRight, lTop, lBottom;
    BVDC_P_Window_CoverageInfo   stCoverageInfo;

    BKNI_Memset((void*)&stCoverageInfo, 0x0, sizeof(BVDC_P_Window_CoverageInfo));
    ulMosaicCount = hWindow->stNewInfo.ulMaxMosaicCount;

    lLeft = hWindow->stNewInfo.astMosaicRect[0].lLeft;
    lTop = hWindow->stNewInfo.astMosaicRect[0].lTop;
    lRight = lLeft + hWindow->stNewInfo.astMosaicRect[0].ulWidth;
    lBottom = lTop + hWindow->stNewInfo.astMosaicRect[0].ulHeight;
    stCoverageInfo.ulMaxMosaicRectWidth = hWindow->stNewInfo.astMosaicRect[0].ulWidth;

    for(i = 1; i < ulMosaicCount; i++)
    {
        lLeft = BVDC_P_MIN(lLeft, hWindow->stNewInfo.astMosaicRect[i].lLeft);
        lTop = BVDC_P_MIN(lTop,hWindow->stNewInfo.astMosaicRect[i].lTop);
        lRight = BVDC_P_MAX(lRight, (int32_t)(hWindow->stNewInfo.astMosaicRect[i].lLeft +
            hWindow->stNewInfo.astMosaicRect[i].ulWidth));
        lBottom = BVDC_P_MAX(lBottom, (int32_t)(hWindow->stNewInfo.astMosaicRect[i].lTop +
            hWindow->stNewInfo.astMosaicRect[i].ulHeight));

        stCoverageInfo.ulMaxMosaicRectWidth = BVDC_P_MAX(stCoverageInfo.ulMaxMosaicRectWidth,
            hWindow->stNewInfo.astMosaicRect[i].ulWidth);
    }
   stCoverageInfo.stBoundingBox.lLeft = lLeft;
   stCoverageInfo.stBoundingBox.lTop = lTop;
   stCoverageInfo.stBoundingBox.ulWidth = lRight - lLeft;
   stCoverageInfo.stBoundingBox.ulHeight = lBottom - lTop;

    if(pCoverageInfo)
        *pCoverageInfo = stCoverageInfo;
}

static BERR_Code BVDC_P_Window_ValidateBoundingBox
    ( const BVDC_Window_Handle         hWindow,
      const BVDC_WindowClassLimits    *pWindowClassLimit,
      uint32_t                         ulBoxMaxFmtWidth,
      uint32_t                         ulBoxMaxFmtHeight,
      BVDC_P_Window_CoverageInfo      *pCoverageInfo )
{
    uint32_t    i, j, ulMosaicCount;
    uint32_t    ulRTSMaxWidth, ulRTSMaxHeight;

    pCoverageInfo->bInsideBoundingBox = false;
    ulMosaicCount = hWindow->stNewInfo.ulMaxMosaicCount;
    for(i = 0; i < BVDC_NUM_BOUNDING_BOXES; i++)
    {
        if(pWindowClassLimit->boundingBox[i].ulPercentW ||
           pWindowClassLimit->boundingBox[i].ulPercentH)
        {
            ulRTSMaxWidth = (ulBoxMaxFmtWidth * pWindowClassLimit->boundingBox[i].ulPercentW) / 100;
            ulRTSMaxHeight = (ulBoxMaxFmtHeight * pWindowClassLimit->boundingBox[i].ulPercentH) / 100;

            if(!pWindowClassLimit->boundingBox[i].bOutside)
            {
                /* inside */
                if((pCoverageInfo->stBoundingBox.ulWidth <= ulRTSMaxWidth) &&
                   (pCoverageInfo->stBoundingBox.ulHeight <= ulRTSMaxHeight))
                {
                    pCoverageInfo->bInsideBoundingBox = true;
                    return BERR_SUCCESS;
                }
            }
            else
            {
                /* TODO: check outside  for non-mosaic live at isr time */
                if(ulMosaicCount)
                {
                    for(j = 0; j < ulMosaicCount; j++)
                    {
                        if((hWindow->stNewInfo.astMosaicRect[j].ulWidth >= ulRTSMaxWidth) &&
                           (hWindow->stNewInfo.astMosaicRect[j].ulHeight >= ulRTSMaxHeight))
                        {
                            pCoverageInfo->bInsideBoundingBox = false;
                            return BERR_SUCCESS;
                        }
                    }
                }
                else
                {
                    if((hWindow->stNewInfo.stDstRect.ulWidth >= ulRTSMaxWidth) &&
                       (hWindow->stNewInfo.stDstRect.ulHeight >= ulRTSMaxHeight))
                    {
                        pCoverageInfo->bInsideBoundingBox = false;
                        return BERR_SUCCESS;
                    }
                }
            }
        }
    }

    BDBG_ERR(("============================================================"));
    BDBG_ERR(("Win[%d] Boxmode[%d] RTS violation: BoundingBox",
        hWindow->eId, hWindow->hCompositor->hVdc->stBoxConfig.stBox.ulBoxId));
    BDBG_ERR(("MaxFmt: %dx%d, count %d", ulBoxMaxFmtWidth, ulBoxMaxFmtHeight,
        ulMosaicCount));
    for(i = 0; i < BVDC_NUM_BOUNDING_BOXES; i++)
    {
        if(pWindowClassLimit->boundingBox[i].ulPercentW ||
           pWindowClassLimit->boundingBox[i].ulPercentH)
        {
            BDBG_ERR(("RTS Limit BoundingBox[%d] %dx%d(%dx%d), bOutside: %d",
                i,
                (ulBoxMaxFmtWidth * pWindowClassLimit->boundingBox[i].ulPercentW) / 100,
                (ulBoxMaxFmtHeight * pWindowClassLimit->boundingBox[i].ulPercentH) / 100,
                pWindowClassLimit->boundingBox[i].ulPercentW,
                pWindowClassLimit->boundingBox[i].ulPercentH,
                pWindowClassLimit->boundingBox[i].bOutside));
        }
    }
    BDBG_ERR(("MosaicRect bounding box: %dx%d",
        pCoverageInfo->stBoundingBox.ulWidth,
        pCoverageInfo->stBoundingBox.ulHeight));
    for(i = 0; i < ulMosaicCount; i++)
    {
        BDBG_ERR(("MosaicRect[%d]: %dx%d",
            i, hWindow->stNewInfo.astMosaicRect[i].ulWidth,
            hWindow->stNewInfo.astMosaicRect[i].ulHeight));
    }
    BDBG_ERR(("============================================================"));

    return BVDC_ERR_INVALID_MOSAIC_MODE;
}

static BERR_Code BVDC_P_Window_ValidateMosaicRectSize
    ( const BVDC_Window_Handle         hWindow,
      const BVDC_WindowClassLimits    *pWindowClassLimit,
      uint32_t                         ulBoxMaxFmtWidth,
      uint32_t                         ulBoxMaxFmtHeight,
      BVDC_P_Window_CoverageInfo      *pCoverageInfo )
{
    bool        bValid = true;
    uint32_t    i, ulIndex;
    uint32_t    ulRTSMaxWidthEqual, ulRTSMaxHeightEqual;
    uint32_t    ulRTSMaxWidthSmall, ulRTSMaxHeightSmall;

    ulIndex = hWindow->stNewInfo.bMosaicMode
        ? hWindow->stNewInfo.ulMaxMosaicCount - 1 : 0;

    /* Check mosaic rectangles for ulPercentEqual */
    ulRTSMaxWidthEqual = (ulBoxMaxFmtWidth * pWindowClassLimit->mosaicRects[ulIndex].ulPercentEqual) / 100;
    ulRTSMaxHeightEqual = (ulBoxMaxFmtHeight * pWindowClassLimit->mosaicRects[ulIndex].ulPercentEqual) / 100;
    for(i = 0; i <= ulIndex; i++)
    {
        if((hWindow->stNewInfo.astMosaicRect[i].ulWidth > ulRTSMaxWidthEqual) ||
           (hWindow->stNewInfo.astMosaicRect[i].ulHeight > ulRTSMaxHeightEqual))
        {
            bValid = false;
            break;
        }
    }

    if(bValid)
    {
        return BERR_SUCCESS;
    }

    /* Does not meet ulPercentEqual limit, check ulPercentBigSmall */
    bValid = true;
    ulRTSMaxWidthSmall = (ulBoxMaxFmtWidth * pWindowClassLimit->mosaicRects[ulIndex].ulPercentBigSmall) / 100;
    ulRTSMaxHeightSmall = (ulBoxMaxFmtHeight * pWindowClassLimit->mosaicRects[ulIndex].ulPercentBigSmall) / 100;
    for(i = 0; i <= ulIndex; i++)
    {
        if(hWindow->stNewInfo.astMosaicRect[i].ulWidth == pCoverageInfo->ulMaxMosaicRectWidth)
        {
            /* Big rect */
            if((hWindow->stNewInfo.astMosaicRect[i].ulWidth > 2*ulRTSMaxWidthSmall) ||
               (hWindow->stNewInfo.astMosaicRect[i].ulHeight > 2*ulRTSMaxHeightSmall))
            {
                bValid = false;
                break;
            }
        }
        else
        {
            if((hWindow->stNewInfo.astMosaicRect[i].ulWidth > ulRTSMaxWidthSmall) ||
               (hWindow->stNewInfo.astMosaicRect[i].ulHeight > ulRTSMaxHeightSmall))
            {
                bValid = false;
                break;
            }
        }
    }


    if(bValid)
    {
        return BERR_SUCCESS;
    }
    else
    {
        BDBG_ERR(("============================================================"));
        BDBG_ERR(("Win[%d] Boxmode[%d] RTS violation: Canvas coverage",
            hWindow->eId, hWindow->hCompositor->hVdc->stBoxConfig.stBox.ulBoxId));
        BDBG_ERR(("MaxFmt: %dx%d, count %d", ulBoxMaxFmtWidth, ulBoxMaxFmtHeight,
            ulIndex+1));
        BDBG_ERR(("RTS Limit ulPercentEqual: %dx%d(%d), ulPercentBigSmall: %dx%d(%d)",
            ulRTSMaxWidthEqual, ulRTSMaxHeightEqual,
            pWindowClassLimit->mosaicRects[ulIndex].ulPercentEqual,
            ulRTSMaxWidthSmall, ulRTSMaxHeightSmall,
            pWindowClassLimit->mosaicRects[ulIndex].ulPercentBigSmall));
        for(i = 0; i <= ulIndex; i++)
        {
            BDBG_ERR(("MosaicRect[%d]: %dx%d",
                i, hWindow->stNewInfo.astMosaicRect[i].ulWidth,
                hWindow->stNewInfo.astMosaicRect[i].ulHeight));
        }
        BDBG_ERR(("============================================================"));
        return BVDC_ERR_INVALID_MOSAIC_MODE;
    }
}

/***************************************************************************
 * {private}
 *
 * Validate coverage for each mosaic rectangle:
 *   - (Rect_Width[i] / Rect_Height[i]) > (0.95)
 *   - (Rect_Width[i] * Rect_Height[i]) < ((x%* Canvas_Width * Canvas_height) / N)
 */
static BERR_Code BVDC_P_Window_ValidateLegacyMosaicCoverage
    ( const BVDC_Window_Handle         hWindow )
{
    uint32_t i, ulMosaicCount = hWindow->stNewInfo.ulMaxMosaicCount;
    uint32_t ulWidth, ulHeight, ulCoverage;
    uint32_t ulCanvasArea, ulMaxRectArea;
    BVDC_DisplayId  eDisplayId = hWindow->hCompositor->hDisplay->eId;
    BBOX_Config    *pBoxConfig;
    BFMT_VideoFmt   eBoxmodeMaxVideoFmt;
    BBOX_Vdc_MosaicModeClass   eMosaicModeClass;
    BVDC_P_MosaicCanvasCoverage     *pCoverageTbl;

    if(!hWindow->stNewInfo.bMosaicMode)
        return BERR_SUCCESS;

    /* Get boxmode mosaic mode class */
    pBoxConfig = &hWindow->hCompositor->hVdc->stBoxConfig;
    eMosaicModeClass = pBoxConfig->stVdc.astDisplay[eDisplayId].eMosaicModeClass;
    /* Get boxmode max format */
    switch(eMosaicModeClass)
    {
        case BBOX_Vdc_MosaicModeClass_eClass0:
            eBoxmodeMaxVideoFmt = BFMT_VideoFmt_eNTSC;
            break;
        case BBOX_Vdc_MosaicModeClass_eClass1:
            eBoxmodeMaxVideoFmt = BFMT_VideoFmt_e1080p;
            break;
        case BBOX_Vdc_MosaicModeClass_eClass2:
        case BBOX_Vdc_MosaicModeClass_eClass3:
        case BBOX_Vdc_MosaicModeClass_eClass4:
        default:
            eBoxmodeMaxVideoFmt = BFMT_VideoFmt_e4096x2160p_60Hz;
            break;
    }

    pCoverageTbl = &hWindow->hCompositor->hVdc->stMosaicCoverageTbl[eDisplayId];
    ulCoverage = pCoverageTbl->aulCanvasCoverageEqual[ulMosaicCount-1];

    if( BFMT_IS_4kx2k(eBoxmodeMaxVideoFmt) &&
       !BFMT_IS_4kx2k(hWindow->hCompositor->stNewInfo.pFmtInfo->eVideoFmt))
    {
            ulCoverage *= BVDC_P_4K_TO_1080P_NORM_FACTOR;
    }
    else if(!BFMT_IS_4kx2k(eBoxmodeMaxVideoFmt) &&
             BFMT_IS_4kx2k(hWindow->hCompositor->stNewInfo.pFmtInfo->eVideoFmt))
    {
            ulCoverage /= BVDC_P_4K_TO_1080P_NORM_FACTOR;
    }
    BDBG_MODULE_MSG(BVDC_WIN_COV,("Win[%d] Coverage: %d, Mosaic Count: %d", hWindow->eId,
        ulCoverage, ulMosaicCount));

    ulCanvasArea = hWindow->hCompositor->stNewInfo.pFmtInfo->ulWidth *
        hWindow->hCompositor->stNewInfo.pFmtInfo->ulHeight;

    for(i = 0; i < ulMosaicCount; i++)
    {
        if(hWindow->stNewInfo.bMosaicMode)
        {
            ulWidth = (i < hWindow->stNewInfo.ulMosaicCount)?
                hWindow->stNewInfo.astMosaicRect[i].ulWidth:0;
            ulHeight = (i < hWindow->stNewInfo.ulMosaicCount)?
                hWindow->stNewInfo.astMosaicRect[i].ulHeight:0;
        }
        else
        {
            ulWidth  = hWindow->stNewInfo.stDstRect.ulWidth;
            ulHeight = hWindow->stNewInfo.stDstRect.ulHeight;
        }
        if((!ulWidth) ||(!ulHeight)) break;

        /* Each Rect w/h must be > 0.95 */
        if(BVDC_P_WIDTH_HEIGHT_RATIO_PERCENTAGE(ulWidth, ulHeight) <= BVDC_P_MIN_WH_RATIO_PERCENTAGE)
        {
            BDBG_ERR(("============================================================"));
            BDBG_ERR(("Boxmode[%d] RTS violation: w/h needs to be larger than 95%%",
                hWindow->hCompositor->hVdc->stBoxConfig.stBox.ulBoxId));
            BDBG_ERR(("Win[%d] Mosaic Rect[%d] Invalid size ratio: %dx%d",
                hWindow->eId, i, ulWidth, ulHeight));
            BDBG_ERR(("============================================================"));
        }

        /* Each Rect w*h must be < (x%*canvasW*canvasH) / N */
        ulMaxRectArea = (ulCanvasArea * ulCoverage) / (BVDC_P_PERCENTAGE_FACTOR*ulMosaicCount);
        if( (ulWidth * ulHeight) > ulMaxRectArea)
        {
            BDBG_ERR(("========================================================================"));
            BDBG_ERR(("Boxmode[%d] RTS violation: Canvas coverage needs to be smaller than %d%%",
                hWindow->hCompositor->hVdc->stBoxConfig.stBox.ulBoxId, ulCoverage));
            BDBG_ERR(("Win[%d] Mosaic Rect[%d](%dx%d) exceeds %d, Mosaic count: %d",
                hWindow->eId, i, ulWidth, ulHeight, ulMaxRectArea, ulMosaicCount));
            BDBG_ERR(("========================================================================"));
        }
    }

    return BERR_SUCCESS;
}

/***************************************************************************
 * {private}
 *
 * Validate coverage for each mosaic rectangle:
 */
static BERR_Code BVDC_P_Window_ValidateCoverage
    ( const BVDC_Window_Handle         hWindow )
{
    uint32_t                 ulWinId;
    uint32_t                 ulBoxMaxFmtWidth, ulBoxMaxFmtHeight;
    BBOX_Config             *pBoxConfig;
    BFMT_VideoFmt            eBoxmodeMaxVideoFmt;
    BVDC_DisplayId           eDisplayId;
    const BFMT_VideoInfo    *pBoxMaxFmtInfo;
    BERR_Code                eStatus = BERR_SUCCESS;

    ulWinId = BVDC_P_WIN_IS_V0(hWindow->eId) ? 0 : 1;
    eDisplayId = hWindow->hCompositor->hDisplay->eId;

    /* Get boxmode mosaic mode class */
    pBoxConfig = &hWindow->hCompositor->hVdc->stBoxConfig;

    if(!pBoxConfig->stVdc.astDisplay[eDisplayId].astWindow[ulWinId].bAvailable)
        return BERR_SUCCESS;

    if(hWindow->eWindowClass == BBOX_VDC_DISREGARD)
    {
        return BERR_SUCCESS;
    }

    if(hWindow->eWindowClass == BBOX_Vdc_WindowClass_eLegacy)
    {
        /* Class 0 is for legacy support */
        return BVDC_P_Window_ValidateLegacyMosaicCoverage(hWindow);
    }

    /* Non-legacy class */
    BDBG_ASSERT(hWindow->eWindowClass != BBOX_Vdc_WindowClass_eLegacy);
    BDBG_ASSERT(hWindow->pWindowClassLimit);
    eBoxmodeMaxVideoFmt = pBoxConfig->stVdc.astDisplay[eDisplayId].eMaxVideoFmt;
    pBoxMaxFmtInfo = BFMT_GetVideoFormatInfoPtr(eBoxmodeMaxVideoFmt);

    BDBG_ASSERT(pBoxMaxFmtInfo);
    ulBoxMaxFmtWidth  = pBoxMaxFmtInfo->ulWidth;
    ulBoxMaxFmtHeight = pBoxMaxFmtInfo->ulHeight;

    BDBG_MODULE_MSG(BVDC_WIN_COV,("Win[%d] windowclass %d, Boxmode Max size: %dx%d",
        hWindow->eId, hWindow->eWindowClass, ulBoxMaxFmtWidth, ulBoxMaxFmtHeight));

    /* Get bounding box */
    BVDC_P_Window_GetCoverageInfo(hWindow, &hWindow->stCoverageInfo);

    /* Check bounding box */
    eStatus = BVDC_P_Window_ValidateBoundingBox(hWindow, hWindow->pWindowClassLimit,
        ulBoxMaxFmtWidth, ulBoxMaxFmtHeight, &hWindow->stCoverageInfo);
    if(eStatus != BERR_SUCCESS)
        return BERR_TRACE(eStatus);

    /* Check mosaic rect size */
    eStatus = BVDC_P_Window_ValidateMosaicRectSize(hWindow, hWindow->pWindowClassLimit,
        ulBoxMaxFmtWidth, ulBoxMaxFmtHeight, &hWindow->stCoverageInfo);
    if(eStatus != BERR_SUCCESS)
        return BERR_TRACE(eStatus);

    return BERR_SUCCESS;
}

#if BVDC_P_SUPPORT_MOSAIC_MODE
/***************************************************************************
 * {private}
 *
 * This adjusts window mosaic rect size and offset to comply with workarounds.
 */
static void BVDC_P_Window_AdjustMosaicRect_isrsafe
    ( const BVDC_Window_Handle         hWindow,
      bool                             bInterlaced,
      BVDC_P_Rect                     *pRect)
{
#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM)
#if (BVDC_P_DCXM_RECT_WORKAROUND)
    /* CRBVN-282: Both offset and width need to be multiple of 4 */
    if((hWindow->bSupportDcxm) &&
       ((pRect->ulWidth % 4) || (pRect->lLeft % 4) || (pRect->lLeft_R % 4)))
    {
        /* Round down to make sure both offset and widht are multiple
         * of 4. Trade off is will result in a slightly different
         * window location in the future chips */
        pRect->lLeft   = BVDC_P_ALIGN_DN(pRect->lLeft, 4);
        pRect->lLeft_R = BVDC_P_ALIGN_DN(pRect->lLeft_R, 4);
        pRect->ulWidth = BVDC_P_ALIGN_DN(pRect->ulWidth, 4);
    }
#endif
    if((hWindow->bSupportDcxm) &&
       ((pRect->ulWidth < BVDC_P_WIN_CAP_MOSAIC_INPUT_H_MIN)||
        (pRect->ulHeight < (BVDC_P_WIN_CAP_MOSAIC_INPUT_V_MIN * (bInterlaced?2:1)))))
    {
        pRect->ulWidth =
            BVDC_P_MAX(pRect->ulWidth, BVDC_P_WIN_CAP_MOSAIC_INPUT_H_MIN);
        pRect->ulHeight=
            BVDC_P_MAX(pRect->ulHeight, BVDC_P_WIN_CAP_MOSAIC_INPUT_V_MIN * (bInterlaced?2:1));
    }
#else
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(bInterlaced);
#endif

    /* 4:2:2 BVB round down the pixel position and width to even number */
    if((pRect->lLeft & 1) || (pRect->lLeft_R & 1) || (pRect->ulWidth & 1))
    {
        BDBG_MODULE_MSG(BVDC_DCXM_PICSIZE,("left=%d, left_R=%d, width=%u; to round down to even pixels:",
            pRect->lLeft, pRect->lLeft_R, pRect->ulWidth));
        pRect->lLeft   = BVDC_P_ALIGN_DN(pRect->lLeft, 2);
        pRect->lLeft_R = BVDC_P_ALIGN_DN(pRect->lLeft_R, 2);
        pRect->ulWidth = BVDC_P_ALIGN_DN(pRect->ulWidth, 2);
        BDBG_MODULE_MSG(BVDC_DCXM_PICSIZE,("left=%d, left_R=%d, width=%u now!",
            pRect->lLeft, pRect->lLeft_R, pRect->ulWidth));
    }
}
#endif

/***************************************************************************
 * {private}
 *
 * This should contains all the information to detect user error settings.
 * User settings that required checking happen here.
 */
BERR_Code BVDC_P_Window_ValidateChanges
    ( const BVDC_Window_Handle         hWindow,
      const BFMT_VideoInfo            *pDstFormatInfo )
{
    BVDC_P_Resource_Handle hResource;
    BVDC_Compositor_Handle hCompositor;
    BVDC_Display_Handle hDisplay;
    BVDC_P_Window_Info *pNewInfo;
    const BVDC_P_Window_Info *pCurInfo;
    BVDC_P_Window_DirtyBits *pNewDirty;
#if (BVDC_P_SUPPORT_HIST)
    const BVDC_ClipRect *pNewRect, *pCurRect;
#endif
    uint32_t ulMinV;
    BVDC_DisplayTg eMasterTg;
    bool bDtg;
    uint32_t ulHsize, ulVsize;
    const BBOX_Vdc_Capabilities *pBoxVdc;
    uint32_t ulBoxWinId;
    BVDC_DisplayId eDisplayId;
    BVDC_P_WindowId eWindowId;
    BDBG_ENTER(BVDC_P_Window_ValidateChanges);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stNewInfo.hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor->hDisplay, BVDC_DSP);
    eWindowId = hWindow->eId;
    hCompositor = hWindow->hCompositor;
    hDisplay = hCompositor->hDisplay;
    hResource = hCompositor->hVdc->hResource;
    eDisplayId = hDisplay->eId;
    eMasterTg = hDisplay->eMasterTg;
    bDtg      = BVDC_P_DISPLAY_USED_DIGTRIG (eMasterTg);
    ulHsize = bDtg? pDstFormatInfo->ulDigitalWidth : pDstFormatInfo->ulWidth;
    ulVsize = bDtg? pDstFormatInfo->ulDigitalHeight: pDstFormatInfo->ulHeight;

    /* Handle 3d case */
    if(!BFMT_IS_3D_MODE(hCompositor->stNewInfo.pFmtInfo->eVideoFmt))
    {
        if(hCompositor->stNewInfo.eOrientation == BFMT_Orientation_e3D_LeftRight)
        {
            ulHsize = ulHsize / 2;
        }
        else if(hCompositor->stNewInfo.eOrientation == BFMT_Orientation_e3D_OverUnder)
        {
            ulVsize = ulVsize / 2;
        }
    }

    /* To reduce the amount of typing */
    pNewInfo = &hWindow->stNewInfo;
    pCurInfo = &hWindow->stCurInfo;
    pNewDirty = &pNewInfo->stDirty;

    /* if display is digital master, user setting of rectangles of full-screen should
     * match digital size */
    if(bDtg)
    {
        if(pNewInfo->stDstRect.ulHeight == pDstFormatInfo->ulHeight)
        {
            pNewInfo->stDstRect.ulHeight = pDstFormatInfo->ulDigitalHeight;
        }
        if(pNewInfo->stScalerOutput.ulHeight == pDstFormatInfo->ulHeight)
        {
            pNewInfo->stScalerOutput.ulHeight = pDstFormatInfo->ulDigitalHeight;
        }
    }

    /* SW7563-101:if display is dvi master,
     * hdmi output needed to be connected before window open */
    if(BVDC_P_DISPLAY_USED_DVI(eMasterTg) &&
       (hDisplay->stCurInfo.stHdmiSettings.stSettings.eMatrixCoeffs == BAVC_MatrixCoefficients_eUnknown))
    {
        BDBG_ERR(("displaying [%d], hdmi output to be connected before window is created",
            hDisplay->eId));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Check if destination rectangle is bigger than BOX limits */
    pBoxVdc = &hCompositor->hVdc->stBoxConfig.stVdc;
    ulBoxWinId = BVDC_P_GetBoxWindowId_isrsafe(eWindowId);

    BDBG_ASSERT(ulBoxWinId < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY);

    /* override if BOX mode say so */
    if (pBoxVdc->astDisplay[eDisplayId].astWindow[ulBoxWinId].eSclCapBias != BBOX_VDC_DISREGARD)
    {
        if(BBOX_Vdc_SclCapBias_eAutoDisable == pBoxVdc->astDisplay[eDisplayId].astWindow[ulBoxWinId].eSclCapBias)
        {
            /* special case to support Live Feed */
            pNewInfo->bForceCapture = false;
            pNewInfo->eSclCapBias   = BVDC_SclCapBias_eAuto;
            pNewInfo->ulBandwidthDelta = BVDC_P_BW_DEFAULT_DELTA;
        }
        else if(BBOX_Vdc_SclCapBias_eAutoDisable1080p == pBoxVdc->astDisplay[eDisplayId].astWindow[ulBoxWinId].eSclCapBias)
        {
            /* What this new rule means is that when the display format is
             * 1080p size or less (includes 1080p120Hz if that display format
             * is allowed) we will use BBOX_Vdc_SclCapBias_eSclBeforeCap.
             * When the display format is larger than 1080p, we will use
             * BBOX_Vdc_SclCapBias_eAutoDisable. However, if we have live feed
             * and mosaic mode, we capture so the bias will become sclBeforeCap.
             */
            if(((pNewInfo->stDstRect.ulWidth  > BFMT_1080I_WIDTH) ||
                (pNewInfo->stDstRect.ulHeight > BFMT_1080I_HEIGHT)) &&
                !pNewInfo->bMosaicMode)
            {
                /* special case to support Live Feed */
                pNewInfo->bForceCapture = false;
                pNewInfo->eSclCapBias   = BVDC_SclCapBias_eAuto;
                pNewInfo->ulBandwidthDelta = BVDC_P_BW_DEFAULT_DELTA;
            }
            else
            {
                pNewInfo->bForceCapture = true;
                pNewInfo->eSclCapBias   = BVDC_SclCapBias_eSclBeforeCap;
                pNewInfo->ulBandwidthDelta = BVDC_P_BW_FIXED_BIAS_DELTA;
            }
        }
        else
        {
            /* http://confluence.broadcom.com/pages/viewpage.action?spaceKey=BSESW&title=Box+Modes+-+Documentation+Guidelines*/
            /* BBOX_Vdc_SclCapBias_eSclBeforeCap/BBOX_Vdc_SclCapBias_eAuto need capture on always */
            pNewInfo->bForceCapture = true;
            pNewInfo->eSclCapBias   = pBoxVdc->astDisplay[eDisplayId].astWindow[ulBoxWinId].eSclCapBias;
            /* if auto bias mode with PIP window < quater screen size, can force eSclCap to avoid vnet reconfig */
            if(pNewInfo->eSclCapBias == BVDC_SclCapBias_eAuto &&
               pNewInfo->stDstRect.ulWidth <= pDstFormatInfo->ulWidth/2 &&
               pNewInfo->stDstRect.ulHeight <= pDstFormatInfo->ulHeight/2 &&
               pNewInfo->eEnableBackgroundBars != BVDC_Mode_eOff)
            {
                pNewInfo->eSclCapBias   = BVDC_SclCapBias_eSclBeforeCap;
            }
            pNewInfo->ulBandwidthDelta = BVDC_P_BW_FIXED_BIAS_DELTA;
        }
    } else
    {
        /* if disregard bias mode with mfd PIP window < quater screen size, can force eSclCap to allow background bars */
        if(pNewInfo->stDstRect.ulWidth <= pDstFormatInfo->ulWidth/2 &&
           pNewInfo->stDstRect.ulHeight <= pDstFormatInfo->ulHeight/2 &&
           pNewInfo->eEnableBackgroundBars != BVDC_Mode_eOff &&
           BVDC_P_SRC_IS_MPEG(pNewInfo->hSource->eId))
        {
            pNewInfo->eSclCapBias   = BVDC_SclCapBias_eSclBeforeCap;
            pNewInfo->ulBandwidthDelta = BVDC_P_BW_FIXED_BIAS_DELTA;
        }
    }

    /* (0) Destination rect should be bigger than min */
    ulMinV = pDstFormatInfo->bInterlaced ? BVDC_P_WIN_DST_OUTPUT_V_MIN * 2 : BVDC_P_WIN_DST_OUTPUT_V_MIN;
    if(pNewInfo->stDstRect.ulHeight < ulMinV)
    {
        pNewInfo->stDstRect.ulHeight = ulMinV;
        BDBG_MSG(("Align up displaying %s, to min dst vertical %d", (pDstFormatInfo->bInterlaced) ?  "interlace" : "progressive", ulMinV));
    }

    if(pNewInfo->stScalerOutput.ulHeight < ulMinV)
    {
        pNewInfo->stScalerOutput.ulHeight = ulMinV;
        BDBG_MSG(("Align up surface %s, to min dst vertical %d", (pDstFormatInfo->bInterlaced) ?  "interlace" : "progressive", ulMinV));
    }

    /* (2) DstRect can not be larger than scaler output. */
    if((pNewInfo->stDstRect.ulWidth  + pNewInfo->stScalerOutput.lLeft > pNewInfo->stScalerOutput.ulWidth) ||
       (pNewInfo->stDstRect.ulHeight + pNewInfo->stScalerOutput.lTop  > pNewInfo->stScalerOutput.ulHeight))
    {
        BDBG_ERR(("DstRect[%dx%d], SclOut[%d, %d, %dx%d].",
            pNewInfo->stDstRect.ulWidth, pNewInfo->stDstRect.ulHeight, pNewInfo->stScalerOutput.lLeft,
            pNewInfo->stScalerOutput.lTop, pNewInfo->stScalerOutput.ulWidth, pNewInfo->stScalerOutput.ulHeight));
        return BERR_TRACE(BVDC_ERR_DST_SIZE_LARGER_THAN_SCL_OUTPUT);
    }

#if BVDC_P_SUPPORT_XCODE_WIN_CAP
    /*(2.7) check capture usage for xcode path*/
    if(BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg))
    {
        hCompositor->hVdc->ulXcodeWinCap += pNewInfo->bForceCapture;

        if (pBoxVdc->stXcode.ulNumXcodeCapVfd == BBOX_VDC_DISREGARD)
        {
            if (hCompositor->hVdc->ulXcodeWinCap > BVDC_P_SUPPORT_XCODE_WIN_CAP)
            {
                BDBG_ERR(("win[%d]: Xcode path only %d capture ", hWindow->eId, BVDC_P_SUPPORT_XCODE_WIN_CAP));
                return BERR_TRACE(BVDC_ERR_NO_AVAIL_CAPTURE_BUFFER);
            }
        }
        else /* pBoxVdc->ulNumXcodeCapVfd != BBOX_VDC_DISREGARD */
        {
            if (hCompositor->hVdc->ulXcodeWinCap > pBoxVdc->stXcode.ulNumXcodeCapVfd)
            {
                BDBG_ERR(("win[%d]: Xcode path only %d capture ", hWindow->eId, pBoxVdc->stXcode.ulNumXcodeCapVfd));
                return BERR_TRACE(BVDC_ERR_NO_AVAIL_CAPTURE_BUFFER);
            }
        }
    }
#endif

    if(BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId))
    {
        bool bIs10BitCore = false, bIs2xClk = false, bMad4kBypass = true;

        /* If MPG window's display is not aligned to or by another display, forced sync
           lock might cause tearing! */
        if(hWindow->stSettings.bForceSyncLock)
        {
            if(!BVDC_P_SRC_IS_MPEG(hWindow->stNewInfo.hSource->eId))
            {
                BDBG_ERR(("Forced synclock is not supported for non-MPG window yet!"));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }

            if(!(hDisplay->stNewInfo.hTargetDisplay || hDisplay->ulAlignSlaves || (1>=hWindow->stNewInfo.hSource->ulConnectedWindow)))
            {
                BDBG_ERR(("MPEG window%d can only be forced sync-lock when VEC aligned! %d", hWindow->eId, hWindow->stNewInfo.hSource->ulConnectedWindow));
                if(hDisplay->stNewInfo.hTargetDisplay)
                {
                    BDBG_ERR(("    Display%d's alignment target is display%d.",
                        hDisplay->eId, hDisplay->stNewInfo.hTargetDisplay->eId));
                }
                else
                {
                    BDBG_ERR(("    Display%d is aligned by %d display(s).", hDisplay->eId, hDisplay->ulAlignSlaves));
                }
            }
        }

        /* (3) Source clipped rect sould be inside source rect.  Source can
         * change overclipped will consider no clip. */

        /* (3.2) Left eye width and right eye width needs to be same, so  */
        if((uint32_t)BVDC_P_ABS(pNewInfo->stSrcClip.lLeftDelta_R) > pNewInfo->stSrcClip.ulRight)
        {
            BDBG_ERR(("Win[%d] Invalid Clipping values for right eye", hWindow->eId));
            BDBG_ERR(("Left eye: (%d, %d), right eye delta: %d",
                pNewInfo->stSrcClip.ulLeft, pNewInfo->stSrcClip.ulRight,
                pNewInfo->stSrcClip.lLeftDelta_R));
            return BERR_TRACE(BVDC_ERR_ILLEGAL_CLIPPING_VALUES);
        }

        /* (4) This video window can not accept gfx source. */
        if(BVDC_P_SRC_IS_GFX(hWindow->stNewInfo.hSource->eId))
        {
            return BERR_TRACE(BVDC_ERR_SOURCE_WINDOW_MISMATCH);
        }

        /* (5) Non-linear horizontal scaling doesn't coexist with ARC/SFR
         *     and PIP window */
        if((((0 != pNewInfo->ulNonlinearSrcWidth) || (0 != pNewInfo->ulNonlinearSclOutWidth)) &&
            ((BVDC_AspectRatioMode_eUseAllSource      == pNewInfo->eAspectRatioMode) ||
             (BVDC_AspectRatioMode_eUseAllDestination == pNewInfo->eAspectRatioMode) ||
             (pNewInfo->stScalerOutput.ulWidth  < (ulHsize  * 3 / 4)))) ||
           (pNewInfo->stScalerOutput.ulWidth < 2 * pNewInfo->ulNonlinearSclOutWidth))
        {
            return BERR_TRACE(BVDC_ERR_INVALID_NONLINEAR_SCALE);
        }

#if BVDC_P_SUPPORT_MOSAIC_MODE
        if(pNewInfo->bClearRect != pCurInfo->bClearRect)
        {
            pNewDirty->stBits.bMosaicMode = BVDC_P_DIRTY;
        }

        if(pNewInfo->eEnableBackgroundBars != pCurInfo->eEnableBackgroundBars)
        {
            pNewDirty->stBits.bRecAdjust = BVDC_P_DIRTY;
        }

        /* hd sd simul mosaic count validation */
        pNewInfo->ulMaxMosaicCount = pNewInfo->bMosaicMode?pNewInfo->ulMosaicCount:1;
        if(pNewInfo->hSource->stNewInfo.ulWindows>1)
        {
            BVDC_Window_Handle         hNewWindow;
            uint32_t jj, ulMinMosiacCount;
            uint32_t ii, ulMosaicCount = pNewInfo->bMosaicMode?pNewInfo->ulMosaicCount:1;
            const BVDC_Source_Handle hSource = hWindow->stNewInfo.hSource;

            /* for loop circling the source connected window */
            for(ii = 0; ii < BVDC_P_MAX_WINDOW_COUNT; ii++)
            {
                /* SKIP: If it's just created or inactive no need to build ruls. */
                if(!hSource->ahWindow[ii] ||
                    BVDC_P_STATE_IS_CREATE(hSource->ahWindow[ii]) ||
                    BVDC_P_STATE_IS_INACTIVE(hSource->ahWindow[ii]))
                {
                    continue;
                }

                hNewWindow = hSource->ahWindow[ii];
                BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
                if(hNewWindow->eId == hWindow->eId)
                    continue;

                ulMinMosiacCount = BVDC_P_MIN(ulMosaicCount,
                    hNewWindow->stNewInfo.bMosaicMode?hNewWindow->stNewInfo.ulMosaicCount:1);
                if(pNewInfo->bMosaicMode)
                {
                    for(jj = 0; jj < ulMinMosiacCount; jj++)
                    {
                        if(pNewInfo->abMosaicVisible[jj] && hNewWindow->stNewInfo.abMosaicVisible[jj] &&
                          (pNewInfo->aucMosaicZOrder[jj] != hNewWindow->stNewInfo.aucMosaicZOrder[jj]))
                        {
                            BDBG_ERR(("Mismatch zorder for HDSD"));
                            BDBG_ERR(("Win[%d] mosaic rect[%d] zorder: %d", hWindow->eId,
                                jj, pNewInfo->aucMosaicZOrder[jj]));
                            BDBG_ERR(("Win[%d] mosaic rect[%d] zorder: %d", hNewWindow->eId,
                                jj, hNewWindow->stNewInfo.aucMosaicZOrder[jj]));
                            return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
                        }
                    }
                }
                if(ulMosaicCount != hNewWindow->stNewInfo.ulMosaicCount)
                {
                    BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC, ("win[%d] mosaic %d win[%d] mosaic %d HD-SD mosaic window count need to be same and apply the change atomicly",
                        hWindow->eId,ulMosaicCount, hNewWindow->eId, hNewWindow->stNewInfo.ulMosaicCount));
                }
                hWindow->stNewInfo.ulMaxMosaicCount = BVDC_P_MAX(hWindow->stNewInfo.ulMaxMosaicCount, hNewWindow->stNewInfo.ulMosaicCount);
            }
        }

        /* (6) Mosaic window size checking; */
        if((pNewInfo->ulMaxMosaicCount>1)||(pNewInfo->bClearRect))
        {
            uint32_t i;
            bool     bSortZorder = false;

            if((pNewInfo->ulMosaicCount == 0)&&(pNewInfo->bMosaicMode))
            {
                return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
            }

            pNewInfo->stScalerOutput.ulWidth = BVDC_P_ALIGN_UP(pNewInfo->stScalerOutput.ulWidth, 2);
            pNewInfo->stDstRect.ulWidth = BVDC_P_ALIGN_UP(pNewInfo->stDstRect.ulWidth, 2);

            /* clear outside mode can not coexist with dest cut */
            if((pNewInfo->stScalerOutput.ulWidth  != pNewInfo->stDstRect.ulWidth)  ||
               (pNewInfo->stScalerOutput.ulHeight != pNewInfo->stDstRect.ulHeight) ||
               (pNewInfo->stScalerOutput.lLeft    != 0) ||
               (pNewInfo->stScalerOutput.lLeft_R  != 0) ||
               (pNewInfo->stScalerOutput.lTop     != 0))
            {
                BDBG_ERR(("Clear outside mosaic mode can not co-exist with dest cut!"));
                return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
            }

            if(pNewInfo->bMosaicMode)
            {
                for(i = 0; i < pNewInfo->ulMosaicCount; i++)
                {
                    /* adjust mosaic rect size and offset alignment with workarounds */
                    BVDC_P_Window_AdjustMosaicRect_isrsafe(hWindow,
                        hCompositor->stNewInfo.pFmtInfo->bInterlaced, &pNewInfo->astMosaicRect[i]);

                    /* the mosaics are bounded within ScalerOutput rect */
                    if((0 == pNewInfo->astMosaicRect[i].ulWidth)  ||
                       (0 == pNewInfo->astMosaicRect[i].ulHeight) ||
                       (0 > pNewInfo->astMosaicRect[i].lLeft) ||
                       (0 > pNewInfo->astMosaicRect[i].lLeft_R) ||
                       (0 > pNewInfo->astMosaicRect[i].lTop) ||
                       (pNewInfo->astMosaicRect[i].lLeft + pNewInfo->astMosaicRect[i].ulWidth
                        > pNewInfo->stScalerOutput.ulWidth)       ||
                       (pNewInfo->astMosaicRect[i].lTop + pNewInfo->astMosaicRect[i].ulHeight
                        > pNewInfo->stScalerOutput.ulHeight))
                    {
                        BDBG_ERR(("Mosaic[%d]: %dx%dx%dx%d - %d", i,
                            pNewInfo->astMosaicRect[i].lLeft, pNewInfo->astMosaicRect[i].lTop,
                            pNewInfo->astMosaicRect[i].ulWidth, pNewInfo->astMosaicRect[i].ulHeight,
                            pNewInfo->astMosaicRect[i].lLeft_R));
                        BDBG_ERR(("stScalerOutput: %d %d", pNewInfo->stScalerOutput.ulWidth,
                            pNewInfo->stScalerOutput.ulHeight));
                        return BERR_TRACE(BVDC_ERR_INVALID_MOSAIC_MODE);
                    }

                    if((pNewInfo->aucMosaicZOrder[i] != pCurInfo->aucMosaicZOrder[i]) ||
                       (pNewInfo->abMosaicVisible[i] != pCurInfo->abMosaicVisible[i]))
                    {
                        bSortZorder = true;
                    }

                    if(!BVDC_P_RECT_CMP_EQ(&pNewInfo->astMosaicRect[i], &pCurInfo->astMosaicRect[i]) ||
                       (pNewInfo->aucMosaicZOrder[i] != pCurInfo->aucMosaicZOrder[i]) ||
                       (pNewInfo->abMosaicVisible[i] != pCurInfo->abMosaicVisible[i]))
                    {
                        pNewDirty->stBits.bMosaicMode = BVDC_P_DIRTY;
                    }
                }
            }
            if((pNewInfo->bClearRectByMaskColor != pCurInfo->bClearRectByMaskColor) ||
               (pNewInfo->bMosaicMode      != pCurInfo->bMosaicMode) ||
               (pNewInfo->ulClearRectAlpha != pCurInfo->ulClearRectAlpha) ||
               (pNewInfo->ulMosaicCount != pCurInfo->ulMosaicCount) ||
               (pNewInfo->ulMaskColorYCrCb != pCurInfo->ulMaskColorYCrCb))
            {
                pNewDirty->stBits.bMosaicMode = BVDC_P_DIRTY;
            }

            if(bSortZorder)
            {
                uint32_t  i, j, k, ulInvisibleCnt, ulCnt;
                uint8_t   ucMaxZ, ulMinZ;

                ulInvisibleCnt = 0;
                ulMinZ = pNewInfo->aucMosaicZOrder[0];

                /* Put all the invisible ones at bottom */
                for(i = 0; i < pNewInfo->ulMosaicCount; i++)
                {
                    if(!pNewInfo->abMosaicVisible[i])
                    {
                        hWindow->aulMosaicZOrderIndex[i] = ulInvisibleCnt;
                        ulInvisibleCnt++;
                    }
                    else if(pNewInfo->aucMosaicZOrder[i] < ulMinZ)
                        ulMinZ = pNewInfo->aucMosaicZOrder[i];
                }

                /* Find the index to hSource->stNewPic */
                ucMaxZ = ulMinZ;
                ulCnt = ulInvisibleCnt;
                for(i = 0; i < pNewInfo->ulMosaicCount; i++)
                {
                    if(!pNewInfo->abMosaicVisible[i])
                        continue;

                    j = ulInvisibleCnt;
                    if(pNewInfo->aucMosaicZOrder[i] < ucMaxZ)
                    {
                        for(k = 0; k < i; k++)
                        {
                            if(!pNewInfo->abMosaicVisible[k])
                                continue;

                            if(pNewInfo->aucMosaicZOrder[k] <= pNewInfo->aucMosaicZOrder[i])
                            {
                                j++;
                            }
                            else
                            {
                                hWindow->aulMosaicZOrderIndex[k]++;
                            }
                        }
                        hWindow->aulMosaicZOrderIndex[i] = j;
                    }
                    else
                    {
                        hWindow->aulMosaicZOrderIndex[i] = ulCnt;
                        ucMaxZ = pNewInfo->aucMosaicZOrder[i];
                    }
                    ulCnt++;
                }

                for(i = 0; i < pNewInfo->ulMosaicCount; i++)
                {
                    BDBG_MSG(("Win[%d] Rect[%d]: zorder: %d (%d) index: %d ",
                        hWindow->eId, i, pNewInfo->aucMosaicZOrder[i],
                        pNewInfo->abMosaicVisible[i],  hWindow->aulMosaicZOrderIndex[i]));
                }

            }
        }
#endif

#if (BVDC_P_SUPPORT_MANR)
        /* (7) check anr and MAD setting compatibility */
        if((pNewInfo->bAnr) &&
           (pNewInfo->bDeinterlace))
        {
            if((BPXL_IS_YCbCr422_FORMAT(pNewInfo->stMadSettings.ePixelFmt) !=
                BPXL_IS_YCbCr422_FORMAT(pNewInfo->stAnrSettings.ePxlFormat)) &&
               (BPXL_IS_YCbCr422_10BIT_PACKED_FORMAT(pNewInfo->stMadSettings.ePixelFmt) !=
                BPXL_IS_YCbCr422_10BIT_PACKED_FORMAT(pNewInfo->stAnrSettings.ePxlFormat)))
            {
                BDBG_ERR(("Window[%d] ANR's pixel format mismatches with Deinterlacer!MAD[%s], ANR[%s]", hWindow->eId,
                    BPXL_ConvertFmtToStr(pNewInfo->stMadSettings.ePixelFmt),
                    BPXL_ConvertFmtToStr(pNewInfo->stAnrSettings.ePxlFormat)));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
        }
#endif

        /* (8) Bypass window can't do dest clip */
        if((BVDC_P_WindowId_eComp2_V0 == hWindow->eId) &&
           (hCompositor->bIsBypass))
        {
                /* due to the fact that bypass display doesn't have compositor, bypass
                 * window must be full screen; */
                if((pNewInfo->stScalerOutput.ulWidth  != ulHsize) ||
                   (pNewInfo->stScalerOutput.ulHeight != ulVsize) ||
                   (pNewInfo->stDstRect.ulWidth       != ulHsize) ||
                   (pNewInfo->stDstRect.ulHeight      != ulVsize))
                {
                    return BERR_TRACE(BVDC_ERR_BYPASS_WINDOW_NOT_FULL_SCREEN);
                }
                if((BVDC_AspectRatioMode_eBypass            != pNewInfo->eAspectRatioMode) &&
                   (BVDC_AspectRatioMode_eUseAllDestination != pNewInfo->eAspectRatioMode))
                {
                    return BERR_TRACE(BVDC_ERR_BYPASS_WINDOW_INVALID_AR_MODE);
                }
        }

        /* (9) bypass win and some 2nd win might might need to use shared SCL */
        if((BVDC_P_SRC_IS_VIDEO(hWindow->stNewInfo.hSource->eId)) &&
           /* then this win must be an one that uses shared scl */
           (   /* a PIP */
               (BVDC_P_WindowId_eComp2_V0 != hWindow->eId) ||
               /* or, bypass win that needs scl */
               ((BVDC_P_SRC_IS_MPEG(hWindow->stNewInfo.hSource->eId)  ||
                 BVDC_P_SRC_IS_HDDVI(hWindow->stNewInfo.hSource->eId)))
           )
          )
        {
            if (NULL == hWindow->stCurResource.hScaler)
            {
                BVDC_P_Scaler_Handle *phScaler=&hWindow->stNewResource.hScaler;

                BKNI_EnterCriticalSection();
                BVDC_P_Resource_AcquireHandle_isr(hResource,
                    BVDC_P_ResourceType_eScl, hWindow->stResourceFeature.ulScl,
                    (unsigned long) hWindow, (void **)phScaler, true);
                BKNI_LeaveCriticalSection();
                if(NULL == hWindow->stNewResource.hScaler)
                {
                    BDBG_ERR(("Window %d failed to allocate scaler ", hWindow->eId));
                    goto fail_res;
                }
                else
                {
                    /* Acquire tmp scaler resource, query capabilities that  */
                    /* needed to allocate XSRC, VFC and ITM, then release */
                    /* tmp scl resource */
                    BVDC_P_Scaler_Handle hTmpScl;
                    phScaler = &hTmpScl;
                    BKNI_EnterCriticalSection();
                    BVDC_P_Resource_AcquireHandle_isr(hResource,
                        BVDC_P_ResourceType_eScl, hWindow->stResourceFeature.ulScl,
                        (unsigned long) hWindow, (void **)phScaler, false);
                    bIs10BitCore = hTmpScl->bIs10BitCore;
                    bIs2xClk = hTmpScl->bIs2xClk;
                    BVDC_P_Resource_ReleaseHandle_isr(hResource,
                        BVDC_P_ResourceType_eScl, (void *)hTmpScl);
                    BKNI_LeaveCriticalSection();
                }

                pNewDirty->stBits.bSharedScl = BVDC_P_DIRTY;
                hWindow->bAllocResource = true;

            }
            else
            {
                hWindow->stNewResource.hScaler = hWindow->stCurResource.hScaler;
                bIs10BitCore = hWindow->stNewResource.hScaler->bIs10BitCore;
                bIs2xClk = hWindow->stNewResource.hScaler->bIs2xClk;
            }
        }

        /* (9-11') Aquire MCVP */
        if((pNewInfo->bDeinterlace || pNewInfo->bAnr))
        {
            if (NULL == hWindow->stCurResource.hMcvp)
            {
                BVDC_P_Mcvp_Handle *phMcvp = &hWindow->stNewResource.hMcvp;

                if (hWindow->stResourceFeature.ulMad != BVDC_P_Able_eInvalid)
                {
                    /* acquire a HW module */
                    BKNI_EnterCriticalSection();
                    BVDC_P_Resource_AcquireHandle_isr(hResource,
                        BVDC_P_ResourceType_eMcvp, hWindow->stResourceFeature.ulMad, hWindow->eId, (void **)phMcvp, true);
                    BKNI_LeaveCriticalSection();

                    if (NULL == hWindow->stNewResource.hMcvp)
                    {
                        BDBG_ERR(("Window %d failed to allocate MCVP.", hWindow->eId));
                        goto fail_res;
                    }
                    else
                    {
                        /* Acquire tmp mad resource, query capabilities that  */
                        /* needed to allocate XSRC then release tmp mad resource */
                        BVDC_P_Mcvp_Handle hTmpMcvp;
                        phMcvp = &hTmpMcvp;
                        BKNI_EnterCriticalSection();
                        BVDC_P_Resource_AcquireHandle_isr(hResource,
                            BVDC_P_ResourceType_eMcvp, hWindow->stResourceFeature.ulMad, hWindow->eId, (void **)phMcvp, false);
                        bMad4kBypass = hTmpMcvp->b4kBypass;
                        BVDC_P_Resource_ReleaseHandle_isr(hResource,
                            BVDC_P_ResourceType_eMcvp, (void *)hTmpMcvp);
                        BKNI_LeaveCriticalSection();
                    }

                    hWindow->bAllocResource = true;
                }
                else
                {
                    BDBG_WRN(("Window %d does not have MCVP resource. BOX mode policy may be limiting this.", hWindow->eId));
                    hWindow->bAllocResource = false;
                }
            }
            else
            {
                hWindow->stNewResource.hMcvp = hWindow->stCurResource.hMcvp;
                bMad4kBypass = hWindow->stNewResource.hMcvp->b4kBypass;
            }
        }

#if (BVDC_P_SUPPORT_XSRC)
        /* (12) Aquire XSRC */
        if(hWindow->stNewInfo.hSource->bIs2xClk &&
           hWindow->stNewInfo.hSource->bIs10BitCore &&
           (!bMad4kBypass || !bIs2xClk || !hWindow->hCompositor->bIs2xClk) &&
           BVDC_P_STATE_IS_CREATE(hWindow))
        {
            BVDC_P_Xsrc_Handle *phXsrc=&hWindow->stNewResource.hXsrc;

            if (NULL == hWindow->stCurResource.hXsrc)
            {
                /* acquire a HW module */
                BKNI_EnterCriticalSection();
                BVDC_P_Resource_AcquireHandle_isr(hResource,
                    BVDC_P_ResourceType_eXsrc, 0, hWindow->stNewInfo.hSource->eId, (void **)phXsrc, true);
                BKNI_LeaveCriticalSection();

                if (NULL == hWindow->stNewResource.hXsrc)
                {
                    BDBG_ERR(("Window %d failed to allocate XSRC ", hWindow->eId));
                    goto fail_res;
                }

                hWindow->bAllocResource = true;
            }
            else
            {
                hWindow->stNewResource.hXsrc = hWindow->stCurResource.hXsrc;
            }
        }
#else
        BSTD_UNUSED(bIs2xClk);
        BSTD_UNUSED(bMad4kBypass);
#endif

#if (BVDC_P_SUPPORT_VFC)

        /* (13) Aquire VFC */
        if(hWindow->stNewInfo.hSource->bIs10BitCore &&
           (!bIs10BitCore || !hWindow->hCompositor->bIs10BitCore) &&
           BVDC_P_STATE_IS_CREATE(hWindow))
        {
            BVDC_P_Vfc_Handle *phVfc=&hWindow->stNewResource.hVfc;

            if (NULL == hWindow->stCurResource.hVfc)
            {
                /* acquire a HW module */
                BKNI_EnterCriticalSection();
                BVDC_P_Resource_AcquireHandle_isr(hResource,
                    BVDC_P_ResourceType_eVfc, 0, hWindow->eId, (void **)phVfc, true);
                BKNI_LeaveCriticalSection();

                if (NULL == hWindow->stNewResource.hVfc)
                {
                    BDBG_ERR(("Window %d failed to allocate VFC ", hWindow->eId));
                    goto fail_res;
                }

                hWindow->bAllocResource = true;
            }
            else
            {
                hWindow->stNewResource.hVfc = hWindow->stCurResource.hVfc;
            }
        }
#else
        BSTD_UNUSED(bIs10BitCore);
#endif

#if (BVDC_P_SUPPORT_TNTD)
        /* Aquire TNTD */
        if(hWindow->stNewInfo.bSharpnessEnable)
        {
            BVDC_P_Tntd_Handle *phTntd=&hWindow->stNewResource.hTntd;

            if (NULL == hWindow->stCurResource.hTntd)
            {
                /* acquire a HW module */
                BKNI_EnterCriticalSection();
                BVDC_P_Resource_AcquireHandle_isr(hResource,
                    BVDC_P_ResourceType_eTntd, 0, hWindow->stNewInfo.hSource->eId, (void **)phTntd, true);
                BKNI_LeaveCriticalSection();

                if (NULL == hWindow->stNewResource.hTntd)
                {
                    BDBG_ERR(("Window %d failed to allocate TNTD ", hWindow->eId));
                    goto fail_res;
                }

                hWindow->bAllocResource = true;
            }
            else
            {
                hWindow->stNewResource.hTntd = hWindow->stCurResource.hTntd;
            }
        }
#endif

#if (BVDC_P_SUPPORT_DNR)
        /* (12) Aquire DNR */
        if((hWindow->stNewInfo.hSource->stNewInfo.bDnr))
        {
            BVDC_P_Dnr_Handle *phDnr=&hWindow->stNewResource.hDnr;
            uint32_t ulDnrCap = hWindow->stNewInfo.hSource->bIs10BitCore ? BVDC_P_Able_e10bits : BVDC_P_Able_e8bits;

            if (NULL == hWindow->stCurResource.hDnr)
            {
                /* acquire a HW module */
                BKNI_EnterCriticalSection();
                BVDC_P_Resource_AcquireHandle_isr(hResource,
                    BVDC_P_ResourceType_eDnr, ulDnrCap, hWindow->stNewInfo.hSource->eId, (void **)phDnr, true);
                BKNI_LeaveCriticalSection();

                if (NULL == hWindow->stNewResource.hDnr)
                {
                    BDBG_ERR(("Window %d failed to allocate DNR ", hWindow->eId));
                    goto fail_res;
                }

                hWindow->bAllocResource = true;
            }
            else
            {
                hWindow->stNewResource.hDnr = hWindow->stCurResource.hDnr;
            }
        }
#endif

#if BVDC_P_SUPPORT_BOX_DETECT
        /* (13) Shared box detect */
        if(pNewInfo->bBoxDetect)
        {
            if (NULL == hWindow->stCurResource.hBoxDetect)
            {
                BVDC_P_BoxDetect_Handle *phBoxDetect=&hWindow->stNewResource.hBoxDetect;
                /* acquire a HW module */
                BKNI_EnterCriticalSection();
                BVDC_P_Resource_AcquireHandle_isr(hResource,
                    BVDC_P_ResourceType_eBoxDetect, 0, hWindow->stNewInfo.hSource->eId, (void **)phBoxDetect, true);
                BKNI_LeaveCriticalSection();

                if (NULL == hWindow->stNewResource.hBoxDetect)
                {
                    BDBG_ERR(("Window %d failed to allocate BoxDetect", hWindow->eId));
                    goto fail_res;
                }
                hWindow->bAllocResource = true;
            }
            else
            {
                hWindow->stNewResource.hBoxDetect = hWindow->stCurResource.hBoxDetect;
            }

        }
#endif

        /* (16) Shared vnet crc  */
        if(pNewInfo->stCbSettings.stMask.bCrc)
        {
            if (NULL == hWindow->stCurResource.hVnetCrc)
            {
                BVDC_P_VnetCrc_Handle *phVnetCrc=&hWindow->stNewResource.hVnetCrc;
                /* acquire a HW module */
                BKNI_EnterCriticalSection();
                BVDC_P_Resource_AcquireHandle_isr(hResource,
                    BVDC_P_ResourceType_eVnetCrc, 0, hWindow->eId, (void **)phVnetCrc, true);
                BKNI_LeaveCriticalSection();

                if (NULL == hWindow->stNewResource.hVnetCrc)
                {
                    BDBG_ERR(("Window %d failed to allocate VnetCrc", hWindow->eId));
                    goto fail_res;
                }
                hWindow->bAllocResource = true;
            }
            else
            {
                hWindow->stNewResource.hVnetCrc = hWindow->stCurResource.hVnetCrc;
            }
        }
    }
    else
    {
        if(!BVDC_P_SRC_IS_GFX(hWindow->stNewInfo.hSource->eId))
        {
            return BERR_TRACE(BVDC_ERR_SOURCE_WINDOW_MISMATCH);
        }
    }

    /* (17) Check coverage */
    if(BVDC_P_Window_ValidateCoverage(hWindow) != BERR_SUCCESS)
    {
        BDBG_ERR(("Win[%d] RTS violation. Check settings", hWindow->eId));
    }

    /* set picture adjust dirty bit if picture adjustment values
     * have changed */
    if((pNewInfo->sHue            != pCurInfo->sHue           ) ||
       (pNewInfo->sContrast       != pCurInfo->sContrast      ) ||
       (pNewInfo->sBrightness     != pCurInfo->sBrightness    ) ||
       (pNewInfo->sSaturation     != pCurInfo->sSaturation    ) ||
       (pNewInfo->sColorTemp      != pCurInfo->sColorTemp     ) ||
       (pNewInfo->lAttenuationR   != pCurInfo->lAttenuationR  ) ||
       (pNewInfo->lAttenuationG   != pCurInfo->lAttenuationG  ) ||
       (pNewInfo->lAttenuationB   != pCurInfo->lAttenuationB  ) ||
       (pNewInfo->lOffsetR        != pCurInfo->lOffsetR       ) ||
       (pNewInfo->lOffsetG        != pCurInfo->lOffsetG       ) ||
       (pNewInfo->lOffsetB        != pCurInfo->lOffsetB       ) ||
       (pNewInfo->bCscRgbMatching != pCurInfo->bCscRgbMatching) ||
       (hDisplay->stNewInfo.stDirty.stBits.bTiming))
    {
        pNewDirty->stBits.bCscAdjust = BVDC_P_DIRTY;
    }
#if BVDC_P_SUPPORT_CMP_DEMO_MODE
    if((pNewInfo->stSplitScreenSetting.eHue != pCurInfo->stSplitScreenSetting.eHue) ||
       (pNewInfo->stSplitScreenSetting.eContrast != pCurInfo->stSplitScreenSetting.eContrast) ||
       (pNewInfo->stSplitScreenSetting.eBrightness != pCurInfo->stSplitScreenSetting.eBrightness) ||
       (pNewInfo->stSplitScreenSetting.eColorTemp != pCurInfo->stSplitScreenSetting.eColorTemp) ||
       (((pNewInfo->stSplitScreenSetting.eHue != BVDC_SplitScreenMode_eDisable) ||
         (pNewInfo->stSplitScreenSetting.eContrast != BVDC_SplitScreenMode_eDisable) ||
         (pNewInfo->stSplitScreenSetting.eBrightness != BVDC_SplitScreenMode_eDisable) ||
         (pNewInfo->stSplitScreenSetting.eColorTemp != BVDC_SplitScreenMode_eDisable)) &&
        (!BVDC_P_RECT_CMP_EQ(&pNewInfo->stDstRect, &pCurInfo->stDstRect))))
    {
        pNewDirty->stBits.bCscAdjust = BVDC_P_DIRTY;
    }
#endif

    /* set color key adjust dirty bit if color key settings have changed */
    if((pNewInfo->stColorKey.bLumaKey      != pCurInfo->stColorKey.bLumaKey)      ||
       (pNewInfo->stColorKey.ucLumaKeyMask != pCurInfo->stColorKey.ucLumaKeyMask) ||
       (pNewInfo->stColorKey.ucLumaKeyHigh != pCurInfo->stColorKey.ucLumaKeyHigh) ||
       (pNewInfo->stColorKey.ucLumaKeyLow  != pCurInfo->stColorKey.ucLumaKeyLow)  ||
       (pNewInfo->stColorKey.bChromaRedKey != pCurInfo->stColorKey.bChromaRedKey) ||
       (pNewInfo->stColorKey.ucChromaRedKeyMask != pCurInfo->stColorKey.ucChromaRedKeyMask) ||
       (pNewInfo->stColorKey.ucChromaRedKeyHigh != pCurInfo->stColorKey.ucChromaRedKeyHigh) ||
       (pNewInfo->stColorKey.ucChromaRedKeyLow  != pCurInfo->stColorKey.ucChromaRedKeyLow)  ||
       (pNewInfo->stColorKey.bChromaBlueKey != pCurInfo->stColorKey.bChromaBlueKey) ||
       (pNewInfo->stColorKey.ucChromaBlueKeyMask != pCurInfo->stColorKey.ucChromaBlueKeyMask) ||
       (pNewInfo->stColorKey.ucChromaBlueKeyHigh != pCurInfo->stColorKey.ucChromaBlueKeyHigh) ||
       (pNewInfo->stColorKey.ucChromaBlueKeyLow  != pCurInfo->stColorKey.ucChromaBlueKeyLow))
    {
        pNewDirty->stBits.bColorKeyAdjust = BVDC_P_DIRTY;
    }

    if(hWindow->bTntAvail &&
       ((pNewInfo->sSharpness       != pCurInfo->sSharpness ||
         pNewInfo->bSharpnessEnable != pCurInfo->bSharpnessEnable ||
         (!BVDC_P_RECT_CMP_EQ(&pNewInfo->stDstRect, &pCurInfo->stDstRect)) ||
          hWindow->stNewInfo.hSource->stNewInfo.stDirty.stBits.bResume) &&
         pNewInfo->bUserSharpnessConfig != true))
    {
#if (BVDC_P_SUPPORT_TNT)
        pNewDirty->stBits.bTntAdjust = BVDC_P_DIRTY;
#endif
    }

    /* Checking against dst size changed. Since PEP demo mode is only */
    /* available for Win0 CMP0, don't need to check for other windows */
    if(hWindow->stResourceRequire.bRequirePep)
    {
        if((pNewInfo->stSplitScreenSetting.eAutoFlesh  != pCurInfo->stSplitScreenSetting.eAutoFlesh) ||
           (pNewInfo->stSplitScreenSetting.eBlueBoost  != pCurInfo->stSplitScreenSetting.eBlueBoost) ||
           (pNewInfo->stSplitScreenSetting.eGreenBoost != pCurInfo->stSplitScreenSetting.eGreenBoost) ||
           (((pNewInfo->stSplitScreenSetting.eAutoFlesh  != BVDC_SplitScreenMode_eDisable) ||
             (pNewInfo->stSplitScreenSetting.eBlueBoost  != BVDC_SplitScreenMode_eDisable) ||
             (pNewInfo->stSplitScreenSetting.eGreenBoost != BVDC_SplitScreenMode_eDisable)) &&
            (!BVDC_P_RECT_CMP_EQ(&pNewInfo->stDstRect, &pCurInfo->stDstRect))))
        {
            pNewDirty->stBits.bCabAdjust = BVDC_P_DIRTY;
        }
        if((pNewInfo->stSplitScreenSetting.eCms !=
            pCurInfo->stSplitScreenSetting.eCms) ||
           ((pNewInfo->stSplitScreenSetting.eCms != BVDC_SplitScreenMode_eDisable) &&
            (!BVDC_P_RECT_CMP_EQ(&pNewInfo->stDstRect, &pCurInfo->stDstRect))))
        {
            pNewDirty->stBits.bCabAdjust = BVDC_P_DIRTY;
        }
        if((pNewInfo->stSplitScreenSetting.eContrastStretch !=
            pCurInfo->stSplitScreenSetting.eContrastStretch) ||
           ((pNewInfo->stSplitScreenSetting.eContrastStretch != BVDC_SplitScreenMode_eDisable) &&
            (!BVDC_P_RECT_CMP_EQ(&pNewInfo->stDstRect, &pCurInfo->stDstRect))))
        {
            pNewDirty->stBits.bLabAdjust = BVDC_P_DIRTY;
        }
    }
    if(hWindow->bTntAvail &&
       ((pNewInfo->stSplitScreenSetting.eSharpness !=
        pCurInfo->stSplitScreenSetting.eSharpness) ||
       ((pNewInfo->stSplitScreenSetting.eSharpness != BVDC_SplitScreenMode_eDisable) &&
        (!BVDC_P_RECT_CMP_EQ(&pNewInfo->stDstRect, &pCurInfo->stDstRect)))))
    {
#if (BVDC_P_SUPPORT_TNT)
        pNewDirty->stBits.bTntAdjust = BVDC_P_DIRTY;
#endif
    }

#if (BVDC_P_SUPPORT_HIST)
    pNewRect  = &pNewInfo->stLumaRect.stRegion;
    pCurRect  = &pCurInfo->stLumaRect.stRegion;
    if((pNewRect->ulLeft   != pCurRect->ulLeft) ||
       (pNewRect->ulRight  != pCurRect->ulRight) ||
       (pNewRect->ulTop    != pCurRect->ulTop) ||
       (pNewRect->ulBottom != pCurRect->ulBottom) ||
       (!BVDC_P_Hist_Level_Cmp(&pNewInfo->stLumaRect.aulLevelThres[0], &pCurInfo->stLumaRect.aulLevelThres[0])) ||
       (pNewInfo->stLumaRect.eNumBins != pCurInfo->stLumaRect.eNumBins) ||
       (!BVDC_P_RECT_CMP_EQ(&pNewInfo->stDstRect, &pCurInfo->stDstRect) ||
       (hCompositor->stNewInfo.pFmtInfo->bInterlaced != hCompositor->stCurInfo.pFmtInfo->bInterlaced)))
    {
        if(BVDC_P_WindowId_eComp0_V0 == hWindow->eId)
        {
            pNewDirty->stBits.bHistoRect = BVDC_P_DIRTY;
        }
    }
#endif

#if (BVDC_P_SUPPORT_STG)
    /* Inherit STG/ViCE enable toggle? */
    pNewDirty->stBits.bStg |=
        hDisplay->stNewInfo.stDirty.stBits.bStgEnable;
#endif

    /* Check delay offset */
    if( pNewInfo->uiVsyncDelayOffset )
    {
        uint32_t ulCount;

        ulCount = hWindow->ulBufCntNeeded +
            pNewInfo->uiVsyncDelayOffset  - pCurInfo->uiVsyncDelayOffset;

        if( (ulCount > BVDC_P_MAX_MULTI_BUFFER_COUNT) &&
            (hWindow->ulBufCntAllocated + pNewInfo->uiVsyncDelayOffset > pCurInfo->uiVsyncDelayOffset))
        {
            BDBG_ERR(("win[%d] BufCnt more than MAX: %d, Lip Sync delay %d -> %d, ulBufCntNeeded: %d",
                hWindow->eId, ulCount, pCurInfo->uiVsyncDelayOffset,
                pNewInfo->uiVsyncDelayOffset, hWindow->ulBufCntNeeded));
            return BERR_TRACE(BVDC_ERR_LIP_SYNC_DELAY_MORE_THAN_MAX);
        }
    }

    /* Check requested user capture buffer count. */
    if(pNewInfo->uiCaptureBufCnt)
    {
        uint32_t ulCount;

        ulCount = hWindow->ulBufCntNeeded;

        if( pNewInfo->uiVsyncDelayOffset )
        {
            ulCount = ulCount + pNewInfo->uiVsyncDelayOffset - pCurInfo->uiVsyncDelayOffset;
        }

        ulCount = ulCount + pNewInfo->uiCaptureBufCnt - pCurInfo->uiCaptureBufCnt;

        if( ulCount > BVDC_P_MAX_MULTI_BUFFER_COUNT )
        {
            BDBG_ERR(("Number of capture buffer requested exceeds the max allowed for window[%d] = %d",
                hWindow->eId, ulCount));
            return BERR_TRACE(BVDC_ERR_CAPTURE_BUFFERS_MORE_THAN_MAX);
        }
    }

    /* Game mode clock adjustment can not work together with slave mode RM */
    if(pNewInfo->stGameDelaySetting.bEnable)
    {
#if BVDC_P_MAX_DACS
        uint32_t uiIndex;
#endif

        /* No display alignment if game delay control is on; */
        if(hDisplay->stNewInfo.hTargetDisplay)
        {
            BDBG_ERR(("No display alignment if game delay control is on!"));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        switch(hDisplay->eMasterTg)
        {
        case BVDC_DisplayTg_ePrimIt:
        case BVDC_DisplayTg_eSecIt:
        case BVDC_DisplayTg_eTertIt:
            if(hDisplay->stNewInfo.bEnableHdmi || hDisplay->stNewInfo.bEnable656)
            {
                BDBG_ERR(("Display%d master TG%d with slaved %s cannot have game mode clock control!",
                    hDisplay->eId, hDisplay->eMasterTg,
                    hDisplay->stNewInfo.bEnableHdmi? "DVO" : "656"));
                return BERR_TRACE(BERR_INVALID_PARAMETER);
            }
            break;
        case BVDC_DisplayTg_e656Dtg:
        case BVDC_DisplayTg_eDviDtg:
#if BVDC_P_MAX_DACS
            for(uiIndex=0; uiIndex < BVDC_P_MAX_DACS; uiIndex++)
            {
                if(hDisplay->stNewInfo.aDacOutput[uiIndex] != BVDC_DacOutput_eUnused)
                {   /* Search for valid Dac combinations */
                    BDBG_ERR(("Display%d digital master TG%d with slaved DAC%d cannot have game mode clock control!",
                        hDisplay->eId, hDisplay->eMasterTg, uiIndex));
                    return BERR_TRACE(BERR_INVALID_PARAMETER);
                }
            }
#endif
            break;
        /* @@@ How to validate the change on STG*/
        case BVDC_DisplayTg_eStg0:
#if (BVDC_P_NUM_SHARED_STG > 1)
        case BVDC_DisplayTg_eStg1:
#endif
            break;
        default:
            BDBG_ERR(("Slave mode display %d cannot adjust clock to reduce game mode delay",
                hDisplay->eId));
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    BDBG_LEAVE(BVDC_P_Window_ValidateChanges);
    return BERR_SUCCESS;

fail_res:
    hWindow->stNewResource = hWindow->stCurResource;
    hWindow->bAllocResource = false;
    BDBG_LEAVE(BVDC_P_Window_ValidateChanges);
    return BERR_TRACE(BVDC_ERR_WINDOW_NOT_AVAILABLE);
}


#if BVDC_P_SUPPORT_CMP_CLEAR_RECT
/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Window_SetClearRect_isr
    ( BVDC_Window_Handle               hWindow,
      const BVDC_P_Window_Info        *pUserInfo,
      BVDC_P_PictureNode              *pPicture)
{
    BVDC_Compositor_Handle hCompositor;
    uint32_t ulMosaicCount;
    const bool     *pMosaicVisible;
    const BVDC_P_Rect *pstMosaicRect;

    BDBG_ENTER(BVDC_P_Window_SetClearRect_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    hCompositor = hWindow->hCompositor;
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_TOP_CTRL) &= ~(
        BCHP_MASK(CMP_0_V0_RECT_TOP_CTRL, RECT_ENABLE) |
        BCHP_MASK(CMP_0_V0_RECT_TOP_CTRL, RECT_CONFIG) |
#ifdef BCHP_CMP_0_V0_RECT_TOP_CTRL_RECT_KEY_VALUE_MASK
        BCHP_MASK(CMP_0_V0_RECT_TOP_CTRL, RECT_KEY_VALUE) |
#endif
        BCHP_MASK(CMP_0_V0_RECT_TOP_CTRL, RECT_COLOR_SRC));

    if(pUserInfo->bClearRect)
    {
        uint32_t ulIdx;
        bool bVsizeShift = hCompositor->stNewInfo.pFmtInfo->bInterlaced;

        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_TOP_CTRL) |=
            BCHP_FIELD_DATA(CMP_0_V0_RECT_TOP_CTRL, RECT_ENABLE, 1) |
            BCHP_FIELD_DATA(CMP_0_V0_RECT_TOP_CTRL, RECT_CONFIG, pUserInfo->bMosaicMode) |
#ifdef BCHP_CMP_0_V0_RECT_TOP_CTRL_RECT_KEY_VALUE_MASK
            BCHP_FIELD_DATA(CMP_0_V0_RECT_TOP_CTRL, RECT_KEY_VALUE, pUserInfo->ulClearRectAlpha) |
#endif
            BCHP_FIELD_DATA(CMP_0_V0_RECT_TOP_CTRL, RECT_COLOR_SRC, pUserInfo->bClearRectByMaskColor);

#ifndef BCHP_CMP_0_V0_RECT_TOP_CTRL_RECT_KEY_VALUE_MASK
        /* SW7425-2236: Use colorkey to masked out garbage */
        if(pUserInfo->bMosaicMode)
        {
            BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_COLOR) =
                BCHP_FIELD_DATA(CMP_0_V0_RECT_COLOR, Y,  0x00) |
                BCHP_FIELD_DATA(CMP_0_V0_RECT_COLOR, CR, 0x80) |
                BCHP_FIELD_DATA(CMP_0_V0_RECT_COLOR, CB, 0x80);
        }
        else
        {
            BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_COLOR) = pUserInfo->ulMaskColorYCrCb;
        }
#else
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_COLOR) = pUserInfo->ulMaskColorYCrCb;
#endif
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_ENABLE_MASK) = 0;

        if(NULL == pPicture)
        {
            ulMosaicCount = pUserInfo->ulMosaicCount;
            pMosaicVisible = &pUserInfo->abMosaicVisible[0];
            pstMosaicRect = &pUserInfo->astMosaicRect[0];
        }
        else
        {
            ulMosaicCount = pPicture->ulMosaicCount;
            pMosaicVisible = &pPicture->abMosaicVisible[0];
            pstMosaicRect = &pPicture->astMosaicRect[0];
        }

        for(ulIdx = 0; ulIdx < ulMosaicCount; ulIdx++)
        {
            if(pMosaicVisible[ulIdx])
            {
                BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_ENABLE_MASK) |= 1<<ulIdx;

                BVDC_P_WIN_GET_REG_DATA_I(ulIdx, CMP_0_V0_RECT_SIZEi_ARRAY_BASE) =
                    BCHP_FIELD_DATA(CMP_0_V0_RECT_SIZEi, HSIZE,
                        BVDC_P_ALIGN_UP(pstMosaicRect[ulIdx].ulWidth, 2)) |
                    BCHP_FIELD_DATA(CMP_0_V0_RECT_SIZEi, VSIZE,
                        pstMosaicRect[ulIdx].ulHeight >> bVsizeShift);

                BVDC_P_WIN_GET_REG_DATA_I(ulIdx, CMP_0_V0_RECT_OFFSETi_ARRAY_BASE) =
                    BCHP_FIELD_DATA(CMP_0_V0_RECT_OFFSETi, X_OFFSET,
                        BVDC_P_ALIGN_UP(pstMosaicRect[ulIdx].lLeft, 2)) |
                    BCHP_FIELD_DATA(CMP_0_V0_RECT_OFFSETi, Y_OFFSET,
                        pstMosaicRect[ulIdx].lTop >> bVsizeShift);
            }
        }
        /* update the rects enable mask */
        hWindow->ulMosaicRectSet = BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_ENABLE_MASK);
    }
    hCompositor->ulMosaicAdjust[hWindow->eId] = BVDC_P_RUL_UPDATE_THRESHOLD;

    BDBG_LEAVE(BVDC_P_Window_SetClearRect_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Window_SetBgColor_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_PictureNode              *pPicture,
      BAVC_Polarity                    ePolarity)
{
    BVDC_Compositor_Handle hCompositor;

    BDBG_ENTER(BVDC_P_Window_SetBgColor_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    hCompositor = hWindow->hCompositor;
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_TOP_CTRL) &= ~(
        BCHP_MASK(CMP_0_V0_RECT_TOP_CTRL, RECT_ENABLE) |
        BCHP_MASK(CMP_0_V0_RECT_TOP_CTRL, RECT_CONFIG) |
#ifdef BCHP_CMP_0_V0_RECT_TOP_CTRL_RECT_KEY_VALUE_MASK
        BCHP_MASK(CMP_0_V0_RECT_TOP_CTRL, RECT_KEY_VALUE) |
#endif
        BCHP_MASK(CMP_0_V0_RECT_TOP_CTRL, RECT_COLOR_SRC));

    if(BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode) &&
       (BVDC_Mode_eOff != hWindow->stCurInfo.eEnableBackgroundBars) &&
       (pPicture->bMosaicMode)
#if BVDC_P_SUPPORT_STG /* don't clear window background for transcoder window */
    && !BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg)
#endif
    )/* mosaic mode and clearRect takes precedence */
    {
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_TOP_CTRL) |=
            BCHP_FIELD_DATA(CMP_0_V0_RECT_TOP_CTRL, RECT_ENABLE, 1) |
            BCHP_FIELD_DATA(CMP_0_V0_RECT_TOP_CTRL, RECT_CONFIG, 1) | /* clear outside */
#ifdef BCHP_CMP_0_V0_RECT_TOP_CTRL_RECT_KEY_VALUE_MASK
            BCHP_FIELD_DATA(CMP_0_V0_RECT_TOP_CTRL, RECT_KEY_VALUE, 0xFF) | /* opaque */
#endif
            BCHP_FIELD_DATA(CMP_0_V0_RECT_TOP_CTRL, RECT_COLOR_SRC, 1);

        /* clear rect mask color is before CMP blender in CSC */
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_COLOR) = hCompositor->ulPreBlendInBgColor;
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_ENABLE_MASK) = 1;

        BVDC_P_WIN_GET_REG_DATA_I(0, CMP_0_V0_RECT_SIZEi_ARRAY_BASE) =
            BCHP_FIELD_DATA(CMP_0_V0_RECT_SIZEi, HSIZE,
                BVDC_P_ALIGN_UP(hWindow->stAdjDstRect.ulWidth, 2)) |
            BCHP_FIELD_DATA(CMP_0_V0_RECT_SIZEi, VSIZE,
                hWindow->stAdjDstRect.ulHeight >> (ePolarity!=BAVC_Polarity_eFrame));

        BVDC_P_WIN_GET_REG_DATA_I(0, CMP_0_V0_RECT_OFFSETi_ARRAY_BASE) =
            BCHP_FIELD_DATA(CMP_0_V0_RECT_OFFSETi, X_OFFSET,
                BVDC_P_ALIGN_UP(pPicture->astMosaicRect[0].lLeft, 2)) |
            BCHP_FIELD_DATA(CMP_0_V0_RECT_OFFSETi, Y_OFFSET,
                (pPicture->astMosaicRect[0].lTop) >> (ePolarity!=BAVC_Polarity_eFrame));
    }
    {
        hCompositor->ulMosaicAdjust[hWindow->eId] = BVDC_P_RUL_UPDATE_THRESHOLD;
    }

    BDBG_LEAVE(BVDC_P_Window_SetBgColor_isr);
    return;
}
#endif


#if BVDC_P_CMP_0_MOSAIC_CFCS
/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Window_SetMosaicCsc_isr
    ( BVDC_Window_Handle               hWindow )
{
    uint32_t usCscIdxShift = BCHP_CMP_0_V0_RECT_CSC_INDEX_0_CSC_COEFF_INDEX_RECT1_SHIFT -
                             BCHP_CMP_0_V0_RECT_CSC_INDEX_0_CSC_COEFF_INDEX_RECT0_SHIFT;
    uint32_t i = 0;

    BDBG_ENTER(BVDC_P_Window_SetMosaicCsc_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(hWindow->stCurInfo.ulMosaicCount <= hWindow->stSettings.ulMaxMosaicRect);

    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_CSC_INDEX_0) = 0;

    if (hWindow->stSettings.ulMaxMosaicRect > 8)
    {
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_CSC_INDEX_1) = 0;
    }

    for (i = 0; i < hWindow->stCurInfo.ulMosaicCount; i++)
    {
        uint8_t ucMosaicCscSlot = hWindow->aucMosaicCfcIdxForRect[i];

        if (i < 8)
        {
            BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_CSC_INDEX_0) |= ucMosaicCscSlot << (i * usCscIdxShift);
        }
        else
        {
            BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_RECT_CSC_INDEX_1) |= ucMosaicCscSlot << ((i-8) * usCscIdxShift);
        }
    }
    BDBG_LEAVE(BVDC_P_Window_SetMosaicCsc_isr);
}
#endif

/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Window_SetBypassColor_isr
    ( BVDC_Window_Handle               hWindow,
      bool                             bFixedColor)
{
    BVDC_Compositor_Handle  hCompositor;
    BVDC_P_WindowId  eV0Id;
    uint32_t ulEnColorConv;
#if (BVDC_P_CMP_CFC_VER <= BVDC_P_CFC_VER_1)
    BCFC_ColorSpace *pColorSpaceIn = &(hWindow->pMainCfc->stColorSpaceExtIn.stColorSpace);
    BCFC_ColorSpace *pColorSpaceOut = &(hWindow->pMainCfc->pColorSpaceExtOut->stColorSpace);
    bool bBypassColorByTf, bBypassCmpCsc, bBypassDviCsc;
#endif

    BDBG_ENTER(BVDC_P_Window_SetBypassColor_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
    hCompositor = hWindow->hCompositor;
    eV0Id = BVDC_P_CMP_GET_V0ID(hCompositor);

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2)
    hWindow->bBypassCmpCsc = hWindow->stSettings.bBypassVideoProcessings || bFixedColor;
    ulEnColorConv = (hWindow->bBypassCmpCsc)? 0 : 1;
#else /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */
    bBypassColorByTf = BCFC_NEED_TF_CONV(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF);
    bBypassCmpCsc = hWindow->stSettings.bBypassVideoProcessings || bBypassColorByTf || bFixedColor ||
        (BCFC_IS_BT2020(pColorSpaceIn->eColorimetry) && BCFC_IS_BT2020(pColorSpaceOut->eColorimetry) && (pColorSpaceIn->eColorFmt != BCFC_ColorFormat_eYCbCr_CL));
    if (bBypassColorByTf && !hWindow->bBypassCmpCsc)
    {
        BDBG_WRN(("Cmp%d_V%d bypass CSC due to TF %d->%d, display might be too bright or too dim", hCompositor->eId, hWindow->eId-eV0Id, pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF));
    }
    else if (hWindow->bBypassCmpCsc != bBypassCmpCsc)
    {
        /* WARNING!! no extra msg out when switch from "bypass due to mismatched eotf" to "matched eotf and matched matrixCoeffs" */
        BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d decides %s CSC, due to (TF %d->%d, b2020 %d->%d)", hCompositor->eId, hWindow->eId-eV0Id, (bBypassCmpCsc)? "bypass":"no longer bypass",
            pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF, BCFC_IS_BT2020(pColorSpaceIn->eColorimetry), BCFC_IS_BT2020(pColorSpaceOut->eColorimetry)));
    }
    hWindow->bBypassCmpCsc = bBypassCmpCsc;
    ulEnColorConv = (hWindow->bBypassCmpCsc)? 0 : 1;
    if((hCompositor->ulActiveVideoWindow == 1) || (hWindow->eId == eV0Id))
    {
        const BCFC_ColorSpace  *pDspOutColorSpace = &hCompositor->hDisplay->stOutColorSpaceExt.stColorSpace;

        /* if there is only one window in this compositor, it decides whether dviCsc bypass,
         * and if two windows are enabled, then window 0 decides it */
        bBypassDviCsc = bFixedColor ||
            ((bBypassColorByTf || (pColorSpaceIn->eColorimetry == pColorSpaceOut->eColorimetry)) &&
             (pDspOutColorSpace->eColorFmt == BCFC_ColorFormat_eYCbCr) &&
             (pDspOutColorSpace->eColorRange == BCFC_ColorRange_eLimited));

        /* Needs DviCsc if we're muting the Dvi output with fixed color using Csc */
        bBypassDviCsc &= !hCompositor->hDisplay->stCurInfo.abOutputMute[BVDC_DisplayOutput_eDvo];

        if (hCompositor->bBypassDviCsc != bBypassDviCsc)
        {
            BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d decides to %s DVI_CSC", hCompositor->eId, hWindow->eId-eV0Id, (bBypassDviCsc)? "bypass":"not bypass"));
            hCompositor->bBypassDviCsc = bBypassDviCsc;
        }
    }
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_1)
    if (hCompositor->stCfcCapability[hWindow->eId - eV0Id].stBits.bMa)
    {
        /* Enable or disable ma_color_conv */
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, MA_COLOR_CONV_ENABLE) |
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, COLOR_CONV_ENABLE));
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
            BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, MA_COLOR_CONV_ENABLE, ulEnColorConv) |
            BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, COLOR_CONV_ENABLE, ulEnColorConv));
    }
    else
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_1) */
    {
        /*BCHP_CMP_0_V0_SURFACE_CTRL_COLOR_CONV_ENABLE_MASK*/
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, COLOR_CONV_ENABLE));
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
            BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, COLOR_CONV_ENABLE, ulEnColorConv));
    }

    BDBG_LEAVE(BVDC_P_Window_SetBypassColor_isr);
}

/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Window_SetMiscellaneous_isr
    ( BVDC_Window_Handle               hWindow,
      const BVDC_P_Window_Info        *pWinInfo )
{
#if (BDBG_DEBUG_BUILD)
    BVDC_P_WindowId eV0Id;
#endif
    BDBG_ENTER(BVDC_P_Window_SetMiscellaneous_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

#if (BDBG_DEBUG_BUILD)
    eV0Id = BVDC_P_CMP_GET_V0ID(hWindow->hCompositor);
#endif

    if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
    {
        /* Enable or disable visibility
         * note: non-zero value being written to the reserved bits of
         * CMP_x_G0_SURFACE_CTRL register might kill VEC */
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE));
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
            BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, pWinInfo->bVisible ? 1 : 0));

        /* done for gfx win */
        BDBG_LEAVE(BVDC_P_Window_SetMiscellaneous_isr);
        return;
    }

    /* Enable or disable dering */
#if BCHP_CMP_0_V0_SURFACE_CTRL_DERING_EN_ENABLE
    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
    BCHP_MASK(CMP_0_V0_SURFACE_CTRL, DERING_EN));
    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
        BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, DERING_EN,
        !hWindow->stSettings.bBypassVideoProcessings));
#endif

    BDBG_MODULE_MSG(BVDC_CFC_5,("Cmp%d_V%d invokes setBypassColor_isr by %s", hWindow->hCompositor->eId, hWindow->eId-eV0Id, BSTD_FUNCTION));
    BVDC_P_Window_SetBypassColor_isr (hWindow,
        (BVDC_MuteMode_eConst == hWindow->stCurInfo.hSource->stCurInfo.eMuteMode) || hWindow->bMuteBypass);

#if BVDC_P_SUPPORT_WIN_CONST_COLOR
    {
        /* Get the window's source. */
        const BVDC_P_Source_Info *pSrcInfo = &pWinInfo->hSource->stNewInfo;

        /* Set window constant color. */
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_CONST_COLOR) = (
            BCHP_FIELD_DATA(CMP_0_V0_CONST_COLOR, Y,  BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pSrcInfo->ulMuteColorYCrCb, 2)) |
            BCHP_FIELD_DATA(CMP_0_V0_CONST_COLOR, CR, BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pSrcInfo->ulMuteColorYCrCb, 0)) |
            BCHP_FIELD_DATA(CMP_0_V0_CONST_COLOR, CB, BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pSrcInfo->ulMuteColorYCrCb, 1)));
    }
#endif

    BDBG_LEAVE(BVDC_P_Window_SetMiscellaneous_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * Configure a blender.
 */
void BVDC_P_Window_SetBlender_isr
    ( BVDC_Window_Handle               hWindow,
      uint8_t                          ucZOrder,
      uint8_t                          ucConstantAlpha,
      BVDC_BlendFactor                 eFrontBlendFactor,
      BVDC_BlendFactor                 eBackBlendFactor )
{
    BVDC_Compositor_Handle hCompositor;
    uint32_t ulBlendAddr     = 0;
    uint32_t ulBlendSrcSel   = 0;
    uint32_t ulPrevBlendAddr = 0;

    BSTD_UNUSED(ucZOrder);
    BDBG_ENTER(BVDC_P_Window_SetBlender_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Get the compositor that this window belongs to.   The blender
     * registers are spread out in the compositor. */
    hCompositor = hWindow->hCompositor;
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* Note: These are compositor's MACRO.  The Z-Order of each window
     * acctually selected the blender. */
    ulBlendAddr       = s_aulBlendAddr[hWindow->ulBlenderId];

    /* set new blender source only if it's not destroyed; */
    /*if(!BVDC_P_STATE_IS_DESTROY(hWindow))*/
    if(hCompositor->aeBlenderWinId[hWindow->ulBlenderId] == hWindow->eId)
    {
        switch (hWindow->eId)
        {
        case BVDC_P_WindowId_eComp0_G0:
        case BVDC_P_WindowId_eComp1_G0:
        case BVDC_P_WindowId_eComp2_G0:
        case BVDC_P_WindowId_eComp3_G0:
        case BVDC_P_WindowId_eComp4_G0:
        case BVDC_P_WindowId_eComp5_G0:
        case BVDC_P_WindowId_eComp6_G0:
            ulBlendSrcSel = BCHP_FIELD_ENUM(CMP_0_BLEND_0_CTRL, BLEND_SOURCE, SURFACE_G0);
            break;

#if (BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT >= 2)
        case BVDC_P_WindowId_eComp0_G1:
            ulBlendSrcSel = BCHP_FIELD_ENUM(CMP_0_BLEND_0_CTRL, BLEND_SOURCE, SURFACE_G1);
            break;
#endif

#if (BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT >= 3)
        case BVDC_P_WindowId_eComp0_G2:
            ulBlendSrcSel = BCHP_FIELD_ENUM(CMP_0_BLEND_0_CTRL, BLEND_SOURCE, SURFACE_G2);
            break;
#endif

        case BVDC_P_WindowId_eComp0_V0:
        case BVDC_P_WindowId_eComp1_V0:
        case BVDC_P_WindowId_eComp2_V0:
        case BVDC_P_WindowId_eComp3_V0:
        case BVDC_P_WindowId_eComp4_V0:
        case BVDC_P_WindowId_eComp5_V0:
        case BVDC_P_WindowId_eComp6_V0:
            ulBlendSrcSel = BCHP_FIELD_ENUM(CMP_0_BLEND_0_CTRL, BLEND_SOURCE, SURFACE_V0);
            break;

        case BVDC_P_WindowId_eComp0_V1:
        case BVDC_P_WindowId_eComp1_V1:
            ulBlendSrcSel = BCHP_FIELD_ENUM(CMP_0_BLEND_0_CTRL, BLEND_SOURCE, SURFACE_V1);
            break;

        default:
            /* should not get here */
            BDBG_ASSERT(false);
            break;
        }

        /* apply blender source to the correct blender */
        BVDC_P_CMP_GET_REG_ADDR_DATA(ulBlendAddr) &= ~(
            BCHP_MASK(CMP_0_BLEND_0_CTRL, BLEND_SOURCE));
        BVDC_P_CMP_GET_REG_ADDR_DATA(ulBlendAddr) |= ulBlendSrcSel;

        if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
        {
            BVDC_P_GfxFeeder_AdjustBlend_isr(hWindow->stNewInfo.hSource->hGfxFeeder,
                &eFrontBlendFactor, &eBackBlendFactor, &ucConstantAlpha);
        }
    }

    /* Clear out the old blender. */
    if(!hWindow->hCompositor->abBlenderUsed[hWindow->ulPrevBlenderId])
    {
        ulPrevBlendAddr = s_aulBlendAddr[hWindow->ulPrevBlenderId];

        BVDC_P_CMP_GET_REG_ADDR_DATA(ulPrevBlendAddr) &= ~(
            BCHP_MASK(CMP_0_BLEND_0_CTRL, BLEND_SOURCE));
        BVDC_P_CMP_GET_REG_ADDR_DATA(ulPrevBlendAddr) |= (
            BCHP_FIELD_ENUM(CMP_0_BLEND_0_CTRL, BLEND_SOURCE, BACKGROUND_BYPASS));
    }

    if(hCompositor->aeBlenderWinId[hWindow->ulBlenderId] == hWindow->eId)
    {
        /* Blending factors */
        BVDC_P_CMP_GET_REG_ADDR_DATA(ulBlendAddr) &= ~(
            BCHP_MASK(CMP_0_BLEND_0_CTRL, CONSTANT_ALPHA          ) |
            BCHP_MASK(CMP_0_BLEND_0_CTRL, FRONT_COLOR_BLEND_FACTOR) |
            BCHP_MASK(CMP_0_BLEND_0_CTRL, BACK_COLOR_BLEND_FACTOR ));
        BVDC_P_CMP_GET_REG_ADDR_DATA(ulBlendAddr) |=  (
            BCHP_FIELD_DATA(CMP_0_BLEND_0_CTRL, CONSTANT_ALPHA, ucConstantAlpha) |
            BCHP_FIELD_DATA(CMP_0_BLEND_0_CTRL, FRONT_COLOR_BLEND_FACTOR, eFrontBlendFactor) |
            BCHP_FIELD_DATA(CMP_0_BLEND_0_CTRL, BACK_COLOR_BLEND_FACTOR, eBackBlendFactor));
    }

    hWindow->ulPrevBlenderId = hWindow->ulBlenderId;
    BDBG_LEAVE(BVDC_P_Window_SetBlender_isr);
    return;
}

#if (BVDC_P_SUPPORT_BOX_DETECT)
/***************************************************************************
 * {private}
 *
 * Enable box detect for the window: create a boxDetect handle if there is
 * free box detect hw module
 */
BERR_Code BVDC_P_Window_EnableBoxDetect
    ( BVDC_Window_Handle                 hWindow,
      BVDC_Window_BoxDetectCallback_isr  BoxDetectCallBack,
      void                              *pvParm1,
      int                                iParm2,
      bool                               bAutoCutBlack)
{
    BVDC_P_Window_Info *pNewInfo;
    BERR_Code  eResult = BERR_SUCCESS;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
    {
        eResult = BVDC_ERR_BOX_DETECT_GFX;
    }

    /* */
    pNewInfo = &hWindow->stNewInfo;
    pNewInfo->bBoxDetect = true;
    pNewInfo->bAutoCutBlack = bAutoCutBlack;
    pNewInfo->BoxDetectCallBack = BoxDetectCallBack;
    pNewInfo->pvBoxDetectParm1 = pvParm1;
    pNewInfo->iBoxDetectParm2 = iParm2;

    pNewInfo->stDirty.stBits.bBoxDetect = 1;
    hWindow->ulBoxDetectCallBckCntr = 0;

    return BERR_TRACE(eResult);
}
#endif

/***************************************************************************
 * {private}
 *
 * Disable box detect for the window: destroy boxDetect handle
 */
BERR_Code BVDC_P_Window_DisableBoxDetect
    ( BVDC_Window_Handle                 hWindow )
{
    BVDC_P_Window_Info *pNewInfo;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    pNewInfo = &hWindow->stNewInfo;

    pNewInfo->bBoxDetect = false;
    pNewInfo->bAutoCutBlack = false;

    pNewInfo->stDirty.stBits.bBoxDetect = (hWindow->stCurResource.hBoxDetect)? 1 : 0;
    hWindow->ulBoxDetectCallBckCntr = 0;

    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 * Set up a window to use a user color matrix.
 */
BERR_Code BVDC_P_Window_SetColorMatrix
    ( BVDC_Window_Handle               hWindow,
      const int32_t                    pl32_UserMatrix[BVDC_CSC_COEFF_COUNT],
      int32_t                          pl32_WinMatrix[BVDC_CSC_COEFF_COUNT])
{
    uint32_t ulIndex;

    BDBG_ENTER(BVDC_P_Window_SetColorMatrix);
    BSTD_UNUSED(hWindow);

    /* set new values */
    for(ulIndex = 0; ulIndex < BVDC_CSC_COEFF_COUNT; ulIndex++)
    {
        pl32_WinMatrix[ulIndex] = pl32_UserMatrix[ulIndex];
    }

    BDBG_LEAVE(BVDC_P_Window_SetColorMatrix);
    return BERR_SUCCESS;
}


#if !B_REFSW_MINIMAL
/***************************************************************************
 * {private}
 *
 * Get the color matrix currently in use.
 */
BERR_Code BVDC_P_Window_GetColorMatrix
    ( BVDC_Window_Handle               hWindow,
      bool                             bOverride,
      int32_t                          pl32_MatrixWin[BVDC_CSC_COEFF_COUNT],
      int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT])
{
    BDBG_ENTER(BVDC_P_Window_GetColorMatrix);
    BSTD_UNUSED(hWindow);

    if(bOverride)
    {
        uint32_t ulIndex;
        for(ulIndex = 0; ulIndex < BVDC_CSC_COEFF_COUNT; ulIndex++)
        {
            pl32_Matrix[ulIndex] = pl32_MatrixWin[ulIndex];
        }
    }
    else
    {
        BKNI_EnterCriticalSection();
        BVDC_P_Cfc_ToMatrix_isr(pl32_Matrix, &hWindow->pMainCfc->stMc, BVDC_P_FIX_POINT_SHIFT);
        BKNI_LeaveCriticalSection();
    }

    BDBG_LEAVE(BVDC_P_Window_GetColorMatrix);
    return BERR_SUCCESS;
}
#endif


/***************************************************************************
 * {private}
 *
 * Configure a Sec color space conversion table inside a compositor, used
 * for demo mode.
 */
void BVDC_P_Window_SetSecCscDemo_isr
    ( BVDC_Window_Handle               hWindow )
{
#if BVDC_P_SUPPORT_CMP_DEMO_MODE
    BVDC_Window_SplitScreenSettings *pSplitSetting;
    uint32_t ulCscDemoEnable = 0, ulCscDemoSide, ulCscDemoBoundary;

    BDBG_ENTER(BVDC_P_Window_SetSecCscDemo_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    pSplitSetting = &hWindow->stCurInfo.stSplitScreenSetting;
    /* Only enable demo mode if the feature itself is enable and the demo */
    /* mode is enable */
    if((pSplitSetting->eHue != BVDC_SplitScreenMode_eDisable &&
        (hWindow->stCurInfo.sHue != 0 || hWindow->stCurInfo.sSaturation != 0)) ||
       (pSplitSetting->eContrast != BVDC_SplitScreenMode_eDisable &&
        hWindow->stCurInfo.sContrast != 0) ||
       (pSplitSetting->eBrightness != BVDC_SplitScreenMode_eDisable &&
        hWindow->stCurInfo.sBrightness != 0) ||
       (pSplitSetting->eColorTemp != BVDC_SplitScreenMode_eDisable &&
        hWindow->stCurInfo.sColorTemp != 0))
    {
        ulCscDemoEnable = 1;
    }
#if (BVDC_P_SUPPORT_CMP_DEMO_MODE >= 2)
    ulCscDemoSide = (pSplitSetting->eHue == BVDC_SplitScreenMode_eLeft) ?
        BCHP_CMP_0_CSC_DEMO_SETTING_DEMO_CSC_L_R_LEFT :
        ((pSplitSetting->eBrightness == BVDC_SplitScreenMode_eLeft) ?
          BCHP_CMP_0_CSC_DEMO_SETTING_DEMO_CSC_L_R_LEFT :
          ((pSplitSetting->eContrast == BVDC_SplitScreenMode_eLeft) ?
            BCHP_CMP_0_CSC_DEMO_SETTING_DEMO_CSC_L_R_LEFT :
            ((pSplitSetting->eColorTemp == BVDC_SplitScreenMode_eLeft) ?
              BCHP_CMP_0_CSC_DEMO_SETTING_DEMO_CSC_L_R_LEFT :
              BCHP_CMP_0_CSC_DEMO_SETTING_DEMO_CSC_L_R_RIGHT)));
#else
    ulCscDemoSide = (pSplitSetting->eHue == BVDC_SplitScreenMode_eLeft) ?
        BCHP_CMP_0_CSC_DEMO_SETTING_DEMO_L_R_LEFT :
        ((pSplitSetting->eBrightness == BVDC_SplitScreenMode_eLeft) ?
          BCHP_CMP_0_CSC_DEMO_SETTING_DEMO_L_R_LEFT :
          ((pSplitSetting->eContrast == BVDC_SplitScreenMode_eLeft) ?
            BCHP_CMP_0_CSC_DEMO_SETTING_DEMO_L_R_LEFT :
            ((pSplitSetting->eColorTemp == BVDC_SplitScreenMode_eLeft) ?
              BCHP_CMP_0_CSC_DEMO_SETTING_DEMO_L_R_LEFT :
              BCHP_CMP_0_CSC_DEMO_SETTING_DEMO_L_R_RIGHT)));
#endif
    ulCscDemoBoundary = hWindow->stAdjDstRect.ulWidth / 2;

    /* Demo mode only for main window.*/
    if(hWindow->eId != BVDC_P_WindowId_eComp0_V0)
    {
        return;
    }

    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
        BCHP_MASK(CMP_0_V0_SURFACE_CTRL, COLOR_CONV_DEMO_ENABLE));
    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
        BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, COLOR_CONV_DEMO_ENABLE, ulCscDemoEnable));

    /* If it's not enabled there is no need to set the coefficients. */
    if(ulCscDemoEnable == 0)
    {
        return;
    }

    /* [ CMP_0_CSC_DEMO_SETTING ] */
#if (BVDC_P_SUPPORT_CMP_DEMO_MODE >= 2)
    BVDC_P_WIN_SET_REG_DATA(CMP_0_CSC_DEMO_SETTING, \
        BCHP_FIELD_DATA(CMP_0_CSC_DEMO_SETTING, DEMO_CSC_L_R, ulCscDemoSide) | \
        BCHP_FIELD_DATA(CMP_0_CSC_DEMO_SETTING, DEMO_CSC_BOUNDARY, ulCscDemoBoundary));
#else
    BVDC_P_WIN_SET_REG_DATA(CMP_0_CSC_DEMO_SETTING, \
        BCHP_FIELD_DATA(CMP_0_CSC_DEMO_SETTING, DEMO_L_R, ulCscDemoSide) | \
        BCHP_FIELD_DATA(CMP_0_CSC_DEMO_SETTING, DEMO_BOUNDARY, ulCscDemoBoundary));
#endif

    BDBG_LEAVE(BVDC_P_Window_SetSecCscDemo_isr);
#else
    BSTD_UNUSED(hWindow);
#endif
    return;
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

/***************************************************************************
 * {private}
 *
 * Configure a input surface size.
 */
void BVDC_P_Window_SetSurfaceSize_isr
    ( BVDC_Window_Handle               hWindow,
      const BVDC_P_Rect               *pSurRect,
      BAVC_Polarity                    eScanType )
{
    uint32_t ulHeight;
    uint32_t ulTop;
    uint32_t ulWidth;
    uint32_t ulShift;

    BDBG_ENTER(BVDC_P_Window_SetSurfaceSize_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* if we did xfer dst cut to src cut, pSurRect->lTop are
     * even number or 0. Odd pSurRect->lTop */
    ulShift  = (BAVC_Polarity_eFrame!=eScanType)? 1 : 0;
    ulTop    = pSurRect->lTop >> ulShift;
    ulHeight = pSurRect->ulHeight >> ulShift;

    ulWidth = pSurRect->ulWidth;

    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_OFFSET) &= ~(
        BCHP_MASK(CMP_0_V0_SURFACE_OFFSET, X_OFFSET) |
        BCHP_MASK(CMP_0_V0_SURFACE_OFFSET, Y_OFFSET));
    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_OFFSET) |=  (
        BCHP_FIELD_DATA(CMP_0_V0_SURFACE_OFFSET, X_OFFSET, pSurRect->lLeft) |
        BCHP_FIELD_DATA(CMP_0_V0_SURFACE_OFFSET, Y_OFFSET, ulTop));

    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_SIZE) &= ~(
        BCHP_MASK(CMP_0_V0_SURFACE_SIZE, HSIZE) |
        BCHP_MASK(CMP_0_V0_SURFACE_SIZE, VSIZE));
    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_SIZE) |=  (
        BCHP_FIELD_DATA(CMP_0_V0_SURFACE_SIZE, HSIZE, ulWidth) |
        BCHP_FIELD_DATA(CMP_0_V0_SURFACE_SIZE, VSIZE, ulHeight));

    BDBG_LEAVE(BVDC_P_Window_SetSurfaceSize_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * Configure a display size surface size (clip window of input surface
 * size).
 */
void BVDC_P_Window_SetDisplaySize_isr
    ( BVDC_Window_Handle               hWindow,
      const BVDC_P_Rect               *pDstRect,
      BAVC_Polarity                    eScanType,
      uint32_t                         ulRWinXOffset)
{
    uint32_t ulHeight;
    uint32_t ulTop;
    uint32_t ulShift;

    BDBG_ENTER(BVDC_P_Window_SetDisplaySize_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* TODO: do we allow odd lTop and/or odd height to be honored? */
    ulShift  = (BAVC_Polarity_eFrame!=eScanType)? 1 : 0;
    ulTop    = pDstRect->lTop >> ulShift;
    ulHeight = pDstRect->ulHeight >> ulShift;

    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_CANVAS_OFFSET) &= ~(
        BCHP_MASK(CMP_0_V0_CANVAS_OFFSET, X_OFFSET) |
        BCHP_MASK(CMP_0_V0_CANVAS_OFFSET, Y_OFFSET));
    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_CANVAS_OFFSET) |=  (
        BCHP_FIELD_DATA(CMP_0_V0_CANVAS_OFFSET, X_OFFSET, pDstRect->lLeft) |
        BCHP_FIELD_DATA(CMP_0_V0_CANVAS_OFFSET, Y_OFFSET, ulTop));

    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_DISPLAY_SIZE) &= ~(
        BCHP_MASK(CMP_0_V0_DISPLAY_SIZE, HSIZE) |
        BCHP_MASK(CMP_0_V0_DISPLAY_SIZE, VSIZE));
    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_DISPLAY_SIZE) |=  (
        BCHP_FIELD_DATA(CMP_0_V0_DISPLAY_SIZE, HSIZE, pDstRect->ulWidth) |
        BCHP_FIELD_DATA(CMP_0_V0_DISPLAY_SIZE, VSIZE, ulHeight));

    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_CANVAS_X_OFFSET_R) =
        BCHP_FIELD_DATA(CMP_0_V0_CANVAS_X_OFFSET_R, X_OFFSET, ulRWinXOffset);

    BDBG_LEAVE(BVDC_P_Window_SetDisplaySize_isr);
    return;
}

#ifndef BVDC_FOR_BOOTUPDATER
/***************************************************************************
 * {private}
 *
 * Set video surface src and dsp rect for muted case and vnet shut-down
 * case (constant color will be displayed).
 */
static void BVDC_P_Window_SetMutedSurSize_isr
    ( BVDC_Window_Handle               hWindow,
      BAVC_Polarity                    eFieldId )
{
    BVDC_P_Rect  stSrcRect;
    uint32_t     ulRWindowXOffset = 0;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    stSrcRect = hWindow->stCurInfo.stDstRect;
    stSrcRect.lTop = 0;
    stSrcRect.lLeft = 0;
    /* user setting stDstRect might not be aligned */
    stSrcRect.ulWidth = BVDC_P_ALIGN_UP(stSrcRect.ulWidth, 2);

    ulRWindowXOffset = (uint32_t) (hWindow->stCurInfo.lRWinXOffsetDelta + hWindow->stCurInfo.stDstRect.lLeft);

    BVDC_P_Window_SetSurfaceSize_isr(hWindow, &stSrcRect, eFieldId);
    BVDC_P_Window_SetDisplaySize_isr(hWindow, &hWindow->stCurInfo.stDstRect, eFieldId, ulRWindowXOffset);

}

/***************************************************************************
 * {private}
 *
 * This function sets the window's output switch to compositor. When the
 * output is diabled, i.e. the reader vnet is shut down, the compositor might
 * use constant color to show the location of the window if the window is
 * active; or really disable the video surface in compositor is the the window
 * is not active or to be destroyed.
 * ucAlpha is used to control alpha setting for the video window.A sepcial way
 * to hide video window is by setting its alpha to source zero.
 */
static void BVDC_P_Window_WinOutSetEnable_isr
    ( BVDC_Window_Handle               hWindow,
      uint8_t                          ucAlpha,
      bool                             bEnable )
{
    BDBG_ENTER(BVDC_P_Window_WinOutSetEnable_isr);
    /* Get the compositor that this window belongs to. */
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Update for miscellaneous applied now. */
#if BVDC_P_SUPPORT_WIN_CONST_COLOR
    if (bEnable)  /* maybe enable foe drain */
    {
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, CONST_COLOR_ENABLE) |
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE));
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
            BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, 1));
    }
    else if ((BVDC_P_State_eActive != hWindow->eState) && /* after create applied */
             (BVDC_P_State_eDestroy != hWindow->eState)) /* before destroy applied */
    {
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, CONST_COLOR_ENABLE) |
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE));
    }
    else  /* disable, but not destroy. i.e. only vnet shut down */
    {
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, CONST_COLOR_ENABLE) |
            BCHP_MASK(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE));
        /* temporarily disable const color due to stress test vfd intr issue */
/*
        BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
            BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, CONST_COLOR_ENABLE, 1) |
            BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, 1));
*/
    }
#else
    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
        BCHP_MASK(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE));
    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
        BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, bEnable));
#endif

    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) &= ~(
        BCHP_MASK(CMP_0_V0_SURFACE_CTRL, VWIN_ALPHA));
    BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
        BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, VWIN_ALPHA, ucAlpha));

    BDBG_LEAVE(BVDC_P_Window_WinOutSetEnable_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * This function xfer partial or complete horizontal scaling from SCL to
 * HSCL.  This reduce the bandwidth requirement on Deinterlacer/ANR.
 */
static void BVDC_P_Window_TryXferHrzSclToHscl_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_PictureNode              *pPicture,
      uint32_t                         ulHsclSrcHrzSclThr )
{
    uint32_t ulNrmHi, ulNrmLo;
    uint32_t ulHsclInWidth, ulHsclOutWidth;
    uint32_t ulHsclCentralSrcRegion;
    bool bBypassSCL = false;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    ulHsclInWidth = pPicture->pHsclIn->ulWidth;
    if(((pPicture->stSclOut.ulWidth < ulHsclInWidth) ||
        (hWindow->stCurInfo.stMadSettings.bShrinkWidth)) &&
       (ulHsclInWidth > ulHsclSrcHrzSclThr))
    {
        /* if mad/mcdi is not used, stMadOut doesn't mean deinterlacer output
         * rectangle any more, it's borrowed to support ANR-only mode; */
        pPicture->stMadOut = *(pPicture->pHsclIn);

        /* try to xfer partial horizontal scale to HSCL to avoid RTS problem
         * note: mad is right before scl */

        if(pPicture->stSclOut.ulWidth <= ulHsclSrcHrzSclThr ||
           hWindow->stCurInfo.stMadSettings.bShrinkWidth)
        {
            /* in this case, for both linear and non-linear scaling, hscl
             * linearly maps its full input to ulHsclSrcHrzSclThr, no crop,
             * and setup in SCL will be ajusted according to this map ratio */
            ulHsclOutWidth = ulHsclSrcHrzSclThr;

            /* "bBypassSCL = false;" will cause setup in HSCL and ratio adjust
             * for setup in SCL */
        }
        else /* if(pPicture->stSclOut.ulWidth > ulHsclSrcHrzSclThr) */
        {
            /* for linear scaling, HSCL will scale to the final
             * size and do crop, SCL should just pass through;
             * for non-linear scaling, HSCL will linearly scale to
             * to the final size too, but no crop, and setup in SCL will be
             * ajusted according to this map ratio -> perform non-linear
             * scale between the same final size */
            ulHsclOutWidth = pPicture->stSclOut.ulWidth;

            if(0 == pPicture->ulNonlinearSclOutWidth)
            {
                /* hscl perform hrz SclCut and scale, SCL should just pass through */
                bBypassSCL = true;

                pPicture->stHsclCut.lLeft = pPicture->stSclCut.lLeft;
                pPicture->stHsclCut.lLeft_R = pPicture->stSclCut.lLeft_R;
                pPicture->stHsclCut.ulWidth = pPicture->stSclCut.ulWidth;
                pPicture->ulHsclNrmHrzSrcStep = pPicture->ulNrmHrzSrcStep;

                pPicture->stSclCut.lLeft = 0;
                pPicture->stSclCut.lLeft_R = 0;
                pPicture->stSclCut.ulWidth = ulHsclOutWidth;
                pPicture->ulNrmHrzSrcStep = (1<<BVDC_P_NRM_SRC_STEP_F_BITS);
            }
        }

        /* stMadOut stores hscl's output rect now */
        pPicture->stMadOut.ulWidth = ulHsclOutWidth;

        if (false == bBypassSCL)
        {
            BDBG_ASSERT(ulHsclInWidth);
            BDBG_ASSERT(ulHsclOutWidth);

            /* pPicture->ulHsclNrmHrzSrcStep = (ulHsclInWidth / ulHsclOutWidth); */
            pPicture->ulHsclNrmHrzSrcStep =
                (ulHsclInWidth << BVDC_P_NRM_SRC_STEP_F_BITS) / ulHsclOutWidth;

            /* pPicture->ulNrmHrzSrcStep *= (ulHsclOutWidth / ulHsclInWidth); */
            BMTH_HILO_32TO64_Mul(pPicture->ulNrmHrzSrcStep, ulHsclOutWidth, &ulNrmHi, &ulNrmLo);
            BMTH_HILO_64TO64_Div32(ulNrmHi, ulNrmLo, ulHsclInWidth, &ulNrmHi, &ulNrmLo);
            BDBG_ASSERT(ulNrmHi == 0);
            pPicture->ulNrmHrzSrcStep = ulNrmLo;

            pPicture->stSclCut.lLeft =
                (pPicture->stSclCut.lLeft * ulHsclOutWidth) / ulHsclInWidth;
            pPicture->stSclCut.ulWidth =
                (pPicture->stSclCut.ulWidth * ulHsclOutWidth) / ulHsclInWidth;
            pPicture->stSclCut.lLeft_R =
                (pPicture->stSclCut.lLeft_R * ulHsclOutWidth) / ulHsclInWidth;

            if(0 != pPicture->ulNonlinearSclOutWidth)
            {
                ulHsclCentralSrcRegion = ulHsclInWidth - 2 * pPicture->ulNonlinearSrcWidth;
                ulHsclCentralSrcRegion =
                    (ulHsclCentralSrcRegion * ulHsclOutWidth) / ulHsclInWidth;
                pPicture->ulNonlinearSrcWidth =
                    (ulHsclOutWidth - ulHsclCentralSrcRegion) / 2;

                BDBG_MODULE_MSG(BVDC_ASP_RAT,("Xfr'd NL SCL step size: 0x%x", pPicture->ulNrmHrzSrcStep));
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("Xfr'd NL SCL src width: 0x%x", pPicture->ulNonlinearSrcWidth));
            }
        }

        /* 3549/3556 MAD uses 4:2:2 internally, so hsclOut, anrIn/Out, and madIn/Out
         * width must be aligned to even number. Note: no need to use AlignPreMadRect_isr
         * since we are not dealing with hscl cnt rect here, but output which does not
         * change*/
        pPicture->stMadOut.ulWidth = BVDC_P_ALIGN_UP(pPicture->stMadOut.ulWidth, 2);

#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
        BVDC_P_Window_PreMadAdjustWidth_isr(pPicture->stMadOut.ulWidth,
            (pPicture->PicComRulInfo.eSrcOrigPolarity == BAVC_Polarity_eFrame) ?
            pPicture->stMadOut.ulHeight : pPicture->stMadOut.ulHeight /2,
            hWindow->stMadCompression.ulBitsPerGroup,
            hWindow->stMadCompression.ulPixelPerGroup,
            &pPicture->stMadOut.ulWidth);
#endif

        /* if HSCL is separate, or inside MCVP  */
        if(BVDC_P_VNET_USED_MVP(pPicture->stVnetMode))
        {
            pPicture->pHsclOut = &pPicture->stMadOut;
            pPicture->pMadIn   = pPicture->pHsclOut;
            pPicture->pMadOut  = pPicture->pMadIn;

            if(BVDC_P_MVP_USED_ANR(pPicture->stMvpMode))
            {
                pPicture->pAnrIn = pPicture->pHsclOut;
                pPicture->pAnrOut = pPicture->pHsclOut;
            }
        }
        else /* HSCL is inside MAD */
        {
            pPicture->pMadOut = &pPicture->stMadOut;
            if(BVDC_P_MVP_USED_ANR(pPicture->stMvpMode))
            {
                pPicture->pAnrOut = &pPicture->stMadOut;
            }
        }
        pPicture->pSclIn = (BVDC_P_MVP_USED_MAD(pPicture->stMvpMode)) ?
            pPicture->pMadOut : pPicture->pAnrOut;
    }
}

/***************************************************************************
 * {private}
 *
 * This function overrides the size that exceeds the limit with the proper
 * limit
 */
static void BVDC_P_Window_EnforceMinSizeLimit_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_Rect                     *pRec,
      const char                      *pchName,
      uint32_t                         ulMinH,
      uint32_t                         ulMinV,
      bool                             bInterlace )
{
    uint32_t ulMinHeight = (bInterlace) ? ulMinV * 2 : ulMinV;

    if(pRec != NULL)
    {
        if(pRec->ulWidth < ulMinH)
        {
            BDBG_MSG(("Win[%d] %s violation: ulWidth = %d vs %d",
                hWindow->eId, pchName, pRec->ulWidth, ulMinH));
            pRec->ulWidth = ulMinH;
        }
        if(pRec->ulHeight < ulMinHeight)
        {
            BDBG_MSG(("Win[%d] %s violation: ulHeight = %d vs %d",
                hWindow->eId, pchName, pRec->ulHeight, ulMinV));
            pRec->ulHeight = ulMinHeight;
        }
    }

    BSTD_UNUSED(pchName);
    BSTD_UNUSED(hWindow);
}

/***************************************************************************
 * {private}
 *
 * This function overide pPicture->stFlags.bPictureRepeatFlag to false if clipping
 * to left or top before mad changed, because to mad, its input is no-longer
 * the same.
 */
static void BVDC_P_Window_AdjustPicRepeatBit_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_PictureNode              *pPicture )
{
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    /* if content or cutiing before mad changes, we overide bPictureRepeatFlag
     * to false 4 times, to flush the last change out */
    if (((hWindow->lPrevSrcOutLeft != pPicture->stSrcOut.lLeft) ||
         (hWindow->lPrevSrcOutTop  != pPicture->stSrcOut.lTop)) ||
        (BVDC_P_VNET_USED_CAPTURE(pPicture->stVnetMode) &&
         BVDC_P_MVP_USED_MAD_AT_READER(pPicture->stVnetMode, pPicture->stMvpMode) &&
         ((hWindow->lPrevCapOutLeft != pPicture->stCapOut.lLeft) ||
          (hWindow->lPrevCapOutTop  != pPicture->stCapOut.lTop))))
    {
        if(BVDC_P_MVP_USED_MAD(hWindow->stMvpMode))
        {
            hWindow->ulMadFlushCntr = 1 + BVDC_P_Mcdi_GetVsyncDelayNum_isr(
                hWindow->stCurResource.hMcvp->hMcdi,
                hWindow->stCurInfo.stMadSettings.eGameMode);
        }
    }
    if (hWindow->ulMadFlushCntr > 0)
    {
        hWindow->ulMadFlushCntr --;
        pPicture->stFlags.bPictureRepeatFlag = false;
    }

    hWindow->lPrevSrcOutLeft = pPicture->stSrcOut.lLeft;
    hWindow->lPrevSrcOutTop  = pPicture->stSrcOut.lTop;
    hWindow->lPrevCapOutLeft = pPicture->stCapOut.lLeft;
    hWindow->lPrevCapOutTop  = pPicture->stCapOut.lTop;
}


/***************************************************************************
 * {private}
 *
 * This function counting non-ignore pictures between Rd and Wr index in
 * stMadDelayed structure
 *
 */
static void BVDC_P_Window_CountNonIgnoredPic_isr
    ( uint32_t                        ulDeferIdxRd,
      uint32_t                        ulDeferIdxWr,
      BVDC_Window_Handle               hWindow,
      uint32_t                        ulPictureIdx)
{
    uint32_t ulDropNonIgnorePics = 0;
    uint32_t i=(ulDeferIdxRd+1)&3;
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

#if (BVDC_P_SUPPORT_STG)

    while (i!= ulDeferIdxWr)
    {
        i = (i+1) & 3;
        ulDropNonIgnorePics += !hWindow->stMadDelayed[ulPictureIdx][i].bIgnorePicture;
    }

#else
    BSTD_UNUSED(ulDeferIdxRd);
    BSTD_UNUSED(ulDeferIdxWr);
    BSTD_UNUSED(ulPictureIdx);
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(i);
#endif

    hWindow->ulDropCntNonIgnoredPics += ulDropNonIgnorePics;

}
/***************************************************************************
 * {private}
 *
 * This function adjust stuff in picture node to reflect the vsync delay
 * caused by MAD.
 *
 * Note: we assume SCL is always following MAD.
 * This function is very tricky in that it has to mimic the pipelinging
 * of the picture info (size/cropping/hist etc) delayed by deinterlacer;
 * the purpose is to program the BVN blocks (SCL/PEP) after the deinterlacer
 * in the frame accurate fashion to avoid visual artifact;
 */
static void BVDC_P_Window_AdjustForMadDelay_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_PictureNode              *pPicture )
{
    BVDC_P_Window_MadDelayed *pWriter, *pReader;
    uint32_t ulPrevDeferIdxWr, ulDeferIdxWr,ulPrevDeferIdxRd, ulDeferIdxRd, ulDeferHistIdxRd, ulPictureIdx;
    uint32_t ulMaxMadDelayBufIdx = BVDC_MADDELAY_BUF_COUNT -1;
    uint16_t usMadVsyncDelay = 0;
    bool bMadHardStart = false;
    bool bRdIdxIncby01 = true;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    ulPictureIdx = pPicture->ulPictureIdx;
    ulPrevDeferIdxRd = hWindow->ulDeferIdxRd[ulPictureIdx];
    ulPrevDeferIdxWr = hWindow->ulDeferIdxWr[ulPictureIdx];

    if(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode))
    {
        bMadHardStart = BVDC_P_Mcdi_BeHardStart_isr(hWindow->stCurResource.hMcvp->hMcdi->bInitial,
            hWindow->stCurResource.hMcvp->hMcdi);
        usMadVsyncDelay = BVDC_P_Mcdi_GetVsyncDelayNum_isr(
            hWindow->stCurResource.hMcvp->hMcdi,
            hWindow->stCurInfo.stMadSettings.eGameMode);
    }

    /* Note: mad hard start must reset the pipeline!
     * MAD would hard start whenever it's enabled for the 1st time or pic size
     * changes; MAD game mode change does not necessarily cause hard start unless
     * buffer re-allocation! */
    if (bMadHardStart || hWindow->bResetMadDelaySwPipe)
    {
        /* initialize the rd to coincide with wr to have valid start point; */
        hWindow->ulDeferIdxRd[ulPictureIdx] = hWindow->ulDeferIdxWr[ulPictureIdx];

        /* clear the reset flag */
        hWindow->bResetMadDelaySwPipe = false;
    }
    /* if deinterlacer input repeats, simulate mad hw freeze state (no r/w pointers bumpup) */
    else if(!pPicture->stFlags.bPictureRepeatFlag)
    {
        hWindow->ulDeferIdxWr[ulPictureIdx] = (ulPrevDeferIdxWr + 1) & ulMaxMadDelayBufIdx; /* % 4 */

        /* we always increase writer ptr, but increase reader ptr only after writer
         * has gone usMadVsyncDelay steps ahead of reader ptr.
         * note: this decision has to be done with the previous ulDeferIdxWr before move,
         * otherwise in the case that usMadVsyncDelay==3, writer will have gone back
         * to the original postion before reader starts to move, and then reader will
         * never move. */
        if(((ulPrevDeferIdxWr + BVDC_MADDELAY_BUF_COUNT - hWindow->ulDeferIdxRd[ulPictureIdx]) & ulMaxMadDelayBufIdx) >= (uint32_t)usMadVsyncDelay)
        {
            /* if mad game mode delay decreases at this vsync, this will make reader ptr
             * jump to catch up writer ptr. If mad game mode delay increases at this vsync,
             * the above test will fail for a few vsync so that reader ptr will pause and
             * writer ptr will move alone until the new delay is reached */
            hWindow->ulDeferIdxRd[ulPictureIdx] = (hWindow->ulDeferIdxWr[ulPictureIdx] + BVDC_MADDELAY_BUF_COUNT - usMadVsyncDelay) & ulMaxMadDelayBufIdx; /* % 4 */
        }
    }

    ulDeferIdxWr = hWindow->ulDeferIdxWr[ulPictureIdx];
    ulDeferIdxRd = hWindow->ulDeferIdxRd[ulPictureIdx];

    bRdIdxIncby01 = ((ulDeferIdxRd == ((ulPrevDeferIdxRd+1)&ulMaxMadDelayBufIdx)) ||
                    ((ulDeferIdxRd == ulPrevDeferIdxRd) && (!bMadHardStart)));


    /* counting non-ignore picture between Prev Rd and Wr idx */
    if(bMadHardStart) {
        BVDC_P_Window_CountNonIgnoredPic_isr(ulPrevDeferIdxRd, ulPrevDeferIdxWr, hWindow, ulPictureIdx);
    } else if(!bRdIdxIncby01) {
        BVDC_P_Window_CountNonIgnoredPic_isr(ulPrevDeferIdxRd, ulDeferIdxRd, hWindow, ulPictureIdx);
    }
    /* update mad delay affected setting in picture node to pipeline the picture
     * info that has to flow through the MAD delays;
     * Note: this ensures the pieplined picture info always reflect the latest
     * to support smooth transition and pipelining; */
    pWriter = &hWindow->stMadDelayed[ulPictureIdx][ulDeferIdxWr];

    pWriter->stSrcOut = pPicture->stSrcOut;
    pWriter->ulOrigPTS = pPicture->ulOrigPTS;

    /* shortcut(no delay) ignorePicture and stallStc from input to output since those require immediate action at STG */
    pWriter->bLast                = pPicture->bLast;
    pWriter->bChannelChange       = pPicture->bChannelChange;
    pWriter->bMute                = pPicture->bMute;
    pWriter->ulDecodePictureId    = pPicture->ulDecodePictureId;
    pWriter->ulStgPxlAspRatio_x_y = pPicture->ulStgPxlAspRatio_x_y;
    pWriter->ePictureType         = pPicture->ePictureType;
    pWriter->eBarDataType         = pPicture->eBarDataType;
    pWriter->ulTopLeftBarValue    = pPicture->ulTopLeftBarValue;
    pWriter->ulBotRightBarValue   = pPicture->ulBotRightBarValue;
    pWriter->bPreChargePicture    = pPicture->bPreChargePicture;
    pWriter->bEndofChunk          = pPicture->bEndofChunk;
    pWriter->ulChunkId            = pPicture->ulChunkId;
    pWriter->eDispOrientation     = pPicture->eDispOrientation;
    BCFC_CopyColorSpace_isrsafe(&pWriter->stMosaicColorSpace, &pPicture->astMosaicColorSpace[ulPictureIdx]);

    /* It is very important for modules to read the correctly delayed picture info! */
    if (usMadVsyncDelay > 0)
    {
        pReader = &hWindow->stMadDelayed[ulPictureIdx][ulDeferIdxRd];


        if(hWindow->ulDropCntNonIgnoredPics)
        {
            pPicture->bIgnorePicture = false;
            hWindow->ulDropCntNonIgnoredPics --;
        }

        pPicture->stSrcOut             = pReader->stSrcOut;
        pPicture->ulOrigPTS            = pReader->ulOrigPTS;
        pPicture->bLast                = pReader->bLast;
        pPicture->bChannelChange       = pReader->bChannelChange;
        pPicture->bMute                = pReader->bMute;
        pPicture->ulDecodePictureId    = pReader->ulDecodePictureId;
        pPicture->ulStgPxlAspRatio_x_y = pReader->ulStgPxlAspRatio_x_y;
        pPicture->ePictureType         = pReader->ePictureType;
        pPicture->eBarDataType         = pReader->eBarDataType;
        pPicture->ulTopLeftBarValue    = pReader->ulTopLeftBarValue;
        pPicture->ulBotRightBarValue   = pReader->ulBotRightBarValue;
        pPicture->bPreChargePicture    = pReader->bPreChargePicture;
        pPicture->bEndofChunk          = pReader->bEndofChunk;
        pPicture->ulChunkId            = pReader->ulChunkId;
        pPicture->eDispOrientation     = pReader->eDispOrientation;
        BCFC_CopyColorSpace_isrsafe(&pPicture->astMosaicColorSpace[ulPictureIdx], &pReader->stMosaicColorSpace);
      #if BVDC_P_DBV_SUPPORT
        pPicture->astMosaicMetaData[ulPictureIdx].stDbvInput.stHdrMetadata.eType = BAVC_HdrMetadataType_eUnknown;
      #elif BVDC_P_TCH_SUPPORT
        pPicture->astMosaicMetaData[ulPictureIdx].stTchInput.stHdrMetadata.eType = BAVC_HdrMetadataType_eUnknown;
      #endif

        /* If Mpeg Src and no capture, reduce the MAD delay by 1 to achieve */
        /* -1 delay histogram and dynamic contrast */
        if(!BVDC_P_VNET_USED_CAPTURE(pPicture->stVnetMode))
        {
            ulDeferHistIdxRd = ulDeferIdxRd;
            if(((ulDeferIdxWr + BVDC_MADDELAY_BUF_COUNT - ulDeferHistIdxRd) & ulMaxMadDelayBufIdx) > (uint32_t)(usMadVsyncDelay - 1))
            {
                ulDeferHistIdxRd = (ulDeferIdxWr + BVDC_MADDELAY_BUF_COUNT - (usMadVsyncDelay - 1)) & ulMaxMadDelayBufIdx; /* % 4 */
            }
            pReader = &hWindow->stMadDelayed[ulPictureIdx][ulDeferHistIdxRd];
        }
    }
}


/* -----------------------------------------------------------------------------------
 * {private}
 *
 * This function used to align a pre-mad rectangles (left, width) or (top, height) to
 * a 1, 2, or 4 pixel/line boundary. Here is the explain to the parameters:
 *
 * min:       left or top, a fixed point number with fraction bits number minFracBits
 * size:      width or height, an integer number (no fraction bits)
 * fullSize:  the original full size. Interval [min, min+size] is inside [0, fullSize]
 * alignUnit: constant 1, 2 or 4, min and max will be aligned to alignUnit boundary
 * newMin:    aligned min, integer number, no fraction bits
 * newSize:   aligned size, an integer number (no fraction bits)
 *
 * In TV app, we might zoon into part of source rect, and then pan the zoom rect left
 * and right, top and bottom. If we simply align min to floor, max to ceiling, the
 * aligned size (i.e. max - min) will change back and forward even if the zoom rect size
 * does not change and is still fully inside the src rect. Since input size change leads
 * to MAD hard-start (flash), we want to avoid it.
 *
 * When the zoom rect is panned through the src rect, the biggest new size caused by
 * min and max alignment is
 *     ceil(size) + alignUnit.
 * This can be seen in the following picture, for ulSize = 1, 2, 3, 4, 5, 6, 7, 8.
 *
 *   |               |               |               |               |
 *   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16
 *   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
 *
 * We make alinged size always be biggest, so it will not change as panning in the src.
 */
void BVDC_P_Window_AlignPreMadRect_isr
    ( int32_t                          lMin,
      int32_t                          lMin_R,
      uint32_t                         ulSize,
      uint32_t                         ulFullSize,
      uint32_t                         ulMinFracBits,
      uint32_t                         ulAlignUnit,
      int32_t                         *plNewMin,
      int32_t                         *plNewMin_R,
      uint32_t                        *pulNewSize )
{
    uint32_t  ulFracAlignUnit, ulMax;
    uint32_t  ulNewMin, ulNewSize, ulNewMax;
    uint32_t  ulNewMin_R;

    ulFracAlignUnit = ulAlignUnit << ulMinFracBits;
    ulNewMin = BVDC_P_ALIGN_DN(lMin, ulFracAlignUnit) >> ulMinFracBits;
    ulNewMin_R = BVDC_P_ALIGN_DN(lMin_R, ulFracAlignUnit) >> ulMinFracBits;

    ulNewSize = ulAlignUnit + BVDC_P_ALIGN_UP(ulSize, ulAlignUnit);

    /* no longer need to maintain MAD input size if src edge is reached */
    ulNewMax = ulNewMin + ulNewSize;
    if ((ulNewMax >= ulFullSize) || (0 == ulNewMin))
    {
        ulMax = ulSize + ((lMin + (1 << ulMinFracBits) - 1) >> ulMinFracBits);

        /* take off the extra ulAlignUnit added above if OK */
        ulNewSize = (((ulMax + ulAlignUnit) <= ulNewMax) ||
                     (ulNewMax > BVDC_P_ALIGN_UP(ulFullSize, ulAlignUnit)))?
            (ulNewSize - ulAlignUnit) : ulNewSize;
    }

#if (!BVDC_P_MADR_HSIZE_WORKAROUND)
    /* expanding can not go beyond FullSize */
    ulNewSize = BVDC_P_MIN(ulNewSize, ulFullSize - ulNewMin);
#endif

    if(plNewMin)
        *plNewMin = (int32_t)ulNewMin;

    if(plNewMin_R)
        *plNewMin_R = (int32_t)ulNewMin_R;

    if(pulNewSize)
        *pulNewSize = ulNewSize;
}

#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
/***************************************************************************
 * {private}
 *
 * This function is the software workaround for HW7425-869 to align a
 * pre-mad rectangles width. The adjustment of the width will be always
 * multiple of 4.
 *
 */
void BVDC_P_Window_PreMadAdjustWidth_isr
    ( uint32_t                  ulPicWidth,
      uint32_t                  ulPicHeight,
      uint32_t                  ulBitsPerGroup,
      uint32_t                  ulPixelPerGroup,
      uint32_t                 *pulNewPicWidth )
{
    bool      bBadAlignment, bBadAlignment2;
    uint32_t  ulRemainder, ulRemainder2, ulNewPicWidth = ulPicWidth;
    /* Minimum is 1/40 = 0.025, 8 bits fraction should be enough */
    uint32_t    ulAlign = 8;
    uint32_t    i, ulDelta, ulIncrease;

    ulRemainder = BVDC_P_MADR_GET_REMAINDER(ulPicWidth, ulPicHeight);
    bBadAlignment = BVDC_P_MADR_BAD_ALIGNMENT(ulRemainder);

#if (BVDC_P_MADR_VARIABLE_RATE)
    ulRemainder2 = BVDC_P_MADR_GET_VARIABLE_RATE_REMAINDER(ulPicWidth,
        ulPicHeight, ulBitsPerPixel);
    bBadAlignment2 = BVDC_P_MADR_VARIABLE_RATE_BAD_ALIGNMENT(ulRemainder2, ulBitsPerPixel);
#else
    ulRemainder2 = BVDC_P_MADR_GET_FIX_RATE_REMAINDER(ulPicWidth,
        ulPicHeight, ulBitsPerGroup, ulPixelPerGroup);
    bBadAlignment2 = BVDC_P_MADR_FIX_RATE_BAD_ALIGNMENT(ulRemainder2, ulBitsPerGroup, ulPixelPerGroup);
#endif

    BDBG_MODULE_MSG(BVDC_MADR_PICSIZE,("BVDC_P_Window_PreMadAdjustWidth_isr: %dx%dx %d %d: %d %d",
        ulNewPicWidth, ulPicHeight, ulBitsPerGroup, ulPixelPerGroup, ulRemainder, ulRemainder2));

    while(bBadAlignment || bBadAlignment2)
    {
        if(bBadAlignment)
        {
            /* Start algo to adjust width. See HW7425-869 for details. */
            /* Determine increment of remainder value when we increase
             * picture_width by 4 pixels. */
            ulDelta = 40 - ((ulPicHeight + 1) % 40);

            /* Determine how many steps needed to get out of the hang zone [32,39].
             * if ulDelta <= ulReminder, ulDelta is negative so we use 31 as base.
             * if ulDelta > ulReminder, the updated remainder value will wrap around
             * and ulDelta becomes positive */

            if(ulDelta < ulRemainder)
            {
                i = BVDC_P_MAKE_FIXED_POINT(ulRemainder - 31, ulDelta, ulAlign);
            }
            else
            {
                ulDelta = 40 - ulDelta;
                i = BVDC_P_MAKE_FIXED_POINT(40 - ulRemainder, ulDelta, ulAlign);
            }

            ulIncrease = BVDC_P_ALIGN_UP(i, 1<<ulAlign);
            BDBG_MODULE_MSG(BVDC_MADR_PICSIZE,(
                "Delta: %d, i: 0x%x, Round up i: %d", ulDelta, i, ulIncrease));
            ulIncrease = ulIncrease >> ulAlign;
            ulNewPicWidth += 4 * ulIncrease;

        }

        if(bBadAlignment2)
        {
            ulNewPicWidth += 4;
        }

        ulRemainder = BVDC_P_MADR_GET_REMAINDER(ulNewPicWidth, ulPicHeight);
        bBadAlignment = BVDC_P_MADR_BAD_ALIGNMENT(ulRemainder);
#if (BVDC_P_MADR_VARIABLE_RATE)
        ulRemainder2 = BVDC_P_MADR_GET_VARIABLE_RATE_REMAINDER(ulNewPicWidth,
            ulPicHeight, ulBitsPerGroup);
        bBadAlignment2 = BVDC_P_MADR_VARIABLE_RATE_BAD_ALIGNMENT(ulRemainder2, ulBitsPerGroup);
#else
        ulRemainder2 = BVDC_P_MADR_GET_FIX_RATE_REMAINDER(ulNewPicWidth,
            ulPicHeight, ulBitsPerGroup, ulPixelPerGroup);
        bBadAlignment2 = BVDC_P_MADR_FIX_RATE_BAD_ALIGNMENT(ulRemainder2, ulBitsPerGroup, ulPixelPerGroup);
#endif

        BDBG_MODULE_MSG(BVDC_MADR_PICSIZE,("After adjustment 2: %dx%d: %d, %d",
            ulNewPicWidth, ulPicHeight, ulRemainder, ulRemainder2));
    }

    if(pulNewPicWidth)
        *pulNewPicWidth = ulNewPicWidth;
}

#endif

/***************************************************************************
 * {private}
 *
 * This function combine the Bar data source_info & user_info, and put it into a
 * picture node to encode bar user data.
 *
 * Note: this function is intended for video window only!
 */
static void BVDC_P_Window_UpdateBarData_isr
        ( BVDC_Window_Handle               hWindow,
          BVDC_P_PictureNode              *pPicture,
          const BAVC_MVD_Field            *pMvdFieldData)
{
    BAVC_BarDataType                 eBarDataType;
    uint32_t                         ulTopLeftBarValue,ulBotRightBarValue;
    bool                             bHorizontalBar = false, bVerticalBar = false, bValidBarData= true;
    BVDC_P_Rect                     *pWinOut, *pScanOut, *pSclCut, stCapOut;
    uint32_t                        ulDstLeft, ulDstRight, ulDstTop, ulDstBottom;
    uint32_t                        ulSrcLeft, ulSrcTop,ulSrcCutTopLeft;
    uint32_t                        ulSrcRight, ulSrcBottom,ulSrcCutBotRight;
    uint32_t                        ulCmpCanvasWidth, ulCmpCanvasHeight;
    uint32_t                        ulBarValue;

    BDBG_ENTER(BVDC_P_Window_UpdateBarData_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    eBarDataType           = pMvdFieldData->eBarDataType;
    ulTopLeftBarValue      = pMvdFieldData->ulTopLeftBarValue;
    ulBotRightBarValue     = pMvdFieldData->ulBotRightBarValue;
    ulBarValue             = pMvdFieldData->ulTopLeftBarValue + pMvdFieldData->ulBotRightBarValue;

    bValidBarData          =(BAVC_BarDataType_eLeftRight==eBarDataType)?
        (ulBarValue < pMvdFieldData->ulDisplayHorizontalSize ):
        ((BAVC_BarDataType_eTopBottom==eBarDataType)?
        (ulBarValue < pMvdFieldData->ulDisplayVerticalSize) :true);

    pScanOut = &pPicture->stSrcOut;
    pSclCut  = &pPicture->stSclCut;
    pWinOut  = &hWindow->stAdjDstRect;
    stCapOut.lTop = stCapOut.lLeft=stCapOut.ulWidth=stCapOut.ulHeight=0;
    if((hWindow->bCapture) && (pPicture->pCapOut))
    {
        stCapOut.lLeft    = pPicture->pCapOut->lLeft;
        stCapOut.lTop     = pPicture->pCapOut->lTop;
        stCapOut.ulWidth  = pPicture->pCapOut->ulWidth;
        stCapOut.ulHeight = pPicture->pCapOut->ulHeight;
    }

    /* construct the compositor canvas rectangular. this is called at writer_isr */
    ulCmpCanvasHeight = (BVDC_P_DISPLAY_USED_DIGTRIG(hWindow->hCompositor->hDisplay->eMasterTg)?
            hWindow->hCompositor->hDisplay->stCurInfo.pFmtInfo->ulDigitalHeight:
            hWindow->hCompositor->hDisplay->stCurInfo.pFmtInfo->ulHeight);

    ulCmpCanvasWidth  = (BVDC_P_DISPLAY_USED_DIGTRIG(hWindow->hCompositor->hDisplay->eMasterTg)?
            hWindow->hCompositor->hDisplay->stCurInfo.pFmtInfo->ulDigitalWidth:
            hWindow->hCompositor->hDisplay->stCurInfo.pFmtInfo->ulWidth);

    ulSrcCutTopLeft = (eBarDataType == BAVC_BarDataType_eLeftRight)?(pScanOut->lLeft + pSclCut->lLeft + stCapOut.lLeft):
                    (eBarDataType == BAVC_BarDataType_eTopBottom?(pScanOut->lTop + pSclCut->lTop + stCapOut.lTop):0);
    ulSrcLeft = (eBarDataType == BAVC_BarDataType_eLeftRight)?ulTopLeftBarValue:0;
    ulSrcTop  = (eBarDataType == BAVC_BarDataType_eTopBottom)?ulTopLeftBarValue:0;

    ulSrcCutBotRight = 0;
    if(eBarDataType == BAVC_BarDataType_eLeftRight)
    {
        ulSrcCutBotRight = pMvdFieldData->ulDisplayHorizontalSize - ulSrcCutTopLeft - pSclCut->ulWidth;
    }
    else if(eBarDataType == BAVC_BarDataType_eTopBottom)
    {
        ulSrcCutBotRight = pMvdFieldData->ulDisplayVerticalSize - ulSrcCutTopLeft - pSclCut->ulHeight;
    }

    ulSrcRight  = (eBarDataType == BAVC_BarDataType_eLeftRight)?ulBotRightBarValue:0;
    ulSrcBottom = (eBarDataType == BAVC_BarDataType_eTopBottom)?ulBotRightBarValue:0;


    ulSrcLeft = (ulSrcCutTopLeft < ulSrcLeft)?(ulSrcLeft - ulSrcCutTopLeft):0;
    ulSrcTop  = (ulSrcCutTopLeft < ulSrcTop)?(ulSrcTop - ulSrcCutTopLeft):0;
    ulSrcRight = (ulSrcCutBotRight <ulSrcRight)?(ulSrcRight - ulSrcCutBotRight) : 0;
    ulSrcBottom= (ulSrcCutBotRight <ulSrcBottom)?(ulSrcBottom- ulSrcCutBotRight) : 0;



    ulDstLeft = pWinOut->lLeft + ulSrcLeft*pWinOut->ulWidth/pSclCut->ulWidth;
    ulDstTop  = pWinOut->lTop + ulSrcTop*pWinOut->ulHeight/pSclCut->ulHeight;
    ulDstRight = ulCmpCanvasWidth - pWinOut->ulWidth - pWinOut->lLeft + ulSrcRight*pWinOut->ulWidth/pSclCut->ulWidth;
    ulDstBottom = ulCmpCanvasHeight - pWinOut->ulHeight - pWinOut->lTop + ulSrcBottom*pWinOut->ulHeight/pSclCut->ulHeight;

    bHorizontalBar = ulDstLeft || ulDstRight;
    bVerticalBar   = ulDstTop  || ulDstBottom;

    if((bHorizontalBar ^ bVerticalBar) && bValidBarData)
    {
        pPicture->eBarDataType = bHorizontalBar ?
            BAVC_BarDataType_eLeftRight: BAVC_BarDataType_eTopBottom;
        pPicture->ulTopLeftBarValue = bHorizontalBar ? ulDstLeft:ulDstTop;
        pPicture->ulBotRightBarValue = bHorizontalBar ? ulDstRight:ulDstBottom;

        if(bHorizontalBar && bVerticalBar)
            BDBG_MODULE_WRN(BVDC_BAR, ("Both Horizontal and verital have bars Hor %d %d Ver %d %d ",
            ulDstLeft, ulDstRight, ulDstTop, ulDstBottom));
    }
    else
    {
        pPicture->eBarDataType = BAVC_BarDataType_eInvalid;
        pPicture->ulTopLeftBarValue = 0;
        pPicture->ulBotRightBarValue = 0;
    }

    BDBG_MODULE_MSG(BVDC_BAR, ("input Src %d x %d eBarDataType %d: %d - %d",
        pMvdFieldData->ulDisplayHorizontalSize, pMvdFieldData->ulDisplayVerticalSize,
        pMvdFieldData->eBarDataType, pMvdFieldData->ulTopLeftBarValue, pMvdFieldData->ulBotRightBarValue));
    BDBG_MODULE_MSG(BVDC_BAR, ("win[%d] Rectangular: SrcScan %d %d %d x %d Cap %d %d %d x %d  SclCut %d %d %d x %d Dsp %d %d %d x %d Canvas %d x %d ",
        hWindow->eId,
        pScanOut->lTop, pScanOut->lLeft, pScanOut->ulWidth, pScanOut->ulHeight,
        stCapOut.lTop, stCapOut.lLeft, stCapOut.ulWidth, stCapOut.ulHeight,
        pSclCut->lTop, pSclCut->lLeft, pSclCut->ulWidth, pSclCut->ulHeight,
        pWinOut->lTop, pWinOut->lLeft, pWinOut->ulWidth, pWinOut->ulHeight,
        ulCmpCanvasWidth, ulCmpCanvasHeight));
    BDBG_MODULE_MSG(BVDC_BAR, ("win[%d] Bar info: Src Hor %d %d Ver %d %d Dst Hor %d %d Ver %d %d",
        hWindow->eId,
        ulSrcLeft, ulSrcRight, ulSrcTop, ulSrcBottom,
        ulDstLeft, ulDstRight, ulDstTop, ulDstBottom));
    BDBG_MODULE_MSG(BVDC_BAR, ("win[%d] output pic[%d] Bar type %d value %d %d ",
        hWindow->eId,
        pPicture->ulDecodePictureId, pPicture->eBarDataType, pPicture->ulTopLeftBarValue, pPicture->ulBotRightBarValue));

    BDBG_LEAVE(BVDC_P_Window_UpdateBarData_isr);
}

/***************************************************************************
 * {private}
 *
 * Get max width/height of supportable by MCVP/MADR of hWindow.
 * The size will be bounded by HW and BOXMODE.  Including the case for
 *     - Non-Mosaic vs Mosaic
 *     - 7250's special Main/PIP using Mosaic
 *     - STG/Encoder special case.
 */
static void BVDC_P_Window_GetDeinterlacerMaxResolution_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_MVD_Field            *pMvdFieldData,
      uint32_t                        *pulMaxWidth,
      uint32_t                        *pulMaxHeight,
      uint32_t                        *pulHsclSizeThreshold,
      bool                             bApplyRestriction )
{
    uint32_t ulMaxWidth, ulMaxHeight, ulHsclSizeThreshold;

    ulMaxWidth  = BFMT_PAL_WIDTH;
    ulMaxHeight = BFMT_PAL_HEIGHT;
    ulHsclSizeThreshold = BVDC_P_MAD_SRC_HORZ_THRESHOLD;
    BSTD_UNUSED(pMvdFieldData); /* hush warnings */

    if(hWindow->stCurResource.hMcvp)
    {
        ulMaxWidth  = hWindow->stCurResource.hMcvp->ulMaxWidth;
        ulMaxHeight = hWindow->stCurResource.hMcvp->ulMaxHeight;
        ulHsclSizeThreshold = hWindow->stCurResource.hMcvp->ulHsclSizeThreshold;
    }

    if(bApplyRestriction)
    {
#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
        if(hWindow->stCurInfo.bMosaicMode)
        {
            if(hWindow->eWindowClass != BBOX_Vdc_WindowClass_eLegacy)
            {
                uint32_t   ulIndex;

                ulIndex = hWindow->stCurInfo.ulMaxMosaicCount - 1;
                ulMaxWidth = hWindow->pWindowClassLimit->mosaicRects[ulIndex].stDeinterlacer.ulWidth;
                ulMaxHeight = hWindow->pWindowClassLimit->mosaicRects[ulIndex].stDeinterlacer.ulHeight;
            }
            else
            {
#if (BCHP_CHIP == 7250)
                /* KLUDGY: special boxmode handling */
                /* Only 1st Channel Mosaic channel get the deintelacer */
                if((8 == hWindow->hCompositor->hVdc->stBoxConfig.stBox.ulBoxId) &&
                   (0 != pMvdFieldData->ulChannelId))
                {
                    ulMaxWidth  = 0;
                    ulMaxHeight = 0;
                }
#else
                /* ALL CHIP: ulMosaicMaxChannels mosaic pal support for multipip */
                if((hWindow->stCurResource.hMcvp) &&
                   (hWindow->stCurInfo.ulMosaicCount <= hWindow->stCurResource.hMcvp->hMcdi->ulMosaicMaxChannels))
                {
                    ulMaxWidth  = BFMT_PAL_WIDTH;
                    ulMaxHeight = BFMT_PAL_HEIGHT;
                }
#endif
                else
                {
                    ulMaxWidth  = 0;
                    ulMaxHeight = 0;
                }
            }
        }
#endif

#if (BVDC_P_SUPPORT_STG)
        /* SW7425-973: (1) USAGE POLICY: Further restrict to SD input base
         * deinterlacing if output to vice.  Need to re-consider expose to
         * API if usage of MAD-R becomes too non-orthogonal and
         * customer/platform dependence. */
        if(hWindow->stCurResource.hMcvp)
        {
            const BFMT_VideoInfo *pDstFmt = hWindow->hCompositor->stCurInfo.pFmtInfo;
            const BBOX_Vdc_Capabilities *pBoxVdc =
                &hWindow->hCompositor->hVdc->stBoxConfig.stVdc;

            if((hWindow->stCurResource.hMcvp->hMcdi->bMadr) &&
               (hWindow->hCompositor->hDisplay->stCurInfo.bEnableStg) &&
               ((hWindow->stAdjSclOut.ulHeight >> pDstFmt->bInterlaced) <= BFMT_720P_HEIGHT) &&
               (pBoxVdc->astDeinterlacer[hWindow->stCurResource.hMcvp->eId].stPictureLimits.ulHeight == BBOX_VDC_DISREGARD))
            {
                ulMaxWidth  = BFMT_PAL_WIDTH;
                ulMaxHeight = BFMT_PAL_HEIGHT;
            }
        }
#endif
    }

    if(pulMaxWidth)
    {
        *pulMaxWidth = ulMaxWidth;
    }
    if(pulMaxHeight)
    {
        *pulMaxHeight = ulMaxHeight;
    }
    if(pulHsclSizeThreshold)
    {
        *pulHsclSizeThreshold = ulHsclSizeThreshold;
    }

    if(bApplyRestriction) {
        BDBG_MSG(("ulMaxWidth = %d, ulMaxHeight = %d, ulHsclSizeThreshold = %d",
            ulMaxWidth, ulMaxHeight, ulHsclSizeThreshold));
    }

    return;
}


/***************************************************************************
 * {private}
 *
 * This function combine the source_info & user_info, and put it into a
 * picture node.
 * This determine if the pPicture will required a capture/scaler and
 * whether if scaler is needed before capture.  It merges the applied
 * user parameters from BVDC_ApplyChanges_isr(), and source parameters.
 *
 * Note: this function is intended for video window only!
 */
static void BVDC_P_Window_UpdateSrcAndUserInfo_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_PictureNode              *pPicture,
      const BAVC_MVD_Field            *pMvdFieldData,
      const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
      const BAVC_Polarity              eFieldId,
      uint32_t                         ulPictureIdx)
{
    const BVDC_P_Window_Info       *pUserInfo;
    const BFMT_VideoInfo           *pSrcFmtInfo;
    BVDC_Source_Handle              hSource;
    int32_t                         lCntTop, lCntLeft;
    int32_t                         lCntLeft_R;
    uint32_t                        ulCntWidth, ulCntHeight;
    bool                            bDispFmtIs3d;
    uint32_t                        ulMinCapHsize, ulMinCapVsize;
    uint32_t                        ulMinVfdHsize, ulMinVfdVsize;
    BAVC_MatrixCoefficients         eMatrixCoefficients = BAVC_MatrixCoefficients_eItu_R_BT_709;
#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
    bool                            bMvpBypass = false;
#endif

    BDBG_ENTER(BVDC_P_Window_UpdateSrcAndUserInfo_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    hSource = hWindow->stCurInfo.hSource;
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    /* Get user applied informations. */
    pUserInfo   = &hWindow->stCurInfo;
    pSrcFmtInfo = hSource->stCurInfo.pFmtInfo;
    hSource->bSrcInterlaced =
        ((BVDC_P_SRC_IS_MPEG(hSource->eId) && (NULL != pMvdFieldData))?
        (BAVC_Polarity_eFrame != pMvdFieldData->eSourcePolarity) :
        (pSrcFmtInfo->bInterlaced));
    pPicture->ulPixelCount = hSource->ulPixelCount;

    /* inform SCL to avoid point sampling if field inversion is expected */
    pPicture->stFlags.bHandleFldInv = hWindow->stCadHndl.bHandleFldInv;

    /* setup the PicComRulInfo that is shared by all sub-modules when they build RUL */
    pPicture->PicComRulInfo.PicDirty.stBits.bInputFormat =
        hSource->stCurInfo.stDirty.stBits.bInputFormat;
    pPicture->PicComRulInfo.PicDirty.stBits.bSrcPicChange =
        hSource->bPictureChanged; /* either src detected change or ApplyChange called for the win */
    bDispFmtIs3d = BFMT_IS_3D_MODE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt);



#if BVDC_P_SUPPORT_MOSAIC_MODE
    /* MosaicMode: update ClearRect mask set; */
    if(pUserInfo->bMosaicMode && (NULL != pMvdFieldData))
    {
        uint32_t      i, ulIndex;

        if((pMvdFieldData->ulChannelId < pUserInfo->ulMosaicCount) &&
           (pUserInfo->abMosaicVisible[pMvdFieldData->ulChannelId]))
        {
            pPicture->ulMosaicRectSet |= 1 << pMvdFieldData->ulChannelId;
        }

        pPicture->bMosaicMode = pUserInfo->bMosaicMode;
        pPicture->ulMosaicCount = pUserInfo->ulMosaicCount;

        for(i = 0; i < pUserInfo->ulMosaicCount; i++)
        {
            ulIndex = hSource->aulChannelId[i];
            pPicture->astMosaicRect[i] = pUserInfo->astMosaicRect[ulIndex];
            pPicture->abMosaicVisible[i] = pUserInfo->abMosaicVisible[ulIndex];
        }

        if(pUserInfo->ulMosaicTrackChannelId == pMvdFieldData->ulChannelId)
        {
            hSource->eFrameRateCode = pMvdFieldData->eFrameRateCode;
            /* Correct way is to convert indivisual input color space from
             * pFieldData->eMatrixCoefficients to tracked channel color space
             * defined by hSource->eMatrixCoefficients. Since we don't have all
             * the matrices yet, just convert all to HD.
             */
#if 0
            hSource->eMatrixCoefficients = pMvdFieldData->eMatrixCoefficients;
#else
            hSource->eMatrixCoefficients = BAVC_MatrixCoefficients_eItu_R_BT_709;
#endif
        }
        if(pUserInfo->ulMosaicCount > 1)
            pPicture->bMosaicIntra = hSource->stCurInfo.stDirty.stBits.bMosaicIntra;
        else
            pPicture->bMosaicIntra = false;
    }
    /* Note mosaic mode and ClearRect setting is preferred to eEnableBackgroundBars */
    else if(BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode) &&
        (BVDC_Mode_eOff != pUserInfo->eEnableBackgroundBars) &&
        (0 < pUserInfo->ucZOrder) &&
        ((pUserInfo->stDstRect.ulWidth > hWindow->stAdjSclOut.ulWidth) ||
         (pUserInfo->stDstRect.ulHeight> hWindow->stAdjSclOut.ulHeight))
#if BVDC_P_SUPPORT_STG /* no window background clear for transcoder */
        && !BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg)
#endif
    )
    {
        /* reuse pPicture bMosaicMode flag to clear window background */
        pPicture->bMosaicMode = true;
        pPicture->bMosaicIntra = false;
        pPicture->ulMosaicCount = 1;
        pPicture->ulMosaicRectSet = 1;
        pPicture->astMosaicRect[0] = hWindow->stAdjSclOut;
        /* Note, AdjustRectangle has calculated aspect ratio corrected window sizes in stAdjSclOut;
          so given window user setting stSclerOutput >= stAdjSclOut/stAdjDstRect, the window background
          bars clip size can be calculated by centering the adjusted rect within the user setting rect; */
        pPicture->astMosaicRect[0].lLeft = (hWindow->stAdjDstRect.lLeft - pUserInfo->stDstRect.lLeft);
        pPicture->astMosaicRect[0].lTop  = (hWindow->stAdjDstRect.lTop - pUserInfo->stDstRect.lTop);

        /* adjust mosaic rect size and offset alignment with workarounds */
        BVDC_P_Window_AdjustMosaicRect_isrsafe(hWindow,
            hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced, &pPicture->astMosaicRect[0]);

        pPicture->abMosaicVisible[0] = true;
        /* mark the flag */
        hWindow->bBarsToBeFilled = true;
    }
    else
    {
        pPicture->bMosaicMode = false;
        pPicture->bMosaicIntra = false;
        pPicture->ulMosaicCount = 0;
        if(hWindow->bBarsToBeFilled && BVDC_Mode_eOn == pUserInfo->eEnableBackgroundBars &&
           ((pUserInfo->stDstRect.ulWidth > hWindow->stAdjSclOut.ulWidth) ||
            (pUserInfo->stDstRect.ulHeight> hWindow->stAdjSclOut.ulHeight)))
        {
            hWindow->bBarsToBeFilled = false;
            BDBG_ERR(("Win[%d] bars are not filled! zorder=%u, scl@w=%d, dst:%ux%u, adjScl=%ux%u",
                hWindow->eId, pUserInfo->ucZOrder, BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode),
                pUserInfo->stDstRect.ulWidth, pUserInfo->stDstRect.ulHeight,
                hWindow->stAdjSclOut.ulWidth, hWindow->stAdjSclOut.ulHeight));
        }
    }
#endif

    /* Update current picture node's pixel format to current pixel format. */
    pPicture->ePixelFormat = pUserInfo->ePixelFormat;
    pPicture->ucAlpha = pUserInfo->bVisible ? pUserInfo->ucAlpha : 0;

    pPicture->eMadPixelHeapId     = hWindow->eMadPixelHeapId[ulPictureIdx];
    pPicture->eMadQmHeapId        = hWindow->eMadQmHeapId[ulPictureIdx];
    pPicture->usMadPixelBufferCnt = hWindow->usMadPixelBufferCnt[ulPictureIdx];
    pPicture->usMadQmBufCnt       = hWindow->usMadQmBufCnt[ulPictureIdx];
    pPicture->ulMadPxlBufSize     = hWindow->ulMadPxlBufSize[ulPictureIdx];
    pPicture->bContinuous         = hWindow->bContinuous[ulPictureIdx];

    if(BVDC_P_IS_CUSTOMFMT(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
    {
        pPicture->stCustomFormatInfo = hWindow->hCompositor->hDisplay->stCurInfo.stCustomFmt;
    }

#if BVDC_P_SUPPORT_STG /* writer passes along the current display fmt info to stg */
    if(BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg))
    {
        pPicture->pStgFmtInfo = (BFMT_VideoInfo*)hWindow->hCompositor->stCurInfo.pFmtInfo;
    }
#endif
    /* (1) This function is only called by BVDC_P_Window_Writer_isr, which
     * is intended for video window only! */
    if((BVDC_P_SRC_IS_MPEG(hSource->eId)) && (pMvdFieldData))
    {
        /* Build the picture */
        pPicture->stFlags.bMuteFixedColor        = false;
        pPicture->stFlags.bMuteMad               = pMvdFieldData->bMute;
        if(pMvdFieldData->bInvisible && !pPicture->bMosaicMode)
        {
            pPicture->ucAlpha = 0;
        }
        pPicture->ulAdjQp                        = pMvdFieldData->ulAdjQp;
        pPicture->eSrcPolarity                   = pMvdFieldData->eSourcePolarity;
        pPicture->PicComRulInfo.eSrcOrigPolarity = pMvdFieldData->eSourcePolarity;
        pPicture->ulOrigPTS                      = pMvdFieldData->ulOrigPTS;
        pPicture->ulChannelId                    = pMvdFieldData->ulChannelId;
        pPicture->ulPictureIdx                   = ulPictureIdx;
        /* don't reset bvn blocks for non-transcode paths */
        pPicture->PicComRulInfo.bNoCoreReset     = !hWindow->hCompositor->hDisplay->stCurInfo.bEnableStg;

        /*Mailbox data for ViCE2*/
        /*@@@ handle from hdmi input*/
        pPicture->iHorzPanScan                   = pMvdFieldData->i32_HorizontalPanScan;
        pPicture->iVertPanScan                   = pMvdFieldData->i32_VerticalPanScan;
        pPicture->ulDispHorzSize                 = pMvdFieldData->ulDisplayHorizontalSize;
        pPicture->ulDispVertSize                 = pMvdFieldData->ulDisplayVerticalSize;
        pPicture->ePictureType                   = pMvdFieldData->ePictureType;
        pPicture->bIgnorePicture                 = pMvdFieldData->bIgnorePicture ;
        pPicture->bStallStc                      = pMvdFieldData->bStallStc;
        pPicture->bLast                          = pMvdFieldData->bLast;
        pPicture->bChannelChange                 = pMvdFieldData->bChannelChange;
        pPicture->bMute                          = pMvdFieldData->bMute;
        pPicture->ulPicOrderCnt                  = pMvdFieldData->int32_PicOrderCnt;
        pPicture->ulDecodePictureId              = pMvdFieldData->ulDecodePictureId;
        pPicture->ulStgPxlAspRatio_x_y           = hWindow->ulStgPxlAspRatio_x_y;
        pPicture->bValidAfd                      = pMvdFieldData->bValidAfd;
        pPicture->ulAfd                          = pMvdFieldData->ulAfd;
        pPicture->bPreChargePicture              = pMvdFieldData->bPreChargePicture;
        pPicture->bEndofChunk                    = pMvdFieldData->bEndOfChunk;
        pPicture->ulChunkId                      = pMvdFieldData->ulChunkId;
        pPicture->eBitDepth                      = pMvdFieldData->eBitDepth;
        pPicture->eChromaBitDepth                = pMvdFieldData->eChromaBitDepth;

        /* hSource->hMpegFeeder->eOutputOrientation is set in BVDC_P_Feeder_SetMpegInfo_isr */
        pPicture->eSrcOrientation = hSource->hMpegFeeder->eOutputOrientation;
        pPicture->eOrigSrcOrientation = pPicture->eSrcOrientation;

        /* increase window based enc picture id */
        hWindow->ulEncPicId +=!pMvdFieldData->bIgnorePicture;
        hWindow->ulDecPicId = pMvdFieldData->ulDecodePictureId;
        hWindow->ulSourceRate = hSource->ulVertFreq;

        if(BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode) &&
            (BFMT_Orientation_e3D_LeftRight == pPicture->eSrcOrientation) &&
            (false == BVDC_P_Scaler_Validate_VertDepth_isr(hWindow, hWindow->stCurResource.hScaler)))
        {
            pPicture->eOrigSrcOrientation = pPicture->eSrcOrientation;
            pPicture->eSrcOrientation = pPicture->eDispOrientation;
        }
        /* Note: the following logic is intended to detect field repeat to
           properly config MAD in C0 to support 50to60Hz or trick mode deinterlacing;
           vdc detects the field repeat by checking both polarity and frame
           address; might be improved by checking the detection flags sent from mvd
           field data structure when it's available; */
        /* Note, the muted picture during channel change should go through multi-buffer
           without being dropped for sync-slipped window; */
        pPicture->stFlags.bPictureRepeatFlag = pMvdFieldData->bPictureRepeatFlag &&
            !pMvdFieldData->bMute;
        pPicture->stFlags.bRepeatField = pMvdFieldData->bRepeatField;
        /* Phase 3 MAD chroma detection */
        pPicture->eChromaType          = (BAVC_YCbCrType_e4_2_2 == pMvdFieldData->eYCbCrType)?
            BVDC_P_ChromaType_eChroma422 :
            ((BAVC_InterpolationMode_eField == pMvdFieldData->eChrominanceInterpolationMode)?
                BVDC_P_ChromaType_eField420 : BVDC_P_ChromaType_eFrame420);

        if (hSource->hMpegFeeder->bGfxSrc)
        {
            BVDC_P_SurfaceInfo  *pCurSur;

            pCurSur = &hSource->hMpegFeeder->stGfxSurface.stCurSurInfo;
            pPicture->pSurface = pCurSur->stAvcPic.pSurface;
            pPicture->pSurface_R = pCurSur->stAvcPic.pRSurface;
            hSource->hMpegFeeder->stPicture.eSrcPolarity = pPicture->eSrcPolarity;
        }
    }
    else if((BVDC_P_SRC_IS_HDDVI(hSource->eId)) && (pXvdFieldData))
    {
        /* Build the picture */
        pPicture->stFlags.bMuteFixedColor        = pXvdFieldData->bMute;
        pPicture->eSrcPolarity                   = pXvdFieldData->eSourcePolarity;
        pPicture->PicComRulInfo.eSrcOrigPolarity = pXvdFieldData->eSourcePolarity;
        pPicture->PicComRulInfo.bNoCoreReset     = false;
        pPicture->eBitDepth                      = BAVC_VideoBitDepth_e8Bit;

        /* bOrientationOverride only valid when original
         * orientation from FMT is 2D */
        if((pSrcFmtInfo->eOrientation == BFMT_Orientation_e2D) &&
                (hSource->stCurInfo.bOrientationOverride))
        {
            pPicture->eSrcOrientation = hSource->stCurInfo.eOrientation;
        }
        else
        {
            pPicture->eSrcOrientation = pSrcFmtInfo->eOrientation;
        }

        /* hd-dvi source callback might identify the real field repeat from polarity
           repeat, but vdc alone cannot detect it;
           so we simply disable the field repeat detection for the moment. */
        pPicture->stFlags.bPictureRepeatFlag = false;

        /* TODO: we should get the correct source chroma type from the HDMI receiver
                 or picture data ready callback;
                 for the timing being, just set it to 'auto' to be safe; */
        pPicture->eChromaType          = BVDC_P_ChromaType_eAuto;
    }
    else if(BVDC_P_SRC_IS_ITU656(hSource->eId))
    {
        /* set the values for analog video picture */
        pPicture->stFlags.bMuteFixedColor = false;
        pPicture->eSrcPolarity            = eFieldId;
        pPicture->PicComRulInfo.eSrcOrigPolarity = eFieldId;
        pPicture->eSrcOrientation                = BFMT_Orientation_e2D;
        /* vdec source has no way to identify the real field repeat from polarity
           repeat, so we simply disable the field repeat detection. */
        pPicture->stFlags.bPictureRepeatFlag    = false;
        pPicture->eBitDepth                     = BAVC_VideoBitDepth_e8Bit;

        /* Determined eMatrixCoefficients for analog source. */
        eMatrixCoefficients  =
            BFMT_IS_27Mhz(hSource->stCurInfo.pFmtInfo->ulPxlFreqMask)
            ? BAVC_MatrixCoefficients_eSmpte_170M
            : BAVC_MatrixCoefficients_eItu_R_BT_709;

        /* we have no way to know the original source content's chroma type; */
        pPicture->eChromaType          = BVDC_P_ChromaType_eAuto;
    }
    else if (BVDC_P_SRC_IS_VFD(hSource->eId))
    {
        BVDC_P_SurfaceInfo  *pCurSur;

        pCurSur = &hSource->hVfdFeeder->stGfxSurface.stCurSurInfo;
        pPicture->ePixelFormat = pCurSur->eInputPxlFmt;
        pPicture->pSurface = pCurSur->stAvcPic.pSurface;
        pPicture->pSurface_R = pCurSur->stAvcPic.pRSurface;
        pPicture->eSrcOrientation = pCurSur->stAvcPic.eInOrientation;
        pPicture->ulOrigPTS = pCurSur->stAvcPic.ulOrigPTS;

        pPicture->stFlags.bMute = (0 == pCurSur->ullAddress);
        pPicture->stFlags.bMuteFixedColor = pPicture->stFlags.bMute;
        pPicture->eSrcPolarity = BAVC_Polarity_eFrame;
        pPicture->PicComRulInfo.eSrcOrigPolarity = pPicture->eSrcPolarity;

        eMatrixCoefficients = hSource->eMatrixCoefficients;
        pPicture->hBuffer = hWindow->hBuffer;
        /*hSource->bPictureChanged = false;*/
    }

    /* update input color space */
    BVDC_P_Window_UpdateVideoInputColorSpace_isr(hWindow, pMvdFieldData, pXvdFieldData,
        eMatrixCoefficients, &(pPicture->astMosaicColorSpace[ulPictureIdx]));

    /* Source classification based on input source and DNR */
    pPicture->bSrc10Bit =
        ((pPicture->eBitDepth != BAVC_VideoBitDepth_e8Bit &&
          hSource->bIs10BitCore &&
          !hSource->stCurInfo.bMosaicMode) ||
         (BVDC_P_VNET_USED_DNR(hWindow->stVnetMode) &&
          hWindow->stCurResource.hDnr->b10BitMode &&
          hSource->stCurInfo.stDnrSettings.eDcrMode == BVDC_FilterMode_eEnable));

    /* Set eDispOrientation and e3dSrcBufSel for 3d */
    pPicture->e3dSrcBufSel = hWindow->hCompositor->hDisplay->stCurInfo.e3dSrcBufSel;
    if(bDispFmtIs3d)
        pPicture->eDispOrientation = hWindow->hCompositor->stCurInfo.pFmtInfo->eOrientation;
    else
        pPicture->eDispOrientation = hWindow->hCompositor->stCurInfo.eOrientation;

    /* Capture can not convert 3d to 2d, it's either in 3d mode or in 2d
     * mode. In the case eSrcOrientation is 3d, eDispOrientation is 2d,
     * set capture in 2d mode, and increase CAP_0_BVB_IN_SIZE to clip to
     * single view.
     */
    if((pPicture->eDispOrientation == BFMT_Orientation_e2D) ||
       (pPicture->eSrcOrientation == BFMT_Orientation_e2D))
    {
        pPicture->eCapOrientation = BFMT_Orientation_e2D;
    }
    else
        pPicture->eCapOrientation = pPicture->eSrcOrientation;

#if (BVDC_P_DCX_3D_WORKAROUND)
    if((pPicture->eSrcOrientation != BFMT_Orientation_e2D) ||
       (pPicture->eDispOrientation != BFMT_Orientation_e2D) || bDispFmtIs3d)
    {
        pPicture->bEnableDcxm  = false;
    }
    else
#endif
    {
        pPicture->bEnableDcxm  = hWindow->bSupportDcxm;
    }
    if(hWindow->bIs10BitCore && pPicture->bEnableDcxm)
        pPicture->bEnable10Bit = true;
    else
        pPicture->bEnable10Bit = false;

    /* Source Frame rate */
    pPicture->eFrameRateCode = hSource->eFrameRateCode;

    /* (2) Determine the VNET mode and properly assign the rectangles according
     * to the modes.  If there are source_size changes or panscan vectors changes. */
    pPicture->stVnetMode = hWindow->stVnetMode;
    pPicture->stMvpMode  = hWindow->stMvpMode;
#if (BVDC_P_SUPPORT_MOSAIC_DEINTERLACE)
    if(hWindow->stCurResource.hMcvp)
    {
        bMvpBypass =
        ((!hWindow->stCurInfo.bMosaicMode) && (ulPictureIdx)) ? true : false;
        pPicture->stMvpMode.stBits.bUseMad =
            bMvpBypass?0:hWindow->stMvpMode.stBits.bUseMad;
        pPicture->stMvpMode.stBits.bUseAnr =
            bMvpBypass?0:hWindow->stMvpMode.stBits.bUseAnr;
        pPicture->stMvpMode.stBits.bUseMvpBypass =
            bMvpBypass?1:hWindow->stMvpMode.stBits.bUseMvpBypass;
        pPicture->stMvpMode.stBits.bUseHscl =
            bMvpBypass?0:hWindow->stMvpMode.stBits.bUseHscl;
    }
#endif

    /* (3) copy scale factor to pPicture */
    pPicture->ulNrmHrzSrcStep = hWindow->ulNrmHrzSrcStep;
    pPicture->ulNrmVrtSrcStep = hWindow->ulNrmVrtSrcStep;
    pPicture->ulNonlinearSrcWidth = hWindow->ulNonlinearSrcWidth;
    pPicture->ulNonlinearSclOutWidth = hWindow->ulNonlinearSclOutWidth;
    pPicture->ulCentralRegionSclOutWidth = hWindow->ulCentralRegionSclOutWidth;

    /* (4) Setup scaler, capture and the following video feeder. Further src clipping
     * after scan-out-clip is performed here.  Notice that user's rectangles are
     * updated by BVDC_P_Window_AdjustRectangles_isr().  For a majority of time if
     * there are no user's changes nor source's stream changes, the rectangles will
     * stay the same.
     *
     * Clip algorithm:
     *    Both scan (i.e. mpeg feeder), capture, video feeder, and compositor
     * could perform clip. However, only scaler can clip to sub-pixel level.
     * Mad will not perform any cut.
     *    Basing on the order defined by vnet mode, we do clip as early as
     * possible.
     *    In the case that the previous modules could not perform the needed clip
     * completely, the following modules clip the rest. For example, scan might
     * not be able to do clip completely becase another display share the mpeg
     * source, then the following module should complete the clipping. Another
     * example is that capture and feeder can only clip to pixel boundary, sub-
     * pixel clipping should be done by the following scaler.
     *    Clip in mpeg feeder is specified by pSrcOut, clip in scaler is specified
     * by stSclCut, clip in capture is specified by pCapOut, clip in feeder is
     * specified by pVfdOut, and clip in compositor is specified by pWinIn. pSrcOut,
     * pSclIn, pCapIn, pVfdIn only specify the input width and height. */
    pPicture->stDnrOut = hSource->stScanOut;
    pPicture->stSrcOut = hSource->stScanOut;
    lCntLeft = hWindow->stSrcCnt.lLeft - (hSource->stScanOut.lLeft<<BVDC_P_16TH_PIXEL_SHIFT);
    lCntLeft_R = hWindow->stSrcCnt.lLeft_R - (hSource->stScanOut.lLeft_R<<BVDC_P_16TH_PIXEL_SHIFT);
    lCntTop  = hWindow->stSrcCnt.lTop  - (hSource->stScanOut.lTop <<BVDC_P_16TH_PIXEL_SHIFT);
    ulCntWidth  = hWindow->stSrcCnt.ulWidth;
    ulCntHeight = hWindow->stSrcCnt.ulHeight;

    if(BVDC_P_VNET_USED_DNR(pPicture->stVnetMode))
    {
        pPicture->pDnrIn = &pPicture->stDnrOut;
        pPicture->pDnrOut= &pPicture->stDnrOut;
    }

#if BVDC_P_SUPPORT_XSRC /* XSRC is after DNR */
    if(BVDC_P_VNET_USED_XSRC(pPicture->stVnetMode))
    {
        pPicture->pXsrcIn = &pPicture->stDnrOut;
        pPicture->pXsrcOut= &pPicture->stSrcOut;

        /* if bigger than 4k30 input with xsrc, enable 2:1 hscl in xsrc */
        if(pPicture->ulPixelCount > BVDC_P_4K30_PIXEL_COUNT)
        {
            pPicture->pXsrcOut->lLeft   /= 2;
            pPicture->pXsrcOut->lLeft_R /= 2;
            pPicture->pXsrcOut->ulWidth /= 2;

            lCntLeft     /= 2;
            lCntLeft_R   /= 2;
            ulCntWidth   /= 2;
            pPicture->ulNrmHrzSrcStep   /= 2;
            pPicture->ulNonlinearSrcWidth /= 2;

            pPicture->ulXsrcNrmHrzSrcStep = (2<<BVDC_P_NRM_SRC_STEP_F_BITS);
        } else
        {
            pPicture->ulXsrcNrmHrzSrcStep = (1<<BVDC_P_NRM_SRC_STEP_F_BITS);
        }
    }
#endif

#if BVDC_P_SUPPORT_VFC
    if(BVDC_P_VNET_USED_VFC(pPicture->stVnetMode))
    {
        if(BVDC_P_VNET_USED_XSRC(pPicture->stVnetMode))
        {
            pPicture->pVfcIn = pPicture->pVfcOut = pPicture->pXsrcOut;
        }
        else
            pPicture->pVfcIn = pPicture->pVfcOut = &pPicture->stDnrOut;
    }
#endif

    pPicture->stSclOut = hWindow->stAdjSclOut;
    pPicture->stWinOut = hWindow->stAdjDstRect;

    /* sclOut width needs to be aligned to multiple of 2 if either SCL or
     * BVN is 4:2:2.
     * note a: srcCnt ulWidth and ulHeight are only used to decide feed/cap/
     * sclIn size, and they are always aligned due to HW limitation and aligned
     * up to avoid video info loose. The pixel + subpixel accuracy is passed to
     * SCL by lLeft/lTop for phase and ulNrmHrzSrcStep/ulNrmVrtSrcStep for src
     * step. SCL FIR will patch if it runs out of src pixels; And extra pixels
     * output from SCL will be clipped by CAP and/or CMP.
     * Note b: dstRect does not really needs align. However, currently we does
     * not honor odd dstRect height or top in interlaced display mode, nor
     * odd sclOut height or top. Indeed we are doing BVDC_P_DIV_ROUND_DN for
     * dstRect/sclOut top and height in interlaced display mode. If we want to
     * honor odd top and/or height, we need both SCL, SetSurfaceSize_isr and
     * SetDisplaySize_isr to coorperate and to align-up the heights here.
     * note c: srcCnt left/top can not be aligned here, its sub-pixel postion is
     * used in SCL. MFD ScanOut and sclCut vertical and horizontal intervals
     * will be aligned in Soure and Scaler respectively. srcCnt width/height
     * alignment is not needed here, and it might even force the final align of MFD
     * ScanOut, sclCut, CapOut to unnecessary bigger.
     * ---------------------------------------------------------------------- */
    pPicture->stSclOut.ulWidth = BVDC_P_ALIGN_UP(pPicture->stSclOut.ulWidth, 2);

    pPicture->pAnrIn  = NULL;
    pPicture->pMadIn  = NULL;
    pPicture->pSclIn  = NULL;
    pPicture->pCapIn  = NULL;
    pPicture->pVfdIn  = NULL;


    /* Connecting the rect based on vnet */

    if(BVDC_P_MVP_USED_HSCL(pPicture->stMvpMode))
    {
        /* init as pass through */
        pPicture->pHsclIn = pPicture->pSrcOut;
        pPicture->pHsclOut = pPicture->pSrcOut;
        pPicture->ulHsclNrmHrzSrcStep = (1 << BVDC_P_NRM_SRC_STEP_F_BITS); /* unit scale by default */
        pPicture->stHsclCut.lTop = 0;
        pPicture->stHsclCut.lLeft = 0; /* no src clip in hscl by default */
        pPicture->stHsclCut.lLeft_R = 0; /* no src clip in hscl by default */
        pPicture->stHsclCut.ulWidth = pPicture->pHsclIn->ulWidth;
        pPicture->stHsclCut.ulHeight = 0;
    }

    if(BVDC_P_MVP_USED_ANR(pPicture->stMvpMode))
    {
        pPicture->pAnrIn = pPicture->pSrcOut;
        pPicture->pAnrOut= pPicture->pSrcOut;
    }

    if(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode))
    {
        pPicture->pMadIn = pPicture->pSrcOut;
        pPicture->pMadOut= pPicture->pSrcOut;
    }

    pPicture->pWinIn = pPicture->pSclOut;
    if(BVDC_P_VNET_USED_CAPTURE(pPicture->stVnetMode))
    {
        if(BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode))
        {
            /* scaler is used before capture, scaler will do clip.
             * note: scaler horizontal/vertical subpixel pan-scan vectors
             * are in S11.6, S11.14 fixed point format respectively. */
            pPicture->stSclCut.lLeft = lCntLeft << 2;  /* 6: SCL_0_HORIZ_*_SRC_PIC_OFFSET */
            pPicture->stSclCut.lLeft_R = lCntLeft_R << 2;  /* 6: SCL_0_HORIZ_*_SRC_PIC_OFFSET */
            pPicture->stSclCut.lTop  = lCntTop  << 10; /* 14: SCL_0_VERT_*_SRC_PIC_OFFSET */
            pPicture->stSclCut.ulWidth  = ulCntWidth;
            pPicture->stSclCut.ulHeight = ulCntHeight; /* clip in scaler */

            pPicture->pSclIn = pPicture->pSrcOut;

            if(pPicture->bMosaicMode)
            {
                /* cut done in scl, no more in cap */
                pPicture->stCapOut.lLeft = 0;
                pPicture->stCapOut.lLeft_R = 0;
                pPicture->stCapOut.lTop  = 0;
                pPicture->stCapOut.ulWidth  = pPicture->stSclOut.ulWidth;
                pPicture->stCapOut.ulHeight = pPicture->stSclOut.ulHeight;
                pPicture->pCapIn = &pPicture->stCapOut;
                pPicture->pCapOut= pPicture->pCapIn;

                /* stVfdOut is not used in this case; so borrow it here; */
                pPicture->stVfdOut.lLeft = 0;
                pPicture->stVfdOut.lLeft_R = 0;
                pPicture->stVfdOut.lTop  = 0;
                pPicture->stVfdOut.ulWidth  = pUserInfo->stScalerOutput.ulWidth;
                pPicture->stVfdOut.ulHeight = pUserInfo->stScalerOutput.ulHeight;
#if (BVDC_P_MVFD_ALIGNMENT_WORKAROUND)
                if( BPXL_IS_YCbCr422_FORMAT(pPicture->ePixelFormat) )
                {
                    uint32_t   ulVfdWidth;

                    ulVfdWidth = (pPicture->stVfdOut.ulWidth + 1) & ~1;
                    if(ulVfdWidth % BVDC_P_SCB_BURST_SIZE == 2)
                    {
                        /* 422 formats: 4 bytes = 2 pixels  */
                        pPicture->stVfdOut.ulWidth  -= 2;
                    }
                }
#endif
                pPicture->pVfdIn = &pPicture->stVfdOut;
                pPicture->pVfdOut= pPicture->pVfdIn;
                pPicture->stWinIn= hWindow->stCurInfo.stScalerOutput;
                pPicture->pWinIn = &pPicture->stWinIn;
                pPicture->stWinIn.ulWidth = BVDC_P_ALIGN_UP(pPicture->stWinIn.ulWidth, 2);
                pPicture->stWinOut= hWindow->stCurInfo.stDstRect;
            }
            else
            {
                uint32_t  ulCapWidth, ulCapHeight;
                uint32_t  ulNewSclOutHeight, ulMoreHeightCut;
                const BFMT_VideoInfo *pDstFmtInfo;

                BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
                pDstFmtInfo = hWindow->hCompositor->stCurInfo.pFmtInfo;

                /* typical cut is already done in scl, but dstCut might have NOT
                 * been completely xfered to src cut, and scale factor rounding up
                 * might cause more sclOut edge, ok to set sclOut size a little big */
                pPicture->stCapOut.lTop = BVDC_P_ALIGN_DN(pPicture->stSclOut.lTop, 2);
                ulCapHeight = hWindow->stAdjDstRect.ulHeight +
                    pPicture->stSclOut.lTop - pPicture->stCapOut.lTop;
                pPicture->stCapOut.ulHeight =
                    BVDC_P_MIN(pDstFmtInfo->ulHeight, BVDC_P_ALIGN_UP(ulCapHeight, 2));

                pPicture->stCapOut.lLeft = BVDC_P_ALIGN_DN(pPicture->stSclOut.lLeft, 2);
                pPicture->stCapOut.lLeft_R = BVDC_P_ALIGN_DN(pPicture->stSclOut.lLeft_R, 2);
                ulCapWidth = hWindow->stAdjDstRect.ulWidth +
                    pPicture->stSclOut.lLeft - pPicture->stCapOut.lLeft;

#if (BVDC_P_MADR_HSIZE_WORKAROUND)
                if(hWindow->stCapCompression.bEnable || pUserInfo->bDeinterlace)
                {
                    pPicture->stCapOut.ulWidth = BVDC_P_ALIGN_UP(
                        BVDC_P_MIN(pDstFmtInfo->ulWidth, ulCapWidth), 4);
                }
                else
#endif
                {
                    pPicture->stCapOut.ulWidth =
                        BVDC_P_MIN(pDstFmtInfo->ulWidth, BVDC_P_ALIGN_UP(ulCapWidth, 2));
                }

                pPicture->stSclOut.ulWidth =
                    pPicture->stCapOut.ulWidth + pPicture->stCapOut.lLeft;
                ulNewSclOutHeight =
                    pPicture->stCapOut.ulHeight + pPicture->stCapOut.lTop;
#if (BVDC_P_WIN_MOSAIC_MODE_BVN_WORKAROUND)
                /* avoid cap intr fires before scl input completes*/
                if (pPicture->stSclOut.ulHeight > ulNewSclOutHeight)
                {
                    ulMoreHeightCut =
                        ((pPicture->stSclOut.ulHeight - ulNewSclOutHeight) *
                         hWindow->ulNrmVrtSrcStep) >> BVDC_P_NRM_SRC_STEP_F_BITS;
                    pPicture->stSclCut.ulHeight -= ulMoreHeightCut;
                }
#endif
                pPicture->stSclOut.ulHeight = ulNewSclOutHeight;

                pPicture->stVfdOut = pPicture->stCapOut;
                pPicture->stVfdOut.lLeft = 0; /* vfd code still honor crop */
                pPicture->stVfdOut.lLeft_R = 0; /* vfd code still honor crop */
                pPicture->stVfdOut.lTop  = 0;
#if (BVDC_P_MVFD_ALIGNMENT_WORKAROUND)
                if( BPXL_IS_YCbCr422_FORMAT(pPicture->ePixelFormat) )
                {
                    uint32_t   ulVfdWidth;

                    ulVfdWidth = (pPicture->stVfdOut.ulWidth + 1) & ~1;
                    if(ulVfdWidth % BVDC_P_SCB_BURST_SIZE == 2)
                    {
                        /* 422 formats: 4 bytes = 2 pixels  */
                        pPicture->stVfdOut.ulWidth  -= 2;
                    }
                }
#endif
                pPicture->stWinIn  = pPicture->stVfdOut;

                pPicture->stWinIn.lLeft = pPicture->stSclOut.lLeft - pPicture->stCapOut.lLeft;
                pPicture->stWinIn.lLeft_R = pPicture->stSclOut.lLeft_R - pPicture->stCapOut.lLeft_R;

                pPicture->stWinOut.ulWidth = BVDC_P_MIN(pPicture->stWinOut.ulWidth,
                    pPicture->stWinIn.ulWidth - pPicture->stWinIn.lLeft);
                pPicture->pCapIn = pPicture->pSclOut;
                pPicture->pCapOut= &pPicture->stCapOut;
                pPicture->pVfdIn = pPicture->pCapOut;
                pPicture->pVfdOut= &pPicture->stVfdOut;
                pPicture->pWinIn = &pPicture->stWinIn;
            }
        }
        else /* (scaler not used) || (scaler used in reader) */
        {
            int32_t  lCapVfdCutTop;

            BDBG_ASSERT(!pUserInfo->bMosaicMode);

            /* if capture before (or without) scaling, capturer clips to integer location,
             * and scaler further cut sub-pixel */
            if(BVDC_P_VNET_USED_SCALER_AT_READER(pPicture->stVnetMode))
            {
                uint32_t  ulAlignUnit;

                /* align left/top to floor and right/bottom to ceil to avoid losing video
                 * info. SCL after CAP/VFD will clip to up to sub-pixel position.
                 * note a: some scl, vfd/cap, anr and mad might still use 4:2:2 format even if
                 * the chip BVN does support 444 format, so we just align for 4:2:2 for simple.
                 * Note b: BVDC_P_16TH_PIXEL_SHIFT = 4 */
                BVDC_P_Window_AlignPreMadRect_isr(lCntLeft, lCntLeft_R,
                    ulCntWidth, pPicture->stSrcOut.ulWidth,
                    4, 2, &(pPicture->stCapOut.lLeft), &(pPicture->stCapOut.lLeft_R),
                    &(pPicture->stCapOut.ulWidth));
                ulAlignUnit = (BAVC_Polarity_eFrame == pPicture->eSrcPolarity)? 1 : 2;
                BVDC_P_Window_AlignPreMadRect_isr(lCntTop, lCntTop,
                    ulCntHeight, pPicture->stSrcOut.ulHeight,
                    4, ulAlignUnit, &(pPicture->stCapOut.lTop),
                    NULL, &(pPicture->stCapOut.ulHeight));

#if (BVDC_P_MADR_HSIZE_WORKAROUND)
                if(hWindow->stCapCompression.bEnable || pUserInfo->bDeinterlace ||
                   hWindow->stMadCompression.bEnable)
                {
                    uint32_t  lCapLeft, lCapLeft_R, ulCapWidth;
                    pPicture->pSrcOut->ulWidth = BVDC_P_ALIGN_UP(pPicture->pSrcOut->ulWidth, 4);
                    lCapLeft = BVDC_P_ALIGN_DN(pPicture->stCapOut.lLeft, 4);
                    lCapLeft_R = BVDC_P_ALIGN_DN(pPicture->stCapOut.lLeft_R, 4);
                    ulCapWidth = pPicture->stCapOut.ulWidth +
                        (pPicture->stCapOut.lLeft - lCapLeft);
                    pPicture->stCapOut.lLeft = lCapLeft;
                    pPicture->stCapOut.lLeft_R = lCapLeft_R;
                    pPicture->stCapOut.ulWidth = BVDC_P_ALIGN_UP(ulCapWidth, 4);
                }
#endif

#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
                if(pUserInfo->bDeinterlace && BVDC_P_VNET_USED_MVP(hWindow->stVnetMode))
                {
                    uint32_t  ulNewWidth;
                    BVDC_P_Window_PreMadAdjustWidth_isr(pPicture->stCapOut.ulWidth,
                        (BAVC_Polarity_eFrame == pPicture->PicComRulInfo.eSrcOrigPolarity)?
                        pPicture->stCapOut.ulHeight: pPicture->stCapOut.ulHeight / 2,
                        hWindow->stMadCompression.ulBitsPerGroup,
                        hWindow->stMadCompression.ulPixelPerGroup,
                        &ulNewWidth);

                    pPicture->pSrcOut->ulWidth += ulNewWidth - pPicture->stCapOut.ulWidth;
                    pPicture->stCapOut.ulWidth = ulNewWidth;
                }
#endif
            }
            else /* scl is not used */
            {
                pPicture->stCapOut.lLeft = lCntLeft >> BVDC_P_16TH_PIXEL_SHIFT;
                pPicture->stCapOut.lLeft_R = lCntLeft_R >> BVDC_P_16TH_PIXEL_SHIFT;
                pPicture->stCapOut.lTop  = lCntTop  >> BVDC_P_16TH_PIXEL_SHIFT;
                pPicture->stCapOut.ulWidth  = ulCntWidth;
                pPicture->stCapOut.ulHeight = ulCntHeight;
            }

            lCapVfdCutTop = pPicture->stCapOut.lTop;
            pPicture->stVfdOut = pPicture->stCapOut;
            pPicture->stVfdOut.lLeft = 0;
            pPicture->stVfdOut.lLeft_R = 0;
#if (BVDC_FORCE_VFD_TOP_CLIP)
            if(hWindow->stCapCompression.bEnable ||
               hWindow->stMadCompression.bEnable)
            {
                /* don't clip in vfd for compression */
                pPicture->stVfdOut.lTop = 0;
            }
            else
            {
                /* switch vrt cut from cap to vfd */
                pPicture->stCapOut.lTop = 0;
                pPicture->stCapOut.ulHeight = pPicture->pSrcOut->ulHeight;
            }
#else
            pPicture->stVfdOut.lTop  = 0;
#endif
            pPicture->pCapIn = pPicture->pSrcOut;
            pPicture->pCapOut= &pPicture->stCapOut; /* clip */
            pPicture->pVfdIn = &pPicture->stVfdOut;
            pPicture->pVfdOut= pPicture->pVfdIn;
            /* Feed out const color for HD source on bypass window */
            if( hWindow->bMuteBypass )
            {
                pPicture->pVfdOut= &pPicture->stWinOut;
            }

            if(BVDC_P_VNET_USED_SCALER_AT_READER(pPicture->stVnetMode))
            {
                /* scaler will do scaling and do sub-pixel clip */
                /* scaler horizontal/vertical subpixel pan-scan vectors
                 * are in S11.6, S11.14 fixed point format respectively. */
                pPicture->stSclCut.lLeft = (lCntLeft -
                    (pPicture->stCapOut.lLeft << BVDC_P_16TH_PIXEL_SHIFT)) << 2;
                pPicture->stSclCut.lLeft_R = (lCntLeft_R -
                    (pPicture->stCapOut.lLeft_R << BVDC_P_16TH_PIXEL_SHIFT)) << 2;
                pPicture->stSclCut.lTop  = (lCntTop  -
                    (lCapVfdCutTop  << BVDC_P_16TH_PIXEL_SHIFT)) << 10;
                pPicture->stSclCut.ulWidth  = ulCntWidth;
                pPicture->stSclCut.ulHeight = ulCntHeight;
#if (BVDC_P_MVFD_ALIGNMENT_WORKAROUND)
                if( BPXL_IS_YCbCr422_FORMAT(pPicture->ePixelFormat) )
                {
                    uint32_t   ulSclCutWidth;

                    ulSclCutWidth = (pPicture->stSclCut.ulWidth + 1) & ~1;
                    if(ulSclCutWidth % BVDC_P_SCB_BURST_SIZE == 2)
                    {
                        /* 422 formats: 4 bytes = 2 pixels  */
                        pPicture->stSclCut.ulWidth  -= 2;
                    }
                }
#endif

                pPicture->pSclIn = pPicture->pVfdOut;

                /* before this point pPicture->pVfdOut is not decided yet */
                if(BVDC_P_VNET_USED_MVP_AT_READER(pPicture->stVnetMode))
                {
                    if(BVDC_P_MVP_USED_HSCL(pPicture->stMvpMode))
                    {
                        pPicture->pHsclIn  = pPicture->pVfdOut;
                        pPicture->pHsclOut = pPicture->pVfdOut;
                        pPicture->stHsclCut.ulWidth = pPicture->pHsclIn->ulWidth;
                    }

                    if(BVDC_P_MVP_USED_ANR(pPicture->stMvpMode))
                    {
                        pPicture->pAnrIn = pPicture->pVfdOut;
                        pPicture->pAnrOut= pPicture->pVfdOut;
                    }

                    if(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode))
                    {

                        pPicture->pMadIn = pPicture->pVfdOut;
                        pPicture->pMadOut= pPicture->pVfdOut;
                    }
                }
            }
        }
    }
    else if(BVDC_P_VNET_USED_SCALER(pPicture->stVnetMode))
    {
        /* scaler is used before capture, scaler will do clip.
         * the following capture and the following feeder don't clip */
        /* Scaler horizontal/vertical subpixel pan-scan vectors
         * are in S11.6, S11.14 fixed point format respectively. */
        pPicture->stSclCut.lLeft = lCntLeft << 2;  /* 6: SCL_0_HORIZ_*_SRC_PIC_OFFSET */
        pPicture->stSclCut.lLeft_R = lCntLeft_R << 2;  /* 6: SCL_0_HORIZ_*_SRC_PIC_OFFSET */
        pPicture->stSclCut.lTop  = lCntTop  << 10; /* 14: SCL_0_VERT_*_SRC_PIC_OFFSET */
        pPicture->stSclCut.ulWidth  = ulCntWidth;
        pPicture->stSclCut.ulHeight = ulCntHeight;
        pPicture->pSclIn = pPicture->pSrcOut;
    }

    if (BVDC_P_SRC_IS_VFD(hSource->eId))
    {
        pPicture->pVfdIn = pPicture->pSrcOut;
        pPicture->pVfdOut = pPicture->pVfdIn;
    }

#if (BDBG_DEBUG_BUILD)
    if((BVDC_P_VNET_USED_SCALER(pPicture->stVnetMode)) &&
       ((pPicture->stSclCut.lLeft < 0) || (pPicture->stSclCut.lTop < 0)))
    {
        BDBG_MSG(("neg sclCut left %d or top %d",
            pPicture->stSclCut.lLeft, pPicture->stSclCut.lTop));
    }
#endif

    if(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode))
    {
        bool bMadIs3dLR = false;
        uint32_t ulHsclSrcHrzSclThr, ulMaxMadWidth;

        BDBG_ASSERT(hWindow->stCurResource.hMcvp);
        BVDC_P_Window_GetDeinterlacerMaxResolution_isr(hWindow, NULL,
            &ulMaxMadWidth, NULL, &ulHsclSrcHrzSclThr, false);

#ifdef HSCL_TUNE_THRESHOLD
        ulHsclSrcHrzSclThr = BREG_Read32(hSource->hVdc->hRegister, BCHP_HSCL_0_SCRATCH_0);
#endif

        /* ulMaxMadWidth is 960 when MCVP is in 3D */
        /* BVDC_P_VNET_USED_MCVP_AT_WRITER = BVDC_P_VNET_USED_MAD_AT_WRITER,
         * BVDC_P_VNET_USED_MCVP_AT_READER = BVDC_P_VNET_USED_MAD_AT_READER
         */
        if((BVDC_P_VNET_USED_MVP_AT_WRITER(pPicture->stVnetMode) &&
            (pPicture->eSrcOrientation != BFMT_Orientation_e2D)) ||
            (BVDC_P_VNET_USED_MVP_AT_READER(pPicture->stVnetMode) &&
            (pPicture->eDispOrientation != BFMT_Orientation_e2D)))
        {
            bMadIs3dLR = true;
        }

        if(bMadIs3dLR)
            ulMaxMadWidth = ulMaxMadWidth / 2;
        ulHsclSrcHrzSclThr = BVDC_P_MIN(ulHsclSrcHrzSclThr, ulMaxMadWidth);
        if (ulHsclSrcHrzSclThr < pPicture->pMadIn->ulWidth)
        {
            BVDC_P_Window_TryXferHrzSclToHscl_isr(hWindow, pPicture, ulHsclSrcHrzSclThr);
        }
    }

#if (BVDC_P_WIN_MOSAIC_MODE_BVN_WORKAROUND)
    if(hSource->stCurInfo.bMosaicMode)
    {
        if(pPicture->pCapIn)
        {
            pPicture->pCapIn->ulWidth = BVDC_P_MAX(pPicture->pCapIn->ulWidth,
                BVDC_P_WIN_MOSAIC_OUTPUT_H_MIN);
            pPicture->pCapIn->ulHeight = BVDC_P_MAX(pPicture->pCapIn->ulHeight,
                BVDC_P_WIN_MOSAIC_OUTPUT_V_MIN);
        }

        if(pPicture->pSclOut)
        {
            pPicture->pSclOut->ulWidth = BVDC_P_MAX(pPicture->pSclOut->ulWidth,
                BVDC_P_WIN_MOSAIC_OUTPUT_H_MIN);
            pPicture->pSclOut->ulHeight = BVDC_P_MAX(pPicture->pSclOut->ulHeight,
                BVDC_P_WIN_MOSAIC_OUTPUT_V_MIN);
        }
    }
#endif

    /* If rectangle computation results in limit violation, override the */
    /* limit-violating rectangle sizes with HW limits */
    if(hWindow->bSupportDcxm && pPicture->bMosaicMode)
    {
        ulMinCapHsize = BVDC_P_WIN_CAP_MOSAIC_INPUT_H_MIN;
        ulMinCapVsize = BVDC_P_WIN_CAP_MOSAIC_INPUT_V_MIN;
        ulMinVfdHsize = BVDC_P_WIN_VFD_MOSAIC_OUTPUT_H_MIN;
        ulMinVfdVsize = BVDC_P_WIN_VFD_MOSAIC_OUTPUT_V_MIN;
    }
    else
    {
        ulMinCapHsize = BVDC_P_WIN_CAP_INPUT_H_MIN;
        ulMinCapVsize = BVDC_P_WIN_CAP_INPUT_V_MIN;
        ulMinVfdHsize = BVDC_P_WIN_VFD_OUTPUT_H_MIN;
        ulMinVfdVsize = BVDC_P_WIN_VFD_OUTPUT_V_MIN;
    }


    {
        uint32_t                        ulWriterHLimit,ulWriterVLimit, ulReaderHLimit, ulReaderVLimit, ulCapHSize, ulCapVSize, ulVfdHSize, ulVfdVSize;
        bool bSrcInterlaced = hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->bInterlaced;
        bool bDstInterlaced = hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced;

        /* pPicture->stSclCut*/
        ulWriterHLimit = BVDC_P_VNET_USED_DNR(pPicture->stVnetMode)?BVDC_P_WIN_DNR_INPUT_H_MIN:0;
        ulWriterHLimit = BVDC_P_MAX(ulWriterHLimit, BVDC_P_MVP_USED_MAD(pPicture->stMvpMode)?BVDC_P_WIN_MAD_INPUT_H_MIN:0);
        ulWriterHLimit = BVDC_P_MAX(ulWriterHLimit, BVDC_P_MVP_USED_ANR(pPicture->stMvpMode)?BVDC_P_WIN_ANR_INPUT_H_MIN:0);
        ulWriterHLimit = BVDC_P_MAX(ulWriterHLimit, BVDC_P_MVP_USED_HSCL(pPicture->stMvpMode)?BVDC_P_WIN_SCL_OUTPUT_H_MIN:0);

        /* vertical */
        ulWriterVLimit = BVDC_P_VNET_USED_DNR(pPicture->stVnetMode)?BVDC_P_WIN_DNR_INPUT_V_MIN:0;
        ulWriterVLimit = BVDC_P_MAX(ulWriterVLimit, BVDC_P_MVP_USED_MAD(pPicture->stMvpMode)?BVDC_P_WIN_MAD_INPUT_V_MIN:0);
        ulWriterVLimit = BVDC_P_MAX(ulWriterVLimit, BVDC_P_MVP_USED_ANR(pPicture->stMvpMode)?BVDC_P_WIN_ANR_INPUT_V_MIN:0);
        ulWriterVLimit = BVDC_P_MAX(ulWriterVLimit, BVDC_P_MVP_USED_HSCL(pPicture->stMvpMode)?BVDC_P_WIN_SCL_OUTPUT_V_MIN:0);


        ulReaderHLimit = pPicture->bMosaicMode?BVDC_P_WIN_MOSAIC_OUTPUT_H_MIN : BVDC_P_WIN_DST_OUTPUT_H_MIN;
        ulReaderHLimit = BVDC_P_MAX(ulReaderHLimit, BVDC_P_WIN_SCL_OUTPUT_H_MIN);
        ulReaderVLimit = pPicture->bMosaicMode?BVDC_P_WIN_MOSAIC_OUTPUT_V_MIN : BVDC_P_WIN_DST_OUTPUT_V_MIN;
        ulReaderVLimit = BVDC_P_MAX(ulReaderVLimit, BVDC_P_WIN_SCL_OUTPUT_V_MIN);



        if(BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode))
        {
            ulReaderHLimit = BVDC_P_MAX(ulReaderHLimit, ulMinCapHsize);
            ulReaderHLimit = BVDC_P_MAX(ulReaderHLimit, ulMinVfdHsize);

            ulReaderVLimit = BVDC_P_MAX(ulReaderVLimit, ulMinCapVsize);
            ulReaderVLimit = BVDC_P_MAX(ulReaderVLimit, ulMinVfdVsize);
        }
        else
        {
            ulWriterHLimit = BVDC_P_MAX(ulWriterHLimit, ulMinCapHsize);
            ulWriterHLimit = BVDC_P_MAX(ulWriterHLimit, ulMinVfdHsize);

            ulWriterVLimit = BVDC_P_MAX(ulWriterVLimit, ulMinCapVsize);
            ulWriterVLimit = BVDC_P_MAX(ulWriterVLimit, ulMinVfdVsize);
        }

        ulWriterVLimit <<= bSrcInterlaced;
        ulReaderVLimit <<= bDstInterlaced;

        ulVfdHSize = ulCapHSize = BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode)?ulReaderHLimit:ulWriterHLimit;
        ulVfdVSize = ulCapVSize = BVDC_P_VNET_USED_SCALER_AT_WRITER(pPicture->stVnetMode)?ulReaderVLimit:ulWriterVLimit;

        /* possible src/dstlimit */
        BVDC_P_Window_EnforceMinSizeLimit_isr(hWindow, pPicture->pCapIn,   BDBG_STRING("CAPIn"), ulCapHSize, ulCapVSize, 0);
        BVDC_P_Window_EnforceMinSizeLimit_isr(hWindow, pPicture->pVfdOut,  BDBG_STRING("VFDOut"), ulVfdHSize, ulVfdVSize, 0);

        /* Destination limit */
        BVDC_P_Window_EnforceMinSizeLimit_isr(hWindow, pPicture->pWinIn,   BDBG_STRING("WinIn"), ulReaderHLimit, ulReaderVLimit, 0);
        BVDC_P_Window_EnforceMinSizeLimit_isr(hWindow, pPicture->pSclOut,  BDBG_STRING("SCL"), ulReaderHLimit, ulReaderVLimit, 0);


        /* source limit */
        BVDC_P_Window_EnforceMinSizeLimit_isr(hWindow, &pPicture->stSclCut,  BDBG_STRING("SCLCUT"), ulWriterHLimit, ulWriterVLimit, 0);
        BVDC_P_Window_EnforceMinSizeLimit_isr(hWindow, pPicture->pDnrIn,   BDBG_STRING("DNR"), ulWriterHLimit,ulWriterVLimit, 0);

        if(BVDC_P_MVP_USED_HSCL(pPicture->stMvpMode))
            BVDC_P_Window_EnforceMinSizeLimit_isr(hWindow, pPicture->pHsclOut, BDBG_STRING("HCL"), ulWriterHLimit, ulWriterVLimit, 0);

        if(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode))
            BVDC_P_Window_EnforceMinSizeLimit_isr(hWindow, pPicture->pMadIn,   BDBG_STRING("MAD"), ulWriterHLimit, ulWriterVLimit, 0);

        if(BVDC_P_MVP_USED_ANR(pPicture->stMvpMode))
            BVDC_P_Window_EnforceMinSizeLimit_isr(hWindow, pPicture->pAnrIn,   BDBG_STRING("ANR"), ulWriterHLimit, ulWriterVLimit, 0);
    }

    hWindow->ulNrmHrzSrcStep = BVDC_P_CAL_HRZ_SRC_STEP(
            ulCntWidth, pPicture->stSclOut.ulWidth);

    hWindow->ulNrmVrtSrcStep = BVDC_P_CAL_VRT_SRC_STEP(
        hWindow->stSrcCnt.ulHeight, pPicture->stSclOut.ulHeight, hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced);

    if(hWindow->stCurInfo.stDirty.stBits.bRecAdjust)
    {
        BVDC_P_Window_DumpRects_isr(hWindow, pPicture);

        /*Update the delay VSYNC for seamless transcode*/
        hWindow->stCurInfo.stDirty.stBits.bRecAdjust = BVDC_P_CLEAN;

        /* Set up dirty bit for callback */
        if(((hWindow->stCbData.stOutputRect.lLeft    != pPicture->pWinOut->lLeft) ||
            (hWindow->stCbData.stOutputRect.lTop     != pPicture->pWinOut->lTop) ||
            (hWindow->stCbData.stOutputRect.ulWidth  != pPicture->pWinOut->ulWidth) ||
            (hWindow->stCbData.stOutputRect.ulHeight != pPicture->pWinOut->ulHeight)) &&
           (pUserInfo->stCbSettings.stMask.bRectAdjust))
        {
            hWindow->stCbData.stOutputRect.lLeft    = pPicture->pWinOut->lLeft;
            hWindow->stCbData.stOutputRect.lTop     = pPicture->pWinOut->lTop;
            hWindow->stCbData.stOutputRect.ulWidth  = pPicture->pWinOut->ulWidth;
            hWindow->stCbData.stOutputRect.ulHeight = pPicture->pWinOut->ulHeight;
            hWindow->stCbData.stMask.bRectAdjust = BVDC_P_DIRTY;
        }
    }

    if((BVDC_P_SRC_IS_MPEG(hSource->eId)) && (pMvdFieldData) &&
       (hWindow->hCompositor->hDisplay->stCurInfo.eBarDataMode == BVDC_Mode_eAuto)) {
        BVDC_P_Window_UpdateBarData_isr(hWindow, pPicture, pMvdFieldData);
    }

#if (BVDC_P_CAP_DCXM_SCB_WORKAROUND)
    if(pPicture->bEnableDcxm && pPicture->pCapIn)
    {
        uint32_t   ulCompBits, ulBurstSize;

        ulBurstSize = 256 * 24;
        /* Always use 10bit for compression */
        ulCompBits = (pPicture->pCapIn->ulWidth * pPicture->pCapIn->ulHeight)*10;

        if(ulCompBits < ulBurstSize)
        {
            pPicture->bEnableDcxm  = false;
            pPicture->bEnable10Bit = false;
        }
    }
#endif

    /* Update compression */
    pPicture->stCapCompression.bEnable = hWindow->stCapCompression.bEnable;
    pPicture->stCapCompression.ulBitsPerGroup = hWindow->stCapCompression.ulBitsPerGroup;
    pPicture->stCapCompression.ulPixelPerGroup = hWindow->stCapCompression.ulPixelPerGroup;

    BDBG_LEAVE(BVDC_P_Window_UpdateSrcAndUserInfo_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 * Partial decision on the vnet mode and cap buf count, according to user's
 * setting only.  Further and complete decision are done later with
 * combination of both user settings and dynamic src info.
 */
void BVDC_P_Window_DecideCapture_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_Source_Handle               hSource,
      const BVDC_Compositor_Handle     hCompositor )
{
    uint32_t ulBufCntNeeded, ulBufDelay;
    const BFMT_VideoInfo *pSrcFmtInfo;
    const BVDC_P_Window_Info *pWinInfo;
    const BVDC_P_Compositor_Info *pCmpInfo;
    bool bEnableCaptureByMosaicOrPsf = false, bProgressivePullDown=false;
    bool bCapture, bEnableCaptureByDelay;
    BVDC_P_WrRateCode eWriterVsReaderRateCode, eReaderVsWriterRateCode;

    BDBG_ENTER(BVDC_P_Window_DecideCapture_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* Not intended for gfx windows. */
    BDBG_ASSERT(BVDC_P_SRC_IS_VIDEO(hSource->eId));

    ulBufCntNeeded = BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT;

    /* Save some typing */
    pWinInfo = &hWindow->stNewInfo;
    pCmpInfo = &hCompositor->stNewInfo;
    pSrcFmtInfo = ((hSource->stNewInfo.bAutoFmtDetection) &&
                   (hSource->stCurInfo.bAutoFmtDetection))?
        hSource->stCurInfo.pFmtInfo : hSource->stNewInfo.pFmtInfo;

    /* Full screen is consider as display destination scale to greater
     * or equal to 75% of full screen.  In addition it must not be ARC
     * that uses all source, because we'll downscale possibly below 75%, and
     * that would required capture.
     *
     * Why >= width-3?  This is to allow app to pass in 480 and interpret it
     * as full screen.  Basically we want to have the lagacy 480 and 482 lines
     * for 480i, 483 for 480p to be similar. */
    if(BBOX_Vdc_SclCapBias_eAutoDisable1080p == hWindow->eBoxSclCapBias)
    {
        hWindow->eScanoutMode = BVDC_P_GetScanoutMode4AutoDisable1080p_isr(&pWinInfo->stScalerOutput);
    }
    else
    {
        if(((pWinInfo->stScalerOutput.ulWidth  == pCmpInfo->pFmtInfo->ulWidth) &&
            (pWinInfo->stScalerOutput.ulHeight >= pCmpInfo->pFmtInfo->ulDigitalHeight)) ||
           ((pWinInfo->stScalerOutput.ulWidth  >= pCmpInfo->pFmtInfo->ulWidth * 3 / 4) &&
            (pWinInfo->stScalerOutput.ulHeight >= pCmpInfo->pFmtInfo->ulDigitalHeight * 3 / 4)&&
            (BVDC_AspectRatioMode_eUseAllSource != pWinInfo->eAspectRatioMode)))
        {
            hWindow->eScanoutMode = BVDC_P_ScanoutMode_eLive;
        }
        else
        {
            hWindow->eScanoutMode = BVDC_P_ScanoutMode_eCapture;
        }
    }

    if(BVDC_P_SRC_IS_MPEG(hSource->eId))
    {
        /* Defer actual determniation until mpeg_data_isr (defer).  Or last state. */

        /* bCapture do we need to capture?
         * TODO: this logic will be moved to _isr to support _SetXyzRect_isr; */
        bCapture = (
#if (BVDC_P_AUTO_ENABLE_CAPTURE)
            (hWindow->eScanoutMode == BVDC_P_ScanoutMode_eCapture) ||
#endif
            !hWindow->bSyncLockSrc          ||
             pWinInfo->bForceCapture        ||
             pWinInfo->bBoxDetect           ||
             hWindow->uiAvailCaptureBuffers ||
            /* In the case of HD-SD simul or single source multiple windows!
             * We'll turn on capture for these addtional cases to relieve bandwidth:
             *   (1) Vertical Cropping: AllDestinataion, PanScan, SrcClip
             *   (2) Locked on non cmp0. */
            ((hSource->stNewInfo.ulWindows > 1) &&
             ((BVDC_AspectRatioMode_eUseAllDestination == pWinInfo->eAspectRatioMode) ||
              (BVDC_PanScanType_eDisable != pWinInfo->ePanScanType) ||
              (pWinInfo->stSrcClip.ulTop + pWinInfo->stSrcClip.ulBottom))));

        /* NRT mode shouldn't capture otherwise EOP trigger might come faster than capture side can sustain;
           upper layer should not add vsync delay buffer either for NRT window; plus NRT window must be sync-locked; */
#if (BVDC_P_SUPPORT_STG)
        bCapture = bCapture && !hCompositor->hDisplay->stNewInfo.bStgNonRealTime;
#endif

        /* Calculate ulBufDelay */
        if( bCapture )
        {
            ulBufDelay = (hWindow->bSyncLockSrc || hWindow->stSettings.bForceSyncLock)?
                BVDC_P_FIELD_VSYNC_DELAY : BVDC_P_FRAME_VSYNC_DELAY;
        }
        else
        {
            ulBufDelay = BVDC_P_NO_CAPTURE_VSYNC_DELAY;
        }
    }
    else if (BVDC_P_SRC_IS_VFD(hSource->eId)) /* capture is not possible with VFD as source */
    {
        bCapture = false;
        ulBufDelay = BVDC_P_NO_CAPTURE_VSYNC_DELAY;
    }
    else
    {
        /* Non mpeg source always require capture. */
        bCapture = true;
        /* Need to or with bForceCapture if not always capture */
        /* bCapture |= pWinInfo->bForceCapture; */
        ulBufDelay = BVDC_P_FRAME_VSYNC_DELAY;
    }

    /* Always capture if uiVsyncDelayOffset is not 0 */
    bEnableCaptureByDelay = false;
    if( pWinInfo->uiVsyncDelayOffset && !bCapture && !BVDC_P_SRC_IS_VFD(hSource->eId))
    {
        bEnableCaptureByDelay = true;
        bCapture = true;
    }

    /* Always capture if source mosaic mode is on; */
    if( (hSource->stNewInfo.bMosaicMode || hSource->stNewInfo.bPsfEnable || hSource->stNewInfo.bForceFrameCapture) &&
        !bCapture && !BVDC_P_SRC_IS_VFD(hSource->eId) )
    {
        bEnableCaptureByMosaicOrPsf = true;
        bCapture = true;
    }

    /* Always capture if source orientation and display orientation not match */
    if((BFMT_IS_3D_MODE(pCmpInfo->pFmtInfo->eVideoFmt) &&
        pSrcFmtInfo->eOrientation != pCmpInfo->pFmtInfo->eOrientation) ||
       (pSrcFmtInfo->eOrientation != pCmpInfo->eOrientation))
    {
        bCapture = true;
    }

    /* user force capture frame */
    if (hWindow->stCurInfo.hSource->stCurInfo.bForceFrameCapture)
    {
        bCapture = true;
    }

    /* hWindow->bCapture is completely decided by user info, it will be used to
     * decide vnetMode */
    if(bCapture != hWindow->bCapture)
    {
        BVDC_P_Window_SetReconfiguring_isr(hWindow, false, true, false);
        hWindow->bCapture = bCapture;
    }

    /* How many buffers do we need for given mode. */
    if(bCapture)
    {
        ulBufCntNeeded = (hWindow->bSyncLockSrc || hWindow->stSettings.bForceSyncLock)?
            BVDC_P_SYNC_LOCK_MULTI_BUFFER_COUNT : BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT;
    }
    else
    {
        /* if (!BVDC_P_SRC_IS_VFD(hWindow->stNewInfo.hSource->eId))
           BDBG_ASSERT(hWindow->bSyncLockSrc); */
        ulBufCntNeeded = BVDC_P_BYPASS_MULTI_BUFFER_COUNT;
    }

    hWindow->bBufCntNeededChanged = (ulBufCntNeeded != hWindow->ulPrevBufCntNeeded);
    hWindow->ulPrevBufCntNeeded = ulBufCntNeeded;

    /* Update ulBufDelay & ulBufCntNeeded based on delay offset */
    if( bEnableCaptureByDelay )
    {
        /* Don't need to capture without delay */
        ulBufDelay     = pWinInfo->uiVsyncDelayOffset ;
        ulBufCntNeeded = ulBufDelay + 1;
    }
    else if( bEnableCaptureByMosaicOrPsf )
    {
        /* Don't need to capture without mosaic */
        ulBufDelay     = BVDC_P_FIELD_VSYNC_DELAY ;
        ulBufCntNeeded = ulBufDelay + 1;
    }
    else
    {
        /* Need to capture without delay */
        ulBufDelay     += pWinInfo->uiVsyncDelayOffset ;
        ulBufCntNeeded += pWinInfo->uiVsyncDelayOffset ;
    }

    hWindow->ulBufCntNeeded = ulBufCntNeeded;
    hWindow->ulBufDelay     = ulBufDelay;
    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] need %d buffers, cap: %d, synclock: %d",
        hWindow->eId, hWindow->ulBufCntNeeded, bCapture,
        hWindow->bSyncLockSrc || hWindow->stSettings.bForceSyncLock));

    /* Determine rate gap */
    BVDC_P_Buffer_CalculateRateGap_isr(hWindow->stNewInfo.hSource->ulVertFreq,
        hCompositor->stNewInfo.pFmtInfo->ulVertFreq,
        &eWriterVsReaderRateCode, &eReaderVsWriterRateCode);

    /* Reset the buffer count decremented/incremented flags */
    hWindow->bBufferCntDecremented = false;
    hWindow->bBufferCntDecrementedForPullDown = false;

    /* If the buffer count needs to be incremented,
     * it will be set when the interlace with rate gap condition is detected. */
    hWindow->bBufferCntIncremented = false;

    /* Decrement buffer count if new video format is progressive and reader and writer rate gaps
     * are the same or if we are doing progressive 24/25/30 Hz to 48/50/60 Hz conversion.
     * Set the bBufferCntDecremented flag.
     */
    bProgressivePullDown = VIDEO_FORMAT_IS_PROGRESSIVE(hSource->stNewInfo.pFmtInfo->eVideoFmt) &&
                           (eWriterVsReaderRateCode == BVDC_P_WrRate_NotFaster) &&
                           (eReaderVsWriterRateCode >= BVDC_P_WrRate_2TimesFaster);

    if((!hWindow->bSyncLockSrc) && (!hWindow->stSettings.bForceSyncLock) && !BVDC_P_SRC_IS_VFD(hWindow->stNewInfo.hSource->eId) &&
        ((VIDEO_FORMAT_IS_PROGRESSIVE(hCompositor->stNewInfo.pFmtInfo->eVideoFmt)
        && (eWriterVsReaderRateCode == eReaderVsWriterRateCode)) || bProgressivePullDown) && bCapture)
    {
        /* From N buffers to N-1 buffers */
        hWindow->ulBufCntNeeded--;
        hWindow->ulBufDelay--;

        if(eWriterVsReaderRateCode != eReaderVsWriterRateCode)
            /* Progressive pull down case */
            hWindow->bBufferCntDecrementedForPullDown = true;

        hWindow->bBufferCntDecremented = true;
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Decrementing buffer count to %d for progressive display format",
            hWindow->eId, hWindow->ulBufCntNeeded));
    }

    BDBG_LEAVE(BVDC_P_Window_DecideCapture_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 * This function calcualtes DCX heap size in bytes.
 */
static uint32_t BVDC_P_Window_GetDCXBufSize_isr
    ( const BVDC_P_Rect                      *pSrcRect,
      const bool                              bInterlaced,
      const bool                              bMinSrc,
      const uint32_t                          ulBitsPerGroup,
      const uint32_t                          ulPixelPerGroup)
{
    uint32_t  ulBufSize = 0; /* in bytes */

    /* DCX buffer:
     * ulBufSize = bits_per_pixel * pixels_per_picture + 16*1024 + 64 */
    ulBufSize = (((pSrcRect->ulWidth * pSrcRect->ulHeight) >> bInterlaced)
        * ulBitsPerGroup) ;
    ulBufSize = BVDC_P_DIV_ROUND_UP(ulBufSize, ulPixelPerGroup);

    /* double the margin for min source progressive format calculation */
    ulBufSize += (16*1024 + 64) << (bMinSrc && (!bInterlaced));

    /* round up to 256-bit chunks */
    ulBufSize = BVDC_P_ALIGN_UP(ulBufSize, BVDC_P_HEAP_ALIGN_BYTES);

    /* In bytes */
    ulBufSize /= 8;

    /* Make sure 32 byte aligned */
    ulBufSize = BVDC_P_ALIGN_UP(ulBufSize, BVDC_P_HEAP_ALIGN_BYTES);

    return ulBufSize;
}

/***************************************************************************
 * {private}
 *
 * This function calcualtes heap size in bytes.
 * bMosaicMode & b3DMode are only used for capture buffers.
 *
 */
void BVDC_P_Window_GetBufSize_isr
    ( BVDC_P_WindowId                         eWinId,
      const BVDC_P_Rect                      *pSrcRect,
      const bool                              bInterlaced,
      const bool                              bMosaicMode,
      const bool                              bFillBars,
      const bool                              b3DMode,
      const bool                              bMinSrc,
      const BPXL_Format                       eBufPxlFmt,
      const BVDC_P_Compression_Settings      *pCompression,
      BVDC_P_BufHeapType                      eBufHeapType,
      uint32_t                               *pulBufSize,
      BAVC_VideoBitDepth                      eBitDepth)
{
    uint32_t             ulBufSize; /* in bytes */
    unsigned int         uiPitch;
    BVDC_P_Rect          stSrcRect;
#if (BVDC_P_BUFFER_ADD_GUARD_MEMORY)
    uint32_t  ulStride;
#endif

#if (!BVDC_P_BUFFER_ADD_GUARD_MEMORY)
    BSTD_UNUSED(bMosaicMode);
    BSTD_UNUSED(bFillBars);
    BSTD_UNUSED(b3DMode);
#endif

#if (BVDC_P_BUFFER_ADD_GUARD_MEMORY)
    ulStride = pSrcRect->ulWidth * 2;
#endif

    /* Get buffer size */
    if(eBufHeapType == BVDC_P_BufHeapType_eMad_QM)
    {
        /* QM buffer is always 2 bits per pixel */
        ulBufSize = (((pSrcRect->ulWidth * pSrcRect->ulHeight) >> bInterlaced)
            * BVDC_P_MAD_QM_BITS_PER_PIXEL ) / 8;
        ulBufSize = BVDC_P_ALIGN_UP(ulBufSize, BVDC_P_PITCH_ALIGN);
        ulBufSize *= BVDC_P_MAD_QM_FIELD_STORE_COUNT;
    }
    else if((eBufHeapType  == BVDC_P_BufHeapType_eMad_Pixel) ||
        (eBufHeapType  == BVDC_P_BufHeapType_eAnr))
    {
        if(pCompression && pCompression->bEnable)
        {
            uint32_t  ulBitsPerGroup  = pCompression->ulBitsPerGroup;
            uint32_t  ulPixelPerGroup = pCompression->ulPixelPerGroup;

            if(eBufHeapType  == BVDC_P_BufHeapType_eMad_Pixel)
            {
                if(eBitDepth == BAVC_VideoBitDepth_e8Bit)
                    ulBitsPerGroup  = BVDC_P_MADR_DCXS_COMPRESSION(ulBitsPerGroup);
                else
                {
                    if(ulBitsPerGroup >= BVDC_53BITS_PER_GROUP)
                        ulBitsPerGroup = BVDC_53BITS_PER_GROUP;
                    else if(ulBitsPerGroup >= BVDC_45BITS_PER_GROUP)
                        ulBitsPerGroup = BVDC_45BITS_PER_GROUP;
                    else
                        ulBitsPerGroup = BVDC_37BITS_PER_GROUP;
                }
            }
            stSrcRect = *pSrcRect;
            ulBufSize = BVDC_P_Window_GetDCXBufSize_isr(&stSrcRect,
                bInterlaced, false, ulBitsPerGroup, ulPixelPerGroup);
        }
        else
        {
            BPXL_GetBytesPerNPixels_isr(eBufPxlFmt, pSrcRect->ulWidth, &uiPitch);
            ulBufSize = (uiPitch * pSrcRect->ulHeight) >> bInterlaced;
        }
    }
    else if(eBufHeapType  == BVDC_P_BufHeapType_eCapture)
    {
        if(pCompression && pCompression->bEnable)
        {
            uint32_t  ulBitsPerGroup  = pCompression->ulBitsPerGroup;
            uint32_t  ulPixelPerGroup = pCompression->ulPixelPerGroup;

            stSrcRect = *pSrcRect;
#if BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM
            if(eBitDepth != BAVC_VideoBitDepth_e8Bit)
            {
                if(pCompression->ulBitsPerGroup >= BVDC_48BITS_PER_GROUP)
                    ulBitsPerGroup = BVDC_48BITS_PER_GROUP;
                else
                    ulBitsPerGroup = BVDC_40BITS_PER_GROUP;

                /* SW7445-2936: additional line in the capture buffer size calculation*/
                stSrcRect.ulHeight += BVDC_P_DCXM_CAP_PADDING_WORKAROUND<<bInterlaced;
            }
#endif

#if (BVDC_P_BUFFER_ADD_GUARD_MEMORY)
            if(bMosaicMode)
            {
                ulStride = (stSrcRect.ulWidth * BVDC_P_DCXM_BITS_PER_PIXEL) / 8;
                ulStride += b3DMode ? BVDC_P_MAX_CAP_GUARD_MEMORY_3D
                    : BVDC_P_MAX_CAP_GUARD_MEMORY_2D;
                ulBufSize = (ulStride * pSrcRect->ulHeight) >> bInterlaced;
            }
            else if(bFillBars)
            {
                ulStride = (stSrcRect.ulWidth * BVDC_P_DCXM_BITS_PER_PIXEL) / 8;
                ulStride += b3DMode ? BVDC_P_MAX_CAP_GUARD_FILLBAR_3D
                    : BVDC_P_MAX_CAP_GUARD_FILLBAR_2D;
                ulBufSize = (ulStride * pSrcRect->ulHeight) >> bInterlaced;
            }
            else
#endif
            {
                ulBufSize = BVDC_P_Window_GetDCXBufSize_isr(&stSrcRect,
                    bInterlaced, bMinSrc, ulBitsPerGroup, ulPixelPerGroup);
            }
        }
        else
        {
            BPXL_GetBytesPerNPixels_isr(eBufPxlFmt, pSrcRect->ulWidth, &uiPitch);

#if (BVDC_P_BUFFER_ADD_GUARD_MEMORY)
            if(bMosaicMode)
            {
                uiPitch += b3DMode ? BVDC_P_MAX_CAP_GUARD_MEMORY_3D
                    : BVDC_P_MAX_CAP_GUARD_MEMORY_2D;
            }
            else if(bFillBars)
            {
                ulStride = (stSrcRect.ulWidth * BVDC_P_DCXM_BITS_PER_PIXEL) / 8;
                ulStride += b3DMode ? BVDC_P_MAX_CAP_GUARD_FILLBAR_3D
                    : BVDC_P_MAX_CAP_GUARD_FILLBAR_2D;
                ulBufSize = (ulStride * pSrcRect->ulHeight) >> bInterlaced;
            }
            else
#endif
            {
                /* Need to be aligned for capture buffers */
                uiPitch = BVDC_P_ALIGN_UP(uiPitch, BVDC_P_PITCH_ALIGN);
            }
#if (BVDC_P_BUFFER_ADD_GUARD_MEMORY)
            ulStride = uiPitch;
#endif
            ulBufSize = (uiPitch * pSrcRect->ulHeight) >> bInterlaced;
        }
    }
    else
    {
        BPXL_GetBytesPerNPixels_isr(eBufPxlFmt, pSrcRect->ulWidth, &uiPitch);
        ulBufSize = (uiPitch * pSrcRect->ulHeight) >> bInterlaced;
    }

    /* Make sure 32 byte aligned */
    ulBufSize = BVDC_P_ALIGN_UP(ulBufSize, BVDC_P_HEAP_ALIGN_BYTES);

#if (BVDC_P_BUFFER_ADD_GUARD_MEMORY)
    BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("Win[%d] (%6s) pSrcRect : %d(%d)x%d (%d) ulBufSize %d",
        eWinId, (eBufHeapType == BVDC_P_BufHeapType_eCapture)   ? "Cap"    :
        (eBufHeapType == BVDC_P_BufHeapType_eMad_Pixel) ? "Mad_PX" :
        (eBufHeapType == BVDC_P_BufHeapType_eMad_QM)    ? "MAD_QM" : "Anr",
        pSrcRect->ulWidth, ulStride,
        (pSrcRect->ulHeight >> bInterlaced), bInterlaced, ulBufSize));
#else
    BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("Win[%d] (%6s) pSrcRect : %dx%d (%d) ulBufSize %d",
        eWinId, (eBufHeapType == BVDC_P_BufHeapType_eCapture)   ? "Cap"    :
        (eBufHeapType == BVDC_P_BufHeapType_eMad_Pixel) ? "Mad_PX" :
        (eBufHeapType == BVDC_P_BufHeapType_eMad_QM)    ? "MAD_QM" : "Anr",
        pSrcRect->ulWidth,
        (pSrcRect->ulHeight >> bInterlaced), bInterlaced, ulBufSize));
#endif

    if( pulBufSize )
    {
        *pulBufSize = ulBufSize;
    }

    BSTD_UNUSED(eWinId);
    return;
}

/***************************************************************************
 * {private}
 *
 * This function attempt to figure out which heap to use (HD, SD, Pip, etc)
 * for capture, MAD or ANR.
 *
 */
static void BVDC_P_Window_GetBufHeapId_isr
    ( const BVDC_Window_Handle         hWindow,
      uint32_t                         ulBufSize,
      BVDC_Heap_Handle                 hHeap,
      BVDC_P_BufferHeapId             *peBufferHeapIdRequest,
      BVDC_P_BufferHeapId             *peBufferHeapIdPrefer )
{
    BVDC_P_BufferHeapId  eBufferHeapId = BVDC_P_BufferHeapId_eUnknown;
    BVDC_P_BufferHeapId  eBufferHeapIdPrefer = BVDC_P_BufferHeapId_eUnknown;
    BVDC_P_BufferHeap_SizeInfo   *pHeapSizeInfo;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hHeap, BVDC_BFH);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
    pHeapSizeInfo = &hHeap->stHeapSizeInfo;
    /* Get buffer heap ID by size */
    BVDC_P_BufferHeap_GetHeapIdBySize_isr(&hHeap->stHeapSizeInfo,
        ulBufSize, &eBufferHeapId);

    /* Update eBufferHeapIdPrefer for capture buffers */
    if(hWindow->eId == BVDC_P_WindowId_eComp0_V0)
    {
        uint32_t  ul4HDBufSize, ul2HDBufSize, ulHDBufSize;

        ulHDBufSize  = pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_eHD]];
        ul2HDBufSize = pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e2HD]];
        ul4HDBufSize = pHeapSizeInfo->aulBufSize[pHeapSizeInfo->aulIndex[BVDC_P_BufferHeapId_e4HD]];

        if((hHeap->stSettings.ulBufferCnt_4HD) &&
           (ulBufSize < ul4HDBufSize))
        {
            eBufferHeapIdPrefer = BVDC_P_BufferHeapId_e4HD;
        }
        else if((hHeap->stSettings.ulBufferCnt_2HD) &&
           (ulBufSize < ul2HDBufSize))
        {
            eBufferHeapIdPrefer = BVDC_P_BufferHeapId_e2HD;
        }
        else if((hHeap->stSettings.ulBufferCnt_HD) &&
           (ulBufSize < ulHDBufSize))
        {
            eBufferHeapIdPrefer = BVDC_P_BufferHeapId_eHD;
        }
    }

    BDBG_MSG(("Window[%d] selects buffer %s, prefered buffer %s", hWindow->eId,
        BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapId),
        BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapIdPrefer)));

    /* Update */
    if(peBufferHeapIdRequest)
    {
        *peBufferHeapIdRequest = eBufferHeapId;
    }

    if( peBufferHeapIdPrefer )
    {
        *peBufferHeapIdPrefer = eBufferHeapIdPrefer;
    }

    return;
}


static void BVDC_P_Window_AcquireBvnResources_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_Resource_Handle hResource;

    hResource = hWindow->hCompositor->hVdc->hResource;


    if(BVDC_P_RESOURCE_ID_AVAIL == (unsigned long) hWindow->stNewResource.hScaler)
    {
        BVDC_P_Scaler_Handle *phScaler=&hWindow->stNewResource.hScaler;

        BVDC_P_Resource_AcquireHandle_isr(hResource,
            BVDC_P_ResourceType_eScl, hWindow->stResourceFeature.ulScl,
            (unsigned long) hWindow, (void **)phScaler, false);

        BDBG_ASSERT(NULL != hWindow->stNewResource.hScaler);
        hWindow->stCurResource.hScaler = hWindow->stNewResource.hScaler;

        BVDC_P_Scaler_Init_isr(hWindow->stCurResource.hScaler, hWindow);
    }


    if(BVDC_P_RESOURCE_ID_AVAIL == (unsigned long) hWindow->stNewResource.hMcvp)
    {
        BVDC_P_Mcvp_Handle *phMcvp=&hWindow->stNewResource.hMcvp;
        BVDC_P_Resource_AcquireHandle_isr(hResource,
            BVDC_P_ResourceType_eMcvp, hWindow->stResourceFeature.ulMad,
            hWindow->eId, (void **)phMcvp, false);

        BDBG_ASSERT(NULL != hWindow->stNewResource.hMcvp);
        hWindow->stCurResource.hMcvp = hWindow->stNewResource.hMcvp;

        BVDC_P_Mcvp_AcquireConnect_isr(hWindow->stCurResource.hMcvp, hWindow->hDeinterlacerHeap, hWindow);

        BVDC_P_Window_Compression_Init_isr(hWindow->bIs10BitCore, hWindow->bSupportDcxm,
        NULL, &hWindow->stMadCompression, hWindow->stCurResource.hMcvp->eDcxCore);
    }

#if (BVDC_P_SUPPORT_XSRC)
    if(BVDC_P_RESOURCE_ID_AVAIL == (unsigned long) hWindow->stNewResource.hXsrc)
    {
        BVDC_P_Xsrc_Handle *phXsrc=&hWindow->stNewResource.hXsrc;

        BVDC_P_Resource_AcquireHandle_isr(hResource,
            BVDC_P_ResourceType_eXsrc, 0, hWindow->stNewInfo.hSource->eId,
            (void **)phXsrc, false);

        BDBG_ASSERT(NULL != hWindow->stNewResource.hXsrc);
        hWindow->stCurResource.hXsrc = hWindow->stNewResource.hXsrc;

        BVDC_P_Xsrc_AcquireConnect_isr(hWindow->stCurResource.hXsrc, hWindow->stNewInfo.hSource);
    }
#endif

#if (BVDC_P_SUPPORT_VFC)
    if(BVDC_P_RESOURCE_ID_AVAIL == (unsigned long) hWindow->stNewResource.hVfc)
    {
        BVDC_P_Vfc_Handle *phVfc=&hWindow->stNewResource.hVfc;

        BVDC_P_Resource_AcquireHandle_isr(hResource,
            BVDC_P_ResourceType_eVfc, 0, hWindow->eId,
            (void **)phVfc, false);

        BDBG_ASSERT(NULL != hWindow->stNewResource.hVfc);
        hWindow->stCurResource.hVfc = hWindow->stNewResource.hVfc;

        BVDC_P_Vfc_AcquireConnect_isr(hWindow->stCurResource.hVfc, hWindow);
    }
#endif

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_RESOURCE_ID_AVAIL == (unsigned long) hWindow->stNewResource.hTntd)
    {
        BVDC_P_Tntd_Handle *phTntd=&hWindow->stNewResource.hTntd;

        BVDC_P_Resource_AcquireHandle_isr(hResource,
            BVDC_P_ResourceType_eTntd, 0, hWindow->stNewInfo.hSource->eId,
            (void **)phTntd, false);

        BDBG_ASSERT(NULL != hWindow->stNewResource.hTntd);
        hWindow->stCurResource.hTntd = hWindow->stNewResource.hTntd;

        BVDC_P_Tntd_AcquireConnect_isr(hWindow->stCurResource.hTntd, hWindow);
    }
#endif

#if (BVDC_P_SUPPORT_DNR)
    if(BVDC_P_RESOURCE_ID_AVAIL == (unsigned long) hWindow->stNewResource.hDnr)
    {
        BVDC_P_Dnr_Handle *phDnr=&hWindow->stNewResource.hDnr;
        uint32_t ulDnrCap = hWindow->stNewInfo.hSource->bIs10BitCore ? BVDC_P_Able_e10bits : BVDC_P_Able_e8bits;

        BVDC_P_Resource_AcquireHandle_isr(hResource,
            BVDC_P_ResourceType_eDnr, ulDnrCap, hWindow->stNewInfo.hSource->eId,
            (void **)phDnr, false);

        BDBG_ASSERT(NULL != hWindow->stNewResource.hDnr);
        hWindow->stCurResource.hDnr = hWindow->stNewResource.hDnr;

        BVDC_P_Dnr_AcquireConnect_isr(hWindow->stCurResource.hDnr, hWindow->stNewInfo.hSource);

        /* Reconfig vnet */
        BVDC_P_Window_SetReconfiguring_isr(hWindow, false, true, false);
    }
#endif

#if (BVDC_P_SUPPORT_BOX_DETECT)
    if(BVDC_P_RESOURCE_ID_AVAIL == (unsigned long) hWindow->stNewResource.hBoxDetect)
    {
        BVDC_P_BoxDetect_Handle *phBoxDetect=&hWindow->stNewResource.hBoxDetect;
        BVDC_P_Resource_AcquireHandle_isr(hResource,
            BVDC_P_ResourceType_eBoxDetect, 0, hWindow->stNewInfo.hSource->eId,
            (void **)phBoxDetect, false);

        BDBG_ASSERT(NULL != hWindow->stNewResource.hBoxDetect);
        hWindow->stCurResource.hBoxDetect = hWindow->stNewResource.hBoxDetect;

        BVDC_P_BoxDetect_AcquireConnect_isr(hWindow->stCurResource.hBoxDetect, hWindow->stNewInfo.hSource->eId,
            &(hWindow->stNewInfo.hSource->stCurInfo));
    }
#endif

    if(BVDC_P_RESOURCE_ID_AVAIL == (unsigned long) hWindow->stNewResource.hVnetCrc)
    {
        BVDC_P_VnetCrc_Handle *phVnetCrc=&hWindow->stNewResource.hVnetCrc;

        BVDC_P_Resource_AcquireHandle_isr(hResource,
            BVDC_P_ResourceType_eVnetCrc, 0, hWindow->eId, (void **)phVnetCrc, false);

        BDBG_ASSERT(NULL != hWindow->stNewResource.hVnetCrc);
        hWindow->stCurResource.hVnetCrc = hWindow->stNewResource.hVnetCrc;

        BVDC_P_VnetCrc_AcquireConnect_isr(
            hWindow->stCurResource.hVnetCrc, hWindow);
    }
}

/***************************************************************************
 * {private}
 */
BERR_Code BVDC_P_Window_ApplyChanges_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_Window_Info *pNewInfo;
    BVDC_P_Window_Info *pCurInfo;
    BVDC_P_Window_DirtyBits *pNewDirty;
    bool  bWindowStateChanged = false;

    BDBG_ENTER(BVDC_P_Window_ApplyChanges_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    /* Get the compositor that this window belongs to. */
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hWindow->stNewInfo.hSource, BVDC_SRC);

    /* To reduce the amount of typing */
    pNewInfo  = &hWindow->stNewInfo;
    pCurInfo  = &hWindow->stCurInfo;
    pNewDirty = &pNewInfo->stDirty;

    /* Update to take in new changes. */
    BVDC_P_Window_SetBlender_isr(hWindow, pNewInfo->ucZOrder,
        pNewInfo->ucConstantAlpha, pNewInfo->eFrontBlendFactor,
        pNewInfo->eBackBlendFactor);

    /* If user change blender factor, will see garbage flash because blender
     * factor is set immediately, while bVisible is saved in BVDC_P_PictureNode,
     * will be set few vsync later. Can not move blender settings in
     * BVDC_P_PictureNode, since graphics doesn't have BVDC_P_PictureNode, and
     * delay for Main and PIP window will be different, so there will be mismatch
     * which could cause BVN error. So Mute window for BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT
     * vsyns for if blender settings and bVisible change
     */
    if(((hWindow->stNewInfo.ucConstantAlpha   != hWindow->stCurInfo.ucConstantAlpha) ||
       (hWindow->stNewInfo.eFrontBlendFactor != hWindow->stCurInfo.eFrontBlendFactor) ||
       (hWindow->stNewInfo.eBackBlendFactor  != hWindow->stCurInfo.eBackBlendFactor)) &&
       (hWindow->stNewInfo.bVisible != hWindow->stCurInfo.bVisible))
    {
        hWindow->ulBlenderMuteCnt = BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT;
        BVDC_P_Window_WinOutSetEnable_isr(hWindow, 0, false);
    }

    BVDC_P_Window_SetMiscellaneous_isr(hWindow, pNewInfo);

    /* Allocate new resource if needed */
    if(hWindow->bAllocResource)
    {
        BVDC_P_Window_AcquireBvnResources_isr(hWindow);
        hWindow->bAllocResource = false;
    }

    /* Release resources that are allocated but unused!  Since it's unused
     * it does not trigger shutdown to release them.  Release them here
     * to avoid forced-shutdown side-effects. */
    if((!hWindow->stVnetMode.stBits.bUseMvp) &&
       (!hWindow->stNewInfo.bDeinterlace) &&
       (!hWindow->stNewInfo.bAnr) &&
       ( hWindow->stCurResource.hMcvp))
    {
        BDBG_MSG(("window[%d] releases shared MCVP", hWindow->eId));
        BVDC_P_Mcvp_ReleaseConnect_isr(&hWindow->stCurResource.hMcvp);
        hWindow->stNewResource.hMcvp = NULL;
    }

#if (BVDC_P_SUPPORT_TNTD)
    if((!hWindow->stVnetMode.stBits.bUseTntd) &&
       (!hWindow->stNewInfo.bSharpnessEnable) &&
       ( hWindow->stCurResource.hTntd))
    {
        BDBG_MSG(("window[%d] releases shared TNTD", hWindow->eId));
        BVDC_P_Tntd_ReleaseConnect_isr(&hWindow->stCurResource.hTntd);
        hWindow->stNewResource.hTntd = NULL;
    }
#endif

    /* State transitions. */
    if(BVDC_P_STATE_IS_CREATE(hWindow))
    {
        /* (1) Connect this window with new source. */
        hWindow->eState = BVDC_P_State_eActive;

        /* this flags a window is being created; */
        bWindowStateChanged = true;

        BVDC_P_Source_ConnectWindow_isr(hWindow->stNewInfo.hSource, hWindow);
        if(BVDC_P_SRC_IS_VIDEO(hWindow->stNewInfo.hSource->eId))
        {
            /* mute the window in src-pending mode */
            if(hWindow->stNewInfo.hSource->stNewInfo.eResumeMode)
            {
                BDBG_MSG(("Source[%d] is pending mode!", hWindow->stNewInfo.hSource->eId));
                BVDC_P_Window_SetReconfiguring_isr(hWindow, true, false, false);
            }
            else  /* start the new win now! */
            {
                /* pCurInfo->eWriterState  = BVDC_P_State_eShutDownPending;
                 * will cause a shudown before vnet start, but it is not needed.
                 * without that, the following cause vnet start right away */
                BVDC_P_Window_SetReconfiguring_isr(hWindow, false, true, false);
            }
        }
        else /* gfx window */
        {
            pNewInfo->eReaderState = BVDC_P_State_eActive;
            pNewInfo->eWriterState = BVDC_P_State_eActive;
        }
#if BVDC_P_SUPPORT_STG
        if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
        {
            hWindow->hCompositor->bGfxChannelChange = false;
        }
#endif
        BDBG_MSG(("Window[%d] activated.", hWindow->eId));
        BDBG_MSG(("Window[%d] is BVDC_P_State_eActive", hWindow->eId));
    }
    else if(BVDC_P_STATE_IS_DESTROY(hWindow))
    {
        /* (1) Disconnecting this window from source. */
        pNewDirty->stBits.bDestroy       = BVDC_P_DIRTY;
        hWindow->bUserAppliedChanges     = true;
        hWindow->bSetDestroyEventPending = true;
        BDBG_MSG(("Window[%d] de-activated.", hWindow->eId));

        /* inform BVDC_P_CheckApplyChangesStatus to wait for destroy-done event */
        hWindow->eState                  = BVDC_P_State_eShutDownPending;
        BDBG_MSG(("(1) Window[%d] is BVDC_P_State_eShutDownPending", hWindow->eId));

#if BVDC_P_SUPPORT_STG
        /* xcode clear bGfxChannelchange indicating meaningless gfx content*/
        if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
        {
            hWindow->hCompositor->bGfxChannelChange = true;
        }
#endif
        /* in this case BLEND_SOURCE is already set to BACKGROUND_BYPASS by
         * BVDC_P_Window_SetBlender_isr, the next reader RUL must start to
         * shut down. Therefore we can not wait for writer_isr to setReconfig
         * in the case that reader builds RUL next */
        BVDC_P_Window_SetReconfiguring_isr(hWindow, false, true, false);
    }

    /* Update does not affect the number of buffer nodes nor vnet. */
    if((pNewInfo->ucAlpha           != pCurInfo->ucAlpha           ) ||
       (pNewInfo->bVisible          != pCurInfo->bVisible          ) ||
       (pNewInfo->ucZOrder          != pCurInfo->ucZOrder          ) ||
       (pNewInfo->ucConstantAlpha   != pCurInfo->ucConstantAlpha   ) ||
       (pNewInfo->eBackBlendFactor  != pCurInfo->eBackBlendFactor  ) ||
       (pNewInfo->eFrontBlendFactor != pCurInfo->eFrontBlendFactor ) ||
       (pNewInfo->ulMosaicTrackChannelId != pCurInfo->ulMosaicTrackChannelId) ||
       (pNewInfo->sHdrPeakBrightness != pCurInfo->sHdrPeakBrightness) ||
       (pNewInfo->sSdrPeakBrightness != pCurInfo->sSdrPeakBrightness) ||
       (pNewInfo->bUseSrcFrameRate  != pCurInfo->bUseSrcFrameRate  ))
    {
        hWindow->bUserAppliedChanges = true;
    }

    /* This ensure that the callback will get call at least once. */
    if((pNewInfo->pfGenCallback      != pCurInfo->pfGenCallback      ) ||
       (pNewInfo->pvGenCallbackParm1 != pCurInfo->pvGenCallbackParm1 ) ||
       (pNewInfo->iGenCallbackParm2  != pCurInfo->iGenCallbackParm2  ))
    {
        hWindow->stCbData.stMask.bSyncLock = BVDC_P_DIRTY;
        hWindow->stCbData.stMask.bRectAdjust = BVDC_P_DIRTY;
        hWindow->stCbData.ulVsyncDelay = BVDC_P_LIP_SYNC_RESET_DELAY;
        hWindow->stCbData.ulDriftDelay = BVDC_P_LIP_SYNC_RESET_DELAY;
        hWindow->stCbData.ulGameModeDelay = BVDC_P_LIP_SYNC_RESET_DELAY;
        hWindow->stCbData.stOutputRect.lLeft= pNewInfo->stDstRect.lLeft;
        hWindow->stCbData.stOutputRect.lTop = pNewInfo->stDstRect.lTop;
        hWindow->stCbData.stOutputRect.ulWidth = pNewInfo->stDstRect.ulWidth;
        hWindow->stCbData.stOutputRect.ulHeight = pNewInfo->stDstRect.ulHeight;
        hWindow->bUserAppliedChanges = true;
    }

#if (BVDC_P_SUPPORT_TNTD)
    if((pNewInfo->bSharpnessEnable != pCurInfo->bSharpnessEnable) ||
       (pNewInfo->sSharpness       != pCurInfo->sSharpness      ))
    {
        hWindow->bUserAppliedChanges = true;
    }
#endif

    /* Any of the dirty bit set then mark as applied to be use for next rul
     * building. */
    if(BVDC_P_IS_DIRTY(pNewDirty))
    {
        hWindow->bUserAppliedChanges = true;
        BDBG_MSG(("Window[%d]'s new dirty bits = 0x%08x",
            hWindow->eId, pNewDirty->aulInts[0]));
    }

    /* Window to be removed or added? */
    if((hWindow->stNewInfo.hSource->stNewInfo.ulWindows != hWindow->stNewInfo.hSource->stCurInfo.ulWindows) ||
       (hWindow->stNewInfo.hSource->stNewInfo.bPsfEnable!= hWindow->stNewInfo.hSource->stCurInfo.bPsfEnable) ||
       (hWindow->stNewInfo.hSource->stNewInfo.bForceFrameCapture != hWindow->stNewInfo.hSource->stCurInfo.bForceFrameCapture) ||
       (hWindow->stNewInfo.hSource->stNewInfo.bDnr      != hWindow->stNewInfo.hSource->stCurInfo.bDnr) ||
       (hWindow->stNewInfo.hSource->stNewInfo.stSplitScreenSetting.eDnr != hWindow->stNewInfo.hSource->stCurInfo.stSplitScreenSetting.eDnr))
    {
        hWindow->bUserAppliedChanges = true;
    }

    /* game mode tracking window */
    if((pNewInfo->stGameDelaySetting.bEnable != pCurInfo->stGameDelaySetting.bEnable) ||
       (pNewInfo->stGameDelaySetting.ulBufferDelayTarget != pCurInfo->stGameDelaySetting.ulBufferDelayTarget) ||
       (pNewInfo->stGameDelaySetting.ulBufferDelayTolerance != pCurInfo->stGameDelaySetting.ulBufferDelayTolerance))
    {
        hWindow->bUserAppliedChanges = true;

        /* Note: only one window could be game mode tracked */
        if(pNewInfo->stGameDelaySetting.bEnable)
        {
            hWindow->hCompositor->hDisplay->hWinGameMode = hWindow;
        }
    }

    /* Update to be pick up by display queue, could change the number
     * of buffers node and/or vnet.  Sensitive to input/output size
     * changes. */
    if((bWindowStateChanged) ||
       (hWindow->bUserAppliedChanges) || /* preset by dirty bits model. */
       (pNewInfo->stAfdSettings.eMode       != pCurInfo->stAfdSettings.eMode ) ||
       (pNewInfo->stAfdSettings.eClip       != pCurInfo->stAfdSettings.eClip ) ||
       (pNewInfo->stAfdSettings.ulAfd       != pCurInfo->stAfdSettings.ulAfd ) ||
       (pNewInfo->ePanScanType              != pCurInfo->ePanScanType        ) ||
       (pNewInfo->lUserHorzPanScan          != pCurInfo->lUserHorzPanScan    ) ||
       (pNewInfo->lUserVertPanScan          != pCurInfo->lUserVertPanScan    ) ||
       (pNewInfo->eAspectRatioMode          != pCurInfo->eAspectRatioMode    ) ||
       (pNewInfo->ulNonlinearSrcWidth       != pCurInfo->ulNonlinearSrcWidth ) ||
       (pNewInfo->ulNonlinearSclOutWidth    != pCurInfo->ulNonlinearSclOutWidth ) ||
       (pNewInfo->uiVsyncDelayOffset        != pCurInfo->uiVsyncDelayOffset  ) ||
       (pNewInfo->bForceCapture             != pCurInfo->bForceCapture       ) ||
       (pNewInfo->ulHrzSclFctRndToler       != pCurInfo->ulHrzSclFctRndToler ) ||
       (pNewInfo->ulVrtSclFctRndToler       != pCurInfo->ulVrtSclFctRndToler ) ||
       (pNewInfo->ulBandwidthDelta          != pCurInfo->ulBandwidthDelta    ) ||
       (pNewInfo->eSclCapBias               != pCurInfo->eSclCapBias         ) ||
       (hWindow->hCompositor->stNewInfo.eOrientation != hWindow->hCompositor->stCurInfo.eOrientation) ||
       (!BVDC_P_RECT_CMP_EQ(&pNewInfo->stScalerOutput, &pCurInfo->stScalerOutput)) ||
       (!BVDC_P_RECT_CMP_EQ(&pNewInfo->stDstRect,      &pCurInfo->stDstRect     )) ||
       (!BVDC_P_CLIP_RECT_CMP_EQ(&pNewInfo->stSrcClip, &pCurInfo->stSrcClip     )) ||
       (hWindow->hCompositor->bDspAspRatDirty) ||
       (BVDC_P_ItState_eActive != hWindow->hCompositor->hDisplay->eItState))
    {
        hWindow->bUserAppliedChanges = true;
        if(BVDC_P_SRC_IS_VIDEO(hWindow->stNewInfo.hSource->eId))
        {
            /* Set number of user capture buffers that will be used */
            if(pNewInfo->uiCaptureBufCnt != pCurInfo->uiCaptureBufCnt)
            {
                hWindow->uiAvailCaptureBuffers = pNewInfo->uiCaptureBufCnt;
                if(pNewInfo->uiCaptureBufCnt > pCurInfo->uiCaptureBufCnt)
                {
                    BDBG_MSG(("Added %d buffers for user capture", hWindow->uiAvailCaptureBuffers));
                }
                else
                {
                    BDBG_MSG(("Decremented %d buffers for user capture", hWindow->uiAvailCaptureBuffers));
                }
            }

            /* Partial decision on the vnet mode and cap buf count, according to user's
             * setting only.  Further and complete decision are done later with
             * combination of both user settings and dynamic src info in writer_isr. */
            BVDC_P_Window_DecideCapture_isr(
                hWindow, hWindow->stNewInfo.hSource, hWindow->hCompositor);

            /* inform writer_isr to redecide vnet mode */
            hWindow->stNewInfo.stDirty.stBits.bReDetVnet = BVDC_P_DIRTY;

            /* Add number of user capture buffers to number of needed buffers. */
            /* This code is a bit tricky here. We always use the uiCaptureBufCnt
             * from pNewInfo. This is under the fact that if we changed capture
             * buffer count before, that number is always stored in hWindow->stNewInfo
             * till user changes it, which will also be done thru hWindow->stNewinfo.
             */
            hWindow->ulBufCntNeeded += pNewInfo->uiCaptureBufCnt;

            if(hWindow->ulBufCntNeeded != hWindow->ulBufCntAllocated)
            {
                BDBG_MSG(("apply: lBufCntAllocated (%d) != ulBufCntNeeded (%d), stVnetMode = 0x%x",
                    hWindow->ulBufCntAllocated,
                    hWindow->ulBufCntNeeded, *(unsigned int *)&hWindow->stVnetMode));
                pNewDirty->stBits.bReallocBuffers = BVDC_P_DIRTY;
            }

            /* The life of dirty bit bUserCaptureBuffer ends here since new dirty bit
             * bReallocBuffers has been set up if buffer re-allocation needs to happen.
             */
            pNewDirty->stBits.bUserCaptureBuffer = BVDC_P_CLEAN;

            /* this has to stay here (rather than in writer_isr) to avoid reader using
             * those old bufs before writer_isr flush them */
            if(BVDC_P_ItState_eActive != hWindow->hCompositor->hDisplay->eItState ||
              ((BVDC_P_ItState_eActive == hWindow->hCompositor->hDisplay->eItState) &&
                BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg) &&
                NULL == hWindow->hCompositor->hDisplay->pStgFmtInfo))
            {
                BDBG_MSG(("window[%d] flushes buffers due to display format change",
                    hWindow->eId));
                BVDC_P_Buffer_Invalidate_isr(hWindow->hBuffer);
                hWindow->pCurWriterNode =
                    BVDC_P_Buffer_GetCurWriterNode_isr(hWindow->hBuffer);
                hWindow->pCurReaderNode =
                    BVDC_P_Buffer_GetCurReaderNode_isr(hWindow->hBuffer);

                /* NOTE: display format switch must invalidate window MAD delay sw pipeline
                 * if MAD is at vnet reader side!
                 * since it might not cause vnet reconfig and MAD delay pipeline might not
                 * have hard_start initialization, which may result in BVN error! */
                if(BVDC_P_MVP_USED_MAD(hWindow->stMvpMode))
                    hWindow->bResetMadDelaySwPipe = true;
            }
        }
    }/* don't make above changes if no changes */

    /* reset the count for gfx window to be programmed later in reader isr to
     * accomandate vbi pass through info. */
    if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
    {
        /* hWindow->ulGwinSetCount = 0; */
        /* BACK OUT the above changes for now to make analog working */
        BAVC_Polarity eScanType = (hWindow->hCompositor->stNewInfo.pFmtInfo->bInterlaced)
            ? BAVC_Polarity_eTopField : BAVC_Polarity_eFrame;
        BVDC_P_GfxFeeder_GetAdjSclOutRect_isr(&pNewInfo->stSrcClip, &pNewInfo->stScalerOutput,
            &pNewInfo->stDstRect, &hWindow->stAdjSclOut);
        BVDC_P_Window_SetSurfaceSize_isr(hWindow, &hWindow->stAdjSclOut, eScanType);
        BVDC_P_Window_SetDisplaySize_isr(hWindow, &pNewInfo->stDstRect, eScanType,
            (uint32_t)(pNewInfo->stDstRect.lLeft + pNewInfo->lRWinXOffsetDelta));
    }

    /* Isr will set event to notify apply done. */
    if(hWindow->bUserAppliedChanges)
    {
        /* inform *DataReady_isr to recalculate the adjusted rectangles,
         * for this window and related window that share the same source. */
        hWindow->bAdjRectsDirty  = true;
        hWindow->stNewInfo.hSource->bPictureChanged = true;
#if BVDC_P_SUPPORT_MOSAIC_MODE
        hWindow->stNewInfo.hSource->ulMosaicCount = hWindow->stNewInfo.ulMaxMosaicCount;
#endif
        /* If BypassDisplay's 656 is not enable it will not produce interrupt
         * hence no one to set the event, so we just set it here. */
        if((!hWindow->hCompositor->hDisplay->bIsBypass) ||
           (hWindow->hCompositor->hDisplay->stCurInfo.bEnable656) ||
           (hWindow->hCompositor->hDisplay->stCurInfo.bEnableHdmi))
        {
            hWindow->bSetAppliedEventPending = true;
            BKNI_ResetEvent(hWindow->hAppliedDoneEvent);
            BDBG_MSG(("Window[%d] New changes reset applied event.", hWindow->eId));
        }

        /* copy stNewInfo to stCurInfo here !!! */
        BVDC_P_Window_UpdateUserState_isr(hWindow);
    }

    /* ANR demo mode */
#if (BVDC_P_SUPPORT_MANR)
    if(hWindow->stCurResource.hMcvp && hWindow->stCurResource.hMcvp->hAnr)
    {
        pNewInfo->stDirty.stBits.bAnrAdjust =
            (hWindow->stNewInfo.stSplitScreenSetting.eAnr != hWindow->stCurResource.hMcvp->hAnr->eDemoMode);
        BVDC_P_Anr_SetDemoMode_isr(hWindow->stCurResource.hMcvp->hAnr, hWindow->stNewInfo.stSplitScreenSetting.eAnr);

    }
#endif

    if(hWindow->bNotEnoughCapBuf)
    {
        /* Clear bBufferPending so we can re-check if there is enough
         * buffers for user change */
        pNewDirty->stBits.bBufferPending = BVDC_P_CLEAN;
        BVDC_P_Window_SetReconfiguring_isr(hWindow, false, true, false);
    }

    BDBG_LEAVE(BVDC_P_Window_ApplyChanges_isr);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 */
BERR_Code BVDC_P_Window_AbortChanges
    ( BVDC_Window_Handle               hWindow )
{
    BDBG_ENTER(BVDC_P_Window_ApplyChanges_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* copy stCurInfo to stNewInfo here !!! */
    hWindow->stNewInfo = hWindow->stCurInfo;

    BDBG_LEAVE(BVDC_P_Window_AbortChanges);
    return BERR_SUCCESS;
}


/***************************************************************************
 * {private}
 *
 * This function is called to release resources, to cleanup, to send done
 * event to ApplyChanges, after vnet shut down is completed.
 *
 * Video Window
 *   Shutdown -> Inactive -> Set destroy done event
 *
 * Gfx Window
 *   ShutdownPending -> Shutdown -> Inactive -> Set destroy done event
 */
static void BVDC_P_Window_ProcPostShutDown_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_Window_Info *pCurInfo;
    BVDC_P_Window_DirtyBits *pCurDirty;
    BVDC_P_Resource_Handle hResource;

    BDBG_ENTER(BVDC_P_Window_ProcPostShutDown_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    pCurInfo  = &hWindow->stCurInfo;
    pCurDirty = &hWindow->stCurInfo.stDirty;

    /* This print alter isr timing dramatically, so leave it to avoid
     * flooding of messages. */
#if (BVDC_P_PRINT_DIRTY_BITS)
    if(BVDC_P_IS_DIRTY(pCurDirty))
    {
        BDBG_ERR(("pCurDirty->stBits.bRecAdjust             = %d", pCurDirty->stBits.bRecAdjust));
        BDBG_ERR(("pCurDirty->stBits.bReDetVnet             = %d", pCurDirty->stBits.bReDetVnet));
        BDBG_ERR(("pCurDirty->stBits.bCscAdjust             = %d", pCurDirty->stBits.bCscAdjust));
        BDBG_ERR(("pCurDirty->stBits.bTntAdjust             = %d", pCurDirty->stBits.bTntAdjust));
        BDBG_ERR(("pCurDirty->stBits.bLabAdjust             = %d", pCurDirty->stBits.bLabAdjust));
        BDBG_ERR(("pCurDirty->stBits.bCabAdjust             = %d", pCurDirty->stBits.bCabAdjust));
        BDBG_ERR(("pCurDirty->stBits.bDitAdjust             = %d", pCurDirty->stBits.bDitAdjust));
        BDBG_ERR(("pCurDirty->stBits.bColorKeyAdjust        = %d", pCurDirty->stBits.bColorKeyAdjust));
        BDBG_ERR(("pCurDirty->stBits.bShutdown              = %d", pCurDirty->stBits.bShutdown));
        BDBG_ERR(("pCurDirty->stBits.bDestroy               = %d", pCurDirty->stBits.bDestroy));
        BDBG_ERR(("pCurDirty->stBits.bSrcPending            = %d", pCurDirty->stBits.bSrcPending));
        BDBG_ERR(("pCurDirty->stBits.bReConfigVnet          = %d", pCurDirty->stBits.bReConfigVnet));
        BDBG_ERR(("pCurDirty->stBits.bDeinterlace           = %d", pCurDirty->stBits.bDeinterlace));
        BDBG_ERR(("pCurDirty->stBits.bAnrAdjust             = %d", pCurDirty->stBits.bAnrAdjust));
        BDBG_ERR(("pCurDirty->stBits.bSharedScl             = %d", pCurDirty->stBits.bSharedScl));
        BDBG_ERR(("pCurDirty->stBits.bUserCaptureBuffer     = %d", pCurDirty->stBits.bUserCaptureBuffer));
        BDBG_ERR(("pCurDirty->stBits.bUserReleaseBuffer     = %d", pCurDirty->stBits.bUserReleaseBuffer));
        BDBG_ERR(("pCurDirty->stBits.bMosaicMode            = %d", pCurDirty->stBits.bMosaicMode));
        BDBG_ERR(("pCurDirty->stBits.bReallocBuffers        = %d", pCurDirty->stBits.bReallocBuffers));
        BDBG_ERR(("pCurDirty->stBits.bBoxDetect             = %d", pCurDirty->stBits.bBoxDetect));
        BDBG_ERR(("pCurDirty->stBits.bHistoRect             = %d", pCurDirty->stBits.bHistoRect));
        BDBG_ERR(("pCurDirty->stBits.bVnetCrc               = %d", pCurDirty->stBits.bVnetCrc));
        BDBG_ERR(("pCurDirty->stBits.bBufferPending         = %d", pCurDirty->stBits.bBufferPending));
#if (BVDC_P_SUPPORT_TNTD)
        BDBG_ERR(("pCurDirty->stBits.bTntd                  = %d", pCurDirty->stBits.bTntd));
#endif
        BDBG_ERR(("bSetAppliedEventPending           = %d", hWindow->bSetAppliedEventPending));
        BDBG_ERR(("----------------------------------------"));
    }
#endif

    /* clean up right after shut-down, or win-destroy applied when
     * the vnet is idle due to src pending */
    if(((BVDC_P_State_eShutDown == pCurInfo->eReaderState) &&
        (BVDC_P_State_eShutDown == pCurInfo->eWriterState)) ||
       ((pCurDirty->stBits.bDestroy) &&
        (BVDC_P_State_eInactive == pCurInfo->eReaderState) &&
        (BVDC_P_State_eInactive == pCurInfo->eWriterState)))
    {
        hResource = hWindow->hCompositor->hVdc->hResource;

        BVDC_P_Window_SetInvalidVnetMode_isr(&(hWindow->stVnetMode));
        pCurInfo->eReaderState   = BVDC_P_State_eInactive;
        pCurInfo->eWriterState   = BVDC_P_State_eInactive;
        pCurDirty->stBits.bShutdown     = BVDC_P_CLEAN;

        BDBG_MODULE_MSG(BVDC_WIN_VNET,("Win[%d] shutdown completed", hWindow->eId));

        BDBG_OBJECT_ASSERT(pCurInfo->hSource, BVDC_SRC);

        /* release shared scaler for SD PIP or Bypass window */
        if((hWindow->stCurResource.hScaler) && (!hWindow->stResourceRequire.bRequireScaler) &&
            /* win destroyed by user, or */
            (pCurDirty->stBits.bDestroy ||
             /* bypass win that does not need scl */
             ((BVDC_P_WindowId_eComp2_V0 == hWindow->eId) &&
              !(BVDC_P_SRC_IS_MPEG(pCurInfo->hSource->eId)  ||
                BVDC_P_SRC_IS_HDDVI(pCurInfo->hSource->eId)))
             ))
        {
            BDBG_MSG(("window %d releases shared SCL", hWindow->eId));
            BVDC_P_Resource_ReleaseHandle_isr(hResource,
                BVDC_P_ResourceType_eScl, (void *)hWindow->stCurResource.hScaler);
            hWindow->stCurResource.hScaler = hWindow->stNewResource.hScaler = NULL;
        }

#if (BVDC_P_SUPPORT_DNR)
        /* relase unneeded dnr */
        if((hWindow->stCurResource.hDnr) &&
           (pCurDirty->stBits.bDestroy ||
           (!pCurInfo->hSource->stCurInfo.bDnr && !pCurInfo->hSource->stNewInfo.bDnr)))
        {
            BDBG_MSG(("Window %d releases shared DNR", hWindow->eId));
            BVDC_P_Dnr_ReleaseConnect_isr(&hWindow->stCurResource.hDnr);
            hWindow->stNewResource.hDnr = NULL;
        }
#endif

#if BVDC_P_SUPPORT_BOX_DETECT
        /* relase unneeded box-detect */
        if((hWindow->stCurResource.hBoxDetect) &&
           (pCurDirty->stBits.bDestroy ||
            (!hWindow->stCurInfo.bBoxDetect &&
             !hWindow->stNewInfo.bBoxDetect)))
        {
            BDBG_MSG(("Window %d releases shared BoxDetect", hWindow->eId));
            BVDC_P_BoxDetect_ReleaseConnect_isr(&hWindow->stCurResource.hBoxDetect);
            hWindow->stNewResource.hBoxDetect = NULL;
            pCurDirty->stBits.bBoxDetect = BVDC_P_CLEAN;
        }
#endif

        /* relase unneeded VnetCrc */
        if((hWindow->stCurResource.hVnetCrc) &&
           (pCurDirty->stBits.bDestroy ||
            (!hWindow->stCurInfo.stCbSettings.stMask.bCrc &&
             !hWindow->stNewInfo.stCbSettings.stMask.bCrc)))
        {
            BDBG_MSG(("Window %d releases shared VnetCrc", hWindow->eId));
            BVDC_P_VnetCrc_ReleaseConnect_isr(&hWindow->stCurResource.hVnetCrc);
            hWindow->stNewResource.hVnetCrc = NULL;
        }

#if (BVDC_P_SUPPORT_TNTD)
        if((hWindow->stCurResource.hTntd) &&
           (pCurDirty->stBits.bDestroy ||
            (!hWindow->stCurInfo.bSharpnessEnable &&
             !hWindow->stNewInfo.bSharpnessEnable)))
        {
            BDBG_MSG(("window %d releases shared TNTD", hWindow->eId));
            BVDC_P_Tntd_ReleaseConnect_isr(&hWindow->stCurResource.hTntd);
            hWindow->stNewResource.hTntd = NULL;
            pCurDirty->stBits.bTntd = BVDC_P_CLEAN;
        }
#endif

#if BVDC_P_SUPPORT_VFC
        if((hWindow->stCurResource.hVfc) &&
           (pCurDirty->stBits.bDestroy))
        {
            BDBG_MSG(("window %d releases shared VFC", hWindow->eId));
            BVDC_P_Vfc_ReleaseConnect_isr(&hWindow->stCurResource.hVfc);
            hWindow->stNewResource.hVfc = NULL;
        }
#endif

#if BVDC_P_SUPPORT_XSRC
        /* If XSRC is used because of MAD non-4k bypass capable, then
           XSCE can be removed when MAD is no longer in the path */
        if((hWindow->stCurResource.hXsrc) &&
           (pCurDirty->stBits.bDestroy ||
            ((hWindow->stCurResource.hMcvp) &&
             (!hWindow->stCurInfo.bDeinterlace &&
              !hWindow->stNewInfo.bDeinterlace &&
              !hWindow->stCurInfo.bAnr &&
              !hWindow->stNewInfo.bAnr))))
        {
            BDBG_MSG(("window %d releases shared XSRC", hWindow->eId));
            BVDC_P_Xsrc_ReleaseConnect_isr(&hWindow->stCurResource.hXsrc);
            hWindow->stNewResource.hXsrc = NULL;
        }
#endif

        if((hWindow->stCurResource.hMcvp) &&
           (pCurDirty->stBits.bDestroy ||
            (!hWindow->stCurInfo.bDeinterlace &&
             !hWindow->stNewInfo.bDeinterlace &&
             !hWindow->stCurInfo.bAnr &&
             !hWindow->stNewInfo.bAnr)))
        {
            BDBG_MSG(("window %d releases shared MCVP", hWindow->eId));
            BVDC_P_Mcvp_ReleaseConnect_isr(&hWindow->stCurResource.hMcvp);
            hWindow->stNewResource.hMcvp = NULL;
        }

        /* The window shutdown is completed for SHUTDOWN::DESTROY. */
        if(pCurDirty->stBits.bDestroy)
        {
            BVDC_P_Source_DisconnectWindow_isr(pCurInfo->hSource, hWindow);
            hWindow->eState   = BVDC_P_State_eInactive;
            hWindow->pMainCfc = NULL;
            BVDC_P_CLEAN_ALL_DIRTY(pCurDirty);
            BDBG_MSG(("(4) Window[%d] is BVDC_P_State_eInactive", hWindow->eId));

            /* */
            BDBG_ASSERT(hWindow->bSetDestroyEventPending);
            hWindow->bSetDestroyEventPending = false;
            BDBG_MSG(("(5) Window[%d] is set destroy done event", hWindow->eId));
            BKNI_SetEvent_isr(hWindow->hDestroyDoneEvent);
        }
    }

    BDBG_LEAVE(BVDC_P_Window_ProcPostShutDown_isr);
    return;
}
#else /* #ifndef BVDC_FOR_BOOTUPDATER */
static void BVDC_P_Window_ProcPostShutDown_isr
( BVDC_Window_Handle               hWindow )
{
    BSTD_UNUSED(hWindow);
}
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

/***************************************************************************
 * {private}
 *
 * Video Window:
 *   Shutdown -> Inactive -> Set destroy done event
 *
 * Gfx Window
 *   ShutdownPending -> Shutdown -> Inactive -> Set destroy done event
 */
void BVDC_P_Window_UpdateState_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_Window_DirtyBits *pCurDirty;
    BDBG_ENTER(BVDC_P_Window_UpdateState_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    pCurDirty = &hWindow->stCurInfo.stDirty;

    /* ProcPostShutDown will check whether we just completed vnet shutdown */
    BVDC_P_Window_ProcPostShutDown_isr(hWindow);

    /* handle done event if window is not mpeg on compositor that drives its
       source (synclocked or identical timing).  Such windows are handled earlier
       in BVDC_Source_MpegDataReady_isr. */
    if (!(hWindow->stCurInfo.hSource) ||
        !(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId)) ||
        ((hWindow->hCompositor->hSyncLockSrc != hWindow->stCurInfo.hSource) &&
         (hWindow->hCompositor->hForceTrigPipSrc != hWindow->stCurInfo.hSource)))
    {
        /* (1) apply done event. */
        if((hWindow->bSetAppliedEventPending) &&
           ((BVDC_P_IS_CLEAN(pCurDirty)) ||
            ((pCurDirty->stBits.bSrcPending || pCurDirty->stBits.bBufferPending) &&
            !pCurDirty->stBits.bShutdown)))
        {
            hWindow->bSetAppliedEventPending = false;
            BDBG_MSG(("Window[%d] set apply done event", hWindow->eId));
            BKNI_SetEvent_isr(hWindow->hAppliedDoneEvent);
        }
    }

    BDBG_LEAVE(BVDC_P_Window_UpdateState_isr);
    return;
}

/***************************************************************************
 * {private}
 */
void BVDC_P_Window_GetNewRectangles_isr
    ( const BVDC_Window_Handle         hWindow,
      const BVDC_P_ClipRect **         ppClipRect,
      const BVDC_P_Rect **             ppSclOutRect,
      const BVDC_P_Rect **             ppDstRect )
{
    BDBG_ENTER(BVDC_P_Window_GetNewRectangles);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(ppClipRect);
    BDBG_ASSERT(ppSclOutRect);
    BDBG_ASSERT(ppDstRect);

    /* Returns the new infos for gfx to validate. */
    *ppClipRect = &hWindow->stNewInfo.stSrcClip;
    *ppSclOutRect = &hWindow->stNewInfo.stScalerOutput;
    *ppDstRect = &hWindow->stNewInfo.stDstRect;

    BDBG_LEAVE(BVDC_P_Window_GetNewRectangles);
    return;
}

/***************************************************************************
 * {private}
 */
void BVDC_P_Window_GetNewRectangles
    ( const BVDC_Window_Handle         hWindow,
      const BVDC_P_ClipRect **         ppClipRect,
      const BVDC_P_Rect **             ppSclOutRect,
      const BVDC_P_Rect **             ppDstRect )
{
    BDBG_ENTER(BVDC_P_Window_GetNewRectangles);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(ppClipRect);
    BDBG_ASSERT(ppSclOutRect);
    BDBG_ASSERT(ppDstRect);

    BKNI_EnterCriticalSection();
    BVDC_P_Window_GetNewRectangles_isr(hWindow, ppClipRect, ppSclOutRect, ppDstRect);
    BKNI_LeaveCriticalSection();

    BDBG_LEAVE(BVDC_P_Window_GetNewRectangles);
    return;
}

/***************************************************************************
 * {private}
 */
void BVDC_P_Window_GetCurrentRectangles_isr
    ( const BVDC_Window_Handle         hWindow,
      const BVDC_P_ClipRect **         ppClipRect,
      const BVDC_P_Rect **             ppSclOutRect,
      const BVDC_P_Rect **             ppDstRect )
{
    BDBG_ENTER(BVDC_P_Window_GetCurrentRectangles_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(ppClipRect);
    BDBG_ASSERT(ppSclOutRect);
    BDBG_ASSERT(ppDstRect);

    /* Returns the new infos for gfx to validate. */
    *ppClipRect = &hWindow->stCurInfo.stSrcClip;
    *ppSclOutRect = &hWindow->stCurInfo.stScalerOutput;
    *ppDstRect = &hWindow->stCurInfo.stDstRect;

    BDBG_LEAVE(BVDC_P_Window_GetCurrentRectangles_isr);
    return;
}

/***************************************************************************
 * {private}
 */
void BVDC_P_Window_GetNewWindowAlpha
    ( const BVDC_Window_Handle         hWindow,
      uint8_t                         *pucWindowAlpha )
{
    const BVDC_P_Window_Info *pUserInfo;

    BDBG_ENTER(BVDC_P_Window_GetNewWindowAlpha);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Returns the new infos for gfx to validate. */
    pUserInfo = &hWindow->stNewInfo;

    if(pucWindowAlpha)
    {
        *pucWindowAlpha = pUserInfo->ucAlpha;
    }

    BDBG_LEAVE(BVDC_P_Window_GetNewWindowAlpha);
    return;
}

/***************************************************************************
 * {private}
 */
void BVDC_P_Window_GetNewDispOrientation
    ( const BVDC_Window_Handle         hWindow,
      BFMT_Orientation                 *pOrientation)
{
    BVDC_Display_Handle   hDisplay;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(pOrientation);

    /* update display orientation */
    hDisplay = hWindow->hCompositor->hDisplay;
    if(BFMT_IS_3D_MODE(hDisplay->stNewInfo.pFmtInfo->eVideoFmt))
        *pOrientation = hDisplay->stNewInfo.pFmtInfo->eOrientation;
    else
        *pOrientation = hDisplay->stNewInfo.eOrientation;
}

/***************************************************************************
 * {private}
 */
void BVDC_P_Window_GetNewBlendFactor
    ( const BVDC_Window_Handle         hWindow,
      BVDC_BlendFactor                *peFrontBlendFactor,
      BVDC_BlendFactor                *peBackBlendFactor,
      uint8_t                         *pucConstantAlpha )
{
    const BVDC_P_Window_Info *pUserInfo;

    BDBG_ENTER(BVDC_P_Window_GetNewBlendFactor);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Returns the new infos for gfx to validate. */
    pUserInfo = &hWindow->stNewInfo;

    if(peFrontBlendFactor)
    {
        *peFrontBlendFactor = pUserInfo->eFrontBlendFactor;
    }
    if(peBackBlendFactor)
    {
        *peBackBlendFactor = pUserInfo->eBackBlendFactor;
    }
    if(pucConstantAlpha)
    {
        *pucConstantAlpha = pUserInfo->ucConstantAlpha;
    }

    BDBG_LEAVE(BVDC_P_Window_GetNewBlendFactor);
    return;
}

/***************************************************************************
 * {private}
 */
void BVDC_P_Window_GetNewScanType
    ( const BVDC_Window_Handle         hWindow,
      bool                            *pbInterlaced )
{
    BDBG_ENTER(BVDC_P_Window_GetNewScanType_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    if(pbInterlaced)
    {
        /* Get the new validated format from BVDC_P_Display! */
        *pbInterlaced = hWindow->hCompositor->stNewInfo.pFmtInfo->bInterlaced;
    }

    BDBG_LEAVE(BVDC_P_Window_GetNewScanType_isr);
    return;
}

#ifndef BVDC_FOR_BOOTUPDATER
/***************************************************************************
 * {private}
 *
 */
void BVDC_P_Window_UpdateUserState_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_PictureNode *pBufferFromUser = NULL;

    BDBG_ENTER(BVDC_P_Window_UpdateUserState_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    /* Use the new applied states, and make it as current. */
    if(hWindow->bUserAppliedChanges)
    {
        uint32_t bCurShutdown;
        BVDC_P_Window_DirtyBits *pNewDirty, *pCurDirty;
        BVDC_P_State eCurReaderState = hWindow->stCurInfo.eReaderState;
        BVDC_P_State eCurWriterState = hWindow->stCurInfo.eWriterState;

        pCurDirty = &hWindow->stCurInfo.stDirty;
        pNewDirty = &hWindow->stNewInfo.stDirty;
        bCurShutdown = pCurDirty->stBits.bShutdown;

        /* Copying the new info to the current info.  Must be careful here
         * of not globble current dirty bits set by source, but rather ORed
         * them together. */
        BVDC_P_OR_ALL_DIRTY(pNewDirty, pCurDirty);

        if(pCurDirty->stBits.bUserReleaseBuffer)
        {
            pBufferFromUser = hWindow->stCurInfo.pBufferFromUser;
        }

        /* copy stNewInfo to stCurInfo here !!! */
        hWindow->stCurInfo = hWindow->stNewInfo;

#if BVDC_P_SUPPORT_MOSAIC_MODE
    if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId))
    {
        /* Copy z-order information. Note: user must set same z-order value on
                * each mosaic window for the same channel in HD_SD simul mode */
        BKNI_Memcpy((void*)hWindow->stCurInfo.hSource->aulMosaicZOrderIndex, (void*)hWindow->aulMosaicZOrderIndex,
        sizeof(uint32_t) * hWindow->stCurInfo.ulMosaicCount);
    }
#endif
        if(pCurDirty->stBits.bUserReleaseBuffer)
        {
            hWindow->stCurInfo.pBufferFromUser = pBufferFromUser;
        }

        /* be careful: restore the current states if current is still dirty,
         * when new Writer/ReaderState is inactive, that is not set on purpose,
         * it should be copied to current Writer/ReaderSate and cause
         * SetWriterVnet_isr/SetReaderVnet_isr to called */
        if((bCurShutdown == BVDC_P_DIRTY) ||
           (BVDC_P_State_eInactive == hWindow->stNewInfo.eReaderState))
        {
            hWindow->stCurInfo.eReaderState = eCurReaderState;
        }
        if((bCurShutdown == BVDC_P_DIRTY) ||
           (BVDC_P_State_eInactive == hWindow->stNewInfo.eWriterState))
        {
            hWindow->stCurInfo.eWriterState = eCurWriterState;
        }

        /* Clear dirty bits since it's already OR'ed into current.  Notes
         * the it might not apply until next vysnc, so we're defering
         * setting the event until next vsync. */
        BVDC_P_CLEAN_ALL_DIRTY(pNewDirty);
        hWindow->bUserAppliedChanges = false;

        /* can not set them into Active here in case ApplyChanges are called
         * twice before Writer_isr and Reader_isr are called ? */
        hWindow->stNewInfo.eWriterState = BVDC_P_State_eInactive;
        hWindow->stNewInfo.eReaderState = BVDC_P_State_eInactive;
    }

    BDBG_LEAVE(BVDC_P_Window_UpdateUserState_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * Utility func called by BVDC_P_Window_AdjustRectangles_isr to calculate
 * user pan scan disp size, in the case that it is not provided by stream
 *
 * The calculated user disp size is stored in hWindow context
 */
static void BVDC_P_Window_CalcuAutoDisplaySize_isr
    ( BVDC_Window_Handle               hWindow,
  uintAR_t                         ulSrcPxlAspRatio,  /* U4.16 fix pt */
  uintAR_t                         ulDspPxlAspRatio ) /* U4.16 fix pt */
{
    uint32_t ulFullSrcWidth, ulFullSrcHeight;
    uint32_t ulFullDspWidth, ulFullDspHeight;
    uint32_t ulTmpWidth, ulTmpHeight;
    uintAR_t ulFullSrcAspR = 0;
    uintAR_t ulFullDspAspR = 0;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    /* calculate the src aspect ratio -- 4.16 fixed pointer numer */
    ulFullSrcWidth  = hWindow->ulFieldSrcWidth;
    ulFullSrcHeight = hWindow->ulFieldSrcHeight;
    BDBG_ASSERT(ulFullSrcWidth);
    BDBG_ASSERT(ulFullSrcHeight);

    ulFullSrcAspR = BVDC_P_DIV_ROUND_NEAR(
        ulSrcPxlAspRatio * ulFullSrcWidth, ulFullSrcHeight);

    /* calculate the full disp aspect ratio -- 4.16 fixed pointer numer */
    ulFullDspWidth  = hWindow->hCompositor->stCurInfo.pFmtInfo->ulWidth;
    ulFullDspHeight = hWindow->hCompositor->stCurInfo.pFmtInfo->ulHeight;
    BDBG_ASSERT(ulFullDspWidth);
    BDBG_ASSERT(ulFullDspHeight);

    ulFullDspAspR = BVDC_P_DIV_ROUND_NEAR(
        ulDspPxlAspRatio * ulFullDspWidth, ulFullDspHeight);

    /* calcu user pan scan disp size (cut src width or height):
     * see the equation in BVDC_P_AspectRatioCorrection_isrsafe */
    if( ulFullSrcAspR > ulFullDspAspR )
    {
        /* needs cut cw: new cw = cw * dar / sar */
        ulTmpWidth = BVDC_P_DIV_ROUND_NEAR(
            ulFullSrcWidth *  ulFullDspAspR, ulFullSrcAspR);
        hWindow->ulAutoDispHorizontalSize = ulTmpWidth;
        hWindow->ulAutoDispVerticalSize   = ulFullSrcHeight;
        BDBG_MODULE_MSG(BVDC_ASP_RAT,(BDBG_STRING("User pan scan cut W: new width %d"), ulTmpWidth));
#ifndef BVDC_UINT32_ONLY
        BDBG_MODULE_MSG(BVDC_ASP_RAT,(BDBG_STRING("New equation cnt side " BDBG_UINT64_FMT ", disp side " BDBG_UINT64_FMT),
            BDBG_UINT64_ARG((ulTmpWidth * ulFullSrcAspR) / ulFullSrcWidth), BDBG_UINT64_ARG(ulFullDspAspR)));
#else
        BDBG_MODULE_MSG(BVDC_ASP_RAT,(BDBG_STRING("New equation cnt side 0x%Lx, disp side 0x%Lx"),
            (ulTmpWidth * ulFullSrcAspR) / ulFullSrcWidth, ulFullDspAspR));
#endif
    }
    else if( ulFullSrcAspR < ulFullDspAspR )
    {
        /* needs cut ch: new ch = ch * sar / dar */
        ulTmpHeight = BVDC_P_DIV_ROUND_NEAR(
            ulFullSrcHeight * ulFullSrcAspR, ulFullDspAspR);
        hWindow->ulAutoDispHorizontalSize = ulFullSrcWidth;
        hWindow->ulAutoDispVerticalSize   = ulTmpHeight;
        BDBG_MODULE_MSG(BVDC_ASP_RAT,(BDBG_STRING("User pan scan cut H: new height %d"), ulTmpHeight));
#ifndef BVDC_UINT32_ONLY
        BDBG_MODULE_MSG(BVDC_ASP_RAT,(BDBG_STRING("New equation cnt side " BDBG_UINT64_FMT ", disp side " BDBG_UINT64_FMT),
            BDBG_UINT64_ARG((ulFullSrcHeight * ulFullSrcAspR) / ulTmpHeight), BDBG_UINT64_ARG(ulFullDspAspR)));
#else
        BDBG_MODULE_MSG(BVDC_ASP_RAT,(BDBG_STRING("New equation cnt side 0x%Lx, disp side 0x%Lx"),
            (ulFullSrcHeight * ulFullSrcAspR) / ulTmpHeight, ulFullDspAspR));
#endif
    }
    else
    {
        /* no cut is needed */
        hWindow->ulAutoDispHorizontalSize = ulFullSrcWidth;
        hWindow->ulAutoDispVerticalSize   = ulFullSrcHeight;
    }
}

static void BVDC_P_Window_Bar_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_Rect                     *pSrcCnt,            /* interm src cnt, in and out */
      BAVC_BarDataType                 eBarDataType,       /* specify top/bottom or left/right data */
      uint32_t                         ulTopLeftBarValue,  /* either the top or left bar data value */
      uint32_t                         ulBotRightBarValue) /* either the bottom or right bar data value */
{
    uint32_t ulTotalSrcCut, ulSrcLength;

    BDBG_ENTER(BVDC_P_Window_Bar_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(pSrcCnt);

    ulTotalSrcCut = ulTopLeftBarValue + ulBotRightBarValue;
    ulSrcLength = (BAVC_BarDataType_eTopBottom==eBarDataType)?
        pSrcCnt->ulHeight: pSrcCnt->ulWidth;

    /* Validate for valid Bar value */
    if(ulSrcLength <= ulTotalSrcCut)
    {
        BDBG_ERR(("Invalid Bar Data value: BarDataType %s %d %d",
            (BAVC_BarDataType_eTopBottom==eBarDataType)?"TopBottom":"LeftRight",
            ulTopLeftBarValue, ulBotRightBarValue));
        return;
    }

    if(BAVC_BarDataType_eTopBottom==eBarDataType)
    {
        pSrcCnt->lTop = ulTopLeftBarValue<< BVDC_P_16TH_PIXEL_SHIFT;
        pSrcCnt->lLeft= 0;
        pSrcCnt->ulHeight -= ulTotalSrcCut;
    }
    else
    {
        pSrcCnt->lTop  = 0 ;
        pSrcCnt->lLeft = ulTopLeftBarValue<< BVDC_P_16TH_PIXEL_SHIFT;
        pSrcCnt->ulWidth -= ulTotalSrcCut;
    }


    pSrcCnt->lLeft_R = pSrcCnt->lLeft;

    BDBG_LEAVE(BVDC_P_Window_Bar_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * This function clip the source based on AFD. When AFD is used, box
 * auto cut  and pan scan are ignored, and AFD is the first clipping
 * applied to src.  It only affect hWindow->stSrcCnt.
 *
 * It is only used for mpeg src. It is called at every vsync when
 * RUL is built, by BVDC_P_Window_AdjustRectangles_isr.
 */
static const BVDC_P_Afd s_a4x3AfdInfo[] =
{
                 /*  a  h0    v0    h1    v1    h2    v2    comment */
    BVDC_P_MAKE_AFD( 0, 0,    0,    0,    0,    0,    0,    "No AFD Specified No AFD Specified"),
    BVDC_P_MAKE_AFD( 1, 0,    0,    0,    0,    0,    0,    "Reserved Reserved"),
    BVDC_P_MAKE_AFD( 2, 0,    2500, 0,    2500, 0,    2500, "16x9 letterbox top"),
    BVDC_P_MAKE_AFD( 3, 0,    1250, 0,    1250, 0,    1250, "14x9 letterbox top"),

    BVDC_P_MAKE_AFD( 4, 0,    1250, 0,    1250, 0,    1250, "Greater than 16x9 letterbox image"),
    BVDC_P_MAKE_AFD( 5, 0,    0,    0,    0,    0,    0,    "Reserved Reserved"),
    BVDC_P_MAKE_AFD( 6, 0,    0,    0,    0,    0,    0,    "Reserved Reserved"),
    BVDC_P_MAKE_AFD( 7, 0,    0,    0,    0,    0,    0,    "Reserved Reserved"),

    BVDC_P_MAKE_AFD( 8, 0,    0,    0,    0,    0,    0,    "4x3 full frame image"),
    BVDC_P_MAKE_AFD( 9, 0,    0,    0,    0,    0,    0,    "4x3 full frame image"),
    BVDC_P_MAKE_AFD(10, 0,    1250, 0,    1250, 0,    1250, "16x9 letterbox image"),
    BVDC_P_MAKE_AFD(11, 0,    625,  0,    625,  0,    625,  "14x9 letterbox image"),

    BVDC_P_MAKE_AFD(12, 0,    0,    0,    0,    0,    0,    "Reserved Reserved"),
    BVDC_P_MAKE_AFD(13, 0,    0,    0,    625,  0,    625,  "4x3 full frame image, alt 14x9 center"),
    BVDC_P_MAKE_AFD(14, 0,    1250, 625,  1250, 625,  1250, "16x9 letterbox image, alt 14x9 center"),
    BVDC_P_MAKE_AFD(15, 0,    1250, 625,  1250, 1250, 1250, "16x9 letterbox image, alt 4x3 center ")
};

static const BVDC_P_Afd s_a16x9AfdInfo[] =
{
                 /*  a  h0    v0   h1    v1   h2    v2   comment */
    BVDC_P_MAKE_AFD( 0, 0,    0,   0,    0,   0,    0,   "No AFD Specified No AFD Specified"),
    BVDC_P_MAKE_AFD( 1, 0,    0,   0,    0,   0,    0,   "Reserved Reserved"),
    BVDC_P_MAKE_AFD( 2, 0,    0,   0,    0,   0,    0,   "16x9 letterbox top"),
    BVDC_P_MAKE_AFD( 3, 625,  0,   625,  0,   625,  0,   "14x9 letterbox top"),

    BVDC_P_MAKE_AFD( 4, 0,    625, 0,    625, 0,    625, "Greater than 16x9 letterbox image"),
    BVDC_P_MAKE_AFD( 5, 0,    0,   0,    0,   0,    0,   "Reserved Reserved"),
    BVDC_P_MAKE_AFD( 6, 0,    0,   0,    0,   0,    0,   "Reserved Reserved"),
    BVDC_P_MAKE_AFD( 7, 0,    0,   0,    0,   0,    0,   "Reserved Reserved"),

    BVDC_P_MAKE_AFD( 8, 0,    0,   0,    0,   0,    0,   "4x3 full frame image"),
    BVDC_P_MAKE_AFD( 9, 1250, 0,   1250, 0,   1250, 0,   "4x3 full frame image"),
    BVDC_P_MAKE_AFD(10, 0,    0,   0,    0,   0,    0,   "16x9 letterbox image"),
    BVDC_P_MAKE_AFD(11, 625,  0,   625,  0,   625,  0,   "14x9 letterbox image"),

    BVDC_P_MAKE_AFD(12, 0,    0,   0,    0,   0,    0,   "Reserved Reserved"),
    BVDC_P_MAKE_AFD(13, 1250, 0,   1250, 625, 1250, 625, "4x3 full frame image, alt 14x9 center"),
    BVDC_P_MAKE_AFD(14, 0,    0,   625,  0,   625,  0,   "16x9 letterbox image, alt 14x9 center"),
    BVDC_P_MAKE_AFD(15, 0,    0,   625,  0,   1250, 0,   "16x9 letterbox image, alt 4x3 center ")
};

static void BVDC_P_Window_Afd_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_Rect                     *pSrcCnt,          /* interm src cnt, in and out */
      uint32_t                         ulAfdVal,         /* AFD value */
      BFMT_AspectRatio                 eSrcAspectRatio,  /* aspect ratio of coded frame */
      uint32_t                         ulSampleAspectRatioX, /* width of one sampled src pixel */
      uint32_t                         ulSampleAspectRatioY, /* height of one sampled src pixel */
      uint32_t                         ulSrcWidth,          /* source width */
      uint32_t                         ulSrcHeight )        /* source height */

{
    uint32_t ulHClip, ulVClip;
    const BVDC_P_Afd *pAfd;
    BFMT_AspectRatio  eAR;

    BDBG_ENTER(BVDC_P_Window_Afd_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(pSrcCnt);

    if(eSrcAspectRatio == BFMT_AspectRatio_eSAR)
    {
        uint32_t ulDAR = (ulSrcWidth * ulSampleAspectRatioX * 100) / (ulSrcHeight * ulSampleAspectRatioY);
        eAR = BVDC_P_EQ_DELTA(ulDAR, 133, 25)
            ? BFMT_AspectRatio_e4_3 :
              (BVDC_P_EQ_DELTA(ulDAR, 177, 25) ? BFMT_AspectRatio_e16_9 : BFMT_AspectRatio_eUnknown);
        BDBG_MSG(("SrcW = %d, SrcH = %d, SarX = %d, SarY = %d, ulDAR = %d, AR = %d",
            ulSrcWidth, ulSrcHeight, ulSampleAspectRatioX, ulSampleAspectRatioY, ulDAR, eAR));
    }
    else
    {
        eAR = eSrcAspectRatio;
    }

    /* only support 4x3 and 16x9 source aspect ration for now */
    if(eAR != BFMT_AspectRatio_e4_3 && eAR != BFMT_AspectRatio_e16_9)
    {
        BDBG_WRN(("AFD is not supported for source aspect ratio %d", eAR));
        return;
    }

    /* Check for valid AFD value */
    if(ulAfdVal >= (sizeof(s_a4x3AfdInfo) / sizeof(BVDC_P_Afd)) ||
       ulAfdVal >= (sizeof(s_a16x9AfdInfo) / sizeof(BVDC_P_Afd)))
    {
        BDBG_ERR(("Invalid AFD value"));
        return;
    }

    pAfd = (eAR == BFMT_AspectRatio_e4_3) ? &(s_a4x3AfdInfo[ulAfdVal]) : &(s_a16x9AfdInfo[ulAfdVal]);
    ulHClip = pAfd->aulHClip[hWindow->stCurInfo.stAfdSettings.eClip];
    ulVClip = pAfd->aulVClip[hWindow->stCurInfo.stAfdSettings.eClip];

    if(eAR == BFMT_AspectRatio_e4_3 && (ulAfdVal == 2 || ulAfdVal == 3))
    {
        pSrcCnt->lTop = 0;
        pSrcCnt->ulHeight = pSrcCnt->ulHeight - (pSrcCnt->ulHeight * ulVClip / 10000);
        pSrcCnt->lLeft = (pSrcCnt->ulWidth * ulHClip / 10000) << BVDC_P_16TH_PIXEL_SHIFT;
        pSrcCnt->ulWidth = pSrcCnt->ulWidth - (pSrcCnt->ulWidth * ulHClip * 2 / 10000);
    }
    else
    {
        pSrcCnt->lTop = (pSrcCnt->ulHeight * ulVClip / 10000) << BVDC_P_16TH_PIXEL_SHIFT;
        pSrcCnt->ulHeight = pSrcCnt->ulHeight - (pSrcCnt->ulHeight * ulVClip * 2 / 10000);
        pSrcCnt->lLeft = (pSrcCnt->ulWidth * ulHClip / 10000) << BVDC_P_16TH_PIXEL_SHIFT;
        pSrcCnt->ulWidth = pSrcCnt->ulWidth - (pSrcCnt->ulWidth * ulHClip * 2 / 10000);
    }

    pSrcCnt->lLeft_R = pSrcCnt->lLeft;

    BDBG_LEAVE(BVDC_P_Window_Afd_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * This function clip the src based on pan-scan. When pan scan is used, box
 * auto cut is ignored, and pan-scan is the first clipping applied to src.
 * It only affect hWindow->stSrcCnt.
 *
 * It is only used for mpeg and hddvi src. It is called at every vsync when
 * RUL is built, by BVDC_P_Window_AdjustRectangles_isr.
 */
static void BVDC_P_Window_PanScan_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_Rect                     *pSrcCnt,  /* interm src cnt, in and out */
      uint32_t                         ulSrcDispWidth,    /* pan disp width */
      uint32_t                         ulSrcDispHeight,   /* pan disp width */
      int32_t                          lHorizontalPanScan,/* stream pan x, 11.4 */
      int32_t                          lVerticalPanScan ) /* stream pan y, 11.4 */
{
    const BVDC_P_Window_Info *pUserInfo;
    int32_t lZeroHorzPanScan, lZeroVertPanScan;
    uint32_t ul16DispWidth, ul16DispHeight; /* src pan disp size, 11.4.fixed ptr */
    uint32_t ul16SrcWidth, ul16SrcHeight;   /* src full size, 11.4.fixed ptr */
    int32_t lCntXMin; /* src x_min, 11.4 fixed ptr */
    int32_t lCntXMax; /* src x_max, 11.4.fixed ptr */
    int32_t lCntYMin; /* src y_min, 11.4.fixed ptr */
    int32_t lCntYMax; /* src y_max, 11.4.fixed ptr */

    BDBG_ENTER(BVDC_P_Window_PanScan_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(pSrcCnt);
    pUserInfo = &hWindow->stCurInfo;

    /* When pan scan is used, box auto cut is ignored, and pan-scan is the
     * first clipping apllied to src. */
    ul16SrcWidth  = pSrcCnt->ulWidth  << BVDC_P_16TH_PIXEL_SHIFT; /* 11.4 fixed ptr */
    ul16SrcHeight = pSrcCnt->ulHeight << BVDC_P_16TH_PIXEL_SHIFT; /* 11.4 fixed ptr */
    ul16DispWidth  = ulSrcDispWidth  << BVDC_P_16TH_PIXEL_SHIFT; /* 11.4 fixed ptr */
    ul16DispHeight = ulSrcDispHeight << BVDC_P_16TH_PIXEL_SHIFT; /* 11.4 fixed ptr */

    /* when srcDisp size > src, it does not mean to further cut src, but adjust
     * the position of display window, hence we don't need to do anything, and let
     * user to set scl-out and dest window in _isr
     * TODO: auto adjust scl-out and dest rect in VDC */
    if((ul16DispWidth > ul16SrcWidth) || (ul16DispHeight > ul16SrcHeight))
    {
        return;
    }

    /* Zero panscan vector specifies the position of the top left corner of the
     * display rectangle from the top left corner of the decompressed frame.
     * Horizontal pan scan = float(dWidth/2)
     * Integer part i = dWidth >> 1
     * Fraction part f = dWidth % 2
     * To convert it = (i << 1 ) + (f)
     * = ((dWidth >> 1) << 1) + (dWidth % 2) = dWidth */
    /* Zero pan scan in units of 1/16 pixel and signed two's complement format; */
    lZeroHorzPanScan = ((int32_t) ul16SrcWidth  - (int32_t) ul16DispWidth)  >> 1;
    lZeroVertPanScan = ((int32_t) ul16SrcHeight - (int32_t) ul16DispHeight) >> 1;
    /* BDBG_MODULE_MSG(BVDC_ASP_RAT,("PANSCAN is enabled: type = %d", pUserInfo->ePanScanType)); */

    if(pUserInfo->ePanScanType == BVDC_PanScanType_eStream)
    {
        /* Use zero pan scan plus stream pan scan;
         * Stream pan scan in units of 1/16th of a pixel in two's complement.
         * Bit 15 is the sign bit. Bits 14:8 are the macro block grid.
         * Bits 7:4 are the pixel grid. Bits 3:0 are 1/16 pixel grid.
         * calculate in two's complement; remember stream panscan vector
         * specifies the position of the centre of the reconstructed frame
         * from the centre of the display rectangle.
         * ResultVector := ZeroVector - StreamVector */
        lCntXMin = (lZeroHorzPanScan - lHorizontalPanScan);
        lCntYMin = (lZeroVertPanScan - lVerticalPanScan);
    }
    else if(pUserInfo->ePanScanType == BVDC_PanScanType_eUser)
    {
        /* Get user pan scan in units of 1/16 pixel and two's complement format;
         * assume user panscan vector follows the same convention of mpeg stream.
         * ResultVector := ZeroVector - UserVector */
        lCntXMin = (lZeroHorzPanScan - (pUserInfo->lUserHorzPanScan << BVDC_P_16TH_PIXEL_SHIFT));
        lCntYMin = (lZeroVertPanScan - (pUserInfo->lUserVertPanScan << BVDC_P_16TH_PIXEL_SHIFT));
    }
    else
    {
        /* This must be reserved.  For BVDC_PanScanType_eStreamAndUser mode, the
         * vector is actually:
         * ResultVector := ZeroVector - StreamVector - UserVector */
        BDBG_ASSERT(BVDC_PanScanType_eStreamAndUser == pUserInfo->ePanScanType);

        lCntXMin = (lZeroHorzPanScan - lHorizontalPanScan -
                    (pUserInfo->lUserHorzPanScan << BVDC_P_16TH_PIXEL_SHIFT));
        lCntYMin = (lZeroVertPanScan - lVerticalPanScan -
                    (pUserInfo->lUserVertPanScan << BVDC_P_16TH_PIXEL_SHIFT));
    }

    /* convert to U27.4 fixed point;
     * Note: it's always non-negative(bound by source boundary left/top). */
    lCntXMin = (lCntXMin > 0)? lCntXMin : 0;
    lCntYMin = (lCntYMin > 0)? lCntYMin : 0;

    /* the width/height comes from source display size */
    lCntXMax = lCntXMin + (int32_t) ul16DispWidth;
    lCntYMax = lCntYMin + (int32_t) ul16DispHeight;

    /* Make sure the window source content is bound by source
     * boundary(right/bottom). */
    if(lCntXMax > (int32_t)(ul16SrcWidth))
    {
        lCntXMax = ul16SrcWidth;
        lCntXMin = lCntXMax - (int32_t) ul16DispWidth;
    }

    if(lCntYMax > (int32_t)(ul16SrcHeight))
    {
        lCntYMax = ul16SrcHeight;
        lCntYMin = lCntYMax - (int32_t) ul16DispHeight;
    }

    /* store result to hWindow->stSrcCnt */
    pSrcCnt->lLeft    = lCntXMin;
    pSrcCnt->lTop     = lCntYMin;
    pSrcCnt->lLeft_R  = lCntXMin;
    pSrcCnt->ulWidth  = (lCntXMax - lCntXMin) >> BVDC_P_16TH_PIXEL_SHIFT;
    pSrcCnt->ulHeight = (lCntYMax - lCntYMin) >> BVDC_P_16TH_PIXEL_SHIFT;

    BDBG_LEAVE(BVDC_P_Window_PanScan_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 * This function clip the src based on box black patch auto cut. If pan
 * scan is used, box auto cut is ignored. However, this function might
 * still be called to report box detect info to user through box detetct call
 * back func. When box auto cut is used, it is the first clipping applied to
 * src. It only affect hWindow->stSrcCnt.
 *
 * It is called only for video window. It is called at every vsync when RUL
 * is built, by BVDC_P_Window_AdjustRectangles_isr.
 */
static void BVDC_P_Window_BoxCut_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_Rect                     *pSrcCnt,  /* interm src cnt, in and out */
      bool                             bDoPanScan )
{
    BVDC_P_Rect stBoxCut;
    uint32_t  ulSrcWidth, ulSrcHeight; /* full src size */
    const BVDC_BoxDetectInfo  *pBoxInfo;
    uint32_t  ulCallBackCntr;

    BDBG_ENTER(BVDC_P_Window_BoxCut_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(pSrcCnt);

    /* no clipping has applied to src yet */
    ulSrcWidth  = pSrcCnt->ulWidth;
    ulSrcHeight = pSrcCnt->ulHeight;

    /* stBoxCut used for in and out by BVDC_P_BoxDetect_Do_isr. */
    stBoxCut.lTop = 0;
    stBoxCut.lLeft_R = 0;
    stBoxCut.lLeft = 0;
    stBoxCut.ulWidth = ulSrcWidth;
    stBoxCut.ulHeight = ulSrcHeight;

    /* Check hw reg, and decide cutting.
     * Note: When pan-scan is enabled, auto-box-cut is ignored. However,
     * even if in this case. this func should still be called because
     * of BoxDetectCallBack calling might still be needed */
    BVDC_P_BoxDetect_GetStatis_isr(hWindow->stCurResource.hBoxDetect,
        &stBoxCut, &pBoxInfo, &ulCallBackCntr);

    if((NULL != hWindow->stCurInfo.BoxDetectCallBack) &&
        (NULL != pBoxInfo) &&
        (ulCallBackCntr != hWindow->ulBoxDetectCallBckCntr))
    {
        (*hWindow->stCurInfo.BoxDetectCallBack) (
            hWindow->stCurInfo.pvBoxDetectParm1,
            hWindow->stCurInfo.iBoxDetectParm2, pBoxInfo);
        hWindow->ulBoxDetectCallBckCntr = ulCallBackCntr;
    }

    /* when pan scan is enabled, box auto cut is ignored */
    if((! bDoPanScan) && (hWindow->stCurInfo.bAutoCutBlack))
    {
        /* apply the box auto cut to hWindow->stSrcCnt. note that with the 1st
         * BVDC_P_BOX_DETECT_NUM_ACCUM vsync stBoxCut.ulWidth/ulHeiight are
         * BVDC_P_BOX_DETECT_MAX_EDGE */
        pSrcCnt->lLeft = stBoxCut.lLeft << BVDC_P_16TH_PIXEL_SHIFT;
        pSrcCnt->lLeft_R = stBoxCut.lLeft_R << BVDC_P_16TH_PIXEL_SHIFT;
        pSrcCnt->lTop  = stBoxCut.lTop  << BVDC_P_16TH_PIXEL_SHIFT;
        pSrcCnt->ulWidth  = stBoxCut.ulWidth;
        pSrcCnt->ulHeight = stBoxCut.ulHeight;

        BDBG_ASSERT(stBoxCut.lLeft >= 0);
        BDBG_ASSERT(stBoxCut.lLeft_R >= 0);
        BDBG_ASSERT(stBoxCut.lTop  >= 0);
        BDBG_ASSERT(stBoxCut.lLeft + pSrcCnt->ulWidth  <= ulSrcWidth);
        BDBG_ASSERT(stBoxCut.lTop  + pSrcCnt->ulHeight <= ulSrcHeight);
    }

    BDBG_LEAVE(BVDC_P_Window_BoxCut_isr);
}


/***************************************************************************
 * {private}
 *
 * This function applys the user set src clip (including the setting by _isr)
 * to hWindow->stSrcCnt, which is used to set picture nodes. This func is
 * called after box auto cut and pan-scan. This func only affect
 * hWindow->stSrcCnt.
 *
 * If the src is mpeg, hddvi, or if letter box auto back cut is enabled, it is
 * called at every vsync when RUL is built. Otherwise, it is called only once
 * at the first vsync after ApplyChanges.
 *
 * It is only used by BVDC_P_Window_AdjustRectangles_isr
 */
static void BVDC_P_Window_UserSrcCut_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_Rect                     *pSrcCnt,
      const BVDC_P_ClipRect           *pSrcClp )
{
    BDBG_ENTER(BVDC_P_Window_UserSrcCut_isr);
    BDBG_ASSERT(pSrcCnt);
    BSTD_UNUSED(hWindow);

    /* Source clip, and ignore over clipping */
    if(((pSrcClp->ulLeft + pSrcClp->ulRight)  < pSrcCnt->ulWidth) &&
       ((pSrcClp->ulTop  + pSrcClp->ulBottom) < pSrcCnt->ulHeight))
    {
        pSrcCnt->lLeft    += (pSrcClp->ulLeft << BVDC_P_16TH_PIXEL_SHIFT);
        pSrcCnt->ulWidth  -= (pSrcClp->ulLeft + pSrcClp->ulRight);
        pSrcCnt->lTop     += (pSrcClp->ulTop  << BVDC_P_16TH_PIXEL_SHIFT);
        pSrcCnt->ulHeight -= (pSrcClp->ulTop  + pSrcClp->ulBottom);

#if (BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP)
        pSrcCnt->lLeft_R = pSrcCnt->lLeft +
            (pSrcClp->lLeftDelta_R << BVDC_P_16TH_PIXEL_SHIFT);
#else
        pSrcCnt->lLeft_R = pSrcCnt->lLeft;
#endif
    }

    BDBG_LEAVE(BVDC_P_Window_UserSrcCut_isr);
    return;
}

/* Those THD values are set to 0 to work around PEP's TNT(sharp) problem with
 * CMP_0_V0_SURFACE_SIZE > 1920. 8 should be a good number if the TNT problem is gone.
 */
/* Smooth scaling is enabled see boxmode . Need to use smaller
 * THD values, otherwise VFD don't have enough bandwidth.
 */
#define BVDC_P_DST_CUT_TOP_THRESH      (0)
#define BVDC_P_DST_CUT_BOTTOM_THRESH   (0)
#define BVDC_P_DST_CUT_LEFT_THRESH     (0)
#define BVDC_P_DST_CUT_RIGHT_THRESH    (0)

/***************************************************************************
 * {private}
 *
 * It is only used by BVDC_P_Window_AdjustRectangles_isr
 */
static void BVDC_P_Window_AdjDstCut_isr
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulMaxWidth,
      uint32_t                         ulMaxHeight,
      BVDC_P_Rect                     *pSrcCnt,
      BVDC_P_Rect                     *pAdjSclOut,
      const BVDC_P_Rect               *pAdjDstRect )
{
    uint32_t  ulOldSclOutRightCut, ulOldSclOutBottomCut;
    uint32_t  ulNewSclOutLeft, ulNewSclOutWidth, ulNewSclOutLeft_R;
    uint32_t  ulNewSclOutTop, ulNewSclOutHeight;
    uint32_t  ulOffCntLeft, ulOffCntTop, ulOffCntLeft_R;
    uint32_t  ulBottomThresh, ulRightThresh;
    int32_t   lTopThresh, lLeftThresh;
    uint32_t  ulNewSrcWidth, ulNewSrcHeight;

    BDBG_ENTER(BVDC_P_Window_AdjDstCut_isr);
    BDBG_ASSERT(pSrcCnt);

    /* If SclOut is larger than canvas, transfer to src cut, otherwise */
    /* allow dst cut */
    if(pAdjSclOut->ulWidth  > ulMaxWidth ||
       pAdjSclOut->ulHeight > ulMaxHeight)
    {
        lTopThresh     = 0;
        ulBottomThresh = 0;
        lLeftThresh    = 0;
        ulRightThresh  = 0;
    }
    else
    {
        lTopThresh     = BVDC_P_DST_CUT_TOP_THRESH;
        ulBottomThresh = BVDC_P_DST_CUT_BOTTOM_THRESH;
        lLeftThresh    = BVDC_P_DST_CUT_LEFT_THRESH;
        ulRightThresh  = BVDC_P_DST_CUT_RIGHT_THRESH;
    }

    /* Transferfing large dest clip to src clip.  Currently when destination cut
     * (specially from top) is relative large the system could run into bandwidth
     * issues.  Here we're transfering the cut to source, so we can cut early in
     * the process to save bandwidth. */
    ulOldSclOutRightCut =
        pAdjSclOut->ulWidth - pAdjSclOut->lLeft - pAdjDstRect->ulWidth;
    if((lLeftThresh < pAdjSclOut->lLeft) || (lLeftThresh < pAdjSclOut->lLeft_R) ||
       (ulRightThresh < ulOldSclOutRightCut))
    {
        /* prepare to xfer hrz dst cut into src cut*/
        ulNewSclOutLeft = (lLeftThresh < pAdjSclOut->lLeft)?
            (uint32_t)lLeftThresh : (uint32_t)pAdjSclOut->lLeft;
        ulNewSclOutLeft_R = (lLeftThresh < pAdjSclOut->lLeft_R)?
            (uint32_t)lLeftThresh : (uint32_t)pAdjSclOut->lLeft_R;

        ulNewSclOutWidth = (ulRightThresh < ulOldSclOutRightCut)?
            (ulNewSclOutLeft + pAdjDstRect->ulWidth + ulRightThresh):
            (ulNewSclOutLeft + pAdjDstRect->ulWidth + ulOldSclOutRightCut);

        /* add src left cut */
        ulOffCntLeft = BVDC_P_DIV_ROUND_NEAR(
            (((pAdjSclOut->lLeft - ulNewSclOutLeft) * pSrcCnt->ulWidth) <<
             BVDC_P_16TH_PIXEL_SHIFT), pAdjSclOut->ulWidth);
        pSrcCnt->lLeft += ulOffCntLeft;

        ulOffCntLeft_R = BVDC_P_DIV_ROUND_NEAR(
            (((pAdjSclOut->lLeft_R - ulNewSclOutLeft_R) * pSrcCnt->ulWidth) <<
             BVDC_P_16TH_PIXEL_SHIFT), pAdjSclOut->ulWidth);
        pSrcCnt->lLeft_R += ulOffCntLeft_R;

        /* add src width cut: set cnt width a little big to ensure no edge content
         * loss, it is ok to do so because phase and SrcStep also control how much
         * edge src contribute */
        ulNewSrcWidth = BVDC_P_DIV_ROUND_UP(
            ulNewSclOutWidth * pSrcCnt->ulWidth, pAdjSclOut->ulWidth);

        /* Make sure not violate HW limiations */
        if(ulNewSrcWidth < BVDC_P_SRC_INPUT_H_MIN)
        {
            int32_t  lLeftAdj;

            /* Adjust clipping */
            lLeftAdj= BVDC_P_SRC_INPUT_H_MIN - ulNewSrcWidth;
            pSrcCnt->lLeft = BVDC_P_MAX(0,
                pSrcCnt->lLeft - (lLeftAdj << BVDC_P_16TH_PIXEL_SHIFT));
            pSrcCnt->lLeft_R = BVDC_P_MAX(0,
                pSrcCnt->lLeft_R - (lLeftAdj << BVDC_P_16TH_PIXEL_SHIFT));
            ulNewSrcWidth = BVDC_P_SRC_INPUT_H_MIN;

            /* Recalculate srcstep */
            hWindow->ulNrmHrzSrcStep = BVDC_P_CAL_HRZ_SRC_STEP(
                ulNewSrcWidth, ulNewSclOutWidth);
        }
        pSrcCnt->ulWidth = ulNewSrcWidth;

        /* Adjust pSclOut since we already move the cut to source. */
        pAdjSclOut->lLeft   = ulNewSclOutLeft;
        pAdjSclOut->lLeft_R = ulNewSclOutLeft_R;
        pAdjSclOut->ulWidth = ulNewSclOutWidth;
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("dstCutXfer: AdjCnt left %d/16(%d), width %d, sclOut left %d, width %d",
            pSrcCnt->lLeft, pSrcCnt->lLeft/16, pSrcCnt->ulWidth, pAdjSclOut->lLeft, pAdjSclOut->ulWidth));
    }

    ulOldSclOutBottomCut =
        pAdjSclOut->ulHeight - pAdjSclOut->lTop - pAdjDstRect->ulHeight;
    if((lTopThresh < pAdjSclOut->lTop) ||
       (ulBottomThresh < ulOldSclOutBottomCut))
    {
        /* prepare to xfer vrt dst cut into src cut*/
        ulNewSclOutTop = (lTopThresh < pAdjSclOut->lTop)?
            ulBottomThresh : (uint32_t)pAdjSclOut->lTop;
        ulNewSclOutHeight = (ulBottomThresh < ulOldSclOutBottomCut)?
            (ulNewSclOutTop + pAdjDstRect->ulHeight + ulBottomThresh):
            (ulNewSclOutTop + pAdjDstRect->ulHeight + ulOldSclOutBottomCut);

        /* add src top cut */
        ulOffCntTop = BVDC_P_DIV_ROUND_NEAR(
            (((pAdjSclOut->lTop - ulNewSclOutTop) * pSrcCnt->ulHeight) <<
             BVDC_P_16TH_PIXEL_SHIFT), pAdjSclOut->ulHeight);
        pSrcCnt->lTop += ulOffCntTop;

        /* add src height cut: set cnt height a little big to ensure no edge content
         * loss, it is ok to do so because phase and SrcStep also control how much
         * edge src contribute */
        ulNewSrcHeight = BVDC_P_DIV_ROUND_UP(
            ulNewSclOutHeight * pSrcCnt->ulHeight, pAdjSclOut->ulHeight);

        /* Make sure not violate HW limiations */
        if(ulNewSrcHeight < BVDC_P_SRC_INPUT_V_MIN)
        {
            int32_t  lTopAdj;

            /* Adjust clipping */
            lTopAdj = BVDC_P_SRC_INPUT_V_MIN - ulNewSrcHeight;
            pSrcCnt->lTop = BVDC_P_MAX(0,
                pSrcCnt->lTop - (lTopAdj << BVDC_P_16TH_PIXEL_SHIFT));
            ulNewSrcHeight = BVDC_P_SRC_INPUT_V_MIN;

            /* Recalculate srcstep */
            hWindow->ulNrmVrtSrcStep = BVDC_P_CAL_VRT_SRC_STEP(
                ulNewSrcHeight, ulNewSclOutHeight, hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced);
        }
        pSrcCnt->ulHeight = ulNewSrcHeight;

        /* Adjust pSclOut since we already move the cut to source. */
        pAdjSclOut->lTop   = ulNewSclOutTop;
        pAdjSclOut->ulHeight = ulNewSclOutHeight;
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("dstCutXfer: AdjCnt top %d/16(%d), height %d, sclOut top %d, height %d",
            pSrcCnt->lTop, pSrcCnt->lTop/16, pSrcCnt->ulHeight, pAdjSclOut->lTop, pAdjSclOut->ulHeight));
    }

    BDBG_LEAVE(BVDC_P_Window_AdjDstCut_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * This function calculate the destination width of the central region of the
 * 9 non-linear horizontal scaling regions to enforce correct aspect ratio.
 */
static void BVDC_P_Window_CentralRegionWidth_isr
    ( BVDC_Window_Handle               hWindow,
      uintAR_t                         ulSrcPxlAspRatio,  /* 4.16 fx pt */
      uintAR_t                         ulDspPxlAspRatio ) /* 4.16 fx pt */
{
    const BVDC_P_Window_Info *pUserInfo;
    uint32_t  ulSclOutWidth, ulSclOutHeight;
    uint32_t  ulCntWidth, ulCntHeight;
    uintAR_t  ulCntAspR, ulDspAspR;
    uint32_t  ulNonlinearSrcWidth, ulNonlinearSclOutWidth;
    uint32_t  ulCentralRegionSrcWidth, ulCentralRegionSclOutWidth;

    BDBG_ENTER(BVDC_P_Window_CentralRegionDispWidth_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    pUserInfo = &hWindow->stCurInfo;
    ulNonlinearSrcWidth = pUserInfo->ulNonlinearSrcWidth;
    ulNonlinearSclOutWidth = BVDC_P_ALIGN_DN(pUserInfo->ulNonlinearSclOutWidth, 2);
    /* note: src step increases on even pixel only in HW for non-linear scl case */

    BDBG_ASSERT((0 != ulNonlinearSrcWidth) || (0 != ulNonlinearSclOutWidth));
    BDBG_ASSERT((BVDC_AspectRatioMode_eUseAllDestination != pUserInfo->eAspectRatioMode) &&
                (BVDC_AspectRatioMode_eUseAllSource != pUserInfo->eAspectRatioMode));

    ulCntWidth  = hWindow->stSrcCnt.ulWidth;
    ulCntHeight = hWindow->stSrcCnt.ulHeight;
    ulSclOutWidth  = hWindow->stAdjSclOut.ulWidth;
    ulSclOutHeight = hWindow->stAdjSclOut.ulHeight;

    BDBG_ASSERT(ulCntWidth);
    BDBG_ASSERT(ulCntHeight);
    BDBG_ASSERT(ulSclOutWidth);
    BDBG_ASSERT(ulSclOutHeight);

    ulNonlinearSrcWidth = BVDC_P_MIN(ulNonlinearSrcWidth, ulCntWidth >> 1);
    ulNonlinearSclOutWidth = BVDC_P_ALIGN_DN(
        BVDC_P_MIN(ulNonlinearSclOutWidth, ulSclOutWidth >> 1), 2);
    ulCentralRegionSclOutWidth = ulSclOutWidth - 2 * ulNonlinearSclOutWidth;
    ulCentralRegionSrcWidth = ulCntWidth - 2 * ulNonlinearSrcWidth;
    if((2 > ulCentralRegionSclOutWidth) || (0 == ulCentralRegionSrcWidth))
    {
        /* HW increase SrcStep by delta at even pixel, this does not work well
         * if ulCentralRegionSclOutWidth is 0 */
        ulNonlinearSclOutWidth = BVDC_P_MAX((ulSclOutWidth >> 1), 2) - 2;
        ulCentralRegionSclOutWidth = ulSclOutWidth - 2 * ulNonlinearSclOutWidth;
        ulNonlinearSrcWidth = 0; /* force to re-calculate */
    }

    /* make compiler happy */
    ulCntAspR = 0;
    ulDspAspR = 0;
    if((0 == ulNonlinearSrcWidth) || (0 == ulNonlinearSclOutWidth))
    {
        /* calculate the aspect ratio of the scalerOutput, in U10.11 fix pt
         */
        ulDspAspR = (BVDC_P_DIV_ROUND_NEAR(ulSclOutWidth * ulDspPxlAspRatio, ulSclOutHeight) >>
                     (BVDC_P_ASPR_FRAC_BITS_NUM - BVDC_P_SUB_ASPR_FRAC_BITS_NUM));
        ulDspAspR = BVDC_P_MIN(ulDspAspR, (((uintAR_t)1<<BVDC_P_SUB_ASPR_ALL_BITS_NUM) - 1));

        /* calculate the aspect ratio of the content rect, in U10.11 fix pt  */
        ulCntAspR = (BVDC_P_DIV_ROUND_NEAR(ulCntWidth * ulSrcPxlAspRatio, ulCntHeight) >>
                     (BVDC_P_ASPR_FRAC_BITS_NUM - BVDC_P_SUB_ASPR_FRAC_BITS_NUM));
        ulCntAspR = BVDC_P_MIN(ulCntAspR, (((uintAR_t)1<<BVDC_P_SUB_ASPR_ALL_BITS_NUM) - 1));
    }

    if(0 == ulNonlinearSclOutWidth)
    {
        /* need to calculate ulNonlinearSclOutWidth based on ulNonlinearSrcWidth
         * and maintaining aps ratio in central area */
        ulNonlinearSrcWidth = BVDC_P_MIN(ulNonlinearSrcWidth, (ulCntWidth >> 1) - 1);
        ulCentralRegionSrcWidth = ulCntWidth - 2 * ulNonlinearSrcWidth;

        ulCentralRegionSclOutWidth = BVDC_P_DIV_ROUND_NEAR(
            (BVDC_P_DIV_ROUND_NEAR(ulSclOutWidth * ulCntAspR, ulDspAspR) *
             ulCentralRegionSrcWidth), ulCntWidth);
        ulCentralRegionSclOutWidth =
            BVDC_P_MIN(ulCentralRegionSclOutWidth, ulSclOutWidth);

        /* side rgn must be even pxl unit src step increases on even pixel only */
        ulNonlinearSclOutWidth = BVDC_P_ALIGN_DN(
            (ulSclOutWidth - ulCentralRegionSclOutWidth) / 2, 2);
        ulCentralRegionSclOutWidth = ulSclOutWidth - 2 * ulNonlinearSclOutWidth;
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("Try SclOut central region width %d <= %d",
                       ulCentralRegionSclOutWidth, ulSclOutWidth));
    }
    else if(0 == ulNonlinearSrcWidth)
    {
        /* need to calculate ulNonlinearSrcWidth based on ulNonlinearSclOutWidth
         * and maintaining aps ratio in central area */
        ulCentralRegionSrcWidth = BVDC_P_DIV_ROUND_NEAR(
            (BVDC_P_DIV_ROUND_NEAR(ulCntWidth * ulDspAspR, ulCntAspR) *
             ulCentralRegionSclOutWidth), ulSclOutWidth);
        ulCentralRegionSrcWidth =
            BVDC_P_MIN(ulCentralRegionSrcWidth, ulCntWidth);
        if((ulCntWidth - ulCentralRegionSrcWidth) & 0x1)
            ulCentralRegionSrcWidth += 1; /* side rgn must be pxl unit */
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("Try Src central region width %d <= %d",
                       ulCentralRegionSrcWidth, ulCntWidth));
    }

    BDBG_ASSERT(ulCentralRegionSclOutWidth);

    /* round to floor to avoid overflow */
    hWindow->ulNrmHrzSrcStep = BVDC_P_DIV_ROUND_DN(
        ulCentralRegionSrcWidth << BVDC_P_NRM_SRC_STEP_F_BITS, ulCentralRegionSclOutWidth);
    hWindow->ulNonlinearSrcWidth = (ulCntWidth - ulCentralRegionSrcWidth) / 2;
    hWindow->ulNonlinearSclOutWidth = (ulSclOutWidth - ulCentralRegionSclOutWidth) / 2;
    hWindow->ulCentralRegionSclOutWidth = ulCentralRegionSclOutWidth;

    BDBG_MODULE_MSG(BVDC_ASP_RAT,("NonLi SrcW %d, SclOutW %d, Cntr SrcW %d, SclOutW %d",
                   hWindow->ulNonlinearSrcWidth, hWindow->ulNonlinearSclOutWidth,
                   ulCentralRegionSrcWidth, ulCentralRegionSclOutWidth));
    BDBG_MODULE_MSG(BVDC_ASP_RAT,("Centr ulNrmHrzSrcStep %x", hWindow->ulNrmHrzSrcStep));

    BDBG_LEAVE(BVDC_P_Window_CentralRegionDispWidth_isr);
    return;
}

#define BVDC_P_SCL_FCTR_SHIFT       12
#define BVDC_P_SCL_FCTR_INT1        (1<<BVDC_P_SCL_FCTR_SHIFT)
#define BVDC_P_SCL_FCTR_FRAC_MASK   (BVDC_P_SCL_FCTR_INT1 - 1)
/***************************************************************************
 * {private}
 *
 * This function atomatically adjust the window's content size (width or
 * height) or scl-out-rect size (width or height), to round scale factor
 * to an integers when it is close to enough.
 *
 * It is called for both horizontal and vertical scale factor rounding. It
 * uses *pulAdjFlags, *pulSrcCutLen, *pulOutCutLen, *pulNrmSrcStep as both
 * input and output. It outputs its further adjustment by modifying them.
 *
 * Scale factor rounding involves either "adjust to cut less from src and/or
 * further cut out", or "adjust to cut less from out and/or further cut src".
 * It will affect the aspect ratio accuracy. However, since rounding is done
 * only when an integer approximates close enough to the scale factor,
 * therefore it does not totally break the aspect ratio correctness.
 *
 * When adjusting to cut less from src, VDC ensures that user set source
 * clipping is honered.
 *
 * When this func is called, it is called after box auto cut, pan scan, user
 * set src/dst cut, and aspect ratio correction.
 *
 * Theoretically, if the src is mpeg, hddvi, or if letter box auto back cut
 * is enabled, it should be called at every vsync when RUL is built. Otherwise,
 * it should be called only once at the first vsync after ApplyChanges.

 * Optimize: aspect ratio correctio and scale factor rounding needs re-do
 * only if values of SrcCut changed after box cut, pan scan, and user clip,
 * or right after ApplyChanges has been called.
 */
static void BVDC_P_Window_ScaleFactorRounding_isr
    ( uint32_t                         ulSclFctRndToler,
      uint32_t                         ulSrcFullLen,
      uint32_t                         ulAdjFlagShift,
      uint32_t                        *pulAdjFlags,    /* in and out */
      uint32_t                        *pulSrcCutLen,   /* in and out */
      uint32_t                        *pulOutCutLen,   /* in and out */
      uint32_t                        *pulNrmSrcStep ) /* in and out */
{
    uint32_t  ulSrcCutLen, ulOutCutLen, ulAdjFlags;
    uint32_t  ulSclFactor, ulRoundFactor, ulFraction;
    uint32_t  ulTolerance;
    bool  bAdjSrcCut, bAdjOutCut, bAdjSrcStep;

    bAdjSrcCut = false;
    bAdjOutCut = false;
    bAdjSrcStep = false;
    ulAdjFlags = *pulAdjFlags;
    ulSrcCutLen = *pulSrcCutLen;
    ulOutCutLen = *pulOutCutLen;

    if((0 != ulSclFctRndToler) && (0!= ulSrcCutLen) && (0 != (*pulNrmSrcStep >> 3)))
    {
        ulTolerance = ulSclFctRndToler << BVDC_P_SCL_FCTR_SHIFT;
        ulSclFactor = (ulAdjFlags & (BVDC_P_ADJ_SRC_STEP << ulAdjFlagShift))?
            (ulOutCutLen << BVDC_P_SCL_FCTR_SHIFT) / ulSrcCutLen :
            (1 << (BVDC_P_NRM_SRC_STEP_F_BITS - 3 + BVDC_P_SCL_FCTR_SHIFT)) / (*pulNrmSrcStep >> 3);
        ulFraction = ulSclFactor & BVDC_P_SCL_FCTR_FRAC_MASK;
        ulRoundFactor = ulSclFactor >> BVDC_P_SCL_FCTR_SHIFT;
        if((ulFraction <= (BVDC_P_SCL_FCTR_INT1 / 2)) && (0 < ulRoundFactor))
        {
            /* try rounding scale factor down, ==> round srcStep up */
            if((0 != ulFraction) &&
               ((100 * ulFraction) < (ulTolerance * ulRoundFactor)))
            {
                /* rounding down, need to adjust to cut less cnt length, and/or cut more out
                 * note:  if we don't adjust srcCut, we might see a 704 dst window not display
                 * the middle part of 720 src, but the left part */
                if(ulSrcCutLen < ulSrcFullLen)
                {
                    *pulSrcCutLen = BVDC_P_MIN(ulSrcFullLen, ulOutCutLen / ulRoundFactor);
                }
                *pulOutCutLen = ulRoundFactor * (*pulSrcCutLen);

                bAdjSrcCut = (*pulSrcCutLen != ulSrcCutLen);
                bAdjOutCut = (*pulOutCutLen != ulOutCutLen);
                bAdjSrcStep = true;
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("Scale up and SclFctr rouding down, new srcCut %d, outCut %d",
                               *pulSrcCutLen, *pulOutCutLen));
            }
        }
        else
        {
            /* try rounding sclae factor up  ==> round srcStep down */
            ulRoundFactor ++;
            ulFraction = BVDC_P_SCL_FCTR_INT1 - ulFraction;
            if((100 * ulFraction) < (ulTolerance * ulRoundFactor))
            {
                /* rounding up, need to adjust to cut less out length, or expand outside.
                 * note 1: ok to make sclOut size big, it will not cause garbage into video
                 * like the srcCnt case, and extra output will be clipped by CMP (dstRect
                 * is not expanded), note 2:  if we don't adjust outCut, we might see a 704
                 * src not displayed in the middile of 720 screen, but to the left. */
                *pulSrcCutLen = BVDC_P_MIN(ulSrcFullLen, ulOutCutLen / ulRoundFactor);
                *pulOutCutLen = ulRoundFactor * (*pulSrcCutLen);
                bAdjSrcCut = (*pulSrcCutLen != ulSrcCutLen);
                bAdjOutCut = (*pulOutCutLen != ulOutCutLen);
                bAdjSrcStep = true;
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("Scale up and SclFctr rouding up, new srcCut %d, outCut %d",
                           *pulSrcCutLen, *pulOutCutLen));
            }
        }

        /* more apply of our adj decision */
        if (bAdjSrcStep)
        {
            *pulNrmSrcStep = BVDC_P_DIV_ROUND_NEAR(
                1 << BVDC_P_NRM_SRC_STEP_F_BITS, ulRoundFactor);
            ulAdjFlags &= ~(BVDC_P_ADJ_SRC_STEP << ulAdjFlagShift); /* just did above */
            ulAdjFlags |= (
                ((bAdjSrcCut)? (BVDC_P_ADJ_CNT_CUT << ulAdjFlagShift) : 0) |
                ((bAdjOutCut)? (BVDC_P_ADJ_OUT_CUT << ulAdjFlagShift) : 0));
            *pulAdjFlags =  ulAdjFlags;
        }
    }
}

/***************************************************************************
 * {private}
 *
 * Utility func called by BVDC_P_Window_AdjustRectangles_isr to perform
 * automatic rectangle cut, decided by BVDC_P_AspectRatioCorrection_isrsafe
 * and/or BVDC_P_Window_ScaleFactorRounding_isr, and stored the result into
 * struct hWindow->stSrcCnt or hWindow->stAdjSclOut / stAdjDstRect.
 */
static void BVDC_P_Window_ApplyAutoAdj_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_AutoAdj                  *pAutoAdj,
      uint32_t                         ulFullSrcWidth,
      uint32_t                         ulFullSrcHeight,
      bool                             bWindowMute )
{
    uint32_t  ulNewSclOutWidth, ulOldSclOutWidth, ulNewDstWidth;
    uint32_t  ulNewSclOutHeight, ulOldSclOutHeight, ulNewDstHeight;
    uint32_t  ulTmpOff;
    uint32_t  ulUsrClipLeft, ulUsrClipRight, ulUsrClipTop, ulUsrClipBottom;
    BVDC_P_ClipRect  *pUsrClip;
    BVDC_P_ClipRect stMuteSrcClip;

    if(bWindowMute)
    {
        stMuteSrcClip.ulLeft = 0;
        stMuteSrcClip.ulRight = 0;
        stMuteSrcClip.ulTop = 0;
        stMuteSrcClip.ulBottom = 0;
        pUsrClip = &stMuteSrcClip;
    }
    else
    {
        pUsrClip = &hWindow->stCurInfo.stSrcClip;
    }

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    /* cnt horiz cut */
    if((pAutoAdj->ulCntWidth < hWindow->stSrcCnt.ulWidth) &&
       (pAutoAdj->ulAdjFlags & BVDC_P_ADJ_CNT_WIDTH))
    {
        /* more cut to src cnt width */
        ulTmpOff = ((hWindow->stSrcCnt.ulWidth - pAutoAdj->ulCntWidth) <<
                    (BVDC_P_16TH_PIXEL_SHIFT - 1));
        hWindow->stSrcCnt.lLeft += ulTmpOff;
        hWindow->stSrcCnt.lLeft_R += ulTmpOff;
        hWindow->stSrcCnt.ulWidth = pAutoAdj->ulCntWidth;
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("Auto cut cnt width, new SrcWidth %d", pAutoAdj->ulCntWidth));
    }
    else if((pAutoAdj->ulCntWidth > hWindow->stSrcCnt.ulWidth) &&
            (pAutoAdj->ulAdjFlags & BVDC_P_ADJ_CNT_WIDTH))
    {
        /* less cut to src cnt width */
        ulTmpOff = ((pAutoAdj->ulCntWidth - hWindow->stSrcCnt.ulWidth) <<
                    (BVDC_P_16TH_PIXEL_SHIFT - 1));

        /* BVDC_P_Window_ScaleFactorRounding_isr make sure that
         * pAutoAdj->ulCntWidth < ulFullSrcWidth - pUsrClip->ulLeft - pUsrClip->ulRight */
        ulUsrClipLeft  = pUsrClip->ulLeft  << BVDC_P_16TH_PIXEL_SHIFT;
        ulUsrClipRight = pUsrClip->ulRight << BVDC_P_16TH_PIXEL_SHIFT;
        if (hWindow->stSrcCnt.lLeft <= (int32_t)(ulTmpOff + ulUsrClipLeft))
        {
            /* left edge over-shoot handle */
            hWindow->stSrcCnt.lLeft = ulUsrClipLeft;
        }
        else if ((hWindow->stSrcCnt.lLeft - ulTmpOff +
                  (pAutoAdj->ulCntWidth << BVDC_P_16TH_PIXEL_SHIFT)) >
                 ((ulFullSrcWidth << BVDC_P_16TH_PIXEL_SHIFT) - ulUsrClipRight))
        {
            /* right edge over-shoot handle */
            hWindow->stSrcCnt.lLeft =
                ((ulFullSrcWidth - pAutoAdj->ulCntWidth) << BVDC_P_16TH_PIXEL_SHIFT) - ulUsrClipRight;
        }
        else
        {
            /* expand both sides */
            hWindow->stSrcCnt.lLeft -= ulTmpOff;
        }

        ulUsrClipLeft  = (pUsrClip->ulLeft  + pUsrClip->lLeftDelta_R) << BVDC_P_16TH_PIXEL_SHIFT;
        ulUsrClipRight = (pUsrClip->ulRight - pUsrClip->lLeftDelta_R) << BVDC_P_16TH_PIXEL_SHIFT;
        if (hWindow->stSrcCnt.lLeft_R <= (int32_t)(ulTmpOff + ulUsrClipLeft))
        {
            /* left edge over-shoot handle */
            hWindow->stSrcCnt.lLeft_R = ulUsrClipLeft;
        }
        else if ((hWindow->stSrcCnt.lLeft_R - ulTmpOff +
                  (pAutoAdj->ulCntWidth << BVDC_P_16TH_PIXEL_SHIFT)) >
                 ((ulFullSrcWidth << BVDC_P_16TH_PIXEL_SHIFT) - ulUsrClipRight))
        {
            /* right edge over-shoot handle */
            hWindow->stSrcCnt.lLeft_R =
                ((ulFullSrcWidth - pAutoAdj->ulCntWidth) << BVDC_P_16TH_PIXEL_SHIFT) - ulUsrClipRight;
        }
        else
        {
            /* expand both sides */
            hWindow->stSrcCnt.lLeft_R -= ulTmpOff;
        }
        hWindow->stSrcCnt.ulWidth = pAutoAdj->ulCntWidth;
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("Auto expand cnt width, new SrcWidth %d", pAutoAdj->ulCntWidth));
    }

    /* cnt vertical cut */
    if((pAutoAdj->ulCntHeight < hWindow->stSrcCnt.ulHeight) &&
       (pAutoAdj->ulAdjFlags & BVDC_P_ADJ_CNT_HEIGHT))
    {
        /* more cut to src height */
        ulTmpOff = ((hWindow->stSrcCnt.ulHeight - pAutoAdj->ulCntHeight) <<
                    (BVDC_P_16TH_PIXEL_SHIFT - 1));
        hWindow->stSrcCnt.lTop += ulTmpOff;
        hWindow->stSrcCnt.ulHeight = pAutoAdj->ulCntHeight;
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("Auto cut cnt height, new SrcHeight %d", pAutoAdj->ulCntHeight));
    }
    else if ((pAutoAdj->ulCntHeight > hWindow->stSrcCnt.ulHeight) &&
             (pAutoAdj->ulAdjFlags & BVDC_P_ADJ_CNT_HEIGHT))
    {
        /* less cut to src cnt height */
        ulTmpOff = ((pAutoAdj->ulCntHeight - hWindow->stSrcCnt.ulHeight) <<
                    (BVDC_P_16TH_PIXEL_SHIFT - 1));

        /* BVDC_P_Window_ScaleFactorRounding_isr make sure that
         * pAutoAdj->ulCntHeight < ulFullSrcHeight - pUsrClip->ulTop - pUsrClip->ulBottom */
        ulUsrClipTop    = pUsrClip->ulTop    << BVDC_P_16TH_PIXEL_SHIFT;
        ulUsrClipBottom = pUsrClip->ulBottom << BVDC_P_16TH_PIXEL_SHIFT;
        if (hWindow->stSrcCnt.lTop <= (int32_t)(ulTmpOff + ulUsrClipTop))
            /* top edge over-shoot handle */
            hWindow->stSrcCnt.lTop = ulUsrClipTop;
        else if ((hWindow->stSrcCnt.lTop - ulTmpOff +
                  (pAutoAdj->ulCntHeight << BVDC_P_16TH_PIXEL_SHIFT)) >
                 ((ulFullSrcHeight << BVDC_P_16TH_PIXEL_SHIFT) - ulUsrClipBottom))
            /* bottom edge over-shoot handle */
            hWindow->stSrcCnt.lTop =
                ((ulFullSrcHeight - pAutoAdj->ulCntHeight) << BVDC_P_16TH_PIXEL_SHIFT) - ulUsrClipBottom;
        else
            /* expand both sides */
            hWindow->stSrcCnt.lTop -= ulTmpOff;
        hWindow->stSrcCnt.ulHeight = pAutoAdj->ulCntHeight;
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("Auto expand cnt height, new SrcHeight %d", pAutoAdj->ulCntHeight));
    }

    /* scl/dst cut */
    if(!hWindow->stCurInfo.bMosaicMode)
    {
        /* cut scl-out/dst width */
        if((pAutoAdj->ulOutWidth < hWindow->stAdjSclOut.ulWidth) &&
           (pAutoAdj->ulAdjFlags & BVDC_P_ADJ_OUT_WIDTH))
        {
            /* Due to HW limit, scale-out width should be >= 16.
             * we should not introduce the violation */
            ulNewSclOutWidth = BVDC_P_MAX(pAutoAdj->ulOutWidth, BVDC_P_WIN_SCL_OUTPUT_H_MIN);

            /* adjust scaler-out width */
            ulOldSclOutWidth = hWindow->stAdjSclOut.ulWidth;
            hWindow->stAdjSclOut.ulWidth = ulNewSclOutWidth;
            BDBG_MODULE_MSG(BVDC_ASP_RAT,("Auto cut scl-out width, new SclOutWidth %d", ulNewSclOutWidth));

            /* adjust dest rect and its offset inside scaler-output rect */
            if(0!=ulOldSclOutWidth)
            {
                hWindow->stAdjSclOut.lLeft = BVDC_P_DIV_ROUND_NEAR(
                    ulNewSclOutWidth * hWindow->stAdjSclOut.lLeft, ulOldSclOutWidth);
                hWindow->stAdjSclOut.lLeft_R = BVDC_P_DIV_ROUND_NEAR(
                    ulNewSclOutWidth * hWindow->stAdjSclOut.lLeft_R, ulOldSclOutWidth);
                ulNewDstWidth = BVDC_P_DIV_ROUND_NEAR(
                    ulNewSclOutWidth * hWindow->stAdjDstRect.ulWidth, ulOldSclOutWidth);
                hWindow->stAdjDstRect.lLeft += (hWindow->stAdjDstRect.ulWidth - ulNewDstWidth) / 2;
                hWindow->stAdjDstRect.lLeft_R += (hWindow->stAdjDstRect.ulWidth - ulNewDstWidth) / 2;
                hWindow->stAdjDstRect.ulWidth = ulNewDstWidth;
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("Auto cut dst w: new left %d, width %d",
                               hWindow->stAdjDstRect.lLeft, ulNewDstWidth));
            }
        }
        else if((pAutoAdj->ulOutWidth > hWindow->stAdjSclOut.ulWidth) &&
                (pAutoAdj->ulAdjFlags & BVDC_P_ADJ_OUT_WIDTH))
        {
            /* sclOut width expanded a little due to sclFactorRounding,
             * but we can not expand dstRect */
            ulNewSclOutWidth = pAutoAdj->ulOutWidth;
            ulOldSclOutWidth = hWindow->stAdjSclOut.ulWidth;
            hWindow->stAdjSclOut.ulWidth = ulNewSclOutWidth;
            hWindow->stAdjSclOut.lLeft += (ulNewSclOutWidth - ulOldSclOutWidth) / 2;
            hWindow->stAdjSclOut.lLeft_R += (ulNewSclOutWidth - ulOldSclOutWidth) / 2;
            BDBG_MODULE_MSG(BVDC_ASP_RAT,("Auto expand scl-out width, new SclOutWidth %d", ulNewSclOutWidth));
            BDBG_MODULE_MSG(BVDC_ASP_RAT,("dstRect no change, new sclOut.left %d", hWindow->stAdjSclOut.lLeft));
        }

        /* cut scl-out/dst height */
        if((pAutoAdj->ulOutHeight < hWindow->stAdjSclOut.ulHeight) &&
           (pAutoAdj->ulAdjFlags & BVDC_P_ADJ_OUT_HEIGHT))
        {
            ulNewSclOutHeight = pAutoAdj->ulOutHeight;

            /* adjust scaler-out height */
            ulOldSclOutHeight = hWindow->stAdjSclOut.ulHeight;
            hWindow->stAdjSclOut.ulHeight = ulNewSclOutHeight;
            BDBG_MODULE_MSG(BVDC_ASP_RAT,("Auto cut scl-out height: new SclOutHeight %d", ulNewSclOutHeight));

            /* adjust dest rect and its offset inside scaler-output rect */
            if(0!=ulOldSclOutHeight)
            {
                hWindow->stAdjSclOut.lTop = BVDC_P_DIV_ROUND_NEAR(
                    ulNewSclOutHeight * hWindow->stAdjSclOut.lTop, ulOldSclOutHeight);
                ulNewDstHeight = BVDC_P_DIV_ROUND_NEAR(
                    ulNewSclOutHeight * hWindow->stAdjDstRect.ulHeight, ulOldSclOutHeight);
                hWindow->stAdjDstRect.lTop += ((hWindow->stAdjDstRect.ulHeight - ulNewDstHeight) / 2);
                hWindow->stAdjDstRect.ulHeight = ulNewDstHeight;
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("Auto cut dst H: new top %d, height %d",
                               hWindow->stAdjDstRect.lTop, ulNewDstHeight));
            }
        }
        else if((pAutoAdj->ulOutHeight > hWindow->stAdjSclOut.ulHeight) &&
                (pAutoAdj->ulAdjFlags & BVDC_P_ADJ_OUT_HEIGHT))
        {
            /* sclOut height expanded a little due to sclFactorRounding,
             * but we can not expand dstRect */
            ulNewSclOutHeight = pAutoAdj->ulOutHeight;
            ulOldSclOutHeight = hWindow->stAdjSclOut.ulHeight;
            hWindow->stAdjSclOut.ulHeight = ulNewSclOutHeight;
            hWindow->stAdjSclOut.lTop += (ulNewSclOutHeight - ulOldSclOutHeight) / 2;
            BDBG_MODULE_MSG(BVDC_ASP_RAT,("Auto expand scl-out height, new SclOutHeight %d", ulNewSclOutHeight));
            BDBG_MODULE_MSG(BVDC_ASP_RAT,("dstRect no change, new sclOut.top %d", hWindow->stAdjSclOut.lTop));
        }
    }
}


/***************************************************************************
 * {private}
 *
 * Utility func called by BVDC_P_Window_AdjustRectangles_isr to perform
 * automatic rectangle adjustment, decided by BVDC_P_Window_AspectRatio-
 * Correction_isr and/or BVDC_P_Window_ScaleFactorRounding_isr,  and stored
 * the result into struct hWindow->stSrcCnt or hWindow->stAdjSclOut /
 * stAdjDstRect.
 */
static void BVDC_P_Window_AutoAdjRelatedToSclRatio_isr
    ( BVDC_Window_Handle               hWindow,
      bool                             bMosaicMode,
      uintAR_t                         ulSrcPxlAspRatio, /* 4.16 fx pt */
      uintAR_t                         ulDspPxlAspRatio, /* 4.16 fx pt */
      uint32_t                         ulSrcWidth,
      uint32_t                         ulSrcHeight,
      bool                             bWindowMute )
{
    BVDC_P_Window_Info *pUserInfo;
    BVDC_P_AutoAdj  stAutoAdj;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    stAutoAdj.ulAdjFlags = 0;
    stAutoAdj.ulCntWidth = hWindow->stSrcCnt.ulWidth;
    stAutoAdj.ulCntHeight = hWindow->stSrcCnt.ulHeight;
    stAutoAdj.ulOutWidth = hWindow->stAdjSclOut.ulWidth;
    stAutoAdj.ulOutHeight = hWindow->stAdjSclOut.ulHeight;

    /* --- (6.1) update stAutoAdj for aspect ratio correction --- */
    pUserInfo = &hWindow->stCurInfo;
    if((BVDC_AspectRatioMode_eUseAllDestination == pUserInfo->eAspectRatioMode) ||
       (BVDC_AspectRatioMode_eUseAllSource      == pUserInfo->eAspectRatioMode))
    {
#if BVDC_P_SHOW_ASP_RAT_MSG
        BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
#endif

        BDBG_MODULE_MSG(BVDC_ASP_RAT,("UserSrcClip: (L %d, R %d, T %d, B %d)",
            pUserInfo->stSrcClip.ulLeft, pUserInfo->stSrcClip.ulRight,
            pUserInfo->stSrcClip.ulTop, pUserInfo->stSrcClip.ulBottom));
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("UserSclOut: (L %d, T %d, W %d, H %d); UserDstRect: (L %d, T %d, W %d, H %d)",
            pUserInfo->stScalerOutput.lLeft, pUserInfo->stScalerOutput.lTop,
            pUserInfo->stScalerOutput.ulWidth, pUserInfo->stScalerOutput.ulHeight,
            pUserInfo->stDstRect.lLeft, pUserInfo->stDstRect.lTop,
            pUserInfo->stDstRect.ulWidth, pUserInfo->stDstRect.ulHeight));
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("CntRect before AspR correct: (L %d.%d/16, T %d.%d/16, W %d, H %d)",
            hWindow->stSrcCnt.lLeft>>4, hWindow->stSrcCnt.lLeft & 0xf,
            hWindow->stSrcCnt.lTop>>4,  hWindow->stSrcCnt.lTop  & 0xf,
            hWindow->stSrcCnt.ulWidth, hWindow->stSrcCnt.ulHeight));
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("SclOut before AspR correct: (L %d, T %d, W %d, H %d)",
            hWindow->stAdjSclOut.lLeft, hWindow->stAdjSclOut.lTop,
            hWindow->stAdjSclOut.ulWidth, hWindow->stAdjSclOut.ulHeight));
        BDBG_MODULE_MSG(BVDC_ASP_RAT,("DstRect before AspR correct: (L %d, T %d, W %d, H %d)",
            hWindow->stAdjDstRect.lLeft, hWindow->stAdjDstRect.lTop,
            hWindow->stAdjDstRect.ulWidth, hWindow->stAdjDstRect.ulHeight));

        BVDC_P_AspectRatioCorrection_isrsafe(
            ulSrcPxlAspRatio,
            ulDspPxlAspRatio,
            hWindow->stCurInfo.eAspectRatioMode,
            hWindow->stCurInfo.ulHrzSclFctRndToler,
            hWindow->stCurInfo.ulVrtSclFctRndToler,
            &stAutoAdj);
    }

    /* --- (6.2) adjust stAutoAdj for scale factor rounding --- */
    if(VIDEO_FORMAT_IS_SD(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt) ||
       VIDEO_FORMAT_IS_ED(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
    {
        if((0 != pUserInfo->ulHrzSclFctRndToler) && !bMosaicMode)
        {
            BVDC_P_Window_ScaleFactorRounding_isr(pUserInfo->ulHrzSclFctRndToler,
                ulSrcWidth - pUserInfo->stSrcClip.ulLeft - pUserInfo->stSrcClip.ulRight,
                BVDC_P_ADJ_FLAG_HRZ_SHIFT, &(stAutoAdj.ulAdjFlags),
                &(stAutoAdj.ulCntWidth), &(stAutoAdj.ulOutWidth), &(hWindow->ulNrmHrzSrcStep));
        }
        if((0 != pUserInfo->ulVrtSclFctRndToler) && !bMosaicMode)
        {
            BVDC_P_Window_ScaleFactorRounding_isr(pUserInfo->ulVrtSclFctRndToler,
                ulSrcHeight - pUserInfo->stSrcClip.ulTop - pUserInfo->stSrcClip.ulBottom,
                BVDC_P_ADJ_FLAG_VRT_SHIFT, &(stAutoAdj.ulAdjFlags),
                &(stAutoAdj.ulCntHeight), &(stAutoAdj.ulOutHeight), &(hWindow->ulNrmVrtSrcStep));
        }
    }

    /* --- (6.3) apply stAutoAdj to hWindow->stSrcCnt/stAdjSclOut/stAdjDstRect --- */
    BVDC_P_Window_ApplyAutoAdj_isr(hWindow, &stAutoAdj, ulSrcWidth, ulSrcHeight, bWindowMute);

    /* --- (6.4) re-calculate nrm srcStep */
    if (stAutoAdj.ulAdjFlags & BVDC_P_ADJ_HRZ_SRC_STEP)
    {
        hWindow->ulNrmHrzSrcStep = BVDC_P_CAL_HRZ_SRC_STEP(
            hWindow->stSrcCnt.ulWidth, hWindow->stAdjSclOut.ulWidth);
    }
    if (stAutoAdj.ulAdjFlags & BVDC_P_ADJ_VRT_SRC_STEP)
    {
        hWindow->ulNrmVrtSrcStep = BVDC_P_CAL_VRT_SRC_STEP(
            hWindow->stSrcCnt.ulHeight, hWindow->stAdjSclOut.ulHeight, hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced);
    }
}

/***************************************************************************
 * {private}
 *
 * Utility func called by BVDC_P_Window_AdjustRectangles_isr to calculate
 * the STG pixel aspect ratio when aspect ratio correction is not performed.
 * It is the scaled source pixel aspect ratio
 */
#define BVDC_P_MSB(x)  (31 -  __builtin_clz(x))
static void BVDC_P_ScaleSrcPxlAspRatio_isr
    ( uint32_t                         ulSrcPxlAspRatio_x_y,
      uint32_t                         ulSrcWidth,
      uint32_t                         ulSrcHeight,
      uint32_t                         ulDstWidth,
      uint32_t                         ulDstHeight,
      uint32_t                        *pulOutPxlAspRatio_x_y )
{
    uint32_t ulPxlAspR_x, ulPxlAspR_y;
    uint32_t ulPxlAspR_x_hi, ulPxlAspR_x_low, ulPxlAspR_y_hi, ulPxlAspR_y_low;
    uint32_t ulTmp, ulMsb;

    ulPxlAspR_x = ulSrcPxlAspRatio_x_y >> 16;
    ulPxlAspR_y = ulSrcPxlAspRatio_x_y & 0xffff;

    ulPxlAspR_x *= ulSrcWidth;
    BMTH_HILO_32TO64_Mul(ulPxlAspR_x, ulDstHeight,
                         &ulPxlAspR_x_hi, &ulPxlAspR_x_low);
    ulPxlAspR_y *= ulSrcHeight;
    BMTH_HILO_32TO64_Mul(ulPxlAspR_y, ulDstWidth,
                         &ulPxlAspR_y_hi, &ulPxlAspR_y_low);

    /* we only need 16 bits for output PxlAspR_x and PxlAspR_y */
    if (ulPxlAspR_x_hi || ulPxlAspR_y_hi)
    {
        /* shift off the low 16 bits. */
        ulPxlAspR_x_low >>= 16;
        ulPxlAspR_y_low >>= 16;
        ulPxlAspR_x_low |= (ulPxlAspR_x_hi << 16);
        ulPxlAspR_y_low |= (ulPxlAspR_y_hi << 16);
    }

    /* STG/ViCE hope to have smaller ulPxlAspR_x and ulPxlAspR_y, so we try to find
     * the GCD of them, and then divide them by the GCD if it is found
     * note: the original numbers are less than 12 + 12 + 16 = 40 bits,
     * so after the 16 bits right shifting, we now have less than 32 bits */
    if(ulPxlAspR_x_low == ulPxlAspR_y_low)
    {
        ulPxlAspR_x_low = ulPxlAspR_y_low = 1;
    }
    else
    {
        uint32_t b = 0, a = 0, m = 0, i = 0;
        a = (ulPxlAspR_y_low > ulPxlAspR_x_low)? ulPxlAspR_y_low : ulPxlAspR_x_low;
        b = (ulPxlAspR_y_low > ulPxlAspR_x_low)? ulPxlAspR_x_low : ulPxlAspR_y_low;

        while (b  && (i<10)) { m = a % b; a = b; b = m; i++; }

        if (i < 10) /* found GCD a */
        {
            ulPxlAspR_y_low /= a;
            ulPxlAspR_x_low /= a;
        }
    }

    /* */
    ulTmp = BVDC_P_MAX(ulPxlAspR_x_low, ulPxlAspR_y_low);
    ulMsb = BVDC_P_MSB(ulTmp);
    if (ulMsb > 15) /* the max of msb of uint16_t number is 15 */
    {
        ulPxlAspR_x_low >>= (ulMsb - 15);
        ulPxlAspR_y_low >>= (ulMsb - 15);
    }

    *pulOutPxlAspRatio_x_y = (ulPxlAspR_x_low << 16) | ulPxlAspR_y_low;
}

/***************************************************************************
 * {private}
 *
 * It is only used by BVDC_P_Window_AdjustRectangles_isr
 */
static void BVDC_P_Window_AdjOffscreen_isr
    ( BVDC_Window_Handle               hWindow,
      const BFMT_VideoInfo            *pDstFmtInfo,
      BVDC_P_Rect                     *pSrcCnt,
      BVDC_P_Rect                     *pAdjSclOut,
      BVDC_P_Rect                     *pAdjDstRect )
{
    uint32_t ulBoxWinId;
    BVDC_DisplayId eDisplayId = hWindow->hCompositor->hDisplay->eId;
    const BBOX_Vdc_Capabilities *pBoxVdc;
    uint32_t ulBoxWindowHeightFraction, ulBoxWindowWidthFraction;
    uint32_t ulMaxWidth, ulMaxHeight;
    uint32_t ulMinV = pDstFmtInfo->bInterlaced ? BVDC_P_WIN_DST_OUTPUT_V_MIN * 2 : BVDC_P_WIN_DST_OUTPUT_V_MIN;

    /* Check if destination rectangle is bigger than BOX limits */
    pBoxVdc = &hWindow->hCompositor->hVdc->stBoxConfig.stVdc;
    ulBoxWinId = BVDC_P_GetBoxWindowId_isrsafe(hWindow->eId);

    BDBG_ASSERT(ulBoxWinId < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY);

    ulBoxWindowHeightFraction = pBoxVdc->astDisplay[eDisplayId].astWindow[ulBoxWinId].stSizeLimits.ulHeightFraction;
    ulBoxWindowWidthFraction = pBoxVdc->astDisplay[eDisplayId].astWindow[ulBoxWinId].stSizeLimits.ulWidthFraction;

    if(pAdjDstRect->lTop  + (int32_t)pAdjDstRect->ulHeight <= 0 ||
       pAdjDstRect->lLeft + (int32_t)pAdjDstRect->ulWidth  <= 0 ||
       pAdjDstRect->lTop  >= (int32_t)pDstFmtInfo->ulHeight ||
       pAdjDstRect->lLeft >= (int32_t)pDstFmtInfo->ulWidth)
    {
        /* Mute video */
        hWindow->bMuteBypass = true;
        BDBG_MSG(("Offscreen MUTE: [%d,%d] [%ux%u] [%ux%u]", pAdjDstRect->lLeft, pAdjDstRect->lTop, pAdjDstRect->ulWidth,
            pAdjDstRect->ulHeight, pDstFmtInfo->ulWidth, pDstFmtInfo->ulHeight));
        pAdjDstRect->lTop = 0;
        pAdjDstRect->lLeft = 0;
        pAdjDstRect->ulWidth = BVDC_P_WIN_DST_OUTPUT_H_MIN;
        pAdjDstRect->ulHeight = ulMinV;
    }
    else
    {
        /* when mute toggles, reprogram CFC */
        if(hWindow->bMuteBypass) {
            hWindow->bCfcDirty = true;
            BDBG_MSG(("Win%d mute -> UNMUTE!", hWindow->eId));
        }
        hWindow->bMuteBypass = false;
        if(pAdjDstRect->lTop < 0)
        {
            if(pAdjDstRect->lTop + pAdjDstRect->ulHeight < pDstFmtInfo->ulHeight)
                pAdjDstRect->ulHeight = pAdjDstRect->lTop + pAdjDstRect->ulHeight;
            else
                pAdjDstRect->ulHeight = pDstFmtInfo->ulHeight;
            pAdjSclOut->lTop = -pAdjDstRect->lTop;
            pAdjDstRect->lTop = 0;
        }
        else
        {
            if(pAdjDstRect->lTop + pAdjDstRect->ulHeight > pDstFmtInfo->ulHeight)
            {
                pAdjDstRect->ulHeight = pDstFmtInfo->ulHeight - pAdjDstRect->lTop;
            }
        }

        if(pAdjDstRect->lLeft < 0)
        {
            if(pAdjDstRect->lLeft + pAdjDstRect->ulWidth < pDstFmtInfo->ulWidth)
                pAdjDstRect->ulWidth = pAdjDstRect->lLeft + pAdjDstRect->ulWidth;
            else
                pAdjDstRect->ulWidth = pDstFmtInfo->ulWidth;
            pAdjSclOut->lLeft = -pAdjDstRect->lLeft;
            pAdjDstRect->lLeft = 0;
        }
        else
        {
            if(pAdjDstRect->lLeft + pAdjDstRect->ulWidth > pDstFmtInfo->ulWidth)
            {
                pAdjDstRect->ulWidth = pDstFmtInfo->ulWidth - pAdjDstRect->lLeft;
            }
        }
    }

    if (ulBoxWindowHeightFraction != BBOX_VDC_DISREGARD && ulBoxWindowWidthFraction != BBOX_VDC_DISREGARD)
    {
        BDBG_ASSERT(ulBoxWindowHeightFraction);
        BDBG_ASSERT(ulBoxWindowWidthFraction);

        ulMaxWidth  = BVDC_P_MIN(pDstFmtInfo->ulWidth,  pDstFmtInfo->ulWidth  / ulBoxWindowWidthFraction);
        ulMaxHeight = BVDC_P_MIN(pDstFmtInfo->ulHeight, pDstFmtInfo->ulHeight / ulBoxWindowHeightFraction);

        pAdjDstRect->ulWidth  = BVDC_P_MIN(pAdjDstRect->ulWidth,  ulMaxWidth);
        pAdjDstRect->ulHeight = BVDC_P_MIN(pAdjDstRect->ulHeight, ulMaxHeight);
    }
    else
    {
        ulMaxWidth  = pDstFmtInfo->ulWidth;
        ulMaxHeight = pDstFmtInfo->ulHeight;
    }

    if(pAdjDstRect->ulHeight < ulMinV)
    {
        pAdjSclOut->lTop = pAdjSclOut->lTop - (ulMinV - pAdjDstRect->ulHeight);
        if(pAdjSclOut->lTop < 0)
        {
            pAdjSclOut->lTop = 0;
        }
        pAdjDstRect->ulHeight = ulMinV;
        BDBG_MSG(("Offscreen: Align up displaying %s, to min dst vertical %d", (pDstFmtInfo->bInterlaced) ?  "interlace" : "progressive", ulMinV));
    }

    if(pAdjSclOut->ulHeight < ulMinV)
    {
        pAdjSclOut->ulHeight = ulMinV;
        BDBG_MSG(("Offscreen: Align up surface %s, to min dst vertical %d", (pDstFmtInfo->bInterlaced) ?  "interlace" : "progressive", ulMinV));
    }

    if(pAdjDstRect->ulWidth < BVDC_P_WIN_DST_OUTPUT_H_MIN)
    {
        pAdjSclOut->lLeft = pAdjSclOut->lLeft - (BVDC_P_WIN_DST_OUTPUT_H_MIN - pAdjDstRect->ulWidth);
        if(pAdjSclOut->lLeft < 0)
        {
            pAdjSclOut->lLeft = 0;
        }
        pAdjDstRect->ulWidth = BVDC_P_WIN_DST_OUTPUT_H_MIN;
        BDBG_MSG(("Offscreen: Align up displaying to min dst horizontal %d", BVDC_P_WIN_DST_OUTPUT_H_MIN));
    }

    if(pAdjSclOut->ulWidth < BVDC_P_WIN_DST_OUTPUT_H_MIN)
    {
        pAdjSclOut->ulWidth = BVDC_P_WIN_DST_OUTPUT_H_MIN;
        BDBG_MSG(("Offscreen: Align up surface to min dst horizontal %d", BVDC_P_WIN_DST_OUTPUT_H_MIN));
    }

    if((pAdjDstRect->lLeft + pAdjDstRect->ulWidth  > pDstFmtInfo->ulWidth) ||
       (pAdjDstRect->lTop  + pAdjDstRect->ulHeight > pDstFmtInfo->ulHeight))
    {
        hWindow->bMuteBypass = true;
        BDBG_MSG(("Offscreen MUTE: [%d,%d] [%ux%u]", pAdjDstRect->lLeft, pAdjDstRect->lTop, pAdjDstRect->ulWidth,
            pAdjDstRect->ulHeight));
        pAdjDstRect->lTop = 0;
        pAdjDstRect->lLeft = 0;
        pAdjDstRect->ulWidth = BVDC_P_WIN_DST_OUTPUT_H_MIN;
        pAdjDstRect->ulHeight = ulMinV;
    }

    BVDC_P_Window_AdjDstCut_isr(hWindow, ulMaxWidth, ulMaxHeight, pSrcCnt, pAdjSclOut, pAdjDstRect);
}

/***************************************************************************
 * {private}
 *
 * This function will be called when source is muted instead of
 * AdjustRectangles_isr to initialize all variables needed by
 * UpdateSrcAndUserInfo_isr.
 */
void BVDC_P_Window_InitMuteRec_isr
    ( BVDC_Window_Handle               hWindow,
      const BFMT_VideoInfo            *pDstFmtInfo,
      const BAVC_MVD_Field            *pMvdFieldData,
      uint32_t                         ulRectIdx )
{
    bool bNonlinearScl;
    bool bDoAspRatCorrect;

    uintAR_t  ulSrcPxlAspRatio = 0, ulDspPxlAspRatio = 0;  /* pixel asp ratio in U4.16 fix pt */
    uint32_t  ulSrcPxlAspRatio_x_y=0;
    bool  bMosaicMode;

    BDBG_ENTER(BVDC_P_Window_InitMuteRec_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    bMosaicMode = hWindow->stCurInfo.bMosaicMode;

#if BVDC_P_SUPPORT_STG
    /* if stg resol ramp transition */
    if(hWindow->hCompositor->hDisplay->stCurInfo.ulResolutionRampCount)
    {
        hWindow->stCurInfo.stDstRect.ulWidth  = hWindow->hCompositor->hDisplay->astStgRampWinDst[hWindow->eId].ulWidth / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        hWindow->stCurInfo.stDstRect.ulHeight = hWindow->hCompositor->hDisplay->astStgRampWinDst[hWindow->eId].ulHeight/ BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        hWindow->stCurInfo.stDstRect.lLeft = hWindow->hCompositor->hDisplay->astStgRampWinDst[hWindow->eId].lLeft / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        hWindow->stCurInfo.stDstRect.lTop  = hWindow->hCompositor->hDisplay->astStgRampWinDst[hWindow->eId].lTop  / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        hWindow->stCurInfo.stScalerOutput.ulWidth  = hWindow->hCompositor->hDisplay->astStgRampWinSclOut[hWindow->eId].ulWidth / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        hWindow->stCurInfo.stScalerOutput.ulHeight = hWindow->hCompositor->hDisplay->astStgRampWinSclOut[hWindow->eId].ulHeight/ BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        hWindow->stCurInfo.stScalerOutput.lLeft = hWindow->hCompositor->hDisplay->astStgRampWinSclOut[hWindow->eId].lLeft / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        hWindow->stCurInfo.stScalerOutput.lTop  = hWindow->hCompositor->hDisplay->astStgRampWinSclOut[hWindow->eId].lTop  / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        BDBG_MSG(("Win%u mute halved output size due to resol ramp[%u]!", hWindow->eId, hWindow->hCompositor->hDisplay->stCurInfo.ulResolutionRampCount));
    }
#endif

#if BVDC_P_SUPPORT_MOSAIC_MODE
    /* MosaicMode: reset SclOut rect as the mosaic sub-window size */
    if(bMosaicMode)
    {
        hWindow->stAdjSclOut.lLeft    = 0;
        hWindow->stAdjSclOut.lLeft_R  = 0;
        hWindow->stAdjSclOut.lTop     = 0;
        hWindow->stAdjSclOut.ulWidth  = hWindow->stCurInfo.astMosaicRect[
            BVDC_P_MIN(ulRectIdx, hWindow->stCurInfo.ulMosaicCount - 1)].ulWidth;
        hWindow->stAdjSclOut.ulHeight = hWindow->stCurInfo.astMosaicRect[
            BVDC_P_MIN(ulRectIdx, hWindow->stCurInfo.ulMosaicCount - 1)].ulHeight;
    }
    else
#else
    BSTD_UNUSED(ulRectIdx);
#endif
    {
        hWindow->stAdjSclOut = hWindow->stCurInfo.stScalerOutput;
    }
    /* Round up to even number (PR48513) */
    hWindow->stAdjSclOut.ulWidth = BVDC_P_ALIGN_UP(hWindow->stAdjSclOut.ulWidth, 2);

    hWindow->stAdjDstRect = hWindow->stCurInfo.stDstRect;
    if((pMvdFieldData->ulSourceHorizontalSize >(hWindow->stCurInfo.stSrcClip.ulLeft + hWindow->stCurInfo.stSrcClip.ulRight + BVDC_P_SRC_INPUT_H_MIN)) &&
        !bMosaicMode)
    {
        hWindow->stSrcCnt.lLeft = hWindow->stCurInfo.stSrcClip.ulLeft;
        hWindow->stSrcCnt.lLeft_R = hWindow->stCurInfo.stSrcClip.ulLeft + hWindow->stCurInfo.stSrcClip.lLeftDelta_R;
        hWindow->stSrcCnt.ulWidth = pMvdFieldData->ulSourceHorizontalSize - (hWindow->stCurInfo.stSrcClip.ulLeft + hWindow->stCurInfo.stSrcClip.ulRight);
    }
    else
    {
        hWindow->stSrcCnt.lLeft = 0;
        hWindow->stSrcCnt.lLeft_R = 0;
        hWindow->stSrcCnt.ulWidth = pMvdFieldData->ulSourceHorizontalSize;
    }
    if((pMvdFieldData->ulSourceVerticalSize >(hWindow->stCurInfo.stSrcClip.ulTop + hWindow->stCurInfo.stSrcClip.ulBottom + BVDC_P_SRC_INPUT_V_MIN)) &&
        !bMosaicMode)
    {
        hWindow->stSrcCnt.lTop = hWindow->stCurInfo.stSrcClip.ulTop;
        hWindow->stSrcCnt.ulHeight = pMvdFieldData->ulSourceVerticalSize - (hWindow->stCurInfo.stSrcClip.ulTop + hWindow->stCurInfo.stSrcClip.ulBottom);
    }
    else
    {
        hWindow->stSrcCnt.lTop = 0;
        hWindow->stSrcCnt.ulHeight = pMvdFieldData->ulSourceVerticalSize;
    }

    /* when a mpeg src is connected to more than one window, eUseAllDestination with
     * mosaic mode could cause diff vertical content clipping --> bandwidth failure;
     * if only one window is using the src, it is fine to use eUseAllDestination with
     * mosaic mode, but we want to have the same effect no matter how many window
     * are connected to the src */
    bDoAspRatCorrect = (
        (!bMosaicMode) &&
        ((BVDC_AspectRatioMode_eUseAllDestination == hWindow->stCurInfo.eAspectRatioMode) ||
         (BVDC_AspectRatioMode_eUseAllSource == hWindow->stCurInfo.eAspectRatioMode)));

    bNonlinearScl = (
        !bDoAspRatCorrect &&
        ((0 != hWindow->stCurInfo.ulNonlinearSrcWidth) ||
         (0 != hWindow->stCurInfo.ulNonlinearSclOutWidth)));

    if(!bMosaicMode)
    {
        BVDC_P_Window_AdjDstCut_isr(hWindow, pDstFmtInfo->ulWidth, pDstFmtInfo->ulHeight,
            &hWindow->stSrcCnt, &hWindow->stAdjSclOut, &hWindow->stAdjDstRect);
    }

    if(bDoAspRatCorrect)
    {
        BVDC_P_CalcuPixelAspectRatio_isrsafe(
            pMvdFieldData->eAspectRatio,
            pMvdFieldData->uiSampleAspectRatioX,
            pMvdFieldData->uiSampleAspectRatioY,
            pMvdFieldData->ulDisplayHorizontalSize,
            pMvdFieldData->ulDisplayVerticalSize,
            &hWindow->stCurInfo.hSource->stCurInfo.stAspRatRectClip,
            &ulSrcPxlAspRatio, &ulSrcPxlAspRatio_x_y,
            pMvdFieldData->eOrientation);
        ulDspPxlAspRatio = hWindow->hCompositor->hDisplay->ulPxlAspRatio;
    }

    if (bMosaicMode || bDoAspRatCorrect || bNonlinearScl ||
        BVDC_P_EQ_DELTA(hWindow->stAdjSclOut.ulWidth,  pDstFmtInfo->ulWidth,  8) ||
        BVDC_P_EQ_DELTA(hWindow->stAdjSclOut.ulHeight, pDstFmtInfo->ulHeight, 8))
    {
        hWindow->ulStgPxlAspRatio_x_y = hWindow->hCompositor->hDisplay->ulPxlAspRatio_x_y;
    }
    else
    {
        if(0 == ulSrcPxlAspRatio)    /* non-0 iff already calculated */
        {
            BVDC_P_CalcuPixelAspectRatio_isrsafe(
                pMvdFieldData->eAspectRatio,
                pMvdFieldData->uiSampleAspectRatioX,
                pMvdFieldData->uiSampleAspectRatioY,
                pMvdFieldData->ulDisplayHorizontalSize,
                pMvdFieldData->ulDisplayVerticalSize,
                &hWindow->stCurInfo.hSource->stCurInfo.stAspRatRectClip,
                &ulSrcPxlAspRatio, &ulSrcPxlAspRatio_x_y,
                pMvdFieldData->eOrientation);
        }
        BVDC_P_ScaleSrcPxlAspRatio_isr(
                ulSrcPxlAspRatio_x_y,
                hWindow->stSrcCnt.ulWidth, hWindow->stSrcCnt.ulHeight,
                hWindow->stAdjSclOut.ulWidth, hWindow->stAdjSclOut.ulHeight,
                &hWindow->ulStgPxlAspRatio_x_y );
    }

    /* ulNrmHrz/VrtSrcStep needed in AutoAdjRelatedToSclRatio_isr */
    hWindow->ulNrmHrzSrcStep = BVDC_P_CAL_HRZ_SRC_STEP(
        hWindow->stSrcCnt.ulWidth, hWindow->stAdjSclOut.ulWidth);
    hWindow->ulNrmVrtSrcStep = BVDC_P_CAL_VRT_SRC_STEP(
        hWindow->stSrcCnt.ulHeight, hWindow->stAdjSclOut.ulHeight, hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced);

    if((bDoAspRatCorrect) ||
       (((0 != hWindow->stCurInfo.ulHrzSclFctRndToler) ||
         (0 != hWindow->stCurInfo.ulVrtSclFctRndToler)) &&
         !bMosaicMode && !bNonlinearScl))
    {
        BVDC_P_Window_AutoAdjRelatedToSclRatio_isr(hWindow, bMosaicMode,
            ulSrcPxlAspRatio, ulDspPxlAspRatio,
            pMvdFieldData->ulSourceHorizontalSize, pMvdFieldData->ulSourceVerticalSize, true);
    }

    hWindow->ulNonlinearSrcWidth = 0;
    hWindow->ulNonlinearSclOutWidth = 0;
    hWindow->ulCentralRegionSclOutWidth = BVDC_P_OVER_CENTER_WIDTH;

    /* ------------------------------------------------------------------*/
    /* support offscreen                                            */
    /* ------------------------------------------------------------------*/
    if(!bMosaicMode)
    {
        BVDC_P_Window_AdjOffscreen_isr(hWindow, pDstFmtInfo,
                &hWindow->stSrcCnt, &hWindow->stAdjSclOut, &hWindow->stAdjDstRect);
    }

    /* bAdjRectsDirty is set to true by BVDC_P_Window_ApplyChanges_isr and
     * BVDC_P_Window_ValidateRects_isr, or something changes here.
     * It will get clear once the rectangle are used,
     * BVDC_P_Window_UpdateSrcAndUserInfo_isr(). */
    hWindow->bAdjRectsDirty = false;
    hWindow->stCurInfo.stDirty.stBits.bRecAdjust = BVDC_P_DIRTY;
    hWindow->stCurInfo.stDirty.stBits.bReDetVnet = BVDC_P_DIRTY;

    /* The source also need to adjust it output rect. */
    hWindow->stCurInfo.hSource->stCurInfo.stDirty.stBits.bRecAdjust = BVDC_P_DIRTY;

    BDBG_LEAVE(BVDC_P_Window_InitMuteRec_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * This function is the root frunction to transfer the 3 user set window's
 * rectangles into the window's "adjusted rectangles", and then adjust them.
 *
 * The "adjusted rectangles" are the ones really used to build picture node
 * and RULs.
 *
 * The adjustment is applied in the order of box black patch cut, pan scan,
 * user's src clip, aspect ratio correctio, and scale factor.
 *
 * This func is called at every vsync.
 *
 * Note:
 *   - The pan/scan can only moves source content rectangle within the source
 *     boundary.
 *   - when "over-clipped" case, we will ignore the user source clip.
 *   - _isr rectangles setting could be done at every vsync.
 *   - (pMvdFieldData and pXvdFieldData are NULL) implies it's an analog source.
 *   - For MosaicMode, we don't wanna cut destination sub-rectangles in any kind,
 *     e.g. AllSource aspec ratio, scale factor rounding;
 *     therefore, don't adjust DstRect; also AdjSclOut is actual sub-rectangle;
 */
void BVDC_P_Window_AdjustRectangles_isr
    ( const BVDC_Window_Handle         hWindow,
      const BAVC_MVD_Field            *pMvdFieldData,
      const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
      uint32_t                         ulRectIdx )
{
    BVDC_P_Window_Info       *pUserInfo;
    const BVDC_P_Source_Info *pSrcInfo;
    const BFMT_VideoInfo     *pDstFmtInfo;
    BVDC_P_Rect stSrcCnt; /* src clip rect after box cut, pan-scan and user clip */
    uint32_t  ulSrcWidth, ulSrcHeight; /* full src size */
    uint32_t  ulSrcDispWidth, ulSrcDispHeight; /* pan scan disp size */
    const BVDC_P_ClipRect *pSrcAspRatRectClip;
    bool  bCaptureCrc = false;
    BVDC_Compositor_Handle       hCompositor;
    BVDC_P_Window_AfdSettings   *pAfdSettings;
    BVDC_P_Window_BarSettings   *pBarSettings;
    BVDC_P_Window_PanScanSettings   *pPanScanSettings;
    BVDC_P_Window_AspRatioSettings  *pAspRatioSettings;

    BDBG_ENTER(BVDC_P_Window_AdjustRectangles_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    /* Get user info */
    pSrcInfo  = &hWindow->stCurInfo.hSource->stCurInfo;
    pUserInfo = &hWindow->stCurInfo;
    hCompositor = hWindow->hCompositor;
    pDstFmtInfo = hCompositor->stCurInfo.pFmtInfo;

#if BVDC_P_SUPPORT_STG
    /* if stg resol ramp transition */
    if(hCompositor->hDisplay->stCurInfo.ulResolutionRampCount)
    {
        pUserInfo->stDstRect.ulWidth  = hCompositor->hDisplay->astStgRampWinDst[hWindow->eId].ulWidth / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pUserInfo->stDstRect.ulHeight = hCompositor->hDisplay->astStgRampWinDst[hWindow->eId].ulHeight/ BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pUserInfo->stDstRect.lLeft = hCompositor->hDisplay->astStgRampWinDst[hWindow->eId].lLeft / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pUserInfo->stDstRect.lTop  = hCompositor->hDisplay->astStgRampWinDst[hWindow->eId].lTop  / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pUserInfo->stScalerOutput.ulWidth  = hCompositor->hDisplay->astStgRampWinSclOut[hWindow->eId].ulWidth / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pUserInfo->stScalerOutput.ulHeight = hCompositor->hDisplay->astStgRampWinSclOut[hWindow->eId].ulHeight/ BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pUserInfo->stScalerOutput.lLeft = hCompositor->hDisplay->astStgRampWinSclOut[hWindow->eId].lLeft / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pUserInfo->stScalerOutput.lTop  = hCompositor->hDisplay->astStgRampWinSclOut[hWindow->eId].lTop  / BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        BDBG_MSG(("Win%u halved output size due to resol ramp[%u]!", hWindow->eId, hCompositor->hDisplay->stCurInfo.ulResolutionRampCount));
    }
#endif

    /* Init Afd settings */
    pAfdSettings = &hWindow->stAfdSettings;
    pAfdSettings->bDoAfd = false;
    pAfdSettings->ulAfdVal = 0;

    /* Init Bar settings */
    pBarSettings = &hWindow->stBarSettings;
    pBarSettings->bDoBar = false;
    pBarSettings->ulTopLeftBarValue = 0;
    pBarSettings->ulBotRightBarValue = 0;
    pBarSettings->eBarDataType = BAVC_BarDataType_eInvalid;

    /* Pan Scan */
    pPanScanSettings = &hWindow->stPanScanSettings;

    /* Aspect Ratio */
    pAspRatioSettings = &hWindow->stAspRatioSettings;
    pAspRatioSettings->ulSrcPxlAspRatio_x_y = 0;
    pAspRatioSettings->uiSampleAspectRatioX = 0;
    pAspRatioSettings->uiSampleAspectRatioY = 0;
    /* become non-zero after calculated */
    pAspRatioSettings->ulSrcPxlAspRatio = 0;
    pAspRatioSettings->ulDspPxlAspRatio = hCompositor->hDisplay->ulPxlAspRatio;
    BDBG_ASSERT(pAspRatioSettings->ulDspPxlAspRatio != 0);

    /* --------- Collect calcu-related info from each src type ------ */
    pSrcAspRatRectClip = &hWindow->stCurInfo.hSource->stCurInfo.stAspRatRectClip;
    if(BVDC_P_SRC_IS_ITU656(hWindow->stCurInfo.hSource->eId))
    {
        const BFMT_VideoInfo  *pSrcFmtInfo;

        pSrcFmtInfo = hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo;
        ulSrcWidth  = pSrcFmtInfo->ulWidth;
        ulSrcHeight = pSrcFmtInfo->ulHeight;

        /* Adjust for oversample */
        ulSrcWidth  = BVDC_P_OVER_SAMPLE(hWindow->stCurInfo.hSource->ulSampleFactor, ulSrcWidth);

        /* only used for aspect ratio correction */
        pAspRatioSettings->eSrcAspectRatio = hWindow->stCurInfo.hSource->stCurInfo.eAspectRatio;

        /* user afd */
        pAfdSettings->bDoAfd = (pUserInfo->stAfdSettings.eMode == BVDC_AfdMode_eUser);
        pAfdSettings->ulAfdVal = pUserInfo->stAfdSettings.ulAfd;

        /* user pan scan set-up */
        pPanScanSettings->bDoPanScan =
            (BVDC_PanScanType_eUser == pUserInfo->ePanScanType) && !pAfdSettings->bDoAfd;
        ulSrcDispWidth  = ulSrcWidth;
        ulSrcDispHeight = ulSrcHeight;
        pPanScanSettings->lHorizontalPanScan = 0;
        pPanScanSettings->lVerticalPanScan   = 0;
    }
    else if(BVDC_P_SRC_IS_HDDVI(hWindow->stCurInfo.hSource->eId))
    {
        /* user afd */
        pAfdSettings->bDoAfd = (pUserInfo->stAfdSettings.eMode == BVDC_AfdMode_eUser);
        pAfdSettings->ulAfdVal = pUserInfo->stAfdSettings.ulAfd;

        pPanScanSettings->bDoPanScan =
            (BVDC_PanScanType_eDisable != pUserInfo->ePanScanType) && !pAfdSettings->bDoAfd;

        BDBG_ASSERT(pXvdFieldData);
        ulSrcWidth  = pXvdFieldData->ulSourceHorizontalSize;
        ulSrcHeight = pXvdFieldData->ulSourceVerticalSize;

        /* only used for aspect ratio correction */
        pAspRatioSettings->eSrcAspectRatio      = pXvdFieldData->eAspectRatio;
        pAspRatioSettings->uiSampleAspectRatioX = pXvdFieldData->uiSampleAspectRatioX;
        pAspRatioSettings->uiSampleAspectRatioY = pXvdFieldData->uiSampleAspectRatioY;

        /* only used for pan scan */
        ulSrcDispWidth     = pXvdFieldData->ulDisplayHorizontalSize;
        ulSrcDispHeight    = pXvdFieldData->ulDisplayVerticalSize;
        pPanScanSettings->lHorizontalPanScan = pXvdFieldData->i32_HorizontalPanScan;
        pPanScanSettings->lVerticalPanScan   = pXvdFieldData->i32_VerticalPanScan;

#if BFMT_LEGACY_3DTV_SUPPORT
        if(BFMT_IS_3D_MODE(pSrcInfo->pFmtInfo->eVideoFmt)) /* 3D format */
        {
            /* HDDVI sends Right frame (bot half) plus some scrap
               area of the 3D format through DE shift; */
            if(pSrcInfo->stHdDviSetting.bEnableDe &&
                pSrcInfo->bHVStartOverride)
            {
                /* 1080p3D has 2205-line active region;
                   DE shift by 1023-line (10-bit register) where the first
                   41-line is VBI (VSYNC_WIDTH + V_BackPorch); */
                ulSrcHeight -= (pSrcInfo->ulVstart - pSrcInfo->pFmtInfo->ulTopActive + 1);
            }
            /* HDDVI sends Left frame (top half) of the 3D format */
            else
            {
                ulSrcHeight = pSrcInfo->pFmtInfo->ulDigitalHeight;
            }
            ulSrcDispHeight = ulSrcHeight;
        }
#endif

        /* bOrientationOverride only valid when original
         * orientation from FMT is 2D */
        if(pSrcInfo->pFmtInfo->eOrientation == BFMT_Orientation_e2D)
        {
            if(pSrcInfo->bOrientationOverride)
            {
                if(pSrcInfo->eOrientation == BFMT_Orientation_e3D_LeftRight)
                {
                    ulSrcWidth = ulSrcWidth / 2;
                }
                else if(pSrcInfo->eOrientation == BFMT_Orientation_e3D_OverUnder)
                {
                    ulSrcHeight = ulSrcHeight / 2;
                }
            }
        }
    }
    else if (BVDC_P_SRC_IS_VFD(hWindow->stCurInfo.hSource->eId))
    {
        BVDC_P_SurfaceInfo  *pCurSurface;

        pCurSurface = &hWindow->stCurInfo.hSource->hVfdFeeder->stGfxSurface.stCurSurInfo;
        pAspRatioSettings->eSrcAspectRatio = BFMT_AspectRatio_eSquarePxl;
        ulSrcDispWidth = pCurSurface->ulWidth;
        ulSrcDispHeight = pCurSurface->ulHeight;
        pPanScanSettings->bDoPanScan = false;
        pPanScanSettings->lHorizontalPanScan = 0;
        pPanScanSettings->lVerticalPanScan = 0;
        bCaptureCrc = false;
        ulSrcWidth  = ulSrcDispWidth;
        ulSrcHeight = ulSrcDispHeight;
    }
    else /* mpeg video source */
    {
        BDBG_ASSERT(pMvdFieldData);

        pAfdSettings->ulAfdVal = (pUserInfo->stAfdSettings.eMode == BVDC_AfdMode_eStream)
            ? pMvdFieldData->ulAfd : pUserInfo->stAfdSettings.ulAfd;

        pAfdSettings->bDoAfd = ((pUserInfo->stAfdSettings.eMode == BVDC_AfdMode_eUser) ||
                  ((pUserInfo->stAfdSettings.eMode == BVDC_AfdMode_eStream) &&
                  (!BVDC_P_WINDOW_ISBARDATA(pAfdSettings->ulAfdVal, pMvdFieldData->eBarDataType)) &&
                   (pMvdFieldData->bValidAfd)));

        pBarSettings->bDoBar = ((!pAfdSettings->bDoAfd) && (pMvdFieldData->bValidAfd) &&
            (pUserInfo->stAfdSettings.eMode == BVDC_AfdMode_eStream)   &&
            (BVDC_P_WINDOW_ISBARDATA(pAfdSettings->ulAfdVal, pMvdFieldData->eBarDataType )));

        pPanScanSettings->bDoPanScan = (BVDC_PanScanType_eDisable != pUserInfo->ePanScanType) &&
            !pAfdSettings->bDoAfd && !pBarSettings->bDoBar;

        ulSrcWidth  = pMvdFieldData->ulSourceHorizontalSize;
        ulSrcHeight = pMvdFieldData->ulSourceVerticalSize;

        /* only used for aspect ratio correction */
        pAspRatioSettings->eSrcAspectRatio      = pMvdFieldData->eAspectRatio;
        pAspRatioSettings->uiSampleAspectRatioX = pMvdFieldData->uiSampleAspectRatioX;
        pAspRatioSettings->uiSampleAspectRatioY = pMvdFieldData->uiSampleAspectRatioY;

        /* only used for Bar Data */
        pBarSettings->eBarDataType       = pMvdFieldData->eBarDataType;
        pBarSettings->ulBotRightBarValue = pMvdFieldData->ulBotRightBarValue;
        pBarSettings->ulTopLeftBarValue  = pMvdFieldData->ulTopLeftBarValue;

        /* only used for pan scan */
        ulSrcDispWidth     = pMvdFieldData->ulDisplayHorizontalSize;
        ulSrcDispHeight    = pMvdFieldData->ulDisplayVerticalSize;
        pPanScanSettings->lHorizontalPanScan = pMvdFieldData->i32_HorizontalPanScan;
        pPanScanSettings->lVerticalPanScan   = pMvdFieldData->i32_VerticalPanScan;

        /* Capture CRC */
        bCaptureCrc        = pMvdFieldData->bCaptureCrc;
    }

    /* in case bad data */
    ulSrcDispWidth  = (BVDC_P_SRC_INPUT_H_MIN > ulSrcDispWidth)?  ulSrcWidth:  ulSrcDispWidth;
    ulSrcDispHeight = (BVDC_P_SRC_INPUT_V_MIN > ulSrcDispHeight)? ulSrcHeight: ulSrcDispHeight;
    ulSrcDispWidth  = BVDC_P_MIN(ulSrcWidth, ulSrcDispWidth);
    ulSrcDispHeight = BVDC_P_MIN(ulSrcHeight, ulSrcDispHeight);

    /* Handle unknown source aspect ratio */
    BVDC_P_SetDefaultAspRatio_isrsafe(
        &pAspRatioSettings->eSrcAspectRatio,
        pAspRatioSettings->uiSampleAspectRatioX,
        pAspRatioSettings->uiSampleAspectRatioY,
        ulSrcDispWidth, ulSrcDispHeight);

    /* (0) Init stSrcCnt: prepare for box cut, pan scan and user's clip */
    stSrcCnt.lLeft    = 0;
    stSrcCnt.lTop     = 0;
    stSrcCnt.lLeft_R  = 0;
    stSrcCnt.ulWidth  = ulSrcWidth;
    stSrcCnt.ulHeight = ulSrcHeight;

    /* The following operations will make adjustment to dst and scl, and
     * produce:
     * hWindow->stSrcCnt
     * hWindow->stAdjSclOut
     * hWindow->stAdjDstRect
     *
     * For a source, the union of the stSrcCnt of all windows shared the
     * source will be store in:
     * hWindow->stCurInfo.hSource->stScanOut
     *
     * Note the adjustment does not always involve cutting, in the case off
     * enable VBI pass-thru we'd added more lines. */
    /* --------------------------------------------------------------- */
    /* (1) do box detect and auto black patch cut                      */
    /* --------------------------------------------------------------- */
    if(hWindow->stCurResource.hBoxDetect && !pAfdSettings->bDoAfd && !pBarSettings->bDoBar)
    {
        BVDC_P_Window_BoxCut_isr(hWindow, &stSrcCnt, pPanScanSettings->bDoPanScan);
    }

    /* --------------------------------------------------------------- */
    /* (2) do pan scan (src cut)                                       */
    /* --------------------------------------------------------------- */
    if(pPanScanSettings->bDoPanScan)
    {
        /* for user pan scan type, use auto-calculated SrcDisp size if it is not available */
        if((BVDC_PanScanType_eUser == pUserInfo->ePanScanType) &&
           (ulSrcWidth  == ulSrcDispWidth) &&
           (ulSrcHeight == ulSrcDispHeight))
        {
            /* reclculate the "auto-calculated SrcDisp size" only if we have to */
            if((hWindow->ulFieldSrcWidth  != ulSrcWidth)      ||
               (hWindow->ulFieldSrcHeight != ulSrcHeight)     ||
               (hWindow->eFieldSrcAspectR != pAspRatioSettings->eSrcAspectRatio) ||
               (pAspRatioSettings->uiPrevSampleAspectRatioX != pAspRatioSettings->uiSampleAspectRatioX) ||
               (pAspRatioSettings->uiPrevSampleAspectRatioY != pAspRatioSettings->uiSampleAspectRatioY) ||
               (hWindow->bAdjRectsDirty)                      ||
               (hWindow->stCurInfo.hSource->bPictureChanged))
            {
                hWindow->ulFieldSrcWidth  = ulSrcWidth;
                hWindow->ulFieldSrcHeight = ulSrcHeight;
                hWindow->eFieldSrcAspectR = pAspRatioSettings->eSrcAspectRatio;
                pAspRatioSettings->uiPrevSampleAspectRatioX = pAspRatioSettings->uiSampleAspectRatioX;
                pAspRatioSettings->uiPrevSampleAspectRatioY = pAspRatioSettings->uiSampleAspectRatioY;

                /* note: CalcuAutoDisplaySize will change the dispSize, it will affect
                 * Src Aspect Ratio. However ulSrcPxlAspRatio is not to be re-calculated
                 * and eSrcAspectRatio is no longer used */
                BVDC_P_CalcuPixelAspectRatio_isrsafe(
                    pAspRatioSettings->eSrcAspectRatio,
                    pAspRatioSettings->uiSampleAspectRatioX,
                    pAspRatioSettings->uiSampleAspectRatioY,
                    ulSrcDispWidth, ulSrcDispHeight,
                    pSrcAspRatRectClip, &pAspRatioSettings->ulSrcPxlAspRatio,
                    &pAspRatioSettings->ulSrcPxlAspRatio_x_y,
                    (NULL == pMvdFieldData)?BFMT_Orientation_e2D:pMvdFieldData->eOrientation);
                BVDC_P_Window_CalcuAutoDisplaySize_isr(hWindow, pAspRatioSettings->ulSrcPxlAspRatio,
                    pAspRatioSettings->ulDspPxlAspRatio);
            }
            ulSrcDispWidth  = hWindow->ulAutoDispHorizontalSize;
            ulSrcDispHeight = hWindow->ulAutoDispVerticalSize;
        }

        /* perform pan scan */
        BVDC_P_Window_PanScan_isr(hWindow, &stSrcCnt, ulSrcDispWidth,
            ulSrcDispHeight, pPanScanSettings->lHorizontalPanScan,
            pPanScanSettings->lVerticalPanScan);
    }

    /* Note:
     * 1). It is quite likely the above clipings result in the same as in last
     *   vsync, and aspect ratio correction and scale factor rounding are
     *   expensive, therefore it is worthwhile to optimize.
     * 2). The following could adjust pSrcCnt or pAdjDstRect/pAdjSclOut.
     * 3). We should not introduce field invesion when we adjust the rectangles,
     *   that means all height offset we introduced should be multiple of 2
     * 4). bAdjRectsDirty is set to true by BVDC_P_Window_ApplyChanges_isr and
     * BVDC_P_Window_ValidateRects_isr */
    if((hWindow->bAdjRectsDirty) ||
       (hWindow->stCurInfo.hSource->bPictureChanged) ||
       (!BVDC_P_RECT_CMP_EQ(&hWindow->stPrevSrcCnt, &stSrcCnt))
#if BVDC_P_SUPPORT_MOSAIC_MODE
       ||
       (pUserInfo->bMosaicMode &&
        ((pUserInfo->astMosaicRect[ulRectIdx].ulWidth != hWindow->stAdjSclOut.ulWidth)||
        (pUserInfo->astMosaicRect[ulRectIdx].ulHeight!= hWindow->stAdjSclOut.ulHeight)))
#endif
         )
    {
        /* for optimization in future vsync */
        hWindow->stSrcCnt     = stSrcCnt;
        hWindow->stAdjDstRect = pUserInfo->stDstRect;

        BDBG_MSG(("--------------------------"));
        BVDC_P_PRINT_RECT("New stSrcCnt", &hWindow->stSrcCnt, false, 0, 0);
        BDBG_MSG(("Window[%d] usr change = %d, src change = %d, srcPol=%d",
            hWindow->eId, hWindow->bAdjRectsDirty,
            !BVDC_P_RECT_CMP_EQ(&hWindow->stPrevSrcCnt, &stSrcCnt),
            ((NULL == pMvdFieldData) ? 99 : pMvdFieldData->eSourcePolarity)));

        /* for optimization in future vsync */
        hWindow->stPrevSrcCnt = stSrcCnt;

        /* init scale factor as 0 to indicate it needs re-compute */
        hWindow->ulNrmHrzSrcStep = 0;
        hWindow->ulNrmVrtSrcStep = 0;
        hWindow->ulNonlinearSrcWidth = 0;
        hWindow->ulNonlinearSclOutWidth = 0;
        hWindow->ulCentralRegionSclOutWidth = BVDC_P_OVER_CENTER_WIDTH;

        /* --------------------------------------------------------------- */
        /* (3) do AFd (src cut)                                            */
        /* --------------------------------------------------------------- */
        if(pBarSettings->bDoBar)
        {
            BVDC_P_Window_Bar_isr(hWindow, &hWindow->stSrcCnt, pBarSettings->eBarDataType,
                pBarSettings->ulTopLeftBarValue, pBarSettings->ulBotRightBarValue);
        }
        if(pAfdSettings->bDoAfd)
        {
            BVDC_P_Window_Afd_isr(hWindow, &hWindow->stSrcCnt, pAfdSettings->ulAfdVal,
                pAspRatioSettings->eSrcAspectRatio,
                pAspRatioSettings->uiSampleAspectRatioX,
                pAspRatioSettings->uiSampleAspectRatioY, ulSrcWidth, ulSrcHeight);
        }

#if BVDC_P_SUPPORT_MOSAIC_MODE
        /* MosaicMode: reset SclOut rect as the mosaic sub-window size */
        if(pUserInfo->bMosaicMode)
        {
            hWindow->stAdjSclOut.lLeft    = 0;
            hWindow->stAdjSclOut.lLeft_R  = 0;
            hWindow->stAdjSclOut.lTop     = 0;
            hWindow->stAdjSclOut.ulWidth  = pUserInfo->astMosaicRect[
                BVDC_P_MIN(ulRectIdx, pUserInfo->ulMosaicCount - 1)].ulWidth;
            hWindow->stAdjSclOut.ulHeight = pUserInfo->astMosaicRect[
                BVDC_P_MIN(ulRectIdx, pUserInfo->ulMosaicCount - 1)].ulHeight;
            /* SW7231-1061 height needs to be even for interlace fmt */
            if(pDstFmtInfo->bInterlaced)
                hWindow->stAdjSclOut.ulHeight &= ~1;

#if (BVDC_P_WIN_MOSAIC_MODE_BVN_WORKAROUND)
            hWindow->stAdjSclOut.ulWidth = BVDC_P_MAX(hWindow->stAdjSclOut.ulWidth,
                BVDC_P_WIN_MOSAIC_OUTPUT_H_MIN);
            hWindow->stAdjSclOut.ulHeight = BVDC_P_MAX(hWindow->stAdjSclOut.ulHeight,
                BVDC_P_WIN_MOSAIC_OUTPUT_V_MIN);
#endif
        }
        else
#else
        BSTD_UNUSED(ulRectIdx);
#endif
        {
            hWindow->stAdjSclOut  = pUserInfo->stScalerOutput;
        }

        /* when a mpeg src is connected to more than one window, eUseAllDestination with
         * mosaic mode could cause diff vertical content clipping --> bandwidth failure;
         * if only one window is using the src, it is fine to use eUseAllDestination with
         * mosaic mode, but we want to have the same effect no matter how many window
         * are connected to the src */
        pAspRatioSettings->bDoAspRatCorrect = (
            (!pUserInfo->bMosaicMode) &&
            ((BVDC_AspectRatioMode_eUseAllDestination == hWindow->stCurInfo.eAspectRatioMode) ||
             (BVDC_AspectRatioMode_eUseAllSource == hWindow->stCurInfo.eAspectRatioMode)));

        /* According to mpeg spec the full aspect ratio is fully contributed by the
               * ulSrcDispWidth x ulSrcDispHeight area */
        if((0 == pAspRatioSettings->ulSrcPxlAspRatio) &&    /* non-0 iff already calculated */
           ((pAspRatioSettings->bDoAspRatCorrect) ||
           (0!=pUserInfo->ulNonlinearSrcWidth)||
           (0!=pUserInfo->ulNonlinearSclOutWidth)))
        {
            BVDC_P_CalcuPixelAspectRatio_isrsafe(
                pAspRatioSettings->eSrcAspectRatio,
                pAspRatioSettings->uiSampleAspectRatioX,
                pAspRatioSettings->uiSampleAspectRatioY,
                ulSrcDispWidth, ulSrcDispHeight,
                pSrcAspRatRectClip, &pAspRatioSettings->ulSrcPxlAspRatio,
                &pAspRatioSettings->ulSrcPxlAspRatio_x_y,
                (NULL == pMvdFieldData)?BFMT_Orientation_e2D:pMvdFieldData->eOrientation);
        }
        pAspRatioSettings->bNonlinearScl = (!pAspRatioSettings->bDoAspRatCorrect &&
                 (pAspRatioSettings->ulSrcPxlAspRatio!=pAspRatioSettings->ulDspPxlAspRatio)&&
                         ((0 != pUserInfo->ulNonlinearSrcWidth) ||
                          (0 != pUserInfo->ulNonlinearSclOutWidth)));

        /* --------------------------------------------------------------- */
        /* (4) do user's src clip setting, including setting by isr        */
        /* --------------------------------------------------------------- */
        /* note: _isr rectangles setting could be done at every vsync */
        if(!pUserInfo->bMosaicMode)
        {
            BVDC_P_Window_UserSrcCut_isr(hWindow, &hWindow->stSrcCnt, &pUserInfo->stSrcClip);
        }

        /* ------------------------------------------------------------------------
         * (5) calculate scale factor before xfering dst cut into src cut for accuracy
         * note: ulWidth and ulHeight are only used to decide feed/cap/scl_in/out
         * size, and they are always aligned due to HW limitation and aligned up to
         * avoid video info loose. The pixel + subpixel accuracy is passed to SCL by
         * lLeft/lTop for phase and ulNrmHrzSrcStep/ulNrmVrtSrcStep for src step.
         * ------------------------------------------------------------------------ */
        hWindow->ulNrmHrzSrcStep = BVDC_P_CAL_HRZ_SRC_STEP(
            hWindow->stSrcCnt.ulWidth, hWindow->stAdjSclOut.ulWidth);

        hWindow->ulNrmVrtSrcStep = BVDC_P_CAL_VRT_SRC_STEP(
            hWindow->stSrcCnt.ulHeight, hWindow->stAdjSclOut.ulHeight, hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced);

        /* --------------------------------------------------------------- */
        /* (6) transfer large dest cut into src cut                        */
        /* --------------------------------------------------------------- */
        /* note: AdjDstCut_isr must be done before aspect ratio correction,
         * because the later will change scale ratio, therefore how dst cut
         * is xfered into src clip, and please notice that aspect ratio
         * correction does not cause new dst cut */
        if(!pUserInfo->bMosaicMode)
        {
            BVDC_P_Window_AdjDstCut_isr(hWindow, pDstFmtInfo->ulWidth, pDstFmtInfo->ulHeight,
                &hWindow->stSrcCnt, &hWindow->stAdjSclOut, &hWindow->stAdjDstRect);
        }

        /* ---------------------------------------------------------------
         * (7) do aspect ratio correction and scale factor rounding
         * --------------------------------------------------------------- */
        if((pAspRatioSettings->bDoAspRatCorrect) ||
           (((0 != pUserInfo->ulHrzSclFctRndToler) ||
             (0 != pUserInfo->ulVrtSclFctRndToler)) && !pUserInfo->bMosaicMode &&
             !pAspRatioSettings->bNonlinearScl))
        {
#if (BVDC_P_SHOW_ASP_RAT_MSG==1)
            if (pAspRatioSettings->bDoAspRatCorrect)
            {
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("--- AspR correct Win Id %d, Src Id %d:",  hWindow->eId, hWindow->stCurInfo.hSource->eId));
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("Src: eAspR %d, (W %d, H %d), CnvsClp (L %d, R %d, T %d, B %d), ulPxlAspR 0x%Lx",
                    pAspRatioSettings->eSrcAspectRatio, ulSrcDispWidth, ulSrcDispHeight,
                    pSrcAspRatRectClip->ulLeft, pSrcAspRatRectClip->ulRight,
                    pSrcAspRatRectClip->ulTop, pSrcAspRatRectClip->ulBottom,
                    pAspRatioSettings->ulSrcPxlAspRatio));
                BDBG_MODULE_MSG(BVDC_ASP_RAT,("Dsp: eAspR %d, (W %d, H %d), CnvsClp (L %d, R %d, T %d, B %d), ulPxlAspR 0x%Lx",
                    hWindow->hCompositor->stCurInfo.eAspectRatio,
                    pDstFmtInfo->ulWidth, pDstFmtInfo->ulHeight,
                    hWindow->hCompositor->stCurInfo.stAspRatRectClip.ulLeft, hWindow->hCompositor->stCurInfo.stAspRatRectClip.ulRight,
                    hWindow->hCompositor->stCurInfo.stAspRatRectClip.ulTop, hWindow->hCompositor->stCurInfo.stAspRatRectClip.ulBottom,
                    ulDspPxlAspRatio));
            }
#endif
            BVDC_P_Window_AutoAdjRelatedToSclRatio_isr(hWindow, pUserInfo->bMosaicMode,
                pAspRatioSettings->ulSrcPxlAspRatio, pAspRatioSettings->ulDspPxlAspRatio,
                ulSrcWidth, ulSrcHeight, false);
        }

        /* ---------------------------------------------------------------
         * (8) non-linear scaling central region should be computed after
         * BVDC_P_Window_AdjDstCut_isr.
         * Note: non-linear scaling doesn't coexist with overall ARC/SFR;
         * --------------------------------------------------------------- */
        if(pAspRatioSettings->bNonlinearScl)
        {
            BVDC_P_Window_CentralRegionWidth_isr(hWindow,
                pAspRatioSettings->ulSrcPxlAspRatio, pAspRatioSettings->ulDspPxlAspRatio);
        }

        /* --------------------------------------------------------------- */
        /* (9) Bypass without scaler adjustment                            */
        /* --------------------------------------------------------------- */
        if((hWindow->eId == BVDC_P_WindowId_eComp2_V0) &&
            (!hWindow->stCurResource.hScaler))
        {
            /* No scaler for bypass. Need to mute video if source size
             * does not match display format */
            if ((hWindow->stSrcCnt.ulWidth == pDstFmtInfo->ulDigitalWidth) &&
                BVDC_P_EQ_DELTA(hWindow->stSrcCnt.ulHeight, pDstFmtInfo->ulDigitalHeight, 3))
            {
                hWindow->bMuteBypass = false;
            }
            else
            {
                BDBG_MSG(("Bypass Window[%d] Src & Dst mismatch w/o scaler", hWindow->eId));
                BDBG_MSG(("Window[%d] SrcCnt: %d, %d", hWindow->eId,
                    hWindow->stSrcCnt.ulWidth, hWindow->stSrcCnt.ulHeight));
                BDBG_MSG(("Window[%d] DstFmt: %d, %d", hWindow->eId,
                    pDstFmtInfo->ulWidth, pDstFmtInfo->ulHeight));

                /* Reset stSrcCnt to MIN to minimize capture. */
                hWindow->stSrcCnt.ulWidth  = BVDC_P_SRC_INPUT_H_MIN;
                hWindow->stSrcCnt.ulHeight = BVDC_P_SRC_INPUT_V_MIN;

                /* Mute video */
                hWindow->bMuteBypass = true;
                BDBG_MSG(("Mute bypass Window[%d]", hWindow->eId));
            }
        }

        /* ------------------------------------------------------------------*/
        /* (10) Must not violate HW limiations.                              */
        /* scan out the minimal size can be supported.                       */
        /* ------------------------------------------------------------------*/
        if((hWindow->stSrcCnt.ulWidth  < BVDC_P_SRC_INPUT_H_MIN) && !bCaptureCrc)
        {
            int32_t  lLeftAdj;

            /* Adjust clipping */
            lLeftAdj = BVDC_P_SRC_INPUT_H_MIN - hWindow->stSrcCnt.ulWidth;
            hWindow->stSrcCnt.lLeft = BVDC_P_MAX(0,
                hWindow->stSrcCnt.lLeft - (lLeftAdj << BVDC_P_16TH_PIXEL_SHIFT));
            hWindow->stSrcCnt.lLeft_R = BVDC_P_MAX(0,
                hWindow->stSrcCnt.lLeft_R - (lLeftAdj << BVDC_P_16TH_PIXEL_SHIFT));
            hWindow->stSrcCnt.ulWidth  = BVDC_P_SRC_INPUT_H_MIN;

            hWindow->stAdjDstRect      = pUserInfo->stDstRect;
        }
        if((hWindow->stSrcCnt.ulHeight < BVDC_P_SRC_INPUT_V_MIN) && !bCaptureCrc)
        {
            int32_t  lTopAdj;

            /* Adjust clipping */
            lTopAdj = BVDC_P_SRC_INPUT_V_MIN - hWindow->stSrcCnt.ulHeight;
            hWindow->stSrcCnt.lTop = BVDC_P_MAX(0,
                hWindow->stSrcCnt.lTop - (lTopAdj << BVDC_P_16TH_PIXEL_SHIFT));
            hWindow->stSrcCnt.ulHeight = BVDC_P_SRC_INPUT_V_MIN;

            hWindow->stAdjDstRect      = pUserInfo->stDstRect;
        }

        /* ------------------------------------------------------------------*/
        /* (11) calculate STG needed pixel aspect ratio                      */
        /* ------------------------------------------------------------------*/
        if (pUserInfo->bMosaicMode || pAspRatioSettings->bDoAspRatCorrect ||
            pAspRatioSettings->bNonlinearScl)
        {
            hWindow->ulStgPxlAspRatio_x_y = hCompositor->hDisplay->ulPxlAspRatio_x_y;
        }
        else
        {
            if(0 == pAspRatioSettings->ulSrcPxlAspRatio)    /* non-0 iff already calculated */
            {
                BVDC_P_CalcuPixelAspectRatio_isrsafe(
                    pAspRatioSettings->eSrcAspectRatio,
                    pAspRatioSettings->uiSampleAspectRatioX,
                    pAspRatioSettings->uiSampleAspectRatioY,
                    ulSrcDispWidth, ulSrcDispHeight,
                    pSrcAspRatRectClip, &pAspRatioSettings->ulSrcPxlAspRatio,
                    &pAspRatioSettings->ulSrcPxlAspRatio_x_y,
                    (NULL == pMvdFieldData)?BFMT_Orientation_e2D:pMvdFieldData->eOrientation);
            }
            BVDC_P_ScaleSrcPxlAspRatio_isr(
                pAspRatioSettings->ulSrcPxlAspRatio_x_y,
                hWindow->stSrcCnt.ulWidth, hWindow->stSrcCnt.ulHeight,
                hWindow->stAdjSclOut.ulWidth, hWindow->stAdjSclOut.ulHeight,
                &hWindow->ulStgPxlAspRatio_x_y );
        }

        /* bAdjRectsDirty is set to true by BVDC_P_Window_ApplyChanges_isr and
         * BVDC_P_Window_ValidateRects_isr, or something changes here.
         * It will get clear once the rectangle are used,
         * BVDC_P_Window_UpdateSrcAndUserInfo_isr(). */
        hWindow->bAdjRectsDirty = false;
        hWindow->stCurInfo.stDirty.stBits.bRecAdjust = BVDC_P_DIRTY;
        hWindow->stCurInfo.stDirty.stBits.bReDetVnet = BVDC_P_DIRTY;

        /* The source also need to adjust it output rect. */
        hWindow->stCurInfo.hSource->stCurInfo.stDirty.stBits.bRecAdjust = BVDC_P_DIRTY;

        /* ------------------------------------------------------------------*/
        /* (12) support offscreen                                            */
        /* ------------------------------------------------------------------*/
        if(!pUserInfo->bMosaicMode)
        {
            BDBG_MSG(("W%d: Offscreen In: %d %d %d %d - %d %d %d %d",
                hWindow->eId, hWindow->stAdjSclOut.lLeft, hWindow->stAdjSclOut.lTop, hWindow->stAdjSclOut.ulWidth, hWindow->stAdjSclOut.ulHeight,
                hWindow->stAdjDstRect.lLeft, hWindow->stAdjDstRect.lTop, hWindow->stAdjDstRect.ulWidth, hWindow->stAdjDstRect.ulHeight));
            BVDC_P_Window_AdjOffscreen_isr(hWindow, pDstFmtInfo,
                    &hWindow->stSrcCnt, &hWindow->stAdjSclOut, &hWindow->stAdjDstRect);
            BDBG_MSG(("W%d: Offscreen Out: %d %d %d %d - %d %d %d %d",
                hWindow->eId, hWindow->stAdjSclOut.lLeft, hWindow->stAdjSclOut.lTop, hWindow->stAdjSclOut.ulWidth, hWindow->stAdjSclOut.ulHeight,
                hWindow->stAdjDstRect.lLeft, hWindow->stAdjDstRect.lTop, hWindow->stAdjDstRect.ulWidth, hWindow->stAdjDstRect.ulHeight));
        }

    }

    BDBG_LEAVE(BVDC_P_Window_AdjustRectangles_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * lWinXMin, lWinYMin, lWinXMax and lWinYMax in S27.4 format
 *
 * When box detect is used with mpeg, capture and replay should be used,
 * and mpeg feeder should fetch the full src
 */
void BVDC_P_Window_GetSourceContentRect_isr
    ( const BVDC_Window_Handle         hWindow,
      const BAVC_MVD_Field            *pMvdFieldData,
      const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
      int32_t                         *plWinXMin,
      int32_t                         *plWinYMin,
      int32_t                         *plWinXMin_R,
      int32_t                         *plWinXMax,
      int32_t                         *plWinYMax )
{
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    BDBG_ASSERT(plWinXMin);
    BDBG_ASSERT(plWinXMax);
    BDBG_ASSERT(plWinYMin);
    BDBG_ASSERT(plWinYMax);
    BSTD_UNUSED(pXvdFieldData);

    if((BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId)) &&
        (hWindow->stCurResource.hBoxDetect))
    {
        BDBG_ASSERT(pMvdFieldData);

        /* force mpeg feeder to feed the whole src */
        *plWinXMin = 0;
        *plWinYMin = 0;
        *plWinXMin_R = 0;
        *plWinXMax = pMvdFieldData->ulSourceHorizontalSize << BVDC_P_16TH_PIXEL_SHIFT;
        *plWinYMax = pMvdFieldData->ulSourceVerticalSize   << BVDC_P_16TH_PIXEL_SHIFT;
    }
    else
    {
        *plWinXMin = hWindow->stSrcCnt.lLeft;
        *plWinYMin = hWindow->stSrcCnt.lTop;
        *plWinXMin_R = hWindow->stSrcCnt.lLeft_R;
        *plWinXMax = (hWindow->stSrcCnt.lLeft +
            (hWindow->stSrcCnt.ulWidth  << BVDC_P_16TH_PIXEL_SHIFT));
        *plWinYMax = (hWindow->stSrcCnt.lTop +
            (hWindow->stSrcCnt.ulHeight << BVDC_P_16TH_PIXEL_SHIFT));
    }

    return;
}

/***************************************************************************
 * {private}
 *
 * Check the status if this window is shut down, and ready for reconfiguring
 * source or vnet.
 */
void BVDC_P_Window_SetReconfiguring_isr
    ( BVDC_Window_Handle               hWindow,
      bool                             bSrcPending,
      bool                             bReConfigVnet,
      bool                             bBufferPending )
{
    BVDC_P_Window_Info *pCurInfo;
    BVDC_P_Window_DirtyBits *pCurDirty;

    /* Get dirty bits to check if we're ready. */
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    pCurInfo  = &hWindow->stCurInfo;
    pCurDirty = &hWindow->stCurInfo.stDirty;

    /* Need to do the following when we want the window to shutdown,
     * Notes there are mutually exclusive. */
    if((bReConfigVnet) &&
       (!bSrcPending) && (!bBufferPending) &&
       (!pCurDirty->stBits.bSrcPending))
    {
        pCurDirty->stBits.bReConfigVnet = BVDC_P_DIRTY;
    }
    else if(bSrcPending || bBufferPending)
    {
        if(bSrcPending)
            pCurDirty->stBits.bSrcPending   = BVDC_P_DIRTY;

        if(bBufferPending)
            pCurDirty->stBits.bBufferPending = BVDC_P_DIRTY;

        pCurDirty->stBits.bReConfigVnet = BVDC_P_CLEAN;
    }

    /* Don't shutdown again if it's in middle of shutdown process; or if
     * we have shutdown but idle for src-pending.  Destroy must go thru
     * shutdown in order to prevent premature return to user */
    if((hWindow->bSetDestroyEventPending) ||
       ((pCurDirty->stBits.bShutdown == BVDC_P_CLEAN) &&
        ((BVDC_P_State_eInactive != pCurInfo->eWriterState) ||
         (BVDC_P_State_eInactive != pCurInfo->eReaderState))))
    {
        pCurInfo->eReaderState  = BVDC_P_State_eShutDownPending;
        pCurInfo->eWriterState  = BVDC_P_State_eShutDownPending;
        pCurDirty->stBits.bShutdown    = BVDC_P_DIRTY;
    }
    return;
}


/***************************************************************************
 * {private}
 *
 * Check the status if this window is shut down, and ready for reconfiguring
 * source or vnet.  Return true if completed.
 */
static bool BVDC_P_Window_NotReconfiguring_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_Window_DirtyBits *pCurDirty;

    /* Get dirty bits to check if we're ready. */
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    pCurDirty = &hWindow->stCurInfo.stDirty;

    if(BVDC_P_IS_CLEAN(pCurDirty))
    {
        return true;
    }
    else
    {
        return ((BVDC_P_CLEAN == pCurDirty->stBits.bShutdown) &&
                (BVDC_P_CLEAN == pCurDirty->stBits.bSrcPending) &&
                (BVDC_P_CLEAN == pCurDirty->stBits.bBufferPending) &&
                (BVDC_P_CLEAN == pCurDirty->stBits.bReConfigVnet));
    }
}


/* Bandwidth equation without deinterlacer
 *    ratio = ((ceil(ox / SCB_burst_size)) / (ceil(ix / SCB_burst_size))) * sy
 * Bandwidth equation with deinterlacer
 *    if MVP before CAP
 *        ratio = ((ceil(ox / SCB_burst_size)) / (ceil((ix * hx) / SCB_burst_size))) * (oy / (iy * 2))
 *    else
 *        ratio = ((ceil(ox / SCB_burst_size)) / (ceil(ix / SCB_burst_size))) * sy * hx * 2
 * If ratio < 1 => SCL before CAP/VFD
 * where:
 *    ceil(x) is ceiling operation that rounds up x value to the immediate next
 *            integer
 *    ix is the width of SCL input picture in pixels unit
 *    ox is the width of SCL output picture in pixels unit
 *    sy is the SCL vertical scaling ratio, i.e. sy = oy / iy, where oy is the
 *          height of SCL output picture and iy is the height of SCL input picture
 *    SCB_burst_size is the SCB burst size of CAP and VFD in pixels unit;
 *                      7420 has SCB_burst_size = 256 pixels (or 16 JWords)
 *    hx is the horizontal scaling ratio of HSCL/MCVP, i.e.
 *          hx = hscl_out_x / hscl_in_x; if HSCL is bypassed, hx = 1
 */
static uint32_t BVDC_P_Window_DecideSclCapAsymmetric_isr
    ( BVDC_Window_Handle               hWindow,
      const BFMT_VideoInfo            *pDstFmt,
      bool                             bSrcInterlace,
      bool                             bDeinterlace)
{
    uint32_t ulCapFdrRate;
    uint32_t ulInX, ulInY, ulOutX, ulOutY;
    uint32_t ulOutRate, ulInRate;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    ulInX      = hWindow->stSrcCnt.ulWidth;
    ulOutX     = hWindow->stAdjSclOut.ulWidth;
    ulInY  = (bSrcInterlace)
        ? (hWindow->stSrcCnt.ulHeight / BVDC_P_FIELD_PER_FRAME)
        : (hWindow->stSrcCnt.ulHeight);

    ulOutY = (pDstFmt->bInterlaced)
        ? (hWindow->stAdjSclOut.ulHeight / BVDC_P_FIELD_PER_FRAME)
        : (hWindow->stAdjSclOut.ulHeight);

    ulOutRate = BVDC_P_DIV_ROUND_UP(ulOutX, BVDC_P_SCB_BURST_SIZE) * ulOutY;
    ulInRate  = BVDC_P_DIV_ROUND_UP(ulInX,  BVDC_P_SCB_BURST_SIZE) * ulInY;

    /* At lease 1 so not do divide by zero!  Use decision osciallation by
     * some delta.  Here we allow to stay with the last decision if it's with
     * some BVDC_P_BW_DELTA.  Currently it's set to 1% for rate oscillation.
     * But cut also overschedule RTS for aspect ratio correct without
     * toggle scl/cap, though need to find the delta and update accordingly. */
    ulInRate     = BVDC_P_MAX(1, ulInRate);
    ulCapFdrRate = (ulOutRate * BVDC_P_BW_RATE_FACTOR / ulInRate);

    BDBG_MSG(("BW: InX=%d InY=%d OutX=%d OutY=%d ulOutRate=%d ulInRate=%d ulCapFdrRate=%d",
        ulInX, ulInY, ulOutX, ulOutY, ulOutRate,
        ulInRate, ulCapFdrRate));

    if(bDeinterlace)
    {
        if(hWindow->bSrcSideDeinterlace)
        {
            BDBG_ASSERT(bSrcInterlace);
            ulOutRate = BVDC_P_DIV_ROUND_UP(ulOutX, BVDC_P_SCB_BURST_SIZE) * ulOutY;
            ulInRate  = BVDC_P_DIV_ROUND_UP(ulInX,  BVDC_P_SCB_BURST_SIZE) * ulInY * 2;
            ulCapFdrRate = (ulOutRate * BVDC_P_BW_RATE_FACTOR / ulInRate);
            BDBG_MSG(("MvpBeforeCap: InX=%d InY=%d OutX=%d OutY=%d ulOutRate=%d ulInRate=%d ulCapFdrRate=%d",
                ulInX, ulInY, ulOutX, ulOutY, ulOutRate,
                ulInRate, ulCapFdrRate));
        }
        else if(ulCapFdrRate > BVDC_P_BW_BASE)
        {
            /* Need to take into account the MAD -> SCL case */
            /* when MAD is placed after CAP (ulCapFdrRate > BVDC_P_BW_BASE), the */
            /* output rate of MAD must  NOT exceed oclk / (sx' * sy') */
            /* where oclk is the display specific output pixel clk rate */
            /* MAD output rate is chip specific, SD-MAD: 27MHz, HD-MAD: 148.5MHz */
            /* (sx' * sy') is the SCL scaling ratio */
            /* where sy = 2sy' when MAD is used and sx = hx*sx' when HSCL is used */
#define BVDC_P_MAD_OUTPUT_RATE   (1485 * BFMT_FREQ_FACTOR / 10)
            /* be careful with 32-bit math overflow! */
            uint32_t ulSclRatio = (ulOutX * ulOutY * (BVDC_P_BW_RATE_FACTOR/10) / (ulInX * hWindow->stSrcCnt.ulHeight)) * 10;
            uint32_t ulOclk = pDstFmt->ulPxlFreq ;
            uint32_t ulMadOutClk = ulOclk * BVDC_P_BW_RATE_FACTOR / ulSclRatio;

            BDBG_MSG(("BW+MAD: InX=%d InY'=%d OutX=%d OutY=%d SclRatio=%d oclk=%d MadOutClk=%d BVDC_P_MAD_OUTPUT_RATE=%d",
                ulInX, ulInY, ulOutX, ulOutY, ulSclRatio,
                ulOclk, ulMadOutClk, BVDC_P_MAD_OUTPUT_RATE));

            if(ulMadOutClk > BVDC_P_MAD_OUTPUT_RATE)
            {
                BDBG_MSG(("Need to force SCL/CAP"));
                ulCapFdrRate = BVDC_P_BW_RATE_FACTOR * BVDC_P_BW_RATE_FACTOR / ulCapFdrRate;
            }
        }
    }

    return ulCapFdrRate;
}

/***************************************************************************
 * {private}
 *
 * Combining user info and src dynamic info, we decide new vnetMode. When
 * bApplyNewVnet is false, we only check whether a vnet reconfiguring is
 * needed, no window context is changed here, and return true if a vnet
 * reconfiguring is needed.  If bApplyNewVnet is true, we store new
 * vnetMode into win context.
 *
 * bForceBypassMcvp should be false normally, it should only be set to true
 * to force bypass mcvp when there are not enough buffers for mcvp
 */
static bool BVDC_P_Window_DecideVnetMode_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_MVD_Field            *pMvdFieldData,
      bool                             bApplyNewVnet,
      bool                            *pbRfcgMcvp,
      bool                             bForceBypassMcvp,
      uint32_t                         ulPictureIdx)
{
    const BFMT_VideoInfo *pSrcFmt;
    const BFMT_VideoInfo *pDstFmt;
    const BVDC_P_FormatInfo *pSrcFmtDetails;
    BVDC_P_Window_Info *pCurInfo;
    BVDC_P_VnetMode stVnetMode;
    BVDC_P_VnetMode *pVnetMode, *pCurVnetMode;
    BVDC_P_MvpMode stMvpMode, *pMvpMode;
    uint32_t ulCapFdrRate;
    bool bSclCapBaseOnRate;
    bool bSrcInterlace = false;
    bool bScalerFirst = false;
    bool bDeinterlace = false;
    bool bAnr = false;
    bool bRecfgVnet = false;
    bool bStreamProgressive = false;
    bool bVnetDiff = false;
#if (BVDC_P_SUPPORT_TNTD)
    uint32_t ulVertRatio;
#endif
    uint32_t ulBoxWinId;
    uint32_t ulMaxMadWidth, ulMaxMadHeight, ulHsclSrcHrzSclThr, ulWidth;

    BVDC_DisplayId eDisplayId;
    BVDC_P_WindowId eWindowId;
    const BBOX_Vdc_Capabilities *pBoxVdc;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    pCurInfo  = &hWindow->stCurInfo;
    pDstFmt = hWindow->hCompositor->stCurInfo.pFmtInfo;
    pSrcFmt = hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo;
    BDBG_ASSERT(NULL!=pbRfcgMcvp);
    *pbRfcgMcvp = false;
    BSTD_UNUSED(ulPictureIdx);

    eDisplayId = hWindow->hCompositor->hDisplay->eId;
    eWindowId = hWindow->eId;
    pBoxVdc = &hWindow->hCompositor->hVdc->stBoxConfig.stVdc;
    ulBoxWinId = BVDC_P_GetBoxWindowId_isrsafe(eWindowId);
    BDBG_ASSERT(ulBoxWinId < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY);

    /* no need to calculate new vnetMode if we are destroying the window */
    if(pCurInfo->stDirty.stBits.bDestroy)
    {
        return true;
    }

    /* disable ANR for any source > 1080p */
    /* check if we need to turn off anr due to anr hw limit */
#if BVDC_P_SUPPORT_MANR
    if((hWindow->stCurResource.hMcvp) &&(hWindow->stCurResource.hMcvp->bAnr))
    {
        BVDC_P_Window_GetDeinterlacerMaxResolution_isr(hWindow, pMvdFieldData,
            &ulMaxMadWidth, &ulMaxMadHeight, &ulHsclSrcHrzSclThr, true);

        if(pCurInfo->stMadSettings.bShrinkWidth)
        {
            ulWidth = BVDC_P_MIN(pSrcFmt->ulWidth, ulHsclSrcHrzSclThr);
        }
        else
            ulWidth = pSrcFmt->ulWidth;

        bAnr = (
            (hWindow->stCurInfo.bAnr) &&     /* User enabled */
#if !BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
            (!hWindow->stCurInfo.bMosaicMode) &&
#endif
            (ulWidth  <= ulMaxMadWidth &&
             pSrcFmt->ulHeight <= ulMaxMadHeight));
    }
    else
#endif
    {
        bAnr = false;
    }

    /* May need to fine tune the dst timing for mpeg source.  In the case  */
    /* "&& (NULL != pMvdFieldData)" check is added only for Coverity check */
    if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId) && (NULL != pMvdFieldData))
    {
        pSrcFmt = pDstFmt;
        bSrcInterlace = (BAVC_Polarity_eFrame != pMvdFieldData->eSourcePolarity);
        bStreamProgressive = pMvdFieldData->bStreamProgressive;
    }
    else if (BVDC_P_SRC_IS_VFD(hWindow->stCurInfo.hSource->eId))
    {
        bSrcInterlace = false;
    }
    else
    {
        bSrcInterlace = pSrcFmt->bInterlaced;
    }

    /* Get the source info with details informations */
    pSrcFmtDetails = BVDC_P_GetFormatInfo_isrsafe(pSrcFmt->eVideoFmt);

    /* Check if we need to turn/on or off deinterlacer.
       Note, mosaic mode should force off MAD since the input pictures list
       could contain dynamic format change while MAD on-the-fly toggling
       requires vnet reconfig process which take multiple vsyncs now; */
    /*bDeinterlace = false;*/
    if (hWindow->stCurInfo.bDeinterlace &&
        (NULL !=hWindow->stCurResource.hMcvp) &&
        pBoxVdc->astDisplay[eDisplayId].astWindow[ulBoxWinId].stResource.ulMad != BBOX_FTR_INVALID)
    {
        bool bMadSrcSizeOk;
        BVDC_P_WrRateCode eWriterVsReaderRateCode;
        BVDC_P_WrRateCode eReaderVsWriterRateCode;
        BFMT_Orientation eOrientation;

        BVDC_P_Window_GetDeinterlacerMaxResolution_isr(hWindow, pMvdFieldData,
            &ulMaxMadWidth, &ulMaxMadHeight, &ulHsclSrcHrzSclThr, true);

        BDBG_ASSERT(hWindow->stCurResource.hMcvp);

        if(pCurInfo->stMadSettings.bShrinkWidth)
        {
            ulWidth = BVDC_P_MIN(hWindow->stCurInfo.hSource->stScanOut.ulWidth, ulHsclSrcHrzSclThr);
        }
        else
            ulWidth = hWindow->stCurInfo.hSource->stScanOut.ulWidth;
        eOrientation = BVDC_P_VNET_USED_MVP_AT_WRITER(hWindow->stVnetMode)?
            hWindow->eSrcOrientation:hWindow->eDispOrientation;

        ulMaxMadWidth >>=(eOrientation == BFMT_Orientation_e3D_LeftRight);
        ulMaxMadHeight>>=(eOrientation == BFMT_Orientation_e3D_OverUnder);

        bMadSrcSizeOk =
             (hWindow->stCurInfo.hSource->stScanOut.ulHeight <= ulMaxMadHeight) &&
             (ulWidth  <= ulMaxMadWidth);

        bDeinterlace = (
            (bSrcInterlace) && (!BVDC_P_SCANOUTMODE_DECIMATE(hWindow->eScanoutMode)) &&
            (bMadSrcSizeOk)
#if !BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
            && (!hWindow->stCurInfo.bMosaicMode)
#endif
            );

        /* No 1080i deinterlacing at 100/120Hz; TODO: what if RTS allows? */
        if(hWindow->stCurInfo.hSource->ulVertFreq >= 100*BFMT_FREQ_FACTOR &&
           hWindow->stCurInfo.hSource->stScanOut.ulHeight > BFMT_720P_HEIGHT
#if (BVDC_P_SUPPORT_MTG)
            && (!hWindow->stCurInfo.hSource->bMtgSrc)
#endif
        ) {
            bDeinterlace = false;
        }

        if(pMvdFieldData)
        {
            BDBG_MODULE_MSG(BVDC_DEINTERLACER_MOSAIC, ("win[%d] picture channel %d bDeinterlacer %d bSrcInterlace %d bMadSrcSizeOk %d %d, vfreq=%u",
                hWindow->eId, ulPictureIdx,
                bDeinterlace, bSrcInterlace, bMadSrcSizeOk,
                hWindow->eScanoutMode,
                hWindow->stCurInfo.hSource->ulVertFreq));
        }

        /* SW7425-772: (2) USAGE POLICY:  Better to have deinterlacer to be off
         * in this case. */
        if(BVDC_Mode_eOff != pCurInfo->stMadSettings.ePqEnhancement)
        {
            bool bRateGap;
            if(hWindow->bSyncLockSrc && bStreamProgressive)
            {
                bRateGap = false;
            }
            else
            {
                uint32_t ulVertFreq = (hWindow->bSyncLockSrc)
                    ? hWindow->stCurInfo.hSource->ulStreamVertFreq
                    : hWindow->stCurInfo.hSource->ulVertFreq;

                BVDC_P_Buffer_CalculateRateGap_isr(ulVertFreq, pDstFmt->ulVertFreq,
                    &eWriterVsReaderRateCode, &eReaderVsWriterRateCode);

                bRateGap = (BVDC_P_WrRate_NotFaster != eReaderVsWriterRateCode);
            }

            if((!bRateGap) &&
#if BVDC_P_SUPPORT_MTG
               (!hWindow->stCurInfo.hSource->bMtgSrc || hWindow->stCurInfo.bMosaicMode) &&
#endif
               ( pDstFmt->bInterlaced) &&
               ( hWindow->ulNrmVrtSrcStep == (1<<BVDC_P_NRM_SRC_STEP_F_BITS)))
            {
                bDeinterlace = false;
            }
        }
    }

    ulCapFdrRate = BVDC_P_Window_DecideSclCapAsymmetric_isr(
                   hWindow, pDstFmt, bSrcInterlace, bDeinterlace);

    if(BVDC_P_EQ_DELTA(ulCapFdrRate, BVDC_P_BW_BASE, hWindow->stCurInfo.ulBandwidthDelta))
    {
        if(hWindow->stCurInfo.eSclCapBias == BVDC_SclCapBias_eAuto)
        {
            if(ulCapFdrRate > BVDC_P_BW_BASE)
            {
                bSclCapBaseOnRate = false;
                BDBG_MSG(("Bias = %d, Rate [r=%d/%d] is more optimized if: CAP -> SCL",
                    hWindow->stCurInfo.eSclCapBias, BVDC_P_BW_BASE, ulCapFdrRate));
            }
            else
            {
                bSclCapBaseOnRate = true;
                BDBG_MSG(("Bias = %d, Rate [r=%d/%d] is more optimized if: SCL -> CAP",
                    hWindow->stCurInfo.eSclCapBias, BVDC_P_BW_BASE, ulCapFdrRate));
            }
        }
        else if(hWindow->stCurInfo.eSclCapBias == BVDC_SclCapBias_eSclBeforeCap)
        {
            bSclCapBaseOnRate = true;
            BDBG_MSG(("Bias = %d, Rate [r=%d/%d] is bias: SCL -> CAP",
                hWindow->stCurInfo.eSclCapBias, BVDC_P_BW_BASE, ulCapFdrRate));
        }
        else /* hWindow->stCurInfo.eSclCapBias == BVDC_SclCapBias_eSclAfterCap */
        {
            bSclCapBaseOnRate = false;
            BDBG_MSG(("Bias = %d, Rate [r=%d/%d] is bias: CAP -> SCL",
                hWindow->stCurInfo.eSclCapBias, BVDC_P_BW_BASE, ulCapFdrRate));
        }
    }
    else if(ulCapFdrRate > BVDC_P_BW_BASE)
    {
        bSclCapBaseOnRate = false;
        BDBG_MSG(("Bias = %d, Rate [r=%d/%d] is more optimized if: CAP -> SCL",
            hWindow->stCurInfo.eSclCapBias, BVDC_P_BW_BASE, ulCapFdrRate));
    }
    else
    {
        bSclCapBaseOnRate = true;
        BDBG_MSG(("Bias = %d, Rate [r=%d/%d] is more optimized if: SCL -> CAP",
            hWindow->stCurInfo.eSclCapBias, BVDC_P_BW_BASE, ulCapFdrRate));
    }

    bScalerFirst =
        /* Feed rate to high for post scaling! */
        ( (bSclCapBaseOnRate) ||

          /* Scale first on sd format with HD or ED sources */
          ((pSrcFmtDetails->bHd || pSrcFmtDetails->bEd) && (VIDEO_FORMAT_IS_SD(pDstFmt->eVideoFmt))) ||

          /* force frame capture */
          (hWindow->stCurInfo.hSource->stCurInfo.bForceFrameCapture) ||

          /* MosaicMode: always scale first! */
          (hWindow->stCurInfo.bMosaicMode) );

    /* calculate new vnetMode */
    pVnetMode = &stVnetMode;
    pCurVnetMode = &hWindow->stVnetMode;
    BVDC_P_CLEAN_ALL_DIRTY(pVnetMode);

    pVnetMode->stBits.bUseScl = (NULL != hWindow->stCurResource.hScaler)? BVDC_P_ON : BVDC_P_OFF;
    pVnetMode->stBits.bUseCap = (hWindow->bCapture)? BVDC_P_ON : BVDC_P_OFF;

    pVnetMode->stBits.bUseDnr = (hWindow->stCurInfo.hSource->stCurInfo.bDnr && hWindow->stCurResource.hDnr)
        ? BVDC_P_ON : BVDC_P_OFF;
    pVnetMode->stBits.bSclBeforeCap = (pVnetMode->stBits.bUseCap && bScalerFirst) ? BVDC_P_ON : BVDC_P_OFF;
    pVnetMode->stBits.bSrcSideDeinterlace = (hWindow->bSrcSideDeinterlace) ? BVDC_P_ON : BVDC_P_OFF;
#if BVDC_P_SUPPORT_XSRC
    pVnetMode->stBits.bUseXsrc = (hWindow->stCurResource.hXsrc)? BVDC_P_ON : BVDC_P_OFF;
#endif
#if BVDC_P_SUPPORT_VFC
    pVnetMode->stBits.bUseVfc = (hWindow->stCurResource.hVfc)? BVDC_P_ON : BVDC_P_OFF;
#endif

    pMvpMode = &stMvpMode;
    BVDC_P_CLEAN_ALL_DIRTY(pMvpMode);
    pMvpMode->stBits.bUseMvp = pVnetMode->stBits.bUseMvp =
#if (!BVDC_P_SUPPORT_MOSAIC_DEINTERLACE)
         (!pCurInfo->bMosaicMode)&&
#endif
         (pCurInfo->bDeinterlace || pCurInfo->bAnr) &&
         ( hWindow->stCurResource.hMcvp); /* to be sure the hmcvp resource available)*/

    pMvpMode->stBits.bUseMvpBypass =
        (((BVDC_P_ON == pMvpMode->stBits.bUseMvp) && (!bDeinterlace) && (!bAnr)) || bForceBypassMcvp)
        ? BVDC_P_ON : BVDC_P_OFF;
    pMvpMode->stBits.bUseMad = (bDeinterlace && !bForceBypassMcvp)? BVDC_P_ON : BVDC_P_OFF;

    /* clear off all the mcvp detail configuration for mode compare */
    pMvpMode->stBits.bUseHscl = ((bDeinterlace || bAnr) && !bForceBypassMcvp)? BVDC_P_ON : BVDC_P_OFF;
#if (BVDC_P_SUPPORT_MANR)
    pMvpMode->stBits.bUseAnr = (bAnr && !bForceBypassMcvp)? BVDC_P_ON : BVDC_P_OFF;
#endif

#if (BVDC_P_SUPPORT_TNTD)
    /* base on vert scaling ratio, need to decide if before or after SCL */
    pVnetMode->stBits.bUseTntd = (pCurInfo->bSharpnessEnable)? BVDC_P_ON : BVDC_P_OFF;
    if(pVnetMode->stBits.bUseTntd)
    {
        if(hWindow->stCurInfo.bMosaicMode)
        {
            pVnetMode->stBits.bTntdBeforeScl = BVDC_P_OFF;
        }
        else
        {
            ulVertRatio = BVDC_P_Tntd_CalcVertSclRatio_isr(hWindow->stSrcCnt.ulHeight,
                    bSrcInterlace & !pVnetMode->stBits.bUseMvp,
                    hWindow->stAdjSclOut.ulHeight,
                    pDstFmt->bInterlaced);
            if(hWindow->stCurInfo.eSclCapBias == BVDC_SclCapBias_eAuto)
            {
                pVnetMode->stBits.bTntdBeforeScl =
                    (ulVertRatio >= BVDC_P_TNTD_BEFORE_SCL_THRESH) ? BVDC_P_ON : BVDC_P_OFF;
            }
            else
            {
                pVnetMode->stBits.bTntdBeforeScl = pCurVnetMode->stBits.bTntdBeforeScl;
            }
        }
        BDBG_MSG(("%s", pVnetMode->stBits.bTntdBeforeScl ? "TNTD->SCL" : "SCL->TNTD"));
    }
#endif

    /* Enable bInvalid bit if vnetmode is not set */
    if(BVDC_P_IS_CLEAN(pVnetMode))
        pVnetMode->stBits.bInvalid = BVDC_P_ON;

    /* add vnetMode bits for vnet crc */
    if(pCurInfo->stCbSettings.stMask.bCrc)
    {
        bRecfgVnet = BVDC_P_VnetCrc_DecideVnetMode_isr(hWindow, hWindow->stCurResource.hVnetCrc, pVnetMode);
    }
    else if ((!BVDC_P_VNET_USED_VNETCRC(hWindow->stVnetMode)) &&
             hWindow->stCurResource.hVnetCrc)
    {
        /* disable vnet crc when the crc src module was not used in vnet mode */
        BDBG_MSG(("Window %d releases shared VnetCrc", hWindow->eId));
        BVDC_P_VnetCrc_ReleaseConnect_isr(&hWindow->stCurResource.hVnetCrc);
        hWindow->stNewResource.hVnetCrc = NULL;
        hWindow->stCurInfo.stDirty.stBits.bVnetCrc = BVDC_P_OFF;
    }

    bVnetDiff = BVDC_P_DIRTY_COMPARE(pVnetMode, pCurVnetMode);
    if(bVnetDiff)
    {
        BDBG_MODULE_MSG(BVDC_WIN_VNET,("Win[%d] ch[%d] %s vnetMode 0x%08lx -> 0x%08lx UseMAD %d -> %d",
            hWindow->eId, ulPictureIdx, bApplyNewVnet ? "changes" : "checks",
            (long unsigned int)pVnetMode->aulInts[0], (long unsigned int)pCurVnetMode->aulInts[0],
            hWindow->stMvpMode.stBits.bUseMad, pMvpMode->stBits.bUseMad));

        bRecfgVnet = true;
        if (bApplyNewVnet)
        {
            /* store new vnetMode into win contrext */
            hWindow->stVnetMode = *pVnetMode;
        }
    }

    BDBG_MODULE_MSG(BVDC_WIN_VNET,("Win[%d] ch[%d] %s McvpMode 0x%08lx",
    hWindow->eId, ulPictureIdx, bApplyNewVnet ? "changes" : "checks",
    (long unsigned int)pMvpMode->aulInts[0]));
    BDBG_MODULE_MSG(BVDC_WIN_VNET,(" UseMcvp %d", pMvpMode->stBits.bUseMvp));
    BDBG_MODULE_MSG(BVDC_WIN_VNET,(" UseMad %d", pMvpMode->stBits.bUseMad));
    BDBG_MODULE_MSG(BVDC_WIN_VNET,(" BypassMcvp %d", pMvpMode->stBits.bUseMvpBypass));
    BDBG_MODULE_MSG(BVDC_WIN_VNET,(" UseAnr %d", pMvpMode->stBits.bUseAnr));

    /* store new MvpMode into win contrext */
    hWindow->stMvpMode = *pMvpMode;

    return bRecfgVnet;
}

static void BVDC_P_Window_GetBufAllocMode_isr
    ( BVDC_Window_Handle               hWindow,
      bool                             bSrcIs3d,
      bool                             bDispIs3d,
      BVDC_P_BufHeapAllocMode         *peBufAllocMode )
{
    const BVDC_P_Source_Info *pSrcInfo;

    BDBG_ASSERT(peBufAllocMode);

    pSrcInfo  = &hWindow->stCurInfo.hSource->stCurInfo;
    if(bSrcIs3d && bDispIs3d)
    {
        if((pSrcInfo->pFmtInfo->eOrientation == BFMT_Orientation_e2D) &&
           (hWindow->hCompositor->stCurInfo.pFmtInfo->eOrientation == BFMT_Orientation_e2D))
        {
            *peBufAllocMode = BVDC_P_BufHeapAllocMode_eLRCombined;
        }
        else
            *peBufAllocMode = BVDC_P_BufHeapAllocMode_eLRSeparate;
    }
    else
    {
        *peBufAllocMode = BVDC_P_BufHeapAllocMode_eLeftOnly;
    }
}

/* This function changes buffer count given a new src-disp rate relationship. */
static void BVDC_P_Window_ChangeBufferCountAndDelay_isr
    ( BVDC_Window_Handle hWindow,
      BVDC_P_BufferCountState eState )
{

    switch (eState)
    {
        case BVDC_P_BufferCount_eIncremented:
            hWindow->ulBufCntNeeded --;
            hWindow->ulBufDelay--;
            hWindow->hBuffer->ulVsyncDelay--;
            hWindow->bBufferCntIncremented = false;
            break;

        case BVDC_P_BufferCount_eToIncrement:
            hWindow->ulBufCntNeeded++;
            hWindow->ulBufDelay++;
            hWindow->hBuffer->ulVsyncDelay++;
            hWindow->bBufferCntIncremented = true;
            break;

        case BVDC_P_BufferCount_eDecremented:
            hWindow->ulBufCntNeeded++;
            if (!hWindow->bBufferCntDecrementedForPullDown)
            {
                hWindow->ulBufDelay++;
                hWindow->hBuffer->ulVsyncDelay++;
            }
            else
            {
                hWindow->bBufferCntDecrementedForPullDown = false;
            }
            hWindow->bBufferCntDecremented = false;
            break;

        case BVDC_P_BufferCount_eToDecrement:
            hWindow->ulBufCntNeeded--;
            if (hWindow->hBuffer->eWriterVsReaderRateCode == hWindow->hBuffer->eReaderVsWriterRateCode)
            {
                hWindow->ulBufDelay--;
                hWindow->hBuffer->ulVsyncDelay--;
            }
            else /* Progressive pull down */
            {
                hWindow->bBufferCntDecrementedForPullDown = true;
            }

            hWindow->bBufferCntDecremented = true;
            break;
    }
}


static void BVDC_P_Window_DetermineBufferCount_isr
    ( BVDC_Window_Handle hWindow )
{
    bool bDoPulldown = false, bProgressivePullDown = false, bBuf50to60Hz = false;
    BVDC_P_WrRateCode eWriterVsReaderRateCode;
    BVDC_P_WrRateCode eReaderVsWriterRateCode;
    uint32_t ulSrcVertRate, ulDstVertRate;
    uint32_t ulPrevBufCntNeeded;

    /* Get previous count */
    ulPrevBufCntNeeded = hWindow->ulBufCntNeeded;

    if (BVDC_P_VNET_USED_CAPTURE(hWindow->stVnetMode))
    {
        /* if source dynamic format change results in possible buffer count change,
           do it as soon as possible to avoid unnecessary big allocation in the 1st
           place;
           Note, ulBufCntNeeded computed at ApplyChanges time might not reflect
           the current situation since the source format might have changed! */
        BVDC_P_Buffer_CalculateRateGap_isr(hWindow->stCurInfo.hSource->ulVertFreq,
            hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
            &eWriterVsReaderRateCode, &eReaderVsWriterRateCode);

        bDoPulldown =
            (!hWindow->bSyncLockSrc && !hWindow->stSettings.bForceSyncLock && !BVDC_P_SRC_IS_VFD(hWindow->stNewInfo.hSource->eId) &&
            ((VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt)
            && (eWriterVsReaderRateCode == eReaderVsWriterRateCode)) ||
               (hWindow->bDoPulldown && eReaderVsWriterRateCode > BVDC_P_WrRate_Faster /* not for 50i-to-60i */)));

        /* This doesn't apply to 50i-to-60i. */
        if(bDoPulldown && !hWindow->bBufferCntDecremented)
        {
            if (hWindow->bBufferCntIncremented)
            {
                /* From N+1 buffers to the N buffers first */
                BVDC_P_Window_ChangeBufferCountAndDelay_isr(hWindow, BVDC_P_BufferCount_eIncremented);
            }

            /* From N buffers to N-1 buffers */
            BVDC_P_Window_ChangeBufferCountAndDelay_isr(hWindow, BVDC_P_BufferCount_eToDecrement);
            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Decrementing buffer count to %d", hWindow->eId, hWindow->ulBufCntNeeded));
        }
        else if(!bDoPulldown && hWindow->bBufferCntDecremented)
        {
            /* From N-1 buffers to N buffers */
            BVDC_P_Window_ChangeBufferCountAndDelay_isr(hWindow, BVDC_P_BufferCount_eDecremented);
            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Change buffer count back to %d", hWindow->eId, hWindow->ulBufCntNeeded));
        }

    }

    if (!hWindow->hBuffer->bSyncLock && !hWindow->stSettings.bForceSyncLock)
    {

        /* When displaying 1080p24/25/30 source as 1080p48/50/60, we cut the number
         * of buffer to 3 to save memory. The algorithm allows writer to catch up
         * reader and both of them point to the same buffer node.
         *
         * Note: This may cause video tearing if reader somehow misses interrupts.
         */
        bProgressivePullDown =  VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->eVideoFmt) &&
                                (hWindow->hBuffer->eWriterVsReaderRateCode == BVDC_P_WrRate_NotFaster) &&
                                (hWindow->hBuffer->eReaderVsWriterRateCode >= BVDC_P_WrRate_2TimesFaster);

        /* Check to see if buffer count was reduced due to progressive display format. If so and
         * the reader or writer rate gaps is 1, increment the buffer cnt. */
        if ((!hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced || bProgressivePullDown) &&
            !hWindow->hBuffer->bMtgMadDisplay1To1RateRelationship &&
            !(hWindow->stCurInfo.hSource->bMtgSrc && !BVDC_P_MVP_USED_MAD(hWindow->stMvpMode)) &&
            hWindow->bCapture)
        {
            if ((hWindow->hBuffer->eWriterVsReaderRateCode == hWindow->hBuffer->eReaderVsWriterRateCode) || bProgressivePullDown)
            {
                if (!hWindow->bBufferCntDecremented)
                {
                    if (hWindow->bBufferCntIncremented)
                    {
                        /* From N+1 buffers to the N buffers first */
                        BVDC_P_Window_ChangeBufferCountAndDelay_isr(hWindow, BVDC_P_BufferCount_eIncremented);
                    }

                    /* From N buffers to N-1 buffers */
                    BVDC_P_Window_ChangeBufferCountAndDelay_isr(hWindow, BVDC_P_BufferCount_eToDecrement);
                    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Decrementing buffer count from %d to %d due progressive display format",
                        hWindow->eId, ulPrevBufCntNeeded, hWindow->ulBufCntNeeded));
                }
            }
            else if (hWindow->bBufferCntDecremented)
            {
                BVDC_P_Window_ChangeBufferCountAndDelay_isr(hWindow, BVDC_P_BufferCount_eDecremented);
                BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Incrementing buffer count from %d to %d due to rate gap",
                    hWindow->eId, ulPrevBufCntNeeded, hWindow->ulBufCntNeeded));
            }
        }

        /* If source/dest relationship requires a writer gap, capture as interlaced and interlaced display,
         * increment the number of buffers. */
        ulSrcVertRate = BVDC_P_ROUND_OFF(hWindow->stCurInfo.hSource->ulVertFreq,
            (BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR);
        ulDstVertRate = BVDC_P_ROUND_OFF(hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
            (BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR);

        /* SW7425-4703: roll back version 255 for SW7425-3748 */
        bBuf50to60Hz = (ulSrcVertRate == 50 && ulDstVertRate == 60) ? true : false;

        /* For 50-to-60 with no deinterlacer */
        if ((!hWindow->bFrameCapture) && (!VIDEO_FORMAT_IS_PROGRESSIVE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
            && ((hWindow->hBuffer->eWriterVsReaderRateCode > BVDC_P_WrRate_NotFaster) ||
                (bBuf50to60Hz && (!BVDC_P_VNET_USED_SCALER_AT_READER(hWindow->stVnetMode)))))
        {
            if (!hWindow->bBufferCntIncremented)
            {
                if (hWindow->bBufferCntDecremented)
                {
                    /* From N-1 buffers to N buffers first */
                    BVDC_P_Window_ChangeBufferCountAndDelay_isr(hWindow, BVDC_P_BufferCount_eDecremented);
                }

                /* From N buffers to N+1 buffers */
                BVDC_P_Window_ChangeBufferCountAndDelay_isr(hWindow, BVDC_P_BufferCount_eToIncrement);
                BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Incrementing buffer count from %d to %d ",
                        hWindow->eId, ulPrevBufCntNeeded, hWindow->ulBufCntNeeded));
            }
        }
        else
        {
            if (hWindow->bBufferCntIncremented)
            {
                BVDC_P_Window_ChangeBufferCountAndDelay_isr(hWindow, BVDC_P_BufferCount_eIncremented);
                BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] Decrementing buffer count from %d to %d ",
                    hWindow->eId, ulPrevBufCntNeeded, hWindow->ulBufCntNeeded));
            }
        }
    }

    /* set dirty bit */
   if((hWindow->ulBufCntNeeded != hWindow->ulBufCntAllocated) ||
      (hWindow->ulBufCntNeeded != ulPrevBufCntNeeded))
    {
        BDBG_MSG(("ulBufCntAllocated (%d) or ulPrevBufCntNeeded (%d) != ulBufCntNeeded (%d), stVnetMode = 0x%x",
            hWindow->ulBufCntAllocated, ulPrevBufCntNeeded,
            hWindow->ulBufCntNeeded, *(unsigned int *)&hWindow->stVnetMode));
        hWindow->stCurInfo.stDirty.stBits.bReallocBuffers = BVDC_P_DIRTY;
    }
}

static bool BVDC_P_Window_DecideCapBufsCfgs_isr
    ( BVDC_Window_Handle               hWindow,
      bool                             bApplyNewCfg,
      bool                             bSrcIs3d,
      bool                             bDispIs3d,
      const BVDC_P_Rect               *pCapBufRect,
      bool                             bInterlace,
      bool                             bMinSrcInterlace,
      BAVC_VideoBitDepth               eBitDepth)
{
    uint32_t ulSrcVertRate, ulDspVertRate, ulBufSize, ulMinDspSize;
    const BFMT_VideoInfo * pMinDspFmt = NULL, *pDstFmtInfo = NULL;
    bool  bCapInterlaced, bDoPulldown, bCapture, bRecfgVnet = false, bUseMadAtWriter = false;
    BVDC_P_Window_Info *pCurInfo;
    BVDC_P_Rect stMinBufRect, stCapBufRect;
    BVDC_P_BufferHeapId  eBufferHeapIdRequest, eBufferHeapIdPrefer;
    BVDC_P_BufHeapAllocMode eBufAllocMode;
    const BVDC_P_Source_Info *pSrcInfo;

    pDstFmtInfo = hWindow->hCompositor->stCurInfo.pFmtInfo;
    pCurInfo = &hWindow->stCurInfo;
    pSrcInfo  = &hWindow->stCurInfo.hSource->stCurInfo;

    ulSrcVertRate = BVDC_P_ROUND_OFF(
        hWindow->stCurInfo.hSource->ulVertFreq, (BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR);
    ulDspVertRate = BVDC_P_ROUND_OFF(
        pDstFmtInfo->ulVertFreq, (BFMT_FREQ_FACTOR/2), BFMT_FREQ_FACTOR);

    bUseMadAtWriter = BVDC_P_MVP_USED_MAD_AT_WRITER(hWindow->stVnetMode, hWindow->stMvpMode);

    if((!bInterlace) && BVDC_P_DO_PULLDOWN(ulSrcVertRate, ulDspVertRate))
    {
        /* P2I frame rate conversion case: frame capture -> fields playback,
         * ideally for 24/25/30 to 50/60 pull-down */
        bCapInterlaced = false;
        bDoPulldown = true;
    }
#if (BVDC_P_SUPPORT_MTG)
    else if (hWindow->stCurInfo.hSource->bMtgSrc && !hWindow->stCurInfo.bMosaicMode &&
             (bUseMadAtWriter))
    {
        bCapInterlaced = false;
        bDoPulldown = false;
    }
#endif
    else if (bInterlace && BVDC_P_DO_PULLDOWN(ulSrcVertRate, ulDspVertRate) &&
             bUseMadAtWriter &&
             BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode))
    {
        /* This is only for 50i to 60i and vice-versa frame rate conversion case with the
           deinterlacer and SCL at the writer. */
        bCapInterlaced = false;
        bDoPulldown = true;
    }
    else
    {
        if(BVDC_P_VNET_USED_SCALER_AT_READER(hWindow->stVnetMode))
        {
            bCapInterlaced = bMinSrcInterlace && !hWindow->bSrcSideDeinterlace;
        }
        else
        {
            bCapInterlaced = pDstFmtInfo->bInterlaced && !pSrcInfo->bForceFrameCapture &&
                            (ulSrcVertRate == ulDspVertRate);
        }
        bDoPulldown = false;
    }

    /* always store bDoPulldown into win context because its change might not need
     * need vnet reconfigure */
    hWindow->bDoPulldown = bDoPulldown;

    bCapture = BVDC_P_VNET_USED_CAPTURE(hWindow->stVnetMode);

    if(bCapture)
    {
        if(BVDC_P_VNET_USED_SCALER_AT_READER(hWindow->stVnetMode))
        {
            BVDC_P_Window_GetBufSize_isr(hWindow->eId, pCapBufRect,
                bCapInterlaced, pCurInfo->bMosaicMode, false, false, false,
                pCurInfo->ePixelFormat, &hWindow->stCapCompression,
                BVDC_P_BufHeapType_eCapture, &ulBufSize, eBitDepth);
        }
        else /* cap and scl used, scl before cap */
        {
            const BFMT_VideoInfo *pAllocFmt = pDstFmtInfo;
            uint32_t  ulMinDspWidth = pAllocFmt->ulWidth;
            uint32_t  ulMinDspHeight = pAllocFmt->ulHeight;
            BFMT_Orientation eDspOrientation = BFMT_Orientation_e2D;

            pMinDspFmt = hWindow->stSettings.pMinDspFmt;
            /* allocate guard band if necessary according to mosaic mode if enabled fill bars */
            stCapBufRect = (pCurInfo->bMosaicMode ||
                (BVDC_Mode_eOff != hWindow->stCurInfo.eEnableBackgroundBars &&
                 0 < hWindow->stCurInfo.ucZOrder))
                ? pCurInfo->stScalerOutput : hWindow->stAdjDstRect;

            /* Max capture is 1080p in AutoDisable1080p mode */
            if(!hWindow->stCurInfo.bMosaicMode &&
               (BBOX_Vdc_SclCapBias_eAutoDisable1080p == hWindow->eBoxSclCapBias))
            {
                if((pAllocFmt->ulWidth*pAllocFmt->ulHeight) > (BFMT_1080I_WIDTH*BFMT_1080I_HEIGHT))
                {
                    pAllocFmt = BFMT_GetVideoFormatInfoPtr_isrsafe(BFMT_VideoFmt_e1080p);
                }
            }

            /* guaranteed some minimal memory allocation to avoid reallocations
             * when display output format changes. */
            if(BFMT_IS_3D_MODE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
                eDspOrientation = hWindow->hCompositor->stCurInfo.pFmtInfo->eOrientation;
            else
                eDspOrientation = hWindow->hCompositor->stCurInfo.eOrientation;

            /* pAllocFmt */
            if(BFMT_IS_3D_MODE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
            {
                stMinBufRect.ulWidth  = pAllocFmt->ulWidth;
                stMinBufRect.ulHeight = pAllocFmt->ulHeight;
            }
            else
            {
                stMinBufRect.ulWidth  = pAllocFmt->ulWidth >> (eDspOrientation==BFMT_Orientation_e3D_LeftRight);
                stMinBufRect.ulHeight = pAllocFmt->ulHeight>> (eDspOrientation==BFMT_Orientation_e3D_OverUnder);
            }
            stMinBufRect.lLeft    = stMinBufRect.lLeft_R = stMinBufRect.lTop = 0;

            if(pMinDspFmt)
            {
                ulMinDspWidth = pMinDspFmt->ulWidth >> (eDspOrientation==BFMT_Orientation_e3D_LeftRight);
                ulMinDspHeight= pMinDspFmt->ulHeight>> (eDspOrientation==BFMT_Orientation_e3D_OverUnder);
                if((ulMinDspWidth * ulMinDspHeight) >
                   (stMinBufRect.ulWidth * stMinBufRect.ulHeight))
                {
                    pAllocFmt = pMinDspFmt;
                    stMinBufRect.ulWidth  = ulMinDspWidth;
                    stMinBufRect.ulHeight = ulMinDspHeight;
                }
            }

#if (BVDC_P_MADR_HSIZE_WORKAROUND)
            /* the "+ 1" is for potential cap left/top align down, and
             * the 4 is for DCX_HSIZE_WORKAROUND */
            stCapBufRect.ulWidth  = BVDC_P_ALIGN_UP(stCapBufRect.ulWidth  + 1, 4);
            stCapBufRect.ulHeight = BVDC_P_ALIGN_UP(stCapBufRect.ulHeight + 1, 2);
            stMinBufRect.ulWidth  = BVDC_P_ALIGN_UP(stMinBufRect.ulWidth  + 1, 4);
            stMinBufRect.ulHeight = BVDC_P_ALIGN_UP(stMinBufRect.ulHeight + 1, 2);
#endif

            /* Don't optimize memory for PIG/PBP or format changes, use fullscreen size */
            ulMinDspSize = 0;
            if((pMinDspFmt) ||
               (hWindow->stSettings.bAllocFullScreen))
            {
                uint32_t ulBoxWindowHeightFraction, ulBoxWindowWidthFraction;
                const BBOX_Vdc_Capabilities *pBoxVdc;
                uint32_t ulBoxWinId, ulMaxWidth, ulMaxHeight;
                BVDC_DisplayId eDisplayId = hWindow->hCompositor->hDisplay->eId;
                const BBOX_Vdc_Window_Capabilities *pBoxWinCap;

                /* Check if destination rectangle is bigger than BOX limits */
                pBoxVdc = &hWindow->hCompositor->hVdc->stBoxConfig.stVdc;
                ulBoxWinId = BVDC_P_GetBoxWindowId_isrsafe(hWindow->eId);
                BDBG_ASSERT(ulBoxWinId < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY);
                pBoxWinCap = &pBoxVdc->astDisplay[eDisplayId].astWindow[ulBoxWinId];

                ulBoxWindowWidthFraction = pBoxWinCap->stSizeLimits.ulWidthFraction;
                ulBoxWindowHeightFraction = pBoxWinCap->stSizeLimits.ulHeightFraction;

                if ((ulBoxWindowWidthFraction != BBOX_VDC_DISREGARD) &&
                    (ulBoxWindowHeightFraction != BBOX_VDC_DISREGARD))
                {
                    BDBG_ASSERT(ulBoxWindowWidthFraction);
                    BDBG_ASSERT(ulBoxWindowHeightFraction);
                    /* constrain pip capture buffer to 1/2 x 1/2 if quarter screen pip size limit is specified; */
                    if(hWindow->hCompositor->hVdc->stSettings.pMemConfigSettings &&
                       hWindow->hCompositor->hVdc->stSettings.pMemConfigSettings->stDisplay[eDisplayId].stWindow[ulBoxWinId].bPip &&
                       (ulBoxWindowWidthFraction * ulBoxWindowHeightFraction < 4))
                    {
                        ulBoxWindowWidthFraction  = 2;
                        ulBoxWindowHeightFraction = 2;
                    }

                    ulMaxWidth = pMinDspFmt ? pMinDspFmt->ulWidth / ulBoxWindowWidthFraction
                        : pDstFmtInfo->ulWidth / ulBoxWindowWidthFraction;
                    ulMaxHeight = pMinDspFmt ? pMinDspFmt->ulHeight / ulBoxWindowHeightFraction
                        : pDstFmtInfo->ulHeight / ulBoxWindowHeightFraction;
                    stMinBufRect.ulWidth = BVDC_P_MIN(stMinBufRect.ulWidth, ulMaxWidth);
                    stMinBufRect.ulHeight = BVDC_P_MIN(stMinBufRect.ulHeight, ulMaxHeight);
                    BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("Win[%d] Minbuf size %dx%d based on boxmode limit %dx%d",
                        hWindow->eId, stMinBufRect.ulWidth, stMinBufRect.ulHeight,
                        ulMaxWidth, ulMaxHeight));
                }
                else if(pMinDspFmt && BVDC_P_WIN_IS_V1(hWindow->eId))
                {
                    /* PIP window:
                     * if size is <= 1/4 full screen, use 1/4 full screen */
                    if((stCapBufRect.ulWidth <= (ulMinDspWidth/2)) &&
                       (stCapBufRect.ulHeight <= (ulMinDspHeight/2)))
                    {
                        stMinBufRect.ulWidth = ulMinDspWidth/2;
                        stMinBufRect.ulHeight = ulMinDspHeight/2;
                        BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("Win[%d] Minbuf size to %dx%d, 1/4 of %s size",
                            hWindow->eId, stMinBufRect.ulWidth, stMinBufRect.ulHeight,
                            pMinDspFmt->pchFormatStr));
                    }
                    else
                    {
                        stMinBufRect.ulWidth  = stCapBufRect.ulWidth;
                        stMinBufRect.ulHeight = stCapBufRect.ulHeight;
                    }
                }

                /*stCapBufRect.ulWidth  = BVDC_P_MAX(stCapBufRect.ulWidth,  pAllocFmt->ulWidth);
                stCapBufRect.ulHeight = BVDC_P_MAX(stCapBufRect.ulHeight, pAllocFmt->ulHeight);*/
                BVDC_P_Window_GetBufSize_isr(hWindow->eId, &stMinBufRect,
                    pAllocFmt->bInterlaced, pCurInfo->bMosaicMode,
                    /* allocate guard band if necessary according to mosaic mode if enabled fill bars */
                    (BVDC_Mode_eOff!=hWindow->stCurInfo.eEnableBackgroundBars &&
                     0 < hWindow->stCurInfo.ucZOrder), false, false,
                    pCurInfo->ePixelFormat, &hWindow->stCapCompression,
                    BVDC_P_BufHeapType_eCapture, &ulMinDspSize, eBitDepth);
            }

            BVDC_P_Window_GetBufSize_isr(hWindow->eId, &stCapBufRect,
                bCapInterlaced, pCurInfo->bMosaicMode,
                /* allocate guard band if necessary according to mosaic mode if enabled fill bars */
                (BVDC_Mode_eOff!=hWindow->stCurInfo.eEnableBackgroundBars &&
                 0 < hWindow->stCurInfo.ucZOrder), false, false,
                pCurInfo->ePixelFormat, &hWindow->stCapCompression,
                BVDC_P_BufHeapType_eCapture, &ulBufSize, eBitDepth);

            if(ulBufSize < ulMinDspSize)
            {
                ulBufSize = ulMinDspSize;
                stCapBufRect.ulWidth  = stMinBufRect.ulWidth ;
                stCapBufRect.ulHeight = stMinBufRect.ulHeight;
                BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("Win[%d] Reset CAP buffer to mini dst size %dx%d",
                    hWindow->eId, stCapBufRect.ulWidth, stCapBufRect.ulHeight));
                BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("Win[%d] pMinDspFmt: %s, bAllocFullScreen: %d",
                    hWindow->eId, pMinDspFmt ? pMinDspFmt->pchFormatStr : "NULL",
                    hWindow->stSettings.bAllocFullScreen));
            }
        }

        /* Check if left and right buffers can be combined */
        /* Capture in 3D mode if src orientation is 3D */
        /* Always create separate buffers for Left and right for now */
        BVDC_P_Window_GetBufAllocMode_isr(hWindow, bSrcIs3d, bDispIs3d, &eBufAllocMode);
        if(eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRCombined)
        {
            ulBufSize = 2* ulBufSize;
        }
        BVDC_P_Window_GetBufHeapId_isr(hWindow, ulBufSize, hWindow->hCapHeap,
            &eBufferHeapIdRequest, &eBufferHeapIdPrefer);

        /* changing eBufAllocMode causes  re-allocate right buffers only */
        if(hWindow->eBufAllocMode != eBufAllocMode)
        {
            pCurInfo->stDirty.stBits.bBufAllocMode = BVDC_P_DIRTY;

            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] changes cap buf allocmode %d -> %d",
                hWindow->eId, hWindow->eBufAllocMode, eBufAllocMode));
            hWindow->ePrevBufAllocMode = hWindow->eBufAllocMode;
            hWindow->eBufAllocMode = eBufAllocMode;
        }

        /* changing cap buf heapId causes VNET reconfiguration */
        if(hWindow->eBufferHeapIdRequest != eBufferHeapIdRequest)
        {
            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] changes cap buf heapIdRequest %s -> %s", hWindow->eId,
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdRequest),
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapIdRequest)));

            bRecfgVnet = true;
            if(!bApplyNewCfg)
            {
                return bRecfgVnet;
            }
            else
            {
                /* store changes into win context */
                hWindow->eBufferHeapIdRequest = eBufferHeapIdRequest;
                hWindow->ePrevBufAllocMode = hWindow->eBufAllocMode;
                hWindow->eBufAllocMode = eBufAllocMode;
            }
        }

        if(bApplyNewCfg)
        {
            hWindow->eBufferHeapIdPrefer = eBufferHeapIdPrefer;
        }

        /* Here we determine if the capture is going to store the picture
         * as frame. We always store bFrameCapture into win context because
         * its change might not need vnet reconfigure.
         * hWindow->bFrameCapture is used in:
         *     1. multibuffer mechanism to increment or decrement buffer count
         *     2. determining forced alternate capture
         */
        hWindow->bFrameCapture = (

            /* (1)  */
            (!bCapInterlaced) ||

            /* (2) Destination is progressive, and we don't use the scaler,
             * or the scaler is at the writer. */
            ((!pDstFmtInfo->bInterlaced) &&
             ( BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode) ||
               (!BVDC_P_VNET_USED_SCALER(hWindow->stVnetMode)))) ||

            /* (3) Source is progressive, and scaler is not on the writer. */
            ((!hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->bInterlaced) &&
             ( BVDC_P_VNET_USED_SCALER_AT_READER(hWindow->stVnetMode))) ||

            /* (4) force frame capture*/
            (hWindow->stCurInfo.hSource->stCurInfo.bForceFrameCapture));
    }

    return (bRecfgVnet);
}


static bool BVDC_P_Window_DecideMcvpBufsCfgs_isr
    ( BVDC_Window_Handle               hWindow,
      bool                             bApplyNewCfg,
      const BVDC_P_Rect               *pMadBufRect,
      bool                             bDoubleBufSize,
      BAVC_VideoBitDepth               eBitDepth,
      uint32_t                         ulChannelId,
      bool                             bInterlace)
{
    bool bDeinterlace, bAnr, bBypass, bRecfgVnet = false;
    uint32_t ulBufSize;
    BVDC_P_Window_Info *pCurInfo;
    bool bBufIsContinuous, bCurContinuous;
    BVDC_P_BufferHeapId eNewMcvpHeapId, eNewMcvpQmHeapId, eCurMcvpHeapId, eCurMcvpQmHeapId;
    uint32_t i, ulSrcWidth, ulSrcHeight;
    BAVC_MVD_Field   *pVdcPic;
    BVDC_P_Rect  stMadBufRect = *pMadBufRect;
    bool bInterlaced = false;
    uint16_t usMcvpPixBufCnt=0, usMcvpQmBufCnt=0, usCurPixBufCnt=0, usCurQmBufCnt=0, usPixBufCnt=0, usStdPixBufCnt, usStdQmBufCnt;
    uint32_t ulBufHeapSize = 0, ulPxlBufSize =0, ulCurPxlBufSize =0 ;
    BVDC_P_Compression_Settings *pWinCompression = NULL;

    bDeinterlace = BVDC_P_MVP_USED_MAD(hWindow->stMvpMode);
    bAnr = BVDC_P_MVP_USED_ANR(hWindow->stMvpMode);
    bBypass = BVDC_P_MVP_BYPASS_MVP(hWindow->stMvpMode);
    pCurInfo  = &hWindow->stCurInfo;

    pWinCompression = &hWindow->stMadCompression;
    bBufIsContinuous = bAnr || pWinCompression->bEnable;

    if(bBypass)
    {
        BDBG_MODULE_MSG(BVDC_WIN_VNET,("Win[%d] chan[%d] mosaic changes mcvp buf cntr %d->0", hWindow->eId, ulChannelId,
                hWindow->usMadPixelBufferCnt[ulChannelId]));

        eNewMcvpHeapId = BVDC_P_BufferHeapId_eUnknown;
        eNewMcvpQmHeapId = BVDC_P_BufferHeapId_eUnknown;
        usPixBufCnt = 0;
        usMcvpQmBufCnt = 0;
        ulPxlBufSize = 0;
    }
    else
    {
        if(hWindow->stSettings.bDeinterlacerAllocFull)
        {
            /* SWSTB-6375: ANR for progressive needs to use 2 continuous frame buffer rather than 4 non-continuous buffer */
            bBufIsContinuous = !hWindow->stCurResource.hMcvp->hMcdi->bMadr;
            usStdPixBufCnt = BVDC_P_Mcdi_GetPixBufCnt_isr(hWindow->stCurResource.hMcvp->hMcdi->bMadr, BVDC_MadGameMode_eOff);
            usStdQmBufCnt = hWindow->stCurResource.hMcvp->hMcdi->bMadr ?
                    BVDC_P_MAD_QM_BUFFER_COUNT : BVDC_P_MCDI_QM_BUFFER_COUNT;
            BDBG_MODULE_MSG(BVDC_WIN_VNET,("Win[%d] mosaic bDeinterlacerAllocFull true mcvp pixel %d QM %d MosaicCount %d",
                hWindow->eId, usStdPixBufCnt, usStdQmBufCnt, hWindow->stCurInfo.ulMosaicCount));
        }
        else
        {
            if(!bDeinterlace)
            {
                usStdPixBufCnt = BVDC_P_ANR_BUFFER_COUNT;
                usStdQmBufCnt  = 0;
            }
            else
            {
                usStdPixBufCnt =BVDC_P_Mcdi_GetPixBufCnt_isr(
                    hWindow->stCurResource.hMcvp->hMcdi->bMadr,
                    pCurInfo->stMadSettings.eGameMode);

                usStdQmBufCnt = hWindow->stCurResource.hMcvp->hMcdi->bMadr ?
                    BVDC_P_MAD_QM_BUFFER_COUNT : BVDC_P_MCDI_QM_BUFFER_COUNT;

                /* no QM buffer needed for 1/0 field buffer force spatial */
                usStdQmBufCnt = BVDC_P_MAD_SPATIAL(pCurInfo->stMadSettings.eGameMode)?
                    0:usStdQmBufCnt;
            }
            bBufIsContinuous =
                (bAnr || pWinCompression->bEnable) && usStdPixBufCnt;
        }

        /* (2) decide mcvp buf heap id
         * changing MCVP buffer heapId causes VNET reconfiguration */
        usPixBufCnt=usMcvpPixBufCnt = usStdPixBufCnt;
        usMcvpQmBufCnt = usStdQmBufCnt;

        if (pCurInfo->stMadSettings.bShrinkWidth && bDeinterlace)
        {
            /* horizontal scl before MAD will shrink width to the MAD supported size */
            stMadBufRect.ulWidth = BVDC_P_MIN(stMadBufRect.ulWidth,
                    hWindow->stCurResource.hMcvp->ulMaxWidth);
        }
        BVDC_P_Window_GetBufSize_isr(hWindow->eId, &stMadBufRect,
            bInterlace, false, false, false, false,
            pCurInfo->stMadSettings.ePixelFmt,
            pWinCompression, bDeinterlace?BVDC_P_BufHeapType_eMad_Pixel: BVDC_P_BufHeapType_eAnr,
            &ulBufSize, eBitDepth);
        ulBufSize = ulBufSize << bDoubleBufSize;
        BVDC_P_Window_GetBufHeapId_isr(hWindow, ulBufSize,
            hWindow->hDeinterlacerHeap, &eNewMcvpHeapId, NULL);
        /* Temp fix to use 2 continous 2HD buffer as 1 4HD buffer for ANR */
        if(eNewMcvpHeapId == BVDC_P_BufferHeapId_eUnknown)
        {
            BVDC_P_Window_GetBufHeapId_isr(hWindow, ulBufSize/2,
                hWindow->hDeinterlacerHeap, &eNewMcvpHeapId, NULL);
            hWindow->usMadPixelBufferCnt[0] = 2*hWindow->usMadPixelBufferCnt[0];
            bBufIsContinuous = true;
        }

        ulPxlBufSize = ulBufSize;

        BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("channel %d buffer size %d %d x %d interlace %d ", ulChannelId, ulBufSize,
            stMadBufRect.ulWidth, stMadBufRect.ulHeight, bDeinterlace));

        BVDC_P_Window_GetBufHeapId_isr(hWindow, ulBufSize,
            hWindow->hDeinterlacerHeap, &eNewMcvpHeapId, NULL);

        BVDC_P_BufferHeap_GetHeapSizeById_isr(hWindow->hDeinterlacerHeap, eNewMcvpHeapId, &ulBufHeapSize);
        BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("win[%d] mvp[%d] pixel buffer %d buf %d heapsize %d",
                hWindow->eId, hWindow->stCurResource.hMcvp->eId, usMcvpPixBufCnt,
                ulBufSize, ulBufHeapSize));

        /* Qm filed buffer */
        BVDC_P_Window_GetBufSize_isr(hWindow->eId, &stMadBufRect,
            bInterlace, false, false, false, false,
            pCurInfo->stMadSettings.ePixelFmt,
            pWinCompression, BVDC_P_BufHeapType_eMad_QM,
            &ulBufSize, eBitDepth);
        ulBufSize = ulBufSize <<bDoubleBufSize;
        BVDC_P_Window_GetBufHeapId_isr(hWindow, ulBufSize,
            hWindow->hDeinterlacerHeap, &eNewMcvpQmHeapId, NULL);

        if(bBufIsContinuous)
        {
            uint32_t  ulOverallBufSize;

            BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("0****continuous win[%d] mvp[%d] channel[%d] pixel buffer cnt %d x %d  %s Qm buf cnt %d x %s",
                hWindow->eId, hWindow->stCurResource.hMcvp->eId, ulChannelId,
                hWindow->usMadPixelBufferCnt[ulChannelId],
                hWindow->ulMadPxlBufSize[ulChannelId],
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eMadPixelHeapId[ulChannelId]),
                hWindow->usMadQmBufCnt[ulChannelId], BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eMadQmHeapId[ulChannelId])));

            ulOverallBufSize = usMcvpPixBufCnt * ulPxlBufSize;
            usPixBufCnt  = BVDC_P_DIV_ROUND_UP(ulOverallBufSize, ulBufHeapSize);

            /* update the pixel buf size */
            BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("1****continuous win[%d] mvp[%d] channel[%d] pixel compressed buffer %d heap %d x %d overall %d",
            hWindow->eId, hWindow->stCurResource.hMcvp->eId, ulChannelId,
                ulPxlBufSize, usMcvpPixBufCnt, ulBufHeapSize, ulOverallBufSize));
        }
    }

    bBufIsContinuous = bBufIsContinuous && usPixBufCnt;

    eCurMcvpHeapId   = hWindow->eMadPixelHeapId[ulChannelId];
    eCurMcvpQmHeapId = hWindow->eMadQmHeapId[ulChannelId];
    usCurPixBufCnt   = hWindow->usMadPixelBufferCnt[ulChannelId];
    usCurQmBufCnt    = hWindow->usMadQmBufCnt[ulChannelId];
    bCurContinuous   = hWindow->bContinuous[ulChannelId];
    ulCurPxlBufSize  = hWindow->ulMadPxlBufSize[ulChannelId];
    /* Changing MCVP buffer heapId causes VNET reconfiguration */
    if((bCurContinuous       != bBufIsContinuous) ||
       (eCurMcvpHeapId       != eNewMcvpHeapId  ) ||
       (eCurMcvpQmHeapId     != eNewMcvpQmHeapId) ||
       (usCurPixBufCnt       != usPixBufCnt)||
       (usCurQmBufCnt        != usMcvpQmBufCnt)||
       (ulCurPxlBufSize      != ulPxlBufSize))
    {
        BDBG_MODULE_MSG(BVDC_WIN_VNET,("Win[%d] applychange %s changes mcvp buf MStartHpId[%s->%s] cnt [%d->%d], QmHpId[%s->%s] cnt [%d->%d] continuous %d ->%d",
            hWindow->eId, bApplyNewCfg?"true":"false",
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eCurMcvpHeapId),
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eNewMcvpHeapId),
            usCurPixBufCnt, usPixBufCnt,
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eCurMcvpQmHeapId),
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eNewMcvpQmHeapId),
            usCurQmBufCnt, usMcvpQmBufCnt,
            bCurContinuous, bBufIsContinuous));

        /* buffer count are all 0s ignore the bufferId*/
        if(usCurPixBufCnt + usCurQmBufCnt)
            hWindow->ulMosaicReconfig = 1<<ulChannelId;
        /* store new values into win context */
        hWindow->eMadPixelHeapId[ulChannelId]     = eNewMcvpHeapId;
        hWindow->eMadQmHeapId[ulChannelId]        = eNewMcvpQmHeapId;
        hWindow->usMadPixelBufferCnt[ulChannelId] = usPixBufCnt;
        hWindow->usMadQmBufCnt[ulChannelId]       = usMcvpQmBufCnt;
        hWindow->ulMadPxlBufSize[ulChannelId]     = ulPxlBufSize;
        hWindow->bContinuous[ulChannelId]         = bBufIsContinuous;

        /* Reconfig vnet if buffer changes */
        bRecfgVnet = (!hWindow->stSettings.bDeinterlacerAllocFull) && (hWindow->stSettings.pMinSrcFmt==NULL);
    }

    /* fill the rest channel if the dimention remains the same*/
    if(hWindow->stCurInfo.bMosaicMode)
    {
        pVdcPic = &(hWindow->stCurInfo.hSource->stNewPic[ulChannelId]);
        ulSrcWidth = pVdcPic->ulSourceHorizontalSize;
        ulSrcHeight = pVdcPic->ulSourceVerticalSize;
        bInterlaced = (BAVC_Polarity_eFrame != pVdcPic->eSourcePolarity);
        for (i= ulChannelId; i< hWindow->stCurInfo.ulMosaicCount; i++)
        {
            pVdcPic = &(hWindow->stCurInfo.hSource->stNewPic[i]);
            if((ulSrcWidth == pVdcPic->ulSourceHorizontalSize) &&
                (ulSrcHeight == pVdcPic->ulSourceVerticalSize) &&
                (bInterlaced == (BAVC_Polarity_eFrame != pVdcPic->eSourcePolarity)))
            {
                hWindow->eMadPixelHeapId[i]     = eNewMcvpHeapId;
                hWindow->eMadQmHeapId[i]        = eNewMcvpQmHeapId;
                hWindow->usMadPixelBufferCnt[i] = usPixBufCnt;
                hWindow->usMadQmBufCnt[i]       = usMcvpQmBufCnt;
                hWindow->ulMadPxlBufSize[i]     = ulPxlBufSize;
                hWindow->bContinuous[i]         = bBufIsContinuous;
        }
        else
            break;
    }


        /* clear bMosaicIntra when vnet reconfig*/
        if(i == (hWindow->stCurInfo.ulMosaicCount -1))
        {
            hWindow->ulMosaicReconfig = 0;
        }
    }

#if (BVDC_P_SUPPORT_MANR)
    if(hWindow->stCurResource.hMcvp->hAnr)
        BVDC_P_Anr_SetBufPxlFmt_isr(hWindow->stCurResource.hMcvp->hAnr, hWindow->stCurInfo.stAnrSettings.ePxlFormat);
#endif

        /* TODO: assert compression mismatch between MAD and ANR */
#if (BDBG_DEBUG_BUILD)
    BDBG_ASSERT(!bDeinterlace || !bAnr ||
                (hWindow->stCurInfo.stMadSettings.ePixelFmt ==
                 hWindow->stCurInfo.stAnrSettings.ePxlFormat));
#endif
    BSTD_UNUSED(bApplyNewCfg);
    return bRecfgVnet;
}


static void BVDC_P_Window_GetOrientation_isr
    ( BVDC_Window_Handle               hWindow,
      bool                            *pbSrcIs3d,
      bool                            *pbDispIs3d )
{
    const BVDC_P_Source_Info *pSrcInfo;

    BDBG_ASSERT(pbSrcIs3d);
    BDBG_ASSERT(pbDispIs3d);

    pSrcInfo  = &hWindow->stCurInfo.hSource->stCurInfo;
    if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId))
    {
        hWindow->eSrcOrientation = hWindow->stCurInfo.hSource->hMpegFeeder->eOutputOrientation;
    }
    else if(BVDC_P_SRC_IS_HDDVI(hWindow->stCurInfo.hSource->eId))
    {
        /* bOrientationOverride only valid when original
         * orientation from FMT is 2D */
        hWindow->eSrcOrientation = ((pSrcInfo->bOrientationOverride) &&
            (pSrcInfo->pFmtInfo->eOrientation == BFMT_Orientation_e2D))?
            pSrcInfo->eOrientation: pSrcInfo->pFmtInfo->eOrientation;
    }

    if(BFMT_IS_3D_MODE(hWindow->hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
        hWindow->eDispOrientation = hWindow->hCompositor->stCurInfo.pFmtInfo->eOrientation;
    else
        hWindow->eDispOrientation = hWindow->hCompositor->stCurInfo.eOrientation;

    *pbSrcIs3d = (hWindow->eSrcOrientation != BFMT_Orientation_e2D);
    *pbDispIs3d = (hWindow->eDispOrientation != BFMT_Orientation_e2D);

}


/***************************************************************************
 * {private}
 *
 */
static void BVDC_P_Window_DumpCapBufHeapConfigure_isr
    ( BVDC_Window_Handle               hWindow,
      bool                             bLeftBuffer )
{
#if (BDBG_DEBUG_BUILD)
    bool     bSyncSlipInMemconfig;
    BVDC_Compositor_Handle   hCompositor = hWindow->hCompositor;

    bSyncSlipInMemconfig =
        hCompositor->hVdc->abSyncSlipInMemconfig[hCompositor->eId][hWindow->eId];

    BDBG_ERR(("Win[%d] Not enough memory for %s! Configuration:",
        hWindow->eId, bLeftBuffer ? "CAP" : "CAP_R"));
    BDBG_ERR(("Win[%d] stVnetMode: 0x%x, ulBufCntNeeded: %d, ulBufCntAllocated: %d",
        hWindow->eId, *(unsigned int *)&hWindow->stVnetMode, hWindow->ulBufCntNeeded,
        hWindow->ulBufCntAllocated));
    BDBG_ERR(("Win[%d] Src: %s, Disp: %s", hWindow->eId,
        hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->pchFormatStr,
        hCompositor->stCurInfo.pFmtInfo->pchFormatStr));
    BDBG_ERR(("Win[%d] uiVsyncDelayOffset: %d, uiCaptureBufCnt: %d", hWindow->eId,
        hWindow->stCurInfo.uiVsyncDelayOffset,
        hWindow->stCurInfo.uiCaptureBufCnt));
    BDBG_ERR(("Win[%d] bDoPulldown: %d, bFrameCapture: %d",
        hWindow->eId, hWindow->bDoPulldown, hWindow->bFrameCapture));
    BDBG_ERR(("Win[%d] SrcVertRate: %d, DstVertRate: %d", hWindow->eId,
        hWindow->stCurInfo.hSource->ulVertFreq,
        hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq));
    BDBG_ERR(("Win[%d] SyncSlip: RunTime: %d, Memconfig: %d  ==> %s ",
        hWindow->eId, !hWindow->bSyncLockSrc, bSyncSlipInMemconfig,
        (hWindow->bSyncLockSrc != bSyncSlipInMemconfig) ? "match" : "mismtach"));
#else
    BSTD_UNUSED(hWindow);
    BSTD_UNUSED(bLeftBuffer);
#endif

    return;
}

/***************************************************************************
 * {private}
 *
 * Check if there is enough buffers for deinterlacer.
 */
static void  BVDC_P_Window_CheckBuffers_isr
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulPictureIdx,
      bool                            *pbNotEnoughCapBuf,
      bool                            *pbNotEnoughMcvpBuffers )
{
    bool                  bNotEnoughCaptureBuffers = false;
    bool                  bNotEnoughMcvpBuffers = false;
    bool                  bCapBufAllocated = false, bCapRBufAllocated = false;
    BERR_Code             err = BERR_SUCCESS;
    BVDC_P_HeapNodePtr    apCapHeapNode[BVDC_P_MAX_MULTI_BUFFER_COUNT];
    BVDC_P_HeapNodePtr    apCapHeapNode_R[BVDC_P_MAX_MULTI_BUFFER_COUNT];

    bool                  bMadPxlBufAllocated = false, bMadQmBufAllocated = false;
    BVDC_P_BufferHeapId   eBufferHeapId;
    BVDC_P_HeapNodePtr    apMadPxlHeapNode[BVDC_P_MAX_MCDI_BUFFER_COUNT];
    BVDC_P_HeapNodePtr    apMadQmHeapNode[BVDC_P_MAX(BVDC_P_MCDI_QM_BUFFER_COUNT, 1)];

    /* Both capture and deinterlacer buffers should be released by now */

    /* Check capture buffers */
    if(hWindow->bCapture && hWindow->eBufferHeapIdRequest != BVDC_P_BufferHeapId_eUnknown)
    {
        err = BVDC_P_BufferHeap_AllocateBuffers_isr(hWindow->hCapHeap,
            apCapHeapNode, hWindow->ulBufCntNeeded, false,
            hWindow->eBufferHeapIdRequest, hWindow->eBufferHeapIdPrefer);
        if(err == BERR_OUT_OF_DEVICE_MEMORY)
        {
            bCapBufAllocated = false;
            bNotEnoughCaptureBuffers = true;
        }
        else
        {
            bCapBufAllocated = true;
        }

        if(hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate)
        {
            err = BVDC_P_BufferHeap_AllocateBuffers_isr(hWindow->hCapHeap,
                apCapHeapNode_R, hWindow->ulBufCntNeeded, false,
                hWindow->eBufferHeapIdRequest, hWindow->eBufferHeapIdPrefer);
            if(err == BERR_OUT_OF_DEVICE_MEMORY)
            {
                bCapRBufAllocated = false;
                bNotEnoughCaptureBuffers = true;
            }
            else
            {
                bCapRBufAllocated = true;
            }
        }
    }

    /* Check pixel buffers */
    if (BVDC_P_VNET_USED_MVP(hWindow->stVnetMode))
    {
        eBufferHeapId = hWindow->eMadPixelHeapId[ulPictureIdx];
        if(eBufferHeapId != BVDC_P_BufferHeapId_eUnknown)
        {
            err = BVDC_P_BufferHeap_AllocateBuffers_isr(hWindow->hDeinterlacerHeap,
                apMadPxlHeapNode, hWindow->usMadPixelBufferCnt[ulPictureIdx],
                hWindow->bContinuous[ulPictureIdx], eBufferHeapId,
                BVDC_P_BufferHeapId_eUnknown);

            if(err == BERR_OUT_OF_DEVICE_MEMORY)
            {
                BDBG_ERR(("Win[%d] Check MCVP Pixel buffer : ", hWindow->eId));
                BDBG_ERR(("App needs to alloc more memory for MCVP Pixel buffers: [%d] %s",
                    hWindow->usMadPixelBufferCnt[ulPictureIdx],
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapId)));
                BVDC_P_PRINT_BUF_DEBUG_INSTRUCTION();

                bMadPxlBufAllocated = false;
                bNotEnoughMcvpBuffers = true;
            }
            else
            {
                bMadPxlBufAllocated = true;
            }
        }

        /* Check qm buffers */
        eBufferHeapId = hWindow->eMadQmHeapId[ulPictureIdx];
        if(eBufferHeapId != BVDC_P_BufferHeapId_eUnknown)
        {
            err = BVDC_P_BufferHeap_AllocateBuffers_isr(hWindow->hDeinterlacerHeap,
                apMadQmHeapNode, hWindow->usMadQmBufCnt[ulPictureIdx], false,
                eBufferHeapId, BVDC_P_BufferHeapId_eUnknown);

            if(err == BERR_OUT_OF_DEVICE_MEMORY)
            {
                BDBG_ERR(("Win[%d] Check MCVP QM buffer : ", hWindow->eId));
                BDBG_ERR(("App needs to alloc more memory for MCVP QM buffers [%d] %s buffers",
                    hWindow->usMadQmBufCnt[ulPictureIdx],
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(eBufferHeapId)));
                BVDC_P_PRINT_BUF_DEBUG_INSTRUCTION();

                bMadQmBufAllocated = false;
                bNotEnoughMcvpBuffers = true;
            }
            else
            {
                bMadQmBufAllocated = true;
            }
        }
    }

    /* Release previous allocated buffers */
    if(bCapBufAllocated)
    {
        BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hCapHeap,
            apCapHeapNode, hWindow->ulBufCntNeeded, false);
    }

    if(bCapRBufAllocated)
    {
        BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hCapHeap,
            apCapHeapNode_R, hWindow->ulBufCntNeeded, false);
    }

    if(bMadPxlBufAllocated)
    {
        BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hDeinterlacerHeap,
            apMadPxlHeapNode, hWindow->usMadPixelBufferCnt[ulPictureIdx],
            hWindow->bContinuous[ulPictureIdx]);
    }

    if(bMadQmBufAllocated)
    {
        BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hDeinterlacerHeap,
            apMadQmHeapNode, hWindow->usMadQmBufCnt[ulPictureIdx],
            false);
    }

    if(pbNotEnoughCapBuf)
        *pbNotEnoughCapBuf = bNotEnoughCaptureBuffers;

    if(pbNotEnoughMcvpBuffers)
        *pbNotEnoughMcvpBuffers = bNotEnoughMcvpBuffers;

    return;
}

/***************************************************************************
 * {private}
 *
 * Combining user info and src dynamic info, we decide new cap/mad/anr bufs
 * heapId. When bApplyNewCfg is false, we only check whether a vnet
 * reconfiguring is needed, no window context is changed here, and return
 * true if a vnet reconfiguring is needed.  If bApplyNewCfg is true,
 * we store new buf configures into win context.
 */
static bool BVDC_P_Window_DecideBufsCfgs_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_MVD_Field            *pMvdFieldData,
      const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
      bool                             bApplyNewCfg,
      uint32_t                         ulPictureIdx)
{
    BVDC_P_Window_Info *pCurInfo;
    bool bInterlace=false, bInterlace4Pulldown=false;
    bool bRecfgVnet = false;
    BVDC_P_Rect stCapBufRect;
    uint32_t ulMinSrcSize, ulSrcSize;
    const BFMT_VideoInfo * pMinSrcFmt = NULL;
    bool bSrcIs3d = false, bMinSrcNoOptimize = true;
    bool bIsMpegSrc=false, bIsHddviSrc = false;
    BAVC_VideoBitDepth       eBitDepth = BAVC_VideoBitDepth_e8Bit;
    bool bDispIs3d = false;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    pCurInfo  = &hWindow->stCurInfo;
    bIsMpegSrc = BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId) && (NULL != pMvdFieldData);
    bIsHddviSrc = BVDC_P_SRC_IS_HDDVI(hWindow->stCurInfo.hSource->eId);

    bMinSrcNoOptimize = bInterlace = bInterlace4Pulldown = (bIsMpegSrc)?
                     (BAVC_Polarity_eFrame != pMvdFieldData->eSourcePolarity) :
                     (hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->bInterlaced);
    stCapBufRect = hWindow->stCurInfo.hSource->stScanOut;
    BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("0 Src Mad %4dx%4d", stCapBufRect.ulWidth, stCapBufRect.ulHeight));

    if(bIsMpegSrc)
    {
        eBitDepth = pMvdFieldData->eBitDepth;
    }
    else if(bIsHddviSrc)
    {
        if(NULL != pXvdFieldData)
        {
            eBitDepth = (BAVC_HDMI_BitsPerPixel_e24bit == pXvdFieldData->eColorDepth)?
                BAVC_VideoBitDepth_e8Bit : BAVC_VideoBitDepth_e10Bit;
        }
    }

    BVDC_P_Window_GetOrientation_isr(hWindow, &bSrcIs3d, &bDispIs3d);

    /* hWindow->stCurInfo.hSource->stScanOut is already set to the full scan out
     * for other sources in BVDC_P_Source_GetScanOutRect_isr */
    if(bIsMpegSrc)
    {
        /* calculate the capture/deinterlacer size assuming the capture in the writer side */
        /* it is re-calculate in BVDC_P_Window_DecideCapBufsCfgs_isr if it is in the reader side*/
#if (!BVDC_P_OPTIMIZE_MEM_USAGE)
        stCapBufRect.ulWidth  = pMvdFieldData->ulSourceHorizontalSize;
        stCapBufRect.ulHeight = pMvdFieldData->ulSourceVerticalSize;
#endif
        /* guaranteed some minimal memory allocation to avoid reallocations. */
        pMinSrcFmt = hWindow->stSettings.pMinSrcFmt;
        if(pMinSrcFmt)
        {
            BVDC_P_Rect stMinBufRect;
            /* Note: stCapBufRect is already accounted for hWindow->eSrcOrientation */
            stMinBufRect.ulWidth  =
                pMinSrcFmt->ulWidth >> (hWindow->eSrcOrientation == BFMT_Orientation_e3D_LeftRight);
            stMinBufRect.ulHeight =
                pMinSrcFmt->ulHeight>> (hWindow->eSrcOrientation == BFMT_Orientation_e3D_OverUnder);
            stMinBufRect.lLeft = stMinBufRect.lLeft_R = stMinBufRect.lTop = 0;

            /* rough estimation minSrc and Src buffer size requirement */
            BVDC_P_Window_GetBufSize_isr(hWindow->eId, &stMinBufRect,
                pMinSrcFmt->bInterlaced, pCurInfo->bMosaicMode,
                false, false, true,
                pCurInfo->ePixelFormat,
                &hWindow->stCapCompression, BVDC_P_BufHeapType_eCapture,
                &ulMinSrcSize, eBitDepth);

            BVDC_P_Window_GetBufSize_isr(hWindow->eId, &stCapBufRect,
                bInterlace, pCurInfo->bMosaicMode,
                false, false, false,
                pCurInfo->ePixelFormat, &hWindow->stCapCompression,
                BVDC_P_BufHeapType_eCapture, &ulSrcSize, eBitDepth);

            if(ulSrcSize < ulMinSrcSize)
            {
                stCapBufRect.ulWidth  = stMinBufRect.ulWidth;
                stCapBufRect.ulHeight = stMinBufRect.ulHeight;
                bMinSrcNoOptimize = bInterlace = pMinSrcFmt->bInterlaced;

                BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("Win[%d] Reset CAP buffer to mini src size %dx%d",
                    hWindow->eId, stCapBufRect.ulWidth, stCapBufRect.ulHeight));
                BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("Win[%d] pMinSrcFmt: %s, bMinSrcNoOptimize: %d",
                    hWindow->eId, pMinSrcFmt ? pMinSrcFmt->pchFormatStr : "NULL",
                    bMinSrcNoOptimize));

                /* progressive min src needs to be compensated interlaced polarity */
                /* dividing by two for deinterlacer if it can fit */
                if(!bMinSrcNoOptimize)
                {
                    bMinSrcNoOptimize = (ulSrcSize > (ulMinSrcSize >>1));
                    BDBG_MODULE_MSG(BVDC_WIN_BUF_SIZE, ("ulSrcSize %d ulMinSrcSize %d bMinSrcOptimize %d",
                        ulSrcSize, ulMinSrcSize >>1, bMinSrcNoOptimize));
                }
            }
        }
    }

    /* (1) decide cap buf heap Id
     * changing cap buf heapId causes VNET reconfiguration
     */
    bRecfgVnet = BVDC_P_Window_DecideCapBufsCfgs_isr(hWindow, bApplyNewCfg,
        bSrcIs3d, bDispIs3d, &stCapBufRect, bInterlace4Pulldown, bMinSrcNoOptimize, eBitDepth);
    if((!bApplyNewCfg) && bRecfgVnet)
    {
        return true;
    }

    /* (2) decide mad buf heapId.
     * changing MCVP buffer heapId or cnt causes VNET reconfiguration */
    if(NULL!= hWindow->stCurResource.hMcvp)
    {
        bool bDoubleBufSize = false;

        /* Make sure it's bounded by boxmode/hw size */
        uint32_t ulMaxMadWidth, ulMaxMadHeight;
        BVDC_P_Window_GetDeinterlacerMaxResolution_isr(hWindow, pMvdFieldData,
            &ulMaxMadWidth, &ulMaxMadHeight, NULL, true);

        stCapBufRect.ulWidth  = BVDC_P_MIN(stCapBufRect.ulWidth, ulMaxMadWidth);
        stCapBufRect.ulHeight = BVDC_P_MIN(stCapBufRect.ulHeight, ulMaxMadHeight);
        stCapBufRect.ulHeight >>= (pMinSrcFmt) ? (!bMinSrcNoOptimize) : (!bInterlace);

        /* double buffer size for 3D format due to two buffers needed*/
        bDoubleBufSize =
            (bSrcIs3d && BVDC_P_VNET_USED_MVP_AT_WRITER(hWindow->stVnetMode)) ||
            (bDispIs3d && BVDC_P_VNET_USED_MVP_AT_READER(hWindow->stVnetMode));
        bRecfgVnet |= BVDC_P_Window_DecideMcvpBufsCfgs_isr(hWindow, bApplyNewCfg, &stCapBufRect, bDoubleBufSize, eBitDepth, ulPictureIdx, bInterlace);
        if((!bApplyNewCfg) && (bRecfgVnet))
            return true;
    }

    return bRecfgVnet;
}

/***************************************************************************
 * {private}
 *
 * This function determines how the window is going to handle picture
 * cadences. Based on the result, the window will either capture
 * the picture using the original source polarity or alternet the captured
 * picture polarity. On the playback side, the window will either
 * match VEC polarity against the captured picture polarity or simply
 * ignore cadence matching.
 *
 */
static void BVDC_P_Window_SetCadenceHandling_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_MVD_Field            *pMvdFieldData,
      bool                             bTrickMode,
      bool                             bReconfig,
      bool                             bInit )
{
    uint32_t  ulSrcVertRate, ulDstVertRate;
    bool bSrcInterlaced;
    bool bFieldDrop = false;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    /* Reset cadence handling related fields */
    hWindow->stCadHndl.bForceAltCap = false;
    hWindow->stCadHndl.bReaderCadMatching = true;
    hWindow->stCadHndl.bTrickMode = false;

    /* */
    ulDstVertRate = BVDC_P_ROUND_OFF(hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreq,
        BFMT_FREQ_FACTOR/2, BFMT_FREQ_FACTOR);
    ulSrcVertRate = BVDC_P_ROUND_OFF(hWindow->stCurInfo.hSource->ulVertFreq,
        BFMT_FREQ_FACTOR/2, BFMT_FREQ_FACTOR);
    if (BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId) && (NULL != pMvdFieldData))
    {
        bSrcInterlaced = (BAVC_Polarity_eFrame != pMvdFieldData->eSourcePolarity);
        hWindow->stCadHndl.bHandleFldInv =
            (bSrcInterlaced &&
             hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced &&
             !(BVDC_P_MVP_USED_MAD(hWindow->stMvpMode)) &&
             pMvdFieldData->bIgnoreCadenceMatch);
    }
    else
    {
        bSrcInterlaced = hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->bInterlaced;
        hWindow->stCadHndl.bHandleFldInv =
            (bSrcInterlaced &&
             hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced &&
             !(BVDC_P_MVP_USED_MAD(hWindow->stMvpMode)) &&
             ((ulSrcVertRate == 50 && ulDstVertRate == 60) ||
              (ulSrcVertRate == 60 && ulDstVertRate == 50)));
    }

    /* Check if the window is handling the case that decoder does
     * 50->60Hz rate conversion by repeating a field of every five or does
     * 60->50Hz rate conversion by dropping a field of every six.
     * In this case, the pictures scanned out from decoder doesn't follow
     * the T/B/T/B cadence and we have to rely on scaler to correct.
     * Trick mode and non-trick mode are handled the same.
     */

    /* Note: When doing 60->50Hz rate converstion, in addition to dropping
     * one field out of six, another option is dropping two fields out of
     * every twelve fields. In this way, the picture scanned out still follow
     * the cadence. However this come at the price of more motion judder.
     * DM will set "bIgnoreCadenceMatch" to false in this frame drop mode.
     */
    hWindow->stCadHndl.bDecoderRateCov = BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId) &&
                        (((ulSrcVertRate == 60) && (ulDstVertRate == 60) &&
                        (BVDC_P_FrameRate50Hz(pMvdFieldData->eFrameRateCode))) ||
                        ((ulSrcVertRate == 50) && (ulDstVertRate == 50) &&
                        (BVDC_P_FrameRate60Hz(pMvdFieldData->eFrameRateCode))));

    if (hWindow->stCadHndl.bDecoderRateCov && (NULL != pMvdFieldData))
    {
        bFieldDrop = pMvdFieldData->bIgnoreCadenceMatch;
    }

    if (hWindow->bSyncLockSrc)
    {
        /* Sync-locked window reader doesn't match captured picture polarity
         * against VEC. For interlaced source, decoder display manager should
         * guarantee pictures being scanned out have the correct cadence sequence.
         * For progressive source and interlaced display, or decoder performs 50<->60Hz
         * convertion, or in trick modes, VDC will alternate capture polarity for
         * interlaced display if scaler is at writer side.
         */
        hWindow->stCadHndl.bReaderCadMatching = false;

        if ( BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode) &&
             (((!bSrcInterlaced) && (hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced)
             &&(!hWindow->bFrameCapture)) || (hWindow->stCadHndl.bDecoderRateCov && bFieldDrop)))
        {
            hWindow->stCadHndl.bForceAltCap = true;
        }
        else
        {
            /* In this category, force alternating capture is enabled
             * only in trick mode, scaler at writer side and interlaced source.
             */
            if (bTrickMode && BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode)
                && bSrcInterlaced && !hWindow->bFrameCapture)
            {
                hWindow->stCadHndl.bForceAltCap = true;
            }
            else
            {
                hWindow->stCadHndl.bForceAltCap = false;
            }
        }
    }
    else
    {
        /* Always capture as source polarity and no reader cadence
         * match in one of the following senarios:
         *
         *  1) Progressive display.
         *     This is because either the picture has been captured
         *     as frame, or we have MAD/Scaler on the reader side
         *     to convert it. Our VNET configuration must guarantee
         *     this.
         *
         *  2) Picture is captured as frame.
         *     Scaler or VFD (if no scaler at reader side) will convert
         *     the frame to field with proper polarity.
         *
         *  3) MAD is at reader side.
         *     MAD will convert the picture to frame, then the scaler,
         *     which is at the reader side as well will convert the
         *     frame to field with proper polarity.
         */

        if ((!hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced)    ||
            (hWindow->bFrameCapture) ||
            (BVDC_P_MVP_USED_MAD_AT_READER(hWindow->stVnetMode, hWindow->stMvpMode)))
        {
            hWindow->stCadHndl.bForceAltCap = false;
            hWindow->stCadHndl.bReaderCadMatching = false;
        }

        /* Progressive source and capture as field for interlaced display.
         */
        else if ((!bSrcInterlaced) && (!hWindow->bFrameCapture))
        {
            hWindow->stCadHndl.bForceAltCap = true;
            hWindow->stCadHndl.bReaderCadMatching = true;

        }
        /* Interlaced source and interlaced display.
         */
        else
        {
            /* For MPEG source, decoder might convert frame rate from 50Hz to 60Hz or from
             * 60Hz to 50Hz by repeating or dropping field. Cases such as progressive pictures from DM,
             * progressive display and capture as frame are handled as usual.
             *
             * When pictures from DM is interlaced and display is also interlaced, we
             * rely on scaler to do field conversion.
             */
            if (hWindow->stCadHndl.bDecoderRateCov)
            {
                if (BVDC_P_VNET_USED_SCALER_AT_READER(hWindow->stVnetMode) ||
                    (!hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced))
                {
                    /* If scaler is at reader side, we will use it to do polarity correction.
                     * No forced alternative capturing and cadence matching for progressive display.
                     */
                    hWindow->stCadHndl.bForceAltCap = false;
                    hWindow->stCadHndl.bReaderCadMatching = bFieldDrop ? false : true;
                }
                else
                {
                    /* If scaler is at writer side, writer side will generate the
                     * correct T/B/T/B cadence and reader side will match captured
                     * polarity against VEC polarity.
                     */
                    hWindow->stCadHndl.bForceAltCap = bFieldDrop;
                    hWindow->stCadHndl.bReaderCadMatching = true;
                }
            }
            else if (!bTrickMode)
            {
                /* Always capture as source polarity and rely on
                 * reader side to match VEC polarity by repeating.
                 */
                hWindow->stCadHndl.bForceAltCap = false;
                hWindow->stCadHndl.bReaderCadMatching = true;
            }
            else
            {
                if (BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode))
                {
                    hWindow->stCadHndl.bForceAltCap = true;
                    hWindow->stCadHndl.bReaderCadMatching = true;
                }
                else
                {
                    /* Rely on scaler to compensate */
                    hWindow->stCadHndl.bForceAltCap = false;
                    hWindow->stCadHndl.bReaderCadMatching = false;
                }
            }
        }
    }

    /*  SW7125-762: eLastCapPolarity should be reset when bInit is true, indicating
        vnet reconfiguration or reset. Other conditions should be able to be
        removed without adverse effects.
    */

    /*  if (bInit && hWindow->stCadHndl.bForceAltCap &&
            (hWindow->stCadHndl.eLastCapPolarity == BAVC_Polarity_eFrame))
    */
    if (bInit && (bReconfig || (hWindow->stCadHndl.bForceAltCap &&
        (hWindow->stCadHndl.eLastCapPolarity == BAVC_Polarity_eFrame))))
    {
        hWindow->stCadHndl.eLastCapPolarity = BAVC_Polarity_eInvalid;
    }

    BDBG_MSG(( "Win[%d]: ForceAltCap=%d, ReaderCadMatching=%d, TrickMode=%d bFrameCap=%d",
                hWindow->eId, (int) hWindow->stCadHndl.bForceAltCap,
                (int) hWindow->stCadHndl.bReaderCadMatching, (int) bTrickMode, hWindow->bFrameCapture));
}


/***************************************************************************
 * {private}
 *
 * This function detects trick modes and adjusts how the window is going to
 * handle picture cadences.
 */
static void BVDC_P_Window_AdjustCadenceHandling_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_MVD_Field            *pMvdFieldData )
{
    bool bTrickMode;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    /* ignore muted pictures
     */
    if (pMvdFieldData != NULL)
    {
        bTrickMode = pMvdFieldData->bIgnoreCadenceMatch;
    }
    else
    {
        bTrickMode = hWindow->stCadHndl.bTrickMode;
    }

    if (hWindow->stCadHndl.bTrickMode != bTrickMode)
    {
        BVDC_P_Window_SetCadenceHandling_isr(hWindow, pMvdFieldData, bTrickMode, false, false);
        hWindow->stCadHndl.bTrickMode = bTrickMode;

        BDBG_MSG(("Win[%d], %s trick mode", hWindow->eId, bTrickMode ? "enter" : "exit"));
    }
}


/***************************************************************************
 * {private}
 *
 * This function sets a picture's polarity to be captured
 *
 */
static void BVDC_P_Window_SetCapturePolarity_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eFieldId,
      BVDC_P_PictureNode              *pPicture )
{
    uint16_t usMadVsyncDelay = 0;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    if(BVDC_P_MVP_USED_MAD_AT_WRITER(hWindow->stVnetMode, hWindow->stMvpMode))
    {
        usMadVsyncDelay = BVDC_P_Mcdi_GetVsyncDelayNum_isr(
            hWindow->stCurResource.hMcvp->hMcdi,
            hWindow->stCurInfo.stMadSettings.eGameMode);
    }

    /* Predicting capture polarity is needed only if scaler is at writer side
     * Sync-locked path always uses interrupt polarity carried by picture to
     * do prediction. Sync-slipped path always uses source polarity.
     * If MAD is at writer side, delays introduced by it should be offset.
     */
    if (BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode))
    {
        if (hWindow->stCadHndl.bForceAltCap)
        {
            if (hWindow->bSyncLockSrc)
            {
                /* Sync-Locked source guarantees pMvdFieldData field is not NULL */
                pPicture->eDstPolarity = (usMadVsyncDelay % BVDC_P_FIELD_PER_FRAME) ?
                            BVDC_P_NEXT_POLARITY(eFieldId) : eFieldId;
            }
            else
            {
                pPicture->eDstPolarity = (hWindow->stCadHndl.eLastCapPolarity == BAVC_Polarity_eInvalid) ?
                    BAVC_Polarity_eTopField :
                    BVDC_P_NEXT_POLARITY(hWindow->stCadHndl.eLastCapPolarity);
            }
        }
        else
        {
            if ((!hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced) || hWindow->bFrameCapture)
            {
                pPicture->eDstPolarity = BAVC_Polarity_eFrame;
            }
            else
            {
                if (hWindow->bSyncLockSrc)
                {
                    /* Sync-Locked source guarantees pMvdFieldData field is not NULL */
                    pPicture->eDstPolarity = (usMadVsyncDelay % BVDC_P_FIELD_PER_FRAME) ?
                                BVDC_P_NEXT_POLARITY(eFieldId) : eFieldId;
                }
                else
                {
                    pPicture->eDstPolarity = (usMadVsyncDelay % BVDC_P_FIELD_PER_FRAME) ?
                            BVDC_P_NEXT_POLARITY(pPicture->eSrcPolarity) : pPicture->eSrcPolarity;
                }
            }
        }
    }
    else
    {
        pPicture->eDstPolarity = pPicture->eSrcPolarity;
    }

    pPicture->stFlags.bCadMatching = hWindow->stCadHndl.bReaderCadMatching;
    hWindow->stCadHndl.eLastCapPolarity = pPicture->eDstPolarity;
}


/***************************************************************************
 *
 */
static void BVDC_P_Window_UpdateCallback_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eFieldId )
{
    BVDC_P_Window_Info *pCurInfo;
    unsigned int uiNewVsyncDelay, ulVsyncRate, ulSrcVsyncRate;
    uint32_t ulAdjustCnt, ulNewNonProcessVsyncDelay;
    uint32_t ulMpegVecDelay;
    uint16_t usMadVsyncDelay = 0;
    BVDC_Window_CallbackData *pCbData;
    BVDC_Window_CallbackMask *pCbMask;
    const BVDC_Window_CallbackSettings *pCbSettings;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);

    pCurInfo = &hWindow->stCurInfo;

    pCbData = &hWindow->stCbData;
    pCbMask = &pCbData->stMask;
    pCbSettings = &pCurInfo->stCbSettings;

    /* (0) initialize uiNewVsyncDelay and ulAdjustCnt */
    if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId))
    {
        /* actually this is RUL delay: from a picture info is passed to VDC to be built into RUL,
           to when it's actually triggered to be scaned out;  */
        ulMpegVecDelay = BVDC_P_LIP_SYNC_VEC_DELAY;
    }
    else /* PR40895: non-mpeg window shall exclude "vec" delay due to live-in/live-out; */
    {
        ulMpegVecDelay = 0;
    }
    uiNewVsyncDelay = ulAdjustCnt = ulMpegVecDelay;

    /* (1) Update vsync delay (excluding user offset);
              This delay is ideally the center of the drift range excluding the user delay offset. */
    /* (1.1) total multibuffer delay - user offset */
    if(hWindow->bCapture)
    {
        uiNewVsyncDelay += (hWindow->ulBufDelay - pCurInfo->uiVsyncDelayOffset);
    }
    ulNewNonProcessVsyncDelay = uiNewVsyncDelay;

    /* (1.2) deinterlace delay */

    if(BVDC_P_MVP_USED_MAD(hWindow->stMvpMode))
    {
        usMadVsyncDelay = BVDC_P_Mcdi_GetVsyncDelayNum_isr(
            hWindow->stCurResource.hMcvp->hMcdi,
            hWindow->stCurInfo.stMadSettings.eGameMode);
        uiNewVsyncDelay += usMadVsyncDelay;
        ulAdjustCnt     += usMadVsyncDelay;
    }



    /* total delay includes user offset minus VEC delay; this is for MPEG source trigger field swap logic; */
    hWindow->ulTotalBvnDelay = uiNewVsyncDelay + pCurInfo->uiVsyncDelayOffset - ulMpegVecDelay;

    if( pCurInfo->pfGenCallback )
    {
        bool bValidTimestamp = true;
        uint32_t ulCaptureTimestamp = 0, ulPlaybackTimestamp = 0;
        uint32_t ulCurrDriftDelay = 0, ulPrevDriftDelay, ulDeltaDriftDelay;
        uint32_t ulGameModeDelay = 0;
        uint32_t ulUsecPerVsync;
        BVDC_P_PictureNode *pNode;

        /* (2) Update drift delay */
        if( hWindow->bCapture )
        {
            if( hWindow->bSyncLockSrc )
            {
                ulAdjustCnt += hWindow->ulBufDelay;/* don't include extra capture buffers for legacy soft transcoder usage */
            }
            else
            {
                /* Wait until both reader and writer RUL executed.
                 * Check for non-zero is a simple way for that, can also check
                 * actual RUL execution status
                 */
                pNode = hWindow->pCurReaderNode;
                if( pNode )
                {
                    ulCaptureTimestamp  = pNode->ulCaptureTimestamp;
                    ulPlaybackTimestamp = pNode->ulPlaybackTimestamp;
                    hWindow->ulCurBufDelay = BVDC_P_Buffer_CalculateBufDelay_isr(
                        pNode, &bValidTimestamp);
                }

                if( !ulCaptureTimestamp || !ulPlaybackTimestamp )
                {
                    BDBG_MSG(("Wind%d: Wait for valid time stamp", hWindow->eId));
                    bValidTimestamp = false;
                    pCbMask->bDriftDelay = BVDC_P_CLEAN;
                }

                ulGameModeDelay = ulCurrDriftDelay = hWindow->ulCurBufDelay;
            }
        }

        /* Minus user delay offset */
        if(pCurInfo->uiVsyncDelayOffset)
        {
            ulAdjustCnt -= pCurInfo->uiVsyncDelayOffset;
        }

        ulSrcVsyncRate = hWindow->stCurInfo.hSource->ulVertFreq;
        ulVsyncRate = hWindow->hCompositor->hDisplay->stCurInfo.ulVertFreq;
        ulUsecPerVsync = BVDC_P_USEC_ONE_VSYNC_INTERVAL(ulVsyncRate);
        ulCurrDriftDelay += ulAdjustCnt * ulUsecPerVsync;

        /* Check drift delay
         * On a heavy loaded system, VDC can miss interrupts for more than
         * 3 fields. Ignore those cases. The drift delay should
         * be < uiNewVsyncDelay + 1 field + RUL execution time
         */
        if((((uiNewVsyncDelay+1)* ulUsecPerVsync +
            BVDC_P_MULTIBUFFER_RW_TOLERANCE) < ulCurrDriftDelay) && bValidTimestamp)
        {
            BDBG_MODULE_MSG(BVDC_TIMESTAMP,("Win[%d]: Invalid time stamp %d -- %d fields, gamedelay = %d, adj=%d, new=%d",
                hWindow->eId, ulCurrDriftDelay,
                ulCurrDriftDelay / ulUsecPerVsync, ulGameModeDelay, ulAdjustCnt, uiNewVsyncDelay));
            bValidTimestamp = false;
        }

        /* Update drift delay change */
        ulPrevDriftDelay = pCbData->ulDriftDelay;
        if( ulPrevDriftDelay < ulCurrDriftDelay )
        {
            ulDeltaDriftDelay = ulCurrDriftDelay - ulPrevDriftDelay;
        }
        else
        {
            ulDeltaDriftDelay = ulPrevDriftDelay - ulCurrDriftDelay;
        }

        /* NOTE: BVN path delay is ignored for NRT transcode mode since BVN
         * doesn't insert bubble pictures to STG/ViCE2!  Only DM non-ignore
         * picture will be encoded in NRT mode!  That means NRT mode AV sync
         * delay matching doesn't need to count the BVN vsync delay! */
#if (BVDC_P_SUPPORT_STG)
        if(hWindow->hCompositor->hDisplay->stCurInfo.bStgNonRealTime) {
            uiNewVsyncDelay = 0;
            ulNewNonProcessVsyncDelay = 0;
        }
#endif

        /* forced frame capture assumed to drop every other writer buffer isr, so vsync delay is doubled based on display vsync rate */
        /* assume display is 60hz; TODO: based on actual display/source rate to adjust vsync delay */
        uiNewVsyncDelay = uiNewVsyncDelay << hWindow->stCurInfo.hSource->stCurInfo.bForceFrameCapture;
        ulNewNonProcessVsyncDelay = ulNewNonProcessVsyncDelay << hWindow->stCurInfo.hSource->stCurInfo.bForceFrameCapture;

        /* (3) Which one triggers callback? */
        if(((uiNewVsyncDelay != pCbData->ulVsyncDelay) ||
            (ulNewNonProcessVsyncDelay != pCbData->ulNonProcessVsyncDelay)  ||
            (ulVsyncRate     != pCbData->ulVsyncRate)  ||
            (pCbData->ulVsyncDelay == BVDC_P_LIP_SYNC_RESET_DELAY)) &&
             pCbSettings->stMask.bVsyncDelay)
        {
            pCbMask->bVsyncDelay = BVDC_P_DIRTY;
        }

        if(((ulDeltaDriftDelay >= pCbSettings->ulLipSyncTolerance) ||
            (pCbData->ulDriftDelay == BVDC_P_LIP_SYNC_RESET_DELAY)) &&
            bValidTimestamp && pCbSettings->stMask.bDriftDelay)
        {
            pCbMask->bDriftDelay = BVDC_P_DIRTY;
        }

        /* Check if game mode delay is out of range.
         */
        if(((ulGameModeDelay >
            (pCbSettings->ulGameModeReadWritePhaseDiff + pCbSettings->ulGameModeTolerance)) ||
           (ulGameModeDelay < (pCbSettings->ulGameModeReadWritePhaseDiff - pCbSettings->ulGameModeTolerance))) &&
           bValidTimestamp && pCbSettings->stMask.bGameModeDelay)
        {
            pCbMask->bGameModeDelay = BVDC_P_DIRTY;
        }

        if(pCbSettings->stMask.bCrc) {
#if BVDC_P_SUPPORT_STG /* SW7445-2544: don't callback CRC for ignore picture in NRT mode */
            if((BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg) &&
                hWindow->hCompositor->hDisplay->stCurInfo.bStgNonRealTime &&
                /* this isr is executed before cmp updates the ignore flag */
                hWindow->hCompositor->bCrcToIgnore)) {
                pCbMask->bCrc = BVDC_P_CLEAN;
            } else
#endif
            pCbMask->bCrc = BVDC_P_DIRTY;
        }

        if(BVDC_P_CB_IS_DIRTY_isr(pCbMask))
        {
            /* Update Callback data */
            if(pCbMask->bVsyncDelay)
            {
                pCbData->ulVsyncDelay = uiNewVsyncDelay;
                pCbData->ulVsyncRate  = ulVsyncRate;
                pCbData->ulSrcVsyncRate  = ulSrcVsyncRate;
                pCbData->ulNonProcessVsyncDelay = ulNewNonProcessVsyncDelay;

                BDBG_MSG(("Window[%d] callback bVsyncDelay: ", hWindow->eId));
                BDBG_MSG(("    ulVsyncDelay = %d, ulNonProcessingVsyncDelay = %d",
                    pCbData->ulVsyncDelay, pCbData->ulNonProcessVsyncDelay));
            }

            if(pCbMask->bDriftDelay)
            {
                pCbData->ulDriftDelay = ulCurrDriftDelay;
                BDBG_MSG(("Window[%d] callback bDriftDelay: ", hWindow->eId));
                BDBG_MSG(("    W = %d, R = %d", ulCaptureTimestamp, ulPlaybackTimestamp));
                BDBG_MSG(("    ulDriftDelay = %d (%d fields)", pCbData->ulDriftDelay,
                    pCbData->ulDriftDelay / ulUsecPerVsync));
            }

            /* Note: game mode delay here is purely multi-buffer delay; */
            if(pCbMask->bGameModeDelay)
            {
                pCbData->ulGameModeDelay = ulGameModeDelay;
                BDBG_MSG(("Window[%d] callback bGameModeDelay: ", hWindow->eId));
                BDBG_MSG(("    W = %d, R = %d", ulCaptureTimestamp, ulPlaybackTimestamp));
                BDBG_MSG(("    ulGameModeDelay = %d (%d fields)", pCbData->ulGameModeDelay,
                    pCbData->ulGameModeDelay / ulUsecPerVsync));
                BDBG_MSG(("Reader B%d : Writer B%d", hWindow->hBuffer->pCurReaderBuf->ulBufferId,
                    hWindow->hBuffer->pCurWriterBuf->ulBufferId));
            }

            if(pCbMask->bRectAdjust)
            {
                /* Match CMP_0_V0_SURFACE_OFFSET.Y_OFFSET and
                * CMP_0_V0_SURFACE_SIZE.VSIZE settings in
                * BVDC_P_Window_SetSurfaceSize_isr  */
                if(eFieldId != BAVC_Polarity_eFrame)
                {
                    pCbData->stOutputRect.lTop = BVDC_P_ALIGN_DN(
                        pCbData->stOutputRect.lTop, 2) ;
                    pCbData->stOutputRect.ulHeight = BVDC_P_ALIGN_DN(
                        pCbData->stOutputRect.ulHeight, 2) ;
                }

                BDBG_MSG(("Window[%d] callback bRectAdjust: ", hWindow->eId));
                BDBG_MSG(("     %d %d %d %d", pCbData->stOutputRect.lLeft,
                    pCbData->stOutputRect.lTop, pCbData->stOutputRect.ulWidth,
                    pCbData->stOutputRect.ulHeight));
            }

            if(pCbMask->bSyncLock)
            {
                pCbData->bSyncLock = hWindow->bSyncLockSrc;
                BDBG_MSG(("Window[%d] callback bSyncLock: ", hWindow->eId));
                BDBG_MSG(("     %d", pCbData->bSyncLock));
            }

            if(pCbMask->bCrc)
            {
                pCbData->ulCrcLuma = hWindow->stCurResource.hVnetCrc->ulCrcLuma;
                pCbData->ulCrcChroma = hWindow->stCurResource.hVnetCrc->ulCrcChroma;
                BDBG_MSG(("Window[%d] callback CrcLuma 0x%08x, CrcChroma 0x%08x",
                          hWindow->eId, pCbData->ulCrcLuma, pCbData->ulCrcChroma));
            }
            /* Callback application with the above data */
            pCurInfo->pfGenCallback(pCurInfo->pvGenCallbackParm1,
                pCurInfo->iGenCallbackParm2, (void*)pCbData);

            /* clear dirty bits */
            BVDC_P_CB_CLEAN_ALL_DIRTY(pCbMask);
        }
    }

    return;
}

/* set SrcMuxValue for each module, from front to back
 * note: a sub-module might be shared by more than one window, but the
 * sharing is handled by each sub-module. And each sub-module will
 * acquire and release the pre-patch of free-channel or loop-back
 * internally basing on eVnetPatchMode.
 */
static BERR_Code BVDC_P_Window_SetWriterVnet_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_VnetMode                 *pVnetMode )
{
    BVDC_P_VnetPatch  eVnetPatchMode;
    uint32_t  ulSrcMuxValue;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor->hVdc, BVDC_VDC);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);

    hWindow->hCompositor->hVdc->bForcePrint
        ? BDBG_ERR(("Set writer vnet, win %d, vnetMode 0x%x ", hWindow->eId, *(unsigned int *)pVnetMode))
        : BDBG_MSG(("Set writer vnet, win %d, vnetMode 0x%x ", hWindow->eId, *(unsigned int *)pVnetMode));

    eVnetPatchMode = BVDC_P_VnetPatch_eNone;
    ulSrcMuxValue = BVDC_P_Source_PostMuxValue(hWindow->stCurInfo.hSource);

    if(BVDC_P_VNET_USED_VNETCRC_AT_WRITER(*pVnetMode))
    {
        BVDC_P_VnetCrc_SetVnet_isr(hWindow->stCurResource.hVnetCrc);
    }

#if (BVDC_P_SUPPORT_DNR)
    /* DNR is always upstream of ANR */
    if(BVDC_P_VNET_USED_DNR_AT_WRITER(*pVnetMode))
    {
        BVDC_P_Dnr_SetVnet_isr(hWindow->stCurResource.hDnr, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Dnr_PostMuxValue(hWindow->stCurResource.hDnr); /* put hDnr in win ? */
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }
#endif

#if (BVDC_P_SUPPORT_XSRC)
    if(BVDC_P_VNET_USED_XSRC_AT_WRITER(*pVnetMode))
    {
        BVDC_P_Xsrc_SetVnet_isr(hWindow->stCurResource.hXsrc, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Xsrc_PostMuxValue(hWindow->stCurResource.hXsrc);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }
#endif

#if (BVDC_P_SUPPORT_VFC)
    if(BVDC_P_VNET_USED_VFC_AT_WRITER(*pVnetMode))
    {
        BVDC_P_Vfc_SetVnet_isr(hWindow->stCurResource.hVfc, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Vfc_PostMuxValue(hWindow->stCurResource.hVfc);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }
#endif

    /* note: with 7420, BVDC_P_SUPPORT_HSCL/ANR/MAD is 0, and
     * BVDC_P_MVP_USED_HSCL/ANR/MAD(hWindow->stVnetMode) is false too */
    if(BVDC_P_VNET_USED_MVP_AT_WRITER(*pVnetMode))
    {
        BVDC_P_Mcvp_SetVnetAllocBuf_isr(hWindow->stCurResource.hMcvp, ulSrcMuxValue, eVnetPatchMode,
            true);
        ulSrcMuxValue = BVDC_P_Mcvp_PostMuxValue(hWindow->stCurResource.hMcvp);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_WRITER(*pVnetMode) && pVnetMode->stBits.bTntdBeforeScl)
    {
        BVDC_P_Tntd_SetVnet_isr(hWindow->stCurResource.hTntd, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Tntd_PostMuxValue(hWindow->stCurResource.hTntd);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }
#endif

    if(BVDC_P_VNET_USED_SCALER_AT_WRITER(*pVnetMode))
    {
        BVDC_P_Scaler_SetVnet_isr(hWindow->stCurResource.hScaler, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Scaler_PostMuxValue(hWindow->stCurResource.hScaler);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_WRITER(*pVnetMode) && !pVnetMode->stBits.bTntdBeforeScl)
    {
        BVDC_P_Tntd_SetVnet_isr(hWindow->stCurResource.hTntd, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Tntd_PostMuxValue(hWindow->stCurResource.hTntd);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }
#endif

    if(BVDC_P_VNET_USED_CAPTURE(*pVnetMode))
    {
        eVnetPatchMode = (BVDC_P_VnetPatch_eLpBack == eVnetPatchMode) ?
            BVDC_P_VnetPatch_eNone : BVDC_P_VnetPatch_eFreeCh;
        /* cap bufs will be allocated in writer_isr together with handling of
         * buf adding/dropping requested by user */
        BVDC_P_Capture_SetVnet_isr(hWindow->stCurResource.hCapture, ulSrcMuxValue, eVnetPatchMode);
    }

    return BERR_SUCCESS;
}


/* set SrcMuxValue for each module, from front to back
 * note: a sub-module might be shared by more than one window, but the
 * sharing is handled by each sub-module. And each sub-module will
 * acquire and release the pre-patch of free-channel or loop-back
 * internally basing on eVnetPatchMode.
 */
static BERR_Code BVDC_P_Window_SetReaderVnet_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_VnetMode                 *pVnetMode )
{
    BVDC_P_VnetPatch  eVnetPatchMode;
    uint32_t  ulSrcMuxValue;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_MSG(("Set reader vnet, win %d, vnetMode 0x%x ", hWindow->eId, *(unsigned int *)pVnetMode));

    eVnetPatchMode = BVDC_P_VnetPatch_eNone;
    if(BVDC_P_VNET_USED_CAPTURE(*pVnetMode))
    {
        ulSrcMuxValue = BVDC_P_Feeder_PostMuxValue(hWindow->stCurResource.hPlayback);
    }
    else
    {
        ulSrcMuxValue = BVDC_P_Source_PostMuxValue(hWindow->stCurInfo.hSource);
    }

#if (BVDC_P_SUPPORT_DNR)
    if(BVDC_P_VNET_USED_DNR_AT_READER(*pVnetMode))
    {
        BVDC_P_Dnr_SetVnet_isr(hWindow->stCurResource.hDnr, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Dnr_PostMuxValue(hWindow->stCurResource.hDnr); /* put hDnr in win ? */
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }
#endif

#if (BVDC_P_SUPPORT_XSRC)
    if(BVDC_P_VNET_USED_XSRC_AT_READER(*pVnetMode))
    {
        BVDC_P_Xsrc_SetVnet_isr(hWindow->stCurResource.hXsrc, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Xsrc_PostMuxValue(hWindow->stCurResource.hXsrc);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }
#endif

#if (BVDC_P_SUPPORT_VFC)
    if(BVDC_P_VNET_USED_VFC_AT_READER(*pVnetMode))
    {
        BVDC_P_Vfc_SetVnet_isr(hWindow->stCurResource.hVfc, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Vfc_PostMuxValue(hWindow->stCurResource.hVfc);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }
#endif

    /* note: with 7420, BVDC_P_SUPPORT_HSCL/ANR/MAD is 0, and
     * BVDC_P_MVP_USED_HSCL/ANR/MAD(hWindow->stVnetMode) is false too */
    if(BVDC_P_VNET_USED_MVP_AT_READER(*pVnetMode))
    {
        BVDC_P_Mcvp_SetVnetAllocBuf_isr(hWindow->stCurResource.hMcvp, ulSrcMuxValue, eVnetPatchMode,
            true);
        ulSrcMuxValue = BVDC_P_Mcvp_PostMuxValue(hWindow->stCurResource.hMcvp);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_READER(*pVnetMode) && pVnetMode->stBits.bTntdBeforeScl)
    {
        BVDC_P_Tntd_SetVnet_isr(hWindow->stCurResource.hTntd, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Tntd_PostMuxValue(hWindow->stCurResource.hTntd);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }
#endif

    if(BVDC_P_VNET_USED_SCALER_AT_READER(*pVnetMode))
    {
        BVDC_P_Scaler_SetVnet_isr(hWindow->stCurResource.hScaler, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Scaler_PostMuxValue(hWindow->stCurResource.hScaler);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_READER(*pVnetMode) && !pVnetMode->stBits.bTntdBeforeScl)
    {
        BVDC_P_Tntd_SetVnet_isr(hWindow->stCurResource.hTntd, ulSrcMuxValue, eVnetPatchMode);
        ulSrcMuxValue = BVDC_P_Tntd_PostMuxValue(hWindow->stCurResource.hTntd);
        eVnetPatchMode = BVDC_P_VnetPatch_eLpBack;
    }
#endif

    /* set vnet for compositor's win src */
    eVnetPatchMode = (BVDC_P_VnetPatch_eLpBack == eVnetPatchMode) ?
        BVDC_P_VnetPatch_eNone : BVDC_P_VnetPatch_eFreeCh;
    BVDC_P_SubRul_SetVnet_isr(&(hWindow->stWinOutVnet),
        ulSrcMuxValue, eVnetPatchMode);

    if(BVDC_P_VNET_USED_VNETCRC_AT_READER(*pVnetMode))
    {
        BVDC_P_VnetCrc_SetVnet_isr(hWindow->stCurResource.hVnetCrc);
    }

    return BERR_SUCCESS;
}

/* NOTE: This function will release software resources, not hw vnet disconnection!
 * The hw vnet is disconnected by SHUTDOWNRUL state!
 * release the free-channel or loop-back that might have been used to patch
 * the vnet. all bufs used by mad, anr and cap are released togerther inside
 * this function to avoid fragmentation.  It must be called after reader's
 * vnet shut-down rul and writer's drain rul are executed
 */
static void BVDC_P_Window_UnsetWriterVnet_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_VnetMode                 *pVnetMode )
{
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_MSG(("Unset writer vnet, win %d, vnetMode 0x%x ", hWindow->eId, *(unsigned int *)pVnetMode));

    if(BVDC_P_VNET_IS_INVALID(*pVnetMode))
        return;

    if(BVDC_P_VNET_USED_VNETCRC_AT_WRITER(*pVnetMode))
        BVDC_P_VnetCrc_UnsetVnet_isr(hWindow->stCurResource.hVnetCrc);

#if (BVDC_P_SUPPORT_DNR)
    if(BVDC_P_VNET_USED_DNR_AT_WRITER(*pVnetMode))
        BVDC_P_Dnr_UnsetVnet_isr(hWindow->stCurResource.hDnr);
#endif

#if (BVDC_P_SUPPORT_XSRC)
    if(BVDC_P_VNET_USED_XSRC_AT_WRITER(*pVnetMode))
        BVDC_P_Xsrc_UnsetVnet_isr(hWindow->stCurResource.hXsrc);
#endif

#if (BVDC_P_SUPPORT_VFC)
    if(BVDC_P_VNET_USED_VFC_AT_WRITER(*pVnetMode))
        BVDC_P_Vfc_UnsetVnet_isr(hWindow->stCurResource.hVfc);
#endif

    /* this will also release anr/mad bufs.
     * note: with 7420, BVDC_P_SUPPORT_HSCL/ANR/MAD is 0, and
     * BVDC_P_MVP_USED_HSCL/ANR/MAD(hWindow->stVnetMode) is false too */
    if(BVDC_P_VNET_USED_MVP_AT_WRITER(*pVnetMode))
        BVDC_P_Mcvp_UnsetVnetFreeBuf_isr(hWindow->stCurResource.hMcvp);

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_WRITER(*pVnetMode) && pVnetMode->stBits.bTntdBeforeScl)
        BVDC_P_Tntd_UnsetVnet_isr(hWindow->stCurResource.hTntd);
#endif

    if(BVDC_P_VNET_USED_SCALER_AT_WRITER(*pVnetMode))
        BVDC_P_Scaler_UnsetVnet_isr(hWindow->stCurResource.hScaler);

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_WRITER(*pVnetMode) && !pVnetMode->stBits.bTntdBeforeScl)
        BVDC_P_Tntd_UnsetVnet_isr(hWindow->stCurResource.hTntd);
#endif

    if(BVDC_P_VNET_USED_CAPTURE(*pVnetMode))
    {
        uint32_t  ulCount;

        /* Release cap buffer after synch with reader shutdown */
        ulCount = hWindow->hBuffer->ulActiveBufCnt;
        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] free %d cap buffers (%s)", hWindow->eId, ulCount,
            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdRequest)));

        if(((hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate) &&
             (BVDC_P_CLEAN == hWindow->stCurInfo.stDirty.stBits.bBufAllocMode)) ||
           ((hWindow->ePrevBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate)  &&
             (BVDC_P_DIRTY == hWindow->stCurInfo.stDirty.stBits.bBufAllocMode)))
        {
            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] free %d cap buffers (%s) for right", hWindow->eId, ulCount,
                BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdRequest)));

            BVDC_P_Buffer_ReleasePictureNodes_isr(hWindow->hBuffer,
                hWindow->apHeapNode, hWindow->apHeapNode_R, ulCount, hWindow->ulBufDelay);
            BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hCapHeap,
                hWindow->apHeapNode_R, ulCount, false);

            hWindow->stCurInfo.stDirty.stBits.bBufAllocMode = BVDC_P_CLEAN;
        }
        else
        {
            BVDC_P_Buffer_ReleasePictureNodes_isr(hWindow->hBuffer,
                hWindow->apHeapNode, NULL, ulCount, hWindow->ulBufDelay);
        }
        BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hCapHeap,
            hWindow->apHeapNode, ulCount, false);

        /* User capture buffer is explicitly released by user so that we
         * won't interrupt its usage.
         */
        hWindow->ulBufCntAllocated -= ulCount;

        /* release capture vnet resource */
        BVDC_P_Capture_UnsetVnet_isr(hWindow->stCurResource.hCapture);
    }

    return;
}

/* NOTE: This function will release software resources, not hw vnet disconnection!
 * The hw vnet is disconnected by SHUTDOWNRUL state!
 * release the free-channel or loop-back that might have been used to patch
 * the vnet. It is called after vnet shut-down and drain rul is executed
 */
static void BVDC_P_Window_UnsetReaderVnet_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_VnetMode                 *pVnetMode )
{
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_MSG(("Unset reader vnet, win %d, vnetMode 0x%x ", hWindow->eId, *(unsigned int *)pVnetMode));

    if(BVDC_P_VNET_IS_INVALID(*pVnetMode))
        return;

#if (BVDC_P_SUPPORT_DNR)
    if(BVDC_P_VNET_USED_DNR_AT_READER(*pVnetMode))
        BVDC_P_Dnr_UnsetVnet_isr(hWindow->stCurResource.hDnr);
#endif

#if (BVDC_P_SUPPORT_XSRC)
    if(BVDC_P_VNET_USED_XSRC_AT_READER(*pVnetMode))
        BVDC_P_Xsrc_UnsetVnet_isr(hWindow->stCurResource.hXsrc);
#endif

#if (BVDC_P_SUPPORT_VFC)
    if(BVDC_P_VNET_USED_VFC_AT_READER(*pVnetMode))
        BVDC_P_Vfc_UnsetVnet_isr(hWindow->stCurResource.hVfc);
#endif

    /* note: with 7420, BVDC_P_SUPPORT_HSCL/ANR/MAD is 0, and
     * BVDC_P_MVP_USED_HSCL/ANR/MAD(hWindow->stVnetMode) is false too */
    if(BVDC_P_VNET_USED_MVP_AT_READER(*pVnetMode))
        BVDC_P_Mcvp_UnsetVnetFreeBuf_isr(hWindow->stCurResource.hMcvp);

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_READER(*pVnetMode) && pVnetMode->stBits.bTntdBeforeScl)
        BVDC_P_Tntd_UnsetVnet_isr(hWindow->stCurResource.hTntd);
#endif

    if(BVDC_P_VNET_USED_SCALER_AT_READER(*pVnetMode))
        BVDC_P_Scaler_UnsetVnet_isr(hWindow->stCurResource.hScaler);

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_READER(*pVnetMode) && !pVnetMode->stBits.bTntdBeforeScl)
        BVDC_P_Tntd_UnsetVnet_isr(hWindow->stCurResource.hTntd);
#endif

    if(BVDC_P_VNET_USED_VNETCRC_AT_READER(*pVnetMode))
        BVDC_P_VnetCrc_UnsetVnet_isr(hWindow->stCurResource.hVnetCrc);

    /* unset vnet for compositor's win src */
    BVDC_P_SubRul_UnsetVnet_isr(&(hWindow->stWinOutVnet));

    return;
}

/***************************************************************************
 * {private}
 *
 * writer vnet shut down process.
 */
static void BVDC_P_Window_WriterShutDown_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eFieldId,
      BVDC_P_ListInfo                 *pList )
{
    bool bLastExecuted;
    BVDC_P_Window_Info *pCurInfo;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);

    /* Get dirty bits to check if needed to rebuild csc rul. */
    pCurInfo  = &hWindow->stCurInfo;

    /* to prevent back-to-back callbacks prematurely exiting writer
     * shutdown without execution;
     * Note: progressive format uses the same slot for the next RUL
     * as the current one; */
    if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId) &&
       (BAVC_Polarity_eFrame != eFieldId))
    {
        BRDC_Slot_Handle hCurSlot;
        BRDC_List_Handle hCurList;
        BAVC_Polarity eNextFieldId = BVDC_P_NEXT_POLARITY(eFieldId);

        hCurSlot = BVDC_P_SRC_GET_SLOT(hWindow->stCurInfo.hSource, eNextFieldId);
        hCurList = BVDC_P_SRC_GET_LIST(hWindow->stCurInfo.hSource, eNextFieldId);
        BRDC_Slot_UpdateLastRulStatus_isr(hCurSlot, hCurList, true);
        bLastExecuted = BRDC_List_GetLastExecStatus_isr(hCurList);
        BDBG_MSG(("Mpeg window (W) current slot bExe = %d for field %d",
                  bLastExecuted, eNextFieldId));
    }
    else
    {
        bLastExecuted = pList->bLastExecuted;
    }

    /* Shutdown writer as soon as possible, then synchronize on reader
     * shutdown process before releasing cap bufs */
    if(((false == bLastExecuted) &&
        (BVDC_P_State_eShutDownRul == pCurInfo->eWriterState)) ||
       (BVDC_P_State_eShutDownPending == pCurInfo->eWriterState))
    {
        if((!BVDC_P_VNET_IS_INVALID(hWindow->stVnetMode)))
        {
            /* in case another win is sharing this MFD/dnr/xsrc, we don't want to
             * start MFD/dnr/xsrc before this win drops off from it */
            if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId))
            {
                BDBG_MODULE_MSG(BVDC_WIN_VNET,("Win[%d] set MFD%u eBldWin", hWindow->eId, hWindow->stCurInfo.hSource->hMpegFeeder->eId));
                BVDC_P_feeder_SetRulBuildWinId_isr(hWindow->stCurInfo.hSource->hMpegFeeder, hWindow->eId);
            }

#if (BVDC_P_SUPPORT_DNR)
            if(BVDC_P_VNET_USED_DNR(hWindow->stVnetMode))
            {
                BDBG_MODULE_MSG(BVDC_WIN_VNET,("WIN[%d] set DNR%u eBldWin", hWindow->eId, hWindow->stCurResource.hDnr->eId));
                BVDC_P_Dnr_SetRulBuildWinId_isr(hWindow->stCurResource.hDnr, hWindow->eId);
            }
#endif

#if (BVDC_P_SUPPORT_XSRC)
            if(BVDC_P_VNET_USED_XSRC(hWindow->stVnetMode))
            {
                BDBG_MODULE_MSG(BVDC_WIN_VNET,("WIN[%d] set XSRC%u eBldWin", hWindow->eId, hWindow->stCurResource.hXsrc->eId));
                BVDC_P_Xsrc_SetRulBuildWinId_isr(hWindow->stCurResource.hXsrc, hWindow->eId);
            }
#endif
        }

        pCurInfo->eWriterState = BVDC_P_State_eShutDownRul;
        BDBG_MSG(("(3.a) Window[%d] Slip(W%d) is BVDC_P_State_eShutDownRul, bExe = %d, vnet_invalid=%d, eRdState=%d",
                  hWindow->eId, eFieldId, bLastExecuted, BVDC_P_VNET_IS_INVALID(hWindow->stVnetMode), pCurInfo->eReaderState));
    }
    else if(((false == bLastExecuted) &&
             (BVDC_P_State_eDrainVnet == pCurInfo->eWriterState)) ||
            (BVDC_P_State_eShutDownRul == pCurInfo->eWriterState))
    {
        pCurInfo->eWriterState = BVDC_P_State_eDrainVnet;
        BDBG_MSG(("(3.b) Window[%d] Slip(W%d) is BVDC_P_State_eDrainVnet, bExe = %d",
                  hWindow->eId, eFieldId, bLastExecuted));
    }
    else /* synchronize with reader shutdown here */
    if((BVDC_P_State_eDrainVnet == pCurInfo->eWriterState) &&
       ((BVDC_P_State_eShutDown == pCurInfo->eReaderState) ||
        (hWindow->bSyncLockSrc)))
    {
        /* clear off writer side vnet sw infor and release resources including loop-back,
         * free-channel, mad/anr/cap bufs; It must be called after reader's vnet shut-down
         * rul and writer's drain rul are executed.
         * note: it is state BVDC_P_State_eShutDownRul that causes each module to build
         * into RUL to disable HW and to drop off from vnet */
        BVDC_P_Window_UnsetWriterVnet_isr(hWindow, &hWindow->stVnetMode);

        pCurInfo->eWriterState = BVDC_P_State_eShutDown;
        BDBG_MSG(("(3.c) Window[%d] Slip(W%d) is BVDC_P_State_eShutDown, bExe = %d",
                  hWindow->eId, eFieldId, bLastExecuted));
    }
    else if(BVDC_P_State_eShutDown == pCurInfo->eWriterState)
    {
        /* similar protection at writer side; */
        BDBG_MSG(("(*) Window[%d] (W%d) stays off", hWindow->eId, eFieldId));
    }
}


/***************************************************************************
 * {private}
 * This function does the following as needed
 *    (1) change the reader state according to writer state.
 *    (2) set flush data flag for mute color buffer
 */
static void BVDC_P_Window_AdjustRdState_isr
    ( BVDC_Window_Handle               hWindow,
      bool*                            pbFlushPicQueue)
{
    BVDC_P_PictureNode *pPicture;

    /* both writer vnet and reader vnet are configured in writer_isr right after new
     * vnetMode is decided; writerState changes to Active there too. readerState neeed
     * to wait for the muted capture buffers are flushed. should not move this to
     * reader_isr in case that reader_isr is called before writer_isr as syncLock */
    if(BVDC_P_State_eInactive == hWindow->stCurInfo.eReaderState)
    {
        if (BVDC_P_SRC_IS_VFD(hWindow->stCurInfo.hSource->eId))
        {
            pPicture = &hWindow->stCurInfo.hSource->hVfdFeeder->stPicture;
        }
        else
        {
            pPicture = BVDC_P_Buffer_GetCurrReaderNode_isr(hWindow->hBuffer);
        }

        if(BVDC_P_State_eActive == hWindow->stCurInfo.eWriterState)
        {
            if(false == pPicture->stFlags.bMute)
            {
                /* this will make VDC modules in reader to build vnet and enabling
                 * into RUL. */
                hWindow->stCurInfo.eReaderState = BVDC_P_State_eActive;
                BDBG_MODULE_MSG(BVDC_WIN_VNET,("Win[%d] ReaderState --> eActive", hWindow->eId));
            }
            else
            {
                /* flush mute color buffer */
                if((NULL !=pbFlushPicQueue) && (BVDC_P_VNET_USED_MVP(hWindow->stVnetMode)))
                    *pbFlushPicQueue = true;
            }
        }
#if (BVDC_P_SHOW_VNET_MSG==1)
        else if((false == pPicture->stFlags.bMute) &&
                 (BVDC_P_State_eActive != hWindow->stCurInfo.eWriterState) &&
                 (false == hWindow->stCurInfo.stDirty.stBits.bSrcPending) &&
                 (false == hWindow->stCurInfo.stDirty.stBits.bBufferPending))
        {
            BDBG_MODULE_MSG(BVDC_WIN_VNET,("Win[%d] reader see pic un-muted before writeState to eActive", hWindow->eId));
        }
#endif
    }

}

/***************************************************************************
 * {private}
 *
 * This WRITER function does the following in order:
 *   (1) handle source pending (new vnet-start pause and resume)
 *   (2) put user returned captured buffer back to hBuffer
 *   (3) start new vnet, if we just created the window, or we just completed
 *       the shuting-downn of the old vnet; otherwise, check if we need to
 *       reconfigure vnet, and start shutdown process if we need to.
 *   (4) add/release, and flush cap-usr buf
 *   (5) setup picture node
 *   (6) Set Info (register softeare shadow) for each sub-modules in
 *       write vnet
 *
 * To be called with source _isr (every vsync).
 */
void BVDC_P_Window_Writer_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_MVD_Field            *pMvdFieldData,
      const BAVC_VDC_HdDvi_Picture    *pXvdFieldData,
      const BAVC_Polarity              eFieldId,
      BVDC_P_ListInfo                 *pList,
      uint32_t                         ulPictureIdx )
{
    BVDC_P_Window_Info *pCurInfo;
    BVDC_P_Window_DirtyBits *pCurDirty;
    BVDC_P_PictureNode   *pPicture;
    const BVDC_P_Source_DirtyBits *pSrcDirty;
    const BVDC_P_Source_Info *pSrcInfo;
    bool bVideoDetect = false;
    bool bFlushPicQueue = false;
    BERR_Code  err = BERR_SUCCESS;

    BDBG_ENTER(BVDC_P_Window_Writer_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);

    /* Get dirty bits to check if needed to rebuild csc rul. */
    pSrcInfo  = &hWindow->stCurInfo.hSource->stCurInfo;
    pCurInfo  = &hWindow->stCurInfo;
    pCurDirty = &hWindow->stCurInfo.stDirty;
    pSrcDirty = &hWindow->stCurInfo.hSource->stCurInfo.stDirty;

    /* Only video window has writer (gfx and others does not). */
    BDBG_ASSERT(BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId));

    /* --------------------------------------------------------------- */
    /* (1) handle source pending (new vnet-start pause and resume      */
    /* --------------------------------------------------------------- */
    if(BVDC_P_IS_DIRTY(pSrcDirty))
    {
        if((pSrcInfo->eResumeMode) &&
           (pSrcDirty->stBits.bInputFormat))
        {
            BDBG_MSG(("Source[%d] is pending mode!", hWindow->stCurInfo.hSource->eId));
            BVDC_P_Window_SetReconfiguring_isr(hWindow, true, false, false);
        }

        if((pSrcDirty->stBits.bResume) &&
           (pCurDirty->stBits.bSrcPending))
        {
            pCurDirty->stBits.bSrcPending = BVDC_P_CLEAN;
            BDBG_MSG(("Source[%d] is now resumed!", hWindow->stCurInfo.hSource->eId));
            BVDC_P_Window_SetReconfiguring_isr(hWindow, false, true, false);
        }

        if(pCurDirty->stBits.bBufferPending)
        {
            /* Clear bBufferPending so we can re-check if there is enough
             * buffers for source change */
            pCurDirty->stBits.bBufferPending = BVDC_P_CLEAN;
            BDBG_MSG(("Source[%d] is now resumed!", hWindow->stCurInfo.hSource->eId));
            BVDC_P_Window_SetReconfiguring_isr(hWindow, false, true, false);
        }

#if BVDC_P_SUPPORT_MOSAIC_DEINTERLACE
        if((pSrcInfo->bMosaicMode) && (pSrcDirty->stBits.bMosaicIntra) &&
           (hWindow->stCurInfo.bDeinterlace || hWindow->stCurInfo.bAnr))
        {
            BDBG_MSG(("Source[%d] is mosaic add window!", hWindow->stCurInfo.hSource->eId));
            pCurDirty->stBits.bRecAdjust = BVDC_P_DIRTY;
            pCurDirty->stBits.bReDetVnet = BVDC_P_DIRTY;

            if(ulPictureIdx == 0)
                pCurDirty->stBits.bMosaicMode = BVDC_P_DIRTY;
        }
#endif

        if(pSrcDirty->stBits.bOrientation ||
        ((pSrcInfo->eOrientation == BFMT_Orientation_e2D) &&
           hWindow->stCurInfo.hSource->stCurInfo.bOrientationOverride))
        {
            bool  bSrcIs3d, bDispIs3d;
            BVDC_P_BufHeapAllocMode  eBufAllocMode;

            BVDC_P_Window_GetOrientation_isr(hWindow, &bSrcIs3d, &bDispIs3d);
            BVDC_P_Window_GetBufAllocMode_isr(hWindow, bSrcIs3d, bDispIs3d, &eBufAllocMode);

            hWindow->ePrevBufAllocMode = hWindow->eBufAllocMode;
            hWindow->eBufAllocMode = eBufAllocMode;
            pCurDirty->stBits.bBufAllocMode = BVDC_P_DIRTY;
            pCurDirty->stBits.bReDetVnet = BVDC_P_DIRTY;
            pCurDirty->stBits.bRecAdjust = BVDC_P_DIRTY;
        }
    }

    if(BVDC_P_IS_DIRTY(pCurDirty))
    {
        /* NOTE: Most of these does not require additional actions, but they do
         * needed to be cleaned.  Otherwise hWindow->hAppliedDoneEvent won't
         * be set and resulted in timeout.  Be aware that these dirty are still
         * require as it is needed to trigger the copy and apply.  Note it can
         * be clear all at once with BVDC_P_CLEAN_ALL_DIRTY() since some dirty
         * bit require more than 1 vsync to handle. */
        pCurDirty->stBits.bMiscCtrl    = BVDC_P_CLEAN;
        pCurDirty->stBits.bCtIndex     = BVDC_P_CLEAN;
        pCurDirty->stBits.bAnrAdjust   = BVDC_P_CLEAN;
        pCurDirty->stBits.bStg         = BVDC_P_CLEAN;
        pCurDirty->stBits.b3D          = BVDC_P_CLEAN;
#if (BVDC_P_SUPPORT_TNTD)
        pCurDirty->stBits.bTntd        = BVDC_P_CLEAN;
#endif

        /* defer MAD changes until MAD is configured */
        if(pCurDirty->stBits.bDeinterlace)
        {
            pCurDirty->stBits.bDeinterlace = BVDC_P_CLEAN;
        }

        /* --------------------------------------------------------------- */
        /* (2) Check if user returned a previously captured buffer       */
        /* --------------------------------------------------------------- */
        if(pCurDirty->stBits.bUserReleaseBuffer)
        {
            /* TODO: need to check heapId, might need to free the buf if
             * heapId does not match */
            if((pCurInfo->pBufferFromUser) &&
               (pCurInfo->pBufferFromUser->pSurface) &&
               (pCurInfo->pBufferFromUser->stFlags.bUsedByUser))
            {
                BDBG_MSG(("Returning user captured picture."));
                BVDC_P_Buffer_ReturnBuffer_isr(hWindow->hBuffer, pCurInfo->pBufferFromUser);
                hWindow->uiAvailCaptureBuffers++;
            }
            /* clean dirty bit */
            pCurDirty->stBits.bUserReleaseBuffer = BVDC_P_CLEAN;
        }

        BDBG_MODULE_MSG(BVDC_WIN_VNET,("win[%d] writer_isr: Dirty 0x%08lx, wrtState %d, rdState %d",
            hWindow->eId, *(long unsigned int *)pCurDirty, pCurInfo->eWriterState, pCurInfo->eReaderState));
        BDBG_MODULE_MSG(BVDC_WIN_VNET,("   rCfgVnet %d, SrcPending %d, ShtDn %d, Dstry %d, rDetVnet %d",
            pCurDirty->stBits.bReConfigVnet, pCurDirty->stBits.bSrcPending, pCurDirty->stBits.bShutdown,
            pCurDirty->stBits.bDestroy, pCurDirty->stBits.bReDetVnet));

        /* --------------------------------------------------------------- */
        /* (3) start new vnet, if we just created the window, or we just   */
        /* completed the shuting-downn of the old vnet;                    */
        /*     otherwise, check if we need to reconfigure vnet, and start  */
        /* shutdown process if we need to.                                 */
        /* --------------------------------------------------------------- */
        if((pCurDirty->stBits.bReConfigVnet) &&
           (BVDC_P_CLEAN == pCurDirty->stBits.bShutdown))    /* start new vnet at this vsync */
        {
            /* we should start the new vnet even if original mpeg src is muted, so that we
             * could show a fixed color win for old chips that does not support this in cmp
             * HW. however, if bSrcPending is on, we will clean bReConfigVnet here to pause
             * the new vnet building, and wait for the user's instruction to resume. */
            if(pCurDirty->stBits.bSrcPending)
            {
                BDBG_MSG(("Window[%d] defer reconfigure stVnetMode", hWindow->eId));
                pCurDirty->stBits.bReConfigVnet = BVDC_P_CLEAN;
                hWindow->ulDropCntNonIgnoredPics += pMvdFieldData && !pMvdFieldData->bIgnorePicture;
                return;
            }
            else if(pCurDirty->stBits.bBufferPending)
            {
                pCurDirty->stBits.bReConfigVnet = BVDC_P_CLEAN;
                return;
            }
            else
            {
                bool bRfcgMcvp = false;
                bool bNotEnoughMcvpBuffers;

                /* If there is not enough memory, wait for user change or source
                 * change to start new vnet */
                if(pCurDirty->stBits.bBufferPending)
                {
                    return;
                }

                /* start new vnet right now */
                BVDC_P_Window_DecideVnetMode_isr(hWindow, pMvdFieldData, true,
                    &bRfcgMcvp, false, ulPictureIdx);
                BVDC_P_Window_DecideBufsCfgs_isr(hWindow, pMvdFieldData, pXvdFieldData, true, ulPictureIdx);
                BVDC_P_Window_CheckBuffers_isr(hWindow, ulPictureIdx,
                    &hWindow->bNotEnoughCapBuf, &bNotEnoughMcvpBuffers);

                if(bNotEnoughMcvpBuffers)
                {
                    /* Reset heapId */
                    hWindow->eMadPixelHeapId[ulPictureIdx] = BVDC_P_BufferHeapId_eUnknown;
                    hWindow->eMadQmHeapId[ulPictureIdx] = BVDC_P_BufferHeapId_eUnknown;

                    /* Bypass mcvp */
                    BDBG_ERR(("Win[%d] force bypass MCVP because there is not enough buffers",
                        hWindow->eId));
                    BVDC_P_Window_DecideVnetMode_isr(hWindow, pMvdFieldData, true,
                        &bRfcgMcvp, true, ulPictureIdx);
                }


                if(hWindow->stCurInfo.hSource->ulMosaicFirstUnmuteRectIndex == ulPictureIdx)
                    BVDC_P_Window_SetCadenceHandling_isr(hWindow, pMvdFieldData, false, true, true);

                /* NOTE: configure vnet into software records!  The following
                 * functions will alloc mad/anr buf memory. */
                BVDC_P_Window_SetWriterVnet_isr(hWindow, &hWindow->stVnetMode);
                BVDC_P_Window_SetReaderVnet_isr(hWindow, &hWindow->stVnetMode);

                /* setting eWriterState as BVDC_P_State_eActive will cause BuildWriterRul
                 * to build vnet into RUL, and start to enable VDC modules in writer */
                hWindow->stCurInfo.eWriterState = BVDC_P_State_eActive;

                /* vnet configure in software completed, so clear dirty bits;
                 * and since we just calucated new vnetMode and buf configures,
                 * so no need to check again until new dirty; also, we need to
                 * alloc fresh bufs for cap. */
                pCurDirty->stBits.bReConfigVnet   = BVDC_P_CLEAN;
                pCurDirty->stBits.bReDetVnet      = BVDC_P_CLEAN;
                pCurDirty->stBits.bReallocBuffers = BVDC_P_DIRTY;
                bFlushPicQueue = true;

                /* TODO: why it is here? */
                pCurDirty->stBits.bSharedScl    = BVDC_P_CLEAN;

                BDBG_MODULE_MSG(BVDC_WIN_VNET,("win[%d] cfg vnet 0x%x, writerState -> eActive, field %d",
                    hWindow->eId, *(unsigned int *)&hWindow->stVnetMode, eFieldId));
            }
        }
        else /* not start new vnet at this vsync */
        {
            /* Check for the need of vnet reconfigure */
            if(pCurDirty->stBits.bReDetVnet &&
               BVDC_P_Window_NotReconfiguring_isr(hWindow))
            {
                bool bRfcgVnet = false;
                bool bRfcgMcvp = false;

                /* check for vnetMode change */
                bRfcgVnet = BVDC_P_Window_DecideVnetMode_isr(hWindow,
                    pMvdFieldData, false, &bRfcgMcvp, false, ulPictureIdx);

                /* if vnetMode does not change, futher check for mad/anr/cap buf heapId changes */
                if(!bRfcgVnet)
                {
                    /* buf heapId could change if (1) input/output size changes; (2) vnet
                     * changes; (3) capture pixel format changes or datamode. */
                    bRfcgVnet = BVDC_P_Window_DecideBufsCfgs_isr(hWindow, pMvdFieldData, pXvdFieldData, false, ulPictureIdx);
                }
                if(hWindow->stCurInfo.hSource->ulMosaicFirstUnmuteRectIndex == ulPictureIdx)
                {
                    BVDC_P_Window_SetCadenceHandling_isr(hWindow, pMvdFieldData, false, bRfcgVnet, true);
                }

                if(bRfcgVnet)
                {
                    /* this sets bShutDown to true unless vnet is already inactive */
                    BVDC_P_Window_SetReconfiguring_isr(hWindow, false, true, false);
                }
            }

            /* we just checked if need to recfg vnet, or already in shut down process */
            pCurDirty->stBits.bReDetVnet = BVDC_P_CLEAN;

            /* note: besides above code, shutdown might be requested in several other
             * places */
            if(pCurDirty->stBits.bShutdown)
            {
                /* Increase writer state if we are shuting down. */
                BVDC_P_Window_WriterShutDown_isr(hWindow, eFieldId, pList);
                hWindow->ulDropCntNonIgnoredPics += pMvdFieldData && !pMvdFieldData->bIgnorePicture;
                return;
            }
            else if(pCurDirty->stBits.bSrcPending)
            {
                /* due to src pending. similar protection at reader side; */
                BDBG_MSG(("($) Window[%d] (W%d) stays off", hWindow->eId, eFieldId));
                hWindow->ulDropCntNonIgnoredPics += pMvdFieldData && !pMvdFieldData->bIgnorePicture;
                return;
            }
        }

        /* --------------------------------------------------------------- */
        /* (4.1) allocate more bufs for capture if we don't have enough;    */
        /* free some bufs if we have more than needed.                      */
        /* note: if we start new vnet at this vsync, all capture bufs are   */
        /* allocated here (to avoid duplicated code), and mad/anr bufs are  */
        /* allocated in SetWriterVnet_isr/SetReaderVnet_isr called above in */
        /* this vsync. Therefore no fragmentation are generated             */
        /* ---------------------------------------------------------------- */
        if(pCurDirty->stBits.bBufAllocMode && !pCurDirty->stBits.bReConfigVnet &&
           !pCurDirty->stBits.bSrcPending && !pCurDirty->stBits.bBufferPending)
        {
            if((hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate) &&
               (hWindow->ePrevBufAllocMode == BVDC_P_BufHeapAllocMode_eLeftOnly) &&
               BVDC_P_VNET_USED_CAPTURE(hWindow->stVnetMode))
            {

                /* 3D: Allocate right buffer */
                BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] alloc %d cap right buffers (req %s, prefer %s)",
                    hWindow->eId, hWindow->ulBufCntAllocated,
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdRequest),
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdPrefer)));
                /* Allocate right buffers for 3D */
                err = BVDC_P_BufferHeap_AllocateBuffers_isr(hWindow->hCapHeap,
                    hWindow->apHeapNode_R, hWindow->ulBufCntAllocated, false, hWindow->eBufferHeapIdRequest,
                    hWindow->eBufferHeapIdPrefer);
                /* Not enough memory, dump out configuration */
                if(err == BERR_OUT_OF_DEVICE_MEMORY)
                {
                    BVDC_P_Window_DumpCapBufHeapConfigure_isr(hWindow, false);
                    BVDC_P_PRINT_BUF_DEBUG_INSTRUCTION();
                    hWindow->bNotEnoughCapBuf = true;
                    BVDC_P_Window_SetReconfiguring_isr(hWindow, false, false, true);
                    return;
                }
                else
                {
                    hWindow->bNotEnoughCapBuf = false;
                    BVDC_P_Buffer_SetRightBufferPictureNodes_isr(hWindow->hBuffer,
                        hWindow->apHeapNode_R, hWindow->ulBufCntAllocated, true);
                }
            }
            else if((hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLeftOnly) &&
               (hWindow->ePrevBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate))
            {
                /* 2D: free right buffer */
                BVDC_P_Buffer_SetRightBufferPictureNodes_isr(hWindow->hBuffer,
                    hWindow->apHeapNode_R, hWindow->ulBufCntAllocated, false);

                BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hCapHeap,
                    hWindow->apHeapNode_R, hWindow->ulBufCntAllocated, false);
            }

            pCurDirty->stBits.bBufAllocMode = BVDC_P_CLEAN;
        }

        if(pCurDirty->stBits.bReallocBuffers &&
           !pCurDirty->stBits.bReConfigVnet && !pCurDirty->stBits.bSrcPending &&
           !pCurDirty->stBits.bBufferPending)
        {
            uint32_t  ulCount;

            if(hWindow->ulBufCntNeeded > hWindow->ulBufCntAllocated)
            {
                ulCount = hWindow->ulBufCntNeeded - hWindow->ulBufCntAllocated;
                BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] alloc %d cap buffers (req %s, prefer %s)", hWindow->eId, ulCount,
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdRequest),
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdPrefer)));

                err = BVDC_P_BufferHeap_AllocateBuffers_isr(hWindow->hCapHeap,
                    hWindow->apHeapNode, ulCount, false, hWindow->eBufferHeapIdRequest,
                    hWindow->eBufferHeapIdPrefer);

                /* Not enough memory, dump out configuration */
                if(err == BERR_OUT_OF_DEVICE_MEMORY)
                {
                    BVDC_P_Window_DumpCapBufHeapConfigure_isr(hWindow, true);

                    BVDC_P_PRINT_BUF_DEBUG_INSTRUCTION();
                    hWindow->bNotEnoughCapBuf = true;
                    BVDC_P_Window_SetReconfiguring_isr(hWindow, false, false, true);
                    return;
                }
                else
                {
                    hWindow->bNotEnoughCapBuf = false;
                    if(hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate)
                    {
                        BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] alloc %d cap right buffers (req %s, prefer %s)",
                            hWindow->eId, ulCount,
                            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdRequest),
                            BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdPrefer)));
                        /* Allocate right buffers for 3D */
                        err = BVDC_P_BufferHeap_AllocateBuffers_isr(hWindow->hCapHeap,
                            hWindow->apHeapNode_R, ulCount, false, hWindow->eBufferHeapIdRequest,
                            hWindow->eBufferHeapIdPrefer);
                        /* Not enough memory, dump out configuration */
                        if(err == BERR_OUT_OF_DEVICE_MEMORY)
                        {
                            BVDC_P_Window_DumpCapBufHeapConfigure_isr(hWindow, false);

                            BVDC_P_PRINT_BUF_DEBUG_INSTRUCTION();
                            hWindow->bNotEnoughCapBuf = true;
                            BVDC_P_Window_SetReconfiguring_isr(hWindow, false, false, true);
                            return;
                        }
                        else
                        {
                            hWindow->stCurInfo.stDirty.stBits.bBufAllocMode = BVDC_P_CLEAN;
                            hWindow->bNotEnoughCapBuf = false;

                            BVDC_P_Buffer_AddPictureNodes_isr(hWindow->hBuffer, hWindow->apHeapNode,
                                hWindow->apHeapNode_R, ulCount,
                                hWindow->ulBufDelay, hWindow->bSyncLockSrc, bFlushPicQueue);
                        }
                    }
                    else
                    {
                        BVDC_P_Buffer_AddPictureNodes_isr(hWindow->hBuffer, hWindow->apHeapNode,
                            NULL, ulCount,
                            hWindow->ulBufDelay, hWindow->bSyncLockSrc, bFlushPicQueue);
                    }
                }

            }
            /* the '=' in the following is for the case that forceCap is turnt off during src
             * pending, so that ReleasePictureNodes_isr will sets ulSyncDelay. Otherwise
             * readerNode will be fixed to buf0, writerNode fixed to buf1 => mute forever */
            else
            {
                ulCount = hWindow->ulBufCntAllocated - hWindow->ulBufCntNeeded;
                BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] free %d cap buffers (%s)", hWindow->eId, ulCount,
                    BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdRequest)));

                if(((hWindow->eBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate) &&
                     (BVDC_P_CLEAN == hWindow->stCurInfo.stDirty.stBits.bBufAllocMode)) ||
                   ((hWindow->ePrevBufAllocMode == BVDC_P_BufHeapAllocMode_eLRSeparate)  &&
                     (BVDC_P_DIRTY == hWindow->stCurInfo.stDirty.stBits.bBufAllocMode)))
                {
                    BDBG_MODULE_MSG(BVDC_WIN_BUF, ("Win[%d] free %d cap buffers (%s) for right",
                        hWindow->eId, ulCount,
                        BVDC_P_BUFFERHEAP_GET_HEAP_ID_NAME(hWindow->eBufferHeapIdRequest)));
                    BVDC_P_Buffer_ReleasePictureNodes_isr(hWindow->hBuffer,
                        hWindow->apHeapNode, hWindow->apHeapNode_R,
                        BVDC_P_MIN(hWindow->hBuffer->ulActiveBufCnt, ulCount),
                        hWindow->ulBufDelay);
                    BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hCapHeap,
                        hWindow->apHeapNode_R, ulCount, false);

                    hWindow->stCurInfo.stDirty.stBits.bBufAllocMode = BVDC_P_CLEAN;
                }
                else
                {
                    BVDC_P_Buffer_ReleasePictureNodes_isr(hWindow->hBuffer,
                        hWindow->apHeapNode, NULL,
                        BVDC_P_MIN(hWindow->hBuffer->ulActiveBufCnt, ulCount),
                        hWindow->ulBufDelay);
                }
                if(hWindow->hCapHeap) { /* in case of captureless window */
                    BVDC_P_BufferHeap_FreeBuffers_isr(hWindow->hCapHeap,
                        hWindow->apHeapNode, ulCount, false);
                }
            }

            hWindow->ulBufCntAllocated = hWindow->ulBufCntNeeded;
            pCurDirty->stBits.bReallocBuffers = BVDC_P_CLEAN;
        }

        /* --------------------------------------------------------------- */
        /* (4.2) flush new allocated bufs                                  */
        /* --------------------------------------------------------------- */
        if(bFlushPicQueue)
        {
            BDBG_MODULE_MSG(BVDC_WIN_BUF, ("win[%d] flushes cap buffers", hWindow->eId));
            BVDC_P_Buffer_Invalidate_isr(hWindow->hBuffer);
            hWindow->pCurWriterNode =
                BVDC_P_Buffer_GetCurWriterNode_isr(hWindow->hBuffer);
            hWindow->pCurReaderNode =
                BVDC_P_Buffer_GetCurReaderNode_isr(hWindow->hBuffer);
        }
    }

    /* --------------------------------------------------------------- */
    /* (5) setup picture node                                          */
    /* --------------------------------------------------------------- */
    if(BVDC_P_SRC_IS_ITU656(hWindow->stCurInfo.hSource->eId))
    {
#if (BVDC_P_SUPPORT_NEW_656_IN_VER)
        BVDC_P_656In_GetStatus_isr(hWindow->stCurInfo.hSource->h656In, &bVideoDetect);
#endif
    }
    else if(BVDC_P_SRC_IS_HDDVI(hWindow->stCurInfo.hSource->eId))
    {
        BVDC_P_HdDvi_GetStatus_isr(hWindow->stCurInfo.hSource->hHdDvi, &bVideoDetect);
    }
    else
    {
        bVideoDetect = true;
    }

    /* Get next buffer node */
    /* MosaicMode: don't advance multi-buffer node for the chained mosaic rects; */
    /* Done advance writer node when in repeat mute mode */
    if (BVDC_P_SRC_IS_VFD(pCurInfo->hSource->eId))
    {
        pPicture = &pCurInfo->hSource->hVfdFeeder->stPicture;
        bVideoDetect = true;
    }
    else if((0 == ulPictureIdx) && (pSrcInfo->eMuteMode != BVDC_MuteMode_eRepeat))
    {
        BVDC_P_Buffer_MtgMode eMtgMode = BVDC_P_Buffer_MtgMode_eNonMtg;

#if (BVDC_P_SUPPORT_MTG)
        /* we read after RUL-done, and hWindow->pCurWriterNode is pointing to the buffer MAD is writing */
        if (pCurInfo->hSource->bMtgSrc && !hWindow->stCurInfo.bMosaicMode)
        {
            if(BVDC_P_MVP_USED_MAD_AT_WRITER(hWindow->stVnetMode, hWindow->stMvpMode))
            {
                BVDC_P_Mcdi_ReadOutPhase_isr(hWindow->stCurResource.hMcvp->hMcdi, hWindow->pCurWriterNode);
            }
        }

        /*  The multi-buffer mechanism will operate in MTG mode when
                1. the source is MTG driven,
                2. the window is not in mosaic mode,
                3. a deinterlacer is active.
                4. when the source's frame rate code is known

            If all 3 items above are true, the deinterlacer's output phase will be used to determine
            which pictures will be dropped by the mutli-buffer algorithm.

            If only items 1 and 2 above are true and the deinterlacer is not active, the bPictureRepeat flag from
            the DM will be used to select which pictures will be dropped by the multi-buffer mechanism. */

        if (pCurInfo->hSource->bMtgSrc &&
            !hWindow->stCurInfo.bMosaicMode &&
            pCurInfo->hSource->eFrameRateCode != BAVC_FrameRateCode_eUnknown)
        {
            eMtgMode = BVDC_P_MVP_USED_MAD(hWindow->stMvpMode) ? BVDC_P_Buffer_MtgMode_eMadPhase : BVDC_P_Buffer_MtgMode_eXdmRepeat;

            if (eMtgMode == BVDC_P_Buffer_MtgMode_eXdmRepeat)
            {
                /* Determine XDM cadence */
                if ((hWindow->stCurInfo.hSource->eFrameRateCode == BAVC_FrameRateCode_e23_976) ||
                    (hWindow->stCurInfo.hSource->eFrameRateCode == BAVC_FrameRateCode_e24))
                {
                    hWindow->pCurWriterNode->stFlags.bRev32Locked = true;
                }
                else
                {
                    hWindow->pCurWriterNode->stFlags.bRev32Locked = false;
                }
            }
        }
        else
        {
            eMtgMode = BVDC_P_Buffer_MtgMode_eNonMtg;
        }

        pPicture = BVDC_P_Buffer_GetNextWriterNode_isr(hWindow, eFieldId, eMtgMode);

        BVDC_P_Window_DetermineBufferCount_isr(hWindow);

        if (pCurInfo->hSource->bMtgSrc && !hWindow->stCurInfo.bMosaicMode)
        {
            /* predict, in case writer_isr / reader_isr are mis-ordered */
            pPicture->stFlags.bRev32Locked = hWindow->pCurWriterNode->stFlags.bRev32Locked;
            pPicture->ulMadOutPhase = (hWindow->pCurWriterNode->ulMadOutPhase + 1) % 5;

#if (BDBG_DEBUG_BUILD)
            if (pPicture->stFlags.bRev32Locked && eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase)
            {
                BDBG_MODULE_MSG(BVDC_MTGW,("w win%d (n%d p%d)", hWindow->eId, pPicture->ulBufferId, pPicture->ulMadOutPhase));
            }
            else if (pCurInfo->hSource->bMtgSrc && !hWindow->stCurInfo.bMosaicMode &&
                     BVDC_P_MVP_USED_MAD_AT_WRITER(hWindow->stVnetMode, hWindow->stMvpMode) &&
                     eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase)
            {
                BDBG_MODULE_MSG(BVDC_MTGW,("w win%d (n%d p%d), no lock", hWindow->eId, pPicture->ulBufferId, pPicture->ulMadOutPhase));
            }
#endif
        }
#else
        pPicture = BVDC_P_Buffer_GetNextWriterNode_isr(hWindow, eFieldId, eMtgMode);

        BVDC_P_Window_DetermineBufferCount_isr(hWindow);
#endif /* BVDC_P_SUPPORT_MTG */

        hWindow->pCurWriterNode = pPicture;

#if BVDC_P_SUPPORT_MOSAIC_MODE
        /* MosaicMode: reset ClearRect mask set, which will be set by the following
           mosaic pictures list UpdateSrcAndUserInfo_isr; */
        pPicture->ulMosaicRectSet = 0;
        BVDC_P_Window_InitVideoInputColorSpace_isr(pPicture);
#endif
    }
    else
    {
        pPicture = hWindow->pCurWriterNode;
    }

    /* This is where we combine the source_info and user_info into
     * node_info.  This node_info is then use to program the scaler and
     * capture to create node, and the created node should have all the
     * info to setup the playback blocks.
     * <source_info> + <user_info> ==> <node_info> */

    BVDC_P_Window_UpdateSrcAndUserInfo_isr(hWindow, pPicture,
        pMvdFieldData, pXvdFieldData, eFieldId, ulPictureIdx);

    /* PR57098: test routine for HDMI garbage issue.
     * Note, it is the application/middle-ware's responsibility
     * to make sure enough buffers are allocated for reader/writer
     * gap. This can be done via BVDC_Window_SetDelayOffset(). */
    if((!bVideoDetect) && hWindow->stCurInfo.hSource->stCurInfo.stDirty.stBits.bVideoDetected)
    {
        BVDC_P_Buffer_Invalidate_isr(hWindow->hBuffer);
        hWindow->pCurWriterNode =
            BVDC_P_Buffer_GetCurWriterNode_isr(hWindow->hBuffer);
        hWindow->pCurReaderNode =
            BVDC_P_Buffer_GetCurReaderNode_isr(hWindow->hBuffer);
    }

    /* Mute the picture if video is not detected.  There are two type of mute:
     * (1) bMute           - Which mean shutdown reader modules to mute!
     * (2) bMuteFixedColor - Display fixed color, like in the case of ne video */
    pPicture->stFlags.bMute            = !hWindow->stCurInfo.hSource->bStartFeed;
    pPicture->stFlags.bMuteFixedColor |= !bVideoDetect;

    /* Tag display format as frame if it's progressive.  Removed the
     * previous prediction of display polarity. */
    if(!hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced)
    {
        pPicture->eDisplayPolarity = BAVC_Polarity_eFrame;
    }

    if(0 == ulPictureIdx)
    {
        BVDC_P_Window_AdjustCadenceHandling_isr(hWindow, pMvdFieldData);
        BVDC_P_Window_SetCapturePolarity_isr(hWindow, eFieldId, pPicture);
    }
    /* Prevent this from flooding the screen.*/
    /* BDBG_MSG(("Win%d W(%d): B(%d) (%d -> %d)", hWindow->eId, eFieldId,
       pPicture->ulBufferId, pPicture->eSrcPolarity, pPicture->eDstPolarity));
    */

    /* --------------------------------------------------------------- */
    /* (6) Set Info (register softeare shadow) for each sub-modules in */
    /* write vnet                                                      */
    /* --------------------------------------------------------------- */

    /* NOTE: we should config MAD32 before SCL since the picture node's source
     * polarity could be modified by mad setinfo_isr; */
    if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId))
    {
        /* restore dropped non-ignore pictures while shutdown/reconfig in NRT mode */
        if((hWindow->ulDropCntNonIgnoredPics) &&
            (!BVDC_P_MVP_USED_MAD(hWindow->stMvpMode)))
        {
            /* assume non-ignore pic always needs to advance STC */
            pPicture->bIgnorePicture = false;
            pPicture->bStallStc      = false;/* assume non-ignore pic always needs to advance STC */
            hWindow->ulDropCntNonIgnoredPics--;
        }

        /* MFD must be build by the last window connected to the src, otherwise
         * Mfd might starts to feed after the 1st win gets enabled, then the
         * following win might join in the vnet after feeding started */
        BVDC_P_feeder_SetRulBuildWinId_isr(hWindow->stCurInfo.hSource->hMpegFeeder, hWindow->eId);
    }

#if (BVDC_P_SUPPORT_DNR)
    /* when the sync-slaved window is just created but its cmp/display slot RUL is not updated yet, the source isr
       should not let this window own the dnr/xsrc programming! */
    if(BVDC_P_VNET_USED_DNR(hWindow->stVnetMode)
#if BVDC_P_SUPPORT_STG
       && !(hWindow->bSyncSlave && !hWindow->hCompositor->bSyncSlave)
#endif
    )
    {
        if(BVDC_P_VNET_USED_DNR_AT_WRITER(hWindow->stVnetMode)) {
            BVDC_P_Dnr_SetInfo_isr(hWindow->stCurResource.hDnr, pPicture);
        }
        /* shared eBldWin ownership for source block - DNR */
        BVDC_P_Dnr_SetRulBuildWinId_isr(hWindow->stCurResource.hDnr, hWindow->eId);
    }
#endif

#if (BVDC_P_SUPPORT_XSRC)
    /* when the sync-slaved window is just created but its cmp/display slot RUL is not updated yet, the source isr
       should not let this window own the dnr/xsrc programming! */
    if(BVDC_P_VNET_USED_XSRC(hWindow->stVnetMode)
#if BVDC_P_SUPPORT_STG
       && !(hWindow->bSyncSlave && !hWindow->hCompositor->bSyncSlave)
#endif
    )
    {
        if(BVDC_P_VNET_USED_XSRC_AT_WRITER(hWindow->stVnetMode)) {
            BVDC_P_Xsrc_SetInfo_isr(hWindow->stCurResource.hXsrc, hWindow, pPicture);
        }
        /* shared eBldWin ownership for source block - XSRC */
        BVDC_P_Xsrc_SetRulBuildWinId_isr(hWindow->stCurResource.hXsrc, hWindow->eId);
    }
#endif

#if (BVDC_P_SUPPORT_VFC)
    if(BVDC_P_VNET_USED_VFC(hWindow->stVnetMode))
    {
        if(BVDC_P_VNET_USED_VFC_AT_WRITER(hWindow->stVnetMode)) {
            BVDC_P_Vfc_SetInfo_isr(hWindow->stCurResource.hVfc, hWindow, pPicture);
        }
        /* shared eBldWin ownership for source block - XSRC */
        BVDC_P_Vfc_SetRulBuildWinId_isr(hWindow->stCurResource.hVfc, hWindow->eId);
    }
#endif

    if(BVDC_P_VNET_USED_MVP_AT_WRITER(pPicture->stVnetMode))
    {
        BVDC_P_MCVP_SetInfo_isr(hWindow->stCurResource.hMcvp,hWindow, pPicture);
        if(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode))
        {
            if(bVideoDetect)
            {
                BVDC_P_Window_AdjustPicRepeatBit_isr(hWindow, pPicture);
                /* this is for downstream submodules like scl and fgt */
                pPicture->eSrcPolarity = BAVC_Polarity_eFrame;
                /* adjust mad delay for non-mosaic and mosaic both*/
                BVDC_P_Window_AdjustForMadDelay_isr(hWindow, pPicture);
            }
        }
    }

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_WRITER(hWindow->stVnetMode) && hWindow->stVnetMode.stBits.bTntdBeforeScl)
    {
        BVDC_P_Tntd_SetInfo_isr(hWindow->stCurResource.hTntd, pPicture);
    }
#endif

    if(BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode))
    {
        BVDC_P_Scaler_SetInfo_isr(hWindow->stCurResource.hScaler, pPicture);
    }

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_WRITER(hWindow->stVnetMode) && !hWindow->stVnetMode.stBits.bTntdBeforeScl)
    {
        BVDC_P_Tntd_SetInfo_isr(hWindow->stCurResource.hTntd, pPicture);
    }
#endif

    if(BVDC_P_VNET_USED_CAPTURE(hWindow->stVnetMode))
    {
        /* MosaicMode: set the rect idx to program capture */
        if(pMvdFieldData != NULL)
        {
            BVDC_P_Capture_SetInfo_isr(hWindow->stCurResource.hCapture, hWindow, pPicture,
                ulPictureIdx, (NULL == pMvdFieldData->pNext));
        }
        else
        {
            BVDC_P_Capture_SetInfo_isr(hWindow->stCurResource.hCapture, hWindow, pPicture,
                ulPictureIdx, true);
        }
    }

    BDBG_LEAVE(BVDC_P_Window_Writer_isr);
    return;
}

/***************************************************************************
 * {private}
 *
 * This READER function displays a buffer node that is captured by the
 * WRITER function.
 */
void BVDC_P_Window_Reader_isr
    ( BVDC_Window_Handle               hWindow,
      const BAVC_Polarity              eFieldId,
      BVDC_P_ListInfo                 *pList )
{
    BVDC_Compositor_Handle hCompositor;
    BVDC_P_PictureNode *pPicture;
    BVDC_P_Window_Info *pCurInfo;
    BVDC_P_Window_DirtyBits *pCurDirty;
    const BVDC_P_Source_Info *pSrcInfo;
    bool bVideoDetect = false;
    bool bFixedColor = false;
#if BFMT_LEGACY_3DTV_SUPPORT
    bool bOrigMute = false;
#endif
    uint32_t                 ulRWindowXOffset;

    BDBG_ENTER(BVDC_P_Window_Reader_isr);

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    hCompositor = hWindow->hCompositor;
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);


    /* Get dirty bits to check if needed to rebuild csc rul. */
    pSrcInfo  = &hWindow->stCurInfo.hSource->stCurInfo;
    pCurInfo  = &hWindow->stCurInfo;
    pCurDirty = &hWindow->stCurInfo.stDirty;

    /* Nothing to do for gfx window, except for when destroying. */
    if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId) &&
      (BVDC_P_CLEAN == pCurDirty->stBits.bDestroy))
    {
        BVDC_P_GfxFeeder_UpdateState_isr(hWindow->stCurInfo.hSource->hGfxFeeder,
            &(hWindow->stCurInfo.hSource->stCurInfo), pList, eFieldId);
        /* Gfx does not need to use these dirty bits. */
        BVDC_P_CLEAN_ALL_DIRTY(pCurDirty);
        return;
    }

    /* Shutting down the READER's blocks that's in the display _isr
     * (1) Shutdown for destroy
     * (2) Shutdown for removing/adding block from/to READER (change vnet)
     * (3) Shutdown for source bringup or transitioning
     *
     * After READER is shutdown (eReaderState == BVDC_P_State_eShutDown)
     * WRITER will shut WRITER's blocks (eWriterState == BVDC_P_State_eShutDown).
     * Target state move:
     *
     * eReaderState == BVDC_P_State_eShutDown
     * eWriterState == BVDC_P_State_eShutDown
     *
     * To initiate the shutdown put both into BVDC_P_State_eShutDownPending. */
    if(BVDC_P_IS_DIRTY(pCurDirty))
    {
        hWindow->bCfcDirty |= pCurDirty->stBits.bCscAdjust;
        pCurDirty->stBits.bCscAdjust = BVDC_P_CLEAN;

        /* Set new luma key */
        if((pCurDirty->stBits.bMosaicMode) ||
           (pCurDirty->stBits.bColorKeyAdjust))
        {
            BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_LUMA_KEYING) =
                BCHP_FIELD_DATA(CMP_0_V0_LUMA_KEYING, ENABLE, pCurInfo->stColorKey.bLumaKey     ) |
                BCHP_FIELD_DATA(CMP_0_V0_LUMA_KEYING, Y_MASK, pCurInfo->stColorKey.ucLumaKeyMask) |
                BCHP_FIELD_DATA(CMP_0_V0_LUMA_KEYING, Y_HIGH, pCurInfo->stColorKey.ucLumaKeyHigh) |
                BCHP_FIELD_DATA(CMP_0_V0_LUMA_KEYING, Y_LOW , pCurInfo->stColorKey.ucLumaKeyLow );

            BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_CR_KEYING) =
                BCHP_FIELD_DATA(CMP_0_V0_CR_KEYING, ENABLE, pCurInfo->stColorKey.bChromaRedKey     ) |
                BCHP_FIELD_DATA(CMP_0_V0_CR_KEYING, C_MASK, pCurInfo->stColorKey.ucChromaRedKeyMask) |
                BCHP_FIELD_DATA(CMP_0_V0_CR_KEYING, C_HIGH, pCurInfo->stColorKey.ucChromaRedKeyHigh) |
                BCHP_FIELD_DATA(CMP_0_V0_CR_KEYING, C_LOW , pCurInfo->stColorKey.ucChromaRedKeyLow );

            BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_CB_KEYING) =
                BCHP_FIELD_DATA(CMP_0_V0_CB_KEYING, ENABLE, pCurInfo->stColorKey.bChromaBlueKey     ) |
                BCHP_FIELD_DATA(CMP_0_V0_CB_KEYING, C_MASK, pCurInfo->stColorKey.ucChromaBlueKeyMask) |
                BCHP_FIELD_DATA(CMP_0_V0_CB_KEYING, C_HIGH, pCurInfo->stColorKey.ucChromaBlueKeyHigh) |
                BCHP_FIELD_DATA(CMP_0_V0_CB_KEYING, C_LOW , pCurInfo->stColorKey.ucChromaBlueKeyLow );

            /* SW7425-2236: Hijacked colorkey to masked out garbage for mosaic mode. */
#if  defined(BCHP_CMP_0_V0_RECT_COLOR) && \
    !defined(BCHP_CMP_0_V0_RECT_TOP_CTRL_RECT_KEY_VALUE_MASK)
            if(pCurInfo->bMosaicMode)
            {
                BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_LUMA_KEYING) =
                    BCHP_FIELD_DATA(CMP_0_V0_LUMA_KEYING, ENABLE, 1   ) |
                    BCHP_FIELD_DATA(CMP_0_V0_LUMA_KEYING, Y_MASK, 0xff) |
                    BCHP_FIELD_DATA(CMP_0_V0_LUMA_KEYING, Y_HIGH, 0x01) |
                    BCHP_FIELD_DATA(CMP_0_V0_LUMA_KEYING, Y_LOW , 0x00);

                BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_CR_KEYING) =
                    BCHP_FIELD_DATA(CMP_0_V0_CR_KEYING, ENABLE, 0   ) |
                    BCHP_FIELD_DATA(CMP_0_V0_CR_KEYING, C_MASK, 0   ) |
                    BCHP_FIELD_DATA(CMP_0_V0_CR_KEYING, C_HIGH, 0xff) |
                    BCHP_FIELD_DATA(CMP_0_V0_CR_KEYING, C_LOW , 0   );

                BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_CB_KEYING) =
                    BCHP_FIELD_DATA(CMP_0_V0_CB_KEYING, ENABLE, 0   ) |
                    BCHP_FIELD_DATA(CMP_0_V0_CB_KEYING, C_MASK, 0   ) |
                    BCHP_FIELD_DATA(CMP_0_V0_CB_KEYING, C_HIGH, 0xff) |
                    BCHP_FIELD_DATA(CMP_0_V0_CB_KEYING, C_LOW , 0   );

            }
#endif
            /* Clear window dirty bit */
            pCurDirty->stBits.bColorKeyAdjust = BVDC_P_CLEAN;
            /* Set dirty bit for compositor */
            hCompositor->ulColorKeyAdjust[hWindow->eId] = BVDC_P_RUL_UPDATE_THRESHOLD;
        }

        if(pCurDirty->stBits.bShutdown)
        {
            if(((BVDC_P_State_eShutDownPending == pCurInfo->eReaderState) ||
                (BVDC_P_State_eShutDownRul == pCurInfo->eReaderState) ||
                (BVDC_P_State_eDrainVnet == pCurInfo->eReaderState)))
            {
                /* eWriterState is waiting for eReaderState to move to shutdown. */
                /* Make sure we executed the RUL that shutdown reader's modules. */
                bool bLastExecuted;

                /* to prevent back-to-back callbacks prematurely exiting reader
                 * shutdown without execution;
                 * Note: progressive format uses the same slot for the next RUL
                 * as the current one; */
                if(BAVC_Polarity_eFrame != eFieldId)
                {
                    BRDC_Slot_Handle hCurSlot;
                    BRDC_List_Handle hCurList;
                    BAVC_Polarity eNextFieldId = BVDC_P_NEXT_POLARITY(eFieldId);

                    if(!hCompositor->hSyncLockSrc)
                    {
                        hCurSlot = BVDC_P_CMP_GET_SLOT(hCompositor, eNextFieldId);
                        hCurList = BVDC_P_CMP_GET_LIST(hCompositor, eNextFieldId);
                    }
                    else
                    {
                        /* if field swap, the field Id already rotated */
                        if(hCompositor->hSyncLockSrc->bFieldSwap)
                        {
                            eNextFieldId = eFieldId;
                        }
                        BDBG_ASSERT(hCompositor->hSyncLockSrc);
                        hCurSlot = BVDC_P_SRC_GET_SLOT(hCompositor->hSyncLockSrc, eNextFieldId);
                        hCurList = BVDC_P_SRC_GET_LIST(hCompositor->hSyncLockSrc, eNextFieldId);
                    }
                    BRDC_Slot_UpdateLastRulStatus_isr(hCurSlot, hCurList, true);
                    bLastExecuted = BRDC_List_GetLastExecStatus_isr(hCurList);
                    BDBG_MSG(("Window (R) current slot bExe = %d for field %d",
                        bLastExecuted, eNextFieldId));
                }
                else
                {
                    bLastExecuted = pList->bLastExecuted;
                }

                if((BVDC_P_State_eShutDownPending == pCurInfo->eReaderState) ||
                   ((!bLastExecuted) && (BVDC_P_State_eShutDownRul == pCurInfo->eReaderState)))
                {
                    hCompositor->bInitial = true; /* inform comp to reset */
                    /* SWSTB-8410 compositor initial needs to reprogram cfc registers */
                    hWindow->bCfcDirty = BVDC_P_DIRTY;
#if BVDC_P_SUPPORT_CMP_CLEAR_RECT
                    if(pCurInfo->bMosaicMode)
                    {
                        BVDC_P_Window_SetClearRect_isr(hWindow, pCurInfo, NULL);
                    }
#endif
                    pCurInfo->eReaderState = BVDC_P_State_eShutDownRul;
                    BDBG_MSG(("(2.a) Window[%d] Slip(R%d) is BVDC_P_State_eShutDownRul, bExe = %d",
                        hWindow->eId, eFieldId, bLastExecuted));

                    /* size must be set so constant color doesn't hang compositor */
                    BVDC_P_Window_SetMutedSurSize_isr(hWindow, eFieldId);

                    /* so that cmp would build less to RUL */
                    BVDC_P_Window_WinOutSetEnable_isr(hWindow, 0, false);
                }
                else if((BVDC_P_State_eShutDownRul == pCurInfo->eReaderState) ||
                        ((!bLastExecuted) && (BVDC_P_State_eDrainVnet == pCurInfo->eReaderState)))
                {
                    pCurInfo->eReaderState = BVDC_P_State_eDrainVnet;
                    BDBG_MSG(("(2.b) Window[%d] Slip(R%d) is BVDC_P_State_eDrainVnet, bExe = %d",
                        hWindow->eId, eFieldId, bLastExecuted));
                }
                else if(BVDC_P_State_eDrainVnet == pCurInfo->eReaderState) /* && bLastEcecuted */
                {
                    /* clear off reader side vnet sw infor and release resources including
                     * loop-back, free-channel, mad/anr/cap bufs; It must be called after
                     * reader's drain rul are executed.
                     * note: it is state BVDC_P_State_eShutDownRul that causes each module
                     * to build into RUL to disable HW and to drop off from vnet */
                    BVDC_P_Window_UnsetReaderVnet_isr(hWindow, &hWindow->stVnetMode);
                    pCurInfo->eReaderState = BVDC_P_State_eShutDown;
                    BDBG_MSG(("(2.c) Window[%d] Slip(R%d) is BVDC_P_State_eShutDown, bExe = %d",
                        hWindow->eId, eFieldId, bLastExecuted));

                    /* this causes empty RDC slot with mpeg src? */
                    /*BVDC_P_Window_ProcPostShutDown_isr(hWindow);*/
                }
                /* Gfx does not have writer so, just complete the shutdown
                 * when reader is shutdown. */
                if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId) &&
                   (BVDC_P_State_eShutDown == pCurInfo->eReaderState))
                {
                    pCurInfo->eWriterState = BVDC_P_State_eShutDown;
                }
            }
            else if(BVDC_P_State_eShutDown == pCurInfo->eReaderState)
            {
                /* Waiting for eWriterState move to shutdown.
                 * stay off after reader turns off the video path; */
                BDBG_MSG(("(*) Window[%d] Slip(R%d) stay off", hWindow->eId, eFieldId));
            }
            return;
        }
        /* TODO: reader will indeed stay off automatically according to readerState
         * and pic->bMute, therefore this short circuit might not be needed. */
        else if((pCurDirty->stBits.bDestroy) ||
                (pCurDirty->stBits.bSrcPending) ||
                (pCurDirty->stBits.bReConfigVnet))
        {
            /* Reader stays off until dirty bits are handle. */
            BDBG_MSG(("($) Window[%d] Slip(R%d) stay off", hWindow->eId, eFieldId));
            return;
        }
    }

    if (!BVDC_P_SRC_IS_VFD(hWindow->stCurInfo.hSource->eId))
    {
        if((pSrcInfo->eMuteMode == BVDC_MuteMode_eRepeat) && (hWindow->pCurReaderNode != hWindow->pCurWriterNode))
        {
            pPicture = hWindow->pCurReaderNode;

            /* repeated picture may need MAD trick mode to handle; */
            pPicture->stFlags.bPictureRepeatFlag = true;
        }
        else
        {
            BVDC_P_Buffer_MtgMode eMtgMode;

            hWindow->hBuffer->bMtgAlignSrcAndDisp = false;

            if (pCurInfo->hSource->bMtgSrc && !hWindow->stCurInfo.bMosaicMode)
            {
                eMtgMode = BVDC_P_MVP_USED_MAD(hWindow->stMvpMode) ? BVDC_P_Buffer_MtgMode_eMadPhase : BVDC_P_Buffer_MtgMode_eXdmRepeat;

                /* For those cases where the source is interlaced, is de-interlaced to a frame, and then scanned
                   out with the opposite polarity to the display field, a vertical bounce is sometimes seen depending
                   on content. See SWSTB-6406. This is mainly due to MTG always being set to send out frames.
                   The solution is to use the srcOriginalPolarity instead of the srcPolarity for determining which
                   picture to display in multibuffering. Note that this only applies to cases wherein the source and
                   display rates are the same and are both interlaced and there is no 3:2 lock detected by the
                   associated deinterlacer. Note that 59.94Hz and 60Hz rates are deemed the same.
                 */
                if (eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase)
                {
                    uint32_t ulDispRateMask = hWindow->hCompositor->stCurInfo.pFmtInfo->ulVertFreqMask;
                    uint32_t ulSrcRateMask = hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->ulVertFreqMask;
                    bool bDispInterlaced = hWindow->hCompositor->stCurInfo.pFmtInfo->bInterlaced;
                    bool bSrcInterlaced = hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo->bInterlaced;

                    /* matching rates and interlaced display */
                    if (!hWindow->pCurReaderNode->stFlags.bRev32Locked &&
                        bDispInterlaced == bSrcInterlaced &&
                        ulDispRateMask == ulSrcRateMask)
                    {
                        hWindow->hBuffer->bMtgAlignSrcAndDisp = true;
                    }
                }
            }
            else
            {
                eMtgMode = BVDC_P_Buffer_MtgMode_eNonMtg;
            }


            pPicture = BVDC_P_Buffer_GetNextReaderNode_isr(hWindow, eFieldId, eMtgMode);

#if (BDBG_DEBUG_BUILD && BVDC_P_SUPPORT_MTG)
            if (pPicture->stFlags.bRev32Locked && eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase)
            {
                hWindow->iLockCntr ++;
                BDBG_MODULE_MSG(BVDC_MTGR,("read win%d (n%d phase %s)", hWindow->eId, pPicture->ulBufferId, (pPicture->ulMadOutPhase==1 || pPicture->ulMadOutPhase==2)? "12" : "  340"));
                if (pPicture->ulMadOutPhase==1 || pPicture->ulMadOutPhase==2)
                {
                    /* current phase 12 */
                    if (hWindow->iPhaseCntr <= 0)
                    {
                        /* same phase continues */
                        hWindow->iPhaseCntr --;
                    }
                    else
                    {
                        /* previous phase was 340, now we switch to phase 12 */
                        if ((0!=hWindow->iPrevPhaseCntr) && (hWindow->iLockCntr > 5*3) &&
                            ((hWindow->iPhaseCntr > (hWindow->iPrevPhaseCntr + 1)) || ((hWindow->iPhaseCntr + 1) < hWindow->iPrevPhaseCntr)))
                        {
                            BDBG_WRN(("MTG: The deinterlacer hasn't achieve cadence lock yet. If this message stops, that means the deinterlacer has achieved lock."));
                            BDBG_MSG(("MTG algorithm not locked: prev prev phase 12 displayed %d times, prev phase 340 displayed %d times, times diff > 1",
                                      hWindow->iPrevPhaseCntr, hWindow->iPhaseCntr));
                        }

                        hWindow->iPrevPhaseCntr = hWindow->iPhaseCntr;
                        hWindow->iPhaseCntr = -1;
                    }
                }
                else
                {
                    /* current phase 340 */
                    if (hWindow->iPhaseCntr >= 0)
                    {
                        /* same phase continues */
                        hWindow->iPhaseCntr ++;
                    }
                    else
                    {
                        /* previous phase was 12, now we switch to phase 340 */
                        if ((0!=hWindow->iPrevPhaseCntr) && (hWindow->iLockCntr > 5*3) &&
                            (((-hWindow->iPhaseCntr) > (hWindow->iPrevPhaseCntr + 1)) || (((-hWindow->iPhaseCntr) + 1) < hWindow->iPrevPhaseCntr)))
                        {
                            BDBG_WRN(("MTG: The deinterlacer hasn't achieve cadence lock yet. If this message stops, that means the deinterlacer has achieved lock."));
                            BDBG_MSG(("MTG algorithm not locked: prev prev phase 340 displayed %d times, prev phase 12 displayed %d times, times diff > 1",
                                      hWindow->iPrevPhaseCntr, -hWindow->iPhaseCntr));
                        }
                        hWindow->iPrevPhaseCntr = -hWindow->iPhaseCntr;
                        hWindow->iPhaseCntr = 1;
                    }
                }
            }
            else if (pCurInfo->hSource->bMtgSrc && !hWindow->stCurInfo.bMosaicMode &&
                     BVDC_P_MVP_USED_MAD_AT_WRITER(hWindow->stVnetMode, hWindow->stMvpMode) &&
                     eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase)
            {
                BDBG_MODULE_MSG(BVDC_MTGR,("read win%d (n%d p%d), no lock", hWindow->eId, pPicture->ulBufferId, pPicture->ulMadOutPhase));
                hWindow->iPhaseCntr = 0;
                hWindow->iPrevPhaseCntr = 0;
                hWindow->iLockCntr = 0;
            }
            else
            {
                BDBG_MODULE_MSG(BVDC_MTGR,("read Win[%d]: (pic %x rp %d)", hWindow->eId, pPicture->ulDecodePictureId, pPicture->stFlags.bPictureRepeatFlag));
            }

            if (pPicture->stFlags.bRev32Locked && eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase)
            {
                BDBG_MODULE_MSG(BVDC_MTGR__,("read win%d (n%d p%d)", hWindow->eId, pPicture->ulBufferId, pPicture->ulMadOutPhase));
            }
            else if (pCurInfo->hSource->bMtgSrc && !hWindow->stCurInfo.bMosaicMode &&
                     BVDC_P_MVP_USED_MAD_AT_WRITER(hWindow->stVnetMode, hWindow->stMvpMode) &&
                     eMtgMode == BVDC_P_Buffer_MtgMode_eMadPhase )
            {
                BDBG_MODULE_MSG(BVDC_MTGR__,("read win%d (n%d p%d), no lock", hWindow->eId, pPicture->ulBufferId, pPicture->ulMadOutPhase));
            }
#endif
        }
    }
    else
    {
        pPicture = &hWindow->stCurInfo.hSource->hVfdFeeder->stPicture;
    }
    BDBG_ASSERT (pPicture);

    /* Set eDstPolarity before set playback */
    if(BVDC_P_VNET_USED_SCALER_AT_READER(hWindow->stVnetMode))
    {
        /* If scaler was used to correct field inverstion here are thing that
         * we need to keep in mind.  If scaler is after capture the destination
         * polarity is determine now (in reader), but if scaler before capture
         * the is destination polarity already determined (in writer). */
#if BFMT_LEGACY_3DTV_SUPPORT
        if(BFMT_IS_CUSTOM_1080P3D(hCompositor->stCurInfo.pFmtInfo->eVideoFmt))
        {
            pPicture->eDstPolarity = BAVC_Polarity_eFrame;
        }
        else
#endif
        {
            pPicture->eDstPolarity = eFieldId;
        }
    }

    /* Save the display polarity as a feedback to help writer to predict
     * destination polarity in the next round. Or in the case that
     * reader node gets moved by writer ISR, this will help predict the
     * VEC polarity.
     */
    pPicture->eDisplayPolarity = eFieldId;

    /* Prevent this from flooding the screen.
    BDBG_MSG(("Win%d R(%d): B(%d) (%d -> %d)", hWindow->eId, eFieldId,
        pPicture->ulBufferId, pPicture->eSrcPolarity, pPicture->eDstPolarity));
    */
    if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId))
    {
        BVDC_P_Source_MpegGetStatus_isr(pCurInfo->hSource, &bVideoDetect);
    }
    else if(BVDC_P_SRC_IS_HDDVI(hWindow->stCurInfo.hSource->eId))
    {
        BVDC_P_HdDvi_GetStatus_isr(pCurInfo->hSource->hHdDvi, &bVideoDetect);
    }
    else if(BVDC_P_SRC_IS_ITU656(hWindow->stCurInfo.hSource->eId))
    {
#if (BVDC_P_SUPPORT_NEW_656_IN_VER)
        BVDC_P_656In_GetStatus_isr(hWindow->stCurInfo.hSource->h656In, &bVideoDetect);
#endif
    }

    /* Update reader node's playback time stamp in either slip case above */
    if(!hWindow->bSyncLockSrc)
    {
        /* also calculate the absolute actual buf delay of the previous picture node;
           Note: if source lost signal, don't need to update the current buf delay
                 or make any clock adjustment;
         */
        /* counting down TS sampling vsyncs */
        if(hWindow->hBuffer->ulGameDelaySampleCnt == 0)
        {
            /* reset sampling count only for new picture */
            if(!pPicture->stFlags.bPictureRepeatFlag)
            {
                hWindow->hBuffer->ulGameDelaySampleCnt = hWindow->hBuffer->ulGameDelaySamplePeriod - 1;
            }

            /* alignment measurement */
            if( hWindow->pCurReaderNode && hWindow->stCurInfo.hSource->bStartFeed)
            {
                bool   bValidDelay;
                hWindow->ulCurBufDelay = BVDC_P_Buffer_CalculateBufDelay_isr(
                    hWindow->pCurReaderNode, &bValidDelay);

                /* check game mode clock adjustment */
                if(pCurInfo->stGameDelaySetting.bEnable && bValidDelay)
                {
                    #define BVDC_P_GAME_MODE_PXL_FREQ_MAX   (14850) /* 1080p60 pxl freq */
                    int32_t lTolerance =
                        (int32_t)pCurInfo->stGameDelaySetting.ulBufferDelayTolerance;

                    hWindow->lCurGameModeLag = hWindow->ulCurBufDelay -
                        pCurInfo->stGameDelaySetting.ulBufferDelayTarget;

                    if((hWindow->lCurGameModeLag > lTolerance) ||
                       (hWindow->lCurGameModeLag < (-lTolerance)))
                    {
                        /* Note: no adjustment for rate gap and force repeat; */
                        hWindow->bAdjGameModeClock =
                            (BVDC_P_WrRate_NotFaster == hWindow->hBuffer->eWriterVsReaderRateCode) &&
                            /*(BVDC_P_WrRate_NotFaster == hWindow->hBuffer->eReaderVsWriterRateCode) &&*/
                            (pSrcInfo->eMuteMode != BVDC_MuteMode_eRepeat);

                        /* VESA formats allow 0.3% variance in rate; 59/60 slip without
                           frame rate tracking; both will automatically turn on coarse
                           clock adjustment to track the source rate; */
                        hWindow->bCoarseAdjClock   =
                            pCurInfo->stGameDelaySetting.bForceCoarseTrack ||
                            VIDEO_FORMAT_IS_VESA(pSrcInfo->pFmtInfo->eVideoFmt) ||
                            !pCurInfo->bUseSrcFrameRate;

                        /* No fast adjustment if both source and display formats are high
                           pixel frequency to avoid RTS violation due to display clock change; */
                        hWindow->bFastAdjClock =
                            (pSrcInfo->pFmtInfo->ulPxlFreq < BVDC_P_GAME_MODE_PXL_FREQ_MAX) ||
                            (hCompositor->stCurInfo.pFmtInfo->ulPxlFreq < BVDC_P_GAME_MODE_PXL_FREQ_MAX);
                    }
                    else
                    {
                        hWindow->bAdjGameModeClock = false;
                    }
                }
            }
            else
            {
                hWindow->bAdjGameModeClock = false;
            }
        }
        else
        {
            /* count down and disable alignment control */
            hWindow->hBuffer->ulGameDelaySampleCnt--;
            hWindow->bAdjGameModeClock = false;
        }
        /*
        BDBG_MSG(("Rd buf delay = %d, lag=%d, sample cnt=%d", hWindow->ulCurBufDelay,
            hWindow->lCurGameModeLag, hWindow->hBuffer->ulGameDelaySampleCnt));*/
    }

#if BFMT_LEGACY_3DTV_SUPPORT /* alternate L/R windows visibility for 1080p3D->2160i48 only */
    bOrigMute = pPicture->stFlags.bMute;
    if(BFMT_IS_CUSTOM_1080P3D(hCompositor->stCurInfo.pFmtInfo->eVideoFmt) &&
       BFMT_IS_1080P_3DOU_AS(pSrcInfo->pFmtInfo->eVideoFmt))
    {
        if(BAVC_Polarity_eTopField == eFieldId)
        {
            if(BVDC_P_WindowId_eComp0_V1 == hWindow->eId)
            {
                pPicture->stFlags.bMute = true;
            }
        }
        else
        {
            if(BVDC_P_WindowId_eComp0_V0 == hWindow->eId)
            {
                pPicture->stFlags.bMute = true;
            }
        }
    }
#endif

    /* General mute when picture is not ready, shutdown is requested! */
    if(pPicture->stFlags.bMute)
    {
        /*BDBG_MSG(("Window[%d] R: %s(%d) mutes", hWindow->eId,
            eFieldId ? "B" : "T", pPicture->ulBufferId)); */

        /* size must be set so constant color doesn't hang compositor */
        BVDC_P_Window_SetMutedSurSize_isr(hWindow, eFieldId);

        /* so that cmp would build less to RUL */
        BVDC_P_Window_WinOutSetEnable_isr(hWindow, 0, false);

        goto done;
    }

    /* Note: only one video window could be the master. */
    if((pCurInfo->bUseSrcFrameRate) &&
       (BAVC_FrameRateCode_eUnknown != pPicture->eFrameRateCode))
    {
        if(hCompositor->eSrcFRateCode != pPicture->eFrameRateCode)
        {
            hCompositor->eSrcFRateCode = pPicture->eFrameRateCode;
            hCompositor->bFullRate     =
                BVDC_P_IS_FULL_FRAMRATE(pPicture->eFrameRateCode);
        }
    }

    /* If we're doing fixed color make sure to bypass colorspace conversion.
     * otherwise fixed color going thru one of the color coversion will be
     * different due precisions lost.  */
    if((BVDC_MuteMode_eConst == pSrcInfo->eMuteMode) ||
       (pPicture->stFlags.bMuteFixedColor) ||
       (hWindow->bMuteBypass))
    {
        bFixedColor = true;
    }

    /* maybe used by VFC first or CMP CFC */
    hWindow->bCfcDirty |= hCompositor->stCurInfo.stDirty.stBits.bOutColorSpace;

    /* Note: the reader side setting should start from upstream then to the downstream. */
    /* Setup video feeder that playback from captured buffer */
    if(BVDC_P_VNET_USED_PLAYBACK(hWindow->stVnetMode))
    {
        BVDC_P_Feeder_SetPlaybackInfo_isr(hWindow->stCurResource.hPlayback,
            pPicture, bFixedColor, pSrcInfo->ulMuteColorYCrCb);
    }

    else if (BVDC_P_SRC_IS_VFD(pCurInfo->hSource->eId))
    {
        BVDC_P_Feeder_SetPlaybackInfo_isr(pCurInfo->hSource->hVfdFeeder,
             pPicture, bFixedColor, pSrcInfo->ulMuteColorYCrCb);
    }

#if (BVDC_P_SUPPORT_DNR)
    if(BVDC_P_VNET_USED_DNR_AT_READER(hWindow->stVnetMode))
    {
        /* TODO: avoid duplicate setinfo when dnr is shared */
        BVDC_P_Dnr_SetInfo_isr(hWindow->stCurResource.hDnr, pPicture);
    }
#endif

#if (BVDC_P_SUPPORT_XSRC)
    if(BVDC_P_VNET_USED_XSRC_AT_READER(hWindow->stVnetMode))
    {
        /* TODO: avoid duplicate setinfo when xsrc is shared */
        BVDC_P_Xsrc_SetInfo_isr(hWindow->stCurResource.hXsrc, hWindow, pPicture);
    }
#endif

#if (BVDC_P_SUPPORT_VFC)
    if(BVDC_P_VNET_USED_VFC_AT_READER(hWindow->stVnetMode))
    {
        BVDC_P_Vfc_SetInfo_isr(hWindow->stCurResource.hVfc, hWindow, pPicture);
    }
#endif

    /* NOTE: we should config MAD32 before SCL since the picture node's source
     * polarity could be modified by mad setinfo_isr; */
    if(BVDC_P_VNET_USED_MVP_AT_READER(pPicture->stVnetMode))
    {
        BVDC_P_MCVP_SetInfo_isr(hWindow->stCurResource.hMcvp, hWindow, pPicture);

        if(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode))
        {
            BVDC_P_Window_AdjustPicRepeatBit_isr(hWindow, pPicture);
            pPicture->eSrcPolarity = BAVC_Polarity_eFrame;
            BVDC_P_Window_AdjustForMadDelay_isr(hWindow, pPicture);
        }
    }

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_READER(hWindow->stVnetMode) && hWindow->stVnetMode.stBits.bTntdBeforeScl)
    {
        BVDC_P_Tntd_SetInfo_isr(hWindow->stCurResource.hTntd, pPicture);
    }
#endif

    if(BVDC_P_VNET_USED_SCALER_AT_READER(hWindow->stVnetMode))
    {
        BVDC_P_Scaler_SetInfo_isr(hWindow->stCurResource.hScaler, pPicture);
    }

#if (BVDC_P_SUPPORT_TNTD)
    if(BVDC_P_VNET_USED_TNTD_AT_READER(hWindow->stVnetMode) && !hWindow->stVnetMode.stBits.bTntdBeforeScl)
    {
        BVDC_P_Tntd_SetInfo_isr(hWindow->stCurResource.hTntd, pPicture);
    }
#endif

    if(BVDC_P_MVP_USED_MAD_AT_READER(pPicture->stVnetMode, pPicture->stMvpMode))
    {
        /* restore original source polarity in case the next time sync slip */
        pPicture->eSrcPolarity = pPicture->PicComRulInfo.eSrcOrigPolarity;
    }

#if(BVDC_P_SUPPORT_HIST)
    if(hWindow->eId == BVDC_P_WindowId_eComp0_V0)
        BVDC_P_Histo_UpdateHistoData_isr(hWindow->stCurResource.hPep);
#endif

    if(hWindow->stResourceRequire.bRequirePep)
    {
        BVDC_P_Pep_SetInfo_isr(hWindow->stCurResource.hPep, pPicture);
    }

    /* Update the compositor's surface color space conversion matrix. Note this must be after possible VFC update. */
    hWindow->bCfcDirty |= BVDC_P_Window_AssignMosaicCfcToRect_isr(
        hWindow, pPicture, hCompositor->stCurInfo.stDirty.stBits.bOutColorSpace);
    if (hWindow->bCfcDirty || pCurDirty->stBits.bMosaicMode)
    {
#if (BDBG_DEBUG_BUILD)
        BVDC_P_WindowId eV0Id = BVDC_P_CMP_GET_V0ID(hCompositor);
#endif
        BDBG_MODULE_MSG(BVDC_CFC_5,("Cmp%d_V%d invokes setBypassColor_isr by %s due to (%d || %d)",
            hCompositor->eId, hWindow->eId-eV0Id, BSTD_FUNCTION, hWindow->bCfcDirty, pCurDirty->stBits.bMosaicMode));
        BVDC_P_Window_SetBypassColor_isr(hWindow, bFixedColor);
    }
    if(!hCompositor->bIsBypass && hWindow->bCfcDirty)
    {
        /* Select the color space conversion. */
#if BVDC_P_CMP_0_MOSAIC_CFCS
        if(hWindow->stCurInfo.bClearRect)
        {
            uint16_t i, ulCmpNumCscs;
            ulCmpNumCscs = (BVDC_CompositorId_eCompositor0 != hCompositor->eId)?
                BVDC_P_CMP_i_MOSAIC_CFCS : BVDC_P_CMP_CFCS;
            for (i = 0; i < ulCmpNumCscs; i++)
            {
                hWindow->astMosaicCfc[i].bForceRgbPrimaryMatch = hWindow->stCurInfo.bCscRgbMatching;
                BVDC_P_Cfc_UpdateCfg_isr(&hWindow->astMosaicCfc[i], true, true);
            }

            /* set the mosaic csc indices into shadow regs */
            BVDC_P_Window_SetMosaicCsc_isr(hWindow);
        }
        else
#endif
        {
            /* pass bForceDirty as true because hCompositor->stCurInfo.stDirty.stBits.bOutColorSpace could be cleared by VFC
             * and also because cfc cfg needs to be updated if win Hue/Contrast/Attenuation/... changes */
            hWindow->pMainCfc->bForceRgbPrimaryMatch = hWindow->stCurInfo.bCscRgbMatching;
            BVDC_P_Cfc_UpdateCfg_isr(hWindow->pMainCfc, false, true);
            if(pCurInfo->bUserCsc)
            {
                BVDC_P_Cfc_FromMatrix_isr(hWindow->pMainCfc, pCurInfo->pl32_Matrix, pCurInfo->ulUserShift);
            }
        }

        /* set count to let cmp RUL update matrix */
        hCompositor->ulCscAdjust[hWindow->eId] = BVDC_P_RUL_UPDATE_THRESHOLD;
        hCompositor->bCscCompute[hWindow->eId] = true;
        hCompositor->bCscDemoCompute[hWindow->eId] = true;
    }
    hWindow->bCfcDirty = BVDC_P_CLEAN;
    pCurDirty->stBits.bMosaicMode = BVDC_P_CLEAN;

    if(!hCompositor->bIsBypass)
    {
        /* Evaluate dither condition for compositor: */
        /* Input Dither is enable if source is 10-bit and window is 10-bit */
        /* and MADR is not in the path or not enable */
        bool bInDitherEnable = (pPicture->bSrc10Bit && hWindow->bIs10BitCore &&
           !(BVDC_P_MVP_USED_MAD(pPicture->stMvpMode) &&
             hWindow->stCurResource.hMcvp->hMcdi->bMadr)) ? true : false;
        /* CSC dither is enabled with the above condition and HDMI output */
        /* color depth is 8-bit */
        bool bHdmiOutput8Bit =
            ((hCompositor->hDisplay->stDviChan.bEnable || hCompositor->hDisplay->stCurInfo.bEnableHdmi) &&
             (hCompositor->hDisplay->stCurInfo.stHdmiSettings.eHdmiColorDepth == BAVC_HDMI_BitsPerPixel_e24bit)) ? true : false;
        bool bCscDitherEnable = bInDitherEnable & bHdmiOutput8Bit & !hWindow->stCurInfo.bMosaicMode;
#if BVDC_P_DBV_SUPPORT && (BVDC_DBV_MODE_BVN_CONFORM)
        bCscDitherEnable &= !BVDC_P_CMP_DBV_MODE(hCompositor);
#endif
        /* disable input dithering for transcode path */
        bInDitherEnable &= !hCompositor->hDisplay->stCurInfo.bEnableStg;
#if BVDC_DITHER_OFF
        bInDitherEnable = false;
        bCscDitherEnable = false;
#endif
        if(bInDitherEnable != hCompositor->bInDitherEnable[hWindow->eId] ||
           bCscDitherEnable != hCompositor->bCscDitherEnable[hWindow->eId])
        {
            hCompositor->bInDitherEnable[hWindow->eId] = bInDitherEnable;
            hCompositor->bCscDitherEnable[hWindow->eId] = bCscDitherEnable;
            hCompositor->ulDitherChange[hWindow->eId] = BVDC_P_RUL_UPDATE_THRESHOLD;
        }
    }

    /* Update the informations that are tied to pPicture;
     * don't set the window's canvas position yet since it might need adjustment
     * if its compositor has a VBI pass-throughed window; */
    ulRWindowXOffset = (uint32_t) (hWindow->stCurInfo.lRWinXOffsetDelta + pPicture->pWinOut->lLeft);

#if BVDC_P_SUPPORT_STG /* update the stg format from picture pipe */
    if(BVDC_P_DISPLAY_USED_STG(hWindow->hCompositor->hDisplay->eMasterTg))
    {
        hWindow->hCompositor->hDisplay->pStgFmtInfo =
            BVDC_P_IS_CUSTOMFMT(pPicture->pStgFmtInfo->eVideoFmt)?
            &pPicture->stCustomFormatInfo:pPicture->pStgFmtInfo;
        /*BDBG_MSG(("win%u disp pStgFmt %p[%ux%up%u]", hWindow->eId, hWindow->hCompositor->hDisplay->pStgFmtInfo,
            hWindow->hCompositor->hDisplay->pStgFmtInfo->ulDigitalWidth, hWindow->hCompositor->hDisplay->pStgFmtInfo->ulHeight,
            hWindow->hCompositor->hDisplay->pStgFmtInfo->ulVertFreq));*/
    }
#endif

    BVDC_P_Window_SetSurfaceSize_isr(hWindow, pPicture->pWinIn, eFieldId);
    BVDC_P_Window_SetDisplaySize_isr(hWindow, pPicture->pWinOut, eFieldId,ulRWindowXOffset);

    /* compositor check the sur enable bit to start program  */
    if(hWindow->ulBlenderMuteCnt)
        hWindow->ulBlenderMuteCnt--;
    BVDC_P_Window_WinOutSetEnable_isr(hWindow, pPicture->ucAlpha, !hWindow->ulBlenderMuteCnt);

    /* MosaicMode: update ClearRect mask */
#if BVDC_P_SUPPORT_CMP_CLEAR_RECT
    if(pCurInfo->bClearRect)
    {
        hWindow->ulMosaicRectSet = pPicture->ulMosaicRectSet;
        BVDC_P_Window_SetClearRect_isr(hWindow, pCurInfo, pPicture);
    } else
    {
        BVDC_P_Window_SetBgColor_isr(hWindow, pPicture, eFieldId);
    }
#endif

    /* This needs to be called after BVDC_P_Window_CheckReaderIsrOrder_isr,
     * where BVDC_P_Window_UpdateTimestamps_isr is called.  Only when user
     * interested in callback event.  Or the first time. */
    BVDC_P_Window_UpdateCallback_isr(hWindow, eFieldId);

    /* Update the corresponding STG meta data*/
    BVDC_P_Compositor_SetMBoxMetaData_isr(pPicture, hWindow->hCompositor);

done:
    hWindow->pCurReaderNode = pPicture;
#if BFMT_LEGACY_3DTV_SUPPORT
    pPicture->stFlags.bMute = bOrigMute;
#endif

    BDBG_LEAVE(BVDC_P_Window_Reader_isr);
    return;
}

static void BVDC_P_Window_WinOutBuildRul_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_PictureNode              *pPicture,
      BVDC_P_ListInfo                 *pList,
      BVDC_P_State                     eVnetState )
{
    uint32_t  ulRulOpsFlags;
    BVDC_Compositor_Handle hCompositor;

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    hCompositor = hWindow->hCompositor;
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    /* Winow_BuildReaderRul_isr could be called before the surface in comp is
     * really enabled. BVDC_P_SubRul_GetOps_isr's internal state could be mess-up
     * if we call it in this case */
    ulRulOpsFlags = BVDC_P_SubRul_GetOps_isr(&(hWindow->stWinOutVnet),
             hWindow->eId, eVnetState, pList->bLastExecuted);

    /* join in vnet after enable. note: its src mux is initialed as disabled */
    if(ulRulOpsFlags & BVDC_P_RulOp_eEnable)
    {
        /* HW does not require to write SURFACE_CTRL_ENABLE every vsync, but we found
         * main win-sur disabled after PIP is opened, so we write it every vsync */
        if(hWindow->ulBlenderMuteCnt)
            hWindow->ulBlenderMuteCnt--;
        BVDC_P_Window_WinOutSetEnable_isr(hWindow, pPicture->ucAlpha, !hWindow->ulBlenderMuteCnt);

        BVDC_P_WIN_WRITE_TO_RUL(CMP_0_V0_SURFACE_CTRL, pList->pulCurrent);

        /* join in vnet after enable. note: its src mux is initialed as disabled */
        if(ulRulOpsFlags & BVDC_P_RulOp_eVnetInit)
        {
            BVDC_P_SubRul_JoinInVnet_isr(&(hWindow->stWinOutVnet), pList);
        }
    }
    else
    {
        if(ulRulOpsFlags & BVDC_P_RulOp_eDisable)
        {
            BVDC_P_SubRul_DropOffVnet_isr(&(hWindow->stWinOutVnet), pList);

            /* win sur will not be disabled automatically per vsync */
#if BVDC_P_SUPPORT_WIN_CONST_COLOR
            BVDC_P_Window_WinOutSetEnable_isr(hWindow, pPicture->ucAlpha, false);
#else
            BVDC_P_Window_WinOutSetEnable_isr(hWindow, 0, false);
#endif

            BVDC_P_WIN_WRITE_TO_RUL(CMP_0_V0_SURFACE_CTRL, pList->pulCurrent);
        }
        else if (ulRulOpsFlags & BVDC_P_RulOp_eDrainVnet)
        {
            /* save current drain mux value */
            BVDC_P_SubRul_Drain_isr(&(hWindow->stWinOutVnet), pList, 0, 0 /* 0 means no need to reset */, 0, 0);
        }
    }
}

/***************************************************************************
 * {private}
 * note: a sub-module might be shared by more than one window, but the
 * sharing is handled by each sub-module. And each sub-module will
 * acquire and release the pre-patch of free-channel or loop-back
 * internally basing on eVnetPatchMode.
 *
 * return: if reader is on
 */
static bool BVDC_P_Window_BuildReaderRul_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eNextFieldId,
      bool                             bBuildCanvasCtrl )
{
    BVDC_Compositor_Handle hCompositor;
    BVDC_P_PicComRulInfo *pPicComRulInfo;
    BVDC_P_State  eReaderState;
    BVDC_P_PictureNode  *pPicture;

    BDBG_ENTER(BVDC_P_Window_BuildReaderRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    hCompositor = hWindow->hCompositor;
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);
    BDBG_OBJECT_ASSERT(hCompositor->hDisplay, BVDC_DSP);

    /* Note: When building RUL, hardware highly recommend that we build from
     * backend to the frontend, that is starting with:
     *   VEC, CMP, SCL, VFD, CAP, MFD.  This to prevent the false start of
     * downstream modules.
     * Note: In the case readerState is not eActive, we still needs to build
     * RUL in order to shut down. VnetMode is the only one that indicates if
     * we need to build RUL */
    if(BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId))
    {
        if((BVDC_P_State_eInactive == hWindow->stCurInfo.eReaderState) &&
           (hWindow->stCurInfo.bVisible))
        {
            hWindow->stCurInfo.eReaderState = BVDC_P_State_eActive;
            hWindow->stCurInfo.eWriterState = BVDC_P_State_eActive;
        }

        eReaderState = hWindow->stCurInfo.eReaderState;

        /* VEC alignment may be too fast to sustain gfx RTS; mute it! */
        if((BVDC_P_State_eActive == eReaderState) &&
           ((hCompositor->hDisplay->bAlignAdjusting && !hCompositor->hDisplay->stCurInfo.stAlignCfg.bKeepBvnConnected)||

            (hCompositor->hDisplay->stCurInfo.stDirty.stBits.bTiming) ||
            (0==hWindow->stCurInfo.hSource->hGfxFeeder->stGfxSurface.stCurSurInfo.ullAddress) || /* no valid sur */

            (!hWindow->stCurInfo.bVisible)))  /* muted by user */
        {
            eReaderState = BVDC_P_State_eShutDownRul;
        }

        /* according to readerState, enable or disable gfx surface in cmp,
         * and build rul for GFD */
        if (BVDC_P_State_eActive != eReaderState)
        {
            BVDC_P_WIN_WRITE_IMM_TO_RUL(CMP_0_V0_SURFACE_CTRL, 0, pList->pulCurrent);
        }
        else
        {
            BVDC_P_WIN_GET_REG_DATA(CMP_0_V0_SURFACE_CTRL) |=  (
                BCHP_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, 1));
            BVDC_P_WIN_WRITE_TO_RUL(CMP_0_V0_SURFACE_CTRL, pList->pulCurrent);
        }

        if (hWindow->stCurInfo.hSource->hGfxFeeder->stGfxSurface.stCurSurInfo.ullAddress)
        {
            BVDC_P_GfxFeeder_BuildRul_isr(hWindow->stCurInfo.hSource->hGfxFeeder,
                pList, eNextFieldId, eReaderState);
        }
    }
    else /* video window */
    {
        eReaderState = hWindow->stCurInfo.eReaderState;

        if (BVDC_P_SRC_IS_VFD(hWindow->stCurInfo.hSource->eId))
        {
            pPicture = &hWindow->stCurInfo.hSource->hVfdFeeder->stPicture;
            pPicComRulInfo = &pPicture->PicComRulInfo;
        }
        else
        {
            pPicture = BVDC_P_Buffer_GetCurrReaderNode_isr(hWindow->hBuffer);
            pPicComRulInfo = &((BVDC_P_Buffer_GetCurrReaderNode_isr(hWindow->hBuffer))->PicComRulInfo);
        }
        pPicComRulInfo->eWin = hWindow->eId;

#if (BVDC_P_SHOW_VNET_MSG==1)
#if BVDC_P_SUPPORT_WIN_CONST_COLOR
        if(pPicture->stFlags.bMute !=
           (BVDC_P_WIN_COMPARE_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, CONST_COLOR_ENABLE, 1) ||
            BVDC_P_WIN_COMPARE_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, 0)))
#else
        if(pPicture->stFlags.bMute !=
           BVDC_P_WIN_COMPARE_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, 0))
#endif
        {
            if(!((!pPicture->stFlags.bMute) &&
                  ((BVDC_P_State_eShutDownRul == hWindow->stCurInfo.eReaderState) ||
                   (BVDC_P_State_eShutDown == hWindow->stCurInfo.eReaderState))))
            {
                BDBG_MODULE_MSG(BVDC_WIN_VNET,("win[%d] build reader: bPicMute != bSurDis, bMute %d, readerState %d, writerState %d",
                    hWindow->eId, pPicture->stFlags.bMute? 1 : 0,
                    hWindow->stCurInfo.eReaderState, hWindow->stCurInfo.eWriterState));
            }
        }
#endif

        /* Fast VEC alignment should mute the video window to avoid RTS failure */
        /* sur should ahve been enabled in reader_isr.   */
        if((eReaderState == BVDC_P_State_eActive) &&
           ((pPicture->stFlags.bMute) ||
            (!hWindow->stCurInfo.bVisible && hWindow->bCapture) ||
            (hCompositor->hDisplay->bAlignAdjusting && !hCompositor->hDisplay->stCurInfo.stAlignCfg.bKeepBvnConnected) ||
            (BVDC_P_VNET_IS_INVALID(hWindow->stVnetMode)) ||
            (BVDC_P_State_eInactive == hWindow->stCurInfo.eWriterState) ||
            (BVDC_P_WIN_COMPARE_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, 0))))
        {
            BDBG_MODULE_MSG(BVDC_WIN_VNET,("win[%d] build reader: force down, bMute %d, bDsipAlign %d, bSurEn %d, vnetMode 0x%x,  writerState %d",
                hWindow->eId, pPicture->stFlags.bMute? 1 : 0, hCompositor->hDisplay->bAlignAdjusting? 1 : 0,
                BVDC_P_WIN_COMPARE_FIELD_DATA(CMP_0_V0_SURFACE_CTRL, SURFACE_ENABLE, 1)? 1 : 0,
                *(uint32_t *) &hWindow->stVnetMode, hWindow->stCurInfo.eWriterState));

            /* this will cause reader modules to drop off vnet and disable. If
             * pic ummute again later without vnet-reconfigure, the reader
             * modules will automatically re-join into vnet and enable.
             * This is handled in subrul. */
            eReaderState = BVDC_P_State_eShutDownRul;

            if (BVDC_P_VNET_IS_INVALID(hWindow->stVnetMode))
            {
#if BVDC_P_SHOW_VNET_MSG
                BVDC_P_Window_DirtyBits  *pDirty = &(hWindow->stCurInfo.stDirty);

                BDBG_MODULE_MSG(BVDC_WIN_VNET,("win[%d] build reader RUL with NULL VnetMode, readerState %d, writerState %d",
                    hWindow->eId, hWindow->stCurInfo.eReaderState, hWindow->stCurInfo.eWriterState));
                BDBG_MODULE_MSG(BVDC_WIN_VNET,("   bShutDown %d, bRecfgVnet %d, bSrcPending %d, dirty 0x%08lx",
                    pDirty->stBits.bShutdown, pDirty->stBits.bReConfigVnet, pDirty->stBits.bSrcPending, BVDC_P_CAST_DIRTY(pDirty)));
#endif
                /* clean up to avoid to build rul for a module accidently */
                BVDC_P_Window_SetInvalidVnetMode_isr(&(hWindow->stVnetMode));
            }
        }

        if(BVDC_P_VNET_USED_VNETCRC_AT_READER(hWindow->stVnetMode))
        {
            BVDC_P_VnetCrc_BuildRul_isr(&(hWindow->stCurResource.hVnetCrc), pList, eReaderState,
                pPicComRulInfo, hWindow->stCurInfo.stCbSettings.stMask.bCrc);
        }

        BVDC_P_Window_WinOutBuildRul_isr(hWindow, pPicture, pList, eReaderState);
        if (bBuildCanvasCtrl)
        {
            BVDC_P_Compositor_BuildConvasCtrlRul_isr(hWindow->hCompositor, pList);
        }

#if (BVDC_P_SUPPORT_TNTD)
        if(BVDC_P_VNET_USED_TNTD_AT_READER(hWindow->stVnetMode) && !hWindow->stVnetMode.stBits.bTntdBeforeScl)
        {
            BVDC_P_Tntd_BuildRul_isr(hWindow->stCurResource.hTntd, pList, eReaderState, pPicComRulInfo);
        }
#endif

        if(BVDC_P_VNET_USED_SCALER_AT_READER(hWindow->stVnetMode))
        {
            BVDC_P_Scaler_BuildRul_isr(hWindow->stCurResource.hScaler, pList, eReaderState, pPicComRulInfo);
        }

#if (BVDC_P_SUPPORT_TNTD)
        if(BVDC_P_VNET_USED_TNTD_AT_READER(hWindow->stVnetMode) && hWindow->stVnetMode.stBits.bTntdBeforeScl)
        {
            BVDC_P_Tntd_BuildRul_isr(hWindow->stCurResource.hTntd, pList, eReaderState, pPicComRulInfo);
        }
#endif

        /* note: with 7420, BVDC_P_SUPPORT_HSCL/ANR/MAD is 0, and
         * BVDC_P_MVP_USED_HSCL/ANR/MAD(hWindow->stVnetMode) is false too */
        if(BVDC_P_VNET_USED_MVP_AT_READER(hWindow->stVnetMode))
        {
#if BVDC_P_STG_RUL_DELAY_WORKAROUND
            bool bMadr;
            BVDC_P_Mcdi_GetDeinterlacerType_isr(hWindow->stCurResource.hMcvp->hMcdi, &bMadr);
            BVDC_P_STG_DelayRUL_isr (hWindow->hCompositor->hDisplay, pList, bMadr);
#endif
            BVDC_P_Mcvp_BuildRul_isr(hWindow->stCurResource.hMcvp, pList, eReaderState, hWindow, pPicture);
        }

#if (BVDC_P_SUPPORT_VFC)
        if(BVDC_P_VNET_USED_VFC_AT_READER(hWindow->stVnetMode))
        {
            BVDC_P_Vfc_BuildRul_isr(hWindow->stCurResource.hVfc, pList, eReaderState, pPicComRulInfo);
        }
#endif

#if (BVDC_P_SUPPORT_XSRC)
        /* XSRC is always upstream of ANR */
        if(BVDC_P_VNET_USED_XSRC_AT_READER(hWindow->stVnetMode))
        {
            BVDC_P_Xsrc_BuildRul_isr(hWindow->stCurResource.hXsrc, pList, eReaderState, pPicComRulInfo);
        }
#endif

#if (BVDC_P_SUPPORT_DNR)
        /* DNR is always upstream of ANR */
        if(BVDC_P_VNET_USED_DNR_AT_READER(hWindow->stVnetMode))
        {
            BVDC_P_Dnr_BuildRul_isr(hWindow->stCurResource.hDnr, pList, eReaderState, pPicComRulInfo);
        }
#endif

        if(BVDC_P_VNET_USED_PLAYBACK(hWindow->stVnetMode))
        {
            /* 656/dvo master mode fixed.  Don't start the feeder if dtg
             * is disabled on the bypass.  Or another word only start vfd
             * if non-bypass, or bypass w/ 656 or dvo out enable. */
            if((!hCompositor->hDisplay->bIsBypass) ||
               (hCompositor->hDisplay->stCurInfo.bEnable656) ||
               (hCompositor->hDisplay->stCurInfo.bEnableHdmi))
            {
                BVDC_P_Feeder_BuildRul_isr(hWindow->stCurResource.hPlayback,
                    pList, pPicture, eReaderState, pPicComRulInfo);
            }
        }

        else if(BVDC_P_SRC_IS_VFD(hWindow->stCurInfo.hSource->eId))
        {
            BVDC_P_Feeder_BuildRul_isr(hWindow->stCurInfo.hSource->hVfdFeeder,
                pList, pPicture, eReaderState, pPicComRulInfo);
        }
    }

    BDBG_LEAVE(BVDC_P_Window_BuildReaderRul_isr);
    return (BVDC_P_State_eActive == eReaderState);
}

static void BVDC_P_Window_BuildWriterRul_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_ListInfo                 *pList,
      bool                             bReaderOn )
{
    BVDC_P_PictureNode  *pPicture;
    BVDC_P_PicComRulInfo *pPicComRulInfo;
    BVDC_P_State  eWriterState;

    BDBG_ENTER(BVDC_P_Window_BuildWriterRul_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);

    eWriterState = hWindow->stCurInfo.eWriterState;
    if (BVDC_P_SRC_IS_VFD(hWindow->stCurInfo.hSource->eId))
    {
        pPicture = &hWindow->stCurInfo.hSource->hVfdFeeder->stPicture;
        pPicComRulInfo = &hWindow->stCurInfo.hSource->hVfdFeeder->stPicture.PicComRulInfo;
    }
    else
    {
        pPicture = BVDC_P_Buffer_GetCurrWriterNode_isr(hWindow->hBuffer);
        pPicComRulInfo = &(pPicture->PicComRulInfo);
    }
    pPicComRulInfo->eWin = hWindow->eId;

    /* Note: When building RUL, hardware highly recommend that we build from
     * backend to the frontend, that is starting with:
     *   VEC, CMP, SCL, VFD, CAP, MFD.  This to prevent the false start of
     * downstream modules.
     * Note: In the case writerState is not eActive, we still needs to build
     * RUL in order to shut down. VnetMode is the only one that indicates if
     * we need to build RUL */
    if(!BVDC_P_VNET_IS_INVALID(hWindow->stVnetMode))
    {
        /* src is not feeding, must have lost signal, we need to tear off writer vnet
         * right away so that src can have a free run and get locked again. The loss
         * of signal will cause vnet reconfigure later. In case that src start to feed
         * again without vnet-reconfigure, the writer modules would also automatically
         * re-join into vnet and enable. This is handled in subrul.
         * we also let writer vnet tear-off as soon as shut-down process starts, in
         * case another win is sharing the src */
        if((BVDC_P_State_eActive == eWriterState) &&
           ((!hWindow->stCurInfo.hSource->bStartFeed) || /* likely vdec / 656 / hdmi src lost */
            ((!BVDC_P_VNET_USED_CAPTURE(hWindow->stVnetMode)) && (!bReaderOn)))) /* syncLocked, reader down */
        {
            eWriterState = BVDC_P_State_eShutDownRul;
            BDBG_MODULE_MSG(BVDC_WIN_VNET,("win[%d] build writer: force down, srcFeed %d, byReader %d, rdState %d, wrtState %d",
                hWindow->eId, hWindow->stCurInfo.hSource->bStartFeed,
                (!BVDC_P_VNET_USED_CAPTURE(hWindow->stVnetMode)) && (!bReaderOn),
                hWindow->stCurInfo.eReaderState, hWindow->stCurInfo.eWriterState));
        }

        /* back -> front for both enabling and disabling (due to new drain alg) */
        if(BVDC_P_VNET_USED_CAPTURE(hWindow->stVnetMode))
        {
            BVDC_P_Capture_BuildRul_isr(hWindow->stCurResource.hCapture, pList, eWriterState, pPicture);
        }

#if (BVDC_P_SUPPORT_TNTD)
        if(BVDC_P_VNET_USED_TNTD_AT_WRITER(hWindow->stVnetMode) && !hWindow->stVnetMode.stBits.bTntdBeforeScl)
        {
            BVDC_P_Tntd_BuildRul_isr(hWindow->stCurResource.hTntd, pList, eWriterState, pPicComRulInfo);
        }
#endif

        if(BVDC_P_VNET_USED_SCALER_AT_WRITER(hWindow->stVnetMode))
        {
            BVDC_P_Scaler_BuildRul_isr(hWindow->stCurResource.hScaler, pList, eWriterState, pPicComRulInfo);
        }

#if (BVDC_P_SUPPORT_TNTD)
        if(BVDC_P_VNET_USED_TNTD_AT_WRITER(hWindow->stVnetMode) && hWindow->stVnetMode.stBits.bTntdBeforeScl)
        {
            BVDC_P_Tntd_BuildRul_isr(hWindow->stCurResource.hTntd, pList, eWriterState, pPicComRulInfo);
        }
#endif

        /* note: with 7420, BVDC_P_SUPPORT_HSCL/ANR/MAD is 0, and
                * BVDC_P_MVP_USED_HSCL/ANR/MAD(hWindow->stVnetMode) is false too */
        if(BVDC_P_VNET_USED_MVP_AT_WRITER(hWindow->stVnetMode))
        {
            BVDC_P_Mcvp_BuildRul_isr(hWindow->stCurResource.hMcvp, pList, eWriterState, hWindow, pPicture);
        }

#if (BVDC_P_SUPPORT_VFC)
        if(BVDC_P_VNET_USED_VFC_AT_WRITER(hWindow->stVnetMode))
        {
            BVDC_P_Vfc_BuildRul_isr(hWindow->stCurResource.hVfc, pList, eWriterState, pPicComRulInfo);
        }
#endif

#if (BVDC_P_SUPPORT_XSRC)
        /* XSRC is always upstream of ANR */
        if(BVDC_P_VNET_USED_XSRC_AT_WRITER(hWindow->stVnetMode))
        {
            BVDC_P_Xsrc_BuildRul_isr(hWindow->stCurResource.hXsrc, pList, eWriterState, pPicComRulInfo);
        }
#endif

#if (BVDC_P_SUPPORT_DNR)
        /* DNR is always upstream of ANR */
        if(BVDC_P_VNET_USED_DNR_AT_WRITER(hWindow->stVnetMode))
        {
            BVDC_P_Dnr_BuildRul_isr(hWindow->stCurResource.hDnr, pList, eWriterState, pPicComRulInfo);
        }
#endif

#if (BVDC_P_SUPPORT_BOX_DETECT)
        if(hWindow->stCurResource.hBoxDetect)
        {
            BVDC_P_BoxDetect_BuildRul_isr(&(hWindow->stCurResource.hBoxDetect), pList, eWriterState,
                pPicComRulInfo, pPicture->pSrcOut, hWindow->stCurInfo.bBoxDetect);
            hWindow->stCurInfo.stDirty.stBits.bBoxDetect = 0;
        }
#endif

        if(BVDC_P_VNET_USED_VNETCRC_AT_WRITER(hWindow->stVnetMode))
        {
            BVDC_P_VnetCrc_BuildRul_isr(&(hWindow->stCurResource.hVnetCrc), pList, eWriterState,
                pPicComRulInfo, hWindow->stCurInfo.stCbSettings.stMask.bCrc);
        }

        if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId))
        {
            BVDC_P_Feeder_BuildRul_isr(hWindow->stCurInfo.hSource->hMpegFeeder,
                pList, pPicture, eWriterState, pPicComRulInfo);
        }
    }
#if BVDC_P_SHOW_VNET_MSG
    else
    {
        BVDC_P_Window_DirtyBits  *pDirty = &(hWindow->stCurInfo.stDirty);
        BDBG_MODULE_MSG(BVDC_WIN_VNET,("win[%d] build writer RUL with NULL VnetMode, readerState %d, writerState %d",
            hWindow->eId, hWindow->stCurInfo.eReaderState, hWindow->stCurInfo.eWriterState));
        BDBG_MODULE_MSG(BVDC_WIN_VNET,("   bShutDown %d, bRecfgVnet %d, bSrcPending %d, dirty 0x%08lx",
            pDirty->stBits.bShutdown, pDirty->stBits.bReConfigVnet, pDirty->stBits.bSrcPending, BVDC_P_CAST_DIRTY(pDirty)));
    }
#endif

    BDBG_LEAVE(BVDC_P_Window_BuildWriterRul_isr);
    return;
}


/***************************************************************************
 * {private}
 * note: a sub-module might be shared by more than one window, but the
 * sharing is handled by each sub-module. And each sub-module will
 * acquire and release the pre-patch of free-channel or loop-back
 * internally basing on eVnetPatchMode.
 */
void BVDC_P_Window_BuildRul_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eNextFieldId,
      bool                             bBuildWriter,
      bool                             bBuildReader,
      bool                             bBuildCanvasCtrl )
{
    BDBG_ENTER(BVDC_P_Window_BuildRul_isr);

    /* Note: When building RUL, hardware highly recommend that we build from
     * backend to the frontend, that is starting with:
     *   VEC, CMP, SCL, VFD, CAP, MFD.  This to prevent the false start of
     * downstream modules. */
    /* TODO: in the case of sync mpeg display, we might want to call
     * BVDC_P_Window_BuildReaderRul_isr and BVDC_P_Window_BuildWriterRul_isr
     * twice in the same vsync/RUL, shut-down old vnet the 1st time and build
     * new vnet the the 2nd time. If we do so, remember to modify subrul to
     * store the last released patch mux addr/mode for RUL loss handling */
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    /*BDBG_MSG(("Build Rul for Window[%d], Reader %d, Writer %d, nextFld %d, List 0x%x",
       hWindow->eId, bBuildReader, bBuildWriter, eNextFieldId, pList));*/
    if((hWindow->stCurInfo.stDirty.stBits.bShutdown) ||
       (BVDC_P_WIN_IS_GFX_WINDOW(hWindow->eId)))
    {
        if(bBuildReader)
        {
            BVDC_P_Window_BuildReaderRul_isr(hWindow, pList, eNextFieldId, bBuildCanvasCtrl);
        }
        if(bBuildWriter)
        {
            BVDC_P_Window_BuildWriterRul_isr(hWindow, pList, false);
        }
    }
    else
    {
        bool  bReaderOn = true; /* default to true for sync slip case */

        if(bBuildReader)
        {
            BVDC_P_Window_AdjustRdState_isr(hWindow, NULL);
            bReaderOn = BVDC_P_Window_BuildReaderRul_isr(hWindow, pList, eNextFieldId, bBuildCanvasCtrl);

            /* PsF: mark the chopped RUL size! */
            if(hWindow->bSyncLockSrc && hWindow->stCurInfo.hSource->bPsfScanout)
            {
                pList->ulPsfMark = (uint32_t)(pList->pulCurrent - pList->pulStart);
            }
        }

        if(bBuildWriter)
        {
            BVDC_P_Window_BuildWriterRul_isr(hWindow, pList, bReaderOn);
        }
    }

    BDBG_LEAVE(BVDC_P_Window_BuildRul_isr);
    return;
}


/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Window_CapturePicture_isr
    ( BVDC_Window_Handle               hWindow,
      BVDC_P_Window_CapturedPicture   *pCapturedPic )
{
    BERR_Code               eRet = BERR_SUCCESS;
    unsigned int            uiPitch;
    BVDC_P_BufferHeapNode  *pHeapNode;
    BVDC_P_BufferHeap_Info *pHeapInfo;
    uint32_t ulBlockOffset = 0;
    BVDC_P_BufferHeapNode  *pHeapNode_R;
    BVDC_P_BufferHeap_Info *pHeapInfo_R;
    uint32_t ulBlockOffset_R = 0;

    BDBG_ENTER(BVDC_P_Window_CapturePicture_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_ASSERT(pCapturedPic);

    /* Clear content */
    BKNI_Memset((void*)pCapturedPic, 0x0, sizeof(BVDC_P_Window_CapturedPicture));

    if(hWindow->uiAvailCaptureBuffers)
    {
        /* Mark the buffer as used by user */
        if((eRet = BVDC_P_Buffer_ExtractBuffer_isr(hWindow->hBuffer, &pCapturedPic->pPicture)))
        {
            pCapturedPic->hPicBlock = NULL;
            pCapturedPic->ulPicBlockOffset = 0;
            pCapturedPic->ulPicBlockOffset_R = 0;

            return eRet;
        }

        /* Decrement number of capture buffers used */
        hWindow->uiAvailCaptureBuffers--;

        pHeapNode = pCapturedPic->pPicture->pHeapNode;

        while (pHeapNode->uiParentNodeBufIndex != BVDC_P_HEAP_INVAID_BUF_INDEX)
        {
            ulBlockOffset += pHeapNode->ulBlockOffset;
            pHeapInfo = pHeapNode->pHeapInfo->pParentHeapInfo;
            pHeapNode = &pHeapInfo->pBufList[pHeapNode->uiParentNodeBufIndex];
        }

        /* Give the MMA block handle to convert a picture to a surface. */
        pCapturedPic->hPicBlock = pHeapNode->pHeapInfo->hMmaBlock;
        pCapturedPic->ulPicBlockOffset = pHeapNode->ulBlockOffset + ulBlockOffset;

        if(pCapturedPic->pPicture->pHeapNode_R != NULL)
        {
            pHeapNode_R = pCapturedPic->pPicture->pHeapNode_R;

            while (pHeapNode_R->uiParentNodeBufIndex != BVDC_P_HEAP_INVAID_BUF_INDEX)
            {
                ulBlockOffset_R += pHeapNode_R->ulBlockOffset;
                pHeapInfo_R = pHeapNode_R->pHeapInfo->pParentHeapInfo;
                pHeapNode_R = &pHeapInfo_R->pBufList[pHeapNode_R->uiParentNodeBufIndex];
            }

            /* Give the MMA block handle to convert a picture to a surface. */
            pCapturedPic->hPicBlock_R = pHeapNode_R->pHeapInfo->hMmaBlock;
            pCapturedPic->ulPicBlockOffset_R = pHeapNode_R->ulBlockOffset + ulBlockOffset_R;
        }

        pCapturedPic->eDispOrientation = pCapturedPic->pPicture->eDispOrientation;

        /* Get polarity */
        pCapturedPic->ePolarity = (BVDC_P_VNET_USED_SCALER_AT_WRITER(pCapturedPic->pPicture->stVnetMode)
                ? pCapturedPic->pPicture->eDstPolarity : pCapturedPic->pPicture->eSrcPolarity);

        /* Get Pixel Format */
        pCapturedPic->ePxlFmt = pCapturedPic->pPicture->ePixelFormat;

        /* Get Height */
        if(pCapturedPic->ePolarity != BAVC_Polarity_eFrame)
        {
            pCapturedPic->ulHeight = pCapturedPic->pPicture->pVfdIn->ulHeight/2;
        }
        else
        {
            pCapturedPic->ulHeight = pCapturedPic->pPicture->pVfdIn->ulHeight;
        }

        /* Get width */
        pCapturedPic->ulWidth = pCapturedPic->pPicture->pVfdIn->ulWidth;

        /* Get pitch. See ulPitch in BVDC_P_Capture_SetEnable_isr and ulStride in
           BVDC_P_Feeder_SetPlaybackInfo_isr */
        BPXL_GetBytesPerNPixels_isr(pCapturedPic->ePxlFmt,
            pCapturedPic->pPicture->pVfdIn->ulWidth, &uiPitch);
        pCapturedPic->ulPitch = BVDC_P_ALIGN_UP(uiPitch, BVDC_P_PITCH_ALIGN);
    }
    else
    {
        pCapturedPic->hPicBlock = NULL;
        pCapturedPic->ulPicBlockOffset = 0;
        pCapturedPic->ulPicBlockOffset_R = 0;
        eRet = BVDC_ERR_NO_AVAIL_CAPTURE_BUFFER;
    }

    BDBG_LEAVE(BVDC_P_Window_CapturePicture_isr);
    return eRet;
}


/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Window_ReleasePicture_isr
    ( BVDC_Window_Handle               hWindow,
      BPXL_Plane                      *pCaptureBuffer )
{
    BVDC_P_PictureNode *pPicture;
    uint32_t cnt = 0;

    BDBG_ENTER(BVDC_P_Window_ReleasePicture_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    pPicture = hWindow->hBuffer->pCurReaderBuf;

    BDBG_MSG(("Returning surface buffer %p", (void *)pCaptureBuffer));

    while ((pCaptureBuffer != pPicture->pSurface) &&
            (cnt < BVDC_P_MAX_USER_CAPTURE_BUFFER_COUNT))
    {
        BVDC_P_Buffer_GetNextUsedByUserNode(pPicture, pPicture);
        cnt++;

        BDBG_MSG(("Surface buffer %p; Is Used? %d",(void *) pPicture->pSurface, pPicture->stFlags.bUsedByUser));
    }

    if(cnt > BVDC_P_MAX_USER_CAPTURE_BUFFER_COUNT)
        return BERR_TRACE(BVDC_ERR_CAPTURED_BUFFER_NOT_FOUND);
    else
    {
        hWindow->stCurInfo.pBufferFromUser = pPicture;
        hWindow->stCurInfo.stDirty.stBits.bUserReleaseBuffer = BVDC_P_DIRTY;
    }

    BDBG_LEAVE(BVDC_P_Window_ReleasePicture_isr);
    return BERR_SUCCESS;
}



/***************************************************************************
 * {private}
 *
 */
bool BVDC_P_Window_CheckForUnReturnedUserCapturedBuffer_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_PictureNode *pPicture;
    bool bBufferHeldByUser = false;
    uint32_t cnt = 0;

    BDBG_ENTER(BVDC_P_Window_CheckForUnReturnedUserCapturedBuffer_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    if(hWindow->hBuffer)
    {
        pPicture = hWindow->hBuffer->pCurReaderBuf;
        if(pPicture)
        {
            while (!pPicture->stFlags.bUsedByUser && (cnt < hWindow->hBuffer->ulActiveBufCnt))
            {
                BVDC_P_Buffer_GetNextActiveNode(pPicture, pPicture);
                cnt++;
            }

            if(cnt >= hWindow->hBuffer->ulActiveBufCnt)
                bBufferHeldByUser = false;
            else
            {
                bBufferHeldByUser = true;
            }
        }
    }

    BDBG_LEAVE(BVDC_P_Window_CheckForUnReturnedUserCapturedBuffer_isr);
    return bBufferHeldByUser;
}


/***************************************************************************
 *
 */
void BVDC_P_Window_CalculateCsc_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_Window_Info *pCurInfo;
    BCFC_Csc3x4 *pCsc;
    BCFC_Csc3x4 *pDemoCsc;
    const BCFC_Csc3x4 *pRGBToYCbCr;
    const BCFC_Csc3x4 *pYCbCrToRGB;

    BDBG_ENTER(BVDC_P_Window_CalculateCsc_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    pCurInfo = &hWindow->stCurInfo;

    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    /*BDBG_MSG(("Window %d: Calculate CSC", hWindow->eId));*/

    pCsc = &hWindow->pMainCfc->stMc;
    pDemoCsc = &hWindow->pDemoCfc->stMc;
    /* use user specified Matrix C as RGB->YCbCr matrix and
    inverse of Matrix C as YCbCr->RGB matrix. */
    BVDC_P_Cfc_GetCfcToApplyAttenuationRGB_isr(hWindow->pMainCfc->pColorSpaceExtOut->stColorSpace.eColorimetry, &pYCbCrToRGB, &pRGBToYCbCr);

    /* apply adjustments to primary color matrix */
    if(pCurInfo->stSplitScreenSetting.eContrast == BVDC_SplitScreenMode_eDisable)
    {
        BVDC_P_Cfc_ApplyContrast_isr(pCurInfo->sContrast, pCsc);
    }
    if(pCurInfo->stSplitScreenSetting.eBrightness == BVDC_SplitScreenMode_eDisable)
    {
        BVDC_P_Cfc_ApplyBrightness_isr(pCurInfo->sBrightness, pCsc);
    }
    if(pCurInfo->stSplitScreenSetting.eHue == BVDC_SplitScreenMode_eDisable)
    {
        BVDC_P_Cfc_ApplySaturationAndHue_isr(pCurInfo->sSaturation,
                                         pCurInfo->sHue,
                                         pCsc);
    }
    if(pCurInfo->stSplitScreenSetting.eColorTemp == BVDC_SplitScreenMode_eDisable)
    {
        BVDC_P_Cfc_ApplyAttenuationRGB_isr(pCurInfo->lAttenuationR,
                                           pCurInfo->lAttenuationG,
                                           pCurInfo->lAttenuationB,
                                           pCurInfo->lOffsetR,
                                           pCurInfo->lOffsetG,
                                           pCurInfo->lOffsetB,
                                           pCsc,
                                           pYCbCrToRGB,
                                           pRGBToYCbCr,
                                           pCurInfo->bUserCsc,
                                           (void *)&hWindow->aullTmpBuf[0]);
    }

    /* apply adjustment to secondary color matrix */
    hWindow->pDemoCfc->ucRulBuildCntr = hWindow->pMainCfc->ucRulBuildCntr;
    if ((pCurInfo->stSplitScreenSetting.eContrast != BVDC_SplitScreenMode_eDisable) ||
        (pCurInfo->stSplitScreenSetting.eBrightness != BVDC_SplitScreenMode_eDisable) ||
        (pCurInfo->stSplitScreenSetting.eHue != BVDC_SplitScreenMode_eDisable) ||
        (pCurInfo->stSplitScreenSetting.eColorTemp != BVDC_SplitScreenMode_eDisable))
    {
        *(hWindow->pDemoCfc) = *(hWindow->pMainCfc);
        hWindow->pDemoCfc->ucMosaicSlotIdx = 1;
        hWindow->pDemoCfc->ucRulBuildCntr = hWindow->pMainCfc->ucRulBuildCntr;
    }

    if(pCurInfo->stSplitScreenSetting.eContrast != BVDC_SplitScreenMode_eDisable)
    {
        BVDC_P_Cfc_ApplyContrast_isr(pCurInfo->sContrast, pDemoCsc);
    }
    if(pCurInfo->stSplitScreenSetting.eBrightness != BVDC_SplitScreenMode_eDisable)
    {
        BVDC_P_Cfc_ApplyBrightness_isr(pCurInfo->sBrightness, pDemoCsc);
    }
    if(pCurInfo->stSplitScreenSetting.eHue != BVDC_SplitScreenMode_eDisable)
    {
        BVDC_P_Cfc_ApplySaturationAndHue_isr(pCurInfo->sSaturation,
                                         pCurInfo->sHue,
                                         pDemoCsc);
    }
    if(pCurInfo->stSplitScreenSetting.eColorTemp != BVDC_SplitScreenMode_eDisable)
    {
        BVDC_P_Cfc_ApplyAttenuationRGB_isr(pCurInfo->lAttenuationR,
                                           pCurInfo->lAttenuationG,
                                           pCurInfo->lAttenuationB,
                                           pCurInfo->lOffsetR,
                                           pCurInfo->lOffsetG,
                                           pCurInfo->lOffsetB,
                                           pDemoCsc,
                                           pYCbCrToRGB,
                                           pRGBToYCbCr,
                                           pCurInfo->bUserCsc,
                                           (void *)&hWindow->aullTmpBuf[0]);
    }
    BDBG_LEAVE(BVDC_P_Window_CalculateCsc_isr);
    return;
}

/***************************************************************************
*
*/
BERR_Code BVDC_P_Window_SetMcvp_DeinterlaceConfiguration
    (BVDC_Window_Handle               hWindow,
     bool                             bDeinterlace,
     const BVDC_Deinterlace_Settings *pMadSettings)
{
    BVDC_P_Deinterlace_Settings *pNewMad;

    pNewMad = &hWindow->stNewInfo.stMadSettings;
    hWindow->stNewInfo.bDeinterlace = bDeinterlace;

    if(pMadSettings->eGameMode)
    {
        pNewMad->eGameMode = pMadSettings->eGameMode;
    }

    return (BERR_SUCCESS);
}


const BVDC_P_ResourceFeature* BVDC_P_Window_GetResourceFeature_isrsafe
    ( BVDC_P_WindowId                  eWindowId )
{
    BDBG_ASSERT(eWindowId < BVDC_P_WindowId_eUnknown);
    return &s_aResourceFeatureTable[eWindowId];
}

const BVDC_P_ResourceRequire* BVDC_P_Window_GetResourceRequire_isrsafe
    ( BVDC_P_WindowId                  eWindowId )
{
    BDBG_ASSERT(eWindowId < BVDC_P_WindowId_eUnknown);
    return &s_aResourceRequireTable[eWindowId];
}


#if BVDC_P_CMP_0_MOSAIC_CFCS
/***************************************************************************
 *
 */
void BVDC_P_Window_CalculateMosaicCsc_isr
    ( BVDC_Window_Handle               hWindow )
{
    BVDC_P_Window_Info *pCurInfo;
    BCFC_Csc3x4 *pCsc;
    const BCFC_Csc3x4 *pYCbCrToRGB, *pRGBToYCbCr;
    int i = 0;

    BDBG_ENTER(BVDC_P_Window_CalculateMosaicCsc_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);
    pCurInfo = &hWindow->stCurInfo;

    BDBG_MSG(("Window %d: Calculate Mosaic CSC", hWindow->eId));
    for (i = 0; i < BVDC_P_CMP_CFCS; i++)
    {
        pCsc = &hWindow->astMosaicCfc[i].stMc;

        BVDC_P_Cfc_GetCfcToApplyAttenuationRGB_isr(hWindow->pMainCfc->pColorSpaceExtOut->stColorSpace.eColorimetry, &pYCbCrToRGB, &pRGBToYCbCr);

        /* apply adjustments to mosaic color matrix */
        BVDC_P_Cfc_ApplyContrast_isr(pCurInfo->sContrast, pCsc);
        BVDC_P_Cfc_ApplyBrightness_isr(pCurInfo->sBrightness, pCsc);
        BVDC_P_Cfc_ApplySaturationAndHue_isr(pCurInfo->sSaturation,
            pCurInfo->sHue, pCsc);
        BVDC_P_Cfc_ApplyAttenuationRGB_isr(pCurInfo->lAttenuationR,
                                           pCurInfo->lAttenuationG,
                                           pCurInfo->lAttenuationB,
                                           pCurInfo->lOffsetR,
                                           pCurInfo->lOffsetG,
                                           pCurInfo->lOffsetB,
                                           pCsc,
                                           pYCbCrToRGB,
                                           pRGBToYCbCr,
                                           pCurInfo->bUserCsc,
                                           (void *)&hWindow->aullTmpBuf[0]);
    }
    BDBG_LEAVE(BVDC_P_Window_CalculateMosaicCsc_isr);
    return;
}
#endif

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_MAD_ANR)
/***************************************************************************
 * {private}
 *
 */
BERR_Code BVDC_P_Window_UpdateMadAnrCompression_isr
    ( BVDC_Window_Handle                       hWindow,
      BPXL_Format                              ePxlFormat,
      const BVDC_P_Rect                       *pRect,
      const BVDC_P_PictureNodePtr              pPicture,
      BVDC_P_Compression_Settings              *pWinCompression,
      bool                                     bWriter)
{
    bool        bSrcInterlaced;
    uint32_t    ulThroughput;
    uint32_t    ulSrcFmtRasterX, ulSrcFmtPixelClk;
    uint32_t    ulVecFmtRasterX, ulVecFmtPixelClk;
    const BFMT_VideoInfo           *pSrcFmtInfo, *pDstFmtInfo;

    BSTD_UNUSED(ePxlFormat);

    BDBG_ENTER(BVDC_P_Window_UpdateMadAnrCompression_isr);
    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);
    BDBG_OBJECT_ASSERT(hWindow->stCurInfo.hSource, BVDC_SRC);
    BDBG_OBJECT_ASSERT(hWindow->hCompositor, BVDC_CMP);

    /* Allow the source pending (rts) to happen first before setting the new
     * settings. */
    if((hWindow->stCurInfo.hSource->stCurInfo.eResumeMode) &&
       ((hWindow->stCurInfo.hSource->stNewInfo.stDirty.stBits.bInputFormat) &&
        (hWindow->stCurInfo.hSource->bUserAppliedChanges)))
    {
        goto Done;
    }

    pSrcFmtInfo = hWindow->stCurInfo.hSource->stCurInfo.pFmtInfo;
    pDstFmtInfo = hWindow->hCompositor->stCurInfo.pFmtInfo;
    bSrcInterlaced = hWindow->stCurInfo.hSource->bSrcInterlaced;

    /* TODO: Use data in window directly when it's available */
    ulSrcFmtPixelClk = pSrcFmtInfo->ulPxlFreq;
    ulVecFmtPixelClk = pDstFmtInfo->ulPxlFreq;
    ulSrcFmtRasterX  = pSrcFmtInfo->ulScanWidth;
    ulVecFmtRasterX  = pDstFmtInfo->ulScanWidth;

    if(bWriter)
    {
        /* MAD/ANR is in writer
         * SRC -> HSCL -> ANR/MAD -> SCL -> CAP -> VFD -> VEC
         * SRC is memory to memory, DST is real time
         */
        if(BVDC_P_SRC_IS_MPEG(hWindow->stCurInfo.hSource->eId))
        {
            ulThroughput = (pRect->ulWidth * (pRect->ulHeight >> bSrcInterlaced)
                * (hWindow->stCurInfo.hSource->ulVertFreq / BFMT_FREQ_FACTOR))/ 1000000;
        }
        else
        {
            ulThroughput =
                (pRect->ulWidth * (pRect->ulHeight >> bSrcInterlaced) *
                (ulSrcFmtPixelClk/BFMT_FREQ_FACTOR)) / (ulSrcFmtRasterX *
                (pPicture->pSrcOut->ulHeight >> pDstFmtInfo->bInterlaced));
        }
    }
    else
    {
        /* MAD/ANR is in reader
         * SRC -> CAP -> VFD -> HSCL -> ANR/MAD -> SCL -> VEC
         */
        ulThroughput =
            (pRect->ulWidth * (pPicture->pVfdOut->ulHeight >> bSrcInterlaced) *
            (ulVecFmtPixelClk/BFMT_FREQ_FACTOR)) / (ulVecFmtRasterX *
            (pPicture->pSclOut->ulHeight >> pDstFmtInfo->bInterlaced));
    }

    if(ulThroughput > (BVDC_P_TestFeature1_THRESHOLD/BFMT_FREQ_FACTOR))
    {
        pWinCompression->ulPredictionMode = 1;
    }
    else
    {
        pWinCompression->ulPredictionMode = 0;
    }

Done:
    BDBG_LEAVE(BVDC_P_Window_UpdateMadAnrCompression_isr);
    return BERR_SUCCESS;
}

#endif
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
/* End of file. */
