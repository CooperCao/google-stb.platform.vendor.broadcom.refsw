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
#include "bstd.h"              /* standard types */
#include "bvdc.h"              /* Video display */
#include "bvdc_source_priv.h"
#include "bvdc_gfxfeeder_priv.h"
#include "bvdc_feeder_priv.h"

BDBG_MODULE(BVDC);

#if !B_REFSW_MINIMAL
/***************************************************************************
*
*/
BERR_Code BVDC_Source_SetChromaExpansion
	( BVDC_Source_Handle    hSource,
	  BVDC_ChromaExpansion  eChromaExpansion )
{
	BERR_Code  eStatus = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Source_SetChromaExpansion);

	BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
	BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);

	if( eChromaExpansion <= BVDC_ChromaExpansion_eLinearInterpolate )
	{
		hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bChromaExpan = 1;
		hSource->hGfxFeeder->stNewCfgInfo.eChromaExpansion = eChromaExpansion;
	}
	else
	{
		BDBG_ERR(("Bad eChromaExpansion."));
		eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BDBG_LEAVE(BVDC_Source_SetChromaExpansion);
	return eStatus;
}
#endif


/***************************************************************************
*
*/
BERR_Code BVDC_Source_EnableColorKey
	( BVDC_Source_Handle  hSource,
	  uint32_t            ulMinAC2C1C0,
	  uint32_t            ulMaxAC2C1C0,
	  uint32_t            ulMaskAC2C1C0,
	  uint8_t             ucKeyedAlpha )
{
	BERR_Code  eStatus = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Source_EnableColorKey);

	BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
	BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);

	hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bKey = 1;
	hSource->hGfxFeeder->stNewCfgInfo.stFlags.bEnableKey = 1;
	hSource->hGfxFeeder->stNewCfgInfo.ulKeyMinAMNO = ulMinAC2C1C0;
	hSource->hGfxFeeder->stNewCfgInfo.ulKeyMaxAMNO = ulMaxAC2C1C0;
	hSource->hGfxFeeder->stNewCfgInfo.ulKeyMaskAMNO = ulMaskAC2C1C0;
	hSource->hGfxFeeder->stNewCfgInfo.ucKeyedAlpha = ucKeyedAlpha;

	BDBG_LEAVE(BVDC_Source_EnableColorKey);
	return eStatus;
}


/***************************************************************************
*
*/
BERR_Code BVDC_Source_DisableColorKey
	( BVDC_Source_Handle  hSource )
{
	BERR_Code  eStatus = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Source_DisableColorKey);

	BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
	BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);

	hSource->hGfxFeeder->stNewCfgInfo.stFlags.bEnableKey = 0;

	BDBG_LEAVE(BVDC_Source_DisableColorKey);
	return eStatus;
}


/***************************************************************************
*
*/
BERR_Code BVDC_Source_SetScaleCoeffs
	( BVDC_Source_Handle               hSource,
	  BVDC_FilterCoeffs                eHorzCoeffs,
	  BVDC_FilterCoeffs                eVertCoeffs )
{
	BERR_Code  eStatus = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Source_SetScaleCoeffs);

	BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
	BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);
	if((eHorzCoeffs <= BVDC_FilterCoeffs_eSharp) &&
	   (eVertCoeffs <= BVDC_FilterCoeffs_eSharp))
	{
		hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bScaleCoeffs = 1;
		hSource->hGfxFeeder->stNewCfgInfo.eHorzScaleCoeffs = eHorzCoeffs;
		hSource->hGfxFeeder->stNewCfgInfo.eVertScaleCoeffs = eVertCoeffs;
	}
	else
	{
		BDBG_ERR(("Bad eCoeffs."));
		eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BDBG_LEAVE(BVDC_Source_SetScaleCoeffs);
	return eStatus;
}

#define  GFD_MAX_NUM_GAMMA_T_ENTR     256

#if !B_REFSW_MINIMAL
/***************************************************************************
*
*/
BERR_Code BVDC_Source_EnableGammaCorrection
	( BVDC_Source_Handle               hSource,
	  uint32_t                         ulNumEntries,
	  const BMMA_Block_Handle          hGammaTable )
{
	BERR_Code  eStatus = BERR_SUCCESS;
	uint32_t  ulClutOffset;

	BDBG_ENTER(BVDC_Source_EnableGammaCorrection);

	BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
	BDBG_ASSERT(hGammaTable);

	if( 256 == ulNumEntries )
	{
		BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);

#if (BVDC_P_SUPPORT_GFD_VER_0 == BVDC_P_SUPPORT_GFD1_VER)
		if (BAVC_SourceId_eGfx1 == hSource->hGfxFeeder->eId)
		{
			return BERR_TRACE(BVDC_ERR_GFX_UNSUPPORTED_GAMMATABLE);
		}
#endif

		if( ulNumEntries <= GFD_MAX_NUM_GAMMA_T_ENTR )
		{
			ulClutOffset = BMMA_LockOffset(hGammaTable);

			/* user might use the same clut buf, but change the content */
			hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bGammaTable = 1;
			hSource->hGfxFeeder->stNewCfgInfo.stFlags.bEnableGammaCorrection = 1;
			hSource->hGfxFeeder->stNewCfgInfo.ulNumGammaClutEntries  = ulNumEntries;
			hSource->hGfxFeeder->stNewCfgInfo.ulGammaClutAddress     = ulClutOffset;
		}
		else
		{
			BDBG_ERR(("Bad ulNumEntries (1)."));
			eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
		}
	}
	else
	{
		BDBG_ERR(("Bad ulNumEntries (2)."));
		eStatus = BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	BDBG_LEAVE(BVDC_Source_EnableGammaCorrection);
	return eStatus;
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
*
*/
BERR_Code BVDC_Source_DisableGammaCorrection
	( BVDC_Source_Handle               hSource,
	  const BMMA_Block_Handle          hGammaTable)
{
	BERR_Code  eStatus = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Source_DisableGammaCorrection);

	BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
	BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);
	BDBG_ASSERT(hGammaTable);

	hSource->hGfxFeeder->stNewCfgInfo.stFlags.bEnableGammaCorrection = 0;

	BMMA_UnlockOffset(hGammaTable, hSource->hGfxFeeder->stNewCfgInfo.ulGammaClutAddress);

	BDBG_LEAVE(BVDC_Source_DisableGammaCorrection);
	return eStatus;
}
#endif


#if !B_REFSW_MINIMAL
/***************************************************************************
*
*/
BERR_Code BVDC_Source_SetConstantColor
	( BVDC_Source_Handle               hSource,
	  uint8_t                          ucRed,
	  uint8_t                          ucGreen,
	  uint8_t                          ucBlue )
{
	BERR_Code  eStatus = BERR_SUCCESS;

	BDBG_ENTER(BVDC_Source_SetConstantColor);

	BDBG_OBJECT_ASSERT(hSource, BVDC_SRC);
	BDBG_OBJECT_ASSERT(hSource->hGfxFeeder, BVDC_GFX);

	hSource->hGfxFeeder->stNewCfgInfo.stDirty.stBits.bConstantColor = 1;
	hSource->hGfxFeeder->stNewCfgInfo.ulConstantColor =
		(ucRed << 16) | (ucGreen << 8) | (ucBlue);

	BDBG_LEAVE(BVDC_Source_SetConstantColor);
	return eStatus;
}
#endif

/* End of File */
