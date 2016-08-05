/******************************************************************************
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
 ******************************************************************************/

#ifndef BVDC_DISPLAY_PRIV_H__
#define BVDC_DISPLAY_PRIV_H__

#include "bvdc.h"
#include "bvdc_priv.h"
#if DCS_SUPPORT
#include "bvdc_dcs_priv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "bchp_common.h"
#include "bchp_it_0.h"
#include "bchp_rm_0.h"
#include "bchp_vf_0.h"
#include "bchp_csc_0.h"
#include "bchp_sm_0.h"
#include "bchp_sdsrc_0.h"
#ifdef BCHP_HDSRC_0_REG_START
#include "bchp_hdsrc_0.h"
#endif
#ifdef BCHP_SECAM_0_REG_START
#include "bchp_secam_0.h"
#endif

#include "bchp_vec_cfg.h"
#include "bchp_misc.h"
#include "bchp_video_enc_decim_0.h"
#include "bchp_timer.h"
#include "bvdc_resource_priv.h"

#include "bchp_hdmi.h"
#include "bchp_hdmi_rm.h"
#include "bchp_hdmi_tx_phy.h"
#include "bchp_dvi_dtg_0.h"
#include "bchp_dvi_dvf_0.h"
#ifdef BCHP_DVI_CSC_0_REG_START
#include "bchp_dvi_csc_0.h"
#endif
#include "bchp_dvp_ht.h"

#ifdef BCHP_DTRAM_0_REG_START
#include "bchp_dtram_0.h"
#endif

#ifdef BCHP_ITU656_0_REG_START
#include "bchp_itu656_0.h"
#include "bchp_itu656_csc_0.h"
#include "bchp_itu656_dtg_0.h"
#include "bchp_itu656_dvf_0.h"
#endif /* 656 */

#ifdef BCHP_VIDEO_ENC_STG_0_REG_START
#include "bchp_video_enc_stg_0.h"
#endif

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif

#define BVDC_P_SHOW_VEC_MSG               0
#if (BVDC_P_SHOW_VEC_MSG==1)
#define BVDC_P_VEC_MSG    BDBG_ERR
#else
#define BVDC_P_VEC_MSG(a)
#endif

#if (BVDC_P_SUPPORT_IT_VER >= 3)
#define BVDC_P_VEC_STANDALONE_BUG_FIXED        1
#else
#define BVDC_P_VEC_STANDALONE_BUG_FIXED        0
#endif

#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY
#define BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY0 BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY
#define BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY1 BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY
#endif

#ifdef BCHP_PWR_RESOURCE_VDC_STG
#define BCHP_PWR_RESOURCE_VDC_STG0 BCHP_PWR_RESOURCE_VDC_STG
#define BCHP_PWR_RESOURCE_VDC_STG1 BCHP_PWR_RESOURCE_VDC_STG
#define BCHP_PWR_RESOURCE_VDC_STG2 BCHP_PWR_RESOURCE_VDC_STG
#define BCHP_PWR_RESOURCE_VDC_STG3 BCHP_PWR_RESOURCE_VDC_STG
#define BCHP_PWR_RESOURCE_VDC_STG4 BCHP_PWR_RESOURCE_VDC_STG
#define BCHP_PWR_RESOURCE_VDC_STG5 BCHP_PWR_RESOURCE_VDC_STG
#endif

/****************************************************************
 *  Defines
 ****************************************************************/
/* Original Orthgonal VEC to 7425Ax */
#define BVDC_P_ORTHOGONAL_VEC_VER_0          (0)

/* 7425 B0 - new vec trigger */
#define BVDC_P_ORTHOGONAL_VEC_VER_1          (1)

/* 7366/7439 - dual HDMI output */
#define BVDC_P_ORTHOGONAL_VEC_VER_2          (2)


/* ---------------------------------------------
 * HDMI_RM revision
 * --------------------------------------------- */
/* 3548 Ax, B0, B1, 3556 Ax, B0, B1
 *  DVPO support
 */
#define BVDC_P_HDMI_RM_VER_0                 (0)

/* 3548 B2 and above, 3556 B2 and above
 *  DVPO support with fixed spread spectrum
 */
#define BVDC_P_HDMI_RM_VER_1                 (1)

/* 7400, 7405, 7335, 7336
 *  DVI support: 65NM 54MHz
 */
#define BVDC_P_HDMI_RM_VER_2                 (2)

/* 7325
 *  DVI support: 65NM 27MHz
 */
#define BVDC_P_HDMI_RM_VER_3                 (3)

/* 7125
 *  DVI support: 65NM 27MHz
 */
#define BVDC_P_HDMI_RM_VER_4                 (4)

/* 7422, 7425, 7358, 7552, 7231, 7346, 7344
 *  DVI support: 40NM 54MHz
 */
#define BVDC_P_HDMI_RM_VER_5                 (5)

/* 7445
 *  DVI support: 28NM 54MHz
 */
#define BVDC_P_HDMI_RM_VER_6                 (6)

/* 7364A
 *  DVI support: BCHP_RM_0_INTEGRATOR_LO/HI, NEW TRACKING RANGES.
 */
#define BVDC_P_HDMI_RM_VER_7                 (7)

#define BVDC_P_SUPPORT_DVI_40NM                   \
    (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_5)

#define BVDC_P_SUPPORT_DVI_28NM                              \
    ((BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_6) || \
     (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_7))

#define BVDC_P_MAX_VEC_RUL_ENTRIES        665
#define BVDC_P_RAM_TABLE_SIZE             256
#define BVDC_P_DTRAM_TABLE_SIZE           (128/2)
#define BVDC_P_RAM_TABLE_TIMESTAMP_IDX    (BVDC_P_RAM_TABLE_SIZE - 2)
#define BVDC_P_RAM_TABLE_CHECKSUM_IDX     (BVDC_P_RAM_TABLE_SIZE - 1)
#define BVDC_P_DTRAM_TABLE_TIMESTAMP_IDX  (BVDC_P_DTRAM_TABLE_SIZE - 2)
#define BVDC_P_DTRAM_TABLE_CHECKSUM_IDX   (BVDC_P_DTRAM_TABLE_SIZE - 1)
#define BVDC_P_CCB_TABLE_SIZE             (1)

#define BVDC_P_CSC_TABLE_SIZE             (uint32_t)((BCHP_CSC_0_CSC_COEFF_C23_C22 - BCHP_CSC_0_CSC_MODE)/4+1)
#define BVDC_P_DITHER_TABLE_SIZE          (uint32_t)((BCHP_CSC_0_DITHER_LFSR_INIT - BCHP_CSC_0_DITHER_CONTROL)/4+1)

/* Programming note: tables will actually be one larger than the following. */
#define BVDC_P_VF_TABLE_SIZE              (uint32_t)((BCHP_VF_0_SYNC_TRANS_1 - BCHP_VF_0_FORMAT_ADDER)/4+1)

#define BVDC_P_CHROMA_TABLE_SIZE          (uint32_t)((BCHP_VF_0_CH0_TAP10 - BCHP_VF_0_CH0_TAP1)/4+1)
#if (BVDC_P_SUPPORT_HDMI_RM_VER <= BVDC_P_HDMI_RM_VER_6)
#define BVDC_P_RM_TABLE_SIZE              (uint32_t)((BCHP_RM_0_INTEGRATOR - BCHP_RM_0_RATE_RATIO)/4+1)
#else
#define BVDC_P_RM_TABLE_SIZE              (uint32_t)((BCHP_RM_0_INTEGRATOR_LO - BCHP_RM_0_RATE_RATIO)/4+1)
#endif
#if (BVDC_P_SUPPORT_IT_VER >= 1)
#define BVDC_P_IT_TABLE_SIZE              (uint32_t)((BCHP_IT_0_STACK_reg_8_9 - BCHP_IT_0_ADDR_0_3)/4+1)
#else
#define BVDC_P_IT_TABLE_SIZE              (uint32_t)((BCHP_IT_0_PCL_5 - BCHP_IT_0_ADDR_0_3)/4+1)
#endif
#define BVDC_P_SM_TABLE_SIZE              (uint32_t)((BCHP_SM_0_COMP_CNTRL - BCHP_SM_0_PG_CNTRL)/4+1)

