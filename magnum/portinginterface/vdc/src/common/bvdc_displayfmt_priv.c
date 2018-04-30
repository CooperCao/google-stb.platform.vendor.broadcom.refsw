/******************************************************************************
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
 *****************************************************************************/

#include "bvdc_displayfmt_priv.h"
#include "bchp_misc.h"
#include "bchp_scl_0.h"
#include "bvdc_displayfmt_macro.h"
#include "bvdc_automation_priv.c"

BDBG_MODULE(BVDC_DISP);

/****************************************************************
 *  Tables
 ****************************************************************/

#if (BFMT_PICK_eNTSC || BFMT_PICK_eNTSC_J || BFMT_PICK_eNTSC_443 || BFMT_PICK_ePAL_60 || BFMT_PICK_e720x482_NTSC || BFMT_PICK_e720x482_NTSC_J)
static const uint32_t* const
    s_aulDtRamBVBInput_DVI_480i_DropTbl[BVDC_P_480i_DROP_TABLE_SIZE] =
{
    s_aulDviMicrocode_NTSC_J,
    BVDC_P_MAKE_DROPTBL(s_aulDtRamBVBInput_DVI_480i_Drop1)
};
#endif

static const uint32_t* const
    s_aulDtRamBVBInput_DVI_480p_DropTbl[BVDC_P_480p_DROP_TABLE_SIZE] =
{
    s_aulDviMicrocode_480p,
    BVDC_P_MAKE_DROPTBL(s_aulDtRamBVBInput_DVI_480p_Drop1)
    BVDC_P_MAKE_DROPTBL(s_aulDtRamBVBInput_DVI_480p_Drop2)
    BVDC_P_MAKE_DROPTBL(s_aulDtRamBVBInput_DVI_480p_Drop3)
};

static const uint32_t* const
    s_aulDtRamBVBInput_DVI_480p_54MHz_DropTbl[BVDC_P_480p_DROP_TABLE_SIZE] =
{
    s_aulDviMicrocode_480p54,
    BVDC_P_MAKE_DROPTBL(s_aulDtRamBVBInput_DVI_480p_Drop1_54MHz)
    BVDC_P_MAKE_DROPTBL(s_aulDtRamBVBInput_DVI_480p_Drop2_54MHz)
    BVDC_P_MAKE_DROPTBL(s_aulDtRamBVBInput_DVI_480p_Drop3_54MHz)
};

#if BVDC_P_NUM_SHARED_VF
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 2) /** { **/

/* All-pass */
static const uint32_t s_aulChFilterTbl_AllPass[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200000,
};

#if 0
/* Cr 0.6 MHz/13.5 MHz */
static const uint32_t s_aulChFilterTbl_Cr_06Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x000000FD, 0x00000A1A, 0x00000AEE, 0x00000FC4, 0x0000A58D,
    0x0000A09B, 0x0000AA1c, 0x0000AE67, 0x0000AF09, 0x0000AFA0,
};
#endif

/* Cr 1.3 Mhz/13.5 Mhz */
static const uint32_t s_aulChFilterTbl_Cr_13Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x0000000E, 0x000000A5, 0x00000040, 0x00000564, 0x00005F28,
    0x000017F9, 0x0000823D, 0x00087562, 0x000A139B, 0x000A0AFF,
};

#if 0
/* Y 4.2 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_42Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0X00000000, 0X00000000, 0X00000000, 0X00000003, 0X000005BE,
    0X00004C2C, 0x00000747, 0X0005FF95, 0X0008EF0E, 0X000FFF05,
};
#endif

#if 0
/* Y 5.5 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_55Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x00000048, 0x00000089, 0x000004EE, 0x00000187, 0x000003D9,
    0x00005E76, 0x0000A99C, 0x00049BBD, 0x0008D023, 0x00A5A55F,
};
#endif

/* Y 6.0 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_60Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x000000EF, 0x00000415, 0x00000C65, 0x00004C44, 0x000085B7,
    0x000046CD, 0x00009F0C, 0x0004ACCA, 0x0009AB77, 0x00A5F5AF,
};

/* Y 6.75 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_675Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x0000008F, 0x00000499, 0x000008B5, 0x00004D3F, 0x0000846D,
    0x00004068, 0x00008D1C, 0x0004ADAA, 0x0009B9A7, 0x00A5F505,
};

/* Cr 6.75 Mhz/27Mhz */
static const uint32_t s_aulChFilterTbl_Cr_675Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x000000F5, 0x00000000, 0x00005FFF, 0x00000000, 0x0000A50F,
    0x00000000, 0x0005FF5F, 0x00000000, 0x000AA005, 0x000F0000,
};

#if 0
/* Cr 16.56 Mhz/74.25 Mhz */
static const uint32_t s_aulChFilterTbl_Cr_1856Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x00000031, 0X00000000, 0x00001333, 0X00000000, 0x00002103,
    0X00000000, 0x00013313, 0X00000000, 0x00022001, 0x00030000,
};
#endif

/* Cr 12 Mhz/54 Mhz */
static const uint32_t s_aulChFilterTbl_Y_12Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x000000EF, 0X00000415, 0x00000C65, 0X00004C44, 0x000085B7,
    0X000046CD, 0x00009F0C, 0X0004ACCA, 0x0009AB77, 0x00A5F5AF,
};

/* Cr SD */
static const uint32_t s_aulChFilterTbl_Cr_SD[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x00000085, 0x000000F0, 0x00000409, 0x0000173E, 0x00008656,
    0x00002940, 0x0004D8B3, 0x00013B0A, 0x000ADEA5, 0x000F0FFA,
};

/* Cr SECAM */
static const uint32_t s_aulChFilterTbl_Cr_SECAM[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x000000A0, 0X00000020, 0x00000490, 0X00004D90, 0x00005FE0,
    0X00000180, 0x00008B20, 0X00087450, 0x000A16F0, 0x000A00F0,
};

#else /** } { VF_VER **/

/* All-pass */
static const uint32_t s_aulChFilterTbl_AllPass[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00200000,
};

#if 0
/* Cr 0.6 MHz/13.5 MHz */
static const uint32_t s_aulChFilterTbl_Cr_06Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x000000FD, 0x00000A1A, 0x00000AEE, 0x00000FC4, 0x0000A58D,
    0x0000A09B, 0x0000AA1c, 0x0000AE67, 0x0000AF09, 0x0000AFA0,
};
#endif

/* Cr 1.3 Mhz/13.5 Mhz */
static const uint32_t s_aulChFilterTbl_Cr_13Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x0000000E, 0x000000A5, 0x00000040, 0x00000564, 0x00005F28,
    0x000017F9, 0x0000823D, 0x00087562, 0x000A139B, 0x000A0AFF,
};

#if 0
/* Y 4.2 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_42Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0X00000000, 0X00000000, 0X00000000, 0X00000003, 0X000005BE,
    0X00004C2C, 0x00000747, 0X0005FF95, 0X0008EF0E, 0X000FFF05,
};
#endif

#if 0
/* Y 5.5 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_55Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x00000048, 0x00000089, 0x000004EE, 0x00000187, 0x000003D9,
    0x00005E76, 0x0000A99C, 0x00049BBD, 0x0008D023, 0x00A5A55F,
};
#endif

/* Y 6.0 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_60Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x000000EF, 0x00000415, 0x00000C65, 0x00004C44, 0x000085B7,
    0x000046CD, 0x00009F0C, 0x0004ACCA, 0x0009AB77, 0x00A5F5AF,
};

/* Y 6.75 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_675Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x0000008F, 0x00000499, 0x000008B5, 0x00004D3F, 0x0000846D,
    0x00004068, 0x00008D1C, 0x0004ADAA, 0x0009B9A7, 0x00A5F505,
};

/* Cr 6.75 Mhz/27Mhz */
static const uint32_t s_aulChFilterTbl_Cr_675Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x000000F5, 0x00000000, 0x00005FFF, 0x00000000, 0x0000A50F,
    0x00000000, 0x0005FF5F, 0x00000000, 0x000AA005, 0x000F0000,
};

#if 0
/* Cr 16.56 Mhz/74.25 Mhz */
static const uint32_t s_aulChFilterTbl_Cr_1856Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x00000031, 0X00000000, 0x00001333, 0X00000000, 0x00002103,
    0X00000000, 0x00013313, 0X00000000, 0x00022001, 0x00030000,
};
#endif

/* Cr 12 Mhz/54 Mhz */
static const uint32_t s_aulChFilterTbl_Y_12Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x000000EF, 0X00000415, 0x00000C65, 0X00004C44, 0x000085B7,
    0X000046CD, 0x00009F0C, 0X0004ACCA, 0x0009AB77, 0x00A5F5AF,
};

/* Cr SD */
static const uint32_t s_aulChFilterTbl_Cr_SD[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x00000085, 0x000000F0, 0x00000409, 0x0000173E, 0x00008656,
    0x00002940, 0x0004D8B3, 0x00013B0A, 0x000ADEA5, 0x000F0FFA,
};

/* Cr SECAM */
static const uint32_t s_aulChFilterTbl_Cr_SECAM[BVDC_P_CHROMA_TABLE_SIZE] =
{
    0x000000A0, 0X00000020, 0x00000490, 0X00004D90, 0x00005FE0,
    0X00000180, 0x00008B20, 0X00087450, 0x000A16F0, 0x000A00F0,
};

#endif /** } VF_VER **/
#endif /* BVDC_P_NUM_SHARED_VF */

#if BVDC_P_NUM_SHARED_SDSRC
/* SRC Control Filter Modes */
static const uint32_t s_ulSrcControlNotRGB =
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, HSYNC,    Minus_6dB) |
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, H_CLAMP,   UNSIGNED) |
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, CH12_CLAMP,  SIGNED) |
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, CH0_CLAMP, UNSIGNED) |
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, CH1_2,    Minus_6dB) |
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, CH0,   Minus_11dB);
static const uint32_t s_ulSrcControlRGB =
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, HSYNC,    Minus_6dB) |
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, H_CLAMP,   UNSIGNED) |
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, CH12_CLAMP,UNSIGNED) |
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, CH0_CLAMP, UNSIGNED) |
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, CH1_2,    Minus_6dB) |
    BVDC_P_SRC_FIELD_ENUM(SRC_CONTROL, CH0,   Minus_11dB);
#endif

/*
 * Definitions needed for HDMI RM table
 */

typedef struct
{
    uint64_t                    ulPixelBaseRate;
    BFMT_ClockMod               eClockMod;
    BAVC_HDMI_PixelRepetition   eHdmiPixelRepetition; /* none, 1x, or 4x */
    BAVC_HDMI_BitsPerPixel      eHdmiColorDepth;      /* 24, 30, 36, or 48 (future) */
    BVDC_P_TmdsClock            eTmdsClock;
}
BVDC_P_HdmiRmLookup;

#if (BVDC_P_SUPPORT_DVI_40NM)
static const BVDC_P_HdmiRmLookup s_HdmiLU[] =
{
#include "bvdc_hdmirm_tmds_lookup_40nm_priv.h"
};
#elif (BVDC_P_SUPPORT_DVI_28NM)
static const BVDC_P_HdmiRmLookup s_HdmiLU[] =
{
#include "bvdc_hdmirm_tmds_lookup_28nm_priv.h"
};
#else
#error Unknown/undefined HDMI Rate Manager hardware version
#endif
#define BVDC_P_RM_LU_ENTRIES \
    (sizeof(s_HdmiLU) / sizeof(BVDC_P_HdmiRmLookup))

/* HDMI Rate Manager */
static const BVDC_P_RateInfo s_HdmiRm[] =
{
#if (BVDC_P_SUPPORT_DVI_40NM)

#include "bvdc_hdmirm_40nm_priv.h"

#elif (BVDC_P_SUPPORT_DVI_28NM)

#include "bvdc_hdmirm_28nm_priv.h"

#else
#error Unknown/undefined HDMI Rate Manager hardware version
#endif
};
#define BVDC_P_RM_TABLE_ENTRIES \
    (sizeof(s_HdmiRm) / sizeof(BVDC_P_RateInfo))

#if (BVDC_P_SUPPORT_HDMI_RM_VER <= BVDC_P_HDMI_RM_VER_6)
#define BVDC_P_MAKE_RM(denom, num, sample, phase, integrator)  \
    BVDC_P_RM_FIELD_DATA(RATE_RATIO, DENOMINATOR,     denom),  \
    BVDC_P_RM_FIELD_DATA(SAMPLE_INC, NUMERATOR,         num) | \
    BVDC_P_RM_FIELD_DATA(SAMPLE_INC, reserved0,           0) | \
    BVDC_P_RM_FIELD_DATA(SAMPLE_INC, SAMPLE_INC,     sample),  \
    BVDC_P_RM_FIELD_DATA(PHASE_INC,  reserved0,           0) | \
    BVDC_P_RM_FIELD_DATA(PHASE_INC,  PHASE_INC,       phase),  \
    BVDC_P_RM_FIELD_DATA(INTEGRATOR, INTEGRATOR, integrator)
#else
#define BVDC_P_MAKE_RMHL(denom, num, sample, phase, integrator_lo, integrator_hi)  \
    BVDC_P_RM_FIELD_DATA(RATE_RATIO, DENOMINATOR,     denom),                      \
    BVDC_P_RM_FIELD_DATA(SAMPLE_INC, NUMERATOR,         num)                     | \
    BVDC_P_RM_FIELD_DATA(SAMPLE_INC, SAMPLE_INC,     sample),                      \
    BVDC_P_RM_FIELD_DATA(PHASE_INC,  reserved0,           0)                     | \
    BVDC_P_RM_FIELD_DATA(PHASE_INC,  PHASE_INC,       phase),                      \
    BVDC_P_RM_FIELD_DATA(INTEGRATOR_LO, INTEGRATOR_LO, integrator_lo),             \
    BVDC_P_RM_FIELD_DATA(INTEGRATOR_HI, INTEGRATOR_HI, integrator_hi)

#define BVDC_P_MAKE_RM(denom, num, sample, phase, integrator_lo)  \
    BVDC_P_MAKE_RMHL(denom, num, sample, phase, integrator_lo, 0)
#endif

#if BFMT_PICK_PXL_25_2MHz
static const uint32_t s_aulRmTable_25_2[BVDC_P_RM_TABLE_SIZE]             = { BVDC_P_MAKE_RM(32760,  2340,    1, 0x1DDDDE, 0) }; /* 25.2MHz for Vesa 640x480p */
#endif

#if BFMT_PICK_PXL_25_2MHz_DIV_1_001
static const uint32_t s_aulRmTable_25_2_Div_1001[BVDC_P_RM_TABLE_SIZE]    = { BVDC_P_MAKE_RM(32400,  2349,    1, 0x1DD63A, 0) }; /* 25.2/1.001 Mhz for Vesa 640x480p */
#endif

static const uint32_t s_aulRmTable_27[BVDC_P_RM_TABLE_SIZE]               = { BVDC_P_MAKE_RM(32764,     0,    1, 0x200000, 0) }; /* 27Mhz for NTSC, NTSC_J, PAL, PAL_M, PAL_N, PAL_NC, 480p, 576p_50Hz */
static const uint32_t s_aulRmTable_27_Mul_1001[BVDC_P_RM_TABLE_SIZE]      = { BVDC_P_MAKE_RM(32032, 32000,    0, 0x200831, 0) }; /* 27*1.001 MHz for 480p @60Hz */

#if BFMT_PICK_PXL_39_79MHz
static const uint32_t s_aulRmTable_39_79[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(27632, 18750,    0, 0x2F289A, 0) }; /* 39.790080 Mhz for Vesa 00x600p @60Hz refresh rate */
#endif

#if BFMT_PICK_PXL_39_79MHz_DIV_1_001
static const uint32_t s_aulRmTable_39_79_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(30144, 20475,    0, 0x2F1C8B, 0) }; /* 39.790080/1.001 Mhz for Vesa 800x600p @59.94Hz refresh rate */
#endif

#if BFMT_PICK_PXL_60_4656MHz
static const uint32_t s_aulRmTable_60_4656[BVDC_P_RM_TABLE_SIZE]          = { BVDC_P_MAKE_RM(29393, 13125,    0,  0x47A9B, 0) }; /* 60.4656 Mhz for Vesa 1280x720p @50Hz refresh rate */
#endif

#if BFMT_PICK_PXL_64_022MHz
static const uint32_t s_aulRmTable_64_0224[BVDC_P_RM_TABLE_SIZE]          = { BVDC_P_MAKE_RM(32604, 13750,    0, 0x4BE0DF, 0) }; /* 64.0224 Mhz for Vesa 1280x720p Reduced Blanking @60Hz refresh rate */
#endif

#if BFMT_PICK_PXL_64_022MHz_DIV_1_001
static const uint32_t s_aulRmTable_64_0224_Div_1001[BVDC_P_RM_TABLE_SIZE] = { BVDC_P_MAKE_RM(31920, 13475,    0, 0x4BCD77, 0) }; /* 64.0224/1.001 Mhz for Vesa 1280x720p Reduced Blanking @59.94Hz refresh rate */
#endif

#if BFMT_PICK_PXL_65MHz
static const uint32_t s_aulRmTable_65[BVDC_P_RM_TABLE_SIZE]               = { BVDC_P_MAKE_RM(32760, 13608,    0, 0x4D097B, 0) }; /* 64.995840 Mhz for Vesa 1024x768p @ 60Hz */
#endif

#if BFMT_PICK_PXL_65MHz_DIV_1_001
static const uint32_t s_aulRmTable_65_Div_1001[BVDC_P_RM_TABLE_SIZE]      = { BVDC_P_MAKE_RM(30000, 12474,    0, 0x4CF5C7, 0) }; /* 64.995840/1.001 Mhz for Vesa 1024x768p @ 59.94Hz */
#endif

#if BFMT_PICK_PXL_65_286MHz
static const uint32_t s_aulRmTable_65_286[BVDC_P_RM_TABLE_SIZE]           = { BVDC_P_MAKE_RM(32643, 13500,    0, 0x4D6041, 0) }; /* 65.286 Mhz for Vesa 1280x768 @60Hz refresh rate */
#endif

#if BFMT_PICK_PXL_65_286MHz_DIV_1_001
static const uint32_t s_aulRmTable_65_286_Div_1001[BVDC_P_RM_TABLE_SIZE]  = { BVDC_P_MAKE_RM(32736, 13552,    0, 0x4D4C77, 0) }; /* 65.286/1.001 Mhz for Vesa 1280x768 @59.94Hz refresh rate */
#endif

#if BFMT_PICK_PXL_74_25MHz
static const uint32_t s_aulRmTable_74_25[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(32758, 11912,    0, 0x580000, 0x0a02530) }; /* 74.25Mhz (1080i, 720p) */
#endif

