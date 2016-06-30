/***************************************************************************
 *     Copyright (c) 2002-2014, Broadcom Corporation
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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bhdm.h"
#include "bhdm_priv.h"
#include "bavc.h"
#include "breg_endian.h"


#if BHDM_CONFIG_HDMI_3D_SUPPORT
#include "bhdm_edid_3d.h"
#endif

#if BHDM_CONFIG_MHL_SUPPORT
#include "bhdm_mhl_priv.h"
#endif


/*=************************ Module Overview ********************************
  The EDID (Enhanced Display Identification Data) functions provide support
  for reading/interpretting the EDID prom contained in the DVI/HDMI Receiver.

  The PROM has an I2C address of 0xA0.  These functions support the Enhanced
  DDC protocol as needed.
***************************************************************************/


BDBG_MODULE(BHDM_EDID) ;


/******************************************************************************
Summary:
Enumeration containing Audio Formats specified in CEA Short Audio Descriptors
*******************************************************************************/
typedef enum BHDM_EDID_P_AudioFormat
{
	BHDM_EDID_P_AudioFormat_eReserved,
	BHDM_EDID_P_AudioFormat_ePCM,
	BHDM_EDID_P_AudioFormat_eAC3,
	BHDM_EDID_P_AudioFormat_eMPEG1,
	BHDM_EDID_P_AudioFormat_eMP3,
	BHDM_EDID_P_AudioFormat_eMPEG2,
	BHDM_EDID_P_AudioFormat_eAAC,
	BHDM_EDID_P_AudioFormat_eDTS,
	BHDM_EDID_P_AudioFormat_eATRAC,
	BHDM_EDID_P_AudioFormat_eOneBit,
	BHDM_EDID_P_AudioFormat_eDDPlus,
	BHDM_EDID_P_AudioFormat_eDTSHD,
	BHDM_EDID_P_AudioFormat_eMATMLP,
	BHDM_EDID_P_AudioFormat_eDST,
	BHDM_EDID_P_AudioFormat_eWMAPro,
	BHDM_EDID_P_AudioFormat_eMaxCount
} BHDM_EDID_P_AudioFormat ;



/******************************************************************************
Summary:
Enumeration containing Sample Rates specified in CEA Short Audio Descriptors
*******************************************************************************/
typedef enum BHDM_EDID_P_AudioSampleRate
{
	BHDM_EDID_P_AudioSampleRate_e32KHz  =  1,
	BHDM_EDID_P_AudioSampleRate_e44KHz  =  2,
	BHDM_EDID_P_AudioSampleRate_e48KHz  =  4,
	BHDM_EDID_P_AudioSampleRate_e88KHz  =  8,
	BHDM_EDID_P_AudioSampleRate_e96KHz  = 16,
	BHDM_EDID_P_AudioSampleRate_e176KHz = 32,
	BHDM_EDID_P_AudioSampleRate_e192KHz = 64
} BHDM_EDID_P_AudioSampleRate;



typedef struct _BHDM_EDID_P_CEA_861B_VIDEO_FORMAT_
{
	uint8_t            CeaVideoCode ;      /* Short Descriptor */
	uint16_t           HorizontalPixels ;  /* Horiz Active Pixels */
	uint16_t           VerticalPixels ;	  /* Vertical Active Pixels */
	BAVC_ScanType      eScanType ;         /* Progressive, Interlaced */
	BAVC_FrameRateCode eFrameRateCode ;    /* Vertical Frequency */
	BFMT_AspectRatio   eAspectRatio ;      /* Horiz  to Vertical Ratio */
} BHDM_EDID_P_CEA_861B_VIDEO_FORMAT ;


const BHDM_EDID_P_CEA_861B_VIDEO_FORMAT BHDM_EDID_P_Cea861bFormats[] =
{
/* Video	Horizontal	Vertical */
/* Codes	(pixels)	(pixels)		  i/p	                Vertical Freq				Aspect Ratio			Remark */

	{ 1,	640,		480, 	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e4_3}, 	/* Default format */
	{ 2,	720,		480, 	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e4_3}, 	/* EDTV */
	{ 3,	720,		480,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e16_9},	/* EDTV */
	{ 4,	1280,		720,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e16_9},	/* HDTV */
	{ 5,	1920,		1080,	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e16_9},	/* HDTV */
	{ 6,    /*720*/1440,	480,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e4_3}, 	/* Optional Double clock for 720x480i */
	{ 7, 	/*720*/1440,	480,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e16_9},	/* Optional Double clock for 720x480i */
	{10,	2880,		480,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e4_3},		/* Game Console */
	{11,	2880,		480,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e16_9},	/* Game Console */
	{14,	1440,		480,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e4_3},		/* high-end DVD */
	{15,	1440,		480,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e16_9},	/* high-end DVD */
	{16,	1920,		1080,	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e16_9},	/* Optional HDTV */

	{17,	720,		576,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e4_3},		/* EDTV */
	{18,	720,		576,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e16_9},	/* EDTV */
	{19,	1280,		720,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e16_9},	/* HDTV */
	{20,	1920,		1080,	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e16_9},	/* HDTV */
	{21,	/*720*/1440,	576,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e4_3},		/* Optional Double clock for 720x576i */
	{22,	/*720*/1440,	576,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e16_9},	/* Optional Double clock for 720x576i */
	{25,	2880,		576,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e4_3},		/* Game Console */
	{26,	2880,		576,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e16_9},	/* Game Console */
	{29,	1440,		576,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e4_3},		/* high-end DVD */
	{30,	1440,		576,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e16_9},	/* high-end DVD */
	{31,	1920,		1080,	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e16_9},	/* Optional HDTV */

	{32,	1920,		1080,	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e23_976, BFMT_AspectRatio_e16_9},	/* Optional HDTV */
	{33,	1920,		1080,	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e25,     BFMT_AspectRatio_e16_9},	/* Optional HDTV */
	{34,	1920,		1080,	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e30,	BFMT_AspectRatio_e16_9},	       /* Optional HDTV */
	{35,	2880,		480,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e4_3},		/* 4x Pixel Rep */
	{36,	2880,		480,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e16_9},	/* 4x Pixel Rep */
	{37,	2880,		576,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,   	BFMT_AspectRatio_e4_3},		/* 4x Pixel Rep */
	{38,	2880,		576,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,   	BFMT_AspectRatio_e16_9},	/* 4x Pixel Rep */

	{60,	1280,		720,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e24,   	BFMT_AspectRatio_e16_9},
	{61,	1280,		720,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e25,   	BFMT_AspectRatio_e16_9},
	{62,	1280,		720,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e30,   	BFMT_AspectRatio_e16_9},

#if BHDM_CONFIG_4Kx2K_30HZ_SUPPORT
	{63,	1920,		1080,	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e119_88,	BFMT_AspectRatio_e16_9},	/* Optional HDTV */
	{64,	1920,		1080,	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e100,		BFMT_AspectRatio_e16_9},	/* Optional HDTV */

	{93, 3840, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e24, 		BFMT_AspectRatio_e16_9},
	{94, 3840, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e25, 		BFMT_AspectRatio_e16_9},
	{95, 3840, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e30, 		BFMT_AspectRatio_e16_9},
#endif

#if BHDM_CONFIG_4Kx2K_60HZ_SUPPORT
	{96, 3840, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e50, BFMT_AspectRatio_e16_9},
	{97, 3840, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e59_94, BFMT_AspectRatio_e16_9},
#endif


#ifdef UNSUPPORTED
	{ 8,	720(1440),	240,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e4_3},		/* Double clock for 720x240p */
	{ 9,	720(1440),	240,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e16_9},	/* Double clock for 720x240p */
	{12,	(2880),		240,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e4_3},		/* Game Console */
	{13,	(2880),		240,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e59_94,	BFMT_AspectRatio_e16_9},	/* Game Console */
	{23,	720(1440),	288,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e4_3},		/* Double clock for 720x288p */
	{24,	720(1440),	288,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e16_9},	/* Double clock for 720x288p */
	{27,	(2880),		288,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e4_3},		/* Game Console */
	{28,	(2880),		288,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e50,		BFMT_AspectRatio_e16_9},	/* Game Console */

	/* UnSupported Frame Rates */
	{39, 1920, 		1080, /*(1250 total)*/BAVC_ScanType_eInterlaced, BAVC_FrameRateCode_e50 BFMT_AspectRatio_e16_9},
	{40, 1920, 		1080,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e100, BFMT_AspectRatio_e16_9},
	{41, 1280, 		720,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e100, BFMT_AspectRatio_e16_9},
	{42, 720, 		576,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e100, BFMT_AspectRatio_e4_3},
	{43, 720, 		576,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e100, BFMT_AspectRatio_e16_9},
	{44, 720(1440), 	576,  	BAVC_ScanType_eInterlaced, 	BAVC_FrameRateCode_e100, BFMT_AspectRatio_e4_3},
	{45, 720(1440), 	576,  	BAVC_ScanType_eInterlaced, 	BAVC_FrameRateCode_e100, BFMT_AspectRatio_e16_9},
	{46, 1920, 		1080,  	BAVC_ScanType_eInterlaced, 	BAVC_FrameRateCode_e120, BFMT_AspectRatio_e16_9},
	{47, 1280, 		720,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e120, BFMT_AspectRatio_e16_9},
	{48, 720, 		480,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e120, BFMT_AspectRatio_e4_3},
	{49, 720, 		480,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e120, BFMT_AspectRatio_e16_9},
	{50, 720(1440), 	480,  	BAVC_ScanType_eInterlaced, 	BAVC_FrameRateCode_e120, BFMT_AspectRatio_e4_3},
	{51, 720(1440), 	480,  	BAVC_ScanType_eInterlaced, 	BAVC_FrameRateCode_e120, BFMT_AspectRatio_e16_9},
	{52, 720, 		576,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e200,  BFMT_AspectRatio_e4_3},
	{53, 720, 		576,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e200,  BFMT_AspectRatio_e16_9},
	{54, 720(1440), 	576,  	BAVC_ScanType_eInterlaced, 	BAVC_FrameRateCode_e200,  BFMT_AspectRatio_e4_3},
	{55, 720(1440), 	576,  	BAVC_ScanType_eInterlaced, 	BAVC_FrameRateCode_e200,  BFMT_AspectRatio_e16_9},
	{56, 720, 		480,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e240, BFMT_AspectRatio_e4_3},
	{57, 720, 		480,  	BAVC_ScanType_eProgressive,	BAVC_FrameRateCode_e240, BFMT_AspectRatio_e16_9},
	{58, 720(1440), 	480,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e240, BFMT_AspectRatio_e4_3},
	{59, 720(1440), 	480,  	BAVC_ScanType_eInterlaced,	BAVC_FrameRateCode_e240, BFMT_AspectRatio_e16_9},


	{98, 4096, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e23_976, 		BFMT_AspectRatio_eUnknown},
	{99, 4096, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e25, 		BFMT_AspectRatio_eUnknown},
	{100, 4096, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e29_97, 	BFMT_AspectRatio_eUnknown},
	{101, 4096, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e50, 		BFMT_AspectRatio_eUnknown},

	{102, 4096, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e59_94, 	BFMT_AspectRatio_eUnknown},
	{103, 4096, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e23_976, 		BFMT_AspectRatio_eUnknown}
	{104, 4096, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e25, 		BFMT_AspectRatio_eUnknown},
	{105, 4096, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e29_97, 	BFMT_AspectRatio_eUnknown},
	{106, 4096, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e50, 		BFMT_AspectRatio_eUnknown},
	{107, 4096, 		2160, 	BAVC_ScanType_eProgressive, 	BAVC_FrameRateCode_e59_94, 	BFMT_AspectRatio_eUnknown},

#endif

/*  0 No Video Code Available (Used with AVI InfoFrame only) */
/*	{35,-127 Reserved for the Future */
} ;


typedef struct _BHDM_EDID_P_ALT_PREFERRED_VIDEOFMT
{
	BFMT_VideoFmt format;
	char *formattext;
}BHDM_EDID_P_ALT_PREFERRED_VIDEOFMT;

const BHDM_EDID_P_ALT_PREFERRED_VIDEOFMT BHDM_EDID_P_AltPreferredFormats[] = {
	{BFMT_VideoFmt_e1080p, "BFMT_VideoFmt_e1080p"},
	{BFMT_VideoFmt_e1080p_50Hz, "BFMT_VideoFmt_e1080p 50Hz"},
	{BFMT_VideoFmt_e720p, "BFMT_VideoFmt_e720p"},
	{BFMT_VideoFmt_e1080i, "BFMT_VideoFmt_e1080i"},
	{BFMT_VideoFmt_e1080i_50Hz, "BFMT_VideoFmt_e1080i 50Hz"},
	{BFMT_VideoFmt_e480p, "BFMT_VideoFmt_e480p"},
	{BFMT_VideoFmt_e576p_50Hz, "BFMT_VideoFmt_e576p"},
	{BFMT_VideoFmt_eDVI_1024x768p, "BFMT_VideoFmt_eDVI_1024x768p"},
	{BFMT_VideoFmt_eDVI_640x480p, "BFMT_VideoFmt_eDVI_640x480p"}
};

#define BHDM_EDID_P_ALT_PREFERRED_VIDEOFMT_MAX \
	(sizeof(BHDM_EDID_P_AltPreferredFormats) / sizeof(BHDM_EDID_P_ALT_PREFERRED_VIDEOFMT))

#if BDBG_DEBUG_BUILD
	static const char Mode[3] = "pi"  ; /* 1080i 480P etc. for debug msg only */
#endif

#define BHDM_EDID_P_BCM_VIDEO_FORMATS_MAX \
	(sizeof(BHDM_EDID_P_Cea861bFormats) / sizeof(BHDM_EDID_P_CEA_861B_VIDEO_FORMAT))


static uint8_t BHDM_EDID_P_EdidCheckSum(uint8_t *pEDID) ;


static BERR_Code BHDM_EDID_P_ParseVideoDB(
	const BHDM_Handle hHDMI,                   /* [in] HDMI handle  */
	uint8_t DataBlockIndex,              /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of Video ID codes */
) ;


static BERR_Code BHDM_EDID_P_ParseFormatPreferenceDB(
	const BHDM_Handle hHDMI,             /* [in] HDMI handle  */
	uint8_t DataBlockIndex,        /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of Video ID codes */
) ;

static BERR_Code BHDM_EDID_P_ParseVideoCapablityDB(
	const BHDM_Handle hHDMI,             /* [in] HDMI handle  */
	uint8_t DataBlockIndex,        /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of Video ID codes */
) ;

static BERR_Code BHDM_EDID_P_ParseYCbCr420VideoDB(
	const BHDM_Handle hHDMI,                   /* [in] HDMI handle  */
	uint8_t DataBlockIndex,              /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of Video ID codes */
) ;


static BERR_Code BHDM_EDID_P_ParseYCbCr420CapabilityMapDB(
	const BHDM_Handle hHDMI,             /* [in] HDMI handle  */
	uint8_t DataBlockIndex,        /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of Video ID codes */
) ;


static BERR_Code BHDM_EDID_P_ParseAudioDB(
	const BHDM_Handle hHDMI,                   /* [in] HDMI handle  */
	uint8_t DataBlockIndex,              /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength             /* [in] length (number) of Video ID codes */
) ;

static BERR_Code BHDM_EDID_P_ParseHDRStaticMetadataDB(
	const BHDM_Handle hHDMI,             /* [in] HDMI handle  */
	uint8_t DataBlockIndex,        /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of Video ID codes */
) ;

static BERR_Code BHDM_EDID_P_ParseEstablishedTimingFormats(
	const BHDM_Handle hHDMI) ;                  /* [in] HDMI handle  */

static BERR_Code BHDM_EDID_P_ParseV1V2TimingExtension(const BHDM_Handle hHDMI) ;

static BERR_Code BHDM_EDID_P_ParseV3TimingExtension(const BHDM_Handle hHDMI);

static BERR_Code BHDM_EDID_P_GetVerticalFrequency(
	uint32_t ulVertFreqMask,            /* [in] Vertical Frequency Mask (bfmt) */
	uint16_t *uiVerticalFrequency)  ;   /* [out] Vertical Frequency value */

static BERR_Code BHDM_EDID_P_ParseMonitorRange(
	const BHDM_Handle hHDMI, uint8_t offset) ;

static BERR_Code BHDM_EDID_P_DetailTiming2VideoFmt(
	const BHDM_Handle hHDMI,
	BHDM_EDID_DetailTiming *pBHDM_EDID_DetailTiming,
	BFMT_VideoFmt *Detail_VideoFmt) ;

static void BHDM_EDID_P_SelectAlternateFormat(
	const BHDM_Handle hHDMI, BFMT_VideoFmt *alternateFormat) ;

#if BDBG_DEBUG_BUILD
static const char * const CeaTagName[] =
{
	"Reserved0  ",
	"Audio    DB",
	"Video    DB",
	"Vendor Specific DB",
	"Speaker  DB",
	"Reserved5  ",
	"Reserved6  ",
	"Extended DB"
} ;

static const char * const ExtendedCeaTagName[] =
{
	"Video Capability Data Block",
	"Vendor-Specific Video Data Block",
	"VESA Display Device Data Block",
	"VESA Video Timing Block Extension",
	"Reserved for HDMI Video Data Block",
	"Colorimetry Data Block",
	"HDR Static Metadata Data Block",
	"Reserved", /* 7 */
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved", /* 12 */
	"Video Format Preference Data Block",
	"YCbCr 4:2:0 Video Data Block",
	"YCbCr 4:2:0 Capability Map Data Block",
	"Reserved for CEA Miscellaneous Audio Fields",
	"Vendor-Specific Audio Data Block",
	"Reserved for HDMI Audio Data Block",
	/* 1931 Reserved for audio-related blocks",*/
	/* "InfoFrame Data Block" */
} ;

static const char * const g_status[] = {"No", "Yes"} ;

static const unsigned char EDIDByPassedText[] = "BYPASSED EDID" ;

#endif


#define BHDM_EDID_USE_DEBUG_EDID 0

/* fix for backward compatibility of BHDM_CONFIG_DEBUG_EDID definition */
#if BHDM_EDID_USE_DEBUG_EDID
#undef BHDM_CONFIG_DEBUG_EDID_PROCESSING
#define BHDM_CONFIG_DEBUG_EDID_PROCESSING 1
#endif

#if BHDM_CONFIG_DEBUG_EDID_PROCESSING

/*
the following EDID can be used to debug any Rx EDID that a BCMxxxx chip may have problems parsing.
a copy of the EDID must first be obtained from the actual TV/Receiver and copied here;

Enable the BHDM_EDID debug messages
     # export msg_modules=BHDM_EDID
to get a copy of the EDID; prior to running any app.

NOTE: This following EDID is used for DEBUGGING ONLY! i.e. debugging the parsing of an EDID which
you may not have the HDMI Rx.  Presumeably the EDID was provided by the user with the TV.

The macro BHDM_EDID_USE_DEBUG_EDID should be reset to 0 when finished debugging
*/

uint8_t DebugRxEdid[] =
{
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x08, 0x6D, 0x48, 0x56, 0xA0, 0xA0, 0xA0, 0xA0,
	0x14, 0x14, 0x01, 0x03, 0x81, 0x5D, 0x34, 0x78, 0x0A, 0x55, 0x50, 0xA7, 0x55, 0x46, 0x99, 0x24,
	0x12, 0x49, 0x4B, 0x21, 0x08, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xE4, 0x57, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
	0x45, 0x00, 0xA2, 0x08, 0x32, 0x00, 0x00, 0x1E, 0xC0, 0x2B, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20,
	0x6E, 0x28, 0x55, 0x00, 0xA2, 0x08, 0x32, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x42,
	0x43, 0x4D, 0x33, 0x35, 0x34, 0x38, 0x5F, 0x35, 0x36, 0x20, 0x33, 0x44, 0x00, 0x00, 0x00, 0xFD,
	0x00, 0x32, 0x4B, 0x0F, 0x50, 0x17, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x25,

	0x02, 0x03, 0x3D, 0x71, 0x78, 0x03, 0x0C, 0x00, 0x10, 0x00, 0xB8, 0x2D, 0x20, 0xC0, 0x0E, 0x01,
	0x01, 0x0F, 0x3F, 0x08, 0x00, 0x20, 0x40, 0x56, 0x16, 0x90, 0x88, 0x00, 0xE6, 0x52, 0x05, 0x04,
	0x40, 0x02, 0x22, 0x20, 0x21, 0x1F, 0x13, 0x11, 0x14, 0x18, 0x15, 0x16, 0x1A, 0x07, 0x06, 0x01,
	0x29, 0x09, 0x07, 0x07, 0x15, 0x50, 0xFF, 0x19, 0x50, 0xFF, 0x82, 0x01, 0x00, 0xE4, 0x57, 0x80,
	0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 0x45, 0x00, 0x13, 0x8E, 0x21, 0x00, 0x00, 0x1E, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xDE
} ;
#endif

static const struct  {
	BHDM_EDID_P_AudioFormat EdidAudioFormat ;
	BAVC_AudioFormat BcmAudioFormat ;
} BHDM_EDID_P_BcmSupportedFormats [] =
{
	{BHDM_EDID_P_AudioFormat_ePCM,   BAVC_AudioFormat_ePCM},
	{BHDM_EDID_P_AudioFormat_eAC3,   BAVC_AudioFormat_eAC3},
	{BHDM_EDID_P_AudioFormat_eMPEG1, BAVC_AudioFormat_eMPEG1},
	{BHDM_EDID_P_AudioFormat_eMP3,   BAVC_AudioFormat_eMP3},
	{BHDM_EDID_P_AudioFormat_eMPEG2, BAVC_AudioFormat_eMPEG2},
	{BHDM_EDID_P_AudioFormat_eAAC,   BAVC_AudioFormat_eAAC},
	{BHDM_EDID_P_AudioFormat_eDTS,   BAVC_AudioFormat_eDTS}

#if BHDM_CONFIG_AUDIO_SUPPORT_DDP
	, {BHDM_EDID_P_AudioFormat_eDDPlus, BAVC_AudioFormat_eDDPlus}
#endif

#if BHDM_CONFIG_AUDIO_SUPPORT_DTSHD
	, {BHDM_EDID_P_AudioFormat_eDTSHD, BAVC_AudioFormat_eDTSHD}
#endif

#if BHDM_CONFIG_AUDIO_SUPPORT_MATMLP
	, {BHDM_EDID_P_AudioFormat_eMATMLP, BAVC_AudioFormat_eMATMLP}
#endif

#if BHDM_CONFIG_AUDIO_SUPPORT_WMAPRO
	, {BHDM_EDID_P_AudioFormat_eWMAPro, BAVC_AudioFormat_eWMAPro}
#endif

#if 0
	, {BHDM_EDID_P_AudioFormat_eATRAC, BAVC_AudioFormat_eATRAC}
	, {BHDM_EDID_P_AudioFormat_eOneBit, BAVC_AudioFormat_eOneBit}
	, {BHDM_EDID_P_AudioFormat_eDST,	 BAVC_AudioFormat_eDST}
#endif

 } ;

static const struct {
	BHDM_EDID_P_AudioSampleRate EdidAudioSampleRate ;
	BAVC_AudioSamplingRate BcmAudioSampleRate ;
} EdidAudioSampleRateTable[] =
{

	{BHDM_EDID_P_AudioSampleRate_e32KHz, BAVC_AudioSamplingRate_e32k},
	{BHDM_EDID_P_AudioSampleRate_e44KHz, BAVC_AudioSamplingRate_e44_1k},
	{BHDM_EDID_P_AudioSampleRate_e48KHz, BAVC_AudioSamplingRate_e48k},
	{BHDM_EDID_P_AudioSampleRate_e96KHz, BAVC_AudioSamplingRate_e96k}

#if BHDM_CONFIG_88_2KHZ_AUDIO_SUPPORT
	, {BHDM_EDID_P_AudioSampleRate_e88KHz,  BAVC_AudioSamplingRate_e88_2k}
#endif

#if BHDM_CONFIG_176_4KHZ_AUDIO_SUPPORT
	, {BHDM_EDID_P_AudioSampleRate_e176KHz, BAVC_AudioSamplingRate_e176_4k}
#endif

#if BHDM_CONFIG_192KHZ_AUDIO_SUPPORT
	, {BHDM_EDID_P_AudioSampleRate_e192KHz, BAVC_AudioSamplingRate_e192k}
#endif

} ;

#if BDBG_DEBUG_BUILD
static const char * const CeaAudioSampleRateTypeText[] =
{
	"32 kHz",
	"44.1 kHz (CD)",
	"48 kHz",
	"96 kHz"
#if BHDM_CONFIG_88_2KHZ_AUDIO_SUPPORT
	, "88.2 kHz"
#endif

#if BHDM_CONFIG_176_4KHZ_AUDIO_SUPPORT
	, "176.4 kHz"
#endif

#if BHDM_CONFIG_192KHZ_AUDIO_SUPPORT
	, "192 kHz"
#endif
} ;
#endif

/******************************************************************************
uint8_t BHDM_EDID_P_EdidCheckSum
Summary:Verify the checksum on an EDID block
*******************************************************************************/
static uint8_t BHDM_EDID_P_EdidCheckSum(uint8_t *pEDID)
{
	uint8_t i ;
	uint8_t checksum = 0 ;


	for (i = 0 ; i < BHDM_EDID_BLOCKSIZE ; i++)
		checksum = checksum + (uint8_t) pEDID[i]  ;

#if BDBG_DEBUG_BUILD
	/*
	-- Determine Checksum if an error (non-zero)
	-- correct value can be inserted above if desired
	*/
	if (checksum % 256)
	{
		BDBG_WRN(("Checksum:      %#X", checksum)) ;
		BDBG_WRN(("Invalid Checksum Byte: %#02X", (unsigned char) pEDID[BHDM_EDID_CHECKSUM])) ;

		/* determine the correct checksum value */
		BDBG_WRN(("Correct Checksum Byte = %#02X",
			(uint8_t) (256 - (checksum - (unsigned int) pEDID[BHDM_EDID_CHECKSUM])))) ;
	}
#endif

	return ( (checksum % 256) ? 0 : 1) ;
} /* end BHDM_EDID_P_EdidCheckSum */



