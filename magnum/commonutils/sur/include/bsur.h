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
#ifndef BSUR_H__
#define BSUR_H__

#include "bchp.h"
#include "bmem.h"
#include "bpxl.h"
#include "bpxl_plane.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=Module Overview: ********************************************************
The purpose of the surface module is to allocate surfaces for use with
other modules. The video display control module uses surfaces for 
hardware cursor support, the graphics compositor module uses surfaces as
a standard input for many of its operations, and the graphics feeder uses
surfaces to represent frame buffers.

This module contains functions to create, destroy and modify surfaces 
and palettes. Palettes are associated with surfaces which specify a
format that requires a palette.

Supported features:
o Can create surface with any pixel format.
o Can create palettes with 4, 16, and 256 entries.
o TODO: Complete list of specific features that are supported and unit
  tested for this module.
****************************************************************************/

#define BSUR_SURFACE_CREATE_HAS_SETTINGS

/***************************************************************************
Summary:
	A handle representing a surface.

Description:
	This handle is used by surface module functions to store a two 
	dimensional array of pixels.

See Also:
	BSUR_Surface_Create
****************************************************************************/
typedef struct BSUR_P_Surface_Handle *BSUR_Surface_Handle;

/***************************************************************************
Summary:
	A handle representing a palette.

Description:
	This handle is used by surface module functions to store a 
	one dimensional array of colors. This handle is used with a surface
	handle when the format of a surface requires a palette.

See Also:
	BSUR_Palette_Create,
	BSUR_Surface_Handle
****************************************************************************/
typedef struct BSUR_P_Palette_Handle *BSUR_Palette_Handle;

/***************************************************************************
Summary:
	Surface TestFeature1 settings structure.

Description:
	This structure is a field in the BSUR_Surface_Settings structure, and 
	is used to set the TestFeature1 settings of a surface when creating it.

	bEnable:        Enables/disables TestFeature1. (default=false)
	ulBitsPerPixel: Bits per pixel in units of 0.5 bits per pixel. 
	                Range from 14 to 64. Note this is in U31.1 fixed point 
	                format, with 31 bit integer and 1 bit fraction.
	                The greater BitsPerPixel is, the better picture quality.
					(default=14,minimum)
	ulPredicationMode: Predication mode, 0=Normal, 1=NOLEFT, 2=NO2LEFT
	                   (default=0,Normal)

See Also:
	BSUR_Surface_Settings,
	BSUR_Surface_Create,
	BSUR_Surface_GetTestFeature1,
	BSUR_Surface_GetDefaultSettings
****************************************************************************/
typedef struct
{
	bool bEnable;                    /* Enables TestFeature1 */
	uint32_t ulBitsPerPixel;         /* Bits per pixel in units of 0.5 */
	uint32_t ulPredictionMode;       /* Predication mode, 0=Normal, 1=NOLEFT, 2=NO2LEFT */
}
BSUR_TestFeature1_Settings;

/***************************************************************************
Summary:
	TestFeature1 settings structure.

Description:
	This structure is used to set the extended settings of a surface
	when creating it.

	stTestFeature1Settings: TestFeature1 settings.

See Also:
	BSUR_TestFeature1_Settings,
	BSUR_Surface_Create,
	BSUR_Surface_GetDefaultSettings
****************************************************************************/
typedef struct
{
	BSUR_TestFeature1_Settings stTestFeature1Settings;
}
BSUR_Surface_Settings;

/***************************************************************************
Summary:
	Constraint masks.

Description:
	The constraint mask is a bit mask describing which restrictions 
	should be put on the properties of a palette or surface when 
	creating it.

See Also:
	BSUR_Palette_Create,
	BSUR_Surface_Create
****************************************************************************/
#define BSUR_CONSTRAINT_ADDRESS_PIXEL_ALIGNED        0x1   /* address and pitch must be pixel aligned */
#define BSUR_CONSTRAINT_WIDTH_POWER_2                0x2   /* width in pixels must be a power of 2 */
#define BSUR_CONSTRAINT_HEIGHT_POWER_2               0x4   /* height in pixels must be a power of 2 */
#define BSUR_CONSTRAINT_YCbCr420_FORMAT              0x8   /* address and pitch must be YCbCr420 aligned (16 dwords) */
#define BSUR_CONSTRAINT_YCbCr422_10BIT_FORMAT        0x10  /* address and pitch must be YCbCr422 10-bit aligned (4 dwords) */
#define BSUR_CONSTRAINT_YCbCr422_10BIT_PACKED_FORMAT 0x20  /* address and pitch must be YCbCr422 10-bit packed aligned (5 dwords) */
#define BSUR_CONSTRAINT_DCX_TESTFEATURE1_FORMAT      0x40  /* address must be DCX TestFeature1 aligned (64 bytes) */

