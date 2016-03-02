/***************************************************************************
 *
 *     Copyright (c) 2003-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *   Contains tables for Display settings.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bvdc_displayfmt_priv.h"
#include "bchp_misc.h"
#include "bchp_scl_0.h"

BDBG_MODULE(BVDC_DISP);

#ifndef BVDC_FOR_BOOTUPDATER
#include "bvdc_displayfmt_macro.h"
#else
#include "bvdc_displayfmt_macro_bootupdater.h"
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

#include "bvdc_automation_priv.c"

/****************************************************************
 *  Tables
 ****************************************************************/

static const uint32_t* const
	s_aulDtRamBVBInput_DVI_480i_DropTbl[BVDC_P_480i_DROP_TABLE_SIZE] =
{
	s_aulDviMicrocode_NTSC_J,
	BVDC_P_MAKE_DROPTBL(s_aulDtRamBVBInput_DVI_480i_Drop1)
};

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

#if BVDC_P_ORTHOGONAL_VEC /** { **/

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

#if BVDC_P_ORTHOGONAL_VEC
#else
/* Cr 3.375 Mhz/13.5 Mhz */
static const uint32_t s_aulChFilterTbl_Cr_3375Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0X000000AC, 0x0000006F, 0x00000569, 0x000001E5, 0x00008741,
	0x00000759, 0x0004DD7D, 0x00009066, 0x000ADB23, 0x000F05FF,
};
#endif

#if 0
/* Y 4.2 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_42Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0X00000000, 0X00000000, 0X00000000, 0X00000003, 0X000005BE,
	0X00004C2C, 0x00000747, 0X0005FF95, 0X0008EF0E, 0X000FFF05,
};
#endif

#if BVDC_P_ORTHOGONAL_VEC
#else
/* Y 5.0 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_50Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0x00000001, 0x0000000F, 0x000005BF, 0x000008A1, 0x00004ED0,
	0x000003F4, 0x0000A438, 0x00049D08, 0x0008C8FF, 0x00A5F5AF,
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
	0x000000F5, 0X00000000, 0x00005FFF, 0X00000000, 0x0000A50F,
	0X00000000, 0x0005FF5F, 0X00000000, 0x000AA005, 0x000F0000,
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

#if BVDC_P_ORTHOGONAL_VEC
#else
/* Cr 3.375 Mhz/13.5 Mhz */
static const uint32_t s_aulChFilterTbl_Cr_3375Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0X000000AC, 0x0000006F, 0x00000569, 0x000001E5, 0x00008741,
	0x00000759, 0x0004DD7D, 0x00009066, 0x000ADB23, 0x000F05FF,
};
#endif

#if 0
/* Y 4.2 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_42Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0X00000000, 0X00000000, 0X00000000, 0X00000003, 0X000005BE,
	0X00004C2C, 0x00000747, 0X0005FF95, 0X0008EF0E, 0X000FFF05,
};
#endif

#if BVDC_P_ORTHOGONAL_VEC
#else
/* Y 5.0 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_50Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0x00000001, 0x0000000F, 0x000005BF, 0x000008A1, 0x00004ED0,
	0x000003F4, 0x0000A438, 0x00049D08, 0x0008C8FF, 0x00A5F5AF,
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
	0x000000f5, 0x00000000, 0x00005fff, 0x00000000, 0x0000a50f,
	0X00000000, 0x0005ff5f, 0X00000000, 0x000aa005, 0x000f0000,
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

#else /** } { modular VEC **/

/* All-pass */
static const uint32_t s_aulChFilterTbl_AllPass[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00030000,
};

#if 0
/* Cr 0.6 MHz/13.5 MHz */
static const uint32_t s_aulChFilterTbl_Cr_06Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0x000E6A1F, 0x00A590FC, 0x00AA9A09, 0x0AF00AE7, 0x00000AFF,
};
#endif

/* Cr 1.3 Mhz/13.5 Mhz */
static const uint32_t s_aulChFilterTbl_Cr_13Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0x000040A8, 0x005F2056, 0x0082317F, 0xA1388756, 0x0000A0AF,
};

#if BVDC_P_ORTHOGONAL_VEC
#else
/* Cr 3.375 Mhz/13.5 Mhz */ /* Changed by rpan */
static const uint32_t s_aulChFilterTbl_Cr_3375Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
/*	0x0005003C, 0x000FC01F, 0x04DD34EF, 0xAD3F0903, 0x0000F05F,*/
	0x000060A0, 0x000C1055, 0x04DB82D2, 0xE5481347, 0x0000FAAA,
};
#endif

#if BVDC_P_ORTHOGONAL_VEC
#else
/* Y 5.0 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_50Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0x0005F0A0, 0x004ED08A, 0x00A4312F, 0x8C8F49D0, 0x000A505A,
};
#endif

/* Y 6.0 Mhz/13.5Mhz */
static const uint32_t s_aulChFilterTbl_Y_60Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0x000A05F3, 0x008604D0, 0x008CC45F, 0x9BC348BC, 0x000A5AF0,
};

/* Cr 6.75 Mhz/27Mhz */
static const uint32_t s_aulChFilterTbl_Cr_675Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
	0x00132003, 0X00203000, 0x01220000, 0X33010000, 0x00020000,
};

/* Y 6.75 Mhz/13.5Mhz */ /* Changed by rpan */
static const uint32_t s_aulChFilterTbl_Y_675Mhz[BVDC_P_CHROMA_TABLE_SIZE] =
{
/*	0x00086058, 0x000CA045, 0x0089C40E, 0x9B934E54, 0x000A5F55,*/
	0x000854C8, 0x000D0047, 0x009A24A7, 0x9A354E24, 0x000A5FA5,
};

#endif /** } orthogonal or modular VEC **/

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

#if BVDC_P_SUPPORT_MHL
static const BVDC_P_PxlFreqMhlFreqPair s_PxlFreqToMhlPxlFreq[] =
{
	{BFMT_PXL_25_2MHz,            BFMT_PXL_75_5_MHz},
	{BFMT_PXL_25_2MHz_DIV_1_001,  BFMT_PXL_75_5_MHz_DIV_1_001},
	{BFMT_PXL_27MHz,              BFMT_PXL_81_0MHz},
	{BFMT_PXL_27MHz_MUL_1_001,    BFMT_PXL_81_0MHz_MUL_1_001},
	{BFMT_PXL_74_25MHz,           BFMT_PXL_222_75MHz},
	{BFMT_PXL_74_25MHz_DIV_1_001, BFMT_PXL_222_75MHz_DIV_1_001},
	{BFMT_PXL_148_5MHz,           BFMT_PXL_297MHz},
	{BFMT_PXL_148_5MHz_DIV_1_001, BFMT_PXL_297MHz_DIV_1_001}
};

/* Number of entries in the above table! */
#define BVDC_P_MHL_FREQ_TABLE_ENTRIES \
	(sizeof(s_PxlFreqToMhlPxlFreq) / sizeof(BVDC_P_PxlFreqMhlFreqPair))
#endif

/* HDMI Rate Manager */
static const BVDC_P_RateInfo s_HdmiRm[] =
{
#if (BVDC_P_SUPPORT_HDMI_RM_VER <= BVDC_P_HDMI_RM_VER_1)
	/* ulPixelClkRate;
	   ulMDiv;
	   ulNDiv; (Offset: 9.22 format)
	   ulRDiv;
	   ulSampleInc;
	   ulNumerator;
	   ulDenominator;
	   ulVcoRange;
	   ulLinkDivCtrl;
	   ulP2; ( pll feedback pre-divider)
	   pchRate;
	*/
	BVDC_P_MAKE_HDMIRM(25_2MHz,             5, 148,        210, 3,    87,   400,      1,      1, 1, BDBG_STRING("25.20"))     /* 25.200000 MHz */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001,   5, 148,        210, 3,     3,    14,      1,      1, 1, BDBG_STRING("25.17"))     /* 25.174825 MHz */

	BVDC_P_MAKE_HDMIRM(27MHz,               4, 0xE000000,  112, 4,   0,     1,        0,      0, 1, BDBG_STRING("27.00") )     /* 480p @ 60Hz  */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,     4, 0x0E039581, 112, 3,   997, 1001,       0,      0, 1, BDBG_STRING("27.02") )     /* 480p @ 60Hz  */

	BVDC_P_MAKE_HDMIRM(74_25MHz,            2, 0x13400000, 112, 2,    10,    11,      1,      1, 1, BDBG_STRING("74.25"))     /* 74.250000 MHz */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001,  2, 0x133B13B0, 112, 2,   114,   125,      1,      1, 1, BDBG_STRING("74.17"))     /* 74.175824 MHz */

	BVDC_P_MAKE_HDMIRM(148_5MHz,            1, 0x13400000, 112, 2,    10,    11,      1,      0, 1, BDBG_STRING("148.5"))     /* 148.50000 MHz */
	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001,  1, 0x133B13B0, 112, 2,   114,   125,      1,      0, 1, BDBG_STRING("148.3"))     /* 148.35000 MHz */

	BVDC_P_MAKE_HDMIRM(162MHz,              1, 190,        210, 2,     1,     2,      1,      1, 1, BDBG_STRING("162.0"))       /* 162.00000 MHz */

	BVDC_P_MAKE_HDMIRM(56_304MHz,           2,  98,        210, 3,   933,  1564,      0,      1, 1, BDBG_STRING("56.30"))       /* 56.304000 MHz 1440x782 @ 50Hz */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001, 2, 157,        210, 3,     3, 25024,      1,      1, 1, BDBG_STRING("67.49"))       /* 67.497303 MHz 1440x782 @ 59.94Hz*/
	BVDC_P_MAKE_HDMIRM(67_565MHz,           2, 154,        210, 2,  3119,  3128,      1,      1, 1, BDBG_STRING("67.56"))       /* 67.564800 MHz 1440x782 @ 60Hz */

	/* 60/50 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz,            3, 109,        210, 3,  5427, 13816,      1,      1, 1, BDBG_STRING("39.79"))       /* 39.790080 MHz */
	BVDC_P_MAKE_HDMIRM(65MHz,               2, 144,        210, 3,  5217, 45136,      1,      1, 1, BDBG_STRING("65.00"))       /* 64.995840 MHz */
	BVDC_P_MAKE_HDMIRM(65_286MHz,           2, 145,        210, 3,    41,   403,      1,      1, 1, BDBG_STRING("65.28"))       /* 65.286000 MHz */
	BVDC_P_MAKE_HDMIRM(60_4656MHz,          2, 107,        210, 3,  2931,  8398,      1,      1, 1, BDBG_STRING("60.46"))       /* 60.465600 MHz */
	BVDC_P_MAKE_HDMIRM(64_022MHz,           2, 146,        210, 3,   161,   988,      1,      1, 1, BDBG_STRING("64.02"))       /* 64.022400 MHz */

	/* 59.94 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001,  3, 109,        210, 3,  3981, 10048,      1,      1, 1, BDBG_STRING("39.79/1.001")) /* 39.750329 MHz */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001,     2, 144,        210, 3,   471,  3968,      1,      1, 1, BDBG_STRING("65.00/1.001")) /* 64.930909 MHz */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001, 2, 145,        210, 3,    13,   124,      1,      1, 1, BDBG_STRING("65.28/1.001")) /* 65.220779 MHz */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001, 2, 146,        210, 3,   101,   608,      1,      1, 1, BDBG_STRING("64.02/1.001")) /* 63.958441 MHz */

#ifdef LATER
	/* Used by certain LCD panels */
	BVDC_P_MAKE_HDMIRM(72_427MHz,           2, 0x12c70524, 112, 2, 14822, 15089,      1,      0, 2,  BDBG_STRING("72427")     ) /* 72427200 */
