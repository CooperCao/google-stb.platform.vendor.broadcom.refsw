/******************************************************************************
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
 ******************************************************************************/

#include "bstd.h"
#include "bchp_common.h"
#ifdef BCHP_HDR_CMP_0_REG_START
#include "bchp_hdr_cmp_0.h"
#endif

#include "bvdc_common_priv.h"
#include "bvdc_buffer_priv.h"
#include "bvdc_gfxfeeder_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_display_priv.h"
#include "bvdc_vfc_priv.h"
#include "bvdc_cfc_priv.h"
#include "bchp_gfd_0.h"
#ifdef BCHP_DVI_CFC_0_REG_START
#include "bchp_dvi_cfc_0.h"
#endif

#include "bvdc_test.h"
#if BVDC_P_CFC_CHECK_MATRIX_ACCURACY
#include <stdio.h>
#endif

BDBG_MODULE(BVDC_CFC);
BDBG_FILE_MODULE(BVDC_CFC_1); /* print CFC in and out color space info */
BDBG_FILE_MODULE(BVDC_CFC_2); /* print CFC high level configure */
BDBG_FILE_MODULE(BVDC_CFC_3); /* print CFC matrix and LRAdjust value */
BDBG_FILE_MODULE(BVDC_CFC_4); /* print more implementation detail info */
BDBG_FILE_MODULE(BVDC_CFC_5); /* print flooded msgs */
BDBG_FILE_MODULE(BVDC_VFC_1); /* print VFC in and out color space info */
BDBG_FILE_MODULE(BVDC_VFC_4); /* print more implementation detail info */
BDBG_FILE_MODULE(BVDC_CFC_BG); /* print compoisitor background color adjustment msg*/
BDBG_FILE_MODULE(BVDC_CFC_BG_MAT); /* print compoisitor background color calculation matrix*/
BDBG_FILE_MODULE(BVDC_MC_ADJ); /* print matrix C adjustment info */

#define BVDC_P_CFC_VIDEO_DATA_BITS    (8)
#define BVDC_P_CFC_FIX_PI             (BMTH_FIX_SIGNED_GET_PI(BCFC_CSC_SW_CX_I_BITS, BCFC_CSC_SW_CX_F_BITS))

/* --------------------------------------------------------------------
 * MB output: XYZ -> LMS
 */
#if BVDC_P_DBV_SUPPORT
static const BCFC_Csc3x3 s_MB_OUT_XYZ_to_LMS = BCFC_MAKE_CSC_3x3
    (  0.400238,      0.707593,     -0.080806,
      -0.226298,      1.165316,      0.045701,
       0.0,           0.0,           0.918225 );

/* MC lms -> ipt xfer matrix (rows 2&3 are halved to fit in fixed point value limit s2.29) */
static const BCFC_Csc3x4 s_MC_Lms_to_Ipt = BCFC_MAKE_CSC_3x4
    (  0.399902,      0.399902,      0.199951,    0,
       2.227539,     -2.425537,      0.197998,  128,
       0.402832,      0.178589,     -0.581421,  128 );
#endif

/* --------------------------------------------------------------------
 * DVI CSC clamp
 */
#ifdef BCHP_DVI_CFC_0_CSC_MIN_MAX_Y_MIN_MASK
#define BVDC_P_CSC_CLAMP_ELM_MASK     BCHP_DVI_CFC_0_CSC_MIN_MAX_Y_MIN_MASK
#elif defined(BCHP_DVI_CSC_0_CSC_MIN_MAX_MIN_MASK)
#define BVDC_P_CSC_CLAMP_ELM_MASK     BCHP_DVI_CSC_0_CSC_MIN_MAX_MIN_MASK
#else
#define BVDC_P_CSC_CLAMP_ELM_MASK     0x00001fff
#endif

#define BVDC_P_MAKE_CSC_CLAMP_ELM(c)  ((c) & BVDC_P_CSC_CLAMP_ELM_MASK)

#define BVDC_P_MAKE_DVI_CSC_CLAMP(YMin,  YMax,  \
                                  CbMin, CbMax, \
                                  CrMin, CrMax) \
{                                     \
    BVDC_P_MAKE_CSC_CLAMP_ELM(YMin),  \
    BVDC_P_MAKE_CSC_CLAMP_ELM(YMax),  \
    BVDC_P_MAKE_CSC_CLAMP_ELM(CbMin), \
    BVDC_P_MAKE_CSC_CLAMP_ELM(CbMax), \
    BVDC_P_MAKE_CSC_CLAMP_ELM(CrMin), \
    BVDC_P_MAKE_CSC_CLAMP_ELM(CrMax)  \
}

#ifdef BCHP_DVI_CSC_0_CSC_COEFF_C01_C00
static const BVDC_P_CscClamp s_CLAMP_RGB_LIMITED_RANGE = BVDC_P_MAKE_DVI_CSC_CLAMP
    (     16, 4064,
          16, 4064,
          16, 4064 );

static const BVDC_P_CscClamp s_CLAMP_RGB_FULL_RANGE = BVDC_P_MAKE_DVI_CSC_CLAMP
    (      0, 4095,
           0, 4095,
           0, 4095 );

static const BVDC_P_CscClamp s_CLAMP_YCbCr = BVDC_P_MAKE_DVI_CSC_CLAMP
    (      0, 4095,
           0, 4095,
           0, 4095 ) ;
#endif

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
static const BVDC_P_CscClamp s_CLAMP_7271B0 = BVDC_P_MAKE_DVI_CSC_CLAMP
    (      0, 4095,
           0, 4095,
           0, 4095 );
#endif
#ifdef BCHP_DVI_CSC_0_CSC_COEFF_C01_C00
static const BVDC_P_CscClamp *const s_aCLAMP_Tbl[][2] =
{
    /* BCFC_ColorFormat_eRGB */
    {
        &s_CLAMP_RGB_LIMITED_RANGE,     /* BCFC_ColorRange_eLimited, */
        &s_CLAMP_RGB_FULL_RANGE         /* BCFC_ColorRange_eFull */
    },

    /* BCFC_ColorFormat_eYCbCr */
    {
        &s_CLAMP_YCbCr,                 /* BCFC_ColorRange_eLimited, */
        &s_CLAMP_YCbCr                  /* BCFC_ColorRange_eFull */
    },

    /* BCFC_ColorFormat_eYCbCr_CL */
    {
        &s_CLAMP_YCbCr,                 /* BCFC_ColorRange_eLimited, */
        &s_CLAMP_YCbCr                  /* BCFC_ColorRange_eFull */
    }
};
#endif

/* for cmp background color approximation for PQ and HLG output */
#include "automation/bvdc_cfcpwl_bkgclr_sdr_to_hlg.c"
#include "automation/bvdc_cfcpwl_bkgclr_sdr_to_ipt.c"

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2)
/* --------------------------------------------------------------------
 * static ram luts
 */
#include "automation/cfclut_ver3/bvdc_cfcramlut_vfc_hdr10_to_sdr.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_gfd_sdr_to_hdr10.c"

/* shared by CFCs of CFC_VER2 and CFC_VER3: ram L2NL used, no LMR used */
#include "automation/cfclut_ver3/bvdc_cfcramlut_gfd_sdr_to_hlg.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_hdr10_to_hlg_mosaic.c"

#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
#include "automation/cfclut_ver2/bvdc_cfcramlut_v0_sdr_to_hlg_v10.c"
#include "automation/cfclut_ver2/bvdc_cfcramlut_v0_hlg_to_hdr10_v10.c"
#include "automation/cfclut_ver2/bvdc_cfcramlut_v0_hlg_to_sdr_v10.c"
#elif (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
#include "automation/cfclut_ver3/bvdc_cfcramlut_gfd_sdr_to_hlg_lmrlut.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_hlg_to_hdr10.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_hlg_to_sdr.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_hdr10_to_hlg.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_hdr10_to_sdr.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_sdr_to_hlg.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_sdr_to_hdr10.c"
#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_3)
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_hdr10_to_sdr_mosaic.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_hlg_to_hdr10_mosaic.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_hlg_to_hlg_mosaic.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_hlg_to_sdr_mosaic.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_sdr_to_hdr10_mosaic.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_cmp0_sdr_to_hlg_mosaic.c"
#else
#include "automation/cfclut_ver4/bvdc_cfcramlut_cmp0_hdr10_to_hlg_lmr_mosaic.c"
#include "automation/cfclut_ver4/bvdc_cfcramlut_cmp0_hdr10_to_sdr_lmr_mosaic.c"
#include "automation/cfclut_ver4/bvdc_cfcramlut_cmp0_hlg_to_hdr10_lmr_mosaic.c"
#include "automation/cfclut_ver4/bvdc_cfcramlut_cmp0_hlg_to_sdr_lmr_mosaic.c"
#include "automation/cfclut_ver4/bvdc_cfcramlut_cmp0_sdr_to_hdr10_lmr_mosaic.c"
#include "automation/cfclut_ver4/bvdc_cfcramlut_cmp0_sdr_to_hlg_lmr_mosaic.c"
#endif
#include "automation/cfclut_ver3/bvdc_cfcramlut_vfc_hlg_to_sdr.c"
#if BVDC_P_TCH_SUPPORT
#include "automation/cfclut_ver3/bvdc_cfcramlut_v0_tp_to_hdr10_cl.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_v0_tp_to_hdr10_cl_maxlum_2000.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_v0_tp_to_hdr10_cl_maxlum_4000.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_v0_tp_to_hdr10_ncl.c"
#include "automation/cfclut_ver3/bvdc_cfcramlut_v0_tp_to_sdr.c"
#endif
#endif /* #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) */

#if 0 /* replaced by s_LRangeAdj_V0_1886_to_2084 */
/* 1886 - > 2084 */
static const BCFC_LRangeAdjTable s_LRangeAdj_1886_to_2084 = BCFC_MAKE_LR_ADJ
    (  3, /* number of points */
       /*     x,        y,            m,  e */
       0.000000, 0.000000, 0.9600000000, -5,
       1.000000, 0.030000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1 );
#endif
/* identity */
static const BCFC_LRangeAdjTable s_LRangeAdj_Identity = BCFC_MAKE_LR_ADJ
    (  2, /* number of points */
       /*     x,        y,            m,  e */
       0.000000, 0.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1,
       1.000000, 1.000000, 0.5000000000,  1 );

#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
static const BCFC_LRangeAdjTable * const s_aaLRangeAdj_in_Hlg_V0_Ver2_Tbl[] =
{
    /* input BVDC_P_ColorTF_eHlg, pre-loaded ram NL2L is used */
    &s_LRangeAdj_V0_hlg_to_1886_v10,  /* output BVDC_P_ColorTF_eBt1886 */
    &s_LRangeAdj_V0_hlg_to_2084_v10,  /* output BVDC_P_ColorTF_eBt2100Pq */
    &s_LRangeAdj_Identity             /* output BVDC_P_ColorTF_eHlg */
};

/* s_RamLutCtrl_V0_L2NL_Hlg_g12 is the same as
 * s_RamLutCtrl_CMP0_L2NL_Hlg_g12 in bvdc_cfcramlut_cmp0_hdr10_to_hlg_mosaic.c, pre-loaded ram NL2L is used */
static const BCFC_LRangeAdjTable * const s_aaLRangeAdj_out_Hlg_V0_Ver2_Tbl[] =
{
    /* output BVDC_P_ColorTF_eHlg */
    &s_LRangeAdj_V0_1886_to_hlg_v10,           /* input BVDC_P_ColorTF_eBt1886 */
    &s_LRangeAdj_CMP0_2084_to_hlg_g12_mosaic,  /* input BVDC_P_ColorTF_eBt2100Pq */
    &s_LRangeAdj_Identity                      /* input BVDC_P_ColorTF_eHlg */
};

#elif (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */
/* --------------------------------------------------------------------
 * Ram Luts
 *
 * note: since pColorSpaceExtOut->stCfg.stBits.SelTF is shared by CFCs in CMP and GFD, we need to make sure those CFCs
 * either all use ram N2NL, or all use rom N2NL.
 */
#define CFC_NL2L(module, tf)        (&s_RamLutCtrl_##module##_NL2L_##tf)
#define CFC_L2NL(module, tf)        (&s_RamLutCtrl_##module##_L2NL_##tf)
#define CFC_LMR(module, conv)       (&s_RamLutCtrl_##module##_LMR_##conv), (s_aulLmrAdj_##module##_##conv)
#define CFC_LMR_NULL                NULL, NULL
#define CFC_LRNGADJ(module, conv)   (&s_LRangeAdj_##module##_##conv)
#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_3)
#define MSC_NL2L(module, tf)        (&s_RamLutCtrl_##module##_NL2L_##tf##_mosaic)
#define MSC_LRNGADJ(module, conv)   (&s_LRangeAdj_##module##_##conv##_mosaic)
#else
#define MSC_NL2L(module, tf)        (&s_RamLutCtrl_##module##_NL2L_##tf##_lmr_mosaic)
#define MSC_LRNGADJ(module, conv)   (&s_LRangeAdj_##module##_##conv##_lmr_mosaic)
#define MSC_LMR(module, conv)       (&s_RamLutCtrl_##module##_LMR_##conv##_lmr_mosaic), (s_aulLmrAdj_##module##_##conv##_lmr_mosaic)
#endif
#define G12_NL2L(module, tf)        (&s_RamLutCtrl_##module##_NL2L_##tf##_g12_mosaic)
#define G12_L2NL(module, tf)        (&s_RamLutCtrl_##module##_L2NL_##tf##_g12)
#define G12_LRNGADJ(module, conv)   (&s_LRangeAdj_##module##_##conv##_g12_mosaic)

