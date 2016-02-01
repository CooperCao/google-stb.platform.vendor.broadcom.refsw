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
#include "bstd_defs.h"
#include "berr.h"
#include "bkni.h"
#include "bmem.h"
#include "bsur.h"
#include "bsur_private.h"

BDBG_MODULE(BSUR);

BDBG_OBJECT_ID(BSUR_Palette);
BDBG_OBJECT_ID(BSUR_Surface);

/***************************************************************************/
BERR_Code BSUR_Palette_Create(
	BMEM_Handle hMem,
	uint32_t ulNumEntries,
	void *pvPaletteAddress,
	BPXL_Format eFormat,
	uint32_t ulConstraintMask,
	BSUR_Palette_Handle *phPalette )
{
	BSUR_Palette_Handle hPalette;
	void *pvCachedAddress = pvPaletteAddress;
	BERR_Code err;

	BDBG_ENTER(BSUR_Palette_Create);
	BDBG_ASSERT( ulNumEntries );
	BDBG_ASSERT( phPalette );

	if( hMem == (BMEM_Handle) 0 )
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	/* check palette format */
	if( (eFormat != BPXL_eA8_Y8_Cb8_Cr8) && (eFormat != BPXL_eA8_R8_G8_B8) )
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	/* check palette address alignment */
	if( pvPaletteAddress && BSUR_P_PALETTE_UNALIGNED( pvPaletteAddress ) )
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	/* allocate memory for private palette data */
	hPalette = (BSUR_Palette_Handle) BKNI_Malloc( sizeof (BSUR_P_Palette_Handle) );
	if( hPalette == NULL )
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);

	BKNI_Memset( hPalette, 0, sizeof (BSUR_P_Palette_Handle) );
	BDBG_OBJECT_SET(hPalette, BSUR_Palette);

	/* allocate memory for palette entries if required */
	if( pvPaletteAddress == NULL )
	{
		/* M2MC HW accesses palette as 1024 bytes, regardless of palette format. so we must alloc a constant 1024 to avoid ARC violations. */
		pvPaletteAddress = BMEM_Heap_AllocAligned( hMem, 1024, BSUR_P_PALETTE_ALIGNMENT, 0 );
		if( pvPaletteAddress == NULL )
		{
			BDBG_OBJECT_DESTROY(hPalette, BSUR_Palette);
			BKNI_Free( (void *) hPalette );
			return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		}

		err = BMEM_Heap_ConvertAddressToCached( hMem, pvPaletteAddress, &pvCachedAddress );
		if( err != BERR_SUCCESS )
		{
			BDBG_OBJECT_DESTROY(hPalette, BSUR_Palette);
			BMEM_Heap_Free( hMem, pvPaletteAddress );
			BKNI_Free( (void *) hPalette );
			return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		}

		hPalette->bDriverOwned = true;
	}

	/* set private palette data */
	hPalette->hMem = hMem;
	hPalette->pvAddress = pvPaletteAddress;
	hPalette->pvCached = pvCachedAddress;
	hPalette->ulNumEntries = ulNumEntries;
	hPalette->ulConstraintMask = ulConstraintMask;
	hPalette->eFormat = eFormat;

	/* set return handle */
	*phPalette = hPalette;

	BDBG_LEAVE(BSUR_Palette_Create);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Palette_Destroy(
	BSUR_Palette_Handle hPalette )
{
	BDBG_ENTER(BSUR_Palette_Destroy);
	BDBG_OBJECT_ASSERT(hPalette, BSUR_Palette);

	/* free palette memory if required */
	if( hPalette->bDriverOwned && hPalette->pvAddress )
		BMEM_Heap_Free( hPalette->hMem, hPalette->pvAddress );

	/* free private palette data */
	BDBG_OBJECT_DESTROY(hPalette, BSUR_Palette);
	BKNI_Free( (void *) hPalette );

	BDBG_LEAVE(BSUR_Palette_Destroy);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Palette_GetConstraintMask(
	BSUR_Palette_Handle hPalette,
	uint32_t *pulConstraintMask )
{
	BDBG_ENTER(BSUR_Palette_GetConstraintMask);
	BDBG_OBJECT_ASSERT(hPalette, BSUR_Palette);
	BDBG_ASSERT( pulConstraintMask );

	/* set return value */
	*pulConstraintMask = hPalette->ulConstraintMask;

	BDBG_LEAVE(BSUR_Palette_GetConstraintMask);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Palette_GetNumEntries(
	BSUR_Palette_Handle hPalette,
	uint32_t *pulNumEntries )
{
	BDBG_ENTER(BSUR_Palette_GetNumEntries);
	BDBG_OBJECT_ASSERT(hPalette, BSUR_Palette);
	BDBG_ASSERT( pulNumEntries );

	/* set return value */
	*pulNumEntries = hPalette->ulNumEntries;

	BDBG_LEAVE(BSUR_Palette_GetNumEntries);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Palette_GetAddress(
	BSUR_Palette_Handle hPalette,
	void **ppvAddress )
{
	BDBG_ENTER(BSUR_Palette_GetAddress);
	BDBG_OBJECT_ASSERT(hPalette, BSUR_Palette);
	BDBG_ASSERT( ppvAddress );

	/* set return value */
	*ppvAddress = hPalette->pvAddress;

	BDBG_LEAVE(BSUR_Palette_GetAddress);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Palette_GetOffset(
	BSUR_Palette_Handle hPalette,
	uint32_t *pulOffset )
{
	BERR_Code err;

	BDBG_ENTER(BSUR_Palette_GetOffset);
	BDBG_OBJECT_ASSERT(hPalette, BSUR_Palette);
	BDBG_ASSERT( pulOffset );

	/* get offset */
	err = BMEM_ConvertAddressToOffset( hPalette->hMem, hPalette->pvAddress, pulOffset );
	if( err != BERR_SUCCESS )
		return BERR_TRACE(err);

	BDBG_LEAVE(BSUR_Palette_GetOffset);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Palette_GetFormat(
	BSUR_Palette_Handle hPalette,
	BPXL_Format *peFormat )
{
	BDBG_ENTER(BSUR_Palette_GetFormat);
	BDBG_OBJECT_ASSERT(hPalette, BSUR_Palette);
	BDBG_ASSERT( peFormat );

	/* set return value */
	*peFormat = hPalette->eFormat;

	BDBG_LEAVE(BSUR_Palette_GetFormat);
	return BERR_SUCCESS;
}

/***************************************************************************/
static const BSUR_Surface_Settings s_SUR_DefaultSettings =
{
	{ false, BSUR_P_DCX_TESTFEATURE1_BITSPERPIXEL_DEFAULT, 0 }  /* stTestFeature1Settings */
};

/***************************************************************************/
BERR_Code BSUR_Surface_GetDefaultSettings(
	BSUR_Surface_Settings *pDefSettings )
{
	BDBG_ENTER(BSUR_Surface_GetDefaultSettings);
	BDBG_ASSERT( pDefSettings );

	/* set return value */
	*pDefSettings = s_SUR_DefaultSettings;

	BDBG_LEAVE(BSUR_Surface_GetDefaultSettings);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_Create(
	BMEM_Handle hMem,
	uint32_t ulWidth,
	uint32_t ulHeight,
	uint32_t ulPitch,
	void *pvPixelAddress,
	BPXL_Format eFormat,
	BSUR_Palette_Handle hPalette,
	uint32_t ulConstraintMask,
	BSUR_Surface_Settings *pSettings,
	BSUR_Surface_Handle *phSurface )
{
	BSUR_Surface_Handle hSurface;
	unsigned int ucAlignBits = 2;
	void *pvCachedAddress = pvPixelAddress;
	BERR_Code err;

	BDBG_ENTER(BSUR_Surface_Create);
	BDBG_ASSERT( ulWidth );
	BDBG_ASSERT( ulHeight );
	BDBG_ASSERT( phSurface );

	/* check parameters */
	if( hMem == (BMEM_Handle) 0 )
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	if( pvPixelAddress && (ulPitch == 0) )
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	/* check for alignment constraints */
	if( BSUR_CONSTRAINT_MATCH(ulConstraintMask, BSUR_CONSTRAINT_ADDRESS_PIXEL_ALIGNED) )
	{
		/* check pixel address alignment */
		if( pvPixelAddress && BSUR_P_PIXEL_UNALIGNED( eFormat, pvPixelAddress ) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);

		/* check pitch alignment */
		if( ulPitch && BSUR_P_PIXEL_UNALIGNED( eFormat, ulPitch ) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* check for power of 2 constraints */
	if( BSUR_CONSTRAINT_MATCH(ulConstraintMask, BSUR_CONSTRAINT_WIDTH_POWER_2) && (ulWidth & (ulWidth - 1)) )
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	if( BSUR_CONSTRAINT_MATCH(ulConstraintMask, BSUR_CONSTRAINT_HEIGHT_POWER_2) && (ulHeight & (ulHeight - 1)) )
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	/* check for YCbCr420 alignment constraints */
	if( BSUR_CONSTRAINT_MATCH(ulConstraintMask, BSUR_CONSTRAINT_YCbCr420_FORMAT) )
	{
		/* check pixel address alignment */
		if( pvPixelAddress && BSUR_P_YCbCr420_UNALIGNED( pvPixelAddress ) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);

		/* check pitch alignment */
		if( ulPitch && BSUR_P_YCbCr420_UNALIGNED( ulPitch ) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);

		ucAlignBits = BSUR_P_YCbCr420_ALIGNMENT;
	}

	/* check for YCbCr422 10-bit alignment constraints */
	if( BSUR_CONSTRAINT_MATCH(ulConstraintMask, BSUR_CONSTRAINT_YCbCr422_10BIT_FORMAT) )
	{
		/* check pixel address alignment */
		if( pvPixelAddress && BSUR_P_YCbCr422_10BIT_UNALIGNED( pvPixelAddress ) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);

		/* check pitch alignment */
		if( ulPitch && BSUR_P_YCbCr422_10BIT_UNALIGNED( ulPitch ) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);

		ucAlignBits = BSUR_P_YCbCr422_10BIT_ALIGNMENT;
	}

	/* check for YCbCr422 10-bit packed alignment constraints */
	if( BSUR_CONSTRAINT_MATCH(ulConstraintMask, BSUR_CONSTRAINT_YCbCr422_10BIT_PACKED_FORMAT) )
	{
		/* check pixel address alignment */
		if( pvPixelAddress && BSUR_P_YCbCr422_10BIT_PACKED_UNALIGNED( pvPixelAddress ) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);

		/* check pitch alignment */
		if( ulPitch && BSUR_P_YCbCr422_10BIT_PACKED_UNALIGNED( ulPitch ) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);

		ucAlignBits = BSUR_P_YCbCr422_10BIT_PACKED_ALIGNMENT;
	}

	/* check for DCX TestFeature1 alignment constraints */
	if( BSUR_CONSTRAINT_MATCH(ulConstraintMask, BSUR_CONSTRAINT_DCX_TESTFEATURE1_FORMAT) )
	{
		/* check pixel address alignment */
		if( pvPixelAddress && BSUR_P_DCX_TESTFEATURE1_UNALIGNED( pvPixelAddress ) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);

		/* check if pitch or palette handle is zero */
		if( ulPitch && hPalette )
			return BERR_TRACE(BERR_INVALID_PARAMETER);

		/* check if format is RGB, YCbCr422 or YCbCr444 */
		if( (BPXL_IS_RGB_FORMAT(eFormat) == 0) && (BPXL_IS_YCbCr444_FORMAT(eFormat) == 0) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);

		ucAlignBits = BSUR_P_DCX_TESTFEATURE1_ALIGNMENT;
	}

	/* verify DCX TestFeature1 settings */
	if( pSettings )
	{
		if( pSettings->stTestFeature1Settings.bEnable && (
			(pSettings->stTestFeature1Settings.ulBitsPerPixel < BSUR_P_DCX_TESTFEATURE1_BITSPERPIXEL_MINIMUM) ||
			(pSettings->stTestFeature1Settings.ulBitsPerPixel > BSUR_P_DCX_TESTFEATURE1_BITSPERPIXEL_MAXIMUM)) )
			return BERR_TRACE(BERR_INVALID_PARAMETER);
	}

	/* check for palette with palette formats */
	if( BPXL_IS_PALETTE_FORMAT(eFormat) && (hPalette == 0) && (eFormat != BPXL_eP0) )
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	/* allocate memory for private surface data */
	hSurface = (BSUR_Surface_Handle) BKNI_Malloc( sizeof (BSUR_P_Surface_Handle) );
	if( hSurface == NULL )
		return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);

	BKNI_Memset( hSurface, 0, sizeof (BSUR_P_Surface_Handle) );
	BDBG_OBJECT_SET(hSurface, BSUR_Surface);

	/* allocate memory for surface pixels if required */
	if( (pvPixelAddress == NULL) && (eFormat != BPXL_eA0) && (eFormat != BPXL_eP0) )
	{
		uint32_t ulMemorySize;
		if( pSettings && pSettings->stTestFeature1Settings.bEnable )
		{
			/* DCX TestFeature1 memory size */
			uint32_t ulAlignMask = (1 << BSUR_P_DCX_TESTFEATURE1_ALIGNMENT) - 1;
			uint32_t ulMemoryBits = (pSettings->stTestFeature1Settings.ulBitsPerPixel * ulWidth * ulHeight + 1) / 2 + 16*1024 + 64;
			ulMemorySize = ((ulMemoryBits + ulAlignMask) & (~ulAlignMask)) / 8;
		}
		else
		{
			/* standard memory size */
			unsigned int uiPitch = 0;
			if( ulPitch == 0 )
			{
				BPXL_GetBytesPerNPixels( eFormat, ulWidth, &uiPitch );
				ulPitch = (uint32_t) uiPitch;
			}
			ulMemorySize = ulPitch * ulHeight;
		}

		pvPixelAddress = BMEM_Heap_AllocAligned( hMem, ulMemorySize, ucAlignBits, 0 );
		if( pvPixelAddress == NULL )
		{
			BDBG_OBJECT_DESTROY(hSurface, BSUR_Surface);
			BKNI_Free( (void *) hSurface );
			return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		}

		err = BMEM_Heap_ConvertAddressToCached( hMem, pvPixelAddress, &pvCachedAddress );
		if( err != BERR_SUCCESS )
		{
			BDBG_OBJECT_DESTROY(hSurface, BSUR_Surface);
			BMEM_Heap_Free( hMem, pvPixelAddress );
			BKNI_Free( (void *) hSurface );
			return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		}

		hSurface->bDriverOwned = true;
	}

	/* set private surface data */
	hSurface->stSettings = pSettings ? *pSettings : s_SUR_DefaultSettings;
	hSurface->hMem = hMem;
	hSurface->pvAddress = pvPixelAddress;
	hSurface->pvCached = pvCachedAddress;
	hSurface->ulWidth = ulWidth;
	hSurface->ulHeight = ulHeight;
	hSurface->ulPitch = ulPitch;
	hSurface->ulConstraintMask = ulConstraintMask;
	hSurface->eFormat = eFormat;
	hSurface->ulID = 0;

	/* attach palette */
	if( hPalette )
	{
		err = BSUR_Surface_SetPalette( hSurface, hPalette );
		if( err != BERR_SUCCESS )
		{
			BSUR_Surface_Destroy( hSurface );
			return err;
		}
	}

	/* set return handle */
	*phSurface = hSurface;

	BDBG_LEAVE(BSUR_Surface_Create);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_Destroy(
	BSUR_Surface_Handle hSurface )
{
	BDBG_ENTER(BSUR_Surface_Destroy);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);

	/* free surface memory if required */
	if( hSurface->bDriverOwned && hSurface->pvAddress )
		BMEM_Heap_Free( hSurface->hMem, hSurface->pvAddress );

	/* free private surface data */
	BDBG_OBJECT_DESTROY(hSurface, BSUR_Surface);
	BKNI_Free( (void *) hSurface );

	BDBG_LEAVE(BSUR_Surface_Destroy);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_SetPalette(
	BSUR_Surface_Handle hSurface,
	BSUR_Palette_Handle hPalette )
{
	BDBG_ENTER(BSUR_Surface_SetPalette);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);
	if (hPalette) {
		BDBG_OBJECT_ASSERT(hPalette, BSUR_Palette);
	}

	/* check compatibility between surface and palette */
	if( (!BPXL_IS_PALETTE_FORMAT(hSurface->eFormat)) || (hPalette &&
		((uint32_t) BPXL_NUM_PALETTE_ENTRIES(hSurface->eFormat) != hPalette->ulNumEntries)) )
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	/* attach palette to surface */
	hSurface->hPalette = hPalette;

	BDBG_LEAVE(BSUR_Surface_SetPalette);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_GetDimensions(
	BSUR_Surface_Handle hSurface,
	uint32_t *pulWidth,
	uint32_t *pulHeight )
{
	BDBG_ENTER(BSUR_Surface_GetDimensions);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);
	BDBG_ASSERT( pulWidth );
	BDBG_ASSERT( pulHeight );

	/* set return values */
	*pulWidth = hSurface->ulWidth;
	*pulHeight = hSurface->ulHeight;

	BDBG_LEAVE(BSUR_Surface_GetDimensions);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_GetPalette(
	BSUR_Surface_Handle hSurface,
	BSUR_Palette_Handle *phPalette )
{
	BDBG_ENTER(BSUR_Surface_GetPalette);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);
	BDBG_ASSERT( phPalette );

	/* set return value */
	*phPalette = hSurface->hPalette;

	if( hSurface->hPalette )
	{
		BDBG_OBJECT_ASSERT(*phPalette, BSUR_Palette);
	}

	BDBG_LEAVE(BSUR_Surface_GetPalette);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_GetConstraintMask(
	BSUR_Surface_Handle hSurface,
	uint32_t *pulConstraintMask )
{
	BDBG_ENTER(BSUR_Surface_GetConstraintMask);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);
	BDBG_ASSERT( pulConstraintMask );

	/* set return value */
	*pulConstraintMask = hSurface->ulConstraintMask;

	BDBG_LEAVE(BSUR_Surface_GetConstraintMask);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_GetAddress(
	BSUR_Surface_Handle hSurface,
	void **ppvPixelAddress,
	uint32_t *pulPitch )
{
	BDBG_ENTER(BSUR_Surface_GetAddress);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);
	BDBG_ASSERT( ppvPixelAddress );
	BDBG_ASSERT( pulPitch );

	/* set return values */
	*ppvPixelAddress = hSurface->pvAddress;
	*pulPitch = hSurface->ulPitch;

	BDBG_LEAVE(BSUR_Surface_GetAddress);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_GetOffset(
	BSUR_Surface_Handle hSurface,
	uint32_t *pulOffset )
{
	BERR_Code err;

	BDBG_ENTER(BSUR_Surface_GetOffset);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);
	BDBG_ASSERT( pulOffset );

	/* get offset */
	if( hSurface->pvAddress )
	{
		err = BMEM_ConvertAddressToOffset( hSurface->hMem, hSurface->pvAddress, pulOffset );
		if( err != BERR_SUCCESS )
			return BERR_TRACE(err);
	}
	else
	{
		*pulOffset = 0;
	}

	BDBG_LEAVE(BSUR_Surface_GetOffset);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_GetFormat(
	BSUR_Surface_Handle hSurface,
	BPXL_Format *peFormat )
{
	BDBG_ENTER(BSUR_Surface_GetFormat);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);
	BDBG_ASSERT( peFormat );

	/* set return value */
	*peFormat = hSurface->eFormat;

	BDBG_LEAVE(BSUR_Surface_GetFormat);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_GetPacketBlitInfo(
	BSUR_Surface_Handle hSurface,
	BPXL_Format *peFormat,
	uint32_t *pulOffset,
	uint32_t *pulPitch,
	uint32_t *pulWidth,
	uint32_t *pulHeight )
{
	BERR_Code err;
	BDBG_ENTER(BSUR_Surface_GetPacketBlitInfo);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);

	/* set return values */
	if( pulOffset )
	{
		err = BMEM_ConvertAddressToOffset( hSurface->hMem, hSurface->pvAddress, pulOffset );
		if( err != BERR_SUCCESS )
			return BERR_TRACE(err);
	}

	if( peFormat )
		*peFormat = hSurface->eFormat;

	if( pulPitch )
		*pulPitch = hSurface->ulPitch;

	if( pulWidth )
		*pulWidth = hSurface->ulWidth;

	if( pulHeight )
		*pulHeight = hSurface->ulHeight;

	BDBG_LEAVE(BSUR_Surface_GetPacketBlitInfo);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_GetID(
	BSUR_Surface_Handle hSurface,
	uint32_t *pulID )
{
	BDBG_ENTER(BSUR_Surface_GetID);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);
	BDBG_ASSERT( pulID );

	/* set return value */
	*pulID = hSurface->ulID;

	BDBG_LEAVE(BSUR_Surface_GetID);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_SetID(
	BSUR_Surface_Handle hSurface,
	uint32_t ulID )
{
	BDBG_ENTER(BSUR_Surface_SetID);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);

	/* return error if ID already set */
	if( hSurface->ulID )
		return BERR_TRACE(BERR_INVALID_PARAMETER);

	hSurface->ulID = ulID;

	BDBG_LEAVE(BSUR_Surface_SetID);
	return BERR_SUCCESS;
}

/***************************************************************************/
BERR_Code BSUR_Surface_GetTestFeature1(
	BSUR_Surface_Handle hSurface,
	BSUR_TestFeature1_Settings *pTestFeature1Settings )
{
	BDBG_ENTER(BSUR_Surface_GetTestFeature1);
	BDBG_OBJECT_ASSERT(hSurface, BSUR_Surface);
	BDBG_ASSERT( pTestFeature1Settings );

	/* set return value */
	*pTestFeature1Settings = hSurface->stSettings.stTestFeature1Settings;

	BDBG_LEAVE(BSUR_Surface_GetTestFeature1);
	return BERR_SUCCESS;
}

void BSUR_Surface_InitPixelPlane(BSUR_Surface_Handle hSurface, BPXL_Plane *pPlane)
{
    BPXL_Plane_Init(pPlane, hSurface->ulWidth, hSurface->ulHeight, hSurface->eFormat);
    pPlane->ulPitch = hSurface->ulPitch;

    BSUR_Surface_GetOffset(hSurface, &pPlane->ulPixelsOffset);
    if(hSurface->hPalette) {
        BSUR_Palette_GetOffset(hSurface->hPalette, &pPlane->ulPaletteOffset);
    }
    return;
}

BERR_Code BSUR_Surface_CreateFromPixelPlane(BMEM_Handle hMem, const BPXL_Plane *pPlane, BSUR_Surface_Handle *phSurface)
{
    void *pixels;

    if(pPlane->hPalette) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    pixels = BMMA_Lock(pPlane->hPixels);
    if(pixels==NULL) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BMMA_Unlock(pPlane->hPixels, pixels);
    pixels = (uint8_t *)pixels + pPlane->ulPixelsOffset;

    return BSUR_Surface_Create(hMem,
                               pPlane->ulWidth,
                               pPlane->ulHeight,
                               pPlane->ulPitch,
                               pixels,
                               pPlane->eFormat,
                               NULL,
                               0,
                               NULL,
                               phSurface);

}


/* End of File */
