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
*
 ******************************************************************************/
#include "bhdm.h"
#include "bhdm_priv.h"

BDBG_MODULE(BHDM) ;
BDBG_OBJECT_ID(HDMI) ;


#if defined(BHDM_CONFIG_DVO_SUPPORT)
#define DVO_PORT 1
#define HDMI_PORT 0
#else
#define DVO_PORT 0
#define HDMI_PORT 1
#endif


/******************************************************************************
Summary:
Enumerated Type of the input display formats for the HDMI core.  These are
indices to the BHDM_P_InputVideoFmt table. See BHDM_SupportedVideoFormats.
*******************************************************************************/
typedef enum
{
	BHDM_InputVideoFmt_ePowerUp = 0, /* Invalid Power Up Format */
	BHDM_InputVideoFmt_e640x480p,    /* Safe Mode */
	BHDM_InputVideoFmt_e1080i,
	BHDM_InputVideoFmt_e480p,
	BHDM_InputVideoFmt_e480i,
	BHDM_InputVideoFmt_e720p,
	BHDM_InputVideoFmt_e720p_24Hz,
	BHDM_InputVideoFmt_e1080p_24Hz,
	BHDM_InputVideoFmt_e1080p_25Hz,
	BHDM_InputVideoFmt_e1080p_30Hz,

#if BHDM_CONFIG_1080P_5060HZ_SUPPORT
	BHDM_InputVideoFmt_e1080p,
	BHDM_InputVideoFmt_e1080p_50Hz,
#endif

	BHDM_InputVideoFmt_e1080i_50Hz,
	BHDM_InputVideoFmt_e720p_50Hz,
	BHDM_InputVideoFmt_e576p_50Hz,

	BHDM_InputVideoFmt_e576i_50Hz,

	BHDM_InputVideoFmt_e720p_3DOU,
	BHDM_InputVideoFmt_e720p_50Hz_3DOU,
	BHDM_InputVideoFmt_e720p_24Hz_3DOU,
	BHDM_InputVideoFmt_e720p_30Hz_3DOU,
	BHDM_InputVideoFmt_e1080p_24Hz_3DOU,
	BHDM_InputVideoFmt_e1080p_30Hz_3DOU,

#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
	BHDM_InputVideoFmt_e1080p_24Hz_3D,
	BHDM_InputVideoFmt_e720p_3D,
	BHDM_InputVideoFmt_e720p_50Hz_3D,
#endif
	BHDM_InputVideoFmt_eCustom,

	BHDM_InputVideoFmt_800x600p,
	BHDM_InputVideoFmt_1024x768p,
	BHDM_InputVideoFmt_1280x768p,
	BHDM_InputVideoFmt_1280x1024p,
	BHDM_InputVideoFmt_1280x720p_50Hz,
	BHDM_InputVideoFmt_1280x720p,

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
	BHDM_InputVideoFmt_3840x2160_30Hz,
	BHDM_InputVideoFmt_3840x2160_25Hz,
	BHDM_InputVideoFmt_3840x2160_24Hz,

	BHDM_InputVideoFmt_4096x2160_24Hz,
	BHDM_InputVideoFmt_e1080p_100Hz,
	BHDM_InputVideoFmt_e1080p_120Hz,
#endif

#if BHDM_CONFIG_4Kx2K_60HZ_SUPPORT
	BHDM_InputVideoFmt_3840x2160_50Hz,
	BHDM_InputVideoFmt_3840x2160_60Hz,
#endif
	BHDM_InputVideoFmt_eMax
} BHDM_P_InputVideoFmt ;


/******************************************************************************
Summary:
HDMI Format Structure;
*******************************************************************************/
typedef enum _BHDM_P_POLARITY_
{
	BHDM_POLARITY_eNEGATIVE,
	BHDM_POLARITY_ePOSITIVE
} BHDM_P_POLARITY ;


typedef struct _BHDM_P_DISPLAY_FORMAT_
{
	uint8_t    FormatName[20] ;
	uint16_t H_ActivePixels ;
	BHDM_P_POLARITY H_Polarity ;
	BHDM_P_POLARITY V_Polarity ;

	uint16_t H_FrontPorch ;
	uint16_t H_SyncPulse ;
	uint16_t H_BackPorch ;

	uint16_t V_ActiveLinesField0 ;
	uint16_t V_FrontPorchField0 ;
	uint16_t V_SyncPulseField0 ;

	uint16_t V_BackPorchField0 ;
	uint16_t V_SyncPulseOffsetField0 ;

	uint16_t V_ActiveLinesField1 ;
	uint16_t V_FrontPorchField1 ;
	uint16_t V_SyncPulseField1 ;

	uint16_t V_BackPorchField1 ;
	uint16_t V_SyncPulseOffsetField1 ;

} BHDM_P_DISPLAY_FORMAT_DEF ;



/******************************************************************************
Summary:
HDMI Format Definitions (1080i, 720p, etc)
*******************************************************************************/
static const BHDM_P_DISPLAY_FORMAT_DEF BHDM_VideoFmtParams[] =
{
	/*
	Name
	H_ActivePixels      HorizPolarity       VertPolarity
	HorizFP 	 HorizSyncPulse      HorizBackPorch
	VertActiveLinesField0 VertFrontPorchField0 VertSyncPulseField0  VertBackPorchField0 VertSyncPulseOffsetField0
	VertActiveLinesField1 VertFrontPorchField1 VertSyncPulseField1  VertBackPorchField1 VertSyncPulseOffsetField1
	*/

	/* Formats for Boot loader usage mode */
#ifdef BHDM_FOR_BOOTUPDATER
	{ /* 1920 x 1080 i	*/
	"1080i	 ",  1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	88,   44, 148,
	540,   2,	5,	15, 0,
	540,   2,	5,	16, 1100,
	},

	{ /* 720 x 480 p	*/
	"480p	 ",  720, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	16,   62, 60,
	480,   9,  6,  30, 0,
	480,   9,  6,  30, 0,
	},

	{ /* 720 x 480 i				*/
	"480i	 ",  1440, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	38,  124, 114,
	240,   4,	3,	15,  0,
	240,   4,	3,	16,  858,
	},

	{ /* 1280 x 720 p				*/
	"720p	 ",  1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	110,  40, 220,
	720,   5,	5,	20,  0,
	720,   5,	5,	20,  0,
	},

	{ /* 720 x 576 p				*/
	"576p@50 ", 720, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	12,  64,  68,
	576,  5,   5,  39,	0,
	576,  5,   5,  39,	0,
	},

	{ /* 720 x 576 i				*/
	"576i@50 ", 1440, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	24,  126,  138,
	288,  2,   3,  19,	0,
	288,  2,   3,  20,	864,
	},

#else

	{ /* Invalid Format - Power Up Values */
	"PowerUp ",  0, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	0,	 0,	 0,
	0,	 0,	 0,	 0,	 0,
	0,	 0,	 0,	 0,	 0
	},

	{ /* 640 x 480 p	*/
	"640x480p",  640, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	16,  96,  48,
	480, 10,   2,  33,  0,
	480, 10,   2,  33,  0
	},

	{ /* 1920 x 1080 i	*/
	"1080i   ",  1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	88,   44, 148,
	540,   2,   5,  15, 0,
	540,   2,   5,  16, 1100,
	},

	{ /* 720 x 480 p	*/
	"480p    ",  720, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	16,   62, 60,
	480,   9,  6,  30, 0,
	480,   9,  6,  30, 0,
	},

	{ /* 720 x 480 i	            */
	"480i    ",  1440, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	38,  124, 114,
	240,   4,   3,  15,  0,
	240,   4,   3,  16,  858,
	},

	{ /* 1280 x 720 p	            */
	"720p    ",  1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	110,  40, 220,
	720,   5,   5,  20,  0,
	720,   5,   5,  20,  0,
	},

	{ /* 1280 x 720 p	@ 24            */
	"720p@24 ",  1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	2585,  40, 220,
	720,   5,   5,  20,  0,
	720,   5,   5,  20,  0,
	},

	{ /* 1920 x 1080 p	 @ 23.976/24 */
	"1080p@24",  1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	638,   44, 148,
	1080,   4,   5,  36, 0,
	1080,   4,   5,  36, 0,
	},

	{ /* 1920 x 1080p	@ 25*/
	"1080p@25",  1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	528,   44, 148,
	1080,   4,   5,  36, 0,
	1080,   4,   5,  36, 0,
	},

	{ /* 1920 x 1080p	@ 30 - See CEA-861B Errata document */
	"1080p@30",  1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	88,     44, 148,
	1080,   4,   5,  36, 0,
	1080,   4,   5,  36, 0,
	},

#if BHDM_CONFIG_1080P_5060HZ_SUPPORT
	{ /* 1920 x 1080 p	 @ 59.94/60 */
	"1080p",  1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	88,   44, 148,
	1080,   4,   5,  36, 0,
	1080,   4,   5,  36, 0,
	},
#endif

	/**************** 50 Hz Formats ****************/
	/*
	Name  	H_ActivePixels      HorizPolarity       VertPolarity
	HorizFP 	 HorizSyncPulse      HorizBackPorch
	VertActiveLinesField0 VertFrontPorchField0 VertSyncPulseField0  VertBackPorchField0 VertSyncPulseOffsetField0
	VertActiveLinesField1 VertFrontPorchField1 VertSyncPulseField1  VertBackPorchField1 VertSyncPulseOffsetField1
	*/

#if	BHDM_CONFIG_1080P_5060HZ_SUPPORT
	{ /* 1920 x 1080 p	 @ 50 */
	"1080p@50",  1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	528,   44, 148,
	1080,	4,	 5,  36, 0,
	1080,	4,	 5,  36, 0,
	},
#endif

	{ /* 1920 x 1080 i	*/
	"1080i@50", 1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	528,  44, 148,
	540,   2,   5,  15, 0,
	540,   2,   5,  16, 1320,
	},

	{ /* 1280 x 720 p	            */
	"720p@50 ", 1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	440,  40, 220,
	720,   5,   5,  20,  0,
	720,   5,   5,  20,  0,
	},

	{ /* 720 x 576 p	            */
	"576p@50 ", 720, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	12,  64,  68,
	576,  5,   5,  39,  0,
	576,  5,   5,  39,  0,
	},

	{ /* 720 x 576 i	            */
	"576i@50 ", 1440, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	24,  126,  138,
	288,  2,   3,  19,  0,
	288,  2,   3,  20,  864,
	},

	/************
	* 3D formats */
	/*
	Name	H_ActivePixels		HorizPolarity		VertPolarity
	HorizFP 	 HorizSyncPulse 	 HorizBackPorch
	VertActiveLinesField0 VertFrontPorchField0 VertSyncPulseField0	VertBackPorchField0 VertSyncPulseOffsetField0
	VertActiveLinesField1 VertFrontPorchField1 VertSyncPulseField1	VertBackPorchField1 VertSyncPulseOffsetField1
	*/

	{ /* 720p@60Hz__3DOU				*/
	"720p@60__3DOU ",  1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	110,  40, 220,
	1470,	5,	 5,  20,  0,
	1470,	5,	 5,  20,  0,
	},

	{ /* 720p@50Hz__3DOU		*/
	"720p@50__3DOU ", 1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	440,  40, 220,
	1470,	5,	5,	20,  0,
	1470,	5,	5,	20,  0,
	},

	{ /* 720p@24Hz__3DOU				*/
	"720p@24__3DOU ",  1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	2585,  40, 220,
	1470,	5,	 5,  20,  0,
	1470,	5,	 5,  20,  0,
	},

	{ /* 720p@30Hz__3DOU		*/
	"720p@30__3DOU ", 1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	440,  40, 220,
	1470,	5,	5,	20,  0,
	1470,	5,	5,	20,  0,
	},

	{ /* 1080p@23.976/24Hz__3DOU */
	"1080p@24__3DOU ",	1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	638,   44, 148,
	2205,	4,	 5,  36, 0,
	2205,	4,	 5,  36, 0,
	},

	{ /* 1080p@30__3DOU */
	"1080p@30__3DOU",  1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	88, 	44, 148,
	2205,	4,	 5,  36, 0,
	2205,	4,	 5,  36, 0,
	},

#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
	{ /* 1920 x 1080 p	 @ 24 3D */
	"1080p@24 3D", 1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	638,   44, 148,
	2205,   4,   5,  36, 0,
	2205,   4,   5,  36, 0,
	},

	{ /* 1280 x 720 p	 @ 60 3D */
	"720p@60 3D ", 1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	110,   40, 220,
	1470,   5,   5,  20, 0,
	1470,   5,   5,  20, 0,
	},

	{ /* 1280 x 720 p	 @ 50 3D */
	"720p@50 3D ", 1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	440,   40, 220,
	1470,   5,   5,  20, 0,
	1470,   5,   5,  20, 0,
	},
#endif

	{ /* Custom Place Holder         */
	"Custom  ", 0, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	},

	/*
	Name    H_ActivePixels      HorizPolarity       VertPolarity
	HorizFP 	 HorizSyncPulse      HorizBackPorch
	VertActiveLinesField0 VertFrontPorchField0 VertSyncPulseField0  VertBackPorchField0 VertSyncPulseOffsetField0
	VertActiveLinesField1 VertFrontPorchField1 VertSyncPulseField1  VertBackPorchField1 VertSyncPulseOffsetField1
	*/

	{ /* 800 x 600                   */
	"800x600 ", 800, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	40,  128,  88,
	600,  1,  4,  23,  0,
	600,  1,  4,  23,  0,
	},

	{ /* 1024 x 768                   */
	"1024x768 ", 1024, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_eNEGATIVE,
	24,  136,  160,
	768,  3,  6,  29,  0,
	768,  3,  6,  29,  0,
	},

	{ /* 1280 x 768                   */
	"1280x768 ", 1280, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_ePOSITIVE,
	64,  128,  192,
	768,  3,  7,  20,  0,
	768,  3,  7,  20,  0,
	},

	{ /* 1280 x 1024                   */
	"1280x1024", 1280, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	48,  112, 248,
	1024,  1,  3,  38,  0,
	1024,  1,  3,  38,	0
	},


	{ /* Place Holder for 1280x720 50Hz        */
	"1280x720*", 0, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	0,  0,  0,
	0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,
	},

	{ /* 1280 x 720                   */
	"1280x720 ", 1280, BHDM_POLARITY_eNEGATIVE, BHDM_POLARITY_ePOSITIVE,
	64,  128,  192,
	720,  3,  7,  20,  0,
	720,  3,  7,  20,  0,
	},

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
	/*
	Name    H_ActivePixels      HorizPolarity       VertPolarity
	HorizFP 	 HorizSyncPulse      HorizBackPorch
	VertActiveLinesField0 VertFrontPorchField0 VertSyncPulseField0  VertBackPorchField0 VertSyncPulseOffsetField0
	VertActiveLinesField1 VertFrontPorchField1 VertSyncPulseField1  VertBackPorchField1 VertSyncPulseOffsetField1
	*/

	{ /* 4K x 2K  30 Hz                */
	"4K x 2K  @30", 3840, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	176,  88,  296,
	2160,  8,  10,  72,  0,
	2160,  8,  10,  72,  0,
	},

	{ /* 4K x 2K  25 Hz                */
	"4K x 2K  @25", 3840, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	1056,  88,  296,
	2160,  8,  10,  72,  0,
	2160,  8,  10,  72,  0,
	},

	{ /* 4K x 2K  24 Hz                */
	"4K x 2K  @24", 3840, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	1276,  88,  296,
	2160,  8,  10,  72,  0,
	2160,  8,  10,  72,  0,
	},

	{ /* 4K x 2K  24 Hz                */
	"4K x 2K  @24 (SMPTE)", 4096, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	1020,  88,  296,
	2160,  8,  10,  72,  0,
	2160,  8,  10,  72,  0,
	},

	{ /* 1920 x 1080 p	 @ 100 */
	"1080p100",  1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	528,   44, 148,
	1080,   4,   5,  36, 0,
	1080,   4,   5,  36, 0,
	},

	{ /* 1920 x 1080 p	 @ 119.88/120 */
	"1080p120",  1920, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	88,    44, 148,
	1080,   4,   5,  36, 0,
	1080,   4,   5,  36, 0,
	},
#endif

#if BHDM_CONFIG_4Kx2K_60HZ_SUPPORT
	/*
	Name    H_ActivePixels      HorizPolarity       VertPolarity
	HorizFP 	 HorizSyncPulse      HorizBackPorch
	VertActiveLinesField0 VertFrontPorchField0 VertSyncPulseField0  VertBackPorchField0 VertSyncPulseOffsetField0
	VertActiveLinesField1 VertFrontPorchField1 VertSyncPulseField1  VertBackPorchField1 VertSyncPulseOffsetField1
	*/

	{ /* 4K x 2K  50 Hz                */
	"4K x 2K  @50", 3840, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	1056,  88,  296,
	2160,  8,  10,  72,  0,
	2160,  8,  10,  72,  0,
	},

	{ /* 4K x 2K  60 Hz                */
	"4K x 2K  @60", 3840, BHDM_POLARITY_ePOSITIVE, BHDM_POLARITY_ePOSITIVE,
	176,  88,  296,
	2160,  8,  10,  72,  0,
	2160,  8,  10,  72,  0,
	}

#endif


#endif	/* #ifndef BHDM_FOR_BOOTUPDATER */

} ; /* BHDM_VideoFmtParams[] */


static BERR_Code BHDM_P_ConfigureInputVideoFmt(const BHDM_Handle hHDMI,
	const BHDM_Settings *NewHdmiSettings) ;

#ifndef BHDM_FOR_BOOTUPDATER
static BERR_Code BHDM_P_ConfigurePixelRepeater(const BHDM_Handle hHDMI,
	BFMT_VideoFmt eVideoFmt, BAVC_HDMI_PixelRepetition ePixelRepetition) ;

static void BHDM_P_HandleMaiFormatUpdate_isr(const BHDM_Handle hHDMI) ;

static void BHDM_StartDriftFIFORecenter_isrsafe(
   const BHDM_Handle hHDMI) ;

#if BHDM_CONFIG_BTMR_SUPPORT
static void BHDM_P_TimerExpiration_isr (const BHDM_Handle hHDM, int parm2) ;
#endif

#if BHDM_CONFIG_DVO_SUPPORT
BERR_Code BHDM_P_EnableDvoDisplay(const BHDM_Handle hHDMI, BHDM_Settings *NewHdmiSettings) ;
#endif

#endif /* #ifndef BHDM_FOR_BOOTUPDATER */


/******************************************************************************
Summary:
Interrupt Callback Table to describe interrupt Names, ISRs, and Masks
*******************************************************************************/
typedef struct BHDM_P_InterruptCbTable
{
	BINT_Id       	  IntrId;
	BINT_CallbackFunc pfCallback;
	int               iParam2;
	bool              enable ; /* debug purposes */
} BHDM_P_InterruptCbTable ;


#if BHDM_HAS_MULTIPLE_PORTS
#define BHDM_INT_CB_DEF(instance, intr, id, enable) \
      {BCHP_INT_ID_HDMI_TX_INTR2##instance##intr, BHDM_P_HandleInterrupt_isr, BHDM_INTR_##id, enable}
#else
#define BHDM_INT_CB_DEF(instance, intr, id, enable) \
      {BCHP_INT_ID_##intr, BHDM_P_HandleInterrupt_isr, BHDM_INTR_##id, enable}
#endif

#if BHDM_CONFIG_DUAL_HPD_SUPPORT
#define HP_DEF(instance) \
    BHDM_INT_CB_DEF(instance, HP_REMOVED_INTR, eHOTPLUG_REMOVED, true),\
    BHDM_INT_CB_DEF(instance, HP_CONNECTED_INTR, eHOTPLUG_CONNECTED, true),
#else
#define HP_DEF(instance) \
	BHDM_INT_CB_DEF(instance, HOTPLUG_INTR, eHOTPLUG, true),
#endif

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
#define RI_PJ_DEF(instance) \
	BHDM_INT_CB_DEF(instance, PJ_MISMATCH_INTR, eHDCP_PJ_MISMATCH, HDMI_PORT), \
	BHDM_INT_CB_DEF(instance, RI_A_MISMATCH_INTR, eHDCP_RI_A_MISMATCH, HDMI_PORT), \
	BHDM_INT_CB_DEF(instance, RI_B_MISMATCH_INTR, eHDCP_RI_B_MISMATCH, HDMI_PORT),
#else
#define RI_PJ_DEF(instance) \
	/* blank line */
#endif

#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT
#define RSEN_DEF(instance) \
	BHDM_INT_CB_DEF(instance, RSEN_UPDATE_INTR, eRSEN, !BHDM_CONFIG_RSEN_POLLING),
#else
#define RSEN_DEF(instance) \
	/*blank line */
#endif

#define BHDM_P_CREATE_TABLE(instance)\
{ \
	HP_DEF(instance) \
	BHDM_INT_CB_DEF(instance, DRIFT_FIFO_FULL_MINUS_INTR, eDF_FULL_MINUS, false), \
	BHDM_INT_CB_DEF(instance, DRIFT_FIFO_ALMOST_FULL_INTR,eDF_ALMOST_FULL, false), \
	BHDM_INT_CB_DEF(instance, DRIFT_FIFO_EMPTY_MINUS_INTR, eDF_EMPTY_MINUS, false), \
	BHDM_INT_CB_DEF(instance, DRIFT_FIFO_ALMOST_EMPTY_INTR,eDF_ALMOST_EMPTY, false), \
\
	BHDM_INT_CB_DEF(instance, ILLEGAL_WRITE_TO_ACTIVE_RAM_PACKET_INTR, ePKT_WRITE_ERR, false), \
\
	BHDM_INT_CB_DEF(instance, HDCP_REPEATER_ERR_INTR, eHDCP_REPEATER_ERR, HDMI_PORT), \
	BHDM_INT_CB_DEF(instance, HDCP_V_MISMATCH_INTR, eHDCP_V_MISMATCH, HDMI_PORT), \
	BHDM_INT_CB_DEF(instance, HDCP_V_MATCH_INTR, eHDCP_V_MATCH, HDMI_PORT), \
	BHDM_INT_CB_DEF(instance, HDCP_RI_INTR, eHDCP_RI, HDMI_PORT), \
	BHDM_INT_CB_DEF(instance, HDCP_AN_READY_INTR, eHDCP_AN, HDMI_PORT), \
\
	BHDM_INT_CB_DEF(instance, PACKET_FIFO_OVERFLOW_INTR, ePKT_OVERFLOW, false), \
	BHDM_INT_CB_DEF(instance, HDCP_PJ_INTR, eHDCP_PJ, false), \
	BHDM_INT_CB_DEF(instance, MAI_FORMAT_UPDATE_INTR, eMAI_FORMAT_UPDATE, true), \
	RI_PJ_DEF(instance) \
	RSEN_DEF(instance) \
\
}

static const BHDM_P_InterruptCbTable BHDM_Interrupts[] = BHDM_P_CREATE_TABLE(_) ;
#if BHDM_HAS_MULTIPLE_PORTS
static const BHDM_P_InterruptCbTable BHDM_Interrupts_1[] = BHDM_P_CREATE_TABLE(_1_);
#endif


#if BHDM_CONFIG_HAS_HDCP22
#if BHDM_HAS_MULTIPLE_PORTS
#define BHDM_HAE_INT_CB_DEF(instance, intr, id, enable) \
	{BCHP_INT_ID_HDMI_TX_HAE_INTR2_##instance##_CPU_STATUS_##intr, BHDM_P_HandleHAEInterrupt_isr, BHDM_HAE_INTR_##id, enable}
#else
#define BHDM_HAE_INT_CB_DEF(instance, intr, id, enable) \
	{BCHP_INT_ID_HDMI_TX_HAE_INTR2_0_CPU_STATUS_##intr, BHDM_P_HandleHAEInterrupt_isr, BHDM_HAE_INTR_##id, enable}
#endif

#define BHDM_P_CREATE_HAE_TABLE(instance) \
{ \
	BHDM_HAE_INT_CB_DEF(instance, OK_TO_ENC_EN, eOK_TO_ENC_EN, true), \
	BHDM_HAE_INT_CB_DEF(instance, REAUTH_REQ_INTR, eREAUTH_REQ, true), \
\
}

static const BHDM_P_InterruptCbTable BHDM_HAE_Interrupts[] = BHDM_P_CREATE_HAE_TABLE(0) ;
#if BHDM_HAS_MULTIPLE_PORTS
static const BHDM_P_InterruptCbTable BHDM_HAE_Interrupts_1[] = BHDM_P_CREATE_HAE_TABLE(1);
#endif
#endif /* #if BHDM_CONFIG_HAS_HDCP22 */



/*******************************************************************************
const char * BHDM_P_GetVersion
Summary: Get the current version of the HDM PI (used to identify the HDM PI) for debugging
*******************************************************************************/
const char * BHDM_P_GetVersion(void)
{
	static const char Version[] = "BHDM URSR 17.3" ;
	return Version ;
}

/*******************************************************************************
void BHDM_P_SetDisplayStartupDefaults_isr
Summary: Set the default settings for starting the HDMI Display (after power down, hot plug, etc.)
*******************************************************************************/
static void BHDM_P_SetDisplayStartupDefaults_isr(BHDM_Handle hHDMI)
{
	BDBG_ENTER(BHDM_P_SetDisplayStartupDefaults_isr) ;

	/* force detection of RxSense when TMDS is re-enabled  */
	hHDMI->rxSensePowerDetected = false ;

	/* force EnableDisplay if/when TMDS lines are enabled */
	/* See BHDM_EnableDisplay */
	hHDMI->DeviceSettings.bForceEnableDisplay = true ;

#if BHDM_HAS_HDMI_20_SUPPORT
	/* force configuration of Scrambling */
	/* See BHDM_ConfigurePhy */
      hHDMI->TmdsBitClockRatioChange = true;
#endif

	BDBG_LEAVE(BHDM_P_SetDisplayStartupDefaults_isr) ;
}


/*******************************************************************************
void BHDM_P_ResetHdmiScheduler_isrsafe
Summary: Set the HDMI Scheduler to its default settings for starting in DVI Mode
(after power down, hot plug, etc.)
*******************************************************************************/
static void BHDM_P_ResetHdmiScheduler_isrsafe(BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register ;
	uint32_t ulOffset ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_SCHEDULER_CONTROL + ulOffset) ;
		Register &= ~(
			  BCHP_MASK(HDMI_SCHEDULER_CONTROL, HDMI_MODE_REQUEST)
			| BCHP_MASK(HDMI_SCHEDULER_CONTROL, ALWAYS_VERT_KEEP_OUT)) ;
	BREG_Write32(hRegister, BCHP_HDMI_SCHEDULER_CONTROL + ulOffset, (uint32_t) Register) ;

	hHDMI->DeviceSettings.eOutputFormat = BHDM_OutputFormat_eDVIMode ;
}


/*******************************************************************************
BERR_Code BHDM_GetDefaultSettings
Summary: Get the default settings for the HDMI device.
*******************************************************************************/
BERR_Code BHDM_GetDefaultSettings(
   BHDM_Settings *pDefault  /* [in] pointer to memory to hold default settings */
)
{
	BERR_Code      rc = BERR_SUCCESS;
	BDBG_ENTER(BHDM_GetDefaultSettings) ;

	BDBG_ASSERT(pDefault) ;
	BKNI_Memset(pDefault, 0, sizeof(BHDM_Settings));

	/* Set default settings */

	pDefault->eInputVideoFmt = BFMT_VideoFmt_e1080i;
	pDefault->eTimebase = BAVC_Timebase_e0;
	pDefault->eOutputPort = BHDM_OutputPort_eHDMI;
	pDefault->eOutputFormat = BHDM_OutputFormat_eDVIMode;
	pDefault->overrideDefaultColorimetry = false ;

	pDefault->eColorimetry = BAVC_MatrixCoefficients_eHdmi_RGB;
	pDefault->eAspectRatio = BFMT_AspectRatio_e4_3;
	pDefault->ePixelRepetition = BAVC_HDMI_PixelRepetition_eNone;
	pDefault->stColorDepth.eBitsPerPixel = BAVC_HDMI_BitsPerPixel_e24bit;

	/**** AVI Info Frame Structure ****/
	{
		pDefault->stAviInfoFrame.bOverrideDefaults = false;

		/* RGB or YCbCr (Y1Y0) generated from BHDM_Settings eColorimetry
		   always RGB444 or YCbCr444 */

		pDefault->stAviInfoFrame.eActiveInfo = BAVC_HDMI_AviInfoFrame_ActiveInfo_eValid; /* A0 */
		pDefault->stAviInfoFrame.eBarInfo = BAVC_HDMI_AviInfoFrame_BarInfo_eInvalid ; /* B1B0 */
		pDefault->stAviInfoFrame.eScanInfo = BAVC_HDMI_AviInfoFrame_ScanInfo_eOverScanned;	/* S1S0 */

		/* Colorimetry (C1C0) generated from BHDM_Settings eColorimetry
		(BAVC_MatrixCoefficients) requires conversion to HDMI AVI values */

		/* Picture AR (M1M0) generated from BHDM_Settings eAspectRatio
		(BFMT_AspectRatio) which requires conversion to HDMI AVI values
		unless overidden */
		pDefault->stAviInfoFrame.ePictureAspectRatio =
			BAVC_HDMI_AviInfoFrame_PictureAspectRatio_e4_3;
		pDefault->stAviInfoFrame.eActiveFormatAspectRatio =
			BAVC_HDMI_AviInfoFrame_ActiveFormatAspectRatio_ePicture; /* R3R0 */
		pDefault->stAviInfoFrame.eScaling =
			BAVC_HDMI_AviInfoFrame_Scaling_eNoScaling, /* SC1SC0 */

		/* Video ID Code (VICn) generated from BHDM_Settings eInputVideoFmt */
		pDefault->stAviInfoFrame.VideoIdCode = BFMT_VideoFmt_eNTSC,

		/* Pixel Repeat (PRn) generated from BHDM_Settings eInputVideoFmt
		   (BFMT_VideoFmt) */

#if BAVC_HDMI_1_3_SUPPORT
		pDefault->stAviInfoFrame.eITContent =
			BAVC_HDMI_AviInfoFrame_ITContent_eNoData;
		pDefault->stAviInfoFrame.eExtendedColorimetry =
			BAVC_HDMI_AviInfoFrame_ExtendedColorimetry_exvYCC601;
		pDefault->stAviInfoFrame.eRGBQuantizationRange =
			BAVC_HDMI_AviInfoFrame_RGBQuantizationRange_eDefault;
#endif

		/* bar info */
		pDefault->stAviInfoFrame.TopBarEndLineNumber = 0;
		pDefault->stAviInfoFrame.BottomBarStartLineNumber = 0;
		pDefault->stAviInfoFrame.LeftBarEndPixelNumber = 0;
		pDefault->stAviInfoFrame.RightBarEndPixelNumber = 0;
	}


	/**** AUDIO Info Frame Structure *****/
	{
		pDefault->stAudioInfoFrame.ChannelCount =
			BAVC_HDMI_AudioInfoFrame_ChannelCount_e2Channels ;

		pDefault->stAudioInfoFrame.CodingType =
			BAVC_HDMI_AudioInfoFrame_CodingType_eReferToStream ;

		pDefault->stAudioInfoFrame.SampleSize =
			BAVC_HDMI_AudioInfoFrame_SampleSize_eReferToStreamHeader ;

		/* Sample Freq always Refer To Stream Header for PCM and compressed */
		pDefault->stAudioInfoFrame.SampleFrequency =
			BAVC_HDMI_AudioInfoFrame_SampleFrequency_eReferToStreamHeader ;

		pDefault->stAudioInfoFrame.SpeakerAllocation = BHDM_ChannelAllocation_eStereo;
		pDefault->stAudioInfoFrame.DownMixInhibit = 0;
		pDefault->stAudioInfoFrame.LevelShift = 0;
	}

	pDefault->eAudioSamplingRate = BAVC_AudioSamplingRate_e48k;
	pDefault->eAudioFormat = BAVC_AudioFormat_ePCM;
	pDefault->eAudioBits = BAVC_AudioBits_e16;

	pDefault->eSpdSourceDevice = BAVC_HDMI_SpdInfoFrame_SourceType_eDigitalSTB;
	BKNI_Memcpy(pDefault->SpdVendorName, "Broadcom",
		BAVC_HDMI_SPD_IF_VENDOR_LEN);
	BKNI_Memcpy(pDefault->SpdDescription, "Reference Board",
		BAVC_HDMI_SPD_IF_DESC_LEN);

	/**** Vendor Specific Info Frame ****/
	{
		pDefault->stVendorSpecificInfoFrame.uIEEE_RegId[0] = 0x03;
		pDefault->stVendorSpecificInfoFrame.uIEEE_RegId[1] = 0x0C;
		pDefault->stVendorSpecificInfoFrame.uIEEE_RegId[2] = 0x0;
		pDefault->stVendorSpecificInfoFrame.eHdmiVideoFormat =
				BAVC_HDMI_VSInfoFrame_HDMIVideoFormat_eNone;
		pDefault->stVendorSpecificInfoFrame.eHdmiVic = 0;
		pDefault->stVendorSpecificInfoFrame.e3DStructure = 0;
		pDefault->stVendorSpecificInfoFrame.e3DExtData =
				BAVC_HDMI_VSInfoFrame_3DExtData_eNone;
	}

	pDefault->CalculateCts = false;
	pDefault->uiDriverAmpDefault = BHDM_CONFIG_DRIVER_AMP_DEFAULT;
	pDefault->AltDvoPath = false;
	pDefault->BypassEDIDChecking = false; /* bypass EDID checking; DEBUG ONLY!*/
	pDefault->bFifoMasterMode = false;	/* use slave mode */
	pDefault->bForceEnableDisplay = true;  /* make sure EnableDisplay is completed at initialization */
	pDefault->bEnableAutoRiPjChecking = false;

	pDefault->bEnableScdcMonitoring = true ;

#if BHDM_CONFIG_28NM_SUPPORT
	BHDM_TMDS_P_GetDefaultPreEmphasisRegisters(pDefault->TmdsPreEmphasisRegisters) ;
#endif

	BDBG_LEAVE(BHDM_GetDefaultSettings) ;
	return rc ;
}

void BHDM_P_EnableInterrupts(const BHDM_Handle hHDMI)
{
	BERR_Code rc ;
	uint8_t i ;
	const BHDM_P_InterruptCbTable *pInterrupts ;

	if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e0)
	{
		pInterrupts = BHDM_Interrupts ;
	}
#if BHDM_HAS_MULTIPLE_PORTS
	else if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e1)
	{
		pInterrupts = BHDM_Interrupts_1 ;
	}
#endif
	else
	{
		BDBG_ERR(("Unknown Core ID: %d", hHDMI->eCoreId)) ;
		BDBG_ASSERT(false) ;
		return ;
	}

	/* enable specified interrupt callbacks */
	for( i = 0; i < MAKE_INTR_ENUM(LAST) ; i++ )
	{
		/* now enable it; if specified for startup */
		if (!pInterrupts[i].enable)
			continue ;

		/* enable Receiver Sense Interrupts when output is first enabled */
#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT && !BHDM_CONFIG_RSEN_POLLING
		if (i == MAKE_INTR_ENUM(RSEN))
			continue ;
#endif

		rc = BINT_EnableCallback( hHDMI->hCallback[i]) ;
		if (rc) {rc = BERR_TRACE(rc) ; }
	}

#if BHDM_CONFIG_HAS_HDCP22
	if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e0) {
		pInterrupts = BHDM_HAE_Interrupts;
	}
#if BHDM_HAS_MULTIPLE_PORTS
	else if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e1)
	{
		pInterrupts = BHDM_HAE_Interrupts_1 ;
	}
