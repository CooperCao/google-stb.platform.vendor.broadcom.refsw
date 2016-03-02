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
 *   Contains tables for Display settings.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bchp.h"
#include "bvdc_display_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_csc_priv.h"

BDBG_MODULE(BVDC_DVI_CSC);

/**********************************************************
 * static tables
 **********************************************************/

/************* the followings are the original settings *****************/
/* SDYCrCb_to_SDRGB_480i */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_RGB = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	0,
	4095,
	BVDC_P_MAKE_VEC_RGB_CSC
	(  0.6256, -0.0016,  0.8543, -119.3583,
	   0.6256, -0.2087, -0.4378,   72.4649,
	   0.6256,  1.0834, -0.0038, -148.6812 )
);

/* calculated by hand since scripts introduce roundoff errors */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_SDYPbPr = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	(  0.6267,  0.0000,  0.0000,  -10.0269,
	   0.0000,  0.6127,  0.0000,  -78.4286,
	   0.0000,  0.0000,  0.6127,  -78.4300 )
);

/* vec_csc_CbYCr2dviGBR */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_RGB_DVI = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	16,   /* clamped for (1-254) */
	4064,
	BVDC_P_MAKE_DVO_RGB_CSC
	(  1.000,  0.000,  1.371, -175.488,
	   1.000, -0.336, -0.698,  132.352,
	   1.000,  1.732,  0.000, -221.696 )
);

/* SDYCrCb_to_SDRGB_DVI */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_RGB_Full_Range_DVI = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	0,
	4095,
	BVDC_P_MAKE_DVO_RGB_CSC
	(  1.1644,  0.0029,  1.5901, -222.1632,
	   1.1644, -0.3885, -0.8149,  134.8800,
	   1.1644,  2.0165,  0.0071, -276.7424 )
);

/* HDYCrCb_to_HDRGB */
static const BVDC_P_DisplayCscMatrix s_HDYCbCr_to_RGB = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	0,
	4095,
	BVDC_P_MAKE_VEC_RGB_CSC
	(  0.6267,  0.0000,  0.9649, -133.5367,
	   0.6267, -0.1148, -0.2868,   41.3785,
	   0.6267,  1.1370,  0.0000, -155.5594 )
);

/* HDYCrCb_to_HDYPrPb_1080i720p */
#define s_HDYCbCr_to_HDYPbPr s_SDYCbCr_to_SDYPbPr

/* HDYCrCb_to_HDRGB_DVI */
static const BVDC_P_DisplayCscMatrix s_HDYCbCr_to_RGB_DVI = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	16,   /* clamped for (1-254) */
	4064,
	BVDC_P_MAKE_DVO_RGB_CSC
	(  1.0000,  0.0000,  1.5396, -197.0749,
	   1.0000, -0.1831, -0.4577,   82.0247,
	   1.0000,  1.8142,  0.0000, -232.2150 )
);

/* vec_csc_hdCbYCr2dviGBR */
static const BVDC_P_DisplayCscMatrix s_HDYCbCr_to_RGB_Full_Range_DVI = BVDC_P_MAKE_VEC_CSC_MATRIX
(
    0,
    4095,
    BVDC_P_MAKE_DVO_RGB_CSC
    (
        1.1644, 0.0000,  1.7927, -248.0896,
        1.1644, -0.2132, -0.5330, 76.8887,
        1.1644,  2.1124,  0.0000, -289.0198 )
);

/* UHDYCrCb_to_UHDRGB_DVI */
static const BVDC_P_DisplayCscMatrix s_UHDYCbCr_to_RGB_DVI = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	16,   /* clamped for (1-254) */
	4064,
	BVDC_P_MAKE_DVO_RGB_CSC
	(  1.000000,  0.000000,  1.441685, -184.535657,
	   1.000000, -0.160880, -0.558600,   92.093411,
	   1.000000,  1.839404,  0.000000, -235.443771 )
);