/* Vec phase adjustment values */
#define BVDC_P_PHASE_OFFSET                0x1d8
#define BVDC_P_THRESHOLD1                  2
#define BVDC_P_THRESHOLD2                  0x15
#define BVDC_P_PHASE_180                   0x200
#define BVDC_P_PHASE_90                    0x100
#define BVDC_P_PHASE_45                    0x080
#define BVDC_P_PHASE_135                   0x180
#define BVDC_P_PHASE_225                   0x280
#define BVDC_P_PHASE_270                   0x300
#define BVDC_P_PHASE_315                   0x380

/* NOTE:
   DAC design spec linearly maps DAC code value between 16 ~ 800 onto
   output voltage level 0 ~ 1000 mV;
   so we have scaling factor for DAC code as:
         (800 - 16) / 1000 = 0.784/mV
   where, value 16 maps sync tip voltage level, and 800 maps to 1000 mV
   peak to peak (picture peak white to sync tip) voltage level;   */
#define BVDC_P_DAC_CODE_MAX_VALUE               (800)
#define BVDC_P_DAC_CODE_MIN_VALUE               (16)
#define BVDC_P_DAC_OUTPUT_RANGE                 (1000) /* 1000 mV */

/* To convert mv into DAC code value relative to sync tip level;
   Note: here 'mv' is relative to sync tip level! */
#define BVDC_P_DAC_CODE_VALUE(mv) \
    ((mv) * (BVDC_P_DAC_CODE_MAX_VALUE - BVDC_P_DAC_CODE_MIN_VALUE) / \
     BVDC_P_DAC_OUTPUT_RANGE + BVDC_P_DAC_CODE_MIN_VALUE)

/* Care needs to be taken for NTSC CVBS/Svideo outputs vs 480i YPrPb/RGB
   or outputs of other formats:
   NTSC CVBS/Svideo outputs have 714mV/286mV Picture/Sync ratio, while
   all other cases have 700mV/300mV Picture/Sync ratio, i.e. picture
   white is at 700mV and sync tip at -300mv, all relative to blank level. */

/* NTSC CVBS/Svideo:
    286mV blank level relative to sync tip; */
#define BVDC_P_DAC_OUTPUT_NTSC_SYNC_LEVEL      (286)
#define BVDC_P_DAC_OUTPUT_NTSC_PEAK_WHITE_LEVEL \
    (BVDC_P_DAC_OUTPUT_RANGE - BVDC_P_DAC_OUTPUT_NTSC_SYNC_LEVEL)

/* All other outputs, including 480i YPrPb/RGB and/or CVBS/Svideo/YPrPb/RGB
   for other formats:
   300mV blank level relative to sync tip; */
#define BVDC_P_DAC_OUTPUT_SYNC_LEVEL           (300)
#define BVDC_P_DAC_OUTPUT_PEAK_WHITE_LEVEL \
    (BVDC_P_DAC_OUTPUT_RANGE - BVDC_P_DAC_OUTPUT_SYNC_LEVEL)

/* To convert mv into DAC code value relative to blank level;
   Note: here 'mv' is relative to the blank level! */
#define BVDC_P_POS_SYNC_AMPLITUDE_VALUE(mv) \
    ((mv) * (BVDC_P_DAC_CODE_MAX_VALUE - BVDC_P_DAC_CODE_MIN_VALUE) \
    / BVDC_P_DAC_OUTPUT_RANGE)

/* To convert mv into DAC code value relative to sync tip level */
#define BVDC_P_NEG_SYNC_AMPLITUDE_VALUE(mv) BVDC_P_DAC_CODE_VALUE(mv)

#define BVDC_P_NEG_SYNC_TIP_VALUE           BVDC_P_DAC_CODE_MIN_VALUE

/* Check for macro vision type */
#define BVDC_P_MACROVISION_ON_RGB(type) \
    ((type) > BVDC_MacrovisionType_eCustomized)

#define BVDC_P_MACROVISION_WITH_2LINES_CS(type) \
    (((type) == BVDC_MacrovisionType_eAgc2Lines) || \
     ((type) == BVDC_MacrovisionType_eAgc2Lines_Rgb))

#define BVDC_P_MACROVISION_WITH_4LINES_CS(type) \
    (((type) == BVDC_MacrovisionType_eAgc4Lines) || \
     ((type) == BVDC_MacrovisionType_eAgc4Lines_Rgb))

/* Where to trigger after vsync (line 1) */
#define BVDC_P_TRIGGER_LINE               (3)

/* Bx software workaround, number of vsyncs to wait before enabling
 * DVI input. */
#define BVDC_P_DVIINPUT_WAIT_VSYNCS       (1)

/* DVI DTRam LOCATION! */
#if (BVDC_P_ORTHOGONAL_VEC_VER <= BVDC_P_ORTHOGONAL_VEC_VER_1)
#define BVDC_P_DVI_DTRAM_START_ADDR       (0x40)
#else
#define BVDC_P_DVI_DTRAM_START_ADDR       (0x0)
#endif

#define BVDC_P_DISP_GET_TOP_TRIGGER(pDisplay) \
    ((pDisplay)->eTopTrigger)

#define BVDC_P_DISP_GET_BOT_TRIGGER(pDisplay) \
    ((pDisplay)->eBotTrigger)

/* Analog channel types */
#define BVDC_P_DISP_IS_ANLG_CHAN_CO(pChan, pDispInfo) \
    (((BVDC_P_Dac_Mask_YPrPb & (((pChan)->ulId == 0)? (pDispInfo)->ulAnlgChan0Mask : (pDispInfo)->ulAnlgChan1Mask)) == BVDC_P_Dac_Mask_YPrPb) || \
     ((BVDC_P_Dac_Mask_RGB & (((pChan)->ulId == 0)? (pDispInfo)->ulAnlgChan0Mask : (pDispInfo)->ulAnlgChan1Mask)) == BVDC_P_Dac_Mask_RGB))

#define BVDC_P_DISP_IS_ANLG_CHAN_CVBS(pChan, pDispInfo) \
    ((BVDC_P_Dac_Mask_Cvbs & (((pChan)->ulId == 0)? (pDispInfo)->ulAnlgChan0Mask : (pDispInfo)->ulAnlgChan1Mask)) == BVDC_P_Dac_Mask_Cvbs)

#define BVDC_P_DISP_IS_ANLG_CHAN_SVIDEO(pChan, pDispInfo) \
    ((BVDC_P_Dac_Mask_Svideo & (((pChan)->ulId == 0)? (pDispInfo)->ulAnlgChan0Mask : (pDispInfo)->ulAnlgChan1Mask)) == BVDC_P_Dac_Mask_Svideo)

#define BVDC_P_DISP_IS_VALID_DISPOUTPUT_AND_DAC(eDispOutput, eDacOutput)                                                               \
    (((eDisplayOutput == BVDC_DisplayOutput_eComponent) &&                                                                       \
      ((eDacOutput == BVDC_DacOutput_eY) || (eDacOutput == BVDC_DacOutput_ePr) || (eDacOutput == BVDC_DacOutput_ePb) ||          \
       (eDacOutput == BVDC_DacOutput_eRed) || (eDacOutput == BVDC_DacOutput_eGreen) || (eDacOutput == BVDC_DacOutput_eBlue) ||   \
       (eDacOutput == BVDC_DacOutput_eGreen_NoSync))) ? true :                                                                   \
        (((eDisplayOutput == BVDC_DisplayOutput_eComposite) && (eDacOutput == BVDC_DacOutput_eComposite)) ? true :               \
         ((eDisplayOutput == BVDC_DisplayOutput_eSVideo) &&                                                                      \
          ((eDacOutput == BVDC_DacOutput_eSVideo_Luma) || (eDacOutput == BVDC_DacOutput_eSVideo_Chroma)) ? true : false)))

#define BVDC_P_DISP_INVALID_VF_CH (-1)

/* Dither settings for DISP */
#define BVDC_P_DITHER_DISP_CSC_LFSR_VALUE            (0xFFC01)
#define BVDC_P_DITHER_DISP_CSC_LFSR_CTRL_T0          (0x5)
#define BVDC_P_DITHER_DISP_CSC_LFSR_CTRL_T1          (0x3)
#define BVDC_P_DITHER_DISP_CSC_LFSR_CTRL_T2          (0x5)
/* Dither settings for DVI, 656 */
#define BVDC_P_DITHER_DISP_DVI_LFSR_VALUE            (0)
#define BVDC_P_DITHER_DISP_DVI_LFSR_CTRL_T0          (0x1)
#define BVDC_P_DITHER_DISP_DVI_LFSR_CTRL_T1          (0x1)
#define BVDC_P_DITHER_DISP_DVI_LFSR_CTRL_T2          (0x1)