#endif
	else
	{
		BDBG_ERR(("Unknown Core ID: %d", hHDMI->eCoreId)) ;
		BDBG_ASSERT(false) ;
		return ;
	}

	/* enable specified interrupt callbacks */
	for( i = 0; i < MAKE_HAE_INTR_ENUM(LAST) ; i++ )
	{
		/* now enable it; if specified for startup */
		if (!pInterrupts[i].enable)
			continue ;

		rc = BINT_EnableCallback( hHDMI->hHAECallback[i]) ;
		if (rc) {rc = BERR_TRACE(rc) ; }
	}

#endif /* #if BHDM_CONFIG_HAS_HDCP22 */
}


void BHDM_P_DisableInterrupts(const BHDM_Handle hHDMI)
{
	BERR_Code rc ;
	uint8_t i ;
	const BHDM_P_InterruptCbTable *pInterrupts ;

	if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e0)
	{
		pInterrupts = BHDM_Interrupts ;
	}
#if BHDM_HAS_MULTIPLE_PORTS
	else if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e1)
	{
		pInterrupts = BHDM_Interrupts_1 ;
	}
#endif
	else
	{
		BDBG_ERR(("Unknown Core ID: %d", hHDMI->eCoreId)) ;
		BDBG_ASSERT(false) ;
		return ;
	}
	/* enable specified interrupt callbacks */
	for( i = 0; i < MAKE_INTR_ENUM(LAST) ; i++ )
	{
		/* now enable it; if specified for startup */
		if (!pInterrupts[i].enable)
			continue ;

		/* enable Receiver Sense Interrupts when output is first enabled */
#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT && !BHDM_CONFIG_RSEN_POLLING
		if (i == MAKE_INTR_ENUM(RSEN))
			continue ;
#endif

		rc = BINT_DisableCallback( hHDMI->hCallback[i]) ;
		if (rc) {rc = BERR_TRACE(rc) ; }
	}


#if BHDM_CONFIG_HAS_HDCP22
	if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e0) {
		pInterrupts = BHDM_HAE_Interrupts ;
	}
#if BHDM_HAS_MULTIPLE_PORTS
	else if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e1) {
		pInterrupts = BHDM_HAE_Interrupts_1 ;
	}
#endif
	else {
		BDBG_ERR(("Unknown Core ID: %d", hHDMI->eCoreId)) ;
		BDBG_ASSERT(false) ;
		return ;
	}

	/* enable specified interrupt callbacks */
	for( i = 0; i < MAKE_HAE_INTR_ENUM(LAST) ; i++ )
	{
		/* now enable it; if specified for startup */
		if (!pInterrupts[i].enable)
			continue ;

		rc = BINT_DisableCallback( hHDMI->hHAECallback[i]) ;
		if (rc) {rc = BERR_TRACE(rc) ; }
	}
#endif /* #if BHDM_CONFIG_HAS_HDCP22 */

}


BERR_Code BHDM_P_BREG_I2C_Read(
	const BHDM_Handle hHDMI,
	uint16_t chipAddr,
	uint8_t subAddr,
	uint8_t *pData,
	size_t length
)
{
	BERR_Code rc = BERR_SUCCESS;

#if BHDM_CONFIG_HAS_HDCP22
	/* make sure polling Auto I2C channels are disabled; prior to the read */
	BKNI_EnterCriticalSection() ;
	BHDM_AUTO_I2C_SetChannels_isr(hHDMI, 0) ;
	BKNI_LeaveCriticalSection() ;
#endif

	rc = BREG_I2C_Read(hHDMI->hI2cRegHandle, chipAddr, subAddr, pData, length) ;
	if (rc) {rc = BERR_TRACE(rc) ; }

#if BHDM_CONFIG_HAS_HDCP22
	/* re-enable any polling Auto I2C channels that had to be disabled prior to the read */
	BKNI_EnterCriticalSection() ;
	BHDM_AUTO_I2C_SetChannels_isr(hHDMI, 1) ;
	BKNI_LeaveCriticalSection() ;
#endif

	return rc;
}


BERR_Code BHDM_P_BREG_I2C_ReadNoAddr(
	const BHDM_Handle hHDMI,
	uint16_t chipAddr,
	uint8_t *pData,
	size_t length
)
{
	BERR_Code rc = BERR_SUCCESS;

#if BHDM_CONFIG_HAS_HDCP22
	/* make sure polling Auto I2C channels are disabled; prior to the read */
	BKNI_EnterCriticalSection() ;
	BHDM_AUTO_I2C_SetChannels_isr(hHDMI, 0) ;
	BKNI_LeaveCriticalSection() ;
#endif

	rc = BREG_I2C_ReadNoAddr(hHDMI->hI2cRegHandle, chipAddr, pData, length) ;
	if (rc) {rc = BERR_TRACE(rc) ; }

#if BHDM_CONFIG_HAS_HDCP22
	/* re-enable any polling Auto I2C channels that had to be disabled prior to the read */
	BKNI_EnterCriticalSection() ;
	BHDM_AUTO_I2C_SetChannels_isr(hHDMI, 1) ;
	BKNI_LeaveCriticalSection() ;
#endif

	return rc;
}


#if BHDM_CONFIG_HAS_HDCP22

static void BHDM_P_ReleaseHDCP22_Resources(BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t i ;

	for ( i = 0; i < MAKE_HAE_INTR_ENUM(LAST); i++ )
	{
		if (!hHDMI->hHAECallback[i])
			continue ;

		rc = BINT_DestroyCallback( hHDMI->hHAECallback[i] ) ;
		if (rc)
		{
			rc = BERR_TRACE(rc) ;
			return ;
		}
	}

	if (hHDMI->BHDM_EventHdcp22EncEnUpdate != NULL) {
		BKNI_DestroyEvent((hHDMI->BHDM_EventHdcp22EncEnUpdate));
		hHDMI->BHDM_EventHdcp22EncEnUpdate = NULL;
	}

	if (hHDMI->BHDM_EventHdcp22ReAuthRequest!= NULL) {
		BKNI_DestroyEvent((hHDMI->BHDM_EventHdcp22ReAuthRequest));
		hHDMI->BHDM_EventHdcp22ReAuthRequest= NULL;
	}
}

BERR_Code BHDM_P_AcquireHDCP22_Resources(
	BHDM_Handle hHDMI
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BCHP_Info chipInfo;
	const BHDM_P_InterruptCbTable *pHAEInterrupts;
	uint8_t i ;


	BCHP_GetInfo(hHDMI->hChip, &chipInfo) ;
	if ((chipInfo.familyId == 0x7445) && (chipInfo.rev == 0x30))
	{
		hHDMI->TxSupport.MaxTmdsRateMHz = BHDM_HDMI_1_4_MAX_RATE ;
	}


/************/
/* HDCP 2.2 Events */
/************/
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventHdcp22EncEnUpdate)));
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventHdcp22ReAuthRequest)));


	if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e0) {
		pHAEInterrupts = BHDM_HAE_Interrupts ;
	}
#if BHDM_HAS_MULTIPLE_PORTS
	else if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e1) {
		pHAEInterrupts = BHDM_HAE_Interrupts_1 ;
	}
#endif
	else
	{
		BDBG_ERR(("Unknown Core ID: %d", hHDMI->eCoreId)) ;
		BDBG_ASSERT(false) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	/* register interrupt callbacks */
	for( i = 0; i < MAKE_HAE_INTR_ENUM(LAST) ; i++ )
	{
		BHDM_CHECK_RC( rc, BINT_CreateCallback(
			&(hHDMI->hHAECallback[i]), hHDMI->hInterrupt,
			pHAEInterrupts[i].IntrId,
			BHDM_P_HandleHAEInterrupt_isr, (void *) hHDMI, i ));

		/* clear interrupt callback */
		BHDM_CHECK_RC(rc, BINT_ClearCallback( hHDMI->hHAECallback[i])) ;
	}
	/* The HAE interrupts will be enabled along with other HDMI interrupts later */


/********************/
/* Allocate I2c Resources */
/********************/
	BHDM_CHECK_RC(rc, BHDM_AUTO_I2C_P_AllocateResources(hHDMI)) ;

	/* all HDMI 2.0 chips than support YCbCr 420 will also support YCbCr422 */
	hHDMI->TxSupport.YCbCr420 = 1 ;
	hHDMI->TxSupport.YCbCr422 = 1 ;

done:
	if (rc)  /* error was detected */
	{
		BHDM_P_ReleaseHDCP22_Resources(hHDMI) ;
	}

	return rc ;
 }
#endif


/******************************************************************************
BERR_Code BHDM_Open
Summary: Open/Initialize the HDMI device
*******************************************************************************/
BERR_Code BHDM_Open(
   BHDM_Handle *phHDMI,       /* [out] HDMI handle */
   BCHP_Handle hChip,         /* [in] Chip handle */
   BREG_Handle hRegister,     /* [in] Register Interface to HDMI Tx Core */
   BINT_Handle hInterrupt,    /* [in] Interrupt handle */
   BREG_I2C_Handle hI2cRegHandle,      /* [in] I2C Interface to HDMI Rx */
   const BHDM_Settings  *pSettings /* [in] default HDMI settings */
)
{
	BERR_Code      rc = BERR_SUCCESS;
	BHDM_Handle hHDMI = NULL ;

	uint32_t ulOffset;
	BHDM_P_HdmCoreId eCoreId ;
	uint8_t i ;
	const BHDM_P_InterruptCbTable *pInterrupts ;
	BCHP_Info chipInfo;

	BDBG_ENTER(BHDM_Open) ;

	/* verify parameters */
	BDBG_ASSERT(hChip) ;
	BDBG_ASSERT(hRegister) ;
	BDBG_ASSERT(hInterrupt) ;

	*phHDMI = NULL ;

	if (!pSettings)
	{
		BDBG_ERR(("BHDM_Settings required for BHDM_Open")) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}


	if((pSettings->eOutputPort == BHDM_OutputPort_eHDMI) && (HDMI_PORT)) {
		BDBG_ASSERT(hI2cRegHandle) ;
	}

	/* output port/compilation consistency check */

	/* if ASSERT here BHDM_Settings may not be properly set */

	BDBG_ASSERT(
		((pSettings->eOutputPort == BHDM_OutputPort_eHDMI) && (HDMI_PORT))
	||   ((pSettings->eOutputPort == BHDM_OutputPort_eDVO) && (DVO_PORT))) ;


	/* create the HDMI Handle */
	hHDMI = (BHDM_Handle) BKNI_Malloc(sizeof(BHDM_P_Handle)) ;
	if (hHDMI == NULL)
	{
		BDBG_ERR(("Tx%d: Unable to allocate memory for HDMI Handle", pSettings->eCoreId)) ;
		rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY) ;
		goto done ;
	}

	/* zero out all memory associated with the HDMI Device Handle before using */
	BKNI_Memset((void *) hHDMI, 0, sizeof(BHDM_P_Handle)) ;
	BDBG_OBJECT_SET(hHDMI, HDMI) ;

	/* assign the handles passed in as parameters */
	hHDMI->hChip      = hChip ;
	hHDMI->hRegister  = hRegister ;
	hHDMI->hInterrupt = hInterrupt ;
	hHDMI->hI2cRegHandle       = hI2cRegHandle ;

	/* save the settings and set the output port-- once set it never changes */
	hHDMI->DeviceSettings = *pSettings ;
	hHDMI->eOutputPort = pSettings->eOutputPort ;

	eCoreId = pSettings->eCoreId ;
	BDBG_ASSERT(eCoreId < BHDM_P_eHdmCoreIdMax) ;
	hHDMI->eCoreId = eCoreId ;

	hHDMI->DeviceStatus.stPort.eCoreId = eCoreId ;
	hHDMI->DeviceStatus.stPort.eDevice = BAVC_HDMI_Device_eTx ;
	/* TODO replace references of eCoreId to hHDMI->DeviceStatus.stPort.eCoreId */

#if BCHP_PWR_SUPPORT
	switch(eCoreId) {
	case BAVC_HDMI_CoreId_e0:
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CLK
	    hHDMI->clkPwrResource[eCoreId] = BCHP_PWR_RESOURCE_HDMI_TX_CLK;
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_PHY
	    hHDMI->phyPwrResource[eCoreId] = BCHP_PWR_RESOURCE_HDMI_TX_PHY;
#endif
	    break;
	case BAVC_HDMI_CoreId_e1:
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_1_CLK
	    hHDMI->clkPwrResource[eCoreId] = BCHP_PWR_RESOURCE_HDMI_TX_1_CLK;
#endif
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_1_PHY
	    hHDMI->phyPwrResource[eCoreId] = BCHP_PWR_RESOURCE_HDMI_TX_1_PHY;
#endif
	    break;
	default:
	    break;
	}
#endif

	/* Register offset from HDM */
	hHDMI->ulOffset = ulOffset = 0
#if BHDM_HAS_MULTIPLE_PORTS
		+ BHDM_P_GET_REG_OFFSET(hHDMI->eCoreId, BCHP_DVP_HT_REG_START, BCHP_DVP_HT_1_REG_START)
#endif
		;

	BDBG_MSG(("Tx%d: *** DVP_HT/HDMI_TX Register Offset: 0x%x ***",
		hHDMI->eCoreId, hHDMI->ulOffset)) ;


	/* set the default pixel clock rate to 27MHz to match 480i default */
	hHDMI->eTmdsClock = BHDM_P_TmdsClock_e27 ;


	/* set default output format to DVI - dynamic based on attached monitor */
	hHDMI->DeviceSettings.eOutputFormat = BHDM_OutputFormat_eDVIMode ;

#if BHDM_CONFIG_BTMR_SUPPORT
	BDBG_ASSERT(pSettings->hTMR) ;
	hHDMI->hTMR = pSettings->hTMR ;

	BHDM_P_AllocateTimers(hHDMI) ;
#endif

#if BCHP_PWR_RESOURCE_HDMI_TX_CLK || BCHP_PWR_RESOURCE_HDMI_TX_1_CLK
	/* Power up everything. This has to happen after the soft reset because the soft reset touches
	the same registers that BCHP_PWR touches. The soft reset does not require HDMI clocks to be on */
	BCHP_PWR_AcquireResource(hChip, hHDMI->clkPwrResource[hHDMI->eCoreId]);
#endif

	/* Used only for legacy 65nm. not required for 28nm and 40nm */
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
	BCHP_PWR_AcquireResource(hHDMI->hChip, BCHP_PWR_RESOURCE_HDMI_TX_CEC);
#endif


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
	{
		uint32_t Register ;

		/* all and any register R/W must happen after the AcquireResource */
#if BHDM_CONFIG_40NM_SUPPORT || BHDM_CONFIG_28NM_SUPPORT
		Register = BREG_Read32(hRegister, BCHP_DVP_HT_CORE_REV + ulOffset);
#else
		Register = BREG_Read32(hRegister, BCHP_HDMI_CORE_REV  + ulOffset);
#endif
		BSTD_UNUSED(Register) ;
	}

	/* display version information */
	BDBG_MSG(("*****************************************")) ;
	BDBG_MSG(("%s   %d", BHDM_P_GetVersion(), BCHP_CHIP)) ;
	BDBG_MSG(("*****************************************")) ;

/* inform if the TMDS lines are swapped */
#if BHDM_CONFIG_SWAP_DEFAULT_PHY_CHANNELS
	BDBG_LOG(("HDMI Tx Phy CK and CH2 signals will be swapped")) ;
#endif

#endif /* #ifndef BHDM_FOR_BOOTUPDATER */

	if (pSettings->bCrcTestMode)
	{
		hHDMI->bCrcTestMode = true ;
		hHDMI->DeviceSettings.UseDebugEdid = true ;
		BDBG_LOG(("Hot Plug and Rx Power detection are bypassed ; HDMI IS ALWAYS ON")) ;
	}


	/* Power on PHY */
	BDBG_MSG(("Power ON HDMI Phy at %d", __LINE__)) ;
	BHDM_P_PowerOnPhy (hHDMI) ;

	BKNI_EnterCriticalSection() ;
		/* check if something is connected and stored */
		BHDM_P_RxDeviceAttached_isr(hHDMI, &hHDMI->RxDeviceAttached) ;

		/* Initialize/Reset HDCP Settings */
		BHDM_HDCP_P_ResetSettings_isr(hHDMI) ;
	BKNI_LeaveCriticalSection() ;

	/* Reset the EDID for reading */
	hHDMI->edidStatus = BHDM_EDID_STATE_eInitialize;  /* Set EDID Initialization flag */


	/* Create Events for use with Interrupts */
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventHDCP))) ;

	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventHotPlug))) ;
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventFIFO))) ;
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventRAM))) ;
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventHDCPRiValue))) ;
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventHDCPPjValue))) ;
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventHDCPRepeater))) ;
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventRxSense))) ;
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventScramble)));
	BHDM_CHECK_RC(rc, BKNI_CreateEvent(&(hHDMI->BHDM_EventAvRateChange)));

/************
HDCP 2.2 Events
************/

	if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e0)
	{
		pInterrupts = BHDM_Interrupts ;
	}
#if BHDM_HAS_MULTIPLE_PORTS
	else if (hHDMI->eCoreId == BAVC_HDMI_CoreId_e1)
	{
		pInterrupts = BHDM_Interrupts_1 ;
	}
#endif
	else
	{
		BDBG_ERR(("Unknown Core ID: %d", hHDMI->eCoreId)) ;
		BDBG_ASSERT(false) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}


	/* register/enable interrupt callbacks */
	for( i = 0; i < MAKE_INTR_ENUM(LAST) ; i++ )
	{
		/*
		** DEBUG
		** Create ALL interrupt callbacks
		** enable debug callbacks as needed;
		*/

		BHDM_CHECK_RC( rc, BINT_CreateCallback(
			&(hHDMI->hCallback[i]), hHDMI->hInterrupt,
			pInterrupts[i].IntrId,
			BHDM_P_HandleInterrupt_isr, (void *) hHDMI, i ));

		/* clear interrupt callback */
		BHDM_CHECK_RC(rc, BINT_ClearCallback( hHDMI->hCallback[i])) ;
	}

	/* enable the output port with the correct output format DVO/HDMI */
#if DVO_PORT
#if BHDM_CONFIG_DVO_SUPPORT
	if (hHDMI->eOutputPort != BHDM_OutputPort_eHDMI) /* HDMI Port is default setting */
	{
		BHDM_CHECK_RC( rc, BHDM_DVO_P_EnableDvoPort(hHDMI, pSettings->eOutputFormat)) ;
		hHDMI->DeviceSettings.BypassEDIDChecking = true ;
	}

#else
#error "Unknown/Unsupported chip for DVO port"
#endif
#endif

	BCHP_GetInfo(hHDMI->hChip, &chipInfo) ;


	hHDMI->TxSupport.MaxTmdsRateMHz = BHDM_CONFIG_MAX_TMDS_RATE ;

	BDBG_MSG(("Platform %x %x Max TMDS Rate: %d",
		chipInfo.familyId, chipInfo.rev, hHDMI->TxSupport.MaxTmdsRateMHz)) ;


#if BHDM_CONFIG_65NM_SUPPORT
	/* set the pre-emphasis mode to DeepColorMode. This settings only makes a difference when
		running at pixel clock higher than 148.5Mhz and makes no harm for other clock rate */
	BHDM_P_SetPreEmphasisMode(hHDMI, BHDM_PreEmphasis_eDeepColorMode, 0);
#endif


#if BHDM_CONFIG_PLL_KICKSTART_WORKAROUND
	hHDMI->uiPllKickStartCount = 0 ;
#endif

	/* keep created pointer */
	*phHDMI = hHDMI ;

#if BHDM_CONFIG_HAS_HDCP22
	/***********************************/
	/* Allocate HDCP22 / I2c Resources */
	/***********************************/
	BHDM_CHECK_RC(rc, BHDM_P_AcquireHDCP22_Resources(hHDMI)) ;
#endif


#if BHDM_CONFIG_28NM_SUPPORT
	BHDM_TMDS_SetPreEmphasisRegisters(hHDMI, pSettings->TmdsPreEmphasisRegisters) ;

	BKNI_Memcpy(&hHDMI->TmdsPreEmphasisRegisters, &pSettings->TmdsPreEmphasisRegisters,
		sizeof(BHDM_TmdsPreEmphasisRegisters) * BHDM_TMDS_RANGES) ;
