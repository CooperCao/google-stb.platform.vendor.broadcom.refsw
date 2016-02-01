/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
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

#ifndef BRFM_PRIV_H__
#define BRFM_PRIV_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Platform definitions */

/* RFM core major revision. Taken from RFM_SYSCLK_REVID.MAJOR */
#if (BCHP_CHIP == 7445) || (BCHP_CHIP == 7145) || (BCHP_CHIP==7439) || (BCHP_CHIP==74371) || (BCHP_CHIP == 7364)
#define BRFM_REVID 52
#elif (BCHP_CHIP==7552 || BCHP_CHIP==7429 || BCHP_CHIP==7360 || BCHP_CHIP==7584 || BCHP_CHIP==75845 || BCHP_CHIP==7543 || BCHP_CHIP==74295)
#define BRFM_REVID 51
/* TODO: 7445 and 7145 have different registers from other 5.1 chips, but are still designated 5.1. why? define a new constant for 7445 and later platforms */
#elif (BCHP_CHIP==7420)
#define BRFM_REVID 50
#elif ((BCHP_CHIP==7400 && BCHP_VER>=BCHP_VER_B0) || BCHP_CHIP==7405 || BCHP_CHIP==7325 || BCHP_CHIP==7335 || \
        BCHP_CHIP==7336)
#define BRFM_REVID 40
#else
#define BRFM_REVID 30
#endif

#if (BCHP_CHIP==7420)
#define BRFM_SPUR_WORKAROUND 1 /* SW workaround for noise spur (SW7420-730) */
#endif

#if ((BCHP_CHIP==7400 && BCHP_VER>=BCHP_VER_D0) || BCHP_CHIP==7420)
#define BRFM_DUAL_DAC 1    /* dual DAC output */
#endif

#if (BRFM_REVID==52)
#define BRFM_PLL_MULT  (0x002E)
#define BRFM_PLL_MISC  (0x00000000)
#elif (BRFM_REVID==51 && ((BCHP_CHIP==7429) || (BCHP_CHIP==74295)))
/* the only difference between 7552 and 7429 is that 7429 hands off HiFiDAC and VEC data on the negative edge of 108M clock
   RFM_SYSCLK_MISC.VIRF_EDGE and RFM_SYSCLK_MISC.HIRF_EDGE must be 1. i.e. PLL_MISC=0x5 */
#define BRFM_PLL_MULT  (0x002E)
#define BRFM_PLL_MISC  (0x00000005)
#elif (BRFM_REVID>=50 || (BCHP_CHIP==7400 && BCHP_VER>=BCHP_VER_B0) /* 7400B0 and up is an exception*/) /* includes 7552, 7360 and 7584 */
#define BRFM_PLL_MULT  (0x002E)
#define BRFM_PLL_MISC  (0x00000004)
#elif (BRFM_REVID==40)
#define BRFM_PLL_MULT  (0x002E)
#define BRFM_PLL_MISC  (0x00000005)
#else
#define BRFM_PLL_MULT  (0x502E)
#define BRFM_PLL_MISC  (0x0000000C)
#endif
#define BRFM_PLL_FS    (378.0)

/* Max/Min volumes in dB */
#if (BRFM_REVID>=50)
#define BRFM_MAX_VOLUME (12)
#define BRFM_MIN_VOLUME (-52)
#else
#define BRFM_MAX_VOLUME (30)
#define BRFM_MIN_VOLUME (-34)
#endif
#define BRFM_MAX_VOLUME_NICAM (12)
#define BRFM_MIN_VOLUME_NICAM (-52)

