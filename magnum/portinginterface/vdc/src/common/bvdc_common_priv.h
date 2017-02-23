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
#ifndef BVDC_COMMON_PRIV_H__
#define BVDC_COMMON_PRIV_H__

#include "bkni.h"
#include "bavc_hdmi.h"
#include "bchp_fmisc.h"
#include "bchp_vfd_0.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************
 * VDC Internal defines
 ***************************************************************************/

/***************************************************************************
 * Chip Features macro to aid conditional compilations!  This is to avoid
 * the many #ifdef (BCHP_CHIP==xxx) through out the vdc.
 ***************************************************************************/
#if (BCHP_CHIP==7358) || (BCHP_CHIP==7552)

#define BVDC_P_SUPPORT_656_MASTER_MODE        (0)
#define BVDC_P_SUPPORT_TDAC_VER               (9) /* TODO */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (1)
#define BVDC_P_SUPPORT_LOOP_BACK              (2)
#define BVDC_P_SUPPORT_DNR                    (1)
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0)
#define BVDC_P_SUPPORT_VEC_GRPD               (0)
#define BVDC_P_SUPPORT_CAP_VER                (4)

/* source */
#define BVDC_P_SUPPORT_GFD                    (2)
#define BVDC_P_SUPPORT_GFD_VER                (4)
#define BVDC_P_SUPPORT_GFD1_VER               (4)
#define BVDC_P_SUPPORT_MFD                    (1)
#if (BCHP_VER >= BCHP_VER_B0)
#define BVDC_P_SUPPORT_MFD_VER                (12)/* MFD HW version */
#else
#define BVDC_P_SUPPORT_MFD_VER                (11)
#endif
#define BVDC_P_SUPPORT_MTG                    (0) /* MFD Trigger Generator */
#if (BCHP_CHIP==7358)
#if (BCHP_VER >= BCHP_VER_A1)
#define BVDC_P_SUPPORT_MADR_VER               (4) /* MAD-R HW version */
#define BVDC_P_SUPPORT_CLOCK_GATING           (1) /* Clock gating for power saving */
#else
#define BVDC_P_SUPPORT_MADR_VER               (2) /* MAD-R HW version */
#define BVDC_P_SUPPORT_CLOCK_GATING           (0) /* Clock gating for power saving */
#endif
#define BVDC_P_SUPPORT_IT_VER                 (2)
#else /* (BCHP_CHIP==7358) */
#if (BCHP_VER >= BCHP_VER_B0)
#define BVDC_P_SUPPORT_MADR_VER               (5) /* MAD-R HW version */
#define BVDC_P_SUPPORT_CLOCK_GATING           (1) /* Clock gating for power saving */
#define BVDC_P_SUPPORT_IT_VER                 (3)
#else
#define BVDC_P_SUPPORT_MADR_VER               (2) /* MAD-R HW version */
#define BVDC_P_SUPPORT_CLOCK_GATING           (0) /* Clock gating for power saving */
#define BVDC_P_SUPPORT_IT_VER                 (2)
#endif
#endif
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (0)  /*Dynamic Memory Power Gating*/
#define BVDC_P_SUPPORT_HDDVI                  (0)
#define BVDC_P_SUPPORT_HDDVI_VER              (0)

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1)
#define BVDC_P_SUPPORT_PEP_VER                (1)
#define BVDC_P_SUPPORT_HIST                   (1)
#define BVDC_P_SUPPORT_TNT                    (1)
#define BVDC_P_SUPPORT_TNT_VER                (6)
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (1)
#define BVDC_P_SUPPORT_BOX_DETECT             (1)
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (1)
#define BVDC_P_SUPPORT_CAP                    (2)
#define BVDC_P_SUPPORT_VFD                    (2)
#define BVDC_P_SUPPORT_SCL                    (2)
#define BVDC_P_SUPPORT_SCL_VER                (7)
#define BVDC_P_SUPPORT_XSRC_VER               (0) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (0) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_HSCL_VER               (5)
#define BVDC_P_SUPPORT_DMISC                  (0)
#define BVDC_P_SUPPORT_FREE_CHANNEL           (3)
#define BVDC_P_SUPPORT_DRAIN_F                (1)
#define BVDC_P_SUPPORT_DRAIN_B                (1)
#define BVDC_P_SUPPORT_DRAIN_VER              (3) /* DRAIN HW version */

#define BVDC_P_SUPPORT_MCVP                   (1)
#define BVDC_P_SUPPORT_MCVP_VER               (3) /* TODO */
#define BVDC_P_SUPPORT_MCDI_VER               (0) /* TODO: MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (1) /* TODO: Number of MAD-R HW */
#define BVDC_P_SUPPORT_MANR                   (0) /* TODO: Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (0) /* TODO: ANR in MCVP version */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (4) /* Testfeature HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (0)
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (0)
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (0)
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (0)
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1)
#define BVDC_P_MIN_XCODE_CMP                  (0)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (1)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (0)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1)
#define BVDC_P_SUPPORT_HDMI_RM_VER            (5)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_ITU656_OUT             (0)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)

#define BVDC_P_MAX_DACS                       (4)
#define BVDC_P_ORTHOGONAL_VEC_VER             (0)
#define BVDC_P_SUPPORT_HD_DAC                 (1)
#define BVDC_P_SUPPORT_STG_VER                (0)
#define BVDC_P_SUPPORT_DTG_RMD                (0)
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)

#define BVDC_P_NUM_SHARED_656                 (1)
#define BVDC_P_NUM_SHARED_DVI                 (1)
#define BVDC_P_NUM_SHARED_STG                 (0)
#define BVDC_P_NUM_SHARED_RF                  (1)
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_NUM_SHARED_VF                  (2)
#define BVDC_P_NUM_SHARED_SDSRC               (1)
#define BVDC_P_NUM_SHARED_HDSRC               (1)
#define BVDC_P_NUM_SHARED_SM                  (1)
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1)
#define BVDC_P_NUM_SHARED_SECAM_HD            (2)
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (0)
#define BVDC_P_SUPPORT_STG                    (0)
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_216_SYSCLK)

#elif (BCHP_CHIP==7422) || (BCHP_CHIP==7425)

#define BVDC_P_SUPPORT_656_MASTER_MODE        (0)
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (1)
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0)
#define BVDC_P_SUPPORT_VEC_GRPD               (0)

/* source */
#define BVDC_P_SUPPORT_GFD                    (4)
#define BVDC_P_SUPPORT_MFD                    (3)
#define BVDC_P_SUPPORT_MTG                    (0) /* MFD Trigger Generator */

#if (BCHP_VER >= BCHP_VER_B0)
#define BVDC_P_SUPPORT_GFD_VER                (5)
#define BVDC_P_SUPPORT_GFD1_VER               (5)
#define BVDC_P_SUPPORT_MFD_VER                (13)/* MFD HW version */
#define BVDC_P_SUPPORT_TDAC_VER               (11)/* TDAC/QDAC HW version */
#define BVDC_P_SUPPORT_SCL_VER                (8) /* SCL HW version */
#define BVDC_P_SUPPORT_HSCL_VER               (6) /* HSCL HW version */
#define BVDC_P_SUPPORT_DNR                    (3) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_MCVP                   (3) /* Number of MCVP HW */
#define BVDC_P_SUPPORT_MADR                   (2) /* Number of MAD-R HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (4) /* Number LBOX HW detect */
#define BVDC_P_SUPPORT_LOOP_BACK              (6) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_HDDVI_VER              (8)
#define BVDC_P_SUPPORT_MADR_VER               (5) /* MAD-R HW version */
#define BVDC_P_SUPPORT_CAP_VER                (5)
#define BVDC_P_SUPPORT_CLOCK_GATING           (1)
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_SUPPORT_DSCL                   (1)
#define BVDC_P_SUPPORT_DSCL_VER               (1)
#else
#define BVDC_P_SUPPORT_GFD_VER                (4)
#define BVDC_P_SUPPORT_GFD1_VER               (4)
#define BVDC_P_SUPPORT_MFD_VER                (10)/* MFD HW version */
#define BVDC_P_SUPPORT_TDAC_VER               (9) /* TDAC/QDAC HW version */
#define BVDC_P_SUPPORT_SCL_VER                (7) /* SCL HW version */
#define BVDC_P_SUPPORT_HSCL_VER               (5) /* HSCL HW version */
#define BVDC_P_SUPPORT_DNR                    (2) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_MCVP                   (2) /* Number of MCVP HW */
#define BVDC_P_SUPPORT_MADR                   (1) /* Number of MAD-R HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (3) /* Number LBOX HW detect */
#define BVDC_P_SUPPORT_LOOP_BACK              (4) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_HDDVI_VER              (7) /* HDDVI HW version */
#define BVDC_P_SUPPORT_MADR_VER               (3) /* MAD-R HW version */
#define BVDC_P_SUPPORT_CAP_VER                (4)
#define BVDC_P_SUPPORT_CLOCK_GATING           (0)
#define BVDC_P_SUPPORT_DTG_RMD                (0)
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)
#endif
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (0)  /*Dynamic Memory Power Gating*/
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (1)
#define BVDC_P_SUPPORT_HDDVI                  (1)

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1)
#define BVDC_P_SUPPORT_PEP_VER                (1)
#define BVDC_P_SUPPORT_HIST                   (1)
#define BVDC_P_SUPPORT_TNT                    (1)
#define BVDC_P_SUPPORT_TNT_VER                (6)
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (1)
#define BVDC_P_SUPPORT_CAP                    (5)
#define BVDC_P_SUPPORT_VFD                    (5)
#define BVDC_P_SUPPORT_SCL                    (5)
#define BVDC_P_SUPPORT_XSRC_VER               (0) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (0) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_DMISC                  (0)
#define BVDC_P_SUPPORT_FREE_CHANNEL           (7)
#define BVDC_P_SUPPORT_DRAIN_F                (3) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (2) /* DRAIN HW version */
#if (BCHP_CHIP==7422)
#define BVDC_P_SUPPORT_MCDI_VER               (2) /* MCDI HW version */
#define BVDC_P_SUPPORT_STG                    (0) /* Number of STG HW */
#define BVDC_P_NUM_SHARED_STG                 (0)
#define BVDC_P_ORTHOGONAL_VEC_VER             (0)
#define BVDC_P_SUPPORT_IT_VER                 (3)
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (0)
#else
#if (BCHP_VER >= BCHP_VER_B0)
#define BVDC_P_SUPPORT_STG                    (2) /* Number of STG HW */
#define BVDC_P_SUPPORT_STG_VER                (2) /* STG HW version */
#define BVDC_P_NUM_SHARED_STG                 (2)
#define BVDC_P_ORTHOGONAL_VEC_VER             (1)
#define BVDC_P_SUPPORT_IT_VER                 (3)
#define BVDC_P_SUPPORT_MCDI_VER               (4) /* MCDI HW version */
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (1)
#else
#define BVDC_P_SUPPORT_STG                    (1) /* Number of STG HW */
#define BVDC_P_SUPPORT_STG_VER                (1) /* STG HW version   */
#define BVDC_P_NUM_SHARED_STG                 (1)
#define BVDC_P_ORTHOGONAL_VEC_VER             (0)
#define BVDC_P_SUPPORT_IT_VER                 (2)
#define BVDC_P_SUPPORT_MCDI_VER               (3) /* MCDI HW version */
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (0)
#endif
#endif
#define BVDC_P_SUPPORT_MANR                   (1) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (2) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_MCVP_VER               (2) /* MCVP HW version */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (4) /* Testfeature HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1)
#define BVDC_P_MIN_XCODE_CMP                  (0)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (1)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1)
#define BVDC_P_SUPPORT_HDMI_RM_VER            (5)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)

#define BVDC_P_MAX_DACS                       (4)
#define BVDC_P_SUPPORT_HD_DAC                 (1)
#define BVDC_P_NUM_SHARED_656                 (1)
#define BVDC_P_NUM_SHARED_DVI                 (1)
#define BVDC_P_NUM_SHARED_RF                  (1)
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_NUM_SHARED_VF                  (2)
#define BVDC_P_NUM_SHARED_SDSRC               (1)
#define BVDC_P_NUM_SHARED_HDSRC               (1)
#define BVDC_P_NUM_SHARED_SM                  (1)
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1)
#define BVDC_P_NUM_SHARED_SECAM_HD            (2)
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_216_SYSCLK)

#elif (BCHP_CHIP==7231) || (BCHP_CHIP==7346) || (BCHP_CHIP==7344) || (BCHP_CHIP==73465)
#define BVDC_P_SUPPORT_656_MASTER_MODE        (1) /* Has 656Out and run as master mode */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (1)
#define BVDC_P_SUPPORT_LOOP_BACK              (2) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_DNR                    (1) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0) /* TODO: recheck */

/* source */
#define BVDC_P_SUPPORT_GFD                    (1) /* Number of GFD HW */
#define BVDC_P_SUPPORT_MFD                    (2) /* Number of MFD HW */
#if (BCHP_CHIP==73465)
#define BVDC_P_SUPPORT_MTG                    (2) /* MFD Trigger Generator */
#else
#define BVDC_P_SUPPORT_MTG                    (0) /* MFD Trigger Generator */
#endif
#if (BCHP_VER >= BCHP_VER_B0) || (BCHP_CHIP==73465)
#if (BCHP_CHIP==7231) || (BCHP_CHIP==7344)

#if ((BCHP_CHIP==7231) && (BCHP_VER >= BCHP_VER_B2) || \
     (BCHP_CHIP==7344) && (BCHP_VER >= BCHP_VER_B1)) /* has BVDC_P_VEC_STANDALONE_BUG_FIXED */
#define BVDC_P_SUPPORT_IT_VER                 (3) /* IT HW version */
#else
#define BVDC_P_SUPPORT_IT_VER                 (2) /* IT HW version */
#endif
#define BVDC_P_SUPPORT_VEC_GRPD               (1) /* Support GRPD hw */
#else /* 7346Bx */
#define BVDC_P_SUPPORT_IT_VER                 (3) /* IT HW version */
#define BVDC_P_SUPPORT_VEC_GRPD               (0) /* Support GRPD hw */
#endif
#if (BCHP_CHIP==7231)
#define BVDC_P_SUPPORT_HDDVI                  (1) /* Number of HDDVI HW */
#define BVDC_P_SUPPORT_HDDVI_VER              (8) /* HDDVI HW version */
#define BVDC_P_SUPPORT_DRAIN_F                (3) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_TDAC_VER               (11)/* DAC HW version */
#else  /* 7344 & 7346 only */
#define BVDC_P_SUPPORT_HDDVI                  (0) /* Number of HDDVI HW */
#define BVDC_P_SUPPORT_HDDVI_VER              (0) /* HDDVI HW version */
#define BVDC_P_SUPPORT_DRAIN_F                (2) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_TDAC_VER               (12)/* DAC HW version */
#endif /* 7344, 7346, and 7231 Bx*/
#define BVDC_P_SUPPORT_GFD_VER                (5) /* GFD_0 HW version */
#define BVDC_P_SUPPORT_GFD1_VER               (5) /* GFD_1 HW version */
#if (BCHP_CHIP==73465)
#define BVDC_P_SUPPORT_MFD_VER                (16)/* MFD HW version */
#else
#define BVDC_P_SUPPORT_MFD_VER                (13)/* MFD HW version */
#endif
#define BVDC_P_SUPPORT_SCL_VER                (8) /* SCL HW version */
#define BVDC_P_SUPPORT_HSCL_VER               (6) /* HSCL HW version */
#define BVDC_P_SUPPORT_MADR_VER               (5) /* MAD-R HW version */
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (1)
#define BVDC_P_SUPPORT_CAP_VER                (5) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_CLOCK_GATING           (1) /* Clock gating for power saving */
#else  /* All Ax */
#define BVDC_P_SUPPORT_GFD_VER                (4) /* GFD_0 HW version */
#define BVDC_P_SUPPORT_GFD1_VER               (4) /* GFD_1 HW version */
#define BVDC_P_SUPPORT_HDDVI                  (0) /* Number of HDDVI HW */
#define BVDC_P_SUPPORT_HDDVI_VER              (0) /* HDDVI HW version */
#define BVDC_P_SUPPORT_DRAIN_F                (2) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_VEC_GRPD               (0) /* Support GRPD hw */
#define BVDC_P_SUPPORT_MFD_VER                (10)/* MFD HW version */
#define BVDC_P_SUPPORT_TDAC_VER               (9) /* DAC HW version */
#define BVDC_P_SUPPORT_SCL_VER                (7) /* SCL HW version */
#define BVDC_P_SUPPORT_HSCL_VER               (5) /* HSCL HW version */
#define BVDC_P_SUPPORT_IT_VER                 (2) /* IT HW version */
#define BVDC_P_SUPPORT_MADR_VER               (2) /* MAD-R HW version */
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (0)
#define BVDC_P_SUPPORT_CAP_VER                (4) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_CLOCK_GATING           (0) /* Clock gating for power saving */
#endif
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (0)  /*Dynamic Memory Power Gating*/

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1) /* Number of PEP HW, see CMP_x */
#define BVDC_P_SUPPORT_PEP_VER                (1) /* PEP HW version */
#define BVDC_P_SUPPORT_HIST                   (1) /* Number of HIST HW */
#define BVDC_P_SUPPORT_TNT                    (1) /* Number of TNT HW */
#define BVDC_P_SUPPORT_TNT_VER                (6) /* TNT HW version */
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (1) /* Number of MASK HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (2) /* Number LBOX HW detect */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (1)
#define BVDC_P_SUPPORT_CAP                    (4) /* Number of CAP HW */
#define BVDC_P_SUPPORT_VFD                    (4) /* Number of VFD HW */
#define BVDC_P_SUPPORT_SCL                    (4) /* Number of SCL HW */
#define BVDC_P_SUPPORT_XSRC_VER               (0) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (0) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_DMISC                  (0) /* TODO: phase out */
#define BVDC_P_SUPPORT_FREE_CHANNEL           (5) /* Number of VNET_F_FCH_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (3) /* DRAIN HW version */
#define BVDC_P_SUPPORT_MCVP                   (1) /* TODO: Number of MCVP HW */
#define BVDC_P_SUPPORT_MCVP_VER               (3) /* TODO: MCVP HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (0) /* TODO: MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (1) /* TODO: Number of MAD-R HW */
#define BVDC_P_SUPPORT_MANR                   (0) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (0) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (4) /* Testfeature HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2) /* Number of CMP_0_Vx */
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_0_Gx */
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (2) /* Number of CMP_1_Vx */
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_1_Gx */
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_2_Vx */
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_2_Gx */
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1) /* TODO: const color */
#define BVDC_P_MIN_XCODE_CMP                  (0)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (0)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_HDMI_RM_VER            (5)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (0)

#define BVDC_P_MAX_DACS                       (4) /* Number of DAC output */
#define BVDC_P_ORTHOGONAL_VEC_VER             (0)
#define BVDC_P_SUPPORT_HD_DAC                 (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_STG_VER                (0)
#define BVDC_P_SUPPORT_DTG_RMD                (0)
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)

#define BVDC_P_NUM_SHARED_656                 (1) /* Number VEC 656 HW */
#define BVDC_P_NUM_SHARED_DVI                 (1) /* Number VEC Digital output HW */
#define BVDC_P_NUM_SHARED_STG                 (0) /* Number of STG HW */
#define BVDC_P_NUM_SHARED_RF                  (1)
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_NUM_SHARED_VF                  (2) /* Number VEC's VF HW, VEC_CFG_VF_x_SOURCE */
#define BVDC_P_NUM_SHARED_SDSRC               (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_HDSRC               (1) /* Number VEC's VF HW, VEC_CFG_HDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_SM                  (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1) /* Number of VEC's SECAM HW, VEC_CFG_SECAM_x_SOURCE */
#define BVDC_P_NUM_SHARED_SECAM_HD            (2) /* Number of VEC's SECAM Passthru HW */
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_STG                    (0) /* STG HW */
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_216_SYSCLK)

#elif (BCHP_CHIP==7429) || (BCHP_CHIP==74295)

#if (BCHP_CHIP==74295)
#define BVDC_P_SUPPORT_MFD_VER                (16)/* MFD HW version */
#elif ((BCHP_CHIP==7429) && (BCHP_VER >= BCHP_VER_B0))
#define BVDC_P_SUPPORT_MFD_VER                (14)/* MFD HW version */
#else
#define BVDC_P_SUPPORT_MFD_VER                (13)/* MFD HW version */
#endif
#if ((BCHP_CHIP==7429) && (BCHP_VER >= BCHP_VER_B0)) || (BCHP_CHIP==74295)
#define BVDC_P_SUPPORT_MADR_VER               (7) /* MAD-R HW version */
#define BVDC_P_SUPPORT_MCVP_VER               (4) /* TODO: MCVP HW version */
#define BVDC_P_SUPPORT_SCL_VER                (9) /* SCL HW version */
#else
#define BVDC_P_SUPPORT_MADR_VER               (5) /* MAD-R HW version */
#define BVDC_P_SUPPORT_MCVP_VER               (3) /* TODO: MCVP HW version */
#define BVDC_P_SUPPORT_SCL_VER                (8) /* SCL HW version */
#endif
#define BVDC_P_SUPPORT_656_MASTER_MODE        (1) /* Has 656Out and run as master mode */
#define BVDC_P_SUPPORT_LOOP_BACK              (2) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_DNR                    (1) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0) /* TODO: recheck */
#define BVDC_P_SUPPORT_CAP_VER                (5) /* CAP HW version, see bvdc_capture_priv.h */