#endif

	/* enable all interrupts */
	BHDM_P_EnableInterrupts(hHDMI) ;

#ifndef BHDM_FOR_BOOTUPDATER
	/* Update audio channel map based on MAI_FORMAT */
	BKNI_EnterCriticalSection();
	BHDM_P_HandleMaiFormatUpdate_isr(hHDMI);
	BKNI_LeaveCriticalSection();
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */

/* turn off HDMI Phy until needed */
	BHDM_DisableDisplay(hHDMI) ;
#if BCHP_PWR_RESOURCE_HDMI_TX_PHY || BCHP_PWR_RESOURCE_HDMI_TX_1_PHY
	BDBG_MSG(("Power OFF HDMI Phy at %d", __LINE__)) ;
	BHDM_P_PowerOffPhy(hHDMI) ;
#endif

done:

	if ((rc != BERR_SUCCESS) && (hHDMI != NULL))
	{

#if BCHP_PWR_RESOURCE_HDMI_TX_CLK || BCHP_PWR_RESOURCE_HDMI_TX_1_CLK
		/* on failure, power everything down */
		BCHP_PWR_ReleaseResource(hChip, hHDMI->clkPwrResource[hHDMI->eCoreId]);
#endif
		/* Used only for legacy 65nm. not required for 28nm and 40nm */
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
		/* on failure, power everything down */
		BCHP_PWR_ReleaseResource(hChip, BCHP_PWR_RESOURCE_HDMI_TX_CEC);
#endif

		BDBG_OBJECT_DESTROY(hHDMI, HDMI) ;
		BKNI_Free((void*) hHDMI) ;
		*phHDMI = NULL ;
	}

	BDBG_LEAVE(BHDM_Open) ;
	return rc ;
} /* end BHDM_Open */


#if BHDM_CONFIG_DEBUG_FIFO
/******************************************************************************
Summary:
	Enable/Disable FIFO interrupts for debuging
*******************************************************************************/
BERR_Code BHDM_P_EnableFIFOInterrupts(
	const BHDM_Handle hHDMI,
	bool on)
{
	BERR_Code	   rc = BERR_SUCCESS ;

	if (on)
	{
		/* clear any pending interrupts first */
		BHDM_CHECK_RC( rc, BINT_ClearCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_FULL_MINUS)] )) ;
		BHDM_CHECK_RC( rc, BINT_ClearCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_ALMOST_FULL)])) ;
		BHDM_CHECK_RC( rc, BINT_ClearCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_EMPTY_MINUS)])) ;
		BHDM_CHECK_RC( rc, BINT_ClearCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_ALMOST_EMPTY)])) ;


		/* enable the interrupts */
		BHDM_CHECK_RC( rc, BINT_EnableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_FULL_MINUS)] )) ;
		BHDM_CHECK_RC( rc, BINT_EnableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_ALMOST_FULL)])) ;
		BHDM_CHECK_RC( rc, BINT_EnableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_EMPTY_MINUS)])) ;
		BHDM_CHECK_RC( rc, BINT_EnableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_ALMOST_EMPTY)])) ;
	}
	else
	{
		/* disable the interrupt callbacks */
		BHDM_CHECK_RC( rc, BINT_DisableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_FULL_MINUS)])) ;
		BHDM_CHECK_RC( rc, BINT_DisableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_ALMOST_FULL)])) ;
		BHDM_CHECK_RC( rc, BINT_DisableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_EMPTY_MINUS)])) ;
		BHDM_CHECK_RC( rc, BINT_DisableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(DF_ALMOST_EMPTY)])) ;
	}

done:
	return rc ;
}
#endif


#if BHDM_CONFIG_DVO_SUPPORT
/******************************************************************************
BERR_Code BHDM_P_EnableDvoDisplay
Summary: Display output to the DVO output port
*******************************************************************************/
BERR_Code BHDM_P_EnableDvoDisplay(const BHDM_Handle hHDMI, BHDM_Settings *NewHdmiSettings)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_P_EnableDvoDisplay) ;

#if BHDM_CONFIG_DEBUG_FIFO
	/* always disable FIFO callbacks when changing the Rate Manager */
	BHDM_CHECK_RC(rc, BHDM_P_EnableFIFOInterrupts(hHDMI, false)) ;
#endif


	BHDM_P_PowerOnPhy(hHDMI);

	/****  15 Enable TMDS Buffers */
	BHDM_EnableTmdsClock(hHDMI, true) ;
	BHDM_EnableTmdsData(hHDMI, true) ;

	/*RB*  07 Configure HDMI to the Input Display Format - Scheduler */
	BHDM_CHECK_RC(rc, BHDM_P_ConfigureInputVideoFmt(hHDMI, NewHdmiSettings)) ;

	/* save the new HDMI Settings we used to enable the HDMI device for this HDMI handle */
	hHDMI->DeviceSettings = *NewHdmiSettings ;

#if BHDM_CONFIG_DEBUG_FIFO
	/* dump FIFO pointers for debugging */
	BHDM_P_CaptureFIFOData(hHDMI) ;

	/* re-enable FIFO Interrupts */
	BHDM_CHECK_RC(rc, BHDM_P_EnableFIFOInterrupts(hHDMI, true)) ;
#endif

done:
	BDBG_LEAVE(BHDM_P_EnableDvoDisplay) ;

	return rc ;
}
#endif


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
/******************************************************************************
BERR_Code BHDM_EnableTmdsClock
Summary: Enable TMDS Clock
*******************************************************************************/
BERR_Code BHDM_EnableTmdsClock(
   const BHDM_Handle hHDMI,		/* [in] HDMI handle */
   bool bEnableTmdsClock	/* [in] boolean to enable/disable */
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_EnableTmdsClock) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* whenever enabling the TMDS Clock make sure phy is powered as well */
	if (bEnableTmdsClock)
	{
		if (!hHDMI->phyPowered)
		{
			BDBG_MSG(("Power ON HDMI Phy at %d", __LINE__)) ;
			BHDM_P_PowerOnPhy(hHDMI) ;
		}
	}
	else
	{
		BDBG_MSG(("Power OFF HDMI Phy at %d", __LINE__)) ;
		BHDM_P_PowerOffPhy(hHDMI) ;
	}

	BKNI_EnterCriticalSection() ;
		BHDM_P_EnableTmdsClock_isr(hHDMI, bEnableTmdsClock) ;
	BKNI_LeaveCriticalSection() ;

	hHDMI->DeviceStatus.tmds.clockEnabled = bEnableTmdsClock ;

	BDBG_LEAVE(BHDM_EnableTmdsClock) ;
	return rc ;
}
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */


/******************************************************************************
BERR_Code BHDM_EnableTmdsData
Summary: Enable (Display) TMDS Output
*******************************************************************************/
BERR_Code BHDM_EnableTmdsData(
   const BHDM_Handle hHDMI,		/* [in] HDMI handle */
   bool bEnableTmdsOutput	/* [in] boolean to enable/disable */
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_EnableTmdsData) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI ) ;

	BKNI_EnterCriticalSection() ;
		BHDM_P_EnableTmdsData_isr(hHDMI, bEnableTmdsOutput) ;
	BKNI_LeaveCriticalSection() ;

	/* whenever enabling/disabling TMDS lines; phy should be powered the same */
	if (bEnableTmdsOutput)
	{
		if (!hHDMI->phyPowered)
		{
			BDBG_MSG(("Power ON HDMI Phy at %d", __LINE__)) ;
			BHDM_P_PowerOnPhy(hHDMI) ;
		}
	}
	else
	{
		/* stop any running timers if TMDS data has been disabled */
		BKNI_EnterCriticalSection() ;
			BHDM_MONITOR_P_StopTimers_isr(hHDMI) ;
		BKNI_LeaveCriticalSection() ;

		/* if the phy is currently powered ON... */
		/*    turn OFF if there is no rx device attached */
		/*    or turn OFF if the chip supports RxSense without Phy Power */
		if ((hHDMI->phyPowered)
#ifndef BHDM_CONFIG_RXSENSE_STANDALONE_SUPPORT
		&& (!hHDMI->RxDeviceAttached)
#endif
		)
		{
			BDBG_MSG(("Power OFF HDMI Phy at %d", __LINE__)) ;
			BHDM_P_PowerOffPhy(hHDMI) ;
		}
	}

	/* set bForceEnableDisplay when disabling output */
	if (!bEnableTmdsOutput)
		hHDMI->DeviceSettings.bForceEnableDisplay = true ;

	hHDMI->DeviceStatus.tmds.dataEnabled = bEnableTmdsOutput ;

	BDBG_LEAVE(BHDM_EnableTmdsData) ;

	return rc ;
}


static bool BHDM_P_HdmiPhyChanges(const BHDM_Handle hHDMI, const BHDM_Settings *NewHdmiSettings)
{
	bool bPhyChanges = false ;

	if ((hHDMI->DeviceSettings.eInputVideoFmt == NewHdmiSettings->eInputVideoFmt)
	&& (hHDMI->DeviceSettings.eTimebase == NewHdmiSettings->eTimebase)
	&& (hHDMI->DeviceSettings.eOutputPort == NewHdmiSettings->eOutputPort)
	&& (hHDMI->DeviceSettings.eOutputFormat == NewHdmiSettings->eOutputFormat)
	&& (hHDMI->DeviceSettings.eAspectRatio == NewHdmiSettings->eAspectRatio)
	&& (hHDMI->DeviceSettings.ePixelRepetition == NewHdmiSettings->ePixelRepetition)
	&& (hHDMI->DeviceSettings.stVideoSettings.eColorSpace == NewHdmiSettings->stVideoSettings.eColorSpace)

	&& (hHDMI->DeviceSettings.stVideoSettings.eBitsPerPixel == NewHdmiSettings->stVideoSettings.eBitsPerPixel)
	&& (hHDMI->DeviceSettings.stColorDepth.eBitsPerPixel == NewHdmiSettings->stColorDepth.eBitsPerPixel)

	&& (hHDMI->DeviceSettings.CalculateCts == NewHdmiSettings->CalculateCts)
	&& (hHDMI->DeviceSettings.uiDriverAmpDefault == NewHdmiSettings->uiDriverAmpDefault)
	&& (hHDMI->DeviceSettings.AltDvoPath == NewHdmiSettings->AltDvoPath)

	&& (hHDMI->DeviceSettings.BypassEDIDChecking == NewHdmiSettings->BypassEDIDChecking)
	&& (hHDMI->DeviceSettings.bFifoMasterMode == NewHdmiSettings->bFifoMasterMode)
	&& (hHDMI->DeviceSettings.bForceEnableDisplay == NewHdmiSettings->bForceEnableDisplay)
	&& (hHDMI->DeviceSettings.bEnableAutoRiPjChecking == NewHdmiSettings->bEnableAutoRiPjChecking))
	{
		bPhyChanges = false ;
	}
	else
	{
		bPhyChanges = true ;
	}

	BDBG_MSG(("HDMI Phy Changes: %s", bPhyChanges ? "Yes" : "No")) ;
	return bPhyChanges ;
}



static bool BHDM_P_HdmiPacketChanges(const BHDM_Handle hHDMI, const BHDM_Settings *pNewHdmiSettings)
{
	bool bPacketChanges = false ;

	/**** AVI Info Frame  ****/
	if ((hHDMI->DeviceSettings.stAviInfoFrame.ePixelEncoding == pNewHdmiSettings->stAviInfoFrame.ePixelEncoding)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.bOverrideDefaults == pNewHdmiSettings->stAviInfoFrame.bOverrideDefaults)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eActiveInfo == pNewHdmiSettings->stAviInfoFrame.eActiveInfo)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eBarInfo == pNewHdmiSettings->stAviInfoFrame.eBarInfo)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eScanInfo == pNewHdmiSettings->stAviInfoFrame.eScanInfo)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eActiveInfo == pNewHdmiSettings->stAviInfoFrame.eActiveInfo)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.ePictureAspectRatio == pNewHdmiSettings->stAviInfoFrame.ePictureAspectRatio)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eActiveFormatAspectRatio == pNewHdmiSettings->stAviInfoFrame.eActiveFormatAspectRatio)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eScaling == pNewHdmiSettings->stAviInfoFrame.eScaling)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.VideoIdCode == pNewHdmiSettings->stAviInfoFrame.VideoIdCode)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eActiveInfo == pNewHdmiSettings->stAviInfoFrame.eActiveInfo)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.PixelRepeat == pNewHdmiSettings->stAviInfoFrame.PixelRepeat)
#if BAVC_HDMI_1_3_SUPPORT
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eITContent == pNewHdmiSettings->stAviInfoFrame.eITContent)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eExtendedColorimetry == pNewHdmiSettings->stAviInfoFrame.eExtendedColorimetry)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eRGBQuantizationRange == pNewHdmiSettings->stAviInfoFrame.eRGBQuantizationRange)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eYccQuantizationRange == pNewHdmiSettings->stAviInfoFrame.eYccQuantizationRange)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.eContentType == pNewHdmiSettings->stAviInfoFrame.eContentType)
#endif
	&& (hHDMI->DeviceSettings.stAviInfoFrame.TopBarEndLineNumber== pNewHdmiSettings->stAviInfoFrame.TopBarEndLineNumber)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.BottomBarStartLineNumber == pNewHdmiSettings->stAviInfoFrame.BottomBarStartLineNumber)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.LeftBarEndPixelNumber == pNewHdmiSettings->stAviInfoFrame.LeftBarEndPixelNumber)
	&& (hHDMI->DeviceSettings.stAviInfoFrame.RightBarEndPixelNumber == pNewHdmiSettings->stAviInfoFrame.RightBarEndPixelNumber)
	&& (hHDMI->DeviceSettings.eColorimetry == pNewHdmiSettings->eColorimetry)


	/**** Audio Info Frame ****/
	&& (hHDMI->DeviceSettings.stAudioInfoFrame.bOverrideDefaults == pNewHdmiSettings->stAudioInfoFrame.bOverrideDefaults)
	&& (hHDMI->DeviceSettings.stAudioInfoFrame.ChannelCount == pNewHdmiSettings->stAudioInfoFrame.ChannelCount)
	&& (hHDMI->DeviceSettings.stAudioInfoFrame.CodingType == pNewHdmiSettings->stAudioInfoFrame.CodingType)
	&& (hHDMI->DeviceSettings.stAudioInfoFrame.SampleSize == pNewHdmiSettings->stAudioInfoFrame.SampleSize)
	&& (hHDMI->DeviceSettings.stAudioInfoFrame.SampleFrequency == pNewHdmiSettings->stAudioInfoFrame.SampleFrequency)
	&& (hHDMI->DeviceSettings.stAudioInfoFrame.SpeakerAllocation == pNewHdmiSettings->stAudioInfoFrame.SpeakerAllocation)
	&& (hHDMI->DeviceSettings.stAudioInfoFrame.DownMixInhibit == pNewHdmiSettings->stAudioInfoFrame.DownMixInhibit)
	&& (hHDMI->DeviceSettings.stAudioInfoFrame.LevelShift == pNewHdmiSettings->stAudioInfoFrame.LevelShift)


	/**** Source Product Description Info Frame ****/
	&& (hHDMI->DeviceSettings.eSpdSourceDevice == pNewHdmiSettings->eSpdSourceDevice)
	&& (BKNI_Memcmp(hHDMI->DeviceSettings.SpdVendorName,
		pNewHdmiSettings->SpdVendorName, BAVC_HDMI_SPD_IF_VENDOR_LEN) == 0)
	&& (BKNI_Memcmp(hHDMI->DeviceSettings.SpdDescription,
		pNewHdmiSettings->SpdDescription, BAVC_HDMI_SPD_IF_DESC_LEN) == 0)


	/**** Vendor Specific Info Frame ****/
	&& (BKNI_Memcmp(hHDMI->DeviceSettings.stVendorSpecificInfoFrame.uIEEE_RegId,
		pNewHdmiSettings->stVendorSpecificInfoFrame.uIEEE_RegId,
		BAVC_HDMI_IEEE_REGID_LEN) == 0)

	&& (hHDMI->DeviceSettings.stVendorSpecificInfoFrame.eHdmiVideoFormat ==
	          pNewHdmiSettings->stVendorSpecificInfoFrame.eHdmiVideoFormat)


	&& (hHDMI->DeviceSettings.stVendorSpecificInfoFrame.eHdmiVic ==
	          pNewHdmiSettings->stVendorSpecificInfoFrame.eHdmiVic)
	/* or */
	&& (hHDMI->DeviceSettings.stVendorSpecificInfoFrame.e3DStructure ==
	          pNewHdmiSettings->stVendorSpecificInfoFrame.e3DStructure)

	&& (hHDMI->DeviceSettings.stVendorSpecificInfoFrame.e3DExtData ==
	          pNewHdmiSettings->stVendorSpecificInfoFrame.e3DExtData))
	{
		bPacketChanges = false ;
	}
	else
	{
		bPacketChanges = true ;
	}

	BDBG_MSG(("HDMI Packet Updates? %s", bPacketChanges ? "Yes" : "No")) ;
	return bPacketChanges ;
}





/******************************************************************************
BERR_Code BHDM_EnableDisplay
Summary: Display output to the HDMI device
*******************************************************************************/
BERR_Code BHDM_EnableDisplay(const BHDM_Handle hHDMI, const BHDM_Settings *NewHdmiSettings)
{
	BERR_Code rc = BERR_SUCCESS ;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint8_t DeviceAttached ;

/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
	uint8_t timeoutFrames ;
	uint8_t HdmiModeSet ;
	uint8_t bHPInterrupt = false;
	bool HdmiVideoFormatChange = false ;

	uint8_t FrameDelay ;

	const BFMT_VideoInfo *pVideoInfo ;
	bool bHdmiPhyChanges ;
#endif	/* #ifndef BHDM_FOR_BOOTUPDATER */

#if BHDM_CONFIG_FORMAT_DETECT_HSYNC_SATURATION_SUPPORT
	uint8_t saturatedHsyncFallCount ;
	uint8_t saturatedHsyncRiseCount ;
#endif

	BDBG_ENTER(BHDM_EnableDisplay) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI ) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

#if BHDM_CONFIG_DVO_SUPPORT
	BSTD_UNUSED(Register) ;
	BSTD_UNUSED(DeviceAttached) ;
	BSTD_UNUSED(timeoutFrames) ;
	BSTD_UNUSED(HdmiModeSet) ;
	BSTD_UNUSED(bHPInterrupt);

	rc = BHDM_P_EnableDvoDisplay(hHDMI, NewHdmiSettings) ;
	if (rc) {rc = BERR_TRACE(rc) ; }

	goto done ;
#else

	/* check if a Receiver is Attached */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &DeviceAttached)) ;

	if (!DeviceAttached)
	{
		BDBG_WRN(("Tx%d: No DVI/HDMI Receiver is Attached", hHDMI->eCoreId)) ;
		goto done;
	}

	if (!NewHdmiSettings->bForceEnableDisplay)
	{
		bHdmiPhyChanges = BHDM_P_HdmiPhyChanges(hHDMI, NewHdmiSettings) ;
		if (!bHdmiPhyChanges)
		{
			if  (!hHDMI->bForcePacketUpdates)
			{
				if (!BHDM_P_HdmiPacketChanges(hHDMI, NewHdmiSettings))
				{
					goto done ;
				}
			}
			BDBG_MSG(("HDMI Packet updates only...")) ;
			goto ConfigureHdmiPackets ;
		}
	}

/* Configure HDMI Phy */
	BHDM_EnableTmdsClock(hHDMI, true) ;  /* make sure clock is enabled */
	BHDM_EnableTmdsData(hHDMI, true) ;

	/* Additional PHY settings, pre-emphasis, etc */
	BHDM_CHECK_RC(rc, BHDM_P_ConfigurePhy(hHDMI, NewHdmiSettings));

	/* ensure GCP is disabled in DVI mode */
	Register = BREG_Read32(hRegister, BCHP_HDMI_GCP_CONFIG + ulOffset);
	Register &= ~BCHP_MASK(HDMI_GCP_CONFIG, GCP_ENABLE) ;
	BREG_Write32(hRegister, BCHP_HDMI_GCP_CONFIG + ulOffset, Register);

	/* Initialize HDMI core */
	/****  01 Determine Supported Output Mode (DVI, HDMI) from Rx */

	/****  05 Check for Lock & Nominal Difference */

#if BHDM_CONFIG_DEBUG_FIFO
	/* always disable FIFO callbacks when changing the Rate Manager */
	BHDM_P_EnableFIFOInterrupts(hHDMI, false) ;
#endif

	/*RB*  07 Configure HDMI to the Input Display Format - Scheduler */
	BHDM_CHECK_RC(rc, BHDM_P_ConfigureInputVideoFmt(hHDMI, NewHdmiSettings)) ;

/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
	/*RB*  08 Configure Pixel Repeater for rate < 25MPixels */
	if  (NewHdmiSettings->eOutputFormat == BHDM_OutputFormat_eHDMIMode)
	{
		BHDM_CHECK_RC(rc, BHDM_P_ConfigurePixelRepeater(hHDMI, NewHdmiSettings->eInputVideoFmt,
						NewHdmiSettings->ePixelRepetition)) ;
	}
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */


	/*RK*  09 Configure the Encoders */

	/* output port already selected during at BHDM_Open */

	/*CP*  09.5 Configure the BCH Encoder    */
	Register =
	    /* clear DISABLE_PARITY bit  - reset value */
		  BCHP_FIELD_DATA(HDMI_BCH_CONFIGURATION, DISABLE_PARITY, 0)
	    /* set   FEEDBACK_MASK = 8'h83   - reset value */
		| BCHP_FIELD_DATA(HDMI_BCH_CONFIGURATION, FEEDBACK_MASK,  0x83) ;
	BREG_Write32(hRegister, BCHP_HDMI_BCH_CONFIGURATION + ulOffset, Register) ;

	/*RK*  14 Configure PERT Off */
	BREG_Write32(hRegister, BCHP_HDMI_PERT_CONFIG + ulOffset, (uint32_t) 0) ;


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER

	if (NewHdmiSettings->eOutputFormat == BHDM_OutputFormat_eHDMIMode)
	{
		/*CP*  13 Enable/Configure RAM Packets */
		BHDM_CHECK_RC(rc, BHDM_InitializePacketRAM(hHDMI)) ;


		/* configure audio input to HDMI */
		BHDM_P_ConfigureInputAudioFmt(hHDMI, &NewHdmiSettings->stAudioInfoFrame) ;

		/*
		*CP*  12 Configure Audio Clock Regeneration Packet
		** Audio Clock Regeneration Packet now re-configured
		** upon Video Rate Change Callback
		*/

		/* all configuration complete; request and switch to HDMI mode	*/
		Register = BREG_Read32(hRegister, BCHP_HDMI_SCHEDULER_CONTROL + ulOffset) ;
		Register &= ~(BCHP_MASK(HDMI_SCHEDULER_CONTROL, HDMI_MODE_REQUEST)
				| BCHP_MASK(HDMI_SCHEDULER_CONTROL, USE_POST_LINE_KEEP_OUT)
				| BCHP_MASK(HDMI_SCHEDULER_CONTROL, POST_LINE_KEEP_OUT));
		Register |=
			( BCHP_FIELD_DATA(HDMI_SCHEDULER_CONTROL, HDMI_MODE_REQUEST, 1)
			| BCHP_FIELD_DATA(HDMI_SCHEDULER_CONTROL, USE_POST_LINE_KEEP_OUT, 0)
			| BCHP_FIELD_DATA(HDMI_SCHEDULER_CONTROL, POST_LINE_KEEP_OUT, 48)
			| BCHP_FIELD_DATA(HDMI_SCHEDULER_CONTROL, reserved0, 0) ) ;
		BREG_Write32(hRegister, BCHP_HDMI_SCHEDULER_CONTROL + ulOffset, (uint32_t) Register) ;


		/* wait for confirmation of transition to HDMI mode */
		HdmiModeSet = 0 ;

		/* number of frames to delay while waiting to switch from DVI to HDMI mode */
#define BHDM_P_TRANSITION_TIMEOUT_FRAMES 5
		timeoutFrames = BHDM_P_TRANSITION_TIMEOUT_FRAMES ;

		pVideoInfo = BFMT_GetVideoFormatInfoPtr(NewHdmiSettings->eInputVideoFmt) ;

		/* configure wait delay based on the referesh rate */
		if (pVideoInfo->ulVertFreq >= 5994)
			FrameDelay = 17 ;
		else if (pVideoInfo->ulVertFreq >= 5000)
			FrameDelay = 20 ;
		else if (pVideoInfo->ulVertFreq >= 2997)
			FrameDelay = 33 ;
		else /* 24/25 Hz refresh rate */
			FrameDelay = 42 ;

		BHDM_ClearHotPlugInterrupt(hHDMI);

#if BHDM_CONFIG_FORMAT_DETECT_HSYNC_SATURATION_SUPPORT

		saturatedHsyncFallCount = 0 ;
		saturatedHsyncRiseCount = 0 ;

		do
		{
			Register = BREG_Read32(hRegister, BCHP_HDMI_SCHEDULER_CONTROL + ulOffset) ;
			HdmiModeSet = (uint8_t) BCHP_GET_FIELD_DATA(Register, HDMI_SCHEDULER_CONTROL, HDMI_MODE_ACTIVE) ;
			if (HdmiModeSet)
				break ;

			/* clear format detection status */
			BREG_Write32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR + ulOffset, 0xFFFFFFFF) ;
			BREG_Write32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_CLEAR + ulOffset, 0x0) ;

			BKNI_Sleep(FrameDelay) ;

			/* if Hsync Rise/Fall saturates, there is no data to the HDMI core */
			Register = BREG_Read32(hRegister, BCHP_HDMI_FORMAT_DET_UPDATE_STATUS + ulOffset) ;

			saturatedHsyncFallCount += BCHP_GET_FIELD_DATA(
				Register, HDMI_FORMAT_DET_UPDATE_STATUS, SATURATED_HSYNC_FALL) ;
			saturatedHsyncRiseCount += BCHP_GET_FIELD_DATA(
				Register, HDMI_FORMAT_DET_UPDATE_STATUS, SATURATED_HSYNC_RISE) ;


			/* Break out if HPD was pulsed. */
			BHDM_CheckHotPlugInterrupt(hHDMI, &bHPInterrupt);
			if (bHPInterrupt == true)
				break ;

		} while ( --timeoutFrames ) ;

		if (!HdmiModeSet)
		{
			BDBG_ERR(("Tx%d: ***************************", hHDMI->eCoreId)) ;
			BDBG_ERR(("Tx%d: Saturated HSync", hHDMI->eCoreId)) ;
			BDBG_ERR(("Tx%d:     Fall Count: %d Rise Count: %d",
				hHDMI->eCoreId, saturatedHsyncFallCount, saturatedHsyncRiseCount)) ;

			BDBG_ERR(("Tx%d: Appears video data is not arriving at HDMI core", hHDMI->eCoreId)) ;
			BDBG_ERR(("Tx%d: ***************************", hHDMI->eCoreId)) ;
		}
		else
		{
			/* debug messages to see how long before video starts */
			BDBG_MSG(("Tx%d: Saturated HSync", hHDMI->eCoreId)) ;
			BDBG_MSG(("Tx%d:     Fall Count: %d Rise Count: %d",
				hHDMI->eCoreId, saturatedHsyncFallCount, saturatedHsyncRiseCount)) ;
		}