#if BFMT_PICK_PXL_74_25MHz_DIV_1_001
static const uint32_t s_aulRmTable_74_25_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(32750, 11921,    0, 0x57e97f, 0x229e910) }; /* 74.25/1.001 Mhz sample rate, 720p_5994Hz or 1080i_2997Hz */
#endif

#if 0
static const uint32_t s_aulRmTable_74_48[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(77584, 28125,    0, 0x5845FA, 0) }; /* 74.480640 Mhz for vesa 1280x720p @60Hz refresh rate */
static const uint32_t s_aulRmTable_74_48_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(47744, 17325,    0, 0x582F67, 0) }; /* 74.406234 Mhz for vesa 1280x720p @59.94Hz refresh rate */
#endif

#if BFMT_PICK_PXL_148_5MHz
static const uint32_t s_aulRmTable_148_5[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(32758,  5956,    0, 0xB00000, 0) }; /* 148.5 Mhz for 1080p @60Hz refresh rate */
#endif

#if BFMT_PICK_PXL_148_5MHz_DIV_1_001
static const uint32_t s_aulRmTable_148_5_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(32500,  5915,    0, 0xAFD2FD, 0) }; /* 148.5 Mhz for 1080p @59.94Hz refresh rate */
#endif

#if BFMT_PICK_PXL_162MHz
static const uint32_t s_aulRmTable_162[BVDC_P_RM_TABLE_SIZE]              = { BVDC_P_MAKE_RM(32766,  5461,    0, 0xc00000, 0) }; /* 162 Mhz for 1600x1200p @60Hz refresh rate */
#endif

#if BFMT_PICK_PXL_67_565MHz
static const uint32_t s_aulRmTable_67_565[BVDC_P_RM_TABLE_SIZE]           = { BVDC_P_MAKE_RM(31280, 12500,    0, 0x5013A9, 0xCD870) }; /* 67.565 Mhz sample rate, 1366x768_60Hz */
#endif

#if BFMT_PICK_PXL_67_565MHz_MUL_1_001
static const uint32_t s_aulRmTable_67_565_Div_1001[BVDC_P_RM_TABLE_SIZE]  = { BVDC_P_MAKE_RM(25024, 10010,    0, 0x4FFF2E, 0xCD870) }; /* 67.565/1.001 Mhz sample rate, 1366x768_5994Hz */
#endif

#if BFMT_PICK_PXL_56_304MHz
static const uint32_t s_aulRmTable_56_304[BVDC_P_RM_TABLE_SIZE]           = { BVDC_P_MAKE_RM(32062, 15375,    0, 0x42BB0C, 0xCD870) }; /* 56.304 Mhz sample rate, 1366x768_50Hz */
#endif

#if BFMT_PICK_PXL_297MHz
static const uint32_t s_aulRmTable_297[BVDC_P_RM_TABLE_SIZE]              = { BVDC_P_MAKE_RM(32758,  2978,    0, 0x1600000, 0) }; /* 297 Mhz for 4kx2k @60Hz refresh rate */
#endif

#if BFMT_PICK_PXL_297MHz_DIV_1_001
static const uint32_t s_aulRmTable_297_Div_1001[BVDC_P_RM_TABLE_SIZE]     = { BVDC_P_MAKE_RM(32000,  2912,    0, 0x15FA5F9, 0) }; /* 297/1.001 Mhz for 4kx2k @59.94Hz refresh rate */
#endif

#if BFMT_PICK_PXL_23_75MHz
static const uint32_t s_aulRmTable_23_75[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(32680,  4472,  0x1, 0x1C25ED, 0) }; /* 23.75 Mhz sample rate, VESA 640x480p_CVT @60Hz */
#endif

#if BFMT_PICK_PXL_23_75MHz_DIV_1_001
static const uint32_t s_aulRmTable_23_75_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(23750,  3277,  0x1, 0x1C1EBA, 0) }; /* 23.75/1.001 Mhz sample rate, VESA 640x480p_CVT @60Hz */
#endif

#if BFMT_PICK_PXL_83_5MHz
static const uint32_t s_aulRmTable_83_5[BVDC_P_RM_TABLE_SIZE]             = { BVDC_P_MAKE_RM(32732, 10584,  0x0, 0x62F684, 0) }; /* 83.5 Mhz sample rate, VESA 1280x800_60Hz */
#endif

#if BFMT_PICK_PXL_83_5MHz_DIV_1_001
static const uint32_t s_aulRmTable_83_5_Div_1001[BVDC_P_RM_TABLE_SIZE]    = { BVDC_P_MAKE_RM(83500, 27027,  0x0, 0x62DD35, 0) }; /* 83.5/1.001 Mhz sample rate, VESA 1280x800_60Hz */
#endif

#if BFMT_PICK_PXL_108MHz
static const uint32_t s_aulRmTable_108[BVDC_P_RM_TABLE_SIZE]              = { BVDC_P_MAKE_RM(32764,  8191,  0x0, 0x800000, 0) }; /* 108 Mhz sample rate, VESA 1280x1024_60Hz */
#endif

#if BFMT_PICK_PXL_108MHz_DIV_1_001
static const uint32_t s_aulRmTable_108_Div_1001[BVDC_P_RM_TABLE_SIZE]     = { BVDC_P_MAKE_RM(32000,  8008,  0x0, 0x7fdf43, 0) }; /* 108/1.001 Mhz sample rate, VESA 1280x1024_60Hz */
#endif

#if BFMT_PICK_PXL_85_5MHz
static const uint32_t s_aulRmTable_85_5[BVDC_P_RM_TABLE_SIZE]             = { BVDC_P_MAKE_RM(32756, 10344,  0x0, 0x655555, 0) }; /* 85.5 Mhz sample rate, VESA 1366x768_60Hz */
#endif

#if BFMT_PICK_PXL_85_5MHz_DIV_1_001
static const uint32_t s_aulRmTable_85_5_Div_1001[BVDC_P_RM_TABLE_SIZE]    = { BVDC_P_MAKE_RM(28500,  9009,  0x0, 0x653B6A, 0) }; /* 85.5/1.001 Mhz sample rate, VESA 1366x768_60Hz */
#endif

#if BFMT_PICK_PXL_106_5MHz
static const uint32_t s_aulRmTable_106_5[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(32731,  8298,  0x0, 0x7E38E3, 0) }; /* 106.5 Mhz sample rate, VESA 1440x900_60Hz */
#endif

#if BFMT_PICK_PXL_106_5MHz_DIV_1_001
static const uint32_t s_aulRmTable_106_5_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(35500,  9009,  0x0, 0x7E189B, 0) }; /* 106.5/1.001 Mhz sample rate, VESA 1440x900_60Hz */
#endif

static const uint32_t s_aulRmTable_54[BVDC_P_RM_TABLE_SIZE]               = { BVDC_P_MAKE_RM(32766, 16383,    0, 0x400000, 0) }; /* 54Mhz for (480P or 576P) at double rate */
static const uint32_t s_aulRmTable_54_Mul_1001[BVDC_P_RM_TABLE_SIZE]      = { BVDC_P_MAKE_RM(32032, 16000,    0, 0x401062, 0) }; /* 54Mhz for 480P at double rate */

#if BVDC_P_NUM_SHARED_SM
#define BVDC_P_MAKE_SM(pixel_frac_en, init_phase, phase_offset, u_coring_en, \
    v_coring_en, fre0, chroma_offset, comp_out_sel, combine_chroma,          \
    sin_coef_en, cos_coef_en)                                                \
    BVDC_P_SM_FIELD_ENUM(PG_CNTRL, PIXEL_FRAC_ENABLE,      pixel_frac_en) |  \
    BVDC_P_SM_FIELD_DATA(PG_CNTRL, INIT_PHASE,                init_phase) |  \
    BVDC_P_SM_FIELD_DATA(PG_CNTRL, ACTIVE_PHASE_OFFSET,     phase_offset),   \
    BVDC_P_SM_FIELD_DATA(PG_CONFIG, U_CORING_ENABLE,         u_coring_en) |  \
    BVDC_P_SM_FIELD_DATA(PG_CONFIG, V_CORING_ENABLE,         v_coring_en),   \
    BVDC_P_SM_FIELD_DATA(SC_FREQ_0, FRE0,                           fre0),   \
    BVDC_P_SM_FIELD_DATA(COMP_CNTRL, CHROMA_OFFSET,        chroma_offset) |  \
    BVDC_P_SM_FIELD_ENUM(COMP_CNTRL, COMP_OUT_SEL,          comp_out_sel) |  \
    BVDC_P_SM_FIELD_ENUM(COMP_CNTRL, COMBINE_CHROMA,      combine_chroma) |  \
    BVDC_P_SM_FIELD_ENUM(COMP_CNTRL, SIN_COEF_EN,            sin_coef_en) |  \
    BVDC_P_SM_FIELD_ENUM(COMP_CNTRL, COS_COEF_EN,            cos_coef_en)

/* SM values for NTSC SVideo/CVBS */
static const uint32_t s_aulSmTable_YQI[BVDC_P_SM_TABLE_SIZE] =
{
#if DCS_SUPPORT
    BVDC_P_MAKE_SM(
        ON, 0x86,  0x005e, 1, 1, 0x21f07c1f, 0x400, COMPOSITE, ON, USE_SIN,
        USE_COS),
#else
    BVDC_P_MAKE_SM(
        ON, 0x1CC, 0x005e, 1, 1, 0x21f07c1f, 0x400, COMPOSITE, ON, USE_SIN,
        USE_COS),
#endif
};

/* Same, but for NTSC-704 */
static const uint32_t s_aulSmTable_YQI_704[BVDC_P_SM_TABLE_SIZE] =
{
#if DCS_SUPPORT
    BVDC_P_MAKE_SM(
        ON, 0x86,  0x005e, 1, 1, 0x21f07c1f, 0x400, COMPOSITE, ON, USE_SIN,
        USE_COS),
#else
    BVDC_P_MAKE_SM(
        ON,  0x3E, 0x005e, 1, 1, 0x21f07c1f, 0x400, COMPOSITE, ON, USE_SIN,
        USE_COS),
#endif
};