/* source */
#define BVDC_P_SUPPORT_GFD                    (2) /* Number of GFD HW */
#define BVDC_P_SUPPORT_GFD_VER                (6) /* GFD_0 HW version */
#define BVDC_P_SUPPORT_GFD1_VER               (6) /* GFD_1 HW version */
#define BVDC_P_SUPPORT_MFD                    (2) /* Number of MFD HW */
#if (BCHP_CHIP==74295)
#define BVDC_P_SUPPORT_MTG                    (2) /* MFD Trigger Generator */
#else
#define BVDC_P_SUPPORT_MTG                    (0) /* MFD Trigger Generator */
#endif
#define BVDC_P_SUPPORT_HDDVI                  (1) /* Number of HDDVI HW */
#define BVDC_P_SUPPORT_HDDVI_VER              (8) /* HDDVI HW version */
#define BVDC_P_SUPPORT_DRAIN_F                (3) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_VEC_GRPD               (1) /* Support GRPD hw */
#define BVDC_P_SUPPORT_IT_VER                 (3) /* IT HW version */
#define BVDC_P_SUPPORT_TDAC_VER               (12)/* DAC HW version */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#define BVDC_P_SUPPORT_HSCL_VER               (6) /* HSCL HW version */

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1) /* Number of PEP HW, see CMP_x */
#define BVDC_P_SUPPORT_PEP_VER                (1) /* PEP HW version */
#define BVDC_P_SUPPORT_HIST                   (1) /* Number of HIST HW */
#define BVDC_P_SUPPORT_TNT                    (1) /* Number of TNT HW */
#define BVDC_P_SUPPORT_TNT_VER                (6) /* TNT HW version */
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (1) /* Number of MASK HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (2) /* Number LBOX HW detect */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (1)
#define BVDC_P_SUPPORT_CAP                    (4) /* Number of CAP HW */
#define BVDC_P_SUPPORT_VFD                    (4) /* Number of VFD HW */
#define BVDC_P_SUPPORT_SCL                    (4) /* Number of SCL HW */
#define BVDC_P_SUPPORT_XSRC_VER               (0) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (0) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_DMISC                  (0) /* TODO: phase out */
#define BVDC_P_SUPPORT_FREE_CHANNEL           (5) /* Number of VNET_F_FCH_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (2) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (3) /* DRAIN HW version */
#define BVDC_P_SUPPORT_MCVP                   (1) /* TODO: Number of MCVP HW */
#define BVDC_P_SUPPORT_MCDI_VER               (0) /* TODO: MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (1) /* TODO: Number of MAD-R HW */
#define BVDC_P_SUPPORT_MANR                   (0) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (0) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (4) /* Testfeature HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2) /* Number of CMP_0_Vx */
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_0_Gx */
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (2) /* Number of CMP_1_Vx */
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_1_Gx */
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_2_Vx */
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_2_Gx */
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (0) /* TODO: const color */
#define BVDC_P_MIN_XCODE_CMP                  (0)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (0)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_HDMI_RM_VER            (5)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)

#define BVDC_P_MAX_DACS                       (4) /* Number of DAC output */
#define BVDC_P_ORTHOGONAL_VEC_VER             (1)
#define BVDC_P_SUPPORT_HD_DAC                 (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_SUPPORT_DSCL                   (1)
#define BVDC_P_SUPPORT_DSCL_VER               (2)

#define BVDC_P_NUM_SHARED_656                 (1) /* Number VEC 656 HW */
#define BVDC_P_NUM_SHARED_DVI                 (1) /* Number VEC Digital output HW */
#define BVDC_P_NUM_SHARED_STG                 (0) /* Number of STG HW */
#define BVDC_P_NUM_SHARED_RF                  (1) /* Number of RFM output */
#define BVDC_P_NUM_SHARED_IT                  (2) /* Numter VEC's VF HW, VEC_CFG_IT_x_SOURCE */
#define BVDC_P_NUM_SHARED_VF                  (2) /* Number VEC's VF HW, VEC_CFG_VF_x_SOURCE */
#define BVDC_P_NUM_SHARED_SDSRC               (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_HDSRC               (1) /* Number VEC's VF HW, VEC_CFG_HDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_SM                  (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1) /* Number of VEC's SECAM HW, VEC_CFG_SECAM_x_SOURCE */
#define BVDC_P_NUM_SHARED_SECAM_HD            (2) /* Number of VEC's SECAM Passthru HW */
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_CLOCK_GATING           (1) /* Clock gating for power saving */
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (0)  /*Dynamic Memory Power Gating*/
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (0) /* Independent source clipping in 3D mode */
#define BVDC_P_SUPPORT_STG                    (0) /* STG HW */
#define BVDC_P_SUPPORT_STG_VER                (0)
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_216_SYSCLK)

#elif (BCHP_CHIP==7435)

#define BVDC_P_SUPPORT_656_MASTER_MODE        (0)
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0)
#define BVDC_P_SUPPORT_VEC_GRPD               (0)

/* source */
#define BVDC_P_SUPPORT_GFD                    (6)
#define BVDC_P_SUPPORT_GFD_VER                (6)
#define BVDC_P_SUPPORT_GFD1_VER               (6)
#define BVDC_P_SUPPORT_MFD                    (4)
#define BVDC_P_SUPPORT_MFD_VER                (14)/* MFD HW version */
#define BVDC_P_SUPPORT_MTG                    (0) /* MFD Trigger Generator */
#define BVDC_P_SUPPORT_TDAC_VER               (12)/* TDAC/QDAC HW version */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#define BVDC_P_SUPPORT_DNR                    (4) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_BOX_DETECT             (4) /* Number LBOX HW detect */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (2)
#define BVDC_P_SUPPORT_HDDVI_VER              (9)
#define BVDC_P_SUPPORT_HDDVI                  (1)

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1)
#define BVDC_P_SUPPORT_PEP_VER                (1)
#define BVDC_P_SUPPORT_HIST                   (1)
#define BVDC_P_SUPPORT_TNT                    (1)
#define BVDC_P_SUPPORT_TNT_VER                (6)
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (1)
#define BVDC_P_SUPPORT_CAP                    (6)
#define BVDC_P_SUPPORT_CAP_VER                (6) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_VFD                    (6)
#define BVDC_P_SUPPORT_SCL                    (6)
#define BVDC_P_SUPPORT_SCL_VER                (9) /* SCL HW version */
#define BVDC_P_SUPPORT_XSRC_VER               (0) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (0) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_HSCL_VER               (6) /* HSCL HW version */
#define BVDC_P_SUPPORT_DMISC                  (0)
#define BVDC_P_SUPPORT_LOOP_BACK              (9) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_FREE_CHANNEL           (10)
#define BVDC_P_SUPPORT_DRAIN_F                (5) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (2) /* DRAIN HW version */
#define BVDC_P_SUPPORT_STG                    (4) /* Number of STG HW */
#define BVDC_P_SUPPORT_STG_VER                (3) /* STG HW version */
#define BVDC_P_NUM_SHARED_STG                 (4)
#define BVDC_P_ORTHOGONAL_VEC_VER             (1)
#define BVDC_P_SUPPORT_IT_VER                 (3)
#define BVDC_P_SUPPORT_MCVP                   (5) /* Number of MCVP HW */
#define BVDC_P_SUPPORT_MCVP_VER               (4) /* MCVP HW version */
#define BVDC_P_SUPPORT_MADR                   (4) /* Number of MAD-R HW */
#if (BCHP_VER >= BCHP_VER_B0)
#define BVDC_P_SUPPORT_MADR_VER               (7) /* MAD-R HW version */
#else
#define BVDC_P_SUPPORT_MADR_VER               (6) /* MAD-R HW version */
#endif
#define BVDC_P_SUPPORT_MCDI_VER               (5) /* MCDI HW version */
#define BVDC_P_SUPPORT_MANR                   (1) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (3) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (4) /* Testfeature HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1)
#define BVDC_P_MIN_XCODE_CMP                  (0)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (1)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1)
#define BVDC_P_SUPPORT_HDMI_RM_VER            (5)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)

#define BVDC_P_MAX_DACS                       (4)
#define BVDC_P_SUPPORT_HD_DAC                 (1)
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_SUPPORT_DSCL                   (1)
#define BVDC_P_SUPPORT_DSCL_VER               (2)
#define BVDC_P_NUM_SHARED_656                 (1)
#define BVDC_P_NUM_SHARED_DVI                 (1)
#define BVDC_P_NUM_SHARED_RF                  (1)
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_NUM_SHARED_VF                  (2)
#define BVDC_P_NUM_SHARED_SDSRC               (1)
#define BVDC_P_NUM_SHARED_HDSRC               (1)
#define BVDC_P_NUM_SHARED_SM                  (1)
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1)
#define BVDC_P_NUM_SHARED_SECAM_HD            (2)
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_CLOCK_GATING           (1)
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (0)  /*Dynamic Memory Power Gating*/
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_216_SYSCLK)

#elif ((BCHP_CHIP==7360)||(BCHP_CHIP==7362)||(BCHP_CHIP==7228) || (BCHP_CHIP==73625))

#if (BCHP_CHIP==73625)
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (2)
#define BVDC_P_SUPPORT_MFD_VER                (16)/* MFD HW version */
#define BVDC_P_SUPPORT_DRAIN_VER              (4) /* DRAIN HW version */
#define BVDC_P_SUPPORT_SCL_VER                (9)
#define BVDC_P_SUPPORT_CAP_VER                (5)
#define BVDC_P_SUPPORT_HSCL_VER               (6)
#else
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (1)
#define BVDC_P_SUPPORT_MFD_VER                (12)/* MFD HW version */
#define BVDC_P_SUPPORT_DRAIN_VER              (3) /* DRAIN HW version */
#define BVDC_P_SUPPORT_SCL_VER                (7)
#define BVDC_P_SUPPORT_CAP_VER                (4)
#define BVDC_P_SUPPORT_HSCL_VER               (5)
#endif
#define BVDC_P_SUPPORT_656_MASTER_MODE        (0)
#define BVDC_P_SUPPORT_TDAC_VER              (10) /* TODO */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#define BVDC_P_SUPPORT_LOOP_BACK              (2)
#define BVDC_P_SUPPORT_DNR                    (1)
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0)
#define BVDC_P_SUPPORT_VEC_GRPD               (0)

/* source */
#define BVDC_P_SUPPORT_GFD                    (2)
#define BVDC_P_SUPPORT_GFD_VER                (4)
#define BVDC_P_SUPPORT_GFD1_VER               (4)
#define BVDC_P_SUPPORT_MFD                    (1)
#if (BCHP_CHIP==73625)
#define BVDC_P_SUPPORT_MTG                    (1) /* MFD Trigger Generator */
#else
#define BVDC_P_SUPPORT_MTG                    (0) /* MFD Trigger Generator */
#endif
#define BVDC_P_SUPPORT_MADR_VER               (5) /* MAD-R HW version */
#define BVDC_P_SUPPORT_HDDVI                  (0)
#define BVDC_P_SUPPORT_HDDVI_VER              (0)

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1)
#define BVDC_P_SUPPORT_PEP_VER                (1)
#define BVDC_P_SUPPORT_HIST                   (1)
#define BVDC_P_SUPPORT_TNT                    (1)
#define BVDC_P_SUPPORT_TNT_VER                (6)
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (1)
#define BVDC_P_SUPPORT_BOX_DETECT             (1)
#define BVDC_P_SUPPORT_CAP                    (2)
#define BVDC_P_SUPPORT_VFD                    (2)
#define BVDC_P_SUPPORT_SCL                    (2)
#define BVDC_P_SUPPORT_XSRC_VER               (0) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (0) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_DMISC                  (0)
#define BVDC_P_SUPPORT_FREE_CHANNEL           (3)
#define BVDC_P_SUPPORT_DRAIN_F                (1)
#define BVDC_P_SUPPORT_DRAIN_B                (1)

#define BVDC_P_SUPPORT_MCVP                   (1)
#define BVDC_P_SUPPORT_MCVP_VER               (3) /* TODO */
#define BVDC_P_SUPPORT_MCDI_VER               (0) /* TODO: MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (1) /* TODO: Number of MAD-R HW */
#define BVDC_P_SUPPORT_MANR                   (0) /* TODO: Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (0) /* TODO: ANR in MCVP version */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (4) /* Testfeature HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (0)
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (0)
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (0)
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (0)
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (0)
#define BVDC_P_MIN_XCODE_CMP                  (0)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (1)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (0)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1)
#define BVDC_P_SUPPORT_HDMI_RM_VER            (5)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_ITU656_OUT             (0)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)

#define BVDC_P_MAX_DACS                       (4)
#define BVDC_P_ORTHOGONAL_VEC_VER             (0)
#define BVDC_P_SUPPORT_HD_DAC                 (1)
#define BVDC_P_SUPPORT_IT_VER                 (3)
#define BVDC_P_SUPPORT_STG_VER                (0)
#define BVDC_P_SUPPORT_DTG_RMD                (0)
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)

#define BVDC_P_NUM_SHARED_656                 (1)
#define BVDC_P_NUM_SHARED_DVI                 (1)
#define BVDC_P_NUM_SHARED_STG                 (0)
#define BVDC_P_NUM_SHARED_RF                  (1)
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_NUM_SHARED_VF                  (2)
#define BVDC_P_NUM_SHARED_SDSRC               (1)
#define BVDC_P_NUM_SHARED_HDSRC               (1)
#define BVDC_P_NUM_SHARED_SM                  (1)
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1)
#define BVDC_P_NUM_SHARED_SECAM_HD            (2)
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_CLOCK_GATING           (1)
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (0)  /*Dynamic Memory Power Gating*/
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (0)
#define BVDC_P_SUPPORT_STG                    (0)
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_216_SYSCLK)

#elif (BCHP_CHIP==7584) || (BCHP_CHIP==75845)

#if (BCHP_CHIP==75845)
#define BVDC_P_SUPPORT_MFD_VER                (16)/* MFD HW version */
#else
#define BVDC_P_SUPPORT_MFD_VER                (14)/* MFD HW version */
#endif
#define BVDC_P_SUPPORT_656_MASTER_MODE        (1) /* Has 656Out and run as master mode */
#define BVDC_P_SUPPORT_TDAC_VER               (12)/* DAC HW version */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#define BVDC_P_SUPPORT_LOOP_BACK              (2) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_DNR                    (1) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0) /* TODO: recheck */
#define BVDC_P_SUPPORT_VEC_GRPD               (1) /* Support GRPD hw */

/* source */
#define BVDC_P_SUPPORT_GFD                    (1) /* Number of GFD HW */
#define BVDC_P_SUPPORT_GFD_VER                (6) /* GFD_0 HW version */
#define BVDC_P_SUPPORT_GFD1_VER               (6) /* GFD_1 HW version */
#define BVDC_P_SUPPORT_MFD                    (2) /* Number of MFD HW */
#if (BCHP_CHIP==75845)
#define BVDC_P_SUPPORT_MTG                    (2) /* MFD Trigger Generator */
#else
#define BVDC_P_SUPPORT_MTG                    (0) /* MFD Trigger Generator */
#endif
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (1)
#define BVDC_P_SUPPORT_CLOCK_GATING           (1) /* Clock gating for power saving */
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (0)  /*Dynamic Memory Power Gating*/
#define BVDC_P_SUPPORT_HDDVI                  (0) /* Number of HDDVI HW */
#define BVDC_P_SUPPORT_HDDVI_VER              (0) /* HDDVI HW version */

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1) /* Number of PEP HW, see CMP_x */
#define BVDC_P_SUPPORT_PEP_VER                (1) /* PEP HW version */
#define BVDC_P_SUPPORT_HIST                   (1) /* Number of HIST HW */
#define BVDC_P_SUPPORT_TNT                    (1) /* Number of TNT HW */
#define BVDC_P_SUPPORT_TNT_VER                (6) /* TNT HW version */
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (1) /* Number of MASK HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (2) /* Number LBOX HW detect */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (2)
#define BVDC_P_SUPPORT_CAP                    (4) /* Number of CAP HW */
#define BVDC_P_SUPPORT_CAP_VER                (5) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_VFD                    (4) /* Number of VFD HW */
#define BVDC_P_SUPPORT_SCL                    (4) /* Number of SCL HW */
#define BVDC_P_SUPPORT_SCL_VER                (9) /* SCL HW version */
#define BVDC_P_SUPPORT_XSRC_VER               (0) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (0) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_HSCL_VER               (6) /* HSCL HW version */
#define BVDC_P_SUPPORT_DMISC                  (0) /* TODO: phase out */
#define BVDC_P_SUPPORT_FREE_CHANNEL           (5) /* Number of VNET_F_FCH_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_F                (2) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (2) /* DRAIN HW version */
#define BVDC_P_SUPPORT_MCVP                   (1) /* TODO: Number of MCVP HW */
#define BVDC_P_SUPPORT_MCVP_VER               (3) /* TODO: MCVP HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (0) /* TODO: MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (1) /* TODO: Number of MAD-R HW */
#define BVDC_P_SUPPORT_MADR_VER               (7) /* MAD-R HW version */
#define BVDC_P_SUPPORT_MANR                   (0) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (0) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (4) /* Testfeature HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2) /* Number of CMP_0_Vx */
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_0_Gx */
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (2) /* Number of CMP_1_Vx */
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_1_Gx */
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_2_Vx */
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_2_Gx */
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1) /* TODO: const color */
#define BVDC_P_MIN_XCODE_CMP                  (0)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (0)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_HDMI_RM_VER            (5)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)

#define BVDC_P_MAX_DACS                       (4) /* Number of DAC output */
#define BVDC_P_ORTHOGONAL_VEC_VER             (1)
#define BVDC_P_SUPPORT_HD_DAC                 (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_IT_VER                 (3) /* IT HW version */
#define BVDC_P_SUPPORT_STG_VER                (0)
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)

#define BVDC_P_NUM_SHARED_656                 (1) /* Number VEC 656 HW */
#define BVDC_P_NUM_SHARED_DVI                 (1) /* Number VEC Digital output HW */
#define BVDC_P_NUM_SHARED_STG                 (0) /* Number of STG HW */
#define BVDC_P_NUM_SHARED_RF                  (1)
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_NUM_SHARED_VF                  (2) /* Number VEC's VF HW, VEC_CFG_VF_x_SOURCE */
#define BVDC_P_NUM_SHARED_SDSRC               (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_HDSRC               (1) /* Number VEC's VF HW, VEC_CFG_HDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_SM                  (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1) /* Number of VEC's SECAM HW, VEC_CFG_SECAM_x_SOURCE */
#define BVDC_P_NUM_SHARED_SECAM_HD            (2) /* Number of VEC's SECAM Passthru HW */
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_STG                    (0) /* STG HW */
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_216_SYSCLK)

#elif (BCHP_CHIP==7563) || (BCHP_CHIP==7543) || (BCHP_CHIP==75635)

#define BVDC_P_SUPPORT_656_MASTER_MODE        (1) /* Has 656Out and run as master mode */
#define BVDC_P_SUPPORT_TDAC_VER               (12)/* DAC HW version */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#define BVDC_P_SUPPORT_DNR                    (1) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0) /* TODO: recheck */
#define BVDC_P_SUPPORT_VEC_GRPD               (1) /* Support GRPD hw */

/* source */
#define BVDC_P_SUPPORT_GFD                    (1) /* Number of GFD HW */
#define BVDC_P_SUPPORT_GFD_VER                (6) /* GFD_0 HW version */
#define BVDC_P_SUPPORT_GFD1_VER               (6) /* GFD_1 HW version */
#define BVDC_P_SUPPORT_MFD                    (1) /* Number of MFD HW */
#if (BCHP_CHIP==75635)
#define BVDC_P_SUPPORT_MFD_VER                (16)/* MFD HW version */
#define BVDC_P_SUPPORT_MTG                    (1) /* MFD Trigger Generator */
#else
#define BVDC_P_SUPPORT_MFD_VER                (14)/* MFD HW version */
#define BVDC_P_SUPPORT_MTG                    (0) /* MFD Trigger Generator */
#endif
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (1)
#define BVDC_P_SUPPORT_CLOCK_GATING           (1) /* Clock gating for power saving */
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (0)  /*Dynamic Memory Power Gating*/
#define BVDC_P_SUPPORT_HDDVI                  (0) /* Number of HDDVI HW */
#define BVDC_P_SUPPORT_HDDVI_VER              (0) /* HDDVI HW version */

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1) /* Number of PEP HW, see CMP_x */
#define BVDC_P_SUPPORT_PEP_VER                (1) /* PEP HW version */
#define BVDC_P_SUPPORT_HIST                   (1) /* Number of HIST HW */
#define BVDC_P_SUPPORT_TNT                    (1) /* Number of TNT HW */
#define BVDC_P_SUPPORT_TNT_VER                (6) /* TNT HW version */
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (1) /* Number of MASK HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (1) /* Number LBOX HW detect */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (2)
#define BVDC_P_SUPPORT_CAP                    (2) /* Number of CAP HW */
#define BVDC_P_SUPPORT_CAP_VER                (5) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_VFD                    (2) /* Number of VFD HW */
#define BVDC_P_SUPPORT_SCL                    (2) /* Number of SCL HW */
#define BVDC_P_SUPPORT_SCL_VER                (8) /* SCL HW version */
#define BVDC_P_SUPPORT_XSRC_VER               (0) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (0) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_HSCL_VER               (6) /* HSCL HW version */
#define BVDC_P_SUPPORT_DMISC                  (0) /* TODO: phase out */
#define BVDC_P_SUPPORT_FREE_CHANNEL           (3) /* Number of VNET_F_FCH_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_F                (1) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (2) /* DRAIN HW version */
#define BVDC_P_SUPPORT_LOOP_BACK              (2) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_MCVP                   (1) /* TODO: Number of MCVP HW */
#define BVDC_P_SUPPORT_MCVP_VER               (4) /* TODO: MCVP HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (0) /* TODO: MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (1) /* TODO: Number of MAD-R HW */
#define BVDC_P_SUPPORT_MADR_VER               (7) /* MAD-R HW version */
#define BVDC_P_SUPPORT_MANR                   (0) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (0) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (4) /* Testfeature HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_0_Vx */
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_0_Gx */
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_1_Vx */
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_1_Gx */
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_2_Vx */
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_2_Gx */
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1) /* TODO: const color */
#define BVDC_P_MIN_XCODE_CMP                  (0)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (0)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (0)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_HDMI_RM_VER            (5)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_ITU656_OUT             (0)

#define BVDC_P_MAX_DACS                       (1) /* Number of DAC output */
#define BVDC_P_ORTHOGONAL_VEC_VER             (1)
#define BVDC_P_SUPPORT_HD_DAC                 (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_IT_VER                 (3) /* IT HW version */
#define BVDC_P_SUPPORT_STG_VER                (0)
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)