static const BCFC_TfConvRamLuts s_aaCmp0TfConvRamLuts_Tbl[][3] =
{
    /* input BVDC_P_ColorTF_eBt1886 */
    {{ NULL,               CFC_LMR_NULL,               &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { NULL,               CFC_LMR(CMP0,1886_to_2084), CFC_LRNGADJ(CMP0,1886_to_2084), NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { NULL,               CFC_LMR(CMP0,1886_to_hlg),  CFC_LRNGADJ(CMP0,1886_to_hlg),  CFC_L2NL(CMP0,Hlg) }  /* out BVDC_P_ColorTF_eHlg */
    },

    /* input BVDC_P_ColorTF_eBt2100Pq */
    {{ NULL,               CFC_LMR(CMP0,2084_to_1886), CFC_LRNGADJ(CMP0,2084_to_1886), NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { NULL,               CFC_LMR_NULL,               &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { NULL,               CFC_LMR(CMP0,2084_to_hlg),  CFC_LRNGADJ(CMP0,2084_to_hlg),  CFC_L2NL(CMP0,Hlg) }  /* out BVDC_P_ColorTF_eHlg */
    },

    /* input BVDC_P_ColorTF_eHlg */
    {{ CFC_NL2L(CMP0,Hlg), CFC_LMR(CMP0,hlg_to_1886),  &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { CFC_NL2L(CMP0,Hlg), CFC_LMR(CMP0,hlg_to_2084),  CFC_LRNGADJ(CMP0,hlg_to_2084),  NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { CFC_NL2L(CMP0,Hlg), CFC_LMR_NULL,               &s_LRangeAdj_Identity,          CFC_L2NL(CMP0,Hlg) }  /* out BVDC_P_ColorTF_eHlg */
    }
};

#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_3)
static const BCFC_TfConvRamLuts s_aaCmp0MosaicTfConvRamLuts_Tbl[][3] =
{
    /* input BVDC_P_ColorTF_eBt1886 */
    {{ NULL,               CFC_LMR_NULL,               &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { NULL,               CFC_LMR_NULL,               MSC_LRNGADJ(CMP0,1886_to_2084), NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { NULL,               CFC_LMR_NULL,               MSC_LRNGADJ(CMP0,1886_to_hlg),  G12_L2NL(CMP0,Hlg) }  /* out BVDC_P_ColorTF_eHlg */
    },

    /* input BVDC_P_ColorTF_eBt2100Pq */
    {{ NULL,               CFC_LMR_NULL,               MSC_LRNGADJ(CMP0,2084_to_1886), NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { NULL,               CFC_LMR_NULL,               &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { NULL,               CFC_LMR_NULL,               G12_LRNGADJ(CMP0,2084_to_hlg),  G12_L2NL(CMP0,Hlg) }  /* out BVDC_P_ColorTF_eHlg */
    },

    /* input BVDC_P_ColorTF_eHlg */
    {{ G12_NL2L(CMP0,Hlg), CFC_LMR_NULL,               MSC_LRNGADJ(CMP0,hlg_to_1886),  NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { G12_NL2L(CMP0,Hlg), CFC_LMR_NULL,               MSC_LRNGADJ(CMP0,hlg_to_2084),  NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { G12_NL2L(CMP0,Hlg), CFC_LMR_NULL,               &s_LRangeAdj_Identity,          G12_L2NL(CMP0,Hlg) }  /* out BVDC_P_ColorTF_eHlg */
    }
};

#else /* #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_3) */
static const BCFC_TfConvRamLuts s_aaCmp0MosaicTfConvRamLuts_Tbl[][3] =
{
    /* input BVDC_P_ColorTF_eBt1886 */
    {{ NULL,               CFC_LMR_NULL,               &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { NULL,               MSC_LMR(CMP0,1886_to_2084), MSC_LRNGADJ(CMP0,1886_to_2084), NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { NULL,               MSC_LMR(CMP0,1886_to_hlg),  MSC_LRNGADJ(CMP0,1886_to_hlg),  CFC_L2NL(CMP0,Hlg) }  /* out BVDC_P_ColorTF_eHlg */
    },

    /* input BVDC_P_ColorTF_eBt2100Pq */
    {{ NULL,               MSC_LMR(CMP0,2084_to_1886), MSC_LRNGADJ(CMP0,2084_to_1886), NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { NULL,               CFC_LMR_NULL,               &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { NULL,               MSC_LMR(CMP0,2084_to_hlg),  MSC_LRNGADJ(CMP0,2084_to_hlg),  CFC_L2NL(CMP0,Hlg) }  /* out BVDC_P_ColorTF_eHlg */
    },

    /* input BVDC_P_ColorTF_eHlg */
    {{ MSC_NL2L(CMP0,Hlg), MSC_LMR(CMP0,hlg_to_1886),  &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { MSC_NL2L(CMP0,Hlg), MSC_LMR(CMP0,hlg_to_2084),  MSC_LRNGADJ(CMP0,hlg_to_2084),  NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { MSC_NL2L(CMP0,Hlg), CFC_LMR_NULL,               &s_LRangeAdj_Identity,          CFC_L2NL(CMP0,Hlg) }  /* out BVDC_P_ColorTF_eHlg */
    }
};

#endif /* #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_3) */

static const BCFC_TfConvRamLuts s_aaGfdTfConvRamLuts_Tbl[][3] =
{
    /* input BVDC_P_ColorTF_eBt1886 */
    {{ NULL,               CFC_LMR_NULL,               &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { NULL,               CFC_LMR_NULL,               CFC_LRNGADJ(GFD,1886_to_2084),  NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { NULL,               CFC_LMR(GFD, 1886_to_hlg),  &s_LRangeAdj_Identity,          CFC_L2NL(GFD,Hlg) }   /* out BVDC_P_ColorTF_eHlg */
    },

    /* input BVDC_P_ColorTF_eBt2100Pq */
    {{ NULL,               CFC_LMR_NULL,               &s_LRangeAdj_VFC_2084_to_1886,  NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { NULL,               CFC_LMR_NULL,               &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { NULL,               CFC_LMR_NULL,               &s_LRangeAdj_VFC_2084_to_1886,  G12_L2NL(GFD,Hlg) }   /* out BVDC_P_ColorTF_eHlg */
    },

    /* input BVDC_P_ColorTF_eHlg */
    {{ NULL,               CFC_LMR_NULL,               &s_LRangeAdj_Identity,          NULL },               /* out BVDC_P_ColorTF_eBt1886 */
     { NULL,               CFC_LMR_NULL,               CFC_LRNGADJ(GFD,1886_to_2084),  NULL },               /* out BVDC_P_ColorTF_eBt2100Pq */
     { NULL,               CFC_LMR_NULL,               &s_LRangeAdj_Identity,          G12_L2NL(GFD,Hlg) }  /* out BVDC_P_ColorTF_eHlg */
    }
};

#if BVDC_P_TCH_SUPPORT
static const BCFC_TfConvRamLuts s_TfConvRamLutsTpTo2084 =
{
    &s_RamLutCtrl_V0_NL2L_square,  NULL,  NULL,  &s_LRangeAdj_V0_tp_to_2084, NULL
};
static const BCFC_TfConvRamLuts s_TfConvRamLutsTpTo2084_cl_maxlum_2000 =
{
    &s_RamLutCtrl_V0_NL2L_cl_maxlum_2000,  NULL,  NULL,  &s_LRangeAdj_V0_tp_to_2084, NULL
};
static const BCFC_TfConvRamLuts s_TfConvRamLutsTpTo2084_cl_maxlum_4000 =
{
    &s_RamLutCtrl_V0_NL2L_cl_maxlum_4000,  NULL,  NULL,  &s_LRangeAdj_V0_tp_to_2084, NULL
};
static const BCFC_TfConvRamLuts s_TfConvRamLutsTpTo2084_ncl =
{
    &s_RamLutCtrl_V0_NL2L_ncl,  NULL,  NULL,  &s_LRangeAdj_V0_tp_to_2084, NULL
};
static const BCFC_TfConvRamLuts s_TfConvRamLutsTpTo1886 =
{
    &s_RamLutCtrl_V0_NL2L_tp,  NULL,  NULL,  &s_LRangeAdj_Identity, NULL
};
#endif /* #if BVDC_P_CMP_SUPPORT_TP */

#endif /* #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) */

#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */

#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) || ((BVDC_P_CMP_CFC_VER > BVDC_P_CFC_VER_2) && BVDC_P_SUPPORT_VFC)
static void BVDC_P_Cfc_InitRamLut_isrsafe(
    const BCFC_RamLut             *pRamLut,
    BREG_Handle                    hRegister,
    uint32_t                       ulLutReg,
    uint32_t                       ulCtrlReg );
#endif

/* init mosaic cfcs in cmp
 */
void BVDC_P_Window_InitCfcs(
    BVDC_Window_Handle          hWindow )
{
    int jj;
    BVDC_Compositor_Handle hCompositor = hWindow->hCompositor;
    int iWinInCmp = hWindow->eId - BVDC_P_CMP_GET_V0ID(hCompositor);
    BCFC_Capability stCapability;

    stCapability.ulInts = hCompositor->stCfcCapability[iWinInCmp].ulInts;
    stCapability.stBits.bBlackBoxNLConv = stCapability.stBits.bNL2L && !stCapability.stBits.bMb;

    /* 7271 B0 CMP 0 has 8 mosaic cfcs, but cmp 1 has 6 */
    if(BVDC_CompositorId_eCompositor0 == hCompositor->eId)
    {
        /* (BVDC_P_CMP_0_TF_CONV_CFCS == 0) for 7439 B0 and older, (BVDC_P_CMP_0_TF_CONV_CFCS == BVDC_P_CMP_CFCS) for 7255, 7211 */
      #if ((BVDC_P_CMP_0_TF_CONV_CFCS == 0) || (BVDC_P_CMP_0_TF_CONV_CFCS == BVDC_P_CMP_CFCS))
        for (jj=0; jj<BVDC_P_CMP_CFCS; jj++)
        {
            hWindow->astMosaicCfc[jj].stCapability.ulInts = stCapability.ulInts;
        }
      #else
        for (jj=0; jj<BVDC_P_CMP_0_TF_CONV_CFCS; jj++)
        {
            hWindow->astMosaicCfc[jj].stCapability.ulInts = stCapability.ulInts;
        }
        stCapability.stBits.bLRngAdj = 0;
        stCapability.stBits.bLMR = 0;
        stCapability.stBits.bTpToneMapping = 0;
        stCapability.stBits.bDbvToneMapping = 0;
        stCapability.stBits.bDbvCmp = 0;
        stCapability.stBits.bRamNL2L = 0;
        stCapability.stBits.bRamL2NL = 0;
        stCapability.stBits.bRamLutScb = 0; /* ??? */
        for (jj=BVDC_P_CMP_0_TF_CONV_CFCS; jj<BVDC_P_CMP_CFCS; jj++)
        {
            hWindow->astMosaicCfc[jj].stCapability.ulInts = stCapability.ulInts;
        }
      #endif

      #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
        if(iWinInCmp == 0) /* with 7271 A0, only V0 in CMP0 has ram NL2L and L2NL */
        {
            /* ram NL2L for HLG input */
            BVDC_P_Cfc_InitRamLut_isrsafe(&s_RamLutCtrl_V0_NL2L_Hlg_g12, hCompositor->hVdc->hRegister,
                BCHP_HDR_CMP_0_V0_NL2L_TF_LUTi_ARRAY_BASE, BCHP_HDR_CMP_0_V0_NL_LUT_CTRL);
            /* ram L2NL for HLG output */
            BVDC_P_Cfc_InitRamLut_isrsafe(&s_RamLutCtrl_V0_L2NL_Hlg_g12, hCompositor->hVdc->hRegister,
                BCHP_HDR_CMP_0_V0_L2NL_TF_LUTi_ARRAY_BASE, BCHP_HDR_CMP_0_V0_LN_LUT_CTRL);
        }
      #endif
    }
    else
    {
        for (jj=0; jj<BVDC_P_CMP_CFCS; jj++)
        {
            hWindow->astMosaicCfc[jj].stCapability.ulInts = stCapability.ulInts;
        }
    }

    /* mark as not set yet */
    for (jj=0; jj<BVDC_P_CMP_CFCS; jj++)
    {
        /* note: at initial state, hWindow->astMosaicCfc[jj].stColorSpaceExtIn.stColorSpace.pMetaData = NULL */
        BCFC_InitCfc_isrsafe(&hWindow->astMosaicCfc[jj]);
        hWindow->astMosaicCfc[jj].eId = (BCFC_Id)(hWindow->eId);
        hWindow->astMosaicCfc[jj].ucMosaicSlotIdx = jj;
        hWindow->astMosaicCfc[jj].pColorSpaceExtOut = &(hCompositor->stOutColorSpaceExt);
      #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
        hWindow->astMosaicCfc[jj].pLutList = &(hCompositor->stCfcLutList);
      #endif
    }
    hCompositor->stOutColorSpaceExt.stColorSpace.eColorFmt = BCFC_ColorFormat_eInvalid;
}

/* init VFC CFC
 */
void BVDC_P_Vfc_InitCfc_isrsafe
    ( BVDC_P_Vfc_Handle   hVfc)
{
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) && (BVDC_P_SUPPORT_VFC)
    BREG_Handle hRegister = hVfc->hWindow->hCompositor->hVdc->hRegister;

    hVfc->stCfc.stCapability.stBits.bBlackBoxNLConv =
        hVfc->stCfc.stCapability.stBits.bNL2L && !hVfc->stCfc.stCapability.stBits.bMb;

    /* mark as not set yet */
    BCFC_InitCfc_isrsafe(&hVfc->stCfc);
    hVfc->stCfc.eId = hVfc->eId + BCFC_Id_eVfc0;
    /* hVfc->stCfc.pLutList = NULL !*/

    /* init ram NL2L and L2NL for HLG -> SDR */
    BVDC_P_Cfc_InitRamLut_isrsafe(&s_RamLutCtrl_VFC_NL2L_Hlg_g12, hRegister,
        BCHP_VFC_0_NL2L_TF_LUTi_ARRAY_BASE + hVfc->ulRegOffset, BCHP_VFC_0_NL2L_LUT_CTRL + hVfc->ulRegOffset);
    BVDC_P_Cfc_InitRamLut_isrsafe(&s_RamLutCtrl_VFC_L2NL_Hlg_g12, hRegister,
        BCHP_VFC_0_L2NL_TF_LUTi_ARRAY_BASE + hVfc->ulRegOffset, BCHP_VFC_0_L2NL_LUT_CTRL + hVfc->ulRegOffset);
#else
    BSTD_UNUSED(hVfc);
#endif
}

/* init GFD CFC
 */
void BVDC_P_GfxFeeder_InitCfc
    ( BVDC_P_GfxFeeder_Handle   hGfxFeeder)
{
    hGfxFeeder->stCfc.stCapability.stBits.bBlackBoxNLConv =
        hGfxFeeder->stCfc.stCapability.stBits.bNL2L && !hGfxFeeder->stCfc.stCapability.stBits.bMb;

    /* mark as not set yet */
    BCFC_InitCfc_isrsafe(&hGfxFeeder->stCfc);
    hGfxFeeder->stCfc.eId = (hGfxFeeder->eId - BAVC_SourceId_eGfx0) + BCFC_Id_eComp0_G0;
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
    hGfxFeeder->stCfc.pLutList = &(hGfxFeeder->stCfcLutList);
#endif
#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
    if(BAVC_SourceId_eGfx0 == hGfxFeeder->eId)
    {
        /* init ram L2NL for HLG output, must be combined with s_LRangeAdj_GFD_1886_to_hlg */
        BVDC_P_Cfc_InitRamLut_isrsafe(&s_RamLutCtrl_GFD_L2NL_Hlg_g12, hGfxFeeder->hRegister,
            BCHP_GFD_0_L2NL_TF_LUTi_ARRAY_BASE, BCHP_GFD_0_L2NL_LUT_CTRL);
    }
#endif
}

/* init hdmi-out VEC CFC
 */
void BVDC_P_Display_InitDviCfc(
    BVDC_Display_Handle         hDisplay )
{
    /* mark as not set yet */
    BCFC_InitCfc_isrsafe(&hDisplay->stCfc);
    hDisplay->stCfc.eId = BCFC_Id_eDisplay0;
    hDisplay->stCfc.pColorSpaceExtOut = &(hDisplay->stOutColorSpaceExt);
    hDisplay->stOutColorSpaceExt.stColorSpace.eColorFmt = BCFC_ColorFormat_eInvalid;
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
    hDisplay->stCfc.pLutList = &(hDisplay->stCfcLutList);
#endif

}

/* init mosaic colorSpace array in picture node as all invalid
 */
void BVDC_P_Window_InitVideoInputColorSpace_isr(
    BVDC_P_PictureNode         *pPicture)
{
    int ii;

    /* mark as not set */
    for (ii=0; ii<BAVC_MOSAIC_MAX; ii++)
    {
        pPicture->astMosaicColorSpace[ii].eColorFmt = BCFC_ColorFormat_eInvalid;
    }
}

/* copy input color space info from gfx surface
 */
void BVDC_P_GfxFeeder_UpdateGfxInputColorSpace_isr(
    const BVDC_P_SurfaceInfo   *pGfxSurface,
    BCFC_ColorSpaceExt         *pColorSpaceExt )
{
    BCFC_ColorSpace  stColorSpace;

    stColorSpace.eColorFmt = BPXL_IS_RGB_FORMAT(pGfxSurface->eActivePxlFmt)?
        BCFC_ColorFormat_eRGB : BCFC_ColorFormat_eYCbCr;
    stColorSpace.eColorimetry = BCFC_Colorimetry_eBt709;
    stColorSpace.eColorTF = BCFC_ColorTF_eBt1886;
    stColorSpace.eColorDepth = BCFC_ColorDepth_e8Bit; /* 565, 4444??? */
    stColorSpace.eColorRange = BCFC_ColorRange_eFull;
    stColorSpace.stMetadata.pDynamic = NULL;

    if (BCFC_COLOR_SPACE_DIFF(&stColorSpace, &pColorSpaceExt->stColorSpace))
    {
        pColorSpaceExt->stCfg.stBits.bDirty = BVDC_P_DIRTY;
        pColorSpaceExt->stColorSpace = stColorSpace;
    }
}

void BVDC_P_Feeder_ParseColorSpace_isr(
    const BAVC_MVD_Field            *pFieldData,
    bool                            *bPqNcl )
{
    BAVC_TransferCharacteristics  eTransferCharacteristics =
        (pFieldData->ePreferredTransferCharacteristics == BAVC_TransferCharacteristics_eArib_STD_B67)?
        BAVC_TransferCharacteristics_eArib_STD_B67 : pFieldData->eTransferCharacteristics;
    BCFC_ColorTF eColorTF = BCFC_AvcTransferCharacteristicsToTF_isrsafe(eTransferCharacteristics);
    BCFC_ColorFormat eColorFmt =
            (BAVC_MatrixCoefficients_eItu_R_BT_2020_CL == pFieldData->eMatrixCoefficients)?
            BCFC_ColorFormat_eYCbCr_CL : BCFC_ColorFormat_eYCbCr;
    *bPqNcl = (eColorTF == BCFC_ColorTF_eBt2100Pq && eColorFmt == BCFC_ColorFormat_eYCbCr) ? true : false;
}

/* called by BVDC_P_Window_Writer_isr, to copy input color space info from BAVC_MVD_Field or pXvdFieldData
 * to mosaic colorSpace array in picture node
 */
void BVDC_P_Window_UpdateVideoInputColorSpace_isr(
    BVDC_Window_Handle            hWindow,
    const BAVC_MVD_Field         *pMvdFieldData,
    const BAVC_VDC_HdDvi_Picture *pXvdFieldData,
    BAVC_MatrixCoefficients       eMatrixCoefficients, /* for analogue */
    BCFC_ColorSpace              *pColorSpace )
{
    BVDC_Source_Handle  hSource;

    hSource = hWindow->stCurInfo.hSource;
    BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);

    if((BVDC_P_SRC_IS_MPEG(hSource->eId)) && (pMvdFieldData))
    {
#if BVDC_P_DBV_SUPPORT || BVDC_P_TCH_SUPPORT
        BVDC_P_CfcMetaData *pMetaData = (BVDC_P_CfcMetaData *)pColorSpace->stMetadata.pDynamic;
#endif
        BAVC_TransferCharacteristics  eTransferCharacteristics =
            (pMvdFieldData->ePreferredTransferCharacteristics == BAVC_TransferCharacteristics_eArib_STD_B67)?
            BAVC_TransferCharacteristics_eArib_STD_B67 : pMvdFieldData->eTransferCharacteristics;
        pColorSpace->eColorFmt =
            (BAVC_MatrixCoefficients_eItu_R_BT_2020_CL == pMvdFieldData->eMatrixCoefficients)?
            BCFC_ColorFormat_eYCbCr_CL : BCFC_ColorFormat_eYCbCr;
        pColorSpace->eColorimetry = BCFC_AvcColorInfoToColorimetry_isrsafe(
            pMvdFieldData->eColorPrimaries, pMvdFieldData->eMatrixCoefficients,
            pMvdFieldData->eTransferCharacteristics == BAVC_TransferCharacteristics_eIec_61966_2_4);
        pColorSpace->eColorTF = BCFC_AvcTransferCharacteristicsToTF_isrsafe(eTransferCharacteristics);
        pColorSpace->eColorRange = (pMvdFieldData->eColorRange == BAVC_ColorRange_eFull)?
            BCFC_ColorRange_eFull : BCFC_ColorRange_eLimited;

        /* add more depth cases later */
        pColorSpace->eColorDepth = (BAVC_VideoBitDepth_e8Bit == pMvdFieldData->eBitDepth)?
            BCFC_ColorDepth_e8Bit : BCFC_ColorDepth_e10Bit;

        if(pMvdFieldData->stHdrMetadata.stStatic.stMasteringDisplayColorVolume.stLuminance.uiMax != 0xffffffff) {/* don't copy if invalid */
            pColorSpace->stMetadata.stStatic = pMvdFieldData->stHdrMetadata.stStatic;
        }

        /* only support DBV from decoder input for now */
#if BVDC_P_DBV_SUPPORT /* update DBV input info */
        pMetaData->stDbvInput.stHdrMetadata.eType = pMvdFieldData->stHdrMetadata.stDynamic.eType;
        if(hWindow->astMosaicCfc[0].stCapability.stBits.bDbvCmp) {
            if(pMvdFieldData->stHdrMetadata.stDynamic.eType == BAVC_HdrMetadataType_eDrpu
               /* NOTE: for backward compatible dbv stream, ignore dbv for now; TODO: bringup dbv for profile 8/9 streams */
             #if BDBV_BACKWARD_COMPATIBLE_MODE /* non-BC dbv streams had eUnknown */
               && pMvdFieldData->eTransferCharacteristics == BAVC_TransferCharacteristics_eUnknown
             #endif
               )
            {
                if(hWindow->eWriterHdrMetaDataType == BAVC_HdrMetadataType_eUnknown)
                {
                    BDBG_MODULE_MSG(BVDC_CFC_1, ("D1) Window%d input metadata type changed from %d to %d", hWindow->eId,
                        hWindow->eWriterHdrMetaDataType, pMvdFieldData->stHdrMetadata.stDynamic.eType));
                    hWindow->eWriterHdrMetaDataType = pMvdFieldData->stHdrMetadata.stDynamic.eType;
                }
                /* copy the hdr10 or dbv input parameters to pic node, so later we can pass them to dbv lib */
                BVDC_P_Dbv_UpdateVideoInputColorSpace_isr(pColorSpace, pMvdFieldData);
            }
            /* detect input toggle from dbv to normal source; also need to update hdr10 source luminance/color space
               parameters in case to support dbv output; */
            else if (pMvdFieldData->stHdrMetadata.stDynamic.eType == BAVC_HdrMetadataType_eUnknown &&
                     hWindow->eWriterHdrMetaDataType == BAVC_HdrMetadataType_eDrpu)
            {
                BDBG_MODULE_MSG(BVDC_CFC_1, ("D2) Window%d input metadata type changed from %d to %d", hWindow->eId,
                    hWindow->eWriterHdrMetaDataType, pMvdFieldData->stHdrMetadata.stDynamic.eType));
                hWindow->eWriterHdrMetaDataType = BAVC_HdrMetadataType_eUnknown;
                BVDC_P_Dbv_UpdateVideoInputColorSpace_isr(pColorSpace, pMvdFieldData);
            }
            /* else pMvdFieldData->stHdrMetadata.eType might be TCH or Unknown
             * do we need to store meta data in this case ??? or when output dbv, i.e.
             * if hCompositor->hDisplay->stCurInfo.stHdmiSettings.stSettings.bDolbyVisionEnabled ??? */
        }
#endif

        /* only support TCH from decoder input for now */
#if BVDC_P_TCH_SUPPORT /* update TCH input info */
      #if !(BVDC_P_DBV_SUPPORT)
        pMetaData->stTchInput.stHdrMetadata.eType = pMvdFieldData->stHdrMetadata.stDynamic.eType;
      #endif
        if(hWindow->astMosaicCfc[0].stCapability.stBits.bTpToneMapping) {
            if(BCFC_IS_TCH(pMvdFieldData->stHdrMetadata.stDynamic.eType))
            {
                if(hWindow->eWriterHdrMetaDataType == BAVC_HdrMetadataType_eUnknown)
                {
                    BDBG_MODULE_MSG(BVDC_CFC_1, ("T1) Window%d input metadata type changed from %d to %d", hWindow->eId,
                        hWindow->eWriterHdrMetaDataType, pMvdFieldData->stHdrMetadata.stDynamic.eType));
                    hWindow->eWriterHdrMetaDataType = pMvdFieldData->stHdrMetadata.stDynamic.eType;
                }
                BVDC_P_Tch_UpdateVideoInputColorSpace_isr(hWindow->hCompositor, pColorSpace, pMvdFieldData);
            } else if (pMvdFieldData->stHdrMetadata.stDynamic.eType == BAVC_HdrMetadataType_eUnknown) {
                if(BCFC_IS_TCH(hWindow->eWriterHdrMetaDataType))
                {
                    BDBG_MODULE_MSG(BVDC_CFC_1, ("T2) Window%d input metadata type changed from %d to %d", hWindow->eId,
                        hWindow->eWriterHdrMetaDataType, pMvdFieldData->stHdrMetadata.stDynamic.eType));
                    hWindow->eWriterHdrMetaDataType = BAVC_HdrMetadataType_eUnknown;
                }
            }
        }
#endif
    }
    else if((BVDC_P_SRC_IS_HDDVI(hSource->eId)) && (pXvdFieldData))
    {
        switch (pXvdFieldData->eEotf)
        {
        case BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084:   /* HDR10, i.e. PQ */
            pColorSpace->eColorTF = BCFC_ColorTF_eBt2100Pq;
            break;
        case BAVC_HDMI_DRM_EOTF_eHLG:             /* HLG */
            pColorSpace->eColorTF = BCFC_ColorTF_eHlg;
            break;
        case BAVC_HDMI_DRM_EOTF_eSDR:
        case BAVC_HDMI_DRM_EOTF_eHDR:             /* obsolete HDR gamma */
        default:
            pColorSpace->eColorTF = BCFC_ColorTF_eBt1886;
            break;
        }
        pColorSpace->eColorFmt =
            (BAVC_MatrixCoefficients_eItu_R_BT_2020_CL == pXvdFieldData->eMatrixCoefficients)?
            BCFC_ColorFormat_eYCbCr_CL : BCFC_ColorFormat_eYCbCr;
        pColorSpace->eColorimetry = BCFC_AvcColorInfoToColorimetry_isrsafe(
            pXvdFieldData->eColorPrimaries, pXvdFieldData->eMatrixCoefficients,
            pXvdFieldData->eTransferCharacteristics == BAVC_TransferCharacteristics_eIec_61966_2_4);

        /* TODO: Only support 8bit and 10bit for now, add more depth cases later */
        pColorSpace->eColorDepth = (BAVC_HDMI_BitsPerPixel_e24bit == pXvdFieldData->eColorDepth)?
            BCFC_ColorDepth_e8Bit : BCFC_ColorDepth_e10Bit;

        pColorSpace->eColorRange =
            ((BAVC_CscMode_e601RgbFullRange == pXvdFieldData->eCscMode) ||
             (BAVC_CscMode_e709RgbFullRange == pXvdFieldData->eCscMode) ||
             (BAVC_CscMode_e2020RgbFullRange == pXvdFieldData->eCscMode)) ?
            BCFC_ColorRange_eFull : BCFC_ColorRange_eLimited;

        if(pXvdFieldData->stHdrMetadata.stStatic.stMasteringDisplayColorVolume.stLuminance.uiMax != 0xffffffff) {/* don't copy if invalid */
            pColorSpace->stMetadata.stStatic = pXvdFieldData->stHdrMetadata.stStatic;
        }
    }
    else
    {
        /* analogue ... */
        pColorSpace->eColorFmt = BCFC_ColorFormat_eYCbCr;
        pColorSpace->eColorimetry = BCFC_AvcColorInfoToColorimetry_isrsafe(BAVC_ColorPrimaries_eUnknown, eMatrixCoefficients, false);
        pColorSpace->eColorTF = BCFC_ColorTF_eBt1886;
        pColorSpace->eColorDepth = BCFC_ColorDepth_e8Bit;
        pColorSpace->eColorRange = BCFC_ColorRange_eLimited;
    }

#if (BVDC_P_SUPPORT_VFC)
    if(BVDC_P_VNET_USED_VFC(hWindow->stVnetMode))
    {
        BVDC_P_Vfc_Handle hVfc = hWindow->stCurResource.hVfc;

        /* either input or output csc change would trigger re-evaluate of VFC's CFC */
        hVfc->bCfcDirty |= hWindow->stCurInfo.stDirty.stBits.bCscAdjust;
        hVfc->bCfcDirty |= hWindow->hCompositor->stCurInfo.stDirty.stBits.bOutColorSpace;
        /* VFC could convert input color space to output color space, so update pPicture node here; */
        if (BCFC_COLOR_SPACE_DIFF(&hVfc->stCfc.stColorSpaceExtIn.stColorSpace, pColorSpace))
        {
            /* input csc change triggers re-evaluate CFC */
            hVfc->bCfcDirty = true;
            hVfc->stCfc.stColorSpaceExtIn.stCfg.stBits.bDirty = BVDC_P_DIRTY;
            BCFC_CopyColorSpace_isrsafe(&hVfc->stCfc.stColorSpaceExtIn.stColorSpace, pColorSpace);
        }
        BCFC_CopyColorSpace_isrsafe(pColorSpace, &hWindow->hCompositor->stOutColorSpaceExt.stColorSpace);
    }
#endif
}

static const BCFC_ColorTF s_aAvcEotf_to_TF[] =
{
    BCFC_ColorTF_eBt1886,                  /* BAVC_HDMI_DRM_EOTF_eSDR */
    BCFC_ColorTF_eBt2100Pq,                /* BAVC_HDMI_DRM_EOTF_eHDR */
    BCFC_ColorTF_eBt2100Pq,                /* BAVC_HDMI_DRM_EOTF_eSMPTE_ST_2084 */
    BCFC_ColorTF_eHlg                      /* BAVC_HDMI_DRM_EOTF_eHLG */
};

/* configure hDisplay->stOutColorSpaceExt.stColorSpace and
 * hDisplay->stCfc.stColorSpaceExtIn.stColorSpace for hdmi out
 *
 * note: CMP always output limited range YCbCr
 *
 */
static bool BVDC_P_Display_UpdateCfcColorSpaces_isr(
    BVDC_Display_Handle         hDisplay,
    BCFC_ColorSpace            *pCmpOutColorSpace )
{
    BCFC_ColorSpace  stColorSpace;
    BCFC_ColorSpace  *pDspColorSpace = &hDisplay->stOutColorSpaceExt.stColorSpace;
    BVDC_P_Display_HdmiSettings *pHdmiSettings = &hDisplay->stCurInfo.stHdmiSettings;
    bool bColorSpacesChanged = false;

    /* display (vec) input color space or cmp output color space changed */
    if (BCFC_COLOR_SPACE_DIFF(pCmpOutColorSpace, &hDisplay->stCfc.stColorSpaceExtIn.stColorSpace) ||
        hDisplay->hCompositor->stOutColorSpaceExt.stCfg.stBits.bDirty)
    {
        hDisplay->stCfc.stColorSpaceExtIn.stColorSpace = *pCmpOutColorSpace;
        hDisplay->stCfc.stColorSpaceExtIn.stCfg.stBits.bDirty = BVDC_P_DIRTY;
        bColorSpacesChanged = true;
    }

    /* display (vec) output color space */
    stColorSpace.eColorFmt =
        (BAVC_Colorspace_eRGB == pHdmiSettings->stSettings.eColorComponent)? BCFC_ColorFormat_eRGB :
        (BAVC_MatrixCoefficients_eHdmi_RGB == pHdmiSettings->stSettings.eMatrixCoeffs ||
         BAVC_MatrixCoefficients_eDvi_Full_Range_RGB == pHdmiSettings->stSettings.eMatrixCoeffs) ? BCFC_ColorFormat_eRGB :
        (BAVC_MatrixCoefficients_eItu_R_BT_2020_CL == pHdmiSettings->stSettings.eMatrixCoeffs)?
        BCFC_ColorFormat_eYCbCr_CL : BCFC_ColorFormat_eYCbCr;
    stColorSpace.eColorRange =
        (BAVC_ColorRange_eLimited == pHdmiSettings->stSettings.eColorRange)? BCFC_ColorRange_eLimited :
        (BAVC_ColorRange_eFull == pHdmiSettings->stSettings.eColorRange)? BCFC_ColorRange_eFull :
        ((BAVC_MatrixCoefficients_eDvi_Full_Range_RGB == pHdmiSettings->stSettings.eMatrixCoeffs) ||
         (BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr == pHdmiSettings->stSettings.eMatrixCoeffs)) ?
        BCFC_ColorRange_eFull : BCFC_ColorRange_eLimited;
    stColorSpace.eColorimetry = pCmpOutColorSpace->eColorimetry;
    stColorSpace.eColorTF = pCmpOutColorSpace->eColorTF;
    stColorSpace.eColorDepth = (BCFC_ColorDepth)pHdmiSettings->eHdmiColorDepth;
    stColorSpace.stMetadata.pDynamic = NULL;
    if (BCFC_COLOR_SPACE_DIFF(&stColorSpace, pDspColorSpace) || hDisplay->stCurInfo.stDirty.stBits.bHdmiSettings)
    {
        BDBG_MODULE_MSG(BVDC_CFC_1,("Display%d Output colorSpace changed:", hDisplay->eId));
      #if (BDBG_DEBUG_BUILD)
        BDBG_MODULE_MSG(BVDC_CFC_1,("   ColorFmt    %9s ===> %-9s", BCFC_GetColorFormatName_isrsafe(pDspColorSpace->eColorFmt),    BCFC_GetColorFormatName_isrsafe(stColorSpace.eColorFmt)));
        BDBG_MODULE_MSG(BVDC_CFC_1,("   Colorimetry %9s ===> %-9s", BCFC_GetColorimetryName_isrsafe(pDspColorSpace->eColorimetry), BCFC_GetColorimetryName_isrsafe(stColorSpace.eColorimetry)));
        BDBG_MODULE_MSG(BVDC_CFC_1,("   ColorRange  %9s ===> %-9s", BCFC_GetColorRangeName_isrsafe(pDspColorSpace->eColorRange),   BCFC_GetColorRangeName_isrsafe(stColorSpace.eColorRange)));
        BDBG_MODULE_MSG(BVDC_CFC_1,("   ColorTF     %9s ===> %-9s", BCFC_GetColorTfName_isrsafe(pDspColorSpace->eColorTF),         BCFC_GetColorTfName_isrsafe(stColorSpace.eColorTF)));
        BDBG_MODULE_MSG(BVDC_CFC_1,("   ColorDepth  %9s ===> %-9s", BCFC_GetColorDepthName_isrsafe(pDspColorSpace->eColorDepth),   BCFC_GetColorDepthName_isrsafe(stColorSpace.eColorDepth)));
      #endif
        BDBG_MODULE_MSG(BVDC_CFC_1,("   output DBV  %9d", pHdmiSettings->stSettings.bDolbyVisionEnabled));
        hDisplay->stOutColorSpaceExt.stCfg.stBits.bDirty = BVDC_P_DIRTY;
        *pDspColorSpace = stColorSpace;
        bColorSpacesChanged = true;
      #if BVDC_P_DBV_SUPPORT
        if(hDisplay->hCompositor->pstDbv) {
            BVDC_P_Compositor_DbvUpdateOutputInfo_isr(hDisplay->hCompositor);
        }
      #endif
    }

    if (bColorSpacesChanged)
    {
        hDisplay->stCurInfo.stDirty.stBits.bHdmiCsc = BVDC_P_DIRTY;
    }

    return bColorSpacesChanged;
}

/* calculate PWL functions (need 24-bit fraction to be precise) */
/* Input: RGB value x in U1.24;
   Output: RGB value y in U1.24;
   PWL slopeM: S0.24; slopeE: S4.0;
 */
static uint32_t BVDC_P_NlPwl_Cal_isrsafe(uint32_t x, const BVDC_P_NL_PwlSegments *pwl) {
    unsigned i;
    uint32_t y=x, signBit = BMTH_P_FIX_SIGN_BIT(BVDC_P_PWL_SLP_E_I_BITS, BVDC_P_PWL_SLP_E_F_BITS);
    int64_t val;
    BDBG_ASSERT(pwl->num <= BCFC_LR_ADJ_PTS);
    if (x >= pwl->point[pwl->num-1].x) {
        y = pwl->point[pwl->num-1].y;
        BDBG_MODULE_MSG(BVDC_CFC_BG, ("pwl point[%u] = (%#x, %#x)",
            pwl->num-1, pwl->point[pwl->num-1].x, pwl->point[pwl->num-1].y));
    } else {
        for (i=1; i<pwl->num; i++) {
            if (x <= pwl->point[i].x) {
                if (x == pwl->point[i-1].x)
                    y = pwl->point[i-1].y;
                else {
                    /* xout = (xin-pwl[i-1].x)*(pwl[i].slope_frac) + pwl[i-1].y; */
                    val = BMTH_FIX_SIGNED_MUL_64_isrsafe((int64_t)x - (int64_t)pwl->point[i-1].x, (int64_t)pwl->slope[i].man,
                            BVDC_P_PWL_XY_F_BITS,
                            BVDC_P_PWL_SLP_M_F_BITS,
                            BVDC_P_PWL_XY_F_BITS);
                    y = pwl->point[i-1].y +
                        ((pwl->slope[i].exp & signBit) ? (uint32_t)(val >> ((signBit<<1) - pwl->slope[i].exp))
                            : (uint32_t)(val << pwl->slope[i].exp));
                    /*BDBG_MODULE_MSG(BVDC_CFC_BG, ("pwl slope[%u]=(%#x, %#x), val="BDBG_UINT64_FMT,
                        i, pwl->slope[i].man, pwl->slope[i].exp, BDBG_UINT64_ARG(val)));*/
                }
                /*BDBG_MODULE_MSG(BVDC_CFC_BG, ("pwl point[%u]=(%#x, %#x), point[%u]=(%#x, %#x)",
                    i-1, pwl->point[i-1].x, pwl->point[i-1].y, i, pwl->point[i].x, pwl->point[i].y));*/
                break;
            }
        }
    }
    BDBG_MODULE_MSG(BVDC_CFC_BG, (" -> (%#x, %#x)", x, y));
    return y;
}

/* non-linear RGB (U1.24)  to linear RGB(U1.24) (709 or 2020) for nlpwl (used for bkClr computation) */
static uint32_t BVDC_P_Nl_to_Ln_NlPwl_isrsafe(uint32_t in, BCFC_ColorTF eTf) {
    uint32_t out;

    in = (in > BMTH_FIX_SIGNED_FTOFIX(1.0, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS)) ?
            BMTH_FIX_SIGNED_FTOFIX(1.0, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS) : in;
    out = BMTH_FIX_SIGNED_MUL_isrsafe(
        /* 200/1000 vs 200/10000 nits */
        (BCFC_ColorTF_eHlg==eTf) ?
            BMTH_FIX_SIGNED_FTOFIX(0.2, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS) :
            BMTH_FIX_SIGNED_FTOFIX(0.02, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS),
        BVDC_P_NlPwl_Cal_isrsafe(in, &s_Nl2l_Pwl_1886),
        BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS,
        BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS,
        BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS);
    out = (out > BMTH_FIX_SIGNED_FTOFIX(1.0, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS)) ?
            BMTH_FIX_SIGNED_FTOFIX(1.0, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS) : out;

    return (uint32_t)out;
}

/* linear RGB(U1.24)  to nonlinear RGB(U1.24) (709 or 2020) for nlpwl (used for bkClr computation) */
static uint32_t BVDC_P_Ln_to_Nl_NlPwl_isrsafe(uint32_t in, BCFC_ColorTF eTf) {
    uint32_t out;

    /* clamp to 0-1 for non xvycc mode, no xvycc nlpwl for now
       only PQ l2nl is applied for now, sel_l2nl = PQ */
    in = (in > BMTH_FIX_SIGNED_FTOFIX(1.0, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS)) ?
            BMTH_FIX_SIGNED_FTOFIX(1.0, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS) : in;
    out = BVDC_P_NlPwl_Cal_isrsafe(in, (BCFC_ColorTF_eHlg==eTf)? &s_L2nl_Pwl_hlg : &s_L2nl_Pwl_2084);
    out = (out > BMTH_FIX_SIGNED_FTOFIX(1.0, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS)) ?
            BMTH_FIX_SIGNED_FTOFIX(1.0, BVDC_P_PWL_XY_I_BITS, BVDC_P_PWL_XY_F_BITS) : out;
    return (uint32_t)out;
}

uint32_t BVDC_P_Compositor_Update_Canvas_Background_isrsafe
    ( BVDC_Compositor_Handle           hCompositor,
      uint8_t                          ucRed,
      uint8_t                          ucGreen,
      uint8_t                          ucBlue)
{
    int64_t llR, llG, llB, ll0, ll1, ll2, llRoundOff;
    uint32_t ul0, ul1, ul2, ulBgColorYCrCb=0;
    int64_t llYOffset, llCbOffset, llCrOffset;
    const BCFC_Csc3x3 *pMbIn, *pMbOut;
    BCFC_Csc3x3 stMb;
    const BCFC_Csc3x4 *pMc;
    BCFC_ColorSpace *pColorSpaceOut;
    BCFC_ColorFormat eColorFmt;

    BDBG_ENTER(BVDC_P_Compositor_Update_Canvas_Background_isr);
    /* check parameters */
    BDBG_OBJECT_ASSERT(hCompositor, BVDC_CMP);

    pColorSpaceOut = &(hCompositor->stOutColorSpaceExt.stColorSpace);
    eColorFmt = BVDC_P_CFC_NEED_BLEND_MATRIX(hCompositor)?
        BCFC_ColorFormat_eRGB : BCFC_ColorFormat_eYCbCr;

#if BVDC_P_DBV_SUPPORT
    if(BVDC_P_CMP_DBV_MODE(hCompositor) &&
       (!hCompositor->ahWindow[BVDC_P_WindowId_eComp0_V0] ||
        !hCompositor->ahWindow[BVDC_P_WindowId_eComp0_V0]->astMosaicCfc[0].stForceCfg.stBits.bDisableDolby))
    {
        /* without gfx blending or with conformance test, use IPT; otherwise, use LMS */
        eColorFmt = (0 == hCompositor->ulActiveGfxWindow ||
            hCompositor->hDisplay->stCurInfo.stHdmiSettings.stSettings.bBlendInIpt)?
            BCFC_ColorFormat_eICtCp : BCFC_ColorFormat_eLMS;

        /* make sure hdmi metadata ram is updated twice to populate double-buffered hdmi metadata ram */
        hCompositor->hDisplay->stCfc.ucRulBuildCntr = BVDC_P_RUL_UPDATE_THRESHOLD;
        hCompositor->hDisplay->stCurInfo.stDirty.stBits.bInputCS = BVDC_P_DIRTY;
    }
#endif
    BDBG_MODULE_MSG(BVDC_CFC_BG,("cmp[%d] =>Out %d", hCompositor->eId, pColorSpaceOut->eColorimetry));

    /*Ma is identity matrix, so ignore here */
    /* Get nlRGB(U8.0 or U0.8)->lnRGB(U0.24 or U24.0) components */
    if(BVDC_P_CMP_DBV_MODE(hCompositor) ||
       (BCFC_ColorTF_eBt1886 != hCompositor->stOutColorSpaceExt.stColorSpace.eColorTF)) {
           BCFC_ColorTF eTf = BVDC_P_CMP_DBV_MODE(hCompositor)? BCFC_ColorTF_eBt2100Pq :
                hCompositor->stOutColorSpaceExt.stColorSpace.eColorTF;
        llR = (int64_t) BVDC_P_Nl_to_Ln_NlPwl_isrsafe(ucRed   << (BVDC_P_PWL_XY_F_BITS - 8), eTf);
        llG = (int64_t) BVDC_P_Nl_to_Ln_NlPwl_isrsafe(ucGreen << (BVDC_P_PWL_XY_F_BITS - 8), eTf);
        llB = (int64_t) BVDC_P_Nl_to_Ln_NlPwl_isrsafe(ucBlue  << (BVDC_P_PWL_XY_F_BITS - 8), eTf);
    } else {
        llR = (int64_t) (ucRed   << (BVDC_P_PWL_XY_F_BITS - 8));
        llG = (int64_t) (ucGreen << (BVDC_P_PWL_XY_F_BITS - 8));
        llB = (int64_t) (ucBlue  << (BVDC_P_PWL_XY_F_BITS - 8));
    }
    BDBG_MODULE_MSG(BVDC_CFC_BG,(" [R] " BDBG_UINT64_FMT " [G] " BDBG_UINT64_FMT " [B] " BDBG_UINT64_FMT,
        BDBG_UINT64_ARG(llR), BDBG_UINT64_ARG(llG), BDBG_UINT64_ARG(llB)));

    /*Mb = Mbout(XYZ->RGB) * Mbin(RGB->XYZ) */
    pMbIn = BCFC_GetCsc3x3_MbIn_isrsafe(BCFC_Colorimetry_eBt709);
#if BVDC_P_DBV_SUPPORT
    pMbOut = (BCFC_ColorFormat_eYCbCr == eColorFmt || BCFC_ColorFormat_eRGB == eColorFmt)?
        BCFC_GetCsc3x3_MbOut_isrsafe(pColorSpaceOut->eColorimetry) : &s_MB_OUT_XYZ_to_LMS;
#else
    pMbOut = BCFC_GetCsc3x3_MbOut_isrsafe(pColorSpaceOut->eColorimetry);
#endif

    BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("MbIn[0][0] %8x MbIn[0][1] %8x MbIn[0][2] %8x", pMbIn->m[0][0], pMbIn->m[0][1], pMbIn->m[0][2]));
    BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("MbIn[1][0] %8x MbIn[1][1] %8x MbIn[1][2] %8x", pMbIn->m[1][0], pMbIn->m[1][1], pMbIn->m[1][2]));
    BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("MbIn[2][0] %8x MbIn[2][1] %8x MbIn[2][2] %8x", pMbIn->m[2][0], pMbIn->m[2][1], pMbIn->m[2][2]));
    BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("MbOut[0][0] %8x MbOut[0][1] %8x MbOut[0][2] %8x", pMbOut->m[0][0], pMbOut->m[0][1], pMbOut->m[0][2]));
    BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("MbOut[1][0] %8x MbOut[1][1] %8x MbOut[1][2] %8x", pMbOut->m[1][0], pMbOut->m[1][1], pMbOut->m[1][2]));
    BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("MbOut[2][0] %8x MbOut[2][1] %8x MbOut[2][2] %8x", pMbOut->m[2][0], pMbOut->m[2][1], pMbOut->m[2][2]));

    /* B = Bout  * Bin */
    BCFC_Csc_Mult_isrsafe((const int*)&pMbOut->m[0][0], 3, (const int*)&pMbIn->m[0][0], 3, &(stMb.m[0][0]));
    BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("Mb[0][0] %8x Mb[0][1] %8x Mb[0][2] %8x", stMb.m[0][0], stMb.m[0][1], stMb.m[0][2]));
    BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("Mb[1][0] %8x Mb[1][1] %8x Mb[1][2] %8x", stMb.m[1][0], stMb.m[1][1], stMb.m[1][2]));
    BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("Mb[2][0] %8x Mb[2][1] %8x Mb[2][2] %8x", stMb.m[2][0], stMb.m[2][1], stMb.m[2][2]));

    /* MB(S2.29) * lnRGB(U24.0) -> lnLMS(S26.29) */
    ll0 = llR * ((int64_t)stMb.m[0][0]) + llG *((int64_t)stMb.m[0][1]) + llB *((int64_t)stMb.m[0][2]);
    ll1 = llR * ((int64_t)stMb.m[1][0]) + llG *((int64_t)stMb.m[1][1]) + llB *((int64_t)stMb.m[1][2]);
    ll2 = llR * ((int64_t)stMb.m[2][0]) + llG *((int64_t)stMb.m[2][1]) + llB *((int64_t)stMb.m[2][2]);
    BDBG_MODULE_MSG(BVDC_CFC_BG,("Ln [0] " BDBG_UINT64_FMT " [1] " BDBG_UINT64_FMT " [2] " BDBG_UINT64_FMT,
        BDBG_UINT64_ARG(ll0), BDBG_UINT64_ARG(ll1), BDBG_UINT64_ARG(ll2)));

    /* shift truncate fraction part -> lnLMS(U32.0 or U8.24) */
    ul0 = (uint32_t)(BVDC_P_MAX(ll0 >> BCFC_CSC_SW_CX_F_BITS, 0));
    ul1 = (uint32_t)(BVDC_P_MAX(ll1 >> BCFC_CSC_SW_CX_F_BITS, 0));
    ul2 = (uint32_t)(BVDC_P_MAX(ll2 >> BCFC_CSC_SW_CX_F_BITS, 0));
    BDBG_MODULE_MSG(BVDC_CFC_BG,("LN [0] %x [1] %x [2] %x", ul0, ul1, ul2));

    /* lnLMS(U.24) -> nlLMS(U0.8 or U8.0) */
    if(BVDC_P_CMP_DBV_MODE(hCompositor) ||
       (BCFC_ColorTF_eBt1886 != hCompositor->stOutColorSpaceExt.stColorSpace.eColorTF)) {
       BCFC_ColorTF eTf = BVDC_P_CMP_DBV_MODE(hCompositor)? BCFC_ColorTF_eBt2100Pq :
            hCompositor->stOutColorSpaceExt.stColorSpace.eColorTF;
        ul0 = BVDC_P_Ln_to_Nl_NlPwl_isrsafe(ul0, eTf);
        ul1 = BVDC_P_Ln_to_Nl_NlPwl_isrsafe(ul1, eTf);
        ul2 = BVDC_P_Ln_to_Nl_NlPwl_isrsafe(ul2, eTf);
    }
    ul0 >>= (BVDC_P_PWL_XY_F_BITS - 8);
    ul1 >>= (BVDC_P_PWL_XY_F_BITS - 8);
    ul2 >>= (BVDC_P_PWL_XY_F_BITS - 8);
    BDBG_MODULE_MSG(BVDC_CFC_BG,("NL [0] %x [1] %x [2] %x", ul0, ul1, ul2));

    /* Mc step */
    if(BCFC_ColorFormat_eYCbCr == eColorFmt || BCFC_ColorFormat_eICtCp == eColorFmt) {
#if BVDC_P_DBV_SUPPORT
        if(BCFC_ColorFormat_eICtCp == eColorFmt) {
            pMc = &s_MC_Lms_to_Ipt;
            BDBG_MODULE_MSG(BVDC_CFC_BG,("Bkgclr in IPT"));
        }
        else
#endif
        {
            pMc = BCFC_GetCsc3x4_Mc_isrsafe(pColorSpaceOut->eColorimetry);
            BDBG_MODULE_MSG(BVDC_CFC_BG,("Bkgclr in YCrCb"));
        }
        BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("Mc[0][0] %8x Mc[0][1] %8x Mc[0][2] %8x Mc[0][3] %8x", pMc->m[0][0], pMc->m[0][1], pMc->m[0][2], pMc->m[0][3]));
        BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("Mc[1][0] %8x Mc[1][1] %8x Mc[1][2] %8x Mc[1][3] %8x", pMc->m[1][0], pMc->m[1][1], pMc->m[1][2], pMc->m[1][3]));
        BDBG_MODULE_MSG(BVDC_CFC_BG_MAT,("Mc[2][0] %8x Mc[2][1] %8x Mc[2][2] %8x Mc[2][3] %8x", pMc->m[2][0], pMc->m[2][1], pMc->m[2][2], pMc->m[2][3]));

        /* Get ARGB components */
        llR = (int64_t) ul0;
        llG = (int64_t) ul1;
        llB = (int64_t) ul2;

        /* possible error place for matrix multiplication*/
        ll0 = llR * ((int64_t)pMc->m[0][0]) + llG *((int64_t)pMc->m[0][1]) + llB *((int64_t)pMc->m[0][2]);
        ll1 = llR * ((int64_t)pMc->m[1][0]) + llG *((int64_t)pMc->m[1][1]) + llB *((int64_t)pMc->m[1][2]);
        ll2 = llR * ((int64_t)pMc->m[2][0]) + llG *((int64_t)pMc->m[2][1]) + llB *((int64_t)pMc->m[2][2]);

        llYOffset  = ((int64_t)pMc->m[0][3])<<BCFC_CSC_SW_F_CX_CO_DIFF_BITS;
        llCbOffset = ((int64_t)pMc->m[1][3])<<BCFC_CSC_SW_F_CX_CO_DIFF_BITS;
        llCrOffset = ((int64_t)pMc->m[2][3])<<BCFC_CSC_SW_F_CX_CO_DIFF_BITS;
        llRoundOff = (1<<(BCFC_CSC_SW_CX_F_BITS-1));

        BDBG_MODULE_MSG(BVDC_CFC_BG,("0) [0] " BDBG_UINT64_FMT " [1] " BDBG_UINT64_FMT " [2] " BDBG_UINT64_FMT "  Roundoff " BDBG_UINT64_FMT,
            BDBG_UINT64_ARG(ll0), BDBG_UINT64_ARG(ll1), BDBG_UINT64_ARG(ll2), BDBG_UINT64_ARG(llRoundOff)));
        BDBG_MODULE_MSG(BVDC_CFC_BG,("0) OFFSET[0] " BDBG_UINT64_FMT " [1] " BDBG_UINT64_FMT " [2] " BDBG_UINT64_FMT,
            BDBG_UINT64_ARG(llYOffset), BDBG_UINT64_ARG(llCbOffset), BDBG_UINT64_ARG(llCrOffset)));

        ll0 += llYOffset  + llRoundOff;
        ll1 += llCbOffset + llRoundOff;
        ll2 += llCrOffset + llRoundOff;
        BDBG_MODULE_MSG(BVDC_CFC_BG,("1) [0] " BDBG_UINT64_FMT " [1] " BDBG_UINT64_FMT " [2] " BDBG_UINT64_FMT "  Roundoff " BDBG_UINT64_FMT,
            BDBG_UINT64_ARG(ll0), BDBG_UINT64_ARG(ll1), BDBG_UINT64_ARG(ll2), BDBG_UINT64_ARG(llRoundOff)));

        ll0 >>= BCFC_CSC_SW_CX_F_BITS;
        ll1 >>= BCFC_CSC_SW_CX_F_BITS;
        ll2 >>= BCFC_CSC_SW_CX_F_BITS;
        BDBG_MODULE_MSG(BVDC_CFC_BG,("2) [0] " BDBG_UINT64_FMT " [1] " BDBG_UINT64_FMT " [2] " BDBG_UINT64_FMT,
        BDBG_UINT64_ARG(ll0), BDBG_UINT64_ARG(ll1), BDBG_UINT64_ARG(ll2)));

        ul0 = (uint32_t)(BVDC_P_MAX(ll0 , 0));
        ul1 = (uint32_t)(BVDC_P_MAX(ll1, 0));
        ul2 = (uint32_t)(BVDC_P_MAX(ll2, 0));
        BDBG_MODULE_MSG(BVDC_CFC_BG,("ch [0] %x [1] %x [2] %x", ul0, ul1, ul2));

        /* reverse 0.85 adj applied to non-BT2020 limited range YCbCr */
        if((BCFC_ColorTF_eBt1886 == hCompositor->stOutColorSpaceExt.stColorSpace.eColorTF) &&
            BCFC_Colorimetry_eBt2020 == pColorSpaceOut->eColorimetry) {
            pMc = BCFC_GetCsc3x4_Cr0p85Adj_NonBT2020_to_BT2020_isrsafe();
            /* Adj(S0.29) * RGB(U8.0) -> nlRGB(S8.29) */
            ll0 = ul0 * ((int64_t)pMc->m[0][0]) + ul1 *((int64_t)pMc->m[0][1]) + ul2 *((int64_t)pMc->m[0][2]);
            ll1 = ul0 * ((int64_t)pMc->m[1][0]) + ul1 *((int64_t)pMc->m[1][1]) + ul2 *((int64_t)pMc->m[1][2]);
            ll2 = ul0 * ((int64_t)pMc->m[2][0]) + ul1 *((int64_t)pMc->m[2][1]) + ul2 *((int64_t)pMc->m[2][2]);
            BDBG_MODULE_MSG(BVDC_CFC_BG,("NL2020 [0] " BDBG_UINT64_FMT " [1] " BDBG_UINT64_FMT " [2] " BDBG_UINT64_FMT,
                BDBG_UINT64_ARG(ll0), BDBG_UINT64_ARG(ll1), BDBG_UINT64_ARG(ll2)));

            llYOffset  = ((int64_t)pMc->m[0][3])<<BCFC_CSC_SW_F_CX_CO_DIFF_BITS;
            llCbOffset = ((int64_t)pMc->m[1][3])<<BCFC_CSC_SW_F_CX_CO_DIFF_BITS;
            llCrOffset = ((int64_t)pMc->m[2][3])<<BCFC_CSC_SW_F_CX_CO_DIFF_BITS;
            llRoundOff = (1<<(BCFC_CSC_SW_CX_F_BITS-1));

            ll0 += llYOffset  + llRoundOff;
            ll1 += llCbOffset + llRoundOff;
            ll2 += llCrOffset + llRoundOff;

            /* shift truncate fraction part -> nlRGB(U8.0) */
            ul0 = (uint32_t)(BVDC_P_MAX(ll0 >> BCFC_CSC_SW_CX_F_BITS, 0));
            ul1 = (uint32_t)(BVDC_P_MAX(ll1 >> BCFC_CSC_SW_CX_F_BITS, 0));
            ul2 = (uint32_t)(BVDC_P_MAX(ll2 >> BCFC_CSC_SW_CX_F_BITS, 0));
            BDBG_MODULE_MSG(BVDC_CFC_BG,("NL2020 [0] %x [1] %x [2] %x", ul0, ul1, ul2));
        }
    }

    ulBgColorYCrCb =
        BPXL_MAKE_PIXEL(BPXL_eA8_Y8_Cb8_Cr8, 0, ul0, ul1, ul2);
    BDBG_MODULE_MSG(BVDC_CFC_BG, ("input bg color R %x G %x B %x output bg color %x",
        hCompositor->stNewInfo.ucRed, hCompositor->stNewInfo.ucGreen, hCompositor->stNewInfo.ucBlue, ulBgColorYCrCb));

    BDBG_LEAVE(BVDC_P_Compositor_Update_Canvas_Background_isr);
    return (ulBgColorYCrCb);
}

/* configure hCompositor->stOutColorSpaceExt.stColorSpace for all cases,
 * plus hDisplay->stOutColorSpaceExt.stColorSpace and
 * hDisplay->stCfc.stColorSpaceExtIn.stColorSpace for hdmi out
 *
 * note: CMP always output limited range YCbCr
 *
 */
void BVDC_P_Compositor_UpdateOutColorSpace_isr
    ( BVDC_Compositor_Handle           hCompositor,
      bool                             bApplyChanges )
{
    const BFMT_VideoInfo *pFmtInfo;
    BAVC_MatrixCoefficients  eOutAvcMatrixCoeffs;
    BCFC_Colorimetry  eOutColorimetry;
    BCFC_ColorSpace  stColorSpace;
    BCFC_ColorSpace  *pCmpColorSpace = &hCompositor->stOutColorSpaceExt.stColorSpace;
    BVDC_Display_Handle  hDisplay = hCompositor->hDisplay;
    BVDC_P_Display_HdmiSettings *pHdmiSettings;
    bool bHdmiOut = false;
    bool bOutputXvYcc;

    if (bApplyChanges)
    {
        BDBG_MODULE_MSG(BVDC_CFC_4, ("Display%d updateOutColorSpace (bApplyChanges 1)", hDisplay->eId));
    }
    else
    {
        /* invoking UpdateOutColorSpace_isr in WindowsReader_isr only once for a new nxclient */
        if (BCFC_ColorFormat_eInvalid != pCmpColorSpace->eColorFmt)
        {
            return;
        }
        BDBG_MODULE_MSG(BVDC_CFC_4, ("Display%d updateOutColorSpace (bApplyChanges 0)", hDisplay->eId));
    }
    pFmtInfo = hDisplay->stCurInfo.pFmtInfo;
    pHdmiSettings = &hDisplay->stCurInfo.stHdmiSettings;
    bOutputXvYcc = hDisplay->stCurInfo.bXvYcc;

    if(!hCompositor->bIsBypass)
    {
        eOutAvcMatrixCoeffs = pHdmiSettings->stSettings.eMatrixCoeffs;

        if (BVDC_P_IS_CUSTOMFMT(pFmtInfo->eVideoFmt))
        {
            /* May need to select something else in the future if
             * custom transcoding formats or customer is something else.
             * The selection is backward compatible. */
            /* customize output color space according to format info:
             *     - if vsync rate is 25/50Hz, choose HD or PAL SD by format height;
             *        else, choose HD or NTSC SD by format height; */
            if(pFmtInfo->ulVertFreq == 25*BFMT_FREQ_FACTOR ||
               pFmtInfo->ulVertFreq == 50*BFMT_FREQ_FACTOR)
            {
                eOutAvcMatrixCoeffs = (pFmtInfo->ulDigitalHeight > 576) ?
                    BAVC_MatrixCoefficients_eItu_R_BT_709 : BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG;
            }
            else
            {
                eOutAvcMatrixCoeffs = (pFmtInfo->ulDigitalHeight > 480) ?
                    BAVC_MatrixCoefficients_eItu_R_BT_709 : BAVC_MatrixCoefficients_eSmpte_170M;
            }
            BDBG_MSG(("CMP%u fmt[%ux%u%c%u] csc = %u", hCompositor->eId, pFmtInfo->ulDigitalWidth,
                pFmtInfo->ulDigitalHeight, pFmtInfo->bInterlaced?'i':'p',
                pFmtInfo->ulVertFreq/BFMT_FREQ_FACTOR, eOutAvcMatrixCoeffs));
        }
        else if (BAVC_MatrixCoefficients_eUnknown == eOutAvcMatrixCoeffs)
        {
            /* hdmi output is off */
            eOutAvcMatrixCoeffs = BAVC_GetDefaultMatrixCoefficients_isrsafe(pFmtInfo->eVideoFmt, bOutputXvYcc);
        }
        else
        {
            if ((eOutAvcMatrixCoeffs == BAVC_MatrixCoefficients_eHdmi_RGB) ||
                (eOutAvcMatrixCoeffs == BAVC_MatrixCoefficients_eUnknown) ||
                (eOutAvcMatrixCoeffs == BAVC_MatrixCoefficients_eDvi_Full_Range_RGB) ||
                (eOutAvcMatrixCoeffs == BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr))
            {
                eOutAvcMatrixCoeffs = BAVC_GetDefaultMatrixCoefficients_isrsafe(pFmtInfo->eVideoFmt, bOutputXvYcc);
            }
            bHdmiOut = true;
        }

        eOutColorimetry = BCFC_AvcColorInfoToColorimetry_isrsafe(BAVC_ColorPrimaries_eUnknown, eOutAvcMatrixCoeffs, bOutputXvYcc);
    }
    else
    {
        int i;
        BVDC_Window_Handle hWindow;

        /* set up cmp output MatrixCoeffs for bypass 656.
           the color space conversion would be done inside 656 encoder. */
        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            hWindow = hCompositor->ahWindow[i];
            BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

            if(BVDC_P_WIN_IS_VIDEO_WINDOW(hWindow->eId))
            {
                eOutColorimetry = hWindow->hBuffer->pCurReaderBuf->astMosaicColorSpace[0].eColorimetry;
                break;
            }
            else
            {
                eOutColorimetry = BFMT_IS_27Mhz(pFmtInfo->ulPxlFreqMask)?
                    BCFC_Colorimetry_eSmpte170M : BCFC_Colorimetry_eBt709;
            }
        }
    }

    /* configure hCompositor->hOutColorSpaceExt for all cases,
     * plus display->stOutColorSpaceExt for hdmi out */
    stColorSpace.eColorFmt = BCFC_ColorFormat_eYCbCr;
    stColorSpace.eColorRange = BCFC_ColorRange_eLimited;
    stColorSpace.eColorimetry = eOutColorimetry;
    stColorSpace.stMetadata.pDynamic = NULL;
    if (bHdmiOut)
    {
        /* cmp output color space */
        stColorSpace.eColorTF = s_aAvcEotf_to_TF[pHdmiSettings->stSettings.eEotf];
        stColorSpace.eColorDepth = (BCFC_ColorDepth)pHdmiSettings->eHdmiColorDepth;
    }
    else
    {
        stColorSpace.eColorTF = BCFC_ColorTF_eBt1886;
        stColorSpace.eColorDepth = BCFC_ColorDepth_e8Bit;
    }
    if (BCFC_COLOR_SPACE_DIFF(&stColorSpace, pCmpColorSpace))
    {
        BDBG_MODULE_MSG(BVDC_CFC_1,("Cmp%d Output colorSpace changed:", hCompositor->eId));
      #if (BDBG_DEBUG_BUILD)
        BDBG_MODULE_MSG(BVDC_CFC_1,("   ColorFmt    %9s ===> %-9s", BCFC_GetColorFormatName_isrsafe(pCmpColorSpace->eColorFmt),    BCFC_GetColorFormatName_isrsafe(stColorSpace.eColorFmt)));
        BDBG_MODULE_MSG(BVDC_CFC_1,("   Colorimetry %9s ===> %-9s", BCFC_GetColorimetryName_isrsafe(pCmpColorSpace->eColorimetry), BCFC_GetColorimetryName_isrsafe(stColorSpace.eColorimetry)));
        BDBG_MODULE_MSG(BVDC_CFC_1,("   ColorRange  %9s ===> %-9s", BCFC_GetColorRangeName_isrsafe(pCmpColorSpace->eColorRange),   BCFC_GetColorRangeName_isrsafe(stColorSpace.eColorRange)));
        BDBG_MODULE_MSG(BVDC_CFC_1,("   ColorTF     %9s ===> %-9s", BCFC_GetColorTfName_isrsafe(pCmpColorSpace->eColorTF),         BCFC_GetColorTfName_isrsafe(stColorSpace.eColorTF)));
        BDBG_MODULE_MSG(BVDC_CFC_1,("   ColorDepth  %9s ===> %-9s", BCFC_GetColorDepthName_isrsafe(pCmpColorSpace->eColorDepth),   BCFC_GetColorDepthName_isrsafe(stColorSpace.eColorDepth)));
      #endif
        hCompositor->stCurInfo.stDirty.stBits.bOutColorSpace = BVDC_P_DIRTY;
        hCompositor->stOutColorSpaceExt.stCfg.stBits.bDirty = BVDC_P_DIRTY;
        *pCmpColorSpace = stColorSpace;
    }

    /* only hdmi out display uses SW module CFC */
    if (bHdmiOut)
    {
        bool bColorSpacesChanged;
        bColorSpacesChanged = BVDC_P_Display_UpdateCfcColorSpaces_isr(hDisplay, &stColorSpace);
        if (bColorSpacesChanged)
        {
            hCompositor->stCurInfo.stDirty.stBits.bOutColorSpace = BVDC_P_DIRTY; /* for SetBypassColor */
        }
    }
}

#define BVDC_P_INVALID_CMP_MOSAIC_CFC_IDX   (0xFF)
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
#define BVDC_P_PREFER_TF_CONV(i, o)     (((i) != (o)) || ((i) == BCFC_ColorTF_eHlg))
#else
#define BVDC_P_PREFER_TF_CONV(i, o)     BCFC_NEED_TF_CONV(i, o)
#endif
/* Called by BVDC_P_Window_Reader_isr, to assign CFC for each mosaic rectangle
 * return true if some mosaic rect's colorSpace changed
 */
bool BVDC_P_Window_AssignMosaicCfcToRect_isr(
    BVDC_Window_Handle          hWindow,
    BVDC_P_PictureNode         *pPicture,
    bool                        bOutColorSpaceDirty)
{
    int ii, jj;
    uint32_t ulMosaicCount = (hWindow->stCurInfo.bMosaicMode)? hWindow->stCurInfo.ulMosaicCount : 1;
    uint32_t ulCmpNumCscs = BVDC_P_CMP_CFCS;
    BCFC_ColorSpace  *pPicColorSpace, *pColorSpace;
    bool bCfcUsed[BVDC_P_CMP_CFCS] = {false};
    bool bAllEarlierCfcsUsed;
    bool bNoTfConvCfc = (BVDC_P_CMP_0_TF_CONV_CFCS == 0);
  #if (BDBG_DEBUG_BUILD)
    BVDC_WindowId eWinInCmp = hWindow->eId - BVDC_P_CMP_GET_V0ID(hWindow->hCompositor);
  #endif

    if (!bOutColorSpaceDirty)
    {
        bool bCfcDirty = false;

        /* quick check for NOT dirty case
         */
        for (ii=0; ii<(int)ulMosaicCount; ii++)
        {
          #if BVDC_P_DBV_SUPPORT
            const BVDC_P_CfcMetaData *pPicMetaData = (BVDC_P_CfcMetaData *)pPicture->astMosaicColorSpace[ii].stMetadata.pDynamic;
            const BVDC_P_CfcMetaData *pMetaData = (BVDC_P_CfcMetaData *)hWindow->astMosaicCfc[hWindow->aucMosaicCfcIdxForRect[ii]].stColorSpaceExtIn.stColorSpace.stMetadata.pDynamic;
          #endif
            pPicColorSpace = &pPicture->astMosaicColorSpace[ii];
            pColorSpace = &hWindow->astMosaicCfc[hWindow->aucMosaicCfcIdxForRect[ii]].stColorSpaceExtIn.stColorSpace;
            if (BCFC_COLOR_SPACE_DIFF(pPicColorSpace, pColorSpace)
              #if BVDC_P_DBV_SUPPORT
                || (pPicMetaData->stDbvInput.stHdrMetadata.eType == BAVC_HdrMetadataType_eDrpu)
                || (pMetaData == NULL) /* 1st time use of this cfc */
                || (pMetaData->stDbvInput.stHdrMetadata.eType == BAVC_HdrMetadataType_eDrpu)
                || BKNI_Memcmp_isr(&pPicColorSpace->stMetadata.stStatic,/* react to dynamic hdr10 parameters during repeat mode */
                                   &pColorSpace->stMetadata.stStatic, sizeof(pColorSpace->stMetadata.stStatic))
                || hWindow->bCfcDirty
              #endif
              #if BVDC_P_TCH_SUPPORT && BVDC_P_TCH_CONFORMANCE
                || BCFC_IS_TCH(pPicMetaData->stTchInput.stHdrMetadata.eType)
              #endif
            )
            {
              #if BVDC_P_DBV_SUPPORT
                if(pMetaData && pMetaData->stDbvInput.stHdrMetadata.eType != pPicMetaData->stDbvInput.stHdrMetadata.eType)
                {
                    hWindow->hCompositor->stOutColorSpaceExt.stCfg.stBits.bDirty = BVDC_P_DIRTY;/* this goes to cmp/gfd */
                    hWindow->hCompositor->hDisplay->stCfc.stColorSpaceExtIn.stCfg.stBits.bDirty = BVDC_P_DIRTY;/* this goes to vec */
                    hWindow->hCompositor->hDisplay->stCurInfo.stDirty.stBits.bInputCS = BVDC_P_DIRTY;
                    hWindow->bCfcDirty = true; /* dynamic source toggled on/off metadata; need to refresh cfc */
                }
                if(pPicMetaData->stDbvInput.stHdrMetadata.eType == BAVC_HdrMetadataType_eDrpu)
                {
                    hWindow->bCfcDirty = true; /* has metadata */
                }
              #endif

                bCfcDirty = true;
                break;
            }
        }
        if (!bCfcDirty)
        {
            return false;
        }
    }

    /*
     * to be here, some mosaic rect's colorSpace, or output colorSpace changed
     */

    /* handle non-mosaic case 1st. To be here, the input colorSpace must have changed
     * note: we assume dolby / tch are NOT supported in in mosaic mode !!! */
    if (0 == hWindow->stCurInfo.ulMosaicCount)
    {
        hWindow->aucMosaicCfcIdxForRect[0] = 0;
        hWindow->pMainCfc = &hWindow->astMosaicCfc[0];
        hWindow->aucMosaicCfcIdxForRect[1] = 1;
        hWindow->pDemoCfc = &hWindow->astMosaicCfc[1];
        /* note: at this moment, if this cfc has been used, hWindow->astMosaicCfc[0].stColorSpaceExtIn.stColorSpace.pMetaData points to
         * the stCfcMetaData in previous reader pic node (unlikely pPicture), otherwise it is NULL;
         * after the following copying, hWindow->astMosaicCfc[0].stColorSpaceExtIn.stColorSpace.pMetaData = &pPicture->astMosaicMetaData[0]
         * it is OK because we will trasfer metaData info into DBV lib in BVDC_P_Compositor_ApplyDbvSettings_isr before rul build,
         * and after this vsync we will never use hWindow->astMosaicCfc[0].stColorSpaceExtIn.stColorSpace.pMetaData again
         * note: writer might repeat to write a pic node, reader can also skip pic nodes. However, that is ok, because metaData in skipped
         * pic nodes will not be applied to later pic nodes */
        hWindow->astMosaicCfc[0].stColorSpaceExtIn.stColorSpace = pPicture->astMosaicColorSpace[0];
        hWindow->astMosaicCfc[0].stColorSpaceExtIn.stCfg.stBits.bDirty = BVDC_P_DIRTY;
        return true;
    }

    /*
     * now we re-assign cfcs for each mosaic rects
     */

    if (BVDC_CompositorId_eCompositor0 != hWindow->hCompositor->eId)
    {
        ulCmpNumCscs = BVDC_P_CMP_i_MOSAIC_CFCS;
        bNoTfConvCfc = true; /* not seen TfConvCfc in a CMP other than CMP0 */
    }

    for (ii=0; ii<(int)ulMosaicCount; ii++)
    {
        pPicColorSpace = &pPicture->astMosaicColorSpace[ii];
        if (BCFC_ColorFormat_eInvalid == pPicColorSpace->eColorFmt)
        {
            pPicColorSpace->eColorFmt = BCFC_ColorFormat_eYCbCr;
        }
        hWindow->aucMosaicCfcIdxForRect[ii] = BVDC_P_INVALID_CMP_MOSAIC_CFC_IDX;
    }

    /* firstly find out the previously assigned / configured CFCs that are still useful in this vsync
     * note: if a TF-conv-capable cfe has been used for a non-TF_conv case, try not use it */
    if (!bOutColorSpaceDirty)
    {
        for (ii=0; ii<(int)ulMosaicCount; ii++)
        {
            pPicColorSpace = &pPicture->astMosaicColorSpace[ii];
            bAllEarlierCfcsUsed = true;
            for (jj=ulCmpNumCscs-1; jj>=0; jj--)
            {
                pColorSpace = &hWindow->astMosaicCfc[jj].stColorSpaceExtIn.stColorSpace;
                if ((!BCFC_COLOR_SPACE_DIFF(pPicColorSpace, pColorSpace)) &&
                    ((BVDC_P_PREFER_TF_CONV(pColorSpace->eColorTF,
                                            hWindow->astMosaicCfc[jj].pColorSpaceExtOut->stColorSpace.eColorTF) ==
                      hWindow->astMosaicCfc[jj].stCapability.stBits.bLRngAdj) ||
                     (hWindow->astMosaicCfc[jj].stCapability.stBits.bLRngAdj && bAllEarlierCfcsUsed) ||
                     (bNoTfConvCfc)))
                {
                    /* this rect's ColorSpace matches this cfc's ColorSpaceIn */
                  #if (BDBG_DEBUG_BUILD)
                    BDBG_MODULE_MSG(BVDC_CFC_4,("Cmp%d_V%d asign Rect%d -> Cfc%d, reuse 1st round", hWindow->hCompositor->eId, eWinInCmp, ii, jj));
                  #endif
                    hWindow->aucMosaicCfcIdxForRect[ii] = jj;
                    bCfcUsed[jj] = true;
                    break;
                }
                bAllEarlierCfcsUsed &= bCfcUsed[jj];
            }
        }
    }

    /* secondly assign unused cfcs to unassigned rects */
    for (ii=0; ii<(int)ulMosaicCount; ii++)
    {
        if (BVDC_P_INVALID_CMP_MOSAIC_CFC_IDX != hWindow->aucMosaicCfcIdxForRect[ii])
        {
            continue; /* CFC already assigned for this mosaic rect */
        }

        /* if a cfc has already been assigned to a rect that has the same colorSpace as this rect */
        pPicColorSpace = &pPicture->astMosaicColorSpace[ii];
        bAllEarlierCfcsUsed = true;
        for (jj=ulCmpNumCscs-1; jj>=0; jj--)
        {
            pColorSpace = &hWindow->astMosaicCfc[jj].stColorSpaceExtIn.stColorSpace;
            if ((!bOutColorSpaceDirty || bCfcUsed[jj] /* bCfcUsed[jj]==true means assigned with cur out colorSpace */) &&
                (!BCFC_COLOR_SPACE_DIFF(pPicColorSpace, pColorSpace)) &&
                ((BVDC_P_PREFER_TF_CONV(pColorSpace->eColorTF,
                                        hWindow->astMosaicCfc[jj].pColorSpaceExtOut->stColorSpace.eColorTF) ==
                  hWindow->astMosaicCfc[jj].stCapability.stBits.bLRngAdj) ||
                 (hWindow->astMosaicCfc[jj].stCapability.stBits.bLRngAdj && bAllEarlierCfcsUsed) ||
                  (bNoTfConvCfc)))
            {
                /* this rect's ColorSpace matches this cfc's ColorSpaceIn,
                 * note: this cfc must have already be marked as used */
                hWindow->aucMosaicCfcIdxForRect[ii] = jj;
              #if (BDBG_DEBUG_BUILD)
                BDBG_MODULE_MSG(BVDC_CFC_4,("Cmp%d_V%d asign Rect%d -> Cfc%d, reuse 2nd round", hWindow->hCompositor->eId, eWinInCmp, ii, jj));
              #endif
                bCfcUsed[jj] = true;
                break;
            }
            bAllEarlierCfcsUsed &= bCfcUsed[jj];
        }

        if (jj < 0)
        {
            /* need to assign a new cfc
             * note: the BVDC_P_TF_CONV_SLOTS cfcs in starting end of the list are the ones that can do TfConv
             * we always try to use the cfcs that can NOT do TfConv */
            for (jj=ulCmpNumCscs-1; jj>=0; jj--)
            {
                if (!bCfcUsed[jj] &&
                    (!BVDC_P_PREFER_TF_CONV(pPicColorSpace->eColorTF,
                                            hWindow->astMosaicCfc[jj].pColorSpaceExtOut->stColorSpace.eColorTF) ||
                     hWindow->astMosaicCfc[jj].stCapability.stBits.bLRngAdj || bNoTfConvCfc))
                {
                    /* note: now hWindow->astMosaicCfc[0].stColorSpaceExtIn.stColorSpace.pMetaData = &pPicture->astMosaicMetaData[jj] */
                    hWindow->astMosaicCfc[jj].stColorSpaceExtIn.stColorSpace = pPicture->astMosaicColorSpace[ii];
                    hWindow->astMosaicCfc[jj].stColorSpaceExtIn.stCfg.stBits.bDirty = BVDC_P_DIRTY;
                    hWindow->aucMosaicCfcIdxForRect[ii] = jj;
                  #if (BDBG_DEBUG_BUILD)
                    BDBG_MODULE_MSG(BVDC_CFC_4,("Cmp%d_V%d asign Rect%d -> Cfc%d, new use. colorFmt %s", hWindow->hCompositor->eId, eWinInCmp,
                        ii, jj, BCFC_GetColorFormatName_isrsafe(pPicColorSpace->eColorFmt)));
                  #endif
                    bCfcUsed[jj] = true;
                    break;
                }
            }

            if (jj<0)
            {
                /* likely not be able to find a TVConv Cfc, assign one without this capability,
                 * this rect's color will not look correct */
                for (jj=ulCmpNumCscs-1; jj>=0; jj--)
                {
                    if (!bCfcUsed[jj])
                    {
                        /* note: now hWindow->astMosaicCfc[0].stColorSpaceExtIn.stColorSpace.pMetaData = &pPicture->astMosaicMetaData[jj] */
                        hWindow->astMosaicCfc[jj].stColorSpaceExtIn.stColorSpace = pPicture->astMosaicColorSpace[ii];
                        hWindow->astMosaicCfc[jj].stColorSpaceExtIn.stCfg.stBits.bDirty = BVDC_P_DIRTY;
                        hWindow->aucMosaicCfcIdxForRect[ii] = jj;
#if (BDBG_DEBUG_BUILD)
                        BDBG_WRN(("Cmp%d_V%d asign Rect%d -> Cfc%d. Need TF, but no TF CFC available, color might be bad", hWindow->hCompositor->eId, eWinInCmp, ii, jj));
#endif
                        bCfcUsed[jj] = true;
                        break;
                    }
                }
                if (jj<0)
                {
                    hWindow->aucMosaicCfcIdxForRect[ii] = 0;
#if (BDBG_DEBUG_BUILD)
                    BDBG_WRN(("Cmp%d_V%d asign Rect%d -> Cfc0 because no more CFC available, color might be bad", hWindow->hCompositor->eId, eWinInCmp, ii));
#endif
                }
            }
        }
    }

    if (bOutColorSpaceDirty)
    {
        /* force to clean all previous stale state */
        for (jj=ulCmpNumCscs-1; jj>=0; jj--)
        {
            if (!bCfcUsed[jj])
            {
                hWindow->astMosaicCfc[jj].stColorSpaceExtIn.stColorSpace.eColorFmt = BCFC_ColorFormat_eInvalid;
            }
        }
    }

    hWindow->pMainCfc = &hWindow->astMosaicCfc[hWindow->aucMosaicCfcIdxForRect[0]];
    return true;
}

#define BVDC_P_CFC_SWAP_FOR_CL_OUT       1
#if BVDC_P_CFC_SWAP_FOR_CL_OUT
#define BVDC_P_CFC_SWAP_NEG_POS_MC       1
#endif

/* Configure a CFC according to its input and output color space
 */
void BVDC_P_Cfc_UpdateCfg_isr
    ( BCFC_Context        *pCfc,
      bool                 bMosaicMode,
      bool                 bForceDirty)
{
    BCFC_ColorSpaceExt *pColorSpaceExtIn = &(pCfc->stColorSpaceExtIn);
    BCFC_ColorSpaceExt *pColorSpaceExtOut = pCfc->pColorSpaceExtOut;
    BCFC_ColorSpace *pColorSpaceIn = &(pColorSpaceExtIn->stColorSpace);
    BCFC_ColorSpace *pColorSpaceOut = &(pColorSpaceExtOut->stColorSpace);
    bool bTchInput = false;
    bool bRamLutCfgDirty;
#if BVDC_P_DBV_SUPPORT || BVDC_P_TCH_SUPPORT
    const BVDC_P_CfcMetaData *pMetaData = (BVDC_P_CfcMetaData *)pColorSpaceIn->stMetadata.pDynamic;
#endif

    /* check for GFD and DVI_CSC: CL input with SDR/HDR, CL display with SDR/HDR ??? */

    if ((BCFC_ColorFormat_eInvalid == pColorSpaceIn->eColorFmt) ||
        (BCFC_ColorFormat_eInvalid == pColorSpaceOut->eColorFmt))
    {
        /* this cfc is not in use yet */
#if (BDBG_DEBUG_BUILD)
        BDBG_MODULE_MSG(BVDC_CFC_4,("%s-Cfc%d not used: ColorFmt %s -> %s", BCFC_GetCfcName_isrsafe(pCfc->eId), pCfc->ucMosaicSlotIdx,
            BCFC_GetColorFormatName_isrsafe(pColorSpaceIn->eColorFmt), BCFC_GetColorFormatName_isrsafe(pColorSpaceOut->eColorFmt)));
#endif
        return;
    }

    pCfc->bForceBypassTfTbl = false; /* init here in case switch to DBV or TCH */

#if BVDC_P_DBV_SUPPORT
    if (pMetaData && pMetaData->stDbvInput.stHdrMetadata.eType == BAVC_HdrMetadataType_eDrpu &&
        !pCfc->stForceCfg.stBits.bDisableDolby)
    {
        pColorSpaceExtIn->stCfg.stBits.bDirty = BVDC_P_CLEAN;
        pColorSpaceExtOut->stCfg.stBits.bDirty = BVDC_P_CLEAN;
        pCfc->ucRulBuildCntr = BVDC_P_RUL_UPDATE_THRESHOLD;
        return; /* defered to build RUL later */
    }
#endif

#if BVDC_P_TCH_SUPPORT /* update TCH input info */
    bTchInput = (pMetaData)? BCFC_IS_TCH(pMetaData->stTchInput.stHdrMetadata.eType) : false;
#endif

    bRamLutCfgDirty = BCFC_UpdateCfg_isr(pCfc, bMosaicMode, bTchInput, bForceDirty);

    /* further modify to use ram lut if it is proper */
    if (bRamLutCfgDirty)
    {
#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
        if (/*pCfc->stCapability.stBits.bRamNL2L &&*/
            !pCfc->stForceCfg.stBits.bDisableRamLuts &&
            BCFC_IN_CMP0V0(pCfc->eId))
        {
            /*if (BCFC_IS_HLG_TO_HDR10(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF)) */
            if (pColorSpaceIn->eColorTF == BCFC_ColorTF_eHlg)
            {
                pColorSpaceExtIn->stCfg.stBits.SelTF = BCFC_NL2L_RAM;
                pCfc->stLRangeAdj.pTable = s_aaLRangeAdj_in_Hlg_V0_Ver2_Tbl[pColorSpaceOut->eColorTF];
            }
            else if (pColorSpaceOut->eColorTF == BCFC_ColorTF_eHlg)
            {
                pColorSpaceExtOut->stCfg.stBits.SelTF = BCFC_L2NL_RAM;
                pCfc->stLRangeAdj.pTable = s_aaLRangeAdj_out_Hlg_V0_Ver2_Tbl[pColorSpaceIn->eColorTF];
            }
        }
        else if (/*pCfc->stCapability.stBits.bRamL2NL &&*/
            !pCfc->stForceCfg.stBits.bDisableRamLuts &&
            BCFC_IN_GFD0(pCfc->eId) &&
            (pColorSpaceOut->eColorTF == BCFC_ColorTF_eHlg))
        {
            pColorSpaceExtOut->stCfg.stBits.SelTF = BCFC_L2NL_RAM;
            if (BCFC_IS_SDR_TO_HLG(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF))
            {
                pCfc->stLRangeAdj.pTable = &s_LRangeAdj_GFD_1886_to_hlg;
            }
        }
#elif (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) /* #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) */
        if (pCfc->stCapability.stBits.bRamLutScb && pCfc->pLutList->pulStart[pCfc->pLutList->ulIndex] &&
            !BCFC_IN_DVI(pCfc->eId) && /* rm this after DVI ramLut is also handled */
            !pCfc->stForceCfg.stBits.bDisableRamLuts)
        {
            BCFC_ColorTF eTfIn = pColorSpaceIn->eColorTF, eTfOut = pColorSpaceOut->eColorTF;
            const BCFC_TfConvRamLuts *pNewRamLuts, *pCurRamLuts;
          #if BVDC_P_TCH_SUPPORT
            const BCFC_TfConvRamLuts *pTpSdrRamLuts = NULL;
            if(pMetaData && BCFC_IS_TCH(pMetaData->stTchInput.stHdrMetadata.eType) &&
               BCFC_IS_SDR_TO_HDR10(eTfIn, eTfOut))
            {
                pTpSdrRamLuts =
                    (pMetaData->stTchInput.bIsNcl)? &s_TfConvRamLutsTpTo2084_ncl :
                    (pMetaData->stTchInput.ulHdrDisplayMaxLuminance == 2000) ? &s_TfConvRamLutsTpTo2084_cl_maxlum_2000 :
                    (pMetaData->stTchInput.ulHdrDisplayMaxLuminance == 4000) ? &s_TfConvRamLutsTpTo2084_cl_maxlum_4000 :
                    &s_TfConvRamLutsTpTo2084;
            }
          #endif
            pCurRamLuts = pCfc->pTfConvRamLuts;
            pNewRamLuts = ((BCFC_IN_GFD(pCfc->eId))? &(s_aaGfdTfConvRamLuts_Tbl[eTfIn][eTfOut]) :
                         #if BVDC_P_TCH_SUPPORT /* use s_aaTpTfConvRamLuts_Tbl[eTfOut] */
                           (pMetaData && BCFC_IS_TCH(pMetaData->stTchInput.stHdrMetadata.eType) &&
                            BCFC_IS_SDR_TO_HDR10(eTfIn, eTfOut) && (!pCfc->stForceCfg.stBits.bDisableTch)) ? pTpSdrRamLuts :
                           (pMetaData && BCFC_IS_TCH(pMetaData->stTchInput.stHdrMetadata.eType) &&
                            BCFC_IS_HDR10_TO_SDR(eTfIn, eTfOut) && (!pCfc->stForceCfg.stBits.bDisableTch)) ? &s_TfConvRamLutsTpTo1886 :
                         #endif /* #if BVDC_P_CMP_SUPPORT_TP */
                           (!bMosaicMode)? &(s_aaCmp0TfConvRamLuts_Tbl[eTfIn][eTfOut]) : &(s_aaCmp0MosaicTfConvRamLuts_Tbl[eTfIn][eTfOut]));
            if (!pCfc->stForceCfg.stBits.bDisableLmr || (NULL == pNewRamLuts->pRamLutLMR))
            {
                if (!pCfc->stForceCfg.stBits.bDisableNl2l && (pNewRamLuts->pRamLutNL2L))
                {
                    pColorSpaceExtIn->stCfg.stBits.SelTF = BCFC_NL2L_RAM;
                    if (pNewRamLuts->pRamLutNL2L != pCurRamLuts->pRamLutNL2L)
                    {
                        pCfc->ucRamNL2LRulBuildCntr = BVDC_P_RUL_UPDATE_THRESHOLD;
                    }
                }
                if (!pCfc->stForceCfg.stBits.bDisableL2nl && (pNewRamLuts->pRamLutL2NL))
                {
                    pColorSpaceExtOut->stCfg.stBits.SelTF = BCFC_L2NL_RAM;
                    if (pNewRamLuts->pRamLutL2NL != pCurRamLuts->pRamLutL2NL)
                    {
                        pCfc->ucRamL2NLRulBuildCntr = BVDC_P_RUL_UPDATE_THRESHOLD;
                    }
                }
                if (pNewRamLuts->pRamLutLMR && (pNewRamLuts->pRamLutLMR != pCurRamLuts->pRamLutLMR))
                {
                    pCfc->ucRamLMRRulBuildCntr = BVDC_P_RUL_UPDATE_THRESHOLD;
                }
                if (!pCfc->stForceCfg.stBits.bDisableLRangeAdj)
                {
                    pCfc->stLRangeAdj.pTable = pNewRamLuts->pLRngAdjTable;
                    /*pCfc->stLRangeAdj.ulLRangeAdjCtrl = BCFC_LR_ADJ_LIMIT_DISABLE;*/
                }
                pCfc->pTfConvRamLuts = pNewRamLuts;
            }
        }
        else if (pCfc->stCapability.stBits.bRamNL2L &&
                 BCFC_IN_VFC(pCfc->eId) &&
                 BCFC_IS_HLG_TO_SDR(pColorSpaceIn->eColorTF, pColorSpaceOut->eColorTF))
        {
            pColorSpaceExtIn->stCfg.stBits.SelTF = BCFC_NL2L_RAM;
            pColorSpaceExtOut->stCfg.stBits.SelTF = BCFC_L2NL_RAM;
            pCfc->stLRangeAdj.pTable = &s_LRangeAdj_Identity;
            /* pCfc->pTfConvRamLuts is not used for vfc */
        }
        /* else pCfc->pTfConvRamLuts was initialized as &s_TfConvRamLutsBypass */
#endif /* #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) */
    }
}

/* utilty functons to print matrix
 */

#define BVDC_P_CSC_FLOATING_POINT_MSG     0

#define BVDC_P_CX_TO_FLOAT(x) \
    (((int32_t)((BCFC_CSC_SW_SIGN_MASK & x) ? -((BCFC_CSC_SW_MASK & ~x) + 1) : x) / (float)(1 << BCFC_CSC_SW_CX_F_BITS)))
#define BVDC_P_CO_TO_FLOAT(x) \
    (((int32_t)((BCFC_CSC_SW_SIGN_MASK & x) ? -((BCFC_CSC_SW_MASK & ~x) + 1) : x) / (float)(1 << BCFC_CSC_SW_CO_F_BITS)))

static void BCFC_PrintFloatMcAdj_isrsafe(const BCFC_Csc3x4 *pCsc, const BCFC_Csc3x4 *pAlt)
{
#if ((BVDC_P_CSC_FLOATING_POINT_MSG) && (BDBG_DEBUG_BUILD))
    int ii;

    for(ii=0; ii<3; ii++)
    {
        BDBG_MODULE_MSG(BVDC_MC_ADJ,("   [%13.8f %13.8f %13.8f %13.8f]",
            BVDC_P_CX_TO_FLOAT(pCsc->m[ii][0]), BVDC_P_CX_TO_FLOAT(pCsc->m[ii][1]), BVDC_P_CX_TO_FLOAT(pCsc->m[ii][2]), BVDC_P_CO_TO_FLOAT(pCsc->m[ii][3])));
    }

    if (pAlt==NULL)
        return;

    for(ii=1; ii<3; ii++)
    {
        BDBG_MODULE_MSG(BVDC_MC_ADJ,("   [%13.8f %13.8f %13.8f %13.8f]",
            BVDC_P_CX_TO_FLOAT(pCsc->m[ii][0]), BVDC_P_CX_TO_FLOAT(pCsc->m[ii][1]), BVDC_P_CX_TO_FLOAT(pCsc->m[ii][2]), BVDC_P_CO_TO_FLOAT(pCsc->m[ii][3])));
    }
#else
    BSTD_UNUSED(pCsc);
    BSTD_UNUSED(pAlt);
#endif
}

/* HW matrix structure:

  7271 B0: ---------------------------------------------------

  CMP0   MC  3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eOff/e1_1/e2_1
         MA  5x4 BCFC_CscType_eMab5x4, BCFC_LeftShift_eOff/e1_1/e2_1
         MB  3x4 BCFC_CscType_eMab3x4, BCFC_LeftShift_eOff/e1_1/e2_1
         MB2 1x4 BCFC_CscType_eMab1x4, BCFC_LeftShift_eNotExist
         BLN 3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eNotExist
  CMP1   MC  3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
  GFD0   MC  3x5 BCFC_CscType_eMc3x5,  BCFC_LeftShift_eOff/e1_1/e2_1
         MA  3x5 BCFC_CscType_eMab3x5, BCFC_LeftShift_eOff/e1_1/e2_1
         MB  3x5 BCFC_CscType_eMab3x5, BCFC_LeftShift_eOff/e1_1/e2_1
         MB2 1x4 BCFC_CscType_eMab1x5, BCFC_LeftShift_eNotExist
         BLN 3x5 BCFC_CscType_eMc3x5,  BCFC_LeftShift_eNotExist
  GFD1   MC  3x5 BCFC_CscType_eMc3x5,  BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
  DVI    MC  5x4 BCFC_CscType_eMc5x4,  BCFC_LeftShift_eOff/e1_0/e2_0
         MA  3x4 BCFC_CscType_eMab3x4, BCFC_LeftShift_eOff/e1_0/e2_0
         MB  3x4 BCFC_CscType_eMab3x4, BCFC_LeftShift_eOff/e1_0/e2_0
         MB2 none
         BLN none

  7271 A0: ---------------------------------------------------

  CMP0   MC  3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eNotExist
         MA  3x4 BCFC_CscType_eMab3x4, BCFC_LeftShift_eNotExist
         MB  3x4 BCFC_CscType_eMab3x4, BCFC_LeftShift_eNotExist
         MB2 none
         BLN 3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eNotExist
  CMP1   MC  3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
  GFD0   MC  3x5 BCFC_CscType_eMc3x5,  BCFC_LeftShift_eNotExist
         MA  3x5 BCFC_CscType_eMab3x5, BCFC_LeftShift_eNotExist
         MB  3x5 BCFC_CscType_eMab3x5, BCFC_LeftShift_eNotExist
         MB2 none
         BLN none
  GFD1   MC  3x5 BCFC_CscType_eMc3x5,  BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
  DVI    MC  3x4 BCFC_CscType_eMcPacked,BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none

  7439 B0: ---------------------------------------------------

  CMP0   MC  3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eNotExist
         MA  3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eNotExist
         MB  none
         MB2 none
         BLN none
  CMP1   MC  3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
  GFD0   MC  3x5 BCFC_CscType_eMc3x5,  BCFC_LeftShift_eNotExist
         MA  3x5 BCFC_CscType_eMc3x5,  BCFC_LeftShift_eNotExist
         MB  none
         MB2 none
         BLN none
  GFD1   MC  3x5 BCFC_CscType_eMc3x5,  BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
  DVI    MC  3x4 BCFC_CscType_eMcPacked,BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none

  7439 A0 and older: ---------------------------------------------------

  CMP0   MC  3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
  CMP1   MC  3x4 BCFC_CscType_eMc3x4,  BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
  GFD0   MC  3x5 BCFC_CscType_eMc3x5,  BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
  GFD1   MC  3x5 BCFC_CscType_eMc3x5,  BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
  DVI    MC  3x4 BCFC_CscType_eMcPacked,BCFC_LeftShift_eNotExist
         MA  none
         MB  none
         MB2 none
         BLN none
*/

#define BVDC_P_MAKE_HW_CX(a) (((a)>>(BCFC_CSC_SW_CX_F_BITS - ulCxFBits)) & ulMask)
#define BVDC_P_MAKE_HW_CO(o) (((o)>>(BCFC_CSC_SW_CO_F_BITS - ulCoFBits)) & ulMask)

#define BVDC_GFD_CO_TO_CX_MUL  \
    BMTH_FIX_SIGNED_FTOFIX((256.0f / 255.0f), BCFC_CSC_SW_CX_I_BITS, BCFC_CSC_SW_CX_F_BITS)
#define BVDC_GFD_CO_TO_CX_SHIFT    (8) /* (1<<8) = 256 */

/* this function could be called to build all matrices except for MB
 */
static void BVDC_P_Cfc_BuildRulForCscRx4_isr(
    const BCFC_Csc3x4               *pCsc,
    const BCFC_Csc3x4               *pAlt,
    uint32_t                         ulCfg,
    uint32_t                         ulStartReg,
    BVDC_P_ListInfo                 *pList)
{
    BCFC_CscType eCscType;
    BCFC_LeftShift eLeftShift;
    uint32_t ulMask, ulCxFBits, ulCoFBits;
    uint32_t ulLeftShiftBits;
    uint32_t ulRows, ulColumns;
    int ii;

    eCscType = BCFC_GET_CSC_TYPE(ulCfg);
    eLeftShift = BCFC_GET_CSC_LSHIFT(ulCfg);

    ulMask = BCFC_CSC_MASK(eCscType);
    ulCxFBits = BCFC_CSC_CX_F_BITS(eCscType);
    ulCoFBits = BCFC_CSC_CO_F_BITS(eCscType);
    ulRows = BCFC_CSC_ROWS(eCscType);
    ulColumns = BCFC_CSC_COLUMS(eCscType);
    if ((ulRows > 3) && (NULL == pAlt))
    {
        ulRows = 3;
    }

    ulLeftShiftBits = BCFC_LSHIFT_BITS(eLeftShift);
    if (ulLeftShiftBits)
    {
        ulMask >>= ulLeftShiftBits;
        ulCxFBits -= ulLeftShiftBits;
        ulCoFBits -= ulLeftShiftBits;
    }

    /* in DVI-CFC LeftShift registers are put at the end of all matrices
     */
    if (BCFC_NEED_WRITE_LSHIFT_REG(eLeftShift))
    {
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS( 1 + ulRows * ulColumns );
        *pList->pulCurrent++ = BRDC_REGISTER( ulStartReg - sizeof(uint32_t) );
        *pList->pulCurrent++ = ulLeftShiftBits;
    }
    else
    {
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulRows * ulColumns );
        *pList->pulCurrent++ = BRDC_REGISTER( ulStartReg );
    }
    if (BCFC_CscType_eMcPacked == eCscType)
    {
        /* this is a special format: always 3 x 4 with two elements in one register */
        BDBG_ASSERT((ulRows==3) && (ulColumns == 2));
        for (ii=0; ii<3; ii++)
        {
            *pList->pulCurrent++ = (BVDC_P_MAKE_HW_CX(pCsc->m[ii][1]) << 16) | (BVDC_P_MAKE_HW_CX(pCsc->m[ii][0]));
            *pList->pulCurrent++ = (BVDC_P_MAKE_HW_CO(pCsc->m[ii][3]) << 16) | (BVDC_P_MAKE_HW_CX(pCsc->m[ii][2]));
        }
    }
    else
    {
        /* column is always either 4 or 5 */
        BDBG_ASSERT((ulColumns == 4) || (ulColumns == 5));
        for (ii=0; ii<(int)BVDC_P_MIN(ulRows,3); ii++)
        {
            *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(pCsc->m[ii][0]);
            *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(pCsc->m[ii][1]);
            *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(pCsc->m[ii][2]);
            if (ulColumns == 4)
            {
                *pList->pulCurrent++ = BVDC_P_MAKE_HW_CO(pCsc->m[ii][3]);
            }
            else if (BCFC_CSC_GFD_CONST_BLEND(eCscType))
            {
                *pList->pulCurrent++ = 0;
                *pList->pulCurrent++ = BVDC_P_MAKE_HW_CO(pCsc->m[ii][3]);
            }
            else /* must be BCFC_CSC_GFD_ALPHA_BLEND(eCscType) */
            {
                uint32_t t1;
                int32_t t2;
                BDBG_ASSERT(BCFC_CSC_GFD_ALPHA_BLEND(eCscType));
                /* pCsc->m[ii][3] * 256.0/255.0 */
                t1 = (uint32_t)(0x00000000FFFFFFFF &
                                ((((int64_t)(pCsc->m[ii][3]) * (int64_t)BVDC_GFD_CO_TO_CX_MUL) +
                                  (1 << (BCFC_CSC_SW_CX_F_BITS - 1))) >> BCFC_CSC_SW_CX_F_BITS));
                t2 = *(int32_t *)(&t1);

                /* t1 / 256 and s9.22 -> s2.29 */
                t2 = t2 >> (BVDC_GFD_CO_TO_CX_SHIFT - (BCFC_CSC_SW_CX_F_BITS - BCFC_CSC_SW_CO_F_BITS));
                *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(t2);
                *pList->pulCurrent++ = 0;
            }
        }
        if ((ulRows > 3) && (NULL != pAlt))
        {
            for (ii=3; ii<(int)ulRows; ii++)
            {
                int iRowIdx = ii - 3 + 1;
                BDBG_ASSERT(ulColumns == 4);
                *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(pAlt->m[iRowIdx][0]);
                *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(pAlt->m[iRowIdx][1]);
                *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(pAlt->m[iRowIdx][2]);
                *pList->pulCurrent++ = BVDC_P_MAKE_HW_CO(pAlt->m[iRowIdx][3]);
            }
        }
    }

    BCFC_PrintFloatCscRx4_isrsafe(pCsc, pAlt);
    BCFC_PrintCscRx4_isrsafe(pList->pulCurrent - ulRows * ulColumns, ulCfg, pAlt!=NULL);
}

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2)

/* this function could be called to build CMP/GFD/VEC matrix B
 * note: MB HW has 4 columns, although the last column is likely set to 0
 */
static void BVDC_P_Cfc_BuildRulForCsc3x3_isr(
    const BCFC_Csc3x3               *pCsc,
    uint32_t                         ulCfg,
    uint32_t                         ulStartReg,
    BVDC_P_ListInfo                 *pList)
{
    BCFC_CscType eCscType;
    BCFC_LeftShift eLeftShift;
    uint32_t ulMask, ulCxFBits, ulCoFBits;
    uint32_t ulLeftShiftBits;
    uint32_t ulRows, ulColumns;
    int ii;

    eCscType = BCFC_GET_CSC_TYPE(ulCfg);
    eLeftShift = BCFC_GET_CSC_LSHIFT(ulCfg);

    ulMask = BCFC_CSC_MASK(eCscType);
    ulCxFBits = BCFC_CSC_CX_F_BITS(eCscType);
    ulCoFBits = BCFC_CSC_CO_F_BITS(eCscType);
    ulRows = BCFC_CSC_ROWS(eCscType);
    ulColumns = BCFC_CSC_COLUMS(eCscType);

    ulLeftShiftBits = BCFC_LSHIFT_BITS(eLeftShift);
    if (ulLeftShiftBits)
    {
        ulMask >>= ulLeftShiftBits;
        ulCxFBits -= ulLeftShiftBits;
        ulCoFBits -= ulLeftShiftBits;
    }

    if (BCFC_NEED_WRITE_LSHIFT_REG(eLeftShift))
    {
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS( 1 + ulRows * ulColumns );
        *pList->pulCurrent++ = BRDC_REGISTER( ulStartReg - sizeof(uint32_t) );
        *pList->pulCurrent++ = ulLeftShiftBits;
    }
    else
    {
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS( ulRows * ulColumns );
        *pList->pulCurrent++ = BRDC_REGISTER( ulStartReg );
    }

    /* column is always either 4 or 5 */
    BDBG_ASSERT((ulRows == 3) && (ulColumns == 4));
    for (ii=0; ii<3; ii++)
    {
        *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(pCsc->m[ii][0]);
        *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(pCsc->m[ii][1]);
        *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(pCsc->m[ii][2]);
        *pList->pulCurrent++ = 0;
    }

    BCFC_PrintFloatCsc3x3_isrsafe(pCsc);
    BCFC_PrintCsc3x3_isrsafe(pList->pulCurrent - 12, ulCfg);
}

static const uint8_t s_aLRAdjHeaderSize[] =
{
    0, /* BCFC_LR_ADJ_LIMIT_NOT_EXIST */
    2, /* BCFC_LR_ADJ_LIMIT_ALWAYS_ON */
    3, /* BCFC_LR_ADJ_LIMIT_DISABLE */
    3  /* BCFC_LR_ADJ_LIMIT_ENABLE */
};

/* this function could be called to build CMP/GFD/VEC LRange adjust
 */
void BVDC_P_Cfc_BuildRulForLRAdjLimit_isr(
    const BCFC_LRangeAdj            *pLRangeAdj,
    uint32_t                         ulStartReg,
    BVDC_P_ListInfo                 *pList)
{
    uint32_t ulLRangeAdjCtrl = pLRangeAdj->ulLRangeAdjCtrl;
    uint32_t ulEnLimit = (ulLRangeAdjCtrl == BCFC_LR_ADJ_LIMIT_DISABLE || ulLRangeAdjCtrl == BCFC_LR_ADJ_LIMIT_NOT_EXIST)? 0 : 1;

    if (ulLRangeAdjCtrl >= BCFC_LR_ADJ_LIMIT_ALWAYS_ON)
    {
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(s_aLRAdjHeaderSize[ulLRangeAdjCtrl]);
        /* use reverse order based on mode since some chips don't have the first limit_en register */
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg - sizeof(uint32_t) * (s_aLRAdjHeaderSize[ulLRangeAdjCtrl]-1));
        if (ulLRangeAdjCtrl >= BCFC_LR_ADJ_LIMIT_DISABLE)
        {
            *pList->pulCurrent++ = ulEnLimit;
        }
        *pList->pulCurrent++ = pLRangeAdj->ulMax;
        *pList->pulCurrent++ = pLRangeAdj->ulMin;
        BDBG_MODULE_MSG(BVDC_CFC_3,("   LRangeAdj limitEn %d, max 0x%8x, min 0x%8x:",
           ulEnLimit, pLRangeAdj->ulMin, pLRangeAdj->ulMax));
    }
}

/* this function could be called to build CMP/GFD/VEC LRange adjust
 */
void BVDC_P_Cfc_BuildRulForLRAdj_isr(
    const BCFC_LRangeAdj            *pLRangeAdj,
    bool                             bDispAdaptation,
    int16_t                          sHdrPeakBrightness,
    uint32_t                         ulStartReg,
    BVDC_P_ListInfo                 *pList)
{
    int ii;
    const BCFC_LRangeAdjTable *pTable = (pLRangeAdj->pTable)? pLRangeAdj->pTable : &s_LRangeAdj_Identity;
    uint32_t ulNewXY=0;

    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(2 * BCFC_LR_ADJ_PTS);
    *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
    for (ii=0; ii<BCFC_LR_ADJ_PTS; ii++)
    {
        *pList->pulCurrent++ = pTable->aulLRangeAdjSlope[ii];
    }
    for (ii=0; ii<BCFC_LR_ADJ_PTS; ii++)
    {
        /* when display adaptation is set, need to scale Y1 value with user */
        /* brightness setting for HDR, normalized at 1000 */
        if(bDispAdaptation && ii == 1)
        {
            uint32_t ulXY = pTable->aulLRangeAdjXY[ii];
            uint32_t ulY = (ulXY & BCFC_P_LR_Y_MASK) >> BCFC_P_LR_Y_SHIFT;
            ulY = ulY * sHdrPeakBrightness / BVDC_P_TCH_DEFAULT_HDR_BRIGHTNESS;
            ulNewXY = (ulXY & ~BCFC_P_LR_Y_MASK) | (ulY << BCFC_P_LR_Y_SHIFT);
            BDBG_MSG(("Implement display adaption for sUserDisplayBrightness=%d => 0x%x",
                sHdrPeakBrightness, ulNewXY));

            *pList->pulCurrent++ = ulNewXY;
        }
        else
        {
            *pList->pulCurrent++ = pTable->aulLRangeAdjXY[ii];
        }
    }
    BDBG_MODULE_MSG(BVDC_CFC_3,("   LRangeAdj (xy, slope_m_e)[8]:"));
    for (ii=0; ii<BCFC_LR_ADJ_PTS; ii++)
    {
        if(bDispAdaptation && ii == 1)
        {
            BDBG_MODULE_MSG(BVDC_CFC_3,("   (0x%08x, 0x%08x)", ulNewXY, pTable->aulLRangeAdjSlope[ii]));
        }
        else
        {
            BDBG_MODULE_MSG(BVDC_CFC_3,("   (0x%08x, 0x%08x)", pTable->aulLRangeAdjXY[ii], pTable->aulLRangeAdjSlope[ii]));
        }
    }
}

#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */

#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) || ((BVDC_P_CMP_CFC_VER > BVDC_P_CFC_VER_2) && BVDC_P_SUPPORT_VFC)
/* init 7271 a0 CMP0 CFC ram lut NL2L for converting HLG to HDR10
 * init 7271 a0 GFD0 CFC ram lut L2NL for converting SDR to HLG (as HLG video bypass)
 * init 7271 b0 VFC  CFC ram lut NL2L for converting HLG to HDR10
 * note: it will init RamLut registers, should only be called during the CFC's init time
 * OK from either user mode or _isr mode, but NOT both!
 */
static void BVDC_P_Cfc_InitRamLut_isrsafe(
    const BCFC_RamLut               *pRamLut,
    BREG_Handle                      hRegister,
    uint32_t                         ulLutReg,
    uint32_t                         ulCtrlReg )
{
    uint32_t ii, ulRegVal;
    uint32_t ulNumSeg = pRamLut->ucNumSeg;
    uint32_t ulNumEntry = pRamLut->usSegBinEnd[ulNumSeg - 1] + 1;

    for(ii = 0; ii < ulNumEntry; ii++)
    {
        BREG_Write32(hRegister, ulLutReg + ii*4, pRamLut->pulTable[ii]);
    }

#if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
    ulRegVal =
        BCHP_FIELD_DATA(HDR_CMP_0_V0_NL_LUT_CTRL, NL_READ_RAM_SEL,    0) |
        BCHP_FIELD_DATA(HDR_CMP_0_V0_NL_LUT_CTRL, NL_LUT_NUM_SEG,     ulNumSeg) |
        BCHP_FIELD_DATA(HDR_CMP_0_V0_NL_LUT_CTRL, NL_LUT_OINT_BITS,   pRamLut->ucOutIntBits) |
        BCHP_FIELD_DATA(HDR_CMP_0_V0_NL_LUT_CTRL, NL_LUT_XSCL,        pRamLut->ucXScale); /* 1.000000 => b100 in U1.2 */
    BREG_Write32(hRegister, ulCtrlReg, ulRegVal);

    for(ii = 0; ii < ulNumSeg; ii++)
    {
        ulRegVal =
            BCHP_FIELD_DATA(HDR_CMP_0_V0_NL_LUT_SEG_CTRLi, NL_LUT_SEG_END,        pRamLut->usSegBinEnd[ii]) |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_NL_LUT_SEG_CTRLi, NL_LUT_SEG_INT_OFFSET, pRamLut->usSegOffset[ii]) |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_NL_LUT_SEG_CTRLi, NL_LUT_SEG_INT_BITS,   pRamLut->usSegIntBits[ii]);
        BREG_Write32(hRegister, ulCtrlReg + ii*sizeof(uint32_t) + sizeof(uint32_t), ulRegVal);
    }
#else
    ulRegVal =
        BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_CTRL, LUT_NUM_SEG,     ulNumSeg) |
        BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_CTRL, LUT_OINT_BITS,   pRamLut->ucOutIntBits) |
        BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_CTRL, LUT_XSCL,        pRamLut->ucXScale); /* 1.000000 => b100 in U1.2 */
    BREG_Write32(hRegister, ulCtrlReg, ulRegVal);

    for(ii = 0; ii < ulNumSeg; ii++)
    {
        ulRegVal =
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_SEG_CTRLi, LUT_SEG_END,        pRamLut->usSegBinEnd[ii]) |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_SEG_CTRLi, LUT_SEG_INT_OFFSET, pRamLut->usSegOffset[ii]) |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_SEG_CTRLi, LUT_SEG_INT_BITS,   pRamLut->usSegIntBits[ii]);
        BREG_Write32(hRegister, ulCtrlReg + ii*sizeof(uint32_t) + sizeof(uint32_t), ulRegVal);
    }
