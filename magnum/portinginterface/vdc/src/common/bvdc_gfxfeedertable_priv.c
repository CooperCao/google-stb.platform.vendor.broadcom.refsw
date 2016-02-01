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
#include "bvdc_gfxfeeder_priv.h"
#include "bvdc_window_priv.h"
#include "bchp.h"
#include "bchp_gfd_0.h"

BDBG_MODULE(BVDC_GFX);

/****************************************************************
 *  Color Conversion Matrix
 ****************************************************************/
/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_SdYCrCb_AlphaBlend = BVDC_P_MAKE_GFD_CSC
	(  0.2570,  0.5040,  0.0980,  16.0000,  0.0000,
	  -0.1480, -0.2910,  0.4390, 128.0000,  0.0000,
	   0.4390, -0.3680, -0.0710, 128.0000,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_HdYCrCb_AlphaBlend = BVDC_P_MAKE_GFD_CSC
	(  0.1830,  0.6140,  0.0620,  16.0000,  0.0000,
	  -0.1010, -0.3380,  0.4390, 128.0000,  0.0000,
	   0.4390, -0.3990, -0.0400, 128.0000,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_HdRGBA_to_UhdYCrCb_AlphaBlend = BVDC_P_MAKE_GFD_CSC
	(  0.181912,   0.611800,   0.061757,   16.000000,0.000000,
	  -0.091906,  -0.291101,   0.383006,  128.000000,0.000000,
	   0.289770,  -0.268632,  -0.021138,  128.000000,0.000000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_BT2020RGBA_to_UhdYCrCb_AlphaBlend = BVDC_P_MAKE_GFD_CSC
	(  0.224732,  0.580008,  0.050729,   16.000000, 0.000000,
	  -0.122176, -0.315324,  0.437500,  128.000000, 0.000000,
	   0.437500, -0.402312, -0.035188,  128.000000, 0.000000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_SdYCbCr_to_RGBA_AlphaBlend = BVDC_P_MAKE_GFD_CSC
	(  1.168950,  0.000000,  1.602425, -223.813572,  0.0000,
	   1.168950, -0.394860, -0.816582,  136.361359,  0.0000,
	   1.168950,  2.024147, -0.000000, -277.794001,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_HdYCbCr_to_RGBA_AlphaBlend = BVDC_P_MAKE_GFD_CSC
	(  1.168950,  0.000000,  1.799682, -249.062527,  0.0000,
	   1.168950, -0.214073, -0.535094,   77.190243,  0.0000,
	   1.168950,  2.120703,  0.000000, -290.153216,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_HdYCbCr_to_UhdYCbCr_AlphaBlend = BVDC_P_MAKE_GFD_CSC
	(  1.000000,  -0.000011,   0.000087,  -0.009648, 0.0000,
	   0.000000,   0.874557,  -0.009669,  17.294425, 0.0000,
	   0.000000,   0.012682,   0.665237,  41.226373, 0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_SdYCrCb_ConstBlend = BVDC_P_MAKE_GFD_CSC
	(  0.2570,  0.5040,  0.0980,   0.0000,  16.0000,
	  -0.1480, -0.2910,  0.4390,   0.0000, 128.0000,
	   0.4390, -0.3680, -0.0710,   0.0000, 128.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_HdYCrCb_ConstBlend = BVDC_P_MAKE_GFD_CSC
	(  0.1830,  0.6140,  0.0620,   0.0000,  16.0000,
	  -0.1010, -0.3380,  0.4390,   0.0000, 128.0000,
	   0.4390, -0.3990, -0.0400,   0.0000, 128.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_HdRGBA_to_UhdYCrCb_ConstBlend = BVDC_P_MAKE_GFD_CSC
	(  0.181912,   0.611800,   0.061757,  0.0000,   16.000000,
	  -0.091906,  -0.291101,   0.383006,  0.0000,  128.000000,
	   0.289770,  -0.268632,  -0.021138,  0.0000,  128.000000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_BT2020RGBA_to_UhdYCrCb_ConstBlend = BVDC_P_MAKE_GFD_CSC
	(  0.224732,  0.580008,  0.050729,  0.0000,   16.000000,
	  -0.122176, -0.315324,  0.437500,  0.0000,  128.000000,
	   0.437500, -0.402312, -0.035188,  0.0000,  128.000000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_SdYCbCr_to_RGBA_ConstBlend = BVDC_P_MAKE_GFD_CSC
	(  1.168950,  0.000000,  1.602425,  0.0000, -223.813572,
	   1.168950, -0.394860, -0.816582,  0.0000,  136.361359,
	   1.168950,  2.024147, -0.000000,  0.0000, -277.794001 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_HdYCbCr_to_RGBA_ConstBlend = BVDC_P_MAKE_GFD_CSC
	(  1.168950,  0.000000,  1.799682,  0.0000, -249.062527,
	   1.168950, -0.214073, -0.535094,  0.0000,   77.190243,
	   1.168950,  2.120703,  0.000000,  0.0000, -290.153216 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_Identity = BVDC_P_MAKE_GFD_CSC
	(  1.000000,  0.000000,  0.000000,  0.000000,  0.000000,
	   0.000000,  1.000000,  0.000000,  0.000000,  0.000000,
	   0.000000,  0.000000,  1.000000,  0.000000,  0.000000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_HdYCbCr_to_UhdYCbCr_ConstBlend = BVDC_P_MAKE_GFD_CSC
	(  1.000000,  -0.000011,   0.000087,  0.0000, -0.009648,
	   0.000000,   0.874557,  -0.009669,  0.0000, 17.294425,
	   0.000000,   0.012682,   0.665237,  0.0000, 41.226373 );

#if 0
/* not used yet, because we don't have Uhd gfx surface yet */
/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_UhdYCrCb_AlphaBlend = BVDC_P_MAKE_GFD_CSC
	(  0.1830,  0.6140,  0.0620,  16.0000,  0.0000,
	  -0.1010, -0.3380,  0.4390, 128.0000,  0.0000,
	   0.4390, -0.3990, -0.0400, 128.0000,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_UhdYCbCr_to_RGBA_AlphaBlend = BVDC_P_MAKE_GFD_CSC
	(  1.168950,  0.000000,  1.799682, -249.062527,  0.0000,
	   1.168950, -0.214073, -0.535094,   77.190243,  0.0000,
	   1.168950,  2.120703,  0.000000, -290.153216,  0.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs  s_RGBA_to_UhdYCrCb_ConstBlend = BVDC_P_MAKE_GFD_CSC
	(  0.1830,  0.6140,  0.0620,   0.0000,  16.0000,
	  -0.1010, -0.3380,  0.4390,   0.0000, 128.0000,
	   0.4390, -0.3990, -0.0400,   0.0000, 128.0000 );

/*--------------------------------------------------------------------*/
static const BVDC_P_CscCoeffs s_UhdYCbCr_to_RGBA_ConstBlend = BVDC_P_MAKE_GFD_CSC
	(  1.168950,  0.000000,  1.799682,  0.0000, -249.062527,
	   1.168950, -0.214073, -0.535094,  0.0000,   77.190243,
	   1.168950,  2.120703,  0.000000,  0.0000, -290.153216 );
#endif

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_DecideColorMatrix_isr
 *
 * output: color matrix to convert from active pixel format to output
 *         color primary (main video window's color primary)
 *
 * Note: Because of gamma effect, of not knowing how user treated alpha
 * when the src gfx surface was created, and of diff between Bt601 and
 * Bt709 is not very noticable for gfx, we decide to use idendity matrix
 * to convert between Bt601 and Bt709 (i.e. not conv).
 * Note: If display is Uhd, we assume gfx surface is Bt 709 color, we do
 * convert from BT 709 to BT 2020
 */
BERR_Code BVDC_P_GfxFeeder_DecideColorMatrix_isr
	( BPXL_Format                  eActivePxlFmt,
	  BVDC_P_GfxFeeder_Handle      hGfxFeeder,
	  bool                         bConstantBlend,
	  BVDC_P_CscCfg               *pCscCfg,
	  const BVDC_P_CscCoeffs     **ppaulRGBToYCbCr,
	  const BVDC_P_CscCoeffs     **ppaulYCbCrToRGB )
{
	BVDC_P_CmpColorSpace  eDestColorSpace;

	/* TODO: in the future, we should distinquish current hd/sd standard
	 * from legacy hd/sd, and NTSC sd from PAL sd */
	BVDC_P_Window_GetCurrentDestColorSpace_isr( hGfxFeeder->hWindow, &eDestColorSpace );

	if ( (BVDC_P_CmpColorSpace_eUhdYCrCb   == eDestColorSpace) ||
		 (BVDC_P_CmpColorSpace_eHdYCrCb    == eDestColorSpace) ||
		 (BVDC_P_CmpColorSpace_eSmpte_240M == eDestColorSpace) )
	{
		*ppaulRGBToYCbCr = (bConstantBlend)?
			&s_RGBA_to_HdYCrCb_ConstBlend : &s_RGBA_to_HdYCrCb_AlphaBlend;

		*ppaulYCbCrToRGB = (bConstantBlend)?
			&s_HdYCbCr_to_RGBA_ConstBlend : &s_HdYCbCr_to_RGBA_AlphaBlend;
	}
	else
	{
		*ppaulRGBToYCbCr = (bConstantBlend)?
			&s_RGBA_to_SdYCrCb_ConstBlend : &s_RGBA_to_SdYCrCb_AlphaBlend;

		*ppaulYCbCrToRGB = (bConstantBlend)?
			&s_SdYCbCr_to_RGBA_ConstBlend : &s_SdYCbCr_to_RGBA_AlphaBlend;
	}

#ifndef BVDC_FOR_BOOTUPDATER
#if BVDC_P_SUPPORT_CMP_NON_LINEAR_CSC
	pCscCfg->pCscMA = &s_Identity;
	pCscCfg->ulNLCnv = BVDC_P_NL_CSC_CTRL_SEL_BYPASS;
	pCscCfg->bNLXvYcc = false;
	if ((BVDC_P_CmpColorSpace_eUhdYCrCb == eDestColorSpace) &&
	    (hGfxFeeder->bSupportNLCsc) &&
	    (hGfxFeeder->bSupportMACsc || (BPXL_IS_RGB_FORMAT(eActivePxlFmt))))
	{
		if (!BPXL_IS_RGB_FORMAT(eActivePxlFmt))
		{
			pCscCfg->pCscMA = &s_HdYCbCr_to_RGBA_ConstBlend;
		}
		pCscCfg->ulNLCnv = BCHP_GFD_0_NL_CSC_CTRL_SEL_CONV_R0_709_RGB_2_2020_RGB;
		pCscCfg->stCscMC = (bConstantBlend)?
			s_BT2020RGBA_to_UhdYCrCb_ConstBlend : s_BT2020RGBA_to_UhdYCrCb_AlphaBlend;
	}
	else
#endif
	if( hGfxFeeder->hWindow->stCurInfo.bUserCsc )
	{
		BDBG_MSG(("Using User WIN CSC for GFX CSC Matrix"));

		if( hGfxFeeder->stCurCfgInfo.stDirty.stBits.bCsc )
		{
			BVDC_P_Csc_FromMatrix_isr( &pCscCfg->stCscMC,
				hGfxFeeder->hWindow->stCurInfo.pl32_Matrix, hGfxFeeder->hWindow->stCurInfo.ulUserShift);
		}
	}
	else
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
	if ( true == BPXL_IS_RGB_FORMAT(eActivePxlFmt) )
	{
		if (BVDC_P_CmpColorSpace_eUhdYCrCb == eDestColorSpace)
		{
			pCscCfg->stCscMC = (bConstantBlend)?
				s_HdRGBA_to_UhdYCrCb_ConstBlend : s_HdRGBA_to_UhdYCrCb_AlphaBlend;
		}
		else
		{
			pCscCfg->stCscMC = **ppaulRGBToYCbCr;
		}
		/* this makes BVDC_P_Csc_ApplyAttenuationRGB_isr work */
		*ppaulYCbCrToRGB = &s_Identity;
		*ppaulRGBToYCbCr = &s_Identity;
	}
	else
	{
#ifndef BVDC_FOR_BOOTUPDATER
		if (BVDC_P_CmpColorSpace_eUhdYCrCb == eDestColorSpace)
		{
			pCscCfg->stCscMC = (bConstantBlend)?
				s_HdYCbCr_to_UhdYCbCr_ConstBlend : s_HdYCbCr_to_UhdYCbCr_AlphaBlend;
		}
		else
#endif /* #ifndef BVDC_FOR_BOOTUPDATER */
		{
			pCscCfg->stCscMC = s_Identity;
		}
	}

	return BERR_SUCCESS;
}


/****************************************************************
 *  GFD HSCL coefficients
 ****************************************************************/

static const uint32_t s_aulFirCoeff_PointSample[] =
{
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0000 ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0100 ),

	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0000 ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x0000 ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0000 ),
};

static const uint32_t s_aulFirCoeff_Bilinear[] =
{
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0000 ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0100 ),

	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0000 ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x0000 ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0080 ),
};

static const uint32_t s_aulFirCoeff_Sharp_1toN[] =
{
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0000 ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0100 ),

	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x000e ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0xffd7 ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x009b ),
};

static const uint32_t s_aulFirCoeff_Smooth_1toN[] =
{
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0000 ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0100 ),

	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0004 ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0xffe5 ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0097 ),
};

static const uint32_t s_aulFirCoeff_Sharp_16to9[] =
{
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0xfff0 ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0044 ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0098 ),

	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0xfff0 ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x000c ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0084 ),
};

static const uint32_t s_aulFirCoeff_Smooth_16to9[] =
{
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0xfffc ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x003d ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x008e ),

	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0xfffc ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x000e ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0076 ),
};

static const uint32_t s_aulFirCoeff_Sharp_3to1[] =
{
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0017 ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x003f ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x0054 ),

	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x000a ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x002a ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x004c ),
};

static const uint32_t s_aulFirCoeff_Smooth_3to1[] =
{
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x001d ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x003d ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE0_02,    COEFF_2, 0x004c ),

	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x000e ) |
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x002c ),
	BCHP_FIELD_DATA( GFD_0_HORIZ_FIR_COEFF_PHASE1_02,    COEFF_2, 0x0046 ),
};