#define BVDC_P_DITHER_DISP_DVI_SCALE_10BIT           (0x1)
#define BVDC_P_DITHER_DISP_DVI_OFFSET_10BIT          (0x0)
#define BVDC_P_DITHER_DISP_DVI_SCALE_8BIT            (0x4)
#define BVDC_P_DITHER_DISP_DVI_OFFSET_8BIT           (0x1)

/* Special settings for 656 dither */
#define BVDC_P_DITHER_DISP_CSC_SCALE_CH0             (0x1)
#define BVDC_P_DITHER_DISP_CSC_SCALE_CH1             (0x5)
#define BVDC_P_DITHER_DISP_CSC_SCALE_CH2             (0x5)

/* Number of channels */
#define BVDC_P_VEC_CH_NUM       3


/****************************************************************
 *  Macros
 ****************************************************************/

/***************** RM clock adjustment macroes *************/
#define BVDC_P_TIMER_FREQ                                    (27000000) /* 27 MHz */

/* alignment threshold in usecs */
#define BVDC_P_USEC_ALIGNMENT_THRESHOLD     (200)

/* in 27 MHz clock cycles */
#define BVDC_P_DISPLAY_ONE_VSYNC_INTERVAL(vrate)    ( \
    ((uint32_t)BVDC_P_TIMER_FREQ * (uint32_t)BFMT_FREQ_FACTOR) / (vrate) )
#define BVDC_P_DISPLAY_ALIGNMENT_THRESHOLD    ( \
    BVDC_P_TIMER_FREQ / 1000000 * BVDC_P_USEC_ALIGNMENT_THRESHOLD) /* 200 usecs */

/* one vsync in usecs */
#define BVDC_P_USEC_ONE_VSYNC_INTERVAL(vrate)    ( \
    (1000000 * BFMT_FREQ_FACTOR / (vrate)))

/* Digital trigger */
#define BVDC_P_DISPLAY_USED_DIGTRIG(eMasterTg)    \
    ((eMasterTg == BVDC_DisplayTg_eDviDtg) || \
     (eMasterTg == BVDC_DisplayTg_e656Dtg) || \
     (BVDC_P_DISPLAY_USED_STG(eMasterTg)))

/* DVI trigger */
#define BVDC_P_DISPLAY_USED_DVI(eMasterTg)    \
    (eMasterTg == BVDC_DisplayTg_eDviDtg)

/* STG trigger transcoding path*/
#define BVDC_P_DISPLAY_USED_STG(eMasterTg)    \
    ((eMasterTg >= BVDC_DisplayTg_eStg0)   && \
     (eMasterTg < BVDC_DisplayTg_eUnknown))

/* STG non realtime */
#define BVDC_P_DISPLAY_NRT_STG(hDisplay)             \
    (BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg) && \
    (hDisplay->stCurInfo.bStgNonRealTime))

/* STG resolution ramp size factor */
#define BVDC_P_STG_RESOL_RAMP_SIZE_RATIO    (2)

/* BAVC_FrameRateCode is full rate (non 1/1001 drop).  */
#define BVDC_P_IS_FULL_FRAMRATE(eFrameRate)    \
   ((BAVC_FrameRateCode_e24 == eFrameRate) || \
    (BAVC_FrameRateCode_e25 == eFrameRate) || \
    (BAVC_FrameRateCode_e30 == eFrameRate) || \
    (BAVC_FrameRateCode_e50 == eFrameRate) || \
    (BAVC_FrameRateCode_e60 == eFrameRate) || \
    (BAVC_FrameRateCode_e15 == eFrameRate) || \
    (BAVC_FrameRateCode_e10 == eFrameRate) || \
    (BAVC_FrameRateCode_e12_5 == eFrameRate) || \
    (BAVC_FrameRateCode_e20 == eFrameRate) || \
    (BAVC_FrameRateCode_e100 == eFrameRate) || \
    (BAVC_FrameRateCode_e120 == eFrameRate) || \
    (BAVC_FrameRateCode_e12 == eFrameRate) || \
    (BAVC_FrameRateCode_e7_5 == eFrameRate))

/* Customer Fmt */
#define BVDC_P_IS_CUSTOMFMT(eVideoFmt)        \
    ((eVideoFmt == BFMT_VideoFmt_eCustom0) || \
     (eVideoFmt == BFMT_VideoFmt_eCustom1) || \
     (eVideoFmt == BFMT_VideoFmt_eCustom2))
/* Custom fmt comparison */
#define BVDC_P_IS_CUSTOMFMT_DIFF(pStgFmtInfo, pCustomFmtInfo)          \
    (((pStgFmtInfo) != NULL) && \
     (BVDC_P_IS_CUSTOMFMT((pStgFmtInfo)->eVideoFmt)) &&                       \
     (BVDC_P_IS_CUSTOMFMT((pCustomFmtInfo)->eVideoFmt)) &&                    \
     (((pStgFmtInfo)->ulDigitalWidth  != (pCustomFmtInfo)->ulDigitalWidth)  ||\
     ((pStgFmtInfo)->ulDigitalHeight != (pCustomFmtInfo)->ulDigitalHeight)  ||\
     (!BVDC_P_EQ_DELTA((pStgFmtInfo)->ulVertFreq, (pCustomFmtInfo)->ulVertFreq, BFMT_FREQ_FACTOR))))

#define BVDC_P_DISPLAY_NODELAY(pStgFmt, pFmt)        \
        (((pStgFmt == NULL) || (pStgFmt == pFmt)) || \
        (( pStgFmt != NULL) && \
         BVDC_P_IS_CUSTOMFMT(pStgFmt->eVideoFmt) &&  \
         BVDC_P_IS_CUSTOMFMT(pFmt->eVideoFmt)))

