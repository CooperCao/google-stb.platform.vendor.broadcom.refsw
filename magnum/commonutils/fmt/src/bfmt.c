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
 *   Video format module header file
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"
#include "bdbg.h"
#include "bfmt.h"

BDBG_MODULE(BFMT);

#if BFMT_DO_PICK

#include "bfmt_pick.h"

/* get rid of char string to save code size */
#define BFMT_P_MAKE_FMT_1(fmt__, width, height, raster_width, raster_height,       \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation) \
{                     \
	fmt__,            \
	width,            \
	height,           \
	width,            \
	height,           \
	raster_width,     \
	raster_height,    \
	top_active,       \
	top_max_vbi,      \
	bot_max_vbi,      \
	active_space,     \
	vert_freq_mask,   \
	vert_freq,        \
	pxl_freq_mask ,   \
	interlaced,       \
	aspect_ratio,     \
	orientation,      \
	pxl_freq,         \
	NULL,             \
	NULL              \
},

#define BFMT_P_MAKE_FMT_0(fmt__, width, height, raster_width, raster_height,       \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation)

/* 3rd stage: BFMT_P_MAKE_FMT__(1, ...) ==> BFMT_P_MAKE_FMT_1(...)
 *            BFMT_P_MAKE_FMT__(0, ...) ==> BFMT_P_MAKE_FMT_0(...) */
#define BFMT_P_MAKE_FMT__(pick__, fmt__, width, height, raster_width, raster_height,       \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation) \
        BFMT_P_MAKE_FMT_##pick__(fmt__, width, height, raster_width, raster_height,       \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation)

/* 2nd stage: BFMT_P_MAKE_FMT_ ==> BFMT_P_MAKE_FMT__
 *            BFMT_PICK_eNTSC ==> 1 */
#define BFMT_P_MAKE_FMT_(pick_, fmt_, width, height, raster_width, raster_height,       \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation) \
        BFMT_P_MAKE_FMT__(pick_, fmt_, width, height, raster_width, raster_height,       \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation)

/* 1st stage: BFMT_P_MAKE_FMT ==> BFMT_P_MAKE_FMT_
 *            eNTSC ==> BFMT_PICK_eNTSC, BFMT_VideoFmt_eNTSC */