#define BVDC_P_GFX_FIRCOEFF_IDX_START (101)
#define BVDC_P_GFX_FIRCOEFF_IDX_END   (126)

/****************************************************************
 *  Indexed GFD HSCL coefficients
 ****************************************************************/
static const uint32_t s_paulFirCoeffTbl[][GFD_NUM_REGS_HSCL_COEFF] =
{
	{ 0x01000400, 0x06000000, 0x00400260, 0x05600000 },  /* 1 */
	{ 0x00B00400, 0x06A00000, 0x00200210, 0x05D00000 },  /* 2 */
	{ 0x00800400, 0x07000000, 0x000001E0, 0x06200000 },  /* 3 */
	{ 0x00500400, 0x07600000, 0x3FF001A0, 0x06700000 },  /* 4 */
	{ 0x002003F0, 0x07E00000, 0x3FE00160, 0x06C00000 },  /* 5 */
	{ 0x3FF003E0, 0x08600000, 0x3FE00120, 0x07000000 },  /* 6 */
	{ 0x3FC003C0, 0x09000000, 0x3FE000D0, 0x07500000 },  /* 7 */
	{ 0x3FA003A0, 0x09800000, 0x3FD00090, 0x07A00000 },  /* 8 */
	{ 0x3F800370, 0x0A200000, 0x3FE00040, 0x07E00000 },  /* 9 */
	{ 0x3F700330, 0x0AC00000, 0x3FE03FF0, 0x08300000 },  /* 10 */
	{ 0x3F6002F0, 0x0B600000, 0x3FE03FA0, 0x08800000 },  /* 11 */
	{ 0x3F6002A0, 0x0C000000, 0x3FF03F50, 0x08C00000 },  /* 12 */
	{ 0x3F600240, 0x0CC00000, 0x00003F00, 0x09000000 },  /* 13 */
	{ 0x3F7001D0, 0x0D800000, 0x00103ED0, 0x09200000 },  /* 14 */
	{ 0x3F900170, 0x0E000000, 0x00103E90, 0x09600000 },  /* 15 */
	{ 0x3FB000F0, 0x0EC00000, 0x00203E70, 0x09700000 },  /* 16 */
	{ 0x3FD00090, 0x0F400000, 0x00203E60, 0x09800000 },  /* 17 */
	{ 0x00000010, 0x0FE00000, 0x00303E50, 0x09800000 },  /* 18 */
	{ 0x00203FA0, 0x10800000, 0x00303E50, 0x09800000 },  /* 19 */
	{ 0x00503F10, 0x11400000, 0x00203E70, 0x09700000 },  /* 20 */
	{ 0x00703EB0, 0x11C00000, 0x00203E90, 0x09500000 },  /* 21 */
	{ 0x00903E40, 0x12600000, 0x00103EC0, 0x09300000 },  /* 22 */
	{ 0x00A03DE0, 0x13000000, 0x00003EF0, 0x09100000 },  /* 23 */
	{ 0x00A03D70, 0x13E00000, 0x3FF03F40, 0x08D00000 },  /* 24 */
	{ 0x00A03D20, 0x14800000, 0x3FF03F90, 0x08800000 },  /* 25 */
	{ 0x00A03CD0, 0x15200000, 0x3FE03FE0, 0x08400000 },  /* 26 */
};

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetFilterCoefficients_Auto_isr(
	uint32_t ulSrcSize,
	uint32_t ulOutSize )
{
	if( ulOutSize == ulSrcSize )
		return (uint32_t *) s_aulFirCoeff_PointSample;
	else if( ulOutSize >= 3 * ulSrcSize ) /* 1to3: 720p -> 4k */
		return (uint32_t *) &s_paulFirCoeffTbl[11];
	else if( ulOutSize >= 2 * ulSrcSize ) /* 1to2: 1080p -> 4k */
		return (uint32_t *) &s_paulFirCoeffTbl[11];
	else if( ulOutSize >= ulSrcSize )     /* 1toN: Any other scaler up */
		return (uint32_t *) s_aulFirCoeff_Smooth_1toN;
	else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
		return (uint32_t *) s_aulFirCoeff_Smooth_16to9;
	else
		return (uint32_t *) s_aulFirCoeff_Smooth_3to1;
}


