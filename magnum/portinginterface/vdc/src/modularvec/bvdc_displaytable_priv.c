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
 *   Contains tables for Display settings.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/
#include "bchp_misc.h"
#include "bvdc_display_priv.h"

#if BVDC_P_ORTHOGONAL_VEC

#if (BVDC_P_MAX_DACS > 6)
#define BVDC_P_MAKE_DAC_SEL(channel)        { 0, 0, 0, 0, 0, 0, 0 }
#elif (BVDC_P_MAX_DACS > 5)
#define BVDC_P_MAKE_DAC_SEL(channel)        { 0, 0, 0, 0, 0, 0 }
#elif (BVDC_P_MAX_DACS > 4)
#define BVDC_P_MAKE_DAC_SEL(channel)        { 0, 0, 0, 0, 0 }
#elif (BVDC_P_MAX_DACS > 3)
#define BVDC_P_MAKE_DAC_SEL(channel)        { 0, 0, 0, 0 }
#elif (BVDC_P_MAX_DACS > 2)
#define BVDC_P_MAKE_DAC_SEL(channel)        { 0, 0, 0 }
#elif (BVDC_P_MAX_DACS > 1)
#define BVDC_P_MAKE_DAC_SEL(channel)        { 0, 0 }
#else /* 1 DAC */
#define BVDC_P_MAKE_DAC_SEL(channel)        { 0 }
#endif

#else

#if (BVDC_P_MAX_DACS > 6)
#define BVDC_P_MAKE_DAC_SEL(channel)                              \
{                                                                 \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC0_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC1_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC2_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC3_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC4_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC5_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC6_SEL, channel),             \
}
#elif (BVDC_P_MAX_DACS > 5)
#define BVDC_P_MAKE_DAC_SEL(channel)                              \
{                                                                 \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC0_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC1_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC2_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC3_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC4_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC5_SEL, channel),             \
}
#elif (BVDC_P_MAX_DACS > 4)
#define BVDC_P_MAKE_DAC_SEL(channel)                              \
{                                                                 \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC0_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC1_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC2_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC3_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC4_SEL, channel),             \
}
#elif (BVDC_P_MAX_DACS > 3)
#define BVDC_P_MAKE_DAC_SEL(channel)                              \
{                                                                 \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC0_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC1_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC2_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC3_SEL, channel),             \
}
#elif (BVDC_P_MAX_DACS > 2)
#define BVDC_P_MAKE_DAC_SEL(channel)                              \
{                                                                 \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC0_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC1_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC2_SEL, channel),             \
}
#else /* 2 DACs */
#define BVDC_P_MAKE_DAC_SEL(channel)                              \
{                                                                 \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC0_SEL, channel),             \
	BCHP_FIELD_ENUM(MISC_OUT_MUX, DAC1_SEL, channel),             \
}
#endif
#endif /* orthogonal VEC */