/* Vec SW init macros */
#if BVDC_P_SUPPORT_NEW_SW_INIT
#define BVDC_P_VEC_SW_INIT_OFFSET(core, start, end) \
    (BCHP_VEC_CFG_SW_INIT_##core##_##start - BCHP_VEC_CFG_SW_INIT_##core##_##end)

#define BVDC_P_VEC_SW_INIT(reg, offset, val) \
{ \
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SW_INIT_##reg + (offset)); \
    *pList->pulCurrent++ = BCHP_FIELD_DATA(VEC_CFG_SW_INIT_##reg, INIT, val); \
}
#else
#define BVDC_P_VEC_SW_INIT_OFFSET(core, start, end) \
    (BCHP_VEC_CFG_SW_RESET_##core##_##start - BCHP_VEC_CFG_SW_RESET_##core##_##end)

#define BVDC_P_VEC_SW_INIT(reg, offset, val) \
{ \
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG(); \
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SW_RESET_##reg + (offset)); \
    *pList->pulCurrent++ = BCHP_FIELD_DATA(VEC_CFG_SW_RESET_##reg, RESET, val); \
}
#endif

/* This value needs to be further tuned.
 *
 * There are conflict interest here. Setting this gap to smaller value
 * will enable faster phase adjustment. However this may cause TV resync,
 * especially for composite output. Setting this gap to bigger value
 * will help the resync issue, however the adjustment process will take
 * longer time to finish.
 *
 * Maybe we should extend our API to allow user control this value.
 */
#define BVDC_P_DISPLAY_SKIP_REPEAT_GAP 70

/***************************************************************************
 *  Private Enums
 ***************************************************************************/
/* Contains VEC path selected by VDC */
typedef enum
{
    BVDC_P_eVecPrimary = 0,
    BVDC_P_eVecSecondary,
    BVDC_P_eVecTertiary,
    BVDC_P_eVecBypass0
} BVDC_P_VecPath;

/* DAC Output */
typedef enum
{
    BVDC_P_Output_eYQI=0,        /* SVideo and/or CVBS for NTSC */
    BVDC_P_Output_eYQI_M,        /* SVideo and/or CVBS for NTSC_J */
    BVDC_P_Output_eYUV,          /* SVideo and/or CVBS for Pal */
    BVDC_P_Output_eYUV_M,        /* SVideo and/or CVBS for Pal_M */
    BVDC_P_Output_eYUV_N,        /* SVideo and/or CVBS for Pal_N */
    BVDC_P_Output_eYUV_NC,       /* SVideo and/or CVBS for PAL_NC */
#if BVDC_P_SUPPORT_VEC_SECAM
    BVDC_P_Output_eYDbDr_LDK,    /* SVideo and/or CVBS for SECAM_L/D/K */
    BVDC_P_Output_eYDbDr_BG,     /* SVideo and/or CVBS for SECAM_B/G */
    BVDC_P_Output_eYDbDr_H,      /* SVideo and/or CVBS for SECAM_H */
#endif
    /* Folks, would you please put all of the component and RGB formats below
     * this line. Thanks.
     */
    BVDC_P_Output_eSDYPrPb,      /* SYPrPb, CYPrPb, YPrPb */
    BVDC_P_Output_eSDRGB,        /* SRGB, CRGB, SCRGB, RGB */
    BVDC_P_Output_eHDYPrPb,      /* HDYPrPb */
    BVDC_P_Output_eHDRGB,        /* HDRGB */
    BVDC_P_Output_eHsync,        /* HSYNC */
    BVDC_P_Output_eUnknown,
    BVDC_P_Output_eNone,
    BVDC_P_Output_eMax
} BVDC_P_Output;

#define BVDC_P_DISP_IS_COMPONENT(eOutput) ( \
    ((eOutput) == BVDC_P_Output_eSDYPrPb) || \
    ((eOutput) == BVDC_P_Output_eSDRGB) || \
    ((eOutput) == BVDC_P_Output_eHDYPrPb) || \
    ((eOutput) == BVDC_P_Output_eHDRGB) )

/* Output Filter Types */
typedef enum
{
    BVDC_P_OutputFilter_eHDYPrPb=0,
    BVDC_P_OutputFilter_eHDRGB,
    BVDC_P_OutputFilter_eED,
    BVDC_P_OutputFilter_eSDYPrPb,
    BVDC_P_OutputFilter_eSDRGB,
    BVDC_P_OutputFilter_eYQI,
    BVDC_P_OutputFilter_eYUV,
    BVDC_P_OutputFilter_eSECAM,
    BVDC_P_OutputFilter_eHsync,
    BVDC_P_OutputFilter_eUnknown,
    BVDC_P_OutputFilter_eNone,
    BVDC_P_OutputFilter_eMax
} BVDC_P_OutputFilter;

typedef enum
{
    BVDC_P_ItState_eActive = 0, /* Active, no change necessary */
    BVDC_P_ItState_eNotActive,  /* Initial state. Vecs are not running */
    BVDC_P_ItState_eSwitchMode, /* Mode switch request */
    BVDC_P_ItState_eNewMode     /* Vec switched to new mode. */
} BVDC_P_ItState;

typedef enum
{
    BVDC_P_Dac_Mask_Cvbs   = 0x001,
    BVDC_P_Dac_Mask_Luma   = 0x002,
    BVDC_P_Dac_Mask_Chroma = 0x004,
    BVDC_P_Dac_Mask_Y      = 0x008,
    BVDC_P_Dac_Mask_Pr     = 0x010,
    BVDC_P_Dac_Mask_Pb     = 0x020,
    BVDC_P_Dac_Mask_R      = 0x040,
    BVDC_P_Dac_Mask_G      = 0x080,
    BVDC_P_Dac_Mask_B      = 0x100
} BVDC_P_DAC_Mask;

#define BVDC_P_Dac_Mask_Svideo (BVDC_P_Dac_Mask_Luma | BVDC_P_Dac_Mask_Chroma)
#define BVDC_P_Dac_Mask_YPrPb  (BVDC_P_Dac_Mask_Y | BVDC_P_Dac_Mask_Pr | BVDC_P_Dac_Mask_Pb)
#define BVDC_P_Dac_Mask_RGB    (BVDC_P_Dac_Mask_R | BVDC_P_Dac_Mask_G | BVDC_P_Dac_Mask_B)
#define BVDC_P_Dac_Mask_SD     (BVDC_P_Dac_Mask_Svideo | BVDC_P_Dac_Mask_Cvbs)
#define BVDC_P_Dac_Mask_HD     (BVDC_P_Dac_Mask_YPrPb | BVDC_P_Dac_Mask_RGB)

typedef enum
{
    BVDC_P_TimeStamp_eStart = 0, /* Initial state */
    BVDC_P_TimeStamp_eRul,       /* build RUL for snap shotting time stamp */
    BVDC_P_TimeStamp_eAvail      /* Time stamp is ready for use */
} BVDC_P_TimeStampState;


typedef enum
{
    BVDC_P_Alignment_eInactive = 0,
    BVDC_P_Alignment_eWaitTimeStamp,
    BVDC_P_Alignment_eActive,
    BVDC_P_Alignment_eDone
} BVDC_P_AlignmentState;

typedef enum
{
    BVDC_P_Slave_eInactive = 0,
    BVDC_P_Slave_eEnable,
    BVDC_P_Slave_eConnectSrc,
    BVDC_P_Slave_eAttached
} BVDC_P_SlaveState;

typedef enum
{
    BVDC_P_DisplayResource_eInactive = 0,
    BVDC_P_DisplayResource_eResInactive,
    BVDC_P_DisplayResource_eCreate,
    BVDC_P_DisplayResource_eActivating,
    BVDC_P_DisplayResource_eActive,
    BVDC_P_DisplayResource_eDestroy,
    BVDC_P_DisplayResource_eShuttingdown
} BVDC_P_DisplayResourceState;


/***************************************************************************
 * Display Context
 ***************************************************************************/
typedef struct
{
    const uint32_t   *pulCCBTbl;
    const char       *pchTblName;
} BVDC_P_FormatCCBTbl;

typedef struct
{
    uint32_t                    bPsAgc            : 1; /* Pseudo-Sync/AGC      */
    uint32_t                    bBp               : 1; /* Back Porch           */
    uint32_t                    bCs               : 1; /* Color-Stripes        */
    uint32_t                    bCycl             : 1; /* AGC Cyclic variation */
    uint32_t                    bHamp             : 1; /* H-sync amaplitude reduction outside VBI */
    uint32_t                    bVamp             : 1; /* V-sync amaplitude reduction outside VBI */
    uint32_t                    bRgb              : 1; /* RGB on/off */
} BVDC_P_MacrovisionCtrlBits;

typedef union
{
    /* some events such as format change which will cause core reset and restart
     * need to be handled first.
     */
    struct
    {
        uint32_t                bChan0            : 1; /* setup/destroy analog chan 0 */
        uint32_t                bChan1            : 1; /* setup/destroy analog chan 1 */
        uint32_t                bTiming           : 1; /* new output timing format */
        uint32_t                bAcp              : 1; /* Change in analog copy protection configuration */

        uint32_t                b3DSetting        : 1; /* new 3D setting */
        uint32_t                bDacSetting       : 1; /* new DAC settings */
        uint32_t                bTimeBase         : 1; /* new time base setting */
        uint32_t                bCallback         : 1; /* new callback mask settings */

        uint32_t                bCallbackFunc     : 1; /* new callback function */
        uint32_t                bWidthTrim        : 1; /* 704-sample vs. 720-sample */
        uint32_t                bInputCS          : 1; /* input color space */
        uint32_t                bSrcFrameRate     : 1; /* Source frame rate */

#if (BVDC_P_SUPPORT_RFM_OUTPUT != 0)
        uint32_t                bRfm              : 1; /* new configuration */
#endif
        uint32_t                bHdmiEnable       : 1; /* enable/disable HDMI output */

        uint32_t                bHdmiCsc          : 1; /* new HDMI output CSC matrix settings */
#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
        uint32_t                b656Enable        : 1; /* enable/disable ITU656 output */
#endif

        uint32_t                bMpaaComp         : 1; /* new component path MPAA settings  */
        uint32_t                bMpaaHdmi         : 1; /* new HDMI path MPAA settings */
        uint32_t                bTimeStamp        : 1; /* Take a time stamp for alignment */
        uint32_t                bAlignment        : 1; /* start/stop alignment */

        uint32_t                bHdmiXvYcc        : 1; /* new HDMI XvYcc setting */
        uint32_t                bHdmiSyncOnly     : 1; /* Turn on/off HDMI sync only feature */
        uint32_t                bHdmiSettings     : 1; /* HDMI setting changed */
        uint32_t                bHdmiRmSettings   : 1; /* HDMI RM changed */

        uint32_t                bAspRatio         : 1; /* aspect ratio might changed */
#if (BVDC_P_SUPPORT_STG != 0)
        uint32_t                bStgEnable        : 1; /* Stg enable/disable */
#endif
        uint32_t                bVfFilter         : 1; /* user VF filters */
        uint32_t                bOutputMute       : 1; /* output Mute */

        uint32_t                bMiscCtrl         : 1; /* Combined various dirty bits does not trigger together. */

    } stBits;

    uint32_t aulInts[BVDC_P_DIRTY_INT_ARRAY_SIZE];
} BVDC_P_Display_DirtyBits;

typedef BERR_Code (* BVDC_Display_ValidateSetting)(BVDC_Display_Handle);
typedef void (* BVDC_Display_CopySetting)(BVDC_Display_Handle);
typedef void (* BVDC_Display_ApplySetting)(BVDC_Display_Handle, BVDC_P_ListInfo *, BAVC_Polarity);

typedef struct
{
    BVDC_Display_ValidateSetting validateSettingHandler;
    BVDC_Display_CopySetting     copySettingHandler;
    BVDC_Display_ApplySetting    applySettingHandler;
} BVDC_Display_EventHandler;

typedef struct
{
    uint32_t                    ulSavRemove;
    uint32_t                    ulSavReplicate;
    uint32_t                    ulEavPredict;
} BVDC_P_Display_ShaperSettings;

typedef struct
{
    uint32_t                    ulMin;
    uint32_t                    ulMax;
    BVDC_P_CscCoeffs            stCscCoeffs;
} BVDC_P_DisplayCscMatrix;

#if (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_5)
#include "bvdc_hdmirm_tmds_enum_40nm.h"
#elif ((BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_6) || (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_7))
#include "bvdc_hdmirm_tmds_enum_28nm.h"
#else /* if (BVDC_P_SUPPORT_HDMI_RM_VER == ...) */
#error Unknown/undefined HDMI Rate Manager hardware version
#endif

#if (BVDC_P_SUPPORT_DVI_40NM)
typedef struct BVDC_P_RateInfo
{
    /* Use for searching a matching one! */
#if BFMT_DO_PICK || BDBG_DEBUG_BUILD
    BVDC_P_TmdsClock            eTmdsClock;
#endif
    BFMT_ClockMod               eClockMod;
    uint32_t                    ulDenominator;
    uint32_t                    ulNumerator;
    uint32_t                    ulOffset;
    uint32_t                    ulDenominatorAdj;
    uint32_t                    ulNumeratorAdj;
    uint32_t                    ulOffsetAdj;
    uint32_t                    ulSampleInc;
    uint32_t                    ulShift;
    uint32_t                    ulRmDiv;
    uint32_t                    ulVcoRange;
    uint32_t                    ulPxDiv;
    uint32_t                    ulKP;
    uint32_t                    ulKI;
    uint32_t                    ulKA;
    const char                 *pchRate;
    const char                 *pchRateAdj;
} BVDC_P_RateInfo;

#elif (BVDC_P_SUPPORT_DVI_28NM)
typedef struct BVDC_P_RateInfo
{
    /* Use for searching a matching one! */
#if BFMT_DO_PICK || BDBG_DEBUG_BUILD
    BVDC_P_TmdsClock            eTmdsClock;
#endif
    BFMT_ClockMod               eClockMod;
    uint32_t                    ulDenominator;
    uint32_t                    ulNumerator;
    uint32_t                    ulOffset;
    uint32_t                    ulDenominatorAdj;
    uint32_t                    ulNumeratorAdj;
    uint32_t                    ulOffsetAdj;
    uint32_t                    ulSampleInc;
    uint32_t                    ulShift;
    uint32_t                    ulRmDiv;
    uint32_t                    ulVcoRange;
    uint32_t                    ulPxDiv;
    uint32_t                    ulVcoSel;
    uint32_t                    ulVcoGain;
    uint32_t                    ulIcp;
    uint32_t                    ulRz;
    uint32_t                    ulCz;
    uint32_t                    ulCp;
    uint32_t                    ulRp;
    uint32_t                    ulCp1;
    const char                 *pchRate;
    const char                 *pchRateAdj;
} BVDC_P_RateInfo;

#else
typedef struct BVDC_P_RateInfo
{
    /* Use for searching a matching one! */
#if BFMT_DO_PICK || BDBG_DEBUG_BUILD
    BVDC_P_TmdsClock            eTmdsClock;
#endif
    BFMT_ClockMod               eClockMod;
    uint32_t                    ulDenominator;
    uint32_t                    ulNumerator;
    uint32_t                    ulOffset;
    uint32_t                    ulDenominatorAdj;
    uint32_t                    ulNumeratorAdj;
    uint32_t                    ulOffsetAdj;
    uint32_t                    ulSampleInc;
    uint32_t                    ulShift;
    uint32_t                    ulRmDiv;
    uint32_t                    ulVcoRange;
    uint32_t                    ulPxDiv;
    uint32_t                    ulInputPreDiv;
    uint32_t                    ulKVcoXs;
    const char                 *pchRate;
    const char                 *pchRateAdj;
} BVDC_P_RateInfo;

#endif

#if BVDC_P_SUPPORT_MHL
typedef struct BVDC_P_PxlFreqMhlFreqPair
{
    uint64_t ulPixelClkRate;
    uint64_t ulMhlClkRate;
} BVDC_P_PxlFreqMhlFreqPair;
#endif

typedef struct
{
    bool                        bWidthTrimmed;
    bool                        bFullRate;
    bool                        bBypassDviCsc;
    BVDC_P_MatrixCoeffs         eCmpMatrixCoeffs;
} BVDC_P_Display_SrcInfo;

typedef struct
{
    BVDC_Display_HdmiSettings    stSettings;
    BAVC_HDMI_BitsPerPixel       eHdmiColorDepth;      /* 24, 30, 36, or 48 (future) */
    BAVC_HDMI_PixelRepetition    eHdmiPixelRepetition; /* none, 1x, or 4x */
    BFMT_VideoFmt                eHDMIFormat;          /* 4kx2k upscale support */
    BFMT_VideoFmt                eMatchingFormat;
} BVDC_P_Display_HdmiSettings;

/*
 * Analog channel state and resources acquired
 */
typedef struct
{
    uint32_t                    ulId;
    bool                        bEnable;    /* on or off */
    bool                        bTearDown;  /* tearing down flag */
    BVDC_P_DisplayResourceState eState;     /* Analog channel state */
    uint32_t                    ulIt;
    uint32_t                    ulVf;
    uint32_t                    ulSecam;
    uint32_t                    ulSecam_HD;
    uint32_t                    ulPrevSecam;
    uint32_t                    ulPrevSecam_HD;
    uint32_t                    ulSdsrc;
    uint32_t                    ulHdsrc;
    uint32_t                    ulPrevSdsrc;
    uint32_t                    ulPrevHdsrc;
    uint32_t                    ulDac_0;
    uint32_t                    ulDac_1;
    uint32_t                    ulDac_2;

    /* offsets */
    uint32_t                    ulItRegOffset;       /* IT instance offset */
    uint32_t                    ulRmRegOffset;       /* RM instance offset */
    uint32_t                    ulVfRegOffset;       /* VF instance offset */
    uint32_t                    ulCscRegOffset;      /* CSC instance offset */
    uint32_t                    ulSecamRegOffset;    /* SECAM instance offset */
    uint32_t                    ulSmRegOffset;       /* SM instance offset */
    uint32_t                    ulSdsrcRegOffset;    /* SDSRC instance offset */
    uint32_t                    ulHdsrcRegOffset;    /* HDSRC instance offset */

    /* SW INIT offset */
    uint32_t                    ulItSwInitOffset;
    uint32_t                    ulVfSwInitOffset;
    uint32_t                    ulSecamSwInitOffset;
    uint32_t                    ulSecamHDSwInitOffset;
    uint32_t                    ulSdsrcSwInitOffset;
    uint32_t                    ulHdsrcSwInitOffset;

    BVDC_P_DisplayCscMatrix     stCscMatrix;

    /* VF filters */
    const uint32_t             *apVfFilter[BVDC_P_VEC_CH_NUM];
    uint32_t                    vfMisc;

#ifdef BCHP_PWR_RESOURCE_VDC_DAC
    uint32_t                    ulDacPwrAcquire;
    uint32_t                    ulDacPwrRelease;
#endif
} BVDC_P_DisplayAnlgChan;

typedef struct
{
    BVDC_P_MatrixCoeffs         eCmpMatrixCoeffs;
    BVDC_P_Output               eAnlg_0_OutputColorSpace;
    BVDC_P_Output               eAnlg_1_OutputColorSpace;
    uint32_t                    ulAnlgChan0Mask;
    uint32_t                    ulAnlgChan1Mask;

    BVDC_DacOutput              aDacOutput[BVDC_P_MAX_DACS];
    uint32_t                    ulHdmi;
    BAVC_MatrixCoefficients     eHdmiOutput;
    const BFMT_VideoInfo       *pFmtInfo;
    BFMT_VideoInfo              stCustomFmt;
    uint32_t                    ulVertFreq;
    uint32_t                    ulHeight;

    /* User Dvo's CSC */
    bool                        bUserCsc;
    uint32_t                    ulUserShift;
    int32_t                     pl32_Matrix[BVDC_CSC_COEFF_COUNT];

    /* Misc shares bits for 656/Hdmi/Rfm */
    BVDC_P_VecPath              e656Vecpath;        /* 656 path output */
    BVDC_P_VecPath              eHdmiVecpath;       /* Prim/Sec, Bypass=off both */
    BVDC_P_VecPath              eRfmVecpath;        /* Prim/Sec, Bypass=off both */

    bool                        bFullRate;
    BAVC_Timebase               eTimeBase;          /* timebase for this display */
    BVDC_P_Display_HdmiSettings stHdmiSettings;

    uint32_t                    uiNumDacsOn;        /* #Dacs enabled for this display */

    /* RFM */
    BVDC_RfmOutput              eRfmOutput;
    uint32_t                    ulRfmConst;         /* constant value to RF port */

    bool                        bEnableHdmi;
    bool                        bXvYcc;             /* hdmi XvYcc output */
    bool                        bEnable656;

    uint32_t                    aulHdmiDropLines[BFMT_VideoFmt_eMaxCount];

    /* MPAA decimation bit field */
    uint32_t                    aulEnableMpaaDeci[BVDC_MpaaDeciIf_eUnused];

    /* Generic callback */
    BVDC_CallbackFunc_isr       pfGenericCallback;
    void                       *pvGenericParm1;
    int                         iGenericParm2;
    BAVC_VdcDisplay_Info        stRateInfo;
    BVDC_Display_CallbackSettings   stCallbackSettings;

    /*
     * Copy protection state
     */

    /* MV N0 control bits */
    BVDC_P_MacrovisionCtrlBits  stN0Bits;
    /* Macrovision type */
    BVDC_MacrovisionType        eMacrovisionType;

#if DCS_SUPPORT
    /* DCS type */
    BVDC_DCS_Mode               eDcsMode;
#endif

    /* 704 vs. 720 samples */
    bool                        bWidthTrimmed;

    /* Force vec to run at 59.94Hz, 29.97Hz, etc. */
    BVDC_Mode                   eDropFrame;

    /* PR28836: DVO H/V/De sync polarity. */
    BVDC_Display_DvoSettings    stDvoCfg;

    /* Display alignment settings */
    BVDC_Display_Handle             hTargetDisplay;
    BVDC_Display_AlignmentSettings  stAlignCfg;

    /* color adjustment attributes */
    int32_t                     lDvoAttenuationR;
    int32_t                     lDvoAttenuationG;
    int32_t                     lDvoAttenuationB;
    int32_t                     lDvoOffsetR;
    int32_t                     lDvoOffsetG;
    int32_t                     lDvoOffsetB;

    bool                        bCvbs;
    bool                        bSvideo;
    bool                        bHsync;
    bool                        bRgb;
    bool                        bYPrPb;
    bool                        abOutputMute[BVDC_DisplayOutput_e656 + 1];

    bool                        bMultiRateAllow;

    bool                        bErrorLastSetting;

    /* display aspect ratio */
    BFMT_AspectRatio            eAspectRatio;
    uint32_t                    uiSampleAspectRatioX;
    uint16_t                    uiSampleAspectRatioY;
    BVDC_P_ClipRect             stAspRatRectClip;

    /* STG mode RealTime/ NonRealTime*/
    bool                        bEnableStg;
    bool                        bStgNonRealTime;
#if (BVDC_P_SUPPORT_STG)
    uint32_t                    ulResolutionRampCount;
#endif
#if (BVDC_P_SUPPORT_VIP)
    BMMA_Heap_Handle            hVipHeap;
    BVDC_VipMemConfigSettings   stVipMemSettings;
#endif
    uint32_t                    ulStcSnapshotLoAddr;
    uint32_t                    ulStcSnapshotHiAddr;
#if (BVDC_P_SUPPORT_IT_VER >= 2)
    uint32_t                    ulTriggerModuloCnt;
#endif
    bool                        bBypassVideoProcess;
    BVDC_Mode                   eBarDataMode;
    BAVC_BarDataType            eBarDataType;
    uint32_t                    ulTopLeftBarData;
    uint32_t                    ulBotRightBarData;

    BFMT_Orientation            eOrientation;
    BVDC_3dSourceBufferSelect   e3dSrcBufSel;

    /* user VF filters */
    bool                        abUserVfFilterCo[BVDC_P_VEC_CH_NUM];
    bool                        abUserVfFilterCvbs[BVDC_P_VEC_CH_NUM];
    uint32_t                    aulUserVfFilterCoSumOfTaps[BVDC_P_VEC_CH_NUM];
    uint32_t                    aulUserVfFilterCvbsSumOfTaps[BVDC_P_VEC_CH_NUM];
    uint32_t                    aaulUserVfFilterCo[BVDC_P_VEC_CH_NUM][BVDC_P_CHROMA_TABLE_SIZE];
    uint32_t                    aaulUserVfFilterCvbs[BVDC_P_VEC_CH_NUM][BVDC_P_CHROMA_TABLE_SIZE];

    /* Artificial vsync */
    bool                        bArtificialVsync;
    uint32_t                    ulArtificialVsyncRegAddr;
    uint32_t                    ulArtificialVsyncMask;

    /* 4kx2k support */
    const BFMT_VideoInfo       *pHdmiFmtInfo;
    bool                        bHdmiFmt;         /* using HDMI format? */
    bool                        bHdmiRmd;         /* running with RMD */

    /* dirty bits */
    BVDC_P_Display_DirtyBits    stDirty;
} BVDC_P_DisplayInfo;

/*
 * DVI channel state and resources acquired
 */
typedef struct
{
    bool              bEnable;          /* on or off */
    uint32_t          ulDvi;
    uint32_t          bMhlMode;         /* indicates whether HDMI PHY is in HDMI or MHL mode */
    uint32_t          ulDviRegOffset;      /* DVI instace offset */
    uint32_t          ulDvpRegOffset;      /* DVP instance offset */

    /* SW INIT offset */
    uint32_t          ulDtgSwInitOffset;
    uint32_t          ulCscSwInitOffset;
    uint32_t          ulDvfSwInitOffset;
    uint32_t          ulFcSwInitOffset;
    uint32_t          ulMiscSwInitOffset;
} BVDC_P_DisplayDviChan;

/*
 * 656 channel state and resources acquired
 */
typedef struct
{
    bool              bEnable;          /* on or off */
    uint32_t          ul656;
} BVDC_P_Display656Chan;

/*
* STG channel state and resources acquired
*/
typedef struct
{
    bool                      bEnable;          /*on or off */
    bool                      bStgNonRealTime;  /* real time mode or not*/
    bool                      bModeSwitch;      /* switch between RT and NRT */
    uint32_t                  ulStg;
    uint32_t                  ulMBoxAddr;       /* Mailbox physical address for ViCE2*/
    uint32_t                  ulViceChannelIdx; /* channel index on ViCE core */
    uint32_t                  ulViceCoreIdx;    /* ViCE core Idx */
    uint32_t                  ulChannelPerCore; /* channel number per core */
    uint32_t                  ulViceIntAddr;    /* Vice interrupt Addr*/
    uint32_t                  ulViceIntMask;    /* Vice interrupt Mask */
#ifdef BVDC_P_SUPPORT_RDC_STC_FLAG
    uint32_t                  ulStcFlag;        /* STC flag to trigger encode STC snapshot and NRT decode STC increment */
#endif
} BVDC_P_DisplayStgChan;

/*
 * MPAA state and resources acquired
 */
typedef struct
{
    BVDC_P_DisplayResourceState eState;  /* MPAA state */
    uint32_t                    ulHwId;
} BVDC_P_DisplayMpaa;

#if BVDC_P_SUPPORT_STG
typedef struct BVDC_P_StgViceWireInfo
{
    uint32_t               ulStgId;
    uint32_t               ulViceCoreId;
    uint32_t               ulViceChannelId;
} BVDC_P_StgViceWireInfo;
#endif

typedef struct BVDC_P_DisplayContext
{
    BDBG_OBJECT(BVDC_DSP)

    BVDC_P_DisplayInfo          stNewInfo;           /* new(to-be apply) display info */
    BVDC_P_DisplayInfo          stCurInfo;           /* current(hw-applied) display info */
    BVDC_P_Display_DirtyBits    stPrevDirty;         /* Previous dirty bits. In case
                                                      * last RUL didn't get executed, we will
                                                      * try again.
                                                      */

    BVDC_DisplayId              eId;                 /* might be different from cmp id */
    BVDC_P_State                eState;              /* Context state. */
    BVDC_Handle                 hVdc;                /* From which main VDC handle */
    BVDC_P_ItState              eItState;            /* Current Vec state */
    BVDC_Compositor_Handle      hCompositor;         /* Conntected to compositor */
    uint32_t                    ulRdcVarAddr;        /* Temp RDC var addr use for format change. */
    uint32_t                    ulStgRegOffset;      /* Stg instance offset */

    bool                        bHdCap;              /* is HD capable */
    bool                        bSecamCap;           /* is Secam capable */
    bool                        bDacProgAlone;       /* need to program analog again with DACs dirty */

    /* Display channels */
    BVDC_P_DisplayDviChan       stDviChan;           /* DVI channel */
#if BVDC_P_SUPPORT_SEAMLESS_ATTACH
    BVDC_P_SlaveState           eDviSlaveState;      /* State used when attaching DVI slave to master path */
#endif
    BVDC_P_DisplayResourceState eDviState;           /* DVI channel state */
    BVDC_P_Display656Chan       st656Chan;           /* 656 channel */
#if BVDC_P_SUPPORT_SEAMLESS_ATTACH
    BVDC_P_SlaveState           e656SlaveState;      /* State used when attaching ITU656 slave to master path */
#endif
    BVDC_P_DisplayResourceState e656State;           /* 656 channel state */

#if BVDC_P_SUPPORT_SEAMLESS_ATTACH
    BVDC_P_SlaveState           eStgSlaveState;      /* State used when attaching ITU656 slave to master path */
#endif
    BVDC_P_DisplayResourceState eStgState;           /* 656 channel state */
    BVDC_P_DisplayStgChan       stStgChan;           /* STG channel */
    BVDC_P_DisplayAnlgChan      stAnlgChan_0;        /* Analog channel 0 */
    BVDC_P_DisplayAnlgChan      stAnlgChan_1;        /* Analog channel 1 */
    bool                        bAnlgEnable;         /* Ananlog master */
    BVDC_P_DisplayResourceState eRfmState;           /* Rfm channel state */

    /* IT VEC status */
    uint32_t                    ulItLctrReg;

    /* Master timing generator among all the channels.
     * Other channels should slave to it.
     */
    BVDC_DisplayTg              eMasterTg;
    BRDC_Trigger                eTopTrigger;
    BRDC_Trigger                eBotTrigger;

    /* DVO, and Main CSC */
    BVDC_P_DisplayCscMatrix     stDvoCscMatrix;
    BVDC_P_DisplayCscMatrix     st656CscMatrix;

    /* Event to nofify that changes has been applied to hardware. */
    BKNI_EventHandle            hAppliedDoneEvent;
    bool                        bSetEventPending;

    bool                        bRateManagerUpdated;

    const BFMT_VideoInfo         *pStgFmtInfo;          /* STG fmt setting, possibly buffer delay */
    /* MPAA decimation supported interface port mask */
    uint32_t                    aulMpaaDeciIfPortMask[BVDC_MpaaDeciIf_eUnused];
    BVDC_P_DisplayMpaa          stMpaaHdmi; /* mpaa state in hdmi chan */
    BVDC_P_DisplayMpaa          stMpaaComp; /* state in component chan */

    /* Is this a bypass display? which means no VEC analog output. */
    bool                        bIsBypass;

    bool                        bCmpBypassDviCsc;
    /* Internal VDC or App handed down. */
    BVDC_Heap_Handle            hHeap;

    /* Specific flavor of 480P output */
    bool bArib480p;

    /* Option to modify sync on 720P, 1080I, 1080P YPrPb video */
    bool bModifiedSync;

    /* Game mode tracking window */
    BVDC_Window_Handle          hWinGameMode;
    const uint32_t             *pRmTable;  /* VEC RM */
    bool                        bRmAdjusted;

#if (!BVDC_P_USE_RDC_TIMESTAMP)
    /* alignment Timestamps */
    BTMR_TimerHandle            hTimer;
    BTMR_TimerRegisters         stTimerReg;
    uint32_t                    ulScratchTsAddr;
#endif

#if (BVDC_P_STG_RUL_DELAY_WORKAROUND)
    uint32_t                    ulScratchDummyAddr;
#endif

#if BVDC_P_SUPPORT_STG
    uint32_t                    ulStgTriggerToBeArmed;
#if (BDBG_DEBUG_BUILD)
    uint32_t                    ulStgTriggerCount;/* to measure ignore ratio for NRT performance */
    uint32_t                    ulStgIgnoreCount;
#endif
#if BVDC_P_SUPPORT_VIP
    BVDC_P_Vip_Handle           hVip;
#endif
    BFMT_VideoFmt               eStgRampFmt; /* STG final rampup format */
    BFMT_VideoInfo              stStgRampCustomFmt; /* the STG final rampup custom format */
    BVDC_P_Rect                 astStgRampWinSclOut[BVDC_P_MAX_WINDOW_COUNT], astStgRampWinDst[BVDC_P_MAX_WINDOW_COUNT];
#endif

    BAVC_Polarity               eTimeStampPolarity;
    BVDC_P_TimeStampState       eTimeStampState;
    BVDC_P_AlignmentState       eAlignmentState;
    uint32_t                    ulAlignSlaves;
    bool                        bAlignAdjusting;

    /* Dither */
    BVDC_P_DitherSetting        stCscDither;
    BVDC_P_DitherSetting        stDviDither;
    BVDC_P_DitherSetting        st656Dither;
    BVDC_Display_CallbackData   stCallbackData;
    bool                        bCallbackInit;

    /* display pixel aspect ratio */
    uintAR_t                    ulPxlAspRatio;     /* PxlAspR_int.PxlAspR_frac */
    uint32_t                    ulPxlAspRatio_x_y; /* PxlAspR_x<<16 | PxlAspR_y */

#if DCS_SUPPORT
    struct BVDC_P_DCS_UpdateInfo stDcsConfirmed;
    struct BVDC_P_DCS_UpdateInfo stDcsRequested;
    bool                         bDcsNeedsLock;
    uint32_t                     ulDcsSequence;
#if 0
    /* Debug code */
    uint32_t                     ulDcsScratch[4];
#endif
#endif

#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY0
    uint32_t                     ulHdmiPwrAcquire;
    uint32_t                     ulHdmiPwrRelease;
    uint32_t                     ulHdmiPwrId;
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_STG0
    uint32_t                     ulStgPwrAcquire;
    uint32_t                     ulStgPwrRelease;
    uint32_t                     ulStgPwrId;
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_656_OUT
    uint32_t                     ul656PwrAcquire;
    uint32_t                     ul656PwrRelease;
#endif
#ifdef BCHP_PWR_RESOURCE_VDC_DAC
    uint32_t                     ulDacPwrAcquire;
    uint32_t                     ulDacPwrRelease;
#endif

    /* For debug logs */
    uint32_t                    ulVsyncCnt;        /* Vysnc heartbeat */
} BVDC_P_DisplayContext;

/***************************************************************************
 * Private tables
 ***************************************************************************/
extern const BVDC_Display_EventHandler  BVDC_P_astDisplayEventHndlTbl[];
extern const unsigned int BVDC_P_astDisplayEventHndlTblSize;


/***************************************************************************
 * Display private functions
 ***************************************************************************/
BERR_Code BVDC_P_Display_Create
    ( BVDC_P_Context                  *pVdc,
      BVDC_Display_Handle             *phDisplay,
      BVDC_DisplayId                   eId);

void BVDC_P_Display_Destroy
    ( BVDC_Display_Handle              hDisplay );

void BVDC_P_Display_Init
    ( BVDC_Display_Handle              hDisplay );

void BVDC_P_Vec_BuildRul_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity );

BERR_Code BVDC_P_Display_ValidateChanges
    ( BVDC_Display_Handle              ahDisplay[] );

void BVDC_P_Display_ApplyChanges_isr
    ( BVDC_Display_Handle              hDisplay);

void BVDC_P_Display_AbortChanges
    ( BVDC_Display_Handle              hDisplay);

bool BVDC_P_Display_FindDac_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_DacOutput                   eDacOutput);

/* Helper functions. */
#if (BSTD_CPU_ENDIAN == BSTD_ENDIAN_BIG)
#define BVDC_P_DISPLAY_DIRTY_MASK_SHIFT(idx) (1 << (BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE - 1 - (idx % BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE)))
#else
#define BVDC_P_DISPLAY_DIRTY_MASK_SHIFT(idx) (1 << idx % BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE)
#endif

#define BVDC_P_DISPLAY_IS_BIT_DIRTY(pDirty, iIdx)    ((*(pDirty)).aulInts[iIdx/BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE] & BVDC_P_DISPLAY_DIRTY_MASK_SHIFT(iIdx))
#define BVDC_P_DISPLAY_CLEAR_DIRTY_BIT(pDirty, iIdx) ((*(pDirty)).aulInts[iIdx/BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE] &= ~(BVDC_P_DISPLAY_DIRTY_MASK_SHIFT(iIdx)))
#define BVDC_P_DISPLAY_SET_DIRTY_BIT(pDirty, iIdx)   ((*(pDirty)).aulInts[iIdx/BVDC_P_DIRTY_INT_ARRAY_ELEMENT_SIZE] |= BVDC_P_DISPLAY_DIRTY_MASK_SHIFT(iIdx))

void BVDC_P_ResetAnalogChanInfo
    (BVDC_P_DisplayAnlgChan *pstChan);

BERR_Code BVDC_P_AllocITResources
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_DisplayId                   eDisplayId,
      BVDC_P_DisplayAnlgChan          *pstChan,
      uint32_t                         ulIt );