/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetFilterCoefficients_PointSample_isr(
	uint32_t ulSrcSize,
	uint32_t ulOutSize )
{
	BSTD_UNUSED(ulSrcSize);
	BSTD_UNUSED(ulOutSize);
	return (uint32_t *) s_aulFirCoeff_PointSample;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetFilterCoefficients_Bilinear_isr(
	uint32_t ulSrcSize,
	uint32_t ulOutSize )
{
	BSTD_UNUSED(ulSrcSize);
	BSTD_UNUSED(ulOutSize);
	return (uint32_t *) s_aulFirCoeff_Bilinear;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetFilterCoefficients_Sharp_isr(
	uint32_t ulSrcSize,
	uint32_t ulOutSize )
{
	if( ulOutSize == ulSrcSize )
		return (uint32_t *) s_aulFirCoeff_PointSample;
	else if( ulOutSize >= ulSrcSize )
		return (uint32_t *) s_aulFirCoeff_Sharp_1toN;
	else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
		return (uint32_t *) s_aulFirCoeff_Sharp_16to9;
	else
		return (uint32_t *) s_aulFirCoeff_Sharp_3to1;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetFilterCoefficients_Smooth_isr(
	uint32_t ulSrcSize,
	uint32_t ulOutSize )
{
	if( ulOutSize == ulSrcSize )
		return (uint32_t *) s_aulFirCoeff_PointSample;
	else if( ulOutSize >= ulSrcSize )
		return (uint32_t *) s_aulFirCoeff_Smooth_1toN;
	else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
		return (uint32_t *) s_aulFirCoeff_Smooth_16to9;
	else
		return (uint32_t *) s_aulFirCoeff_Smooth_3to1;
}

/*--------------------------------------------------------------------
 * {private}
 * Description:
 * This is the private type for coefficient functions.
*--------------------------------------------------------------------*/
typedef uint32_t *(* BVDC_P_GetFilterCoefFunc)( uint32_t ulSrcSize, uint32_t ulOutSize );

/*--------------------------------------------------------------------
 * {private}
 * Note: The order has to match the def of BVDC_FilterCoeffs
 *-------------------------------------------------------------------*/
static const BVDC_P_GetFilterCoefFunc s_pfnGetFilterCoefficients[] =
{
	BVDC_P_GetFilterCoefficients_Auto_isr,
	BVDC_P_GetFilterCoefficients_PointSample_isr,
	BVDC_P_GetFilterCoefficients_Bilinear_isr,
	BVDC_P_GetFilterCoefficients_Smooth_isr,
	BVDC_P_GetFilterCoefficients_Sharp_isr
};

#define BVDC_P_GFX_PF_TABLE_COUNT \
	(sizeof(s_pfnGetFilterCoefficients)/sizeof(BVDC_P_GetFilterCoefFunc))

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_DecideFilterCoeff_isr
 *
 * output: Hscl filter coeff
 */
BERR_Code BVDC_P_GfxFeeder_DecideFilterCoeff_isr
	( BVDC_FilterCoeffs     eCoeffs,
	  uint32_t              ulCtIndex,
	  uint32_t              ulSrcSize,
	  uint32_t              ulOutSize,
	  uint32_t **           paulCoeff )
{
	if ((eCoeffs <= BVDC_FilterCoeffs_eSharp) && (NULL != paulCoeff))
	{
		if (eCoeffs == BVDC_FilterCoeffs_eSharp && ulCtIndex != 0)
		{
			if (ulCtIndex < BVDC_P_GFX_FIRCOEFF_IDX_START || ulCtIndex > BVDC_P_GFX_FIRCOEFF_IDX_END)
			{
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			}

			*paulCoeff = (uint32_t *)s_paulFirCoeffTbl[ulCtIndex - BVDC_P_GFX_FIRCOEFF_IDX_START];
		}
		else
		{
			*paulCoeff = (* s_pfnGetFilterCoefficients[eCoeffs])(ulSrcSize, ulOutSize);
		}
		return BERR_SUCCESS;
	}
	else
	{
		BDBG_ASSERT( NULL != paulCoeff );
		BDBG_ASSERT( eCoeffs <= BVDC_FilterCoeffs_eSharp );
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
}

#if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3)
/****************************************************************
 *  GFD VSCL coefficients
 ****************************************************************/

static const uint32_t s_aulVsclFirCoeff_PointSample[] =
{
	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0100 ),

	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0000 ) |
	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x0000 ),
	BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Bilinear[] =
{
	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0000 ) |
	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0100 ),

	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0000 ) |
	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x0080 ),
	BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Sharp_1toN[] =
{
	0x09200370, 0x076000A0,
	BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Sharp_16to9[] =
{
	0x08E00390, 0x075000B0,
	BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Sharp_3to1[] =
{
	0x08E00390, 0x075000B0,
	BVDC_P_SCL_LAST
};


static const uint32_t s_aulVsclFirCoeff_Smooth_1toN[] =
{
	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_0, 0x0037 ) |
	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE0_00_01, COEFF_1, 0x0092 ),

	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_0, 0x0009 ) |
	BCHP_FIELD_DATA( GFD_0_VERT_FIR_COEFF_PHASE1_00_01, COEFF_1, 0x0077 ),
	BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Smooth_16to9[] =
{
	0x05800540, 0x042003E0,
	BVDC_P_SCL_LAST
};

