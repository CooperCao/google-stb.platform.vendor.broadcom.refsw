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
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bstd.h"
#include "bvdc_scaler_priv.h"
#include "bvdc_window_priv.h"
#include "bvdc_displayfmt_priv.h"
#include "bvdc_csc_priv.h"
#include "bchp_cmp_0.h"

BDBG_MODULE(BVDC_CSC);

/***************************************************************************
 * video surface color space conversion matrices Set I
 * Note:
 *  - these csc tables match the color primaries of different standards;
 *  - we do not do non-linear gamma correction;
 *  - the CMP matrix is for [YCbCr] -> [YCbCr];
 *  - PR10417: coeffs are in two's complement fixed point format;
 * -----------------------------------------------------
 * Color Space Standards:
 *  - ITU-R BT.709, i.e. ATSC HD or PAL HD;
 *  - FCC, i.e. NTSC SD 1953 standard, with ITU-R BT.470-2 System M color primaries;
 *  - ITU-R BT.470-2 System B, G, i.e. PAL SD;
 *  - SMPTE 170M, i.e. modern NTSC SD;
 *  - SMPTE 240M (1987 standard), the legacy ATSC HD;
 **************************************************************************/

/* Group 0: xyz -> BT.709 (i.e. HD) */
/* SMPTE 170M (i.e. modern SD NTSC) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_NtscSDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.086918, -0.198602,  36.546531,
	   0.000000,  1.008912,  0.109928, -15.211527,
	  -0.000000,  0.057004,  0.942791,   0.026243 );

/* BT.470-2 System B, G (i.e. SD Pal) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_PalSDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.115168, -0.189312,  38.973396,
	   0.000000,  1.004499,  0.099809, -13.351437,
	  -0.000000,  0.084468,  1.072557, -20.099241 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.000000, -0.000000,  0.000000,
	  -0.000000,  0.990940, -0.000253,  1.192039,
	  -0.000000,  0.004734,  0.924649,  9.039049 );

/* FCC (i.e. 1953 NTSC) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_FccSDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.000000,  0.000000,   0.0000000,
	   0.054988,  1.121448, -0.005106, -15.771562,
	   0.033634, -0.011863,  1.511262, -64.461172 );

/* Group 1: xyz -> SMPTE 170M (i.e. modern SD NTSC) */
/* BT.709 (i.e. HD) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_HDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.074740,  0.201939, -35.414914,
	  -0.000000,  0.997740, -0.116335,  15.180202,
	   0.000000, -0.060327,  1.067715,  -0.945678 );

/* BT.470-2 System B, G (i.e. SD Pal) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_PalSDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.023033,  0.034738,  -1.498223,
	  -0.000000,  0.992402, -0.025193,   4.197192,
	  -0.000000,  0.029590,  1.139164, -21.600485 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.075019,  0.186703, -33.500488,
	  -0.000000,  0.988150, -0.107822,  15.317986,
	  -0.000000, -0.054726,  0.987276,   8.633535 );

/* FCC (i.e. 1953 NTSC) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_FccSDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.010902,  0.081422,  0.304801, -49.610885,
	   0.050951,  1.120293, -0.180907,   6.943399,
	   0.032594, -0.080320,  1.613905, -68.820370 );

/* Group 2: xyz -> BT.470-2 System B, G (i.e. SD Pal) */
/* SMPTE 170M (i.e. modern SD NTSC) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_CscCoeffs  s_CMP_NtscSDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.024103, -0.029962,   0.749873,
	   0.000000,  1.006992,  0.022270,  -3.745506,
	   0.000000, -0.026157,  0.877258,  19.058994 );

/* BT.709 (i.e. HD) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_CscCoeffs  s_CMP_HDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.100597,  0.167144, -34.270816,
	  -0.000000,  1.003373, -0.093371,  11.519780,
	   0.000000, -0.079020,  0.939705,  17.832324 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.100476,  0.154524, -32.640077,
	  -0.000000,  0.993840, -0.086589,  11.871852,
	   0.000000, -0.073855,  0.868917,  26.232166 );

/* FCC (i.e. 1953 NTSC) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_CscCoeffs  s_CMP_FccSDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.011153,  0.110831,  0.252085, -46.631688,
	   0.052033,  1.126338, -0.146231,   1.713843,
	   0.027261, -0.099765,  1.420544, -41.495877 );

/* BT.709 (i.e. HD) -> to xvYCC BT.601 (i.e. SD) */
static const BVDC_P_CscCoeffs  s_CMP_HDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.100178,  0.191707, -37.361314,
	   0.000000,  0.989849, -0.110711,  15.470362,
	  -0.000000, -0.073079,  0.983251,  11.497916 );

/* FCC (i.e. 1953 NTSC) -> to xvYCC BT.601 (i.e. SD) */
static const BVDC_P_CscCoeffs  s_CMP_FccSDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.011956,  0.110070,  0.289208, -51.298935,
	   0.050706,  1.111377, -0.172368,   6.995471,
	   0.029052, -0.093619,  1.486324, -50.731057 );

/* BT.470-2 System B, G (i.e. SD Pal) -> xvYCC BT.601 (i.e. SD) */
static const BVDC_P_CscCoeffs  s_CMP_PalSDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.001655,  0.026303, -3.578607,
	   0.000000,  0.984950, -0.019948,  4.479666,
	  -0.000000,  0.009646,  1.047299, -7.288983 );

/* SMPTE 170M (i.e. modern SD NTSC)) -> xvYCC BT.601 (i.e. SD) */
static const BVDC_P_CscCoeffs  s_CMP_NtscSDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.025081, -0.006850,  -2.333618,
	   0.000000,  0.992359,  0.004435,   0.410342,
	  -0.000000, -0.017681,  0.918967,  12.635360 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> xvYCC BT.601 (i.e. SD) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.100178,  0.177236, -35.509050,
	  -0.000000,  0.980357, -0.102619,  15.649576,
	  -0.000000, -0.067762,  0.909181,  20.298460 );

/* xvYCC BT.601 (i.e. SD) -> to BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_XvYCCSDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.116569, -0.208098,  41.557373,
	   0.000000,  1.018724,  0.114705, -17.078894,
	  -0.000000,  0.075715,  1.025559, -12.963136 );

/* xvYCC BT.601 (i.e. SD) -> SMPTE 170M (i.e. modern SD NTSC)) */
static const BVDC_P_CscCoeffs  s_CMP_XvYCCSDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.025140,  0.007575,   2.248217,
	  -0.000000,  1.007613, -0.004863,  -0.352023,
	   0.000000,  0.019386,  1.088085, -13.756298 );

/* xvYCC BT.601 (i.e. SD) -> BT.470-2 System B, G (i.e. SD Pal) */
static const BVDC_P_CscCoeffs  s_CMP_XvYCCSDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.001434, -0.025143,  3.401765,
	   0.000000,  1.015090,  0.019334, -4.406337,
	   0.000000, -0.009349,  0.954659,  7.000373 );