BERR_Code BVDC_P_AllocAnalogChanResources
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_DisplayId                   eDisplayId,
      BVDC_P_DisplayAnlgChan          *pstChan,
      bool                             bHdMust,
      bool                             bHdRec,
      bool                             bSecamCap );

void BVDC_P_FreeITResources_isr
    (BVDC_P_Resource_Handle            hResource,
     BVDC_P_DisplayAnlgChan           *pstChan );

void BVDC_P_FreeAnalogChanResources_isr
    (BVDC_P_Resource_Handle            hResource,
     BVDC_P_DisplayAnlgChan           *pstChan);

void BVDC_P_Reset656ChanInfo
    (BVDC_P_Display656Chan *pstChan);

#if BVDC_P_SUPPORT_ITU656_OUT
BERR_Code BVDC_P_Alloc656ChanResources_isr
    (BVDC_P_Resource_Handle hResource,
     BVDC_DisplayId eDisplayId,
     BVDC_P_Display656Chan *pstChan,
     uint32_t eSrcId);

void BVDC_P_Free656ChanResources_isr
    (BVDC_P_Resource_Handle hResource,
     BVDC_Display_Handle    hDisplay);
#endif

void BVDC_P_ResetDviChanInfo
    (BVDC_P_DisplayDviChan *pstChan);