#define BVDC_P_NUM_SHARED_656                 (1) /* Number VEC 656 HW */
#define BVDC_P_NUM_SHARED_DVI                 (1) /* Number VEC Digital output HW */
#define BVDC_P_NUM_SHARED_STG                 (0) /* Number of STG HW */
#if (BCHP_CHIP==7563) || (BCHP_CHIP==75635)
#define BVDC_P_NUM_SHARED_IT                  (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)
#else
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (0)
#endif
#define BVDC_P_NUM_SHARED_RF                  (BVDC_P_SUPPORT_RFM_OUTPUT)
#define BVDC_P_NUM_SHARED_VF                  (1) /* Number VEC's VF HW, VEC_CFG_VF_x_SOURCE */
#define BVDC_P_NUM_SHARED_SDSRC               (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_HDSRC               (0) /* Number VEC's VF HW, VEC_CFG_HDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_SM                  (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1) /* Number of VEC's SECAM HW, VEC_CFG_SECAM_x_SOURCE */
#define BVDC_P_NUM_SHARED_SECAM_HD            (0) /* Number of VEC's SECAM Passthru HW */
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_STG                    (0) /* STG HW */
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_216_SYSCLK)

#elif (BCHP_CHIP==7445) || (BCHP_CHIP==11360)

#define BVDC_P_SUPPORT_656_MASTER_MODE        (0)
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0)
#define BVDC_P_SUPPORT_VEC_GRPD               (0)

/* source */
#define BVDC_P_SUPPORT_GFD                    (7)
#define BVDC_P_SUPPORT_GFD_VER                (7)
#define BVDC_P_SUPPORT_GFD1_VER               (7)
#define BVDC_P_SUPPORT_MFD                    (6)
#define BVDC_P_SUPPORT_MTG                    (6) /* MFD Trigger Generator */
#define BVDC_P_SUPPORT_TDAC_VER               (13)/* TDAC/QDAC HW version */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#define BVDC_P_SUPPORT_DNR                    (6) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_BOX_DETECT             (0) /* Number LBOX HW detect, See SW7445-782 */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (3)
#define BVDC_P_SUPPORT_HDDVI_VER              (11)
#define BVDC_P_SUPPORT_HDDVI                  (1)

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1)
#define BVDC_P_SUPPORT_PEP_VER                (6) /* TODO: add support for PEP change */
#define BVDC_P_SUPPORT_HIST                   (1)
#define BVDC_P_SUPPORT_TNT                    (1) /* TODO: add support to new TNT */
#define BVDC_P_SUPPORT_TNT_VER                (7)
#define BVDC_P_SUPPORT_TNTD                   (1)
#define BVDC_P_SUPPORT_TNTD_VER               (1)
#define BVDC_P_SUPPORT_MASK_DITHER            (1)
#define BVDC_P_SUPPORT_CAP                    (8)
#define BVDC_P_SUPPORT_CAP_VER                (7) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_VFD                    (8)
#define BVDC_P_SUPPORT_SCL                    (8)
#define BVDC_P_SUPPORT_SCL_VER                (11) /* TODO: add support for SCL change */
#define BVDC_P_SUPPORT_XSRC_VER               (1) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (2) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_HSCL_VER               (6) /* HSCL HW version */
#define BVDC_P_SUPPORT_DMISC                  (1)
#define BVDC_P_SUPPORT_LOOP_BACK              (15) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_FREE_CHANNEL           (10)
#define BVDC_P_SUPPORT_DRAIN_F                (2) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (5) /* DRAIN HW version */
#define BVDC_P_SUPPORT_STG                    (6) /* Number of STG HW */
#define BVDC_P_SUPPORT_STG_VER                (5) /* STG HW version */
#define BVDC_P_NUM_SHARED_STG                 (6)
#define BVDC_P_ORTHOGONAL_VEC_VER             (2)
#define BVDC_P_SUPPORT_IT_VER                 (3)
#define BVDC_P_SUPPORT_MCVP                   (6) /* TODO: add support for 6 MCVP */
#define BVDC_P_SUPPORT_MCVP_VER               (5) /* MCVP HW version */
#define BVDC_P_SUPPORT_MADR                   (5) /* Number of MAD-R HW */
#define BVDC_P_SUPPORT_MADR_VER               (10) /* MAD-R HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (8) /* MCDI HW version */
#define BVDC_P_SUPPORT_MANR                   (1) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (5) /* TODO: add support for ANR change */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (5) /* Testfeature HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (1)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (1)
#define BVDC_P_SUPPORT_MFD_VER                (16)/* MFD HW version */

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (16)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1)
#define BVDC_P_MIN_XCODE_CMP                  (3)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (1)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1)
#define BVDC_P_SUPPORT_HDMI_RM_VER            (7)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)

#define BVDC_P_MAX_DACS                       (4)
#define BVDC_P_SUPPORT_HD_DAC                 (1)
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)
#define BVDC_P_NUM_SHARED_656                 (1)
#define BVDC_P_NUM_SHARED_DVI                 (1)
#define BVDC_P_NUM_SHARED_RF                  (1)
#define BVDC_P_NUM_SHARED_IT                  (3)
#define BVDC_P_NUM_SHARED_VF                  (2)
#define BVDC_P_NUM_SHARED_SDSRC               (1)
#define BVDC_P_NUM_SHARED_HDSRC               (1)
#define BVDC_P_NUM_SHARED_SM                  (1)
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1)
#define BVDC_P_NUM_SHARED_SECAM_HD            (2)
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_CLOCK_GATING           (1)
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (1)  /*Dynamic Memory Power Gating*/
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_324_SYSCLK)

#elif ((BCHP_CHIP==7439) && (BCHP_VER >= BCHP_VER_B0))

#define BVDC_P_SUPPORT_XSRC                   (2) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_XSRC_VER               (2) /* XSRC HW version */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (0) /* Number LBOX HW detect, see SW7366-151 */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (3)
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (7) /* Testfeature HW version */
#define BVDC_P_SUPPORT_MANR_VER               (5) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_CAP_VER                (7) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_HDMI_RM_VER            (7)
#define BVDC_P_SUPPORT_MFD                    (4)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_MFD_VER                (16)/* MFD HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (9) /* MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (3) /* Number of MAD-R HW */
#define BVDC_P_SUPPORT_MADR_VER               (10)/* MAD-R HW version */
#define BVDC_P_SUPPORT_MCVP                   (4) /* Number of MCVP HW */
#define BVDC_P_SUPPORT_MCVP_VER               (6) /* MCVP HW version */
#define BVDC_P_SUPPORT_PEP_VER                (6)
#define BVDC_P_SUPPORT_SCL_VER                (11)/* SCL HW version */
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)
#define BVDC_P_SUPPORT_DNR                    (4) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_LOOP_BACK              (10) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_F                (2) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (2) /* DRAIN HW version */
#define BVDC_P_SUPPORT_MTG                    (4) /* MFD Trigger Generator */
#define BVDC_P_MAX_DACS                       (4)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (1)
#define BVDC_P_SUPPORT_656_MASTER_MODE        (0)
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0)
#define BVDC_P_SUPPORT_VEC_GRPD               (0)

/* source */
#define BVDC_P_SUPPORT_GFD                    (4)
#define BVDC_P_SUPPORT_GFD_VER                (9)
#define BVDC_P_SUPPORT_GFD1_VER               (9)
#define BVDC_P_SUPPORT_TDAC_VER               (13)/* TDAC/QDAC HW version */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#define BVDC_P_SUPPORT_HDDVI                  (1)
#define BVDC_P_SUPPORT_HDDVI_VER              (11)

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1)
#define BVDC_P_SUPPORT_HIST                   (1)
#define BVDC_P_SUPPORT_TNT                    (2)
#define BVDC_P_SUPPORT_TNT_VER                (6)
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (2)
#define BVDC_P_SUPPORT_CAP                    (6)
#define BVDC_P_SUPPORT_VFD                    (6)
#define BVDC_P_SUPPORT_SCL                    (6)
#define BVDC_P_SUPPORT_HSCL_VER               (7) /* HSCL HW version */
#define BVDC_P_SUPPORT_DMISC                  (1)
#define BVDC_P_SUPPORT_FREE_CHANNEL           (7) /* Number of VNET_F_FCH_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_STG                    (2) /* Number of STG HW  */
#define BVDC_P_SUPPORT_STG_VER                (4) /* STG HW version */
#define BVDC_P_NUM_SHARED_STG                 (2)
#define BVDC_P_ORTHOGONAL_VEC_VER             (2)
#define BVDC_P_SUPPORT_IT_VER                 (3)
#define BVDC_P_SUPPORT_MANR                   (1) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (1)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (6)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (6) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1)
#define BVDC_P_MIN_XCODE_CMP                  (2)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (4)
#define BVDC_P_SUPPORT_COLOR_CLIP             (1)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)

#define BVDC_P_SUPPORT_HD_DAC                 (1)
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_NUM_SHARED_656                 (1)
#define BVDC_P_NUM_SHARED_DVI                 (2)
#define BVDC_P_NUM_SHARED_RF                  (1)
#define BVDC_P_NUM_SHARED_IT                  (3)
#define BVDC_P_NUM_SHARED_VF                  (2)
#define BVDC_P_NUM_SHARED_HDSRC               (1)
#define BVDC_P_NUM_SHARED_SDSRC               (1)
#define BVDC_P_NUM_SHARED_SM                  (1)
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1)     /* Number of VEC's SECAM HW, SECAM_x_register*/
#define BVDC_P_NUM_SHARED_SECAM_HD            (2)     /* Number of VEC's SECAM Passthru HW VEC_CFG_SECAM_x_SOURCE - BVDC_P_NUM_SHARED_SECAM*/
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_CLOCK_GATING           (1)
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (1)  /*Dynamic Memory Power Gating*/
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_324_SYSCLK)

#elif (BCHP_CHIP==7366) || (BCHP_CHIP==7439) || \
      ((BCHP_CHIP==74371) && (BCHP_VER==BCHP_VER_A0))

#if BCHP_VER >= BCHP_VER_B0
#define BVDC_P_SUPPORT_HDDVI                  (0)
#define BVDC_P_SUPPORT_XSRC_VER               (1) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (2) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (0) /* Number LBOX HW detect, see SW7366-151 */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (3)
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (6) /* Testfeature HW version */
#define BVDC_P_SUPPORT_MANR_VER               (5) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_CAP_VER                (7) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_HDMI_RM_VER            (7)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_MFD                    (3)
#define BVDC_P_SUPPORT_MFD_VER                (16)/* MFD HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (8) /* MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (2) /* Number of MAD-R HW */
#define BVDC_P_SUPPORT_MADR_VER               (10)/* MAD-R HW version */
#define BVDC_P_SUPPORT_MCVP                   (3) /* Number of MCVP HW */
#define BVDC_P_SUPPORT_MCVP_VER               (5) /* MCVP HW version */
#define BVDC_P_SUPPORT_PEP_VER                (6)
#define BVDC_P_SUPPORT_SCL_VER                (11)/* SCL HW version */
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)
#define BVDC_P_SUPPORT_DNR                    (3) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_LOOP_BACK              (8) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_F                (2) /* Number of VNET_F_DRAIN_x_SRC */
#if (BCHP_CHIP==7366)
#define BVDC_P_SUPPORT_DRAIN_VER              (5) /* DRAIN HW version */
#define BVDC_P_SUPPORT_MTG                    (3) /* MFD Trigger Generator */
#else
#define BVDC_P_SUPPORT_DRAIN_VER              (2) /* DRAIN HW version */
#define BVDC_P_SUPPORT_MTG                    (2) /* MFD Trigger Generator */
#endif
#define BVDC_P_MAX_DACS                       (3)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (1)
#else
#define BVDC_P_SUPPORT_HDDVI                  (1)
#define BVDC_P_SUPPORT_XSRC_VER               (0) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (0) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (2) /* Number LBOX HW detect */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (3)
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (4) /* Testfeature HW version */
#define BVDC_P_SUPPORT_MANR_VER               (4) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_CAP_VER                (6) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_HDMI_RM_VER            (6)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_MFD                    (2)
#define BVDC_P_SUPPORT_MFD_VER                (14)/* MFD HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (7) /* MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (1) /* Number of MAD-R HW */
#define BVDC_P_SUPPORT_MADR_VER               (8) /* MAD-R HW version */
#define BVDC_P_SUPPORT_MCVP                   (2) /* Number of MCVP HW */
#define BVDC_P_SUPPORT_MCVP_VER               (4) /* MCVP HW version */
#define BVDC_P_SUPPORT_PEP_VER                (1)
#define BVDC_P_SUPPORT_SCL_VER               (10) /* SCL HW version */
#define BVDC_P_SUPPORT_DSCL                   (1)
#define BVDC_P_SUPPORT_DSCL_VER               (2)
#define BVDC_P_SUPPORT_DNR                    (2) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_LOOP_BACK              (4) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_F                (3) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (2) /* DRAIN HW version */
#define BVDC_P_MAX_DACS                       (1)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)
#endif

#define BVDC_P_SUPPORT_656_MASTER_MODE        (0)
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0)
#define BVDC_P_SUPPORT_VEC_GRPD               (0)

/* source */
#define BVDC_P_SUPPORT_GFD                    (3)
#define BVDC_P_SUPPORT_GFD_VER                (6)
#define BVDC_P_SUPPORT_GFD1_VER               (6)
#define BVDC_P_SUPPORT_TDAC_VER               (13)/* TDAC/QDAC HW version */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#if (BCHP_CHIP==7366)
#define BVDC_P_SUPPORT_HDDVI_VER              (9)
#else
#define BVDC_P_SUPPORT_HDDVI_VER              (10)
#endif

/* BVN */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_PEP                    (1)
#define BVDC_P_SUPPORT_HIST                   (1)
#define BVDC_P_SUPPORT_TNT                    (2)
#define BVDC_P_SUPPORT_TNT_VER                (6)
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (2)
#define BVDC_P_SUPPORT_CAP                    (4)
#define BVDC_P_SUPPORT_VFD                    (4)
#define BVDC_P_SUPPORT_SCL                    (4)
#define BVDC_P_SUPPORT_HSCL_VER               (6) /* HSCL HW version */
#define BVDC_P_SUPPORT_DMISC                  (1)
#define BVDC_P_SUPPORT_FREE_CHANNEL           (6)
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#if (BCHP_CHIP==7366)
#if (BCHP_VER >= BCHP_VER_B0)
#define BVDC_P_SUPPORT_STG                    (2) /* Number of STG HW  */
#define BVDC_P_SUPPORT_STG_VER                (4) /* STG HW version */
#define BVDC_P_NUM_SHARED_STG                 (2)
#define BVDC_P_NUM_SHARED_HDSRC               (1)
#else
#define BVDC_P_SUPPORT_STG                    (0) /* Number of STG HW  */
#define BVDC_P_SUPPORT_STG_VER                (4) /* STG HW version */
#define BVDC_P_NUM_SHARED_STG                 (0)
#define BVDC_P_NUM_SHARED_HDSRC               (0)
#endif
#else    /* 7439 / 74371 */
#if (BCHP_CHIP==74371)
#define BVDC_P_SUPPORT_STG                    (0) /* Number of STG HW  */
#define BVDC_P_SUPPORT_STG_VER                (0) /* STG HW version */
#else    /* 7439 */
#define BVDC_P_SUPPORT_STG                    (2) /* Number of STG HW  */
#define BVDC_P_SUPPORT_STG_VER                (4) /* STG HW version */
#endif
#define BVDC_P_NUM_SHARED_STG                 (2)
#define BVDC_P_NUM_SHARED_HDSRC               (0)
#endif
#define BVDC_P_ORTHOGONAL_VEC_VER             (2)
#define BVDC_P_SUPPORT_IT_VER                 (3)
#define BVDC_P_SUPPORT_MANR                   (1) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1)
#define BVDC_P_MIN_XCODE_CMP                  (2)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (1)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (2)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)

#define BVDC_P_SUPPORT_HD_DAC                 (1)
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_NUM_SHARED_656                 (1)
#define BVDC_P_NUM_SHARED_DVI                 (2)
#define BVDC_P_NUM_SHARED_RF                  (1)
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_NUM_SHARED_VF                  (1)
#define BVDC_P_NUM_SHARED_SDSRC               (1)
#define BVDC_P_NUM_SHARED_SM                  (1)
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1)
#define BVDC_P_NUM_SHARED_SECAM_HD            (0)
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_CLOCK_GATING           (1)
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (1)  /*Dynamic Memory Power Gating*/
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_216_SYSCLK)

#elif (BCHP_CHIP==7364) || (BCHP_CHIP==7250)

#define BVDC_P_SUPPORT_656_MASTER_MODE        (1) /* Has 656Out and run as master mode */
#define BVDC_P_SUPPORT_LOOP_BACK              (4) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_DNR                    (2) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0) /* TODO: recheck */
#define BVDC_P_SUPPORT_CAP_VER                (7) /* CAP HW version, see bvdc_capture_priv.h */

/* source */
#define BVDC_P_SUPPORT_GFD                    (2) /* Number of GFD HW */
#define BVDC_P_SUPPORT_GFD_VER                (6) /* GFD_0 HW version */
#define BVDC_P_SUPPORT_GFD1_VER               (6) /* GFD_1 HW version */
#define BVDC_P_SUPPORT_MFD                    (2) /* Number of MFD HW */
#define BVDC_P_SUPPORT_MTG                    (2) /* MFD Trigger Generator */
#if ((BCHP_CHIP==7364) && (BCHP_VER>=BCHP_VER_C0) || \
     (BCHP_CHIP==7250) && (BCHP_VER>=BCHP_VER_B0))
#define BVDC_P_SUPPORT_HDDVI                  (0) /* Number of HDDVI HW */
#define BVDC_P_SUPPORT_HDDVI_VER              (0) /* HDDVI HW version */
#else
#define BVDC_P_SUPPORT_HDDVI                  (1)  /* Number of HDDVI HW */
#define BVDC_P_SUPPORT_HDDVI_VER              (11) /* HDDVI HW version */
#endif
#define BVDC_P_SUPPORT_DRAIN_F                (3) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_VEC_GRPD               (0) /* Support GRPD hw */
#define BVDC_P_SUPPORT_IT_VER                 (3) /* IT HW version */
#define BVDC_P_SUPPORT_MFD_VER                (16)/* MFD HW version */
#define BVDC_P_SUPPORT_TDAC_VER               (13)/* DAC HW version */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#define BVDC_P_SUPPORT_SCL_VER                (11) /* SCL HW version */
#define BVDC_P_SUPPORT_XSRC_VER               (2)  /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (1)  /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_HSCL_VER               (7)  /* HSCL HW version */
#define BVDC_P_SUPPORT_MADR_VER               (10) /* MAD-R HW version */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1) /* Number of PEP HW, see CMP_x */
#define BVDC_P_SUPPORT_PEP_VER                (5) /* TODO: PEP HW version */
#define BVDC_P_SUPPORT_HIST                   (1) /* Number of HIST HW */
#define BVDC_P_SUPPORT_TNT                    (1) /* Number of TNT HW */
#define BVDC_P_SUPPORT_TNT_VER                (6) /* TNT HW version */
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (1) /* Number of MASK HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (1) /* Number LBOX HW detect */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (3)
#define BVDC_P_SUPPORT_CAP                    (2) /* Number of CAP HW */
#define BVDC_P_SUPPORT_VFD                    (2) /* Number of VFD HW */
#define BVDC_P_SUPPORT_SCL                    (2) /* Number of SCL HW */
#define BVDC_P_SUPPORT_DMISC                  (1) /* TODO: phase out */
#define BVDC_P_SUPPORT_FREE_CHANNEL           (4) /* Number of VNET_F_FCH_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (4) /* DRAIN HW version */
#define BVDC_P_SUPPORT_MCVP                   (1) /* TODO: Number of MCVP HW */
#define BVDC_P_SUPPORT_MCVP_VER               (6) /* TODO: MCVP HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (0) /* TODO: MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (1) /* TODO: Number of MAD-R HW */
#define BVDC_P_SUPPORT_MANR                   (0) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (0) /* ANR in MCVP version */
#if (BCHP_CHIP==7364) && (BCHP_VER>=BCHP_VER_C0)
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (8) /* Testfeature HW version */
#else
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (6) /* Testfeature HW version */
#endif
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2) /* Number of CMP_0_Vx */
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_0_Gx */
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_1_Vx */
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_1_Gx */
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_2_Vx */
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_2_Gx */
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1) /* TODO: const color */
#define BVDC_P_MIN_XCODE_CMP                  (0)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (0)

/* display */
#if (BCHP_CHIP==7364)
#define BVDC_P_MAX_DACS                       (3) /* Number of DAC output */
#define BVDC_P_NUM_SHARED_VF                  (1) /* Number VEC's VF HW, VEC_CFG_VF_x_SOURCE */
#define BVDC_P_NUM_SHARED_SECAM               (1) /* Number of VEC's SECAM HW, VEC_CFG_SECAM_x_SOURCE */
#define BVDC_P_NUM_SHARED_SECAM_HD            (0) /* Number of VEC's SECAM Passthru HW */
#else
#define BVDC_P_MAX_DACS                       (4) /* Number of DAC output */
#define BVDC_P_NUM_SHARED_VF                  (2) /* Number VEC's VF HW, VEC_CFG_VF_x_SOURCE */
#define BVDC_P_NUM_SHARED_SECAM               (1) /* Number of VEC's SECAM HW, VEC_CFG_SECAM_x_SOURCE */
#define BVDC_P_NUM_SHARED_SECAM_HD            (2) /* Number of VEC's SECAM Passthru HW */
#endif

#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_HDMI_RM_VER            (7)
#if (BCHP_CHIP==7364 || (BCHP_CHIP==7250 && BCHP_VER>=BCHP_VER_B0))
#define BVDC_P_SUPPORT_MHL                    (1)
#else
#define BVDC_P_SUPPORT_MHL                    (0)
#endif
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (1)

#define BVDC_P_ORTHOGONAL_VEC_VER             (2)
#define BVDC_P_SUPPORT_HD_DAC                 (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_SUPPORT_DSCL                   (1)
#define BVDC_P_SUPPORT_DSCL_VER               (3)

#define BVDC_P_NUM_SHARED_656                 (1) /* Number VEC 656 HW */
#define BVDC_P_NUM_SHARED_DVI                 (1) /* Number VEC Digital output HW */
#define BVDC_P_NUM_SHARED_STG                 (1) /* Number of STG HW */
#define BVDC_P_NUM_SHARED_RF                  (1) /* Number of RFM output */
#define BVDC_P_NUM_SHARED_IT                  (2) /* Numter VEC's VF HW, VEC_CFG_IT_x_SOURCE */
#define BVDC_P_NUM_SHARED_SDSRC               (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_HDSRC               (1) /* Number VEC's VF HW, VEC_CFG_HDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_SM                  (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_CLOCK_GATING           (1) /* Clock gating for power saving */
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (1)  /*Dynamic Memory Power Gating*/
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (0) /* Independent source clipping in 3D mode */
#define BVDC_P_SUPPORT_STG                    (1) /* STG HW */
#define BVDC_P_SUPPORT_STG_VER                (4)
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_324_SYSCLK)
#define BVDC_P_MANAGE_VIP                     (1) /* VDC owns VIP */