#endif /* LATER */

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_2)
	/* Mask                         Denom   Num Sample Offset shift rmdiv vco pxDiv Feed prediv */
	BVDC_P_MAKE_HDMIRM(25_2MHz,                  1,     0,  3, 0x5d55555,  1,  70,  0,   5,    1,   1, BDBG_STRING("25.20") ) /* 25.200000 MHz */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001,    10000,  9601,  2, 0x5d3d76c,  1,  69,  0,   5,    1,   1, BDBG_STRING("25.17") ) /* 25.174825 MHz */

	BVDC_P_MAKE_HDMIRM(27MHz,                    1,     0,  3, 0x6400000,  1,  75,  0,   5,    1,   1, BDBG_STRING("27.00") ) /* 27.000000 MHz  */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,       1001,   998,  2, 0x6419999,  1,  75,  0,   5,    1,   1, BDBG_STRING("27.02") ) /* 27.027000 MHz  */

	BVDC_P_MAKE_HDMIRM(67_565MHz,             3128,  3119,  2, 0xc83126e,  2,  75,  0,   2,    1,   1, BDBG_STRING("67.56") ) /* 67.564800 MHz 1440x782 @ 60Hz */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001,  12512, 12013,  2, 0xc7fdf43,  2,  74,  0,   2,    1,   1, BDBG_STRING("67.49") ) /* 67.497303 MHz 1440x782 @ 59.94Hz*/

	BVDC_P_MAKE_HDMIRM(74_25MHz,                55,    54,  2, 0x6e00000,  1,  82,  0,   2,    1,   1, BDBG_STRING("74.25") ) /* 74.250000 MHz */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001,     1250,  1231,  2, 0x6de3de3,  1,  82,  0,   2,    1,   1, BDBG_STRING("74.17") ) /* 74.175824 MHz */

	BVDC_P_MAKE_HDMIRM(148_5MHz,                 11,     9,  3, 0xdc00000,  2,  105, 0,   1,    1,   2, BDBG_STRING("148.5") ) /* 148.50000 MHz */
	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001,     1250,  1231,  2, 0xdbc7bc7,  2,  82,  0,   1,    1,   2, BDBG_STRING("148.35")) /* 148351648 MHz */

	/* 60/50 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz,                628,   619,  2, 0x586c226,  1,  66,  0,   3,    1,   1, BDBG_STRING("39.79") ) /* 39.790080 MHz */
	BVDC_P_MAKE_HDMIRM(65MHz,                   325,   322,  2, 0x604bda1,  1,  72,  0,   2,    1,   1, BDBG_STRING("65.00") ) /* 65.000000 MHz */
	BVDC_P_MAKE_HDMIRM(65_286MHz,               403,   394,  2, 0x60b851e,  1,  72,  0,   2,    1,   1, BDBG_STRING("65.28") ) /* 65.286000 MHz */
	BVDC_P_MAKE_HDMIRM(64_022MHz,              2964,  2947,  2, 0x5ed9168,  1,  71,  0,   2,    1,   1, BDBG_STRING("64.02") ) /* 64.022400 MHz */
	BVDC_P_MAKE_HDMIRM(83_5MHz,				   835,	  814,  2, 0x3dda12f,  0,  46,  0,   1,    1,   1, BDBG_STRING("83500") ) /* 83.500000 MHz */
	BVDC_P_MAKE_HDMIRM(85_5MHz,                  95,    92,  2, 0x3f55555,  0,  47,  0,   1,    1,   1, BDBG_STRING("85500") ) /* 85.500000 MHz */

	/* 59.94 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001,     5024,  4967,  2, 0x5855855,  1,  66,  0,   3,    1,   1, BDBG_STRING("39.70/1.001") ) /* 39.750329 MHz */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001,         496,   493,  2, 0x6031a60,  1,  72,  0,   2,    1,   1, BDBG_STRING("65.00/1.001") ) /* 64.930909 MHz */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001,     155,   152,  2, 0x609f959,  1,  72,  0,   2,    1,   1, BDBG_STRING("65.28/1.001") ) /* 65.220779 MHz */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001,    1824,  1819,  2, 0x5ec0d4c,  1,  71,  0,   2,    1,   1, BDBG_STRING("64.02/1.001") ) /* 63.958441 MHz */

	/* For (480P or 576P), 54 MHz pixel rate. */
	BVDC_P_MAKE_HDMIRM(54MHz,                     1,     0,  3, 0xa000000,  2,  60,  0,   2,    1,   2, BDBG_STRING("54.00")       ) /* 54.00 MHz  */
	BVDC_P_MAKE_HDMIRM(54MHz_MUL_1_001,        1001,   998,  2, 0xa028f5c,  2,  60,  0,   2,    1,   2, BDBG_STRING("54.00/1.001") ) /* 54.054 MHz */
	/* Mask                          Denom    Num Sample Offset shift rmdiv vco pxDiv Feed prediv */

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_3)
	/* Mask                      Denom   Num Sample  Offset shift rmdiv vco pxDiv Feed prediv */
	BVDC_P_MAKE_HDMIRM(25_2MHz,               1,    0,  3, 0xbaaaaaa,  2,  70,    0,   5,   1,    1, BDBG_STRING("25.20") ) /* 25.200000 MHz */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001, 10000, 9601,  2, 0xba7aed8,  2,  69,    0,   5,   1,    1, BDBG_STRING("25.20") ) /* 25.174825 MHz */

	BVDC_P_MAKE_HDMIRM(27MHz,                 1,    0,  3, 0xc800000,  2,  75,    0,   5,    1,   1, BDBG_STRING("27.00") ) /* 27.000000 MHz */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,    1001,  998,  2, 0xc833333,  2,  75,    0,   5,    1,   1, BDBG_STRING("27.02") ) /* 27.027000 MHz */

	BVDC_P_MAKE_HDMIRM(74_25MHz,             55,   54,  2, 0xdc00000,  2,   82,   0,   2,    1,   1, BDBG_STRING("74.25") ) /* 74.250000 MHz */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001, 1250, 1231,  2, 0xdbc7bc7,  2,   82,   0,   2,    1,   1, BDBG_STRING("74.17") ) /* 74.175824 MHz */

	BVDC_P_MAKE_HDMIRM(148_5MHz,           11,     9,  3, 0xdc00000,  2,  105,    0,   1,    1,   1, BDBG_STRING("148.5")) /* 148.50000 MHz */

	/* 60/50 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz,              628,   619,  2, 0xb0d844d,  2,  66,  0,   3,    1,   1, BDBG_STRING("39.79") ) /* 39.790080 MHz */
	BVDC_P_MAKE_HDMIRM(65MHz,                 325,   322,  2, 0xc097b42,  2,  72,  0,   2,    1,   1, BDBG_STRING("65.00") ) /* 65.000000 MHz */
	BVDC_P_MAKE_HDMIRM(65_286MHz,             403,   394,  2, 0xc170a3d,  2,  72,  0,   2,    1,   1, BDBG_STRING("65.28") ) /* 65.286000 MHz */
	BVDC_P_MAKE_HDMIRM(64_022MHz,            2964,  2947,  2, 0xbdb22d0,  2,  71,  0,   2,    1,   1, BDBG_STRING("64.02") ) /* 64.022400 MHz */

	/* 59.94 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001,   5024,  4967,  2, 0xb0ab0ab,  2,  66,  0,   3,    1,   1, BDBG_STRING("39.79/1.001") ) /* 39.750329 MHz */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001,       496,   493,  2, 0xc0634c0,  2,  72,  0,   2,    1,   1, BDBG_STRING("65.00/1.001") ) /* 64.930909 MHz */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001,   155,   152,  2, 0xc13f2b3,  2,  72,  0,   2,    1,   1, BDBG_STRING("65.28/1.001") ) /* 65.220779 MHz */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001,  1824,  1819,  2, 0xbd81a98,  2,  71,  0,   2,    1,   1, BDBG_STRING("64.02/1.001") ) /* 63.958441 MHz */

	/* For (480P or 576P), 54 MHz pixel rate. */
	BVDC_P_MAKE_HDMIRM(54MHz,                   1,     0,  3, 0xa000000,  2,  60,  0,   2,    1,   1, BDBG_STRING("54.00")       ) /* 54.00 MHz  */
	BVDC_P_MAKE_HDMIRM(54MHz_MUL_1_001,      1001,   998,  2, 0xa028f5c,  2,  60,  0,   2,    1,   1, BDBG_STRING("54.00/1.001") ) /* 54.054 MHz */

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_4)
/* Parameters generated 01/16/2014 at time 10:51:48 AM                       */
/* Note: The parameters are intended for 65nm silicon.                       */
/*       The PLL reference frequency used to generate                        */
/*       this table is  54.000Mhz.                                           */
/*       The PLL reference frequency is not doubled (which is good          */
/*       since that is not an available feature in 65nm)                    */
/*                                                                           */
/* The following fields need to be programmed for each case:                 */
/*   Mask                                                                    */
/*   Color Depth                                                             */
/*   Pixel Repetition,                                                       */
/*   DEN          HDMI_RM.RATE_RATIO.DENOMINATOR                             */
/*   NUM          HDMI_RM.SAMPLE_INC.NUMERATOR                               */
/*   SampleInc    HDMI_RM.SAMPLE_INC.SAMPLE_INC                              */
/*   OFFSET,      HDMI_RM.OFFSET.OFFSET                                      */
/*   SHIFT,       HDMI_RM.FORMAT.SHIFT                                       */
/*   CkDivRM,     HDMI_TX_PHY.HDMI_TX_PHY_PLL_CFG.PLL_RM_DIV                 */
/*   VCO_Rng      HDMI_TX_PHY.HDMI_TX_PHY_PLL_CFG.PLL_VCO_RANGE              */
/*   CkDivVCO,    HDMI_TX_PHY.HDMI_TX_PHY_PLL_CFG.PLL_PX_DIV                 */
/*   PreDiv       HDMI_TX_PHY.HDMI_TX_PHY_PLL_CFG.PLL_FEEDBACK_PRE_DIVIDER   */
/*                (Always set to 1.  This is the predivider before CkDivRM)  */
/*   PDIV,        HDMI_TX_PHY.HDMI_TX_PHY_PLL_CFG.PLL_INPUT_PRE_DIVIDER      */
/*   KVcoXS       HDMI_TX_PHY.HDMI_TX_PHY_CTL_2.KVCO_XS                      */
/*   TMDS_Freq    Text string with TMDS Character rate in kHz.               */
	/* Mask                          ColorDepth                     PixelRep                         ColorComp,                 Denom    Num     Sample  Offset      shift    rmdiv      vco     pxDiv    Feed prediv  KVcoXS  pchRate */
	BVDC_P_MAKE_HDMIRM(25_2MHz              , e24bit, eNone, eYCbCr444, 32760, 32292,         0, 0x0BAAAAAA,     2,      23,       0,        5,      1,    2,       4, BDBG_STRING("25200.000")) /* TMDS Clock =    25.200000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.200000000, fRM =  27.391, fVCO = ( 1260.000000000,  1260.000000000,  1259.999995708) */
	BVDC_P_MAKE_HDMIRM(25_2MHz              , e30bit, eNone, eYCbCr444, 32760, 32292,         0, 0x0BAAAAAA,     2,      23,       0,        4,      1,    2,       4, BDBG_STRING("31500.000")) /* TMDS Clock =    31.500000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.200000000, fRM =  27.391, fVCO = ( 1260.000000000,  1260.000000000,  1259.999995708) */
	BVDC_P_MAKE_HDMIRM(25_2MHz              , e36bit, eNone, eYCbCr444, 32767,     0,         1, 0x0A800000,     2,      21,       0,        3,      1,    2,       4, BDBG_STRING("37800.000")) /* TMDS Clock =    37.800000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.200000000, fRM =  27.000, fVCO = ( 1134.000000000,  1134.000000000,  1134.000000000) */

	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001    , e24bit, eNone, eYCbCr444, 30000, 29601,         0, 0x0BA7AED8,     2,      23,       0,        5,      1,    2,       4, BDBG_STRING("25174.825")) /* TMDS Clock =    25.174825175, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.174825175, fRM =  27.419, fVCO = ( 1258.741258741,  1258.741258741,  1258.741258621) */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001    , e30bit, eNone, eYCbCr444, 30000, 29601,         0, 0x0BA7AED8,     2,      23,       0,        4,      1,    2,       4, BDBG_STRING("31468.531")) /* TMDS Clock =    31.468531469, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.174825175, fRM =  27.419, fVCO = ( 1258.741258741,  1258.741258741,  1258.741258621) */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001    , e36bit, eNone, eYCbCr444, 32000,    32,         1, 0x0A7D508F,     2,      21,       0,        3,      1,    2,       4, BDBG_STRING("37762.238")) /* TMDS Clock =    37.762237762, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.174825175, fRM =  27.027, fVCO = ( 1132.867132867,  1132.867132867,  1132.867131472) */

	BVDC_P_MAKE_HDMIRM(27MHz                , e24bit, eNone, eYCbCr444, 32760, 31122,         0, 0x0A000000,     2,      19,       0,        4,      1,    2,       4, BDBG_STRING("27000.000")) /* TMDS Clock =    27.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.000000000, fRM =  28.421, fVCO = ( 1080.000000000,  1080.000000000,  1080.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e30bit, eNone, eYCbCr444, 32750, 31440,         0, 0x0C800000,     2,      24,       0,        4,      1,    2,       4, BDBG_STRING("33750.000")) /* TMDS Clock =    33.750000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.000000000, fRM =  28.125, fVCO = ( 1350.000000000,  1350.000000000,  1350.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e36bit, eNone, eYCbCr444, 32760, 32032,         0, 0x0B400000,     2,      22,       0,        3,      1,    2,       4, BDBG_STRING("40500.000")) /* TMDS Clock =    40.500000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.000000000, fRM =  27.614, fVCO = ( 1215.000000000,  1215.000000000,  1215.000000000) */

	BVDC_P_MAKE_HDMIRM(27MHz                , e24bit, e2x  , eYCbCr444, 32760, 31122,         0, 0x0A000000,     2,      19,       0,        2,      1,    2,       4, BDBG_STRING("54000.000")) /* TMDS Clock =    54.000000000, CD = 24, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  28.421, fVCO = ( 1080.000000000,  1080.000000000,  1080.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e30bit, e2x  , eYCbCr444, 32750, 31440,         0, 0x0C800000,     2,      24,       0,        2,      1,    2,       4, BDBG_STRING("67500.000")) /* TMDS Clock =    67.500000000, CD = 30, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  28.125, fVCO = ( 1350.000000000,  1350.000000000,  1350.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e36bit, e2x  , eYCbCr444, 32760, 30576,         0, 0x07800000,     1,      14,       0,        1,      1,    2,       4, BDBG_STRING("81000.000")) /* TMDS Clock =    81.000000000, CD = 36, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  28.929, fVCO = (  810.000000000,   810.000000000,   810.000000000) */

	BVDC_P_MAKE_HDMIRM(27MHz                , e24bit, e4x  , eYCbCr444, 32760, 31122,         0, 0x0A000000,     2,      19,       0,        1,      1,    2,       4, BDBG_STRING("108000.000")) /* TMDS Clock =   108.000000000, CD = 24, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  28.421, fVCO = ( 1080.000000000,  1080.000000000,  1080.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e30bit, e4x  , eYCbCr444, 32750, 31440,         0, 0x0C800000,     2,      24,       0,        1,      1,    2,       4, BDBG_STRING("135000.000")) /* TMDS Clock =   135.000000000, CD = 30, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  28.125, fVCO = ( 1350.000000000,  1350.000000000,  1350.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e36bit, e4x  , eYCbCr444, 32760, 31668,         0, 0x0F000000,     2,      29,       1,        1,      1,    2,       7, BDBG_STRING("162000.000")) /* TMDS Clock =   162.000000000, CD = 36, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.931, fVCO = ( 1620.000000000,  1620.000000000,  1620.000000000) */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e24bit, eNone, eYCbCr444, 32032, 30400,         0, 0x0A028F5C,     2,      19,       0,        4,      1,    2,       4, BDBG_STRING("27027.000")) /* TMDS Clock =    27.027000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.027000000, fRM =  28.393, fVCO = ( 1081.080000000,  1081.080000000,  1081.079998970) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e30bit, eNone, eYCbCr444, 32032, 30720,         0, 0x0C833333,     2,      24,       0,        4,      1,    2,       4, BDBG_STRING("33783.750")) /* TMDS Clock =    33.783750000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.027000000, fRM =  28.097, fVCO = ( 1351.350000000,  1351.350000000,  1351.349998713) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e36bit, eNone, eYCbCr444, 32760, 32000,         0, 0x0B42E147,     2,      22,       0,        3,      1,    2,       4, BDBG_STRING("40540.500")) /* TMDS Clock =    40.540500000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.027000000, fRM =  27.586, fVCO = ( 1216.215000000,  1216.215000000,  1216.214995623) */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e24bit, e2x  , eYCbCr444, 32032, 30400,         0, 0x0A028F5C,     2,      19,       0,        2,      1,    2,       4, BDBG_STRING("54054.000")) /* TMDS Clock =    54.054000000, CD = 24, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.054000000, fRM =  28.393, fVCO = ( 1081.080000000,  1081.080000000,  1081.079998970) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e30bit, e2x  , eYCbCr444, 32032, 30720,         0, 0x0C833333,     2,      24,       0,        2,      1,    2,       4, BDBG_STRING("67567.500")) /* TMDS Clock =    67.567500000, CD = 30, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.054000000, fRM =  28.097, fVCO = ( 1351.350000000,  1351.350000000,  1351.349998713) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e36bit, e2x  , eYCbCr444, 32604, 30400,         0, 0x0781EB85,     1,      14,       0,        1,      1,    2,       4, BDBG_STRING("81081.000")) /* TMDS Clock =    81.081000000, CD = 36, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.054000000, fRM =  28.900, fVCO = (  810.810000000,   810.810000000,   810.809999228) */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e24bit, e4x  , eYCbCr444, 32032, 30400,         0, 0x0A028F5C,     2,      19,       0,        1,      1,    2,       4, BDBG_STRING("108108.000")) /* TMDS Clock =   108.108000000, CD = 24, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.108000000, fRM =  28.393, fVCO = ( 1081.080000000,  1081.080000000,  1081.079998970) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e30bit, e4x  , eYCbCr444, 32032, 30720,         0, 0x0C833333,     2,      24,       0,        1,      1,    2,       4, BDBG_STRING("135135.000")) /* TMDS Clock =   135.135000000, CD = 30, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.108000000, fRM =  28.097, fVCO = ( 1351.350000000,  1351.350000000,  1351.349998713) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e36bit, e4x  , eYCbCr444, 30030, 29000,         0, 0x0F03D70A,     2,      29,       1,        1,      1,    2,       7, BDBG_STRING("162162.000")) /* TMDS Clock =   162.162000000, CD = 36, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.108000000, fRM =  27.903, fVCO = ( 1621.620000000,  1621.620000000,  1621.619998455) */

	BVDC_P_MAKE_HDMIRM(67_565MHz            , e24bit, eNone, eYCbCr444, 31280, 31250,         0, 0x0C83126E,     2,      25,       0,        2,      1,    2,       4,  BDBG_STRING("67564.800")) /* TMDS Clock =    67.564800000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.564800000, fRM =  27.026, fVCO = ( 1351.296000000,  1351.296000000,  1351.295996189) */
	BVDC_P_MAKE_HDMIRM(67_565MHz            , e30bit, eNone, eYCbCr444, 32453, 31125,         0, 0x07D1EB85,     1,      15,       0,        1,      1,    2,       4,  BDBG_STRING("84456.000")) /* TMDS Clock =    84.456000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.564800000, fRM =  28.152, fVCO = (  844.560000000,   844.560000000,   844.559999228) */
	BVDC_P_MAKE_HDMIRM(67_565MHz            , e36bit, eNone, eYCbCr444, 32453, 31125,         0, 0x09624DD2,     2,      18,       0,        1,      1,    2,       4, BDBG_STRING("101347.200")) /* TMDS Clock =   101.347200000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.564800000, fRM =  28.152, fVCO = ( 1013.472000000,  1013.472000000,  1013.471993923) */

	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001  , e24bit, eNone, eYCbCr444, 25024, 25025,         0, 0x0C7FDF43,     2,      25,       0,        2,      1,    2,       4,  BDBG_STRING("67497.303")) /* TMDS Clock =    67.497302697, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.497302697, fRM =  27.053, fVCO = ( 1349.946053946,  1349.946053946,  1349.946048975) */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001  , e30bit, eNone, eYCbCr444, 31280, 30030,         0, 0x07CFEB8A,     1,      15,       0,        1,      1,    2,       4,  BDBG_STRING("84371.628")) /* TMDS Clock =    84.371628372, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.497302697, fRM =  28.180, fVCO = (  843.716283716,   843.716283716,   843.716281414) */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001  , e36bit, eNone, eYCbCr444, 31280, 30030,         0, 0x095FE772,     2,      18,       0,        1,      1,    2,       4, BDBG_STRING("101245.954")) /* TMDS Clock =   101.245954046, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.497302697, fRM =  28.180, fVCO = ( 1012.459540460,  1012.459540460,  1012.459535122) */

	BVDC_P_MAKE_HDMIRM(74_25MHz             , e24bit, eNone, eYCbCr444, 32725, 32130,         0, 0x0DC00000,     2,      27,       0,        2,      1,    2,       4,  BDBG_STRING("74250.000")) /* TMDS Clock =    74.250000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.250000000, fRM =  27.500, fVCO = ( 1485.000000000,  1485.000000000,  1485.000000000) */
	BVDC_P_MAKE_HDMIRM(74_25MHz             , e30bit, eNone, eYCbCr444, 32725, 32368,         0, 0x08980000,     2,      17,       0,        1,      1,    2,       4,  BDBG_STRING("92812.500")) /* TMDS Clock =    92.812500000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.250000000, fRM =  27.298, fVCO = (  928.125000000,   928.125000000,   928.125000000) */
	BVDC_P_MAKE_HDMIRM(74_25MHz             , e36bit, eNone, eYCbCr444, 32736, 31744,         0, 0x0A500000,     2,      20,       0,        1,      1,    2,       4, BDBG_STRING("111375.000")) /* TMDS Clock =   111.375000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.250000000, fRM =  27.844, fVCO = ( 1113.750000000,  1113.750000000,  1113.750000000) */

	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001   , e24bit, eNone, eYCbCr444, 32500, 31941,         0, 0x0DBC7BC7,     2,      27,       0,        2,      1,    2,       4,  BDBG_STRING("74175.824")) /* TMDS Clock =    74.175824176, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.175824176, fRM =  27.528, fVCO = ( 1483.516483516,  1483.516483516,  1483.516478777) */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001   , e30bit, eNone, eYCbCr444, 31250, 30940,         0, 0x0895CD5C,     2,      17,       0,        1,      1,    2,       4,  BDBG_STRING("92719.780")) /* TMDS Clock =    92.719780220, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.175824176, fRM =  27.325, fVCO = (  927.197802198,   927.197802198,   927.197796822) */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001   , e36bit, eNone, eYCbCr444, 32625, 31668,         0, 0x0A4D5CD5,     2,      20,       0,        1,      1,    2,       4, BDBG_STRING("111263.736")) /* TMDS Clock =   111.263736264, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.175824176, fRM =  27.872, fVCO = ( 1112.637362637,  1112.637362637,  1112.637357473) */

	BVDC_P_MAKE_HDMIRM(148_5MHz             , e24bit, eNone, eYCbCr444, 32725, 32130,         0, 0x0DC00000,     2,      27,       0,        1,      1,    2,       4, BDBG_STRING("148500.000")) /* TMDS Clock =   148.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.500000000, fRM =  27.500, fVCO = ( 1485.000000000,  1485.000000000,  1485.000000000) */
	BVDC_P_MAKE_HDMIRM(148_5MHz             , e30bit, eNone, eYCbCr444, 32725, 32368,         0, 0x11300000,     3,      34,       1,        1,      1,    2,       7, BDBG_STRING("185625.000")) /* TMDS Clock =   185.625000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.500000000, fRM =  27.298, fVCO = ( 1856.250000000,  1856.250000000,  1856.250000000) */
	BVDC_P_MAKE_HDMIRM(148_5MHz             , e36bit, eNone, eYCbCr444, 32670, 32472,         0, 0x14A00000,     3,      41,       1,        1,      1,    2,       7, BDBG_STRING("222750.000")) /* TMDS Clock =   222.750000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.500000000, fRM =  27.165, fVCO = ( 2227.500000000,  2227.500000000,  2227.500000000) */

	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001   , e24bit, eNone, eYCbCr444, 32500, 31941,         0, 0x0DBC7BC7,     2,      27,       0,        1,      1,    2,       4, BDBG_STRING("148351.648")) /* TMDS Clock =   148.351648352, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.351648352, fRM =  27.528, fVCO = ( 1483.516483516,  1483.516483516,  1483.516478777) */
	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001   , e30bit, eNone, eYCbCr444, 31250, 30940,         0, 0x112B9AB9,     3,      34,       1,        1,      1,    2,       7, BDBG_STRING("185439.560")) /* TMDS Clock =   185.439560440, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.351648352, fRM =  27.325, fVCO = ( 1854.395604396,  1854.395604396,  1854.395600080) */
	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001   , e36bit, eNone, eYCbCr444, 30000, 29848,         0, 0x149AB9AB,     3,      41,       1,        1,      1,    2,       7, BDBG_STRING("222527.473")) /* TMDS Clock =   222.527472527, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.351648352, fRM =  27.192, fVCO = ( 2225.274725275,  2225.274725275,  2225.274721384) */

	/* 60/50 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz             , e24bit, eNone, eYCbCr444, 32656, 32500,         0, 0x0B0D844D,     2,      22,       0,        3,      1,    2,       4, BDBG_STRING("39790.080")) /* TMDS Clock =    39.790080000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.130, fVCO = ( 1193.702400000,  1193.702400000,  1193.702399969) */
	BVDC_P_MAKE_HDMIRM(39_79MHz             , e30bit, eNone, eYCbCr444, 31086, 30375,         0, 0x0935EE40,     2,      18,       0,        2,      1,    2,       4, BDBG_STRING("49737.600")) /* TMDS Clock =    49.737600000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.632, fVCO = (  994.752000000,   994.752000000,   994.751998901) */
	BVDC_P_MAKE_HDMIRM(39_79MHz             , e36bit, eNone, eYCbCr444, 32656, 32500,         0, 0x0B0D844D,     2,      22,       0,        2,      1,    2,       4, BDBG_STRING("59685.120")) /* TMDS Clock =    59.685120000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.130, fVCO = ( 1193.702400000,  1193.702400000,  1193.702399969) */

	BVDC_P_MAKE_HDMIRM(65MHz                , e24bit, eNone, eYCbCr444, 32500, 32400,         0, 0x0C097B42,     2,      24,       0,        2,      1,    2,       4, BDBG_STRING("65000.000")) /* TMDS Clock =    65.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.000000000, fRM =  27.083, fVCO = ( 1300.000000000,  1300.000000000,  1299.999997616) */
	BVDC_P_MAKE_HDMIRM(65MHz                , e30bit, eNone, eYCbCr444, 32500, 32400,         0, 0x0785ED09,     1,      15,       0,        1,      1,    2,       4, BDBG_STRING("81250.000")) /* TMDS Clock =    81.250000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.000000000, fRM =  27.083, fVCO = (  812.500000000,   812.500000000,   812.499996901) */
	BVDC_P_MAKE_HDMIRM(65MHz                , e36bit, eNone, eYCbCr444, 32500, 32400,         0, 0x09071C71,     2,      18,       0,        1,      1,    2,       4, BDBG_STRING("97500.000")) /* TMDS Clock =    97.500000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.000000000, fRM =  27.083, fVCO = (  975.000000000,   975.000000000,   974.999994993) */

	BVDC_P_MAKE_HDMIRM(65_286MHz            , e24bit, eNone, eYCbCr444, 32643, 32400,         0, 0x0C170A3D,     2,      24,       0,        2,      1,    2,       4, BDBG_STRING("65286.000")) /* TMDS Clock =    65.286000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.286000000, fRM =  27.203, fVCO = ( 1305.720000000,  1305.720000000,  1305.719997168) */
	BVDC_P_MAKE_HDMIRM(65_286MHz            , e30bit, eNone, eYCbCr444, 32643, 32400,         0, 0x078E6666,     1,      15,       0,        1,      1,    2,       4, BDBG_STRING("81607.500")) /* TMDS Clock =    81.607500000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.286000000, fRM =  27.203, fVCO = (  816.075000000,   816.075000000,   816.074997425) */
	BVDC_P_MAKE_HDMIRM(65_286MHz            , e36bit, eNone, eYCbCr444, 32643, 32400,         0, 0x091147AE,     2,      18,       0,        1,      1,    2,       4, BDBG_STRING("97929.000")) /* TMDS Clock =    97.929000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.286000000, fRM =  27.203, fVCO = (  979.290000000,   979.290000000,   979.289999485) */

	BVDC_P_MAKE_HDMIRM(64_022MHz            , e24bit, eNone, eYCbCr444, 32604, 31625,         0, 0x0BDB22D0,     2,      23,       0,        2,      1,    2,       4, BDBG_STRING("64022.400")) /* TMDS Clock =    64.022400000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.022400000, fRM =  27.836, fVCO = ( 1280.448000000,  1280.448000000,  1280.447994232) */
	BVDC_P_MAKE_HDMIRM(64_022MHz            , e30bit, eNone, eYCbCr444, 32604,   396,         1, 0x0768F5C2,     1,      15,       0,        1,      1,    2,       4, BDBG_STRING("80028.000")) /* TMDS Clock =    80.028000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.022400000, fRM =  26.676, fVCO = (  800.280000000,   800.280000000,   800.279996395) */
	BVDC_P_MAKE_HDMIRM(64_022MHz            , e36bit, eNone, eYCbCr444, 31122, 29750,         0, 0x08E45A1C,     2,      17,       0,        1,      1,    2,       4, BDBG_STRING("96033.600")) /* TMDS Clock =    96.033600000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.022400000, fRM =  28.245, fVCO = (  960.336000000,   960.336000000,   960.335995674) */

	/* 59.94 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001   , e24bit, eNone, eYCbCr444, 30144, 30030,         0, 0x0B0AB0AB,     2,      22,       0,        3,      1,    2,       4, BDBG_STRING("39750.330")) /* TMDS Clock =    39.750329670, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.157, fVCO = ( 1192.509890110,  1192.509890110,  1192.509889841) */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001   , e30bit, eNone, eYCbCr444, 32656, 31941,         0, 0x09339339,     2,      18,       0,        2,      1,    2,       4, BDBG_STRING("49687.912")) /* TMDS Clock =    49.687912088, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.660, fVCO = (  993.758241758,   993.758241758,   993.758240461) */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001   , e36bit, eNone, eYCbCr444, 30144, 30030,         0, 0x0B0AB0AB,     2,      22,       0,        2,      1,    2,       4, BDBG_STRING("59625.495")) /* TMDS Clock =    59.625494505, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.157, fVCO = ( 1192.509890110,  1192.509890110,  1192.509889841) */

	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001      , e24bit, eNone, eYCbCr444, 31250, 31185,         0, 0x0C066730,     2,      24,       0,        2,      1,    2,       4, BDBG_STRING("64935.065")) /* TMDS Clock =    64.935064935, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.935064935, fRM =  27.110, fVCO = ( 1298.701298701,  1298.701298701,  1298.701297760) */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001      , e30bit, eNone, eYCbCr444, 31250, 31185,         0, 0x0784007E,     1,      15,       0,        1,      1,    2,       4, BDBG_STRING("81168.831")) /* TMDS Clock =    81.168831169, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.935064935, fRM =  27.110, fVCO = (  811.688311688,   811.688311688,   811.688311100) */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001      , e36bit, eNone, eYCbCr444, 31250, 31185,         0, 0x0904CD64,     2,      18,       0,        1,      1,    2,       4, BDBG_STRING("97402.597")) /* TMDS Clock =    97.402597403, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.935064935, fRM =  27.110, fVCO = (  974.025974026,   974.025974026,   974.025973320) */

	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001  , e24bit, eNone, eYCbCr444, 32705, 32494,         0, 0x0C13F2B3,     2,      24,       0,        2,      1,    2,       4, BDBG_STRING("65220.779")) /* TMDS Clock =    65.220779221, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.220779221, fRM =  27.230, fVCO = ( 1304.415584416,  1304.415584416,  1304.415580988) */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001  , e30bit, eNone, eYCbCr444, 32705, 32494,         0, 0x078C77B0,     1,      15,       0,        1,      1,    2,       4, BDBG_STRING("81525.974")) /* TMDS Clock =    81.525974026, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.220779221, fRM =  27.230, fVCO = (  815.259740260,   815.259740260,   815.259738922) */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001  , e36bit, eNone, eYCbCr444, 32705, 32494,         0, 0x090EF606,     2,      18,       0,        1,      1,    2,       4, BDBG_STRING("97831.169")) /* TMDS Clock =    97.831168831, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.220779221, fRM =  27.230, fVCO = (  978.311688312,   978.311688312,   978.311684132) */

	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001  , e24bit, eNone, eYCbCr444, 31008, 30107,         0, 0x0BD81A98,     2,      23,       0,        2,      1,    2,       4, BDBG_STRING("63958.442")) /* TMDS Clock =    63.958441558, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    63.958441558, fRM =  27.864, fVCO = ( 1279.168831169,  1279.168831169,  1279.168825150) */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001  , e30bit, eNone, eYCbCr444, 32756,   431,         1, 0x0767109F,     1,      15,       0,        1,      1,    2,       4, BDBG_STRING("79948.052")) /* TMDS Clock =    79.948051948, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    63.958441558, fRM =  26.703, fVCO = (  799.480519481,   799.480519481,   799.480515718) */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001  , e36bit, eNone, eYCbCr444, 31464, 30107,         0, 0x08E213F2,     2,      17,       0,        1,      1,    2,       4, BDBG_STRING("95937.662")) /* TMDS Clock =    95.937662338, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    63.958441558, fRM =  28.273, fVCO = (  959.376623377,   959.376623377,   959.376618862) */

