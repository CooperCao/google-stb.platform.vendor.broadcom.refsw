/***************************************************************************
 *     Copyright (c) 2003-2012, Broadcom Corporation
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
 *   Video format module header file
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#ifndef BFMT_H__
#define BFMT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*=************************ Module Overview ********************************
The purpose of the VideoFormatModule is to provide common Video formats
information to other Software Modules. It does not require any chip specific
data, and therefore does not require a handle.

This module defines all possible Video formats, and allows the user to get
fixed information about the format such as: width, height, top porch, bottom
porch, front porch, back porch, whether the format is progressive, etc. These
sort of functions are required by most video applications and by graphics.
These values are constant from chip to chip. The included functions do no
access any memory or registers.
**************************************************************************=*/

/****************************************************************
 *  Macros
 ****************************************************************/

/* KLUDGE: legacy 3DTV chipsets have picture size limitation, require special
   display format to support */
#if (3548 == BCHP_CHIP) || (3556 == BCHP_CHIP)
#define BFMT_LEGACY_3DTV_SUPPORT            (1)
#else
#define BFMT_LEGACY_3DTV_SUPPORT            (0)
#endif

/* This is a bit of a kludge. There is not a syntax for 64-bit literals that is
 * accepted by all compilers */
#define BFMT_SHIFT32(a) ((uint64_t)(a) << 32)

/* The vertical refresh rate and pixel frequency scaling factor:
    For example, NTSC has vsync rate of 59.94Hz, its stored ulVertFreq value is
		59.94 x BFMT_FREQ_FACTOR = 5994;
    720p has pixel frequency of 74.25MHz, its stored ulPxlFreq value is
		74.25 x BFMT_FREQ_FACTOR = 7425; */
#define BFMT_FREQ_FACTOR        (100)

/* To be OBSOLETED: These masks are hitting the limitation of as new format
 * growth.  These are to be replace with BFMT_Vert_e*. */
/* The bit masks for picture vertical frequency, or refresh rate */
#define BFMT_VERT_INVALID            0x00000000
#define BFMT_VERT_50Hz               0x00000001
#define BFMT_VERT_59_94Hz            0x00000002
#define BFMT_VERT_60Hz               0x00000004
#define BFMT_VERT_70Hz               0x00000008
#define BFMT_VERT_72Hz               0x00000010
#define BFMT_VERT_75Hz               0x00000020
#define BFMT_VERT_85Hz               0x00000040
#define BFMT_VERT_23_976Hz           0x00000080
#define BFMT_VERT_24Hz               0x00000100
#define BFMT_VERT_25Hz               0x00000200
#define BFMT_VERT_29_97Hz            0x00000400
#define BFMT_VERT_30Hz               0x00000800
#define BFMT_VERT_56Hz               0x00001000
#define BFMT_VERT_87Hz               0x00002000
#define BFMT_VERT_48Hz               0x00004000
#define BFMT_VERT_47_952Hz           0x00008000
#define BFMT_VERT_66Hz               0x00010000
#define BFMT_VERT_12_5Hz             0x00020000
#define BFMT_VERT_14_985Hz           0x00040000
#define BFMT_VERT_15Hz               0x00080000
#define BFMT_VERT_20Hz               0x00100000
#define BFMT_VERT_100Hz              0x00200000
#define BFMT_VERT_119_88Hz           0x00400000
#define BFMT_VERT_120Hz              0x00800000

/* To be OBSOLETED: These masks are hitting the limitation of as new format
 * growth.  These are to be replace with BFMT_Pxl_e*. */
/* The bit masks for video pixel frequency, or sample rate */
/* Safe mode: 640x480p */
#define BFMT_PXL_25_2MHz             0x00000001
#define BFMT_PXL_25_2MHz_MUL_1_001   0x00000000 /* not used */
#define BFMT_PXL_25_2MHz_DIV_1_001   0x00000002

/* NTSC and 480p, NTSC is upsampled in VEC from 13.5MHz */
#define BFMT_PXL_27MHz               0x00000004
#define BFMT_PXL_27MHz_MUL_1_001     0x00000008 /* 480p @60Hz */
#define BFMT_PXL_27MHz_DIV_1_001     0x00000000 /* not used */

/* 1080i and 720p */
#define BFMT_PXL_74_25MHz            0x00000010
#define BFMT_PXL_74_25MHz_MUL_1_001  0x00000000 /* not used */
#define BFMT_PXL_74_25MHz_DIV_1_001  0x00000020

/* 1080p60 3D or 1080p100/120
 * Reuse bits. BFMT_PXL_39_79MHz and BFMT_PXL_39_79MHz_DIV_1_001
 * are used for input only */
#define BFMT_PXL_297MHz              0x00000040 /* 60    Hz */
#define BFMT_PXL_297MHz_DIV_1_001    0x00000080 /* 59.94 Hz */

/* VESA mode: 1024x768p @ 60/59.94Hz */
#define BFMT_PXL_65MHz               0x00000100
#define BFMT_PXL_65MHz_DIV_1_001     0x00000200