#elif (BCHP_CHIP==7586)

#define BVDC_P_SUPPORT_656_MASTER_MODE        (1) /* Has 656Out and run as master mode */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#define BVDC_P_SUPPORT_LOOP_BACK              (4) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_DNR                    (1) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0) /* TODO: recheck */

/* source */
#define BVDC_P_SUPPORT_GFD                    (2) /* Number of GFD HW */
#define BVDC_P_SUPPORT_MFD                    (2) /* Number of MFD HW */
#define BVDC_P_SUPPORT_MTG                    (2) /* MFD Trigger Generator */
#define BVDC_P_SUPPORT_GFD_VER                (4) /* GFD_0 HW version */
#define BVDC_P_SUPPORT_GFD1_VER               (4) /* GFD_1 HW version */
#define BVDC_P_SUPPORT_HDDVI                  (1) /* Number of HDDVI HW */
#define BVDC_P_SUPPORT_HDDVI_VER              (12) /* HDDVI HW version */
#define BVDC_P_SUPPORT_DRAIN_F                (2) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_VEC_GRPD               (0) /* Support GRPD hw */
#define BVDC_P_SUPPORT_MFD_VER                (16)/* MFD HW version */
#define BVDC_P_SUPPORT_TDAC_VER               (13) /* DAC HW version */
#define BVDC_P_SUPPORT_SCL_VER                (11) /* SCL HW version */
#define BVDC_P_SUPPORT_HSCL_VER               (7) /* HSCL HW version */
#define BVDC_P_SUPPORT_IT_VER                 (3) /* IT HW version */
#define BVDC_P_SUPPORT_MADR_VER               (10) /* MAD-R HW version */
#define BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP      (0)
#define BVDC_P_SUPPORT_CAP_VER                (7) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_CLOCK_GATING           (0) /* Clock gating for power saving */
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (0)  /*Dynamic Memory Power Gating*/

/* BVN */
#define BVDC_P_SUPPORT_PEP                    (1) /* Number of PEP HW, see CMP_x */
#define BVDC_P_SUPPORT_PEP_VER                (5) /* PEP HW version */
#define BVDC_P_SUPPORT_HIST                   (1) /* Number of HIST HW */
#define BVDC_P_SUPPORT_TNT                    (1) /* Number of TNT HW */
#define BVDC_P_SUPPORT_TNT_VER                (6) /* TNT HW version */
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (1) /* Number of MASK HW */
#define BVDC_P_SUPPORT_BOX_DETECT             (1) /* Number LBOX HW detect */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (3)
#define BVDC_P_SUPPORT_CAP                    (4) /* Number of CAP HW */
#define BVDC_P_SUPPORT_VFD                    (4) /* Number of VFD HW */
#define BVDC_P_SUPPORT_SCL                    (4) /* Number of SCL HW */
#define BVDC_P_SUPPORT_XSRC_VER               (0) /* XSRC HW version */
#define BVDC_P_SUPPORT_XSRC                   (0) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_DMISC                  (1) /* TODO: phase out */
#define BVDC_P_SUPPORT_FREE_CHANNEL           (5) /* Number of VNET_F_FCH_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (4) /* DRAIN HW version */
#define BVDC_P_SUPPORT_MCVP                   (1) /* TODO: Number of MCVP HW */
#define BVDC_P_SUPPORT_MCVP_VER               (6) /* TODO: MCVP HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (0) /* TODO: MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (1) /* TODO: Number of MAD-R HW */
#define BVDC_P_SUPPORT_MANR                   (0) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (0) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (4) /* Testfeature HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2) /* Number of CMP_0_Vx */
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_0_Gx */
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (2) /* Number of CMP_1_Vx */
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_1_Gx */
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_2_Vx */
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_2_Gx */
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (5)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (5) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_0_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (16)/* Number of clear rect support CMP_1_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V0_RECT_ENABLE_MASK */
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0) /* Number of clear rect support CMP_2_V1_RECT_ENABLE_MASK */
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1) /* TODO: const color */
#define BVDC_P_MIN_XCODE_CMP                  (0)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (2)
#define BVDC_P_SUPPORT_COLOR_CLIP             (0)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_HDMI_RM_VER            (7)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (0)

#define BVDC_P_MAX_DACS                       (1) /* Number of DAC output */
#define BVDC_P_ORTHOGONAL_VEC_VER             (2)
#define BVDC_P_SUPPORT_HD_DAC                 (1) /* TODO: phase out? */
#define BVDC_P_SUPPORT_STG_VER                (0)
#define BVDC_P_SUPPORT_DTG_RMD                (0)
#define BVDC_P_SUPPORT_DSCL                   (1)
#define BVDC_P_SUPPORT_DSCL_VER               (3)

#define BVDC_P_NUM_SHARED_656                 (1) /* Number VEC 656 HW */
#define BVDC_P_NUM_SHARED_DVI                 (1) /* Number VEC Digital output HW */
#define BVDC_P_NUM_SHARED_STG                 (2) /* Number of STG HW */
#define BVDC_P_NUM_SHARED_RF                  (1)
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_NUM_SHARED_VF                  (1) /* Number VEC's VF HW, VEC_CFG_VF_x_SOURCE */
#define BVDC_P_NUM_SHARED_SDSRC               (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_HDSRC               (0) /* Number VEC's VF HW, VEC_CFG_HDSRC_x_SOURCE */
#define BVDC_P_NUM_SHARED_SM                  (1) /* Number VEC's VF HW, VEC_CFG_SDSRC_x_SOURCE */
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1) /* Number of VEC's SECAM HW, VEC_CFG_SECAM_x_SOURCE */
#define BVDC_P_NUM_SHARED_SECAM_HD            (0) /* Number of VEC's SECAM Passthru HW */
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_STG                    (0) /* STG HW */
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_324_SYSCLK)

#elif (BCHP_CHIP==7271) || (BCHP_CHIP==7268) || (BCHP_CHIP==7260)

#if(BCHP_CHIP==7260)
#define BVDC_P_SUPPORT_XSRC                   (1) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_MADR                   (1) /* Number of MAD-R HW */
#define BVDC_P_SUPPORT_MCVP                   (1) /* Number of MCVP HW */
#define BVDC_P_SUPPORT_LOOP_BACK              (4) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_VEC_GRPD               (1)
#define BVDC_P_SUPPORT_HDDVI                  (0)
#define BVDC_P_SUPPORT_DSCL                   (1)
#define BVDC_P_SUPPORT_DSCL_VER               (3)
#else
#if BCHP_VER >= BCHP_VER_B0
#define BVDC_P_SUPPORT_XSRC                   (3) /* Number of stand alone XSRC HW */
#else
#define BVDC_P_SUPPORT_XSRC                   (2) /* Number of stand alone XSRC HW */
#endif
#define BVDC_P_SUPPORT_MADR                   (2) /* Number of MAD-R HW */
#define BVDC_P_SUPPORT_MCVP                   (2) /* Number of MCVP HW */
#define BVDC_P_SUPPORT_LOOP_BACK              (6) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_VEC_GRPD               (0)
#define BVDC_P_SUPPORT_HDDVI                  (1)
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)
#endif

#define BVDC_P_SUPPORT_XSRC_VER               (2) /* XSRC HW version */
#define BVDC_P_SUPPORT_BOX_DETECT             (0) /* Number LBOX HW detect, see SW7366-151 */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (3)
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (8) /* Testfeature HW version */
#define BVDC_P_SUPPORT_MANR_VER               (5) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_CAP_VER                (8) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_HDMI_RM_VER            (7)
#define BVDC_P_SUPPORT_MFD                    (2)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_MFD_VER                (17)/* MFD HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (0) /* MCDI HW version */
#define BVDC_P_SUPPORT_MADR_VER               (11)/* MAD-R HW version */
#define BVDC_P_SUPPORT_MCVP_VER               (6) /* MCVP HW version */
#define BVDC_P_SUPPORT_PEP_VER                (6)
#define BVDC_P_SUPPORT_SCL_VER                (11)/* SCL HW version */
#define BVDC_P_SUPPORT_DNR                    (2) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_DRAIN_F                (2) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (2) /* DRAIN HW version */
#define BVDC_P_SUPPORT_MTG                    (2) /* MFD Trigger Generator */
#define BVDC_P_MAX_DACS                       (1)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (1)
#define BVDC_P_SUPPORT_656_MASTER_MODE        (0)
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0)

/* source */
#define BVDC_P_SUPPORT_GFD                    (2)
#define BVDC_P_SUPPORT_GFD_VER                (9)
#define BVDC_P_SUPPORT_GFD1_VER               (9)
#define BVDC_P_SUPPORT_TDAC_VER               (13)/* TDAC/QDAC HW version */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)
#define BVDC_P_SUPPORT_HDDVI_VER              (12)

/* BVN */
#define BVDC_P_SUPPORT_TNT                    (1)
#define BVDC_P_SUPPORT_TNT_VER                (6)
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#if BCHP_VER >= BCHP_VER_B0
#define BVDC_P_SUPPORT_MASK_DITHER            (0)
#define BVDC_P_SUPPORT_PEP                    (0)
#define BVDC_P_SUPPORT_HIST                   (0)
#define BVDC_P_SUPPORT_VFC                    (1) /* Number of stand alone VFC HW */
#else
#define BVDC_P_SUPPORT_MASK_DITHER            (1)
#define BVDC_P_SUPPORT_PEP                    (1)
#define BVDC_P_SUPPORT_HIST                   (1)
#define BVDC_P_SUPPORT_VFC                    (0) /* Number of stand alone VFC HW */
#endif
#define BVDC_P_SUPPORT_CAP                    (2)
#define BVDC_P_SUPPORT_VFD                    (2)
#define BVDC_P_SUPPORT_SCL                    (2)
#define BVDC_P_SUPPORT_HSCL_VER               (7) /* HSCL HW version */
#define BVDC_P_SUPPORT_DMISC                  (1)
#define BVDC_P_SUPPORT_FREE_CHANNEL           (4) /* Number of VNET_F_FCH_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_NUM_SHARED_STG                 (1)
#define BVDC_P_ORTHOGONAL_VEC_VER             (2)
#define BVDC_P_SUPPORT_IT_VER                 (3)
#define BVDC_P_SUPPORT_MANR                   (0) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (0)
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (0)
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */

#if BCHP_VER >= BCHP_VER_B0
#define BVDC_P_CMP_CFC_VER                    (3)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (8)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (4)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (6) /* cmps other than cmp 0 */
#else
#define BVDC_P_CMP_CFC_VER                    (2)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (6)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (6)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (6) /* cmps other than cmp 0 */
#endif
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (0)
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1)
#define BVDC_P_MIN_XCODE_CMP                  (2)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (4)
#define BVDC_P_SUPPORT_COLOR_CLIP             (1)

/* display */
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (0)

#define BVDC_P_SUPPORT_HD_DAC                 (0)
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_NUM_SHARED_656                 (1)
#define BVDC_P_NUM_SHARED_DVI                 (1)
#define BVDC_P_NUM_SHARED_RF                  (0)
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_NUM_SHARED_VF                  (1)
#define BVDC_P_NUM_SHARED_HDSRC               (0)
#define BVDC_P_NUM_SHARED_SDSRC               (1)
#define BVDC_P_NUM_SHARED_SM                  (1)
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1)     /* Number of VEC's SECAM HW, SECAM_x_register*/
#define BVDC_P_NUM_SHARED_SECAM_HD            (0)     /* Number of VEC's SECAM Passthru HW VEC_CFG_SECAM_x_SOURCE - BVDC_P_NUM_SHARED_SECAM*/
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_CLOCK_GATING           (1)
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (1)  /*Dynamic Memory Power Gating*/
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_324_SYSCLK)
#define BVDC_P_SUPPORT_STG                    (1) /* STG HW */
#define BVDC_P_SUPPORT_STG_VER                (4)
#define BVDC_P_MANAGE_VIP                     (1) /* VDC owns VIP */
#define BVDC_P_SUPPORT_RDC_STC_FLAG           (1) /* RDC STC flags */

#elif (BCHP_CHIP==7278)

/* source */
#define BVDC_P_SUPPORT_GFD                    (4)
#define BVDC_P_SUPPORT_GFD_VER                (9)
#define BVDC_P_SUPPORT_GFD1_VER               (9)
#define BVDC_P_SUPPORT_XSRC                   (3) /* Number of stand alone XSRC HW */
#define BVDC_P_SUPPORT_XSRC_VER               (2) /* XSRC HW version */
#define BVDC_P_SUPPORT_VFC                    (3) /* Number of stand alone VFC HW */
#define BVDC_P_SUPPORT_HDDVI                  (1)
#define BVDC_P_SUPPORT_HDDVI_VER              (12)
#define BVDC_P_SUPPORT_BOX_DETECT             (1) /* Number LBOX HW detect, see SW7366-151 */
#define BVDC_P_SUPPORT_BOX_DETECT_VER         (3)
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER (8) /* Testfeature HW version */
#define BVDC_P_SUPPORT_LPDDR4                 (1)
#define BVDC_P_SUPPORT_MHL                    (0)
#define BVDC_P_SUPPORT_MFD                    (4)
#define BVDC_P_SUPPORT_MFD_VER                (17)/* MFD HW version */
#define BVDC_P_SUPPORT_DNR                    (4) /* Number of DNR_x core */
#define BVDC_P_SUPPORT_MTG                    (2) /* MFD Trigger Generator */
#define BVDC_P_SUPPORT_NEW_656_IN_VER         (0)

/* BVN */
#define BVDC_P_SUPPORT_LOOP_BACK              (15) /* Number of VNET_B_LOOP_BACK_x_SRC */
#define BVDC_P_SUPPORT_MCVP                   (4) /* Number of MCVP HW */
#define BVDC_P_SUPPORT_MCVP_VER               (6) /* MCVP HW version */
#define BVDC_P_SUPPORT_MCDI_VER               (9) /* MCDI HW version */
#define BVDC_P_SUPPORT_MADR                   (3) /* Number of MAD-R HW */
#define BVDC_P_SUPPORT_MADR_VER               (10)/* MAD-R HW version */
#define BVDC_P_SUPPORT_MANR                   (1) /* Number of ANR HW in MCVP */
#define BVDC_P_SUPPORT_MANR_VER               (5) /* ANR in MCVP version */
#define BVDC_P_SUPPORT_TNT                    (2)
#define BVDC_P_SUPPORT_TNT_VER                (6)
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_TNTD_VER               (0)
#define BVDC_P_SUPPORT_MASK_DITHER            (0)
#define BVDC_P_SUPPORT_PEP                    (0)
#define BVDC_P_SUPPORT_PEP_VER                (6)
#define BVDC_P_SUPPORT_HIST                   (0)
#define BVDC_P_SUPPORT_CAP                    (4)
#define BVDC_P_SUPPORT_CAP_VER                (8) /* CAP HW version, see bvdc_capture_priv.h */
#define BVDC_P_SUPPORT_VFD                    (4)
#define BVDC_P_SUPPORT_SCL                    (4)
#define BVDC_P_SUPPORT_SCL_VER                (11)/* SCL HW version */
#define BVDC_P_SUPPORT_HSCL_VER               (7) /* HSCL HW version */
#define BVDC_P_SUPPORT_DMISC                  (1)
#define BVDC_P_SUPPORT_FREE_CHANNEL           (5) /* Number of VNET_F_FCH_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_B                (1) /* Number of VNET_B_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_F                (2) /* Number of VNET_F_DRAIN_x_SRC */
#define BVDC_P_SUPPORT_DRAIN_VER              (2) /* DRAIN HW version */
#define BVDC_P_SUPPORT_XCODE_WIN_CAP          (0)

/* CMP */
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (2)
#define BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (1)
#define BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT     (1)
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (1) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT     (1) /* Number of CMP_3_Gx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_4_Gx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_5_Gx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */
#define BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT     (0) /* Number of CMP_6_Gx */
#define BVDC_P_CMP_CFC_VER                    (3)
#define BVDC_P_CMP_0_MOSAIC_CFCS              (6)
#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (6) /* cmps other than cmp 0 */
#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (0)
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (16)
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)
#define BVDC_P_SUPPORT_WIN_CONST_COLOR        (1)
#define BVDC_P_MIN_XCODE_CMP                  (2)

/* csc */
#define BVDC_P_SUPPORT_CMP_DEMO_MODE          (2)
#define BVDC_P_SUPPORT_CSC_MAT_COEF_VER       (4)
#define BVDC_P_SUPPORT_COLOR_CLIP             (1)

/* display */
#define BVDC_P_ORTHOGONAL_VEC_VER             (2)
#define BVDC_P_SUPPORT_IT_VER                 (3)
#define BVDC_P_MAX_DACS                       (1)
#define BVDC_P_SUPPORT_4kx2k_60HZ             (1)
#define BVDC_P_SUPPORT_VBI_ENC_656            (1)
#define BVDC_P_SUPPORT_VEC_VF_VER             (2)
#define BVDC_P_SUPPORT_DVI_OUT                (1)
#define BVDC_P_SUPPORT_HDMI_RM_VER            (7)
#define BVDC_P_SUPPORT_ITU656_OUT             (1)
#define BVDC_P_SUPPORT_656_MASTER_MODE        (0)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (0)
#define BVDC_P_SUPPORT_VEC_GRPD               (0)
#define BVDC_P_SUPPORT_DSCL                   (0)
#define BVDC_P_SUPPORT_DSCL_VER               (0)
#define BVDC_P_SUPPORT_TDAC_VER               (13)/* TDAC/QDAC HW version */
#define BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND (0)

#define BVDC_P_SUPPORT_HD_DAC                 (0)
#define BVDC_P_SUPPORT_DTG_RMD                (1)
#define BVDC_P_NUM_SHARED_STG                 (2)
#define BVDC_P_NUM_SHARED_656                 (1)
#define BVDC_P_NUM_SHARED_DVI                 (1)
#define BVDC_P_NUM_SHARED_RF                  (0)
#define BVDC_P_NUM_SHARED_IT                  (2)
#define BVDC_P_NUM_SHARED_VF                  (1)
#define BVDC_P_NUM_SHARED_HDSRC               (0)
#define BVDC_P_NUM_SHARED_SDSRC               (1)
#define BVDC_P_NUM_SHARED_SM                  (1)
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#define BVDC_P_NUM_SHARED_SECAM               (1)     /* Number of VEC's SECAM HW, SECAM_x_register*/
#define BVDC_P_NUM_SHARED_SECAM_HD            (0)     /* Number of VEC's SECAM Passthru HW VEC_CFG_SECAM_x_SOURCE - BVDC_P_NUM_SHARED_SECAM*/
#define BVDC_P_NUM_SHARED_DAC                 BVDC_P_MAX_DACS
#define BVDC_P_SUPPORT_CLOCK_GATING           (1)
#define BVDC_P_SUPPORT_MEM_PWR_GATING         (1)  /*Dynamic Memory Power Gating*/
#define BVDC_P_BVB_BUS_CLOCK                  (BVDC_P_324_SYSCLK)
#define BVDC_P_SUPPORT_STG                    (2) /* two STGs are supported */
#define BVDC_P_SUPPORT_STG_VER                (4)
#define BVDC_P_MANAGE_VIP                     (0) /* TODO: VDC owns VIP */
#define BVDC_P_SUPPORT_RDC_STC_FLAG           (1) /* RDC STC flags */

#else
    #error "Port required for VDC."
#endif

/* Checking assumes memory is continous, possible problem when there is
 * hole in memory address. Disable the checking till we have correction
 * information from BCHP. */
#define BVDC_P_CHECK_MEMC_INDEX               (0)

#if BVDC_P_MANAGE_VIP || BVDC_SUPPORT_VIP_DEBUG
#define BVDC_P_SUPPORT_VIP                    (BVDC_P_SUPPORT_STG) /* VIP HW */
#else
#define BVDC_P_SUPPORT_VIP                    (0)
#endif

#define BVDC_P_NUM_SHARED_MPAA                (1)

#define BVDC_P_SUPPORT_CMP_V0_CLEAR_RECT      (BVDC_P_CMP_0_V0_CLEAR_RECTS || BVDC_P_CMP_1_V0_CLEAR_RECTS || BVDC_P_CMP_2_V0_CLEAR_RECTS ||\
    BVDC_P_CMP_3_V0_CLEAR_RECTS || BVDC_P_CMP_4_V0_CLEAR_RECTS || BVDC_P_CMP_5_V0_CLEAR_RECTS || BVDC_P_CMP_6_V0_CLEAR_RECTS)
#define BVDC_P_SUPPORT_CMP_V1_CLEAR_RECT      (BVDC_P_CMP_0_V1_CLEAR_RECTS || BVDC_P_CMP_1_V1_CLEAR_RECTS)
#define BVDC_P_SUPPORT_CMP_CLEAR_RECT         \
    (BVDC_P_SUPPORT_CMP_V0_CLEAR_RECT || BVDC_P_SUPPORT_CMP_V1_CLEAR_RECT)

#if defined(BVDC_FOR_BOOTUPDATER)
#define BVDC_P_CMP_CFCS                       1
#elif (BVDC_P_CMP_0_MOSAIC_CFCS>0)
#define BVDC_P_CMP_CFCS                       BVDC_P_CMP_0_MOSAIC_CFCS
#else
#define BVDC_P_CMP_CFCS                       2 /* primary and demo */
#endif

/* cause reconfig vdet after input signal loss and re-lock */
#ifndef BVDC_P_CLEANUP_VNET
#define BVDC_P_CLEANUP_VNET                   (1)
#endif

#define BVDC_P_AUTO_ENABLE_CAPTURE            (1)    /* auto: turn on capture if needed */
#define BVDC_P_OPTIMIZE_MEM_USAGE             (1)    /* auto: best fit allocation */
#define BVDC_P_MAD_SRC_HORZ_THRESHOLD         (1920) /* auto: trigger MAD's hscl threshold */