#elif (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_5)
/* Parameters generated 01/16/2014 at time 10:54:17 AM                       */
/* Note: The parameters are intended for 40nm silicon.                       */
/*       The PLL reference frequency used to generate                        */
/*       this table is  54.000Mhz.                                           */
/*       The PLL reference frequency is doubled.                             */
/*               ==> REF_CLK_CFG = 2.                                        */
/*                                                                           */
/* The following fields need to be programmed for each case:                 */
/*       Mask,                                                               */
/*       Color Depth,                                                        */
/*       Pixel Repetition,                                                   */
/*       DEN,         HDMI_RM.RATE_RATIO.DENOMINATOR                         */
/*       NUM,         HDMI_RM.SAMPLE_INC.NUMERATOR                           */
/*       SampleInc,   HDMI_RM.SAMPLE_INC.SAMPLE_INC                          */
/*       OFFSET,      HDMI_RM.OFFSET.OFFSET                                  */
/*       SHIFT,       HDMI_RM.FORMAT.SHIFT                                   */
/*       CkDivRM,     HDMI_TX_PHY.CK_DIV.RM                                  */
/*       CkDivVCO,    HDMI_TX_PHY.CK_DIV.VCO                                 */
/*       PDIV,        HDMI_TX_PHY.PLL_CFG.PDIV                               */
/*       Kp,          HDMI_TX_PHY.CTL_2.KP                                   */
/*       Ki,          HDMI_TX_PHY.CTL_2.KI                                   */
/*       Ka,          HDMI_TX_PHY.CTL_2.KA                                   */
/*       TMDS_Freq    Text string with TMDS Character rate in MHz.           */
	/* Mask                          ColorDepth                     PixelRep                         ColorComp,                 Denom    Num         Inc      Offset  shift       rmdiv         vco pxDiv  KP  KI  KA   pchRate */
	BVDC_P_MAKE_HDMIRM(25_2MHz,               e24bit, eNone, eYCbCr444, 32760, 18135,          1, 0x07000000,      1,        87,         12,    1, 10,  4,  3,  BDBG_STRING("25.200"))  /* Pixel Clock =  25200000.000000, TMDS Clock =  25200000.000000, Color Depth = 24, fRM =  17.379, fVCO = (3024.00000000000, 3024.00000000000, 3024.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(25_2MHz,               e30bit, eNone, eYCbCr444, 32760, 16536,          1, 0x06900000,      1,        79,          9,    1, 10,  4,  3,  BDBG_STRING("31.500"))  /* Pixel Clock =  25200000.000000, TMDS Clock =  31500000.000000, Color Depth = 30, fRM =  17.943, fVCO = (2835.00000000000, 2835.00000000000, 2835.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(25_2MHz,               e36bit, eNone, eYCbCr444, 32760, 18135,          1, 0x07000000,      1,        87,          8,    1, 10,  4,  3,  BDBG_STRING("37.800"))  /* Pixel Clock =  25200000.000000, TMDS Clock =  37800000.000000, Color Depth = 36, fRM =  17.379, fVCO = (3024.00000000000, 3024.00000000000, 3024.00000000000), ABS(Offset Error) =   0 */

	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001,     e24bit, eNone, eYCbCr444, 32000, 17764,          1, 0x06FE35B5,      1,        87,         12,    1, 10,  4,  3,  BDBG_STRING("25.175"))  /* Pixel Clock =  25174825.174825, TMDS Clock =  25174825.174825, Color Depth = 24, fRM =  17.362, fVCO = (3020.97902097902, 3020.97902097902, 3020.97902584076), ABS(Offset Error) = 0.188811182975769 */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001,     e30bit, eNone, eYCbCr444, 30000, 15188,          1, 0x068E525A,      1,        79,          9,    1, 10,  4,  3,  BDBG_STRING("31.469"))  /* Pixel Clock =  25174825.174825, TMDS Clock =  31468531.468532, Color Depth = 30, fRM =  17.925, fVCO = (2832.16783216783, 2832.16783216783, 2832.16784477234), ABS(Offset Error) = 0.489510461688042 */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001,     e36bit, eNone, eYCbCr444, 32000, 17764,          1, 0x06FE35B5,      1,        87,          8,    1, 10,  4,  3,  BDBG_STRING("37.762"))  /* Pixel Clock =  25174825.174825, TMDS Clock =  37762237.762238, Color Depth = 36, fRM =  17.362, fVCO = (3020.97902097902, 3020.97902097902, 3020.97902584076), ABS(Offset Error) = 0.188811182975769 */

	BVDC_P_MAKE_HDMIRM(27MHz,                 e24bit, eNone, eYCbCr444, 32725, 16660,          1, 0x06E00000,      1,        83,         11,    1, 10,  4,  3,  BDBG_STRING("27.000"))  /* Pixel Clock =  27000000.000000, TMDS Clock =  27000000.000000, Color Depth = 24, fRM =  17.892, fVCO = (2970.00000000000, 2970.00000000000, 2970.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(27MHz,                 e30bit, eNone, eYCbCr444, 32750, 31178,          1, 0x07D00000,      1,       122,         10,    1, 10,  4,  3,  BDBG_STRING("33.750"))  /* Pixel Clock =  27000000.000000, TMDS Clock =  33750000.000000, Color Depth = 30, fRM =  13.832, fVCO = (3375.00000000000, 3375.00000000000, 3375.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(27MHz,                 e36bit, eNone, eYCbCr444, 32760, 17160,          1, 0x06900000,      1,        80,          7,    1, 10,  4,  3,  BDBG_STRING("40.500"))  /* Pixel Clock =  27000000.000000, TMDS Clock =  40500000.000000, Color Depth = 36, fRM =  17.719, fVCO = (2835.00000000000, 2835.00000000000, 2835.00000000000), ABS(Offset Error) =   0 */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,       e24bit, eNone, eYCbCr444, 22022, 11178,          1, 0x06E1C28F,      1,        83,         11,    1, 10,  4,  3,  BDBG_STRING("27.027"))  /* Pixel Clock =  27027000.000000, TMDS Clock =  27027000.000000, Color Depth = 24, fRM =  17.909, fVCO = (2972.97000000000, 2972.97000000000, 2972.96999073029), ABS(Offset Error) = 0.359999999403954 */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,       e30bit, eNone, eYCbCr444, 32032, 30432,          1, 0x07D20000,      1,       122,         10,    1, 10,  4,  3,  BDBG_STRING("33.784"))  /* Pixel Clock =  27027000.000000, TMDS Clock =  33783750.000000, Color Depth = 30, fRM =  13.846, fVCO = (3378.37500000000, 3378.37500000000, 3378.37500000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,       e36bit, eNone, eYCbCr444, 21021, 10979,          1, 0x0691AE14,      1,        80,          7,    1, 10,  4,  3,  BDBG_STRING("40.541"))  /* Pixel Clock =  27027000.000000, TMDS Clock =  40540500.000000, Color Depth = 36, fRM =  17.736, fVCO = (2837.83500000000, 2837.83500000000, 2837.83498764038), ABS(Offset Error) = 0.479999989271164 */

	BVDC_P_MAKE_HDMIRM(27MHz,                 e24bit, e2x,   eYCbCr444, 32760, 18018,          1, 0x07800000,      1,        93,          6,    1, 10,  4,  3,  BDBG_STRING("54.000"))  /* Pixel Clock =  54000000.000000, TMDS Clock =  54000000.000000, Color Depth = 24, fRM =  17.419, fVCO = (3240.00000000000, 3240.00000000000, 3240.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(27MHz,                 e30bit, e2x,   eYCbCr444, 32750, 21746,          1, 0x07D00000,      1,       104,          5,    1, 10,  4,  3,  BDBG_STRING("67.500"))  /* Pixel Clock =  54000000.000000, TMDS Clock =  67500000.000000, Color Depth = 30, fRM =  16.226, fVCO = (3375.00000000000, 3375.00000000000, 3375.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(27MHz,                 e36bit, e2x,   eYCbCr444, 32760, 18018,          1, 0x07800000,      1,        93,          4,    1, 10,  4,  3,  BDBG_STRING("81.000"))  /* Pixel Clock =  54000000.000000, TMDS Clock =  81000000.000000, Color Depth = 36, fRM =  17.419, fVCO = (3240.00000000000, 3240.00000000000, 3240.00000000000), ABS(Offset Error) =   0 */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,       e24bit, e2x,   eYCbCr444, 32032, 17568,          1, 0x0781EB85,      1,        93,          6,    1, 10,  4,  3,  BDBG_STRING("54.054"))  /* Pixel Clock =  54054000.000000, TMDS Clock =  54054000.000000, Color Depth = 24, fRM =  17.437, fVCO = (3243.24000000000, 3243.24000000000, 3243.23999691010), ABS(Offset Error) = 0.11999998986721 */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,       e30bit, e2x,   eYCbCr444, 32725, 21675,          1, 0x07D20000,      1,       104,          5,    1, 10,  4,  3,  BDBG_STRING("67.568"))  /* Pixel Clock =  54054000.000000, TMDS Clock =  67567500.000000, Color Depth = 30, fRM =  16.242, fVCO = (3378.37500000000, 3378.37500000000, 3378.37500000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,       e36bit, e2x,   eYCbCr444, 32032, 17568,          1, 0x0781EB85,      1,        93,          4,    1, 10,  4,  3,  BDBG_STRING("81.081"))  /* Pixel Clock =  54054000.000000, TMDS Clock =  81081000.000000, Color Depth = 36, fRM =  17.437, fVCO = (3243.24000000000, 3243.24000000000, 3243.23999691010), ABS(Offset Error) = 0.11999998986721 */

	BVDC_P_MAKE_HDMIRM(27MHz,                 e24bit, e3x,   eYCbCr444, 32760, 18018,          1, 0x07800000,      1,        93,          3,    1, 10,  4,  3, BDBG_STRING("108.000"))  /* Pixel Clock = 108000000.000000, TMDS Clock = 108000000.000000, Color Depth = 24, fRM =  17.419, fVCO = (3240.00000000000, 3240.00000000000, 3240.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(27MHz,                 e30bit, e3x,   eYCbCr444, 32750, 17685,          1, 0x06400000,      1,        77,          2,    1, 10,  4,  3, BDBG_STRING("135.000"))  /* Pixel Clock = 108000000.000000, TMDS Clock = 135000000.000000, Color Depth = 30, fRM =  17.532, fVCO = (2700.00000000000, 2700.00000000000, 2700.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(27MHz,                 e36bit, e3x,   eYCbCr444, 32760, 18018,          1, 0x07800000,      1,        93,          2,    1, 10,  4,  3, BDBG_STRING("162.000"))  /* Pixel Clock = 108000000.000000, TMDS Clock = 162000000.000000, Color Depth = 36, fRM =  17.419, fVCO = (3240.00000000000, 3240.00000000000, 3240.00000000000), ABS(Offset Error) =   0 */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,       e24bit, e3x,   eYCbCr444, 32032, 17568,          1, 0x0781EB85,      1,        93,          3,    1, 10,  4,  3, BDBG_STRING("108.108"))  /* Pixel Clock = 108108000.000000, TMDS Clock = 108108000.000000, Color Depth = 24, fRM =  17.437, fVCO = (3243.24000000000, 3243.24000000000, 3243.23999691010), ABS(Offset Error) = 0.11999998986721 */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,       e30bit, e3x,   eYCbCr444, 32760, 17640,          1, 0x0641999A,      1,        77,          2,    1, 10,  4,  3, BDBG_STRING("135.135"))  /* Pixel Clock = 108108000.000000, TMDS Clock = 135135000.000000, Color Depth = 30, fRM =  17.550, fVCO = (2702.70000000000, 2702.70000000000, 2702.70001029968), ABS(Offset Error) = 0.400000005960464 */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,       e36bit, e3x,   eYCbCr444, 32032, 17568,          1, 0x0781EB85,      1,        93,          2,    1, 10,  4,  3, BDBG_STRING("162.162"))  /* Pixel Clock = 108108000.000000, TMDS Clock = 162162000.000000, Color Depth = 36, fRM =  17.437, fVCO = (3243.24000000000, 3243.24000000000, 3243.23999691010), ABS(Offset Error) = 0.11999998986721 */

	BVDC_P_MAKE_HDMIRM(67_565MHz,             e24bit, eNone, eYCbCr444, 32453, 29797,          1, 0x06418937,      1,        96,          4,    1, 10,  4,  3,  BDBG_STRING("67.565"))  /* Pixel Clock =  67564800.000000, TMDS Clock =  67564800.000000, Color Depth = 24, fRM =  14.076, fVCO = (2702.59200000000, 2702.59200000000, 2702.59199237823), ABS(Offset Error) = 0.296000003814697 */
	BVDC_P_MAKE_HDMIRM(67_565MHz,             e30bit, eNone, eYCbCr444, 32453, 17347,          1, 0x05DD70A4,      1,        72,          3,    1, 10,  4,  3,  BDBG_STRING("84.456"))  /* Pixel Clock =  67564800.000000, TMDS Clock =  84456000.000000, Color Depth = 30, fRM =  17.595, fVCO = (2533.68000000000, 2533.68000000000, 2533.68000411987), ABS(Offset Error) = 0.15999998152256 */
	BVDC_P_MAKE_HDMIRM(67_565MHz,             e36bit, eNone, eYCbCr444, 31671, 16704,          1, 0x0709BA5E,      1,        86,          3,    1, 10,  4,  3, BDBG_STRING("101.347"))  /* Pixel Clock =  67564800.000000, TMDS Clock = 101347200.000000, Color Depth = 36, fRM =  17.677, fVCO = (3040.41600000000, 3040.41600000000, 3040.41599464417), ABS(Offset Error) = 0.208000004291534 */

	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001,   e24bit, eNone, eYCbCr444, 31280, 28780,          1, 0x063FEFA2,      1,        96,          4,    1, 10,  4,  3,  BDBG_STRING("67.497"))  /* Pixel Clock =  67497302.697303, TMDS Clock =  67497302.697303, Color Depth = 24, fRM =  14.062, fVCO = (2699.89210789211, 2699.89210789211, 2699.89211082459), ABS(Offset Error) = 0.113886088132858 */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001,   e30bit, eNone, eYCbCr444, 31280, 16768,          1, 0x05DBF0A8,      1,        72,          3,    1, 10,  4,  3,  BDBG_STRING("84.372"))  /* Pixel Clock =  67497302.697303, TMDS Clock =  84371628.371628, Color Depth = 30, fRM =  17.577, fVCO = (2531.14885114885, 2531.14885114885, 2531.14885711670), ABS(Offset Error) = 0.231768220663071 */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001,   e36bit, eNone, eYCbCr444, 28152, 14891,          1, 0x0707ED96,      1,        86,          3,    1, 10,  4,  3, BDBG_STRING("101.246"))  /* Pixel Clock =  67497302.697303, TMDS Clock = 101245954.045954, Color Depth = 36, fRM =  17.659, fVCO = (3037.37862137862, 3037.37862137862, 3037.37861824036), ABS(Offset Error) = 0.121878132224083 */

	BVDC_P_MAKE_HDMIRM(74_25MHz,              e24bit, eNone, eYCbCr444, 32725, 16660,          1, 0x06E00000,      1,        83,          4,    1, 10,  4,  3,  BDBG_STRING("74.250"))  /* Pixel Clock =  74250000.000000, TMDS Clock =  74250000.000000, Color Depth = 24, fRM =  17.892, fVCO = (2970.00000000000, 2970.00000000000, 2970.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(74_25MHz,              e30bit, eNone, eYCbCr444, 32670, 18018,          1, 0x06720000,      1,        80,          3,    1, 10,  4,  3,  BDBG_STRING("92.813"))  /* Pixel Clock =  74250000.000000, TMDS Clock =  92812500.000000, Color Depth = 30, fRM =  17.402, fVCO = (2784.37500000000, 2784.37500000000, 2784.37500000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(74_25MHz,              e36bit, eNone, eYCbCr444, 32670, 18018,          1, 0x07BC0000,      1,        96,          3,    1, 10,  4,  3, BDBG_STRING("111.375"))  /* Pixel Clock =  74250000.000000, TMDS Clock = 111375000.000000, Color Depth = 36, fRM =  17.402, fVCO = (3341.25000000000, 3341.25000000000, 3341.25000000000), ABS(Offset Error) =   0 */

	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001,    e24bit, eNone, eYCbCr444, 30000, 15318,          1, 0x06DE3DE4,      1,        83,          4,    1, 10,  4,  3,  BDBG_STRING("74.176"))  /* Pixel Clock =  74175824.175824, TMDS Clock =  74175824.175824, Color Depth = 24, fRM =  17.874, fVCO = (2967.03296703297, 2967.03296703297, 2967.03297042847), ABS(Offset Error) = 0.131868109107018 */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001,    e30bit, eNone, eYCbCr444, 31875, 17629,          1, 0x06705A06,      1,        80,          3,    1, 10,  4,  3,  BDBG_STRING("92.720"))  /* Pixel Clock =  74175824.175824, TMDS Clock =  92719780.219780, Color Depth = 30, fRM =  17.385, fVCO = (2781.59340659341, 2781.59340659341, 2781.59341621399), ABS(Offset Error) = 0.373626351356506 */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001,    e36bit, eNone, eYCbCr444, 31875, 17629,          1, 0x07BA05A0,      1,        96,          3,    1, 10,  4,  3, BDBG_STRING("111.264"))  /* Pixel Clock =  74175824.175824, TMDS Clock = 111263736.263736, Color Depth = 36, fRM =  17.385, fVCO = (3337.91208791209, 3337.91208791209, 3337.91207885742), ABS(Offset Error) = 0.351648360490799 */

	BVDC_P_MAKE_HDMIRM(148_5MHz,              e24bit, eNone, eYCbCr444, 32725, 16660,          1, 0x06E00000,      1,        83,          2,    1, 10,  4,  3, BDBG_STRING("148.500"))  /* Pixel Clock = 148500000.000000, TMDS Clock = 148500000.000000, Color Depth = 24, fRM =  17.892, fVCO = (2970.00000000000, 2970.00000000000, 2970.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(148_5MHz,              e30bit, eNone, eYCbCr444, 32725, 17731,          1, 0x044C0000,      1,        53,          1,    1, 10,  4,  3, BDBG_STRING("185.625"))  /* Pixel Clock = 148500000.000000, TMDS Clock = 185625000.000000, Color Depth = 30, fRM =  17.512, fVCO = (1856.25000000000, 1856.25000000000, 1856.25000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(148_5MHz,              e36bit, eNone, eYCbCr444, 32670, 18018,          1, 0x05280000,      1,        64,          1,    1, 10,  4,  3, BDBG_STRING("222.750"))  /* Pixel Clock = 148500000.000000, TMDS Clock = 222750000.000000, Color Depth = 36, fRM =  17.402, fVCO = (2227.50000000000, 2227.50000000000, 2227.50000000000), ABS(Offset Error) =   0 */

	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001,    e24bit, eNone, eYCbCr444, 30000, 15318,          1, 0x06DE3DE4,      1,        83,          2,    1, 10,  4,  3, BDBG_STRING("148.352"))  /* Pixel Clock = 148351648.351648, TMDS Clock = 148351648.351648, Color Depth = 24, fRM =  17.874, fVCO = (2967.03296703297, 2967.03296703297, 2967.03297042847), ABS(Offset Error) = 0.131868109107018 */
	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001,    e30bit, eNone, eYCbCr444, 31250, 16980,          1, 0x044AE6AE,      1,        53,          1,    1, 10,  4,  3, BDBG_STRING("185.440"))  /* Pixel Clock = 148351648.351648, TMDS Clock = 185439560.439560, Color Depth = 30, fRM =  17.494, fVCO = (1854.39560439560, 1854.39560439560, 1854.39559364319), ABS(Offset Error) = 0.417582422494888 */
	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001,    e36bit, eNone, eYCbCr444, 31875, 17629,          1, 0x0526AE6B,      1,        64,          1,    1, 10,  4,  3, BDBG_STRING("222.527"))  /* Pixel Clock = 148351648.351648, TMDS Clock = 222527472.527473, Color Depth = 36, fRM =  17.385, fVCO = (2225.27472527473, 2225.27472527473, 2225.27472782135), ABS(Offset Error) = 0.098901093006134 */

	BVDC_P_MAKE_HDMIRM(297MHz,                e24bit, eNone, eYCbCr444, 32725, 16660,          1, 0x06E00000,      1,        83,          1,    1, 10,  4,  3, BDBG_STRING("297.000"))  /* Pixel Clock = 297000000.000000, TMDS Clock = 297000000.000000, Color Depth = 24, fRM =  17.892, fVCO = (2970.00000000000, 2970.00000000000, 2970.00000000000), ABS(Offset Error) =   0 */

	BVDC_P_MAKE_HDMIRM(297MHz_DIV_1_001,      e24bit, eNone, eYCbCr444, 30000, 15318,          1, 0x06DE3DE4,      1,        83,          1,    1, 10,  4,  3, BDBG_STRING("296.703"))  /* Pixel Clock = 296703296.703297, TMDS Clock = 296703296.703297, Color Depth = 24, fRM =  17.874, fVCO = (2967.03296703297, 2967.03296703297, 2967.03297042847), ABS(Offset Error) = 0.131868109107018 */

	/* 60/50 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz,              e24bit, eNone, eYCbCr444, 27632, 26250,          0, 0x075E5833,      1,        56,          8,    1, 10,  4,  3,  BDBG_STRING("39.790")) /* TMDS Clock =    39.790080000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  28.421, fVCO = ( 3183.206400000,  3183.206400000,  3183.206391335) */
	BVDC_P_MAKE_HDMIRM(39_79MHz,              e30bit, eNone, eYCbCr444, 31086, 30375,          0, 0x06E872B0,      1,        54,          6,    1, 10,  4,  3,  BDBG_STRING("49.738")) /* TMDS Clock =    49.737600000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.632, fVCO = ( 2984.256000000,  2984.256000000,  2984.255996704) */
	BVDC_P_MAKE_HDMIRM(39_79MHz,              e36bit, eNone, eYCbCr444, 31086, 30375,          0, 0x06E872B0,      1,        54,          5,    1, 10,  4,  3,  BDBG_STRING("59.685")) /* TMDS Clock =    59.685120000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.632, fVCO = ( 2984.256000000,  2984.256000000,  2984.255996704) */

	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001,    e24bit, eNone, eYCbCr444, 30144, 28665,          0, 0x075C75C7,      1,        56,          8,    1, 10,  4,  3,  BDBG_STRING("39.750")) /* TMDS Clock =    39.750329670, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  28.450, fVCO = ( 3180.026373626,  3180.026373626,  3180.026364326) */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001,    e30bit, eNone, eYCbCr444, 32656, 31941,          0, 0x06E6AE6A,      1,        54,          6,    1, 10,  4,  3,  BDBG_STRING("49.688")) /* TMDS Clock =    49.687912088, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.660, fVCO = ( 2981.274725275,  2981.274725275,  2981.274702072) */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001,    e36bit, eNone, eYCbCr444, 32656, 31941,          0, 0x06E6AE6A,      1,        54,          5,    1, 10,  4,  3,  BDBG_STRING("59.625")) /* TMDS Clock =    59.625494505, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.660, fVCO = ( 2981.274725275,  2981.274725275,  2981.274702072) */

	BVDC_P_MAKE_HDMIRM(65_286MHz,             e24bit, eNone, eYCbCr444, 32643, 17982,          1, 0x060B851F,      1,        75,          4,    1, 10,  4,  3,  BDBG_STRING("65.286"))  /* Pixel Clock =  65286000.000000, TMDS Clock =  65286000.000000, Color Depth = 24, fRM =  17.410, fVCO = (2611.44000000000, 2611.44000000000, 2611.44000720978), ABS(Offset Error) = 0.280000001192093 */
	BVDC_P_MAKE_HDMIRM(65_286MHz,             e30bit, eNone, eYCbCr444, 32760, 17640,          1, 0x078E6666,      1,        93,          4,    1, 10,  4,  3,  BDBG_STRING("81.608"))  /* Pixel Clock =  65286000.000000, TMDS Clock =  81607500.000000, Color Depth = 30, fRM =  17.550, fVCO = (3264.30000000000, 3264.30000000000, 3264.29998970032), ABS(Offset Error) = 0.400000005960464 */
	BVDC_P_MAKE_HDMIRM(65_286MHz,             e36bit, eNone, eYCbCr444, 32643, 17757,          1, 0x06CCF5C3,      1,        84,          3,    1, 10,  4,  3,  BDBG_STRING("97.929"))  /* Pixel Clock =  65286000.000000, TMDS Clock =  97929000.000000, Color Depth = 36, fRM =  17.487, fVCO = (2937.87000000000, 2937.87000000000, 2937.87001132965), ABS(Offset Error) = 0.439999967813492 */

	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001,   e24bit, eNone, eYCbCr444, 32736, 18084,          1, 0x0609F95A,      1,        75,          4,    1, 10,  4,  3,  BDBG_STRING("65.221"))  /* Pixel Clock =  65220779.220779, TMDS Clock =  65220779.220779, Color Depth = 24, fRM =  17.392, fVCO = (2608.83116883117, 2608.83116883117, 2608.83117485046), ABS(Offset Error) = 0.233766198158264 */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001,   e30bit, eNone, eYCbCr444, 32750, 17685,          1, 0x078C77B0,      1,        93,          4,    1, 10,  4,  3,  BDBG_STRING("81.526"))  /* Pixel Clock =  65220779.220779, TMDS Clock =  81525974.025974, Color Depth = 30, fRM =  17.532, fVCO = (3261.03896103896, 3261.03896103896, 3261.03895568848), ABS(Offset Error) = 0.207792222499847 */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001,   e36bit, eNone, eYCbCr444, 32085, 17503,          1, 0x06CB3885,      1,        84,          3,    1, 10,  4,  3,  BDBG_STRING("97.831"))  /* Pixel Clock =  65220779.220779, TMDS Clock =  97831168.831169, Color Depth = 36, fRM =  17.470, fVCO = (2934.93506493507, 2934.93506493506, 2934.93506526947), ABS(Offset Error) = 1.29869729280472E-02 */

	BVDC_P_MAKE_HDMIRM(64_022MHz,             e24bit, eNone, eYCbCr444, 29640, 15985,          1, 0x05ED9168,      1,        73,          4,    1, 10,  4,  3,  BDBG_STRING("64.022"))  /* Pixel Clock =  64022400.000000, TMDS Clock =  64022400.000000, Color Depth = 24, fRM =  17.540, fVCO = (2560.89600000000, 2560.89600000000, 2560.89598846436), ABS(Offset Error) = 0.448000013828278 */
	BVDC_P_MAKE_HDMIRM(64_022MHz,             e30bit, eNone, eYCbCr444, 32718, 17507,          1, 0x0768F5C3,      1,        91,          4,    1, 10,  4,  3,  BDBG_STRING("80.028"))  /* Pixel Clock =  64022400.000000, TMDS Clock =  80028000.000000, Color Depth = 30, fRM =  17.589, fVCO = (3201.12000000000, 3201.12000000000, 3201.12001132965), ABS(Offset Error) = 0.439999982714653 */
	BVDC_P_MAKE_HDMIRM(64_022MHz,             e36bit, eNone, eYCbCr444, 26676, 14324,          1, 0x06AB4396,      1,        82,          3,    1, 10,  4,  3,  BDBG_STRING("96.034"))  /* Pixel Clock =  64022400.000000, TMDS Clock =  96033600.000000, Color Depth = 36, fRM =  17.567, fVCO = (2881.00800000000, 2881.00800000000, 2881.00801277161), ABS(Offset Error) = 0.495999976992607 */

	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001,   e24bit, eNone, eYCbCr444, 29184, 15784,          1, 0x05EC0D4C,      1,        73,          4,    1, 10,  4,  3,  BDBG_STRING("63.958"))  /* Pixel Clock =  63958441.558442, TMDS Clock =  63958441.558442, Color Depth = 24, fRM =  17.523, fVCO = (2558.33766233766, 2558.33766233766, 2558.33765029907), ABS(Offset Error) = 0.467532500624657 */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001,   e30bit, eNone, eYCbCr444, 31920, 17129,          1, 0x076710A0,      1,        91,          4,    1, 10,  4,  3,  BDBG_STRING("79.948"))  /* Pixel Clock =  63958441.558442, TMDS Clock =  79948051.948052, Color Depth = 30, fRM =  17.571, fVCO = (3197.92207792208, 3197.92207792208, 3197.92208862305), ABS(Offset Error) = 0.41558438539505 */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001,   e36bit, eNone, eYCbCr444, 30780, 16575,          1, 0x06A98EF7,      1,        82,          3,    1, 10,  4,  3,  BDBG_STRING("95.938"))  /* Pixel Clock =  63958441.558442, TMDS Clock =  95937662.337662, Color Depth = 36, fRM =  17.550, fVCO = (2878.12987012987, 2878.12987012987, 2878.12989521027), ABS(Offset Error) = 0.974025920033455 */

	BVDC_P_MAKE_HDMIRM(65MHz,                 e24bit, eNone, eYCbCr444, 32760, 18270,          1, 0x0604BDA1,      1,        75,          4,    1, 10,  4,  3,  BDBG_STRING("65.000"))  /* Pixel Clock =  65000000.000000, TMDS Clock =  65000000.000000, Color Depth = 24, fRM =  17.333, fVCO = (2600.00000000000, 2600.00000000000, 2599.99999523163), ABS(Offset Error) = 0.185185179114342 */
	BVDC_P_MAKE_HDMIRM(65MHz,                 e30bit, eNone, eYCbCr444, 32500, 17180,          1, 0x0785ED09,      1,        92,          4,    1, 10,  4,  3,  BDBG_STRING("81.250"))  /* Pixel Clock =  65000000.000000, TMDS Clock =  81250000.000000, Color Depth = 30, fRM =  17.663, fVCO = (3250.00000000000, 3250.00000000000, 3249.99998760223), ABS(Offset Error) = 0.481481477618217 */
	BVDC_P_MAKE_HDMIRM(65MHz,                 e36bit, eNone, eYCbCr444, 32500, 17300,          1, 0x06C55555,      1,        83,          3,    1, 10,  4,  3,  BDBG_STRING("97.500"))  /* Pixel Clock =  65000000.000000, TMDS Clock =  97500000.000000, Color Depth = 36, fRM =  17.620, fVCO = (2925.00000000000, 2925.00000000000, 2924.99999141693), ABS(Offset Error) = 0.33333332836628 */

	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001,       e24bit, eNone, eYCbCr444, 32000, 17896,          1, 0x06033398,      1,        75,          4,    1, 10,  4,  3,  BDBG_STRING("64.935"))  /* Pixel Clock =  64935064.935065, TMDS Clock =  64935064.935065, Color Depth = 24, fRM =  17.316, fVCO = (2597.40259740260, 2597.40259740260, 2597.40259552002), ABS(Offset Error) = 7.31120854616165E-02 */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001,       e30bit, eNone, eYCbCr444, 31250, 16567,          1, 0x0784007E,      1,        92,          4,    1, 10,  4,  3,  BDBG_STRING("81.169"))  /* Pixel Clock =  64935064.935065, TMDS Clock =  81168831.168831, Color Depth = 30, fRM =  17.645, fVCO = (3246.75324675325, 3246.75324675325, 3246.75324440002), ABS(Offset Error) = 9.13901031017303E-02 */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001,       e36bit, eNone, eYCbCr444, 25000, 13346,          1, 0x06C39A0B,      1,        83,          3,    1, 10,  4,  3,  BDBG_STRING("97.403"))  /* Pixel Clock =  64935064.935065, TMDS Clock =  97402597.402597, Color Depth = 36, fRM =  17.603, fVCO = (2922.07792207792, 2922.07792207792, 2922.07791996002), ABS(Offset Error) = 8.22510868310928E-02 */

	BVDC_P_MAKE_HDMIRM(108MHz,                e24bit, eNone, eYCbCr444, 32760, 18018,          1, 0x07800000,      1,        93,          3,    1, 10,  4,  3, BDBG_STRING("108.000"))  /* Pixel Clock = 108000000.000000, TMDS Clock = 108000000.000000, Color Depth = 24, fRM =  17.419, fVCO = (3240.00000000000, 3240.00000000000, 3240.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(108MHz,                e30bit, eNone, eYCbCr444, 32750, 18340,          1, 0x06400000,      1,        78,          2,    1, 10,  4,  3, BDBG_STRING("135.000"))  /* Pixel Clock = 108000000.000000, TMDS Clock = 135000000.000000, Color Depth = 30, fRM =  17.308, fVCO = (2700.00000000000, 2700.00000000000, 2700.00000000000), ABS(Offset Error) =   0 */
	BVDC_P_MAKE_HDMIRM(108MHz,                e36bit, eNone, eYCbCr444, 32760, 18018,          1, 0x07800000,      1,        93,          2,    1, 10,  4,  3, BDBG_STRING("162.000"))  /* Pixel Clock = 108000000.000000, TMDS Clock = 162000000.000000, Color Depth = 36, fRM =  17.419, fVCO = (3240.00000000000, 3240.00000000000, 3240.00000000000), ABS(Offset Error) =   0 */

	BVDC_P_MAKE_HDMIRM(108MHz_DIV_1_001,      e24bit, eNone, eYCbCr444, 20000, 11031,          1, 0x077E14F9,      1,        93,          3,    1, 10,  4,  3, BDBG_STRING("107.892"))  /* Pixel Clock = 107892107.892108, TMDS Clock = 107892107.892108, Color Depth = 24, fRM =  17.402, fVCO = (3236.76323676324, 3236.76323676324, 3236.76324748993), ABS(Offset Error) = 0.41658341884613 */
	BVDC_P_MAKE_HDMIRM(108MHz_DIV_1_001,      e30bit, eNone, eYCbCr444, 25000, 14039,          1, 0x063E66CF,      1,        78,          2,    1, 10,  4,  3, BDBG_STRING("134.865"))  /* Pixel Clock = 107892107.892108, TMDS Clock = 134865134.865135, Color Depth = 30, fRM =  17.290, fVCO = (2697.30269730270, 2697.30269730270, 2697.30269336700), ABS(Offset Error) = 0.152847170829773 */
	BVDC_P_MAKE_HDMIRM(108MHz_DIV_1_001,      e36bit, eNone, eYCbCr444, 20000, 11031,          1, 0x077E14F9,      1,        93,          2,    1, 10,  4,  3, BDBG_STRING("161.838"))  /* Pixel Clock = 107892107.892108, TMDS Clock = 161838161.838162, Color Depth = 36, fRM =  17.402, fVCO = (3236.76323676324, 3236.76323676324, 3236.76324748993), ABS(Offset Error) = 0.41658341884613 */