static const uint32_t s_aulVsclFirCoeff_Smooth_3to1[] =
{
	0x05600550, 0x041003F0,
	BVDC_P_SCL_LAST
};


/****************************************************************
 *  Indexed GFD VSCL coefficients
 ****************************************************************/
static const uint32_t s_paulVsclFirCoeffTbl[][GFD_NUM_REGS_VSCL_COEFF] =
{
	{ 0x0A000300, 0x07A00060 },  /* 1 */
	{ 0x0A6002D0, 0x07B00050 },  /* 2 */
	{ 0x0AA002B0, 0x07C00040 },  /* 3 */
	{ 0x0B000280, 0x07D00030 },  /* 4 */
	{ 0x0B400260, 0x07E00020 },  /* 5 */
	{ 0x0BA00230, 0x07F00010 },  /* 6 */
	{ 0x0C000200, 0x08000000 },  /* 7 */
	{ 0x0C6001D0, 0x08103FF0 },  /* 8 */
	{ 0x0CE00190, 0x08203FE0 },  /* 9 */
	{ 0x0D600150, 0x08303FD0 },  /* 10 */
	{ 0x0DE00110, 0x08303FD0 },  /* 11 */
	{ 0x0E6000D0, 0x08403FC0 },  /* 12 */
	{ 0x0EE00090, 0x08403FC0 },  /* 13 */
	{ 0x0F600050, 0x08403FC0 },  /* 14 */
	{ 0x10000000, 0x08403FC0 },  /* 15 */
	{ 0x10603FD0, 0x08403FC0 },  /* 16 */
	{ 0x11003F80, 0x08403FC0 },  /* 17 */
	{ 0x11603F50, 0x08403FC0 },  /* 18 */
	{ 0x11E03F10, 0x08303FD0 },  /* 19 */
	{ 0x12403EE0, 0x08303FD0 },  /* 20 */
	{ 0x12C03EA0, 0x08203FE0 },  /* 21 */
	{ 0x13003E80, 0x08103FF0 },  /* 22 */
	{ 0x13403E60, 0x08000000 },  /* 23 */
	{ 0x13603E50, 0x08000000 },  /* 24 */
	{ 0x13803E40, 0x07F00010 },  /* 25 */
	{ 0x13803E40, 0x07E00020 },  /* 26 */
};


