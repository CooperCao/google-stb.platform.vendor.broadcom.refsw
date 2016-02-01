/***************************************************************************
 *     Copyright (c) 2010-2011, Broadcom Corporation
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
 *   Custom Video format info file; it contains raster size, front porch,
 * back porch etc format info, and DVO microcode as well as rate manager
 * settings.
 *  NOTE: This file is to be replace by customer specific timing data!
 *  following are for reference board only.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BFMT_CUSTOM_H__
#define BFMT_CUSTOM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bstd.h"
#include "bfmt.h"
#include "bkni.h"

typedef struct BFMT_P_RateInfo
{
	/* Use for searching a matching one! */
	uint32_t                    ulPixelClkRate; /*                  ----3548/3556----                */
	uint32_t                    ulMDiv;         /* LVDS_PHY_0_LVDS_PLL_CFG, PLL_M_DIV,               */
	uint32_t                    ulNDiv;         /* DVPO_RM_0_OFFSET, OFFSET,                         */
	uint32_t                    ulRDiv;         /* LVDS_PHY_0_LVDS_PLL_CFG, PLL_R_DIV,               */
	uint32_t                    ulSampleInc;    /* DVPO_RM_0_SAMPLE_INC, SAMPLE_INC,                 */
	uint32_t                    ulNumerator;    /* DVPO_RM_0_SAMPLE_INC, NUMERATOR,                  */
	uint32_t                    ulDenominator;  /* DVPO_RM_0_RATE_RATIO, DENOMINATOR,                */
	uint32_t                    ulVcoRange;     /* LVDS_PHY_0_LVDS_PLL_CFG, PLL_VCO_RANGE,           */
	uint32_t                    ulLinkDivCtrl;  /* LVDS_PHY_0_LVDS_PLL_CFG, LINKDIV_CTRL,            */
	uint32_t                    ulP2;           /* LVDS_PHY_0_LVDS_PLL_CFG, PLL_FEEDBACK_PRE_DIVIDER */
	const char                 *pchRate;
} BFMT_P_RateInfo;

#ifdef __cplusplus
}
#endif

#endif /* #ifndef BFMT_CUSTOM_H__ */

/* End of File */