#endif
}
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)

#define BVDC_P_LUT_SEGS               (7)
#define BVDC_P_SEG_CTRL_REGS          (1)
#define BVDC_P_LUT_HEAD_CTRL_REGS     (1)
#define BVDC_P_LUT_CTRL_REGS          (BVDC_P_LUT_HEAD_CTRL_REGS + BVDC_P_LUT_SEGS * BVDC_P_SEG_CTRL_REGS)
#define BVDC_P_MOSAIC_NL2L_LUT_SIZE   (300)
#define BVDC_P_LMR_ADJ_REGS           (2)
#define BVDC_P_LMR_MB2_REGS           (4)
#define BVDC_P_LMR_CTRL_REGS          (BVDC_P_LMR_ADJ_REGS + BVDC_P_LUT_CTRL_REGS)
#define BVDC_P_MOSAIC_LMR_LUT_SIZE    (64)

#ifndef BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I0
#define BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I0  (BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR)
#define BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I0  (BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR)
#endif
#ifndef BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I1
#define BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I1  (BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I0 + 1)
#define BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I2  (BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I0 + 2)
#define BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I3  (BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_DLBV_COMP3 + 1)
#define BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I1  (BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I0 + 1)
#define BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I2  (BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I0 + 2)
#define BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I3  (BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I0 + 3)
#endif
#ifndef BCHP_HDR_CMP_0_V0_R0_MB2_COEF_C00
#define BCHP_HDR_CMP_0_V0_R0_MB2_COEF_C00                   (BCHP_HDR_CMP_0_V0_MB2_COEF_C00)
#endif