/***************************************************************************
Summary:
	Checks if surface has required constraints.

Description:
	This macro is used to determine which contraints a surface has.
	Possible contraints include BSUR_CONSTRAINT_ADDRESS_PIXEL_ALIGNED,
	BSUR_CONSTRAINT_WIDTH_POWER_2, and BSUR_CONSTRAINT_HEIGHT_POWER_2.

****************************************************************************/
#define BSUR_CONSTRAINT_MATCH(surface_constraint, required_constraint) \
	(((surface_constraint) & (required_constraint)) == (required_constraint))

/***************************************************************************
Summary:
	Creates a palette.

Description:
	Palettes are created and used by surfaces that need them. They are
	attached to a surface and may be shared between surfaces.
	Surfaces which require palettes must have one associated with it.

	There are two ways to create a palette:

    o Module allocated palette: The user doesn't provide a palette
	  address, flagging this function to allocate memory from the 
	  surface module's memory heap. When the palette is destroyed,
	  this memory will be freed by the module. This memory is
	  considered to be uninitialized after creation.

    o User allocated palette: The user provides a palette address, 
	  flagging the function that this given memory should be used for
	  the palette and that no additional memory should be allocated to
	  store the palette. When the palette is destroyed the provided memory
	  will not be freed. Checks will be made by this function to
	  insure that the provided memory can be used for a palette.

	The user must provide the format of the pixels stored in the palette.
	This user must know what this format is in order to load the palette
	with appropriate color values. Currently, all palettes must use a
	format of BPXL_eA8_Y8_Cb8_Cr8 or BPXL_eA8_R8_G8_B8.

	The constraint mask is a bit mask describing which restrictions 
	should be put on the properties of the palette when creating it. 
	Currently the only valid constraint for this function is 
	BSUR_CONSTRAINT_ADDRESS_PIXEL_ALIGNED.

Returns:
	BERR_SUCCESS - Palette was successfully created.
	BERR_INVALID_PARAMETER - Input parameter value was invalid.
	BERR_OUT_OF_SYSTEM_MEMORY - Unable to allocate memory for the handle.
	BERR_OUT_OF_DEVICE_MEMORY - Unable to allocate memory for the 
	palette entries themselves.

See Also:
	BSUR_Palette_Destroy
	BSUR_Surface_Create
****************************************************************************/
BERR_Code BSUR_Palette_Create(
	BMEM_Handle hMem,               /* [in] Handle to BMEM module */
	uint32_t ulNumEntries,          /* [in] Number of palette entries */
	void *pvPaletteAddress,         /* [in] Pointer to first palette entry */
	BPXL_Format eFormat,            /* [in] Format of palette */
	uint32_t ulConstraintMask,      /* [in] Constraint mask */
	BSUR_Palette_Handle *phPalette  /* [out] Pointer to palette handle */
);

/***************************************************************************
Summary:
	Destroys a created palette.

Description:
	If the palette memory was allocated by the surface module it will be
	freed.

    The user should call this function only after all modules are no
	longer using this palette.

	Failing to destroy all created palettes will result in an error when
	the module is closed with BSUR_Close.

Returns:
	BERR_SUCCESS - Palette was successfully destroyed.

See Also:
	BSUR_Palette_Create
	BSUR_Close
****************************************************************************/
BERR_Code BSUR_Palette_Destroy(
	BSUR_Palette_Handle hPalette    /* [in] Handle of palette to destroy */
);

/***************************************************************************
Summary:
	Returns the constraint mask associated with the palette.

Description:
	The constraint mask is a bit mask describing which restrictions 
	were put on the properties of the palette when it was created. 

Returns:
	BERR_SUCCESS - Returned Constraint mask is valid.

See Also:
	BSUR_Palette_Create
****************************************************************************/
BERR_Code BSUR_Palette_GetConstraintMask(
	BSUR_Palette_Handle hSurface,    /* [in] Handle to palette to examine */
	uint32_t *pulConstraintMask      /* [out] Pointer to constraint mask */
);