/* PAL_M SVideo/CVBS */
static const uint32_t s_aulSmTable_PALM_YUV[BVDC_P_SM_TABLE_SIZE] =
{
    BVDC_P_MAKE_SM(ON, 0x22A, 0x0, 1, 1, 0x21E6EFA4, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};

#if BFMT_PICK_ePAL_NC
/* PAL_NC SVideo/CVBS */
static const uint32_t s_aulSmTable_PALNC_YUV[BVDC_P_SM_TABLE_SIZE] =
{
    BVDC_P_MAKE_SM(ON,  0xCB, 0x0, 1, 1, 0x21F69447, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};
#endif

#if BFMT_PICK_ePAL_N
/* PAL_N SVideo/CVBS */
static const uint32_t s_aulSmTable_PALN_YUV[BVDC_P_SM_TABLE_SIZE] =
{
    BVDC_P_MAKE_SM(
        ON, 0xFB, 0x0, 1, 1, 0x2A098ACB, 0x400, COMPOSITE, ON, USE_SIN,
        USE_COS),
};
#endif

#if BFMT_PICK_ePAL_I
/* PAL_I SVideo/CVBS */
static const uint32_t s_aulSmTable_PALI_YUV[BVDC_P_SM_TABLE_SIZE] =
{
#if DCS_SUPPORT
    BVDC_P_MAKE_SM(
        ON, 0x1D,  0x0, 1, 1, 0x2A098ACB, 0x400, COMPOSITE, ON, USE_SIN,
        USE_COS),
#else
    BVDC_P_MAKE_SM(
        ON, 0x1A0, 0x0, 1, 1, 0x2A098ACB, 0x400, COMPOSITE, ON, USE_SIN,
        USE_COS),
#endif
};
#endif

/* All other PAL SVideo/CVBS */
static const uint32_t s_aulSmTable_PAL_YUV[BVDC_P_SM_TABLE_SIZE] =
{
#if DCS_SUPPORT
    BVDC_P_MAKE_SM(
        ON, 0x1D,  0x0, 1, 1, 0x2A098ACB, 0x400, COMPOSITE, ON, USE_SIN,
        USE_COS),
#else
    BVDC_P_MAKE_SM(
        ON, 0x179, 0x0, 1, 1, 0x2A098ACB, 0x400, COMPOSITE, ON, USE_SIN,
        USE_COS),
#endif
};

#if BFMT_PICK_eSECAM_L
/* SECAM SVideo/CVBS */
static const uint32_t s_aulSmTable_SECAM[BVDC_P_SM_TABLE_SIZE] =
{
    BVDC_P_MAKE_SM(ON, 0, 0x0, 0, 0, 0x284BDA12, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};
#endif

/* SM values for Component output */
static const uint32_t s_aulSmTable_Component[BVDC_P_SM_TABLE_SIZE] =
{
    BVDC_P_MAKE_SM(OFF, 0, 0x0, 0, 0, 0, 0, COMPONENT, OFF, USE_ONE, USE_ONE),
};
#endif

/****************************************************************
 *  Format Tables
 ****************************************************************/
static const BVDC_P_FormatInfo s_aFormatInfoTable[] =
{
    /* Format                                     HD     ED     SD     VESA   Prog   MacVin DCS    HDMI   Use    {Shaper}
                                                                                                          DropTbl          */
    BVDC_P_MAKE_FMTINFO(eNTSC,                    false, false, true,  false, false, true,  true,  true,  true , 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(eNTSC_J,                  false, false, true,  false, false, true,  true,  true,  true , 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(eNTSC_443,                false, false, true,  false, false, false, false, true,  true , 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(ePAL_B,                   false, false, true,  false, false, true,  true,  true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_B1,                  false, false, true,  false, false, true,  true,  true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_D,                   false, false, true,  false, false, true,  true,  true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_D1,                  false, false, true,  false, false, true,  true,  true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_G,                   false, false, true,  false, false, true,  true,  true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_H,                   false, false, true,  false, false, true,  true,  true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_K,                   false, false, true,  false, false, true,  true,  true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_I,                   false, false, true,  false, false, true,  true,  true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_M,                   false, false, true,  false, false, true,  false, true,  false, 3, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_N,                   false, false, true,  false, false, true,  false, true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_NC,                  false, false, true,  false, false, true,  true,  true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(ePAL_60,                  false, false, true,  false, false, false, false, true,  true , 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(eSECAM_L,                 false, false, true,  false, false, true,  false, true,  false, 9, 6, 3)
    BVDC_P_MAKE_FMTINFO(eSECAM_B,                 false, false, true,  false, false, true,  false, true,  false, 9, 6, 3)
    BVDC_P_MAKE_FMTINFO(eSECAM_G,                 false, false, true,  false, false, true,  false, true,  false, 9, 6, 3)
    BVDC_P_MAKE_FMTINFO(eSECAM_D,                 false, false, true,  false, false, true,  false, true,  false, 9, 6, 3)
    BVDC_P_MAKE_FMTINFO(eSECAM_K,                 false, false, true,  false, false, true,  false, true,  false, 9, 6, 3)
    BVDC_P_MAKE_FMTINFO(eSECAM_H,                 false, false, true,  false, false, true,  false, true,  false, 9, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080i,                   true,  false, false, false, false, false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080p,                   true,  false, false, false, true,  false, false, true,  false, 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720p,                    true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720p_60Hz_3DOU_AS,       true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720p_50Hz_3DOU_AS,       true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720p_30Hz_3DOU_AS,       true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720p_24Hz_3DOU_AS,       true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e480p,                    false, true,  false, false, true,  true,  true,  true,  true , 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080i_50Hz,              true,  false, false, false, false, false, false, true,  false, 0, 2, 0)
    BVDC_P_MAKE_FMTINFO(e1080p_24Hz_3DOU_AS,      true,  false, false, false, true,  false, false, true,  false, 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080p_30Hz_3DOU_AS,      true,  false, false, false, true,  false, false, true,  false, 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080p_60Hz_3DOU_AS,      true,  false, false, false, true,  false, false, true,  false, 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080p_60Hz_3DLR,         true,  false, false, false, true,  false, false, true,  false, 3, 6, 3)

    BVDC_P_MAKE_FMTINFO(e1080p_24Hz,              true,  false, false, false, true,  false, false, true,  false, 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080p_25Hz,              true,  false, false, false, true,  false, false, true,  false, 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080p_30Hz,              true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080p_50Hz,              true,  false, false, false, true,  false, false, true,  false, 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080p_100Hz,             true,  false, false, false, true,  false, false, true,  false, 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1080p_120Hz,             true,  false, false, false, true,  false, false, true,  false, 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1250i_50Hz,              true,  false, false, false, false, false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720p_24Hz,               true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720p_25Hz,               true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720p_30Hz,               true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720p_50Hz,               true,  false, false, false, true,  false, false, true,  false, 0, 2, 0)
    BVDC_P_MAKE_FMTINFO(e576p_50Hz,               false, true,  false, false, true,  true,  true,  true,  false, 9, 2, 0)
    BVDC_P_MAKE_FMTINFO(e240p_60Hz,               false, false, true,  false, true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e288p_50Hz,               false, false, true,  false, true,  false, false, false, false, 9, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1440x480p_60Hz,          false, false, true,  false, true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e1440x576p_50Hz,          false, false, true,  false, true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e3840x2160p_24Hz,         true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e3840x2160p_25Hz,         true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e3840x2160p_30Hz,         true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e3840x2160p_50Hz,         true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e3840x2160p_60Hz,         true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e4096x2160p_24Hz,         true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e4096x2160p_25Hz,         true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e4096x2160p_30Hz,         true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e4096x2160p_50Hz,         true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(e4096x2160p_60Hz,         true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
#if BFMT_LEGACY_3DTV_SUPPORT
    BVDC_P_MAKE_FMTINFO(eCUSTOM1920x2160i_48Hz,   true,  false, false, false, false, false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eCUSTOM1920x2160i_60Hz,   true,  false, false, false, false, false, false, true,  false, 0, 6, 3)
#endif
    BVDC_P_MAKE_FMTINFO(eCUSTOM_1440x240p_60Hz,   false, false, true,  false, true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eCUSTOM_1440x288p_50Hz,   false, false, true,  false, true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eCUSTOM_1366x768p,        true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eCUSTOM_1366x768p_50Hz,   true,  false, false, false, true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x480p,            true,  false, false, true,  true,  false, false, true,  false, 0, 9, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_640x480p_CVT,        true,  false, false, true,  true,  false, false, true,  false, 0, 9, 0)

    BVDC_P_MAKE_FMTINFO(eDVI_800x600p,            true,  false, false, true,  true,  false, false, true,  false, 0, 9, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1024x768p,           true,  false, false, true,  true,  false, false, true,  false, 0, 9, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x768p,           true,  false, false, true,  true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x768p_Red,       true,  false, false, true,  true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x720p_50Hz,      true,  false, false, true,  true,  false, false, true,  false, 0, 9, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x720p,           true,  false, false, true,  true,  false, false, true,  false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x720p_Red,       true,  false, false, true,  true,  false, false, true,  false, 0, 6, 3)

    /* PC input format! */
    BVDC_P_MAKE_FMTINFO(eDVI_640x350p_60Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x350p_70Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x350p_72Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x350p_75Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x350p_85Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)

    BVDC_P_MAKE_FMTINFO(eDVI_640x400p_60Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x400p_70Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x400p_72Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x400p_75Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x400p_85Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)

    BVDC_P_MAKE_FMTINFO(eDVI_640x480p_66Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x480p_70Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x480p_72Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x480p_75Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_640x480p_85Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)

    BVDC_P_MAKE_FMTINFO(eDVI_720x400p_60Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_720x400p_70Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_720x400p_72Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_720x400p_75Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_720x400p_85Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)

    BVDC_P_MAKE_FMTINFO(eDVI_800x600p_56Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_800x600p_59Hz_Red,   true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_800x600p_70Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_800x600p_72Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_800x600p_75Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_800x600p_85Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)

    BVDC_P_MAKE_FMTINFO(eDVI_1024x768p_66Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_1024x768p_70Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_1024x768p_72Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_1024x768p_75Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_1024x768p_85Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)

    BVDC_P_MAKE_FMTINFO(eDVI_1280x720p_70Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x720p_72Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x720p_75Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x720p_85Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 6, 3)

    /* New DVI or PC vdec input support */
    BVDC_P_MAKE_FMTINFO(eDVI_1024x768i_87Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1152x864p_75Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x768p_75Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x768p_85Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x800p_60Hz,      true,  false, false, true,  true,  false, false, true,  false, 0, 9, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x960p_60Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x960p_85Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x1024p_60Hz,     true,  false, false, true,  true,  false, false, true,  false, 0, 9, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x1024p_69Hz,     true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x1024p_75Hz,     true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1280x1024p_85Hz,     true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_832x624p_75Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1360x768p_60Hz,      true,  false, false, true,  true,  false, false, true,  false, 0, 9, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1366x768p_60Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 9, 9)
    BVDC_P_MAKE_FMTINFO(eDVI_1400x1050p_60Hz_Red, true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1400x1050p_60Hz,     true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1400x1050p_75Hz,     true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1600x1200p_60Hz,     true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1920x1080p_60Hz_Red, true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_848x480p_60Hz,       true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1064x600p_60Hz,      true,  false, false, true,  true,  false, false, false, false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eDVI_1440x900p_60Hz,      true,  false, false, true,  true,  false, false, true,  false, 0, 9, 0)

    /* SW7435-276: New format enums for 482/483 */
    BVDC_P_MAKE_FMTINFO(e720x482_NTSC,            false, false, true,  false, false, true,  true,  true,  true , 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720x482_NTSC_J,          false, false, true,  false, false, false, true,  true,  true , 3, 6, 3)
    BVDC_P_MAKE_FMTINFO(e720x483p,                false, true,  false, false, true,  true,  true,  true,  true , 0, 6, 3)

    /* custom formats */
    BVDC_P_MAKE_FMTINFO(eCustom0,                 true,  false, false, false, true,  false, false, true,  false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eCustom1,                 true,  false, false, false, true,  false, false, true,  false, 0, 0, 0)
    BVDC_P_MAKE_FMTINFO(eCustom2,                 true,  false, false, false, true,  false, false, true,  false, 0, 0, 0)

    /* Must be last */
    BVDC_P_MAKE_FMTINFO(eMaxCount,                false, false, false, false, false, false, false, false, false, 0, 4, 5)
};

static const BVDC_P_FormatData s_aFormatDataTable[] =
{
    /* Format                                        Analog Microcode                          Digital Microcode                     Digital Microcode DropTbl           ItTable                     ItConfig           */
    BVDC_P_MAKE_FMTDATA(eNTSC,                       s_aulAnalogMicrocode_NTSC_J,                    NULL,                   s_aulDtRamBVBInput_DVI_480i_DropTbl, s_aulItTable_480i,            s_aulItConfig_480i)
    BVDC_P_MAKE_FMTDATA(eNTSC_J,                     s_aulAnalogMicrocode_NTSC_J,                      NULL,                   s_aulDtRamBVBInput_DVI_480i_DropTbl, s_aulItTable_NTSC_J,        s_aulItConfig_NTSC_J)
    BVDC_P_MAKE_FMTDATA(eNTSC_443,                   s_aulAnalogMicrocode_NTSC_J,                    NULL,                   s_aulDtRamBVBInput_DVI_480i_DropTbl, s_aulItTable_480i,            s_aulItConfig_480i)
    BVDC_P_MAKE_FMTDATA(ePAL_B,                      s_aulAnalogMicrocode_PAL_B,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_576i,            s_aulItConfig_576i)
    BVDC_P_MAKE_FMTDATA(ePAL_B1,                     s_aulAnalogMicrocode_PAL_B1,              s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_576i,            s_aulItConfig_576i)
    BVDC_P_MAKE_FMTDATA(ePAL_D,                      s_aulAnalogMicrocode_PAL_D,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_576i,            s_aulItConfig_576i)
    BVDC_P_MAKE_FMTDATA(ePAL_D1,                     s_aulAnalogMicrocode_PAL_D1,              s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_576i,            s_aulItConfig_576i)
    BVDC_P_MAKE_FMTDATA(ePAL_G,                      s_aulAnalogMicrocode_PAL_G,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_576i,            s_aulItConfig_576i)
    BVDC_P_MAKE_FMTDATA(ePAL_H,                      s_aulAnalogMicrocode_PAL_H,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_576i,            s_aulItConfig_576i)
    BVDC_P_MAKE_FMTDATA(ePAL_K,                      s_aulAnalogMicrocode_PAL_K,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_576i,            s_aulItConfig_576i)
    BVDC_P_MAKE_FMTDATA(ePAL_I,                      s_aulAnalogMicrocode_PAL_I,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_576i,            s_aulItConfig_576i)
    BVDC_P_MAKE_FMTDATA(ePAL_M,                      s_aulAnalogMicrocode_PAL_M,                s_aulDviMicrocode_PAL_M,        NULL,                               s_aulItTable_PAL_M,          s_aulItConfig_PAL_M)
    BVDC_P_MAKE_FMTDATA(ePAL_N,                      s_aulAnalogMicrocode_PAL_N,               s_aulDtRamBVBInput_DVI_576i_OLD, NULL,                               s_aulItTable_480i,           s_aulItConfig_576i)
    BVDC_P_MAKE_FMTDATA(ePAL_NC,                     s_aulAnalogMicrocode_PAL_NC,              s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_576i,            s_aulItConfig_576i)
    BVDC_P_MAKE_FMTDATA(ePAL_60,                     s_aulAnalogMicrocode_NTSC_J,                    NULL,                    s_aulDtRamBVBInput_DVI_480i_DropTbl,s_aulItTable_480i,            s_aulItConfig_480i)
    BVDC_P_MAKE_FMTDATA(eSECAM_L,                    s_aulAnalogMicrocode_SECAM,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_SECAM,      s_aulItConfig_SECAM)
    BVDC_P_MAKE_FMTDATA(eSECAM_B,                    s_aulAnalogMicrocode_SECAM,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_SECAM,      s_aulItConfig_SECAM)
    BVDC_P_MAKE_FMTDATA(eSECAM_G,                    s_aulAnalogMicrocode_SECAM,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_SECAM,      s_aulItConfig_SECAM)
    BVDC_P_MAKE_FMTDATA(eSECAM_D,                    s_aulAnalogMicrocode_SECAM,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_SECAM,      s_aulItConfig_SECAM)
    BVDC_P_MAKE_FMTDATA(eSECAM_K,                    s_aulAnalogMicrocode_SECAM,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_SECAM,      s_aulItConfig_SECAM)
    BVDC_P_MAKE_FMTDATA(eSECAM_H,                    s_aulAnalogMicrocode_SECAM,               s_aulDviMicrocode_576i,          NULL,                               s_aulItTable_SECAM,      s_aulItConfig_SECAM)
    BVDC_P_MAKE_FMTDATA(e1080i,                      s_aulAnalogMicrocode_1080i,               s_aulDviMicrocode_1080i,         NULL,                               s_aulItTable_1080i,            s_aulItConfig_1080i)
    BVDC_P_MAKE_FMTDATA(e1080p,                      s_aulAnalogMicrocode_1080p_60hz,          s_aulDviMicrocode_1080p_60hz,    NULL,                               s_aulItTable_1080p_60hz,       s_aulItConfig_1080p_60hz)
    BVDC_P_MAKE_FMTDATA(e720p,                       s_aulAnalogMicrocode_720p,                s_aulDviMicrocode_720p,          NULL,                               s_aulItTable_720p,             s_aulItConfig_720p)
    BVDC_P_MAKE_FMTDATA(e720p_60Hz_3DOU_AS,          s_aulAnalogMicrocode_720p_60hz3D,         s_aulDviMicrocode_720p_60hz3D,   NULL,                               s_aulItTable_3D,               &s_ulItConfig_3D)
    BVDC_P_MAKE_FMTDATA(e720p_50Hz_3DOU_AS,          s_aulAnalogMicrocode_720p_50hz3D,         s_aulDviMicrocode_720p_50hz3D,   NULL,                               s_aulItTable_3D,               &s_ulItConfig_3D)
    BVDC_P_MAKE_FMTDATA(e720p_30Hz_3DOU_AS,          s_aulAnalogMicrocode_720p,                s_aulDviMicrocode_720p_60hz3D,   NULL,                               s_aulItTable_720p_30hz,        s_aulItConfig_720p_30hz3D)
    BVDC_P_MAKE_FMTDATA(e720p_24Hz_3DOU_AS,          s_aulAnalogMicrocode_720p,                s_aulDviMicrocode_720p_60hz3D,   NULL,                               s_aulItTable_720p_24hz,        s_aulItConfig_720p_24hz3D)
    BVDC_P_MAKE_FMTDATA(e480p,                       s_aulAnalogMicrocode_480p,                NULL,                            s_aulDtRamBVBInput_DVI_480p_DropTbl, s_aulItTable_480p54,             s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(e1080i_50Hz,                 s_aulAnalogMicrocode_1080i_50hz,          s_aulDviMicrocode_1080i_50hz,    NULL,                               s_aulItTable_1080i_50hz,       s_aulItConfig_1080i_50hz)
    BVDC_P_MAKE_FMTDATA(e1080p_24Hz_3DOU_AS,         s_aulAnalogMicrocode_1080p_24hz3D,        s_aulDviMicrocode_1080p_24hz3D,  NULL,                               s_aulItTable_3D,               &s_ulItConfig_3D)
    BVDC_P_MAKE_FMTDATA(e1080p_30Hz_3DOU_AS,         s_aulAnalogMicrocode_1080p_30hz3D,        s_aulDviMicrocode_1080p_30hz3D,  NULL,                               s_aulItTable_3D,               &s_ulItConfig_3D)
    BVDC_P_MAKE_FMTDATA(e1080p_60Hz_3DOU_AS,         s_aulRamBVBInput_1080p_60hz_3DOU,         s_aulDviMicrocode_1080p_30hz3D,  NULL,                               s_aulItTable_3D,               &s_ulItConfig_3D)
    BVDC_P_MAKE_FMTDATA(e1080p_60Hz_3DLR,            s_aulRamBVBInput_1080p_60hz_3DLR,         s_aulDtRamBVBInput_DVI_3DLR_1080p_60hz, NULL,                        s_aulItTable_3D,               &s_ulItConfig_3D)
    BVDC_P_MAKE_FMTDATA(e1080p_24Hz,                 s_aulAnalogMicrocode_1080p_24hz,          s_aulDviMicrocode_1080p_24hz,    NULL,                               s_aulItTable_1080p_24hz,       s_aulItConfig_1080p_24hz)
    BVDC_P_MAKE_FMTDATA(e1080p_25Hz,                 s_aulAnalogMicrocode_1080p_25hz,          s_aulDviMicrocode_1080p_25hz,    NULL,                               s_aulItTable_1080p_25hz,       s_aulItConfig_1080p_25hz)
    BVDC_P_MAKE_FMTDATA(e1080p_30Hz,                 s_aulAnalogMicrocode_1080p_30hz,          s_aulDviMicrocode_1080p_30hz,    NULL,                               s_aulItTable_1080p_30hz,       s_aulItConfig_1080p_30hz)
    BVDC_P_MAKE_FMTDATA(e1080p_50Hz,                 s_aulAnalogMicrocode_1080p_50hz,          s_aulDviMicrocode_1080p_50hz,    NULL,                               s_aulItTable_1080p_30hz,       s_aulItConfig_1080p_50hz)
    BVDC_P_MAKE_FMTDATA(e1080p_100Hz,                s_aulAnalogMicrocode_1080p_50hz,          s_aulDviMicrocode_1080p_50hz,    NULL,                               s_aulItTable_1080p_30hz,       s_aulItConfig_1080p_50hz)
    BVDC_P_MAKE_FMTDATA(e1080p_120Hz,                s_aulAnalogMicrocode_1080p_60hz,          s_aulDviMicrocode_1080p_60hz,    NULL,                               s_aulItTable_1080p_30hz,       s_aulItConfig_1080p_50hz)
    BVDC_P_MAKE_FMTDATA(e1250i_50Hz,                 s_aulAnalogMicrocode_1250i_50hz,          NULL,                            NULL,                               s_aulItTable_1080i_50hz,       s_aulItConfig_1250i_50hz)
    BVDC_P_MAKE_FMTDATA(e720p_24Hz,                  s_aulAnalogMicrocode_720p_24hz,           s_aulDviMicrocode_720p_24hz,     NULL,                               s_aulItTable_720p_24hz,        s_aulItConfig_720p_24hz)
    BVDC_P_MAKE_FMTDATA(e720p_25Hz,                  s_aulAnalogMicrocode_720p_25hz,           s_aulDviMicrocode_720p_25hz,     NULL,                               s_aulItTable_720p_25hz,        s_aulItConfig_720p_25hz)
    BVDC_P_MAKE_FMTDATA(e720p_30Hz,                  s_aulAnalogMicrocode_720p_30hz,           s_aulDviMicrocode_720p_30hz,     NULL,                               s_aulItTable_720p_30hz,        s_aulItConfig_720p_30hz)
    BVDC_P_MAKE_FMTDATA(e720p_50Hz,                  s_aulAnalogMicrocode_720p_50hz,           s_aulDviMicrocode_720p_50hz,     NULL,                               s_aulItTable_720p_50hz,        s_aulItConfig_720p_50hz)
    BVDC_P_MAKE_FMTDATA(e576p_50Hz,                  s_aulAnalogMicrocode_576p,                s_aulDviMicrocode_576p,          NULL,                               s_aulItTable_576p54,           s_aulItConfig_576p)
    BVDC_P_MAKE_FMTDATA(e240p_60Hz,                  NULL,                                     NULL,                            NULL,                               NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(e288p_50Hz,                  NULL,                                     NULL,                            NULL,                               NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(e1440x480p_60Hz,             NULL,                                     NULL,                            NULL,                               NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(e1440x576p_50Hz,             NULL,                                     NULL,                            NULL,                               NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(e3840x2160p_24Hz,            s_aulRamBVBInput_3840x2160p_24Hz,         s_aulDtRamBVBInput_3840x2160p_24Hz, NULL,                            s_aulItTable_1080p_24hz,       s_aulItConfig_1080p_24hz)
    BVDC_P_MAKE_FMTDATA(e3840x2160p_25Hz,            NULL,                                     s_aulDtRamBVBInput_3840x2160p_25Hz, NULL,                            s_aulItTable_1080p_25hz,       s_aulItConfig_1080p_25hz)
    BVDC_P_MAKE_FMTDATA(e3840x2160p_30Hz,            NULL,                                     s_aulDtRamBVBInput_3840x2160p_30Hz, NULL,                            s_aulItTable_1080p_30hz,       s_aulItConfig_1080p_30hz)
    BVDC_P_MAKE_FMTDATA(e3840x2160p_50Hz,            NULL,                                     s_aulDtRamBVBInput_3840x2160p_25Hz, NULL,                            s_aulItTable_1080p_25hz,       s_aulItConfig_1080p_25hz)
    BVDC_P_MAKE_FMTDATA(e3840x2160p_60Hz,            NULL,                                     s_aulDtRamBVBInput_3840x2160p_30Hz, NULL,                            s_aulItTable_1080p_30hz,       s_aulItConfig_1080p_30hz)
    BVDC_P_MAKE_FMTDATA(e4096x2160p_24Hz,            NULL,                                     s_aulDtRamBVBInput_4096x2160p_24Hz, NULL,                            s_aulItTable_1080p_24hz,       s_aulItConfig_1080p_24hz)
    BVDC_P_MAKE_FMTDATA(e4096x2160p_25Hz,            NULL,                                     s_aulDtRamBVBInput_4096x2160p_25Hz, NULL,                            s_aulItTable_1080p_24hz,       s_aulItConfig_1080p_24hz)
    BVDC_P_MAKE_FMTDATA(e4096x2160p_30Hz,            NULL,                                     s_aulDtRamBVBInput_4096x2160p_30Hz, NULL,                            s_aulItTable_1080p_24hz,       s_aulItConfig_1080p_24hz)
    BVDC_P_MAKE_FMTDATA(e4096x2160p_50Hz,            NULL,                                     s_aulDtRamBVBInput_4096x2160p_25Hz, NULL,                            s_aulItTable_1080p_24hz,       s_aulItConfig_1080p_24hz)
    BVDC_P_MAKE_FMTDATA(e4096x2160p_60Hz,            NULL,                                     s_aulDtRamBVBInput_4096x2160p_30Hz, NULL,                            s_aulItTable_1080p_24hz,       s_aulItConfig_1080p_24hz)
#if BFMT_LEGACY_3DTV_SUPPORT
    BVDC_P_MAKE_FMTDATA(eCUSTOM1920x2160i_48Hz,      NULL,                                     s_aulDtRamBVBInput_DVI_2160i_48hz,  NULL,                            s_aulItTable_1080p_60hz,       s_aulItConfig_1080p_60hz)
    BVDC_P_MAKE_FMTDATA(eCUSTOM1920x2160i_60Hz,      NULL,                                     s_aulDtRamBVBInput_DVI_2160i_60hz,  NULL,                            s_aulItTable_1080p_60hz,       s_aulItConfig_1080p_60hz)
#endif
    BVDC_P_MAKE_FMTDATA(eCUSTOM_1440x240p_60Hz,      NULL,                                     NULL,                            NULL,                               NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eCUSTOM_1440x288p_50Hz,      NULL,                                     NULL,                            NULL,                               NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eCUSTOM_1366x768p,           s_aulRamBVBInput_CUSTOM_1366x768p,        s_aulDtRamBVBInput_CUSTOM_1366x768p, NULL,                           s_aulItTable_CUSTOM_1366x768p, &s_ulItConfig_CUSTOM_1366x768p)
    BVDC_P_MAKE_FMTDATA(eCUSTOM_1366x768p_50Hz,      s_aulRamBVBInput_CUSTOM_1366x768p,        s_aulDtRamBVBInput_CUSTOM_1366x768p, NULL,                           s_aulItTable_CUSTOM_1366x768p, &s_ulItConfig_CUSTOM_1366x768p)
    BVDC_P_MAKE_FMTDATA(eDVI_640x480p,               s_aulRamBVBInput_DVI_640x480p,            s_aulDtRamBVBInput_DVI_640x480p,  NULL,                               s_aulItTable_480p,             s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_640x480p_CVT,           s_aulRamBVBInput_DVI_640x480p_CVT,        NULL,                             NULL,                               s_aulItTable_VESA,             s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_800x600p,               s_aulRamBVBInput_DVI_800x600p,            s_aulDtRamBVBInput_DVI_800x600p,  NULL,                               s_aulItTable_480p,             s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_1024x768p,              s_aulRamBVBInput_DVI_1024x768p,           s_aulDtRamBVBInput_DVI_1024x768p, NULL,                               s_aulItTable_480p,             s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x768p,              s_aulRamBVBInput_DVI_1280x768p,           s_aulDtRamBVBInput_DVI_1280x768p, NULL,                               s_aulItTable_1280x720p,        s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x768p_Red,          NULL,                                     NULL,                             NULL,                               NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x720p_50Hz,         s_aulRamBVBInput_DVI_1280x720p_50Hz,      s_aulDtRamBVBInput_DVI_1280x720p_50Hz,NULL,                           s_aulItTable_1280x720p,        s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x720p,              s_aulRamBVBInput_DVI_1280x720p,           s_aulDtRamBVBInput_DVI_1280x720p,  NULL,                              s_aulItTable_1280x720p,        s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x720p_Red,          s_aulRamBVBInput_DVI_1280x720p_Red,       s_aulDtRamBVBInput_DVI_1280x720p_Red, NULL,                           s_aulItTable_1280x720p,        s_aulItConfig_480p)

    /* PC input format! */
    BVDC_P_MAKE_FMTDATA(eDVI_640x350p_60Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x350p_70Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x350p_72Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x350p_75Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x350p_85Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)

    BVDC_P_MAKE_FMTDATA(eDVI_640x400p_60Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x400p_70Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x400p_72Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x400p_75Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x400p_85Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)

    BVDC_P_MAKE_FMTDATA(eDVI_640x480p_66Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x480p_70Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x480p_72Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x480p_75Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_640x480p_85Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)

    BVDC_P_MAKE_FMTDATA(eDVI_720x400p_60Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_720x400p_70Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_720x400p_72Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_720x400p_75Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_720x400p_85Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)

    BVDC_P_MAKE_FMTDATA(eDVI_800x600p_56Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_800x600p_59Hz_Red,      NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_800x600p_70Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_800x600p_72Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_800x600p_75Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_800x600p_85Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)

    BVDC_P_MAKE_FMTDATA(eDVI_1024x768p_66Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1024x768p_70Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1024x768p_72Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1024x768p_75Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1024x768p_85Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)

    BVDC_P_MAKE_FMTDATA(eDVI_1280x720p_70Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x720p_72Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x720p_75Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x720p_85Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)

    /* New DVI or PC vdec input support */
    BVDC_P_MAKE_FMTDATA(eDVI_1024x768i_87Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1152x864p_75Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x768p_75Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x768p_85Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x800p_60Hz,         s_aulRamBVBInput_DVI_1280x800p_60Hz,      s_aulDtRamBVBInput_DVI_1280x800p,     NULL,                            s_aulItTable_VESA,             s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x960p_60Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x960p_85Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x1024p_60Hz,        s_aulRamBVBInput_DVI_1280x1024p_60Hz,     s_aulDtRamBVBInput_DVI_1280x1024p,    NULL,                            s_aulItTable_VESA,             s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x1024p_69Hz,        NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x1024p_75Hz,        NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1280x1024p_85Hz,        NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_832x624p_75Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1360x768p_60Hz,         s_aulRamBVBInput_DVI_1360x768p_60Hz,      s_aulDtRamBVBInput_DVI_1360x768p,     NULL,                            s_aulItTable_VESA,             s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_1366x768p_60Hz,         s_aulRamBVBInput_DVI_1366x768p_60Hz,      NULL,                                 NULL,                            s_aulItTable_VESA,             s_aulItConfig_480p)
    BVDC_P_MAKE_FMTDATA(eDVI_1400x1050p_60Hz,        NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1400x1050p_60Hz_Red,    NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1400x1050p_75Hz,        NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1600x1200p_60Hz,        s_aulRamBVBInput_1600x1200p_60Hz,         s_aulDtRamBVBInput_1600x1200p_60Hz,   NULL,                            s_aulItTable_1600x1200p_60Hz,  &s_ulItConfig_CUSTOM_1366x768p)
    BVDC_P_MAKE_FMTDATA(eDVI_1920x1080p_60Hz_Red,    NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_848x480p_60Hz,          NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1064x600p_60Hz,         NULL,                                     NULL,                                 NULL,                            NULL,                          NULL)
    BVDC_P_MAKE_FMTDATA(eDVI_1440x900p_60Hz,         s_aulRamBVBInput_DVI_1440x900p_60Hz,      s_aulDtRamBVBInput_DVI_1440x900p,     NULL,                            s_aulItTable_VESA,             s_aulItConfig_480p)

    /* SW7435-276: New format enums for 482/483 */
    BVDC_P_MAKE_FMTDATA(e720x482_NTSC,               s_aulAnalogMicrocode_NTSC_J,            NULL,                                 s_aulDtRamBVBInput_DVI_480i_DropTbl,s_aulItTable_480i,            s_aulItConfig_480i)
    BVDC_P_MAKE_FMTDATA(e720x482_NTSC_J,             s_aulAnalogMicrocode_NTSC_J,            NULL,                                 s_aulDtRamBVBInput_DVI_480i_DropTbl,s_aulItTable_480i,            s_aulItConfig_480i)
    BVDC_P_MAKE_FMTDATA(e720x483p,                   s_aulAnalogMicrocode_480p,                NULL,                                 s_aulDtRamBVBInput_DVI_480p_DropTbl,s_aulItTable_480p,             s_aulItConfig_480p)

    /* custom formats */
    BVDC_P_MAKE_FMTDATA(eCustom0,                    s_aulRamBVBInput_CUSTOM_1366x768p,        s_aulDtRamBVBInput_CUSTOM_1366x768p,  NULL,                            s_aulItTable_CUSTOM_1366x768p, &s_ulItConfig_CUSTOM_1366x768p)
    BVDC_P_MAKE_FMTDATA(eCustom1,                    s_aulRamBVBInput_CUSTOM_1366x768p,        s_aulDtRamBVBInput_CUSTOM_1366x768p,  NULL,                            s_aulItTable_CUSTOM_1366x768p, &s_ulItConfig_CUSTOM_1366x768p)
    BVDC_P_MAKE_FMTDATA(eCustom2,                    s_aulRamBVBInput_CUSTOM_1366x768p,        s_aulDtRamBVBInput_CUSTOM_1366x768p,  NULL,                            s_aulItTable_CUSTOM_1366x768p, &s_ulItConfig_CUSTOM_1366x768p)

    /* Must be last */
    BVDC_P_MAKE_FMTDATA(eMaxCount,                   NULL,                                     NULL,                                 NULL,                            NULL,                       NULL)
};

/****************************************************************
 *  When adding additional formats, make sure each format is
 *  properly configured for all colorspaces it supports in the
 *  tables below.  Please also ensure that SD and HD specific
 *  configuration tables also configure them properly.
 *
 *  Each table below has an entry with BFMT_VideoFmt_eMaxCount
 *  as the format.  This entry specifies the default
 *  configuration for formats that aren't treated as a special
 *  case. Special case formats with non-default configurations
 *  are specified by adding an entry for them above the
 *  BFMT_VideoFmt_eMaxCount entry.
 *
 *  The BFMT_VideoFmt_eMaxCount format entry should always be the
 *  last in a table, and is used by search loops to determine
 *  when to terminate.
 ****************************************************************/

#if BVDC_P_NUM_SHARED_SM
/****************************************************************
 *  Sm Tables
 ****************************************************************/
/* YQI, YQI-M SmTables */
static const BVDC_P_SmTableInfo s_aulSmTable_Tbl_YQI[] =
{
    {BFMT_VideoFmt_eMaxCount,           s_aulSmTable_YQI}

};

/* YUV, YUV-N SmTables */
static const BVDC_P_SmTableInfo s_aulSmTable_Tbl_YUV[] =
{
    BVDC_P_MAKE_MISC(ePAL_I,              s_aulSmTable_PALI_YUV)
    BVDC_P_MAKE_MISC(ePAL_M,              s_aulSmTable_PALM_YUV)
    BVDC_P_MAKE_MISC(ePAL_N,              s_aulSmTable_PALN_YUV)
    BVDC_P_MAKE_MISC(ePAL_NC,             s_aulSmTable_PALNC_YUV)
    BVDC_P_MAKE_MISC(eSECAM_L,            s_aulSmTable_SECAM)
    BVDC_P_MAKE_MISC(eSECAM_B,            s_aulSmTable_SECAM)
    BVDC_P_MAKE_MISC(eSECAM_G,            s_aulSmTable_SECAM)
    BVDC_P_MAKE_MISC(eSECAM_D,            s_aulSmTable_SECAM)
    BVDC_P_MAKE_MISC(eSECAM_K,            s_aulSmTable_SECAM)
    BVDC_P_MAKE_MISC(eSECAM_H,            s_aulSmTable_SECAM)
    {BFMT_VideoFmt_eMaxCount,             s_aulSmTable_PAL_YUV}
};

/* RGB SmTables */
static const BVDC_P_SmTableInfo s_aulSmTable_Tbl_RGB[] =
{
    {BFMT_VideoFmt_eMaxCount,           s_aulSmTable_Component}
};

/* SDYPrPb SmTables */
static const BVDC_P_SmTableInfo s_aulSmTable_Tbl_SDYPrPb[] =
{
    {BFMT_VideoFmt_eMaxCount,           s_aulSmTable_Component}
};

/* HDYPrPb SmTables */
static const BVDC_P_SmTableInfo s_aulSmTable_Tbl_HDYPrPb[] =
{
    {BFMT_VideoFmt_eMaxCount,           s_aulSmTable_Component}
};

/* Hsync SmTables */
static const BVDC_P_SmTableInfo s_aulSmTable_Tbl_Hsync[] =
{
    {BFMT_VideoFmt_eMaxCount,           s_aulSmTable_Component}
};

/* Unknown SmTables */
static const BVDC_P_SmTableInfo s_aulSmTable_Tbl_Unknown[] =
{
    {BFMT_VideoFmt_eMaxCount,           s_aulSmTable_Component}
};
#endif

#if BVDC_P_NUM_SHARED_VF
/****************************************************************
 *  Channel Filter Tables
 ****************************************************************/
static const BVDC_P_FilterTableInfo s_aulFilterTable_Tbl[] =
{
    /* OutputFilter                ChFilter_Ch0               ChFilter_Ch1                 ChFilter_Ch2 */
    {BVDC_P_OutputFilter_eHDYPrPb, s_aulChFilterTbl_AllPass,  s_aulChFilterTbl_Cr_675Mhz,  s_aulChFilterTbl_Cr_675Mhz},
    {BVDC_P_OutputFilter_eHDRGB,   s_aulChFilterTbl_AllPass,  s_aulChFilterTbl_AllPass,    s_aulChFilterTbl_AllPass},
    {BVDC_P_OutputFilter_eED,      s_aulChFilterTbl_AllPass,  s_aulChFilterTbl_AllPass,    s_aulChFilterTbl_AllPass},
    {BVDC_P_OutputFilter_eSDYPrPb, s_aulChFilterTbl_Y_60Mhz,  s_aulChFilterTbl_Cr_SD,      s_aulChFilterTbl_Cr_SD},
    {BVDC_P_OutputFilter_eSDRGB,   s_aulChFilterTbl_Y_60Mhz,  s_aulChFilterTbl_Y_60Mhz,    s_aulChFilterTbl_Y_60Mhz},
    {BVDC_P_OutputFilter_eYQI,     s_aulFilterTable_NTSC_0,   s_aulFilterTable_NTSC_1,     s_aulFilterTable_NTSC_2},
    {BVDC_P_OutputFilter_eYUV,     s_aulChFilterTbl_Y_60Mhz,  s_aulChFilterTbl_Cr_13Mhz,   s_aulChFilterTbl_Cr_13Mhz},
    {BVDC_P_OutputFilter_eSECAM,   s_aulChFilterTbl_Y_12Mhz,  s_aulChFilterTbl_Cr_SECAM,   s_aulChFilterTbl_Cr_SECAM},
    {BVDC_P_OutputFilter_eHsync,   s_aulChFilterTbl_Y_675Mhz, s_aulChFilterTbl_Y_675Mhz,   s_aulChFilterTbl_Y_675Mhz},
    {BVDC_P_OutputFilter_eUnknown, s_aulChFilterTbl_Y_60Mhz,  s_aulChFilterTbl_Cr_SD,      s_aulChFilterTbl_Cr_SD},
};

/****************************************************************
 *  Vf Tables
 ****************************************************************/

/* Hd HDYPrPb VfTables */
static const BVDC_P_VfTableInfo s_aulHdVfTable_Tbl_HDYPrPb[] =
{
    BVDC_P_MAKE_MISC(e480p,                  s_aulVfTable_480p)
    BVDC_P_MAKE_MISC(e720x483p,              s_aulVfTable_480p)
    BVDC_P_MAKE_MISC(e576p_50Hz,             s_aulVfTable_576p)
    BVDC_P_MAKE_MISC(eDVI_640x480p,          s_aulVfTable_VESA_RGB)
    BVDC_P_MAKE_MISC(eDVI_640x480p_CVT,      s_aulVfTable_VESA_RGB)
    BVDC_P_MAKE_MISC(eDVI_800x600p,          s_aulVfTable_VESA_RGB)
    BVDC_P_MAKE_MISC(eDVI_1024x768p,         s_aulVfTable_VESA_RGB)
    BVDC_P_MAKE_MISC(eDVI_1280x720p,         s_aulVfTable_VESA_RGB)
    BVDC_P_MAKE_MISC(eDVI_1280x800p_60Hz,    s_aulVfTable_VESA_RGB)
    BVDC_P_MAKE_MISC(eDVI_1280x1024p_60Hz,   s_aulVfTable_VESA_RGB)
    BVDC_P_MAKE_MISC(eDVI_1360x768p_60Hz,    s_aulVfTable_VESA_RGB)
    BVDC_P_MAKE_MISC(eDVI_1366x768p_60Hz,    s_aulVfTable_VESA_RGB)
    BVDC_P_MAKE_MISC(eDVI_1440x900p_60Hz,    s_aulVfTable_VESA_RGB)
    BVDC_P_MAKE_MISC(eCUSTOM_1366x768p,      s_aulVfTable_1080i)
    BVDC_P_MAKE_MISC(eCUSTOM_1366x768p_50Hz, s_aulVfTable_1080i)
    {BFMT_VideoFmt_eMaxCount,                s_aulVfTable_1080i}
};

/* Same, but for double rate pixels. */
static const BVDC_P_VfTableInfo s_aulHdVfTable_Tbl_54MHz_HDYPrPb[] =
{
    BVDC_P_MAKE_MISC(e480p,                  s_aulVfTable_480p54)
    BVDC_P_MAKE_MISC(e720x483p,              s_aulVfTable_480p54)
    BVDC_P_MAKE_MISC(e576p_50Hz,             s_aulVfTable_576p54)
    {BFMT_VideoFmt_eMaxCount,                s_aulVfTable_1080i}
};

/* Unknown VfTables */
static const BVDC_P_VfTableInfo s_aulHdVfTable_Tbl_Unknown[] =
{
    BVDC_P_MAKE_MISC(e480p,                  s_aulVfTable_480p)
    BVDC_P_MAKE_MISC(e720x483p,              s_aulVfTable_480p)
    BVDC_P_MAKE_MISC(e576p_50Hz,             s_aulVfTable_576p)
    BVDC_P_MAKE_MISC(eCUSTOM_1366x768p,      s_aulVfTable_1080i)
    BVDC_P_MAKE_MISC(eCUSTOM_1366x768p_50Hz, s_aulVfTable_1080i)
    {BFMT_VideoFmt_eMaxCount,                s_aulVfTable_1080i}
};

/* Same, for 480P / 54 MHz pixel rate */
static const BVDC_P_VfTableInfo s_aulHdVfTable_Tbl_54MHz_Unknown[] =
{
    BVDC_P_MAKE_MISC(e480p,                  s_aulVfTable_480p54)
    BVDC_P_MAKE_MISC(e720x483p,              s_aulVfTable_480p54)
    BVDC_P_MAKE_MISC(e576p_50Hz,             s_aulVfTable_576p54)
    {BFMT_VideoFmt_eMaxCount,                s_aulVfTable_1080i}
};

/* Sd YQI VfTables */
static const BVDC_P_VfTableInfo s_aulSdVfTable_Tbl_YQI[] =
{
    BVDC_P_MAKE_MISC(eNTSC_J,   s_aulVfTable_NTSC_J)
    {BFMT_VideoFmt_eMaxCount,   s_aulVfTable_NTSC_ITU}
};

/* Sd YQI_M VfTables */
static const BVDC_P_VfTableInfo s_aulSdVfTable_Tbl_YQI_M[] =
{
    {BFMT_VideoFmt_eMaxCount, s_aulVfTable_NTSC_ITU}
};

/* Sd YUV VfTables */
static const BVDC_P_VfTableInfo s_aulSdVfTable_Tbl_YUV[] =
{
    BVDC_P_MAKE_MISC(ePAL_I, s_aulVfTable_PAL_I)
    BVDC_P_MAKE_MISC(ePAL_N, s_aulVfTable_PAL_N)
    BVDC_P_MAKE_MISC(ePAL_NC, s_aulVfTable_PAL_NC)
    BVDC_P_MAKE_MISC(ePAL_M, s_aulVfTable_PAL_M)
    {BFMT_VideoFmt_eMaxCount,   s_aulVfTable_PAL}
};

/* VFTable for SECAM */
static const BVDC_P_VfTableInfo s_aulSdVfTable_Tbl_YDbDr[] =
{
    {BFMT_VideoFmt_eMaxCount, s_aulVfTable_SECAM}
};

/* Sd YPrPb VfTables */
static const BVDC_P_VfTableInfo s_aulSdVfTable_Tbl_SDYPrPb[] =
{
    BVDC_P_MAKE_MISC(eNTSC,           s_aulVfTable_480i)
    BVDC_P_MAKE_MISC(eNTSC_J,         s_aulVfTable_480i)
    BVDC_P_MAKE_MISC(e720x482_NTSC,   s_aulVfTable_480i)
    BVDC_P_MAKE_MISC(e720x482_NTSC_J, s_aulVfTable_480i)
    BVDC_P_MAKE_MISC(eNTSC_443,       s_aulVfTable_480i)
    BVDC_P_MAKE_MISC(ePAL_60,         s_aulVfTable_480i)
    BVDC_P_MAKE_MISC(ePAL_M,          s_aulVfTable_480i)
    {BFMT_VideoFmt_eMaxCount,         s_aulVfTable_576i}
};

/* Sd Unknown VfTables */
static const BVDC_P_VfTableInfo s_aulSdVfTable_Tbl_Unknown[] =
{
    {BFMT_VideoFmt_eMaxCount, s_aulVfTable_480i}
};

#define BVDC_P_MAKE_ENVELOPGENERATOR(amp_cntl, amp_clamp, u_burst, v_burst,    \
    u_en, v_en, init_amp)                                                      \
    BVDC_P_VF_FIELD_ENUM(ENVELOPE_GENERATOR, AMP_CONTROL,       amp_cntl) | \
    BVDC_P_VF_FIELD_DATA(ENVELOPE_GENERATOR, AMP_CLAMP,        amp_clamp) | \
    BVDC_P_VF_FIELD_ENUM(ENVELOPE_GENERATOR, U_BURST_AMP_SEL,    u_burst) | \
    BVDC_P_VF_FIELD_ENUM(ENVELOPE_GENERATOR, V_BURST_AMP_SEL,    v_burst) | \
    BVDC_P_VF_FIELD_ENUM(ENVELOPE_GENERATOR, U_ENABLE,              u_en) | \
    BVDC_P_VF_FIELD_ENUM(ENVELOPE_GENERATOR, V_ENABLE,              v_en) | \
    BVDC_P_VF_FIELD_DATA(ENVELOPE_GENERATOR, INIT_AMP,          init_amp)

#define BVDC_EG_YQI_SETTING       BVDC_P_MAKE_ENVELOPGENERATOR(CONSTANT,     0, NEGATIVE, NEGATIVE,  ON, OFF, 0x70)
#define BVDC_EG_PAL_YUV_SETTING   BVDC_P_MAKE_ENVELOPGENERATOR(CONSTANT,     0, NEGATIVE, POSITIVE,  ON,  ON, 0x54)
#define BVDC_EG_PALNC_YUV_SETTING BVDC_P_MAKE_ENVELOPGENERATOR(CONSTANT,     0, NEGATIVE, POSITIVE,  ON,  ON, 0x54)
#define BVDC_EG_PALM_YUV_SETTING  BVDC_P_MAKE_ENVELOPGENERATOR(CONSTANT,     0, NEGATIVE, POSITIVE,  ON,  ON, 0x58)
#define BVDC_EG_SECAM_SETTING     BVDC_P_MAKE_ENVELOPGENERATOR(INC_BY_ONE, 424, NEGATIVE, NEGATIVE,  ON, OFF, 0x78)
#define BVDC_EG_COMPONET_SETTING  BVDC_P_MAKE_ENVELOPGENERATOR(CONSTANT,     0, NEGATIVE, NEGATIVE, OFF, OFF,    0)

/****************************************************************
 *  When adding additional formats, make sure each format is
 *  properly configured for all colorspaces it supports in the
 *  tables below.
 *
 *  Each table below has an entry with BFMT_VideoFmt_eMaxCount
 *  as the format.  This entry specifies the default
 *  configuration for formats that aren't treated as a special
 *  case. Special case formats with non-default configurations
 *  are specified by adding an entry for them above the
 *  BFMT_VideoFmt_eMaxCount entry.
 *
 *  The BFMT_VideoFmt_eMaxCount format entry should always be the
 *  last in a table, and is used by search loops to determine
 *  when to terminate.
 ****************************************************************/

/****************************************************************
 *  Envelop Generator Control Setting Tables
 ****************************************************************/
static const BVDC_P_EnvelopGeneratorSetting s_aulEG_Tbl_YQI[] =
{
    {BFMT_VideoFmt_eMaxCount,           BVDC_EG_YQI_SETTING}
};

static const BVDC_P_EnvelopGeneratorSetting s_aulEG_Tbl_YQI_M[] =
{
    {BFMT_VideoFmt_eMaxCount,           BVDC_EG_YQI_SETTING}
};

static const BVDC_P_EnvelopGeneratorSetting s_aulEG_Tbl_YUV[] =
{
    BVDC_P_MAKE_MISC(ePAL_NC,             BVDC_EG_PALNC_YUV_SETTING)
    BVDC_P_MAKE_MISC(eSECAM_L,            BVDC_EG_SECAM_SETTING)
    BVDC_P_MAKE_MISC(eSECAM_B,            BVDC_EG_SECAM_SETTING)
    BVDC_P_MAKE_MISC(eSECAM_G,            BVDC_EG_SECAM_SETTING)
    BVDC_P_MAKE_MISC(eSECAM_D,            BVDC_EG_SECAM_SETTING)
    BVDC_P_MAKE_MISC(eSECAM_K,            BVDC_EG_SECAM_SETTING)
    BVDC_P_MAKE_MISC(eSECAM_H,            BVDC_EG_SECAM_SETTING)
    {BFMT_VideoFmt_eMaxCount,             BVDC_EG_PAL_YUV_SETTING}
};

static const BVDC_P_EnvelopGeneratorSetting s_aulEG_Tbl_YUV_N[] =
{
    BVDC_P_MAKE_MISC(ePAL_M,              BVDC_EG_PALM_YUV_SETTING)
    {BFMT_VideoFmt_eMaxCount,             BVDC_EG_PAL_YUV_SETTING}
};

static const BVDC_P_EnvelopGeneratorSetting s_aulEG_Tbl_RGB[] =
{
    {BFMT_VideoFmt_eMaxCount,           BVDC_EG_COMPONET_SETTING}
};

static const BVDC_P_EnvelopGeneratorSetting s_aulEG_Tbl_SDYPrPb[] =
{
    {BFMT_VideoFmt_eMaxCount,           BVDC_EG_COMPONET_SETTING}
};

static const BVDC_P_EnvelopGeneratorSetting s_aulEG_Tbl_HDYPrPb[] =
{
    {BFMT_VideoFmt_eMaxCount,           BVDC_EG_COMPONET_SETTING}
};

static const BVDC_P_EnvelopGeneratorSetting s_aulEG_Tbl_Hsync[] =
{
    {BFMT_VideoFmt_eMaxCount,           BVDC_EG_COMPONET_SETTING}
};

static const BVDC_P_EnvelopGeneratorSetting s_aulEG_Tbl_Unknown[] =
{
    {BFMT_VideoFmt_eMaxCount,           BVDC_EG_COMPONET_SETTING}
};

/* VF Envelop Generator Control settings */
static const BVDC_P_EnvelopGeneratorInfo s_aEnvelopGeneratorTable[] =
{
    /* Output Color Space            Envelope Generator Control Setting */
    {BVDC_P_Output_eYQI,             s_aulEG_Tbl_YQI                      },
    {BVDC_P_Output_eYQI_M,           s_aulEG_Tbl_YQI_M                    },
    {BVDC_P_Output_eYUV,             s_aulEG_Tbl_YUV                      },
    {BVDC_P_Output_eYUV_M,           s_aulEG_Tbl_YUV_N                    },
    {BVDC_P_Output_eYUV_N,           s_aulEG_Tbl_YUV_N                    },
    {BVDC_P_Output_eYUV_NC,          s_aulEG_Tbl_YUV                      },
    {BVDC_P_Output_eYDbDr_LDK,       s_aulEG_Tbl_YUV                      },
    {BVDC_P_Output_eYDbDr_BG,        s_aulEG_Tbl_YUV                      },
    {BVDC_P_Output_eYDbDr_H,         s_aulEG_Tbl_YUV                      },
    /* special handling for 480p and 576p, see above */
    {BVDC_P_Output_eSDYPrPb,         s_aulEG_Tbl_SDYPrPb                  },
    {BVDC_P_Output_eSDRGB,           s_aulEG_Tbl_RGB                      },
    {BVDC_P_Output_eHDYPrPb,         s_aulEG_Tbl_HDYPrPb                  },
    {BVDC_P_Output_eHDRGB,           s_aulEG_Tbl_HDYPrPb                  },
    {BVDC_P_Output_eHsync,           s_aulEG_Tbl_Hsync                    },
    {BVDC_P_Output_eUnknown,         s_aulEG_Tbl_Unknown                  }
};
#endif


/****************************************************************
 *  Rate Manager Tables
 ****************************************************************/
static const BVDC_P_RmTableInfo s_aRmFullRate_Tbl[] =
{
    BVDC_P_MAKE_RMTBL(25_2MHz,         s_aulRmTable_25_2,         BDBG_STRING("25.20"))
    BVDC_P_MAKE_RMTBL(27MHz_MUL_1_001, s_aulRmTable_27_Mul_1001,  BDBG_STRING("27.00*1.001"))
    BVDC_P_MAKE_RMTBL(74_25MHz,        s_aulRmTable_74_25,        BDBG_STRING("74.25"))
    BVDC_P_MAKE_RMTBL(148_5MHz,        s_aulRmTable_148_5,        BDBG_STRING("148.5"))
    BVDC_P_MAKE_RMTBL(56_304MHz,       s_aulRmTable_56_304,       BDBG_STRING("56.304"))
    BVDC_P_MAKE_RMTBL(67_565MHz,       s_aulRmTable_67_565,       BDBG_STRING("67.565"))
    BVDC_P_MAKE_RMTBL(39_79MHz,        s_aulRmTable_39_79,        BDBG_STRING("39.79"))
    BVDC_P_MAKE_RMTBL(65MHz,           s_aulRmTable_65,           BDBG_STRING("64.99584"))
    BVDC_P_MAKE_RMTBL(65_286MHz,       s_aulRmTable_65_286,       BDBG_STRING("65.286"))
    BVDC_P_MAKE_RMTBL(60_4656MHz,      s_aulRmTable_60_4656,      BDBG_STRING("60.4656"))
    BVDC_P_MAKE_RMTBL(64_022MHz,       s_aulRmTable_64_0224,      BDBG_STRING("64.0224"))
    BVDC_P_MAKE_RMTBL(162MHz,          s_aulRmTable_162,          BDBG_STRING("162"))
    BVDC_P_MAKE_RMTBL(23_75MHz,        s_aulRmTable_23_75,        BDBG_STRING("23.75"))
    BVDC_P_MAKE_RMTBL(83_5MHz,         s_aulRmTable_83_5,         BDBG_STRING("83.5"))
    BVDC_P_MAKE_RMTBL(108MHz,          s_aulRmTable_108,          BDBG_STRING("108"))
    BVDC_P_MAKE_RMTBL(85_5MHz,         s_aulRmTable_85_5,         BDBG_STRING("85.5"))
    BVDC_P_MAKE_RMTBL(106_5MHz,        s_aulRmTable_106_5,        BDBG_STRING("106.5"))
    BVDC_P_MAKE_RMTBL(54MHz_MUL_1_001, s_aulRmTable_54_Mul_1001,  BDBG_STRING("54.00*1.001"))
    BVDC_P_MAKE_RMTBL(297MHz,          s_aulRmTable_297,          BDBG_STRING("297"))
    BVDC_P_MAKE_RMTBL(594MHz,          s_aulRmTable_297,          BDBG_STRING("594"))/*use TWICE_DTG_RM setting in MISC_DVI_DTG_0_MASTER_SEL */
};
#define BVDC_P_FULLRATE_TBL_SIZE (sizeof(s_aRmFullRate_Tbl) / sizeof(BVDC_P_RmTableInfo))

static const BVDC_P_RmTableInfo s_aRmDropRate_Tbl[] =
{
    BVDC_P_MAKE_RMTBL(25_2MHz_DIV_1_001,   s_aulRmTable_25_2_Div_1001,   BDBG_STRING("25.20/1.001"))
    BVDC_P_MAKE_RMTBL(27MHz,               s_aulRmTable_27,              BDBG_STRING("27.00"))
    BVDC_P_MAKE_RMTBL(74_25MHz_DIV_1_001,  s_aulRmTable_74_25_Div_1001,  BDBG_STRING("74.25/1.001"))
    BVDC_P_MAKE_RMTBL(148_5MHz_DIV_1_001,  s_aulRmTable_148_5_Div_1001,  BDBG_STRING("148.5/1.001"))
    BVDC_P_MAKE_RMTBL(67_565MHz_DIV_1_001, s_aulRmTable_67_565_Div_1001, BDBG_STRING("67.565/1.001"))
    BVDC_P_MAKE_RMTBL(39_79MHz_DIV_1_001,  s_aulRmTable_39_79_Div_1001,  BDBG_STRING("39.79/1.001"))
    BVDC_P_MAKE_RMTBL(65MHz_DIV_1_001,     s_aulRmTable_65_Div_1001,     BDBG_STRING("64.99584/1.001"))
    BVDC_P_MAKE_RMTBL(65_286MHz_DIV_1_001, s_aulRmTable_65_286_Div_1001, BDBG_STRING("65.286/1.001"))
    BVDC_P_MAKE_RMTBL(64_022MHz_DIV_1_001, s_aulRmTable_64_0224_Div_1001,BDBG_STRING("64.0224/1.001"))
    BVDC_P_MAKE_RMTBL(23_75MHz_DIV_1_001,  s_aulRmTable_23_75_Div_1001,  BDBG_STRING("23.75/1.001"))
    BVDC_P_MAKE_RMTBL(83_5MHz_DIV_1_001,   s_aulRmTable_83_5_Div_1001,   BDBG_STRING("83.5/1.001"))
    BVDC_P_MAKE_RMTBL(108MHz_DIV_1_001,    s_aulRmTable_108_Div_1001,    BDBG_STRING("108/1.001"))
    BVDC_P_MAKE_RMTBL(85_5MHz_DIV_1_001,   s_aulRmTable_85_5_Div_1001,   BDBG_STRING("85.5/1.001"))
    BVDC_P_MAKE_RMTBL(106_5MHz_DIV_1_001,  s_aulRmTable_106_5_Div_1001,  BDBG_STRING("106.5/1.001"))
    BVDC_P_MAKE_RMTBL(54MHz,               s_aulRmTable_54,              BDBG_STRING("54.00"))
    BVDC_P_MAKE_RMTBL(297MHz_DIV_1_001,    s_aulRmTable_297_Div_1001,    BDBG_STRING("297/1.001"))
    BVDC_P_MAKE_RMTBL(594MHz_DIV_1_001,    s_aulRmTable_297_Div_1001,    BDBG_STRING("594/1.001"))/*use TWICE_DTG_RM setting in MISC_DVI_DTG_0_MASTER_SEL */
};
#define BVDC_P_DROPRATE_TBL_SIZE (sizeof(s_aRmDropRate_Tbl) / sizeof(BVDC_P_RmTableInfo))

#if BVDC_P_NUM_SHARED_SM
/****************************************************************
 *  ColorSpace Data Table
 *  This table bears an entry for every colorspace.  Each
 *  entry specifies the config tables needed to support that
 *  colorspace.
 *
 *  Some colorspaces are used by both HD and SD display formats,
 *  and their entries include both HD and SD configuration tables
 *  accordingly.
 *
 *  Configuration tables are loaded by selecting the correct
 *  table entry according to whichever colorspace the display is
 *  configured for, and then selecting the correct configuration
 *  tables from that entry, depending on whether the display
 *  format is HD or SD.
 *
 *  BVDC_P_Output_eSDYPrPb is a special case.  While it is
 *  technically an SD-only colorspace, the HD formats 480p and
 *  576p both use it for loading CSC tables elsewhere, but still
 *  need to be configured as HD here, and thus, HD configuration
 *  tables exist in the entry in order to comply with the
 *  convention used above.
 *
 *  This table should not need to be modified when
 *  adding formats.
 *
 *  Note:
 *    1. NTSC uses YQI settings; NTSC_J uses YQI_M settings;
 *    2. PAL_M has the same CSC matrix as PAL_N, both with 7.5IRE
 *       pedestal on luma, but the same chroma as PAL/BDGHI;
 *    3. PAL_M has sub-carrier freq at 3.575611.49 MHz; PAL_N has the
 *       same FSC as PAL/BDGHI at 4.433618.75 MHz;
 *    4. PAL_NC has the same CSC matrix as PAL; but PAL_NC
 *       has sub-carrier frequency at 3.58205625 MHz;
 ****************************************************************/
static const BVDC_P_ColorSpaceData s_aColorSpaceDataTable[] =
{
    /* Format                  Sm Table Tbl              SD Filter Table Tbl               HD Filter Table Tbl     SD Vf Table Tbl             HD Vf Table Tbl */
    {BVDC_P_Output_eYQI,       s_aulSmTable_Tbl_YQI,     BVDC_P_OutputFilter_eYQI,       BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YQI,     NULL},
    {BVDC_P_Output_eYQI_M,     s_aulSmTable_Tbl_YQI,     BVDC_P_OutputFilter_eYQI,       BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YQI_M,   NULL},
    {BVDC_P_Output_eYUV,       s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eYUV,       BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YUV,     NULL},
    {BVDC_P_Output_eYUV_M,     s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eYUV,       BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YUV,     NULL},
    {BVDC_P_Output_eYUV_N,     s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eYUV,       BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YUV,     NULL},
    {BVDC_P_Output_eYUV_NC,    s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eYUV,       BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YUV,     NULL},
    {BVDC_P_Output_eYDbDr_LDK, s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eSECAM,     BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YDbDr,   NULL},
    {BVDC_P_Output_eYDbDr_BG,  s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eSECAM,     BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YUV,     NULL},
    {BVDC_P_Output_eYDbDr_H,   s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eSECAM,     BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YUV,     NULL},
    /* special handling for 480p and 576p, see above */
    {BVDC_P_Output_eSDYPrPb,   s_aulSmTable_Tbl_SDYPrPb, BVDC_P_OutputFilter_eSDYPrPb,   BVDC_P_OutputFilter_eED,      s_aulSdVfTable_Tbl_SDYPrPb, s_aulHdVfTable_Tbl_HDYPrPb},
    {BVDC_P_Output_eSDRGB,     s_aulSmTable_Tbl_RGB,     BVDC_P_OutputFilter_eSDRGB,     BVDC_P_OutputFilter_eSDRGB,   s_aulSdVfTable_Tbl_SDYPrPb, s_aulHdVfTable_Tbl_HDYPrPb},
    {BVDC_P_Output_eHDYPrPb,   s_aulSmTable_Tbl_HDYPrPb, BVDC_P_OutputFilter_eNone,      BVDC_P_OutputFilter_eHDYPrPb, NULL,                       s_aulHdVfTable_Tbl_HDYPrPb},
    {BVDC_P_Output_eHDRGB,     s_aulSmTable_Tbl_HDYPrPb, BVDC_P_OutputFilter_eNone,      BVDC_P_OutputFilter_eHDRGB,   NULL,                       s_aulHdVfTable_Tbl_HDYPrPb},
    {BVDC_P_Output_eHsync,     s_aulSmTable_Tbl_Hsync,   BVDC_P_OutputFilter_eHsync,     BVDC_P_OutputFilter_eHDYPrPb, s_aulSdVfTable_Tbl_SDYPrPb, s_aulHdVfTable_Tbl_HDYPrPb},
    {BVDC_P_Output_eUnknown,   s_aulSmTable_Tbl_Unknown, BVDC_P_OutputFilter_eUnknown,   BVDC_P_OutputFilter_eHDYPrPb, s_aulSdVfTable_Tbl_Unknown, s_aulHdVfTable_Tbl_Unknown}
};

/* special handling for 480p and 576p at double pixel rate (HDMI) */
static const BVDC_P_ColorSpaceData s_aColorSpaceDataTable_54MHz[] =
{
    /* Format                  Sm Table Tbl              SD Filter Table Tbl             HD Filter Table Tbl           SD Vf Table Tbl             HD Vf Table Tbl */
    {BVDC_P_Output_eSDYPrPb,   s_aulSmTable_Tbl_SDYPrPb, BVDC_P_OutputFilter_eNone,      BVDC_P_OutputFilter_eSDYPrPb, NULL,                       s_aulHdVfTable_Tbl_54MHz_HDYPrPb},
    {BVDC_P_Output_eSDRGB,     s_aulSmTable_Tbl_RGB,     BVDC_P_OutputFilter_eNone,      BVDC_P_OutputFilter_eSDRGB,   NULL,                       s_aulHdVfTable_Tbl_54MHz_HDYPrPb},
    {BVDC_P_Output_eUnknown,   s_aulSmTable_Tbl_Unknown, BVDC_P_OutputFilter_eNone,      BVDC_P_OutputFilter_eUnknown, NULL,                       s_aulHdVfTable_Tbl_54MHz_Unknown}
};
#endif

/* For table size sanity check */
#define BVDC_P_DISPLAY_FMTINFO_COUNT \
    (sizeof(s_aFormatInfoTable) / sizeof(BVDC_P_FormatInfo))

#define BVDC_P_DISPLAY_FMTDATA_COUNT \
    (sizeof(s_aFormatDataTable) / sizeof(BVDC_P_FormatData))

#if BVDC_P_NUM_SHARED_VF
/* Pick color space data with colorspace search */
static const BVDC_P_ColorSpaceData* BVDC_P_GetColorSpaceData_isr(
    const BVDC_P_ColorSpaceData *paColorSpaceDataTable,
    BVDC_P_Output eOutputColorSpace)
{
    const BVDC_P_ColorSpaceData *paSpace = &paColorSpaceDataTable[0];
    while ((paSpace->eOutputColorSpace != eOutputColorSpace) &&
           (paSpace->eOutputColorSpace != BVDC_P_Output_eUnknown))
    {
        ++paSpace;
    }
    return paSpace;
}
#endif

/*************************************************************************
 *
 * End of static tables
 *
 **************************************************************************/

#ifndef BFMT_DO_PICK
/*************************************************************************
 *  {secret}
 *  Utility to return pointer to appropriate BVDC_P_FormatInfo
 **************************************************************************/
const BVDC_P_FormatInfo *BVDC_P_GetFormatInfo_isrsafe
(
    BFMT_VideoFmt eVideoFmt
)
{
#if (BDBG_DEBUG_BUILD)
    if(BVDC_P_DISPLAY_FMTINFO_COUNT != (BFMT_VideoFmt_eMaxCount+1))
    {
        BDBG_ERR(("BVDC_P_DISPLAY_FMTINFO_COUNT = %u", (unsigned int)BVDC_P_DISPLAY_FMTINFO_COUNT));
        BDBG_ASSERT(false);
    }
    BDBG_ASSERT(s_aFormatInfoTable[eVideoFmt].eVideoFmt == eVideoFmt);
    BDBG_ASSERT(eVideoFmt < BFMT_VideoFmt_eMaxCount);
#if (BVDC_P_TEST_ORDER_CORRECTNESS)
    {
        BFMT_VideoFmt eFormat;
        for(eFormat = 0; eFormat < BFMT_VideoFmt_eMaxCount; eFormat++)
        {
            BDBG_ASSERT(s_aFormatInfoTable[eFormat].eVideoFmt == eFormat);
        }
    }
#endif
#endif
    return &s_aFormatInfoTable[eVideoFmt];
}

/*************************************************************************
 *  {secret}
 *  Utility to return pointer to appropriate BVDC_P_FormatData
 **************************************************************************/
static const BVDC_P_FormatData *BVDC_P_GetFormatData_isrsafe
(
    BFMT_VideoFmt eVideoFmt
)
{
#if (BDBG_DEBUG_BUILD)
    if(BVDC_P_DISPLAY_FMTDATA_COUNT != (BFMT_VideoFmt_eMaxCount+1))
    {
        BDBG_ERR(("BVDC_P_DISPLAY_FMTDATA_COUNT = %u", (unsigned int)BVDC_P_DISPLAY_FMTDATA_COUNT));
        BDBG_ASSERT(false);
    }
    BDBG_ASSERT(s_aFormatDataTable[eVideoFmt].eVideoFmt == eVideoFmt);
    BDBG_ASSERT(eVideoFmt < BFMT_VideoFmt_eMaxCount);
#if (BVDC_P_TEST_ORDER_CORRECTNESS)
    {
        BFMT_VideoFmt eFormat;
        for(eFormat = 0; eFormat < BFMT_VideoFmt_eMaxCount; eFormat++)
        {
            BDBG_ASSERT(s_aFormatDataTable[eFormat].eVideoFmt == eFormat);
        }
    }
#endif
#endif
    return &s_aFormatDataTable[eVideoFmt];
}

#else /* #ifndef BFMT_DO_PICK */
/*************************************************************************
 *  {secret}
 *  Utility to return pointer to appropriate BVDC_P_FormatInfo
 **************************************************************************/
const BVDC_P_FormatInfo *BVDC_P_GetFormatInfo_isrsafe
(
    BFMT_VideoFmt eVideoFmt
)
{
    unsigned int ii;

    for (ii=0; ii<BVDC_P_DISPLAY_FMTINFO_COUNT; ii++)
    {
        if (s_aFormatInfoTable[ii].eVideoFmt == eVideoFmt)
        {
            /*printf("FmtVdcInfo found videoFmt %d at entry %d\n", eVideoFmt, ii);*/
            return &s_aFormatInfoTable[ii];
        }
    }
    /*printf("FmtVdcInfo found no entry for videoFmt %d!!!\n", eVideoFmt);*/
    return NULL;
}

/*************************************************************************
 *  {secret}
 *  Utility to return pointer to appropriate BVDC_P_FormatData
 **************************************************************************/
static const BVDC_P_FormatData *BVDC_P_GetFormatData_isrsafe
(
    BFMT_VideoFmt eVideoFmt
)
{
    unsigned int ii;

    for (ii=0; ii<BVDC_P_DISPLAY_FMTDATA_COUNT; ii++)
    {
        if (s_aFormatDataTable[ii].eVideoFmt == eVideoFmt)
        {
            /*printf("FmtVdcData found videoFmt %d at entry %d\n", eVideoFmt, ii);*/
            return &s_aFormatDataTable[ii];
        }
    }
    /*printf("FmtVdcData found no entry for videoFmt %d!!!\n", eVideoFmt);*/
    return NULL;
}
#endif /* #ifndef BFMT_DO_PICK */

/*************************************************************************
 *  {secret}
 *  Returns pointer to appropriate RamTable for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_GetRamTableSub_isr
(
    const BVDC_P_DisplayInfo *pDispInfo,
    bool                     bArib480p
)
{
    const uint32_t *pTable = NULL;

    /* Analog VEC, (480P or 576P) will always be double rate.
     * Improves frequency response. */
    if(BFMT_IS_480P(pDispInfo->pFmtInfo->eVideoFmt))
    {
        pTable =
            (bArib480p ?
                s_aulRamBVBInput_480pARIB_54MHz :
                s_aulAnalogMicrocode_480p54);
    }
    else if (BFMT_VideoFmt_e576p_50Hz      == pDispInfo->pFmtInfo->eVideoFmt )
    {
        pTable = s_aulAnalogMicrocode_576p54;
    }
    else if (
        pDispInfo->bWidthTrimmed &&
        BFMT_IS_NTSC(pDispInfo->pFmtInfo->eVideoFmt))
    {
        BDBG_MSG(("NTSC width = 704 samples microcode"));
        pTable = s_aulAnalogMicrocode_NTSC_704;
    }
    else if(BFMT_IS_NTSC(pDispInfo->pFmtInfo->eVideoFmt) &&
             (true == bArib480p))
    {
        pTable = s_aulRamBVBInput_NTSCARIB;
    }
    else
    {
        /*
         * Programming note: the following table supplies only 27 MHz
         * microcodes for 480P and 576P. The table does not supply any ARIB
         * microcodes. The table does not supply any "width trimmed"
         * microcodes.
         */
        pTable =
            BVDC_P_GetFormatData_isrsafe(pDispInfo->pFmtInfo->eVideoFmt)->pRamBVBInput;
    }

    return pTable;
}


/*************************************************************************
 *  {secret}
 *  Returns pointer to appropriate ItTable for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_GetItTableSub_isr
(
    const BVDC_P_DisplayInfo     *pDispInfo
)
{
    const uint32_t *pTable = NULL;

    pTable = BVDC_P_GetFormatData_isrsafe(pDispInfo->pFmtInfo->eVideoFmt)->pItTable;

    return pTable;
}


/*************************************************************************
 *  {secret}
 *  Returns pointer to appropriate ItConfig for display modes.
 **************************************************************************/
uint32_t BVDC_P_GetItConfigSub_isr
(
    const BVDC_P_DisplayInfo     *pDispInfo
)
{
    uint32_t ulItConfig=0;

    /* Table size sanity check!  Just in case someone added new format in fmt,
     * but forgot to add the new into these table. */
    ulItConfig =
        *(BVDC_P_GetFormatData_isrsafe(pDispInfo->pFmtInfo->eVideoFmt)->pulItConfig);

    return ulItConfig;
}


/*************************************************************************
 *  {secret}
 *  Returns pointer to appropriate DtramTable for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_GetDtramTable_isr
(
    const BVDC_P_DisplayInfo     *pDispInfo,
    const BFMT_VideoInfo         *pFmtInfo,
    bool                          bArib480p
)
{
    const uint32_t *pTable = NULL;
    uint32_t ulHdmiDropLines =
        (pFmtInfo->bInterlaced)                                 ?
        (pDispInfo->aulHdmiDropLines[pFmtInfo->eVideoFmt] /
            BVDC_P_FIELD_PER_FRAME)                                        :
        (pDispInfo->aulHdmiDropLines[pFmtInfo->eVideoFmt]);
    BAVC_HDMI_PixelRepetition eHdmiPixelRepetition;
    eHdmiPixelRepetition = pDispInfo->stHdmiSettings.eHdmiPixelRepetition;

    /* Analog VEC, 480P may be either single or double rate */
    if((BFMT_IS_480P(pFmtInfo->eVideoFmt)) &&
       (bArib480p))
    {
        if ((BAVC_HDMI_PixelRepetition_e1x == eHdmiPixelRepetition)||
            (BAVC_HDMI_PixelRepetition_e4x == eHdmiPixelRepetition))
        {
            pTable = s_aulDtRamBVBInput_DVI_480p_Drop1_54MHz;
        }
        else
        {
            pTable = s_aulDtRamBVBInput_DVI_480p_Drop1;
        }
    }
    else if(BFMT_IS_480P(pFmtInfo->eVideoFmt))
    {
        if ((BAVC_HDMI_PixelRepetition_e1x == eHdmiPixelRepetition)||
            (BAVC_HDMI_PixelRepetition_e4x == eHdmiPixelRepetition))
        {
            const uint32_t * const * apDropTbl =
                s_aulDtRamBVBInput_DVI_480p_54MHz_DropTbl;
            pTable = apDropTbl[ulHdmiDropLines];
        }
        else
        {
            const uint32_t * const * apDropTbl =
                s_aulDtRamBVBInput_DVI_480p_DropTbl;
            pTable = apDropTbl[ulHdmiDropLines];
        }
    }
    else if (BVDC_P_IS_CUSTOMFMT(pFmtInfo->eVideoFmt))
    {
        pTable = pFmtInfo->pCustomInfo->pDvoMicrocodeTbl;
    }
    /* Analog VEC, 576P may be either single or double rate */
    else if ((BFMT_VideoFmt_e576p_50Hz == pFmtInfo->eVideoFmt ) &&
             ((BAVC_HDMI_PixelRepetition_e1x == eHdmiPixelRepetition) ||
              (BAVC_HDMI_PixelRepetition_e4x == eHdmiPixelRepetition)) )
    {
        pTable = s_aulDviMicrocode_576p54;
    }
    else if (BVDC_P_GetFormatInfo_isrsafe(pFmtInfo->eVideoFmt)->bUseDropTbl)
    {
        const uint32_t * const * apDropTbl =
            BVDC_P_GetFormatData_isrsafe(pFmtInfo->eVideoFmt)->apDtRamBVBInput_DropTbl;
        pTable = apDropTbl[ulHdmiDropLines];
    }
    else
    {
        pTable = BVDC_P_GetFormatData_isrsafe(pFmtInfo->eVideoFmt)->pDtRamBVBInput;
    }

    BDBG_ASSERT(pTable);

    return pTable;
}


#if (BVDC_P_SUPPORT_ITU656_OUT)
/*************************************************************************
 *  {secret}
 *  Returns pointer to appropriate 656Dtram for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_Get656DtramTable_isr
(
    const BVDC_P_DisplayInfo     *pDispInfo
)
{
    const uint32_t *pTable = NULL;

    /* 656 only supports 480i and PAL */
    if (BFMT_IS_525_LINES(pDispInfo->pFmtInfo->eVideoFmt))
    {
        pTable = &(s_aul656Microcode_NTSC_704[0]);
    }
    else if (BFMT_IS_625_LINES(pDispInfo->pFmtInfo->eVideoFmt))
    {
        pTable = &(s_aul656Microcode_PAL_I[0]);
    }

    return pTable;
}
#endif /* BVDC_P_SUPPORT_ITU656_OUT */

#if BVDC_P_NUM_SHARED_SM
/*************************************************************************
 *  {secret}
 *  Returns pointer to appropriate SmTable for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_GetSmTable_isr
(
    const BVDC_P_DisplayInfo     *pDispInfo,
    BVDC_P_Output                 eOutputCS
)
{
    const BVDC_P_SmTableInfo *pSmTable_Tbl = NULL;
    const uint32_t           *pTable = NULL;
    uint32_t                  ulCurSmTable = 0;

    /* to make coverity check silent */
    if (eOutputCS > BVDC_P_Output_eUnknown)
    {
        BDBG_ASSERT(eOutputCS <= BVDC_P_Output_eUnknown);
        return NULL;
    }

    pSmTable_Tbl = s_aColorSpaceDataTable[eOutputCS].pSmTable_Tbl;

    BDBG_ASSERT(pSmTable_Tbl);

    while(true)
    {
        if((pSmTable_Tbl[ulCurSmTable].eVideoFmt == pDispInfo->pFmtInfo->eVideoFmt) ||
           (pSmTable_Tbl[ulCurSmTable].eVideoFmt == BFMT_VideoFmt_eMaxCount))
        {
            pTable = pSmTable_Tbl[ulCurSmTable].pSmTable;
            /* Special case */
            if (
                pDispInfo->bWidthTrimmed &&
                BFMT_IS_NTSC(pDispInfo->pFmtInfo->eVideoFmt))
            {
                pTable = s_aulSmTable_YQI_704;
            }
            break;
        }

        ulCurSmTable++;
    }

    BDBG_ASSERT(pTable);

    return pTable;
}


/*************************************************************************
 *  {secret}
 *  Returns pointers to appropriate channel filters for display modes.
 **************************************************************************/
BERR_Code BVDC_P_GetChFilters_isr
(
    const BVDC_P_DisplayInfo *pDispInfo,
    BVDC_P_Output             eOutputColorSpace,
    const uint32_t          **ppChFilter_Ch0,
    const uint32_t          **ppChFilter_Ch1,
    const uint32_t          **ppChFilter_Ch2
)
{
    BVDC_P_OutputFilter           eOutputFilter;
    const BVDC_P_ColorSpaceData *paSpace;

    /* to make coverity check silent */
    if (eOutputColorSpace > BVDC_P_Output_eUnknown)
    {
        BDBG_ASSERT(eOutputColorSpace <= BVDC_P_Output_eUnknown);
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Analog VEC, (480P or 576P) will always be double rate.
     * Improves frequency response. */
    if (VIDEO_FORMAT_IS_ED(pDispInfo->pFmtInfo->eVideoFmt))
    {
        paSpace =
            BVDC_P_GetColorSpaceData_isr(
                s_aColorSpaceDataTable_54MHz, eOutputColorSpace);
    }
    else
    {
        paSpace =
            BVDC_P_GetColorSpaceData_isr(
                s_aColorSpaceDataTable, eOutputColorSpace);
    }

    if (VIDEO_FORMAT_IS_HD(pDispInfo->pFmtInfo->eVideoFmt))
    {
        eOutputFilter = paSpace->eHdOutputFilter;
    }
    else
    {
        eOutputFilter = paSpace->eSdOutputFilter;
    }

    BDBG_ASSERT(
        s_aulFilterTable_Tbl[eOutputFilter].eOutputFilter == eOutputFilter);

    *ppChFilter_Ch0 = s_aulFilterTable_Tbl[eOutputFilter].pChFilter_Ch0;
    *ppChFilter_Ch1 = s_aulFilterTable_Tbl[eOutputFilter].pChFilter_Ch1;
    *ppChFilter_Ch2 = s_aulFilterTable_Tbl[eOutputFilter].pChFilter_Ch2;

    BDBG_ASSERT(*ppChFilter_Ch0);
    BDBG_ASSERT(*ppChFilter_Ch1);
    BDBG_ASSERT(*ppChFilter_Ch2);

    return BERR_SUCCESS;
}
#endif


#if BVDC_P_NUM_SHARED_SDSRC
/*************************************************************************
 *  {secret}
 *  Returns appropriate SrcControl for display modes.
 **************************************************************************/
uint32_t BVDC_P_GetSrcControl_isr
(
    BVDC_P_Output eOutputCS
)
{
    uint32_t ulSrcControl =
        (((eOutputCS == BVDC_P_Output_eSDRGB) || (eOutputCS == BVDC_P_Output_eHDRGB)) ?
            s_ulSrcControlRGB : s_ulSrcControlNotRGB);

    return ulSrcControl;
}
#endif


#if BVDC_P_NUM_SHARED_VF
/*************************************************************************
 *  {secret}
 *  Returns pointer to appropriate VfTable for display modes.
 **************************************************************************/
void BVDC_P_FillVfTable_isr
(
    const BVDC_P_DisplayInfo *pDispInfo,
    BVDC_P_Output             eOutputColorSpace,
    uint32_t                 *pTable,
    uint32_t                 *pulNsaeReg,
    BVDC_P_Display_ShaperSettings *pstShaperSettings
)
{
    const BVDC_P_ColorSpaceData *paSpace;
    int index;
    const BVDC_P_VfTableInfo *pVfTable_Tbl = NULL;
    const uint32_t           *pSource      = NULL;
    uint32_t                  ulCurVfTable = 0;
    bool                      bRgbEdit     = false;
    bool                      bHsyncEdit   = false;

    BDBG_ASSERT(eOutputColorSpace <= BVDC_P_Output_eUnknown);

    /* Programming note: For VF Table, RGB is handled as a modification of
     * ----------------  YPrPb VF Table. Because automation.
     */

    switch (eOutputColorSpace)
    {
    case BVDC_P_Output_eSDRGB:
        eOutputColorSpace = BVDC_P_Output_eSDYPrPb;
        bRgbEdit = true;
        break;
    case BVDC_P_Output_eHDRGB:
        eOutputColorSpace = BVDC_P_Output_eHDYPrPb;
        bRgbEdit = true;
        break;
    case BVDC_P_Output_eHsync:
        eOutputColorSpace = BVDC_P_Output_eHsync;
        bHsyncEdit = true;
        break;
    default:
        break;
    }

    /* Analog VEC, (480P or 576P) will always be double rate. Improves
     * frequency response. */
    if (VIDEO_FORMAT_IS_ED(pDispInfo->pFmtInfo->eVideoFmt))
    {
        paSpace =
            BVDC_P_GetColorSpaceData_isr(
                s_aColorSpaceDataTable_54MHz, eOutputColorSpace);
    }
    else
    {
        paSpace =
            BVDC_P_GetColorSpaceData_isr(
                s_aColorSpaceDataTable, eOutputColorSpace);
    }

    if(VIDEO_FORMAT_IS_HD(pDispInfo->pFmtInfo->eVideoFmt))
    {
        pVfTable_Tbl = paSpace->pHdVfTable_Tbl;
    }
    else
    {
        pVfTable_Tbl = paSpace->pSdVfTable_Tbl;
    }
    BDBG_ASSERT(pVfTable_Tbl);

    /* Locate vf table */
    while(true)
    {
        if((pVfTable_Tbl[ulCurVfTable].eVideoFmt ==
                pDispInfo->pFmtInfo->eVideoFmt) ||
           (pVfTable_Tbl[ulCurVfTable].eVideoFmt ==
                BFMT_VideoFmt_eMaxCount))
        {
            pSource = pVfTable_Tbl[ulCurVfTable].pVfTable;
            break;
        }
        ulCurVfTable++;
    }

    /* Return VF table settings */
    if(pTable)
    {
        BDBG_ASSERT (pSource);
        BKNI_Memcpy (
            (void*)pTable, (void*)pSource,
            sizeof(uint32_t) * BVDC_P_VF_TABLE_SIZE);
        if (bRgbEdit || bHsyncEdit)
        {
            index = 0;
            pTable[index] &= ~(
                BVDC_P_VF_MASK       (FORMAT_ADDER, ADD_SYNC_TO_OFFSET      ) |
                BVDC_P_VF_MASK       (FORMAT_ADDER, C1_OFFSET               ) |
                BVDC_P_VF_MASK       (FORMAT_ADDER, C2_OFFSET               ) |
                BVDC_P_VF_MASK       (FORMAT_ADDER, OFFSET                  ) );
            pTable[index] |= (
                BVDC_P_VF_FIELD_DATA (FORMAT_ADDER, ADD_SYNC_TO_OFFSET, 0x0 ) |
                BVDC_P_VF_FIELD_DATA (FORMAT_ADDER, C1_OFFSET         , 0x1 ) |
                BVDC_P_VF_FIELD_DATA (FORMAT_ADDER, C2_OFFSET         , 0x1 ) |
                BVDC_P_VF_FIELD_DATA (FORMAT_ADDER, OFFSET            , 0xFB) );
        }
        if (bHsyncEdit)
        {
            index = (BCHP_VF_0_NEG_SYNC_VALUES - BCHP_VF_0_FORMAT_ADDER)/4;
            pTable[index] &= ~(
#if (BVDC_P_SUPPORT_VEC_VF_VER == 1)
                BVDC_P_VF_MASK       (NEG_SYNC_VALUES, VALUE2      ) |
#endif
                BVDC_P_VF_MASK       (NEG_SYNC_VALUES, VALUE1      ) |
                BVDC_P_VF_MASK       (NEG_SYNC_VALUES, VALUE0      ) );
            pTable[index] |= (
#if (BVDC_P_SUPPORT_VEC_VF_VER == 1)
                BVDC_P_VF_FIELD_DATA (NEG_SYNC_VALUES, VALUE2, 0xFB) |
#endif
                BVDC_P_VF_FIELD_DATA (NEG_SYNC_VALUES, VALUE1, 0xFB) |
                BVDC_P_VF_FIELD_DATA (NEG_SYNC_VALUES, VALUE0, 0xFB) );
        }
    }
    if (pulNsaeReg)
    {
        uint32_t regVal;
        BDBG_ASSERT (pSource);
        regVal = pSource[BVDC_P_VF_TABLE_SIZE];
#if (BVDC_P_SUPPORT_VEC_VF_VER >= 2)
        if (bHsyncEdit)
        {
            regVal &=
                ~BVDC_P_VF_MASK       (NEG_SYNC_AMPLITUDE_EXTN, VALUE2      );
            regVal |=
                 BVDC_P_VF_FIELD_DATA (NEG_SYNC_AMPLITUDE_EXTN, VALUE2, 0xFB);
        }
#endif
        *pulNsaeReg = regVal;
    }

    /* Extract vf left cut as well */
    if(pstShaperSettings)
    {
        *pstShaperSettings =
            BVDC_P_GetFormatInfo_isrsafe(pDispInfo->pFmtInfo->eVideoFmt)->stShaper;
        if (pDispInfo->bWidthTrimmed &&
            BFMT_IS_NTSC(pDispInfo->pFmtInfo->eVideoFmt) &&
            (pDispInfo->eMacrovisionType < BVDC_MacrovisionType_eTest01))
        {
            BDBG_MSG(("NTSC width = 704 samples, left cut 8 pixels"));
            pstShaperSettings->ulSavRemove = 8;
        }
    }
}


/*************************************************************************
 *  {secret}
 *  Extracts SUM_OF_TAPS from VF_n_MISC register and converts it to a
 *  meaningful number.
 **************************************************************************/
uint32_t BVDC_P_ExtractSumOfTaps_isr (uint32_t vfMiscRegVal)
{
    uint32_t val;

    val = BCHP_GET_FIELD_DATA (vfMiscRegVal, VF_0_MISC, SUM_OF_TAPS);
    switch (val)
    {
        case BCHP_VF_0_MISC_SUM_OF_TAPS_SUM_256:
        case BCHP_VF_0_MISC_SUM_OF_TAPS_SUM_512:
#ifdef BCHP_VF_0_MISC_SUM_OF_TAPS_SUM_1024
        case BCHP_VF_0_MISC_SUM_OF_TAPS_SUM_1024:
#endif
#ifdef BCHP_VF_0_MISC_SUM_OF_TAPS_SUM_2048
        case BCHP_VF_0_MISC_SUM_OF_TAPS_SUM_2048:
#endif
            break;
        default:
            BDBG_ERR(("Invalid user VF filter settings"));
            return BERR_INVALID_PARAMETER;
            break;
    }

    return val;
}


/*************************************************************************
 *  {secret}
 *  Returns appropriate VF.MISC register value for display modes.
 **************************************************************************/
uint32_t BVDC_P_GetVfMisc_isr
(
    const BVDC_P_DisplayInfo *pDispInfo,
    BVDC_P_Output             eOutputColorSpace
)
{
    uint32_t aulVfTable[BVDC_P_VF_TABLE_SIZE];
    uint32_t index;

    BVDC_P_FillVfTable_isr (
        pDispInfo, eOutputColorSpace, aulVfTable, NULL, NULL);
    index = (BCHP_VF_0_MISC - BCHP_VF_0_FORMAT_ADDER) / 4;
    return aulVfTable[index];
}

/*************************************************************************
 *  {secret}
 *  Returns VF envelop generator control settings
 **************************************************************************/
uint32_t BVDC_P_GetVfEnvelopGenerator_isr
(
    const BVDC_P_DisplayInfo *pDispInfo,
    BVDC_P_Output             eOutputCS
)
{
    const BVDC_P_EnvelopGeneratorSetting *pEG_Tbl = NULL;
    uint32_t    ulEGSetting;
    uint32_t    ulEG_idx = 0;

    /* to make coverity check silent */
    if (eOutputCS > BVDC_P_Output_eUnknown)
    {
        BDBG_ASSERT(eOutputCS <= BVDC_P_Output_eUnknown);
        return 0;
    }

    pEG_Tbl = s_aEnvelopGeneratorTable[eOutputCS].pEG_Tbl;

    BDBG_ASSERT(pEG_Tbl);

    while(true)
    {
        if((pEG_Tbl[ulEG_idx].eVideoFmt == pDispInfo->pFmtInfo->eVideoFmt) ||
           (pEG_Tbl[ulEG_idx].eVideoFmt == BFMT_VideoFmt_eMaxCount))
        {
            ulEGSetting = pEG_Tbl[ulEG_idx].ulSetting;
            break;
        }

        ulEG_idx++;
    }

    return ulEGSetting;
}
#endif

/*************************************************************************
 *  {secret}
 * Finds appropriate RmTable and frequency info for display modes.
 * Will update:
 *    pRateInfo->ulPixelClockRate
 *    pRateInfo->ulVertRefreshRate
 **************************************************************************/
static void BVDC_P_SelectRmTable_isr
(
    const BVDC_P_DisplayInfo  *pDispInfo,
    const BFMT_VideoInfo      *pFmtInfo,
    const BVDC_P_RmTableInfo **pRmTable_Tbl,
    bool                       bFullRate,
    uint32_t                  *pulRmTable_TblSize,
    BAVC_VdcDisplay_Info      *pRateInfo )
{
    BAVC_VdcDisplay_Info stRateInfo;

    /* intialize with full rate. */
    stRateInfo.ulPixelClkRate    = pFmtInfo->ulPxlFreqMask;
    stRateInfo.ulPixelClockRate  = pFmtInfo->ulPxlFreq;
    stRateInfo.ulVertRefreshRate = pFmtInfo->ulVertFreq;

    if(pDispInfo->bMultiRateAllow)
    {
        if(bFullRate)
        {
            *pRmTable_Tbl = s_aRmFullRate_Tbl;
            *pulRmTable_TblSize = BVDC_P_FULLRATE_TBL_SIZE;
        }
        else
        {
            *pRmTable_Tbl = s_aRmDropRate_Tbl;
            *pulRmTable_TblSize = BVDC_P_DROPRATE_TBL_SIZE;
            stRateInfo.ulPixelClockRate  = (pFmtInfo->ulPxlFreq  * 1000) / 1001;
            stRateInfo.ulVertRefreshRate = (pFmtInfo->ulVertFreq * 1000) / 1001;
        }
    }
    else
    {
        /* Single vertical refresh rate. */
        if(!VIDEO_FORMAT_IS_SD(pFmtInfo->eVideoFmt) &&
           !VIDEO_FORMAT_IS_ED(pFmtInfo->eVideoFmt))
        {
            *pRmTable_Tbl = s_aRmFullRate_Tbl;
            *pulRmTable_TblSize = BVDC_P_FULLRATE_TBL_SIZE;
        }
        else
        {
            *pRmTable_Tbl = s_aRmDropRate_Tbl;
            *pulRmTable_TblSize = BVDC_P_DROPRATE_TBL_SIZE;
            if(BFMT_IS_525_LINES(pFmtInfo->eVideoFmt))
            {
                stRateInfo.ulPixelClockRate  = (pFmtInfo->ulPxlFreq  * 1000) / 1001;
                stRateInfo.ulVertRefreshRate = (pFmtInfo->ulVertFreq * 1000) / 1001;
            }
        }
    }

    if(pRateInfo)
    {
        *pRateInfo = stRateInfo;
    }

    return;
}

BERR_Code BVDC_P_GetRmTable_isr
(
    const BVDC_P_DisplayInfo *pDispInfo,
    const BFMT_VideoInfo     *pFmtInfo,
    const uint32_t          **ppTable,
    bool                      bFullRate,
    BAVC_VdcDisplay_Info     *pRateInfo
)
{
    uint64_t                  ulPxlFreqMask;
    const BVDC_P_RmTableInfo *pRmTable_Tbl=NULL;
    const uint32_t           *pTable = NULL;
    BERR_Code                 eErr = BVDC_ERR_UNSUPPORTED_PIXEL_RATE;
    uint32_t                  ulRmTable_TblSize = 0;
    uint32_t                  ulCurRmTable = 0;

    /* Select pxl freq.  Need a way to notify HDMI that our pixel rate has
     * changed.  Handle by callback */
    BVDC_P_SelectRmTable_isr(pDispInfo, pFmtInfo, &pRmTable_Tbl, bFullRate,
        &ulRmTable_TblSize, pRateInfo);
    ulPxlFreqMask = pFmtInfo->ulPxlFreqMask;

    /* For (480P or 576P), operate the analog VEC at double rate.
     * Improves frequency response. */
    if ((BFMT_VideoFmt_e480p      == pFmtInfo->eVideoFmt) ||
        (BFMT_VideoFmt_e720x483p  == pFmtInfo->eVideoFmt) ||
        (BFMT_VideoFmt_e576p_50Hz == pFmtInfo->eVideoFmt))
    {
        if (ulPxlFreqMask & BFMT_PXL_27MHz)
        {
            ulPxlFreqMask &= ~BFMT_PXL_27MHz;
            ulPxlFreqMask |=  BFMT_PXL_54MHz;
        }
        if (ulPxlFreqMask & BFMT_PXL_27MHz_MUL_1_001)
        {
            ulPxlFreqMask &= ~BFMT_PXL_27MHz_MUL_1_001;
            ulPxlFreqMask |=  BFMT_PXL_54MHz_MUL_1_001;
        }
    }

    /* find and get correct rmtable and pixel clock rate */
    for (ulCurRmTable = 0; ulCurRmTable < ulRmTable_TblSize; ulCurRmTable++)
    {
        if (pRmTable_Tbl[ulCurRmTable].ulPixelClkRate & ulPxlFreqMask)
        {
            pTable = pRmTable_Tbl[ulCurRmTable].pRmTable;
            pRateInfo->ulPixelClkRate =
                pRmTable_Tbl[ulCurRmTable].ulPixelClkRate;
            break;
        }
    }

    if (pTable)
    {
        *ppTable = pTable;
        eErr = BERR_SUCCESS;
    }

    return eErr;
}

/*************************************************************************
 *  {secret}
 *  Returns pointer to appropriate RmString name for display modes.
 **************************************************************************/
const char* BVDC_P_GetRmString_isr
(
    const BVDC_P_DisplayInfo     *pDispInfo,
    const BFMT_VideoInfo         *pFmtInfo
)
{
    const BVDC_P_RmTableInfo *pRmTable_Tbl=NULL;
    uint32_t                  ulRmTable_TblSize = 0;
    uint32_t                  ulCurRmTable = 0;

    /* Select pxl freq.  Need a way to notify HDMI that our pixel rate has
     * changed.  Handle by callback */

    /* find rm and return its text string */
    BVDC_P_SelectRmTable_isr(pDispInfo, pFmtInfo, &pRmTable_Tbl, pDispInfo->bFullRate,
        &ulRmTable_TblSize, NULL);

    /* find and get correct rmtable and pixel clock rate */
    for (ulCurRmTable = 0; ulCurRmTable < ulRmTable_TblSize; ulCurRmTable++)
    {
        if (pRmTable_Tbl[ulCurRmTable].ulPixelClkRate == pDispInfo->stRateInfo.ulPixelClkRate)
        {
            return pRmTable_Tbl[ulCurRmTable].pString;
        }
    }

    return BDBG_STRING("Unknown-");
}


/*************************************************************************
 *
 *
 */
/* HDMI color depth support */

static uint64_t BVDC_P_HdmiRm_HalfRate_isr (uint64_t ulPixelClkRate)
{
    unsigned int index;
    static const uint64_t P_half[][2] =
    {
        {BFMT_PXL_594MHz          , BFMT_PXL_297MHz          },
        {BFMT_PXL_594MHz_DIV_1_001, BFMT_PXL_297MHz_DIV_1_001}
    };
    #define P_N_HALF (sizeof(P_half) / sizeof(P_half[0]))
    uint64_t retval = (uint64_t)0;
    for (index = 0 ; index < P_N_HALF ; ++index)
    {
        if (ulPixelClkRate == P_half[index][0])
        {
            retval = P_half[index][1];
            break;
        }
    }
    return retval;
}

bool BVDC_P_HdmiRmTable_isr
(
    BFMT_VideoFmt             eVideoFmt,
    uint64_t                  ulPixelClkRate,
    BAVC_HDMI_BitsPerPixel    eHdmiColorDepth,
    BAVC_HDMI_PixelRepetition eHdmiPixelRepetition,
    BAVC_Colorspace           eColorComponent,
    const BVDC_P_RateInfo**   ppRateInfo
)
{
    bool     bModified;
    uint32_t iLookup, iClock, iRm;
    uint64_t ulPixelBaseRate;
    BAVC_HDMI_PixelRepetition ePxlRep = eHdmiPixelRepetition;
    const BVDC_P_RateInfo* pRateInfo = NULL;

    /* Obtain unmodified pixel rate */
    if (ulPixelClkRate <= (uint64_t)0xFFFFFFFF)
    {
        ulPixelBaseRate = ulPixelClkRate;
        bModified = false;
    }
    else
    {
        ulPixelBaseRate = ulPixelClkRate >> 32;
        bModified = true;
    }

    /* for 480i/576i Pixel Repetition 2x is inherent from VEC so use the
     * same RM as BAVC_HDMI_PixelRepetition_eNone */
    if(BFMT_IS_NTSC(eVideoFmt) || BFMT_IS_PAL(eVideoFmt))
    {
        if(eHdmiPixelRepetition == BAVC_HDMI_PixelRepetition_e1x)
        {
            ePxlRep = BAVC_HDMI_PixelRepetition_eNone;
        }
    }
    else if((eVideoFmt != BFMT_VideoFmt_e480p     ) &&
            (eVideoFmt != BFMT_VideoFmt_e720x483p ) &&
            (eVideoFmt != BFMT_VideoFmt_e576p_50Hz))
    {
        if(eHdmiPixelRepetition != BAVC_HDMI_PixelRepetition_eNone)
        {
            ePxlRep = BAVC_HDMI_PixelRepetition_eNone;
        }
    }

    /* Handle color space decision */
    switch (eColorComponent)
    {
    case BAVC_Colorspace_eYCbCr420:
        /* Use the 4:4:4 info for the half frequency */
        ulPixelBaseRate = BVDC_P_HdmiRm_HalfRate_isr (ulPixelBaseRate);
        break;
    case BAVC_Colorspace_eYCbCr422:
        /* Use the 8 bit-per-pixel 4:4:4 info */
        eHdmiColorDepth = BAVC_HDMI_BitsPerPixel_e24bit;
        break;
    case BAVC_Colorspace_eYCbCr444:
        /* No modification */
        break;
    case BAVC_Colorspace_eRGB:
        /* Same rate as YCbCr 4:4:4 so no modification */
        break;
    default:
        BDBG_ERR(("Color Component %d unsupported",eColorComponent));
        break;
    }

    for (iLookup = 0; iLookup < BVDC_P_RM_LU_ENTRIES; ++iLookup)
    {
        if((s_HdmiLU[iLookup].ulPixelBaseRate      == ulPixelBaseRate) &&
           (s_HdmiLU[iLookup].eHdmiColorDepth      == eHdmiColorDepth) &&
           (s_HdmiLU[iLookup].eHdmiPixelRepetition ==        ePxlRep))
        {
            iClock = s_HdmiLU[iLookup].eTmdsClock;
            break;
        }
    }
    if (iLookup == BVDC_P_RM_LU_ENTRIES)
    {
        goto Err;
    }

#if BFMT_DO_PICK
    for (iRm = 0 ; iRm < BVDC_P_RM_TABLE_ENTRIES ; ++iRm)
    {
        if (s_HdmiRm[iRm].eTmdsClock == iClock) break;
    }
#else
    iRm = iClock;
    BDBG_ASSERT (s_HdmiRm[iRm].eTmdsClock == iRm);
#endif
    BDBG_ASSERT (iRm < BVDC_P_RM_TABLE_ENTRIES);
    pRateInfo = &s_HdmiRm[iRm];

    /* Success */
    *ppRateInfo = pRateInfo;
    return bModified;

Err:
    BDBG_ERR(("PxlClkRate " BDBG_UINT64_FMT ", HDMI Color Depth %d, Pxl Repetition %d, Color Component %d unsupported",
        BDBG_UINT64_ARG(ulPixelClkRate), eHdmiColorDepth, eHdmiPixelRepetition, eColorComponent));
    *ppRateInfo = NULL;
    bModified = false;  /* Arbitrary */
    return bModified;
}


/* H, then V */
const uint32_t* BVDC_P_GetDviDtgToggles_isr
(
    const BVDC_P_DisplayInfo *pDispInfo
)
{
    static const uint32_t s_zeroes[2] = {0, 0};
    static const uint32_t s_ones[2] = {1, 1};
    static const uint32_t s_mixed[2] = {1, 0};

    const uint32_t* retval;
    BFMT_VideoFmt eFmt = pDispInfo->pFmtInfo->eVideoFmt;

    /* A rule for NTSC, PAL, and SECAM */
    if (BFMT_IS_525_LINES(eFmt) || BFMT_IS_625_LINES(eFmt) ||
        (BFMT_VideoFmt_e480p == eFmt) || (BFMT_VideoFmt_e576p_50Hz == eFmt) ||
        (BFMT_VideoFmt_e720x483p == eFmt))
    {
        retval = s_ones;
    }
    else
    {
        /* Individual rules for some VESA formats */
        switch (eFmt)
        {
        case BFMT_VideoFmt_eDVI_1024x768p:
        case BFMT_VideoFmt_eDVI_640x480p_CVT:
        case BFMT_VideoFmt_eDVI_640x480p:
            retval = s_ones;
            break;
        case BFMT_VideoFmt_eDVI_1280x720p:
        case BFMT_VideoFmt_eDVI_1280x720p_Red:
        case BFMT_VideoFmt_eDVI_1440x900p_60Hz:
            retval = s_mixed;
            break;
        /* Everything else */
        default:
            retval = s_zeroes;
            break;
        }
    }

    return retval;
}

/***************************************************************************
 *
 * Utility function called by BVDC_Display_GetCapabilities
 */
bool  BVDC_P_IsVidfmtSupported
    ( BFMT_VideoFmt                    eVideoFmt)
{
    const BVDC_P_FormatData *pFormatData;

    BDBG_ASSERT(eVideoFmt < BFMT_VideoFmt_eMaxCount);

    /* Support 4k ? */
    if(BFMT_IS_4kx2k(eVideoFmt))
    {
#ifndef BCHP_SCL_0_HW_CONFIGURATION_LINE_STORE_DEPTH_LS_DEPTH_4096
        return false;
#else
        if(BCHP_SCL_0_HW_CONFIGURATION_LINE_STORE_DEPTH_DEFAULT !=
           BCHP_SCL_0_HW_CONFIGURATION_LINE_STORE_DEPTH_LS_DEPTH_4096)
        {
            return false;
        }
#endif
    }

    /* Support 4k@60hz? */
    if((BFMT_IS_4kx2k_50_60HZ(eVideoFmt)) &&
       (0 == BVDC_P_SUPPORT_4kx2k_60HZ))
    {
        return false;
    }

    /* No ucodes */
    pFormatData = BVDC_P_GetFormatData_isrsafe(eVideoFmt);
    if((!pFormatData->pRamBVBInput) &&
       (!pFormatData->pDtRamBVBInput))
    {
        return false;
    }

    return true;

}


#if !B_REFSW_MINIMAL /** { **/
/*************************************************************************
 *
 *
 */
#if BDBG_DEBUG_BUILD
#define BVDC_P_DISPLAY_DUMP
#endif
#ifdef BVDC_P_DISPLAY_DUMP /** { **/

#if BVDC_P_NUM_SHARED_VF
void BVDC_P_Display_Dump_aulVfTable (const char* name, const uint32_t* table)
{
    BKNI_Printf ("//%s %d\n", name, BVDC_P_VF_TABLE_SIZE+1);

    BKNI_Printf ("//%s %08x\n", "FORMAT_ADDER",       table[0]);
    BKNI_Printf ("//%s %08x\n", "VF_MISC",            table[1]);
    BKNI_Printf ("//%s %08x\n", "VF_NEG_SYNC_VALUES", table[2]);
    BKNI_Printf ("//%s %08x\n", "VF_POS_SYNC_VALUES", table[3]);
    BKNI_Printf ("//%s %08x\n", "VF_SYNC_TRANS_0",    table[4]);
    BKNI_Printf ("//%s %08x\n", "VF_SYNC_TRANS_1",    table[5]);
    BKNI_Printf ("//%s %08x\n", "VF_NEG_SYNC_EXTN",   table[6]);
}

void BVDC_Display_DumpAll_aulVfTable (void)
{
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_NTSC_ITU", s_aulVfTable_NTSC_ITU);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_PAL", s_aulVfTable_PAL);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_PAL_I", s_aulVfTable_PAL_I);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_PAL_N", s_aulVfTable_PAL_N);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_PAL_NC", s_aulVfTable_PAL_NC);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_480i", s_aulVfTable_480i);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_576i", s_aulVfTable_576i);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_1080i", s_aulVfTable_1080i);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_480p", s_aulVfTable_480p);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_576p", s_aulVfTable_576p);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_576p54", s_aulVfTable_576p54);
    BVDC_P_Display_Dump_aulVfTable (
        "s_aulVfTable_480p54", s_aulVfTable_480p54);
}

void BVDC_P_Display_Dump_aulChFilterTbl (
    const char* name, const uint32_t* table)
{
    BKNI_Printf ("//%s %d\n", name, BVDC_P_CHROMA_TABLE_SIZE);

    BKNI_Printf ("//%s %08x\n", "PRIM_VF_CH2_TAP1_3", table[0]);
    BKNI_Printf ("//%s %08x\n", "PRIM_VF_CH2_TAP4_5", table[1]);
    BKNI_Printf ("//%s %08x\n", "PRIM_VF_CH2_TAP6_7", table[2]);
    BKNI_Printf ("//%s %08x\n", "PRIM_VF_CH2_TAP8_9", table[3]);
    BKNI_Printf ("//%s %08x\n", "PRIM_VF_CH2_TAP10",  table[4]);
}

void BVDC_Display_DumpAll_aulChFilterTbl (void)
{
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_AllPass", s_aulChFilterTbl_AllPass);
#if 0
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Cr_06Mhz", s_aulChFilterTbl_Cr_06Mhz);
#endif
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Cr_13Mhz", s_aulChFilterTbl_Cr_13Mhz);
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Y_60Mhz", s_aulChFilterTbl_Y_60Mhz);
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Y_675Mhz", s_aulChFilterTbl_Y_675Mhz);
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Cr_675Mhz", s_aulChFilterTbl_Cr_675Mhz);
#if 0
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Y_42Mhz", s_aulChFilterTbl_Y_42Mhz);
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Y_55Mhz", s_aulChFilterTbl_Y_55Mhz);
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Cr_1856Mhz", s_aulChFilterTbl_Cr_1856Mhz);
#endif
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Y_12Mhz", s_aulChFilterTbl_Y_12Mhz);
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Cr_SD", s_aulChFilterTbl_Cr_SD);
    BVDC_P_Display_Dump_aulChFilterTbl (
        "s_aulChFilterTbl_Cr_SECAM", s_aulChFilterTbl_Cr_SECAM);
}
#endif

void BVDC_P_Display_Dump_aulRmTable (const char* name, const uint32_t* table)
{
    BKNI_Printf ("//%s %d\n", name, BVDC_P_RM_TABLE_SIZE);

    BKNI_Printf ("//%s %08x\n", "PRIM_RM_RATE_RATIO", table[0]);
    BKNI_Printf ("//%s %08x\n", "PRIM_RM_PHASE_INC",  table[1]);
    BKNI_Printf ("//%s %08x\n", "PRIM_RM_INTEGRATOR", table[2]);
}

void BVDC_Display_DumpAll_aulRmTable (void)
{
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_25_2", s_aulRmTable_25_2);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_25_2_Div_1001", s_aulRmTable_25_2_Div_1001);
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_27", s_aulRmTable_27);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_27_Mul_1001", s_aulRmTable_27_Mul_1001);
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_39_79", s_aulRmTable_39_79);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_39_79_Div_1001", s_aulRmTable_39_79_Div_1001);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_60_4656", s_aulRmTable_60_4656);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_64_0224", s_aulRmTable_64_0224);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_64_0224_Div_1001", s_aulRmTable_64_0224_Div_1001);
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_65", s_aulRmTable_65);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_65_Div_1001", s_aulRmTable_65_Div_1001);
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_65_286", s_aulRmTable_65_286);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_65_286_Div_1001", s_aulRmTable_65_286_Div_1001);
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_74_25", s_aulRmTable_74_25);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_74_25_Div_1001", s_aulRmTable_74_25_Div_1001);
#if 0
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_74_48", s_aulRmTable_74_48);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_74_48_Div_1001", s_aulRmTable_74_48_Div_1001);
#endif
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_148_5", s_aulRmTable_148_5);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_148_5_Div_1001", s_aulRmTable_148_5_Div_1001);
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_162", s_aulRmTable_162);
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_67_565", s_aulRmTable_67_565);
    BVDC_P_Display_Dump_aulRmTable (
        "s_aulRmTable_67_565_Div_1001", s_aulRmTable_67_565_Div_1001);
    BVDC_P_Display_Dump_aulRmTable ("s_aulRmTable_56_304", s_aulRmTable_56_304);
}

