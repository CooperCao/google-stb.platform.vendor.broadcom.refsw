/***************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "bvdc_anr_priv.h"
#include "bmth_fix.h"

BDBG_MODULE(BVDC_ANR);

#if (BVDC_P_SUPPORT_MANR)

/*
 * (Additional global) programmable register parameters that affect the new I-pulse control algorithm:
 */
#define BVDC_P_ANR_SNR_DRIFT_THR        6 /* Threshold used to enable min(SNR) update when cadence follows an upward drift, conservative=4 */
#define BVDC_P_ANR_SNR_SCE_CHG_THR      6 /* Threshold used to avoid scene change disruption in SNR monitoring */
#define BVDC_P_ANR_SNR_OFFSET           0 /* (in dB) -> default = 0 , experimented with non-zero values which could be static */
#define BVDC_P_ANR_FIX_MC_INDEX         3 /* psnr index to chose MC blend if hAnr->bFixMcCurve is set to '1', recommended = 3 (corresponding to 48dB) */
#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
#define BVDC_P_ANR_LUT_LEN             20 /* (Reduced with respect to the existing code )  */
#else
#define BVDC_P_ANR_LUT_LEN             17 /* (Reduced with respect to the existing code )  */
#endif
#define BVDC_P_ANR_REFRESH_COUNT      128 /* no. of pictures after which the global min is refreshed */
#define BVDC_P_ANR_MASK_WIDTH           7
#define BVDC_P_ANR_IPULSE_CADENCE_THR   0

#if (BVDC_P_SUPPORT_MANR_VER < BVDC_P_MANR_VER_5)
static const uint32_t s_ulDbConvert = 255*25.6*15;

/* dB - The * 4 signifies 2 fractional LSBs */
static const uint16_t s_psnr[] =
    { 4*51.00, 4*50.00, 4*49.00, 4*48.00, 4*47.00, 4*46.00, 4*45.50,
      4*45.00, 4*44.50, 4*44.00, 4*43.50, 4*43.00, 4*42.50, 4*42.00,
      4*41.25, 4*40.75, 0.00 };

/* Updated LUT for "Low" slider setting */
static const uint16_t s_low_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_low_mc_iir_k1[] =
    {    0,  160,  300,  500,  900, 1300, 1300, 1300, 1300, 1300, 1300,
      1300, 1300, 1300, 1300, 1300, 1300};
static const uint16_t s_low_mc_alpha_low[] =
    {  176,  176,  176,  176,  176 , 176,  176,  176 , 176,  176,  176,
       176,  176,  176,  176,  176,  176};
static const uint16_t s_low_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  332,  332,  332,  332,
       332,  332,  332,  332,  332,  332};
static const uint16_t s_low_nmc_iir_k1[] =
    {    0,   10,   18,   28,   42,   56,   56,   56,   56,   56,   56,
        56,   56,   56,   56,   56,   56};
static const uint16_t s_low_nmc_alpha_low[] =
    {   48,   48,   48,   48,   48,   47,   47,   47,   47,   47,   47,
        47,   47,   47,   47,   47,   47};
static const uint16_t s_low_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_low_blend_k1[] =
    {    1,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
         2,    2,    2,    2,    2,    2};
static const uint16_t s_low_mc_adj[] =
    {    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
         2,    2,    2,    2,    2,    2};
static const uint16_t s_low_nmc_adj[] =
    {    3,    3,    3,    3,    3,    2,    2,    2,    2,    2,    2,
         2,    2,    2,    2,    2,    2};

/* Updated LUT for "Med Low" slider setting */
static const uint16_t s_medlo_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_medlo_mc_iir_k1[] =
    {    0,  160,  300,  500,  900, 1300, 1300, 1300, 1300, 1300, 1300,
      1300, 1300, 1300, 1300, 1300, 1300};
static const uint16_t s_medlo_mc_alpha_low[] =
    {  176,  176,  176,  176,  176,  176,  176,  176,  176,  176,  176,
       176,  176,  176,  176,  176,  176};
static const uint16_t s_medlo_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  332,  332,  332,  332,
       332,  332,  332,  332,  332,  332};
static const uint16_t s_medlo_nmc_iir_k1[] =
    {    0,   10,   18,   28,   42,   56,   56,   56,   56,   56,   56,
        56,   56,   56,   56,   56,   56};
static const uint16_t s_medlo_nmc_alpha_low[] =
    {   48,   48,   48,   48,   48,   47,   47,   47,   47,   47,   47,
        47,   47,   47,   47,   47,   47};
static const uint16_t s_medlo_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_medlo_blend_k1[] =
    {    2,    3,    3,    3,    4,    4,    4,    4,    4,    4,    4,
         4,    4,    4,    4,    4,    4};
static const uint16_t s_medlo_mc_adj[] =
    {    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
         2,    2,    2,    2,    2,    2};
static const uint16_t s_medlo_nmc_adj[] =
    {    3,    3,    3,    3,    3,    2,    2,    2,    2,    2,    2,
         2,    2,    2,    2,    2,    2};

/* Updated LUT for "Medium" slider setting */
static const uint16_t s_med_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_med_mc_iir_k1[] =
    {    0,  160,  300,  500,  900, 1300, 1600, 2000, 2500, 3100, 3800,
      4600, 4600, 4600, 4600, 4600, 4600};
static const uint16_t s_med_mc_alpha_low[] =
    {  176,  176,  176,  176,  176 , 176,  176,  176 , 176,  174,  170,
       166,  166,  166,  166,  166,  166};
static const uint16_t s_med_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_med_nmc_iir_k1[] =
    {    0,   10,   18,   28,   42,   56,   68,   84,  100,  116,  136,
       160,  160,  160,  160,  160,  160};
static const uint16_t s_med_nmc_alpha_low[] =
    {   48,   48,   48,   48,   48,   47,   46,   45,   45,   45,   45,
        45,   45,   45,   45,   45,   45};
static const uint16_t s_med_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_med_blend_k1[] =
    {    3,    4,    4,    4,    5,    5,    5,    6,    8,    9,   11,
        13,   13,   13,   13,   13,   13};
static const uint16_t s_med_mc_adj[] =
    {    2,    2,    2,    2,    2,    2,    2,    1,    1,    1,    1,
         1,    1,    1,    1,    1,    1};
static const uint16_t s_med_nmc_adj[] =
    {    3,    3,    3,    3,    3,    2,    2,    2,    2,    2,    2,
         2,    2,    2,    2,    2,    2};

/* Updated LUT for "Med High" slider setting */
static const uint16_t s_medhi_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_medhi_mc_iir_k1[] =
    {    0,  160,  300,  500,  900, 1300, 1600, 2000, 2500, 3100, 3800,
      4600, 5400, 6200, 7000, 8000, 9000};
static const uint16_t s_medhi_mc_alpha_low[] =
    {  176,  176,  176,  176,  176,  176,  176,  176,  176,  174,  170,
       166,  162,  162,  158,  154,  150};
static const uint16_t s_medhi_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_medhi_nmc_iir_k1[] =
    {    0,   10,   18,   28,   42,   56,   68,   84,  100,  116,  136,
       160,  176,  192,  210,  224,  240};
static const uint16_t s_medhi_nmc_alpha_low[] =
    {   48,   48,   48,   48,   48,   47,   46,   45,   45,   45,   45,
        45,   45,   45,   45,   45,   45};
static const uint16_t s_medhi_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_medhi_blend_k1[] =
    {    3,    4,    4,    5,    6,    6,    6,    7,    9,   11,   13,
        15,   15,   16,   16,   17,   17};
static const uint16_t s_medhi_mc_adj[] =
    {    2,    2,    2,    2,    2,    2,    2,    1,    1,    1,    1,
         1,    1,    0,    0,    0,    0};
static const uint16_t s_medhi_nmc_adj[] =
    {    3,    3,    3,    3,    3,    2,    2,    2,    2,    2,    2,
         2,    2,    3,    3,    3,    2};

/* Updated LUT for "High" slider setting */
static const uint16_t s_high_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_high_mc_iir_k1[] =
    {    0,  160,  300,  500,  900, 1300, 1600, 2000, 2500, 3100, 3800,
      4600, 5400, 6200, 7000, 8000, 9000};
static const uint16_t s_high_mc_alpha_low[] =
    {  176,  176,  176,  176,  176 , 176,  176,  176,  176,  174,  170,
       166,  162,  162,  158,  154,  150};
static const uint16_t s_high_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_high_nmc_iir_k1[] =
    {    0,   10,   18,   28,   42,   56,   68,   84,  100,  116,  136,
       160,  176,  192,  210,  224,  240};
static const uint16_t s_high_nmc_alpha_low[] =
    {   48,   48,   48,   48,   48,   47,   46,   45,   45,   45,   45,
        45,   45,   45,   45,   45,   45};
static const uint16_t s_high_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270};
static const uint16_t s_high_blend_k1[] =
    {    4,    5,    5,    5,    6,    6,    6,    8,   10,   12,   14,
        16,   16,   17,   17,   18,   18};
static const uint16_t s_high_mc_adj[] =
    {    2,    2,    2,    2,    2,    2,    2,    1,    1,    1,    1,
         1,    1,    0,    0,    0,    0};
static const uint16_t s_high_nmc_adj[] =
    {    3,    3,    3,    3,    3,    2,    2,    2,    2,    2,    2,
         2,    2,    3,    3,    3,    2};