BERR_Code BVDC_P_AllocDviChanResources_isr
    (BVDC_P_Resource_Handle hResource,
     BREG_Handle hRegister,
     BVDC_DisplayId eDisplayId,
     uint32_t       ulHdmi,
     BVDC_P_DisplayDviChan *pstChan,
     uint32_t eSrcId);

void BVDC_P_FreeDviChanResources_isr
    (BVDC_P_Resource_Handle hResource,
     BVDC_P_DisplayDviChan *pstChan);

BERR_Code BVDC_P_AllocDacResources
    ( BVDC_P_Resource_Handle     hResource,
      BVDC_P_DisplayAnlgChan    *pstChan,
      uint32_t                   ulDacId );

BERR_Code BVDC_P_AllocMpaaResources
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_DisplayId                   eDisplayId,
      BVDC_P_DisplayMpaa              *pstChan );

void BVDC_P_FreeMpaaResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_P_DisplayMpaa              *pstChan );

void  BVDC_P_Display_SetSourceInfo_isr
    ( BVDC_Display_Handle  hDisplay,
      const BVDC_P_Display_SrcInfo *pSrcInfo );

uint32_t BVDC_P_GetPosSyncValue_isr
    ( BVDC_P_DisplayContext     *pDisplay,
      uint32_t                 **ppulRul );