void BVDC_P_Display_Dump_aulItTable (const char* name, const uint32_t* table)
{
    BKNI_Printf ("//%s %d\n", name, BVDC_P_IT_TABLE_SIZE);

    BKNI_Printf ("//%s %08x\n", "PRIM_IT_ADDR_0_3",        table[0]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_ADDR_4_6",        table[1]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_STACK_reg_0_1",   table[2]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_STACK_reg_2_3",   table[3]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_STACK_reg_4_5",   table[4]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_STACK_reg_6_7",   table[5]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_EVENT_SELECTION", table[6]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_PCL_0",           table[7]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_PCL_1",           table[8]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_PCL_2",           table[9]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_PCL_3",           table[10]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_PCL_4",           table[11]);
#if (BVDC_P_SUPPORT_IT_VER >= 1)
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_PCL_5",           table[12]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_PCL_6",           table[13]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_PCL_7",           table[14]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_PCL_8",           table[15]);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_STACK_reg_8_9",   table[16]);
#endif
}

void BVDC_Display_DumpAll_aulItTable (void)
{
    BVDC_P_Display_Dump_aulItTable (
        "s_aulItTable_1080p_60hz", s_aulItTable_1080p_60hz);
    BVDC_P_Display_Dump_aulItTable ("s_aulItTable_1080i", s_aulItTable_1080i);
    BVDC_P_Display_Dump_aulItTable ("s_aulItTable_720p", s_aulItTable_720p);
    BVDC_P_Display_Dump_aulItTable (
        "s_aulItTable_720p_24hz", s_aulItTable_720p_24hz);
    BVDC_P_Display_Dump_aulItTable (
        "s_aulItTable_720p_50hz", s_aulItTable_720p_50hz);
    BVDC_P_Display_Dump_aulItTable ("s_aulItTable_480p", s_aulItTable_480p);
    BVDC_P_Display_Dump_aulItTable ("s_aulItTable_480p54", s_aulItTable_480p54);
    BVDC_P_Display_Dump_aulItTable (
        "s_aulItTable_1280x720p", s_aulItTable_1280x720p);
    BVDC_P_Display_Dump_aulItTable (
        "s_aulItTable_1600x1200p_60Hz", s_aulItTable_1600x1200p_60Hz);
    BVDC_P_Display_Dump_aulItTable (
        "s_aulItTable_CUSTOM_1366x768p", s_aulItTable_CUSTOM_1366x768p);
}