/* VESA mode: 1280x720p @ 50Hz */
#define BFMT_PXL_60_4656MHz          0x00000400
#define BFMT_PXL_60_375MHz           0x00000000 /* not used */

/* 480P output, with 54 MHz pixel sampling with pixel doubling */
#define BFMT_PXL_54MHz               0x00000800
#define BFMT_PXL_54MHz_MUL_1_001     0x00001000

/* VESA mode: 800x600p @ 60/59.94Hz */
#define BFMT_PXL_40MHz               0x00000000 /* not used */
#define BFMT_PXL_39_79MHz            0x00002000 /* 60    Hz */
#define BFMT_PXL_39_79MHz_DIV_1_001  0x00004000 /* 59.94 Hz */

/* VESA mode: 640x480p_CVT @ 60Hz */
#define BFMT_PXL_23_75MHz            0x00008000
#define BFMT_PXL_23_75MHz_DIV_1_001  0x00010000

/* VESA mode: 1280x800p @ 60Hz */
#define BFMT_PXL_83_5MHz             0x00020000
#define BFMT_PXL_83_5MHz_DIV_1_001   0x00040000

/* VESA mode: 1280x1024p @ 60Hz */
#define BFMT_PXL_108MHz              0x00080000
#define BFMT_PXL_108MHz_DIV_1_001    0x00100000

/* VESA mode: 1440x900p @ 60Hz */
#define BFMT_PXL_106_5MHz            0x00200000
#define BFMT_PXL_106_5MHz_DIV_1_001  0x00400000

/* VESA mode: 1360x768p @ 60Hz */
#define BFMT_PXL_85_5MHz             0x00800000
#define BFMT_PXL_85_5MHz_DIV_1_001   0x01000000

/* 1080p */
#define BFMT_PXL_148_5MHz            0x02000000 /* 60Hz */
#define BFMT_PXL_148_5MHz_DIV_1_001  0x04000000 /* 59.94 Hz */

/* 1600x1200p_60Hz */
#define BFMT_PXL_162MHz              0x08000000 /* 60Hz */

/* These are low priority, using the 4 msb bits as count instead of mask */
/* DTV: 1366x768p  */
#define	BFMT_PXL_56_304MHz           0x10000000 /* 50 Hz */
#define BFMT_PXL_67_565MHz           0x20000000 /* 60 Hz    */
#define BFMT_PXL_67_565MHz_MUL_1_001 0x00000000 /* not used */
#define BFMT_PXL_67_565MHz_DIV_1_001 0x40000000 /* 59.94 Hz */

/* VESA mode: 1280x720p @ 60/59.94Hz Reduced Blanking */
#define BFMT_PXL_64MHz               0x00000000 /* not used */
#define BFMT_PXL_64_022MHz           0x80000000
#define BFMT_PXL_64_022MHz_DIV_1_001 BFMT_SHIFT32(0x00000001)

/* VESA mode: 1280x768p @ 60/59.94Hz */
#define BFMT_PXL_65_286MHz           BFMT_SHIFT32(0x00000002)
#define BFMT_PXL_65_286MHz_DIV_1_001 BFMT_SHIFT32(0x00000004)

/* 4k2k@50/60 */
#define BFMT_PXL_594MHz              BFMT_SHIFT32(0x00000008)
#define BFMT_PXL_594MHz_DIV_1_001    BFMT_SHIFT32(0x00000010)

/* 640x480 VESA */
#define BFMT_PXL_75_5_MHz            BFMT_SHIFT32(0x00000020)
#define BFMT_PXL_75_5_MHz_DIV_1_001  BFMT_SHIFT32(0x00000040)

/* 480p, 1440x480i, 576p, 144x576i MHL */
#define BFMT_PXL_81_0MHz             BFMT_SHIFT32(0x00000080)
#define BFMT_PXL_81_0MHz_MUL_1_001   BFMT_SHIFT32(0x00000100)

/* 720p, 1080i, 1080p MHL */
#define BFMT_PXL_222_75MHz           BFMT_SHIFT32(0x00000200)
#define BFMT_PXL_222_75MHz_DIV_1_001 BFMT_SHIFT32(0x00000400)

/* VESA mode: 1280x720p @ 60/59.94Hz */
#define BFMT_PXL_74_375MHz           0x00000000 /* not used */
#define BFMT_PXL_74_48MHz            0x00000000 /* not used */
#define BFMT_PXL_74_48MHz_DIV_1_001  0x00000000 /* not used */

/* More PC mode pixel rate */
#define BFMT_PXL_31_50MHz            0x00000000 /* not used */
#define BFMT_PXL_35_50MHz            0x00000000 /* not used */
#define BFMT_PXL_36_00MHz            0x00000000 /* not used */
#define BFMT_PXL_49_50MHz            0x00000000 /* not used */
#define BFMT_PXL_50_00MHz            0x00000000 /* not used */
#define BFMT_PXL_56_25MHz            0x00000000 /* not used */
#define BFMT_PXL_75_00MHz            0x00000000 /* not used */
#define BFMT_PXL_78_75MHz            0x00000000 /* not used */
#define BFMT_PXL_94_50MHz            0x00000000 /* not used */