#else
#define BVDC_P_ANR_SNR_DELTA_AVG_THR1   2
#define BVDC_P_ANR_SNR_DELTA_AVG_THR2   5
#define BVDC_P_ANR_SNR_REF             51
#define BVDC_P_ANR_SCALE_SNR_THR       40
#define BVDC_P_ANR_AUTO_SCALE           1 /* 1=> enabling automatic scaling of the elements of mc_alpha_low (lut) entries ,
                                             0=> scaling defined by MC_ALPHA_LOW_SCALE& NMC_ALPHA_LOW_SCALE */
#define BVDC_P_ANR_MC_ALPHA_LOW_SCALE   (uint16_t)BMTH_FIX_SIGNED_FTOFIX(1.0, 20, 5) /* Scaler between (0, 1] */
#define BVDC_P_ANR_NMC_ALPHA_LOW_SCALE  (uint16_t)BMTH_FIX_SIGNED_FTOFIX(1.0, 20, 5) /* Scaler between (0, 1] */

#define BVDC_P_ANR_MINIM_PEAKS_DETECTED          3
#define BVDC_P_ANR_PEAK_DISTANCE_LARGE           6
#define BVDC_P_ANR_PULSE_DISTANCE_MIN            3
#define BVDC_P_ANR_PULSE_DISTANCE_MAX            8
#define BVDC_P_ANR_PULSE_PERCENTAGE_OF_AVERAGE   5
#define BVDC_P_ANR_MAX_PULSE_DEVIATION           2

static const uint32_t s_ulDbConvert = 1023*25.6*15;

/* dB - The * 4 signifies 2 fractional LSBs */
static const uint16_t s_psnr[] =
    { 51,   50,   49,   48,   47,   46,   45,   44,   43,   42,
      41,   40,   39,   38,   37,   36,   35,   34,    33,   0 };

/* Updated LUT for "Low" slider setting */
static const uint16_t s_lut0_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut0_mc_iir_k1[] =
    {    0,  100,  200,  300,  500,  700,  800, 1000, 1300, 1600,
      1900, 2300, 2700, 3100, 3500, 4000, 4500, 5000, 5700, 6500};
static const uint16_t s_lut0_mc_alpha_low[] =
    {   96,   96,   96,   96,   96,   94,   92,   90,   88,   87,
        85,   83,   81,   81,   79,   77,   75,   74,   73,   70};
static const uint16_t s_lut0_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut0_nmc_iir_k1[] =
    {    0,    3,    7,   12,   19,   25,   30,   38,   45,   52,
        61,   72,   80,   87,   96,  102,  110,  120,  140,  171};
static const uint16_t s_lut0_nmc_alpha_low[] =
    {  128,  128,  128,  128,  128,  122,  116,  110,  110,  107,
       102,  100,   98,   98,   96,   94,   92,   90,   88,   81};
static const uint16_t s_lut0_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut0_blend_k1[] =
    {    4,    5,    5,    6,    7,    7,    7,    9,   11,   13,
        16,   18,   18,   19,   19,   20,   20,   21,   21,   21};
static const uint16_t s_lut0_mc_adj[] =
    {    1,    1,    1,    1,    1,    1,    1,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,    0,    0};
static const uint16_t s_lut0_nmc_adj[] =
    {    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
         2,    2,    2,    2,    2,    2,    2,    2,    1,    1};

/* Updated LUT for "Med Low" slider setting */
static const uint16_t s_lut1_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut1_mc_iir_k1[] =
    {    0,  100,  200,  300,  500,  700,  800, 1000, 1300, 1600,
      1900, 2300, 2700, 3100, 3500, 4000, 4500, 5000, 5700, 6500};
static const uint16_t s_lut1_mc_alpha_low[] =
    {   96,   96,   96,   96,   96,   94,   92,   90,   88,   87,
        85,   83,   81,   81,   79,   77,   75,   74,   73,   70};
static const uint16_t s_lut1_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut1_nmc_iir_k1[] =
    {    0,    3,    7,   12,  19,    25,   30,   38,   45,   52,
        61,   72,   80,   87,  96,   102,  110,  120,  140,  171};
static const uint16_t s_lut1_nmc_alpha_low[] =
    {  128,  128,  128,  128,  128,  122,  116,  110,  110,  107,
       102,  100,   98,   98,   96,   94,   92,   90,   88,   81};
static const uint16_t s_lut1_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut1_blend_k1[] =
    {    4,    5,    5,    6,    7,    7,    7,    9,   11,   13,
        16,   18,   18,   19,   19,   20,   20,   21,   21,   21};
static const uint16_t s_lut1_mc_adj[] =
    {    1,    1,    1,    1,    1,    1,    1,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,    0,    0};
static const uint16_t s_lut1_nmc_adj[] =
    {    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
         1,    1,    1,    1,    1,    1,    1,    1,    0,    0};

/* Updated LUT for "Medium" slider setting */
static const uint16_t s_lut2_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut2_mc_iir_k1[] =
    {    0,  100,  200,  300,  500,  700,  800, 1000, 1300, 1600,
      1900, 2300, 2700, 3100, 3500, 4000, 4500, 5000, 5700, 6500};
static const uint16_t s_lut2_mc_alpha_low[] =
    {   96,   96,   96,   96,   96,   94,   92,   90,   88,   87,
        85,   83,   81,   81,   79,   77,   75,   74,   73,   70};
static const uint16_t s_lut2_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut2_nmc_iir_k1[] =
    {    0,    3,    7,   12,  19,    25,   30,   38,   45,   52,
        61,   72,   80,   87,  96,   102,  110,  120,  140,  171};
static const uint16_t s_lut2_nmc_alpha_low[] =
    {  128,  128,  128,  128,  128,  122,  116,  110,  110,  107,
       102,  100,   98,   98,   96,   94,   92,   90,   88,   81};
static const uint16_t s_lut2_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut2_blend_k1[] =
    {    4,    5,    5,    6,    7,    7,    7,    9,   11,   13,
        16,   18,   18,   19,   19,   20,   20,   21,   21,   21};
static const uint16_t s_lut2_mc_adj[] =
    {    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,    0,    0};
static const uint16_t s_lut2_nmc_adj[] =
    {    1,    1,    1,    1,    1,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,    0,    0};

/* Updated LUT for "Med High" slider setting */
static const uint16_t s_lut3_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut3_mc_iir_k1[] =
    {    0,  160,  300,  500,  900, 1300, 1600, 2000, 2500, 3100,
      3800, 4600, 5400, 6200, 7000, 8000, 9000,10000,11400,13000};
static const uint16_t s_lut3_mc_alpha_low[] =
    {   88,   88,   88,   88,   88,   88,   88,   88,   88,   87,
        85,   83,   81,   81,   79,   77,   75,   74,   73,   70};
static const uint16_t s_lut3_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut3_nmc_iir_k1[] =
    {    0,    5,    9,   14,   21,   28,   34,   42,   50,   58,
        68,   80,   88,   96,  105,  112,  120,  133,  156,  196};
static const uint16_t s_lut3_nmc_alpha_low[] =
    {   96,   96,   96,   96,   96,   94,   92,   90,   90,   90,
        90,   90,   90,   90,   90,   90,   90,   88,   86,   81};
static const uint16_t s_lut3_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  332,  332};
static const uint16_t s_lut3_blend_k1[] =
    {    4,    5,    5,    6,    7,    7,    7,    9,   11,   13,
        16,   18,   18,   19,   19,   20,   20,   21,   21,   21};
static const uint16_t s_lut3_mc_adj[] =
    {    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,    0,    0};
static const uint16_t s_lut3_nmc_adj[] =
    {    1,     1,   1,    1,    1,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,    0,    0};

/* Updated LUT for "High" slider setting */
static const uint16_t s_lut4_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut4_mc_iir_k1[] =
    {    0,  256,  480,  800, 1440, 2080, 2560, 3200, 4000, 4960,
      6080, 7360, 8640, 9920,11200,12800,14400,16000,18240,20800};
static const uint16_t s_lut4_mc_alpha_low[] =
    {   70,   70,   70,   70,   70,   70,   70,   70,   70,   70,
        70,   70,   70,   70,   70,   70,   70,   70,   70,   70};
static const uint16_t s_lut4_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut4_nmc_iir_k1[] =
    {    0,   15,   27,   42,   63,   84,  102,  126,  150,  174,
       204,  240,  264,  288,  315,  336,  360,  399,  468,  588};
static const uint16_t s_lut4_nmc_alpha_low[] =
    {   70,   70,   70,   70,   70,   70,   70,   70,   70,   70,
        70,   70,   70,   70,   70,   70,   69,   68,   67,   66};
static const uint16_t s_lut4_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  332,  332};
static const uint16_t s_lut4_blend_k1[] =
    {    3,    4,    4,    5,    6,    6,    6,    7,    9,   11,
        13,   15,   15,   16,   16,   17,   17,   18,   18,   18};
static const uint16_t s_lut4_mc_adj[] =
    {    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,    0,    0};
static const uint16_t s_lut4_nmc_adj[] =
    {    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,    0,    0};

/* Updated LUT for "High" slider setting */
static const uint16_t s_lut5_mc_iir_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut5_mc_iir_k1[] =
    {    0,  256,  480,  800, 1440, 2080, 2560, 3200, 4000, 4960,
      6080, 7360, 8640, 9920,11200,12800,14400,16000,18240,20800};
static const uint16_t s_lut5_mc_alpha_low[] =
    {   70,   70,   70,   70,   70,   70,   70,   70,   70,   70,
        70,   70,   70,   70,   70,   70,   70,   70,   70,   70};
static const uint16_t s_lut5_nmc_iir_k0[] =
    {  332,  332,  332,  332,  332,  332,  332,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  270,  270};
static const uint16_t s_lut5_nmc_iir_k1[] =
    {    0,   15,   27,   42,   63,   84,  102,  126,  150,  174,
       204,  240,  264,  288,  315,  336,  360,  399,  468,  588};