/* Build LUT RUL for RAM (NL2L/L2NL/LMR) loading and RDC RUL for usage control
 */
void BVDC_P_Cfc_BuildRulForRamLut_isr
    ( const BCFC_RamLut          *pRamLut,
      uint32_t                    ulStartReg,
      uint32_t                    ulLutId,
      BCFC_LutLoadListInfo       *pLutList,
      BVDC_P_ListInfo            *pList)
{
    uint8_t ucEntryWidth;
    uint32_t ulRamOffset;
    uint32_t ulNumSeg = pRamLut->ucNumSeg;
    uint32_t ulNumEntry = pRamLut->usSegBinEnd[ulNumSeg - 1] + 1;
    int ii;

    BDBG_ASSERT(ulNumSeg <= BVDC_P_LUT_SEGS);

    switch (ulLutId) {
    case BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_L2NL:
    case BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_L2NL:
    case BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_VEC_L2NL:
    case BCHP_GFD_TYPE_LUT_HEADER_W0_LUT_ID_G0_L2NL:

    case BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_TP_LUTP:
    case BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_TP_LUTC:
    case BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_TP_LUTI:
    case BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_TP_LUTS:

    case BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_DLBV_CVM:
    case BCHP_GFD_TYPE_LUT_HEADER_W0_LUT_ID_G0_DLBV_CVM:
        ucEntryWidth = sizeof(uint16_t);
        break;

    default:
        ucEntryWidth = sizeof(uint32_t);
        break;
    }

    /* only NL2L/LMR have RAM_OFFSET. The others don't and should have value 0 in the field */
    if (/*ulLutId >= BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_NL2L_I0 &&*/ ulLutId <= BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_NL2L_I3)
    {
        ulRamOffset = BVDC_P_MOSAIC_NL2L_LUT_SIZE * (ulLutId - BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_NL2L_I0);
    }
    else if (ulLutId >= BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_NL2L_I0 && ulLutId <= BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_NL2L_I3)
    {
        ulRamOffset = BVDC_P_MOSAIC_NL2L_LUT_SIZE * (ulLutId - BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_NL2L_I0);
    }
    else if (ulLutId == BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I3)
    {
        ulRamOffset = BVDC_P_MOSAIC_LMR_LUT_SIZE * 3;
    }
    else if (ulLutId >= BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I0 && ulLutId <= BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I2)
    {
        ulRamOffset = BVDC_P_MOSAIC_LMR_LUT_SIZE * (ulLutId - BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I0);
    }
    else if (ulLutId >= BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I0 && ulLutId <= BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I3)
    {
        ulRamOffset = BVDC_P_MOSAIC_LMR_LUT_SIZE * (ulLutId - BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I0);
    }
    else
    {
        ulRamOffset = 0;
    }

    if (ulLutId == BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_VEC_NL2L || ulLutId == BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_VEC_L2NL)
    {
        /* build RDC rul for ram lut ctrl: dvi-cfc put ram lut per seg control seperate from header */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(DVI_CFC_0_NL2L_LUT_CTRL, LUT_NUM_SEG,     ulNumSeg) |
            BCHP_FIELD_DATA(DVI_CFC_0_NL2L_LUT_CTRL, LUT_OINT_BITS,   pRamLut->ucOutIntBits) |
            BCHP_FIELD_DATA(DVI_CFC_0_NL2L_LUT_CTRL, LUT_XSCL,        pRamLut->ucXScale);

        ulStartReg = (ulLutId == BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_VEC_L2NL)?
            ulStartReg + (BCHP_DVI_CFC_0_L2NL_LUT_SEG_CTRLi_ARRAY_BASE - BCHP_DVI_CFC_0_L2NL_LUT_CTRL) :
            ulStartReg + (BCHP_DVI_CFC_0_NL2L_LUT_SEG_CTRLi_ARRAY_BASE - BCHP_DVI_CFC_0_NL2L_LUT_CTRL);
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(ulNumSeg * BVDC_P_SEG_CTRL_REGS);
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
        for (ii=0; ii<(int)ulNumSeg; ii++)
        {
            *pList->pulCurrent++ =
                BCHP_FIELD_DATA(DVI_CFC_0_NL2L_LUT_SEG_CTRLi, LUT_SEG_INT_BITS,   pRamLut->usSegIntBits[ii]) |
                BCHP_FIELD_DATA(DVI_CFC_0_NL2L_LUT_SEG_CTRLi, LUT_SEG_INT_OFFSET, pRamLut->usSegOffset[ii]) |
                BCHP_FIELD_DATA(DVI_CFC_0_NL2L_LUT_SEG_CTRLi, LUT_SEG_END,        pRamLut->usSegBinEnd[ii]);
        }
    }
    else
    {
        /* build RDC rul for ram lut ctrl */
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_LUT_HEAD_CTRL_REGS + ulNumSeg * BVDC_P_SEG_CTRL_REGS);
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
        *pList->pulCurrent++ =
          #if (BVDC_P_CMP_0_TF_CONV_CFCS > 1)
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_CTRL, NL2L_RAM_OFFSET, ulRamOffset) |
          #endif
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_CTRL, LUT_NUM_SEG,     ulNumSeg) |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_CTRL, LUT_OINT_BITS,   pRamLut->ucOutIntBits) |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_CTRL, LUT_XSCL,        pRamLut->ucXScale);
        for (ii=0; ii<(int)ulNumSeg; ii++)
        {
            *pList->pulCurrent++ =
                BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_SEG_CTRLi, LUT_SEG_INT_BITS,   pRamLut->usSegIntBits[ii]) |
                BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_SEG_CTRLi, LUT_SEG_INT_OFFSET, pRamLut->usSegOffset[ii]) |
                BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL2L_LUT_SEG_CTRLi, LUT_SEG_END,        pRamLut->usSegBinEnd[ii]);
        }
    }

    /* build LUT rul for ram lut loading */
    *pLutList->pulCurrent++ = /* header W0 */
        BCHP_FIELD_ENUM(CMP_0_HDR_TYPE_LUT_HEADER_W0, OPCODE,      INSTR) |
        BCHP_FIELD_DATA(CMP_0_HDR_TYPE_LUT_HEADER_W0, ENTRY_WIDTH, ucEntryWidth) |
        BCHP_FIELD_DATA(CMP_0_HDR_TYPE_LUT_HEADER_W0, LUT_ID,      ulLutId);
    *pLutList->pulCurrent++ = /* header W1 */
        BCHP_FIELD_DATA(CMP_0_HDR_TYPE_LUT_HEADER_W1, ENTRY_CNT,  ulNumEntry) |
        BCHP_FIELD_DATA(CMP_0_HDR_TYPE_LUT_HEADER_W1, ADR_OFFSET, ulRamOffset);
    if (ucEntryWidth == sizeof(uint32_t))
    {
        BKNI_Memcpy((void *)(pLutList->pulCurrent), pRamLut->pulTable, ulNumEntry * ucEntryWidth);
        pLutList->pulCurrent += ulNumEntry;
    }
    else
    {
        int iNumWords = ulNumEntry / 2;
        for (ii=0; ii<iNumWords; ii++)
        {
            *pLutList->pulCurrent++ =
                BCHP_FIELD_DATA(CMP_0_HDR_TYPE_LUT_NARROW_DATA, WORD0, *(pRamLut->pulTable+2*ii)) |
                BCHP_FIELD_DATA(CMP_0_HDR_TYPE_LUT_NARROW_DATA, WORD1, *(pRamLut->pulTable+2*ii+1));
        }
        if ((ulNumEntry - 2 * iNumWords) > 0)
        {
            *pLutList->pulCurrent++ =
                BCHP_FIELD_DATA(CMP_0_HDR_TYPE_LUT_NARROW_DATA, WORD0, *(pRamLut->pulTable+2*ii));
        }
    }
    BDBG_MODULE_MSG(BVDC_CFC_4,("   Build LutLoading: ulLutId %d, ulNumEntry %d, ucEntryWidth %d", ulLutId, ulNumEntry, ucEntryWidth));
}

/* Build RUL for LMR MB2 and LMR_A/B/C
 */
static void BVDC_P_Cfc_BuildRulForLmrAdj_isr
    ( BCFC_Context               *pCfc,
      uint32_t                    ulLmrMb2StartReg,
      uint32_t                    ulLmrAdjStartReg,
      BVDC_P_ListInfo            *pList)
{

    int jj;
    const BCFC_Csc3x4 *pMc;
    BCFC_Csc3x4 stMb2, stTmp;
    uint32_t ulMask, ulCxFBits, ulCoFBits;

    /* Mb2 */
    pMc = BCFC_GetCsc3x4_Mc_isrsafe(pCfc->pColorSpaceExtOut->stColorSpace.eColorimetry);
    BCFC_Csc_Mult_isrsafe(&(BCFC_GetCsc3x4_YCbCr_Limited_to_Full_isrsafe()->m[0][0]), 4, &(pMc->m[0][0]), 4, &(stTmp.m[0][0]));
    BCFC_Csc_Mult_isrsafe(&(stTmp.m[0][0]), 4, &(pCfc->stMb.m[0][0]), 3, &(stMb2.m[0][0]));
    ulMask = BCFC_CSC_MASK(BCFC_CscType_eMb3x4_25);
    ulCxFBits = BCFC_CSC_CX_F_BITS(BCFC_CscType_eMb3x4_25);
    ulCoFBits = BCFC_CSC_CO_F_BITS(BCFC_CscType_eMb3x4_25);
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_LMR_MB2_REGS);
    *pList->pulCurrent++ = BRDC_REGISTER(ulLmrMb2StartReg);
    for (jj=0; jj<3; jj++)
    {
        *pList->pulCurrent++ = BVDC_P_MAKE_HW_CX(stMb2.m[0][jj]);
    }
    *pList->pulCurrent++ = BVDC_P_MAKE_HW_CO(stMb2.m[0][3]);

    /* LMR_A / B / C */
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_LMR_ADJ_REGS);
    *pList->pulCurrent++ = BRDC_REGISTER(ulLmrAdjStartReg);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_LMR_MULT_COEF, LMR_A, *(pCfc->pTfConvRamLuts->pulLmrAdj + 0)) |
        BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_LMR_MULT_COEF, LMR_B, *(pCfc->pTfConvRamLuts->pulLmrAdj + 1));
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_LMR_ADD_COEFF, LMR_C, *(pCfc->pTfConvRamLuts->pulLmrAdj + 2));
}

#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */

#if defined(BCHP_HDR_CMP_0_V0_R0_LR_SLOPEi_ARRAY_BASE)
#define BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_BASE   BCHP_HDR_CMP_0_V0_R0_LR_SLOPEi_ARRAY_BASE
#define BCHP_HDR_CMP_0_V1_R0_NL_LR_SLOPEi_ARRAY_BASE   BCHP_HDR_CMP_0_V1_R0_LR_SLOPEi_ARRAY_BASE
#endif

#if defined(BCHP_HDR_CMP_0_V0_R1_LR_SLOPEi_ARRAY_BASE)
#define BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_ARRAY_BASE   BCHP_HDR_CMP_0_V0_R1_LR_SLOPEi_ARRAY_BASE
#endif

/* Build RUL for a mosaic-rect / cfc inside a compositor.
 */
void BVDC_P_Window_BuildCfcRul_isr
    ( BVDC_Window_Handle               hWindow,
      uint32_t                         ulRectIdx, /* Mosaic rect index */
      BVDC_P_ListInfo                 *pList)
{
    uint32_t ulStartReg;
    uint32_t ulV0V1Offset = 0; /* main vs pip reg offset */
    uint8_t ucCfcIdxForRect;
    BCFC_Context *pCfc;
    BVDC_Compositor_Handle hCompositor;
    BVDC_WindowId eWinInCmp;
    bool bBuildCfcRul;
    BCFC_LeftShift eLeftShift = BCFC_LeftShift_eNotExist;
    BCFC_ColorSpaceExt *pColorSpaceExtIn;
    BCFC_ColorSpaceExt *pColorSpaceExtOut;
#if BVDC_P_TCH_SUPPORT
    const BVDC_P_CfcMetaData *pMetaData;
#endif
    bool bDispAdaptation = false;
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_1)
    bool bBypassCfc;
#endif
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2)
    BCFC_CscType eMaCscType = BCFC_CscType_eMa3x4_25;
#endif

    BDBG_OBJECT_ASSERT(hWindow, BVDC_WIN);

    hCompositor = hWindow->hCompositor;
    eWinInCmp = hWindow->eId - BVDC_P_CMP_GET_V0ID(hCompositor);

    ucCfcIdxForRect = hWindow->aucMosaicCfcIdxForRect[ulRectIdx];
    pCfc = &(hWindow->astMosaicCfc[ucCfcIdxForRect]);
    pColorSpaceExtOut = pCfc->pColorSpaceExtOut;
    pColorSpaceExtIn = &(pCfc->stColorSpaceExtIn);

    if ((BCFC_ColorFormat_eInvalid == pColorSpaceExtOut->stColorSpace.eColorFmt) ||
        (BCFC_ColorFormat_eInvalid == pColorSpaceExtIn->stColorSpace.eColorFmt))
    {
        /* input colorSpace or output ColorSpace not set yet */
        return;
    }

    bBuildCfcRul = (pCfc->ucRulBuildCntr >= hCompositor->ulCscAdjust[hWindow->eId]);
    if (bBuildCfcRul)
    {
        pCfc->ucRulBuildCntr--;
    }

#if BVDC_P_DBV_SUPPORT
    /* DBV will compose video inputs together */
    if (pCfc->stCapability.stBits.bDbvCmp && (!pCfc->stForceCfg.stBits.bDisableDolby) &&
        BVDC_P_CMP_DBV_MODE(hCompositor))
    {
        /* TODO: add DBV enhancement layer */
        if(eWinInCmp == BVDC_WindowId_eVideo0) {
            pCfc->bBlendInMatrixOn = false; /* clear it for now in case of dbv */
            BVDC_P_Compositor_BuildDbvRul_isr(hCompositor, pList);
        }
        return; /* skip the rest */
    }
#endif
#if BVDC_P_TCH_SUPPORT
    pMetaData = (BVDC_P_CfcMetaData *)pColorSpaceExtIn->stColorSpace.stMetadata.pDynamic;
    if(pCfc->stCapability.stBits.bTpToneMapping && (!pCfc->stForceCfg.stBits.bDisableTch) &&
       hWindow->eId == BVDC_P_WindowId_eComp0_V0 &&
       pMetaData && BCFC_IS_TCH(pMetaData->stTchInput.stHdrMetadata.eType))
    {
        BVDC_P_Compositor_BuildTchRul_isr(hCompositor, pList);
        /* Need to set flag for display adaptation if user sets different */
        /* brightness for HDR.  Default values for LRangeAdj is for brightness */
        /* level 1000 */
        if(BCFC_IS_SDR_TO_HDR10(pCfc->stColorSpaceExtIn.stColorSpace.eColorTF, pCfc->pColorSpaceExtOut->stColorSpace.eColorTF) &&
           hWindow->stCurInfo.sHdrPeakBrightness != BVDC_P_TCH_DEFAULT_HDR_BRIGHTNESS)
        {
            bDispAdaptation = true;
        }
    }
#else
    BSTD_UNUSED(bDispAdaptation);
#endif

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_1)
    bBypassCfc = hWindow->bBypassCmpCsc || pCfc->bBypassCfc;

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2)
    /* note: 7271 B0 CMP0-V0 Cfc4...7 has Mb, but no LRngAdj */
    if (pCfc->stCapability.stBits.bMb)
    {
        uint32_t ulNLCfg;
        uint32_t ulNL2L = (pCfc->bForceBypassTfTbl)? BCFC_NL2L_BYPASS : pColorSpaceExtIn->stCfg.stBits.SelTF;
        uint32_t ulL2NL = (pCfc->bForceBypassTfTbl)? BCFC_L2NL_BYPASS : pColorSpaceExtOut->stCfg.stBits.SelTF;

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
/* 7271 B */
        uint32_t ulLutId;
        uint32_t ulSelLRangeAdj = (pCfc->stCapability.stBits.bLRngAdj  && !bBypassCfc)?
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LRANGE_ADJ, ucCfcIdxForRect) :
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LRANGE_ADJ, DISABLE);
      #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_4)
        uint32_t ulSelLmr = (pCfc->stCapability.stBits.bLMR  && (NULL != pCfc->pTfConvRamLuts->pRamLutLMR) && !bBypassCfc)?
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LMR, ucCfcIdxForRect) :
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LMR, DISABLE);
      #else
        uint32_t ulSelLmr = 0;
      #endif
        eMaCscType = BCFC_CscType_eMa5x4_25;
        eLeftShift = BCFC_LeftShift_eOff;

        if ((!bBypassCfc) && (ulL2NL == BCFC_L2NL_RAM) && (pCfc->pTfConvRamLuts->pRamLutL2NL == NULL))
        {
            /* pColorSpaceExtOut->stCfg.stBits.SelTF likely set to BCFC_L2NL_RAM by GFX cfc */
            BDBG_ERR(("Cmp%d_V%d-Rect%d fail to load RAM L2NL, likely hCfcHeap passded from nexus is NULL", hCompositor->eId, eWinInCmp, ulRectIdx));
            BDBG_ASSERT(pCfc->pTfConvRamLuts->pRamLutL2NL);
        }

        /* BCHP_HDR_CMP_0_V0_CTRL is only related to CMP output, so there is only one reg */
      #if BVDC_P_DBV_SUPPORT
        /* if input is DBV, this register is controlled by DBV; skip here;
           Note, mosaic mode non-DBV context can reach here;
           TODO: mosaic non-DBV context needs to have Mc to convert to IPT color like DBV context; */
        if(!hCompositor->pstDbv || !hCompositor->pstDbv->metadataPresent || pCfc->stForceCfg.stBits.bDisableDolby)
      #endif
        {
          #ifdef BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_BASE
            ulV0V1Offset = eWinInCmp * (BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_BASE -
                BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE);
          #endif
            ulNLCfg = (bBypassCfc)?
                BCHP_FIELD_ENUM(HDR_CMP_0_CMP_HDR_V0_CTRL, SEL_L2NL,       BYPASS) |
                BCHP_FIELD_ENUM(HDR_CMP_0_CMP_HDR_V0_CTRL, SEL_XVYCC_L2NL, DEFAULT) /* 0xa0 */
                :
                BCHP_FIELD_DATA(HDR_CMP_0_CMP_HDR_V0_CTRL, SEL_L2NL,       ulL2NL) |
              #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_3)
                BCHP_FIELD_DATA(HDR_CMP_0_CMP_HDR_V0_CTRL, LMR_ADJ_EN,     (pCfc->pTfConvRamLuts->pRamLutLMR)? 1 : 0) |
              #endif
                BCHP_FIELD_DATA(HDR_CMP_0_CMP_HDR_V0_CTRL, TP_TONE_MAP_EN, pColorSpaceExtIn->stCfg.stBits.bEnTpToneMap) |
                BCHP_FIELD_DATA(HDR_CMP_0_CMP_HDR_V0_CTRL, SEL_XVYCC_L2NL, pColorSpaceExtOut->stCfg.stBits.bSelXvYcc);
            ulStartReg = BCHP_HDR_CMP_0_CMP_HDR_V0_CTRL + ulV0V1Offset + hCompositor->ulRegOffset;
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
            *pList->pulCurrent++ = ulNLCfg;
            BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d-Rect%d OUT HDR_V0_CTRL 0x%08x", hCompositor->eId, eWinInCmp, ulRectIdx, ulNLCfg));
        }

        ulNLCfg = (bBypassCfc)?
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_MA_COEF,    ucCfcIdxForRect) | /* using MA-0 if disabled ??? */
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, DLBV_COMP_SEL,  DISABLE) |
            ulSelLmr                                                                     |
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LRANGE_ADJ, DISABLE) |
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_MB_COEF,    DISABLE) |
          #ifdef BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LR_LIMIT_SHIFT
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LR_LIMIT,   DISABLE)  |
          #endif
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_NL2L,       BYPASS)  |
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_CL_IN,      DEFAULT) |
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_XVYCC_NL2L, DEFAULT) /* 0xXX048050 */
            :
          #if (BVDC_P_CMP_0_TF_CONV_CFCS > 1)
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_NL2L_RAM_ADDR, (pCfc->pTfConvRamLuts->pRamLutNL2L)? ucCfcIdxForRect : 0) |
          #endif
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_MA_COEF,    ucCfcIdxForRect) |
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, DLBV_COMP_SEL,  DISABLE) | /* ucCfcIdxForRect if enabled */
            ulSelLmr                                                                     |
            ulSelLRangeAdj                                                               |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_MB_COEF,    ucCfcIdxForRect) |
          #ifdef BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LR_LIMIT_SHIFT
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_LR_LIMIT,   DISABLE)  |
          #endif
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_NL2L,       ulNL2L)  |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_CL_IN,      pColorSpaceExtIn->stCfg.stBits.bSelCL) |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, SEL_XVYCC_NL2L, pColorSpaceExtIn->stCfg.stBits.bSelXvYcc);
        ulStartReg = BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE + ulV0V1Offset + hCompositor->ulRegOffset;
        ulStartReg += ulRectIdx * sizeof(int32_t);
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
        *pList->pulCurrent++ = ulNLCfg;
        BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d-Rect%d NL_CONFIGi      0x%08x", hCompositor->eId, eWinInCmp, ulRectIdx, ulNLCfg));

        /* build rul for ram lut NL2L */
        if (pCfc->pTfConvRamLuts->pRamLutNL2L && pCfc->ucRamNL2LRulBuildCntr)
        {
            pCfc->ucRamNL2LRulBuildCntr--;

            ulLutId = (eWinInCmp)?
                BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_NL2L_I0 : BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_NL2L_I0;
            ulStartReg = BCHP_HDR_CMP_0_V0_R0_NL2L_LUT_CTRL + ulV0V1Offset + hCompositor->ulRegOffset;
            ulLutId += ucCfcIdxForRect;
            ulStartReg += (ucCfcIdxForRect * BVDC_P_LUT_CTRL_REGS * sizeof(uint32_t));
            BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d-Cfc%d uses RAM NL2L, lutId %d", hCompositor->eId, eWinInCmp, ucCfcIdxForRect, ulLutId));
            BVDC_P_Cfc_BuildRulForRamLut_isr(pCfc->pTfConvRamLuts->pRamLutNL2L, ulStartReg, ulLutId, &hCompositor->stCfcLutList, pList);
        }

        /* build rul for LMR */
        if (pCfc->pTfConvRamLuts->pRamLutLMR && pCfc->ucRamLMRRulBuildCntr)
        {
            uint32_t ulMb2StartReg, ulAdjStartReg, ulCfcOffset, ulMb2CfcOffset;

            pCfc->ucRamLMRRulBuildCntr--;

            ulCfcOffset = (ucCfcIdxForRect * BVDC_P_LMR_CTRL_REGS * sizeof(uint32_t));
            ulMb2CfcOffset = (ucCfcIdxForRect * BVDC_P_LMR_MB2_REGS * sizeof(uint32_t));
            ulAdjStartReg = BCHP_HDR_CMP_0_V0_R0_LMR_MULT_COEF + ulCfcOffset + ulV0V1Offset + hCompositor->ulRegOffset;
            ulMb2StartReg = BCHP_HDR_CMP_0_V0_R0_MB2_COEF_C00 + ulMb2CfcOffset + ulV0V1Offset + hCompositor->ulRegOffset;
            BVDC_P_Cfc_BuildRulForLmrAdj_isr(pCfc, ulMb2StartReg, ulAdjStartReg, pList);

            ulLutId = (eWinInCmp)?
                BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_LMR_I0 : BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I0;
            ulLutId = (eWinInCmp == 0 && ucCfcIdxForRect == 3)? BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_LMR_I3 : ulLutId + ucCfcIdxForRect;
            ulStartReg = BCHP_HDR_CMP_0_V0_R0_LMR_LUT_CTRL + ulCfcOffset + ulV0V1Offset + hCompositor->ulRegOffset;
            BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d-Cfc%d uses LMR, lutId %d", hCompositor->eId, eWinInCmp, ucCfcIdxForRect, ulLutId));
            BVDC_P_Cfc_BuildRulForRamLut_isr(pCfc->pTfConvRamLuts->pRamLutLMR, ulStartReg, ulLutId, &hCompositor->stCfcLutList, pList);
        }

        /* build rul for ram lut L2NL */
        if (pCfc->pTfConvRamLuts->pRamLutL2NL && pCfc->ucRamL2NLRulBuildCntr && (0==ulRectIdx))
        {
            pCfc->ucRamL2NLRulBuildCntr--;

            ulLutId = (eWinInCmp)?
                BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V1_L2NL : BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_V0_L2NL;
            ulStartReg = BCHP_HDR_CMP_0_V0_L2NL_LUT_CTRL + ulV0V1Offset + hCompositor->ulRegOffset;
            BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d-Cfc%d uses RAM L2NL, lutId %d", hCompositor->eId, eWinInCmp, ucCfcIdxForRect, ulLutId));
            BVDC_P_Cfc_BuildRulForRamLut_isr(pCfc->pTfConvRamLuts->pRamLutL2NL, ulStartReg, ulLutId, &hCompositor->stCfcLutList, pList);
        }