#define BFMT_PXL_101MHz              0x00000000 /* not used */
#define BFMT_PXL_121_75MHz           0x00000000 /* not used */
#define BFMT_PXL_156MHz              0x00000000 /* not used */

/* VESA mode: 1920x1080p@60Hz_Red */
#define BFMT_PXL_138_625MHz          0x00000000 /* not used */

/* 1366x768@60 */
#define BFMT_PXL_72_014MHz           0x00000000 /* not used */

/* 1024x768i@87 */
#define BFMT_PXL_44_900MHz           0x00000000 /* not used */

/* Check with HDM PI owner before modifying BFMT_IS_VESA_MODE and
 * BFMT_SUPPORT_HDMI lists */
#define BFMT_IS_VESA_MODE(fmt) \
	((BFMT_VideoFmt_eDVI_640x480p == (fmt)) || \
	 (BFMT_VideoFmt_eDVI_800x600p == (fmt)) || \
	 (BFMT_VideoFmt_eDVI_1024x768p == (fmt)) || \
	 (BFMT_VideoFmt_eDVI_1280x768p == (fmt)) || \
	 (BFMT_VideoFmt_eDVI_1280x720p == (fmt)) || \
	 (BFMT_VideoFmt_eDVI_640x480p_CVT == (fmt)) || \
	 (BFMT_VideoFmt_eDVI_1280x1024p_60Hz == (fmt)) || \
	 (BFMT_VideoFmt_eDVI_1360x768p_60Hz == (fmt)) || \
	 (BFMT_VideoFmt_eDVI_1366x768p_60Hz == (fmt)))

#define BFMT_SUPPORT_HDMI(fmt) \
	((BFMT_VideoFmt_eNTSC == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_B == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_B1 == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_D == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_D1 == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_G == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_H == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_I == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_K == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_M == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_N == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_NC == (fmt)) || \
	 (BFMT_VideoFmt_e1080i == (fmt)) || \
	 (BFMT_VideoFmt_e1080p == (fmt)) || \
	 (BFMT_VideoFmt_e720p == (fmt)) || \
	 (BFMT_VideoFmt_e720p_60Hz_3DOU_AS == (fmt)) || \
	 (BFMT_VideoFmt_e720p_50Hz_3DOU_AS == (fmt)) || \
	 (BFMT_VideoFmt_e480p == (fmt)) || \
	 (BFMT_VideoFmt_e576p_50Hz == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_24Hz_3DOU_AS == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_30Hz_3DOU_AS == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_60Hz_3DOU_AS == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_60Hz_3DLR == (fmt)) || \
	 (BFMT_VideoFmt_e3840x2160p_24Hz == (fmt)) || \
	 (BFMT_VideoFmt_e3840x2160p_25Hz == (fmt)) || \
	 (BFMT_VideoFmt_e3840x2160p_30Hz == (fmt)) || \
	 (BFMT_VideoFmt_e3840x2160p_50Hz == (fmt)) || \
	 (BFMT_VideoFmt_e3840x2160p_60Hz == (fmt)) || \
	 (BFMT_VideoFmt_e4096x2160p_24Hz == (fmt)) || \
	 (BFMT_VideoFmt_e4096x2160p_25Hz == (fmt)) || \
	 (BFMT_VideoFmt_e4096x2160p_30Hz == (fmt)) || \
	 (BFMT_VideoFmt_e4096x2160p_50Hz == (fmt)) || \
	 (BFMT_VideoFmt_e4096x2160p_60Hz == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_24Hz == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_25Hz == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_30Hz == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_50Hz == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_100Hz == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_120Hz == (fmt)) || \
	 (BFMT_VideoFmt_e1080i_50Hz == (fmt)) || \
	 (BFMT_VideoFmt_e720p_24Hz == (fmt)) || \
	 (BFMT_VideoFmt_e720p_30Hz == (fmt)) || \
	 (BFMT_VideoFmt_e720p_50Hz == (fmt)) || \
	 (BFMT_VideoFmt_eCUSTOM_1366x768p == (fmt)) || \
	 (BFMT_VideoFmt_e720x482_NTSC == (fmt)) || \
	 (BFMT_VideoFmt_e720x482_NTSC_J == (fmt)) || \
	 (BFMT_VideoFmt_e720x483p == (fmt)) || \
	 (BFMT_IS_VESA_MODE(fmt)))