/* this is an outside register that is platform-dependent */
#if (BCHP_CHIP==7325)
#define BRFM_P_CLK_PM_CTRL_BCHP    BCHP_CLKGEN_PWRDN_CTRL_1
#define BRFM_P_CLK_PM_CTRL_MASK    BCHP_CLKGEN_PWRDN_CTRL_1_PWRDN_CLOCK_108_CG_RFM_MASK
#define BRFM_P_CLK_PM_CTRL_REG     CLKGEN_PWRDN_CTRL_1
#define BRFM_P_CLK_PM_CTRL_FIELD   PWRDN_CLOCK_108_CG_RFM
#define BRFM_P_CLK_PM_CTRL_ENABLED 0
#elif (BCHP_CHIP==7420)
#define BRFM_P_CLK_PM_CTRL_BCHP    BCHP_CLK_RFM_CLK_PM_CTRL
#define BRFM_P_CLK_PM_CTRL_MASK    BCHP_CLK_RFM_CLK_PM_CTRL_DIS_RFM_108M_CLK_MASK
#define BRFM_P_CLK_PM_CTRL_REG     CLK_RFM_CLK_PM_CTRL
#define BRFM_P_CLK_PM_CTRL_FIELD   DIS_RFM_108M_CLK
#define BRFM_P_CLK_PM_CTRL_ENABLED 0
#elif (BRFM_REVID>=51)
    #if (BCHP_CHIP==7552 || BCHP_CHIP==7360 || BCHP_CHIP==7584 || BCHP_CHIP==75845 || BCHP_CHIP==7543)
#define BRFM_P_CLK_PM_CTRL_BCHP    BCHP_CLKGEN_RFM_TOP_CLOCK_ENABLE
#define BRFM_P_CLK_PM_CTRL_MASK    BCHP_CLKGEN_RFM_TOP_CLOCK_ENABLE_RFM_108_CLOCK_ENABLE_MASK
#define BRFM_P_CLK_PM_CTRL_REG     CLKGEN_RFM_TOP_CLOCK_ENABLE
#define BRFM_P_CLK_PM_CTRL_FIELD   RFM_108_CLOCK_ENABLE
#define BRFM_P_CLK_PM_CTRL_ENABLED 1 /* if enabled, then 1. other platforms have the opposite meaning */
    #elif (BCHP_CHIP==7364)
#define BRFM_P_CLK_PM_CTRL_BCHP    BCHP_CLKGEN_ONOFF_RFM_TOP_INST_CLOCK_ENABLE
#define BRFM_P_CLK_PM_CTRL_MASK    BCHP_CLKGEN_ONOFF_RFM_TOP_INST_CLOCK_ENABLE_RFM_108_CLOCK_ENABLE_MASK
#define BRFM_P_CLK_PM_CTRL_REG     CLKGEN_ONOFF_RFM_TOP_INST_CLOCK_ENABLE
#define BRFM_P_CLK_PM_CTRL_FIELD   RFM_108_CLOCK_ENABLE
#define BRFM_P_CLK_PM_CTRL_ENABLED 1 /* if enabled, then 1. other platforms have the opposite meaning */
    #else /* 7429, 7445 and 7145 */
#define BRFM_P_CLK_PM_CTRL_BCHP    BCHP_CLKGEN_RFM_TOP_INST_CLOCK_ENABLE
#define BRFM_P_CLK_PM_CTRL_MASK    BCHP_CLKGEN_RFM_TOP_INST_CLOCK_ENABLE_RFM_108_CLOCK_ENABLE_MASK
#define BRFM_P_CLK_PM_CTRL_REG     CLKGEN_RFM_TOP_INST_CLOCK_ENABLE
#define BRFM_P_CLK_PM_CTRL_FIELD   RFM_108_CLOCK_ENABLE
#define BRFM_P_CLK_PM_CTRL_ENABLED 1 /* if enabled, then 1. other platforms have the opposite meaning */
    #endif
#else /* all others */
#define BRFM_P_CLK_PM_CTRL_BCHP    BCHP_CLK_PM_CTRL
#define BRFM_P_CLK_PM_CTRL_MASK    BCHP_CLK_PM_CTRL_DIS_RFM_108M_CLK_MASK
#define BRFM_P_CLK_PM_CTRL_REG     CLK_PM_CTRL
#define BRFM_P_CLK_PM_CTRL_FIELD   DIS_RFM_108M_CLK
#define BRFM_P_CLK_PM_CTRL_ENABLED 0
#endif

/*******************************************************************************
*
* Private Module Handles
*
*******************************************************************************/
typedef struct BRFM_P_ModulationInfo
{
    BRFM_ModulationType modType;
    const uint32_t *setupScrCh[BRFM_OutputChannel_eLast];
    const uint32_t *setupScrAudioEncoding[BRFM_AudioEncoding_eLast];
    const uint32_t *setupScrConfig;
} BRFM_P_ModulationInfo;

#ifdef __cplusplus
}
#endif

#endif