/* 472M (i.e. SD Pal) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_PalSDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.114060,	-0.220759,	42.856807,
	  -0.000458,  0.876936,  0.091411,   4.058961,
	   0.001246,  0.060051,  0.716182,  28.622240 );

/* 240M (i.e. 1987 ATSC HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_240MHDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  -0.000795,   0.000649,   0.018744,
	   0.000000,   0.866564,  -0.009478,  18.292975,
	   0.000000,   0.015509,   0.615175,  47.272494 );

/* BT601 (i.e. xvYCC BT.601) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_XvYCCSDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,   0.001252,   0.000420,  -0.214032,
	   0.048687,   0.982230,  -0.020016,   4.057565,
	   0.019779,   0.006763,   1.005020,  -1.824650 );

/* 170M	(i.e. modern SD NTSC) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_NtscSDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  -0.085911,  -0.233380,  40.869177,
	   0.000000,   0.881816,   0.103899,   1.828532,
	   0.000000,   0.044681,   0.628677,  41.810198 );

/* BT2020 (i.e. UHD) -> BT601 (i.e. xvYCC BT.601)*/
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_XvYCCSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000071,  -0.001272,  -0.000444,   0.218397,
	  -0.049966,   1.018015,   0.020296,  -4.104323,
	  -0.019346,  -0.006825,   0.994877,   1.838855 );

/* BT2020 (i.e. UHD) -> 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,   0.079279,   0.358121,  -55.987253,
	   0.000000,   1.143600,  -0.188998,    5.810930,
	   0.000000,  -0.081277,   1.604074,  -66.918015 );

/* BT2020 (i.e. UHD) -> 472M (i.e. SD Pal) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_PalSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  0.999684,   0.109884,   0.294122,  -51.707697,
	   0.000710,   1.150467,  -0.146623,   -0.503457,
	  -0.001799,  -0.096657,   1.408076,  -39.832873 );

/* BT2020 (i.e. UHD) -> BT709 (i.e. HD) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,   0.000015, -0.000130,    0.014753,
	   0.000000,   1.143195,  0.016617,  -20.455953,
	   0.000000,  -0.021793,  1.502906,  -61.582478 );

/* BT709 (i.e. HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_HDYCbCr_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  -0.000011,   0.000087,  -0.009648,
	   0.000000,   0.874557,  -0.009669,  17.294425,
	   0.000000,   0.012682,   0.665237,  41.226373 );

/* identity matrix */
static const BVDC_P_CscCoeffs s_CMP_Identity_YCbCr_to_YCbCr = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.000000,  0.000000,  0.000000,
	   0.000000,  1.000000,  0.000000,  0.000000,
	   0.000000,  0.000000,  1.000000,  0.000000 );


/***************************************************************************
 * video surface color space conversion matrices Set II
 * Note:
 *  - these csc matrices DON'T MATCH the color primaries of different standards;
 *  - we do not do non-linear gamma correction;
 *  - the CMP matrix is for [YCbCr] -> [YCbCr];
 *  - PR10417: coeffs are in two's complement fixed point format;
 * -----------------------------------------------------
 * Color Space Standards:
 *  - ITU-R BT.709, i.e. ATSC HD or PAL HD;
 *  - FCC, i.e. NTSC SD 1953 standard, with ITU-R BT.470-2 System M color primaries;
 *  - ITU-R BT.470-2 System B, G, i.e. PAL SD;
 *  - SMPTE 170M, i.e. modern NTSC SD;
 *  - SMPTE 240M (1987 standard), the legacy ATSC HD;
 **************************************************************************/

/* Group 0: xyz -> BT.709 (i.e. HD) */
/* SMPTE 170M (i.e. modern SD NTSC) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_NtscSDYCbCr_to_HDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.116569, -0.208098,  41.557373,
	   0.000000,  1.018724,  0.114705, -17.078894,
	   0.000000,  0.075715,  1.025559, -12.963136 );

/* BT.470-2 System B, G (i.e. SD Pal) -> BT.709 (i.e. HD),
   the same as  SMPTE 170M (i.e. modern SD NTSC) -> BT.709 (i.e. HD) */

/* SMPTE 240M (i.e. 1987 ATSC HD) -> BT.709 (i.e. HD) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_HDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.028780, -0.006178,  4.474660,
	  -0.000000,  1.000374,  0.003405, -0.483788,
	  -0.000000,  0.018694,  1.004346, -2.949129 );

/* FCC (i.e. 1953 NTSC) -> BT.709 (i.e. HD),
   the same as  SMPTE 170M (i.e. modern SD NTSC) -> BT.709 (i.e. HD) */

/* Group 1: xyz -> SMPTE 170M (i.e. modern SD NTSC) */
/* BT.709 (i.e. HD) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_HDYCbCr_to_NtscSDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.100178,  0.191707, -37.361314,
	   0.000000,  0.989849, -0.110711,  15.470362,
	   0.000000, -0.073079,  0.983251,  11.497916 );

/* BT.470-2 System B, G (i.e. SD Pal) -> SMPTE 170M (i.e. modern SD NTSC),
   the same as  SMPTE 170M (i.e. modern SD NTSC) -> SMPTE 170M (i.e. modern SD NTSC) */

/* SMPTE 240M (i.e. 1987 ATSC HD) -> SMPTE 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs  s_CMP_240MHDYCbCr_to_NtscSDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.075019,  0.186703, -33.500488,
	  -0.000000,  0.988150, -0.107822,  15.317986,
	  -0.000000, -0.054726,  0.987276,   8.633535 );

/* FCC (i.e. 1953 NTSC) -> SMPTE 170M (i.e. modern SD NTSC),
   the same as  SMPTE 170M (i.e. modern SD NTSC) -> SMPTE 170M (i.e. modern SD NTSC) */

/* Group 2: xyz -> BT.470-2 System B, G (i.e. SD Pal) */
/* xyz -> BT.709 (i.e. HD),
   the same as xyz -> BT.470-2 System B, G (i.e. SD Pal) */

/* non color primary matching matrix should never be used when UHD is involved */
#if BVDC_P_TRY_MATRIX_II_FOR_UHD
/* 240M (i.e. 1987 ATSC HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_240MHDYCbCr_to_UHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.044355,  0.088837,  -5.693701,
	   0.000000,  0.994668, -0.048297,   6.864513,
	   0.000000,  0.030766,  1.007144,  -4.852506 );

/* BT601 (i.e. xvYCC BT.601) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_XvYCCSDYCbCr_to_UHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.125382, -0.113292,  30.550350,
	   0.000000,  1.010016,  0.061592,  -9.165859,
	   0.000000,  0.086969,  1.029350, -14.888815 );

/* BT 472M (i.e. SD Pal) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_PalSDYCbCr_to_UHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.125382, -0.113292,  30.550350,
	  -0.000000,  1.010016,  0.061592,  -9.165859,
	  -0.000000,  0.086969,  1.029350, -14.888815 );

/*170M (i.e moden SD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_NtscSDYCbCr_to_UHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.125382, -0.113292,  30.550350,
	   0.000000,  1.010016,  0.061592,  -9.165859,
	   0.000000,  0.086969,  1.029350, -14.888815 );

/* BT709 (i.e HD) -> BT2020 (i.e. UHD) */
static const BVDC_P_CscCoeffs s_CMP_HDYCbCr_to_UHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000, -0.016590,  0.094162,  -9.929200,
	   0.000000,  0.995306, -0.051192,   7.153340,
	   0.000000,  0.011507,  1.002637,  -1.810461 );