#define BFMT_IS_3D_MODE(fmt) \
	((BFMT_VideoFmt_e720p_60Hz_3DOU_AS  == (fmt)) || \
	 (BFMT_VideoFmt_e720p_50Hz_3DOU_AS  == (fmt)) || \
	 (BFMT_VideoFmt_e720p_30Hz_3DOU_AS  == (fmt)) || \
	 (BFMT_VideoFmt_e720p_24Hz_3DOU_AS  == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_24Hz_3DOU_AS == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_30Hz_3DOU_AS == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_60Hz_3DOU_AS == (fmt)) || \
	 (BFMT_VideoFmt_e1080p_60Hz_3DLR == (fmt)))

#define BFMT_IS_UHD(fmt) \
	((BFMT_VideoFmt_e3840x2160p_24Hz <= (fmt)) && \
	 ((fmt) <= BFMT_VideoFmt_e4096x2160p_60Hz))

/***************************************************************************
Summary:
	This macro are commonly used to described a format.

See Also:
	BFMT_GetVideoFormatInfoPtr_isr
***************************************************************************/
#define BFMT_NTSC_WIDTH                (720)
#define BFMT_NTSC_HEIGHT               (480)

#define BFMT_PAL_WIDTH                 (720)
#define BFMT_PAL_HEIGHT                (576)

#define BFMT_PAL_M_WIDTH               BFMT_NTSC_WIDTH    /* Yes! */
#define BFMT_PAL_M_HEIGHT              BFMT_NTSC_HEIGHT

#define BFMT_PAL_N_WIDTH               BFMT_PAL_WIDTH
#define BFMT_PAL_N_HEIGHT              BFMT_PAL_HEIGHT

#define BFMT_PAL_NC_WIDTH              BFMT_PAL_WIDTH
#define BFMT_PAL_NC_HEIGHT             BFMT_PAL_HEIGHT

#define BFMT_SECAM_WIDTH               BFMT_PAL_WIDTH
#define BFMT_SECAM_HEIGHT              BFMT_PAL_HEIGHT

#define BFMT_DVI_480P_WIDTH            (640)
#define BFMT_DVI_480P_HEIGHT           (480)

#define BFMT_DVI_600P_WIDTH            (800)
#define BFMT_DVI_600P_HEIGHT           (600)

#define BFMT_DVI_768P_WIDTH            (1024)
#define BFMT_DVI_768P_HEIGHT           (768)

#define BFMT_DVI_720P_WIDTH            (1280)
#define BFMT_DVI_720P_HEIGHT           (720)

#define BFMT_480P_WIDTH                BFMT_NTSC_WIDTH
#define BFMT_480P_HEIGHT               (480)

#define BFMT_576P_WIDTH                BFMT_PAL_WIDTH
#define BFMT_576P_HEIGHT               BFMT_PAL_HEIGHT

#define BFMT_720P_WIDTH                (1280)
#define BFMT_720P_HEIGHT               (720)

#define BFMT_1080I_WIDTH               (1920)
#define BFMT_1080I_HEIGHT              (1080)

#define BFMT_1080P_WIDTH               BFMT_1080I_WIDTH
#define BFMT_1080P_HEIGHT              BFMT_1080I_HEIGHT

#define BFMT_1080P3D_HEIGHT            (1125+1080)
#define BFMT_720P3D_HEIGHT             (750+720)

#define BFMT_2160P_HEIGHT              (2160)

/***************************************************************************
Summary:
	Used to specify the possible aspect ratios.

Description:
	The values assigned to these enumerations should be kept in step with
	the ISO 13818-2 specification to minimize effort converting to these
	types.

See Also:
****************************************************************************/
typedef enum
{
	BFMT_AspectRatio_eUnknown   = 0, /* Unkown/Reserved */
	BFMT_AspectRatio_eSquarePxl = 1, /* square pixel */
	BFMT_AspectRatio_e4_3          , /* 4:3 */
	BFMT_AspectRatio_e16_9         , /* 16:9 */
	BFMT_AspectRatio_e221_1        , /* 2.21:1 */
	BFMT_AspectRatio_e15_9,          /* 15:9 */
	BFMT_AspectRatio_eSAR            /* no DAR available, use SAR instead */

} BFMT_AspectRatio;

/***************************************************************************
Summary:
	Used to specify orientation of the format.

Description:
	BFMT_Orientation defines all possible orientation of a format.

See Also:
****************************************************************************/
typedef enum
{
	BFMT_Orientation_e2D = 0,         /* 2D */
	BFMT_Orientation_e3D_LeftRight,   /* 3D left right */
	BFMT_Orientation_e3D_OverUnder,   /* 3D over under */
	BFMT_Orientation_e3D_Left,        /* 3D left */
	BFMT_Orientation_e3D_Right,       /* 3D right */
	BFMT_Orientation_eLeftRight_Enhanced  /* multi-resolution 3D */

} BFMT_Orientation;

