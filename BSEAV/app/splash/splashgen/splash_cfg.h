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

#ifndef SPLASH_CFG_H__
#define SPLASH_CFG_H__

/* ---------------------------------------------------------
 * At first we must include "bsplash_board.h"
 */
#include "bsplash_board.h"

/* the following macros are defined in bsplash_board.h" */
/* ---------------------------------------------------
 * Notes:
 * 1. max number supported:
 *        mem heap:   2
 *        surface:    2
 *        display:    2
 *    however, they are very easy to further extend.
 *    Just search for key word "PORT" in the directory, you would find all
 *       places where a few similar lines are needed to add,
 *    or talk to Steven Yang
 * 2. surface size is defined by the display format automatically
 * 3. surface pxl format supported: 
 *       BPXL_eR5_G6_B5
 *       BPXL_eA8_Y8_Cb8_Cr8
 *       BPXL_eA8_R8_G8_B8
 * 4. a surface must use the right mem heap. It is decided by the display that uses
 *       this surface. e.g. in 7435 1u4t board, display 0 must uses surface created with
 *       mem heap 0, and display 1 must uses surface created with mem heap 1.
 *    a). if wrong mem heap is used, the display will be likely BLACK SCREEN.
 *    b). talk to software / hardware chip leader or nexus / VDC team if you need to find 
 *       which mem heap to use for a display.
 * 5. surfaces could use diff bmp files, also share bmp file
 * 6. display can uses any surface, as long as the surface is created in the right mem
 *       heap. displays can share surface.
 * 7. display video format supported: any format, as long as your heap has enough memeory
 * 8. RUL typically uses mem heap 0, but it is not always true, 7420 chip uses mem heap 1
 * 9. see splash_cfg.h for the default value
 * 10. refer to 97435/bsplash_board.h for an example
 */
#if 0 
#define SPLASH_NUM_MEM        2

#define SPLASH_RUL_MEM        0   /* idx to BMEM_Handle array */

#define SPLASH_NUM_SURFACE    2

#define SPLASH_SURF0_MEM      0   /* idx to BMEM_Handle array */
#define SPLASH_SURF0_PXL_FMT  BPXL_eA8_Y8_Cb8_Cr8
#define SPLASH_SURF0_BMP      "splash.bmp"

#define SPLASH_SURF1_MEM      1   /* idx to BMEM_Handle array */
#define SPLASH_SURF1_PXL_FMT  BPXL_eR5_G6_B5
#define SPLASH_SURF1_BMP      "splash_2.bmp"

#define SPLASH_NUM_DISPLAY    2

#define SPLASH_DISP0_FMT      BFMT_VideoFmt_e720p
#define SPLASH_DISP0_SUR      0

#define SPLASH_DISP1_FMT      BFMT_VideoFmt_eNTSC;
#define SPLASH_DISP1_SUR      1

#define SPLASH_NUM_SVIDEO_OUTPUTS       0
#define SPLASH_NUM_COMPONENT_OUTPUTS    1
#define SPLASH_NUM_COMPOSITE_OUTPUTS    1
#endif

/* ---------------------------------------------------------
 * At 2nd we define the default value for those missing macros 
 */
#ifndef SPLASH_NODISP_MAIN
#define SPLASH_SUPPORT_DISP_MAIN  1
#endif

#if SPLASH_NOHDM
#ifdef SPLASH_SUPORT_HDM
#undef SPLASH_SUPORT_HDM
#endif
#else
#ifndef SPLASH_SUPORT_HDM
#define SPLASH_SUPORT_HDM     1
#endif
#endif

#ifndef SPLASH_NUM_MEM
#define SPLASH_NUM_MEM        1
#endif

#ifndef SPLASH_RUL_MEM
#define SPLASH_RUL_MEM        0
#endif

#ifndef SPLASH_NUM_SURFACE
#define SPLASH_NUM_SURFACE    1
#endif

#ifndef SPLASH_SURF0_MEM
#define SPLASH_SURF0_MEM      0
#endif

#ifndef SPLASH_SURF0_PXL_FMT
#ifdef SPLASH_SURFACE_PXL_TYPE 
#define SPLASH_SURF0_PXL_FMT  SPLASH_SURFACE_PXL_TYPE
#else
#define SPLASH_SURF0_PXL_FMT  BPXL_eR5_G6_B5 
#endif
#endif

#ifndef SPLASH_SURF0_BMP
#define SPLASH_SURF0_BMP      "splash.bmp"
#endif

#ifndef SPLASH_SURF1_MEM
#define SPLASH_SURF1_MEM      0
#endif

#ifndef SPLASH_SURF1_PXL_FMT
#define SPLASH_SURF1_PXL_FMT  BPXL_eR5_G6_B5 
#endif

#ifndef SPLASH_SURF1_BMP
#define SPLASH_SURF1_BMP      SPLASH_SURF0_BMP
#endif

/* PORT POINT: if we have more than 2 surfaces */

#ifndef SPLASH_NUM_DISPLAY
#define SPLASH_NUM_DISPLAY    2
#endif

#ifndef SPLASH_DISP0_FMT
#ifdef SPLASH_CMP_0_PATH_DISPLAY
#define SPLASH_DISP0_FMT      SPLASH_CMP_0_PATH_DISPLAY
#else
#define SPLASH_DISP0_FMT      BFMT_VideoFmt_e480p
#endif
#endif

#ifndef SPLASH_DISP0_SUR
#define SPLASH_DISP0_SUR      0
#endif

#ifndef SPLASH_DISP1_FMT
#ifdef SPLASH_CMP_1_PATH_DISPLAY
#define SPLASH_DISP1_FMT      SPLASH_CMP_1_PATH_DISPLAY
#else
#define SPLASH_DISP1_FMT      BFMT_VideoFmt_eNTSC;
#endif
#endif

#ifndef SPLASH_DISP1_SUR
#define SPLASH_DISP1_SUR      0
#endif

/* PORT POINT: if we have more than 2 displays */

#ifndef SPLASH_NUM_SVIDEO_OUTPUTS
#define SPLASH_NUM_SVIDEO_OUTPUTS       0
#endif

#ifndef SPLASH_NUM_COMPONENT_OUTPUTS
#define SPLASH_NUM_COMPONENT_OUTPUTS    1
#endif

#ifndef SPLASH_NUM_COMPOSITE_OUTPUTS
#define SPLASH_NUM_COMPOSITE_OUTPUTS    1
#endif

#if ((SPLASH_DISP0_SUR>(SPLASH_NUM_SURFACE-1)) || (SPLASH_DISP1_SUR>(SPLASH_NUM_SURFACE-1)))
#error "Some disp is using surface 1 but you only have surface 0, please correct bplash_board.h"
#endif


#endif /* #ifndef SPLASH_CFG_H__ */

/* End of file */