/* BT2020 (i.e. UHD) -> 170M (i.e. modern SD NTSC) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_NtscSDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.115256,  0.103166, -27.957919,
	   0.000000,  0.995211, -0.059549,   8.235343,
	   0.000000, -0.084085,  0.976518,  13.768493 );

/* 	BT2020 (i.e. UHD) -> BT601 (i.e. xvYCC BT.601) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_XvYCCSDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.115256,  0.103166, -27.957919,
	   0.000000,  0.995211, -0.059549,   8.235343,
	   0.000000, -0.084085,  0.976518,  13.768493 );

/* 	BT2020 (i.e. UHD) -> 240M (i.e. 1987 ATSC HD) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_240MHDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.047251, -0.085941,   4.952314,
	   0.000000,  1.003872,  0.048140,  -6.657492,
	   0.000000, -0.030666,  0.991436,   5.021459 );

/* 	BT2020 (i.e. UHD) -> 472M (SD Pal) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_PalSDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.115256,  0.103166,  -27.957919,
	   0.000000,  0.995211, -0.059549,    8.235343,
	   0.000000, -0.084085,  0.976518,   13.768493 );

/* 	BT2020 (i.e. UHD) -> BT709 (i.e. HD) */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_HDYCbCr_II = BVDC_P_MAKE_CMP_CSC
	(  1.000000,  0.017744, -0.093008,  9.633887,
	   0.000000,  1.004123,  0.051267, -7.090019,
	   0.000000, -0.011524,  0.996782,  1.887072 );
#else /* #ifdef BVDC_P_TRY_MATRIX_II_FOR_UHD */
#define s_CMP_240MHDYCbCr_to_UHDYCbCr_II   s_CMP_240MHDYCbCr_to_UHDYCbCr
#define s_CMP_XvYCCSDYCbCr_to_UHDYCbCr_II  s_CMP_XvYCCSDYCbCr_to_UHDYCbCr
#define s_CMP_PalSDYCbCr_to_UHDYCbCr_II    s_CMP_PalSDYCbCr_to_UHDYCbCr
#define s_CMP_NtscSDYCbCr_to_UHDYCbCr_II   s_CMP_NtscSDYCbCr_to_UHDYCbCr
#define s_CMP_HDYCbCr_to_UHDYCbCr_II       s_CMP_HDYCbCr_to_UHDYCbCr
#define s_CMP_UHDYCbCr_to_NtscSDYCbCr_II   s_CMP_UHDYCbCr_to_NtscSDYCbCr
#define s_CMP_UHDYCbCr_to_XvYCCSDYCbCr_II  s_CMP_UHDYCbCr_to_XvYCCSDYCbCr
#define s_CMP_UHDYCbCr_to_240MHDYCbCr_II   s_CMP_UHDYCbCr_to_240MHDYCbCr
#define s_CMP_UHDYCbCr_to_PalSDYCbCr_II    s_CMP_UHDYCbCr_to_PalSDYCbCr
#define s_CMP_UHDYCbCr_to_HDYCbCr_II       s_CMP_UHDYCbCr_to_HDYCbCr
#endif /* #if BVDC_P_TRY_MATRIX_II_FOR_UHD */

#if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC
/***************************************************************************
 * video surface color space conversion matrices Set MA and MC for CMPs with
 * non-linear conversion HW
 * Note:
 *  - The color space standards are devided into 2 groups: the BT2020 group
 *    and the non BT2020 group.
 *  - The BT2020 group has NCL and CL. They have the same primary and white
 *    point. But the matrix between R'G'B' and YCbCr are derived differently.
 *  - The non Bt.2020 group has all standards before BT2020. Their primary
 *    and white point are all similar, conversion between them can be well
 *    approximated with a single matrix (i.e. the non-linear process can be
 *    ignored).
 *  - The Non-linear color space converter HW can transfer from BT709 R'G'B'
 *    to BT2020 R'G'B'; from NCL YCbCr, CL YCbCr, and BT2020 R'G'B' to BT709
 *    or BT2020 R'G'B'.
 *  - MA and MC work together with the non-linear converter HW to convert any
 *    color space to limited range BT2020 NCL YCbCr, or any limited range
 *    non-BT2020 YCbCr.
 *  - VEC will further convert from the output of MC to desired format.
 *  - When BT2020 CL YCbCr is needed, output of MC is limited range BT2020
 *    NCL YCbCr, DVI_CSC converts it to BT2020 R'G'B', and the following
 *    non-linear converter HW further transfers it to BT2020 CL YCbCr
 * -----------------------------------------------------
 * Color Space Standards:
 *  - ITU-R BT.709, i.e. ATSC HD or PAL HD;
 *  - FCC, i.e. NTSC SD 1953 standard, with ITU-R BT.470-2 System M color primaries;
 *  - ITU-R BT.470-2 System B, G, i.e. PAL SD;
 *  - SMPTE 170M, i.e. modern NTSC SD;
 *  - SMPTE 240M (1987 standard), the legacy ATSC HD;
 *  - BT2020 NCL, UHD
 *  - BT2020 CL, UHD
 **************************************************************************/

/***************************************************************************
 * MA
 */

/* BT.709 (i.e. HD) YCbCr -> RGB Primaries */
static const BVDC_P_CscCoeffs s_CMP_HdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
	(  1.168950,  0.000000,  1.799682, -249.062527,
	   1.168950, -0.214073, -0.535094,   77.190243,
	   1.168950,  2.120703,  0.000000, -290.153216 );

/* 170M (i.e moden Ntsc SD) YCbCr -> BT.709 (i.e. HD) R'G'B' */
static const BVDC_P_CscCoeffs s_CMP_NtscSdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
	(  1.168950,  0.001075,  1.464459, -206.291588,
	   1.168950, -0.346562, -0.759761,  122.906169,
	   1.168950,  2.038995,  0.000968, -279.818453	);

/* 472M (i.e Pal SD) YCbCr -> BT.709 (i.e. HD) R'G'B' */
static const BVDC_P_CscCoeffs s_CMP_PalSdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
	(  1.172936,  0.017306,  1.714167, -240.395562,
	   1.167872, -0.392936, -0.815404,  135.981513,
	   1.167882,  1.994798, -0.009616, -272.789425 );

/* 240M (i.e. 1987 ATSC HD) YCbCr -> BT.709 (i.e. HD) R'G'B' */
static const BVDC_P_CscCoeffs s_CMP_240MHdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
	(  1.168950,  0.008449,  1.664915, -232.893802,
	   1.168950, -0.215840, -0.494068,   72.164925,
	   1.168950,  2.100492, -0.000540, -287.496988 );

/* FCC (i.e. 1953 NTSC SD) YCbCr -> BT.709 (i.e. HD) R'G'B' */
static const BVDC_P_CscCoeffs s_CMP_FccSdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
	(  1.229483, -0.021351,  2.719926, -365.089315,
	   1.139184, -0.233738, -0.807431,  115.042689,
	   1.285562,  2.378239, -0.010828, -323.597494 );

/* XvYCCSD (i.e. xvYCC BT.601) YCbCr -> BT.709 (i.e. HD) R'G'B' */
static const BVDC_P_CscCoeffs s_CMP_XvYCCSdYCbCr_to_BT709RGB = BVDC_P_MAKE_CMP_CSC
	(  1.168950,  0.000006,  1.602516, -223.826139,
	   1.168950, -0.394864, -0.816486,  136.349644,
	   1.168950,  2.024130, -0.000003, -277.791453 );

/* BT.2020 (i.e. UHD) YCbCr -> RGB Primaries */
const BVDC_P_CscCoeffs s_CMP_UhdYCbCr_to_BT2020RGB = BVDC_P_MAKE_CMP_CSC
	(  1.168950,  0.000000,  1.685257, -234.416111,
	   1.168950, -0.188061, -0.652975,   88.949376,
	   1.168950,  2.150171,  0.000000, -293.925139 );

/***************************************************************************
 * MC
 */