/******************************************************************************
BERR_Code BHDM_EDID_GetNthBlock
Summary:Retrieve the Nth 128-byte EDID block
*******************************************************************************/
BERR_Code BHDM_EDID_GetNthBlock(
   const BHDM_Handle hHDMI,   /* [in] HDMI handle */
   uint8_t BlockNumber, /* [in] EDID Block Number to read */
   uint8_t *pBuffer,    /* [out] pointer to input buffer */
   uint16_t uiBufSize    /* [in] Size of input buffer to write EDID to */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t uiSegment ;
#if ! BHDM_CONFIG_DEBUG_EDID_PROCESSING
	uint8_t timeoutMs ;
#endif
	uint8_t RxDeviceAttached ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	/* check if the requested block already cached */
	if ((hHDMI->bUseCachedEdid)
	&& (BlockNumber == hHDMI->AttachedEDID.CachedBlock)
	&& (hHDMI->edidStatus == BHDM_EDID_STATE_eOK))
	{
		BDBG_MSG(("Skip reading EDID; Block %d already cached", BlockNumber)) ;

		/* copy the cached EDID block to the user buffer (if external) */
		if (pBuffer != hHDMI->AttachedEDID.Block)
			BKNI_Memcpy(pBuffer, hHDMI->AttachedEDID.Block, BHDM_EDID_BLOCKSIZE) ;

		rc = BERR_SUCCESS ;
		goto done ;
	}

	hHDMI->bUseCachedEdid = true ;

	if (uiBufSize <  BHDM_EDID_BLOCKSIZE)
	{
		BDBG_ERR(("Incorrect Specified EDID Block Size: %d; expecting %d",
			uiBufSize, BHDM_EDID_BLOCKSIZE)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}


	/* Use E-DDC protocol for reading the EDID... */

	/* each segment can hold two EDID blocks of 128 bytes each */
	/* determine which segment the requested block lies in */
	uiSegment = BlockNumber / 2 ;


	if (hHDMI->DeviceSettings.UseDebugEdid)
	{
		const uint8_t *DebugRxEdid ;

		BDBG_WRN(("<$$$ BHDM_CONFIG_DEBUG_EDID_PROCESSING  $$$>")) ;
		BDBG_WRN(("<$$$ Using EDID declared in bhdm_edid_debug.c $$$>")) ;

		DebugRxEdid = BHDM_EDID_P_GetDebugEdid() ;

		BKNI_Memcpy(pBuffer,
			DebugRxEdid + (uint8_t) (BHDM_EDID_BLOCKSIZE * (BlockNumber % 2)) + (BHDM_EDID_BLOCKSIZE * 2 * uiSegment),
			BHDM_EDID_BLOCKSIZE) ;
	}
	else
	{
#if BHDM_CONFIG_HAS_HDCP22
		/* make sure polling Auto I2C channels are disabled; prior to the read */
		BKNI_EnterCriticalSection() ;
		BHDM_AUTO_I2C_SetChannels_isr(hHDMI, 0) ;
		BKNI_LeaveCriticalSection() ;
#endif

		/* Read the EDID block; sometimes the EDID block is not ready/available */
		/* try again if a READ failure occurs */

		timeoutMs = 1 ;
		do
		{
#if BHDM_CONFIG_MHL_SUPPORT
			if (hHDMI->bMhlMode)
			{
				BKNI_Memcpy((void *)pBuffer,
							(void *)(hHDMI->hMhl->hDdcReq->pucEdidBlock + (BHDM_EDID_BLOCKSIZE * (BlockNumber % 2))),
							sizeof(uint8_t) * BHDM_P_MHL_EDID_BLOCK_SIZE);
				rc = BERR_SUCCESS;
			}
			else
#endif
			{
				rc = BREG_I2C_ReadEDDC(hHDMI->hI2cRegHandle, (uint8_t) BHDM_EDID_I2C_ADDR,
					(uint8_t) uiSegment,
					(uint8_t) (BHDM_EDID_BLOCKSIZE * (BlockNumber % 2)), /* offset */
					pBuffer, (uint16_t) BHDM_EDID_BLOCKSIZE) ;	/* storage & length  */
			}

			if (rc == BERR_SUCCESS)
				break ;

			BDBG_WRN(("Error reading EDID Block; Attempt to re-read...")) ;
			BKNI_Sleep(1) ;
		} while ( timeoutMs-- ) ;

		if (rc != BERR_SUCCESS)
		{
			BDBG_ERR(("Can't find/read Rx EDID device")) ;
			rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
			goto done ;
		}
	}
#if BHDM_CONFIG_HAS_HDCP22
		/* re-enable any polling Auto I2C channels that had to be disabled prior to the read */
		BKNI_EnterCriticalSection() ;
		BHDM_AUTO_I2C_SetChannels_isr(hHDMI, 1) ;
		BKNI_LeaveCriticalSection() ;
#endif

	/* copy the EDID block to the EDID handle */
	if (pBuffer != hHDMI->AttachedEDID.Block)
		BKNI_Memcpy(hHDMI->AttachedEDID.Block, pBuffer, BHDM_EDID_BLOCKSIZE) ;

	/* check for a valid checksum */
	if (!BHDM_EDID_P_EdidCheckSum(hHDMI->AttachedEDID.Block))
	{
		BDBG_WRN(("Checksum Error for EDID Block #%d Ignored", BlockNumber)) ;
#if 0
		rc = BERR_TRACE(BHDM_EDID_CHECKSUM_ERROR) ;
		goto done ;
#endif
	}

	hHDMI->AttachedEDID.CachedBlock = BlockNumber ;
	hHDMI->edidStatus = BHDM_EDID_STATE_eOK;

done:
	return rc ;
} /* end BHDM_EDID_GetNthBlock */


/******************************************************************************
BERR_Code BHDM_EDID_ReadNthBlock
Summary:Read the Nth 128-byte EDID block
*******************************************************************************/
BERR_Code BHDM_EDID_ReadNthBlock(
   const BHDM_Handle hHDMI,   /* [in] HDMI handle */
   uint8_t BlockNumber, /* [in] EDID Block Number to read */
   uint8_t *pBuffer,    /* [out] pointer to input buffer */
   uint16_t uiBufSize    /* [in] Size of input buffer to write EDID to */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	hHDMI->bUseCachedEdid = false ;
	rc = BHDM_EDID_GetNthBlock(hHDMI, BlockNumber, pBuffer, uiBufSize) ;
	if (rc) {rc = BERR_TRACE(rc) ;}
	return rc ;
}



/******************************************************************************
BERR_Code BHDM_EDID_GetBasicData
Summary: Retrieve the basic EDID data
*******************************************************************************/
BERR_Code BHDM_EDID_GetBasicData(
   const BHDM_Handle hHDMI,               /* [in] HDMI handle */
   BHDM_EDID_BasicData *pMonitorData /* [out] pointer to structure to hold Basic
                                            EDID Data */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t RxDeviceAttached ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	if (hHDMI->AttachedEDID.SupportedDetailTimingsIn1stBlock)
	{
		/* use the first supported descriptor found */
		/* convert to BFMT and return as a preferred format */
		rc = BHDM_EDID_P_DetailTiming2VideoFmt(hHDMI,
			&hHDMI->AttachedEDID.SupportedDetailTimings[0],
			&hHDMI->AttachedEDID.BasicData.eVideoFmt) ;
		if (rc) {rc = BERR_TRACE(rc) ;}
	}
	else
	{
		BDBG_WRN(("No BCM Supported Detail/Preferred Timing Descriptors found; selecting an alternate...")) ;
		BHDM_EDID_P_SelectAlternateFormat(hHDMI, &hHDMI->AttachedEDID.BasicData.eVideoFmt) ;
	}

#if BDBG_DEBUG_BUILD
	{
		const BFMT_VideoInfo *pVideoFormatInfo ;

		pVideoFormatInfo =
			(BFMT_VideoInfo *) BFMT_GetVideoFormatInfoPtr(hHDMI->AttachedEDID.BasicData.eVideoFmt) ;

		BDBG_MSG(("BCM Supported Detail Timings Found: %d",
			hHDMI->AttachedEDID.SupportedDetailTimingsIn1stBlock)) ;
		BDBG_MSG(("HDMI Rx Preferred Video Format: (%d) %s",
			hHDMI->AttachedEDID.BasicData.eVideoFmt,
			pVideoFormatInfo->pchFormatStr)) ;
	}
#endif


	/* copy the EDID Basic Data */
	BKNI_Memcpy(pMonitorData, (void *) &(hHDMI->AttachedEDID.BasicData),
		sizeof(BHDM_EDID_BasicData)) ;

done:
	return rc ;
}/* end BHDM_EDID_GetBasicData */



/******************************************************************************
BERR_Code BHDM_EDID_GetHdmiVsdb
Summary: Retrieve the Vendor Specific Data Block from the first Version 3 Timing
Extension in the EDID
*******************************************************************************/
BERR_Code BHDM_EDID_GetHdmiVsdb(
   const BHDM_Handle hHDMI,                  /* [in] HDMI handle */
   BHDM_EDID_RxVendorSpecificDB *RxVSDB  /* [out] pointer to Vendor Specific Data
                                            Block to hold the retrieved data */
)
{
	uint8_t RxDeviceAttached ;
	BERR_Code rc = BERR_SUCCESS ;


	BDBG_ENTER(BHDM_EDID_GetHdmiVsdb) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(RxVSDB, 0, sizeof(BHDM_EDID_RxVendorSpecificDB)) ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {
		BDBG_WRN(("No Valid EDID Found. Default to DVI Device"));
		goto done;
	}

	if (!hHDMI->AttachedEDID.RxHasHdmiSupport)
		goto done ;

	/* EDID processed at initialization; copy information */
	BKNI_Memcpy(RxVSDB,  &hHDMI->AttachedEDID.RxVSDB,
		sizeof(BHDM_EDID_RxVendorSpecificDB)) ;

done:
	BDBG_LEAVE(BHDM_EDID_GetHdmiVsdb) ;
	return 	rc ;
}


/******************************************************************************
BERR_Code BHDM_EDID_GetHdmiForumVsdb
Summary: Retrieve the optional HDMI 2.0 HDMI Forum Vendor Specific Data Block
from the first Version 3 Timing Extension in the EDID
*******************************************************************************/
BERR_Code BHDM_EDID_GetHdmiForumVsdb(
   const BHDM_Handle hHDMI,                  /* [in] HDMI handle */
   BHDM_EDID_RxHfVsdb *RxHdmiForumVSDB  /* [out] pointer to Vendor Specific Data
                                            Block to hold the retrieved data */
)
{
	uint8_t RxDeviceAttached ;
	BERR_Code rc = BERR_SUCCESS ;


	BDBG_ENTER(BHDM_EDID_GetHdmiForumVsdb) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(RxHdmiForumVSDB, 0, sizeof(BHDM_EDID_RxHfVsdb)) ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {
		BDBG_WRN(("No Valid EDID Found. Default to DVI Device"));
		goto done;
	}

	if (!hHDMI->AttachedEDID.RxHasHdmiSupport)
		goto done ;

	/* EDID processed at initialization; copy information */
	BKNI_Memcpy(RxHdmiForumVSDB,  &hHDMI->AttachedEDID.RxHdmiForumVsdb,
		sizeof(BHDM_EDID_RxHfVsdb)) ;

done:
	BDBG_LEAVE(BHDM_EDID_GetHdmiForumVsdb) ;
	return 	rc ;
} /* BHDM_EDID_GetHdmiForumVsdb */



/******************************************************************************
static void BHDM_EDID_ParseDetailedTimingDescriptor
Summary: Parse a Detailed Timing Descriptor Block
*******************************************************************************/
static void BHDM_EDID_ParseDetailedTimingDescriptor
	(unsigned char *pDescriptor, BHDM_EDID_DetailTiming *DetailedTiming)
{

	BDBG_ENTER(BHDM_EDID_ParseDetailedTimingDescriptor) ;

	DetailedTiming->PixelClock =
    	( ((pDescriptor[BHDM_EDID_DESC_PIXEL_CLOCK_HI]) << 8)
		| ((pDescriptor[BHDM_EDID_DESC_PIXEL_CLOCK_LO]))) / 100 ;


	DetailedTiming->HorizActivePixels =
		    pDescriptor[BHDM_EDID_DESC_HACTIVE_PIXELS_LSB]
		| ((pDescriptor[BHDM_EDID_DESC_HACTIVE_PIXELS_UN_F0] & 0xF0) << 4) ;

	DetailedTiming->HorizBlankingPixels =
		pDescriptor[ BHDM_EDID_DESC_HBLANK_PIXELS_LSB]
		| (((pDescriptor[BHDM_EDID_DESC_HBLANK_PIXELS_UN_0F] & 0x0F) << 8)) ;



	DetailedTiming->VerticalActiveLines =
		    pDescriptor[BHDM_EDID_DESC_VACTIVE_LINES_LSB]
		| ((pDescriptor[BHDM_EDID_DESC_VACTIVE_LINES_UN_F0] & 0xF0) << 4) ;

	DetailedTiming->VerticalBlankingLines =
		pDescriptor[BHDM_EDID_DESC_VBLANK_LINES_LSB]
		| (((pDescriptor[BHDM_EDID_DESC_VBLANK_LINES_UN_0F] & 0x0F) << 8)) ;

	/* Mode */
	DetailedTiming->Mode =
		(pDescriptor[BHDM_EDID_DESC_PREFERRED_FLAGS] & 0x80) ? 1 : 0 ;


	/* HSync Offset / Width */
	DetailedTiming->HSyncOffset =
		   pDescriptor[BHDM_EDID_DESC_HSYNC_OFFSET_LSB]
		| (pDescriptor[BHDM_EDID_DESC_HSYNC_OFFSET_U2_C0] & 0xC0) << 2 ;

	DetailedTiming->HSyncWidth =
		   pDescriptor[BHDM_EDID_DESC_HSYNC_WIDTH_LSB]
		| (pDescriptor[BHDM_EDID_DESC_HSYNC_WIDTH_U2_30] & 0x30) << 4 ;


	/* VSync Offset / Width */
	DetailedTiming->VSyncOffset =
		  ((pDescriptor[BHDM_EDID_DESC_VSYNC_OFFSET_LN_F0] & 0xF0) >> 4)
		| ((pDescriptor[BHDM_EDID_DESC_VSYNC_OFFSET_U2_0C] & 0x0C) << 2) ;

	DetailedTiming->VSyncWidth =
		   (pDescriptor[BHDM_EDID_DESC_VSYNC_WIDTH_LN_0F] & 0x0F)
		| ((pDescriptor[BHDM_EDID_DESC_VSYNC_WIDTH_U2_03] & 0x03) << 4) ;

	/*Screen Size */
	DetailedTiming->HSize_mm =
		    pDescriptor[BHDM_EDID_DESC_HSIZE_MM_LSB]
		| ((pDescriptor[BHDM_EDID_DESC_HSIZE_UN_F0] & 0xF0) << 4) ;

	DetailedTiming->VSize_mm =
		    pDescriptor[BHDM_EDID_DESC_VSIZE_MM_LSB]
		| ((pDescriptor[BHDM_EDID_DESC_VSIZE_UN_0F] & 0x0F) << 8) ;

	DetailedTiming->HorizBorderPixels = pDescriptor[BHDM_EDID_DESC_HBORDER_PIXELS] ;
	DetailedTiming->VerticalBorderLines = pDescriptor[BHDM_EDID_DESC_VBORDER_LINES] ;

	BDBG_LEAVE(BHDM_EDID_ParseDetailedTimingDescriptor) ;
	return ;
} /* END BHDM_EDID_ParseDetailedTimingDescriptor */



/******************************************************************************
BERR_Code BHDM_EDID_P_DetailTiming2VideoFmt(
Summary: Determine the BFMT_VideoFmt for the associated Detailed Timing Format
*******************************************************************************/
static BERR_Code BHDM_EDID_P_DetailTiming2VideoFmt(
	const BHDM_Handle hHDMI,
	BHDM_EDID_DetailTiming *pBHDM_EDID_DetailTiming,
	BFMT_VideoFmt *Detail_VideoFmt
)
{
	BERR_Code rc = BHDM_EDID_DETAILTIMING_NOT_SUPPORTED ;
	const BFMT_VideoInfo *pVideoFormatInfo ;
	BFMT_VideoFmt eVideoFmt ;

	uint8_t i ;
	bool Interlaced ;
	uint16_t
		uiVerticalFrequency ;
	uint16_t AdjustedHorizPixels ;
	uint16_t AdjustedVerticalLines ;

	/* default to VGA format */
	*Detail_VideoFmt = BFMT_VideoFmt_eDVI_640x480p  ;

	/* adjust Detailed Timing vertical parameters for interlaced formats */
	/* Do not adjust the original number of pixels in the Detail Timing  */
	Interlaced = pBHDM_EDID_DetailTiming->Mode ? true : false ;
	if (Interlaced)
		AdjustedVerticalLines = pBHDM_EDID_DetailTiming->VerticalActiveLines * 2 ;
	else
		AdjustedVerticalLines = pBHDM_EDID_DetailTiming->VerticalActiveLines ;

	/* adjust any pixel repititions to match the formats */
	/* as specified in BFMT (e.g 1440 -> 720) */

	if ((pBHDM_EDID_DetailTiming->HorizActivePixels == 1440)
	||  (pBHDM_EDID_DetailTiming->HorizActivePixels == 2880))
		AdjustedHorizPixels = 720 ;
	else
		AdjustedHorizPixels = pBHDM_EDID_DetailTiming->HorizActivePixels ;


#if 0
      /* debug message for conversion */
	BDBG_MSG(("Convert Detailed Timing To Video Format")) ;
#endif

	for (i = 0; i < BFMT_VideoFmt_eMaxCount; i++)
	{
		eVideoFmt = (BFMT_VideoFmt) i ;

		/* check HDMI formats only */
		if (!BFMT_SUPPORT_HDMI(eVideoFmt))
			continue ;

		pVideoFormatInfo = BFMT_GetVideoFormatInfoPtr(eVideoFmt) ;
		if (!pVideoFormatInfo)
		{
#if 0
			/* debug format not found / unknown */
			BDBG_WRN(("eVideoFmt: %d Max Video Count: %d", eVideoFmt,
				BFMT_VideoFmt_eMaxCount)) ;
#endif
			continue;
		}

		/* check if the specified parameters match the requested formats */

		/* 1st, Check if Pixel Format matches */
		if ((pVideoFormatInfo->ulDigitalWidth != AdjustedHorizPixels)
		|| (pVideoFormatInfo->ulDigitalHeight != AdjustedVerticalLines))
		{
			continue;
		}

		/* 2nd, Check Scan Type (i/p) */
		if (pVideoFormatInfo->bInterlaced != Interlaced)
		{
			continue;
		}


		/* 3rd, for all 720p, 1080i/p, and 4K formats, make another
		check to help differentiate the 24, 50, and 60Hz formats.
		This is needed for Detailed Timing Descriptors because the
		format width is the same so the blanking time must also be considered
		when matching formats */

		if ((pVideoFormatInfo->ulDigitalWidth == BFMT_1080I_WIDTH)
		|| (pVideoFormatInfo->ulDigitalWidth == BFMT_1080P_WIDTH)
		|| (pVideoFormatInfo->ulDigitalWidth == BFMT_720P_WIDTH)
		|| (pVideoFormatInfo->ulDigitalWidth == 3840)
		|| (pVideoFormatInfo->ulDigitalWidth == 4096))
		{
			uint16_t ulHorizontalScanWidth ;

			ulHorizontalScanWidth =
				   pBHDM_EDID_DetailTiming->HorizActivePixels
				+ pBHDM_EDID_DetailTiming->HorizBlankingPixels ;

			if (pVideoFormatInfo->ulScanWidth != ulHorizontalScanWidth)
			{
				continue;
			}
		}

		/* use the BFMT freqency parameter to get the Vertical Frequency */
		/* Ignore rc; default of 60Hz will be used for unknown Frequency */
		BHDM_EDID_P_GetVerticalFrequency(
			pVideoFormatInfo->ulVertFreqMask, &uiVerticalFrequency) ;

#if !defined(BHDM_CONFIG_1080P_5060HZ_SUPPORT)
		if ((eVideoFmt == BFMT_VideoFmt_e1080p)
		&& (uiVerticalFrequency >= 50))
		{
			/* 1080p 60hz */
			/* assign format but return unsupported */

			*Detail_VideoFmt = eVideoFmt ;
			BDBG_WRN(("This device does not support %s",
				pVideoFormatInfo->pchFormatStr)) ;
			rc = BHDM_EDID_DETAILTIMING_NOT_SUPPORTED ;
			break ;
		}
#endif

		/* 3rd check the Pixel Clock reported in Detail Timing Block
		    matches the Pxl Frequency specified in the BCM supported format */
		if (pVideoFormatInfo->ulPxlFreq / BFMT_FREQ_FACTOR != pBHDM_EDID_DetailTiming->PixelClock)
		{
			continue;
		}

		/* 4th check the vertical frequency range */
		/* notify if format is out of the vertical range specified in MonitorRange descriptor */
		if  ((hHDMI->AttachedEDID.BcmMonitorRangeParsed)
		&& !((uiVerticalFrequency >= hHDMI->AttachedEDID.MonitorRange.MinVertical)
		&&  (uiVerticalFrequency <= hHDMI->AttachedEDID.MonitorRange.MaxVertical)))
		{
			BDBG_MSG(("  Format %s refresh rate of %dHz does not fall in the Vertical Frequency range of %dHz to %dHz",
				pVideoFormatInfo->pchFormatStr,  uiVerticalFrequency,
				hHDMI->AttachedEDID.MonitorRange.MinVertical,
				hHDMI->AttachedEDID.MonitorRange.MaxVertical)) ;
		}
#if 0
		/* debug Vertical Frequency */
		BDBG_MSG(("Vertical Frequency %d <= %d <=%d",
			hHDMI->AttachedEDID.MonitorRange.MinVertical,
			uiVerticalFrequency,
			hHDMI->AttachedEDID.MonitorRange.MaxVertical));
#endif

		/* 5th Detailed Timings indicate support for 2D formats Only
		make sure the selected format is 2D */
		if (pVideoFormatInfo->eOrientation != BFMT_Orientation_e2D)
		{
			continue;
		}

		/* 6th Make sure format is supported on current platform */
		if (pBHDM_EDID_DetailTiming->PixelClock > BHDM_CONFIG_MAX_TMDS_RATE)
		{
			BDBG_MSG(("HDMI Rx Detailed Format %s", pVideoFormatInfo->pchFormatStr)) ;
			BDBG_MSG(("    Pixel Clock Rate of %d exceeds STB's MAX Pixel Clock Rate %d",
				pBHDM_EDID_DetailTiming->PixelClock, BHDM_CONFIG_MAX_TMDS_RATE)) ;
			continue ;
		}


#if 0
      /* debug message for conversion */
		BDBG_MSG(("   %s (#%d) %4d x %4d %c %dHz",
			pVideoFormatInfo->pchFormatStr, i,
			pVideoFormatInfo->ulDigitalWidth, pVideoFormatInfo->ulDigitalHeight,
			Mode[pVideoFormatInfo->bInterlaced], uiVerticalFrequency)) ;
#endif
		/* found associated BFMT_eVideoFmt */
		*Detail_VideoFmt = eVideoFmt ;
		rc = BERR_SUCCESS ;
		break ;
	}

	return rc ;
}


static void BHDM_EDID_P_SelectAlternateFormat(const BHDM_Handle hHDMI, BFMT_VideoFmt *alternateFormat)
{
	uint8_t i ;

	/* default to VGA in case the alternate table does not have an entry */
	/* NOTE VGA is the last entry in the table */

	*alternateFormat = BFMT_VideoFmt_eDVI_640x480p ;

	for (i = 0 ; i < BHDM_EDID_P_ALT_PREFERRED_VIDEOFMT_MAX ; i++)
	{
		if (hHDMI->AttachedEDID.BcmSupportedVideoFormats[BHDM_EDID_P_AltPreferredFormats[i].format])
		{
#if BHDM_CONFIG_MHL_SUPPORT
			if (BHDM_MHL_P_ValidatePreferredFormat(hHDMI, BHDM_EDID_P_AltPreferredFormats[i].format) ==  BERR_SUCCESS)
#endif
			{
				BDBG_WRN(("Selecting alternate supported BCM supported format: %s",
					BHDM_EDID_P_AltPreferredFormats[i].formattext));
				*alternateFormat  = BHDM_EDID_P_AltPreferredFormats[i].format;
				break;
			}
		}
	}
}


/******************************************************************************
BERR_Code BHDM_EDID_GetDetailTiming
Summary: Retrieve EDID Detail Timing Data
*******************************************************************************/
BERR_Code BHDM_EDID_GetDetailTiming(
	const BHDM_Handle hHDMI,			/* [in] HDMI handle */
	uint8_t NthTimingRequested, /* [in] number of the Detail Block to get */
	BHDM_EDID_DetailTiming
		*pBHDM_EDID_DetailTiming, /* [out] pointer to a BHDM_EDID_DetailTiming
								   structure to hold the data */
	BFMT_VideoFmt *BCM_VideoFmt
)
{
	/* the first 128 bytes of the EDID should contain the preferred
	 * timing block.  It should be in the first Descriptor Block...
	 */

	/*
	 *	LISTED HERE FOR DOCUMENTATION PURPOSES ONLY...
	 *
	 *  By searching for the following DESCRIPTOR BLOCKS
	 *  EDID information (Name, S/N etc) can be gathered from the
	 *  Display Device; the search and retrieval of such data
	 *  could be implemented in this function
	 *
	 *	static DESCRIPTOR_BLOCK TimingDescriptors[] =
	 *	{
	 *		{0x00, 0x00, 0x00, 0xFF, 0x00}, -- Monitor Serial Number
	 *		{0x00, 0x00, 0x00, 0xFE, 0x00}, -- ASCII String
	 *		{0x00, 0x00, 0x00, 0xFD, 0x00}, -- Monitor Range Limits
	 *		{0x00, 0x00, 0x00, 0xFC, 0x00}, -- Monitor Name (ASCII)
	 *		{0x00, 0x00, 0x00, 0xFB, 0x00}, -- Color Point Data
	 *		{0x00, 0x00, 0x00, 0xFA, 0x00}, -- Additional Standard Timings
	 *				                        -- F9 - 11 Unused
	 *			                            -- 0F - 00 Manufacturer Descriptor
	 *		{0x00, 0x00, 0x00, 0x10, 0x00}, -- Dummy Descriptor
	 *	} ;
	 */

	BERR_Code rc = BERR_SUCCESS ;

	uint8_t i, j ;
	uint8_t offset ;
	uint8_t MaxDescriptors ;
	uint8_t extensions, DataOffset ;
	uint8_t NumDetailedTimingsFound = 0 ;
	uint8_t RxDeviceAttached ;

	BDBG_ENTER(BHDM_EDID_GetDetailTiming) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	*BCM_VideoFmt = BFMT_VideoFmt_eNTSC; /* initialize the out param for error path */

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

#if BDBG_DEBUG_BUILD
	if (hHDMI->DeviceSettings.BypassEDIDChecking)
	{
		goto done ;
	}
#endif

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done;
	}

	if (!NthTimingRequested)
	{
		BDBG_ERR(("Incorrectly specified Detail Timing: %d", NthTimingRequested)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	/* set Number of Detailed Timings Found to 2; number in 1st Block */
	NumDetailedTimingsFound = hHDMI->AttachedEDID.SupportedDetailTimingsIn1stBlock ;

	/* up to two preferred formats should be stored in handle; if requested copy and exit */
	if ((NumDetailedTimingsFound) && (NthTimingRequested <= NumDetailedTimingsFound))
	{
		BKNI_Memcpy(pBHDM_EDID_DetailTiming,
			&(hHDMI->AttachedEDID.SupportedDetailTimings[NthTimingRequested - 1]),
			sizeof(BHDM_EDID_DetailTiming)) ;

		/* function below displays just the BFMT name; parse/display done at Initialize */
		rc = BHDM_EDID_P_DetailTiming2VideoFmt(hHDMI, pBHDM_EDID_DetailTiming, BCM_VideoFmt);
		if (rc == BERR_SUCCESS)
		{
#if BHDM_CONFIG_MHL_SUPPORT
			rc = BHDM_MHL_P_ValidatePreferredFormat(hHDMI,  *BCM_VideoFmt);
			if (rc != BERR_SUCCESS) goto BcmSupportedFormatNotFound;
#endif
			goto BcmSupportedFormatFound;
		}
		else {
			goto BcmSupportedFormatNotFound;
		}
	}

	/* otherwise search the extensions for the requested format */
	extensions = hHDMI->AttachedEDID.BasicData.Extensions ;
	if (!extensions)
		goto BcmSupportedFormatNotFound;

	/* Search EDID Extension blocks for additional timing descriptors */
	for (i = 1 ; i <= extensions; i++)
	{
		/* read the next 128 Byte EDID block */
		BHDM_CHECK_RC(rc, BHDM_EDID_GetNthBlock(hHDMI, i, hHDMI->AttachedEDID.Block, BHDM_EDID_BLOCKSIZE)) ;

		/* check Extension Tag type for Timing Data */
		offset = 0 ;
		if (hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_TAG] != BHDM_EDID_EXT_TIMING_DATA)
			continue ;

		/* check if data blocks exist before the 18 Byte Detailed Timing data */
		DataOffset = hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_DATA_OFFSET] ;
		if (DataOffset == 0) /* no timing info in the block */
		{
			BDBG_MSG(("EDID Ext contains no Detail Timing Descriptors")) ;
			continue ;          /* continue to the next Timing Extension */
		}

		/* determine the number of timing data */
		switch (hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_VERSION])
		{
		case 0x01 :  /* Version 1 See CEA-861        */
			MaxDescriptors = 6 ;
			break ;

		case 0x02 :  /* Version 2 See CEA-861 A Spec */
		case 0x03 :  /* Version 3 See CEA-861 B Spec */
			MaxDescriptors = hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_MONITOR_SUPPORT] ;
			MaxDescriptors = MaxDescriptors & 0x0F ;
			MaxDescriptors = MaxDescriptors - NumDetailedTimingsFound ;

			break ;

		default :
			/* Check all blocks regardless of */
			/* the version of the EDID Extension */
			BDBG_WRN(("Timing Extension Version '%d' Not Supported",
				hHDMI->AttachedEDID.Block[offset+1])) ;
			rc = BHDM_EDID_EXT_VERSION_NOT_SUPPORTED ;
			goto BcmSupportedFormatNotFound;
		}

		BDBG_MSG(("EDID Extension #%d contains additional Timing Data", i)) ;


		/* skip start of EDID Extension Block */
		offset = offset + hHDMI->AttachedEDID.Block[offset + 2] ;

		for (j = 0 ; j < MaxDescriptors; j++)
		{
			/* advance to next descriptor in the extension */
			offset = offset + 18 * j ;

			/* skip any Detailed Timing blocks used as descriptors */
			/* Descriptor Blocks begin with 0x00 0x00 0x00...      */
			/* Timing Blocks do not...					           */

#ifdef DEBUG_DESCRIPTOR
			BDBG_MSG(("COMPARE %02X %02X %02X %02X %02X ",
				hHDMI->AttachedEDID.Block[offset],
				hHDMI->AttachedEDID.Block[offset+1],hHDMI->AttachedEDID.Block[offset+2],
				hHDMI->AttachedEDID.Block[offset+3], hHDMI->AttachedEDID.Block[offset+4])) ;
			BDBG_MSG(("COMPARE %02X %02X %02X %02X %02X ",
				hHDMI->AttachedEDID.DescriptorHeader[0],
				hHDMI->AttachedEDID.DescriptorHeader[1], hHDMI->AttachedEDID.DescriptorHeader[2],
				hHDMI->AttachedEDID.DescriptorHeader[3], hHDMI->AttachedEDID.DescriptorHeader[4])) ;

			BDBG_MSG(("Check Timing Block #%d of possible %d ", j+1, MaxDescriptors)) ;
			BDBG_MSG(("DescriptorHeader %02X %02X %02X %02X %02X",
				hHDMI->AttachedEDID.Block[offset],
				hHDMI->AttachedEDID.Block[offset+1], 	hHDMI->AttachedEDID.Block[offset+2],
				hHDMI->AttachedEDID.Block[offset+3], 	hHDMI->AttachedEDID.Block[offset+4])) ;
#endif

			/* skip EDID descriptor blocks */
			if (BKNI_Memcmp(&hHDMI->AttachedEDID.Block[offset], (void *) &hHDMI->AttachedEDID.DescriptorHeader, 3) == 0)
				continue ;


			/* stop searching if the Requested Timing is found */
			if (++NumDetailedTimingsFound == NthTimingRequested)
			{
				BHDM_EDID_ParseDetailedTimingDescriptor(
					&hHDMI->AttachedEDID.Block[offset], pBHDM_EDID_DetailTiming ) ;

				BDBG_MSG(("Detailed Timing #%d found in EDID Extension #%d, Descriptor Block #%d",
					NthTimingRequested, i, j+1)) ;

				rc = BHDM_EDID_P_DetailTiming2VideoFmt(hHDMI, pBHDM_EDID_DetailTiming, BCM_VideoFmt);
				if (rc != BERR_SUCCESS)
					goto BcmSupportedFormatNotFound;

#if BHDM_CONFIG_MHL_SUPPORT
				else
				{
					rc = BHDM_MHL_P_ValidatePreferredFormat(hHDMI,  *BCM_VideoFmt);
					if (rc != BERR_SUCCESS) goto BcmSupportedFormatNotFound;
				}
#endif


				break ;  /* falls to BcmSupportedFormatFound: label */
			}
		}
	}