static const uint16_t s_lut5_nmc_alpha_low[] =
    {   70,   70,   70,   70,   70,   70,   70,   70,   70,   70,
        70,   70,   70,   70,   70,   70,   69,   68,   67,   66};
static const uint16_t s_lut5_blend_k0[] =
    {  270,  270,  270,  270,  270,  270,  270,  270,  270,  270,
       270,  270,  270,  270,  270,  270,  270,  270,  332,  332};
static const uint16_t s_lut5_blend_k1[] =
    {    6,    7,    9,   11,   13,   15,   15,   16,   16,   17,
        17,   18,   18,   18,   19,   19,   19,   20,   20,   20};
static const uint16_t s_lut5_mc_adj[] =
    {    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,    0,    0};
static const uint16_t s_lut5_nmc_adj[] =
    {    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
         0,    0,    0,    0,    0,    0,    0,    0,    0,    0};
#endif

static int32_t BVDC_P_ANR_LinearLog10_isr( uint32_t ulVal )
{
    /*
             x                                20*log10(x)
             x <= 2                          (6x - 6) / 1
         2 < x <= 4                          (6x + 0) / 2
         4 < x <= 8                          (6x + 24) / 4
         8 < x <= 12                         (6x + 60) / 6
        12 < x <= 25                         (6x + 214) / 13
        25 < x <= 50                         (6x + 550) / 25
        50 < x <= 100                        (6x + 1400) / 50
       100 < x <= 200                        (6x + 3400) / 100
       200 < x                               (6x + 8000) / 200
    */
    uint32_t lResult = 6 * ulVal;
    if(ulVal <= 2)
        lResult = (lResult - 6) / 1;
    else if(ulVal <= 4)
        lResult = (lResult + 0) / 2;
    else if(ulVal <= 8)
        lResult = (lResult + 24) / 4;
    else if(ulVal <= 12)
        lResult = (lResult + 60) / 6;
    else if(ulVal <= 25)
        lResult = (lResult + 214) / 13;
    else if(ulVal <= 50)
        lResult = (lResult + 550) / 25;
    else if(ulVal <= 100)
        lResult = (lResult + 1400) / 50;
    else if(ulVal <= 200)
        lResult = (lResult + 3400) / 100;
    else
        lResult = (lResult + 8000) / 200;

    return lResult;
}

static void BVDC_P_Anr_ArrayShift_isr
    ( uint32_t    *pArray,
      uint32_t     ulNewv,
      uint32_t     ulLength )
{
    uint32_t ulId;
    for(ulId = ulLength - 1; ulId >= 1; ulId--)
    {
        pArray[ulId] = pArray[ulId-1];
    }
    pArray[0] = ulNewv;
}

static uint32_t BVDC_P_Anr_ArraySum_isr
    ( uint32_t    *pArray,
      uint32_t     ulLength )
{
    uint32_t ulId;
    uint32_t ulSum = 0;
    for(ulId = 0; ulId < ulLength; ulId++)
        ulSum += pArray[ulId];
    return ulSum;
}

static uint32_t BVDC_P_Anr_ArrayMean_isr
    ( uint32_t    *pArray,
      uint32_t     ulLength )
{
    return ((BVDC_P_Anr_ArraySum_isr(pArray, ulLength) + ulLength/2) / ulLength);
}

static uint32_t BVDC_P_Anr_CalcNoise_isr
    ( BVDC_P_Anr_Handle      hAnr,
      uint32_t              *pulNoiseSampleNum,
      uint32_t              *pulNoiseMsb,
      uint32_t              *pulNoiseLsb )
{
    int32_t  lBinStart = -1, lBinEnd;   /* bin start: first bin that exceed num_noise_thd */
    uint32_t aulNoiseLevel[BVDC_P_NOISE_LEVELS];
    uint32_t ulNoise = 2*15;
    uint32_t ulId;
    uint32_t ulBinCnt;

    /*BDBG_MSG(("Entering Calc_noise: %d %d %d %d %d %d %d %d %d %d",
        pulNoiseSampleNum[0], pulNoiseSampleNum[1], pulNoiseSampleNum[2], pulNoiseSampleNum[3], pulNoiseSampleNum[4],
        aulNoiseLevel[0], aulNoiseLevel[1], aulNoiseLevel[2], aulNoiseLevel[3], aulNoiseLevel[4]));*/

    for(ulId = 0; ulId < BVDC_P_NOISE_LEVELS; ulId++) {
        if((pulNoiseSampleNum[ulId] >= hAnr->ulNumNoisySampleThd) && (-1 == lBinStart)) {
            lBinStart = ulId;
        }

        if(pulNoiseSampleNum[ulId] > 0) {
            /*aulNoiseLevel[ulId] = 32 * aulNoiseLevel[ulId] / pulNoiseSampleNum[ulId];*/
            aulNoiseLevel[ulId] = 32 * (uint32_t)((((uint64_t)(pulNoiseMsb[ulId]) << 32) + pulNoiseLsb[ulId]) / pulNoiseSampleNum[ulId]);
            if(aulNoiseLevel[ulId] > 4) {
                aulNoiseLevel[ulId] -= 4;
            }
        }
    }

    /*BDBG_MSG(("calc_noise: lBinStart=%d, ulNumNoisySampleThd=%d, %d %d %d %d %d",
        lBinStart, hAnr->ulNumNoisySampleThd,
        aulNoiseLevel[0], aulNoiseLevel[1], aulNoiseLevel[2], aulNoiseLevel[3], aulNoiseLevel[4]));*/

    if(-1 != lBinStart) {   /* no one exceeds thresh */
        ulNoise = aulNoiseLevel[lBinStart];
        lBinEnd = lBinStart;    /* bin end: last bin used in noise calc */
        for(ulId = lBinStart + 1; ulId < BVDC_P_MIN(BVDC_P_NOISE_LEVELS, (uint32_t)lBinStart+3); ulId++) {
            /* just to quiet the overrun coverity check */
            if(ulId >= BVDC_P_NOISE_LEVELS) break;
            if(pulNoiseSampleNum[ulId] < hAnr->ulNumNoisySampleThd) { break; }
            ulNoise += aulNoiseLevel[ulId];
            lBinEnd = ulId;
        }
        ulBinCnt = lBinEnd - lBinStart + 1;   /* num items summed */

        if(3 == ulBinCnt) {   /* shift to preferred location */
            for(ulId = lBinEnd + 1; ulId < BVDC_P_NOISE_LEVELS; ulId++) {
                if((pulNoiseSampleNum[ulId] > hAnr->ulNumNoisySampleThd + hAnr->ulNumDiffThd) ||
                   (pulNoiseSampleNum[ulId] < hAnr->ulNumNoisySampleThdBig)) { break; }
                ulNoise -= aulNoiseLevel[lBinStart++];
                ulNoise += aulNoiseLevel[ulId];
            }
        }
        /* ulNoise /= ulBinCnt; */
        ulNoise = 4 * ulNoise / ulBinCnt;   /* To obtain 2 extra LSBs for pulsing algorithm - Advait */
    }

    return ulNoise;
}

static int32_t BVDC_P_Anr_MinBlock_isr( uint32_t aArray[BVDC_P_NOISE_HIST_NUM] )
{
    /* Actually, calculate MAX of 'BVDC_P_NOISE_HIST_NUM' consecutive readings,
      (since we use 1/min_val for the log function): */
    uint32_t ulMinVal = aArray[0];
    uint32_t ulId;

    for(ulId = 1; ulId < BVDC_P_NOISE_HIST_NUM; ulId++) {
        if(aArray[ulId] > ulMinVal)
            ulMinVal = aArray[ulId];
    }
    /* Convert to dB, use table look-up: */
    return (ulMinVal > 0 ) ? BVDC_P_ANR_LinearLog10_isr(s_ulDbConvert*4/ulMinVal) : BVDC_P_ANR_LinearLog10_isr(0);
}

static int32_t BVDC_P_Anr_MinLongBlock_isr
    ( BVDC_P_Anr_Handle      hAnr,
      int32_t                lNoiseSample )
{
    /* Actually, calculate MAX of 'BVDC_P_NOISE_HIST_NUM' consecutive readings,
      (since we use 1/min_val for the log function): */
    int32_t lMinVal = hAnr->alLongArray[0];
    uint32_t ulId;

    for(ulId = 1; ulId < BVDC_P_LONG_ARRAY_LEN; ulId++) {
        if(hAnr->alLongArray[ulId] > lMinVal)
            lMinVal = hAnr->alLongArray[ulId];
        /* Shift the alLongArray pipeline: */
        hAnr->alLongArray[ulId-1] = hAnr->alLongArray[ulId];
    }
    hAnr->alLongArray[BVDC_P_LONG_ARRAY_LEN-1] = lNoiseSample;

    /* Convert to dB, use table look-up: */
    return (lMinVal > 0) ? BVDC_P_ANR_LinearLog10_isr(s_ulDbConvert*4/ lMinVal) : BVDC_P_ANR_LinearLog10_isr(0);
}

