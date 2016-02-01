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

#include "bstd.h"
#include "bavc.h"

BDBG_MODULE( BAVC );

/***************************************************************************
 *
 */
/* NTSC-J for Japan */
#define BAVC_P_VIDEO_FORMAT_IS_NTSC_J(fmt) \
	((BFMT_VideoFmt_eNTSC_J == (fmt)) || (BFMT_VideoFmt_e720x482_NTSC_J == (fmt)))

/* NTSC-M for North America */
#define BAVC_P_VIDEO_FORMAT_IS_NTSC_M(fmt) \
	((BFMT_VideoFmt_eNTSC == (fmt)) || (BFMT_VideoFmt_e720x482_NTSC == (fmt)) || \
	 (BFMT_VideoFmt_eNTSC_443 == (fmt)))

#define BAVC_P_VIDEO_FORMAT_IS_NTSC(fmt) \
	(BAVC_P_VIDEO_FORMAT_IS_NTSC_J(fmt) || BAVC_P_VIDEO_FORMAT_IS_NTSC_M(fmt))

#define BAVC_P_VIDEO_FORMAT_IS_480P(fmt) \
	((BFMT_VideoFmt_e480p == (fmt)) || (BFMT_VideoFmt_e720x483p == (fmt)))

#define BAVC_P_VIDEO_FORMAT_IS_PAL_BB1D1G(fmt) \
	((BFMT_VideoFmt_ePAL_B == (fmt)) || (BFMT_VideoFmt_ePAL_B1 == (fmt)) || \
	(BFMT_VideoFmt_ePAL_D1 == (fmt)) || (BFMT_VideoFmt_ePAL_G == (fmt)))

#define BAVC_P_VIDEO_FORMAT_IS_PAL(fmt) \
	(BAVC_P_VIDEO_FORMAT_IS_PAL_BB1D1G(fmt) || (BFMT_VideoFmt_ePAL_H == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_I == (fmt)) || (BFMT_VideoFmt_ePAL_D == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_K == (fmt)) || (BFMT_VideoFmt_ePAL_M == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_N == (fmt)) || (BFMT_VideoFmt_ePAL_NC == (fmt)) || \
	 (BFMT_VideoFmt_ePAL_60 == (fmt)))

#define BAVC_P_DEFAULT_MATRIX_COEFF(fmt, xvycc) \
	( (BAVC_P_VIDEO_FORMAT_IS_NTSC(fmt) || BAVC_P_VIDEO_FORMAT_IS_480P(fmt)) ? ((xvycc) ? BAVC_MatrixCoefficients_eXvYCC_601 : BAVC_MatrixCoefficients_eSmpte_170M) : \
	  ((BAVC_P_VIDEO_FORMAT_IS_PAL(fmt) || (BFMT_VideoFmt_e576p_50Hz == fmt)) ? BAVC_MatrixCoefficients_eItu_R_BT_470_2_BG : \
	  BAVC_MatrixCoefficients_eItu_R_BT_709) )

BAVC_MatrixCoefficients  BAVC_GetDefaultMatrixCoefficients_isrsafe(
	BFMT_VideoFmt eDisplayFmt,
	bool          bXvYcc )
{
	return BAVC_P_DEFAULT_MATRIX_COEFF(eDisplayFmt, bXvYcc);
}

/* End of File */