#else /* BHDM_CONFIG_FORMAT_DETECT_HSYNC_SATURATION_SUPPORT */

		do
		{
			Register = BREG_Read32(hRegister, BCHP_HDMI_SCHEDULER_CONTROL + ulOffset) ;
			HdmiModeSet = (uint8_t) BCHP_GET_FIELD_DATA(Register, HDMI_SCHEDULER_CONTROL, HDMI_MODE_ACTIVE) ;
			if (HdmiModeSet)
				break ;

			BKNI_Sleep(FrameDelay) ;

			/* Break out if HPD was pulsed. */
			BHDM_CheckHotPlugInterrupt(hHDMI, &bHPInterrupt);
			if (bHPInterrupt == true)
			{
				break ;
			}
		} while ( timeoutFrames-- ) ;
#endif
		if (!HdmiModeSet)
		{
			rc = BERR_TRACE(BERR_TIMEOUT) ;
			goto done ;
		}
	} /* END if HDMI Mode */

#if BDBG_DEBUG_BUILD
	else
		BDBG_LOG(("Tx%d: Output Mode: DVI (Video Only)", hHDMI->eCoreId)) ;
#endif

	/*
	At initial installation of the Audio Rate Change callback, there is no initial
	callback; force an update here to use settings contained in the NewHdmiSettings
	*/
	if (hHDMI->DeviceSettings.eAudioSamplingRate != NewHdmiSettings->eAudioSamplingRate)
	{
		BAVC_Audio_Info AudioData ;
		AudioData.eAudioSamplingRate = NewHdmiSettings->eAudioSamplingRate ;

		BKNI_EnterCriticalSection() ;
			BHDM_AudioVideoRateChangeCB_isr(hHDMI,
				BHDM_Callback_Type_eManualAudioChange, &AudioData) ;
		BKNI_LeaveCriticalSection() ;
	}

	/* note if the Video Format Has Changed */
	if (NewHdmiSettings->eInputVideoFmt != hHDMI->DeviceSettings.eInputVideoFmt)
	{
		hHDMI->MonitorStatus.UnstableFormatDetectedCounter = 0 ;
		HdmiVideoFormatChange = true ;
	}

ConfigureHdmiPackets:
	/* save the new HDMI Settings we used to enable the HDMI device for this HDMI handle */
	hHDMI->DeviceSettings = *NewHdmiSettings ;

	/* format and send out HDMI info packets */

	if  ((hHDMI->DeviceSettings.eOutputFormat == BHDM_OutputFormat_eHDMIMode))
	{
		/* set and enable the General Control Packet */
		/* keep AvMute at its current state in case modified earlier */
		BHDM_CHECK_RC(rc, BHDM_SetAvMute(hHDMI, hHDMI->AvMuteState)) ;

		/* set and enable the AVI Info Frame */
		BHDM_CHECK_RC(rc, BHDM_SetAVIInfoFramePacket(hHDMI,
			&hHDMI->DeviceSettings.stAviInfoFrame)) ;

		/* set and enable Gamut Metadata packets if xvYCC; clear otherwise */
		if ((hHDMI->DeviceSettings.eColorimetry == BAVC_MatrixCoefficients_eXvYCC_601)
		||  (hHDMI->DeviceSettings.eColorimetry == BAVC_MatrixCoefficients_eXvYCC_709))
			BHDM_CHECK_RC(rc, BHDM_P_SetGamutMetadataPacket(hHDMI)) ;
		else
			BHDM_CHECK_RC(rc, BHDM_DisablePacketTransmission(hHDMI, BHDM_PACKET_eGamutMetadata_ID)) ;

		/* color depth to be set directly by app/middle layer with direct call to BHDM_SetColorDepth */

		/* set and enable the Audio Info Frame */
		BHDM_CHECK_RC(rc, BHDM_SetAudioInfoFramePacket(hHDMI,
			&hHDMI->DeviceSettings.stAudioInfoFrame)) ;

		/* set and enable the Source Product Description Info Frame */
		BHDM_CHECK_RC(rc, BHDM_SetSPDInfoFramePacket(hHDMI)) ;

		/* set and enable the Vendor Specific Info Info Frame */
		BHDM_CHECK_RC(rc, BHDM_SetVendorSpecificInfoFrame(hHDMI,
			&hHDMI->DeviceSettings.stVendorSpecificInfoFrame)) ;
	}

#ifdef BCHP_HDMI_HDR_CFG
	{
		uint32_t reg;
		reg = BREG_Read32(hHDMI->hRegister, BCHP_HDMI_HDR_CFG);
		if(hHDMI->DeviceSettings.stVendorSpecificInfoFrame.bDolbyVisionEnabled)
		{
			/* set to Dolby MD mode */
			BCHP_SET_FIELD_DATA(reg, HDMI_HDR_CFG, HDR_RAM_ENABLE, 1);
			BCHP_SET_FIELD_DATA(reg, HDMI_HDR_CFG, MODE, 1); /* DOLBY */
		} else {
			/* set to Dolby MD mode */
			BCHP_SET_FIELD_DATA(reg, HDMI_HDR_CFG, HDR_RAM_ENABLE, 0);
			BCHP_SET_FIELD_DATA(reg, HDMI_HDR_CFG, MODE, 0);
		}
		BREG_Write32(hHDMI->hRegister, BCHP_HDMI_HDR_CFG, reg);
	}
#endif

	/* recenter/initialize the FIFO only if the video format has changed */
	if (HdmiVideoFormatChange || hHDMI->DeviceSettings.bForceEnableDisplay)
	{
		BDBG_MSG(("RECENTER FIFO...")) ;
		BHDM_CHECK_RC(rc, BHDM_InitializeDriftFIFO(hHDMI)) ;
	}

#if BHDM_CONFIG_DEBUG_FIFO
	{
		BHDM_P_FIFO_DATA FifoData ;

		/* dump FIFO pointers for debugging */
		BHDM_P_CaptureFIFOData(hHDMI, &FifoData) ;

		/* re-enable FIFO Interrupts */
		BHDM_P_EnableFIFOInterrupts(hHDMI, true) ;
	}
#endif
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */

#endif /* #if BHDM_CONFIG_DVO_SUPPORT */


/* enable RSEN interrupt AFTER	turning ON TMDS lines */
#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT

#if !BHDM_CONFIG_RSEN_POLLING
#if BHDM_CONFIG_DEBUG_RSEN
	BDBG_WRN(("Tx%d: Enabling RSEN Interrupts", hHDMI->eCoreId)) ;
#endif
	BINT_EnableCallback(hHDMI->hCallback[MAKE_INTR_ENUM(RSEN)]) ;
#endif

#endif

	BHDM_MONITOR_P_StartTimers(hHDMI) ;

	hHDMI->DeviceSettings.bForceEnableDisplay = false ;
	hHDMI->bForcePacketUpdates = false ;

done:
	BDBG_LEAVE(BHDM_EnableDisplay) ;
	return rc ;
}


/******************************************************************************
BERR_Code BHDM_P_DisableDisplay_isr
Summary: Disable Display output from the HDMI Tx
*******************************************************************************/
void BHDM_P_DisableDisplay_isr(
   const BHDM_Handle hHDMI  /* [in] HDMI handle */
)
{
	BDBG_ENTER(BHDM_P_DisableDisplay_isr) ;

#if defined(BHDM_GFX_PERSIST)
	return;
#endif

	BHDM_MONITOR_P_StopTimers_isr(hHDMI) ;

/* disable RSEN interrupt BEFORE  turning OFF TMDS lines */
#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT

#if !BHDM_CONFIG_RSEN_POLLING
#if BHDM_CONFIG_DEBUG_RSEN
	BDBG_WRN(("Tx%d: Disabling RSEN Interrupts", hHDMI->eCoreId)) ;
#endif
	BINT_DisableCallback_isr(hHDMI->hCallback[MAKE_INTR_ENUM(RSEN)]) ;
#endif

#endif

	/* turn off the TMDS clock/data lines */
	BHDM_P_EnableTmdsData_isr(hHDMI, false) ;
	BHDM_P_EnableTmdsClock_isr(hHDMI, false) ;

	/* reset the HDMI core to DVI Mode whenever disconnected */
	BHDM_P_ResetHdmiScheduler_isrsafe(hHDMI) ;

	BHDM_P_SetDisplayStartupDefaults_isr(hHDMI) ;

	BDBG_LEAVE(BHDM_P_DisableDisplay_isr) ;
}


/******************************************************************************
BERR_Code BHDM_DisableDisplay
Summary: Disable Display output from the HDMI Tx
*******************************************************************************/
BERR_Code BHDM_DisableDisplay(
   const BHDM_Handle hHDMI  /* [in] HDMI handle */
)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_DisableDisplay) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI ) ;

#if defined(BHDM_GFX_PERSIST)
	return BERR_SUCCESS;
#endif

	BKNI_EnterCriticalSection() ;
		BHDM_P_DisableDisplay_isr(hHDMI) ;
	BKNI_LeaveCriticalSection() ;

	/* Power Off the Phy */
	if (hHDMI->phyPowered)
	{
		BDBG_MSG(("Power OFF HDMI Phy at %d", __LINE__)) ;
		BHDM_P_PowerOffPhy(hHDMI) ;
	}


	BDBG_LEAVE(BHDM_DisableDisplay) ;

	return rc ;
}


/******************************************************************************
BERR_Code BHDM_Close
Summary: Close the HDMI connection to the HDMI Rx.
*******************************************************************************/
BERR_Code BHDM_Close(
   BHDM_Handle hHDMI  /* [in] HDMI handle */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint32_t i ;
	BHDM_EDID_P_VideoDescriptor *pVideoDescriptor ;


	BDBG_ENTER(BHDM_Close) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI ) ;

#if defined(BHDM_GFX_PERSIST)
	return BERR_SUCCESS;
#endif

	/* if in standby, power up HDMI Tx clock to configure core off */
#if BCHP_PWR_RESOURCE_HDMI_TX_CLK || BCHP_PWR_RESOURCE_HDMI_TX_1_CLK
	if (hHDMI->standby)
	{
		BCHP_PWR_AcquireResource(hHDMI->hChip, hHDMI->clkPwrResource[hHDMI->eCoreId]);
	}
#endif

#if BHDM_HAS_HDMI_20_SUPPORT
	BHDM_SCDC_DisableScrambleTx(hHDMI) ;

	rc = BHDM_AUTO_I2C_P_FreeResources(hHDMI) ;
	/* if error, dump trace and continue close of HDMI */
	if (rc) {rc = BERR_TRACE(rc) ;}
#endif

	/* make sure display is disabled  */
	if (hHDMI->DeviceStatus.tmds.clockEnabled || hHDMI->DeviceStatus.tmds.dataEnabled)
	{
		BHDM_DisableDisplay(hHDMI);
	}

	rc = BTMR_StopTimer(hHDMI->TimerHotPlugChange) ;
	/* if error, dump trace and continue close of HDMI */
	if (rc) {rc = BERR_TRACE(rc) ;}
	hHDMI->HpdTimerEnabled = false ;

	BHDM_P_FreeTimers(hHDMI) ;

	BHDM_P_DisableInterrupts(hHDMI) ;

	/* Destroy the HDMI Callbacks */
	for ( i = 0; i < MAKE_INTR_ENUM(LAST); i++ )
	{
		/* all interrupts are now created; destroy all on close */
		rc = BINT_DestroyCallback( hHDMI->hCallback[i] ) ;
		/* if error, dump trace and continue close of HDMI */
		if (rc) {rc = BERR_TRACE(rc) ;}
	}

	/* reset HDCP registers (variables don't matter here) to their initial state */
	BKNI_EnterCriticalSection() ;
		/* Initialize/Reset HDCP Settings */
		BHDM_HDCP_P_ResetSettings_isr(hHDMI) ;
	BKNI_LeaveCriticalSection() ;


	/* Destroy the Events */
	BKNI_DestroyEvent((hHDMI->BHDM_EventHDCP)) ;
	BKNI_DestroyEvent((hHDMI->BHDM_EventHotPlug)) ;
	BKNI_DestroyEvent((hHDMI->BHDM_EventFIFO)) ;
	BKNI_DestroyEvent((hHDMI->BHDM_EventRAM)) ;
	BKNI_DestroyEvent((hHDMI->BHDM_EventHDCPRiValue)) ;
	BKNI_DestroyEvent((hHDMI->BHDM_EventHDCPPjValue)) ;
	BKNI_DestroyEvent((hHDMI->BHDM_EventHDCPRepeater)) ;
	BKNI_DestroyEvent((hHDMI->BHDM_EventRxSense)) ;
	BKNI_DestroyEvent((hHDMI->BHDM_EventScramble)) ;
	BKNI_DestroyEvent((hHDMI->BHDM_EventAvRateChange)) ;

#if BHDM_CONFIG_HAS_HDCP22
	BHDM_P_ReleaseHDCP22_Resources(hHDMI) ;
#endif

	/* Used only for legacy 65nm. not required for 28nm and 40nm */
#ifdef BCHP_PWR_RESOURCE_HDMI_TX_CEC
	/* release the CEC	*/
	rc = BCHP_PWR_ReleaseResource(hHDMI->hChip, BCHP_PWR_RESOURCE_HDMI_TX_CEC);
	/* if error, dump trace and continue close of HDMI */
	if (rc) {rc = BERR_TRACE(rc) ;}
#endif

	/*  power down the TX Clock */
#if BCHP_PWR_RESOURCE_HDMI_TX_CLK || BCHP_PWR_RESOURCE_HDMI_TX_1_CLK
	rc = BCHP_PWR_ReleaseResource(hHDMI->hChip, hHDMI->clkPwrResource[hHDMI->eCoreId]);
	/* if error, dump trace and continue close of HDMI */
	if (rc) {rc = BERR_TRACE(rc) ;}
#endif

	/* delete previous video descriptors if they exist */
	if (!BLST_Q_EMPTY(&hHDMI->AttachedEDID.VideoDescriptorList))
	{
		for (pVideoDescriptor=BLST_Q_FIRST(&hHDMI->AttachedEDID.VideoDescriptorList) ;
			pVideoDescriptor ;
			pVideoDescriptor=BLST_Q_FIRST(&hHDMI->AttachedEDID.VideoDescriptorList))
		{
			BLST_Q_REMOVE_HEAD(&hHDMI->AttachedEDID.VideoDescriptorList, link);
			BKNI_Free(pVideoDescriptor); /* free memory */
		}
	}

	/* free any create Repeater KSV List */
	if (hHDMI->HDCP_RepeaterKsvList != NULL)
	{
		BKNI_Free(hHDMI->HDCP_RepeaterKsvList) ;
	}

	/* free memory associated with the HDMI handle */
	BDBG_OBJECT_DESTROY(hHDMI, HDMI ) ;
	BKNI_Free( (void *) hHDMI) ;
	hHDMI = (BHDM_Handle) NULL ;

	BDBG_LEAVE(BHDM_Close) ;
	return rc ;
}


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
/******************************************************************************
BERR_Code BHDM_ClearHotPlugInterrupt
Summary: Cler the HotPlug Interrupt.
*******************************************************************************/
BERR_Code BHDM_ClearHotPlugInterrupt(
   const BHDM_Handle hHDMI        /* [in] HDMI handle */
)
{
	BDBG_ENTER(BHDM_ClearHotPlugInterrupt) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BHDM_P_ClearHotPlugInterrupt(hHDMI);

	BDBG_LEAVE(BHDM_ClearHotPlugInterrupt) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
BERR_Code BHDM_CheckHotPlugInterrupt
Summary: Check the HotPlug Interrupt.
*******************************************************************************/
BERR_Code BHDM_CheckHotPlugInterrupt(
	const BHDM_Handle hHDMI,		 /* [in] HDMI handle */
	uint8_t *bHotPlugInterrupt	/* [out] Interrupt asserted or not */
)
{
	BDBG_ENTER(BHDM_CheckHotPlugInterrupt) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BHDM_P_CheckHotPlugInterrupt(hHDMI, bHotPlugInterrupt) ;

	BDBG_LEAVE(BHDM_CheckHotPlugInterrupt) ;
	return BERR_SUCCESS ;
}


/******************************************************************************
BERR_Code BHDM_ResetHdmiRx
Summary: Reset HDMI/DVI Rx device (for non-compliant receivers)
*******************************************************************************/
BERR_Code BHDM_ResetHdmiRx(
   const BHDM_Handle hHDMI	    /* [in] HDMI handle */
)
{
	BERR_Code      rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_ResetHdmiRx) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BHDM_EnableTmdsData(hHDMI, false) ;
	BHDM_EnableTmdsClock(hHDMI, false) ;

	BKNI_Sleep(200) ;  /* 200 ms */

	BHDM_EnableTmdsClock(hHDMI, true) ;
	BHDM_EnableTmdsData(hHDMI, true) ;

	BDBG_LEAVE(BHDM_ResetHdmiRx) ;
	return rc ;
}


/******************************************************************************
BERR_Code BHDM_SetAvMute
Summary: Set the AvMute (True/False) functionality for HDMI.
*******************************************************************************/
BERR_Code BHDM_SetAvMute(
   const BHDM_Handle hHDMI,              /* [in] HDMI handle */
   bool bEnableAvMute              /* [in] boolean to enable/disable */
)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint32_t AvMuteFlag ;
	BERR_Code rc = BERR_SUCCESS;


	BDBG_ENTER(BHDM_SetAvMute) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* AvMute valid for HDMI only */
	if  (hHDMI->DeviceSettings.eOutputFormat != BHDM_OutputFormat_eHDMIMode)
		goto done ;

	if (!hHDMI->DeviceStatus.tmds.dataEnabled) {
		/* warn, but keep going */
		BDBG_MSG(("Tx%d: BHDM_SetAvMute called while display is disabled", hHDMI->eCoreId));
	}

	if (bEnableAvMute)
	{
		AvMuteFlag = 0x01 ; /* AvMute ON */
	}
	else
	{
		AvMuteFlag = 0x10 ; /* AvMute OFF */
	}

	BDBG_MSG(("Tx%d: AvMute %d", hHDMI->eCoreId, bEnableAvMute)) ;


#if BHDM_CONFIG_REGISTER_GCP_FOR_AV_MUTE

	/* Modify AV Mute settings */
	Register = BREG_Read32(hRegister, BCHP_HDMI_GCP_WORD_1 + ulOffset);
	Register &= ~ BCHP_MASK(HDMI_GCP_WORD_1, GCP_SUBPACKET_BYTE_0);
	Register |= BCHP_FIELD_DATA(HDMI_GCP_WORD_1, GCP_SUBPACKET_BYTE_0, AvMuteFlag);
	BREG_Write32(hRegister, BCHP_HDMI_GCP_WORD_1 + ulOffset, Register) ;

	/* Enable GCP packets */
	Register = BREG_Read32(hRegister, BCHP_HDMI_GCP_CONFIG + ulOffset);
	Register &= ~ BCHP_MASK(HDMI_GCP_CONFIG, GCP_ENABLE);
	Register |= BCHP_FIELD_DATA(HDMI_GCP_CONFIG, GCP_ENABLE,  1) ;
	BREG_Write32(hRegister, BCHP_HDMI_GCP_CONFIG + ulOffset, Register) ;

#else

#if BHDM_CONFIG_DISABLE_MUX_VSYNC_ON_AVMUTE
	{
		uint8_t vsync ;

		if (bEnableAvMute)
		{
			vsync = 0 ;
		}
		else
		{
			vsync = 1 ;
		}

		/* disable/enable MUX_VSYNC when muting/unmuting */
		Register = BREG_Read32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset) ;
		Register &= ~BCHP_MASK(HDMI_CP_CONFIG, I_MUX_VSYNC) ;
		Register |= BCHP_FIELD_DATA(HDMI_CP_CONFIG, I_MUX_VSYNC, vsync) ;
		BREG_Write32(hRegister, BCHP_HDMI_CP_CONFIG + ulOffset, Register) ;
	}
#endif


	/* Disable the GCP (RAM_PACKET_0) in the Active RAM */
	/* Preserve other Active RAM Packets */
	BHDM_CHECK_RC(rc, BHDM_DisablePacketTransmission(hHDMI, BHDM_PACKET_eGCP_ID)) ;

	/* Modify GCP Registers in Packet RAM */
	Register =
		  BCHP_FIELD_DATA(HDMI_RAM_GCP_0, HEADER_BYTE_0,  0x03)
		| BCHP_FIELD_DATA(HDMI_RAM_GCP_0, HEADER_BYTE_1,  0x00)
		| BCHP_FIELD_DATA(HDMI_RAM_GCP_0, HEADER_BYTE_2,  0x00) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_GCP_0 + ulOffset, Register) ;

	Register =
		  BCHP_FIELD_DATA(HDMI_RAM_GCP_1, SUBPACKET_1_BYTE_3,  0x00)
		| BCHP_FIELD_DATA(HDMI_RAM_GCP_1, SUBPACKET_1_BYTE_2,  0x00)
		| BCHP_FIELD_DATA(HDMI_RAM_GCP_1, SUBPACKET_1_BYTE_1,  0x00)
		| BCHP_FIELD_DATA(HDMI_RAM_GCP_1, SUBPACKET_1_BYTE_0, AvMuteFlag) ;
	/* Write all four identical GCP subpackets bytes 3-0 */
	BREG_Write32(hRegister, BCHP_HDMI_RAM_GCP_1 + ulOffset, Register) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_GCP_3 + ulOffset, Register) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_GCP_5 + ulOffset, Register) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_GCP_7 + ulOffset, Register) ;

	Register =
		  BCHP_FIELD_DATA(HDMI_RAM_GCP_2, SUBPACKET_1_BYTE_6,  0x00)
		| BCHP_FIELD_DATA(HDMI_RAM_GCP_2, SUBPACKET_1_BYTE_5,  0x00)
		| BCHP_FIELD_DATA(HDMI_RAM_GCP_2, SUBPACKET_1_BYTE_4,  0x00)
		| BCHP_FIELD_DATA(HDMI_RAM_GCP_2, SUBPACKET_1_PARITY_BYTE_BCH_64_56,  0x00) ;

	/* Write all four identical GCP subpackets bytes 6-4 */
	BREG_Write32(hRegister, BCHP_HDMI_RAM_GCP_2 + ulOffset, Register) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_GCP_4 + ulOffset, Register) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_GCP_6 + ulOffset, Register) ;
	BREG_Write32(hRegister, BCHP_HDMI_RAM_GCP_8 + ulOffset, Register) ;

	/* Re-Enable the GCP in the Active RAM */
	/* Preserve current Active RAM packets */
	BHDM_CHECK_RC(rc, BHDM_EnablePacketTransmission(hHDMI, BHDM_PACKET_eGCP_ID)) ;

#endif

	hHDMI->AvMuteState = bEnableAvMute ;

done:
	BDBG_LEAVE(BHDM_SetAvMute) ;
	return rc ;
}  /* END BHDM_SetAvMute */


/***************************************************************************
BHDM_P_ConfigurePixelRepeater
Summary: Configure the Pixel Repeat Register
****************************************************************************/
static BERR_Code BHDM_P_ConfigurePixelRepeater(
	const BHDM_Handle hHDMI,           /* [in] HDMI handle */
	BFMT_VideoFmt eVideoFmt, /* [in] Input Display Format */
	BAVC_HDMI_PixelRepetition ePixelRepetition /* [in] Input Pixel Repetition */
)
{
	BERR_Code      rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint8_t pixelRepeat;

	BDBG_ENTER(BHDM_P_ConfigurePixelRepeater) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;


#ifdef BHDM_CONFIG_BLURAY_PLATFORMS
	pixelRepeat = ePixelRepetition;
#else
	/**********************
	VEC will handle pixel replication up to 54Mhz only. For 480p 4x pixel repetition
	(applicable for 7420 and later only) 27x4=108Mhz, HDMI core will handle a 2:1 pixel
	replication from the 54Mhz input from VDC
	***********************/
	if (ePixelRepetition == BAVC_HDMI_PixelRepetition_e4x)
		pixelRepeat = 1;
	else
		/* Actual pixel repeat is done by the VEC up to 54Mhz  */
		pixelRepeat = 0 ;
#endif


	switch (eVideoFmt)
	{

	case BFMT_VideoFmt_eNTSC :
	case BFMT_VideoFmt_eNTSC_J :         /* 480i (Japan) */

#if BHDM_CONFIG_ANALOG_TRANSLATION_SUPPORT
	case BFMT_VideoFmt_e720x482_NTSC :	 /* 720x482i NSTC-M for North America */
   	case BFMT_VideoFmt_e720x482_NTSC_J : /* 720x482i Japan */
	case BFMT_VideoFmt_e720x483p :			/* 720x483p */
#endif

	/* PAL Formats */
	case BFMT_VideoFmt_ePAL_B  :
	case BFMT_VideoFmt_ePAL_B1 :
	case BFMT_VideoFmt_ePAL_D  :
	case BFMT_VideoFmt_ePAL_D1 :
	case BFMT_VideoFmt_ePAL_G  :
	case BFMT_VideoFmt_ePAL_H  :
	case BFMT_VideoFmt_ePAL_I  :
	case BFMT_VideoFmt_ePAL_K  :
	case BFMT_VideoFmt_ePAL_M  :
	case BFMT_VideoFmt_ePAL_N  :
	case BFMT_VideoFmt_ePAL_NC :
	case BFMT_VideoFmt_eSECAM :  /* 576i LDK/SECAM (France :Russia) */

		/* For above formats Pixel Repeat Handled by VEC */
		/* Pixel Repeat Register should still be 0 */

	case BFMT_VideoFmt_e1080i  :
	case BFMT_VideoFmt_e720p   :
	case BFMT_VideoFmt_e480p   :

	case BFMT_VideoFmt_eDVI_640x480p :     /* DVI Safe mode for computer monitors */

	case BFMT_VideoFmt_e1080i_50Hz :       /* HD 1080i 50Hz (Europe) */
	case BFMT_VideoFmt_e720p_50Hz :        /* HD 720p 50Hz (Australia) */
	case BFMT_VideoFmt_e576p_50Hz :        /* HD 576p 50Hz (Australia) */
	case BFMT_VideoFmt_e1250i_50Hz :

	case BFMT_VideoFmt_e720p_24Hz :
	case BFMT_VideoFmt_e1080p_24Hz :      /* HD 1080p 24Hz, 2750 sample per line, SMPTE 274M-1998 */
	case BFMT_VideoFmt_e1080p_25Hz :      /* HD 1080p 25Hz, 2640 sample per line, SMPTE 274M-1998 */
	case BFMT_VideoFmt_e1080p_30Hz :      /* HD 1080p 30Hz, 2200 sample per line, SMPTE 274M-1998 */
	case BFMT_VideoFmt_e1080p_50Hz :	  /* HD 1080p 50Hz, 2200 sample per line, SMPTE 274M-1998 */
	case BFMT_VideoFmt_e1080p :			  /* HD 1080p 60Hz, 2200 sample per line, SMPTE 274M-1998 */

	/* 3D Formats */
	case BFMT_VideoFmt_e720p_60Hz_3DOU_AS:
	case BFMT_VideoFmt_e720p_50Hz_3DOU_AS:
	case BFMT_VideoFmt_e720p_24Hz_3DOU_AS:
	case BFMT_VideoFmt_e720p_30Hz_3DOU_AS:
	case BFMT_VideoFmt_e1080p_24Hz_3DOU_AS:
	case BFMT_VideoFmt_e1080p_30Hz_3DOU_AS:

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
	case BFMT_VideoFmt_e3840x2160p_30Hz :            /* 3840x2160 30Hz */
	case BFMT_VideoFmt_e3840x2160p_25Hz :            /* 3840x2160 25Hz */
	case BFMT_VideoFmt_e3840x2160p_24Hz :            /* 3840x2160 24Hz */
	case BFMT_VideoFmt_e4096x2160p_24Hz :            /* 4096x2160 24Hz */
	case BFMT_VideoFmt_e1080p_100Hz :                /* 1080p 100Hz */
	case BFMT_VideoFmt_e1080p_120Hz :                /* 1080p 120Hz */
#endif

#if BHDM_CONFIG_4Kx2K_60HZ_SUPPORT
	case BFMT_VideoFmt_e3840x2160p_50Hz :            /* 3840x2160 50Hz */
	case BFMT_VideoFmt_e3840x2160p_60Hz :            /* 3840x2160 60Hz */
#endif

#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
	case BFMT_VideoFmt_e1080p_24Hz_3D :	  /* HD 1080p 24Hz 3D, 2750 sample per line */
	case BFMT_VideoFmt_e720p_3D :	          /* HD 720p 60Hz 3D, 1650 sample per line */
	case BFMT_VideoFmt_e720p_50Hz_3D :	  /* HD 720p 50Hz 3D, 1980 sample per line */
#endif
		break ;

	default :
		/* HDMI can now support PC formats other than VGA  */
		/* if not specified as a PC format return INVALID PARAMETER */

		if (! BFMT_IS_VESA_MODE(eVideoFmt))
		{
			BDBG_ERR(("Tx%d: Invalid eVideoFmt : %d", hHDMI->eCoreId, eVideoFmt)) ;
			rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
			goto done ;
		}
	}

	/* Set Pixel Repititions value / Preserve other HDMI_MISC_CONTROL bits */
	Register = BREG_Read32(hRegister, BCHP_HDMI_MISC_CONTROL + ulOffset) ;
	Register &= ~BCHP_MASK(HDMI_MISC_CONTROL, PIXEL_REPEAT) ;
	Register |= BCHP_FIELD_DATA(HDMI_MISC_CONTROL, PIXEL_REPEAT, pixelRepeat) ;
	BREG_Write32(hRegister, BCHP_HDMI_MISC_CONTROL + ulOffset, Register);

	BDBG_MSG(("Tx%d: Pixel Repeater Value: %d Register: %X",
		pixelRepeat, hHDMI->eCoreId, Register)) ;

done:
	BDBG_LEAVE(BHDM_P_ConfigurePixelRepeater) ;
	return rc ;
}