/* vec_csc_uhdCbYCr2dviGBR */
static const BVDC_P_DisplayCscMatrix s_UHDYCbCr_to_RGB_Full_Range_DVI = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	0,
	4095,
	BVDC_P_MAKE_DVO_RGB_CSC
	(  1.168950,  0.000000,	 1.685257, -234.416111,
	   1.168950, -0.188061, -0.652975,   88.949376,
	   1.168950,  2.150171,  0.000000, -293.925139 )
);

/* vec_csc_uhdCbYCr2dviGBR_4CL */
static const BVDC_P_DisplayCscMatrix s_UHDYCbCr_to_RGB_Full_Range_DVI_for_CL = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	0,
	4095,
	BVDC_P_MAKE_DVO_CSC
	(  1.168950,  0.000000,	 1.685257, -234.416111,
	   1.168950, -0.188061, -0.652975,   88.949376,
	   1.168950,  2.150171,  0.000000, -293.925139 )
);

/* For custom panel, can be replaced as necessary */
static const BVDC_P_DisplayCscMatrix s_CustomYCbCr_to_RGB_Full_Range_DVI = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	0,
	4095,
	BVDC_P_MAKE_DVO_RGB_CSC
	(  1.168950,  0.000000,  1.727430, -239.814253,
	   1.168950, -0.183493, -0.605934,   82.343430,
	   1.168950,  2.149871,  0.000000, -293.886738 )
);

/* YCbCr limited to full range, used with BT 2020 only */
static const BVDC_P_DisplayCscMatrix s_YCbCr_Limited_Range_to_YCbCr_Full_Range_Hdmi = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_DVO_CSC
	(  1.164383562, 0.000000000, 0.000000000, -18.63013699,
	   0.000000000, 1.138392857, 0.000000000, -18.21428571,
	   0.000000000,	0.000000000, 1.138392857, -18.21428571 )
);

/* For SECAM Composite/SVideo (SDYCrCb_to_secamYDrDb) */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YDbDr = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	(  0.6267,  0.0000,  0.0000,  -10.0274,
	   0.0000,  0.8238,  0.0000, -103.1917,
	   0.0000,  0.0000, -1.0040,  128.4742 )
);

#if 1 /* CMP_CSC outputs Smpte_170M for PAL formats due to HDMI out requirement */
/* For PAL Composite/SVideo (SMPTE 170M SDYCrCb to PAL YUV) */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	( 0.6267,  0.01510535,  -0.01877718,  -9.55745459,
	  0.0000,  0.53571974,   0.01184764,  -70.0821091,
	  0.0000, -0.01962559,   0.65820667,  -81.7247368 )
);

/* For PAL_M Composite/SVideo (SDYCrCb_to_?)
 * I don't have a reference for this matrix.
 */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV_M = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	( 0.5886,  0.014187026, -0.017635633,  1.480175248,
	  0.0000,  0.546403929,  0.012083925, -71.48604901,
	  0.0000, -0.020015336,  0.671277822, -83.36475779 )
);

/* For PAL_N/NC Composite/SVideo (SDYCrCb_to_nYUV)
   The color space conversion matrix for pal_n is a little
   different from other pal modes, there is a 7.5IRE pedestal
 */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV_N = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	( 0.5886,  0.014187026, -0.017635633,  1.480175248,
	  0.0000,  0.546403929,  0.012083925, -71.48604901,
	  0.0000, -0.020015336,  0.671277822, -83.36475779 )
);

#else
/* For PAL Composite/SVideo (SDYCrCb_to_YUV) */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	( 0.6267, 0.0000, 0.0000, -10.0274,
	  0.0000, 0.5320, 0.0000, -68.0895,
	  0.0000, 0.0000, 0.7503, -96.0247 )
);

/* For PAL_M Composite/SVideo (SDYCrCb_to_?)
 * I don't have a reference for this matrix.
 */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV_M = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	(  0.5886,  0.0000,  0.0000,   1.0388,
	   0.0000,  0.5426,  0.0000, -69.4537,
	   0.0000,  0.0000,  0.7652, -97.9487 )
);