#if (BVDC_P_SUPPORT_STG)
BERR_Code BVDC_P_AllocStgChanResources_isr
    ( BVDC_P_Resource_Handle           hResource,
       BVDC_Display_Handle              hDisplay);
void BVDC_P_FreeStgChanResources_isr
    (BVDC_P_Resource_Handle hResource,
     BVDC_Display_Handle              hDisplay);
void BVDC_P_ResetStgChanInfo
    (BVDC_P_DisplayStgChan            *pstStgChan);
void BVDC_P_Display_EnableSTGTriggers_isr
    ( BVDC_Display_Handle              hDisplay,
    bool                             bEnable );

BERR_Code BVDC_P_Display_Validate_Stg_Setting
    ( BVDC_Display_Handle              hDisplay );

void BVDC_P_Display_Copy_Stg_Setting_isr
    ( BVDC_Display_Handle              hDisplay );

void BVDC_P_Display_Apply_Stg_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity );

void BVDC_P_ProgrameStgMBox_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity );

void BVDC_P_ProgramStgChan_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList );

void BVDC_P_STG_Build_RM_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList  );

void BVDC_P_SetupStg_isr
      (uint32_t                       ulRegOffset,
       BVDC_P_ListInfo               *pList );

void BVDC_P_TearDownStgChan_isr
      (BVDC_Display_Handle            hDisplay,
       BVDC_P_ListInfo                *pList );