#elif ((BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_6) || (BVDC_P_SUPPORT_HDMI_RM_VER == BVDC_P_HDMI_RM_VER_7))
/* Parameters generated 01/16/2014 at time 10:55:27 AM                       */
/* Note: The parameters are intended for 28nm silicon.                       */
/*       The PLL reference frequency used to generate                        */
/*       this table is  54.000Mhz.                                           */
/*       The PLL reference frequency is doubled.                             */
/*               ==> FREQ_DOUBLER_ENABLE = 1.                                */
/*                                                                           */
/* The following fields need to be programmed for each case:                 */
/*       Mask,                                                               */
/*       Color Depth,                                                        */
/*       Pixel Repetition,                                                   */
/*       DEN,         HDMI_RM.RATE_RATIO.DENOMINATOR                         */
/*       NUM,         HDMI_RM.SAMPLE_INC.NUMERATOR                           */
/*       SampleInc,   HDMI_RM.SAMPLE_INC.SAMPLE_INC                          */
/*       OFFSET,      HDMI_RM.OFFSET.OFFSET                                  */
/*       SHIFT,       HDMI_RM.FORMAT.SHIFT                                   */
/*       CkDivRM,     HDMI_TX_PHY.CLK_DIV.RM                                 */
/*       CkDivVCO,    HDMI_TX_PHY.CLK_DIV.VCO                                */
/*       PDIV,        HDMI_TX_PHY.PLL_CFG.PDIV                               */
/*       f_DBL_EN,    HDMI_TX_PHY.PLL_CTL_1.FREQ_DOUBLER_ENABLE              */
/*       VCO_Gain     HDMI_TX_PHY.CTL_2.VCO_GAIN                             */
/*       i_icp        HDMI_TX_PHY.CTL_3.ICP                                  */
/*       i_Rz         HDMI_TX_PHY.CTL_3.RZ                                   */
/*       i_Cz         HDMI_TX_PHY.CTL_3.CZ                                   */
/*       i_Cp         HDMI_TX_PHY.CTL_3.CP                                   */
/*       Rp           HDMI_TX_PHY.CTL_3.RP                                   */
/*       Cp1          HDMI_TX_PHY.CTL_3.CP1                                  */
/*       TMDS_Freq    Text string with TMDS Character rate in MHz.           */
/*       VCO_SEL      HDMI_TX_PHY.PLL_CTL_0.VCO_SEL                          */
/*                     Mask,                   CD,     PR,    ColorComp, DEN,   NUM, SAMPLE_INC,     OFFSET,  SHIFT, CK_DIV_RM, CK_DIV_VCO, PDIV, REF_CLK_CFG, VCO_Gain, i_icp, i_Rz, i_Cz, i_Cp, Rp, Cp1, TMDS_Freq,              VCO_SEL */
	BVDC_P_MAKE_HDMIRM(25_2MHz              , e24bit, eNone, eYCbCr444, 32760, 32292,         0, 0x08C00000,     2,      69,       15,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("25.200"),       0) /* TMDS Clock =    25.200000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.200000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(25_2MHz              , e30bit, eNone, eYCbCr444, 32760, 32292,         0, 0x08C00000,     2,      69,       12,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("31.500"),       0) /* TMDS Clock =    31.500000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.200000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(25_2MHz              , e36bit, eNone, eYCbCr444, 32760, 32292,         0, 0x08C00000,     2,      69,       10,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("37.800"),       0) /* TMDS Clock =    37.800000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.200000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */

	BVDC_P_MAKE_HDMIRM(25_2MHz              , e24bit, eNone, eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,       15,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("25.200"),       0) /* TMDS Clock =    25.200000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.200000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(25_2MHz              , e30bit, eNone, eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,       15,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("25.200"),       0) /* TMDS Clock =    25.200000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.200000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(25_2MHz              , e36bit, eNone, eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,       15,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("25.200"),       0) /* TMDS Clock =    25.200000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.200000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */

	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001    , e24bit, eNone, eYCbCr444, 30000, 29601,         0, 0x08BDC322,     2,      69,       15,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("25.175"),       0) /* TMDS Clock =    25.174825175, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.174825175, fRM =  27.419, fVCO = ( 3776.223776224,  3776.223776224,  3776.223775864) */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001    , e30bit, eNone, eYCbCr444, 30000, 29601,         0, 0x08BDC322,     2,      69,       12,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("31.469"),       0) /* TMDS Clock =    31.468531469, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.174825175, fRM =  27.419, fVCO = ( 3776.223776224,  3776.223776224,  3776.223775864) */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001    , e36bit, eNone, eYCbCr444, 30000, 29601,         0, 0x08BDC322,     2,      69,       10,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("37.762"),       0) /* TMDS Clock =    37.762237762, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.174825175, fRM =  27.419, fVCO = ( 3776.223776224,  3776.223776224,  3776.223775864) */

	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001    , e24bit, eNone, eYCbCr422, 30000, 29601,         0, 0x08BDC322,     2,      69,       15,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("25.175"),       0) /* TMDS Clock =    25.174825175, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.174825175, fRM =  27.419, fVCO = ( 3776.223776224,  3776.223776224,  3776.223775864) */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001    , e30bit, eNone, eYCbCr422, 30000, 29601,         0, 0x08BDC322,     2,      69,       15,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("25.175"),       0) /* TMDS Clock =    25.174825175, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.174825175, fRM =  27.419, fVCO = ( 3776.223776224,  3776.223776224,  3776.223775864) */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001    , e36bit, eNone, eYCbCr422, 30000, 29601,         0, 0x08BDC322,     2,      69,       15,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("25.175"),       0) /* TMDS Clock =    25.174825175, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    25.174825175, fRM =  27.419, fVCO = ( 3776.223776224,  3776.223776224,  3776.223775864) */

	BVDC_P_MAKE_HDMIRM(27MHz                , e24bit, eNone, eYCbCr444, 32760, 32292,         0, 0x08C00000,     2,      69,       14,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("27.000"),       0) /* TMDS Clock =    27.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e30bit, eNone, eYCbCr444, 32725, 32368,         0, 0x08980000,     2,      68,       11,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("33.750"),       0) /* TMDS Clock =    33.750000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.000000000, fRM =  27.298, fVCO = ( 3712.500000000,  3712.500000000,  3712.500000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e36bit, eNone, eYCbCr444, 32760, 32032,         0, 0x08700000,     2,      66,        9,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("40.500"),       0) /* TMDS Clock =    40.500000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.000000000, fRM =  27.614, fVCO = ( 3645.000000000,  3645.000000000,  3645.000000000) */

	BVDC_P_MAKE_HDMIRM(27MHz                , e24bit, eNone, eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,       14,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("27.000"),       0) /* TMDS Clock =    27.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e30bit, eNone, eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,       14,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("27.000"),       0) /* TMDS Clock =    27.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e36bit, eNone, eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,       14,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("27.000"),       0) /* TMDS Clock =    27.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e24bit, eNone, eYCbCr444, 28028, 27600,         0, 0x08C23D70,     2,      69,       14,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("27.027"),       0) /* TMDS Clock =    27.027000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.027000000, fRM =  27.364, fVCO = ( 3783.780000000,  3783.780000000,  3783.779983521) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e30bit, eNone, eYCbCr444, 22022, 21760,         0, 0x089A3333,     2,      68,       11,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("33.784"),       0) /* TMDS Clock =    33.783750000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.027000000, fRM =  27.271, fVCO = ( 3716.212500000,  3716.212500000,  3716.212494850) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e36bit, eNone, eYCbCr444, 32760, 32000,         0, 0x087228F5,     2,      66,        9,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("40.541"),       0) /* TMDS Clock =    40.540500000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.027000000, fRM =  27.586, fVCO = ( 3648.645000000,  3648.645000000,  3648.644980431) */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e24bit, eNone, eYCbCr422, 28028, 27600,         0, 0x08C23D70,     2,      69,       14,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("27.027"),       0) /* TMDS Clock =    27.027000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.027000000, fRM =  27.364, fVCO = ( 3783.780000000,  3783.780000000,  3783.779983521) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e30bit, eNone, eYCbCr422, 28028, 27600,         0, 0x08C23D70,     2,      69,       14,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("27.027"),       0) /* TMDS Clock =    27.027000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.027000000, fRM =  27.364, fVCO = ( 3783.780000000,  3783.780000000,  3783.779983521) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e36bit, eNone, eYCbCr422, 28028, 27600,         0, 0x08C23D70,     2,      69,       14,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("27.027"),       0) /* TMDS Clock =    27.027000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    27.027000000, fRM =  27.364, fVCO = ( 3783.780000000,  3783.780000000,  3783.779983521) */

	BVDC_P_MAKE_HDMIRM(27MHz                , e24bit, e2x  , eYCbCr444, 32760, 32292,         0, 0x08C00000,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.000"),       0) /* TMDS Clock =    54.000000000, CD = 24, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e30bit, e2x  , eYCbCr444, 32700, 32264,         0, 0x09600000,     2,      74,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.500"),       0) /* TMDS Clock =    67.500000000, CD = 30, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.365, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e36bit, e2x  , eYCbCr444, 32700, 32264,         0, 0x09600000,     2,      74,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.000"),       0) /* TMDS Clock =    81.000000000, CD = 36, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.365, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */

	BVDC_P_MAKE_HDMIRM(27MHz                , e24bit, e2x  , eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.000"),       0) /* TMDS Clock =    54.000000000, CD = 24, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e30bit, e2x  , eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.000"),       0) /* TMDS Clock =    54.000000000, CD = 24, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e36bit, e2x  , eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.000"),       0) /* TMDS Clock =    54.000000000, CD = 24, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e24bit, e2x  , eYCbCr444, 28028, 27600,         0, 0x08C23D70,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.054"),       0) /* TMDS Clock =    54.054000000, CD = 24, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.054000000, fRM =  27.364, fVCO = ( 3783.780000000,  3783.780000000,  3783.779983521) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e30bit, e2x  , eYCbCr444, 30030, 29600,         0, 0x09626666,     2,      74,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.567"),       0) /* TMDS Clock =    67.567500000, CD = 30, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.054000000, fRM =  27.338, fVCO = ( 4054.050000000,  4054.050000000,  4054.049989700) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e36bit, e2x  , eYCbCr444, 30030, 29600,         0, 0x09626666,     2,      74,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.081"),       0) /* TMDS Clock =    81.081000000, CD = 36, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.054000000, fRM =  27.338, fVCO = ( 4054.050000000,  4054.050000000,  4054.049989700) */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e24bit, e2x  , eYCbCr422, 28028, 27600,         0, 0x08C23D70,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.054"),       0) /* TMDS Clock =    54.054000000, CD = 24, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.054000000, fRM =  27.364, fVCO = ( 3783.780000000,  3783.780000000,  3783.779983521) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e30bit, e2x  , eYCbCr422, 28028, 27600,         0, 0x08C23D70,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.054"),       0) /* TMDS Clock =    54.054000000, CD = 24, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.054000000, fRM =  27.364, fVCO = ( 3783.780000000,  3783.780000000,  3783.779983521) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e36bit, e2x  , eYCbCr422, 28028, 27600,         0, 0x08C23D70,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.054"),       0) /* TMDS Clock =    54.054000000, CD = 24, PR =  1,  4:2:0 = FALSE, Pixel Clock =    54.054000000, fRM =  27.364, fVCO = ( 3783.780000000,  3783.780000000,  3783.779983521) */

	BVDC_P_MAKE_HDMIRM(27MHz                , e24bit, e3x  , eYCbCr444, 32760, 32214,         0, 0x07800000,     1,      59,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.000"),       0) /* TMDS Clock =   108.000000000, CD = 24, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.458, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e30bit, e3x  , eYCbCr444, 32700, 32264,         0, 0x09600000,     2,      74,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("135.000"),       0) /* TMDS Clock =   135.000000000, CD = 30, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.365, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e36bit, e3x  , eYCbCr444, 32760, 32214,         0, 0x07800000,     1,      59,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.000"),       0) /* TMDS Clock =   162.000000000, CD = 36, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.458, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */

	BVDC_P_MAKE_HDMIRM(27MHz                , e24bit, e3x  , eYCbCr422, 32760, 32214,         0, 0x07800000,     1,      59,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.000"),       0) /* TMDS Clock =   108.000000000, CD = 24, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.458, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e30bit, e3x  , eYCbCr422, 32760, 32214,         0, 0x07800000,     1,      59,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.000"),       0) /* TMDS Clock =   108.000000000, CD = 24, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.458, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(27MHz                , e36bit, e3x  , eYCbCr422, 32760, 32214,         0, 0x07800000,     1,      59,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.000"),       0) /* TMDS Clock =   108.000000000, CD = 24, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.458, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e24bit, e3x  , eYCbCr444, 30030, 29500,         0, 0x0781EB85,     1,      59,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.108"),       0) /* TMDS Clock =   108.108000000, CD = 24, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.108000000, fRM =  27.430, fVCO = ( 3243.240000000,  3243.240000000,  3243.239996910) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e30bit, e3x  , eYCbCr444, 30030, 29600,         0, 0x09626666,     2,      74,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("135.135"),       0) /* TMDS Clock =   135.135000000, CD = 30, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.108000000, fRM =  27.338, fVCO = ( 4054.050000000,  4054.050000000,  4054.049989700) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e36bit, e3x  , eYCbCr444, 30030, 29500,         0, 0x0781EB85,     1,      59,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.162"),       0) /* TMDS Clock =   162.162000000, CD = 36, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.108000000, fRM =  27.430, fVCO = ( 3243.240000000,  3243.240000000,  3243.239996910) */

	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e24bit, e3x  , eYCbCr422, 30030, 29500,         0, 0x0781EB85,     1,      59,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.108"),       0) /* TMDS Clock =   108.108000000, CD = 24, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.108000000, fRM =  27.430, fVCO = ( 3243.240000000,  3243.240000000,  3243.239996910) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e30bit, e3x  , eYCbCr422, 30030, 29500,         0, 0x0781EB85,     1,      59,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.108"),       0) /* TMDS Clock =   108.108000000, CD = 24, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.108000000, fRM =  27.430, fVCO = ( 3243.240000000,  3243.240000000,  3243.239996910) */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001      , e36bit, e3x  , eYCbCr422, 30030, 29500,         0, 0x0781EB85,     1,      59,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.108"),       0) /* TMDS Clock =   108.108000000, CD = 24, PR =  3,  4:2:0 = FALSE, Pixel Clock =   108.108000000, fRM =  27.430, fVCO = ( 3243.240000000,  3243.240000000,  3243.239996910) */

	BVDC_P_MAKE_HDMIRM(39_79MHz             , e24bit, eNone, eYCbCr444, 32656, 32500,         0, 0x084A2339,     2,      66,        9,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("39.790"),       0) /* TMDS Clock =    39.790080000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.130, fVCO = ( 3581.107200000,  3581.107200000,  3581.107180595) */
	BVDC_P_MAKE_HDMIRM(39_79MHz             , e30bit, eNone, eYCbCr444, 31086, 30375,         0, 0x0935EE40,     2,      72,        8,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("49.738"),       0) /* TMDS Clock =    49.737600000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.632, fVCO = ( 3979.008000000,  3979.008000000,  3979.007995605) */
	BVDC_P_MAKE_HDMIRM(39_79MHz             , e36bit, eNone, eYCbCr444, 32656, 32500,         0, 0x084A2339,     2,      66,        6,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("59.685"),       0) /* TMDS Clock =    59.685120000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.130, fVCO = ( 3581.107200000,  3581.107200000,  3581.107180595) */

	BVDC_P_MAKE_HDMIRM(39_79MHz             , e24bit, eNone, eYCbCr422, 32656, 32500,         0, 0x084A2339,     2,      66,        9,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("39.790"),       0) /* TMDS Clock =    39.790080000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.130, fVCO = ( 3581.107200000,  3581.107200000,  3581.107180595) */
	BVDC_P_MAKE_HDMIRM(39_79MHz             , e30bit, eNone, eYCbCr422, 32656, 32500,         0, 0x084A2339,     2,      66,        9,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("39.790"),       0) /* TMDS Clock =    39.790080000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.130, fVCO = ( 3581.107200000,  3581.107200000,  3581.107180595) */
	BVDC_P_MAKE_HDMIRM(39_79MHz             , e36bit, eNone, eYCbCr422, 32656, 32500,         0, 0x084A2339,     2,      66,        9,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("39.790"),       0) /* TMDS Clock =    39.790080000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.790080000, fRM =  27.130, fVCO = ( 3581.107200000,  3581.107200000,  3581.107180595) */

	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001   , e24bit, eNone, eYCbCr444, 30144, 30030,         0, 0x08480480,     2,      66,        9,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("39.750"),       0) /* TMDS Clock =    39.750329670, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.157, fVCO = ( 3577.529670330,  3577.529670330,  3577.529663086) */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001   , e30bit, eNone, eYCbCr444, 32656, 31941,         0, 0x09339339,     2,      72,        8,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("49.688"),       0) /* TMDS Clock =    49.687912088, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.660, fVCO = ( 3975.032967033,  3975.032967033,  3975.032961845) */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001   , e36bit, eNone, eYCbCr444, 30144, 30030,         0, 0x08480480,     2,      66,        6,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("59.625"),       0) /* TMDS Clock =    59.625494505, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.157, fVCO = ( 3577.529670330,  3577.529670330,  3577.529663086) */

	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001   , e24bit, eNone, eYCbCr422, 30144, 30030,         0, 0x08480480,     2,      66,        9,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("39.750"),       0) /* TMDS Clock =    39.750329670, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.157, fVCO = ( 3577.529670330,  3577.529670330,  3577.529663086) */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001   , e30bit, eNone, eYCbCr422, 30144, 30030,         0, 0x08480480,     2,      66,        9,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("39.750"),       0) /* TMDS Clock =    39.750329670, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.157, fVCO = ( 3577.529670330,  3577.529670330,  3577.529663086) */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001   , e36bit, eNone, eYCbCr422, 30144, 30030,         0, 0x08480480,     2,      66,        9,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("39.750"),       0) /* TMDS Clock =    39.750329670, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    39.750329670, fRM =  27.157, fVCO = ( 3577.529670330,  3577.529670330,  3577.529663086) */

	BVDC_P_MAKE_HDMIRM(54MHz                , e24bit, eNone, eYCbCr444, 32760, 32292,         0, 0x08C00000,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.000"),       0) /* TMDS Clock =    54.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(54MHz                , e30bit, eNone, eYCbCr444, 32767,     0,         1, 0x09600000,     2,      75,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.500"),       0) /* TMDS Clock =    67.500000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(54MHz                , e36bit, eNone, eYCbCr444, 32767,     0,         1, 0x09600000,     2,      75,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.000"),       0) /* TMDS Clock =    81.000000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */

	BVDC_P_MAKE_HDMIRM(54MHz                , e24bit, eNone, eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.000"),       0) /* TMDS Clock =    54.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(54MHz                , e30bit, eNone, eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.000"),       0) /* TMDS Clock =    54.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */
	BVDC_P_MAKE_HDMIRM(54MHz                , e36bit, eNone, eYCbCr422, 32760, 32292,         0, 0x08C00000,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("54.000"),       0) /* TMDS Clock =    54.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    54.000000000, fRM =  27.391, fVCO = ( 3780.000000000,  3780.000000000,  3780.000000000) */

	BVDC_P_MAKE_HDMIRM(54MHz_MUL_1_001      , e24bit, eNone, eYCbCr444, 30000, 29601,         0, 0x08BDC322,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("53.946"),       0) /* TMDS Clock =    53.946053946, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    53.946053946, fRM =  27.419, fVCO = ( 3776.223776224,  3776.223776224,  3776.223775864) */
	BVDC_P_MAKE_HDMIRM(54MHz_MUL_1_001      , e30bit, eNone, eYCbCr444, 32000,    32,         1, 0x095D9A36,     2,      75,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.433"),       0) /* TMDS Clock =    67.432567433, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    53.946053946, fRM =  27.027, fVCO = ( 4045.954045954,  4045.954045954,  4045.954027176) */
	BVDC_P_MAKE_HDMIRM(54MHz_MUL_1_001      , e36bit, eNone, eYCbCr444, 32000,    32,         1, 0x095D9A36,     2,      75,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("80.919"),       0) /* TMDS Clock =    80.919080919, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    53.946053946, fRM =  27.027, fVCO = ( 4045.954045954,  4045.954045954,  4045.954027176) */

	BVDC_P_MAKE_HDMIRM(54MHz_MUL_1_001      , e24bit, eNone, eYCbCr422, 30000, 29601,         0, 0x08BDC322,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("53.946"),       0) /* TMDS Clock =    53.946053946, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    53.946053946, fRM =  27.419, fVCO = ( 3776.223776224,  3776.223776224,  3776.223775864) */
	BVDC_P_MAKE_HDMIRM(54MHz_MUL_1_001      , e30bit, eNone, eYCbCr422, 30000, 29601,         0, 0x08BDC322,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("53.946"),       0) /* TMDS Clock =    53.946053946, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    53.946053946, fRM =  27.419, fVCO = ( 3776.223776224,  3776.223776224,  3776.223775864) */
	BVDC_P_MAKE_HDMIRM(54MHz_MUL_1_001      , e36bit, eNone, eYCbCr422, 30000, 29601,         0, 0x08BDC322,     2,      69,        7,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("53.946"),       0) /* TMDS Clock =    53.946053946, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    53.946053946, fRM =  27.419, fVCO = ( 3776.223776224,  3776.223776224,  3776.223775864) */

	BVDC_P_MAKE_HDMIRM(64_022MHz            , e24bit, eNone, eYCbCr444, 31122, 30625,         0, 0x08E45A1C,     2,      70,        6,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("64.022"),       0) /* TMDS Clock =    64.022400000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.022400000, fRM =  27.438, fVCO = ( 3841.344000000,  3841.344000000,  3841.343982697) */
	BVDC_P_MAKE_HDMIRM(64_022MHz            , e30bit, eNone, eYCbCr444, 32604, 32120,         0, 0x09433333,     2,      73,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("80.028"),       0) /* TMDS Clock =    80.028000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.022400000, fRM =  27.407, fVCO = ( 4001.400000000,  4001.400000000,  4001.399994850) */
	BVDC_P_MAKE_HDMIRM(64_022MHz            , e36bit, eNone, eYCbCr444, 31122, 30625,         0, 0x08E45A1C,     2,      70,        4,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("96.034"),       0) /* TMDS Clock =    96.033600000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.022400000, fRM =  27.438, fVCO = ( 3841.344000000,  3841.344000000,  3841.343982697) */

	BVDC_P_MAKE_HDMIRM(64_022MHz            , e24bit, eNone, eYCbCr422, 31122, 30625,         0, 0x08E45A1C,     2,      70,        6,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("64.022"),       0) /* TMDS Clock =    64.022400000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.022400000, fRM =  27.438, fVCO = ( 3841.344000000,  3841.344000000,  3841.343982697) */
	BVDC_P_MAKE_HDMIRM(64_022MHz            , e30bit, eNone, eYCbCr422, 31122, 30625,         0, 0x08E45A1C,     2,      70,        6,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("64.022"),       0) /* TMDS Clock =    64.022400000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.022400000, fRM =  27.438, fVCO = ( 3841.344000000,  3841.344000000,  3841.343982697) */
	BVDC_P_MAKE_HDMIRM(64_022MHz            , e36bit, eNone, eYCbCr422, 31122, 30625,         0, 0x08E45A1C,     2,      70,        6,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("64.022"),       0) /* TMDS Clock =    64.022400000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.022400000, fRM =  27.438, fVCO = ( 3841.344000000,  3841.344000000,  3841.343982697) */

	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001  , e24bit, eNone, eYCbCr444, 30096, 29645,         0, 0x08E213F2,     2,      70,        6,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("63.958"),       0) /* TMDS Clock =    63.958441558, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    63.958441558, fRM =  27.466, fVCO = ( 3837.506493506,  3837.506493506,  3837.506475449) */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001  , e30bit, eNone, eYCbCr444, 28500, 28105,         0, 0x0940D4C7,     2,      73,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("79.948"),       0) /* TMDS Clock =    79.948051948, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    63.958441558, fRM =  27.434, fVCO = ( 3997.402597403,  3997.402597403,  3997.402585030) */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001  , e36bit, eNone, eYCbCr444, 30096, 29645,         0, 0x08E213F2,     2,      70,        4,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("95.938"),       0) /* TMDS Clock =    95.937662338, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    63.958441558, fRM =  27.466, fVCO = ( 3837.506493506,  3837.506493506,  3837.506475449) */

	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001  , e24bit, eNone, eYCbCr422, 30096, 29645,         0, 0x08E213F2,     2,      70,        6,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("63.958"),       0) /* TMDS Clock =    63.958441558, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    63.958441558, fRM =  27.466, fVCO = ( 3837.506493506,  3837.506493506,  3837.506475449) */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001  , e30bit, eNone, eYCbCr422, 30096, 29645,         0, 0x08E213F2,     2,      70,        6,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("63.958"),       0) /* TMDS Clock =    63.958441558, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    63.958441558, fRM =  27.466, fVCO = ( 3837.506493506,  3837.506493506,  3837.506475449) */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001  , e36bit, eNone, eYCbCr422, 30096, 29645,         0, 0x08E213F2,     2,      70,        6,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("63.958"),       0) /* TMDS Clock =    63.958441558, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    63.958441558, fRM =  27.466, fVCO = ( 3837.506493506,  3837.506493506,  3837.506475449) */

	BVDC_P_MAKE_HDMIRM(65MHz                , e24bit, eNone, eYCbCr444, 32760, 31752,         0, 0x09071C71,     2,      70,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.000"),       0) /* TMDS Clock =    65.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.000000000, fRM =  27.857, fVCO = ( 3900.000000000,  3900.000000000,  3899.999979973) */
	BVDC_P_MAKE_HDMIRM(65MHz                , e30bit, eNone, eYCbCr444, 32500, 32400,         0, 0x0967684B,     2,      75,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.250"),       0) /* TMDS Clock =    81.250000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.000000000, fRM =  27.083, fVCO = ( 4062.500000000,  4062.500000000,  4062.499978065) */
	BVDC_P_MAKE_HDMIRM(65MHz                , e36bit, eNone, eYCbCr444, 32760, 31752,         0, 0x09071C71,     2,      70,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("97.500"),       0) /* TMDS Clock =    97.500000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.000000000, fRM =  27.857, fVCO = ( 3900.000000000,  3900.000000000,  3899.999979973) */

	BVDC_P_MAKE_HDMIRM(65MHz                , e24bit, eNone, eYCbCr422, 32760, 31752,         0, 0x09071C71,     2,      70,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.000"),       0) /* TMDS Clock =    65.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.000000000, fRM =  27.857, fVCO = ( 3900.000000000,  3900.000000000,  3899.999979973) */
	BVDC_P_MAKE_HDMIRM(65MHz                , e30bit, eNone, eYCbCr422, 32760, 31752,         0, 0x09071C71,     2,      70,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.000"),       0) /* TMDS Clock =    65.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.000000000, fRM =  27.857, fVCO = ( 3900.000000000,  3900.000000000,  3899.999979973) */
	BVDC_P_MAKE_HDMIRM(65MHz                , e36bit, eNone, eYCbCr422, 32760, 31752,         0, 0x09071C71,     2,      70,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.000"),       0) /* TMDS Clock =    65.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.000000000, fRM =  27.857, fVCO = ( 3900.000000000,  3900.000000000,  3899.999979973) */

	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001      , e24bit, eNone, eYCbCr444, 30000, 29106,         0, 0x0904CD64,     2,      70,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("64.935"),       0) /* TMDS Clock =    64.935064935, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.935064935, fRM =  27.885, fVCO = ( 3896.103896104,  3896.103896104,  3896.103893280) */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001      , e30bit, eNone, eYCbCr444, 31250, 31185,         0, 0x0965009D,     2,      75,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.169"),       0) /* TMDS Clock =    81.168831169, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.935064935, fRM =  27.110, fVCO = ( 4058.441558442,  4058.441558442,  4058.441542625) */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001      , e36bit, eNone, eYCbCr444, 30000, 29106,         0, 0x0904CD64,     2,      70,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("97.403"),       0) /* TMDS Clock =    97.402597403, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.935064935, fRM =  27.885, fVCO = ( 3896.103896104,  3896.103896104,  3896.103893280) */

	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001      , e24bit, eNone, eYCbCr422, 30000, 29106,         0, 0x0904CD64,     2,      70,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("64.935"),       0) /* TMDS Clock =    64.935064935, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.935064935, fRM =  27.885, fVCO = ( 3896.103896104,  3896.103896104,  3896.103893280) */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001      , e30bit, eNone, eYCbCr422, 30000, 29106,         0, 0x0904CD64,     2,      70,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("64.935"),       0) /* TMDS Clock =    64.935064935, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.935064935, fRM =  27.885, fVCO = ( 3896.103896104,  3896.103896104,  3896.103893280) */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001      , e36bit, eNone, eYCbCr422, 30000, 29106,         0, 0x0904CD64,     2,      70,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("64.935"),       0) /* TMDS Clock =    64.935064935, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    64.935064935, fRM =  27.885, fVCO = ( 3896.103896104,  3896.103896104,  3896.103893280) */

	BVDC_P_MAKE_HDMIRM(65_286MHz            , e24bit, eNone, eYCbCr444, 32643, 31950,         0, 0x091147AE,     2,      71,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.286"),       0) /* TMDS Clock =    65.286000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.286000000, fRM =  27.586, fVCO = ( 3917.160000000,  3917.160000000,  3917.159997940) */
	BVDC_P_MAKE_HDMIRM(65_286MHz            , e30bit, eNone, eYCbCr444, 32643, 31968,         0, 0x09720000,     2,      74,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.608"),       0) /* TMDS Clock =    81.607500000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.286000000, fRM =  27.570, fVCO = ( 4080.375000000,  4080.375000000,  4080.375000000) */
	BVDC_P_MAKE_HDMIRM(65_286MHz            , e36bit, eNone, eYCbCr444, 32643, 31950,         0, 0x091147AE,     2,      71,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("97.929"),       0) /* TMDS Clock =    97.929000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.286000000, fRM =  27.586, fVCO = ( 3917.160000000,  3917.160000000,  3917.159997940) */

	BVDC_P_MAKE_HDMIRM(65_286MHz            , e24bit, eNone, eYCbCr422, 32643, 31950,         0, 0x091147AE,     2,      71,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.286"),       0) /* TMDS Clock =    65.286000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.286000000, fRM =  27.586, fVCO = ( 3917.160000000,  3917.160000000,  3917.159997940) */
	BVDC_P_MAKE_HDMIRM(65_286MHz            , e30bit, eNone, eYCbCr422, 32643, 31950,         0, 0x091147AE,     2,      71,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.286"),       0) /* TMDS Clock =    65.286000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.286000000, fRM =  27.586, fVCO = ( 3917.160000000,  3917.160000000,  3917.159997940) */
	BVDC_P_MAKE_HDMIRM(65_286MHz            , e36bit, eNone, eYCbCr422, 32643, 31950,         0, 0x091147AE,     2,      71,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.286"),       0) /* TMDS Clock =    65.286000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.286000000, fRM =  27.586, fVCO = ( 3917.160000000,  3917.160000000,  3917.159997940) */

	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001  , e24bit, eNone, eYCbCr444, 27900, 27335,         0, 0x090EF606,     2,      71,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.221"),       0) /* TMDS Clock =    65.220779221, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.220779221, fRM =  27.613, fVCO = ( 3913.246753247,  3913.246753247,  3913.246736526) */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001  , e30bit, eNone, eYCbCr444, 23250, 22792,         0, 0x096F959C,     2,      74,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.526"),       0) /* TMDS Clock =    81.525974026, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.220779221, fRM =  27.598, fVCO = ( 4076.298701299,  4076.298701299,  4076.298694611) */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001  , e36bit, eNone, eYCbCr444, 27900, 27335,         0, 0x090EF606,     2,      71,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("97.831"),       0) /* TMDS Clock =    97.831168831, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.220779221, fRM =  27.613, fVCO = ( 3913.246753247,  3913.246753247,  3913.246736526) */

	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001  , e24bit, eNone, eYCbCr422, 27900, 27335,         0, 0x090EF606,     2,      71,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.221"),       0) /* TMDS Clock =    65.220779221, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.220779221, fRM =  27.613, fVCO = ( 3913.246753247,  3913.246753247,  3913.246736526) */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001  , e30bit, eNone, eYCbCr422, 27900, 27335,         0, 0x090EF606,     2,      71,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.221"),       0) /* TMDS Clock =    65.220779221, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.220779221, fRM =  27.613, fVCO = ( 3913.246753247,  3913.246753247,  3913.246736526) */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001  , e36bit, eNone, eYCbCr422, 27900, 27335,         0, 0x090EF606,     2,      71,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("65.221"),       0) /* TMDS Clock =    65.220779221, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    65.220779221, fRM =  27.613, fVCO = ( 3913.246753247,  3913.246753247,  3913.246736526) */

	BVDC_P_MAKE_HDMIRM(67_565MHz            , e24bit, eNone, eYCbCr444, 31280, 31250,         0, 0x09624DD2,     2,      75,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.565"),       0) /* TMDS Clock =    67.564800000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.564800000, fRM =  27.026, fVCO = ( 4053.888000000,  4053.888000000,  4053.887975693) */
	BVDC_P_MAKE_HDMIRM(67_565MHz            , e30bit, eNone, eYCbCr444, 32062, 31775,         0, 0x07D1EB85,     1,      62,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("84.456"),       0) /* TMDS Clock =    84.456000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.564800000, fRM =  27.244, fVCO = ( 3378.240000000,  3378.240000000,  3378.239996910) */
	BVDC_P_MAKE_HDMIRM(67_565MHz            , e36bit, eNone, eYCbCr444, 31280, 31250,         0, 0x09624DD2,     2,      75,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("101.347"),       0) /* TMDS Clock =   101.347200000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.564800000, fRM =  27.026, fVCO = ( 4053.888000000,  4053.888000000,  4053.887975693) */

	BVDC_P_MAKE_HDMIRM(67_565MHz            , e24bit, eNone, eYCbCr422, 31280, 31250,         0, 0x09624DD2,     2,      75,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.565"),       0) /* TMDS Clock =    67.564800000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.564800000, fRM =  27.026, fVCO = ( 4053.888000000,  4053.888000000,  4053.887975693) */
	BVDC_P_MAKE_HDMIRM(67_565MHz            , e30bit, eNone, eYCbCr422, 31280, 31250,         0, 0x09624DD2,     2,      75,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.565"),       0) /* TMDS Clock =    67.564800000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.564800000, fRM =  27.026, fVCO = ( 4053.888000000,  4053.888000000,  4053.887975693) */
	BVDC_P_MAKE_HDMIRM(67_565MHz            , e36bit, eNone, eYCbCr422, 31280, 31250,         0, 0x09624DD2,     2,      75,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.565"),       0) /* TMDS Clock =    67.564800000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.564800000, fRM =  27.026, fVCO = ( 4053.888000000,  4053.888000000,  4053.887975693) */

	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001  , e24bit, eNone, eYCbCr444, 25024, 25025,         0, 0x095FE772,     2,      75,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.497"),       0) /* TMDS Clock =    67.497302697, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.497302697, fRM =  27.053, fVCO = ( 4049.838161838,  4049.838161838,  4049.838140488) */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001  , e30bit, eNone, eYCbCr444, 31280, 31031,         0, 0x07CFEB8A,     1,      62,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("84.372"),       0) /* TMDS Clock =    84.371628372, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.497302697, fRM =  27.271, fVCO = ( 3374.865134865,  3374.865134865,  3374.865125656) */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001  , e36bit, eNone, eYCbCr444, 25024, 25025,         0, 0x095FE772,     2,      75,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("101.246"),       0) /* TMDS Clock =   101.245954046, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.497302697, fRM =  27.053, fVCO = ( 4049.838161838,  4049.838161838,  4049.838140488) */

	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001  , e24bit, eNone, eYCbCr422, 25024, 25025,         0, 0x095FE772,     2,      75,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.497"),       0) /* TMDS Clock =    67.497302697, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.497302697, fRM =  27.053, fVCO = ( 4049.838161838,  4049.838161838,  4049.838140488) */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001  , e30bit, eNone, eYCbCr422, 25024, 25025,         0, 0x095FE772,     2,      75,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.497"),       0) /* TMDS Clock =    67.497302697, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.497302697, fRM =  27.053, fVCO = ( 4049.838161838,  4049.838161838,  4049.838140488) */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001  , e36bit, eNone, eYCbCr422, 25024, 25025,         0, 0x095FE772,     2,      75,        6,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("67.497"),       0) /* TMDS Clock =    67.497302697, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    67.497302697, fRM =  27.053, fVCO = ( 4049.838161838,  4049.838161838,  4049.838140488) */

	BVDC_P_MAKE_HDMIRM(74_25MHz             , e24bit, eNone, eYCbCr444, 32725, 32368,         0, 0x08980000,     2,      68,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("74.250"),       0) /* TMDS Clock =    74.250000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.250000000, fRM =  27.298, fVCO = ( 3712.500000000,  3712.500000000,  3712.500000000) */
	BVDC_P_MAKE_HDMIRM(74_25MHz             , e30bit, eNone, eYCbCr444, 32725, 32368,         0, 0x08980000,     2,      68,        4,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("92.813"),       0) /* TMDS Clock =    92.812500000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.250000000, fRM =  27.298, fVCO = ( 3712.500000000,  3712.500000000,  3712.500000000) */
	BVDC_P_MAKE_HDMIRM(74_25MHz             , e36bit, eNone, eYCbCr444, 32670, 32208,         0, 0x07BC0000,     1,      61,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("111.375"),       0) /* TMDS Clock =   111.375000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.250000000, fRM =  27.387, fVCO = ( 3341.250000000,  3341.250000000,  3341.250000000) */

	BVDC_P_MAKE_HDMIRM(74_25MHz             , e24bit, eNone, eYCbCr422, 32725, 32368,         0, 0x08980000,     2,      68,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("74.250"),       0) /* TMDS Clock =    74.250000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.250000000, fRM =  27.298, fVCO = ( 3712.500000000,  3712.500000000,  3712.500000000) */
	BVDC_P_MAKE_HDMIRM(74_25MHz             , e30bit, eNone, eYCbCr422, 32725, 32368,         0, 0x08980000,     2,      68,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("74.250"),       0) /* TMDS Clock =    74.250000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.250000000, fRM =  27.298, fVCO = ( 3712.500000000,  3712.500000000,  3712.500000000) */
	BVDC_P_MAKE_HDMIRM(74_25MHz             , e36bit, eNone, eYCbCr422, 32725, 32368,         0, 0x08980000,     2,      68,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("74.250"),       0) /* TMDS Clock =    74.250000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.250000000, fRM =  27.298, fVCO = ( 3712.500000000,  3712.500000000,  3712.500000000) */

	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001   , e24bit, eNone, eYCbCr444, 31250, 30940,         0, 0x0895CD5C,     2,      68,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("74.176"),       0) /* TMDS Clock =    74.175824176, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.175824176, fRM =  27.325, fVCO = ( 3708.791208791,  3708.791208791,  3708.791187286) */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001   , e30bit, eNone, eYCbCr444, 31250, 30940,         0, 0x0895CD5C,     2,      68,        4,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("92.720"),       0) /* TMDS Clock =    92.719780220, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.175824176, fRM =  27.325, fVCO = ( 3708.791208791,  3708.791208791,  3708.791187286) */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001   , e36bit, eNone, eYCbCr444, 28125, 27755,         0, 0x07BA05A0,     1,      61,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("111.264"),       0) /* TMDS Clock =   111.263736264, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.175824176, fRM =  27.415, fVCO = ( 3337.912087912,  3337.912087912,  3337.912078857) */

	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001   , e24bit, eNone, eYCbCr422, 31250, 30940,         0, 0x0895CD5C,     2,      68,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("74.176"),       0) /* TMDS Clock =    74.175824176, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.175824176, fRM =  27.325, fVCO = ( 3708.791208791,  3708.791208791,  3708.791187286) */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001   , e30bit, eNone, eYCbCr422, 31250, 30940,         0, 0x0895CD5C,     2,      68,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("74.176"),       0) /* TMDS Clock =    74.175824176, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.175824176, fRM =  27.325, fVCO = ( 3708.791208791,  3708.791208791,  3708.791187286) */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001   , e36bit, eNone, eYCbCr422, 31250, 30940,         0, 0x0895CD5C,     2,      68,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("74.176"),       0) /* TMDS Clock =    74.175824176, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    74.175824176, fRM =  27.325, fVCO = ( 3708.791208791,  3708.791208791,  3708.791187286) */

	BVDC_P_MAKE_HDMIRM(83_5MHz              , e24bit, eNone, eYCbCr444, 32732,  9604,         1, 0x07BB425E,     1,      80,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("83.500"),       0) /* TMDS Clock =    83.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.500000000, fRM =  20.875, fVCO = ( 3340.000000000,  3340.000000000,  3339.999979019) */
	BVDC_P_MAKE_HDMIRM(83_5MHz              , e30bit, eNone, eYCbCr444, 32732,  9604,         1, 0x09AA12F6,     2,     100,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("104.375"),       0) /* TMDS Clock =   104.375000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.500000000, fRM =  20.875, fVCO = ( 4175.000000000,  4175.000000000,  4174.999986649) */
	BVDC_P_MAKE_HDMIRM(83_5MHz              , e36bit, eNone, eYCbCr444, 32732,   196,         1, 0x08B2AAAA,     2,      70,        3,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("125.250"),       0) /* TMDS Clock =   125.250000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.500000000, fRM =  26.839, fVCO = ( 3757.500000000,  3757.500000000,  3757.499982834) */

	BVDC_P_MAKE_HDMIRM(83_5MHz              , e24bit, eNone, eYCbCr422, 32732,  9604,         1, 0x07BB425E,     1,      80,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("83.500"),       0) /* TMDS Clock =    83.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.500000000, fRM =  20.875, fVCO = ( 3340.000000000,  3340.000000000,  3339.999979019) */
	BVDC_P_MAKE_HDMIRM(83_5MHz              , e30bit, eNone, eYCbCr422, 32732,  9604,         1, 0x07BB425E,     1,      80,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("83.500"),       0) /* TMDS Clock =    83.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.500000000, fRM =  20.875, fVCO = ( 3340.000000000,  3340.000000000,  3339.999979019) */
	BVDC_P_MAKE_HDMIRM(83_5MHz              , e36bit, eNone, eYCbCr422, 32732,  9604,         1, 0x07BB425E,     1,      80,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("83.500"),       0) /* TMDS Clock =    83.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.500000000, fRM =  20.875, fVCO = ( 3340.000000000,  3340.000000000,  3339.999979019) */

	BVDC_P_MAKE_HDMIRM(83_5MHz_DIV_1_001    , e24bit, eNone, eYCbCr444, 20875,  6152,         1, 0x07B9482F,     1,      80,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("83.417"),       0) /* TMDS Clock =    83.416583417, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.416583417, fRM =  20.896, fVCO = ( 3336.663336663,  3336.663336663,  3336.663319588) */
	BVDC_P_MAKE_HDMIRM(83_5MHz_DIV_1_001    , e30bit, eNone, eYCbCr444, 20875,  6152,         1, 0x09A79A3B,     2,     100,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("104.271"),       0) /* TMDS Clock =   104.270729271, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.416583417, fRM =  20.896, fVCO = ( 4170.829170829,  4170.829170829,  4170.829155922) */
	BVDC_P_MAKE_HDMIRM(83_5MHz_DIV_1_001    , e36bit, eNone, eYCbCr444, 20875,   146,         1, 0x08B07135,     2,      70,        3,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("125.125"),       0) /* TMDS Clock =   125.124875125, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.416583417, fRM =  26.866, fVCO = ( 3753.746253746,  3753.746253746,  3753.746237755) */

	BVDC_P_MAKE_HDMIRM(83_5MHz_DIV_1_001    , e24bit, eNone, eYCbCr422, 20875,  6152,         1, 0x07B9482F,     1,      80,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("83.417"),       0) /* TMDS Clock =    83.416583417, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.416583417, fRM =  20.896, fVCO = ( 3336.663336663,  3336.663336663,  3336.663319588) */
	BVDC_P_MAKE_HDMIRM(83_5MHz_DIV_1_001    , e30bit, eNone, eYCbCr422, 20875,  6152,         1, 0x07B9482F,     1,      80,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("83.417"),       0) /* TMDS Clock =    83.416583417, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.416583417, fRM =  20.896, fVCO = ( 3336.663336663,  3336.663336663,  3336.663319588) */
	BVDC_P_MAKE_HDMIRM(83_5MHz_DIV_1_001    , e36bit, eNone, eYCbCr422, 20875,  6152,         1, 0x07B9482F,     1,      80,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("83.417"),       0) /* TMDS Clock =    83.416583417, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    83.416583417, fRM =  20.896, fVCO = ( 3336.663336663,  3336.663336663,  3336.663319588) */

	BVDC_P_MAKE_HDMIRM(85_5MHz              , e24bit, eNone, eYCbCr444, 32680,   344,         1, 0x07EAAAAA,     1,      64,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("85.500"),       0) /* TMDS Clock =    85.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.500000000, fRM =  26.719, fVCO = ( 3420.000000000,  3420.000000000,  3419.999982834) */
	BVDC_P_MAKE_HDMIRM(85_5MHz              , e30bit, eNone, eYCbCr444, 32750, 31440,         0, 0x09E55555,     2,      76,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("106.875"),       0) /* TMDS Clock =   106.875000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.500000000, fRM =  28.125, fVCO = ( 4275.000000000,  4275.000000000,  4274.999991417) */
	BVDC_P_MAKE_HDMIRM(85_5MHz              , e36bit, eNone, eYCbCr444, 32718, 32144,         0, 0x08E80000,     2,      70,        3,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("128.250"),       0) /* TMDS Clock =   128.250000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.500000000, fRM =  27.482, fVCO = ( 3847.500000000,  3847.500000000,  3847.500000000) */

	BVDC_P_MAKE_HDMIRM(85_5MHz              , e24bit, eNone, eYCbCr422, 32680,   344,         1, 0x07EAAAAA,     1,      64,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("85.500"),       0) /* TMDS Clock =    85.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.500000000, fRM =  26.719, fVCO = ( 3420.000000000,  3420.000000000,  3419.999982834) */
	BVDC_P_MAKE_HDMIRM(85_5MHz              , e30bit, eNone, eYCbCr422, 32680,   344,         1, 0x07EAAAAA,     1,      64,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("85.500"),       0) /* TMDS Clock =    85.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.500000000, fRM =  26.719, fVCO = ( 3420.000000000,  3420.000000000,  3419.999982834) */
	BVDC_P_MAKE_HDMIRM(85_5MHz              , e36bit, eNone, eYCbCr422, 32680,   344,         1, 0x07EAAAAA,     1,      64,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("85.500"),       0) /* TMDS Clock =    85.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.500000000, fRM =  26.719, fVCO = ( 3420.000000000,  3420.000000000,  3419.999982834) */

	BVDC_P_MAKE_HDMIRM(85_5MHz_DIV_1_001    , e24bit, eNone, eYCbCr444, 23750,   274,         1, 0x07E8A45B,     1,      64,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("85.415"),       0) /* TMDS Clock =    85.414585415, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.414585415, fRM =  26.745, fVCO = ( 3416.583416583,  3416.583416583,  3416.583397865) */
	BVDC_P_MAKE_HDMIRM(85_5MHz_DIV_1_001    , e30bit, eNone, eYCbCr444, 31250, 30030,         0, 0x09E2CD72,     2,      76,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("106.768"),       0) /* TMDS Clock =   106.768231768, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.414585415, fRM =  28.153, fVCO = ( 4270.729270729,  4270.729270729,  4270.729253769) */
	BVDC_P_MAKE_HDMIRM(85_5MHz_DIV_1_001    , e36bit, eNone, eYCbCr444, 28500, 28028,         0, 0x08E5B8E7,     2,      70,        3,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("128.122"),       0) /* TMDS Clock =   128.121878122, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.414585415, fRM =  27.510, fVCO = ( 3843.656343656,  3843.656343656,  3843.656338692) */

	BVDC_P_MAKE_HDMIRM(85_5MHz_DIV_1_001    , e24bit, eNone, eYCbCr422, 23750,   274,         1, 0x07E8A45B,     1,      64,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("85.415"),       0) /* TMDS Clock =    85.414585415, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.414585415, fRM =  26.745, fVCO = ( 3416.583416583,  3416.583416583,  3416.583397865) */
	BVDC_P_MAKE_HDMIRM(85_5MHz_DIV_1_001    , e30bit, eNone, eYCbCr422, 23750,   274,         1, 0x07E8A45B,     1,      64,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("85.415"),       0) /* TMDS Clock =    85.414585415, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.414585415, fRM =  26.745, fVCO = ( 3416.583416583,  3416.583416583,  3416.583397865) */
	BVDC_P_MAKE_HDMIRM(85_5MHz_DIV_1_001    , e36bit, eNone, eYCbCr422, 23750,   274,         1, 0x07E8A45B,     1,      64,        4,    1,        1,       12,    28,    6,    3,    1,   4,  1,   BDBG_STRING("85.415"),       0) /* TMDS Clock =    85.414585415, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    85.414585415, fRM =  26.745, fVCO = ( 3416.583416583,  3416.583416583,  3416.583397865) */

	BVDC_P_MAKE_HDMIRM(106_5MHz             , e24bit, eNone, eYCbCr444, 32731,   461,         1, 0x09DC71C7,     2,      80,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("106.500"),       0) /* TMDS Clock =   106.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.500000000, fRM =  26.625, fVCO = ( 4260.000000000,  4260.000000000,  4259.999997139) */
	BVDC_P_MAKE_HDMIRM(106_5MHz             , e30bit, eNone, eYCbCr444, 32750, 31440,         0, 0x093EAAAA,     2,      71,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("133.125"),       0) /* TMDS Clock =   133.125000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.500000000, fRM =  28.125, fVCO = ( 3993.750000000,  3993.750000000,  3993.749982834) */
	BVDC_P_MAKE_HDMIRM(106_5MHz             , e36bit, eNone, eYCbCr444, 32731,   461,         1, 0x07655555,     1,      60,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("159.750"),       0) /* TMDS Clock =   159.750000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.500000000, fRM =  26.625, fVCO = ( 3195.000000000,  3195.000000000,  3194.999991417) */

	BVDC_P_MAKE_HDMIRM(106_5MHz             , e24bit, eNone, eYCbCr422, 32731,   461,         1, 0x09DC71C7,     2,      80,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("106.500"),       0) /* TMDS Clock =   106.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.500000000, fRM =  26.625, fVCO = ( 4260.000000000,  4260.000000000,  4259.999997139) */
	BVDC_P_MAKE_HDMIRM(106_5MHz             , e30bit, eNone, eYCbCr422, 32731,   461,         1, 0x09DC71C7,     2,      80,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("106.500"),       0) /* TMDS Clock =   106.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.500000000, fRM =  26.625, fVCO = ( 4260.000000000,  4260.000000000,  4259.999997139) */
	BVDC_P_MAKE_HDMIRM(106_5MHz             , e36bit, eNone, eYCbCr422, 32731,   461,         1, 0x09DC71C7,     2,      80,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("106.500"),       0) /* TMDS Clock =   106.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.500000000, fRM =  26.625, fVCO = ( 4260.000000000,  4260.000000000,  4259.999997139) */

	BVDC_P_MAKE_HDMIRM(106_5MHz_DIV_1_001   , e24bit, eNone, eYCbCr444, 26625,   402,         1, 0x09D9EC2A,     2,      80,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("106.394"),       0) /* TMDS Clock =   106.393606394, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.393606394, fRM =  26.652, fVCO = ( 4255.744255744,  4255.744255744,  4255.744245529) */
	BVDC_P_MAKE_HDMIRM(106_5MHz_DIV_1_001   , e30bit, eNone, eYCbCr444, 31250, 30030,         0, 0x093C4D67,     2,      71,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("132.992"),       0) /* TMDS Clock =   132.992007992, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.393606394, fRM =  28.153, fVCO = ( 3989.760239760,  3989.760239760,  3989.760220528) */
	BVDC_P_MAKE_HDMIRM(106_5MHz_DIV_1_001   , e36bit, eNone, eYCbCr444, 26625,   402,         1, 0x0763711F,     1,      60,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("159.590"),       0) /* TMDS Clock =   159.590409590, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.393606394, fRM =  26.652, fVCO = ( 3191.808191808,  3191.808191808,  3191.808171272) */

	BVDC_P_MAKE_HDMIRM(106_5MHz_DIV_1_001   , e24bit, eNone, eYCbCr422, 26625,   402,         1, 0x09D9EC2A,     2,      80,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("106.394"),       0) /* TMDS Clock =   106.393606394, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.393606394, fRM =  26.652, fVCO = ( 4255.744255744,  4255.744255744,  4255.744245529) */
	BVDC_P_MAKE_HDMIRM(106_5MHz_DIV_1_001   , e30bit, eNone, eYCbCr422, 26625,   402,         1, 0x09D9EC2A,     2,      80,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("106.394"),       0) /* TMDS Clock =   106.393606394, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.393606394, fRM =  26.652, fVCO = ( 4255.744255744,  4255.744255744,  4255.744245529) */
	BVDC_P_MAKE_HDMIRM(106_5MHz_DIV_1_001   , e36bit, eNone, eYCbCr422, 26625,   402,         1, 0x09D9EC2A,     2,      80,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("106.394"),       0) /* TMDS Clock =   106.393606394, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   106.393606394, fRM =  26.652, fVCO = ( 4255.744255744,  4255.744255744,  4255.744245529) */

	BVDC_P_MAKE_HDMIRM(108MHz               , e24bit, eNone, eYCbCr444, 32760, 31668,         0, 0x07800000,     1,      58,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.000"),       0) /* TMDS Clock =   108.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(108MHz               , e30bit, eNone, eYCbCr444, 32767,     0,         1, 0x09600000,     2,      75,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("135.000"),       0) /* TMDS Clock =   135.000000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(108MHz               , e36bit, eNone, eYCbCr444, 32760, 31668,         0, 0x07800000,     1,      58,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.000"),       0) /* TMDS Clock =   162.000000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */

	BVDC_P_MAKE_HDMIRM(108MHz               , e24bit, eNone, eYCbCr422, 32760, 31668,         0, 0x07800000,     1,      58,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.000"),       0) /* TMDS Clock =   108.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(108MHz               , e30bit, eNone, eYCbCr422, 32760, 31668,         0, 0x07800000,     1,      58,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.000"),       0) /* TMDS Clock =   108.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(108MHz               , e36bit, eNone, eYCbCr422, 32760, 31668,         0, 0x07800000,     1,      58,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("108.000"),       0) /* TMDS Clock =   108.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   108.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */

	BVDC_P_MAKE_HDMIRM(108MHz_DIV_1_001     , e24bit, eNone, eYCbCr444, 30000, 29029,         0, 0x077E14F8,     1,      58,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("107.892"),       0) /* TMDS Clock =   107.892107892, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   107.892107892, fRM =  27.959, fVCO = ( 3236.763236763,  3236.763236763,  3236.763221741) */
	BVDC_P_MAKE_HDMIRM(108MHz_DIV_1_001     , e30bit, eNone, eYCbCr444, 32000,    32,         1, 0x095D9A36,     2,      75,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("134.865"),       0) /* TMDS Clock =   134.865134865, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   107.892107892, fRM =  27.027, fVCO = ( 4045.954045954,  4045.954045954,  4045.954027176) */
	BVDC_P_MAKE_HDMIRM(108MHz_DIV_1_001     , e36bit, eNone, eYCbCr444, 30000, 29029,         0, 0x077E14F8,     1,      58,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("161.838"),       0) /* TMDS Clock =   161.838161838, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   107.892107892, fRM =  27.959, fVCO = ( 3236.763236763,  3236.763236763,  3236.763221741) */

	BVDC_P_MAKE_HDMIRM(108MHz_DIV_1_001     , e24bit, eNone, eYCbCr422, 30000, 29029,         0, 0x077E14F8,     1,      58,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("107.892"),       0) /* TMDS Clock =   107.892107892, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   107.892107892, fRM =  27.959, fVCO = ( 3236.763236763,  3236.763236763,  3236.763221741) */
	BVDC_P_MAKE_HDMIRM(108MHz_DIV_1_001     , e30bit, eNone, eYCbCr422, 30000, 29029,         0, 0x077E14F8,     1,      58,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("107.892"),       0) /* TMDS Clock =   107.892107892, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   107.892107892, fRM =  27.959, fVCO = ( 3236.763236763,  3236.763236763,  3236.763221741) */
	BVDC_P_MAKE_HDMIRM(108MHz_DIV_1_001     , e36bit, eNone, eYCbCr422, 30000, 29029,         0, 0x077E14F8,     1,      58,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("107.892"),       0) /* TMDS Clock =   107.892107892, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   107.892107892, fRM =  27.959, fVCO = ( 3236.763236763,  3236.763236763,  3236.763221741) */

	BVDC_P_MAKE_HDMIRM(148_5MHz             , e24bit, eNone, eYCbCr444, 32725, 32130,         0, 0x0A500000,     2,      81,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("148.500"),       0) /* TMDS Clock =   148.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.500000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */
	BVDC_P_MAKE_HDMIRM(148_5MHz             , e30bit, eNone, eYCbCr444, 32725, 32368,         0, 0x08980000,     2,      68,        2,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("185.625"),       0) /* TMDS Clock =   185.625000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.500000000, fRM =  27.298, fVCO = ( 3712.500000000,  3712.500000000,  3712.500000000) */
	BVDC_P_MAKE_HDMIRM(148_5MHz             , e36bit, eNone, eYCbCr444, 32725, 32130,         0, 0x0A500000,     2,      81,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("222.750"),       0) /* TMDS Clock =   222.750000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.500000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */

	BVDC_P_MAKE_HDMIRM(148_5MHz             , e24bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0A500000,     2,      81,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("148.500"),       0) /* TMDS Clock =   148.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.500000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */
	BVDC_P_MAKE_HDMIRM(148_5MHz             , e30bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0A500000,     2,      81,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("148.500"),       0) /* TMDS Clock =   148.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.500000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */
	BVDC_P_MAKE_HDMIRM(148_5MHz             , e36bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0A500000,     2,      81,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("148.500"),       0) /* TMDS Clock =   148.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.500000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */

	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001   , e24bit, eNone, eYCbCr444, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("148.352"),       0) /* TMDS Clock =   148.351648352, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.351648352, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */
	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001   , e30bit, eNone, eYCbCr444, 31250, 30940,         0, 0x0895CD5C,     2,      68,        2,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("185.440"),       0) /* TMDS Clock =   185.439560440, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.351648352, fRM =  27.325, fVCO = ( 3708.791208791,  3708.791208791,  3708.791187286) */
	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001   , e36bit, eNone, eYCbCr444, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("222.527"),       0) /* TMDS Clock =   222.527472527, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.351648352, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */

	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001   , e24bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("148.352"),       0) /* TMDS Clock =   148.351648352, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.351648352, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */
	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001   , e30bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("148.352"),       0) /* TMDS Clock =   148.351648352, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.351648352, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */
	BVDC_P_MAKE_HDMIRM(148_5MHz_DIV_1_001   , e36bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        3,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("148.352"),       0) /* TMDS Clock =   148.351648352, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   148.351648352, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */

	BVDC_P_MAKE_HDMIRM(162MHz               , e24bit, eNone, eYCbCr444, 32760, 31668,         0, 0x07800000,     1,      58,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.000"),       0) /* TMDS Clock =   162.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(162MHz               , e30bit, eNone, eYCbCr444, 32767,     0,         1, 0x09600000,     2,      75,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("202.500"),       0) /* TMDS Clock =   202.500000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(162MHz               , e36bit, eNone, eYCbCr444, 32760, 32032,         0, 0x0B400000,     2,      88,        2,    1,        1,        7,    24,    6,    3,    1,   4,  1,  BDBG_STRING("243.000"),       1) /* TMDS Clock =   243.000000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.614, fVCO = ( 4860.000000000,  4860.000000000,  4860.000000000) */

	BVDC_P_MAKE_HDMIRM(162MHz               , e24bit, eNone, eYCbCr422, 32760, 31668,         0, 0x07800000,     1,      58,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.000"),       0) /* TMDS Clock =   162.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(162MHz               , e30bit, eNone, eYCbCr422, 32760, 31668,         0, 0x07800000,     1,      58,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.000"),       0) /* TMDS Clock =   162.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(162MHz               , e36bit, eNone, eYCbCr422, 32760, 31668,         0, 0x07800000,     1,      58,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.000"),       0) /* TMDS Clock =   162.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */

	BVDC_P_MAKE_HDMIRM(297MHz               , e24bit, eNone, eYCbCr444, 32725, 32130,         0, 0x0DC00000,     2,     108,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("297.000"),       1) /* TMDS Clock =   297.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   297.000000000, fRM =  27.500, fVCO = ( 5940.000000000,  5940.000000000,  5940.000000000) */
	BVDC_P_MAKE_HDMIRM(297MHz               , e30bit, eNone, eYCbCr444, 32725, 32368,         0, 0x08980000,     2,      68,        1,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("371.250"),       0) /* TMDS Clock =   371.250000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   297.000000000, fRM =  27.298, fVCO = ( 3712.500000000,  3712.500000000,  3712.500000000) */
	BVDC_P_MAKE_HDMIRM(297MHz               , e36bit, eNone, eYCbCr444, 32725, 32130,         0, 0x0A500000,     2,      81,        1,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("445.500"),       0) /* TMDS Clock =   445.500000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   297.000000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */

	BVDC_P_MAKE_HDMIRM(297MHz               , e24bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0DC00000,     2,     108,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("297.000"),       1) /* TMDS Clock =   297.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   297.000000000, fRM =  27.500, fVCO = ( 5940.000000000,  5940.000000000,  5940.000000000) */
	BVDC_P_MAKE_HDMIRM(297MHz               , e30bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0DC00000,     2,     108,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("297.000"),       1) /* TMDS Clock =   297.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   297.000000000, fRM =  27.500, fVCO = ( 5940.000000000,  5940.000000000,  5940.000000000) */
	BVDC_P_MAKE_HDMIRM(297MHz               , e36bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0DC00000,     2,     108,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("297.000"),       1) /* TMDS Clock =   297.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   297.000000000, fRM =  27.500, fVCO = ( 5940.000000000,  5940.000000000,  5940.000000000) */

	BVDC_P_MAKE_HDMIRM(297MHz_DIV_1_001     , e24bit, eNone, eYCbCr444, 32500, 31941,         0, 0x0DBC7BC7,     2,     108,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("296.703"),       1) /* TMDS Clock =   296.703296703, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   296.703296703, fRM =  27.528, fVCO = ( 5934.065934066,  5934.065934066,  5934.065915108) */
	BVDC_P_MAKE_HDMIRM(297MHz_DIV_1_001     , e30bit, eNone, eYCbCr444, 31250, 30940,         0, 0x0895CD5C,     2,      68,        1,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("370.879"),       0) /* TMDS Clock =   370.879120879, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   296.703296703, fRM =  27.325, fVCO = ( 3708.791208791,  3708.791208791,  3708.791187286) */
	BVDC_P_MAKE_HDMIRM(297MHz_DIV_1_001     , e36bit, eNone, eYCbCr444, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        1,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("445.055"),       0) /* TMDS Clock =   445.054945055, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   296.703296703, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */

	BVDC_P_MAKE_HDMIRM(297MHz_DIV_1_001     , e24bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0DBC7BC7,     2,     108,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("296.703"),       1) /* TMDS Clock =   296.703296703, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   296.703296703, fRM =  27.528, fVCO = ( 5934.065934066,  5934.065934066,  5934.065915108) */
	BVDC_P_MAKE_HDMIRM(297MHz_DIV_1_001     , e30bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0DBC7BC7,     2,     108,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("296.703"),       1) /* TMDS Clock =   296.703296703, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   296.703296703, fRM =  27.528, fVCO = ( 5934.065934066,  5934.065934066,  5934.065915108) */
	BVDC_P_MAKE_HDMIRM(297MHz_DIV_1_001     , e36bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0DBC7BC7,     2,     108,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("296.703"),       1) /* TMDS Clock =   296.703296703, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   296.703296703, fRM =  27.528, fVCO = ( 5934.065934066,  5934.065934066,  5934.065915108) */

	BVDC_P_MAKE_HDMIRM(594MHz               , e24bit, eNone, eYCbCr444, 32725, 32130,         0, 0x0DC00000,     2,     108,        1,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("594.000"),       1) /* TMDS Clock =   594.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   594.000000000, fRM =  27.500, fVCO = ( 5940.000000000,  5940.000000000,  5940.000000000) */

	BVDC_P_MAKE_HDMIRM(594MHz               , e24bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0DC00000,     2,     108,        1,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("594.000"),       1) /* TMDS Clock =   594.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   594.000000000, fRM =  27.500, fVCO = ( 5940.000000000,  5940.000000000,  5940.000000000) */
	BVDC_P_MAKE_HDMIRM(594MHz               , e30bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0DC00000,     2,     108,        1,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("594.000"),       1) /* TMDS Clock =   594.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   594.000000000, fRM =  27.500, fVCO = ( 5940.000000000,  5940.000000000,  5940.000000000) */
	BVDC_P_MAKE_HDMIRM(594MHz               , e36bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0DC00000,     2,     108,        1,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("594.000"),       1) /* TMDS Clock =   594.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   594.000000000, fRM =  27.500, fVCO = ( 5940.000000000,  5940.000000000,  5940.000000000) */

	BVDC_P_MAKE_HDMIRM(594MHz               , e24bit, eNone, eYCbCr420, 32725, 32130,         0, 0x0DC00000,     2,     108,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("297.000"),       1) /* TMDS Clock =   297.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   297.000000000, fRM =  27.500, fVCO = ( 5940.000000000,  5940.000000000,  5940.000000000) */
	BVDC_P_MAKE_HDMIRM(594MHz               , e30bit, eNone, eYCbCr420, 32725, 32368,         0, 0x08980000,     2,      68,        1,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("371.250"),       0) /* TMDS Clock =   371.250000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   297.000000000, fRM =  27.298, fVCO = ( 3712.500000000,  3712.500000000,  3712.500000000) */
	BVDC_P_MAKE_HDMIRM(594MHz               , e36bit, eNone, eYCbCr420, 32725, 32130,         0, 0x0A500000,     2,      81,        1,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("445.500"),       0) /* TMDS Clock =   445.500000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   297.000000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */

	BVDC_P_MAKE_HDMIRM(594MHz_DIV_1_001     , e24bit, eNone, eYCbCr444, 32500, 31941,         0, 0x0DBC7BC7,     2,     108,        1,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("593.407"),       1) /* TMDS Clock =   593.406593407, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   593.406593407, fRM =  27.528, fVCO = ( 5934.065934066,  5934.065934066,  5934.065915108) */

	BVDC_P_MAKE_HDMIRM(594MHz_DIV_1_001     , e24bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0DBC7BC7,     2,     108,        1,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("593.407"),       1) /* TMDS Clock =   593.406593407, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   593.406593407, fRM =  27.528, fVCO = ( 5934.065934066,  5934.065934066,  5934.065915108) */
	BVDC_P_MAKE_HDMIRM(594MHz_DIV_1_001     , e30bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0DBC7BC7,     2,     108,        1,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("593.407"),       1) /* TMDS Clock =   593.406593407, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   593.406593407, fRM =  27.528, fVCO = ( 5934.065934066,  5934.065934066,  5934.065915108) */
	BVDC_P_MAKE_HDMIRM(594MHz_DIV_1_001     , e36bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0DBC7BC7,     2,     108,        1,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("593.407"),       1) /* TMDS Clock =   593.406593407, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   593.406593407, fRM =  27.528, fVCO = ( 5934.065934066,  5934.065934066,  5934.065915108) */

	BVDC_P_MAKE_HDMIRM(594MHz_DIV_1_001     , e24bit, eNone, eYCbCr420, 32500, 31941,         0, 0x0DBC7BC7,     2,     108,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("296.703"),       1) /* TMDS Clock =   296.703296703, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   296.703296703, fRM =  27.528, fVCO = ( 5934.065934066,  5934.065934066,  5934.065915108) */
	BVDC_P_MAKE_HDMIRM(594MHz_DIV_1_001     , e30bit, eNone, eYCbCr420, 31250, 30940,         0, 0x0895CD5C,     2,      68,        1,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("370.879"),       0) /* TMDS Clock =   370.879120879, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   296.703296703, fRM =  27.325, fVCO = ( 3708.791208791,  3708.791208791,  3708.791187286) */
	BVDC_P_MAKE_HDMIRM(594MHz_DIV_1_001     , e36bit, eNone, eYCbCr420, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        1,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("445.055"),       0) /* TMDS Clock =   445.054945055, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   296.703296703, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */

	/* Below is for MHL */
	BVDC_P_MAKE_HDMIRM(75_5_MHz             , e24bit, eNone, eYCbCr444, 32767, 14105,         1, 0x08BD097B,     2,     100,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("75.500"),       0) /* TMDS Clock =    75.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.500000000, fRM =  18.875, fVCO = ( 3775.000000000,  3775.000000000,  3774.999993324) */
	BVDC_P_MAKE_HDMIRM(75_5_MHz             , e30bit, eNone, eYCbCr444, 32767, 14105,         1, 0x08BD097B,     2,     100,        4,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("94.375"),       0) /* TMDS Clock =    94.375000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.500000000, fRM =  18.875, fVCO = ( 3775.000000000,  3775.000000000,  3774.999993324) */
	BVDC_P_MAKE_HDMIRM(75_5_MHz             , e36bit, eNone, eYCbCr444, 32767, 31248,         0, 0x07DD5555,     1,      60,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("113.250"),       0) /* TMDS Clock =   113.250000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.500000000, fRM =  28.313, fVCO = ( 3397.500000000,  3397.500000000,  3397.499991417) */

	BVDC_P_MAKE_HDMIRM(75_5_MHz             , e24bit, eNone, eYCbCr422, 32767, 14105,         1, 0x08BD097B,     2,     100,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("75.500"),       0) /* TMDS Clock =    75.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.500000000, fRM =  18.875, fVCO = ( 3775.000000000,  3775.000000000,  3774.999993324) */
	BVDC_P_MAKE_HDMIRM(75_5_MHz             , e30bit, eNone, eYCbCr422, 32767, 14105,         1, 0x08BD097B,     2,     100,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("75.500"),       0) /* TMDS Clock =    75.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.500000000, fRM =  18.875, fVCO = ( 3775.000000000,  3775.000000000,  3774.999993324) */
	BVDC_P_MAKE_HDMIRM(75_5_MHz             , e36bit, eNone, eYCbCr422, 32767, 14105,         1, 0x08BD097B,     2,     100,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("75.500"),       0) /* TMDS Clock =    75.500000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.500000000, fRM =  18.875, fVCO = ( 3775.000000000,  3775.000000000,  3774.999993324) */

	BVDC_P_MAKE_HDMIRM(75_5_MHz_DIV_1_001   , e24bit, eNone, eYCbCr444, 18875,  8152,         1, 0x08BACD5F,     2,     100,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("75.425"),       0) /* TMDS Clock =    75.424575425, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.424575425, fRM =  18.894, fVCO = ( 3771.228771229,  3771.228771229,  3771.228764534) */
	BVDC_P_MAKE_HDMIRM(75_5_MHz_DIV_1_001   , e30bit, eNone, eYCbCr444, 18875,  8152,         1, 0x08BACD5F,     2,     100,        4,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("94.281"),       0) /* TMDS Clock =    94.280719281, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.424575425, fRM =  18.894, fVCO = ( 3771.228771229,  3771.228771229,  3771.228764534) */
	BVDC_P_MAKE_HDMIRM(75_5_MHz_DIV_1_001   , e36bit, eNone, eYCbCr444, 18875, 18018,         0, 0x07DB526F,     1,      60,        3,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("113.137"),       0) /* TMDS Clock =   113.136863137, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.424575425, fRM =  28.341, fVCO = ( 3394.105894106,  3394.105894106,  3394.105885506) */

	BVDC_P_MAKE_HDMIRM(75_5_MHz_DIV_1_001   , e24bit, eNone, eYCbCr422, 18875,  8152,         1, 0x08BACD5F,     2,     100,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("75.425"),       0) /* TMDS Clock =    75.424575425, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.424575425, fRM =  18.894, fVCO = ( 3771.228771229,  3771.228771229,  3771.228764534) */
	BVDC_P_MAKE_HDMIRM(75_5_MHz_DIV_1_001   , e30bit, eNone, eYCbCr422, 18875,  8152,         1, 0x08BACD5F,     2,     100,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("75.425"),       0) /* TMDS Clock =    75.424575425, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.424575425, fRM =  18.894, fVCO = ( 3771.228771229,  3771.228771229,  3771.228764534) */
	BVDC_P_MAKE_HDMIRM(75_5_MHz_DIV_1_001   , e36bit, eNone, eYCbCr422, 18875,  8152,         1, 0x08BACD5F,     2,     100,        5,    1,        1,        6,    24,    6,    3,    1,   4,  1,   BDBG_STRING("75.425"),       0) /* TMDS Clock =    75.424575425, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    75.424575425, fRM =  18.894, fVCO = ( 3771.228771229,  3771.228771229,  3771.228764534) */

	BVDC_P_MAKE_HDMIRM(81_0MHz              , e24bit, eNone, eYCbCr444, 32767,     0,         1, 0x09600000,     2,      75,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.000"),       0) /* TMDS Clock =    81.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(81_0MHz              , e30bit, eNone, eYCbCr444, 32767,     0,         1, 0x09600000,     2,      75,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("101.250"),       0) /* TMDS Clock =   101.250000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(81_0MHz              , e36bit, eNone, eYCbCr444, 32760, 32032,         0, 0x08700000,     2,      66,        3,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("121.500"),       0) /* TMDS Clock =   121.500000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.000000000, fRM =  27.614, fVCO = ( 3645.000000000,  3645.000000000,  3645.000000000) */

	BVDC_P_MAKE_HDMIRM(81_0MHz              , e24bit, eNone, eYCbCr422, 32767,     0,         1, 0x09600000,     2,      75,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.000"),       0) /* TMDS Clock =    81.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(81_0MHz              , e30bit, eNone, eYCbCr422, 32767,     0,         1, 0x09600000,     2,      75,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.000"),       0) /* TMDS Clock =    81.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(81_0MHz              , e36bit, eNone, eYCbCr422, 32767,     0,         1, 0x09600000,     2,      75,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.000"),       0) /* TMDS Clock =    81.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */

	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e24bit, eNone, eYCbCr444, 30030, 29600,         0, 0x09626666,     2,      74,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.081"),       0) /* TMDS Clock =    81.081000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.081000000, fRM =  27.338, fVCO = ( 4054.050000000,  4054.050000000,  4054.049989700) */
	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e30bit, eNone, eYCbCr444, 30030, 29600,         0, 0x09626666,     2,      74,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("101.351"),       0) /* TMDS Clock =   101.351250000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.081000000, fRM =  27.338, fVCO = ( 4054.050000000,  4054.050000000,  4054.049989700) */
	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e36bit, eNone, eYCbCr444, 32760, 32000,         0, 0x087228F5,     2,      66,        3,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("121.622"),       0) /* TMDS Clock =   121.621500000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.081000000, fRM =  27.586, fVCO = ( 3648.645000000,  3648.645000000,  3648.644980431) */

	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e24bit, eNone, eYCbCr422, 30030, 29600,         0, 0x09626666,     2,      74,        5,    1,        1,        5,    24,    6,    3,    1,   4,  1,   BDBG_STRING("81.081"),       0) /* TMDS Clock =    81.081000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.081000000, fRM =  27.338, fVCO = ( 4054.050000000,  4054.050000000,  4054.049989700) */
	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e30bit, eNone, eYCbCr422, 30030, 29600,         0, 0x09626666,     2,      74,        4,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("101.351"),       0) /* TMDS Clock =   101.351250000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.081000000, fRM =  27.338, fVCO = ( 4054.050000000,  4054.050000000,  4054.049989700) */
	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e36bit, eNone, eYCbCr422, 32760, 32000,         0, 0x087228F5,     2,      66,        3,    1,        1,        6,    24,    6,    3,    1,   4,  1,  BDBG_STRING("121.622"),       0) /* TMDS Clock =   121.621500000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =    81.081000000, fRM =  27.586, fVCO = ( 3648.645000000,  3648.645000000,  3648.644980431) */

	/* New 162MHz entries but renamed to 81 for 2x pixel repetition */
	BVDC_P_MAKE_HDMIRM(81_0MHz              , e24bit, e2x  , eYCbCr444, 32760, 31668,         0, 0x07800000,     1,      58,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.000"),       0) /* TMDS Clock =   162.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(81_0MHz              , e30bit, e2x  , eYCbCr444, 32767,     0,         1, 0x09600000,     2,      75,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("202.500"),       0) /* TMDS Clock =   202.500000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(81_0MHz              , e36bit, e2x  , eYCbCr444, 32760, 32032,         0, 0x0B400000,     2,      88,        2,    1,        1,        7,    24,    6,    3,    1,   4,  1,  BDBG_STRING("243.000"),       1) /* TMDS Clock =   243.000000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.614, fVCO = ( 4860.000000000,  4860.000000000,  4860.000000000) */

	BVDC_P_MAKE_HDMIRM(81_0MHz              , e24bit, e2x  , eYCbCr422, 32760, 31668,         0, 0x07800000,     1,      58,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.000"),       0) /* TMDS Clock =   162.000000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.931, fVCO = ( 3240.000000000,  3240.000000000,  3240.000000000) */
	BVDC_P_MAKE_HDMIRM(81_0MHz              , e30bit, e2x  , eYCbCr422, 32767,     0,         1, 0x09600000,     2,      75,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("202.500"),       0) /* TMDS Clock =   202.500000000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.000, fVCO = ( 4050.000000000,  4050.000000000,  4050.000000000) */
	BVDC_P_MAKE_HDMIRM(81_0MHz              , e36bit, e2x  , eYCbCr422, 32760, 32032,         0, 0x0B400000,     2,      88,        2,    1,        1,        7,    24,    6,    3,    1,   4,  1,  BDBG_STRING("243.000"),       1) /* TMDS Clock =   243.000000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.000000000, fRM =  27.614, fVCO = ( 4860.000000000,  4860.000000000,  4860.000000000) */

	/* New 162MHz*1.001 entries renamed to 81*1.001 for 2x pixel repetition */
	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e24bit, e2x  , eYCbCr444, 30030, 29500,         0, 0x0781EB85,     1,      59,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.162"),       0) /* TMDS Clock =   162.162000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.162000000, fRM =  27.430, fVCO = ( 3243.240000000,  3243.240000000,  3243.239996910) */
	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e30bit, e2x  , eYCbCr444, 30030, 29600,         0, 0x09626666,     2,      74,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("202.702"),       0) /* TMDS Clock =   202.702500000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.162000000, fRM =  27.338, fVCO = ( 4054.050000000,  4054.050000000,  4054.049989700) */
	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e36bit, e2x  , eYCbCr444, 32760, 32000,         0, 0x0B42E147,     2,      88,        2,    1,        1,        7,    24,    6,    3,    1,   4,  1,  BDBG_STRING("243.243"),       1) /* TMDS Clock =   243.243000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.162000000, fRM =  27.586, fVCO = ( 4864.860000000,  4864.860000000,  4864.859982491) */

	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e24bit, e2x  , eYCbCr422, 30030, 29500,         0, 0x0781EB85,     1,      59,        2,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("162.162"),       0) /* TMDS Clock =   162.162000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.162000000, fRM =  27.430, fVCO = ( 3243.240000000,  3243.240000000,  3243.239996910) */
	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e30bit, e2x  , eYCbCr422, 30030, 29600,         0, 0x09626666,     2,      74,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("202.702"),       0) /* TMDS Clock =   202.702500000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.162000000, fRM =  27.338, fVCO = ( 4054.050000000,  4054.050000000,  4054.049989700) */
	BVDC_P_MAKE_HDMIRM(81_0MHz_MUL_1_001    , e36bit, e2x  , eYCbCr422, 32760, 32000,         0, 0x0B42E147,     2,      88,        2,    1,        1,        7,    24,    6,    3,    1,   4,  1,  BDBG_STRING("243.243"),       1) /* TMDS Clock =   243.243000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   162.162000000, fRM =  27.586, fVCO = ( 4864.860000000,  4864.860000000,  4864.859982491) */

	BVDC_P_MAKE_HDMIRM(222_75MHz            , e24bit, eNone, eYCbCr444, 32725, 32130,         0, 0x0A500000,     2,      81,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("222.750"),       0) /* TMDS Clock =   222.750000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.750000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */
	BVDC_P_MAKE_HDMIRM(222_75MHz            , e30bit, eNone, eYCbCr444, 32175, 31512,         0, 0x0CE40000,     2,     101,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("278.438"),       1) /* TMDS Clock =   278.437500000, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.750000000, fRM =  27.568, fVCO = ( 5568.750000000,  5568.750000000,  5568.750000000) */
	BVDC_P_MAKE_HDMIRM(222_75MHz            , e36bit, eNone, eYCbCr444, 32670, 32208,         0, 0x07BC0000,     1,      61,        1,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("334.125"),       0) /* TMDS Clock =   334.125000000, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.750000000, fRM =  27.387, fVCO = ( 3341.250000000,  3341.250000000,  3341.250000000) */

	BVDC_P_MAKE_HDMIRM(222_75MHz            , e24bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0A500000,     2,      81,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("222.750"),       0) /* TMDS Clock =   222.750000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.750000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */
	BVDC_P_MAKE_HDMIRM(222_75MHz            , e30bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0A500000,     2,      81,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("222.750"),       0) /* TMDS Clock =   222.750000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.750000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */
	BVDC_P_MAKE_HDMIRM(222_75MHz            , e36bit, eNone, eYCbCr422, 32725, 32130,         0, 0x0A500000,     2,      81,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("222.750"),       0) /* TMDS Clock =   222.750000000, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.750000000, fRM =  27.500, fVCO = ( 4455.000000000,  4455.000000000,  4455.000000000) */

	BVDC_P_MAKE_HDMIRM(222_75MHz_DIV_1_001  , e24bit, eNone, eYCbCr444, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("222.527"),       0) /* TMDS Clock =   222.527472527, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.527472527, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */
	BVDC_P_MAKE_HDMIRM(222_75MHz_DIV_1_001  , e30bit, eNone, eYCbCr444, 28125, 27573,         0, 0x0CE0B40B,     2,     101,        2,    1,        1,        2,    24,    6,    3,    1,   4,  1,  BDBG_STRING("278.159"),       1) /* TMDS Clock =   278.159340659, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.527472527, fRM =  27.596, fVCO = ( 5563.186813187,  5563.186813187,  5563.186806679) */
	BVDC_P_MAKE_HDMIRM(222_75MHz_DIV_1_001  , e36bit, eNone, eYCbCr444, 28125, 27755,         0, 0x07BA05A0,     1,      61,        1,    1,        1,       12,    28,    6,    3,    1,   4,  1,  BDBG_STRING("333.791"),       0) /* TMDS Clock =   333.791208791, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.527472527, fRM =  27.415, fVCO = ( 3337.912087912,  3337.912087912,  3337.912078857) */

	BVDC_P_MAKE_HDMIRM(222_75MHz_DIV_1_001  , e24bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("222.527"),       0) /* TMDS Clock =   222.527472527, CD = 24, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.527472527, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */
	BVDC_P_MAKE_HDMIRM(222_75MHz_DIV_1_001  , e30bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("222.527"),       0) /* TMDS Clock =   222.527472527, CD = 30, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.527472527, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */
	BVDC_P_MAKE_HDMIRM(222_75MHz_DIV_1_001  , e36bit, eNone, eYCbCr422, 32500, 31941,         0, 0x0A4D5CD5,     2,      81,        2,    1,        1,        5,    24,    6,    3,    1,   4,  1,  BDBG_STRING("222.527"),       0) /* TMDS Clock =   222.527472527, CD = 36, PR =  0,  4:2:0 = FALSE, Pixel Clock =   222.527472527, fRM =  27.528, fVCO = ( 4450.549450549,  4450.549450549,  4450.549429893) */


