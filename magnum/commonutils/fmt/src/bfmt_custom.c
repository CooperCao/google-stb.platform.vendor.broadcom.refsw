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
 *   Custom Video format info file; it contains raster size, front porch,
 * back porch etc format info, and DVO microcode as well as rate manager
 * settings.
 *  NOTE: This file is to be replace by customer specific timing data!
 *  following are for reference board only.
 *
 ***************************************************************************/
#include "bstd.h"
#include "bfmt.h"
#include "bkni.h"
#include "bfmt_custom.h"

static const uint32_t s_vec_tb_noprim_dvim1920x1080p_60hz_bvb_input_bss_wxga[] =
{
     0x0064A001, /*  64 */
     0x00650009, /*  65 */
     0x00656001, /*  66 */
     0x0065C00F, /*  67 */
     0x00668300, /*  68 */
     0x0065C007, /*  69 */
     0x00662001, /*  70 */
     0x00840000, /*  71 */
     0x00000000, /*  72 */
     0x00000000, /*  73 */
     0x00284038, /*  74 */
     0x002D400C, /*  75 */
     0x0024406A, /*  76 */
     0x0034554B, /*  77 */
     0x00000000, /*  78 */
     0x00000000, /*  79 */
     0x00244038, /*  80 */
     0x0025400C, /*  81 */
     0x0024406A, /*  82 */
     0x0034554B, /*  83 */
     0x00000000, /*  84 */
     0x00000000, /*  85 */
     0x00244038, /*  86 */
     0x0021400C, /*  87 */
     0x0020406A, /*  88 */
     0x0030554B, /*  89 */
     0x00000000, /*  90 */
     0x00000000, /*  91 */
     0x00204038, /*  92 */
     0x0021400C, /*  93 */
     0x0020406A, /*  94 */
     0x0030554B, /*  95 */
     0x00000000, /*  96 */
     0x00000000, /*  97 */
     0x00204038, /*  98 */
     0x0021400C, /*  99 */
     0x0020406A, /* 100 */
     0x00305540, /* 101 */
     0x00000000, /* 102 */
     0x00000000, /* 103 */
     0x00202038, /* 104 */
     0x0021200C, /* 105 */
     0x0020206A, /* 106 */
     0x0032354B, /* 107 */
     0x00000000, /* 108 */
     0x00000000, /* 109 */
     0x00202038, /* 110 */
     0x0021200C, /* 111 */
     0x0020206A, /* 112 */
     0x00323540, /* 113 */
     0x00000000, /* 114 */
     0x00000000, /* 115 */
     0x00000000, /* 116 */
     0x00000000, /* 117 */
     0x00000000, /* 118 */
     0x00000000, /* 119 */
     0x00000000, /* 120 */
     0x00000000, /* 121 */
     0x00000000, /* 122 */
     0x00000000, /* 123 */
     0x00000000, /* 124 */
     0x00000000, /* 125 */
     0x00072507, /* 126 */
     0x005D4D0E, /* 127 */
};

static const uint32_t s_vec_tb_noprim_dvim1920x1080p_60hz_bvb_input_bss_fhd[] =
{
    0x0064A001, /*  64 */
    0x00650004, /*  65 */
    0x00656001, /*  66 */
    0x0065C023, /*  67 */
    0x00668438, /*  68 */
    0x0065C003, /*  69 */
    0x00662001, /*  70 */
    0x00840000, /*  71 */
    0x00000000, /*  72 */
    0x00000000, /*  73 */
    0x002840C8, /*  74 */
    0x002D402C, /*  75 */
    0x00244024, /*  76 */
    0x00345775, /*  77 */
    0x00000000, /*  78 */
    0x00000000, /*  79 */
    0x002440C8, /*  80 */
    0x0025402C, /*  81 */
    0x00244024, /*  82 */
    0x00345775, /*  83 */
    0x00000000, /*  84 */
    0x00000000, /*  85 */
    0x002440C8, /*  86 */
    0x0021402C, /*  87 */
    0x00204024, /*  88 */
    0x00305775, /*  89 */
    0x00000000, /*  90 */
    0x00000000, /*  91 */
    0x002040C8, /*  92 */
    0x0021402C, /*  93 */
    0x00204024, /*  94 */
    0x00305775, /*  95 */
    0x00000000, /*  96 */
    0x00000000, /*  97 */
    0x002040C8, /*  98 */
    0x0021402C, /*  99 */
    0x00204024, /* 100 */
    0x0030576A, /* 101 */
    0x00000000, /* 102 */
    0x00000000, /* 103 */
    0x002020C8, /* 104 */
    0x0021202C, /* 105 */
    0x00202024, /* 106 */
    0x00323775, /* 107 */
    0x00000000, /* 108 */
    0x00000000, /* 109 */
    0x002020C8, /* 110 */
    0x0021202C, /* 111 */
    0x00202024, /* 112 */
    0x0032376A, /* 113 */
    0x00000000, /* 114 */
    0x00000000, /* 115 */
    0x00000000, /* 116 */
    0x00000000, /* 117 */
    0x00000000, /* 118 */
    0x00000000, /* 119 */
    0x00000000, /* 120 */
    0x00000000, /* 121 */
    0x00000000, /* 122 */
    0x00000000, /* 123 */
    0x00000000, /* 124 */
    0x00000000, /* 125 */
    0x00011207, /* 126 */
    0x005D605D, /* 127 */
};