/* For PAL_N/NC Composite/SVideo (SDYCrCb_to_nYUV)
   The color space conversion matrix for pal_n is a little
   different from other pal modes, there is a 7.5IRE pedestal
 */
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YUV_N = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
      ( 0.5886,  0,          0,           1.0388,
        0,       0.54261,    0,         -69.4537,
        0,       0,          0.7652,    -97.9487 )

);
#endif

/* For NTSC Composite/SVideo (SDYCrCb_to_ntscYIQ)
   Color space convert CbYCr to YIQ + 7.5 IRE pedestal on Y
*/
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YIQ = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_YIQ_CSC
	(  0.5913,  0.0000,  0.0000,    1.0388,
	   0.0000, -0.2766,  0.5998,  -41.3690,
	   0.0000,  0.4200,  0.3891, -103.5615 )
);

/* For NTSC_J Composite/SVideo (SDYCrCb_to_mYIQ)*/
static const BVDC_P_DisplayCscMatrix s_SDYCbCr_to_YIQ_M = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_YIQ_CSC
	(  0.6393,  0.0000,  0.0000,  -10.2283,
	   0.0000, -0.2990,  0.6484,  -44.7232,
	   0.0000,  0.4541,  0.4206, -111.9584 )
);

/* For 656 output. */
static const BVDC_P_DisplayCscMatrix s_HDYCbCr_to_SDYCbCr_656 = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_656_CSC
	(  1.000102,  0.099579,  0.191999, -37.323650,
	   0.001012,  0.989664, -0.110798,  15.488960,
	  -0.000073, -0.071500,  0.983585,  11.254214 )
);

static const BVDC_P_DisplayCscMatrix s_HD240MYCbCr_to_SDYCbCr_656 = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_656_CSC
	(  1.000000,  0.073380,  0.187140, -33.346601,
	  -0.000000,  0.988035, -0.108776,  15.454889,
	   0.000000, -0.053362,  0.987620,   8.414941 )
);

static const BVDC_P_DisplayCscMatrix s_Identity_656 = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_656_CSC
	(  1.0000,  0.0000,  0.0000,  0.0000,
	   0.0000,  1.0000,  0.0000,  0.0000,
	   0.0000,  0.0000,  1.0000,  0.0000 )
);

/* vec_csc_std_init */
static const BVDC_P_DisplayCscMatrix s_Identity_DVI = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_DVO_CSC
	(  1.0000,  0.0000,  0.0000,  0.0000,
	   0.0000,  1.0000,  0.0000,  0.0000,
	   0.0000,  0.0000,  1.0000,  0.0000 )
);

/* RGB + Hsync matrix */
static const BVDC_P_DisplayCscMatrix s_HsyncMatrix = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	(  0.0000,  0.0000,  0.0000,   0.0000,
	   0.0000,  0.0000,  0.0000,  16.0000,
	   0.0000,  0.0000,  0.0000,  16.0000 )
);

/* XvYCC to no XvYCC matrices for analog output.

Calculated by post-multiplying SDYCbCr_to_XXX matrix by the inverse
of s_CMP_NtscSDYCbCr_to_XvYCCSDYCbCr which strips XvYCC from the source */

/* SDYCrCb_to_SDRGB_480i */
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_RGB = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	0,
	4095,
	BVDC_P_MAKE_VEC_RGB_CSC
	(  0.6267, -0.0008,  0.9360, -129.9406,
	   0.6267, -0.2350, -0.4715,   80.1133,
	   0.6267,  1.0778,  0.0047, -147.8734 )
);

/* SDYCrCb_to_SDYPrPb_480i */
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_SDYPbPr = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	(  0.6267, -0.0158,  0.0047,  -8.5615,
	   0.0000,  0.6176,  0.0025, -78.6586,
	   0.0000,  0.0115,  0.6667, -86.8115 )
);