void BVDC_P_Display_Dump_ulItConfig (const char* name, uint32_t value)
{
    BKNI_Printf ("//%s %d\n", name, 1);
    BKNI_Printf ("//%s %08x\n", "PRIM_IT_TG_CONFIG", value);
}

void BVDC_Display_DumpAll_ulItConfig (void)
{
    BVDC_P_Display_Dump_ulItConfig ("s_aulItConfig_480i",
        s_aulItConfig_480i[0]);
    BVDC_P_Display_Dump_ulItConfig ("s_aulItConfig_PAL_M",
        s_aulItConfig_PAL_M[0]);
    BVDC_P_Display_Dump_ulItConfig ("s_aulItConfig_576i",
        s_aulItConfig_576i[0]);
    BVDC_P_Display_Dump_ulItConfig ("s_aulItConfig_SECAM",
        s_aulItConfig_SECAM[0]);
    BVDC_P_Display_Dump_ulItConfig (
        "s_aulItConfig_1080p_60hz", s_aulItConfig_1080p_60hz[0]);
    BVDC_P_Display_Dump_ulItConfig (
        "s_aulItConfig_1080i", s_aulItConfig_1080i[0]);
    BVDC_P_Display_Dump_ulItConfig (
        "s_aulItConfig_1080i_50hz", s_aulItConfig_1080i_50hz[0]);
    BVDC_P_Display_Dump_ulItConfig (
        "s_aulItConfig_720p", s_aulItConfig_720p[0]);
    BVDC_P_Display_Dump_ulItConfig ("s_aulItConfig_480p",
        s_aulItConfig_480p[0]);
    BVDC_P_Display_Dump_ulItConfig (
        "s_ulItConfig_CUSTOM_1366x768p", s_ulItConfig_CUSTOM_1366x768p);
}