BcmSupportedFormatFound:
	if (*BCM_VideoFmt == BFMT_VideoFmt_eCUSTOM_1366x768p
	|| *BCM_VideoFmt == BFMT_VideoFmt_eCUSTOM_1366x768p_50Hz
#if BHDM_CONFIG_1366_FORMAT_CHECK
	|| *BCM_VideoFmt == BFMT_VideoFmt_eDVI_1366x768p_60Hz
	|| *BCM_VideoFmt == BFMT_VideoFmt_eDVI_1360x768p_60Hz
#endif
	)
	{
		BFMT_VideoFmt eVideoFmt ;
		const BFMT_VideoInfo *pVideoFormatInfo  ;

#define BHDM_EDID_P_OVERRIDE_FORMATS 2
		BFMT_VideoFmt OverrideFormats[BHDM_EDID_P_OVERRIDE_FORMATS] =
		{
			BFMT_VideoFmt_e720p,
			BFMT_VideoFmt_e720p_50Hz
		} ;

 		for (i = 0 ; i < BHDM_EDID_P_OVERRIDE_FORMATS ; i++)
 		{
			eVideoFmt = OverrideFormats[i] ;

			if (hHDMI->AttachedEDID.BcmSupportedVideoFormats[eVideoFmt])
			{
				pVideoFormatInfo = BFMT_GetVideoFormatInfoPtr(eVideoFmt) ;
				BDBG_MSG(("Overriding DetailTiming #%d (1366x768p/1360x768p) to %s",
					NthTimingRequested, pVideoFormatInfo->pchFormatStr));
				BSTD_UNUSED(pVideoFormatInfo) ; /* supress coverity message for non-debug builds */
				*BCM_VideoFmt = eVideoFmt ;
				goto done ;
			}
 		}

		/*	else fall to BcmSupportedFormatNotFound: label	*/
		goto BcmSupportedFormatNotFound;
	}
	else if (*BCM_VideoFmt == BFMT_VideoFmt_eDVI_640x480p)
	{
		BDBG_MSG(("Preferred format is VGA...")) ;
	}
	else if (BFMT_IS_VESA_MODE(*BCM_VideoFmt))
	{
		*BCM_VideoFmt = BFMT_VideoFmt_eDVI_640x480p ;
		BDBG_WRN(("Requested Detail Timing #%d is a PC format; Override with HDMI supported VGA",
			NthTimingRequested)) ;
	}

	goto done ;


BcmSupportedFormatNotFound:
	rc = BHDM_EDID_DETAILTIMING_NOT_SUPPORTED;
	/* Detailed Timing Request Not Found */
	BDBG_WRN(("Requested Detailed Timing %d NOT SUPPORTED - Detailed Timings found: %d",
		NthTimingRequested, NumDetailedTimingsFound)) ;
	BHDM_EDID_P_SelectAlternateFormat(hHDMI, BCM_VideoFmt) ;


done:
	BDBG_LEAVE(BHDM_EDID_GetDetailTiming) ;

	return rc ;
}


/******************************************************************************
BERR_Code BHDM_EDID_GetVideoDescriptor
Summary: Retrieve Nth Video ID Code retrieved from Video Data Block
*******************************************************************************/
BERR_Code BHDM_EDID_GetVideoDescriptor(
	const BHDM_Handle hHDMI,			 /* [in] HDMI handle */
	uint8_t NthIdCode,           /* [in] number of the Video ID Code to get */
	uint8_t *VideoIdCode,        /* [out] 861B Video ID Code  */
	BFMT_VideoFmt *BCM_VideoFmt, /* [out] associated BFMT */
	uint8_t *NativeFormat        /* Native Monitor Format */
)
{
	BERR_Code rc = BERR_SUCCESS ;

	uint8_t i ;
	uint8_t RxDeviceAttached ;
	BHDM_EDID_P_VideoDescriptor  *pVideoDescriptor ;

	BDBG_ENTER(BHDM_EDID_GetVideoDescriptor) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

#if BDBG_DEBUG_BUILD
	if (hHDMI->DeviceSettings.BypassEDIDChecking)
	{
		goto done ;
	}
#endif

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done;
	}

	if (!hHDMI->AttachedEDID.RxHasHdmiSupport)
	{
		rc = BERR_TRACE(BHDM_EDID_HDMI_NOT_SUPPORTED) ;
		goto done ;
	}

	if (!NthIdCode)
	{
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		BDBG_ERR(("Incorrectly specified Id Code: %d", NthIdCode)) ;
		goto done ;
	}

	i = 1 ;
	for( pVideoDescriptor = BLST_Q_FIRST(&hHDMI->AttachedEDID.VideoDescriptorList);
			pVideoDescriptor ; pVideoDescriptor = BLST_Q_NEXT(pVideoDescriptor, link))
	{
		if (i++ == NthIdCode)
		{
			*VideoIdCode  = (uint8_t) pVideoDescriptor->VideoIdCode ;
			*BCM_VideoFmt = (BFMT_VideoFmt) pVideoDescriptor->eVideoFmt  ;
			*NativeFormat = (uint8_t) pVideoDescriptor->NativeFormat ;

			BDBG_MSG(("VICn: %d; BCM_Fmt: %d; Native Fmt: %d",
				*VideoIdCode, *BCM_VideoFmt, *NativeFormat)) ;
			goto done ;
		}
	}

done:
	BDBG_LEAVE(BHDM_EDID_GetVideoDescriptor) ;

	return rc ;
}
/*
*** BASE_HDMI: Attached Monitor VESA Formats Only
--- BHDM_EDID: 2 VICn: 6; BCM_Fmt: 26; Monitor Native Format: 0
*/

/******************************************************************************
BERR_Code BHDM_EDID_GetDescriptor
Summary: Retrieve a specified EDID descriptor
*******************************************************************************/
BERR_Code BHDM_EDID_GetDescriptor(
   const BHDM_Handle hHDMI, /* [in] HDMI handle */
   BHDM_EDID_Tag tag, /* [in] id of the descriptor tag to retrieve */
   uint8_t *pDescriptorText, /* [out] pointer to memory to hold retrieved tag data */
   uint8_t uiBufSize         /* [in ] mem size in bytes of pDescriptorText */
)
{
	uint8_t TagId ;
	uint8_t i, j ;
	uint8_t offset ;
	uint8_t extensions ;
	uint8_t MaxDescriptors ;
	uint8_t RxDeviceAttached ;

	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_EDID_GetDescriptor) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done;
	}


#if BDBG_DEBUG_BUILD
	if (hHDMI->DeviceSettings.BypassEDIDChecking)
	{
		BKNI_Memcpy(pDescriptorText, EDIDByPassedText,
			BHDM_EDID_MONITOR_DESC_SIZE - BHDM_EDID_DESC_HEADER_LEN) ;
		goto done ;
	}
#endif

	if ((!uiBufSize)
	||  (uiBufSize > BHDM_EDID_MONITOR_DESC_SIZE)
	||  (uiBufSize < BHDM_EDID_MONITOR_DESC_SIZE - BHDM_EDID_DESC_HEADER_LEN))
	{
		BDBG_ERR(("Incorrect Specified Descriptor Length: %d", uiBufSize)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}


	/* check for valid tag */
	switch (tag)
	{
	case BHDM_EDID_Tag_eMONITOR_NAME  :
		/* monitor name should have been read at EDID_Initialize */
		if (hHDMI->AttachedEDID.MonitorName[0] == 0x00)
		{
			BKNI_Memcpy(pDescriptorText, &hHDMI->AttachedEDID.MonitorName,
				BHDM_EDID_MONITOR_DESC_SIZE - BHDM_EDID_DESC_HEADER_LEN) ;

			rc = BERR_SUCCESS ;  /* Descriptor Found and Copied */
			goto done ;
		}
		TagId = BHDM_EDID_TAG_MONITOR_NAME ;
		break ;

	case BHDM_EDID_Tag_eMONITOR_ASCII :
		TagId = BHDM_EDID_TAG_MONITOR_ASCII ;
		break ;

	case BHDM_EDID_Tag_eMONITOR_SN    :
		TagId = BHDM_EDID_TAG_MONITOR_SN ;
		break ;

	default :
		BDBG_ERR(("Invalid Descriptor Tag: %d", tag)) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	/* Insert the Tag we are searching for in the Descriptor Header */
	BKNI_Memset((void *) &hHDMI->AttachedEDID.DescriptorHeader, 0x0, BHDM_EDID_DESC_HEADER_LEN);
	hHDMI->AttachedEDID.DescriptorHeader[BHDM_EDID_DESC_TAG] = TagId ;

	/* read the 1st EDID Block */
	BHDM_CHECK_RC(rc, BHDM_EDID_GetNthBlock(hHDMI,
		0, hHDMI->AttachedEDID.Block, BHDM_EDID_BLOCKSIZE)) ;

	/* Check the four Descriptor Blocks in the initial 128 EDID  bytes */
	for (i = 0 ; i < 4; i++)   /* 1-4 Detailed Timing Descriptor */
	{
		offset = BHDM_EDID_MONITOR_DESC_1 + BHDM_EDID_MONITOR_DESC_SIZE * i ;

		/*
		** Check if we've found the Descriptor tag we're looking for
		** Descriptor Blocks begin with 0x00 0x00 0x00 <tag> 0x00
		** Detailed Timings do not...
		*/
		if (BKNI_Memcmp(&hHDMI->AttachedEDID.Block[offset], (void *) &hHDMI->AttachedEDID.DescriptorHeader,
			BHDM_EDID_DESC_HEADER_LEN) == 0)
		{
			BKNI_Memcpy(pDescriptorText, &hHDMI->AttachedEDID.Block[offset + BHDM_EDID_DESC_DATA],
				BHDM_EDID_MONITOR_DESC_SIZE - BHDM_EDID_DESC_HEADER_LEN) ;

			rc = BERR_SUCCESS ;  /* Descriptor Found and Copied */
			goto done ;
		}
	}

	/* Descriptor Not Found...check extension blocks */
	extensions = hHDMI->AttachedEDID.BasicData.Extensions;
	if (!extensions)
	{
		rc = BERR_TRACE(BHDM_EDID_DESCRIPTOR_NOT_FOUND) ;
		goto done ;
	}


	/* Search EDID Extension blocks for additional descriptors */
	for (i = 1 ; i <= extensions; i++)
	{
		/* read the next 128 Byte EDID block */
		BHDM_CHECK_RC(rc, BHDM_EDID_GetNthBlock(hHDMI,
			i, hHDMI->AttachedEDID.Block, BHDM_EDID_BLOCKSIZE)) ;


		/* check Extension Tag type for Timing Data */
		offset = 0 ;
		if (hHDMI->AttachedEDID.Block[offset] != BHDM_EDID_EXT_TIMING_DATA)
			continue ;

		/* determine the number of Detailed Timing Descripors */
		switch (hHDMI->AttachedEDID.Block[offset+1])
		{
		case 0x01 :  /* Check all blocks regardless of */
			MaxDescriptors = 6 ;   /* (128 - 6) / 18 */
			break ;

		case 0x02 :  /* the version of the EDID Extension */
		case 0x03 :  /* See EA-861 B Spec */
			MaxDescriptors = hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_MONITOR_SUPPORT] ;
			MaxDescriptors = MaxDescriptors & 0x0F ;
			break ;

		default :
			BDBG_WRN(("Timing Extension Version '%d' Not Supported",
				hHDMI->AttachedEDID.Block[offset+1])) ;
			rc = BHDM_EDID_EXT_VERSION_NOT_SUPPORTED ;
			goto done ;
		}


		/* skip start of EDID Extension Block */
		offset = hHDMI->AttachedEDID.Block[offset + 2] ;

		for (j = 0 ; j < MaxDescriptors; j++)
		{
			if (BKNI_Memcmp(&hHDMI->AttachedEDID.Block[offset + BHDM_EDID_MONITOR_DESC_SIZE*j],
				       (void *) &hHDMI->AttachedEDID.DescriptorHeader, BHDM_EDID_DESC_HEADER_LEN) == 0)
			{
				BKNI_Memcpy(pDescriptorText, &hHDMI->AttachedEDID.Block[offset + BHDM_EDID_DESC_DATA],
					BHDM_EDID_MONITOR_DESC_SIZE - BHDM_EDID_DESC_HEADER_LEN) ;
				rc = BERR_SUCCESS ;  /* Descriptor Found and Copied */
				goto done ;
			}
		}
	}

done:
	BDBG_LEAVE(BHDM_EDID_GetDescriptor) ;
	return rc ;
} /* end BHDM_EDID_GetDescriptor */



static BERR_Code BHDM_EDID_P_ParseMonitorRange(const BHDM_Handle hHDMI, uint8_t offset)
{
	BERR_Code rc = BERR_SUCCESS ;

	/* indicate a Monitor Range descriptor has been parsed */
	hHDMI->AttachedEDID.BcmMonitorRangeParsed = true ;

	hHDMI->AttachedEDID.MonitorRange.MinVertical =
		hHDMI->AttachedEDID.Block[offset + BHDM_EDID_DESC_RANGE_MIN_V_RATE] ;

	hHDMI->AttachedEDID.MonitorRange.MaxVertical =
		hHDMI->AttachedEDID.Block[offset +  BHDM_EDID_DESC_RANGE_MAX_V_RATE] ;

	hHDMI->AttachedEDID.MonitorRange.MinHorizontal =
		hHDMI->AttachedEDID.Block[offset +  BHDM_EDID_DESC_RANGE_MIN_H_RATE] ;

	hHDMI->AttachedEDID.MonitorRange.MaxHorizontal =
		hHDMI->AttachedEDID.Block[offset +  BHDM_EDID_DESC_RANGE_MAX_H_RATE] ;

	hHDMI->AttachedEDID.MonitorRange.MaxPixelClock
		= hHDMI->AttachedEDID.Block[offset + BHDM_EDID_DESC_RANGE_MAX_PCLOCK] * 10 ;

	/*
	** check for secondary timing formula but don't decode it...
	** make the bytes available for decoding by caller..
	*/
	hHDMI->AttachedEDID.MonitorRange.SecondaryTiming =
		hHDMI->AttachedEDID.Block[offset + EDID_DESC_TIMING_FORMULA] ;

	BKNI_Memcpy(hHDMI->AttachedEDID.MonitorRange.SecondaryTimingParameters,
		   &hHDMI->AttachedEDID.Block[offset + EDID_DESC_TIMING_FORMULA + 1], 7) ;

#if BDBG_DEBUG_BUILD
	/* Display DEBUG Messages */
	BDBG_MSG(("Min - Max Vertical:   %d - %d Hz",
		hHDMI->AttachedEDID.MonitorRange.MinVertical,
		hHDMI->AttachedEDID.MonitorRange.MaxVertical)) ;

	BDBG_MSG(("Min - Max Horizontal: %d - %d kHz",
		hHDMI->AttachedEDID.MonitorRange.MinHorizontal,
		hHDMI->AttachedEDID.MonitorRange.MaxHorizontal)) ;

	BDBG_MSG(("Max Pixel Clock %d MHz", hHDMI->AttachedEDID.MonitorRange.MaxPixelClock)) ;

#if 0
	/* Display More DEBUG messages for Timing Support */
	switch (hHDMI->AttachedEDID.MonitorRange.SecondaryTiming)
	{
	case 0x00 : BDBG_MSG(("No Timing Formula Support")) ;     break ;
	case 0x02 : BDBG_MSG(("Secondary GTF curve supported")) ; break ;
	default :   BDBG_MSG(("Other secondary timing formula")) ;
	}
#endif
	BDBG_MSG(("[END Monitor Range Descriptor]")) ; /* add a blank line */
#endif

	return rc ;
}



/******************************************************************************
BERR_Code BHDM_EDID_GetMonitorRange
Summary: Retrieve the monitor ranges supported as specified in the EDID
*******************************************************************************/
BERR_Code BHDM_EDID_GetMonitorRange(
   const BHDM_Handle hHDMI,                    /* [in] HDMI handle */
   BHDM_EDID_MonitorRange *pMonitorRange /* [out] pointer to BHDM_EDID_MonitorRange
										    to hold the retrieved data */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t RxDeviceAttached ;

	BDBG_ENTER(BHDM_EDID_GetMonitorRange) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->DeviceSettings.BypassEDIDChecking)
	{
		goto done ;
	}

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	/* If MonitorRange has not been parsed, EDID has not been found, etc. */
	/* create a basic monitor range */
	if (!hHDMI->AttachedEDID.BcmMonitorRangeParsed)
	{
		hHDMI->AttachedEDID.MonitorRange.MinVertical = 60 ;
		hHDMI->AttachedEDID.MonitorRange.MaxVertical = 60 ;

		hHDMI->AttachedEDID.MonitorRange.MinHorizontal  = 26 ;
		hHDMI->AttachedEDID.MonitorRange.MaxHorizontal  = 68 ;

		hHDMI->AttachedEDID.MonitorRange.MaxPixelClock = 80 ;

		BDBG_WRN(("Required EDID Monitor Range Descriptor Not Parsed/Found")) ;
		BDBG_WRN(("Forcing 60Hz monitor type monitor")) ;
		BDBG_WRN(("   Vertical Refresh: %d", hHDMI->AttachedEDID.MonitorRange.MinVertical)) ;
		BDBG_WRN(("   Horizontal Refresh %d - %d ",
			hHDMI->AttachedEDID.MonitorRange.MinHorizontal,
			hHDMI->AttachedEDID.MonitorRange.MaxHorizontal)) ;

		/* set MonitorRangeParsed to true so WRN messages do not repeat */
		hHDMI->AttachedEDID.BcmMonitorRangeParsed = true ;
	}

	BKNI_Memcpy(pMonitorRange, &hHDMI->AttachedEDID.MonitorRange, sizeof(BHDM_EDID_MonitorRange)) ;

done:
	BDBG_LEAVE(BHDM_EDID_GetMonitorRange) ;
	return rc ;
} /* BHDM_EDID_GetMonitorRange */



/******************************************************************************
BERR_Code BHDM_EDID_IsRxDeviceHdmi
Summary: Retrieve the Vendor Specific Data Block from the first Version 3 Timing
Extension in the EDID
*******************************************************************************/
BERR_Code BHDM_EDID_IsRxDeviceHdmi(
   const BHDM_Handle hHDMI,                  /* [in] HDMI handle */
   BHDM_EDID_RxVendorSpecificDB *RxVSDB,  /* [out] pointer to Vendor Specific Data
                                            Block to hold the retrieved data */
   bool *bHdmiDevice
)
{
	uint8_t RxDeviceAttached ;
	BERR_Code rc = BERR_SUCCESS ;


	BDBG_ENTER(BHDM_EDID_IsRxDeviceHdmi) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(RxVSDB, 0, sizeof(BHDM_EDID_RxVendorSpecificDB)) ;


	*bHdmiDevice = false ;  /* assume device is not HDMI */

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

#if BDBG_DEBUG_BUILD
	if (hHDMI->DeviceSettings.BypassEDIDChecking)
	{
		*bHdmiDevice = true ;
		goto done ;
	}
#endif

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {
		BDBG_WRN(("No Valid EDID Found. Default to DVI Device"));
		goto done;
	}

	*bHdmiDevice = hHDMI->AttachedEDID.RxHasHdmiSupport ;
	if (!*bHdmiDevice)
		goto done ;

	/* EDID processed at initialization; copy information */
	BKNI_Memcpy(RxVSDB,  &hHDMI->AttachedEDID.RxVSDB, sizeof(BHDM_EDID_RxVendorSpecificDB)) ;


#if BDBG_DEBUG_BUILD
	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInitialize)
	{
		BDBG_MSG(("HDMI Rx Supported Features (%p):", (void *)RxVSDB)) ;
		BDBG_MSG(("   Underscan:    %s", g_status[RxVSDB->Underscan ? 1 : 0])) ;
		BDBG_MSG(("   Audio Caps:   %s", g_status[RxVSDB->Audio ? 1 : 0])) ;
		BDBG_MSG(("   YCbCr: 4:2:2 %s   4:4:4 %s",
			g_status[RxVSDB->YCbCr422 ? 1 : 0],
			g_status[RxVSDB->YCbCr444 ? 1 : 0])) ;

		BDBG_MSG(("   Native Formats in Descriptors: %d",
			RxVSDB->NativeFormatsInDescriptors)) ;

		BDBG_MSG(("END HDMI Rx Supported Features")) ;
		hHDMI->edidStatus = BHDM_EDID_STATE_eProcessing ;
	}
#endif


done:
	BDBG_LEAVE(BHDM_EDID_IsRxDeviceHdmi) ;
	return 	rc ;
} /* BHDM_EDID_IsRxDeviceHdmi */