static void BHDM_StartDriftFIFORecenter_isrsafe(
   const BHDM_Handle hHDMI		/* [in] HDMI handle */
)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, reserved0) ;
#if BCHP_HDMI_FIFO_CTL_reserved1_MASK
		Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, reserved1) ;
#endif
		Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, RECENTER) ;
	BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register);

		Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, RECENTER, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register);

#if BHDM_CONFIG_DEBUG_FIFO
	BDBG_WRN(("Tx%d: <RECENTER FIFO>", hHDMI->eCoreId)) ;
#endif
}


/******************************************************************************
BERR_Code BHDM_InitializeDriftFIFO
Summary: Initialize the Drift FIFO
*******************************************************************************/
BERR_Code BHDM_InitializeDriftFIFO(
   const BHDM_Handle hHDMI		/* [in] HDMI handle */
)
{
	BERR_Code   rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;
	uint8_t timeoutMs ;
	uint8_t RecenterDone ;
	uint8_t bHPInterrupt = false ;
	uint32_t timeDelayed;
	bool masterMode;
	bool bAuthenticated ;

	BDBG_ENTER(BHDM_InitializeDriftFIFO) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	/* print debug message anytime FIFO is recentered */
	/* set BHDM_CONFIG_DEBUG_FIFO for more debug info */
	BDBG_MSG(("Start <RECENTER FIFO>...")) ;

	/* do not recenter FIFO if HDCP is enabled */
	rc = BHDM_HDCP_IsLinkAuthenticated(hHDMI, &bAuthenticated) ;
	if (rc)
	{
		rc = BERR_TRACE(rc) ;
		goto done ;
	}

	if (bAuthenticated)
	{
		BDBG_WRN(("Tx%d: HDCP is enabled ; <RECENTER FIFO> aborted...", hHDMI->eCoreId)) ;
		goto done ;
	}

	rc = BHDM_GetHdmiDataTransferMode(hHDMI, &masterMode);
	if (rc)
	{
		BDBG_ERR(("Tx%d: Unable to determine HDMI Transfer Mode; defaulting to Slave",
			hHDMI->eCoreId)) ;
		masterMode = 0 ;  /* default to Slave mode */
		rc = BERR_TRACE(rc) ;
	}

	/**** 06 Recenter the Drift FIFO */
	/*
	**	Read the  HDMI_FIFO_CTL Register...
	**	Clear the RECENTER bit
	**	Write the HDMI_FIFO_CTL Register
	**	Set   the RECENTER bit
	**	Read      RECENTER_DONE bit until equal to one
	*/

	/* set   the FIFO CTL RECENTER bit */
	if (!masterMode)
	{
		BHDM_StartDriftFIFORecenter_isrsafe(hHDMI) ;

		BHDM_ClearHotPlugInterrupt(hHDMI);
		for (timeDelayed=0; timeDelayed<1000; timeDelayed+=50)
		{
			BKNI_Delay(50) ;

			/* Bail out if HPD was pulsed. */
			BHDM_CheckHotPlugInterrupt(hHDMI, &bHPInterrupt);
			if (bHPInterrupt == true)
				break ;
		}

		/* set   the FIFO CTL RECENTER bit */
		BHDM_StartDriftFIFORecenter_isrsafe(hHDMI) ;


		/* Wait for RECENTER_DONE bit to be equal to one */
		timeoutMs = 10 ;
		do
		{
			Register = BREG_Read32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset) ;
			RecenterDone = BCHP_GET_FIELD_DATA(Register, HDMI_FIFO_CTL, RECENTER_DONE) ;
			if (RecenterDone)
			    break ;

			#if BHDM_CONFIG_DEBUG_FIFO
			BDBG_WRN(("Tx%d: Wait for <RECENTER FIFO> %dms", hHDMI->eCoreId, timeoutMs)) ;
			#endif

			BKNI_Sleep(1) ;
		} while ( timeoutMs-- ) ;

		if (!RecenterDone)
		{
			BDBG_ERR(("Tx%d: <RECENTER FIFO> Timed out...", hHDMI->eCoreId)) ;
			rc = BERR_TRACE(BERR_TIMEOUT) ;
			goto done ;
		}

		#if BHDM_CONFIG_DEBUG_FIFO
		BDBG_WRN(("Tx%d: <RECENTER FIFO> done", hHDMI->eCoreId)) ;
		#endif
	}

	Register = BREG_Read32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset) ;
		Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, USE_FULL) ;
		Register &= ~ BCHP_MASK(HDMI_FIFO_CTL, USE_EMPTY) ;

		/* USE_FULL should always be set to 0 in master mode */
		Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_FULL, masterMode ? 0 : 1) ;
		Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_EMPTY, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register);

done:
	BDBG_LEAVE(BHDM_InitializeDriftFIFO) ;
	return rc ;
}


/******************************************************************************
BERR_Code BHDM_CheckForValidVideo
Summary: Check the input video display format to the HDMI core is enabled/stable
*******************************************************************************/
BERR_Code BHDM_CheckForValidVideo(
	const BHDM_Handle hHDMI              /* [in] HDMI handle */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	bool bMasterMode ;
	BHDM_P_FIFO_DATA FifoData ;
	uint8_t uiInitialRdAddr , uiInitialWrAddr ;
	uint8_t uiCurrentRdAddr , uiCurrentWrAddr ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(&FifoData, 0, sizeof(BHDM_P_FIFO_DATA));

	/* make sure PLL is up/running */
#if BHDM_CONFIG_65NM_SUPPORT
	{
		BREG_Handle hRegister ;
		uint32_t Register, ulOffset ;
		bool bPllPoweredDown ;

		hRegister = hHDMI->hRegister ;
		ulOffset = hHDMI->ulOffset;

		/* check if pll is powered on */
		Register = BREG_Read32(hRegister, BCHP_HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL + ulOffset) ;

		/* use bUnderFlow variable to check pll pwrdn status */
		bPllPoweredDown = BCHP_GET_FIELD_DATA(Register, HDMI_TX_PHY_HDMI_TX_PHY_RESET_CTL, PLL_PWRDN) ;
		if (bPllPoweredDown)
		{
			BDBG_ERR(("Tx%d: PLL is powered down; HDCP Authentication cannot proceed", hHDMI->eCoreId)) ;
			rc = BERR_TRACE(BHDM_HDCP_PLL_PWRDN) ;
			goto done ;
		}

		BDBG_MSG(("Tx%d: PLL power... OK", hHDMI->eCoreId)) ;
	}
#endif

        /* Check Drift FIFO, but do so only if we're in Slave mode */
	rc = BHDM_GetHdmiDataTransferMode(hHDMI, &bMasterMode);
	if (rc)
	{
		BDBG_ERR(("Tx%d: Unable to determine Master Mode setting", hHDMI->eCoreId)) ;
		rc = BERR_TRACE(rc) ;
		goto done ;
	}

	if (bMasterMode)
	{
		BDBG_MSG(("Tx%d: HDMI core is in Master Mode No need to check for valid video", hHDMI->eCoreId)) ;
		rc = BERR_SUCCESS ;
		goto done ;
	}


	{
		uint8_t uiInitialFullness, uiCurrentFullness ;

		BHDM_P_CaptureFIFOData(hHDMI, &FifoData) ;
			uiInitialFullness = FifoData.uiFullness ;
			uiInitialRdAddr = FifoData.uiRdAddr ;
			uiInitialWrAddr = FifoData.uiWrAddr ;

		if ((FifoData.bUnderFlow) || (FifoData.bOverFlow))
		{
			if ( (uiInitialFullness > 178) || (uiInitialFullness < 78 ))
			{
				BDBG_WRN(("Tx%d: FIFO is railed, fullness: %d give up and try again later",
					hHDMI->eCoreId, uiInitialFullness)) ;
				BHDM_InitializeDriftFIFO(hHDMI);
				rc = BERR_TRACE(BHDM_HDCP_FIFO_UNDERFLOW) ;
				goto done ;
			}

			 /* valid FIFO */
		 	/* confirm current fullness/stability */
			BHDM_P_CaptureFIFOData(hHDMI, &FifoData) ;
				uiCurrentFullness = FifoData.uiFullness ;
				uiCurrentRdAddr = FifoData.uiRdAddr ;
				uiCurrentWrAddr = FifoData.uiWrAddr ;

			if ((uiInitialRdAddr == uiCurrentRdAddr)
			|| (uiInitialWrAddr == uiCurrentWrAddr))
			{
				BDBG_ERR(("Tx%d: Video to HDMI Core is stalled...", hHDMI->eCoreId)) ;
				rc = BERR_TRACE(BHDM_UNSUPPORTED_VIDEO_FORMAT) ;
				goto done ;
			}

#ifndef ABS
#define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

			if (ABS(uiCurrentFullness - uiInitialFullness) > 6)
			{
				BDBG_WRN(("Tx%d: FIFO is unstable (1) ; Initial Fullness: %d Current Fullness %d, recenter and try again",
					hHDMI->eCoreId, uiInitialFullness, uiCurrentFullness)) ;
				rc = BHDM_InitializeDriftFIFO(hHDMI);
				if (rc)
				{
					BDBG_ERR(("Unable to Recenter FIFO after detecting a FIFO UNDERFLOW")) ;
					rc = BERR_TRACE(BHDM_HDCP_FIFO_UNDERFLOW) ;
				}
				rc = BHDM_HDCP_FIFO_UNDERFLOW ;
				goto done ;
 			 }


			/* confirm current fullness/stability */
			BHDM_P_CaptureFIFOData(hHDMI, &FifoData) ;
				uiInitialFullness = FifoData.uiFullness ;
				uiInitialRdAddr = FifoData.uiRdAddr ;
				uiInitialWrAddr = FifoData.uiWrAddr ;

			if (ABS(uiCurrentFullness - uiInitialFullness) > 6)
			{
				BDBG_WRN(("Tx%d: FIFO is unstable (2) ; Initial Fullness: %d Current Fullness %d, recenter and try again",
					hHDMI->eCoreId, uiInitialFullness, uiCurrentFullness)) ;
				rc = BHDM_InitializeDriftFIFO(hHDMI) ;
				if (rc)
				{
					BDBG_ERR(("Unable to Recenter FIFO after detecting a FIFO UNDERFLOW")) ;
					rc = BERR_TRACE(BHDM_HDCP_FIFO_UNDERFLOW) ;
				}
				rc = BHDM_HDCP_FIFO_UNDERFLOW ;
				goto done ;
			}
		}
	}

	/* FIFO is centered if between 122 <= x <= 134 */
	if (!((FifoData.uiFullness >= 122) &&  (FifoData.uiFullness <= 134)))
	{
		BDBG_WRN(("Tx%d: FIFO fullness %d is not in range  122 <= x <= 134 ; <RECENTER FIFO>",
			hHDMI->eCoreId, FifoData.uiFullness)) ;
		rc = BHDM_InitializeDriftFIFO(hHDMI) ;
		if (rc)
		{
			BDBG_ERR(("Unable to Recenter FIFO after detecting a FIFO UNDERFLOW")) ;
			rc = BERR_TRACE(rc) ;
		}
		rc = BHDM_HDCP_FIFO_UNDERFLOW ;
		goto done ;
	}
	BDBG_MSG(("Tx%d: Video data to HDMI Core... OK", hHDMI->eCoreId)) ;


done:
	return rc ;

}
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */


void BHDM_P_RxDeviceAttached_isr(
    const BHDM_Handle hHDMI,         /* [in] HDMI handle */
    uint8_t *bDeviceAttached /* [out] Device Attached Status  */
)
{
    BREG_Handle hRegister ;
    uint32_t Register, ulOffset ;

    BDBG_ENTER(BHDM_P_RxDeviceAttached_isr) ;

    hRegister = hHDMI->hRegister ;
    ulOffset = hHDMI->ulOffset ;

	if (hHDMI->bCrcTestMode)
	{
		*bDeviceAttached = true ;
	}
	else

	{
	    Register = BREG_Read32(hRegister, BCHP_HDMI_HOTPLUG_STATUS + ulOffset) ;
	    *bDeviceAttached =
	        BCHP_GET_FIELD_DATA(Register, HDMI_HOTPLUG_STATUS, HOTPLUG_STATUS) ;
	}

    BDBG_LEAVE(BHDM_P_RxDeviceAttached_isr) ;
    return ;
}


/******************************************************************************
BERR_Code BHDM_RxDeviceAttached
Summary: Check for an attached Rx Device.
*******************************************************************************/
BERR_Code BHDM_RxDeviceAttached(
	const BHDM_Handle hHDMI,		/* [in] HDMI handle */
	uint8_t *bDeviceAttached
)
{
	BERR_Code rc = BERR_SUCCESS;
	uint8_t status;

	BDBG_ENTER(BHDM_RxDeviceAttached) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* DVO interface is always attached; as opposed to HDMI Port */
	if (hHDMI->eOutputPort == BHDM_OutputPort_eDVO)
	{
		*bDeviceAttached = 1 ;
		return rc ;
	}

	/* read hotplug register and check if connected */
	BKNI_EnterCriticalSection() ;
		BHDM_P_RxDeviceAttached_isr(hHDMI, &status) ;
	BKNI_LeaveCriticalSection() ;

	if (status) {
		*bDeviceAttached = 1 ;
	}
	else {
		*bDeviceAttached = 0 ;
		BDBG_MSG(("Tx%d: RxDeviceAttached: No DVI/HDMI Device Detected", hHDMI->eCoreId)) ;
	}

	BDBG_LEAVE(BHDM_RxDeviceAttached) ;
	return rc ;
}


#if BDBG_DEBUG_BUILD
static void BHDM_P_DebugInputVideoFmtConfiguration(
	const BHDM_Handle hHDMI,		/* [in] HDMI handle */
	const BHDM_Settings *NewHdmiSettings,
	const uint8_t index) /* [in] HDMI Settings (pixel encoding, repetition) */
{
	static const char * const sPolarity[] = {"-", "+"} ;

       const BFMT_VideoInfo *pVideoInfo ;

	uint32_t tmdsRate ;
	uint8_t divider ;
	const char *bitsPerPixelStr;

	divider = (hHDMI->DeviceSettings.stVideoSettings.eColorSpace == BAVC_Colorspace_eYCbCr420)  ? 2 : 1 ;
	BDBG_MSG(("Horizontal Parameter Divider: %d", divider)) ;

       pVideoInfo = BFMT_GetVideoFormatInfoPtr(NewHdmiSettings->eInputVideoFmt) ;

	BHDM_TMDS_P_VideoFormatSettingsToTmdsRate(hHDMI,
		NewHdmiSettings->eInputVideoFmt, &NewHdmiSettings->stVideoSettings, &tmdsRate) ;


	if (NewHdmiSettings->stVideoSettings.eColorSpace == BAVC_Colorspace_eYCbCr422)
	{
		/* NOTE: there is no such thing as deep color 4:2:2 */
		/* Deep Color implies that the pixels are packed differently for different bits per component. */
		/* See HDMI 1.4 Figure 6-2 */
		bitsPerPixelStr = "36 bpp" ;
	}
	else
	{
		bitsPerPixelStr = BAVC_HDMI_BitsPerPixelToStr(NewHdmiSettings->stVideoSettings.eBitsPerPixel) ;
	}

	BDBG_LOG(("Tx%d Output: %s (%s %s) %d [%s] x %d/%d [%s], PxlClk: %dMHz (TMDS %d Mcsc)",
		hHDMI->eCoreId,
		BHDM_VideoFmtParams[index].FormatName,

		BAVC_HDMI_AviInfoFrame_ColorspaceToStr_isrsafe(NewHdmiSettings->stVideoSettings.eColorSpace),
		bitsPerPixelStr,

		BHDM_VideoFmtParams[index].H_ActivePixels / divider,
		sPolarity[BHDM_VideoFmtParams[index].V_Polarity],
		BHDM_VideoFmtParams[index].V_ActiveLinesField0,
		BHDM_VideoFmtParams[index].V_ActiveLinesField1,
		sPolarity[BHDM_VideoFmtParams[index].H_Polarity],
		pVideoInfo->ulPxlFreq / BFMT_FREQ_FACTOR, tmdsRate)) ;

	if (index == BHDM_InputVideoFmt_eCustom)
	{
		BDBG_WRN(("Tx%d: Using Custom DVI/DVO format...", hHDMI->eCoreId)) ;
	}

	BDBG_MSG(("Tx%d: Horz Front Porch: %3d ",
		hHDMI->eCoreId, BHDM_VideoFmtParams[index].H_FrontPorch / divider)) ;
	BDBG_MSG(("Tx%d: Horz Back  Porch: %3d ",
		hHDMI->eCoreId, BHDM_VideoFmtParams[index].H_BackPorch / divider )) ;
	BDBG_MSG(("Tx%d: Horz SyncPulse:   %3d ",
		hHDMI->eCoreId, BHDM_VideoFmtParams[index].H_SyncPulse / divider )) ;
	BDBG_MSG(("Tx%d:                          FIELD 0     FIELD 1", hHDMI->eCoreId)) ;
	BDBG_MSG(("Tx%d: Vert Front Porch :        %3d          %3d",
		hHDMI->eCoreId, BHDM_VideoFmtParams[index].V_FrontPorchField0,
		BHDM_VideoFmtParams[index].V_FrontPorchField1)) ;
	BDBG_MSG(("Tx%d: Vert Back  Porch :        %3d          %3d",
		hHDMI->eCoreId,
		BHDM_VideoFmtParams[index].V_BackPorchField0,
		BHDM_VideoFmtParams[index].V_BackPorchField1)) ;
	BDBG_MSG(("Tx%d: Vert Sync Pulse :         %3d          %3d",
		hHDMI->eCoreId,
		BHDM_VideoFmtParams[index].V_SyncPulseField0,
		BHDM_VideoFmtParams[index].V_SyncPulseField1)) ;
	BDBG_MSG(("Tx%d: Vert Sync Pulse Offset :  %3d          %3d",
		hHDMI->eCoreId,
		BHDM_VideoFmtParams[index].V_SyncPulseOffsetField0,
		BHDM_VideoFmtParams[index].V_SyncPulseOffsetField1)) ;
}
#endif