#if (BVDC_P_SUPPORT_MANR_VER < BVDC_P_MANR_VER_5)
static int32_t BVDC_P_Anr_IPulseCadenceBlock_isr
    ( BVDC_P_Anr_Handle              hAnr,
      uint32_t                       aArray[BVDC_P_NOISE_HIST_NUM],
      uint32_t                       ulCurrp  )
{
    uint32_t ulId, ulIdMinus1, ulIdPlus2;
    int32_t lIpulseCadenceDeltaP, lTmp, lj;
    uint32_t ulMaxVal, ulMinVal, ulMaxId;

    ulMinVal = ulMaxVal = aArray[ulCurrp];
    ulMaxId = 0;

    for(ulId = 1; ulId < BVDC_P_ANR_MASK_WIDTH; ulId++) {
        lj = ulCurrp - ulId;   /* Next, address wrap around */
        if(lj < 0)
            lj = lj + BVDC_P_NOISE_HIST_NUM;
        if(aArray[lj] > ulMaxVal) {   /* Compute max */
            ulMaxVal = aArray[lj];
            ulMaxId = ulId;
        }
        if(aArray[lj] < ulMinVal) {   /* Compute min */
            ulMinVal = aArray[lj];
        }
    }
    /* Generate the indices to the left and to the right of the max index 'i': */
    ulIdMinus1 = (ulCurrp >= (BVDC_P_ANR_MASK_WIDTH-1)) ? (ulCurrp-(BVDC_P_ANR_MASK_WIDTH-1)) : BVDC_P_NOISE_HIST_NUM+(ulCurrp-(BVDC_P_ANR_MASK_WIDTH-1));
    ulIdPlus2  = ulCurrp;
    /* Next, compute the cadence "change" (if any), tmp has 8.2 precision  - required by the LUT: */
    /*tmp = (int)(20*log10((float)max_val/(float)((a[i_minus_1] + a[i_plus_2])/2)) + 0.5);*/
    if((aArray[ulIdMinus1] + aArray[ulIdPlus2]) > 0)
        lTmp = BVDC_P_ANR_LinearLog10_isr(ulMaxVal/((aArray[ulIdMinus1] + aArray[ulIdPlus2])/2));
      else
         lTmp = 0;

    if(hAnr->ulRndCeil == 3)
    {
        /* round up */
        lTmp += 1;
    }

    /* Check to see if the 'max_val' does not occur at the edge of the MASK_WIDTH wide sample window
       and if the cadence pulse is < BVDC_P_ANR_SNR_SCE_CHG_THR
       and if the samples surrounding the cadence pulse are close (i.e. it is a pulse and not a level change): */
    if((ulMaxId != 0) && (ulMaxId != (BVDC_P_ANR_MASK_WIDTH-1)) &&
       (BVDC_P_ABS(lTmp) < BVDC_P_ANR_SNR_SCE_CHG_THR) &&
        ((ulMaxVal-ulMinVal) > (uint32_t)(BVDC_P_ABS((int)aArray[ulIdMinus1]-(int)aArray[ulIdPlus2])*2)))
    {
        lIpulseCadenceDeltaP = lTmp;
    } else {   /* Invalid min - flagged by '999': */
        lIpulseCadenceDeltaP = 999;
    }
    return lIpulseCadenceDeltaP;
}
#endif

static void BVDC_P_Anr_ProcessSnrMetrics_isr
    ( BVDC_P_Anr_Handle         hAnr,
      int32_t                   lIpulseCadenceDeltaP,
      int32_t                  *lSnrDeltaAvgP )
{
    uint32_t ulId, ulId2 = 0;
    int32_t lTmp=0;

    for (ulId=0; ulId < BVDC_P_LONG_ARRAY_LEN; ulId++) {
        /* valid delta reading */
        if((hAnr->alDeltaArray[ulId] != 999) && (hAnr->alDeltaArray[ulId] > BVDC_P_ANR_IPULSE_CADENCE_THR)) {
            lTmp += hAnr->alDeltaArray[ulId];
            ulId2++;
        }
    }
    /* Average over valid delta readings */
    if (ulId2 == 0) {
        *lSnrDeltaAvgP = 0;   /* no valid data available */
    } else {
        *lSnrDeltaAvgP = 4 * lTmp / ulId2;
    }
    /* Update arrays by shifting in current values: */
    for(ulId = 0; ulId < BVDC_P_LONG_ARRAY_LEN - 1; ulId++) {
        hAnr->alDeltaArray[ulId] = hAnr->alDeltaArray[ulId+1];
    }
    hAnr->alDeltaArray[BVDC_P_LONG_ARRAY_LEN-1] = lIpulseCadenceDeltaP;

    return;
}

static void BVDC_P_Anr_IpulseControl_isr
    ( BVDC_P_Anr_Handle      hAnr,
      int32_t               *plSnrDeltaAvg,
      int32_t               *plSnrAdj )
{
    int32_t lPrevNoiseMean, lTmp;
    uint32_t ulId;
    int32_t lSnrInstant, lSnrAvg, lSnrRef, lLocalMin, lMinLong = 0;
    int32_t lIpulseCadenceDeltaP;

    /* Compute the previous noise (block) mean from the array 'aulNoiseHist'
       of 'BVDC_P_NOISE_HIST_NUM' stored consecutive noise values: */
    uint32_t ulNoise = hAnr->aulNoiseHist[0];

    lPrevNoiseMean = BVDC_P_Anr_ArrayMean_isr(hAnr->aulNoiseHist, BVDC_P_NOISE_HIST_NUM) / 4;
    /* where mean_block() simply calculates the mean of the BVDC_P_NOISE_HIST_NUM noise values
       Note that the /4 and *4 at various stages is to get 2 additional LSBs of precision
       Instantaneous (raw) flt. pt. SNR (dB) output: */
    /*lSnrInstant = (ulNoise < 4) ? 100 : (int)(20 * log10(255*25.6*15*4/ulNoise) + 0.5); */ /* lSnrInstant has 8.2 precision - required by the LUT */
    lSnrInstant = (ulNoise < 4) ? 100 : BVDC_P_ANR_LinearLog10_isr(s_ulDbConvert*4/ulNoise);

    ulNoise = BVDC_P_MAX(ulNoise, 275*4);   /* set min noise level corresponding to an SNR of 51 dB */
    if(hAnr->bInitializeArray) {   /* initialize whole history */
        for(ulId = 0; ulId < BVDC_P_LONG_ARRAY_LEN; ulId++) {
            hAnr->alLongArray[ulId]= ulNoise;
        }
        for(ulId = 0; ulId < BVDC_P_NOISE_HIST_NUM; ulId++) {
            hAnr->aulNoiseHist[ulId]= ulNoise;
        }
        lPrevNoiseMean = ulNoise / 4;
    }

    /* convert the 'lPrevNoiseMean' value to dB (i.e. previous snr block avg.), as in: */
    /*lPrevNoiseMean = (lPrevNoiseMean < 1) ? 100 : (int) (20 * log10(255*25.6*15/lPrevNoiseMean) + 0.5);*/ /* lPrevNoiseMean has 8.2 precision - required by the LUT */
    lTmp = (lPrevNoiseMean > 0) ? BVDC_P_ANR_LinearLog10_isr(s_ulDbConvert/lPrevNoiseMean) : BVDC_P_ANR_LinearLog10_isr(0);
    lPrevNoiseMean = (lPrevNoiseMean < 1) ? 100 : lTmp;

    /* and compute the running snr block average 'lSnrAvg', (with block size = BVDC_P_NOISE_HIST_NUM): */
    /*lSnrAvg = (int)(20 * log10(255*25.6*15*4/ BVDC_P_Anr_ArrayMean_isr(hAnr->aulNoiseHist, BVDC_P_NOISE_HIST_NUM)) + 0.5);*/ /* lSnrAvg has 8.2 precision - required by the LUT */
    lTmp = BVDC_P_Anr_ArrayMean_isr(hAnr->aulNoiseHist, BVDC_P_NOISE_HIST_NUM);
    lSnrAvg = (lTmp > 0) ? BVDC_P_ANR_LinearLog10_isr(s_ulDbConvert*4/ lTmp) : BVDC_P_ANR_LinearLog10_isr(0);

    /* Run the cadence processing block, which computes the cadence drop: */
#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
    lIpulseCadenceDeltaP = 0;
#else
    lIpulseCadenceDeltaP = BVDC_P_Anr_IPulseCadenceBlock_isr(hAnr, hAnr->aulNoiseHist, 0);
#endif

    /* Update the cadence delta array, based upon current value of the cadence
       drop and compute the average cadence drop 'snr_delta_avg': */
    BVDC_P_Anr_ProcessSnrMetrics_isr(hAnr, lIpulseCadenceDeltaP, plSnrDeltaAvg);

    /* Next, define 'lSnrRef' for the subsequent minimum snr tracking computation, based upon the assignment of 'MIN_METHOD': */
#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
    lSnrRef = (hAnr->ulMinMethod == 0) ? lSnrInstant : lSnrAvg / 4;
#else
    lSnrRef = (hAnr->ulMinMethod == 0) ? lSnrInstant : lSnrAvg;
#endif

    /* Concurrently, maintain the 'lLocalMin', (i.e. the minimum of
       'BVDC_P_NOISE_HIST_NUM' consecutive snr levels) and 'lMinLong' (computed
       over 'BVDC_P_LONG_ARRAY_LEN' consecutive snr levels): */
    lLocalMin = BVDC_P_Anr_MinBlock_isr(hAnr->aulNoiseHist);

    if(BVDC_P_ABS(lSnrRef - lPrevNoiseMean) < BVDC_P_ANR_SNR_SCE_CHG_THR)
        lMinLong = BVDC_P_Anr_MinLongBlock_isr(hAnr, ulNoise);
    else   /* do not pipe in the current noise value */
        lMinLong = BVDC_P_Anr_MinLongBlock_isr(hAnr, hAnr->alLongArray[BVDC_P_LONG_ARRAY_LEN-1]);

    /* The penultimate step: compute the "global" snr minimum: 'snr_min': */
    if(hAnr->bInitializeArray) {
        /* Update the min snr register upon initialization: */
        /* else, if upward cadence drift exceeds threshold, replace snr_min by the local minimum */
        hAnr->lSnrMinP = lSnrRef;
        hAnr->ulIframeCnt = 0;   /* Reset the non-repeating frame count */
    } else if(hAnr->lSnrMinP <= lLocalMin - BVDC_P_ANR_SNR_DRIFT_THR)  {
        hAnr->lSnrMinP = lLocalMin; /* else, update if new min is found and there is no accompanying scene change: */
        hAnr->ulIframeCnt = 0; /* Reset the non-repeating frame count */
    } else if((lSnrRef < hAnr->lSnrMinP) && (BVDC_P_ABS(lSnrRef - lPrevNoiseMean) < BVDC_P_ANR_SNR_SCE_CHG_THR)) {
        hAnr->lSnrMinP = lSnrRef;   /* finally, re-fresh the min after every BVDC_P_ANR_REFRESH_COUNT no. of pictures: */
    } else if(hAnr->ulIframeCnt >= BVDC_P_ANR_REFRESH_COUNT) {
        hAnr->lSnrMinP = lMinLong;  /* can also try: hAnr->lSnrMinP = hAnr->lSnrMinP + 1; */
        hAnr->ulIframeCnt = 0;  /* Reset the non-repeating frame count */
    }

    /* Finally, set MCTF IIR filter parameters for the next picture, based upon 'snr_adj': */
#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
    if(hAnr->ulFilterUpdate == 0)   /* based upon the avg. snr (old scheme) */
        *plSnrAdj = ((lSnrAvg - 4 * BVDC_P_ANR_SNR_OFFSET) >= 0) ? (lSnrAvg - 4 * BVDC_P_ANR_SNR_OFFSET) / 4 : 0;
    else if(hAnr->ulFilterUpdate == 1) /* ' based upon the min. snr */
        *plSnrAdj = ((hAnr->lSnrMinP - BVDC_P_ANR_SNR_OFFSET) >= 0) ? (hAnr->lSnrMinP - BVDC_P_ANR_SNR_OFFSET) : 0;
    else if(hAnr->ulFilterUpdate == 2) { /* ' based upon ulSnrAvg-snr_delta_avg */
        *plSnrAdj = BVDC_P_MAX(lMinLong, (lSnrAvg - *plSnrDeltaAvg) / 4);
        *plSnrAdj = ((*plSnrAdj - BVDC_P_ANR_SNR_OFFSET) >= 0) ? (*plSnrAdj - BVDC_P_ANR_SNR_OFFSET) : 0;
    }
#else
    if(hAnr->ulFilterUpdate == 0)   /* based upon the avg. snr (old scheme) */
        *plSnrAdj = ((lSnrAvg - 4 * BVDC_P_ANR_SNR_OFFSET) >= 0) ? (lSnrAvg - 4 * BVDC_P_ANR_SNR_OFFSET) : 0;
    else if(hAnr->ulFilterUpdate == 1) /* ' based upon the min. snr */
        *plSnrAdj = ((hAnr->lSnrMinP - BVDC_P_ANR_SNR_OFFSET) >= 0) ? (hAnr->lSnrMinP - BVDC_P_ANR_SNR_OFFSET) * 4 : 0;
    else if(hAnr->ulFilterUpdate == 2) /* ' based upon ulSnrAvg-snr_delta_avg */
        *plSnrAdj = (((lSnrAvg - *plSnrDeltaAvg) - 4 * BVDC_P_ANR_SNR_OFFSET) >= 0) ? ((lSnrAvg - *plSnrDeltaAvg) - 4 * BVDC_P_ANR_SNR_OFFSET) : 0;
#endif

    return;
}