/******************************************************************************
BERR_Code BHDM_EDID_CheckRxHdmiAudioSupport
Summary: Check if the input Audio Format is supported by the attached HDMI
Receiver
*******************************************************************************/
BERR_Code BHDM_EDID_CheckRxHdmiAudioSupport(
   const BHDM_Handle hHDMI,                         /* [in] HDMI handle  */
   BAVC_AudioFormat       eAudioFormat,       /* [in] Audio Format */
   BAVC_AudioSamplingRate eAudioSamplingRate, /* [in] Audio Rate to check for */
   BAVC_AudioBits         eAudioBits,         /* [in] Quantization Bits to search for */
   uint16_t               iCompressedBitRate, /* [in] Bit Rate if Compressed Audio */
   uint8_t                *iSupported         /* [out] audio format is supported */
)
{
	BERR_Code rc = BERR_SUCCESS ;

	uint8_t
		i,
		FormatFound,
		EdidAudioSamplingRate,
		EdidAudioBits,
		EdidMaxCompressedBitRate ;

	uint8_t RxDeviceAttached ;

	BDBG_ENTER(BHDM_EDID_CheckRxHdmiAudioSupport) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	*iSupported = 0 ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	/* 1st check for requested format */
	if(!hHDMI->AttachedEDID.BcmSupportedAudioFormats[eAudioFormat].Supported)
		goto done ;

	/**********************************************/
	/* 2nd, Check for requested Audio Sample Rate */
	/* convert the BAVC_AudioSampleRate to EdidAudioSampleRate */

	EdidAudioSamplingRate = BAVC_AudioSamplingRate_eUnknown  ;
	for (i = 0; i < sizeof(EdidAudioSampleRateTable) / sizeof(*EdidAudioSampleRateTable) ; i++)
		if (eAudioSamplingRate == EdidAudioSampleRateTable[i].BcmAudioSampleRate)
		{
			EdidAudioSamplingRate = EdidAudioSampleRateTable[i].EdidAudioSampleRate ;
			break ;
		}

	if (EdidAudioSamplingRate == BAVC_AudioSamplingRate_eUnknown)
		goto done ;

	if (!(EdidAudioSamplingRate
		& hHDMI->AttachedEDID.BcmSupportedAudioFormats[i].ucCeaSampleRates))
		goto done ;


	/********************************************************/
	/* 3rd, Get the number of Audio Bits (quantization) the */
	/* monitor supports for the requested Audio Format etc  */


	EdidAudioBits =
		hHDMI->AttachedEDID.BcmSupportedAudioFormats[i].ucCeaNBits_BitRate ;

	FormatFound = 0 ;
	/* get the number of bits supported by this format */
	if (eAudioFormat != BAVC_AudioFormat_ePCM) /* compressed */
	{										   /*  formats   */
		/* Max Bit Rate = EdidAudioBits * 8 */
		EdidMaxCompressedBitRate = EdidAudioBits * 8 ;
		if (iCompressedBitRate  <= EdidMaxCompressedBitRate)
		{
			BDBG_MSG(("<%.*s> Max Bit Rate Supported: %d",
					BHDM_EDID_DESC_ASCII_STRING_LEN, hHDMI->AttachedEDID.MonitorName,
					EdidMaxCompressedBitRate)) ;
			FormatFound = 1 ;
		}
	} /* if compressed formats */
	else									 /* uncompressed */
	{										 /*     PCM      */
		if (EdidAudioBits & 0x01)           EdidAudioBits = 16 ;
		else if (EdidAudioBits & 0x02)		EdidAudioBits = 20 ;
		else if (EdidAudioBits & 0x04)		EdidAudioBits = 24 ;
		else
		{
			BDBG_ERR(("Unknown Supported Bit Rate")) ;
			rc = BERR_TRACE(BHDM_EDID_HDMI_UNKNOWN_BIT_RATE) ;
			goto done ;
		}

		/* check if the number of bits matches the requested value */
		if (eAudioBits == BAVC_AudioBits_e16)		FormatFound = (EdidAudioBits == 16) ;
		else if (eAudioBits == BAVC_AudioBits_e20)	FormatFound = (EdidAudioBits == 20) ;
		else if (eAudioBits == BAVC_AudioBits_e24)	FormatFound = (EdidAudioBits == 24) ;
	} /* else uncompressed formats */


	if (FormatFound)
		*iSupported = 1 ;

done:
	BDBG_LEAVE(BHDM_EDID_CheckRxHdmiAudioSupport) ;
	return 	rc ;
} /* BHDM_EDID_CheckRxHdmiAudioSupport */



/******************************************************************************
BERR_Code BHDM_EDID_CheckRxHdmiVideoSupport
Summary: Check if the input Video Format is supported by the attached HDMI Receiver
*******************************************************************************/
BERR_Code BHDM_EDID_CheckRxHdmiVideoSupport(
	const BHDM_Handle hHDMI,                    /* [in] HDMI handle  */
	uint16_t           HorizontalPixels,  /* [in] Horiz Active Pixels */
	uint16_t           VerticalPixels,    /* [in] Vertical Active Pixels */
	BAVC_ScanType      eScanType,         /* [in] Progressive, Interlaced */
	BAVC_FrameRateCode eFrameRateCode,    /* [in] Vertical Frequency */
	BFMT_AspectRatio   eAspectRatio,      /* [in] Horiz to Vertical Ratio */
	uint8_t            *pNativeFormat	  /* [out] Requested format is a
										      native format to the monitor */
)
{
	BERR_Code rc = BHDM_EDID_HDMI_VIDEO_FORMAT_UNSUPPORTED ;

	uint8_t
		i, j, k, /* indexes */
		extensions,
		DataOffset,
		DataBlockIndex,
		DataBlockTag,
		DataBlockLength,
		FormatFound,
		NumVideoDescriptors,
		EdidVideoIDCode ;

	uint8_t RxDeviceAttached ;

	BDBG_ENTER(BHDM_EDID_CheckRxHdmiVideoSupport) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;


	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	/* first get the number of extensions in the EDID */
	/* audio support is always in first V3 Timing Extension; never in block 0 */
	extensions = hHDMI->AttachedEDID.BasicData.Extensions ;

	if (!extensions)
	{
		BDBG_WRN(("No EDID Extensions Found... Monitor supports DVI (Video) only")) ;
		rc = BHDM_EDID_HDMI_VIDEO_FORMAT_UNSUPPORTED ;
		goto done ;
	}

	/* check ALL extensions for Version 3 Timing Extensions */
	for (i = 1 ; i <= extensions; i++)
	{
		if (hHDMI->edidStatus != BHDM_EDID_STATE_eOK)
		{
			BHDM_CHECK_RC(rc,
				BHDM_EDID_GetNthBlock(hHDMI, i, (uint8_t *) &hHDMI->AttachedEDID.Block, BHDM_EDID_BLOCKSIZE)) ;
		}

		/* check for Timing Data Extension */
		if (hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_TAG] != BHDM_EDID_EXT_TIMING_DATA)
			continue ;

		/* check for Version 3 Timing Data Extension */
		if (hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_VERSION] != BHDM_EDID_TIMING_VERSION_3)
			continue ;


		/* check if data blocks exist before the 18 Byte Detailed Timing data */
		DataOffset = hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_DATA_OFFSET] ;
		if ((DataOffset == 0)
		||  (DataOffset == 4)) /* no Reserved Data is Available */
		{
			BDBG_WRN(("-----V3 Timing Extension contains no CEA Data Blocks")) ;
			continue ;          /* continue to the next Timing Extension */
		}


		/* set the index to the start of Data Blocks */
		DataBlockIndex = BHDM_EDID_EXT_DATA_BLOCK_COLLECTION ;

		/* scan through the data blocks and retrieve the necessary information */
		while (DataBlockIndex < DataOffset)
		{
			/* get the Data Block type */
			DataBlockTag =
				hHDMI->AttachedEDID.Block[DataBlockIndex] >> 5 ;

			/* get the Data Block length */
			DataBlockLength =
				hHDMI->AttachedEDID.Block[DataBlockIndex] & 0x1F ;

			BDBG_MSG(("[%02X] CEA-861 %s (0x%02x) found; %d bytes",
				hHDMI->AttachedEDID.Block[DataBlockIndex],
				CeaTagName[DataBlockTag], DataBlockTag, DataBlockLength)) ;

			switch (DataBlockTag)
			{

			/* return error on unknown Tags */
			default :
				BDBG_WRN((" ")) ;
				BDBG_WRN(("CEA-861 Data Block Tag Code <%d> is not supported",
					DataBlockTag)) ;
				rc = BHDM_EDID_HDMI_UNKNOWN_CEA_TAG ;
				goto done ;

			/* skip Block Tags that are of no interest to this function */
			case BHDM_EDID_CeaDataBlockTag_eVSDB :      /* Vendor Specific DB */
			case BHDM_EDID_CeaDataBlockTag_eAudioDB :   /* Audio DB */
			case BHDM_EDID_CeaDataBlockTag_eSpeakerDB : /* Speaker Allocation DB */
			case BHDM_EDID_CeaDataBlockTag_eReserved0 :
			case BHDM_EDID_CeaDataBlockTag_eReserved5 :
			case BHDM_EDID_CeaDataBlockTag_eReserved6 :
			case BHDM_EDID_CeaDataBlockTag_eExtendedDB:
				break ;

			case BHDM_EDID_CeaDataBlockTag_eVideoDB :   /* Video DB */
				/* check each video descriptor for requested video support */

				FormatFound = 0 ;
				NumVideoDescriptors = DataBlockLength ;

				/* for each CEA Video ID Code Found */
				for (j = 0 ; j < NumVideoDescriptors && !FormatFound ; j++ )
				{
					/* get the supported Video Code ID; check if a native format */
					EdidVideoIDCode = hHDMI->AttachedEDID.Block[DataBlockIndex + j + 1] ;
					EdidVideoIDCode = EdidVideoIDCode  & 0x7F ;

					*pNativeFormat = EdidVideoIDCode & 0x80 ;

					BDBG_MSG(("Find CEA Video ID Code %02d parameters...",
						EdidVideoIDCode)) ;

					/* search BCM 861-B supported formats for format found in EDID */
					for (k = 0 ; k < BHDM_EDID_P_BCM_VIDEO_FORMATS_MAX ; k++)
					{
						if (EdidVideoIDCode != BHDM_EDID_P_Cea861bFormats[k].CeaVideoCode)
							continue ;

						BDBG_MSG(("Found supported CEA Video ID Code: %02d (%d x %d)",
							EdidVideoIDCode,
							BHDM_EDID_P_Cea861bFormats[k].HorizontalPixels,
							BHDM_EDID_P_Cea861bFormats[k].VerticalPixels)) ;

						/* check if the specified parameters match the requested formats */
						/* 1st, Check if Pixel Format matches */
						if ((HorizontalPixels != BHDM_EDID_P_Cea861bFormats[k].HorizontalPixels)
						||	(VerticalPixels != BHDM_EDID_P_Cea861bFormats[k].VerticalPixels))
							break  ;
						BDBG_MSG(("Pixel Format Match")) ;


						/* 2nd, Check Scan Type (i/p) */
						if (eScanType != BHDM_EDID_P_Cea861bFormats[k].eScanType)
							break  ;
						BDBG_MSG(("Scan Type Match..")) ;


						/* 3rd, Check Vertical Frequency */
						if (eFrameRateCode != BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode)
							break ;
						BDBG_MSG(("Frame Rate Match..")) ;


						/* 4th  Check Aspect Ratio (4:3, 16:9) etc. */
						if (eAspectRatio != BHDM_EDID_P_Cea861bFormats[k].eAspectRatio)
							break ;

						BDBG_MSG(("Requested format is supported..")) ;
						rc = BERR_SUCCESS ;
						goto done ;
					} /* for each BCM Supported CEA Video ID Code */
				} /* for each Supported CEA Video ID Code */
			} /* for each CEA Video ID Code found */


			DataBlockIndex += DataBlockLength + 1;

		} /* while DataBlockIndex < DataOffset */

	} /* for each extension */

	rc = BHDM_EDID_HDMI_VIDEO_FORMAT_UNSUPPORTED ;

done:
	BDBG_LEAVE(BHDM_EDID_CheckRxHdmiVideoSupport) ;
	return 	rc ;
} /* BHDM_EDID_CheckRxHdmiVideoSupport */



/******************************************************************************
BERR_Code BHDM_EDID_VideoFmtSupported
Summary: Check if the requested VideoFmt is supported by the attached HDMI Rx
*******************************************************************************/
BERR_Code BHDM_EDID_VideoFmtSupported(
	const BHDM_Handle hHDMI,        /* [in] HDMI handle  */
	BFMT_VideoFmt eVideoFmt,  /* requested video format */
	uint8_t *Supported        /* [out] true/false Requested format is
							    supported by the monitor */
)
{
	BERR_Code rc = BERR_SUCCESS  ;
	const BFMT_VideoInfo *pVideoFormatInfo ;
	uint8_t RxDeviceAttached ;

	BDBG_ENTER(BHDM_EDID_VideoFmtSupported) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	/* default to format not supported */
	*Supported = 0 ;

	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
		goto done ;

#if BDBG_DEBUG_BUILD
	if (hHDMI->DeviceSettings.BypassEDIDChecking)
	{
		*Supported = 1 ;
		goto done ;
	}
#endif

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto error;
	}

	if ((hHDMI->AttachedEDID.BcmVideoFormatsChecked == 0)
	&&  (hHDMI->AttachedEDID.BcmSupported420VideoFormatsChecked == 0))
	{
		rc = BERR_TRACE(BHDM_EDID_VIDEO_FORMATS_UNAVAILABLE) ;
		goto error ;
	}

	if ((hHDMI->AttachedEDID.BcmSupportedVideoFormats[eVideoFmt])
	||  (hHDMI->AttachedEDID.BcmSupported420VideoFormats[eVideoFmt]))
	{
		*Supported = 1 ;
		goto done;
	}
	else
	{
		/* Warn the BCM supported format is not supported by the Rx only once */
		if (!hHDMI->AttachedEDID.UnsupportedVideoFormatReported[eVideoFmt])
		{
			pVideoFormatInfo = BFMT_GetVideoFormatInfoPtr(eVideoFmt) ;

			BDBG_MSG(("%-30s %4d x %4d %c NOT SUPPORTED by attached <%.13s> receiver",
				pVideoFormatInfo->pchFormatStr,
				pVideoFormatInfo->ulDigitalWidth, pVideoFormatInfo->ulDigitalHeight,
				Mode[pVideoFormatInfo->bInterlaced],
				hHDMI->AttachedEDID.MonitorName)) ;
			BSTD_UNUSED(pVideoFormatInfo) ; /* supress coverity message for non-debug builds */

			hHDMI->AttachedEDID.UnsupportedVideoFormatReported[eVideoFmt] = true ;
		}

		goto done;
	}

error:
	if (eVideoFmt == BFMT_VideoFmt_eDVI_640x480p) {

		BDBG_MSG(("Can't find/read EDID. Assume HDMI receiver supports VGA (640x480p)"));
		*Supported = 1;
		rc = BERR_SUCCESS;
	}

done:
	BDBG_LEAVE(BHDM_EDID_VideoFmtSupported) ;
	return 	rc ;
} /* BHDM_EDID_VideoFmtSupported */


static BERR_Code BHDM_EDID_P_GetVerticalFrequency(
	uint32_t ulVertFreqMask, uint16_t *uiVerticalFrequency)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t i ;

static const struct {
	uint32_t ulVertFreqMask ;
	uint32_t uiVerticalFrequency ;
	} VerticalFrequencies[] =
	{
		{BFMT_VERT_50Hz,        50},
		{BFMT_VERT_59_94Hz,   59},
		{BFMT_VERT_60Hz,        60},

		{BFMT_VERT_100Hz,      100},
		{BFMT_VERT_119_88Hz,   119},
		{BFMT_VERT_120Hz,      120},

	 	{BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,        60},
		{BFMT_VERT_50Hz | BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,        60},

		{BFMT_VERT_66Hz,        66},
		{BFMT_VERT_70Hz,        70},
		{BFMT_VERT_72Hz,        72},
		{BFMT_VERT_75Hz,        75},
		{BFMT_VERT_85Hz,        85},

		{BFMT_VERT_23_976Hz, 23},
		{BFMT_VERT_24Hz,        24},
		{BFMT_VERT_23_976Hz  | BFMT_VERT_24Hz, 24},

		{BFMT_VERT_25Hz,        25},

		{BFMT_VERT_29_97Hz,   29},
		{BFMT_VERT_30Hz,        30},
		{BFMT_VERT_29_97Hz  | BFMT_VERT_30Hz, 30},
	};

	for (i=0; i < sizeof(VerticalFrequencies) / sizeof(*VerticalFrequencies); i++)
	{
		if (ulVertFreqMask & VerticalFrequencies[i].ulVertFreqMask)
		{
			*uiVerticalFrequency = VerticalFrequencies[i].uiVerticalFrequency ;
			return rc ;
		}
	}

	BDBG_WRN(("Unknown Vertical Frequency Mask %#08X; using 60", ulVertFreqMask)) ;
	*uiVerticalFrequency = 60 ;

	return rc ;
}


/******************************************************************************
Summary:
Parse the Established Timings to check for CEA 861 B video formats
*******************************************************************************/
static BERR_Code BHDM_EDID_P_ParseEstablishedTimingFormats(
	const BHDM_Handle hHDMI                  /* [in] HDMI handle  */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t EstablishedTimings ;

#if BDBG_DEBUG_BUILD
 	uint8_t i ;
	 static const char * const EstablishedTimingsIText[8] =
	{
		 "800 x 600 @ 60Hz VESA",
		 "800 x 600 @ 56Hz VESA",
		 "640 x 480 @ 75Hz VESA",
		 "640 x 480 @ 72Hz VESA",
		 "640 x 480 @ 67Hz Apple, Mac II",
		 "640 x 480 @ 60Hz IBM, VGA",
		 "720 x 400 @ 88Hz IBM, XGA2",
		 "720 x 400 @ 70Hz IBM, VGA"
	 } ;

	 static const char * const EstablishedTimingsIIText[8] =
	{
		 "1280 x 1024 @ 75Hz VESA",
		 "1024 x 768 @ 75Hz VESA",
		 "1024 x 768 @ 70Hz VESA",
		 "1024 x 768 @ 60Hz VESA",
		 "1024 x 768 @ 87Hz(I) IBM",
		 "832 x 624 @ 75Hz Apple, Mac II",
		 "800 x 600 @ 75Hz VESA",
		"800 x 600 @ 72Hz VESA"
	 } ;
#endif

	/* assumes current block is 0 */

	/* ESTABLISHED #1 */


	EstablishedTimings = hHDMI->AttachedEDID.Block[BHDM_EDID_ESTABLISHED_TIMINGS1] ;
#if BDBG_DEBUG_BUILD
      if (!EstablishedTimings)
	  	goto CheckEstablishTiming2 ;

	BDBG_MSG(("Monitor Established Timings  I (%#02x)", EstablishedTimings)) ;
	for (i = 0 ;i  < 8; i++)
	{
		if (EstablishedTimings & (1 << i))
			BDBG_MSG(("   %s", EstablishedTimingsIText[i])) ;
	}
#endif

	if (EstablishedTimings & BHDM_EDID_ESTABLISHED_TIMINGS_1_640x480_60HZ)
	{
		BDBG_MSG(("Found BCM Supported 640x480p")) ;
		hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_eDVI_640x480p] = true ;
	}

	if (EstablishedTimings & BHDM_EDID_ESTABLISHED_TIMINGS_1_800x600_60HZ)
	{
		BDBG_MSG(("Found BCM Supported 800x600p")) ;
		hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_eDVI_800x600p] = true ;
	}
	BDBG_MSG((" ")) ;


#if BDBG_DEBUG_BUILD
CheckEstablishTiming2 :
#endif
	/* ESTABLISHED #2 */
	EstablishedTimings = hHDMI->AttachedEDID.Block[BHDM_EDID_ESTABLISHED_TIMINGS2] ;
      if (!EstablishedTimings)
	  	goto done ;
#if BDBG_DEBUG_BUILD
	BDBG_MSG(("Monitor Established Timings II (%#02x)", EstablishedTimings)) ;
	for (i = 0 ;i  < 8; i++)
	{
		if (EstablishedTimings & (1 << i))
			BDBG_MSG(("   %s", EstablishedTimingsIIText[i])) ;
	}
#endif

	if (EstablishedTimings & BHDM_EDID_ESTABLISHED_TIMINGS_2_1024x768_60HZ)
	{
		BDBG_MSG(("Found BCM Supported 1024x768p")) ;
		hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_eDVI_1024x768p] = true ;
	}
	BDBG_MSG((" ")) ;

done:
	/* indicate formats have been checked */
	hHDMI->AttachedEDID.BcmVideoFormatsChecked = 1 ;
	return rc ;
}


/******************************************************************************
Summary:
Return all CEA 861 B video formats supported by the attached monitor as
sepecified in the V3 Timing Ext Video Data Block.
*******************************************************************************/
static BERR_Code BHDM_EDID_P_ParseVideoDB(
	const BHDM_Handle hHDMI,              /* [in] HDMI handle  */
	uint8_t DataBlockIndex,         /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength         /* [in] length (number) of Video ID codes */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	const BFMT_VideoInfo *pVideoFormatInfo ;
	uint8_t
		j, k, l, /* indexes */
		NumVideoDescriptors,
		EdidVideoIDCode,
		NativeFormat,
		LastIdAdded,
		SupportedIdCount ;

	BFMT_VideoFmt eVideoFmt ;
	uint16_t temp ;
	uint16_t tempH;
	BHDM_EDID_P_VideoDescriptor 	*pVideoDescriptor = NULL ;
	bool bVideoIdCodeFound ;


	/* indicate formats have been checked */
	hHDMI->AttachedEDID.BcmVideoFormatsChecked = 1 ;

	/* set the number of video descriptors to look at  */
	NumVideoDescriptors = DataBlockLength ;

	LastIdAdded = 0 ;
	SupportedIdCount = 0 ;

#if BHDM_CONFIG_HDMI_3D_SUPPORT
	if (!hHDMI->AttachedEDID.First16VideoDescriptorsChecked)
		hHDMI->AttachedEDID.First16VideoDescriptorsMask = 0;
#endif

	/* for each CEA Video ID Code Found */
	BDBG_MSG(("<%s> CEA-861 Supported Video Formats (%d):",
		hHDMI->AttachedEDID.MonitorName, NumVideoDescriptors)) ;

	for (j = 0 ; j < NumVideoDescriptors ; j++ )
	{
		/* get the supported Video Code ID; check if a native format */
		EdidVideoIDCode = hHDMI->AttachedEDID.Block[DataBlockIndex + j + 1] ;
		EdidVideoIDCode = EdidVideoIDCode  & 0x7F ;

		/* native formats are 64 and below  */
		if (EdidVideoIDCode <= 64)
			NativeFormat = (EdidVideoIDCode & 0x80) ? 1 : 0 ;
		else /* otherwise use all 8 bits for the ID code */
		{
			NativeFormat = 0 ;
			EdidVideoIDCode = hHDMI->AttachedEDID.Block[DataBlockIndex + j + 1] ;
		}

		bVideoIdCodeFound = false ;

		/* search BCM 861-B supported formats for format found in EDID */
		for (k = 0 ; k < BHDM_EDID_P_BCM_VIDEO_FORMATS_MAX ; k++)
		{
			/* find the retrieved Video ID Code in our table */
			if (EdidVideoIDCode != BHDM_EDID_P_Cea861bFormats[k].CeaVideoCode)
				continue ;

			/* a BCM supported Video ID code has been found; add it to our list */
			/* since this is a loop make sure it is added only once */

			bVideoIdCodeFound = true ;

#if BHDM_CONFIG_HDMI_3D_SUPPORT
			if (!hHDMI->AttachedEDID.First16VideoDescriptorsChecked)
			{
				/* Set the support mask for the first 16 video descriptors for later use on parsing 3D support */
				if (j < 16)
					hHDMI->AttachedEDID.First16VideoDescriptorsMask |= (uint16_t) (1 << j);
			}
#endif

			if (LastIdAdded == EdidVideoIDCode)
				continue ;

			/* adjust pixel repeat formats if necessary */
			if ((BHDM_EDID_P_Cea861bFormats[k].HorizontalPixels == 1440) ||
				(BHDM_EDID_P_Cea861bFormats[k].HorizontalPixels == 2880))
				tempH = 720 ;
			else
				tempH = BHDM_EDID_P_Cea861bFormats[k].HorizontalPixels ;

			/* set all matching BFMT_VideoFmts as supported */
			for (l = 0; l < BFMT_VideoFmt_eMaxCount; l++)
			{
				/* no longer skip the already supported formats; since the corresponding
				   BCM_VideoFmt has to also be added to the VideoIdCode structure */

				eVideoFmt = (BFMT_VideoFmt) l ;

				/* check HDMI formats only */
				if (!BFMT_SUPPORT_HDMI(eVideoFmt))
					continue ;

				pVideoFormatInfo = BFMT_GetVideoFormatInfoPtr(eVideoFmt) ;
				if (!pVideoFormatInfo)
					continue;

				/* Skip 3D formats */
				if (BFMT_IS_3D_MODE(eVideoFmt)) {
					continue;
				}

				/* check if the specified parameters match the requested formats */
				/* 1st, Check if Pixel Format matches */

				if ((pVideoFormatInfo->ulDigitalWidth != tempH)
				||	(pVideoFormatInfo->ulDigitalHeight != BHDM_EDID_P_Cea861bFormats[k].VerticalPixels))
				{
#if 0
					BDBG_MSG(("BFMT%d %d x %d <> %d x %d", l,
						pVideoFormatInfo->ulDigitalWidth, pVideoFormatInfo->ulDigitalHeight,
						tempH, BHDM_EDID_P_Cea861bFormats[k].VerticalPixels)) ;
#endif
					continue ;
				}

				/* 2nd, Check Scan Type (i/p) */
				if (pVideoFormatInfo->bInterlaced !=
						(BAVC_ScanType_eInterlaced == BHDM_EDID_P_Cea861bFormats[k].eScanType))
					continue  ;

				/* use the BFMT frequency parameter to get the Vertical Frequency */
				/* Ignore rc; default of 60Hz will be used for unknown Frequency */
				BHDM_EDID_P_GetVerticalFrequency(
					pVideoFormatInfo->ulVertFreqMask, &temp) ;

#if 0
				BDBG_MSG(("Check if Video Fmt %s (%d)  Frequency: %d matches ",
					pVideoFormatInfo->pchFormatStr, eVideoFmt, temp)) ;
#endif

				/* 3rd check the 861 vertical frequency matches the BCM format  */
				if ((temp == 59) || (temp == 60))
				{
					if ((BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e59_94)
					&&  (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e60))
						continue ;
				}
				else if  ((temp == 50)
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e50))
					continue ;

				if ((temp == 119) || (temp == 120))
				{
					if ((BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e119_88)
					&&  (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e120))
						continue ;
				}
				else if  ((temp == 100)
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e100))
					continue ;

				else if (((temp == 23) || (temp == 24))
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e23_976)
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e24))
					continue;

				else if ((temp == 25)
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e25))
					continue;

				else if (((temp == 29) || (temp == 30))
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e29_97)
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e30))
					continue;

				/* format matches */
				hHDMI->AttachedEDID.BcmSupportedVideoFormats[l] = true ;
				if (LastIdAdded != EdidVideoIDCode)
				{
					BDBG_MSG(("Video ID: %3d (%-27s) : SUPPORTED",
						EdidVideoIDCode, (char *) BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr(EdidVideoIDCode))) ;

					pVideoDescriptor = (BHDM_EDID_P_VideoDescriptor  *)
						BKNI_Malloc(sizeof(BHDM_EDID_P_VideoDescriptor )) ;
					if (pVideoDescriptor == NULL)
					{
						rc = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
						return rc;
					}
					pVideoDescriptor->VideoIdCode  = EdidVideoIDCode ;
					pVideoDescriptor->NativeFormat = NativeFormat ;
					pVideoDescriptor->nthDescriptor = j ;  /* remember position of Video ID Code */

					BLST_Q_INSERT_TAIL(&hHDMI->AttachedEDID.VideoDescriptorList, pVideoDescriptor, link) ;

					/* add Video Id code to supported list for GetVideoInfo */
					hHDMI->AttachedEDID.BcmSupportedVideoIdCodes[SupportedIdCount++] = EdidVideoIDCode ;

					hHDMI->AttachedEDID.NumBcmSupportedVideoDescriptors++;

					pVideoDescriptor->eVideoFmt    = eVideoFmt ;
					LastIdAdded = EdidVideoIDCode ;

				}
			} /* for each BCM BFMT_VideoFmt */
		} /* for each BCM Supported CEA-861-B Video ID Code */

		if (!bVideoIdCodeFound)
		{
			BDBG_MSG(("Video ID: %3d (%-27s) : **NO SUPPORT**",
				EdidVideoIDCode,
				(char *) BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr(EdidVideoIDCode))) ;
		}
	} /* for each CEA-861-B Video Descriptor */


	hHDMI->AttachedEDID.First16VideoDescriptorsChecked = true ;


	return rc ;
}