/******************************************************************************
BERR_Code BHDM_P_ConfigureInputVideoFmt
Summary: Set the input video display format to the HDMI core
*******************************************************************************/
static BERR_Code BHDM_P_ConfigureInputVideoFmt(
   const BHDM_Handle hHDMI,		/* [in] HDMI handle */
   const BHDM_Settings *NewHdmiSettings) /* [in] HDMI Settings (pixel encoding, repetition) */
{
	BREG_Handle hRegister ;
	BERR_Code   rc = BERR_SUCCESS;
	uint32_t Register, ulOffset ;
	uint8_t     i, index ;
	BFMT_VideoFmt eVideoFmt ;

	BAVC_HDMI_PixelRepetition  ePixelRepetition ;
	uint8_t pixelRepetitionCount;
	uint8_t divider ;


	typedef struct BHDM_BFMT_VideoFormat
	{
		BFMT_VideoFmt eVideoFmt ;
		uint8_t index ;
	} BHDM_BFMT_VideoFormat ;

	static const  BHDM_BFMT_VideoFormat BHDM_SupportedVideoFormats[] =
	{
		/* Formats for Boot loader usage mode */
#ifdef BHDM_FOR_BOOTUPDATER
		{BFMT_VideoFmt_eDVI_640x480p,  BHDM_InputVideoFmt_e640x480p}, /* DVI/HDMI Safe mode */
		{BFMT_VideoFmt_eNTSC,	BHDM_InputVideoFmt_e480i},		 /* 480i,NSTC-M for North America */
		{BFMT_VideoFmt_e480p,	BHDM_InputVideoFmt_e480p},	 /* HD 480p */
		{BFMT_VideoFmt_e720p,	BHDM_InputVideoFmt_e720p},	 /* HD 720p */
		{BFMT_VideoFmt_e1080i,	BHDM_InputVideoFmt_e1080i},  /* HD 1080i */
		{BFMT_VideoFmt_e576p_50Hz,	BHDM_InputVideoFmt_e576p_50Hz},   /* HD 576p 50Hz (Australia) */
		{BFMT_VideoFmt_ePAL_B , BHDM_InputVideoFmt_e576i_50Hz},   /* Austrilia,*/
		{BFMT_VideoFmt_ePAL_B1, BHDM_InputVideoFmt_e576i_50Hz},   /* Hungry */
		{BFMT_VideoFmt_ePAL_D , BHDM_InputVideoFmt_e576i_50Hz},   /* China */
		{BFMT_VideoFmt_ePAL_D1, BHDM_InputVideoFmt_e576i_50Hz},  /* Poland */
		{BFMT_VideoFmt_ePAL_G , BHDM_InputVideoFmt_e576i_50Hz},   /* Europe */
		{BFMT_VideoFmt_ePAL_H , BHDM_InputVideoFmt_e576i_50Hz},   /* Europe */
		{BFMT_VideoFmt_ePAL_K , BHDM_InputVideoFmt_e576i_50Hz},   /* Europe */
		{BFMT_VideoFmt_ePAL_I , BHDM_InputVideoFmt_e576i_50Hz},    /* U.K. */
		{BFMT_VideoFmt_ePAL_M , BHDM_InputVideoFmt_e480i},	 /* 480i (Brazil) */
		{BFMT_VideoFmt_ePAL_N , BHDM_InputVideoFmt_e576i_50Hz},   /* 576i (Paraguay,Uruguay)*/
		{BFMT_VideoFmt_ePAL_NC, BHDM_InputVideoFmt_e576i_50Hz},  /* 576i N combination (Argentina) */
		{BFMT_VideoFmt_eSECAM,	BHDM_InputVideoFmt_e576i_50Hz},  /* 576i LDK/SECAM (France :Russia) */

#else

		{BFMT_VideoFmt_eDVI_640x480p,  BHDM_InputVideoFmt_e640x480p}, /* DVI/HDMI Safe mode */


		{BFMT_VideoFmt_eNTSC,   BHDM_InputVideoFmt_e480i},       /* 480i,NSTC-M for North America */
		{BFMT_VideoFmt_eNTSC_J, BHDM_InputVideoFmt_e480i},   /* 480i (Japan) */
		{BFMT_VideoFmt_e480p,   BHDM_InputVideoFmt_e480p},   /* HD 480p */

#if BHDM_CONFIG_ANALOG_TRANSLATION_SUPPORT
		{BFMT_VideoFmt_e720x482_NTSC, BHDM_InputVideoFmt_e480i}, /* 720x482i NSTC-M for North America */
		{BFMT_VideoFmt_e720x482_NTSC_J, BHDM_InputVideoFmt_e480i}, /* 720x482i Japan */
		{BFMT_VideoFmt_e720x483p, BHDM_InputVideoFmt_e480p},
#endif

		{BFMT_VideoFmt_e720p,   BHDM_InputVideoFmt_e720p},   /* HD 720p */
		{BFMT_VideoFmt_e720p_24Hz,   BHDM_InputVideoFmt_e720p_24Hz},   /* HD 720p */
		{BFMT_VideoFmt_e1080i,  BHDM_InputVideoFmt_e1080i},  /* HD 1080i */

		{BFMT_VideoFmt_e1080p_24Hz,  BHDM_InputVideoFmt_e1080p_24Hz},  /* HD 1080p */
		{BFMT_VideoFmt_e1080p_25Hz,  BHDM_InputVideoFmt_e1080p_25Hz},  /* HD 1080p */
		{BFMT_VideoFmt_e1080p_30Hz,  BHDM_InputVideoFmt_e1080p_30Hz},  /* HD 1080p */

#if BHDM_CONFIG_1080P_5060HZ_SUPPORT
		{BFMT_VideoFmt_e1080p,  BHDM_InputVideoFmt_e1080p},  /* HD 1080p */
		{BFMT_VideoFmt_e1080p_50Hz,  BHDM_InputVideoFmt_e1080p_50Hz},  /* HD 1080p */
#endif

		{BFMT_VideoFmt_e1080i_50Hz, BHDM_InputVideoFmt_e1080i_50Hz},  /* HD 1080i 50Hz (Europe) */
		{BFMT_VideoFmt_e720p_50Hz,  BHDM_InputVideoFmt_e720p_50Hz},   /* HD 720p 50Hz (Australia) */
		{BFMT_VideoFmt_e576p_50Hz,  BHDM_InputVideoFmt_e576p_50Hz},   /* HD 576p 50Hz (Australia) */

		{BFMT_VideoFmt_ePAL_B , BHDM_InputVideoFmt_e576i_50Hz},   /* Austrilia,*/
		{BFMT_VideoFmt_ePAL_B1, BHDM_InputVideoFmt_e576i_50Hz},   /* Hungry */
		{BFMT_VideoFmt_ePAL_D , BHDM_InputVideoFmt_e576i_50Hz},   /* China */
		{BFMT_VideoFmt_ePAL_D1, BHDM_InputVideoFmt_e576i_50Hz},  /* Poland */
		{BFMT_VideoFmt_ePAL_G , BHDM_InputVideoFmt_e576i_50Hz},   /* Europe */
		{BFMT_VideoFmt_ePAL_H , BHDM_InputVideoFmt_e576i_50Hz},   /* Europe */
		{BFMT_VideoFmt_ePAL_K , BHDM_InputVideoFmt_e576i_50Hz},   /* Europe */
		{BFMT_VideoFmt_ePAL_I , BHDM_InputVideoFmt_e576i_50Hz},    /* U.K. */
		{BFMT_VideoFmt_ePAL_M , BHDM_InputVideoFmt_e480i},   /* 480i (Brazil) */
		{BFMT_VideoFmt_ePAL_N , BHDM_InputVideoFmt_e576i_50Hz},   /* 576i (Paraguay,Uruguay)*/
		{BFMT_VideoFmt_ePAL_NC, BHDM_InputVideoFmt_e576i_50Hz},  /* 576i N combination (Argentina) */
		{BFMT_VideoFmt_eSECAM,  BHDM_InputVideoFmt_e576i_50Hz},  /* 576i LDK/SECAM (France :Russia) */

		/* 3D formats */
		{BFMT_VideoFmt_e720p_60Hz_3DOU_AS,	BHDM_InputVideoFmt_e720p_3DOU},
		{BFMT_VideoFmt_e720p_50Hz_3DOU_AS,	BHDM_InputVideoFmt_e720p_50Hz_3DOU},
		{BFMT_VideoFmt_e720p_24Hz_3DOU_AS,	BHDM_InputVideoFmt_e720p_24Hz_3DOU},
		{BFMT_VideoFmt_e720p_30Hz_3DOU_AS,	BHDM_InputVideoFmt_e720p_30Hz_3DOU},
		{BFMT_VideoFmt_e1080p_24Hz_3DOU_AS, BHDM_InputVideoFmt_e1080p_24Hz_3DOU},
		{BFMT_VideoFmt_e1080p_30Hz_3DOU_AS, BHDM_InputVideoFmt_e1080p_30Hz_3DOU},

#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
		{BFMT_VideoFmt_e1080p_24Hz_3D,  BHDM_InputVideoFmt_e1080p_24Hz_3D},  /* HD 1080p 24Hz 3D */
		{BFMT_VideoFmt_e720p_3D,       BHDM_InputVideoFmt_e720p_3D},         /* HD 720p 60Hz 3D */
		{BFMT_VideoFmt_e720p_50Hz_3D,  BHDM_InputVideoFmt_e720p_50Hz_3D},  /* HD 720p 50Hz 3D */
#endif
		/* DVI formats */
		{BFMT_VideoFmt_eCUSTOM_1366x768p, BHDM_InputVideoFmt_eCustom},

		{BFMT_VideoFmt_eDVI_800x600p,  BHDM_InputVideoFmt_800x600p},
		{BFMT_VideoFmt_eDVI_1024x768p, BHDM_InputVideoFmt_1024x768p},
		{BFMT_VideoFmt_eDVI_1280x768p, BHDM_InputVideoFmt_1280x768p},
		{BFMT_VideoFmt_eDVI_1280x1024p_60Hz, BHDM_InputVideoFmt_1280x1024p},

		{BFMT_VideoFmt_eDVI_1280x720p_50Hz, BHDM_InputVideoFmt_1280x720p_50Hz},
		{BFMT_VideoFmt_eDVI_1280x720p, BHDM_InputVideoFmt_1280x720p}

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
		,
		{BFMT_VideoFmt_e3840x2160p_30Hz, BHDM_InputVideoFmt_3840x2160_30Hz},
		{BFMT_VideoFmt_e3840x2160p_25Hz, BHDM_InputVideoFmt_3840x2160_25Hz},
		{BFMT_VideoFmt_e3840x2160p_24Hz, BHDM_InputVideoFmt_3840x2160_24Hz},

		{BFMT_VideoFmt_e4096x2160p_24Hz, BHDM_InputVideoFmt_4096x2160_24Hz},
		{BFMT_VideoFmt_e1080p_100Hz, BHDM_InputVideoFmt_e1080p_100Hz},
		{BFMT_VideoFmt_e1080p_120Hz, BHDM_InputVideoFmt_e1080p_120Hz}
#endif

#if BHDM_CONFIG_4Kx2K_60HZ_SUPPORT
		,
		{BFMT_VideoFmt_e3840x2160p_50Hz, BHDM_InputVideoFmt_3840x2160_50Hz},
		{BFMT_VideoFmt_e3840x2160p_60Hz, BHDM_InputVideoFmt_3840x2160_60Hz},
#endif

#endif /*#ifndef BHDM_FOR_BOOTUPDATER */
	} ;


	BDBG_ENTER(BHDM_P_ConfigureInputVideoFmt) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	eVideoFmt = NewHdmiSettings->eInputVideoFmt ;
	ePixelRepetition = NewHdmiSettings->ePixelRepetition ;

	/* translate the input video fmt to an index in the HDMI internal table */
	index = BHDM_InputVideoFmt_ePowerUp ;
	for (i = 0 ; i < sizeof(BHDM_SupportedVideoFormats) / sizeof(BHDM_BFMT_VideoFormat) ; i++)
		if (eVideoFmt == BHDM_SupportedVideoFormats[i].eVideoFmt)
		{
			index = BHDM_SupportedVideoFormats[i].index ;
			break ;
		}

	if (index == BHDM_InputVideoFmt_ePowerUp)
	{
		const BFMT_VideoInfo *pVideoInfo ;

		pVideoInfo = BFMT_GetVideoFormatInfoPtr(eVideoFmt) ;
		BDBG_ERR(("Tx%d: BFMT_VideoFmt: %s (%d) is UNKNOWN/UNSUPPORTED in HDM",
			hHDMI->eCoreId, pVideoInfo->pchFormatStr, eVideoFmt)) ;
		BSTD_UNUSED(pVideoInfo) ; /* supress coverity message for non-debug builds */
		rc = BERR_TRACE(BERR_NOT_SUPPORTED) ;
		goto done ;
	}

	/* the pixel repetition count/multiply */
	pixelRepetitionCount = ePixelRepetition + 1;

	divider = (hHDMI->DeviceSettings.stVideoSettings.eColorSpace == BAVC_Colorspace_eYCbCr420)  ? 2 : 1 ;

#if BDBG_DEBUG_BUILD
	BHDM_P_DebugInputVideoFmtConfiguration(hHDMI, NewHdmiSettings, index) ;
#endif


#if BHDM_CONFIG_SCHEDULER_REV_2
	/* always load the HORZA first; locks faster */
	Register =
		(uint32_t) 0
		| BCHP_FIELD_DATA(HDMI_HORZA, MANUAL_HFP,
			(BHDM_VideoFmtParams[index].H_FrontPorch * pixelRepetitionCount) / divider)
		| BCHP_FIELD_DATA(HDMI_HORZA, MANUAL_VPOL, BHDM_VideoFmtParams[index].V_Polarity )
		| BCHP_FIELD_DATA(HDMI_HORZA, MANUAL_HPOL, BHDM_VideoFmtParams[index].H_Polarity )
		| BCHP_FIELD_DATA(HDMI_HORZA, MANUAL_HAP,
			(BHDM_VideoFmtParams[index].H_ActivePixels * pixelRepetitionCount) / divider );
	BREG_Write32(hRegister, BCHP_HDMI_HORZA + ulOffset, Register) ;

	/* Load HORZB Register */
	Register =
		(uint32_t) 0
		| BCHP_FIELD_DATA(HDMI_HORZB, MANUAL_HSP,
			(BHDM_VideoFmtParams[index].H_SyncPulse * pixelRepetitionCount) / divider )
		| BCHP_FIELD_DATA(HDMI_HORZB, MANUAL_HBP,
			(BHDM_VideoFmtParams[index].H_BackPorch * pixelRepetitionCount) / divider ) ;
	BREG_Write32(hRegister, BCHP_HDMI_HORZB + ulOffset, Register) ;

#else
	/* LEGACY */
	/* always load the HORZA first; locks faster */
	Register =
		(uint32_t) 0
		| BCHP_FIELD_DATA(HDMI_HORZA, MANUAL_VPOL, BHDM_VideoFmtParams[index].V_Polarity)
		| BCHP_FIELD_DATA(HDMI_HORZA, MANUAL_HPOL, BHDM_VideoFmtParams[index].H_Polarity)
		| BCHP_FIELD_DATA(HDMI_HORZA, MANUAL_HAP, (BHDM_VideoFmtParams[index].H_ActivePixels * pixelRepetitionCount) / divider) ;
	BREG_Write32(hRegister, BCHP_HDMI_HORZA + ulOffset, Register) ;

	/* Load HORZB Register */
	Register =
		(uint32_t) 0
		| BCHP_FIELD_DATA(HDMI_HORZB, MANUAL_HFP,
			(BHDM_VideoFmtParams[index].H_FrontPorch * pixelRepetitionCount) / divider)
		| BCHP_FIELD_DATA(HDMI_HORZB, MANUAL_HSP,
			(BHDM_VideoFmtParams[index].H_SyncPulse * pixelRepetitionCount) /divider)
		| BCHP_FIELD_DATA(HDMI_HORZB, MANUAL_HBP,
			(BHDM_VideoFmtParams[index].H_BackPorch * pixelRepetitionCount) / divider) ;
	BREG_Write32(hRegister, BCHP_HDMI_HORZB + ulOffset, Register) ;
#endif

	/* Load VERTA0 Register */
	Register =
		(uint32_t) 0
		| BCHP_FIELD_DATA(HDMI_VERTA0, MANUAL_VSP0, BHDM_VideoFmtParams[index].V_SyncPulseField0 )
		| BCHP_FIELD_DATA(HDMI_VERTA0, MANUAL_VFP0, BHDM_VideoFmtParams[index].V_FrontPorchField0 )
		| BCHP_FIELD_DATA(HDMI_VERTA0, MANUAL_VAL0, BHDM_VideoFmtParams[index].V_ActiveLinesField0 ) ;
	BREG_Write32(hRegister, BCHP_HDMI_VERTA0 + ulOffset, Register) ;


	/* Load VERTB0 Register */
	Register =
		(uint32_t) 0
		| BCHP_FIELD_DATA(HDMI_VERTB0, MANUAL_VSPO0, BHDM_VideoFmtParams[index].V_SyncPulseOffsetField0 )
		| BCHP_FIELD_DATA(HDMI_VERTB0, MANUAL_VBP0, BHDM_VideoFmtParams[index].V_BackPorchField0 ) ;
	BREG_Write32(hRegister, BCHP_HDMI_VERTB0 + ulOffset, Register) ;


	/* Load VERTA1 Register */
	Register =
		(uint32_t) 0
		| BCHP_FIELD_DATA(HDMI_VERTA1, MANUAL_VSP1, BHDM_VideoFmtParams[index].V_SyncPulseField1 )
		| BCHP_FIELD_DATA(HDMI_VERTA1, MANUAL_VFP1, BHDM_VideoFmtParams[index].V_FrontPorchField1 )
		| BCHP_FIELD_DATA(HDMI_VERTA1, MANUAL_VAL1, BHDM_VideoFmtParams[index].V_ActiveLinesField1 ) ;
	BREG_Write32(hRegister, BCHP_HDMI_VERTA1 + ulOffset, Register) ;


	/* Load VERTB1 Register */
	Register =
		(uint32_t) 0
		| BCHP_FIELD_DATA(HDMI_VERTB1, MANUAL_VSPO1, BHDM_VideoFmtParams[index].V_SyncPulseOffsetField1 )
		| BCHP_FIELD_DATA(HDMI_VERTB1, MANUAL_VBP1, BHDM_VideoFmtParams[index].V_BackPorchField1 ) ;
	BREG_Write32(hRegister, BCHP_HDMI_VERTB1 + ulOffset, Register) ;

	/* update pixel clock rate */
	hHDMI->DeviceStatus.pixelClockRate = BHDM_P_TmdsClockToValue_isrsafe(hHDMI->eTmdsClock);

done:
	BDBG_LEAVE(BHDM_P_ConfigureInputVideoFmt) ;
	return rc ;
}


/***************************************************************************
BERR_Code BHDM_GetEventHandle
Summary: Get the event handle for checking HDMI events.
****************************************************************************/
BERR_Code BHDM_GetEventHandle(
   const BHDM_Handle hHDMI,           /* [in] HDMI handle */
   BHDM_EventType eEventType,
   BKNI_EventHandle *pBHDMEvent	/* [out] event handle */
)
{
	BERR_Code      rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_GetEventHandle) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	switch (eEventType)
	{
	case BHDM_EventHDCP :
		*pBHDMEvent = hHDMI->BHDM_EventHDCP ;
		break ;

	case BHDM_EventHotPlug :
		*pBHDMEvent = hHDMI->BHDM_EventHotPlug ;
		break ;

	case BHDM_EventFIFO : /* debugging event */
		*pBHDMEvent = hHDMI->BHDM_EventFIFO ;
		break ;

	case BHDM_EventRAM :  /* debugging event */
		*pBHDMEvent = hHDMI->BHDM_EventRAM ;
		break ;

	case BHDM_EventHDCPRiValue :
		*pBHDMEvent = hHDMI->BHDM_EventHDCPRiValue ;
		break ;

	case BHDM_EventHDCPPjValue :
		*pBHDMEvent = hHDMI->BHDM_EventHDCPPjValue ;
		break ;

	case BHDM_EventHDCPRepeater:
		*pBHDMEvent = hHDMI->BHDM_EventHDCPRepeater;
		break;

	case BHDM_EventRxSense:
		*pBHDMEvent = hHDMI->BHDM_EventRxSense;
		break;

	case BHDM_EventScramble :
		*pBHDMEvent = hHDMI->BHDM_EventScramble ;
		break;

	case BHDM_EventAvRateChange :
		*pBHDMEvent = hHDMI->BHDM_EventAvRateChange ;
		break ;

	case BHDM_EventHDCP22EncryptionEnable:
#if BHDM_CONFIG_HAS_HDCP22
		*pBHDMEvent = hHDMI->BHDM_EventHdcp22EncEnUpdate;
#else
		*pBHDMEvent = NULL;
#endif
		break;

	case BHDM_EventHDCP22ReAuthRequest:
#if BHDM_CONFIG_HAS_HDCP22
		*pBHDMEvent = hHDMI->BHDM_EventHdcp22ReAuthRequest;
#else
		*pBHDMEvent = NULL;
#endif
		break;

	default :
		BDBG_ERR(("BHDM_GetEventHandle: Unknown Event Handle: %d", eEventType)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}


done:
	BDBG_LEAVE(BHDM_GetEventHandle) ;
	return rc ;
}


/******************************************************************************
void BHDM_P_Hotplug_isr
Summary: Process Hot Plug Interrupt
*******************************************************************************/

#if BHDM_CONFIG_DUAL_HPD_SUPPORT

/******************************/
/*   DUAL HOT PLUG INTERRUPTS */
/*      HOTPLUG_REMOVED or    */
/*      HOTPLUG_CONNECTED     */
/*****************************/

void BHDM_P_Hotplug_isr(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	uint8_t RxDeviceAttached ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	BHDM_P_RxDeviceAttached_isr(hHDMI, &RxDeviceAttached) ;
	hHDMI->RxDeviceAttached = RxDeviceAttached;

	if (!RxDeviceAttached)
	{
		BDBG_LOG(("Tx%d: HotPlug (Dual Intr) : DEVICE REMOVED!!", hHDMI->eCoreId)) ;

#if BHDM_CONFIG_HAS_HDCP22
		/* hard reset HDCP_I2C/HDCP SW_INIT first */
		BHDM_P_ResetHDCPI2C_isr(hHDMI);
#endif

		BHDM_P_DisableDisplay_isr(hHDMI) ;

		hHDMI->RxDeviceAttached = 0;
		hHDMI->hotplugInterruptFired = true;

#if BHDM_HAS_HDMI_20_SUPPORT
		BKNI_Memset(&hHDMI->stStatusControlData, 0, sizeof(BHDM_SCDC_StatusControlData)) ;
#endif
		BHDM_MONITOR_P_HpdChanges_isr(hHDMI) ;

		/* always disable AvMute after a hot plug */
		hHDMI->AvMuteState = false ;

		/* abort any pending HDCP requests */
		BHDM_HDCP_P_ResetSettings_isr(hHDMI) ;

#if BHDM_CONFIG_HAS_HDCP22
		BHDM_AUTO_I2C_SetChannels_isr(hHDMI, false) ;
#endif


		/* Set CLEAR_RDB_AUTHENTICATED BIT only - all other bits must be zero */
		Register = BCHP_FIELD_DATA(HDMI_HDCP_CTL, I_CLEAR_RDB_AUTHENTICATED, 1) ;
		BREG_Write32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset, Register) ;

	}
	else /* HOTPLUG_CONNECTED */
	{
		BDBG_LOG(("Tx%d: HotPlug  (Dual Intr): DEVICE CONNECTED", hHDMI->eCoreId)) ;
		hHDMI->RxDeviceAttached = 1;
		hHDMI->hotplugInterruptFired = true;
		hHDMI->edidStatus = BHDM_EDID_STATE_eInitialize;	/* Set Initialize EDID read flag */

		BHDM_MONITOR_P_HpdChanges_isr(hHDMI) ;
	}

	/* make sure hot plug callback has been installed */
	if (hHDMI->pfHotplugChangeCallback)
	{
		/* Fire hotplug callback */
		hHDMI->pfHotplugChangeCallback(hHDMI->pvHotplugChangeParm1,
			hHDMI->iHotplugChangeParm2, &hHDMI->RxDeviceAttached) ;
	}

	BKNI_SetEvent_isr(hHDMI->BHDM_EventHotPlug) ;
}

#else

/****************************************/
/*     SINGLE HOT PLUG INTERRUPT        */
/*       HOTPLUG               */
/* fires for both REMOVED and CONNECTED */
/****************************************/

void BHDM_P_Hotplug_isr(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	uint8_t RxDeviceAttached ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	BHDM_P_RxDeviceAttached_isr(hHDMI, &RxDeviceAttached) ;

	/* if same state return without setting an event */
	if (RxDeviceAttached == hHDMI->RxDeviceAttached)
	{
		BDBG_MSG(("Tx%d: Skip Multiple Hotplug Interrupts: %s", hHDMI->eCoreId,
			RxDeviceAttached ? "CONNECTED" : "REMOVED")) ;
		return ;
	}
	hHDMI->RxDeviceAttached = RxDeviceAttached;

	if (!hHDMI->RxDeviceAttached)
	{
		BDBG_LOG(("Tx%d: HotPlug: DEVICE REMOVED!!", hHDMI->eCoreId)) ;
		BHDM_P_DisableDisplay_isr(hHDMI) ;

		/* always disable AvMute after a hot plug */
		hHDMI->AvMuteState = false ;
#if BHDM_HAS_HDMI_20_SUPPORT
		BKNI_Memset(&hHDMI->stStatusControlData, 0, sizeof(BHDM_SCDC_StatusControlData)) ;
#endif
	}
	else
	{
		BDBG_LOG(("Tx%d: HotPlug: DEVICE CONNECTED", hHDMI->eCoreId)) ;
	}

	hHDMI->edidStatus = BHDM_EDID_STATE_eInitialize;	/* Set Initialize EDID read flag */

	BHDM_MONITOR_P_HpdChanges_isr(hHDMI) ;

	/* abort any pending HDCP requests */
	BHDM_HDCP_P_ResetSettings_isr(hHDMI) ;

	/* Set CLEAR_RDB_AUTHENTICATED BIT only - all other bits must be zero */
	Register = BCHP_FIELD_DATA(HDMI_HDCP_CTL, I_CLEAR_RDB_AUTHENTICATED, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_HDCP_CTL + ulOffset, Register) ;

	if (hHDMI->pfHotplugChangeCallback)
	{
		hHDMI->pfHotplugChangeCallback(hHDMI->pvHotplugChangeParm1,
			hHDMI->iHotplugChangeParm2, &hHDMI->RxDeviceAttached) ;
	}

	BKNI_SetEvent_isr(hHDMI->BHDM_EventHotPlug) ;
}
#endif


#if BHDM_CONFIG_HAS_HDCP22
void BHDM_P_HandleHAEInterrupt_isr(
	void *pParam1,						/* [in] Device handle */
	int parm2							/* [in] not used */
)
{
	BHDM_Handle hHDMI ;
	BREG_Handle hRegister ;
	uint32_t  ulOffset ;

	hHDMI = (BHDM_Handle) pParam1 ;
	ulOffset = hHDMI->ulOffset ;
	hRegister = hHDMI->hRegister ;

	switch (parm2)
	{
	case MAKE_HAE_INTR_ENUM(OK_TO_ENC_EN):
		BDBG_LOG(("Tx%d: HAE_Int0x%x! - OK_TO_ENC_EN", hHDMI->eCoreId, parm2));
		{
			uint32_t Register;

			/* Update authentication status in HW */
			Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset);
				Register &= ~BCHP_MASK(HDMI_HDCP2TX_AUTH_CTL, HDCP2_AUTHENTICATED);
				Register |= BCHP_FIELD_DATA(HDMI_HDCP2TX_AUTH_CTL, HDCP2_AUTHENTICATED, 1);
			BREG_Write32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset, Register) ;
		}

		BKNI_SetEvent_isr(hHDMI->BHDM_EventHdcp22EncEnUpdate) ;
		break;

	case MAKE_HAE_INTR_ENUM(REAUTH_REQ):
		BDBG_LOG(("Tx%d: HAE_Int0x%x! - REAUTH_REQ (from Rx)", hHDMI->eCoreId, parm2));
		{
			uint32_t Register;

			/* disable hdcp2.2 encryption */
			Register = BREG_Read32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset);
				Register &= ~BCHP_MASK(HDMI_HDCP2TX_AUTH_CTL, ENABLE_HDCP2_ENCRYPTION);
				Register |= BCHP_FIELD_DATA(HDMI_HDCP2TX_AUTH_CTL, ENABLE_HDCP2_ENCRYPTION, 0);
			BREG_Write32(hRegister, BCHP_HDMI_HDCP2TX_AUTH_CTL + ulOffset, Register) ;
		}

		hHDMI->bReAuthRequestPending = true;
		BKNI_SetEvent_isr(hHDMI->BHDM_EventHdcp22ReAuthRequest) ;
		break;

	default :
		BDBG_WRN(("Tx%d: BHDM_HAE Unknown Interrupt ID=0x%x !",
			hHDMI->eCoreId, parm2 ));
	}
}
#endif /* #if BHDM_CONFIG_HAS_HDCP22 */


/******************************************************************************
void BHDM_P_HandleInterrupt_isr
Summary: Handle interrupts from the HDMI core.
*******************************************************************************/
void BHDM_P_HandleInterrupt_isr(
	void *pParam1,						/* [in] Device handle */
	int parm2							/* [in] not used */
)
{
	BHDM_Handle hHDMI ;

	/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
	uint8_t MASK_INTERRUPTS = 1 ;  /* debug tool */
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */

	BERR_Code rc ;

	hHDMI = (BHDM_Handle) pParam1 ;

	/*
	-- Interrupts to be handled
	18 RSEN_UPDATE_INTR
	17 HDCP_RI_B_MISMATCH_INTR
	16 HDCP_RI_A_MISMATCH_INTR
	15 HDCP_PJ_MISMATCH_INTR

	14 MAI_FORMAT_UPDATE_INTR
	13 HDCP_PJ_INTR
	11 HDCP_AN_READY_INTR
	10 HDCP_RI_INTR
	09 HDCP_V_MATCH_INTR
	08 HDCP_V_MISMATCH_INTR
	07 HDCP_REPEATER_ERR_INTR

	06 CEC_INTR  ---> handled by CEC PI

	05 PKT_WRITE_ERR_INTR
	12 PACKET_FIFO_OVERFLOW_INTR

	04 DRIFT_FIFO_ALMOST_EMPTY_INTR
	03 DRIFT_FIFO_EMPTY_MINUS_INTR
	02 DRIFT_FIFO_ALMOST_FULL_INTR
	01 DRIFT_FIFO_FULL_MINUS_INTR

	00 HOTPLUG_INTR
	*/

	switch (parm2)
	{

#if BHDM_CONFIG_DUAL_HPD_SUPPORT
	/*****************************/
	/*   DUAL Hotplug Interrupts */
	/*****************************/
	case MAKE_INTR_ENUM(HOTPLUG_REMOVED) :		/* 00 */
	case MAKE_INTR_ENUM(HOTPLUG_CONNECTED) :		/* 01 */
#else
	/*****************************/
	/* SINGLE Hotplug Interrupt  */
	/*****************************/
	case MAKE_INTR_ENUM(HOTPLUG) :					/* 00 */
#endif

#if BHDM_CONFIG_BTMR_SUPPORT
		if (BHDM_CONFIG_HOTPLUG_DELAY_MS)
		{
			/* wait before processing hot plug */
			BDBG_WRN(("Tx%d: Delay %d ms before Hotplug Callback",
				hHDMI->eCoreId, BHDM_CONFIG_HOTPLUG_DELAY_MS)) ;

			/* stop timer if already running */
			rc = BTMR_StopTimer_isr(hHDMI->TimerHotPlug) ;
			if (rc) {rc = BERR_TRACE(rc) ; goto done ;}

			rc = BTMR_StartTimer_isr(hHDMI->TimerHotPlug,
				BHDM_P_MILLISECOND * BHDM_CONFIG_HOTPLUG_DELAY_MS) ;
			if (rc) {rc = BERR_TRACE(rc) ; goto done ;}

			break ;
		}
#endif

		BHDM_P_Hotplug_isr(hHDMI) ;
		break ;


/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER

	case MAKE_INTR_ENUM(DF_FULL_MINUS) :			   /* 01 */
		BDBG_ERR(("Tx%d: Int0x%x FULL MINUS!", hHDMI->eCoreId, parm2));
		if (MASK_INTERRUPTS)
		{
			BHDM_CHECK_RC( rc, BINT_DisableCallback_isr( hHDMI->hCallback[parm2] ) );
			BDBG_MSG(("Tx%d: Full Minus Int Masked", hHDMI->eCoreId)) ;
		}
		BKNI_SetEvent_isr(hHDMI->BHDM_EventFIFO) ;

		break ;


	case MAKE_INTR_ENUM(DF_ALMOST_FULL) : 			   /* 02 */
		BDBG_ERR(("Tx%d: Int0x%x ALMOST FULL!", hHDMI->eCoreId, parm2));
		if (MASK_INTERRUPTS)
		{
			BHDM_CHECK_RC( rc, BINT_DisableCallback_isr( hHDMI->hCallback[parm2] ) );
			BDBG_MSG(("Tx%d: Almost Full Int Masked", hHDMI->eCoreId)) ;
		}
		BKNI_SetEvent_isr(hHDMI->BHDM_EventFIFO) ;
		break ;


	case MAKE_INTR_ENUM(DF_EMPTY_MINUS) :			   /* 03 */
		BDBG_ERR(("Tx%d: Int0x%x EMPTY MINUS!", hHDMI->eCoreId, parm2));
		if (MASK_INTERRUPTS)
		{
			BHDM_CHECK_RC( rc, BINT_DisableCallback_isr( hHDMI->hCallback[parm2] ) );
			BDBG_MSG(("Tx%d: Empty Minus Int Masked", hHDMI->eCoreId)) ;
		}
		BKNI_SetEvent_isr(hHDMI->BHDM_EventFIFO) ;
		break ;


	case MAKE_INTR_ENUM(DF_ALMOST_EMPTY) : 		       /* 04 */
		BDBG_ERR(("Tx%d: Int0x%x ALMOST EMPTY!", hHDMI->eCoreId, parm2));
		if (MASK_INTERRUPTS)
		{
			BHDM_CHECK_RC( rc, BINT_DisableCallback_isr( hHDMI->hCallback[parm2] ) );
			BDBG_MSG(("Tx%d: Almost Empty Int Masked", hHDMI->eCoreId)) ;
		}
		BKNI_SetEvent_isr(hHDMI->BHDM_EventFIFO) ;
		break ;


	case MAKE_INTR_ENUM(PKT_WRITE_ERR) :               /* 05 */
	case MAKE_INTR_ENUM(PKT_OVERFLOW) :			       /* 12 */
		BDBG_ERR(("Tx%d: Int0x%x!", hHDMI->eCoreId, parm2));
		BKNI_SetEvent_isr(hHDMI->BHDM_EventRAM) ;
		break ;

	case MAKE_INTR_ENUM(HDCP_RI) :					   /* 10 */
#if 0
		BDBG_MSG(("Tx%d: Int0x%x RiValue!", hHDMI->eCoreId, parm2));
#endif
		if (hHDMI->HDCP_RiCount == 0) /* skip processing r0 value twice */
			break ;

		BKNI_SetEvent_isr(hHDMI->BHDM_EventHDCPRiValue) ;
		break ;


	case MAKE_INTR_ENUM(HDCP_PJ) :					   /* 13 */
#if 0
		BDBG_MSG(("Tx%d: Int0x%x PjValue!", hHDMI->eCoreId, parm2));
#endif

		BKNI_SetEvent_isr(hHDMI->BHDM_EventHDCPPjValue) ;
		break ;

	case MAKE_INTR_ENUM(MAI_FORMAT_UPDATE):
#if BHDM_CONFIG_DEBUG_MAI_CHANNEL_MAP
		BDBG_MSG(("Tx%d: MAI FORMAT UPDATE Interrupt (%x%x)!", hHDMI->eCoreId, parm2));
#endif
		BHDM_P_HandleMaiFormatUpdate_isr(hHDMI);
		break;

#if BHDM_CONFIG_HDCP_AUTO_RI_PJ_CHECKING_SUPPORT
	case MAKE_INTR_ENUM(HDCP_RI_A_MISMATCH):		/* 15 */
		BDBG_ERR(("Tx%d: Ri mismatch Interrupt (0x%x)!", hHDMI->eCoreId, parm2));
		hHDMI->HDCP_AutoRiMismatch_A = 1;
		BKNI_SetEvent_isr(hHDMI->BHDM_EventHDCPRiValue);
		break;

	case MAKE_INTR_ENUM(HDCP_RI_B_MISMATCH):		/* 16 */
		BDBG_ERR(("Tx%d: Ri mismatch Interrupt (0x%x)!", hHDMI->eCoreId, parm2));
		hHDMI->HDCP_AutoRiMismatch_B = 1;
		BKNI_SetEvent_isr(hHDMI->BHDM_EventHDCPRiValue);
		break;

	case MAKE_INTR_ENUM(HDCP_PJ_MISMATCH):			/* 14 */
		BDBG_ERR(("Tx%d: Pj mismatch Interrupt (0x%x)!", hHDMI->eCoreId, parm2));
		hHDMI->HDCP_AutoPjMismatch = 1;
		BKNI_SetEvent_isr(hHDMI->BHDM_EventHDCPPjValue);
		break;
#endif

	case MAKE_INTR_ENUM(HDCP_REPEATER_ERR) :		   /* 07 */
	case MAKE_INTR_ENUM(HDCP_V_MISMATCH) :			   /* 08 */
		BDBG_ERR(("Tx%d: Repeater Interrupt (0x%x)!", hHDMI->eCoreId, parm2));
		BKNI_SetEvent_isr(hHDMI->BHDM_EventHDCPRepeater);
		break;

	case MAKE_INTR_ENUM(HDCP_V_MATCH) :				   /* 09 */
		BDBG_MSG(("Tx%d: BHDM Interrupt ID=0x%x!", hHDMI->eCoreId, parm2));
		/* continue to setting event */
	case MAKE_INTR_ENUM(HDCP_AN) :					   /* 11 */
		BKNI_SetEvent_isr(hHDMI->BHDM_EventHDCP) ;
		break ;


#if BHDM_CONFIG_RECEIVER_SENSE_SUPPORT && !BHDM_CONFIG_RSEN_POLLING
	case MAKE_INTR_ENUM(RSEN) :							/* 17 */
	{
		bool RxSense ;

#if BHDM_CONFIG_DEBUG_RSEN
		BDBG_WRN(("Tx%d: RxSense Interrupt!", hHDMI->eCoreId)) ;
#endif
		/* update the RxSense value */
		BHDM_P_GetReceiverSense_isr(hHDMI, &RxSense) ;

		if (RxSense == hHDMI->rxSensePowerDetected)
		{
#if BHDM_CONFIG_DEBUG_RSEN
			BDBG_WRN(("Tx%d: No change in RxSense Status of  %s",
			hHDMI->eCoreId, RxSense ? "On" : "Off")) ;
#endif
			return ;
		}

		hHDMI->rxSensePowerDetected = RxSense ;


#if BHDM_CONFIG_DEBUG_RSEN
		BDBG_WRN(("Tx%d: RxSense: %s",
			hHDMI->eCoreId, hHDMI->rxSensePowerDetected ? "ON" : "OFF" )) ;
#endif


		if (hHDMI->pfRxSenseChangeCallback)
		{
			hHDMI->pfRxSenseChangeCallback(hHDMI->pvRxSenseChangeParm1,
				hHDMI->iRxSenseChangeParm2, &hHDMI->rxSensePowerDetected) ;
		}

		BKNI_SetEvent_isr(hHDMI->BHDM_EventRxSense);
		break;
	}
#endif
#endif /* #ifndef BHDM_FOR_BOOTUPDATER */

	default :
		BDBG_WRN(("Tx%d: BHDM Unknown Interrupt ID=0x%x !",
			hHDMI->eCoreId, parm2 ));
	}

/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
done:
	/* semicolon (blank statement) fixes compiler error
	error: label at end of compound statement */
	;
#endif  /* #ifndef BHDM_FOR_BOOTUPDATER */

	/* L2 interrupts are reset automatically */
}