/* BT.709 (i.e. HD) R'G'B' ->  BT.709 (i.e. HD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_BT709RGB_to_HdYCbCr = BVDC_P_MAKE_CMP_CSC
	(  0.181906,  0.611804,  0.061758,   16.000000,
	  -0.100268, -0.337232,  0.437500,  128.000000,
	   0.437500, -0.397386, -0.040114,  128.000000 );

/* BT.709 (i.e. HD) R'G'B' -> 170M (i.e moden Ntsc SD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_BT709RGB_to_NtscSdYCbCr = BVDC_P_MAKE_CMP_CSC
	(  0.262197,  0.496284,  0.096988,   16.000000,
	  -0.152227, -0.289903,  0.442130,  128.000000,
	   0.473259, -0.395563, -0.077697,  128.000000 );

/* BT.709 (i.e. HD) R'G'B' -> 472M (i.e Pal SD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_BT709RGB_to_PalSdYCbCr = BVDC_P_MAKE_CMP_CSC
	(  0.243560,  0.501559,  0.110080,   16.000000,
	  -0.142304, -0.300979,  0.443891,  128.000000,
	   0.417802, -0.339846, -0.079766,  128.000000 );

/* BT.709 (i.e. HD) R'G'B' -> XvYCCSD (i.e. xvYCC BT.601) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_BT709RGB_to_XvYCCSdYCbCr = BVDC_P_MAKE_CMP_CSC
	(  0.255702,  0.501865,  0.097902,   16.000013,
	  -0.147669, -0.289831,  0.437500,  127.999975,
	   0.437499, -0.366083, -0.071416,  128.000064 );

/* RGB Primaries -> BT.2020 (i.e. UHD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_BT2020RGB_to_UhdYCbCr = BVDC_P_MAKE_CMP_CSC
	(  0.224732,  0.580008,  0.050729,   16.000000,
	  -0.122176, -0.315324,  0.437500,  128.000000,
	   0.437500, -0.402312, -0.035188,  128.000000 );

#endif /* #if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC */

/***************************************************************************
 * color matrix (YCbCr->RGB) for BVDC_P_Csc_ApplyAttenuationRGB_isr
 * -----------------------------------------------------
 * Color Space Standards:
 *  - ITU-R BT.709, i.e. ATSC HD or PAL HD;
 *  - FCC, i.e. NTSC SD 1953 standard, with ITU-R BT.470-2 System M color primaries;
 *  - ITU-R BT.470-2 System B, G, i.e. PAL SD;
 *  - SMPTE 170M, i.e. modern NTSC SD;
 *  - SMPTE 240M (1987 standard), the legacy ATSC HD;
 **************************************************************************/

/* BT.709 (i.e. HD) YCbCr -> RGB Primaries */
static const BVDC_P_CscCoeffs s_CMP_HDYCbCr_to_RGBPrim = BVDC_P_MAKE_CMP_CSC
	(  1.168950,  0.000000,  1.799682, -249.062527,
	   1.168950, -0.214073, -0.535094,   77.190243,
	   1.168950,  2.120703,  0.000000, -290.153216 );

/* FCC (i.e. 1953 NTSC) -> RGB Primaries,
   the same as  SMPTE 170M (i.e. modern SD NTSC) */

/* BT.470-2 System B, G (i.e. SD Pal) -> RGB Primaries,
   the same as  SMPTE 170M (i.e. modern SD NTSC) */

/* SMPTE 170M (i.e. modern SD NTSC) YCbCr -> RGB Primaries */
static const BVDC_P_CscCoeffs s_CMP_NtscSDYCbCr_to_RGBPrim = BVDC_P_MAKE_CMP_CSC
	(  1.168950,  0.000000,  1.602425, -223.813572,
	   1.168950, -0.394860, -0.816582,  136.361359,
	   1.168950,  2.024147, -0.000000, -277.794001 );

/* SMPTE 240M (i.e. 1987 ATSC HD) -> RGB Primaries */
static const BVDC_P_CscCoeffs s_CMP_240MHDYCbCr_to_RGBPrim = BVDC_P_MAKE_CMP_CSC
	(  1.168950,  0.000000,  1.800283, -249.139370,
	   1.168950, -0.257799, -0.545371,   84.102524,
	   1.168950,  2.087854,  0.000000, -285.948536 );

/* BT.2020 (i.e. UHD) YCbCr -> RGB Primaries */
static const BVDC_P_CscCoeffs s_CMP_UHDYCbCr_to_RGBPrim = BVDC_P_MAKE_CMP_CSC
	(  1.168950,  0.000000,  1.685257, -234.416111,
	   1.168950, -0.188061, -0.652975,   88.949376,
	   1.168950,  2.150171,  0.000000, -293.925139 );

/***************************************************************************
 * color matrix (RGB->YCbCr) for BVDC_P_Csc_ApplyAttenuationRGB_isr
 * -----------------------------------------------------
 * Color Space Standards:
 *  - ITU-R BT.709, i.e. ATSC HD or PAL HD;
 *  - ITU-R BT.470-2 System B, G, i.e. PAL SD;
 *  - SMPTE 170M, i.e. modern NTSC SD;
 *  - Custom Panel, with source white points for proper functionality of CCB;
 **************************************************************************/

/* RGB Primaries -> BT.709 (i.e. HD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_RGBPrim_to_HDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  0.181906,  0.611804,  0.061758,   16.000000,
	  -0.100268, -0.337232,  0.437500,  128.000000,
	   0.437500, -0.397386, -0.040114,  128.000000 );

/* RGB Primaries -> SMPTE 170M (i.e. modern SD NTSC) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_RGBPrim_to_NtscSDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  0.255733,  0.501839,  0.097896,   16.000000,
	  -0.147686, -0.289814,  0.437500,  128.000000,
	   0.437500, -0.366086, -0.071414,  128.000000 );

/* RGB Primaries -> BT.470-2 System B, G (i.e. SD Pal) YCbCr,
   the same as  SMPTE 170M (i.e. modern SD NTSC) */

/* MC matrices for Custom Panels, using source white points,
   can be replaced with panel specific matrices. */
/* RGB Primaries -> Custom Panel YCbCr, using BT.709 white points */
static const BVDC_P_CscCoeffs s_CMP_RGBPrim_to_CustomYCbCr_HDWp = BVDC_P_MAKE_CMP_CSC
	(  0.208948,  0.595680,  0.050842,   16.000000,
	  -0.113611, -0.323889,  0.437500,  128.000000,
	   0.437500, -0.403096, -0.034404,  128.000000 );

/* RGB Primaries -> Custom Panel YCbCr, using SMPTE 170M (i.e. modern SD NTSC) white points */
static const BVDC_P_CscCoeffs s_CMP_RGBPrim_to_CustomYCbCr_NtscWp = BVDC_P_MAKE_CMP_CSC
	(  0.217468,  0.582272,  0.055729,   16.000000,
	  -0.118966, -0.318534,  0.437500,  128.000000,
	   0.437500, -0.399285, -0.038215,  128.000000 );

/* RGB Primaries -> BT.2020 (i.e. UHD) YCbCr */
static const BVDC_P_CscCoeffs s_CMP_RGBPrim_to_UHDYCbCr = BVDC_P_MAKE_CMP_CSC
	(  0.224732,  0.580008,  0.050729,   16.000000,
	  -0.122176, -0.315324,  0.437500,  128.000000,
	   0.437500, -0.402312, -0.035188,  128.000000 );

/* TODO:  get real matrix for UHD
 */