/* The following default settings are from 3563C0 1366x768 DVO bringup:
' Pixel Clock = h_total * v_total * refresh_rate
'             = 1540 * 802 * 60
'             = 74104800 Hz
' VCO = 2074934400.00 Hz
BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_FEEDBACK_PRE_DIVIDER = 1 ' (p2)
BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_INPUT_PRE_DIVIDER = 2    ' (p1)
BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_2.ndiv_mode = 1
BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_1 = &h382C24A0&
BCM3563.DVPO_0.FIFO_CTL.MASTER_OR_SLAVE_N = 1
 */
static const BFMT_P_RateInfo s_stDvoRmTbl0_wxga =
    {BFMT_PXL_148_5MHz,           2, 0x13365CEF,  99, 2,  11327,    19649,  1,  1,  1, BDBG_STRING_INLINE("74.10")}; /* 74.10 MHz */

/* The following default settings are from 3563C0 1366x768 DVO bringup:
' Pixel Clock = h_total * v_total * refresh_rate
'             = 1540 * 802 * 60 * 1000 / 1001
'             = 74030769 Hz
' VCO = 2072861532.00 Hz
BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_INPUT_PRE_DIVIDER = 2    ' (p1)
BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_2.ndiv_mode = 1
BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_1 = &h382C24A0&
BCM3563.DVPO_0.FIFO_CTL.MASTER_OR_SLAVE_N = 1
 */

static const BFMT_P_RateInfo s_stDvoRmTbl1_wxga =
    {BFMT_PXL_148_5MHz_DIV_1_001,           2, 0x13317316,  112, 2,  368,    401,  1,  1,  1, BDBG_STRING_INLINE("74.03")}; /* 74.03 MHz */

/* The following default settings are from 3563C0 1366x768 DVO bringup:
' 1366x768p @ 60Hz: 1540*802*60
' Pixel Clock = h_total * v_total * refresh_rate
'             = 1540 * 802 * 50
'             = 61754000 Hz
BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_FEEDBACK_PRE_DIVIDER = 1 ' (p2)
BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_INPUT_PRE_DIVIDER = 2    ' (p1)
BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_2.ndiv_mode = 1
BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_1 = &h382C24A0&
BCM3563.DVPO_0.FIFO_CTL.MASTER_OR_SLAVE_N = 1
 */
static const BFMT_P_RateInfo s_stDvoRmTbl2_wxga =
    {BFMT_PXL_148_5MHz_DIV_1_001,           2, 0x1002A2C7,  99, 2,  1803,    19649,  1,  1,  1, BDBG_STRING_INLINE("61.75")}; /* 61.75 MHz */

/* The following default settings are from 3563C0 1080p DVO bringup:
' 1920x1080p @ 60Hz: 2200*1125*60
' Pixel Clock = 148500000 Hz
' VCO = 2079000000 Hz
' M = 1, N = 77, P1 = 2; P2 = 1
' For RDIV=112, Rate Manager sees 9281250 Hz
' Sample_Inc = 2, NUM/DEN = 10/11
' BCM3563 Register Programming:
  BCM3563.DVPO_RM_0.RATE_RATIO.DENOMINATOR = 11
  BCM3563.DVPO_RM_0.SAMPLE_INC.NUMERATOR = 10
  BCM3563.DVPO_RM_0.SAMPLE_INC.SAMPLE_INC = 2
  BCM3563.DVPO_RM_0.OFFSET = &h13400000&
  BCM3563.DVPO_RM_0.FORMAT.SHIFT = 3
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.LINKDIV_CTRL = 0
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_R_DIV = 112
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_VCO_RANGE = 1
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_M_DIV = 1
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_FEEDBACK_PRE_DIVIDER = 1
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_INPUT_PRE_DIVIDER = 2
  BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_2.ndiv_mode = 1
  BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_1 = &h382C24A0&
  BCM3563.DVPO_0.FIFO_CTL.MASTER_OR_SLAVE_N = 1
 */