/*******************************************************************************
BERR_Code BHDM_GetHdmiSettings
Summary: Get the current settings for the HDMI device.
*******************************************************************************/
BERR_Code BHDM_GetHdmiSettings(const BHDM_Handle hHDMI, /* [in] handle to HDMI device */
	BHDM_Settings *pHdmiSettings  /* [in] pointer to memory to hold the current
	                                  HDMI settings */
)
{
	BERR_Code      rc = BERR_SUCCESS;
	BDBG_ENTER(BHDM_GetHdmiSettings) ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;
	BDBG_ASSERT(pHdmiSettings) ;

	BKNI_Memset(pHdmiSettings, 0, sizeof(BHDM_Settings)) ;
	*pHdmiSettings = hHDMI->DeviceSettings ;

	BDBG_LEAVE(BHDM_GetHdmiSettings) ;
	return rc ;
}


/*******************************************************************************
BERR_Code BHDM_GetHdmiStatus
Summary: Get the current status for the HDMI device.
*******************************************************************************/
BERR_Code BHDM_GetHdmiStatus(const BHDM_Handle hHDMI, /* [in] handle to HDMI device */
	BHDM_Status *pHdmiStatus  /* [in] pointer to memory to hold the current HDMI status */
)
{
	BERR_Code      rc = BERR_SUCCESS;
	BDBG_ENTER(BHDM_GetHdmiStatus) ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;
	BDBG_ASSERT(pHdmiStatus) ;

	*pHdmiStatus = hHDMI->DeviceStatus ;

	BDBG_LEAVE(BHDM_GetHdmiStatus) ;
	return rc ;
}


/*******************************************************************************
BERR_Code BHDM_SetHdmiSettings
Summary: Save the current settings for the HDMI device.

Note: the settings for HDMI device will be saved when calling BHDM_EnableDisplay
         This call should not be use if BHDM_EnableDisplay is used

See Also:
	BHDM_EnableDisplay()
*******************************************************************************/
BERR_Code BHDM_SetHdmiSettings(const BHDM_Handle hHDMI, /* [in] handle to HDMI device */
	BHDM_Settings *pHdmiSettings  /* [in] pointer to memory to hold the current
									  HDMI settings */
)
{
	BERR_Code rc = BERR_SUCCESS;
	bool bForceEnableDisplay ;
	bool bForcePacketUpdates ;
	BDBG_ENTER(BHDM_SetHdmiSettings) ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;
	BDBG_ASSERT(pHdmiSettings) ;

	/* force BHDM_EnableDisplay to update if HdmiSettings have changed */
	bForceEnableDisplay = BHDM_P_HdmiPhyChanges(hHDMI, pHdmiSettings) ;
	bForcePacketUpdates = BHDM_P_HdmiPacketChanges(hHDMI, pHdmiSettings) ;

	BKNI_Memcpy(&(hHDMI->DeviceSettings), pHdmiSettings, sizeof(BHDM_Settings));

	/* do not overwrite possible previous bForceEnableDisplay */
	if (bForceEnableDisplay)
		hHDMI->DeviceSettings.bForceEnableDisplay = true ;

	/* do not overwrite possible previous bForceUpdatePackets */
	if (bForcePacketUpdates)
		hHDMI->bForcePacketUpdates = true ;

	BDBG_LEAVE(BHDM_SetHdmiSettings) ;
	return rc ;
}


void BHDM_ReenableHotplugInterrupt(const BHDM_Handle hHDMI)
{
	BERR_Code rc ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

#if BHDM_CONFIG_DUAL_HPD_SUPPORT
	rc = BINT_EnableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(HOTPLUG_REMOVED)] ) ;
	if (rc) {rc = BERR_TRACE(rc) ;}

	rc = BINT_EnableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(HOTPLUG_CONNECTED)] ) ;
	if (rc) {rc = BERR_TRACE(rc) ; }
#else
	rc = BINT_EnableCallback( hHDMI->hCallback[MAKE_INTR_ENUM(HOTPLUG)] ) ;
	if (rc) {rc = BERR_TRACE(rc) ; }
#endif

	BDBG_LOG(("**********")) ;
	BDBG_LOG(("Tx%d HPD INTRs RE-ENABLED", hHDMI->eCoreId)) ;
	BDBG_LOG(("Tx%d Total Hotplug Changes reset to 0", hHDMI->eCoreId)) ;
	BDBG_LOG(("**********")) ;

	/* reset Total Hot Plug Changes */
	hHDMI->MonitorStatus.TotalHotPlugChanges = 0 ;
	hHDMI->MonitorStatus.TxHotPlugInterruptDisabled = false ;
}




/* For boot loader usage */
#ifndef BHDM_FOR_BOOTUPDATER
void BHDM_P_CaptureFIFOData(const BHDM_Handle hHDMI, BHDM_P_FIFO_DATA *FifoData)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;


	/* get the CAPTURE POINTER data for debug, fine tuning, etc. */
	Register = BREG_Read32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset);
		Register &= ~BCHP_MASK(HDMI_FIFO_CTL, reserved0) ;
#if BCHP_HDMI_FIFO_CTL_reserved1_MASK
		Register &= ~BCHP_MASK(HDMI_FIFO_CTL, reserved1) ;
#endif
		Register &= ~BCHP_MASK(HDMI_FIFO_CTL, CAPTURE_POINTERS) ;

		Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, CAPTURE_POINTERS, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register);

		Register &= ~BCHP_MASK(HDMI_FIFO_CTL, CAPTURE_POINTERS) ;
	BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register);


	/* check for over/underflow */
	Register = BREG_Read32(hRegister, BCHP_HDMI_READ_POINTERS + ulOffset) ;
	FifoData->bOverFlow =
		BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_OVERFLOW) ;

	FifoData->bUnderFlow =
		BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_UNDERFLOW) ;

	FifoData->uiRdAddr =
		(uint8_t) BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_RD_ADDR_7_0) ;

	FifoData->uiWrAddr =
		(uint8_t) BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_WR_ADDR_7_0) ;

	FifoData->uiFullness =
		(uint16_t) ((FifoData->uiWrAddr - FifoData->uiRdAddr + 256) % 256) ;

#if BHDM_CONFIG_DEBUG_FIFO
	/* read the pointers */
	{
		uint8_t RdPtr, WrPtr ;

		bool
			bOverflow, bUnderflow,
			bAlmostFull, bFullMinus,
			bAlmostEmpty, bEmptyMinus ;

		BDBG_WRN(("Tx%d: ----------------- Begin Drift FIFO Debug Dump -----------------",
			hHDMI->eCoreId)) ;

		Register = BREG_Read32(hRegister, BCHP_HDMI_READ_POINTERS + ulOffset);

			RdPtr = BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_RD_ADDR_7_0) ;
			WrPtr = BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_WR_ADDR_7_0) ;

			bAlmostFull =
				BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_ALMOST_FULL) ;
			bFullMinus=
				BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_FULL_MINUS) ;
			bOverflow =
				BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_OVERFLOW) ;
			bAlmostEmpty =
				BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_ALMOST_EMPTY) ;
			bEmptyMinus =
				BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_EMPTY_MINUS) ;
			bUnderflow =
				BCHP_GET_FIELD_DATA(Register, HDMI_READ_POINTERS, DRIFT_UNDERFLOW) ;


		BDBG_WRN(("Tx%d: Read/Write Pointer: %d / %d    FULLNESS: %d", hHDMI->eCoreId,
			 RdPtr, WrPtr, FifoData->uiFullness)) ;

		if ( bUnderflow && bOverflow)
		{
			BDBG_WRN(("Tx%d: UNDERFLOW / OVERFLOW: %d / %d", hHDMI->eCoreId,
				bUnderflow, bOverflow)) ;
		}

		if (bAlmostFull && bFullMinus)
		{
			BDBG_WRN(("Tx%d: ALMOST_FULL: %d  FULL_MINUS: %d", hHDMI->eCoreId,
				bAlmostFull, bFullMinus)) ;
		}

		if (bAlmostEmpty && bEmptyMinus)
		{
			BDBG_WRN(("Tx%d: ALMOST_EMPTY: %d  EMPTY_MINUS %d",  hHDMI->eCoreId,
				bAlmostEmpty, bEmptyMinus)) ;
		}

		BDBG_WRN(("Tx%d: -----------------  End Drift FIFO Debug Dump ------------------", hHDMI->eCoreId)) ;
	}
#endif
}


#if !B_REFSW_MINIMAL
BERR_Code BHDM_SetTimebase(
   const BHDM_Handle hHDMI,          /* [in] HDMI handle */
   BAVC_Timebase eTimebase     /* [in] Timebase */
)
{
	BERR_Code      rc = BERR_SUCCESS;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hHDMI->DeviceSettings.eTimebase = eTimebase ;
	return rc ;
}


BERR_Code BHDM_GetTimebase(
   const BHDM_Handle hHDMI,          /* [in] HDMI handle */
   BAVC_Timebase *eTimebase     /* [out] Timebase */
)
{
	BERR_Code      rc = BERR_SUCCESS;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	*eTimebase = hHDMI->DeviceSettings.eTimebase  ;
	return rc ;
}
#endif


void BHDM_P_VideoFmt2CEA861Code(BFMT_VideoFmt eVideoFmt,
	BFMT_AspectRatio eAspectRatio, BAVC_HDMI_PixelRepetition ePixelRepetition, uint8_t *VideoID)
{
	switch (eVideoFmt)
	{
	case BFMT_VideoFmt_e1080i  :           /* HD 1080i */
		*VideoID = 5 ;
		break ;

	case BFMT_VideoFmt_e720p   :           /* HD 720p */
	case BFMT_VideoFmt_e720p_60Hz_3DOU_AS:	/* 720p 60Hz 3D frame packing */
#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
	case BFMT_VideoFmt_e720p_3D :          /* HD 720p 3D */
#endif
		*VideoID = 4 ;
		break ;

	case BFMT_VideoFmt_e480p   :           /* HD 480p */
		if (eAspectRatio == BFMT_AspectRatio_e16_9)
		{
			switch (ePixelRepetition)
			{
			case BAVC_HDMI_PixelRepetition_eNone:
				*VideoID = 3 ;
				break;
			case BAVC_HDMI_PixelRepetition_e1x:
				*VideoID = 15 ;
				break;
			case BAVC_HDMI_PixelRepetition_e4x:
				*VideoID = 36 ;
				break;
			default:
				break;
			}
		}
		else   /* default 4:3 */
		{
			switch (ePixelRepetition)
			{
			case BAVC_HDMI_PixelRepetition_eNone:
				*VideoID = 2 ;
				break;
			case BAVC_HDMI_PixelRepetition_e1x:
				*VideoID = 14 ;
				break;
			case BAVC_HDMI_PixelRepetition_e4x:
				*VideoID = 35 ;
				break;
			default:
				break;
			}
		}
		break ;

	case BFMT_VideoFmt_eNTSC   :		   /* 480i, NSTC-M for North America */
	case BFMT_VideoFmt_eNTSC_J :		   /* 480i (Japan) */
	case BFMT_VideoFmt_ePAL_M  :		   /* 525-lines (Brazil) */
		if (eAspectRatio == BFMT_AspectRatio_e16_9)
		{
			switch (ePixelRepetition)
			{
			case BAVC_HDMI_PixelRepetition_eNone:
				*VideoID = 7 ;
				break;
			case BAVC_HDMI_PixelRepetition_e1x:
				*VideoID = 11 ;
				break;
			default:
				break;
			}
		}
		else   /* default 4:3 */
		{
			switch (ePixelRepetition)
			{
			case BAVC_HDMI_PixelRepetition_eNone:
				*VideoID = 6 ;
				break;
			case BAVC_HDMI_PixelRepetition_e1x:
				*VideoID = 10 ;
				break;
			default:
				break;
			}
		}
		break ;

	case BFMT_VideoFmt_ePAL_B  :		   /* Australia */
	case BFMT_VideoFmt_ePAL_B1 :		   /* Hungary */
	case BFMT_VideoFmt_ePAL_D  :		   /* China */
	case BFMT_VideoFmt_ePAL_D1 :		   /* Poland */
	case BFMT_VideoFmt_ePAL_G  :		   /* Europe */
	case BFMT_VideoFmt_ePAL_H  :		   /* Europe */
	case BFMT_VideoFmt_ePAL_K  :		   /* Europe */
	case BFMT_VideoFmt_ePAL_I  :		   /* U.K. */
	case BFMT_VideoFmt_ePAL_N  :		   /* Jamaica, Uruguay */
	case BFMT_VideoFmt_ePAL_NC :		   /* N combination (Argentina) */
	case BFMT_VideoFmt_eSECAM  :		   /* LDK/SECAM (France,Russia) */
		if (eAspectRatio == BFMT_AspectRatio_e16_9)
		{
			switch (ePixelRepetition)
			{
			case BAVC_HDMI_PixelRepetition_eNone:
				*VideoID = 22 ;
				break;
			case BAVC_HDMI_PixelRepetition_e1x:
				*VideoID = 26 ;
				break;
			default:
				break;
			}
		}
		else   /* default 4:3 */
		{
			switch (ePixelRepetition)
			{
			case BAVC_HDMI_PixelRepetition_eNone:
				*VideoID = 21 ;
				break;
			case BAVC_HDMI_PixelRepetition_e1x:
				*VideoID = 25 ;
				break;
			default:
				break;
			}
		}
		break ;

	case BFMT_VideoFmt_e1250i_50Hz :	   /* HD 1250i 50Hz, another 1080i_50hz standard SMPTE 295M */
		BDBG_WRN(("Verify AVI Frame Video Code for 1250i Format")) ;
		/* fall through to use 1080i 50Hz Video Code */
	case BFMT_VideoFmt_e1080i_50Hz :	   /* HD 1080i 50Hz, 1125 line, SMPTE 274M */
		*VideoID = 20 ;
		break ;

	case BFMT_VideoFmt_e720p_50Hz  :	   /* HD 720p 50Hz (Australia) */
	case BFMT_VideoFmt_e720p_50Hz_3DOU_AS: /* 720p 50Hz 3D Frame Packing */
#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
	case BFMT_VideoFmt_e720p_50Hz_3D :	   /* HD 720p 50Hz 3D */
#endif
		*VideoID  = 19 ;
		break ;

	case BFMT_VideoFmt_e720p_24Hz:			/* 720p 24Hz */
	case BFMT_VideoFmt_e720p_24Hz_3DOU_AS:	/* 720p 24Hz 3D frame packing */
		*VideoID  = 60 ;
		break ;

	case BFMT_VideoFmt_e720p_25Hz:			/* 720p 25Hz */
		*VideoID  = 61 ;
		break ;

	case BFMT_VideoFmt_e720p_30Hz:			/* 720p 30Hz */
	case BFMT_VideoFmt_e720p_30Hz_3DOU_AS:	/* 720p 30Hz 3D frame packing */
		*VideoID  = 62 ;
		break ;

	case BFMT_VideoFmt_e576p_50Hz  :	   /* HD 576p 50Hz (Australia) */
		if (eAspectRatio  == BFMT_AspectRatio_e16_9)
		{
			switch (ePixelRepetition)
			{
			case BAVC_HDMI_PixelRepetition_eNone:
				*VideoID = 18 ;
				break;
			case BAVC_HDMI_PixelRepetition_e1x:
				*VideoID = 30 ;
				break;
			case BAVC_HDMI_PixelRepetition_e4x:
				*VideoID = 38 ;
				break;
			default:
				break;
			}
		}
		else   /* default 4:3 */
		{
			switch (ePixelRepetition)
			{
			case BAVC_HDMI_PixelRepetition_eNone:
				*VideoID = 17 ;
				break;
			case BAVC_HDMI_PixelRepetition_e1x:
				*VideoID = 29 ;
				break;
			case BAVC_HDMI_PixelRepetition_e4x:
				*VideoID = 37 ;
				break;
			default:
				break;
			}
		}
		break ;

	case BFMT_VideoFmt_eDVI_640x480p :	   /* DVI Safe mode for computer monitors */
		*VideoID = 1 ;
		break ;

	case BFMT_VideoFmt_e1080p	:	  /* HD 1080p 60Hz */
	case BFMT_VideoFmt_e1080p_60Hz_3DOU_AS:
	case BFMT_VideoFmt_e1080p_60Hz_3DLR:
		*VideoID = 16 ;
		break ;

	case BFMT_VideoFmt_e1080p_50Hz	:	  /* HD 1080p 50Hz */
		*VideoID = 31 ;
		break ;

	case BFMT_VideoFmt_e1080p_30Hz	:	  /* HD 1080p 30Hz */
	case BFMT_VideoFmt_e1080p_30Hz_3DOU_AS:
		*VideoID = 34 ;
		break ;

	case BFMT_VideoFmt_e1080p_24Hz	:	  /* HD 1080p 24Hz */
	case BFMT_VideoFmt_e1080p_24Hz_3DOU_AS:
#ifdef BHDM_CONFIG_BLURAY_3D_SUPPORT
	case BFMT_VideoFmt_e1080p_24Hz_3D :	  /* HD 1080p 24Hz 3D */
#endif
		*VideoID = 32 ;
		break ;

	case BFMT_VideoFmt_e1080p_25Hz	:	  /* HD 1080p 25Hz */
		*VideoID = 33 ;
		break ;

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
	case BFMT_VideoFmt_e3840x2160p_30Hz :
	case BFMT_VideoFmt_e3840x2160p_25Hz :
	case BFMT_VideoFmt_e3840x2160p_24Hz :
	case BFMT_VideoFmt_e4096x2160p_24Hz :
		BDBG_MSG(("Video ID Code of 0 used for for HDMI Extended Resolution formats"));
		*VideoID = 0 ;
		break ;

	case BFMT_VideoFmt_e1080p_100Hz :
		BDBG_MSG(("Video ID Code of 64 used for for 1080p100"));
		*VideoID = 64 ;
		break ;
	case BFMT_VideoFmt_e1080p_120Hz :
		BDBG_MSG(("Video ID Code of 63 used for for 1080p120"));
		*VideoID = 63 ;
		break ;
#endif

#if BHDM_CONFIG_4Kx2K_60HZ_SUPPORT
	case BFMT_VideoFmt_e3840x2160p_50Hz :
		*VideoID = 96 ;
		break ;
	case BFMT_VideoFmt_e3840x2160p_60Hz :
		*VideoID = 97 ;
		break ;
#endif

	case BFMT_VideoFmt_eDVI_800x600p:
	case BFMT_VideoFmt_eDVI_1024x768p:
	case BFMT_VideoFmt_eDVI_1280x768p:
	case BFMT_VideoFmt_eDVI_1280x720p_50Hz:
	case BFMT_VideoFmt_eDVI_1280x720p:
	case BFMT_VideoFmt_eDVI_1280x1024p_60Hz :
		*VideoID = 0;
		BDBG_MSG(("Video ID Code of 0 used for DVI formats"));
		break;

	default :
		*VideoID = 0 ;
		BDBG_ERR(("BFMT_VideoFmt %d NOT IMPLEMENTED",  eVideoFmt)) ;
		break ;
	}

	BDBG_MSG(("BFMT_eVideoFmt %d ==> CEA 861 Video ID Code: %d",
		eVideoFmt, *VideoID)) ;

}



#if !B_REFSW_MINIMAL
/******************************************************************************
Summary:
Set the pixel repetition setting
*******************************************************************************/
BERR_Code BHDM_SetPixelRepetition(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BAVC_HDMI_PixelRepetition ePixelRepetition
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_SetPixelRepetition) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	switch (ePixelRepetition)
	{
	case BAVC_HDMI_PixelRepetition_eNone:
	case BAVC_HDMI_PixelRepetition_e1x:
	case BAVC_HDMI_PixelRepetition_e3x:
	case BAVC_HDMI_PixelRepetition_e4x:
	case BAVC_HDMI_PixelRepetition_e5x:
	case BAVC_HDMI_PixelRepetition_e6x:
	case BAVC_HDMI_PixelRepetition_e7x:
	case BAVC_HDMI_PixelRepetition_e8x:
	case BAVC_HDMI_PixelRepetition_e9x:
	case BAVC_HDMI_PixelRepetition_e10x:
		hHDMI->DeviceSettings.ePixelRepetition = ePixelRepetition;
		break;

	default:
		BDBG_ERR(("Invalid Pixel Repetition %d", ePixelRepetition));
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		break;
	}

	BDBG_LEAVE(BHDM_SetPixelRepetition) ;
	return rc;

}


/******************************************************************************
Summary:
Get the pixel repetition setting
*******************************************************************************/
BERR_Code BHDM_GetPixelRepetition(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BAVC_HDMI_PixelRepetition *ePixelRepetition
)
{
	BDBG_ENTER(BHDM_GetPixelRepetition) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (ePixelRepetition)
	{
		*ePixelRepetition = hHDMI->DeviceSettings.ePixelRepetition;
	}

	BDBG_LEAVE(BHDM_GetPixelRepetition) ;
	return BERR_SUCCESS;
}
#endif


/******************************************************************************
Summary: install Hot Plug Change Callback to notify of HP detect changes
*******************************************************************************/
BERR_Code BHDM_InstallHotplugChangeCallback(
	const BHDM_Handle hHDMI,			/* [in] HDMI Handle */
	const BHDM_CallbackFunc pfCallback_isr, /* [in] cb for hot plug changes */
	void *pvParm1, /* [in] the first argument (void *) passed to the callback function */
	int iParm2)    /* [in] the second argument(int) passed to the callback function */
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_InstallHotplugChangeCallback) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* Check if this is a valid function */
	if( pfCallback_isr == NULL )
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER);
		return rc;
	}

	BKNI_EnterCriticalSection() ;
		hHDMI->pfHotplugChangeCallback = pfCallback_isr ;
		hHDMI->pvHotplugChangeParm1 = pvParm1 ;
		hHDMI->iHotplugChangeParm2 = iParm2 ;

	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDM_InstallHotplugChangeCallback);

	return rc ;
}


#if !B_REFSW_MINIMAL
/******************************************************************************
Summary: Uninstall HotPlug Change Callback
*******************************************************************************/
BERR_Code BHDM_UnInstallHotplugChangeCallback(
	const BHDM_Handle hHDMI,						 /* [in] HDMI Handle */
	const BHDM_CallbackFunc pfCallback_isr) /* [in] cb for format changes */
{
	BERR_Code rc = BERR_SUCCESS ;
	BSTD_UNUSED(pfCallback_isr) ;

	BDBG_ENTER(BHDM_UnInstallHotplugChangeCallback) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_EnterCriticalSection() ;
		hHDMI->pfHotplugChangeCallback = (BHDM_CallbackFunc) NULL ;

	BKNI_LeaveCriticalSection();

	BDBG_LEAVE(BHDM_UnInstallHotplugChangeCallback) ;
	return rc;
}
#endif

BERR_Code BHDM_GetCrcValue_isr(
	const BHDM_Handle hHDMI,	/* [in] HDMI Handle */
	BHDM_CrcData *stCrcData
)
{
#if BHDM_CONFIG_CRC_SUPPORT
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;
	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, REGADDR_CRC_CHECK_STATUS_0 + ulOffset) ;
	stCrcData->crc = BCHP_GET_FIELD_DATA(Register, REGNAME_CRC_CHECK_STATUS_0, CRC_VALUE);

#else
	BSTD_UNUSED(hHDMI);
	BSTD_UNUSED(stCrcData);
#endif

	return BERR_SUCCESS;
}