/* Number of Video+Gfx Windows configuration */
#if defined(BVDC_FOR_BOOTUPDATER) ||\
    (BCHP_CHIP==7358) || (BCHP_CHIP==7552) || (BCHP_CHIP==7360)   || \
    (BCHP_CHIP==7563) || (BCHP_CHIP==7543) || (BCHP_CHIP==7362)   || \
    (BCHP_CHIP==7228) || (BCHP_CHIP==75635) || (BCHP_CHIP==73625)
#define BVDC_P_WIN_GET_REG_OFFSET(window_id) \
    ((BVDC_P_WindowId_eComp0_G0==(window_id)) ? (BCHP_CMP_0_G0_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp1_G0==(window_id)) ? (BCHP_CMP_1_G0_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BCHP_CMP_0_V0_SURFACE_SIZE - BCHP_CMP_0_REVISION))

#elif (BCHP_CHIP==7364) || (BCHP_CHIP==7250) || (BCHP_CHIP==7271) || \
      (BCHP_CHIP==7268) || (BCHP_CHIP==7260)
#define BVDC_P_WIN_GET_REG_OFFSET(window_id) \
    ((BVDC_P_WindowId_eComp0_V1==(window_id)) ? (BCHP_CMP_0_V1_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp0_G0==(window_id)) ? (BCHP_CMP_0_G0_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp1_G0==(window_id)) ? (BCHP_CMP_1_G0_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BCHP_CMP_0_V0_SURFACE_SIZE - BCHP_CMP_0_REVISION))

#elif (BCHP_CHIP==7231)  || (BCHP_CHIP==7346)  || \
      (BCHP_CHIP==7344) || (BCHP_CHIP==7429)  || (BCHP_CHIP==7584)  || \
      (BCHP_CHIP==7586) || (BCHP_CHIP==75845) || (BCHP_CHIP==74295) || \
      (BCHP_CHIP==73465)
#define BVDC_P_WIN_GET_REG_OFFSET(window_id) \
    ((BVDC_P_WindowId_eComp0_V1==(window_id)) ? (BCHP_CMP_0_V1_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp0_G0==(window_id)) ? (BCHP_CMP_0_G0_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp1_V1==(window_id)) ? (BCHP_CMP_1_V1_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp1_G0==(window_id)) ? (BCHP_CMP_1_G0_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BCHP_CMP_0_V0_SURFACE_SIZE - BCHP_CMP_0_REVISION))

#elif ((BCHP_CHIP==7439) && (BCHP_VER>=BCHP_VER_B0))
#define BVDC_P_WIN_GET_REG_OFFSET(window_id) \
    ((BVDC_P_WindowId_eComp0_V1==(window_id)) ? (BCHP_CMP_0_V1_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp0_G0==(window_id)) ? (BCHP_CMP_0_G0_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp1_V1==(window_id)) ? (BCHP_CMP_1_V1_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp1_G0==(window_id)) ? (BCHP_CMP_1_G0_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp2_G0==(window_id)) ? (BCHP_CMP_2_G0_SURFACE_SIZE - BCHP_CMP_2_REVISION) \
    :(BVDC_P_WindowId_eComp3_G0==(window_id)) ? (BCHP_CMP_3_G0_SURFACE_SIZE - BCHP_CMP_3_REVISION) \
    :(BCHP_CMP_0_V0_SURFACE_SIZE - BCHP_CMP_0_REVISION))

#elif (BCHP_CHIP==7366) || \
      ((BCHP_CHIP==7439)  && (BCHP_VER==BCHP_VER_A0)) || \
      ((BCHP_CHIP==74371) && (BCHP_VER==BCHP_VER_A0))
#define BVDC_P_WIN_GET_REG_OFFSET(window_id) \
    ((BVDC_P_WindowId_eComp0_V1==(window_id)) ? (BCHP_CMP_0_V1_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp0_G0==(window_id)) ? (BCHP_CMP_0_G0_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp1_V1==(window_id)) ? (BCHP_CMP_1_V1_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp1_G0==(window_id)) ? (BCHP_CMP_1_G0_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp2_G0==(window_id)) ? (BCHP_CMP_2_G0_SURFACE_SIZE - BCHP_CMP_2_REVISION) \
    :(BCHP_CMP_0_V0_SURFACE_SIZE - BCHP_CMP_0_REVISION))

#elif (BCHP_CHIP==7422) || (BCHP_CHIP==7425)
#define BVDC_P_WIN_GET_REG_OFFSET(window_id) \
    ((BVDC_P_WindowId_eComp0_V1==(window_id)) ? (BCHP_CMP_0_V1_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp0_G0==(window_id)) ? (BCHP_CMP_0_G0_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp1_V1==(window_id)) ? (BCHP_CMP_1_V1_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp1_G0==(window_id)) ? (BCHP_CMP_1_G0_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp2_G0==(window_id)) ? (BCHP_CMP_2_G0_SURFACE_SIZE - BCHP_CMP_2_REVISION) \
    :(BVDC_P_WindowId_eComp3_G0==(window_id)) ? (BCHP_CMP_3_G0_SURFACE_SIZE - BCHP_CMP_3_REVISION) \
    :(BCHP_CMP_0_V0_SURFACE_SIZE - BCHP_CMP_0_REVISION))
#elif (BCHP_CHIP==7435)
#define BVDC_P_WIN_GET_REG_OFFSET(window_id) \
    ((BVDC_P_WindowId_eComp0_V1==(window_id)) ? (BCHP_CMP_0_V1_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp0_G0==(window_id)) ? (BCHP_CMP_0_G0_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp1_V1==(window_id)) ? (BCHP_CMP_1_V1_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp1_G0==(window_id)) ? (BCHP_CMP_1_G0_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp2_G0==(window_id)) ? (BCHP_CMP_2_G0_SURFACE_SIZE - BCHP_CMP_2_REVISION) \
    :(BVDC_P_WindowId_eComp3_G0==(window_id)) ? (BCHP_CMP_3_G0_SURFACE_SIZE - BCHP_CMP_3_REVISION) \
    :(BVDC_P_WindowId_eComp4_G0==(window_id)) ? (BCHP_CMP_4_G0_SURFACE_SIZE - BCHP_CMP_4_REVISION) \
    :(BVDC_P_WindowId_eComp5_G0==(window_id)) ? (BCHP_CMP_5_G0_SURFACE_SIZE - BCHP_CMP_5_REVISION) \
    :(BCHP_CMP_0_V0_SURFACE_SIZE - BCHP_CMP_0_REVISION))
#elif (BCHP_CHIP==7445) || (BCHP_CHIP==11360)
#define BVDC_P_WIN_GET_REG_OFFSET(window_id) \
    ((BVDC_P_WindowId_eComp0_V1==(window_id)) ? (BCHP_CMP_0_V1_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp0_G0==(window_id)) ? (BCHP_CMP_0_G0_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp1_V1==(window_id)) ? (BCHP_CMP_1_V1_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp1_G0==(window_id)) ? (BCHP_CMP_1_G0_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp2_G0==(window_id)) ? (BCHP_CMP_2_G0_SURFACE_SIZE - BCHP_CMP_2_REVISION) \
    :(BVDC_P_WindowId_eComp3_G0==(window_id)) ? (BCHP_CMP_3_G0_SURFACE_SIZE - BCHP_CMP_3_REVISION) \
    :(BVDC_P_WindowId_eComp4_G0==(window_id)) ? (BCHP_CMP_4_G0_SURFACE_SIZE - BCHP_CMP_4_REVISION) \
    :(BVDC_P_WindowId_eComp5_G0==(window_id)) ? (BCHP_CMP_5_G0_SURFACE_SIZE - BCHP_CMP_5_REVISION) \
    :(BVDC_P_WindowId_eComp6_G0==(window_id)) ? (BCHP_CMP_6_G0_SURFACE_SIZE - BCHP_CMP_6_REVISION) \
    :(BCHP_CMP_0_V0_SURFACE_SIZE - BCHP_CMP_0_REVISION))
#elif (BCHP_CHIP==7278)
#define BVDC_P_WIN_GET_REG_OFFSET(window_id) \
    ((BVDC_P_WindowId_eComp0_V1==(window_id)) ? (BCHP_CMP_0_V1_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp0_G0==(window_id)) ? (BCHP_CMP_0_G0_SURFACE_SIZE - BCHP_CMP_0_REVISION) \
    :(BVDC_P_WindowId_eComp1_G0==(window_id)) ? (BCHP_CMP_1_G0_SURFACE_SIZE - BCHP_CMP_1_REVISION) \
    :(BVDC_P_WindowId_eComp2_G0==(window_id)) ? (BCHP_CMP_2_G0_SURFACE_SIZE - BCHP_CMP_2_REVISION) \
    :(BVDC_P_WindowId_eComp3_G0==(window_id)) ? (BCHP_CMP_3_G0_SURFACE_SIZE - BCHP_CMP_3_REVISION) \
    :(BCHP_CMP_0_V0_SURFACE_SIZE - BCHP_CMP_0_REVISION))

#else
#error "Unknown chip!  Not yet supported in VDC."
#endif

/* Derive! */
#define BVDC_P_SUPPORT_SEC_CMP                (0 != BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT)
#define BVDC_P_SUPPORT_TER_CMP                (0 != BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT)

#define BVDC_P_SUPPORT_SEC_VEC                BVDC_P_SUPPORT_SEC_CMP
#define BVDC_P_SUPPORT_TER_VEC                BVDC_P_SUPPORT_TER_CMP
#define BVDC_P_SUPPORT_PRM_VEC_CMPN_ONLY      ((!BVDC_P_SUPPORT_SEC_VEC) && (BVDC_P_MAX_DACS > 4))
#define BVDC_P_SUPPORT_SEC_VEC_CMPN_ONLY      (BVDC_P_SUPPORT_SEC_VEC)
#define BVDC_P_SUPPORT_COMPONENT_ONLY         (BVDC_P_SUPPORT_PRM_VEC_CMPN_ONLY || BVDC_P_SUPPORT_SEC_VEC_CMPN_ONLY)

/* TODO: clarify this definition */
#define BVDC_P_SUPPORT_MOSAIC_MODE            BVDC_P_SUPPORT_CMP_CLEAR_RECT
#if ((BVDC_P_SUPPORT_MCDI_VER >= 8)  || (BVDC_P_SUPPORT_MADR_VER >= 10))
#define BVDC_P_SUPPORT_MOSAIC_DEINTERLACE     (1)
#else
#define BVDC_P_SUPPORT_MOSAIC_DEINTERLACE     (0)
#endif

/* Common private defines */
#define BVDC_P_MAX_COMPOSITOR_COUNT           (7)
#define BVDC_P_MAX_DISPLAY_COUNT              BVDC_P_MAX_COMPOSITOR_COUNT
#define BVDC_P_MAX_SOURCE_COUNT               (28) /* includes VFD0, VFD1, VFD2, VFD3, and VFD4 */
#define BVDC_P_MAX_GFX_WINDOWS                \
    (BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT + \
     BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT + \
     BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT + \
     BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT + \
     BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT + \
     BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT + \
     BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT)

/* Miscellaneous macros */
#define BVDC_P_EVEN_PIXEL                     (2)   /* even pixel alignment */
#define BVDC_P_PITCH_ALIGN                    (32)  /* 32-byte alignment */
#define BVDC_P_BUFFER_ALIGN                   (4)   /* 4-byte alignment */
#define BVDC_P_FIELD_PER_FRAME                (2)   /* Tf / Bf */
#define BVDC_P_MAX_POLARITY                   (3)   /* TF/BF/Frame */

/* default 8 bit 422, since this is the only format that the 3D module supports. */
#define BVDC_P_CAP_PIXEL_FORMAT_8BIT422       (BPXL_eY18_Cb8_Y08_Cr8)
/* default 8 bit 422 on BE system, since this is the only format that the 3D module supports. */
#define BVDC_P_CAP_PIXEL_FORMAT_8BIT422_BE    (BPXL_eCr8_Y08_Cb8_Y18)
/* default 10 bit 444 */
#define BVDC_P_CAP_PIXEL_FORMAT_10BIT444      (BPXL_eX2_Cr10_Y10_Cb10)
/* default 10 bit 422 */
#define BVDC_P_CAP_PIXEL_FORMAT_10BIT422      \
    (BPXL_eX2_Y010_Cb10_Y110_X2_Cr10_Y010_Cb10_X2_Y110_Cr10_Y010_X2_Cb10_Y110_Cr10)

/* delay VDEC/656's BVB output by some fields to avoid garbage data: */
/* for non-component interface */
#define BVDEC_P_BVB_OUT_SLOW_START_COUNT      (20)
/* for component interface with vert frequency equal or above 50 */
#define BVDEC_P_BVB_OUT_SLOW_START_COUNT_CMP  (5)
/* for component interface with vert frequency less than 50 */
#define BVDEC_P_BVB_OUT_SLOW_START_COUNT_CMP_LOW_FREQ   (18)

/* Auto repeat trigger */
#define BVDC_P_AUTO_TRIGGER_VSYNC_DELAY       (5)   /* in vsync(s) */


/*****************************************************************************
   SCRATCH registers usage statements:
   ----------------------------------

   The VDC currently uses a lot of scratch registers to communicate between
   register DMA and the CPU. For example, to detect input video lock status
   for the VDEC as well as timestamps for multi-buffering etc.

   It turns out that there are some severe restrictions on which scratch
   registers can be used by the VDC. Specifically, if the block containing the
   register can be reset, we could get a bus error. Please see PR 22437 for
   more details.

   To help remove the possibility of running into this case again, we need one
   central location in the VDC where scratch registers are assigned.

   NOTE: Scratch regsiters from blocks that can be reset should avoid!!!
   ---------------------------------------------------------------------------

   We used the following scratch registers for s/w workaround or debug purpose:

    LBOX_0: one scratch for mpeg0 dual debug purposes; (used up)
    LBOX_1: one scratch for mpeg1 dual debug purposes; (used up)
    GFD_0: both scratches for VBI encoder control workaround; (used up)
    GFD_1: one scratch for VBI encoder control workaround; (one left)
    CMP_0: for nop RUL workaround; (used up)
    HSCL_0: one scratch for tuning the HSCL_TUNE_THRESHOLD
    BMISC:
        BMISC_SCRATCH_0[00:00] - activate rectangle size debug

    So we have the following BVN scratch registers unused:

    VFD_x: two;
    CAP_x: two;
    PEP_CMP_0_V0: one
    MMISC: one
    VNET_F: one;
    VNET_B: one;
    FMISC: one;
    BMISC: one;
    MFD_x: two;
    CMP_x: one for each block;
    MAD_x: one for each;
    GFD_1: one left;
    GFD_2: two;
    SCL_?: two for each;
    MASK_x: one;
    DNR_0: one;
 */
/* Theses are for VBI encoder program synchronization; defined in bavc.h;
#define BAVC_VBI_ENC_0_CTRL_SCRATCH           (BCHP_GFD_0_SCRATCH0)
#define BAVC_VBI_ENC_1_CTRL_SCRATCH           (BCHP_GFD_1_SCRATCH0)
#define BAVC_VBI_ENC_2_CTRL_SCRATCH           (BCHP_GFD_0_SCRATCH1)
 */

/* A scratch register for write only to.  Reading from this register
 * yield unknown result. This is to workaround a RDC NOP bug.
 * It's no harm to have a scratch register here coming from a resetable block
 * since software(CPU) would not access this register. */
#define BVDC_P_SCRATCH_WO_REG                 (BCHP_CMP_0_SCRATCH_REGISTER)

/* These are for mpeg data ready callback debug purpose */
#if BVDC_P_SUPPORT_BOX_DETECT > 0
#include "bchp_lbox_0.h"
#endif

#if BVDC_P_SUPPORT_BOX_DETECT >= 2
#define BVDC_P_MPEG_DEBUG_SCRATCH(id) \
    (((id) == BAVC_SourceId_eMpeg0) ? BCHP_LBOX_0_SCRATCH : \
    ((BCHP_LBOX_1_REG_START - BCHP_LBOX_0_REG_START) * id + BCHP_LBOX_0_SCRATCH))
#elif BVDC_P_SUPPORT_BOX_DETECT == 1
#define BVDC_P_MPEG_DEBUG_SCRATCH(id) (BCHP_LBOX_0_SCRATCH)
#else
#define BVDC_P_MPEG_DEBUG_SCRATCH(id) (BCHP_FMISC_SCRATCH_0)
#endif

/*****************************************************************/

/* All Compostior windows */
#define BVDC_P_CMP_0_MAX_WINDOW_COUNT \
    (BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT + BVDC_P_CMP_0_MAX_GFX_WINDOW_COUNT)
#define BVDC_P_CMP_1_MAX_WINDOW_COUNT \
    (BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT + BVDC_P_CMP_1_MAX_GFX_WINDOW_COUNT)
#define BVDC_P_CMP_2_MAX_WINDOW_COUNT \
    (BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT + BVDC_P_CMP_2_MAX_GFX_WINDOW_COUNT)
#define BVDC_P_CMP_3_MAX_WINDOW_COUNT \
    (BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT + BVDC_P_CMP_3_MAX_GFX_WINDOW_COUNT)
#define BVDC_P_CMP_4_MAX_WINDOW_COUNT \
    (BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT + BVDC_P_CMP_4_MAX_GFX_WINDOW_COUNT)
#define BVDC_P_CMP_5_MAX_WINDOW_COUNT \
    (BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT + BVDC_P_CMP_5_MAX_GFX_WINDOW_COUNT)
#define BVDC_P_CMP_6_MAX_WINDOW_COUNT \
    (BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT + BVDC_P_CMP_6_MAX_GFX_WINDOW_COUNT)
#define BVDC_P_MAX_WINDOW_COUNT              (18)

#define BVDC_P_MAX_VIDEO_WINS_PER_CMP         (2)

/* For scaler */
#define BVDC_P_NRM_SRC_STEP_F_BITS            (19)/* assume max pic size in 12-bit to normalize to 32-bit fixed point math */
#define BVDC_P_OVER_CENTER_WIDTH              (0xffff)

/* Define lip sync delay */
#define BVDC_P_MICROSECOND_PER_FIELD          (1000000/60)
#define BVDC_P_LIP_SYNC_RESET_DELAY           UINT32_C(-1)
#define BVDC_P_LIP_SYNC_VEC_DELAY             (1)
#define BVDC_P_LIP_SYNC_CAP_PLK_DELAY         (1)
#define BVDC_P_LIP_SYNC_CAP_PLK_LOCK_DELAY    (1)
#define BVDC_P_LIP_SYNC_CAP_PLK_SLIP_DELAY    (2)
#define BVDC_P_LIP_SYNC_DEINTERLACED_DELAY    (3)
#define BVDC_P_LIP_SYNC_TOLERANCE             (BVDC_P_MICROSECOND_PER_FIELD)

/* Multi-buffer count.  Field buffers use for capturing and playback. */
#define BVDC_P_BYPASS_MULTI_BUFFER_COUNT      (0)
#define BVDC_P_SYNC_LOCK_MULTI_BUFFER_COUNT   (2)
#define BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT   (4)
#define BVDC_P_MAX_USER_CAPTURE_BUFFER_COUNT (25)
#define BVDC_P_MAX_MULTI_BUFFER_COUNT      \
    (BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT + \
     BVDC_P_LIP_SYNC_VEC_DELAY + \
     BVDC_P_LIP_SYNC_DEINTERLACED_DELAY + \
     BVDC_P_MAX_USER_CAPTURE_BUFFER_COUNT)

/* Vsync delay */
#define BVDC_P_NO_CAPTURE_VSYNC_DELAY         (0)
#define BVDC_P_FIELD_VSYNC_DELAY              (1)
#define BVDC_P_FRAME_VSYNC_DELAY              (2)

/* Max number of RULs */
#define BVDC_P_MAX_ENTRY_PER_MPEG_RUL         (0x4000) /* dword */
#define BVDC_P_MAX_ENTRY_PER_RUL              (0xC00) /* dword */
#define BVDC_P_MAX_VEC_APPLY_WAIT_TIMEOUT     (2000)  /* ms */
#define BVDC_P_MAX_APPLY_WAIT_TIMEOUT         (2000)  /* ms */
#define BVDC_P_MAX_DESTROY_WAIT_TIMEOUT       (2000)  /* ms */
#define BVDC_P_TRIGGER_LOST_THRESHOLD         (20)    /* vsync */

/* For timestamp use */
#define BVDC_P_CLOCK_RATE                     (27ul)  /* 27Mhz */
#define BVDC_P_TIMER_WRAP                     (1<<30) /* timer wraps at this
                                                         count (is 39.7 seconds) */
#define BVDC_P_MAX_TIMER_VALUE                (BVDC_P_TIMER_WRAP-1)  /* the max timer count is one
                                                                        less than the wrap point */

#ifdef BCHP_RDC_desc_0_tm_snapshot
#define BVDC_P_USE_RDC_TIMESTAMP              (1)
#else
#define BVDC_P_USE_RDC_TIMESTAMP              (0)
#endif

#if (BVDC_P_USE_RDC_TIMESTAMP)
/* RDC timestamp is taken as soon as slot fires. This uses RDC timer. */
#define BVDC_P_MULTIBUFFER_RW_TOLERANCE       5
#else
/* Non-RDC timestamp is taken as part of the RUL. This uses BTMR. */
/* (BVDC_P_MAX_ENTRY_PER_RUL/64)*. RDC_Blockout is 6.4 for 108MHz, 3.2 for 216MHz. Use 6.4 for worst case. */
#define BVDC_P_MULTIBUFFER_RW_TOLERANCE       (BVDC_P_MAX_ENTRY_PER_RUL/10)
#endif

/* 16th of a pixel unit */
#define BVDC_P_16TH_PIXEL_SHIFT               (4)
#define BVDC_P_16TH_PIXEL_MASK                (0xF)

/* Multi-buffer count for RUL */
#define BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT     (2)

/* mosaic slave RUL is triple-buffered: double-buffer (isr build) + one in-flight */
#define BVDC_P_MAX_MULTI_SLAVE_RUL_BUFFER_COUNT (3)

#define BVDC_P_NEXT_RUL_IDX(cur_idx) \
    (((cur_idx) + 1) % BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT)

/* Number of time to write full RUL */
#define BVDC_P_RUL_UPDATE_THRESHOLD           (2)

/* Black YCrCb   Use for background and fixed color. */
#define BVDC_P_YCRCB_BLACK                    (0x108080)