/***************************************************************************
Summary:
	Used to specify veritical refresh rate (vsync rate) of format.

Description:
	BFMT_Vert defines all possible refresh rate of a format.

See Also:
****************************************************************************/
typedef enum
{
	BFMT_Vert_eInvalid = 0,
	BFMT_Vert_e12_5Hz,
	BFMT_Vert_e14_985Hz,
	BFMT_Vert_e15Hz,
	BFMT_Vert_e20Hz,
	BFMT_Vert_e23_976Hz,
	BFMT_Vert_e24Hz,
	BFMT_Vert_e25Hz,
	BFMT_Vert_e29_97Hz,
	BFMT_Vert_e30Hz,
	BFMT_Vert_e48Hz,
	BFMT_Vert_e50Hz,
	BFMT_Vert_e59_94Hz,
	BFMT_Vert_e60Hz,
	BFMT_Vert_e100Hz,
	BFMT_Vert_e119_88Hz,
	BFMT_Vert_e120Hz,
	BFMT_Vert_eLast

} BFMT_Vert;


/***************************************************************************
Summary:
	This enumeration contains the Video formats available

Description:
	BFMT_VideoFmt defines all possible standard Video formats for HD and
	SD modes. Modes that are supported for DVI outputs only, will have
	the DVI identification.

	BFMT_VideoFmt_e1080i will set 29.97 or 30 Hz, to match with the input
	source. BFMT_VideoFmt_e720p will set 59.94 or 60 Hz, depending on the
	input source.

See Also:
	BVDC_Display_SetVideoFormat, BVDC_Display_GetVideoFormat,
	BFMT_GetVideoFormatInfoPtr_isr
***************************************************************************/
typedef enum
{
	BFMT_VideoFmt_eNTSC = 0,                   /* 480i, NTSC-M for North America */
	BFMT_VideoFmt_eNTSC_J,                     /* 480i (Japan) */
	BFMT_VideoFmt_eNTSC_443,                   /* NTSC-443 */
	BFMT_VideoFmt_ePAL_B,                      /* Australia */
	BFMT_VideoFmt_ePAL_B1,                     /* Hungary */
	BFMT_VideoFmt_ePAL_D,                      /* China */
	BFMT_VideoFmt_ePAL_D1,                     /* Poland */
	BFMT_VideoFmt_ePAL_G,                      /* Europe */
	BFMT_VideoFmt_ePAL_H,                      /* Europe */
	BFMT_VideoFmt_ePAL_K,                      /* Europe */
	BFMT_VideoFmt_ePAL_I,                      /* U.K. */
	BFMT_VideoFmt_ePAL_M,                      /* 525-lines (Brazil) */
	BFMT_VideoFmt_ePAL_N,                      /* Jamaica, Uruguay */
	BFMT_VideoFmt_ePAL_NC,                     /* N combination (Argentina) */
	BFMT_VideoFmt_ePAL_60,                     /* 60Hz PAL */
	BFMT_VideoFmt_eSECAM_L,                    /* France */
	BFMT_VideoFmt_eSECAM_B,                    /* Middle East */
	BFMT_VideoFmt_eSECAM_G,                    /* Middle East */
	BFMT_VideoFmt_eSECAM_D,                    /* Eastern Europe */
	BFMT_VideoFmt_eSECAM_K,                    /* Eastern Europe */
	BFMT_VideoFmt_eSECAM_H,                    /* Line SECAM */
	BFMT_VideoFmt_e1080i,                      /* HD 1080i */
	BFMT_VideoFmt_e1080p,                      /* HD 1080p 60/59.94Hz, SMPTE 274M-1998 */
	BFMT_VideoFmt_e720p,                       /* HD 720p */
	BFMT_VideoFmt_e720p_60Hz_3DOU_AS,          /* HD 3D 720p */
	BFMT_VideoFmt_e720p_50Hz_3DOU_AS,          /* HD 3D 720p50 */
	BFMT_VideoFmt_e720p_30Hz_3DOU_AS,          /* HD 3D 720p30 */
	BFMT_VideoFmt_e720p_24Hz_3DOU_AS,          /* HD 3D 720p24 */
	BFMT_VideoFmt_e480p,                       /* HD 480p */
	BFMT_VideoFmt_e1080i_50Hz,                 /* HD 1080i 50Hz, 1125 sample per line, SMPTE 274M */
	BFMT_VideoFmt_e1080p_24Hz_3DOU_AS,         /* HD 1080p 24Hz, 2750 sample per line, SMPTE 274M-1998 */
	BFMT_VideoFmt_e1080p_30Hz_3DOU_AS,         /* HD 1080p 30Hz, 2200 sample per line, SMPTE 274M-1998 */
	BFMT_VideoFmt_e1080p_60Hz_3DOU_AS,         /* HD 1080p 60Hz, 2200 sample per line  */
	BFMT_VideoFmt_e1080p_60Hz_3DLR,            /* HD 1080p 60Hz, 4400 sample per line  */
	BFMT_VideoFmt_e1080p_24Hz,                 /* HD 1080p 24Hz, 2750 sample per line, SMPTE 274M-1998 */
	BFMT_VideoFmt_e1080p_25Hz,                 /* HD 1080p 25Hz, 2640 sample per line, SMPTE 274M-1998 */
	BFMT_VideoFmt_e1080p_30Hz,                 /* HD 1080p 30Hz, 2200 sample per line, SMPTE 274M-1998 */
	BFMT_VideoFmt_e1080p_50Hz,                 /* HD 1080p 50Hz. */
	BFMT_VideoFmt_e1080p_100Hz,                /* HD 1080p 100Hz. */
	BFMT_VideoFmt_e1080p_120Hz,                /* HD 1080p 120Hz. */
	BFMT_VideoFmt_e1250i_50Hz,                 /* HD 1250i 50Hz, another 1080i_50hz standard SMPTE 295M */
	BFMT_VideoFmt_e720p_24Hz,                  /* HD 720p 23.976/24Hz, 750 line, SMPTE 296M */
	BFMT_VideoFmt_e720p_25Hz,                  /* HD 720p 25Hz, 750 line, SMPTE 296M */
	BFMT_VideoFmt_e720p_30Hz,                  /* HD 720p 30Hz, 750 line, SMPTE 296M */
	BFMT_VideoFmt_e720p_50Hz,                  /* HD 720p 50Hz (Australia) */
	BFMT_VideoFmt_e576p_50Hz,                  /* HD 576p 50Hz (Australia) */
	BFMT_VideoFmt_e240p_60Hz,                  /* NTSC 240p */
	BFMT_VideoFmt_e288p_50Hz,                  /* PAL 288p */
	BFMT_VideoFmt_e1440x480p_60Hz,             /* CEA861B */
	BFMT_VideoFmt_e1440x576p_50Hz,             /* CEA861B */
	BFMT_VideoFmt_e3840x2160p_24Hz,            /* 3840x2160 24Hz */
	BFMT_VideoFmt_e3840x2160p_25Hz,            /* 3840x2160 25Hz */
	BFMT_VideoFmt_e3840x2160p_30Hz,            /* 3840x2160 30Hz */
	BFMT_VideoFmt_e3840x2160p_50Hz,            /* 3840x2160 50Hz */
	BFMT_VideoFmt_e3840x2160p_60Hz,            /* 3840x2160 60Hz */
	BFMT_VideoFmt_e4096x2160p_24Hz,            /* 4096x2160 24Hz */
	BFMT_VideoFmt_e4096x2160p_25Hz,            /* 4096x2160 25Hz */
	BFMT_VideoFmt_e4096x2160p_30Hz,            /* 4096x2160 30Hz */
	BFMT_VideoFmt_e4096x2160p_50Hz,            /* 4096x2160 50Hz */
	BFMT_VideoFmt_e4096x2160p_60Hz,            /* 4096x2160 60Hz */
#if BFMT_LEGACY_3DTV_SUPPORT
	BFMT_VideoFmt_eCUSTOM1920x2160i_48Hz,    /* 3548 LVDS output for legacy 3DTV support */
	BFMT_VideoFmt_eCUSTOM1920x2160i_60Hz,    /* 3548 LVDS output for legacy 3DTV support */
#endif
	BFMT_VideoFmt_eCUSTOM_1440x240p_60Hz,      /* 240p 60Hz 7411 custom format. */
	BFMT_VideoFmt_eCUSTOM_1440x288p_50Hz,      /* 288p 50Hz 7411 custom format. */
	BFMT_VideoFmt_eCUSTOM_1366x768p,           /* Custom 1366x768 mode */
	BFMT_VideoFmt_eCUSTOM_1366x768p_50Hz,      /* Custom 1366x768 50Hz mode */
	BFMT_VideoFmt_eDVI_640x480p,               /* DVI Safe mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x480p_CVT,           /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_800x600p,               /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1024x768p,              /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x768p,              /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x768p_Red,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x720p_50Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x720p,              /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x720p_Red,          /* DVI VESA mode for computer monitors */

	/* Added for HDMI/HDDVI input support!  VEC does not support these timing format!
	 * Convention: BFMT_VideoFmt_eDVI_{av_width}x{av_height}{i/p}_{RefreshRateInHz}.
	 * Eventually VEC can output all these timing formats when we get the microcodes
	 * for it.  Currently there are no microcode for these yet. */
	BFMT_VideoFmt_eDVI_640x350p_60Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x350p_70Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x350p_72Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x350p_75Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x350p_85Hz,          /* DVI VESA mode for computer monitors */

	BFMT_VideoFmt_eDVI_640x400p_60Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x400p_70Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x400p_72Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x400p_75Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x400p_85Hz,          /* DVI VESA mode for computer monitors */

	BFMT_VideoFmt_eDVI_640x480p_66Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x480p_70Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x480p_72Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x480p_75Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_640x480p_85Hz,          /* DVI VESA mode for computer monitors */

	BFMT_VideoFmt_eDVI_720x400p_60Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_720x400p_70Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_720x400p_72Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_720x400p_75Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_720x400p_85Hz,          /* DVI VESA mode for computer monitors */

	BFMT_VideoFmt_eDVI_800x600p_56Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_800x600p_59Hz_Red,      /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_800x600p_70Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_800x600p_72Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_800x600p_75Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_800x600p_85Hz,          /* DVI VESA mode for computer monitors */

	BFMT_VideoFmt_eDVI_1024x768p_66Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1024x768p_70Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1024x768p_72Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1024x768p_75Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1024x768p_85Hz,         /* DVI VESA mode for computer monitors */

	BFMT_VideoFmt_eDVI_1280x720p_70Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x720p_72Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x720p_75Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x720p_85Hz,         /* DVI VESA mode for computer monitors */

	/* New DVI or PC vdec input support */
	BFMT_VideoFmt_eDVI_1024x768i_87Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1152x864p_75Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x768p_75Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x768p_85Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x800p_60Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x960p_60Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x960p_85Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x1024p_60Hz,        /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x1024p_69Hz,        /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x1024p_75Hz,        /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1280x1024p_85Hz,        /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_832x624p_75Hz,          /*   MAC-16 */
	BFMT_VideoFmt_eDVI_1360x768p_60Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1366x768p_60Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1400x1050p_60Hz,        /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1400x1050p_60Hz_Red,    /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1400x1050p_75Hz,        /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1600x1200p_60Hz,        /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1920x1080p_60Hz_Red,    /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_848x480p_60Hz,          /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1064x600p_60Hz,         /* DVI VESA mode for computer monitors */
	BFMT_VideoFmt_eDVI_1440x900p_60Hz,         /* DVI VESA mode for computer monitors */

	/* SW7435-276: New format enums for 482/483 */
	BFMT_VideoFmt_e720x482_NTSC,               /* 720x482i NSTC-M for North America */
	BFMT_VideoFmt_e720x482_NTSC_J,             /* 720x482i Japan */
	BFMT_VideoFmt_e720x483p,                   /* 720x483p */

	/* statics: custom formats */
	BFMT_VideoFmt_eCustom0,         /* 59.94/60 Hz */
	BFMT_VideoFmt_eCustom1,         /* 50 Hz */

	/* dynamics: custom format */
	BFMT_VideoFmt_eCustom2,         /* defined at run time by app */

	/* Must be last */
	BFMT_VideoFmt_eMaxCount         /* Counter. Do not use! */

} BFMT_VideoFmt;