static void BHDM_P_HandleMaiFormatUpdate_isr(const BHDM_Handle hHDMI)
{
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	BAVC_AudioSamplingRate eSampleRate;
	uint8_t
		MaiChannel0, MaiChannel1,
		MaiChannel2, MaiChannel3,
		MaiChannel4, MaiChannel5,
		MaiChannel6, MaiChannel7 ;

	/**** Default Set Channel Map 	*/
	/* set CHANNEL_0_MAP = 0  - reset value */
	/* set CHANNEL_1_MAP = 1  - reset value */
	/* set CHANNEL_2_MAP = 2  - reset value */
	/* set CHANNEL_3_MAP = 3  - reset value */
	/* set CHANNEL_4_MAP = 4  - reset value */
	/* set CHANNEL_5_MAP = 5  - reset value */
	/* set CHANNEL_6_MAP = 6  - reset value */
	/* set CHANNEL_7_MAP = 7  - reset value */

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32_isr(hRegister, BCHP_HDMI_MAI_FORMAT + ulOffset) ;
	switch ((Register >> 16) & 0xFF)
	{
	case 0xc8:
		/* High Bit Rate (HBR) Audio */
		MaiChannel0 = 0 ;
		MaiChannel1 = 1 ;
		MaiChannel2 = 2 ;
		MaiChannel3 = 3 ;
		MaiChannel4 = 4 ;
		MaiChannel5 = 5 ;
		MaiChannel6 = 6 ;
		MaiChannel7 = 7 ;
		break ;

	default:
		/* N Channel PCM or 2ch Compressed
		   requires a swap to convert from FMM ordering to HDMI ordering */

		MaiChannel0 = 0 ;
		MaiChannel1 = 1 ;
		MaiChannel2 = 4 ;
		MaiChannel3 = 5 ;
		MaiChannel4 = 3 ;
		MaiChannel5 = 2 ;
		MaiChannel6 = 6 ;
		MaiChannel7 = 7 ;
		break;
	}


	Register = 0 ;
		Register |=
		  BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_0_MAP, MaiChannel0)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_1_MAP, MaiChannel1)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_2_MAP, MaiChannel2)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_3_MAP, MaiChannel3)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_4_MAP, MaiChannel4)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_5_MAP, MaiChannel5)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_6_MAP, MaiChannel6)
		| BCHP_FIELD_DATA(HDMI_MAI_CHANNEL_MAP, CHANNEL_7_MAP, MaiChannel7) ;

	BREG_Write32_isr(hRegister, BCHP_HDMI_MAI_CHANNEL_MAP + ulOffset, Register) ;

#if BHDM_CONFIG_DEBUG_MAI_CHANNEL_MAP
	BDBG_WRN(("Tx%d: MAI Channel  0   1   2   3   4   5   6   7", hHDMI->eCoreId)) ;
	BDBG_WRN(("Tx%d:  Mapping   %3d %3d %3d %3d %3d %3d %3d %3d",
		hHDMI->eCoreId,
		MaiChannel0, 	MaiChannel1, MaiChannel2, MaiChannel3,
		MaiChannel4, MaiChannel5, MaiChannel6, MaiChannel7)) ;
	BDBG_WRN(("Tx%d: Updating MAI Channel Map to %#x", hHDMI->eCoreId, Register));
#endif

	/* Parse Sample Rate from MAI_FORMAT */
	Register = BREG_Read32_isr(hRegister, BCHP_HDMI_MAI_FORMAT + ulOffset) ;
	switch ( (Register >> 8) & 0xFF )
	{
	default:
	case 0x00:
		/* Not Indicated or Invalid */
		eSampleRate = BAVC_AudioSamplingRate_eUnknown;
		break;

	case 0x07:
		eSampleRate = BAVC_AudioSamplingRate_e32k;
		break;

	case 0x08:
		eSampleRate = BAVC_AudioSamplingRate_e44_1k;
		break;

	case 0x09:
		eSampleRate = BAVC_AudioSamplingRate_e48k;
		break;

	case 0xC:
		eSampleRate = BAVC_AudioSamplingRate_e96k;
		break;

	case 0xD:
		eSampleRate = BAVC_AudioSamplingRate_e128k;
		break;

	case 0xE:
		eSampleRate = BAVC_AudioSamplingRate_e176_4k;
		break;

	case 0xF:
		eSampleRate = BAVC_AudioSamplingRate_e192k;
		break;
	}

	/* Update Audio Rate */
	if ( eSampleRate != BAVC_AudioSamplingRate_eUnknown )
	{
		BAVC_Audio_Info AudioInfo;

		BDBG_MSG(("Tx%d: MAI Bus Interrupt;  force AVRateChange callback", hHDMI->eCoreId)) ;
		AudioInfo.eAudioSamplingRate = eSampleRate;
		BHDM_AudioVideoRateChangeCB_isr(hHDMI, BHDM_Callback_Type_eManualAudioChange, &AudioInfo);
	}
}


#if BHDM_CONFIG_BTMR_SUPPORT
BERR_Code BHDM_P_CreateTimer(const BHDM_Handle hHDMI, BTMR_TimerHandle * timerHandle, uint8_t timerId)
{
	BERR_Code rc ;
	BTMR_Settings timerSettings  ;

	BDBG_ENTER(BHDM_P_CreateTimer) ;

	BDBG_MSG(("Tx%d: Create BHDM_P_TIMER_eNNN ID %d", hHDMI->eCoreId, timerId)) ;

	/* create OTP Calculation Check expiration timer */
	BTMR_GetDefaultTimerSettings(&timerSettings) ;
		timerSettings.type =  BTMR_Type_eCountDown ;
		timerSettings.cb_isr = (BTMR_CallbackFunc) BHDM_P_TimerExpiration_isr ;
		timerSettings.pParm1 = hHDMI ;
		timerSettings.parm2 = timerId ;
		timerSettings.exclusive = false ;
	rc = BTMR_CreateTimer(hHDMI->hTMR, timerHandle, &timerSettings) ;
	if (rc) {rc = BERR_TRACE(rc) ; }

	BDBG_LEAVE(BHDM_P_CreateTimer) ;
	return rc ;
}


BERR_Code BHDM_P_DestroyTimer(const BHDM_Handle hHDMI, BTMR_TimerHandle timerHandle, uint8_t timerId)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_P_DestroyTimer) ;

#if BDBG_DEBUG_BUILD
	BDBG_MSG(("Tx%d: Destroy BHDM_P_TIMER_eNNN ID %d", hHDMI->eCoreId, timerId)) ;
#else
	BSTD_UNUSED(hHDMI) ;
	BSTD_UNUSED(timerId) ;
#endif


	rc = BTMR_DestroyTimer(timerHandle) ;
	if (rc) {rc = BERR_TRACE(rc) ; }

	BDBG_LEAVE(BHDM_P_DestroyTimer) ;
	return rc ;
}


static void BHDM_P_TimerExpiration_isr (const BHDM_Handle hHDMI, int parm2)
{
	switch (parm2)
	{
	case BHDM_P_TIMER_eHotPlug :
		BHDM_P_Hotplug_isr(hHDMI) ;

		break ;

	case BHDM_P_TIMER_eHotPlugChange:
		BHDM_MONITOR_P_ResetHpdChanges_isr(hHDMI) ;
		break ;

	case BHDM_P_TIMER_eFormatDetection :
		BHDM_MONITOR_P_FormatChanges_isr(hHDMI) ;
		break ;

	case BHDM_P_TIMER_eMonitorStatus :
		BHDM_MONITOR_P_StatusChanges_isr(hHDMI) ;
		break ;

	case BHDM_P_TIMER_eScdcStatus :
		/* SetEvent to read scrambling status */
		BKNI_SetEvent_isr(hHDMI->BHDM_EventScramble) ;
		break ;

	case BHDM_P_TIMER_eTxScramble :
		{
#if BHDM_HAS_HDMI_20_SUPPORT
			BHDM_ScrambleConfig ScrambleSettings ;

			/* TMDS Bit Clock Ratio change required TMDS lines to be OFF */
			/* now turn the TMDS lines back on */
			BHDM_P_EnableTmdsClock_isr(hHDMI, true) ;
			BHDM_P_EnableTmdsData_isr(hHDMI, true) ;

			BHDM_SCDC_P_GetScrambleParams_isrsafe(hHDMI, &ScrambleSettings) ;
			BHDM_SCDC_P_ConfigureScramblingTx_isr(hHDMI, &ScrambleSettings) ;

			hHDMI->TmdsBitClockRatioChange = false ;

			/* Wait 1s before reading Scramble Status; see BHDM_P_TimerExpiration_isr */
			BTMR_StartTimer_isr(hHDMI->TimerScdcStatus,
				BHDM_P_MILLISECOND * 1000) ;

#endif
			hHDMI->TmdsDisabledForBitClockRatioChange = false ;
		}
		break ;

	case BHDM_P_TIMER_ePacketChangeDelay :
		{
			BHDM_Packet PacketId ;
			uint32_t PacketMask ;

			for (PacketId = 0 ; PacketId < BHDM_Packet_eMax; PacketId++)
			{
				PacketMask = 1 << (uint32_t) PacketId ;
				if (PacketMask & hHDMI->uiPacketRestartMask)
				{
					BDBG_MSG(("Enable Packet: %d", PacketId)) ;
					BHDM_P_EnablePacketTransmission_isr(hHDMI, PacketId) ;

					/* clear the Restart Mask for this packet */
					BDBG_MSG(("Before RestartMask: %x", hHDMI->uiPacketRestartMask)) ;
					hHDMI->uiPacketRestartMask &= ~PacketMask ;
					BDBG_MSG(("After RestartMask: %x", hHDMI->uiPacketRestartMask)) ;
				}
			}
		}

		break ;


	default :
		BDBG_ERR(("Tx%d: hHDM %p Timer %d not handled", hHDMI->eCoreId, (void *)hHDMI, parm2)) ;
	}
}


void BHDM_P_AllocateTimers(const BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS ;

	/* create a hot plug settle time counter */
	BHDM_CHECK_RC(rc, BHDM_P_CreateTimer(hHDMI,
		&hHDMI->TimerHotPlug, BHDM_P_TIMER_eHotPlug)) ;

	BHDM_MONITOR_P_CreateTimers(hHDMI) ;

#if BHDM_CONFIG_HAS_HDCP22
	/* create a Bit Clock Ratio change countdown timer */
	BHDM_CHECK_RC(rc, BHDM_P_CreateTimer(hHDMI,
		&hHDMI->TimerScdcStatus, BHDM_P_TIMER_eScdcStatus)) ;

	/* create a timer to delay before enabling scrambling */
	BHDM_CHECK_RC(rc, BHDM_P_CreateTimer(hHDMI,
		&hHDMI->TimerTxScramble, BHDM_P_TIMER_eTxScramble)) ;
#endif

	/* create a packet delay timer  */
	BHDM_CHECK_RC(rc, BHDM_P_CreateTimer(hHDMI,
		&hHDMI->TimerPacketChangeDelay, BHDM_P_TIMER_ePacketChangeDelay)) ;


done:
	(void) BERR_TRACE(rc) ;
	return  ;
}


void BHDM_P_FreeTimers(const BHDM_Handle hHDMI)
{
	BHDM_P_DestroyTimer(hHDMI, hHDMI->TimerHotPlug, BHDM_P_TIMER_eHotPlug) ;
	BHDM_MONITOR_P_DestroyTimers(hHDMI) ;

#if BHDM_CONFIG_HAS_HDCP22
	BHDM_P_DestroyTimer(hHDMI, hHDMI->TimerScdcStatus, BHDM_P_TIMER_eScdcStatus) ;
	BHDM_P_DestroyTimer(hHDMI, hHDMI->TimerTxScramble, BHDM_P_TIMER_eTxScramble) ;
#endif

	BHDM_P_DestroyTimer(hHDMI, hHDMI->TimerPacketChangeDelay, BHDM_P_TIMER_ePacketChangeDelay) ;
}
#endif


#if !B_REFSW_MINIMAL
BERR_Code BHDM_ConfigurePreEmphasis(
	const BHDM_Handle hHDMI, BHDM_PreEmphasisSetting eValue)
{
	BERR_Code      rc = BERR_SUCCESS ;
	uint8_t uDriverAmp ;

	BDBG_ENTER(BHDM_ConfigurePreEmphasis) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	switch (eValue)
	{
	case BHDM_PreEmphasis_eOFF :
		uDriverAmp = hHDMI->DeviceSettings.uiDriverAmpDefault  ;
		break ;

	case BHDM_PreEmphasis_eLOW :
	case BHDM_PreEmphasis_eMED :
	case BHDM_PreEmphasis_eMAX :
		uDriverAmp = 0xF ;
		break ;

	default :
		BDBG_ERR(("Tx%d: Invalid PreEmphasis Value specified ", hHDMI->eCoreId)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	BHDM_P_SetPreEmphasisMode (hHDMI, eValue, uDriverAmp);

done:
	BDBG_LEAVE(BHDM_ConfigurePreEmphasis) ;

	return rc ;
}
#endif
#endif /*#ifndef BHDM_FOR_BOOTUPDATER */


#if BHDM_CONFIG_PRE_EMPHASIS_SUPPORT
/******************************************************************************
Summary:
	Get current pre-emphasis settings
*******************************************************************************/
BERR_Code BHDM_GetPreEmphasisConfiguration(
	const BHDM_Handle hHDMI,
	BHDM_PreEmphasis_Configuration *stPreEmphasisConfig
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_GetPreEmphasisConfiguration);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BHDM_P_GetPreEmphasisConfiguration (hHDMI, stPreEmphasisConfig);

	BDBG_LEAVE(BHDM_GetPreEmphasisConfiguration);
	return rc;
}


/******************************************************************************
Summary:
	Set pre-emphasis settings with the provided information
*******************************************************************************/
BERR_Code BHDM_SetPreEmphasisConfiguration(
	const BHDM_Handle hHDMI,
	BHDM_PreEmphasis_Configuration *stPreEmphasisConfig
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_ENTER(BHDM_SetPreEmphasisConfiguration);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* Set Preemphasis configurations */
	BHDM_P_SetPreEmphasisConfiguration(hHDMI, stPreEmphasisConfig);


	BDBG_LEAVE(BHDM_SetPreEmphasisConfiguration);
	return rc;
}
#endif


/******************************************************************************
Summary:
	Set data transferring mode (master or slave) for HDMI
*******************************************************************************/
BERR_Code BHDM_SetHdmiDataTransferMode(
	const BHDM_Handle hHDMI,
	bool masterMode
)
{
	BERR_Code rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	BDBG_ENTER(BHDM_SetHdmiDataTransferMode);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset) ;
	Register &= ~( BCHP_MASK(HDMI_FIFO_CTL, MASTER_OR_SLAVE_N)
				| BCHP_MASK(HDMI_FIFO_CTL, USE_FULL));

	Register |= BCHP_FIELD_DATA(HDMI_FIFO_CTL, MASTER_OR_SLAVE_N, masterMode?1:0)
				| BCHP_FIELD_DATA(HDMI_FIFO_CTL, USE_FULL, masterMode?0:1);
	BREG_Write32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset, Register) ;

	BDBG_LEAVE(BHDM_SetHdmiDataTransferMode);
	return rc;
}


/******************************************************************************
Summary:
	Get the current data transferring mode (master or slave) for HDMI
*******************************************************************************/
BERR_Code BHDM_GetHdmiDataTransferMode(
	const BHDM_Handle hHDMI,
	bool *masterMode
)
{
	BERR_Code rc = BERR_SUCCESS;
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset ;

	BDBG_ENTER(BHDM_GetHdmiDataTransferMode);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;


	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	Register = BREG_Read32(hRegister, BCHP_HDMI_FIFO_CTL + ulOffset) ;
	*masterMode =
		1 == BCHP_GET_FIELD_DATA(Register, HDMI_FIFO_CTL, MASTER_OR_SLAVE_N) ;

	BDBG_LEAVE(BHDM_GetHdmiDataTransferMode);
	return rc;
}


/******************************************************************************
Summary:
Set the color mode setting and update the General Control Packet to reflect the current color mode settings
*******************************************************************************/
BERR_Code BHDM_SetColorDepth(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BHDM_ColorDepth_Settings *pstColorDepthSettings
)
{
	BERR_Code rc = BERR_SUCCESS;
#if BHDM_HAS_HDMI_1_3_FEATURES
	BREG_Handle hRegister ;
	uint32_t Register, ulOffset;
	uint8_t GcpSubPacketByte1 ;
	BAVC_HDMI_GCP_ColorDepth eColorDepth ;
	BAVC_Colorspace eColorSpace ;
	BFMT_VideoFmt eVideoFmt ;
	const BFMT_VideoInfo *pVideoFormatInfo ;
	bool bHdmi4Kp50_60Format ;
	bool bHdmi420ColorSpace ;

	BDBG_ENTER(BHDM_SetColorDepth);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hRegister = hHDMI->hRegister ;
	ulOffset = hHDMI->ulOffset ;

	eVideoFmt = hHDMI->DeviceSettings.eInputVideoFmt ;
	eColorSpace = pstColorDepthSettings->eColorSpace ;

	bHdmi4Kp50_60Format = BFMT_IS_4kx2k_50_60HZ(eVideoFmt) ;
	bHdmi420ColorSpace = (eColorSpace == BAVC_Colorspace_eYCbCr420) ? true : false ;

	pVideoFormatInfo =
		(BFMT_VideoInfo *) BFMT_GetVideoFormatInfoPtr(eVideoFmt) ;

	BDBG_MSG(("Set ColorDepth %d for %s  with colorspace %d",
		pstColorDepthSettings->eBitsPerPixel,
		pVideoFormatInfo->pchFormatStr, eColorSpace)) ;

	/* Check bits per pixel settings */
	switch (pstColorDepthSettings->eBitsPerPixel)
	{
	case BAVC_HDMI_BitsPerPixel_e24bit:
		/* regular 24bpp mode; no deep color */

		eColorDepth = BAVC_HDMI_GCP_ColorDepth_e24bpp ;
		GcpSubPacketByte1 = 0 ;
		break;


	case BAVC_HDMI_BitsPerPixel_e30bit:
		if ((bHdmi4Kp50_60Format && bHdmi420ColorSpace && !hHDMI->AttachedEDID.RxHdmiForumVsdb.DeepColor_420_30bit)
		||  (!bHdmi4Kp50_60Format && !bHdmi420ColorSpace && !hHDMI->AttachedEDID.RxVSDB.DeepColor_30bit))
		{
			BDBG_WRN(("Tx%d: Attached sink does not support 10-bit deep color mode for %s",
				hHDMI->eCoreId, pVideoFormatInfo->pchFormatStr)) ;
			rc = BERR_NOT_SUPPORTED ;
			goto done;
		}

		eColorDepth = BAVC_HDMI_GCP_ColorDepth_e30bpp ;
		GcpSubPacketByte1 = BAVC_HDMI_GCP_ColorDepth_e30bpp ;
		break;


	case BAVC_HDMI_BitsPerPixel_e36bit:
		if ((bHdmi4Kp50_60Format && bHdmi420ColorSpace && !hHDMI->AttachedEDID.RxHdmiForumVsdb.DeepColor_420_36bit)
		||  (!bHdmi4Kp50_60Format && !bHdmi420ColorSpace && !hHDMI->AttachedEDID.RxVSDB.DeepColor_36bit))
		{
			BDBG_WRN(("Tx%d: Attached sink does not support 12-bit deep color mode for %s",
				hHDMI->eCoreId, pVideoFormatInfo->pchFormatStr)) ;
			rc = BERR_NOT_SUPPORTED ;
			goto done;
		}

		eColorDepth = BAVC_HDMI_GCP_ColorDepth_e36bpp ;
		GcpSubPacketByte1 = BAVC_HDMI_GCP_ColorDepth_e36bpp ;
		break;


	case BAVC_HDMI_BitsPerPixel_e48bit:
		BDBG_WRN(("Tx%d: 16 bit deep color is not supported",
			hHDMI->eCoreId));
		rc = BERR_NOT_SUPPORTED ;
		goto done ;

	default:  /* use 24bpp if unknown color depth */
		BDBG_WRN(("Tx%d: Invalid Color Depth %d specified; default to 24bpp",
			hHDMI->eCoreId,	pstColorDepthSettings->eBitsPerPixel));

		eColorDepth = BAVC_HDMI_GCP_ColorDepth_e24bpp ;
		GcpSubPacketByte1 = 0 ;

		break;
	}

	/* Configure color depth and GCP packets related register so that the hardware
		will update the packing phase accordingly */
	Register = BREG_Read32(hRegister, BCHP_HDMI_DEEP_COLOR_CONFIG_1 + ulOffset);
	Register &= ~BCHP_MASK(HDMI_DEEP_COLOR_CONFIG_1, COLOR_DEPTH) ;
	Register &= ~BCHP_MASK(HDMI_DEEP_COLOR_CONFIG_1, DEFAULT_PHASE) ;
	Register |= BCHP_FIELD_DATA(HDMI_DEEP_COLOR_CONFIG_1, COLOR_DEPTH, eColorDepth) ;
	BREG_Write32(hRegister, BCHP_HDMI_DEEP_COLOR_CONFIG_1 + ulOffset, Register);

	Register = BREG_Read32(hRegister, BCHP_HDMI_GCP_WORD_1 + ulOffset);
	Register &= ~BCHP_MASK(HDMI_GCP_WORD_1, GCP_SUBPACKET_BYTE_1) ;
	Register |= BCHP_FIELD_DATA(HDMI_GCP_WORD_1, GCP_SUBPACKET_BYTE_1, GcpSubPacketByte1) ;
	BREG_Write32(hRegister, BCHP_HDMI_GCP_WORD_1 + ulOffset, Register);

	Register = BREG_Read32(hRegister, BCHP_HDMI_GCP_CONFIG + ulOffset);
	Register &= ~BCHP_MASK(HDMI_GCP_CONFIG, GCP_ENABLE) ;
	Register |= BCHP_FIELD_DATA(HDMI_GCP_CONFIG, GCP_ENABLE, 1) ;
	BREG_Write32(hRegister, BCHP_HDMI_GCP_CONFIG + ulOffset, Register);

	BKNI_Memcpy(&hHDMI->DeviceSettings.stColorDepth, pstColorDepthSettings, sizeof(BHDM_ColorDepth_Settings));
done:
	BDBG_LEAVE(BHDM_SetColorDepth);
	return rc;
#else
	BSTD_UNUSED(hHDMI) ;
	BSTD_UNUSED(pstColorDepthSettings) ;
	return rc ;
#endif
}


#if !B_REFSW_MINIMAL
/******************************************************************************
Summary:
Get the current color depth setting
*******************************************************************************/
BERR_Code BHDM_GetColorDepth(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BHDM_ColorDepth_Settings *pstColorDepthSettings /* [out] color depth setting returns */
)
{
	BDBG_ENTER(BHDM_GetColorDepth);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (pstColorDepthSettings)
		BKNI_Memcpy(pstColorDepthSettings, &hHDMI->DeviceSettings.stColorDepth, sizeof(BHDM_ColorDepth_Settings));

	BDBG_LEAVE(BHDM_GetColorDepth);
	return BERR_SUCCESS;
}
#endif


/******************************************************************************
Summary:
Set the Video Settings to reflect the current video properties (colospace, color depth etc.)
*******************************************************************************/
BERR_Code BHDM_SetVideoSettings(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BHDM_Video_Settings *pstVideoSettings
)
{
	BERR_Code rc = BERR_SUCCESS;
	bool bPixelEncodingChange =false ;
	BHDM_ColorDepth_Settings stColorDepthSettings ;

	BDBG_ENTER(BHDM_SetVideoSettings);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_EnterCriticalSection() ;

	/* The analog phy (PLL) settings are identical for 422 12 bits and 444 8 bits */
	/* set our internal bpp to 8 bits for freq calculations and phy settings */
	if ((pstVideoSettings->eColorSpace == BAVC_Colorspace_eYCbCr422)
	&& (pstVideoSettings->eBitsPerPixel >= BAVC_HDMI_BitsPerPixel_e30bit))
	{
		pstVideoSettings->eBitsPerPixel = BAVC_HDMI_BitsPerPixel_e24bit ;
	}

	/* set color depth */
	stColorDepthSettings.eBitsPerPixel = pstVideoSettings->eBitsPerPixel ;
	stColorDepthSettings.eColorSpace = pstVideoSettings->eColorSpace ;
	rc = BHDM_SetColorDepth(hHDMI, &stColorDepthSettings) ;
	if (rc) {rc = BERR_TRACE(rc) ;}

	bPixelEncodingChange =
		((pstVideoSettings->eColorSpace == BAVC_Colorspace_eYCbCr420)
		&& (hHDMI->DeviceSettings.stVideoSettings.eColorSpace != BAVC_Colorspace_eYCbCr420))

		|| ((pstVideoSettings->eColorSpace != BAVC_Colorspace_eYCbCr420)
		&& (hHDMI->DeviceSettings.stVideoSettings.eColorSpace == BAVC_Colorspace_eYCbCr420))

		|| ((pstVideoSettings->eColorSpace == BAVC_Colorspace_eYCbCr422)
		&& (hHDMI->DeviceSettings.stVideoSettings.eColorSpace != BAVC_Colorspace_eYCbCr422))

		|| ((pstVideoSettings->eColorSpace != BAVC_Colorspace_eYCbCr422)
		&& (hHDMI->DeviceSettings.stVideoSettings.eColorSpace == BAVC_Colorspace_eYCbCr422)) ;

	BDBG_MSG(("Force Pixel Encoding Change: %s", bPixelEncodingChange ? "yes" : "no")) ;


	/* force an update to the HDMI Phy Output */
	/*   if not already required to do so e.g. resume from standby  */
	/*   OR settings have changed */
	if (!hHDMI->DeviceSettings.bForceEnableDisplay)
	{
		hHDMI->DeviceSettings.bForceEnableDisplay =
			((hHDMI->DeviceSettings.stVideoSettings.eBitsPerPixel != pstVideoSettings->eBitsPerPixel)
			|| (bPixelEncodingChange)) ;
	}


	if (!hHDMI->bForcePacketUpdates)
	{
		hHDMI->bForcePacketUpdates =
			(( hHDMI->DeviceSettings.stVideoSettings.eColorSpace != pstVideoSettings->eColorSpace)
			|| (hHDMI->DeviceSettings.stVideoSettings.eColorRange != pstVideoSettings->eColorRange)) ;
	}

	BKNI_Memcpy(&hHDMI->DeviceSettings.stVideoSettings, pstVideoSettings, sizeof(BHDM_Video_Settings));
	BKNI_LeaveCriticalSection() ;

	BDBG_LEAVE(BHDM_SetVideoSettings);
	return rc;

}

/******************************************************************************
Summary:
Get the current Video Settings (colospace, color depth etc)
*******************************************************************************/
BERR_Code BHDM_GetVideoSettings(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BHDM_Video_Settings *stVideoSettings /* [out] color depth setting returns */
)
{
	BDBG_ENTER(BHDM_GetVideoSettings);
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BDBG_ASSERT(stVideoSettings) ;
	BKNI_Memset(stVideoSettings, 0,  sizeof(BHDM_Video_Settings)) ;

	if (stVideoSettings)
		BKNI_Memcpy(stVideoSettings, &hHDMI->DeviceSettings.stVideoSettings, sizeof(BHDM_Video_Settings));

	BDBG_LEAVE(BHDM_GetVideoSettings);
	return BERR_SUCCESS;
}

BERR_Code BHDM_GetTxSupportStatus(
   const BHDM_Handle hHDMI,		   /* [in] HDMI handle */
   BHDM_TxSupport *pstTxSupport /* [out] TxSupport Status; */
)
{
	BKNI_Memset(pstTxSupport, 0, sizeof(BHDM_TxSupport)) ;

	BKNI_Memcpy(pstTxSupport, &hHDMI->TxSupport, sizeof(BHDM_TxSupport)) ;
	return BERR_SUCCESS ;
}