#define s_CMP_FccSDYCbCr_to_UHDYCbCr         s_CMP_FccSDYCbCr_to_HDYCbCr
#define s_CMP_FccSDRGBPrim_to_BT2020RGBPrim  s_CMP_FccSDRGBPrim_to_BT709RGBPrim
#define s_CMP_BT2020RGBPrim_to_CustomRGBPrim s_CMP_BT709RGBPrim_to_CustomRGBPrim
#define s_CMP_RGBPrim_to_CustomYCbCr_UHDWp   s_CMP_RGBPrim_to_CustomYCbCr_HDWp

/* used to lookup for an src MatrixCoefficients enum, that is used to decide which matrix to
 * be set to mosaic matrix entry [i]. The matrix should transfer from the src MatrixCoefficients
 * to the current color space of the compositor */
const BVDC_P_MatrixCoefficients s_aCMP_MosaicCscType_To_ClrSpace_MapTbl[] =
{
	BVDC_P_MatrixCoefficients_eItu_R_BT_709,       /* BVDC_P_CmpColorSpace_eHdYCrCb, BT.709 */
	BVDC_P_MatrixCoefficients_eSmpte_170M,         /* BVDC_P_CmpColorSpace_eNtscSdYCrCb, same as BT. 601 except for color primaries */
	BVDC_P_MatrixCoefficients_eItu_R_BT_470_2_BG,  /* BVDC_P_CmpColorSpace_ePalSdYCrCb, same as BT. 601*/
	BVDC_P_MatrixCoefficients_eItu_R_BT_2020_NCL,  /* BVDC_P_CmpColorSpace_eUhdYCrCb, BT. 2020 */
	BVDC_P_MatrixCoefficients_eSmpte_240M,         /* BVDC_P_CmpColorSpace_eSmpte_240M, legacy HD(1987) */
	BVDC_P_MatrixCoefficients_eXvYcc_601           /* BVDC_P_CmpColorSpace_eXvYcc_601, */
};

/* used to lookup for the index (the i) to the pre-programmed mosaic matrix array in the registers, for the thumb to use
 * must return a value <= BVDC_P_CmpColorSpace_eXvYcc_601 */
const BVDC_P_CmpColorSpace s_aCMP_ClrSpace_To_MosaicCscType_MapTbl[] =
{
	BVDC_P_CmpColorSpace_eNtscSdYCrCb,             /* BVDC_P_MatrixCoefficients_eHdmi_RGB           */
	BVDC_P_CmpColorSpace_eHdYCrCb,                 /* BVDC_P_MatrixCoefficients_eItu_R_BT_709       */
	BVDC_P_CmpColorSpace_eNtscSdYCrCb,             /* BVDC_P_MatrixCoefficients_eUnknown            */
	BVDC_P_CmpColorSpace_eNtscSdYCrCb,             /* BVDC_P_MatrixCoefficients_eDvi_Full_Range_RGB */
	BVDC_P_CmpColorSpace_eNtscSdYCrCb,             /* BVDC_P_MatrixCoefficients_eFcc                */
	BVDC_P_CmpColorSpace_ePalSdYCrCb,              /* BVDC_P_MatrixCoefficients_eItu_R_BT_470_2_BG  */
	BVDC_P_CmpColorSpace_eNtscSdYCrCb,             /* BVDC_P_MatrixCoefficients_eSmpte_170M         */
	BVDC_P_CmpColorSpace_eSmpte_240M,              /* BVDC_P_MatrixCoefficients_eSmpte_240M         */
	BVDC_P_CmpColorSpace_eNtscSdYCrCb,             /* BVDC_P_MatrixCoefficients_eUnknown            */
	BVDC_P_CmpColorSpace_eUhdYCrCb,                /* BVDC_P_MatrixCoefficients_eItu_R_BT_2020_NCL  */
	BVDC_P_CmpColorSpace_eUhdYCrCb,                /* BVDC_P_MatrixCoefficients_eItu_R_BT_2020_CL   */
	BVDC_P_CmpColorSpace_eHdYCrCb,                 /* BVDC_P_MatrixCoefficients_eXvYcc_709          */
	BVDC_P_CmpColorSpace_eXvYcc_601                /* BVDC_P_MatrixCoefficients_eXvYcc_601          */
};