/***************************************************************************
Summary:
	Returns the number of entries in the specified palette.

Description:

Returns:
	BERR_SUCCESS - Returned count is valid.
****************************************************************************/
BERR_Code BSUR_Palette_GetNumEntries(
	BSUR_Palette_Handle hPalette,    /* [in] Handle to palette to examine */
	uint32_t *pulNumEntries          /* [out] Pointer to number of entries */
);

/***************************************************************************
Summary:
	Returns the address of the palette entries.

Description:
    The address returned will point to the actual location of the
	palette. The user can use this data to fill in the palette
	with appropriate values. The format of the palette determines
	the type of entries pointed to by this address.

Returns:
	BERR_SUCCESS - Returned address is valid.
****************************************************************************/
BERR_Code BSUR_Palette_GetAddress(
	BSUR_Palette_Handle hPalette,    /* [in] Handle to palette to examine */
	void **ppvAddress                /* [out] Pointer to palette address */
);

/***************************************************************************
Summary:
	Returns the offset of the palette.

Returns:
	BERR_SUCCESS - Returned address is valid.
****************************************************************************/
BERR_Code BSUR_Palette_GetOffset(
	BSUR_Palette_Handle hPalette,    /* [in] Handle to palette to examine */
	uint32_t *pulOffset              /* [out] Pointer to palette offset */
);

#define BSUR_Palette_GetEntryFormat   BSUR_Palette_GetFormat /* for back compatibility only */	
/***************************************************************************
Summary:
	Returns the pixel format of the palette entries.

Description:
	Currently, the supported pixel formats for palettes entries are 
	BPXL_eA8_Y8_Cb8_Cr8 and BPXL_eA8_R8_G8_B8.

Returns:
	BERR_SUCCESS - Returned entry pixel format is valid.

See Also:
	BSUR_Palette_Create
****************************************************************************/
BERR_Code BSUR_Palette_GetFormat(
	BSUR_Palette_Handle hPalette,    /* [in] Handle to palette to examine */
	BPXL_Format *peEntryFormat       /* [out] Pointer to entry format */
);
	
/***************************************************************************
Summary:
	Gets default surface settings.

Description:
	This function gets the default surface settings that can be modified
	and used when creating a surface. The surface will adopt the specified
	settings when created.

Returns:
	BERR_SUCCESS - Returned settings are valid.

See Also:
	BSUR_Surface_Create,
	BSUR_Surface_Settings,
	BSUR_TestFeature1_Settings
****************************************************************************/
BERR_Code BSUR_Surface_GetDefaultSettings(
	BSUR_Surface_Settings *pDefSettings   /* [out] Ptr to default settings */
);

/***************************************************************************
Summary:
	Creates a surface.

Description:
	There are two ways to create a surface:
	
	o Module allocated pixels: The user doesn't provide a pixel address,
	  flagging the function to allocate memory from the surface module's
	  memory heap. The user may specify a pitch value to use for this
	  allocation or specify zero if they wish the function to calculate
	  the pitch. When the surface is destroyed this pixel memory will be
	  freed by the module. This memory is considered to be uninitialized
	  after creation.

	o User allocated pixels: The user provides a pixel address, flagging
	  that no memory should be allocated by the module. Instead, the
	  function should use the memory provided. When this option is
	  given the supplied pitch cannot be zero. When the surface is
	  destroyed the provided pixel memory will not be freed.

	If the surface format requires a palette, a handle to one must be
	provided to this function. If the surface's format does not require 
	a palette handle, the provided palette handle must be NULL.

	The constraint mask is a bit mask describing which restrictions 
	should be put on the properties of the surface when creating it. 
	Currently the valid constraints for this function are
	BSUR_CONSTRAINT_ADDRESS_PIXEL_ALIGNED, BSUR_CONSTRAINT_WIDTH_POWER_2,
	and BSUR_CONSTRAINT_HEIGHT_POWER_2.

Returns:
	BERR_SUCCESS - Surface was successfully created.
	BERR_INVALID_PARAMETER - Input parameter value was invalid.
	BERR_OUT_OF_SYSTEM_MEMORY - Unable to allocate memory for the handle.
	BERR_OUT_OF_DEVICE_MEMORY - Unable to allocate memory for the surface
	itself.

See Also:
	BSUR_Surface_Destroy
****************************************************************************/
BERR_Code BSUR_Surface_Create(
	BMEM_Handle hMem,                /* [in] Handle to BMEM module */
	uint32_t ulWidth,                /* [in] Width of surface in pixels */
	uint32_t ulHeight,               /* [in] Height of surface in pixels */
	uint32_t ulPitch,                /* [in] Pitch of surface in bytes */
	void *pvPixelAddress,            /* [in] Pointer surface's first pixel */
	BPXL_Format eFormat,             /* [in] Pixel format of surface */
	BSUR_Palette_Handle hPalette,    /* [in] Palette to associate with */
	uint32_t ulConstraintMask,       /* [in] Constraint mask */
	BSUR_Surface_Settings *pSettings,/* [in] Extended settings, can be NULL*/
	BSUR_Surface_Handle *phSurface   /* [out] Pointer to surface handle */
);

