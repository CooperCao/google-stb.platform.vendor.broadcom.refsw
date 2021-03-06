/***************************************************************************
 *     Copyright (c) 2003-2013, Broadcom Corporation
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
#ifndef BBOX_VDC_H__
#define BBOX_VDC_H__

#include "bstd.h"
#include "berr_ids.h"    /* Error codes */
#include "bbox_vdc_priv.h"
#include "bfmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: This is only used in VDC. See usage below. */
#define BBOX_VDC_DISREGARD                     0x7FFF

/* This needs to be updated if the latest chip has more than this */
#define BBOX_VDC_DEINTERLACER_COUNT            6

/* This is the max number of video windows a display may have. */
#define BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY     (2)

/* This is the max number of graphics windows a display may have. */
#define BBOX_VDC_GFX_WINDOW_COUNT_PER_DISPLAY       (3)

/* This is the max number of windows a display may have. */
#define BBOX_VDC_WINDOW_COUNT_PER_DISPLAY         \
	(BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY + BBOX_VDC_GFX_WINDOW_COUNT_PER_DISPLAY)

/* This needs to be updated if the display ID enum in VDC is updated. */
#define BBOX_VDC_DISPLAY_COUNT                 7

/* This needs to be updated if the number of STG displays in VDC is updated. */
#define BBOX_VDC_STG_DISPLAY_COUNT             6

#define BBOX_FTR_SD       (0)
#define BBOX_FTR_HD       (BBOX_Vdc_Resource_eHd)
#define BBOX_FTR_M0       (BBOX_Vdc_Resource_eMem0)
#define BBOX_FTR_M1       (BBOX_Vdc_Resource_eMem1)
#define BBOX_FTR_M2       (BBOX_Vdc_Resource_eMem2)
#define BBOX_FTR_M01      (BBOX_Vdc_Resource_eMem0 | BBOX_Vdc_Resource_eMem1)
#define BBOX_FTR_HD_M0    (BBOX_Vdc_Resource_eMem0 | BBOX_Vdc_Resource_eHd)
#define BBOX_FTR_HD_M1    (BBOX_Vdc_Resource_eMem1 | BBOX_Vdc_Resource_eHd)
#define BBOX_FTR_HD_M2    (BBOX_Vdc_Resource_eMem2 | BBOX_Vdc_Resource_eHd)
#define BBOX_FTR_HD_MR0   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr0)
#define BBOX_FTR_HD_MR1   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr1)
#define BBOX_FTR_HD_MR2   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr2)
#define BBOX_FTR_HD_MR3   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr3)
#define BBOX_FTR_HD_MR4   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr4)
#define BBOX_FTR_HD_MR5   (BBOX_Vdc_Resource_eHd | BBOX_Vdc_Resource_eMadr5)


#define BBOX_FTR_INVALID  (BBOX_Vdc_Resource_eInvalid)
#define BBOX_INVALID_NUM_MEMC 0xFFFFFFFF

/* Macro for RDC memc index table entry */
#define BBOX_MK_RDC_MEMC_IDX(MemcIdxRdc)   (BBOX_MemcIndex_##MemcIdxRdc)

/* Macro for video window capture memc index table entry */
#define BBOX_MK_VID_WIN_CAP_MEMC_IDX(MemcIdxW0, MemcIdxW1)   \
{ \
	BBOX_MemcIndex_##MemcIdxW0, \
	BBOX_MemcIndex_##MemcIdxW1  \
}

/* Macro for video window deinterlacer memc index table entry */
#define BBOX_MK_VID_WIN_MAD_MEMC_IDX(MemcIdxMad0, MemcIdxMad1)   \
{ \
	BBOX_MemcIndex_##MemcIdxMad0, \
	BBOX_MemcIndex_##MemcIdxMad1  \
}

/* Macro for graphics window memc index table entry.
*  G1 and G2 are not used for now */
#define BBOX_MK_GFD_WIN_MEMC_IDX(MemcIdxG0)   \
{ \
	BBOX_MemcIndex_##MemcIdxG0, \
	BBOX_MemcIndex_Invalid,     \
	BBOX_MemcIndex_Invalid      \
}

/* Macro for video and graphics window memc index table entry */
#define BBOX_MK_WIN_MEMC_IDX(MemcIdxW0, MemcIdxW1, MemcIdxMad0, MemcIdxMad1, MemcIdxG0)   \
{ \
	BBOX_MK_VID_WIN_CAP_MEMC_IDX(MemcIdxW0, MemcIdxW1), \
	BBOX_MK_VID_WIN_MAD_MEMC_IDX(MemcIdxMad0, MemcIdxMad1), \
	BBOX_MK_GFD_WIN_MEMC_IDX(MemcIdxG0)               \
}