/* NOTE: These names could go away anytime better change to use the real
 * one from the enumerations above. */
/* Define BFMT_VideoFmt_eSECAM for backwards compatibility */
#define BFMT_VideoFmt_eSECAM BFMT_VideoFmt_eSECAM_L
#define BFMT_VideoFmt_eDVI_1280x720p_ReducedBlank BFMT_VideoFmt_eDVI_1280x720p_Red

#define BFMT_VideoFmt_e3D_720p         BFMT_VideoFmt_e720p_60Hz_3DOU_AS
#define BFMT_VideoFmt_e3D_720p_50Hz    BFMT_VideoFmt_e720p_50Hz_3DOU_AS
#define BFMT_VideoFmt_e3D_720p_30Hz    BFMT_VideoFmt_e720p_30Hz_3DOU_AS
#define BFMT_VideoFmt_e3D_720p_24Hz    BFMT_VideoFmt_e720p_24Hz_3DOU_AS
#define BFMT_VideoFmt_e3D_1080p_24Hz   BFMT_VideoFmt_e1080p_24Hz_3DOU_AS
#define BFMT_VideoFmt_e3D_1080p_30Hz   BFMT_VideoFmt_e1080p_30Hz_3DOU_AS

/***************************************************************************
Summary:
	This structure describes a custom format tables to be used to program VEC
	and DVO.

Description:
	eVideoFmt has to be a custom format;
	for now, it only supports DVO master mode;
	a custom format cannot have both 50 and 60 Hz rate manager entries since
	they are different custom formats; a custom format only tracks either
	60/59.94Hz or 50Hz frame rate;

	pDvoMicrocodeTbl - Microcode for the custom format timing such as LCD
	panel.

	pDvoRmTbl0 - Rate manager setting for given refresh rate.  Such as 60.00Hz,
	50.00Hz, 120.00Hz, etc.

	pDvoRmTbl1 - Rate manager setting for given refresh rate.  Such as 59.94Hz,
	50.00Hz, 120.00Hz, etc.  But this is a frame drop version of pDvoRmTbl0.
	For example if pDvoRmTbl0 is 60.00Hz, then pDvoRmTbl1 is 60.00/1.001 Hz

See Also:
	BFMT_SetCustomFormatInfo
***************************************************************************/
typedef struct
{
	/* 64-dword array */
	void                               *pDvoMicrocodeTbl;
	void                               *pDvoRmTbl0;
	void                               *pDvoRmTbl1;

} BFMT_CustomFormatInfo;