#else
	/* Mask                        M   P    N     RDiv  Sample   Num   Denom   OFFSET   BHDM_PixelClock */
	BVDC_P_MAKE_HDMIRM(25_2MHz,             0,  5,   28,   210,     7,     1,      2,      0, BDBG_STRING("25.20")) /* 25.200000 MHz */
	BVDC_P_MAKE_HDMIRM(25_2MHz_DIV_1_001,   0,  5,   28,   210,     7,   203,    400,  15860, BDBG_STRING("25.17")) /* 25.174825 MHz */

	BVDC_P_MAKE_HDMIRM(27MHz,               0,  2,   15,   105,     7,     0,      1,      0, BDBG_STRING("27.00")) /* 27.000000 MHz */
	BVDC_P_MAKE_HDMIRM(27MHz_MUL_1_001,     0,  2,   15,   105,     6,   142,    143,    523, BDBG_STRING("27.02")) /* 27.027000 MHz */

	BVDC_P_MAKE_HDMIRM(74_25MHz,            1,  1,   55,    70,     2,     6,     11,      0, BDBG_STRING("74.25")) /* 74.250000 MHz */
	BVDC_P_MAKE_HDMIRM(74_25MHz_DIV_1_001,  1,  1,   55,    70,     2,   137,    250,  15860, BDBG_STRING("74.17")) /* 74.175824 MHz */

	BVDC_P_MAKE_HDMIRM(56_304MHz,           3,  1,   83,    70,     3,   279,    782,   2597, BDBG_STRING("56.30")) /* 56.304000 MHz 1440x782 @ 50Hz */
	BVDC_P_MAKE_HDMIRM(67_565MHz_DIV_1_001, 0,  1,   25,    70,     2, 10011,  12512,  16364, BDBG_STRING("67.49")) /* 67.497303 MHz 1440x782 @ 59.94Hz*/
	BVDC_P_MAKE_HDMIRM(67_565MHz,           0,  1,   25,    70,     2,  1247,   1564,    502, BDBG_STRING("67.56")) /* 67.564800 MHz 1440x782 @ 60Hz */

	/* 60/50 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz,            1,  3,   59,   140,     4, 10361,  13816,  15924, BDBG_STRING("39.79")) /* 39.790080 MHz */
	BVDC_P_MAKE_HDMIRM(65MHz,               0,  1,   24,    70,     2,  2927,   3224,   1579, BDBG_STRING("65.00")) /* 64.995840 MHz */
	BVDC_P_MAKE_HDMIRM(65_286MHz,           3,  1,   97,    70,     2,  1082,   1209,  14867, BDBG_STRING("65.28")) /* 65.286000 MHz */
	BVDC_P_MAKE_HDMIRM(60_4656MHz,          2,  1,   67,    70,     3,   528,   4199,   1435, BDBG_STRING("60.47")) /* 60.465600 MHz */
	BVDC_P_MAKE_HDMIRM(64_022MHz,           3,  1,   95,    70,     2,  1411,   1482,  15544, BDBG_STRING("64.02"))  /* 64.022400 MHz */

	/* 59.94 Hz VESA formats */
	BVDC_P_MAKE_HDMIRM(39_79MHz_DIV_1_001,  1,  3,   59,   140,     4,  7583,  10048,  15400, BDBG_STRING("39.79/1.001")) /* 39.750329 MHz */
	BVDC_P_MAKE_HDMIRM(65MHz_DIV_1_001,     0,  1,   24,    70,     2,  1807,   1984,   1057, BDBG_STRING("65.00/1.001")) /* 64.930909 MHz */
	BVDC_P_MAKE_HDMIRM(65_286MHz_DIV_1_001, 3,  1,   97,    70,     2,   167,    186,  14341, BDBG_STRING("65.28/1.001")) /* 65.220779 MHz */
	BVDC_P_MAKE_HDMIRM(64_022MHz_DIV_1_001, 2,  1,   71,    70,     2,   871,    912,    479, BDBG_STRING("64.02/1.001"))  /* 63.958441 MHz */