typedef enum
{
	BBOX_Vdc_SclCapBias_eAuto = 0,
	BBOX_Vdc_SclCapBias_eSclBeforeCap,
	BBOX_Vdc_SclCapBias_eSclAfterCap,
	BBOX_Vdc_SclCapBias_eAutoDisable,	                 /* Disables CAP when possible and selects SCL placement based on bandwidth */
	BBOX_Vdc_SclCapBias_eDisregard = BBOX_VDC_DISREGARD  /* VDC default is selected */
} BBOX_Vdc_SclCapBias;

typedef enum
{
	BBOX_Vdc_Bpp_e8bit,
	BBOX_Vdc_Bpp_e10bit,
	BBOX_Vdc_Bpp_e12bit,
	BBOX_Vdc_Bpp_eDisregard = BBOX_VDC_DISREGARD
} BBOX_Vdc_Bpp;

typedef enum
{
	BBOX_Vdc_Colorspace_eRGB = 0,
	BBOX_Vdc_Colorspace_eYCbCr422,
	BBOX_Vdc_Colorspace_eYCbCr444,
	BBOX_Vdc_Colorspace_eYCbCr420,
	BBOX_Vdc_Colorspace_eFuture,
	BBOX_Vdc_Colorspace_eDisregard = BBOX_VDC_DISREGARD
} BBOX_Vdc_Colorspace;

typedef struct
{
	uint32_t  ulMad;
} BBOX_Vdc_ResourceFeature;

typedef struct
{
	uint32_t ulWidth;
	uint32_t ulHeight;
} BBOX_Vdc_PictureSizeLimits;

typedef struct
{
	uint32_t ulWidthFraction;       /* this is a fraction of the full-screen width. */
	uint32_t ulHeightFraction;      /* this is a fraction of the full-screen height */
} BBOX_Vdc_WindowSizeLimits;

/***************************************************************************
Summary:
	This structure describes box capabilities for VDC sources. Only GFX and
	HD-DVI sources are supported by BOX's VDC sub-module. MPEG feeders
	would be in BOX's XVD sub-module.

Description:
	bAvailable   - specifies if source is available
	bMtgCapable  - only applies to Mpeg Feeders (MFD) and specifies if MFD
				   is MTG-capable
	stSizeLimits - specifies the source's frame buffer size
	eColorSpace - specifies source's colorspace
	eBpp - specifies number of bits per pixel
See Also:
***************************************************************************/
typedef struct
{
	bool                       bAvailable;
	bool                       bMtgCapable;
	BBOX_Vdc_PictureSizeLimits stSizeLimits;
	BBOX_Vdc_Colorspace        eColorSpace;
	BBOX_Vdc_Bpp               eBpp;
} BBOX_Vdc_Source_Capabilities;

/***************************************************************************
Summary:
	This structure describes box capabilities for VDC windows.

Description:
	bAvailable   - specifies if window is available
	stSizeLimits - specifies the window size based on a fraction of the
				   full-screen size. For example, 1/2 of full-screen height
				   and 1/2 of full-screen width. To use, simply divide the
				   fullscreen size by this number to determine the
				   maximum size for the given window.
	stAvailableResource - indicates shared resource that is available
	bTntAvailable - indicates if TNT is available for the given window
	bAnrAvailable - indicates if a ANR is available for the given window

See Also:
***************************************************************************/
typedef struct
{
	bool                       bAvailable;
	BBOX_Vdc_WindowSizeLimits  stSizeLimits;
	BBOX_Vdc_ResourceFeature   stResource;
	BBOX_Vdc_SclCapBias        eSclCapBias;
} BBOX_Vdc_Window_Capabilities;

/***************************************************************************
Summary:
	This structure describes the matching of a STG display and an encoder that
	is used with the display

Description:
	bAvailable             - specifies if a STG display and its associated encoder are
				             available
	ulStgId                - the paired STG display
	ulEncoderCoreId        - the paired encoder
	ulEncoderChannel       - the encoder channel to use

See Also:
***************************************************************************/
typedef struct
{
	bool                       bAvailable;
	uint32_t                   ulStgId;
	uint32_t                   ulEncoderCoreId;
	uint32_t                   ulEncoderChannel;
} BBOX_Vdc_StgEncoderPair;