static const BFMT_P_RateInfo s_stDvoRmTbl0_fhd =
    {BFMT_PXL_148_5MHz,           1, 0x13400000,  112, 2,  10,    11,  1,  0,  1, BDBG_STRING_INLINE("148.5")}; /* 148.50000 MHz */

/* The following default settings are from 3563C0 1080p DVO bringup:
' 1920x1080p @ 59.94Hz: 2200*1125*59.94
' Pixel Clock = 148351648.351648 Hz
' VCO = 2076923076.92308 Hz
' M = 1, N = 76.9230769230769, P1 = 2; P2 = 1
' For RDIV=112, Rate Manager sees 9271978.02197802 Hz
' Sample_Inc = 2, NUM/DEN = 114/125
' BCM3563 Register Programming:
  BCM3563.DVPO_RM_0.RATE_RATIO.DENOMINATOR = 125
  BCM3563.DVPO_RM_0.SAMPLE_INC.NUMERATOR = 114
  BCM3563.DVPO_RM_0.SAMPLE_INC.SAMPLE_INC = 2
  BCM3563.DVPO_RM_0.OFFSET = &H133b13b1&
  BCM3563.DVPO_RM_0.FORMAT.SHIFT = 3
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.LINKDIV_CTRL = 0
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_R_DIV = 112
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_VCO_RANGE = 1
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_M_DIV = 1
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_FEEDBACK_PRE_DIVIDER = 1 ' (p2)
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_INPUT_PRE_DIVIDER = 2    ' (p1)
  BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_2.ndiv_mode = 1
  BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_1 = &h382C24A0&
  BCM3563.DVPO_0.FIFO_CTL.MASTER_OR_SLAVE_N = 1
 */
static const BFMT_P_RateInfo s_stDvoRmTbl1_fhd =
    {BFMT_PXL_148_5MHz_DIV_1_001, 1, 0x133B13B1, 112, 2, 114,   125,  1,  0,  1, BDBG_STRING_INLINE("148.3")}; /* 148.35 MHz */

/* The following default settings are from 3563C0 1080p DVO bringup:
' 1920x1080p @ 50Hz: 2200*1125*50
' Pixel Clock = 123750000 Hz
' VCO = 1732500000 Hz
' M = 1, N = 64.1666666666667, P1 = 2; P2 = 1
' For RDIV=112, Rate Manager sees 7734375 Hz
' Sample_Inc = 3, NUM/DEN = 27/55
' BCM3563 Register Programming:
  BCM3563.DVPO_RM_0.RATE_RATIO.DENOMINATOR = 55
  BCM3563.DVPO_RM_0.SAMPLE_INC.NUMERATOR = 27
  BCM3563.DVPO_RM_0.SAMPLE_INC.SAMPLE_INC = 3
  BCM3563.DVPO_RM_0.OFFSET = &H100aaaaa&
  BCM3563.DVPO_RM_0.FORMAT.SHIFT = 3
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.LINKDIV_CTRL = 0
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_R_DIV = 112
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_VCO_RANGE = 1
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_M_DIV = 1
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_FEEDBACK_PRE_DIVIDER = 1 ' (p2)
  BCM3563.LVDS_PHY_0.LVDS_PLL_CFG.PLL_INPUT_PRE_DIVIDER = 2    ' (p1)
  BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_2.ndiv_mode = 1
  BCM3563.LVDS_PHY_0.LVDS_PLL_CTL_1 = &h382C24A0&
  BCM3563.DVPO_0.FIFO_CTL.MASTER_OR_SLAVE_N = 1
 */
static const BFMT_P_RateInfo s_stDvoRmTbl2_fhd =
    {BFMT_PXL_148_5MHz,           1, 0x100aaaaa, 112, 3,  27,    55,  1,  0,  1, BDBG_STRING_INLINE("123.7")}; /* 123.75 MHz */


/* WARNINGS: below code are for internal use only! */
/*
 * This is a one-way runtime switch to go from WXGA to FHD.
 * Apps can call this function before BVDC_Open if they want this feature.
 * BFMT_P_SetFhd() is not part of the public API and is subject to change or
 * removal. Add an extern to your own code to use it.
*/
void BFMT_P_SetFhd
    ( void )
{
    BDBG_ERR(("No longer supported"));
    return;
}

/* End of File */