#if BVDC_P_NUM_SHARED_SM
void BVDC_P_Display_Dump_aulSmTable (const char* name, const uint32_t* table)
{
    BKNI_Printf ("//%s %d\n", name, BVDC_P_SM_TABLE_SIZE);

    BKNI_Printf ("//%s %08x\n", "PRIM_SM_PG_CNTRL",           *table++);
    BKNI_Printf ("//%s %08x\n", "PRIM_SM_PG_CONFIG",          *table++);
    BKNI_Printf ("//%s %08x\n", "PRIM_SM_SC_FREQ_0",          *table++);
    BKNI_Printf ("//%s %08x\n", "PRIM_SM_COMP_CNTRL",         *table++);
}

void BVDC_Display_DumpAll_aulSmTable (void)
{
    BVDC_P_Display_Dump_aulSmTable ("s_aulSmTable_YQI", s_aulSmTable_YQI);
    BVDC_P_Display_Dump_aulSmTable (
        "s_aulSmTable_PAL_YUV", s_aulSmTable_PAL_YUV);
    BVDC_P_Display_Dump_aulSmTable (
        "s_aulSmTable_PALN_YUV", s_aulSmTable_PALN_YUV);
    BVDC_P_Display_Dump_aulSmTable (
        "s_aulSmTable_PALNC_YUV", s_aulSmTable_PALNC_YUV);
    BVDC_P_Display_Dump_aulSmTable (
        "s_aulSmTable_PALM_YUV", s_aulSmTable_PALM_YUV);
    BVDC_P_Display_Dump_aulSmTable ("s_aulSmTable_SECAM", s_aulSmTable_SECAM);
    BVDC_P_Display_Dump_aulSmTable (
        "s_aulSmTable_Component", s_aulSmTable_Component);
}
#endif

void BVDC_Display_DumpTables (void)
{
#if BVDC_P_NUM_SHARED_VF
    BVDC_Display_DumpAll_aulVfTable();
    BVDC_Display_DumpAll_aulChFilterTbl();
#endif
    BVDC_Display_DumpAll_aulRmTable();
    BVDC_Display_DumpAll_aulItTable();
#if BVDC_P_NUM_SHARED_SM
    BVDC_Display_DumpAll_aulSmTable();
#endif
}
#endif /** } BVDC_P_DISPLAY_DUMP **/
#endif /** } !B_REFSW_MINIMAL **/

/* End of File */