/****************************** Set One *********************************/
/* the compositor color matrices table WITH Color Primaries Matching */
static const BVDC_P_CscCoeffs *const s_aaCMP_YCbCr_MatrixTbl[][6] =
{
	/* BVDC_P_MatrixCoefficients_eHdmi_RGB = 0 */
	{
		NULL,
	},

	/* BVDC_P_MatrixCoefficients_eItu_R_BT_709 = 1, */
	{
		&s_CMP_Identity_YCbCr_to_YCbCr,    /* HD -> HD, XvYCC_HD */
		&s_CMP_HDYCbCr_to_NtscSDYCbCr,     /* HD -> NTSC */
		&s_CMP_HDYCbCr_to_PalSDYCbCr,      /* HD -> PAL */
		&s_CMP_HDYCbCr_to_UHDYCbCr,        /* HD -> UHD */
		NULL,                              /* -> SMPTE_240M, not supported */
		&s_CMP_HDYCbCr_to_XvYCCSDYCbCr     /* HD -> XvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eUnknown = 2 */
	{
		&s_CMP_Identity_YCbCr_to_YCbCr,    /* Identity */
		&s_CMP_Identity_YCbCr_to_YCbCr,    /* Identity */
		&s_CMP_Identity_YCbCr_to_YCbCr,    /* Identity */
		&s_CMP_Identity_YCbCr_to_YCbCr,    /* Identity */
		&s_CMP_Identity_YCbCr_to_YCbCr,    /* Identity */
		&s_CMP_Identity_YCbCr_to_YCbCr     /* Identity */
	},

	/* forbidden 3 */
	{
		NULL,
	},

	/* BVDC_P_MatrixCoefficients_eFCC = 4 */
	{
		&s_CMP_FccSDYCbCr_to_HDYCbCr,       /* FCC -> HD, XvYCC_HD */
		&s_CMP_FccSDYCbCr_to_NtscSDYCbCr,   /* FCC -> NTSC */
		&s_CMP_FccSDYCbCr_to_PalSDYCbCr,    /* FCC -> PAL */
		&s_CMP_FccSDYCbCr_to_UHDYCbCr,      /* FCC -> UHD */
		NULL,                               /* -> SMPTE_240M, not supported */
		&s_CMP_FccSDYCbCr_to_XvYCCSDYCbCr   /* FCC -> XvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eItu_R_BT_470_2_BG = 5 */
	{
		&s_CMP_PalSDYCbCr_to_HDYCbCr,       /* PAL -> HD, XvYCC_HD */
		&s_CMP_PalSDYCbCr_to_NtscSDYCbCr,   /* PAL -> NTSC */
		&s_CMP_Identity_YCbCr_to_YCbCr,     /* PAL -> PAL */
		&s_CMP_PalSDYCbCr_to_UHDYCbCr,      /* PAL -> UHD */
		NULL,                               /* -> SMPTE_240M, not supported */
		&s_CMP_PalSDYCbCr_to_XvYCCSDYCbCr   /* PAL -> XvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eSmpte_170M = 6 */
	{
		&s_CMP_NtscSDYCbCr_to_HDYCbCr,      /* NTSC -> HD, XvYCC_HD */
		&s_CMP_Identity_YCbCr_to_YCbCr,     /* NTSC -> NTSC */
		&s_CMP_NtscSDYCbCr_to_PalSDYCbCr,   /* NTSC -> PAL */
		&s_CMP_NtscSDYCbCr_to_UHDYCbCr,     /* NTSC -> UHD */
		NULL,                               /* -> SMPTE_240M, not supported */
		&s_CMP_NtscSDYCbCr_to_XvYCCSDYCbCr  /* NTSC -> XvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eSmpte_240M = 7 */
	{
		&s_CMP_240MHDYCbCr_to_HDYCbCr,      /* 240M -> HD, XvYCC_HD */
		&s_CMP_240MHDYCbCr_to_NtscSDYCbCr,  /* 240M -> NTSC */
		&s_CMP_240MHDYCbCr_to_PalSDYCbCr,   /* 240M -> PAL */
		&s_CMP_240MHDYCbCr_to_UHDYCbCr,     /* 240M -> UHD */
		NULL,                               /* -> SMPTE_240M, not supported */
		&s_CMP_240MHDYCbCr_to_XvYCCSDYCbCr  /* 240M -> XvYCC_SD */
	},

	/* unused 8 */
	{
		NULL,
	},

	/* BVDC_P_MatrixCoefficients_eItu_R_BT_2020_NCL = 9, */
	{
		&s_CMP_UHDYCbCr_to_HDYCbCr,        /* UHD -> HD, XvYCC_HD */
		&s_CMP_UHDYCbCr_to_NtscSDYCbCr,    /* UHD -> NTSC */
		&s_CMP_UHDYCbCr_to_PalSDYCbCr,     /* UHD -> PAL */
		&s_CMP_Identity_YCbCr_to_YCbCr,    /* UHD -> UHD */
		NULL,                              /* -> SMPTE_240M, not supported */
		&s_CMP_UHDYCbCr_to_XvYCCSDYCbCr    /* UHD -> XvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eItu_R_BT_2020_CL = 10, */
	{
		&s_CMP_UHDYCbCr_to_HDYCbCr,        /* UHD -> HD, XvYCC_HD */
		&s_CMP_UHDYCbCr_to_NtscSDYCbCr,    /* UHD -> NTSC */
		&s_CMP_UHDYCbCr_to_PalSDYCbCr,     /* UHD -> PAL */
		&s_CMP_Identity_YCbCr_to_YCbCr,    /* UHD -> UHD */
		NULL,                              /* -> SMPTE_240M, not supported */
		&s_CMP_UHDYCbCr_to_XvYCCSDYCbCr    /* UHD -> XvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eXvYCC_709 = 11 */
	{
		&s_CMP_Identity_YCbCr_to_YCbCr,    /* HD -> HD, XvYCC_HD */
		&s_CMP_HDYCbCr_to_NtscSDYCbCr,     /* HD -> NTSC */
		&s_CMP_HDYCbCr_to_PalSDYCbCr,      /* HD -> PAL */
		&s_CMP_HDYCbCr_to_UHDYCbCr,        /* HD -> UHD */
		NULL,                              /* -> SMPTE_240M, not supported */
		&s_CMP_HDYCbCr_to_XvYCCSDYCbCr     /* HD -> XvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eXvYCC_601 = 12 */
	{
		&s_CMP_XvYCCSDYCbCr_to_HDYCbCr,     /* XvYCC_SD -> HD, XvYCC_HD */
		&s_CMP_XvYCCSDYCbCr_to_NtscSDYCbCr, /* XvYCC_SD -> NTSC */
		&s_CMP_XvYCCSDYCbCr_to_PalSDYCbCr,  /* XvYCC_SD -> PAL */
		&s_CMP_XvYCCSDYCbCr_to_UHDYCbCr,    /* XvYCC_SD -> UHD */
		NULL,                               /* -> SMPTE_240M, not supported */
		&s_CMP_Identity_YCbCr_to_YCbCr      /* XvYCC_SD -> XvYCC_SD */
	}
};

/****************************** Set Two *********************************/
/* the compositor color matrices table WITHOUT Color Primaries Matching */
static const BVDC_P_CscCoeffs * const s_aaCMP_YCbCr_MatrixTbl_II[][6] =
{
	/* BVDC_P_MatrixCoefficients_eHdmi_RGB = 0 */
	{
		NULL,
	},

	/* BVDC_P_MatrixCoefficients_eItu_R_BT_709 = 1 */
	{
		&s_CMP_Identity_YCbCr_to_YCbCr,     /* HD -> HD */
		&s_CMP_HDYCbCr_to_NtscSDYCbCr_II,   /* HD -> NTSC */
		&s_CMP_HDYCbCr_to_NtscSDYCbCr_II,   /* HD -> PAL */
		&s_CMP_HDYCbCr_to_UHDYCbCr_II,      /* HD -> UHD */
		NULL,                               /* -> SMPTE_240M, not supported */
		&s_CMP_HDYCbCr_to_NtscSDYCbCr_II    /* HD -> xvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eUnknown = 2 */
	{
		&s_CMP_Identity_YCbCr_to_YCbCr,     /* Identity */
		&s_CMP_Identity_YCbCr_to_YCbCr,     /* Identity */
		&s_CMP_Identity_YCbCr_to_YCbCr,     /* Identity */
		&s_CMP_Identity_YCbCr_to_YCbCr,     /* Identity */
		NULL,                               /* -> SMPTE_240M, not supported */
		&s_CMP_Identity_YCbCr_to_YCbCr      /* Identity */
	},

	/* forbidden 3 */
	{
		NULL,
	},

	/* BVDC_P_MatrixCoefficients_eFCC = 4 */
	{
		&s_CMP_NtscSDYCbCr_to_HDYCbCr_II,    /* FCC -> HD */
		&s_CMP_Identity_YCbCr_to_YCbCr,      /* FCC -> NTSC */
		&s_CMP_Identity_YCbCr_to_YCbCr,      /* FCC -> PAL */
		&s_CMP_NtscSDYCbCr_to_UHDYCbCr_II,   /* FCC -> UHD */
		NULL,                                /* -> SMPTE_240M, not supported */
		&s_CMP_Identity_YCbCr_to_YCbCr       /* FCC -> xvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eItu_R_BT_470_2_BG = 5 */
	{
		&s_CMP_NtscSDYCbCr_to_HDYCbCr_II,    /* PAL -> HD */
		&s_CMP_Identity_YCbCr_to_YCbCr,      /* PAL -> NTSC */
		&s_CMP_Identity_YCbCr_to_YCbCr,      /* PAL -> PAL */
		&s_CMP_NtscSDYCbCr_to_UHDYCbCr_II,   /* FCC -> UHD */
		NULL,                                /* -> SMPTE_240M, not supported */
		&s_CMP_Identity_YCbCr_to_YCbCr       /* PAL -> xvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eSmpte_170M = 6 */
	{
		&s_CMP_NtscSDYCbCr_to_HDYCbCr_II,    /* NTSC -> HD */
		&s_CMP_Identity_YCbCr_to_YCbCr,      /* NTSC -> NTSC */
		&s_CMP_Identity_YCbCr_to_YCbCr,      /* NTSC -> PAL */
		&s_CMP_NtscSDYCbCr_to_UHDYCbCr_II,   /* FCC -> UHD */
		NULL,                                /* -> SMPTE_240M, not supported */
		&s_CMP_Identity_YCbCr_to_YCbCr       /* NTSC -> xvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eSmpte_240M = 7 */
	{
		&s_CMP_240MHDYCbCr_to_HDYCbCr_II,    /* 240M -> HD */
		&s_CMP_240MHDYCbCr_to_NtscSDYCbCr_II,/* 240M -> NTSC */
		&s_CMP_240MHDYCbCr_to_NtscSDYCbCr_II,/* 240M -> PAL */
		&s_CMP_240MHDYCbCr_to_UHDYCbCr_II,   /* 240M -> UHD */
		NULL,                                /* -> SMPTE_240M, not supported */
		&s_CMP_240MHDYCbCr_to_NtscSDYCbCr_II /* 240M -> xvYCC_SD */
	},

	/* forbidden 8 */
	{
		NULL,
	},

	/* BVDC_P_MatrixCoefficients_eItu_R_BT_2020_NCL = 9 */
	{
		&s_CMP_UHDYCbCr_to_HDYCbCr_II,       /* UHD -> HD */
		&s_CMP_UHDYCbCr_to_NtscSDYCbCr_II,   /* UHD -> NTSC */
		&s_CMP_UHDYCbCr_to_NtscSDYCbCr_II,   /* UHD -> PAL */
		&s_CMP_Identity_YCbCr_to_YCbCr,      /* UHD -> UHD */
		NULL,                                /* -> SMPTE_240M, not supported */
		&s_CMP_UHDYCbCr_to_NtscSDYCbCr_II    /* UHD -> xvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eItu_R_BT_2020_CL = 10 */
	{
		&s_CMP_UHDYCbCr_to_HDYCbCr_II,       /* UHD -> HD */
		&s_CMP_UHDYCbCr_to_NtscSDYCbCr_II,   /* UHD -> NTSC */
		&s_CMP_UHDYCbCr_to_NtscSDYCbCr_II,   /* UHD -> PAL */
		&s_CMP_Identity_YCbCr_to_YCbCr,      /* UHD -> UHD */
		NULL,                                /* -> SMPTE_240M, not supported */
		&s_CMP_UHDYCbCr_to_NtscSDYCbCr_II    /* UHD -> xvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eXvYCC_709 = 11 */
	{
		&s_CMP_Identity_YCbCr_to_YCbCr,     /* HD -> HD */
		&s_CMP_HDYCbCr_to_NtscSDYCbCr_II,   /* HD -> NTSC */
		&s_CMP_HDYCbCr_to_NtscSDYCbCr_II,   /* HD -> PAL */
		&s_CMP_HDYCbCr_to_UHDYCbCr_II,      /* HD -> UHD */
		NULL,                               /* -> SMPTE_240M, not supported */
		&s_CMP_HDYCbCr_to_NtscSDYCbCr_II    /* HD -> xvYCC_SD */
	},

	/* BVDC_P_MatrixCoefficients_eXvYCC_601 = 12 */
	{
		&s_CMP_NtscSDYCbCr_to_HDYCbCr_II,   /* XvYCC_SD -> HD, XvYCC_HD */
		&s_CMP_Identity_YCbCr_to_YCbCr,     /* XvYCC_SD -> NTSC */
		&s_CMP_Identity_YCbCr_to_YCbCr,     /* XvYCC_SD -> PAL */
		&s_CMP_XvYCCSDYCbCr_to_UHDYCbCr_II, /* XvYCC_SD -> UHD */
		NULL,                               /* -> SMPTE_240M, not supported */
		&s_CMP_Identity_YCbCr_to_YCbCr      /* XvYCC_SD -> XvYCC_SD */
	}

};

#if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC
/****************************** Non-linear Set *********************************/
static const BVDC_P_CscCoeffs *const s_aCMP_MA_TO_UHD_MatrixTbl[] =
{
	NULL,                             /* BVDC_P_MatrixCoefficients_eHdmi_RGB = 0 */
	&s_CMP_HdYCbCr_to_BT709RGB,       /* BVDC_P_MatrixCoefficients_eItu_R_BT_709 = 1, */
	&s_CMP_HdYCbCr_to_BT709RGB,       /* BVDC_P_MatrixCoefficients_eUnknown = 2 */
	NULL,                             /* forbidden 3 */
	&s_CMP_FccSdYCbCr_to_BT709RGB,    /* BVDC_P_MatrixCoefficients_eFCC = 4 */
	&s_CMP_PalSdYCbCr_to_BT709RGB,    /* BVDC_P_MatrixCoefficients_eItu_R_BT_470_2_BG = 5 */
	&s_CMP_NtscSdYCbCr_to_BT709RGB,   /* BVDC_P_MatrixCoefficients_eSmpte_170M = 6 */
	&s_CMP_240MHdYCbCr_to_BT709RGB,   /* BVDC_P_MatrixCoefficients_eSmpte_240M = 7 */
	NULL,                             /* forbidden 8 */
	&s_CMP_Identity_YCbCr_to_YCbCr,   /* BVDC_P_MatrixCoefficients_eItu_R_BT_2020_NCL = 9, */
	&s_CMP_Identity_YCbCr_to_YCbCr,   /* BVDC_P_MatrixCoefficients_eItu_R_BT_2020_CL = 10, */
	&s_CMP_HdYCbCr_to_BT709RGB,       /* BVDC_P_MatrixCoefficients_eXvYCC_709 = 11, */
	&s_CMP_XvYCCSdYCbCr_to_BT709RGB   /* BVDC_P_MatrixCoefficients_eXvYCC_601 = 12 */
};

static const BVDC_P_CscCoeffs *const s_aCMP_BT709RGB_to_YCbCr_MatrixTbl[] =
{
	&s_CMP_BT709RGB_to_HdYCbCr,       /* HD, XvYCC_HD */
	&s_CMP_BT709RGB_to_NtscSdYCbCr,   /* NTSC */
	&s_CMP_BT709RGB_to_PalSdYCbCr,    /* PAL */
	NULL,                             /* UHD:  */
	NULL,                             /* SMPTE_240M, not supported */
	&s_CMP_BT709RGB_to_XvYCCSdYCbCr   /* XvYCC_SD */
};
#endif /* #if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC */

/* color matrix (YCbCr->RGB) for BVDC_P_Csc_ApplyAttenuationRGB_isr */
static const BVDC_P_CscCoeffs *const s_aCMP_YCbCr_to_RGBPrim_MatrixTbl[] =
{
	&s_CMP_HDYCbCr_to_RGBPrim,        /* HD, XvYCC_HD */
	&s_CMP_NtscSDYCbCr_to_RGBPrim,    /* NTSC */
	&s_CMP_NtscSDYCbCr_to_RGBPrim,    /* PAL */
	&s_CMP_UHDYCbCr_to_RGBPrim,       /* UHD */
	NULL,                             /* SMPTE_240M, not supported */
	&s_CMP_NtscSDYCbCr_to_RGBPrim     /* XvYCC_SD */
};

/* color matrix (RGB->YCbCr) for BVDC_P_Csc_ApplyAttenuationRGB_isr */
/* XvYCC output is handled like non-XvYCC output, and conversions are */
/* only to HD and SD */
static const BVDC_P_CscCoeffs *const s_aCMP_RGBPrim_to_YCbCr_MatrixTbl[] =
{
	&s_CMP_RGBPrim_to_HDYCbCr,        /* HD, XvYCC_HD */
	&s_CMP_RGBPrim_to_NtscSDYCbCr,    /* NTSC */
	&s_CMP_RGBPrim_to_NtscSDYCbCr,    /* PAL */
	&s_CMP_RGBPrim_to_UHDYCbCr,       /* UHD */
	NULL,                             /* SMPTE_240M, not supported */
	&s_CMP_RGBPrim_to_NtscSDYCbCr     /* XvYCC_SD */
};

/***************************************************************************
 * make sure BVDC_P_CmpColorSpace and s_aCMP_ClrSpace_To_MosaicCscType_MapTbl
 * do not violate our code assumption
 */
#if (BDBG_DEBUG_BUILD)
void BVDC_P_AssertEnumAndTables(void)
{
	BVDC_P_MatrixCoefficients eInMatrixCoeff;

	/* check MatrixCoefficients matchness for NTSC, HD, UHD and the last number */
	BDBG_CASSERT(BVDC_P_MatrixCoefficients_eSmpte_170M == (BVDC_P_MatrixCoefficients)BAVC_MatrixCoefficients_eSmpte_170M);
	BDBG_CASSERT(BVDC_P_MatrixCoefficients_eItu_R_BT_709 == (BVDC_P_MatrixCoefficients)BAVC_MatrixCoefficients_eItu_R_BT_709);
	BDBG_CASSERT(BVDC_P_MatrixCoefficients_eItu_R_BT_2020_NCL == (BVDC_P_MatrixCoefficients)BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL);
	BDBG_CASSERT(BVDC_P_MatrixCoefficients_eXvYcc_601 == (BVDC_P_MatrixCoefficients)BAVC_MatrixCoefficients_eXvYCC_601);

#if BVDC_P_SUPPORT_CMP_MOSAIC_CSC
	for (eInMatrixCoeff = 0; eInMatrixCoeff < BVDC_P_MatrixCoefficients_eMax; eInMatrixCoeff++)
	{
		BDBG_ASSERT(s_aCMP_ClrSpace_To_MosaicCscType_MapTbl[eInMatrixCoeff] <= BVDC_P_CmpColorSpace_eXvYcc_601);
	}
#else
	BSTD_UNUSED(eInMatrixCoeff);
#endif
}
#endif

/***************************************************************************
 * Return the desired color space conversion for CSC in compositor.
 *
 */
void BVDC_P_Compositor_GetCscTable_isrsafe
	( BVDC_P_CscCfg                   *pCscCfg,
	  bool                             bCscRgbMatching,
	  BAVC_MatrixCoefficients          eInputColorSpace,
	  BVDC_P_CmpColorSpace             eOutputColorSpace,
	  bool                             bInputXvYcc )
{
	BAVC_MatrixCoefficients eVdcInputColorSpace = eInputColorSpace;

	/* convert to internal xvYCC color space */
	if(bInputXvYcc && bCscRgbMatching)
	{
		if ((BAVC_MatrixCoefficients)eVdcInputColorSpace == BAVC_MatrixCoefficients_eItu_R_BT_709)
		{
			eVdcInputColorSpace = BAVC_MatrixCoefficients_eXvYCC_709;
		}
		else if((BAVC_MatrixCoefficients)eVdcInputColorSpace == BAVC_MatrixCoefficients_eSmpte_170M)
		{
			eVdcInputColorSpace = BAVC_MatrixCoefficients_eXvYCC_601;
		}
		else
		{
			BDBG_WRN(("BAVC_TransferCharacteristics_eIec_61966_2_4 with non xvYCC capable colorspace"));
		}
	}

	/* Ouptut debug msgs */
	BDBG_MSG(("bInputXvYcc = %d, bCscRgbMatching = %d", bInputXvYcc, bCscRgbMatching));
	BDBG_MSG(("eVdcInputColorSpace       = %d", eVdcInputColorSpace));
	BDBG_MSG(("eOutputColorSpace         = %d", eOutputColorSpace));

#if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC
	if (eOutputColorSpace == BVDC_P_CmpColorSpace_eUhdYCrCb)
	{
		pCscCfg->pCscMA = s_aCMP_MA_TO_UHD_MatrixTbl[eVdcInputColorSpace];
		if (eVdcInputColorSpace == BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL)
		{
			pCscCfg->ulNLCnv = BVDC_P_NL_CSC_CTRL_SEL_BYPASS;
			pCscCfg->stCscMC = s_CMP_Identity_YCbCr_to_YCbCr;
		}
		else
		{
			pCscCfg->ulNLCnv = (eVdcInputColorSpace == BAVC_MatrixCoefficients_eItu_R_BT_2020_CL)?
				BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_CL_YCbCr_2_2020_RGB :
				BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_709_RGB_2_2020_RGB;
			pCscCfg->stCscMC = s_CMP_BT2020RGB_to_UhdYCbCr;
		}
	}
	else /* output non-BT2020 */
	{
		pCscCfg->pCscMA = &s_CMP_Identity_YCbCr_to_YCbCr;
		if ((eVdcInputColorSpace == BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL) ||
			(eVdcInputColorSpace == BAVC_MatrixCoefficients_eItu_R_BT_2020_CL ))
		{
			pCscCfg->ulNLCnv = (eVdcInputColorSpace == BAVC_MatrixCoefficients_eItu_R_BT_2020_NCL)?
				BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_NCL_YCbCr_2_709_RGB :
				BCHP_CMP_0_V0_NL_CSC_CTRL_SEL_CONV_R0_CL_YCbCr_2_709_RGB;
			pCscCfg->stCscMC = *(s_aCMP_BT709RGB_to_YCbCr_MatrixTbl[eOutputColorSpace]);
		}
		else
		{
			pCscCfg->ulNLCnv = BVDC_P_NL_CSC_CTRL_SEL_BYPASS;
			if(bCscRgbMatching)
			{
				pCscCfg->stCscMC = *(s_aaCMP_YCbCr_MatrixTbl[eVdcInputColorSpace][eOutputColorSpace]);
			}
			else
			{
				pCscCfg->stCscMC = *(s_aaCMP_YCbCr_MatrixTbl_II[eVdcInputColorSpace][eOutputColorSpace]);
			}
		}
	}
	pCscCfg->bNLXvYcc = ((eVdcInputColorSpace == BAVC_MatrixCoefficients_eXvYCC_601) ||
						 (eVdcInputColorSpace == BAVC_MatrixCoefficients_eXvYCC_709) ||
						 (eOutputColorSpace   == BVDC_P_CmpColorSpace_eXvYcc_601));
	BDBG_MSG(("ulNLCnv %d, bNLXvYcc %d", pCscCfg->ulNLCnv, pCscCfg->bNLXvYcc));
#else /* #if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC */
	if(bCscRgbMatching)
	{
		pCscCfg->stCscMC = *(s_aaCMP_YCbCr_MatrixTbl[eVdcInputColorSpace][eOutputColorSpace]);
	}
	else
	{
		pCscCfg->stCscMC = *(s_aaCMP_YCbCr_MatrixTbl_II[eVdcInputColorSpace][eOutputColorSpace]);
	}
#endif /* #if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC */

	return;
}


/***************************************************************************
 * Return the desired matrices for converting between YCbCr and R'G'B' for
 * BVDC_P_Csc_ApplyAttenuationRGB_isr
 *
 */
void BVDC_P_Compositor_GetCscToApplyAttenuationRGB_isr
	( const BVDC_P_CscCoeffs         **ppYCbCrToRGB,
	  const BVDC_P_CscCoeffs         **ppRGBToYCbCr,
	  BVDC_P_CmpColorSpace             eOutputColorSpace )
{
	/* Ouptut debug msgs */
	BDBG_MSG(("BVDC_P_Compositor_GetCscTables_YCbCr_RGB_isr:"));
	BDBG_MSG(("eOutputColorSpace         = %d", eOutputColorSpace));

	*ppYCbCrToRGB = s_aCMP_YCbCr_to_RGBPrim_MatrixTbl[eOutputColorSpace];
	BDBG_ASSERT(*ppYCbCrToRGB);
	*ppRGBToYCbCr = s_aCMP_RGBPrim_to_YCbCr_MatrixTbl[eOutputColorSpace];
	BDBG_ASSERT(*ppRGBToYCbCr);

	return;
}

/* End of file */