/***************************************************************************
Summary:
	This structure describes box capabilities for VDC displays.

Description:
	bAvailable   - specifies if window is available
	stWindow     - This specifies the capabilities available for a given window
				   of a given display. There are 2 windows per display.
	stStgEnc     - specifies the STG display and encoder pair to the given display
	eMaxVideoFmt - specifies the maximum BVN frame buffer resolution
	eMaxHdmiTxVideoFmt - specifies the maximum display resolution for HDMI display.
						 This may have the same value as eMaxVideoFmt. If different,
						 this indicates that the HDMI display can be scaled up to the
						 specified eMaxHdmiTxVideoFmt. For example, if eMaxVideoFmt
						 is 1080p and eMaxHdmiTxVideoFmt is 3840x2160p, the HDMI
						 display format can be scaled up to 3840x2160p.

See Also:
***************************************************************************/
typedef struct BBOX_Vdc_Display_Capabilities
{
	bool                         bAvailable;
	BBOX_Vdc_Window_Capabilities astWindow[BBOX_VDC_WINDOW_COUNT_PER_DISPLAY];
	BBOX_Vdc_StgEncoderPair      stStgEnc;
	BFMT_VideoFmt                eMaxVideoFmt;
	BFMT_VideoFmt                eMaxHdmiDisplayFmt;
} BBOX_Vdc_Display_Capabilities;

/***************************************************************************
Summary:
	List of VDC's exposed capabilities
****************************************************************************/
typedef struct BBOX_Vdc_Capabilities
{
	/* Indicates if a deinterlacer is limited by the given height and width.
	   An element of the array corresponds to a deinterlacer, eg., element 0
	   corresponds to deinterlacer 0. BBOX_VDC_DISREGARD means the limits
	   used are imposed by VDC on a per deinterlacer basis. */
	BBOX_Vdc_PictureSizeLimits	astDeinterlacerLimits[BBOX_VDC_DEINTERLACER_COUNT];

	/* Indicates the horizontal size threshold that will trigger the use of the
	   HSCL before deinterlacing. In other words, a source with a horizontal
	   size greater or equal to this threshold will be horintally scaled by the
	   HSCL first prior to deinterlacing. BBOX_VDC_DISREGARD  means there is
	   no threshold and therefore no HSCL is used. */
	uint32_t aulDeinterlacerHsclThreshold[BBOX_VDC_DEINTERLACER_COUNT];

	/* Indicates the number of available CAP/VFD for transcode. 0 means
	   there is no bandwidth allocated for capture on any transcode path.
	   BBOX_VDC_DISREGARD means all available CAP/VFD can be used. */
	uint32_t  ulNumXcodeCapVfd;

	/* Indicates the number of available GFD for transcode. 0 means
	   there is no bandwidth allocated for capture on any transcode path.
	   BBOX_VDC_DISREGARD means all available CAP/VFD can be used. */
	uint32_t  ulNumXcodeGfd;

	/* This specifies the capabilities available for a given display. Each element
	   of the array maps directly to a VDC display ID enum. */
	BBOX_Vdc_Display_Capabilities astDisplay[BBOX_VDC_DISPLAY_COUNT];

	/* This specifies the capabilities available for a given source. Note that
	   only GFX and HD-DVI are have imposed VDC BOX limits. The rest are
	   disregarded by VDC. */
	BBOX_Vdc_Source_Capabilities astSource[BAVC_SourceId_eMax];

} BBOX_Vdc_Capabilities;

/***************************************************************************
Summary:
	This structure describes memc index settings for VDC.

Description:

See Also:
	BBOX_MemConfig, BBOX_GetMemConfig, BBOX_Vdc_Display_MemcIndexSettings
****************************************************************************/
typedef struct BBOX_Vdc_MemcIndexSettings
{
	uint32_t     ulRdcMemcIndex;

	struct {
		/* Memc Index for video windows */
		uint32_t aulVidWinCapMemcIndex[BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY];

		/* Memc Index for deinterlacer on video windows */
		uint32_t aulVidWinMadMemcIndex[BBOX_VDC_VIDEO_WINDOW_COUNT_PER_DISPLAY];

		/* Memc Index for graphics windows */
		uint32_t aulGfdWinMemcIndex[BBOX_VDC_GFX_WINDOW_COUNT_PER_DISPLAY];

	} astDisplay[BBOX_VDC_DISPLAY_COUNT];

} BBOX_Vdc_MemcIndexSettings;


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BBOX_VDC_H__ */

/* end of file */
