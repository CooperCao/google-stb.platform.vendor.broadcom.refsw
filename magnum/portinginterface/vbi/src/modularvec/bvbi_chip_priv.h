/***************************************************************************
 *     Copyright (c) 2003-2010, Broadcom Corporation
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
 *     Define features that are hardware-specific. For private use by BVBI
 *     software module.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#ifndef BVBI_CHIP_PRIV_H__
#define BVBI_CHIP_PRIV_H__

/*
 * Explanation of VEC/VDEC counts and capabilities:
 * BVBI_P_HAS_WSE_PARITY    VEC(s) have WSS parity bit generation capability.
 * BVBI_P_HAS_XSER_TT:      TTX core has serial output capability.
 * BVBI_P_TTXADR_WAROUND:   TTX core has the PR22720 bug in accessing DRAM. A
 *                          software fix is provided.
 * BVBI_P_CGMSAE_VER2:      CGMSAE core is version first appearing in 3548-A0.
 *                          Capable of CGMS-B output.
 * BVBI_P_CGMSAE_VER3:      CGMSAE core is version first appearing in 3548-B0.
 *                          Capable of CEA-805-D style output.
 * BVBI_P_WSE_VER2:         WSE core is version first appearing in 3548-A0.
 *                          ITU-R 656 output is handled in a different way.
 * BVBI_P_WSE_VER3:         WSE core is version first appearing in 7601-A0.
 *                          Capable of IEC-62375 output on 576P video.
 * BVBI_P_TTE_WA15          TTE core has trouble with 15 lines or more of
 *                          teletext data per field. Workaround involves
 *                          wasting some DRAM.
 */

#if (BCHP_CHIP==7325)
	#define BVBI_P_HAS_EXT_656 1
	#define BVBI_P_HAS_XSER_TT 1
	#define BVBI_P_HAS_WSE_PARITY 1
	#if (BCHP_VER >= BCHP_VER_B0)
		#define BVBI_P_CGMSAE_VER2 1
		#define BVBI_P_WSE_VER3 1
	#endif
	#define BVBI_P_TTE_WA15 1
#elif (BCHP_CHIP==7335) ||  (BCHP_CHIP==7336)
	#define BVBI_P_HAS_EXT_656 1
	#define BVBI_P_HAS_XSER_TT 1
	#define BVBI_P_HAS_WSE_PARITY 1
	#if (BCHP_VER < BCHP_VER_B0)
		#define BVBI_P_CGMSAE_VER2 1
	#else
		#define BVBI_P_CGMSAE_VER3 1
	#endif
	#define BVBI_P_WSE_VER3 1
	#define BVBI_P_TTE_WA15 1
#elif (BCHP_CHIP==7400)
	/* NOTE: chip revision -A0 is not supported */
	#define BVBI_P_HAS_WSE_PARITY 1
	#define BVBI_P_HAS_EXT_656 1
	#define BVBI_P_HAS_XSER_TT 1
	#if (BCHP_VER >= BCHP_VER_E0)
		#define BVBI_P_CGMSAE_VER3 1
	#endif
#elif (BCHP_CHIP==7405)
	#define BVBI_P_HAS_EXT_656 1
	#define BVBI_P_HAS_XSER_TT 1
	#define BVBI_P_HAS_WSE_PARITY 1
#elif (BCHP_CHIP==7601)
	#define BVBI_P_CGMSAE_VER2 1
	#define BVBI_P_WSE_VER3 1
	#define BVBI_P_HAS_FE_BE 1
	#define BVBI_P_HAS_WSE_PARITY 1
#else
	#error Unknown video chip name
#endif

/* I should have started these series in a different way */
#if !defined(BVBI_P_CGMSAE_VER2) && !defined(BVBI_P_CGMSAE_VER3)
	#define BVBI_P_CGMSAE_VER1 1
#endif
#if !defined(BVBI_P_WSE_VER2) && !defined(BVBI_P_WSE_VER3)
	#define BVBI_P_WSE_VER1 1
#endif

#endif /* BVBI_CHIP_PRIV_H__ */