/***************************************************************************
Summary:
	Destroys the provided surface.

Description:
	If the pixel memory was allocated by the surface module it will be
	freed. Associated palette handles are left untouched.

	The user should call this function only after all modules are no
	longer using this surface.

	Failing to destroy all created surfaces will result in an error
	when the module is closed with BSUR_Close.

Returns:
	BERR_SUCCESS - Palette was successfully destroyed.

See Also:
	BSUR_Surface_Create
****************************************************************************/
BERR_Code BSUR_Surface_Destroy(
	BSUR_Surface_Handle hSurface     /* [in] Handle to surface to destroy */
);

/***************************************************************************
Summary:
	Sets the palette associated with a given surface.

Description:
	This will be allowed as long as the surface requires a palette and the
	supplied palette contains at least the number of required entries.

Returns:
	BERR_SUCCESS - Surface's palette was successfully changed.
	BERR_INVALID_PARAMETER - Input parameter value was invalid.
****************************************************************************/
BERR_Code BSUR_Surface_SetPalette(
	BSUR_Surface_Handle hSurface,    /* [in] Handle to surface */
	BSUR_Palette_Handle hPalette     /* [in] Handle to palette */
);

/***************************************************************************
Summary:
	Returns the pixel width and height of a given surface.

Description:
	Gets the width and height of the supplied surface.

Returns:
	BERR_SUCCESS - Returned dimensions are valid.:

See Also:
	BSUR_Surface_Create
****************************************************************************/
BERR_Code BSUR_Surface_GetDimensions(
	BSUR_Surface_Handle hSurface,   /* [in] Handle to surface to examine */
	uint32_t *pulWidth,             /* [out] Pointer to width in pixels*/
	uint32_t *pulHeight             /* [out] Pointer to height in pixels */
);

/***************************************************************************
Summary:
	Returns the palette associated with a surface.

Description:
	If the surface does not have a palette associated with it, this
	function will return an error.

Returns:
	BERR_SUCCESS - Returned palette handle is valid.
	BERR_INVALID_PARAMETER - Surface does not contain a palette.
****************************************************************************/
BERR_Code BSUR_Surface_GetPalette(
	BSUR_Surface_Handle hSurface,   /* [in] Handle to surface to examine */
	BSUR_Palette_Handle *phPalette  /* [out] Pointer to palette handle */
);

/***************************************************************************
Summary:
	Returns the Constraint mask associated with the surface.

Description:
	The constraint mask is a bit mask describing which restrictions 
	were put on the properties of the surface when it was created. 

Returns:
	BERR_SUCCESS - Returned Constraint mask is valid.
****************************************************************************/
BERR_Code BSUR_Surface_GetConstraintMask(
	BSUR_Surface_Handle hSurface,   /* [in] Handle to surface to examine */
	uint32_t *pulConstraintMask     /* [out] Pointer to constraint mask */
);

/***************************************************************************
Summary:
	Returns information about pixel memory.

Description:
	Returns pixel address and pitch of surface.

Returns:
	BERR_SUCCESS - Returned information is valid.:
****************************************************************************/
BERR_Code BSUR_Surface_GetAddress(
	BSUR_Surface_Handle hSurface,   /* [in] Handle to surface to examine */
	void **ppvPixelAddress,         /* [out] Pointer to pixel address */
	uint32_t *pulPitch              /* [out] Pointer to pitch in bytes */
);

/***************************************************************************
Summary:
	Returns information about pixel memory at isr.

Description:
	Returns pixel address and pitch of surface.

Returns:
	BERR_SUCCESS - Returned information is valid.:

See Also:
	BSUR_Surface_GetAddress
****************************************************************************/
#define BSUR_Surface_GetAddress_isr(hSurface, ppvPixelAddress, pulPitch) \
	BSUR_Surface_GetAddress(hSurface, ppvPixelAddress, pulPitch)