/* For SECAM Composite/SVideo (SDYCrCb_to_secamYDrDb) */
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_YDbDr = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	(  0.6267, -0.0158,  0.0047,   -8.6184,
	   0.0000,  0.8301, -0.0040, -103.4817,
	   0.0000, -0.0195, -1.0924,  142.2855 )
);

/* For PAL, PAL_NC Composite/SVideo (SDYCrCb_to_YUV) */
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_YUV = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	(  0.6267, -0.0158,  0.0047,   -8.6184,
	   0.0000,  0.5361, -0.0026,  -68.2768,
	   0.0000,  0.0145,  0.8164, -106.3461 )
);

/* For PAL_N/M Composite/SVideo (SDYCrCb_to_nYUV)
   The color space conversion matrix for pal_n is a little
   different from other pal modes, there is a 7.5IRE pedestal
 */
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_YUV_N = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_CSC
	(  0.5886, -0.0148,  0.0045,    2.3621,
	   0.0000,  0.5467, -0.0026,  -69.6447,
	   0.0000,  0.0148,  0.8326, -108.4750 )
);

/* For NTSC Composite/SVideo (SDYCrCb_to_ntscYIQ)
   Color space convert CbYCr to YIQ + 7.5 IRE pedestal on Y
*/
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_YIQ = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_YIQ_CSC
	(  0.5913, -0.0149,  0.0045,    2.3682,
	   0.0000, -0.2671,  0.6540,  -49.5227,
	   0.0000,  0.4307,  0.4213, -109.0619 )
);

/* For NTSC_J Composite/SVideo (SDYCrCb_to_mYIQ)*/
static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_YIQ_M = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_VEC_YIQ_CSC
	(  0.6393, -0.0161,  0.0048,   -8.7910,
	   0.0000, -0.2887,  0.7070,  -53.5375,
	   0.0000,  0.4657,  0.4554, -117.9042 )
);

static const BVDC_P_DisplayCscMatrix s_XvYCC_SDYCbCr_to_SDYCbCr_656 = BVDC_P_MAKE_VEC_CSC_MATRIX
(
	-4096,
	4095,
	BVDC_P_MAKE_656_CSC
	(  1.0000, -0.0251,  0.0076,   2.2482,
	   0.0000,  1.0076, -0.0049,  -0.3520,
	   0.0000,  0.0194,  1.0881, -13.7563 )
);

/****************************************************************
 *  Global Tables
 *  3560 might not need to compensate CMP truncation error;
 ****************************************************************/

/* PAL specializations go here: */

/* use normal PAL matrix for PAL-NC: */
#define s_SDYCbCr_to_YUV_NC s_SDYCbCr_to_YUV

/* SDYCbCr CSC table, based on BVDC_P_Output */
static const BVDC_P_DisplayCscMatrix* const s_apSDYCbCr_MatrixTbl[BVDC_P_Output_eMax] =
{
	&s_SDYCbCr_to_YIQ,       /* SVideo, Composite, SC (for NTSC) */
	&s_SDYCbCr_to_YIQ_M,     /* SVideo, Composite, SC (for NTSC_J) */
	&s_SDYCbCr_to_YUV,       /* SVideo, Composite, SC (for generic Pal) */
	&s_SDYCbCr_to_YUV_M,     /* SVideo, Composite, SC (for Pal_M) */
	&s_SDYCbCr_to_YUV_N,     /* SVideo, Composite, SC (for Pal_N) */
	&s_SDYCbCr_to_YUV_NC,    /* SVideo, Composite, SC (for PAL_NC) */
#if BVDC_P_SUPPORT_VEC_SECAM
	&s_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
	&s_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
	&s_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
#endif
	&s_SDYCbCr_to_SDYPbPr,   /* SYPbPr, CYPbPr, SCYPbPr, SDYPbPr */
	&s_SDYCbCr_to_RGB,       /* RGBSRGB, CRGB, SCRGB, RGB */
};

