/***************************************************************************
 *     Copyright (c) 2013, Broadcom Corporation
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
 * [File Description:]
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#ifndef BPXL_PLANE_H__
#define BPXL_PLANE_H__

#include "bmma.h"
#include "bpxl.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
Summary:
    Pixel Plane (Surface)
Description:
    This structure describes graphics plane.
    It's regular data structure and it could be copied and assigned without restrictions.
    For backward compatibility reasons, if hPixels is NULL, then ulPixelsOffset
    should be treated as the device offset.
    Same applies for hPalette and ulPaletteOffset for palletized surface
****************************************************************************/
typedef struct BPXL_Plane {
    BMMA_Block_Handle   hPixels;        /* memory block that holds pixels */
    unsigned            ulPixelsOffset; /* offset from start of hPixels */
    size_t              ulWidth;        /* width of surface in pixels */
    size_t              ulHeight;       /* height of surface in pixels */
    size_t              ulPitch;        /* bpxl aligned pitch of surface in bytes */
    size_t              ulBufSize;      /* bpxl aligned buf size of surface in bytes */
    unsigned            ulAlignment;    /* bpxl decided alignment for start address, in bytes, power of 2 */
    BPXL_Format         eFormat;        /* Pixel format of surface */

    size_t              ulNumPaletteEntries;    /* Number of palette entries, 0 if not palletized surface */
    BPXL_Format         ePalettePixelFormat;    /* Format of palette */
    BMMA_Block_Handle   hPalette;               /* memory block that holds pixels */
    unsigned            ulPaletteOffset;        /* offset from start of hPalette */
} BPXL_Plane;

/***************************************************************************
Summary:
    This function initializes the BPXL_Plane structure

Description:
    BPXL_Plane_Init initializes the user provided pointer to BPXL_Plane
    with default values, based on the user specified information.
    Every new instance of BPXL_Plane should be initialized using this function.
    All fields in the BAVC_PixePlane would be lost.
    This function will not allocate memory for the PixelPlane
****************************************************************************/
void BPXL_Plane_Init(
        BPXL_Plane *pPlane,  /* pointer to structure to be initialized */
        size_t witdth,            /* width of surface in pixels */
        size_t height,            /* height of surface in pixels */
        BPXL_Format format        /* pixel format for the surface */
        );

/***************************************************************************
Summary:
    This function allocate buffers in BPXL_Plane

Description:
    BPXL_Plane_AllocateBuffers function could be used to allocate buffers
    from the user provided heap, using allocation parameters best suited to specified plane
    This function is use convenience function, and not necessarily to be called for each plane,
    for example user could use allocation scheme different from implemented in this function.

Returns:
    BERR_SUCCESS - Surface was successfully created.
    BERR_OUT_OF_DEVICE_MEMORY - Unable to allocate memory for the surface
****************************************************************************/
BERR_Code BPXL_Plane_AllocateBuffers(
        BPXL_Plane *pPlane,
        BMMA_Heap_Handle hHeap
        );

#ifdef __cplusplus
}
#endif

#endif /* BPXL_PLANE_H__ */
