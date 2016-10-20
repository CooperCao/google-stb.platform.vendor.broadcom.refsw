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

#include "bstd.h"
#include "bdbg.h"
#include "bfmt.h"

/* Note: Tricky here!  bavc.h needs bchp_gfd_x.h defininitions.
 * The check here is to see if chips has more than one gfx feeder. */
#include "bchp_gfd_0.h"
#include "bchp_gfd_1.h"

#include "bavc.h"
#include "bchp.h"
#include "bvdc_compositor_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_common_priv.h"
#include "bvdc_display_priv.h"
#include "bchp_common.h"

#if BVDC_P_SUPPORT_DTG_RMD
#include "bchp_dvi_dtg_rm_0.h"
#endif
#if BVDC_P_SUPPORT_DSCL
#include "bchp_dscl_0.h"
#endif

#ifdef BCHP_DVI_FC_0_REG_START
#include "bchp_dvi_fc_0.h"
#endif

#if (BVDC_P_SUPPORT_VEC_GRPD)
#include "bchp_grpd_0.h"
#endif

#if BVDC_P_SUPPORT_MHL
#include "bchp_mpm_cpu_ctrl.h"
#define BVDC_P_FORCED_MHL_MODE 0 /* for debug purposes only */
#endif

#ifdef BCHP_PWR_SUPPORT
#include "bchp_pwr.h"
#endif
#if BCHP_DVI_MISC_0_REG_START
#include "bchp_dvi_misc_0.h"
#endif
#define BVDC_P_VF_EAV_PRDCT              3
#define BVDC_P_VF_THRESH                 1  /* HW reset value */
#define BVDC_P_VF_ENABLE                 1  /* HW reset value */

#if (BCHP_CHIP == 7422) || ((BCHP_CHIP == 7425) && (BCHP_VER < BCHP_VER_B0))
#define BVDC_P_VEC_HW7425_475_WORK_AROUND  1
#else
#define BVDC_P_VEC_HW7425_475_WORK_AROUND  0
#endif

/* HDMI & VEC RM's tracking range. */
#if (BVDC_P_SUPPORT_HDMI_RM_VER > BVDC_P_HDMI_RM_VER_6)
#define BVDC_P_TRACKING_RANGE_DEFAULT    (BCHP_RM_0_CONTROL_TRACKING_RANGE_ZERO_to_244)
/* SW7445-1678: 28nm only support up 5000ppm. */
#define BVDC_P_TRACKING_RANGE_GAMEMODE   (BCHP_RM_0_CONTROL_TRACKING_RANGE_ZERO_to_3904)
#else
#define BVDC_P_TRACKING_RANGE_DEFAULT    (BCHP_RM_0_CONTROL_TRACKING_RANGE_ZERO_to_244)
#define BVDC_P_TRACKING_RANGE_GAMEMODE   (BCHP_RM_0_CONTROL_TRACKING_RANGE_ZERO_to_15625)
#endif

BDBG_MODULE(BVDC_DISP);
BDBG_FILE_MODULE(BVDC_DISP_CSC);
BDBG_FILE_MODULE(BVDC_CMP_SIZE);
BDBG_FILE_MODULE(BVDC_DVI_NLCSC);

/* A forward definition for convenience */
static void BVDC_P_Display_Copy_Acp_isr(
    BVDC_Display_Handle              hDisplay );

/****************************************************************
 *                   VEC Helper Functions                       *
 ****************************************************************/
#if (BVDC_P_SUPPORT_VEC_GRPD)
#define BVDC_P_DLAY_FILTER_COUNT BVDC_P_REGS_ENTRIES(GRPD_0_GRP00,  GRPD_0_GRP19 )
#define BVDC_P_TRAP_FILTER_COUNT BVDC_P_REGS_ENTRIES(GRPD_0_TRAP00, GRPD_0_TRAP23)

static const struct
{
    uint32_t aulDlay[BVDC_P_DLAY_FILTER_COUNT];
    uint32_t aulTrap[BVDC_P_TRAP_FILTER_COUNT];

} s_aulGrpdFilters[2] =
{
    /* NTSC */
    {
        {
            0x00000180, 0xf8d010e0, 0xe2202920, 0xd6601650, 0x0a80e140,
            0x10c011a0, 0xe1e0fd80, 0x3e904110, 0x1600fbb0, 0xfbc00110,
            0x01a0ffc0, 0xff200000, 0x00800010, 0xffc00010, 0x00500020,
            0xfff0ffe0, 0x00000000, 0x00000000, 0x00000000, 0x00000000
        },
        {
            0xFFF40014, 0xFFF8FFF4, 0x00180000, 0xFFE4001C, 0x0010FFD0,
            0x00140030, 0xFFC0FFF8, 0x0058FFC0, 0xFFC8007C, 0xFFE0FF80,
            0x00900028, 0xFF300074, 0x00A0FEF4, 0x00180134, 0xFEF0FF68,
            0x01C4FF50, 0xFE68021C, 0x0044FD30, 0x01F401F4, 0xFBE800DC,
            0x04B8FAC8, 0xFDD009FC, 0xFA04F254, 0x25F44F14,
        }
    },

    /* PAL */
    {
        {
            0xffc0ffb0, 0x01f0fa80, 0x0b10ee50, 0x1710e7c0, 0x1310f820,
            0xfab00e90, 0xf21003e0, 0x0ba0eeb0, 0x09101480, 0xe9401180,
            0x5a602840, 0xf9e0f530, 0xfcd00050, 0x0130fc60, 0x0560f930,
            0x0680fb20, 0x02a0ff60, 0xff200180, 0xfe700110, 0xff500040,
        },
        {
            0x000c0004, 0xffec0028, 0xffc4003c, 0xffd80000, 0x002cffb8,
            0x0044ffe8, 0xffd80064, 0xff8c004c, 0x000cff90, 0x00b0ff64,
            0x00340064, 0xff1c0100, 0xff64ffd4, 0x0100fe88, 0x013cffb0,
            0xff0c01f4, 0xfde4012c, 0x0098fd98, 0x035cfd4c, 0x006402cc,
            0xfa9405e8, 0xfce0fcf4, 0x0b74ebf0, 0x1a806320,
        }
    },
};
#endif /* (BVDC_P_SUPPORT_VEC_GRPD) */

/* make sure matches with BAVC_MatrixCoefficients */
static const struct
{
    BAVC_MatrixCoefficients eAvcCs;
    const char *pcAvcCsStr;

} stAVC_MatrixCoefficient_InfoTbl[] =
{
    {BAVC_MatrixCoefficients_eHdmi_RGB,              BDBG_STRING("BAVC_MatrixCoefficients_eHdmi_RGB")},
    {BAVC_MatrixCoefficients_eItu_R_BT_709,          BDBG_STRING("BAVC_MatrixCoefficients_eItu_R_BT_709")},
    {BAVC_MatrixCoefficients_eUnknown,               BDBG_STRING("BAVC_MatrixCoefficients_eUnknown")},
    {BAVC_MatrixCoefficients_eDvi_Full_Range_RGB,    BDBG_STRING("BAVC_MatrixCoefficients_eDvi_Full_Range_RGB")},
    {BAVC_MatrixCoefficients_eFCC,                   BDBG_STRING("BAVC_MatrixCoefficients_eFCC")},
    {BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG,     BDBG_STRING("BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG")},
    {BAVC_MatrixCoefficients_eSmpte_170M,            BDBG_STRING("BAVC_MatrixCoefficients_eSmpte_170M")},
    {BAVC_MatrixCoefficients_eSmpte_240M,            BDBG_STRING("BAVC_MatrixCoefficients_eSmpte_240M")},
    {BAVC_MatrixCoefficients_eUnknown,               BDBG_STRING("unknown_8")},
    {BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL,     BDBG_STRING("BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL")},
    {BAVC_MatrixCoefficients_eItu_R_BT_2020_CL,      BDBG_STRING("BAVC_MatrixCoefficients_eItu_R_BT_2020_CL")},
    {BAVC_MatrixCoefficients_eXvYCC_709,             BDBG_STRING("BAVC_MatrixCoefficients_eXvYCC_709")},
    {BAVC_MatrixCoefficients_eXvYCC_601,             BDBG_STRING("BAVC_MatrixCoefficients_eXvYCC_601")},
    {BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr, BDBG_STRING("BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr")}
};

/* make sure matches with BVDC_P_Output */
static const struct
{
    BVDC_P_Output eVdcOutput;
    const char *pcVdcOutputStr;

} stVDC_P_Output_InfoTbl[] =
{
    {BVDC_P_Output_eYQI,           BDBG_STRING("BVDC_P_Output_eYQI")},
    {BVDC_P_Output_eYQI_M,         BDBG_STRING("BVDC_P_Output_eYQI_M")},
    {BVDC_P_Output_eYUV,           BDBG_STRING("BVDC_P_Output_eYUV")},
    {BVDC_P_Output_eYUV_M,         BDBG_STRING("BVDC_P_Output_eYUV_M")},
    {BVDC_P_Output_eYUV_N,         BDBG_STRING("BVDC_P_Output_eYUV_N")},
    {BVDC_P_Output_eYUV_NC,        BDBG_STRING("BVDC_P_Output_eYUV_NC")},
    {BVDC_P_Output_eYDbDr_LDK,     BDBG_STRING("BVDC_P_Output_eYDbDr_LDK")},
    {BVDC_P_Output_eYDbDr_BG,      BDBG_STRING("BVDC_P_Output_eYDbDr_BG")},
    {BVDC_P_Output_eYDbDr_H,       BDBG_STRING("BVDC_P_Output_eYDbDr_H")},
    {BVDC_P_Output_eSDYPrPb,       BDBG_STRING("BVDC_P_Output_eSDYPrPb")},
    {BVDC_P_Output_eSDRGB,         BDBG_STRING("BVDC_P_Output_eSDRGB")},
    {BVDC_P_Output_eHDYPrPb,       BDBG_STRING("BVDC_P_Output_eHDYPrPb")},
    {BVDC_P_Output_eHDRGB,         BDBG_STRING("BVDC_P_Output_eHDRGB")},
    {BVDC_P_Output_eHsync,         BDBG_STRING("BVDC_P_Output_eHsync")},
    {BVDC_P_Output_eUnknown,       BDBG_STRING("BVDC_P_Output_eUnknown")},
    {BVDC_P_Output_eNone,          BDBG_STRING("BVDC_P_Output_eNone")}
};


/*************************************************************************
 *  {secret}
 * BVDC_P_Display_FindDac_isr
 *  Return true if found, false otherwise.
 **************************************************************************/
bool BVDC_P_Display_FindDac_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_DacOutput                   eDacOutput )
{
    uint32_t               uiIndex;
    BVDC_P_DisplayInfo     *pCurInfo = &hDisplay->stCurInfo;

    /* Find the Dac output in the array */
    for(uiIndex=0; uiIndex < BVDC_P_MAX_DACS; uiIndex++)
    {
        if (pCurInfo->aDacOutput[uiIndex] == eDacOutput)
        {
            return true;
        }
    }
    return false;
}

static void BVDC_P_Display_CalculateOffset_isr
    ( BVDC_P_DisplayAnlgChan          *pstAnlgChan,
      BVDC_P_DisplayDviChan           *pstDviChan,
      BVDC_P_ResourceType              eResourceType )
{
    switch(eResourceType)
    {
        case BVDC_P_ResourceType_eIt:
            switch(pstAnlgChan->ulIt)
            {
            case 0:
                pstAnlgChan->ulItRegOffset = 0;
                pstAnlgChan->ulRmRegOffset = 0;
                pstAnlgChan->ulItSwInitOffset    = 0;
                break;
#ifdef BCHP_IT_1_REG_START
            case 1:
                pstAnlgChan->ulItRegOffset = BCHP_IT_1_REG_START - BCHP_IT_0_REG_START;
                pstAnlgChan->ulRmRegOffset = BCHP_RM_1_REG_START - BCHP_RM_0_REG_START;
                pstAnlgChan->ulItSwInitOffset    = BVDC_P_VEC_SW_INIT_OFFSET(IT, 1, 0);
                break;
#endif
#ifdef BCHP_IT_2_REG_START
            case 2:
                pstAnlgChan->ulItRegOffset = BCHP_IT_2_REG_START - BCHP_IT_0_REG_START;
                pstAnlgChan->ulRmRegOffset = BCHP_RM_2_REG_START - BCHP_RM_0_REG_START;
                pstAnlgChan->ulItSwInitOffset  = BVDC_P_VEC_SW_INIT_OFFSET(IT, 2, 0);
                break;
#endif
            default:
                BDBG_ERR(("Unsupported IT %d", pstAnlgChan->ulIt));
                BDBG_ASSERT(0);
            }
            break;

        case BVDC_P_ResourceType_eVf:
            switch(pstAnlgChan->ulVf)
            {
            case 0:
                pstAnlgChan->ulVfRegOffset = 0;
                pstAnlgChan->ulCscRegOffset = 0;
                pstAnlgChan->ulVfSwInitOffset = 0;
                break;
#ifdef BCHP_VF_1_REG_START
            case 1:
                pstAnlgChan->ulVfRegOffset  = BCHP_VF_1_REG_START  - BCHP_VF_0_REG_START;
                pstAnlgChan->ulCscRegOffset = BCHP_CSC_1_REG_START - BCHP_CSC_0_REG_START;
                pstAnlgChan->ulVfSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(VF, 1, 0);
                break;
#endif
#ifdef BCHP_VF_2_REG_START
            case 2:
                pstAnlgChan->ulVfRegOffset  = BCHP_VF_2_REG_START  - BCHP_VF_0_REG_START;
                pstAnlgChan->ulCscRegOffset = BCHP_CSC_2_REG_START - BCHP_CSC_0_REG_START;
                pstAnlgChan->ulVfSwInitOffset  = BVDC_P_VEC_SW_INIT_OFFSET(VF, 2, 0);
                break;
#endif
#ifdef BCHP_VF_3_REG_START
            case 3:
                pstAnlgChan->ulVfRegOffset  = BCHP_VF_3_REG_START  - BCHP_VF_0_REG_START;
                pstAnlgChan->ulCscRegOffset = BCHP_CSC_3_REG_START - BCHP_CSC_0_REG_START;
                pstAnlgChan->ulVfSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(VF, 3, 0);
                break;
#endif
            default:
                BDBG_ERR(("Unsupported VF %d", pstAnlgChan->ulVf));
                BDBG_ASSERT(0);
            }
            break;

        case BVDC_P_ResourceType_eSecam:
            switch(pstAnlgChan->ulSecam)
            {
            case 0:
                pstAnlgChan->ulSecamRegOffset = 0;
                pstAnlgChan->ulSecamSwInitOffset = 0;
                break;
#ifdef BCHP_SECAM_1_REG_START
            case 1:
                pstAnlgChan->ulSecamRegOffset = BCHP_SECAM_1_REG_START - BCHP_SECAM_0_REG_START;
                pstAnlgChan->ulSecamSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(SECAM, 1, 0);
                break;
#endif
#ifdef BCHP_SECAM_2_REG_START
            case 2:
                pstAnlgChan->ulSecamRegOffset = BCHP_SECAM_2_REG_START - BCHP_SECAM_0_REG_START;
                pstAnlgChan->ulSecamSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(SECAM, 2, 0);
                break;
#endif
            default:
                BDBG_ERR(("Unsupported SECAM %d", pstAnlgChan->ulSecam));
                BDBG_ASSERT(0);
            }
            break;

        case BVDC_P_ResourceType_eSecam_HD:
            /* TODO */
            switch(BVDC_P_NUM_SHARED_SECAM + pstAnlgChan->ulSecam_HD)
            {

            case 0:
                pstAnlgChan->ulSecamHDSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(SECAM, 0, 0);
                break;
#ifdef BCHP_VEC_CFG_SECAM_1_SOURCE
            case 1:
                pstAnlgChan->ulSecamHDSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(SECAM, 1, 0);
                break;
#endif
#ifdef BCHP_VEC_CFG_SECAM_2_SOURCE
            case 2:
                pstAnlgChan->ulSecamHDSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(SECAM, 2, 0);
                break;
#endif
#ifdef BCHP_VEC_CFG_SECAM_3_SOURCE
            case 3:
                pstAnlgChan->ulSecamHDSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(SECAM, 3, 0);
                break;
#endif
#ifdef BCHP_VEC_CFG_SECAM_4_SOURCE
            case 4:
                pstAnlgChan->ulSecamHDSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(SECAM, 4, 0);
                break;
#endif
#ifdef BCHP_VEC_CFG_SECAM_5_SOURCE
            case 5:
                pstAnlgChan->ulSecamHDSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(SECAM, 5, 0);
                break;
#endif
            default:
                BDBG_ERR(("Unsupported BVDC_P_NUM_SHARED_SECAM=%d secam_HD %d",
                    BVDC_P_NUM_SHARED_SECAM, pstAnlgChan->ulSecam_HD));
                BDBG_ASSERT(0);
            }
            break;

        case BVDC_P_ResourceType_eSdsrc:
            switch(pstAnlgChan->ulSdsrc)
            {
            case 0:
                pstAnlgChan->ulSmRegOffset = 0;
                pstAnlgChan->ulSdsrcRegOffset = 0;
                pstAnlgChan->ulSdsrcSwInitOffset = 0;
                break;

#ifdef BCHP_SDSRC_1_REG_START
            case 1:
                pstAnlgChan->ulSmRegOffset    = BCHP_SM_1_REG_START    - BCHP_SM_0_REG_START;
                pstAnlgChan->ulSdsrcRegOffset = BCHP_SDSRC_1_REG_START - BCHP_SDSRC_0_REG_START;
                pstAnlgChan->ulSdsrcSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(SDSRC, 1, 0);
                break;
#endif
#ifdef BCHP_SDSRC_2_REG_START
            case 2:
                pstAnlgChan->ulSmRegOffset    = BCHP_SM_2_REG_START    - BCHP_SM_0_REG_START;
                pstAnlgChan->ulSdsrcRegOffset = BCHP_SDSRC_2_REG_START - BCHP_SDSRC_0_REG_START;
                pstAnlgChan->ulSdsrcSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(SDSRC, 2, 0);
                break;
#endif
            default:
                BDBG_ERR(("Unsupported SDSRC %d", pstAnlgChan->ulSdsrc));
                BDBG_ASSERT(0);
            }
            break;

        case BVDC_P_ResourceType_eHdsrc:
            switch(pstAnlgChan->ulHdsrc)
            {
            case 0:
                pstAnlgChan->ulHdsrcRegOffset = 0;
                pstAnlgChan->ulHdsrcSwInitOffset = 0;
                break;
            default:
                BDBG_ERR(("Unsupported HDSRC %d", pstAnlgChan->ulHdsrc));
                BDBG_ASSERT(0);
            }
            break;

        case BVDC_P_ResourceType_eDvi:
            switch(pstDviChan->ulDvi)
            {
            case 0:
                pstDviChan->ulDviRegOffset = 0;
                pstDviChan->ulDvpRegOffset = 0;
                pstDviChan->ulDtgSwInitOffset  = 0;
                pstDviChan->ulCscSwInitOffset  = 0;
                pstDviChan->ulDvfSwInitOffset  = 0;
                pstDviChan->ulMiscSwInitOffset = 0;
                pstDviChan->ulFcSwInitOffset = 0;
                break;
#ifdef BCHP_DVI_DTG_1_REG_START
            case 1:
                pstDviChan->ulDviRegOffset = BCHP_DVI_DTG_1_REG_START - BCHP_DVI_DTG_0_REG_START;
#ifdef BCHP_DVI_HT_1_REG_START
                pstDviChan->ulDvpRegOffset = BCHP_DVP_HT_1_REG_START  - BCHP_DVP_HT_REG_START;
#else
                pstDviChan->ulDvpRegOffset = 0;
#endif
                pstDviChan->ulDtgSwInitOffset  = BVDC_P_VEC_SW_INIT_OFFSET(DVI_DTG,  1, 0);
                pstDviChan->ulCscSwInitOffset  = BVDC_P_VEC_SW_INIT_OFFSET(DVI_CSC,  1, 0);
                pstDviChan->ulDvfSwInitOffset  = BVDC_P_VEC_SW_INIT_OFFSET(DVI_DVF,  1, 0);
                pstDviChan->ulMiscSwInitOffset = BVDC_P_VEC_SW_INIT_OFFSET(DVI_MISC, 1, 0);
#ifdef BCHP_DVI_FC_1_REG_START
                pstDviChan->ulFcSwInitOffset   = BVDC_P_VEC_SW_INIT_OFFSET(DVI_FC,   1, 0);
#endif
                break;
#endif
            default:
                BDBG_ERR(("Unsupported DVI %d", pstDviChan->ulDvi));
                BDBG_ASSERT(0);
            }
            break;

        default:
            BDBG_ERR(("Unsupported Resource type %d", eResourceType));
            BDBG_ASSERT(0);
    }
}

void BVDC_P_ResetAnalogChanInfo
    ( BVDC_P_DisplayAnlgChan          *pstChan )
{
    pstChan->bEnable             = false; /* off */
    pstChan->bTearDown           = false;
    pstChan->eState              = BVDC_P_DisplayResource_eInactive;
    pstChan->ulIt                = BVDC_P_HW_ID_INVALID;
    pstChan->ulVf                = BVDC_P_HW_ID_INVALID;
    pstChan->ulSecam             = BVDC_P_HW_ID_INVALID;
    pstChan->ulSecam_HD          = BVDC_P_HW_ID_INVALID;
    pstChan->ulPrevSecam         = BVDC_P_HW_ID_INVALID;
    pstChan->ulPrevSecam_HD      = BVDC_P_HW_ID_INVALID;
    pstChan->ulSdsrc             = BVDC_P_HW_ID_INVALID;
    pstChan->ulPrevSdsrc         = BVDC_P_HW_ID_INVALID;
    pstChan->ulHdsrc             = BVDC_P_HW_ID_INVALID;
    pstChan->ulPrevHdsrc         = BVDC_P_HW_ID_INVALID;
    pstChan->ulDac_0             = BVDC_P_HW_ID_INVALID;
    pstChan->ulDac_1             = BVDC_P_HW_ID_INVALID;
    pstChan->ulDac_2             = BVDC_P_HW_ID_INVALID;
}

void BVDC_P_FreeAnalogChanResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_P_DisplayAnlgChan          *pstChan )
{
    pstChan->ulPrevSdsrc = pstChan->ulSdsrc;
    pstChan->ulPrevHdsrc = pstChan->ulHdsrc;

    if (pstChan->ulVf != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eVf, pstChan->ulVf);
        pstChan->ulVf = BVDC_P_HW_ID_INVALID;
    }

    if (pstChan->ulSecam != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eSecam, pstChan->ulSecam);
        pstChan->ulSecam = BVDC_P_HW_ID_INVALID;
    }

    if (pstChan->ulSecam_HD != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eSecam_HD, pstChan->ulSecam_HD);
        pstChan->ulSecam_HD = BVDC_P_HW_ID_INVALID;
    }

    if (pstChan->ulSdsrc != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eSdsrc, pstChan->ulSdsrc);
        pstChan->ulSdsrc = BVDC_P_HW_ID_INVALID;
    }

    if (pstChan->ulHdsrc != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eHdsrc, pstChan->ulHdsrc);
        pstChan->ulHdsrc = BVDC_P_HW_ID_INVALID;
    }

    return;
}

void BVDC_P_FreeITResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_P_DisplayAnlgChan          *pstChan )
{
    if (pstChan->ulId == 0 && (pstChan->ulIt != BVDC_P_HW_ID_INVALID))
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eIt, pstChan->ulIt);
        pstChan->ulIt = BVDC_P_HW_ID_INVALID;
    }
    return;
}

BERR_Code BVDC_P_AllocITResources
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_DisplayId                   eDisplayId,
      BVDC_P_DisplayAnlgChan          *pstChan,
      uint32_t                         ulIt )
{
    BERR_Code err = BERR_SUCCESS;

    if (ulIt == BVDC_P_HW_ID_INVALID)
    {
        err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eIt, 0, eDisplayId, &pstChan->ulIt, false);
        if (err)
        {
            BDBG_ERR(("No IT available"));
            goto fail;
        }
        BVDC_P_Display_CalculateOffset_isr(pstChan, NULL, BVDC_P_ResourceType_eIt);
    }
    else
    {
        pstChan->ulIt = ulIt;
    }

    BDBG_MSG(("     ulIt = %d ItOffset=0x%08x RmOffset=0x%08x",
        pstChan->ulIt, pstChan->ulItRegOffset, pstChan->ulRmRegOffset));
    return BERR_SUCCESS;

fail:
    BVDC_P_FreeITResources_isr(hResource, pstChan);
    return BERR_TRACE(err);
}

BERR_Code BVDC_P_AllocAnalogChanResources
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_DisplayId                   eDisplayId,
      BVDC_P_DisplayAnlgChan          *pstChan,
      bool                             bHdMust,
      bool                             bHdRec,
      bool                             bSecamCap )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_MSG(("bHdMust = %d, bHdRec = %d, bSecamCap = %d", bHdMust, bHdRec, bSecamCap));

    if(pstChan->ulVf == BVDC_P_HW_ID_INVALID)
    {
        err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eVf, 0, eDisplayId, &pstChan->ulVf, false);
        if (err)
        {
            BDBG_ERR(("No VF available"));
            goto fail;
        }
        BVDC_P_Display_CalculateOffset_isr(pstChan, NULL, BVDC_P_ResourceType_eVf);
    }
    BDBG_MSG(("     ulVf = %d VfOffset=0x%08x CscOffset=0x%08x",
        pstChan->ulVf, pstChan->ulVfRegOffset, pstChan->ulCscRegOffset));

    pstChan->ulPrevSecam = pstChan->ulSecam;
    pstChan->ulPrevSecam_HD = pstChan->ulSecam_HD;
    if (bSecamCap)
    {
        if(pstChan->ulSecam_HD != BVDC_P_HW_ID_INVALID)
        {
            BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eSecam_HD, pstChan->ulSecam_HD);
            pstChan->ulSecam_HD = BVDC_P_HW_ID_INVALID;
        }
        if(pstChan->ulSecam == BVDC_P_HW_ID_INVALID)
        {
            err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eSecam, 0, eDisplayId, &pstChan->ulSecam, false);
            if (err)
            {
                BDBG_ERR(("No SECAM  available"));
                goto fail;
            }
            BVDC_P_Display_CalculateOffset_isr(pstChan, NULL, BVDC_P_ResourceType_eSecam);
        }
    }
    else
    {
        if(pstChan->ulSecam != BVDC_P_HW_ID_INVALID)
        {
            BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eSecam, pstChan->ulSecam);
            pstChan->ulSecam = BVDC_P_HW_ID_INVALID;
        }
        if(pstChan->ulSecam_HD == BVDC_P_HW_ID_INVALID)
        {
            err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eSecam_HD, 0, eDisplayId, &pstChan->ulSecam_HD, true);
            if (err)
            {
                BDBG_MSG(("No HD SECAM available... trying to acquire SECAM"));
                err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eSecam, 0, eDisplayId, &pstChan->ulSecam, false);
                if (err)
                {
                    BDBG_ERR(("No SECAM available"));
                    goto fail;
                }
                BVDC_P_Display_CalculateOffset_isr(pstChan, NULL, BVDC_P_ResourceType_eSecam);
            }
            else
            {
                err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eSecam_HD, 0, eDisplayId, &pstChan->ulSecam_HD, false);
                if(err)
                {
                    BDBG_ERR(("No SECAM HD available"));
                    goto fail;
                }
                BVDC_P_Display_CalculateOffset_isr(pstChan, NULL, BVDC_P_ResourceType_eSecam_HD);
            }
        }
    }
    BDBG_MSG(("     ulSecam = %d SecamOffset=0x%08x", pstChan->ulSecam, pstChan->ulSecamRegOffset));
    BDBG_MSG(("     ulSecam_HD = %d", pstChan->ulSecam_HD));
    BDBG_MSG(("     ulPrevSecam = %d", pstChan->ulPrevSecam));
    BDBG_MSG(("     ulPrevSecam_HD = %d", pstChan->ulPrevSecam_HD));

    pstChan->ulPrevSdsrc = pstChan->ulSdsrc;
    pstChan->ulPrevHdsrc = pstChan->ulHdsrc;
    if (bHdMust)
    {
        if(pstChan->ulSdsrc != BVDC_P_HW_ID_INVALID)
        {
            BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eSdsrc, pstChan->ulSdsrc);
            pstChan->ulSdsrc = BVDC_P_HW_ID_INVALID;
        }
        if(pstChan->ulHdsrc == BVDC_P_HW_ID_INVALID)
        {
            err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eHdsrc, 0, eDisplayId, &pstChan->ulHdsrc, false);

            if (err)
            {
                BDBG_ERR(("No HDSRC available"));
                goto fail;
            }
            BVDC_P_Display_CalculateOffset_isr(pstChan, NULL, BVDC_P_ResourceType_eHdsrc);
        }
        BDBG_MSG(("     ulSdsrc = %d", pstChan->ulSdsrc));
        BDBG_MSG(("     ulHdsrc = %d HdsrcOffset=0x%08x",
            pstChan->ulHdsrc, pstChan->ulHdsrcRegOffset));
    }
    else if (bHdRec)
    {
        if(pstChan->ulSdsrc != BVDC_P_HW_ID_INVALID)
        {
            BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eSdsrc, pstChan->ulSdsrc);
            pstChan->ulSdsrc = BVDC_P_HW_ID_INVALID;
        }
        if(pstChan->ulHdsrc == BVDC_P_HW_ID_INVALID)
        {
            err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eHdsrc, 0, eDisplayId, &pstChan->ulHdsrc, true);

            if (err)
            {
                BDBG_MSG(("No HDSRC available... trying to acquire SDSRC"));
                err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eSdsrc, 0, eDisplayId, &pstChan->ulSdsrc, false);
                if (err)
                {
                    BDBG_ERR(("No SDSRC available"));
                    goto fail;
                }
                BVDC_P_Display_CalculateOffset_isr(pstChan, NULL, BVDC_P_ResourceType_eSdsrc);
                BDBG_MSG(("     ulSdsrc = %d SdsrcOffset=0x%08x SmOffset=0x%08x",
                    pstChan->ulSdsrc, pstChan->ulSdsrcRegOffset, pstChan->ulSmRegOffset));
                BDBG_MSG(("     ulHdsrc = %d", pstChan->ulHdsrc));
            }
            else
            {
                err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eHdsrc, 0, eDisplayId, &pstChan->ulHdsrc, false);
                if(err)
                {
                    BDBG_ERR(("No HDSRC available"));
                    goto fail;
                }
                BVDC_P_Display_CalculateOffset_isr(pstChan, NULL, BVDC_P_ResourceType_eHdsrc);
                BDBG_MSG(("     ulSdsrc = %d", pstChan->ulSdsrc));
                BDBG_MSG(("     ulHdsrc = %d HdsrcOffset=0x%08x",
                    pstChan->ulHdsrc, pstChan->ulHdsrcRegOffset));
            }
        }
        else
        {
            BDBG_MSG(("     ulSdsrc = %d", pstChan->ulSdsrc));
            BDBG_MSG(("     ulHdsrc = %d", pstChan->ulHdsrc));
        }
    }
    else
    {
        if(pstChan->ulHdsrc != BVDC_P_HW_ID_INVALID)
        {
            BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eHdsrc, pstChan->ulHdsrc);
            pstChan->ulHdsrc = BVDC_P_HW_ID_INVALID;
        }
        if(pstChan->ulSdsrc == BVDC_P_HW_ID_INVALID)
        {
            err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eSdsrc, 0, eDisplayId, &pstChan->ulSdsrc, true);

            if (err)
            {
                BDBG_MSG(("No SDSRC available... trying to acquire HDSRC"));
                err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eHdsrc, 0, eDisplayId, &pstChan->ulHdsrc, false);
                if (err)
                {
                    BDBG_ERR(("No HDSRC available"));
                    goto fail;
                }
                BVDC_P_Display_CalculateOffset_isr(pstChan, NULL, BVDC_P_ResourceType_eHdsrc);
                BDBG_MSG(("     ulSdsrc = %d", pstChan->ulSdsrc));
                BDBG_MSG(("     ulHdsrc = %d HdsrcOffset=0x%08x",
                    pstChan->ulHdsrc, pstChan->ulHdsrcRegOffset));
            }
            else
            {
                err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eSdsrc, 0, eDisplayId, &pstChan->ulSdsrc, false);
                if(err)
                {
                    BDBG_ERR(("No SDSRC available"));
                    goto fail;
                }
                BVDC_P_Display_CalculateOffset_isr(pstChan, NULL, BVDC_P_ResourceType_eSdsrc);
                BDBG_MSG(("     ulSdsrc = %d SdsrcOffset=0x%08x SmOffset=0x%08x",
                    pstChan->ulSdsrc, pstChan->ulSdsrcRegOffset, pstChan->ulSmRegOffset));
                BDBG_MSG(("     ulHdsrc = %d", pstChan->ulHdsrc));
            }
        }
        else
        {
            BDBG_MSG(("     ulSdsrc = %d", pstChan->ulSdsrc));
            BDBG_MSG(("     ulHdsrc = %d", pstChan->ulHdsrc));
        }
    }
    BDBG_MSG(("     ulPrevSdsrc = %d", pstChan->ulPrevSdsrc));
    BDBG_MSG(("     ulPrevHdsrc = %d", pstChan->ulPrevHdsrc));

    return BERR_SUCCESS;

fail:
    BVDC_P_FreeAnalogChanResources_isr(hResource, pstChan);
    return BERR_TRACE(err);
}


static void BVDC_P_TearDownIT_isr
    ( BVDC_P_DisplayAnlgChan             *pstChan,
      BVDC_P_ListInfo                    *pList )
{
    /* Disable modules in the path. This will drain data in the pipeline. */
    if ((pstChan->ulIt != BVDC_P_HW_ID_INVALID))
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_IT_0_SOURCE + pstChan->ulIt * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_IT_0_SOURCE, SOURCE, BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_DISABLE);
    }

    if ((pstChan->ulIt != BVDC_P_HW_ID_INVALID))
    {
        BVDC_P_VEC_SW_INIT(IT_0, pstChan->ulItSwInitOffset, 1);
        BVDC_P_VEC_SW_INIT(IT_0, pstChan->ulItSwInitOffset, 0);
    }

    return;
}

static void BVDC_P_TearDownAnalogChan_isr
    ( BVDC_P_DisplayAnlgChan             *pstChan,
      BVDC_P_ListInfo                    *pList )
{
    if (pstChan->ulVf != BVDC_P_HW_ID_INVALID)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_VF_0_SOURCE + pstChan->ulVf * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_VF_0_SOURCE, SOURCE, BCHP_VEC_CFG_VF_0_SOURCE_SOURCE_DISABLE);
    }

    if (pstChan->ulSecam != BVDC_P_HW_ID_INVALID)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SECAM_0_SOURCE + pstChan->ulSecam * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_SECAM_0_SOURCE, SOURCE, BCHP_VEC_CFG_SECAM_0_SOURCE_SOURCE_DISABLE);
    }

    if (pstChan->ulSecam_HD != BVDC_P_HW_ID_INVALID)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SECAM_0_SOURCE + (BVDC_P_NUM_SHARED_SECAM + pstChan->ulSecam_HD) * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_SECAM_0_SOURCE, SOURCE, BCHP_VEC_CFG_SECAM_0_SOURCE_SOURCE_DISABLE);
    }

    if (pstChan->ulSdsrc != BVDC_P_HW_ID_INVALID)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SDSRC_0_SOURCE + pstChan->ulSdsrc * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_SDSRC_0_SOURCE, SOURCE, BCHP_VEC_CFG_SDSRC_0_SOURCE_SOURCE_DISABLE);
    }

#if (BVDC_P_NUM_SHARED_HDSRC > 0)
    if (pstChan->ulHdsrc != BVDC_P_HW_ID_INVALID)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_HDSRC_0_SOURCE + pstChan->ulHdsrc * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_HDSRC_0_SOURCE, SOURCE, BCHP_VEC_CFG_HDSRC_0_SOURCE_SOURCE_DISABLE);
    }
#endif

    /* Reset and unreset all the modules on the path.
     * SECAM0 can not be left at reset state. It would
     * affect other cores.
     */
    if (pstChan->ulVf != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_VEC_SW_INIT(VF_0, pstChan->ulVfSwInitOffset, 1);
        BVDC_P_VEC_SW_INIT(VF_0, pstChan->ulVfSwInitOffset, 0);
    }

    if (pstChan->ulSecam != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_VEC_SW_INIT(SECAM_0, pstChan->ulSecamSwInitOffset, 1);
        BVDC_P_VEC_SW_INIT(SECAM_0, pstChan->ulSecamSwInitOffset, 0);
    }

    if (pstChan->ulSecam_HD != BVDC_P_HW_ID_INVALID
#if BVDC_P_VEC_HW7425_475_WORK_AROUND
        && pstChan->ulSecam_HD != 1 /* SECAM_2 */
#endif
        )
    {
        BVDC_P_VEC_SW_INIT(SECAM_0, pstChan->ulSecamHDSwInitOffset, 1);
        BVDC_P_VEC_SW_INIT(SECAM_0, pstChan->ulSecamHDSwInitOffset, 0);
    }

    if (pstChan->ulSdsrc != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_VEC_SW_INIT(SDSRC_0, pstChan->ulSdsrcSwInitOffset, 1);
        BVDC_P_VEC_SW_INIT(SDSRC_0, pstChan->ulSdsrcSwInitOffset, 0);
    }

#if (BVDC_P_NUM_SHARED_HDSRC > 0)
    if (pstChan->ulHdsrc != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_VEC_SW_INIT(HDSRC_0, pstChan->ulHdsrcSwInitOffset, 1);
        BVDC_P_VEC_SW_INIT(HDSRC_0, pstChan->ulHdsrcSwInitOffset, 0);
    }
#endif

    return;
}

/* return the IT/DECIM/ITU656/DVI source ID */
static uint32_t BVDC_P_GetVecCfgSrc_isr
    ( BVDC_Display_Handle              hDisplay )
{
    uint32_t ulSrc = 0;

    switch(hDisplay->hCompositor->eId)
    {
        case BVDC_CompositorId_eCompositor0:
            ulSrc = BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_S_0;
            break;

        case BVDC_CompositorId_eCompositor1:
            ulSrc = BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_S_1;
            break;

#if (BVDC_P_CMP_2_MAX_VIDEO_WINDOW_COUNT > 0)
        case BVDC_CompositorId_eCompositor2:
            ulSrc = BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_S_2;
            break;
#endif

#if (BVDC_P_CMP_3_MAX_VIDEO_WINDOW_COUNT > 0)
        case BVDC_CompositorId_eCompositor3:
            ulSrc = BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_S_3;
            break;
#endif

#if (BVDC_P_CMP_4_MAX_VIDEO_WINDOW_COUNT > 0)
        case BVDC_CompositorId_eCompositor4:
            ulSrc = BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_S_4;
            break;
#endif

#if (BVDC_P_CMP_5_MAX_VIDEO_WINDOW_COUNT > 0)
        case BVDC_CompositorId_eCompositor5:
            ulSrc = BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_S_5;
            break;
#endif

#if (BVDC_P_CMP_6_MAX_VIDEO_WINDOW_COUNT > 0)
        case BVDC_CompositorId_eCompositor6:
            ulSrc = BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_S_6;
            break;
#endif

        default:
            /* Add other source later */
            BDBG_ERR(("unsupport source %d", hDisplay->hCompositor->eId));
            BDBG_ASSERT(0);
            break;
    }

    return ulSrc;
}


static void BVDC_P_SetupIT_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayAnlgChan          *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t ulSrc = 0;

    BDBG_OBJECT_ASSERT(hDisplay, BVDC_DSP);
    BDBG_OBJECT_ASSERT(hDisplay->hCompositor, BVDC_CMP);

    /* Connect each module to its source */
    if ((pstChan->ulIt != BVDC_P_HW_ID_INVALID))
    {
        ulSrc = BVDC_P_GetVecCfgSrc_isr(hDisplay);
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_IT_0_SOURCE + pstChan->ulIt * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_IT_0_SOURCE, SOURCE, ulSrc);
    }
    return;
}

static void BVDC_P_SetupAnalogChan_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_P_DisplayAnlgChan          *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t ulSrcSource = 0;

    if (pstChan->ulVf != BVDC_P_HW_ID_INVALID)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_VF_0_SOURCE + pstChan->ulVf * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_VF_0_SOURCE, SOURCE, pstChan->ulIt);
    }

    if (pstChan->ulSecam != BVDC_P_HW_ID_INVALID)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SECAM_0_SOURCE + pstChan->ulSecam * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_SECAM_0_SOURCE, SOURCE, pstChan->ulVf);
        if(pstChan->ulPrevSecam_HD != pstChan->ulSecam_HD &&
           BVDC_P_Resource_GetHwIdAcquireCntr_isr(hResource, BVDC_P_ResourceType_eSecam_HD, pstChan->ulPrevSecam_HD) == 0)
        {
            BDBG_MSG(("Need to disable Secam_HD %d", pstChan->ulPrevSecam_HD));
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SECAM_0_SOURCE + (BVDC_P_NUM_SHARED_SECAM + pstChan->ulPrevSecam_HD) * 4);
            *pList->pulCurrent++ =
                BCHP_FIELD_DATA(VEC_CFG_SECAM_0_SOURCE, SOURCE, BCHP_VEC_CFG_SECAM_0_SOURCE_SOURCE_DISABLE);
        }
    }

    if (pstChan->ulSecam_HD != BVDC_P_HW_ID_INVALID)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SECAM_0_SOURCE + (BVDC_P_NUM_SHARED_SECAM + pstChan->ulSecam_HD) * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_SECAM_0_SOURCE, SOURCE, pstChan->ulVf);
        if(pstChan->ulPrevSecam != pstChan->ulSecam &&
           BVDC_P_Resource_GetHwIdAcquireCntr_isr(hResource, BVDC_P_ResourceType_eSecam, pstChan->ulPrevSecam) == 0)
        {
            BDBG_MSG(("Need to disable prev Secam %d", pstChan->ulPrevSecam));
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SECAM_0_SOURCE + pstChan->ulPrevSecam * 4);
            *pList->pulCurrent++ =
                BCHP_FIELD_DATA(VEC_CFG_SECAM_0_SOURCE, SOURCE, BCHP_VEC_CFG_SECAM_0_SOURCE_SOURCE_DISABLE);
        }
    }

    if(pstChan->ulSecam != BVDC_P_HW_ID_INVALID)
    {
        ulSrcSource = pstChan->ulSecam;
    }
    else if(pstChan->ulSecam_HD != BVDC_P_HW_ID_INVALID)
    {
        ulSrcSource = pstChan->ulSecam_HD;
    }

    if(pstChan->ulSecam_HD != BVDC_P_HW_ID_INVALID)
    {
        ulSrcSource += BVDC_P_NUM_SHARED_SECAM;
    }

    if (pstChan->ulSdsrc != BVDC_P_HW_ID_INVALID)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SDSRC_0_SOURCE + pstChan->ulSdsrc * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_SDSRC_0_SOURCE, SOURCE, ulSrcSource);

#if (BVDC_P_NUM_SHARED_HDSRC > 0)
        if(pstChan->ulSdsrc != pstChan->ulPrevSdsrc &&
           pstChan->ulHdsrc != pstChan->ulPrevHdsrc &&
           BVDC_P_Resource_GetHwIdAcquireCntr_isr(hResource, BVDC_P_ResourceType_eHdsrc, pstChan->ulPrevHdsrc) == 0)
        {
            BDBG_MSG(("Need to disable prev HDSRC %d", pstChan->ulPrevHdsrc));
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_HDSRC_0_SOURCE + pstChan->ulPrevHdsrc * 4);
            *pList->pulCurrent++ =
                BCHP_FIELD_DATA(VEC_CFG_HDSRC_0_SOURCE, SOURCE, BCHP_VEC_CFG_HDSRC_0_SOURCE_SOURCE_DISABLE);
        }
#endif
    }

#if (BVDC_P_NUM_SHARED_HDSRC > 0)
    if (pstChan->ulHdsrc != BVDC_P_HW_ID_INVALID)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_HDSRC_0_SOURCE + pstChan->ulHdsrc * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_HDSRC_0_SOURCE, SOURCE, BCHP_VEC_CFG_HDSRC_0_SOURCE_SOURCE_SSP_0 +
            ulSrcSource);
        if(pstChan->ulSdsrc != pstChan->ulPrevSdsrc &&
           pstChan->ulHdsrc != pstChan->ulPrevHdsrc &&
           BVDC_P_Resource_GetHwIdAcquireCntr_isr(hResource, BVDC_P_ResourceType_eSdsrc, pstChan->ulPrevSdsrc) == 0)
        {
            BDBG_MSG(("Need to disable Prev SDSRC %d", pstChan->ulPrevSdsrc));
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_SDSRC_0_SOURCE + pstChan->ulPrevSdsrc * 4);
            *pList->pulCurrent++ =
                BCHP_FIELD_DATA(VEC_CFG_SDSRC_0_SOURCE, SOURCE, BCHP_VEC_CFG_SDSRC_0_SOURCE_SOURCE_DISABLE);
        }
    }
#endif

    return;
}

static BERR_Code BVDC_P_FreeDacResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_P_DisplayAnlgChan          *pstChan )
{
    BERR_Code err = BERR_SUCCESS;

    if(BVDC_P_HW_ID_INVALID != pstChan->ulDac_0)
    {
        BDBG_MSG(("     release Dac %d", pstChan->ulDac_0));
        err = BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eDac, pstChan->ulDac_0);
        pstChan->ulDac_0 = BVDC_P_HW_ID_INVALID;
    }
    if(BVDC_P_HW_ID_INVALID != pstChan->ulDac_1)
    {
        BDBG_MSG(("     release Dac %d", pstChan->ulDac_1));
        err = BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eDac, pstChan->ulDac_1);
        pstChan->ulDac_1 = BVDC_P_HW_ID_INVALID;
    }
    if(BVDC_P_HW_ID_INVALID != pstChan->ulDac_2)
    {
        BDBG_MSG(("     release Dac %d", pstChan->ulDac_2));
        err = BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eDac, pstChan->ulDac_2);
        pstChan->ulDac_2 = BVDC_P_HW_ID_INVALID;
    }

#ifdef BCHP_PWR_RESOURCE_VDC_DAC
    if(pstChan->ulDacPwrAcquire != 0)
    {
        pstChan->ulDacPwrAcquire--;
        pstChan->ulDacPwrRelease = 1;
        BDBG_MSG(("DAC disable: Release pending BCHP_PWR_RESOURCE_VDC_DAC"));
    }
#endif

    return BERR_TRACE(err);
}

BERR_Code BVDC_P_AllocDacResources
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_P_DisplayAnlgChan          *pstChan,
      uint32_t                         ulDacMask )
{
    BERR_Code err;

    err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eDac, 0, ulDacMask, &pstChan->ulDac_0, false);
    if (err)
    {
        BDBG_ERR(("No DAC available"));
        goto fail;
    }
    BDBG_MSG(("     Dac_0 ID: %d", pstChan->ulDac_0));

    err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eDac, 0, ulDacMask, &pstChan->ulDac_1, false);
    if (err)
    {
        BDBG_ERR(("No DAC available"));
        goto fail;
    }
    BDBG_MSG(("     Dac_1 ID: %d", pstChan->ulDac_1));

    err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eDac, 0, ulDacMask, &pstChan->ulDac_2, false);
    if (err)
    {
        BDBG_ERR(("No DAC available"));
        goto fail;
    }
    BDBG_MSG(("     Dac_2 ID: %d", pstChan->ulDac_2));

#ifdef BCHP_PWR_RESOURCE_VDC_DAC
    if(pstChan->ulDacPwrAcquire == 0)
    {
        BDBG_MSG(("Chan%d: Acquire BCHP_PWR_RESOURCE_VDC_DAC", pstChan->ulId));
        pstChan->ulDacPwrAcquire++;
    }
#endif

    return BERR_SUCCESS;

fail:

    BVDC_P_FreeDacResources_isr(hResource, pstChan);
    return BERR_TRACE(err);
}


static uint32_t BVDC_P_Display_GetTg_isr
    ( BVDC_Display_Handle              hDisplay )
{
    uint32_t masterTg = 0;

    /* Set up master and slave timing generators */
    if(hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bHdmiRmd)
    {
        /* dvi in master mode */
        masterTg = BCHP_MISC_IT_0_MASTER_SEL_SELECT_DVI_DTG_0 + hDisplay->stDviChan.ulDvi;
    }
    else if(hDisplay->bAnlgEnable)
    {
            /* Analog channel is the TG master */
            BDBG_ASSERT(hDisplay->stAnlgChan_0.ulIt != BVDC_P_HW_ID_INVALID);
            masterTg = hDisplay->stAnlgChan_0.ulIt;
    }
#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
    else if(hDisplay->st656Chan.bEnable)
    {
        /* 656 masster mode */
        masterTg = BCHP_MISC_DVI_DTG_0_MASTER_SEL_SELECT_ITU656_DTG_0;
    }
#endif
    return masterTg;
}


static void BVDC_P_Display_SetupAnlgTG_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t masterTg;

    if(hDisplay->bAnlgEnable ||   /* if analog master */
       (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
       (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
    {
        masterTg = BVDC_P_Display_GetTg_isr(hDisplay);
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_IT_0_MASTER_SEL + hDisplay->stAnlgChan_0.ulIt * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(MISC_IT_0_MASTER_SEL, SELECT, masterTg);
    }

    return;
}

static void BVDC_P_Display_SetupDviTG_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t masterTg;
    uint32_t ulFreeRun;

    if(hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bEnableHdmi)
    {
        masterTg = BVDC_P_Display_GetTg_isr(hDisplay);
#if BVDC_P_SUPPORT_DTG_RMD
        ulFreeRun =
            (hDisplay->stDviChan.bEnable)  ? BCHP_MISC_DVI_DTG_0_MASTER_SEL_FREERUN_DWNSTRM_RM :
#if BCHP_HW7439_439_WORKAROUND
            (hDisplay->stCurInfo.bHdmiRmd) ? BCHP_MISC_DVI_DTG_0_MASTER_SEL_FREERUN_TWICE_DTG_RM :
#else
#if BCHP_MISC_DVI_DTG_0_MASTER_SEL_FREERUN_TWICE_DTG_RM
            (hDisplay->stCurInfo.stRateInfo.ulPixelClkRate == BFMT_PXL_594MHz ||
             hDisplay->stCurInfo.stRateInfo.ulPixelClkRate == BFMT_PXL_594MHz_DIV_1_001) ? BCHP_MISC_DVI_DTG_0_MASTER_SEL_FREERUN_TWICE_DTG_RM :
#endif
            (hDisplay->stCurInfo.bHdmiRmd) ? BCHP_MISC_DVI_DTG_0_MASTER_SEL_FREERUN_DTG_RM :
#endif
            BCHP_MISC_DVI_DTG_0_MASTER_SEL_FREERUN_OFF;
#else
        ulFreeRun = (hDisplay->stDviChan.bEnable)  ? 1 : 0;
#endif
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DVI_DTG_0_MASTER_SEL + hDisplay->stDviChan.ulDvi * 4);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(MISC_DVI_DTG_0_MASTER_SEL, SELECT, masterTg) |
            BCHP_FIELD_DATA(MISC_DVI_DTG_0_MASTER_SEL, FREERUN, ulFreeRun);
    }

    return;
}

#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
static void BVDC_P_Display_Setup656TG_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t masterTg;

    if (hDisplay->st656Chan.bEnable)
    {
        masterTg = BVDC_P_Display_GetTg_isr(hDisplay);
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_ITU656_DTG_0_MASTER_SEL);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(MISC_ITU656_DTG_0_MASTER_SEL, SELECT, masterTg) |
            BCHP_FIELD_DATA(MISC_ITU656_DTG_0_MASTER_SEL, FREERUN, (hDisplay->eMasterTg == BVDC_DisplayTg_e656Dtg) ? 1 : 0);
    }
    return;
}
#endif




static void BVDC_P_Vec_Build_CSC_isr
    ( const BVDC_P_DisplayCscMatrix   *pCscMatrix,
      BVDC_P_ListInfo                 *pList )
{
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(CSC_0_CSC_MODE, CLAMP_MODE_C0, MIN_MAX) |
        BCHP_FIELD_ENUM(CSC_0_CSC_MODE, CLAMP_MODE_C1, MIN_MAX) |
        BCHP_FIELD_ENUM(CSC_0_CSC_MODE, CLAMP_MODE_C2, MIN_MAX) |
        BCHP_FIELD_DATA(CSC_0_CSC_MODE, RANGE1, 0x005A) |
        BCHP_FIELD_DATA(CSC_0_CSC_MODE, RANGE2, 0x007F);

    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CSC_0_CSC_MIN_MAX, MIN, pCscMatrix->ulMin) |
        BCHP_FIELD_DATA(CSC_0_CSC_MIN_MAX, MAX, pCscMatrix->ulMax);

    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C01_C00, COEFF_C0, pCscMatrix->stCscCoeffs.usY0) |
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C01_C00, COEFF_C1, pCscMatrix->stCscCoeffs.usY1);

    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C03_C02, COEFF_C2, pCscMatrix->stCscCoeffs.usY2) |
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C03_C02, COEFF_C3, pCscMatrix->stCscCoeffs.usYOffset);

    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C11_C10, COEFF_C0, pCscMatrix->stCscCoeffs.usCb0) |
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C11_C10, COEFF_C1, pCscMatrix->stCscCoeffs.usCb1);

    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C13_C12, COEFF_C2, pCscMatrix->stCscCoeffs.usCb2) |
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C13_C12, COEFF_C3, pCscMatrix->stCscCoeffs.usCbOffset);

    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C21_C20, COEFF_C0, pCscMatrix->stCscCoeffs.usCr0) |
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C21_C20, COEFF_C1, pCscMatrix->stCscCoeffs.usCr1);

    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C23_C22, COEFF_C2, pCscMatrix->stCscCoeffs.usCr2) |
        BCHP_FIELD_DATA(CSC_0_CSC_COEFF_C23_C22, COEFF_C3, pCscMatrix->stCscCoeffs.usCrOffset);

    BDBG_MODULE_MSG(BVDC_DISP_CSC, ("BVDC_P_Vec_Build_CSC_isr: "));
    BDBG_MODULE_MSG(BVDC_DISP_CSC, ("     min = 0x%x, max = 0x%x", pCscMatrix->ulMin, pCscMatrix->ulMax));
    BDBG_MODULE_MSG(BVDC_DISP_CSC, ("     0x%08x 0x%08x 0x%08x 0x%08x",
        pCscMatrix->stCscCoeffs.usY0, pCscMatrix->stCscCoeffs.usY1,
        pCscMatrix->stCscCoeffs.usY2, pCscMatrix->stCscCoeffs.usYOffset));
    BDBG_MODULE_MSG(BVDC_DISP_CSC, ("     0x%08x 0x%08x 0x%08x 0x%08x",
        pCscMatrix->stCscCoeffs.usCb0, pCscMatrix->stCscCoeffs.usCb1,
        pCscMatrix->stCscCoeffs.usCb2, pCscMatrix->stCscCoeffs.usCbOffset));
    BDBG_MODULE_MSG(BVDC_DISP_CSC, ("     0x%08x 0x%08x 0x%08x 0x%08x",
        pCscMatrix->stCscCoeffs.usCr0, pCscMatrix->stCscCoeffs.usCr1,
        pCscMatrix->stCscCoeffs.usCr2, pCscMatrix->stCscCoeffs.usCrOffset));

    return;
}

static void BVDC_P_Vec_Build_Dither_isr
    ( BVDC_P_DitherSetting            *pDither,
      BVDC_P_ListInfo                 *pList )
{
    /* CSC_0_DITHER_CONTROL */
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CSC_0_DITHER_CONTROL, MODE,       pDither->ulMode     ) |
        BCHP_FIELD_DATA(CSC_0_DITHER_CONTROL, OFFSET_CH2, pDither->ulCh2Offset) |
        BCHP_FIELD_DATA(CSC_0_DITHER_CONTROL, SCALE_CH2,  pDither->ulCh2Scale ) |
        BCHP_FIELD_DATA(CSC_0_DITHER_CONTROL, OFFSET_CH1, pDither->ulCh1Offset) |
        BCHP_FIELD_DATA(CSC_0_DITHER_CONTROL, SCALE_CH1,  pDither->ulCh1Scale ) |
        BCHP_FIELD_DATA(CSC_0_DITHER_CONTROL, OFFSET_CH0, pDither->ulCh0Offset) |
        BCHP_FIELD_DATA(CSC_0_DITHER_CONTROL, SCALE_CH0,  pDither->ulCh0Scale );

    /* CSC_0_DITHER_LFSR */
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CSC_0_DITHER_LFSR, T0, pDither->ulLfsrCtrlT0) |
        BCHP_FIELD_DATA(CSC_0_DITHER_LFSR, T1, pDither->ulLfsrCtrlT1) |
        BCHP_FIELD_DATA(CSC_0_DITHER_LFSR, T2, pDither->ulLfsrCtrlT2);

    /* CSC_0_DITHER_LFSR_INIT */
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(CSC_0_DITHER_LFSR_INIT, SEQ,   pDither->ulLfsrSeq  ) |
        BCHP_FIELD_DATA(CSC_0_DITHER_LFSR_INIT, VALUE, pDither->ulLfsrValue);

    return;
}

static void BVDC_P_Vec_Build_SRC_isr
    ( uint32_t                         ulSrcControl,
      BVDC_P_DisplayAnlgChan          *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
#if (BVDC_P_NUM_SHARED_HDSRC > 0)
    *pList->pulCurrent++ = (pstChan->ulHdsrc != BVDC_P_HW_ID_INVALID) ?
        BRDC_REGISTER(BCHP_HDSRC_0_HDSRC_CONTROL + pstChan->ulHdsrcRegOffset) :
        BRDC_REGISTER(BCHP_SDSRC_0_SRC_CONTROL + pstChan->ulSdsrcRegOffset);
#else
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_SDSRC_0_SRC_CONTROL + pstChan->ulSdsrcRegOffset);
#endif
    *pList->pulCurrent++ = ulSrcControl;

#if (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_9)
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
#if (BVDC_P_NUM_SHARED_HDSRC > 0)
    *pList->pulCurrent++ = (pstChan->ulHdsrc != BVDC_P_HW_ID_INVALID) ?
        BRDC_REGISTER(BCHP_HDSRC_0_HDSRC_ANA_SCL_0_1 + pstChan->ulHdsrcRegOffset) :
        BRDC_REGISTER(BCHP_SDSRC_0_SRC_ANA_SCL_0_1 + pstChan->ulSdsrcRegOffset);
#else
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_SDSRC_0_SRC_ANA_SCL_0_1 + pstChan->ulSdsrcRegOffset);
#endif
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(SDSRC_0_SRC_ANA_SCL_0_1, OFFSET_1, 0x0  ) |
        BCHP_FIELD_DATA(SDSRC_0_SRC_ANA_SCL_0_1, VALUE_1,  0x133) |
        BCHP_FIELD_DATA(SDSRC_0_SRC_ANA_SCL_0_1, OFFSET_0, 0x0  ) |
        BCHP_FIELD_DATA(SDSRC_0_SRC_ANA_SCL_0_1, VALUE_0,  0x180);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
#if (BVDC_P_NUM_SHARED_HDSRC > 0)
    *pList->pulCurrent++ = (pstChan->ulHdsrc != BVDC_P_HW_ID_INVALID) ?
        BRDC_REGISTER(BCHP_HDSRC_0_HDSRC_ANA_SCL_2_3 + pstChan->ulHdsrcRegOffset) :
        BRDC_REGISTER(BCHP_SDSRC_0_SRC_ANA_SCL_2_3 + pstChan->ulSdsrcRegOffset);
#else
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_SDSRC_0_SRC_ANA_SCL_2_3 + pstChan->ulSdsrcRegOffset);
#endif
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(SDSRC_0_SRC_ANA_SCL_2_3, OFFSET_3, 0x0  ) |
        BCHP_FIELD_DATA(SDSRC_0_SRC_ANA_SCL_2_3, VALUE_3,  0x100) |
        BCHP_FIELD_DATA(SDSRC_0_SRC_ANA_SCL_2_3, OFFSET_2, 0x0  ) |
        BCHP_FIELD_DATA(SDSRC_0_SRC_ANA_SCL_2_3, VALUE_2,  0x120);
#endif

    return;
}

static void BVDC_P_Vec_Build_SM_isr
    ( BFMT_VideoFmt                    eVideoFmt,
      BVDC_P_Output                    eOutputCS,
      const uint32_t                  *pTable,
      BVDC_P_DisplayAnlgChan          *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t ulOffset = pstChan->ulSmRegOffset;

    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_SM_TABLE_SIZE);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_SM_0_PG_CNTRL + ulOffset);

    /* SM_PG_CNTRL -> SM_COMP_CNTRL */
    BKNI_Memcpy((void*)pList->pulCurrent, (void*)pTable,
        BVDC_P_SM_TABLE_SIZE * sizeof(uint32_t));
    pList->pulCurrent += BVDC_P_SM_TABLE_SIZE;

    /* Setup SM_COMP_CONFIG */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_SM_0_COMP_CONFIG + ulOffset);

    if(BFMT_IS_NTSC(eVideoFmt))
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, U_LINE_SEL,          0) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, COMPOSITE_CLAMP_SEL, 1) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, V_LINE_SEL,          0) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, U_FIXED_LINE,        0) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, COMPOSITE_OFFSET,    0);
    }
    else if(BFMT_IS_SECAM(eVideoFmt))
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, U_LINE_SEL,          0) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, COMPOSITE_CLAMP_SEL, 1) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, V_LINE_SEL,          1) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, U_FIXED_LINE,        0) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, COMPOSITE_OFFSET, 0xd0);
    }
    else /* PAL */
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, U_LINE_SEL,          0) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, COMPOSITE_CLAMP_SEL, 1) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, V_LINE_SEL,
            ((eOutputCS == BVDC_P_Output_eYUV)   ||
             (eOutputCS == BVDC_P_Output_eYUV_M) ||
             (eOutputCS == BVDC_P_Output_eYUV_N) ||
             (eOutputCS == BVDC_P_Output_eYUV_NC))                  ) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, U_FIXED_LINE,        0) |
            BCHP_FIELD_DATA(SM_0_COMP_CONFIG, COMPOSITE_OFFSET, 0);
    }

    return;
}

static void BVDC_P_Vec_Build_SECAM_isr
    ( BFMT_VideoFmt                    eVideoFmt,
      BVDC_P_DisplayAnlgChan          *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t ulOffset = pstChan->ulSecamRegOffset;

    if(BFMT_IS_SECAM(eVideoFmt))
    {
        /* Setup SECAM_FM_FMAMP */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_SECAM_0_FM_FMAMP + ulOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(SECAM_0_FM_FMAMP, LOWER_LIMIT,  64) |
            BCHP_FIELD_DATA(SECAM_0_FM_FMAMP, UPPER_LIMIT, 192) |
            BCHP_FIELD_DATA(SECAM_0_FM_FMAMP, SLOPE_ADJUST,  3) |
            BCHP_FIELD_DATA(SECAM_0_FM_FMAMP, SCALE, 92);

        /* Setup SECAM_FM_CONTROL */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_SECAM_0_FM_CONTROL + ulOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(SECAM_0_FM_CONTROL, FINE_LUMA_DELAY,  0) |
            BCHP_FIELD_DATA(SECAM_0_FM_CONTROL, GROSS_LUMA_DELAY, 0x18) |
            BCHP_FIELD_DATA(SECAM_0_FM_CONTROL, FINE_SC_DELAY,    0x3) |
            BCHP_FIELD_DATA(SECAM_0_FM_CONTROL, GROSS_SC_DELAY,   0x8) |
            BCHP_FIELD_ENUM(SECAM_0_FM_CONTROL, ENABLE, ON);
    }
    else /* NTSC or PAL */
    {
        /* Setup SECAM_FM_CONTROL */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_SECAM_0_FM_CONTROL + ulOffset);
        *pList->pulCurrent++ = BCHP_FIELD_ENUM(SECAM_0_FM_CONTROL, ENABLE, OFF);
    }
    return;
}

static void BDVC_P_Vec_Build_Grpd_isr
    ( const BFMT_VideoInfo            *pFmtInfo,
      const BVDC_P_DisplayAnlgChan    *pChannel,
      BVDC_P_ListInfo                 *pList )
{
#if (BVDC_P_SUPPORT_VEC_GRPD)
    /* TODO: Support multiple instances after 7550 to minimized change set.. */
    uint32_t ulOffset = pChannel->ulSdsrc;
    uint32_t ulVidIndex = BFMT_IS_NTSC(pFmtInfo->eVideoFmt) ? 0 : 1;
    BDBG_ASSERT(0 == ulOffset);

    /* GRPD_0_GRP00 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_DLAY_FILTER_COUNT);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_GRP00 + ulOffset);
    BKNI_Memcpy(pList->pulCurrent, s_aulGrpdFilters[ulVidIndex].aulDlay,
        sizeof(s_aulGrpdFilters[ulVidIndex].aulDlay));
    pList->pulCurrent += BVDC_P_DLAY_FILTER_COUNT;

    /* BCHP_GRPD_0_TRAP00 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_TRAP_FILTER_COUNT);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_TRAP00 + ulOffset);
    BKNI_Memcpy(pList->pulCurrent, s_aulGrpdFilters[ulVidIndex].aulTrap,
        sizeof(s_aulGrpdFilters[ulVidIndex].aulTrap));
    pList->pulCurrent += BVDC_P_TRAP_FILTER_COUNT;

    /* GRPD_0_CLIP0 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_CLIP0 + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_CLIP0, CLIP0_LO_TH, 0    ) |
        BCHP_FIELD_DATA(GRPD_0_CLIP0, CLIP0_HI_TH, 4095 );

    /* GRPD_0_CLIP1 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_CLIP1 + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_CLIP1, CLIP1_LO_TH, 0    ) |
        BCHP_FIELD_DATA(GRPD_0_CLIP1, CLIP1_HI_TH, 8191 );

    /* GRPD_0_CLIPBYP (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_CLIPBYP + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_CLIPBYP, BYP_CLIP1, 0 ) |
        BCHP_FIELD_DATA(GRPD_0_CLIPBYP, BYP_CLIP0, 0 );

    /* GRPD_0_VIDEOSCL (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_VIDEOSCL + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_VIDEOSCL, VIDEOOUTSCL, 302 ) |
        BCHP_FIELD_DATA(GRPD_0_VIDEOSCL, VIDEOINSCL,  32  );

    /* GRPD_0_AMCTL0 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_AMCTL0 + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_AMCTL0, DEPTH,    4096 ) |
        BCHP_FIELD_DATA(GRPD_0_AMCTL0, PREPILOT, 0    );

    /* GRPD_0_AMCTL1 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_AMCTL1 + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_AMCTL1, POSTPILOT, 0 ) |
        BCHP_FIELD_DATA(GRPD_0_AMCTL1, PRENEG,    0 );

    /* GRPD_0_DDFSFCW0 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_DDFSFCW0 + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_DDFSFCW0, DDFSFCW0, 7954 );

    /* GRPD_0_DDFSFCW1 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_DDFSFCW1 + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_DDFSFCW1, DDFSFCW1, 19884 );

    /* GRPD_0_DDFSSCL01 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_DDFSSCL01 + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_DDFSSCL01, SCLDDFS0, 2011 ) |
        BCHP_FIELD_DATA(GRPD_0_DDFSSCL01, SCLDDFS1, 1851 );

    /* GRPD_0_VIDEOBYP (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_VIDEOBYP + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_VIDEOBYP, TRAPBYP,  0 ) |
        BCHP_FIELD_DATA(GRPD_0_VIDEOBYP, BYP_GDY,  0 ) |
        BCHP_FIELD_DATA(GRPD_0_VIDEOBYP, BYP_QBPH, 0 ) |
        BCHP_FIELD_DATA(GRPD_0_VIDEOBYP, BYP_QB,   0 ) |
        BCHP_FIELD_DATA(GRPD_0_VIDEOBYP, BYP_HB,   0 ) |
        BCHP_FIELD_DATA(GRPD_0_VIDEOBYP, EN_TPIN,  0 ) |
        BCHP_FIELD_DATA(GRPD_0_VIDEOBYP, BYP_FV,   0 );

    /* GRPD_0_VIDEOTONE (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_VIDEOTONE + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_VIDEOTONE, DDFS_EN, 0 ) |
        BCHP_FIELD_DATA(GRPD_0_VIDEOTONE, FCW,     0 );

    /* GRPD_0_MODBYP (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_MODBYP + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_MODBYP, BYP_FILT, 0 ) |
        BCHP_FIELD_DATA(GRPD_0_MODBYP, SEL_MIX,  0 ) |
        BCHP_FIELD_DATA(GRPD_0_MODBYP, BYP_VID,  0 ) |
        BCHP_FIELD_DATA(GRPD_0_MODBYP, BYP_MIX,  0 ) |
        BCHP_FIELD_DATA(GRPD_0_MODBYP, BYP_SINC, 0 );

    /* GRPD_0_OUTSCL (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_GRPD_0_OUTSCL + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(GRPD_0_OUTSCL, EN,     0   ) |
        BCHP_FIELD_DATA(GRPD_0_OUTSCL, SCLOUT, 128 );

    /* GRPD_0_RESERVED0 (RW) */
    /* GRPD_0_RESERVED1 (RW) */
    /* VEC_CFG_GRPD_0_SOURCE (RW) */
#ifdef BCHP_VEC_CFG_GRPD_0_SOURCE_SOURCE_MASK
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_GRPD_0_SOURCE + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(VEC_CFG_GRPD_0_SOURCE, SOURCE, pChannel->ulSdsrc);
#endif
#else /* BVDC_P_SUPPORT_VEC_GRPD */
    BSTD_UNUSED(pFmtInfo);
    BSTD_UNUSED(pChannel);
    BSTD_UNUSED(pList);
#endif

    return;
}

/*************************************************************************
 *
 *  BVDC_P_Program_CSC_SRC_SM_isr
 *  Adds CSC, SRC and SM blocks to RUL for a display.
 *  Bypass - CSC
 *
 **************************************************************************/
static void BVDC_P_Vec_Build_CSC_SRC_SM_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayAnlgChan          *pstChan,
      BVDC_P_Output                    eOutputCS,
      BVDC_P_ListInfo                 *pList )
{
    const BVDC_P_DisplayCscMatrix *pCscMatrix = NULL;
    const uint32_t     *pTable = NULL;
    BVDC_P_DisplayInfo *pCurInfo = &hDisplay->stCurInfo;
    uint32_t            ulSrcControl;

    /* Setup Main CSC */
    if( eOutputCS != BVDC_P_Output_eUnknown )
    {
        /* Setup SRC */
        ulSrcControl = BVDC_P_GetSrcControl_isr(eOutputCS);
        BVDC_P_Vec_Build_SRC_isr(ulSrcControl, pstChan, pList);

        /* Setup SM */
        if (pstChan->ulSdsrc != BVDC_P_HW_ID_INVALID)
        {
            pTable = BVDC_P_GetSmTable_isr(pCurInfo, eOutputCS);
            BVDC_P_Vec_Build_SM_isr(pCurInfo->pFmtInfo->eVideoFmt, eOutputCS, pTable, pstChan, pList);
            if(pstChan->ulSecam != BVDC_P_HW_ID_INVALID)
            {
                BVDC_P_Vec_Build_SECAM_isr(pCurInfo->pFmtInfo->eVideoFmt, pstChan, pList);
            }

            /* TODO: Which versioning to use?  SM, SRC, or by itself? */
            /* SRC, has hardwired GRPD into it.  Need to separate if HW
             * introduces Mux after SRC to bypass or selectable GRPD_x. */
            BDVC_P_Vec_Build_Grpd_isr(pCurInfo->pFmtInfo, pstChan, pList);
        }

        /* Setup CSC for  */
        BVDC_P_Display_GetCscTable_isr(pCurInfo, eOutputCS, &pCscMatrix);

        pstChan->stCscMatrix = *pCscMatrix;

        /* TODO: handle user csc */
        /* Handle CSC mute */
        if (((pCurInfo->abOutputMute[BVDC_DisplayOutput_eComponent]) && BVDC_P_DISP_IS_ANLG_CHAN_CO(pstChan, pCurInfo)) ||
            ((pCurInfo->abOutputMute[BVDC_DisplayOutput_eComposite]) && BVDC_P_DISP_IS_ANLG_CHAN_CVBS(pstChan, pCurInfo)) ||
            ((pCurInfo->abOutputMute[BVDC_DisplayOutput_eSVideo]) && BVDC_P_DISP_IS_ANLG_CHAN_SVIDEO(pstChan, pCurInfo)))
        {
            const BVDC_P_Compositor_Info *pCmpInfo = &hDisplay->hCompositor->stCurInfo;
            uint8_t ucCh0 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 2);
            uint8_t ucCh1 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 1);
            uint8_t ucCh2 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 0);

            /* Swap ch0 and 1 of input color to match vec csc layout */
            BVDC_P_Csc_ApplyYCbCrColor_isr(&pstChan->stCscMatrix.stCscCoeffs, ucCh1, ucCh0, ucCh2);
        }

        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_CSC_TABLE_SIZE);
        /* CSC module pairs with VF */
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_CSC_0_CSC_MODE + pstChan->ulCscRegOffset);

        BVDC_P_Vec_Build_CSC_isr(&pstChan->stCscMatrix, pList);
    }

    return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_Build_VF_isr
 *  Builds either: SEC_VF, SEC_VF_CO or PRIM_VF
 *  Required for Videoformat or colorspace change
 **************************************************************************/
static void BVDC_P_Vec_Build_VF_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayAnlgChan          *pstChan,
      BVDC_P_Output                    eOutputCS,
      BVDC_P_ListInfo                 *pList )
{
    BVDC_P_Display_ShaperSettings stShaperSettings;
    uint32_t             ulNsaeReg;
    BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;
    uint32_t             ulOffset = pstChan->ulVfRegOffset;
    uint32_t             i=0;
    uint32_t             ulVal;
    uint32_t             ulValEx;
    uint32_t             ulVfMiscRegIdx    , ulVfMiscRegValue ;
    uint32_t             ulVfFmtAdderRegIdx, ulVfFmtAdderValue;
    uint32_t             ulSumOfTapsBits;
    bool                 bOverride = false;

    /* Channel band-limiting filter
                        Ch0    Ch1    Ch2
    YQI (NTSC,NTSC_J)   6.0    0.6    1.3
    YUV (M,N,NC)        4.2    1.3    1.3
    YUV (B,B1,D1,G,H)   5.0    1.3    1.3
    YUV (I)             5.5    1.3    1.3
    YUV (D,K,K1)        6.0    1.3    1.3
    RGB                 6.75   6.75   6.75
    YPrPb               6.75   3.375  3.375 */

    /* get the default channel filters */
    BVDC_P_GetChFilters_isr(pCurInfo, eOutputCS, &pstChan->apVfFilter[0],
        &pstChan->apVfFilter[1], &pstChan->apVfFilter[2]);

    /* override with user filters */
    for (i=0; i < BVDC_P_VEC_CH_NUM; i++)
    {
        if (pCurInfo->abUserVfFilterCo[i] &&
            BVDC_P_DISP_IS_ANLG_CHAN_CO(pstChan, pCurInfo))
        {
            pstChan->apVfFilter[i] = pCurInfo->aaulUserVfFilterCo[i];
            if (BVDC_P_GetVfFilterSumOfTapsBits_isr (
                pCurInfo, BVDC_DisplayOutput_eComponent, &ulSumOfTapsBits,
                &bOverride) != BERR_SUCCESS)
            {
                /* Should have been caught in validate stage */
                BDBG_ASSERT(0);
            }
        }
        else if (pCurInfo->abUserVfFilterCvbs[i] &&
                 BVDC_P_DISP_IS_ANLG_CHAN_CVBS(pstChan, pCurInfo))
        {
            pstChan->apVfFilter[i] = pCurInfo->aaulUserVfFilterCvbs[i];
            if (BVDC_P_GetVfFilterSumOfTapsBits_isr (
                pCurInfo, BVDC_DisplayOutput_eComposite, &ulSumOfTapsBits,
                &bOverride) != BERR_SUCCESS)
            {
                /* Should have been caught in validate stage */
                BDBG_ASSERT(0);
            }
        }
    }
    /* Still need to process the modified VF_MISC register value */

    /* get the correct vf table */
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_VF_TABLE_SIZE);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VF_0_FORMAT_ADDER + ulOffset);
    BVDC_P_FillVfTable_isr (
        pCurInfo, eOutputCS, pList->pulCurrent, &ulNsaeReg, &stShaperSettings);

    /* Record some values and their locations */
    ulVfMiscRegIdx =
        (BCHP_VF_0_MISC - BCHP_VF_0_FORMAT_ADDER) / sizeof(uint32_t);
    ulVfMiscRegValue = pList->pulCurrent[ulVfMiscRegIdx];
    ulVfFmtAdderRegIdx =
        (BCHP_VF_0_FORMAT_ADDER - BCHP_VF_0_FORMAT_ADDER) / sizeof(uint32_t);
    ulVfFmtAdderValue = pList->pulCurrent[ulVfFmtAdderRegIdx];

    /* Modified sum of taps? */
    if (bOverride)
    {
        ulVfMiscRegValue &=
            ~BCHP_MASK      (VF_0_MISC, SUM_OF_TAPS             );
        ulVfMiscRegValue |=
             BCHP_FIELD_DATA(VF_0_MISC, SUM_OF_TAPS, ulSumOfTapsBits);
    }

    /* prepare for setting vec BVB left cut */
    ulVfMiscRegValue &=
        ~BCHP_MASK(VF_0_MISC, BVB_SAV_REMOVE);
    ulVfMiscRegValue |=
        BCHP_FIELD_DATA(
            VF_0_MISC, BVB_SAV_REMOVE, stShaperSettings.ulSavRemove);

    /* SECAM is a special case. This bitfield value does not follow colorspace
     * like other formats. */
    if (BFMT_IS_SECAM(pCurInfo->pFmtInfo->eVideoFmt))
    {
        ulVfMiscRegValue &=
            ~BCHP_FIELD_DATA(VF_0_MISC, BVB_LINE_REMOVE_TOP, 1);
    }

    /* PAL line 23 is reserved for WSS vbi data; Cx and beyond can drop the
     * 1st active video line from BVN; */
    else if (BFMT_IS_625_LINES(pCurInfo->pFmtInfo->eVideoFmt))
    {
        ulVfMiscRegValue |=
            BCHP_FIELD_DATA(VF_0_MISC, BVB_LINE_REMOVE_TOP, 1);
    }

    /* Also for PAL-N. */
    else if ((pCurInfo->pFmtInfo->eVideoFmt == BFMT_VideoFmt_ePAL_N) &&
             ((pCurInfo->eMacrovisionType ==
                BVDC_MacrovisionType_eNoProtection) ||
             (pCurInfo->eMacrovisionType ==
                BVDC_MacrovisionType_eTest01) ||
             (pCurInfo->eMacrovisionType ==
                BVDC_MacrovisionType_eTest02)) )
    {
        ulVfMiscRegValue |=
            BCHP_FIELD_DATA(VF_0_MISC, BVB_LINE_REMOVE_TOP, 1);
    }

    /* Done with MISC register */
    pList->pulCurrent[ulVfMiscRegIdx] = ulVfMiscRegValue;  /* override */
    pstChan->vfMisc = ulVfMiscRegValue;

    /* SD with RGB? */
    if ( (VIDEO_FORMAT_IS_SD(pCurInfo->pFmtInfo->eVideoFmt)) &&
         ((eOutputCS == BVDC_P_Output_eSDRGB) ||
          (eOutputCS == BVDC_P_Output_eHDRGB)) )
    {
        ulVfFmtAdderValue = BVDC_P_GetFmtAdderValue_isr(pCurInfo);
    }

    if ( (pCurInfo->stN0Bits.bPsAgc) &&
         ((eOutputCS != BVDC_P_Output_eSDRGB) &&
          (eOutputCS != BVDC_P_Output_eHDRGB)) )
    {
        ulVfFmtAdderValue |=
            BCHP_MASK(VF_0_FORMAT_ADDER, SECOND_NEGATIVE_SYNC);
    }

#if DCS_SUPPORT
    if (VIDEO_FORMAT_SUPPORTS_DCS (pCurInfo->pFmtInfo->eVideoFmt))
    {
        ulVfFmtAdderValue &=
            ~BCHP_MASK       (VF_0_FORMAT_ADDER, C0_POSITIVESYNC);
        ulVfFmtAdderValue |=
             BCHP_FIELD_DATA (VF_0_FORMAT_ADDER, C0_POSITIVESYNC, 1);
    }
#endif

    /* Alternate sync arrangement? */
    if (!(hDisplay->bModifiedSync) && (eOutputCS == BVDC_P_Output_eHDYPrPb))
    {
        switch (pCurInfo->pFmtInfo->eVideoFmt)
        {
        case BFMT_VideoFmt_e720p:
        case BFMT_VideoFmt_e720p_24Hz:
        case BFMT_VideoFmt_e720p_25Hz:
        case BFMT_VideoFmt_e720p_30Hz:
        case BFMT_VideoFmt_e720p_50Hz:
        case BFMT_VideoFmt_e1080i:
        case BFMT_VideoFmt_e1080i_50Hz:
        case BFMT_VideoFmt_e1250i_50Hz:
        case BFMT_VideoFmt_e1080p:
        case BFMT_VideoFmt_e1080p_24Hz:
        case BFMT_VideoFmt_e1080p_25Hz:
        case BFMT_VideoFmt_e1080p_30Hz:
        case BFMT_VideoFmt_e1080p_50Hz:
        case BFMT_VideoFmt_e1080p_100Hz:
        case BFMT_VideoFmt_e1080p_120Hz:
            ulVfFmtAdderValue &= ~(
                BCHP_MASK       (VF_0_FORMAT_ADDER,    C2_POSITIVESYNC) |
                BCHP_MASK       (VF_0_FORMAT_ADDER,    C1_POSITIVESYNC) |
                BCHP_MASK       (VF_0_FORMAT_ADDER, ADD_SYNC_TO_OFFSET) |
                BCHP_MASK       (VF_0_FORMAT_ADDER,             OFFSET) );
            ulVfFmtAdderValue |= (
                BCHP_FIELD_DATA (VF_0_FORMAT_ADDER,    C2_POSITIVESYNC,     0) |
                BCHP_FIELD_DATA (VF_0_FORMAT_ADDER,    C1_POSITIVESYNC,     0) |
                BCHP_FIELD_DATA (VF_0_FORMAT_ADDER, ADD_SYNC_TO_OFFSET,     0) |
                BCHP_FIELD_DATA (VF_0_FORMAT_ADDER,             OFFSET, 0x1EE));
            break;
        default:
            break;
        }
    }

    /* Done with FORMAT_ADDER register */
    pList->pulCurrent[ulVfFmtAdderRegIdx] = ulVfFmtAdderValue;  /* override */

    /* Done with the first part of the table */
    pList->pulCurrent += BVDC_P_VF_TABLE_SIZE;

    /* Setup Channel VF filters */
    *pList->pulCurrent++ =
        BRDC_OP_IMMS_TO_REGS(BVDC_P_VEC_CH_NUM * BVDC_P_CHROMA_TABLE_SIZE);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VF_0_CH0_TAP1 + ulOffset);
    for (i=0; i < BVDC_P_VEC_CH_NUM; i++)
    {
        BKNI_Memcpy((void*)pList->pulCurrent, (void*)pstChan->apVfFilter[i],
            BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
        pList->pulCurrent += BVDC_P_CHROMA_TABLE_SIZE;
    }

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VF_0_SHAPER + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(VF_0_SHAPER, SAV_REPLICATE, stShaperSettings.ulSavReplicate) |
        BCHP_FIELD_DATA(VF_0_SHAPER, EAV_PREDICT,   stShaperSettings.ulEavPredict  ) |
        BCHP_FIELD_DATA(VF_0_SHAPER, THRESHOLD,     BVDC_P_VF_THRESH)    |
        BCHP_FIELD_DATA(VF_0_SHAPER, CONTROL,       BVDC_P_VF_ENABLE);

/* Unfortunately, part of NEG_SYNC_AMPLITUDE is in a distant register. */
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_VF_0_NEG_SYNC_AMPLITUDE_EXTN + ulOffset);
        *pList->pulCurrent++ = ulNsaeReg;
#endif

#if DCS_SUPPORT /** { **/
#ifndef BVDC_P_SMOOTH_DCS /** { **/
    if (VIDEO_FORMAT_SUPPORTS_DCS (pCurInfo->pFmtInfo->eVideoFmt))
    {
        bool bComposite =
            (BVDC_P_DISP_IS_ANLG_CHAN_CVBS  (pstChan,pCurInfo) ||
             BVDC_P_DISP_IS_ANLG_CHAN_SVIDEO(pstChan,pCurInfo)     );
        const BVDC_P_DCS_VFvalues* pVfValues =
            BVDC_P_DCS_GetVFvalues_isr (
                pCurInfo->pFmtInfo->eVideoFmt, bComposite, pCurInfo->eDcsMode);
        BVDC_P_DCS_VF_Update_isr (pVfValues, ulOffset, &(pList->pulCurrent));
    }
#endif /** } ! BVDC_P_SMOOTH_DCS **/
#endif /** } DCS_SUPPORT **/

    /* NTSC/PAL could have sync reduction */
    /* RGB could have no sync on green */
    if((VIDEO_FORMAT_IS_SD(pCurInfo->pFmtInfo->eVideoFmt)) &&
        (eOutputCS != BVDC_P_Output_eUnknown))
    {
        /* Note: for RGB with external sync,
         * need to remove sync signals from G channel; */
        if (BVDC_P_Display_FindDac_isr(hDisplay, BVDC_DacOutput_eHsync))
        {
            ulVal   = 0;
            ulValEx = 0;
        }
        else
        {
            bool bDacOutput_Green_NoSync =
                (BVDC_P_Display_FindDac_isr(
                    hDisplay, BVDC_DacOutput_eGreen_NoSync));
            BVDC_P_Macrovision_GetNegSyncValue_isr(
                pCurInfo, eOutputCS, bDacOutput_Green_NoSync, &ulVal, &ulValEx);
        }

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_VF_0_NEG_SYNC_VALUES + ulOffset);
        *pList->pulCurrent++ = ulVal;
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 1)
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_VF_0_NEG_SYNC_AMPLITUDE_EXTN + ulOffset);
        *pList->pulCurrent++ = ulValEx;
#endif
    }

    /* Set up envelop generator */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VF_0_ENVELOPE_GENERATOR + ulOffset);
    *pList->pulCurrent++ = BVDC_P_GetVfEnvelopGenerator_isr(pCurInfo, eOutputCS);

#if (BVDC_P_SUPPORT_VEC_VF_VER > 1)
    /* Program DRAIN_PIXELS_SCART for CVBS channel when it's in scart mode */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VF_0_DRAIN_PIXELS_SCART + ulOffset);
    if(pCurInfo->bRgb && pCurInfo->bCvbs && BVDC_P_DISP_IS_ANLG_CHAN_CVBS(pstChan, pCurInfo))
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(VF_0_DRAIN_PIXELS_SCART, ENABLE, ENABLE) |
            BCHP_FIELD_DATA(VF_0_DRAIN_PIXELS_SCART, NUM_OF_SAMPLES,
                BFMT_IS_NTSC(pCurInfo->pFmtInfo->eVideoFmt) ? 15 : 14);
    }
    else
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(VF_0_DRAIN_PIXELS_SCART, ENABLE, DISABLE) |
            BCHP_FIELD_DATA(VF_0_DRAIN_PIXELS_SCART, NUM_OF_SAMPLES, 0);
    }
#endif
    return;
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_Build_RM_isr
 **************************************************************************/
static void BVDC_P_Vec_Build_RM_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayAnlgChan          *pstChan,
      BVDC_P_ListInfo                 *pList  )
{
    /* RM pairs with IT */
    BVDC_P_DisplayInfo   *pCurInfo = &hDisplay->stCurInfo;
    const uint32_t       *pRmTable;
    BAVC_VdcDisplay_Info  lRateInfo;
    BAVC_VdcDisplay_Info  stPrevRateInfo;

    BDBG_ENTER(BVDC_P_Vec_Build_RM_isr);

    stPrevRateInfo = pCurInfo->stRateInfo;
    if(BVDC_P_GetRmTable_isr(pCurInfo, pCurInfo->pFmtInfo, &pRmTable, pCurInfo->bFullRate, &lRateInfo) != BERR_SUCCESS)
        BDBG_ASSERT(0);
    pCurInfo->stRateInfo = lRateInfo;
    pCurInfo->ulVertFreq = lRateInfo.ulVertRefreshRate;
    hDisplay->pRmTable = pRmTable;

    /* Notify external modules that Rate Manager has been changed. */
    if((lRateInfo.ulPixelClkRate    != stPrevRateInfo.ulPixelClkRate   ) ||
       (lRateInfo.ulVertRefreshRate != stPrevRateInfo.ulVertRefreshRate))
    {
        BDBG_MSG(("    VEC's RM PxlClk = %sMHz, RefRate = %d (1/%dth Hz), MultiRate=%d",
            BVDC_P_GetRmString_isr(pCurInfo, pCurInfo->pFmtInfo),
            pCurInfo->ulVertFreq, BFMT_FREQ_FACTOR, pCurInfo->bMultiRateAllow));
        hDisplay->bRateManagerUpdated = true;
    }

    /* --- Setup RM --- */
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_RM_TABLE_SIZE);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_RM_0_RATE_RATIO + pstChan->ulRmRegOffset);

    /* Setup RM_RATE_RATIO to RM_INTEGRATOR */
    BKNI_Memcpy((void*)pList->pulCurrent, (void*)pRmTable,
        BVDC_P_RM_TABLE_SIZE * sizeof(uint32_t));
    pList->pulCurrent += BVDC_P_RM_TABLE_SIZE;

    BDBG_LEAVE(BVDC_P_Vec_Build_RM_isr);

    return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Display_Build_ItBvbSize_isr
 *  Builds IT BVB Size register
 **************************************************************************/
static void BVDC_P_Display_Build_ItBvbSize_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      uint32_t                         ulOffset )
{
    BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;
    uint32_t ulHeight = pCurInfo->ulHeight;

    if(BFMT_IS_3D_MODE(pCurInfo->pFmtInfo->eVideoFmt))
    {
        ulHeight = ulHeight * 2 + pCurInfo->pFmtInfo->ulActiveSpace;
    }

    /* Update IT BVB size */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_IT_0_BVB_SIZE + ulOffset);
    if(pCurInfo->bWidthTrimmed &&
       BFMT_IS_NTSC(pCurInfo->pFmtInfo->eVideoFmt) &&
       (pCurInfo->eMacrovisionType < BVDC_MacrovisionType_eTest01))
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(IT_0_BVB_SIZE, HORIZONTAL,  712) |
            BCHP_FIELD_DATA(IT_0_BVB_SIZE, VERTICAL,    ulHeight);
    }
    else
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(IT_0_BVB_SIZE, HORIZONTAL,  pCurInfo->pFmtInfo->ulWidth) |
            BCHP_FIELD_DATA(IT_0_BVB_SIZE, VERTICAL,    ulHeight);
    }

    return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_Build_ItMc_isr
 **************************************************************************/
static void BVDC_P_Vec_Build_ItMc_isr
    ( BVDC_P_ListInfo                 *pList,
      const uint32_t                  *pRamTbl,
      uint32_t                         ulOffset )
{
    /* Write IT_MICRO_INSTRUCTIONS 0..255 */
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_RAM_TABLE_SIZE);
    *pList->pulCurrent++ =
        BRDC_REGISTER(BCHP_IT_0_MICRO_INSTRUCTIONi_ARRAY_BASE + ulOffset);

    BKNI_Memcpy((void*)pList->pulCurrent, (void*)pRamTbl,
        BVDC_P_RAM_TABLE_SIZE * sizeof(uint32_t));
    pList->pulCurrent += BVDC_P_RAM_TABLE_SIZE;

    return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_Build_It3D_isr
 **************************************************************************/
static void BVDC_P_Vec_Build_It3D_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t ulLineNum, ulStartLine;
    BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;

    /* TODO */
    ulStartLine = pCurInfo->pFmtInfo->ulHeight;
    ulLineNum = ulStartLine + pCurInfo->pFmtInfo->ulActiveSpace + 1;

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_IT_0_AS_CONTROL + hDisplay->stAnlgChan_0.ulItRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(IT_0_AS_CONTROL, AS_INPUT_TYPE, COMBINED   ) |
        BCHP_FIELD_DATA(IT_0_AS_CONTROL, AS_NUM_LINES,  pCurInfo->pFmtInfo->ulActiveSpace) |
        BCHP_FIELD_DATA(IT_0_AS_CONTROL, AS_START_LINE, ulStartLine) |
        (BFMT_IS_3D_MODE(pCurInfo->pFmtInfo->eVideoFmt) ?
          BCHP_FIELD_DATA(IT_0_AS_CONTROL, AS_ENABLE, 1) :
          BCHP_FIELD_DATA(IT_0_AS_CONTROL, AS_ENABLE, 0));

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_IT_0_AS_LINE_NUMBER + hDisplay->stAnlgChan_0.ulItRegOffset);
    *pList->pulCurrent++ = BCHP_FIELD_DATA(IT_0_AS_LINE_NUMBER, LINE_NUMBER, ulLineNum);

    return;
}


/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_Build_IT_isr
 *  Builds IT and Video_Enc blocks (for mode switch)
 **************************************************************************/
static void BVDC_P_Vec_Build_IT_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayAnlgChan          *pstChan,
      bool                             bLoadMicrocode,
      BVDC_P_ListInfo                 *pList )
{
    const uint32_t      *pRamTbl = NULL;
    const uint32_t      *pTable  = NULL;
    BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;
    uint32_t             ulOffset = pstChan->ulItRegOffset;
    uint32_t             ulTopTrigVal, ulBotTrigVal;

    /* Compute Trigger Values.  For pal the Top trigger is in field1.
       Note: it seems the new PAL microcode corrected the trigger polarity, so we
       don't need to swap it now; */
    ulTopTrigVal = BVDC_P_TRIGGER_LINE;
    ulBotTrigVal = (pCurInfo->pFmtInfo->bInterlaced)
        ? (((pCurInfo->pFmtInfo->ulScanHeight + 1) / 2) + BVDC_P_TRIGGER_LINE) : 0;

#if (BVDC_P_SUPPORT_IT_VER >= 2)
    /* Having IT trigger modulo count set to 2 means the chipset has
     * a OSCL which produces 1080p output. BVN still runs as 1080i.
     */
    if (1 != pCurInfo->ulTriggerModuloCnt)
        ulBotTrigVal = BVDC_P_TRIGGER_LINE;
#endif

    pTable     = BVDC_P_GetItTable_isr(pCurInfo);
    BDBG_ASSERT (pTable);
    pRamTbl    = BVDC_P_GetRamTable_isr(pCurInfo, hDisplay->bArib480p);
    BDBG_ASSERT (pRamTbl);

    if (bLoadMicrocode)
    {
        BDBG_MSG(("Analog vec microcode - timestamp: 0x%.8x",
            pRamTbl[BVDC_P_RAM_TABLE_TIMESTAMP_IDX]));
        BDBG_MSG(("Analog vec microcode - checksum:  0x%.8x",
            pRamTbl[BVDC_P_RAM_TABLE_CHECKSUM_IDX]));
    }

    /* to support MV mode control byte change */
#if DCS_SUPPORT /** { **/

    if (bLoadMicrocode)
    {
#ifndef BVDC_P_SMOOTH_DCS /** { **/
        bool bDcsOn =
            (pCurInfo->eDcsMode != BVDC_DCS_Mode_eOff      ) &&
            (pCurInfo->eDcsMode != BVDC_DCS_Mode_eUndefined)    ;
#endif /** } ! BVDC_P_SMOOTH_DCS **/

        /* --- Setup IT block --- */
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_IT_TABLE_SIZE);
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_IT_0_ADDR_0_3 + ulOffset);

        /* Setup IT_ADDR_0_3 to IT_STACK_REG_8_9 */
        BKNI_Memcpy((void*)pList->pulCurrent, (void*)pTable,
            BVDC_P_IT_TABLE_SIZE * sizeof(uint32_t));

        /* move pointer after adjustment */
        pList->pulCurrent += BVDC_P_IT_TABLE_SIZE;

#ifndef BVDC_P_SMOOTH_DCS /** { **/
        if (VIDEO_FORMAT_SUPPORTS_DCS (pCurInfo->pFmtInfo->eVideoFmt))
        {
            uint32_t ulItConfig = BVDC_P_GetItConfig_isr(pCurInfo);
            BDBG_ASSERT (ulItConfig);
            uint32_t ulItConfigOff =
                ulItConfig & ~BCHP_MASK(IT_0_TG_CONFIG, MC_ENABLES);

            BVDC_P_DCS_IT_ON_OFF_isr (
                ulOffset, pCurInfo->pFmtInfo->eVideoFmt, bDcsOn,
                &(pList->pulCurrent));
            BVDC_P_DCS_IT_Final_ON_OFF_isr (
                ulOffset, pCurInfo->pFmtInfo->eVideoFmt, bDcsOn,
                &(pList->pulCurrent));

            /* Hardware problem
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ =
                BRDC_REGISTER(BCHP_IT_0_TG_CONFIG + ulOffset);
            *pList->pulCurrent++ = ulItConfigOff;
            *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
            *pList->pulCurrent++ =
                BRDC_REGISTER(BCHP_IT_0_TG_CONFIG + ulOffset);
            *pList->pulCurrent++ = ulItConfig;
            */
        }
#endif /** } ! BVDC_P_SMOOTH_DCS **/
    }

#else /** } DCS_SUPPORT { **/

    if (VIDEO_FORMAT_SUPPORTS_MACROVISION(pCurInfo->pFmtInfo->eVideoFmt))
    {
        uint32_t ulAddr03Offset;
        uint32_t ulAddr46Offset;
        uint32_t ulPcl4Offset;
        uint32_t ulAddr03;
        uint32_t ulAddr46;
        uint32_t ulPcl4;

        /* --- Setup IT block --- */
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_IT_TABLE_SIZE);
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_IT_0_ADDR_0_3 + ulOffset);

        /* Setup IT_ADDR_0_3 to IT_STACK_REG_8_9 */
        BKNI_Memcpy((void*)pList->pulCurrent, (void*)pTable,
            BVDC_P_IT_TABLE_SIZE * sizeof(uint32_t));

        ulAddr03Offset = 0x0;
        ulAddr46Offset =
            (BCHP_IT_0_ADDR_4_6 - BCHP_IT_0_ADDR_0_3)/sizeof(uint32_t);
        ulPcl4Offset =
            (BCHP_IT_0_PCL_4 - BCHP_IT_0_ADDR_0_3)/sizeof(uint32_t);
        ulAddr03 = pTable[ulAddr03Offset];
        ulAddr46 = pTable[ulAddr46Offset];
        ulPcl4   = pTable[ulPcl4Offset];

        /* if toggle N0 control bits for CS(MC4/5) or BP(PCL_4) */
        if(!pCurInfo->stN0Bits.bBp)
        {
            ulPcl4 &= ~(BCHP_MASK(IT_0_PCL_4, PSB_AND_TERM_2));
            ulPcl4 |= BCHP_FIELD_ENUM(IT_0_PCL_4, PSB_AND_TERM_2, ZERO);
            *(pList->pulCurrent + ulPcl4Offset) = ulPcl4;
        }
        if(!pCurInfo->stN0Bits.bPsAgc)
        {
            ulAddr03 &= ~(
                BCHP_MASK(IT_0_ADDR_0_3, MC_2_START_ADDR) |
                BCHP_MASK(IT_0_ADDR_0_3, MC_3_START_ADDR));
            ulAddr03 |= (
                BCHP_FIELD_DATA(IT_0_ADDR_0_3, MC_2_START_ADDR, 0xfd) |
                BCHP_FIELD_DATA(IT_0_ADDR_0_3, MC_3_START_ADDR, 0xfd));

            *(pList->pulCurrent + ulAddr03Offset) = ulAddr03;
        }
        if(!pCurInfo->stN0Bits.bCs && (!(BFMT_IS_SECAM(pCurInfo->pFmtInfo->eVideoFmt))))
        {
            ulAddr46 &= ~(
                BCHP_MASK(IT_0_ADDR_4_6, MC_4_START_ADDR) |
                BCHP_MASK(IT_0_ADDR_4_6, MC_5_START_ADDR));
            ulAddr46 |= (
                BCHP_FIELD_DATA(IT_0_ADDR_4_6, MC_4_START_ADDR, 0xfd) |
                BCHP_FIELD_DATA(IT_0_ADDR_4_6, MC_5_START_ADDR, 0xfd));

            *(pList->pulCurrent + ulAddr46Offset) = ulAddr46;
        }

        /* move pointer after adjustment */
        pList->pulCurrent += BVDC_P_IT_TABLE_SIZE;
    }
    else
    {
        /* --- Setup IT block --- */
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_IT_TABLE_SIZE);
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_IT_0_ADDR_0_3 + ulOffset);

        /* Setup IT_ADDR_0_3 to IT_STACK_REG_8_9 */
        BKNI_Memcpy((void*)pList->pulCurrent, (void*)pTable,
            BVDC_P_IT_TABLE_SIZE * sizeof(uint32_t));

        /* move pointer after adjustment */
        pList->pulCurrent += BVDC_P_IT_TABLE_SIZE;
    }

#endif /** } !DCS_SUPPORT **/

    BVDC_P_Vec_Build_It3D_isr(hDisplay, pList);

#if (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_9)
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_IT_0_CABLE_DETECT_SEL + ulOffset);
    *pList->pulCurrent++ =
#if BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND
        BCHP_FIELD_DATA(IT_0_CABLE_DETECT_SEL, LINE,
            (BFMT_IS_NTSC(pCurInfo->pFmtInfo->eVideoFmt)   ? 2 :
            (VIDEO_FORMAT_IS_HD(pCurInfo->pFmtInfo->eVideoFmt) &&
             !VIDEO_FORMAT_IS_ED(pCurInfo->pFmtInfo->eVideoFmt))   ? 6 :
            (BFMT_IS_480P(pCurInfo->pFmtInfo->eVideoFmt)) ? 4 : 1)) |
        BCHP_FIELD_ENUM(IT_0_CABLE_DETECT_SEL, SEL,  FLAG0          );
#else
        BCHP_FIELD_DATA(IT_0_CABLE_DETECT_SEL, LINE,
            (BFMT_IS_NTSC(pCurInfo->pFmtInfo->eVideoFmt)   ? 4 :
            (VIDEO_FORMAT_IS_HD(pCurInfo->pFmtInfo->eVideoFmt) &&
             !VIDEO_FORMAT_IS_ED(pCurInfo->pFmtInfo->eVideoFmt))   ? 6 :
            (BFMT_IS_480P(pCurInfo->pFmtInfo->eVideoFmt)) ? 7 : 1)) |
        BCHP_FIELD_ENUM(IT_0_CABLE_DETECT_SEL, SEL,  FLAG0          );
#endif
#endif

    if (bLoadMicrocode)
    {
        BVDC_P_Display_Build_ItBvbSize_isr(hDisplay, pList, ulOffset);

        /* Setup Vec triggers */
        /* Must be in the Rul, otherwise the Reset will destroy it */
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(2);
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_IT_0_VEC_TRIGGER_0 + ulOffset);

        /* Set 1st trigger */
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_0, TRIGGER_VALUE, ulTopTrigVal) |
#if (BVDC_P_SUPPORT_IT_VER >= 2)
            BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_0, TRIGGER_SELECT, 0) |
            BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_0, MODULO_COUNT, pCurInfo->ulTriggerModuloCnt) |
#endif
            BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_0, ENABLE, 0);

        /* Set 2nd trigger (for interlaced modes) */
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_1, TRIGGER_VALUE, ulBotTrigVal) |
#if (BVDC_P_SUPPORT_IT_VER >= 2)
            BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_1, TRIGGER_SELECT, (1 != pCurInfo->ulTriggerModuloCnt) ? 1 : 0) |
            BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_1, MODULO_COUNT, pCurInfo->ulTriggerModuloCnt) |
#endif
            BCHP_FIELD_DATA(IT_0_VEC_TRIGGER_1, ENABLE, 0);

        BVDC_P_Vec_Build_ItMc_isr(pList, pRamTbl, ulOffset);
    }

    return;
}


static void BVDC_P_ProgramIT_isr
    ( BVDC_Display_Handle                 hDisplay,
      BVDC_P_DisplayAnlgChan             *pstChan,
      bool                                bReloadMicrocode,
      BVDC_P_ListInfo                    *pList )
{
    /*
     * Program IT and RM for analog channel 0. Analog channel 1 shares
     * the same IT and RM with channel 0.
     *
     * Note: programming cores like CSC, SRC, SM and VF depends on
     * the value of output color space, which is computed from DAC settings.
     * Hence those cores get programmed at BVDC_P_Display_Apply_DAC_Setting().
     */
    if (pstChan->ulId == 0)
    {
        BVDC_P_Vec_Build_IT_isr(hDisplay, pstChan, bReloadMicrocode, pList);
        BVDC_P_Vec_Build_RM_isr(hDisplay, pstChan, pList);
        BVDC_P_Display_SetupAnlgTG_isr(hDisplay, pList);
    }

    return;
}

static void BVDC_P_ProgramAnalogChan_isr
    ( BVDC_Display_Handle                 hDisplay,
      BVDC_P_DisplayAnlgChan             *pstChan,
      BVDC_P_ListInfo                    *pList )
{
    BVDC_P_Output eOutputCS;

    if(pstChan->ulId == 0)
    {
        eOutputCS = hDisplay->stCurInfo.eAnlg_0_OutputColorSpace;
        BDBG_ASSERT(stVDC_P_Output_InfoTbl[eOutputCS].eVdcOutput == hDisplay->stCurInfo.eAnlg_0_OutputColorSpace);
    }
    else
    {
        eOutputCS = hDisplay->stCurInfo.eAnlg_1_OutputColorSpace;
        BDBG_ASSERT(stVDC_P_Output_InfoTbl[eOutputCS].eVdcOutput == hDisplay->stCurInfo.eAnlg_1_OutputColorSpace);
    }
    BDBG_MSG(("Display %d Anlg %d using %s", hDisplay->eId, pstChan->ulId, stVDC_P_Output_InfoTbl[eOutputCS].pcVdcOutputStr));
    BVDC_P_Vec_Build_CSC_SRC_SM_isr(hDisplay, pstChan, eOutputCS, pList);
    BVDC_P_Vec_Build_VF_isr(hDisplay, pstChan, eOutputCS, pList);
    return;
}

void BVDC_P_ResetDviChanInfo
    ( BVDC_P_DisplayDviChan           *pstChan )
{
    pstChan->bEnable    = false; /* off */
    pstChan->ulDvi       = BVDC_P_HW_ID_INVALID;

    return;
}

BERR_Code BVDC_P_AllocDviChanResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BREG_Handle                      hRegister,
      BVDC_DisplayId                   eDisplayId,
      uint32_t                         ulHdmi,
      BVDC_P_DisplayDviChan           *pstChan,
      uint32_t                         ulSrcId )
{
    BERR_Code err = BERR_SUCCESS;
    uint32_t  ulHdmiCap = (ulHdmi == BVDC_Hdmi_1) ? BVDC_P_Able_eHdmi1 : BVDC_P_Able_eHdmi0;

#if BVDC_P_SUPPORT_MHL
#if BVDC_P_FORCED_MHL_MODE
    BSTD_UNUSED(hRegister);
#else
    uint32_t ulData;
#endif
#else
    BSTD_UNUSED(hRegister);
#endif

    BSTD_UNUSED(ulSrcId);

    if(pstChan->ulDvi == BVDC_P_HW_ID_INVALID)
    {
        err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eDvi, ulHdmiCap, eDisplayId, &pstChan->ulDvi, false);
        if (err)
        {
            BDBG_ERR(("No DVI block available for display %d. Check hardware capability.", eDisplayId));
        }
        else
        {

#if BVDC_P_SUPPORT_MHL

#if BVDC_P_FORCED_MHL_MODE
            pstChan->bMhlMode = true;
#else
            ulData = BREG_Read32(hRegister, BCHP_MPM_CPU_CTRL_STATUS);
            pstChan->bMhlMode = BCHP_GET_FIELD_DATA(ulData, MPM_CPU_CTRL_STATUS, STRAP_MHL_POWERUP)?true:false;
#endif
            BDBG_MSG(("     MHL mode = %d", pstChan->bMhlMode));
#endif

            BVDC_P_Display_CalculateOffset_isr(NULL, pstChan, BVDC_P_ResourceType_eDvi);
            BDBG_MSG(("     ulDvi = %d DviOffset=0x%08x DvpOffset=0x%08x",
                pstChan->ulDvi, pstChan->ulDviRegOffset, pstChan->ulDvpRegOffset));
        }
    }
    return BERR_TRACE(err);
}

void BVDC_P_FreeDviChanResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayDviChan           *pstChan )
{
    if (pstChan->ulDvi != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eDvi, pstChan->ulDvi);
        pstChan->ulDvi = BVDC_P_HW_ID_INVALID;
#if BVDC_P_SUPPORT_MHL
        pstChan->bMhlMode = false;
#endif
    }

#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY0
    if(hDisplay->ulHdmiPwrAcquire != 0)
    {
        hDisplay->ulHdmiPwrAcquire--;
        hDisplay->ulHdmiPwrRelease = 1;
        BDBG_MSG(("hdmi slave mode disable: Release pending BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY"));
    }
#else
    BSTD_UNUSED(hDisplay);
#endif

    return;
}

static void BVDC_P_Vec_Build_Dscl_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
#if BVDC_P_SUPPORT_DSCL
    /* These are static registers */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_TOP_CONTROL);
    *pList->pulCurrent++ =
#if (BVDC_P_SUPPORT_DSCL_VER >= 2)
        BCHP_FIELD_ENUM(DSCL_0_TOP_CONTROL, OU_PREFETCH_DIS,    DISABLE          ) |
#else
        BCHP_FIELD_ENUM(DSCL_0_TOP_CONTROL, 3D_OU_PREFETCH_DIS, DISABLE          ) |
#endif
        BCHP_FIELD_ENUM(DSCL_0_TOP_CONTROL, FIELD_POLARITY,     TOP              ) |
        BCHP_FIELD_ENUM(DSCL_0_TOP_CONTROL, SCAN_TYPE_CONV,     DISABLE          ) |
        BCHP_FIELD_ENUM(DSCL_0_TOP_CONTROL, ENABLE_CTRL,        ALWAYS_ENABLE    ) |
#if (BVDC_P_SUPPORT_DSCL_VER < 3)
        BCHP_FIELD_ENUM(DSCL_0_TOP_CONTROL, UPDATE_SEL,         UPDATE_BY_PICTURE) |
#endif
        BCHP_FIELD_ENUM(DSCL_0_TOP_CONTROL, FILTER_ORDER,       VERT_FIRST       );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_VERT_CONTROL);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DSCL_0_VERT_CONTROL, BAVG_BLK_SIZE,    0      ) |
        BCHP_FIELD_ENUM(DSCL_0_VERT_CONTROL, SEL_4TAP_IN_FIR8, DISABLE) |
        BCHP_FIELD_ENUM(DSCL_0_VERT_CONTROL, FIR_ENABLE,       ON     );

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_HORIZ_CONTROL);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(DSCL_0_HORIZ_CONTROL, MASK_HSCL_LONG_LINE,  OFF) |
        BCHP_FIELD_ENUM(DSCL_0_HORIZ_CONTROL, MASK_HSCL_SHORT_LINE, OFF) |
        BCHP_FIELD_ENUM(DSCL_0_HORIZ_CONTROL, STALL_DRAIN_ENABLE,   OFF) |
        BCHP_FIELD_ENUM(DSCL_0_HORIZ_CONTROL, FIR_ENABLE,           ON ) |
        BCHP_FIELD_ENUM(DSCL_0_HORIZ_CONTROL, HWF1_ENABLE,          OFF) |
        BCHP_FIELD_ENUM(DSCL_0_HORIZ_CONTROL, HWF0_ENABLE,          OFF);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_VERT_FIR_INIT_PIC_STEP);
    *pList->pulCurrent++ = 0x2000000;

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_HORIZ_FIR_INIT_STEP_FRAC);
    *pList->pulCurrent++ = 0x2000000;

    BDBG_ASSERT((((BCHP_DSCL_0_VERT_FIR_CHROMA_COEFF_PHASE7_00_01 - BCHP_DSCL_0_VERT_FIR_LUMA_COEFF_PHASE0_00_01) / sizeof(uint32_t)) + 1) == 16);
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_DSCL_0_VERT_FIR_CHROMA_COEFF_PHASE7_00_01 - BCHP_DSCL_0_VERT_FIR_LUMA_COEFF_PHASE0_00_01) / sizeof(uint32_t)) + 1);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_VERT_FIR_LUMA_COEFF_PHASE0_00_01);
    *pList->pulCurrent++ = 0x10000000;
    *pList->pulCurrent++ = 0x0f783f50;
    *pList->pulCurrent++ = 0x0dcc3f1c;
    *pList->pulCurrent++ = 0x0b5c3f3c;
    *pList->pulCurrent++ = 0x08903f70;
    *pList->pulCurrent++ = 0x05c03fa8;
    *pList->pulCurrent++ = 0x03403fd8;
    *pList->pulCurrent++ = 0x014c3fec;
    *pList->pulCurrent++ = 0x10000000;
    *pList->pulCurrent++ = 0x0f783f50;
    *pList->pulCurrent++ = 0x0dcc3f1c;
    *pList->pulCurrent++ = 0x0b5c3f3c;
    *pList->pulCurrent++ = 0x08903f70;
    *pList->pulCurrent++ = 0x05c03fa8;
    *pList->pulCurrent++ = 0x03403fd8;
    *pList->pulCurrent++ = 0x014c3fec;

    BDBG_ASSERT((((BCHP_DSCL_0_HORIZ_FIR_CHROMA_COEFF_PHASE7_04_05 - BCHP_DSCL_0_HORIZ_FIR_LUMA_COEFF_PHASE0_00_01) / sizeof(uint32_t)) + 1) == 48);
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(((BCHP_DSCL_0_HORIZ_FIR_CHROMA_COEFF_PHASE7_04_05 - BCHP_DSCL_0_HORIZ_FIR_LUMA_COEFF_PHASE0_00_01) / sizeof(uint32_t)) + 1);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_HORIZ_FIR_LUMA_COEFF_PHASE0_00_01);
    *pList->pulCurrent++ = 0x3fa40018;
    *pList->pulCurrent++ = 0x3e8000d4;
    *pList->pulCurrent++ = 0x0dd00208;
    *pList->pulCurrent++ = 0x3fb40018;
    *pList->pulCurrent++ = 0x3f1c00a0;
    *pList->pulCurrent++ = 0x0d88006c;
    *pList->pulCurrent++ = 0x3fc40018;
    *pList->pulCurrent++ = 0x3fb0006c;
    *pList->pulCurrent++ = 0x0cb03f2c;
    *pList->pulCurrent++ = 0x3fd80014;
    *pList->pulCurrent++ = 0x00280034;
    *pList->pulCurrent++ = 0x0b7c3e3c;
    *pList->pulCurrent++ = 0x3fe80008;
    *pList->pulCurrent++ = 0x00903ff8;
    *pList->pulCurrent++ = 0x09d43db4;
    *pList->pulCurrent++ = 0x3ff80008;
    *pList->pulCurrent++ = 0x00d43fd4;
    *pList->pulCurrent++ = 0x07d43d84;
    *pList->pulCurrent++ = 0x00080004;
    *pList->pulCurrent++ = 0x00f43fb4;
    *pList->pulCurrent++ = 0x05d43da4;
    *pList->pulCurrent++ = 0x00143ff8;
    *pList->pulCurrent++ = 0x00f43fa4;
    *pList->pulCurrent++ = 0x03e03e00;
    *pList->pulCurrent++ = 0x3fb40018;
    *pList->pulCurrent++ = 0x3ef800a0;
    *pList->pulCurrent++ = 0x0e80015c;
    *pList->pulCurrent++ = 0x3fc80018;
    *pList->pulCurrent++ = 0x3fa0005c;
    *pList->pulCurrent++ = 0x0e2c3fc4;
    *pList->pulCurrent++ = 0x3fe80008;
    *pList->pulCurrent++ = 0x00280018;
    *pList->pulCurrent++ = 0x0d583e84;
    *pList->pulCurrent++ = 0x3ff80008;
    *pList->pulCurrent++ = 0x00a03fe8;
    *pList->pulCurrent++ = 0x0bc83dc4;
    *pList->pulCurrent++ = 0x00080000;
    *pList->pulCurrent++ = 0x00f03fb4;
    *pList->pulCurrent++ = 0x09e83d6c;
    *pList->pulCurrent++ = 0x00183ff8;
    *pList->pulCurrent++ = 0x01083fa4;
    *pList->pulCurrent++ = 0x07c03d70;
    *pList->pulCurrent++ = 0x00183ff8;
    *pList->pulCurrent++ = 0x01083f94;
    *pList->pulCurrent++ = 0x05803dc8;
    *pList->pulCurrent++ = 0x00183ff8;
    *pList->pulCurrent++ = 0x00e43fa4;
    *pList->pulCurrent++ = 0x034c3e50;

    if(hDisplay->stCurInfo.bHdmiRmd && hDisplay->stCurInfo.bHdmiFmt)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_BVB_IN_SIZE);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(DSCL_0_BVB_IN_SIZE, HSIZE, hDisplay->stCurInfo.pFmtInfo->ulWidth) |
            BCHP_FIELD_DATA(DSCL_0_BVB_IN_SIZE, VSIZE, hDisplay->stCurInfo.pFmtInfo->ulHeight );

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_SRC_PIC_SIZE);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(DSCL_0_SRC_PIC_SIZE, HSIZE, hDisplay->stCurInfo.pFmtInfo->ulWidth) |
            BCHP_FIELD_DATA(DSCL_0_SRC_PIC_SIZE, VSIZE, hDisplay->stCurInfo.pFmtInfo->ulHeight );

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_DEST_PIC_SIZE);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(DSCL_0_DEST_PIC_SIZE, HSIZE, hDisplay->stCurInfo.pHdmiFmtInfo->ulWidth) |
            BCHP_FIELD_DATA(DSCL_0_DEST_PIC_SIZE, VSIZE, hDisplay->stCurInfo.pHdmiFmtInfo->ulHeight );

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_HORIZ_DEST_PIC_REGION_N1_END);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(DSCL_0_HORIZ_DEST_PIC_REGION_N1_END, POSITION, hDisplay->stCurInfo.pHdmiFmtInfo->ulWidth );

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_HORIZ_DEST_PIC_REGION_0_END);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(DSCL_0_HORIZ_DEST_PIC_REGION_0_END, POSITION, hDisplay->stCurInfo.pHdmiFmtInfo->ulWidth );

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_HORIZ_DEST_PIC_REGION_1_END);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(DSCL_0_HORIZ_DEST_PIC_REGION_1_END, POSITION, hDisplay->stCurInfo.pHdmiFmtInfo->ulWidth );

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_HORIZ_DEST_PIC_REGION_2_END);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(DSCL_0_HORIZ_DEST_PIC_REGION_2_END, POSITION, hDisplay->stCurInfo.pHdmiFmtInfo->ulWidth );

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_ENABLE);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(DSCL_0_ENABLE, SCALER_ENABLE, ON);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_DSCL_CTRL_0);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(VEC_CFG_DSCL_CTRL_0, OUTPUT_SEL, DSCL);
    }
    else
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_DSCL_CTRL_0);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(VEC_CFG_DSCL_CTRL_0, OUTPUT_SEL, BYPASS);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DSCL_0_ENABLE);
        *pList->pulCurrent++ =
            BCHP_FIELD_ENUM(DSCL_0_ENABLE, SCALER_ENABLE, OFF);
    }
#else
    BSTD_UNUSED(hDisplay);
    BSTD_UNUSED(pList);
#endif

    return;
}

static void BVDC_P_Vec_Build_DVI_RM_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayDviChan           *pstChan,
      BVDC_P_ListInfo                 *pList,
      bool                             bProgramFull )
{
    bool                   bModified;
    uint32_t               ulNum, ulDen, ulOffset;
    uint64_t               ulPixelClkRate;
    const BVDC_P_RateInfo *pRmInfo;
    BAVC_VdcDisplay_Info   lRateInfo;
    const uint32_t        *pRmTable;
    BVDC_P_DisplayInfo    *pCurInfo = &hDisplay->stCurInfo;
    const BFMT_VideoInfo  *pFmtInfo;
    const char* pcRateString;

#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY0
    BDBG_ASSERT(hDisplay->ulHdmiPwrAcquire);
#endif

    if(pCurInfo->bHdmiFmt)
    {
#if BCHP_HW7439_439_WORKAROUND
        /* Due to this HW jira, the BVB clock is set to run at 216MHz */
        /* Therefore in order to output 4kx2k format, DVI_RM needs to run at */
        /* 1080p60 rate (148.5MHz) and the DVI_MASTER_SEL will be set to be */
        /* set at TWICE_DTG_RM to double that rate to 297Mhz */
        pFmtInfo = BFMT_GetVideoFormatInfoPtr_isr(BFMT_VideoFmt_e1080p);
#else
        pFmtInfo = pCurInfo->pHdmiFmtInfo;
#endif
    }
    else
    {
        pFmtInfo = pCurInfo->pFmtInfo;
    }

    if(BVDC_P_GetRmTable_isr(pCurInfo, pFmtInfo, &pRmTable, pCurInfo->bFullRate, &lRateInfo) != BERR_SUCCESS)
        BDBG_ASSERT(0);
    pCurInfo->stRateInfo = lRateInfo;
    pCurInfo->ulVertFreq = lRateInfo.ulVertRefreshRate;

    /* +++ Setup RM +++ */
#if BVDC_P_SUPPORT_DTG_RMD
    if(pCurInfo->bHdmiRmd)
    {
        uint32_t ulPhaseInc;

        BDBG_MSG(("    VEC's RMD PxlClk = %sMHz, RefRate = %d (1/%dth Hz), MultiRate=%d",
            BVDC_P_GetRmString_isr(pCurInfo, pFmtInfo),
            pCurInfo->ulVertFreq, BFMT_FREQ_FACTOR, pCurInfo->bMultiRateAllow));

        /* --- Setup RMD --- */
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_RM_TABLE_SIZE);
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_RM_0_RATE_RATIO + pstChan->ulDviRegOffset);

        /* Setup RM_RATE_RATIO to RM_INTEGRATOR */
        BKNI_Memcpy((void*)pList->pulCurrent, (void*)pRmTable,
            BVDC_P_RM_TABLE_SIZE * sizeof(uint32_t));
        pList->pulCurrent += BVDC_P_RM_TABLE_SIZE;

        /* modified PHASE_INC to match HDMI clock */
        /* Formula: PHASE_INC = pixel_rate * 0x800000 / (clk / 2) */
        /* where for IT, clk = 216 and HDMI, clk = 324 */
        /* so for RMD, PHASE_INC = RM.PHASE_INC * 108 / 162 */
        /* DVI_DTG_RM_0_PHASE_INC (RW) */
        if(lRateInfo.ulPixelClkRate == BFMT_PXL_297MHz || lRateInfo.ulPixelClkRate == BFMT_PXL_594MHz)
        {
            ulPhaseInc = 0x1600000;
        }
        else if(lRateInfo.ulPixelClkRate == BFMT_PXL_297MHz_DIV_1_001 ||
                lRateInfo.ulPixelClkRate == BFMT_PXL_594MHz_DIV_1_001)
        {
            ulPhaseInc = 0x15FA5F9;
        }
        else
        {
            ulPhaseInc = pRmTable[2];
        }

#if !BCHP_HW7439_439_WORKAROUND
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_RM_0_PHASE_INC + pstChan->ulDviRegOffset);
        *pList->pulCurrent++ = /*0xEAAAAA;*/
            BCHP_FIELD_DATA(RM_0_PHASE_INC, PHASE_INC, (ulPhaseInc * 108 / 162));
#else
        /* Due to the work-around, only the DVI_RM is running at 1080p60 rate */
        /* HDMI RM still need to run at 4kx2k rate */
        pFmtInfo = pCurInfo->pHdmiFmtInfo;
        if(BVDC_P_GetRmTable_isr(pCurInfo, pFmtInfo, &pRmTable, pCurInfo->bFullRate, &lRateInfo) != BERR_SUCCESS)
            BDBG_ASSERT(0);
        pCurInfo->stRateInfo = lRateInfo;
        pCurInfo->ulVertFreq = lRateInfo.ulVertRefreshRate;
#endif
    }
#endif /* BVDC_P_SUPPORT_DTG_RMD */

    /* Analog VEC operates at double rate for 480P and 576P. So we have to
     * convert back to single rate frequency to access the HDMI rate table. */
    ulPixelClkRate = pCurInfo->stRateInfo.ulPixelClkRate;
    if (VIDEO_FORMAT_IS_ED(pFmtInfo->eVideoFmt))
    {
        if (ulPixelClkRate & BFMT_PXL_54MHz)
        {
            ulPixelClkRate &= ~BFMT_PXL_54MHz;
            ulPixelClkRate |=  BFMT_PXL_27MHz;
        }
        if (ulPixelClkRate & BFMT_PXL_54MHz_MUL_1_001)
        {
            ulPixelClkRate &= ~BFMT_PXL_54MHz_MUL_1_001;
            ulPixelClkRate |=  BFMT_PXL_27MHz_MUL_1_001;
        }
    }

#if BVDC_P_SUPPORT_MHL
    /* Look up MHL frequency and use this to look up RM and PLL parameters */
    if (pstChan->bMhlMode)
    {
        ulPixelClkRate = BVDC_P_PxlFreqToMhlFreq_isr(ulPixelClkRate);
        BDBG_ASSERT(ulPixelClkRate);
    }
#endif


    bModified = BVDC_P_HdmiRmTable_isr(
        pFmtInfo->eVideoFmt,
        ulPixelClkRate,
        pCurInfo->stHdmiSettings.eHdmiColorDepth,
        pCurInfo->stHdmiSettings.eHdmiPixelRepetition,
        pCurInfo->stHdmiSettings.stSettings.eColorComponent,
        &pRmInfo);
    BDBG_ASSERT(pRmInfo);
    if (bModified)
    {
        ulDen        = pRmInfo->ulDenominatorAdj;
        ulNum        = pRmInfo->ulNumeratorAdj;
        ulOffset     = pRmInfo->ulOffsetAdj;
        pcRateString = pRmInfo->pchRateAdj;
    }
    else
    {
        ulDen        = pRmInfo->ulDenominator;
        ulNum        = pRmInfo->ulNumerator;
        ulOffset     = pRmInfo->ulOffset;
        pcRateString = pRmInfo->pchRate;
    }
    BDBG_MSG(("    Hdmi_%d's RM PxlClk = %sMhz", pstChan->ulDvi, pcRateString));

    if(bProgramFull)
    {
#if (BVDC_P_SUPPORT_DVI_40NM)
        /* HDMI_TX_PHY_CK_DIV (RW) */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_CK_DIV + pstChan->ulDvpRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(HDMI_TX_PHY_CK_DIV, AUX,                    0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CK_DIV, EXT,                    0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CK_DIV, VCO,  pRmInfo->ulVcoRange) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CK_DIV, RM,   pRmInfo->ulRmDiv   );

        /* HDMI_TX_PHY_PLL_CFG (RW) */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_PLL_CFG + pstChan->ulDvpRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CFG, LOCAL_BYPASS_CONTROL,    0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CFG, BYPASS_OVERRIDE_CONTROL, 0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CFG, RNDGEN_START,            0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CFG, PDIV,     pRmInfo->ulPxDiv);

        /* HDMI_TX_PHY_PLL_CTRL (RW) */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_PLL_CTRL, REF_CLK_CFG)),
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTRL, REF_CLK_CFG, 2),
            BCHP_HDMI_TX_PHY_PLL_CTRL + pstChan->ulDvpRegOffset);

        /* HDMI_TX_PHY_CTL_2 (RW) */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_CTL_2, KP) |
              BCHP_MASK(HDMI_TX_PHY_CTL_2, KI) |
              BCHP_MASK(HDMI_TX_PHY_CTL_2, KA)),
            BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, KP, pRmInfo->ulKP) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, KI, pRmInfo->ulKI) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2, KA, pRmInfo->ulKA),
            BCHP_HDMI_TX_PHY_CTL_2 + pstChan->ulDvpRegOffset);

        /* HDMI_TX_PHY_MDIV_LOAD (RW) */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_MDIV_LOAD, VCO) |
              BCHP_MASK(HDMI_TX_PHY_MDIV_LOAD,  RM)),
            BCHP_FIELD_DATA(HDMI_TX_PHY_MDIV_LOAD, VCO, 1) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_MDIV_LOAD, RM,  1),
            BCHP_HDMI_TX_PHY_MDIV_LOAD + pstChan->ulDvpRegOffset);

        /* HW7422-770: Instead of wait for 1 vysnc, add few noops */
        BVDC_P_BUILD_NO_OPS(pList->pulCurrent);
        BVDC_P_BUILD_NO_OPS(pList->pulCurrent);
        BVDC_P_BUILD_NO_OPS(pList->pulCurrent);

        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_MDIV_LOAD, VCO) |
              BCHP_MASK(HDMI_TX_PHY_MDIV_LOAD,  RM)),
            BCHP_FIELD_DATA(HDMI_TX_PHY_MDIV_LOAD, VCO, 0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_MDIV_LOAD, RM,  0),
            BCHP_HDMI_TX_PHY_MDIV_LOAD + pstChan->ulDvpRegOffset);
#endif
    }
#if (BVDC_P_SUPPORT_HDMI_RM_VER >= BVDC_P_HDMI_RM_VER_7)
    else
    {
        /* HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_1 (RW),
         * For frequency shitf (aka rate vs rate/1.001) we need
         * ALWAYS_RESET_PLL_ON_FREQ_CHANGE = 0 to avoid unintented reset. */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_1, ALWAYS_RESET_PLL_ON_FREQ_CHANGE)),
             BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_1, ALWAYS_RESET_PLL_ON_FREQ_CHANGE, 0),
            BCHP_HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_1 + pstChan->ulDvpRegOffset);
    }
#endif

    /* HDMI_RM_RATE_RATIO (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_RATE_RATIO + pstChan->ulDvpRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HDMI_RM_RATE_RATIO, DENOMINATOR, ulDen);

    /* HDMI_RM_SAMPLE_INC (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_SAMPLE_INC + pstChan->ulDvpRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HDMI_RM_SAMPLE_INC, NUMERATOR,  ulNum) |
        BCHP_FIELD_DATA(HDMI_RM_SAMPLE_INC, SAMPLE_INC, pRmInfo->ulSampleInc);

    /* HDMI_RM_OFFSET (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_OFFSET + pstChan->ulDvpRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HDMI_RM_OFFSET, OFFSET, ulOffset);

#if (BVDC_P_SUPPORT_DVI_40NM)
    /* HDMI_RM_FORMAT (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_FORMAT + pstChan->ulDvpRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HDMI_RM_FORMAT, SHIFT, pRmInfo->ulShift) |
        BCHP_FIELD_DATA(HDMI_RM_FORMAT, STABLE_COUNT, 0);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_FORMAT + pstChan->ulDvpRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HDMI_RM_FORMAT, SHIFT, pRmInfo->ulShift) |
        BCHP_FIELD_DATA(HDMI_RM_FORMAT, STABLE_COUNT, 10000);
#else /* (BVDC_P_SUPPORT_DVI_28NM) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_FORMAT + pstChan->ulDvpRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HDMI_RM_FORMAT, SHIFT, pRmInfo->ulShift) |
        BCHP_FIELD_DATA(HDMI_RM_FORMAT, STABLE_COUNT, 8);

    if(bProgramFull)
    {
        uint32_t ulFcw, ulStableThrsh, ulHoldThrsh = 0;

        /* HDMI_TX_PHY_CK_DIV (RW) */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_CLK_DIV + pstChan->ulDvpRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(HDMI_TX_PHY_CLK_DIV, AUX,                   0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CLK_DIV, VCO, pRmInfo->ulVcoRange) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CLK_DIV, RM,  pRmInfo->ulRmDiv   );

        /* HDMI_TX_PHY_PLL_CFG (RW) */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_PLL_CFG + pstChan->ulDvpRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CFG, LOCAL_BYPASS_CONTROL,    0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CFG, BYPASS_OVERRIDE_CONTROL, 0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CFG, RNDGEN_START,            0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CFG, PDIV,     pRmInfo->ulPxDiv);

        /* HDMI_TX_PHY_PLL_CTL_0 (RW) */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            /* power on settings - these are set and forget; can they be moved */
            ~(BCHP_MASK(HDMI_TX_PHY_PLL_CTL_0, ENA_VCO_CLK)
            | BCHP_MASK(HDMI_TX_PHY_PLL_CTL_0, VCO_CONT_EN)
            | BCHP_MASK(HDMI_TX_PHY_PLL_CTL_0, VCO_POST_DIV2)
            | BCHP_MASK(HDMI_TX_PHY_PLL_CTL_0, EMULATE_VC_HIGH)
            | BCHP_MASK(HDMI_TX_PHY_PLL_CTL_0, EMULATE_VC_LOW)
            | BCHP_MASK(HDMI_TX_PHY_PLL_CTL_0, VC_RANGE_EN)
            | BCHP_MASK(HDMI_TX_PHY_PLL_CTL_0, MASH11_MODE)

            | BCHP_MASK(HDMI_TX_PHY_PLL_CTL_0, VCO_SEL)),

            /* power on settings - these are set and forget; can they be moved */
              BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_0, ENA_VCO_CLK, 1)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_0, VCO_CONT_EN, 1)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_0, VCO_POST_DIV2, 0)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_0, EMULATE_VC_HIGH, 0)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_0, EMULATE_VC_LOW, 0)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_0, VC_RANGE_EN, 0)
            | BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_0, MASH11_MODE, 1)

            | BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_0, VCO_SEL, pRmInfo->ulVcoSel),
            BCHP_HDMI_TX_PHY_PLL_CTL_0 + pstChan->ulDvpRegOffset);

        /* HDMI_TX_PHY_PLL_CTL_1 (RW) */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_PLL_CTL_1, FREQ_DOUBLER_ENABLE) |
                BCHP_MASK(HDMI_TX_PHY_PLL_CTL_1, CPP)), /* Set and forget */
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_1, FREQ_DOUBLER_ENABLE, 1) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CTL_1, CPP, 0x8A),  /* Set and forget */
            BCHP_HDMI_TX_PHY_PLL_CTL_1 + pstChan->ulDvpRegOffset);

        /* HDMI_TX_PHY_CTL_2 (RW) */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_CTL_2,  VCO_GAIN)),
            BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_2,  VCO_GAIN, pRmInfo->ulVcoGain),
            BCHP_HDMI_TX_PHY_CTL_2 + pstChan->ulDvpRegOffset);

        /* HDMI_TX_PHY_CTL_3 (RW) */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_CTL_3, RP ) |
              BCHP_MASK(HDMI_TX_PHY_CTL_3, RZ ) |
              BCHP_MASK(HDMI_TX_PHY_CTL_3, CP1) |
              BCHP_MASK(HDMI_TX_PHY_CTL_3, CP ) |
              BCHP_MASK(HDMI_TX_PHY_CTL_3, CZ ) |
              BCHP_MASK(HDMI_TX_PHY_CTL_3, ICP)),
            BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_3, RP,  pRmInfo->ulRp ) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_3, RZ,  pRmInfo->ulRz ) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_3, CP1, pRmInfo->ulCp1) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_3, CP,  pRmInfo->ulCp ) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_3, CZ,  pRmInfo->ulCz ) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_CTL_3, ICP, pRmInfo->ulIcp),
            BCHP_HDMI_TX_PHY_CTL_3 + pstChan->ulDvpRegOffset);

#if (BVDC_P_SUPPORT_HDMI_RM_VER >= BVDC_P_HDMI_RM_VER_7)
        ulFcw = pRmInfo->ulOffset / 16;
        ulStableThrsh = ulFcw / 4000;
        ulHoldThrsh = ulFcw / 250;

        /* HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_1 (RW) */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_1 + pstChan->ulDvpRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_1, USE_FIXED_FOR_HOLD,
                (ulHoldThrsh < 0x10000) ?  0 : 1) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_1, AUTO_LIMIT,
                (ulHoldThrsh < 0x10000) ?  1 : 0) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_1, ALWAYS_RESET_PLL_ON_FREQ_CHANGE, 1) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_1, MIN_LIMIT,
                (ulHoldThrsh < 0x10000) ? 0 : ulFcw - ulHoldThrsh);

        /* HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_2 (RW) */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_2,  MAX_LIMIT)),
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_2,  MAX_LIMIT,
            (ulHoldThrsh < 0x10000) ? 0 : ulFcw + ulHoldThrsh),
            BCHP_HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_2 + pstChan->ulDvpRegOffset);

        /* HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_4 (RW) */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_4 + pstChan->ulDvpRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_4, STABLE_THRESHOLD,
                (ulHoldThrsh < 0x10000) ? ulStableThrsh:
                (ulStableThrsh < 0x10000) ? ulStableThrsh : 0xFFFF) |
            BCHP_FIELD_DATA(HDMI_TX_PHY_PLL_CALIBRATION_CONFIG_4, HOLD_THRESHOLD,
                (ulHoldThrsh < 0x10000) ? ulHoldThrsh:
                (ulStableThrsh < 0x10000) ? ulStableThrsh : 0xFFFF);
#else
    BSTD_UNUSED(ulFcw);
    BSTD_UNUSED(ulStableThrsh);
    BSTD_UNUSED(ulHoldThrsh);
#endif
    }

    /* Restored stable count */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_FORMAT + pstChan->ulDvpRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(HDMI_RM_FORMAT, SHIFT, pRmInfo->ulShift) |
        BCHP_FIELD_DATA(HDMI_RM_FORMAT, STABLE_COUNT, 10000);

    if(bProgramFull)
    {
        /* HDMI_TX_PHY_RESET_CTL (RW) */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_RESET_CTL, PLL_RESETB) |
              BCHP_MASK(HDMI_TX_PHY_RESET_CTL, PLLDIV_RSTB)),
             (BCHP_FIELD_DATA(HDMI_TX_PHY_RESET_CTL, PLL_RESETB,  0) |
              BCHP_FIELD_DATA(HDMI_TX_PHY_RESET_CTL, PLLDIV_RSTB, 0)),
            BCHP_HDMI_TX_PHY_RESET_CTL + pstChan->ulDvpRegOffset);

        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_TX_PHY_RESET_CTL, PLL_RESETB) |
              BCHP_MASK(HDMI_TX_PHY_RESET_CTL, PLLDIV_RSTB)),
             (BCHP_FIELD_DATA(HDMI_TX_PHY_RESET_CTL, PLL_RESETB,  1) |
              BCHP_FIELD_DATA(HDMI_TX_PHY_RESET_CTL, PLLDIV_RSTB, 1)),
            BCHP_HDMI_TX_PHY_RESET_CTL + pstChan->ulDvpRegOffset);

        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_FIFO_CTL, RECENTER)),
            BCHP_FIELD_DATA(HDMI_FIFO_CTL, RECENTER, 1),
            BCHP_HDMI_FIFO_CTL + pstChan->ulDvpRegOffset);

        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_FIFO_CTL, RECENTER)),
            BCHP_FIELD_DATA(HDMI_FIFO_CTL, RECENTER, 0),
            BCHP_HDMI_FIFO_CTL + pstChan->ulDvpRegOffset);
    }
#endif

    hDisplay->bRateManagerUpdated = true;
    BSTD_UNUSED(pcRateString);
    return;
}

static void BVDC_P_Vec_Build_DVI_DTG_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayDviChan           *pstChan,
      bool                             bReloadMicrocode,
      BVDC_P_ListInfo                 *pList )
{
    const uint32_t *pDtRam;
    uint32_t ulTopTrigVal, ulBotTrigVal;
    const BVDC_P_DisplayInfo *pCurInfo = &hDisplay->stCurInfo;
    uint32_t ulHeight;
    const BFMT_VideoInfo *pFmtInfo;

    pFmtInfo = (hDisplay->stCurInfo.bHdmiFmt)
        ? hDisplay->stCurInfo.pHdmiFmtInfo : hDisplay->stCurInfo.pFmtInfo;

    ulHeight = pFmtInfo->ulDigitalHeight>>(pFmtInfo->bInterlaced);

    if(BFMT_IS_3D_MODE(pFmtInfo->eVideoFmt))
    {
        ulHeight = ulHeight * 2 + pFmtInfo->ulActiveSpace;
    }

    /*-------------------------------*/
    /* NOTE: Toggle Reset first */
    /*-------------------------------*/

    if (bReloadMicrocode)
    {
        /* Dtram[40..7f] is for DVI */
        pDtRam = BVDC_P_GetDtramTable_isr(
            pCurInfo, pFmtInfo, hDisplay->bArib480p);
        BDBG_MSG((
            "Dtram microcode - timestamp: 0x%.8x",
            pDtRam[BVDC_P_DTRAM_TABLE_TIMESTAMP_IDX]));
        BDBG_MSG((
            "Dtram microcode - checksum:  0x%.8x",
            pDtRam[BVDC_P_DTRAM_TABLE_CHECKSUM_IDX]));

        /* Load uCode */
        *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_DTRAM_TABLE_SIZE);
#if (BVDC_P_ORTHOGONAL_VEC_VER <= BVDC_P_ORTHOGONAL_VEC_VER_1)
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_DTRAM_0_DMC_INSTRUCTIONi_ARRAY_BASE +
                (BVDC_P_DVI_DTRAM_START_ADDR * sizeof(uint32_t)));
#else
        *pList->pulCurrent++ =
            BRDC_REGISTER(BCHP_DVI_DTG_0_DMC_INSTRUCTIONi_ARRAY_BASE +
                (BVDC_P_DVI_DTRAM_START_ADDR * sizeof(uint32_t)) +
                pstChan->ulDviRegOffset);
#endif
        BKNI_Memcpy((void*)pList->pulCurrent, (void*)pDtRam,
            BVDC_P_DTRAM_TABLE_SIZE * sizeof(uint32_t));
        pList->pulCurrent += BVDC_P_DTRAM_TABLE_SIZE;
    }

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
#if (BVDC_P_ORTHOGONAL_VEC_VER <= BVDC_P_ORTHOGONAL_VEC_VER_1)
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTRAM_0_DTRAM_CONFIG);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DTRAM_0_DTRAM_CONFIG, ARBITER_LATENCY, 0xb);
#else
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_DTRAM_CONFIG + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DVI_DTG_0_DTRAM_CONFIG, ARBITER_LATENCY, 0xb);
#endif

    /* DVI_DTG_DTG_BVB (RW) */
    /* DVI_DTG_CCIR_PCL (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_CCIR_PCL + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DVI_DTG_0_CCIR_PCL, HACTIVE_ENABLE,  1) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_CCIR_PCL, HACTIVE_SEL,     0) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_CCIR_PCL, VACTIVE_ENABLE,  1) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_CCIR_PCL, VACTIVE_SEL,     0) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_CCIR_PCL, VBLANK_ENABLE,   1) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_CCIR_PCL, VBLANK_SEL,      0) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_CCIR_PCL, ODD_EVEN_ENABLE, 1) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_CCIR_PCL, ODD_EVEN_SEL,    0);  /* nominal */

    /* DVI_DTG_DVI_PCL (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_DVI_PCL + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DVI_DTG_0_DVI_PCL, DIGITAL_HSYNC_ENABLE, 1) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_DVI_PCL, DIGITAL_HSYNC_SEL,    0) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_DVI_PCL, DVI_V_ENABLE,         1) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_DVI_PCL, DVI_V_SEL,            1) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_DVI_PCL, DVI_DE_ENABLE,        1) | /* nominal */
        BCHP_FIELD_DATA(DVI_DTG_0_DVI_PCL, DVI_DE_SEL,           3);  /* nominal */

    /* DVI_DTG_CNTL_PCL (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_CNTL_PCL + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DVI_DTG_0_CNTL_PCL, NEW_LINE_CLR_SEL, 3 );  /* nominal */

    /* DVI_DTG_RAM_ADDR (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_RAM_ADDR + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DVI_DTG_0_RAM_ADDR, MC_START_ADDR, 0x40);  /* nominal */

    /* DVI_DTG_DTG_BVB_SIZE (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_DTG_BVB_SIZE + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_BVB_SIZE, HORIZONTAL, pFmtInfo->ulDigitalWidth ) |
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_BVB_SIZE, VERTICAL,   ulHeight);

    /* Set up triggers */
    ulTopTrigVal = BVDC_P_TRIGGER_LINE;
    ulBotTrigVal = (pFmtInfo->bInterlaced) ?
        (((pFmtInfo->ulScanHeight + 1) / 2) + BVDC_P_TRIGGER_LINE) : 0;

#if (BVDC_P_SUPPORT_IT_VER >= 2)
    /* Having IT trigger modulo count set to 2 means the chipset has
     * a OSCL which produces 1080p output. BVN still runs as 1080i.
     */
    if (1 != pCurInfo->ulTriggerModuloCnt)
        ulBotTrigVal = BVDC_P_TRIGGER_LINE;
#endif

    /* DVI_DTG_DTG_TRIGGER_0 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_DTG_TRIGGER_0 + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_TRIGGER_0, TRIGGER_VALUE, ulTopTrigVal) |
#if (BVDC_P_SUPPORT_IT_VER >= 2)
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_TRIGGER_0, TRIGGER_SELECT, 0) |
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_TRIGGER_0, MODULO_COUNT, pCurInfo->ulTriggerModuloCnt) |
#endif
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_TRIGGER_0, ENABLE, 0);

    /* DVI_DTG_DTG_TRIGGER_1 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_DTG_TRIGGER_1 + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_TRIGGER_1, TRIGGER_VALUE, ulBotTrigVal) |
#if (BVDC_P_SUPPORT_IT_VER >= 2)
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_TRIGGER_1, TRIGGER_SELECT, (1 != pCurInfo->ulTriggerModuloCnt) ? 1 : 0) |
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_TRIGGER_1, MODULO_COUNT, pCurInfo->ulTriggerModuloCnt) |
#endif
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_TRIGGER_1, ENABLE, 0);

{
    uint32_t ulLineNum, ulStartLine;

    ulStartLine = pFmtInfo->ulHeight;
    ulLineNum = ulStartLine + pFmtInfo->ulActiveSpace + 1;

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_DTG_AS_CONTROL + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(DVI_DTG_0_DTG_AS_CONTROL, AS_INPUT_TYPE, COMBINED   ) |
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_AS_CONTROL, AS_NUM_LINES,  pFmtInfo->ulActiveSpace) |
        BCHP_FIELD_DATA(DVI_DTG_0_DTG_AS_CONTROL, AS_START_LINE, ulStartLine) |
        (BFMT_IS_3D_MODE(pFmtInfo->eVideoFmt) ?
          BCHP_FIELD_DATA(DVI_DTG_0_DTG_AS_CONTROL, AS_ENABLE, 1) :
          BCHP_FIELD_DATA(DVI_DTG_0_DTG_AS_CONTROL, AS_ENABLE, 0));

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_DTG_AS_LINE_NUMBER + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ = BCHP_FIELD_DATA(DVI_DTG_0_DTG_AS_LINE_NUMBER, LINE_NUMBER, ulLineNum);
}

    return;
}

static void BVDC_P_Vec_Build_DVI_CSC_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayDviChan           *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    const BVDC_P_DisplayCscMatrix *pCscMatrix = NULL;
    const BVDC_P_DisplayInfo *pCurInfo = &hDisplay->stCurInfo;
    uint8_t ucCh0, ucCh1, ucCh2;
    const BVDC_P_Compositor_Info *pCmpInfo = &hDisplay->hCompositor->stCurInfo;
#if BCHP_DVI_MISC_0_REG_START
    bool  bDviCscPassThrough;
#endif

    BDBG_ASSERT(stAVC_MatrixCoefficient_InfoTbl[hDisplay->stCurInfo.stHdmiSettings.stSettings.eMatrixCoeffs].eAvcCs == hDisplay->stCurInfo.stHdmiSettings.stSettings.eMatrixCoeffs);
    BDBG_MSG(("Display %d Hdmi %d using %s",
        hDisplay->eId, pstChan->ulDvi, stAVC_MatrixCoefficient_InfoTbl[hDisplay->stCurInfo.stHdmiSettings.stSettings.eMatrixCoeffs].pcAvcCsStr));
#if BCHP_DVI_MISC_0_REG_START
    bDviCscPassThrough = (hDisplay->bCmpBypassDviCsc || hDisplay->stCurInfo.bBypassVideoProcess);
    if (bDviCscPassThrough)
    {
        BDBG_MODULE_MSG(BVDC_DVI_NLCSC, ("disp[%d] bypass DVI_CSC", hDisplay->eId));
    }
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_MISC_0_CSC_BYPASS_OVERRIDE_CONTROL + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
         BCHP_FIELD_DATA(DVI_MISC_0_CSC_BYPASS_OVERRIDE_CONTROL, OVERRIDE_ENABLE, bDviCscPassThrough) |
         BCHP_FIELD_DATA(DVI_MISC_0_CSC_BYPASS_OVERRIDE_CONTROL, OVERRIDE_VALUE,  bDviCscPassThrough);
#endif

    /* --- Setup DVI CSC --- */
    BVDC_P_Display_GetDviCscTable_isr(pCurInfo, &pCscMatrix);

    /* TODO: fold sync only into generic display output mute functionality */
    if(pCurInfo->bUserCsc)
    {
        BVDC_P_Csc_FromMatrixDvo_isr(&hDisplay->stDvoCscMatrix.stCscCoeffs,
            pCurInfo->pl32_Matrix, pCurInfo->ulUserShift,
            ((pCurInfo->stHdmiSettings.stSettings.eMatrixCoeffs == BAVC_MatrixCoefficients_eHdmi_RGB) ||
             (pCurInfo->stHdmiSettings.stSettings.eMatrixCoeffs == BAVC_MatrixCoefficients_eDvi_Full_Range_RGB))? true : false);

        hDisplay->stDvoCscMatrix.ulMin = pCscMatrix->ulMin;
        hDisplay->stDvoCscMatrix.ulMax = pCscMatrix->ulMax;
    }
    else
    {
        /* For BVDC_Display_GetDvoColorMatrix() */
        hDisplay->stDvoCscMatrix = *pCscMatrix;
    }

    /* Blank color, use in CSC and DVI_DVF_0_DVF_VALUES */
    ucCh0 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 2);
    ucCh1 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 1);
    ucCh2 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 0);

    if(!pCurInfo->abOutputMute[BVDC_DisplayOutput_eDvo])
    {
        BVDC_P_Csc_DvoApplyAttenuationRGB_isr(hDisplay->stCurInfo.lDvoAttenuationR,
                                              hDisplay->stCurInfo.lDvoAttenuationG,
                                              hDisplay->stCurInfo.lDvoAttenuationB,
                                              hDisplay->stCurInfo.lDvoOffsetR,
                                              hDisplay->stCurInfo.lDvoOffsetG,
                                              hDisplay->stCurInfo.lDvoOffsetB,
                                             &hDisplay->stDvoCscMatrix.stCscCoeffs);
    }
    else
    {
        /* Swap ch0 and 1 of input color to match vec csc layout */
        BVDC_P_Csc_ApplyYCbCrColor_isr(&hDisplay->stDvoCscMatrix.stCscCoeffs, ucCh1, ucCh0, ucCh2);
    }

    if((BAVC_MatrixCoefficients_eHdmi_RGB == pCurInfo->stHdmiSettings.stSettings.eMatrixCoeffs) ||
       (BAVC_MatrixCoefficients_eDvi_Full_Range_RGB == pCurInfo->stHdmiSettings.stSettings.eMatrixCoeffs))
    {
        ucCh0 = pCmpInfo->ucGreen;
        ucCh1 = pCmpInfo->ucBlue;
        ucCh2 = pCmpInfo->ucRed;
    }

#if BVDC_P_CMP_NON_LINEAR_CSC_VER < 3 /* TODO: bring up 7271B0 */
    /* DVI_CSC_CSC_MODE (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_CSC_TABLE_SIZE);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_CSC_0_CSC_MODE + pstChan->ulDviRegOffset);
    BVDC_P_Vec_Build_CSC_isr(&hDisplay->stDvoCscMatrix, pList);
#if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC
    /* enable converting from BT2020 NCL R'G'B' to BT2020 CL YCbCr */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_CSC_0_CL2020_CONTROL + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ = (BAVC_MatrixCoefficients_eItu_R_BT_2020_CL == pCurInfo->stHdmiSettings.stSettings.eMatrixCoeffs)?
        BCHP_FIELD_ENUM(DVI_CSC_0_CL2020_CONTROL, CTRL, ENABLE ) | BCHP_FIELD_ENUM(DVI_CSC_0_CL2020_CONTROL, SEL_GAMMA, BT1886_GAMMA ):
        BCHP_FIELD_ENUM(DVI_CSC_0_CL2020_CONTROL, CTRL, DISABLE );
#endif /* #if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC */

    /* Blank padding color */
    /* DVI_DVF_DVF_VALUES (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DVF_0_DVF_VALUES + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DVI_DVF_0_DVF_VALUES, CH2_BLANK, ucCh2 ) |
        BCHP_FIELD_DATA(DVI_DVF_0_DVF_VALUES, CH1_BLANK, ucCh1 ) |
        BCHP_FIELD_DATA(DVI_DVF_0_DVF_VALUES, CH0_BLANK, ucCh0 );

    /* DVI_CSC_DITHER */
    if(hDisplay->stCurInfo.stHdmiSettings.eHdmiColorDepth == BAVC_HDMI_BitsPerPixel_e24bit)
    {
        if(!hDisplay->hCompositor->bIs10BitCore)
        {
            hDisplay->stDviDither.ulCh0Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_8BIT;
            hDisplay->stDviDither.ulCh1Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_8BIT;
            hDisplay->stDviDither.ulCh2Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_8BIT;
            hDisplay->stDviDither.ulCh0Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_8BIT;
            hDisplay->stDviDither.ulCh1Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_8BIT;
            hDisplay->stDviDither.ulCh2Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_8BIT;
            hDisplay->stDviDither.ulMode      = 2; /* dither 8-bit output */
        }
        else
        {
            hDisplay->stDviDither.ulCh0Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_10BIT;
            hDisplay->stDviDither.ulCh1Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_10BIT;
            hDisplay->stDviDither.ulCh2Offset = BVDC_P_DITHER_DISP_DVI_OFFSET_10BIT;
            hDisplay->stDviDither.ulCh0Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_10BIT;
            hDisplay->stDviDither.ulCh1Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_10BIT;
            hDisplay->stDviDither.ulCh2Scale  = BVDC_P_DITHER_DISP_DVI_SCALE_10BIT;
            hDisplay->stDviDither.ulMode      = 2; /* dither 10-bit output */
        }
    }
    else /* TODO: shall we dither for 10/12-bit? */
    {
        hDisplay->stDviDither.ulMode      = 0; /* rounding */
    }

    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_DITHER_TABLE_SIZE);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_CSC_0_DITHER_CONTROL + pstChan->ulDviRegOffset);
    BVDC_P_Vec_Build_Dither_isr(&hDisplay->stDviDither, pList);
#endif

    return;
}


/*************************************************************************
 *  {secret}
 *
 *  Builds the blank color of DVI or 656 output.  Use the compositor
 *  background color.
 **************************************************************************/
static void BVDC_P_Vec_Build_DVI_DVF_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayDviChan           *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t upsample2x, downsample2x;
    const BFMT_VideoInfo *pFmtInfo;

    pFmtInfo = (hDisplay->stCurInfo.bHdmiFmt)
        ? hDisplay->stCurInfo.pHdmiFmtInfo : hDisplay->stCurInfo.pFmtInfo;

    if (VIDEO_FORMAT_IS_ED(pFmtInfo->eVideoFmt))
    {
        if ((BAVC_HDMI_PixelRepetition_e1x ==
            hDisplay->stCurInfo.stHdmiSettings.eHdmiPixelRepetition) ||
            (BAVC_HDMI_PixelRepetition_e4x ==
            hDisplay->stCurInfo.stHdmiSettings.eHdmiPixelRepetition))
        {
            upsample2x   = BCHP_DVI_DVF_0_DVF_CONFIG_UPSAMPLE2X_ON;
            downsample2x = BCHP_DVI_DVF_0_DVF_CONFIG_DOWNSAMPLE2X_OFF;
        }
        else
        {
            upsample2x   = BCHP_DVI_DVF_0_DVF_CONFIG_UPSAMPLE2X_OFF;
            downsample2x = BCHP_DVI_DVF_0_DVF_CONFIG_DOWNSAMPLE2X_ON;
        }
    }
    else
    {
        upsample2x   =
            (VIDEO_FORMAT_IS_SD(pFmtInfo->eVideoFmt)) ?
            BCHP_DVI_DVF_0_DVF_CONFIG_UPSAMPLE2X_ON :
            BCHP_DVI_DVF_0_DVF_CONFIG_UPSAMPLE2X_OFF ;
        downsample2x = BCHP_DVI_DVF_0_DVF_CONFIG_DOWNSAMPLE2X_OFF;
    }

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DVF_0_DVF_CONFIG + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ = (
        BCHP_FIELD_DATA(DVI_DVF_0_DVF_CONFIG,   UPSAMPLE2X,   upsample2x) |
        BCHP_FIELD_DATA(DVI_DVF_0_DVF_CONFIG, DOWNSAMPLE2X, downsample2x) );

    return;
}

/*************************************************************************
 *  {secret}
 *
 *  Builds the DVI frontend Format Conversion block.
 **************************************************************************/
static void BVDC_P_Vec_Build_DVI_FC_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayDviChan           *pstChan,
      BVDC_P_ListInfo                 *pList )
{
#ifdef BCHP_DVI_FC_0_REG_START
    const BVDC_P_DisplayInfo *pCurInfo = &hDisplay->stCurInfo;

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_FC_0_FORMAT_CONV_CONTROL + pstChan->ulDviRegOffset);
    *pList->pulCurrent++ = (
        BCHP_FIELD_DATA(DVI_FC_0_FORMAT_CONV_CONTROL, HTOTAL_SIZE, pCurInfo->pFmtInfo->ulDigitalWidth) |
        BCHP_FIELD_DATA(DVI_FC_0_FORMAT_CONV_CONTROL, COEFF_1, 4) |
        BCHP_FIELD_DATA(DVI_FC_0_FORMAT_CONV_CONTROL, COEFF_0, 4) |
        BCHP_FIELD_DATA(DVI_FC_0_FORMAT_CONV_CONTROL, FORMAT,
            pCurInfo->stHdmiSettings.stSettings.eColorComponent == BAVC_Colorspace_eYCbCr422 ? BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FORMAT_FORMAT_422 :
            pCurInfo->stHdmiSettings.stSettings.eColorComponent == BAVC_Colorspace_eYCbCr420 ? BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FORMAT_FORMAT_420 :
                BCHP_DVI_FC_0_FORMAT_CONV_CONTROL_FORMAT_FORMAT_444) |
        BCHP_FIELD_ENUM(DVI_FC_0_FORMAT_CONV_CONTROL, FILTER_MODE, BYPASS) |
        BCHP_FIELD_ENUM(DVI_FC_0_FORMAT_CONV_CONTROL, DERING_EN, OFF) );

#else
    BSTD_UNUSED(hDisplay);
    BSTD_UNUSED(pstChan);
    BSTD_UNUSED(pList);
#endif
}

static void BVDC_P_ProgramDviChan_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayDviChan           *pstChan,
      bool                             bReloadMicrocode,
      BVDC_P_ListInfo                 *pList )
{
    if(pstChan->ulDvi == 0)
    {
        BVDC_P_Vec_Build_Dscl_isr(hDisplay, pList);
    }
    BVDC_P_Vec_Build_DVI_RM_isr(hDisplay, pstChan, pList, true);
    BVDC_P_Vec_Build_DVI_DTG_isr(hDisplay, pstChan, bReloadMicrocode, pList);
    BVDC_P_Vec_Build_DVI_DVF_isr(hDisplay, pstChan, pList);
    BVDC_P_Vec_Build_DVI_CSC_isr(hDisplay, pstChan, pList);
    BVDC_P_Vec_Build_DVI_FC_isr(hDisplay, pstChan, pList);
    BVDC_P_Display_SetupDviTG_isr(hDisplay, pList);

    return;
}

static void BVDC_P_SetupDviChan_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayDviChan           *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    BSTD_UNUSED(hDisplay);
    BVDC_P_VEC_SW_INIT(DVI_DTG_0,  pstChan->ulDtgSwInitOffset,  1);
#ifdef BCHP_DVI_CSC_0_REG_START
    BVDC_P_VEC_SW_INIT(DVI_CSC_0,  pstChan->ulCscSwInitOffset,  1);
#endif
    BVDC_P_VEC_SW_INIT(DVI_DVF_0,  pstChan->ulDvfSwInitOffset,  1);
    BVDC_P_VEC_SW_INIT(DVI_MISC_0, pstChan->ulMiscSwInitOffset, 1);
#ifdef BCHP_DVI_FC_0_REG_START
    BVDC_P_VEC_SW_INIT(DVI_FC_0,   pstChan->ulFcSwInitOffset,   1);
#endif

    BVDC_P_VEC_SW_INIT(DVI_DTG_0,  pstChan->ulDtgSwInitOffset,  0);
#ifdef BCHP_DVI_CSC_0_REG_START
    BVDC_P_VEC_SW_INIT(DVI_CSC_0,  pstChan->ulCscSwInitOffset,  0);
#endif
    BVDC_P_VEC_SW_INIT(DVI_DVF_0,  pstChan->ulDvfSwInitOffset,  0);
    BVDC_P_VEC_SW_INIT(DVI_MISC_0, pstChan->ulMiscSwInitOffset, 0);
#ifdef BCHP_DVI_FC_0_REG_START
    BVDC_P_VEC_SW_INIT(DVI_FC_0,   pstChan->ulFcSwInitOffset,   0);
#endif
    return;
}

static uint32_t BVDC_P_GetDviSrc_isr
    ( BVDC_Display_Handle              hDisplay )
{
    uint32_t ulDviSrc = 0;

    if(hDisplay->stCurInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi])
    {
        ulDviSrc = BCHP_VEC_CFG_DVI_DTG_0_SOURCE_SOURCE_DECIM_0;
    }
    else
    {
        ulDviSrc = BVDC_P_GetVecCfgSrc_isr(hDisplay);
    }

    return ulDviSrc;
}

static void BVDC_P_ConnectDviSrc_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayDviChan           *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t ulDviSrc = BVDC_P_GetDviSrc_isr(hDisplay);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_DVI_DTG_0_SOURCE + pstChan->ulDvi * 4);
    *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_DVI_DTG_0_SOURCE, SOURCE, ulDviSrc);

    return;
}

static void BVDC_P_TearDownDviChan_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayDviChan           *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    BSTD_UNUSED(hDisplay);

    /* Disable DVI source */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_DVI_DTG_0_SOURCE + pstChan->ulDvi * 4);
    *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_DVI_DTG_0_SOURCE, SOURCE, BCHP_VEC_CFG_DVI_DTG_0_SOURCE_SOURCE_DISABLE);

    return;
}

void BVDC_P_Reset656ChanInfo
    ( BVDC_P_Display656Chan           *pstChan )
{
    pstChan->bEnable    = false; /* off */
    pstChan->ul656       = BVDC_P_HW_ID_INVALID;

    return;
}

#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
BERR_Code BVDC_P_Alloc656ChanResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_DisplayId                   eDisplayId,
      BVDC_P_Display656Chan           *pstChan,
      uint32_t                         ulSrcId )
{
    BERR_Code err;

    BSTD_UNUSED(ulSrcId);

    err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_e656, 0, eDisplayId, &pstChan->ul656, false);

    if (err)
    {
        BDBG_ERR(("No 656 available"));
    }

    return BERR_TRACE(err);
}

void BVDC_P_Free656ChanResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_Display_Handle              hDisplay )
{

    if (hDisplay->st656Chan.ul656 != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_e656, hDisplay->st656Chan.ul656);
        hDisplay->st656Chan.ul656 = BVDC_P_HW_ID_INVALID;
    }

#ifdef BCHP_PWR_RESOURCE_VDC_656_OUT
    if(hDisplay->ul656PwrAcquire != 0)
    {
        hDisplay->ul656PwrAcquire--;
        hDisplay->ul656PwrRelease = 1;
        BDBG_MSG(("656 slave mode disable: Release pending BCHP_PWR_RESOURCE_VDC_656_OUT"));
    }
#endif
    return;
}

static void BVDC_P_TearDown656Chan_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_Display656Chan           *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    BSTD_UNUSED(hDisplay);
    BSTD_UNUSED(pstChan); /* For multiple 656 instances in the future */

    /* Disable 656 source */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_ITU656_DTG_0_SOURCE);
    *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_ITU656_DTG_0_SOURCE, SOURCE, BCHP_VEC_CFG_ITU656_DTG_0_SOURCE_SOURCE_DISABLE);

    return;
}

static void BVDC_P_Setup656Chan_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_Display656Chan           *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    BSTD_UNUSED(hDisplay);
    BSTD_UNUSED(pstChan); /* For multiple 656 instances in the future */

    /* reset */
    BVDC_P_VEC_SW_INIT(ITU656_DTG_0,       0, 1);
    BVDC_P_VEC_SW_INIT(ITU656_CSC_0,       0, 1);
    BVDC_P_VEC_SW_INIT(ITU656_DVF_0,       0, 1);
    BVDC_P_VEC_SW_INIT(ITU656_FORMATTER_0, 0, 1);

    BVDC_P_VEC_SW_INIT(ITU656_DTG_0,       0, 0);
    BVDC_P_VEC_SW_INIT(ITU656_CSC_0,       0, 0);
    BVDC_P_VEC_SW_INIT(ITU656_DVF_0,       0, 0);
    BVDC_P_VEC_SW_INIT(ITU656_FORMATTER_0, 0, 0);

    return;
}

static void BVDC_P_Connect656Src_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_Display656Chan           *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t ulSrc = 0;

    BSTD_UNUSED(pstChan); /* For multiple 656 instances in the future */

    /* Connect 656 to source */
    ulSrc = BVDC_P_GetVecCfgSrc_isr(hDisplay);
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_ITU656_DTG_0_SOURCE);
    *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VEC_CFG_ITU656_DTG_0_SOURCE, SOURCE, ulSrc);

    return;
}

static void BVDC_P_Vec_Build_656_DTG_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    const uint32_t *p656Dtram = NULL;
    uint32_t ulTopTrigVal, ulBotTrigVal;
    const BVDC_P_DisplayInfo *pCurInfo = &hDisplay->stCurInfo;

    /* get correct 656 dtram */
    p656Dtram = BVDC_P_Get656DtramTable_isr(pCurInfo);

    /* Load uCode */
    /* Dtram[0..3f] is reserved for 656 */
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_DTRAM_TABLE_SIZE);
#if (BVDC_P_ORTHOGONAL_VEC_VER <= BVDC_P_ORTHOGONAL_VEC_VER_1)
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTRAM_0_DMC_INSTRUCTIONi_ARRAY_BASE);
#else
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DTG_0_DMC_INSTRUCTIONi_ARRAY_BASE);
#endif
    BKNI_Memcpy((void*)pList->pulCurrent, (void*)p656Dtram,
        BVDC_P_DTRAM_TABLE_SIZE * sizeof(uint32_t));
    pList->pulCurrent += BVDC_P_DTRAM_TABLE_SIZE;

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
#if (BVDC_P_ORTHOGONAL_VEC_VER <= BVDC_P_ORTHOGONAL_VEC_VER_1)
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DTRAM_0_DTRAM_CONFIG);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(DTRAM_0_DTRAM_CONFIG, ARBITER_LATENCY, 0xb);
#else
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DTG_0_DTRAM_CONFIG);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(ITU656_DTG_0_DTRAM_CONFIG, ARBITER_LATENCY, 0xb);
#endif

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DTG_0_CCIR_PCL);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(ITU656_DTG_0_CCIR_PCL, HACTIVE_ENABLE,  1) |
        BCHP_FIELD_DATA(ITU656_DTG_0_CCIR_PCL, HACTIVE_SEL,     0) |
        BCHP_FIELD_DATA(ITU656_DTG_0_CCIR_PCL, VACTIVE_ENABLE,  1) |
        BCHP_FIELD_DATA(ITU656_DTG_0_CCIR_PCL, VACTIVE_SEL,     0) |
        BCHP_FIELD_DATA(ITU656_DTG_0_CCIR_PCL, VBLANK_ENABLE,   1) |
        BCHP_FIELD_DATA(ITU656_DTG_0_CCIR_PCL, VBLANK_SEL,      0) |
        BCHP_FIELD_DATA(ITU656_DTG_0_CCIR_PCL, ODD_EVEN_ENABLE, 1) |
        BCHP_FIELD_DATA(ITU656_DTG_0_CCIR_PCL, ODD_EVEN_SEL,    0);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DTG_0_DVI_PCL);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(ITU656_DTG_0_DVI_PCL, DIGITAL_HSYNC_ENABLE, 1) |
        BCHP_FIELD_DATA(ITU656_DTG_0_DVI_PCL, DIGITAL_HSYNC_SEL,    0) |
        BCHP_FIELD_DATA(ITU656_DTG_0_DVI_PCL, DVI_V_ENABLE,     0) |
        BCHP_FIELD_DATA(ITU656_DTG_0_DVI_PCL, DVI_V_SEL,        0) |
        BCHP_FIELD_DATA(ITU656_DTG_0_DVI_PCL, DVI_DE_ENABLE,    0) |
        BCHP_FIELD_DATA(ITU656_DTG_0_DVI_PCL, DVI_DE_SEL,       0);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DTG_0_CNTL_PCL);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(ITU656_DTG_0_CNTL_PCL, NEW_LINE_CLR_SEL, 3);

    /* DTG_RAM_ADDR = 0 for 656out */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DTG_0_RAM_ADDR);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(ITU656_DTG_0_RAM_ADDR, MC_START_ADDR, 0);

    /* DTG_BVB_SIZE (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DTG_0_DTG_BVB_SIZE);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(ITU656_DTG_0_DTG_BVB_SIZE, HORIZONTAL,pCurInfo->pFmtInfo->ulDigitalWidth) |
        BCHP_FIELD_DATA(ITU656_DTG_0_DTG_BVB_SIZE, VERTICAL,  pCurInfo->pFmtInfo->ulDigitalHeight>>(pCurInfo->pFmtInfo->bInterlaced));

    /* Set up triggers */
    ulTopTrigVal = BVDC_P_TRIGGER_LINE;
    ulBotTrigVal = (pCurInfo->pFmtInfo->bInterlaced) ?
        (((pCurInfo->pFmtInfo->ulScanHeight + 1) / 2) + BVDC_P_TRIGGER_LINE) : 0;
#if (BVDC_P_SUPPORT_IT_VER >= 2)
    /* Having IT trigger modulo count set to 2 means the chipset has
     * a OSCL which produces 1080p output. BVN still runs as 1080i.
     */
    if (1 != pCurInfo->ulTriggerModuloCnt)
        ulBotTrigVal = BVDC_P_TRIGGER_LINE;
#endif

    /* DTG_TRIGGER_0 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DTG_0_DTG_TRIGGER_0);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(ITU656_DTG_0_DTG_TRIGGER_0, ENABLE, 0 ) |
#if (BVDC_P_SUPPORT_IT_VER >= 2)
        BCHP_FIELD_DATA(ITU656_DTG_0_DTG_TRIGGER_0, TRIGGER_SELECT, 0) |
        BCHP_FIELD_DATA(ITU656_DTG_0_DTG_TRIGGER_0, MODULO_COUNT, pCurInfo->ulTriggerModuloCnt) |
#endif
        BCHP_FIELD_DATA(ITU656_DTG_0_DTG_TRIGGER_0, TRIGGER_VALUE, ulTopTrigVal );

    /* DVI_DTG_DTG_TRIGGER_1 (RW) */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DTG_0_DTG_TRIGGER_1);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(ITU656_DTG_0_DTG_TRIGGER_1, ENABLE, 0 ) |
#if (BVDC_P_SUPPORT_IT_VER >= 2)
        BCHP_FIELD_DATA(ITU656_DTG_0_DTG_TRIGGER_1, TRIGGER_SELECT, (1 != pCurInfo->ulTriggerModuloCnt) ? 1 : 0) |
        BCHP_FIELD_DATA(ITU656_DTG_0_DTG_TRIGGER_1, MODULO_COUNT, pCurInfo->ulTriggerModuloCnt) |
#endif
        BCHP_FIELD_DATA(ITU656_DTG_0_DTG_TRIGGER_1, TRIGGER_VALUE, ulBotTrigVal );

    return;
}

static void BVDC_P_Vec_Build_656_DVF_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    BSTD_UNUSED(hDisplay);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DVF_0_DVF_CONFIG);
    *pList->pulCurrent++ =
        BCHP_FIELD_ENUM(ITU656_DVF_0_DVF_CONFIG, VBI_PREFERRED,   OFF)|
        BCHP_FIELD_ENUM(ITU656_DVF_0_DVF_CONFIG, VBI_ENABLE,      OFF)|
        BCHP_FIELD_ENUM(ITU656_DVF_0_DVF_CONFIG, UPSAMPLE2X,       ON)|
        BCHP_FIELD_DATA(ITU656_DVF_0_DVF_CONFIG, reserved0,         0)|
        BCHP_FIELD_DATA(ITU656_DVF_0_DVF_CONFIG, reserved_for_eco1, 0);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DVF_0_DVF_VALUES);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(ITU656_DVF_0_DVF_VALUES, CH0_VBI_OFFSET,  1) |
        BCHP_FIELD_DATA(ITU656_DVF_0_DVF_VALUES, CH2_BLANK,     128) |
        BCHP_FIELD_DATA(ITU656_DVF_0_DVF_VALUES, CH1_BLANK,     128) |
        BCHP_FIELD_DATA(ITU656_DVF_0_DVF_VALUES, CH0_BLANK,      16);
}

/*************************************************************************
 *  {secret}
 *  BVDC_P_Vec_Build_656_CSC_isr
 *  Build CSC
 **************************************************************************/
static void BVDC_P_Vec_Build_656_CSC_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    const BVDC_P_DisplayCscMatrix *pCscMatrix = NULL;
    const BVDC_P_DisplayInfo *pCurInfo = &hDisplay->stCurInfo;

    /* --- Setup CSC ---*/
    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_CSC_TABLE_SIZE);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_CSC_0_CSC_MODE);

    BVDC_P_Display_Get656CscTable_isr(&hDisplay->stCurInfo,
        hDisplay->hCompositor->bIsBypass, &pCscMatrix);

    /* TODO: handle user csc */
    hDisplay->st656CscMatrix = *pCscMatrix;

    /* Handle CSC mute */
    if (pCurInfo->abOutputMute[BVDC_DisplayOutput_e656])
    {
        uint8_t ucCh0, ucCh1, ucCh2;
        const BVDC_P_Compositor_Info *pCmpInfo = &hDisplay->hCompositor->stCurInfo;

        /* Blank color, use in CSC */
        ucCh0 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 2);
        ucCh1 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 1);
        ucCh2 = BPXL_GET_COMPONENT(BPXL_eA8_Y8_Cb8_Cr8, pCmpInfo->ulBgColorYCrCb, 0);

        /* Swap ch0 and 1 of input color to match vec csc layout */
        BVDC_P_Csc_ApplyYCbCrColor_isr(&hDisplay->st656CscMatrix.stCscCoeffs, ucCh1, ucCh0, ucCh2);
    }

    BVDC_P_Vec_Build_CSC_isr(&hDisplay->st656CscMatrix, pList);

    *pList->pulCurrent++ = BRDC_OP_IMMS_TO_REGS(BVDC_P_DITHER_TABLE_SIZE);
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_CSC_0_DITHER_CONTROL);
    BVDC_P_Vec_Build_Dither_isr(&hDisplay->st656Dither, pList);

    return;
}

static void BVDC_P_Vec_Build_656_Formatter_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_0_ITU656_CONFIG);
    if (hDisplay->bIsBypass)
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(ITU656_0_ITU656_CONFIG, FILTER_MODE, 0)|
            BCHP_FIELD_ENUM(ITU656_0_ITU656_CONFIG, INPUT_MODE, BYPASS)|
            BCHP_FIELD_ENUM(ITU656_0_ITU656_CONFIG, DATA_OUT_PATTERN, ZERO)|
            BCHP_FIELD_DATA(ITU656_0_ITU656_CONFIG, reserved0, 0);
    }
    else
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(ITU656_0_ITU656_CONFIG, FILTER_MODE, 0)|
            BCHP_FIELD_ENUM(ITU656_0_ITU656_CONFIG, INPUT_MODE, SHARED)|
            BCHP_FIELD_ENUM(ITU656_0_ITU656_CONFIG, DATA_OUT_PATTERN, ZERO)|
            BCHP_FIELD_DATA(ITU656_0_ITU656_CONFIG, reserved0, 0);
    }

    return;
}

static void BVDC_P_Program656Chan_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_Display656Chan           *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    BSTD_UNUSED(pstChan); /* This info will be used when there are multiple 656 cores */

    BVDC_P_Vec_Build_656_DTG_isr(hDisplay, pList);
    BVDC_P_Vec_Build_656_DVF_isr(hDisplay, pList);
    BVDC_P_Vec_Build_656_CSC_isr(hDisplay, pList);
    BVDC_P_Vec_Build_656_Formatter_isr(hDisplay, pList);
    BVDC_P_Display_Setup656TG_isr(hDisplay, pList);
}

static void BVDC_P_Display_Start656Ctrler_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    BSTD_UNUSED(pList);

    if (hDisplay->st656Chan.bEnable)
    {
        uint32_t ulIsSlave;
        ulIsSlave = (hDisplay->eMasterTg == BVDC_DisplayTg_e656Dtg) ? 0 : 1;

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_ITU656_DTG_0_DTG_CONFIG);
        *pList->pulCurrent++ =
            /* master mode 656 out should turn off AUTO_RESTART */
            BCHP_FIELD_DATA(ITU656_DTG_0_DTG_CONFIG, AUTO_RESTART, ulIsSlave) |
            BCHP_FIELD_DATA(ITU656_DTG_0_DTG_CONFIG, RESTART_WIN, 31) |
            BCHP_FIELD_DATA(ITU656_DTG_0_DTG_CONFIG, MCS_ENABLE,   1) |
            BCHP_FIELD_DATA(ITU656_DTG_0_DTG_CONFIG, SLAVE_MODE,  ulIsSlave) |
            BCHP_FIELD_DATA(ITU656_DTG_0_DTG_CONFIG, TRIGGER_CNT_CLR_COND, 0) |
            BCHP_FIELD_DATA(ITU656_DTG_0_DTG_CONFIG, TOGGLE_DVI_H, 0) |
            BCHP_FIELD_DATA(ITU656_DTG_0_DTG_CONFIG, TOGGLE_DVI_V, 0) |
            BCHP_FIELD_DATA(ITU656_DTG_0_DTG_CONFIG, TOGGLE_DVI_DE, 0);
    }
}
#endif /* BVDC_P_SUPPORT_ITU656_OUT != 0 */


static void BVDC_P_Display_StartAnlgCtrler_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t             ulItConfig = 0;
    BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;

    if(hDisplay->bAnlgEnable ||   /* if analog master */
       (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
       (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
    {
        ulItConfig = BVDC_P_GetItConfig_isr(pCurInfo);
        BDBG_ASSERT (ulItConfig);

        if(hDisplay->bAnlgEnable)
        {
            /* Master mode */
            ulItConfig &= ~(BCHP_FIELD_ENUM(IT_0_TG_CONFIG, SLAVE_MODE, ENABLED));
        }
        else
        {
            /* Slave mode */
            ulItConfig |= (BCHP_FIELD_ENUM(IT_0_TG_CONFIG, SLAVE_MODE, ENABLED));
        }

#if (BVDC_P_SUPPORT_IT_VER >= 3)
        if(hDisplay->stCurInfo.ulAnlgChan0Mask == 0 &&
           hDisplay->stCurInfo.ulAnlgChan1Mask == 0)
        {
            /* HDMI slave no analog (require at least IT) */
            ulItConfig |= (BCHP_FIELD_ENUM(IT_0_TG_CONFIG, STAND_ALONE, ON));
        }
        else
        {
            ulItConfig |= (BCHP_FIELD_ENUM(IT_0_TG_CONFIG, STAND_ALONE, OFF));
        }
#endif

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_IT_0_TG_CONFIG + hDisplay->stAnlgChan_0.ulItRegOffset);
        *pList->pulCurrent++ = ulItConfig;
    }
}

static void BVDC_P_Display_StartDviCtrler_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;
    const BVDC_Display_DvoSettings *pDvoSettings =
        &hDisplay->stCurInfo.stDvoCfg;
    BFMT_VideoFmt eCurVideoFmt = pCurInfo->pFmtInfo->eVideoFmt;
    uint32_t ulIsSlave =
        (hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bHdmiRmd) ? 0 : 1;
    uint32_t ulDviAutoRestart = ulIsSlave;

    if(hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bEnableHdmi)
    {
        /* HDMI transmitter needs specific values for DVI/DTG toggles. But
         * custom hardware (display panels) need values provided by user. */
        uint32_t toggles[2];

        if(BVDC_P_IS_CUSTOMFMT(eCurVideoFmt))
        {
            toggles[0] = pDvoSettings->eHsPolarity;
            toggles[1] = pDvoSettings->eVsPolarity;
        }
        else
        {
            const uint32_t* pToggles = BVDC_P_GetDviDtgToggles_isr (pCurInfo);
            toggles[0] = pToggles[0];
            toggles[1] = pToggles[1];
        }

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_0_DTG_CONFIG + hDisplay->stDviChan.ulDviRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(DVI_DTG_0_DTG_CONFIG, AUTO_RESTART,
                                                           ulDviAutoRestart ) |
            BCHP_FIELD_DATA(DVI_DTG_0_DTG_CONFIG, RESTART_WIN,           31 ) |
            BCHP_FIELD_DATA(DVI_DTG_0_DTG_CONFIG, TOGGLE_DVI_DE,
                                                  pDvoSettings->eDePolarity ) |
            BCHP_FIELD_DATA(DVI_DTG_0_DTG_CONFIG, TOGGLE_DVI_V,  toggles[1] ) |
            BCHP_FIELD_DATA(DVI_DTG_0_DTG_CONFIG, TOGGLE_DVI_H,  toggles[0] ) |
            BCHP_FIELD_DATA(DVI_DTG_0_DTG_CONFIG, TRIGGER_CNT_CLR_COND,   0 ) |
            BCHP_FIELD_DATA(DVI_DTG_0_DTG_CONFIG, SLAVE_MODE,     ulIsSlave ) |
            BCHP_FIELD_DATA(DVI_DTG_0_DTG_CONFIG, MCS_ENABLE,             1 ) ;
    }
}

static void BVDC_P_Display_SetFormatSwitchMarker_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    /* This RUL sets a marker to indicate that we are switching
     * format and VEC triggers are disabled. Upon detecting this marker,
     * BVDC_P_CompositorDisplay_isr() will re-enable the triggers.
     */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(hDisplay->ulRdcVarAddr);
    *pList->pulCurrent++ = 1;

}

static void BVDC_P_Display_StartMicroCtrler_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    /* MicroControllers of the master timing generator will
     * be started the last.
     */
    if(hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bHdmiRmd)
    {
        BVDC_P_Display_StartAnlgCtrler_isr(hDisplay, pList);
#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
        BVDC_P_Display_Start656Ctrler_isr(hDisplay, pList);
#endif
        BVDC_P_Display_StartDviCtrler_isr(hDisplay, pList);
    }
    else if(hDisplay->bAnlgEnable)
    {
#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
        BVDC_P_Display_Start656Ctrler_isr(hDisplay, pList);
#endif
        BVDC_P_Display_StartDviCtrler_isr(hDisplay, pList);
        BVDC_P_Display_StartAnlgCtrler_isr(hDisplay, pList);
    }
    else if(hDisplay->st656Chan.bEnable)
    {
        BVDC_P_Display_StartAnlgCtrler_isr(hDisplay, pList);
        BVDC_P_Display_StartDviCtrler_isr(hDisplay, pList);
#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
        BVDC_P_Display_Start656Ctrler_isr(hDisplay, pList);
#endif
    }
    return;
}

#if (!BVDC_P_USE_RDC_TIMESTAMP)
static void BVDC_P_Display_SnapshotTimer_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    *pList->pulCurrent++ = BRDC_OP_REG_TO_VAR(BRDC_Variable_0);
    *pList->pulCurrent++ = BRDC_REGISTER(hDisplay->stTimerReg.status);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_VAR(BRDC_Variable_1);
    *pList->pulCurrent++ = BCHP_TIMER_TIMER0_STAT_COUNTER_VAL_MASK;

    *pList->pulCurrent++ = BRDC_OP_VAR_AND_VAR_TO_VAR(BRDC_Variable_0, BRDC_Variable_1, BRDC_Variable_2);

    *pList->pulCurrent++ = BRDC_OP_VAR_TO_REG(BRDC_Variable_2);
    *pList->pulCurrent++ = BRDC_REGISTER(hDisplay->ulScratchTsAddr);

    return;
}
#endif

    /****************************************************************
     *                                                              *
     *                    Display Event Handlers                    *
     *                                                              *
     ****************************************************************/

/* Validation helper functions */
static BERR_Code BVDC_P_Display_Validate_AnalogChan
    ( BVDC_Display_Handle              hDisplay )
{
    const uint32_t          *pTable;
    BAVC_VdcDisplay_Info     lRateInfo;
    BVDC_P_DisplayInfo      *pNewInfo = &hDisplay->stNewInfo;
    bool                     bMissingTable = false;

    bMissingTable |= (BVDC_P_GetRmTable_isr(
        pNewInfo, pNewInfo->pFmtInfo, &pTable, true, &lRateInfo)
        != BERR_SUCCESS);
    if (pNewInfo->bMultiRateAllow)
    {
        bMissingTable |= (BVDC_P_GetRmTable_isr(
            pNewInfo, pNewInfo->pFmtInfo, &pTable, false, &lRateInfo)
            != BERR_SUCCESS);
    }
    if (bMissingTable)
    {
        BDBG_ERR(("Failed to locate Rate Manager Table"));
        return
            BERR_TRACE(BVDC_ERR_FORMAT_NOT_SUPPORT_ANALOG_OUTPUT);
    }

    /* Need this to trap out 240P@60Hz */
    pTable = BVDC_P_GetRamTable_isr(pNewInfo, hDisplay->bArib480p);
    if (pTable == NULL && !BFMT_IS_4kx2k(pNewInfo->pFmtInfo->eVideoFmt))
    {
        BDBG_ERR(("Failed to locate RAM table"));
        return
            BERR_TRACE(BVDC_ERR_FORMAT_NOT_SUPPORT_ANALOG_OUTPUT);
    }

    return BERR_SUCCESS;
}

static BERR_Code BVDC_P_Display_Validate_DviRm
    ( const BVDC_P_DisplayInfo *pDispInfo,
      const BFMT_VideoInfo     *pFmtInfo,
      bool                      bFullRate,
      BVDC_P_DisplayDviChan    *pstChan )
{
    const uint32_t          *pTable;
    BAVC_VdcDisplay_Info     lRateInfo;
    uint64_t                 ulPixelClkRate;
    const BVDC_P_RateInfo*   pRateInfo;

    if(BVDC_P_GetRmTable_isr(pDispInfo, pFmtInfo, &pTable, bFullRate, &lRateInfo) != BERR_SUCCESS)
    {
        BDBG_ERR(("Failed to get Rate Manager table"));
        return BERR_TRACE(BVDC_ERR_INVALID_HDMI_MODE);
    }

    /* Analog VEC operates at double rate for 480P and 576P. So we have to
     * convert back to single rate frequency to access the HDMI rate table. */
    ulPixelClkRate = lRateInfo.ulPixelClkRate;
    if (VIDEO_FORMAT_IS_ED(pFmtInfo->eVideoFmt))
    {
        if (ulPixelClkRate & BFMT_PXL_54MHz)
        {
            ulPixelClkRate &= ~BFMT_PXL_54MHz;
            ulPixelClkRate |=  BFMT_PXL_27MHz;
        }
        if (ulPixelClkRate & BFMT_PXL_54MHz_MUL_1_001)
        {
            ulPixelClkRate &= ~BFMT_PXL_54MHz_MUL_1_001;
            ulPixelClkRate |=  BFMT_PXL_27MHz_MUL_1_001;
        }
    }

#if BVDC_P_SUPPORT_MHL
    /* Look up MHL frequency and use this to look up RM and PLL parameters */
    if (pstChan->bMhlMode)
    {
        ulPixelClkRate = BVDC_P_PxlFreqToMhlFreq_isr(ulPixelClkRate);
        BDBG_ASSERT(ulPixelClkRate);
    }
#else
    BSTD_UNUSED(pstChan);
#endif

    (void)BVDC_P_HdmiRmTable_isr(
        pFmtInfo->eVideoFmt,
        ulPixelClkRate,
        pDispInfo->stHdmiSettings.eHdmiColorDepth,
        pDispInfo->stHdmiSettings.eHdmiPixelRepetition,
        pDispInfo->stHdmiSettings.stSettings.eColorComponent,
        &pRateInfo);
    if (pRateInfo == NULL)
    {
        BDBG_ERR(("Failed to allocate HDMI Rate Manager settings for video format %d(%s), \
            pixel clock rate 0x%x, HDMI color depth %d, pixel repetition %d, color component %d",
            pFmtInfo->eVideoFmt, pFmtInfo->pchFormatStr, (uint32_t)ulPixelClkRate,
            pDispInfo->stHdmiSettings.eHdmiColorDepth,
            pDispInfo->stHdmiSettings.eHdmiPixelRepetition,
            pDispInfo->stHdmiSettings.stSettings.eColorComponent));
        return BERR_TRACE(BVDC_ERR_INVALID_HDMI_MODE);
    }

    return BERR_SUCCESS;
}

static BERR_Code BVDC_P_Display_Validate_DviChan
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo *pNewInfo = &hDisplay->stNewInfo;
    const BFMT_VideoInfo *pFmtInfo;

    pFmtInfo = (pNewInfo->bHdmiFmt)
        ? pNewInfo->pHdmiFmtInfo : pNewInfo->pFmtInfo;

    if (!VIDEO_FORMAT_IS_HDMI(pFmtInfo->eVideoFmt))
    {
        BDBG_ERR(("DISP[%d] Invalid HDMI video format (%s)", hDisplay->eId, pFmtInfo->pchFormatStr));
        return BERR_TRACE(BVDC_ERR_INVALID_HDMI_MODE);
    }

    if((BVDC_P_Display_Validate_DviRm(pNewInfo, pFmtInfo, true, &hDisplay->stDviChan)  != BERR_SUCCESS) ||
       (BVDC_P_Display_Validate_DviRm(pNewInfo, pFmtInfo, false, &hDisplay->stDviChan) != BERR_SUCCESS))
    {
        BDBG_ERR(("DISP[%d] Invalid HDMI video format (%s)", hDisplay->eId, pFmtInfo->pchFormatStr));
        return BERR_TRACE(BVDC_ERR_INVALID_HDMI_MODE);
    }

    return BERR_SUCCESS;
}

#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
static BERR_Code BVDC_P_Display_Validate_656Chan
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo      *pNewInfo = &hDisplay->stNewInfo;
    const uint32_t          *pTable;
    BAVC_VdcDisplay_Info     lRateInfo;
    bool                     bMissingTable = false;

    if (!BFMT_IS_525_LINES(pNewInfo->pFmtInfo->eVideoFmt)
        &&
        !BFMT_IS_625_LINES(pNewInfo->pFmtInfo->eVideoFmt))
    {
        BDBG_ERR(("Invalid video format %d for 656", pNewInfo->pFmtInfo->eVideoFmt));
        return BERR_TRACE(BVDC_ERR_INVALID_656OUT_USE);
    }

    bMissingTable |= (BVDC_P_GetRmTable_isr(
        pNewInfo, pNewInfo->pFmtInfo, &pTable, true, &lRateInfo)
        != BERR_SUCCESS);
    if (pNewInfo->bMultiRateAllow)
    {
        bMissingTable |= (BVDC_P_GetRmTable_isr(
            pNewInfo, pNewInfo->pFmtInfo, &pTable, false, &lRateInfo)
            != BERR_SUCCESS);
    }
    if (bMissingTable)
    {
        BDBG_ERR(("Failed to get Rate Manager table"));
        return BERR_TRACE(BVDC_ERR_INVALID_656OUT_MODE);
    }

    return BERR_SUCCESS;
}
#endif

/**************** New analog channel event hanlders **************/
static void BVDC_P_Display_Apply_Chan_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayAnlgChan          *pstChan,
      BVDC_P_ListInfo                 *pList )
{
    bool bHwBugWorkAround = false;

#if BVDC_P_VEC_STANDALONE_BUG_FIXED
    bHwBugWorkAround = false;
#else
    bHwBugWorkAround = hDisplay->bAnlgEnable &&
        (((pstChan->ulId == 0) &&
        ((hDisplay->stAnlgChan_1.eState == BVDC_P_DisplayResource_eDestroy) ||
         (hDisplay->stAnlgChan_1.eState == BVDC_P_DisplayResource_eShuttingdown) ||
         (hDisplay->stAnlgChan_1.eState == BVDC_P_DisplayResource_eInactive))) ||
         ((pstChan->ulId == 1) &&
          (hDisplay->stAnlgChan_0.eState == BVDC_P_DisplayResource_eInactive)));
#endif

    switch (pstChan->eState)
    {
        case BVDC_P_DisplayResource_eCreate:
            BDBG_MSG(("Display %d set up AnlgChan_%d", hDisplay->eId, pstChan->ulId));
            BVDC_P_ProgramAnalogChan_isr(hDisplay, pstChan, pList);
            BVDC_P_SetupAnalogChan_isr(hDisplay->hVdc->hResource, pstChan, pList);
            pstChan->eState = BVDC_P_DisplayResource_eActivating;
            break;

        case BVDC_P_DisplayResource_eActivating:
            if (pList->bLastExecuted)
            {
                BDBG_MSG(("Display %d AnlgChan_%d is active", hDisplay->eId, pstChan->ulId));
                pstChan->eState = BVDC_P_DisplayResource_eActive;
                if(pstChan->ulId == 0)
                {
                    hDisplay->stCurInfo.stDirty.stBits.bChan0 = BVDC_P_CLEAN;
                }
                else
                {
                    hDisplay->stCurInfo.stDirty.stBits.bChan1 = BVDC_P_CLEAN;
                }
            }
            else
            {
                BDBG_MSG(("Display %d set up AnlgChan_%d again because last RUL didn't execute",
                    hDisplay->eId, pstChan->ulId));
                BVDC_P_ProgramAnalogChan_isr(hDisplay, pstChan, pList);
                BVDC_P_SetupAnalogChan_isr(hDisplay->hVdc->hResource, pstChan, pList);
            }
            break;

        case BVDC_P_DisplayResource_eDestroy:
            if(bHwBugWorkAround)
            {
                BDBG_MSG(("No Dacs - Don't tear down Chan%d", pstChan->ulId));
            }
            else
            {
                BDBG_MSG(("Display %d tear down AnlgChan_%d", hDisplay->eId, pstChan->ulId));
                BVDC_P_TearDownAnalogChan_isr(pstChan, pList);
                pstChan->bEnable = false;
                pstChan->bTearDown = true;
            }
            pstChan->eState = BVDC_P_DisplayResource_eShuttingdown;
            break;

        case BVDC_P_DisplayResource_eShuttingdown:
            /* We can reach here for two reasons:
             * 1) The channel one tear down RUL has been exectued and we need to free the resources;
             * 2) The channel one tear down RUL somehow didn't get executed. So we have to buile
             *    and run the RUL again.
             */
            if (pList->bLastExecuted)
            {
                if(!pstChan->bTearDown)
                {
                    BDBG_MSG(("No Dacs - Keep Chan%d Resources", pstChan->ulId));
                    pstChan->eState = BVDC_P_DisplayResource_eResInactive;
                }
                else
                {
                    BDBG_MSG(("Display %d free AnlgChan_%d resources", hDisplay->eId, pstChan->ulId));
                    /* if not analog master, free IT resource as well */
                    if(!hDisplay->bAnlgEnable)
                        BVDC_P_FreeITResources_isr(hDisplay->hVdc->hResource, pstChan);
                    BVDC_P_FreeAnalogChanResources_isr(hDisplay->hVdc->hResource, pstChan);
                    pstChan->eState = BVDC_P_DisplayResource_eInactive;
                }
                pstChan->bTearDown = false;
                BVDC_P_FreeDacResources_isr(hDisplay->hVdc->hResource, pstChan);
                if(pstChan->ulId == 0)
                {
                    hDisplay->stCurInfo.stDirty.stBits.bChan0 = BVDC_P_CLEAN;
                }
                else
                {
                    hDisplay->stCurInfo.stDirty.stBits.bChan1 = BVDC_P_CLEAN;
                }
            }
            else
            {
                if(pstChan->bTearDown)
                {
                    BDBG_MSG(("Display %d tear down AnlgChan_%d because last RUL didn't execute",
                        hDisplay->eId, pstChan->ulId));
                    BVDC_P_TearDownAnalogChan_isr(pstChan, pList);
                }
            }
            break;

        default:
            /* Do nothing */
            if(pstChan->ulId == 0)
            {
                hDisplay->stCurInfo.stDirty.stBits.bChan0 = BVDC_P_CLEAN;
            }
            else
            {
                hDisplay->stCurInfo.stDirty.stBits.bChan1 = BVDC_P_CLEAN;
            }
            break;
    }

    return;
}

/**************** New analog channel 0 event hanlders **************/
static void BVDC_P_Display_Apply_Chan0_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BSTD_UNUSED(eFieldPolarity);
    BVDC_P_Display_Apply_Chan_isr(hDisplay, &hDisplay->stAnlgChan_0, pList);
    return;
}

/**************** New analog channel 1 event hanlders **************/
static void BVDC_P_Display_Apply_Chan1_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BSTD_UNUSED(eFieldPolarity);
    BVDC_P_Display_Apply_Chan_isr(hDisplay, &hDisplay->stAnlgChan_1, pList);
    return;
}

/**************** New video format (output timing) event hanlders **************/
static BERR_Code BVDC_P_Display_Validate_VideoFormat
    ( BVDC_Display_Handle              hDisplay )
{
    BERR_Code err;
    BVDC_P_DisplayInfo *pNewInfo;
    BVDC_P_DisplayInfo *pCurInfo;

    /* Currently video format validation is actually divided into two parts.
     *
     * 1) New video format vs DAC assignment.
     *
     * This is achieved by BVDC_P_Display_ValidateDacSettings() in bvdc_display_priv.c.
     * The reason is that DACs are considered as resources shared among all the
     * displays. Any DAC assignment change to one display could possibly affect other
     * displays. BVDC_P_Display_ValidateChanges() is a place we have access to all
     * display handles.
     *
     *
     * 2) New video format for a particular display.
     *
     * Here we validate items like if we have valid settings such as RM table for this format,
     * outputs (analog/HDMI/ITU-656) allows this format, etc.
     */

    pNewInfo = &hDisplay->stNewInfo;
    pCurInfo = &hDisplay->stCurInfo;

    if(hDisplay->bAnlgEnable ||   /* if analog master */
       (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
       (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
    {
        if ((err = BVDC_P_Display_Validate_AnalogChan(hDisplay)) !=
            BERR_SUCCESS)
        {
            return BERR_TRACE(err);
        }

        if ((err = BVDC_P_ValidateMacrovision (hDisplay)) != BERR_SUCCESS)
        {
            return BERR_TRACE(err);
        }

        /* validate secam */
        if(BFMT_IS_SECAM(pCurInfo->pFmtInfo->eVideoFmt) !=
           BFMT_IS_SECAM(pNewInfo->pFmtInfo->eVideoFmt))
        {
            if(hDisplay->stAnlgChan_0.bEnable && hDisplay->stAnlgChan_0.eState != BVDC_P_DisplayResource_eDestroy)
            {
                BDBG_MSG(("Realocate Analog resource for chan 0"));
                BKNI_EnterCriticalSection();
                err = BVDC_P_AllocAnalogChanResources(hDisplay->hVdc->hResource, hDisplay->eId * 2, &hDisplay->stAnlgChan_0,
                            (!(hDisplay->stNewInfo.ulAnlgChan0Mask & BVDC_P_Dac_Mask_SD) && hDisplay->bHdCap) ? true : false ,
                            (hDisplay->stNewInfo.ulAnlgChan0Mask & BVDC_P_Dac_Mask_HD) ? true : false ,
                            ((hDisplay->stNewInfo.ulAnlgChan0Mask & BVDC_P_Dac_Mask_SD) && (BVDC_P_NUM_SHARED_SECAM != 0) && (BFMT_IS_SECAM(pNewInfo->pFmtInfo->eVideoFmt))) ? true : false);
                BKNI_LeaveCriticalSection();
            }
            if(hDisplay->stAnlgChan_1.bEnable && hDisplay->stAnlgChan_1.eState != BVDC_P_DisplayResource_eDestroy)
            {
                BDBG_MSG(("Realocate Analog resource for chan 1"));
                BKNI_EnterCriticalSection();
                err |= BVDC_P_AllocAnalogChanResources(hDisplay->hVdc->hResource, hDisplay->eId * 2 + 1, &hDisplay->stAnlgChan_1,
                            (!(hDisplay->stNewInfo.ulAnlgChan1Mask & BVDC_P_Dac_Mask_SD) && hDisplay->bHdCap) ? true : false ,
                            (hDisplay->stNewInfo.ulAnlgChan1Mask & BVDC_P_Dac_Mask_HD) ? true : false ,
                            ((hDisplay->stNewInfo.ulAnlgChan1Mask & BVDC_P_Dac_Mask_SD) && (BVDC_P_NUM_SHARED_SECAM != 0) && (BFMT_IS_SECAM(pNewInfo->pFmtInfo->eVideoFmt))) ? true : false);
                BKNI_LeaveCriticalSection();
            }
            if(err != BERR_SUCCESS)
                return BERR_TRACE(err);
        }
    }

    if(hDisplay->stDviChan.bEnable || hDisplay->stNewInfo.bEnableHdmi)
    {
        if ((err = BVDC_P_Display_Validate_DviChan(hDisplay)) != BERR_SUCCESS)
            return BERR_TRACE(err);
    }

#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
    if (hDisplay->st656Chan.bEnable)
    {
        if ((err = BVDC_P_Display_Validate_656Chan(hDisplay)) != BERR_SUCCESS)
            return BERR_TRACE(err);
    }
#endif

    return BERR_SUCCESS;
}

#if (BVDC_P_SUPPORT_STG)
static void BVDC_P_Display_StartStgRamp_VideoFormat_isr
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo* pCurInfo = &hDisplay->stCurInfo;
    BVDC_P_DisplayInfo* pNewInfo = &hDisplay->stNewInfo;

    /* if stg resol ramp, back up the final ramped resol, and shrink the transient resol by half as custom format. */
    if(pNewInfo->ulResolutionRampCount)
    {
        unsigned i;
        /* back up the final format info */
        hDisplay->eStgRampFmt = pCurInfo->pFmtInfo->eVideoFmt;
        if(BVDC_P_IS_CUSTOMFMT(pCurInfo->pFmtInfo->eVideoFmt))
        {
            hDisplay->stStgRampCustomFmt = pCurInfo->stCustomFmt;
        }
        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            if(!hDisplay->hCompositor->ahWindow[i] ||
                BVDC_P_STATE_IS_CREATE(hDisplay->hCompositor->ahWindow[i]) ||
                BVDC_P_STATE_IS_INACTIVE(hDisplay->hCompositor->ahWindow[i]))
            {
                continue;
            }
            hDisplay->astStgRampWinDst[i] = hDisplay->hCompositor->ahWindow[i]->stNewInfo.stDstRect;
            hDisplay->astStgRampWinSclOut[i] = hDisplay->hCompositor->ahWindow[i]->stNewInfo.stScalerOutput;
        }
        /* update the transient/ramp format info */
        pCurInfo->stCustomFmt = *(pNewInfo->pFmtInfo);
        pCurInfo->stCustomFmt.eVideoFmt = BFMT_VideoFmt_eCustom2;
        pCurInfo->stCustomFmt.ulDigitalWidth  /= BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pCurInfo->stCustomFmt.ulDigitalHeight /= BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pCurInfo->stCustomFmt.ulWidth  /= BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pCurInfo->stCustomFmt.ulHeight /= BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pCurInfo->stCustomFmt.ulVertFreq /= BVDC_P_STG_RESOL_RAMP_SIZE_RATIO;
        pCurInfo->pFmtInfo = &pCurInfo->stCustomFmt;
        pCurInfo->ulResolutionRampCount = pNewInfo->ulResolutionRampCount;
        BDBG_MSG(("Disp%u ramp resol: %ux%up%u", hDisplay->eId,
            pCurInfo->stCustomFmt.ulDigitalWidth, pCurInfo->stCustomFmt.ulDigitalHeight, pCurInfo->stCustomFmt.ulVertFreq));
    }
    pNewInfo->ulResolutionRampCount = 0; /* clear it so be one-shot */
}

static void BVDC_P_Display_UpdateStgRamp_VideoFormat_isr
    ( BVDC_Display_Handle              hDisplay )
{
    BDBG_MSG(("Disp%u pStgFmt %s[%ux%u%c%u], pFmt %s[%ux%u%c%u]; ramp?%u", hDisplay->eId,
        hDisplay->pStgFmtInfo?hDisplay->pStgFmtInfo->pchFormatStr : "",
        hDisplay->pStgFmtInfo?hDisplay->pStgFmtInfo->ulDigitalWidth:1,
        hDisplay->pStgFmtInfo?hDisplay->pStgFmtInfo->ulDigitalHeight:1,
        hDisplay->pStgFmtInfo?(hDisplay->pStgFmtInfo->bInterlaced?'i':'p'):'n',
        hDisplay->pStgFmtInfo? hDisplay->pStgFmtInfo->ulVertFreq : 1,
        hDisplay->stCurInfo.pFmtInfo->pchFormatStr, hDisplay->stCurInfo.pFmtInfo->ulDigitalWidth, hDisplay->stCurInfo.pFmtInfo->ulDigitalHeight,
        hDisplay->stCurInfo.pFmtInfo->bInterlaced?'i':'p',hDisplay->stCurInfo.pFmtInfo->ulVertFreq,
        hDisplay->stCurInfo.ulResolutionRampCount));
    /* decrement stg ramp count if non-ignore picture */
    if((hDisplay->stCurInfo.ulResolutionRampCount>1) && !hDisplay->hCompositor->bIgnorePicture)
    {
        hDisplay->stCurInfo.ulResolutionRampCount--;
        BDBG_MSG(("Disp%u resol ramp count = %u", hDisplay->eId, hDisplay->stCurInfo.ulResolutionRampCount));
    } else if ((hDisplay->stCurInfo.ulResolutionRampCount <= 1) && hDisplay->eStgRampFmt != BFMT_VideoFmt_eMaxCount) {/* resume the final rampup resol */
        unsigned i;

        hDisplay->stCurInfo.ulResolutionRampCount = 0;
        hDisplay->stCurInfo.pFmtInfo = BFMT_GetVideoFormatInfoPtr_isr(hDisplay->eStgRampFmt);
        if(BVDC_P_IS_CUSTOMFMT(hDisplay->eStgRampFmt))
        {
            hDisplay->stCurInfo.stCustomFmt = hDisplay->stStgRampCustomFmt;
            hDisplay->stCurInfo.pFmtInfo = &hDisplay->stCurInfo.stCustomFmt;
        }
        hDisplay->eStgRampFmt = BFMT_VideoFmt_eMaxCount;
        /* resume window rect */
        for(i = 0; i < BVDC_P_MAX_WINDOW_COUNT; i++)
        {
            if(!hDisplay->hCompositor->ahWindow[i] ||
                BVDC_P_STATE_IS_CREATE(hDisplay->hCompositor->ahWindow[i]) ||
                BVDC_P_STATE_IS_INACTIVE(hDisplay->hCompositor->ahWindow[i]))
            {
                continue;
            }
            hDisplay->hCompositor->ahWindow[i]->stCurInfo.stDstRect = hDisplay->astStgRampWinDst[i];
            hDisplay->hCompositor->ahWindow[i]->stCurInfo.stScalerOutput = hDisplay->astStgRampWinSclOut[i];
            hDisplay->hCompositor->ahWindow[i]->bAdjRectsDirty = BVDC_P_DIRTY;/* recompute window rectangles */
        }
        BDBG_MSG(("Disp%u resol resumed to %s[%ux%u%c]", hDisplay->eId, hDisplay->stCurInfo.pFmtInfo->pchFormatStr,
            hDisplay->stCurInfo.pFmtInfo->ulDigitalWidth, hDisplay->stCurInfo.pFmtInfo->ulDigitalHeight,
            hDisplay->stCurInfo.pFmtInfo->bInterlaced? 'i' : 'p'));
    }
}
#endif

static void BVDC_P_Display_Copy_VideoFormat_isr
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo* pCurInfo = &hDisplay->stCurInfo;
    BVDC_P_DisplayInfo* pNewInfo = &hDisplay->stNewInfo;
    int i;

#if (BVDC_P_SUPPORT_STG)
    {
        /* 6 is the maximum difference between frequency-tracking support freq: 6000/5994, 2400/2397 */
        bool     bInterlacedChange = false;

        if((hDisplay->pStgFmtInfo) && (hDisplay->pStgFmtInfo->bInterlaced != pNewInfo->pFmtInfo->bInterlaced))
            bInterlacedChange = true;

        /* apply stg fmt right away when no window connected */
        if((NULL == hDisplay->hCompositor->hSyncLockWin) &&
            BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg))
        {
            hDisplay->pStgFmtInfo = pNewInfo->pFmtInfo;
            /* if resol ramp, use custom format as transient format */
            if(pNewInfo->ulResolutionRampCount)
            {
                hDisplay->pStgFmtInfo = &pCurInfo->stCustomFmt;
            }
        }

        if((BVDC_P_ItState_eActive == hDisplay->eItState) &&
            BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg) &&
            (!pCurInfo->bStgNonRealTime) && (!pNewInfo->bStgNonRealTime) &&
            (bInterlacedChange))
        {
            hDisplay->pStgFmtInfo = NULL;
            BDBG_MSG(("disp[%d] STG [%d] change i/p", hDisplay->eId, hDisplay->stStgChan.ulStg));
        }
    }

#endif

    BDBG_MSG(("dsp[%d] Format change %s -> %s", hDisplay->eId,
        pCurInfo->pFmtInfo->pchFormatStr, pNewInfo->pFmtInfo->pchFormatStr));
    pCurInfo->pFmtInfo     = pNewInfo->pFmtInfo;

    if(BVDC_P_IS_CUSTOMFMT(pNewInfo->pFmtInfo->eVideoFmt))
    {
        pCurInfo->stCustomFmt = pNewInfo->stCustomFmt;
        pCurInfo->pFmtInfo    = &pNewInfo->stCustomFmt;
    }
#if (BVDC_P_SUPPORT_STG)
    BVDC_P_Display_StartStgRamp_VideoFormat_isr(hDisplay);
    if(!BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg))
#endif
    pCurInfo->ulVertFreq   = pNewInfo->ulVertFreq;
    pCurInfo->ulHeight     = pNewInfo->ulHeight;
    pCurInfo->eOrientation = pNewInfo->eOrientation;

    pCurInfo->eAnlg_0_OutputColorSpace = pNewInfo->eAnlg_0_OutputColorSpace;
    pCurInfo->eAnlg_1_OutputColorSpace = pNewInfo->eAnlg_1_OutputColorSpace;
    pCurInfo->eCmpMatrixCoeffs = pNewInfo->eCmpMatrixCoeffs;
    pCurInfo->bMultiRateAllow = pNewInfo->bMultiRateAllow;

    /* This is an event handler for analog copy protection (ACP) */
    BVDC_P_Display_Copy_Acp_isr (hDisplay);

    /* We set to Switch Mode here instead of in ISR.
     * This is just to accommdate the legacy logic in other sub-lib.
     */
    if ((hDisplay->eItState != BVDC_P_ItState_eNotActive) &&
        (!BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg)))
        hDisplay->eItState = BVDC_P_ItState_eSwitchMode;

#if (BVDC_P_SUPPORT_IT_VER >= 2)
    pCurInfo->ulTriggerModuloCnt = pNewInfo->ulTriggerModuloCnt;
#endif

    for (i = 0; i < BFMT_VideoFmt_eMaxCount; i++)
    {
        pCurInfo->aulHdmiDropLines[i] = pNewInfo->aulHdmiDropLines[i];
    }
}

static void BVDC_P_Display_Apply_VideoFormat_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BSTD_UNUSED(eFieldPolarity);

    /* The copy handler BVDC_P_Display_Copy_VideoFormat() sets hDisplay->eItState
     * to BVDC_P_ItState_eSwitchMode. If BVDC_P_Display_Apply_VideoFormat() is
     * invoked by normal apply change path, BVDC_P_Display_Copy_VideoFormat() will
     * get executed hence the state gets set. If BVDC_P_Display_Apply_VideoFormat()
     * gets invoked because the previous RUL didn't get executed or partially executed
     * and eItState was changed to BVDC_P_ItState_eActive, we have to force
     * eItState to BVDC_P_ItState_eSwitchMode so that BVDC_P_CompositorDisplay_isr()
     * knows it needs to enable VEC trigger again. */
    if ((hDisplay->eItState  != BVDC_P_ItState_eNotActive ) &&
        (hDisplay->eItState  != BVDC_P_ItState_eSwitchMode) &&
        (!BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg)))
        hDisplay->eItState = BVDC_P_ItState_eSwitchMode;

    if(hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bEnableHdmi)
    {
        /*
         * 1) Tear down previous path and free up acquired resources
         */
        BVDC_P_TearDownDviChan_isr(hDisplay, &hDisplay->stDviChan, pList);

        /*
         * 2) Acquire necessary resources and set up the new path
         */
        BDBG_MSG(("BVDC_P_Display_Apply_VideoFormat Display %d allocates resource for DviChan", hDisplay->eId));
        BVDC_P_SetupDviChan_isr(hDisplay, &hDisplay->stDviChan, pList);

        /*
         * 3) Program the modules to the new format
         */
        BVDC_P_ProgramDviChan_isr(hDisplay, &hDisplay->stDviChan, true, pList);
        BVDC_P_ConnectDviSrc_isr(hDisplay, &hDisplay->stDviChan, pList);
    }

    if((hDisplay->bAnlgEnable) ||   /* if analog master */
       (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
       (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
    {
        /*
         * 1) Tear down IT
         */
        BDBG_MSG(("VideoFmt: Tear down display[%d] IT", hDisplay->eId));
        BVDC_P_TearDownIT_isr(&hDisplay->stAnlgChan_0, pList);

        if(!hDisplay->stCurInfo.bHdmiRmd)
        {
            /*
             * 2) Program the modules to the new format
             *    Note: Format changing involves reprogramming all the settings
             *          in the current info. We will call each event handler
             *          and clears all the dirty bits.
             */
            BVDC_P_ProgramIT_isr(
                hDisplay, &hDisplay->stAnlgChan_0, true, pList);

            /*
             * 3) Set up the new path
             */
            BDBG_MSG(("VideoFmt: Setup display[%d] IT", hDisplay->eId));
            BVDC_P_SetupIT_isr(hDisplay, &hDisplay->stAnlgChan_0, pList);
        }
    }

    if (hDisplay->stAnlgChan_0.bEnable)
    {
        /*
         * 1) Tear down previous path
         */
        BDBG_MSG(("VideoFmt: Tear down display[%d] analog channel 0", hDisplay->eId));
        BVDC_P_TearDownAnalogChan_isr(&hDisplay->stAnlgChan_0, pList);

        /*
         * 2) Program the new path
         */
        BVDC_P_ProgramAnalogChan_isr(hDisplay, &hDisplay->stAnlgChan_0, pList);

        /*
         * 3) Set up the new path
         */
        BDBG_MSG(("VideoFmt: Setup display[%d] analog channel 0", hDisplay->eId));
        BVDC_P_SetupAnalogChan_isr(hDisplay->hVdc->hResource, &hDisplay->stAnlgChan_0, pList);
    }

    /* Secam_0 shares reset with Secam_3 (pass_thru Secam). So we
     * don't use Secam_0.
     */
    if (hDisplay->stAnlgChan_1.bEnable)
    {
        /*
         * 1) Tear down previous path and free up acquired resources
         */
        BDBG_MSG(("VideoFmt: Tear down display[%d] analog channel 1 ", hDisplay->eId));
        BVDC_P_TearDownAnalogChan_isr(&hDisplay->stAnlgChan_1, pList);

        /*
         * 2) Program the new path
         */
        BVDC_P_ProgramAnalogChan_isr(hDisplay, &hDisplay->stAnlgChan_1, pList);

        /*
         * 3) Set up the new path
         */
        BDBG_MSG(("VideoFmt: Setup display[%d] analog channel 1", hDisplay->eId));
        BVDC_P_SetupAnalogChan_isr(hDisplay->hVdc->hResource, &hDisplay->stAnlgChan_1, pList);
    }

#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
    if (hDisplay->st656Chan.bEnable)
    {
        /*
         * 1) Tear down previous path and free up acquired resources
         */
        BVDC_P_TearDown656Chan_isr(hDisplay, &hDisplay->st656Chan, pList);

        /*
         * 2) Acquire necessary resources and set up the new path
         */
        BDBG_MSG(("BVDC_P_Display_Apply_VideoFormat Display %d allocates resource for 656Chan", hDisplay->eId));
        BVDC_P_Setup656Chan_isr(hDisplay, &hDisplay->st656Chan, pList);

        /*
         * 3) Program the modules to the new format
         */
        BVDC_P_Program656Chan_isr(hDisplay, &hDisplay->st656Chan, pList);
        BVDC_P_Connect656Src_isr(hDisplay, &hDisplay->st656Chan, pList);
    }
#endif

#if (BVDC_P_SUPPORT_STG)
    if((hDisplay->stCurInfo.bEnableStg) &&
        (BVDC_P_DISPLAY_NODELAY(hDisplay->pStgFmtInfo, hDisplay->stCurInfo.pFmtInfo) ||
         (!BVDC_P_IS_CUSTOMFMT_DIFF(hDisplay->pStgFmtInfo, hDisplay->stCurInfo.pFmtInfo))))
    {
        /*
         * 1) Tear down previous path and free up acquired resources;
         *    Note: STG has no extra resource to free, and the only crossbar could be immediately switched without teardown;
         *          Removing the teardown is no harm and more importantly, it avoids masking next timer trigger!
         */

        /*
         * 2) Acquire necessary resources and set up the new path
         */
        BDBG_MSG(("BVDC_P_Display_Apply_VideoFormat Display %d connect STG chan", hDisplay->eId));
        BVDC_P_ConnectStgSrc_isr(hDisplay, pList);

        /*
         * 3) Program the modules to the new format
         */
        BVDC_P_ProgramStgChan_isr(hDisplay, pList);
        hDisplay->eStgState = BVDC_P_DisplayResource_eActive;
        hDisplay->stCurInfo.stDirty.stBits.bStgEnable = BVDC_P_CLEAN;
    }
    if(!BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg) ||
        (BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg) && (BVDC_P_ItState_eSwitchMode == hDisplay->eItState)))
#endif

    BVDC_P_Display_SetFormatSwitchMarker_isr(hDisplay, pList);

#if (BVDC_P_SUPPORT_STG)
    if(hDisplay->stStgChan.bEnable)
    {
        /*bool bWidth, bHeight, bVertFreq, bStg, bCur;*/
        bool bDiffFmt = ((!BVDC_P_DISPLAY_NODELAY(hDisplay->pStgFmtInfo, hDisplay->stCurInfo.pFmtInfo)) ||  /* regular fmt */
                        BVDC_P_IS_CUSTOMFMT_DIFF(hDisplay->pStgFmtInfo, hDisplay->stCurInfo.pFmtInfo));     /* custom fmt */

        hDisplay->stCurInfo.stDirty.stBits.bTiming = bDiffFmt || hDisplay->stCurInfo.ulResolutionRampCount;
        BVDC_P_Display_UpdateStgRamp_VideoFormat_isr(hDisplay);
    }
    else
    {
        hDisplay->stCurInfo.stDirty.stBits.bTiming = BVDC_P_CLEAN;
    }
#else
    hDisplay->stCurInfo.stDirty.stBits.bTiming = BVDC_P_CLEAN;
#endif

#if DCS_SUPPORT
    BVDC_P_DCS_StateFault_isr (hDisplay);
#endif

    return;
}

/************ New ACP format event handlers **********/

static BERR_Code BVDC_P_Display_Validate_Acp
    ( BVDC_Display_Handle              hDisplay )
{
    BERR_Code err;

    if(hDisplay->bAnlgEnable ||   /* if analog master */
       (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
       (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
    {
        if ((err = BVDC_P_ValidateMacrovision (hDisplay)) != BERR_SUCCESS)
        {
            return BERR_TRACE (err);
        }
    }

    return BERR_SUCCESS;
}

static void BVDC_P_Display_Copy_Acp_isr
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo* pCurInfo = &hDisplay->stCurInfo;
    BVDC_P_DisplayInfo* pNewInfo = &hDisplay->stNewInfo;

#if DCS_SUPPORT
    BDBG_MSG(("dsp[%d] DCS change %d -> %d",
        hDisplay->eId, pCurInfo->eDcsMode, pNewInfo->eDcsMode));
#else
    BDBG_MSG(("dsp[%d] Macrovision change %d -> %d",
        hDisplay->eId, pCurInfo->eMacrovisionType, pNewInfo->eMacrovisionType));
#endif

    /* Copy protection state */
    pCurInfo->eMacrovisionType = pNewInfo->eMacrovisionType;
    pCurInfo->stN0Bits = pNewInfo->stN0Bits;
#if DCS_SUPPORT
    pCurInfo->eDcsMode = pNewInfo->eDcsMode;
#endif
}

static void BVDC_P_Display_Apply_Acp_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BVDC_P_DisplayInfo *pCurInfo = &hDisplay->stCurInfo;

#if DCS_SUPPORT /** { **/

    BSTD_UNUSED(eFieldPolarity);
    BSTD_UNUSED(pList);

    BVDC_P_DCS_StateFault_isr (hDisplay);

#else /** } !DCS_SUPPORT { **/

    BSTD_UNUSED(eFieldPolarity);

    if(!hDisplay->stCurInfo.bHdmiRmd)
    {
        /* Program IT_?_TG_CONFIG register */
        BVDC_P_Display_StartAnlgCtrler_isr (hDisplay, pList);

        if(hDisplay->bAnlgEnable ||   /* if analog master */
           (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
           (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
        {
            /*
             * 3) Program the modules to the new format
             *    Note: Format changing involves reprogramming all the settings
             *          in the current info. We will call each event handler
             *          and clears all the dirty bits.
             */
            BVDC_P_Vec_Build_IT_isr(
                hDisplay, &hDisplay->stAnlgChan_0, false, pList);

            /* Update VF settings */
            if (hDisplay->stAnlgChan_0.bEnable)
            {
                BVDC_P_Vec_Build_VF_isr(hDisplay, &hDisplay->stAnlgChan_0,
                    pCurInfo->eAnlg_0_OutputColorSpace, pList);
            }

            if (hDisplay->stAnlgChan_1.bEnable)
            {
                BVDC_P_Vec_Build_VF_isr(hDisplay, &hDisplay->stAnlgChan_1,
                    pCurInfo->eAnlg_1_OutputColorSpace, pList);
            }
        }
    }

#endif /** } !DCS_SUPPORT **/

    pCurInfo->stDirty.stBits.bAcp = BVDC_P_CLEAN;
}

/**************** Setting time base event hanlders **************/
static void BVDC_P_Display_Copy_TimeBase_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    hDisplay->stCurInfo.eTimeBase = hDisplay->stNewInfo.eTimeBase;

    return;
}

static void BVDC_P_Display_Apply_TimeBase_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BAVC_Timebase eTimeBase = hDisplay->stCurInfo.eTimeBase;
    BSTD_UNUSED(eFieldPolarity);

    if((hDisplay->bAnlgEnable && !hDisplay->stCurInfo.bHdmiRmd) ||   /* if analog master */
       (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
       (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
    {
        /* RM pairs with IT */
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_RM_0_CONTROL + hDisplay->stAnlgChan_0.ulRmRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(RM_0_CONTROL, TRACKING_RANGE, BVDC_P_TRACKING_RANGE_DEFAULT) |
            BCHP_FIELD_DATA(RM_0_CONTROL, RESET,          0         ) |
            BCHP_FIELD_DATA(RM_0_CONTROL, INT_GAIN,       BVDC_P_RM_INTEGRAL_GAIN ) |
            BCHP_FIELD_DATA(RM_0_CONTROL, DIRECT_GAIN,    BVDC_P_RM_DIRECT_GAIN   ) |
            BCHP_FIELD_DATA(RM_0_CONTROL, DITHER,         0         ) |
            BCHP_FIELD_DATA(RM_0_CONTROL, FREE_RUN,       0         ) |
            BCHP_FIELD_DATA(RM_0_CONTROL, TIMEBASE,       eTimeBase );
    }

#if BVDC_P_SUPPORT_DTG_RMD
    if(hDisplay->stCurInfo.bHdmiRmd)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_DVI_DTG_RM_0_CONTROL + hDisplay->stDviChan.ulDviRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(RM_0_CONTROL, TRACKING_RANGE, BVDC_P_TRACKING_RANGE_DEFAULT) |
            BCHP_FIELD_DATA(RM_0_CONTROL, RESET,          0         ) |
            BCHP_FIELD_DATA(RM_0_CONTROL, INT_GAIN,       BVDC_P_RM_INTEGRAL_GAIN ) |
            BCHP_FIELD_DATA(RM_0_CONTROL, DIRECT_GAIN,    BVDC_P_RM_DIRECT_GAIN   ) |
            BCHP_FIELD_DATA(RM_0_CONTROL, DITHER,         0         ) |
            BCHP_FIELD_DATA(RM_0_CONTROL, FREE_RUN,       0         ) |
            BCHP_FIELD_DATA(RM_0_CONTROL, TIMEBASE,       eTimeBase );
    }
#endif

    if(hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bEnableHdmi)
    {
        /* HDMI_RM_CONTROL (RW) */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_RM_CONTROL, TRACKING_RANGE  ) |
              BCHP_MASK(HDMI_RM_CONTROL, RESET           ) |
              BCHP_MASK(HDMI_RM_CONTROL, INT_GAIN        ) |
              BCHP_MASK(HDMI_RM_CONTROL, DIRECT_GAIN     ) |
              BCHP_MASK(HDMI_RM_CONTROL, DITHER          ) |
              BCHP_MASK(HDMI_RM_CONTROL, FREE_RUN        ) |
              BCHP_MASK(HDMI_RM_CONTROL, TIMEBASE        )),
            BCHP_FIELD_DATA(HDMI_RM_CONTROL, TRACKING_RANGE, BVDC_P_TRACKING_RANGE_DEFAULT) |
            BCHP_FIELD_ENUM(HDMI_RM_CONTROL, RESET         , RESET_OFF ) |
            BCHP_FIELD_DATA(HDMI_RM_CONTROL, INT_GAIN      , BVDC_P_RM_INTEGRAL_GAIN ) |
            BCHP_FIELD_DATA(HDMI_RM_CONTROL, DIRECT_GAIN   , BVDC_P_RM_DIRECT_GAIN   ) |
            BCHP_FIELD_ENUM(HDMI_RM_CONTROL, DITHER        , DITHER_OFF) |
            BCHP_FIELD_ENUM(HDMI_RM_CONTROL, FREE_RUN      , TIMEBASE  ) |
            BCHP_FIELD_DATA(HDMI_RM_CONTROL, TIMEBASE      , eTimeBase ),
            BCHP_HDMI_RM_CONTROL + hDisplay->stDviChan.ulDvpRegOffset  );
    }

#if BVDC_P_SUPPORT_STG /* RT mode need to update STG timebase here; NRT mode will update per vsync along with mailbox; */
    if(hDisplay->stCurInfo.bEnableStg && !hDisplay->stCurInfo.bStgNonRealTime)
    {
        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_STG_0_CONTROL + hDisplay->ulStgRegOffset);
        *pList->pulCurrent++ = /* disable both timer and EOP triggers */
#if (BVDC_P_SUPPORT_STG_VER > BVDC_P_STG_VER_1)
            BCHP_FIELD_ENUM(VIDEO_ENC_STG_0_CONTROL, TRIG_MODE, AND) |
#endif
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, TIMEBASE_SEL, hDisplay->stCurInfo.eTimeBase) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, SLAVE_MODE, !BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg)) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, HOST_ARM_ENABLE,   0) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, TIMER_TRIG_ENABLE, 1) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, EOP_TRIG_ENABLE,   0) |
            BCHP_FIELD_DATA(VIDEO_ENC_STG_0_CONTROL, SCAN_MODE, hDisplay->stCurInfo.pFmtInfo->bInterlaced);
    }
#endif

    hDisplay->bRateManagerUpdated = true;

    hDisplay->stCurInfo.stDirty.stBits.bTimeBase = BVDC_P_CLEAN;

    return;
}


/***************** Setting DAC event hanlders *********************/
static void BVDC_P_Display_Copy_DAC_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo *pCurInfo, *pNewInfo;
    int i;

    pCurInfo = &hDisplay->stCurInfo;
    pNewInfo = &hDisplay->stNewInfo;

    for(i = 0; i < BVDC_P_MAX_DACS; i++)
    {
        pCurInfo->aDacOutput[i] = pNewInfo->aDacOutput[i];
    }

    pCurInfo->bCvbs   = pNewInfo->bCvbs;
    pCurInfo->bSvideo = pNewInfo->bSvideo;
    pCurInfo->bHsync  = pNewInfo->bHsync;
    pCurInfo->bRgb    = pNewInfo->bRgb;
    pCurInfo->bYPrPb  = pNewInfo->bYPrPb;
    pCurInfo->ulAnlgChan0Mask = pNewInfo->ulAnlgChan0Mask;
    pCurInfo->ulAnlgChan1Mask = pNewInfo->ulAnlgChan1Mask;
    pCurInfo->eAnlg_0_OutputColorSpace = pNewInfo->eAnlg_0_OutputColorSpace;
    pCurInfo->eAnlg_1_OutputColorSpace = pNewInfo->eAnlg_1_OutputColorSpace;
    pCurInfo->bMultiRateAllow = pNewInfo->bMultiRateAllow;

    return;
}

static void BVDC_P_Display_ProgramDac_isr
    ( BVDC_DacOutput                   eDacOutput,
      uint32_t                         ulDac,
      uint32_t                         ulDacSrc,
      BVDC_P_ListInfo                 *pList )
{
#if (BVDC_P_MAX_DACS > 1)
    uint32_t ulOffset = (BCHP_MISC_DAC_1_CFG - BCHP_MISC_DAC_0_CFG) * ulDac;
#else
    uint32_t ulOffset = 0;
#endif
    uint32_t ulDacSel = 0;

    switch(eDacOutput)
    {
        case BVDC_DacOutput_eY:
        case BVDC_DacOutput_eGreen:
        case BVDC_DacOutput_eGreen_NoSync:
        case BVDC_DacOutput_eSVideo_Luma:
            ulDacSel = 0 + ulDacSrc;
            break;
        case BVDC_DacOutput_eComposite:
        case BVDC_DacOutput_ePb:
        case BVDC_DacOutput_eBlue:
            ulDacSel = 1 + ulDacSrc;
            break;
        case BVDC_DacOutput_ePr:
        case BVDC_DacOutput_eRed:
        case BVDC_DacOutput_eSVideo_Chroma:
            ulDacSel = 2 + ulDacSrc;
            break;
        case BVDC_DacOutput_eUnused:
            ulDacSel = BCHP_MISC_DAC_0_CFG_SEL_CONST;
            break;
#ifdef BCHP_MISC_DAC_0_CFG_SEL_GRPD_0_CVBS
        case BVDC_DacOutput_eFilteredCvbs:
            ulDacSel = BCHP_MISC_DAC_0_CFG_SEL_GRPD_0_CVBS;
            break;
#endif
#ifdef BCHP_MISC_DAC_0_CFG_SEL_HDSRC_0_HSYNC
        case BVDC_DacOutput_eHsync:
            ulDacSel = BCHP_MISC_DAC_0_CFG_SEL_HDSRC_0_HSYNC;
            break;
#endif
        default:
            BDBG_WRN(("eDacOutput=%d not supported", eDacOutput));
            BDBG_ASSERT(0);
            break;
    }

    BDBG_MSG(("     DAC %d Src %d Chan %d, output %d, ulOffset = %d, power %s",
        ulDac, ulDacSrc, ulDacSel - ulDacSrc, eDacOutput, ulOffset,
        (eDacOutput == BVDC_DacOutput_eUnused)? "off" : "on"));

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_0_CFG + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MISC_DAC_0_CFG, CONST,        0) |
        BCHP_FIELD_ENUM(MISC_DAC_0_CFG, SINC,        ON) |
        BCHP_FIELD_DATA(MISC_DAC_0_CFG, DELAY,        0) |
        BCHP_FIELD_DATA(MISC_DAC_0_CFG, SEL,   ulDacSel);

#if (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_11)
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_0_CTRL + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MISC_DAC_0_CTRL, FORMAT,       0 ) | /* optimmal */
        BCHP_FIELD_DATA(MISC_DAC_0_CTRL, RSTB,         1 ) | /* optimmal */
        BCHP_FIELD_DATA(MISC_DAC_0_CTRL, PWRDN,
            (eDacOutput == BVDC_DacOutput_eUnused) ? 1 : 0);
#elif (BVDC_P_SUPPORT_TDAC_VER < BVDC_P_SUPPORT_TDAC_VER_9)
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_0_CTRL + ulOffset);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MISC_DAC_0_CTRL, PWRUP_BAIS,
            (eDacOutput == BVDC_DacOutput_eUnused) ? 0 : 1) |
        BCHP_FIELD_DATA(MISC_DAC_0_CTRL, PWRDN,
            (eDacOutput == BVDC_DacOutput_eUnused) ? 1 : 0);
#endif

    BSTD_UNUSED(ulDac);
    return;
}

static uint32_t BVDC_P_Dislay_FindDacSrc_isr
    ( BVDC_Display_Handle              hDisplay,
      uint32_t                         ulDacMask )
{
    BVDC_P_DisplayAnlgChan *pChan = NULL;
    uint32_t ulDacSrc  = UINT32_C(-1);

    if((hDisplay->stCurInfo.ulAnlgChan0Mask & ulDacMask) == ulDacMask)
    {
        pChan = &hDisplay->stAnlgChan_0;
    }
    else if((hDisplay->stCurInfo.ulAnlgChan1Mask & ulDacMask) == ulDacMask)
    {
        pChan = &hDisplay->stAnlgChan_1;
    }
    else
    {
        return ulDacSrc;
    }

    if(!pChan->bEnable)
    {
        ulDacSrc = UINT32_C(-1);
    }
#if (BVDC_P_NUM_SHARED_HDSRC > 0)
    else if(pChan->ulHdsrc != BVDC_P_HW_ID_INVALID)
    {
        ulDacSrc = BCHP_MISC_DAC_0_CFG_SEL_HDSRC_0_CH0;
    }
#endif
    else
    {
        ulDacSrc = BCHP_MISC_DAC_0_CFG_SEL_SDSRC_0_CH0 + 3 * pChan->ulSdsrc;
    }

    return ulDacSrc;
}

static void BVDC_P_Display_Apply_DAC_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    uint32_t i, ulCurDac, ulDacSrc, ulDacMask = 0, ulDacGroupMask;
    BVDC_P_DisplayInfo *pCurInfo = &hDisplay->stCurInfo;
    uint32_t *pulDacBGAdj;
    const uint32_t *pulDacGrouping;
    BVDC_DacOutput *aDacOutput;
    bool bPowerupBG, bPowerdownBG, bValidBG;
#if (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_9)
    int iDac;
#endif

    BDBG_MSG(("Display%d programming Dac...", hDisplay->eId));

    pulDacBGAdj = hDisplay->hVdc->stSettings.aulDacBandGapAdjust;
    pulDacGrouping = hDisplay->hVdc->aulDacGrouping;
    aDacOutput = hDisplay->hVdc->aDacOutput;

    if(hDisplay->bAnlgEnable ||   /* if analog master */
       (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
       (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
    {
        for(i = 0; i < BVDC_P_MAX_DACS; i++)
        {
            switch(aDacOutput[i])
            {
                case BVDC_DacOutput_eFilteredCvbs:
                    ulDacMask = BVDC_P_Dac_Mask_Cvbs;
                    break;
                case BVDC_DacOutput_eComposite:
                    ulDacMask = BVDC_P_Dac_Mask_Cvbs;
                    break;
                case BVDC_DacOutput_eSVideo_Luma:
                case BVDC_DacOutput_eSVideo_Chroma:
                    ulDacMask = BVDC_P_Dac_Mask_Svideo;
                    break;
                case BVDC_DacOutput_eGreen:
                case BVDC_DacOutput_eGreen_NoSync:
                case BVDC_DacOutput_eRed:
                case BVDC_DacOutput_eBlue:
                    ulDacMask = BVDC_P_Dac_Mask_RGB;
                    break;
                case BVDC_DacOutput_eY:
                case BVDC_DacOutput_ePr:
                case BVDC_DacOutput_ePb:
                    ulDacMask = BVDC_P_Dac_Mask_YPrPb;
                    break;
                case BVDC_DacOutput_eUnused:
                default:
                    ulDacMask = 0;
                    break;
            }

            if (aDacOutput[i] != BVDC_DacOutput_eUnused)
            {
                /* Only attempt to find DAC source for those DACs belong to this display */
                if(hDisplay->hVdc->aDacDisplay[i] == hDisplay->eId)
                {
                    ulDacSrc = BVDC_P_Dislay_FindDacSrc_isr(hDisplay, ulDacMask);
                }
                else
                    ulDacSrc = UINT32_C(-1);
                if(!hDisplay->hVdc->bDacDetectionEnable)
                {
                    /* if cable detect disable, callback return status unknown */
                    hDisplay->hVdc->aeDacStatus[i] = BVDC_DacConnectionState_eUnknown;
                }
            }
            else
            {
                ulDacSrc = 0;
                if(!hDisplay->hVdc->bDacDetectionEnable)
                {
                    /* if cable detect disable, callback return status unknown */
                    hDisplay->hVdc->aeDacStatus[i] = BVDC_DacConnectionState_eUnknown;
                }
            }

            if(ulDacSrc != UINT32_C(-1))
            {
                BVDC_P_Display_ProgramDac_isr(aDacOutput[i], i, ulDacSrc, pList);
            }
        }

        /* Power up bandgap control if any DAC in the group is in use. */
        /* Power down bandgap control if all the DACs in the group are not in use. */
        for(ulDacGroupMask = 1; ulDacGroupMask < (BVDC_P_MAX_DACS + 1); ulDacGroupMask++)
        {
            bPowerupBG = false;
            bPowerdownBG = true;
            bValidBG     = false;
            for (ulCurDac = 0; ulCurDac < BVDC_P_MAX_DACS; ulCurDac++)
            {
                /*BDBG_MSG(("ulDacGroupMask = %d, ulCurDac = %d, pualDacGrouping[%d] = %d, adacOutput[%d] = %d",
                    ulDacGroupMask, ulCurDac, ulCurDac, pulDacGrouping[ulCurDac], ulCurDac, aDacOutput[ulCurDac]));*/

                if ((pulDacGrouping[ulCurDac] == ulDacGroupMask) && (aDacOutput[ulCurDac] != BVDC_DacOutput_eUnused))
                {
                    bPowerupBG = true;
                    bValidBG = true;
                    bPowerdownBG = false;
                    break;
                }
                else if (pulDacGrouping[ulCurDac] == ulDacGroupMask)
                {
                    bValidBG = true;
                }
            }

            if (bPowerdownBG && bValidBG)
            {
                BDBG_MSG(("power down BG %d", ulDacGroupMask - 1));

#if (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_11)

                /* Power down the cable detect ADC */
                if(hDisplay->hVdc->bDacDetectionEnable)
                {
                    BVDC_P_RD_MOD_WR_RUL (
                        pList->pulCurrent,
                        ~BCHP_MASK (MISC_ADC_CTRL_0, PWRDN),
                        BCHP_FIELD_DATA (MISC_ADC_CTRL_0, PWRDN, 1),
                        BCHP_MISC_ADC_CTRL_0);
                }

                /* Power down the video DACs */
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_INST_BIAS_CTRL_0);
                *pList->pulCurrent++ =
                    BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, PWRDN, 1);
#elif (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_9)
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_BIAS_CTRL_0);
                *pList->pulCurrent++ =
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, PWRDN, 1);
#else
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_BG_CTRL_0 + (ulDacGroupMask - 1) * 4);
                *pList->pulCurrent++ =
                    BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_0, PWRDN,        PWRDN     ) |
                    BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_0, PWRDN_REFAMP, POWER_DOWN);
                BDBG_MSG(("power down BG %d", ulDacGroupMask - 1));
#endif
            }

            if (bPowerupBG)
            {
#if (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_9)
                uint32_t ulHDDacSyncSrc = 0;
                uint32_t ulSvideoDacSyncSrc = 0;
                uint32_t ulCvbsDacSyncSrc = 0;

                for (iDac = 0 ; iDac < BVDC_P_MAX_DACS ; ++iDac)
                {
                    if((aDacOutput[iDac] == BVDC_DacOutput_eY    ) ||
                       (aDacOutput[iDac] == BVDC_DacOutput_eGreen) )
                    {
                        ulHDDacSyncSrc = iDac;
                        break;
                    }
                }

                for (iDac = 0 ; iDac < BVDC_P_MAX_DACS ; ++iDac)
                {
                    if(aDacOutput[iDac] == BVDC_DacOutput_eSVideo_Luma)
                    {
                        ulSvideoDacSyncSrc = iDac;
                        break;
                    }
                }

                for (iDac = 0 ; iDac < BVDC_P_MAX_DACS ; ++iDac)
                {
                    if(aDacOutput[iDac] == BVDC_DacOutput_eComposite)
                    {
                        ulCvbsDacSyncSrc = iDac;
                        break;
                    }
                }

                for (iDac = 0 ; iDac < BVDC_P_MAX_DACS ; ++iDac)
                {
                    if(aDacOutput[iDac] == BVDC_DacOutput_eGreen_NoSync)
                    {
                        ulHDDacSyncSrc = ulCvbsDacSyncSrc;
                        break;
                    }
                }

#if !BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND
                if(hDisplay->hVdc->bDacDetectionEnable)
                {
                    /* Toggling plug in and plug out in BCHP_MISC_DAC_DETECT_EN_0 */
                    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_DETECT_EN_0);
                    *pList->pulCurrent++ =
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, USE_STEP_DLY,   0) |
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, SW_CALIBRATE,   0) |
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, CALIBRATE,      1) |
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, PLUGOUT_DETECT, 0) |
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, PLUGIN_DETECT,  0) |
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, AUTO_DETECT,    1);
                }
#endif

                for (i = 0; i < BVDC_P_MAX_DACS; i++)
                {
                    hDisplay->hVdc->aulDacScaleSel[i] =
#if (BVDC_P_SUPPORT_TDAC_VER < BVDC_P_SUPPORT_TDAC_VER_13) && (!BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND)
                        (hDisplay->hVdc->bCalibrated == 0) ? 3 :
#endif
                        (aDacOutput[i] == BVDC_DacOutput_eComposite) ? 3 :
                        (aDacOutput[i] == BVDC_DacOutput_eY &&
                         pCurInfo->eMacrovisionType != BVDC_MacrovisionType_eNoProtection) ? 2 :
                        (aDacOutput[i] == BVDC_DacOutput_eY     ||
                         aDacOutput[i] == BVDC_DacOutput_ePb    ||
                         aDacOutput[i] == BVDC_DacOutput_ePr    ||
                         aDacOutput[i] == BVDC_DacOutput_eGreen ||
                         aDacOutput[i] == BVDC_DacOutput_eGreen_NoSync ||
                         aDacOutput[i] == BVDC_DacOutput_eBlue  ||
                         aDacOutput[i] == BVDC_DacOutput_eRed) ? 1 : 0;

                    hDisplay->hVdc->aulDacSyncSource[i] =
                        (aDacOutput[i] == BVDC_DacOutput_ePb   ||
                         aDacOutput[i] == BVDC_DacOutput_ePr   ||
                         aDacOutput[i] == BVDC_DacOutput_eY ||
                         aDacOutput[i] == BVDC_DacOutput_eGreen ||
                         aDacOutput[i] == BVDC_DacOutput_eGreen_NoSync ||
                         aDacOutput[i] == BVDC_DacOutput_eBlue ||
                         aDacOutput[i] == BVDC_DacOutput_eRed) ? ulHDDacSyncSrc :
                        (aDacOutput[i] == BVDC_DacOutput_eSVideo_Luma ||
                         aDacOutput[i] == BVDC_DacOutput_eSVideo_Chroma) ? ulSvideoDacSyncSrc :
                        (aDacOutput[i] == BVDC_DacOutput_eComposite) ? ulCvbsDacSyncSrc : i;

                    hDisplay->hVdc->aulDacSyncEn[i] =
                        (aDacOutput[i] == BVDC_DacOutput_eY ||
                         aDacOutput[i] == BVDC_DacOutput_eGreen ||
                         aDacOutput[i] == BVDC_DacOutput_eComposite) ? 1 : 0;
                }

#if (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_9) || (BVDC_P_SUPPORT_TDAC_VER == BVDC_P_SUPPORT_TDAC_VER_10)
                /* BCHP_MISC_DAC_BIAS_CTRL_0 */
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_BIAS_CTRL_0);
                *pList->pulCurrent++ =
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC3_SCALE_LP,  0             ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC3_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[3]) |
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC2_SCALE_LP,  0             ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC2_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[2]) |
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC1_SCALE_LP,  0             ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC1_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[1]) |
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC0_SCALE_LP,  0             ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, DAC0_SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[0]) |
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, GAIN_ADJ,       pulDacBGAdj[ulCurDac]) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, GAIN_OVERRIDE,  0             ) |
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, REG_ADJ,        4             ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, BG_ADJ,         5             ) |
                    BCHP_FIELD_DATA(MISC_DAC_BIAS_CTRL_0, PWRDN,          0             );

                /* BCHP_MISC_DAC_CTRL_0 */
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_CTRL_0);
                *pList->pulCurrent++ =
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC3_RSTB,         1   ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC2_RSTB,         1   ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC1_RSTB,         1   ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC0_RSTB,         1   ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, TBD,               0   ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, PRBS_LENGTH,       0   ) | /* optimmal */
                    BCHP_FIELD_ENUM(MISC_DAC_CTRL_0, PRBS_MODE,         PRBS) | /* optimmal */
                    BCHP_FIELD_ENUM(MISC_DAC_CTRL_0, DISABLE_PRBS,      EN  ) | /* optimmal */
                    BCHP_FIELD_ENUM(MISC_DAC_CTRL_0, DISABLE_SCRAMBLER, EN  ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC3_FORMAT,       0   ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC2_FORMAT,       0   ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC1_FORMAT,       0   ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC0_FORMAT,       0   ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC3_PWRDN,
                        (aDacOutput[3] == BVDC_DacOutput_eUnused) ? 1 : 0) |
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC2_PWRDN,
                        (aDacOutput[2] == BVDC_DacOutput_eUnused) ? 1 : 0) |
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC1_PWRDN,
                        (aDacOutput[1] == BVDC_DacOutput_eUnused) ? 1 : 0) |
                    BCHP_FIELD_DATA(MISC_DAC_CTRL_0, DAC0_PWRDN,
                        (aDacOutput[0] == BVDC_DacOutput_eUnused) ? 1 : 0);

#elif (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_11)
                /* BCHP_MISC_DAC_INST_BIAS_CTRL_0 */
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_INST_BIAS_CTRL_0);
                *pList->pulCurrent++ =
                    BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, GAIN_ADJ,       pulDacBGAdj[ulCurDac]) |
                    BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, GAIN_OVERRIDE,  0             ) |
                    BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, REG_ADJ,        4             ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, BG_ADJ,         5             ) |
                BCHP_FIELD_DATA(MISC_DAC_INST_BIAS_CTRL_0, PWRDN,          0             );

#if (BVDC_P_SUPPORT_TDAC_VER <= BVDC_P_SUPPORT_TDAC_VER_12)
                /* BCHP_MISC_DAC_INST_PRBS_CTRL_0 */
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_INST_PRBS_CTRL_0);
                *pList->pulCurrent++ =
                    BCHP_FIELD_DATA(MISC_DAC_INST_PRBS_CTRL_0, TBD,               0   ) | /* optimmal */
                    BCHP_FIELD_DATA(MISC_DAC_INST_PRBS_CTRL_0, PRBS_LENGTH,       0   ) | /* optimmal */
                    BCHP_FIELD_ENUM(MISC_DAC_INST_PRBS_CTRL_0, PRBS_MODE,         PRBS) | /* optimmal */
                    BCHP_FIELD_ENUM(MISC_DAC_INST_PRBS_CTRL_0, DISABLE_PRBS,      EN  ) | /* optimmal */
                    BCHP_FIELD_ENUM(MISC_DAC_INST_PRBS_CTRL_0, DISABLE_SCRAMBLER, EN  );  /* optimmal */
#endif
                for (i = 0; i < BVDC_P_MAX_DACS; i++)
                {
#if (BVDC_P_MAX_DACS > 1)
                    uint32_t ulOffset = (BCHP_MISC_DAC_1_SCALE_CTRL - BCHP_MISC_DAC_0_SCALE_CTRL) * i;
#else
                    uint32_t ulOffset = 0;
#endif
                    /* BCHP_MISC_DAC_x_SCALE_CTRL  */
                    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_0_SCALE_CTRL + ulOffset);
                    *pList->pulCurrent++ =
                        BCHP_FIELD_DATA(MISC_DAC_0_SCALE_CTRL, SCALE_LP,  0             ) | /* optimmal */
                        BCHP_FIELD_DATA(MISC_DAC_0_SCALE_CTRL, SCALE_SEL, hDisplay->hVdc->aulDacScaleSel[i]);
                }
#endif

#if (BVDC_P_MAX_DACS > 1)
                /* if only 1 DAC, use RESET val = DAC0 */
                /* BCHP_MISC_DAC_DETECT_SYNC_CTRL_0 */
                hDisplay->hVdc->ulDacDetectSyncCtrl =
#ifdef BCHP_MISC_DAC_DETECT_SYNC_CTRL_0_DAC3_SYNC_SOURCE_DEFAULT
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[3]) |
#endif
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[2]) |
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[1]) |
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[0]) |
#ifdef BCHP_MISC_DAC_DETECT_SYNC_CTRL_0_DAC3_SYNC_EN_DEFAULT
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_EN, hDisplay->hVdc->aulDacSyncEn[3]        ) |
#endif
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_EN, hDisplay->hVdc->aulDacSyncEn[2]        ) |
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_EN, hDisplay->hVdc->aulDacSyncEn[1]        ) |
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_EN, hDisplay->hVdc->aulDacSyncEn[0]        );
#else
                hDisplay->hVdc->ulDacDetectSyncCtrl =
#ifdef BCHP_MISC_DAC_DETECT_SYNC_CTRL_0_DAC3_SYNC_SOURCE_DEFAULT
                    BCHP_FIELD_ENUM(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_SOURCE, DAC3) |
#endif
#ifdef BCHP_MISC_DAC_DETECT_SYNC_CTRL_0_DAC1_SYNC_SOURCE_DEFAULT
                    BCHP_FIELD_ENUM(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_SOURCE, DAC2) |
                    BCHP_FIELD_ENUM(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_SOURCE, DAC1) |
#endif
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_SOURCE, hDisplay->hVdc->aulDacSyncSource[0]) |
#ifdef BCHP_MISC_DAC_DETECT_SYNC_CTRL_0_DAC3_SYNC_EN_DEFAULT
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC3_SYNC_EN, 0        ) |
#endif
#ifdef BCHP_MISC_DAC_DETECT_SYNC_CTRL_0_DAC1_SYNC_EN_DEFAULT
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC2_SYNC_EN, 0        ) |
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC1_SYNC_EN, 0        ) |
#endif
                    BCHP_FIELD_DATA(MISC_DAC_DETECT_SYNC_CTRL_0, DAC0_SYNC_EN, hDisplay->hVdc->aulDacSyncEn[0]        );
#endif /* (BVDC_P_MAX_DACS > 1) */
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_DETECT_SYNC_CTRL_0);
                *pList->pulCurrent++ = hDisplay->hVdc->ulDacDetectSyncCtrl;

#if !BVDC_P_VEC_CABLE_DETECT_SW_WORKAROUND
                if(hDisplay->hVdc->bDacDetectionEnable)
                {
                    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_DETECT_EN_0);
                    *pList->pulCurrent++ =
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, USE_STEP_DLY,   0) |
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, SW_CALIBRATE,   0) |
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, CALIBRATE,      1) |
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, PLUGOUT_DETECT, 1) |
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, PLUGIN_DETECT,  1) |
                        BCHP_FIELD_DATA(MISC_DAC_DETECT_EN_0, AUTO_DETECT,    1);
                }
#endif
#else
                *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
                *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_DAC_BG_CTRL_0 + (ulDacGroupMask - 1) * 4);
                *pList->pulCurrent++ =
                    BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_0, PWRDN, PWRUP            ) |
                    BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_0, CORE_ADJ, FOUR          ) |
                    BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_0, BANDGAP_BYP, BANDGAP    ) |
                    BCHP_FIELD_DATA(MISC_DAC_BG_CTRL_0, IREF_ADJ, pulDacBGAdj[ulCurDac]) |
                    BCHP_FIELD_ENUM(MISC_DAC_BG_CTRL_0, PWRDN_REFAMP, POWER_UP  );

                BSTD_UNUSED(pCurInfo);
#endif
                BDBG_MSG(("power up BG %d", ulDacGroupMask - 1));

#if (BVDC_P_SUPPORT_TDAC_VER >= BVDC_P_SUPPORT_TDAC_VER_11)
                /* Power up the cable detect ADC too */
                if(hDisplay->hVdc->bDacDetectionEnable)
                {
                    BVDC_P_RD_MOD_WR_RUL (
                        pList->pulCurrent,
                        ~BCHP_MASK (MISC_ADC_CTRL_0, PWRDN),
                        BCHP_FIELD_DATA (MISC_ADC_CTRL_0, PWRDN, 0),
                        BCHP_MISC_ADC_CTRL_0);
                }
#endif
            }
            /*BDBG_MSG(("end of mask %d", ulDacGroupMask));*/
        }
    }

    if (hDisplay->stAnlgChan_0.bEnable && hDisplay->bDacProgAlone)
    {
        BVDC_P_ProgramAnalogChan_isr(hDisplay, &hDisplay->stAnlgChan_0, pList);
    }
    if (hDisplay->stAnlgChan_1.bEnable && hDisplay->bDacProgAlone)
    {
        BVDC_P_ProgramAnalogChan_isr(hDisplay, &hDisplay->stAnlgChan_1, pList);
    }

    /* Start micro controllers. This has to be after
     * the VEC pipeline is established.  */
    BVDC_P_Display_StartMicroCtrler_isr(hDisplay, pList);

#if DCS_SUPPORT
    BVDC_P_DCS_StateFault_isr (hDisplay);
#endif

    hDisplay->stCurInfo.stDirty.stBits.bDacSetting = BVDC_P_CLEAN;

    BSTD_UNUSED(eFieldPolarity);

    return;
}


/***************** Setting callback hanlders *********************/
static void BVDC_P_Display_Copy_Callback_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    hDisplay->stCurInfo.stCallbackSettings = hDisplay->stNewInfo.stCallbackSettings;

    return;
}

static void BVDC_P_Display_Apply_Callback_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BSTD_UNUSED(pList);
    BSTD_UNUSED(eFieldPolarity);

    hDisplay->stCurInfo.stDirty.stBits.bCallback = BVDC_P_CLEAN;
    hDisplay->bCallbackInit = true;

    return;
}

static void BVDC_P_Display_Copy_CallbackFunc_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo *pCurInfo, *pNewInfo;

    pCurInfo = &hDisplay->stCurInfo;
    pNewInfo = &hDisplay->stNewInfo;

    pCurInfo->pfGenericCallback = pNewInfo->pfGenericCallback;
    pCurInfo->pvGenericParm1    = pNewInfo->pvGenericParm1;
    pCurInfo->iGenericParm2      = pNewInfo->iGenericParm2;

    return;
}

static void BVDC_P_Display_Apply_CallbackFunc_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BSTD_UNUSED(pList);
    BSTD_UNUSED(eFieldPolarity);

    hDisplay->stCurInfo.stDirty.stBits.bCallbackFunc = BVDC_P_CLEAN;

    return;
}


/****************** Trimming width event handlers ******************/
static void BVDC_P_Display_Apply_TrimmingWidth_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    const uint32_t         *pRamTbl = NULL;
    BVDC_P_DisplayInfo     *pCurInfo = &hDisplay->stCurInfo;
    BVDC_P_DisplayAnlgChan *pstChan = NULL;

    BSTD_UNUSED(pList);
    BSTD_UNUSED(eFieldPolarity);

    if((hDisplay->bAnlgEnable && !hDisplay->stCurInfo.bHdmiRmd) ||   /* if analog master */
       (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
       (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
    {

        /* No VEC reset when switching between 704 and 720. Only load
         * new micro-code and update some register settings. This can
         * avoid TV resync.
         */
        BVDC_P_Display_Build_ItBvbSize_isr(hDisplay, pList, hDisplay->stAnlgChan_0.ulItRegOffset);

        /* Update VF shaper settings */
        pstChan = &hDisplay->stAnlgChan_0;
        if (pstChan->bEnable)
        {
            BVDC_P_Vec_Build_VF_isr(
                hDisplay, pstChan, pCurInfo->eAnlg_0_OutputColorSpace, pList);
        }
        pstChan = &hDisplay->stAnlgChan_1;
        if (pstChan->bEnable)
        {
            BVDC_P_Vec_Build_VF_isr(
                hDisplay, pstChan, pCurInfo->eAnlg_1_OutputColorSpace, pList);
        }

        /* Load new micro-code */
        pRamTbl   = BVDC_P_GetRamTable_isr(pCurInfo, hDisplay->bArib480p);
        BDBG_ASSERT (pRamTbl);
        BVDC_P_Vec_Build_ItMc_isr(
            pList, pRamTbl, hDisplay->stAnlgChan_0.ulItRegOffset);

        /* SM may need reprogramming because init_phase */
        pstChan = &hDisplay->stAnlgChan_0;
        if (pstChan->bEnable)
        {
            BVDC_P_Output eOutputCS =
                hDisplay->stCurInfo.eAnlg_0_OutputColorSpace;
            if( eOutputCS != BVDC_P_Output_eUnknown )
            {
                if (pstChan->ulSdsrc != BVDC_P_HW_ID_INVALID)
                {
                    const uint32_t* pTable =
                        BVDC_P_GetSmTable_isr(pCurInfo, eOutputCS);
                    BVDC_P_Vec_Build_SM_isr(
                        pCurInfo->pFmtInfo->eVideoFmt, eOutputCS, pTable,
                        pstChan, pList);
                }
            }
        }
        pstChan = &hDisplay->stAnlgChan_1;
        if (pstChan->bEnable)
        {
            BVDC_P_Output eOutputCS =
                hDisplay->stCurInfo.eAnlg_1_OutputColorSpace;
            if( eOutputCS != BVDC_P_Output_eUnknown )
            {
                if (pstChan->ulSdsrc != BVDC_P_HW_ID_INVALID)
                {
                    const uint32_t* pTable =
                        BVDC_P_GetSmTable_isr(pCurInfo, eOutputCS);
                    BVDC_P_Vec_Build_SM_isr(
                        pCurInfo->pFmtInfo->eVideoFmt, eOutputCS, pTable,
                        pstChan, pList);
                }
            }
        }
    }

    hDisplay->stCurInfo.stDirty.stBits.bWidthTrim = BVDC_P_CLEAN;

    return;
}


/****************** Input color space event handlers ******************/
static void BVDC_P_Display_Apply_InputColorSpace_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BVDC_P_Output       eOutputCS;

    if (hDisplay->stAnlgChan_0.bEnable)
    {
        eOutputCS = hDisplay->stCurInfo.eAnlg_0_OutputColorSpace;
        BVDC_P_Vec_Build_CSC_SRC_SM_isr(hDisplay, &hDisplay->stAnlgChan_0, eOutputCS, pList);
    }

    if (hDisplay->stAnlgChan_1.bEnable)
    {
        eOutputCS = hDisplay->stCurInfo.eAnlg_1_OutputColorSpace;
        BVDC_P_Vec_Build_CSC_SRC_SM_isr(hDisplay, &hDisplay->stAnlgChan_1, eOutputCS, pList);
    }

    if(hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bEnableHdmi)
    {
        BVDC_P_Vec_Build_DVI_CSC_isr(hDisplay, &hDisplay->stDviChan, pList );
    }

#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
    if (hDisplay->st656Chan.bEnable)
    {
        BVDC_P_Vec_Build_656_CSC_isr(hDisplay, pList );
    }
#endif

    hDisplay->stCurInfo.stDirty.stBits.bInputCS = BVDC_P_CLEAN;

    BSTD_UNUSED(eFieldPolarity);

    return;
}

/****************** Source frame rate change event handlers ******************/
static BERR_Code BVDC_P_Display_Validate_SrcFrameRate_Setting
    ( BVDC_Display_Handle              hDisplay )
{
    const uint32_t          *pTable;
    BAVC_VdcDisplay_Info     lRateInfo;
    BVDC_P_DisplayInfo      *pNewInfo = &hDisplay->stNewInfo;
    bool                     bMissingTable = false;

    /* STG doesn't have RM */
    if (!BVDC_P_DISPLAY_USED_STG(hDisplay->eMasterTg))
    {
        bMissingTable |= (BVDC_P_GetRmTable_isr(
            pNewInfo, pNewInfo->pFmtInfo, &pTable, true, &lRateInfo)
            != BERR_SUCCESS);
        if (pNewInfo->bMultiRateAllow)
        {
            bMissingTable |= (BVDC_P_GetRmTable_isr(
                pNewInfo, pNewInfo->pFmtInfo, &pTable, false, &lRateInfo)
                != BERR_SUCCESS);
        }
        if (bMissingTable)
        {
            BDBG_ERR(("Failed to locate Rate Manager Table"));
            return BERR_TRACE(BVDC_ERR_FORMAT_NOT_SUPPORT_ANALOG_OUTPUT);
        }
    }

    return BERR_SUCCESS;
}

static void BVDC_P_Display_Copy_SrcFrameRate_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    hDisplay->stCurInfo.eDropFrame = hDisplay->stNewInfo.eDropFrame;

    return;
}

static void BVDC_P_Display_Apply_SrcFrameRate_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BSTD_UNUSED(eFieldPolarity);

    if(hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bEnableHdmi)
    {
        BVDC_P_Vec_Build_DVI_RM_isr(hDisplay, &hDisplay->stDviChan, pList, false);
    }

    if((hDisplay->bAnlgEnable && !hDisplay->stCurInfo.bHdmiRmd) ||   /* if analog master */
       (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
       (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
    {
        BVDC_P_Vec_Build_RM_isr(hDisplay, &hDisplay->stAnlgChan_0, pList);
    }

#if (BVDC_P_SUPPORT_STG != 0)
    if (hDisplay->stCurInfo.bEnableStg)
    {
        BVDC_P_STG_Build_RM_isr(hDisplay, pList);
    }
#endif
    hDisplay->stCurInfo.stDirty.stBits.bSrcFrameRate = BVDC_P_CLEAN;

    return;
}


/**************** Component path MPAA setting event hanlders **************/
void BVDC_P_FreeMpaaResources_isr
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_P_DisplayMpaa              *pstChan )
{
    if (pstChan->ulHwId != BVDC_P_HW_ID_INVALID)
    {
        BVDC_P_Resource_ReleaseHwId_isr(hResource, BVDC_P_ResourceType_eMpaa, pstChan->ulHwId);
        pstChan->ulHwId = BVDC_P_HW_ID_INVALID;
    }
    return;
}

BERR_Code BVDC_P_AllocMpaaResources
    ( BVDC_P_Resource_Handle           hResource,
      BVDC_DisplayId                   eDisplayId,
      BVDC_P_DisplayMpaa              *pstChan )
{
    BERR_Code err = BERR_SUCCESS;

    if (pstChan->ulHwId == BVDC_P_HW_ID_INVALID)
    {
        err = BVDC_P_Resource_AcquireHwId_isr(hResource, BVDC_P_ResourceType_eMpaa, 0, eDisplayId, &pstChan->ulHwId, false);
        if (err)
        {
            BDBG_ERR(("No MPAA available"));
        }
    }

    BDBG_MSG(("     Mpaa ulHwId = %d", pstChan->ulHwId));
    return BERR_TRACE(err);
}

static BERR_Code BVDC_P_Display_Validate_CompMpaa_Setting
    ( BVDC_Display_Handle              hDisplay )
{
    BERR_Code err;

    if (hDisplay->stNewInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent])
    {
        BKNI_EnterCriticalSection();
        err = BVDC_P_AllocMpaaResources(hDisplay->hVdc->hResource,
            hDisplay->eId, &hDisplay->stMpaaComp);
        BKNI_LeaveCriticalSection();

        if (BERR_SUCCESS != err)
        {
            return BERR_TRACE(err);
        }
    }

    return BERR_SUCCESS;
}

static void BVDC_P_Display_Copy_CompMpaa_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    if (hDisplay->stNewInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent] &&
        hDisplay->stMpaaComp.eState == BVDC_P_DisplayResource_eInactive &&
        (hDisplay->stNewInfo.bRgb || hDisplay->stNewInfo.bYPrPb))
    {
        /* init enabling and connecting */
        hDisplay->stMpaaComp.eState = BVDC_P_DisplayResource_eCreate;
    }
    else if(hDisplay->stNewInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent] |=
        hDisplay->stCurInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent] &&
        hDisplay->stMpaaComp.eState == BVDC_P_DisplayResource_eActive)
    {
        /* init shut down process */
        hDisplay->stMpaaComp.eState = BVDC_P_DisplayResource_eDestroy;
    }

    hDisplay->stCurInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent] =
        hDisplay->stNewInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent];

    return;
}

static void BVDC_P_EnableMpaa_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t    ulSrc = 0;

    /* during format change, we need to recfg even if already enabled */
#if 0
    if (BVDC_P_DisplayResource_eActive == hDisplay->stMpaaHdmi.eState ||
        BVDC_P_DisplayResource_eActive == hDisplay->stMpaaComp.eState)
    {
        /* already enabled */
        return;
    }
#endif

    /* reset DECIM */
    BVDC_P_VEC_SW_INIT(DECIM_0, 0, 1);
    BVDC_P_VEC_SW_INIT(DECIM_0, 0, 0);

    /* Connect DECIM to compositor */
    ulSrc = BVDC_P_GetVecCfgSrc_isr(hDisplay);
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_DECIM_0_SOURCE);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(VEC_CFG_DECIM_0_SOURCE, SOURCE, ulSrc);

    /* Configure DECIM */
    /* DECIMATE_RATIO: The MPAA standard specifies the resolution to be
     * not greater than 520000 pixels in a frame */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VIDEO_ENC_DECIM_0_DECIM_CONTROL);
    if((hDisplay->stCurInfo.pFmtInfo->ulWidth *
        hDisplay->stCurInfo.pFmtInfo->ulHeight)/2 < 520000)
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VIDEO_ENC_DECIM_0_DECIM_CONTROL, PASSTHROUGH_COUNT, 0) |
            BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_0_DECIM_CONTROL, DECIMATE_RATIO, BY2)  |
            BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_0_DECIM_CONTROL, DECIMATE_SAMPLING_EN, ON) |
            BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_0_DECIM_CONTROL, DECIMATE_FILTER_EN, ON);
    }
    else
    {
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(VIDEO_ENC_DECIM_0_DECIM_CONTROL, PASSTHROUGH_COUNT, 0) |
            BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_0_DECIM_CONTROL, DECIMATE_RATIO, BY4) |
            BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_0_DECIM_CONTROL, DECIMATE_SAMPLING_EN, ON) |
            BCHP_FIELD_ENUM(VIDEO_ENC_DECIM_0_DECIM_CONTROL, DECIMATE_FILTER_EN, ON);
    }

    return;
}

static void BVDC_P_DisableMpaa_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    if (BVDC_P_DisplayResource_eInactive == hDisplay->stMpaaHdmi.eState &&
        BVDC_P_DisplayResource_eInactive == hDisplay->stMpaaComp.eState)
    {
        /* already disabled */
        return;
    }

    /* Disable DECIM source */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_DECIM_0_SOURCE);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(VEC_CFG_DECIM_0_SOURCE, SOURCE, BCHP_VEC_CFG_DECIM_0_SOURCE_SOURCE_DISABLE);

    /* reset DECIM */
    BVDC_P_VEC_SW_INIT(DECIM_0, 0, 1);
    BVDC_P_VEC_SW_INIT(DECIM_0, 0, 0);

    return;
}

static void BVDC_P_MpaaComp_Setup_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BVDC_P_DisplayAnlgChan          *pstChan)
{
    /* inside it will avoid re-do */
    BVDC_P_EnableMpaa_isr(hDisplay, pList);

    /* Connect IT to DECIM */
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_IT_0_SOURCE + pstChan->ulIt * 4);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(VEC_CFG_IT_0_SOURCE, SOURCE, BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_DECIM_0);
}

static void BVDC_P_MpaaComp_Shutdown_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BVDC_P_DisplayAnlgChan          *pstChan)
{
    uint32_t    ulSrc = 0;

    /* Connect IT to compositor */
    if(hDisplay->stCurInfo.bHdmiRmd)
    {
        ulSrc = BCHP_VEC_CFG_IT_0_SOURCE_SOURCE_DISABLE;
    }
    else
    {
        ulSrc = BVDC_P_GetVecCfgSrc_isr(hDisplay);
    }

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_IT_0_SOURCE + pstChan->ulIt * 4);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(VEC_CFG_IT_0_SOURCE, SOURCE, ulSrc);

    if (!hDisplay->stCurInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi])
    {
        /* Both component and HDMI paths disable MPAA. So we disable the HW
         */
        BVDC_P_DisableMpaa_isr(hDisplay, pList);
    }
}

static void BVDC_P_Display_Apply_CompMpaa_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BVDC_P_DisplayAnlgChan *pstChan;
    BSTD_UNUSED(eFieldPolarity);

    pstChan = &hDisplay->stAnlgChan_0;

    /* enable/disable info is already coded in eState */
    switch (hDisplay->stMpaaComp.eState)
    {
        /* Enable MPAA */
        case BVDC_P_DisplayResource_eCreate:
            BVDC_P_MpaaComp_Setup_isr(hDisplay, pList, pstChan);
            hDisplay->stMpaaComp.eState = BVDC_P_DisplayResource_eActivating;
            break;
        case BVDC_P_DisplayResource_eActivating:
            if (pList->bLastExecuted)
            {
                hDisplay->stMpaaComp.eState = BVDC_P_DisplayResource_eActive;
                hDisplay->stCurInfo.stDirty.stBits.bMpaaComp = BVDC_P_CLEAN;
            }
            else
            {
                /* Re-build the setup RUL */
                BVDC_P_MpaaComp_Setup_isr(hDisplay, pList, pstChan);
            }
            break;

        /* Disable MPAA */
        case BVDC_P_DisplayResource_eDestroy:
            BVDC_P_MpaaComp_Shutdown_isr(hDisplay, pList, pstChan);
            hDisplay->stMpaaComp.eState = BVDC_P_DisplayResource_eShuttingdown;
            break;

        case BVDC_P_DisplayResource_eShuttingdown:
            if (pList->bLastExecuted)
            {
                if (!hDisplay->stCurInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent])
                {
                    BVDC_P_FreeMpaaResources_isr(hDisplay->hVdc->hResource,
                        &hDisplay->stMpaaComp);
                }
                hDisplay->stMpaaComp.eState = BVDC_P_DisplayResource_eInactive;
                hDisplay->stCurInfo.stDirty.stBits.bMpaaComp = BVDC_P_CLEAN;
            }
            else
            {
                /* Re-build the shutdown RUL */
                BVDC_P_MpaaComp_Shutdown_isr(hDisplay, pList, pstChan);
            }
            break;

        default:
            hDisplay->stCurInfo.stDirty.stBits.bMpaaComp = BVDC_P_CLEAN;
            break;
    }
}

/**************** HDMI path MPAA setting event hanlders **************/

static BERR_Code BVDC_P_Display_Validate_HdmiMpaa_Setting
    ( BVDC_Display_Handle              hDisplay )
{
    BERR_Code err;

    if (hDisplay->stNewInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi])
    {
        BKNI_EnterCriticalSection();
        err = BVDC_P_AllocMpaaResources(hDisplay->hVdc->hResource,
            hDisplay->eId, &hDisplay->stMpaaHdmi);
        BKNI_LeaveCriticalSection();

        if (BERR_SUCCESS != err)
        {
            return BERR_TRACE(err);
        }
    }

    return BERR_SUCCESS;
}

static void BVDC_P_Display_Copy_HdmiMpaa_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    if (hDisplay->stNewInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi] &&
        hDisplay->stMpaaHdmi.eState == BVDC_P_DisplayResource_eInactive &&
        hDisplay->stNewInfo.bEnableHdmi)
    {
        /* init enabling and connect */
        hDisplay->stMpaaHdmi.eState = BVDC_P_DisplayResource_eCreate;
    }
    else if(hDisplay->stNewInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi] !=
        hDisplay->stCurInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi] &&
        hDisplay->stMpaaHdmi.eState == BVDC_P_DisplayResource_eActive)
    {
        /* init shut down process */
        hDisplay->stMpaaHdmi.eState = BVDC_P_DisplayResource_eDestroy;
    }

    hDisplay->stCurInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi] =
        hDisplay->stNewInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi];

    return;
}

static void BVDC_P_MpaaHdmi_Setup_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t ulDviSrc = 0;

    /* inside it will avoid re-do */
    BVDC_P_EnableMpaa_isr(hDisplay, pList);

    /* Connect DVI to DECIM */
    ulDviSrc = BCHP_VEC_CFG_DVI_DTG_0_SOURCE_SOURCE_DECIM_0;
    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_DVI_DTG_0_SOURCE + hDisplay->stDviChan.ulDvi * 4);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(VEC_CFG_DVI_DTG_0_SOURCE, SOURCE, ulDviSrc);
}

static void BVDC_P_MpaaHdmi_Shutdown_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    uint32_t ulDviSrc = 0;

    /* Connect DVI to compositor */
    ulDviSrc = BVDC_P_GetVecCfgSrc_isr(hDisplay);

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_VEC_CFG_DVI_DTG_0_SOURCE + hDisplay->stDviChan.ulDvi * 4);
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(VEC_CFG_DVI_DTG_0_SOURCE, SOURCE, ulDviSrc);

    if (!hDisplay->stCurInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eComponent])
    {
        /* Both component and hdmi paths disable MPAA. So we disable the HW
         */
        BVDC_P_DisableMpaa_isr(hDisplay, pList);
    }
}

static void BVDC_P_Display_Apply_HdmiMpaa_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BSTD_UNUSED(eFieldPolarity);

    /* enable/disable info is already coded in eState */
    switch (hDisplay->stMpaaHdmi.eState)
    {
        /* Enable MPAA */
        case BVDC_P_DisplayResource_eCreate:
            BVDC_P_MpaaHdmi_Setup_isr(hDisplay, pList);
            hDisplay->stMpaaHdmi.eState = BVDC_P_DisplayResource_eActivating;
            break;
        case BVDC_P_DisplayResource_eActivating:
            if (pList->bLastExecuted)
            {
                hDisplay->stMpaaHdmi.eState = BVDC_P_DisplayResource_eActive;
                hDisplay->stCurInfo.stDirty.stBits.bMpaaHdmi = BVDC_P_CLEAN;
            }
            else
            {
                /* Re-build the setup RUL */
                BVDC_P_MpaaHdmi_Setup_isr(hDisplay, pList);
            }
            break;

        /* Disable MPAA */
        case BVDC_P_DisplayResource_eDestroy:
            BVDC_P_MpaaHdmi_Shutdown_isr(hDisplay, pList);
            hDisplay->stMpaaHdmi.eState = BVDC_P_DisplayResource_eShuttingdown;
            break;
        case BVDC_P_DisplayResource_eShuttingdown:
            if (pList->bLastExecuted)
            {
                if (!hDisplay->stCurInfo.aulEnableMpaaDeci[BVDC_MpaaDeciIf_eHdmi])
                {
                    BVDC_P_FreeMpaaResources_isr(hDisplay->hVdc->hResource,
                        &hDisplay->stMpaaHdmi);
                }
                hDisplay->stMpaaHdmi.eState = BVDC_P_DisplayResource_eInactive;
                hDisplay->stCurInfo.stDirty.stBits.bMpaaHdmi = BVDC_P_CLEAN;
            }
            else
            {
                /* Re-build the shutdown RUL */
                BVDC_P_MpaaHdmi_Shutdown_isr(hDisplay, pList);
            }
            break;
        default:
            hDisplay->stCurInfo.stDirty.stBits.bMpaaHdmi = BVDC_P_CLEAN;
            break;
    }
}

/**************** RFM configuration event hanlders **************/
#if (BVDC_P_SUPPORT_RFM_OUTPUT != 0)

static BERR_Code BVDC_P_Display_Validate_Rfm_Setting
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo *pNewInfo;

    if (!hDisplay->bAnlgEnable)
    {
        BDBG_ERR(("Analog channel of display[%d] is not enabled. No RFM",
            hDisplay->eId));
        return BERR_TRACE(BVDC_ERR_INVALID_RFM_PATH);
    }

    if(hDisplay->stAnlgChan_0.ulSdsrc == BVDC_P_HW_ID_INVALID &&
       hDisplay->stAnlgChan_1.ulSdsrc == BVDC_P_HW_ID_INVALID)
    {
        BDBG_ERR(("Display[%d] does not acquire SDSRC", hDisplay->eId));
        return BERR_TRACE(BVDC_ERR_INVALID_RFM_PATH);
    }

    pNewInfo = &hDisplay->stNewInfo;

    if(BVDC_RfmOutput_eCVBS == pNewInfo->eRfmOutput)
    {
        if ( VIDEO_FORMAT_IS_HD(pNewInfo->pFmtInfo->eVideoFmt) )
        {
            BDBG_ERR(("Display[%d] video format %d doesn't support RFM CVBS output ",
                hDisplay->eId, pNewInfo->pFmtInfo->eVideoFmt));
            return BERR_TRACE(BVDC_ERR_VIDEOFMT_OUTPUT_MISMATCH);
        }
    }

    return BERR_SUCCESS;
}

static void BVDC_P_Display_Copy_Rfm_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    hDisplay->stCurInfo.eRfmOutput = hDisplay->stNewInfo.eRfmOutput;
    hDisplay->stCurInfo.ulRfmConst = hDisplay->stNewInfo.ulRfmConst;

    return;
}

static void BVDC_P_Display_Apply_Rfm_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    uint32_t ulRfmSel=0, ulRfmSyncSel=0, ulConst=0, ulSdsrcId = BVDC_P_HW_ID_INVALID;

    BSTD_UNUSED(eFieldPolarity);

    if(hDisplay->stCurInfo.eRfmOutput != BVDC_RfmOutput_eCVBS)
    {
        ulRfmSel = BCHP_MISC_RFM_0_CFG_SEL_CONST;
        ulConst = hDisplay->stCurInfo.ulRfmConst;
    }
    else
    {
        ulSdsrcId = (hDisplay->stAnlgChan_0.ulSdsrc != BVDC_P_HW_ID_INVALID) ?
            hDisplay->stAnlgChan_0.ulSdsrc : hDisplay->stAnlgChan_1.ulSdsrc;

        switch(ulSdsrcId)
        {
            case 0:
                ulRfmSel = BCHP_MISC_RFM_0_CFG_SEL_SDSRC_0;
                ulRfmSyncSel = BCHP_MISC_RFM_0_SYNC_0_CFG_SEL_SDSRC_0;
                break;

#if (BVDC_P_NUM_SHARED_SDSRC > 1)
            case 1:
                ulRfmSel = BCHP_MISC_RFM_0_CFG_SEL_SDSRC_1;
                ulRfmSyncSel = BCHP_MISC_RFM_0_SYNC_0_CFG_SEL_SDSRC_1;
                break;

#if (BVDC_P_NUM_SHARED_SDSRC > 2)
            case 2:
                ulRfmSel = BCHP_MISC_RFM_0_CFG_SEL_SDSRC_2;
                ulRfmSyncSel = BCHP_MISC_RFM_0_SYNC_0_CFG_SEL_SDSRC_2;
                break;
#endif
#endif
            default:
                BDBG_ERR(("Display [%d] analog channel acquires no SDSRC sub-block", hDisplay->eId));
                BDBG_ASSERT(0);
        }

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_RFM_0_SYNC_0_CFG);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(MISC_RFM_0_SYNC_0_CFG, SEL, ulRfmSyncSel);
    }

    *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
    *pList->pulCurrent++ = BRDC_REGISTER(BCHP_MISC_RFM_0_CFG );
    *pList->pulCurrent++ =
        BCHP_FIELD_DATA(MISC_RFM_0_CFG, SEL, ulRfmSel) |
        BCHP_FIELD_DATA(MISC_RFM_0_CFG, CONST, ulConst);

    hDisplay->stCurInfo.stDirty.stBits.bRfm = BVDC_P_CLEAN;

    return;
}
#endif

/**************** HDMI output enable/disable event handlers **************/
static BERR_Code BVDC_P_Display_Validate_Hdmi_Config
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo *pNewInfo;
    BERR_Code err;
    pNewInfo = &hDisplay->stNewInfo;

    /* By checking "stDviChan.bEnable" in BVDC_P_Display_Validate_VideoFormat()
     * and checking "pNewInfo->bEnableHdmi" here guarantees this validation won't be done
     * two times.
     */
    if (pNewInfo->bEnableHdmi)
    {
        if ((err = BVDC_P_Display_Validate_DviChan(hDisplay)) != BERR_SUCCESS)
            return BERR_TRACE(err);

        BKNI_EnterCriticalSection();
        err = BVDC_P_AllocDviChanResources_isr(
                hDisplay->hVdc->hResource, hDisplay->hVdc->hRegister,
                hDisplay->eId, pNewInfo->stHdmiSettings.stSettings.ulPortId,
                &hDisplay->stDviChan, hDisplay->hCompositor->eId);
        BKNI_LeaveCriticalSection();
        /* SW7563-101: close connected window before disable hdmi */
        if((hDisplay->stNewInfo.stHdmiSettings.stSettings.eMatrixCoeffs == BAVC_MatrixCoefficients_eUnknown)
            &&(BVDC_P_DISPLAY_USED_DVI(hDisplay->eMasterTg))
            &&(hDisplay->hCompositor->ulActiveGfxWindow + hDisplay->hCompositor->ulActiveVideoWindow))
        {
            BDBG_ERR(("display [%d] hdmi output can NOT be removed before all its connected windows are closed", hDisplay->eId));
            err = BERR_INVALID_PARAMETER;
        }

        if(err != BERR_SUCCESS)
            return BERR_TRACE(err);

#ifdef BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY0
        if(hDisplay->ulHdmiPwrAcquire == 0)
        {
            BDBG_MSG(("hdmi slave mode: Acquire BCHP_PWR_RESOURCE_VDC_HDMI_TX_PHY %d", hDisplay->stDviChan.ulDvi));
            BCHP_PWR_AcquireResource(hDisplay->hVdc->hChip, hDisplay->ulHdmiPwrId);
            hDisplay->ulHdmiPwrAcquire++;
        }
#endif
    }

    return BERR_SUCCESS;
}

/**************** HDMI output CSC matrix event handlers **************/
static void BVDC_P_Display_Copy_HdmiCsc_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    int i;

    hDisplay->stCurInfo.stHdmiSettings.stSettings.eMatrixCoeffs = hDisplay->stNewInfo.stHdmiSettings.stSettings.eMatrixCoeffs;
    hDisplay->stCurInfo.stDvoCfg = hDisplay->stNewInfo.stDvoCfg;

    hDisplay->stCurInfo.lDvoAttenuationR = hDisplay->stNewInfo.lDvoAttenuationR;
    hDisplay->stCurInfo.lDvoAttenuationG = hDisplay->stNewInfo.lDvoAttenuationG;
    hDisplay->stCurInfo.lDvoAttenuationB = hDisplay->stNewInfo.lDvoAttenuationB;
    hDisplay->stCurInfo.lDvoOffsetR = hDisplay->stNewInfo.lDvoOffsetR;
    hDisplay->stCurInfo.lDvoOffsetG = hDisplay->stNewInfo.lDvoOffsetG;
    hDisplay->stCurInfo.lDvoOffsetB = hDisplay->stNewInfo.lDvoOffsetB;
    hDisplay->stCurInfo.bBypassVideoProcess = hDisplay->stNewInfo.bBypassVideoProcess;

    hDisplay->stCurInfo.bUserCsc = hDisplay->stNewInfo.bUserCsc;
    for (i = 0; i < BVDC_CSC_COEFF_COUNT; i++)
    {
        hDisplay->stCurInfo.pl32_Matrix[i] = hDisplay->stNewInfo.pl32_Matrix[i];
    }

    return;
}

static void BVDC_P_Display_Apply_HdmiCsc_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )

{
    BSTD_UNUSED(eFieldPolarity);

    if(hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bEnableHdmi)
    {
        BVDC_P_Vec_Build_DVI_CSC_isr(hDisplay, &hDisplay->stDviChan, pList);
    }

    hDisplay->stCurInfo.stDirty.stBits.bHdmiCsc = BVDC_P_CLEAN;

    return;
}

static void BVDC_P_Display_Copy_Hdmi_Config_isr
    ( BVDC_Display_Handle              hDisplay )
{
    int i;

    hDisplay->stCurInfo.stHdmiSettings.stSettings.ulPortId = hDisplay->stNewInfo.stHdmiSettings.stSettings.ulPortId;
    hDisplay->stCurInfo.bEnableHdmi = hDisplay->stNewInfo.bEnableHdmi;

    if (hDisplay->stCurInfo.bEnableHdmi)
    {
        hDisplay->eDviState = BVDC_P_DisplayResource_eCreate;
    }
    else
    {
        /* Tear down the DVI channel */
        hDisplay->eDviState = BVDC_P_DisplayResource_eDestroy;
    }

    BVDC_P_Display_Copy_HdmiCsc_Setting_isr(hDisplay);

    hDisplay->stCurInfo.bUserCsc = hDisplay->stNewInfo.bUserCsc;
    for (i = 0; i < BVDC_CSC_COEFF_COUNT; i++)
    {
        hDisplay->stCurInfo.pl32_Matrix[i] = hDisplay->stNewInfo.pl32_Matrix[i];
    }

    return;
}

static void BVDC_P_Display_Apply_Hdmi_Config_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )

{
    BVDC_P_DisplayInfo *pCurInfo;
    BVDC_P_DisplayDviChan *pstChan;

    BSTD_UNUSED(eFieldPolarity);

    pCurInfo = &hDisplay->stCurInfo;
    pstChan = &hDisplay->stDviChan;

    if (pCurInfo->bEnableHdmi)
    {
        /* When DVI is in slave mode, a format switch has to be done. Even
         * if the RUL somehow didn't get exeucted, our re-do mechanism will keep
         * trying. So it is safe to move the state to eActive directly. */
        hDisplay->eDviState = BVDC_P_DisplayResource_eActive;
        hDisplay->stCurInfo.stDirty.stBits.bHdmiEnable = BVDC_P_CLEAN;
    }
    else
    {
        /* Reset DVI core, disconnect source.
         * If DVI is the master timing generator, will this automatically
         * shut down all the slave paths?
         */
        switch (hDisplay->eDviState)
        {
            case BVDC_P_DisplayResource_eDestroy:
                BVDC_P_TearDownDviChan_isr(hDisplay, pstChan, pList);
                hDisplay->eDviState = BVDC_P_DisplayResource_eShuttingdown;
                break;

            case BVDC_P_DisplayResource_eShuttingdown:
                if (pList->bLastExecuted)
                {
                    if(!hDisplay->stDviChan.bEnable)
                        BVDC_P_FreeDviChanResources_isr(hDisplay->hVdc->hResource, hDisplay, pstChan);
                    hDisplay->eDviState = BVDC_P_DisplayResource_eInactive;
                    hDisplay->stCurInfo.stDirty.stBits.bHdmiEnable = BVDC_P_CLEAN;
                }
                else
                {
                    /* Re-build the teardown RUL */
                    BVDC_P_TearDownDviChan_isr(hDisplay, pstChan, pList);
                }
                break;

            default:
                hDisplay->stCurInfo.stDirty.stBits.bHdmiEnable = BVDC_P_CLEAN;
                break;
        }
    }

    return;
}

/**************** 656 output enable/disable event handlers **************/
#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
static BERR_Code BVDC_P_Display_Validate_656_Setting
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_DisplayInfo *pNewInfo;
    BERR_Code err;
    pNewInfo = &hDisplay->stNewInfo;

    /* By checking "st656Chan.bEnable" in BVDC_P_Display_Validate_VideoFormat()
     * and checking "pNewInfo->bEnable656" here guarantees this validation won't be done
     * two times.
     */
    if (pNewInfo->bEnable656)
    {
        if ((err = BVDC_P_Display_Validate_656Chan(hDisplay)) != BERR_SUCCESS)
            return BERR_TRACE(err);

        BKNI_EnterCriticalSection();
        err = BVDC_P_Alloc656ChanResources_isr(hDisplay->hVdc->hResource,
              hDisplay->eId, &hDisplay->st656Chan, hDisplay->hCompositor->eId);
        BKNI_LeaveCriticalSection();
        if (err != BERR_SUCCESS)
            return BERR_TRACE(err);

#ifdef BCHP_PWR_RESOURCE_VDC_656_OUT
        if(hDisplay->ul656PwrAcquire == 0)
        {
            BDBG_MSG(("656 slave mode: Acquire BCHP_PWR_RESOURCE_VDC_656_OUT %d", hDisplay->st656Chan.ul656));
            BCHP_PWR_AcquireResource(hDisplay->hVdc->hChip, BCHP_PWR_RESOURCE_VDC_656_OUT);
            hDisplay->ul656PwrAcquire++;
        }
#endif
    }

    hDisplay->st656Chan.bEnable = pNewInfo->bEnable656;
    return BERR_SUCCESS;
}


static void BVDC_P_Display_Copy_656_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    hDisplay->stCurInfo.bEnable656 = hDisplay->stNewInfo.bEnable656;

    if (hDisplay->stCurInfo.bEnable656)
    {
        hDisplay->e656State = BVDC_P_DisplayResource_eCreate;
    }
    else
    {
        /* Tear down the 656 channel */
        hDisplay->e656State = BVDC_P_DisplayResource_eDestroy;
    }
}

static void BVDC_P_Display_Apply_656_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )

{
    BVDC_P_DisplayInfo *pCurInfo;
    BVDC_P_Display656Chan *pstChan;

    BSTD_UNUSED(eFieldPolarity);

    pCurInfo = &hDisplay->stCurInfo;
    pstChan = &hDisplay->st656Chan;

    if (pCurInfo->bEnable656)
    {
        /* When 656 is in slave mode, a format switch has to be done. Even
         * if the RUL somehow didn't get exeucted, our re-do mechanism will keep
         * trying. So it is safe to move the state to eActive directly.
         */
        hDisplay->e656State = BVDC_P_DisplayResource_eActive;
        hDisplay->stCurInfo.stDirty.stBits.b656Enable = BVDC_P_CLEAN;
    }
    else
    {
        /* Reset 656 core, disconnect source.
         * If 656 is the master timing generator, will this automatically
         * shut down all the slave paths?
         */
        switch (hDisplay->e656State)
        {

            case BVDC_P_DisplayResource_eDestroy:
                BVDC_P_TearDown656Chan_isr(hDisplay, pstChan, pList);
                hDisplay->st656Chan.bEnable = false;
                hDisplay->e656State = BVDC_P_DisplayResource_eShuttingdown;
                break;

            case BVDC_P_DisplayResource_eShuttingdown:
                if (pList->bLastExecuted)
                {
                    BVDC_P_Free656ChanResources_isr(hDisplay->hVdc->hResource, hDisplay);
                    hDisplay->e656State = BVDC_P_DisplayResource_eInactive;
                    hDisplay->stCurInfo.stDirty.stBits.b656Enable = BVDC_P_CLEAN;
                }
                else
                {
                    /* Re-build the teardown RUL */
                    BVDC_P_TearDown656Chan_isr(hDisplay, pstChan, pList);
                }
                break;

            default:
                hDisplay->stCurInfo.stDirty.stBits.b656Enable = BVDC_P_CLEAN;
                break;
        }
    }

    return;
}
#endif

/**************** aspect ratio event handlers **************/

static BERR_Code BVDC_P_Display_Validate_AspRatio_Setting
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_P_ClipRect *pAspRatRectClip;
    BVDC_P_DisplayInfo *pNewInfo = &hDisplay->stNewInfo;

    /* aspect ratio canvas clip validation */
    pAspRatRectClip = &(pNewInfo->stAspRatRectClip);
    if ((pAspRatRectClip->ulLeft + pAspRatRectClip->ulRight >= pNewInfo->pFmtInfo->ulWidth) ||
        (pAspRatRectClip->ulTop + pAspRatRectClip->ulBottom >= pNewInfo->pFmtInfo->ulHeight))
    {
        return BERR_TRACE(BVDC_ERR_INVALID_DISP_ASPECT_RATIO_RECT);
    }

    if(BVDC_P_IS_UNKNOWN_ASPR(pNewInfo->eAspectRatio, pNewInfo->uiSampleAspectRatioX, pNewInfo->uiSampleAspectRatioY))
    {
        BDBG_ERR(("Invalid aspect ratio settings eAspectRatio: %d, uiSampleAspectRatioX: %d, uiSampleAspectRatioY: %d",
            pNewInfo->eAspectRatio, pNewInfo->uiSampleAspectRatioX, pNewInfo->uiSampleAspectRatioY));
        return BERR_TRACE(BVDC_ERR_INVALID_DISP_ASPECT_RATIO_RECT);
    }

    return BERR_SUCCESS;

}

static void BVDC_P_Display_Copy_AspRatio_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    bool bUseDigTrg = BVDC_P_DISPLAY_USED_DIGTRIG(hDisplay->eMasterTg);
    hDisplay->stCurInfo.eAspectRatio = hDisplay->stNewInfo.eAspectRatio;
    hDisplay->stCurInfo.uiSampleAspectRatioX = hDisplay->stNewInfo.uiSampleAspectRatioX;
    hDisplay->stCurInfo.uiSampleAspectRatioY = hDisplay->stNewInfo.uiSampleAspectRatioY;
    hDisplay->stCurInfo.stAspRatRectClip = hDisplay->stNewInfo.stAspRatRectClip;

    BVDC_P_CalcuPixelAspectRatio_isr(
        hDisplay->stCurInfo.eAspectRatio,
        hDisplay->stCurInfo.uiSampleAspectRatioX,
        hDisplay->stCurInfo.uiSampleAspectRatioY,
        bUseDigTrg?hDisplay->stCurInfo.pFmtInfo->ulDigitalWidth : hDisplay->stCurInfo.pFmtInfo->ulWidth,
        bUseDigTrg?hDisplay->stCurInfo.pFmtInfo->ulDigitalHeight: hDisplay->stCurInfo.pFmtInfo->ulHeight,
        &hDisplay->stCurInfo.stAspRatRectClip,
        &hDisplay->ulPxlAspRatio,
        &hDisplay->ulPxlAspRatio_x_y,
        BFMT_Orientation_e2D);

    if(hDisplay->stCurInfo.eOrientation == BFMT_Orientation_e3D_OverUnder)
    {
        hDisplay->ulPxlAspRatio = hDisplay->ulPxlAspRatio / 2;
    }
    else if(hDisplay->stCurInfo.eOrientation == BFMT_Orientation_e3D_LeftRight)
    {
        hDisplay->ulPxlAspRatio = hDisplay->ulPxlAspRatio * 2;
    }

    hDisplay->hCompositor->ulStgPxlAspRatio_x_y = hDisplay->ulPxlAspRatio_x_y;
    /* inform window ApplyChanges  */
    hDisplay->hCompositor->bDspAspRatDirty = true;

    return;
}

static void BVDC_P_Display_Apply_AspRatio_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )

{
    BSTD_UNUSED(hDisplay);
    BSTD_UNUSED(pList);
    BSTD_UNUSED(eFieldPolarity);

    /* nothing to do here */
    hDisplay->stCurInfo.stDirty.stBits.bAspRatio = BVDC_P_CLEAN;
    return;
}

/**************** Take a snapshot of timer event handlers **************/
static  BERR_Code BVDC_P_Display_CalculateTimeGap_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_DisplayContext           *pTargetDisplay,
      int32_t                         *plDeltaTs )
{
    int32_t lDeltaTs;
    uint32_t ulVsyncInterval =
        BVDC_P_DISPLAY_ONE_VSYNC_INTERVAL(hDisplay->stCurInfo.pFmtInfo->ulVertFreq);
    uint32_t ulTs, ulTargetTs, ulDecelTime=0, ulAccelTime=0, ulAdjustTsCnt = 0;
#if (BVDC_P_USE_RDC_TIMESTAMP)
    BRDC_Slot_Handle   hPlaybackSlot;
#endif

#if (BVDC_P_USE_RDC_TIMESTAMP)
    if(hDisplay->eTimeStampPolarity == BAVC_Polarity_eFrame)
    {
        /* Progressive format use top field slot */
        hPlaybackSlot = hDisplay->hCompositor->ahSlot[BAVC_Polarity_eTopField];
    }
    else
    {
        hPlaybackSlot = hDisplay->hCompositor->ahSlot[hDisplay->eTimeStampPolarity];
    }
    /* Get timer snapshot in microseconds */
    ulTs = BRDC_Slot_GetTimerSnapshot_isr(hPlaybackSlot);
    /* Rate manager is 27 MHz. Convert to tick value in 27 Mhz */
    ulTs = ulTs * (BVDC_P_TIMER_FREQ / 1000000);

    if(pTargetDisplay->eTimeStampPolarity == BAVC_Polarity_eFrame)
    {
        hPlaybackSlot = pTargetDisplay->hCompositor->ahSlot[BAVC_Polarity_eTopField];
    }
    else
    {
        hPlaybackSlot = pTargetDisplay->hCompositor->ahSlot[pTargetDisplay->eTimeStampPolarity];
    }
    /* Get timer snapshot in microseconds */
    ulTargetTs = BRDC_Slot_GetTimerSnapshot_isr(hPlaybackSlot);
    /* Rate manager is 27 MHz. Convert to tick value in 27 Mhz */
    ulTargetTs = 27 * ulTargetTs;
#else
    ulTs = BREG_Read32_isr(hDisplay->hVdc->hRegister, hDisplay->ulScratchTsAddr);
    ulTargetTs = BREG_Read32_isr(pTargetDisplay->hVdc->hRegister, pTargetDisplay->ulScratchTsAddr);
#endif

    /* The delta between the two displays should be within one vsync.
     * Otherwise the timestamp is invalid.
     */
    if (((ulTs < ulTargetTs) && ((ulTs + ulVsyncInterval) < ulTargetTs)) ||
        ((ulTs > ulTargetTs) && (ulTs > (ulVsyncInterval + ulTargetTs))))
    {
        /* This could be caused by execution of the RUL that samples timestamp
         * was delayed or timer happened to wraparound between the two samples.
         *
         */
        BDBG_ERR(("Invalid timestamp value: display[%d] %d, target display[%d] %d",
                    hDisplay->eId, ulTs, pTargetDisplay->eId, ulTargetTs));
        return BERR_TRACE(BVDC_ERR_INVALID_TIMESTAMP);
    }

    /* Adjust time stamp based on the alignment pattern */
    ulAdjustTsCnt = (hDisplay->stCurInfo.ulVertFreq == 2397 && pTargetDisplay->stCurInfo.ulVertFreq==5994) ?
                    1 : 0;
    ulTs += ulAdjustTsCnt * ulVsyncInterval;

    if(BVDC_AlignmentDirection_eAuto == hDisplay->stCurInfo.stAlignCfg.eDirection)
    {
        /* interlaced/interlaced case; */
        if(hDisplay->stCurInfo.pFmtInfo->bInterlaced &&
            pTargetDisplay->stCurInfo.pFmtInfo->bInterlaced)
        {
            /* same polarity */
            if (hDisplay->eTimeStampPolarity == pTargetDisplay->eTimeStampPolarity)
            {
                lDeltaTs = (int32_t)ulTs - (int32_t)ulTargetTs;
            }
            else /* opposite polarity */
            {
                /* Acceleration or deceleration will take the same amount of time.
                 * The goal is to match polarity as well after adjustment.
                 */
                lDeltaTs = (int32_t)ulVsyncInterval + (int32_t)ulTs - (int32_t)ulTargetTs;
            }
        }
        /* progressive/interlaced case */
        else
        {
            /* We can either accelaerate or decelerate. The more efficient approach
             * will be chosen.
             */
            if (ulTs < ulTargetTs)
            {
                ulDecelTime = ulTargetTs - ulTs;
                ulAccelTime = (ulVsyncInterval/2 + ulTs - ulTargetTs);
            }
            else
            {
                ulAccelTime = ulTs - ulTargetTs;
                ulDecelTime = (ulVsyncInterval/2 + ulTargetTs - ulTs);
            }

            lDeltaTs = (ulDecelTime < ulAccelTime) ? ((int32_t)0 - (int32_t) ulDecelTime) : (int32_t)ulAccelTime;
        }
    }
    else /* manual mode */
    {
        /* interlaced/interlaced case; */
        if(hDisplay->stCurInfo.pFmtInfo->bInterlaced &&
            pTargetDisplay->stCurInfo.pFmtInfo->bInterlaced)
        {
            if (ulTs < ulTargetTs)
            {
                /* same polarity */
                if (hDisplay->eTimeStampPolarity == pTargetDisplay->eTimeStampPolarity)
                {
                    ulAccelTime = 2 * ulVsyncInterval + ulTs - ulTargetTs;
                    ulDecelTime = ulTargetTs - ulTs;
                }
                else /* opposite polarity */
                {
                    ulAccelTime = ulVsyncInterval + ulTs - ulTargetTs;
                    ulDecelTime = ulVsyncInterval + ulTargetTs - ulTs;
                }
            }
        }
        /* progressive/interlaced case */
        else
        {
            if (ulTs < ulTargetTs)
            {
                ulAccelTime = ulVsyncInterval + ulTs - ulTargetTs;
                ulDecelTime = ulTargetTs - ulTs;
            }
            else
            {
                ulAccelTime = ulTargetTs - ulTs;
                ulDecelTime = ulVsyncInterval + ulTargetTs - ulTs;
            }
        }

        lDeltaTs = (hDisplay->stCurInfo.stAlignCfg.eDirection == BVDC_AlignmentDirection_eFaster) ?
                    (int32_t) ulAccelTime : ((int32_t)0 - (int32_t) ulDecelTime);
    }

    *plDeltaTs = lDeltaTs;

    return BERR_SUCCESS;
}

static void BVDC_P_Display_StartAlignAdjustment_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      int32_t                          lDeltaTs )
{
    uint32_t ulCount;
    bool bSkip;
    uint32_t ulOffset;

    /* Timebase pulse is 27MHz which is the same as the timer frequency.
     * We can directly use the time stamp delta value as the repeat-skip
     * count. */
    ulCount = BVDC_P_ABS(lDeltaTs);
    BDBG_MSG(("About to start alignment adjustment, delta %d,  %d us", lDeltaTs, ulCount/27));

    bSkip = (lDeltaTs < 0) ? true : false;

    if(hDisplay->bAnlgEnable)
    {
        /* RM pairs with IT */
        ulOffset = hDisplay->stAnlgChan_0.ulRmRegOffset;

        /* Change Tracking_Range to 15625 ppm */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(RM_0_CONTROL, TRACKING_RANGE)),
            BCHP_FIELD_DATA(RM_0_CONTROL, TRACKING_RANGE, BVDC_P_TRACKING_RANGE_GAMEMODE),
            BCHP_RM_0_CONTROL + ulOffset);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_RM_0_SKIP_REPEAT_GAP + ulOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(RM_0_SKIP_REPEAT_GAP, GAP_COUNT, BVDC_P_DISPLAY_SKIP_REPEAT_GAP);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_RM_0_SKIP_REPEAT_NUMBER + ulOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(RM_0_SKIP_REPEAT_NUMBER, COUNT, ulCount);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_RM_0_SKIP_REPEAT_CONTROL + ulOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(RM_0_SKIP_REPEAT_CONTROL, MODE, bSkip ? 1 : 0) |
            BCHP_FIELD_DATA(RM_0_SKIP_REPEAT_CONTROL, ENABLE, 1);
    }

    if(hDisplay->stDviChan.bEnable)
    {
        ulOffset = hDisplay->stDviChan.ulDvpRegOffset;
        /* Change Tracking_Range to 15625 ppm */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_RM_CONTROL, TRACKING_RANGE)),
            BCHP_FIELD_DATA(HDMI_RM_CONTROL, TRACKING_RANGE, BVDC_P_TRACKING_RANGE_GAMEMODE),
            BCHP_HDMI_RM_CONTROL + ulOffset);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_SKIP_REPEAT_GAP + ulOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(HDMI_RM_SKIP_REPEAT_GAP, GAP_COUNT, BVDC_P_DISPLAY_SKIP_REPEAT_GAP);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_SKIP_REPEAT_NUMBER + ulOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(HDMI_RM_SKIP_REPEAT_NUMBER, COUNT, ulCount);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_SKIP_REPEAT_CONTROL + ulOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(HDMI_RM_SKIP_REPEAT_CONTROL, MODE, bSkip ? 1 : 0) |
            BCHP_FIELD_DATA(HDMI_RM_SKIP_REPEAT_CONTROL, ENABLE, 1);
    }

    return;
}

static void BVDC_P_Display_StopAlignAdjustment_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    BDBG_MSG(("Alignment is done "));

    if(hDisplay->bAnlgEnable)
    {
        /* RM pairs with IT */
        /* Change Tracking_Range to default. */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(RM_0_CONTROL, TRACKING_RANGE)),
            BCHP_FIELD_DATA(RM_0_CONTROL, TRACKING_RANGE, BVDC_P_TRACKING_RANGE_DEFAULT),
            BCHP_RM_0_CONTROL + hDisplay->stAnlgChan_0.ulRmRegOffset);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_RM_0_SKIP_REPEAT_CONTROL + hDisplay->stAnlgChan_0.ulRmRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(RM_0_SKIP_REPEAT_CONTROL, ENABLE, 0);
    }

    if(hDisplay->stDviChan.bEnable)
    {
        /* Change Tracking_Range to default. */
        BVDC_P_RD_MOD_WR_RUL(pList->pulCurrent,
            ~(BCHP_MASK(HDMI_RM_CONTROL, TRACKING_RANGE)),
            BCHP_FIELD_DATA(HDMI_RM_CONTROL, TRACKING_RANGE, BVDC_P_TRACKING_RANGE_DEFAULT),
            BCHP_HDMI_RM_CONTROL + hDisplay->stDviChan.ulDvpRegOffset);

        *pList->pulCurrent++ = BRDC_OP_IMM_TO_REG();
        *pList->pulCurrent++ = BRDC_REGISTER(BCHP_HDMI_RM_SKIP_REPEAT_CONTROL + hDisplay->stDviChan.ulDvpRegOffset);
        *pList->pulCurrent++ =
            BCHP_FIELD_DATA(HDMI_RM_SKIP_REPEAT_CONTROL, ENABLE, 0);
    }
}


void BVDC_P_Display_Alignment_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList )
{
    int32_t lDeltaTs = 0;
    BVDC_Display_Handle hTargetDisplay;
    uint32_t ulCount = 0;

    switch(hDisplay->eAlignmentState)
    {
        case BVDC_P_Alignment_eInactive:
            break;

        case BVDC_P_Alignment_eWaitTimeStamp:
            if ((hDisplay->eTimeStampState == BVDC_P_TimeStamp_eAvail) &&
                (hDisplay->stCurInfo.hTargetDisplay->eTimeStampState == BVDC_P_TimeStamp_eAvail))
            {
                hTargetDisplay = hDisplay->stCurInfo.hTargetDisplay;
                BDBG_OBJECT_ASSERT(hTargetDisplay, BVDC_DSP);
                if (BVDC_P_Display_CalculateTimeGap_isr(hDisplay, hTargetDisplay, &lDeltaTs))
                {
                    /* Invalid timestamp. We have to start over again.
                     * This dirty bit will be picked up at next Vysnc.
                     */
                    hDisplay->stCurInfo.stDirty.stBits.bAlignment = BVDC_P_DIRTY;
                }
                else
                {
                    BDBG_MSG(("Display[%d] and TargetDisplay[%d] time gap %d, %d us",
                        hDisplay->eId, hTargetDisplay->eId, lDeltaTs, BVDC_P_ABS(lDeltaTs)/27));

                    if (BVDC_P_ABS(lDeltaTs) < BVDC_P_DISPLAY_ALIGNMENT_THRESHOLD)
                    {
                        /* no adjustment needed */
                        BDBG_MSG(("Display[%d] and TargetDisplay[%d] time gap %d within the threshold",
                                    hDisplay->eId, hTargetDisplay->eId, lDeltaTs));
                        hDisplay->eAlignmentState = BVDC_P_Alignment_eDone;
                    }
                    else
                    {
                        /* Start the adjustment process */
                        BVDC_P_Display_StartAlignAdjustment_isr(hDisplay, pList, lDeltaTs);
                        hDisplay->eAlignmentState = BVDC_P_Alignment_eActive;
                        hDisplay->bAlignAdjusting = true;
                    }
                }
            }
            break;

        case BVDC_P_Alignment_eActive:
            /* Poll SKIP_REPEAT_NUM to see if the adjustment finishes.
             */
            if(hDisplay->stDviChan.bEnable)
            {
                ulCount = BREG_Read32_isr(hDisplay->hVdc->hRegister,
                    BCHP_HDMI_RM_SKIP_REPEAT_NUMBER + hDisplay->stDviChan.ulDvpRegOffset) &
                    BCHP_HDMI_RM_SKIP_REPEAT_NUMBER_COUNT_MASK;
            }
            else if(hDisplay->bAnlgEnable)
            {
                /* RM pairs with IT */
                ulCount = BREG_Read32_isr(hDisplay->hVdc->hRegister,
                    BCHP_RM_0_SKIP_REPEAT_NUMBER + hDisplay->stAnlgChan_0.ulRmRegOffset) &
                    BCHP_RM_0_SKIP_REPEAT_NUMBER_COUNT_MASK;
            }

            if (ulCount == 0)
            {
                BVDC_P_Display_StopAlignAdjustment_isr(hDisplay, pList);
                hDisplay->eAlignmentState = BVDC_P_Alignment_eDone;
                hDisplay->bAlignAdjusting = false;
            }
            break;

        case BVDC_P_Alignment_eDone:
            /* A BVDC_P_Alignment_eDone state is created so that
             * we can add watch-dog later to maintain the alignment
             * even after the adjustment is done.
             */
        default:
            break;
    }

    return;
}

static void BVDC_P_Display_Apply_TimeStamp_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
#if (BVDC_P_USE_RDC_TIMESTAMP)
    BSTD_UNUSED(pList);
#endif

    switch (hDisplay->eTimeStampState )
    {
        case BVDC_P_TimeStamp_eStart:
#if (!BVDC_P_USE_RDC_TIMESTAMP)
            BVDC_P_Display_SnapshotTimer_isr(hDisplay, pList);
#endif
            hDisplay->eTimeStampPolarity = eFieldPolarity;
            hDisplay->eTimeStampState = BVDC_P_TimeStamp_eRul;
            break;

        case BVDC_P_TimeStamp_eRul:
            hDisplay->eTimeStampState = BVDC_P_TimeStamp_eAvail;
            hDisplay->stCurInfo.stDirty.stBits.bTimeStamp = BVDC_P_CLEAN;
            break;

        case BVDC_P_TimeStamp_eAvail:
        default:
            hDisplay->stCurInfo.stDirty.stBits.bTimeStamp = BVDC_P_CLEAN;
            break;
    }

    return;
}

/**************** Display alignment event handlers **************/
static BERR_Code BVDC_P_Display_Validate_Alignment_Setting
    ( BVDC_Display_Handle              hDisplay )
{

    BVDC_Display_Handle hTargetDisplay = hDisplay->stNewInfo.hTargetDisplay;


    if (hTargetDisplay && ((hTargetDisplay->eAlignmentState == BVDC_P_Alignment_eActive) ||
        (hTargetDisplay->eItState != BVDC_P_ItState_eActive)))
    {
        BDBG_ERR(("Display %d is not a valid alignment target: alignment state %d, IT state %d",
            hTargetDisplay->eId, hTargetDisplay->eAlignmentState, hTargetDisplay->eItState));

        return BERR_TRACE(BVDC_ERR_INVALID_TARGET_DISPLAY);
    }

    /* Check alignment chain */
    if(hTargetDisplay && hTargetDisplay->stNewInfo.hTargetDisplay)
    {
        BDBG_ERR(("Chained display alignment is not allowed! Display%d->Display%d->Display%d",
            hDisplay->eId, hTargetDisplay->eId,
            hTargetDisplay->stNewInfo.hTargetDisplay->eId));
        return BERR_TRACE(BVDC_ERR_INVALID_MODE_PATH);
    }

    /* check timebase */
    if(hTargetDisplay && hDisplay->stNewInfo.eTimeBase != hTargetDisplay->stNewInfo.eTimeBase)
    {
        BDBG_ERR(("Bad alignment config: Display%d's timebase(%d) != target display%d's timebase(%d)!",
            hDisplay->eId, hDisplay->stNewInfo.eTimeBase,
            hTargetDisplay->eId, hTargetDisplay->stNewInfo.eTimeBase));
        return BERR_TRACE(BVDC_ERR_INVALID_MODE_PATH);
    }

    /* check default display frame rate */
    if(hTargetDisplay &&
       ((hDisplay->hVdc->stSettings.eDisplayFrameRate == BAVC_FrameRateCode_e60) ||
        (hDisplay->hVdc->stSettings.eDisplayFrameRate == BAVC_FrameRateCode_e30) ||
        (hDisplay->hVdc->stSettings.eDisplayFrameRate == BAVC_FrameRateCode_e24)))
    {
        BDBG_ERR(("Bad VDC default display rate config for VEC locking: %d!",
            hDisplay->hVdc->stSettings.eDisplayFrameRate));
        return BERR_TRACE(BVDC_ERR_INVALID_FRAMERATE_USE);
    }

    /* currently supported vertical frequency pair for alignment */
    if (hTargetDisplay &&
        (!((hDisplay->stNewInfo.ulVertFreq == hTargetDisplay->stNewInfo.ulVertFreq) ||
         (hDisplay->stNewInfo.ulVertFreq == 2997 && hTargetDisplay->stNewInfo.ulVertFreq == 5994) ||
         (hDisplay->stNewInfo.ulVertFreq == 2397 && hTargetDisplay->stNewInfo.ulVertFreq == 5994) ||
         (hDisplay->stNewInfo.ulVertFreq == 2500 && hTargetDisplay->stNewInfo.ulVertFreq == 5000))))
    {
        BDBG_ERR(("Vertical frequency not supported for alignment. Display%d %d, Target display%d %d",
            hDisplay->eId, hDisplay->stNewInfo.ulVertFreq,
            hTargetDisplay->eId, hTargetDisplay->stNewInfo.ulVertFreq));
        return BERR_TRACE(BVDC_ERR_INVALID_FRAMERATE_USE);
    }

    return BERR_SUCCESS;
}

static void BVDC_P_Display_Copy_Alignment_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{

    hDisplay->stCurInfo.hTargetDisplay = hDisplay->stNewInfo.hTargetDisplay;
    hDisplay->stCurInfo.stAlignCfg     = hDisplay->stNewInfo.stAlignCfg;

    return;
}

static void BVDC_P_Display_Apply_Alignment_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )

{
    BVDC_P_DisplayInfo *pCurInfo;

    pCurInfo = &hDisplay->stCurInfo;

    if(pCurInfo->hTargetDisplay)
    {
        /* Set target display bTimeStamp dirty bit so that
         * it can build RUL to take a snapshot of timer.
         */
        pCurInfo->hTargetDisplay->eTimeStampState = BVDC_P_TimeStamp_eStart;
        pCurInfo->hTargetDisplay->stCurInfo.stDirty.stBits.bTimeStamp = BVDC_P_DIRTY;

        /*
         * The display itself builds RUL to take a snapshot of timer clock as well.
         */
#if (!BVDC_P_USE_RDC_TIMESTAMP)
        BVDC_P_Display_SnapshotTimer_isr(hDisplay, pList);
#endif
        hDisplay->eTimeStampPolarity = eFieldPolarity;
        hDisplay->eTimeStampState = BVDC_P_TimeStamp_eRul;
        hDisplay->eAlignmentState = BVDC_P_Alignment_eWaitTimeStamp;
        pCurInfo->stDirty.stBits.bTimeStamp = BVDC_P_DIRTY;
    }
    else
    {
        /* Disable alignment process */
        if ((hDisplay->eAlignmentState == BVDC_P_Alignment_eWaitTimeStamp) ||
            (hDisplay->eAlignmentState == BVDC_P_Alignment_eActive))
        {
            BVDC_P_Display_StopAlignAdjustment_isr(hDisplay, pList);
        }

        hDisplay->eAlignmentState = BVDC_P_Alignment_eInactive;
    }

    hDisplay->stCurInfo.stDirty.stBits.bAlignment = BVDC_P_CLEAN;

    return;
}


/**************** HDMI XvYcc event handlers **************/
static void BVDC_P_Display_Copy_HdmiXvYcc_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    hDisplay->stCurInfo.bXvYcc = hDisplay->stNewInfo.bXvYcc;

    return;
}

static void BVDC_P_Display_Apply_HdmiXvYcc_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BVDC_P_Output eOutputCS;

    BSTD_UNUSED(eFieldPolarity);

    if (hDisplay->stAnlgChan_0.bEnable)
    {
        /* Reload analog path CSC settings to undo the
         * XvYcc color space change made at compositor level.
         */
        eOutputCS = hDisplay->stCurInfo.eAnlg_0_OutputColorSpace;
        BVDC_P_Vec_Build_CSC_SRC_SM_isr(hDisplay, &hDisplay->stAnlgChan_0, eOutputCS, pList);
    }

    if (hDisplay->stAnlgChan_1.bEnable)
    {
        eOutputCS = hDisplay->stCurInfo.eAnlg_1_OutputColorSpace;
        BVDC_P_Vec_Build_CSC_SRC_SM_isr(hDisplay, &hDisplay->stAnlgChan_1, eOutputCS, pList);
    }

    hDisplay->stCurInfo.stDirty.stBits.bHdmiXvYcc = BVDC_P_CLEAN;

    return;
}


/**************** HDMI sync-only event handlers **************/
static BERR_Code BVDC_P_Display_Validate_HdmiSyncOnly_Setting
    ( BVDC_Display_Handle              hDisplay )
{
    if(!hDisplay->stNewInfo.bEnableHdmi)
    {
        BDBG_ERR(("Display [%d] doesn't have DVI channel enabled", hDisplay->eId));
        return BERR_TRACE(BVDC_ERR_INVALID_HDMI_PATH);
    }

    return BERR_SUCCESS;
}

static void BVDC_P_Display_Apply_HdmiSyncOnly_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BSTD_UNUSED(pList);
    BSTD_UNUSED(eFieldPolarity);

    if(!hDisplay->stCurInfo.bEnableHdmi)
    {
        goto done;
    }

    /* The new setting is achieved through a mute settings.
     */
done:
    hDisplay->stCurInfo.stDirty.stBits.bHdmiSyncOnly = BVDC_P_CLEAN;

    return;
}

/**************** HDMI settings event handlers **************/
static BERR_Code BVDC_P_Display_Validate_Hdmi_Setting
    ( BVDC_Display_Handle              hDisplay )
{
    BERR_Code err;
    if ((err = BVDC_P_Display_Validate_DviChan(hDisplay)) != BERR_SUCCESS)
        return BERR_TRACE(err);

    return BERR_SUCCESS;
}

static void BVDC_P_Display_Copy_Hdmi_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    if(hDisplay->stCurInfo.stHdmiSettings.stSettings.eColorComponent != hDisplay->stNewInfo.stHdmiSettings.stSettings.eColorComponent)
    {
        BDBG_MSG(("dsp[%d] Color Component change %d -> %d", hDisplay->eId,
            hDisplay->stCurInfo.stHdmiSettings.stSettings.eColorComponent,
            hDisplay->stNewInfo.stHdmiSettings.stSettings.eColorComponent));
    }

    if(hDisplay->stCurInfo.stHdmiSettings.eHDMIFormat != hDisplay->stNewInfo.stHdmiSettings.eHDMIFormat ||
       hDisplay->stCurInfo.bHdmiFmt    != hDisplay->stNewInfo.bHdmiFmt)
    {
        BDBG_MSG(("dsp[%d] HDMI Format change %s -> %s: %s", hDisplay->eId,
            (hDisplay->stCurInfo.pHdmiFmtInfo) ? hDisplay->stCurInfo.pHdmiFmtInfo->pchFormatStr : "Unknown",
            (hDisplay->stNewInfo.pHdmiFmtInfo) ? hDisplay->stNewInfo.pHdmiFmtInfo->pchFormatStr : "Unknown",
            (hDisplay->stNewInfo.bHdmiFmt) ? "Matched" : "Not Matched"));
    }
    if(hDisplay->stCurInfo.bHdmiRmd != hDisplay->stNewInfo.bHdmiRmd)
    {
        BDBG_MSG(("dsp[%d] RM change %s -> %s", hDisplay->eId,
            (hDisplay->stCurInfo.bHdmiRmd) ? "DTG RM" : "IT RM",
            (hDisplay->stNewInfo.bHdmiRmd) ? "DTG RM" : "IT RM"));
    }

    hDisplay->stCurInfo.stHdmiSettings  = hDisplay->stNewInfo.stHdmiSettings;
    hDisplay->stCurInfo.pHdmiFmtInfo    = hDisplay->stNewInfo.pHdmiFmtInfo ;
    hDisplay->stCurInfo.bHdmiFmt        = hDisplay->stNewInfo.bHdmiFmt;
    hDisplay->stCurInfo.bHdmiRmd        = hDisplay->stNewInfo.bHdmiRmd;

    return;
}

static void BVDC_P_Display_Apply_Hdmi_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BSTD_UNUSED(pList);
    BSTD_UNUSED(eFieldPolarity);
    hDisplay->stCurInfo.stDirty.stBits.bHdmiSettings = BVDC_P_CLEAN;

    return;
}

/**************** HDMI RM settings event handlers **************/
static void BVDC_P_Display_Apply_Hdmi_RM_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    if(hDisplay->stDviChan.bEnable || hDisplay->stCurInfo.bEnableHdmi)
    {
        BVDC_P_Vec_Build_DVI_RM_isr(hDisplay, &hDisplay->stDviChan, pList, true);
    }
    hDisplay->stCurInfo.stDirty.stBits.bHdmiRmSettings = BVDC_P_CLEAN;

    BSTD_UNUSED(eFieldPolarity);
    return;
}

/**************** 3D settings event handlers **************/
static void BVDC_P_Display_Copy_3D_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    hDisplay->stCurInfo.eOrientation = hDisplay->stNewInfo.eOrientation;
    hDisplay->stCurInfo.e3dSrcBufSel = hDisplay->stNewInfo.e3dSrcBufSel;
}

static void BVDC_P_Display_Apply_3D_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    if(hDisplay->bAnlgEnable ||   /* if analog master */
       (!hDisplay->bAnlgEnable &&  /* or anlog slave with DACs */
       (hDisplay->stAnlgChan_0.bEnable || hDisplay->stAnlgChan_1.bEnable)))
    {
        BVDC_P_Vec_Build_It3D_isr(hDisplay, pList);
    }

    BSTD_UNUSED(eFieldPolarity);
    hDisplay->stCurInfo.stDirty.stBits.b3DSetting = BVDC_P_CLEAN;
}

/**************** VF filter settings event handlers **************/
static BERR_Code BVDC_P_Display_Validate_VfFilter_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    BVDC_DisplayOutput   eDisplayOutput;
    BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;
    BERR_Code            eErr = BERR_SUCCESS;

    /* validate VF filter settings */
    if (hDisplay->stAnlgChan_0.bEnable)
    {
        if (BVDC_P_DISP_IS_COMPONENT (pCurInfo->eAnlg_0_OutputColorSpace))
            eDisplayOutput = BVDC_DisplayOutput_eComponent;
        else
            eDisplayOutput = BVDC_DisplayOutput_eComposite;
        if ((eErr = BVDC_P_GetVfFilterSumOfTapsBits_isr (
            pCurInfo, eDisplayOutput, NULL, NULL)) != BERR_SUCCESS)
        {
            goto fail;
        }
    }

    if (hDisplay->stAnlgChan_1.bEnable)
    {
        if (BVDC_P_DISP_IS_COMPONENT (pCurInfo->eAnlg_1_OutputColorSpace))
            eDisplayOutput = BVDC_DisplayOutput_eComponent;
        else
            eDisplayOutput = BVDC_DisplayOutput_eComposite;
        if ((eErr = BVDC_P_GetVfFilterSumOfTapsBits_isr (
            pCurInfo, eDisplayOutput, NULL, NULL)) != BERR_SUCCESS)
        {
            goto fail;
        }
    }

    return BERR_SUCCESS;

fail:
    BDBG_ERR(("Inconsistent SUM_OF_TAPS in user VF filters"));
    return BERR_TRACE(eErr);
}

static void BVDC_P_Display_Copy_VfFilter_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    int i;

    for (i = 0; i < BVDC_P_VEC_CH_NUM; i++)
    {
        hDisplay->stCurInfo.abUserVfFilterCo[i] = hDisplay->stNewInfo.abUserVfFilterCo[i];
        hDisplay->stCurInfo.abUserVfFilterCvbs[i] = hDisplay->stNewInfo.abUserVfFilterCvbs[i];
        hDisplay->stCurInfo.aulUserVfFilterCoSumOfTaps[i] = hDisplay->stNewInfo.aulUserVfFilterCoSumOfTaps[i];
        hDisplay->stCurInfo.aulUserVfFilterCvbsSumOfTaps[i] = hDisplay->stNewInfo.aulUserVfFilterCvbsSumOfTaps[i];
        BKNI_Memcpy(hDisplay->stCurInfo.aaulUserVfFilterCo[i], hDisplay->stNewInfo.aaulUserVfFilterCo[i], BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
        BKNI_Memcpy(hDisplay->stCurInfo.aaulUserVfFilterCvbs[i], hDisplay->stNewInfo.aaulUserVfFilterCvbs[i], BVDC_P_CHROMA_TABLE_SIZE * sizeof(uint32_t));
    }

    return;
}

static void BVDC_P_Display_Apply_VfFilter_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BVDC_P_DisplayInfo  *pCurInfo = &hDisplay->stCurInfo;
    BSTD_UNUSED(eFieldPolarity);

    /* Update VF filter settings */
    if (hDisplay->stAnlgChan_0.bEnable)
    {
        BVDC_P_Vec_Build_VF_isr(hDisplay, &hDisplay->stAnlgChan_0,
            pCurInfo->eAnlg_0_OutputColorSpace, pList);
    }

    if (hDisplay->stAnlgChan_1.bEnable)
    {
        BVDC_P_Vec_Build_VF_isr(hDisplay, &hDisplay->stAnlgChan_1,
            pCurInfo->eAnlg_1_OutputColorSpace, pList);
    }

    hDisplay->stCurInfo.stDirty.stBits.bVfFilter = BVDC_P_CLEAN;

    return;
}


/**************** Output mute settings event handlers **************/
static void BVDC_P_Display_Copy_OutputMute_Setting_isr
    ( BVDC_Display_Handle              hDisplay )
{
    BKNI_Memcpy(hDisplay->stCurInfo.abOutputMute, hDisplay->stNewInfo.abOutputMute, sizeof (hDisplay->stCurInfo.abOutputMute));

    return;
}

static void BVDC_P_Display_Apply_OutputMute_Setting_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BVDC_P_Output       eOutputCS;

    if (hDisplay->stAnlgChan_0.bEnable)
    {
        eOutputCS = hDisplay->stCurInfo.eAnlg_0_OutputColorSpace;
        BVDC_P_Vec_Build_CSC_SRC_SM_isr(hDisplay, &hDisplay->stAnlgChan_0, eOutputCS, pList);
    }

    if (hDisplay->stAnlgChan_1.bEnable)
    {
        eOutputCS = hDisplay->stCurInfo.eAnlg_1_OutputColorSpace;
        BVDC_P_Vec_Build_CSC_SRC_SM_isr(hDisplay, &hDisplay->stAnlgChan_1, eOutputCS, pList);
    }

    if(hDisplay->stCurInfo.bEnableHdmi)
    {
        BVDC_P_Vec_Build_DVI_CSC_isr(hDisplay, &hDisplay->stDviChan, pList );
    }

#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
    if (hDisplay->st656Chan.bEnable)
    {
        BVDC_P_Vec_Build_656_CSC_isr(hDisplay, pList );
    }
#endif

    hDisplay->stCurInfo.stDirty.stBits.bOutputMute = BVDC_P_CLEAN;

    BSTD_UNUSED(eFieldPolarity);

    return;
}


/**************** Misc Ctrl settings event handlers **************/
static void BVDC_P_Display_Copy_bMiscCtrl_isr
    ( BVDC_Display_Handle              hDisplay )
{
    hDisplay->stCurInfo.bArtificialVsync   = hDisplay->stNewInfo.bArtificialVsync;
    hDisplay->stCurInfo.ulArtificialVsyncRegAddr = hDisplay->stNewInfo.ulArtificialVsyncRegAddr;
    hDisplay->stCurInfo.ulArtificialVsyncMask = hDisplay->stNewInfo.ulArtificialVsyncMask;
    hDisplay->stCurInfo.eBarDataMode = hDisplay->stNewInfo.eBarDataMode;
    hDisplay->stCurInfo.eBarDataType = hDisplay->stNewInfo.eBarDataType;
    hDisplay->stCurInfo.ulTopLeftBarData    = hDisplay->stNewInfo.ulTopLeftBarData;
    hDisplay->stCurInfo.ulBotRightBarData   = hDisplay->stNewInfo.ulBotRightBarData;

    return;
}

static void BVDC_P_Display_Apply_bMiscCtrl_isr
    ( BVDC_Display_Handle              hDisplay,
      BVDC_P_ListInfo                 *pList,
      BAVC_Polarity                    eFieldPolarity )
{
    BSTD_UNUSED(pList);
    BSTD_UNUSED(eFieldPolarity);
    hDisplay->stCurInfo.stDirty.stBits.bMiscCtrl = BVDC_P_CLEAN;
    return;
}


/* This table has the handlers for all the display events.
 *
 * Note, it is critical to keep the position of each hander match with
 * the position of its associated dirty bit. The position of dirty bit
 * will be used to index this table to obtain the handlers.
 *
 */
const BVDC_Display_EventHandler BVDC_P_astDisplayEventHndlTbl[] =
{
    /* New analog channel 0    event                               - index 0 */
    {
        NULL,
        NULL,
        BVDC_P_Display_Apply_Chan0_isr
    },

    /* New analog channel 1    event                               - index 1 */
    {
        NULL,
        NULL,
        BVDC_P_Display_Apply_Chan1_isr
    },

    /* New video timing format event                               - index 2 */
    {
        BVDC_P_Display_Validate_VideoFormat,
        BVDC_P_Display_Copy_VideoFormat_isr,
        BVDC_P_Display_Apply_VideoFormat_isr
    },

    /* New video analog copy protection event                       - index 3 */
    {
        BVDC_P_Display_Validate_Acp,
        BVDC_P_Display_Copy_Acp_isr,
        BVDC_P_Display_Apply_Acp_isr
    },

    /* Change 3D setting                                           - index 4 */
    {
        /* Changing 3D setting */
        NULL,
        BVDC_P_Display_Copy_3D_Setting_isr,
        BVDC_P_Display_Apply_3D_Setting_isr
    },

    /* Setting DAC event                                           - index 5 */
    {
        NULL, /* Validated together with video format */
        BVDC_P_Display_Copy_DAC_Setting_isr,
        BVDC_P_Display_Apply_DAC_Setting_isr
    },

    /* Setting time base event                                     - index 6 */
    {
        NULL,
        BVDC_P_Display_Copy_TimeBase_Setting_isr,
        BVDC_P_Display_Apply_TimeBase_Setting_isr
    },

    /* New callback mask event                                     - index 7 */
    {
        NULL,
        BVDC_P_Display_Copy_Callback_Setting_isr,
        BVDC_P_Display_Apply_Callback_Setting_isr
    },

    /* New callback function event                                 - index 8 */
    {
        NULL,
        BVDC_P_Display_Copy_CallbackFunc_Setting_isr,
        BVDC_P_Display_Apply_CallbackFunc_Setting_isr
    },

    /* Trimming width evet                                         - index 9 */
    {
        /* Internal event from compositor. So no validation and copy here. Settings
         * go to current info directly
         */
        NULL,
        NULL,
        BVDC_P_Display_Apply_TrimmingWidth_Setting_isr
    },

    /* Input color space change event                              - index 10 */
    {
        /* Internal event from compositor. So no validation and copy here. Settings
         * go to current info directly
         */
        NULL,
        NULL,
        BVDC_P_Display_Apply_InputColorSpace_Setting_isr
    },

    /* Source frame rate change event                              - index 11 */
    {
        /* Internal event from compositor. So no validation and copy here. Settings
         * go to current info directly
         */
        BVDC_P_Display_Validate_SrcFrameRate_Setting,
        BVDC_P_Display_Copy_SrcFrameRate_Setting_isr,
        BVDC_P_Display_Apply_SrcFrameRate_Setting_isr
    },

#if (BVDC_P_SUPPORT_RFM_OUTPUT != 0)
    /* RFM configuration event                                     - index 12 */
    {
        /* Enable/disable RFM ouput.
         */
        BVDC_P_Display_Validate_Rfm_Setting,
        BVDC_P_Display_Copy_Rfm_Setting_isr,
        BVDC_P_Display_Apply_Rfm_Setting_isr
    },
#endif

    /* Enable/disable HDMI event                                   - index 13 */
    {
        /* Enable/disable HDMI ouput.
         */
        BVDC_P_Display_Validate_Hdmi_Config,
        BVDC_P_Display_Copy_Hdmi_Config_isr,
        BVDC_P_Display_Apply_Hdmi_Config_isr
    },

    /* HDMI output CSC matrix setting event                        - index 14 */
    {
        /* Set new HDMI output CSC matrix.
         */
        NULL,
        BVDC_P_Display_Copy_HdmiCsc_Setting_isr,
        BVDC_P_Display_Apply_HdmiCsc_Setting_isr
    },

#if (BVDC_P_SUPPORT_ITU656_OUT != 0)
    /* Enable/disable 656 event                                    - index 15 */
    {
        /* Enable/disable 656 ouput.
         */
        BVDC_P_Display_Validate_656_Setting,
        BVDC_P_Display_Copy_656_Setting_isr,
        BVDC_P_Display_Apply_656_Setting_isr
    },
#endif

    /* Compenent MPAA setting event                                - index 16 */
    {
        /* Enable/disable MPAA.
         */
        BVDC_P_Display_Validate_CompMpaa_Setting,
        BVDC_P_Display_Copy_CompMpaa_Setting_isr,
        BVDC_P_Display_Apply_CompMpaa_Setting_isr
    },

    /* HDMI setting event                                          - index 17 */
    {
        /* Enable/disable MPAA.
         */
        BVDC_P_Display_Validate_HdmiMpaa_Setting,
        BVDC_P_Display_Copy_HdmiMpaa_Setting_isr,
        BVDC_P_Display_Apply_HdmiMpaa_Setting_isr
    },

    /* Snapshot timestamp event                                    - index 18 */
    {
        /* Take a snapshot of temstamp.
         */
        NULL,
        NULL,
        BVDC_P_Display_Apply_TimeStamp_Setting_isr
    },

    /* display alignment event                                     - index 19 */
    {
        /* Turn on/off alignment.
         */
        BVDC_P_Display_Validate_Alignment_Setting,
        BVDC_P_Display_Copy_Alignment_Setting_isr,
        BVDC_P_Display_Apply_Alignment_Setting_isr
    },

    /* HDMI XvYcc event                                            - index 20 */
    {
        /* New HDMI XvYcc settings */
        NULL,
        BVDC_P_Display_Copy_HdmiXvYcc_Setting_isr,
        BVDC_P_Display_Apply_HdmiXvYcc_Setting_isr
    },

    /* HDMI sync-only event                                        - index 21 */
    {
        /* Turn on/off HDMI sync-only feature */
        BVDC_P_Display_Validate_HdmiSyncOnly_Setting,
        NULL,
        BVDC_P_Display_Apply_HdmiSyncOnly_Setting_isr
    },

    /* HDMI settings event                                         - index 22 */
    {
        /* Changing Hdmi settings */
        BVDC_P_Display_Validate_Hdmi_Setting,
        BVDC_P_Display_Copy_Hdmi_Setting_isr,
        BVDC_P_Display_Apply_Hdmi_Setting_isr
    },

    /* HDMI RM       event                                         - index 23 */
    {
        /* Changing Hdmi RM settings */
        NULL,
        NULL,
        BVDC_P_Display_Apply_Hdmi_RM_Setting_isr
    },

    /* display aspect ratio event                                  - index 24 */
    {
        /* asp ratio or aspR canvas clip settings */
        BVDC_P_Display_Validate_AspRatio_Setting,
        BVDC_P_Display_Copy_AspRatio_Setting_isr,
        BVDC_P_Display_Apply_AspRatio_Setting_isr
    },

#if (BVDC_P_SUPPORT_STG != 0)
    /* Enable/disable Stg event                                    - index 25 */
    {
        /* Enable/disable Stg ouput.
         */
        BVDC_P_Display_Validate_Stg_Setting,
        BVDC_P_Display_Copy_Stg_Setting_isr,
        BVDC_P_Display_Apply_Stg_Setting_isr
    },
#endif

    /* Vf filter                                                   - index 26 */
    {
        BVDC_P_Display_Validate_VfFilter_Setting_isr,
        BVDC_P_Display_Copy_VfFilter_Setting_isr,
        BVDC_P_Display_Apply_VfFilter_Setting_isr
    },

    /* Enable/disable output mute event.                           - index 27 */
    {
        NULL,
        BVDC_P_Display_Copy_OutputMute_Setting_isr,
        BVDC_P_Display_Apply_OutputMute_Setting_isr
    },

    /* Misc Ctrl                                                   - index 28 */
    {
        NULL,
        BVDC_P_Display_Copy_bMiscCtrl_isr,
        BVDC_P_Display_Apply_bMiscCtrl_isr
    },

    /* the following place holders are for coverity check only */
    /* to silent coverity defects */
#if (BVDC_P_SUPPORT_RFM_OUTPUT == 0)
    /* Extra Place holder                                          - index 12 */
    {
        /* Empty */
        NULL,
        NULL,
        NULL
    },
#endif

#if (BVDC_P_SUPPORT_ITU656_OUT == 0)
    /* Extra Place holder                                          - index 15 */
    {
        /* Empty */
        NULL,
        NULL,
        NULL
    },
#endif

#if (BVDC_P_SUPPORT_STG == 0)
    /* Extra Place holder                                          - index 25 */
    {
        /* Empty */
        NULL,
        NULL,
        NULL
    },
#endif

    /* Extra Place holder (Coverity)                               - index 29 */
    {
        /* Empty */
        NULL,
        NULL,
        NULL
    },

    /* Extra Place holder (Coverity)                               - index 30 */
    {
        /* Empty */
        NULL,
        NULL,
        NULL
    },

    /* Extra Place holder (Coverity)                               - index 31 */
    {
        /* Empty */
        NULL,
        NULL,
        NULL
    },
};
const unsigned int BVDC_P_astDisplayEventHndlTblSize =
    sizeof(BVDC_P_astDisplayEventHndlTbl) /
    sizeof(BVDC_P_astDisplayEventHndlTbl[0]);

/* End of File */