static BERR_Code BHDM_EDID_P_ParseFormatPreferenceDB(
	const BHDM_Handle hHDMI,             /* [in] HDMI handle  */
	uint8_t DataBlockIndex,        /* [in] start offset of the Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of bytes */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t j, NumSVRs, KDtd ;
	uint8_t SVR ; /* Short Video Reference Code */

	/* indicate formats have been checked */
	hHDMI->AttachedEDID.BcmFormatPreferenceBlockFound = 1 ;


	/* set the number of video descriptors to look at  */
	NumSVRs= DataBlockLength - 1;
	for (j = 0 ; j < NumSVRs ; j++ )
	{
		/* get the supported SVR;  */
		SVR = hHDMI->AttachedEDID.Block[DataBlockIndex + 2 + j ] ;

		if ( (SVR ==    0) || (SVR == 128)
		|| ((SVR >= 145) && (SVR <= 192))
		||  (SVR == 254) || (SVR == 255))
		{
			BDBG_WRN(("Reserved SVR %d found", SVR)) ;
			continue ;
		}

		else if (((SVR >= 1) && (SVR <= 127))
		|| ((SVR >= 193) && (SVR <= 253)))
			/* Interpret as a VIC */
			continue ;

		else if ((SVR >= 129) && (SVR <= 144))
		{
			/* Interpret as the Kth DTD in the EDID, where K = SVR  128 (for K=1 to 16) */
			KDtd = SVR - 128 ;
			BDBG_MSG(("Kth DTD (%d) should  be used a preferred format", KDtd)) ;
		}
		else
		{
			BDBG_ERR(("Unknown SVR ID: %d", SVR)) ;
		}
	}

	BDBG_WRN(("TODO: Video Format Preferences Not Stored")) ;

	return rc ;
}


static BERR_Code BHDM_EDID_P_ParseVideoCapablityDB(
	const BHDM_Handle hHDMI,             /* [in] HDMI handle  */
	uint8_t DataBlockIndex,        /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of Video ID codes */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t VideoCapabilityByte ;

	BSTD_UNUSED(DataBlockLength) ;

	hHDMI->AttachedEDID.VideoCapabilityDB.valid = true ;

	VideoCapabilityByte =
		hHDMI->AttachedEDID.Block[DataBlockIndex + 2] ;

	hHDMI->AttachedEDID.VideoCapabilityDB.eCeBehavior =
		VideoCapabilityByte & 0x03 ;
	hHDMI->AttachedEDID.VideoCapabilityDB.eItBehavior =
		(VideoCapabilityByte & 0x0C) >> 2 ;
	hHDMI->AttachedEDID.VideoCapabilityDB.ePtBehavior =
		(VideoCapabilityByte & 0x30) >> 4 ;
	hHDMI->AttachedEDID.VideoCapabilityDB.bQuantizationSelectatbleRGB =
		VideoCapabilityByte & 0x40 ;
	hHDMI->AttachedEDID.VideoCapabilityDB.bQuantizationSelectatbleYCC =
		VideoCapabilityByte & 0x80 ;

#if BDBG_DEBUG_BUILD
	BDBG_MSG(("Over/Under Scan Behaviors: CE= %d; IT= %d; PT= %d",
		hHDMI->AttachedEDID.VideoCapabilityDB.eCeBehavior,
		hHDMI->AttachedEDID.VideoCapabilityDB.eItBehavior,
		hHDMI->AttachedEDID.VideoCapabilityDB.ePtBehavior)) ;
	BDBG_MSG(("Quantization Selection: RGB= %s  YCC= %s",
		g_status[hHDMI->AttachedEDID.VideoCapabilityDB.bQuantizationSelectatbleRGB],
		g_status[hHDMI->AttachedEDID.VideoCapabilityDB.bQuantizationSelectatbleYCC])) ;

#endif

	return rc ;
}


static BERR_Code BHDM_EDID_P_ParseYCbCr420VideoDB(
	const BHDM_Handle hHDMI,                   /* [in] HDMI handle  */
	uint8_t DataBlockIndex,              /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of Video ID codes */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	const BFMT_VideoInfo *pVideoFormatInfo ;
	uint8_t
		j, k, l, /* indexes */
		NumVideoDescriptors,
		EdidSvdCode ;

	BFMT_VideoFmt eVideoFmt ;
	uint16_t temp ;
	uint16_t tempH;
	bool bVideoIdCodeFound ;


	/* set the number of video descriptors to look at  */
	NumVideoDescriptors = DataBlockLength - 1 ;

	/* for each CEA Video ID Code Found */
	BDBG_MSG(("Number of YCbCr 4:2:0 Video Formats: %d",
		NumVideoDescriptors)) ;

	for (j = 0 ; j < NumVideoDescriptors ; j++ )
	{
		/* get the supported Video Code ID; check if a native format */
		EdidSvdCode = hHDMI->AttachedEDID.Block[DataBlockIndex + 2 + j] ;


		bVideoIdCodeFound = false ;

		/* search BCM 861-B supported formats for format found in EDID */
		for (k = 0 ; k < BHDM_EDID_P_BCM_VIDEO_FORMATS_MAX ; k++)
		{
			/* find the retrieved Video ID Code in our table */
			if (EdidSvdCode != BHDM_EDID_P_Cea861bFormats[k].CeaVideoCode)
				continue ;

			/* a BCM supported Video ID code has been found; add it to our list */
			/* since this is a loop make sure it is added only once */

			BDBG_MSG(("CEA-861 Video ID: %3d (%-27s) : SUPPORTED",
				EdidSvdCode,
				(char *) BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr(EdidSvdCode))) ;

			bVideoIdCodeFound = true ;

			/* adjust pixel repeat formats if necessary */
			if ((BHDM_EDID_P_Cea861bFormats[k].HorizontalPixels == 1440) ||
				(BHDM_EDID_P_Cea861bFormats[k].HorizontalPixels == 2880))
				tempH = 720 ;
			else
				tempH = BHDM_EDID_P_Cea861bFormats[k].HorizontalPixels ;

			/* set all matching BFMT_VideoFmts as supported */
			for (l = 0; l < BFMT_VideoFmt_eMaxCount; l++)
			{
				/* no longer skip the already supported formats; since the corresponding
				   BCM_VideoFmt has to also be added to the VideoIdCode structure */

				eVideoFmt = (BFMT_VideoFmt) l ;

				/* check HDMI formats only */
				if (!BFMT_SUPPORT_HDMI(eVideoFmt))
					continue ;

				pVideoFormatInfo = BFMT_GetVideoFormatInfoPtr(eVideoFmt) ;
				if (!pVideoFormatInfo)
					continue;

				/* Skip 3D formats */
				if (BFMT_IS_3D_MODE(eVideoFmt)) {
					continue;
				}

				/* check if the specified parameters match the requested formats */
				/* 1st, Check if Pixel Format matches */

				if ((pVideoFormatInfo->ulDigitalWidth != tempH)
				||	(pVideoFormatInfo->ulDigitalHeight != BHDM_EDID_P_Cea861bFormats[k].VerticalPixels))
				{
#if 0
					BDBG_MSG(("BFMT%d %d x %d <> %d x %d", l,
						pVideoFormatInfo->ulDigitalWidth, pVideoFormatInfo->ulDigitalHeight,
						tempH, BHDM_EDID_P_Cea861bFormats[k].VerticalPixels)) ;
#endif
					continue ;
				}

				/* 2nd, Check Scan Type (i/p) */
				if (pVideoFormatInfo->bInterlaced !=
						(BAVC_ScanType_eInterlaced == BHDM_EDID_P_Cea861bFormats[k].eScanType))
					continue  ;

				/* use the BFMT frequency parameter to get the Vertical Frequency */
				/* Ignore rc; default of 60Hz will be used for unknown Frequency */
				BHDM_EDID_P_GetVerticalFrequency(
					pVideoFormatInfo->ulVertFreqMask, &temp) ;

				/* 3rd check the 861 vertical frequency matches the BCM format  */
				if ((temp == 59) || (temp == 60))
				{
					if ((BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e59_94)
					&&  (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e60))
						continue ;
				}
				else if  ((temp == 50)
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e50))
					continue ;

				if ((temp == 119) || (temp == 120))
				{
					if ((BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e119_88)
					&&  (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e120))
						continue ;
				}
				else if  ((temp == 100)
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e100))
					continue ;

				else if (((temp == 23) || (temp == 24))
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e23_976)
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e24))
					continue;

				else if ((temp == 25)
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e25))
					continue;

				else if (((temp == 29) || (temp == 30))
				&&   (BHDM_EDID_P_Cea861bFormats[k].eFrameRateCode != BAVC_FrameRateCode_e30))
					continue;


				/* format matches */
				hHDMI->AttachedEDID.BcmSupported420VideoFormats[eVideoFmt] = true ;

			} /* for each BCM BFMT_VideoFmt */
		} /* for each BCM Supported CEA-861-B Video ID Code */

		if (!bVideoIdCodeFound)
		{
			BDBG_MSG(("Found CEA-861 Video ID: %3d (%-27s) : **NO SUPPORT**",
				EdidSvdCode,
				(char *) BAVC_HDMI_AviInfoFrame_VideoIdCodeToStr(EdidSvdCode))) ;
		}

	} /* for each CEA-861-B Video Descriptor */

	hHDMI->AttachedEDID.BcmSupported420VideoFormatsChecked = 1 ;

	return rc ;
}



static BERR_Code BHDM_EDID_P_ParseYCbCr420CapabilityMapDB(
	const BHDM_Handle hHDMI,             /* [in] HDMI handle  */
	uint8_t DataBlockIndex,        /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of Video ID codes */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	BFMT_VideoInfo *pVideoFormatInfo ;

	BHDM_EDID_P_VideoDescriptor 	*pVideoDescriptor = NULL ;
	uint8_t CapabilityBitMask ;
	uint8_t CapabilityByteOffset ;
	uint8_t CapabilityByte ;

	for (pVideoDescriptor = BLST_Q_FIRST(&hHDMI->AttachedEDID.VideoDescriptorList);
			pVideoDescriptor ; pVideoDescriptor = BLST_Q_NEXT(pVideoDescriptor, link))
	{
		CapabilityByteOffset = pVideoDescriptor->nthDescriptor / 8 ;
		CapabilityBitMask = 1 << ((pVideoDescriptor->nthDescriptor)  % 8) ;

		CapabilityByte =
			hHDMI->AttachedEDID.Block[DataBlockIndex + CapabilityByteOffset + 2] ;

		if (CapabilityByteOffset > DataBlockLength - 1)
		{
			BDBG_MSG(("No 420 support specified for additional VICs in VideoDB")) ;
			break ;
		}

		pVideoFormatInfo = (BFMT_VideoInfo *) BFMT_GetVideoFormatInfoPtr(pVideoDescriptor->eVideoFmt) ;

#if 0
		/* debug message for CapabilityByte/Bit Mask */
		BDBG_MSG((" Capability Byte 0x%02x BitMask 0x%02x", CapabilityByte, CapabilityBitMask)) ;
#endif

		/* check if 4:2:0 format is supported */
		if (CapabilityByte & CapabilityBitMask )
		{
			hHDMI->AttachedEDID.BcmSupported420VideoFormats[pVideoDescriptor->eVideoFmt] = true ;
			BDBG_MSG(("   YCbCr 4:2:0 %s ", pVideoFormatInfo->pchFormatStr)) ;
		}
	}

	hHDMI->AttachedEDID.BcmSupported420VideoFormatsChecked = 1 ;

	return rc ;
}


/******************************************************************************
Summary:
Return all CEA 861 B video formats supported by the attached monitor as
sepecified in the V3 Timing Ext Video Data Block.
*******************************************************************************/
static BERR_Code BHDM_EDID_P_ParseAudioDB(
	const BHDM_Handle hHDMI,             /* [in] HDMI handle  */
	uint8_t DataBlockIndex,        /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength        /* [in] length (number) of Video ID codes */
)
{

	BERR_Code rc = BERR_SUCCESS ;

#if BDBG_DEBUG_BUILD
	static const char * const CeaAudioTypeText[] =
	{
		"Reserved",	"PCM",
		"AC3",		"MPEG1",
		"MP3",		"MPEG2",
		"AAC",		"DTS",
		"ATRAC",		"One Bit Audio",
		"DDP",		"DTS-HD",
		"MAT (MLP)",	"DST",
		"WMA Pro",	"Reserved15"
	} ;
#endif


	uint8_t
		i, j, /* indexes */
		NumAudioDescriptors,
		BcmAudioFormat,
		SampleRateFound,
		EdidAudioFormat,
		EdidAudioMaxChannels,
		EdidAudioSampleRate,
		EdidAudioBits ;


	NumAudioDescriptors = DataBlockLength / 3 ;

	/* for each CEA Audio Format Code Found */
	BDBG_MSG(("<%s> CEA-861 Supported audio format:",
		hHDMI->AttachedEDID.MonitorName)) ;

	for (j = 0 ; j < NumAudioDescriptors ; j++)
	{
		EdidAudioFormat        = hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 1] ;
		EdidAudioSampleRate = hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 2] ;
		EdidAudioBits             = hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 3] ;

		EdidAudioFormat = EdidAudioFormat & 0x7F ; /* clear reserved bit */
		EdidAudioMaxChannels = (EdidAudioFormat & 0x07) + 1 ; /* max channels */
		EdidAudioFormat = EdidAudioFormat >> 3 ;

		/************************************************/
		/* 1st check if format is supported  */
		BcmAudioFormat = BAVC_AudioFormat_eMaxCount ;
		for (i = 0; i < sizeof(BHDM_EDID_P_BcmSupportedFormats) / sizeof(*BHDM_EDID_P_BcmSupportedFormats) ; i++)
			if (EdidAudioFormat == BHDM_EDID_P_BcmSupportedFormats[i].EdidAudioFormat)
			{
				BcmAudioFormat = BHDM_EDID_P_BcmSupportedFormats[i].BcmAudioFormat;
				break ;
			}

		if (BcmAudioFormat == BAVC_AudioFormat_eMaxCount)
		{
			BDBG_MSG(("%s - **** NOT Implemented/Supported by BCM%d ; [%02X %02X %02X]",
				CeaAudioTypeText[EdidAudioFormat],  BCHP_CHIP,
				hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 1],
				hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 2],
				hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 3])) ;

			continue ;
		}

		hHDMI->AttachedEDID.BcmAudioFormatsChecked = 1 ;

		/**********************************************/
		/* 2nd, Check for supported Audio Sample Rate */
		EdidAudioSampleRate &= 0x7F ; /* clear reserved bit */

		SampleRateFound = 0 ;
		for (i = 0; i < sizeof(EdidAudioSampleRateTable) / sizeof(*EdidAudioSampleRateTable); i++)
			/* check that at least one sample rate is supported */
			if (EdidAudioSampleRate & EdidAudioSampleRateTable[i].EdidAudioSampleRate)
			{
				SampleRateFound = 1 ;
				break ;
			}

		if (!SampleRateFound)
			continue ;

		/********************************************************/
		/* 3rd, Determine the number of Audio Bits (quantization) the */
		/* monitor supports for the requested Audio Format etc  */

		/* get the number of bits supported by this format */
		if (EdidAudioFormat != BHDM_EDID_P_AudioFormat_ePCM) /* compressed */
		{										           /*  formats   */
			/* Max Bit Rate = EdidAudioBits * 8 */
			EdidAudioBits = EdidAudioBits /** 8*/ ;

			/* display debug information */
			BDBG_MSG(("Found BCM supported CEA-861 Audio: %s - %d Ch [%d max bit rate] ; [%02X %02X %02X]",
				CeaAudioTypeText[EdidAudioFormat],
				EdidAudioMaxChannels, EdidAudioBits *8,
				hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 1],
				hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 2],
				hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 3])) ;

#if BDBG_DEBUG_BUILD
			/* show the supported sample rates */
			for (i = 0; i < sizeof(EdidAudioSampleRateTable) / sizeof(*EdidAudioSampleRateTable); i++)
				/* check that at least one sample rate is supported */
				if (EdidAudioSampleRate & EdidAudioSampleRateTable[i].EdidAudioSampleRate)
				{
					BDBG_MSG(("   Sample Rate %s", CeaAudioSampleRateTypeText[i])) ;
				}
#endif
		} /* END if compressed formats */
		else									 /* uncompressed */
		{										 /*     PCM      */
			uint8_t uiAudioSampleSize ;

			if (EdidAudioBits & 0x04)       uiAudioSampleSize = 24 ;
			else if (EdidAudioBits & 0x02)	uiAudioSampleSize = 20 ;
			else if (EdidAudioBits & 0x01)	uiAudioSampleSize = 16 ;
			else
			{
				BDBG_WRN(("Unknown/Un-Supported Bit Rate")) ;
				rc = BHDM_EDID_HDMI_UNKNOWN_BIT_RATE ;
				continue ;
			}

			/* display debug information */
			BDBG_MSG(("Found BCM supported CEA-861 Audio: %s - %d Ch [up to %d bits] ; [%02X %02X %02X]",
				CeaAudioTypeText[EdidAudioFormat],
				EdidAudioMaxChannels, uiAudioSampleSize,
				hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 1],
				hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 2],
				hHDMI->AttachedEDID.Block[DataBlockIndex+ j*3 + 3])) ;

#if BDBG_DEBUG_BUILD
			/* show the supported sample rates */
			for (i = 0; i < sizeof(EdidAudioSampleRateTable) / sizeof(*EdidAudioSampleRateTable); i++)
				/* check that at least one sample rate is supported */
				if (EdidAudioSampleRate & EdidAudioSampleRateTable[i].EdidAudioSampleRate)
				{
					BDBG_MSG(("   Sample Rate %s", CeaAudioSampleRateTypeText[i])) ;
				}
#endif
		} /* END ELSE uncompressed formats */

		/* update audio supported information */

		hHDMI->AttachedEDID.BcmSupportedAudioFormats[BcmAudioFormat].Supported = 1 ;

		/* update supported format only if descriptor contains a larger number of Audio Channels */
		if (EdidAudioMaxChannels > hHDMI->AttachedEDID.BcmSupportedAudioFormats[BcmAudioFormat].AudioChannels )
		{
			hHDMI->AttachedEDID.BcmSupportedAudioFormats[BcmAudioFormat].AudioChannels
				= EdidAudioMaxChannels ;
		}

		hHDMI->AttachedEDID.BcmSupportedAudioFormats[BcmAudioFormat].ucCeaSampleRates
			= EdidAudioSampleRate ;

		hHDMI->AttachedEDID.BcmSupportedAudioFormats[BcmAudioFormat].ucCeaNBits_BitRate
			= EdidAudioBits ;
	} /* for each CEA Audio Format Code Found in EDID */

	return rc ;
}


/******************************************************************************
Summary:
Set all BCM_VideoFmts that match eVideoFmt as being supported.
*******************************************************************************/
static void BHDM_EDID_P_SetSupportedMatchingFmts(
	const BHDM_Handle hHDMI, BFMT_VideoFmt eVideoFmt)
{
	uint8_t i ;

	uint16_t
		uiVerticalFrequency ,
		uiSupportedVerticalFrequency ;

	BFMT_VideoInfo
		*pVideoFormatInfo,
		*pSupportedVideoFormatInfo ;

	pSupportedVideoFormatInfo = (BFMT_VideoInfo *) BFMT_GetVideoFormatInfoPtr(eVideoFmt) ;
	if (!pSupportedVideoFormatInfo)
	{
		/* BFMT_VideoFmt_eCustom2 is used. BFMT does not have any
				information on this format so it returns NULL.
				HDMI does not support this custom format */

		BDBG_WRN(("No support for custom user defined format"));
		return;
	}

	BHDM_EDID_P_GetVerticalFrequency(pSupportedVideoFormatInfo->ulVertFreqMask,
			 &uiSupportedVerticalFrequency) ;

	hHDMI->AttachedEDID.BcmSupportedVideoFormats[eVideoFmt] = true ;

	for (i = 0 ; i < BFMT_VideoFmt_eMaxCount ; i++)
	{
		/* check HDMI formats only */
		if (!BFMT_SUPPORT_HDMI((BFMT_VideoFmt) i))
			continue ;

		/* skip if already listed as supported */
		if (hHDMI->AttachedEDID.BcmSupportedVideoFormats[i])
			continue ;

		pVideoFormatInfo = (BFMT_VideoInfo *) BFMT_GetVideoFormatInfoPtr((BFMT_VideoFmt) i) ;
		if (!pVideoFormatInfo)
			continue;

		/* 1st, Check if Pixel Format matches */
		/* check the width/height, and interlace parameters */
		if ((pVideoFormatInfo->ulDigitalWidth != pSupportedVideoFormatInfo->ulDigitalWidth)
		||  (pVideoFormatInfo->ulDigitalHeight != pSupportedVideoFormatInfo->ulDigitalHeight)
		||  (pVideoFormatInfo->bInterlaced != pSupportedVideoFormatInfo->bInterlaced))
			continue ;

		/* 2nd make sure the Video Format's frequency is supported by the monitor */
		BHDM_EDID_P_GetVerticalFrequency(
			pVideoFormatInfo->ulVertFreqMask, &uiVerticalFrequency) ;

		/* if no Monitor Range Has been found; the format cannot be confirmed as supported */
		if (!hHDMI->AttachedEDID.BcmMonitorRangeParsed)
		{
			BDBG_WRN(("No Monitor Range Descriptor found to confirm support for <%s>",
				pVideoFormatInfo->pchFormatStr)) ;
			continue ;
		}

		/* if the frequency does not fall into the range the monitor supports */
		/* the format cannot be supported */
		if  ((uiVerticalFrequency < hHDMI->AttachedEDID.MonitorRange.MinVertical)
		||   (uiVerticalFrequency > hHDMI->AttachedEDID.MonitorRange.MaxVertical))
			continue ;

		/* Finally, we don't want to falsely conclude that
		   25Hz, 30Hz, and 50Hz are supported if 24Hz is
		   supported (by virtue of the above range check).
		   This is only pertinent for 720p and 1080i,p. */
		if ((pVideoFormatInfo->ulDigitalWidth == BFMT_1080I_WIDTH)
		|| (pVideoFormatInfo->ulDigitalWidth == BFMT_1080P_WIDTH)
		|| (pVideoFormatInfo->ulDigitalWidth == BFMT_720P_WIDTH)
		|| (pVideoFormatInfo->ulDigitalWidth == 3840)
		|| (pVideoFormatInfo->ulDigitalWidth == 4096))
		{
			if (uiVerticalFrequency != uiSupportedVerticalFrequency)
			        continue;
 		}

		/* set as supported */
		hHDMI->AttachedEDID.BcmSupportedVideoFormats[i] = true ;
	}

}  /* BHDM_EDID_P_SetSupportedMatchingFmts */



/******************************************************************************
Summary:
Return all YCbCr 420 formats supported by the attached monitor.
*******************************************************************************/
BERR_Code BHDM_EDID_GetSupported420VideoFormats(
	const BHDM_Handle hHDMI,                   /* [in] HDMI handle  */
	bool *VideoFormats                   /* [out] supported true/false */
)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_EDID_GetSupported420VideoFormats) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {
		rc = BERR_TRACE( BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	if ((hHDMI->AttachedEDID.BcmSupported420VideoFormatsChecked == 0)
	&& (hHDMI->AttachedEDID.BcmSupported420VideoFormatsReported == 0))
	{
		BDBG_MSG(("No 420 formats were found in the EDID")) ;
		hHDMI->AttachedEDID.BcmSupported420VideoFormatsReported = 1 ;
	}
	BKNI_Memcpy(VideoFormats, &hHDMI->AttachedEDID.BcmSupported420VideoFormats,
		sizeof(hHDMI->AttachedEDID.BcmSupported420VideoFormats)) ;

done:
	BDBG_LEAVE(BHDM_EDID_GetSupported420VideoFormats) ;
	return rc ;

} /* BHDM_EDID_GetSupported420VideoFormats */

/******************************************************************************
Summary:
Return all video formats supported by the attached monitor.
*******************************************************************************/
BERR_Code BHDM_EDID_GetSupportedVideoFormats(
	const BHDM_Handle hHDMI,                   /* [in] HDMI handle  */
	bool *VideoFormats                   /* [out] supported true/false */
)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_EDID_GetSupportedVideoFormats) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	if (hHDMI->AttachedEDID.BcmVideoFormatsChecked == 0)
	{
		rc = BERR_TRACE(BHDM_EDID_VIDEO_FORMATS_UNAVAILABLE) ;
		goto done ;
	}

#if BHDM_CONFIG_MHL_SUPPORT
	BHDM_MHL_P_GetSupportedVideoFormats(hHDMI, VideoFormats);
#else
	BKNI_Memcpy(VideoFormats, &hHDMI->AttachedEDID.BcmSupportedVideoFormats,
		sizeof(hHDMI->AttachedEDID.BcmSupportedVideoFormats)) ;
#endif

done:
	BDBG_LEAVE(BHDM_EDID_GetSupportedVideoFormats) ;
	return rc ;

} /* BHDM_EDID_GetSupportedVideoFormats */


#if BHDM_CONFIG_HDMI_3D_SUPPORT
/******************************************************************************
Summary:
Return all video CEA-861 Viideo ID codes supported by the attached monitor.
See BHDM_EDID_GetSupportedVideoFormats
*******************************************************************************/
BERR_Code BHDM_EDID_GetSupportedVideoInfo(
	const BHDM_Handle hHDMI,					 /* [in] HDMI handle  */
	BHDM_EDID_VideoDescriptorInfo *stSupportedVideoInfo       /* [out] supported true/false */
)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_EDID_GetSupportedVideoInfo) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done;
	}

	if (hHDMI->AttachedEDID.BcmVideoFormatsChecked == 0) {
		rc = BERR_TRACE(BHDM_EDID_VIDEO_FORMATS_UNAVAILABLE) ;
		goto done;
	}

	/* zero out return structure */
	BKNI_Memset(stSupportedVideoInfo, 0, sizeof(BHDM_EDID_VideoDescriptorInfo));


	stSupportedVideoInfo->numDescriptors
		= hHDMI->AttachedEDID.NumBcmSupportedVideoDescriptors ;

	BDBG_MSG(("Num Supported Descriptors: %d",
		hHDMI->AttachedEDID.NumBcmSupportedVideoDescriptors)) ;

	BKNI_Memcpy(
		stSupportedVideoInfo->VideoIDCode, hHDMI->AttachedEDID.BcmSupportedVideoIdCodes,
		hHDMI->AttachedEDID.NumBcmSupportedVideoDescriptors) ;

done:

	BDBG_LEAVE(BHDM_EDID_GetSupportedVideoInfo) ;
	return rc ;

}
#endif


/******************************************************************************
Summary:
Return all audio formats supported by the attached monitor.
*******************************************************************************/
BERR_Code BHDM_EDID_GetSupportedAudioFormats(
	const BHDM_Handle hHDMI,                          /* [in] HDMI handle  */
	BHDM_EDID_AudioDescriptor *BcmAudioFormats  /* [out] supported formats */
)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_ENTER(BHDM_EDID_GetSupportedAudioFormats) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	BKNI_Memset(BcmAudioFormats, 0, sizeof(hHDMI->AttachedEDID.BcmSupportedAudioFormats)) ;

	if (hHDMI->AttachedEDID.BcmAudioFormatsChecked == 0)
	{
		rc = BERR_TRACE(BHDM_EDID_AUDIO_FORMATS_UNAVAILABLE) ;
		goto done ;
	}

	BKNI_Memcpy(BcmAudioFormats, &hHDMI->AttachedEDID.BcmSupportedAudioFormats,
		sizeof(hHDMI->AttachedEDID.BcmSupportedAudioFormats)) ;