void BVDC_P_ConnectStgSrc_isr
    (BVDC_Display_Handle             hDisplay,
     BVDC_P_ListInfo                *pList );
void BVDC_P_Stg_Init_isr
    ( BVDC_Display_Handle              hDisplay);
void BVDC_P_AcquireStgPwr
    ( BVDC_Display_Handle              hDisplay );
#endif
#if (BVDC_P_STG_RUL_DELAY_WORKAROUND)
void BVDC_P_STG_DelayRUL_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      bool                             bMadr);
#endif

void BVDC_P_Macrovision_GetNegSyncValue_isr
    ( BVDC_P_DisplayInfo              *pDispInfo,
      BVDC_P_Output                    eOutputColorSpace,
      bool                             bDacOutput_Green_NoSync,
      uint32_t*                        ulRegVal,
      uint32_t*                        ulRegValEx);

uint32_t BVDC_P_GetFmtAdderValue_isr
    ( BVDC_P_DisplayInfo              *pDispInfo );

BERR_Code BVDC_P_ValidateMacrovision
    ( BVDC_P_DisplayContext           *pDisplay );

void BVDC_P_Vec_Init_Misc_isr
    ( BVDC_P_Context                  *pVdc );

void BVDC_P_Vec_Update_OutMuxes_isr
    ( BVDC_P_Context                  *pVdc );

void BVDC_P_Vec_BuildVsync_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity );

void BVDC_P_Display_EnableTriggers_isr
    ( BVDC_Display_Handle              hDisplay,
      bool                             bEnable );

void BVDC_P_ResetVec
    ( BVDC_P_Context                  *pVdc );

void BVDC_P_Display_Alignment_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList);

void BVDC_P_Display_GetAnlgChanByOutput_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayInfo              *pDispInfo,
      BVDC_DisplayOutput               eDisplayOutput,
      BVDC_P_DisplayAnlgChan         **pstChan );

void BVDC_P_CableDetect_isr
    ( BVDC_Display_Handle              hDisplay );

BERR_Code BVDC_P_Display_GetRasterLineNumber
    ( BVDC_Display_Handle              hDisplay,
      uint32_t                        *pulRasterLineNumber );

BERR_Code BVDC_P_Display_GetFieldPolarity_isr
    ( BVDC_Display_Handle              hDisplay,
      uint32_t                       **ppulRulCur,
      BAVC_Polarity                    eFieldPolarity );

BERR_Code BVDC_P_GetVfFilterSumOfTapsBits_isr
    ( const BVDC_P_DisplayInfo        *pDispInfo,
      BVDC_DisplayOutput               eDisplayOutput,
      uint32_t                        *pulSumOfTapsBits,
      bool                            *pbOverride);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BVDC_DISPLAY_PRIV_H__ */
/* End of file. */