#define BVDC_P_TEST_SQUARE_PXL                (0)

#define BVDC_P_MAX_3DCOMB_SD_BUFFER_COUNT     (8)

/* MAD32 field stores count and format */
#define BVDC_P_MAD_QM_BITS_PER_PIXEL          (2)

/* compile option to decide how many field stores to allocate for MAD */
#ifdef BVDC_MAD_NOT_SUPPORT_TRICK_MODE
    #define BVDC_P_MAD_PIXEL_SD_BUFFER_COUNT  (5)
#else
    #define BVDC_P_MAD_PIXEL_SD_BUFFER_COUNT  (5)
#endif

#if (BVDC_P_SUPPORT_MCDI_VER>2)
#define BVDC_P_MAD_QM_FIELD_STORE_COUNT       (4)  /* MADR */
#define BVDC_P_MCDI_QM_FIELD_STORE_COUNT      (8)  /* 7425 MCVP */
#else
#define BVDC_P_MAD_QM_FIELD_STORE_COUNT       (4)  /* MADR */
#define BVDC_P_MCDI_QM_FIELD_STORE_COUNT      (4)  /* 7420 MCVP */
#endif

#if (BVDC_P_SUPPORT_MCDI_VER>2)
#define BVDC_P_MCDI_QM_BUFFER_COUNT        (2)  /* 7425 MCVP */
#else
#define BVDC_P_MCDI_QM_BUFFER_COUNT        (0)  /* 7420 MCVP */
#endif
#define BVDC_P_MAD_QM_BUFFER_COUNT         (1)  /* MAD/MADR */

#if (BVDC_P_SUPPORT_MCDI_VER>2)
#define BVDC_P_MAX_MAD_SD_BUFFER_COUNT \
    (BVDC_P_MAD_PIXEL_SD_BUFFER_COUNT + BVDC_P_MCDI_QM_BUFFER_COUNT)
#else
#define BVDC_P_MAX_MAD_SD_BUFFER_COUNT \
    (BVDC_P_MAD_PIXEL_SD_BUFFER_COUNT + BVDC_P_MAD_QM_BUFFER_COUNT)
#endif

#define BVDC_P_MAX_MCDI_BUFFER_COUNT          (4)

/* TODO: Modify later. */
#define BVDC_P_MAX_4HD_BUFFER_COUNT           (0)
#define BVDC_P_MAX_2HD_BUFFER_COUNT           (0)

#define BVDC_P_MAX_HD_BUFFER_COUNT \
    (BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT * BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT)

#define BVDC_P_MAX_SD_BUFFER_COUNT \
    (BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT * BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT + \
      BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT * BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT + \
      BVDC_P_MAX_MAD_SD_BUFFER_COUNT + BVDC_P_MAX_3DCOMB_SD_BUFFER_COUNT)

#define BVDC_P_DEFAULT_4HD_PIP_BUFFER_COUNT   (0)
#define BVDC_P_DEFAULT_2HD_PIP_BUFFER_COUNT   (0)
#define BVDC_P_DEFAULT_HD_PIP_BUFFER_COUNT    (0)
#define BVDC_P_DEFAULT_SD_PIP_BUFFER_COUNT    (0)

/* BVB System clk!  Use to figure out vertical refresh rate! */
/* Used by 656 and HD_DVI in new trigger mode */
#define BVDC_P_108_SYSCLK                     (108000000) /* 108 Mhz */

/* BVB System clk!  Use to figure out vertical refresh rate! */
/* Used by VDEC in new trigger mode */
#define BVDC_P_216_SYSCLK                     (216000000) /* 216 Mhz */

/* BVB System clk!  Use to figure out vertical refresh rate! */
/* Used by VDEC in new trigger mode */
#define BVDC_P_324_SYSCLK                     (324000000) /* 324 Mhz */

#define BVDC_P_648_SYSCLK                     (648000000) /* 648 Mhz */

/* Use for video TestFeature1 */
#define BVDC_P_72_SYSCLK                      (72000000)  /* 72 Mhz */

/* Total amount of pixels for 4k30 */
#define BVDC_P_4K30_PIXEL_COUNT               (3840*2160*30)

/* Trigger offset!  Where to trigger after end of field. */
#define BVDC_P_TRIGGER_OFFSET(max_vbi_lines)  ((max_vbi_lines) * 65 / 100)

/* Amount of line delay after EOP */
#define BVDC_P_TRIGGER_DELAY                  (12)

#define BVDC_P_ASPR_MAX                       (50)


/* CAP/VFD burst size for bandwidth equation.
 * BURST_SIZE_IN_BYTE = (jword * 256bits/16bits) */
#ifdef BCHP_VFD_0_HW_CONFIGURATION_MAX_BURST_SIZE_DEFAULT
#if (BCHP_VFD_0_HW_CONFIGURATION_MAX_BURST_SIZE_DEFAULT == BCHP_VFD_0_HW_CONFIGURATION_MAX_BURST_SIZE_BURST_8)
#define BVDC_P_SCB_BURST_SIZE                 (128)
#elif (BCHP_VFD_0_HW_CONFIGURATION_MAX_BURST_SIZE_DEFAULT == BCHP_VFD_0_HW_CONFIGURATION_MAX_BURST_SIZE_BURST_16)
#define BVDC_P_SCB_BURST_SIZE                 (256)
#elif (BCHP_VFD_0_HW_CONFIGURATION_MAX_BURST_SIZE_DEFAULT == BCHP_VFD_0_HW_CONFIGURATION_MAX_BURST_SIZE_BURST_24)
#define BVDC_P_SCB_BURST_SIZE                 (384)
#elif (BCHP_VFD_0_HW_CONFIGURATION_MAX_BURST_SIZE_DEFAULT == BCHP_VFD_0_HW_CONFIGURATION_MAX_BURST_SIZE_BURST_32)
#define BVDC_P_SCB_BURST_SIZE                 (512)
#else
#error "Unknown chip!  Required porting for CAP/VFD burst size!"
#endif
#else /* pre HW configuration knowledge assumes 256 */
#define BVDC_P_SCB_BURST_SIZE                 (256)
#endif

/*
DCX macro
*/
#define BVDC_36BITS_PER_GROUP                 (36)
#define BVDC_37BITS_PER_GROUP                 (37)
#define BVDC_38BITS_PER_GROUP                 (38)
#define BVDC_40BITS_PER_GROUP                 (40)
#define BVDC_44BITS_PER_GROUP                 (44)
#define BVDC_45BITS_PER_GROUP                 (45)
#define BVDC_48BITS_PER_GROUP                 (48)
#define BVDC_53BITS_PER_GROUP                 (53)
#define BVDC_DCX_PIXEL_PER_GROUP              (4)

#define BVDC_P_MADR_DCXS_COMPRESSION(ulBitsPerGroup) \
    (ulBitsPerGroup > BVDC_36BITS_PER_GROUP)?44:36

#define BVDC_P_DCXM_BITS_PER_PIXEL                     (10)

/* guard memory for 12 mosaic rectangles
 *      (2N -1)*32, (N-1)*32?
 */
#define BVDC_P_CAP_GUARD_MEMORY_2D              (736)
#define BVDC_P_CAP_GUARD_MEMORY_3D              (352)
/* guard memory for 16 mosaic rectangles */
#define BVDC_P_MAX_CAP_GUARD_MEMORY_2D          (992)
#define BVDC_P_MAX_CAP_GUARD_MEMORY_3D          (480)

/***************************************************************************
 * Macros
***************************************************************************/
/* utility macro: to calculate how many entries from "start" register
   to "end" register inclusively. */