#endif
};

/* Number of entries in the above table! */
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
	BVDC_P_RM_FIELD_DATA(RATE_RATIO, DENOMINATOR,	  denom),                      \
	BVDC_P_RM_FIELD_DATA(SAMPLE_INC, NUMERATOR,			num)                     | \
	BVDC_P_RM_FIELD_DATA(SAMPLE_INC, SAMPLE_INC,	 sample),                      \
	BVDC_P_RM_FIELD_DATA(PHASE_INC,  reserved0,			  0)                     | \
	BVDC_P_RM_FIELD_DATA(PHASE_INC,  PHASE_INC,		  phase),                      \
	BVDC_P_RM_FIELD_DATA(INTEGRATOR_LO, INTEGRATOR_LO, integrator_lo),             \
	BVDC_P_RM_FIELD_DATA(INTEGRATOR_HI, INTEGRATOR_HI, integrator_hi)

#define BVDC_P_MAKE_RM(denom, num, sample, phase, integrator_lo)  \
	BVDC_P_MAKE_RMHL(denom, num, sample, phase, integrator_lo, 0)
#endif

static const uint32_t s_aulRmTable_25_2[BVDC_P_RM_TABLE_SIZE]             = { BVDC_P_MAKE_RM(32760,  2340,    1, 0x1DDDDE, 0) }; /* 25.2MHz for Vesa 640x480p */
static const uint32_t s_aulRmTable_25_2_Div_1001[BVDC_P_RM_TABLE_SIZE]    = { BVDC_P_MAKE_RM(32400,  2349,    1, 0x1DD63A, 0) }; /* 25.2/1.001 Mhz for Vesa 640x480p */
static const uint32_t s_aulRmTable_27[BVDC_P_RM_TABLE_SIZE]               = { BVDC_P_MAKE_RM(32764,     0,    1, 0x200000, 0) }; /* 27Mhz for NTSC, NTSC_J, PAL, PAL_M, PAL_N, PAL_NC, 480p, 576p_50Hz */
static const uint32_t s_aulRmTable_27_Mul_1001[BVDC_P_RM_TABLE_SIZE]      = { BVDC_P_MAKE_RM(32032, 32000,    0, 0x200831, 0) }; /* 27*1.001 MHz for 480p @60Hz */
static const uint32_t s_aulRmTable_39_79[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(27632, 18750,    0, 0x2F289A, 0) }; /* 39.790080 Mhz for Vesa 00x600p @60Hz refresh rate */
static const uint32_t s_aulRmTable_39_79_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(30144, 20475,    0, 0x2F1C8B, 0) }; /* 39.790080/1.001 Mhz for Vesa 800x600p @59.94Hz refresh rate */
static const uint32_t s_aulRmTable_60_4656[BVDC_P_RM_TABLE_SIZE]          = { BVDC_P_MAKE_RM(29393, 13125,    0,  0x47A9B, 0) }; /* 60.4656 Mhz for Vesa 1280x720p @50Hz refresh rate */
static const uint32_t s_aulRmTable_64_0224[BVDC_P_RM_TABLE_SIZE]          = { BVDC_P_MAKE_RM(32604, 13750,    0, 0x4BE0DF, 0) }; /* 64.0224 Mhz for Vesa 1280x720p Reduced Blanking @60Hz refresh rate */
static const uint32_t s_aulRmTable_64_0224_Div_1001[BVDC_P_RM_TABLE_SIZE] = { BVDC_P_MAKE_RM(31920, 13475,    0, 0x4BCD77, 0) }; /* 64.0224/1.001 Mhz for Vesa 1280x720p Reduced Blanking @59.94Hz refresh rate */
static const uint32_t s_aulRmTable_65[BVDC_P_RM_TABLE_SIZE]               = { BVDC_P_MAKE_RM(32760, 13608,    0, 0x4D097B, 0) }; /* 64.995840 Mhz for Vesa 1024x768p @ 60Hz */
static const uint32_t s_aulRmTable_65_Div_1001[BVDC_P_RM_TABLE_SIZE]      = { BVDC_P_MAKE_RM(30000, 12474,    0, 0x4CF5C7, 0) }; /* 64.995840/1.001 Mhz for Vesa 1024x768p @ 59.94Hz */
static const uint32_t s_aulRmTable_65_286[BVDC_P_RM_TABLE_SIZE]           = { BVDC_P_MAKE_RM(32643, 13500,    0, 0x4D6041, 0) }; /* 65.286 Mhz for Vesa 1280x768 @60Hz refresh rate */
static const uint32_t s_aulRmTable_65_286_Div_1001[BVDC_P_RM_TABLE_SIZE]  = { BVDC_P_MAKE_RM(32736, 13552,    0, 0x4D4C77, 0) }; /* 65.286/1.001 Mhz for Vesa 1280x768 @59.94Hz refresh rate */
static const uint32_t s_aulRmTable_74_25[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(32758, 11912,    0, 0x580000, 0x0a02530) }; /* 74.25Mhz (1080i, 720p) */
static const uint32_t s_aulRmTable_74_25_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(32750, 11921,    0, 0x57e97f, 0x229e910) }; /* 74.25/1.001 Mhz sample rate, 720p_5994Hz or 1080i_2997Hz */
#if 0
static const uint32_t s_aulRmTable_74_48[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(77584, 28125,    0, 0x5845FA, 0) }; /* 74.480640 Mhz for vesa 1280x720p @60Hz refresh rate */
static const uint32_t s_aulRmTable_74_48_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(47744, 17325,    0, 0x582F67, 0) }; /* 74.406234 Mhz for vesa 1280x720p @59.94Hz refresh rate */
#endif
static const uint32_t s_aulRmTable_148_5[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(32758,  5956,    0, 0xB00000, 0) }; /* 148.5 Mhz for 1080p @60Hz refresh rate */
static const uint32_t s_aulRmTable_148_5_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(32500,  5915,    0, 0xAFD2FD, 0) }; /* 148.5 Mhz for 1080p @59.94Hz refresh rate */
static const uint32_t s_aulRmTable_162[BVDC_P_RM_TABLE_SIZE]              = { BVDC_P_MAKE_RM(32766,  5461,    0, 0xc00000, 0) }; /* 162 Mhz for 1600x1200p @60Hz refresh rate */
static const uint32_t s_aulRmTable_67_565[BVDC_P_RM_TABLE_SIZE]           = { BVDC_P_MAKE_RM(31280, 12500,    0, 0x5013A9, 0xCD870) }; /* 67.565 Mhz sample rate, 1366x768_60Hz */
static const uint32_t s_aulRmTable_67_565_Div_1001[BVDC_P_RM_TABLE_SIZE]  = { BVDC_P_MAKE_RM(25024, 10010,    0, 0x4FFF2E, 0xCD870) }; /* 67.565/1.001 Mhz sample rate, 1366x768_5994Hz */
static const uint32_t s_aulRmTable_56_304[BVDC_P_RM_TABLE_SIZE]           = { BVDC_P_MAKE_RM(32062, 15375,    0, 0x42BB0C, 0xCD870) }; /* 56.304 Mhz sample rate, 1366x768_50Hz */
static const uint32_t s_aulRmTable_297[BVDC_P_RM_TABLE_SIZE]              = { BVDC_P_MAKE_RM(32758,  2978,    0, 0x1600000, 0) }; /* 297 Mhz for 4kx2k @60Hz refresh rate */
static const uint32_t s_aulRmTable_297_Div_1001[BVDC_P_RM_TABLE_SIZE]     = { BVDC_P_MAKE_RM(32000,  2912,    0, 0x15FA5F9, 0) }; /* 297/1.001 Mhz for 4kx2k @59.94Hz refresh rate */