#elif (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */
/* 7271 A */
        int ii, jj;

        /* BCHP_HDR_CMP_0_CMP_HDR_V0_CTRL is only related to CMP output, so there is only one reg */
      #ifdef BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_BASE
        ulV0V1Offset = eWinInCmp * (BCHP_HDR_CMP_0_V1_R00_TO_R15_NL_CONFIGi_ARRAY_BASE -
            BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE);
      #endif

        /* Programming NL_CONFIG for this rect
         */
        ii = ulRectIdx / 2;
        jj = ulRectIdx - ii * 2;
        ulNLCfg = (bBypassCfc)?
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_L2NL,       BYPASS) |
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_LRANGE_ADJ, DISABLE)|
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_MB_COEF,    DISABLE)|
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_NL2L,       BYPASS) |
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_NL_CSC_EN,      BYPASS) /* 0x17fa */
            :
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_L2NL,       ulL2NL) |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_LRANGE_ADJ, ucCfcIdxForRect)                    |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_MB_COEF,    ucCfcIdxForRect)                    |
            BCHP_FIELD_DATA(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_SEL_NL2L,       ulNL2L)  |
            BCHP_FIELD_ENUM(HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi, RECT0_NL_CSC_EN,      ENABLE);
        hCompositor->aulNLCfg[eWinInCmp][ii] = (jj==0)? ulNLCfg :
            hCompositor->aulNLCfg[eWinInCmp][ii] | (ulNLCfg << BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_RECT1_NL_CSC_EN_SHIFT);

        ulStartReg = BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_ARRAY_BASE + ulV0V1Offset + hCompositor->ulRegOffset;
        ulStartReg += (ii * sizeof(int32_t));
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
        *pList->pulCurrent++ = hCompositor->aulNLCfg[eWinInCmp][ii];
        BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d-Rect%d NLCfg: this Cfc 0x%x, whole reg 0x%08x", hCompositor->eId, eWinInCmp, ulRectIdx, ulNLCfg, hCompositor->aulNLCfg[eWinInCmp][ii]));
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */

        /* programming HDR blending-in-matrix
         */
        if (ulRectIdx == 0)
        {
            bool bEnBlendMatrix = BVDC_P_CFC_NEED_BLEND_MATRIX(hCompositor);
            {
                ulStartReg = BCHP_HDR_CMP_0_V0_BLENDER_IN_CSC_EN + ulV0V1Offset + hCompositor->ulRegOffset;
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
                if (bEnBlendMatrix)
                {
                    const BCFC_Csc3x4 *pCsc;
                    *pList->pulCurrent++ = BCHP_FIELD_ENUM(HDR_CMP_0_V0_BLENDER_IN_CSC_EN, BLENDER_IN_CSC_ENABLE, ENABLE);
                    BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d: enable blend-in-matrices", hCompositor->eId, eWinInCmp));
                    pCsc = BCFC_GetCsc3x4_Ma_isrsafe(hCompositor->stOutColorSpaceExt.stColorSpace.eColorimetry);
                    ulStartReg += (BCHP_HDR_CMP_0_V0_BLENDER_IN_COEFF_C00 - BCHP_HDR_CMP_0_V0_BLENDER_IN_CSC_EN);
                    BVDC_P_Cfc_BuildRulForCscRx4_isr(
                        pCsc, NULL, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMc3x4_16, BCFC_LeftShift_eNotExist), ulStartReg, pList);
                }
                else
                {
                    *pList->pulCurrent++ = BCHP_FIELD_ENUM(HDR_CMP_0_V0_BLENDER_IN_CSC_EN, BLENDER_IN_CSC_ENABLE, DISABLE);
                    BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d: disable blend-in-matrices, due to %s",
                        hCompositor->eId, eWinInCmp, (!hCompositor->abBlenderUsed[0])? "no blending" : "non-HDR10 output"));
                }
                pCfc->bBlendInMatrixOn = bEnBlendMatrix;
            }
        }

        /* programming for this CFC
         */
        if (bBuildCfcRul)
        {
            /* Programming MA
             */
            BCFC_ColorSpace *pColorSpaceIn = &(pCfc->stColorSpaceExtIn.stColorSpace);
            ulStartReg = BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C00 + ulV0V1Offset + hCompositor->ulRegOffset;
          #if (BVDC_P_CMP_0_TF_CONV_CFCS > 1)
            ulStartReg += ucCfcIdxForRect * (BCHP_HDR_CMP_0_V0_R1_MA_COEFF_C00 - BCHP_HDR_CMP_0_V0_R0_MA_COEFF_C00);
          #endif
            BDBG_MODULE_MSG(BVDC_CFC_3,("Cmp%d_V%d-Cfc%d MA:", hCompositor->eId, eWinInCmp, ucCfcIdxForRect));
            if (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceIn->eColorFmt)
            {
                BVDC_P_Cfc_BuildRulForCscRx4_isr(
                    pCfc->pMa, &pColorSpaceExtIn->stMalt, BCFC_MAKE_CSC_CFG(eMaCscType, eLeftShift), ulStartReg, pList);
            }
            else
            {
                BVDC_P_Cfc_BuildRulForCscRx4_isr(
                    pCfc->pMa, NULL, BCFC_MAKE_CSC_CFG(eMaCscType, eLeftShift), ulStartReg, pList);
            }

            /* Programming MB
             */
            ulStartReg = BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C00 + ulV0V1Offset + hCompositor->ulRegOffset;
          #if (BVDC_P_CMP_0_TF_CONV_CFCS > 1)
            ulStartReg += ucCfcIdxForRect * (BCHP_HDR_CMP_0_V0_R1_MB_COEFF_C00 - BCHP_HDR_CMP_0_V0_R0_MB_COEFF_C00);
          #endif
            BDBG_MODULE_MSG(BVDC_CFC_3,("Cmp%d_V%d-Cfc%d MB:", hCompositor->eId, eWinInCmp, ucCfcIdxForRect));
            BVDC_P_Cfc_BuildRulForCsc3x3_isr(
                &pCfc->stMb, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMb3x4_25, eLeftShift), ulStartReg, pList);

            if (pCfc->stCapability.stBits.bLRngAdj)
            {
                /* Programming LRange Adj and NL_CSC_CTRL
                 */
              #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
                pCfc->stLRangeAdj.ulLRangeAdjCtrl = BCFC_LR_ADJ_LIMIT_NOT_EXIST;
              #elif (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) /* #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) */
              #ifdef BCHP_HDR_CMP_0_V0_R00_TO_R15_NL_CONFIGi_SEL_LR_LIMIT_SHIFT
                pCfc->stLRangeAdj.ulLRangeAdjCtrl = BCFC_LR_ADJ_LIMIT_ALWAYS_ON; /* controlled by NL_CONFIGi_SEL_LR_LIMIT */
              #else
                pCfc->stLRangeAdj.ulLRangeAdjCtrl = BCFC_LR_ADJ_LIMIT_DISABLE;
              #endif
                ulStartReg = BCHP_HDR_CMP_0_V0_R0_LR_MIN_LIMIT + ulV0V1Offset + hCompositor->ulRegOffset;
              #if (BVDC_P_CMP_0_TF_CONV_CFCS > 1)
                ulStartReg += ucCfcIdxForRect * (BCHP_HDR_CMP_0_V0_R1_LR_MIN_LIMIT - BCHP_HDR_CMP_0_V0_R0_LR_MIN_LIMIT);
              #endif
              #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
                if (ucCfcIdxForRect == 5)
                {
                    /* by accident, there is a 0x10 gap between HDR_CMP_0_V0_R4_NL_LR_XY_0[7] and HDR_CMP_0_V0_R4_NL_CSC_CTRL !!! */
                    ulStartReg = BCHP_HDR_CMP_0_V0_R5_LR_MIN_LIMIT + ulV0V1Offset + hCompositor->ulRegOffset;
                }
              #endif
              #ifdef BCHP_HDR_CMP_0_V0_R0_LR_LIMIT_ENABLE
                BDBG_ASSERT(pCfc->stLRangeAdj.ulLRangeAdjCtrl != BCFC_LR_ADJ_LIMIT_ALWAYS_ON);
              #else
                BDBG_ASSERT(pCfc->stLRangeAdj.ulLRangeAdjCtrl == BCFC_LR_ADJ_LIMIT_ALWAYS_ON);
              #endif
                BVDC_P_Cfc_BuildRulForLRAdjLimit_isr(&pCfc->stLRangeAdj, ulStartReg, pList);
              #endif /* #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) */

                ulStartReg = BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_BASE + ulV0V1Offset + hCompositor->ulRegOffset;
              #if (BVDC_P_CMP_0_TF_CONV_CFCS > 1)
                ulStartReg += ucCfcIdxForRect * (BCHP_HDR_CMP_0_V0_R1_NL_LR_SLOPEi_ARRAY_BASE - BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_BASE);
              #endif
              #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
                if (ucCfcIdxForRect == 5)
                {
                    /* by accident, there is a 0x10 gap between HDR_CMP_0_V0_R4_NL_LR_XY_0[7] and HDR_CMP_0_V0_R4_NL_CSC_CTRL !!! */
                    ulStartReg = BCHP_HDR_CMP_0_V0_R5_NL_LR_SLOPEi_ARRAY_BASE + ulV0V1Offset + hCompositor->ulRegOffset;
                }
              #endif
                BDBG_MODULE_MSG(BVDC_CFC_3,("Cmp%d_V%d-Cfc%d LRAdj:", hCompositor->eId, eWinInCmp, ucCfcIdxForRect));
                BVDC_P_Cfc_BuildRulForLRAdj_isr(&pCfc->stLRangeAdj, bDispAdaptation, hWindow->stCurInfo.sHdrPeakBrightness, ulStartReg, pList);

              #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
                ulStartReg += (BCHP_HDR_CMP_0_V0_R0_NL_CSC_CTRL - BCHP_HDR_CMP_0_V0_R0_NL_LR_SLOPEi_ARRAY_BASE);
                if (ucCfcIdxForRect == 4)
                {
                    /* by accident, there is a 0x10 gap between HDR_CMP_0_V0_R4_NL_LR_XY_0[7] and HDR_CMP_0_V0_R4_NL_CSC_CTRL !!!! */
                    ulStartReg = BCHP_HDR_CMP_0_V0_R4_NL_CSC_CTRL + ulV0V1Offset + hCompositor->ulRegOffset;
                }
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
                *pList->pulCurrent++ =
                    BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL_CSC_CTRL, SEL_CL_IN, pColorSpaceExtIn->stCfg.stBits.bSelCL) |
                    BCHP_FIELD_DATA(HDR_CMP_0_V0_R0_NL_CSC_CTRL, SEL_XVYCC, pColorSpaceExtIn->stCfg.stBits.bSelXvYcc | pColorSpaceExtOut->stCfg.stBits.bSelXvYcc);
              #endif /* #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2) */
            }
        }
    }
    else
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */
    /* 7271 CMP1, or 7439 B0 */
    if (pCfc->stCapability.stBits.bBlackBoxNLConv)
    {
    #ifdef BCHP_CMP_0_V0_NL_CSC_CTRL
        /* Programming NL_CSC_CTRL
         */
        uint32_t ulNumNLCtrlCnvBits, ulNewNLCtrl;

        ulNumNLCtrlCnvBits =  BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R1_SHIFT - BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_SHIFT;
        ulNewNLCtrl =
            ((pColorSpaceExtIn->stCfg.stBits.bSelXvYcc | pColorSpaceExtOut->stCfg.stBits.bSelXvYcc) << (BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_XVYCC_R0_SHIFT + ucCfcIdxForRect)) |
            (((BCFC_NL_SEL_BYPASS != pCfc->ucSelBlackBoxNL) && (!bBypassCfc))?
             ((1 << (BCHP_CMP_0_V0_NL_CSC_CTRL_NL_CSC_R0_SHIFT + ucCfcIdxForRect)) |
              (pCfc->ucSelBlackBoxNL << (BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_SHIFT + ucCfcIdxForRect * ulNumNLCtrlCnvBits))) : 0);
      #ifdef BCHP_CMP_0_V1_NL_CSC_CTRL
        ulV0V1Offset = eWinInCmp * (BCHP_CMP_0_V1_NL_CSC_CTRL - BCHP_CMP_0_V0_NL_CSC_CTRL);
      #endif
        ulStartReg = BCHP_CMP_0_V0_NL_CSC_CTRL + ulV0V1Offset + hCompositor->ulRegOffset;
        hCompositor->ulNLCscCtrl[eWinInCmp] = (ulRectIdx == 0)? ulNewNLCtrl :
            hCompositor->ulNLCscCtrl[eWinInCmp] | ulNewNLCtrl; /* |= prev calls with other cfcs */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
        *pList->pulCurrent++ = hCompositor->ulNLCscCtrl[eWinInCmp];
        BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d_V%d-Cfc%d CMP_CSC_NLCtrl: this Cfc 0x%x, whole reg 0x%08x",
            hCompositor->eId, eWinInCmp, ucCfcIdxForRect, ulNewNLCtrl, hCompositor->ulNLCscCtrl[eWinInCmp]));

      #if BCHP_CMP_0_V0_R0_MA_COEFF_C00 /* 7439 B0 CMP has Ma, 7172 A0/B0 CMP1 doesn't */
        if (pCfc->stCapability.stBits.bMa && bBuildCfcRul)
        {
            /* Programming MA
             */
            ulStartReg = BCHP_CMP_0_V0_R0_MA_COEFF_C00 + ulV0V1Offset + hCompositor->ulRegOffset;
            ulStartReg += ucCfcIdxForRect * (BCHP_CMP_0_V0_R1_MA_COEFF_C00 - BCHP_CMP_0_V0_R0_MA_COEFF_C00);
            BDBG_MODULE_MSG(BVDC_CFC_3,("Cmp%d_V%d-Cfc%d MA:", hCompositor->eId, eWinInCmp, ucCfcIdxForRect));
            BVDC_P_Cfc_BuildRulForCscRx4_isr(
                pCfc->pMa, NULL, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMa3x4_16, eLeftShift), ulStartReg, pList);
        }
      #endif /* #if BCHP_CMP_0_V0_R0_MA_COEFF_C00 */
    #endif /* #ifdef BCHP_CMP_0_V0_NL_CSC_CTRL */
    }

#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_1) */

    if (bBuildCfcRul)
    {
        /* Programming MC
         */
      #ifdef BCHP_CMP_0_V1_R0_MC_COEFF_C00
        ulV0V1Offset = eWinInCmp * (BCHP_CMP_0_V1_R0_MC_COEFF_C00 - BCHP_CMP_0_V0_R0_MC_COEFF_C00);
      #endif
        ulStartReg = BCHP_CMP_0_V0_R0_MC_COEFF_C00 + ulV0V1Offset + hCompositor->ulRegOffset;
      #ifdef BCHP_CMP_0_V0_R1_MC_COEFF_C00
        ulStartReg += (ucCfcIdxForRect * (BCHP_CMP_0_V0_R1_MC_COEFF_C00 - BCHP_CMP_0_V0_R0_MC_COEFF_C00));
      #endif
        BDBG_MODULE_MSG(BVDC_CFC_3,("Cmp%d_V%d-Cfc%d MC:", hCompositor->eId, eWinInCmp, ucCfcIdxForRect));
        BVDC_P_Cfc_BuildRulForCscRx4_isr(
            &pCfc->stMc, NULL, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMc3x4_16, eLeftShift), ulStartReg, pList);
    }

    return;
}

/* Build RUL for cfc inside VFC
 */
void BVDC_P_Vfc_BuildCfcRul_isr
    ( BVDC_P_Vfc_Handle                hVfc,
      BVDC_P_ListInfo                 *pList)
{
#if (BVDC_P_SUPPORT_VFC)
    BCFC_Context *pCfc = &hVfc->stCfc;
    BCFC_ColorSpaceExt *pColorSpaceExtIn = &(pCfc->stColorSpaceExtIn);
    BCFC_ColorSpaceExt *pColorSpaceExtOut = pCfc->pColorSpaceExtOut;
    BCFC_ColorSpace *pColorSpaceOut = &pColorSpaceExtOut->stColorSpace;
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
    BCFC_LeftShift eLeftShift = BCFC_LeftShift_eNotExist;
    uint32_t ulTmpStartReg;
    uint32_t ulStartReg = BCHP_VFC_0_NL_CSC_CTRL + hVfc->ulRegOffset;
#else
    BSTD_UNUSED(pList);
#endif
    if ((BCFC_ColorFormat_eInvalid == pColorSpaceOut->eColorFmt) ||
        (BCFC_ColorFormat_eInvalid == pColorSpaceExtIn->stColorSpace.eColorFmt) ||
        (0 == pCfc->ucRulBuildCntr))
    {
        /* input colorSpace or output ColorSpace not set yet, or no change */
        return;
    }
    pCfc->ucRulBuildCntr--;

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
/* 7271 B */
    /* must have ulStartReg = BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL in this case */
    if (pCfc->stCapability.stBits.bLRngAdj) /* bSupportTfConv */
    {
        uint32_t ulNLCfg;
        uint32_t ulNL2L = (pCfc->bForceBypassTfTbl)? BCFC_NL2L_BYPASS : pColorSpaceExtIn->stCfg.stBits.SelTF;
        uint32_t ulL2NL = (pCfc->bForceBypassTfTbl)? BCFC_L2NL_BYPASS : pColorSpaceExtOut->stCfg.stBits.SelTF;
        bool bBypassCfc = pCfc->bBypassCfc;

        /* set eLeftShift = BCFC_LeftShift_e1/2_0 if needed */
        ulNLCfg = (bBypassCfc)?
            BCHP_FIELD_ENUM(VFC_0_NL_CSC_CTRL, CSC_MC_ENABLE,   DISABLE) |
            BCHP_FIELD_ENUM(VFC_0_NL_CSC_CTRL, CSC_MB_ENABLE,   DISABLE) |
            BCHP_FIELD_ENUM(VFC_0_NL_CSC_CTRL, CSC_MA_ENABLE,   DISABLE) |
            BCHP_FIELD_ENUM(VFC_0_NL_CSC_CTRL, LRANGE_ADJ_EN,   DISABLE) |
            BCHP_FIELD_ENUM(VFC_0_NL_CSC_CTRL, SEL_L2NL,        BYPASS)  |
            BCHP_FIELD_ENUM(VFC_0_NL_CSC_CTRL, SEL_NL2L,        BYPASS)  |
            BCHP_FIELD_DATA(VFC_0_NL_CSC_CTRL, SEL_CL_IN,            0) /* 0x550 */
            :
            BCHP_FIELD_ENUM(VFC_0_NL_CSC_CTRL, CSC_MC_ENABLE,   ENABLE)  |
            BCHP_FIELD_ENUM(VFC_0_NL_CSC_CTRL, CSC_MB_ENABLE,   ENABLE)  |
            BCHP_FIELD_ENUM(VFC_0_NL_CSC_CTRL, CSC_MA_ENABLE,   ENABLE)  |
            BCHP_FIELD_ENUM(VFC_0_NL_CSC_CTRL, LRANGE_ADJ_EN,   ENABLE)  |
            BCHP_FIELD_DATA(VFC_0_NL_CSC_CTRL, SEL_L2NL,        ulL2NL) |
            BCHP_FIELD_DATA(VFC_0_NL_CSC_CTRL, SEL_NL2L,        ulNL2L)  |
            BCHP_FIELD_DATA(VFC_0_NL_CSC_CTRL, SEL_CL_IN,       pColorSpaceExtIn->stCfg.stBits.bSelCL) |
            BCHP_FIELD_DATA(VFC_0_NL_CSC_CTRL, SEL_XVYCC_L2NL,  pColorSpaceExtOut->stCfg.stBits.bSelXvYcc) |
            BCHP_FIELD_DATA(VFC_0_NL_CSC_CTRL, SEL_XVYCC_NL2L,  pColorSpaceExtIn->stCfg.stBits.bSelXvYcc);
        BDBG_MODULE_MSG(BVDC_CFC_2,("VFC%d-Cfc NL_CSC_CTRL 0x%08x:", hVfc->eId, ulNLCfg));
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG( );
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
        *pList->pulCurrent++ = ulNLCfg;

        /* Programming MA
         */
        ulTmpStartReg = ulStartReg + (BCHP_VFC_0_CSC_R0_MA_COEFF_C00 - BCHP_VFC_0_NL_CSC_CTRL);
        BDBG_MODULE_MSG(BVDC_CFC_3,("VFC%d-Cfc MA:", hVfc->eId));
        BVDC_P_Cfc_BuildRulForCscRx4_isr(
            pCfc->pMa, NULL, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMa5x4_25, eLeftShift), ulTmpStartReg, pList);

        /* Programming MB
         */
        ulTmpStartReg = ulStartReg + (BCHP_VFC_0_CSC_R0_MB_COEFF_C00 - BCHP_VFC_0_NL_CSC_CTRL);
        BDBG_MODULE_MSG(BVDC_CFC_3,("VFC%d-Cfc MB:", hVfc->eId));
        BVDC_P_Cfc_BuildRulForCsc3x3_isr(
            &pCfc->stMb, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMb3x4_25, eLeftShift), ulTmpStartReg, pList);

        /* Programming MC
         */
        BDBG_MODULE_MSG(BVDC_CFC_3,("VFC%d-Cfc MC:", hVfc->eId));
        ulTmpStartReg = ulStartReg + (BCHP_VFC_0_CSC_R0_MC_COEFF_C00 - BCHP_VFC_0_NL_CSC_CTRL);
        BVDC_P_Cfc_BuildRulForCscRx4_isr(
            &pCfc->stMc, NULL, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMc3x4_16, eLeftShift), ulTmpStartReg, pList);

        /* TODO: programming Ma, Mb and Mc lshift
         */

        /* Programming LRange Adj
         */
        pCfc->stLRangeAdj.ulLRangeAdjCtrl = BCFC_LR_ADJ_LIMIT_NOT_EXIST;/* ignore limit for now; TODO */
        BDBG_MODULE_MSG(BVDC_CFC_3,("VFC%d-Cfc LRAdj:", hVfc->eId));
        ulTmpStartReg = ulStartReg + (BCHP_VFC_0_LRANGE_MIN - BCHP_VFC_0_NL_CSC_CTRL);
        BVDC_P_Cfc_BuildRulForLRAdjLimit_isr(&pCfc->stLRangeAdj, ulTmpStartReg, pList);
        ulTmpStartReg = ulStartReg + (BCHP_VFC_0_NL_LR_SLOPEi_ARRAY_BASE - BCHP_VFC_0_NL_CSC_CTRL);
        BVDC_P_Cfc_BuildRulForLRAdj_isr(&pCfc->stLRangeAdj, false, 0, ulTmpStartReg, pList);
    }
#endif /* if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */

#else
    BSTD_UNUSED(hVfc);
    BSTD_UNUSED(pList);
#endif
}

/* Build RUL for cfc inside a GFD
 */
void BVDC_P_GfxFeeder_BuildCfcRul_isr
    ( BVDC_P_GfxFeeder_Handle          hGfxFeeder,
      BVDC_P_ListInfo                 *pList)
{
    BCFC_Context *pCfc = &hGfxFeeder->stCfc;
    BCFC_ColorSpaceExt *pColorSpaceExtIn = &(pCfc->stColorSpaceExtIn);
    BCFC_ColorSpaceExt *pColorSpaceExtOut = pCfc->pColorSpaceExtOut;
    uint32_t ulStartReg;
    bool bConstantBlend = hGfxFeeder->stCurCfgInfo.stFlags.bConstantBlending;
    BCFC_LeftShift eLeftShift = BCFC_LeftShift_eNotExist;
    BCFC_CscType eMcCscType = (bConstantBlend)? BCFC_CscType_eMc3x5_16CB : BCFC_CscType_eMc3x5_16AB;
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_1)
    BCFC_CscType eMaCscType = (bConstantBlend)? BCFC_CscType_eMa3x5_25CB : BCFC_CscType_eMa3x5_25AB;
    bool bBypassCfc = pCfc->bBypassCfc;
#endif

    if ((BCFC_ColorFormat_eInvalid == pColorSpaceExtOut->stColorSpace.eColorFmt) ||
        (BCFC_ColorFormat_eInvalid == pColorSpaceExtIn->stColorSpace.eColorFmt) ||
        (0 == pCfc->ucRulBuildCntr))
    {
        /* input colorSpace or output ColorSpace not set yet, or no change */
        return;
    }
#if BVDC_P_DBV_SUPPORT
    /* DBV may require RAM LUT; TODO: add DBV gfx support */
    if (pCfc->stCapability.stBits.bRamLutScb && hGfxFeeder->bDbvEnabled &&
        !pCfc->stForceCfg.stBits.bDisableDolby)
    {
        BVDC_P_GfxFeeder_BuildDbvRul_isr(hGfxFeeder, pList);
        return; /* skip the rest */
    }
#endif

#if (BVDC_P_CMP_CFC_VER <= BVDC_P_CFC_VER_1)
    /* GFD output is always limited range YCbCr */
    if ((BVDC_P_RUL_UPDATE_THRESHOLD == pCfc->ucRulBuildCntr) && /* avoid applying stCscSdr2Hdr twice */
        (BCFC_ColorFormat_eRGB == pColorSpaceExtIn->stColorSpace.eColorFmt) &&
        (BCFC_Colorimetry_eBt709 == pColorSpaceExtIn->stColorSpace.eColorimetry) &&
        BCFC_IS_SDR(pColorSpaceExtIn->stColorSpace.eColorTF) &&
        (BCFC_IS_HDR10(pColorSpaceExtOut->stColorSpace.eColorTF) || BCFC_IS_HLG(pColorSpaceExtOut->stColorSpace.eColorTF)))
    {
        BVDC_P_GfxFeeder_AdjustMcForSdr2Hdr_isr(hGfxFeeder);
    }
#endif /* #if (BVDC_P_CMP_CFC_VER <= BVDC_P_CFC_VER_1) */

    pCfc->ucRulBuildCntr = 0;

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_1)

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2)
    if (pCfc->stCapability.stBits.bLRngAdj) /* bSupportTfConv */
    {
        uint32_t ulNLCfg;
        uint32_t ulNL2L = (pCfc->bForceBypassTfTbl)? BCFC_NL2L_BYPASS : pColorSpaceExtIn->stCfg.stBits.SelTF;
        uint32_t ulL2NL = (pCfc->bForceBypassTfTbl)? BCFC_L2NL_BYPASS : pColorSpaceExtOut->stCfg.stBits.SelTF;

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
/* 7271 B */
        uint32_t ulLutId;

        if ((!bBypassCfc) && (ulL2NL == BCFC_L2NL_RAM) && (pCfc->pTfConvRamLuts->pRamLutL2NL == NULL))
        {
            /* pColorSpaceExtOut->stCfg.stBits.SelTF likely set to BCFC_L2NL_RAM by cmp0 cfc */
            BDBG_ERR(("Gfd%d-Cfc fail to load RAM L2NL, likely hCfcHeap passded from nexus is NULL!", (hGfxFeeder->eId - BAVC_SourceId_eGfx0)));
            BDBG_ASSERT(pCfc->pTfConvRamLuts->pRamLutL2NL);
        }

        eLeftShift = BCFC_LeftShift_eOff;
        ulNLCfg = (bBypassCfc) ?
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, CSC_MC_ENABLE,  DISABLE) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, CSC_MB_ENABLE,  DISABLE) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, CSC_MA_ENABLE,  DISABLE) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, DLBV_CVM_EN,    DISABLE) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, LMR_ADJ_EN,     DISABLE) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN,  DISABLE) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, SEL_L2NL,       BYPASS)  |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, SEL_NL2L,       BYPASS)  |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, SEL_CL_IN,      DEFAULT) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, SEL_XVYCC_L2NL, DEFAULT) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, SEL_XVYCC_NL2L, DEFAULT)
            :
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, CSC_MC_ENABLE,  ENABLE)  |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, CSC_MB_ENABLE,  ENABLE)  |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, CSC_MA_ENABLE,  ENABLE)  |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, DLBV_CVM_EN,    DISABLE) |
            BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, LMR_ADJ_EN,     (pCfc->pTfConvRamLuts->pRamLutLMR)? 1 : 0) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN,  ENABLE)  |
            BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_L2NL,       ulL2NL) |
            BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_NL2L,       ulNL2L) |
            BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_CL_IN,      pColorSpaceExtIn->stCfg.stBits.bSelCL) |
            BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_XVYCC_L2NL, pColorSpaceExtOut->stCfg.stBits.bSelXvYcc) |
            BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_XVYCC_NL2L, pColorSpaceExtIn->stCfg.stBits.bSelXvYcc);
        BDBG_MODULE_MSG(BVDC_CFC_2,("Gfd%d-Cfc NL_CSC_CTRL 0x%08x:", (hGfxFeeder->eId - BAVC_SourceId_eGfx0), ulNLCfg));
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG( );
        *pList->pulCurrent++ = BRDC_REGISTER( BCHP_GFD_0_NL_CSC_CTRL ) + hGfxFeeder->ulRegOffset;
        *pList->pulCurrent++ = ulNLCfg;

        /* build rul for ram lut NL2L */
        if (pCfc->pTfConvRamLuts->pRamLutNL2L && pCfc->ucRamNL2LRulBuildCntr)
        {
            pCfc->ucRamNL2LRulBuildCntr--;

            ulLutId = BCHP_GFD_TYPE_LUT_HEADER_W0_LUT_ID_G0_NL2L;
            ulStartReg = BCHP_GFD_0_NL2L_LUT_CTRL + hGfxFeeder->ulRegOffset;
            BDBG_MODULE_MSG(BVDC_CFC_2,("Gfd%d-Cfc uses RAM NL2L, lutId %d", (hGfxFeeder->eId - BAVC_SourceId_eGfx0), ulLutId));
            BVDC_P_Cfc_BuildRulForRamLut_isr(pCfc->pTfConvRamLuts->pRamLutNL2L, ulStartReg, ulLutId, &hGfxFeeder->stCfcLutList, pList);
        }

        /* build rul for LMR */
        if (pCfc->pTfConvRamLuts->pRamLutLMR && pCfc->ucRamLMRRulBuildCntr)
        {
            uint32_t ulMb2StartReg, ulAdjStartReg;

            pCfc->ucRamLMRRulBuildCntr--;

            ulMb2StartReg = BCHP_GFD_0_CSC_R0_MB2_COEFF_C00 + hGfxFeeder->ulRegOffset;
            ulAdjStartReg = BCHP_GFD_0_LMR_MULT_COEFF + hGfxFeeder->ulRegOffset;
            BVDC_P_Cfc_BuildRulForLmrAdj_isr(pCfc, ulMb2StartReg, ulAdjStartReg, pList);

            ulLutId = BCHP_GFD_TYPE_LUT_HEADER_W0_LUT_ID_G0_LMR;
            ulStartReg = BCHP_GFD_0_LMR_LUT_CTRL + hGfxFeeder->ulRegOffset;
            BDBG_MODULE_MSG(BVDC_CFC_2,("Gfd%d-Cfc uses LMR, lutId %d", (hGfxFeeder->eId - BAVC_SourceId_eGfx0), ulLutId));
            BVDC_P_Cfc_BuildRulForRamLut_isr(pCfc->pTfConvRamLuts->pRamLutLMR, ulStartReg, ulLutId, &hGfxFeeder->stCfcLutList, pList);
        }

        /* build rul for ram lut L2NL */
        if (pCfc->pTfConvRamLuts->pRamLutL2NL && pCfc->ucRamL2NLRulBuildCntr)
        {
            pCfc->ucRamL2NLRulBuildCntr--;

            ulLutId = BCHP_GFD_TYPE_LUT_HEADER_W0_LUT_ID_G0_L2NL;
            ulStartReg = BCHP_GFD_0_L2NL_LUT_CTRL + hGfxFeeder->ulRegOffset;
            BDBG_MODULE_MSG(BVDC_CFC_2,("Gfd%d-Cfc uses RAM L2NL, lutId %d", (hGfxFeeder->eId - BAVC_SourceId_eGfx0), ulLutId));
            BVDC_P_Cfc_BuildRulForRamLut_isr(pCfc->pTfConvRamLuts->pRamLutL2NL, ulStartReg, ulLutId, &hGfxFeeder->stCfcLutList, pList);
        }

        /* start ram luts loading if they are used */
        BVDC_P_Cfc_BuildRulForLutLoading_isr(&hGfxFeeder->stCfcLutList, BCHP_GFD_0_LUT_DESC_ADDR + hGfxFeeder->ulRegOffset,
            BCHP_GFD_0_LUT_DESC_CFG + hGfxFeeder->ulRegOffset, pList);

        /* Programming HDR blend-in-matrix: GFD_0_CTRL.ALPHA_DIV_EN controls whether GFD blend-in-matrix is used.
         * 7271 A0 does not have blend-in-matrix, but has alpha-div
         * is it possible that bEnBlendMatrix changes to true
         */
        if (hGfxFeeder->hWindow)
        {
            BVDC_Compositor_Handle hCompositor = hGfxFeeder->hWindow->hCompositor;
            bool bEnBlendMatrix = BVDC_P_CFC_NEED_BLEND_MATRIX(hCompositor);
            eMaCscType = BCFC_CscType_eMa3x5_25CB;
            /*if ((bEnBlendMatrix != pCfc->bBlendInMatrixOn) || hCompositor->stCurInfo.stDirty.stBits.bOutColorSpace)*/
            {
                if (bEnBlendMatrix)
                {
                    const BCFC_Csc3x4 *pCsc;
                    BDBG_MODULE_MSG(BVDC_CFC_2,("Gfd%d-Cfc: enable blend-in-matrices", (hGfxFeeder->eId - BAVC_SourceId_eGfx0)));
                    pCsc = BCFC_GetCsc3x4_Ma_isrsafe(hCompositor->stOutColorSpaceExt.stColorSpace.eColorimetry);
                    ulStartReg = BCHP_GFD_0_CSC_COEFF_C00 + hGfxFeeder->ulRegOffset;
                    BVDC_P_Cfc_BuildRulForCscRx4_isr(
                        pCsc, NULL, BCFC_MAKE_CSC_CFG(eMcCscType, BCFC_LeftShift_eNotExist), ulStartReg, pList);
                    pCfc->bBlendInMatrixOn = bEnBlendMatrix;
                }
                else
                {
                    BDBG_MODULE_MSG(BVDC_CFC_2,("Gfd%d-Cfc: disable blend-in-matrices, due to %s",
                        (hGfxFeeder->eId - BAVC_SourceId_eGfx0), (!hCompositor->abBlenderUsed[0])? "no blending" : "non-HDR10 output"));
                    pCfc->bBlendInMatrixOn = bEnBlendMatrix;
                }
            }
        }