#define BFMT_P_MAKE_FMT(fmt, width, height, raster_width, raster_height,       \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation) \
        BFMT_P_MAKE_FMT_(BFMT_PICK_##fmt, BFMT_VideoFmt_##fmt, width, height, raster_width, raster_height, \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation)

#define BFMT_P_MAKE_BLANK(fmt)

#else /* #if BFMT_DO_PICK */

#if !B_REFSW_MINIMAL
/* user-defined formats and microcodes */
#include "bfmt_custom.c"
#endif

#define BFMT_P_MAKE_FMT__(fmt__, width, height, raster_width, raster_height,   \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation) \
{                     \
	fmt__,            \
	width,            \
	height,           \
	width,            \
	height,           \
	raster_width,     \
	raster_height,    \
	top_active,       \
	top_max_vbi,      \
	bot_max_vbi,      \
	active_space,     \
	vert_freq_mask,   \
	vert_freq,        \
	pxl_freq_mask ,   \
	interlaced,       \
	aspect_ratio,     \
	orientation,      \
	pxl_freq,         \
	BDBG_STRING(#fmt__),\
	NULL              \
},

#define BFMT_P_MAKE_FMT(fmt, width, height, raster_width, raster_height,       \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation) \
        BFMT_P_MAKE_FMT__(BFMT_VideoFmt_##fmt, width, height, raster_width, raster_height, \
	top_active, top_max_vbi, bot_max_vbi, active_space, vert_freq_mask,        \
	vert_freq, pxl_freq_mask, pxl_freq, interlaced, aspect_ratio, orientation)


/* Fill entries in with blank. */
#define BFMT_P_MAKE_BLANK(fmt)    \
	BFMT_P_MAKE_FMT(              \
		fmt,                      \
		BFMT_NTSC_WIDTH,          \
		BFMT_NTSC_HEIGHT,         \
		858,                      \
		525,                      \
		22,                       \
		12,                       \
		12,                       \
		0,                        \
		BFMT_VERT_60Hz,           \
		6000,                     \
		BFMT_PXL_27MHz,           \
		13.50 * BFMT_FREQ_FACTOR, \
		true,                     \
		BFMT_AspectRatio_e4_3,    \
		BFMT_Orientation_e2D)

#endif /* #if BFMT_DO_PICK */

/* Video Format Information!  There are a couple of things needed to be
 * updated beside this one.  Check VDC module!
 * (1) BVDC_P_aFormatInfoTable in bvdc_displayfmt_priv.c
 * (2) s_aFormatDataTable in bvdc_displayfmt_priv.c
 * (3) add macro in section I, II, and III in bfmt_pick.h
 *
 * TODO: Additional update Pixel Frequency needed! */
static const BFMT_VideoInfo s_aVideoFmtInfoTbls[] =
{
	BFMT_P_MAKE_FMT(
		eNTSC,
		BFMT_NTSC_WIDTH,
		BFMT_NTSC_HEIGHT,
		858,
		525,
		22,
		12,
		12,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz | BFMT_PXL_27MHz_MUL_1_001,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eNTSC_J,
		BFMT_NTSC_WIDTH,
		BFMT_NTSC_HEIGHT,
		858,
		525,
		22,
		12,
		12,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz | BFMT_PXL_27MHz_MUL_1_001,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eNTSC_443,
		BFMT_NTSC_WIDTH,
		BFMT_NTSC_HEIGHT,
		858,
		525,
		22,
		12,
		12,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz | BFMT_PXL_27MHz_MUL_1_001,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_B,
		BFMT_PAL_WIDTH,
		BFMT_PAL_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_B1,
		BFMT_PAL_WIDTH,
		BFMT_PAL_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_D,
		BFMT_PAL_WIDTH,
		BFMT_PAL_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_D1,
		BFMT_PAL_WIDTH,
		BFMT_PAL_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_G,
		BFMT_PAL_WIDTH,
		BFMT_PAL_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_H,
		BFMT_PAL_WIDTH,
		BFMT_PAL_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_K,
		BFMT_PAL_WIDTH,
		BFMT_PAL_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_I,
		BFMT_PAL_WIDTH,
		BFMT_PAL_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_M,
		BFMT_PAL_M_WIDTH,
		BFMT_PAL_M_HEIGHT,
		858,
		525,
		22,
		12,
		12,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz | BFMT_PXL_27MHz_MUL_1_001,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_N,
		BFMT_PAL_N_WIDTH,
		BFMT_PAL_N_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_NC,
		BFMT_PAL_NC_WIDTH,
		BFMT_PAL_NC_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		ePAL_60,
		BFMT_NTSC_WIDTH,
		BFMT_NTSC_HEIGHT,
		858,
		525,
		22,
		12,
		12,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz | BFMT_PXL_27MHz_MUL_1_001,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eSECAM_L,
		BFMT_SECAM_WIDTH,
		BFMT_SECAM_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eSECAM_B,
		BFMT_SECAM_WIDTH,
		BFMT_SECAM_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eSECAM_G,
		BFMT_SECAM_WIDTH,
		BFMT_SECAM_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eSECAM_D,
		BFMT_SECAM_WIDTH,
		BFMT_SECAM_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eSECAM_K,
		BFMT_SECAM_WIDTH,
		BFMT_SECAM_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eSECAM_H,
		BFMT_SECAM_WIDTH,
		BFMT_SECAM_HEIGHT,
		864,
		625,
		23,
		17,
		18,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1080i,
		BFMT_1080I_WIDTH,
		BFMT_1080I_HEIGHT,
		2200,
		1125,
		21,
		14,
		15,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz_DIV_1_001 | BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1080p,
		BFMT_1080I_WIDTH,
		BFMT_1080I_HEIGHT,
		2200,
		1125,
		42,
		36,
		36,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_148_5MHz_DIV_1_001 | BFMT_PXL_148_5MHz,
		148.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e720p,
		BFMT_720P_WIDTH,
		BFMT_720P_HEIGHT,
		1650,
		750,
		26,
		20,
		20,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz_DIV_1_001 | BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e720p_60Hz_3DOU_AS,
		BFMT_720P_WIDTH,
		BFMT_720P_HEIGHT,
		1650,
		750 * 2,
		26,
		20,
		20,
		30,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_148_5MHz_DIV_1_001 | BFMT_PXL_148_5MHz,
		148.5 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e3D_OverUnder)

	BFMT_P_MAKE_FMT(
		e720p_50Hz_3DOU_AS,
		BFMT_720P_WIDTH,
		BFMT_720P_HEIGHT,
		1980,
		750 * 2,
		26,
		20,
		20,
		30,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_148_5MHz,
		148.5 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e3D_OverUnder)

	BFMT_P_MAKE_FMT(
		e720p_30Hz_3DOU_AS,
		BFMT_720P_WIDTH,
		BFMT_720P_HEIGHT,
		3300,
		750 * 2,
		30,
		20,
		20,
		30,
		BFMT_VERT_30Hz,
		30 * BFMT_FREQ_FACTOR,
		BFMT_PXL_148_5MHz_DIV_1_001 | BFMT_PXL_148_5MHz,
		148.5 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e3D_OverUnder)

	BFMT_P_MAKE_FMT(
		e720p_24Hz_3DOU_AS,
		BFMT_720P_WIDTH,
		BFMT_720P_HEIGHT,
		4125,
		750 * 2,
		30,
		20,
		20,
		30,
		BFMT_VERT_24Hz | BFMT_VERT_23_976Hz,
		24 * BFMT_FREQ_FACTOR,
		BFMT_PXL_148_5MHz_DIV_1_001 | BFMT_PXL_148_5MHz,
		148.5 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e3D_OverUnder)

	BFMT_P_MAKE_FMT(
		e480p,
		BFMT_480P_WIDTH,
		BFMT_480P_HEIGHT,
		858,
		525,
		43,
		30,
		30,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz_MUL_1_001 | BFMT_PXL_27MHz,
		27 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1080i_50Hz,
		BFMT_1080I_WIDTH,
		BFMT_1080I_HEIGHT,
		2640,
		1125,
		21,
		14,
		15,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1080p_24Hz_3DOU_AS,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT,
		2750,
		1125 * 2,
		42,
		41,
		4,
		45,
		BFMT_VERT_24Hz | BFMT_VERT_23_976Hz,
		24 * BFMT_FREQ_FACTOR,
		BFMT_PXL_148_5MHz_DIV_1_001 | BFMT_PXL_148_5MHz,
		148.5 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e3D_OverUnder)

	BFMT_P_MAKE_FMT(
		e1080p_30Hz_3DOU_AS,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT,
		2200,
		1125 * 2,
		42,
		41,
		4,
		45,
		BFMT_VERT_30Hz,
		30 * BFMT_FREQ_FACTOR,
		BFMT_PXL_148_5MHz_DIV_1_001 | BFMT_PXL_148_5MHz,
		148.5 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e3D_OverUnder)

	BFMT_P_MAKE_FMT(
		e1080p_60Hz_3DOU_AS,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT,
		2200,
		1125 * 2,
		42,
		41,
		4,
		45,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_297MHz_DIV_1_001 | BFMT_PXL_297MHz,
		297 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e3D_OverUnder)

	BFMT_P_MAKE_FMT(
		e1080p_60Hz_3DLR,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT,
		2200 * 2,
		1125,
		42,
		41,
		4,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_297MHz_DIV_1_001 | BFMT_PXL_297MHz,
		297 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e3D_LeftRight)

	BFMT_P_MAKE_FMT(
		e1080p_24Hz,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT,
		2750,
		1125,
		42,
		41,
		4,
		0,
		BFMT_VERT_24Hz | BFMT_VERT_23_976Hz,
		24 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz_DIV_1_001 | BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1080p_25Hz,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT,
		2640,
		1125,
		42,
		41,
		4,
		0,
		BFMT_VERT_25Hz,
		25 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1080p_30Hz,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT,
		2200,
		1125,
		42,
		41,
		4,
		0,
		BFMT_VERT_30Hz,
		30 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz_DIV_1_001 | BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1080p_50Hz,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT,
		2640,
		1125,
		42,
		41,
		4,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_148_5MHz,
		148.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1080p_100Hz,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT,
		2640,
		1125,
		42,
		36,
		36,
		0,
		BFMT_VERT_100Hz,
		100 * BFMT_FREQ_FACTOR,
		BFMT_PXL_297MHz,
		297 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1080p_120Hz,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT,
		2200,
		1125,
		42,
		36,
		36,
		0,
		BFMT_VERT_120Hz,
		120 * BFMT_FREQ_FACTOR,
		BFMT_PXL_297MHz | BFMT_PXL_297MHz_DIV_1_001,
		297 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1250i_50Hz,
		BFMT_1080I_WIDTH,
		BFMT_1080I_HEIGHT,
		2376,
		1250,
		81,
		79,
		79,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e720p_24Hz,
		BFMT_720P_WIDTH,
		BFMT_720P_HEIGHT,
		4125,
		750,
		26,
		20,
		20,
		0,
		BFMT_VERT_24Hz | BFMT_VERT_23_976Hz,
		24 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz_DIV_1_001 | BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e720p_25Hz,
		BFMT_720P_WIDTH,
		BFMT_720P_HEIGHT,
		3960,
		750,
		26,
		20,
		20,
		0,
		BFMT_VERT_25Hz,
		25 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e720p_30Hz,
		BFMT_720P_WIDTH,
		BFMT_720P_HEIGHT,
		3300,
		750,
		26,
		20,
		20,
		0,
		BFMT_VERT_29_97Hz | BFMT_VERT_30Hz,
		30 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz_DIV_1_001 | BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e720p_50Hz,
		BFMT_720P_WIDTH,
		BFMT_720P_HEIGHT,
		1980,
		750,
		26,
		20,
		20,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz,
		74.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e576p_50Hz,
		BFMT_576P_WIDTH,
		BFMT_576P_HEIGHT,
		864,
		625,
		45,
		39,
		39,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		27 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e240p_60Hz,
		720,
		240,
		858 * 2,
		263,
		22,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz,
		59.94 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		27 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e288p_50Hz,
		720,
		BFMT_PAL_HEIGHT/2,
		864 * 2,
		313,
		23,
		0,
		0,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		27 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1440x480p_60Hz,
		BFMT_480P_WIDTH * 2,
		480,
		858 * 2,
		525,
		43,
		30,
		30,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_54MHz_MUL_1_001 | BFMT_PXL_54MHz,
		54 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e1440x576p_50Hz,
		BFMT_576P_WIDTH * 2,
		BFMT_576P_HEIGHT,
		864 * 2,
		625,
		45,
		39,
		39,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_54MHz,
		54 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e3840x2160p_24Hz,
		3840,
		2160,
		5500,
		2250,
		83,
		0,
		0,
		0,
		BFMT_VERT_24Hz | BFMT_VERT_23_976Hz,
		24 * BFMT_FREQ_FACTOR,
		BFMT_PXL_297MHz_DIV_1_001 | BFMT_PXL_297MHz,
		297 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e3840x2160p_25Hz,
		3840,
		2160,
		5280,
		2250,
		83,
		0,
		0,
		0,
		BFMT_VERT_25Hz,
		25 * BFMT_FREQ_FACTOR,
		BFMT_PXL_297MHz,
		297 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e3840x2160p_30Hz,
		3840,
		2160,
		4400,
		2250,
		83,
		0,
		0,
		0,
		BFMT_VERT_29_97Hz | BFMT_VERT_30Hz,
		30 * BFMT_FREQ_FACTOR,
		BFMT_PXL_297MHz_DIV_1_001 | BFMT_PXL_297MHz,
		297 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e3840x2160p_50Hz,
		3840,
		2160,
		5280,
		2250,
		83,
		0,
		0,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_594MHz,
		594 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e3840x2160p_60Hz,
		3840,
		2160,
		4400,
		2250,
		83,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_594MHz_DIV_1_001 | BFMT_PXL_594MHz,
		594 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e4096x2160p_24Hz,
		4096,
		2160,
		5500,
		2250,
		83,
		0,
		0,
		0,
		BFMT_VERT_24Hz,
		24 * BFMT_FREQ_FACTOR,
		BFMT_PXL_297MHz_DIV_1_001 | BFMT_PXL_297MHz,
		297 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e4096x2160p_25Hz,
		4096,
		2160,
		5280,
		2250,
		83,
		0,
		0,
		0,
		BFMT_VERT_25Hz,
		25 * BFMT_FREQ_FACTOR,
		BFMT_PXL_297MHz,
		297 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e4096x2160p_30Hz,
		4096,
		2160,
		4400,
		2250,
		83,
		0,
		0,
		0,
		BFMT_VERT_30Hz,
		30 * BFMT_FREQ_FACTOR,
		BFMT_PXL_297MHz_DIV_1_001 | BFMT_PXL_297MHz,
		297 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e4096x2160p_50Hz,
		4096,
		2160,
		5280,
		2250,
		83,
		0,
		0,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_594MHz,
		594 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e4096x2160p_60Hz,
		4096,
		2160,
		4400,
		2250,
		83,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_594MHz_DIV_1_001 | BFMT_PXL_594MHz,
		594 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

#if BFMT_LEGACY_3DTV_SUPPORT
	BFMT_P_MAKE_FMT(
		eCUSTOM1920x2160i_48Hz,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT * 2,
		2750,
		1125 * 2,
		42,
		41,
		4,
		0,
		BFMT_VERT_48Hz | BFMT_VERT_47_952Hz,
		48 * BFMT_FREQ_FACTOR,
		BFMT_PXL_148_5MHz_DIV_1_001 | BFMT_PXL_148_5MHz,
		148.5 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eCUSTOM1920x2160i_60Hz,
		BFMT_1080P_WIDTH,
		BFMT_1080P_HEIGHT * 2,
		2200,
		1125 * 2,
		42,
		41,
		4,
		0,
		BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_148_5MHz_DIV_1_001 | BFMT_PXL_148_5MHz,
		148.5 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)
#endif

	BFMT_P_MAKE_FMT(
		eCUSTOM_1440x240p_60Hz,
		1440,
		240,
		858 * 2,
		263,
		22,
		0,
		0,
		0,
		BFMT_VERT_60Hz | BFMT_VERT_59_94Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		27 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eCUSTOM_1440x288p_50Hz,
		1440,
		BFMT_PAL_HEIGHT/2,
		864 * 2,
		313,
		23,
		0,
		0,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz,
		27 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eCUSTOM_1366x768p,
		1366,
		768,
		1440,
		782,
		13,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_67_565MHz_DIV_1_001 | BFMT_PXL_67_565MHz,
		67.56 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eCUSTOM_1366x768p_50Hz,
		1366,
		768,
		1440,
		782,
		13,
		0,
		0,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_56_304MHz,
		56.30 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x480p,
		BFMT_DVI_480P_WIDTH,
		BFMT_DVI_480P_HEIGHT,
		800,
		525,
		35,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_25_2MHz_DIV_1_001 | BFMT_PXL_25_2MHz,
		25.20 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x480p_CVT,
		BFMT_DVI_480P_WIDTH,
		BFMT_DVI_480P_HEIGHT,
		800,
		525,
		35,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_23_75MHz | BFMT_PXL_23_75MHz_DIV_1_001,
		23.75 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	/* The following VESA display formats complies with
	   http://www.sj.broadcom.com/projects/dvt/Chip_Architecture/Clocking/Released/DVT_format_support.doc;
	   Note: 59.94/60 Hz support frame rate tracking for TV formats input; */
	BFMT_P_MAKE_FMT(
		eDVI_800x600p,
		BFMT_DVI_600P_WIDTH,
		BFMT_DVI_600P_HEIGHT,
		1056,
		628,
		27,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_39_79MHz | BFMT_PXL_39_79MHz_DIV_1_001,
		39.79 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1024x768p,
		BFMT_DVI_768P_WIDTH,
		BFMT_DVI_768P_HEIGHT,
		1344,
		806,
		35,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_65MHz | BFMT_PXL_65MHz_DIV_1_001,
		64.99 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x768p,
		1280,
		BFMT_DVI_768P_HEIGHT,
		1664,
		798,
		27,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_65_286MHz | BFMT_PXL_65_286MHz_DIV_1_001,
		79.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e15_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x768p_Red,
		1280,
		BFMT_DVI_768P_HEIGHT,
		1440,
		790,
		19,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_65_286MHz | BFMT_PXL_65_286MHz_DIV_1_001,
		68.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e15_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x720p_50Hz,
		BFMT_DVI_720P_WIDTH,
		BFMT_DVI_720P_HEIGHT,
		1632,
		741,
		35,
		0,
		0,
		0,
		BFMT_VERT_50Hz,
		50 * BFMT_FREQ_FACTOR,
		BFMT_PXL_60_4656MHz,
		60.47 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x720p,
		BFMT_DVI_720P_WIDTH,
		BFMT_DVI_720P_HEIGHT,
		1664,
		746,
		35,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_25MHz | BFMT_PXL_74_25MHz_DIV_1_001,
		74.48 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x720p_Red,
		BFMT_DVI_720P_WIDTH,
		BFMT_DVI_720P_HEIGHT,
		1440,
		741,
		35,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_64_022MHz | BFMT_PXL_64_022MHz_DIV_1_001,
		64.02 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x350p_60Hz,
		640,
		350,
		800,
		525,
		75,
		0,
		0,
		0,
		BFMT_VERT_60Hz | BFMT_VERT_59_94Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_25_2MHz_DIV_1_001 | BFMT_PXL_25_2MHz,
		25.20 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x350p_70Hz,
		640,
		350,
		800,
		450,
		62,
		0,
		0,
		0,
		BFMT_VERT_70Hz,
		70 * BFMT_FREQ_FACTOR,
		BFMT_PXL_25_2MHz_DIV_1_001 | BFMT_PXL_25_2MHz,
		25.20 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x350p_72Hz,
		640,
		350,
		832,
		445,
		63,
		0,
		0,
		0,
		BFMT_VERT_72Hz,
		72 * BFMT_FREQ_FACTOR,
		BFMT_PXL_25_2MHz_DIV_1_001 | BFMT_PXL_25_2MHz,
		25.20 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x350p_75Hz,
		640,
		350,
		832,
		445,
		63,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75 * BFMT_FREQ_FACTOR,
		BFMT_PXL_25_2MHz_DIV_1_001 | BFMT_PXL_25_2MHz,
		25.20 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x350p_85Hz,
		640,
		350,
		832,
		445,
		63,
		0,
		0,
		0,
		BFMT_VERT_85Hz,
		85 * BFMT_FREQ_FACTOR,
		BFMT_PXL_31_50MHz,
		31.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x400p_60Hz,
		640,
		400,
		800,
		525,
		50,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_25_2MHz_DIV_1_001 | BFMT_PXL_25_2MHz,
		25.20 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x400p_70Hz,
		640,
		400,
		800,
		450,
		12,
		0,
		0,
		0,
		BFMT_VERT_70Hz,
		70 * BFMT_FREQ_FACTOR,
		BFMT_PXL_25_2MHz_DIV_1_001 | BFMT_PXL_25_2MHz,
		25.20 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x400p_72Hz,
		640,
		400,
		832,
		425,
		12,
		0,
		0,
		0,
		BFMT_VERT_72Hz,
		72 * BFMT_FREQ_FACTOR,
		BFMT_PXL_25_2MHz_DIV_1_001 | BFMT_PXL_25_2MHz,
		25.20 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x400p_75Hz,
		640,
		400,
		832,
		425,
		12,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75 * BFMT_FREQ_FACTOR,
		BFMT_PXL_25_2MHz_DIV_1_001 | BFMT_PXL_25_2MHz,
		25.20 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x400p_85Hz,
		640,
		400,
		832,
		445,
		44,
		0,
		0,
		0,
		BFMT_VERT_85Hz,
		85 * BFMT_FREQ_FACTOR,
		BFMT_PXL_31_50MHz,
		31.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x480p_66Hz,
		640,
		480,
		864,
		525,
		42,
		0,
		0,
		0,
		BFMT_VERT_66Hz,
		66.67 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		30.24 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x480p_70Hz,
		640,
		480,
		832,
		503,
		23,
		0,
		0,
		0,
		BFMT_VERT_70Hz,
		70 * BFMT_FREQ_FACTOR,
		BFMT_PXL_25_2MHz_DIV_1_001 | BFMT_PXL_25_2MHz,
		25.20 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x480p_72Hz,
		640,
		480,
		832,
		520,
		31,
		0,
		0,
		0,
		BFMT_VERT_72Hz,
		72.81 * BFMT_FREQ_FACTOR,
		BFMT_PXL_31_50MHz,
		31.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x480p_75Hz,
		640,
		480,
		840,
		500,
		19,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75 * BFMT_FREQ_FACTOR,
		BFMT_PXL_31_50MHz,
		31.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_640x480p_85Hz,
		640,
		480,
		832,
		509,
		28,
		0,
		0,
		0,
		BFMT_VERT_85Hz,
		8500,
		BFMT_PXL_36_00MHz,
		36 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_720x400p_60Hz,
		720,
		400,
		900,
		525,
		19,
		0,
		0,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_40MHz,
		40 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_720x400p_70Hz,
		720,
		400,
		900,
		449,
		36,
		0,
		0,
		0,
		BFMT_VERT_70Hz,
		70 * BFMT_FREQ_FACTOR,
		BFMT_PXL_31_50MHz,
		28.32 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_720x400p_72Hz,
		720,
		400,
		936,
		525,
		20,
		0,
		0,
		0,
		BFMT_VERT_72Hz,
		72 * BFMT_FREQ_FACTOR,
		BFMT_PXL_40MHz,
		40 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_720x400p_75Hz,
		720,
		400,
		936,
		525,
		21,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75 * BFMT_FREQ_FACTOR,
		BFMT_PXL_40MHz,
		40 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_720x400p_85Hz,
		720,
		400,
		936,
		446,
		45,
		0,
		0,
		0,
		BFMT_VERT_85Hz,
		85 * BFMT_FREQ_FACTOR,
		BFMT_PXL_35_50MHz,
		35.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_800x600p_56Hz,
		800,
		600,
		1024,
		625,
		24,
		0,
		0,
		0,
		BFMT_VERT_56Hz,
		56 * BFMT_FREQ_FACTOR,
		BFMT_PXL_36_00MHz,
		36 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_800x600p_59Hz_Red,
		800,
		600,
		1024,
		624,
		21,
		0,
		0,
		0,
		BFMT_VERT_60Hz |BFMT_VERT_59_94Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		38.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_800x600p_70Hz,
		800,
		400,
		1056,
		628,
		28,
		0,
		0,
		0,
		BFMT_VERT_70Hz,
		70 * BFMT_FREQ_FACTOR,
		BFMT_PXL_50_00MHz,
		50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_800x600p_72Hz,
		800,
		600,
		1040,
		666,
		29,
		0,
		0,
		0,
		BFMT_VERT_72Hz,
		72.19 * BFMT_FREQ_FACTOR,
		BFMT_PXL_50_00MHz,
		50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_800x600p_75Hz,
		800,
		600,
		1056,
		625,
		24,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75 * BFMT_FREQ_FACTOR,
		BFMT_PXL_49_50MHz,
		49.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_800x600p_85Hz,
		800,
		600,
		1048,
		631,
		30,
		0,
		0,
		0,
		BFMT_VERT_85Hz,
		85 * BFMT_FREQ_FACTOR,
		BFMT_PXL_56_25MHz,
		56.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1024x768p_66Hz,
		BFMT_DVI_768P_WIDTH,
		BFMT_DVI_768P_HEIGHT,
		1328,
		816,
		40,
		0,
		0,
		0,
		BFMT_VERT_66Hz,
		66.11 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		71.64 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1024x768p_70Hz,
		BFMT_DVI_768P_WIDTH,
		BFMT_DVI_768P_HEIGHT,
		1328,
		806,
		35,
		0,
		0,
		0,
		BFMT_VERT_70Hz,
		70 * BFMT_FREQ_FACTOR,
		BFMT_PXL_75_00MHz,
		75 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1024x768p_72Hz,
		BFMT_DVI_768P_WIDTH,
		BFMT_DVI_768P_HEIGHT,
		1344,
		806,
		35,
		0,
		0,
		0,
		BFMT_VERT_72Hz,
		72 * BFMT_FREQ_FACTOR,
		BFMT_PXL_65MHz,
		65 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1024x768p_75Hz,
		BFMT_DVI_768P_WIDTH,
		BFMT_DVI_768P_HEIGHT,
		1312,
		800,
		31,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75 * BFMT_FREQ_FACTOR,
		BFMT_PXL_78_75MHz,
		78.75 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1024x768p_85Hz,
		BFMT_DVI_768P_WIDTH,
		BFMT_DVI_768P_HEIGHT,
		1376,
		808,
		39,
		0,
		0,
		0,
		BFMT_VERT_85Hz,
		85 * BFMT_FREQ_FACTOR,
		BFMT_PXL_94_50MHz,
		94.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x720p_70Hz,
		BFMT_DVI_720P_WIDTH,
		BFMT_DVI_720P_HEIGHT,
		1664,
		746,
		32,
		0,
		0,
		0,
		BFMT_VERT_70Hz,
		70.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_375MHz,
		74.37 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x720p_72Hz,
		BFMT_DVI_720P_WIDTH,
		BFMT_DVI_720P_HEIGHT,
		1664,
		746,
		33,
		0,
		0,
		0,
		BFMT_VERT_72Hz,
		72.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_375MHz,
		74.37 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x720p_75Hz,
		BFMT_DVI_720P_WIDTH,
		BFMT_DVI_720P_HEIGHT,
		1664,
		746,
		35,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_375MHz,
		74.37 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x720p_85Hz,
		BFMT_DVI_720P_WIDTH,
		BFMT_DVI_720P_HEIGHT,
		1664,
		746,
		39,
		0,
		0,
		0,
		BFMT_VERT_85Hz,
		85.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_74_375MHz,
		74.37 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1024x768i_87Hz,
		1024,
		768,
		1264,
		817,
		24,
		0,
		0,
		0,
		BFMT_VERT_87Hz,
		87.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_44_900MHz,
		44.90 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1152x864p_75Hz,
		1152,
		864,
		1152+448,
		864+36,
		35,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		108.00 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x768p_75Hz,
		1280,
		768,
		1696,
		805,
		34,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		102.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e15_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x768p_85Hz,
		1280,
		768,
		1712,
		809,
		38,
		0,
		0,
		0,
		BFMT_VERT_85Hz,
		85.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		117.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e15_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x800p_60Hz,
		1280,
		800,
		1680,
		828,
		28,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_83_5MHz | BFMT_PXL_83_5MHz_DIV_1_001,
		83.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e15_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x960p_60Hz,
		1280,
		960,
		1800,
		1000,
		39,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		108.00 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x960p_85Hz,
		1280,
		960,
		1728,
		1011,
		50,
		0,
		0,
		0,
		BFMT_VERT_85Hz,
		85.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		148.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x1024p_60Hz,
		1280,
		1024,
		1712,
		1066,
		39,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_108MHz | BFMT_PXL_108MHz_DIV_1_001,
		108.00 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x1024p_69Hz,
		1280,
		1024,
		1680,
		1063,
		38,
		0,
		0,
		0,
		BFMT_VERT_70Hz,
		69.99 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		125.00 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x1024p_75Hz,
		1280,
		1024,
		1688,
		1066,
		41,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		135.00 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1280x1024p_85Hz,
		1280,
		1024,
		1728,
		1072,
		47,
		0,
		0,
		0,
		BFMT_VERT_85Hz,
		85.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		157.00 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_832x624p_75Hz,
		832,
		624,
		1152,
		667,
		35,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_56_25MHz,
		56.25 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1360x768p_60Hz,
		1360,
		BFMT_DVI_768P_HEIGHT,
		1792,
		795,
		24,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_85_5MHz,
		85.50 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1366x768p_60Hz,
		1366,
		BFMT_DVI_768P_HEIGHT,
		1528,
		790,
		20,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_85_5MHz | BFMT_PXL_85_5MHz_DIV_1_001,
		85.5 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1400x1050p_60Hz,
		1400,
		1050,
		1864,
		1089,
		36,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_121_75MHz,
		121.75 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1400x1050p_60Hz_Red,
		1400,
		1050,
		1560,
		1080,
		27,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_101MHz,
		101.00 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1400x1050p_75Hz,
		1400,
		1050,
		1896,
		1099,
		46,
		0,
		0,
		0,
		BFMT_VERT_75Hz,
		75.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_156MHz,
		156.00 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1600x1200p_60Hz,
		1600,
		1200,
		2160,
		1250,
		49,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_162MHz,
		162.00 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1920x1080p_60Hz_Red,
		1920,
		1080,
		2080,
		1111,
		29,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_138_625MHz,
		138.63 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_848x480p_60Hz,
		848,
		480,
		1056,
		500,
		20,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		59.61 * BFMT_FREQ_FACTOR,
		BFMT_PXL_31_50MHz,
		31.48 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1064x600p_60Hz,
		1064,
		600,
		1352,
		624,
		24,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		59.81 * BFMT_FREQ_FACTOR,
		BFMT_PXL_36_00MHz,
		37.32 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		eDVI_1440x900p_60Hz,
		1440,
		900,
		1904,
		932,
		32,
		0,
		0,
		0,
		BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_106_5MHz | BFMT_PXL_106_5MHz_DIV_1_001,
		106.5 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e16_9,
		BFMT_Orientation_e2D)

	/* SW7435-276: New format enums for 482/483 */
	BFMT_P_MAKE_FMT(
		e720x482_NTSC,
		BFMT_NTSC_WIDTH,
		482,
		858,
		525,
		22,
		12,
		12,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz | BFMT_PXL_27MHz_MUL_1_001,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e720x482_NTSC_J,
		BFMT_NTSC_WIDTH,
		482,
		858,
		525,
		22,
		12,
		12,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz | BFMT_PXL_27MHz_MUL_1_001,
		13.50 * BFMT_FREQ_FACTOR,
		true,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	BFMT_P_MAKE_FMT(
		e720x483p,
		BFMT_480P_WIDTH,
		483,
		858,
		525,
		43,
		30,
		30,
		0,
		BFMT_VERT_59_94Hz | BFMT_VERT_60Hz,
		60.00 * BFMT_FREQ_FACTOR,
		BFMT_PXL_27MHz_MUL_1_001 | BFMT_PXL_27MHz,
		27 * BFMT_FREQ_FACTOR,
		false,
		BFMT_AspectRatio_e4_3,
		BFMT_Orientation_e2D)

	/* custom formats placeholders*/
	BFMT_P_MAKE_BLANK(eCustom0)
	BFMT_P_MAKE_BLANK(eCustom1)
	BFMT_P_MAKE_BLANK(eCustom2)

	/* Must be last */
	BFMT_P_MAKE_BLANK(eMaxCount)
};

/* For table size sanity check */
#define BVDC_P_FMT_INFO_COUNT \
	(sizeof(s_aVideoFmtInfoTbls) / sizeof(BFMT_VideoInfo))

/***************************************************************************
 *
 */
BERR_Code BFMT_GetVideoFormatInfo
	( BFMT_VideoFmt                      eVideoFmt,
	  BFMT_VideoInfo                    *pVideoFmtInfo )
{
	BDBG_ENTER(BFMT_GetVideoFormatInfo);

	if(pVideoFmtInfo)
	{
		const BFMT_VideoInfo* info = BFMT_GetVideoFormatInfoPtr_isr(eVideoFmt);
		if (info == 0x0)
			return BERR_TRACE (BERR_INVALID_PARAMETER);
		*pVideoFmtInfo = *((const BFMT_VideoInfo*)
			BFMT_GetVideoFormatInfoPtr_isr(eVideoFmt));
	}

	BDBG_LEAVE(BFMT_GetVideoFormatInfo);
	return BERR_SUCCESS;
}


/***************************************************************************
 *
 */
#ifndef BFMT_DO_PICK
const BFMT_VideoInfo* BFMT_GetVideoFormatInfoPtr_isr
	( BFMT_VideoFmt                      eVideoFmt )
{
	const BFMT_VideoInfo *pVideoInfo = NULL;
	BDBG_ENTER(BFMT_GetVideoFormatInfoPtr_isr);

	/* Table size sanity check!  Just in case someone added new format in fmt,
	 * but forgot to add the new into these table. */
#if (BDBG_DEBUG_BUILD)
	if(BVDC_P_FMT_INFO_COUNT != BFMT_VideoFmt_eMaxCount+1)
	{
		BDBG_ERR(("Format Look Up Table out of sync!"));
		BDBG_ASSERT(false);
	}
#endif
	BDBG_ASSERT(eVideoFmt < BFMT_VideoFmt_eMaxCount);

	switch (eVideoFmt)
	{
	case BFMT_VideoFmt_eCustom0:
	case BFMT_VideoFmt_eCustom1:
		BDBG_MSG(("%d No longer supported", eVideoFmt));
		pVideoInfo = 0x0;
		break;
	case BFMT_VideoFmt_eCustom2:
		BDBG_MSG(("User defined, bfmt does not have information!"));
		pVideoInfo = 0x0;
		break;
	default:
		pVideoInfo = &s_aVideoFmtInfoTbls[eVideoFmt];
		break;
	}

	BDBG_LEAVE(BFMT_GetVideoFormatInfoPtr_isr);
	return pVideoInfo;
}

#else /* #ifndef BFMT_DO_PICK */
const BFMT_VideoInfo* BFMT_GetVideoFormatInfoPtr_isr
	( BFMT_VideoFmt                      eVideoFmt )
{
	unsigned int ii;

	for(ii=0; ii<BVDC_P_FMT_INFO_COUNT; ii++)
	{
		if (s_aVideoFmtInfoTbls[ii].eVideoFmt == eVideoFmt)
		{
			/* printf("found videoFmt %d at entry %d\n", eVideoFmt, ii); */
			return &s_aVideoFmtInfoTbls[ii];
		}
	}

	/* printf("found no entry for videoFmt %d!!!\n", eVideoFmt);*/
	return NULL;
}
#endif /* #ifndef BFMT_DO_PICK */

/* End of File */