#define BVDC_P_REGS_ENTRIES(start, end)    \
    ((((BCHP##_##end) - (BCHP##_##start)) / sizeof(uint32_t)) + 1)

/* define for bit-field */
#define BVDC_P_ON                             (1)
#define BVDC_P_OFF                            (0)

/* define that use dirty for building RUL. */
#define BVDC_P_DIRTY                          (1)
#define BVDC_P_CLEAN                          (0)
#define BVDC_P_ALL_DIRTY_BYTE                 (0xFF)

/* Dirty bit macros.  Uses unions.  Treats bitfields as 32-bit int arrays for speed. */
/* Currently handles up to 32 dirty bits.  Modify macros below to handle more. */
/* Note: Remember to update debug messages using pDirty->aulInts[x] when changing array size */
#define BVDC_P_DIRTY_INT_ARRAY_SIZE           (1)  /* Size of dirty bit int array representation. 1=32-bits, 2=64-bits, etc.  */
#define BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE   (sizeof(uint32_t) * 8) /* Size of dirty bit int array element in bits*/
#define BVDC_P_CLEAN_ALL_DIRTY(pDirty)        (BKNI_Memset((*pDirty).aulInts, 0, sizeof((*pDirty).aulInts)))

#define BVDC_P_SET_ALL_DIRTY(pDirty)          (BKNI_Memset((*pDirty).aulInts, BVDC_P_ALL_DIRTY_BYTE, sizeof((*pDirty).aulInts)))
#define BVDC_P_OR_ALL_DIRTY(pDst, pSrc)       ((*pDst).aulInts[0] |= (*pSrc).aulInts[0])
/* Check if dirty bit is clean or dirty */
#define BVDC_P_IS_DIRTY(pDirty)               ((*pDirty).aulInts[0])
#define BVDC_P_IS_CLEAN(pDirty)               (!BVDC_P_IS_DIRTY(pDirty))
#define BVDC_P_NUM_DIRTY_BITS(pDirty)         (sizeof((*pDirty).stBits) * 8)
#define BVDC_P_DIRTY_COMPARE(pDirtyA, pDirtyB)                         \
    (BKNI_Memcmp(&pDirtyA->aulInts[0], &pDirtyB->aulInts[0], sizeof(uint32_t)*BVDC_P_DIRTY_INT_ARRAY_SIZE))

/* Callback dirty bit macros. */
#define BVDC_P_CB_IS_DIRTY(pDirty)            (BVDC_P_CbIsDirty(pDirty, sizeof(*pDirty)))
#define BVDC_P_CB_IS_DIRTY_isr(pDirty)        (BVDC_P_CbIsDirty_isr(pDirty, sizeof(*pDirty)))
#define BVDC_P_CB_CLEAN_ALL_DIRTY(pDirty)     (BKNI_Memset(pDirty, 0, sizeof(*pDirty)))

/* Check for field delta. */
#define BVDC_P_FIELD_DIFF(currstruct, prevstruct, field) \
    ((currstruct)->field != (prevstruct)->field)

/* Print debug instruction. */
#define BVDC_P_PRINT_BUF_DEBUG_INSTRUCTION()   \
    BDBG_ERR(("============================================================")); \
    BDBG_ERR(("Re-run with export msg_modules=BVDC_MEMCONFIG,BVDC_WIN_BUF"));   \
    BDBG_ERR(("and include msg_modules log in the bug report"));                \
    BDBG_ERR(("============================================================"))

/* Print a rect struct. */
#define BVDC_P_PRINT_RECT(name, pRect, bForcePrint, shift, mask) \
    if ((pRect != NULL) && (bForcePrint)) {\
        BDBG_ERR(("%-8s (x, y, w, h): (%d+%d/16, %d+%d/16, %4d, %4d) - (%d+%d/16)", \
                (name), (pRect)->lLeft >> shift, (pRect)->lLeft & mask, (pRect)->lTop >> shift, \
                (pRect)->lTop & 0xf, (pRect)->ulWidth, (pRect)->ulHeight,  \
                (pRect)->lLeft_R >> shift, (pRect)->lLeft_R & mask)); \
    } else if (pRect != NULL) \
        BDBG_MSG(("%-8s (x, y, w, h): (%d+%d/16, %d+%d/16, %4d, %4d) - (%d+%d/16)", \
                (name), (pRect)->lLeft >> shift, (pRect)->lLeft & mask, (pRect)->lTop >> shift, \
                (pRect)->lTop & 0xf, (pRect)->ulWidth, (pRect)->ulHeight,  \
                (pRect)->lLeft_R >> shift, (pRect)->lLeft_R & mask))

#define BVDC_P_PRINT_CLIP(name, pRect, bForcePrint) \
    if ((pRect != NULL) && (bForcePrint)) {\
        BDBG_ERR(("%-8s (x, y, w, h): (%4d, %4d, %4d, %4d) - (%4d)", (name), \
                (pRect)->ulLeft, (pRect)->ulRight, (pRect)->ulTop, (pRect)->ulBottom, \
                (pRect)->lLeftDelta_R)); \
    } else if (pRect != NULL) \
        BDBG_MSG(("%-8s (x, y, w, h): (%4d, %4d, %4d, %4d) - (%4d)", (name), \
                (pRect)->ulLeft, (pRect)->ulRight, (pRect)->ulTop, (pRect)->ulBottom, \
                (pRect)->lLeftDelta_R))

#define BVDC_P_PRINT_SCLCUT_RECT(name, pRect, bForcePrint) \
    if ((pRect != NULL) && (bForcePrint)) {\
        BDBG_ERR(("%-8s (x, y, w, h): (%d+%d/64, %d+%d/16384, %4d, %4d) - (%d+%d/64)", \
            (name), (pRect)->lLeft >> BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS, (pRect)->lLeft & BVDC_P_SCL_LEFT_PIC_OFFSET_F_MASK, \
            (pRect)->lTop >> BVDC_P_SCL_TOP_PIC_OFFSET_F_BITS, (pRect)->lTop & BVDC_P_SCL_TOP_PIC_OFFSET_F_MASK, \
            (pRect)->ulWidth, (pRect)->ulHeight,  \
            (pRect)->lLeft_R >> BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS, (pRect)->lLeft_R & BVDC_P_SCL_LEFT_PIC_OFFSET_F_MASK)); \
    } else if (pRect != NULL) \
        BDBG_MSG(("%-8s (x, y, w, h): (%d+%d/64, %d+%d/16384, %4d, %4d) - (%d+%d/64)", \
            (name), (pRect)->lLeft >> BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS, (pRect)->lLeft & BVDC_P_SCL_LEFT_PIC_OFFSET_F_MASK, \
            (pRect)->lTop >> BVDC_P_SCL_TOP_PIC_OFFSET_F_BITS, (pRect)->lTop & BVDC_P_SCL_TOP_PIC_OFFSET_F_MASK, \
            (pRect)->ulWidth, (pRect)->ulHeight,  \
            (pRect)->lLeft_R >> BVDC_P_SCL_LEFT_PIC_OFFSET_F_BITS, (pRect)->lLeft_R & BVDC_P_SCL_LEFT_PIC_OFFSET_F_MASK))

/* Take absolute value of a number */
#define BVDC_P_ABS(value) (((value)<0) ? (-(value)) : (value))

#define BVDC_P_IS_UNKNOWN_ASPR(asprMode, sarX, sarY)                      \
    ((BFMT_AspectRatio_eUnknown == asprMode) ||                               \
       ((BFMT_AspectRatio_eSAR == asprMode) &&                                \
        (((0 == sarX) || (0 == sarY)) || ((sarX / sarY) > BVDC_P_ASPR_MAX) || \
        ((sarY / sarX) > BVDC_P_ASPR_MAX))))

/* Turn a float to fixed-point */
#define BVDC_P_FLOAT_TO_FIXED(fvalue, i_bit, f_bit) \
    (((uint32_t)(BVDC_P_ABS(fvalue) * (1 << (f_bit)))) | \
    (((fvalue)<0) << ((i_bit) + (f_bit))))

/* Make a point base on numerator, denominator, and fraction bits */
#define BVDC_P_MAKE_FIXED_POINT(num, denom, f_bits) \
    (((num) * (1 << (f_bits))) / (denom))

/* Turn the fixed into two's complements, use int op might cause overflow */
#define BVDC_P_FIXED_A_MINUS_FIXED_B(fixed_a, fixed_b) \
    (((fixed_a) > (fixed_b)) ? ((fixed_a) - (fixed_b)) \
    : (~((fixed_b) - (fixed_a)) + 1))

#define BVDC_P_VALID_PIXEL_FORMAT(ePixelFormat)       \
    ( BPXL_IS_YCbCr422_FORMAT(ePixelFormat))

/* ---------------------------------------------
 * TestFeature1 revision
 * --------------------------------------------- */
/* 7125 AxBx:
 *  No TestFeature1 support. */
#define BVDC_P_VIDEO_TESTFEATURE1_VER_0       (0)

/* Obsolete chipsets:
 * 3548 B0B1, 3556 B0B1:
 *  Support TestFeature1 in CAP, MAD and GFD.
 *  Software workaround needed:
 *      BVDC_P_DCX_CAP_OVERFLOW_WORKAROUND
 *      BVDC_P_DCX_CAP_RESET_WORKAROUND
 *      BVDC_P_DCX_VFD_RESET_WORKAROUND
 *      BVDC_P_DCX_HSIZE_WORKAROUND
 */
#define BVDC_P_VIDEO_TESTFEATURE1_VER_1       (1)

/* Obsolete chipsets:
 * 3548 B2 and above, 3556 B2 and above:
 *  Support TestFeature1 in CAP, MAD and GFD.
 *  Capture FIFO overflow issue is fixed
 *  Software workaround needed:
 *      BVDC_P_DCX_CAP_RESET_WORKAROUND
 *      BVDC_P_DCX_VFD_RESET_WORKAROUND
 *      BVDC_P_DCX_HSIZE_WORKAROUND
 */
#define BVDC_P_VIDEO_TESTFEATURE1_VER_2       (2)

/* 7125 Cx and above:
 *  Support TestFeature1 in MAD and ANR, no TestFeature1
 *  support in CAP or GFD.
 */
#define BVDC_P_VIDEO_TESTFEATURE1_VER_3       (3)

/* 7422, 7425, 7358, 7552, 7231, 7346, 7344:
 *  Support TestFeature1 in MAD and ANR (MCVP and MADR),
 *  no TestFeature1 support in CAP or GFD.
 * ANR DCD and DCE cross over issue is fixed.
 */
#define BVDC_P_VIDEO_TESTFEATURE1_VER_4       (4)

/* 7445 Dx:
 *  Support TestFeature1 in deinterlacer, CAP and VFD
 */
#define BVDC_P_VIDEO_TESTFEATURE1_VER_5       (5)

/* 7366 Bx:
 *  Support TestFeature1 in deinterlacer, CAP and VFD
 *  Fixed BVDC_P_DCXM_RECT_WORKAROUND and BVDC_P_DCXM_VARIABLE_RATE_WORKAROUND
 */
#define BVDC_P_VIDEO_TESTFEATURE1_VER_6       (6)

/* 7439 Bx:
 *  Support TestFeature1 in deinterlacer, CAP and VFD
 *  Fixed BVDC_P_DCX_3D_WORKAROUND
 */
#define BVDC_P_VIDEO_TESTFEATURE1_VER_7       (7)

/* 7271 Ax:
 *  Support TestFeature1 in deinterlacer, CAP and VFD
 *  Fixed SW workaround for SW7364-197 enable free run for dcxm chips
 */
#define BVDC_P_VIDEO_TESTFEATURE1_VER_8       (8)

/* TestFeature1 support in each block */
#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_MAD_ANR   \
     (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER > BVDC_P_VIDEO_TESTFEATURE1_VER_0)

#define BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_CAP_DCXM   \
    (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER >= BVDC_P_VIDEO_TESTFEATURE1_VER_5)

#define BVDC_P_DCX_ANR_CROSS_OVER_WORKAROUND        \
     (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER < BVDC_P_VIDEO_TESTFEATURE1_VER_4)

#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER == BVDC_P_VIDEO_TESTFEATURE1_VER_5) || \
    (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER == BVDC_P_VIDEO_TESTFEATURE1_VER_6)
#define BVDC_P_DCX_3D_WORKAROUND               (1)
#endif

/* CRBVN-282: Both offset and width need to be multiple of 4 */
#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER == BVDC_P_VIDEO_TESTFEATURE1_VER_5)
#define BVDC_P_DCXM_RECT_WORKAROUND            (1)
#endif

/* CRBVN-486: Mosiac compression mode does not support H size smaller than 28 */
#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER == BVDC_P_VIDEO_TESTFEATURE1_VER_5)
#define BVDC_P_DCXM_MIN_HSIZE_WORKAROUND       (1)
#endif

/* Variable rate for DCXM not working for mosaic */
#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER == BVDC_P_VIDEO_TESTFEATURE1_VER_5)
#define BVDC_P_DCXM_VARIABLE_RATE_WORKAROUND   (1)
#endif

    /* SW7364-197 enable free run for dcxm chips*/
#if ((BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER >= BVDC_P_VIDEO_TESTFEATURE1_VER_5) && \
     (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER <= BVDC_P_VIDEO_TESTFEATURE1_VER_7))
#define BVDC_P_SUPPORT_CLOCK_GATING_FMISC_FR   (1)
/*SW7445-2936*/
#define BVDC_P_DCXM_CAP_PADDING_WORKAROUND           (1) /* 1 line padding */
#define BVDC_P_DCXM_BUFFERHEAP_INCREASE_WORKAROUND   (BVDC_P_DCXM_CAP_PADDING_WORKAROUND*2) /* for top and bottom fields */
#else
#define BVDC_P_SUPPORT_CLOCK_GATING_FMISC_FR         (0)
#define BVDC_P_DCXM_CAP_PADDING_WORKAROUND           (0)
#define BVDC_P_DCXM_BUFFERHEAP_INCREASE_WORKAROUND   (0)
#endif

/*SW7439-400:SWSTB-2319: 72 pixel restriction */
#if (BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER >= BVDC_P_VIDEO_TESTFEATURE1_VER_8)
#define BVDC_P_DCXM_72PIXELS   (0)
#else
#define BVDC_P_DCXM_72PIXELS   (1)
#endif
/* This needs to match SIOB_0_DCXS_CFG.FIXED_RATE for MADR */
#define BVDC_P_MADR_VARIABLE_RATE              (0)

#if ((BVDC_P_SUPPORT_MADR) && (BVDC_P_SUPPORT_MADR_VER < 5))
/* HW7425-820: HSIZE needs to be multiple of 4.*/
#define BVDC_P_MADR_HSIZE_WORKAROUND           (1)
#endif

#if ((BVDC_P_SUPPORT_MADR) && (BVDC_P_SUPPORT_MADR_VER < 4))
/* HW7425-869: SIOB QM capture doesn't end data transmission
 * correctly when (picture_width/4)*(picture_height+1)%40 > 31 */
#define BVDC_P_MADR_PICSIZE_WORKAROUND         (1)
#endif
/* SW7439-5 MCVP is used in xcode path */
#if ( BVDC_P_SUPPORT_STG  && \
    ((BVDC_P_SUPPORT_MADR && (BVDC_P_SUPPORT_MADR_VER < 6)) || \
    (BVDC_P_SUPPORT_MCVP && (BVDC_P_SUPPORT_MCVP_VER < 5))))
/* HW7425-1244/1255 MADR scb cycle incomplete */
#define BVDC_P_STG_RUL_DELAY_WORKAROUND        (1)
#endif

#if (BVDC_P_SUPPORT_STG && (BVDC_P_SUPPORT_STG_VER < 3))
/* HW7425-1429 STG repeat polarity trigger dependent on STC */
#define BVDC_P_STG_NRT_CADENCE_WORKAROUND        (1)
#endif
#if ((BVDC_P_SUPPORT_MFD_VER >= 12) && \
     (BVDC_P_SUPPORT_MFD_VER <= 13))
/* HW7425-1404: BVN MVFD: YC unpack flow control issue */
#define BVDC_P_MVFD_ALIGNMENT_WORKAROUND       (1)
#endif

#if (BVDC_P_MADR_PICSIZE_WORKAROUND)
#define BVDC_P_MADR_GET_REMAINDER(ulWidth, ulHeight)  \
    ( ((ulWidth*(ulHeight+1))/4) % 40 )
#define BVDC_P_MADR_BAD_ALIGNMENT(ulRemainder)   \
    ( ulRemainder > 31 )

#if (BVDC_P_MADR_VARIABLE_RATE)
#define BVDC_P_MADR_GET_VARIABLE_RATE_REMAINDER(ulWidth, ulHeight, bpp)   \
    ( (ulWidth*ulHeight*bpp + 128) % 2048 )

#define BVDC_P_MADR_VARIABLE_RATE_BAD_ALIGNMENT(ulRemainder, bpp)   \
    ( !((ulRemainder==0) || (ulRemainder > (128+bpp))) )

#else
#define BVDC_P_MADR_GET_FIX_RATE_REMAINDER(ulWidth, ulHeight, bpg, ppg)   \
    ( (BVDC_P_DIV_ROUND_UP((ulWidth*ulHeight*bpg), ppg)) % 2048 )

#define BVDC_P_MADR_FIX_RATE_BAD_ALIGNMENT(ulRemainder, bpg, ppg)    \
    ( !((ulRemainder==0) || ((ulRemainder*ppg) > bpg)) )

#endif

#endif

/* HW7420-976 or SW7420-2017 workaround */
/* SCL rev less than to 0.3.0.5 */
#if ((BVDC_P_SUPPORT_SCL_VER < 9))
#define BVDC_P_SCL_V_STEP_SIZE_WORKAROUND      (1)
#else
#define BVDC_P_SCL_V_STEP_SIZE_WORKAROUND      (0)
#endif
#define BVDC_P_NUM_MOSAIC_CSC_TYPE             (6)


/***************************************************************************
 * VDC Internal enums
 ***************************************************************************/
/* capture trigger type */
typedef enum
{
    BVDC_P_CapTriggerType_eDisable = 0,
    BVDC_P_CapTriggerType_eBvbField,
    BVDC_P_CapTriggerType_eLineCompare
} BVDC_P_CapTriggerType;

/* source chroma type for MAD chroma config */
typedef enum
{
    BVDC_P_ChromaType_eChroma422 = 0,
    BVDC_P_ChromaType_eField420,
    BVDC_P_ChromaType_eFrame420,
    BVDC_P_ChromaType_eAuto = 7
} BVDC_P_ChromaType;

/* internal mvp compression technology */
typedef enum
{
    BVDC_P_Mvp_DcxNoComp = 0,
    BVDC_P_Mvp_Dcx,           /* 7425/35 Mcdi */
    BVDC_P_Mvp_Dcxs,          /* Madr */
    BVDC_P_Mvp_Dcxs2,         /* 7445 D0 Mcdi */
    BVDC_P_Mvp_DcxUnknown
} BVDC_P_MvpDcxCore;

/* Example modes:
 *
 * (A) [src] -> [...] -> [   ] -> [   ] -> [dst]  no capture nor scaler.
 * (B) [src] -> [scl] -> [   ] -> [   ] -> [dst]  scaler_only or bypass.
 * (C) [src] -> [cap] -> [plk] -> [scl] -> [dst]  capture_before_scaler.
 * (D) [src] -> [scl] -> [cap] -> [plk] -> [dst]  scaler_before_capture.
 * (E) [src] -> [mad] -> [   ] -> [   ] -> [dst]  mad_only.
 * (F) [src] -> [cap] -> [plk] -> [mad] -> [dst]  capture_before_mad.
 * (G) [src] -> [cap] -> [plk] -> [   ] -> [dst]  capture_only.
 *
 * (A) Where we have the source go directly to a compositor.  This mode
 * is mainly for debugging purposes only. This is realized when all vnet
 * mode bits are 0.
 *
 * Currently we have the following conventions:
 * (1). MAD is always before SCL
 * (2)  DNR is always right after SRC
 * (3). ANR is always inside MCVP / MADR
 */
typedef union
{
    struct
    {
        /* sub-module usage bits */
        uint32_t  bUseCap                : 1;  /* bit 0 */
        uint32_t  bUseScl                : 1;
        uint32_t  bUseXsrc               : 1;
        uint32_t  bUseTntd               : 1;
        uint32_t  bUseDnr                : 1;  /* bit 4 */
        uint32_t  bUseMvp                : 1;
        uint32_t  bUseVnetCrc            : 1;

        /* sub-modules order bits */
        uint32_t  bSclBeforeCap          : 1;
        uint32_t  bTntdBeforeScl         : 1;  /* bit 8 */
        uint32_t  bVnetCrcBeforeCap      : 1;
        uint32_t  bUseVfc                : 1;

        /* bit indicate invalid vnet */
        uint32_t  bInvalid               : 1;
    } stBits;

    uint32_t aulInts[BVDC_P_DIRTY_INT_ARRAY_SIZE];
} BVDC_P_VnetMode;

typedef union
{
    struct
    {
        /* sub-module usage bits */
        uint32_t  bUseMvp                : 1;  /* bit 0 */
        uint32_t  bUseMvpBypass          : 1;
        uint32_t  bUseHscl               : 1;
        uint32_t  bUseMad                : 1;
        uint32_t  bUseAnr                : 1;  /* bit 4 */
    } stBits;

    uint32_t aulInts[BVDC_P_DIRTY_INT_ARRAY_SIZE];
} BVDC_P_MvpMode;

/* Uses to index some of the internal table, care must be taken when changing
 * the enum. */
typedef enum
{
    /* Video Window on compositor 0 (Primary) */
    BVDC_P_WindowId_eComp0_V0 = 0,
    BVDC_P_WindowId_eComp0_V1,

    /* Video Window on compositor 1 (Secondary) */
    BVDC_P_WindowId_eComp1_V0,
    BVDC_P_WindowId_eComp1_V1,

    /* Video Window Compositor Bypass */
    BVDC_P_WindowId_eComp2_V0,

    /* Vice */
    BVDC_P_WindowId_eComp3_V0,
    BVDC_P_WindowId_eComp4_V0,
    BVDC_P_WindowId_eComp5_V0,
    BVDC_P_WindowId_eComp6_V0,

    /* Gfx Window on compositor 0 (Primary) */
    BVDC_P_WindowId_eComp0_G0,
    BVDC_P_WindowId_eComp0_G1,
    BVDC_P_WindowId_eComp0_G2,

    /* Gfx Window on compositor 1 (Secondary) */
    BVDC_P_WindowId_eComp1_G0,

    /* Gfx Window on compositor 2 (Tertiary) */
    BVDC_P_WindowId_eComp2_G0,

    /* Gfx Window on compositor 3 (Vice) */
    BVDC_P_WindowId_eComp3_G0,
    BVDC_P_WindowId_eComp4_G0,
    BVDC_P_WindowId_eComp5_G0,
    BVDC_P_WindowId_eComp6_G0,

    /* Must be last */
    BVDC_P_WindowId_eUnknown
} BVDC_P_WindowId;

typedef enum
{
    BVDC_P_FeederId_eMfd0 = 0,
    BVDC_P_FeederId_eMfd1,
    BVDC_P_FeederId_eMfd2,
    BVDC_P_FeederId_eMfd3,
    BVDC_P_FeederId_eMfd4,
    BVDC_P_FeederId_eMfd5,
    BVDC_P_FeederId_eVfd0,
    BVDC_P_FeederId_eVfd1,
    BVDC_P_FeederId_eVfd2,
    BVDC_P_FeederId_eVfd3,
    BVDC_P_FeederId_eVfd4,
    BVDC_P_FeederId_eVfd5,
    BVDC_P_FeederId_eVfd6,
    BVDC_P_FeederId_eVfd7,
    BVDC_P_FeederId_eUnknown
} BVDC_P_FeederId;

typedef enum
{
    BVDC_P_CaptureId_eCap0 = 0,
    BVDC_P_CaptureId_eCap1,
    BVDC_P_CaptureId_eCap2,
    BVDC_P_CaptureId_eCap3,
    BVDC_P_CaptureId_eCap4,
    BVDC_P_CaptureId_eCap5,
    BVDC_P_CaptureId_eCap6,
    BVDC_P_CaptureId_eCap7,
    BVDC_P_CaptureId_eUnknown
} BVDC_P_CaptureId;

typedef enum
{
    BVDC_P_ScalerId_eScl0 = 0,
    BVDC_P_ScalerId_eScl1,
    BVDC_P_ScalerId_eScl2,
    BVDC_P_ScalerId_eScl3,
    BVDC_P_ScalerId_eScl4,
    BVDC_P_ScalerId_eScl5,
    BVDC_P_ScalerId_eScl6,
    BVDC_P_ScalerId_eScl7,
    BVDC_P_ScalerId_eUnknown
} BVDC_P_ScalerId;

typedef enum
{
    BVDC_P_MadId_eMad0,
    BVDC_P_MadId_eMad1,
    BVDC_P_MadId_eUnknown
} BVDC_P_MadId;

typedef enum
{
    BVDC_P_HscalerId_eHscl0 = 0,
    BVDC_P_HscalerId_eHscl1 ,
    BVDC_P_HscalerId_eHscl2 ,
    BVDC_P_HscalerId_eHscl3 ,
    BVDC_P_HscalerId_eHscl4 ,
    BVDC_P_HscalerId_eHscl5 ,
    BVDC_P_HscalerId_eUnknown
} BVDC_P_HscalerId;

typedef enum
{
    BVDC_P_XsrcId_eXsrc0 = 0,
    BVDC_P_XsrcId_eXsrc1 ,
    BVDC_P_XsrcId_eXsrc2 ,
    BVDC_P_XsrcId_eUnknown
} BVDC_P_XsrcId;

typedef enum
{
    BVDC_P_VfcId_eVfc0 = 0,
    BVDC_P_VfcId_eVfc1 ,
    BVDC_P_VfcId_eVfc2 ,
    BVDC_P_VfcId_eUnknown
} BVDC_P_VfcId;

typedef enum
{
    BVDC_P_TntdId_eTntd0 = 0,
    BVDC_P_TntdId_eUnknown
} BVDC_P_TntdId;

typedef enum
{
    BVDC_P_DnrId_eDnr0 = 0,
    BVDC_P_DnrId_eDnr1,
    BVDC_P_DnrId_eDnr2,
    BVDC_P_DnrId_eDnr3,
    BVDC_P_DnrId_eDnr4,
    BVDC_P_DnrId_eDnr5,
    BVDC_P_DnrId_eUnknown
} BVDC_P_DnrId;

typedef enum
{
    BVDC_P_AnrId_eAnr0 = 0,
    BVDC_P_AnrId_eAnr1,
    BVDC_P_AnrId_eAnr2,
    BVDC_P_AnrId_eAnr3,
    BVDC_P_AnrId_eAnr4,
    BVDC_P_AnrId_eAnr5,
    BVDC_P_AnrId_eUnknown
} BVDC_P_AnrId;

typedef enum
{
    BVDC_P_VnetCrcId_eVnetCrc0 = 0,
    BVDC_P_VnetCrcId_eUnknown
} BVDC_P_VnetCrcId;

typedef enum
{
    BVDC_P_McdiId_eMcdi0 = 0,
    BVDC_P_McdiId_eMcdi1,
    BVDC_P_McdiId_eMcdi2,
    BVDC_P_McdiId_eMcdi3,
    BVDC_P_McdiId_eMcdi4,
    BVDC_P_McdiId_eMcdi5,
    BVDC_P_McdiId_eUnknown
} BVDC_P_McdiId;

typedef enum
{
    BVDC_P_MvpId_eMvp0 = 0,
    BVDC_P_MvpId_eMvp1,
    BVDC_P_MvpId_eMvp2,
    BVDC_P_MvpId_eMvp3,
    BVDC_P_MvpId_eMvp4,
    BVDC_P_MvpId_eMvp5,
    BVDC_P_MvpId_eUnknown
} BVDC_P_McvpId;

typedef enum
{
    BVDC_P_BoxDetectId_eBoxDetect0,
    BVDC_P_BoxDetectId_eBoxDetect1,
    BVDC_P_BoxDetectId_eBoxDetect2,
    BVDC_P_BoxDetectId_eBoxDetect3,
    BVDC_P_BoxDetectId_eBoxDetect4,
    BVDC_P_BoxDetectId_eBoxDetect5,
    BVDC_P_BoxDetectId_eUnknown
} BVDC_P_BoxDetectId;

typedef enum
{
    BVDC_P_FreeChId_eCh0 = 0,
    BVDC_P_FreeChId_eCh1,
    BVDC_P_FreeChId_eCh2,
    BVDC_P_FreeChId_eCh3,
    BVDC_P_FreeChId_eCh4,
    BVDC_P_FreeChId_eCh5,
    BVDC_P_FreeChId_eCh6,
    BVDC_P_FreeChId_eCh7,
    BVDC_P_FreeChId_eCh8,
    BVDC_P_FreeChId_eCh9,
    BVDC_P_FreeChId_eCh10,
    BVDC_P_FreeChId_eCh11,
    BVDC_P_FreeChId_eCh12,
    BVDC_P_FreeChId_eCh13,
    BVDC_P_FreeChId_eUnknown
} BVDC_P_FreeChId;

typedef enum
{
    BVDC_P_LpBckId_eLp0 = 0,
    BVDC_P_LpBckId_eLp1,
    BVDC_P_LpBckId_eLp2,
    BVDC_P_LpBckId_eLp3,
    BVDC_P_LpBckId_eLp4,
    BVDC_P_LpBckId_eLp5,
    BVDC_P_LpBckId_eLp6,
    BVDC_P_LpBckId_eLp7,
    BVDC_P_LpBckId_eLp8,
    BVDC_P_LpBckId_eLp9,
    BVDC_P_LpBckId_eLp10,
    BVDC_P_LpBckId_eLp11,
    BVDC_P_LpBckId_eLp12,
    BVDC_P_LpBckId_eLp13,
    BVDC_P_LpBckId_eLp14,
    BVDC_P_LpBckId_eUnknown
} BVDC_P_LpBckId;

typedef enum
{
    BVDC_P_DrainFrnId_eDrain0 = 0,
    BVDC_P_DrainFrnId_eDrain1,
    BVDC_P_DrainFrnId_eDrain2,
    BVDC_P_DrainFrnId_eDrain3,
    BVDC_P_DrainFrnId_eDrain4,
    BVDC_P_DrainFrnId_eDrain5,
    BVDC_P_DrainFrnId_eDrain6,
    BVDC_P_DrainFrnId_eUnknown
} BVDC_P_DrainFrnId;

typedef enum
{
    BVDC_P_DrainBckId_eDrain0 = 0,  /* back drain */
    BVDC_P_DrainBckId_eUnknown
} BVDC_P_DrainBckId;

typedef enum
{
    BVDC_P_PcPllId_ePll0 = 0,
    BVDC_P_PcPllId_eUnknown
} BVDC_P_PcPllId;

typedef enum
{
    /* The context is newly created and inactive for use.
     * Next state: eActive */
    BVDC_P_State_eInactive = 0,

    /* Mark it as eCreate.  It will become eActive when apply. */
    BVDC_P_State_eCreate,

    /* Currently in use.  Normal running state.  User call destroy/apply
     * to mark it as eDestroy. */
    BVDC_P_State_eActive,

    /* Mark for destroy, not applyed yet. Will become eShutDownPending
     * -> eShutDown -> eInactive. This state prompt the apply to mark it
     * as eShutDownPending */
    BVDC_P_State_eDestroy,

    /* Mark for destroy, is applyed, harware not disabled yet. Will become
     * BVDC_P_State_eShutDownRul when a disable RUL is build. This state
     * prompt the ISR to build disable RUL and mark it as eShutDownRul. */
    BVDC_P_State_eShutDownPending,

    /* Mark for destroy, is applyed, hardware not disabled yet. Will become
     * eDrainVnet (for writer) or eShutDown (for reader) when hardware is
     * disabled. This state prompt the ISR to build drain RUL (for writer)
     * and mark it as eDrainVnet (for writer) or only to mark it as
     * eShutDown (for reader). */
    BVDC_P_State_eShutDownRul,

    /* Mark for destroy, is applyed, hardware disabled. This state is for
     * writer only. Will become eShutDown when vnet is drained. This state
     * prompt the ISR to mark it as eShutDown.
     * Note: this state is brought back due to PR 31849:  Soft Reset when
     * CAP is Busy Corrupts Memory. We find that CAP might still have pending
     * memory write after disabled. This could potentially a problem for
     * other memory writers such as ANR and MAD too */
    BVDC_P_State_eDrainVnet,

    /* Mark for destroy, is applyed, and harware is disabled. Will become
     * eInactive. */
    BVDC_P_State_eShutDown

} BVDC_P_State;

/* Rectangle structure */
typedef struct
{
    int32_t      lLeft;
    int32_t      lTop;
    uint32_t     ulWidth;
    uint32_t     ulHeight;

    /* Delta for right eye */
    int32_t      lLeft_R;

} BVDC_P_Rect;


/* Scissor clipping values.  Relative clipping values from the four
 * edges of the rectangle. */
typedef struct
{
    uint32_t     ulLeft;
    uint32_t     ulRight;
    uint32_t     ulTop;
    uint32_t     ulBottom;

    /* Delta for right eye */
    int32_t      lLeftDelta_R;

} BVDC_P_ClipRect;

/* Supported features of vdc; number of compositor/sources/windows */
typedef struct
{
    bool         bCmpBIsBypass;
    bool         abAvailCmp[BVDC_P_MAX_COMPOSITOR_COUNT];
    bool         abAvailSrc[BVDC_P_MAX_SOURCE_COUNT];
    bool         ab3dSrc[BVDC_P_MAX_SOURCE_COUNT];
} BVDC_P_Features;

/* RDC list information.  Since we were going to do a lot of BRDC_List_Get_*
 * and set BRDC_List_Set_*, this would save all those call, and just have
 * the main initial _isr call it and store it in this struct, and call a set
 * when every is done building the RUL. */
typedef struct
{
    bool         bLastExecuted;
    /* mosaic mode could build chained slave RULs, which shouldn't overwrite
       timestamp captured by master RUL; otherwise, bMasterList is true,
       and timestamp readback takes place in the RUL; */
    bool         bMasterList;
    uint32_t    *pulStart;
    uint32_t    *pulCurrent;

    /* PsF: mark the playback RUL size; if ISR is missed, chop the RUL size
       to not to scanout consecutive 1080p from MFD and capture; */
    uint32_t     ulPsfMark;
} BVDC_P_ListInfo;

/* CFC Versions
 */
/* only has basic Mc matrix */
#define BVDC_P_CFC_VER_0                  (0) /* older than 7439 B0 */

/* Able to convert among SDR BT709 / BT2020NCL / BT2020CL / ... */
#define BVDC_P_CFC_VER_1                  (1) /* 7439 B0 and up */

/* Able to convert between any combination of SDR / HDR, BT709 / BT2020NCL / BT2020CL / ... */
#define BVDC_P_CFC_VER_2                  (2) /* 7271 A0 and up */

/* Able to handle HLG OOTF-Adj, bolby and TpToneMapping  */
#define BVDC_P_CFC_VER_3                  (3) /* 7271 B0 and up */

#define BVDC_P_CMP_NL_CFG_REGS                (BAVC_MOSAIC_MAX / 2)
#define BVDC_P_CMP_LR_ADJ_PTS                 (8)

/* CSC configure for a CMP/GFD that has NL CSC HW */
#define BVDC_P_NL_CSC_CTRL_SEL_BYPASS         (0xFFFFFFFF)

/* 4x3 Color Conversion Matrix. two's complement fixed point values.
 * Precision see bvdc_csc_priv.h. */
typedef struct
{
    uint16_t     usY0;
    uint16_t     usY1;
    uint16_t     usY2;
    uint16_t     usYAlpha;
    uint16_t     usYOffset;

    uint16_t     usCb0;
    uint16_t     usCb1;
    uint16_t     usCb2;
    uint16_t     usCbAlpha;
    uint16_t     usCbOffset;

    uint16_t     usCr0;
    uint16_t     usCr1;
    uint16_t     usCr2;
    uint16_t     usCrAlpha;
    uint16_t     usCrOffset;

    uint16_t     usCxIntBits;    /* integer fixed point precision for coefficients */
    uint16_t     usCxFractBits;  /* fraction fixed point precision for coefficients */

    uint16_t     usCoIntBits;    /* integer fixed point precision for offset */
    uint16_t     usCoFractBits;  /* fraction fixed point precision for offset */

    uint16_t     usCoVideoBits;  /* video bit format for offset */

} BVDC_P_CscCoeffs;

/* Use for Dither */
typedef struct
{
    /* xxx_DITHER_LFSR_INIT */
    uint32_t                        ulLfsrInitReg;
    /* xxx_DITHER_LFSR_CTRL */
    uint32_t                        ulLfsrCtrlReg;
    /* xxx_DITHER_CTRL */
    uint32_t                        ulCtrlReg;
} BVDC_P_DitherSetting;


/* Use for buffer Compression  */
typedef struct
{
    bool                            bEnable;
    uint32_t                        ulPixelPerGroup;
    uint32_t                        ulBitsPerGroup;
    uint32_t                        ulPredictionMode;

} BVDC_P_Compression_Settings;

/***************************************************************************
 * VDC Internal macros
 ***************************************************************************/
/* Not to use a parameter. */
#define BVDC_P_UNUSED(x)                    BSTD_UNUSED(x)

#define BVDC_P_ALIGN_UP(addr, alignment) \
    (((uint32_t)(addr) + (alignment) - 1) & ~((alignment) - 1))

#define BVDC_P_ALIGN_DN(addr, alignment) \
    (((uint32_t)(addr)) & ~((alignment) - 1))

#define BVDC_P_IS_ALIGN(addr, alignment) \
    (((uint32_t)(addr)) == BVDC_P_ALIGN_DN((addr), (alignment)))

#define BVDC_P_DIV_ROUND_UP(a, b) \
    (((a) + ((b) - 1)) / (b))

#define BVDC_P_DIV_ROUND_DN(a, b) \
    ((a) / (b))

#define BVDC_P_DIV_ROUND_NEAR(a, b) \
    (((a) + ((b)>>1)) / (b))

/* This macro test state of handle */
#define BVDC_P_STATE_IS_INACTIVE(handle) \
    ((NULL != (handle)) && (BVDC_P_State_eInactive==(handle)->eState))

#define BVDC_P_STATE_IS_ACTIVE(handle) \
    ((NULL != (handle)) && (BVDC_P_State_eActive   ==(handle)->eState))

#define BVDC_P_STATE_IS_CREATE(handle) \
    ((NULL != (handle)) && (BVDC_P_State_eCreate   ==(handle)->eState))

#define BVDC_P_STATE_IS_DESTROY(handle) \
    ((NULL != (handle)) && (BVDC_P_State_eDestroy  ==(handle)->eState))

#define BVDC_P_STATE_IS_SHUTDOWNPENDING(handle) \
    ((NULL != (handle)) && (BVDC_P_State_eShutDownPending ==(handle)->eState))

#define BVDC_P_STATE_IS_SHUTDOWNRUL(handle) \
    ((NULL != (handle)) && (BVDC_P_State_eShutDownRul ==(handle)->eState))

#define BVDC_P_STATE_IS_SHUTDOWN(handle) \
    ((NULL != (handle)) && (BVDC_P_State_eShutDown  ==(handle)->eState))

     /* Take two rectangle pointers and compare src to dst. */
#define BVDC_P_RECT_CMP_EQ(dst, src) \
    (((src)->lLeft   ==(dst)->lLeft   ) && \
     ((src)->lTop    ==(dst)->lTop    ) && \
     ((src)->ulWidth ==(dst)->ulWidth ) && \
     ((src)->ulHeight==(dst)->ulHeight) && \
     ((src)->lLeft_R ==(dst)->lLeft_R ))

#define BVDC_P_CLIP_RECT_CMP_EQ(dst, src) \
    (((src)->ulLeft  ==(dst)->ulLeft  ) && \
     ((src)->ulRight ==(dst)->ulRight ) && \
     ((src)->ulTop   ==(dst)->ulTop   ) && \
     ((src)->ulBottom==(dst)->ulBottom) && \
     ((src)->lLeftDelta_R ==(dst)->lLeftDelta_R))

/* Get the next field/frame polarity. */
#define BVDC_P_NEXT_POLARITY(polarity) \
    ((BAVC_Polarity_eTopField==(polarity)) ? BAVC_Polarity_eBotField \
    :(BAVC_Polarity_eBotField==(polarity)) ? BAVC_Polarity_eTopField \
    :BAVC_Polarity_eFrame)

/* Get the min / max of two numbers */
#define BVDC_P_MIN(a, b)        (((a) < (b)) ? (a) : (b))
#define BVDC_P_MAX(a, b)        (((a) > (b)) ? (a) : (b))

/* Compare value a == (b +/- delta) */
#define BVDC_P_EQ_DELTA(value_a, value_b, delta) \
    (((value_a) <= ((value_b) + (delta))) && \
     ((value_b) <= ((value_a) + (delta))))

/* Round off given a threshold and a precision */
#define BVDC_P_ROUND_OFF(a, threshold, precision)  \
    (((a) + (threshold)) / (precision))

/*
 * Build RUL for read/modify/write a register.
 */
#define BVDC_P_RD_MOD_WR_RUL(pulCurrent, ulAndMask, ulOrMask, ulRegAddr) \
    BRDC_BuildRul_RdModWr_isr(&pulCurrent, ulAndMask, ulOrMask, ulRegAddr)

/* Build a imm write instruction. */
#define BVDC_P_BUILD_IMM_WR(pulCurrent, ulAddr, ulValue) \
do { \
    *(pulCurrent)++ = BRDC_OP_IMM_TO_REG(); \
    *(pulCurrent)++ = BRDC_REGISTER(ulAddr); \
    *(pulCurrent)++ = (ulValue); \
} while(0)

/* Add in the no ops (real no-op intruction does not work correctly). */
#define BVDC_P_BUILD_NO_OPS(pulCurrent) \
    BVDC_P_BUILD_IMM_WR(pulCurrent, BVDC_P_SCRATCH_WO_REG, 0xbaadf00d);

/* Add in the a force trigger. */
#define BVDC_P_BUILD_IMM_EXEC_OPS(pulCurrent, ulImmTriggerAddr) \
    BVDC_P_BUILD_IMM_WR(pulCurrent, ulImmTriggerAddr, 1);

/* move reg to reg or reg copy*/
#define BVDC_P_BUILD_REG_COPY_OPS(pulCurrent, ulSrcAddr, ulDstAddr) \
do {\
    *(pulCurrent)++ = BRDC_OP_REG_TO_REG(1); \
    *(pulCurrent)++ = BRDC_REGISTER(ulSrcAddr); \
    *(pulCurrent)++ = BRDC_REGISTER(ulDstAddr); \
} while(0)

/* Add in the reset for a BVN module!  Reset procedure is:
 * Write 1 and then 0 to reset. */
#define BVDC_P_BUILD_RESET(pulCurrent, ulResetAddr, ulResetMask) \
do { \
    *(pulCurrent)++ = BRDC_OP_IMM_TO_REG(); \
    *(pulCurrent)++ = BRDC_REGISTER(ulResetAddr); \
    *(pulCurrent)++ = (ulResetMask); \
    *(pulCurrent)++ = BRDC_OP_IMM_TO_REG(); \
    *(pulCurrent)++ = BRDC_REGISTER(ulResetAddr); \
    *(pulCurrent)++ = 0; \
} while(0)

/* Add NO-ops between reset: recommend 4 no-ops between 1 and 0 */
#define BVDC_P_BUILD_RESET_NOOPS(pulCurrent, ulResetAddr, ulResetMask) \
do { \
    *(pulCurrent)++ = BRDC_OP_IMM_TO_REG(); \
    *(pulCurrent)++ = BRDC_REGISTER(ulResetAddr); \
    *(pulCurrent)++ = (ulResetMask); \
    *(pulCurrent)++ = BRDC_OP_NOP(); \
    *(pulCurrent)++ = BRDC_OP_NOP(); \
    *(pulCurrent)++ = BRDC_OP_NOP(); \
    *(pulCurrent)++ = BRDC_OP_NOP(); \
    *(pulCurrent)++ = BRDC_OP_IMM_TO_REG(); \
    *(pulCurrent)++ = BRDC_REGISTER(ulResetAddr); \
    *(pulCurrent)++ = 0; \
} while(0)

/* Copy table to current list pointer */
#define BVDC_P_ADD_TO_RUL(pulCurrent, s_aulTable) \
{ \
    BKNI_Memcpy(pulCurrent, s_aulTable, sizeof(s_aulTable)); \
    pulCurrent += sizeof(s_aulTable) / sizeof(uint32_t); \
}

/***************************************************************************
 * Generic set/get/compare field
 ***************************************************************************/
/* Get register field value by register name and field name.
 *
 * This is equivalent to:
 * ulFoo = (regvar & BCHP_VDEC_MODE_TV_SYSTEM_MASK) >>
 *    BCHP_VDEC_MODE_TV_SYSTEM_SHIFT);
 *
 * Example:
 *   ulReg = BREG_Read32(hRegister, BCHP_VDEC_MODE);
 *   BVDC_P_GET_FIELD(ulReg, VDEC_MODE, TV_SYSTEM);
 *   BREG_Write32(BCHP_VDEC_MODE, &ulReg).
 *
 */
#define BVDC_P_GET_FIELD(regvar, reg, field) \
    (((regvar) & BCHP_##reg##_##field##_MASK) >> \
    BCHP_##reg##_##field##_SHIFT)

/* Compare a register field by value or name.
 * Example:
 *   ...
 *   ulReg = BREG_Read32(hRegister, BCHP_VDEC_MODE);
 *   if(BVDC_P_COMPARE_FIELD_NAME(ulReg, VDEC_MODE, TV_SYSTEM, 0))
 *   {
 *      BDBG_MSG("Input video is NTSC");
 *   }
 *or
 *   if(BVDC_P_COMPARE_FIELD_NAME(ulReg, VDEC_MODE, TV_SYSTEM, NTSC))
 *   {
 *      BDBG_MSG("Input video is NTSC");
 *   }
 */
#define BVDC_P_COMPARE_FIELD_DATA(regvar, reg, field, data) \
    (BVDC_P_GET_FIELD((regvar), reg, field)==(data))

#define BVDC_P_COMPARE_FIELD_NAME(regvar, reg, field, name) \
    (BVDC_P_GET_FIELD((regvar), reg, field)==BCHP##_##reg##_##field##_##name)

/***************************************************************************
 * VDC Internal data structures
 ***************************************************************************/
/* Forward declarations of different type of source. */
typedef struct BVDC_P_GfxFeederContext   *BVDC_P_GfxFeeder_Handle;
typedef struct BVDC_P_FeederContext      *BVDC_P_Feeder_Handle;
typedef struct BVDC_P_ScalerContext      *BVDC_P_Scaler_Handle;
typedef struct BVDC_P_HscalerContext     *BVDC_P_Hscaler_Handle;
typedef struct BVDC_P_XsrcContext        *BVDC_P_Xsrc_Handle;
typedef struct BVDC_P_VfcContext         *BVDC_P_Vfc_Handle;
typedef struct BVDC_P_TntdContext        *BVDC_P_Tntd_Handle;
typedef struct BVDC_P_DnrContext         *BVDC_P_Dnr_Handle;
typedef struct BVDC_P_AnrContext         *BVDC_P_Anr_Handle;
typedef struct BVDC_P_VnetCrcContext     *BVDC_P_VnetCrc_Handle;
typedef struct BVDC_P_McvpContext        *BVDC_P_Mcvp_Handle;
typedef struct BVDC_P_McdiContext        *BVDC_P_Mcdi_Handle;
typedef struct BVDC_P_BoxDetectContext   *BVDC_P_BoxDetect_Handle;
typedef struct BVDC_P_CaptureContext     *BVDC_P_Capture_Handle;
typedef struct BVDC_P_BufferContext      *BVDC_P_Buffer_Handle;
typedef struct BVDC_P_VnetContext        *BVDC_P_Vnet_Handle;
typedef struct BVDC_P_HdDviContext       *BVDC_P_HdDvi_Handle;
typedef struct BVDC_P_BufferHeapContext  *BVDC_P_BufferHeap_Handle;
typedef struct BVDC_P_PepContext         *BVDC_P_Pep_Handle;
typedef struct BVDC_P_656InContext       *BVDC_P_656In_Handle;
typedef struct BVDC_P_ResourceContext    *BVDC_P_Resource_Handle;
typedef struct BVDC_P_Source_Info        *BVDC_P_Source_InfoPtr;
typedef struct BVDC_P_PictureNode        *BVDC_P_PictureNodePtr;
typedef struct BVDC_P_BufferHeapNode     *BVDC_P_HeapNodePtr;
typedef struct BVDC_P_BufferHeap_Info    *BVDC_P_HeapInfoPtr;
typedef struct BVDC_P_VipContext         *BVDC_P_Vip_Handle;

/* override to make the system as basic as possible for boot loader usage */
#ifdef BVDC_FOR_BOOTUPDATER

#undef BVDC_P_SUPPORT_656_MASTER_MODE
#undef BVDC_P_SUPPORT_LOOP_BACK
#undef BVDC_P_SUPPORT_VEC_GRPD
#undef BVDC_P_SUPPORT_CAP_VER

/* source */
#undef BVDC_P_SUPPORT_MFD
#undef BVDC_P_SUPPORT_MFD_VER
#undef BVDC_P_SUPPORT_MTG
#undef BVDC_P_SUPPORT_MADR_VER
#undef BVDC_P_SUPPORT_HDDVI
#undef BVDC_P_SUPPORT_HDDVI_VER

/* BVN */
#undef BVDC_P_SUPPORT_PEP
#undef BVDC_P_SUPPORT_PEP_VER
#undef BVDC_P_SUPPORT_HIST
#undef BVDC_P_SUPPORT_TNT
#undef BVDC_P_SUPPORT_TNT_VER
#undef BVDC_P_SUPPORT_TNTD
#undef BVDC_P_SUPPORT_TNTD_VER
#undef BVDC_P_SUPPORT_MASK_DITHER
#undef BVDC_P_SUPPORT_DNR
#undef BVDC_P_SUPPORT_BOX_DETECT
#undef BVDC_P_SUPPORT_BOX_DETECT_VER
#undef BVDC_P_SUPPORT_CAP
#undef BVDC_P_SUPPORT_VFD
#undef BVDC_P_SUPPORT_SCL
#undef BVDC_P_SUPPORT_SCL_VER
#undef BVDC_P_SUPPORT_XSRC
#undef BVDC_P_SUPPORT_XSRC_VER
#undef BVDC_P_SUPPORT_VFC
#undef BVDC_P_SUPPORT_HSCL_VER
#undef BVDC_P_SUPPORT_DMISC
#undef BVDC_P_SUPPORT_FREE_CHANNEL
#undef BVDC_P_SUPPORT_DRAIN_F
#undef BVDC_P_SUPPORT_DRAIN_B
#undef BVDC_P_SUPPORT_DRAIN_VER

#undef BVDC_P_SUPPORT_MCVP
#undef BVDC_P_SUPPORT_MCVP_VER
#undef BVDC_P_SUPPORT_MCDI_VER
#undef BVDC_P_SUPPORT_MADR
#undef BVDC_P_SUPPORT_MANR
#undef BVDC_P_SUPPORT_MANR_VER
#undef BVDC_P_SUPPORT_VIDEO_TESTFEATURE1_VER
#undef BVDC_P_SUPPORT_XCODE_WIN_CAP

/* CMP */
#undef BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT
#undef BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT
#undef BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT
#undef BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT
#undef BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT
#undef BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT
#undef BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT
#undef BVDC_P_CMP_CFC_VER

#undef BVDC_P_CMP_0_MOSAIC_CFCS
#undef BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS
#undef BVDC_P_CMP_i_MOSAIC_CFCS
#undef BVDC_P_CMP_0_V0_CLEAR_RECTS
#undef BVDC_P_CMP_0_V1_CLEAR_RECTS
#undef BVDC_P_CMP_1_V0_CLEAR_RECTS
#undef BVDC_P_CMP_1_V1_CLEAR_RECTS
#undef BVDC_P_CMP_2_V0_CLEAR_RECTS
#undef BVDC_P_CMP_3_V0_CLEAR_RECTS
#undef BVDC_P_CMP_4_V0_CLEAR_RECTS
#undef BVDC_P_CMP_5_V0_CLEAR_RECTS
#undef BVDC_P_CMP_6_V0_CLEAR_RECTS

/* csc */
#undef BVDC_P_SUPPORT_CMP_DEMO_MODE

/* display */
#undef BVDC_P_SUPPORT_STG_VER

#undef BVDC_P_NUM_SHARED_656
#undef BVDC_P_NUM_SHARED_STG
/* Note, here SHARED_SECAM/HDSECAM means the secam cross bar.
 * Some secam cross bar may not have a secam module associated with it.
 */
#undef BVDC_P_SUPPORT_MEM_PWR_GATING
#undef BVDC_P_SUPPORT_3D_INDEP_SRC_CLIP
#undef BVDC_P_SUPPORT_STG

#undef BCHP_PWR_RESOURCE_VDC
#undef BCHP_PWR_SUPPORT

#undef BVDC_SUPPORT_BVN_DEBUG

#undef BDBG_DEBUG_BUILD

#define BVDC_P_SUPPORT_MFD                    (0)
#define BVDC_P_SUPPORT_VFD                    (0)
#define BVDC_P_SUPPORT_DNR                    (0)
#define BVDC_P_SUPPORT_XSRC                   (0)
#define BVDC_P_SUPPORT_VFC                    (0)
#define BVDC_P_SUPPORT_MCVP                   (0)
#define BVDC_P_SUPPORT_CAP                    (0)
#define BVDC_P_SUPPORT_PEP                    (0)
#define BVDC_P_SUPPORT_TNT                    (0)
#define BVDC_P_SUPPORT_TNTD                   (0)
#define BVDC_P_SUPPORT_STG                    (0)
#define BVDC_P_SUPPORT_HDDVI                  (0)

#define BVDC_P_SUPPORT_BOX_DETECT             (0)
#define BVDC_P_SUPPORT_HIST                   (0)
#define BVDC_P_SUPPORT_FREE_CHANNEL           (0)
#define BVDC_P_SUPPORT_LOOP_BACK              (0)
#define BVDC_P_SUPPORT_DRAIN_F                (0)
#define BVDC_P_SUPPORT_DRAIN_B                (0)
#define BVDC_P_NUM_SHARED_656                 (0)
#define BVDC_P_NUM_SHARED_STG                 (0)

#define BVDC_P_CMP_0_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_0_V1_CLEAR_RECTS           (0)
#define BVDC_P_CMP_1_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_1_V1_CLEAR_RECTS           (0)
#define BVDC_P_CMP_2_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_3_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_4_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_5_V0_CLEAR_RECTS           (0)
#define BVDC_P_CMP_6_V0_CLEAR_RECTS           (0)

#define BVDC_P_CMP_CFC_VER                    (0)
#define BVDC_P_CMP_0_MAX_VIDEO_WINDOW_COUNT   (0)
#define BVDC_P_CMP_1_MAX_VIDEO_WINDOW_COUNT   (0)
#define BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT   (0)
#define BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_3_Vx */
#define BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_4_Vx */
#define BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_5_Vx */
#define BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT   (0) /* Number of CMP_6_Vx */

#define BVDC_P_CMP_0_MOSAIC_TF_CONV_CFCS      (0)
#define BVDC_P_CMP_i_MOSAIC_CFCS              (0)

#undef BVDC_P_MAX_4HD_BUFFER_COUNT
#undef BVDC_P_MAX_2HD_BUFFER_COUNT
#undef BVDC_P_BYPASS_MULTI_BUFFER_COUNT
#undef BVDC_P_SYNC_LOCK_MULTI_BUFFER_COUNT
#undef BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT
#undef BVDC_P_MAX_USER_CAPTURE_BUFFER_COUNT
#undef BVDC_P_MAX_MULTI_BUFFER_COUNT
#undef BVDC_P_MAX_HD_BUFFER_COUNT
#undef BVDC_P_MAX_SD_BUFFER_COUNT
#undef BVDC_P_MAX_MCDI_BUFFER_COUNT
#undef BVDC_P_MAX_MAD_SD_BUFFER_COUNT

#undef BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT

#undef BVDC_P_MAX_3DCOMB_SD_BUFFER_COUNT
#undef BVDC_P_MAD_PIXEL_SD_BUFFER_COUNT
#undef BVDC_P_MCDI_QM_BUFFER_COUNT
#undef BVDC_P_MAD_QM_FIELD_STORE_COUNT

#undef BVDC_P_SUPPORT_ITU656_OUT
#undef BVDC_P_SUPPORT_RFM_OUTPUT

#define BVDC_P_MAX_MULTI_RUL_BUFFER_COUNT     (1)

#define BVDC_P_MAX_3DCOMB_SD_BUFFER_COUNT     (0)
#define BVDC_P_MAX_4HD_BUFFER_COUNT           (0)
#define BVDC_P_MAX_2HD_BUFFER_COUNT           (0)
#define BVDC_P_MAX_HD_BUFFER_COUNT            (0)
#define BVDC_P_MAX_SD_BUFFER_COUNT            (0)
#define BVDC_P_SUPPORT_RFM_OUTPUT             (0)
#define BVDC_P_SUPPORT_ITU656_OUT             (0)
#define BVDC_P_MAD_PIXEL_SD_BUFFER_COUNT      (0)
#define BVDC_P_MAX_MCDI_BUFFER_COUNT          (0)
#define BVDC_P_MCDI_QM_BUFFER_COUNT           (0)
#define BVDC_P_MAX_USER_CAPTURE_BUFFER_COUNT  (0)
#define BVDC_P_SYNC_SLIP_MULTI_BUFFER_COUNT   (0)
#define BVDC_P_SYNC_LOCK_MULTI_BUFFER_COUNT   (0)
#define BVDC_P_BYPASS_MULTI_BUFFER_COUNT      (0)
#define BVDC_P_MAD_QM_FIELD_STORE_COUNT       (0)
#define BVDC_P_MAX_MULTI_BUFFER_COUNT         (0)

#undef BVDC_P_STG_NRT_CADENCE_WORKAROUND
#undef BVDC_P_DCX_CAP_RESET_WORKAROUND
#undef BVDC_P_DCX_VFD_RESET_WORKAROUND
#undef BVDC_P_DCX_HSIZE_WORKAROUND
#undef BVDC_P_MVFD_ALIGNMENT_WORKAROUND

/* not support 4k display in bootupdater yet */
#undef BVDC_P_SUPPORT_4kx2k_60HZ
#define BVDC_P_SUPPORT_4kx2k_60HZ             (0)
#undef BVDC_P_MANAGE_VIP
#define BVDC_P_MANAGE_VIP                     (0)
#undef BVDC_SUPPORT_VIP_DEBUG
#define BVDC_SUPPORT_VIP_DEBUG                (0)

#endif /* #ifdef BVDC_FOR_BOOTUPDATER */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_COMMON_PRIV_H__*/

/* End of file. */