#if 1
/* xvYCC SDYCbCr CSC table, based on BVDC_P_Output */
static const BVDC_P_DisplayCscMatrix* const s_apXvYCC_SDYCbCr_MatrixTbl[BVDC_P_Output_eMax] =
{
	&s_XvYCC_SDYCbCr_to_YIQ,       /* SVideo, Composite, SC (for NTSC) */
	&s_XvYCC_SDYCbCr_to_YIQ_M,     /* SVideo, Composite, SC (for NTSC_J) */
	&s_XvYCC_SDYCbCr_to_YUV,       /* SVideo, Composite, SC (for generic Pal) */
	&s_XvYCC_SDYCbCr_to_YUV_N,     /* SVideo, Composite, SC (for Pal_N/M) */
	&s_XvYCC_SDYCbCr_to_YUV,       /* SVideo, Composite, SC (for PAL_NC) */
#if BVDC_P_SUPPORT_VEC_SECAM
	&s_XvYCC_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
	&s_XvYCC_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
	&s_XvYCC_SDYCbCr_to_YDbDr,     /* SVideo, Composite, SC (for SECAM) */
#endif
	&s_XvYCC_SDYCbCr_to_SDYPbPr,   /* SYPbPr, CYPbPr, SCYPbPr, SDYPbPr */
	&s_XvYCC_SDYCbCr_to_RGB,       /* RGBSRGB, CRGB, SCRGB, RGB */
};
#endif

static const BVDC_P_DisplayCscMatrix* const s_apHDYCbCr_MatrixTbl[] =
{
	&s_HDYCbCr_to_RGB,       /* RGBSRGB, CRGB, SCRGB, RGB */
	&s_HDYCbCr_to_HDYPbPr,   /* HDYPbPr */
};

/* SDYCbCr DVI CSC table, based on BVDC_HdmiOutput */
static const BVDC_P_DisplayCscMatrix* const s_apSDYCbCr_DVI_MatrixTbl[] =
{
	&s_SDYCbCr_to_RGB_DVI,
	&s_Identity_DVI,
	&s_SDYCbCr_to_RGB_Full_Range_DVI
};

/* HDYCbCr DVI CSC table, based on BVDC_HdmiOutput */
static const BVDC_P_DisplayCscMatrix* const s_apHDYCbCr_DVI_MatrixTbl[] =
{
	&s_HDYCbCr_to_RGB_DVI,
	&s_Identity_DVI,
	&s_HDYCbCr_to_RGB_Full_Range_DVI
};

/* UHDYCbCr DVI CSC table, based on BVDC_HdmiOutput */
static const BVDC_P_DisplayCscMatrix* const s_apUHDYCbCr_DVI_MatrixTbl[] =
{
	&s_UHDYCbCr_to_RGB_DVI,
	&s_Identity_DVI,
	&s_UHDYCbCr_to_RGB_Full_Range_DVI
};

/* SDYCbCr 656 CSC table */
static const BVDC_P_DisplayCscMatrix* const s_apSDYCbCr_656_Bypass_MatrixTbl[] =
{
	&s_HDYCbCr_to_SDYCbCr_656,            /* from HD video source */
	&s_Identity_656,                      /* from NTSC SD video source */
	&s_Identity_656,                      /* from PAL SD video source */
	NULL,
	&s_Identity_656,                      /* from FCC 1953 video source */
	&s_HD240MYCbCr_to_SDYCbCr_656         /* from SMPTE 240M HD video source */
};


/***************************************************************************
 *
 */