#elif (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
        ulNLCfg = (bBypassCfc)?
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN,  DISABLE) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, SEL_L2NL,       BYPASS)  |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, SEL_NL2L,       BYPASS)  |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, SEL_CL_IN,      DEFAULT) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, SEL_XVYCC,      DEFAULT) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, NL_CSC,         BYPASS)
            :
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, LRANGE_ADJ_EN,  ENABLE)  |
            BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_L2NL,       ulL2NL) |
            BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_NL2L,       ulNL2L)  |
            BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_CL_IN,      pColorSpaceExtIn->stCfg.stBits.bSelCL) |
            BCHP_FIELD_DATA(GFD_0_NL_CSC_CTRL, SEL_XVYCC,      pColorSpaceExtIn->stCfg.stBits.bSelXvYcc | pColorSpaceExtOut->stCfg.stBits.bSelXvYcc) |
            BCHP_FIELD_ENUM(GFD_0_NL_CSC_CTRL, NL_CSC,         ENABLE);
        BDBG_MODULE_MSG(BVDC_CFC_2,("Gfd%d-Cfc NL_CSC_CTRL 0x%08x:", (hGfxFeeder->eId - BAVC_SourceId_eGfx0), ulNLCfg));
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG( );
        *pList->pulCurrent++ = BRDC_REGISTER( BCHP_GFD_0_NL_CSC_CTRL ) + hGfxFeeder->ulRegOffset;
        *pList->pulCurrent++ = ulNLCfg;
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */

        /* Programming MB
         */
        ulStartReg = BCHP_GFD_0_CSC_R0_MB_COEFF_C00 + hGfxFeeder->ulRegOffset;
        BDBG_MODULE_MSG(BVDC_CFC_3,("Gfd%d-Cfc MB:", (hGfxFeeder->eId - BAVC_SourceId_eGfx0)));
        BVDC_P_Cfc_BuildRulForCsc3x3_isr(
            &pCfc->stMb, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMb3x4_25, eLeftShift), ulStartReg, pList);

        /* Programming LRange Adj
         */
        BDBG_MODULE_MSG(BVDC_CFC_3,("Gfd%d-Cfc LRAdj:", (hGfxFeeder->eId - BAVC_SourceId_eGfx0)));
        ulStartReg = BCHP_GFD_0_NL_LR_SLOPEi_ARRAY_BASE + hGfxFeeder->ulRegOffset;
      #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
        pCfc->stLRangeAdj.ulLRangeAdjCtrl = BCFC_LR_ADJ_LIMIT_NOT_EXIST;
      #elif (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
        pCfc->stLRangeAdj.ulLRangeAdjCtrl = BCFC_LR_ADJ_LIMIT_DISABLE;
        BDBG_CASSERT(BCHP_GFD_0_LRANGE_LIMIT_EN);
        BVDC_P_Cfc_BuildRulForLRAdjLimit_isr(&pCfc->stLRangeAdj, BCHP_GFD_0_LRANGE_MIN + hGfxFeeder->ulRegOffset, pList);
      #endif
        BVDC_P_Cfc_BuildRulForLRAdj_isr(&pCfc->stLRangeAdj, false, 0, ulStartReg, pList);
    }
    else
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */

    /* 7271 CMP1, or 7439 B0 */
    if (pCfc->stCapability.stBits.bBlackBoxNLConv)
    {
      #ifdef BCHP_GFD_0_NL_CSC_CTRL_NL_CSC_R0_ENABLE
        uint32_t ulNLCfg = ((BCFC_NL_SEL_BYPASS == pCfc->ucSelBlackBoxNL) || bBypassCfc)? 0 :
            (BCHP_FIELD_ENUM( GFD_0_NL_CSC_CTRL, NL_CSC_R0, ENABLE) |
             BCHP_FIELD_DATA( GFD_0_NL_CSC_CTRL, SEL_CONV_R0, pCfc->ucSelBlackBoxNL));
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG( );
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GFD_0_NL_CSC_CTRL) + hGfxFeeder->ulRegOffset;
        *pList->pulCurrent++ = ulNLCfg;
        BDBG_MODULE_MSG(BVDC_CFC_2,("Gfd%d-Cfc NL_CSC_CTRL: 0x%x", (hGfxFeeder->eId - BAVC_SourceId_eGfx0), ulNLCfg));
      #endif /* #if defined(BCHP_GFD_0_NL_CSC_CTRL_NL_CSC_R0_ENABLE) */
        eMaCscType = (bConstantBlend)? BCFC_CscType_eMa3x5_16CB : BCFC_CscType_eMa3x5_16AB;
    }

    /* Programming MA
     */
    if (pCfc->stCapability.stBits.bMa)
    {
        ulStartReg = BCHP_GFD_0_CSC_R0_MA_COEFF_C00 + hGfxFeeder->ulRegOffset;
        BDBG_MODULE_MSG(BVDC_CFC_3,("Gfd%d-Cfc MA:", (hGfxFeeder->eId - BAVC_SourceId_eGfx0)));
        BVDC_P_Cfc_BuildRulForCscRx4_isr(
            pCfc->pMa, NULL, BCFC_MAKE_CSC_CFG(eMaCscType, eLeftShift), ulStartReg, pList);
    }
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_1) */

    /* Programming MC
     */
  #if defined(BCHP_GFD_0_CSC_R0_MC_COEFF_C00) && !defined(BVDC_FOR_BOOTUPDATER)
    ulStartReg = (hGfxFeeder->eId == BAVC_SourceId_eGfx0)?
        BCHP_GFD_0_CSC_R0_MC_COEFF_C00 + hGfxFeeder->ulRegOffset :
        BCHP_GFD_0_CSC_COEFF_C00 + hGfxFeeder->ulRegOffset;
  #else
    ulStartReg = BCHP_GFD_0_CSC_COEFF_C00 + hGfxFeeder->ulRegOffset;
  #endif
    BDBG_MODULE_MSG(BVDC_CFC_3,("Gfd%d-Cfc MC:", hGfxFeeder->eId - BAVC_SourceId_eGfx0));
  #if (BVDC_P_CMP_CFC_VER == BVDC_P_CFC_VER_2)
    if ((hGfxFeeder->hWindow) && BVDC_P_CFC_NEED_BLEND_MATRIX(hGfxFeeder->hWindow->hCompositor))
    {
        /* thid GFD version does not have blend-matrix, but we could mimic by using identity MC */
        BVDC_P_Cfc_BuildRulForCscRx4_isr(
            BCFC_GetCsc3x4_Identity_isrsafe(), NULL, BCFC_MAKE_CSC_CFG(eMcCscType, eLeftShift), ulStartReg, pList);
        return;
    }
  #endif
    BVDC_P_Cfc_BuildRulForCscRx4_isr(
        &pCfc->stMc, NULL, BCFC_MAKE_CSC_CFG(eMcCscType, eLeftShift), ulStartReg, pList);
}

/* Build RUL for blend out matrix
 */
void BVDC_P_Compositor_BuildBlendOutMatrixRul_isr
    ( BVDC_Compositor_Handle           hCompositor,
      BVDC_P_ListInfo                 *pList)
{
#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2)
    if (hCompositor->stCfcCapability[0].stBits.bLRngAdj)
    {
        uint32_t ulStartReg;
        const BCFC_Csc3x4 *pCsc;
        bool bEnBlendMatrix;

        bEnBlendMatrix = BVDC_P_CFC_NEED_BLEND_MATRIX(hCompositor);
#if BVDC_P_DBV_SUPPORT
        /* DBV will bypass blendout logic here and defer */
        if(BVDC_P_CMP_DBV_MODE(hCompositor) &&
           (!hCompositor->ahWindow[BVDC_P_WindowId_eComp0_V0] ||
            !hCompositor->ahWindow[BVDC_P_WindowId_eComp0_V0]->astMosaicCfc[0].stForceCfg.stBits.bDisableDolby))
        {
            hCompositor->bBlendMatrixOn = false;
            return;
        }
#endif
        /* DBV out will have blender out configured in dbv code; */
        if ((bEnBlendMatrix != hCompositor->bBlendMatrixOn) || hCompositor->stCurInfo.stDirty.stBits.bOutColorSpace)
        {
            hCompositor->bBlendMatrixOn = bEnBlendMatrix;
            hCompositor->ucBlendMatrixOnRulBuildCntr = BVDC_P_RUL_UPDATE_THRESHOLD;
        }
        if (hCompositor->ucBlendMatrixOnRulBuildCntr)
        {
            ulStartReg = BCHP_HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN + hCompositor->ulRegOffset;
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
            if (bEnBlendMatrix)
            {
                *pList->pulCurrent++ =
                    BCHP_FIELD_ENUM(HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN, BLENDER_PQ_ADJ_ENABLE,  ENABLE) |
                    BCHP_FIELD_ENUM(HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN, BLENDER_OUT_CSC_ENABLE, ENABLE);
                BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d: enable blend-out-matrices", hCompositor->eId));
                pCsc = BCFC_GetCsc3x4_Mc_isrsafe(hCompositor->stOutColorSpaceExt.stColorSpace.eColorimetry);
                ulStartReg = BCHP_HDR_CMP_0_BLENDER_OUT_COEFF_C00 + hCompositor->ulRegOffset;
                BVDC_P_Cfc_BuildRulForCscRx4_isr(
                    pCsc, NULL, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMc3x4_16, BCFC_LeftShift_eNotExist), ulStartReg, pList);
            }
            else
            {
                *pList->pulCurrent++ =
                    BCHP_FIELD_ENUM(HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN, BLENDER_PQ_ADJ_ENABLE,  DISABLE) |
                    BCHP_FIELD_ENUM(HDR_CMP_0_CMP_BLENDER_OUT_PQ_CSC_EN, BLENDER_OUT_CSC_ENABLE, DISABLE);
                BDBG_MODULE_MSG(BVDC_CFC_2,("Cmp%d: disable blend-out-matrices, due to %s",
                    hCompositor->eId, (!hCompositor->abBlenderUsed[0])? "no blending" : "non-HDR10 output"));
            }
            hCompositor->ucBlendMatrixOnRulBuildCntr --;
        }
    }
#else
    BSTD_UNUSED(hCompositor);
    BSTD_UNUSED(pList);
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_2) */
}

static const uint8_t s_aVecChanSwap_RGB_Row_Tbl[] =
{
    1,    /* HW reg row 0 uses SW matrix row 1 */
    2,    /* HW reg row 1 uses SW matrix row 2 */
    0     /* HW reg row 2 uses SW matrix row 0 */
};

static const uint8_t s_aVecChanSwap_Column_Tbl[] =
{
    1,    /* HW reg column 0 uses SW matrix column 1 */
    0,    /* HW reg column 1 uses SW matrix column 0 */
    2,    /* HW reg column 2 uses SW matrix column 2 */
    3     /* HW reg column 4 uses SW matrix column 3 */
};

#if (BVDC_P_CFC_SWAP_FOR_CL_OUT && (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3))
static const uint8_t s_aVecChanSwap_MC_Column_For_CL_Tbl[] =
{
    2,    /* HW reg column 0 uses SW matrix column 2 */
    1,    /* HW reg column 1 uses SW matrix column 1 */
    0,    /* HW reg column 2 uses SW matrix column 0 */
    3     /* HW reg column 4 uses SW matrix column 3 */
};
#endif /* #if (BVDC_P_CFC_SWAP_FOR_CL_OUT && (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)) */

/* swap rows and columns due to diff channel connection in HW
 */
static void BVDC_P_Display_SwapRowAndColumn_isr
    (  BCFC_ColorSpace    *pColorSpaceOut,
       const BCFC_Csc3x4  *pSwCsc,
       BCFC_Csc3x4        *pHwCsc )
{
    int ii, jj;

    if (BCFC_ColorFormat_eRGB == pColorSpaceOut->eColorFmt)
    {
        BDBG_MODULE_MSG(BVDC_CFC_3,("  DVI_CSC RGB out: SwRow1->HwRow0, SwRow2->HwRow1, SwRow0->HwRow2"));
        BDBG_MODULE_MSG(BVDC_CFC_3,("                   SwCol1->HwCol0, SwCol0->HwCol1, SwCol2->HwCol2, SwCol3->HwCol3"));
        for (ii=0; ii<3; ii++)
            for (jj=0; jj<4; jj++)
            {
                pHwCsc->m[ii][jj] = pSwCsc->m[s_aVecChanSwap_RGB_Row_Tbl[ii]][s_aVecChanSwap_Column_Tbl[jj]];
            }
    }
  #if (BVDC_P_CFC_SWAP_FOR_CL_OUT && (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3))
    else if (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceOut->eColorFmt)
    {
        BDBG_MODULE_MSG(BVDC_CFC_3,("  DVI_CSC: SwCol2->HwCol0, SwCol1->HwCol1, SwCol0->HwCol2, SwCol3->HwCol3"));
        for (ii=0; ii<3; ii++)
            for (jj=0; jj<4; jj++)
            {
                pHwCsc->m[ii][jj] = pSwCsc->m[ii][s_aVecChanSwap_MC_Column_For_CL_Tbl[jj]];
            }
    }
  #endif /* #if (BVDC_P_CFC_SWAP_FOR_CL_OUT && (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)) */
    else
    {
        BDBG_MODULE_MSG(BVDC_CFC_3,("  DVI_CSC: SwCol1->HwCol0, SwCol0->HwCol1, SwCol2->HwCol2, SwCol3->HwCol3"));
        for (ii=0; ii<3; ii++)
            for (jj=0; jj<4; jj++)
            {
                pHwCsc->m[ii][jj] = pSwCsc->m[ii][s_aVecChanSwap_Column_Tbl[jj]];
            }
    }
}

#define BVDC_P_MAKE_DVI_LR_ADJ_LIMIT(m)    \
    (((m) & BCHP_DVI_CFC_0_NL_LR_XYi_LRA_X_MASK) << (BCFC_LR_ADJ_LIMIT_F_BITS - BCFC_LR_XY_F_BITS))

/* Build RUL for cfc inside DVI output
 */
void BVDC_P_Display_BuildCfcRul_isr
    ( BVDC_Display_Handle              hDisplay,
      uint32_t                         ulStartReg,
      BVDC_P_ListInfo                 *pList)
{
    BCFC_Context *pCfc = &hDisplay->stCfc;
    BCFC_LeftShift eLeftShift = BCFC_LeftShift_eNotExist;
    uint32_t ulTmpStartReg;
    const BVDC_P_CscClamp *pCscClamp;
    BCFC_ColorSpaceExt *pColorSpaceExtIn = &(pCfc->stColorSpaceExtIn);
    BCFC_ColorSpaceExt *pColorSpaceExtOut = pCfc->pColorSpaceExtOut;
    BCFC_ColorSpace *pColorSpaceOut = &pColorSpaceExtOut->stColorSpace;
    BCFC_Csc3x4 stHwMc, *pHwMc = &stHwMc;

    if ((BCFC_ColorFormat_eInvalid == pColorSpaceOut->eColorFmt) ||
        (BCFC_ColorFormat_eInvalid == pColorSpaceExtIn->stColorSpace.eColorFmt) ||
        (0 == pCfc->ucRulBuildCntr))
    {
        /* input colorSpace or output ColorSpace not set yet, or no change */
        return;
    }
    pCfc->ucRulBuildCntr--;

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
/* 7271 B */
#if BVDC_P_DBV_SUPPORT
    /* DBV will compose video inputs together */
    if (BVDC_P_CMP_DBV_MODE(hDisplay->hCompositor) &&
        (!hDisplay->hCompositor->ahWindow[BVDC_P_WindowId_eComp0_V0] ||
         !hDisplay->hCompositor->ahWindow[BVDC_P_WindowId_eComp0_V0]->astMosaicCfc[0].stForceCfg.stBits.bDisableDolby))
    {
        BVDC_P_Display_BuildDbvRul_isr(hDisplay, pList);
        return; /* skip the rest */
    }
#endif

    /* must have ulStartReg = BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL in this case */
    if (pCfc->stCapability.stBits.bLRngAdj) /* bSupportTfConv */
    {
        uint32_t ulNLCfg, ulLutId;
        BCFC_Csc3x4 stHwAlt, *pHwAlt = NULL;
        bool bBypassCfc = pCfc->bBypassCfc || hDisplay->bCmpBypassDviCsc || hDisplay->stCurInfo.bBypassVideoProcess;
        uint32_t ulNL2L = (pCfc->bForceBypassTfTbl)? BCFC_NL2L_BYPASS : pColorSpaceExtIn->stCfg.stBits.SelTF;
        uint32_t ulL2NL = (pCfc->bForceBypassTfTbl)? BCFC_L2NL_BYPASS : pColorSpaceExtOut->stCfg.stBits.SelTF;

        /* set eLeftShift = BCFC_LeftShift_e1/2_0 if needed */
        ulNLCfg = (bBypassCfc)?
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, CSC_MB_ENABLE,   DISABLE) |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, CSC_MA_ENABLE,   DISABLE) |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, LRANGE_LIMIT_EN, DEFAULT) |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, LRANGE_ADJ_EN,   DISABLE) |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, SEL_L2NL,        BYPASS)  |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, SEL_NL2L,        BYPASS)  |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, SEL_CL_OUT,      DEFAULT) |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, NL_CSC,          BYPASS) /* 0x550 */
            :
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, CSC_MB_ENABLE,   ENABLE)  |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, CSC_MA_ENABLE,   ENABLE)  |
            BCHP_FIELD_DATA(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, LRANGE_LIMIT_EN, 1)       |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, LRANGE_ADJ_EN,   ENABLE)  |
            BCHP_FIELD_DATA(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, SEL_L2NL,        ulL2NL) |
            BCHP_FIELD_DATA(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, SEL_NL2L,        ulNL2L) |
            BCHP_FIELD_DATA(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, SEL_CL_OUT,      pColorSpaceExtOut->stCfg.stBits.bSelCL) |
            BCHP_FIELD_DATA(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, SEL_XVYCC_L2NL,  pColorSpaceExtOut->stCfg.stBits.bSelXvYcc) |
            BCHP_FIELD_DATA(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, SEL_XVYCC_NL2L,  pColorSpaceExtIn->stCfg.stBits.bSelXvYcc) |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, NL_CSC,          ENABLE);
        ulNLCfg |= (bBypassCfc)?
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, CSC_MC_ENABLE,   ENABLE) /* 0x20550, cannot bypass MC ??? */
            :
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, CSC_MC_ENABLE,   ENABLE);
        BDBG_MODULE_MSG(BVDC_CFC_2,("Display%d-Cfc NL_CSC_CTRL 0x%08x:", hDisplay->eId, ulNLCfg));
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG( );
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
        *pList->pulCurrent++ = ulNLCfg;

        /* build rul for ram lut NL2L */
        if (pCfc->pTfConvRamLuts && pCfc->pTfConvRamLuts->pRamLutNL2L && pCfc->ucRamNL2LRulBuildCntr)
        {
            pCfc->ucRamNL2LRulBuildCntr--;

            ulLutId = BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_VEC_NL2L;
            ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_NL2L_LUT_CTRL - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
            BDBG_MODULE_MSG(BVDC_CFC_2,("Display%d-Cfc uses RAM NL2L, lutId %d", hDisplay->eId, ulLutId));
            BVDC_P_Cfc_BuildRulForRamLut_isr(pCfc->pTfConvRamLuts->pRamLutNL2L, ulTmpStartReg, ulLutId, &hDisplay->stCfcLutList, pList);
        }

        /* build rul for ram lut L2NL */
        if (pCfc->pTfConvRamLuts && pCfc->pTfConvRamLuts->pRamLutL2NL && pCfc->ucRamL2NLRulBuildCntr)
        {
            pCfc->ucRamL2NLRulBuildCntr--;

            ulLutId = BCHP_CMP_0_HDR_TYPE_LUT_HEADER_W0_LUT_ID_VEC_L2NL;
            ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_L2NL_LUT_CTRL - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
            BDBG_MODULE_MSG(BVDC_CFC_2,("Display%d-Cfc uses RAM L2NL, lutId %d", hDisplay->eId, ulLutId));
            BVDC_P_Cfc_BuildRulForRamLut_isr(pCfc->pTfConvRamLuts->pRamLutL2NL, ulTmpStartReg, ulLutId, &hDisplay->stCfcLutList, pList);
        }

        /* start ram luts loading if they are used */
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_LUT_DESC_ADDR - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
        BVDC_P_Cfc_BuildRulForLutLoading_isr(&hDisplay->stCfcLutList, ulTmpStartReg,
            ulStartReg + (BCHP_DVI_CFC_0_LUT_DESC_CFG - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL), pList);

        /* Programming MA
         */
      #if BVDC_P_CFC_SWAP_FOR_CL_OUT
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_CFC_MUX_SEL_YCbCr - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG( );
        *pList->pulCurrent++ = BRDC_REGISTER(ulTmpStartReg);
        *pList->pulCurrent++ = (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceOut->eColorFmt)?
            BCHP_FIELD_ENUM(DVI_CFC_0_CFC_MUX_SEL_YCbCr, SEL_Y,  USE_CB) |
            BCHP_FIELD_ENUM(DVI_CFC_0_CFC_MUX_SEL_YCbCr, SEL_Cb, USE_CR) |
            BCHP_FIELD_ENUM(DVI_CFC_0_CFC_MUX_SEL_YCbCr, SEL_Cr, USE_Y)    /* 0x18 */
            :
            BCHP_FIELD_ENUM(DVI_CFC_0_CFC_MUX_SEL_YCbCr, SEL_Y,  USE_Y)  |
            BCHP_FIELD_ENUM(DVI_CFC_0_CFC_MUX_SEL_YCbCr, SEL_Cb, USE_CB) |
            BCHP_FIELD_ENUM(DVI_CFC_0_CFC_MUX_SEL_YCbCr, SEL_Cr, USE_CR);  /* 0x6 */
      #endif /* #if BVDC_P_CFC_SWAP_FOR_CL_OUT */
        BDBG_MODULE_MSG(BVDC_CFC_3,("Display%d-Cfc MA:", hDisplay->eId));
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_CSC_R0_MA_COEFF_C00 - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
        BVDC_P_Cfc_BuildRulForCscRx4_isr(
            pCfc->pMa, NULL, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMa3x4_25, eLeftShift), ulTmpStartReg, pList);

        /* Programming MB
         */
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_CSC_R0_MB_COEFF_C00 - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
        BDBG_MODULE_MSG(BVDC_CFC_3,("Display%d-Cfc MB:", hDisplay->eId));
        BVDC_P_Cfc_BuildRulForCsc3x3_isr(
            &pCfc->stMb, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMb3x4_25, eLeftShift), ulTmpStartReg, pList);

        /* Programming MC
         */
        BDBG_MODULE_MSG(BVDC_CFC_3,("Display%d-Cfc MC:", hDisplay->eId));
        BVDC_P_Display_SwapRowAndColumn_isr(pColorSpaceOut, &pCfc->stMc, pHwMc);
        if (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceOut->eColorFmt)
        {
            pHwAlt = &stHwAlt;
            BVDC_P_Display_SwapRowAndColumn_isr(pColorSpaceOut, &pColorSpaceExtOut->stMalt, pHwAlt);
        }
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_CSC_R0_MC_COEFF_C00 - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
        BVDC_P_Cfc_BuildRulForCscRx4_isr(
            pHwMc, pHwAlt, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMc5x4_16, eLeftShift), ulTmpStartReg, pList);

        /* programming Ma, Mb and Mc lshift
         */
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_VEC_MA_COEFF_LSHF - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(3);
        *pList->pulCurrent++ = BRDC_REGISTER(ulTmpStartReg);
        *pList->pulCurrent++ = BCFC_LSHIFT_BITS(eLeftShift);
        *pList->pulCurrent++ = BCFC_LSHIFT_BITS(eLeftShift);
        *pList->pulCurrent++ = BCFC_LSHIFT_BITS(eLeftShift);

        /* Programming LRange Adj
         */
        pCfc->stLRangeAdj.ulLRangeAdjCtrl = BCFC_LR_ADJ_LIMIT_ALWAYS_ON;
        pCfc->stLRangeAdj.ulMin = BVDC_P_MAKE_DVI_LR_ADJ_LIMIT(pCfc->stLRangeAdj.pTable->aulLRangeAdjXY[0]);
        pCfc->stLRangeAdj.ulMax = BVDC_P_MAKE_DVI_LR_ADJ_LIMIT(pCfc->stLRangeAdj.pTable->aulLRangeAdjXY[7]) - 1; /* 0x3FFFFFF */
        BDBG_MODULE_MSG(BVDC_CFC_3,("Display%d-Cfc LRAdj:", hDisplay->eId));
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_LRANGE_MIN - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
        BVDC_P_Cfc_BuildRulForLRAdjLimit_isr(&pCfc->stLRangeAdj, ulTmpStartReg, pList);
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_NL_LR_SLOPEi_ARRAY_BASE - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
        BVDC_P_Cfc_BuildRulForLRAdj_isr(&pCfc->stLRangeAdj, false, 0, ulTmpStartReg, pList);

        /* programming Y, Cb, and Cr clamp
         */
        pCscClamp = &s_CLAMP_7271B0;
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_CSC_MIN_MAX_Y - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(3);
        *pList->pulCurrent++ = BRDC_REGISTER(ulTmpStartReg);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(DVI_CFC_0_CSC_MIN_MAX_Y,  CLAMP_EN, ENABLE) |
            BCHP_FIELD_DATA(DVI_CFC_0_CSC_MIN_MAX_Y,  MIN,      pCscClamp->ulYMin) |
            BCHP_FIELD_DATA(DVI_CFC_0_CSC_MIN_MAX_Y,  MAX,      pCscClamp->ulYMax);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(DVI_CFC_0_CSC_MIN_MAX_CB, CLAMP_EN, ENABLE) |
            BCHP_FIELD_DATA(DVI_CFC_0_CSC_MIN_MAX_CB, MIN,      pCscClamp->ulCbMin) |
            BCHP_FIELD_DATA(DVI_CFC_0_CSC_MIN_MAX_CB, MAX,      pCscClamp->ulCbMax);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(DVI_CFC_0_CSC_MIN_MAX_CR, CLAMP_EN, ENABLE) |
            BCHP_FIELD_DATA(DVI_CFC_0_CSC_MIN_MAX_CR, MIN,      pCscClamp->ulCrMin) |
            BCHP_FIELD_DATA(DVI_CFC_0_CSC_MIN_MAX_CR, MAX,      pCscClamp->ulCrMax);
    }
    else
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */

    {
        /* must have ulStartReg = DVI_CSC_0_CSC_MODE in these cases */
#ifdef BCHP_DVI_CSC_0_CL2020_CONTROL
        /* 7439 B0, 7271 A0: pCfc->stCapability.stBits.bMa indicates it can output YCbCr_CL */
        if (pCfc->stCapability.stBits.bMa)
        {
            uint32_t ulNLCfg;
            ulNLCfg = ((!pCfc->bBypassCfc) &&
                       (BCFC_ColorFormat_eYCbCr_CL == pColorSpaceOut->eColorFmt) &&
                       (BCFC_Colorimetry_eBt2020 == pColorSpaceOut->eColorimetry)) ?
                BCHP_FIELD_ENUM(DVI_CSC_0_CL2020_CONTROL, CTRL,      ENABLE ) |
                BCHP_FIELD_ENUM(DVI_CSC_0_CL2020_CONTROL, SEL_GAMMA, BT1886_GAMMA )
                :
                BCHP_FIELD_ENUM(DVI_CSC_0_CL2020_CONTROL, CTRL,      DISABLE );
            ulTmpStartReg = ulStartReg + (BCHP_DVI_CSC_0_CL2020_CONTROL - BCHP_DVI_CSC_0_CSC_MODE);
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(ulTmpStartReg);
            *pList->pulCurrent++ = ulNLCfg;
            BDBG_MODULE_MSG(BVDC_CFC_2,("Display%d-Cfc CL2020_CTRL 0x%08x:", hDisplay->eId, ulNLCfg));
        }
#endif /* #ifdef BCHP_DVI_CSC_0_CL2020_CONTROL */

#ifdef BCHP_DVI_CSC_0_CSC_COEFF_C01_C00
        /* Programming MC (it is indeed Ma with CL output)
         */
        BDBG_MODULE_MSG(BVDC_CFC_3,("Display%d-Cfc MC:", hDisplay->eId));
        BVDC_P_Display_SwapRowAndColumn_isr(pColorSpaceOut, &pCfc->stMc, pHwMc);
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CSC_0_CSC_COEFF_C01_C00 - BCHP_DVI_CSC_0_CSC_MODE);
        BVDC_P_Cfc_BuildRulForCscRx4_isr(
            pHwMc, NULL, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMcPacked, eLeftShift), ulTmpStartReg, pList);

        /* Programming clamp
         */
        pCscClamp = s_aCLAMP_Tbl[pColorSpaceOut->eColorFmt][pColorSpaceOut->eColorRange];
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS((BCHP_CSC_0_CSC_MIN_MAX - BCHP_CSC_0_CSC_MODE)/4 + 1);
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(CSC_0_CSC_MODE, CLAMP_MODE_C0, MIN_MAX) |
            BCHP_FIELD_ENUM(CSC_0_CSC_MODE, CLAMP_MODE_C1, MIN_MAX) |
            BCHP_FIELD_ENUM(CSC_0_CSC_MODE, CLAMP_MODE_C2, MIN_MAX) |
            BCHP_FIELD_DATA(CSC_0_CSC_MODE, RANGE1, 0x005A) |
            BCHP_FIELD_DATA(CSC_0_CSC_MODE, RANGE2, 0x007F);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(CSC_0_CSC_MIN_MAX, MIN, pCscClamp->ulYMin) |
            BCHP_FIELD_DATA(CSC_0_CSC_MIN_MAX, MAX, pCscClamp->ulYMax);

#elif defined(BVDC_FOR_BOOTUPDATER)
        BSTD_UNUSED(pCscClamp);
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG( );
        *pList->pulCurrent++ = BRDC_REGISTER(ulStartReg);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, CSC_MC_ENABLE,   ENABLE) |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, NL_CSC,          ENABLE) |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, SEL_L2NL,        BYPASS) |
            BCHP_FIELD_ENUM(DVI_CFC_0_VEC_HDR_NL_CSC_CTRL, SEL_NL2L,        BYPASS);

        BVDC_P_Display_SwapRowAndColumn_isr(pColorSpaceOut, &pCfc->stMc, pHwMc);
        ulTmpStartReg = ulStartReg + (BCHP_DVI_CFC_0_CSC_R0_MC_COEFF_C00 - BCHP_DVI_CFC_0_VEC_HDR_NL_CSC_CTRL);
        BVDC_P_Cfc_BuildRulForCscRx4_isr(
            pHwMc, NULL, BCFC_MAKE_CSC_CFG(BCFC_CscType_eMc5x4_16, eLeftShift), ulTmpStartReg, pList);
#endif /* BCHP_DVI_CSC_0_CSC_COEFF_C01_C00 */
    }
}

#if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3)
/* Build RDC RUL for ram LUT loading
 */
#define BVDC_P_CFC_LUT_DESC_LOADING_REG_NUM   (2)
void BVDC_P_Cfc_BuildRulForLutLoading_isr
    ( BCFC_LutLoadListInfo         *pLutList,
      uint32_t                      ulAddrReg, /* *_LUT_DESC_ADDR */
      uint32_t                      ulCfgReg, /* *_LUT_DESC_CFG */
      BVDC_P_ListInfo              *pList)
{
    if (pLutList->pulCurrent > pLutList->pulStart[pLutList->ulIndex])
    {
        uint32_t ulNumWords = pLutList->pulCurrent - pLutList->pulStart[pLutList->ulIndex];
        BMMA_FlushCache_isr(pLutList->hMmaBlock[pLutList->ulIndex], pLutList->pulStart[pLutList->ulIndex], ulNumWords * sizeof(uint32_t));
        BDBG_MODULE_MSG(BVDC_CFC_4,("Start LUT Loading: regAddr 0x%x, lutRul words %d, lut[%d]=%p", ulAddrReg, ulNumWords,
            pLutList->ulIndex, (void*)pLutList->pulStart[pLutList->ulIndex]));

        BRDC_AddrRul_ImmToReg_isr(&pList->pulCurrent, ulAddrReg, pLutList->ullStartDeviceAddr[pLutList->ulIndex]);
        /* SWSTB-11379: DVI_CFC LUT needs manual disable/enable edge to trigger LUT loading */
        if(ulCfgReg == BCHP_DVI_CFC_0_LUT_DESC_CFG) {
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(ulCfgReg);
            *pList->pulCurrent++ = BCHP_FIELD_ENUM(HDR_CMP_0_LUT_DESC_CFG, ENABLE, DISABLE);
        }
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(ulCfgReg);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(HDR_CMP_0_LUT_DESC_CFG, ENABLE, ENABLE) |
            BCHP_FIELD_DATA(HDR_CMP_0_LUT_DESC_CFG, TABLE_RUL_LENGTH, ulNumWords);

        pLutList->ulIndex = 1 - pLutList->ulIndex;/* double-buffered LUT memory rotation */
        pLutList->pulCurrent = pLutList->pulStart[pLutList->ulIndex]; /* so that no more rul build */
    }
}
#endif /* #if (BVDC_P_CMP_CFC_VER >= BVDC_P_CFC_VER_3) */

/***************************************************************************
 * Return the color space converstion for CSC in HDDVI or CMP from user matrix.
 */
void BVDC_P_Cfc_FromMatrix_isr
    ( BCFC_Context               *pCfc,
      const int32_t               pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      uint32_t                    ulShift )
{
    BCFC_Csc3x4 *pCsc = &pCfc->stMc;

    pCsc->m[0][0]    = BVDC_P_MAKE_CFC_CX_FR_USR(pl32_Matrix[0],  ulShift);
    pCsc->m[0][1]    = BVDC_P_MAKE_CFC_CX_FR_USR(pl32_Matrix[1],  ulShift);
    pCsc->m[0][2]    = BVDC_P_MAKE_CFC_CX_FR_USR(pl32_Matrix[2],  ulShift);
    pCsc->m[0][3]    = BVDC_P_MAKE_CFC_CO_FR_USR(pl32_Matrix[4],  ulShift);

    pCsc->m[1][0]    = BVDC_P_MAKE_CFC_CX_FR_USR(pl32_Matrix[5],  ulShift);
    pCsc->m[1][1]    = BVDC_P_MAKE_CFC_CX_FR_USR(pl32_Matrix[6],  ulShift);
    pCsc->m[1][2]    = BVDC_P_MAKE_CFC_CX_FR_USR(pl32_Matrix[7],  ulShift);
    pCsc->m[1][3]    = BVDC_P_MAKE_CFC_CO_FR_USR(pl32_Matrix[9],  ulShift);

    pCsc->m[2][0]    = BVDC_P_MAKE_CFC_CX_FR_USR(pl32_Matrix[10], ulShift);
    pCsc->m[2][1]    = BVDC_P_MAKE_CFC_CX_FR_USR(pl32_Matrix[11], ulShift);
    pCsc->m[2][2]    = BVDC_P_MAKE_CFC_CX_FR_USR(pl32_Matrix[12], ulShift);
    pCsc->m[2][3]    = BVDC_P_MAKE_CFC_CO_FR_USR(pl32_Matrix[14], ulShift);

#if (BDBG_DEBUG_BUILD)
    BDBG_MODULE_MSG(BVDC_CFC_3,("%s-Cfc%d using user Matrix:", BCFC_GetCfcName_isrsafe(pCfc->eId), pCfc->ucMosaicSlotIdx));
    BCFC_PrintFloatCscRx4_isrsafe(pCsc, NULL);
#endif
    return;
}

#if !B_REFSW_MINIMAL
/***************************************************************************
 * Return the user matrix from vdec color space coverstion table.
 */
void BVDC_P_Cfc_ToMatrix_isr
    ( int32_t                          pl32_Matrix[BVDC_CSC_COEFF_COUNT],
      const BCFC_Csc3x4               *pCsc,
      uint32_t                         ulShift )
{
    pl32_Matrix[0]  = BVDC_P_MAKE_CFC_CX_TO_USR(pCsc->m[0][0], ulShift);
    pl32_Matrix[1]  = BVDC_P_MAKE_CFC_CX_TO_USR(pCsc->m[0][1], ulShift);
    pl32_Matrix[2]  = BVDC_P_MAKE_CFC_CX_TO_USR(pCsc->m[0][2], ulShift);
    pl32_Matrix[4]  = BVDC_P_MAKE_CFC_CO_TO_USR(pCsc->m[0][3], ulShift);

    pl32_Matrix[5]  = BVDC_P_MAKE_CFC_CX_TO_USR(pCsc->m[1][0], ulShift);
    pl32_Matrix[6]  = BVDC_P_MAKE_CFC_CX_TO_USR(pCsc->m[1][1], ulShift);
    pl32_Matrix[7]  = BVDC_P_MAKE_CFC_CX_TO_USR(pCsc->m[1][2], ulShift);
    pl32_Matrix[9]  = BVDC_P_MAKE_CFC_CO_TO_USR(pCsc->m[1][3], ulShift);

    pl32_Matrix[10] = BVDC_P_MAKE_CFC_CX_TO_USR(pCsc->m[2][0], ulShift);
    pl32_Matrix[11] = BVDC_P_MAKE_CFC_CX_TO_USR(pCsc->m[2][1], ulShift);
    pl32_Matrix[12] = BVDC_P_MAKE_CFC_CX_TO_USR(pCsc->m[2][2], ulShift);
    pl32_Matrix[14] = BVDC_P_MAKE_CFC_CO_TO_USR(pCsc->m[2][3], ulShift);

    return;
}
#endif

/***************************************************************************
 * Return the desired matrices for converting between YCbCr and R'G'B' for
 * BVDC_P_Cfc_ApplyAttenuationRGB_isr
 *
 */
void BVDC_P_Cfc_GetCfcToApplyAttenuationRGB_isr
    ( BCFC_Colorimetry               eColorimetry,
      const BCFC_Csc3x4            **ppYCbCrToRGB,
      const BCFC_Csc3x4            **ppRGBToYCbCr )
{
    /* Ouptut debug msgs */
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("BVDC_P_Cfc_GetCfcTables_YCbCr_RGB_isr:"));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("eColorimetry         = %d", eColorimetry));

    *ppYCbCrToRGB = BCFC_GetCsc3x4_Ma_isrsafe(eColorimetry);
    BDBG_ASSERT(*ppYCbCrToRGB);
    *ppRGBToYCbCr = BCFC_GetCsc3x4_Mc_isrsafe(eColorimetry);
    BDBG_ASSERT(*ppRGBToYCbCr);

    return;
}

/* Color adjustment range coefficients */
/* These values have been changed several times to comply with different
 * requests.  The following describes how to convert contrast, saturation,
 * hue, and brightness values between code with different minimums and
 * maximum values.
 *
 * Saturation and Contrast
 *
 * for values < 0:  new value = old value(1 - old min/1 - new min)
 * for values > 0:  new value = old value(old max - 1/new max - 1)
 *
 * for saturation,  min = (1 - BVDC_P_SATURATION_FIX_KA_RANGE) or 0 if it does not exist.
 *                  max = (1 + BVDC_P_SATURATION_FIX_KA_RANGE) or old BVDC_P_CFC_SATURATION_FIX_KA_MAX.
 *
 * for contrast,    min = BVDC_P_CFC_CONTRAST_FIX_K_MIN
 *                  max = BVDC_P_CFC_CONTRAST_FIX_K_MAX
 *
 *
 * Hue and Brightness
 *
 * new value = old value(old max/new max)
 *
 * for hue,         max = BVDC_P_CFC_HUE_FIX_KH_MAX
 *
 * for brightness,  max = BVDC_P_CFC_BRIGHTNESS_MAX
 */
#define BVDC_P_CFC_CONTRAST_FIX_K_MIN     0x0 /* 0.0 */
#define BVDC_P_CFC_CONTRAST_FIX_K_MAX     (BVDC_P_CFC_ITOFIX(4)) /* 4.0 */
#define BVDC_P_CFC_LUMA_BLACK_OFFSET      (64  >> (10 - BVDC_P_CFC_VIDEO_DATA_BITS))
#define BVDC_P_CFC_CHROMA_BLACK_OFFSET    (512 >> (10 - BVDC_P_CFC_VIDEO_DATA_BITS))
#define BVDC_P_CFC_BRIGHTNESS_MAX         (1 << BVDC_P_CFC_VIDEO_DATA_BITS)
#define BVDC_P_CFC_SATURATION_FIX_KA_MAX  BVDC_P_CFC_ITOFIX(4) /* 4.0 */
#define BVDC_P_CFC_HUE_FIX_KH_MAX         BVDC_P_CFC_FIX_PI /* 180 degrees */

/* convert CX coeffs to common fixed notation */
#define BVDC_P_CFC_CXTOFIX(x) \
    (BMTH_FIX_SIGNED_CONVERT_64_isrsafe(x, BCFC_CSC_SW_CX_F_BITS, BVDC_P_CFC_FIX_FRACTION_BITS))

/* convert CO offsets to common fixed notation */
#define BVDC_P_CFC_COTOFIX(x) \
    (BMTH_FIX_SIGNED_CONVERT_64_isrsafe(x, BCFC_CSC_SW_CO_F_BITS, BVDC_P_CFC_FIX_FRACTION_BITS))

/* convert common fixed notation to CX coeffs */
#define BVDC_P_CFC_FIXTOCX(x) \
    (BMTH_FIX_SIGNED_CONVERT_64_isrsafe(x, BVDC_P_CFC_FIX_FRACTION_BITS, BCFC_CSC_SW_CX_F_BITS))

/* convert common fixed notation to CO offsets */
#define BVDC_P_CFC_FIXTOCO(x) \
    (BMTH_FIX_SIGNED_CONVERT_64_isrsafe(x, BVDC_P_CFC_FIX_FRACTION_BITS, BCFC_CSC_SW_CO_F_BITS))