/***************************************************************************
Summary:
	This structure contains the display format information

Description:
	BFMT_VideoInfo provides the Video data for a Video format, such
	as screen size, resolution, type, and name associated with the format.

	eVideoFmt            - This video format enumeration.  Should match
		with the enum that user pass in to query the information.

	ulWidth              - The active analog screen width.  With given
		eVideoFmt the VEC can output multiple paths.  For example with NTSC
		the analog height, and its counter part HDMI output have different size.

	ulHeight             -  The active analog screen width.  With given
		eVideoFmt the VEC can output multiple paths.  For example with NTSC
		the analog height, and its counter part HDMI output have different size.

	ulDigitalWidth       - The active digital screen width

	ulDigitalHeight      - The active digital screen height

	ulScanWidth          - The total rasterization width which include
		blanking and active video.

	ulScanHeight         - The rasterization height which include blanking and
		active video.

	ulTopActive          - The start active video line of the top field or frame.

	ulTopMaxVbiPassThru  - Maximum VBI Pass Through lines at the top field or frame

	ulBotMaxVbiPassThru  - Maximum VBI Pass Through lines at the bottom field

	ulActiveSpace        - Active space for the format. Specify number of pixels
		between the left and right buffers for 3D format. Should be 0 for all 2D
		formats.

	ulVertFreqMask       - To be obsoleted!  Use ulVertFreq.

	ulVertFreq           - To be obsoleted! picture vertical frequency, or
		refresh rate in units of 1/100th Hz.  For example 60.00Hz would be
		6000, and 59.94Hz would be 5994.

	ulPxlFreqMask        - To be obsoleted!  Use ulPxlFreq.

	bInterlaced          - Indicate if the format is interlaced or progressive
		mode.

	eAspectRatio         - Default Aspect Ratio associated with this format.

	eOrientation         - Default orientation associated with the format.

	ulPxlFreq            - To be obsoleted!  Pixel frequencies
		in units of 1/100th Mhz.  For example 74.24Mhz would be 7425, and
		148.50Mhz would be 14850.

	pchFormatStr         - Video format name.

	pCustomInfo          - Custom format info, including microcode/rm tables;
		NULL for non-custom formats;  This is mainly for DVO output.

See Also:
	BFMT_GetVideoFormatInfoPtr_isr
***************************************************************************/
typedef struct
{
	BFMT_VideoFmt                      eVideoFmt;
	uint32_t                           ulWidth;
	uint32_t                           ulHeight;
	uint32_t                           ulDigitalWidth;
	uint32_t                           ulDigitalHeight;
	uint32_t                           ulScanWidth;
	uint32_t                           ulScanHeight;
	uint32_t                           ulTopActive;
	uint32_t                           ulTopMaxVbiPassThru;
	uint32_t                           ulBotMaxVbiPassThru;
	uint32_t                           ulActiveSpace;
	uint32_t                           ulVertFreqMask;
	uint32_t                           ulVertFreq; /* in 1 /BFMT_FREQ_FACTOR Hz */
	uint64_t                           ulPxlFreqMask;
	bool                               bInterlaced;
	BFMT_AspectRatio                   eAspectRatio;
	BFMT_Orientation                   eOrientation;
	uint32_t                           ulPxlFreq;  /* in 1 /BFMT_FREQ_FACTOR MHz */
	const char                        *pchFormatStr;
	BFMT_CustomFormatInfo             *pCustomInfo;

} BFMT_VideoInfo;