#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
static int BVDC_P_Anr_GetFilterParam_isr
    ( BVDC_P_Anr_Handle              hAnr,
      int32_t                        lSnr,
      uint32_t                       ulIdx,
      BVDC_P_AnrKValue              *pKValue )
{
    /* Set MCTF filtering parameters to process the next picture.  In the following comments:
      Using the MCTF parameter notation K0 ' K5 described in Section 4 of the MCTF doc */
    if(hAnr->pAnrSetting->iSnDbAdjust < 0)
    {
        pKValue->ulMctfSetting = 0;
        pKValue->ulMcK0     = s_lut0_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_lut0_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_lut0_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_lut0_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_lut0_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_lut0_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_lut0_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_lut0_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_lut0_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_lut0_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_lut0_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_lut0_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_lut0_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("MCTF setting %d snr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            pKValue->ulMctfSetting, lSnr, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }
    else if(hAnr->pAnrSetting->iSnDbAdjust<= 100)
    {
        pKValue->ulMctfSetting = 1;
        pKValue->ulMcK0     = s_lut1_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_lut1_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_lut1_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_lut1_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_lut1_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_lut1_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_lut1_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_lut1_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_lut1_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_lut1_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_lut1_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_lut1_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_lut1_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("MCTF setting %d snr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            pKValue->ulMctfSetting, lSnr, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }
    else if(hAnr->pAnrSetting->iSnDbAdjust <= 130)
    {
        pKValue->ulMctfSetting = 2;
        pKValue->ulMcK0     = s_lut2_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_lut2_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_lut2_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_lut2_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_lut2_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_lut2_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_lut2_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_lut2_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_lut2_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_lut2_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_lut2_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_lut2_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_lut2_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("MCTF setting %d snr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            pKValue->ulMctfSetting, lSnr, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }
    else if(hAnr->pAnrSetting->iSnDbAdjust <= 160)
    {
        pKValue->ulMctfSetting = 3;
        pKValue->ulMcK0     = s_lut3_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_lut3_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_lut3_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_lut3_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_lut3_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_lut3_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_lut3_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_lut3_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_lut3_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_lut3_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_lut3_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_lut3_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_lut3_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("MCTF setting %d snr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            pKValue->ulMctfSetting, lSnr, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }
    else if(hAnr->pAnrSetting->iSnDbAdjust <= 190)
    {
        pKValue->ulMctfSetting = 4;
        pKValue->ulMcK0     = s_lut4_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_lut4_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_lut4_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_lut4_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_lut4_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_lut4_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_lut4_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_lut4_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_lut4_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_lut4_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_lut4_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_lut4_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_lut4_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("MCTF setting %d snr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            pKValue->ulMctfSetting, lSnr, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }
    else
    {
        pKValue->ulMctfSetting = 5;
        pKValue->ulMcK0     = s_lut5_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_lut5_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_lut5_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_lut5_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_lut5_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_lut5_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_lut5_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_lut5_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_lut5_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_lut5_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_lut5_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_lut5_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_lut5_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("MCTF setting %d snr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            pKValue->ulMctfSetting, lSnr, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }

    pKValue->ulMcK0_CH = pKValue->ulMcK0;
    pKValue->ulMcK1_CH = pKValue->ulMcK1;
    pKValue->ulNonMcK0_CH = pKValue->ulNonMcK0;
    pKValue->ulNonMcK1_CH = pKValue->ulNonMcK1;

    BSTD_UNUSED(lSnr);
    return 0;
}

static int BVDC_P_Anr_GetFilterParameters_isr
    ( BVDC_P_Anr_Handle              hAnr,
      int32_t                        lSnr,
      int32_t                        lSnrDeltaAvg,
      BVDC_P_AnrKValue              *pKValue )
{
    uint32_t ulIdx;
    int32_t  lSnr_int, lDelta;
    uint16_t  ulMmc_alpha_scale, ulMc_alpha_scale;

    if(lSnrDeltaAvg <= BVDC_P_ANR_SNR_DELTA_AVG_THR1) {
        /* ramp up the scaling for 0 < snr_delta_avg <= 2 */
        lDelta = lSnrDeltaAvg * (int32_t)((BVDC_P_ANR_SNR_REF - lSnr) / (2 * BVDC_P_ANR_SNR_DELTA_AVG_THR1));
    } else if((lSnrDeltaAvg > BVDC_P_ANR_SNR_DELTA_AVG_THR1) && (lSnrDeltaAvg <= BVDC_P_ANR_SNR_DELTA_AVG_THR2)) {
        /* taper down the scaling for 2 < snr_delta_avg <= 5 */
        lDelta = (BVDC_P_ANR_SNR_DELTA_AVG_THR2 - lSnrDeltaAvg) * (int32_t)((BVDC_P_ANR_SNR_REF - lSnr) / (2 * (BVDC_P_ANR_SNR_DELTA_AVG_THR2 - BVDC_P_ANR_SNR_DELTA_AVG_THR1)));
    } else {
        /* else, for snr_delta_avg > THR2: do no scaling */
        lDelta = 0;
    }
    /* Offset the snr by 'delta': */
    /* Note that if snr_delta_avg is always 0, then so is lDelta and lSnr_int = snr */
    lSnr_int = lSnr - lDelta;

    /* obtain the psnr index to reference appropriate elements from the MCTF lut */
    if(lSnr_int <= 0) {
        ulIdx = BVDC_P_ANR_LUT_LEN - 1;
    } else {
        for(ulIdx = 0; ulIdx < BVDC_P_ANR_LUT_LEN - 1; ulIdx += 1) {
            if (lSnr_int >= s_psnr[ulIdx]) break;
        }
    }

    BVDC_P_Anr_GetFilterParam_isr(hAnr, lSnr_int, ulIdx, pKValue);

    if (lSnrDeltaAvg <= BVDC_P_ANR_SNR_DELTA_AVG_THR1) {
        /* ramp up the scaling for 0 < snr_delta_avg <= 2 */
        pKValue->ulAlphLowThdNMC = pKValue->ulAlphLowThdNMC * (uint32_t)(((2 * BVDC_P_ANR_SNR_DELTA_AVG_THR1) - lSnrDeltaAvg) / (2 * BVDC_P_ANR_SNR_DELTA_AVG_THR1));
        pKValue->ulAlphLowThdMC  = pKValue->ulAlphLowThdMC  * (uint32_t)(((2 * BVDC_P_ANR_SNR_DELTA_AVG_THR1) - lSnrDeltaAvg) / (2 * BVDC_P_ANR_SNR_DELTA_AVG_THR1));
    } else if ((lSnrDeltaAvg > BVDC_P_ANR_SNR_DELTA_AVG_THR1) && (lSnrDeltaAvg <= BVDC_P_ANR_SNR_DELTA_AVG_THR2)) {
        /* taper down the scaling for 2 < snr_delta_avg <= 5 */
        pKValue->ulAlphLowThdNMC = pKValue->ulAlphLowThdNMC * (uint32_t)((lSnrDeltaAvg + BVDC_P_ANR_SNR_DELTA_AVG_THR2 - (2 * BVDC_P_ANR_SNR_DELTA_AVG_THR1)) / (2 * (BVDC_P_ANR_SNR_DELTA_AVG_THR2 - BVDC_P_ANR_SNR_DELTA_AVG_THR1)));
        pKValue->ulAlphLowThdMC  = pKValue->ulAlphLowThdMC  * (uint32_t)((lSnrDeltaAvg + BVDC_P_ANR_SNR_DELTA_AVG_THR2 - (2 * BVDC_P_ANR_SNR_DELTA_AVG_THR1)) / (2 * (BVDC_P_ANR_SNR_DELTA_AVG_THR2 - BVDC_P_ANR_SNR_DELTA_AVG_THR1)));
    }

    /* Scale the elements of the alpha arrays, if necessary: */
    if(BVDC_P_ANR_AUTO_SCALE == 0) { /* Register programmed scaling */
        ulMmc_alpha_scale = BVDC_P_ANR_NMC_ALPHA_LOW_SCALE;
        ulMc_alpha_scale  = BVDC_P_ANR_MC_ALPHA_LOW_SCALE;
    } else {
        /* AUTO scaling: currently a two level adjustment -> could be made more granular. */
        if ((lSnr < BVDC_P_ANR_SCALE_SNR_THR) && (lSnrDeltaAvg > 0))
            ulMc_alpha_scale = ulMmc_alpha_scale = (uint16_t)BMTH_FIX_SIGNED_FTOFIX(0.5, 20, 5);
        else
            ulMc_alpha_scale = ulMmc_alpha_scale = (uint16_t)BMTH_FIX_SIGNED_FTOFIX(1.0, 20, 5);
    }
    /* Apply scaling: */
    pKValue->ulAlphLowThdNMC = (uint16_t)BMTH_FIX_SIGNED_FIXTOI((uint32_t)(pKValue->ulAlphLowThdNMC * ulMmc_alpha_scale), 20, 5);
    pKValue->ulAlphLowThdMC  = (uint16_t)BMTH_FIX_SIGNED_FIXTOI((uint32_t)(pKValue->ulAlphLowThdMC  * ulMc_alpha_scale),  20, 5);
    return 0;
}

static void ipulse_control_01_isr
    ( BVDC_P_Anr_Handle      hAnr,
      int32_t               *plSnrAdj,
      int32_t               *plLockedNoiseCandence,
      int32_t               *plAmplitNoiseCadence )
{
    uint32_t ulPulse1 = 0;
    uint32_t ulPulse2 = 0;
    uint32_t ulPulseSelected;
    uint32_t ulPeakThreshold;
    uint32_t ulNoiseAverage;
    uint32_t ulPeakDetected;
    uint32_t ulFrameIndex;
    uint32_t ulIndexInFifo;
    uint32_t ulWraparound;
    uint32_t ulMinAmplitude;
    uint32_t ulMaxAmplitude;
    int32_t  lPulseDistanceThreshold;

    /* Average noise is calculated*/
    ulNoiseAverage = BVDC_P_Anr_ArrayMean_isr(hAnr->aulNoiseHist, BVDC_P_NOISE_HIST_NUM);
    if((hAnr->aulNoiseHist[2] >  hAnr->aulNoiseHist[0]) &&
       (hAnr->aulNoiseHist[2] >= hAnr->aulNoiseHist[4])) {
        ulPulse2 = BVDC_P_MAX(hAnr->aulNoiseHist[2] - hAnr->aulNoiseHist[0], hAnr->aulNoiseHist[2] - hAnr->aulNoiseHist[4]);
    }

    /* Check if there is a pulse on the signal */
    if((hAnr->aulNoiseHist[2] >  hAnr->aulNoiseHist[3]) &&
       (hAnr->aulNoiseHist[2] >= hAnr->aulNoiseHist[1])) {
        ulPulse1 = BVDC_P_MAX(hAnr->aulNoiseHist[2] - hAnr->aulNoiseHist[1], hAnr->aulNoiseHist[2] - hAnr->aulNoiseHist[3]);
    }
    /* Pulses that are far apart need to be treated by ulPulse2 which is more reliable in that case */
    if((hAnr->ulPeakCounter >= BVDC_P_ANR_MINIM_PEAKS_DETECTED) &&
       ((hAnr->aulPeakLocation[0] - hAnr->aulPeakLocation[1]) > BVDC_P_ANR_PEAK_DISTANCE_LARGE)) {
        ulPulseSelected = ulPulse2;
    } else {
        ulPulseSelected = ulPulse1;
    }
    /* The expected distance between pulses depends somewhat on the pulse size,
       close pulses have to be large to be accepted, more distant pulses can
       have smaller amplitude. */
    if(ulPulseSelected > ulNoiseAverage) {
        lPulseDistanceThreshold = BVDC_P_ANR_PULSE_DISTANCE_MIN;
    } else if(ulPulseSelected > (ulNoiseAverage >> 3)) {
        lPulseDistanceThreshold = BVDC_P_ANR_PULSE_DISTANCE_MIN +
            (int32_t)(((BVDC_P_ANR_PULSE_DISTANCE_MAX - BVDC_P_ANR_PULSE_DISTANCE_MIN)*(ulNoiseAverage - ulPulseSelected))/
            ((ulNoiseAverage*7) >> 3));
    }
    else {
        lPulseDistanceThreshold = BVDC_P_ANR_PULSE_DISTANCE_MAX;
    }
    BVDC_P_Anr_ArrayShift_isr(hAnr->aulFrameCount, hAnr->ulIframeCnt, BVDC_P_PEAKS_NUM);
    /* The pulse threshold is taken as 5% of the Average */
    ulPeakThreshold = (ulNoiseAverage * BVDC_P_ANR_PULSE_PERCENTAGE_OF_AVERAGE) / 100;
    ulPeakDetected = 0;
    if(hAnr->ulPeakCounter == 0) {
        ulFrameIndex = hAnr->aulFrameCount[2];
    } else {
        /* Check the wraparound 16384 */
        if(hAnr->aulFrameCount[2] < hAnr->aulPeakLocation[0]) {
            ulFrameIndex = hAnr->aulFrameCount[2] + 16384;
        } else {
            ulFrameIndex = hAnr->aulFrameCount[2];
        }
    }
    /* Check if the current pulse is not just next to another one and the pulse
       is large enough compared with the threshold */
    if(ulPulseSelected > ulPeakThreshold && ulFrameIndex > hAnr->aulPeakLocation[0] + 1) {
        BVDC_P_Anr_ArrayShift_isr(hAnr->aulPeakLocation,  ulFrameIndex,    BVDC_P_PEAKS_NUM);
        BVDC_P_Anr_ArrayShift_isr(hAnr->aulPeakAmplitude, ulPulseSelected, BVDC_P_PEAKS_NUM);

        /* the frame index maybe exceeding 16384. If it does, subtract 16384 */
        ulWraparound = 0;
        for(ulIndexInFifo = 0; ulIndexInFifo < BVDC_P_PEAKS_NUM; ulIndexInFifo++) {
            if(hAnr->aulPeakLocation[ulIndexInFifo] >= 16384) {
                ulWraparound = 1;
            }
        }
        /* If there was a wrap around just compensate for that */
        if(ulWraparound == 1) {
            for(ulIndexInFifo = 0; ulIndexInFifo < BVDC_P_PEAKS_NUM; ulIndexInFifo++) {
                hAnr->aulPeakLocation[ulIndexInFifo] = hAnr->aulPeakLocation[ulIndexInFifo] - 16384;
            }
        }
        /* one more peak is addedd to the fifo and the peak counter is incremented */
        if(hAnr->ulPeakCounter < BVDC_P_PEAKS_NUM) {
            hAnr->ulPeakCounter++;
        }
        ulPeakDetected = 1;
    }/* end adding a peak to the Fifo if the peak was detected */
    /* When the Fifo is full make a decision */
    if(hAnr->ulPeakCounter > 3) {
        /* the sync state maybe has been reached */
        if(hAnr->ulSyncReached == 1) {
            /* check if a pulse exists when is due. */
            if(ulPeakDetected == 0) {
                /* first adjust for the wrap around */
                if(hAnr->aulFrameCount[2] < hAnr->aulPeakLocation[0]) {
                    ulFrameIndex = hAnr->aulFrameCount[2] + 16384;
                } else {
                    ulFrameIndex = hAnr->aulFrameCount[2];
                }
                if((ulFrameIndex - hAnr->aulPeakLocation[0])- (hAnr->aulPeakLocation[0] - hAnr->aulPeakLocation[1]) >= BVDC_P_ANR_MAX_PULSE_DEVIATION && hAnr->ulHoldSync == 0) {
                    hAnr->ulHoldSync = 1;
                    hAnr->ulSyncReached = 0;
                    hAnr->ulWaitingCyclesToResync = BVDC_P_MIN(4*(hAnr->aulPeakLocation[1] - hAnr->aulPeakLocation[2]), 128);
                }
            } else {
                /* peak was detected but maybe too early and went into the Fifo and maybe I can be corrected */
                if((hAnr->aulPeakLocation[1] - hAnr->aulPeakLocation[2]) - (hAnr->aulPeakLocation[0] - hAnr->aulPeakLocation[1]) > BVDC_P_ANR_MAX_PULSE_DEVIATION && hAnr->ulUnexpectedPeak == 0) {
                    hAnr->ulUnexpectedPeak = 1;
                } else {
                    if(BVDC_P_ABS((int32_t)(hAnr->aulPeakLocation[0] - hAnr->aulPeakLocation[2]) - (int32_t)(hAnr->aulPeakLocation[2] - hAnr->aulPeakLocation[3])) <= BVDC_P_ANR_MAX_PULSE_DEVIATION && hAnr->ulUnexpectedPeak == 1) {
                        /* remove the pulse at index 1 which is out of sync among others that are in sync */
                        hAnr->aulPeakLocation[1] = hAnr->aulPeakLocation[2];
                        hAnr->aulPeakLocation[2] = hAnr->aulPeakLocation[3];
                    }
                    hAnr->ulUnexpectedPeak = 0;
                }
            }
        } /* end of hAnr->ulSyncReached */
        else {
            /* find the min and max value in the Fifo */
            ulMinAmplitude = hAnr->aulPeakAmplitude[0];
            ulMaxAmplitude = hAnr->aulPeakAmplitude[0];
            for(ulIndexInFifo = 1; ulIndexInFifo < BVDC_P_PEAKS_NUM; ulIndexInFifo++) {
                if(ulMinAmplitude > hAnr->aulPeakAmplitude[ulIndexInFifo]) {
                    ulMinAmplitude = hAnr->aulPeakAmplitude[ulIndexInFifo];
                }
                if(ulMaxAmplitude < hAnr->aulPeakAmplitude[ulIndexInFifo]) {
                    ulMaxAmplitude = hAnr->aulPeakAmplitude[ulIndexInFifo];
                }
            }
            if(BVDC_P_ABS((int32_t)(hAnr->aulPeakLocation[0]+ hAnr->aulPeakLocation[2] - 2 * hAnr->aulPeakLocation[1])) <= BVDC_P_ANR_MAX_PULSE_DEVIATION &&
               BVDC_P_ABS((int32_t)(hAnr->aulPeakLocation[1]+ hAnr->aulPeakLocation[3] - 2 * hAnr->aulPeakLocation[2])) <= BVDC_P_ANR_MAX_PULSE_DEVIATION &&
               (int32_t)(hAnr->aulPeakLocation[0] - hAnr->aulPeakLocation[1]) > lPulseDistanceThreshold &&
               ulMaxAmplitude - ulMinAmplitude < (ulNoiseAverage >> 2) && ulPeakDetected == 1) {
                hAnr->ulSyncReached = 1;
                hAnr->ulHoldSync = 0;
                *plLockedNoiseCandence = 1;
                *plAmplitNoiseCadence = BVDC_P_Anr_ArrayMean_isr(hAnr->aulPeakAmplitude, BVDC_P_PEAKS_NUM);
            }
            /* just wait when in the hold sync state */
            if(hAnr->ulHoldSync == 1 && hAnr->ulWaitingCyclesToResync > 0) {
                hAnr->ulWaitingCyclesToResync--;
                if(hAnr->ulWaitingCyclesToResync == 0) { /* the waiting period expired */
                    hAnr->ulHoldSync = 0;
                    hAnr->ulPeakCounter = 0;
                    hAnr->ulSyncReached = 0;
                    *plLockedNoiseCandence = 0;
                }
            }
        }
    }
    if(hAnr->ulPeakCounter > 3) {
        *plAmplitNoiseCadence = BVDC_P_Anr_ArrayMean_isr(hAnr->aulPeakAmplitude, BVDC_P_PEAKS_NUM);
    }
    *plSnrAdj = (ulNoiseAverage < 4) ? 100 : BVDC_P_ANR_LinearLog10_isr(s_ulDbConvert*4/ulNoiseAverage);
    return;
}

static int set_filter_parameters_01_isr
    ( BVDC_P_Anr_Handle              hAnr,
      int32_t                        lSnr,
      int32_t                        lLockedNoiseCadence,
      int32_t                        lAmplitNoiseCadence,
      BVDC_P_AnrKValue              *pKValue )
{
    uint32_t ulIdx;
    uint16_t ulMmc_alpha_scale, ulMc_alpha_scale;
    BSTD_UNUSED(lAmplitNoiseCadence);

    if(lSnr <= 0) {
        ulIdx = BVDC_P_ANR_LUT_LEN - 1;
    } else {
        for(ulIdx = 0; ulIdx < BVDC_P_ANR_LUT_LEN - 1; ulIdx += 1) {
            if (lSnr >= s_psnr[ulIdx]) break;
        }
    }

    if(lLockedNoiseCadence == 1) {
        ulIdx++;
    }
    if(ulIdx > BVDC_P_ANR_LUT_LEN - 1) {
        ulIdx = BVDC_P_ANR_LUT_LEN - 1;
    }

    BVDC_P_Anr_GetFilterParam_isr(hAnr, lSnr, ulIdx, pKValue);

    /* Scale the elements of the alpha arrays, if necessary: */
    if(BVDC_P_ANR_AUTO_SCALE == 0) { /* Register programmed scaling */
        ulMmc_alpha_scale = BVDC_P_ANR_NMC_ALPHA_LOW_SCALE;
        ulMc_alpha_scale  = BVDC_P_ANR_MC_ALPHA_LOW_SCALE;
    } else { /* AUTO scaling: currently a two level adjustment -> could be made more granular. */
        ulMc_alpha_scale = ulMmc_alpha_scale = (uint16_t)BMTH_FIX_SIGNED_FTOFIX(1.0, 20, 5);
    }
    /* Apply scaling: */
    pKValue->ulAlphLowThdNMC = (uint16_t)BMTH_FIX_SIGNED_FIXTOI((uint32_t)(pKValue->ulAlphLowThdNMC * ulMmc_alpha_scale), 20, 5);
    pKValue->ulAlphLowThdMC  = (uint16_t)BMTH_FIX_SIGNED_FIXTOI((uint32_t)(pKValue->ulAlphLowThdMC * ulMc_alpha_scale), 20, 5);
    return 0;
}
#else
static int BVDC_P_Anr_GetFilterParameters_isr
    ( BVDC_P_Anr_Handle              hAnr,
      int32_t                        lSnr,
      BVDC_P_AnrKValue              *pKValue )
{
    uint32_t ulIdx;

    /* obtain the psnr index to reference appropriate elements from the MCTF lut */
    if(lSnr <= 0) {
        ulIdx = BVDC_P_ANR_LUT_LEN - 1;
    } else {
        for(ulIdx = 0; ulIdx < BVDC_P_ANR_LUT_LEN - 1; ulIdx += 1) {
            if (lSnr >= s_psnr[ulIdx]) break;
        }
    }

    /* Set MCTF filtering parameters to process the next picture.  In the following comments:
      Using the MCTF parameter notation K0 ' K5 described in Section 4 of the MCTF doc */
    if(hAnr->pAnrSetting->iSnDbAdjust <= -12)
    {
        pKValue->ulMcK0     = s_low_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_low_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_low_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_low_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_low_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_low_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_low_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_low_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_low_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_low_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_low_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_low_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_low_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("LOW lSnr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            lSnr, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }
    else if(hAnr->pAnrSetting->iSnDbAdjust <= -6)
    {
        pKValue->ulMcK0     = s_medlo_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_medlo_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_medlo_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_medlo_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_medlo_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_medlo_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_medlo_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_medlo_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_medlo_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_medlo_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_medlo_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_medlo_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_medlo_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("MED LOW lSnr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            lSnr / 4, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }
    else if(hAnr->pAnrSetting->iSnDbAdjust == 0)
    {
        pKValue->ulMcK0     = s_med_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_med_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_med_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_med_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_med_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_med_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_med_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_med_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_med_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_med_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_med_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_med_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_med_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("MED lSnr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            lSnr / 4, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }
    else if(hAnr->pAnrSetting->iSnDbAdjust <= 6)
    {
        pKValue->ulMcK0     = s_medhi_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_medhi_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_medhi_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_medhi_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_medhi_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_medhi_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_medhi_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_medhi_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_medhi_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_medhi_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_medhi_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_medhi_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_medhi_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("MED HIGH lSnr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            lSnr / 4, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }
    else
    {
        pKValue->ulMcK0     = s_high_mc_iir_k0[ulIdx];   /* K0 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcK1 = s_high_mc_iir_k1[ulIdx];   /* K1 */
        else
            pKValue->ulMcK1 = s_high_mc_iir_k1[BVDC_P_ANR_FIX_MC_INDEX];   /* K1 */

        pKValue->ulNonMcK0  = s_high_nmc_iir_k0[ulIdx];   /* K2 */
        pKValue->ulNonMcK1  = s_high_nmc_iir_k1[ulIdx];   /* K3 */
        pKValue->ulFinalK0  = s_high_blend_k0[ulIdx];     /* K4 */
        pKValue->ulFinalK1  = s_high_blend_k1[ulIdx];     /* K5 */

        if(!hAnr->bFixMcCurve)
            pKValue->ulMcAdj = s_high_mc_adj[BVDC_P_ANR_FIX_MC_INDEX];
        else
            pKValue->ulMcAdj = s_high_mc_adj[ulIdx];

        pKValue->ulNonMcAdj  = s_high_nmc_adj[ulIdx];

        pKValue->ulAlphLowThdNMC = s_high_nmc_alpha_low[ulIdx];
        if(!hAnr->bFixMcCurve)
            pKValue->ulAlphLowThdMC  = s_high_mc_alpha_low[ulIdx];
        else
            pKValue->ulAlphLowThdMC  = s_high_mc_alpha_low[BVDC_P_ANR_FIX_MC_INDEX];

        BDBG_MSG(("HIGH lSnr=%d ulIdx=%d: %d %d %d %d %d %d %d %d %d %d",
            lSnr / 4, ulIdx, pKValue->ulMcK0, pKValue->ulMcK1,
            pKValue->ulNonMcK0, pKValue->ulNonMcK1,
            pKValue->ulFinalK0, pKValue->ulFinalK1,
            pKValue->ulAlphLowThdNMC, pKValue->ulAlphLowThdMC,
            pKValue->ulMcAdj, pKValue->ulNonMcAdj));

    }

    pKValue->ulMcK0_CH = pKValue->ulMcK0;
    pKValue->ulMcK1_CH = pKValue->ulMcK1;
    pKValue->ulNonMcK0_CH = pKValue->ulNonMcK0;
    pKValue->ulNonMcK1_CH = pKValue->ulNonMcK1;

    return 0;
}
#endif

void  BVDC_P_Anr_McndReview_isr
    ( BVDC_P_Anr_Handle              hAnr,
      uint32_t                      *pulNoiseSampleNum,  /* array of BVDC_P_NOISE_LEVELS => read fr BCHP_HD_ANR_AND_0_NOISY_SAMPLE_NUM_BIN_x */
      uint32_t                      *pulNoiseMsb,        /* array of BVDC_P_NOISE_LEVELS => read fr BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_x */
      uint32_t                      *pulNoiseLsb,        /* array of BVDC_P_NOISE_LEVELS => read fr BCHP_HD_ANR_AND_0_NOISE_LEVEL_0_x */
      BVDC_P_AnrKValue              *pKValue )
{
    int32_t  lSnrAdj = 0, lSnrDeltaAvg;   /* Output parameters from the BVDC_P_Anr_IpulseControl_isr() routine used to adjust MCTF filter settings */
    uint32_t ulRatio = 0, ulAvgRatio;
    uint32_t ulId;
    uint32_t ulSum, ulRepeatFrame;
    uint32_t ulNoise;
#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
    uint32_t ulLh_5 = 1;
#endif

    if(hAnr->bMcndEnable) {
        ulSum = BVDC_P_Anr_ArraySum_isr(pulNoiseSampleNum, BVDC_P_NOISE_LEVELS);

        /* If mcnd is enabled, then execute the following routine: */
        if(ulSum != 0)
            ulRatio = (1024 * pulNoiseSampleNum[0]) / ulSum;
        if(hAnr->bInitializeArray) {
            /* On second picture: initialize whole aulNoiseRatio history */
            for(ulId = 0; ulId < BVDC_P_NOISE_HIST_NUM; ulId++) {
                hAnr->aulNoiseRatio[ulId]= ulRatio;
            }
        }

        /* Use the aulNoiseRatio array to compute the average ratio & update the aulNoiseRatio array with ratio: */
        ulAvgRatio = BVDC_P_Anr_ArrayMean_isr(hAnr->aulNoiseRatio, BVDC_P_NOISE_HIST_NUM);
        ulRepeatFrame = ((ulRatio > ulAvgRatio) && hAnr->bTstRepeat) ? 1 : 0;
        BVDC_P_Anr_ArrayShift_isr(hAnr->aulNoiseRatio, ulRatio, BVDC_P_NOISE_HIST_NUM);

        if(!ulRepeatFrame) { /* Check for repeat frame: */
            hAnr->ulDropHist = 0;
        } else if (hAnr->ulDropHist < 2) {
            hAnr->ulDropHist += 1;
        } else {    /* three repeats in a row looks suspicious */
            ulRepeatFrame = 0;
        }
        /*BDBG_MSG(("sum=%d, ratio=%d, avg_ratio=%d, repeat_fr=%d, drop_hist=%d",
            ulSum, ulRatio, ulAvgRatio, ulRepeatFrame, hAnr->ulDropHist));*/

        /* Calculate the noise from pulNoiseSampleNum & noise_level arrays: */
        if(!ulRepeatFrame) {
            ulNoise = BVDC_P_Anr_CalcNoise_isr(hAnr, pulNoiseSampleNum, pulNoiseMsb, pulNoiseLsb);
            BVDC_P_Anr_ArrayShift_isr(hAnr->aulNoiseHist, ulNoise, BVDC_P_NOISE_HIST_NUM); /* Shift noise into 'BVDC_P_NOISE_HIST_NUM' deep array aulNoiseHist */

            /*BDBG_MSG(("noise=%d, init_array=%d", noise, hAnr->bInitializeArray));*/

#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
            lSnrAdj = (ulNoise < 4) ? 100 : BVDC_P_ANR_LinearLog10_isr(s_ulDbConvert*4/ulNoise);
            /* A signal to noise ration (snr) value is calculated by eliminating the factor 4 added for the 2 extra bits */
            /* At this point the meaning of Signal to Noise Ratio is relative, this is not a true SNR */
#endif

            if(hAnr->bInitializeArray) { /* On second picture: initialize whole aulNoiseHist history */
                for(ulId = 0; ulId < BVDC_P_NOISE_HIST_NUM; ulId++) {
                    hAnr->aulNoiseHist[ulId] = ulNoise;
                }
            }

            /*BDBG_MSG(("%d %d %d %d %d %d %d %d",
                hAnr->aulNoiseHist[0], hAnr->aulNoiseHist[1], hAnr->aulNoiseHist[2], hAnr->aulNoiseHist[3],
                hAnr->aulNoiseHist[4], hAnr->aulNoiseHist[5], hAnr->aulNoiseHist[6], hAnr->aulNoiseHist[7]));*/

#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
            if (0 != ulLh_5) {
                /* ulLh_5 is called that way for historical reasons (it is not known why and it should be set to 1) */
                uint32_t ulTmp = BVDC_P_Anr_ArrayMean_isr(hAnr->aulNoiseHist, BVDC_P_NOISE_HIST_NUM);
                /* The factor of 4 stems from the 2 extra LSB's that were added in the noise calculation before adding the
                   noise value to the noise history */
                lSnrAdj = (ulTmp < 4) ? 100 : BVDC_P_ANR_LinearLog10_isr(s_ulDbConvert*4/ulTmp);
                /* the difference compared with the case above is that when the ulLh_5 is set the average noise matters,
                   not the noise calculated from the last picture */
            }
#endif

            /* If enabled, perform I-pulse control: */
            if(hAnr->bIPulsingReduceEnable) {
                hAnr->ulIframeCnt++;   /* Increment the non-repeating frame count */

#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
                /* For the special case of mctf_setting 0 or 1 a special I pulse reduction method is applied*/
                if(hAnr->pAnrSetting->iSnDbAdjust <= 0 && !hAnr->bInitializeArray) {
                    int32_t lAmplitNoiseCadence = 0, lLockedNoiseCadence = 0;
                    ipulse_control_01_isr(hAnr, &lSnrAdj,&lLockedNoiseCadence,
                            &lAmplitNoiseCadence);
                    set_filter_parameters_01_isr(hAnr, lSnrAdj, lLockedNoiseCadence, lAmplitNoiseCadence, pKValue);
                }
                else {
                    BVDC_P_Anr_IpulseControl_isr(hAnr, &lSnrDeltaAvg, &lSnrAdj);
                    /* Adjust the MCTF filter parameters w/I pulsing control */
                    BVDC_P_Anr_GetFilterParameters_isr(hAnr, lSnrAdj, lSnrDeltaAvg, pKValue);
                }
#else
                BVDC_P_Anr_IpulseControl_isr(hAnr, &lSnrDeltaAvg, &lSnrAdj);
                /* Adjust the MCTF filter parameters w/I pulsing control */
                BVDC_P_Anr_GetFilterParameters_isr(hAnr, lSnrAdj, pKValue);
#endif
            } else {   /* else, adjust the MCTF filter parameters wo/I pulsing control */
                int tmp = BVDC_P_Anr_ArrayMean_isr(hAnr->aulNoiseHist, BVDC_P_NOISE_HIST_NUM);
                int tmp_snr = (tmp > 0) ? BVDC_P_ANR_LinearLog10_isr(s_ulDbConvert*4/tmp) : BVDC_P_ANR_LinearLog10_isr(0);
                /*lSnrAdj = (tmp < 4) ? 100 : (int)(20*log10(255*25.6*15*4/tmp) + 0.5);*/  /* lSnrAdj has 8.2 precision - required by the LUT */
                lSnrAdj = (tmp < 4) ? 100 : tmp_snr;
#if (BVDC_P_SUPPORT_MANR_VER >= BVDC_P_MANR_VER_5)
                BVDC_P_Anr_GetFilterParameters_isr(hAnr, lSnrAdj, 0, pKValue);
#else
                BVDC_P_Anr_GetFilterParameters_isr(hAnr, lSnrAdj, pKValue);
#endif
            } /* end ipulsing_reduce_enable condition */
            pKValue->bBypassFilter = false;
        }
        else
        {
            pKValue->bBypassFilter = true;
        }

        hAnr->bInitializeArray = false;
    }/* end bMcndEnable condition */

    return;
}

#endif /* #if (BVDC_P_SUPPORT_MANR) */