done:
	BDBG_LEAVE(BHDM_EDID_GetSupportedAudioFormats) ;
	return rc ;

} /* BHDM_EDID_GetSupportedAudioFormats */

static void BHDM_EDID_P_ParseColorimetryDB(const BHDM_Handle hHDMI, uint8_t DataBlockIndex)
{
	uint8_t ColorimetrySupportByte ;
	uint8_t MetaDataSupportByte ;

	/* Check for Colorimetry Support */
	ColorimetrySupportByte = hHDMI->AttachedEDID.Block[DataBlockIndex+2] ;
	hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_exvYCC601]
		= ColorimetrySupportByte & 0x01 ;
	hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_exvYCC709]
		= ColorimetrySupportByte & 0x02 ;
	hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_esYCC601]
		= ColorimetrySupportByte & 0x04 ;
	hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eAdobeYCC601]
		= ColorimetrySupportByte & 0x08 ;

	hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eAdobeRGB]
		= ColorimetrySupportByte & 0x10 ;
	hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020cYCC]
		= ColorimetrySupportByte & 0x20 ;
	hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020YCC]
		= ColorimetrySupportByte & 0x40 ;
	hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020RGB]
		= ColorimetrySupportByte & 0x80 ;


	/* Check for metadata support in colorimetry block	*/
	MetaDataSupportByte = hHDMI->AttachedEDID.Block[DataBlockIndex+3] ;
	hHDMI->AttachedEDID.ColorimetryDB.bMetadataProfile[BHDM_EDID_ColorimetryDbMetadataProfile_eMD0] =
		MetaDataSupportByte & 0x01 ;

	hHDMI->AttachedEDID.ColorimetryDB.bMetadataProfile[BHDM_EDID_ColorimetryDbMetadataProfile_eMD1] =
		MetaDataSupportByte & 0x02 ;

	hHDMI->AttachedEDID.ColorimetryDB.bMetadataProfile[BHDM_EDID_ColorimetryDbMetadataProfile_eMD2] =
		MetaDataSupportByte & 0x04 ;

	hHDMI->AttachedEDID.ColorimetryDB.bMetadataProfile[BHDM_EDID_ColorimetryDbMetadataProfile_eMD3] =
		MetaDataSupportByte & 0x08 ;

#if BDBG_DEBUG_BUILD
	BDBG_MSG((" ")) ;
	BDBG_MSG(("Colorimetry Data Block[3]= 0x%02X", ColorimetrySupportByte)) ;
	BDBG_MSG(("Supported Standards")) ;
	BDBG_MSG(("   xvYCC601 = %3s,  xvYCC709 =  %3s,  sYCC601 =  %s",
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_exvYCC601]
			? 1 : 0],
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_exvYCC709]
			? 1 : 0],
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_esYCC601]
			? 1 : 0])) ;

	BDBG_MSG(("   Adobe:  YCC601 = %3s;  RGB = %3s;",
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eAdobeYCC601]
			? 1 : 0],
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eAdobeRGB]
			? 1 : 0])) ;

	BDBG_MSG(("   BT2020: cYCC   = %3s;  YCC = %3s;  RGB = %3s",
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020cYCC]
			? 1 : 0],
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020YCC]
			? 1 : 0],
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020RGB]
			? 1 : 0])) ;

	BDBG_MSG((" ")) ;
	BDBG_MSG(("Colorimetry Data Block[4] = 0x%02X", MetaDataSupportByte)) ;
	BDBG_MSG(("Supported Gamut Metadata profiles")) ;
	BDBG_MSG(("   MD0: %s;  MD1: %s;  MD2: %s;  MD3: %s",
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bMetadataProfile[BHDM_EDID_ColorimetryDbMetadataProfile_eMD0] ? 1 : 0],
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bMetadataProfile[BHDM_EDID_ColorimetryDbMetadataProfile_eMD1] ? 1 : 0],
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bMetadataProfile[BHDM_EDID_ColorimetryDbMetadataProfile_eMD2] ? 1 : 0],
		g_status[hHDMI->AttachedEDID.ColorimetryDB.bMetadataProfile[BHDM_EDID_ColorimetryDbMetadataProfile_eMD3] ? 1 : 0])) ;
#endif
}

static void BHDM_EDID_P_ParseHdmi_1_4VSDB(const BHDM_Handle hHDMI, uint8_t DataBlockIndex, uint8_t DataBlockLength)
{
	uint8_t offset ;
	uint8_t offsetByte;

	BKNI_Memset(&hHDMI->AttachedEDID.RxVSDB, 0, sizeof(BHDM_EDID_RxVendorSpecificDB)) ;

	/* existence of HDMI VSDB indicates HDMI Support */
	hHDMI->AttachedEDID.RxVSDB.valid = true ;
	hHDMI->AttachedEDID.RxHasHdmiSupport = true ;
#if BHDM_CONFIG_HDMI_3D_SUPPORT
	/* indicate 3D Formats checked even if we do not proces due to DVI device (no HDMI) or no 3D Support */
	hHDMI->AttachedEDID.Bcm3DFormatsChecked = true;
#endif

	/* get the components of My Source Physical Address */
	hHDMI->AttachedEDID.RxVSDB.PhysAddr_A = (hHDMI->AttachedEDID.Block[DataBlockIndex+4] & 0xF0) >> 4 ;
	hHDMI->AttachedEDID.RxVSDB.PhysAddr_B = (hHDMI->AttachedEDID.Block[DataBlockIndex+4] & 0x0F) ;
	hHDMI->AttachedEDID.RxVSDB.PhysAddr_C = (hHDMI->AttachedEDID.Block[DataBlockIndex+5] & 0xF0) >> 4 ;
	hHDMI->AttachedEDID.RxVSDB.PhysAddr_D = (hHDMI->AttachedEDID.Block[DataBlockIndex+5] & 0x0F) ;
	BDBG_MSG(("my_address (CEC physical) = (%d.%d.%d.%d)",
		hHDMI->AttachedEDID.RxVSDB.PhysAddr_A, hHDMI->AttachedEDID.RxVSDB.PhysAddr_B,
		hHDMI->AttachedEDID.RxVSDB.PhysAddr_C, hHDMI->AttachedEDID.RxVSDB.PhysAddr_D)) ;

#if BHDM_CONFIG_CEC_LEGACY_SUPPORT && BHDM_CEC_SUPPORT
	BKNI_Memcpy(hHDMI->cecConfiguration.CecPhysicalAddr,
		&hHDMI->AttachedEDID.Block[DataBlockIndex+4], 2) ;
#endif

	/* DataBlockLength is N (which is 1 less than actual length). */
	if (DataBlockLength >= 6)
	{
		/* for HDMI 1.1 Check if Rx supports ACP or ISRC packets */
		hHDMI->AttachedEDID.RxVSDB.SupportsAI = (hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x80) >> 7 ;
		BDBG_MSG(("Supports AI = %d", hHDMI->AttachedEDID.RxVSDB.SupportsAI)) ;

		/* retrieve support for Deep Color modes	*/
		hHDMI->AttachedEDID.RxVSDB.DeepColor_48bit = (hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x40) >> 6;
		hHDMI->AttachedEDID.RxVSDB.DeepColor_36bit = (hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x20) >> 5;
		hHDMI->AttachedEDID.RxVSDB.DeepColor_30bit = (hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x10) >> 4;
		hHDMI->AttachedEDID.RxVSDB.DeepColor_Y444 = (hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x08) >> 3;
		hHDMI->AttachedEDID.RxVSDB.DVI_Dual = (hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x01);
	}
	else
	{
		hHDMI->AttachedEDID.RxVSDB.SupportsAI = false;
		hHDMI->AttachedEDID.RxVSDB.DeepColor_48bit = false;
		hHDMI->AttachedEDID.RxVSDB.DeepColor_36bit = false;
		hHDMI->AttachedEDID.RxVSDB.DeepColor_30bit = false;
		hHDMI->AttachedEDID.RxVSDB.DeepColor_Y444 = false;
		hHDMI->AttachedEDID.RxVSDB.DVI_Dual = false;
	}

	if (DataBlockLength >= 7)
	{
		/* get the maximum TMDS clock rate supported	*/
		hHDMI->AttachedEDID.RxVSDB.Max_TMDS_Clock_Rate =
			hHDMI->AttachedEDID.Block[DataBlockIndex+7] * 5;

		if (hHDMI->AttachedEDID.RxVSDB.Max_TMDS_Clock_Rate > 300 )
		{
			BDBG_WRN(("HDMI Vendor Specific DB Max Tmds Clock Rate of %d exceeds rate of 300",
				hHDMI->AttachedEDID.RxVSDB.Max_TMDS_Clock_Rate)) ;
			BDBG_WRN(("Adjust incorrectly specified Max TMDS Clock Rate")) ;
			hHDMI->AttachedEDID.RxVSDB.Max_TMDS_Clock_Rate = 300 ;
		}
	}

	/* determine present/non present fields */
	if (DataBlockLength >= 8)
	{
		offsetByte = hHDMI->AttachedEDID.Block[DataBlockIndex+8] ;

		/* get lipsync-related fields	*/
		hHDMI->AttachedEDID.RxVSDB.Latency_Fields_Present =
			offsetByte >> 7 ;

		hHDMI->AttachedEDID.RxVSDB.Interlaced_Latency_Fields_Present =
			(offsetByte & 0x40) >> 6 ;

#if BHDM_CONFIG_HDMI_3D_SUPPORT
		/* HDMI Video Present */
		hHDMI->AttachedEDID.RxVSDB.HDMI_Video_Present=
			(offsetByte & 0x20) >> 5 ;


		/* get Content Type related fields */
		hHDMI->AttachedEDID.RxVSDB.SupportedContentTypeGraphicsText =
			offsetByte & 0x01 ;

		hHDMI->AttachedEDID.RxVSDB.SupportedContentTypePhoto =
			offsetByte & 0x02 ;

		hHDMI->AttachedEDID.RxVSDB.SupportedContentTypeCinema =
			offsetByte & 0x04 ;

		hHDMI->AttachedEDID.RxVSDB.SupportedContentTypeGame =
			offsetByte & 0x08 ;
#endif
	}


	offset = 9 ;  /* earliest byte where optional data can be located */
	if (offset > DataBlockLength)  /* no more optional data */
		goto done ;

	/* retrieve Video and Audio Latency if available	*/
	if (hHDMI->AttachedEDID.RxVSDB.Latency_Fields_Present)
	{
		hHDMI->AttachedEDID.RxVSDB.Video_Latency =
				hHDMI->AttachedEDID.Block[DataBlockIndex + offset] ;
		offset++  ;

		hHDMI->AttachedEDID.RxVSDB.Audio_Latency =
				hHDMI->AttachedEDID.Block[DataBlockIndex + offset] ;
		offset++ ;
	}
	else
	{
		/* force Interlaced Latency Fields Present to false when no Progressive Latency fields */
		/* cannot have Interlaced Latency Fields without Progressive Latency Fields */
		hHDMI->AttachedEDID.RxVSDB.Interlaced_Latency_Fields_Present = false ;
	}

	/* retrieve Video and Audio Latency for interlaced formats if available	*/
	if (hHDMI->AttachedEDID.RxVSDB.Interlaced_Latency_Fields_Present)
	{
		hHDMI->AttachedEDID.RxVSDB.Interlaced_Video_Latency =
				hHDMI->AttachedEDID.Block[DataBlockIndex + offset] ;
		offset++ ;

		hHDMI->AttachedEDID.RxVSDB.Interlaced_Audio_Latency =
				hHDMI->AttachedEDID.Block[DataBlockIndex + offset] ;
		offset++ ;
	}

	/* AV and interlaced AV latencies have been retrieved */
	if (offset > DataBlockLength)
		goto done ;  /* no more optional data */


#if BHDM_CONFIG_HDMI_3D_SUPPORT
	/* Now check for HDMI VICs and 3D support info */
	BHDM_EDID_P_ParseVSDB3D(hHDMI, DataBlockIndex, &offset, DataBlockLength);
#endif


done:
	if ((DataBlockLength >= 7)
	&& (hHDMI->AttachedEDID.RxVSDB.DeepColor_48bit
	||  hHDMI->AttachedEDID.RxVSDB.DeepColor_36bit
	||  hHDMI->AttachedEDID.RxVSDB.DeepColor_30bit
	||  hHDMI->AttachedEDID.RxVSDB.DeepColor_Y444
	||  hHDMI->AttachedEDID.RxVSDB.DVI_Dual))
	{
		BDBG_MSG((" ")) ;
		BDBG_MSG(("HDMI 1.3 Features")) ;
		BDBG_MSG(("   Deep Color Support")) ;
		BDBG_MSG(("      16bit: %s;   12bit: %s;   10bit: %s;  Y444: %s",
			g_status[hHDMI->AttachedEDID.RxVSDB.DeepColor_48bit ? 1 : 0],
			g_status[hHDMI->AttachedEDID.RxVSDB.DeepColor_36bit ? 1 : 0],
			g_status[hHDMI->AttachedEDID.RxVSDB.DeepColor_30bit ? 1 : 0],
			g_status[hHDMI->AttachedEDID.RxVSDB.DeepColor_Y444 ? 1 : 0])) ;

		BDBG_MSG(("   DVI Dual Link Support: %s",
			g_status[hHDMI->AttachedEDID.RxVSDB.DVI_Dual ? 1 : 0]));

		BDBG_MSG(("   Max TMDS Clock Rate: %d",
			hHDMI->AttachedEDID.RxVSDB.Max_TMDS_Clock_Rate));
	}

	BDBG_MSG(("   Video Latency Data Present:            %s",
		g_status[hHDMI->AttachedEDID.RxVSDB.Latency_Fields_Present ? 1 : 0])) ;
	if (hHDMI->AttachedEDID.RxVSDB.Latency_Fields_Present)
	{
		BDBG_MSG(("      Video: %d, Audio: %d",
			hHDMI->AttachedEDID.RxVSDB.Video_Latency,
			hHDMI->AttachedEDID.RxVSDB.Audio_Latency)) ;
	}

	BDBG_MSG(("   Interlaced Video Latency Data Present: %s",
		g_status[hHDMI->AttachedEDID.RxVSDB.Interlaced_Latency_Fields_Present ? 1 : 0])) ;
	if (hHDMI->AttachedEDID.RxVSDB.Interlaced_Latency_Fields_Present)
	{
		BDBG_MSG(("      Video: %d, Audio: %d",
			hHDMI->AttachedEDID.RxVSDB.Interlaced_Video_Latency,
			hHDMI->AttachedEDID.RxVSDB.Interlaced_Audio_Latency));
	}

	BDBG_MSG(("   HDMI Video Present: %s",
		g_status[hHDMI->AttachedEDID.RxVSDB.HDMI_Video_Present ? 1 : 0])) ;

#if BHDM_CONFIG_HDMI_3D_SUPPORT
	if (hHDMI->AttachedEDID.RxVSDB.HDMI_Video_Present)
	{
		BDBG_MSG((" ")) ;
		BDBG_MSG(("HDMI 1.4 Features")) ;
		BDBG_MSG(("   Content Types Supported")) ;
		BDBG_MSG(("      Graphics (Text): %s  Cinema: %s  Photo: %s  Game: %s",
			g_status[hHDMI->AttachedEDID.RxVSDB.SupportedContentTypeGraphicsText ? 1 : 0],
			g_status[hHDMI->AttachedEDID.RxVSDB.SupportedContentTypeCinema ? 1 : 0],
			g_status[hHDMI->AttachedEDID.RxVSDB.SupportedContentTypePhoto ? 1 : 0],
			g_status[hHDMI->AttachedEDID.RxVSDB.SupportedContentTypeGame ? 1 : 0])) ;

		BDBG_MSG(("   HDMI 3D Present: %s;   Multi 3D Present (Optional): %#x",
			g_status[hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Present ? 1 : 0],
			hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Multi_Present)) ;


		BDBG_MSG(("   HDMI VIC LEN: %d;  HDMI 3D LEN %d",
			hHDMI->AttachedEDID.RxVSDB.HDMI_VIC_Len,
			hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Len)) ;

		BDBG_MSG(("   Image Size: %d",
			hHDMI->AttachedEDID.RxVSDB.HDMI_Image_Size)) ;


		if (hHDMI->AttachedEDID.RxVSDB.HDMI_3D_Present)
		{
			uint8_t i;
			BDBG_MSG(("   Broadcom 3D Formats Supported:")) ;
			for (i = 0 ; i < BFMT_VideoFmt_eMaxCount; i++)
			{
				const BFMT_VideoInfo *pVideoFormatInfo ;
				pVideoFormatInfo = (BFMT_VideoInfo *) BFMT_GetVideoFormatInfoPtr((BFMT_VideoFmt) i) ;

				if (hHDMI->AttachedEDID.BcmSupported3DFormats[i] & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FRAME_PACKING) {
					BDBG_MSG(("   %s Frame Packing ", pVideoFormatInfo->pchFormatStr)) ;
				}

				if (hHDMI->AttachedEDID.BcmSupported3DFormats[i] & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_FIELD_ALTERNATIVE) {
					BDBG_MSG(("   %s FieldAlternative ",	pVideoFormatInfo->pchFormatStr)) ;
				}

				if (hHDMI->AttachedEDID.BcmSupported3DFormats[i] & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LINE_ALTERNATIVE) {
					BDBG_MSG(("   %s LineAlternative ", pVideoFormatInfo->pchFormatStr)) ;
				}

				if (hHDMI->AttachedEDID.BcmSupported3DFormats[i] & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LDEPTH) {
					BDBG_MSG(("   %s LDepth ", pVideoFormatInfo->pchFormatStr)) ;
				}

				if (hHDMI->AttachedEDID.BcmSupported3DFormats[i] & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_LDEPTH_GFX)	{
					BDBG_MSG(("    %s LDepth+Graphics ", pVideoFormatInfo->pchFormatStr)) ;
				}

				if (hHDMI->AttachedEDID.BcmSupported3DFormats[i] & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_TOP_BOTTOM) {
					BDBG_MSG(("   %s TopAndBottom ", pVideoFormatInfo->pchFormatStr)) ;
				}

				if (hHDMI->AttachedEDID.BcmSupported3DFormats[i] & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_FULL) {
					BDBG_MSG(("   %s SideBySide_Full ", pVideoFormatInfo->pchFormatStr)) ;
				}

				if (hHDMI->AttachedEDID.BcmSupported3DFormats[i] & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_HALF_HORIZ) {
					BDBG_MSG(("   %s SideBySide_Half_Horizontal ", pVideoFormatInfo->pchFormatStr)) ;
				}

				if (hHDMI->AttachedEDID.BcmSupported3DFormats[i] & BHDM_EDID_VSDB_3D_STRUCTURE_ALL_SBS_HALF_QUINC) {
					BDBG_MSG(("   %s SideBySide_Half_QuincunxMatrix ", pVideoFormatInfo->pchFormatStr)) ;
				}
			}
		}
	}
#endif
}

static void BHDM_EDID_P_ParseHdmi_HF_VSDB(const BHDM_Handle hHDMI, uint8_t DataBlockIndex, uint8_t DataBlockLength)
{
	uint8_t offset ;
	uint8_t offsetByte ;

	BSTD_UNUSED(offset) ;
	BSTD_UNUSED(offsetByte) ;
	BSTD_UNUSED(DataBlockLength) ;

	BKNI_Memset(&hHDMI->AttachedEDID.RxHdmiForumVsdb, 0,  sizeof(BHDM_EDID_RxHfVsdb)) ;

	hHDMI->AttachedEDID.RxHdmiForumVsdb.exists = true ;

	hHDMI->AttachedEDID.RxHdmiForumVsdb.Version = hHDMI->AttachedEDID.Block[DataBlockIndex + 4] ;

	/* get the maximum TMDS clock rate supported */
	hHDMI->AttachedEDID.RxHdmiForumVsdb.Max_TMDS_Clock_Rate =
		hHDMI->AttachedEDID.Block[DataBlockIndex+5] * 5 ;

	hHDMI->AttachedEDID.RxHdmiForumVsdb._3D_OSD_Disparity =
		hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x01 ;
	hHDMI->AttachedEDID.RxHdmiForumVsdb.DualView =
		(hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x02) >> 1 ;
	hHDMI->AttachedEDID.RxHdmiForumVsdb.IndependentView =
		(hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x04) >> 2 ;

	/* Sink supports scrambling for pixel clocks <= 340MHz */
	hHDMI->AttachedEDID.RxHdmiForumVsdb.LTE_340MScrambleSupport =
		(hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x08) >> 3 ;

	/* Sink can initiate SCDC Read Request */
	hHDMI->AttachedEDID.RxHdmiForumVsdb.RRCapable =
		(hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x40) >> 6 ;
	hHDMI->AttachedEDID.RxHdmiForumVsdb.SCDCSupport =
		(hHDMI->AttachedEDID.Block[DataBlockIndex+6] & 0x80) >> 7 ;

	/* TODO  */
	hHDMI->AttachedEDID.RxHdmiForumVsdb.DeepColor_420_30bit =
		(hHDMI->AttachedEDID.Block[DataBlockIndex+7] & 0x01)  ;
	hHDMI->AttachedEDID.RxHdmiForumVsdb.DeepColor_420_36bit =
		(hHDMI->AttachedEDID.Block[DataBlockIndex+7] & 0x02) >> 1 ;
	hHDMI->AttachedEDID.RxHdmiForumVsdb.DeepColor_420_48bit =
		(hHDMI->AttachedEDID.Block[DataBlockIndex+7] & 0x04) >> 2 ;

#if BHDM_CONFIG_HAS_HDCP22
	if (hHDMI->AttachedEDID.RxHdmiForumVsdb.SCDCSupport)
	{
		BERR_Code rc ;
		rc = BHDM_SCDC_Initialize(hHDMI) ;
		if (rc) { BERR_TRACE(rc) ; }
	}
#endif


#if BDBG_DEBUG_BUILD

	BDBG_MSG(("HDMI Forum VSDB Version %d",
		hHDMI->AttachedEDID.RxHdmiForumVsdb.Version));
	BDBG_MSG(("Max TMDS Character Rate: %d",
		hHDMI->AttachedEDID.RxHdmiForumVsdb.Max_TMDS_Clock_Rate));


	BDBG_MSG(("3D OSD Disparity: %s ",
		g_status[hHDMI->AttachedEDID.RxHdmiForumVsdb._3D_OSD_Disparity ? 1 : 0])) ;
	BDBG_MSG(("Dual View Support: %s ",
		g_status[hHDMI->AttachedEDID.RxHdmiForumVsdb.DualView ? 1 : 0])) ;
	BDBG_MSG(("Independent View Support: %s ",
		g_status[hHDMI->AttachedEDID.RxHdmiForumVsdb.IndependentView ? 1 : 0])) ;
	BDBG_MSG(("Scrambling support for Pixel Clock < 340: %s ",
		g_status[hHDMI->AttachedEDID.RxHdmiForumVsdb.LTE_340MScrambleSupport ? 1 : 0])) ;
	BDBG_MSG(("SCDC Read Initiation Support: %s ",
		g_status[hHDMI->AttachedEDID.RxHdmiForumVsdb.RRCapable ? 1 : 0])) ;
	BDBG_MSG(("SCDC Support: %s ",
		g_status[hHDMI->AttachedEDID.RxHdmiForumVsdb.SCDCSupport ? 1 : 0])) ;

	BDBG_MSG(("HDMI Forum 4:2:0 Deep Color Support")) ;
	BDBG_MSG(("    16 bit: %s;   12 bit: %s;   10 bit: %s;  ",
		g_status[hHDMI->AttachedEDID.RxHdmiForumVsdb.DeepColor_420_48bit ? 1 : 0],
		g_status[hHDMI->AttachedEDID.RxHdmiForumVsdb.DeepColor_420_36bit ? 1 : 0],
		g_status[hHDMI->AttachedEDID.RxHdmiForumVsdb.DeepColor_420_30bit ? 1 : 0])) ;

#endif

}


static void BHDM_EDID_P_ParseVendorSpecificDB(const BHDM_Handle hHDMI, uint8_t DataBlockIndex, uint8_t DataBlockLength)
{
	static const uint8_t ucpIEEE_HDMI_1_4_RegId[3] = {0x03, 0x0C, 0x00} ; /* LSB.. MSB */
	static const uint8_t ucpIEEE_HF_2_0_RegId[3] = {0xD8, 0x5D, 0xC4} ; /* LSB.. MSB */
	uint8_t *ucpIEEE_RegId ;

	ucpIEEE_RegId = &hHDMI->AttachedEDID.Block[DataBlockIndex+1] ;

	BDBG_MSG(("*** VSDB IEEE Reg ID <%02X%02X%02X> ***",
			ucpIEEE_RegId[2], ucpIEEE_RegId[1], ucpIEEE_RegId[0])) ;

	/* make sure it is the correct IEEE Registration ID */
	if (BKNI_Memcmp(ucpIEEE_RegId, ucpIEEE_HDMI_1_4_RegId, 3) == 0)
	{
		BHDM_EDID_P_ParseHdmi_1_4VSDB(hHDMI, DataBlockIndex, DataBlockLength) ;
	}
	else if (BKNI_Memcmp(ucpIEEE_RegId, ucpIEEE_HF_2_0_RegId, 3) == 0)
	{
		BHDM_EDID_P_ParseHdmi_HF_VSDB(hHDMI, DataBlockIndex, DataBlockLength) ;
	}
	else
	{
		BDBG_ERR(("VSDB IEEE Reg ID <%02X%02X%02X> not recognized/supported and will not be processed",
			ucpIEEE_RegId[2], ucpIEEE_RegId[1], ucpIEEE_RegId[0])) ;
	}

	return;

}


static BERR_Code BHDM_EDID_P_ProcessDetailedTimingBlock(const BHDM_Handle hHDMI,
	unsigned char *pDescriptor, BHDM_EDID_DetailTiming *DetailTimingBlock, BFMT_VideoFmt *eVideoFmt)
{
	BERR_Code rc = BERR_SUCCESS ;
	BHDM_EDID_ParseDetailedTimingDescriptor(pDescriptor, DetailTimingBlock) ;

	BDBG_MSG(("%4d x %d (%4d%c)  %3d   %3d   %3d      %2d   %2d    %2d      %dMHz   %dx%d",
		DetailTimingBlock->HorizActivePixels, DetailTimingBlock->VerticalActiveLines,
		DetailTimingBlock->Mode ?
			DetailTimingBlock->VerticalActiveLines * 2 : DetailTimingBlock->VerticalActiveLines, Mode[DetailTimingBlock->Mode],
		DetailTimingBlock->HorizBlankingPixels, DetailTimingBlock->HSyncOffset, DetailTimingBlock->HSyncWidth,
		DetailTimingBlock->VerticalBlankingLines, DetailTimingBlock->VSyncOffset, DetailTimingBlock->VSyncWidth,
		DetailTimingBlock->PixelClock, DetailTimingBlock->HSize_mm, DetailTimingBlock->VSize_mm)) ;

	/* convert the DetailTiming to BFMT_eVideoFmt type; */
	rc = BHDM_EDID_P_DetailTiming2VideoFmt(hHDMI, DetailTimingBlock, eVideoFmt) ;
	if (rc)
	{
		BDBG_WRN(("Unknown/Unsupported Detailed Timing Format %4d x %d (%4d%c)",
			DetailTimingBlock->HorizActivePixels, DetailTimingBlock->VerticalActiveLines,
			DetailTimingBlock->Mode ?
				DetailTimingBlock->VerticalActiveLines * 2 : DetailTimingBlock->VerticalActiveLines,
			Mode[DetailTimingBlock->Mode])) ;
		BDBG_WRN((" ")) ;

		/* no need to error trace here; format is unknown or unsupported by the STB */
	}

	return rc;
}