/***************************************************************************
Summary:
	Returns the offset of the surface.

Returns:
	BERR_SUCCESS - Returned address is valid.
****************************************************************************/
BERR_Code BSUR_Surface_GetOffset(
	BSUR_Surface_Handle hSurface,    /* [in] Handle to surface to examine */
	uint32_t *pulOffset              /* [out] Pointer to surface offset */
);

/***************************************************************************
Summary:
	Returns offset of surface at isr.

Returns:
	BERR_SUCCESS - Returned information is valid.:
****************************************************************************/
#define BSUR_Surface_GetOffset_isr(hSurface, pulOffset) \
	BSUR_Surface_GetOffset(hSurface, pulOffset)

/***************************************************************************
Summary:
	Returns pixel format for the surface.

Description:

Returns:
	BERR_SUCCESS - Returned information is valid.:
****************************************************************************/
BERR_Code BSUR_Surface_GetFormat(
	BSUR_Surface_Handle hSurface,   /* [in] Handle to surface to examine */
	BPXL_Format *peFormat           /* [out] Pointer to pixel format */
);

/***************************************************************************
Summary:
	Returns packet blit information for the surface.

Description:

Returns:
	BERR_SUCCESS - Returned information is valid.:
****************************************************************************/
BERR_Code BSUR_Surface_GetPacketBlitInfo(
	BSUR_Surface_Handle hSurface,   /* [in] Handle to surface to examine */
	BPXL_Format *peFormat,          /* [out] Pointer to pixel format */
	uint32_t *pulOffset,            /* [out] Pointer to surface offset */
	uint32_t *pulPitch,             /* [out] Pointer to pitch in bytes */
	uint32_t *pulWidth,             /* [out] Pointer to width in pixels*/
	uint32_t *pulHeight             /* [out] Pointer to height in pixels*/
);

/***************************************************************************
Summary:
	Returns ID for the surface.

Description:

Returns:
	BERR_SUCCESS - Returned information is valid.:
****************************************************************************/
BERR_Code BSUR_Surface_GetID(
	BSUR_Surface_Handle hSurface,   /* [in] Handle to surface to examine */
	uint32_t *pulID                 /* [out] Pointer to ID */
);

/***************************************************************************
Summary:
	Sets ID for the surface.

Description:
	Can only be set when current ID is zero (uninitialized).

Returns:
	BERR_SUCCESS - Returned information is valid.:
****************************************************************************/
BERR_Code BSUR_Surface_SetID(
	BSUR_Surface_Handle hSurface,   /* [in] Handle to surface to examine */
	uint32_t ulID                   /* [in] Surface ID */
);

/***************************************************************************
Summary:
	Gets surface TestFeature1 settings.

Description:
	This function gets the surface TestFeature1 settings that can be
	specified through the BSUR_Surface_Settings structure when creating 
	a surface.

Returns:
	BERR_SUCCESS - Returned settings are valid.

See Also:
	BSUR_Surface_Create,
	BSUR_Surface_Settings,
	BSUR_TestFeature1_Settings,
	BSUR_Surface_SetTestFeature1
****************************************************************************/
BERR_Code BSUR_Surface_GetTestFeature1(
	BSUR_Surface_Handle hSurface,     /* [in] Handle to surface to examine */
	BSUR_TestFeature1_Settings *pTestFeature1Settings  /* [out] Ptr to TestFeature1 settings */
);

/***************************************************************************
Summary:
    Initializes BPXL_Plane structure based on surface

Description:
    This is convenience function for application that are transiting from BSUR to BPXL_Plane

****************************************************************************/
void BSUR_Surface_InitPixelPlane(
    BSUR_Surface_Handle hSurface,     /* [in] Handle to surface to examine */
    BPXL_Plane *pPlane           /* [in] pointer to structure to be initialized */
);

/***************************************************************************
Summary:
    Creates instance of BSUR_Surface based on BPXL_Plane

Description:
    This is convenience function for application that are transiting from BSUR to BPXL_Plane

****************************************************************************/

BERR_Code BSUR_Surface_CreateFromPixelPlane(
    BMEM_Handle hMem,               /* [in] Handle to BMEM module */
    const BPXL_Plane *pPlane,       /* [in] pointer to structure to be initialized */
    BSUR_Surface_Handle *phSurface  /* [out] Pointer to surface handle */
);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef BSUR_H__ */

/* end of file */