/* fixed point operation multiply */
#define BVDC_P_CFC_FIX_MUL(x, y) \
    BMTH_FIX_SIGNED_MUL_64_isrsafe(x, y, BVDC_P_CFC_FIX_FRACTION_BITS, \
                           BVDC_P_CFC_FIX_FRACTION_BITS, BVDC_P_CFC_FIX_FRACTION_BITS)

#define BVDC_P_CFC_FIX_MUL_OFFSET(x, y) \
    BMTH_FIX_SIGNED_MUL_64_isrsafe(x, BVDC_P_CFC_FIXTOCO(y), BVDC_P_CFC_FIX_FRACTION_BITS, \
                           BCFC_CSC_SW_CO_F_BITS, BVDC_P_CFC_FIX_FRACTION_BITS)

/* sin, with linear interpolation */
#define BVDC_P_CFC_FIX_SIN(x) \
    BMTH_FIX_SIGNED_SIN_64_isrsafe(x, BCFC_CSC_SW_CX_I_BITS, BCFC_CSC_SW_CX_F_BITS, \
                           BCFC_CSC_SW_CX_F_BITS)

/* cos, with linear interpolation */
#define BVDC_P_CFC_FIX_COS(x) \
    BMTH_FIX_SIGNED_COS_64_isrsafe(x, BCFC_CSC_SW_CX_I_BITS, BCFC_CSC_SW_CX_F_BITS, \
                           BCFC_CSC_SW_CX_F_BITS)

/* Convert csc matrix object to 4x4 matrix of fixed point values */
#define BVDC_P_CFC_MAKE4X4(matrix4x4, cscmatrix)                 \
    matrix4x4[0][0] = BVDC_P_CFC_CXTOFIX(cscmatrix->m[0][0]);    \
    matrix4x4[0][1] = BVDC_P_CFC_CXTOFIX(cscmatrix->m[0][1]);    \
    matrix4x4[0][2] = BVDC_P_CFC_CXTOFIX(cscmatrix->m[0][2]);    \
    matrix4x4[0][3] = BVDC_P_CFC_COTOFIX(cscmatrix->m[0][3]);    \
                                                                 \
    matrix4x4[1][0] = BVDC_P_CFC_CXTOFIX(cscmatrix->m[1][0]);    \
    matrix4x4[1][1] = BVDC_P_CFC_CXTOFIX(cscmatrix->m[1][1]);    \
    matrix4x4[1][2] = BVDC_P_CFC_CXTOFIX(cscmatrix->m[1][2]);    \
    matrix4x4[1][3] = BVDC_P_CFC_COTOFIX(cscmatrix->m[1][3]);    \
                                                                 \
    matrix4x4[2][0] = BVDC_P_CFC_CXTOFIX(cscmatrix->m[2][0]);    \
    matrix4x4[2][1] = BVDC_P_CFC_CXTOFIX(cscmatrix->m[2][1]);    \
    matrix4x4[2][2] = BVDC_P_CFC_CXTOFIX(cscmatrix->m[2][2]);    \
    matrix4x4[2][3] = BVDC_P_CFC_COTOFIX(cscmatrix->m[2][3]);    \
                                                                 \
    matrix4x4[3][0] = 0;                                         \
    matrix4x4[3][1] = 0;                                         \
    matrix4x4[3][2] = 0;                                         \
    matrix4x4[3][3] = BVDC_P_CFC_ITOFIX(1)

/* ColorTemp calculation Linear Model */
#define BVDC_P_CFC_MAKE_CLRTEMP_LMODEL(R0, R1,                      \
                                   G0, G1,                          \
                                   B0, B1,                          \
                                   cx_i_bits, cx_f_bits)            \
{                                                                   \
    {                                                               \
        BMTH_FIX_SIGNED_FTOFIX(R0, cx_i_bits, cx_f_bits),           \
        BMTH_FIX_SIGNED_FTOFIX(R1, cx_i_bits, cx_f_bits)            \
    },                                                              \
    {                                                               \
        BMTH_FIX_SIGNED_FTOFIX(G0, cx_i_bits, cx_f_bits),           \
        BMTH_FIX_SIGNED_FTOFIX(G1, cx_i_bits, cx_f_bits)            \
    },                                                              \
    {                                                               \
        BMTH_FIX_SIGNED_FTOFIX(B0, cx_i_bits, cx_f_bits),           \
        BMTH_FIX_SIGNED_FTOFIX(B1, cx_i_bits, cx_f_bits)            \
    }                                                               \
}

/* Color Temperature - Piecewise Linear Model Parameters - (Assuming Temp is in 100s of K) */
/* Low Temp Model - Under 6500 K
    Atten_R = (0 * Temp + 100)/100
    Atten_G = (0.62695 * Temp + 59.223)/100
    Atten_B = (1.33301 * Temp + 13.333)/100      */
#define BVDC_P_CFC_MAKE_CLRTEMP_LMODEL_PARAML(f_bits)          \
    BVDC_P_CFC_MAKE_CLRTEMP_LMODEL                             \
        (  0.00000, 100.000,                                   \
           0.62695,  59.223,                                   \
           1.33301,  13.333,                                   \
           BCFC_CSC_SW_CO_I_BITS, f_bits)

/* High Temp Model - Over 6500 K
    Atten_R = (-0.57422 * Temp + 137.328)/100
    Atten_G = (-0.44727 * Temp + 129.134)/100
    Atten_B = (0 * Temp + 100)/100      */
#define BVDC_P_CFC_MAKE_CLRTEMP_LMODEL_PARAMH(f_bits)         \
    BVDC_P_CFC_MAKE_CLRTEMP_LMODEL                            \
        ( -0.57422, 137.328,                                  \
          -0.44727, 129.134,                                  \
           0.00000, 100.000,                                  \
           BCFC_CSC_SW_CO_I_BITS, f_bits)

static const int32_t s_Cfc_ClrTemp_LModel_ParamL_Cmp[3][2] =
    BVDC_P_CFC_MAKE_CLRTEMP_LMODEL_PARAML(BCFC_CSC_SW_CO_F_BITS);

static const int32_t s_Cfc_ClrTemp_LModel_ParamH_Cmp[3][2] =
    BVDC_P_CFC_MAKE_CLRTEMP_LMODEL_PARAMH(BCFC_CSC_SW_CO_F_BITS);

/***************************************************************************
 * Convert color temperature to attenuation RGB
 */
BERR_Code BVDC_P_Cfc_ColorTempToAttenuationRGB
    ( int16_t                          sColorTemp,
      int32_t                         *plAttenuationR,
      int32_t                         *plAttenuationG,
      int32_t                         *plAttenuationB,
      BCFC_Csc3x4                     *pCscCoeffs )
{
    int64_t lSlope, lKelvin;
    int64_t lAttenuationR;
    int64_t lAttenuationG;
    int64_t lAttenuationB;

    /* Maximum and Minimum Values of Brightness Slider
      (Presumably these would be BVDC_P_BRIGHTNESS_VAL_MAX and BVDC_P_BRIGHTNESS_VAL_MIN,
       however this is not the case at the moment) */

    int32_t lColorTempMax = BVDC_P_COLORTEMP_VAL_MAX;
    int32_t lColorTempMin = BVDC_P_COLORTEMP_VAL_MIN;
    int64_t lColorTempCenter;

    /* Maximum, Minimum, and Center Values of Color Temperature in 100s of Kelvin */
    int32_t lKelvinMax    = BVDC_P_COLORTEMP_KELVIN_MAX;
    int32_t lKelvinMin    = BVDC_P_COLORTEMP_KELVIN_MIN;
    int32_t lKelvinCenter = BVDC_P_COLORTEMP_KELVIN_CENTER;


    /* Color Temperature - Piecewise Linear Model Parameters - (Assuming Temp is in 100s of K) */
    int64_t lModel_ParamL[3][2];
    int64_t lModel_ParamH[3][2];
    int i, j;
    int32_t lFixOne = BVDC_P_CFC_ITOFIX(1);

    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 2; j++)
        {
            lModel_ParamL[i][j] = BMTH_FIX_SIGNED_CONVERT_64_isrsafe(s_Cfc_ClrTemp_LModel_ParamL_Cmp[i][j],
                BCFC_CSC_SW_CO_F_BITS, BVDC_P_CFC_FIX_FRACTION_BITS);
            lModel_ParamH[i][j] = BMTH_FIX_SIGNED_CONVERT_64_isrsafe(s_Cfc_ClrTemp_LModel_ParamH_Cmp[i][j],
                BCFC_CSC_SW_CO_F_BITS, BVDC_P_CFC_FIX_FRACTION_BITS);
        }
    }

    lColorTempCenter = (lColorTempMin + lColorTempMax)/2;

    if(sColorTemp < lColorTempCenter) {
        lSlope = (lColorTempCenter - lColorTempMin)/(lKelvinCenter - lKelvinMin);
    }
    else {
        lSlope = (BVDC_P_COLORTEMP_VAL_MAX - lColorTempCenter)/(lKelvinMax - lKelvinCenter);
    }

    lKelvin = sColorTemp/lSlope - lColorTempCenter/lSlope + lKelvinCenter;

    /* Determine Attenuation Factors Using Piecewise Linear Model of Color Temperature */
    if(lKelvin < lKelvinCenter) {
        lAttenuationR = ((lModel_ParamL[0][0] * lKelvin) + lModel_ParamL[0][1]) / 100;
        lAttenuationG = ((lModel_ParamL[1][0] * lKelvin) + lModel_ParamL[1][1]) / 100;
        lAttenuationB = ((lModel_ParamL[2][0] * lKelvin) + lModel_ParamL[2][1]) / 100;
    }
    else
    {
        lAttenuationR = ((lModel_ParamH[0][0] * lKelvin) + lModel_ParamH[0][1]) / 100;
        lAttenuationG = ((lModel_ParamH[1][0] * lKelvin) + lModel_ParamH[1][1]) / 100;
        lAttenuationB = ((lModel_ParamH[2][0] * lKelvin) + lModel_ParamH[2][1]) / 100;
    }

    /* Ensure Attenuation Factors are Between 0 and 1 */
    lAttenuationR = (lAttenuationR < 0) ? 0 : (lAttenuationR > lFixOne) ? lFixOne : lAttenuationR;
    lAttenuationG = (lAttenuationG < 0) ? 0 : (lAttenuationG > lFixOne) ? lFixOne : lAttenuationG;
    lAttenuationB = (lAttenuationB < 0) ? 0 : (lAttenuationB > lFixOne) ? lFixOne : lAttenuationB;

    *plAttenuationR = lAttenuationR;
    *plAttenuationG = lAttenuationG;
    *plAttenuationB = lAttenuationB;
    *pCscCoeffs     = *(BCFC_GetCsc3x4_Identity_isrsafe());

    return BERR_SUCCESS;
}

/***************************************************************************
 * Apply the contrast calculation to the color matrix
 */
void BVDC_P_Cfc_ApplyContrast_isr
    ( int16_t                          sContrast,
      BCFC_Csc3x4                     *pCscCoeffs )
{
    int64_t lFixK;
    int64_t lFixKMin = BVDC_P_CFC_CONTRAST_FIX_K_MIN;
    int64_t lFixKMax = BVDC_P_CFC_CONTRAST_FIX_K_MAX;
    int64_t lFixOne  = BVDC_P_CFC_ITOFIX(1);

    int64_t lFixYOffset;
#if (BVDC_SUPPORT_CONTRAST_WITH_CBCR)
    int64_t lFixCbOffset;
    int64_t lFixCrOffset;
#endif

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Apply contrast = %d:", sContrast));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Input CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));

    if(sContrast == 0)
    {
        BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
            pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
            pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
            pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
        return;
    }

    /* K of 1.0 is no contrast adjustment.
     * K changes linearly from Kmin to 1 with input contrast from -32768 to 0
     */
    if (sContrast <= 0)
    {
        lFixK = (((int64_t)(lFixOne - lFixKMin) * (sContrast - BVDC_P_CONTRAST_VAL_MIN)) /
                 -BVDC_P_CONTRAST_VAL_MIN
                )
                + lFixKMin;
    }
    /* K changes linearly from slightly greater than 1.0 to KMax with input contrast from 1 to 32767 */
    else
    {
        lFixK = (((int64_t)(lFixKMax - lFixOne) * sContrast) /
                 BVDC_P_CONTRAST_VAL_MAX
                )
                + lFixOne;
    }
    lFixYOffset  = BVDC_P_CFC_FIX_MUL_OFFSET(lFixK, BVDC_P_CFC_COTOFIX(pCscCoeffs->m[0][3])) +
                   (BVDC_P_CFC_LUMA_BLACK_OFFSET * (lFixOne - lFixK));

#if (BVDC_SUPPORT_CONTRAST_WITH_CBCR)
    lFixCbOffset = BVDC_P_CFC_FIX_MUL_OFFSET(lFixK, BVDC_P_CFC_COTOFIX(pCscCoeffs->m[1][3])) +
                   (BVDC_P_CFC_CHROMA_BLACK_OFFSET * (lFixOne - lFixK));

    lFixCrOffset = BVDC_P_CFC_FIX_MUL_OFFSET(lFixK, BVDC_P_CFC_COTOFIX(pCscCoeffs->m[2][3])) +
                   (BVDC_P_CFC_CHROMA_BLACK_OFFSET * (lFixOne - lFixK));
#endif

    /* Y */
    pCscCoeffs->m[0][0] = BVDC_P_CFC_FIXTOCX(
                                 BVDC_P_CFC_FIX_MUL(lFixK, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[0][0])));
    pCscCoeffs->m[0][1] = BVDC_P_CFC_FIXTOCX(
                                 BVDC_P_CFC_FIX_MUL(lFixK, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[0][1])));
    pCscCoeffs->m[0][2] = BVDC_P_CFC_FIXTOCX(
                                 BVDC_P_CFC_FIX_MUL(lFixK, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[0][2])));

    pCscCoeffs->m[0][3] = BVDC_P_CFC_FIXTOCO(lFixYOffset);

#if (BVDC_SUPPORT_CONTRAST_WITH_CBCR)
    /* Cb */
    pCscCoeffs->m[1][0] = BVDC_P_CFC_FIXTOCX(
                                 BVDC_P_CFC_FIX_MUL(lFixK, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][0])));
    pCscCoeffs->m[1][1] = BVDC_P_CFC_FIXTOCX(
                                 BVDC_P_CFC_FIX_MUL(lFixK, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][1])));
    pCscCoeffs->m[1][2] = BVDC_P_CFC_FIXTOCX(
                                 BVDC_P_CFC_FIX_MUL(lFixK, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][2])));

    pCscCoeffs->m[1][3] = BVDC_P_CFC_FIXTOCO(lFixCbOffset);

    /* Cr */
    pCscCoeffs->m[2][0] = BVDC_P_CFC_FIXTOCX(
                                 BVDC_P_CFC_FIX_MUL(lFixK, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][0])));
    pCscCoeffs->m[2][1] = BVDC_P_CFC_FIXTOCX(
                                 BVDC_P_CFC_FIX_MUL(lFixK, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][1])));
    pCscCoeffs->m[2][2] = BVDC_P_CFC_FIXTOCX(
                                 BVDC_P_CFC_FIX_MUL(lFixK, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][2])));

    pCscCoeffs->m[2][3] = BVDC_P_CFC_FIXTOCO(lFixCrOffset);
#endif

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
    BDBG_MODULE_MSG(BVDC_CFC_3,("Applied contrast = %d", sContrast));

    return;
}

/***************************************************************************
 * Apply the Saturation and Hue calculation to color matrix
 */
void BVDC_P_Cfc_ApplySaturationAndHue_isr
    ( int16_t                          sSaturation,
      int16_t                          sHue,
      BCFC_Csc3x4                     *pCscCoeffs )
{
    int64_t lTmpCb0;
    int64_t lTmpCb1;
    int64_t lTmpCb2;
    int64_t lTmpCbOffset;

    int64_t lTmpCr0;
    int64_t lTmpCr1;
    int64_t lTmpCr2;
    int64_t lTmpCrOffset;

    int64_t lFixKa;
    int64_t lFixKt;

    int64_t lFixKaMax = BVDC_P_CFC_SATURATION_FIX_KA_MAX;
    int64_t lFixKhMax = BVDC_P_CFC_HUE_FIX_KH_MAX;

    int64_t lFixKSinKt;
    int64_t lFixKCosKt;
    int64_t lFixC0;
    int64_t lFixC1;

    int64_t lFixOne = BVDC_P_CFC_ITOFIX(1);

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Apply sat = %d, hue = %d:", sSaturation, sHue));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Input CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));

    if(sSaturation == 0 && sHue == 0)
    {
        BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
            pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
            pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
            pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
        return;
    }

    /**
     * Ka of 1.0 is no saturation adjustment.
     * Ka changes linearly with input saturation value, from 0 to 1.0 when saturation
     * is negative, and from 1.0 to 4 when saturation is positive.
     *
     * With KaMax = 4, minimum saturation = -32768, maximum saturation = 32767:
     * -32768 input saturation equals Ka of 0
     *      0 input saturation equals Ka of 1
     *  32767 input saturation equals Ka of 4
     */

    if (sSaturation <= 0)
    {
        lFixKa = (lFixOne * (sSaturation - BVDC_P_CONTRAST_VAL_MIN)) /
                 -BVDC_P_SATURATION_VAL_MIN;
    }
    else
    {
        lFixKa = ((((lFixKaMax - lFixOne) * sSaturation) /
                   BVDC_P_SATURATION_VAL_MAX))
                 + lFixOne;
    }

    /**
     * Kt of 0 is no hue adjustment.
     * Kt changes linearly with input hue value, bounded by +/- Khmax.
     * hue of -32768 is clamped to -32767.
     *
     * With KhMax = pi, minimum hue = -32767, maximum hue = 32767:
     * -32767 input hue equals Kt of -pi
     *      0 input hue equals Kt of 0
     *  32767 input hue equals Kt of pi
     */
    if (sHue < -BVDC_P_HUE_VAL_MAX)
    {
        sHue = -BVDC_P_HUE_VAL_MAX;
    }

    lFixKt = (lFixKhMax * sHue) / BVDC_P_HUE_VAL_MAX;

    lFixKSinKt = BVDC_P_CFC_FIX_SIN(lFixKt);
    lFixKCosKt = BVDC_P_CFC_FIX_COS(lFixKt);

    lFixKa     = BVDC_P_CFC_CXTOFIX(lFixKa);
    lFixOne    = BVDC_P_CFC_CXTOFIX(lFixOne);
    lFixKt     = BVDC_P_CFC_CXTOFIX(lFixKt);
    lFixKSinKt = BVDC_P_CFC_CXTOFIX(lFixKSinKt);
    lFixKCosKt = BVDC_P_CFC_CXTOFIX(lFixKCosKt);

    lFixKSinKt = BVDC_P_CFC_FIX_MUL(lFixKa , lFixKSinKt);
    lFixKCosKt = BVDC_P_CFC_FIX_MUL(lFixKa , lFixKCosKt);

    /* offset coefficient is stored in integer format */
    lFixC0 = ((1 << (BVDC_P_CFC_VIDEO_DATA_BITS - 1)) * (lFixKSinKt - lFixKCosKt + lFixOne));
    lFixC1 = ((1 << (BVDC_P_CFC_VIDEO_DATA_BITS - 1)) * (-lFixKSinKt - lFixKCosKt + lFixOne));

    lTmpCb0      = BVDC_P_CFC_FIX_MUL(lFixKCosKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][0])) -
                   BVDC_P_CFC_FIX_MUL(lFixKSinKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][0]));
    lTmpCb1      = BVDC_P_CFC_FIX_MUL(lFixKCosKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][1])) -
                   BVDC_P_CFC_FIX_MUL(lFixKSinKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][1]));
    lTmpCb2      = BVDC_P_CFC_FIX_MUL(lFixKCosKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][2])) -
                   BVDC_P_CFC_FIX_MUL(lFixKSinKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][2]));
    lTmpCbOffset = BVDC_P_CFC_FIX_MUL_OFFSET(lFixKCosKt, BVDC_P_CFC_COTOFIX(pCscCoeffs->m[1][3])) -
                   BVDC_P_CFC_FIX_MUL_OFFSET(lFixKSinKt, BVDC_P_CFC_COTOFIX(pCscCoeffs->m[2][3])) +
                   lFixC0;

    lTmpCr0      = BVDC_P_CFC_FIX_MUL(lFixKSinKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][0])) +
                   BVDC_P_CFC_FIX_MUL(lFixKCosKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][0]));
    lTmpCr1      = BVDC_P_CFC_FIX_MUL(lFixKSinKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][1])) +
                   BVDC_P_CFC_FIX_MUL(lFixKCosKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][1]));
    lTmpCr2      = BVDC_P_CFC_FIX_MUL(lFixKSinKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][2])) +
                   BVDC_P_CFC_FIX_MUL(lFixKCosKt, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][2]));
    lTmpCrOffset = BVDC_P_CFC_FIX_MUL_OFFSET(lFixKSinKt, BVDC_P_CFC_COTOFIX(pCscCoeffs->m[1][3])) +
                   BVDC_P_CFC_FIX_MUL_OFFSET(lFixKCosKt, BVDC_P_CFC_COTOFIX(pCscCoeffs->m[2][3])) +
                   lFixC1;

    pCscCoeffs->m[1][0] = BVDC_P_CFC_FIXTOCX(lTmpCb0);
    pCscCoeffs->m[1][1] = BVDC_P_CFC_FIXTOCX(lTmpCb1);
    pCscCoeffs->m[1][2] = BVDC_P_CFC_FIXTOCX(lTmpCb2);
    pCscCoeffs->m[1][3] = BVDC_P_CFC_FIXTOCO(lTmpCbOffset);

    pCscCoeffs->m[2][0] = BVDC_P_CFC_FIXTOCX(lTmpCr0);
    pCscCoeffs->m[2][1] = BVDC_P_CFC_FIXTOCX(lTmpCr1);
    pCscCoeffs->m[2][2] = BVDC_P_CFC_FIXTOCX(lTmpCr2);
    pCscCoeffs->m[2][3] = BVDC_P_CFC_FIXTOCO(lTmpCrOffset);

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
    BDBG_MODULE_MSG(BVDC_CFC_3,("Applied sat = %d, hue = %d", sSaturation, sHue));

    return;
}

/***************************************************************************
 * Apply brightness calculation to color matrix
 */
void BVDC_P_Cfc_ApplyBrightness_isr
    ( int16_t                          sBrightness,
      BCFC_Csc3x4                     *pCscCoeffs )
{
    int64_t sK;
    uint32_t ulCoFBits = BCFC_CSC_CO_F_BITS(BCFC_CscType_eMc3x4_16);

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Apply brightness = %d:", sBrightness));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Input CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));

    if(sBrightness == 0)
    {
        BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
            pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
            pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
            pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
        return;
    }

    /* brightness of -32768 clamped to -32767. */
    if (sBrightness < -BVDC_P_BRIGHTNESS_VAL_MAX)
    {
        sBrightness = -BVDC_P_BRIGHTNESS_VAL_MAX;
    }

    /* sK varies linearly from -KMax to KMax based on input brightness */
    sK = (int64_t)(sBrightness << ulCoFBits) * BVDC_P_CFC_BRIGHTNESS_MAX /
         BVDC_P_BRIGHTNESS_VAL_MAX;

    pCscCoeffs->m[0][3] += (sK) << (BCFC_CSC_SW_CO_F_BITS - ulCoFBits);

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
    BDBG_MODULE_MSG(BVDC_CFC_3,("Applied brightness = %d", sBrightness));

    return;
}

typedef struct BVDC_P_BufForApplyAttenuationRGB
{
    int64_t M0[4][4];
    int64_t M1[4][4];
    int64_t M2[4][4];
    int64_t MTmp[4][4];

} BVDC_P_BufForApplyAttenuationRGB;

/***************************************************************************
 * Multiplies two csc matrices set up in 4x4 format.
 */
static void BVDC_P_Csc_Mult_64_isr
    ( int64_t                          *pM,  /* matrix M element buf ptr */
      const int64_t                    *pA,  /* matrix A element buf ptr */
      const int64_t                    *pB)  /* matrix B element buf ptr */

{
    int i, j, k;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            uint64_t t = 0;
            for (k = 0; k < 4; k++)
            {
                if (j == 3)
                {
                    t += BVDC_P_CFC_FIX_MUL_OFFSET(*(pA + i * 4 + k), *(pB + k * 4 + j));
                }
                else
                {
                    t += BVDC_P_CFC_FIX_MUL(*(pA + i * 4 + k), *(pB + k * 4 + j));
                }
            }
            *(pM + i * 4 + j) = *(int64_t *)(&t);
        }
    }
}

/***************************************************************************
 * Apply RGB attenuation calculation to color matrix
 */
void BVDC_P_Cfc_ApplyAttenuationRGB_isr
    ( int32_t                          lAttenuationR,
      int32_t                          lAttenuationG,
      int32_t                          lAttenuationB,
      int32_t                          lOffsetR,
      int32_t                          lOffsetG,
      int32_t                          lOffsetB,
      BCFC_Csc3x4                     *pCscCoeffs,
      const BCFC_Csc3x4               *pYCbCrToRGB,
      const BCFC_Csc3x4               *pRGBToYCbCr,
      bool                             bUserCsc,
      void                            *pTmpBuf)
{
    BVDC_P_BufForApplyAttenuationRGB *pTmp = (BVDC_P_BufForApplyAttenuationRGB *) pTmpBuf;

    BDBG_CASSERT(sizeof(BVDC_P_BufForApplyAttenuationRGB) <= BVDC_P_WIN_TMP_BUF_SIZE);
    BDBG_ASSERT(pCscCoeffs);
    BDBG_ASSERT(pYCbCrToRGB);
    BDBG_ASSERT(pRGBToYCbCr);
    BDBG_ASSERT(pTmpBuf);

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Apply RGB Attenuation R=%x, G=%x, B=%x, Offset R=%d, G=%d, B=%d:",
        lAttenuationR, lAttenuationG, lAttenuationB, lOffsetR, lOffsetG, lOffsetB));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("YCbCrToRGB matrix:"));
    BCFC_PrintFloatMcAdj_isrsafe(pYCbCrToRGB, NULL);
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("RGBToYCbCr matrix:"));
    BCFC_PrintFloatMcAdj_isrsafe(pRGBToYCbCr, NULL);
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Input CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
    BCFC_PrintFloatMcAdj_isrsafe(pCscCoeffs, NULL);

    if(lAttenuationB == (int32_t)BVDC_P_CFC_ITOFIX(1) &&
       lAttenuationG == (int32_t)BVDC_P_CFC_ITOFIX(1) &&
       lAttenuationR == (int32_t)BVDC_P_CFC_ITOFIX(1) &&
       lOffsetB == 0 &&
       lOffsetG == 0 &&
       lOffsetR == 0)
    {
        BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC 1 : %x %x %x %x, %x %x %x %x, %x %x %x %x",
            pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
            pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
            pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
        BCFC_PrintFloatMcAdj_isrsafe(pCscCoeffs, NULL);
        return;
    }
    else if (bUserCsc)
    {
        BDBG_ERR(("Color Temp or RGB Attenuation adjustment requires BVDC_Window_SetColorMatrixNonLinearC to be set when BVDC_Window_SetColorMatrix is used."));
        return;
    }

    /* M0 = Original CSC Matrix */
    BVDC_P_CFC_MAKE4X4(pTmp->M0, pCscCoeffs);

    /* M1 = YCrCb to RGB Matrix */
    BVDC_P_CFC_MAKE4X4(pTmp->M1, pYCbCrToRGB);

    pTmp->M1[0][0] = BVDC_P_CFC_FIX_MUL(lAttenuationR, pTmp->M1[0][0]);
    pTmp->M1[0][1] = BVDC_P_CFC_FIX_MUL(lAttenuationR, pTmp->M1[0][1]);
    pTmp->M1[0][2] = BVDC_P_CFC_FIX_MUL(lAttenuationR, pTmp->M1[0][2]);
    pTmp->M1[0][3] = BVDC_P_CFC_FIX_MUL_OFFSET(lAttenuationR, pTmp->M1[0][3]) + lOffsetR ;

    pTmp->M1[1][0] = BVDC_P_CFC_FIX_MUL(lAttenuationG, pTmp->M1[1][0]);
    pTmp->M1[1][1] = BVDC_P_CFC_FIX_MUL(lAttenuationG, pTmp->M1[1][1]);
    pTmp->M1[1][2] = BVDC_P_CFC_FIX_MUL(lAttenuationG, pTmp->M1[1][2]);
    pTmp->M1[1][3] = BVDC_P_CFC_FIX_MUL_OFFSET(lAttenuationG, pTmp->M1[1][3]) + lOffsetG ;

    pTmp->M1[2][0] = BVDC_P_CFC_FIX_MUL(lAttenuationB, pTmp->M1[2][0]);
    pTmp->M1[2][1] = BVDC_P_CFC_FIX_MUL(lAttenuationB, pTmp->M1[2][1]);
    pTmp->M1[2][2] = BVDC_P_CFC_FIX_MUL(lAttenuationB, pTmp->M1[2][2]);
    pTmp->M1[2][3] = BVDC_P_CFC_FIX_MUL_OFFSET(lAttenuationB, pTmp->M1[2][3]) + lOffsetB ;

    /* M2 = RGB to YCrCb Matrix */
    BVDC_P_CFC_MAKE4X4(pTmp->M2, pRGBToYCbCr);

    /* Multiply M2*M1 -> Store in MTmp  */
    BVDC_P_Csc_Mult_64_isr(&pTmp->MTmp[0][0], &pTmp->M2[0][0], &pTmp->M1[0][0]);

    /* Multiply MTmp*M0 -> store in M1 */
    BVDC_P_Csc_Mult_64_isr(&pTmp->M1[0][0], &pTmp->MTmp[0][0], &pTmp->M0[0][0]);

    pCscCoeffs->m[0][0] = BVDC_P_CFC_FIXTOCX(pTmp->M1[0][0]);
    pCscCoeffs->m[0][1] = BVDC_P_CFC_FIXTOCX(pTmp->M1[0][1]);
    pCscCoeffs->m[0][2] = BVDC_P_CFC_FIXTOCX(pTmp->M1[0][2]);
    pCscCoeffs->m[0][3] = BVDC_P_CFC_FIXTOCO(pTmp->M1[0][3]);

    pCscCoeffs->m[1][0] = BVDC_P_CFC_FIXTOCX(pTmp->M1[1][0]);
    pCscCoeffs->m[1][1] = BVDC_P_CFC_FIXTOCX(pTmp->M1[1][1]);
    pCscCoeffs->m[1][2] = BVDC_P_CFC_FIXTOCX(pTmp->M1[1][2]);
    pCscCoeffs->m[1][3] = BVDC_P_CFC_FIXTOCO(pTmp->M1[1][3]);

    pCscCoeffs->m[2][0] = BVDC_P_CFC_FIXTOCX(pTmp->M1[2][0]);
    pCscCoeffs->m[2][1] = BVDC_P_CFC_FIXTOCX(pTmp->M1[2][1]);
    pCscCoeffs->m[2][2] = BVDC_P_CFC_FIXTOCX(pTmp->M1[2][2]);
    pCscCoeffs->m[2][3] = BVDC_P_CFC_FIXTOCO(pTmp->M1[2][3]);

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC 2 : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
    BCFC_PrintFloatMcAdj_isrsafe(pCscCoeffs, NULL);
    BDBG_MODULE_MSG(BVDC_CFC_3,("Applied RGB Attenuation R=%x, G=%x, B=%x, Offset R=%d, G=%d, B=%d",
        lAttenuationR, lAttenuationG, lAttenuationB, lOffsetR, lOffsetG, lOffsetB));

    return;
}

/***************************************************************************
 * Apply RGB attenuation calculation to color matrix, performed in RGB
 * colorspace
 */
void BVDC_P_Cfc_DvoApplyAttenuationRGB_isr
    ( int32_t                          lAttenuationR,
      int32_t                          lAttenuationG,
      int32_t                          lAttenuationB,
      int32_t                          lOffsetR,
      int32_t                          lOffsetG,
      int32_t                          lOffsetB,
      BCFC_Csc3x4                     *pCscCoeffs )
{
    int64_t lNewOffsetR;
    int64_t lNewOffsetG;
    int64_t lNewOffsetB;

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Apply Dvo RGB Attenuation"));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Attenuation R=%d, G=%d, B=%d, Offset R=%d, G=%d, B=%d:",
        lAttenuationR, lAttenuationG, lAttenuationB, lOffsetR, lOffsetG, lOffsetB));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Input CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
    BCFC_PrintFloatMcAdj_isrsafe(pCscCoeffs, NULL);

    if(lAttenuationB == (int32_t)BVDC_P_CFC_ITOFIX(1) &&
       lAttenuationG == (int32_t)BVDC_P_CFC_ITOFIX(1) &&
       lAttenuationR == (int32_t)BVDC_P_CFC_ITOFIX(1) &&
       lOffsetB == 0 &&
       lOffsetG == 0 &&
       lOffsetR == 0)
    {
        BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC: %x %x %x %x, %x %x %x %x, %x %x %x %x",
            pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
            pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
            pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));

        return;
    }

    lNewOffsetR = BVDC_P_CFC_FIX_MUL_OFFSET(lAttenuationR, BVDC_P_CFC_COTOFIX(pCscCoeffs->m[2][3])) + lOffsetR;
    lNewOffsetG = BVDC_P_CFC_FIX_MUL_OFFSET(lAttenuationG, BVDC_P_CFC_COTOFIX(pCscCoeffs->m[0][3])) + lOffsetG;
    lNewOffsetB = BVDC_P_CFC_FIX_MUL_OFFSET(lAttenuationB, BVDC_P_CFC_COTOFIX(pCscCoeffs->m[1][3])) + lOffsetB;

    /* R */
    pCscCoeffs->m[2][0] = BVDC_P_CFC_FIXTOCX(BVDC_P_CFC_FIX_MUL(lAttenuationR, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][0])));
    pCscCoeffs->m[2][1] = BVDC_P_CFC_FIXTOCX(BVDC_P_CFC_FIX_MUL(lAttenuationR, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][1])));
    pCscCoeffs->m[2][2] = BVDC_P_CFC_FIXTOCX(BVDC_P_CFC_FIX_MUL(lAttenuationR, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[2][2])));
    pCscCoeffs->m[2][3] = BVDC_P_CFC_FIXTOCO(lNewOffsetR);

    /* G */
    pCscCoeffs->m[0][0] = BVDC_P_CFC_FIXTOCX(BVDC_P_CFC_FIX_MUL(lAttenuationG, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[0][0])));
    pCscCoeffs->m[0][1] = BVDC_P_CFC_FIXTOCX(BVDC_P_CFC_FIX_MUL(lAttenuationG, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[0][1])));
    pCscCoeffs->m[0][2] = BVDC_P_CFC_FIXTOCX(BVDC_P_CFC_FIX_MUL(lAttenuationG, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[0][2])));
    pCscCoeffs->m[0][3] = BVDC_P_CFC_FIXTOCO(lNewOffsetG);

    /* B */
    pCscCoeffs->m[1][0] = BVDC_P_CFC_FIXTOCX(BVDC_P_CFC_FIX_MUL(lAttenuationB, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][0])));
    pCscCoeffs->m[1][1] = BVDC_P_CFC_FIXTOCX(BVDC_P_CFC_FIX_MUL(lAttenuationB, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][1])));
    pCscCoeffs->m[1][2] = BVDC_P_CFC_FIXTOCX(BVDC_P_CFC_FIX_MUL(lAttenuationB, BVDC_P_CFC_CXTOFIX(pCscCoeffs->m[1][2])));
    pCscCoeffs->m[1][3] = BVDC_P_CFC_FIXTOCO(lNewOffsetB);

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC: %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
    BCFC_PrintFloatMcAdj_isrsafe(pCscCoeffs, NULL);
    BDBG_MODULE_MSG(BVDC_CFC_3,("Applied Dvo RGB Attenuation R=%d, G=%d, B=%d, Offset R=%d, G=%d, B=%d",
        lAttenuationR, lAttenuationG, lAttenuationB, lOffsetR, lOffsetG, lOffsetB));

    return;
}

typedef struct BVDC_P_BufForApplyYCbCrColor
{
    BMTH_FIX_Matrix_64 stMatrix;
    BMTH_FIX_Vector_64 stVector;
    BMTH_FIX_Vector_64 stRetVector;
} BVDC_P_BufForApplyYCbCrColor;

/***************************************************************************
 * Set a matrix to output specified color in its original colorspace.
 */
void BVDC_P_Cfc_ApplyYCbCrColor_isr
    ( BCFC_Csc3x4                     *pCscCoeffs,
      uint32_t                         ulColor0,
      uint32_t                         ulColor1,
      uint32_t                         ulColor2,
      void                            *pTmpBuf)
{
    BVDC_P_BufForApplyYCbCrColor *pTmp = (BVDC_P_BufForApplyYCbCrColor *) pTmpBuf;

    BDBG_CASSERT(sizeof(BVDC_P_BufForApplyYCbCrColor) <= BVDC_P_DISP_TMP_BUF_SIZE);
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Apply YCbCr Color"));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("ulColor0=%d, ulColor1=%d, ulColor2=%d", ulColor0, ulColor1, ulColor2));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Input Matrix:"));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Input CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
    BCFC_PrintFloatMcAdj_isrsafe(pCscCoeffs, NULL);

    BVDC_P_CFC_MAKE4X4(pTmp->stMatrix.data, pCscCoeffs);
    pTmp->stMatrix.ulSize = 4;
    pTmp->stMatrix.ulFractBits = BVDC_P_CFC_FIX_FRACTION_BITS;

    pTmp->stVector.data[0] = ulColor0;
    pTmp->stVector.data[1] = ulColor1;
    pTmp->stVector.data[2] = ulColor2;
    pTmp->stVector.data[3] = 1;
    pTmp->stVector.ulSize = 4;
    pTmp->stVector.ulFractBits = BVDC_P_CFC_FIX_FRACTION_BITS;

    /* multiply before we shift to fract bits to avoid overflow */
    BMTH_FIX_Matrix_MultVector_64(&pTmp->stMatrix, &pTmp->stVector, &pTmp->stRetVector);

    pCscCoeffs->m[0][0] = 0;
    pCscCoeffs->m[0][1] = 0;
    pCscCoeffs->m[0][2] = 0;
    pCscCoeffs->m[0][3] = (pTmp->stRetVector.data[0]) << BCFC_CSC_SW_CO_F_BITS;
    pCscCoeffs->m[1][0] = 0;
    pCscCoeffs->m[1][1] = 0;
    pCscCoeffs->m[1][2] = 0;
    pCscCoeffs->m[1][3] = (pTmp->stRetVector.data[1]) << BCFC_CSC_SW_CO_F_BITS;
    pCscCoeffs->m[2][0] = 0;
    pCscCoeffs->m[2][1] = 0;
    pCscCoeffs->m[2][2] = 0;
    pCscCoeffs->m[2][3] = (pTmp->stRetVector.data[2]) << BCFC_CSC_SW_CO_F_BITS;

    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output Matrix:"));
    BDBG_MODULE_MSG(BVDC_MC_ADJ,("Output CSC : %x %x %x %x, %x %x %x %x, %x %x %x %x",
        pCscCoeffs->m[0][0], pCscCoeffs->m[0][1], pCscCoeffs->m[0][2], pCscCoeffs->m[0][3],
        pCscCoeffs->m[1][0], pCscCoeffs->m[1][1], pCscCoeffs->m[1][2], pCscCoeffs->m[1][3],
        pCscCoeffs->m[2][0], pCscCoeffs->m[2][1], pCscCoeffs->m[2][2], pCscCoeffs->m[2][3]));
    BCFC_PrintFloatMcAdj_isrsafe(pCscCoeffs, NULL);
    BDBG_MODULE_MSG(BVDC_CFC_3,("Applied YCbCr Color ulColor0=%d, ulColor1=%d, ulColor2=%d", ulColor0, ulColor1, ulColor2));
    return;
}