static const uint32_t s_aulRmTable_23_75[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(32680,  4472,  0x1, 0x1C25ED, 0) }; /* 23.75 Mhz sample rate, VESA 640x480p_CVT @60Hz */
static const uint32_t s_aulRmTable_23_75_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(23750,  3277,  0x1, 0x1C1EBA, 0) }; /* 23.75/1.001 Mhz sample rate, VESA 640x480p_CVT @60Hz */
static const uint32_t s_aulRmTable_83_5[BVDC_P_RM_TABLE_SIZE]             = { BVDC_P_MAKE_RM(32732, 10584,  0x0, 0x62F684, 0) }; /* 83.5 Mhz sample rate, VESA 1280x800_60Hz */
static const uint32_t s_aulRmTable_83_5_Div_1001[BVDC_P_RM_TABLE_SIZE]    = { BVDC_P_MAKE_RM(83500, 27027,  0x0, 0x62DD35, 0) }; /* 83.5/1.001 Mhz sample rate, VESA 1280x800_60Hz */
static const uint32_t s_aulRmTable_108[BVDC_P_RM_TABLE_SIZE]              = { BVDC_P_MAKE_RM(32764,  8191,  0x0, 0x800000, 0) }; /* 108 Mhz sample rate, VESA 1280x1024_60Hz */
static const uint32_t s_aulRmTable_108_Div_1001[BVDC_P_RM_TABLE_SIZE]     = { BVDC_P_MAKE_RM(32000,  8008,  0x0, 0x7fdf43, 0) }; /* 108/1.001 Mhz sample rate, VESA 1280x1024_60Hz */
static const uint32_t s_aulRmTable_85_5[BVDC_P_RM_TABLE_SIZE]             = { BVDC_P_MAKE_RM(32756, 10344,  0x0, 0x655555, 0) }; /* 85.5 Mhz sample rate, VESA 1366x768_60Hz */
static const uint32_t s_aulRmTable_85_5_Div_1001[BVDC_P_RM_TABLE_SIZE]    = { BVDC_P_MAKE_RM(28500,  9009,  0x0, 0x653B6A, 0) }; /* 85.5/1.001 Mhz sample rate, VESA 1366x768_60Hz */
static const uint32_t s_aulRmTable_106_5[BVDC_P_RM_TABLE_SIZE]            = { BVDC_P_MAKE_RM(32731,  8298,  0x0, 0x7E38E3, 0) }; /* 106.5 Mhz sample rate, VESA 1440x900_60Hz */
static const uint32_t s_aulRmTable_106_5_Div_1001[BVDC_P_RM_TABLE_SIZE]   = { BVDC_P_MAKE_RM(35500,  9009,  0x0, 0x7E189B, 0) }; /* 106.5/1.001 Mhz sample rate, VESA 1440x900_60Hz */
static const uint32_t s_aulRmTable_54[BVDC_P_RM_TABLE_SIZE]               = { BVDC_P_MAKE_RM(32766, 16383,    0, 0x400000, 0) }; /* 54Mhz for (480P or 576P) at double rate */
static const uint32_t s_aulRmTable_54_Mul_1001[BVDC_P_RM_TABLE_SIZE]      = { BVDC_P_MAKE_RM(32032, 16000,    0, 0x401062, 0) }; /* 54Mhz for 480P at double rate */

#if BVDC_P_ORTHOGONAL_VEC /** { **/
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

