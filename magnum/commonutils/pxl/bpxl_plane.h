/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/


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
    unsigned            ulMipLevel;             /* level for UIF mip level;*/
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
    This function initializes the BPXL_Plane structure for Uif format only

Description:
    BPXL_Plane_Uif_Init initializes the user provided pointer to BPXL_Plane
    with default values, based on the user specified information.
    Every new instance of BPXL_Plane should be initialized using this function.
    All fields in the BAVC_PixePlane would be lost.
    This function will not allocate memory for the PixelPlane
****************************************************************************/

BERR_Code BPXL_Plane_Uif_Init(
    BPXL_Plane *plane,
    size_t width,
    size_t height,
    unsigned mipLevel,
    BPXL_Format format,
    BCHP_Handle hChip);

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