/* Select if needed to use the prim_co or prim */
#if BVDC_P_SUPPORT_PRM_VEC_CMPN_ONLY
#define BVDC_P_MAKE_DAC_SEL_COMP(channel)                         \
	BVDC_P_MAKE_DAC_SEL(PRIM_CO_##channel)
#else
#define BVDC_P_MAKE_DAC_SEL_COMP(channel)                         \
	BVDC_P_MAKE_DAC_SEL(PRIM_##channel)
#endif

/****************************************************************
 *  Tables
 ****************************************************************/
#if (BVDC_P_SUPPORT_TER_VEC)
const uint32_t BVDC_P_aaulTertiaryDacTable[][BVDC_P_MAX_DACS] =
{
	BVDC_P_MAKE_DAC_SEL(TERT_CH0), /* BVDC_DacOutput_eSVideo_Luma */
	BVDC_P_MAKE_DAC_SEL(TERT_CH2), /* BVDC_DacOutput_eSVideo_Chroma */
	BVDC_P_MAKE_DAC_SEL(TERT_CH1), /* BVDC_DacOutput_eComposite */
	BVDC_P_MAKE_DAC_SEL(TERT_CH2), /* BVDC_DacOutput_eRed */
	BVDC_P_MAKE_DAC_SEL(TERT_CH0), /* BVDC_DacOutput_eGreen */
	BVDC_P_MAKE_DAC_SEL(TERT_CH1), /* BVDC_DacOutput_eBlue */
	BVDC_P_MAKE_DAC_SEL(TERT_CH0), /* BVDC_DacOutput_eY */
	BVDC_P_MAKE_DAC_SEL(TERT_CH2), /* BVDC_DacOutput_ePr */
	BVDC_P_MAKE_DAC_SEL(TERT_CH1), /* BVDC_DacOutput_ePb */
	BVDC_P_MAKE_DAC_SEL(TERT_CH0), /* BVDC_DacOutput_eHsync */
	BVDC_P_MAKE_DAC_SEL(TERT_CH0), /* BVDC_DacOutput_eGreen_NoSync */
};
#endif

#if (BVDC_P_SUPPORT_SEC_VEC)
const uint32_t BVDC_P_aaulSecondaryDacTable[][BVDC_P_MAX_DACS] =
{
	BVDC_P_MAKE_DAC_SEL(SEC_CH0),    /* BVDC_DacOutput_eSVideo_Luma */
	BVDC_P_MAKE_DAC_SEL(SEC_CH2),    /* BVDC_DacOutput_eSVideo_Chroma */
	BVDC_P_MAKE_DAC_SEL(SEC_CH1),    /* BVDC_DacOutput_eComposite */
	BVDC_P_MAKE_DAC_SEL(SEC_CO_CH2), /* BVDC_DacOutput_eRed */
	BVDC_P_MAKE_DAC_SEL(SEC_CO_CH0), /* BVDC_DacOutput_eGreen */
	BVDC_P_MAKE_DAC_SEL(SEC_CO_CH1), /* BVDC_DacOutput_eBlue */
	BVDC_P_MAKE_DAC_SEL(SEC_CO_CH0), /* BVDC_DacOutput_eY */
	BVDC_P_MAKE_DAC_SEL(SEC_CO_CH2), /* BVDC_DacOutput_ePr */
	BVDC_P_MAKE_DAC_SEL(SEC_CO_CH1), /* BVDC_DacOutput_ePb */
	BVDC_P_MAKE_DAC_SEL(SEC_HSYNC),  /* BVDC_DacOutput_eHsync */
	BVDC_P_MAKE_DAC_SEL(SEC_CO_CH0), /* BVDC_DacOutput_eGreen_NoSync */
};
#endif

/* Dac settings for Primary Vec path */
const uint32_t BVDC_P_aaulPrimaryDacTable[][BVDC_P_MAX_DACS] =
{
	BVDC_P_MAKE_DAC_SEL(PRIM_CH0),  /* BVDC_DacOutput_eSVideo_Luma */
	BVDC_P_MAKE_DAC_SEL(PRIM_CH2),  /* BVDC_DacOutput_eSVideo_Chroma */
	BVDC_P_MAKE_DAC_SEL(PRIM_CH1),  /* BVDC_DacOutput_eComposite */
	BVDC_P_MAKE_DAC_SEL_COMP(CH2),  /* BVDC_DacOutput_eRed */
	BVDC_P_MAKE_DAC_SEL_COMP(CH0),  /* BVDC_DacOutput_eGreen */
	BVDC_P_MAKE_DAC_SEL_COMP(CH1),  /* BVDC_DacOutput_eBlue */
	BVDC_P_MAKE_DAC_SEL_COMP(CH0),  /* BVDC_DacOutput_eY */
	BVDC_P_MAKE_DAC_SEL_COMP(CH2),  /* BVDC_DacOutput_ePr */
	BVDC_P_MAKE_DAC_SEL_COMP(CH1),  /* BVDC_DacOutput_ePb */
	BVDC_P_MAKE_DAC_SEL(PRIM_CH0),  /* BVDC_DacOutput_eHsync */
	BVDC_P_MAKE_DAC_SEL(PRIM_CH0),  /* BVDC_DacOutput_eGreen_NoSync */
};

/* End of File */