/* PAL_NC SVideo/CVBS */
static const uint32_t s_aulSmTable_PALNC_YUV[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(ON,  0xCB, 0x0, 1, 1, 0x21F69447, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};

/* PAL_N SVideo/CVBS */
static const uint32_t s_aulSmTable_PALN_YUV[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(
		ON, 0xFB, 0x0, 1, 1, 0x2A098ACB, 0x400, COMPOSITE, ON, USE_SIN,
		USE_COS),
};

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

/* SECAM SVideo/CVBS */
static const uint32_t s_aulSmTable_SECAM[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(ON, 0, 0x0, 0, 0, 0x284BDA12, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};

/* SM values for Component output */
static const uint32_t s_aulSmTable_Component[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(OFF, 0, 0x0, 0, 0, 0, 0, COMPONENT, OFF, USE_ONE, USE_ONE),
};

#else /** } { not orthogonal VEC **/
#define BVDC_P_MAKE_SM(amp_cntl, amp_clamp, u_burst, v_burst, u_en, v_en,      \
	init_amp, pixel_frac_en, init_phase, phase_offset, u_coring_en,            \
	v_coring_en, fre0, chroma_offset, comp_out_sel, combine_chroma,            \
	sin_coef_en, cos_coef_en)                                                  \
	BVDC_P_SM_FIELD_ENUM(ENVELOPE_GENERATOR, AMP_CONTROL,       amp_cntl) | \
	BVDC_P_SM_FIELD_DATA(ENVELOPE_GENERATOR, AMP_CLAMP,        amp_clamp) | \
	BVDC_P_SM_FIELD_ENUM(ENVELOPE_GENERATOR, U_BURST_AMP_SEL,    u_burst) | \
	BVDC_P_SM_FIELD_ENUM(ENVELOPE_GENERATOR, V_BURST_AMP_SEL,    v_burst) | \
	BVDC_P_SM_FIELD_ENUM(ENVELOPE_GENERATOR, U_ENABLE,              u_en) | \
	BVDC_P_SM_FIELD_ENUM(ENVELOPE_GENERATOR, V_ENABLE,              v_en) | \
	BVDC_P_SM_FIELD_DATA(ENVELOPE_GENERATOR, INIT_AMP,          init_amp),  \
	BVDC_P_SM_FIELD_ENUM(PG_CNTRL, PIXEL_FRAC_ENABLE,      pixel_frac_en) | \
	BVDC_P_SM_FIELD_DATA(PG_CNTRL, INIT_PHASE,                init_phase) | \
	BVDC_P_SM_FIELD_DATA(PG_CNTRL, ACTIVE_PHASE_OFFSET,     phase_offset),  \
	BVDC_P_SM_FIELD_DATA(PG_CONFIG, U_CORING_ENABLE,         u_coring_en) | \
	BVDC_P_SM_FIELD_DATA(PG_CONFIG, V_CORING_ENABLE,         v_coring_en),  \
	BVDC_P_SM_FIELD_DATA(SC_FREQ_0, FRE0,                           fre0),  \
	BVDC_P_SM_FIELD_DATA(COMP_CNTRL, CHROMA_OFFSET,        chroma_offset) | \
	BVDC_P_SM_FIELD_ENUM(COMP_CNTRL, COMP_OUT_SEL,          comp_out_sel) | \
	BVDC_P_SM_FIELD_ENUM(COMP_CNTRL, COMBINE_CHROMA,      combine_chroma) | \
	BVDC_P_SM_FIELD_ENUM(COMP_CNTRL, SIN_COEF_EN,            sin_coef_en) | \
	BVDC_P_SM_FIELD_ENUM(COMP_CNTRL, COS_COEF_EN,            cos_coef_en)

/* SM values for NTSC SVideo/CVBS */
static const uint32_t s_aulSmTable_YQI[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(CONSTANT, 0, NEGATIVE, NEGATIVE, ON, OFF, 0x70, ON, 0x14A, 0x005e, 1, 1, 0x21f07c1f, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};

/* Same, but for NTSC-704 */
#define s_aulSmTable_YQI_704 s_aulSmTable_YQI

/* PAL SVideo/CVBS */
static const uint32_t s_aulSmTable_PAL_YUV[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(CONSTANT, 0, NEGATIVE, POSITIVE, ON, ON, 0x54, ON, 0x0E5, 0x0, 1, 1, 0x2A098ACB, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};

/* PAL_N SVideo/CVBS */
static const uint32_t s_aulSmTable_PALN_YUV[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(CONSTANT, 0, NEGATIVE, POSITIVE, ON, ON, 0x54, ON, 0x0CE, 0x0, 1, 1, 0x2A098ACB, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};

/* PAL_NC SVideo/CVBS */
static const uint32_t s_aulSmTable_PALNC_YUV[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(CONSTANT, 0, NEGATIVE, POSITIVE, ON, ON, 0x54, ON, 0x1AA, 0x0, 1, 1, 0x21F69447, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};

/* PAL_I SVideo/CVBS */
static const uint32_t s_aulSmTable_PALI_YUV[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(CONSTANT, 0, NEGATIVE, POSITIVE, ON, ON, 0x54, ON, 0x100, 0x0, 1, 1, 0x2A098ACB, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};

/* PAL_M SVideo/CVBS */
static const uint32_t s_aulSmTable_PALM_YUV[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(CONSTANT, 0, NEGATIVE, POSITIVE, ON, ON, 0x58, ON, 0x250, 0x0, 1, 1, 0x21E6EFA4, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};

/* SECAM SVideo/CVBS */
static const uint32_t s_aulSmTable_SECAM[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(INC_BY_ONE, 424, NEGATIVE, NEGATIVE, ON, OFF, 0x78, ON, 0, 0x0, 0, 0, 0x284BDA12, 0x400, COMPOSITE, ON, USE_SIN, USE_COS),
};

/* SM values for Component output */
static const uint32_t s_aulSmTable_Component[BVDC_P_SM_TABLE_SIZE] =
{
	BVDC_P_MAKE_SM(CONSTANT, 0, NEGATIVE, NEGATIVE, OFF, OFF, 0, OFF, 0, 0x0, 0, 0, 0, 0, COMPONENT, OFF, USE_ONE, USE_ONE),
};

#endif /** } orthogonal VEC **/

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

/****************************************************************
 *  Channel Filter Tables
 ****************************************************************/
static const BVDC_P_FilterTableInfo s_aulFilterTable_Tbl[] =
{
	/* OutputFilter                ChFilter_Ch0               ChFilter_Ch1                 ChFilter_Ch2 */
#if BVDC_P_ORTHOGONAL_VEC
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
#else
	{BVDC_P_OutputFilter_eHDYPrPb, s_aulChFilterTbl_AllPass,  s_aulChFilterTbl_AllPass,    s_aulChFilterTbl_AllPass},
	{BVDC_P_OutputFilter_eHDRGB,   s_aulChFilterTbl_AllPass,  s_aulChFilterTbl_AllPass,    s_aulChFilterTbl_AllPass},
	{BVDC_P_OutputFilter_eED,      s_aulChFilterTbl_AllPass,  s_aulChFilterTbl_AllPass,    s_aulChFilterTbl_AllPass},
	{BVDC_P_OutputFilter_eSDYPrPb, s_aulChFilterTbl_Y_675Mhz, s_aulChFilterTbl_Cr_3375Mhz, s_aulChFilterTbl_Cr_3375Mhz},
	{BVDC_P_OutputFilter_eSDRGB,   s_aulChFilterTbl_Y_675Mhz, s_aulChFilterTbl_Y_675Mhz,   s_aulChFilterTbl_Y_675Mhz},
	{BVDC_P_OutputFilter_eYQI,     s_aulChFilterTbl_Y_60Mhz,  s_aulChFilterTbl_Cr_13Mhz,   s_aulChFilterTbl_Cr_13Mhz},
	{BVDC_P_OutputFilter_eYUV,     s_aulChFilterTbl_Y_60Mhz,  s_aulChFilterTbl_Cr_13Mhz,   s_aulChFilterTbl_Cr_13Mhz},
	{BVDC_P_OutputFilter_eSECAM,   s_aulChFilterTbl_Y_50Mhz,  s_aulChFilterTbl_Cr_13Mhz,   s_aulChFilterTbl_Cr_13Mhz},
	{BVDC_P_OutputFilter_eHsync,   s_aulChFilterTbl_Y_675Mhz, s_aulChFilterTbl_Y_675Mhz,   s_aulChFilterTbl_Y_675Mhz},
	{BVDC_P_OutputFilter_eUnknown, s_aulChFilterTbl_Y_60Mhz,  s_aulChFilterTbl_Cr_3375Mhz, s_aulChFilterTbl_Cr_3375Mhz},
#endif
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



#if BVDC_P_ORTHOGONAL_VEC
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
#if BVDC_P_SUPPORT_VEC_SECAM
	{BVDC_P_Output_eYDbDr_LDK,       s_aulEG_Tbl_YUV                      },
	{BVDC_P_Output_eYDbDr_BG,        s_aulEG_Tbl_YUV                      },
	{BVDC_P_Output_eYDbDr_H,         s_aulEG_Tbl_YUV                      },
#endif
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
#if BVDC_P_SUPPORT_VEC_SECAM
#if !BVDC_P_ORTHOGONAL_VEC
	{BVDC_P_Output_eYDbDr_LDK, s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eYUV,       BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YDbDr,   NULL},
#else
	{BVDC_P_Output_eYDbDr_LDK, s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eSECAM,     BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YDbDr,   NULL},
#endif
	{BVDC_P_Output_eYDbDr_BG,  s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eSECAM,     BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YUV,     NULL},
	{BVDC_P_Output_eYDbDr_H,   s_aulSmTable_Tbl_YUV,     BVDC_P_OutputFilter_eSECAM,     BVDC_P_OutputFilter_eNone,    s_aulSdVfTable_Tbl_YUV,     NULL},
#endif
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

/* For table size sanity check */
#define BVDC_P_DISPLAY_FMTINFO_COUNT \
	(sizeof(s_aFormatInfoTable) / sizeof(BVDC_P_FormatInfo))

#define BVDC_P_DISPLAY_FMTDATA_COUNT \
	(sizeof(s_aFormatDataTable) / sizeof(BVDC_P_FormatData))

/* Pick color space data with colorspace search */
const BVDC_P_ColorSpaceData* BVDC_P_GetColorSpaceData_isr(
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

/*************************************************************************
 *
 * End of static tables
 *
 **************************************************************************/

#ifndef BVDC_FOR_BOOTUPDATER
/*************************************************************************
 *  {secret}
 *	Utility to return pointer to appropriate BVDC_P_FormatInfo
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
 *	Utility to return pointer to appropriate BVDC_P_FormatData
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

#else /* #ifndef BVDC_FOR_BOOTUPDATER */
/*************************************************************************
 *  {secret}
 *	Utility to return pointer to appropriate BVDC_P_FormatInfo
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
 *	Utility to return pointer to appropriate BVDC_P_FormatData
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
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */

/*************************************************************************
 *  {secret}
 *	Returns pointer to appropriate RamTable for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_GetRamTableSub_isr
(
	const BVDC_P_DisplayInfo *pDispInfo,
	bool                     bArib480p
)
{
	const uint32_t *pTable = NULL;

#if BVDC_P_ORTHOGONAL_VEC
	/* Analog VEC, (480P or 576P) will always be double rate.
	 * Improves frequency response. */
	if(VIDEO_FORMAT_IS_480P(pDispInfo->pFmtInfo->eVideoFmt))
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
#else
	/* Analog VEC, (480P or 576P) may be either single or double rate */
	if ((VIDEO_FORMAT_IS_480P(pDispInfo->pFmtInfo->eVideoFmt)) &&
	    (BAVC_HDMI_PixelRepetition_e2x == pDispInfo->eHdmiPixelRepetition))
	{
		pTable =
			(bArib480p ?
				s_aulRamBVBInput_480pARIB_54MHz :
				s_aulAnalogMicrocode_480p54);
	}
	else if(VIDEO_FORMAT_IS_480P(pDispInfo->pFmtInfo->eVideoFmt))
	{
		pTable =
			(bArib480p ?
				s_aulRamBVBInput_480pARIB : s_aulAnalogMicrocode_480p);
	}
	else if ((BFMT_VideoFmt_e576p_50Hz      == pDispInfo->pFmtInfo->eVideoFmt ) &&
			 (BAVC_HDMI_PixelRepetition_e2x == pDispInfo->eHdmiPixelRepetition))
	{
		pTable = s_aulAnalogMicrocode_576p54;
	}
#endif
	else if (
		pDispInfo->bWidthTrimmed &&
		VIDEO_FORMAT_IS_NTSC(pDispInfo->pFmtInfo->eVideoFmt))
	{
		BDBG_MSG(("NTSC width = 704 samples microcode"));
		pTable = s_aulAnalogMicrocode_NTSC_704;
	}
	else if(VIDEO_FORMAT_IS_NTSC(pDispInfo->pFmtInfo->eVideoFmt) &&
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

#ifdef BVDC_P_WSE_VER3
		if (VIDEO_FORMAT_IS_PAL(pDispInfo->pFmtInfo->eVideoFmt))
		{
			pTable = s_aulRamBVBInput_PAL;
		}
#endif
	}

	return pTable;
}


/*************************************************************************
 *  {secret}
 *	Returns pointer to appropriate ItTable for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_GetItTableSub_isr
(
	const BVDC_P_DisplayInfo     *pDispInfo
)
{
	const uint32_t *pTable = NULL;

	pTable = BVDC_P_GetFormatData_isrsafe(pDispInfo->pFmtInfo->eVideoFmt)->pItTable;

#ifdef BVDC_P_WSE_VER3
	if (VIDEO_FORMAT_IS_PAL(pDispInfo->pFmtInfo->eVideoFmt))
	{
		pTable = BVDC_P_aulItTable_PAL;
	}
#endif

	return pTable;
}


/*************************************************************************
 *  {secret}
 *	Returns pointer to appropriate ItConfig for display modes.
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
 *	Returns pointer to appropriate DtramTable for display modes.
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
#if BVDC_P_ORTHOGONAL_VEC
	eHdmiPixelRepetition = pDispInfo->stHdmiSettings.eHdmiPixelRepetition;
#else
	eHdmiPixelRepetition = pDispInfo->eHdmiPixelRepetition;
#endif

	/* Analog VEC, 480P may be either single or double rate */
	if((VIDEO_FORMAT_IS_480P(pFmtInfo->eVideoFmt)) &&
	   (bArib480p))
	{
		if ((BAVC_HDMI_PixelRepetition_e2x == eHdmiPixelRepetition)||
			(BAVC_HDMI_PixelRepetition_e4x == eHdmiPixelRepetition))
		{
			pTable = s_aulDtRamBVBInput_DVI_480p_Drop1_54MHz;
		}
		else
		{
			pTable = s_aulDtRamBVBInput_DVI_480p_Drop1;
		}
	}
	else if(VIDEO_FORMAT_IS_480P(pFmtInfo->eVideoFmt))
	{
		if ((BAVC_HDMI_PixelRepetition_e2x == eHdmiPixelRepetition)||
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
			 ((BAVC_HDMI_PixelRepetition_e2x == eHdmiPixelRepetition) ||
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


/*************************************************************************
 *  {secret}
 *	Returns pointer to appropriate 656Dtram for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_Get656DtramTable_isr
(
	const BVDC_P_DisplayInfo     *pDispInfo
)
{
	const uint32_t *pTable = NULL;

	/* 656 only supports 480i and PAL */
	if (VIDEO_FORMAT_IS_525_LINES(pDispInfo->pFmtInfo->eVideoFmt))
	{
		pTable = &(s_aul656Microcode_NTSC_704[0]);
	}
	else if (VIDEO_FORMAT_IS_625_LINES(pDispInfo->pFmtInfo->eVideoFmt))
	{
		pTable = &(s_aul656Microcode_PAL_I[0]);
	}

	return pTable;
}


/*************************************************************************
 *  {secret}
 *	Returns pointer to appropriate SmTable for display modes.
 **************************************************************************/
const uint32_t* BVDC_P_GetSmTable_isr
(
	const BVDC_P_DisplayInfo     *pDispInfo,
	BVDC_P_Output			      eOutputCS
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
				VIDEO_FORMAT_IS_NTSC(pDispInfo->pFmtInfo->eVideoFmt))
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
 *	Returns pointers to appropriate channel filters for display modes.
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

#if BVDC_P_ORTHOGONAL_VEC
	/* Analog VEC, (480P or 576P) will always be double rate.
	 * Improves frequency response. */
	if (VIDEO_FORMAT_IS_ED(pDispInfo->pFmtInfo->eVideoFmt))
#else
	/* Analog VEC, 480P may be either single or double rate */
	if ((VIDEO_FORMAT_IS_ED(pDispInfo->pFmtInfo->eVideoFmt)) &&
		(BAVC_HDMI_PixelRepetition_e2x == pDispInfo->eHdmiPixelRepetition))
#endif
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


/*************************************************************************
 *  {secret}
 *	Returns appropriate SrcControl for display modes.
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


/*************************************************************************
 *  {secret}
 *	Returns pointer to appropriate VfTable for display modes.
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

	/* to make coverity check silent */
	if (eOutputColorSpace > BVDC_P_Output_eUnknown)
	{
		BDBG_ASSERT(eOutputColorSpace <= BVDC_P_Output_eUnknown);
		return;
	}

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

#if BVDC_P_ORTHOGONAL_VEC
	/* Analog VEC, (480P or 576P) will always be double rate. Improves
	 * frequency response. */
	if (VIDEO_FORMAT_IS_ED(pDispInfo->pFmtInfo->eVideoFmt))
#else
	/* Analog VEC, 480P may be either single or double rate */
	if ((VIDEO_FORMAT_IS_ED(pDispInfo->pFmtInfo->eVideoFmt)) &&
	    (BAVC_HDMI_PixelRepetition_e2x == pDispInfo->eHdmiPixelRepetition))
#endif
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
#if BVDC_P_ORTHOGONAL_VEC
			index = (BCHP_VF_0_NEG_SYNC_VALUES - BCHP_VF_0_FORMAT_ADDER)/4;
#else
			index = (BCHP_PRIM_VF_NEG_SYNC_VALUES - BCHP_PRIM_VF_FORMAT_ADDER)/4;
#endif
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
		    VIDEO_FORMAT_IS_NTSC(pDispInfo->pFmtInfo->eVideoFmt) &&
			(pDispInfo->eMacrovisionType < BVDC_MacrovisionType_eTest01))
		{
			BDBG_MSG(("NTSC width = 704 samples, left cut 8 pixels"));
			pstShaperSettings->ulSavRemove = 8;
		}
	}
}

#if BVDC_P_ORTHOGONAL_VEC
/*************************************************************************
 *  {secret}
 *	Returns VF envelop generator control settings
 **************************************************************************/
uint32_t BVDC_P_GetVfEnvelopGenerator_isr
(
	const BVDC_P_DisplayInfo *pDispInfo,
	BVDC_P_Output			  eOutputCS
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
			if(VIDEO_FORMAT_IS_525_LINES(pFmtInfo->eVideoFmt))
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

#if BVDC_P_ORTHOGONAL_VEC
	/* For (480P or 576P), operate the analog VEC at double rate.
	 * Improves frequency response. */
	if ((BFMT_VideoFmt_e480p      == pFmtInfo->eVideoFmt) ||
	    (BFMT_VideoFmt_e720x483p  == pFmtInfo->eVideoFmt) ||
	    (BFMT_VideoFmt_e576p_50Hz == pFmtInfo->eVideoFmt))
#else
	/* This is only to support (480P or 576P) with 54 MHz pixel rate */
	if (((BFMT_VideoFmt_e480p      == pFmtInfo->eVideoFmt) ||
	     (BFMT_VideoFmt_e720x483p  == pFmtInfo->eVideoFmt) ||
	     (BFMT_VideoFmt_e576p_50Hz == pFmtInfo->eVideoFmt))
	&&
	   (BAVC_HDMI_PixelRepetition_e2x == pDispInfo->eHdmiPixelRepetition))
#endif
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
 *	Returns pointer to appropriate RmString name for display modes.
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
#if BVDC_P_ORTHOGONAL_VEC
/* HDMI color depth support */
const BVDC_P_RateInfo* BVDC_P_HdmiRmTable_isr
(
	BFMT_VideoFmt             eVideoFmt,
	uint64_t                  ulPixelClkRate,
	BAVC_HDMI_BitsPerPixel    eHdmiColorDepth,
	BAVC_HDMI_PixelRepetition eHdmiPixelRepetition,
	BAVC_Colorspace           eColorComponent

)
{
	uint32_t i;
	BAVC_HDMI_PixelRepetition ePxlRep = eHdmiPixelRepetition;

	/* for 480i/576i Pixel Repetition 2x is inherent from VEC so use the
	 * same RM as BAVC_HDMI_PixelRepetition_eNone */
	if(VIDEO_FORMAT_IS_NTSC(eVideoFmt) || VIDEO_FORMAT_IS_PAL(eVideoFmt))
	{
		if(eHdmiPixelRepetition == BAVC_HDMI_PixelRepetition_e2x)
		{
			BDBG_MSG(("VideoFmt %d PxlRep e2x => use PxlRep eNone", eVideoFmt));
			ePxlRep = BAVC_HDMI_PixelRepetition_eNone;
		}
	}
	else if((eVideoFmt != BFMT_VideoFmt_e480p     ) &&
	        (eVideoFmt != BFMT_VideoFmt_e720x483p ) &&
			(eVideoFmt != BFMT_VideoFmt_e576p_50Hz))
	{
		if(eHdmiPixelRepetition != BAVC_HDMI_PixelRepetition_eNone)
		{
			BDBG_MSG(("VideoFmt %d PxlRep %d => use PxlRep eNone", eVideoFmt, eHdmiPixelRepetition));
			ePxlRep = BAVC_HDMI_PixelRepetition_eNone;
		}
	}

	for(i = 0; i < BVDC_P_RM_TABLE_ENTRIES; i++)
	{
		if((s_HdmiRm[i].ulPixelClkRate       ==  ulPixelClkRate) &&
		   (s_HdmiRm[i].eHdmiColorDepth      == eHdmiColorDepth) &&
		   (s_HdmiRm[i].eHdmiPixelRepetition ==         ePxlRep) &&
		   ((s_HdmiRm[i].eColorComponent      == eColorComponent) ||
		    (s_HdmiRm[i].eColorComponent == BAVC_Colorspace_eYCbCr444 && eColorComponent == BAVC_Colorspace_eRGB)))
		{
			BDBG_MSG(("PxlClkRate " BDBG_UINT64_FMT ", HDMI Color Depth %d, Pxl Repetition %d, Color Component %d => %sMHz",
				BDBG_UINT64_ARG(ulPixelClkRate), eHdmiColorDepth, eHdmiPixelRepetition, eColorComponent,
				s_HdmiRm[i].pchRate));
			return &s_HdmiRm[i];
		}
	}

	BDBG_ERR(("PxlClkRate " BDBG_UINT64_FMT ", HDMI Color Depth %d, Pxl Repetition %d, Color Component %d unsupported",
		BDBG_UINT64_ARG(ulPixelClkRate), eHdmiColorDepth, eHdmiPixelRepetition, eColorComponent));
	return NULL;
}

#else
const BVDC_P_RateInfo* BVDC_P_HdmiRmTable_isr
(
	BFMT_VideoFmt             eVideoFmt,
	uint64_t                  ulPixelClkRate,
	BAVC_HDMI_BitsPerPixel    eHdmiColorDepth,
	BAVC_HDMI_PixelRepetition eHdmiPixelRepetition,
	BAVC_Colorspace           eColorComponent

)
{
	uint32_t i;

	BSTD_UNUSED(eVideoFmt);
	BSTD_UNUSED(eHdmiColorDepth);
	BSTD_UNUSED(eHdmiPixelRepetition);
	BSTD_UNUSED(eColorComponent);

	for(i = 0; i < BVDC_P_RM_TABLE_ENTRIES; i++)
	{
		if(s_HdmiRm[i].ulPixelClkRate == ulPixelClkRate)
		{
			return &s_HdmiRm[i];
		}
	}
	return NULL;
}
#endif

#if BVDC_P_SUPPORT_MHL
uint64_t BVDC_P_PxlFreqToMhlFreq_isr
	( uint64_t ulPxlFreq)
{
	uint32_t i;
	uint64_t ulVal = 0;

	for (i=0; i<BVDC_P_MHL_FREQ_TABLE_ENTRIES; i++)
	{
		if (s_PxlFreqToMhlPxlFreq[i].ulPixelClkRate == ulPxlFreq)
		{
			ulVal = s_PxlFreqToMhlPxlFreq[i].ulMhlClkRate;
		}
	}
	return ulVal;
}
#endif


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
	if (VIDEO_FORMAT_IS_525_LINES(eFmt) || VIDEO_FORMAT_IS_625_LINES(eFmt) ||
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

#if !BVDC_P_SUPPORT_VEC_SECAM
	if(VIDEO_FORMAT_IS_SECAM(eVideoFmt))
	{
		return false;
	}
#endif

#if !BVDC_P_SUPPORT_1080p_60HZ
	if((eVideoFmt == BFMT_VideoFmt_e1080p) ||
	   (eVideoFmt == BFMT_VideoFmt_e1080p_50Hz))
	{
		return false;
	}
#endif

#if !BVDC_P_SUPPORT_3D_VIDEO
	if(BFMT_IS_3D_MODE(eVideoFmt))
	{
		return false;
	}
#endif

	/* Support 4k ? */
	if(VIDEO_FORMAT_IS_4kx2k(eVideoFmt))
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
	if((VIDEO_FORMAT_IS_4kx2k_50_60HZ(eVideoFmt)) &&
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
#if BVDC_P_ORTHOGONAL_VEC
#else
	BVDC_P_Display_Dump_aulChFilterTbl (
		"s_aulChFilterTbl_Cr_3375Mhz", s_aulChFilterTbl_Cr_3375Mhz);
	BVDC_P_Display_Dump_aulChFilterTbl (
		"s_aulChFilterTbl_Y_50Mhz", s_aulChFilterTbl_Y_50Mhz);
#endif
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
#if BVDC_P_ORTHOGONAL_VEC
	BVDC_P_Display_Dump_aulChFilterTbl (
		"s_aulChFilterTbl_Y_12Mhz", s_aulChFilterTbl_Y_12Mhz);
	BVDC_P_Display_Dump_aulChFilterTbl (
		"s_aulChFilterTbl_Cr_SD", s_aulChFilterTbl_Cr_SD);
	BVDC_P_Display_Dump_aulChFilterTbl (
		"s_aulChFilterTbl_Cr_SECAM", s_aulChFilterTbl_Cr_SECAM);
#endif
}

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

void BVDC_P_Display_Dump_aulSmTable (const char* name, const uint32_t* table)
{
	BKNI_Printf ("//%s %d\n", name, BVDC_P_SM_TABLE_SIZE);

#if !BVDC_P_ORTHOGONAL_VEC
	BKNI_Printf ("//%s %08x\n", "PRIM_SM_ENVELOPE_GENERATOR", *table++);
#endif
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

void BVDC_Display_DumpTables (void)
{
	BVDC_Display_DumpAll_aulVfTable();
	BVDC_Display_DumpAll_aulChFilterTbl();
	BVDC_Display_DumpAll_aulRmTable();
	BVDC_Display_DumpAll_aulItTable();
	BVDC_Display_DumpAll_aulSmTable();
}
#endif /** } BVDC_P_DISPLAY_DUMP **/
#endif /** } !B_REFSW_MINIMAL **/

/* End of File */