static BERR_Code BHDM_EDID_P_ParseHDRStaticMetadataDB(
	const BHDM_Handle hHDMI,             /* [in] HDMI handle  */
	uint8_t DataBlockIndex,        /* [in] start offset of Video Data Block */
	uint8_t DataBlockLength              /* [in] length (number) of Video ID codes */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t EotfByte  ;


	hHDMI->AttachedEDID.HdrDB.valid = true ;

	EotfByte =
		hHDMI->AttachedEDID.Block[DataBlockIndex + 2] ;

	hHDMI->AttachedEDID.HdrDB.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eSDR] = EotfByte & 0x01 ;
	hHDMI->AttachedEDID.HdrDB.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eHDR] = EotfByte & 0x02 ;
	hHDMI->AttachedEDID.HdrDB.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eSMPTESt2084] = EotfByte & 0x04 ;
	hHDMI->AttachedEDID.HdrDB.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eFuture] = EotfByte & 0x08 ;

	EotfByte =
		hHDMI->AttachedEDID.Block[DataBlockIndex + 3] ;
	hHDMI->AttachedEDID.HdrDB.bMetadataSupport[BHDM_EDID_HdrDbStaticMetadataSupport_eType1] = EotfByte & 0x01 ;

	switch (DataBlockLength)
	{
	case 3 :
		/* do nothing */
		break ;

	case 6 :
		hHDMI->AttachedEDID.HdrDB.MinLuminanceValid = true ;
		hHDMI->AttachedEDID.HdrDB.MinLuminance = hHDMI->AttachedEDID.Block[DataBlockIndex+6] ;

		/***************************************/
		/* FALL THROUGH to get remaining values */
		/***************************************/

	case 5 :
		hHDMI->AttachedEDID.HdrDB.AverageLuminanceValid = true ;
		hHDMI->AttachedEDID.HdrDB.AverageLuminance = hHDMI->AttachedEDID.Block[DataBlockIndex+5] ;

		/***************************************/
		/* FALL THROUGH to get remaining values */
		/***************************************/

	case 4 :
		hHDMI->AttachedEDID.HdrDB.MaxLuminanceValid = true ;
		hHDMI->AttachedEDID.HdrDB.MaxLuminance = hHDMI->AttachedEDID.Block[DataBlockIndex+4] ;
		break ;

	default :
		BDBG_ERR(("Invalid/Unsupported HDR Data Block length %d", DataBlockLength)) ;
	}


#if BDBG_DEBUG_BUILD
	BDBG_MSG(("HDR DB Length: %d", DataBlockLength)) ;
	BDBG_MSG(("Electro_Optical Transfer Function: ")) ;
	BDBG_MSG(("   SDR Lumuinance: %s",
		g_status[hHDMI->AttachedEDID.HdrDB.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eSDR] ? 1 : 0])) ;
	BDBG_MSG(("   HDR Lumuinance: %s",
		g_status[hHDMI->AttachedEDID.HdrDB.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eHDR] ? 1 : 0])) ;
	BDBG_MSG(("   SMPTE ST 2084: %s",
		g_status[hHDMI->AttachedEDID.HdrDB.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eSMPTESt2084] ? 1 : 0])) ;
	BDBG_MSG(("   Future EOTF: %s",
		g_status[hHDMI->AttachedEDID.HdrDB.bEotfSupport[BHDM_EDID_HdrDbEotfSupport_eFuture] ? 1 : 0])) ;

	BDBG_MSG(("   META Data Type 1: %s",
		g_status[hHDMI->AttachedEDID.HdrDB.bMetadataSupport[BHDM_EDID_HdrDbStaticMetadataSupport_eType1] ? 1 : 0])) ;


	if (!hHDMI->AttachedEDID.HdrDB.MaxLuminanceValid) {
		BDBG_MSG(("Max Luminance Not Present")) ;
	}
	else {
		BDBG_MSG(("Max Luminance %d", hHDMI->AttachedEDID.HdrDB.MaxLuminance)) ;
	}

	if (!hHDMI->AttachedEDID.HdrDB.AverageLuminanceValid) {
		BDBG_MSG(("Avg Luminance Not Present")) ;
	}
	else {
		BDBG_MSG(("Avg Luminance %d", hHDMI->AttachedEDID.HdrDB.AverageLuminance)) ;
	}

	if (!hHDMI->AttachedEDID.HdrDB.MinLuminanceValid) {
		BDBG_MSG(("Min Luminance Not Present")) ;
	}
	else {
		BDBG_MSG(("Min Luminance %d", hHDMI->AttachedEDID.HdrDB.MinLuminance)) ;
	}
#endif

	return rc ;
}

static BERR_Code BHDM_EDID_P_ParseV1V2TimingExtension(const BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t
		i, offset,
		DataOffset ;
	BFMT_VideoFmt eVideoFmt ;
	BHDM_EDID_DetailTiming DetailTimingBlock ;

#if BDBG_DEBUG_BUILD
	uint8_t TimingsFound = 0 ;
#endif


	/* check if data blocks exist before the detailed timing data */
	DataOffset = hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_DATA_OFFSET] ;
	if (DataOffset == 0)  /* no detailed timing descriptors */
		return rc ;

	i = 0 ;
	offset = DataOffset ;

	BKNI_Memset((void *) &hHDMI->AttachedEDID.DescriptorHeader, 0x0, BHDM_EDID_DESC_HEADER_LEN) ;


	while (offset < BHDM_EDID_BLOCKSIZE)
	{
		if (BKNI_Memcmp(&hHDMI->AttachedEDID.Block[offset], hHDMI->AttachedEDID.DescriptorHeader,
			BHDM_EDID_DESC_HEADER_LEN) == 0)
		{
			break ;
		}

		/* skip EDID descriptor blocks */
		if (BKNI_Memcmp(&hHDMI->AttachedEDID.Block[offset], (void *) &hHDMI->AttachedEDID.DescriptorHeader, 3) == 0)
		{
			offset = DataOffset + BHDM_EDID_MONITOR_DESC_SIZE * i++ ;
			continue ;
		}

		BHDM_EDID_ParseDetailedTimingDescriptor(
			&hHDMI->AttachedEDID.Block[offset], &DetailTimingBlock) ;

		/* convert the DetailTiming to BFMT_eVideoFmt type */
		if (BHDM_EDID_P_DetailTiming2VideoFmt(hHDMI, &DetailTimingBlock, &eVideoFmt)
		== BERR_SUCCESS)
		{
			/* set BFMTs that match this Detailed Timing Format as being supported */
			BHDM_EDID_P_SetSupportedMatchingFmts(hHDMI, eVideoFmt) ;

#if BDBG_DEBUG_BUILD
			BDBG_MSG(("Preferred Timing #%d found in V1/2 Timing Ext block", ++TimingsFound)) ;
#endif
		}

		offset = DataOffset + BHDM_EDID_MONITOR_DESC_SIZE * i++ ;
	}
	return rc ;
}


static BERR_Code BHDM_EDID_P_ParseV3TimingExtension (const BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS;
	uint8_t
		DataOffset,
		DataBlockIndex,
		DataBlockTag,
		DataBlockLength,
		MonitorSupport,
		ExtendedTagCode;

	DataOffset = hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_DATA_OFFSET] ;

	/* check if data blocks exist before the detailed timing data */
	if ((DataOffset != 0) &&
		(DataOffset != 4)) /* yes, data blocks exist. */
	{
		/* Look for Video Data Block and parse it first;
		     it is needed prior to determining 3D support which is specified in the VSDB
		*/

		/* Set the index to the start of Data Blocks */
		DataBlockIndex = BHDM_EDID_EXT_DATA_BLOCK_COLLECTION ;

		while (DataBlockIndex < DataOffset)
		{
			/* get the Data Block type */
			DataBlockTag = hHDMI->AttachedEDID.Block[DataBlockIndex] >> 5 ;

			/* get the Data Block length */
			DataBlockLength = hHDMI->AttachedEDID.Block[DataBlockIndex] & 0x1F ;

			if (DataBlockTag == BHDM_EDID_CeaDataBlockTag_eVideoDB)
			{
				BDBG_MSG(("******************** DATA BLOCK *******************")) ;
				BDBG_MSG(("[%#02X] CEA-861 %s (%#02x) (%d bytes)",
						  hHDMI->AttachedEDID.Block[DataBlockIndex],
						  CeaTagName[DataBlockTag], DataBlockTag, DataBlockLength)) ;

				/* adds to supported BCM video formats */
				BHDM_EDID_P_ParseVideoDB(hHDMI, DataBlockIndex, DataBlockLength) ;
				BDBG_MSG(("*************************************************")) ;
				BDBG_MSG((" ")) ;
			}
			DataBlockIndex += DataBlockLength + 1;
		} /* while DataBlockIndex < DataOffset */


		/* scan through the data blocks once again and retrieve all other the necessary information.
		Set the index to the start of Data Blocks */
		DataBlockIndex = BHDM_EDID_EXT_DATA_BLOCK_COLLECTION ;

		while (DataBlockIndex < DataOffset)
		{
			/* get the Data Block type */
			DataBlockTag = hHDMI->AttachedEDID.Block[DataBlockIndex] >> 5 ;

			/* get the Data Block length */
			DataBlockLength = hHDMI->AttachedEDID.Block[DataBlockIndex] & 0x1F ;

			/* this is repetitive, but it avoids extra output on debug messages
			     since Video Data Blocks have already been parsed  */
			if (DataBlockTag == BHDM_EDID_CeaDataBlockTag_eVideoDB)
			{
				DataBlockIndex += DataBlockLength + 1;
				continue ;
			}

			BDBG_MSG(("******************** DATA BLOCK *******************")) ;
			BDBG_MSG(("*** [%#02X] CEA-861 %s (%#02x)  (%d bytes) ***",
					  hHDMI->AttachedEDID.Block[DataBlockIndex],
					  CeaTagName[DataBlockTag], DataBlockTag, DataBlockLength)) ;

			switch (DataBlockTag)
			{

			case BHDM_EDID_CeaDataBlockTag_eAudioDB :	/* Audio DB */
				/* adds to supported BCM audio formats */
				BHDM_EDID_P_ParseAudioDB(hHDMI, DataBlockIndex, DataBlockLength) ;
				break ;

			/* Video Data Block already parse at this point */
#if 0
			/* coverity[dead_error_condition] */
			case BHDM_EDID_CeaDataBlockTag_eVideoDB :	/* Video DB */
				break ;
#endif

			case BHDM_EDID_CeaDataBlockTag_eVSDB :		/* Vendor Specific DB */
				/* populates RxVSDB */
				BHDM_EDID_P_ParseVendorSpecificDB(hHDMI, DataBlockIndex, DataBlockLength) ;
				break ;

			case BHDM_EDID_CeaDataBlockTag_eSpeakerDB : /* Speaker Allocation DB */
				break ;

			case BHDM_EDID_CeaDataBlockTag_eExtendedDB:
				ExtendedTagCode = hHDMI->AttachedEDID.Block[DataBlockIndex+1];
				if (ExtendedTagCode <= 18)
				{
					BDBG_MSG(("   Extended Tag <%02d>  [%s]",
						ExtendedTagCode, ExtendedCeaTagName[ExtendedTagCode])) ;

#if BDBG_DEBUG_BUILD
#if 0
					{
						/* code to debug raw data bytes */
						uint8_t i ;
						BDBG_MSG(("   Extended DB bytes:")) ;
						for (i = 0 ; i <= DataBlockLength; i++)
						{
							BDBG_MSG(("      Data Block[%d] %02x",
								DataBlockIndex+i , hHDMI->AttachedEDID.Block[DataBlockIndex+i])) ;
						}
					}
#endif
#endif
				}

				switch (ExtendedTagCode)
				{
				case BHDM_EDID_CeaExtendedDBTag_eColorimetryDB :
					BHDM_EDID_P_ParseColorimetryDB(hHDMI, DataBlockIndex) ;
					break ;


				case BHDM_EDID_CeaExtendedDBTag_eVideoCapabilityDB :
					BHDM_EDID_P_ParseVideoCapablityDB(hHDMI, DataBlockIndex, DataBlockLength) ;
					break ;

				case BHDM_EDID_CeaExtendedDBTag_eYCBCR420CapabilityMapDB :
					BHDM_EDID_P_ParseYCbCr420CapabilityMapDB(hHDMI, DataBlockIndex, DataBlockLength) ;
					break ;

				case BHDM_EDID_CeaExtendedDBTag_eVideoFormatPreferenceDB :
					BHDM_EDID_P_ParseFormatPreferenceDB(hHDMI, DataBlockIndex, DataBlockLength) ;
					break ;

				case BHDM_EDID_CeaExtendedDBTag_eYCBCR420VideoDB :
					BHDM_EDID_P_ParseYCbCr420VideoDB(hHDMI, DataBlockIndex, DataBlockLength) ;
					break ;

				case BHDM_EDID_CeaExtendedDBTag_eHDRStaticMetaDB :
					BHDM_EDID_P_ParseHDRStaticMetadataDB(hHDMI, DataBlockIndex, DataBlockLength) ;
					break ;

				default :
					{
						BDBG_MSG((" Extended Tag Code <%d> is not supported",
								ExtendedTagCode)) ;
					}
					break ;
				}
				break;

			case BHDM_EDID_CeaDataBlockTag_eReserved0 :
			case BHDM_EDID_CeaDataBlockTag_eReserved5 :
			case BHDM_EDID_CeaDataBlockTag_eReserved6 :


			default : /* note any Unknown/Unsupported Tags */
				BDBG_WRN(("CEA Data Block Tag Code %d is not supported",
					   DataBlockTag)) ;
				break ;
			}

			DataBlockIndex += DataBlockLength + 1;
			BDBG_MSG(("*************************************************")) ;
			BDBG_MSG((" ")) ;

		} /* while DataBlockIndex < DataOffset */
	}


	/* check what the monitor supports */
	MonitorSupport = hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_MONITOR_SUPPORT] ;

	hHDMI->AttachedEDID.RxVSDB.Underscan = MonitorSupport & 0x80 ;
	hHDMI->AttachedEDID.RxVSDB.Audio = MonitorSupport & 0x40 ;
	hHDMI->AttachedEDID.RxVSDB.YCbCr444  = MonitorSupport & 0x20 ;
	hHDMI->AttachedEDID.RxVSDB.YCbCr422  = MonitorSupport & 0x10 ;
	hHDMI->AttachedEDID.RxVSDB.NativeFormatsInDescriptors = MonitorSupport & 0xF ;

	BDBG_MSG(("Pixel Encodings Supported by Monitor: RGB Yes ; YCbCr 4:4:4 %s ; YCbCr 4:2:2 %s",
		g_status[hHDMI->AttachedEDID.RxVSDB.YCbCr444 ? 1 : 0],
		g_status[hHDMI->AttachedEDID.RxVSDB.YCbCr422 ? 1 : 0])) ;

	/* check if there are any detailed timing descriptors. */
	if (DataOffset > 0)
	{
		uint16_t offset = DataOffset ;
		BHDM_EDID_DetailTiming DetailTimingBlock ;
		BFMT_VideoFmt eVideoFmt ;
		bool HeaderPrinted = false ;

		BKNI_Memset((void *) &hHDMI->AttachedEDID.DescriptorHeader, 0x0, BHDM_EDID_DESC_HEADER_LEN);

		while ((offset > 0) && (offset < BHDM_EDID_BLOCKSIZE)
			&& BKNI_Memcmp( /* detailed timing descriptors contain non-zeros */
				&hHDMI->AttachedEDID.Block[offset],
				&hHDMI->AttachedEDID.DescriptorHeader, 3))
		{
			/* now process the timing descriptors */
			if (!HeaderPrinted)
			{
				BDBG_MSG(("Additional Detailed Timing Blocks found in V3 Timing Extension:")) ;
				BDBG_MSG(("Format             HBlnk HOfst HWidth  VBlnk VOfst VWidth  PxlClk  ScrSz")) ;
				HeaderPrinted = true ;
			}

			rc = BHDM_EDID_P_ProcessDetailedTimingBlock(hHDMI,
				&hHDMI->AttachedEDID.Block[offset], &DetailTimingBlock, &eVideoFmt) ;
			if (rc != BERR_SUCCESS)
			{
				/* skip to next descriptor */
				offset = offset + BHDM_EDID_MONITOR_DESC_SIZE ;
				continue ;
			}

			/* set BFMTs that match this Detailed Timing Format as being supported */
			BHDM_EDID_P_SetSupportedMatchingFmts(hHDMI, eVideoFmt) ;

			/* skip to next descriptor */
			offset = offset + BHDM_EDID_MONITOR_DESC_SIZE ;
		}
		if (HeaderPrinted) { BDBG_MSG(("  ")) ; }
	}

	/* Check if a RxVSDB has been found in the V3 Timing Extension */
	if (!hHDMI->AttachedEDID.RxVSDB.valid)
	{
		BDBG_WRN(("NO HDMI SUPPORT; HDMI Rx Vendor Specific Data Block (VSDB) not found")) ;
	}
	else if (!hHDMI->AttachedEDID.RxVSDB.Audio)
	{
		BDBG_WRN(("Attached HDMI device '%s' does not support audio",
			hHDMI->AttachedEDID.MonitorName)) ;
	}
	else if (!hHDMI->AttachedEDID.BcmSupportedAudioFormats[BAVC_AudioFormat_ePCM].Supported)
	{
		/* all HDMI Rx that set VSDB.Audio must support Basic Audio (eg 2 Ch PCM)
		 although not all devices explicitly specify it in Audio Descriptors */
		BDBG_WRN(("VSDB indicates audio support, but no PCM Audio Descriptors found!")) ;
		BDBG_WRN(("Assuming 2 channel PCM is supported")) ;

		/* all HDMI Rxs are required to support PCM Audio; list 48KHz PCM as supported */
		hHDMI->AttachedEDID.BcmSupportedAudioFormats[BAVC_AudioFormat_ePCM].Supported = 1 ;
		hHDMI->AttachedEDID.BcmSupportedAudioFormats[BAVC_AudioFormat_ePCM].AudioChannels = 2 ;
		hHDMI->AttachedEDID.BcmSupportedAudioFormats[BAVC_AudioFormat_ePCM].ucCeaSampleRates
			= BAVC_AudioSamplingRate_e48k ;
		hHDMI->AttachedEDID.BcmSupportedAudioFormats[BAVC_AudioFormat_ePCM].ucCeaNBits_BitRate
			= 1 ;	/* 16 bits */
		hHDMI->AttachedEDID.BcmAudioFormatsChecked = 1 ;
	}

	return rc;

}


BERR_Code BHDM_EDID_P_ProcessTimingExtension (const BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS;

	/* check for Version 1/2/3 Timing Data Extensions */
	switch (hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_VERSION])
	{
	case BHDM_EDID_TIMING_VERSION_1 :
	case BHDM_EDID_TIMING_VERSION_2 :
		BDBG_MSG(("Parse Version 1/2 Timing Extension ")) ;
		BHDM_EDID_P_ParseV1V2TimingExtension(hHDMI) ;
		break  ;

	case BHDM_EDID_TIMING_VERSION_3 :
		BDBG_MSG(("V3 Timing Extension Found in EDID Extension.")) ;
		BHDM_EDID_P_ParseV3TimingExtension(hHDMI) ;
		break ;

	case BHDM_EDID_TIMING_VERSION_4 :
		BDBG_WRN(("Parsing Timing Extension Version 4 as Version 3")) ;
		BHDM_EDID_P_ParseV3TimingExtension(hHDMI) ;
		break ;

	default :
		BDBG_WRN(("Uknown/Unsupported Timing Extension Version %d",
			hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_VERSION])) ;
	}

	return rc;
}

static BERR_Code BHDM_EDID_P_ProcessExtensionBlock (const BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS;

	/* check for Extension type */
	switch (hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_TAG])
	{
	case BHDM_EDID_EXT_TIMING_DATA :
		BHDM_EDID_P_ProcessTimingExtension(hHDMI) ;
		break;

	case BHDM_EDID_EXT_LCD_TIMINGS :
	case BHDM_EDID_EXT_EDID_2_0 :
	case BHDM_EDID_EXT_COLOR_INFO :
	case BHDM_EDID_EXT_DVI_FEATURE :
	case BHDM_EDID_EXT_TOUCH_SCREEN :
	case BHDM_EDID_EXT_MANUFACTURER :
	default :
		BDBG_WRN(("Uknown/Unsupported Extension  %d",
			hHDMI->AttachedEDID.Block[BHDM_EDID_EXT_TAG])) ;
	}

	return rc;
}

static BERR_Code BHDM_EDID_P_ProcessBlockMap (const BHDM_Handle hHDMI)
{
	BERR_Code rc = BERR_SUCCESS;
	uint8_t block_map[BHDM_EDID_BLOCKSIZE];
	uint8_t i, index;

	index = hHDMI->AttachedEDID.CachedBlock;

	/* Copy the block map from the hHDMI handle */
	for(i=0; i< BHDM_EDID_BLOCKSIZE; i++){
		block_map[i]=hHDMI->AttachedEDID.Block[i];
	}

	i = 1;
	while((i < (BHDM_EDID_BLOCKSIZE-1)) && (block_map[i])){
		BHDM_CHECK_RC(rc, BHDM_EDID_GetNthBlock(hHDMI, (i + index), hHDMI->AttachedEDID.Block, BHDM_EDID_BLOCKSIZE)) ;
		BHDM_EDID_P_ProcessExtensionBlock (hHDMI);
		i++;
	}

	if(hHDMI->AttachedEDID.BasicData.Extensions < BHDM_EDID_BLOCKSIZE) {
		if(hHDMI->AttachedEDID.BasicData.Extensions != i) {
			hHDMI->AttachedEDID.BasicData.Extensions = i;
		}
	}
	else {
		if(hHDMI->AttachedEDID.BasicData.Extensions != (i+index)) {
			hHDMI->AttachedEDID.BasicData.Extensions = i;
		}
	}

done:
	return rc;
}