void BVDC_P_Display_GetCscTable_isr
	( const BVDC_P_DisplayInfo        *pDispInfo,
	  BVDC_P_Output                    eOutputColorSpace,
	  const BVDC_P_DisplayCscMatrix  **ppCscMatrix )
{

	/* Component only CSC */
	/* Get proper color space conversion table.
	 * Note: assume non-bypass displays only have HD color space intput! */
	if(BVDC_P_Output_eHsync == eOutputColorSpace)
	{
		/* RGB + Hsync + Vsync */
		*ppCscMatrix = &s_HsyncMatrix;
	}
	else if(VIDEO_FORMAT_27Mhz(pDispInfo->pFmtInfo->ulPxlFreqMask))
	{
		BDBG_ASSERT(eOutputColorSpace <= BVDC_P_Output_eSDRGB);

		/* NTSCs, PALs, 480p and 576p;
		   TODO: differentiate NTSC SD and PAL SD color space. */
		*ppCscMatrix =
#ifndef BVDC_FOR_BOOTUPDATER
			(pDispInfo->bXvYcc) ? s_apXvYCC_SDYCbCr_MatrixTbl[eOutputColorSpace] :
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
			s_apSDYCbCr_MatrixTbl[eOutputColorSpace];
	}
	else
	{
		/* HD output */
		uint32_t   ulIndex = (eOutputColorSpace == BVDC_P_Output_eHDYPrPb) ? 1 : 0;
		if(pDispInfo->eCmpColorSpace <= BVDC_P_CmpColorSpace_eSmpte_240M)
		{
			*ppCscMatrix = s_apHDYCbCr_MatrixTbl[ulIndex];
		}
		else
		{
			BDBG_ASSERT(pDispInfo->eCmpColorSpace <= BVDC_P_CmpColorSpace_eSmpte_240M);
		}
	}

	return;
}

extern const BVDC_P_CscCoeffs s_CMP_UhdYCbCr_to_BT2020RGB;
/***************************************************************************
 * This is to convert the color space from CMP -> DVO.  Since the CMP's CSC
 * already convert color space of specific input -> YCbCr ; the DVO only needs
 * to convert it further to:
 *   (1) RGB
 *   (2) RGB Full Range
 *   (3) bypass (identity matrix)
 */