/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetVsclFilterCoefficients_Auto_isr(
	uint32_t ulSrcSize,
	uint32_t ulOutSize )
{
	if( ulOutSize == ulSrcSize )
		return (uint32_t *) s_aulVsclFirCoeff_PointSample;
	else if( ulOutSize >= 3 * ulSrcSize ) /* 1to3: 720p -> 4k */
		return (uint32_t *) &s_paulVsclFirCoeffTbl[11];
	else if( ulOutSize >= 2 * ulSrcSize ) /* 1to2: 1080p -> 4k */
		return (uint32_t *) s_paulVsclFirCoeffTbl[11];
	else if( ulOutSize >= ulSrcSize )
		return (uint32_t *) s_aulVsclFirCoeff_Smooth_1toN;
	else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
		return (uint32_t *) s_aulVsclFirCoeff_Smooth_16to9;
	else
		return (uint32_t *) s_aulVsclFirCoeff_Smooth_3to1;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetVsclFilterCoefficients_PointSample_isr(
	uint32_t ulSrcSize,
	uint32_t ulOutSize )
{
	BSTD_UNUSED(ulSrcSize);
	BSTD_UNUSED(ulOutSize);
	return (uint32_t *) s_aulVsclFirCoeff_PointSample;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetVsclFilterCoefficients_Bilinear_isr(
	uint32_t ulSrcSize,
	uint32_t ulOutSize )
{
	BSTD_UNUSED(ulSrcSize);
	BSTD_UNUSED(ulOutSize);
	return (uint32_t *) s_aulVsclFirCoeff_Bilinear;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetVsclFilterCoefficients_Sharp_isr(
	uint32_t ulSrcSize,
	uint32_t ulOutSize )
{
	if( ulOutSize == ulSrcSize )
		return (uint32_t *) s_aulVsclFirCoeff_PointSample;
	else if( ulOutSize >= ulSrcSize )
		return (uint32_t *) s_aulVsclFirCoeff_Sharp_1toN;
	else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
		return (uint32_t *) s_aulVsclFirCoeff_Sharp_16to9;
	else
		return (uint32_t *) s_aulVsclFirCoeff_Sharp_3to1;
}

/*--------------------------------------------------------------------*/
static uint32_t *BVDC_P_GetVsclFilterCoefficients_Smooth_isr(
	uint32_t ulSrcSize,
	uint32_t ulOutSize )
{
	if( ulOutSize == ulSrcSize )
		return (uint32_t *) s_aulVsclFirCoeff_PointSample;
	else if( ulOutSize >= ulSrcSize )
		return (uint32_t *) s_aulVsclFirCoeff_Smooth_1toN;
	else if( ulOutSize * 12 >= ulSrcSize * 16 )  /* rounding to 9/16 */
		return (uint32_t *) s_aulVsclFirCoeff_Smooth_16to9;
	else
		return (uint32_t *) s_aulVsclFirCoeff_Smooth_3to1;
}

/*--------------------------------------------------------------------
 * {private}
 * Description:
 * This is the private type for coefficient functions.
*--------------------------------------------------------------------*/
typedef uint32_t *(* BVDC_P_GetVsclFilterCoefFunc)( uint32_t ulSrcSize, uint32_t ulOutSize );

/*--------------------------------------------------------------------
 * {private}
 * Note: The order has to match the def of BVDC_FilterCoeffs
 *-------------------------------------------------------------------*/
static const BVDC_P_GetVsclFilterCoefFunc s_pfnGetVsclFirCoefficients[] =
{
	BVDC_P_GetVsclFilterCoefficients_Auto_isr,
	BVDC_P_GetVsclFilterCoefficients_PointSample_isr,
	BVDC_P_GetVsclFilterCoefficients_Bilinear_isr,
	BVDC_P_GetVsclFilterCoefficients_Smooth_isr,
	BVDC_P_GetVsclFilterCoefficients_Sharp_isr
};

#define BVDC_P_GFX_VSCL_PF_TABLE_COUNT \
	(sizeof(s_pfnGetVsclFirCoefficients)/sizeof(BVDC_P_GetVsclFilterCoefFunc))

/*------------------------------------------------------------------------
 * {private}
 * BVDC_P_GfxFeeder_DecideVsclFirCoeff_isr
 *
 * output: filter coeff
 *
 * Note: This implementation is originally copied from bgrc, we should watch
 * bgrc's update and update this code accordingly.
 */
BERR_Code BVDC_P_GfxFeeder_DecideVsclFirCoeff_isr
	( BVDC_FilterCoeffs     eCoeffs,
	  uint32_t              ulCtIndex,
	  uint32_t              ulSrcSize,
	  uint32_t              ulOutSize,
	  uint32_t **           paulCoeff )
{
	if ((eCoeffs <= BVDC_FilterCoeffs_eSharp) && (NULL != paulCoeff))
	{
		if (eCoeffs == BVDC_FilterCoeffs_eSharp && ulCtIndex != 0)
		{
			if (ulCtIndex < BVDC_P_GFX_FIRCOEFF_IDX_START || ulCtIndex > BVDC_P_GFX_FIRCOEFF_IDX_END)
			{
				return BERR_TRACE(BERR_INVALID_PARAMETER);
			}

			*paulCoeff = (uint32_t *)s_paulVsclFirCoeffTbl[ulCtIndex - BVDC_P_GFX_FIRCOEFF_IDX_START];
		}
		else
		{
			*paulCoeff = (*s_pfnGetVsclFirCoefficients[eCoeffs])(ulSrcSize, ulOutSize);
		}
		return BERR_SUCCESS;
	}
	else
	{
		BDBG_ASSERT( NULL != paulCoeff );
		BDBG_ASSERT( eCoeffs <= BVDC_FilterCoeffs_eSharp );
		return BERR_TRACE(BERR_INVALID_PARAMETER);
	}
}
#endif  /* #if (BVDC_P_SUPPORT_GFD_VER >= BVDC_P_SUPPORT_GFD_VER_3) */

/* End of File */