/***************************************************************************
Summary:
	This function queries the Video information for a specific Video
	format.

Description:
	Returns the pointer to video information for a specific Video format,
	such as width, height, top/bottom/back/front porch, interlaced or
	progressive mode, etc.  Users can query the information for any Video
	format, whether or not it is supported by a particular chipset.
	Used by applications, SysLib, and PortingInterface modules.

	NOTE: for BFMT_VideoFmt_eCustom2 BFMT does not have the information
	of this format.  It is just an enumeration to indicate that this format
	timing will be define at runtime.  It will be loaded thru VDC via
	BVDC_Display_SetCustomVideoFormat() API.  The format information of is
	kept in application, and also can be query from VDC API
	BVDC_Display_GetCustomVideoFormat().

Input:
	eVideoFmt - Video format

Returns:
	This function return a 'const BFMT_VideoInfo*'.  If eVideoFmt is an
	invalid video format it will return a NULL pointer.  Or passing in
	BFMT_VideoFmt_eCustom2.

See Also:
	BVDC_Display_SetCustomVideoFormat

**************************************************************************/
const BFMT_VideoInfo* BFMT_GetVideoFormatInfoPtr_isr
	( BFMT_VideoFmt                      eVideoFmt );
#define BFMT_GetVideoFormatInfoPtr BFMT_GetVideoFormatInfoPtr_isr

/***************************************************************************
 * Obsoleted!  Should be using BFMT_GetVideoFormatInfoPtr_isr()
 ***************************************************************************/
BERR_Code BFMT_GetVideoFormatInfo
	( BFMT_VideoFmt                    eVideoFmt,
	  BFMT_VideoInfo                  *pVideoFmtInfo );

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BFMT_H__ */
/* End of File */