void BVDC_P_Display_GetDviCscTable_isr
	( const BVDC_P_DisplayInfo        *pDispInfo,
	  const BVDC_P_DisplayCscMatrix  **ppCscMatrix )
{
	BAVC_Colorspace  eColorComponent;
	BAVC_ColorRange  eColorRange;

#if BVDC_P_ORTHOGONAL_VEC
	eColorComponent = pDispInfo->stHdmiSettings.stSettings.eColorComponent;
	eColorRange = pDispInfo->stHdmiSettings.stSettings.eColorRange;
#else
	eColorComponent = ((BAVC_MatrixCoefficients_eHdmi_RGB           == pDispInfo->eHdmiOutput) ||
					   (BAVC_MatrixCoefficients_eDvi_Full_Range_RGB == pDispInfo->eHdmiOutput))?
		BAVC_Colorspace_eRGB : BAVC_Colorspace_eYCbCr444;
	eColorRange = BAVC_ColorRange_eAuto;
#endif

	BDBG_MSG(("dviOut color Space %d, range %d, standard %d", eColorComponent, eColorRange, pDispInfo->eHdmiOutput));

	/* this is for back compatibility */
	if (BAVC_ColorRange_eAuto == eColorRange)
	{
		eColorRange = ((BAVC_MatrixCoefficients_eDvi_Full_Range_RGB    == pDispInfo->eHdmiOutput) ||
					   (BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr == pDispInfo->eHdmiOutput)) ?
			BAVC_ColorRange_eFull : BAVC_ColorRange_eLimited;
	}
	if (((BAVC_MatrixCoefficients_eHdmi_RGB           == pDispInfo->eHdmiOutput) ||
		 (BAVC_MatrixCoefficients_eDvi_Full_Range_RGB == pDispInfo->eHdmiOutput)) &&
		(BAVC_Colorspace_eRGB != eColorComponent))
	{
		eColorComponent = BAVC_Colorspace_eRGB;
		BDBG_MSG(("Hdmi output BAVC_MatrixCoefficients = %d but ColorSpace is NOT RGB (%d)", pDispInfo->eHdmiOutput, eColorComponent));
	}

#if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC
	if (BAVC_MatrixCoefficients_eItu_R_BT_2020_CL == pDispInfo->eHdmiOutput)
	{
		/* setup for NL CSC HW */
		*ppCscMatrix = &s_UHDYCbCr_to_RGB_Full_Range_DVI_for_CL;
	}
	else
#endif
	if (BAVC_Colorspace_eRGB == eColorComponent)
	{
		if (BAVC_ColorRange_eLimited == eColorRange)
		{
			if (pDispInfo->pFmtInfo->ulHeight >= BFMT_2160P_HEIGHT)
			{
				*ppCscMatrix = s_apUHDYCbCr_DVI_MatrixTbl[0];
			}
			else if (BFMT_IS_VESA_MODE(pDispInfo->pFmtInfo->eVideoFmt) ||
					 (pDispInfo->pFmtInfo->ulWidth > BFMT_NTSC_WIDTH))
			{
				*ppCscMatrix = s_apHDYCbCr_DVI_MatrixTbl[0];
			}
			else
			{
				*ppCscMatrix = s_apSDYCbCr_DVI_MatrixTbl[0];
			}
		}
		else /* full range */
		{
			if(BVDC_P_IS_CUSTOMFMT(pDispInfo->pFmtInfo->eVideoFmt))
			{
				*ppCscMatrix = &s_CustomYCbCr_to_RGB_Full_Range_DVI;
			}
			else
			{
				if (pDispInfo->pFmtInfo->ulHeight >= BFMT_2160P_HEIGHT)
				{
					*ppCscMatrix = s_apUHDYCbCr_DVI_MatrixTbl[2];
				}
				else if (BFMT_IS_VESA_MODE(pDispInfo->pFmtInfo->eVideoFmt) ||
						 (pDispInfo->pFmtInfo->ulWidth > BFMT_NTSC_WIDTH))
				{
					*ppCscMatrix = s_apHDYCbCr_DVI_MatrixTbl[2];
				}
				else
				{
					*ppCscMatrix = s_apSDYCbCr_DVI_MatrixTbl[2];
				}
			}
		}
	}
	else /* YCbCr */
	{
		if (BAVC_ColorRange_eLimited == eColorRange)
		{
			*ppCscMatrix = &s_Identity_DVI;
		}
		else /* full range */
		{
			*ppCscMatrix = &s_YCbCr_Limited_Range_to_YCbCr_Full_Range_Hdmi;
		}
	}

	BDBG_MSG(("Refer to BVDC_P_MAKE_VEC_CSC, YCbCr col 0 and col 1 are swapped."));
	BDBG_MSG(("Refer to BVDC_P_MAKE_VEC_RGB_CSC, RGB raws are also swapped: r0 on r1 position, r1 on r2, r2 on r0"));
	BDBG_MSG(("DVI_CSC: CL2020_NLCsc_En %d", (BAVC_MatrixCoefficients_eItu_R_BT_2020_CL == pDispInfo->eHdmiOutput)? 1: 0));
	BVDC_P_Csc_Print_isr(&((*ppCscMatrix)->stCscCoeffs));
	return;
}


/***************************************************************************
 *
 */
void BVDC_P_Display_Get656CscTable_isr
	( const BVDC_P_DisplayInfo        *pDispInfo,
	  bool                             bBypass,
	  const BVDC_P_DisplayCscMatrix  **ppCscMatrix )
{
	/* Note the new color space conversion in compositor would always
	 * output SD to 656 output. */
	if(pDispInfo->eCmpColorSpace <= BVDC_P_CmpColorSpace_eSmpte_240M)
	{
		*ppCscMatrix = (bBypass) ?
			s_apSDYCbCr_656_Bypass_MatrixTbl[pDispInfo->eCmpColorSpace] :
			((pDispInfo->bXvYcc) ? &s_XvYCC_SDYCbCr_to_SDYCbCr_656 :
			                       &s_Identity_656);
	}
	else
	{
		BDBG_ASSERT(pDispInfo->eCmpColorSpace <= BVDC_P_CmpColorSpace_eSmpte_240M);
	}
	return;
}

/* End of file */