/* **********************************************************************************
 * The following code checks matrix computation accuracy
 * It should typically be turned off
 */
#if BVDC_CFC_CHECK_MATRIX_ACCURACY

/*#define USE_24BITS 1*/
/*#define USE_DAVID_MATRIX 1*/

#if USE_24BITS
#define BVDC_P_TestCscCoeffs      BVDC_P_CscAbCoeffs
#define BVDC_P_MAKE_CMP_CSC_TEST  BVDC_P_MAKE_CMP_CSC_AB
#else
#define BVDC_P_TestCscCoeffs      BVDC_P_CscCoeffs
#define BVDC_P_MAKE_CMP_CSC_TEST  BVDC_P_MAKE_CMP_CSC
#endif

static const BVDC_P_TestCscCoeffs  s_YCbCr_Identity = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,      0.000000,      0.000000,       0.000000,
       0.000000,      1.000000,      0.000000,       0.000000,
       0.000000,      0.000000,      1.000000,       0.000000 );

#if USE_DAVID_MATRIX

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT709_to_240M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,      0.000794,     -0.000522,      -0.034832,
       0.000000,      1.009170,      0.000569,      -1.246612,
       0.000000,     -0.005679,      1.081363,      -9.687514 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT709_to_BT601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000071,     -0.001129,     -0.000167,       0.164707,
      -0.049966,      0.890570,      0.003106,      14.409010,
      -0.022760,      0.007821,      0.661894,      42.640635 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT709_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,      0.073865,      0.201922,     -35.300713,
       0.000000,      0.997746,     -0.116268,      15.170838,
       0.000000,     -0.059692,      1.067875,      -1.047420 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT709_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  0.999684,      0.099819,      0.165482,     -33.953438,
       0.000710,      1.004289,     -0.092364,      11.262150,
      -0.002116,     -0.078441,      0.937639,      18.056537 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_240M_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,     -0.000784,      0.000483,       0.038535,
       0.000000,      0.990910,     -0.000521,       1.230231,
       0.000000,      0.005204,      0.924756,       8.965078 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_240M_to_BT601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000071,     -0.001904,      0.000330,       0.200362,
      -0.049966,      0.882530,      0.002384,      15.530533,
      -0.022760,      0.011213,      0.612075,      48.583308 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_240M_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,      0.073460,      0.187174,     -33.361060,
       0.000000,      0.988072,     -0.108040,      15.355941,
       0.000000,     -0.053593,      0.987556,       8.452730 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_240M_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  0.999684,      0.098988,      0.153462,     -32.308555,
       0.000710,      0.994679,     -0.085937,      11.669638,
      -0.002116,     -0.072847,      0.867128,      26.365961 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT601_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  0.999998,      0.001265,      0.000246,      -0.193427,
       0.055988,      1.122994,     -0.005255,     -15.966381,
       0.033724,     -0.013227,      1.510887,     -64.240172 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT601_to_240M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000025,      0.002164,     -0.000547,      -0.207400,
       0.056520,      1.133284,     -0.004444,     -17.395950,
       0.036150,     -0.020680,      1.633847,     -79.063760 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT601_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.010943,      0.081544,      0.304939,     -49.645005,
       0.051940,      1.122001,     -0.180911,       6.709545,
       0.032671,     -0.081158,      1.613753,     -68.694842 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT601_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.010851,      0.111172,      0.249746,     -46.371144,
       0.053823,      1.129033,     -0.144828,       1.160605,
       0.025113,     -0.100493,      1.417079,     -40.924724 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_170M_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,     -0.085904,     -0.198441,      36.396093,
       0.000000,      1.008830,      0.109840,     -15.189752,
       0.000000,      0.056392,      0.942579,       0.131765 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_170M_to_240M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,     -0.085132,     -0.198846,      36.349129,
       0.000000,      1.018114,      0.111383,     -16.575585,
       0.000000,      0.055251,      1.018646,      -9.458767 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_170M_to_BT601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000071,     -0.087058,     -0.198736,      36.580506,
      -0.049966,      0.902901,      0.110662,      -0.936669,
      -0.022760,      0.047171,      0.629262,      41.780676 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_170M_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  0.999684,      0.024155,     -0.031434,       0.936733,
       0.000710,      1.007888,      0.023110,      -3.979088,
      -0.002116,     -0.026077,      0.875603,      19.294565 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_472M_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,     -0.114055,     -0.187723,      38.627563,
      -0.000503,      1.003507,      0.098941,     -13.105260,
       0.002215,      0.083694,      1.074362,     -20.266631 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_472M_to_240M = BVDC_P_MAKE_CMP_CSC_TEST
    (  0.999998,     -0.113301,     -0.188205,      38.592904,
      -0.000507,      1.012757,      0.100459,     -14.483580,
       0.002398,      0.084805,      1.161213,     -31.528667 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_472M_to_BT601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000071,     -0.115210,     -0.188027,      38.813181,
      -0.050407,      0.899652,      0.100830,       0.744871,
      -0.021298,      0.065841,      0.716160,      28.244624 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_472M_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000410,     -0.023031,      0.036523,      -1.733447,
      -0.000760,      0.991514,     -0.026196,       4.451483,
       0.002395,      0.029473,      1.141379,     -21.907371 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT2020_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,      0.000015,     -0.000130,       0.014753,
       0.000000,      1.143195,      0.016617,     -20.455953,
       0.000000,     -0.021793,      1.502906,     -61.582478 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT709_to_BT2020 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,     -0.000011,      0.000087,      -0.009648,
       0.000000,      0.874557,     -0.009669,      17.294425,
       0.000000,      0.012682,      0.665237,      41.226373 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT2020_to_240M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,      0.000936,     -0.001040,       0.013293,
       0.000000,      1.153664,      0.017775,     -21.944211,
       0.000000,     -0.029085,      1.625107,     -76.290793 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_240M_to_BT2020 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,     -0.000795,      0.000649,       0.018744,
       0.000000,      0.866564,     -0.009478,      18.292975,
       0.000000,      0.015509,      0.615175,      47.272494 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT2020_to_BT601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000071,     -0.001272,     -0.000444,       0.218397,
      -0.049966,      1.018015,      0.020296,      -4.104323,
      -0.019346,     -0.006825,      0.994877,       1.838855 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT601_to_BT2020 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,      0.001252,      0.000420,      -0.214032,
       0.048687,      0.982230,     -0.020016,       4.057565,
       0.019779,      0.006763,      1.005020,      -1.824650 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT2020_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,      0.079279,      0.358121,     -55.987253,
       0.000000,      1.143600,     -0.188998,       5.810930,
       0.000000,     -0.081277,      1.604074,     -66.918015 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_170M_to_BT2020 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,     -0.085911,     -0.233380,      40.869177,
       0.000000,      0.881816,      0.103899,       1.828532,
       0.000000,      0.044681,      0.628677,      41.810198 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_BT2020_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  0.999684,      0.109884,      0.294122,     -51.707697,
       0.000710,      1.150467,     -0.146623,      -0.503457,
      -0.001799,     -0.096657,      1.408076,     -39.832873 );

static const BVDC_P_TestCscCoeffs  s_YCbCr_472M_to_BT2020 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,     -0.114060,     -0.220759,      42.856807,
      -0.000458,      0.876936,      0.091411,       4.058961,
       0.001246,      0.060051,      0.716182,      28.622240 );

#else /* #if USE_DAVID_MATRIX */

/* Group 0: xyz -> BT.709 (i.e. HD) */
/* SMPTE 170M (i.e. modern SD NTSC) -> BT.709 (i.e. HD) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_170M_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000, -0.086918, -0.198602,  36.546531,
       0.000000,  1.008912,  0.109928, -15.211527,
      -0.000000,  0.057004,  0.942791,   0.026243 );

/* BT.470-2 System B, G (i.e. SD Pal) -> BT.709 (i.e. HD) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_472M_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000, -0.115168, -0.189312,  38.973396,
       0.000000,  1.004499,  0.099809, -13.351437,
      -0.000000,  0.084468,  1.072557, -20.099241 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> BT.709 (i.e. HD) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_240M_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  0.000000, -0.000000,  0.000000,
      -0.000000,  0.990940, -0.000253,  1.192039,
      -0.000000,  0.004734,  0.924649,  9.039049 );

/* FCC (i.e. 1953 NTSC) -> BT.709 (i.e. HD) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_BT601_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000, -0.000000,  0.000000,   0.0000000,
       0.054988,  1.121448, -0.005106, -15.771562,
       0.033634, -0.011863,  1.511262, -64.461172 );

/* Group 1: xyz -> SMPTE 170M (i.e. modern SD NTSC) */
/* BT.709 (i.e. HD) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_BT709_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  0.074740,  0.201939, -35.414914,
      -0.000000,  0.997740, -0.116335,  15.180202,
       0.000000, -0.060327,  1.067715,  -0.945678 );

/* BT.470-2 System B, G (i.e. SD Pal) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_472M_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000, -0.023033,  0.034738,  -1.498223,
      -0.000000,  0.992402, -0.025193,   4.197192,
      -0.000000,  0.029590,  1.139164, -21.600485 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_240M_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  0.075019,  0.186703, -33.500488,
      -0.000000,  0.988150, -0.107822,  15.317986,
      -0.000000, -0.054726,  0.987276,   8.633535 );

/* FCC (i.e. 1953 NTSC) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_BT601_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.010902,  0.081422,  0.304801, -49.610885,
       0.050951,  1.120293, -0.180907,   6.943399,
       0.032594, -0.080320,  1.613905, -68.820370 );

/* Group 2: xyz -> BT.470-2 System B, G (i.e. SD Pal) */
/* SMPTE 170M (i.e. modern SD NTSC) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_170M_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  0.024103, -0.029962,   0.749873,
       0.000000,  1.006992,  0.022270,  -3.745506,
       0.000000, -0.026157,  0.877258,  19.058994 );

/* BT.709 (i.e. HD) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_BT709_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  0.100597,  0.167144, -34.270816,
      -0.000000,  1.003373, -0.093371,  11.519780,
       0.000000, -0.079020,  0.939705,  17.832324 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_240M_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  0.100476,  0.154524, -32.640077,
      -0.000000,  0.993840, -0.086589,  11.871852,
       0.000000, -0.073855,  0.868917,  26.232166 );

/* FCC (i.e. 1953 NTSC) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_BT601_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.011153,  0.110831,  0.252085, -46.631688,
       0.052033,  1.126338, -0.146231,   1.713843,
       0.027261, -0.099765,  1.420544, -41.495877 );

/* BT.709 (i.e. HD) -> to xvYCC BT.601 (i.e. SD) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_BT709_to_XvYcc601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  0.100178,  0.191707, -37.361314,
       0.000000,  0.989849, -0.110711,  15.470362,
      -0.000000, -0.073079,  0.983251,  11.497916 );

/* FCC (i.e. 1953 NTSC) -> to xvYCC BT.601 (i.e. SD) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_BT601_to_XvYcc601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.011956,  0.110070,  0.289208, -51.298935,
       0.050706,  1.111377, -0.172368,   6.995471,
       0.029052, -0.093619,  1.486324, -50.731057 );

/* BT.470-2 System B, G (i.e. SD Pal) -> xvYCC BT.601 (i.e. SD) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_472M_to_XvYcc601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  0.001655,  0.026303, -3.578607,
       0.000000,  0.984950, -0.019948,  4.479666,
      -0.000000,  0.009646,  1.047299, -7.288983 );

/* SMPTE 170M (i.e. modern SD NTSC)) -> xvYCC BT.601 (i.e. SD) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_170M_to_XvYcc601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  0.025081, -0.006850,  -2.333618,
       0.000000,  0.992359,  0.004435,   0.410342,
      -0.000000, -0.017681,  0.918967,  12.635360 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> xvYCC BT.601 (i.e. SD) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_240M_to_XvYcc601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  0.100178,  0.177236, -35.509050,
      -0.000000,  0.980357, -0.102619,  15.649576,
      -0.000000, -0.067762,  0.909181,  20.298460 );

/* xvYCC BT.601 (i.e. SD) -> to BT.709 (i.e. HD) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_XvYcc601_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000, -0.116569, -0.208098,  41.557373,
       0.000000,  1.018724,  0.114705, -17.078894,
      -0.000000,  0.075715,  1.025559, -12.963136 );

/* xvYCC BT.601 (i.e. SD) -> SMPTE 170M (i.e. modern SD NTSC)) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_XvYcc601_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000, -0.025140,  0.007575,   2.248217,
      -0.000000,  1.007613, -0.004863,  -0.352023,
       0.000000,  0.019386,  1.088085, -13.756298 );

/* xvYCC BT.601 (i.e. SD) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_TestCscCoeffs  s_YCbCr_XvYcc601_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000, -0.001434, -0.025143,  3.401765,
       0.000000,  1.015090,  0.019334, -4.406337,
       0.000000, -0.009349,  0.954659,  7.000373 );

/* 472M (i.e. SD Pal) -> BT2020 (i.e. UHD) */
static const BVDC_P_TestCscCoeffs s_YCbCr_472M_to_BT2020 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000, -0.114060, -0.220759,  42.856807,
      -0.000458,  0.876936,  0.091411,   4.058961,
       0.001246,  0.060051,  0.716182,  28.622240 );

/* 240M (i.e. 1987 ATSC HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_TestCscCoeffs s_YCbCr_240M_to_BT2020 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  -0.000795,   0.000649,   0.018744,
       0.000000,   0.866564,  -0.009478,  18.292975,
       0.000000,   0.015509,   0.615175,  47.272494 );

/* BT601 (i.e. xvYCC BT.601) -> BT2020 (i.e. UHD) */
static const BVDC_P_TestCscCoeffs s_YCbCr_XvYcc601_to_BT2020 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,   0.001252,   0.000420,  -0.214032,
       0.048687,   0.982230,  -0.020016,   4.057565,
       0.019779,   0.006763,   1.005020,  -1.824650 );

/* 170M (i.e. modern SD NTSC) -> BT2020 (i.e. UHD) */
static const BVDC_P_TestCscCoeffs s_YCbCr_170M_to_BT2020 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  -0.085911,  -0.233380,  40.869177,
       0.000000,   0.881816,   0.103899,   1.828532,
       0.000000,   0.044681,   0.628677,  41.810198 );

/* BT2020 (i.e. UHD) -> BT601 (i.e. xvYCC BT.601)*/
static const BVDC_P_TestCscCoeffs s_YCbCr_BT2020_to_XvYcc601 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000071,  -0.001272,  -0.000444,   0.218397,
      -0.049966,   1.018015,   0.020296,  -4.104323,
      -0.019346,  -0.006825,   0.994877,   1.838855 );

/* BT2020 (i.e. UHD) -> 170M (i.e. modern SD NTSC) */
static const BVDC_P_TestCscCoeffs s_YCbCr_BT2020_to_170M = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,   0.079279,   0.358121,  -55.987253,
       0.000000,   1.143600,  -0.188998,    5.810930,
       0.000000,  -0.081277,   1.604074,  -66.918015 );

/* BT2020 (i.e. UHD) -> 472M (i.e. SD Pal) */
static const BVDC_P_TestCscCoeffs s_YCbCr_BT2020_to_472M = BVDC_P_MAKE_CMP_CSC_TEST
    (  0.999684,   0.109884,   0.294122,  -51.707697,
       0.000710,   1.150467,  -0.146623,   -0.503457,
      -0.001799,  -0.096657,   1.408076,  -39.832873 );

/* SDR BT2020 (i.e. UHD) -> BT709 (i.e. HD) */
static const BVDC_P_TestCscCoeffs s_YCbCr_BT2020_to_BT709 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,   0.000015, -0.000130,    0.014753,
       0.000000,   1.143195,  0.016617,  -20.455953,
       0.000000,  -0.021793,  1.502906,  -61.582478 );

/* SDR BT709 (i.e. HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_TestCscCoeffs s_YCbCr_BT709_to_BT2020 = BVDC_P_MAKE_CMP_CSC_TEST
    (  1.000000,  -0.000011,   0.000087,  -0.009648,
       0.000000,   0.874557,  -0.009669,  17.294425,
       0.000000,   0.012682,   0.665237,  41.226373 );

#define s_YCbCr_BT601_to_BT2020   s_YCbCr_BT601_to_BT709
#endif /* #if USE_DAVID_MATRIX */

#define s_YCbCr_BT709_to_BT709    s_YCbCr_Identity
#define s_YCbCr_170M_to_170M      s_YCbCr_Identity
#define s_YCbCr_472M_to_472M      s_YCbCr_Identity
#define s_YCbCr_BT2020_to_BT2020  s_YCbCr_Identity

static const BVDC_P_TestCscCoeffs *const s_aaCMP_YCbCr_MatrixTbl[][8] =
{
    /* BCFC_Colorimetry_eBt709 */
    {
        &s_YCbCr_BT709_to_BT709,       /* BT709 -> BT709 */
        &s_YCbCr_170M_to_BT709,        /* Smpte170M -> BT709 */
        &s_YCbCr_472M_to_BT709,        /* BT470_BG -> BT709 */
        &s_YCbCr_BT2020_to_BT709,      /* BT2020 -> BT709 */
        &s_YCbCr_170M_to_BT709,        /* XvYcc601 -> BT709 */
        &s_YCbCr_BT709_to_BT709,       /* XvYcc709 -> BT709 */
        &s_YCbCr_BT601_to_BT709,       /* Fcc -> BT709 */
        &s_YCbCr_240M_to_BT709         /* Smpte240M -> BT709 */
    },

    /* BCFC_Colorimetry_eSmpte170M */
    {
        &s_YCbCr_BT709_to_170M,        /* BT709 -> 170M */
        &s_YCbCr_170M_to_170M,         /* Smpte170M -> 170M */
        &s_YCbCr_472M_to_170M,         /* BT470_BG -> 170M */
        &s_YCbCr_BT2020_to_170M,       /* BT2020 -> 170M */
        &s_YCbCr_170M_to_170M,         /* XvYcc601 -> 170M */
        &s_YCbCr_BT709_to_170M,        /* XvYcc709 -> 170M */
        &s_YCbCr_BT601_to_170M,        /* Fcc -> 170M */
        &s_YCbCr_240M_to_170M          /* Smpte240M -> 170M */
    },

    /* BCFC_Colorimetry_eBt470_BG */
    {
        &s_YCbCr_BT709_to_472M,        /* BT709 -> 472M */
        &s_YCbCr_170M_to_472M,         /* Smpte170M -> 472M */
        &s_YCbCr_472M_to_472M,         /* BT470_BG -> 472M */
        &s_YCbCr_BT2020_to_472M,       /* BT2020 -> 472M */
        &s_YCbCr_170M_to_472M,         /* XvYcc601 -> 472M */
        &s_YCbCr_BT709_to_472M,        /* XvYcc709 -> 472M */
        &s_YCbCr_BT601_to_472M,        /* Fcc -> 472M */
        &s_YCbCr_240M_to_472M          /* Smpte240M -> 472M */
    },

    /* BCFC_Colorimetry_eBt2020 */
    {
        &s_YCbCr_BT709_to_BT2020,      /* BT709 -> BT2020-NCL*/
        &s_YCbCr_170M_to_BT2020,       /* Smpte170M -> BT2020-NCL*/
        &s_YCbCr_472M_to_BT2020,       /* BT470_BG -> BT2020-NCL*/
        &s_YCbCr_BT2020_to_BT2020,     /* BT2020 -> BT2020-NCL*/
        &s_YCbCr_170M_to_BT2020,       /* XvYcc601 -> BT2020-NCL*/
        &s_YCbCr_BT709_to_BT2020,      /* XvYcc709 -> BT2020-NCL*/
        &s_YCbCr_BT601_to_BT2020,      /* Fcc -> BT2020-NCL*/
        &s_YCbCr_240M_to_BT2020        /* Smpte240M -> BT2020-NCL*/
    },

    /* BCFC_Colorimetry_eXvYcc601 */
    {
        &s_YCbCr_BT709_to_170M,        /* BT709 -> 170M */
        &s_YCbCr_170M_to_170M,         /* Smpte170M -> 170M */
        &s_YCbCr_472M_to_170M,         /* BT470_BG -> 170M */
        &s_YCbCr_BT2020_to_170M,       /* BT2020 -> 170M */
        &s_YCbCr_170M_to_170M,         /* XvYcc601 -> 170M */
        &s_YCbCr_BT709_to_170M,        /* XvYcc709 -> 170M */
        &s_YCbCr_BT601_to_170M,        /* Fcc -> 170M */
        &s_YCbCr_240M_to_170M          /* Smpte240M -> 170M */
    },

    /* BCFC_Colorimetry_eXvYcc709 */
    {
        &s_YCbCr_BT709_to_BT709,       /* BT709 -> BT709 */
        &s_YCbCr_170M_to_BT709,        /* Smpte170M -> BT709 */
        &s_YCbCr_472M_to_BT709,        /* BT470_BG -> BT709 */
        &s_YCbCr_BT2020_to_BT709,      /* BT2020 -> BT709 */
        &s_YCbCr_170M_to_BT709,        /* XvYcc601 -> BT709 */
        &s_YCbCr_BT709_to_BT709,       /* XvYcc709 -> BT709 */
        &s_YCbCr_BT601_to_BT709,       /* Fcc -> BT709 */
        &s_YCbCr_240M_to_BT709         /* Smpte240M -> BT709 */
    }
};

#if !(USE_24BITS)
/* 0.0286865234375 = 235.0 / (1 << 13), because max input Y/Cb/Cr = 235, matrix coeff has 13 bits fraction
 * 0.001953125 = 16.0 / (1<<13), because min input Y/Cb/Cr = 16, matrix coeff has 13 bits fraction
 * 0.015625 = 1.0 / (1 << 6), because matrix offset has 6 bits fraction
 */
#define ADD_COEF(m)  \
    if (m < 0) {fNegErr -= (m * 0.0286865234375); fPosErr += (m * 0.001953125);} \
    else       {fPosErr += (m * 0.0286865234375); fNegErr -= (m * 0.001953125);}
static float BVDC_P_Cfc_RowMaxError(int32_t m0, int32_t m1, int32_t m2, int32_t m3)
{
    float fPosErr = 0, fNegErr = 0;

    ADD_COEF(m0);
    ADD_COEF(m1);
    ADD_COEF(m2);

    if (m3 < 0) {
        fNegErr -= m3 * 0.015625;  fPosErr += m3 * 0.015625;
    } else {
        fPosErr += m3 * 0.015625;  fNegErr -= m3 * 0.015625;
    }

    if (fPosErr >= fNegErr)
        return fPosErr;
    else
        return fNegErr;
}
#endif /* #if !(USE_24BITS) */

#if USE_24BITS
#define HW_CX(a) (((a)>>(BCFC_CSC_SW_CX_F_BITS - BVDC_P_CSC_CMP_AB_CX_F_BITS)) & 0x1FFFFFF)
#define HW_CO(o) (((o)>>(BCFC_CSC_SW_CO_F_BITS - BVDC_P_CSC_CMP_AB_CO_F_BITS)) & 0x1FFFFFF)
#define HW2_CX(a) (((a)>>(BCFC_CSC_SW_CX_F_BITS - BVDC_P_CSC_CMP_AB_CX_F_BITS)))
#define HW2_CO(o) (((o)>>(BCFC_CSC_SW_CO_F_BITS - BVDC_P_CSC_CMP_AB_CO_F_BITS)))
#define OLD_CX(a) (((a) & 0x1000000) ? ((a) | 0xFF000000) : (a))
#define OLD_CO(a) (((a) & 0x1000000) ? ((a) | 0xFF000000) : (a))
#else
#define HW_CX(a) (((a)>>(BCFC_CSC_SW_CX_F_BITS - BVDC_P_CSC_CMP_CX_F_BITS)) & 0xFFFF)
#define HW_CO(o) (((o)>>(BCFC_CSC_SW_CO_F_BITS - BVDC_P_CSC_CMP_CO_F_BITS)) & 0xFFFF)
#define HW2_CX(a) (((a)>>(BCFC_CSC_SW_CX_F_BITS - BVDC_P_CSC_CMP_CX_F_BITS)))
#define HW2_CO(o) (((o)>>(BCFC_CSC_SW_CO_F_BITS - BVDC_P_CSC_CMP_CO_F_BITS)))
#define OLD_CX(a) (((a) & 0x8000) ? ((a) | 0xFFFF0000) : (a))
#define OLD_CO(a) (((a) & 0x8000) ? ((a) | 0xFFFF0000) : (a))
#endif

void BVDC_Cfc_CheckMatrixAccuracy(void)
{
    BCFC_ColorSpaceExt  stOutColorSpaceExt;
    BCFC_Context  stCfc;
    BCFC_ColorSpace *pColorSpaceIn = &(stCfc.stColorSpaceExtIn.stColorSpace);
    BCFC_ColorSpace *pColorSpaceOut = &(stOutColorSpaceExt.stColorSpace);
    const BVDC_P_TestCscCoeffs *pCsc;
    BCFC_Colorimetry eOutColorimetry, eInColorimetry;
    float fGlobalMaxError = 0;
    BCFC_Colorimetry eBadOutColorimetry = BCFC_Colorimetry_eBt709, eBadInColorimetry = BCFC_Colorimetry_eBt709;

    BKNI_Memset((void*)&stOutColorSpaceExt, 0x0, sizeof(BCFC_ColorSpaceExt));
    BKNI_Memset((void*)&stCfc, 0x0, sizeof(BCFC_Context));

    stCfc.stCapability.stBits.bMc = 1;
    stCfc.pColorSpaceOut = &stOutColorSpaceExt;
    BVDC_P_Cfc_Init(&stCfc);

    pColorSpaceIn->eColorFmt = BCFC_ColorFormat_eYCbCr;
    pColorSpaceIn->eColorimetry = BCFC_Colorimetry_eBt709;
    pColorSpaceIn->eColorTF = BCFC_ColorTF_eBt1886;
    pColorSpaceIn->eColorRange = BCFC_ColorRange_eLimited;
    pColorSpaceIn->eColorDepth = BCFC_ColorDepth_e8Bit;

    pColorSpaceOut->eColorFmt = BCFC_ColorFormat_eYCbCr;
    pColorSpaceOut->eColorimetry = BCFC_Colorimetry_eBt709;
    pColorSpaceOut->eColorTF = BCFC_ColorTF_eBt1886;
    pColorSpaceOut->eColorRange = BCFC_ColorRange_eLimited;
    pColorSpaceOut->eColorDepth = BCFC_ColorDepth_e8Bit;

    for (eOutColorimetry=BCFC_Colorimetry_eBt709; eOutColorimetry<=BCFC_Colorimetry_eXvYcc709; eOutColorimetry++)
    {
        pColorSpaceOut->eColorimetry = eOutColorimetry;
        stOutColorSpaceExt.stCfg.stBits.bDirty = 1;
        for (eInColorimetry=BCFC_Colorimetry_eBt709; eInColorimetry<BCFC_Colorimetry_eMax; eInColorimetry++)
        {
            uint32_t a0, a1, a2, a3;
            int32_t  b0, b1, b2, b3;
            float fMatrixMaxError, fRowMaxError;

            if ((eOutColorimetry == eInColorimetry) ||
                ((eOutColorimetry == BCFC_Colorimetry_eBt2020) && (eInColorimetry == BCFC_Colorimetry_eFcc)))
                continue;

            pColorSpaceIn->eColorimetry = eInColorimetry;
            stCfc.stColorSpaceExtIn.stCfg.stBits.bDirty = 1;
            BVDC_P_Cfc_UpdateCfg_isr(&stCfc, true);

            pCsc = s_aaCMP_YCbCr_MatrixTbl[eOutColorimetry][eInColorimetry];

            printf("\nmatrix YCbCr_%s_to_%s\n", BCFC_GetColorimetryName_isrsafe(eInColorimetry), BCFC_GetColorimetryName_isrsafe(eOutColorimetry));
#if USE_24BITS
            printf("old:\n");
            printf("  [[0x%08x 0x%08x 0x%08x 0x%08x]\n",
                   pCsc->ulY0, pCsc->ulY1, pCsc->ulY2, pCsc->ulYOffset);
            printf("   [0x%08x 0x%08x 0x%08x 0x%08x]\n",
                   pCsc->ulCb0, pCsc->ulCb1, pCsc->ulCb2, pCsc->ulCbOffset);
            printf("   [0x%08x 0x%08x 0x%08x 0x%08x]]\n",
                   pCsc->ulCr0, pCsc->ulCr1, pCsc->ulCr2, pCsc->ulCrOffset);
            printf("new:\n");
            printf("  [[0x%08x 0x%08x 0x%08x 0x%08x]\n",
                   HW_CX(stCfc.stMc.m[0][0]), HW_CX(stCfc.stMc.m[0][1]), HW_CX(stCfc.stMc.m[0][2]), HW_CO(stCfc.stMc.m[0][3]));
            printf("   [0x%08x 0x%08x 0x%08x 0x%08x]\n",
                   HW_CX(stCfc.stMc.m[1][0]), HW_CX(stCfc.stMc.m[1][1]), HW_CX(stCfc.stMc.m[1][2]), HW_CO(stCfc.stMc.m[1][3]));
            printf("   [0x%08x 0x%08x 0x%08x 0x%08x]]\n",
                   HW_CX(stCfc.stMc.m[2][0]), HW_CX(stCfc.stMc.m[2][1]), HW_CX(stCfc.stMc.m[2][2]), HW_CO(stCfc.stMc.m[2][3]));
            printf("new - old:\n");
            a0 = OLD_CX(pCsc->ulY0); a1 = OLD_CX(pCsc->ulY1); a2 = OLD_CX(pCsc->ulY2); a3 = OLD_CO(pCsc->ulYOffset);
            b0 = *(int32_t*)&a0; b1 = *(int32_t*)&a1; b2 = *(int32_t*)&a2; b3 = *(int32_t*)&a3;
            printf("  [[%10d %10d %10d %10d]\n",
                   HW2_CX(stCfc.stMc.m[0][0]) - b0, HW2_CX(stCfc.stMc.m[0][1]) - b1, HW2_CX(stCfc.stMc.m[0][2]) - b2, HW2_CX(stCfc.stMc.m[0][3]) - b3);
            a0 = OLD_CX(pCsc->ulCb0); a1 = OLD_CX(pCsc->ulCb1); a2 = OLD_CX(pCsc->ulCb2); a3 = OLD_CO(pCsc->ulCbOffset);
            b0 = *(int32_t*)&a0; b1 = *(int32_t*)&a1; b2 = *(int32_t*)&a2; b3 = *(int32_t*)&a3;
            printf("   [%10d %10d %10d %10d]\n",
                   HW2_CX(stCfc.stMc.m[1][0]) - b0, HW2_CX(stCfc.stMc.m[1][1]) - b1, HW2_CX(stCfc.stMc.m[1][2]) - b2, HW2_CX(stCfc.stMc.m[1][3]) - b3);

            a0 = OLD_CX(pCsc->ulCr0); a1 = OLD_CX(pCsc->ulCr1); a2 = OLD_CX(pCsc->ulCr2); a3 = OLD_CO(pCsc->ulCrOffset);
            b0 = *(int32_t*)&a0; b1 = *(int32_t*)&a1; b2 = *(int32_t*)&a2; b3 = *(int32_t*)&a3;
            printf("   [%10d %10d %10d %10d]]\n",
                   HW2_CX(stCfc.stMc.m[2][0]) - b0, HW2_CX(stCfc.stMc.m[2][1]) - b1, HW2_CX(stCfc.stMc.m[2][2]) - b2, HW2_CX(stCfc.stMc.m[2][3]) - b3);
            BSTD_UNUSED(fRowMaxError);
            BSTD_UNUSED(fMatrixMaxError);
#else /* #if USE_24BITS    */
            printf("old:\n");
            printf("  [[0x%04x 0x%04x 0x%04x 0x%04x]\n",
                   pCsc->usY0, pCsc->usY1, pCsc->usY2, pCsc->usYOffset);
            printf("   [0x%04x 0x%04x 0x%04x 0x%04x]\n",
                   pCsc->usCb0, pCsc->usCb1, pCsc->usCb2, pCsc->usCbOffset);
            printf("   [0x%04x 0x%04x 0x%04x 0x%04x]]\n",
                   pCsc->usCr0, pCsc->usCr1, pCsc->usCr2, pCsc->usCrOffset);
            printf("new:\n");
            printf("  [[0x%04x 0x%04x 0x%04x 0x%04x]\n",
                   HW_CX(stCfc.stMc.m[0][0]), HW_CX(stCfc.stMc.m[0][1]), HW_CX(stCfc.stMc.m[0][2]), HW_CO(stCfc.stMc.m[0][3]));
            printf("   [0x%04x 0x%04x 0x%04x 0x%04x]\n",
                   HW_CX(stCfc.stMc.m[1][0]), HW_CX(stCfc.stMc.m[1][1]), HW_CX(stCfc.stMc.m[1][2]), HW_CO(stCfc.stMc.m[1][3]));
            printf("   [0x%04x 0x%04x 0x%04x 0x%04x]]\n",
                   HW_CX(stCfc.stMc.m[2][0]), HW_CX(stCfc.stMc.m[2][1]), HW_CX(stCfc.stMc.m[2][2]), HW_CO(stCfc.stMc.m[2][3]));
            printf("new - old:\n");
            a0 = OLD_CX(pCsc->usY0); a1 = OLD_CX(pCsc->usY1); a2 = OLD_CX(pCsc->usY2); a3 = OLD_CO(pCsc->usYOffset);
            b0 = *(int32_t*)&a0; b1 = *(int32_t*)&a1; b2 = *(int32_t*)&a2; b3 = *(int32_t*)&a3;
            fMatrixMaxError = fRowMaxError = BVDC_P_Cfc_RowMaxError(HW2_CX(stCfc.stMc.m[0][0])-b0, HW2_CX(stCfc.stMc.m[0][1])-b1, HW2_CX(stCfc.stMc.m[0][2])-b2, HW2_CX(stCfc.stMc.m[0][3])-b3);
            printf("  [[%6d %6d %6d %6d]\n",
                   HW2_CX(stCfc.stMc.m[0][0]) - b0, HW2_CX(stCfc.stMc.m[0][1]) - b1, HW2_CX(stCfc.stMc.m[0][2]) - b2, HW2_CX(stCfc.stMc.m[0][3]) - b3);
            a0 = OLD_CX(pCsc->usCb0); a1 = OLD_CX(pCsc->usCb1); a2 = OLD_CX(pCsc->usCb2); a3 = OLD_CO(pCsc->usCbOffset);
            b0 = *(int32_t*)&a0; b1 = *(int32_t*)&a1; b2 = *(int32_t*)&a2; b3 = *(int32_t*)&a3;
            fRowMaxError = BVDC_P_Cfc_RowMaxError(HW2_CX(stCfc.stMc.m[1][0])-b0, HW2_CX(stCfc.stMc.m[1][1])-b1, HW2_CX(stCfc.stMc.m[1][2])-b2, HW2_CX(stCfc.stMc.m[1][3])-b3);
            fMatrixMaxError = BVDC_P_MAX(fMatrixMaxError, fRowMaxError);
            printf("   [%6d %6d %6d %6d]\n",
                   HW2_CX(stCfc.stMc.m[1][0]) - b0, HW2_CX(stCfc.stMc.m[1][1]) - b1, HW2_CX(stCfc.stMc.m[1][2]) - b2, HW2_CX(stCfc.stMc.m[1][3]) - b3);

            a0 = OLD_CX(pCsc->usCr0); a1 = OLD_CX(pCsc->usCr1); a2 = OLD_CX(pCsc->usCr2); a3 = OLD_CO(pCsc->usCrOffset);
            b0 = *(int32_t*)&a0; b1 = *(int32_t*)&a1; b2 = *(int32_t*)&a2; b3 = *(int32_t*)&a3;
            fRowMaxError = BVDC_P_Cfc_RowMaxError(HW2_CX(stCfc.stMc.m[2][0])-b0, HW2_CX(stCfc.stMc.m[2][1])-b1, HW2_CX(stCfc.stMc.m[2][2])-b2, HW2_CX(stCfc.stMc.m[2][3])-b3);
            fMatrixMaxError = BVDC_P_MAX(fMatrixMaxError, fRowMaxError);
            printf("   [%6d %6d %6d %6d]]\n",
                   HW2_CX(stCfc.stMc.m[2][0]) - b0, HW2_CX(stCfc.stMc.m[2][1]) - b1, HW2_CX(stCfc.stMc.m[2][2]) - b2, HW2_CX(stCfc.stMc.m[2][3]) - b3);

            printf("   max matrix resulted error %f\n", fMatrixMaxError);
            if (fMatrixMaxError > fGlobalMaxError)
            {
                fGlobalMaxError = fMatrixMaxError;
                eBadOutColorimetry = eOutColorimetry; eBadInColorimetry = eInColorimetry;
            }

#endif /* #if USE_24BITS */
        }
    }
#if USE_24BITS
    BSTD_UNUSED(eBadInColorimetry);
    BSTD_UNUSED(eBadOutColorimetry);
    BSTD_UNUSED(fGlobalMaxError);
#else
    printf("\nThe worst matrix resulted error %f\n", fGlobalMaxError);
    printf("It is achieved by inColorimetry %s, outColorimetry %s\n\n\n", BCFC_GetColorimetryName_isrsafe(eBadInColorimetry), BCFC_GetColorimetryName_isrsafe(eBadOutColorimetry));
#endif
}

#endif /* #if BVDC_P_CFC_CHECK_MATRIX_ACCURACY */

/* End of file */