/******************************************************************************
Summary:
Notify the EDID Block to re-read the EDID; initializing the values read from the EDID
*******************************************************************************/
BERR_Code BHDM_EDID_Initialize(
	const BHDM_Handle hHDMI                    /* [in] HDMI handle  */
)
{
	BERR_Code rc ;
	uint8_t
		offset,
		Tag ;
	uint8_t SupportedTimingsFound = 0 ;
	BFMT_VideoFmt eVideoFmt ;

	uint8_t i; /* indexes */

	uint8_t RxDeviceAttached ;
	static const uint8_t ucEdidHeader[BHDM_EDID_HEADER_SIZE] =
		{ 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00} ;

	BHDM_EDID_DetailTiming DetailTiming ;
	BHDM_EDID_P_VideoDescriptor *pVideoDescriptor ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;
	BDBG_ENTER(BHDM_EDID_Initialize) ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	/* use cached EDID when available */
	hHDMI->bUseCachedEdid = true ;

	/* dont bother reading if EDID has been bypassed (DEBUG mode only) */
#if BDBG_DEBUG_BUILD
	if (hHDMI->DeviceSettings.BypassEDIDChecking)
	{
		BDBG_WRN(("*************************************************")) ;
		BDBG_WRN(("!!!EDID is BYPASSED!!!")) ;
		BDBG_WRN(("STB will attempt to display all formats in HDMI mode")) ;
		BDBG_WRN(("ByPassed Mode should only be used for debugging")) ;
		BDBG_WRN(("*************************************************")) ;

		for (i = 0 ; i < BFMT_VideoFmt_eMaxCount ; i++)
		{
			/* check HDMI formats only */
			if (!BFMT_SUPPORT_HDMI((BFMT_VideoFmt) i))
				continue ;

			hHDMI->AttachedEDID.BcmSupportedVideoFormats[i] = true ; /* set as supported */
			hHDMI->AttachedEDID.BcmSupported420VideoFormats[i] = true ;
		}
		hHDMI->AttachedEDID.BcmVideoFormatsChecked = 1 ;
		hHDMI->AttachedEDID.BcmSupported420VideoFormatsChecked = 1 ;
		hHDMI->AttachedEDID.RxHasHdmiSupport = true ;

		hHDMI->edidStatus = BHDM_EDID_STATE_eOK;
		rc = BERR_SUCCESS ;

		goto done ;
	}
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
	/* clear all current EDID information */
	BKNI_Memset((void *) &hHDMI->AttachedEDID, 0, sizeof(BHDM_EDID_DATA)) ;
	hHDMI->AttachedEDID.RxHdmiForumVsdb.Max_TMDS_Clock_Rate = BHDM_HDMI_1_4_MAX_RATE ;

	BDBG_MSG(("Initializing/Reading EDID Information   (%s %d)",
		BHDM_P_GetVersion(), BCHP_CHIP)) ;

	/* always add support for VGA (640x480p) */
	/* in case EDID is completely bogus and cannot be read or parsed */
	hHDMI->AttachedEDID.BcmSupportedVideoFormats[BFMT_VideoFmt_eDVI_640x480p] = true ;

	/* read the 1st EDID Block and parse the information of interest it contains */

	/* during InitializeEDID, always force the reading of EDID block 0 */
	/* incorrectly implemented Hot Plug signals sometimes cause a problem */
	hHDMI->edidStatus = BHDM_EDID_STATE_eInitialize;

	BHDM_CHECK_RC(rc, BHDM_EDID_GetNthBlock(hHDMI, 0, hHDMI->AttachedEDID.Block, BHDM_EDID_BLOCKSIZE)) ;

	/**************************************
	 Parse Basic EDID Data
	**************************************/

	if (BKNI_Memcmp(&ucEdidHeader[0], &hHDMI->AttachedEDID.Block[0],
		BHDM_EDID_HEADER_SIZE))
	{
		BDBG_WRN(("Invalid/Missing EDID Header %02X %02X %02X %02X %02X %02X %02X %02X... ignoring",
			hHDMI->AttachedEDID.Block[0], hHDMI->AttachedEDID.Block[1],
			hHDMI->AttachedEDID.Block[2], hHDMI->AttachedEDID.Block[3],
			hHDMI->AttachedEDID.Block[4], hHDMI->AttachedEDID.Block[5],
			hHDMI->AttachedEDID.Block[6], hHDMI->AttachedEDID.Block[7])) ;
	}

	BKNI_Memcpy(&hHDMI->AttachedEDID.BasicData.VendorID,
		&hHDMI->AttachedEDID.Block[BHDM_EDID_VENDOR_ID], 2) ;

	BKNI_Memcpy(&hHDMI->AttachedEDID.BasicData.ProductID,
		&hHDMI->AttachedEDID.Block[BHDM_EDID_PRODUCT_ID], 2) ;

	BKNI_Memcpy(&hHDMI->AttachedEDID.BasicData.SerialNum,
		&hHDMI->AttachedEDID.Block[BHDM_EDID_SERIAL_NO], 4) ;

	 hHDMI->AttachedEDID.BasicData.ManufWeek=
	 	hHDMI->AttachedEDID.Block[BHDM_EDID_MANUFACTURE_WEEK] ;
	 hHDMI->AttachedEDID.BasicData.ManufYear =
	 	hHDMI->AttachedEDID.Block[BHDM_EDID_MANUFACTURE_YEAR] ;
	 hHDMI->AttachedEDID.BasicData.features =
	 	hHDMI->AttachedEDID.Block[BHDM_EDID_FEATURE_SUPPORT] ;


	hHDMI->AttachedEDID.BasicData.EdidVersion = hHDMI->AttachedEDID.Block[BHDM_EDID_VERSION] ;
	hHDMI->AttachedEDID.BasicData.EdidRevision = hHDMI->AttachedEDID.Block[BHDM_EDID_REVISION] ;
	hHDMI->AttachedEDID.BasicData.Extensions = hHDMI->AttachedEDID.Block[BHDM_EDID_EXTENSION] ;

	if ((hHDMI->AttachedEDID.BasicData.EdidVersion == 0xFF)
	||  (hHDMI->AttachedEDID.BasicData.EdidRevision	== 0xFF)
	||  (hHDMI->AttachedEDID.BasicData.Extensions == 0xFF)
	|| ( (hHDMI->AttachedEDID.BasicData.EdidVersion == 0x00) &&
		(hHDMI->AttachedEDID.BasicData.EdidRevision == 0x00) &&
		(hHDMI->AttachedEDID.BasicData.Extensions == 0x00) ))
	{
		/* probably read all 0xFF or 0x00; invalid EDID */
		BDBG_ERR(("EDID returns all 0xFF or 0x00 values. Invalid EDID information"));
		hHDMI->edidStatus = BHDM_EDID_STATE_eInvalid;
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	BDBG_MSG(("Version %d.%d  Number of Extensions: %d",
		hHDMI->AttachedEDID.BasicData.EdidVersion,
		hHDMI->AttachedEDID.BasicData.EdidRevision, hHDMI->AttachedEDID.BasicData.Extensions)) ;

	hHDMI->AttachedEDID.BasicData.MaxHorizSize = hHDMI->AttachedEDID.Block[BHDM_EDID_MAX_HORIZ_SIZE] ;
	hHDMI->AttachedEDID.BasicData.MaxVertSize  = hHDMI->AttachedEDID.Block[BHDM_EDID_MAX_VERT_SIZE] ;
	BDBG_MSG(("H x V Size (cm): %d x %d",
		hHDMI->AttachedEDID.BasicData.MaxHorizSize,
		hHDMI->AttachedEDID.BasicData.MaxVertSize)) ;

	/*
	** Additional Information for Display Purposes only
	** Add to EDID Data structure if desired...
	*/
	BDBG_MSG(("Manufacture Week / Year: %d / %d",
		hHDMI->AttachedEDID.Block[BHDM_EDID_MANUFACTURE_WEEK],
		hHDMI->AttachedEDID.Block[BHDM_EDID_MANUFACTURE_YEAR] + 1990)) ;

	/* Serial Number may also be placed in a Descriptor Tag */
	BDBG_MSG(("Serial Number: %02X %02X %02X %02X",
		hHDMI->AttachedEDID.Block[BHDM_EDID_SERIAL_NO], hHDMI->AttachedEDID.Block[BHDM_EDID_SERIAL_NO+1],
		hHDMI->AttachedEDID.Block[BHDM_EDID_SERIAL_NO+2], hHDMI->AttachedEDID.Block[BHDM_EDID_SERIAL_NO+3])) ;

#if BDBG_DEBUG_BUILD
	{
		 static const char * const DisplayType[] =
		 {
		 	"Monochrome Display",
		 	"RGB Display",
		 	"Non-RGB Display",
		 	"Undefined"
		 } ;
		 BDBG_MSG(("Feature Support: <%#x>", hHDMI->AttachedEDID.BasicData.features)) ;
		 BDBG_MSG(("   Standby Supported: %s",
		 	g_status[(hHDMI->AttachedEDID.BasicData.features & 0x80) ? 1 : 0])) ;
		 BDBG_MSG(("   Suspend Supported: %s",
			 g_status[(hHDMI->AttachedEDID.BasicData.features & 0x40) ? 1 : 0])) ;
		 BDBG_MSG(("   Active Off Supported: %s",
			 g_status[(hHDMI->AttachedEDID.BasicData.features & 0x20) ? 1 : 0])) ;
		 BDBG_MSG(("   Display Type: %s",
		 	DisplayType[ (hHDMI->AttachedEDID.BasicData.features & 0x18) >> 3])) ;
		 BDBG_MSG(("   sRGB Colorspace Supported: %s",
			 g_status[(hHDMI->AttachedEDID.BasicData.features & 0x04) ? 1 : 0])) ;
		 BDBG_MSG(("   Preferred Timing in Block 1 (must always be set): %s",
			 g_status[(hHDMI->AttachedEDID.BasicData.features & 0x02) ? 1 : 0])) ;
		 BDBG_MSG(("   Default GTF Supported: %s",
			 g_status[(hHDMI->AttachedEDID.BasicData.features & 0x01) ? 1 : 0])) ;
	}
#endif

	/*************************************************************************
	 Parse the 4 Detailed Timing Blocks contained in this EDID Block.
	 Should contain:
	 	 1 Monitor Name,
	 	 1 Monitor Range,
	 	 and 1 to 2 Detailed Timing Descriptors
	 EDID Version 1.3 and up
	**************************************************************************/

	BKNI_Memset((void *) &hHDMI->AttachedEDID.DescriptorHeader, 0x0, BHDM_EDID_DESC_HEADER_LEN);

	/* Check the four Descriptor Blocks in the initial 128 EDID  bytes */
	/* search for non-timing desriptors first.  Especially the Monitor Range */
	for (i = 0 ; i < 4 ; i++)
	{
		offset = BHDM_EDID_MONITOR_DESC_1 + BHDM_EDID_MONITOR_DESC_SIZE * i ;

		/* Non-timing descriptors start with 0x00, 0x00, 0x00, <tag>, 0x00 */
		if (BKNI_Memcmp(&hHDMI->AttachedEDID.Block[offset],
			(void *) &hHDMI->AttachedEDID.DescriptorHeader, 3) == 0)
		{
			Tag = hHDMI->AttachedEDID.Block[offset+3] ;

			/* Check which descriptor tag we found */
			switch (Tag)
			{
			case BHDM_EDID_TAG_MONITOR_RANGE : 	/* Range Limits found */
				BDBG_MSG(("Monitor Range found in Descriptor %d", i + 1)) ;
				BHDM_EDID_P_ParseMonitorRange(hHDMI, offset) ;
				break ;

			case BHDM_EDID_TAG_MONITOR_NAME : /* Monitor Name found */
				BKNI_Memcpy(
					hHDMI->AttachedEDID.MonitorName, &hHDMI->AttachedEDID.Block[offset + BHDM_EDID_DESC_DATA],
					BHDM_EDID_MONITOR_DESC_SIZE - BHDM_EDID_DESC_HEADER_LEN) ;

				/* also copy Monitor Name to Basic EDID Data structure */
				BKNI_Memcpy(
					hHDMI->AttachedEDID.BasicData.monitorName, &hHDMI->AttachedEDID.Block[offset + BHDM_EDID_DESC_DATA],
					BHDM_EDID_MONITOR_DESC_SIZE - BHDM_EDID_DESC_HEADER_LEN) ;


				/* replace linefeed with NULL; use offset variable as index */
				for (offset = 0 ; offset < BHDM_EDID_DESC_ASCII_STRING_LEN; offset++)
				{
					if (hHDMI->AttachedEDID.MonitorName[offset] == '\n')
					{
						hHDMI->AttachedEDID.MonitorName[offset] = '\0' ;
						break ;
					}
				}

				BDBG_MSG(("Monitor Name  found in Descriptor %d", i + 1)) ;
				BDBG_MSG(("   %s ", hHDMI->AttachedEDID.MonitorName)) ;

				break ;

			case BHDM_EDID_TAG_MONITOR_SN :
				BDBG_MSG(("Monitor S/N   found in Descriptor %d", i + 1)) ;
				break ;

			case BHDM_EDID_TAG_MONITOR_ASCII :
				/* no support using this in EDID structure */
				break ;

			case BHDM_EDID_TAG_DUMMY_DESC:
				/* descriptor space is unsed. */
				break;

			default :
				/* unknown; continue processing anyway */
				BDBG_WRN(("Unknown Tag <%X> found in Descriptor %d; skip it...",
					Tag, i+1)) ;
			}
		}
	}


	/* now process the timing descriptors */
	BDBG_MSG(("Detailed Timing (Preferred) Formats:")) ;
	BDBG_MSG(("Format             HBlnk HOfst HWidth  VBlnk VOfst VWidth  PxlClk  ScrSz")) ;

	for (i = 0 ; i < 4 ; i++)
	{
		offset = BHDM_EDID_MONITOR_DESC_1 + BHDM_EDID_MONITOR_DESC_SIZE * i ;

		if (BKNI_Memcmp(&hHDMI->AttachedEDID.Block[offset],
			(void *) &hHDMI->AttachedEDID.DescriptorHeader, 3) != 0)
		{

			/* at least one Detail Timing Format has been found; */
			/* indicate formats have been checked */
			hHDMI->AttachedEDID.BcmVideoFormatsChecked = 1 ;

			rc = BHDM_EDID_P_ProcessDetailedTimingBlock(hHDMI,
				&hHDMI->AttachedEDID.Block[offset], &DetailTiming, &eVideoFmt) ;
			if (rc != BERR_SUCCESS)
			{
				/* unable to process Detailed Timing Block continue to next descriptor */
				continue ;
			}

			BDBG_MSG((" ")) ;

			/* keep a copy of first two supported detailed timings for quick retrieval */
			if (SupportedTimingsFound < 2)
			{
				BKNI_Memcpy(
					&(hHDMI->AttachedEDID.SupportedDetailTimings[SupportedTimingsFound]),
					&DetailTiming, sizeof(BHDM_EDID_DetailTiming)) ;
			}

			/* set BFMTs that match this Detailed Timing Format as being supported */
			BHDM_EDID_P_SetSupportedMatchingFmts(hHDMI, eVideoFmt) ;

			SupportedTimingsFound++ ;

		}
	}

	BHDM_EDID_P_ParseEstablishedTimingFormats(hHDMI) ;

	hHDMI->AttachedEDID.SupportedDetailTimingsIn1stBlock = SupportedTimingsFound ;

	/******************************************************************
	 Parse Extension Blocks - primarily interested in Timing Extensions
	******************************************************************/

	if (!hHDMI->AttachedEDID.BasicData.Extensions)
	{
		hHDMI->AttachedEDID.RxHasHdmiSupport = 0 ;
		/* Its not mandatory to have extension blocks. so there is no error. Display a message and continue */
		BDBG_WRN(("No EDID Extensions Found... Monitor supports DVI (Video) only")) ;
		hHDMI->edidStatus = BHDM_EDID_STATE_eOK;
		rc = BERR_SUCCESS ;
		goto done ;
	}

	/* Read the 1st EDID Block and parse to see if its a block map or an extension block(extensions count should be 1). */
	BHDM_CHECK_RC(rc, BHDM_EDID_GetNthBlock(hHDMI, 1, hHDMI->AttachedEDID.Block, BHDM_EDID_BLOCKSIZE)) ;

	if(hHDMI->AttachedEDID.BasicData.Extensions == 1) {
		rc = BHDM_EDID_P_ProcessExtensionBlock(hHDMI);
	}
	else if(hHDMI->AttachedEDID.Block[0] == BHDM_EDID_EXT_BLOCK_MAP){
		rc = BHDM_EDID_P_ProcessBlockMap(hHDMI);

		/* If the number of extension blocks are more than 127, the standard needs  one more block map to accomodate their extension tags. */
		if(hHDMI->AttachedEDID.BasicData.Extensions >= BHDM_EDID_BLOCKSIZE){
			/* Read the 128th EDID Block which should be a block map. */
			BHDM_CHECK_RC(rc, BHDM_EDID_GetNthBlock(hHDMI, 128, hHDMI->AttachedEDID.Block, BHDM_EDID_BLOCKSIZE));

			if(hHDMI->AttachedEDID.Block[0] == BHDM_EDID_EXT_BLOCK_MAP){
				rc = BHDM_EDID_P_ProcessBlockMap(hHDMI);
			}
			else {
				BDBG_ERR(("Wrong EDID extension tag of %d found in Block 128(Block Map); should be 0xF0",
					hHDMI->AttachedEDID.Block[0])) ;
				rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
				goto done ;
			}
		}
	}
	else {
		BDBG_ERR(("Wrong coding of the EDID Extension count in Block 0...")) ;
		rc = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		goto done ;
	}

	hHDMI->edidStatus = BHDM_EDID_STATE_eOK;

done:

#if BDBG_DEBUG_BUILD
	/* print raw EDID info */
	BHDM_EDID_DumpRawEDID(hHDMI);
#endif
	BDBG_LEAVE(BHDM_EDID_Initialize) ;
	return rc ;
} /* BHDM_EDID_Initialize */


/******************************************************************************
BERR_Code BHDM_EDID_GetMonitorName
Summary: Retrieve a copy of the Monitor Name stored in the EDID
*******************************************************************************/
BERR_Code BHDM_EDID_GetMonitorName(
   const BHDM_Handle hHDMI, /* [in] HDMI handle */
   uint8_t *pDescriptorText /* [out] pointer to memory to hold retrieved name */
)
{
	BERR_Code rc = BERR_SUCCESS ;
	uint8_t RxDeviceAttached ;

	BDBG_ENTER(BHDM_EDID_GetMonitorName) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BERR_TRACE(BHDM_NO_RX_DEVICE) ;
		goto done ;
	}

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

#if BDBG_DEBUG_BUILD
	if (hHDMI->DeviceSettings.BypassEDIDChecking)
	{
		BKNI_Memcpy(pDescriptorText, EDIDByPassedText,
			BHDM_EDID_MONITOR_DESC_SIZE - BHDM_EDID_DESC_HEADER_LEN) ;
		goto done ;
	}
#endif

	BKNI_Memcpy(pDescriptorText, hHDMI->AttachedEDID.MonitorName, BHDM_EDID_DESC_ASCII_STRING_LEN) ;

done:
	BDBG_LEAVE(BHDM_EDID_GetMonitorName) ;
	return rc ;
} /* end BHDM_EDID_GetMonitorName */


BERR_Code BHDM_EDID_GetPreferredColorimetry(
	BHDM_Handle hHDMI,
	const BHDM_EDID_ColorimetryParams *parameters,
	BAVC_MatrixCoefficients *eColorimetry)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->DeviceSettings.BypassEDIDChecking)
	{
		if (hHDMI->AttachedEDID.RxHasHdmiSupport)
			*eColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_709 ;
		else
			*eColorimetry = BAVC_MatrixCoefficients_eHdmi_RGB ;
		goto done ;
	}

	*eColorimetry = BAVC_MatrixCoefficients_eDvi_Full_Range_RGB;
	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {

		/* Whether or not the EDID is valid; HDMI requires all devices to support full range RGB */
		/* Default to Full range RGB */
		goto done ;
	}

	if (!hHDMI->AttachedEDID.RxHasHdmiSupport)
	{
		goto done ;
	}

	if (hHDMI->DeviceSettings.overrideDefaultColorimetry)
	{
		BDBG_MSG(("Use non default colorimetry: %d", hHDMI->DeviceSettings.eColorimetry)) ;
		*eColorimetry = hHDMI->DeviceSettings.eColorimetry ;
		goto done ;
	}

	if (! BFMT_IS_UHD(parameters->eVideoFmt)) /* if not UHD */
	{
		*eColorimetry = BAVC_GetDefaultMatrixCoefficients_isrsafe(
			parameters->eVideoFmt, parameters->xvYccEnabled) ;

		/* confirm the default matrix is supported by the attached Rx */
		switch (*eColorimetry)
		{
		case BAVC_MatrixCoefficients_eXvYCC_709 :
			if (!hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_exvYCC709])
			{
				BDBG_WRN(("Requested xvYCC BT709 is not supported by attached Rx; default to BT709")) ;
				*eColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_709 ;
			}
			break ;

		case BAVC_MatrixCoefficients_eXvYCC_601 :
			if (!hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_exvYCC601])
			{
				BDBG_WRN(("Requested xvYCC BT601 is not supported by attached Rx; default to 170M")) ;
				*eColorimetry = BAVC_MatrixCoefficients_eSmpte_170M ;
			}
			break ;

		default :
			/* do nothing */ ;
		}
	}
	else if (hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020cYCC])
	{
		*eColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_2020_CL ;
	}
	else if (hHDMI->AttachedEDID.ColorimetryDB.bExtended[BHDM_EDID_ColorimetryDbExtendedSupport_eBT2020YCC])
	{
		*eColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL ;
	}
	else /* BT 2020 not supported */
	{
		*eColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_709 ;
	}

done:
	return rc  ;
}


BERR_Code BHDM_EDID_GetSupportedColorimetry(
	const BHDM_Handle hHDMI, BHDM_OutputFormat eOutputFormat,
	BFMT_VideoFmt eVideoFmt, BAVC_MatrixCoefficients *eColorimetry)
{
	BERR_Code 	rc = BERR_SUCCESS ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BDBG_WRN(("********************************************")) ;
	BDBG_WRN(("BHDM_EDID_GetSupportedColorimetry has been deprecated")) ;
	BDBG_WRN(("Use BHDM_EDID_GetPreferred Colorimetry instead")) ;
	BDBG_WRN(("********************************************")) ;

	if (hHDMI->DeviceSettings.BypassEDIDChecking)
	{
		if (hHDMI->AttachedEDID.RxHasHdmiSupport)
			*eColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_709 ;
		else
			*eColorimetry = BAVC_MatrixCoefficients_eHdmi_RGB ;
		goto done ;
	}


	*eColorimetry = BAVC_MatrixCoefficients_eDvi_Full_Range_RGB;
	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {
		/* Default to Full range RGB */
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	if (!hHDMI->AttachedEDID.RxHasHdmiSupport)
	{
		goto done ;
	}

	if (hHDMI->DeviceSettings.overrideDefaultColorimetry)
	{
		BDBG_MSG(("Use non default colorimetry: %d", hHDMI->DeviceSettings.eColorimetry)) ;
		*eColorimetry = hHDMI->DeviceSettings.eColorimetry ;
		goto done ;
	}


	/* default to Limited Range RGB */
	*eColorimetry = BAVC_MatrixCoefficients_eHdmi_RGB ;

	/* use YPrPb for HDMI monitors */
	if ((eOutputFormat == BHDM_OutputFormat_eHDMIMode)
	&& (hHDMI->AttachedEDID.RxVSDB.YCbCr444))  /* if supported by attached monitor */
	{
		switch (eVideoFmt)
		{
		case BFMT_VideoFmt_eDVI_640x480p  :  /* except for VGA mode */
			BDBG_MSG(("Use Full Range RGB")) ;
			*eColorimetry = BAVC_MatrixCoefficients_eDvi_Full_Range_RGB ;
			break ;

		case BFMT_VideoFmt_eNTSC :
		case BFMT_VideoFmt_eNTSC_J :

#if BHDM_CONFIG_ANALOG_TRANSLATION_SUPPORT
		case BFMT_VideoFmt_e720x482_NTSC :	   /* 720x482i NSTC-M for North America */
		case BFMT_VideoFmt_e720x482_NTSC_J:    /* 720x482i Japan */
		case BFMT_VideoFmt_e720x483p :
#endif


		case BFMT_VideoFmt_e480p :

		case BFMT_VideoFmt_ePAL_B :
		case BFMT_VideoFmt_ePAL_B1 :
		case BFMT_VideoFmt_ePAL_D :
		case BFMT_VideoFmt_ePAL_D1 :
		case BFMT_VideoFmt_ePAL_G :
		case BFMT_VideoFmt_ePAL_H :
		case BFMT_VideoFmt_ePAL_K :
		case BFMT_VideoFmt_ePAL_I :
		case BFMT_VideoFmt_ePAL_M :
		case BFMT_VideoFmt_ePAL_N :
		case BFMT_VideoFmt_ePAL_NC :
		case BFMT_VideoFmt_ePAL_60 :
		case BFMT_VideoFmt_eSECAM_L :
		case BFMT_VideoFmt_eSECAM_B :
		case BFMT_VideoFmt_eSECAM_G :
		case BFMT_VideoFmt_eSECAM_D :
		case BFMT_VideoFmt_eSECAM_K :
		case BFMT_VideoFmt_eSECAM_H :
		case BFMT_VideoFmt_e576p_50Hz :
			BDBG_MSG(("Use YCrCb SD")) ;
			*eColorimetry = BAVC_MatrixCoefficients_eSmpte_170M ;   /* SD Colorspace */
			break ;

		default : /* all others */
			BDBG_MSG(("Use YCrCb HD")) ;
			*eColorimetry = BAVC_MatrixCoefficients_eItu_R_BT_709 ; /* HD Colorspace */
		}
	}
#if BDBG_DEBUG_BUILD
	else
		BDBG_MSG(("DVI Monitors/Mode support RGB Only")) ;
#endif


	/* use full range rgb for DVI and HDMI 640x480 */
	if (*eColorimetry == BAVC_MatrixCoefficients_eHdmi_RGB)
	{
		switch (eVideoFmt)
		{
		default :
			BDBG_MSG(("Use Limited Range RGB")) ;
			/* keep limited range RGB */
			break ;

		case BFMT_VideoFmt_eDVI_640x480p  :
		case BFMT_VideoFmt_eDVI_800x600p  :
		case BFMT_VideoFmt_eDVI_1024x768p :
		case BFMT_VideoFmt_eDVI_1280x768p :
		case BFMT_VideoFmt_eDVI_1280x720p_50Hz :
		case BFMT_VideoFmt_eDVI_1280x720p :
		case BFMT_VideoFmt_eDVI_1280x720p_ReducedBlank :
			BDBG_MSG(("Use Full Range RGB")) ;
			*eColorimetry = BAVC_MatrixCoefficients_eDvi_Full_Range_RGB ;
			break ;
		}
	}

done:
	*eColorimetry =
		BAVC_GetDefaultMatrixCoefficients_isrsafe(eVideoFmt, false);
	return rc  ;
}


/******************************************************************************
BERR_Code BHDM_EDID_GetVideoCapabilityDB
Summary: Retrieve Rx Video Capabilities stored in the EDID's Video Capability Data Block
*******************************************************************************/
BERR_Code BHDM_EDID_GetVideoCapabilityDB(
	const BHDM_Handle hHDMI,
	BHDM_EDID_VideoCapabilityDataBlock *pVideoCapabilityDataBlock)
{
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(pVideoCapabilityDataBlock, 0, sizeof(BHDM_EDID_VideoCapabilityDataBlock)) ;
	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	BKNI_Memcpy(pVideoCapabilityDataBlock, &hHDMI->AttachedEDID.VideoCapabilityDB,
		sizeof(BHDM_EDID_VideoCapabilityDataBlock)) ;

done:
	return rc  ;
}


/******************************************************************************
BERR_Code BHDM_EDID_GetColorimetryDB
Summary: Retrieve colorimetry info stored in the EDID's Colorimetry Data Block
*******************************************************************************/
BERR_Code BHDM_EDID_GetColorimetryDB(
	const BHDM_Handle hHDMI,
	BHDM_EDID_ColorimetryDataBlock *pColorimetryDataBlock)
{
	BERR_Code rc = BERR_SUCCESS ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	BKNI_Memcpy(pColorimetryDataBlock, &hHDMI->AttachedEDID.ColorimetryDB,
		sizeof(BHDM_EDID_ColorimetryDataBlock)) ;

done:
	return rc  ;
}

/******************************************************************************
BERR_Code BHDM_EDID_GetSupportedColorDepth
Summary: Retrieve a copy of the Monitor Name stored in the EDID
*******************************************************************************/
BERR_Code BHDM_EDID_GetSupportedColorDepth(
	const BHDM_Handle hHDMI,
	BHDM_EDID_ColorDepth *stSupportedColorDepth,	/* [out] */
	bool *bYCbCrPixelEncoding 	/* [out] */
)
{
	BERR_Code rc = BERR_SUCCESS;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	stSupportedColorDepth->bColorDepth24bit = true; /* standard color depth, always supported */
	stSupportedColorDepth->bColorDepth30bit =
		hHDMI->AttachedEDID.RxVSDB.DeepColor_30bit ? true : false;
	stSupportedColorDepth->bColorDepth36bit =
		hHDMI->AttachedEDID.RxVSDB.DeepColor_36bit ? true : false;
	stSupportedColorDepth->bColorDepth48bit =
		hHDMI->AttachedEDID.RxVSDB.DeepColor_48bit ? true : false;

	*bYCbCrPixelEncoding = hHDMI->AttachedEDID.RxVSDB.DeepColor_Y444 ? true : false;

done:
	return rc;
}



/******************************************************************************
BERR_Code BHDM_EDID_GetMyCecPhysicalAddr
Summary: Retrieve CEC Physical info stored in the EDID
*******************************************************************************/
BERR_Code BHDM_EDID_GetMyCecPhysicalAddr(
	const BHDM_Handle hHDMI,
	uint8_t *pMyPhysicalAddr)
{
	BERR_Code rc = BERR_SUCCESS ;

	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	if (hHDMI->edidStatus != BHDM_EDID_STATE_eOK)
	{
		rc = BERR_TRACE(BHDM_EDID_NOT_FOUND) ;
		goto done ;
	}

	pMyPhysicalAddr[0] = hHDMI->AttachedEDID.RxVSDB.PhysAddr_A;
	pMyPhysicalAddr[0] <<= 4;
	pMyPhysicalAddr[0] |= hHDMI->AttachedEDID.RxVSDB.PhysAddr_B;

	pMyPhysicalAddr[1] = hHDMI->AttachedEDID.RxVSDB.PhysAddr_C;
	pMyPhysicalAddr[1] <<= 4;
	pMyPhysicalAddr[1] |= hHDMI->AttachedEDID.RxVSDB.PhysAddr_D;

done:
	return rc  ;
}

/******************************************************************************
BERR_Code BHDM_EDID_GetHdrStaticMetadatadb
Summary: Retrieve the HDR Data Block from the first Version 3 Timing Extension in the EDID
*******************************************************************************/
BERR_Code BHDM_EDID_GetHdrStaticMetadatadb(
   const BHDM_Handle hHDMI,                  /* [in] HDMI handle */
   BHDM_EDID_HDRStaticDB *RxHdrDB  /* [out] pointer to HDR Data
                                            Block to hold the retrieved data */
)
{
	uint8_t RxDeviceAttached ;
	BERR_Code rc = BERR_SUCCESS ;


	BDBG_ENTER(BHDM_EDID_GetHdmiHdrdb) ;
	BDBG_OBJECT_ASSERT(hHDMI, HDMI) ;

	BKNI_Memset(RxHdrDB, 0, sizeof(BHDM_EDID_HDRStaticDB)) ;

	/* make sure HDMI Cable is connected to something... */
	BHDM_CHECK_RC(rc, BHDM_RxDeviceAttached(hHDMI, &RxDeviceAttached));
	if (!RxDeviceAttached)
	{
		rc = BHDM_NO_RX_DEVICE ;
		goto done ;
	}

	if (hHDMI->edidStatus == BHDM_EDID_STATE_eInvalid) {
		BDBG_WRN(("No Valid EDID Found..."));
		goto done;
	}

	if (!hHDMI->AttachedEDID.RxHasHdmiSupport)
		goto done ;

	/* EDID processed at initialization; copy information */
	BKNI_Memcpy(RxHdrDB,  &hHDMI->AttachedEDID.HdrDB,
		sizeof(BHDM_EDID_HDRStaticDB)) ;

done:
	BDBG_LEAVE(BHDM_EDID_GetHdmiHdrdb) ;
	return 	rc ;
}
